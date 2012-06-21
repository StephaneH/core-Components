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


#define RFC2616_COMPLIANT 1

/*
	see : http://www.ietf.org/rfc/rfc2616.txt for a complete HTTP/1.1 protocol description
*/


//--------------------------------------------------------------------------------------------------


const XBOX::VString STRING_REQUEST_VALIDATION_PATTERN	= CVSTR ("^([A-Za-z]{1,}) .* HTTP/[0-9]{1,}.[0-9]{1,}$");


typedef std::vector <std::pair<XBOX::VString, Real> > VectorOfHeadersWithQFactors;


static
void _ExtractValuesWithQFactors (const XBOX::VString& inFieldValue, VectorOfHeadersWithQFactors& outValuesWithQFactors)
{
	/* Parse Field values like Accept-Encoding: compress;q=0, deflate;q=0.5, gzip;q=1.0;level=1 */

	outValuesWithQFactors.clear();

	if (inFieldValue.IsEmpty())
		return;

	XBOX::VectorOfVString			values;
	XBOX::VectorOfVString			pairedValues;
	XBOX::VString					value;
	Real							qFactor;
	XBOX::VString					qFactorString;
	XBOX::StStringConverter<char>	converter (XBOX::VTC_US_ASCII);

	inFieldValue.GetSubStrings (CHAR_COMMA, values, false, true);

	for (XBOX::VectorOfVString::const_iterator it = values.begin(); it != values.end(); ++it)
	{
		(*it).GetSubStrings (CHAR_SEMICOLON, pairedValues, false, true);
		if (pairedValues.size() > 0)
		{
			value = pairedValues[0];
			qFactor = 1.0;

			if (pairedValues.size() > 1)
			{
				XBOX::VectorOfVString::const_iterator found = std::find_if (pairedValues.begin(), pairedValues.end(), FindVStringFunctor (CVSTR ("q=")));
				if (found != pairedValues.end())
				{
					qFactorString = (*found);
					if (qFactorString.GetLength() > 2)
					{
						qFactorString.SubString (3, qFactorString.GetLength() - 2);
						qFactor = atof (converter.ConvertString (qFactorString));
					}
				}
			}

			HTTPServerTools::TrimUniChar (value, HTTP_CR);
			HTTPServerTools::TrimUniChar (value, HTTP_LF);
			HTTPServerTools::TrimUniChar (value, HTTP_HT);

			outValuesWithQFactors.push_back (std::make_pair (value, qFactor));
		}
	}
}


inline
bool _QFactorComparator (const std::pair<XBOX::VString, Real>& inValue1, const std::pair<XBOX::VString, Real>& inValue2)
{
	return (inValue1.second > inValue2.second);
}


inline
bool _EqualMediaType (const XBOX::VString& inMediaType1, const XBOX::VString& inMediaType2)
{
	if (inMediaType1.IsEmpty() || inMediaType2.IsEmpty())
		return false;

	XBOX::VectorOfVString typeValues1;
	XBOX::VectorOfVString typeValues2;

	inMediaType1.GetSubStrings (CHAR_SOLIDUS, typeValues1);
	inMediaType2.GetSubStrings (CHAR_SOLIDUS, typeValues2);

	if ((typeValues1.size() > 0) && (typeValues2.size() > 0))
	{
		if ((HTTPServerTools::EqualASCIIVString (typeValues1[0], typeValues2[0]) || HTTPServerTools::EqualASCIIVString (typeValues1[0], STRING_STAR)) &&
			((typeValues1.size() > 1) && (typeValues2.size() > 1) && HTTPServerTools::EqualASCIIVString (typeValues1[1], typeValues2[1]) || HTTPServerTools::EqualASCIIVString ((typeValues1.size() > 1) ? typeValues1[1] : typeValues1[0], STRING_STAR)))
			return true;
	}

	return false;
}


inline
void _MakeServerString (const XBOX::VString& inProductName, const XBOX::VString& inProductVersion, XBOX::VString& outServerString, bool addExtendedInfos = false)
{
	outServerString.Printf ("%S/%S", &inProductName, &inProductVersion);

	if (addExtendedInfos)
	{
		outServerString.AppendCString (" (");
	#if VERSIONMAC
		outServerString.AppendCString ("MacOS");
	#elif VERSIONWIN
		outServerString.AppendCString ("Windows");
	#elif VERSION_LINUX
		outServerString.AppendCString ("Linux");
	#endif

	#if ARCH_64
		outServerString.AppendCString ("-x64");
	#elif ARCH_386
		outServerString.AppendCString ("-i386");
	#endif

		outServerString.AppendCString (")");
	}
}


//--------------------------------------------------------------------------------------------------


XBOX::VRefPtr<XBOX::VRegexMatcher>	HTTPProtocol::fRequestRegexMatcher;
XBOX::VCriticalSection				HTTPProtocol::fRequestRegexMatcherMutex;


/* static */
bool HTTPProtocol::IsValidStatusCode (sLONG inStatusCode)
{
	switch (inStatusCode)
	{
		case HTTP_CONTINUE:
		case HTTP_SWITCHING_PROTOCOLS:
		case HTTP_OK:
		case HTTP_CREATED:
		case HTTP_ACCEPTED:
		case HTTP_NON_AUTHORITATIVE_INFORMATION:
		case HTTP_NO_CONTENT:
		case HTTP_RESET_CONTENT:
		case HTTP_PARTIAL_CONTENT:
		case HTTP_MULTIPLE_CHOICE:
		case HTTP_MOVED_PERMANENTLY:
		case HTTP_FOUND:
		case HTTP_SEE_OTHER:
		case HTTP_NOT_MODIFIED:
		case HTTP_USE_PROXY:
		case HTTP_TEMPORARY_REDIRECT:
		case HTTP_BAD_REQUEST:
		case HTTP_UNAUTHORIZED:
		case HTTP_PAYMENT_REQUIRED:
		case HTTP_FORBIDDEN:
		case HTTP_NOT_FOUND:
		case HTTP_METHOD_NOT_ALLOWED:
		case HTTP_NOT_ACCEPTABLE:
		case HTTP_PROXY_AUTHENTICATION_REQUIRED:
		case HTTP_REQUEST_TIMEOUT:
		case HTTP_CONFLICT:
		case HTTP_GONE:
		case HTTP_LENGTH_REQUIRED:
		case HTTP_PRECONDITION_FAILED:
		case HTTP_REQUEST_ENTITY_TOO_LARGE:
		case HTTP_REQUEST_URI_TOO_LONG:
		case HTTP_UNSUPPORTED_MEDIA_TYPE:
		case HTTP_REQUESTED_RANGE_NOT_SATISFIABLE:
		case HTTP_EXPECTATION_FAILED:
		case HTTP_INTERNAL_SERVER_ERROR:
		case HTTP_NOT_IMPLEMENTED:
		case HTTP_BAD_GATEWAY:
		case HTTP_SERVICE_UNAVAILABLE:
		case HTTP_GATEWAY_TIMEOUT:
		case HTTP_HTTP_VERSION_NOT_SUPPORTED:
			return true;
	}

	return false;
}


