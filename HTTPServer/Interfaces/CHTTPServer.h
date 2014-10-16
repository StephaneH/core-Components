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
#ifndef __HTTP_SERVER_COMPONENT_INCLUDED__
#define __HTTP_SERVER_COMPONENT_INCLUDED__

#include "KernelIPC/Sources/VComponentManager.h"
#include "ServerNet/VServerNet.h"
#include "../Headers/HTTPPublicTypes.h"
#include "../Headers/HTTPErrors.h"
#include "Security Manager/Interfaces/CSecurityManager.h"
#include "XML/Sources/VLocalizationManager.h"

#ifndef LOG_IN_CONSOLE
	#define LOG_IN_CONSOLE							0
#endif
#ifndef HTTP_SERVER_VERBOSE_MODE
	#define HTTP_SERVER_VERBOSE_MODE				0	//VERSIONDEBUG
#endif
#ifndef HTTP_SERVER_DETAILED_OPERATIONS_TIMES
	#define HTTP_SERVER_DETAILED_OPERATIONS_TIMES	0	//VERSIONDEBUG
#endif
#ifndef HTTP_SERVER_USE_SYSTEM_NOTIFICATIONS						/* Uses file system notifications in order to invalidate cached objects */
	#define HTTP_SERVER_USE_SYSTEM_NOTIFICATIONS	0
#endif
#ifndef HTTP_SERVER_USE_PROJECT_PATTERNS
	#define HTTP_SERVER_USE_PROJECT_PATTERNS		0
#endif
#ifndef HTTP_SERVER_HANDLE_PARTIAL_CONTENT
	#define HTTP_SERVER_HANDLE_PARTIAL_CONTENT		1
#endif

#ifndef HTTP_SERVER_USE_CUSTOM_ERROR_PAGE
	#define HTTP_SERVER_USE_CUSTOM_ERROR_PAGE		1
#endif

class IVirtualHost;
class IHTTPResponse;


class IAuthenticationInfos
{
public:
	virtual HTTPAuthenticationMethod		GetAuthenticationMethod() const = 0;
	virtual void							GetUserName (XBOX::VString& outUserName) const = 0;
	virtual void							GetPassword (XBOX::VString& outPassword) const = 0;
	virtual CUAGSession*					GetUAGSession() const  = 0;
	virtual void							GetRealm (XBOX::VString& outRealm) const = 0;

	/* DIGEST */
	virtual void							GetNonce (XBOX::VString& outValue) const = 0;
	virtual void							GetOpaque (XBOX::VString& outValue) const = 0;
	virtual void							GetCnonce (XBOX::VString& outValue) const = 0;
	virtual void							GetQop (XBOX::VString& outValue) const = 0;
	virtual void							GetNonceCount (XBOX::VString& outValue) const = 0;
	virtual void							GetAlgorithm (XBOX::VString& outValue) const = 0;
	virtual void							GetResponse (XBOX::VString& outValue) const = 0;
	virtual void							GetURI (XBOX::VString& outValue) const = 0;
	virtual void							GetDomain (XBOX::VString& outValue) const = 0;
	virtual bool							NeedAddUAGSession() const = 0;

	virtual	void							SetUAGSession( CUAGSession* inSession) = 0;
};


class IAuthenticationReferee : public XBOX::IRefCountable
{
public:
	/* The bag should contain some "resources" elements */
	virtual XBOX::VError					LoadFromBag (const XBOX::VValueBag *inValueBag) = 0;
	virtual XBOX::VError					LoadFromJSONFile (const XBOX::VFilePath& inFilePath) = 0;
	virtual XBOX::VError					SaveToBag (XBOX::VValueBag *outBag) = 0;
};


class IAuthenticationManager : public XBOX::IRefCountable
{
public:
	virtual XBOX::VError					CheckAndValidateAuthentication (IHTTPResponse *ioResponse) = 0;
	virtual XBOX::VError					SetAuthorizationHeader (IHTTPResponse *ioResponse) = 0;
	virtual IAuthenticationReferee *		GetAuthenticationReferee() = 0;
};


