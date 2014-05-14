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
#ifndef __HTTP_SERVER_SETTINGS_INCLUDED__
#define __HTTP_SERVER_SETTINGS_INCLUDED__


namespace RIASettingsKeys
{
	// Settings constants keys

	// Project settings
	namespace Project
	{
		extern const XBOX::VString kXmlElement;

		EXTERN_BAGKEY (refCount);
		EXTERN_BAGKEY_WITH_DEFAULT (publicName, XBOX::VString);
		EXTERN_BAGKEY_WITH_DEFAULT_SCALAR (administrator, XBOX::VBoolean, bool);
		EXTERN_BAGKEY_WITH_DEFAULT (hostName, XBOX::VString);
		EXTERN_BAGKEY_WITH_DEFAULT (pattern, XBOX::VString);
		EXTERN_BAGKEY_WITH_DEFAULT (listen, XBOX::VString);
		EXTERN_BAGKEY_WITH_DEFAULT (responseFormat, XBOX::VString);
		EXTERN_BAGKEY_NO_DEFAULT (realm, XBOX::VString);
		EXTERN_BAGKEY_NO_DEFAULT (authType, XBOX::VString);

		namespace HTTPServer
		{
			EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( started, XBOX::VBoolean, bool);
		}
	}

	// HTTP Server settings
	namespace HTTP
	{
		extern const XBOX::VString kXmlElement;

		EXTERN_BAGKEY_WITH_DEFAULT_SCALAR (port, XBOX::VLong, sLONG);
		EXTERN_BAGKEY_WITH_DEFAULT_SCALAR (maximumProcess, XBOX::VLong, sLONG);
		EXTERN_BAGKEY_WITH_DEFAULT (SSLCertificatePath, XBOX::VString);
		EXTERN_BAGKEY_WITH_DEFAULT_SCALAR (allowSSL, XBOX::VBoolean, bool);
		EXTERN_BAGKEY_WITH_DEFAULT_SCALAR (SSLMandatory, XBOX::VBoolean, bool);
		EXTERN_BAGKEY_WITH_DEFAULT_SCALAR (allowHttpOnLocal, XBOX::VBoolean, bool);
		EXTERN_BAGKEY_WITH_DEFAULT_SCALAR (SSLPort, XBOX::VLong, sLONG);
		EXTERN_BAGKEY_WITH_DEFAULT_SCALAR (SSLMinVersion, XBOX::VLong, sLONG);
		EXTERN_BAGKEY_WITH_DEFAULT_SCALAR (useCache, XBOX::VBoolean, bool);
		EXTERN_BAGKEY_WITH_DEFAULT_SCALAR (pageCacheSize, XBOX::VLong, sLONG);
		EXTERN_BAGKEY_WITH_DEFAULT_SCALAR (cachedObjectMaxSize, XBOX::VLong, sLONG);
		EXTERN_BAGKEY_WITH_DEFAULT_SCALAR (inactiveProcessTimeout, XBOX::VLong, sLONG);
		EXTERN_BAGKEY_WITH_DEFAULT_SCALAR (sendExtendedCharacterSetDirectly, XBOX::VBoolean, bool);
		EXTERN_BAGKEY_WITH_DEFAULT (standardSet, XBOX::VString);
		EXTERN_BAGKEY_WITH_DEFAULT_SCALAR (acceptKeepAliveConnections, XBOX::VBoolean, bool);
		EXTERN_BAGKEY_WITH_DEFAULT_SCALAR (maximumRequestsByConnection, XBOX::VLong, sLONG);
		EXTERN_BAGKEY_WITH_DEFAULT_SCALAR (maximumTimeout, XBOX::VLong, sLONG);
		EXTERN_BAGKEY_WITH_DEFAULT (logFormat, XBOX::VString);
		EXTERN_BAGKEY_WITH_DEFAULT (logPath, XBOX::VString);
		EXTERN_BAGKEY_WITH_DEFAULT (logFileName, XBOX::VString);
		EXTERN_BAGKEY_WITH_DEFAULT_SCALAR (logMaxSize, XBOX::VLong, sLONG);

