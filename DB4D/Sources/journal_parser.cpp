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

// =================================================================================

DB4DJournalHeader::DB4DJournalHeader()
{
	fFileSignature1 = 0;
	fFileSignature2 = 0;
}

DB4DJournalHeader::~DB4DJournalHeader()
{
}

bool DB4DJournalHeader::IsValid()
{
	bool result = true;
	if ( fFileSignature1 != '4lOG' || fFileSignature2 != 'DATA' )
		result = false;
	return result;
}


bool DB4DJournalHeader::IsBeforeV14()
{
	return (fFileSignature1 == '4LOG') && (fFileSignature2 == 'DATA');
}


bool DB4DJournalHeader::IsValid( const VUUID &inDataLink )
{
	Boolean result = true;
	if ( fFileSignature1 != '4lOG' || fFileSignature2 != 'DATA' )
		result = false;
	else
	{
		if (!inDataLink.IsNull())
		{
			if ( inDataLink != fDataLink )
				result = false;
		}
	}
	return result;
}

void DB4DJournalHeader::Init( VUUID &inDataLink )
{
	fDataLink = inDataLink;
	fFileSignature1 = '4lOG';
	fFileSignature2 = 'DATA';
}

VError DB4DJournalHeader::ReadFromStream (VStream* ioStream, sLONG inParam )
{
	VError error = ioStream->GetLong(fFileSignature1);
	if ( error == VE_OK )
		error = ioStream->GetLong(fFileSignature2);
	if ( error == VE_OK )
		error = fDataLink.ReadFromStream(ioStream);

	return error;
}

VError DB4DJournalHeader::WriteToStream (VStream* ioStream, sLONG inParam ) const
{
	VError error = ioStream->PutLong(fFileSignature1);
	if ( error == VE_OK )
		error = ioStream->PutLong(fFileSignature2);
	if ( error == VE_OK )
		error = fDataLink.WriteToStream(ioStream);

	return error;
}

// =================================================================================

DB4DJournalParser::DB4DJournalParser(VFile* inJournalFile)
: fFirstOperation( 0)
, fLastOperation( 0)
{
	fCurrentOperation = 0;
	fTotalOperationCount = 0;
	fLogFile = inJournalFile;
	fLogFile->Retain();
	fFileStream = NULL;
	fCurrentData = NULL;
}

DB4DJournalParser::~DB4DJournalParser()
{
	fLogFile->Release();
	fLogFile = NULL;
}

VError DB4DJournalParser::Init( uLONG8 &outTotalOperationCount, VDB4DProgressIndicator *inProgressIndicator )
{
	VError error = VE_OK;
	outTotalOperationCount = 0;
	if ( fLogFile && fLogFile->Exists() )
	{
		fFileStream = new VFileStream(fLogFile);
		if ( fFileStream )
		{
			fFileStream->SetBufferSize(1024*1024);
			error = fFileStream->OpenReading();
			
			if ( error == VE_OK )
				error = fFileHeader.ReadFromStream(fFileStream);

			if ( fFileHeader.IsValid() )
			{
				fModuloDataOffset.AppendLong8(fFileStream->GetPos());
				error = _CheckFile( inProgressIndicator );
				outTotalOperationCount = fTotalOperationCount;
			}
			else
			{
				error = VE_UNIMPLEMENTED; // invalid file
			}
		}
		else
		{
			error = VE_MEMORY_FULL;
		}
	}
	else
	{
		error = VE_UNIMPLEMENTED; // file not found
	}
	return error;
}

VError DB4DJournalParser::DeInit()
{
	VError error = VE_OK;
	if ( fFileStream )
	{
		error = fFileStream->CloseReading();
		delete fFileStream;
		fFileStream = NULL;

		if ( fCurrentData )
		{
			fCurrentData->Release();
			fCurrentData = NULL;
		}

		fModuloDataOffset.Clear();

	}
	else
	{
		error = VE_UNIMPLEMENTED; // not initialized
	}
	return error;
}

