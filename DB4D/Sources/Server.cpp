/*
* This file is part of Wakanda software, licensed by 4D under
*  (i) the GNU General Public License version 3 (GNU GPL v3), or
*  (ii) the Affero General Public License version 3 (AGPL v3) or
*  (iii) a commercial license.
* This file remains the exclusive property of 4D and/or its licensors
* and is protected by national and international legislations.
* In any event, Licensee's compliance with the terms and conditions
* of the applicable license constitutes a prerequisite to any use of this file.
* Except as otherwise expressly stated in the applicable license,
* such license does not include any other license or rights on this file,
* 4D's and/or its licensors' trademarks and/or other proprietary rights.
* Consequently, no title, copyright or other proprietary rights
* other than those specified in the applicable license is granted.
*/
#include "4DDBHeaders.h"

#if debugServer_Streams
uBOOL debugServer_Streams_state = 1;
#endif

DB4DConnectionHandlerCollection DB4DConnectionHandler::sAllConnections;
VCriticalSection DB4DConnectionHandler::sAllConnectionsMutex;
Boolean DB4DConnectionHandler::sAcceptNewConnection = true;

const sLONG kTagOpenConnection = 4448;
const sLONG kTagRetourOpenConnection = 4447;
const sLONG kTagReOpenConnection = 4449;


bool _WithSelectIo()
{
	static bool sWithSelectIO = false;
	static bool sWithSelectIO_inited = false;

	if (!sWithSelectIO_inited)
	{
		VFolder* execfold = VFolder::RetainSystemFolder( eFK_Executable, false);
		if (execfold != nil)
		{
			VFile ff(*execfold, L"NoSelectIO.txt");
			sWithSelectIO = !ff.Exists();
			execfold->Release();
		}
		sWithSelectIO_inited = true;
	}

	return sWithSelectIO;
}



ServerManager::ServerManager()
{
	fServer = nil;

}


ServerManager::~ServerManager()
{
	Stop();
}


VError ServerManager::Start(sWORD inPortNumber, bool inSSL)
{
	VError err = VE_OK;

	if (fServer == nil)
	{
		fServer = new VServer();
		if (fServer == nil)
			err = memfull;
	}

	if (err == VE_OK)
	{
		VTCPConnectionListener* vtcpcListener = new VTCPConnectionListener ( /*VDBMgr::GetManager()->GetRequestLogger()*/ );
		if (vtcpcListener == nil)
			err = ThrowBaseError(memfull, DBaction_StartingServer);
		else
		{
			IDB4D_ApplicationIntf *applicationIntf = VDBMgr::GetManager()->GetApplicationInterface();
			if (applicationIntf != nil)
			{
				vtcpcListener->AddWorkerPool( applicationIntf->GetSharedWorkerPool());
				if (_WithSelectIo())
				{
					vtcpcListener->AddSelectIOPool( applicationIntf->GetSharedSelectIOPool());
				}
			}

			vtcpcListener->SetName(L"DB4D Server");

			DB4DConnectionHandlerFactory* DB4DFactory = new DB4DConnectionHandlerFactory ( );
			if (DB4DFactory == nil)
				err = ThrowBaseError(memfull, DBaction_StartingServer);
			else
			{
#if WITH_DEPRECATED_IPV4_API			
				DB4DFactory-> SetIP ( 0 ); // listen to all IP addresses
				err = DB4DFactory-> AddNewPort ( inPortNumber );
#else
				DB4DFactory->SetIP(VNetAddress::GetAnyIP());
				err=DB4DFactory->AddNewPort(inPortNumber);
#endif

				DB4DFactory-> SetIsSSL ( inSSL );
				if (err == VE_OK)
				{
					err = vtcpcListener-> AddConnectionHandlerFactory ( DB4DFactory );
					if (err == VE_OK)
					{
						err = fServer-> AddConnectionListener ( vtcpcListener );
						if (err == VE_OK)
						{
							err = fServer-> Start ( );
						}
					}
				}
				DB4DFactory-> Release ( );
			}
		}
	}

	if (err != VE_OK)
		err = ThrowBaseError(VE_DB4D_CANNOT_START_SERVER, DBaction_StartingServer);
	return err;
}


void ServerManager::Stop()
{
	if (fServer != nil)
	{
		fServer->Stop();
		fServer->Release();
		fServer = nil;
	}
}




					// ------------------------------------------------------


VConnectionHandler* DB4DConnectionHandlerFactory::CreateConnectionHandler ( VError& outError )
{
	VConnectionHandler* result = nil;
	if (DB4DConnectionHandler::AcceptNewConnection())
	{
		result = new DB4DConnectionHandler();
		if (result == nil)
		{
			outError = ThrowBaseError(memfull, DBaction_ServerConnecting);
		}
		else
		{
			DB4DConnectionHandler::AddNewConnection((DB4DConnectionHandler*)result);
			outError = VE_OK;
		}
	}
	else
		outError = VE_DB4D_TASK_IS_DYING;
	return result;
}


VError DB4DConnectionHandlerFactory::AddNewPort ( PortNumber iPort )
{
	if (find(fPorts.begin(), fPorts.end(), iPort) == fPorts.end())
	{
		try
		{
			fPorts.push_back(iPort);
		}
		catch (...)
		{
			return ThrowBaseError(memfull, DBaction_ServerConnecting);
		}
	}
	return VE_OK;
}


VError DB4DConnectionHandlerFactory::GetPorts ( vector<PortNumber>& oPorts )
{
	try
	{
		oPorts = fPorts;
	}
	catch (...)
	{
		return ThrowBaseError(memfull, DBaction_ServerConnecting);
	}
	return VE_OK;
}



					// ------------------------------------------------------


DB4DConnectionHandler::DB4DConnectionHandler ( )
{
	fEndPoint = nil;
	fClientNeedSwap = false;
	fClientIsSwapped = false;
	fContext = nil; //fContext = new VDB4DContext();
	fShouldStop = false;
}


DB4DConnectionHandler::~DB4DConnectionHandler ( )
{
	{
		VTaskLock lock(&sAllConnectionsMutex);
		sAllConnections.erase(this);
	}

	ReleaseRefCountable( &fEndPoint);
	ReleaseRefCountable( &fContext);
}


VError DB4DConnectionHandler::SetEndPoint ( VEndPoint* inEndPoint )
{
	fEndPoint = dynamic_cast<VTCPEndPoint*> ( inEndPoint );
	if (fEndPoint == nil)
		return VE_DB4D_INVALID_PARAMETER;
	else
	{
		if (_WithSelectIo())
		{
			fEndPoint-> SetIsBlocking ( false );
			fEndPoint->SetIsSelectIO(true);
		}
		else
			fEndPoint-> SetIsBlocking ( true );
		return VE_OK;
	}
}

enum VConnectionHandler::E_WORK_STATUS DB4DConnectionHandler::Handle ( VError& outError )
{
	const sLONG quibufferlen = 8192;
	char quickbuffer[quibufferlen];
	VUUID receivedClientID;