/*
	RFC 2616 - 6.1.1 Status Code and Reason Phrase

	   The Status-Code element is a 3-digit integer result code of the
	   attempt to understand and satisfy the request. These codes are fully
	   defined in section 10. The Reason-Phrase is intended to give a short
	   textual description of the Status-Code. The Status-Code is intended
	   for use by automata and the Reason-Phrase is intended for the human
	   user. The client is not required to examine or display the Reason-
	   Phrase.
*/

void HTTPProtocol::GetStatusCodeExplanation (sLONG inStatusCode, XBOX::VString& outExplanation, bool clearStringFirst)
{
	if (clearStringFirst)
		outExplanation.Clear();

	switch (inStatusCode)
	{
		case 100:	outExplanation.AppendCString ("Continue");							break;
		case 101:	outExplanation.AppendCString ("Switching Protocols");				break;
		case 200:	outExplanation.AppendCString ("OK");								break;
		case 201:	outExplanation.AppendCString ("Created");							break;
		case 202:	outExplanation.AppendCString ("Accepted");							break;
		case 203:	outExplanation.AppendCString ("Non-Authoritative Information");		break;
		case 204:	outExplanation.AppendCString ("No Content");						break;
		case 205:	outExplanation.AppendCString ("Reset Content");						break;
		case 206:	outExplanation.AppendCString ("Partial Content");					break;
		case 300:	outExplanation.AppendCString ("Multiple Choices");					break;
		case 301:	outExplanation.AppendCString ("Moved Permanently");					break;
		case 302:	outExplanation.AppendCString ("Found");								break;
		case 303:	outExplanation.AppendCString ("See Other");							break;
		case 304:	outExplanation.AppendCString ("Not Modified");						break;
		case 305:	outExplanation.AppendCString ("Use Proxy");							break;
		case 307:	outExplanation.AppendCString ("Temporary Redirect");				break;
		case 400:	outExplanation.AppendCString ("Bad Request");						break;
		case 401:	outExplanation.AppendCString ("Unauthorized");						break;
		case 402:	outExplanation.AppendCString ("Payment Required");					break;
		case 403:	outExplanation.AppendCString ("Forbidden");							break;
		case 404:	outExplanation.AppendCString ("Not Found");							break;
		case 405:	outExplanation.AppendCString ("Method Not Allowed");				break;
		case 406:	outExplanation.AppendCString ("Not Acceptable");					break;
		case 407:	outExplanation.AppendCString ("Proxy Authentication Required");		break;
		case 408:	outExplanation.AppendCString ("Request Time-out");					break;
		case 409:	outExplanation.AppendCString ("Conflict");							break;
		case 410:	outExplanation.AppendCString ("Gone");								break;
		case 411:	outExplanation.AppendCString ("Length Required");					break;
		case 412:	outExplanation.AppendCString ("Precondition Failed");				break;
		case 413:	outExplanation.AppendCString ("Request Entity Too Large");			break;
		case 414:	outExplanation.AppendCString ("Request-URI Too Large");				break;
		case 415:	outExplanation.AppendCString ("Unsupported Media Type");			break;
		case 416:	outExplanation.AppendCString ("Requested range not satisfiable");	break;
		case 417:	outExplanation.AppendCString ("Expectation Failed");				break;
		case 500:	outExplanation.AppendCString ("Internal Server Error");				break;
		case 501:	outExplanation.AppendCString ("Not Implemented");					break;
		case 502:	outExplanation.AppendCString ("Bad Gateway");						break;
		case 503:	outExplanation.AppendCString ("Service Unavailable");				break;
		case 504:	outExplanation.AppendCString ("Gateway Time-out");					break;
		case 505:	outExplanation.AppendCString ("HTTP Version not supported");		break;
		default:	assert(false);														break;
	}
}


/*
	RFC 2616 - 6.1 Status-Line

	   The first line of a Response message is the Status-Line, consisting
	   of the protocol version followed by a numeric status code and its
	   associated textual phrase, with each element separated by SP
	   characters. No CR or LF is allowed except in the final CRLF sequence.

		   Status-Line = HTTP-Version SP Status-Code SP Reason-Phrase CRLF
*/

void HTTPProtocol::MakeStatusLine (HTTPVersion inVersion, sLONG inStatusCode, XBOX::VString& outStatusLine)
{
	sLONG statusCode = inStatusCode;

	switch (inVersion)
	{
		case VERSION_1_0:
			outStatusLine.FromCString ("HTTP/1.0");
			break;
		case VERSION_1_1:
			outStatusLine.FromCString ("HTTP/1.1");
			break;
		default: // unknown HTTP version
			outStatusLine.FromCString ("HTTP/1.1");
			statusCode = 505;
			break;;
	}
	
	outStatusLine.AppendUniChar (CHAR_SPACE);
	outStatusLine.AppendLong (statusCode);
	outStatusLine.AppendUniChar (CHAR_SPACE);

	GetStatusCodeExplanation (statusCode, outStatusLine, false);

	outStatusLine.AppendCString (HTTP_CRLF);
}


/*
	RFC 2616 - 5.1 Request-Line

	   The Request-Line begins with a method token, followed by the
	   Request-URI and the protocol version, and ending with CRLF. The
	   elements are separated by SP characters. No CR or LF is allowed
	   except in the final CRLF sequence.

			Request-Line   = Method SP Request-URI SP HTTP-Version CRLF
*/

