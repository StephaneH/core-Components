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
#include "VDebugInfosHTTPRequestHandler.h"


//--------------------------------------------------------------------------------------------------


XBOX::VError VDebugInfosHTTPRequestHandler::GetPatterns (XBOX::VectorOfVString *outPatterns) const
{
	if (NULL == outPatterns)
		return VE_HTTP_INVALID_ARGUMENT;

	outPatterns->clear();
	outPatterns->push_back (CVSTR ("(?i)/debuginfos$"));
	return XBOX::VE_OK;
}


XBOX::VError VDebugInfosHTTPRequestHandler::HandleRequest (IHTTPResponse *ioResponse)
{
	if (NULL == ioResponse)
		return VHTTPServer::ThrowError (VE_HTTP_PROTOCOL_INTERNAL_SERVER_ERROR, STRING_ERROR_INVALID_PARAMETER);

	VHTTPResponse *				response = dynamic_cast<VHTTPResponse *>(ioResponse);
	VVirtualHost *				virtualHost = (NULL != response) ? dynamic_cast<VVirtualHost *>(response->GetVirtualHost()) : NULL;
	VAuthenticationManager *	authenticationManager = (NULL != virtualHost) ? dynamic_cast<VAuthenticationManager *>(virtualHost->GetProject()->GetAuthenticationManager()) : NULL;

	if (NULL != authenticationManager && (authenticationManager->CheckAdminAccessGranted (ioResponse) != XBOX::VE_OK))
		return VE_HTTP_PROTOCOL_UNAUTHORIZED;

	VHTTPServer * httpServer = dynamic_cast<VHTTPResponse *>(ioResponse)->GetHTTPServer();

	if (NULL != httpServer)
	{
		XBOX::VValueBag bag;

		httpServer->SaveToBag (bag);
		HTTPServerTools::SendValueBagResponse (*ioResponse, bag, CVSTR ("debugInfos"));

		return XBOX::VE_OK;
	}

	return VHTTPServer::ThrowError (VE_HTTP_PROTOCOL_INTERNAL_SERVER_ERROR, STRING_EMPTY);
}


//--------------------------------------------------------------------------------------------------


#if WITH_DETACH_ENDPOINT_REQUEST_HANDLER_TEST
static
sLONG _WebTaskRunProc (XBOX::VTask *inTask)
{
	if (!inTask->IsDying())
	{
		XBOX::VTCPEndPoint *endPoint = (XBOX::VTCPEndPoint *)inTask->GetKindData();
		if (NULL != endPoint)
		{
			XBOX::VString stringBuffer;

			stringBuffer.AppendCString ("HTTP/1.1 200 - OK\r\n");
			stringBuffer.AppendCString ("Connection: close\r\n");
			stringBuffer.AppendCString ("Content-Type: text/plain\r\n");
			stringBuffer.AppendCString ("Content-Length: 2\r\n");
			stringBuffer.AppendCString ("X-DebugInfos: Sent by VTestDetachEndPointHTTPRequestHandler\r\n\r\n");
			stringBuffer.AppendCString ("OK");

			XBOX::StStringConverter<char> buffer (stringBuffer, XBOX::VTC_UTF_8);

			endPoint->WriteExactly (buffer.GetCPointer(), buffer.GetSize());
			endPoint->Close();
		}


		XBOX::ReleaseRefCountable (&endPoint);
	}
	return 0;
}


XBOX::VError VTestDetachEndPointHTTPRequestHandler::GetPatterns (XBOX::VectorOfVString *outPatterns) const
{
	if (NULL == outPatterns)
		return VE_HTTP_INVALID_ARGUMENT;

	outPatterns->clear();
	outPatterns->push_back (CVSTR ("(?i)/testDetachEndPoint$"));
	return XBOX::VE_OK;
}


XBOX::VError VTestDetachEndPointHTTPRequestHandler::HandleRequest (IHTTPResponse *ioResponse)
{
	if (NULL == ioResponse)
		return VHTTPServer::ThrowError (VE_HTTP_PROTOCOL_INTERNAL_SERVER_ERROR, STRING_ERROR_INVALID_PARAMETER);

	VHTTPResponse *response = dynamic_cast<VHTTPResponse *>(ioResponse);
	XBOX::VTCPEndPoint *endPoint = response->DetachEndPoint();
	XBOX::VTask *task = new XBOX::VTask (NULL, 0, eTaskStylePreemptive, _WebTaskRunProc);
	task->SetKindData ((sLONG_PTR)endPoint);
	task->Run();
	task->Release();

	return XBOX::VE_OK;
}
#endif	// WITH_DETACH_ENDPOINT_REQUEST_HANDLER_TEST
