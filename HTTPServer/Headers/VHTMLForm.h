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
#ifndef __HTML_FORM_INCLUDED__
#define __HTML_FORM_INCLUDED__


// class IHTTPRequest;
// 
// 
// extern const XBOX::VString CONST_DEFAULT_CONTENT_TYPE;


//--------------------------------------------------------------------------------------------------


// class VMIMEMessagePart : public XBOX::VObject, public IMIMEPart
// {
// public:
// 									VMIMEMessagePart();
// 
// 	virtual const XBOX::VString&	GetName() const { return fName; }
// 	virtual	const XBOX::VString&	GetFileName() const { return fFileName; }
// 	virtual	const XBOX::VString&	GetMediaType() const { return fMediaType; }
// 	virtual const MimeTypeKind		GetMediaTypeKind() const { return fMediaTypeKind; }
// 	virtual const XBOX::CharSet		GetMediaTypeCharSet() const { return fMediaTypeCharSet; }
// 	virtual const XBOX::VSize		GetSize() const { return fStream.GetDataSize(); }
// 	virtual	const XBOX::VPtrStream&	GetData() const { return fStream; }
// 
// 	void							SetName (const XBOX::VString& inName) { fName.FromString (inName); }
// 	void							SetFileName (const XBOX::VString& inFileName) { fFileName.FromString (inFileName); }
// 	void							SetMediaType (const XBOX::VString& inMediaType);
// 	XBOX::VError					SetData (void *inDataPtr, XBOX::VSize inSize);
// 	XBOX::VError					PutData (void *inDataPtr, XBOX::VSize inSize);
// 
// private:
// 	XBOX::VString					fName;
// 	XBOX::VString					fFileName;
// 	XBOX::VString					fMediaType;
// 	MimeTypeKind					fMediaTypeKind;
// 	XBOX::CharSet					fMediaTypeCharSet;
// 	XBOX::VPtrStream				fStream;
// 
// 	virtual							~VMIMEMessagePart();
// };


//--------------------------------------------------------------------------------------------------


// class VMIMEMessage: public XBOX::VObject, public IMIMEMessage
// {
// 	friend class VMIMEWriter;
// 
// public:
// 									VMIMEMessage();
// 	virtual							~VMIMEMessage();
// 
// 	const XBOX::VString&			GetEncoding() const { return fEncoding; }
// 	const XBOX::VString&			GetBoundary() const { return fBoundary; }
// 	void							GetMIMEParts (XBOX::VectorOfMIMEPart& outMIMEPartsList) const;
// 
// 	void							Load (	const HTTPRequestMethod inRequestMethod,
// 											const XBOX::VString& inContentType,
// 											const XBOX::VString& inURLQuery,
// 											const XBOX::VStream& inStream);
// 	void							Clear();
// 
// 	XBOX::VError					ToStream (XBOX::VStream& outStream);
// 
// protected:
// 	void							_ReadUrl (const XBOX::VString& inString);
// 	void							_ReadUrl (const XBOX::VStream& inStream);
// 	void							_ReadMultipart (const XBOX::VStream& inStream);
// 	void							_AddFilePart (const XBOX::VString& inName, const XBOX::VString& inFileName, const XBOX::VString& inContentType, XBOX::VPtrStream& inStream);
// 	void							_AddValuePair (const XBOX::VString& inName, const XBOX::VString& inContentType, void *inData, const XBOX::VSize inDataSize);
// 
// private:
// 	XBOX::VString					fEncoding;
// 	XBOX::VString					fBoundary;
// 	XBOX::VectorOfMIMEPart			fMIMEParts;
// };


// class VMIMEReader : public XBOX::VObject
// {
// public:
// 									VMIMEReader (const XBOX::VString& inBoundary, XBOX::VStream& inStream);
// 	virtual							~VMIMEReader();
// 
// 	void							GetNextPart (VHTTPMessage& ioMessage);
// 	bool							HasNextPart();
// 	XBOX::VStream&					GetStream() const;
// 	const XBOX::VString&			GetBoundary() const;
// 
// protected:
// 	void							_FindFirstBoundary();
// 	void							_GuessBoundary();
// 	void							_ParseMessage (VHTTPMessage& ioMessage);
// 	bool							_ReadLine (XBOX::VString& line, XBOX::VSize inSize);
// 	bool							_LastPart();
// 
// private:
// 	XBOX::VStream&					fStream;
// 	XBOX::VString					fBoundary;
// 	XBOX::VString					fFirstBoundaryDelimiter;
// 	XBOX::VString					fLastBoundaryDelimiter;
// };


// class VMIMEWriter: public XBOX::VObject, public IMIMEWriter
// {
// public:
// 									VMIMEWriter (const XBOX::VString& inBoundary = CVSTR (""));
// 	virtual							~VMIMEWriter();
// 
// 	const XBOX::VString&			GetBoundary() const { return fBoundary; }
// 	const XBOX::VMIMEMessage&		GetMIMEMessage() const { return fMIMEMessage; }
// 
// 	void							AddPart (const XBOX::VString& inName, const XBOX::VString& inFileName, const XBOX::VString& inMIMEType, XBOX::VPtrStream& inStream);
// 
// private:
// 	XBOX::VString					fBoundary;
// 	XBOX::VMIMEMessage				fMIMEMessage;
// };


#endif	// __HTML_FORM_INCLUDED__


