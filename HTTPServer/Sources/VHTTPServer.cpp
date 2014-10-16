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
#include "VHTTPServer.h"

#include "HTTPNetworking.h"
#include "HTTPServerCache.h"
#include "VVirtualHost.h"
#include "VHTTPConnectionListener.h"
#include "VHTTPConnectionHandler.h"
#include "HTTPServerSettings.h"
#include "VStaticPagesHTTPRequestHandler.h"
#include "VHTTPServerProject.h"
#include "VHTTPWebsocketHandler.h"


USING_TOOLBOX_NAMESPACE


VHTTPServerComponentLibrary *gHTTPServerComponentLibrary = NULL;


void XBOX::xDllMain (void)
{
	::DebugMsg ("\n***Hello from HTTP Server!***\n");

	gHTTPServerComponentLibrary = new VHTTPServerComponentLibrary (kCOMPONENT_TYPE_LIST, kCOMPONENT_TYPE_COUNT);
}


//--------------------------------------------------------------------------------------------------


CZipComponent *						VHTTPServer::fZipComponent = NULL;
bool								VHTTPServer::fZipComponentInited = false;
XBOX::VRefPtr<VLocalizationManager>	VHTTPServer::fLocalizationManager;

XBOX::VString						VHTTPServer::fServerName = CVSTR ("Wakanda");
XBOX::VString						VHTTPServer::fServerVersion = CVSTR ("1.0");
bool								VHTTPServer::fShowExtendedInfos = true;


namespace NSHTTPServer
{
	CREATE_BAGKEY (refCount);
	CREATE_BAGKEY (zipComponentAvailable);
	CREATE_BAGKEY (requestLoggerAvailable);
	CREATE_BAGKEY (localizationManagerAvailable);
	CREATE_BAGKEY (virtualHostManager);
	CREATE_BAGKEY (httpServerProjects);
}


//--------------------------------------------------------------------------------------------------


struct ConnectionListenerFunctor
{
	ConnectionListenerFunctor (const VHTTPServerProjectSettings *inSettings)
	: fSettings (inSettings)
	{
	}

	bool operator() (const VHTTPConnectionListener *inConnectionListener) const
	{
		assert (NULL != inConnectionListener);

		VHTTPConnectionListener *listener = const_cast<VHTTPConnectionListener *>(inConnectionListener);

		if (listener->GetListeningIP() == fSettings->GetListeningAddress() &&
			listener->GetPort() == fSettings->GetListeningPort() &&
			listener->IsSSLEnabled() == fSettings->GetAllowSSL() &&
			listener->GetSSLPort() == fSettings->GetListeningSSLPort())
			return true;
		else
			return false;
	}

private:
	const VHTTPServerProjectSettings *	fSettings;
};


struct HTTPServerProjectFunctor
{
	HTTPServerProjectFunctor (const IHTTPServerProjectSettings *inProjectSettings)
		: fSettings (inProjectSettings)
	{
	}

	bool operator() (const IHTTPServerProject *inProject) const
	{
		assert (NULL != inProject);

		const IHTTPServerProjectSettings *settings = inProject->GetSettings();

		/*
			Project's are considered as identical when they use the same Port (or SSL port) and use the same hostname...
		*/
		if ((settings->GetListeningPort() == fSettings->GetListeningPort() || (settings->GetAllowSSL() && fSettings->GetAllowSSL() && settings->GetListeningSSLPort() == fSettings->GetListeningSSLPort())) &&
			HTTPServerTools::EqualASCIIVString (settings->GetHostName(), fSettings->GetHostName()))
			return true;
		else
			return false;
	}

private:
	const IHTTPServerProjectSettings *	fSettings;
};


//--------------------------------------------------------------------------------------------------


void VHTTPServerSleepHandler::OnSleepNotification()
{
	XBOX::VInterlocked::Exchange (&fSleepNotifierState, SNS_ASLEEP);
}


void VHTTPServerSleepHandler::OnWakeUp()
{
	XBOX::VInterlocked::Exchange (&fSleepNotifierState, SNS_AWAKE);
}


const XBOX::VString VHTTPServerSleepHandler::GetInfoTag()
{
	return CVSTR ("VHTTPServerSleepHandler");
}


