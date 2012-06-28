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
#include "VHTMLForm.h"
#include <cctype>


//--------------------------------------------------------------------------------------------------


const XBOX::VString CONST_FORM_PART_NAME			= CVSTR ("name");
const XBOX::VString CONST_FORM_PART_FILENAME		= CVSTR ("filename");
const XBOX::VString CONST_FORM_ENCODING_URL			= CVSTR ("application/x-www-form-urlencoded");
const XBOX::VString CONST_FORM_ENCODING_MULTIPART	= CVSTR ("multipart/form-data");
const XBOX::VString CONST_FORM_BOUNDARY				= CVSTR ("boundary");
const XBOX::VString CONST_FORM_BOUNDARY_DELIMITER	= CVSTR ("--");
const XBOX::VString CONST_TEXT_PLAIN				= CVSTR ("text/plain");
const XBOX::VString CONST_TEXT_PLAIN_UTF_8			= CVSTR ("text/plain; charset=utf-8");


//--------------------------------------------------------------------------------------------------


VHTMLFormPart::VHTMLFormPart()
: fName()
, fFileName()
, fMediaType()
, fMediaTypeKind (MIMETYPE_UNDEFINED)
, fMediaTypeCharSet (XBOX::VTC_UNKNOWN)
, fStream()
{
}

VHTMLFormPart::~VHTMLFormPart()
{
	fStream.Clear();
}


XBOX::VError VHTMLFormPart::SetData (void *inDataPtr, XBOX::VSize inSize)
{
	if ((NULL == inDataPtr) || (0 == inSize))
		return XBOX::VE_INVALID_PARAMETER;


	fStream.SetDataPtr (inDataPtr, inSize);

	return XBOX::VE_OK;
}


XBOX::VError VHTMLFormPart::PutData (void *inDataPtr, XBOX::VSize inSize)
{
	if ((NULL == inDataPtr) || (0 == inSize))
		return XBOX::VE_INVALID_PARAMETER;

	XBOX::VError error = XBOX::VE_OK;

	if (XBOX::VE_OK == (error = fStream.OpenWriting()))
		error = fStream.PutData (inDataPtr, inSize);
	fStream.CloseWriting();

	return error;
}


void VHTMLFormPart::SetMediaType (const XBOX::VString& inMediaType)
{
	HTTPServerTools::ExtractContentTypeAndCharset (inMediaType, fMediaType, fMediaTypeCharSet);
	fMediaTypeKind = HTTPServerTools::GetMimeTypeKind (fMediaType);
}


//--------------------------------------------------------------------------------------------------


VHTMLForm::VHTMLForm()
: fEncoding (CONST_FORM_ENCODING_URL)
{
}


VHTMLForm::~VHTMLForm()
{
	Clear();
}


void VHTMLForm::Clear()
{
	fFormPartVector.clear();
}


void VHTMLForm::GetFormPartsList (VHTMLFormPartVector& outFormPartsList) const
{
	outFormPartsList.clear();
//	std::copy (fFormPartVector.begin(), fFormPartVector.end(), outFormPartsList.begin());

	for (VHTMLFormPartVector::const_iterator it = fFormPartVector.begin(); it != fFormPartVector.end(); ++it)
	{
		outFormPartsList.push_back ((*it));
	}
}


void VHTMLForm::_AddFilePart (const XBOX::VString& inName, const XBOX::VString& inFileName, const XBOX::VString& inContentType, XBOX::VPtrStream& inStream)
{
	VHTMLFormPart *partSource = new VHTMLFormPart();
	if (NULL != partSource)
	{
		partSource->SetName (inName);
		partSource->SetFileName (inFileName);
		partSource->SetMediaType (inContentType);
		if (XBOX::VE_OK == inStream.OpenReading())
			partSource->SetData (inStream.GetDataPtr(), inStream.GetDataSize());
		inStream.CloseReading();

		fFormPartVector.push_back (partSource);
		partSource->Release();
	}
}


