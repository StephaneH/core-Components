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
#include "VStaticPagesHTTPRequestHandler.h"
#include "VDebugInfosHTTPRequestHandler.h"



namespace NSHTTPServerProject
{
	CREATE_BAGKEY (refCount);
	CREATE_BAGKEY (taskID);
	CREATE_BAGKEY (startingTime);
	CREATE_BAGKEY (acceptRequests);
	CREATE_BAGKEY (hitCount);
	CREATE_BAGKEY (runningRequests);
	CREATE_BAGKEY (securityComponentAvailable);
	CREATE_BAGKEY (authenticationManagerAvailable);
	namespace requestHandlers
	{
		CREATE_BAGKEY (refCount);
		CREATE_BAGKEY (patterns);
		CREATE_BAGKEY (enabled);
	}
	CREATE_BAGKEY (settings);
	namespace serverInfos
	{
		CREATE_BAGKEY (refCount);
		CREATE_BAGKEY (zipComponentAvailable);
	}
}


//--------------------------------------------------------------------------------------------------


template <typename Type>
bool ProcessingHandlerExists (std::map<XBOX::VRefPtr<XBOX::VRegexMatcher>, XBOX::VRefPtr<Type> >& ioMap, Type *inProcessHandler)
{
	if ((NULL == inProcessHandler) || (0 == ioMap.size()))
		return false;

	XBOX::VectorOfVString	patterns;

	inProcessHandler->GetPatterns (&patterns);
	for (XBOX::VectorOfVString::iterator it = patterns.begin(); it != patterns.end(); ++it)
	{
		typename std::map<XBOX::VRefPtr<XBOX::VRegexMatcher>, XBOX::VRefPtr<Type> >::iterator found = std::find_if (ioMap.begin(), ioMap.end(), EqualVRegexMatcherFunctor<XBOX::VRefPtr<Type> > (*it));
		if (ioMap.end() != found)
			return true;
	}

	return false;
}


template <typename Type>
XBOX::VError AddProcessingHandler (std::map<XBOX::VRefPtr<XBOX::VRegexMatcher>, XBOX::VRefPtr<Type> >& ioMap, Type *inProcessHandler)
{
	if (NULL == inProcessHandler)
		return VE_HTTP_INVALID_ARGUMENT;

	XBOX::VError error = VE_PROCESSING_HANDLER_ALREADY_EXISTS;

	if (!ProcessingHandlerExists (ioMap, inProcessHandler))
	{
		XBOX::VectorOfVString	patterns;
		XBOX::VRefPtr<Type>		processHandler = inProcessHandler;

		processHandler->GetPatterns (&patterns);
		for (XBOX::VectorOfVString::iterator it = patterns.begin(); it != patterns.end(); ++it)
		{
			error = XBOX::VE_OK;
			XBOX::VRegexMatcher *matcher = XBOX::VRegexMatcher::Create ((*it), &error);

			if (NULL != matcher)
			{
				if (XBOX::VE_OK == error)
				{
					ioMap[matcher] = processHandler.Get();
				}
				else
				{
					XBOX::QuickReleaseRefCountable (matcher);
				}
			}
		}
	}

	return error;
}


template <typename Type>
XBOX::VError RemoveProcessingHandler (std::map<XBOX::VRefPtr<XBOX::VRegexMatcher>, XBOX::VRefPtr<Type> >& ioMap, Type *inProcessHandler)
{
	if (NULL == inProcessHandler)
		return VE_HTTP_INVALID_ARGUMENT;

	XBOX::VError		error = VE_PROCESSING_HANDLER_DOES_NOT_EXIST;
	XBOX::VRefPtr<Type>	handler = inProcessHandler;

	typename std::map<XBOX::VRefPtr<XBOX::VRegexMatcher>, XBOX::VRefPtr<Type> >::iterator it = ioMap.begin();
	typename std::map<XBOX::VRefPtr<XBOX::VRegexMatcher>, XBOX::VRefPtr<Type> >::iterator iter;

	while (it != ioMap.end())
	{
		if ((*it).second.Get() == handler.Get())
		{
			iter = it;
			it++;
			ioMap.erase (iter);
			error = XBOX::VE_OK;
			break;
		}

		++it;
	}

	return error;
}


