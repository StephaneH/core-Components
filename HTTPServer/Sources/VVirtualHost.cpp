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
#include "HTTPServerCache.h"
#include "HTTPServerSettings.h"
#include "VVirtualHost.h"
#include "VVirtualFolder.h"


namespace NSVirtualHost
{
	CREATE_BAGKEY (refCount);
	CREATE_BAGKEY (hostName);
	CREATE_BAGKEY (keyWord);
	CREATE_BAGKEY (rootFolder);
	CREATE_BAGKEY (indexFileName);
	namespace virtualFolder
	{
		CREATE_BAGKEY (refCount);
		CREATE_BAGKEY (name);
		CREATE_BAGKEY (parentFolder);
		CREATE_BAGKEY (indexFileName);
	}
}


static
void MakeHostPattern (const XBOX::VString& inHost, IP4 inListeningAddress, PortNumber inPort, bool inEnableSSL, PortNumber inSSLPort, XBOX::VString& outHostPattern)
{
	XBOX::VString			string;
	XBOX::VectorOfVString	addressesPatterns;
	XBOX::VectorOfVString	portsPatterns;
	XBOX::VString			hostname;

	XBOX::VSystem::GetHostName (hostname);
	hostname.ToLowerCase();

	if (!inHost.IsEmpty())
	{
		if (!hostname.IsEmpty() && !HTTPServerTools::EqualASCIIVString (inHost, hostname))
			AppendUniqueValueToVector (addressesPatterns, inHost);

		string.Clear();
#if WITH_DEPRECATED_IPV4_API
		XBOX::ServerNetTools::ResolveAddress (inHost, &string);
#elif DEPRECATED_IPV4_API_SHOULD_NOT_COMPILE
	#error NEED AN IP V6 UPDATE
#endif
		
		AppendUniqueValueToVector (addressesPatterns, string);
	}

	if (!hostname.IsEmpty())
	{
		// YT 26-Jan-2011 - For Ke-Fong: There was a bug in VirtualHost resolve with Safari and Bonjour
		// KFL 27-Jan-2011 - Accept "<hostname>" or "<hostname>.local" or "<hostname>.local." (note the ending dot).

		hostname.AppendCString ("(.local(.|)|)"); 
		AppendUniqueValueToVector (addressesPatterns, hostname);
	}

	// Listening on all local addresses
	if (0 == inListeningAddress)
	{
		std::vector<IP4> ipv4Addresses;
#if WITH_DEPRECATED_IPV4_API
		XBOX::ServerNetTools::GetLocalIPv4Addresses (ipv4Addresses);
#elif DEPRECATED_IPV4_API_SHOULD_NOT_COMPILE
	#error NEED AN IP V6 UPDATE
#endif
		AppendUniqueValueToVector (addressesPatterns, CVSTR ("localhost"));
		AppendUniqueValueToVector (addressesPatterns, CVSTR ("127.0.0.1"));

		for (std::vector<IP4>::const_iterator it = ipv4Addresses.begin(); it != ipv4Addresses.end(); ++it)
		{
			string.Clear();
#if WITH_DEPRECATED_IPV4_API			
			XBOX::ServerNetTools::GetIPAdress ((*it), string);
#elif DEPRECATED_IPV4_API_SHOULD_NOT_COMPILE
	#error NEED AN IP V6 UPDATE
#endif
			AppendUniqueValueToVector (addressesPatterns, string);
		}
	}
	// Listening on a specific local address
	else
	{
		string.Clear();
#if WITH_DEPRECATED_IPV4_API
		XBOX::ServerNetTools::GetIPAdress (inListeningAddress, string);
#elif DEPRECATED_IPV4_API_SHOULD_NOT_COMPILE
	#error NEED AN IP V6 UPDATE
#endif		
		AppendUniqueValueToVector (addressesPatterns, string);
	}

	// Listening Port
	if (inPort != DEFAULT_LISTENING_PORT)
	{
		string.FromLong (inPort);
		AppendUniqueValueToVector (portsPatterns, string);
	}

	// Listening SSL Port
	if (inEnableSSL)
	{
		string.FromLong (inSSLPort);
		AppendUniqueValueToVector (portsPatterns, string);
	}

	// Build Pattern such as "(?i)(localhost|127.0.0.1)+:(8080:443)"
	size_t i = 0;
	size_t size = addressesPatterns.size();

	outHostPattern.FromCString ("(?i)");	// UREGEX_CASE_INSENSITIVE
	if (size > 1)
		outHostPattern.AppendUniChar (CHAR_LEFT_PARENTHESIS);

	for (i = 0; i < size; ++i)
	{
		outHostPattern.AppendString (addressesPatterns.at (i));
		if (i < size -1)
			outHostPattern.AppendUniChar (CHAR_VERTICAL_LINE);
	}

	if (size > 1)
		outHostPattern.AppendUniChar (CHAR_RIGHT_PARENTHESIS);

	size = portsPatterns.size();
	if (size > 0)
	{
		outHostPattern.AppendCString ("+:");

		if (size > 1)
			outHostPattern.AppendUniChar (CHAR_LEFT_PARENTHESIS);

		for (i = 0; i < size; ++i)
		{
			outHostPattern.AppendString (portsPatterns.at (i));
			if (i < size -1)
				outHostPattern.AppendUniChar (CHAR_VERTICAL_LINE);
		}

		if (size > 1)
			outHostPattern.AppendUniChar (CHAR_RIGHT_PARENTHESIS);
	}
	else
	{
		/*
			Build Pattern such as "(?i)(localhost|127.0.0.1)(|:80|:443)"
			Some HTTP Client (iSort for example) append systematically port number in host header even if a standard port is used
		*/
		outHostPattern.AppendUniChar (CHAR_LEFT_PARENTHESIS);

		if (DEFAULT_LISTENING_PORT == inPort)
			outHostPattern.AppendCString ("|:80");

		if (DEFAULT_LISTENING_SSL_PORT == inSSLPort)
			outHostPattern.AppendCString ("|:443");

		outHostPattern.AppendUniChar (CHAR_RIGHT_PARENTHESIS);
	}

	outHostPattern.AppendUniChar (CHAR_DOLLAR_SIGN);
}


