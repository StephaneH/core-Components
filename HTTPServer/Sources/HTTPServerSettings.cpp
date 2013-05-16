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
#include "HTTPServerSettings.h"
#include "HTTPServerLog.h"


const XBOX::VString	HTTP_SERVER_LOG_FILE_PATH = CVSTR ("Logs/");
const XBOX::VString	HTTP_SERVER_LOG_FILE_EXTENSION = CVSTR ("waLog");
const XBOX::VString	HTTP_SERVER_LOG_FILE_NAME = CVSTR ("HTTPServer.") + HTTP_SERVER_LOG_FILE_EXTENSION;


const XBOX::VString DEFAULT_WEB_FOLDER_PATH = "./WebFolder";
const XBOX::VString DEFAULT_INDEX_PAGE_NAME = "index.html";
const XBOX::VString DEFAULT_SSL_CERTIFICATE_FOLDER_PATH = "";	//"./Certificates";
const bool			DEFAULT_ALLOW_SSL = false;
const bool			DEFAULT_SSL_MANDATORY = false;
const bool			DEFAULT_ENABLE_CACHE = true;
const sLONG			DEFAULT_CACHE_MAX_SIZE = 5 * 1024 * 1024;		// in bytes
const sLONG			DEFAULT_CACHED_OBJECT_MAX_SIZE = 512 * 1024;	// in bytes
const bool			DEFAULT_COMPRESSION_ENABLED = true;
const sLONG			DEFAULT_COMPRESSION_MIN_THRESHOLD = 1024;				// 1 KBytes (in bytes)
const sLONG			DEFAULT_COMPRESSION_MAX_THRESHOLD = 10 * 1024 * 1024;	// 10 MBytes (in bytes)
const XBOX::CharSet	DEFAULT_WEB_CHARSET = XBOX::VTC_UTF_8;
const XBOX::VString	DEFAULT_WEB_CHARSET_NAME = CVSTR ("UTF-8");
const sLONG			DEFAULT_LOG_FORMAT = 0;
const XBOX::VString DEFAULT_LOG_FILE_FOLDER_PATH = "./Logs";
const XBOX::VString DEFAULT_LOG_FILE_NAME = "LogWeb.txt";
const sLONG			DEFAULT_LOG_FILE_MAX_SIZE = 1024; // in kb
const bool			DEFAULT_KEEP_ALIVE_ENABLED = true;
const sLONG			DEFAULT_KEEP_ALIVE_TIMEOUT = 3;
const sLONG			DEFAULT_KEEP_ALIVE_MAX_CONNECTIONS = 100;
const IP4			LOCALHOST_ADDRESS = 2130706433;	// 127.0.0.1


//--------------------------------------------------------------------------------------------------


/* Typical Web settings (cd Spec 306 - Web Settings)
<services>
	<service type="http"
		activated="true"
		port="80"
		defaultHTMLRoot="WebFolder"
		defaultHomePage="index.html"
		allowSSL= "true"
		SSLCertificatePath="path/to/default/certificate/file"
		HTTPSPort="443"
		useWebCache="true"
		pageCacheSize="5192"
		inactiveWebProcessTimeout="5"
		sendExtendedCharacterSetDirectly="false" // Obsolete
		standardSet="UTF-8"
		acceptKeepAliveConnections="true"
		maximumRequestsByConnection="100"
		maximumTimeout="15"
		logFormat="No Log File">
		<eventHandlers>
			<handler name="onWebConnection" path="path/to/js/file" />
			<handler name="onWebAuthentication" path="path/to/js/file" />
		</eventHandlers>
	</service>
</services>
*/


//--------------------------------------------------------------------------------------------------


namespace RIASettingsKeys
{
	// Settings constants keys

	// Project settings
	namespace Project
	{
		const XBOX::VString kXmlElement ("project");

		CREATE_BAGKEY (refCount);
		CREATE_BAGKEY_WITH_DEFAULT (publicName, XBOX::VString, L"");
		CREATE_BAGKEY_WITH_DEFAULT_SCALAR (administrator, XBOX::VBoolean, bool, false);
		CREATE_BAGKEY_WITH_DEFAULT (hostName, XBOX::VString, L"localhost");
		CREATE_BAGKEY_WITH_DEFAULT (pattern, XBOX::VString, L"");
		CREATE_BAGKEY_WITH_DEFAULT (listen, XBOX::VString, L"127.0.0.1");
		CREATE_BAGKEY_WITH_DEFAULT (responseFormat, XBOX::VString, L"json");
		CREATE_BAGKEY_NO_DEFAULT (realm, XBOX::VString);
		CREATE_BAGKEY_NO_DEFAULT (authType, XBOX::VString);

		namespace HTTPServer
		{
			CREATE_PATHBAGKEY_WITH_DEFAULT_SCALAR ("servers/http", started, XBOX::VBoolean, bool, true);
		}
	}

	// HTTP Server settings
	namespace HTTP
	{
		const XBOX::VString kXmlElement ("http");