template <typename Type>
Type *RetainProcessingHandler (std::map<XBOX::VRefPtr<XBOX::VRegexMatcher>, XBOX::VRefPtr<Type> >& inMap, const XBOX::VString& inURLString, bool inContinueSearching = true)
{
	Type *resultHandler = NULL;

	typename std::map<XBOX::VRefPtr<XBOX::VRegexMatcher>, XBOX::VRefPtr<Type> >::iterator it = std::find_if (inMap.begin(), inMap.end(), MatchVRegexMatcherFunctor<XBOX::VRefPtr<Type> > (inURLString, inContinueSearching));
	if (it != inMap.end())
		resultHandler = XBOX::RetainRefCountable ((*it).second.Get());

	return resultHandler;
}


//--------------------------------------------------------------------------------------------------


VHTTPServerProject::VHTTPServerProject (VHTTPServer *inServer, const XBOX::VValueBag *inSettings, const XBOX::VString& inProjectFolderPath)
: fSettings (NULL)
, fHTTPServer (NULL)
, fConnectionListener (NULL)
, fReuseConnectionListener (false)
, fServerTaskID (0)
, fStartingTime (0)
, fDefaultRequestHandler (NULL)
, fAcceptIncommingRequests (false)
, fDidStartSignal (VSignal::ESM_Asynchonous)
, fDidStopSignal (VSignal::ESM_Asynchonous)
, fHitsCount (0)
, fRunningRequests()
, fRunningRequestsLock()
, fVirtualHosts()
, fVirtualHostsLock()
, fSecurityManager (NULL)
, fPreProcessingRegExCache()
, fPreProcessingRegExCacheLock()
, fPostProcessingRegExCache()
, fPostProcessingRegExCacheLock()
, fSocketDescriptor (-1)
, fSSLSocketDescriptor (-1)
, fAuthenticationManager (NULL)
, fErrorProcessingHandler (NULL)
, fAuthenticationDelegate (NULL)
{
	fSettings = new VHTTPServerProjectSettings (inSettings, inProjectFolderPath);
	fHTTPServer = XBOX::RetainRefCountable (inServer);
	fDefaultRequestHandler = new VStaticPagesHTTPRequestHandler();
}


VHTTPServerProject::~VHTTPServerProject()
{
	Clear();

	XBOX::ReleaseRefCountable (&fAuthenticationManager);
	XBOX::ReleaseRefCountable (&fSecurityManager);
	XBOX::ReleaseRefCountable (&fHTTPServer);
	XBOX::ReleaseRefCountable (&fSettings);
	XBOX::ReleaseRefCountable (&fDefaultRequestHandler);
	XBOX::ReleaseRefCountable (&fErrorProcessingHandler);
	XBOX::ReleaseRefCountable (&fAuthenticationDelegate);
}


void VHTTPServerProject::Clear()
{
	_ResetListener();

	fRequestHandlers.clear();
	fRegexCache.clear();

	assert (0 == fRunningRequests.size());
	fRunningRequests.erase (fRunningRequests.begin(), fRunningRequests.end());

	std::vector<VVirtualHost *>::iterator itHosts = fVirtualHosts.begin();
	while (itHosts != fVirtualHosts.end())
	{
		fHTTPServer->GetVirtualHostManager()->RemoveVirtualHost ((*itHosts));
		(*itHosts)->Release();
		++itHosts;
	}

	fVirtualHosts.clear();

	fPreProcessingRegExCache.clear();
	fPostProcessingRegExCache.clear();
}


void VHTTPServerProject::_ResetListener()
{
	if (!fReuseConnectionListener)
	{
		VHTTPConnectionListener * httpCL = dynamic_cast<VHTTPConnectionListener*>(fConnectionListener);
		XBOX::ReleaseRefCountable (&httpCL);
	}

	fConnectionListener = NULL;
}


IHTTPServerProjectSettings *VHTTPServerProject::GetSettings() const
{
	return dynamic_cast<IHTTPServerProjectSettings *>(fSettings);
}


IAuthenticationManager * VHTTPServerProject::GetAuthenticationManager() const
{
	return fAuthenticationManager;
}


void VHTTPServerProject::SetAuthenticationManager (IAuthenticationManager *inAuthenticationManager)
{
	if (NULL != fAuthenticationManager)
		XBOX::QuickReleaseRefCountable (fAuthenticationManager);

	fAuthenticationManager = XBOX::RetainRefCountable (inAuthenticationManager);
}


#if HTTP_SERVER_GLOBAL_SETTINGS
IHTTPServerSettings *VHTTPServerProject::GetHTTPServerSettings() const
{
	if (NULL != fHTTPServer)
		return fHTTPServer->GetSettings();

	return NULL;
}
#endif