void VHTMLForm::_AddValuePair (const XBOX::VString& inName, const XBOX::VString& inContentType, void *inData, const XBOX::VSize inDataSize)
{
	VHTMLFormPart *partSource = new VHTMLFormPart();
	if (NULL != partSource)
	{
		partSource->SetName (inName);
		partSource->SetMediaType (inContentType);
		partSource->PutData (inData, inDataSize);

		fFormPartVector.push_back (partSource);
		partSource->Release();
	}
}


void VHTMLForm::Load (const IHTTPRequest& inRequest)
{
	Clear();

	if (inRequest.GetRequestMethod() == HTTP_POST)
	{
		XBOX::VString	mediaType;
		XBOX::VString	contentType;
		NameValueCollection params;

		inRequest.GetHTTPHeaders().GetHeaderValue (STRING_HEADER_CONTENT_TYPE, contentType);

		VHTTPHeader::SplitParameters (contentType, mediaType, params);

		fEncoding = mediaType;
		if (HTTPServerTools::EqualASCIIVString (fEncoding, CONST_FORM_ENCODING_MULTIPART))
		{
			fBoundary = params.get (CONST_FORM_BOUNDARY);
			_ReadMultipart (inRequest.GetRequestBody());
		}
		else
		{
			_ReadUrl (inRequest.GetRequestBody());
		}
	}
	else
	{
		XBOX::VString urlString;
		urlString.FromString (inRequest.GetURLQuery());
		_ReadUrl (urlString);
	}
}


void VHTMLForm::_ReadUrl (const XBOX::VStream& inStream)
{
	XBOX::VStream& stream = const_cast<XBOX::VStream&>(inStream);

	if (XBOX::VE_OK == stream.OpenReading())
	{
		XBOX::VString urlString;
		stream.GetText (urlString);
		_ReadUrl (urlString);
		stream.CloseReading();
	}
}


void VHTMLForm::_ReadUrl (const XBOX::VString& inString)
{
	if (!inString.IsEmpty())
	{
		const UniChar *stringPtr = inString.GetCPointer();
		UniChar ch = *stringPtr;

		while (ch != '\0')
		{
			XBOX::VString	name;
			XBOX::VString	value;

			while (ch != '\0' && ch != CHAR_EQUALS_SIGN && ch != CHAR_AMPERSAND)
			{
				if (ch == CHAR_PLUS_SIGN) ch = CHAR_SPACE;
				name.AppendUniChar (ch);
				ch = *(++stringPtr);
			}

			if (ch == CHAR_EQUALS_SIGN)
			{
				ch = *(++stringPtr);
				while (ch != '\0' && ch != CHAR_AMPERSAND)
				{
					if (ch == CHAR_PLUS_SIGN) ch = CHAR_SPACE;
					value.AppendUniChar (ch);
					ch = *(++stringPtr);
				}
			}

			XBOX::VString decodedName (name);
			XBOX::VString decodedValue (value);
			XBOX::VURL::Decode (decodedName);
			XBOX::VURL::Decode (decodedValue);

			XBOX::StStringConverter<char> buffer (decodedValue, XBOX::VTC_UTF_8);
			_AddValuePair (decodedName, CONST_TEXT_PLAIN_UTF_8, (void *)buffer.GetCPointer(), buffer.GetLength());

			if (ch == CHAR_AMPERSAND) ch = *(++stringPtr);
		}
	}
}