		CREATE_BAGKEY_WITH_DEFAULT_SCALAR (port, XBOX::VLong, sLONG, DEFAULT_LISTENING_PORT);
		CREATE_BAGKEY_WITH_DEFAULT_SCALAR (maximumProcess, XBOX::VLong, sLONG, 100);
		CREATE_BAGKEY_WITH_DEFAULT (SSLCertificatePath, XBOX::VString, DEFAULT_SSL_CERTIFICATE_FOLDER_PATH);
		CREATE_BAGKEY_WITH_DEFAULT_SCALAR (allowSSL, XBOX::VBoolean, bool, DEFAULT_ALLOW_SSL);
		CREATE_BAGKEY_WITH_DEFAULT_SCALAR (SSLMandatory, XBOX::VBoolean, bool, DEFAULT_SSL_MANDATORY);
		CREATE_BAGKEY_WITH_DEFAULT_SCALAR (SSLPort, XBOX::VLong, sLONG, DEFAULT_LISTENING_SSL_PORT);
		CREATE_BAGKEY_WITH_DEFAULT_SCALAR (SSLMinVersion, XBOX::VLong, sLONG, 3);
		CREATE_BAGKEY_WITH_DEFAULT_SCALAR (useCache, XBOX::VBoolean, bool, DEFAULT_ENABLE_CACHE);
		CREATE_BAGKEY_WITH_DEFAULT_SCALAR (pageCacheSize, XBOX::VLong, sLONG, DEFAULT_CACHE_MAX_SIZE);
		CREATE_BAGKEY_WITH_DEFAULT_SCALAR (cachedObjectMaxSize, XBOX::VLong, sLONG, DEFAULT_CACHED_OBJECT_MAX_SIZE);
		CREATE_BAGKEY_WITH_DEFAULT_SCALAR (inactiveProcessTimeout, XBOX::VLong, sLONG, 5);
		CREATE_BAGKEY_WITH_DEFAULT_SCALAR (sendExtendedCharacterSetDirectly, XBOX::VBoolean, bool, false);
		CREATE_BAGKEY_WITH_DEFAULT (standardSet, XBOX::VString, DEFAULT_WEB_CHARSET_NAME);
		CREATE_BAGKEY_WITH_DEFAULT_SCALAR (acceptKeepAliveConnections, XBOX::VBoolean, bool, DEFAULT_KEEP_ALIVE_ENABLED);
		CREATE_BAGKEY_WITH_DEFAULT_SCALAR (maximumRequestsByConnection, XBOX::VLong, sLONG, DEFAULT_KEEP_ALIVE_MAX_CONNECTIONS);
		CREATE_BAGKEY_WITH_DEFAULT_SCALAR (maximumTimeout, XBOX::VLong, sLONG, DEFAULT_KEEP_ALIVE_TIMEOUT);
		CREATE_BAGKEY_WITH_DEFAULT (logFormat, XBOX::VString, L"No Log File");
		CREATE_BAGKEY_WITH_DEFAULT (logPath, XBOX::VString, HTTP_SERVER_LOG_FILE_PATH);
		CREATE_BAGKEY_WITH_DEFAULT (logFileName, XBOX::VString, HTTP_SERVER_LOG_FILE_NAME);
		CREATE_BAGKEY_WITH_DEFAULT_SCALAR (logMaxSize, XBOX::VLong, sLONG, DEFAULT_LOG_FILE_MAX_SIZE);

		CREATE_BAGKEY_WITH_DEFAULT_SCALAR (allowCompression, XBOX::VBoolean, bool, DEFAULT_COMPRESSION_ENABLED);
		CREATE_BAGKEY_WITH_DEFAULT_SCALAR (compressionMinThreshold, XBOX::VLong, sLONG, DEFAULT_COMPRESSION_MIN_THRESHOLD);
		CREATE_BAGKEY_WITH_DEFAULT_SCALAR (compressionMaxThreshold, XBOX::VLong, sLONG, DEFAULT_COMPRESSION_MAX_THRESHOLD);

		// Log Tokens settings
		namespace Log
		{
			const XBOX::VString kXmlElement ("log");
			CREATE_BAGKEY (field);
		}
	}


	// Web App Service settings
	namespace WebApp
	{
		const XBOX::VString kXmlElement ("webApp");

		CREATE_BAGKEY_WITH_DEFAULT_SCALAR (enabled, XBOX::VBoolean, bool, true);
		CREATE_BAGKEY_WITH_DEFAULT (documentRoot, XBOX::VString, DEFAULT_INDEX_PAGE_NAME);
		CREATE_BAGKEY_WITH_DEFAULT (directoryIndex, XBOX::VString, DEFAULT_WEB_FOLDER_PATH);
	}

	// Resources settings
	/*
		see: http://wiki.4d.fr/techdoc/?q=node/54
	*/
	namespace Resources
	{
		const XBOX::VString kXmlElement ("resources");

		CREATE_BAGKEY_NO_DEFAULT (location, XBOX::VString);
		CREATE_BAGKEY_NO_DEFAULT (locationMatch, XBOX::VString);
		CREATE_BAGKEY_NO_DEFAULT (allow, XBOX::VString);
		CREATE_BAGKEY_NO_DEFAULT (disallow, XBOX::VString);
		CREATE_BAGKEY_NO_DEFAULT (group, XBOX::VString);
		CREATE_BAGKEY_NO_DEFAULT (authType, XBOX::VString);
		CREATE_BAGKEY_NO_DEFAULT (realm, XBOX::VString);
		CREATE_BAGKEY_NO_DEFAULT (expire, XBOX::VString);
		CREATE_BAGKEY_NO_DEFAULT (lifeTime, XBOX::VString);
	}

	// Virtual Folders settings
	namespace VirtualFolders
	{
		const XBOX::VString kXmlElement ("virtualFolder");

		CREATE_BAGKEY_NO_DEFAULT (name, XBOX::VString);
		CREATE_BAGKEY_NO_DEFAULT (location, XBOX::VString);
		CREATE_BAGKEY_NO_DEFAULT (index, XBOX::VString);
	}
}