VError DB4DJournalParser::SetCurrentOperation( uLONG8 inOperation, CDB4DJournalData **outJournalData )
{
	VError error = VE_OK;
	if ( fFileStream )
	{
		if ( inOperation == fCurrentOperation )
		{
			*outJournalData = fCurrentData;
			if (fCurrentData != nil)
				fCurrentData->Retain();
		}
		else
		{
			if ((inOperation < fCurrentOperation) || (inOperation > (fCurrentOperation+4096)))
			{
				if ( inOperation%512 )
				{
					fFileStream->SetPos(fModuloDataOffset.GetLong8((inOperation/512)+1));
					fCurrentOperation = ((inOperation/512)*512);
				}
				else
				{
					fFileStream->SetPos(fModuloDataOffset.GetLong8(inOperation/512));
					fCurrentOperation = (((inOperation-1)/512)*512);
				}
			}

			*outJournalData = NULL;
			while ( error == VE_OK && fCurrentOperation < inOperation )
			{
				if ( *outJournalData )
					(*outJournalData)->Release();
				error = NextOperation( fCurrentOperation, NULL, outJournalData );
			}
		}

		if ( *outJournalData && error != VE_OK )
		{
			(*outJournalData)->Release();
			*outJournalData = NULL;
		}
	}
	else
	{
		*outJournalData = NULL;
		error = VE_UNIMPLEMENTED; // not initialized
	}
	return error;
}

const VValueBag *DB4DJournalParser::GetContextExtraData( uLONG8 inContextID) const
{
	MapOfExtraByID::const_iterator i = fContextExtraByID.find( inContextID);
	return (i != fContextExtraByID.end()) ? i->second : NULL;
}