class IHTTPRequest
{
public:
	virtual HTTPRequestMethod				GetRequestMethod() const = 0;
	virtual void							GetRequestMethodString (XBOX::VString& outMethodString) const = 0;
	virtual const XBOX::VString&			GetURL() const  = 0;			// Full decoded URL
	virtual const XBOX::VString&			GetRawURL() const  = 0;			// Raw URL
	virtual const XBOX::VString&			GetURLPath() const  = 0;		// Path part of the URL "/path/file.html"
	virtual const XBOX::VString&			GetURLQuery() const  = 0;		// Query Part of the URL "?param1=1&param2=2"
	virtual const XBOX::VString&			GetHost() const = 0;
	virtual	const XBOX::VHTTPHeader&		GetHTTPHeaders() const = 0;
	virtual HTTPVersion						GetHTTPVersion() const = 0;
	virtual void							GetRequestHTTPVersionString (XBOX::VString& outVersionString) const = 0;
	virtual	const XBOX::VPtrStream&			GetRequestBody() const = 0;
	virtual IAuthenticationInfos *			GetAuthenticationInfos() = 0;
	virtual IAuthenticationInfos *			GetAuthenticationInfos() const = 0;
	virtual	const XBOX::VString&			GetRequestLine() const = 0;
	virtual void							GetContentTypeHeader (XBOX::VString& outContentType, XBOX::CharSet *outCharSet = NULL) const = 0;
	virtual XBOX::MimeTypeKind				GetContentTypeKind() const = 0;
	virtual bool							GetCookies (XBOX::VectorOfCookie& outCookies) const = 0;
	virtual bool							GetCookie (const XBOX::VString& inName, XBOX::VString& outValue) const = 0;
	virtual XBOX::VError					GetParsingError() const = 0 ;
	virtual bool							IsParsingComplete() const = 0;
	virtual const XBOX::VMIMEMessage *		GetHTMLForm() const = 0;
	virtual XBOX::VString					GetLocalIP() const = 0;
	virtual PortNumber						GetLocalPort() const = 0;
	virtual bool							IsSSL() const = 0;
	virtual const XBOX::VString&			GetPeerIP() const = 0;
	virtual PortNumber						GetPeerPort() const = 0;
	virtual IVirtualHost *					GetVirtualHost() = 0;
	virtual XBOX::VJSONObject*				BuildRequestInfo() const = 0;
};


class IHTTPResponse
{
public:
	/* HTTP Request Message manipulation functions */
	virtual	const IHTTPRequest&				GetRequest() const = 0;
	virtual HTTPVersion						GetRequestHTTPVersion() const = 0;
	virtual	const XBOX::VHTTPHeader&		GetRequestHeader() const = 0;
	virtual	const XBOX::VString&			GetRequestURL() const = 0;
	virtual	const XBOX::VString&			GetRequestRawURL() const = 0;
	virtual	HTTPRequestMethod				GetRequestMethod() const = 0;
	virtual	void							GetRequestMethodName (XBOX::VString& outMethodName) const = 0;
	virtual void							SetRequestURLPath (const XBOX::VString& inURLPath) = 0;

	/* HTTP Response Message manipulation functions */
	virtual	HTTPStatusCode					GetResponseStatusCode() const = 0;
	virtual	void							SetResponseStatusCode (HTTPStatusCode inValue) = 0;

	virtual	XBOX::VPtrStream&				GetResponseBody() = 0;
	virtual	XBOX::VError					SetResponseBody (const void *inData, XBOX::VSize inDataSize) = 0;

	/* Used to send larges files with chunked buffer */
	virtual XBOX::VError					SetFileToSend (XBOX::VFile *inFileToSend) = 0;
	virtual bool							HasFileToSend() = 0;

	/* Response Header manipulation functions */
	virtual void							SetHTTPVersion (HTTPVersion inValue) = 0;
	virtual	bool							AddResponseHeader (const XBOX::VString& inName, const XBOX::VString& inValue, bool inOverride = true) = 0;
	virtual	bool							AddResponseHeader (const XBOX::HTTPCommonHeaderCode inCode, const XBOX::VString& inValue, bool inOverride = true) = 0;
	virtual	bool							SetContentLengthHeader (const sLONG8 inValue) = 0;
	virtual	bool							SetExpiresHeader (const sLONG inValue) = 0; /* possible values: -1-> Yesterday, 0-> Now, 1-> Tomorrow, 2-> Far Future (10 years) */
	virtual bool							SetExpiresHeader (const XBOX::VTime& inValue) = 0;
	virtual	bool							IsResponseHeaderSet (const XBOX::HTTPCommonHeaderCode inCode) const = 0;
	virtual	bool							IsResponseHeaderSet (const XBOX::VString& inName) const = 0;
	virtual bool							GetResponseHeader (const XBOX::HTTPCommonHeaderCode inCode, XBOX::VString& outValue) const = 0;
	virtual bool							GetResponseHeader (const XBOX::VString& inName, XBOX::VString& outValue) const = 0;
	virtual	bool							SetContentTypeHeader (const XBOX::VString& inValue, const XBOX::CharSet inCharSet = XBOX::VTC_UNKNOWN) = 0;
	virtual bool							GetContentTypeHeader (XBOX::VString& outContentType, XBOX::CharSet *outCharSet = NULL) const = 0;
	virtual XBOX::MimeTypeKind				GetContentTypeKind() const = 0;
	virtual	const XBOX::VHTTPHeader&		GetResponseHeader() const = 0;
	virtual	XBOX::VHTTPHeader&				GetResponseHeader() = 0;

