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
#include "VHTTPServer.h"

#if !VERSIONWIN
	#include <arpa/inet.h>
#endif

//--------------------------------------------------------------------------------------------------


const XBOX::VString STRING_EMPTY						= CVSTR ("");
const XBOX::VString STRING_STAR							= CVSTR ("*");


	/**
	*	Some common HTTP Request headers
	*/
const XBOX::VString STRING_HEADER_ACCEPT				= CVSTR ("Accept");
const XBOX::VString STRING_HEADER_ACCEPT_CHARSET		= CVSTR ("Accept-Charset");
const XBOX::VString STRING_HEADER_ACCEPT_ENCODING		= CVSTR ("Accept-Encoding");
const XBOX::VString STRING_HEADER_ACCEPT_LANGUAGE		= CVSTR ("Accept-Language");
const XBOX::VString STRING_HEADER_AUTHORIZATION			= CVSTR ("Authorization");
const XBOX::VString STRING_HEADER_CONTENT_DISPOSITION	= CVSTR ("Content-Disposition");
const XBOX::VString STRING_HEADER_COOKIE				= CVSTR ("Cookie");
const XBOX::VString STRING_HEADER_EXPECT				= CVSTR ("Expect");
const XBOX::VString STRING_HEADER_FROM					= CVSTR ("From");
const XBOX::VString STRING_HEADER_HOST					= CVSTR ("Host");
const XBOX::VString STRING_HEADER_IF_MATCH				= CVSTR ("If-Match");
const XBOX::VString STRING_HEADER_IF_MODIFIED_SINCE		= CVSTR ("If-Modified-Since");
const XBOX::VString STRING_HEADER_IF_NONE_MATCH			= CVSTR ("If-None-Match");
const XBOX::VString STRING_HEADER_IF_RANGE				= CVSTR ("If-Range");
const XBOX::VString STRING_HEADER_IF_UNMODIFIED_SINCE	= CVSTR ("If-Unmodified-Since");
const XBOX::VString STRING_HEADER_KEEP_ALIVE			= CVSTR ("Keep-Alive");
const XBOX::VString STRING_HEADER_MAX_FORWARDS			= CVSTR ("Max-Forwards");
const XBOX::VString STRING_HEADER_PROXY_AUTHORIZATION	= CVSTR ("Proxy-Authorization");
const XBOX::VString STRING_HEADER_RANGE					= CVSTR ("Range");
const XBOX::VString STRING_HEADER_REFERER				= CVSTR ("Referer");
const XBOX::VString STRING_HEADER_TE					= CVSTR ("TE");							// RFC 2616 - Section 14.39
const XBOX::VString STRING_HEADER_TRANSFER_ENCODING		= CVSTR	("Transfer-Encoding");
const XBOX::VString STRING_HEADER_USER_AGENT			= CVSTR ("User-Agent");

	/**
	*	Some common HTTP Response headers
	*/
const XBOX::VString STRING_HEADER_ACCEPT_RANGES			= CVSTR ("Accept-Ranges");
const XBOX::VString STRING_HEADER_AGE					= CVSTR ("Age");
const XBOX::VString STRING_HEADER_ALLOW					= CVSTR ("Allow");
const XBOX::VString STRING_HEADER_CACHE_CONTROL			= CVSTR ("Cache-Control");
const XBOX::VString STRING_HEADER_CONNECTION			= CVSTR ("Connection");
const XBOX::VString STRING_HEADER_DATE					= CVSTR ("Date");
const XBOX::VString STRING_HEADER_ETAG					= CVSTR ("ETag");
const XBOX::VString STRING_HEADER_CONTENT_ENCODING		= CVSTR ("Content-Encoding");
const XBOX::VString STRING_HEADER_CONTENT_LANGUAGE		= CVSTR ("Content-Language");
const XBOX::VString STRING_HEADER_CONTENT_LENGTH		= CVSTR ("Content-Length");
const XBOX::VString STRING_HEADER_CONTENT_LOCATION		= CVSTR ("Content-Location");
const XBOX::VString STRING_HEADER_CONTENT_MD5			= CVSTR ("Content-MD5");
const XBOX::VString STRING_HEADER_CONTENT_RANGE			= CVSTR ("Content-Range");
const XBOX::VString STRING_HEADER_CONTENT_TYPE			= CVSTR ("Content-Type");
const XBOX::VString STRING_HEADER_EXPIRES				= CVSTR ("Expires");
const XBOX::VString STRING_HEADER_LAST_MODIFIED			= CVSTR ("Last-Modified");
const XBOX::VString STRING_HEADER_LOCATION				= CVSTR ("Location");
const XBOX::VString STRING_HEADER_PRAGMA				= CVSTR ("Pragma");
const XBOX::VString STRING_HEADER_PROXY_AUTHENTICATE	= CVSTR ("Proxy-Authenticate");
const XBOX::VString STRING_HEADER_RETRY_AFTER			= CVSTR ("Retry-After");
const XBOX::VString STRING_HEADER_SERVER				= CVSTR ("Server");
const XBOX::VString STRING_HEADER_SET_COOKIE			= CVSTR ("Set-Cookie");
const XBOX::VString STRING_HEADER_STATUS				= CVSTR ("Status");
const XBOX::VString STRING_HEADER_VARY					= CVSTR ("Vary");
const XBOX::VString STRING_HEADER_WWW_AUTHENTICATE		= CVSTR ("WWW-Authenticate");
const XBOX::VString STRING_HEADER_X_STATUS				= CVSTR ("X-Status");
const XBOX::VString STRING_HEADER_X_POWERED_BY			= CVSTR ("X-Powered-By");
const XBOX::VString STRING_HEADER_X_VERSION				= CVSTR ("X-Version");
	/**
	*	Some custom HTTP headers
	*/
const XBOX::VString STRING_HEADER_X_WA_PATTERN			= CVSTR ("X-WA-Pattern");

	/**
	*	Some common HTTP header values
	*/
const XBOX::VString STRING_HEADER_VALUE_CHUNKED			= CVSTR ("chunked");
const XBOX::VString STRING_HEADER_VALUE_CLOSE			= CVSTR ("close");
const XBOX::VString STRING_HEADER_VALUE_COMPRESS		= CVSTR ("compress");
const XBOX::VString STRING_HEADER_VALUE_DEFLATE			= CVSTR ("deflate");
const XBOX::VString STRING_HEADER_VALUE_GZIP			= CVSTR ("gzip");
const XBOX::VString STRING_HEADER_VALUE_KEEP_ALIVE		= CVSTR ("keep-alive");
const XBOX::VString STRING_HEADER_VALUE_MAX_AGE			= CVSTR ("max-age");
const XBOX::VString STRING_HEADER_VALUE_NONE			= CVSTR ("none");
const XBOX::VString STRING_HEADER_VALUE_NO_CACHE		= CVSTR ("no-cache");
const XBOX::VString STRING_HEADER_VALUE_X_COMPRESS		= CVSTR ("x-compress");
const XBOX::VString STRING_HEADER_VALUE_X_GZIP			= CVSTR ("x-gzip");
const XBOX::VString STRING_HEADER_VALUE_IDENTITY		= CVSTR ("identity");
	/**
	*	Some common HTTP Content-Types
	*/
