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
#include "HTTPServer.h"
#include "VTCPServer.h"
#include "VHTTPConnectionHandler.h"
#include "VHTTPConnectionHandlerFactory.h"
#include "VHTTPConnectionListener.h"
#include "VHTTPServer.h"


//--------------------------------------------------------------------------------------------------


VHTTPConnectionListener::VHTTPConnectionListener (VHTTPServer *inServer, IRequestLogger* inRequestLogger)
: XBOX::VTask (NULL, 0, XBOX::eTaskStylePreemptive, NULL)
, fCertificatesFolderPath()
, fServerListener (NULL)
, fWorkerPool (NULL)
, fSelectIOPool (NULL)
, fRequestLogger (inRequestLogger)
, fListeningIP (DEFAULT_LISTENING_ADDRESS)	// 0: All IP addresses
, fPort (DEFAULT_LISTENING_PORT)
, fSSLPort (DEFAULT_LISTENING_SSL_PORT)
, fSSLEnabled (false)
, fSSLMandatory (false)
, fConnectionHandlerFactory (NULL)
, fHTTPServer (NULL)
, fUsageCounter (1)
, fSocketDescriptor (-1)
, fSSLSocketDescriptor (-1)
, fReuseAddressSocketOption (true)
, fAbortTask (false)
{
	fHTTPServer = XBOX::RetainRefCountable (inServer);

	XBOX::VString taskName;
	if (testAssert (NULL != fHTTPServer))
		fHTTPServer->LocalizeString (CVSTR ("SNET_CONNECTION_LISTENER"), taskName);
		
	SetName (taskName);
	SetKind (kServerNetTaskKind);
	SetKindData (kSNET_ConnectionListenerTaskKindData);

	const short	SHARED_WORKER_COUNT = 0;
	const short	SHARED_WORKER_MAX_COUNT = 0;
	const short	SHARED_WORKER_MAX_BUSYNESS = 0;
	const short	EXCLUSIVE_WORKER_COUNT = 5;
	const short	EXCLUSIVE_WORKER_MAX_COUNT = kMAX_sWORD;

	fWorkerPool = new VWorkerPool (	SHARED_WORKER_COUNT,
									SHARED_WORKER_MAX_COUNT,
									SHARED_WORKER_MAX_BUSYNESS,
									EXCLUSIVE_WORKER_COUNT,
									EXCLUSIVE_WORKER_MAX_COUNT);

	fSelectIOPool = new VTCPSelectIOPool();
}


VHTTPConnectionListener::~VHTTPConnectionListener()
{
	if (NULL != fServerListener)
	{
		delete fServerListener;
		fServerListener = NULL;
	}
	
	if (fHTTPServer)
		fHTTPServer->RemoveConnectionListener (this);

	XBOX::ReleaseRefCountable (&fConnectionHandlerFactory);
	XBOX::ReleaseRefCountable (&fWorkerPool);
	XBOX::ReleaseRefCountable (&fSelectIOPool);
	XBOX::ReleaseRefCountable (&fHTTPServer);
}


void VHTTPConnectionListener::IncrementUsageCounter()
{
	XBOX::VInterlocked::Increment (&fUsageCounter);
}


void VHTTPConnectionListener::DecrementUsageCounter()
{
	XBOX::VInterlocked::Decrement (&fUsageCounter);
}


sLONG VHTTPConnectionListener::GetUsageCounter()
{
	return XBOX::VInterlocked::AtomicGet (&fUsageCounter);
}



void VHTTPConnectionListener::SetSSLMandatory (bool inValue)
{
	fSSLMandatory = inValue;

	if (fSSLMandatory && !fSSLEnabled)
		fSSLEnabled = true;
}


bool VHTTPConnectionListener::operator == (const VHTTPConnectionListener& inConnectionListener) const
{
	return (fListeningIP == inConnectionListener.fListeningIP &&
		fPort == inConnectionListener.fPort &&
		fSSLPort == inConnectionListener.fSSLPort &&
		fSSLEnabled == inConnectionListener.fSSLEnabled &&
		fSSLMandatory == inConnectionListener.fSSLMandatory);
}


XBOX::VError VHTTPConnectionListener::AddConnectionHandlerFactory (VConnectionHandlerFactory *inFactory)
{
	return XBOX::VE_OK;
}


XBOX::VError VHTTPConnectionListener::AddWorkerPool (XBOX::VWorkerPool *inPool)
{
	if (inPool != fWorkerPool)
	{
		XBOX::ReleaseRefCountable (&fWorkerPool);
		fWorkerPool = XBOX::RetainRefCountable (inPool);
	}

	return XBOX::VE_OK;
}


XBOX::VError VHTTPConnectionListener::AddSelectIOPool (XBOX::VTCPSelectIOPool *inPool)
{
	if (inPool != fSelectIOPool)
	{
		XBOX::ReleaseRefCountable (&fSelectIOPool);
		fSelectIOPool = XBOX::RetainRefCountable (inPool);
	}

	return XBOX::VE_OK;
}


void VHTTPConnectionListener::SetSSLCertificatesFolderPath (const XBOX::VFilePath& inCertificatesFolderPath)
{
	fCertificatesFolderPath.FromFilePath (inCertificatesFolderPath);
}