XBOX::VError VHTTPServerProject::StartProcessing()
{
#if HTTP_SERVER_GLOBAL_SETTINGS
	if (fHTTPServer->GetSettings()->GetEnableCompression() && !VHTTPServer::GetZipComponentAvailable())
		fHTTPServer->GetSettings()->SetEnableCompression (false);
#else
	if (fSettings->GetEnableCompression() && !VHTTPServer::GetZipComponentAvailable())
		fSettings->SetEnableCompression (false);
#endif

	if (fConnectionListener && fConnectionListener->IsListening())
	{
		XBOX::VTime::Now (fStartingTime);
		_Tell_DidStart();

		return VE_OK;
	}

	fConnectionListener = fHTTPServer->FindConnectionListener (fSettings);
	if (NULL != fConnectionListener)
	{
		fReuseConnectionListener = true;
	}
	else
	{
		fConnectionListener = fHTTPServer->CreateConnectionListener (fSettings);
		fReuseConnectionListener = false;
	}

	if (NULL != fConnectionListener)
		fAcceptIncommingRequests = true;

	AddVirtualHostFromSettings();

	// Install some request Handlers
	IHTTPRequestHandler * handler = new VCacheManagerHTTPRequestHandler();
	if (testAssert (NULL != handler))
		AddHTTPRequestHandler (handler);
	XBOX::ReleaseRefCountable (&handler);

#if 1 /*VERSIONDEBUG*/
	handler = new VDebugInfosHTTPRequestHandler();
	if (testAssert (NULL != handler))
		AddHTTPRequestHandler (handler);
	XBOX::ReleaseRefCountable (&handler);
#endif



	/*handler = new VChromeDebugHandler();
	if (testAssert (NULL != handler))
		AddHTTPRequestHandler (handler);
	XBOX::ReleaseRefCountable (&handler);
*/

	XBOX::VError error = XBOX::VE_OK;	

	if ((NULL != fConnectionListener) && !fConnectionListener->IsListening())
	{
		if (fSocketDescriptor != -1)
			dynamic_cast<VHTTPConnectionListener *>(fConnectionListener)->SetSocketDescriptor (fSocketDescriptor);

		if (fSSLSocketDescriptor != -1)
			dynamic_cast<VHTTPConnectionListener *>(fConnectionListener)->SetSSLSocketDescriptor (fSSLSocketDescriptor);
		
		error = fConnectionListener->StartListening();
	}

	if (XBOX::VE_OK == error)
	{
		XBOX::VTime::Now (fStartingTime);
		fServerTaskID = VTaskMgr::Get()->GetCurrentTaskID();
		_Tell_DidStart();
	}
	else
	{
		fConnectionListener->StopListening();
		fHTTPServer->RemoveConnectionListener (fConnectionListener);

		fHTTPServer->GetVirtualHostManager()->Notify_DidStop();
	}

	return error;
}


bool VHTTPServerProject::IsProcessing()
{
	if (NULL != fConnectionListener)
		return fConnectionListener->IsListening();

	return false;
}


XBOX::VError VHTTPServerProject::StopProcessing()
{
	fAcceptIncommingRequests = false;

	if (NULL == fConnectionListener)
		return XBOX::VE_OK;

	if  (!fConnectionListener->IsListening())
		return XBOX::VE_OK;

	while (fRunningRequests.size())
		XBOX::VTask::GetCurrent()->Sleep (500);

	fHitsCount = 0;
	fServerTaskID = 0;
	fStartingTime.Clear();

	_Tell_DidStop();

	fConnectionListener->StopListening();
	fHTTPServer->RemoveConnectionListener (fConnectionListener);
	_ResetListener();

	fHTTPServer->GetVirtualHostManager()->Notify_DidStop();

	XBOX::VTask::GetCurrent()->Sleep (1000);

	return XBOX::VE_OK;
}