const XBOX::VValueBag *RetainSettings (const XBOX::VValueBag *inBag, const XBOX::VValueBag::StKey& inElementName)
{
	const VValueBag *bag = NULL;
	const VBagArray *bagArray = inBag->RetainElements (inElementName);

	if ((NULL != bagArray) && (bagArray->GetCount() > 0))
	{
		bag = XBOX::RetainRefCountable (bagArray->GetNth(1));		// take only the first element
	}

	XBOX::QuickReleaseRefCountable (bagArray);

	return bag;
}


const XBOX::VBagArray *RetainMultipleSettings (const XBOX::VValueBag *inBag, const XBOX::VValueBag::StKey& inElementName)
{
	return inBag->RetainElements (inElementName);
}


static
void BuildFolderPath (const XBOX::VFilePath& inBaseFolder, const XBOX::VString& inPath, XBOX::VFilePath& outPath)
{
	if (inPath.IsEmpty())
	{
		outPath.FromFilePath (inBaseFolder);
	}
	else
	{
		XBOX::VString	pathString (inPath);
		
		if ((pathString[0] == CHAR_SOLIDUS) // POSIX Path ?
#if VERSIONWIN
			|| ((pathString.GetLength() > 2) && (pathString[1] == CHAR_COLON) && (pathString[2] == CHAR_SOLIDUS)) // POSIX path like c:/blahblah/
#endif
		)
		{
			if (!pathString.IsEmpty() && (pathString[pathString.GetLength()-1] != CHAR_SOLIDUS))
				pathString.AppendUniChar (CHAR_SOLIDUS);
			
			outPath.FromFullPath (pathString, XBOX::FPS_POSIX);
		}
		else if ((pathString[0] != CHAR_FULL_STOP) && (pathString.FindUniChar (XBOX::FOLDER_SEPARATOR) > 0))
		{
			if (!pathString.IsEmpty() && (pathString[pathString.GetLength()-1] != XBOX::FOLDER_SEPARATOR))
				pathString.AppendUniChar (XBOX::FOLDER_SEPARATOR);
			
			outPath.FromFullPath (pathString, XBOX::FPS_SYSTEM);
		}
		else
		{
			XBOX::VFilePath baseFolder (inBaseFolder);
			
			if ((pathString[0] == CHAR_FULL_STOP) && (pathString[1] == CHAR_SOLIDUS))
				pathString.Remove (1, 2);
			
			while ((pathString[0] == CHAR_FULL_STOP) && (pathString[1] == CHAR_FULL_STOP) && (pathString[2] == CHAR_SOLIDUS))
			{
				pathString.Remove (1, 3);
				baseFolder = baseFolder.ToParent();
			}
			
			pathString.ExchangeAll (CHAR_SOLIDUS, XBOX::FOLDER_SEPARATOR);
			
			if (!pathString.IsEmpty() && (pathString[pathString.GetLength()-1] != XBOX::FOLDER_SEPARATOR))
				pathString.AppendUniChar (XBOX::FOLDER_SEPARATOR);
			
			outPath.FromRelativePath (baseFolder, pathString);
		}
	}
}


//--------------------------------------------------------------------------------------------------


VVirtualFolderSetting::VVirtualFolderSetting()
: fName()
, fLocation()
, fIndex()
, fUseRedirect (false)
{
}


VVirtualFolderSetting::~VVirtualFolderSetting()
{
}


void VVirtualFolderSetting::LoadFromBag (const XBOX::VValueBag& inValueBag, const XBOX::VFilePath& inProjectFolderPath)
{
	inValueBag.GetString (RIASettingsKeys::VirtualFolders::name, fName);
	inValueBag.GetString (RIASettingsKeys::VirtualFolders::location, fLocation);
	inValueBag.GetString (RIASettingsKeys::VirtualFolders::index, fIndex);

	if (HTTPServerTools::BeginsWithASCIICString (fLocation, "http://") ||
		HTTPServerTools::BeginsWithASCIICString (fLocation, "https://"))
	{
		fUseRedirect = true;
	}
	else
	{
		XBOX::VFilePath path;
		BuildFolderPath (inProjectFolderPath, fLocation, path);
		if (path.IsFolder())
		{
			path.GetPath (fLocation);
		}
		else
		{
			XBOX::VFilePath	folder;
			XBOX::VString	string;

			path.GetFolder (folder);
			path.GetFileName (string);
			folder.ToSubFolder (string);
			folder.GetPath (fLocation);
		}
	}
}


void VVirtualFolderSetting::SaveToBag (XBOX::VValueBag& ioValueBag)
{
	if (!fName.IsEmpty())
		ioValueBag.SetString (RIASettingsKeys::VirtualFolders::name, fName);

	if (!fLocation.IsEmpty())
		ioValueBag.SetString (RIASettingsKeys::VirtualFolders::location, fLocation);

	if (!fIndex.IsEmpty())
		ioValueBag.SetString (RIASettingsKeys::VirtualFolders::index, fIndex);
}


//--------------------------------------------------------------------------------------------------


