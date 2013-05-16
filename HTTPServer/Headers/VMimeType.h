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
#ifndef __MIME_TYPE_INCLUDED__
#define __MIME_TYPE_INCLUDED__



class VMimeType;

typedef XBOX::VRefPtr<VMimeType>					VMimeTypeSP;
typedef std::map<XBOX::VString, VMimeTypeSP>		VMimeTypeMap;


class VMimeTypeManager : public XBOX::VObject
{
public:
	static void						Init (const XBOX::VFilePath& inFilePath);
	static void						Deinit();

	static void						AppendMimeType (const XBOX::VString& inContentType, const XBOX::VString& inExtensions, bool isCompressible = false, bool isParsable = false);
	static bool						IsMimeTypesListEmpty();
	static VMimeTypeSP				FindMimeTypeByExtension (const XBOX::VString& inExtension);
	static VMimeTypeSP				FindMimeTypeByContentType (const XBOX::VString& inContentType);

	static void						FindContentType (const XBOX::VString& inExtension, XBOX::VString& outContentType, bool *outIsCompressible = NULL, bool *outIsParsable = NULL);
	static bool						IsMimeTypeCompressible (const XBOX::VString& inContentType);
	static bool						IsMimeTypeParsable (const XBOX::VString& inContentType);
	static MimeTypeKind				GetMimeTypeKind (const XBOX::VString& inContentType);

protected:
	static void						LoadFromBag (const XBOX::VValueBag& inBag);
	static void						SaveToBag (XBOX::VValueBag& outBag);

private:
	static VMimeTypeMap				fContentTypesMap;
	static VMimeTypeMap				fExtensionsTypeMap;

private:
	static void						_AppendMimeType (VMimeType *inMimeType);
	static void						_UpdateMimeType (VMimeType *inSourceMimeType, VMimeType *ioTargetMimeType, bool inOverwrite = false);
};


class VMimeType : public XBOX::VObject , public XBOX::IRefCountable
{
	friend class VMimeTypeManager;

public:
									VMimeType();
									VMimeType (const XBOX::VString& inContentType, const XBOX::VString& inExtensions, bool isCompressible, bool isParsable, MimeTypeKind inMimeTypeKind = MIMETYPE_BINARY);
	virtual							~VMimeType();

	const XBOX::VString&			GetContentType() const { return fContentType; }
	bool							IsCompressible() const { return fCompressible; }
	bool							IsParsable() const { return fParsable; }
	bool							IsCustomType() const { return fCustomType; }
	const XBOX::VectorOfVString&	GetExtensions() const { return fExtensions; }
	MimeTypeKind					GetKind() const { return fKind; }

	void							LoadFromBag (const XBOX::VValueBag& inBag);
	void							SaveToBag (XBOX::VValueBag& outBag);

private:
	XBOX::VString					fContentType;
	XBOX::VectorOfVString			fExtensions;
	bool							fCompressible;
	bool							fParsable;
	bool							fCustomType;
	MimeTypeKind					fKind;
};


#endif	// __MIME_TYPE_INCLUDED__