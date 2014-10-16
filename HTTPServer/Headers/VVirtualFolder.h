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
#ifndef __VIRTUAL_FOLDER_INCLUDED__
#define __VIRTUAL_FOLDER_INCLUDED__


class VVirtualFolder : public XBOX::VObject, public XBOX::IRefCountable
{
public:
							VVirtualFolder (const XBOX::VString& inLocationPath,
											const XBOX::VString& inIndexFileName,
											const XBOX::VString& inName,
											const XBOX::VString& inProjectPattern,
											bool createIndexIfNotExist = false,
											XBOX::VFileSystemNamespace *inFileSystemNameSpace = NULL);
	virtual					~VVirtualFolder();

	XBOX::VFolder *			GetFolder() const { return fFolder; }
	const XBOX::VString&	GetLocationPath() const { return fLocationPath; }
	const XBOX::VString&	GetIndexFileName() const { return fIndexFileName; }
	XBOX::VError			GetFilePathFromURL (const XBOX::VString& inURL, XBOX::VString& outLocationPath);
	bool					ResolveURLForAlternatePlatformPage (const XBOX::VString& inURL, const XBOX::VString& inPlatform, XBOX::VString& outResolvedURL);
	const XBOX::VString&	GetName() const { return fName; }
	bool					IsLocalFolder() const { return fLocalFolder; }

private:
	XBOX::VFolder *			fFolder;
	XBOX::VString			fLocationPath;
	XBOX::VString			fIndexFileName;
	XBOX::VString			fName;
	XBOX::VString			fProjectPattern;
	bool					fLocalFolder;	// HTTP Server will send an HTTP Redirect otherelse

	/* private methods */
	void					_NormalizeFolder();
	XBOX::VError			_GetFilePathFromURL (const XBOX::VString& inURL, const XBOX::VString& inDefaultIndexPage, XBOX::VString& outLocationPath, XBOX::VString *outURL = NULL);
};


#endif // __VIRTUAL_FOLDER_INCLUDED__