		EXTERN_BAGKEY_WITH_DEFAULT_SCALAR (allowCompression, XBOX::VBoolean, bool);
		EXTERN_BAGKEY_WITH_DEFAULT_SCALAR (compressionMinThreshold, XBOX::VLong, sLONG);
		EXTERN_BAGKEY_WITH_DEFAULT_SCALAR (compressionMaxThreshold, XBOX::VLong, sLONG);

		// Log Tokens settings
		namespace Log
		{
			extern const XBOX::VString kXmlElement;
			EXTERN_BAGKEY (field);
		}
	}

	// Web App Service settings
	namespace WebApp
	{
		extern const XBOX::VString kXmlElement;

		EXTERN_BAGKEY_WITH_DEFAULT_SCALAR (enabled, XBOX::VBoolean, bool);
		EXTERN_BAGKEY_WITH_DEFAULT (documentRoot, XBOX::VString);
		EXTERN_BAGKEY_WITH_DEFAULT (directoryIndex, XBOX::VString);
	}

	// Resources settings
	/*
		see: http://wiki.4d.fr/techdoc/?q=node/54
	*/
	namespace Resources
	{
		extern const XBOX::VString kXmlElement;

		EXTERN_BAGKEY_NO_DEFAULT (location, XBOX::VString);
		EXTERN_BAGKEY_NO_DEFAULT (locationMatch, XBOX::VString);
		EXTERN_BAGKEY_NO_DEFAULT (urlPath, XBOX::VString);
		EXTERN_BAGKEY_NO_DEFAULT (urlRegex, XBOX::VString);
		EXTERN_BAGKEY_NO_DEFAULT (allow, XBOX::VString);
		EXTERN_BAGKEY_NO_DEFAULT (disallow, XBOX::VString);
		EXTERN_BAGKEY_NO_DEFAULT (group, XBOX::VString);
		EXTERN_BAGKEY_NO_DEFAULT (authType, XBOX::VString);
		EXTERN_BAGKEY_NO_DEFAULT (realm, XBOX::VString);
		EXTERN_BAGKEY_NO_DEFAULT (expire, XBOX::VString);
		EXTERN_BAGKEY_NO_DEFAULT (lifeTime, XBOX::VString);
	}

	// Virtual Folders settings
	namespace VirtualFolders
	{
		extern const XBOX::VString kXmlElement;

		EXTERN_BAGKEY_NO_DEFAULT (name, XBOX::VString);
		EXTERN_BAGKEY_NO_DEFAULT (localPath, XBOX::VString);
		EXTERN_BAGKEY_NO_DEFAULT (location, XBOX::VString); /* Used for REDIRECT only*/
		EXTERN_BAGKEY_NO_DEFAULT (index, XBOX::VString);
	}
}


//--------------------------------------------------------------------------------------------------


class VHTTPResource;
class VHTTPServerProjectSettings;
typedef std::vector<XBOX::VRefPtr<VHTTPResource> >		VHTTPResourcesVector;
typedef XBOX::VSignalT_1<VHTTPServerProjectSettings *>	VSignalSettingsChanged;


class VVirtualFolderSetting : public XBOX::VObject, public XBOX::IRefCountable
{
public:
										VVirtualFolderSetting();
	virtual								~VVirtualFolderSetting();

	void								LoadFromBag (const XBOX::VValueBag& inValueBag, XBOX::VFileSystemNamespace *inFileSystemNamespace);
	void								LoadFromJSONValue (const XBOX::VJSONValue& inJSONValue, XBOX::VFileSystemNamespace *inFileSystemNamespace);
	void								SaveToBag (XBOX::VValueBag& ioValueBag);