const XBOX::VString STRING_CONTENT_TYPE_HTML			= CVSTR ("text/html");
const XBOX::VString STRING_CONTENT_TYPE_JSON			= CVSTR ("application/json");
const XBOX::VString STRING_CONTENT_TYPE_MESSAGE			= CVSTR ("message/http");
const XBOX::VString STRING_CONTENT_TYPE_TEXT			= CVSTR ("text/plain");
const XBOX::VString STRING_CONTENT_TYPE_XML				= CVSTR ("application/xml");
const XBOX::VString STRING_CONTENT_TYPE_BINARY			= CVSTR ("application/octet-stream");

	/**
	*	Some common HTTP Authentication schemes
	*/
const XBOX::VString STRING_AUTHENTICATION_BASIC			= CVSTR ("basic");
const XBOX::VString STRING_AUTHENTICATION_DIGEST		= CVSTR ("digest");
const XBOX::VString STRING_AUTHENTICATION_KERBEROS		= CVSTR ("kerberos");
const XBOX::VString STRING_AUTHENTICATION_NEGOTIATE		= CVSTR ("negotiate"); // used in KERBEROS authentication header
const XBOX::VString STRING_AUTHENTICATION_NTLM			= CVSTR ("ntlm");

		/**
		*	Some common HTTP Authentication fields
		*/
const XBOX::VString STRING_AUTH_FIELD_REALM				= CVSTR ("realm");
const XBOX::VString STRING_AUTH_FIELD_NONCE				= CVSTR ("nonce");
const XBOX::VString STRING_AUTH_FIELD_OPAQUE			= CVSTR ("opaque");
const XBOX::VString STRING_AUTH_FIELD_CNONCE			= CVSTR ("cnonce");
const XBOX::VString STRING_AUTH_FIELD_QOP				= CVSTR ("qop");
const XBOX::VString STRING_AUTH_FIELD_NC				= CVSTR ("nc");		// Nonce Count
const XBOX::VString STRING_AUTH_FIELD_ALGORITHM			= CVSTR ("algorithm");
const XBOX::VString STRING_AUTH_FIELD_RESPONSE			= CVSTR ("response");
const XBOX::VString STRING_AUTH_FIELD_USERNAME			= CVSTR ("username");
const XBOX::VString STRING_AUTH_FIELD_URI				= CVSTR ("uri");

		/**
		*	HTTP Method Names
		*/
const XBOX::VString STRING_HTTP_METHOD_GET				= CVSTR ("get");
const XBOX::VString STRING_HTTP_METHOD_HEAD				= CVSTR ("head");
const XBOX::VString STRING_HTTP_METHOD_POST				= CVSTR ("post");
const XBOX::VString STRING_HTTP_METHOD_PUT				= CVSTR ("put");
const XBOX::VString STRING_HTTP_METHOD_DELETE			= CVSTR ("delete");
const XBOX::VString STRING_HTTP_METHOD_TRACE			= CVSTR ("trace");
const XBOX::VString STRING_HTTP_METHOD_OPTIONS			= CVSTR ("options");

		/**
		*	Default Cert & Key file names
		*/
const XBOX::VString STRING_CERT_FILE_NAME				= CVSTR ("cert.pem");
const XBOX::VString STRING_KEY_FILE_NAME				= CVSTR ("key.pem");

		/**
		*	Common Error Strings
		*/
const XBOX::VString STRING_ERROR_INVALID_PARAMETER		= CVSTR ("Invalid Parameter");
const XBOX::VString STRING_ERROR_INVALID_REQUEST_HANDLER= CVSTR ("Invalid Request Handler");
const XBOX::VString STRING_ERROR_ALLOCATION_FAILED		= CVSTR ("Memory Allocation Failed");
const XBOX::VString STRING_ERROR_FAILED_TO_OPEN_STREAM	= CVSTR ("Failed To Open Stream");
const XBOX::VString STRING_ERROR_FILE_LIMIT_REACHED		= CVSTR ("File Limit Reached");
const XBOX::VString STRING_ERROR_PARAMETER_NAME			= CVSTR ("p1"); /* used to set parameter stringin VBaseError Bag*/

const XBOX::VTime TIME_EMPTY_DATE;

#if WITH_DEPRECATED_IPV4_API
const IP4 /*done*/	DEFAULT_LISTENING_ADDRESS = 0;
#else
const XBOX::VString	DEFAULT_LISTENING_ADDRESS = CVSTR("::1");
#endif

const PortNumber	DEFAULT_LISTENING_PORT = 80;
const PortNumber	DEFAULT_LISTENING_SSL_PORT = 443;


//--------------------------------------------------------------------------------------------------


#define LOWERCASE(c) \
	if ((c >= CHAR_LATIN_CAPITAL_LETTER_A) && (c <= CHAR_LATIN_CAPITAL_LETTER_Z))\
		c += 0x0020


template <typename T1, typename T2>
inline bool _EqualASCIICString (const T1 *const inCString1, const sLONG inString1Len, const T2 *const inCString2, const sLONG inString2Len, bool isCaseSensitive)
{
	if (!inCString1 || !inCString2 || (inString1Len != inString2Len))
		return false;
	else if (!(*inCString1) && !(*inCString2) && (inString1Len == 0))
		return true;
	
	bool result = false;
	const T1 *p1 = inCString1;
	const T2 *p2 = inCString2;
	
	while (*p1)
	{
		if ((NULL != p1) && (NULL != p2))
		{
			T1 c1 = *p1;
			T2 c2 = *p2;
			
			if (!isCaseSensitive)
			{
				LOWERCASE (c1);
				LOWERCASE (c2);
			}
			
			if (c1 == c2)
				result = true;
			else
				return false;
			p1++;
			p2++;
		}
		else
		{
			return false;
		}
	}
	
	return result;
}


template <typename T1, typename T2>
inline sLONG _FindASCIICString (const T1 *inText, const sLONG inTextLen, const T2 *inPattern, const sLONG inPatternLen, bool isCaseSensitive)
{
	// see http://fr.wikipedia.org/wiki/Algorithme_de_Knuth-Morris-Pratt

	if (inPatternLen > inTextLen)
		return 0;

	sLONG	textSize = inTextLen;
	sLONG	patternSize = inPatternLen;
	sLONG *	target = (sLONG *)malloc (sizeof(sLONG) * (patternSize + 1));

	if (NULL != target)
	{
		sLONG	m = 0;
		sLONG	i = 0;
		sLONG	j = -1;
		T2		c = '\0';

		target[0] = j;
		while (i < patternSize)
		{
			T2 pchar = inPattern[i];

			if (!isCaseSensitive)
				LOWERCASE (pchar);

			if (pchar == c)
			{
				target[i + 1] = j + 1;
				++j;
				++i;
			}
			else if (j > 0)
			{
				j = target[j];
			}
			else
			{
				target[i + 1] = 0;
				++i;
				j = 0;
			}

			pchar = inPattern[j];
			if (!isCaseSensitive)
				LOWERCASE (pchar);

			c = pchar;
		}

		m = 0;
		i = 0;
		while (((m + i) < textSize) && (i < patternSize))
		{
			T1		tchar = inText[m + i];
			T2		pchar = inPattern[i];

			if (!isCaseSensitive)
			{
				LOWERCASE (tchar);
				LOWERCASE (pchar);
			}

			if (tchar == (T1)pchar)
			{
				++i;
			}
			else
			{
				m += i - target[i];
				if (i > 0)
					i = target[i];
			}
		}

		free (target);

		if (i >= patternSize)
			return m + 1; // 1-based position !!! Just to work as VString.Find()
	}

	return 0;
}