bool VHTTPServerSleepHandler::IsSleeping()
{
	return (XBOX::VInterlocked::AtomicGet (&fSleepNotifierState) == SNS_ASLEEP);
}


//--------------------------------------------------------------------------------------------------


VHTTPServer::VHTTPServer ()
: fServerNet (NULL)
, fRequestLogger (NULL)
, fConnectionListeners()
, fVirtualHostManager (NULL)
, fHTTPServerProjects()
, fConnectionListenersLock()
, fHTTPServerProjectsLock()
, fCacheManager (NULL)
, fSleepHandler (NULL)
{
	fVirtualHostManager = new VVirtualHostManager();

	fServerNet = new VTCPServer();
	fCacheManager = new VCacheManager();

	fSleepHandler = new VHTTPServerSleepHandler();
	XBOX::VSleepNotifier::Instance()->RetainSleepHandler (fSleepHandler);
}


VHTTPServer::~VHTTPServer()
{
	fHTTPServerProjects.clear();

	fServerNet->Stop();
	XBOX::ReleaseRefCountable (&fServerNet);
	XBOX::ReleaseRefCountable (&fVirtualHostManager);
	XBOX::ReleaseRefCountable (&fCacheManager);

	fRequestLogger = NULL;

	VectorOfHTTPConnectionListener::iterator iter = fConnectionListeners.begin();
	while (iter != fConnectionListeners.end())
	{
		(*iter)->Release();
		++iter;
	}

	fConnectionListeners.clear();

	XBOX::ReleaseRefCountable (&fSleepHandler);
}


/* static */
void VHTTPServer::Init()
{
	if (testAssert (gHTTPServerComponentLibrary))
	{
		VFolder *resources = gHTTPServerComponentLibrary->GetLibrary()->RetainFolder (kBF_RESOURCES_FOLDER);
		if (NULL != resources)
		{
			XBOX::VFilePath filePath = resources->GetPath();
			filePath.ToSubFile ("MimeTypes.xml");
			VMimeTypeManager::Init (filePath);
			XBOX::ReleaseRefCountable (&resources);
		}
	}
}


/* static */
void VHTTPServer::InitLate()
{
	/* YT 15-Sep-2010 - Load ZLib here, not in VHTTPServer::Init(), because ZLib component is not initialized yet. */
	if (!fZipComponentInited)
	{
		if (XBOX::VComponentManager::IsComponentAvailable ((XBOX::CType)CZipComponent::Component_Type))
			fZipComponent = (CZipComponent *)VComponentManager::RetainComponent ((CType)CZipComponent::Component_Type);

		fZipComponentInited = true;
	}

	// Init Localization Manager
	if (fLocalizationManager.IsNull())
	{
		XBOX::VFolder *componentFolder = VHTTPServer::RetainComponentFolder (kBF_BUNDLE_FOLDER);

		if (NULL != componentFolder)
		{
			fLocalizationManager.Adopt (new VLocalizationManager (VComponentManager::GetLocalizationLanguage (componentFolder, true)));

			if (fLocalizationManager->LoadDefaultLocalizationFoldersForComponentOrPlugin (componentFolder))
				XBOX::VErrorBase::RegisterLocalizer (CHTTPServer::Component_Type, fLocalizationManager.Get());

			XBOX::QuickReleaseRefCountable (componentFolder);
		}
	}
}


/* static */
void VHTTPServer::Deinit()
{
	VMimeTypeManager::Deinit();

	XBOX::ReleaseRefCountable (&fZipComponent);
}


IHTTPServerProject *VHTTPServer::NewHTTPServerProject (const XBOX::VFilePath& inProjectFolderPath, bool inAppend)
{
	XBOX::VFileSystemNamespace *fileSystemNameSpace = new XBOX::VFileSystemNamespace (XBOX::VProcess::Get()->GetFileSystemNamespace());

	if (fileSystemNameSpace != NULL)
		fileSystemNameSpace->RegisterFileSystem (CVSTR ("PROJECT"), inProjectFolderPath, XBOX::eFSO_Default);

	IHTTPServerProject *result = NewHTTPServerProject (fileSystemNameSpace, inAppend);

	XBOX::ReleaseRefCountable (&fileSystemNameSpace);

	return result;
}


