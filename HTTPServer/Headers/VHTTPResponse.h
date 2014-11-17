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
#ifndef __HTTP_RESPONSE_INCLUDED__
#define __HTTP_RESPONSE_INCLUDED__


#include "ServerNet/VServerNet.h"

class VHTTPRequest;
class VHTTPServer;
class VVirtualHost;


class VHTTPResponse : public VHTTPMessage, public IHTTPResponse
{
	typedef VHTTPMessage inherited;

public:
								VHTTPResponse (VHTTPServer * inServer, XBOX::VTCPEndPoint* inEndPoint, XBOX::VTCPSelectIOPool* inIOPool);
	virtual 					~VHTTPResponse();

	void						Reset();

	/* HTTP Request Message manipulation functions */
	XBOX::VError				ReadRequestFromEndPoint (uLONG inTimeout = 0);
	const IHTTPRequest&			GetRequest() const;
	HTTPVersion					GetRequestHTTPVersion() const;
	const XBOX::VHTTPHeader&	GetRequestHeader() const;
	const XBOX::VString&		GetRequestURL() const;
	const XBOX::VString&		GetRequestRawURL() const;
	const XBOX::VString&		GetRequestURLPath() const;
	const XBOX::VString&		GetRequestURLQuery() const;
	HTTPRequestMethod			GetRequestMethod() const;
	void						GetRequestMethodName (XBOX::VString& outMethodName) const;
	void						SetRequestURLPath (const XBOX::VString& inURLPath);

	/* Request Header accessor functions */
	bool						GetIfModifiedSinceHeader (XBOX::VTime& outTime);
	bool						GetIfUnmodifiedSinceHeader (XBOX::VTime& outTime);

	/* HTTP Response Message manipulation functions */
	HTTPStatusCode				GetResponseStatusCode() const { return fResponseStatusCode; }
	void						SetResponseStatusCode (HTTPStatusCode inValue) { fResponseStatusCode = inValue; }

	XBOX::VPtrStream&			GetResponseBody() { return GetBody(); }
	XBOX::VError				SetResponseBody (const void *inData, XBOX::VSize inDataSize);
	XBOX::VError				SetFileToSend (XBOX::VFile *inFileToSend);	// Used to send larges files with chunked buffer
	bool						HasFileToSend();

	/* Response Header manipulation functions */
	void						SetHTTPVersion (HTTPVersion inValue) { fHTTPVersion = inValue; }
	HTTPVersion					GetHTTPVersion() const { return fHTTPVersion; }
	bool						AddResponseHeader (const HTTPCommonHeaderCode inCode, const XBOX::VString& inValue, bool inOverride = true);
	bool						AddResponseHeader (const XBOX::VString& inName, const XBOX::VString& inValue, bool inOverride = true);
	bool						AddResponseHeader (const XBOX::VString& inName, sLONG inValue, bool inOverride = true);
	bool						AddResponseHeader (const XBOX::VString& inName, const XBOX::VTime& inValue, bool inOverride = true);
	bool						SetContentLengthHeader (const sLONG8 inValue);
	bool						SetExpiresHeader (const sLONG inValue);
	bool						SetExpiresHeader (const XBOX::VTime& inValue);
	bool						IsResponseHeaderSet (const HTTPCommonHeaderCode inCode) const;
	bool						IsResponseHeaderSet (const XBOX::VString& inName) const;
	bool						GetResponseHeader (const HTTPCommonHeaderCode inCode, XBOX::VString& outValue) const;
	bool						GetResponseHeader (const XBOX::VString& inName, XBOX::VString& outValue) const;
	bool						SetContentTypeHeader (const XBOX::VString& inValue, const XBOX::CharSet inCharSet = XBOX::VTC_UNKNOWN);
	bool						GetContentTypeHeader (XBOX::VString& outValue, XBOX::CharSet *outCharSet = NULL) const;
	MimeTypeKind				GetContentTypeKind() const;
	const XBOX::VHTTPHeader&	GetResponseHeader() const { return GetHeaders(); }
	XBOX::VHTTPHeader&			GetResponseHeader() { return GetHeaders(); }

	/* Cookies manipulation functions */
	bool						AddCookie (	const XBOX::VString& inName,
											const XBOX::VString& inValue,
											const XBOX::VString& inComment,
											const XBOX::VString& inDomain,
											const XBOX::VString& inPath,
											bool inSecure,
											bool inHTTPOnly,
											sLONG inMaxAge,
											bool inAlwaysUseExpires = true);

	/* Cookies manipulations functions */
	bool						IsCookieSet (const XBOX::VString& inName) const;
	bool						IsCookieSet (const VHTTPCookie& inCookie) const;
	bool						GetCookie (const XBOX::VString& inName, XBOX::VString& outValue) const;
	bool						SetCookie (const XBOX::VString& inName, const XBOX::VString& inValue);
	bool						DropCookie (const XBOX::VString& inName);

