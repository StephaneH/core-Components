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
#ifndef __HTTP_SERVER_SESSION_INCLUDED__
#define __HTTP_SERVER_SESSION_INCLUDED__


class VHTTPServerSession : public XBOX::VTCPServerSession
{
public:
									VHTTPServerSession (VHTTPResponse* inEndPoint);
	virtual							~VHTTPServerSession();

	void							Reset();

	XBOX::VTCPEndPoint *			GetEndPoint() { return fHTTPResponse->GetEndPoint(); }

	bool							KeepAlive() const { return fKeepAlive; }
	void							SetKeepAlive (bool inValue) { fKeepAlive = inValue; }

	bool							IsTimedOut() const;
	bool							IsMaxConnectionsReached() const;

	bool							IsClosed() { return (NULL == fHTTPResponse); }
	XBOX::VError					Close();

	XBOX::VError					HandleTRACE();
	XBOX::VError					HandleRequest();

	XBOX::VError					ReplyWithError (XBOX::VError inError);
	void							LogMessage (const XBOX::VString& inMessageString, sLONG inDuration);

private:
	uLONG							fStartTime;
	uLONG							fKeepAliveTimeOut;
	VHTTPResponse *					fHTTPResponse;
	uLONG							fRequestCount;
	uLONG							fMaxRequest;
	bool							fKeepAlive;
};


#endif	// __HTTP_SERVER_SESSION_INCLUDED__