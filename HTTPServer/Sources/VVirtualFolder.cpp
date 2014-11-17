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
#include "VVirtualFolder.h"


VVirtualFolder::VVirtualFolder (const XBOX::VString& inLocationPath, const XBOX::VString& inIndexFileName, const XBOX::VString& inName, const XBOX::VString& inProjectPattern, bool createIndexIfNotExist, XBOX::VFileSystemNamespace *inFileSystemNameSpace)
: fIndexFileName (inIndexFileName)
, fFolder (NULL)
, fName (inName)
, fProjectPattern (inProjectPattern)
, fLocationPath (inLocationPath)
, fLocalFolder (true)
{
	if (HTTPServerTools::BeginsWithASCIICString (fLocationPath, "http://") ||
		HTTPServerTools::BeginsWithASCIICString (fLocationPath, "https://"))
	{
		fLocalFolder = false;
	}
	else
	{
		bool isOK = false;

		if (NULL != inFileSystemNameSpace)
		{
			isOK = inFileSystemNameSpace->ParsePathWithFileSystem (fLocationPath, NULL, &fFolder);
			if (!isOK && (NULL != fFolder))
				XBOX::ReleaseRefCountable (&fFolder);
		}

		if (!isOK || (NULL == fFolder))
			fFolder = new XBOX::VFolder (fLocationPath);

		fFolder->GetPath().GetPosixPath (fLocationPath);

		if (!fFolder->Exists())
			fFolder->CreateRecursive();

		if (createIndexIfNotExist)
			_NormalizeFolder();
	}

	if (HTTPServerTools::BeginsWithASCIICString (fName, "/"))
		fName.SubString (2, fName.GetLength() - 1);

	if (HTTPServerTools::EndsWithASCIICString (fName, "/"))
		fName.SubString (1, fName.GetLength() - 1);
}


VVirtualFolder::~VVirtualFolder()
{
	XBOX::ReleaseRefCountable (&fFolder);
}


XBOX::VError VVirtualFolder::_GetFilePathFromURL (const XBOX::VString& inURL, const XBOX::VString& inDefaultIndexPage, XBOX::VString& outLocationPath, XBOX::VString *outURL)
{
	XBOX::VError	error = XBOX::VE_FILE_NOT_FOUND;
	XBOX::VFilePath	path (fFolder->GetPath());
	XBOX::VString	pathString (inURL);
	XBOX::VString	folder;
	XBOX::VString	docName;

	if ((pathString.GetLength() == 1) && (pathString.GetUniChar (1) == CHAR_SOLIDUS))
	{
		docName.FromString (inDefaultIndexPage);
	}
	else
	{
		bool	notDone = true;
		sLONG	folderLen = 0;
		sLONG	pos = 0;
		sLONG	curPos = 0;

		// YT 16-Nov-2011 - ACI0073914
		if (pathString.FindUniChar (CHAR_COLON) > 0) // ':'
			pathString.ExchangeAll (CHAR_COLON, CHAR_SOLIDUS);

		if (pathString.FindUniChar (CHAR_REVERSE_SOLIDUS) > 0) // '\'
			pathString.ExchangeAll (CHAR_REVERSE_SOLIDUS, CHAR_SOLIDUS);

		while (notDone)
		{
			if ((pos = pathString.FindUniChar (CHAR_SOLIDUS, curPos + 1)) > 0)	// '/'
			{
				HTTPServerTools::GetSubString (pathString, curPos, pos - 2, folder);
				folderLen = folder.GetLength();
				if (folderLen > 0)
				{
					/* If URL first folder equals Virtual Folder Name or Project Pattern... Do nothing... */
					if ((curPos == 1) && !fName.IsEmpty() && HTTPServerTools::EqualASCIIVString (fName, folder))
						;
					/* YT 24-Feb-2011 - ACI0069901 - Project Pattern is already removed from URL in VHTTPResponse::_UpdateRequestURL()
					else if ((curPos == 1) && !fProjectPattern.IsEmpty() && HTTPServerTools::EqualASCIIVString (fProjectPattern, folder))
					{
						pathString.SubString (curPos + fProjectPattern.GetLength() + 1, pathString.GetLength() - fProjectPattern.GetLength() + 1); // YT 24-Nov-2010 - ACI0068942 - Remove Project Pattern from URL...
						folderLen = 0;
						curPos = -1;
					}
					*/
					else if ((folderLen == 2) && (folder[0] == CHAR_FULL_STOP) && (folder[1] == CHAR_FULL_STOP)) // ".."
						path = path.ToParent();
					else if  ((folderLen == 1) && (folder[0] == CHAR_FULL_STOP)) // "."
						;	// unchanged
					else
						path = path.ToSubFolder (folder);

					curPos += (folderLen + 1);
				}
				else
					curPos += 1;
			}
			else
				notDone = false;

			if (curPos >= pathString.GetLength())
				break;
		}
		
		if (curPos < pathString.GetLength())
			HTTPServerTools::GetSubString (pathString, curPos, pathString.GetLength() - 1, docName);
	}

	/* if URL does not include a filename, try using the index file name set in prefs */
	if (docName.IsEmpty())
		docName.FromString (inDefaultIndexPage);

	path = path.ToSubFile (docName);

	/*
		at this stage path should contain a full path pointing to the wanted file
		check that this is inside the web folder (if it's a web connection)
	*/
	// SECURITY CHECK - change it with great care
	if (path.GetPath().BeginsWith (fFolder->GetPath().GetPath()))
	{
		outLocationPath.FromString (path.GetPath());

		if (NULL != outURL)
		{
			path.GetRelativePosixPath (fFolder->GetPath(), *outURL);
			if (outURL->GetUniChar (1) != CHAR_SOLIDUS)
				outURL->Insert (CHAR_SOLIDUS, 1);
		}
		error = VE_OK;
	}
	else
	{
		// UNDER ATTACK !!!
		path.Clear();
		error = VE_HTTP_PROTOCOL_FORBIDDEN;
	}

	return error;
}


