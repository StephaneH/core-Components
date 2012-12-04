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


const XBOX::VString CONST_MIME_PART_NAME					= CVSTR ("name");
const XBOX::VString CONST_MIME_PART_FILENAME				= CVSTR ("filename");
const XBOX::VString CONST_MIME_MESSAGE_ENCODING_URL			= CVSTR ("application/x-www-form-urlencoded");
const XBOX::VString CONST_MIME_MESSAGE_ENCODING_MULTIPART	= CVSTR ("multipart/form-data");
const XBOX::VString CONST_MIME_MESSAGE_BOUNDARY				= CVSTR ("boundary");
const XBOX::VString CONST_MIME_MESSAGE_BOUNDARY_DELIMITER	= CVSTR ("--");
const XBOX::VString CONST_MIME_MESSAGE_DEFAULT_CONTENT_TYPE	= CVSTR ("multipart/mixed");

const XBOX::VString CONST_TEXT_PLAIN						= CVSTR ("text/plain");
const XBOX::VString CONST_TEXT_PLAIN_UTF_8					= CVSTR ("text/plain; charset=utf-8");


//--------------------------------------------------------------------------------------------------

#if 0
VMIMEMessagePart::VMIMEMessagePart()
: fName()
, fFileName()
, fMediaType()
, fMediaTypeKind (MIMETYPE_UNDEFINED)
, fMediaTypeCharSet (XBOX::VTC_UNKNOWN)
, fStream()
{
}

VMIMEMessagePart::~VMIMEMessagePart()
{
	fStream.Clear();
}


XBOX::VError VMIMEMessagePart::SetData (void *inDataPtr, XBOX::VSize inSize)
{
	if ((NULL == inDataPtr) || (0 == inSize))
		return XBOX::VE_INVALID_PARAMETER;


	fStream.SetDataPtr (inDataPtr, inSize);

	return XBOX::VE_OK;
}


XBOX::VError VMIMEMessagePart::PutData (void *inDataPtr, XBOX::VSize inSize)
{
	if ((NULL == inDataPtr) || (0 == inSize))
		return XBOX::VE_INVALID_PARAMETER;

	XBOX::VError error = XBOX::VE_OK;

	if (XBOX::VE_OK == (error = fStream.OpenWriting()))
		error = fStream.PutData (inDataPtr, inSize);
	fStream.CloseWriting();

	return error;
}


void VMIMEMessagePart::SetMediaType (const XBOX::VString& inMediaType)
{
	HTTPServerTools::ExtractContentTypeAndCharset (inMediaType, fMediaType, fMediaTypeCharSet);
	fMediaTypeKind = HTTPServerTools::GetMimeTypeKind (fMediaType);
}
#endif

//--------------------------------------------------------------------------------------------------

#if 0
VMIMEMessage::VMIMEMessage()
: fEncoding (CONST_MIME_MESSAGE_ENCODING_URL)
{
}


VMIMEMessage::~VMIMEMessage()
{
	Clear();
}


void VMIMEMessage::Clear()
{
	fMIMEParts.clear();
}


void VMIMEMessage::GetMIMEParts (XBOX::VectorOfMIMEPart& outMIMEPartsList) const
{
	outMIMEPartsList.clear();

	for (XBOX::VectorOfMIMEPart::const_iterator it = fMIMEParts.begin(); it != fMIMEParts.end(); ++it)
		outMIMEPartsList.push_back ((*it));
}


void VMIMEMessage::_AddFilePart (const XBOX::VString& inName, const XBOX::VString& inFileName, const XBOX::VString& inContentType, XBOX::VPtrStream& inStream)
{
	VMIMEMessagePart *partSource = new VMIMEMessagePart();
	if (NULL != partSource)
	{
		partSource->SetName (inName);
		partSource->SetFileName (inFileName);
		partSource->SetMediaType (inContentType);
		if (XBOX::VE_OK == inStream.OpenReading())
			partSource->SetData (inStream.GetDataPtr(), inStream.GetDataSize());
		inStream.CloseReading();

		fMIMEParts.push_back (partSource);
		partSource->Release();
	}
}


void VMIMEMessage::_AddValuePair (const XBOX::VString& inName, const XBOX::VString& inContentType, void *inData, const XBOX::VSize inDataSize)
{
	VMIMEMessagePart *partSource = new VMIMEMessagePart();
	if (NULL != partSource)
	{
		partSource->SetName (inName);
		partSource->SetMediaType (inContentType);
		partSource->PutData (inData, inDataSize);

		fMIMEParts.push_back (partSource);
		partSource->Release();
	}
}


