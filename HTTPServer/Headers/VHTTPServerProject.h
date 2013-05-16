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
#ifndef __HTTP_SERVER_PROJECT_INCLUDED__
#define __HTTP_SERVER_PROJECT_INCLUDED__


typedef std::map<XBOX::VRefPtr<XBOX::VRegexMatcher>, XBOX::VRefPtr<IHTTPRequestHandler> >		VRegexMatcherCache;
typedef std::map<XBOX::VRefPtr<XBOX::VRegexMatcher>, XBOX::VRefPtr<IPreProcessingHandler> >		VPreProcessingRegExCache;
typedef std::map<XBOX::VRefPtr<XBOX::VRegexMatcher>, XBOX::VRefPtr<IPostProcessingHandler> >	VPostProcessingRegExCache;
typedef std::vector<XBOX::VRefPtr<IHTTPRequestHandler> >										VectorOfRequestHandler;

class VVirtualHost;
class VHTTPServer;
class VAuthenticationManager;
class VHTTPServerProjectSettings;

	
class VHTTPServerProject: public XBOX::VObject, public IHTTPServerProject
{
public:
											VHTTPServerProject (VHTTPServer *inServer, const XBOX::VValueBag *inSettings, const XBOX::VString& inProjectFolderPath);
	virtual									~VHTTPServerProject();

	virtual XBOX::VError					StartProcessing();
	virtual bool							IsProcessing();
	virtual XBOX::VError					StopProcessing();
	
	virtual void							Clear();

	/* Deal with opened socket */
	virtual XBOX::VError					SetListeningSocketDescriptor (sLONG inSocketDescriptor) { fSocketDescriptor = inSocketDescriptor; return XBOX::VE_OK; }
	virtual XBOX::VError					SetListeningSSLSocketDescriptor (sLONG inSSLSocketDescriptor) { fSSLSocketDescriptor = inSSLSocketDescriptor; return XBOX::VE_OK; }

	virtual void							SetReuseAddressSocketOption (bool inValue) { fReuseAddressSocketOption = inValue; }

	virtual IHTTPServerProjectSettings *	GetSettings() const;
	VHTTPServer *							GetHTTPServer() const { return fHTTPServer; }

	/* Deal with custom HTTP Request Handlers */
	virtual XBOX::VError					AddHTTPRequestHandler (IHTTPRequestHandler *inRequestHandler);
	virtual XBOX::VError					RemoveHTTPRequestHandler (IHTTPRequestHandler *inRequestHandler);
	virtual IHTTPRequestHandler *			RetainHTTPRequestHandlerByPrivateData (const void* inPrivateData);

	/* Default Request Handler */
	virtual IHTTPRequestHandler *			GetDefaultRequestHandler() const;

	/* Deal with custom PreProcessingHandlers & PostProcessingHandlers */
	virtual XBOX::VError					AddPreProcessingHandler (IPreProcessingHandler *inPreProcessingHandler);
	virtual XBOX::VError					RemovePreProcessingHandler (IPreProcessingHandler *inPreProcessingHandler);
	virtual IPreProcessingHandler *			RetainPreProcessingHandler (const XBOX::VString& inURLString);
	virtual XBOX::VError					AddPostProcessingHandler (IPostProcessingHandler *inPostProcessingHandler);
	virtual XBOX::VError					RemovePostProcessingHandler (IPostProcessingHandler *inPostProcessingHandler);
	virtual IPostProcessingHandler *		RetainPostProcessingHandler (const XBOX::VString& inURLString);

	virtual void							SetErrorProcessingHandler (IErrorProcessingHandler *inErrorProcessingHandler);
	virtual IErrorProcessingHandler *		GetErrorProcessingHandler();

	virtual void							SetAuthenticationDelegate (IAuthenticationDelegate *inAuthenticationDelegate);
	virtual IAuthenticationDelegate *		GetAuthenticationDelegate();

	/* Deal with static pages service request handler */
	virtual void							EnableStaticPagesService (bool inValue);
	virtual bool							GetEnableStaticPagesService();

	virtual uLONG8							GetHitsCount();
	virtual XBOX::VTaskID					GetTaskID() { return fServerTaskID; }
	virtual XBOX::VTime						GetStartingTime() { return fStartingTime; }

