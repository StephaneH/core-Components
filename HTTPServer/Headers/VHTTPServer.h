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
#ifndef __HTTP_SERVER_INCLUDED__
#define __HTTP_SERVER_INCLUDED__


#include "../Interfaces/CHTTPServer.h"

#include "ServerNet/VServerNet.h"
#include "VHTTPConnectionListener.h"


class VHTTPServerProjectSettings;
class VVirtualHostManager;
class VVirtualHost;
class VVirtualFolder;


class VHTTPServerSleepHandler : public XBOX::VSleepNotifier::VSleepHandler
{
public:
											VHTTPServerSleepHandler() : fSleepNotifierState (SNS_AWAKE) {};
	virtual									~VHTTPServerSleepHandler() {};


	virtual void							OnSleepNotification();
	virtual void							OnWakeUp();
	virtual const VString					GetInfoTag();

	bool									IsSleeping();

	typedef enum SleepNotifierState
	{
		SNS_AWAKE = 0,
		SNS_ASLEEP
	} SleepNotifierState;

private:
	sLONG									fSleepNotifierState;
};


class VHTTPServer : public VComponentImp<CHTTPServer>
{
public:
											VHTTPServer ();
	virtual									~VHTTPServer();

	static void								Init();
	static void								InitLate();
	static void								Deinit();

	virtual XBOX::VError					StopConnectionHandler (XBOX::VTaskID nTaskID);

	virtual IHTTPServerProject *			NewHTTPServerProject (const XBOX::VFilePath& inProjectFolderPath, bool inAppend);
	virtual IHTTPServerProject *			NewHTTPServerProject (XBOX::VFileSystemNamespace *inFileSystemNameSpace, bool inAppend);
	virtual XBOX::VError					RemoveHTTPServerProject (IHTTPServerProject *inHTTPServerProject);
	virtual XBOX::VError					AppendHTTPServerProject (IHTTPServerProject *inHTTPServerProject);
	virtual void							SetRequestLogger (IRequestLogger * inRequestLogger) { fRequestLogger = inRequestLogger; }
	virtual IHTTPWebsocketServer*			NewHTTPWebsocketServerHandler();
	virtual IHTTPWebsocketClient*			NewHTTPWebsocketClientHandler();
	virtual VCacheManager *					GetCacheManager() const { return fCacheManager; }

	IConnectionListener *					FindConnectionListener (const VHTTPServerProjectSettings *inSettings);
	IConnectionListener *					CreateConnectionListener (const VHTTPServerProjectSettings *inSettings);
	XBOX::VError							RemoveConnectionListener (IConnectionListener *inConnectionHandler);

	virtual XBOX::VError					AddVirtualHost (const VVirtualHost *inVirtualHost);

	VVirtualFolder *						RetainVirtualFolder (const XBOX::VString& inLocationPath, const XBOX::VString& inIndexPage, const XBOX::VString& inKeyword, XBOX::VFileSystemNamespace *inFileSystemNameSpace = NULL);

	VVirtualHostManager *					GetVirtualHostManager() const { return fVirtualHostManager; }

	IRequestLogger *						GetRequestLogger() { return fRequestLogger; }

	static XBOX::VFolder *					RetainComponentFolder (BundleFolderKind inFolderKind = kBF_BUNDLE_FOLDER);
	static void								GetErrorTemplateString (XBOX::VString& outErrorTemplateString);

	static bool								GetZipComponentAvailable() { return (NULL != fZipComponent); }
	static CZipComponent *					RetainZipComponent() { return XBOX::RetainRefCountable (fZipComponent); }

	void									SaveToBag (XBOX::VValueBag &ioBag);

	static XBOX::VError						ThrowError (const XBOX::VError inErrorCode, const XBOX::VString& inParamString);

	static void								LocalizeString (const XBOX::VString& inString, XBOX::VString& outLocalizedString);
	static void								LocalizeErrorMessage (const XBOX::VError inErrorCode, XBOX::VString& outLocalizedString);

