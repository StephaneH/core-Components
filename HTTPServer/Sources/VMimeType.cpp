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


//--------------------------------------------------------------------------------------------------


namespace NSVMimeTypeList
{
	CREATE_BAGKEY (mimeTypes);
}

namespace NSVMimeType
{
	CREATE_BAGKEY (mimeType);
	CREATE_BAGKEY (contentType);
	CREATE_BAGKEY (extensions);
	CREATE_BAGKEY (compressible);
	CREATE_BAGKEY (parsable);
	CREATE_BAGKEY (kind);
}


//--------------------------------------------------------------------------------------------------


VMimeType::VMimeType()
: fContentType()
, fCompressible (false)
, fParsable (false)
, fCustomType (false)
, fExtensions()
, fKind (MIMETYPE_UNDEFINED)
{
}


VMimeType::VMimeType (const XBOX::VString& inContentType, const XBOX::VString& inExtensions, bool isCompressible, bool isParsable, MimeTypeKind inMimeTypeKind)
: fContentType (inContentType)
, fCompressible (isCompressible)
, fParsable (isParsable)
, fCustomType (true)
, fExtensions()
, fKind (inMimeTypeKind)
{
	if (!inExtensions.IsEmpty())
		inExtensions.GetSubStrings (CHAR_SEMICOLON, fExtensions, true, true);

	fKind = HTTPServerTools::GetMimeTypeKind (fContentType);
}


VMimeType::~VMimeType()
{
	fExtensions.clear();
}


void VMimeType::LoadFromBag (const XBOX::VValueBag& inBag)
{
	XBOX::VString string;

	inBag.GetString (NSVMimeType::contentType, fContentType);
	inBag.GetString (NSVMimeType::extensions, string);
	inBag.GetBool (NSVMimeType::compressible, fCompressible);
	inBag.GetBool (NSVMimeType::parsable, fParsable);

	if (!string.IsEmpty())
		string.GetSubStrings (CHAR_SEMICOLON, fExtensions, true, true);

	fKind = HTTPServerTools::GetMimeTypeKind (fContentType);
}


void VMimeType::SaveToBag (XBOX::VValueBag& outBag)
{
	XBOX::VString string;

	for (VectorOfVString::const_iterator it = fExtensions.begin(); it != fExtensions.end(); ++it)
	{
		string.AppendString (*it);
		if ((it + 1) != fExtensions.end())
			string.AppendUniChar (CHAR_SEMICOLON);
	}

	outBag.SetString (NSVMimeType::contentType, fContentType);
	outBag.SetString (NSVMimeType::extensions, string);
	outBag.SetBool (NSVMimeType::compressible, fCompressible);
	outBag.SetBool (NSVMimeType::parsable, fParsable);
	outBag.SetLong (NSVMimeType::kind, (sLONG)fKind);
}


//--------------------------------------------------------------------------------------------------


VMimeTypeMap VMimeTypeManager::fContentTypesMap;
VMimeTypeMap VMimeTypeManager::fExtensionsTypeMap;


