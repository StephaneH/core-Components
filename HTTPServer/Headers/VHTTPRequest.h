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
#ifndef __HTTP_REQUEST_INCLUDED__
#define __HTTP_REQUEST_INCLUDED__

#include "VTCPServer.h"
#include "VHTTPMessage.h"
#include "VAuthenticationManager.h"


class VHTTPServer;
class VAuthenticationInfos;


class VHTTPRequest : public VHTTPMessage, public IHTTPRequest
{
	friend class VHTTPResponse;
	typedef VHTTPMessage inherited;

public:
									VHTTPRequest();
	virtual							~VHTTPRequest();

	void							Reset();

	XBOX::VError					ReadFromEndPoint (XBOX::VTCPEndPoint& inEndPoint, uLONG inTimeout = 0);
	XBOX::VError					ReadFromStream (XBOX::VStream& inStream);

	HTTPRequestMethod				GetRequestMethod() const { return fRequestMethod; }
	void							GetRequestMethodString (XBOX::VString& outMethodString) const;
	const XBOX::VString&			GetURL() const { return fURL; }				// Decoded URL
	const XBOX::VString&			GetRawURL() const { return fRawURL; }		// Raw URL (before url decoding)
	const XBOX::VString&			GetURLPath() const { return fURLPath; }		// Path part of the URL "/path/file.html"
	const XBOX::VString&			GetURLQuery() const { return fURLQuery; }	// Query Part of the URL "?param1=1&param2=2"
	const XBOX::VString&			GetHost() const { return fHost; }
	HTTPVersion						GetHTTPVersion() const { return fHTTPVersion; }
	void							GetRequestHTTPVersionString (XBOX::VString& outVersionString) const;
	const XBOX::VHTTPHeader&		GetHTTPHeaders() const { return GetHeaders(); }
	const XBOX::VPtrStream&			GetRequestBody() const { return GetBody(); }
	IAuthenticationInfos *			GetAuthenticationInfos();
	IAuthenticationInfos *			GetAuthenticationInfos() const;
	const XBOX::VString&			GetRequestLine() const { return fRequestLine; }
	void							GetContentTypeHeader (XBOX::VString& outContentType, XBOX::CharSet *outCharSet = NULL) const;
	MimeTypeKind					GetContentTypeKind() const;
	bool							GetCookies (XBOX::VectorOfCookie& outCookies) const;
	bool							GetCookie (const XBOX::VString& inName, XBOX::VString& outValue) const;
	const XBOX::VError				GetParsingError() const { return fParsingError; }
	bool							IsParsingComplete() const { return (PS_ParsingFinished == fParsingState); }
	const XBOX::VMIMEMessage *		GetHTMLForm() const;

	static XBOX::VSize				GetMaxIncomingDataSize() { return fMaxIncomingDataSize; }
	static void						SetMaxIncomingDataSize (XBOX::VSize inValue) { fMaxIncomingDataSize = inValue; }

	XBOX::VString					GetLocalIP() const { return fLocalAddress.GetIP(); }
	PortNumber						GetLocalPort() const { return fLocalAddress.GetPort(); }
	bool							IsSSL() const { return fIsSSL; }

	XBOX::VString					GetPeerIP() const { return fPeerAddress.GetIP(); }
	PortNumber						GetPeerPort() const { return fPeerAddress.GetPort(); }

private:
	HTTPRequestMethod				fRequestMethod;
	XBOX::VString					fRequestLine;
	XBOX::VString					fURL;
	XBOX::VString					fRawURL;		// Encoded URL
	XBOX::VString					fURLPath;
	XBOX::VString					fURLQuery;
	XBOX::VString					fHost;
	HTTPVersion						fHTTPVersion;
	mutable VAuthenticationInfos *	fAuthenticationInfos;
	HTTPParsingState				fParsingState;
	XBOX::VError					fParsingError;
	mutable XBOX::VMIMEMessage *	fHTMLForm;

	XBOX::XNetAddr					fLocalAddress;
	XBOX::XNetAddr					fPeerAddress;
	bool							fIsSSL;
	
	static XBOX::VSize				fMaxIncomingDataSize;

protected:
	bool							_ExtractAuthenticationInfos ();
	bool							_AcceptIncomingDataSize (XBOX::VSize inSize);
};


#endif	// __HTTP_REQUEST_INCLUDED__