	/* Support External folders */
	virtual XBOX::VError					AddVirtualFolder (const XBOX::VString& inLocationPath, const XBOX::VString& inIndexFileName, const XBOX::VString& inMatchingKeyWord);

	void									SetAcceptIncommingRequests (bool inValue) { fAcceptIncommingRequests = inValue; }
	bool									GetAcceptIncommingRequests() const { return fAcceptIncommingRequests; }

	IHTTPRequestHandler *					RetainMatchingHTTPRequestHandler (const XBOX::VString& inURL);
	IHTTPRequestHandler *					RetainHTTPRequestHandlerMatchingPattern (const XBOX::VString& inPattern);

	XBOX::VSignalT_0 *						GetSignal_DidStart() { return &fDidStartSignal; }
	XBOX::VSignalT_0 *						GetSignal_DidStop() { return &fDidStopSignal; }

	uLONG8									IncreaseHitsCount();

	void									RegisterRequest (IHTTPResponse *inRequest);
	void									UnregisterRequest (IHTTPResponse *inRequest);
	sLONG									GetRunningRequestsCount ();

	void									SaveToBag (XBOX::VValueBag& outBag);

	bool									AddVirtualHostFromSettings();

	/* Deal with custom authentication manager */
	virtual void				   			SetAuthenticationManager (IAuthenticationManager *inAuthenticationManager);
	virtual IAuthenticationManager *		GetAuthenticationManager() const;

	virtual void							SetSecurityManager (CSecurityManager* inSecurityManager);
	virtual CSecurityManager *				GetSecurityManager() const { return fSecurityManager; }

	/*	Check Project sanity:
	 *	May throw an VE_HTTP_SERVER_PROJECT_ALREADY_EXIST error when Project already exists 
	 *	or when 2 or many projects use the same hostname with the same port (or SSLPort)
	 */
	virtual XBOX::VError					Validate();

private:
	IHTTPRequestHandler *					fDefaultRequestHandler;
	VectorOfRequestHandler					fRequestHandlers;
	XBOX::VCriticalSection					fRequestHandlersLock;
	VRegexMatcherCache						fRegexCache;
	XBOX::VCriticalSection					fRegexCacheLock;
	VHTTPServerProjectSettings *			fSettings;
	VHTTPServer *							fHTTPServer;
	IConnectionListener *					fConnectionListener;
	bool									fReuseConnectionListener;
	XBOX::VTaskID							fServerTaskID;
	XBOX::VTime								fStartingTime;
	bool									fAcceptIncommingRequests;
	XBOX::VSignalT_0						fDidStartSignal;
	XBOX::VSignalT_0						fDidStopSignal;
	uLONG8									fHitsCount;
	XBOX::VCriticalSection					fHitsCountLock;
	std::vector<IHTTPResponse *>			fRunningRequests;
	XBOX::VCriticalSection					fRunningRequestsLock;
	std::vector<VVirtualHost *>				fVirtualHosts;
	XBOX::VCriticalSection					fVirtualHostsLock;
	IAuthenticationManager *				fAuthenticationManager;
	CSecurityManager *						fSecurityManager;
	VPreProcessingRegExCache				fPreProcessingRegExCache;
	XBOX::VCriticalSection					fPreProcessingRegExCacheLock;
	VPostProcessingRegExCache				fPostProcessingRegExCache;
	XBOX::VCriticalSection					fPostProcessingRegExCacheLock;
	IErrorProcessingHandler *				fErrorProcessingHandler;
	IAuthenticationDelegate *				fAuthenticationDelegate;
	sLONG									fSocketDescriptor;
	sLONG									fSSLSocketDescriptor;

	bool									fReuseAddressSocketOption;

private:
	void									_Tell_DidStart() { fDidStartSignal(); }
	void									_Tell_DidStop() { fDidStopSignal(); }

	XBOX::VError							_BuildRegexMatcher (const XBOX::VString& inPatternString, XBOX::VRegexMatcher **outMatcher);
	void									_ResetListener();

	void									_AddVirtualFoldersFromSettings (VVirtualHost *ioVirtualHost);
};


#endif	// __HTTP_SERVER_PROJECT_INCLUDED__