VHTTPServerProjectSettings::VHTTPServerProjectSettings (const XBOX::VValueBag *inBag, const XBOX::VString& inProjectFolderPath)
: fListeningAddress (DEFAULT_LISTENING_ADDRESS)
, fPort (DEFAULT_LISTENING_PORT)
, fAllowSSL (DEFAULT_ALLOW_SSL)
, fSSLMandatory (DEFAULT_SSL_MANDATORY)
, fSSLPort (DEFAULT_LISTENING_SSL_PORT)
, fSSLCertificatesFolderPath (DEFAULT_SSL_CERTIFICATE_FOLDER_PATH)
, fWebFolderPath (DEFAULT_WEB_FOLDER_PATH)
, fIndexPageName (DEFAULT_INDEX_PAGE_NAME)
, fDefaultCharSet (DEFAULT_WEB_CHARSET)
, fLogFormat (DEFAULT_LOG_FORMAT)
, fLogFolderPath ( VFilePath( DEFAULT_LOG_FILE_FOLDER_PATH))	// WARNING: DEFAULT_LOG_FILE_FOLDER_PATH is wrong ./Logs
, fLogFileName (DEFAULT_LOG_FILE_NAME)
, fLogBackupFolderName()
, fLogBackupSettings()
, fRealm()
, fAuthType (AUTH_NONE)
, fResourcesVector()
, fVirtualFoldersVector()
, fLogTokensVector()
, fProjectFolderPath (inProjectFolderPath)
, fUseWALibVirtualFolder (true)
, fEnableCache (DEFAULT_ENABLE_CACHE)
, fCacheMaxSize (DEFAULT_CACHE_MAX_SIZE)
, fCachedObjectMaxSize (DEFAULT_CACHED_OBJECT_MAX_SIZE)
, fEnableCompression (DEFAULT_COMPRESSION_ENABLED)
, fCompressionMinThreshold (DEFAULT_COMPRESSION_MIN_THRESHOLD)
, fCompressionMaxThreshold (DEFAULT_COMPRESSION_MAX_THRESHOLD)
, fEnableKeepAlive (DEFAULT_KEEP_ALIVE_ENABLED)
, fKeepAliveTimeout (DEFAULT_KEEP_ALIVE_TIMEOUT)
, fKeepAliveMaxConnections (DEFAULT_KEEP_ALIVE_MAX_CONNECTIONS)
, fMaxIncomingDataSize (XBOX::MaxLongInt)
, fSignal_SettingsChanged (XBOX::VSignal::ESM_Asynchonous)
{
	LoadFromBag (inBag);
}


VHTTPServerProjectSettings::~VHTTPServerProjectSettings()
{
	fResourcesVector.clear();
	fVirtualFoldersVector.clear();
	fLogTokensVector.clear();
}


void VHTTPServerProjectSettings::ResetToFactorySettings()
{
	SetListeningAddress (DEFAULT_LISTENING_ADDRESS);
	SetListeningPort (DEFAULT_LISTENING_PORT);
	SetAllowSSL (DEFAULT_ALLOW_SSL);
	SetSSLMandatory (DEFAULT_SSL_MANDATORY);
	SetListeningSSLPort (DEFAULT_LISTENING_SSL_PORT);
	SetSSLCertificatesFolderPath (DEFAULT_SSL_CERTIFICATE_FOLDER_PATH);
	SetWebFolderPath (DEFAULT_WEB_FOLDER_PATH);
	SetIndexPageName (DEFAULT_INDEX_PAGE_NAME);
	SetDefaultCharSet (DEFAULT_WEB_CHARSET);
	SetLogFormat (DEFAULT_LOG_FORMAT);
	SetLogFolderPath (VFilePath(DEFAULT_LOG_FILE_FOLDER_PATH));
	SetLogFileName (DEFAULT_LOG_FILE_NAME);
	fLogBackupFolderName.Clear();
	fLogBackupSettings.ResetToFactorySettings();
	fHostName.Clear();
#if HTTP_SERVER_USE_PROJECT_PATTERNS
	fProjectPattern.Clear();
#endif
	fRealm.Clear();
	fAuthType = AUTH_NONE;
	fResourcesVector.clear();
	fVirtualFoldersVector.clear();
	fLogTokensVector.clear();
	fProjectFolderPath.Clear();
	SetUseWALibVirtualFolder (true);
	SetEnableCache (DEFAULT_ENABLE_CACHE);
	SetCacheMaxSize (DEFAULT_CACHE_MAX_SIZE);
	SetCachedObjectMaxSize (DEFAULT_CACHED_OBJECT_MAX_SIZE);
	SetEnableCompression (DEFAULT_COMPRESSION_ENABLED);
	SetCompressionMinThreshold (DEFAULT_COMPRESSION_MIN_THRESHOLD);
	SetCompressionMaxThreshold (DEFAULT_COMPRESSION_MAX_THRESHOLD);
	SetEnableKeepAlive (DEFAULT_KEEP_ALIVE_ENABLED);
	SetKeepAliveTimeout (DEFAULT_KEEP_ALIVE_TIMEOUT);
	SetKeepAliveMaxConnections (DEFAULT_KEEP_ALIVE_MAX_CONNECTIONS);
	SetMaxIncomingDataSize (XBOX::MaxLongInt);

	Tell_SettingsChanged();
}


void VHTTPServerProjectSettings::SetListeningAddress (const XBOX::VString& inIPAddressString)
{
#if WITH_DEPRECATED_IPV4_API
	fListeningAddress = ServerNetTools::GetIPAddress (inIPAddressString);

	if ((fListeningAddress != 0) && (fListeningAddress != LOCALHOST_ADDRESS))
	{
		std::vector<IP4> ipv4Addresses;
		if (ServerNetTools::GetLocalIPv4Addresses (ipv4Addresses) > 0)
		{
			if (!FindValueInVector (ipv4Addresses, fListeningAddress))
				fListeningAddress = 0; // Listening on all IP addresses
		}
	}
#else
	fListeningAddress.FromString (inIPAddressString);

	if ((fListeningAddress != XBOX::VNetAddress::GetAnyIP()) && (fListeningAddress != XBOX::VNetAddress::GetLoopbackIP()))
	{
		XBOX::VectorOfVString	localIPAddresses;

		if (HTTPServerTools::GetLocalIPAddresses (localIPAddresses) > 0)
		{
			if (!FindValueInVector (localIPAddresses, fListeningAddress))
				fListeningAddress.FromString (VNetAddress::GetAnyIP()); // Listening on all IP addresses
		}
	}
#endif	

	Tell_SettingsChanged();
}