	VTask *currentTask = VTask::GetCurrent();
	fOwnerTask = VTask::GetCurrentID();
	sLONG tag = 0;
	VError err = fEndPoint->ReadExactly(&tag, 4);
	if (err == VE_OK)
	{
		if (tag != kTagOpenConnection && tag != kTagReOpenConnection)
		{
			ByteSwap(&tag);
			fClientIsSwapped = true;
			if (tag != kTagOpenConnection && tag != kTagReOpenConnection)
				err = VE_DB4D_INVALID_PARAMETER;
		}
		if (err == VE_OK)
		{
			if (tag == kTagOpenConnection)
			{
				VUUIDBuffer xid;
				err = fEndPoint->ReadExactly(&xid, sizeof(xid));
				VUUID newid(xid);
				if (err == VE_OK)
				{
					fContext = VDBMgr::GetManager()->RetainOrCreateContext(newid, nil, kJSContextCreator, false);
					if (fContext == nil)
						err = memfull;
				}
				if (err == VE_OK)
				{
					err = fEndPoint->ReadExactly(&xid, sizeof(xid));
					receivedClientID.FromBuffer(xid);
				}
				
				// read extra data
				if (err == VE_OK)
				{
					uLONG extraSize = 0;
					err = fEndPoint->ReadExactly( &extraSize, sizeof(extraSize));
					if ( (err == VE_OK) && (extraSize > 0) )
					{
						VMemoryBuffer<> buffer;
						if (buffer.SetSize( extraSize))
						{
							err = fEndPoint->ReadExactly( buffer.GetDataPtr(), extraSize);
							if (err == VE_OK)
							{
								VValueBag *extraData = new VValueBag;
								if (extraData != nil)
								{
									VConstPtrStream stream( buffer.GetDataPtr(), buffer.GetDataSize());
									stream.OpenReading();
									extraData->ReadFromStream( &stream);
									if (stream.CloseReading() == VE_OK)
									{
										VImpCreator<VDB4DContext>::GetImpObject(fContext)->SetExtraData( extraData);
									}
								}
								ReleaseRefCountable( &extraData);
							}
						}
						else
						{
							err = memfull;
						}
					}
				}
				
			}
			else
			{
				assert(tag == kTagReOpenConnection);
				sLONG len = 0;
				err = fEndPoint->ReadExactly(&len, sizeof(sLONG));
				if (fClientIsSwapped)
					ByteSwap(&len);
				if (err == VE_OK)
				{
					VString sID;
					err = fEndPoint->ReadExactly(sID.GetCPointerForWrite(len), len*2);
					sID.Validate(len);
					if (err == VE_OK)
					{
						fContext = VDBMgr::GetManager()->StealPostPonedContext(sID);
						if (fContext == nil)
							err = memfull;
					}
				}

			}
		}
		sLONG tagretour = 0;
		uLONG len = 4;
		if (err == VE_OK)
		{
			tagretour = kTagRetourOpenConnection;
		}
		VError err2 = fEndPoint->WriteExactly(&tagretour, len);
		if (err == VE_OK)
			err = err2;
	}

	if (err == VE_OK)
	{
		VDB4DContext *context = VImpCreator<VDB4DContext>::GetImpObject(fContext);
		if (context != nil)
		{
			const VValueBag *extradata = context->GetExtraData();
			if (extradata != nil)
			{
				VString taskname;
				extradata->GetString(DB4DBagKeys::task_name, taskname);
				currentTask->SetName(taskname);
			}
		}
		
		currentTask->SetKind(kDB4DTaskKind_WorkerPool_User);
		/* Do not curtask->SetKindData(): the TaskKindData is handled by the worker pool (see VExclusiveWorker)*/

		IDB4D_ApplicationIntf *applicationIntf = VDBMgr::GetManager()->GetApplicationInterface();
		if (applicationIntf != NULL)
			applicationIntf->RegisterVTaskForRemoteUID(currentTask, receivedClientID, fContext->GetID());
		
		bool postpone = false;

		TaskState state = currentTask->GetState();
		while (state != TS_DYING && state != TS_DEAD && err == VE_OK && !fShouldStop && !postpone) 
		{
			sWORD header[3];
			err = fEndPoint->ReadExactly(&header[0], 6);
			//xbox_assert ( err == VE_OK && err != VE_SRVR_CONNECTION_BROKEN );
			if (err == VE_OK)
			{
				sWORD reqid = header[0];
				sLONG len = *((sLONG*)&header[1]);
				if (fClientNeedSwap)
				{
					ByteSwap(&reqid);
					ByteSwap(&len);
				}

				if (reqid == req_PostPone)
				{
					postpone = true;
					VString sID;
					err = fEndPoint->ReadExactly(sID.GetCPointerForWrite(len), len*2);
					sID.Validate(len);
					if (err == VE_OK)
						VDBMgr::GetManager()->KeepPostPonedContext(sID, fContext);
					if (err == VE_OK)
					{
						char c = 1;
						fEndPoint->WriteExactly(&c,1);
					}
				}
				else
				{
					if (testAssert(reqid >= kRangeReqDB4D && reqid < kMaxRangeReqDB4D))
					{
						if (testAssert(len >= 0))
						{
							Boolean DataWasAllocated = false;
							void* data = &quickbuffer[0];
							if (len > quibufferlen)
							{
								DataWasAllocated = true;
								data = VObject::GetMainMemMgr()->Malloc(len, false, 'req ');
							}
							if (data == nil)
							{
								// a faire
								// pouvoir demander au endpoint de skipper len bytes
								TCPRequestReply reply(fEndPoint, reqid, nil, 0, fClientNeedSwap);
								reply.InitReply(-2);
							}
							else
							{
								if (len > 0)
									err = fEndPoint->ReadExactly(data, len);
								if (err == VE_OK)
								{
									
#if debugServer_Streams
									if (debugServer_Streams_state)
									{
										VStr<512> debugmess(L"Received ");
										debugmess += ToString(len);
										debugmess += L" bytes  ,  query = ";
										debugmess += ToString((sLONG)reqid);
										debugmess += L" ,  TaskID = ";
										debugmess += ToString(VTask::GetCurrentID());
										debugmess += L"\n";

										debugmess.EnsureSize(debugmess.GetLength()+(len*3)+(len/32)+10);
										for (sLONG i = 0; i < len; i++)
										{
											if ((i & 31) == 31)
												debugmess += "\n";
											VStr<10> s;
											uCHAR c1,c2;
											c1 = ((uCHAR*)data)[i];
											c2 = c1 & 15;
											c1 = c1 / 16;
											if (c1<10)
												c1 += 48;
											else
												c1 += 55;
											if (c2<10)
												c2 += 48;
											else
												c2 += 55;
											s.AppendUniChar(c1);
											s.AppendUniChar(c2);
											s.AppendUniChar(' ');
											debugmess += s;
										}
										debugmess += L"\n\n";
										DebugMsg(debugmess);
									}
#endif
									TCPRequestReply reply(fEndPoint, reqid, data, len, fClientNeedSwap);
									VDBMgr::GetManager()->ExecuteRequest(reqid, &reply, fContext);
								}
								
							}
							if (DataWasAllocated && data != nil)
								VObject::GetMainMemMgr()->Free(data);
						}
					}
				}
			}
			VTask::FlushErrors();
			state = VTask::GetCurrent()->GetState();
		}
		
		if(applicationIntf != NULL)
		{
			applicationIntf->UnregisterVTaskForRemoteUID( currentTask, receivedClientID);
		}
	}

	currentTask->SetProperties( nil);
	// At this point, the workerpool will change the TaskKind, TaskKindData and taskname

	return VConnectionHandler::eWS_DONE;
}


void DB4DConnectionHandler::AddNewConnection(DB4DConnectionHandler* connection)
{
	VTaskLock lock(&sAllConnectionsMutex);
	sAllConnections.insert(connection);
}


void DB4DConnectionHandler::CloseConnection(CDB4DContext* inContext)
{
	{
		VDBMgr::GetManager()->ForgetPostPonedContext(inContext);
		VTaskLock lock(&sAllConnectionsMutex);

		for (DB4DConnectionHandlerCollection::iterator cur = sAllConnections.begin(), end = sAllConnections.end(); cur != end; cur++)
		{
			if ((*cur)->GetContext() == inContext)
				(*cur)->ForceClose();
		}
	}

}


void DB4DConnectionHandler::CloseAllConnections()
{
	bool needSleep = false;

	{
		VDBMgr::GetManager()->ForgetAllPostPonedContext();
		VTaskLock lock(&sAllConnectionsMutex);
		sAcceptNewConnection = false;

		for (DB4DConnectionHandlerCollection::iterator cur = sAllConnections.begin(), end = sAllConnections.end(); cur != end; cur++)
		{
			(*cur)->ForceClose();
			needSleep = true;
		}
	}
	
	if (needSleep)
		VTaskMgr::Get()->GetCurrentTask()->Sleep(20);

	sLONG nbconnections;
	do 
	{
		{
			VTaskLock lock2(&sAllConnectionsMutex);
			nbconnections = (sLONG)sAllConnections.size();

			// L.E. TEMP: pour pouvoir a nouveau ouvrir des bases.
			if (nbconnections == 0)
				sAcceptNewConnection = true;
		}

		if (nbconnections > 0)
			VTaskMgr::Get()->GetCurrentTask()->Sleep(100);

	} while(nbconnections > 0);

}



					// ------------------------------------------------------