VError DB4DJournalParser::NextOperation( uLONG8 &outOperation, uLONG8 *outGlobalOperation, CDB4DJournalData **outJournalData )
{
	VError error = VE_OK;
	RecordHeader recHeader;
	
	if ( outJournalData )
		*outJournalData = NULL;

	outOperation = fCurrentOperation;
	sLONG8 globaloperation = 0;
	
	if ( fFileStream )
	{
		uLONG operationTag;
		error = fFileStream->GetLong(operationTag);
		if ( error == VE_OK )
		{
			if ( operationTag == kTagLogDB4D )
			{
				sLONG8 contextID;
				DB4D_LogAction logAction;
				sLONG len;
				sLONG8 curpos;
				error = fFileStream->GetLong8(globaloperation);

				if ( error == VE_OK )
				{
					error = fFileStream->GetLong(len);
				}

				if ( !outJournalData )
				{
					error = fFileStream->SetPosByOffset( len - 24 );//- 4 /*Tag*/ - 8 /*Operation#*/- 4 /*len*/ - 4 /*len at the end*/ - 4 /*tag at the end*/
				}
				else
				{

				if ( error == VE_OK )
					error = fFileStream->GetLong((uLONG&)logAction);

				if ( error == VE_OK )
					error = fFileStream->GetLong8(contextID);

				uLONG8 timeStamp;
				if (error == VE_OK)
					error = fFileStream->GetLong8(timeStamp);


				if (error == VE_OK)
				{
					switch (logAction)
					{
						case DB4D_Log_OpenData:
						case DB4D_Log_CloseData:
						case DB4D_Log_StartBackup:
						case DB4D_Log_StartTrans:
						case DB4D_Log_Commit:
						case DB4D_Log_RollBack:
						case DB4D_Log_StartCommit:
						case DB4D_Log_EndCommit:
							{
								*outJournalData = new VDB4DJournalData(globaloperation, logAction,contextID, GetContextExtraData( contextID),timeStamp);
							}
							break;

						case DB4D_Log_CloseContext:
							{
								*outJournalData = new VDB4DJournalData(globaloperation, logAction,contextID, GetContextExtraData( contextID),timeStamp);
								fContextExtraByID.erase(contextID);
							}
							break;

						case DB4D_Log_CreateRecordWithPrimKey:
						case DB4D_Log_ModifyRecordWithPrimKey:
							{
								PrimKey* primkey = new PrimKey();
								error = primkey->GetFrom(fFileStream);

								VUUID xTableID;
								if ( error == VE_OK )
									error = recHeader.ReadFromStream(fFileStream);

								if ( error == VE_OK )
									if ( !recHeader.Match(DBOH_Record) )
										error = ThrowBaseError(VE_DB4D_WRONGRECORDHEADER);

								if ( error == VE_OK )
									error = xTableID.ReadFromStream(fFileStream);

								curpos = fFileStream->GetPos();
								if ( error == VE_OK )
								{
									sLONG dataSize = recHeader.GetLen() + sizeof(ChampHeader)*(recHeader.GetNbFields());
									error = fFileStream->SetPosByOffset(dataSize);
								}
								if ( error == VE_OK )
									*outJournalData = new VDB4DJournalData(globaloperation, logAction,contextID, GetContextExtraData( contextID),&recHeader,timeStamp, curpos, fFileStream, xTableID, primkey);

								primkey->Release();
							}
							break;

						case DB4D_Log_DeleteRecordWithPrimKey:
							{
								PrimKey* primkey = new PrimKey();
								error = primkey->GetFrom(fFileStream);

								sLONG recordNumber;
								if ( error == VE_OK )
									error = fFileStream->GetLong(recordNumber);
								/*
								sLONG tableIndex;
								if ( error == VE_OK )
									error = fFileStream->GetLong(tableIndex);
								*/
								VUUID xTableID;
								if ( error == VE_OK )
									error = xTableID.ReadFromStream(fFileStream);

								if ( error == VE_OK )
									*outJournalData = new VDB4DJournalData(globaloperation, logAction,contextID, GetContextExtraData( contextID),timeStamp,recordNumber,xTableID, primkey);
								primkey->Release();
							}
							break;

						case DB4D_Log_CreateBlobWithPrimKey:
						case DB4D_Log_ModifyBlobWithPrimKey:
							{
								PrimKey* primkey = new PrimKey();
								error = primkey->GetFrom(fFileStream);

								sLONG fieldnum = -1;
								if (error == VE_OK)
									error = fFileStream->GetLong(fieldnum);

								VString path;

								sLONG lenblob = 0;
								sLONG blobNumber = 0;
								if ( error == VE_OK )
									error = fFileStream->GetLong(blobNumber);

								if (blobNumber == -2)
								{
									sLONG lenpath = 0;
									error = fFileStream->GetLong(lenpath);
									if (lenpath > 0)
									{
										tempBuffer<256> buff(len);
										error = fFileStream->GetWords((sWORD*)buff.GetPtr(), &lenpath);
										path.FromBlock(buff.GetPtr(), lenpath * sizeof(UniChar), VTC_UTF_16);
									}
								}


								VUUID xTableID;
								if ( error == VE_OK )
									error = xTableID.ReadFromStream(fFileStream);

								curpos = fFileStream->GetPos();
								if ( error == VE_OK )
								{
									error = fFileStream->GetLong(lenblob);
									error = fFileStream->SetPosByOffset(lenblob);
								}
								if ( error == VE_OK )
								{
									VDB4DJournalData* jdata = new VDB4DJournalData(globaloperation, logAction,contextID, GetContextExtraData( contextID),timeStamp, blobNumber, lenblob, xTableID, curpos, fFileStream, primkey, fieldnum);
									*outJournalData = jdata;
									if (!path.IsEmpty())
									{
										jdata->SetPath(path);
									}
								}
								primkey->Release();
							}
							break;

						case DB4D_Log_DeleteBlobWithPrimKey:
							{
								PrimKey* primkey = new PrimKey();
								error = primkey->GetFrom(fFileStream);

								sLONG fieldnum = -1;
								if (error == VE_OK)
									error = fFileStream->GetLong(fieldnum);

								VString path;
								sLONG blobnumber = 0;
								if ( error == VE_OK )
									error = fFileStream->GetLong(blobnumber);

								if (blobnumber == -2)
								{
									sLONG lenpath = 0;
									error = fFileStream->GetLong(lenpath);
									if (lenpath > 0)
									{
										tempBuffer<256> buff(len);
										error = fFileStream->GetWords((sWORD*)buff.GetPtr(), &lenpath);
										path.FromBlock(buff.GetPtr(), lenpath * sizeof(UniChar), VTC_UTF_16);
									}
								}

								VUUID xTableID;
								if ( error == VE_OK )
									error = xTableID.ReadFromStream(fFileStream);

								if ( error == VE_OK )
								{
									VDB4DJournalData* jdata = new VDB4DJournalData(globaloperation, logAction,contextID, GetContextExtraData( contextID),timeStamp, blobnumber ,xTableID, primkey, fieldnum);
									*outJournalData = jdata;
									if (!path.IsEmpty())
									{
										jdata->SetPath(path);
									}
								}
								primkey->Release();
							}
							break;


						case DB4D_Log_CreateRecord:
						case DB4D_Log_ModifyRecord:
							{
								VUUID xTableID;
								if ( error == VE_OK )
									error = recHeader.ReadFromStream(fFileStream);
								
								if ( error == VE_OK )
									if ( !recHeader.Match(DBOH_Record) )
										error = VE_DB4D_WRONGRECORDHEADER;
								
								if ( error == VE_OK )
									error = xTableID.ReadFromStream(fFileStream);

								curpos = fFileStream->GetPos();
								if ( error == VE_OK )
								{
									sLONG dataSize = recHeader.GetLen() + sizeof(ChampHeader)*(recHeader.GetNbFields());
									error = fFileStream->SetPosByOffset(dataSize);
								}
								if ( error == VE_OK )
									*outJournalData = new VDB4DJournalData(globaloperation, logAction,contextID, GetContextExtraData( contextID),&recHeader,timeStamp, curpos, fFileStream, xTableID);
							}
							break;

						case DB4D_Log_DeleteBlob:
							{
								VString path;
								sLONG blobnumber = 0;
								if ( error == VE_OK )
									error = fFileStream->GetLong(blobnumber);

								if (blobnumber == -2)
								{
									sLONG lenpath = 0;
									error = fFileStream->GetLong(lenpath);
									if (lenpath > 0)
									{
										tempBuffer<256> buff(len);
										error = fFileStream->GetWords((sWORD*)buff.GetPtr(), &lenpath);
										path.FromBlock(buff.GetPtr(), lenpath * sizeof(UniChar), VTC_UTF_16);
									}
								}
								
								VUUID xTableID;
								if ( error == VE_OK )
									error = xTableID.ReadFromStream(fFileStream);

								if ( error == VE_OK )
								{
									VDB4DJournalData* jdata = new VDB4DJournalData(globaloperation, logAction,contextID, GetContextExtraData( contextID),timeStamp, blobnumber ,xTableID);
									*outJournalData = jdata;
									if (!path.IsEmpty())
									{
										jdata->SetPath(path);
									}
								}
							}
							break;

						case DB4D_Log_DeleteRecord:
						case DB4D_Log_TruncateTable:
							{
								sLONG recordNumber;
								if ( error == VE_OK )
									error = fFileStream->GetLong(recordNumber);
								/*
								sLONG tableIndex;
								if ( error == VE_OK )
									error = fFileStream->GetLong(tableIndex);
								*/
								VUUID xTableID;
								if ( error == VE_OK )
									error = xTableID.ReadFromStream(fFileStream);

								if ( error == VE_OK )
									*outJournalData = new VDB4DJournalData(globaloperation, logAction,contextID, GetContextExtraData( contextID),timeStamp,recordNumber,xTableID);
							}
							break;

						case DB4D_Log_CreateContextWithUserUUID:
							{
								VUUID userID;
								error = userID.ReadFromStream(fFileStream);
								if (error == VE_OK)
									*outJournalData = new VDB4DJournalData(globaloperation, logAction,contextID, GetContextExtraData( contextID),timeStamp,userID);
							}
							break;
						
						case DB4D_Log_CreateContextWithExtra:
							{
								VValueBag *bag = new VValueBag;
								if (bag != NULL)
								{
									// the extra data is always stored in little endian
									Boolean oldNeedSwap = fFileStream->NeedSwap();
									fFileStream->SetLittleEndian();
									error = bag->ReadFromStream(fFileStream);
									fFileStream->SetNeedSwap( oldNeedSwap);
									
									if (error == VE_OK)
									{
										try
										{
											fContextExtraByID[contextID] = bag;
										}
										catch(...)
										{
										}
										*outJournalData = new VDB4DJournalData(globaloperation, logAction,contextID, GetContextExtraData( contextID),timeStamp);
									}
								}
								else
								{
									error = memfull;
								}
								ReleaseRefCountable( &bag);
							}
							break;

						case DB4D_Log_CreateBlob:
						case DB4D_Log_ModifyBlob:
							{
								VString path;

								sLONG lenblob = 0;
								sLONG blobNumber = 0;
								if ( error == VE_OK )
									error = fFileStream->GetLong(blobNumber);

								if (blobNumber == -2)
								{
									sLONG lenpath = 0;
									error = fFileStream->GetLong(lenpath);
									if (lenpath > 0)
									{
										tempBuffer<256> buff(len);
										error = fFileStream->GetWords((sWORD*)buff.GetPtr(), &lenpath);
										path.FromBlock(buff.GetPtr(), lenpath * sizeof(UniChar), VTC_UTF_16);
									}
								}


								VUUID xTableID;
								if ( error == VE_OK )
									error = xTableID.ReadFromStream(fFileStream);

								curpos = fFileStream->GetPos();
								if ( error == VE_OK )
								{
									error = fFileStream->GetLong(lenblob);
									error = fFileStream->SetPosByOffset(lenblob);
								}
								if ( error == VE_OK )
								{
									VDB4DJournalData* jdata = new VDB4DJournalData(globaloperation, logAction,contextID, GetContextExtraData( contextID),timeStamp, blobNumber, lenblob, xTableID, curpos, fFileStream);
									*outJournalData = jdata;
									if (!path.IsEmpty())
									{
										jdata->SetPath(path);
									}
								}
							}
							break;

						case DB4D_Log_SaveSeqNum:
							{
								sLONG8 seqnum;
								if ( error == VE_OK )
									error = fFileStream->GetLong8(seqnum);
								/*
								sLONG tableIndex;
								if ( error == VE_OK )
								error = fFileStream->GetLong(tableIndex);
								*/
								VUUID xTableID;
								if ( error == VE_OK )
									error = xTableID.ReadFromStream(fFileStream);

								if ( error == VE_OK )
									*outJournalData = new VDB4DJournalData(globaloperation, logAction,contextID, GetContextExtraData( contextID), timeStamp, seqnum, xTableID, true);
							}
							break;

						default:
							xbox_assert(false);
							break;
					}
				}
				}
				sLONG lenEnd;
				if ( error == VE_OK )
				{
					error = fFileStream->GetLong(lenEnd);
					if (len != lenEnd)
						error = VE_UNIMPLEMENTED;
				}

				uLONG operationTagEnd;
				if (error == VE_OK)
				{
					error = fFileStream->GetLong((uLONG&)operationTagEnd);
					if (operationTagEnd != kTagLogDB4DEnd)
						error = VE_UNIMPLEMENTED;
				}
			}
			else
			{
				error = VE_UNIMPLEMENTED; // bad file
			}

			if ( error != VE_OK )
			{
				if ( outJournalData && *outJournalData )
				{
					(*outJournalData)->Release();
					*outJournalData = NULL;
				}
			}
			else
			{
				outOperation = ++fCurrentOperation;
				if ( outJournalData && *outJournalData )
				{
					if ( fCurrentData )
						fCurrentData->Release();
					fCurrentData = *outJournalData;
					fCurrentData->Retain();
				}
			}
		}
	}
	else
	{
		error = VE_UNIMPLEMENTED; // not initialized
	}

	if (outGlobalOperation != NULL)
		*outGlobalOperation = globaloperation;
	
	return error;
}