XBOX::VError VHTTPServerProject::AddHTTPRequestHandler (IHTTPRequestHandler *inRequestHandler)
{
	if (NULL == inRequestHandler)
		return VE_HTTP_INVALID_ARGUMENT;

	XBOX::VRefPtr<IHTTPRequestHandler> requestHandler = inRequestHandler;
	XBOX::VTaskLock lock (&fRequestHandlersLock);
	XBOX::VError	error = XBOX::VE_OK;

	VectorOfRequestHandler::iterator	iterHandlers = fRequestHandlers.begin();
	VectorOfRequestHandler::iterator	foundHandler = fRequestHandlers.end();

	while (iterHandlers != fRequestHandlers.end())
	{
		foundHandler = std::find_if (fRequestHandlers.begin(), fRequestHandlers.end(), FindMatchingRequestHandlerFunctor (requestHandler.Get()));
		if (foundHandler != fRequestHandlers.end())
			break;
		++iterHandlers;
	}

	if (foundHandler == fRequestHandlers.end())
	{
		XBOX::VectorOfVString handlerPatterns;

		requestHandler->GetPatterns (&handlerPatterns);
		for (XBOX::VectorOfVString::iterator it = handlerPatterns.begin(); it != handlerPatterns.end(); ++it)
		{
			XBOX::VString			patternString (*it);
			XBOX::VRegexMatcher *	matcher = NULL;
			
			error = _BuildRegexMatcher (*it, &matcher);

			if (NULL != matcher)
			{
				if (XBOX::VE_OK == error)
				{
					fRegexCache[matcher] = requestHandler.Get();
				}
				else
				{
					XBOX::QuickReleaseRefCountable (matcher);
				}
			}
		}

		fRequestHandlers.push_back (requestHandler.Get());
	}
	else
	{
		return VE_HTTP_REQUEST_HANDLER_ALREADY_EXISTS;
	}

	return error;
}


XBOX::VError VHTTPServerProject::RemoveHTTPRequestHandler (IHTTPRequestHandler *inRequestHandler)
{
	if (NULL == inRequestHandler)
		return VE_HTTP_INVALID_ARGUMENT;

	XBOX::VError error = VE_HTTP_REQUEST_HANDLER_DOES_NOT_EXIST;
	XBOX::VRefPtr<IHTTPRequestHandler> requestHandler = inRequestHandler;
	XBOX::VTaskLock lock (&fRequestHandlersLock);

	VectorOfRequestHandler::iterator foundHandler = std::find (fRequestHandlers.begin(), fRequestHandlers.end(), requestHandler);

	if (foundHandler != fRequestHandlers.end())
	{
		VRegexMatcherCache::iterator it = fRegexCache.begin();
		VRegexMatcherCache::iterator iter;
		while (it != fRegexCache.end())
		{
			if ((*it).second.Get() == requestHandler.Get())
			{
				iter = it;
				it++;
				fRegexCache.erase (iter);
				error = XBOX::VE_OK;
			}
			else
			{
				++it;
			}
		}

		fRequestHandlers.erase (foundHandler);
	}

	return error;
}


IHTTPRequestHandler *VHTTPServerProject::RetainHTTPRequestHandlerByPrivateData (const void *inPrivateData)
{
	IHTTPRequestHandler *handler = NULL;

	if (NULL != inPrivateData)
	{
		XBOX::VTaskLock lock (&fRequestHandlersLock);

		VectorOfRequestHandler::iterator iter = fRequestHandlers.begin();
		while ((NULL == handler) && (iter != fRequestHandlers.end()))
		{
			if ((*iter)->GetPrivateData() == inPrivateData)
			{
				handler = XBOX::RetainRefCountable (iter->Get());
				break;
			}
			++iter;
		}
	}

	return handler;
}


IHTTPRequestHandler *VHTTPServerProject::RetainMatchingHTTPRequestHandler (const XBOX::VString& inURL)
{
	IHTTPRequestHandler *resultHandler = NULL;
	XBOX::VTaskLock		lock (&fRegexCacheLock);
	XBOX::VString		urlString (inURL);
/*
	// URL Start with project pattern ?
	if (!fSettings->GetProjectPattern().IsEmpty())
	{
		sLONG pos = HTTPServerTools::FindASCIIVString (urlString, fSettings->GetProjectPattern());
		if (pos == 2) // Takes the starting CHAR_SOLIDUS into account
			inURL.GetSubString (pos + fSettings->GetProjectPattern().GetLength(), inURL.GetLength() - fSettings->GetProjectPattern().GetLength() - 1, urlString);
	}
*/

	resultHandler = RetainProcessingHandler<IHTTPRequestHandler> (fRegexCache, urlString, false);

	/* YT 18-Jan-2012 - ACI0075015, ACI0074936 & ACI0074300
		When no requestHandler match and the URL ends by SOLIDUS '/', try to find a requestHandler matching the URL + 
		the index page name (typically for 4D: /index.shtml have to be handled by a specific requestHandler)
	*/
	if ((NULL == resultHandler) && (urlString.GetUniChar (urlString.GetLength()) == CHAR_SOLIDUS))
	{
		urlString.AppendString (fSettings->GetIndexPageName());

		resultHandler = RetainProcessingHandler<IHTTPRequestHandler> (fRegexCache, urlString, false);
	}

	return resultHandler;
}


