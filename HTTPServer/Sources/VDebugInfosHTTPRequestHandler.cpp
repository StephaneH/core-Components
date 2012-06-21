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