HTTPRequestMethod HTTPProtocol::GetMethodFromRequestLine (const UniChar *inBuffer, sLONG inBufferLen)
{
	if ((NULL == inBuffer) || (inBufferLen < 3))
		return HTTP_UNKNOWN;

	/*
		YT 02-May-2011 - RFC2616 compliance
		Reference: RFC2616 chapter: 5.1.1 Method
		The Method  token indicates the method to be performed on the
		resource identified by the Request-URI. The method is case-sensitive.
	*/
#if RFC2616_COMPLIANT
	if ((inBufferLen >= 3)
		&& (inBuffer[0] == 'G')
		&& (inBuffer[1] == 'E')
		&& (inBuffer[2] == 'T'))
	{
		return HTTP_GET;
	}
	else if ((inBufferLen >= 3)
		&& (inBuffer[0] == 'P')
		&& (inBuffer[1] == 'U')
		&& (inBuffer[2] == 'T'))
	{
		return HTTP_PUT;
	}
	else if ((inBufferLen >= 4)
		&& (inBuffer[0] == 'P')
		&& (inBuffer[1] == 'O')
		&& (inBuffer[2] == 'S')
		&& (inBuffer[3] == 'T'))
	{
		return HTTP_POST;
	}
	else if ((inBufferLen >= 4)
		&& (inBuffer[0] == 'H')
		&& (inBuffer[1] == 'E')
		&& (inBuffer[2] == 'A')
		&& (inBuffer[3] == 'D'))
	{
		return HTTP_HEAD;
	}
	else if ((inBufferLen >= 5)
		&& (inBuffer[0] == 'T')
		&& (inBuffer[1] == 'R')
		&& (inBuffer[2] == 'A')
		&& (inBuffer[3] == 'C')
		&& (inBuffer[4] == 'E'))
	{
		return HTTP_TRACE;
	}
	else if ((inBufferLen >= 6)
		&& (inBuffer[0] == 'D')
		&& (inBuffer[1] == 'E')
		&& (inBuffer[2] == 'L')
		&& (inBuffer[3] == 'E')
		&& (inBuffer[4] == 'T')
		&& (inBuffer[5] == 'E'))
	{
		return HTTP_DELETE;
	}
	else if ((inBufferLen >= 7)
		&& (inBuffer[0] == 'O')
		&& (inBuffer[1] == 'P')
		&& (inBuffer[2] == 'T')
		&& (inBuffer[3] == 'I')
		&& (inBuffer[4] == 'O')
		&& (inBuffer[5] == 'N')
		&& (inBuffer[6] == 'S'))
	{
		return HTTP_OPTIONS;
	}
#else
	if ((inBufferLen >= 3)
		&& ((inBuffer[0] == 'G') || (inBuffer[0] == 'g'))
		&& ((inBuffer[1] == 'E') || (inBuffer[1] == 'e'))
		&& ((inBuffer[2] == 'T') || (inBuffer[2] == 't')))
	{
		return HTTP_GET;
	}
	else if ((inBufferLen >= 3)
		&& ((inBuffer[0] == 'P') || (inBuffer[0] == 'p'))
		&& ((inBuffer[1] == 'U') || (inBuffer[1] == 'u'))
		&& ((inBuffer[2] == 'T') || (inBuffer[2] == 't')))
	{
		return HTTP_PUT;
	}
	else if ((inBufferLen >= 4)
		&& ((inBuffer[0] == 'P') || (inBuffer[0] == 'p'))
		&& ((inBuffer[1] == 'O') || (inBuffer[1] == 'o'))
		&& ((inBuffer[2] == 'S') || (inBuffer[2] == 's'))
		&& ((inBuffer[3] == 'T') || (inBuffer[3] == 't')))
	{
		return HTTP_POST;
	}
	else if ((inBufferLen >= 4)
		&& ((inBuffer[0] == 'H') || (inBuffer[0] == 'h'))
		&& ((inBuffer[1] == 'E') || (inBuffer[1] == 'e'))
		&& ((inBuffer[2] == 'A') || (inBuffer[2] == 'a'))
		&& ((inBuffer[3] == 'D') || (inBuffer[3] == 'd')))
	{
		return HTTP_HEAD;
	}
	else if ((inBufferLen >= 5)
		&& ((inBuffer[0] == 'T') || (inBuffer[0] == 't'))
		&& ((inBuffer[1] == 'R') || (inBuffer[1] == 'r'))
		&& ((inBuffer[2] == 'A') || (inBuffer[2] == 'a'))
		&& ((inBuffer[3] == 'C') || (inBuffer[3] == 'c'))
		&& ((inBuffer[4] == 'E') || (inBuffer[4] == 'e')))
	{
		return HTTP_TRACE;
	}
	else if ((inBufferLen >= 6)
		&& ((inBuffer[0] == 'D') || (inBuffer[0] == 'd'))
		&& ((inBuffer[1] == 'E') || (inBuffer[1] == 'e'))
		&& ((inBuffer[2] == 'L') || (inBuffer[2] == 'l'))
		&& ((inBuffer[3] == 'E') || (inBuffer[3] == 'e'))
		&& ((inBuffer[4] == 'T') || (inBuffer[4] == 't'))
		&& ((inBuffer[5] == 'E') || (inBuffer[5] == 'e')))
	{
		return HTTP_DELETE;
	}
	else if ((inBufferLen >= 7)
		&& ((inBuffer[0] == 'O') || (inBuffer[0] == 'o'))
		&& ((inBuffer[1] == 'P') || (inBuffer[1] == 'p'))
		&& ((inBuffer[2] == 'T') || (inBuffer[2] == 't'))
		&& ((inBuffer[3] == 'I') || (inBuffer[3] == 'i'))
		&& ((inBuffer[4] == 'O') || (inBuffer[4] == 'o'))
		&& ((inBuffer[5] == 'N') || (inBuffer[5] == 'n'))
		&& ((inBuffer[6] == 'S') || (inBuffer[5] == 's')))
	{
		return HTTP_OPTIONS;
	}
#endif

	return HTTP_UNKNOWN;
}