	/* Cookies manipulation functions */
	virtual bool							IsCookieSet (const XBOX::VString& inCookieName) const = 0;
	virtual bool							AddCookie (	const XBOX::VString& inName,
														const XBOX::VString& inValue,
														const XBOX::VString& inComment,
														const XBOX::VString& inDomain,
														const XBOX::VString& inPath,
														bool inSecure,
														bool inHTTPOnly,
														sLONG inMaxAge,
														bool inAlwaysUseExpires = true) = 0;
	virtual bool							DropCookie (const XBOX::VString& inCookieName) = 0;

	/* Tell if body can be put in cache */
	virtual void							SetCacheBodyMessage (bool inValue) = 0;
	virtual bool							CanCacheBodyMessage() const = 0;

	/*  Tell if body can be compressed (only if HTTP client accept encoding,
	 *	if content/type accept encoding (See MimeTypeOptions in VMimeType.h)
	 *	and if body size > the minimal threshold (1024 bytes by default) && body size < maximal threshold (1000000 bytes by default) )
	 *	inMinThreshol & inMaxThreshold changes default values temporarily for the current Response (-1: means "use default value")
	 */
	virtual void							AllowCompression (bool inValue, sLONG inMinThreshold = -1, sLONG inMaxThreshold = -1) = 0;
	virtual bool							CompressionAllowed() const = 0;

	/*	Send an automatic response using a standard status code
	 *	for example if a resource is not found, simply write
	 *	httpResponse->ReplyWithStatusCode(HTTP_NOT_FOUND);
	 */
	virtual XBOX::VError					ReplyWithStatusCode (HTTPStatusCode inValue, XBOX::VString *inExplanationString = NULL) = 0;

	/*
	 *	Send data in chunked when applicable
	 */
	virtual XBOX::VError					SendData (void *inData, XBOX::VSize inDataSize, bool isChunked) = 0;

	/*
	 *	Normalize and Send Response Header.
	 *	Headers are automatically sent, so you don't have to call that function. Just for some specific uses !!!
	 */
	virtual XBOX::VError					SendResponseHeader() = 0;

	virtual void							GetIP (XBOX::VString& outIP) const = 0;
	
	virtual XBOX::VTCPEndPoint *			GetEndPoint() const = 0;
	virtual XBOX::VTCPEndPoint *			DetachEndPoint() = 0;
	virtual sLONG							GetRawSocket() const = 0;

	virtual uLONG							GetStartRequestTime() const = 0;

	virtual IVirtualHost *					GetVirtualHost() = 0;

	virtual bool							IsSSL() const = 0;

	/*
	 *	Authentication stuffs
	 */
	virtual void							SetWantedAuthMethod (HTTPAuthenticationMethod inValue) = 0;
	virtual HTTPAuthenticationMethod 		GetWantedAuthMethod() const = 0;

	virtual void							SetWantedAuthRealm (const XBOX::VString& inValue) = 0;
	virtual void							GetWantedAuthRealm (XBOX::VString& outValue) const = 0;

	/*
	 *	Set ForceCloseSession flag to avoid waiting Keep-Alive timeout when HTTP Server is shutting down 
	 */
	virtual bool							GetForceCloseSession() const = 0;
	virtual void							SetForceCloseSession (bool inValue = true) = 0;

	/*
	 *	Set SetUseDefaultCharset to prevent the charset parameter to be automatically set 
	 *	in Content-Type response header (typically for static resources)
	 */
	virtual bool							GetUseDefaultCharset() const = 0;
	virtual void							SetUseDefaultCharset (bool inValue) = 0;

	virtual sLONG							GetNumOfChunksSent() const = 0;
};


class IHTTPRequestHandler : public XBOX::IRefCountable
{
public:
	/*
	 *	Return the list of the RegEx patterns handled by the IHTTPRequestHandler
	 */
	virtual XBOX::VError					GetPatterns (XBOX::VectorOfVString *outPatterns) const = 0;
	virtual XBOX::VError					HandleRequest (IHTTPResponse *ioResponse) = 0;