IHTTPRequestHandler *VHTTPServerProject::RetainHTTPRequestHandlerMatchingPattern (const XBOX::VString& inPattern)
{
	IHTTPRequestHandler *	resultHandler = NULL;
	VTaskLock				lock (&fRegexCacheLock);

	VRegexMatcherCache::iterator it = std::find_if (fRegexCache.begin(), fRegexCache.end(), EqualVRegexMatcherFunctor<XBOX::VRefPtr<IHTTPRequestHandler> > (inPattern));
	if (it != fRegexCache.end())
		resultHandler = XBOX::RetainRefCountable ((*it).second.Get());

	return resultHandler;
}


void VHTTPServerProject::EnableStaticPagesService (bool inValue)
{
	fDefaultRequestHandler->SetEnable (inValue);
}


bool VHTTPServerProject::GetEnableStaticPagesService()
{
	return fDefaultRequestHandler->GetEnable();
}


IHTTPRequestHandler *VHTTPServerProject::GetDefaultRequestHandler() const
{
	return fDefaultRequestHandler;
}


uLONG8 VHTTPServerProject::GetHitsCount()
{
	XBOX::VTaskLock lock (&fHitsCountLock);
	return fHitsCount;
}


uLONG8 VHTTPServerProject::IncreaseHitsCount()
{
	XBOX::VTaskLock lock (&fHitsCountLock);
	return ++fHitsCount;
}


void VHTTPServerProject::_AddVirtualFoldersFromSettings (VVirtualHost *ioVirtualHost)
{
	if (fSettings->GetUseWALibVirtualFolder())
	{
		XBOX::VRefPtr<VFolder>	wakandaServerFolder;
#if VERSIONWIN
		wakandaServerFolder.Adopt (XBOX::VProcess::Get()->RetainFolder (XBOX::VProcess::eFS_Executable));
#else
		XBOX::VFilePath path (VProcess::Get()->GetExecutableFolderPath());

#if VERSION_LINUX
		//jmo - Check that : It should break on Mac !
		XBOX::VFolder *folder = new VFolder (path);
#else
		XBOX::VFolder *folder = new VFolder (path.ToParent());
#endif
		wakandaServerFolder.Adopt (folder);
#endif

		if (NULL != wakandaServerFolder)
		{
			XBOX::VString	indexFileName;
			XBOX::VString	keyword (CVSTR ("walib"));
			XBOX::VFilePath	waLibPath (wakandaServerFolder->GetPath());
			VVirtualFolder *virtualFolder = NULL;

			waLibPath.ToSubFolder (keyword);
			virtualFolder = fHTTPServer->RetainVirtualFolder (waLibPath.GetPath(), indexFileName, keyword);
			if (NULL != virtualFolder)
				ioVirtualHost->AddVirtualFolder (virtualFolder);

			XBOX::ReleaseRefCountable (&virtualFolder);
		}

		// Load Virtual Folders from settings
		const VVirtualFolderSettingsVector&	virtualFolderSettings = fSettings->GetVirtualFoldersVector();
		for (VVirtualFolderSettingsVector::const_iterator it = virtualFolderSettings.begin(); it != virtualFolderSettings.end(); ++it)
		{
			VVirtualFolder *virtualFolder = fHTTPServer->RetainVirtualFolder ((*it)->GetLocation(), (*it)->GetIndex(), (*it)->GetName());
			if (NULL != virtualFolder)
				ioVirtualHost->AddVirtualFolder (virtualFolder);

			XBOX::ReleaseRefCountable (&virtualFolder);
		}
	}
}


bool VHTTPServerProject::AddVirtualHostFromSettings()
{
	XBOX::VError error = XBOX::VE_OK;

	if (fSettings->GetWebFolderPath().IsValid())
	{
		// sc 16/02/2012 create a virtual host if needed
		if (fVirtualHosts.empty())
		{
			// sc 16/02/2012 create a virtual host if needed
			VVirtualHost *virtualHost = new VVirtualHost (this);
			if (testAssert(virtualHost != NULL))
			{
				fVirtualHosts.push_back (virtualHost);
				error = fHTTPServer->AddVirtualHost (virtualHost);
			}
			else
			{
				error = XBOX::VE_MEMORY_FULL;
			}
		}

		if (error == XBOX::VE_OK)
		{
			for (std::vector<VVirtualHost *>::const_iterator it = fVirtualHosts.begin(); it != fVirtualHosts.end(); ++it)
			{
				_AddVirtualFoldersFromSettings (*it);
			}
		}
	}
	
	return (error == XBOX::VE_OK);
}