void VMIMEMessage::Load (const HTTPRequestMethod inRequestMethod, const XBOX::VString& inContentType, const XBOX::VString& inURLQuery, const XBOX::VStream& inStream)
{
	Clear();

	if (HTTP_POST == inRequestMethod)
	{
		XBOX::VNameValueCollection	params;

		VHTTPHeader::SplitParameters (inContentType, fEncoding, params);

		if (HTTPServerTools::EqualASCIIVString (fEncoding, CONST_MIME_MESSAGE_ENCODING_MULTIPART))
		{
			fBoundary = params.Get (CONST_MIME_MESSAGE_BOUNDARY);
			_ReadMultipart (inStream);
		}
		else
		{
			_ReadUrl (inStream);
		}
	}
	else
	{
		_ReadUrl (inURLQuery);
	}
}


void VMIMEMessage::_ReadUrl (const XBOX::VStream& inStream)
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


void VMIMEMessage::_ReadUrl (const XBOX::VString& inString)
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


void VMIMEMessage::_ReadMultipart (const XBOX::VStream& inStream)
{
	XBOX::VStream& stream = const_cast<XBOX::VStream&>(inStream);

	VMIMEReader reader (fBoundary, stream);
	while (reader.HasNextPart())
	{
		VHTTPMessage message;
		reader.GetNextPart (message);

		XBOX::VString dispositionValue;
		XBOX::VNameValueCollection params;
		if (message.GetHeaders().IsHeaderSet (STRING_HEADER_CONTENT_DISPOSITION))
		{
			XBOX::VString contentDispositionHeaderValue;
			message.GetHeaders().GetHeaderValue (STRING_HEADER_CONTENT_DISPOSITION, contentDispositionHeaderValue);
			VHTTPHeader::SplitParameters (contentDispositionHeaderValue, dispositionValue, params, true); // YT 25-Jan-2012 - ACI0075142
		}

		if (params.Has (CONST_MIME_PART_FILENAME))
		{
			XBOX::VString fileName;
			XBOX::VString name;
			XBOX::VString contentType;

			message.GetHeaders().GetHeaderValue (STRING_HEADER_CONTENT_TYPE, contentType);
			fileName = params.Get (CONST_MIME_PART_FILENAME);
			name = params.Get (CONST_MIME_PART_NAME);

			_AddFilePart (name, fileName, contentType, message.GetBody());
			message.SetDisposeBody (false); // Data ownership is transfered to VMIMEMessagePart object
		}
		else
		{
			XBOX::VString		nameString = params.Get (CONST_MIME_PART_NAME);
			XBOX::VURL::Decode (nameString);
			_AddValuePair (nameString, CONST_TEXT_PLAIN, message.GetBody().GetDataPtr(), message.GetBody().GetDataSize());
		}
	}
}


XBOX::VError VMIMEMessage::ToStream (XBOX::VStream& outStream)
{
	XBOX::VError error = XBOX::VE_OK;

	if (XBOX::VE_OK == outStream.OpenWriting())
	{
		if (fMIMEParts.size() > 0)
		{
			XBOX::VString	string;
			XBOX::VString	charsetName;
			bool			bEncodeBody = false;

			for (XBOX::VectorOfMIMEPart::const_iterator it = fMIMEParts.begin(); it != fMIMEParts.end(); ++it)
			{
				outStream.PutPrintf ("\r\n--%S\r\n", &fBoundary);

				string.FromCString ("Content-Type: ");
				string.AppendString ((*it)->GetMediaType());

				if (!(*it)->GetFileName().IsEmpty())
				{
					string.AppendCString ("; name=\"");
					if (!(*it)->GetName().IsEmpty())
						string.AppendString ((*it)->GetName());
					else
						string.AppendString ((*it)->GetFileName());
					string.AppendCString ("\"\r\nContent-Disposition: attachment; filename=\"");
					string.AppendString ((*it)->GetFileName());
					string.AppendCString ("\"\r\n");
					string.AppendCString ("Content-Transfer-Encoding: base64\r\n");

					bEncodeBody = true;
				}
				else
				{
					if ((*it)->GetMediaTypeCharSet() != XBOX::VTC_UNKNOWN)
					{
						string.AppendCString ("; charset=\"");
						XBOX::VTextConverters::Get()->GetNameFromCharSet ((*it)->GetMediaTypeCharSet(), charsetName);
						string.AppendString (charsetName);
						string.AppendCString ("\"");
					}
					string.AppendCString ("\r\n");
					bEncodeBody = false;
				}

				string.AppendCString ("\r\n");

				outStream.PutText (string);

				if (bEncodeBody)
				{
					XBOX::VMemoryBuffer<> buffer;
					if (XBOX::Base64Coder::Encode ((*it)->GetData().GetDataPtr(), (*it)->GetData().GetDataSize(), buffer))
					{
						outStream.PutData (buffer.GetDataPtr(), buffer.GetDataSize());
					}
				}
				else
				{
					outStream.PutData ((*it)->GetData().GetDataPtr(), (*it)->GetData().GetDataSize());
				}
			}

			outStream.PutPrintf ("\r\n--%S--\r\n", &fBoundary);
		}

		outStream.CloseWriting();
	}

	return error;
}
#endif

