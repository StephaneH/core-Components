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
#ifndef __HTTP_SERVER_TOOLS_INCLUDED__
#define __HTTP_SERVER_TOOLS_INCLUDED__

#include "HTTPPublicTypes.h"


		/**
		*	Some useful constants strings
		*/
extern const XBOX::VString STRING_EMPTY;
extern const XBOX::VString STRING_STAR;

		/**
		*	Some common HTTP Request headers
		*/
extern const XBOX::VString STRING_HEADER_ACCEPT;
extern const XBOX::VString STRING_HEADER_ACCEPT_CHARSET;
extern const XBOX::VString STRING_HEADER_ACCEPT_ENCODING;
extern const XBOX::VString STRING_HEADER_ACCEPT_LANGUAGE;
extern const XBOX::VString STRING_HEADER_AUTHORIZATION;
extern const XBOX::VString STRING_HEADER_CONTENT_DISPOSITION;
extern const XBOX::VString STRING_HEADER_COOKIE;
extern const XBOX::VString STRING_HEADER_EXPECT;
extern const XBOX::VString STRING_HEADER_FROM;
extern const XBOX::VString STRING_HEADER_HOST;
extern const XBOX::VString STRING_HEADER_IF_MATCH;
extern const XBOX::VString STRING_HEADER_IF_MODIFIED_SINCE;
extern const XBOX::VString STRING_HEADER_IF_NONE_MATCH;
extern const XBOX::VString STRING_HEADER_IF_RANGE;
extern const XBOX::VString STRING_HEADER_IF_UNMODIFIED_SINCE;
extern const XBOX::VString STRING_HEADER_KEEP_ALIVE;
extern const XBOX::VString STRING_HEADER_MAX_FORWARDS;
extern const XBOX::VString STRING_HEADER_PROXY_AUTHORIZATION;
extern const XBOX::VString STRING_HEADER_RANGE;
extern const XBOX::VString STRING_HEADER_REFERER;
extern const XBOX::VString STRING_HEADER_TE;							// RFC 2616 - Section 14.39
extern const XBOX::VString STRING_HEADER_TRANSFER_ENCODING;
extern const XBOX::VString STRING_HEADER_USER_AGENT;

		/**
		*	Some common HTTP Response headers
		*/
extern const XBOX::VString STRING_HEADER_ACCEPT_RANGES;
extern const XBOX::VString STRING_HEADER_AGE;
extern const XBOX::VString STRING_HEADER_ALLOW;
extern const XBOX::VString STRING_HEADER_CACHE_CONTROL;
extern const XBOX::VString STRING_HEADER_CONNECTION;
extern const XBOX::VString STRING_HEADER_DATE;
extern const XBOX::VString STRING_HEADER_ETAG;
extern const XBOX::VString STRING_HEADER_CONTENT_ENCODING;
extern const XBOX::VString STRING_HEADER_CONTENT_LANGUAGE;
extern const XBOX::VString STRING_HEADER_CONTENT_LENGTH;
extern const XBOX::VString STRING_HEADER_CONTENT_LOCATION;
extern const XBOX::VString STRING_HEADER_CONTENT_MD5;
extern const XBOX::VString STRING_HEADER_CONTENT_RANGE;
extern const XBOX::VString STRING_HEADER_CONTENT_TYPE;
extern const XBOX::VString STRING_HEADER_EXPIRES;
extern const XBOX::VString STRING_HEADER_LAST_MODIFIED;
extern const XBOX::VString STRING_HEADER_LOCATION;
extern const XBOX::VString STRING_HEADER_PRAGMA;
extern const XBOX::VString STRING_HEADER_PROXY_AUTHENTICATE;
extern const XBOX::VString STRING_HEADER_RETRY_AFTER;
extern const XBOX::VString STRING_HEADER_SERVER;
extern const XBOX::VString STRING_HEADER_SET_COOKIE;
extern const XBOX::VString STRING_HEADER_STATUS;
extern const XBOX::VString STRING_HEADER_VARY;
extern const XBOX::VString STRING_HEADER_WWW_AUTHENTICATE;
extern const XBOX::VString STRING_HEADER_X_STATUS;
extern const XBOX::VString STRING_HEADER_X_POWERED_BY;
extern const XBOX::VString STRING_HEADER_X_VERSION;

		/**
		*	Some custom HTTP headers
		*/
extern const XBOX::VString STRING_HEADER_X_WA_PATTERN;

		/**
		*	Some common HTTP header values
		*/