/* static */
HTTPRequestMethod HTTPProtocol::GetMethodFromName (const XBOX::VString& inName)
{
	if (inName.IsEmpty())
		return HTTP_UNKNOWN;

	if (HTTPServerTools::EqualASCIIVString (inName, STRING_HTTP_METHOD_GET))
	{
		return HTTP_GET;
	}
	else if (HTTPServerTools::EqualASCIIVString (inName, STRING_HTTP_METHOD_HEAD))
	{
		return HTTP_HEAD;
	}
	else if (HTTPServerTools::EqualASCIIVString (inName, STRING_HTTP_METHOD_PUT))
	{
		return HTTP_PUT;
	}
	else if (HTTPServerTools::EqualASCIIVString (inName, STRING_HTTP_METHOD_POST))
	{
		return HTTP_POST;
	}
	else if (HTTPServerTools::EqualASCIIVString (inName, STRING_HTTP_METHOD_DELETE))
	{
		return HTTP_DELETE;
	}
	else if (HTTPServerTools::EqualASCIIVString (inName, STRING_HTTP_METHOD_TRACE))
	{
		return HTTP_TRACE;
	}
	else if (HTTPServerTools::EqualASCIIVString (inName, STRING_HTTP_METHOD_OPTIONS))
	{
		return HTTP_OPTIONS;
	}

	return HTTP_UNKNOWN;
}


HTTPVersion HTTPProtocol::GetVersionFromRequestLine (const UniChar *inBuffer, sLONG inBufferLen)
{
/*
	YT 02-May-2011	RFC2616 compliance (Test Case HttpMsg_Version01): HTTP/001.001 should strictly equivalent to HTTP/1.1 !!!
					Hope this will not affect performances ?!?!
	Reference: RFC2616 chapter: 3.1 HTTP Version
	[...]
	Leading zeros MUST be ignored by recipients and
	MUST NOT be sent.
*/
#if RFC2616_COMPLIANT 
	if ((NULL != inBuffer) && (inBufferLen > 3))
	{
		sLONG posHTTP = HTTPServerTools::FindASCIICString (inBuffer, "http/");
		if (posHTTP > 0)
		{
			XBOX::VString	httpVersion (const_cast<UniChar *>(inBuffer + posHTTP + 4), -1);
			sLONG			fullStopPos = httpVersion.FindUniChar (CHAR_FULL_STOP);
			if (fullStopPos > 0)
			{
				XBOX::VString	majorVersion;
				XBOX::VString	minorVersion;

				httpVersion.GetSubString (1, fullStopPos - 1, majorVersion);
				httpVersion.GetSubString (fullStopPos + 1, httpVersion.GetLength() - fullStopPos, minorVersion);

				if (majorVersion.GetLong() == 1)
				{
					switch (minorVersion.GetLong())
					{
					case 0:
						return VERSION_1_0;
						break;
					case 1:
						return VERSION_1_1;
						break;
					}
				}
			}
		}
	}

	return VERSION_UNSUPPORTED;
#else
	HTTPVersion version = VERSION_UNSUPPORTED;

	if ((NULL == inBuffer) || (inBufferLen < 3))
		return version;

	sLONG lastCharPos = inBufferLen - 1;

	while ((lastCharPos > 3) && ((inBuffer[lastCharPos] == HTTP_CR) || (inBuffer[lastCharPos] == HTTP_LF)))
		--lastCharPos;

	const UniChar *buffer = inBuffer + (lastCharPos - 2);

	if ((buffer[0] == CHAR_DIGIT_ONE) && (buffer[1] == CHAR_FULL_STOP) && (buffer[2] == CHAR_DIGIT_ONE))
		version = VERSION_1_1;
	else if ((buffer[0] == CHAR_DIGIT_ONE) && (buffer[1] == CHAR_FULL_STOP) && (buffer[2] == CHAR_DIGIT_ZERO))
		version = VERSION_1_0;

	return version;
#endif
}


bool HTTPProtocol::GetRequestURIFromRequestLine (const UniChar *inBuffer, sLONG inBufferLen, XBOX::VString& outURI, bool decodeURI)
{
	if ((NULL == inBuffer) || (inBufferLen < 3))
		return false;

	sLONG startURI = 0;
	sLONG endURI = inBufferLen -1;
	const UniChar *buffer = inBuffer;

	HTTPRequestMethod method = GetMethodFromRequestLine (inBuffer, inBufferLen);
	switch (method)
	{
		case HTTP_HEAD:
		case HTTP_POST:		startURI += 4; break;
		case HTTP_TRACE:	startURI += 5; break;
		case HTTP_DELETE:	startURI += 6; break;
		case HTTP_OPTIONS:	startURI += 7; break;
		case HTTP_GET:
		case HTTP_PUT:
		default:			startURI += 3; break;
	}

	/* We should have an SP before the Request-URI */
	while ((startURI < inBufferLen) && (buffer[startURI] == HTTP_SP))
		++startURI;

	/* And another one after the Request-URI */
	while ((endURI > startURI) && (buffer[endURI] != HTTP_SP))
		--endURI;

	if (endURI > startURI)
	{
		outURI.FromBlock (buffer + startURI, (endURI - startURI) * sizeof(UniChar), XBOX::VTC_UTF_16);

		if (decodeURI)
			VURL::Decode (outURI);
	}
	else
	{
		outURI.Clear();
		return false;
	}

	return true;
}


