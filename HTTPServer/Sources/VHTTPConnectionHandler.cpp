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
#include "VHTTPConnectionHandler.h"
#include "HTTPServerCache.h"
#include "VVirtualHost.h"
#include "VHTTPServer.h"


//--------------------------------------------------------------------------------------------------

XBOX::VString VHTTPConnectionHandler::fTaskName;


VHTTPConnectionHandler::VHTTPConnectionHandler (VHTTPServer * inServer)
: fHTTPServerSession (NULL)
, fHTTPServer (NULL)
, fShouldStop (false)
{
	fHTTPServer = XBOX::RetainRefCountable (inServer);
}


VHTTPConnectionHandler::~VHTTPConnectionHandler()
{
	XBOX::ReleaseRefCountable (&fHTTPServerSession);
	XBOX::ReleaseRefCountable (&fHTTPServer);
}


XBOX::VError VHTTPConnectionHandler::SetEndPoint (XBOX::VEndPoint *inEndPoint)
{
	XBOX::VTCPEndPoint *tcpEndPoint = dynamic_cast<XBOX::VTCPEndPoint*> (inEndPoint);
	if (!tcpEndPoint)
		return XBOX::VE_INVALID_PARAMETER;

	VHTTPResponse * httpResponse = new VHTTPResponse (fHTTPServer, tcpEndPoint, tcpEndPoint->GetSelectIOPool());

	if (NULL != httpResponse)
	{
		fHTTPServerSession = new VHTTPServerSession (httpResponse);
	}

	return XBOX::VE_OK;
}


XBOX::VError VHTTPConnectionHandler::Stop()
{
	fShouldStop = true;

	if (NULL == fHTTPServerSession)
		return XBOX::VE_OK;

	XBOX::VTCPEndPoint *tcpEndPoint = fHTTPServerSession->GetEndPoint();
	if (NULL != tcpEndPoint)
	{
		/* TODO: Some sort of synchronization mechanism is needed to make sure that this code is called
		only when the socket is actually blocked on reading.*/
		tcpEndPoint->SetIsBlocking (false);
		tcpEndPoint->Close();
	}
	
	return XBOX::VE_OK;
}


enum VConnectionHandler::E_WORK_STATUS VHTTPConnectionHandler::Handle (XBOX::VError& outError)
{
	XBOX::VTask *currentTask = XBOX::VTask::GetCurrent();
	if (fTaskName.IsEmpty())
		fHTTPServer->LocalizeString (CVSTR ("HTTP_WORKER_THREAD"), fTaskName);
	currentTask->SetName (fTaskName);
	currentTask->SetKind (kHTTPTaskKind_WorkerPool_Server);
	/* Do not set curtask->SetKindData(): the TaskKindData is handled by the worker pool (see VExclusiveWorker)*/

	while ((XBOX::VTask::GetCurrent()->GetState() != XBOX::TS_DYING) && (XBOX::VTask::GetCurrent()->GetState() != XBOX::TS_DEAD) && !fShouldStop)
	{
#if LOG_IN_CONSOLE
		VDebugTimer timer;
#endif
		outError = fHTTPServerSession->HandleRequest();
#if LOG_IN_CONSOLE
		timer.DebugMsg ("* HTTPConnection::HandleRequest()");
#endif

		if (fHTTPServerSession->KeepAlive())
		{
			if (fHTTPServerSession->IsTimedOut() || fHTTPServerSession->IsMaxConnectionsReached())
			{
				fHTTPServerSession->Close();
				break;
			}

			if (VE_HTTP_PROTOCOL_REQUEST_TIMEOUT == outError)
				outError = XBOX::VE_OK;
		}
		else if (fHTTPServerSession->IsClosed())
		{
			outError = XBOX::VE_OK;
			break;
		}

		if (XBOX::VE_OK != outError)
			break;
	}
	/* end of: while (current thread is alive) */

	if (NULL != fHTTPServerSession)
	{
		if (!fHTTPServerSession->IsClosed())
			fHTTPServerSession->Close();

		XBOX::ReleaseRefCountable (&fHTTPServerSession);
	}

	// At this point, the workerpool will change the TaskKind, TaskKindData and taskname

	return XBOX::VConnectionHandler::eWS_DONE;
}