	virtual bool							GetEnable() { return true; }
	virtual void							SetEnable (bool /*inValue */) {;}
	virtual	void *							GetPrivateData() const { return NULL; }
};


class IWebSocketHandler : public XBOX::IRefCountable
{
public:

	virtual XBOX::VError					GetPatterns (XBOX::VectorOfVString *outPatterns) const = 0;
	virtual XBOX::VError					HandleRequest (IHTTPResponse *ioResponse) = 0;

	/*
	 *	Enable / Disable WebSocketHandler
	 */
	virtual bool							GetEnable () const				{	return true;	}
	virtual void							SetEnable (bool inYesNo)		{	;				}
};


/*
 *	IPreProcessingHandler & IPostProcessingHandler are interfaces designed to be called respectively before & after IHTTPRequestHandler::HandlerRequest
 */
class IPreProcessingHandler : public XBOX::IRefCountable
{
public:
	virtual XBOX::VError					GetPatterns (XBOX::VectorOfVString *outPatterns) const = 0;
	virtual XBOX::VError					HandleRequest (IHTTPResponse *ioResponse) = 0;
};


class IPostProcessingHandler : public XBOX::IRefCountable
{
public:
	virtual XBOX::VError					GetPatterns (XBOX::VectorOfVString *outPatterns) const = 0;
	virtual XBOX::VError					HandleRequest (IHTTPResponse *ioResponse) = 0;
};


/*
 *	IErrorProcessingHandler is an interface designed to be called when IHTTPRequestHandler::HandlerRequest return an error
 */
class IErrorProcessingHandler : public XBOX::IRefCountable
{
public:
	virtual XBOX::VError					ProcessError (XBOX::VError& inError, IHTTPRequestHandler * inHTTPRequestHandler, IHTTPResponse *ioResponse) = 0;
};



namespace xbox
{
	class VJSGlobalContext;
};



class IAuthenticationDelegate : public XBOX::IRefCountable
{
public:
	virtual	XBOX::VJSGlobalContext*			RetainJSContext( XBOX::VError* outError, bool inReusable, const IHTTPRequest* inRequest) = 0;
	virtual	XBOX::VError					ReleaseJSContext( XBOX::VJSGlobalContext* inContext) = 0;

	virtual CUAGSession *					CopyUAGSession (const XBOX::VString& inUAGSessionID) = 0;
	virtual void							SetUAGSessionAndCookie (IHTTPResponse *ioResponse) = 0; // CUAGSession can be retrieved with ioResponse->GetRequest().GetAuthenticationInfos()->GetUAGSession(), 
};


class IHTTPServerProjectSettings : public XBOX::IRefCountable
{
public:
	/* Configuration settings accessors */
	virtual const XBOX::VString&			GetListeningAddress() const = 0;
	
	virtual PortNumber						GetListeningPort() const = 0;
	virtual bool							GetAllowSSL() const = 0;
	virtual bool							GetSSLMandatory() const = 0;
	virtual bool							GetAllowHTTPOnLocal() const = 0;
	virtual PortNumber						GetListeningSSLPort() const = 0;
	virtual const XBOX::VFilePath&			GetSSLCertificatesFolderPath() const  = 0;
	virtual const XBOX::VFilePath&			GetWebFolderPath() const = 0;
	virtual const XBOX::VString&			GetIndexPageName() const = 0;
	virtual XBOX::CharSet					GetDefaultCharSet() const = 0;
	virtual const XBOX::VString&			GetHostName() const = 0;
#if HTTP_SERVER_USE_PROJECT_PATTERNS
	virtual const XBOX::VString&			GetProjectPattern() const = 0;
#endif
	virtual const XBOX::VString&			GetDefaultRealm() const = 0;
	virtual HTTPAuthenticationMethod		GetDefaultAuthType() const = 0;
	virtual void							SetListeningAddress (const XBOX::VString& inIPAddressString) = 0;
	virtual void							SetListeningPort (PortNumber inValue) = 0;
	virtual void							SetAllowSSL (bool inValue) = 0;
	virtual void							SetSSLMandatory (bool inValue) = 0;
	virtual void							SetAllowHTTPOnLocal (bool inValue) = 0;
	virtual void							SetListeningSSLPort (PortNumber inValue) = 0;
	virtual void							SetSSLCertificatesFolderPath (const XBOX::VFilePath& inValue) = 0;
	virtual void							SetWebFolderPath (const XBOX::VFilePath& inValue) = 0;
	virtual void							SetWebFolderPath (const XBOX::VString& inValue) = 0;
	virtual void							SetIndexPageName (const XBOX::VString& inValue) = 0;
	virtual void							SetDefaultCharSet (const XBOX::CharSet inValue) = 0;
	virtual void							SetHostName (const XBOX::VString& inValue) = 0;
#if HTTP_SERVER_USE_PROJECT_PATTERNS
	virtual void							SetProjectPattern (const XBOX::VString& inValue) = 0;
#endif
	virtual void							SetDefaultRealm (const XBOX::VString& inValue) = 0;
	virtual void							SetDefaultAuthType (HTTPAuthenticationMethod inValue) = 0;
	virtual void							SetDefaultAuthType (const XBOX::VString& inAuthenticationMethodName) = 0;