IHTTPServerProject *VHTTPServer::NewHTTPServerProject (XBOX::VFileSystemNamespace *inFileSystemNameSpace, bool inAppend)
{
	InitLate();

	VHTTPServerProject *result = new VHTTPServerProject (this, inFileSystemNameSpace);

	if (NULL != result)
	{
		VAuthenticationManager *authenticationManager = new VAuthenticationManager();
		if (NULL != authenticationManager)
		{
			result->SetAuthenticationManager (authenticationManager);
			XBOX::QuickReleaseRefCountable (authenticationManager);
		}

		if (inAppend)
			AppendHTTPServerProject (result);
	}

	return result;
}


IHTTPWebsocketServer*			VHTTPServer::NewHTTPWebsocketServerHandler()
{

	VHTTPWebsocketServerHandler *result = new VHTTPWebsocketServerHandler();

	return result;
}

IHTTPWebsocketClient*			VHTTPServer::NewHTTPWebsocketClientHandler()
{

	VHTTPWebsocketClientHandler *result = new VHTTPWebsocketClientHandler();

	return result;
}

XBOX::VError VHTTPServer::RemoveHTTPServerProject (IHTTPServerProject *inHTTPServerProject)
{
	if (NULL == dynamic_cast<IHTTPServerProject *>(inHTTPServerProject))
		return XBOX::VE_INVALID_PARAMETER;

	XBOX::VTaskLock							locker (&fHTTPServerProjectsLock);
	XBOX::VRefPtr<IHTTPServerProject>		serverProject = inHTTPServerProject;
	VectorOfHTTPServerProjects::iterator	foundProject = std::find (fHTTPServerProjects.begin(), fHTTPServerProjects.end(), serverProject);

	if (foundProject != fHTTPServerProjects.end())
	{
		fHTTPServerProjects.erase (foundProject);
	}
	else
	{
		return VE_HTTP_SERVER_PROJECT_DOES_NOT_EXIST;
	}

	return XBOX::VE_OK;
}


XBOX::VError VHTTPServer::AppendHTTPServerProject (IHTTPServerProject *inHTTPServerProject)
{
	if (NULL == dynamic_cast<IHTTPServerProject *>(inHTTPServerProject))
		return XBOX::VE_INVALID_PARAMETER;

	XBOX::VTaskLock							locker (&fHTTPServerProjectsLock);
	XBOX::VRefPtr<IHTTPServerProject>		serverProject = inHTTPServerProject;
	VectorOfHTTPServerProjects::iterator	foundProject = std::find_if (fHTTPServerProjects.begin(), fHTTPServerProjects.end(), HTTPServerProjectFunctor (inHTTPServerProject->GetSettings()));

	if (foundProject == fHTTPServerProjects.end())
	{
		fHTTPServerProjects.push_back (serverProject);
	}
	else
	{
		// sc 13/01/2014, append in the error string the ports numbers and hostname which cause the conflict
		VString strPort, strSLLPort;
		strPort.FromLong( inHTTPServerProject->GetSettings()->GetListeningPort());
		strSLLPort.FromLong( inHTTPServerProject->GetSettings()->GetListeningSSLPort());

		return vThrowError ( VE_HTTP_SERVER_PROJECT_ALREADY_EXIST, strPort, strSLLPort, inHTTPServerProject->GetSettings()->GetHostName() );
	}

	return XBOX::VE_OK;
}


XBOX::VFolder *VHTTPServer::_RetainExecutableFolder() const
{
	return XBOX::VProcess::Get()->RetainFolder( XBOX::VProcess::eFS_Executable);
}


XBOX::VFolder *VHTTPServer::_RetainApplicationPackageFolder() const
{
	return XBOX::VProcess::Get()->RetainFolder( XBOX::VProcess::eFS_Bundle);
}


XBOX::VFolder *VHTTPServer::_RetainApplicationPackageContentFolder() const
{
	VFolder *folder = NULL;

#if VERSIONMAC
	XBOX::VFilePath path(VProcess::Get()->GetExecutableFolderPath());
	folder = new VFolder (path.ToParent());
#else
	folder = _RetainExecutableFolder();
#endif

	return folder;
}


