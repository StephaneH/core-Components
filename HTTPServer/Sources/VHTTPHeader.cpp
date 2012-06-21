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


VHTTPHeader::VHTTPHeader()
: fHeaderList()
{
}


VHTTPHeader::~VHTTPHeader()
{
	fHeaderList.clear();
}


bool VHTTPHeader::IsHeaderSet (const HTTPCommonHeaderCode inHeaderCode) const
{
	return IsHeaderSet (HTTPServerTools::GetHTTPHeaderName (inHeaderCode));
}


bool VHTTPHeader::IsHeaderSet (const XBOX::VString& inName) const
{
	return fHeaderList.has (inName);
}


bool VHTTPHeader::RemoveHeader (const HTTPCommonHeaderCode inHeaderCode)
{
	return RemoveHeader (HTTPServerTools::GetHTTPHeaderName (inHeaderCode));
}


bool VHTTPHeader::RemoveHeader (const XBOX::VString& inName)
{
	if (fHeaderList.has (inName))
	{
		fHeaderList.erase (inName);
		return true;
	}

	return false;
}


bool VHTTPHeader::GetHeaderValue (const HTTPCommonHeaderCode inHeaderCode, XBOX::VString& outValue) const
{
	return GetHeaderValue (HTTPServerTools::GetHTTPHeaderName (inHeaderCode), outValue);
}


bool VHTTPHeader::GetHeaderValue (const XBOX::VString& inName, XBOX::VString& outValue) const
{
	VHeaderListConstIterator it = fHeaderList.find (inName);
	if (it != fHeaderList.end())
	{
		outValue.FromString ((*it).second);
		return true;
	}
	else
		outValue.Clear();

	return false;
}


bool VHTTPHeader::GetHeaderValue (const XBOX::VString& inName, sLONG& outValue) const
{
	XBOX::VString stringValue;
	if (GetHeaderValue (inName, stringValue))
	{
		outValue = HTTPServerTools::GetLongFromString (stringValue);
		return true;
	}
	
	return false;
}


bool VHTTPHeader::GetHeaderValue (const XBOX::VString& inName, XBOX::VTime& outValue) const
{
	XBOX::VString stringValue;
	if (GetHeaderValue (inName, stringValue))
	{
		outValue.FromRfc822String (stringValue);
		return true;
	}
	
	return false;
}


bool VHTTPHeader::SetHeaderValue (const HTTPCommonHeaderCode inHeaderCode, const XBOX::VString& inValue, bool inOverride)
{
	return SetHeaderValue (HTTPServerTools::GetHTTPHeaderName (inHeaderCode), inValue, inOverride);
}


bool VHTTPHeader::SetHeaderValue (const XBOX::VString& inName, const XBOX::VString& inValue, bool inOverride)
{
	bool bIsCookie = HTTPServerTools::EqualASCIIVString (inName, STRING_HEADER_SET_COOKIE);

	VHeaderListConstIterator it = fHeaderList.find (inName);
	if ((it != fHeaderList.end()) && !bIsCookie)
	{
		if (inOverride)
			fHeaderList.set (inName, inValue);
		else
			fHeaderList.add (inName, inValue);
	}
	else
	{
		fHeaderList.add (inName, inValue);
	}

	return true;
}


bool VHTTPHeader::SetHeaderValue (const XBOX::VString& inName, const sLONG inValue, bool inOverride)
{
	XBOX::VString stringValue;
	stringValue.FromLong (inValue);

	return SetHeaderValue (inName, stringValue, inOverride);
}


bool VHTTPHeader::SetHeaderValue (const XBOX::VString& inName, const XBOX::VTime& inValue, bool inOverride)
{
	XBOX::VString stringValue;
	HTTPProtocol::MakeRFC822GMTDateString (inValue, stringValue);

	return SetHeaderValue (inName, stringValue, inOverride);
}


bool VHTTPHeader::GetContentType (XBOX::VString& outContentType, XBOX::CharSet *outCharSet) const
{
	outContentType.Clear();

	XBOX::VString	headerValue;
	XBOX::CharSet	charSet = XBOX::VTC_UNKNOWN;

	if (GetHeaderValue (STRING_HEADER_CONTENT_TYPE, headerValue))
	{
		HTTPServerTools::ExtractContentTypeAndCharset (headerValue, outContentType, charSet);

		if (NULL != outCharSet)
			*outCharSet = charSet;

		return true;
	}

	return false;
}