extern const XBOX::VString STRING_HEADER_VALUE_CHUNKED;
extern const XBOX::VString STRING_HEADER_VALUE_CLOSE;
extern const XBOX::VString STRING_HEADER_VALUE_COMPRESS;
extern const XBOX::VString STRING_HEADER_VALUE_DEFLATE;
extern const XBOX::VString STRING_HEADER_VALUE_GZIP;
extern const XBOX::VString STRING_HEADER_VALUE_KEEP_ALIVE;
extern const XBOX::VString STRING_HEADER_VALUE_MAX_AGE;
extern const XBOX::VString STRING_HEADER_VALUE_NONE;
extern const XBOX::VString STRING_HEADER_VALUE_NO_CACHE;
extern const XBOX::VString STRING_HEADER_VALUE_X_COMPRESS;
extern const XBOX::VString STRING_HEADER_VALUE_X_GZIP;
extern const XBOX::VString STRING_HEADER_VALUE_IDENTITY;

		/**
		*	Some common HTTP Content-Types
		*/
extern const XBOX::VString STRING_CONTENT_TYPE_HTML;
extern const XBOX::VString STRING_CONTENT_TYPE_JSON;
extern const XBOX::VString STRING_CONTENT_TYPE_MESSAGE;
extern const XBOX::VString STRING_CONTENT_TYPE_TEXT;
extern const XBOX::VString STRING_CONTENT_TYPE_XML;
extern const XBOX::VString STRING_CONTENT_TYPE_BINARY;

		/**
		*	Some common HTTP Authentication schemes
		*/
extern const XBOX::VString STRING_AUTHENTICATION_BASIC;
extern const XBOX::VString STRING_AUTHENTICATION_DIGEST;
extern const XBOX::VString STRING_AUTHENTICATION_KERBEROS;
extern const XBOX::VString STRING_AUTHENTICATION_NEGOTIATE; // used in KERBEROS authentication header
extern const XBOX::VString STRING_AUTHENTICATION_NTLM;

		/**
		*	Some common HTTP Authentication fields
		*/
extern const XBOX::VString STRING_AUTH_FIELD_REALM;
extern const XBOX::VString STRING_AUTH_FIELD_NONCE;
extern const XBOX::VString STRING_AUTH_FIELD_OPAQUE;
extern const XBOX::VString STRING_AUTH_FIELD_CNONCE;
extern const XBOX::VString STRING_AUTH_FIELD_QOP;
extern const XBOX::VString STRING_AUTH_FIELD_NC;	// Nonce Count
extern const XBOX::VString STRING_AUTH_FIELD_ALGORITHM;
extern const XBOX::VString STRING_AUTH_FIELD_RESPONSE;
extern const XBOX::VString STRING_AUTH_FIELD_USERNAME;
extern const XBOX::VString STRING_AUTH_FIELD_URI;

		/**
		*	HTTP Method Names
		*/
extern const XBOX::VString STRING_HTTP_METHOD_GET;
extern const XBOX::VString STRING_HTTP_METHOD_HEAD;
extern const XBOX::VString STRING_HTTP_METHOD_POST;
extern const XBOX::VString STRING_HTTP_METHOD_PUT;
extern const XBOX::VString STRING_HTTP_METHOD_DELETE;
extern const XBOX::VString STRING_HTTP_METHOD_TRACE;
extern const XBOX::VString STRING_HTTP_METHOD_OPTIONS;

		/**
		*	Default Cert & Key file names
		*/
extern const XBOX::VString STRING_CERT_FILE_NAME;
extern const XBOX::VString STRING_KEY_FILE_NAME;

		/**
		*	Common Error Strings
		*/
extern const XBOX::VString STRING_ERROR_INVALID_PARAMETER;
extern const XBOX::VString STRING_ERROR_INVALID_REQUEST_HANDLER;
extern const XBOX::VString STRING_ERROR_ALLOCATION_FAILED;
extern const XBOX::VString STRING_ERROR_FAILED_TO_OPEN_STREAM;
extern const XBOX::VString STRING_ERROR_FILE_LIMIT_REACHED;
extern const XBOX::VString STRING_ERROR_PARAMETER_NAME;

extern const XBOX::VTime TIME_EMPTY_DATE;

#if WITH_DEPRECATED_IPV4_API
extern const IP4 /*done*/		DEFAULT_LISTENING_ADDRESS;
#else
extern const XBOX::VString	DEFAULT_LISTENING_ADDRESS;
#endif
extern const PortNumber	DEFAULT_LISTENING_PORT;
extern const PortNumber	DEFAULT_LISTENING_SSL_PORT;