static
void MakeVirtualFolderPattern (const XBOX::VString& inKeyword, XBOX::VString& outPatternString)
{
	XBOX::VString	keyword (inKeyword);

	if (HTTPServerTools::BeginsWithASCIICString (keyword, "/"))
		keyword.SubString (2, keyword.GetLength() - 1);

	if (HTTPServerTools::EndsWithASCIICString (keyword, "/"))
		keyword.SubString (1, keyword.GetLength() - 1);

	if (!HTTPServerTools::BeginsWithASCIICString (keyword, "(?i)"))
		outPatternString.AppendCString ("(?i)");

	if (!HTTPServerTools::FindASCIICString (keyword, "^"))
		outPatternString.AppendCString ("^");

	outPatternString.AppendCString ("/");
	outPatternString.AppendString (keyword);

	if (!HTTPServerTools::EndsWithASCIICString (keyword, "/*"))
		outPatternString.AppendCString ("/*");
}


//--------------------------------------------------------------------------------------------------


VVirtualHost::VVirtualHost (const VHTTPServerProject *inProject)
: fVirtualFolderMap()
, fVirtualFolderMapLock()
, fDefaultVirtualFolder (NULL)
, fProject (NULL)
, fServerLog (NULL)
, fHostRegexMatcher (NULL)
#if !HTTP_SERVER_GLOBAL_CACHE
, fCacheManager (NULL)
#endif
, fUUIDString()
{
	fProject = (VHTTPServerProject *)inProject;

	_Init();
}


VVirtualHost::~VVirtualHost()
{
	fVirtualFolderMap.erase (fVirtualFolderMap.begin(), fVirtualFolderMap.end());

#if HTTP_SERVER_USE_SYSTEM_NOTIFICATIONS
	XBOX::VFileSystemNotifier::Instance()->StopWatchingForChanges (*fDefaultVirtualFolder->GetFolder(), this);
#endif

#if HTTP_SERVER_GLOBAL_CACHE
	_Deinit();
#else
	XBOX::ReleaseRefCountable (&fCacheManager);
#endif

	XBOX::ReleaseRefCountable (&fDefaultVirtualFolder);
	XBOX::ReleaseRefCountable (&fServerLog);
	fProject = NULL;

	XBOX::ReleaseRefCountable (&fHostRegexMatcher);
}


inline
VHTTPServerProjectSettings *VVirtualHost::GetSettings() const
{
	return dynamic_cast<VHTTPServerProjectSettings *>(fProject->GetSettings());
}


