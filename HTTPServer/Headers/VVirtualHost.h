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
#ifndef __VIRTUAL_HOST_INCLUDED__
#define __VIRTUAL_HOST_INCLUDED__

class VCacheManager;
class VHTTPServer;
class VVirtualFolder;


typedef std::map<XBOX::VRefPtr<XBOX::VRegexMatcher>, XBOX::VRefPtr<VVirtualFolder> >	VVirtualFolderMap;


#if HTTP_SERVER_USE_SYSTEM_NOTIFICATIONS
class VVirtualHost : public XBOX::VObject, public IVirtualHost, public XBOX::VFileSystemNotifier::IEventHandler
#else
class VVirtualHost : public XBOX::VObject, public IVirtualHost
#endif
{
public:
								VVirtualHost (const VHTTPServerProject *inProject);
	virtual						~VVirtualHost();

	/* Support of external files */
	XBOX::VError				AddVirtualFolder (const VVirtualFolder *inVirtualFolder, bool inOverwrite = false);
	VVirtualFolder *			RetainVirtualFolder (const XBOX::VString& inLocationPath, const XBOX::VString& inIndexPage, const XBOX::VString& inKeyword);


	const XBOX::VString&		GetHostPattern() const { return fHostPattern; }
#if HTTP_SERVER_USE_PROJECT_PATTERNS
	const XBOX::VString&		GetURLPattern() const { return fURLPattern; }
#endif
	VVirtualFolder *			RetainMatchingVirtualFolder (const XBOX::VString& inURL);

	virtual XBOX::VError		GetFilePathFromURL (const XBOX::VString& inURL, XBOX::VString& outLocationPath);
	virtual XBOX::VError		GetFileContentFromURL (const XBOX::VString& inURL, IHTTPResponse *ioResponse, bool inUseFullPath = false);

	void						SaveToBag (XBOX::VValueBag& inValueBag);

#if !HTTP_SERVER_GLOBAL_CACHE
	VCacheManager *				GetCacheManager() const { return fCacheManager; }
#endif
	VHTTPServerLog *			GetServerLog() const { return fServerLog; }
	VHTTPServerProjectSettings *GetSettings() const;
	VHTTPServerProject *		GetProject() const { return fProject; }

	const XBOX::VString&		GetUUIDString() const { return fUUIDString; }

	bool						MatchHostPattern (const XBOX::VString& inHostString) const;
	bool						AcceptConnectionsOnPort (const PortNumber inPort) const;
	bool						AcceptConnectionsOnAddress (const IP4 inIPv4Address) const;

	void						SettingsChanged_Message (VHTTPServerProjectSettings *inSettings);

protected:
	void						_Init();
	void						_Deinit();
#if HTTP_SERVER_USE_SYSTEM_NOTIFICATIONS
	virtual void				FileSystemEventHandler (const std::vector<XBOX::VFilePath> &inFilePaths, XBOX::VFileSystemNotifier::EventKind inKind);
#endif

private:
	XBOX::VString				fHostPattern;
#if HTTP_SERVER_USE_PROJECT_PATTERNS
	XBOX::VString				fProjectPattern;	// ie: http://my_site.com[/projectPattern]/path/index.html
	XBOX::VString				fURLPattern;
#endif

	XBOX::VRegexMatcher *		fHostRegexMatcher;

#if !HTTP_SERVER_GLOBAL_CACHE
	VCacheManager *				fCacheManager;
#endif
	VVirtualFolderMap			fVirtualFolderMap;
	XBOX::VCriticalSection		fVirtualFolderMapLock;
	VVirtualFolder *			fDefaultVirtualFolder;
	VHTTPServerLog *			fServerLog;
	VHTTPServerProject *		fProject;
	XBOX::VString				fUUIDString;
};


#endif // __VIRTUAL_HOST_INCLUDED__