VError TCPOutputStream::DoOpenWriting()
{
	return VE_OK;
}


VError TCPOutputStream::DoOpenReading()
{
	return vThrowError( VE_STREAM_CANNOT_READ);
}


VError TCPOutputStream::DoPutData( const void* inBuffer, VSize inNbBytes)
{
	bool ok = fBuffer.PutDataAmortized( (VSize) (GetPos() - fSentBytes), inBuffer, inNbBytes);

	if (ok)
		return VE_OK;
	else
		return ThrowBaseError(memfull, DBaction_SendingData);
}


VError TCPOutputStream::DoGetData(void* inBuffer, VSize* ioCount)
{
	return vThrowError( VE_STREAM_CANNOT_READ);
}


VError TCPOutputStream::DoFlush()
{
	VError err=VE_OK;

	assert(fBuffer.GetDataSize() >= 6);
	if (fBuffer.GetDataSize() > 0)
	{
		char* p = (char*)fBuffer.GetDataPtr();
		if (testAssert(p != nil))
		{
			p = p + 2;
			*((sLONG*)p) = (sLONG)fBuffer.GetDataSize()-6;
			if (NeedSwap())
				ByteSwap((sLONG*)p);
		}
		err = fEndPoint->WriteExactly(fBuffer.GetDataPtr(), (uLONG)fBuffer.GetDataSize());

		/*
		uLONG len = (uLONG)fBuffer.GetDataSize();
		uLONG totlen = 0;
		char* xp = (char*)fBuffer.GetDataPtr();
		do
		{
			uLONG curlen = len - totlen;
			err = fEndPoint->Write(xp, &curlen, false);
			if (err == VE_OK || err == VE_SRVR_RESOURCE_TEMPORARILY_UNAVAILABLE)
			{
				xp = xp + curlen;
				totlen = totlen + curlen;
			}
			VTask::Yield();
		} while (err == VE_SRVR_RESOURCE_TEMPORARILY_UNAVAILABLE || totlen < len);
		*/

		if (err == VE_OK)
		{
#if debugServer_Streams
			if (debugServer_Streams_state)
			{
				VStr<512> debugmess(L"Sent ");
				debugmess += ToString((sLONG)len);
				debugmess += L" ,  TaskID = ";
				debugmess += ToString(VTask::GetCurrentID());
				debugmess += L"\n";

				uCHAR* cp = (uCHAR*)fBuffer.GetDataPtr();
				debugmess.EnsureSize(debugmess.GetLength()+(len*3)+(len/32)+10);
				for (sLONG i = 0; i < (sLONG)len; i++)
				{
					if ((i & 31) == 31)
						debugmess += "\n";
					VStr<10> s;
					uCHAR c1,c2;
					c1 = cp[i];
					c2 = c1 & 15;
					c1 = c1 / 16;
					if (c1<10)
						c1 += 48;
					else
						c1 += 55;
					if (c2<10)
						c2 += 48;
					else
						c2 += 55;
					s.AppendUniChar(c1);
					s.AppendUniChar(c2);
					s.AppendUniChar(' ');
					debugmess += s;
				}
				debugmess += L"\n\n";
				DebugMsg(debugmess);
			}
#endif

			fSentBytes += fBuffer.GetDataSize();
			fBuffer.Clear();
		}
		else
		{
			sLONG tobreak = 1; // put a break here
			//assert(false);
		}
	}

	if (err != VE_OK)
		err = ThrowBaseError(VE_DB4D_CANNOT_SEND_DATA, DBaction_SendingData);

	return err;
}


VError TCPOutputStream::DoCloseWriting( Boolean /*inSetSize*/)
{
	VError err = VE_OK;
	if (GetLastError()!= VE_OK)
	{
		sWORD buffer[3] = { -2, 0, 0 };
		uLONG len = 6;
		err = fEndPoint->WriteExactly(&buffer[0], len);
	}
	else
	{
		if (!fBuffer.IsEmpty())
		{
			err = DoFlush();
		}
	}

	if (err != VE_OK)
		err = ThrowBaseError(VE_DB4D_CANNOT_SEND_DATA, DBaction_SendingData);

	return err;
}


VError TCPOutputStream::DoSetPos( sLONG8 inNewPos)
{
	return vThrowError( VE_STREAM_CANNOT_SET_POS);
}


sLONG8 TCPOutputStream::DoGetSize()
{
	return fSentBytes + fBuffer.GetDataSize();
}


VError TCPOutputStream::DoSetSize(sLONG8 inNewSize)
{
	return vThrowError( VE_STREAM_CANNOT_SET_SIZE);
}



					// ------------------------------------------------------



TCPRequestReply::TCPRequestReply( VTCPEndPoint* inEndPoint, sWORD inRequestID, const void *inRequestData, size_t inRequestDataSize, Boolean inSwapForClient)
: fInputStream( inRequestData, inRequestDataSize)
, fOutputStream( inEndPoint)
, fRequestID( inRequestID)
{
	fInputStream.SetNeedSwap( inSwapForClient);
	fInputStream.OpenReading();

	fOutputStream.SetNeedSwap( inSwapForClient);
	fOutputStream.OpenWriting();
}

TCPRequestReply::~TCPRequestReply()
{
	fInputStream.CloseReading();
	fOutputStream.CloseWriting();	// final flush
}


const void* TCPRequestReply::GetRemainingInputDataPtr()
{
	const void *data = fInputStream.GetDataPtr();
	return (const char*) data + fInputStream.GetPos();
}


VSize TCPRequestReply::GetRemainingInputDataSize()
{
	return fInputStream.GetDataSize() - (VSize) fInputStream.GetPos();
}



void TCPRequestReply::InitReply( XBOX::RequestStatusCode inErrorCode)
{
	fOutputStream.PutWord(inErrorCode);
	fOutputStream.PutLong(0);
}



					// ------------------------------------------------------



TCPRequest::TCPRequest( VTCPEndPoint* inEndPoint, Boolean inClientNeedSwap, sWORD inRequestID, CDB4DBaseContext *inContext)
: fOutputStream( inEndPoint)
		, fEndPoint( inEndPoint)
		, fClientNeedSwap( inClientNeedSwap)
		, fSent( false)
		, fSendError( VE_OK)
{
	fRequestID = inRequestID;
	fContext = inContext;
	fInputStream.SetNeedSwap( inClientNeedSwap);
	fOutputStream.SetNeedSwap( inClientNeedSwap);
	fOutputStream.OpenWriting();
	fOutputStream.PutWord(inRequestID);
	fOutputStream.PutLong(0);
}


TCPRequest::~TCPRequest()
{
}