void VHTMLForm::_ReadMultipart (const XBOX::VStream& inStream)
{
	XBOX::VStream& stream = const_cast<XBOX::VStream&>(inStream);

	VMultipartReader reader (stream, fBoundary);
	while (reader.HasNextPart())
	{
		VHTTPMessage message;
		reader.GetNextPart (message);

		XBOX::VString dispositionValue;
		NameValueCollection params;
		if (message.GetHeaders().IsHeaderSet (STRING_HEADER_CONTENT_DISPOSITION))
		{
			XBOX::VString contentDispositionHeaderValue;
			message.GetHeaders().GetHeaderValue (STRING_HEADER_CONTENT_DISPOSITION, contentDispositionHeaderValue);
			VHTTPHeader::SplitParameters (contentDispositionHeaderValue, dispositionValue, params, true); // YT 25-Jan-2012 - ACI0075142
		}

		if (params.has (CONST_FORM_PART_FILENAME))
		{
			XBOX::VString fileName;
			XBOX::VString name;
			XBOX::VString contentType;

			message.GetHeaders().GetHeaderValue (STRING_HEADER_CONTENT_TYPE, contentType);
			fileName = params.get (CONST_FORM_PART_FILENAME);
			name = params.get (CONST_FORM_PART_NAME);

			_AddFilePart (name, fileName, contentType, message.GetBody());
			message.SetDisposeBody (false); // Data ownership is transfered to VHTMLFormPart object
		}
		else
		{
			XBOX::VString		nameString = params.get (CONST_FORM_PART_NAME);
			XBOX::VURL::Decode (nameString);
			_AddValuePair (nameString, CONST_TEXT_PLAIN, message.GetBody().GetDataPtr(), message.GetBody().GetDataSize());
		}
	}
}


//--------------------------------------------------------------------------------------------------


VMultipartReader::VMultipartReader (XBOX::VStream& inStream, const XBOX::VString& inBoundary)
: fStream (inStream)
, fBoundary (inBoundary)
, fFirstBoundaryDelimiter()
, fLastBoundaryDelimiter()
{
	fStream.OpenReading ();
}


VMultipartReader::~VMultipartReader()
{
	fStream.CloseReading ();
}


void VMultipartReader::GetNextPart (VHTTPMessage& ioMessage)
{
	if (fBoundary.IsEmpty())
		_GuessBoundary();
	else
		_FindFirstBoundary();

	_ParseMessage (ioMessage);
}


bool VMultipartReader::HasNextPart()
{
	return (!_LastPart());
}


bool VMultipartReader::_LastPart()
{
	XBOX::VString	lineString;
	XBOX::VError	error = XBOX::VE_OK;

	if (fLastBoundaryDelimiter.IsEmpty())
	{
		fLastBoundaryDelimiter.FromString (CONST_FORM_BOUNDARY_DELIMITER);
		fLastBoundaryDelimiter.AppendString (fBoundary);
		fLastBoundaryDelimiter.AppendString (CONST_FORM_BOUNDARY_DELIMITER);
	}

	sLONG8 lastPos = fStream.GetPos();
	error = fStream.GetTextLine (lineString, false, NULL);
	fStream.SetPos (lastPos);
	return ((XBOX::VE_OK != error) || (HTTPServerTools::EqualASCIIVString (lineString, fLastBoundaryDelimiter)));
}


const XBOX::VString& VMultipartReader::GetBoundary() const
{
	return fBoundary;
}


void VMultipartReader::_FindFirstBoundary()
{
	XBOX::VString	lineString;
	XBOX::VError	error = XBOX::VE_OK;

	if (fFirstBoundaryDelimiter.IsEmpty())
	{
		fFirstBoundaryDelimiter.FromString (CONST_FORM_BOUNDARY_DELIMITER);
		fFirstBoundaryDelimiter.AppendString (fBoundary);
	}

	do
	{
		error = fStream.GetTextLine (lineString, false, NULL);
	}
	while ((XBOX::VE_OK == error) && !HTTPServerTools::EqualASCIIVString (lineString, fFirstBoundaryDelimiter));
}


void VMultipartReader::_GuessBoundary()
{
	XBOX::VString lineString;

	sLONG8 lastPos = fStream.GetPos();
	fStream.GetTextLine (lineString, false, NULL);

	// Does the line start with "--" ?
	if (!lineString.IsEmpty() && (lineString.GetUniChar (1) == CHAR_HYPHEN_MINUS) && (lineString.GetUniChar (2) == CHAR_HYPHEN_MINUS))
	{
		fBoundary.FromString (lineString);
	}
	else
	{
		fStream.SetPos (lastPos);
	}
}


void VMultipartReader::_ParseMessage (VHTTPMessage& ioMessage)
{
	ioMessage.Clear();
	ioMessage.ReadFromStream (fStream, fBoundary);
}