void VVirtualHost::_Init()
{
#if HTTP_SERVER_USE_PROJECT_PATTERNS
	fProjectPattern.FromString (GetSettings()->GetProjectPattern());

	if (!fProjectPattern.IsEmpty())
	{
		fURLPattern.FromCString ("(?i)^/");	// YT 24-Feb-2011 - ACI0069901
		fURLPattern.AppendString (fProjectPattern);
		fURLPattern.AppendCString ("/*");
	}
	else
	{
		fURLPattern.Clear();
	}
#endif

	XBOX::VString string;
	string.FromString (GetSettings()->GetHostName());

	MakeHostPattern (string, GetSettings()->GetListeningAddress(), GetSettings()->GetListeningPort(), GetSettings()->GetAllowSSL(), GetSettings()->GetListeningSSLPort(), fHostPattern);

	XBOX::VError error = XBOX::VE_OK;
	fHostRegexMatcher = XBOX::VRegexMatcher::Create (fHostPattern, &error);
	if (XBOX::VE_OK != error)
		XBOX::ReleaseRefCountable (&fHostRegexMatcher);

#if HTTP_SERVER_GLOBAL_CACHE
	XBOX::VValueBag *bag = new XBOX::VValueBag();
	if (testAssert (NULL != bag))
	{
		if (XBOX::VE_OK == GetSettings()->SaveToBag (bag))
			fProject->GetHTTPServer()->GetCacheManager()->LoadRulesFromBag (bag);

		bag->Release();
	}
#else
	/* Init Cache Manager */
	fCacheManager = new VCacheManager();
	if (testAssert (NULL != fCacheManager))
	{
		fCacheManager->SetEnableDataCache (GetSettings()->GetEnableCache());
		fCacheManager->SetCachedObjectMaxSize (GetSettings()->GetCachedObjectMaxSize());
		fCacheManager->SetCacheMaxSize (GetSettings()->GetCacheMaxSize());

		XBOX::VValueBag *bag = new XBOX::VValueBag();
		if (testAssert (NULL != bag))
		{
			if (XBOX::VE_OK == GetSettings()->SaveToBag (bag))
				fCacheManager->LoadRulesFromBag (bag);

			bag->Release();
		}
	}
#endif

	/* Init HTTP Log */
	fServerLog = new VHTTPServerLog ();
	if (testAssert (fServerLog))
	{
		XBOX::VString	logFileName (GetSettings()->GetLogFileName());
		XBOX::VString	extension;
		sLONG			posChar = logFileName.FindUniChar (CHAR_FULL_STOP);

		if (posChar > 0)
			logFileName.GetSubString (posChar + 1, logFileName.GetLength() - posChar, extension);

		fServerLog->GetSettings().SetLogFolderPath (GetSettings()->GetLogFolderPath());
		fServerLog->GetSettings().SetLogFormat ((EHTTPServerLogFormat)GetSettings()->GetLogFormat());
		fServerLog->GetSettings().SetLogTokens (GetSettings()->GetLogTokens());
		fServerLog->GetSettings().SetLogFileName (logFileName);
		fServerLog->GetSettings().SetLogFileNameExtension (extension);
		fServerLog->GetSettings().SetArchivesFolderName (GetSettings()->GetLogBackupFolderName());

		/* Log Rotation Settings */
		fServerLog->SetBackupSettings (GetSettings()->GetLogBackupSettings());

		/* Install Signal */
		GetSettings()->GetSignal_SettingsChanged()->Connect (this, XBOX::VTask::GetCurrent(), &VVirtualHost::SettingsChanged_Message);
	}

#if HTTP_SERVER_USE_PROJECT_PATTERNS
	fDefaultVirtualFolder = new VVirtualFolder (GetSettings()->GetWebFolderPath(), GetSettings()->GetIndexPageName(), CVSTR("")/*GetSettings()->GetProjectPattern()*/, fProjectPattern);
#else
	const XBOX::VString emptyString;
	fDefaultVirtualFolder = new VVirtualFolder (GetSettings()->GetWebFolderPath().GetPath(), GetSettings()->GetIndexPageName(), emptyString/*GetSettings()->GetProjectPattern()*/, emptyString/*fProjectPattern*/);
#endif

#if HTTP_SERVER_USE_SYSTEM_NOTIFICATIONS
	/* Install System Notification */
	if (NULL != fDefaultVirtualFolder)
		XBOX::VFileSystemNotifier::Instance()->StartWatchingForChanges (*fDefaultVirtualFolder->GetFolder(), VFileSystemNotifier::kAll, this, 100);
#endif

	XBOX::VUUID uid (true);
	uid.GetString (fUUIDString);
}