/* static */
void VMimeTypeManager::Init (const XBOX::VFilePath& inFilePath)
{
#if LOG_IN_CONSOLE
	VDebugTimer timer;
#endif

	if (fContentTypesMap.size())
		fContentTypesMap.clear();

	if (fExtensionsTypeMap.size())
		fExtensionsTypeMap.clear();

	if (testAssert (inFilePath.IsValid() && inFilePath.IsFile()))
	{
		XBOX::VFile *file = new XBOX::VFile (inFilePath);

		if (testAssert ((NULL != file) && file->Exists()))
		{
			XBOX::VPreferences *prefs = XBOX::VPreferences::Create (file, NULL);
			if (testAssert (NULL != prefs))
			{
				const XBOX::VValueBag *bag = prefs->RetainBag ("/mimeTypes");

				if (testAssert (NULL != bag))
				{
					VMimeTypeManager::LoadFromBag (*bag);
					bag->Release();

#if 0 && VERSIONDEBUG
					XBOX::VValueBag *testBag = new XBOX::VValueBag();
					VMimeTypeManager::SaveToBag (*testBag);
					XBOX::VString xmlString ("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
					testBag->DumpXML (xmlString, CVSTR ("mimeTypesList"), true);
					XBOX::QuickReleaseRefCountable (testBag);
#endif
				}
				
				XBOX::QuickReleaseRefCountable (prefs);
			}
		}

		XBOX::QuickReleaseRefCountable (file);
	}

#if LOG_IN_CONSOLE
	timer.DebugMsg ("* VMimeTypeManager::Init()");
#endif
}


/* static */
void VMimeTypeManager::AppendMimeType (const XBOX::VString& inContentType, const XBOX::VString& inExtensions, bool isCompressible, bool isParsable)
{
	VMimeType *mimeType = new VMimeType (inContentType, inExtensions, isCompressible, isParsable);

	if (NULL != mimeType)
	{
		_AppendMimeType (mimeType);
		XBOX::QuickReleaseRefCountable (mimeType);
	}
}


/* static */
bool VMimeTypeManager::IsMimeTypesListEmpty()
{
	return fContentTypesMap.empty();
}


/* static */
void VMimeTypeManager::Deinit()
{
	fContentTypesMap.clear();
	fExtensionsTypeMap.clear();
}


/* static */
VMimeTypeSP VMimeTypeManager::FindMimeTypeByExtension (const XBOX::VString& inExtension)
{
#if LOG_IN_CONSOLE
	static XBOX::VString sFindMimeTypeByExtensionString ("***Stats on FindMimeTypeByExtension***");
	static XBOX::VProfilingCounter sFindMimeTypeByExtensionCounter (&sFindMimeTypeByExtensionString, false, true);
#endif

	XBOX::VString string (inExtension);

	if (string.FindUniChar (CHAR_FULL_STOP) == 1)
		string.Remove (1, 1);

#if LOG_IN_CONSOLE
	sFindMimeTypeByExtensionCounter.Start();
#endif

	VMimeTypeMap::const_iterator found = fExtensionsTypeMap.find (string);

#if LOG_IN_CONSOLE
	sFindMimeTypeByExtensionCounter.Stop();
#endif

	if (found != fExtensionsTypeMap.end())
		return (*found).second.Get();

	return NULL;
}


/* static */
void VMimeTypeManager::FindContentType (const XBOX::VString& inExtension, XBOX::VString& outContentType, bool *outIsCompressible, bool *outIsParsable)
{
#if LOG_IN_CONSOLE
	VDebugTimer timer;
#endif

	if (NULL != outIsCompressible)
		*outIsCompressible = false;

	if (NULL != outIsParsable)
		*outIsParsable = false;

	if (!inExtension.IsEmpty())
	{
		VMimeTypeSP result = VMimeTypeManager::FindMimeTypeByExtension (inExtension);

		if (!result.IsNull())
		{
			outContentType.FromString (result->GetContentType());

			if (NULL != outIsCompressible)
				*outIsCompressible = result->IsCompressible();
			
			if (NULL != outIsParsable)
				*outIsParsable = result->IsParsable();
		}
	}

	if (outContentType.IsEmpty())
		outContentType.FromString (STRING_CONTENT_TYPE_BINARY);

#if LOG_IN_CONSOLE
	timer.DebugMsg ("* VMimeTypeManager::FindContentType()");
#endif
}


/* static */
bool VMimeTypeManager::IsMimeTypeCompressible (const XBOX::VString& inContentType)
{
	if (inContentType.IsEmpty())
		return false;

#if LOG_IN_CONSOLE
	VDebugTimer timer;
#endif
	bool result = false;

	VMimeTypeSP found = VMimeTypeManager::FindMimeTypeByContentType (inContentType);
	if (!found.IsNull())
	{
		result = found->IsCompressible();
	}
	else
	{
		result = (MIMETYPE_TEXT == HTTPServerTools::GetMimeTypeKind (inContentType));
	}

#if LOG_IN_CONSOLE
	timer.DebugMsg ("* VMimeTypeManager::IsMimeTypeCompressible()");
#endif

	return result;
}


/* static */
bool VMimeTypeManager::IsMimeTypeParsable (const XBOX::VString& inContentType)
{
	if (inContentType.IsEmpty())
		return false;

#if LOG_IN_CONSOLE
	VDebugTimer timer;
#endif

	bool result = false;

	VMimeTypeSP found = VMimeTypeManager::FindMimeTypeByContentType (inContentType);
	if (!found.IsNull())
	{
		result = found->IsParsable();
	}

#if LOG_IN_CONSOLE
	timer.DebugMsg ("* VMimeTypeManager::IsMimeTypeParsable()");
#endif

	return result;
}


/* static */
MimeTypeKind VMimeTypeManager::GetMimeTypeKind (const XBOX::VString& inContentType)
{
	if (inContentType.IsEmpty())
		return MIMETYPE_UNDEFINED;

#if LOG_IN_CONSOLE
	VDebugTimer timer;
#endif

	XBOX::VString	contentType (inContentType);
	MimeTypeKind	result = MIMETYPE_UNDEFINED;
	XBOX::VIndex	posSemiColon = contentType.FindUniChar (CHAR_SEMICOLON);

	if (posSemiColon > 0)
	{
		contentType.SubString (1, posSemiColon - 1);
		HTTPServerTools::TrimUniChar (contentType, CHAR_SPACE);
	}

	VMimeTypeSP found = VMimeTypeManager::FindMimeTypeByContentType (contentType);
	if (!found.IsNull())
	{
		result = found->GetKind();
	}
	else
	{
		result = HTTPServerTools::GetMimeTypeKind (contentType);
	}

#if LOG_IN_CONSOLE
	timer.DebugMsg ("* VMimeTypeManager::GetMimeTypeKind()");
#endif

	return result;
}


/* static */
VMimeTypeSP VMimeTypeManager::FindMimeTypeByContentType (const XBOX::VString& inContentType)
{
#if LOG_IN_CONSOLE
	static XBOX::VString sFindMimeTypeByContentTypeString ("***Stats on FindMimeTypeByContentType***");
	static XBOX::VProfilingCounter sFindMimeTypeByContentTypeStringCounter (&sFindMimeTypeByContentTypeString, false, true);
	sFindMimeTypeByContentTypeStringCounter.Start();
#endif

	VMimeTypeMap::const_iterator found = fContentTypesMap.find (inContentType);

#if LOG_IN_CONSOLE
	sFindMimeTypeByContentTypeStringCounter.Stop();
#endif
	if (found != fContentTypesMap.end())
		return (*found).second.Get();

	return NULL;
}


/* static */
void VMimeTypeManager::LoadFromBag (const XBOX::VValueBag& inBag)
{
	const XBOX::VValueBag *bag = inBag.GetUniqueElement (NSVMimeTypeList::mimeTypes);
	if (NULL != bag)
	{
		const XBOX::VBagArray *bagArray = bag->GetElements (NSVMimeType::mimeType);
		if (NULL != bagArray)
		{
			XBOX::VIndex count = bagArray->GetCount();
			for (XBOX::VIndex i = 1; i <= count; ++i)
			{
				const XBOX::VValueBag *bag = bagArray->GetNth (i);
				if (NULL != bag)
				{
					VMimeType *mimeType = new VMimeType();
					if (NULL != mimeType)
					{
						mimeType->LoadFromBag (*bag);
						_AppendMimeType (mimeType);
						XBOX::QuickReleaseRefCountable (mimeType);
					}
				}
			}
		}
	}
}


/* static */
void VMimeTypeManager::SaveToBag (XBOX::VValueBag& outBag)
{
	XBOX::VBagArray *bagArray = new XBOX::VBagArray();

	if (NULL != bagArray)
	{
		XBOX::VValueBag *parBag = new XBOX::VValueBag();
		if (NULL != parBag)
		{
			for (VMimeTypeMap::const_iterator it = fContentTypesMap.begin(); it != fContentTypesMap.end(); ++it)
			{
				XBOX::VValueBag *bag = new XBOX::VValueBag(); 
				(*it).second.Get()->SaveToBag (*bag);

				bagArray->AddTail (bag);
				bag->Release();
			}

			parBag->SetCData (CVSTR ("<!-- kind: 0 = UNDEFINED, 1 = BINARY, 2 = TEXT, 3 : IMAGE -->"));
			parBag->SetElements (NSVMimeType::mimeType, bagArray);
			outBag.AddElement (NSVMimeTypeList::mimeTypes, parBag);

			parBag->Release();
		}

		bagArray->Release();
	}
}


/* static */
void VMimeTypeManager::_AppendMimeType (VMimeType *inMimeType)
{
	XBOX::VString contentType (inMimeType->GetContentType());

	if (!contentType.IsEmpty())
	{
		VMimeTypeMap::const_iterator found = fContentTypesMap.find (contentType);
		
		if (fContentTypesMap.end() == found)
		{
			fContentTypesMap[contentType] = inMimeType;

			const XBOX::VectorOfVString extensions = inMimeType->GetExtensions();
			for (XBOX::VectorOfVString::const_iterator it = extensions.begin(); it != extensions.end(); ++it)
			{
				XBOX::VString				 extension (*it);

				if (!extension.IsEmpty())
				{
					VMimeTypeMap::const_iterator found = fExtensionsTypeMap.find (extension);

					if (fExtensionsTypeMap.end() == found)
					{
						fExtensionsTypeMap[extension] = inMimeType;
					}
				}
			}
		}
		else
		{
			_UpdateMimeType (inMimeType, found->second);
		}
	}
}


/* static */
void VMimeTypeManager::_UpdateMimeType (VMimeType *inSourceMimeType, VMimeType *ioTargetMimeType, bool inOverwrite)
{
	if ((inSourceMimeType == ioTargetMimeType) || (NULL == inSourceMimeType) || (NULL == ioTargetMimeType))
		return;

	ioTargetMimeType->fContentType = inSourceMimeType->fContentType;
	ioTargetMimeType->fCompressible = inSourceMimeType->fCompressible;
	ioTargetMimeType->fParsable = inSourceMimeType->fParsable;
	ioTargetMimeType->fCustomType = (inOverwrite) ? true : inSourceMimeType->fCustomType;

	if (inOverwrite)
	{
		ioTargetMimeType->fExtensions.clear();
		std::copy (inSourceMimeType->fExtensions.begin(), inSourceMimeType->fExtensions.end(), ioTargetMimeType->fExtensions.begin());
	}
	else
	{
		XBOX::VectorOfVString vectorCopy;
		vectorCopy.resize (ioTargetMimeType->fExtensions.size());
		std::copy (ioTargetMimeType->fExtensions.begin(), ioTargetMimeType->fExtensions.end(), vectorCopy.begin());
		ioTargetMimeType->fExtensions.resize (inSourceMimeType->fExtensions.size() + vectorCopy.size());
		std::merge (inSourceMimeType->fExtensions.begin(), inSourceMimeType->fExtensions.end(), vectorCopy.begin(), vectorCopy.end(), ioTargetMimeType->fExtensions.begin());
	}

	for (XBOX::VectorOfVString::const_iterator it = ioTargetMimeType->fExtensions.begin(); it != ioTargetMimeType->fExtensions.end(); ++it)
	{
		XBOX::VString	extension (*it);

		if (!extension.IsEmpty())
		{
			VMimeTypeMap::const_iterator found = fExtensionsTypeMap.find (extension);

			if ((fExtensionsTypeMap.end() == found) || inOverwrite)
			{
				fExtensionsTypeMap[extension] = ioTargetMimeType;
			}
		}
	}
}