XBOX::VError VHTTPServerProject::AddVirtualFolder (const XBOX::VString& inLocationPath, const XBOX::VString& inIndexFileName, const XBOX::VString& inMatchingKeyWord)
{
	if (inLocationPath.IsEmpty() || inMatchingKeyWord.IsEmpty())
		return XBOX::VE_INVALID_PARAMETER;

	XBOX::VError		error = XBOX::VE_MEMORY_FULL;
	VVirtualFolder *	virtualFolder = fHTTPServer->RetainVirtualFolder (inLocationPath, inIndexFileName, inMatchingKeyWord);

	if (NULL!= virtualFolder)
	{
		XBOX::VTaskLock lock (&fVirtualHostsLock);

		error = XBOX::VE_OK;
		if (fVirtualHosts.empty())
		{
			// sc 16/02/2012 create a virtual host if needed
			VVirtualHost *virtualHost = new VVirtualHost (this);
			if (testAssert(virtualHost != NULL))
			{
				fVirtualHosts.push_back (virtualHost);
				error = fHTTPServer->AddVirtualHost (virtualHost);
			}
			else
			{
				error = XBOX::VE_MEMORY_FULL;
			}
		}

		if (error == XBOX::VE_OK)
		{
			for (std::vector<VVirtualHost *>::const_iterator it = fVirtualHosts.begin(); (it != fVirtualHosts.end()) && (error == XBOX::VE_OK); ++it)
			{
				error = (*it)->AddVirtualFolder (virtualFolder);
			}
		}
		
		XBOX::ReleaseRefCountable (&virtualFolder);
	}

	return error;
}


void VHTTPServerProject::RegisterRequest (IHTTPResponse *inRequest)
{
	XBOX::VTaskLock lock (&fRunningRequestsLock);
	fRunningRequests.push_back (inRequest);
}


void VHTTPServerProject::UnregisterRequest (IHTTPResponse *inRequest)
{
	XBOX::VTaskLock lock (&fRunningRequestsLock);
	
	std::vector<IHTTPResponse *>::iterator it = std::find (fRunningRequests.begin(), fRunningRequests.end(), inRequest);
	if (it != fRunningRequests.end())
		fRunningRequests.erase (it);
}


sLONG VHTTPServerProject::GetRunningRequestsCount()
{
	XBOX::VTaskLock lock (&fRunningRequestsLock);
	return (sLONG)fRunningRequests.size();
}