void VVirtualHost::_Deinit()
{
#if HTTP_SERVER_GLOBAL_CACHE

	XBOX::VString	patternString;
	XBOX::VError	error = XBOX::VE_OK;
	const VHTTPResourcesVector resourcesVector = GetSettings()->GetResourcesVector();
	VCacheManager *	cacheManager = fProject->GetHTTPServer()->GetCacheManager();

	for (VHTTPResourcesVector::const_iterator it = resourcesVector.begin(); it != resourcesVector.end(); ++it)
	{
		if (!(*it)->GetURLMatch().IsEmpty())
			patternString.FromString ((*it)->GetURLMatch());
		else
			patternString.FromString ((*it)->GetURL());

		if (!patternString.IsEmpty())
		{
			error = cacheManager->RemoveReleaseResource (patternString);
			if (XBOX::VE_OK != error)
				break;
		}
	}

	GetSettings()->GetSignal_SettingsChanged()->Disconnect (this);

#endif
}


void VVirtualHost::SettingsChanged_Message (VHTTPServerProjectSettings *inSettings)
{
	if (testAssert((NULL != fServerLog) && (NULL != inSettings)))
	{
		fServerLog->GetSettings().SetLogFormat ((EHTTPServerLogFormat)inSettings->GetLogFormat());
		fServerLog->GetSettings().SetLogTokens (inSettings->GetLogTokens());
	}
}


XBOX::VError VVirtualHost::AddVirtualFolder (const VVirtualFolder *inVirtualFolder, bool inOverwrite/* = false*/)
{
	if ((NULL == inVirtualFolder) || ((NULL != inVirtualFolder) && inVirtualFolder->GetName().IsEmpty()))
		return XBOX::VE_INVALID_PARAMETER;

	XBOX::VError			error = XBOX::VE_OK;
	XBOX::VTaskLock			lock (&fVirtualFolderMapLock);
	XBOX::VString			regexString;
	XBOX::VRegexMatcher *	matcher = NULL;

	MakeVirtualFolderPattern (inVirtualFolder->GetName(), regexString);

	matcher = XBOX::VRegexMatcher::Create (regexString, &error);

	if ((NULL != matcher) && (XBOX::VE_OK == error))
	{
		VVirtualFolderMap::iterator found = std::find_if (fVirtualFolderMap.begin(), fVirtualFolderMap.end(), EqualVRegexMatcherFunctor<VVirtualFolder *> (regexString));
		if (found == fVirtualFolderMap.end())
		{
			fVirtualFolderMap[matcher] = const_cast<VVirtualFolder *>(inVirtualFolder);
		}
		else if (inOverwrite)
		{
			found->second->Release();
			found->second = const_cast<VVirtualFolder *>(inVirtualFolder);
		}
		else
		{
			error = VE_VIRTUAL_FOLDER_ALREADY_EXIST;
		}
	}

	XBOX::QuickReleaseRefCountable (matcher);

	return error;
}


VVirtualFolder * VVirtualHost::RetainVirtualFolder (const XBOX::VString& inLocationPath, const XBOX::VString& inIndexPage, const XBOX::VString& inKeyword)
{
	XBOX::VTaskLock			lock (&fVirtualFolderMapLock);
	XBOX::VString			regexString;
	VVirtualFolder *		resultVirtualFolder = NULL;
	XBOX::VFilePath			locationPath (inLocationPath);
	XBOX::VString			locationPosixPath;

	locationPath.GetPosixPath (locationPosixPath);

	MakeVirtualFolderPattern (inKeyword, regexString);

	VVirtualFolderMap::const_iterator found = std::find_if (fVirtualFolderMap.begin(), fVirtualFolderMap.end(), EqualVRegexMatcherFunctor<VVirtualFolder *> (regexString));
	if (found != fVirtualFolderMap.end())
	{
		resultVirtualFolder = (*found).second.Get();

		if ((!HTTPServerTools::EqualASCIIVString (resultVirtualFolder->GetLocationPath(), locationPosixPath)) ||
			(!HTTPServerTools::EqualASCIIVString (resultVirtualFolder->GetIndexFileName(), inIndexPage)))
			resultVirtualFolder = NULL;
	}

	if (NULL != resultVirtualFolder)
		resultVirtualFolder->Retain();

	return resultVirtualFolder;
}