	void						SetCacheBodyMessage (bool inValue) { fCanCacheBody = inValue; }
	bool						CanCacheBodyMessage() const { return fCanCacheBody; }

	void						AllowCompression (bool inValue, sLONG inMinThreshold = -1, sLONG inMaxThreshold = -1);
	bool						CompressionAllowed() const { return fCanCompressBody; }

	HTTPCompressionMethod		GetCompressionMethod() const { return fCompressionMode; }

	XBOX::VError				SendResponse();
	XBOX::VError				ReplyWithStatusCode (HTTPStatusCode inValue, XBOX::VString *inExplanationString = NULL);

	/* Send chunked data */
	XBOX::VError				SendData (void *inData, XBOX::VSize inDataSize, bool isChunked);

	/* Normalize and Send Header only - Thou SHALL not use that function, for specific uses (WebSockets) only */
	XBOX::VError				SendResponseHeader();

	void						GetIP (XBOX::VString& outIP) const;
	
	XBOX::VTCPEndPoint *		GetEndPoint() const { return fEndPoint; }
	XBOX::VTCPEndPoint *		DetachEndPoint();
	bool						IsValidEndPoint() const { return (fEndPoint != NULL); }

	VHTTPServer *				GetHTTPServer() const { return fHTTPServer; }
	IVirtualHost *				GetVirtualHost();

	bool						IsSSL() const;

	void						SetWantedAuthMethod (HTTPAuthenticationMethod inValue) { fWantedAuthMethod = inValue; }
	HTTPAuthenticationMethod 	GetWantedAuthMethod() const { return fWantedAuthMethod; }

	void						SetWantedAuthRealm (const XBOX::VString& inValue);
	void						GetWantedAuthRealm (XBOX::VString& outValue) const;

	uLONG						GetStartRequestTime() const { return fStartRequestTime; }
	sLONG						GetRawSocket() const;

	bool						GetForceCloseSession() const { return fForceCloseSession; }
	void						SetForceCloseSession (bool inValue = true) { fForceCloseSession = inValue; }

	bool						GetUseDefaultCharset() const { return fUseDefaultCharset; }
	void						SetUseDefaultCharset (bool inValue) { fUseDefaultCharset = inValue; }

	sLONG						GetNumOfChunksSent() const { return fNumOfChunkSent; }

	void						AnswerIt() { fAnswered = true; }
	bool						Answered() const { return fAnswered; }

private:
	uLONG						fStartRequestTime;
	VHTTPServer *				fHTTPServer;
	XBOX::VTCPEndPoint *		fEndPoint;
	VHTTPRequest *				fRequest;
	HTTPStatusCode				fResponseStatusCode;
	HTTPVersion					fHTTPVersion;
	bool						fCanCacheBody;
	bool						fCanCompressBody;
	HTTPCompressionMethod		fCompressionMode; // Compression Method Used for body when applicable
	HTTPAuthenticationMethod	fWantedAuthMethod;
	XBOX::VFile *				fFileToSend;
	/* Handle Chunked Responses */
	bool						fIsChunked;
	sLONG						fNumOfChunkSent;
	/* Compression Thresholds */
	sLONG						fMinCompressionThreshold;
	sLONG						fMaxCompressionThreshold;

	bool						fHeaderSent;
	bool						fForceCloseSession;	// To avoid waiting Keep-Alive timeout when HTTP Server is shutting down
	bool						fUseDefaultCharset;
	XBOX::CharSet				fDefaultCharset;
	bool						fBodyCharsetInited;

	bool						fAnswered;

private:
	/* private functions */
	bool						_NormalizeResponseHeader();
	XBOX::VError				_SendResponseHeader();
	XBOX::VError				_SendResponseBody();
	XBOX::VError				_SendResponseWithStatusCode (HTTPStatusCode inStatusCode);
	XBOX::VError				_WriteChunkSize (XBOX::VSize inChunkSize);
	XBOX::VError				_WriteChunkSize (XBOX::VSize inChunkSize, XBOX::VStream& ioStream);
	XBOX::VError				_WriteChunk (void *inDataPtr, XBOX::VSize inDataSize);
	XBOX::VError				_WriteToSocket (void *inBuffer, uLONG *ioBytes);
	bool						_UpdateRequestURL (const XBOX::VString& inProjectPattern);

#if HTTP_SERVER_USE_CUSTOM_ERROR_PAGE
	bool						_ReplyWithCustomErrorPage();
#endif

	/*	Automatically compress Response Body-Message when applicable
	 *	(size > compressionMinThreshold && size < compressionMaxThreshold && mime-type allows compression)
	 */
	XBOX::VError				_CompressData(); 
};


#endif // __HTTP_RESPONSE_INCLUDED__