XBOX::VError HTTPProtocol::ReadRequestLine (const XBOX::VString& inRequestLine, HTTPRequestMethod& outMethod, XBOX::VString& outURI, HTTPVersion& outVersion, bool inCheckValidity)
{
	if (inRequestLine.IsEmpty())
		return VHTTPServer::ThrowError (VE_HTTP_PROTOCOL_BAD_REQUEST, CVSTR ("Request Line is empty !!:<br />"));

	if (inCheckValidity && !HTTPProtocol::IsValidRequestLine (inRequestLine))
	{
		XBOX::VString errorString (CVSTR ("Request Line validation failed !!:<br />"));
		errorString.AppendString (inRequestLine);
		return VHTTPServer::ThrowError (VE_HTTP_PROTOCOL_BAD_REQUEST, errorString);
	}

	const UniChar *	bufferPtr = inRequestLine.GetCPointer();
	XBOX::VString	methodString;
	XBOX::VString	versionString;

	outMethod = HTTP_UNKNOWN;
	outVersion = VERSION_UNKNOWN;
	outURI.Clear();

	while ((NULL != *bufferPtr) && std::isspace (*bufferPtr)) { ++bufferPtr; }
	while ((NULL != *bufferPtr) && !std::isspace (*bufferPtr)) { methodString.AppendUniChar (*bufferPtr); ++bufferPtr; }
	outMethod = GetMethodFromRequestLine (methodString.GetCPointer(), methodString.GetLength());
	if (outMethod == HTTP_UNKNOWN)
		return VE_HTTP_PROTOCOL_NOT_IMPLEMENTED; // VHTTPServer::ThrowError (VE_HTTP_PROTOCOL_NOT_IMPLEMENTED, STRING_EMPTY);
	while ((NULL != *bufferPtr) && std::isspace (*bufferPtr)) { ++bufferPtr; }
	while ((NULL != *bufferPtr) && !std::isspace (*bufferPtr)) { outURI.AppendUniChar (*bufferPtr); ++bufferPtr; }
	if (outURI.IsEmpty())
		return VE_HTTP_PROTOCOL_BAD_REQUEST;	// VHTTPServer::ThrowError (VE_HTTP_PROTOCOL_BAD_REQUEST, STRING_EMPTY);
	while ((NULL != *bufferPtr) && std::isspace (*bufferPtr)) { ++bufferPtr; }
	while ((NULL != *bufferPtr) && !std::isspace (*bufferPtr)) { versionString.AppendUniChar (*bufferPtr); ++bufferPtr; }
	outVersion = GetVersionFromRequestLine (versionString.GetCPointer(), versionString.GetLength());
	if (outVersion == VERSION_UNSUPPORTED)
		return VE_HTTP_PROTOCOL_HTTP_VERSION_NOT_SUPPORTED; // VHTTPServer::ThrowError (VE_HTTP_PROTOCOL_HTTP_VERSION_NOT_SUPPORTED, STRING_EMPTY);

	return XBOX::VE_OK;
}


void HTTPProtocol::ParseURL (const XBOX::VString& inRawURL, XBOX::VString& outDecodedURL, XBOX::VString& outURLPath, XBOX::VString& outURLQuery)
{
	outDecodedURL.FromString (inRawURL);
	XBOX::VURL::Decode (outDecodedURL);

	sLONG questionMarkPos = outDecodedURL.FindUniChar (CHAR_QUESTION_MARK);
	if (questionMarkPos > 0)
	{
		outDecodedURL.GetSubString (1, questionMarkPos - 1, outURLPath);
		outDecodedURL.GetSubString (questionMarkPos + 1, outDecodedURL.GetLength() - questionMarkPos, outURLQuery);
	}
	else
	{
		outURLPath.FromString (outDecodedURL);
		outURLQuery.Clear();
	}
}


/* static */
XBOX::VError HTTPProtocol::ReadHeaderLine (const XBOX::VString& inLineString, XBOX::VString& outHeaderName, XBOX::VString& outHeaderValue)
{
	if (inLineString.IsEmpty())
		return XBOX::VE_OK;

	const UniChar *	bufferPtr = inLineString.GetCPointer();
	const UniChar *	startPtr = bufferPtr;

	while ((NULL != *bufferPtr) && std::isspace (*bufferPtr)) { ++startPtr; ++bufferPtr; }
	while ((NULL != *bufferPtr) && !std::isspace (*bufferPtr) && (*bufferPtr != ':')) { ++bufferPtr; }
	outHeaderName.FromBlock (startPtr, (bufferPtr - startPtr) * sizeof(UniChar), VTC_UTF_16);

	startPtr = bufferPtr;
	while ((NULL != *bufferPtr) && (std::isspace (*bufferPtr) || (*bufferPtr == ':'))) { ++startPtr; ++bufferPtr; }
	outHeaderValue.FromUniCString (startPtr);

	return XBOX::VE_OK;
}


/*
	RFC 2616 - 3.3.1 Full Date

	   HTTP applications have historically allowed three different formats
	   for the representation of date/time stamps:

		  Sun, 06 Nov 1994 08:49:37 GMT  ; RFC 822, updated by RFC 1123
		  Sunday, 06-Nov-94 08:49:37 GMT ; RFC 850, obsoleted by RFC 1036
		  Sun Nov  6 08:49:37 1994       ; ANSI C's asctime() format

	   The first format is preferred as an Internet standard and represents
	   a fixed-length subset of that defined by RFC 1123 [8] (an update to
	   RFC 822 [9]). The second format is in common use, but is based on the
	   obsolete RFC 850 [12] date format and lacks a four-digit year.
	   HTTP/1.1 clients and servers that parse the date value MUST accept
	   all three formats (for compatibility with HTTP/1.0), though they MUST
	   only generate the RFC 1123 format for representing HTTP-date values
	   in header fields. See section 19.3 for further information.

		  Note: Recipients of date values are encouraged to be robust in
		  accepting date values that may have been sent by non-HTTP
		  applications, as is sometimes the case when retrieving or posting
		  messages via proxies/gateways to SMTP or NNTP.

	   All HTTP date/time stamps MUST be represented in Greenwich Mean Time
	   (GMT), without exception. For the purposes of HTTP, GMT is exactly
	   equal to UTC (Coordinated Universal Time). This is indicated in the
	   first two formats by the inclusion of "GMT" as the three-letter
	   abbreviation for time zone, and MUST be assumed when reading the
	   asctime format. HTTP-date is case sensitive and MUST NOT include
	   additional LWS beyond that specifically included as SP in the
	   grammar.

		   HTTP-date    = rfc1123-date | rfc850-date | asctime-date
		   rfc1123-date = wkday "," SP date1 SP time SP "GMT"
		   rfc850-date  = weekday "," SP date2 SP time SP "GMT"
		   asctime-date = wkday SP date3 SP time SP 4DIGIT
		   date1        = 2DIGIT SP month SP 4DIGIT
						  ; day month year (e.g., 02 Jun 1982)
		   date2        = 2DIGIT "-" month "-" 2DIGIT
						  ; day-month-year (e.g., 02-Jun-82)
		   date3        = month SP  (2DIGIT |  (SP 1DIGIT))
						  ; month day (e.g., Jun  2)
		   time         = 2DIGIT ":" 2DIGIT ":" 2DIGIT
						  ; 00:00:00 - 23:59:59
		   wkday        = "Mon" | "Tue" | "Wed"
						| "Thu" | "Fri" | "Sat" | "Sun"
		   weekday      = "Monday" | "Tuesday" | "Wednesday"
						| "Thursday" | "Friday" | "Saturday" | "Sunday"
		   month        = "Jan" | "Feb" | "Mar" | "Apr"
						| "May" | "Jun" | "Jul" | "Aug"
						| "Sep" | "Oct" | "Nov" | "Dec"

		  Note: HTTP requirements for the date/time stamp format apply only
		  to their usage within the protocol stream. Clients and servers are
		  not required to use these formats for user presentation, request
		  logging, etc.
*/