namespace HTTPServerTools
{
		/**
		*   @function EqualASCIIxString 
		*	@brief Fast comparision function
		*	@param isCaseSensitive set if the function should work in case sensitive mode or not
		*	@discussion function returns true when both strings are equal
		*/
bool	EqualASCIIVString (const XBOX::VString& inString1, const XBOX::VString& inString2, bool isCaseSensitive = false);
bool	EqualASCIICString (const XBOX::VString& inString1, const char *const inCString2, bool isCaseSensitive = false);

		/**
		*   @function FindASCIIxString
		*	@brief Fast Find function using the KMP alogorithm (http://fr.wikipedia.org/wiki/Algorithme_de_Knuth-Morris-Pratt)
		*	@param isCaseSensitive set if the function should work in case sensitive mode or not
		*	@discussion function returns the found pattern 1-based position in string or 0 (ZERO) if not found
		*/
sLONG	FindASCIIVString (const XBOX::VString& inText, const XBOX::VString& inPattern, bool isCaseSensitive = false);
sLONG	FindASCIICString (const XBOX::VString& inText, const char *inPattern, bool isCaseSensitive = false);

		/**
		*   @function BeginsWithASCIIxString
		*/
bool	BeginsWithASCIIVString (const XBOX::VString& inText, const XBOX::VString& inPattern, bool isCaseSensitive = false);
bool	BeginsWithASCIICString (const XBOX::VString& inText, const char *inPattern, bool isCaseSensitive = false);

		/**
		*   @function EndsWithASCIIxString
		*/
bool	EndsWithASCIIVString (const XBOX::VString& inText, const XBOX::VString& inPattern, bool isCaseSensitive = false);
bool	EndsWithASCIICString (const XBOX::VString& inText, const char *inPattern, bool isCaseSensitive = false);

		/**
		*   @function TrimUniChar
		*	@brief Works like the VString::TrimeSpaces() except we can define the char to trim.
		*/
void	TrimUniChar (XBOX::VString& ioString, const UniChar inCharToTrim);

		/**
		 *	@function FastGetLongFromString 
		 *	@brief VString::GetLong() equivalent and faster function
		 */
sLONG	GetLongFromString (const XBOX::VString& inString);

		/**
		*   @function CopyUniString
		*	@brief Fast string copy function
		*/
void	CopyUniString (UniChar* const target, const UniChar* const src);

		/**
		*   @function GetSubString
		*	@brief Fast string copy function
		*/
void	GetSubString (const XBOX::VString& inString, sLONG inFirst, sLONG inLast, XBOX::VString& outString);

		/**
		*   @function LowerCaseUniASCIIString
		*	@brief Fast lowercase function for Uni ASCII strings (does NOT handle extended chars)
		*/
void	LowerCaseUniASCIIString (UniChar* const toLowerCase);

		/**
		*   @function CompressStream
		*	@brief Compress the stream content using the two compression mode supported by CZipComponent gzip or deflate
		*	@discussion the input/output Stream is not intended to be already opened
		*/
XBOX::VError	CompressStream (XBOX::VStream& ioStream, HTTPCompressionMethod inMethod);

		/**
		*   @function DecompressStream
		*	@brief Decompress the stream content using the two compression mode supported by CZipComponent gzip or deflate
		*	@discussion the input/output Stream is not intended to be already opened
		*/
XBOX::VError	DecompressStream (XBOX::VStream& ioStream);

		/**
		*   @function Base64Encode
		*	@brief Encode string to Base64
		*/
XBOX::VError	Base64Encode (XBOX::VString& ioString);

		/**
		*   @function Base64Decode
		*	@brief Decode string from Base64
		*/
XBOX::VError	Base64Decode (XBOX::VString& ioString);

		/**
		 *	@function GetHTTPHeaderName
		 *	@brief Retrieve common header name from header code (see HTTPCommonHeaderCode HTTPServerTypes.h)
		 */
const XBOX::VString&	GetHTTPHeaderName (const HTTPCommonHeaderCode inHeaderCode);

		/**
		 *	@function GetHTTPHeaderValue
		 *	@brief Retrieve common header value from header code (see HTTPCommonValueCode HTTPServerTypes.h)
		 */
const XBOX::VString&	GetHTTPHeaderValue (const HTTPCommonValueCode inValueCode);