bool VHTTPHeader::SetContentType (const XBOX::VString& inContentType, const XBOX::CharSet& inCharSet)
{
	if (inCharSet != VTC_UNKNOWN)
	{
		XBOX::VString charSetName;
		XBOX::VString contentType (inContentType);

		VTextConverters::Get()->GetNameFromCharSet (inCharSet, charSetName);
		if (!charSetName.IsEmpty())
		{
			contentType.AppendCString (HTTP_SEMICOLONSP);
			contentType.AppendCString ("charset=");
			contentType.AppendString (charSetName);
		}

		return SetHeaderValue (STRING_HEADER_CONTENT_TYPE, contentType, true);
	}
	else
		return SetHeaderValue (STRING_HEADER_CONTENT_TYPE, inContentType, true);
}


bool VHTTPHeader::GetHeaderValues (const HTTPCommonHeaderCode inHeaderCode, XBOX::VectorOfVString& outValues) const
{
	if (!fHeaderList.size())
		return false;
	
	const XBOX::VString headerName (HTTPServerTools::GetHTTPHeaderName (inHeaderCode));
	
	outValues.erase (outValues.begin(), outValues.end());
	
	VHeaderListConstIterator first = fHeaderList.begin();
	VHeaderListConstIterator it = fHeaderList.end();

	while ((it = std::find_if (first, fHeaderList.end(), FindFirstVStringFunctor<XBOX::VString> (headerName))) != fHeaderList.end())
	{
		outValues.push_back ((*it).second);
		first = ++it;
	}

	return (outValues.size() > 0);
}


bool VHTTPHeader::IsCookieSet (const XBOX::VString& inCookieName) const
{
	XBOX::VectorOfVString cookieValues;
	
	if (GetHeaderValues (HEADER_COOKIE, cookieValues))
	{
		XBOX::VString cookieName (inCookieName);
		cookieName.AppendUniChar (CHAR_EQUALS_SIGN);
		
		for (XBOX::VectorOfVString::const_iterator it = cookieValues.begin(); it != cookieValues.end(); ++it)
		{
			if (HTTPServerTools::FindASCIIVString ((*it), cookieName) > 0)
				return true;
		}
	}

	return false;
}


bool VHTTPHeader::GetCookie (const XBOX::VString& inName, XBOX::VString& outValue) const
{
	XBOX::VectorOfVString cookieValues;
	
	if (GetHeaderValues (HEADER_COOKIE, cookieValues))
	{
		XBOX::VString cookieName (inName);
		cookieName.AppendUniChar (CHAR_EQUALS_SIGN);
		
		for (XBOX::VectorOfVString::const_iterator it = cookieValues.begin(); it != cookieValues.end(); ++it)
		{
			XBOX::VectorOfVString multipleCookieValues;

			(*it).GetSubStrings (CHAR_SEMICOLON, multipleCookieValues, false, true);

			XBOX::VectorOfVString::const_iterator found = std::find_if (multipleCookieValues.begin(), multipleCookieValues.end(), FindVStringFunctor (cookieName));
			if (found != multipleCookieValues.end())
			{
				VHTTPCookie cookie (*found);
				outValue.FromString (cookie.GetValue());
				return true;
			}
		}
	}
	
	outValue.Clear();
	
	return false;
}


bool VHTTPHeader::SetCookie (const XBOX::VString& inName, const XBOX::VString& inValue)
{
	XBOX::VString cookieString;

	cookieString.FromString (inName);
	cookieString.AppendUniChar (CHAR_EQUALS_SIGN);
	cookieString.AppendString (inValue);

	return SetHeaderValue (STRING_HEADER_SET_COOKIE, cookieString);
}


bool VHTTPHeader::DropCookie (const XBOX::VString& inName)
{
	bool			isOK = false;
	XBOX::VString	cookieName (inName);

	cookieName.AppendUniChar (CHAR_EQUALS_SIGN);

	for (VHeaderListIterator it = fHeaderList.begin(); it != fHeaderList.end(); ++it)
	{
		if (HTTPServerTools::EqualASCIIVString (it->first, STRING_HEADER_SET_COOKIE) &&
			HTTPServerTools::FindASCIIVString (it->second, cookieName) > 0)
		{
			// Set Max-Age to 0 to force the browser to delete the cookie immediately.
			VHTTPCookie cookie (it->second);
			cookie.SetMaxAge (0);
			it->second = cookie.ToString();
			isOK = true;
			break;
		}
	}

/*
	if (!isOK)
	{
		VHTTPCookie cookie;
		cookie.SetName (inName);
		cookie.SetMaxAge (0);
		isOK = 	SetHeaderValue (STRING_HEADER_SET_COOKIE, cookie.ToString());
	}
*/

	return isOK;
}