void HTTPProtocol::MakeRFC822GMTDateString (const sLONG inMode, XBOX::VString& outDateString, bool withHeaderName, sLONG inTimeoutSeconds)
{
	XBOX::VTime currentTime;
	XBOX::VTime::Now (currentTime);
	
	switch (inMode)
	{
		case GMT_YESTERDAY:
			currentTime.AddSeconds (-86400);
			break;
			
		case GMT_TOMOROW:
			currentTime.AddSeconds (86400);
			break;

		case GMT_FAR_FUTURE:
			currentTime.AddYears (10);
			break;

		case GMT_AFTER_TIMEOUT:
			currentTime.AddSeconds (inTimeoutSeconds);
			break;

		default:
		case GMT_NOW:
			break;
	}

	MakeRFC822GMTDateString (currentTime, outDateString, withHeaderName);
}


void HTTPProtocol::MakeRFC822GMTDateString (const XBOX::VTime& inTime, XBOX::VString& outDateString, bool withHeaderName)
{
	inTime.GetRfc822String (outDateString);

	if (withHeaderName)
		outDateString.Insert (L"Date: ", 1);
}


void HTTPProtocol::MakeServerString (XBOX::VString& outServerString, bool inSecureMode, bool withHeaderName)
{
	static XBOX::VString sServerString;

	outServerString.Clear();

	if (withHeaderName)
		outServerString.FromCString ("Server: ");

	// Do NOT display version info in Secure Mode
	if (inSecureMode)
	{
		outServerString.AppendString (VHTTPServer::GetServerName());
	}
	else
	{
		if (!HTTPServerTools::BeginsWithASCIIVString (sServerString, VHTTPServer::GetServerName()))
			_MakeServerString (VHTTPServer::GetServerName(), VHTTPServer::GetServerVersion(), sServerString, VHTTPServer::GetShowExtendedInfos());

		outServerString.AppendString (sServerString);
	}
}


void HTTPProtocol::MakeErrorResponse (const XBOX::VError inError, XBOX::VStream& outStream, XBOX::VString *inAdditionnalExplanationString)
{
	if (inError != VE_HTTP_PROTOCOL_UNAUTHORIZED)
	{
		sLONG			statusCode = ERRCODE_FROM_VERROR (inError);
		XBOX::VString	templateString;
		XBOX::VString	explanationString;
		XBOX::VString	errorString;
		XBOX::VString	detailString;

		if (NULL != inAdditionnalExplanationString)
			explanationString.FromString (*inAdditionnalExplanationString);

		if (HTTPProtocol::IsValidStatusCode (statusCode))
		{
			if (((statusCode / 100) == 4) || ((statusCode / 100) == 5))
			{
				errorString.FromLong (statusCode);
				errorString.AppendCString (" - ");
				GetStatusCodeExplanation (statusCode, errorString, false);

				if ((HTTP_INTERNAL_SERVER_ERROR == statusCode) ||
					(HTTP_BAD_REQUEST == statusCode))
				{
					XBOX::VErrorTaskContext *	errorTaskContext = XBOX::VTask::GetCurrent()->GetErrorContext (false);
					XBOX::VErrorContext *		errorContext = (errorTaskContext != NULL) ? errorTaskContext->GetLastContext() : NULL;

					if (errorContext != NULL)
					{
						const XBOX::VErrorStack& errorStack = errorContext->GetErrorStack();

						for (XBOX::VErrorStack::const_iterator it = errorStack.begin(); it != errorStack.end(); ++it)
						{
							XBOX::VString	string;

							(*it)->GetBag()->GetString (STRING_ERROR_PARAMETER_NAME, string);

							if (!string.IsEmpty())
								explanationString.AppendPrintf ("%S<br />", &string);

#if WITH_ASSERT
							(*it)->DumpToString (string);

							if (!string.IsEmpty())
							{
								string.ConvertCarriageReturns (eCRM_CR);
								string.ExchangeAll ("\r", "<br />");
								detailString.AppendPrintf ("%S<br />", &string);
							}
#endif
						}
					}
				}
			}
		}
		else
		{
			errorString.FromString (VHTTPServer::GetServerName());
			errorString.AppendCString (" returns an error: ");
			errorString.AppendLong (statusCode);
		}

		if (!errorString.IsEmpty() && (XBOX::VE_OK == outStream.OpenWriting()))
		{
			VHTTPServer::GetErrorTemplateString (templateString);

			if (templateString.IsEmpty())
			{
				templateString.FromString (errorString);
				if (!explanationString.IsEmpty())
				{
					templateString.AppendCString (" (");
					templateString.AppendString (explanationString);
					templateString.AppendCString (")");
				}
			}
			else
			{
				templateString.ExchangeAll (CVSTR ("[WKT_TITLE]"), errorString);
				templateString.ExchangeAll (CVSTR ("[WKT_MESSAGE]"), explanationString);
				templateString.ExchangeAll (CVSTR ("[WKT_DETAILS]"), detailString);
			}

			XBOX::StStringConverter<char> buffer (templateString, XBOX::VTC_UTF_8);
			outStream.PutData (buffer.GetCPointer(), buffer.GetLength());
			outStream.CloseWriting();
		}
	}
}