	/* Cache settings accessors */
	virtual bool							GetEnableCache() const = 0;
	virtual sLONG							GetCacheMaxSize() const = 0;
	virtual sLONG							GetCachedObjectMaxSize() const = 0;

	virtual void							SetEnableCache (bool inValue) = 0;
	virtual void							SetCacheMaxSize (sLONG inValue) = 0;
	virtual void							SetCachedObjectMaxSize (sLONG inValue) = 0;

	/* Compression settings */
	virtual bool							GetEnableCompression() const = 0;
	virtual sLONG							GetCompressionMinThreshold() const = 0;
	virtual sLONG							GetCompressionMaxThreshold() const = 0;

	virtual void							SetEnableCompression (bool inValue) = 0;
	virtual void							SetCompressionMinThreshold (sLONG inValue) = 0;
	virtual void							SetCompressionMaxThreshold (sLONG inValue) = 0;

	/* Keep-Alive Settings */
	virtual bool							GetEnableKeepAlive() const = 0;
	virtual sLONG							GetKeepAliveTimeout() const = 0;
	virtual sLONG							GetKeepAliveMaxConnections() const = 0;

	virtual void							SetEnableKeepAlive (bool inValue) = 0;
	virtual void							SetKeepAliveTimeout (sLONG inValue) = 0;
	virtual void							SetKeepAliveMaxConnections (sLONG inValue) = 0;

	/* Max Incoming Data Settings */
	virtual XBOX::VSize						GetMaxIncomingDataSize() const = 0;
	virtual void							SetMaxIncomingDataSize (XBOX::VSize inSize) = 0;

	/* Log settings */
	virtual sLONG							GetLogFormat() const = 0;
	virtual const XBOX::VFilePath&			GetLogFolderPath() const = 0;
	virtual const XBOX::VString&			GetLogFileName() const = 0;
	virtual const XBOX::VString&			GetLogBackupFolderName() const = 0;

	virtual void							SetLogFormat (sLONG inValue, VectorOfLogToken *inLogTokens = NULL) = 0;
	virtual void							SetLogFormat (const XBOX::VString& inLogFormatName, VectorOfLogToken *inLogTokens = NULL) = 0;
	virtual	void							SetLogFormat (const XBOX::VString& inLogFormatName, const XBOX::VectorOfVString& inLogTokensNames) = 0;
	virtual void							SetLogFolderPath (const XBOX::VFilePath& inValue) = 0;
	virtual void							SetLogFileName (const XBOX::VString& inValue) = 0;
	virtual void							SetLogBackupFolderName (const XBOX::VString& inValue) = 0;

	/* Log Rotation Setting */
	virtual bool							RotateOnSchedule() const = 0;

	virtual ELogRotationMode				GetLogRotationMode() const = 0;
	virtual sLONG							GetLogMaxSize() const = 0;
	virtual sLONG							GetFrequency() const = 0;
	virtual sLONG							GetStartingTime() const = 0;
	virtual const std::map<sLONG, sLONG>&	GetDaysHoursMap() const = 0;

	virtual void							SetLogRotationMode (ELogRotationMode inValue) = 0;
	virtual void							SetLogMaxSize (sLONG inValue) = 0;
	virtual void							SetFrequency (sLONG inValue) = 0;
	virtual void							SetStartingTime (sLONG inValue) = 0;
	virtual void							SetDaysHoursMap (const std::map<sLONG, sLONG>& inValue) = 0;

	/* Resources Settings */
	virtual	void							LoadResourcesSettingsFromBag( const XBOX::VValueBag& inBag) = 0;
	virtual	void							LoadResourcesSettingsFromJSONFile (const XBOX::VFilePath& inFilePath) = 0;