namespace HTTPServerTools {


bool EqualASCIIVString (const XBOX::VString& inString1, const XBOX::VString& inString2, bool isCaseSensitive)
{
	return _EqualASCIICString (inString1.GetCPointer(), inString1.GetLength(), inString2.GetCPointer(), inString2.GetLength(), isCaseSensitive);
}


bool EqualASCIICString (const XBOX::VString& inString1, const char *const inCString2, bool isCaseSensitive)
{
	return _EqualASCIICString (inString1.GetCPointer(), inString1.GetLength(), inCString2, (sLONG)strlen (inCString2), isCaseSensitive);
}


sLONG FindASCIIVString (const XBOX::VString& inText, const XBOX::VString& inPattern, bool isCaseSensitive)
{
	return _FindASCIICString (inText.GetCPointer(), inText.GetLength(), inPattern.GetCPointer(), inPattern.GetLength(), isCaseSensitive);
}


sLONG FindASCIICString (const XBOX::VString& inText, const char *inPattern, bool isCaseSensitive)
{
	return _FindASCIICString (inText.GetCPointer(), inText.GetLength(), inPattern, (sLONG)strlen (inPattern), isCaseSensitive);
}


bool BeginsWithASCIIVString (const XBOX::VString& inText, const XBOX::VString& inPattern, bool isCaseSensitive)
{
	return (_FindASCIICString (inText.GetCPointer(), inText.GetLength(), inPattern.GetCPointer(), inPattern.GetLength(), isCaseSensitive) == 1);
}


bool BeginsWithASCIICString (const XBOX::VString& inText, const char *inPattern, bool isCaseSensitive)
{
	return (_FindASCIICString (inText.GetCPointer(), inText.GetLength(), inPattern, (sLONG)strlen (inPattern), isCaseSensitive) == 1);
}


bool EndsWithASCIIVString (const XBOX::VString& inText, const XBOX::VString& inPattern, bool isCaseSensitive)
{
	sLONG textSize = inText.GetLength();
	sLONG patternSize = inPattern.GetLength();

	return (_FindASCIICString (inText.GetCPointer() + (textSize - patternSize), patternSize, inPattern.GetCPointer(), patternSize, isCaseSensitive) == 1);
}


bool EndsWithASCIICString (const XBOX::VString& inText, const char *inPattern, bool isCaseSensitive)
{
	sLONG textSize = inText.GetLength();
	sLONG patternSize = (sLONG)strlen (inPattern);

	return (_FindASCIICString (inText.GetCPointer() + (textSize - patternSize), patternSize, inPattern, patternSize, isCaseSensitive) == 1);
}


void TrimUniChar (XBOX::VString& ioString, const UniChar inCharToTrim)
{
	if (ioString.GetLength() > 0)
	{
		sLONG			length = ioString.GetLength();
		UniChar *		data = (UniChar *)ioString.GetCPointer();
		XBOX::VIndex	leadingChars = 0;
		XBOX::VIndex	endingChars = 0;

		for (UniChar *p = data, *end = (data + length); (p != end) && (*p == inCharToTrim); p++, leadingChars++);
		for (UniChar *p = (data + length - 1), *start = (data - 1); (p != start) && (*p == inCharToTrim); p--, endingChars++);

		if ((0 != leadingChars) || (0 != endingChars))
		{
			if ((leadingChars + endingChars) >= length)
			{
				ioString.Clear();
			}
			else
			{
				ioString.SubString (leadingChars + 1, length - leadingChars - endingChars);
			}
		}
	}
}


sLONG GetLongFromString (const XBOX::VString& inString)		
{
	bool	isneg = false;		

	XBOX::VIndex sepPos = HTTPServerTools::FindASCIIVString (inString, XBOX::VIntlMgr::GetDefaultMgr()->GetDecimalSeparator());
	if (sepPos <= 0)
		sepPos = inString.GetLength();

	const UniChar* bb = inString.GetCPointer();
	sLONG result = 0;
	for (XBOX::VIndex i = 0; i < sepPos; ++i)
	{
		if ((0 == result) && (bb[i] == CHAR_HYPHEN_MINUS))
			isneg = true;
		if ((bb[i] < CHAR_DIGIT_ZERO) || (bb[i] > CHAR_DIGIT_NINE))
			continue;
		result *= 10;
		result += bb[i] - CHAR_DIGIT_ZERO;
	}

	if (isneg)
		result = -result;
	
	return result;
}


void CopyUniString (UniChar* const target, const UniChar* const src)
{
    if (NULL == src)
    {
        *target = 0;
        return;
    }

    UniChar* pszOut = target;
    const UniChar* pszIn = src;
    while (*pszIn)
        *pszOut++ = *pszIn++;

    // Capp off the target where we ended
    *pszOut = 0;
}


void GetSubString (const XBOX::VString& inString, sLONG inFirst, sLONG inLast, XBOX::VString& outString)
{
	if (testAssert ((inFirst >= 0) && (inLast < inString.GetLength())))
		outString.FromBlock (inString.GetCPointer() + inFirst, (inLast - inFirst + 1) * sizeof(UniChar), XBOX::VTC_UTF_16);
	else
		outString.Clear();
}


void LowerCaseUniASCIIString (UniChar* const toLowerCase)
{
    UniChar* psz1 = toLowerCase;

    if (NULL == psz1)
        return;

    while (*psz1)
	{
		LOWERCASE (*psz1);
		psz1++;        
    }    
}


XBOX::VError CompressStream (XBOX::VStream& ioStream, HTTPCompressionMethod inMethod)
{
	if ((inMethod == COMPRESSION_NONE) || (inMethod > COMPRESSION_LAST_SUPPORTED_METHOD))
		return VE_INVALID_PARAMETER;

	XBOX::VError error = VE_COMP_LIBRARY_NOT_FOUND;

	if (VHTTPServer::GetZipComponentAvailable())
	{
		CZipComponent *zipComponent = VHTTPServer::RetainZipComponent();

		if (NULL != zipComponent)
		{
			XBOX::VPtrStream		compressedStream;
			EZipCompressionLevel	compressionLevel = ((inMethod == COMPRESSION_GZIP) || (inMethod == COMPRESSION_X_GZIP)) ? eCompressionLevel_GZip_BestSpeed : eCompressionLevel_BestSpeed;

			if ((XBOX::VE_OK == (error = ioStream.OpenReading())) && !compressedStream.OpenWriting())
			{
				error = zipComponent->CompressStream (&ioStream, compressionLevel, &compressedStream);
				ioStream.CloseReading();
				compressedStream.CloseWriting();

				if ((XBOX::VE_OK == error) && (!compressedStream.IsEmpty()))
				{
					if (XBOX::VE_OK == (error = ioStream.OpenWriting()))
					{
						ioStream.SetSize (0);
						ioStream.PutData (compressedStream.GetDataPtr(), compressedStream.GetDataSize());
						ioStream.CloseWriting();
					}
				}
			}

			XBOX::QuickReleaseRefCountable (zipComponent);
		}
	}

	return error;
}


XBOX::VError DecompressStream (XBOX::VStream& ioStream)
{
	XBOX::VError error = VE_COMP_LIBRARY_NOT_FOUND;

	if (XBOX::VComponentManager::IsComponentAvailable ((XBOX::CType)CZipComponent::Component_Type))
	{
		CZipComponent *zipComponent = (CZipComponent *)VComponentManager::RetainComponent ((CType)CZipComponent::Component_Type);

		if (zipComponent)
		{
			XBOX::VPtrStream decompressedStream;

			if ((XBOX::VE_OK == (error = ioStream.OpenReading())) && !decompressedStream.OpenWriting())
			{
				error = zipComponent->ExpandStream (&ioStream, &decompressedStream);
				ioStream.CloseReading();
				decompressedStream.CloseWriting();

				if ((XBOX::VE_OK == error) && (!decompressedStream.IsEmpty()))
				{
					if (XBOX::VE_OK == (error = ioStream.OpenWriting()))
					{
						ioStream.SetSize (0);
						ioStream.PutData (decompressedStream.GetDataPtr(), decompressedStream.GetDataSize());
						error = ioStream.CloseWriting();
					}
				}
			}

			zipComponent->Release();
		}
	}

	return error;
}


XBOX::VError Base64Encode (XBOX::VString& ioString)
{
	if (!ioString.EncodeBase64())
		return VE_HTTP_BASE64_ENCODING_FAILED;

	return XBOX::VE_OK;
}


XBOX::VError Base64Decode (XBOX::VString& ioString)
{
	if (!ioString.DecodeBase64())
		return VE_HTTP_BASE64_DECODING_FAILED;

	return XBOX::VE_OK;
}


const XBOX::VString& GetHTTPHeaderName (const HTTPCommonHeaderCode inHeaderCode)
{
	switch (inHeaderCode)
	{
		case HEADER_ACCEPT:						return STRING_HEADER_ACCEPT;
		case HEADER_ACCEPT_CHARSET:				return STRING_HEADER_ACCEPT_CHARSET;
		case HEADER_ACCEPT_ENCODING:			return STRING_HEADER_ACCEPT_ENCODING;
		case HEADER_ACCEPT_LANGUAGE:			return STRING_HEADER_ACCEPT_LANGUAGE;
		case HEADER_AUTHORIZATION:				return STRING_HEADER_AUTHORIZATION;
		case HEADER_COOKIE:						return STRING_HEADER_COOKIE;
		case HEADER_EXPECT:						return STRING_HEADER_EXPECT;
		case HEADER_FROM:						return STRING_HEADER_FROM;
		case HEADER_HOST:						return STRING_HEADER_HOST;
		case HEADER_IF_MATCH:					return STRING_HEADER_IF_MATCH;
		case HEADER_IF_MODIFIED_SINCE:			return STRING_HEADER_IF_MODIFIED_SINCE;
		case HEADER_IF_NONE_MATCH:				return STRING_HEADER_IF_NONE_MATCH;
		case HEADER_IF_RANGE:					return STRING_HEADER_IF_RANGE;
		case HEADER_IF_UNMODIFIED_SINCE:		return STRING_HEADER_IF_UNMODIFIED_SINCE;
		case HEADER_KEEP_ALIVE:					return STRING_HEADER_KEEP_ALIVE;
		case HEADER_MAX_FORWARDS:				return STRING_HEADER_MAX_FORWARDS;
		case HEADER_PROXY_AUTHORIZATION:		return STRING_HEADER_PROXY_AUTHORIZATION;
		case HEADER_RANGE:						return STRING_HEADER_RANGE;
		case HEADER_REFERER:					return STRING_HEADER_REFERER;
		case HEADER_TE:							return STRING_HEADER_TE;							// RFC 2616 - Section 14.39
		case HEADER_USER_AGENT:					return STRING_HEADER_USER_AGENT;

		case HEADER_ACCEPT_RANGES:				return STRING_HEADER_ACCEPT_RANGES;
		case HEADER_AGE:						return STRING_HEADER_AGE;
		case HEADER_ALLOW:						return STRING_HEADER_ALLOW;
		case HEADER_CACHE_CONTROL:				return STRING_HEADER_CACHE_CONTROL;
		case HEADER_CONNECTION:					return STRING_HEADER_CONNECTION;
		case HEADER_DATE:						return STRING_HEADER_DATE;
		case HEADER_ETAG:						return STRING_HEADER_ETAG;
		case HEADER_CONTENT_ENCODING:			return STRING_HEADER_CONTENT_ENCODING;
		case HEADER_CONTENT_LANGUAGE:			return STRING_HEADER_CONTENT_LANGUAGE;
		case HEADER_CONTENT_LENGTH:				return STRING_HEADER_CONTENT_LENGTH;
		case HEADER_CONTENT_LOCATION:			return STRING_HEADER_CONTENT_LOCATION;
		case HEADER_CONTENT_MD5:				return STRING_HEADER_CONTENT_MD5;
		case HEADER_CONTENT_RANGE:				return STRING_HEADER_CONTENT_RANGE;
		case HEADER_CONTENT_TYPE:				return STRING_HEADER_CONTENT_TYPE;
		case HEADER_EXPIRES:					return STRING_HEADER_EXPIRES;
		case HEADER_LAST_MODIFIED:				return STRING_HEADER_LAST_MODIFIED;
		case HEADER_LOCATION:					return STRING_HEADER_LOCATION;
		case HEADER_PRAGMA:						return STRING_HEADER_PRAGMA;
		case HEADER_PROXY_AUTHENTICATE:			return STRING_HEADER_PROXY_AUTHENTICATE;
		case HEADER_RETRY_AFTER:				return STRING_HEADER_RETRY_AFTER;
		case HEADER_SERVER:						return STRING_HEADER_SERVER;
		case HEADER_SET_COOKIE:					return STRING_HEADER_SET_COOKIE;
		case HEADER_STATUS:						return STRING_HEADER_STATUS;
		case HEADER_VARY:						return STRING_HEADER_VARY;
		case HEADER_WWW_AUTHENTICATE:			return STRING_HEADER_WWW_AUTHENTICATE;
		case HEADER_X_STATUS:					return STRING_HEADER_X_STATUS;
		case HEADER_X_POWERED_BY:				return STRING_HEADER_X_POWERED_BY;
		case HEADER_X_VERSION:					return STRING_HEADER_X_VERSION;
	}

	return STRING_EMPTY;
}


const XBOX::VString& GetHTTPHeaderValue (const HTTPCommonValueCode inValueCode)
{
	switch (inValueCode)
	{
		case HEADER_VALUE_CLOSE:				return STRING_HEADER_VALUE_CLOSE;
		case HEADER_VALUE_COMPRESS:				return STRING_HEADER_VALUE_COMPRESS;
		case HEADER_VALUE_DEFLATE:				return STRING_HEADER_VALUE_DEFLATE;
		case HEADER_VALUE_GZIP:					return STRING_HEADER_VALUE_GZIP;
		case HEADER_VALUE_KEEP_ALIVE:			return STRING_HEADER_VALUE_KEEP_ALIVE;
		case HEADER_VALUE_MAX_AGE:				return STRING_HEADER_VALUE_MAX_AGE;
		case HEADER_VALUE_NONE:					return STRING_HEADER_VALUE_NONE;
		case HEADER_VALUE_NO_CACHE:				return STRING_HEADER_VALUE_NO_CACHE;
		case HEADER_VALUE_X_COMPRESS:			return STRING_HEADER_VALUE_X_COMPRESS;
		case HEADER_VALUE_X_GZIP:				return STRING_HEADER_VALUE_X_GZIP;
	}

	return STRING_EMPTY;
}


const XBOX::VString& GetHTTPContentType (const HTTPCommonContentType inCode)
{
	switch (inCode)
	{
		case HTTP_CONTENT_TYPE_HTML:			return STRING_CONTENT_TYPE_HTML;
		case HTTP_CONTENT_TYPE_JSON:			return STRING_CONTENT_TYPE_JSON;
		case HTTP_CONTENT_TYPE_MESSAGE:			return STRING_CONTENT_TYPE_MESSAGE;
		case HTTP_CONTENT_TYPE_TEXT:			return STRING_CONTENT_TYPE_TEXT;
		case HTTP_CONTENT_TYPE_XML:				return STRING_CONTENT_TYPE_XML;
		case HTTP_CONTENT_TYPE_BINARY:			return STRING_CONTENT_TYPE_BINARY;
	}

	return STRING_EMPTY;
}

	
#if WITH_DEPRECATED_IPV4_API
	
void MakeIPv4AddressString (IP4 /*done*/ inIPv4, XBOX::VString& outIPv4String)
{
	struct in_addr	addr = {0};
	char *			buffer = NULL;

	addr.s_addr = htonl (inIPv4);
	buffer = inet_ntoa (addr);

	if (NULL != buffer)
		outIPv4String.AppendCString (buffer);
	else
		outIPv4String.Clear();
}


void GetIPv4FromString (const XBOX::VString& inIPv4String, IP4& /*done*/ outIPv4)
{
	sLONG	hostNameSize = inIPv4String.GetLength() + 1;
	char *	hostName = new char[hostNameSize];

	outIPv4 = INADDR_NONE;
	if (NULL != hostName)
	{
		struct hostent *	remoteHost = NULL;
		struct in_addr		addr = {0};

		inIPv4String.ToCString (hostName, hostNameSize);

		if (isalpha (hostName[0]))	// host address is a name
		{
			remoteHost = gethostbyname (hostName);
		}
		else
		{
			addr.s_addr = inet_addr (hostName);
			if (addr.s_addr != INADDR_NONE)
				remoteHost = gethostbyaddr ((char *) &addr, 4, AF_INET);
		}

		if (NULL != remoteHost)
			outIPv4 = *((sLONG *)remoteHost->h_addr_list[0]);

		delete [] hostName;
	}
}

#endif

#if WITH_DEPRECATED_IPV4_API	
void MakeHostString (IP4 /*done*/ inIPv4, PortNumber inPort, XBOX::VString& outHostString)
{
	XBOX::VString IPv4String;

	MakeIPv4AddressString (inIPv4, IPv4String);
	MakeHostString (IPv4String, inPort, outHostString);
}
#endif

void MakeHostString (const XBOX::VString& inHost, PortNumber inPort, XBOX::VString& outHostString)
{
	outHostString.Clear();

	if (!inHost.IsEmpty())
		outHostString.AppendString (inHost);

	if (inPort != DEFAULT_LISTENING_PORT)
	{
		outHostString.AppendUniChar (CHAR_COLON);
		outHostString.AppendLong (inPort);
	}
}


#if WITH_DEPRECATED_IPV4_API
void ParseHostString (const XBOX::VString& inHostString, IP4& /*done*/ outIPv4, PortNumber& outPort)
{
	XBOX::VString	ipv4String;

	ParseHostString (inHostString, ipv4String, outPort);
	GetIPv4FromString (ipv4String, outIPv4);
}
#endif

	
void ParseHostString (const XBOX::VString& inHostString, XBOX::VString& outIPString, PortNumber& outPort)
{
	XBOX::VIndex pos = inHostString.FindUniChar (CHAR_COLON);

	if (pos > 0)
	{
		XBOX::VString	portString;
		inHostString.GetSubString (1, pos - 1, outIPString);
		inHostString.GetSubString (pos + 1, inHostString.GetLength() - pos, portString);

		outPort = portString.GetLong();
	}
	else
	{
		outIPString.FromString (inHostString);
		outPort = DEFAULT_LISTENING_PORT;
	}
}


//--------------------------------------------------------------------------------------------------


const EHTTPServerLogToken CLF_LOG_TOKEN_LIST[] = {
	LOG_TOKEN_ELF_C_IP,
	LOG_TOKEN_RFC_931,
	LOG_TOKEN_USER,
	LOG_TOKEN_DATE,
	LOG_TOKEN_TIME,
	LOG_TOKEN_HTTP_REQUEST,
	LOG_TOKEN_STATUS,
	LOG_TOKEN_BYTES_SENT,
	LOG_TOKEN_END
};

const EHTTPServerLogToken DLF_LOG_TOKEN_LIST[] = {
	LOG_TOKEN_ELF_C_IP,
	LOG_TOKEN_RFC_931,
	LOG_TOKEN_USER,
	LOG_TOKEN_DATE,
	LOG_TOKEN_TIME,
	LOG_TOKEN_HTTP_REQUEST,
	LOG_TOKEN_STATUS,
	LOG_TOKEN_BYTES_SENT,
	LOG_TOKEN_CS_REFERER,
	LOG_TOKEN_CS_USER_AGENT,
	LOG_TOKEN_END
};

const EHTTPServerLogToken ELF_LOG_TOKEN_LIST[] = {
	LOG_TOKEN_BYTES_SENT,
	LOG_TOKEN_ELF_C_DNS,
	LOG_TOKEN_ELF_C_IP,
	LOG_TOKEN_ELF_CS_COOKIE,
	LOG_TOKEN_ELF_CS_HOST,
	LOG_TOKEN_CS_REFERER,
	LOG_TOKEN_CS_USER_AGENT,
	LOG_TOKEN_USER,
	LOG_TOKEN_METHOD,
	LOG_TOKEN_ELF_S_IP,
	LOG_TOKEN_STATUS,
	LOG_TOKEN_ELF_URI,
	LOG_TOKEN_ELF_CS_URI_QUERY,
	LOG_TOKEN_ELF_CS_URI_STEM,
	LOG_TOKEN_DATE,
	LOG_TOKEN_TIME,
	LOG_TOKEN_TRANSFER_TIME,
	LOG_TOKEN_END
};

const EHTTPServerLogToken WLF_LOG_TOKEN_LIST[] = {
	LOG_TOKEN_WLF_BYTES_RECEIVED,
	LOG_TOKEN_BYTES_SENT,
	LOG_TOKEN_ELF_C_DNS,
	LOG_TOKEN_ELF_C_IP,
	LOG_TOKEN_CONNECTION_ID,
	LOG_TOKEN_ELF_CS_COOKIE,
	LOG_TOKEN_ELF_CS_HOST,
	LOG_TOKEN_CS_REFERER,
	LOG_TOKEN_CS_USER_AGENT,
	LOG_TOKEN_ELF_S_IP,
	LOG_TOKEN_ELF_URI,
	LOG_TOKEN_ELF_CS_URI_QUERY,
	LOG_TOKEN_ELF_CS_URI_STEM,
	LOG_TOKEN_DATE,
	LOG_TOKEN_METHOD,
	LOG_TOKEN_PATH_ARGS,
	LOG_TOKEN_STATUS,
	LOG_TOKEN_TIME,
	LOG_TOKEN_TRANSFER_TIME,
	LOG_TOKEN_URL,
	LOG_TOKEN_USER,
	LOG_TOKEN_END
};


void GetLogFormatName (const EHTTPServerLogFormat inLogFormat, XBOX::VString& outLogFormatName)
{
	switch (inLogFormat)
	{
		case LOG_FORMAT_WLF:		outLogFormatName.FromCString ("WLF");			break;
		case LOG_FORMAT_CLF:		outLogFormatName.FromCString ("CLF");			break;
		case LOG_FORMAT_ELF:		outLogFormatName.FromCString ("ELF");			break;
		case LOG_FORMAT_DLF:		outLogFormatName.FromCString ("DLF");			break;
		default:					outLogFormatName.FromCString ("Unknown");		break;
	}
}


EHTTPServerLogFormat GetLogFormatFromName (const XBOX::VString& inLogFormatName)
{
	if (HTTPServerTools::EqualASCIICString (inLogFormatName, "clf"))
		return LOG_FORMAT_CLF;
	else if (HTTPServerTools::EqualASCIICString (inLogFormatName, "dlf"))
		return LOG_FORMAT_DLF;
	else if (HTTPServerTools::EqualASCIICString (inLogFormatName, "elf"))
		return LOG_FORMAT_ELF;
	else if (HTTPServerTools::EqualASCIICString (inLogFormatName, "wlf"))
		return LOG_FORMAT_WLF;

	return LOG_FORMAT_NO_LOG;
}


void GetLogTokenName (const EHTTPServerLogToken inToken, XBOX::VString& outTokenName)
{
	switch (inToken)
	{
		case LOG_TOKEN_DATE:				outTokenName.FromCString ("DATE");				break;
		case LOG_TOKEN_TIME:				outTokenName.FromCString ("TIME");				break;
		case LOG_TOKEN_HOST_NAME:			outTokenName.FromCString ("HOST-NAME");			break;
		case LOG_TOKEN_URL:					outTokenName.FromCString ("URL");				break;
		case LOG_TOKEN_PATH_ARGS:			outTokenName.FromCString ("PATH_ARGS");			break;
		case LOG_TOKEN_SEARCH_ARGS:			outTokenName.FromCString ("SEARCH_ARGS");		break;
		case LOG_TOKEN_METHOD:				outTokenName.FromCString ("METHOD");			break;
		case LOG_TOKEN_ELF_URI:				outTokenName.FromCString ("CS-URI");			break;
		case LOG_TOKEN_BYTES_SENT:			outTokenName.FromCString ("BYTES_SENT");		break;
		case LOG_TOKEN_TRANSFER_TIME:		outTokenName.FromCString ("TIME-TAKEN");		break;
		case LOG_TOKEN_AGENT:				outTokenName.FromCString ("AGENT");				break;
		case LOG_TOKEN_USER:				outTokenName.FromCString ("USER");				break;
		case LOG_TOKEN_REFERER:				outTokenName.FromCString ("REFERER");			break;
		case LOG_TOKEN_CONNECTION_ID:		outTokenName.FromCString ("CONNECTION_ID");		break;
		case LOG_TOKEN_STATUS:				outTokenName.FromCString ("SC-STATUS");			break;
		case LOG_TOKEN_ELF_C_IP:			outTokenName.FromCString ("C-IP");				break;
		case LOG_TOKEN_ELF_C_DNS:			outTokenName.FromCString ("CS-CDNS");			break;
		case LOG_TOKEN_ELF_CS_URI_STEM:		outTokenName.FromCString ("CS-URI-STEM");		break;
		case LOG_TOKEN_ELF_CS_URI_QUERY:	outTokenName.FromCString ("CS-URI-QUERY");		break;
		case LOG_TOKEN_ELF_CS_HOST:			outTokenName.FromCString ("CS(HOST)");			break;
		case LOG_TOKEN_CS_REFERER:			outTokenName.FromCString ("CS(REFERER)");		break;
		case LOG_TOKEN_CS_USER_AGENT:		outTokenName.FromCString ("CS(USER-AGENT)");	break;
		case LOG_TOKEN_ELF_CS_COOKIE:		outTokenName.FromCString ("CS(COOKIE)");		break;
		case LOG_TOKEN_ELF_S_IP:			outTokenName.FromCString ("CS-SIP");			break;
		case LOG_TOKEN_WLF_BYTES_RECEIVED:	outTokenName.FromCString ("BYTES_RECEIVED");	break;
		case LOG_TOKEN_RFC_931:				outTokenName.FromCString ("RFC-931");			break;
		case LOG_TOKEN_HTTP_REQUEST:		outTokenName.FromCString ("HTTP-REQUEST");		break;
		default:						assert(false);
										outTokenName.FromCString ("UNKNOWN");			break;
	}
}


EHTTPServerLogToken GetLogTokenFromName (const XBOX::VString& inTokenName)
{
	if (HTTPServerTools::EqualASCIICString (inTokenName, "date"))
		return LOG_TOKEN_DATE;
	else if (HTTPServerTools::EqualASCIICString (inTokenName, "time"))
		return LOG_TOKEN_TIME;
	else if (HTTPServerTools::EqualASCIICString (inTokenName, "host-name"))
		return LOG_TOKEN_HOST_NAME;
	else if (HTTPServerTools::EqualASCIICString (inTokenName, "url"))
		return LOG_TOKEN_URL;
	else if (HTTPServerTools::EqualASCIICString (inTokenName, "path_args"))
		return LOG_TOKEN_PATH_ARGS;
	else if (HTTPServerTools::EqualASCIICString (inTokenName, "search_args"))
		return LOG_TOKEN_SEARCH_ARGS;
	else if (HTTPServerTools::EqualASCIICString (inTokenName, "method"))
		return LOG_TOKEN_METHOD;
	else if (HTTPServerTools::EqualASCIICString (inTokenName, "cs-uri"))
		return LOG_TOKEN_ELF_URI;
	else if (HTTPServerTools::EqualASCIICString (inTokenName, "bytes_sent") || HTTPServerTools::EqualASCIICString (inTokenName, "bytes-sent"))
		return LOG_TOKEN_BYTES_SENT;
	else if (HTTPServerTools::EqualASCIICString (inTokenName, "time-taken") || HTTPServerTools::EqualASCIICString (inTokenName, "transfert_time"))
		return LOG_TOKEN_TRANSFER_TIME;
	else if (HTTPServerTools::EqualASCIICString (inTokenName, "agent"))
		return LOG_TOKEN_AGENT;
	else if (HTTPServerTools::EqualASCIICString (inTokenName, "user"))
		return LOG_TOKEN_USER;
	else if (HTTPServerTools::EqualASCIICString (inTokenName, "referer"))
		return LOG_TOKEN_REFERER;
	else if (HTTPServerTools::EqualASCIICString (inTokenName, "connection_id"))
		return LOG_TOKEN_CONNECTION_ID;
	else if (HTTPServerTools::EqualASCIICString (inTokenName, "sc-status") || HTTPServerTools::EqualASCIICString (inTokenName, "status"))
		return LOG_TOKEN_STATUS;
	else if (HTTPServerTools::EqualASCIICString (inTokenName, "c-ip"))
		return LOG_TOKEN_ELF_C_IP;
	else if (HTTPServerTools::EqualASCIICString (inTokenName, "cs-cdns") || HTTPServerTools::EqualASCIICString (inTokenName, "c-dns"))
		return LOG_TOKEN_ELF_C_DNS;
	else if (HTTPServerTools::EqualASCIICString (inTokenName, "cs-uri-stem"))
		return LOG_TOKEN_ELF_CS_URI_STEM;
	else if (HTTPServerTools::EqualASCIICString (inTokenName, "cs-uri-query"))
		return LOG_TOKEN_ELF_CS_URI_QUERY;
	else if (HTTPServerTools::EqualASCIICString (inTokenName, "cs(host)"))
		return LOG_TOKEN_ELF_CS_HOST;
	else if (HTTPServerTools::EqualASCIICString (inTokenName, "cs(referer)"))
		return LOG_TOKEN_CS_REFERER;
	else if (HTTPServerTools::EqualASCIICString (inTokenName, "cs(user-agent)"))
		return LOG_TOKEN_CS_USER_AGENT;
	else if (HTTPServerTools::EqualASCIICString (inTokenName, "cs(cookie)"))
		return LOG_TOKEN_ELF_CS_COOKIE;
	else if (HTTPServerTools::EqualASCIICString (inTokenName, "cs-sip"))
		return LOG_TOKEN_ELF_S_IP;
	else if (HTTPServerTools::EqualASCIICString (inTokenName, "bytes_received"))
		return LOG_TOKEN_WLF_BYTES_RECEIVED;
	else if (HTTPServerTools::EqualASCIICString (inTokenName, "rfc-931"))
		return LOG_TOKEN_RFC_931;
	else if (HTTPServerTools::EqualASCIICString (inTokenName, "http-request"))
		return LOG_TOKEN_HTTP_REQUEST;

	return LOG_TOKEN_NONE;
}


void GetDefaultLogTokenList (const EHTTPServerLogFormat inLogFormat, VectorOfLogToken& outLogTokens)
{
	const EHTTPServerLogToken * defaultTokens = NULL;

	switch (inLogFormat)
	{
		case LOG_FORMAT_WLF:		defaultTokens = WLF_LOG_TOKEN_LIST;			break;
		case LOG_FORMAT_CLF:		defaultTokens = CLF_LOG_TOKEN_LIST;			break;
		case LOG_FORMAT_ELF:		defaultTokens = ELF_LOG_TOKEN_LIST;			break;
		case LOG_FORMAT_DLF:		defaultTokens = DLF_LOG_TOKEN_LIST;			break;
	}

	outLogTokens.clear();

	if (NULL != defaultTokens)
	{
		sLONG i = 0;

		while (defaultTokens[i] != LOG_TOKEN_END)
		{
			if ((defaultTokens[i] > LOG_TOKEN_NONE) && (defaultTokens[i] < LOG_TOKEN_END))
				AppendUniqueValueToVector (outLogTokens, defaultTokens[i]);
			++i;
		}
	}
}


void GetLogTokenNamesList (const VectorOfLogToken& inLogTokens, XBOX::VString& outTokenNames, const UniChar inSeparator)
{
	XBOX::VString						string;
	VectorOfLogToken::const_iterator	nextToken = inLogTokens.begin();

	outTokenNames.Clear();

	for (VectorOfLogToken::const_iterator it = inLogTokens.begin(); it != inLogTokens.end(); ++it)
	{
		GetLogTokenName (*it, string);
		outTokenNames.AppendString (string);
		nextToken = it; 
		if (++nextToken != inLogTokens.end())
			outTokenNames.AppendUniChar (CHAR_SPACE);
	}
}


//--------------------------------------------------------------------------------------------------


XBOX::VError SendValueBagResponse (IHTTPResponse& ioResponse, const XBOX::VValueBag& inBag, const XBOX::VString& inBagName)
{
	XBOX::VError		error = XBOX::VE_OK;
	const XBOX::VString	stringURL = ioResponse.GetRequest().GetURLQuery();
	HTTPRequestMethod	method = ioResponse.GetRequest().GetRequestMethod();
	XBOX::VString		resultString;
	bool				isJSON = true;
	bool				prettyFormatting = false;
	bool				isValidRequest = ((method == HTTP_GET) || (method == HTTP_HEAD));

	if (isValidRequest)
	{
		sLONG			posFormat = 0, posPretty = 0;
		const UniChar *	stringPtr = stringURL.GetCPointer();
		const sLONG		stringLen = stringURL.GetLength();

		if ((posFormat = HTTPServerTools::FindASCIICString (stringPtr, "format=")) > 0)
		{
			posFormat += 6;

			sLONG startPos = 0;
			sLONG endPos = HTTPServerTools::FindASCIICString (stringPtr + posFormat, "&");
			if (endPos <= 0)
				endPos = stringLen;
			else
				endPos += (posFormat - 1);

			if (((startPos = HTTPServerTools::FindASCIICString (stringPtr + posFormat, "xml")) > 0) && (startPos < endPos))
				isJSON = false;
			else if(((startPos = HTTPServerTools::FindASCIICString (stringPtr + posFormat, "json")) > 0) && (startPos < endPos))
				isJSON = true;
			else
				isValidRequest = false;
		}

		if ((posPretty = HTTPServerTools::FindASCIICString (stringPtr, "pretty=")) > 0)
		{
			XBOX::VString prettyString;

			posPretty += 6;
			sLONG endPos = HTTPServerTools::FindASCIICString (stringPtr + posPretty, "&");
			if (endPos <= 0)
				endPos = stringLen;
			else
				endPos += (posPretty - 1);

			if (endPos > posPretty)
			{
				GetSubString (stringURL, posPretty, endPos - 1, prettyString);
				prettyFormatting = (HTTPServerTools::EqualASCIICString (prettyString, "yes"));
			}
			else
				isValidRequest = false;
		}
	}

	if (isValidRequest)
	{
		if (isJSON)
		{
			inBag.GetJSONString (resultString, prettyFormatting ? JSON_PrettyFormatting : JSON_Default);
		}
		else
		{
			resultString.FromCString ("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
			inBag.DumpXML (resultString,  inBagName, prettyFormatting);
		}

		XBOX::StStringConverter<char> buffer (resultString, XBOX::VTC_UTF_8);

		error = ioResponse.SetResponseBody (buffer.GetCPointer(), buffer.GetLength());

		ioResponse.SetExpiresHeader (GMT_NOW);
		ioResponse.AddResponseHeader (STRING_HEADER_PRAGMA, STRING_HEADER_VALUE_NO_CACHE);
		ioResponse.AddResponseHeader (STRING_HEADER_CONTENT_TYPE, (isJSON) ? STRING_CONTENT_TYPE_JSON : STRING_CONTENT_TYPE_XML);

		ioResponse.SetContentLengthHeader (buffer.GetLength());
		ioResponse.AllowCompression (true);
	}
	else
	{
		error = ioResponse.ReplyWithStatusCode (HTTP_BAD_REQUEST);
	}

	return error;
}


//--------------------------------------------------------------------------------------------------


HTTPAuthenticationMethod GetAuthenticationMethodFromName (const XBOX::VString& inAuthenticationMethodName)
{
	if (HTTPServerTools::EqualASCIICString (inAuthenticationMethodName, "basic"))
		return AUTH_BASIC;
	else if (HTTPServerTools::EqualASCIICString (inAuthenticationMethodName, "digest"))
		return AUTH_DIGEST;
	else if (HTTPServerTools::EqualASCIICString (inAuthenticationMethodName, "kerberos"))
		return AUTH_KERBEROS;
	else if (HTTPServerTools::EqualASCIICString (inAuthenticationMethodName, "ntlm"))
		return AUTH_NTLM;

	return AUTH_NONE;
}


void GetAuthenticationMethodName (const HTTPAuthenticationMethod inAuthenticationMethod, XBOX::VString& outAuthenticationMethodName)
{
	switch (inAuthenticationMethod)
	{
		case AUTH_BASIC:				outAuthenticationMethodName.FromCString ("Basic");			break;
		case AUTH_DIGEST:				outAuthenticationMethodName.FromCString ("Digest");			break;
		case AUTH_KERBEROS:				outAuthenticationMethodName.FromCString ("Kerberos");		break;
		case AUTH_NTLM:					outAuthenticationMethodName.FromCString ("NTLM");			break;
		default:						outAuthenticationMethodName.FromCString ("None");			break;
	}
}


void MakeRFC822GMTDateString (const sLONG inMode, XBOX::VString& outDateString, bool withHeaderName, sLONG inTimeoutSeconds)
{
	HTTPProtocol::MakeRFC822GMTDateString (inMode, outDateString, withHeaderName, inTimeoutSeconds);
}


void MakeIPv4String (const uLONG inIPv4, XBOX::VString& outString)
{
	outString.FromLong (inIPv4 >> 24);
	outString.AppendUniChar (CHAR_FULL_STOP);
	outString.AppendLong ((inIPv4 >> 16) & 0x00FF);
	outString.AppendUniChar (CHAR_FULL_STOP);
	outString.AppendLong ((inIPv4 >> 8) & 0x00FF);
	outString.AppendUniChar (CHAR_FULL_STOP);
	outString.AppendLong (inIPv4 & 0x00FF);
}


bool ExtractFieldNameValue (const XBOX::VString& inString, XBOX::VString& outName, XBOX::VString& outValue)
{
	bool					isOK = false;
	XBOX::VectorOfVString	nameValueStrings;

	outName.Clear();
	outValue.Clear();

	if (inString.GetSubStrings (CHAR_EQUALS_SIGN, nameValueStrings, true, true))
	{
		if (nameValueStrings.size())
			outName = nameValueStrings.at (0);

		if (nameValueStrings.size() > 1)
		{
			outValue = nameValueStrings.at (1);

			// Clean-up value String
			TrimUniChar (outValue, CHAR_QUOTATION_MARK);
			TrimUniChar (outValue, CHAR_SPACE);

			isOK = true;
		}
	}

	return isOK;
}


XBOX::VError GetFileInfos (const XBOX::VFilePath& inFilePath, XBOX::VTime *outModificationTime, sLONG8 *outFileSize)
{
	XBOX::VError error = XBOX::VE_MEMORY_FULL;
	XBOX::VFile *file = new XBOX::VFile (inFilePath);

	if (NULL != file)
	{
		if (file->Exists())
		{
			if ((NULL != outModificationTime) || (NULL != outFileSize))
			{
				if (NULL != outModificationTime)
					error = file->GetTimeAttributes (outModificationTime);

				if ((XBOX::VE_OK == error) && (NULL != outFileSize))
					error = file->GetSize (outFileSize);
			}
			else
			{
				error = XBOX::VE_INVALID_PARAMETER;
			}
		}
		else
		{
			error = XBOX::VE_FILE_NOT_FOUND;
		}

		XBOX::QuickReleaseRefCountable (file);
	}

	return error;
}


void ExtractContentTypeAndCharset (const XBOX::VString& inString, XBOX::VString& outContentType, XBOX::CharSet& outCharSet)
{
	outContentType.FromString (inString);
	outCharSet = XBOX::VTC_UNKNOWN;

	if (!outContentType.IsEmpty())
	{
		sLONG pos = HTTPServerTools::FindASCIICString (outContentType, "charset=");
		if (pos > 0)
		{
			XBOX::VString charSetName;
			charSetName.FromBlock (outContentType.GetCPointer() + pos + 7, (outContentType.GetLength() - (pos + 7)) * sizeof(UniChar), XBOX::VTC_UTF_16);
			if (!charSetName.IsEmpty())
			{
				outCharSet = XBOX::VTextConverters::Get()->GetCharSetFromName (charSetName);
			}

			outContentType.SubString (1, outContentType.FindUniChar (CHAR_SEMICOLON) - 1);
			HTTPServerTools::TrimUniChar (outContentType, CHAR_SPACE);
		}
	}
}


MimeTypeKind GetMimeTypeKind (const XBOX::VString& inContentType)
{
	// Test "image" first because of "image/svg+xml"...

	if (HTTPServerTools::FindASCIICString (inContentType, "image/") == 1)
	{
		return MIMETYPE_IMAGE;
	}
	else if ((HTTPServerTools::FindASCIICString (inContentType, "text/") == 1) ||
		(HTTPServerTools::FindASCIICString (inContentType, "/json") > 0) ||
		(HTTPServerTools::FindASCIICString (inContentType, "/javascript") > 0) ||
		(HTTPServerTools::FindASCIICString (inContentType, "application/xml") == 1) ||
		(HTTPServerTools::FindASCIICString (inContentType, "+xml") > 0))
	{
		return MIMETYPE_TEXT;
	}
	else
	{
		return MIMETYPE_BINARY;
	}
}


sLONG GetLocalIPAddresses (XBOX::VectorOfVString& outIPAddresses, bool  bClearFirst)
{
	if (bClearFirst)
		outIPAddresses.clear();

#if WITH_DEPRECATED_IPV4_API
	std::vector<IP4>	ipv4Addresses;
	XBOX::VString		string;

	XBOX::ServerNetTools::GetLocalIPv4Addresses (ipv4Addresses);

	for (std::vector<IP4>::const_iterator it = ipv4Addresses.begin(); it != ipv4Addresses.end(); ++it)
	{
		string.Clear();
		XBOX::ServerNetTools::GetIPAdress ((*it), string);
		outIPAddresses.push_back (string);
	}
#else
	XBOX::VNetAddressList	addrList;
	XBOX::VError		error = addrList.FromLocalInterfaces();

	if (XBOX::VE_OK == error)
	{
		for (XBOX::VNetAddressList::const_iterator it = addrList.begin(); it != addrList.end(); ++it)
			outIPAddresses.push_back (it->GetIP());
	}
#endif

	return (sLONG)outIPAddresses.size();
}


} // namespace HTTPServerTools


//--------------------------------------------------------------------------------------------------


VDebugTimer::VDebugTimer (sLONG8 inStartTime)
: fStartTime (inStartTime)
{
}


VDebugTimer::VDebugTimer()
: fStartTime (0)
{
	Reset();
}


void VDebugTimer::Reset()
{
	XBOX::VSystem::GetProfilingCounter (fStartTime);
}


sLONG8 VDebugTimer::GetElapsedTime() const
{
	sLONG8			endTime = 0;

	XBOX::VSystem::GetProfilingCounter (endTime);
	return (endTime - fStartTime);
}


void VDebugTimer::DebugMsg (const char *inMsg)
{
	XBOX::VString	debugMsg;
	sLONG8			microsec = sLONG8 (GetElapsedTime() * 1000000.0 / XBOX::VSystem::GetProfilingFrequency());

	debugMsg.FromCString (inMsg);
	debugMsg.AppendCString (" - time: ");
	debugMsg.AppendLong8 (microsec);
	debugMsg.AppendCString (" µs\n");

	::DebugMsg (debugMsg);
}