		/**
		 *	@function GetHTTPContentType
		 *	@brief Retrieve common content-type value from header code (see HTTPCommonContentType HTTPServerTypes.h)
		 */
const XBOX::VString&	GetHTTPContentType (const HTTPCommonContentType inCode);


		/**
		 *	@function MakeIPv4AddressString
		 *	@brief Make an IPv4 address string from an IPv4 address
		 */
void	MakeIPv4AddressString (IP4 inIPv4, XBOX::VString& outIPv4String);

		/**
		 *	@function GetIPv4FromString
		 */
void	GetIPv4FromString (const XBOX::VString& inIPv4String, IP4& outIPv4);

		/**
		 *	@function MakeHostString
		 *	@brief Make an host string such as "127.0.0.1:8080"
		 */
#if WITH_DEPRECATED_IPV4_API
void	MakeHostString (IP4 /*done*/ inIPv4, PortNumber inPort, XBOX::VString& outHostString);
#endif
	
void	MakeHostString (const XBOX::VString& inHost, PortNumber inPort, XBOX::VString& outHostString);

		/**
		 *	@function ParseHostString
		 */
#if WITH_DEPRECATED_IPV4_API	
void	ParseHostString (const XBOX::VString& inHostString, IP4& /*done*/ outIPv4, PortNumber& outPort);
#endif
	
void	ParseHostString (const XBOX::VString& inHostString, XBOX::VString& outIPString, PortNumber& outPort);

		/**
		 *	@function GetLogFormatName
		 */
void	GetLogFormatName (const EHTTPServerLogFormat inLogFormat, XBOX::VString& outLogFormatName);

		/**
		 *	@function GetLogFormatFromName
		 */
EHTTPServerLogFormat	GetLogFormatFromName (const XBOX::VString& inLogFormatName);

		/**
		 *	@function GetLogTokenName
		 */
void	GetLogTokenName (const EHTTPServerLogToken inToken, XBOX::VString& outTokenName);

		/**
		 *	@function GetLogTokenName
		 */
EHTTPServerLogToken	GetLogTokenFromName (const XBOX::VString& inTokenName);

		/**
		 *	@function GetDefaultLogTokenList
		 *	@brief Retrieve default tokens according to Log Format
		 */
void	GetDefaultLogTokenList (const EHTTPServerLogFormat inLogFormat, VectorOfLogToken& outLogTokens);

		/**
		 *	@function GetLogTokenNamesList
		 *	@brief Get tokens names
		 */
void	GetLogTokenNamesList (const VectorOfLogToken& inLogTokens, XBOX::VString& outTokenNames, const UniChar inSeparator = CHAR_SPACE);

		/**
		 *	@function SendValueBagResponse
		 *	@brief Helper fonction to send an XBOX::VValueBag using JSON or XML format according to URL (handle pretty format: pretty=yes/no)
		 *	Sample URL: /myVValueBagResult?format=json&pretty=yes
		 */
XBOX::VError	SendValueBagResponse (IHTTPResponse& ioResponse, const XBOX::VValueBag& inBag, const XBOX::VString& inBagName);

		/**
		 *	@function GetAuthenticationMethodFromName
		 */
HTTPAuthenticationMethod	GetAuthenticationMethodFromName (const XBOX::VString& inAuthenticationMethodName);

		/**
		 *	@function GetAuthenticationMethodName
		 */
void	GetAuthenticationMethodName (const HTTPAuthenticationMethod inAuthenticationMethod, XBOX::VString& outAuthenticationMethodName);

		/**
		 *	@function MakeRFC822GMTDateString
		 */
void	MakeRFC822GMTDateString (const sLONG inMode, XBOX::VString& outDateString,bool withHeaderName = false, sLONG inTimeout = 5 * 60L);

		/**
		 *	@function MakeIPv4String
		 */
void	MakeIPv4String (const uLONG inIPv4, XBOX::VString& outString);

		/**
		 *	@function ExtractFieldNameValue
		 *	Extract Name and Value from an HTTP Field
		 *	Example Keep-Alive: maxCount=100, timeout=15: will extract {{maxCount;100);{timeout;15}} values pairs
		 */
bool	ExtractFieldNameValue (const XBOX::VString& inString, XBOX::VString& outName, XBOX::VString& outValue);

		/**
		 *	@function GetFileInfos
		 *	Retrieve File Informations such as modification time and/or file size
		 */
XBOX::VError	GetFileInfos (const XBOX::VFilePath& inFilePath, XBOX::VTime *outModificationTime = NULL, sLONG8 *outFileSize = NULL);