void VHTTPServerProject::SaveToBag (XBOX::VValueBag& outBag)
{
	outBag.SetLong (NSHTTPServerProject::refCount, GetRefCount());
	outBag.SetLong (NSHTTPServerProject::taskID, fServerTaskID);
	outBag.SetTime (NSHTTPServerProject::startingTime, fStartingTime);
	outBag.SetBool (NSHTTPServerProject::acceptRequests, fAcceptIncommingRequests);
	outBag.SetLong (NSHTTPServerProject::hitCount, GetHitsCount());
	outBag.SetLong (NSHTTPServerProject::runningRequests, GetRunningRequestsCount());
	outBag.SetBool (NSHTTPServerProject::securityComponentAvailable, (NULL != fSecurityManager));
	outBag.SetBool (NSHTTPServerProject::authenticationManagerAvailable, (NULL != fAuthenticationManager));

	XBOX::VBagArray *bagArray = new XBOX::VBagArray();
	if (NULL != bagArray)
	{
		XBOX::VTaskLock lock (&fRequestHandlersLock);
		for (VectorOfRequestHandler::const_iterator it = fRequestHandlers.begin(); it != fRequestHandlers.end(); ++it)
		{
			XBOX::VString string;
			XBOX::VectorOfVString patterns;
			(*it)->GetPatterns (&patterns);

			for (XBOX::VectorOfVString::const_iterator itString = patterns.begin(); itString != patterns.end(); ++itString)
			{
				string.AppendString (*itString);
				string.AppendUniChar (CHAR_SEMICOLON);
			}

			XBOX::VValueBag *handlerBag = new XBOX::VValueBag();
			if (NULL != handlerBag)
			{
				handlerBag->SetLong (NSHTTPServerProject::requestHandlers::refCount, (*it)->GetRefCount());
				handlerBag->SetString (NSHTTPServerProject::requestHandlers::patterns, string);
				handlerBag->SetBool (NSHTTPServerProject::requestHandlers::enabled, (*it)->GetEnable());
				bagArray->AddTail (handlerBag);
				XBOX::QuickReleaseRefCountable (handlerBag);
			}
		}

		outBag.SetElements (L"requestHandlers", bagArray);
		XBOX::QuickReleaseRefCountable (bagArray);
	}

	XBOX::VValueBag *settingsBag = new XBOX::VValueBag();
	if (NULL != settingsBag)
	{
		fSettings->SaveToBag (settingsBag);
		outBag.AddElement (L"settings", settingsBag);
		XBOX::QuickReleaseRefCountable (settingsBag);
	}

	XBOX::VValueBag *serverBag = new XBOX::VValueBag();
	if (NULL != serverBag)
	{
		serverBag->SetLong (NSHTTPServerProject::serverInfos::refCount, fHTTPServer->GetRefCount());
		serverBag->SetBool (NSHTTPServerProject::serverInfos::zipComponentAvailable, fHTTPServer->GetZipComponentAvailable());

		outBag.AddElement (L"server", serverBag);
		XBOX::QuickReleaseRefCountable (serverBag);
	}

	bagArray = new XBOX::VBagArray();
	if (NULL != bagArray)
	{
		XBOX::VTaskLock lock (&fVirtualHostsLock);
		for (std::vector<VVirtualHost *>::const_iterator it = fVirtualHosts.begin(); it != fVirtualHosts.end(); ++it)
		{
			XBOX::VValueBag *virtualHostBag = new XBOX::VValueBag();
			if (NULL != virtualHostBag)
			{
				(*it)->SaveToBag (*virtualHostBag);
				bagArray->AddTail (virtualHostBag);
				XBOX::QuickReleaseRefCountable (virtualHostBag);
			}
		}

		outBag.SetElements (L"virtualHosts", bagArray);
		XBOX::QuickReleaseRefCountable (bagArray);
	}

	bagArray = new XBOX::VBagArray();
	if (NULL != bagArray)
	{
		if ((NULL != fAuthenticationManager) && (NULL != fAuthenticationManager->GetAuthenticationReferee()))
		{
			XBOX::VValueBag *authResourcesBag = new XBOX::VValueBag();
			if (NULL != authResourcesBag)
			{
				fAuthenticationManager->GetAuthenticationReferee()->SaveToBag (authResourcesBag);
				bagArray->AddTail (authResourcesBag);
				XBOX::QuickReleaseRefCountable (authResourcesBag);
			}
		}

		if (bagArray->GetCount() > 0)
			outBag.SetElements (L"authResources", bagArray);

		XBOX::QuickReleaseRefCountable (bagArray);
	}
}


XBOX::VError VHTTPServerProject::_BuildRegexMatcher (const XBOX::VString& inPatternString, XBOX::VRegexMatcher **outMatcher)
{
	if (NULL == outMatcher)
		return XBOX::VE_INVALID_PARAMETER;

	XBOX::VString	patternString (inPatternString);
	XBOX::VError	error = XBOX::VE_OK;
	sLONG			pos = 0;

	if (!HTTPServerTools::BeginsWithASCIICString (patternString.GetCPointer(), "(?i)"))
		patternString.Insert (CVSTR ("(?i)"), 1);

	if (HTTPServerTools::EndsWithASCIICString (patternString, "/") &&
		!HTTPServerTools::EndsWithASCIICString (patternString, "$"))
		patternString.AppendString (CVSTR (".*"));

	XBOX::VString string;

	if ((pos = patternString.FindUniChar (CHAR_CIRCUMFLEX_ACCENT)) == 0)
	{
		string.FromCString ("^");
		pos = HTTPServerTools::FindASCIICString (patternString, "(?i)") + 4;
	}
	else
	{
		++pos;
	}

#if HTTP_SERVER_USE_PROJECT_PATTERNS
	if (!fSettings->GetProjectPattern().IsEmpty())
	{
		string.AppendCString ("(/");
		string.AppendString (fSettings->GetProjectPattern());
		string.AppendCString ("|)");
	}
#endif

	patternString.Insert (string, pos);

	if (!patternString.IsEmpty())
		*outMatcher = XBOX::VRegexMatcher::Create (patternString, &error);

	return error;
}