	const XBOX::VString&				GetURLPath() const { return fURLPath; }
	const XBOX::VString&				GetLocation() const { return fLocation; }
	const XBOX::VString&				GetIndex() const { return fIndex; }

protected:
	void								_VerifyAndUpdateLocation (XBOX::VFileSystemNamespace *inFileSystemNameSpace);

private:
	XBOX::VString						fURLPath;
	XBOX::VString						fLocation;
	XBOX::VString						fIndex;
	bool								fUseRedirect;
};

typedef std::vector<XBOX::VRefPtr<VVirtualFolderSetting> > VVirtualFolderSettingsVector;


class VHTTPServerProjectSettings : public XBOX::VObject, public IHTTPServerProjectSettings
{
public:
										VHTTPServerProjectSettings (XBOX::VFileSystemNamespace *inFileSystemNameSpace);
	virtual								~VHTTPServerProjectSettings();

	XBOX::VError						LoadFromBag (const XBOX::VValueBag *inBag);
	XBOX::VError						SaveToBag (XBOX::VValueBag *outBag);

	void								ResetToFactorySettings();

	/* Configuration settings accessors */
	const XBOX::VString&				GetListeningAddress() const { return fListeningAddress; }	
	PortNumber							GetListeningPort() const { return fPort; }
	bool								GetAllowSSL() const { return fAllowSSL; }
	bool								GetSSLMandatory() const { return fSSLMandatory; }
	bool								GetAllowHTTPOnLocal() const { return fAllowHTTPOnLocal; }
	PortNumber							GetListeningSSLPort() const { return fSSLPort; }
	const XBOX::VFilePath&				GetSSLCertificatesFolderPath() const { return fSSLCertificatesFolderPath; }
	const XBOX::VFilePath&				GetWebFolderPath() const { return fWebFolderPath; }
	const XBOX::VString&				GetIndexPageName() const { return fIndexPageName; }
	XBOX::CharSet						GetDefaultCharSet() const { return fDefaultCharSet; }
	const XBOX::VString&				GetHostName() const { return fHostName; }
#if HTTP_SERVER_USE_PROJECT_PATTERNS
	const XBOX::VString&				GetProjectPattern() const { return fProjectPattern; }
#endif
	const XBOX::VString&				GetDefaultRealm() const { return fRealm; }
	HTTPAuthenticationMethod			GetDefaultAuthType() const { return fAuthType; }

	void								SetListeningAddress (const XBOX::VString& inIPAddressString);
	void								SetListeningPort (PortNumber inValue) { fPort = inValue; }
	void								SetAllowSSL (bool inValue) { fAllowSSL = inValue; }
	void								SetSSLMandatory (bool inValue) { fSSLMandatory = inValue; }
	void								SetAllowHTTPOnLocal (bool inValue) { fAllowHTTPOnLocal = inValue; }
	void								SetListeningSSLPort (PortNumber inValue) { fSSLPort = inValue; }
	void								SetSSLCertificatesFolderPath (const XBOX::VFilePath& inValue);
	void								SetSSLCertificatesFolderPath (const XBOX::VString& inValue);
	void								SetWebFolderPath (const XBOX::VFilePath& inValue);
	void								SetWebFolderPath (const XBOX::VString& inValue);
	void								SetIndexPageName (const XBOX::VString& inValue) { fIndexPageName.FromString (inValue); }
	void								SetDefaultCharSet (const XBOX::CharSet inValue) { fDefaultCharSet = inValue; }
	void								SetHostName (const XBOX::VString& inValue) { fHostName.FromString (inValue); }
#if HTTP_SERVER_USE_PROJECT_PATTERNS
	void								SetProjectPattern (const XBOX::VString& inValue) { fProjectPattern.FromString (inValue); }
#endif
	void								SetDefaultRealm (const XBOX::VString& inValue) { fRealm.FromString (inValue); }
	void								SetDefaultAuthType (HTTPAuthenticationMethod inValue) { fAuthType = inValue; }
	void								SetDefaultAuthType (const XBOX::VString& inAuthenticationMethodName);