VError DB4DJournalParser::_CheckFile( VDB4DProgressIndicator *inProgressIndicator )
{
	VError error = VE_OK;
	if ( fFileStream )
	{
		sLONG8 streamPos = 0;
		if ( inProgressIndicator )
		{
			inProgressIndicator->BeginSession( fFileStream->GetSize(), GetString(1005, 24) );
		}

		while ( error == VE_OK )
		{
			CDB4DJournalData *jData = NULL;
			uLONG8 globalOperation;
			error = NextOperation( fTotalOperationCount, &globalOperation, fTotalOperationCount ? NULL : &jData );
			if ( jData )
			{
				jData->Release();
			}

			if (globalOperation != 0)
				fLastOperation = globalOperation;
			if (fFirstOperation == 0)
				fFirstOperation = globalOperation;
				
			if ( inProgressIndicator )
			{
				if ( !inProgressIndicator->Progress( streamPos ) )
					error = VE_STREAM_USER_ABORTED;
				else if ( fTotalOperationCount%128 == 0 )
					VTask::Yield();
			}

			if ( fTotalOperationCount%512 == 0 ) 
			{
				streamPos = fFileStream->GetPos();
				fModuloDataOffset.AppendLong8(streamPos);
			}
		}

		if ( error == VE_STREAM_EOF )
		{
			fFileStream->ResetLastError();
			error = VE_OK;
		}

		if ( inProgressIndicator )
		{
			inProgressIndicator->EndSession();
		}
	}
	else
	{
		error = VE_UNIMPLEMENTED; // not initialized
	}
	return error;
}

VError DB4DJournalParser::SetEndOfJournal( uLONG8 inOperation )
{
	VError error = VE_OK;
	if ( fFileStream )
	{
		CDB4DJournalData *journalData = NULL;
		error = SetCurrentOperation( inOperation, &journalData );
		if ( journalData )
			journalData->Release();
		if ( error == VE_OK )
		{
			sLONG8 filePos = fFileStream->GetPos();
			error = DeInit();
			if ( error == VE_OK )
			{
				VFileDesc *fileDesc = NULL;
				error = fLogFile->Open( FA_READ_WRITE, &fileDesc );
				if ( error == VE_OK )
				{
					error = fileDesc->SetSize( filePos );
					delete fileDesc;
				}
				if ( error == VE_OK )
					fTotalOperationCount = fCurrentOperation;
			}
		}
	}
	else
	{
		error = VE_UNIMPLEMENTED; // not initialized
	}
	return error;
}