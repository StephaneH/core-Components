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
#ifndef __DEBUG_INFOS_HTTP_REQUEST_HANDLER_INCLUDED__
#define __DEBUG_INFOS_HTTP_REQUEST_HANDLER_INCLUDED__

#define WITH_DETACH_ENDPOINT_REQUEST_HANDLER_TEST 0


class VDebugInfosHTTPRequestHandler : public IHTTPRequestHandler
{
public:
									VDebugInfosHTTPRequestHandler() {};
	virtual XBOX::VError			GetPatterns (XBOX::VectorOfVString *outPatterns) const;
	virtual XBOX::VError			HandleRequest (IHTTPResponse *ioResponse);
};


#if WITH_DETACH_ENDPOINT_REQUEST_HANDLER_TEST
class VTestDetachEndPointHTTPRequestHandler : public IHTTPRequestHandler
{
public:
									VTestDetachEndPointHTTPRequestHandler() {};
	virtual XBOX::VError			GetPatterns (XBOX::VectorOfVString *outPatterns) const;
	virtual XBOX::VError			HandleRequest (IHTTPResponse *ioResponse);
};
#endif	// WITH_DETACH_ENDPOINT_REQUEST_HANDLER_TEST


#endif	// __DEBUG_INFOS_HTTP_REQUEST_HANDLER_INCLUDED__