	/* Cache settings accessors */
	bool								GetEnableCache() const { return fEnableCache; }
	sLONG								GetCacheMaxSize() const { return fCacheMaxSize; }
	sLONG								GetCachedObjectMaxSize() const { return fCachedObjectMaxSize; }

	void								SetEnableCache (bool inValue) { fEnableCache = inValue; }
	void								SetCacheMaxSize (sLONG inValue) { fCacheMaxSize = inValue; }
	void								SetCachedObjectMaxSize (sLONG inValue) { fCachedObjectMaxSize = inValue; }

	/* Compression settings */
	bool								GetEnableCompression() const { return fEnableCompression; }
	sLONG								GetCompressionMinThreshold() const { return fCompressionMinThreshold; }
	sLONG								GetCompressionMaxThreshold() const { return fCompressionMaxThreshold; }

	void								SetEnableCompression (bool inValue) { fEnableCompression = inValue; }
	void								SetCompressionMinThreshold (sLONG inValue) { fCompressionMinThreshold = inValue; }
	void								SetCompressionMaxThreshold (sLONG inValue) { fCompressionMaxThreshold = inValue; }

	/* Keep-Alive Settings */
	bool								GetEnableKeepAlive() const { return fEnableKeepAlive; }
	sLONG								GetKeepAliveTimeout() const { return fKeepAliveTimeout; }
	sLONG								GetKeepAliveMaxConnections() const { return fKeepAliveMaxConnections; }

	void								SetEnableKeepAlive (bool inValue) { fEnableKeepAlive = inValue; }
	void								SetKeepAliveTimeout (sLONG inValue) { fKeepAliveTimeout = inValue; }
	void								SetKeepAliveMaxConnections (sLONG inValue) { fKeepAliveMaxConnections = inValue; }

	XBOX::VSize							GetMaxIncomingDataSize() const { return fMaxIncomingDataSize; }
	void								SetMaxIncomingDataSize (XBOX::VSize inValue);

	/* Log settings */
	sLONG								GetLogFormat() const { return fLogFormat; }
	const VectorOfLogToken&				GetLogTokens() const;
	const XBOX::VFilePath&				GetLogFolderPath() const { return fLogFolderPath; }
	const XBOX::VString&				GetLogFileName() const { return fLogFileName; }
	const XBOX::VString&				GetLogBackupFolderName() const { return fLogBackupFolderName; }

	void								SetLogFormat (sLONG inValue, VectorOfLogToken *inTokens = NULL);
	void								SetLogFormat (const XBOX::VString& inLogFormatName, VectorOfLogToken *inLogTokens = NULL);
	void								SetLogFormat (const XBOX::VString& inLogFormatName, const XBOX::VectorOfVString& inLogTokensNames);
	void								SetLogFolderPath (const XBOX::VFilePath& inValue) { fLogFolderPath.FromFilePath (inValue); }
	void								SetLogFileName (const XBOX::VString& inValue) { fLogFileName.FromString (inValue); }
	void								SetLogBackupFolderName (const XBOX::VString& inValue) { fLogBackupFolderName.FromString (inValue); }

	/* Log Rotation Setting */
	bool								RotateOnSchedule() const;

	const VHTTPServerLogBackupSettings&	GetLogBackupSettings() const { return fLogBackupSettings; }

	ELogRotationMode					GetLogRotationMode() const;
	sLONG								GetLogMaxSize () const;
	sLONG								GetFrequency() const;
	sLONG								GetStartingTime() const;
	const std::map<sLONG, sLONG>&		GetDaysHoursMap() const;