	/* Virtual Folders settings */
	virtual	void							LoadVirtualFoldersSettingsFromBag( const XBOX::VValueBag& inBag) = 0;
	virtual void							LoadVirtualFoldersSettingsFromJSONFile (const XBOX::VFilePath& inFilePath) = 0;
};


class IHTTPServerProject : public XBOX::IRefCountable
{
public:
	virtual XBOX::VError					StartProcessing() = 0;
	virtual bool							IsProcessing() = 0;
	virtual XBOX::VError					StopProcessing() = 0;

	virtual void							Clear() = 0;
	
	/* Deal with opened socket descriptors (ie for publishing sockets already opened by AuthorizationHelpers with permissions. See AuthorizationHelpers.h) */
	virtual XBOX::VError					SetListeningSocketDescriptor (sLONG inSocketDescriptor) = 0;
	virtual XBOX::VError					SetListeningSSLSocketDescriptor (sLONG inSSLSocketDescriptor) = 0;

	virtual void							SetReuseAddressSocketOption (bool inValue) = 0;

	virtual IHTTPServerProjectSettings *	GetSettings() const = 0;

	/* Deal with custom HTTP Request Handlers */
	virtual XBOX::VError					AddHTTPRequestHandler (IHTTPRequestHandler *inRequestHandler) = 0;
	virtual XBOX::VError					RemoveHTTPRequestHandler (IHTTPRequestHandler *inRequestHandler) = 0;
	virtual IHTTPRequestHandler *			RetainHTTPRequestHandlerByPrivateData (const void *inPrivateData) = 0;
	virtual IHTTPRequestHandler *			RetainMatchingHTTPRequestHandler (const XBOX::VString& inURL) = 0;
	virtual IHTTPRequestHandler *			RetainHTTPRequestHandlerMatchingPattern (const XBOX::VString& inPattern) = 0;

	/* Deal with WebSocketHandler */
	virtual XBOX::VError					AddWebSocketHandler (IWebSocketHandler *inWebSocketHandler) = 0;
	virtual XBOX::VError					RemoveWebSocketHandler (IWebSocketHandler *inWebSocketHandler) = 0;
	virtual IWebSocketHandler				*RetainMatchingWebSocketHandler (const XBOX::VString &inPath) = 0;

	/* Default Request Handler */
	virtual IHTTPRequestHandler *			GetDefaultRequestHandler() const = 0;

	/* Deal with custom PreProcessingHandlers & PostProcessingHandlers */
	virtual XBOX::VError					AddPreProcessingHandler (IPreProcessingHandler *inPreProcessingHandler) = 0;
	virtual XBOX::VError					RemovePreProcessingHandler (IPreProcessingHandler *inPreProcessingHandler) = 0;
	virtual IPreProcessingHandler *			RetainPreProcessingHandler (const XBOX::VString& inURLString) = 0;
	virtual XBOX::VError					AddPostProcessingHandler (IPostProcessingHandler *inPostProcessingHandler) = 0;
	virtual XBOX::VError					RemovePostProcessingHandler (IPostProcessingHandler *inPostProcessingHandler) = 0;
	virtual IPostProcessingHandler *		RetainPostProcessingHandler (const XBOX::VString& inURLString) = 0;

	virtual void							SetErrorProcessingHandler (IErrorProcessingHandler *inErrorProcessingHandler) = 0;
	virtual IErrorProcessingHandler *		GetErrorProcessingHandler() = 0;

	virtual void							SetAuthenticationDelegate (IAuthenticationDelegate *inAuthenticationDelegate) = 0;
	virtual IAuthenticationDelegate *		GetAuthenticationDelegate() = 0;

	/* Deal with static pages service request handler */
	virtual void							EnableStaticPagesService (bool inValue) = 0;
	virtual bool							GetEnableStaticPagesService() = 0;

	virtual uLONG8							GetHitsCount() = 0;
	virtual XBOX::VTaskID					GetTaskID() = 0;
	virtual XBOX::VTime						GetStartingTime() = 0;
	
	/* Support of External Folder */
	/* inLocationPath can be either a local Folder Path or an URL pointing an external server (i.e "c:\Users\" or "http://mysite.com/") */
	virtual XBOX::VError					AddVirtualFolder (const XBOX::VString& inLocationPath, const XBOX::VString& inIndexFileName, const XBOX::VString& inMatchingKeyWord) = 0;

	virtual void							SetAcceptIncommingRequests (bool inValue) = 0;