VError TCPRequest::Send()
{
	// protect against reentrancy
	VTaskMgr::Get()->SetDirectUpdate( false);

	IRequestLogger* reqlog = VDBMgr::GetManager()->GetRequestLogger();
	uLONG startTime;
	Boolean oklog = false;
	sLONG RequestNbBytes;
	sLONG receivedNbBytes = 0;
	if (reqlog != nil && reqlog->IsEnable())
	{
		oklog = true;
		startTime = VSystem::GetCurrentTime();
		RequestNbBytes = fOutputStream.GetSize();
	}

	sWORD action = 0;
	Boolean oksent = false;
	Boolean okreply = false;
	VError err = fEndPoint->Use();
	if (err == VE_OK)
		err = fOutputStream.CloseWriting();
	else
		fOutputStream.CloseWriting();
	if (err == VE_OK)
	{
		oksent = true;
		sWORD buffer[3];
		err = fEndPoint->ReadExactly(&buffer[0], 6);
		if (err == VE_OK)
		{
			action = buffer[0];
			sLONG len = *((sLONG*)&buffer[1]);
			if (fClientNeedSwap)
			{
				ByteSwap(&action);
				ByteSwap(&len);
			}
			receivedNbBytes = len + 6;
			assert(len >= 0);
			if (len > 0)
			{
				void* p = VObject::GetMainMemMgr()->Malloc(len, false, 'req ');
				if (p == nil)
				{
					err = ThrowBaseError(memfull, DBaction_ReceivingData);
					// a faire
					// pouvoir demander au endpoint de skipper len bytes
				}
				else
				{
					err = fEndPoint->ReadExactly(p, len);
					if (err == VE_OK)
					{
						okreply = true;
						fInputStream.SetDataPtr(p, len);
						fInputStream.OpenReading();
						
						// check error codes
						if ( (action == -10004) || (action == -10005) )
						{
							fInputStream.Get( &err);
							
							if (action == -10005)
							{
								// read error context
								VValueBag bag;
								bag.ReadFromStream( &fInputStream);

								XBOX::VErrorTaskContext *taskContext = XBOX::VTask::GetCurrent()->GetErrorContext( true);
								if (taskContext != NULL)
								{
									taskContext->PushErrorsFromBag( bag);
								}
							}
						}
					}
				}
			}
			else
				okreply = true;
		}
	}

	if (err == VE_OK)
		err = fEndPoint->UnUse();
	else
		fEndPoint->UnUse();

	if (err == VE_OK && action < 0)
	{
		err = MAKE_VERROR( '4DRT', action);
	}

	if (!oksent)
		err = ThrowBaseError(VE_DB4D_CANNOT_SEND_REQUEST, DBaction_SendingRequest);
	else
	{
		if (!okreply)
		{
			err = ThrowBaseError(VE_DB4D_CANNOT_RECEIVE_DATA, DBaction_SendingRequest);
		}
	}

	if (reqlog!= nil && oklog)
	{
		reqlog->Log(CDB4DManager::Component_Type, fContext, fRequestID, RequestNbBytes, receivedNbBytes, VSystem::GetCurrentTime()-startTime);
	}

	fSendError = err;

	VTaskMgr::Get()->SetDirectUpdate( true);

	return err;
}



					// ------------------------------------------------------


VError DB4DClientSession::Postpone ( VTCPEndPoint& inEndPoint )
{
	VError err = VE_OK;
	sWORD header[3];
	header[0] = req_PostPone;
	VString sID;
	inEndPoint.GetSessionID(sID);
	sLONG len = sID.GetLength();
	*((sLONG*)&header[1]) = len;
	if ( fNetManager-> ClientNeedsSwap ( ) )
	{
		ByteSwap ( ( sWORD* ) ( &header[0] ) );
		ByteSwap ( ( sLONG* ) ( &header[1] ) );
	}
	err = inEndPoint.WriteExactly(&header[0], 6);
	if (err == VE_OK)
		err = inEndPoint.WriteExactly((void*)sID.GetCPointer(), len*2);

	char c;
	/*if (err == VE_OK)
	{
		uLONG		timeOut = 0;
		if (!inEndPoint.IsBlocking())
		{
			timeOut = 5000;
			if (inEndPoint.IsSelectIO())
				inEndPoint.SetIsSelectIO(false);
		}
		
		err = inEndPoint.ReadExactly(&c, 1, timeOut);
	}*/
	if (err == VE_OK)
		err = inEndPoint.ReadExactly(&c, 1, 20*1000);

	return err;
}


VTCPEndPoint* DB4DClientSession::Restore ( VTCPEndPoint& inEndPoint, VError& outError )
{
	VString ipString;
	inEndPoint.GetIP(ipString);

	VError err = VE_OK;
	VTCPEndPoint* newEndPoint = VTCPEndPointFactory::CreateClientConnection(ipString, inEndPoint.GetPort(), inEndPoint.IsSSL(), inEndPoint.IsBlocking(), 20 * 1000, inEndPoint.GetSelectIOPool(), err);
	if (err == VE_OK && newEndPoint != nil)
	{
		if (_WithSelectIo())
			newEndPoint->SetIsSelectIO(true);
		sLONG tag = kTagReOpenConnection;
		uLONG len = 4;
		err = newEndPoint->WriteExactly(&tag, len);
		if (err == VE_OK)
		{
			VString sID;
			inEndPoint.GetSessionID(sID);

			sLONG len = sID.GetLength();
			err = newEndPoint->WriteExactly((void*)&len, 4);
			if (err == VE_OK)
				err = newEndPoint->WriteExactly((void*)sID.GetCPointer(), len*2);
		}							

		if (err == VE_OK)
		{
			sLONG tagretour = 0;
			err = newEndPoint->ReadExactly(&tagretour, 4, 20*1000);
			if (err == VE_OK)
			{
				if (tagretour != kTagRetourOpenConnection)
				{
					ByteSwap(&tagretour);
					if (tagretour != kTagRetourOpenConnection)
						err = VE_DB4D_INVALID_PARAMETER;
				}
			}
		}

		if (err == VE_OK)
		{
			newEndPoint->EnableAutoReconnect();
			newEndPoint->SetClientSession(this);
			newEndPoint->SetPostponeTimeout(0);
			newEndPoint->SetIdleTimeout(inEndPoint.GetIdleTimeout());
		}
	}

	outError = err;
	return newEndPoint;
}



					// ------------------------------------------------------


DB4DNetManager::~DB4DNetManager()
{
	CloseConnection();
	ReleaseRefCountable( &fEndPoint);
	delete fClientSession;
}


VError DB4DNetManager::CloseConnection() 
{ 
	fEndPoint->Close();
	return VE_OK; 
}


VError DB4DNetManager::OpenConnection()
{
	//With new servernet, each XSocket is connected on creation
	VError err=VE_OK;
	
	if (err == VE_OK)
	{
		if (_WithSelectIo())
			fEndPoint->SetIsSelectIO(true);
		sLONG tag = kTagOpenConnection;
		uLONG len = 4;
		err = fEndPoint->WriteExactly(&tag, len);
		if (err == VE_OK)
		{
			VUUID newid(true);
			const VUUIDBuffer* xid;
			xid = &(newid.GetBuffer());
			uLONG xlen = sizeof(VUUIDBuffer);
			err = fEndPoint->WriteExactly((void*)xid, xlen);
			xid = &(VUUID::sNullUUID.GetBuffer());
			xlen = sizeof(VUUIDBuffer);
			err = fEndPoint->WriteExactly((void*)xid, xlen);

			// no context extra data
			uLONG extraSize = 0;
			err = fEndPoint->WriteExactly( &extraSize, sizeof( extraSize));
		}

		if (err == VE_OK)
		{
			sLONG tagretour = 0;
			err = fEndPoint->ReadExactly(&tagretour, 4);
			if (err == VE_OK)
			{
				if (tagretour != kTagRetourOpenConnection)
				{
					fClientNeedSwap = true;
					ByteSwap(&tagretour);
					if (tagretour != kTagRetourOpenConnection)
						err = VE_DB4D_INVALID_PARAMETER;
				}
			}
		}

		if (err == VE_OK)
		{
			fEndPoint->EnableAutoReconnect();
			fEndPoint->SetClientSession(fClientSession);
			fEndPoint->SetPostponeTimeout(0);
			fEndPoint->SetIdleTimeout(ServerNetTools::GetDefaultClientIdleTimeOut());
		}
	}

	return err;
}