IConnectionListener *VHTTPServer::FindConnectionListener (const VHTTPServerProjectSettings *inSettings)
{
	VHTTPConnectionListener *connectionListener = NULL;

	/* Find an already existing connection listener using the same parameters */
	XBOX::VTaskLock lock (&fConnectionListenersLock);
	VectorOfHTTPConnectionListener::const_iterator it = std::find_if (fConnectionListeners.begin(), fConnectionListeners.end(), ConnectionListenerFunctor (inSettings));
	if (it != fConnectionListeners.end())
	{
		connectionListener = dynamic_cast<VHTTPConnectionListener *>(*it);
		connectionListener->IncrementUsageCounter();
	}

	return connectionListener;
}


IConnectionListener *VHTTPServer::CreateConnectionListener (const VHTTPServerProjectSettings *inSettings)
{
	XBOX::VError				error = XBOX::VE_MEMORY_FULL;
	VHTTPConnectionListener *	connectionListener = dynamic_cast<VHTTPConnectionListener *>(FindConnectionListener (inSettings));

	if (testAssert (NULL == connectionListener))
	{
		connectionListener = new VHTTPConnectionListener (this, fRequestLogger);
		if (NULL != connectionListener)
		{
			connectionListener->SetListeningIP (inSettings->GetListeningAddress());
			connectionListener->SetPort (inSettings->GetListeningPort());
			connectionListener->SetSSLPort (inSettings->GetListeningSSLPort());
			connectionListener->SetSSLEnabled (inSettings->GetAllowSSL());
			connectionListener->SetSSLMandatory (inSettings->GetSSLMandatory());
			connectionListener->SetAcceptHTTPOnLocal (inSettings->GetAllowHTTPOnLocal());

			error = XBOX::VE_OK;
		}
	}

	if (connectionListener && !connectionListener->IsListening())
	{
		if (XBOX::VE_OK == error)
		{
			if (inSettings->GetAllowSSL() || inSettings->GetSSLMandatory())
			{
				if (inSettings->GetSSLCertificatesFolderPath().IsValid())
					connectionListener->SetSSLCertificatesFolderPath (inSettings->GetSSLCertificatesFolderPath());
			}

			error = fServerNet->AddConnectionListener (connectionListener);
			if (XBOX::VE_OK == error)
			{
				XBOX::VTaskLock lock (&fConnectionListenersLock);
				fConnectionListeners.push_back (connectionListener);
			}
		}
	}

	return connectionListener;
}


XBOX::VError VHTTPServer::RemoveConnectionListener (IConnectionListener *inConnectionListener)
{
	XBOX::VTaskLock lock (&fConnectionListenersLock);
	VectorOfHTTPConnectionListener::iterator it = std::find (fConnectionListeners.begin(), fConnectionListeners.end(), inConnectionListener);
	if (it != fConnectionListeners.end())
	{
		(*it)->DecrementUsageCounter();
		if (0 == (*it)->GetUsageCounter())
			fConnectionListeners.erase (it);
	}

	return dynamic_cast<VTCPServer *>(fServerNet)->RemoveConnectionListener (inConnectionListener);
}


XBOX::VError VHTTPServer::AddVirtualHost (const VVirtualHost *inVirtualHost)
{
	if (testAssert (inVirtualHost))
	{
		return fVirtualHostManager->AddVirtualHost (inVirtualHost);
	}
	
	return XBOX::VE_INVALID_PARAMETER;
}


VVirtualFolder * VHTTPServer::RetainVirtualFolder (const XBOX::VString& inLocationPath, const XBOX::VString& inIndexPage, const XBOX::VString& inKeyword, XBOX::VFileSystemNamespace *inFileSystemNameSpace)
{
	return fVirtualHostManager->RetainVirtualFolder (inLocationPath, inIndexPage, inKeyword, inFileSystemNameSpace);
}


XBOX::VError VHTTPServer::StopConnectionHandler (XBOX::VTaskID nTaskID)
{
	XBOX::VError error = XBOX::VE_OK;

	for (VectorOfHTTPConnectionListener::iterator it = fConnectionListeners.begin(); it != fConnectionListeners.end(); ++it)
	{
		(*it)->GetWorkerPool()->StopConnectionHandlers (VHTTPConnectionHandler::Handler_Type, nTaskID);
	}

	return error;
}