void HTTPProtocol::GetEncodingMethodName (HTTPCompressionMethod inMethod, XBOX::VString& outName)
{
	switch (inMethod)
	{
		case COMPRESSION_DEFLATE:
			outName.FromString (STRING_HEADER_VALUE_DEFLATE);
			break;
		case COMPRESSION_GZIP:
			outName.FromString (STRING_HEADER_VALUE_GZIP);
			break;
		case COMPRESSION_COMPRESS:
			outName.FromString (STRING_HEADER_VALUE_COMPRESS);
			break;
		case COMPRESSION_X_GZIP:
			outName.FromString (STRING_HEADER_VALUE_X_GZIP);
			break;
		case COMPRESSION_X_COMPRESS:
			outName.FromString (STRING_HEADER_VALUE_X_COMPRESS);
			break;
		case COMPRESSION_IDENTITY:
			outName.FromString (STRING_HEADER_VALUE_IDENTITY);
			break;
		default:
			outName.Clear();
			break;
	}
}


HTTPCompressionMethod HTTPProtocol::NegotiateEncodingMethod (XBOX::VString& inEncodingHeaderValue)
{
#if HTTP_SERVER_USE_QUALITY_FACTOR

	if (inEncodingHeaderValue.IsEmpty())
		return COMPRESSION_NONE;

	XBOX::VString preferedEncodingValue;
	HTTPProtocol::GetBestQualifiedValueUsingQFactor (inEncodingHeaderValue, preferedEncodingValue);

	/* gzip & x-gzip */
	if (HTTPServerTools::EqualASCIIVString (preferedEncodingValue, STRING_HEADER_VALUE_X_GZIP))
		return COMPRESSION_X_GZIP;
	else if (HTTPServerTools::EqualASCIIVString (preferedEncodingValue, STRING_HEADER_VALUE_GZIP))
		return COMPRESSION_GZIP;
	/* deflate */
	else if (HTTPServerTools::EqualASCIIVString (preferedEncodingValue, STRING_HEADER_VALUE_DEFLATE))
		return COMPRESSION_DEFLATE;
	/* compress & x-compress */
	else if (HTTPServerTools::EqualASCIIVString (preferedEncodingValue, STRING_HEADER_VALUE_X_COMPRESS))
		return COMPRESSION_X_COMPRESS;
	else if (HTTPServerTools::EqualASCIIVString (preferedEncodingValue, STRING_HEADER_VALUE_COMPRESS))
		return COMPRESSION_COMPRESS;
	/* identity (RFC2616 compliance - see Chapter 3.5 Content Codings)*/
	else if (HTTPServerTools::EqualASCIIVString (preferedEncodingValue, STRING_HEADER_VALUE_IDENTITY) ||
			HTTPServerTools::EqualASCIIVString (preferedEncodingValue, STRING_STAR))
		return COMPRESSION_NONE;

#else
	if (inEncodingHeaderValue.IsEmpty() || HTTPServerTools::EqualASCIICString (inEncodingHeaderValue, "*")) // YT 06-Jul-2011 - TestCase ContentCoding06 (See RFC2616 14.3)
		return COMPRESSION_NONE;

	/* gzip & x-gzip */
	if (HTTPServerTools::FindASCIIVString (inEncodingHeaderValue, STRING_HEADER_VALUE_X_GZIP))
		return COMPRESSION_X_GZIP;
	else if (HTTPServerTools::FindASCIIVString (inEncodingHeaderValue, STRING_HEADER_VALUE_GZIP))
		return COMPRESSION_GZIP;
	/* deflate */
	else if (HTTPServerTools::FindASCIIVString (inEncodingHeaderValue, STRING_HEADER_VALUE_DEFLATE))
		return COMPRESSION_DEFLATE;
	/* compress & x-compress */
	else if (HTTPServerTools::FindASCIIVString (inEncodingHeaderValue, STRING_HEADER_VALUE_X_COMPRESS))
		return COMPRESSION_X_COMPRESS;
	else if (HTTPServerTools::FindASCIIVString (inEncodingHeaderValue, STRING_HEADER_VALUE_COMPRESS))
		return COMPRESSION_COMPRESS;
	/* identity (RFC2616 compliance - see Chapter 3.5 Content Codings)*/
	else if (HTTPServerTools::FindASCIIVString (inEncodingHeaderValue, STRING_HEADER_VALUE_IDENTITY))
		return COMPRESSION_NONE;
#endif

	return COMPRESSION_UNKNOWN;
}


bool HTTPProtocol::AcceptEncodingMethod (const XBOX::VString& inEncodingHeaderValue, HTTPCompressionMethod inEncodingMethod)
{
	// "Content-Encoding" required & cached data not compressed --> Accept encoding
	if (!inEncodingHeaderValue.IsEmpty() && (COMPRESSION_NONE == inEncodingMethod))
		return true;
	// No "Content-Encoding" required & cached data compressed --> Refuse encoding
	else if (inEncodingHeaderValue.IsEmpty() && (COMPRESSION_NONE != inEncodingMethod))
		return false;

	VectorOfHeadersWithQFactors	valuesWithQFactors;
	_ExtractValuesWithQFactors (inEncodingHeaderValue, valuesWithQFactors);

	if (valuesWithQFactors.size() > 0)
	{
		XBOX::VString encodingMethodString;
		HTTPProtocol::GetEncodingMethodName (inEncodingMethod, encodingMethodString);

		std::sort (valuesWithQFactors.begin(), valuesWithQFactors.end(), _QFactorComparator);
		for (VectorOfHeadersWithQFactors::const_iterator it = valuesWithQFactors.begin(); it != valuesWithQFactors.end(); ++it)
		{
			if (HTTPServerTools::EqualASCIIVString ((*it).first, encodingMethodString) && ((*it).second > 0.0))
				return true;
		}
	}

	return false;
}