	static const XBOX::VString&				GetServerName()	{ return fServerName; }
	static const XBOX::VString&				GetServerVersion() { return fServerVersion; }
	static bool								GetShowExtendedInfos() { return fShowExtendedInfos; }

	/* Export some utilities functions from HTTPServerTools.h */
	virtual bool							EqualASCIIVString (const XBOX::VString& inString1, const XBOX::VString& inString2, bool isCaseSensitive = false);
	virtual sLONG							FindASCIIVString (const XBOX::VString& inText, const XBOX::VString& inPattern, bool isCaseSensitive = false);

	virtual void							MakeRFC822GMTDateString (const sLONG inMode, XBOX::VString& outDateString,bool withHeaderName = false, sLONG inTimeout = 5 * 60L);
	virtual void							MakeIPv4String (const uLONG inIPv4, XBOX::VString& outString);
	virtual void							MakeServerString (XBOX::VString& outServerString, bool inSecureMode = false, bool withHeaderName = false);

	/* Export some utilities functions from HTTPProtocol.h */
	virtual bool							MakeHTTPAuthenticateHeaderValue (const HTTPAuthenticationMethod inMethod, const XBOX::VString& inRealm, const XBOX::VString& inDomain, XBOX::VString& outAuthString);
	virtual void							SetServerInformations (const XBOX::VString& inServerName, const XBOX::VString& inServerVersion, bool withExtendedInformations = false);

	/* Export some functions from VMimeTypeManager */
	virtual void							AppendMimeType (const XBOX::VString& inContentType, const XBOX::VString& inExtensions, bool isCompressible = false, bool isParsable = false);
	virtual void							FindContentType (const XBOX::VString& inExtension, XBOX::VString& outContentType, bool *outIsCompressible = NULL, bool *outIsParsable = NULL);
	virtual bool							IsMimeTypeCompressible (const XBOX::VString& inContentType);
	virtual bool							IsMimeTypeParsable (const XBOX::VString& inContentType);
	virtual MimeTypeKind					GetMimeTypeKind (const XBOX::VString& inContentType);

	/* Check that project uses distinct ports, SSLPorts or hostnames settings from other projects */
	bool									CheckProjectSanity (const VHTTPServerProjectSettings *inSettings);

	/*
	 *	Tell if Server received a Sleep Notification from System
	 */
	bool									IsSleeping();

	typedef std::vector<XBOX::VRefPtr<IHTTPServerProject> >		VectorOfHTTPServerProjects;
	typedef std::vector<VHTTPConnectionListener *>				VectorOfHTTPConnectionListener;

private:
	VServer *								fServerNet;
	VectorOfHTTPConnectionListener			fConnectionListeners;
	VCriticalSection						fConnectionListenersLock;
	IRequestLogger *						fRequestLogger;
	VVirtualHostManager *					fVirtualHostManager;
	VectorOfHTTPServerProjects				fHTTPServerProjects;
	VCriticalSection						fHTTPServerProjectsLock;
	VCacheManager *							fCacheManager;
	VHTTPServerSleepHandler *				fSleepHandler;

	static CZipComponent *					fZipComponent;
	static bool								fZipComponentInited;
	static XBOX::VRefPtr<VLocalizationManager>	fLocalizationManager;

	static XBOX::VString					fServerName;
	static XBOX::VString					fServerVersion;
	static bool								fShowExtendedInfos;

	/* private methods */
	XBOX::VFolder *							_RetainExecutableFolder() const;
	XBOX::VFolder *							_RetainApplicationPackageFolder() const;
	XBOX::VFolder *							_RetainApplicationPackageContentFolder() const;

	void									_BuildCertificatesFilePathes (const XBOX::VFilePath& inPath, XBOX::VFilePath& outCertFilePath, XBOX::VFilePath& outKeyFilePath);
};


#endif	// __HTTP_SERVER_INCLUDED__