void VHTTPServerProjectSettings::SetMaxIncomingDataSize (XBOX::VSize inValue)
{
	fMaxIncomingDataSize = inValue;
	VHTTPRequest::SetMaxIncomingDataSize (fMaxIncomingDataSize);

	Tell_SettingsChanged();
}


void VHTTPServerProjectSettings::SetDefaultAuthType (const XBOX::VString& inAuthenticationMethodName)
{
	fAuthType = HTTPServerTools::GetAuthenticationMethodFromName (inAuthenticationMethodName);

	Tell_SettingsChanged();
}


const VectorOfLogToken& VHTTPServerProjectSettings::GetLogTokens() const
{
	if (fLogTokensVector.empty())
		HTTPServerTools::GetDefaultLogTokenList ((EHTTPServerLogFormat)fLogFormat, fLogTokensVector);

	return fLogTokensVector;
}


void VHTTPServerProjectSettings::SetLogFormat (sLONG inValue, VectorOfLogToken *inTokens)
{
	fLogFormat = inValue;

	if (NULL != inTokens)
	{
		XBOX::VTaskLock lock (&fLogTokensVectorLock);
		fLogTokensVector.clear();
		fLogTokensVector.assign (inTokens->begin(), inTokens->end());
	}

	Tell_SettingsChanged();
}


void VHTTPServerProjectSettings::SetLogFormat (const XBOX::VString& inLogFormatName, VectorOfLogToken *inLogTokens)
{
	SetLogFormat (HTTPServerTools::GetLogFormatFromName (inLogFormatName), inLogTokens);
}


void VHTTPServerProjectSettings::SetLogFormat (const XBOX::VString& inLogFormatName, const XBOX::VectorOfVString& inLogTokensNames)
{
	VectorOfLogToken logTokens;
	
	for (VectorOfVString::const_iterator iter = inLogTokensNames.begin() ; iter != inLogTokensNames.end() ; ++iter)
	{
		EHTTPServerLogToken token = HTTPServerTools::GetLogTokenFromName( *iter);
		if (token != LOG_TOKEN_NONE)
			AppendUniqueValueToVector( logTokens, token);
	}

	SetLogFormat( HTTPServerTools::GetLogFormatFromName (inLogFormatName), &logTokens);
}


bool VHTTPServerProjectSettings::RotateOnSchedule() const
{
	return fLogBackupSettings.RotateOnSchedule();
}


ELogRotationMode VHTTPServerProjectSettings::GetLogRotationMode() const
{
	return fLogBackupSettings.GetLogRotationMode();
}


sLONG VHTTPServerProjectSettings::GetLogMaxSize () const
{
	return fLogBackupSettings.GetLogMaxSize();
}


sLONG VHTTPServerProjectSettings::GetFrequency() const
{
	return fLogBackupSettings.GetFrequency();
}


sLONG VHTTPServerProjectSettings::GetStartingTime() const
{
	return fLogBackupSettings.GetStartingTime();
}


const std::map<sLONG, sLONG>& VHTTPServerProjectSettings::GetDaysHoursMap() const
{
	return fLogBackupSettings.GetDaysHoursMap();
}


void VHTTPServerProjectSettings::SetLogRotationMode (ELogRotationMode inValue)
{
	fLogBackupSettings.SetLogRotationMode (inValue);

	Tell_SettingsChanged();
}


void VHTTPServerProjectSettings::SetLogMaxSize (sLONG inValue)
{
	fLogBackupSettings.SetLogMaxSize (inValue);

	Tell_SettingsChanged();
}


void VHTTPServerProjectSettings::SetFrequency (sLONG inValue)
{
	fLogBackupSettings.SetFrequency (inValue);

	Tell_SettingsChanged();
}


void VHTTPServerProjectSettings::SetStartingTime (sLONG inValue)
{
	fLogBackupSettings.SetStartingTime (inValue);

	Tell_SettingsChanged();
}


void VHTTPServerProjectSettings::SetDaysHoursMap (const std::map<sLONG, sLONG>& inValue)
{
	return fLogBackupSettings.SetDaysHoursMap (inValue);

	Tell_SettingsChanged();
}


void VHTTPServerProjectSettings::LoadResourcesSettingsFromBag (const XBOX::VValueBag& inBag)
{
	const XBOX::VBagArray *resourcesSettings = inBag.RetainElements( RIASettingsKeys::Resources::kXmlElement);
	if (NULL != resourcesSettings)
	{
		XBOX::VIndex count = resourcesSettings->GetCount();

		if (count > 0)
		{
			XBOX::VTaskLock lock (&fResourcesVectorLock);

			fResourcesVector.clear();
			for (XBOX::VIndex i = 1; i <= count; ++i)
			{
				const XBOX::VValueBag *bag = resourcesSettings->GetNth (i);
				if (NULL != bag)
				{
					VHTTPResource *resource = new VHTTPResource();
					if (NULL != resource)
					{
						resource->LoadFromBag (*bag);
						fResourcesVector.push_back (resource);
						resource->Release();
					}
				}
			}
		}

		XBOX::QuickReleaseRefCountable (resourcesSettings);
	}
}