XBOX::VError VHTTPConnectionListener::StartListening()
{
	XBOX::VError error = XBOX::VE_OK;

	if (NULL == fServerListener)
	{
		fServerListener = new VSockListener  (fRequestLogger);

		if (fSSLEnabled || fSSLMandatory)
		{
			if (fCertificatesFolderPath.IsFolder() && fCertificatesFolderPath.IsValid())
			{
				XBOX::VFile keyFile (fCertificatesFolderPath, STRING_KEY_FILE_NAME);
				XBOX::VFile certFile (fCertificatesFolderPath, STRING_CERT_FILE_NAME);

				if (keyFile.Exists() && certFile.Exists())
					error = fServerListener->SetCertificateFolder (fCertificatesFolderPath, STRING_KEY_FILE_NAME, STRING_CERT_FILE_NAME);
			}

			if (XBOX::VE_OK != error)
			{
				/*
				 *	If SSL publication is enabled but not mandatory: Silently disable SSL Publishing 
				 *	other else, if SSL is Mandatory: no Way... Throw an error and prevent StartListening to run
				 */

				if (fSSLEnabled)
				{
					XBOX::VErrorTaskContext *	errorTaskContext = XBOX::VTask::GetCurrent()->GetErrorContext (false);
					XBOX::VErrorContext *		errorContext = (errorTaskContext != NULL) ? errorTaskContext->GetLastContext() : NULL;

					errorContext->Flush();

					error = XBOX::VE_OK;
					fSSLEnabled = false;
				}
			}
		}

		if (XBOX::VE_OK == error)
		{
			/* We just here need to add 2 listeners (one on for http and another one for https)*/
			bool isOK = true;

			if (!fSSLMandatory)
				isOK = fServerListener->AddListeningPort (fListeningIP, fPort, false, fSocketDescriptor, fReuseAddressSocketOption);

			if (isOK && fSSLEnabled)
				fServerListener->AddListeningPort (fListeningIP, fSSLPort, true, fSSLSocketDescriptor, fReuseAddressSocketOption);

			/* Add a VConnectionHandlerFactory for HTTP requests */
			fConnectionHandlerFactory = new VHTTPConnectionHandlerFactory (fHTTPServer);
			if (NULL == fConnectionHandlerFactory)
				error = VE_HTTP_MEMORY_ERROR;

			if (XBOX::VE_OK == error)
			{
				fAbortTask = false;

				if (fServerListener->StartListening())
					Run();
				else
					error = XBOX::ServerNetTools::ThrowNetError (VE_SRVR_FAILED_TO_START_LISTENER);
			}
		}

		if  (XBOX::VE_OK != error)
			DeInit();
	}

	return error;
}


inline
bool VHTTPConnectionListener::IsListening()
{
	return ((GetState() != XBOX::TS_DYING) && (GetState() != XBOX::TS_DEAD) && (NULL != fServerListener) && (!fAbortTask));
}


XBOX::VError VHTTPConnectionListener::StopListening()
{
	fAbortTask = true;

	Kill();
	
	//jmo - tmp fix : wait for listening socket to be closed.
	VTask::Sleep(1000);

	return XBOX::VE_OK;
}


void VHTTPConnectionListener::DeInit()
{
	if (NULL != fServerListener)
		fServerListener->StopListeningAndClearPorts();

	if (NULL != fConnectionHandlerFactory)
		fWorkerPool->StopConnectionHandlers (fConnectionHandlerFactory->GetType(), GetID());
}


Boolean VHTTPConnectionListener::DoRun()
{
	XBOX::VError error = VE_OK;

	if (NULL != fRequestLogger)
		fRequestLogger->Log ('SRNT', 0, "SERVER_NET::VHTTPConnectionListener::DoRun()::Enter", 1);

	while (IsListening())
	{
        //jmo - Do not let errors accumulate in thread's error stack
        XBOX::VErrorContext errCtx(false, false);
        
		XTCPSock *	newConnection = fServerListener->GetNewConnectedSocket (1000 /*MsTimeout*/);

		if (NULL != newConnection)
		{
			XBOX::VTCPEndPoint *endPoint = new XBOX::VTCPEndPoint (newConnection, fSelectIOPool);

			if (testAssert (NULL != endPoint))
			{
				XBOX::VConnectionHandler * connectionHandler = fConnectionHandlerFactory->CreateConnectionHandler (error);

				if (testAssert ((NULL != connectionHandler) && (XBOX::VE_OK == error)))
				{
					connectionHandler->_ResetRedistributionCount();
					error = connectionHandler->SetEndPoint (endPoint);
					assert (XBOX::VE_OK == error);
				}

				/* Transfer connectionHandler to the thread pool for execution. */
				if (testAssert (NULL != fWorkerPool) && IsListening())
				{
					error = fWorkerPool->AddConnectionHandler (connectionHandler);
					assert (XBOX::VE_OK == error);
				}
			}
		}
	}

	DeInit();

	XBOX::VTask::GetCurrent()->Sleep (1000);

	if (NULL != fRequestLogger)
		fRequestLogger->Log ('SRNT', 0, "SERVER_NET::VHTTPConnectionListener::DoRun()::Exit", 1);

	return false;
}