/* static */
XBOX::VFolder *VHTTPServer::RetainComponentFolder (BundleFolderKind inFolderKind)
{
	return gHTTPServerComponentLibrary->GetLibrary()->RetainFolder (inFolderKind);
}


/* static */
void VHTTPServer::GetErrorTemplateString (XBOX::VString& outErrorTemplateString)
{
	static bool				sInited = false;
	static XBOX::VString	sErrorTemplateString;

	if (!sInited)
	{
		XBOX::VFolder *	componentFolder = VHTTPServer::RetainComponentFolder (kBF_RESOURCES_FOLDER);

		sInited = true;

		if (NULL != componentFolder)
		{
			XBOX::VFilePath		defaultIndexPath = componentFolder->GetPath();

			defaultIndexPath.ToSubFile (CVSTR ("Error_Template.html"));

			if (defaultIndexPath.IsFile())
			{
				XBOX::VFile templateFile (defaultIndexPath);

				if (templateFile.Exists())
				{
					XBOX::VFileStream templateStream (&templateFile);
					if (XBOX::VE_OK == templateStream.OpenReading())
					{
						templateStream.GuessCharSetFromLeadingBytes (XBOX::VTC_UTF_8);
						templateStream.GetText (sErrorTemplateString);
						templateStream.CloseReading();
					}
				}
			}

			XBOX::QuickReleaseRefCountable (componentFolder);
		}
	}

	outErrorTemplateString.FromString (sErrorTemplateString);
}


void VHTTPServer::SaveToBag (XBOX::VValueBag &outBag)
{
	outBag.SetLong (NSHTTPServer::refCount, GetRefCount());
	outBag.SetBool (NSHTTPServer::zipComponentAvailable, VHTTPServer::GetZipComponentAvailable());
	outBag.SetBool (NSHTTPServer::requestLoggerAvailable, (NULL != fRequestLogger));
	outBag.SetBool (NSHTTPServer::localizationManagerAvailable, (!fLocalizationManager.IsNull()));

	XBOX::VValueBag *virtualHostManagerBag = new XBOX::VValueBag();
	if (NULL != virtualHostManagerBag)
	{
		fVirtualHostManager->SaveToBag (*virtualHostManagerBag);
		outBag.AddElement (NSHTTPServer::virtualHostManager, virtualHostManagerBag);
		XBOX::QuickReleaseRefCountable (virtualHostManagerBag);
	}

	XBOX::VBagArray *bagArray = new XBOX::VBagArray();
	if (NULL != bagArray)
	{
		XBOX::VTaskLock locker (&fHTTPServerProjectsLock);

		for (VectorOfHTTPServerProjects::iterator it = fHTTPServerProjects.begin(); it != fHTTPServerProjects.end(); ++it)
		{
			XBOX::VValueBag *projectBag = new XBOX::VValueBag();
			if (NULL != projectBag)
			{
				IHTTPServerProject *serverProject = (*it); 
				dynamic_cast<VHTTPServerProject *>(serverProject)->SaveToBag (*projectBag);
				bagArray->AddTail (projectBag);
				XBOX::QuickReleaseRefCountable (projectBag);
			}
		}

		outBag.SetElements (NSHTTPServer::httpServerProjects, bagArray);
		XBOX::QuickReleaseRefCountable (bagArray);
	}
}


/* static */
XBOX::VError VHTTPServer::ThrowError (const XBOX::VError inErrorCode, const XBOX::VString& inParamString)
{
	if (XBOX::VE_OK != inErrorCode)
	{
		VHTTPError *error = new VHTTPError (inErrorCode);

		if (!inParamString.IsEmpty())
			error->GetBag()->SetString (STRING_ERROR_PARAMETER_NAME, inParamString);

		VTask::GetCurrent()->PushRetainedError (error);
	}

	return inErrorCode;
}


/* static */
void VHTTPServer::LocalizeString (const XBOX::VString& inString, XBOX::VString& outLocalizedString)
{
	if (!fLocalizationManager.IsNull())
		fLocalizationManager->LocalizeStringWithKey (inString, outLocalizedString);

	if (outLocalizedString.IsEmpty())
		outLocalizedString.FromString (inString);
}