IStreamRequest*	DB4DNetManager::CreateRequest( CDB4DBaseContext *inContext, sWORD inRequestID)
{
	BaseTaskInfo* context = ConvertContext(inContext);
	if (context != nil && context->IsLocal())
	{
		return context->GetBase()->CreateRequest(inContext, inRequestID, true);
	}
	else
	{
		VTCPEndPoint* endpoint = nil;
		TCPRequest* req = nil;

		if (sAcceptNewConnection)
		{
			if (inContext == nil)
			{
				endpoint = fEndPoint;
			}
			else
			{
				char servername[8192];

				{
					VTaskLock lock(&fMutex);

					EndPointsCollection::iterator found = fEndPointsByContext.find(inContext);
					endpoint = (found == fEndPointsByContext.end()) ? nil : found->second;
					fServerName.ToCString(servername, 8192);
				}

				if (endpoint == nil)
				{
					XTCPSock* conn=XTCPSock::NewClientConnectedSock(fServerName, fPortNum, 30000 /*MsTimeout*/);

					if(conn!=NULL && fSSL)
					{
						VError verr=conn->PromoteToSSL();

						xbox_assert(verr==VE_OK);

						if(verr!=VE_OK)
						{
							delete conn;
							conn=NULL;
						}
					}

					if (conn != nil)
					{
						VTCPSelectIOPool* vIOPool = nil;
						if (_WithSelectIo())
						{
							IDB4D_ApplicationIntf *applicationIntf = VDBMgr::GetManager()->GetApplicationInterface();
							if (applicationIntf != NULL)	// may happen during unit testing
							{
								vIOPool = applicationIntf->GetSharedSelectIOPool();
							}
							endpoint = new VTCPEndPoint(conn, vIOPool);
						}
						else
							endpoint = new VTCPEndPoint(conn);

						if (endpoint == nil)
						{
							delete conn;
							conn = nil;
						}
						else
						{
							endpoint->EnableCriticalErrorSignal(); /* Sergiy - 14 May 2008 - Enable signaling of critical network errors back to 4D. */

							//With new servernet, each XSocket is connected on creation
							VError err=VE_OK;

							if (err == VE_OK)
							{
								if (_WithSelectIo())
									endpoint->SetIsSelectIO(vIOPool != nil);

								sLONG tag = kTagOpenConnection;
								uLONG len = 4;
								err = endpoint->WriteExactly(&tag, len);
								CDB4DContext* parent = nil;
								if (inContext != nil)
									parent = inContext->GetContextOwner();

								if (err == VE_OK)
								{
									VUUID newid(true);
									const VUUIDBuffer* xid;
									if (parent == nil)
									{
										xid = &(newid.GetBuffer());
									}
									else
									{
										xid = &(parent->GetID().GetBuffer());
									}
									uLONG xlen = sizeof(VUUIDBuffer);
									err = endpoint->WriteExactly((void*)xid, xlen);
									
									xlen = sizeof(VUUIDBuffer);
									xid = &(context->GetBase()->GetClientID().GetBuffer());
									err = endpoint->WriteExactly((void*)xid, xlen);
									
									// write context extra data if any
									const VValueBag *extra = (parent == nil) ? nil : VImpCreator<VDB4DContext>::GetImpObject(parent)->GetExtraData();
									if (extra != nil)
									{
										VPtrStream stream;
										stream.OpenWriting();
										extra->WriteToStream( &stream);
										if (stream.CloseWriting() == VE_OK)
										{
											uLONG extraSize = (uLONG) stream.GetDataSize();
											err = endpoint->WriteExactly( &extraSize, sizeof( extraSize));
											err = endpoint->WriteExactly( stream.GetDataPtr(), extraSize);
										}
									}
									else
									{
										uLONG extraSize = 0;
										err = endpoint->WriteExactly( &extraSize, sizeof( extraSize));
									}
								}

								if (err == VE_OK)
								{
									sLONG tagretour = 0;
									err = endpoint->ReadExactly(&tagretour, 4);
									if (err == VE_OK)
									{
										if (tagretour != kTagRetourOpenConnection)
										{
											//fClientNeedSwap = true;
											ByteSwap(&tagretour);
											if (tagretour != kTagRetourOpenConnection)
												err = VE_DB4D_INVALID_PARAMETER;
										}
									}
								}

								if (err == VE_OK)
								{
									endpoint->EnableAutoReconnect();
									endpoint->SetClientSession(fClientSession);
									endpoint->SetPostponeTimeout(0);
									endpoint->SetIdleTimeout(ServerNetTools::GetDefaultClientIdleTimeOut());
								}
							}

							if (err == VE_OK)
							{
								{
									VTaskLock lock(&fMutex);
									fEndPointsByContext.insert(make_pair(inContext, endpoint));
									VImpCreator<BaseTaskInfo>::GetImpObject(inContext)->SetRemoteConnection(this);
								}
								{
									VTaskLock lock(&sMutex);
									sEndPointsByContext.insert(make_pair(inContext, endpoint));
								}
							}
							else
							{
								ReleaseRefCountable( &endpoint);
							}
						}
					}

				}
			}
		}

		if (endpoint != nil)
		{
			req = new TCPRequest(endpoint, fClientNeedSwap, inRequestID, inContext);
		}
		return req;
	}
}

EndPointsCollection DB4DNetManager::sEndPointsByContext;
VCriticalSection DB4DNetManager::sMutex;
bool DB4DNetManager::sAcceptNewConnection = true;

void DB4DNetManager::CloseConnection(CDB4DBaseContext* inContext)
{
	{
		VTaskLock lock(&sMutex);
		EndPointsCollection::iterator found = sEndPointsByContext.find(inContext);
		if (found != sEndPointsByContext.end())
		{
			found->second->SetIsBlocking(false);
			found->second->ForceClose();
		}
	}

}

void DB4DNetManager::CloseAllConnections()
{
	bool needSleep = false;
	
	{
		VTaskLock lock(&sMutex);

		for (EndPointsCollection::iterator cur = sEndPointsByContext.begin(), end = sEndPointsByContext.end(); cur != end; cur++)
		{
			cur->second->SetIsBlocking(false);
			cur->second->ForceClose();
			needSleep = true;
		}
	}

	if (needSleep)
		VTaskMgr::Get()->GetCurrentTask()->Sleep(20);

	sAcceptNewConnection = false;
	sLONG nbconnections;
	do 
	{
		{
			VTaskLock lock2(&sMutex);
			nbconnections = (sLONG)sEndPointsByContext.size();

			// L.E. TEMP: pour pouvoir a nouveau ouvrir des bases.
			if (nbconnections == 0)
				sAcceptNewConnection = true;
		}

		if (nbconnections > 0)
			VTaskMgr::Get()->GetCurrentTask()->Sleep(100);

	} while(nbconnections > 0);

}


void DB4DNetManager::RemoveContext(CDB4DBaseContext *inContext)
{
	{
		VTaskLock lock(&sMutex);
		sEndPointsByContext.erase(inContext);
	}
	{
		VTaskLock lock(&fMutex);

		EndPointsCollection::iterator found = fEndPointsByContext.find(inContext);
		if (found != fEndPointsByContext.end())
		{
			VTCPEndPoint* endpoint = found->second;
			endpoint->Close();
			ReleaseRefCountable( &endpoint);
			fEndPointsByContext.erase( found);
		}
	}
}

VError DB4DNetManager::SetContextIdleTimeOut(CDB4DBaseContext *inContext, uLONG inMilliseconds)
{
	VError err = VE_OK;
	VTaskLock lock(&fMutex);

	EndPointsCollection::iterator found = fEndPointsByContext.find(inContext);
	if (found != fEndPointsByContext.end())
	{
		VTCPEndPoint* endpoint = found->second;
		endpoint->SetIdleTimeout(inMilliseconds);
	}
	return err;
}


VError DB4DNetManager::SetIdleTimeOut(uLONG inMilliseconds)
{
	VError err = VE_OK;
	VTaskLock lock(&fMutex);

	if (fEndPoint != nil)
		fEndPoint->SetIdleTimeout(inMilliseconds);
	return err;
}



DB4DNetManager* DB4DNetManager::NewConnection(const VString& inServerName, sWORD inPortNum, VError& outerr, bool inSSL)
{
	DB4DNetManager* result = nil;
	char buffer[8192];
	char* servername = &buffer[0];

	if (inPortNum == 0)
		inPortNum = DB4D_Default_Server_Port;
	inServerName.ToCString(servername, 8192);

	XTCPSock* conn=XTCPSock::NewClientConnectedSock(inServerName, inPortNum, 30000 /*MsTimeout*/);

	if(conn!=NULL && inSSL)
	{
		VError verr=conn->PromoteToSSL();
		
		xbox_assert(verr==VE_OK);
		
		if(verr!=VE_OK)
		{
			delete conn;
			conn=NULL;
		}
	}
	
	if (conn == nil)
	{
		outerr = -1;
	}
	else
	{
		VTCPEndPoint* endpoint = nil;

		if (_WithSelectIo())
		{
			VTCPSelectIOPool* vIOPool = nil;
			IDB4D_ApplicationIntf *applicationIntf = VDBMgr::GetManager()->GetApplicationInterface();
			if (applicationIntf != NULL)	// may happen during unit testing
			{
				vIOPool = applicationIntf->GetSharedSelectIOPool();
			}
			endpoint = new VTCPEndPoint(conn, vIOPool);
		}
		else
			endpoint = new VTCPEndPoint(conn);

		if (endpoint == nil)
			outerr = ThrowBaseError(memfull, DBaction_ServerConnecting);
		else
		{
			endpoint->EnableCriticalErrorSignal(); /* Sergiy - 14 May 2008 - Enable signaling of critical network errors back to 4D. */
			conn = nil;
			result = new DB4DNetManager(endpoint, inServerName, inPortNum, inSSL);
			if (result == nil)
			{
				ReleaseRefCountable( &endpoint);
				outerr = ThrowBaseError(memfull, DBaction_ServerConnecting);
			}
			else
			{
				outerr = result->OpenConnection();
				if (outerr != VE_OK)
				{
					result->Release();
					result = nil;
				}
			}
		}

		if (conn != nil)
			delete conn;
	}

	if (outerr != VE_OK)
		outerr = outerr = ThrowBaseError(VE_DB4D_CANNOT_CREATE_CONNECTION, DBaction_ServerConnecting);
	return result;
}