		/**
		 *	@function ExtractContentTypeAndCharset
		 *	Retrieve Content-Type & CharSet from Content-Type header value
		 *	Example Content-Type: text/plain; charset="UTF-8"
		 */
void	ExtractContentTypeAndCharset (const XBOX::VString& inString, XBOX::VString& outContentType, XBOX::CharSet& outCharSet);

		/**
		 *	@function GetMimeTypeKind
		 *	Try to determine mime-type kind (may be MIMETYPE_BINARY, MIMETYPE_TEXT or MIMETYPE_IMAGE)
		 */
MimeTypeKind GetMimeTypeKind (const XBOX::VString& inContentType);

		/**
		 *	@function GetLocalIPAddresses
		 */
sLONG GetLocalIPAddresses (XBOX::VectorOfVString& outIPAddresses, bool  bClearFirst = false);

}


//--------------------------------------------------------------------------------------------------


		/**
		 *	@function FindValueInVector
		 */
template <typename T>
bool FindValueInVector (const std::vector<T>& inVector, const T& inValue)
{
	if (std::find (inVector.begin(), inVector.end(), inValue) != inVector.end())
		return true;
	
	return false;
}


		/**
		 *	@function AppendUniqueValueToVector
		 */
template <typename T>
bool AppendUniqueValueToVector (std::vector<T>& inVector, const T& inValue)
{
	if (std::find (inVector.begin(), inVector.end(), inValue) == inVector.end())
	{
		inVector.push_back (inValue);
		return true;
	}
	
	return false;
}


#if 0
		/**
		 *	@function RemoveValueFromVector
		 */
template <typename T>
bool RemoveValueFromVector (std::vector<T>& inVector, const T& inValue)
{
	std::vector<T>::iterator it = std::find (inVector.begin(), inVector.end(), inValue);

	if (it != inVector.end())
	{
		inVector.erase (it);
		return true;
	}
	
	return false;
}
#endif


//--------------------------------------------------------------------------------------------------


		/**
		 *	@function EqualVStringFunctor
		 *	@brief Functor designed to be used with a single XBOX::VString container such as std::vector <XBOX::VString> or std::list<XBOX::VString>
		 */
struct EqualVStringFunctor
{
	EqualVStringFunctor (const XBOX::VString& inString)
	: fString (inString)
	{
	}

	bool operator() (const XBOX::VString& inString) const
	{
		return HTTPServerTools::EqualASCIIVString (fString, inString, false);
	}

private:
	const XBOX::VString&	fString;
};


		/**
		 *	@function FindVStringFunctor
		 *	@brief Functor designed to be used with a single XBOX::VString container such as std::vector <XBOX::VString> or std::list<XBOX::VString>
		 */
struct FindVStringFunctor
{
	FindVStringFunctor (const XBOX::VString& inString)
	: fString (inString)
	{
	}

	bool operator() (const XBOX::VString&	inString) const
	{
		return (HTTPServerTools::FindASCIIVString (inString, fString, false) > 0);
	}

private:
	const XBOX::VString&	fString;
};


		/**
		 *	@function EqualFirstVStringFunctor
		 *	@brief Functor designed to be used with a multiple XBOX::VString container such as std::map <XBOX::VString, XBOX::VString> or std::multimap<XBOX::VString, XBOX::VString>
		 */
template <typename T>
struct EqualFirstVStringFunctor
{
	EqualFirstVStringFunctor (const XBOX::VString& inString)
		: fString (inString)
	{
	}

	bool operator() (const std::pair<XBOX::VString, T>& inPair) const
	{
		return HTTPServerTools::EqualASCIIVString (fString, inPair.first, false);
	}

private:
	const XBOX::VString&	fString;
};


		/**
		 *	@function FindFirstVStringFunctor
		 *	@brief Functor designed to be used with a multiple XBOX::VString container such as std::map <XBOX::VString, XBOX::VString> or std::multimap<XBOX::VString, XBOX::VString>
		 */
template <typename T>
struct FindFirstVStringFunctor
{
	FindFirstVStringFunctor (const XBOX::VString& inString)
		: fString (inString)
	{
	}