/* static */
void VHTTPServer::LocalizeErrorMessage (const XBOX::VError inErrorCode, XBOX::VString& outLocalizedString)
{
	if (!fLocalizationManager.IsNull())
	{
		fLocalizationManager->LocalizeErrorMessage (inErrorCode, outLocalizedString);
	}
	else
	{
		outLocalizedString.Printf ("HTTP Server returned an error: %d\n", ERRCODE_FROM_VERROR (inErrorCode));
	}
}


void VHTTPServer::_BuildCertificatesFilePathes (const XBOX::VFilePath& inPath, XBOX::VFilePath& outCertFilePath, XBOX::VFilePath& outKeyFilePath)
{
	XBOX::VFilePath path (inPath);

	path.SetFileName (STRING_CERT_FILE_NAME);
	outCertFilePath=path;

	path.SetFileName (STRING_KEY_FILE_NAME);
	outKeyFilePath=path;
}


bool VHTTPServer::EqualASCIIVString (const XBOX::VString& inString1, const XBOX::VString& inString2, bool isCaseSensitive)
{
	return HTTPServerTools::EqualASCIIVString (inString1, inString2, isCaseSensitive);
}


sLONG VHTTPServer::FindASCIIVString (const XBOX::VString& inText, const XBOX::VString& inPattern, bool isCaseSensitive)
{
	return HTTPServerTools::FindASCIIVString (inText, inPattern, isCaseSensitive);
}


void VHTTPServer::MakeRFC822GMTDateString (const sLONG inMode, XBOX::VString& outDateString, bool withHeaderName, sLONG inTimeout)
{
	HTTPServerTools::MakeRFC822GMTDateString (inMode, outDateString, withHeaderName, inTimeout);
}


void VHTTPServer::MakeIPv4String (const uLONG inIPv4, XBOX::VString& outString)
{
	HTTPServerTools::MakeIPv4String (inIPv4, outString);
}


void VHTTPServer::MakeServerString (XBOX::VString& outServerString, bool inSecureMode, bool withHeaderName)
{
	HTTPProtocol::MakeServerString (outServerString, inSecureMode, withHeaderName);
}


bool VHTTPServer::MakeHTTPAuthenticateHeaderValue (const HTTPAuthenticationMethod inMethod, const XBOX::VString& inRealm, const XBOX::VString& inDomain, XBOX::VString& outAuthString)
{
	return HTTPProtocol::MakeHTTPAuthenticateHeaderValue (inMethod, inRealm, inDomain, outAuthString);
}


void VHTTPServer::SetServerInformations (const XBOX::VString& inServerName, const XBOX::VString& inServerVersion, bool withExtendedInformations)
{
	fServerName.FromString (inServerName);
	fServerVersion.FromString (inServerVersion);
	fShowExtendedInfos = withExtendedInformations;
}


void VHTTPServer::AppendMimeType (const XBOX::VString& inContentType, const XBOX::VString& inExtensions, bool isCompressible, bool isParsable)
{
	VMimeTypeManager::AppendMimeType (inContentType, inExtensions, isCompressible, isParsable);
}


void VHTTPServer::FindContentType (const XBOX::VString& inExtension, XBOX::VString& outContentType, bool *outIsCompressible, bool *outIsParsable)
{
	VMimeTypeManager::FindContentType (inExtension, outContentType, outIsCompressible, outIsParsable);
}


bool VHTTPServer::IsMimeTypeCompressible (const XBOX::VString& inContentType)
{
	return VMimeTypeManager::IsMimeTypeCompressible (inContentType);
}


bool VHTTPServer::IsMimeTypeParsable (const XBOX::VString& inContentType)
{
	return VMimeTypeManager::IsMimeTypeParsable (inContentType);
}


MimeTypeKind VHTTPServer::GetMimeTypeKind (const XBOX::VString& inContentType)
{
	return VMimeTypeManager::GetMimeTypeKind (inContentType);
}


bool VHTTPServer::CheckProjectSanity (const VHTTPServerProjectSettings *inSettings)
{
	XBOX::VTaskLock	locker (&fHTTPServerProjectsLock);

	if (std::count_if (fHTTPServerProjects.begin(), fHTTPServerProjects.end(), HTTPServerProjectFunctor (inSettings)) > 1)
		return false;

	return true;
}


bool VHTTPServer::IsSleeping()
{
	if (NULL != fSleepHandler)
		return fSleepHandler->IsSleeping();

	return false;
}