XBOX::VError VVirtualFolder::GetFilePathFromURL (const XBOX::VString& inURL, XBOX::VString& outLocationPath)
{
	if (!fLocalFolder)
	{
		XBOX::VString URL (inURL);
		sLONG pos = HTTPServerTools::FindASCIIVString (URL, fName);

		if (pos > 0)
			URL.Remove (1, pos + fName.GetLength() - 1);

		if ((URL.GetLength() == 1) && (URL.GetUniChar (1) == CHAR_SOLIDUS) && (!fIndexFileName.IsEmpty()))
			URL.AppendString (fIndexFileName);
		
		outLocationPath.FromString (fLocationPath);
		if (outLocationPath.GetUniChar (outLocationPath.GetLength()) == CHAR_SOLIDUS)
			outLocationPath.Truncate (outLocationPath.GetLength() - 1);
		outLocationPath.AppendString (URL);

		/*
         *  Send a 301 - MOVED PERMANENTLY instead of 302 - Found (to prevent resources from being requested again)
         */
        return VE_HTTP_PROTOCOL_MOVED_PERMANENTLY;
	}
	else
	{
		return _GetFilePathFromURL (inURL, fIndexFileName, outLocationPath);
	}
}


bool VVirtualFolder::ResolveURLForAlternatePlatformPage (const XBOX::VString& inURL, const XBOX::VString& inPlatform, XBOX::VString& outResolvedURL)
{
	outResolvedURL.Clear();

	if (!inPlatform.IsEmpty() && !fIndexFileName.IsEmpty())
	{
		XBOX::VString	URL (inURL);
		XBOX::VString	alternatePageName (fIndexFileName);
		XBOX::VString	alternateLocationPath;
		XBOX::VIndex	pos = alternatePageName.FindUniChar (CHAR_FULL_STOP, alternatePageName.GetLength(), true);

		if (pos > 0)
		{
			XBOX::VString platform (inPlatform);
			platform.Insert (CHAR_HYPHEN_MINUS, 1);
			alternatePageName.Insert (platform, pos);
		}

		if (XBOX::VE_OK == _GetFilePathFromURL (URL, alternatePageName, alternateLocationPath, &outResolvedURL))
		{
			if (HTTPServerTools::EndsWithASCIIVString (outResolvedURL, alternatePageName) ||
				HTTPServerTools::EndsWithASCIIVString (outResolvedURL, fIndexFileName))
			{
				XBOX::VFile file (alternateLocationPath);
				if (!file.Exists())
					outResolvedURL.Clear();
			}
		}
	}

	return !outResolvedURL.IsEmpty();
}


/* private */
void VVirtualFolder::_NormalizeFolder()
{
	assert (NULL != fFolder);

	if (fFolder->Exists())
	{
		XBOX::VFilePath folderPath (fFolder->GetPath());
		XBOX::VFilePath indexPath;

		indexPath = folderPath.ToSubFile (fIndexFileName);

		if (indexPath.IsFile())
		{
			XBOX::VFile indexFile (indexPath);

			if (!indexFile.Exists())
			{
				XBOX::VFolder *componentFolder = VHTTPServer::RetainComponentFolder (kBF_RESOURCES_FOLDER);

				if (NULL != componentFolder)
				{
					XBOX::VFilePath		defaultIndexPath = componentFolder->GetPath();
					XBOX::DialectCode	dialectCode = XBOX::VIntlMgr::GetDefaultMgr()->GetCurrentDialectCode();
					XBOX::VString		languageCode;
					XBOX::VString		fileName;

					XBOX::VIntlMgr::GetDefaultMgr()->GetISO6391LanguageCode (dialectCode, languageCode);

					fileName.AppendCString ("index_");
					fileName.AppendString (languageCode);
					fileName.AppendCString (".html");

					defaultIndexPath.ToSubFolder (CVSTR ("Default Page")).ToSubFile (fileName);

					if (defaultIndexPath.IsFile())
					{
						XBOX::VFile defaultIndexFile (defaultIndexPath);

						if (defaultIndexFile.Exists())
							indexFile.CopyFrom (defaultIndexFile);
					}

					XBOX::QuickReleaseRefCountable (componentFolder);
				}
			}
		}
	}
}