VVirtualFolder *VVirtualHost::RetainMatchingVirtualFolder (const XBOX::VString& inURL)
{
	VVirtualFolder *	virtualFolder = NULL;
	XBOX::VTaskLock		lock (&fVirtualFolderMapLock);

	VVirtualFolderMap::iterator it = std::find_if (fVirtualFolderMap.begin(), fVirtualFolderMap.end(), MatchVRegexMatcherFunctor<VVirtualFolder *> (inURL, false));
	if (it != fVirtualFolderMap.end())
		virtualFolder = XBOX::RetainRefCountable ((*it).second.Get());

	if (NULL == virtualFolder)
		virtualFolder = XBOX::RetainRefCountable (fDefaultVirtualFolder);

	return virtualFolder;
}


void VVirtualHost::SaveToBag(XBOX::VValueBag &ioValueBag)
{
	XBOX::VString posixPath;

	ioValueBag.SetLong (NSVirtualHost::refCount, GetRefCount());
	ioValueBag.SetString (NSVirtualHost::hostName, fHostPattern);
#if HTTP_SERVER_USE_PROJECT_PATTERNS
	ioValueBag.SetString (NSVirtualHost::keyWord, fURLPattern);
#endif
	
	posixPath.FromString (fDefaultVirtualFolder->GetLocationPath());
	ioValueBag.SetString (NSVirtualHost::rootFolder, posixPath);
	if (!fDefaultVirtualFolder->GetIndexFileName().IsEmpty())
		ioValueBag.SetString (NSVirtualHost::indexFileName, fDefaultVirtualFolder->GetIndexFileName());

	if (fVirtualFolderMap.size() > 0)
	{
		XBOX::VBagArray *bagArray = new XBOX::VBagArray();
		if (bagArray)
		{
			XBOX::VString	indexFileName;
			XBOX::VTaskLock lock (&fVirtualFolderMapLock);

			for (VVirtualFolderMap::const_iterator it = fVirtualFolderMap.begin(); it != fVirtualFolderMap.end(); ++it)
			{
				XBOX::VValueBag *virtualFolderBag = new XBOX::VValueBag();
				if (NULL != virtualFolderBag)
				{
					posixPath.FromString ((*it).second->GetLocationPath());
					indexFileName.FromString ((*it).second->GetIndexFileName());
					virtualFolderBag->SetLong (NSVirtualHost::virtualFolder::refCount, (*it).second->GetRefCount());
					virtualFolderBag->SetString (NSVirtualHost::virtualFolder::name, (*it).second->GetName());
					virtualFolderBag->SetString (NSVirtualHost::virtualFolder::parentFolder, posixPath);
					if (!indexFileName.IsEmpty())
						virtualFolderBag->SetString (NSVirtualHost::virtualFolder::indexFileName, indexFileName);

					bagArray->AddTail (virtualFolderBag);
					XBOX::QuickReleaseRefCountable (virtualFolderBag);
				}
			}

			ioValueBag.SetElements (L"virtualFolders", bagArray);
			XBOX::QuickReleaseRefCountable (bagArray);
		}
	}
}


XBOX::VError VVirtualHost::GetFilePathFromURL (const XBOX::VString& inURL, XBOX::VString& outLocationPath)
{
	XBOX::VError		error = XBOX::VE_FILE_NOT_FOUND;
	VVirtualFolder *	virtualFolder = RetainMatchingVirtualFolder (inURL);

	if (NULL != virtualFolder)
		error = virtualFolder->GetFilePathFromURL (inURL, outLocationPath);

	XBOX::QuickReleaseRefCountable (virtualFolder);

	return error;
}