bool VHTTPHeader::GetKeepAliveInfos (sLONG& outTimeout, sLONG& outMaxConnections) const
{
	XBOX::VString keepAliveValue;

	outTimeout = 0;
	outMaxConnections = 0;

	if (GetHeaderValue (STRING_HEADER_KEEP_ALIVE, keepAliveValue))
	{
		/* Keep-alive header can be:
		 * Keep-Alive: timeout=300, max=100
		 * Keep-Alive: timeout=300
		 * Keep-Alive: 300
		 */
		XBOX::VectorOfVString	nameValueStrings;

		if (keepAliveValue.GetSubStrings (CHAR_COMMA, nameValueStrings, true, true))
		{
			XBOX::VString fieldName;
			XBOX::VString fieldValue;

			for (XBOX::VectorOfVString::const_iterator it = nameValueStrings.begin(); it != nameValueStrings.end(); ++it)
			{
				HTTPServerTools::ExtractFieldNameValue (*it, fieldName, fieldValue);

				HTTPServerTools::TrimUniChar (fieldValue, CHAR_QUOTATION_MARK);
				HTTPServerTools::TrimUniChar (fieldValue, CHAR_SPACE);

				if (HTTPServerTools::EqualASCIICString (fieldName, "timeout"))
				{
					outTimeout = fieldValue.GetLong();
				}
				else if (HTTPServerTools::EqualASCIICString (fieldName, "max"))
				{
					outMaxConnections = fieldValue.GetLong();
				}
			}
		}
		else
		{
			outTimeout = keepAliveValue.GetLong();
		}
	}

	return (outTimeout != 0);
}


void VHTTPHeader::GetHeadersList (std::vector<std::pair<XBOX::VString, XBOX::VString> > &outHeadersList) const
{
	outHeadersList.clear();

	for (VHeaderListConstIterator it = fHeaderList.begin(); it != fHeaderList.end(); ++it)
	{
		outHeadersList.push_back (std::make_pair ((*it).first, (*it).second));
	}
}


bool VHTTPHeader::GetBoundary (XBOX::VString& outBoundary) const
{
	// Get Boundary (ex: Content-Type: multipart/form-data; boundary=---------------------------174211871619718)

	const XBOX::VString	STRING_MULTIPART ("multipart/");
	const XBOX::VString	STRING_BOUNDARY ("boundary=");
	XBOX::VString		contentType;

	outBoundary.Clear();
	if (GetHeaderValue (STRING_HEADER_CONTENT_TYPE, contentType))
	{
		sLONG posMultiPart = HTTPServerTools::FindASCIIVString (contentType, STRING_MULTIPART);
		if (posMultiPart > 0)
		{
			posMultiPart += STRING_MULTIPART.GetLength();
			sLONG posBoundary = HTTPServerTools::FindASCIIVString (contentType.GetCPointer() + posMultiPart, STRING_BOUNDARY);
			if (posBoundary > 0)
			{
				posMultiPart += (posMultiPart + STRING_BOUNDARY.GetLength());
				outBoundary.FromBlock (contentType.GetCPointer() + (posMultiPart - 1), (contentType.GetLength() - (posMultiPart - 1)) * sizeof (UniChar), XBOX::VTC_UTF_16);
				if (!outBoundary.IsEmpty())
				{
					HTTPServerTools::TrimUniChar (outBoundary, CHAR_QUOTATION_MARK);
					HTTPServerTools::TrimUniChar (outBoundary, CHAR_SPACE);
					return true;
				}
			}
		}
	}

	return false;
}


void VHTTPHeader::ToString (XBOX::VString& outString) const
{
	for (VHeaderListConstIterator it = fHeaderList.begin(); it != fHeaderList.end(); ++it)
	{
		outString.AppendString (it->first);
		outString.AppendUniChar (CHAR_COLON);
		outString.AppendUniChar (CHAR_SPACE);
		outString.AppendString (it->second);
		outString.AppendCString (HTTP_CRLF);
	}
}


void VHTTPHeader::FromString (const XBOX::VString& inString)
{
	XBOX::VString	string (inString);

	if (!string.IsEmpty())
	{
		XBOX::VectorOfVString nameValuePairs;
		const XBOX::VString CRLF_STRING (HTTP_CRLF);

		sLONG posCRLF_CRLF = HTTPServerTools::FindASCIICString (string, HTTP_CRLFCRLF);
		if (posCRLF_CRLF > 0)
			string.SubString (1, posCRLF_CRLF);

		string.GetSubStrings (CRLF_STRING, nameValuePairs, false, true);
		if (nameValuePairs.size() > 0)
		{
			XBOX::VString	headerName;
			XBOX::VString	headerValue;
			sLONG			pos = 0;

			for (XBOX::VectorOfVString::const_iterator it = nameValuePairs.begin(); it != nameValuePairs.end(); ++it)
			{
				if ((pos = (*it).FindUniChar (CHAR_COLON)) > 0)
				{
					(*it).GetSubString (1, pos - 1, headerName);
					(*it).GetSubString (pos + 1, (*it).GetLength() - pos, headerValue);
					if (!headerName.IsEmpty() && !headerValue.IsEmpty())
					{
						HTTPServerTools::TrimUniChar (headerValue, CHAR_SPACE);
						SetHeaderValue (headerName, headerValue);
					}
				}
			}
		}
	}
}