//--------------------------------------------------------------------------------------------------


#if 0
VMIMEReader::VMIMEReader (const XBOX::VString& inBoundary, XBOX::VStream& inStream)
: fStream (inStream)
, fBoundary (inBoundary)
, fFirstBoundaryDelimiter()
, fLastBoundaryDelimiter()
{
	fStream.OpenReading ();
}


VMIMEReader::~VMIMEReader()
{
	fStream.CloseReading ();
}


void VMIMEReader::GetNextPart (VHTTPMessage& ioMessage)
{
	if (fBoundary.IsEmpty())
		_GuessBoundary();
	else
		_FindFirstBoundary();

	_ParseMessage (ioMessage);
}


bool VMIMEReader::HasNextPart()
{
	return (!_LastPart());
}


bool VMIMEReader::_LastPart()
{
	XBOX::VString	lineString;
	XBOX::VError	error = XBOX::VE_OK;

	if (fLastBoundaryDelimiter.IsEmpty())
	{
		fLastBoundaryDelimiter.FromString (CONST_MIME_MESSAGE_BOUNDARY_DELIMITER);
		fLastBoundaryDelimiter.AppendString (fBoundary);
		fLastBoundaryDelimiter.AppendString (CONST_MIME_MESSAGE_BOUNDARY_DELIMITER);
	}

	sLONG8 lastPos = fStream.GetPos();
	error = fStream.GetTextLine (lineString, false, NULL);
	fStream.SetPos (lastPos);
	return ((XBOX::VE_OK != error) || (HTTPServerTools::EqualASCIIVString (lineString, fLastBoundaryDelimiter)));
}


const XBOX::VString& VMIMEReader::GetBoundary() const
{
	return fBoundary;
}


void VMIMEReader::_FindFirstBoundary()
{
	XBOX::VString	lineString;
	XBOX::VError	error = XBOX::VE_OK;

	if (fFirstBoundaryDelimiter.IsEmpty())
	{
		fFirstBoundaryDelimiter.FromString (CONST_MIME_MESSAGE_BOUNDARY_DELIMITER);
		fFirstBoundaryDelimiter.AppendString (fBoundary);
	}

	do
	{
		error = fStream.GetTextLine (lineString, false, NULL);
	}
	while ((XBOX::VE_OK == error) && !HTTPServerTools::EqualASCIIVString (lineString, fFirstBoundaryDelimiter));
}


void VMIMEReader::_GuessBoundary()
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


void VMIMEReader::_ParseMessage (VHTTPMessage& ioMessage)
{
	ioMessage.Clear();
	ioMessage.ReadFromStream (fStream, fBoundary);
}
#endif

//--------------------------------------------------------------------------------------------------


// VMIMEWriter::VMIMEWriter (const XBOX::VString& inBoundary)
// : fBoundary (inBoundary)
// , fMIMEMessage()
// {
// 	if (fBoundary.IsEmpty())
// 	{
// 		XBOX::VUUID		uuid (true);
// 		XBOX::VString	uuidString;
// 		uuid.GetString (uuidString);
// 
// 		fBoundary.Printf ("----=NextPart_%S=----", &uuidString);
// 	}
// }
// 
// 
// VMIMEWriter::~VMIMEWriter()
// {
// }
// 
// 
// void VMIMEWriter::AddPart (const XBOX::VString& inName, const XBOX::VString& inFileName, const XBOX::VString& inMIMEType, XBOX::VPtrStream& inStream)
// {
// 	fMIMEMessage._AddFilePart (inName, inFileName, inMIMEType, inStream);
// }