void VHTTPServerProjectSettings::LoadVirtualFoldersSettingsFromBag( const XBOX::VValueBag& inBag)
{
	const XBOX::VBagArray *virtualFoldersSettings = inBag.RetainElements( RIASettingsKeys::VirtualFolders::kXmlElement);
	if (NULL != virtualFoldersSettings)
	{
		XBOX::VIndex count = virtualFoldersSettings->GetCount();

		if (count > 0)
		{
			XBOX::VTaskLock lock (&fVirtualFoldersVectorLock);

			fVirtualFoldersVector.clear();
			for (XBOX::VIndex i = 1; i <= count; ++i)
			{
				const XBOX::VValueBag *bag = virtualFoldersSettings->GetNth (i);
				if (NULL != bag)
				{
					VVirtualFolderSetting *settings = new VVirtualFolderSetting();
					if (NULL != settings)
					{
						settings->LoadFromBag (*bag, fProjectFolderPath);
						fVirtualFoldersVector.push_back (settings);
						settings->Release();
					}
				}
			}
		}

		XBOX::QuickReleaseRefCountable (virtualFoldersSettings);
	}
}


XBOX::VError VHTTPServerProjectSettings::LoadFromBag (const XBOX::VValueBag *inBag)
{
	if (NULL == inBag)
		return XBOX::VE_INVALID_PARAMETER;

#if 0 /*VERSIONDEBUG*/
	XBOX::VString xmlString ("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	inBag->DumpXML (xmlString, CVSTR ("settings"), true);
#endif

	/* Project settings */
	const XBOX::VValueBag *projectSettings = RetainSettings (inBag, RIASettingsKeys::Project::kXmlElement);
	if (projectSettings)
	{
		XBOX::VString ipString = RIASettingsKeys::Project::listen.Get (projectSettings);
#if WITH_DEPRECATED_IPV4_API
		fListeningAddress = ServerNetTools::GetIPAddress (ipString);
#else
		fListeningAddress.FromString (ipString);
#endif		
		fHostName = RIASettingsKeys::Project::hostName.Get (projectSettings);
#if HTTP_SERVER_USE_PROJECT_PATTERNS
		fProjectPattern = RIASettingsKeys::Project::pattern.Get (projectSettings);
#endif

#if WITH_DEPRECATED_IPV4_API
		if ((fListeningAddress != 0) && (fListeningAddress != LOCALHOST_ADDRESS))
#else
		if ((fListeningAddress != XBOX::VNetAddress::GetAnyIP()) && (fListeningAddress != XBOX::VNetAddress::GetLoopbackIP()))
#endif		
		{
#if WITH_DEPRECATED_IPV4_API
			std::vector<IP4> ipv4Addresses;
			if (ServerNetTools::GetLocalIPv4Addresses (ipv4Addresses) > 0)
			{
				if (!FindValueInVector (ipv4Addresses, fListeningAddress))
					fListeningAddress = 0; // Listening on all IP addresses
			}
#else
			XBOX::VectorOfVString	localIPAddresses;

			if (HTTPServerTools::GetLocalIPAddresses (localIPAddresses) > 0)
			{
				if (!FindValueInVector (localIPAddresses, fListeningAddress))
					fListeningAddress.FromString (XBOX::VNetAddress::GetAnyIP()); // Listening on all IP addresses
			}
#endif			
		}

		XBOX::VString authType;
		RIASettingsKeys::Project::realm.Get (projectSettings, fRealm);
		RIASettingsKeys::Project::authType.Get (projectSettings, authType);
		fAuthType = HTTPServerTools::GetAuthenticationMethodFromName (authType);

		XBOX::QuickReleaseRefCountable (projectSettings);
	}


	/*  HTTP Settings */
	const XBOX::VValueBag *httpSettings = RetainSettings (inBag, RIASettingsKeys::HTTP::kXmlElement);
	if (httpSettings)
	{
		fPort = RIASettingsKeys::HTTP::port.Get (httpSettings);
		fAllowSSL = RIASettingsKeys::HTTP::allowSSL.Get (httpSettings);
		fSSLMandatory = RIASettingsKeys::HTTP::SSLMandatory.Get (httpSettings);
		fSSLPort = RIASettingsKeys::HTTP::SSLPort.Get (httpSettings);

		if (fSSLMandatory && !fAllowSSL)
			fAllowSSL = true;

		if (fAllowSSL)
		{
			XBOX::VString certificatePath = RIASettingsKeys::HTTP::SSLCertificatePath.Get (httpSettings);
			BuildFolderPath (fProjectFolderPath, certificatePath, fSSLCertificatesFolderPath);
		}

		XBOX::VString charSetString = RIASettingsKeys::HTTP::standardSet.Get (httpSettings);
		XBOX::CharSet charSet = VTextConverters::Get()->GetCharSetFromName (charSetString);
		fDefaultCharSet = (charSet != XBOX::VTC_UNKNOWN) ? charSet : XBOX::VTC_UTF_8;

		/* cache settings */
		fEnableCache = RIASettingsKeys::HTTP::useCache.Get (httpSettings);
		fCacheMaxSize = RIASettingsKeys::HTTP::pageCacheSize.Get (httpSettings) * 1024;			// expressed in Kilo-Bytes in settings file
		fCachedObjectMaxSize = RIASettingsKeys::HTTP::cachedObjectMaxSize.Get (httpSettings);	// expressed in Bytes in settings file

		/* compression settings */
		fEnableCompression = RIASettingsKeys::HTTP::allowCompression.Get (httpSettings);
		fCompressionMinThreshold = RIASettingsKeys::HTTP::compressionMinThreshold.Get (httpSettings);
		fCompressionMaxThreshold = RIASettingsKeys::HTTP::compressionMaxThreshold.Get (httpSettings);

		/* Keep-Alive settings */
		fEnableKeepAlive = RIASettingsKeys::HTTP::acceptKeepAliveConnections.Get (httpSettings);
/* Temporary disable theses settings loading... because of a bug with long timeout values (sic...)
		fKeepAliveTimeout = RIASettingsKeys::HTTP::maximumTimeout.Get (httpSettings);
		fKeepAliveMaxConnections = RIASettingsKeys::HTTP::maximumRequestsByConnection.Get (httpSettings);
*/

		/* Log settings */
		fLogFormat = HTTPServerTools::GetLogFormatFromName (RIASettingsKeys::HTTP::logFormat.Get (httpSettings));
		SetLogRotationMode (LRC_ROTATE_ON_FILE_SIZE);
		SetLogMaxSize (RIASettingsKeys::HTTP::logMaxSize.Get (httpSettings));
		XBOX::VString logFolderPathString = RIASettingsKeys::HTTP::logPath.Get (httpSettings);
		BuildFolderPath (fProjectFolderPath, logFolderPathString, fLogFolderPath);
		fLogFileName = RIASettingsKeys::HTTP::logFileName.Get (httpSettings);

		const XBOX::VBagArray *logTokens = RetainMultipleSettings (httpSettings, RIASettingsKeys::HTTP::Log::kXmlElement);
		if (NULL != logTokens)
		{
			XBOX::VIndex count = logTokens->GetCount();

			if (count > 0)
			{
				XBOX::VString tokenName;
				XBOX::VTaskLock lock (&fLogTokensVectorLock);

				fLogTokensVector.clear();
				for (XBOX::VIndex i = 1; i <= count; ++i)
				{
					const XBOX::VValueBag *bag = logTokens->GetNth (i);
					if (NULL != bag)
					{
						bag->GetString (RIASettingsKeys::HTTP::Log::field, tokenName);
						if (!tokenName.IsEmpty())
						{
							EHTTPServerLogToken token = HTTPServerTools::GetLogTokenFromName (tokenName);
							if (token != LOG_TOKEN_NONE)
								AppendUniqueValueToVector (fLogTokensVector, token);
						}
					}
				}
			}

			XBOX::QuickReleaseRefCountable (logTokens);
		}

		XBOX::QuickReleaseRefCountable (httpSettings);
	}

	/* Web App settings */
	const XBOX::VValueBag *webAppSettings = RetainSettings (inBag, RIASettingsKeys::WebApp::kXmlElement);
	if (webAppSettings)
	{
		XBOX::VString webFolderPath;
		webFolderPath = RIASettingsKeys::WebApp::documentRoot.Get (webAppSettings);
		BuildFolderPath (fProjectFolderPath, webFolderPath, fWebFolderPath);
		fIndexPageName = RIASettingsKeys::WebApp::directoryIndex.Get (webAppSettings);

		XBOX::QuickReleaseRefCountable (webAppSettings);
	}

	/* Resources settings */
	LoadResourcesSettingsFromBag( *inBag);

	/* Virtual Folders settings */
	LoadVirtualFoldersSettingsFromBag( *inBag);

	Tell_SettingsChanged();

	return XBOX::VE_OK;
}


XBOX::VError VHTTPServerProjectSettings::SaveToBag (XBOX::VValueBag *outBag)
{
	if (NULL == outBag)
		return XBOX::VE_INVALID_PARAMETER;

	XBOX::VValueBag *projectSettings = new XBOX::VValueBag ();
	if (projectSettings)
	{
		XBOX::VString	ipString;
		XBOX::VString	authTypeString;

#if WITH_DEPRECATED_IPV4_API
		ServerNetTools::GetIPAdress (fListeningAddress, ipString);
#else
		ipString.FromString (fListeningAddress);
#endif
		HTTPServerTools::GetAuthenticationMethodName (fAuthType, authTypeString);

		projectSettings->SetLong (RIASettingsKeys::Project::refCount, GetRefCount());
		projectSettings->SetString (RIASettingsKeys::Project::listen, ipString);
		projectSettings->SetString (RIASettingsKeys::Project::hostName, fHostName);
#if HTTP_SERVER_USE_PROJECT_PATTERNS
		projectSettings->SetString (RIASettingsKeys::Project::pattern, fProjectPattern);
#endif
		projectSettings->SetString (RIASettingsKeys::Project::realm, fRealm);
		projectSettings->SetString (RIASettingsKeys::Project::authType, authTypeString);
		outBag->AddElement (RIASettingsKeys::Project::kXmlElement, projectSettings);
		XBOX::ReleaseRefCountable (&projectSettings);
	}

	XBOX::VValueBag *httpSettings = new XBOX::VValueBag ();
	if (httpSettings)
	{
		XBOX::VString posixPath;

		httpSettings->SetLong (RIASettingsKeys::HTTP::port, fPort);
		httpSettings->SetBool (RIASettingsKeys::HTTP::allowSSL, fAllowSSL);
		httpSettings->SetBool (RIASettingsKeys::HTTP::SSLMandatory, fSSLMandatory);
		httpSettings->SetLong (RIASettingsKeys::HTTP::SSLPort, fSSLPort);

		if (fSSLCertificatesFolderPath.IsValid())
		{
			fSSLCertificatesFolderPath.GetPosixPath (posixPath);
			httpSettings->SetString (RIASettingsKeys::HTTP::SSLCertificatePath, posixPath);
		}

		XBOX::VString charSetName;
		VTextConverters::Get()->GetNameFromCharSet (fDefaultCharSet, charSetName);
		httpSettings->SetString (RIASettingsKeys::HTTP::standardSet, charSetName);

		/* cache settings */
		httpSettings->SetBool (RIASettingsKeys::HTTP::useCache, fEnableCache);
		httpSettings->SetLong (RIASettingsKeys::HTTP::pageCacheSize, fCacheMaxSize);
		httpSettings->SetLong (RIASettingsKeys::HTTP::cachedObjectMaxSize, fCachedObjectMaxSize);

		/* compression settings */
		httpSettings->SetBool (RIASettingsKeys::HTTP::allowCompression, fEnableCompression);
		httpSettings->SetLong (RIASettingsKeys::HTTP::compressionMinThreshold, fCompressionMinThreshold);
		httpSettings->SetLong (RIASettingsKeys::HTTP::compressionMaxThreshold, fCompressionMaxThreshold);

		/* Keep-Alive settings */
		httpSettings->SetBool (RIASettingsKeys::HTTP::acceptKeepAliveConnections, fEnableKeepAlive);
		httpSettings->SetLong (RIASettingsKeys::HTTP::maximumTimeout, fKeepAliveTimeout);
		httpSettings->SetLong (RIASettingsKeys::HTTP::maximumRequestsByConnection, fKeepAliveMaxConnections);

		/* Log settings */
		XBOX::VString logFormatName;
		HTTPServerTools::GetLogFormatName ((EHTTPServerLogFormat)fLogFormat, logFormatName);
		posixPath.Clear();
		fLogFolderPath.GetPosixPath (posixPath);
		httpSettings->SetString (RIASettingsKeys::HTTP::logFormat, logFormatName);
		httpSettings->SetString (RIASettingsKeys::HTTP::logPath, posixPath);
		httpSettings->SetString (RIASettingsKeys::HTTP::logFileName, fLogFileName);
		httpSettings->SetLong (RIASettingsKeys::HTTP::logMaxSize, GetLogMaxSize());

		/* Log Tokens settings */
		XBOX::VBagArray *logTokenSettings = new XBOX::VBagArray ();
		if (NULL != logTokenSettings)
		{
			XBOX::VTaskLock lock (&fLogTokensVectorLock);
			XBOX::VString	tokenName;

			for (VectorOfLogToken::const_iterator it = fLogTokensVector.begin(); it != fLogTokensVector.end(); ++it)
			{
				XBOX::VValueBag *bag = new XBOX::VValueBag ();
				if (NULL != bag)
				{
					HTTPServerTools::GetLogTokenName (*it, tokenName);
					bag->SetString (RIASettingsKeys::HTTP::Log::field, tokenName);
					logTokenSettings->AddTail (bag);
					bag->Release();
				}
			}

			if (logTokenSettings->GetCount())
				httpSettings->SetElements (RIASettingsKeys::HTTP::Log::kXmlElement, logTokenSettings);

			XBOX::ReleaseRefCountable (&logTokenSettings);
		}

		outBag->AddElement (RIASettingsKeys::HTTP::kXmlElement, httpSettings);
		XBOX::ReleaseRefCountable (&httpSettings);
	}

	/* Web App settings */
	XBOX::VValueBag *webAppSettings = new XBOX::VValueBag ();
	if (webAppSettings)
	{
		XBOX::VString posixPath;
		fWebFolderPath.GetPosixPath (posixPath);
		webAppSettings->SetString (RIASettingsKeys::WebApp::documentRoot, posixPath);
		webAppSettings->SetString (RIASettingsKeys::WebApp::directoryIndex, fIndexPageName);

		outBag->AddElement (RIASettingsKeys::WebApp::kXmlElement, webAppSettings);
		XBOX::ReleaseRefCountable (&webAppSettings);
	}

	/* Resources settings */
	XBOX::VBagArray *resourcesSettings = new XBOX::VBagArray ();
	if (resourcesSettings)
	{
		XBOX::VTaskLock lock (&fResourcesVectorLock);

		for (VHTTPResourcesVector::const_iterator it = fResourcesVector.begin(); it != fResourcesVector.end(); ++it)
		{
			if (!it->IsNull())
			{
				XBOX::VValueBag *bag = new XBOX::VValueBag ();
				if (bag)
				{
					(*it)->SaveToBag (*bag);
					resourcesSettings->AddTail (bag);
					bag->Release();
				}
			}
		}

		if (resourcesSettings->GetCount())
			outBag->SetElements (RIASettingsKeys::Resources::kXmlElement, resourcesSettings);

		XBOX::ReleaseRefCountable (&resourcesSettings);
	}

#if 0 /*VERSIONDEBUG*/
	XBOX::VString xmlString ("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	outBag->DumpXML (xmlString, CVSTR ("settings"), true);
#endif

	return XBOX::VE_OK;
}


void VHTTPServerProjectSettings::SetSSLCertificatesFolderPath (const XBOX::VFilePath& inValue)
{
	BuildFolderPath (fProjectFolderPath, inValue.GetPath(), fSSLCertificatesFolderPath);

	Tell_SettingsChanged();
}


void VHTTPServerProjectSettings::SetSSLCertificatesFolderPath (const XBOX::VString& inValue)
{
	BuildFolderPath (fProjectFolderPath, inValue, fSSLCertificatesFolderPath);

	Tell_SettingsChanged();
}


void VHTTPServerProjectSettings::SetWebFolderPath (const XBOX::VFilePath& inValue)
{
	BuildFolderPath (fProjectFolderPath, inValue.GetPath(), fWebFolderPath);

	Tell_SettingsChanged();
}


void VHTTPServerProjectSettings::SetWebFolderPath (const XBOX::VString& inValue)
{
	BuildFolderPath (fProjectFolderPath, inValue, fWebFolderPath);

	Tell_SettingsChanged();
}


VSignalSettingsChanged *VHTTPServerProjectSettings::GetSignal_SettingsChanged()
{
	return &fSignal_SettingsChanged;
}


void VHTTPServerProjectSettings::Tell_SettingsChanged()
{
	fSignal_SettingsChanged (this);
}