	/* Deal with authentication and Security Manager */
	virtual void				   			SetSecurityManager (CSecurityManager *inSecurityManager) = 0;
	virtual CSecurityManager * 				GetSecurityManager() const = 0;

	/* Deal with custom authentication manager */
	virtual void				   			SetAuthenticationManager (IAuthenticationManager *inAuthenticationManager) = 0;
	virtual IAuthenticationManager *		GetAuthenticationManager() const = 0;

	/* Signals */
	virtual XBOX::VSignalT_0 *				GetSignal_DidStart() = 0;
	virtual XBOX::VSignalT_0 *				GetSignal_DidStop() = 0;

	/*	Check Project sanity:
	 *	May throw an VE_HTTP_SERVER_PROJECT_ALREADY_EXIST error when Project already exists 
	 *	or when 2 or many projects use the same hostname with the same port (or SSLPort)
	 */
	virtual XBOX::VError					Validate() = 0;

	/* Make IHTTPServerProject available/unavailable (Client will receive a 503 status code when unavailable) */
	virtual void							MakeAvailable() = 0;
	virtual void							MakeUnavailable() = 0;
	virtual bool							IsAvailable() = 0;
};


class ICacheManager : public XBOX::IRefCountable
{
public:
	virtual void							SetCacheMaxSize (const XBOX::VSize inSize) = 0;
	virtual XBOX::VSize						GetCacheMaxSize() const = 0;

	virtual void							SetCachedObjectMaxSize (const XBOX::VSize inSize) = 0;
	virtual XBOX::VSize						GetCachedObjectMaxSize() const = 0;

	virtual void							SetEnableDataCache (bool inAllowToCacheData) = 0;
	virtual bool							GetEnableDataCache() const = 0;

	virtual void							ClearCache() = 0;

	/*
		sortOption can be:
		-> 0: No sort option
		-> 1: Sort by Loads in cache
		-> 2: Sort by Age in Cache
	*/
	virtual void							ToJSONString (XBOX::VString& outJSONString, const XBOX::VString& inFilter, bool prettyFormatting = false, HTTPServerCacheSortOption sortOption = STATS_SORT_NONE) = 0;
	virtual void							ToXMLString (XBOX::VString& outXMLString, const XBOX::VString& inFilter, bool prettyFormatting = false, HTTPServerCacheSortOption sortOption = STATS_SORT_NONE) = 0;

	/* Used for 4D GET WEB STATISTICS command only */
	virtual void							GetStatistics (std::vector<XBOX::VString>& outPages, std::vector<sLONG>& outHits, sLONG& outUsage) = 0;

	/* The bag should contain some "resources" elements */
	virtual	XBOX::VError					LoadRulesFromBag (const XBOX::VValueBag *inValueBag) = 0;
};


class IVirtualHost : public XBOX::IRefCountable
{
public:
	virtual IHTTPServerProject *			GetProject() const = 0;
	virtual XBOX::VError					GetFilePathFromURL (const XBOX::VString& inURL, XBOX::VString& outLocationPath) = 0;
	virtual XBOX::VError					GetFileContentFromURL (const XBOX::VString& inURL, IHTTPResponse *ioResponse, bool inUseFullPath = false) = 0;
	virtual bool							GetMatchingVirtualFolderInfos (const XBOX::VString& inURL, XBOX::VFilePath& outVirtualFolderPath, XBOX::VString& outDefaultIndexName, XBOX::VString& outVirtualFolderName) = 0;
	virtual bool							ResolveURLForAlternatePlatformPage (const XBOX::VString& inURL, const XBOX::VString& inPlatform, XBOX::VString& outResolvedURL) = 0;

	/* Make VirtualHost available/unavailable (Client will receive a 503 status code when unavailable) */
	virtual void							MakeAvailable() = 0;
	virtual void							MakeUnavailable() = 0;
	virtual bool							IsAvailable() const = 0;
};

class IHTTPWebsocketHandler : public XBOX::IRefCountable
{
public:

	static const sLONG	kREAD_MESSAGE_TIMEOUT	= 1000;

	// Return WebSocket current state.

	virtual XBOX::VWebSocket::STATES	GetState () const;

	// Close the WebSocket (initiate closing handshake), no more transmissions is possible after call.
	// Default reason code is 1000 (normal closure). If the WebSocket is not in OPEN state, then do nothing.

	virtual XBOX::VError				Close (sLONG inReasonCode = 1000);
	