	void								SetLogRotationMode (ELogRotationMode inValue);
	void								SetLogMaxSize (sLONG inValue);
	void								SetFrequency (sLONG inValue);
	void								SetStartingTime (sLONG inValue);
	void								SetDaysHoursMap (const std::map<sLONG, sLONG>& inValue);

	/* The bag should contain some "resources" elements */
	void								LoadResourcesSettingsFromBag( const XBOX::VValueBag& inBag);
	void								LoadResourcesSettingsFromJSONFile (const XBOX::VFilePath& inFilePath);
	const VHTTPResourcesVector&			GetResourcesVector() const { return fResourcesVector; }

	/* The bag should contain some "virtualFolder" elements */
	void								LoadVirtualFoldersSettingsFromBag( const XBOX::VValueBag& inBag);
	void								LoadVirtualFoldersSettingsFromJSONFile (const XBOX::VFilePath& inFilePath);
	const VVirtualFolderSettingsVector&	GetVirtualFoldersVector() const { return fVirtualFoldersVector; }

	/* Signals */
	VSignalSettingsChanged *			GetSignal_SettingsChanged();
	void								Tell_SettingsChanged();

protected:
	/* Configuration settings */
	XBOX::VString						fListeningAddress;
	PortNumber							fPort;
	bool								fAllowSSL;
	bool								fSSLMandatory;
    bool                                fAllowHTTPOnLocal;
	PortNumber							fSSLPort;
	XBOX::VFilePath						fSSLCertificatesFolderPath;
	XBOX::VFilePath						fWebFolderPath;
	XBOX::VString						fIndexPageName;
	XBOX::CharSet						fDefaultCharSet;
	XBOX::VString						fHostName;
#if HTTP_SERVER_USE_PROJECT_PATTERNS
	XBOX::VString						fProjectPattern;	// To handle http://localhost/myApp1/index.html
#endif

	/* Cache settings */
	bool								fEnableCache;
	sLONG								fCacheMaxSize;
	sLONG								fCachedObjectMaxSize;

	/* Compression settings */
	bool								fEnableCompression;
	sLONG								fCompressionMinThreshold;
	sLONG								fCompressionMaxThreshold;

	/* Keep-Alive Settings */
	bool								fEnableKeepAlive;
	sLONG								fKeepAliveTimeout;
	sLONG								fKeepAliveMaxConnections;

	/* Max Incoming Data Size */
	XBOX::VSize							fMaxIncomingDataSize;

	/* Log settings */
	sLONG								fLogFormat;
	XBOX::VFilePath						fLogFolderPath;
	XBOX::VString						fLogFileName;
	mutable VectorOfLogToken			fLogTokensVector;
	XBOX::VCriticalSection				fLogTokensVectorLock;
	XBOX::VString						fLogBackupFolderName;

	/* Log Rotation Setting */
	VHTTPServerLogBackupSettings		fLogBackupSettings;

	/* Project Realm & Authentication Type */
	XBOX::VString						fRealm;
	HTTPAuthenticationMethod			fAuthType;

	/* Resources Settings */
	VHTTPResourcesVector				fResourcesVector;
	XBOX::VCriticalSection				fResourcesVectorLock;

	/* Virtual Folders Settings */
	VVirtualFolderSettingsVector		fVirtualFoldersVector;
	XBOX::VCriticalSection				fVirtualFoldersVectorLock;

	/* Default Project Folder */
	XBOX::VFileSystemNamespace *		fFileSystemNameSpace;
	XBOX::VFileSystem *					fProjectFileSystem;

	/* Signals */
	VSignalSettingsChanged				fSignal_SettingsChanged;
};


const XBOX::VValueBag *RetainSettings (const XBOX::VValueBag *inBag, const XBOX::VValueBag::StKey& inElementName);
const XBOX::VBagArray *RetainMultipleSettings (const XBOX::VValueBag *inBag, const XBOX::VValueBag::StKey& inElementName);


#endif	// __HTTP_SERVER_SETTINGS_INCLUDED__