void VHTTPServerProject::SetSecurityManager (CSecurityManager *inSecurityManager)
{
	if (NULL != fSecurityManager)
		XBOX::QuickReleaseRefCountable (fSecurityManager);

	fSecurityManager = XBOX::RetainRefCountable (inSecurityManager);

	if (NULL != dynamic_cast<VAuthenticationManager *>(fAuthenticationManager))
		dynamic_cast<VAuthenticationManager *>(fAuthenticationManager)->SetSecurityManager (inSecurityManager);
}


XBOX::VError VHTTPServerProject::AddPreProcessingHandler (IPreProcessingHandler *inPreProcessingHandler)
{
	XBOX::VTaskLock lock (&fPreProcessingRegExCacheLock);
	return AddProcessingHandler<IPreProcessingHandler> (fPreProcessingRegExCache, inPreProcessingHandler);
}


XBOX::VError VHTTPServerProject::RemovePreProcessingHandler (IPreProcessingHandler *inPreProcessingHandler)
{
	XBOX::VTaskLock lock (&fPreProcessingRegExCacheLock);
	return RemoveProcessingHandler<IPreProcessingHandler> (fPreProcessingRegExCache, inPreProcessingHandler);
}


IPreProcessingHandler *VHTTPServerProject::RetainPreProcessingHandler (const XBOX::VString& inURLString)
{
	if (0 == fPreProcessingRegExCache.size() || inURLString.IsEmpty())
		return NULL;

	XBOX::VTaskLock lock (&fPreProcessingRegExCacheLock);
	return RetainProcessingHandler <IPreProcessingHandler> (fPreProcessingRegExCache, inURLString);
}


XBOX::VError VHTTPServerProject::AddPostProcessingHandler (IPostProcessingHandler *inPostProcessingHandler)
{
	XBOX::VTaskLock lock (&fPostProcessingRegExCacheLock);
	return AddProcessingHandler<IPostProcessingHandler> (fPostProcessingRegExCache, inPostProcessingHandler);
}


XBOX::VError VHTTPServerProject::RemovePostProcessingHandler (IPostProcessingHandler *inPostProcessingHandler)
{
	XBOX::VTaskLock lock (&fPostProcessingRegExCacheLock);
	return RemoveProcessingHandler<IPostProcessingHandler> (fPostProcessingRegExCache, inPostProcessingHandler);
}


IPostProcessingHandler *VHTTPServerProject::RetainPostProcessingHandler (const XBOX::VString& inURLString)
{
	if ((0 == fPostProcessingRegExCache.size()) || inURLString.IsEmpty())
		return NULL;

	XBOX::VTaskLock lock (&fPostProcessingRegExCacheLock);
	return RetainProcessingHandler <IPostProcessingHandler> (fPostProcessingRegExCache, inURLString);
}


inline
void VHTTPServerProject::SetErrorProcessingHandler (IErrorProcessingHandler *inErrorProcessingHandler)
{
	if (NULL == inErrorProcessingHandler)
		return;

	if (fErrorProcessingHandler == inErrorProcessingHandler)
	{
		XBOX::CopyRefCountable (&fErrorProcessingHandler, inErrorProcessingHandler);
	}
	else
	{
		if (NULL != fErrorProcessingHandler)
			XBOX::QuickReleaseRefCountable (fErrorProcessingHandler);

		fErrorProcessingHandler = XBOX::RetainRefCountable (inErrorProcessingHandler);
	}
}


inline
IErrorProcessingHandler *VHTTPServerProject::GetErrorProcessingHandler()
{
	return fErrorProcessingHandler;
}


inline
void VHTTPServerProject::SetAuthenticationDelegate (IAuthenticationDelegate *inAuthenticationDelegate)
{
	if (NULL == inAuthenticationDelegate)
		return;

	if (fAuthenticationDelegate == inAuthenticationDelegate)
	{
		XBOX::CopyRefCountable (&fAuthenticationDelegate, inAuthenticationDelegate);
	}
	else
	{
		if (NULL != fAuthenticationDelegate)
			XBOX::QuickReleaseRefCountable (fAuthenticationDelegate);
	
		fAuthenticationDelegate = XBOX::RetainRefCountable (inAuthenticationDelegate);
	}
}


inline
IAuthenticationDelegate *VHTTPServerProject::GetAuthenticationDelegate()
{
	return fAuthenticationDelegate;
}