void SendError(IRequestReply *inRequest, CDB4DBaseContext *inContext, VError errToSend)
{
	if (errToSend == VE_OK)
	{
		errToSend = VTask::GetCurrent()->GetLastError();
	}
	if (errToSend == VE_OK)
		errToSend = -1;
	inRequest->InitReply(-10005);
	VStream* reqsend = inRequest->GetOutputStream();
	reqsend->PutLong8(errToSend);
	
	// send error context
	XBOX::VErrorTaskContext *context = XBOX::VTask::GetCurrent()->GetErrorContext( false);
	if (context)
	{
		XBOX::VErrorContext combinedContext;
		context->GetErrors( combinedContext);
		
		VValueBag bag;
		combinedContext.SaveToBag( bag);
		
		bag.WriteToStream( reqsend);
	}
}




// ------------------------------------------------------------------------------------------------



FieldsForCacheOnOneTable* FieldsForCacheOnOneTable::Clone() const
{
	FieldsForCacheOnOneTable* lacopy = new FieldsForCacheOnOneTable(fOwner);
	if (lacopy != nil)
	{
		lacopy->fFields.CopyFrom(fFields);
	}
	return lacopy;
}


VError FieldsForCacheOnOneTable::FillStreamWithData(FicheInMem* rec, VStream* toStream, BaseTaskInfo* context)
{
	sLONG nb = rec->NBCrit();
	VError err = toStream->PutLong(rec->GetNum());
	for (sLONG i = 1; i <= nb && err == VE_OK; i++)
	{
		if (fFields.isOn(i))
		{
			ValPtr cv = rec->GetNthField(i, err);
			if (cv != nil)
			{
				err = toStream->PutLong(i);
				if (err == VE_OK)
					err = toStream->PutLong((sLONG)cv->GetValueKind());
				if (err == VE_OK)
					err = cv->WriteToStream(toStream);
			}
		}
	}

	if (err == VE_OK)
		toStream->PutLong(0);

	return err;
}


VError FieldsForCacheOnOneTable::ReadDataFromStream(FicheInMem* rec, VStream* fromStream, BaseTaskInfo* context)
{
	VError err = VE_OK;
	sLONG numfield;

	do 
	{
		err = fromStream->GetLong(numfield);
		if ( (err == VE_OK) && (numfield != 0) )
		{
			sLONG kind;
			err = fromStream->GetLong(kind);
			if (err == VE_OK)
			{
				ValPtr cv = (ValPtr)VValue::NewValueFromValueKind((ValueKind)kind);
				if (cv == nil)
					err = ThrowBaseError(memfull);
				else
				{
					err = cv->ReadFromStream( fromStream);
					if (err == VE_OK)
						err = rec->SetNthFieldOld(numfield, cv,false);
					if (err != VE_OK)
						delete cv;
				}

			}
		}
	} while(numfield != 0 && err == VE_OK);

	return err;
}


// -----------------------------------------------------------------


FieldsForCacheOnOneTable* FieldsForCache::RetainFieldsForTable(Table* inTable)
{
	assert(inTable != nil);
	FieldsForCacheOnOneTable* Deja = GetFieldsForTable(inTable);
	if (Deja == nil)
	{
		Deja = new FieldsForCacheOnOneTable(this);
		if (Deja == nil)
			ThrowBaseError(memfull, noaction);
		else
		{
			try
			{
				fAllFields[inTable->GetNum()].Adopt(Deja);
			}
			catch (...)
			{
				ThrowBaseError(memfull, noaction);
				ReleaseRefCountable(&Deja);
			}
		}

	}

	if (Deja != nil)
		Deja->Retain();
	return Deja;
	/*
	FieldsForCacheOnOneTable* result = nil;

	FieldsForCacheMap::iterator found = fAllFields.find(inTable->GetNum());
	if (found != fAllFields.end())
	{
		result = found->second.Retain();
	}
	return result;
	*/
}


FieldsForCacheOnOneTable* FieldsForCache::GetFieldsForTable(Table* inTable)
{
	assert(inTable != nil);
	FieldsForCacheOnOneTable* result = nil;

	FieldsForCacheMap::iterator found = fAllFields.find(inTable->GetNum());
	if (found != fAllFields.end())
	{
		result = found->second.Get();
	}
	return result;
}


void FieldsForCache::AddSel(sLONG TableNum)
{
	FieldsForCacheOnOneTable* Deja = nil;

	FieldsForCacheMap::iterator found = fAllFields.find(TableNum);
	if (found != fAllFields.end())
	{
		Deja = found->second.Get();
	}

	if (Deja == nil)
	{
		Deja = new FieldsForCacheOnOneTable(this);
		if (Deja != nil)
		{
			try
			{
				fAllFields[TableNum].Adopt(Deja);
			}
			catch (...)
			{
				ReleaseRefCountable(&Deja);
			}
		}

	}

	if (Deja != nil)
	{
		Deja->AddField((sLONG)0);
	}

}


VError FieldsForCache::AddField(Field* inField)
{
	VError err = VE_OK;
	assert(inField != nil);
	Table* tt = inField->GetOwner();

	FieldsForCacheOnOneTable* Deja = GetFieldsForTable(tt);
	if (Deja == nil)
	{
		Deja = new FieldsForCacheOnOneTable(this);
		if (Deja == nil)
			err = ThrowBaseError(memfull, noaction);
		else
		{
			try
			{
				fAllFields[tt->GetNum()].Adopt(Deja);
			}
			catch (...)
			{
				err = ThrowBaseError(memfull, noaction);
				ReleaseRefCountable(&Deja);
			}
		}

	}

	if (Deja != nil)
	{
		Deja->AddField(inField);
	}

	Touch();

	return err;
}


VError FieldsForCache::WriteToStream(VStream* ToStream)
{
	sLONG nb = (sLONG)fAllFields.size();
	VError err = ToStream->PutLong(nb);
	for (FieldsForCacheMap::iterator cur = fAllFields.begin(), end = fAllFields.end(); cur != end && err == VE_OK; cur++)
	{
		err = ToStream->PutLong(cur->first);
		if (err == VE_OK)
		{
			if (cur->second == nil)
				err = ToStream->PutByte('-');
			else
			{
				err = ToStream->PutByte('+');
				err = cur->second->WriteToStream(ToStream);
			}
		}
	}
	return err;
}


VError FieldsForCache::ReadFromStream(VStream* FromStream)
{
	sLONG nb;
	VError err = FromStream->GetLong(nb);
	for (sLONG i = 0; i < nb && err == VE_OK; i++)
	{
		sLONG tablenum;
		err = FromStream->GetLong(tablenum);
		if (err == VE_OK)
		{
			FieldsForCacheOnOneTable* ffc = new FieldsForCacheOnOneTable(this);
			if (ffc == nil)
				err = ThrowBaseError(memfull, noaction);
			else
			{
				uBYTE cc;
				err = FromStream->GetByte(cc);
				if (err == VE_OK)
				{
					if (cc == '+')
					{
						err = ffc->ReadFromStream(FromStream);
						if (err == VE_OK)
						{
							try
							{
								fAllFields[tablenum].Adopt(ffc);
							}
							catch (...)
							{
								err = ThrowBaseError(memfull, noaction);
							}
						}
					}
				}
			}

			if (err != VE_OK)
				ReleaseRefCountable(&ffc);
		}
	}

	return err;
}