	// Attempt to read ioLength bytes from current frame into inData. ioLength is updated with the number of bytes actually read. 
	// outIsTerminated is set when all the bytes of the current message have been read. No data is available if ioLength is zero.
	// Read time-out can be specified in millisecond(s).

	virtual XBOX::VError				ReadMessage (void *ioData, XBOX::VSize &ioLength, bool &outIsTerminated, sLONG inReadTimeOut = kREAD_MESSAGE_TIMEOUT);

	// Writes inLength bytes into a frame. If inIsTerminated is true, then this frame completes a message.
	
	virtual XBOX::VError				WriteMessage (const void *inData, XBOX::VSize inLength, bool inIsTerminated);

protected:

	XBOX::VWebSocket					*fWebSocket;
	XBOX::VWebSocketMessage				*fWebSocketMessage;
	bool								fMessageStarted;
};

class IHTTPWebsocketClient : public IHTTPWebsocketHandler
{
public:

	virtual	XBOX::VError		ConnectToServer (const XBOX::VURL &inUrl, bool inIsBlocking = false) = 0;
	virtual	XBOX::VString		GetPeerIP () const = 0;
};

class IHTTPWebsocketServer : public IHTTPWebsocketHandler
{
public:

	virtual XBOX::VError		TreatNewConnection(IHTTPResponse* inResponse, bool inDetachEndPoint) = 0;
};

class CHTTPServer : public XBOX::CComponent
{
public:
	enum	{ Component_Type = HTTP_SERVER_COMPONENT_SIGNATURE };

	virtual IHTTPServerProject *			NewHTTPServerProject (const XBOX::VFilePath& inProjectFolderPath, bool inAppend = true) = 0;
	virtual IHTTPServerProject *			NewHTTPServerProject (XBOX::VFileSystemNamespace *inFileSystemNameSpace, bool inAppend = true) = 0;
	virtual XBOX::VError					RemoveHTTPServerProject (IHTTPServerProject *inHTTPServerProject) = 0;
	virtual XBOX::VError					AppendHTTPServerProject (IHTTPServerProject *inHTTPServerProject) = 0;
	virtual IHTTPWebsocketServer*			NewHTTPWebsocketServerHandler() = 0;
	virtual IHTTPWebsocketClient*			NewHTTPWebsocketClientHandler() = 0;
	virtual void							SetRequestLogger (IRequestLogger *inRequestLogger) = 0;

	virtual XBOX::VError					StopConnectionHandler (XBOX::VTaskID nTaskID) = 0;

	virtual ICacheManager *					GetCacheManager() const = 0;

	/* Export some utilities functions from HTTPServerTools.h */
	virtual bool							EqualASCIIVString (const XBOX::VString& inString1, const XBOX::VString& inString2, bool isCaseSensitive = false) = 0;
	virtual sLONG							FindASCIIVString (const XBOX::VString& inText, const XBOX::VString& inPattern, bool isCaseSensitive = false) = 0;

	virtual void							MakeRFC822GMTDateString (const sLONG inMode, XBOX::VString& outDateString,bool withHeaderName = false, sLONG inTimeout = 5 * 60L) = 0;
	virtual void							MakeIPv4String (const uLONG inIPv4, XBOX::VString& outString) = 0;
	virtual void							MakeServerString (XBOX::VString& outServerString, bool inSecureMode = false, bool withHeaderName = false) = 0;

	/* Export some utilities functions from HTTPProtocol.h */
	virtual bool							MakeHTTPAuthenticateHeaderValue (const HTTPAuthenticationMethod inMethod, const XBOX::VString& inRealm, const XBOX::VString& inDomain, XBOX::VString& outAuthString) = 0;
	virtual void							SetServerInformations (const XBOX::VString& inServerName, const XBOX::VString& inServerVersion, bool withExtendedInformations = false) = 0;

	/* Export some functions from VMimeTypeManager */
	virtual void							AppendMimeType (const XBOX::VString& inContentType, const XBOX::VString& inExtensions, bool isCompressible = false, bool isParsable = false) = 0;
	virtual void							FindContentType (const XBOX::VString& inExtension, XBOX::VString& outContentType, bool *outIsCompressible = NULL, bool *outIsParsable = NULL) = 0;
	virtual bool							IsMimeTypeCompressible (const XBOX::VString& inContentType) = 0;
	virtual bool							IsMimeTypeParsable (const XBOX::VString& inContentType) = 0;
	virtual XBOX::MimeTypeKind				GetMimeTypeKind (const XBOX::VString& inContentType) = 0;
};


#endif	// __HTTP_SERVER_COMPONENT_INCLUDED__