	bool operator() (const std::pair<XBOX::VString, T>& inPair) const
	{
		return (HTTPServerTools::FindASCIIVString (fString, inPair.first, false) > 0);
	}

private:
	const XBOX::VString&	fString;
};


		/**
		 *	@function EqualSecondVStringFunctor
		 *	@brief Functor designed to be used with a multiple XBOX::VString container such as std::map <XBOX::VString, XBOX::VString> or std::multimap<XBOX::VString, XBOX::VString>
		 */
template <typename T>
struct EqualSecondVStringFunctor
{
	EqualSecondVStringFunctor (const XBOX::VString& inString)
		: fString (inString)
	{
	}

	bool operator() (const std::pair<T, XBOX::VString>& inPair) const
	{
		return HTTPServerTools::EqualASCIIVString (fString, inPair.second, false);
	}

private:
	const XBOX::VString&	fString;
};


		/**
		 *	@function FindSecondVStringFunctor
		 *	@brief Functor designed to be used with a multiple XBOX::VString container such as std::map <XBOX::VString, XBOX::VString> or std::multimap<XBOX::VString, XBOX::VString>
		 */
template <typename T>
struct FindSecondVStringFunctor
{
	FindSecondVStringFunctor (const XBOX::VString& inString)
	: fString (inString)
	{
	}

	bool operator() (const std::pair<T, XBOX::VString>& inPair) const
	{
		return (HTTPServerTools::FindASCIIVString (fString, inPair.second, false) > 0);
	}

private:
	const XBOX::VString&	fString;
};


struct FindMatchingRequestHandlerFunctor
{
	FindMatchingRequestHandlerFunctor (const IHTTPRequestHandler *inRequestHandler)
	: fRequestHandler (inRequestHandler)
	{
	}

	bool operator() (const IHTTPRequestHandler *inRequestHandler) const
	{
		assert (NULL != inRequestHandler);

		XBOX::VectorOfVString			patterns;
		XBOX::VectorOfVString			srcPatterns;
		XBOX::VectorOfVString::iterator	iter;

		inRequestHandler->GetPatterns (&patterns);
		fRequestHandler->GetPatterns (&srcPatterns);

		iter = srcPatterns.begin();
		while (iter != srcPatterns.end())
		{
			if (std::find_if (patterns.begin(), patterns.end(), EqualVStringFunctor ((*iter))) != patterns.end())
				return true;
			++iter;
		}

		return false;
	}

private:
	const IHTTPRequestHandler *	fRequestHandler;
};


template <typename T>
struct EqualVRegexMatcherFunctor
{
	EqualVRegexMatcherFunctor (const XBOX::VString& inPattern)
	: fPattern (inPattern)
	{
	}

	bool operator() (const std::pair<XBOX::VRegexMatcher *, T>& inValueType) const
	{
		return inValueType.first->IsSamePattern (fPattern);
	}

private:
	const XBOX::VString&	fPattern;
};


inline
bool MatchRegex (XBOX::VRegexMatcher *inRegexMatcher, const XBOX::VString& inString, bool inContinueSearching = true)
{
	if (NULL == inRegexMatcher)
		return false;

	XBOX::VError error = XBOX::VE_OK;
	return (inRegexMatcher->Find (inString, 1, inContinueSearching, &error) && (XBOX::VE_OK == error));
}


template <typename T>
struct MatchVRegexMatcherFunctor
{
	MatchVRegexMatcherFunctor (const XBOX::VString& inString, bool inContinueSearching = true)
	: fString (inString)
	, fContinueSearching (inContinueSearching)
	{
	}

	bool operator() (const std::pair<XBOX::VRegexMatcher *, T>& inValueType) const
	{
		return MatchRegex (inValueType.first, fString, fContinueSearching);
	}

private:
	const XBOX::VString&	fString;
	bool					fContinueSearching;
};


//--------------------------------------------------------------------------------------------------


inline
bool IsVTimeValid (const XBOX::VTime& inValue)
{
	XBOX::CompareResult result = inValue.CompareTo (TIME_EMPTY_DATE);
	return ((XBOX::CR_EQUAL != result) && (XBOX::CR_UNRELEVANT != result));
}


//--------------------------------------------------------------------------------------------------


class VDebugTimer : public XBOX::VObject
{
public:
									VDebugTimer();
									VDebugTimer (sLONG8 inStartTime);
	virtual							~VDebugTimer() {};

	void							Reset();
	sLONG8							GetStartTime() const { return fStartTime; }
	sLONG8							GetElapsedTime() const;
	void							DebugMsg (const char *inMsg);

private:
	sLONG8							fStartTime;
};


#endif	//__HTTP_SERVER_TOOLS_INCLUDED__