void FieldsForCache::GetTableNums(vector<sLONG>& outNums)
{
	for (FieldsForCacheMap::iterator cur = fAllFields.begin(), end = fAllFields.end(); cur != end; cur++)
	{
		outNums.push_back(cur->first);
	}
}


VError FieldsForCache::BuildRelationsOnServer(BaseTaskInfo* context, const vector<uBYTE>& inWayOfLocking)
{
	VError err = VE_OK;
	sLONG maintablenum = fMainTable->GetNum();
	for (FieldsForCacheMap::iterator cur = fAllFields.begin(), end = fAllFields.end(); cur != end; cur++)
	{
		FieldsForCacheOnOneTable* ffct = cur->second;
		sLONG tablenum = cur->first;
		if (tablenum != maintablenum)
		{
			RelationsCacheMap::iterator found = fRelPaths.find(tablenum);
			if (found == fRelPaths.end())
			{
				fRelPaths[tablenum].BuildPath(context, maintablenum, tablenum, err, true, true, inWayOfLocking);
			}
		}
	}
	return err;
}


VError FieldsForCache::ReadDataFromStream(RowCacheInfo* row, VStream* fromStream, BaseTaskInfo* context, RemoteRecordCache* displaycache)
{
	VError err = VE_OK;
	uBYTE cc;

	row->fMainRec = nil;

	sLONG tablenum;
	sLONG maintablenum = fMainTable->GetNum();
	Base4D* base = fMainTable->GetOwner();

	try
	{
		do
		{	
			err = fromStream->GetLong(tablenum);
			if (err == VE_OK && tablenum != 0)
			{
				err = fromStream->GetByte(cc);
				if (cc == '-')
				{

				}
				else
				{
					if (cc == '+')
					{
						sLONG recnum;
						err = fromStream->GetLong(recnum);
						if (err == VE_OK)
						{
							FicheInMem* rec = new FicheInMem();
							if (rec == nil)
								err = ThrowBaseError(memfull);
							else
							{
								Table* tt = base->RetainTable(tablenum);
								rec->PrepareForCacheDisplay(this, recnum, context, tt, displaycache);
								QuickReleaseRefCountable(tt);
								if (tablenum == maintablenum)
								{
									row->fMainRec = rec;
								}
								else
								{
									row->fRelatedRecords[tablenum].Adopt(rec);
								}
								err = fAllFields[tablenum]->ReadDataFromStream(rec, fromStream, context);
							}
						}
						
					}
					else
						err = ThrowBaseError(VE_DB4D_INVALID_PARAMETER);
				}
			}
		} while (err == VE_OK && tablenum != 0);
	}
	catch (...)
	{
		err = ThrowBaseError(memfull);
	}

	return err;
}



VError FieldsForCache::FillStreamWithData(FicheInMem* mainrec, VStream* toStream, BaseTaskInfo* context)
{
	VError err = VE_OK;
	Boolean dejamain = false;
	sLONG maintablenum = fMainTable->GetNum();
	for (FieldsForCacheMap::iterator cur = fAllFields.begin(), end = fAllFields.end(); cur != end && err == VE_OK; cur++)
	{
		FieldsForCacheOnOneTable* ffct = cur->second;
		sLONG tablenum = cur->first;
		FicheInMem* rec = nil;

		if (tablenum != maintablenum)
		{
			RelationPath* relpath = &fRelPaths[tablenum];
			if (!relpath->IsEmpty())
			{
				err = relpath->ActivateRelation(mainrec, rec, context, true);
			}
		}
		else
		{
			dejamain = true;
			rec = mainrec;
			rec->Retain();
		}

		if (err == VE_OK)
			err = toStream->PutLong(tablenum);

		if (err == VE_OK)
		{
			if (rec == nil)
				err = toStream->PutByte('-');
			else
			{
				err = toStream->PutByte('+');
				err = ffct->FillStreamWithData(rec, toStream, context);
			}
		}
		QuickReleaseRefCountable(rec);
	}

	if (!dejamain && err == VE_OK)
	{
		err = toStream->PutLong(maintablenum);
		if (err == VE_OK)
		{
			err = toStream->PutByte('+');
			if (err == VE_OK)
				err = toStream->PutLong(mainrec->GetNum());
			if (err == VE_OK)
				err = toStream->PutLong(0);
		}
	}

	if (err == VE_OK)
		err = toStream->PutLong(0);

	return err;
}




// -----------------------------------------------------------------



VError RemoteRecordCache::RetainCacheRecord(RecIDType inRecIndex, CDB4DRecord* &outRecord, vector<CachedRelatedRecord>& outRelatedRecords)
{
	VError err = VE_OK;
	Base4D* base = fFields->GetMainTable()->GetOwner();
	CDB4DBase* xbase = base->RetainBaseX();

	if (fStampCopy != fFields->GetStamp())
	{
		err = StartCachingRemoteRecords(fSel, fStartRecIndex, fEndRecIndex, fWayOfLocking);
		QuickReleaseRefCountable(fSel);
	}

	if (inRecIndex >= fStartRecIndex && inRecIndex <= fEndRecIndex && err == VE_OK)
	{
		fCurRecIndex = inRecIndex;
		RowCacheInfo* row = &(fRows[inRecIndex-fStartRecIndex]);
		if (row->fMainRec == nil)
			outRecord = nil;
		else
			outRecord = new VDB4DRecord(VDBMgr::GetManager(), RetainRefCountable(row->fMainRec), fContext->GetEncapsuleur());
		outRelatedRecords.resize(fEmptyRelatedInfos.size());
		sLONG i = 0;
		for (EmptyRelatedInfoMap::iterator cur = fEmptyRelatedInfos.begin(), end = fEmptyRelatedInfos.end(); cur != end && err == VE_OK; cur++, i++)
		{
			CDB4DRecord* xrec = nil;
			CDB4DSelection* xsel = nil;

			CachedRelatedRecord* p = &(outRelatedRecords[i]);
			sLONG tablenum = cur->first;
			RelatedRecordCacheInfoMap::iterator found = row->fRelatedRecords.find(tablenum);

			Table* tt = base->RetainTable(tablenum);
			Selection* sel = nil;
			if (tt != nil)
			{
				sel = new PetiteSel(base, fContext->GetEncapsuleur(), tablenum);
				xsel = new VDB4DSelection(VDBMgr::GetManager(), VImpCreator<VDB4DBase>::GetImpObject(xbase), tt, sel);
			}

			if (found == row->fRelatedRecords.end())
			{
				FicheInMem* rec = new FicheInMem();
				rec->PrepareForCacheDisplay(fFields, -1, fContext, tt, this);
				xrec = new VDB4DRecord(VDBMgr::GetManager(), rec, fContext->GetEncapsuleur());
				if (sel != nil)
				{
					sel->SetSelAsVirtualForCache(fFields);
				}
			}
			else
			{
				FicheInMem* rec = found->second.Retain();
				if (rec != nil)
				{
					if (sel != nil)
					{
						sel->FixFic(1);
						sel->PutFic(0, rec->GetNum());
					}
					xrec = new VDB4DRecord(VDBMgr::GetManager(), rec, fContext->GetEncapsuleur());
				}
			}

			QuickReleaseRefCountable(tt);

			p->AdoptRecord(xrec);
			p->AdoptSelection(xsel);
			p->SetTableNum(tablenum);
		}
	}
	else
		outRecord = nil;

	xbase->Release();

	return err;
}


VError RemoteRecordCache::_InitAllRelatedRecords(Table* onTable, vector<TempRelInfo>& outNewRels)
{
	onTable->occupe();
	DepRelationArrayIncluded* rels = onTable->GetRelNto1Deps();
	for (DepRelationArrayIncluded::Iterator cur = rels->First(), end = rels->End(); cur != end; cur++)
	{
		Relation* rel = *cur;
		if (rel != nil)
		{
			if (rel->IsAutoLoadNto1(fContext))
			{
				Table* dest = rel->GetDestTable();
				if (OKTable(dest->GetNum()) && ! fDejaScanRel.isOn(dest->GetNum()))
				{
					fDejaScanRel.Set(dest->GetNum());
					TempRelInfo temprel;
					temprel.fRel = RetainRefCountable(rel);
					temprel.fTable = dest;
					outNewRels.push_back(temprel);
				}
			}
		}
	}
	onTable->libere();

	return VE_OK;
}