/* static */
void VHTTPHeader::SplitElements (const XBOX::VString& inString, XBOX::VectorOfVString& outElements, bool ignoreEmpty)
{
	const UniChar *	it  = inString.GetCPointer();
	const UniChar *	end = it + inString.GetLength();
	XBOX::VString	element;

	outElements.clear();

	while (it != end)
	{
		if (*it == CHAR_QUOTATION_MARK)
		{
			element.AppendUniChar (*it++);
			while ((it != end) && (*it != CHAR_QUOTATION_MARK))
			{
				if (*it == CHAR_REVERSE_SOLIDUS)
				{
					++it;
					if (it != end)
						element.AppendUniChar (*it++);
				}
				else
					element.AppendUniChar (*it++);
			}
			if (it != end)
				element.AppendUniChar (*it++);
		}
		else if (*it == CHAR_REVERSE_SOLIDUS)
		{
			++it;
			if (it != end)
				element.AppendUniChar (*it++);
		}
		else if (*it == CHAR_COMMA)
		{
			element.TrimeSpaces();
			if (!ignoreEmpty || !element.IsEmpty())
				outElements.push_back (element);
			element.Clear();
			++it;
		}
		else
			element.AppendUniChar (*it++);
	}

	if (!element.IsEmpty())
	{
		element.TrimeSpaces();
		if (!ignoreEmpty || !element.IsEmpty())
			outElements.push_back (element);
	}
}


/* static */
void VHTTPHeader::SplitParameters (const XBOX::VString& inString, XBOX::VString& outValue, NameValueCollection& outParameters, bool withReverseSolidus)
{
	const UniChar *	it  = inString.GetCPointer();
	const UniChar *	end = it + inString.GetLength();

	outValue.Clear();
	outParameters.clear();

	while ((it != end) && std::isspace(*it))
		++it;

	while ((it != end) && (*it != CHAR_SEMICOLON))
		outValue.AppendUniChar (*it++);

	outValue.TrimeSpaces();
	if (it != end)
		++it;

	SplitParameters (it, end, outParameters, withReverseSolidus);
}


/* static */
void VHTTPHeader::SplitParameters (const UniChar *begin, const UniChar *end, NameValueCollection& outParameters, bool withReverseSolidus)
{
	XBOX::VString name;
	XBOX::VString value;

	const UniChar *	it = begin;
	while (it != end)
	{
		name.Clear();
		value.Clear();

		while ((it != end) && std::isspace(*it))
			++it;

		while ((it != end) && (*it != CHAR_EQUALS_SIGN) && (*it != CHAR_SEMICOLON))
			name.AppendUniChar (*it++);

		name.TrimeSpaces();
		
		if ((it != end) && (*it != CHAR_SEMICOLON))
			++it;

		while ((it != end) && std::isspace(*it))
			++it;

		while ((it != end) && (*it != CHAR_SEMICOLON))
		{
			if (*it == CHAR_QUOTATION_MARK)
			{
				++it;
				while ((it != end) && (*it != CHAR_QUOTATION_MARK))
				{
					if (!withReverseSolidus && (*it == CHAR_REVERSE_SOLIDUS))
					{
						++it;
						if (it != end)
							value.AppendUniChar (*it++);
					}
					else
						value.AppendUniChar (*it++);
				}

				if (it != end)
					++it;
			}
			else if (!withReverseSolidus && (*it == CHAR_REVERSE_SOLIDUS))
			{
				++it;
				if (it != end)
					value.AppendUniChar (*it++);
			}
			else
				value.AppendUniChar (*it++);
		}

		value.TrimeSpaces();
		if (!name.IsEmpty())
			outParameters.add (name, value);

		if (it != end)
			++it;
	}
}


/* static */
void VHTTPHeader::Quote (const XBOX::VString& inValue, XBOX::VString& outResult, bool allowSpace)
{
	const UniChar *	begin = inValue.GetCPointer();
	const UniChar *	end = begin + inValue.GetLength();
	bool mustQuote = false;

	for (const UniChar *it = begin; !mustQuote && (it != end); ++it)
	{
		if (!std::isalnum (*it) && (*it != CHAR_FULL_STOP) && (*it != CHAR_LOW_LINE) && (*it != CHAR_HYPHEN_MINUS) && !(std::isspace (*it) && allowSpace))
			mustQuote = true;
	}

	if (mustQuote)
		outResult.AppendUniChar (CHAR_QUOTATION_MARK);

	outResult.AppendString (inValue);

	if (mustQuote)
		outResult.AppendUniChar (CHAR_QUOTATION_MARK);
}


