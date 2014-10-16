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
#ifndef __HTTP_PROTOCOL_INCLUDED__
#define __HTTP_PROTOCOL_INCLUDED__


class HTTPProtocol
{
public:
	static HTTPRequestMethod					GetMethodFromRequestLine (const UniChar *inBuffer, sLONG inBufferLen);
	static HTTPRequestMethod					GetMethodFromName (const XBOX::VString& inName);
	static HTTPVersion							GetVersionFromRequestLine (const UniChar *inBuffer, sLONG inBufferLen);
	static bool									GetRequestURIFromRequestLine (const UniChar *inBuffer, sLONG inBufferLen, XBOX::VString& outURI, bool decodeURI = true);
	static XBOX::VError							ReadRequestLine (const XBOX::VString& inRequestLine, HTTPRequestMethod& outMethod, XBOX::VString& outURI, HTTPVersion& outVersion, bool inCheckValidity = false);
	static void									ParseURL (const XBOX::VString& inRawURL, XBOX::VString& outDecodedURL, XBOX::VString& outURLPath, XBOX::VString& outURLQuery);
	static XBOX::VError							ReadHeaderLine (const XBOX::VString& inLineString, XBOX::VString& outHeaderName, XBOX::VString& outHeaderValue);
	static void									GetStatusCodeExplanation (sLONG inStatusCode, XBOX::VString& outExplanation, bool clearStringFirst = false);
	static bool									IsValidStatusCode (sLONG inStatusCode);

	static void									GetEncodingMethodName (HTTPCompressionMethod inMethod, XBOX::VString& outName);
	static HTTPCompressionMethod				NegotiateEncodingMethod (XBOX::VString& inEncodingHeaderValue);
	static bool									AcceptEncodingMethod (const XBOX::VString& inEncodingHeaderValue, HTTPCompressionMethod inWantedEncodingMethod);

	static void									MakeStatusLine (HTTPVersion inVersion, sLONG inStatusCode, XBOX::VString& outStatusLine);
	static void									MakeRFC822GMTDateString (const sLONG inMode, XBOX::VString& outDateString, bool withHeaderName = false, sLONG inTimeout = 5 * 60L);
	static void									MakeRFC822GMTDateString (const XBOX::VTime& inTime, XBOX::VString& outDateString, bool withHeaderName = false);
	static void									MakeServerString (XBOX::VString& outServerString, bool inSecureMode = false, bool withHeaderName = false);
	static void									MakeErrorResponse (const XBOX::VError inError, XBOX::VStream& outResponse, XBOX::VString *inAdditionnalExplanationString = NULL);
	static void									MakeHTTPMethodString (const HTTPRequestMethod inMethod, XBOX::VString& outString);
	static void									MakeHTTPVersionString (const HTTPVersion inVersion, XBOX::VString& outVersionString);
	static bool									MakeHTTPAuthenticateHeaderValue (const HTTPAuthenticationMethod inMethod, const XBOX::VString& inRealm, const XBOX::VString& inDomain, XBOX::VString& outAuthString);
	static bool									GetAuthenticationType (const HTTPAuthenticationMethod inMethod, XBOX::VString& outAuthString);

	static void									GetBestQualifiedValueUsingQFactor (const XBOX::VString& inFieldValue, XBOX::VString& outValue);

	static XBOX::CharSet						NegotiateCharSet (XBOX::VString& inAcceptCharsetHeaderValue);
	static bool									IsAcceptableCharSet (const XBOX::VString& inAcceptValue, const XBOX::CharSet inCharSet);

	static bool									IsValidRequestLine (const XBOX::VString& inRequestLine);
	static bool									IsAcceptableMediaType (const XBOX::VString& inAcceptValue, const XBOX::VString& inContentType);

	static bool									ParseRangeHeaderValue (const XBOX::VString& inRangeHeader, std::vector< std::pair<XBOX::VIndex, xbox::VIndex> >& outIndexes);
	static void									MakeRangeHeaderValue (XBOX::VIndex inStartIndex, XBOX::VIndex inEndIndex, XBOX::VSize inMaxSize, XBOX::VString& outRangeHeader);
	static bool									AcceptUpdateRangeIndexes (XBOX::VIndex& ioStartIndex, XBOX::VIndex& ioEndIndex, const XBOX::VSize inMaxSize);

private:
	static XBOX::VRefPtr<XBOX::VRegexMatcher>	fRequestRegexMatcher;
	static XBOX::VCriticalSection				fRequestRegexMatcherMutex;
};


#endif // __HTTP_PROTOCOL_INCLUDED__