VError RemoteRecordCache::InitAllRelatedRecordsOnClient()
{
	VError err = VE_OK;
	try
	{
		Table* MainTable = fFields->GetMainTable();

		fDejaScanRel.ClearAll();
		fDejaScanRel.Set(MainTable->GetNum());

		vector<TempRelInfo> goodrels;
		vector<TempRelInfo> temprels;

		_InitAllRelatedRecords(MainTable, temprels);

		while (! temprels.empty())
		{
			vector<TempRelInfo> temprels2;
			copy(temprels.begin(), temprels.end(), back_insert_iterator< vector<TempRelInfo> >(goodrels));
			for (vector<TempRelInfo>::iterator cur = temprels.begin(), end = temprels.end(); cur != end; cur++)
			{
				_InitAllRelatedRecords(cur->fTable, temprels2);
			}
			temprels.clear();
			temprels.swap(temprels2);
		}

		for (vector<TempRelInfo>::iterator cur = goodrels.begin(), end = goodrels.end(); cur != end; cur++)
		{
			Table* dest = cur->fTable;
			Relation* rel = cur->fRel;
			Table* source = rel->GetSourceTable();
			EmptyRelatedInfo eri;

			if (source != MainTable)
			{
				EmptyRelatedInfoMap::iterator deja = fEmptyRelatedInfos.find(source->GetNum());
				if (testAssert(deja != fEmptyRelatedInfos.end()))
				{
					eri.fRelPath = deja->second.fRelPath;
				}
			}

			eri.fRelPath.push_back(RelationInList(rel, true));

			rel->Release();

			fEmptyRelatedInfos[dest->GetNum()] = eri;
		}

		/*
		vector<sLONG> TableAlreadyInCacheFields;
		fFields->GetTableNums(TableAlreadyInCacheFields);
		for (vector<sLONG>::iterator cur = TableAlreadyInCacheFields.begin(), end = TableAlreadyInCacheFields.end(); cur != end; cur++)
		{
			fEmptyRelatedInfos.erase(*cur);
		}
		*/
	}
	catch (...)
	{
		err = ThrowBaseError(memfull, noaction);
	}

	return err;
}


FicheInMem* RemoteRecordCache::RetainTrueRelatedRecord(Table* targetTable, VError& err)
{
	FicheInMem* result = nil;
	err = VE_OK;

	EmptyRelatedInfoMap::iterator found = fEmptyRelatedInfos.find(targetTable->GetNum());
	if (found != fEmptyRelatedInfos.end())
	{
		FicheInMem* mainrec = nil;
		RowCacheInfo* row;
		if (fCurRecIndex != -1)
		{
			row = &(fRows[fCurRecIndex-fStartRecIndex]);
			mainrec = row->fMainRec;
		}

		if (mainrec != nil)
		{
			Base4D* base = targetTable->GetOwner();
			IRequest *req = base->CreateRequest( fContext->GetEncapsuleur(), Req_ActivateRelsOnAPath + kRangeReqDB4D);
			if (req == nil)
			{
				err = ThrowBaseError(memfull, noaction);
			}
			else
			{
				req->PutBaseParam(base);
				req->PutThingsToForget( VDBMgr::GetManager(), fContext);
				req->PutTableParam(fFields->GetMainTable());
				req->PutFicheInMemParam(mainrec, fContext->GetEncapsuleur());
				ListOfRelation* rels = &(found->second.fRelPath);

				req->PutLongParam((sLONG)rels->size());
				for (ListOfRelation::iterator cur = rels->begin(), end = rels->end(); cur != end; cur++)
				{
					req->PutRelationParam((*cur).GetRel());
				}
				err = req->GetLastError();
				if (err == VE_OK)
				{
					err = req->Send();
					if (err == VE_OK)
					{
						Boolean onerec = req->GetBooleanReply(err);
						if (err == VE_OK && onerec)
						{
							result = req->RetainFicheInMemReply(targetTable, err, fContext->GetEncapsuleur());
						}
					}
				}
				req->Release();
			}
		}
	}

	return result;
}


VError RemoteRecordCache::StartCachingRemoteRecords(Selection* inSel, RecIDType FromRecIndex, RecIDType ToRecIndex, const vector<uBYTE>& inWayOfLocking)
{
	fWayOfLocking = inWayOfLocking;

	fStampCopy = fFields->GetStamp();

	fSel = RetainRefCountable(inSel);

	VError err = InitAllRelatedRecordsOnClient();

	assert(inSel != nil);

	if (err == VE_OK)
	{
		if (FromRecIndex < 0)
			FromRecIndex = 0;

		if (ToRecIndex >= inSel->GetQTfic())
			ToRecIndex = inSel->GetQTfic()-1;

		fStartRecIndex = FromRecIndex;
		fEndRecIndex = ToRecIndex;

		Base4D* base = fFields->GetMainTable()->GetOwner();
		IRequest *req = base->CreateRequest( fContext->GetEncapsuleur(), Req_CacheDisplaySelection + kRangeReqDB4D);
		if (req == nil)
		{
			err = ThrowBaseError(memfull, noaction);
		}
		else
		{
			req->PutBaseParam(base);
			req->PutThingsToForget( VDBMgr::GetManager(), fContext);
			req->PutTableParam(fFields->GetMainTable());
			req->PutSelectionParam(inSel, fContext->GetEncapsuleur());
			req->PutLongParam(FromRecIndex);
			req->PutLongParam(ToRecIndex);
			req->PutLongParam((sLONG)inWayOfLocking.size());
			if (inWayOfLocking.size() != 0)
				req->GetOutputStream()->PutData(&inWayOfLocking[0], inWayOfLocking.size());
			
			err = fFields->WriteToStream(req->GetOutputStream());
			if (err == VE_OK)
				err = req->GetLastError();
			if (err == VE_OK)
			{
				err = req->Send();
				if (err== VE_OK)
					err = req->GetUpdatedInfo(base, fContext);

				if (err == VE_OK)
				{
					try
					{
						sLONG nbrows = ToRecIndex - FromRecIndex + 1;
						fRows.clear();
						if (nbrows > 0)
							fRows.resize(nbrows);

						sLONG currow = FromRecIndex;

						VStream* fromserver = req->GetInputStream();
						uBYTE recpresent;
						do 
						{
							err = fromserver->GetByte(recpresent);
							if (err == VE_OK && recpresent != '.')
							{
								RowCacheInfo* row = &(fRows[currow-FromRecIndex]);
								if (recpresent == '+')
								{
									err = fFields->ReadDataFromStream(row, fromserver, fContext, this);
								}
								else
								{
									if (recpresent == '-')
									{
										row->fMainRec = nil;
									}
									else
										err = ThrowBaseError(VE_DB4D_INVALID_PARAMETER);
								}
							}

							currow++;
						} while(recpresent != '.' && err == VE_OK && currow <= ToRecIndex);
					}
					catch (...)
					{
						err = ThrowBaseError(memfull);
					}
				}
			}
			req->Release();
		}

	}

	return err;
}


			// -------------------------------------------------------------------------



void ServerProgressTask::DoInit()
{

	// on attend que notre createur soit completement initialise
	Sleep( 500L);
}

void ServerProgressTask::DoDeInit()
{
}


Boolean ServerProgressTask::DoRun()
{
	Boolean cont = true;

	if( GetState() < TS_DYING) 
	{
		// main loop here
		if (GetState() < TS_DYING)
		{
			fTimer.ResetAndLock();
			fTimer.Wait(400);
		}
	}
	else cont = false;

	return cont;
}



		// ---------------------------------------------------



void ClientProgressTask::DoInit()
{

	// on attend que notre createur soit completement initialise
	Sleep( 500L);
}

void ClientProgressTask::DoDeInit()
{
}


Boolean ClientProgressTask::DoRun()
{
	Boolean cont = true;

	if( GetState() < TS_DYING) 
	{
		// main loop here
		if (GetState() < TS_DYING)
		{
			fTimer.ResetAndLock();
			fTimer.Wait(400);
		}
	}
	else cont = false;

	return cont;
}