XBOX::VError VVirtualHost::GetFileContentFromURL (const XBOX::VString& inURL, IHTTPResponse *ioResponse, bool inUseFullPath)
{
	XBOX::VError		error = XBOX::VE_FILE_NOT_FOUND;
	XBOX::VFilePath		filePath;

	if (inUseFullPath)
	{
		filePath.FromFullPath (inURL);
		if (filePath.IsValid() && filePath.IsFile())
			error = XBOX::VE_OK;
	}
	else
	{
		XBOX::VString locationPath;
		error = GetFilePathFromURL (inURL, locationPath);

		if (XBOX::VE_OK == error)
			filePath.FromFullPath (locationPath);
	}

	if (XBOX::VE_OK == error)
	{
		XBOX::VFile *file = new XBOX::VFile (filePath);

		if (NULL != file)
		{
			if (file->Exists())
			{
				XBOX::VFileStream fileStream (file);
				if (testAssert (fileStream.OpenReading() == XBOX::VE_OK))
				{
					const sLONG8 MAX_FILE_SIZE = 10 * 1024 * 1024; //kMAX_sLONG
					sLONG8 fileSize = 0;

					file->GetSize (&fileSize);
					if ((fileSize >= 0) && (fileSize < MAX_FILE_SIZE))
					{
						if (fileSize > 0)
						{
							XBOX::VSize bufferSize = (XBOX::VSize)fileSize;
							void *buffer = XBOX::vMalloc (bufferSize, 0);
							if (NULL != buffer)
							{
								fileStream.GetBytes (buffer, &bufferSize);
								error = ioResponse->SetResponseBody (buffer, bufferSize);
								XBOX::vFree (buffer);
								buffer = NULL;
							}
							else
							{
								error = VHTTPServer::ThrowError (VE_HTTP_PROTOCOL_INTERNAL_SERVER_ERROR, STRING_ERROR_ALLOCATION_FAILED);
							}
						}
						
						if (XBOX::VE_OK == error)
							ioResponse->SetContentLengthHeader (fileSize);
					}
					else
					{
						error = VHTTPServer::ThrowError (VE_HTTP_PROTOCOL_INTERNAL_SERVER_ERROR, STRING_ERROR_FILE_LIMIT_REACHED);
					}

					fileStream.CloseReading();
				}
				else
				{
					error = VHTTPServer::ThrowError (VE_HTTP_PROTOCOL_INTERNAL_SERVER_ERROR, STRING_ERROR_FAILED_TO_OPEN_STREAM);
				}
			}
			else
			{
				error = VE_HTTP_PROTOCOL_NOT_FOUND;
			}
		}
		else
		{
			error = VHTTPServer::ThrowError (VE_HTTP_PROTOCOL_INTERNAL_SERVER_ERROR, STRING_ERROR_ALLOCATION_FAILED);
		}

		XBOX::QuickReleaseRefCountable (file);
	}

	if ((error == XBOX::VE_OK) && filePath.IsFile() && !ioResponse->IsResponseHeaderSet (HEADER_CONTENT_TYPE))
	{
		XBOX::VString contentType;
		XBOX::VString extension;
		filePath.GetExtension (extension);
		VMimeTypeManager::FindContentType (extension, contentType);
		ioResponse->AddResponseHeader (STRING_HEADER_CONTENT_TYPE, contentType);
	}

	return error;
}


#if HTTP_SERVER_USE_SYSTEM_NOTIFICATIONS
void VVirtualHost::FileSystemEventHandler (const std::vector<XBOX::VFilePath> &inFilePaths, XBOX::VFileSystemNotifier::EventKind inKind)
{
	if ((inKind & XBOX::VFileSystemNotifier::kFileModified) || (inKind & XBOX::VFileSystemNotifier::kFileDeleted))
	{
		for (std::vector<XBOX::VFilePath>::const_iterator it = inFilePaths.begin(); it != inFilePaths.end(); ++it)
		{
#if HTTP_SERVER_GLOBAL_CACHE
			fProject->GetHTTPServer()->GetCacheManager()->RemoveCachedObject (*it);
#else
			fCacheManager->RemoveCachedObject (*it);
#endif
		}
	}
}
#endif


bool VVirtualHost::MatchHostPattern (const XBOX::VString& inHostString) const
{
	if (testAssert (NULL != fHostRegexMatcher))
		return MatchRegex (fHostRegexMatcher, inHostString);

	return false;
}


bool VVirtualHost::AcceptConnectionsOnPort (const PortNumber inPort) const
{
	return ((GetSettings()->GetListeningPort() == inPort) || (GetSettings()->GetAllowSSL() && (GetSettings()->GetListeningSSLPort() == inPort)));
}


bool VVirtualHost::AcceptConnectionsOnAddress (const IP4 inIPv4Address) const
{
	return ((GetSettings()->GetListeningAddress() == 0) || (GetSettings()->GetListeningAddress() == inIPv4Address));
}