void HTTPProtocol::MakeHTTPMethodString (const HTTPRequestMethod inMethod, XBOX::VString& outString)
{
	switch (inMethod)
	{
		case HTTP_GET:
			outString.FromCString ("GET");
			break;
		case HTTP_HEAD:
			outString.FromCString ("HEAD");
			break;
		case HTTP_POST:
			outString.FromCString ("POST");
			break;
		case HTTP_PUT:
			outString.FromCString ("PUT");
			break;
		case HTTP_DELETE:
			outString.FromCString ("DELETE");
			break;
		case HTTP_TRACE:
			outString.FromCString ("TRACE");
			break;
		case HTTP_OPTIONS:
			outString.FromCString ("OPTIONS");
			break;
		case HTTP_UNKNOWN:
		default:
			outString.FromCString ("-");
			break;
	}
}


void HTTPProtocol::MakeHTTPVersionString (const HTTPVersion inVersion, XBOX::VString& outVersionString)
{
	switch (inVersion)
	{
		case VERSION_1_0:
			outVersionString.FromCString ("HTTP/1.0");
			break;
		case VERSION_1_1:
			outVersionString.FromCString ("HTTP/1.1");
			break;
		default: // unknown HTTP version
			outVersionString.FromCString ("UNKNOWN");
			break;;
	}
}


/* static */
bool HTTPProtocol::MakeHTTPAuthenticateHeaderValue (const HTTPAuthenticationMethod inMethod, const XBOX::VString& inRealm, const XBOX::VString& inDomain, XBOX::VString& outAuthString)
{
	outAuthString.Clear();

	switch (inMethod)
	{
	case AUTH_KERBEROS:
		outAuthString.AppendCString ("Negotiate");
		break;

	case AUTH_BASIC:
		outAuthString.AppendCString ("Basic realm=\"");
		outAuthString.AppendString (inRealm);
		outAuthString.AppendCString ("\"");
		break;

	case AUTH_DIGEST:
		{
			XBOX::VString nonceValue;
			VNonce::GetNewNonceValue (nonceValue);

			outAuthString.AppendCString ("Digest realm=\"");
			outAuthString.AppendString (inRealm);
			outAuthString.AppendCString ("\", qop=\"auth\", nonce=\"");
			outAuthString.AppendString (nonceValue);
			outAuthString.AppendCString ("\", algorithm=\"MD5\", domain=\"");
			outAuthString.AppendString (inDomain);
			outAuthString.AppendCString ("\"");
			break;
		}

	default:
		break;
	}

	return (!outAuthString.IsEmpty());
}


/* static */
bool HTTPProtocol::GetAuthenticationType (const HTTPAuthenticationMethod inMethod, XBOX::VString& outAuthString)
{
	outAuthString.Clear();

	switch (inMethod)
	{
	case AUTH_KERBEROS:
		outAuthString.AppendCString ("KERBEROS");
		break;

	case AUTH_BASIC:
		outAuthString.AppendCString ("BASIC");
		break;

	case AUTH_DIGEST:
		outAuthString.AppendCString ("DIGEST");
		break;

	case AUTH_NTLM:
		outAuthString.AppendCString ("NTLM");
		break;

	default:
		outAuthString.AppendCString ("NONE");
		break;
	}

	return (!outAuthString.IsEmpty());
}


/* static */
void HTTPProtocol::GetBestQualifiedValueUsingQFactor (const XBOX::VString& inFieldValue, XBOX::VString& outValue)
{
	VectorOfHeadersWithQFactors	valuesWithQFactors;
	Real						qFactor = 0.0;

	_ExtractValuesWithQFactors (inFieldValue, valuesWithQFactors);

	for (VectorOfHeadersWithQFactors::const_iterator it = valuesWithQFactors.begin(); it != valuesWithQFactors.end(); ++it)
	{
		if ((*it).second > qFactor)
		{
			qFactor = (*it).second;
			outValue.FromString ((*it).first);
		}
	}
}


/* static */
XBOX::CharSet HTTPProtocol::NegotiateCharSet (XBOX::VString& inAcceptCharsetHeaderValue)
{
	VectorOfHeadersWithQFactors	valuesWithQFactors;
	XBOX::CharSet				charset = XBOX::VTC_UNKNOWN;

	_ExtractValuesWithQFactors (inAcceptCharsetHeaderValue, valuesWithQFactors);

	std::sort (valuesWithQFactors.begin(), valuesWithQFactors.end(), _QFactorComparator);
	for (VectorOfHeadersWithQFactors::const_iterator it = valuesWithQFactors.begin(); it != valuesWithQFactors.end(); ++it)
	{
		if (HTTPServerTools::EqualASCIIVString ((*it).first, STRING_STAR))
			charset = XBOX::VTC_ISO_8859_1;
		else
			charset = ((*it).second > 0.0) ? XBOX::VTextConverters::Get()->GetCharSetFromName ((*it).first) : XBOX::VTC_UNKNOWN;

		if (XBOX::VTC_UNKNOWN != charset)
			break;
	}

	return charset;
}


/* static */
bool HTTPProtocol::IsValidRequestLine (const XBOX::VString& inRequestLine)
{
	if (inRequestLine.IsEmpty())
		return false;

	XBOX::VError	error = XBOX::VE_OK;
	bool			isOK = false;

	if (fRequestRegexMatcher.IsNull())
		fRequestRegexMatcher = XBOX::VRegexMatcher::Create (STRING_REQUEST_VALIDATION_PATTERN, &error);

	if ((!fRequestRegexMatcher.IsNull()) && (XBOX::VE_OK == error))
	{
		XBOX::VTaskLock	lock (&fRequestRegexMatcherMutex);
		isOK = (MatchRegex (fRequestRegexMatcher, inRequestLine, false));
	}

	return isOK;
}


/* static */
bool HTTPProtocol::IsAcceptableMediaType (const XBOX::VString& inAcceptValue, const XBOX::VString& inContentType)
{
	VectorOfHeadersWithQFactors	valuesWithQFactors;

	_ExtractValuesWithQFactors (inAcceptValue, valuesWithQFactors);

	std::sort (valuesWithQFactors.begin(), valuesWithQFactors.end(), _QFactorComparator);
	for (VectorOfHeadersWithQFactors::const_iterator it = valuesWithQFactors.begin(); it != valuesWithQFactors.end(); ++it)
	{
		if (_EqualMediaType ((*it).first, inContentType))
			return true;
	}

	return false;
}

