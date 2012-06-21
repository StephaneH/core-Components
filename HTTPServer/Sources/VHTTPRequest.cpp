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
#include "HTTPNetworking.h"
#include "VVirtualHost.h"


//--------------------------------------------------------------------------------------------------


XBOX::VSize VHTTPRequest::fMaxIncomingDataSize = XBOX::MaxLongInt;


VHTTPRequest::VHTTPRequest()
: fRequestMethod (HTTP_UNKNOWN)
, fRequestLine()
, fURL()
, fRawURL()
, fURLPath()
, fURLQuery()
, fHTTPVersion (VERSION_UNKNOWN)
, fHost()
, fAuthenticationInfos (NULL)
, fParsingState (PS_Undefined)
, fParsingError (XBOX::VE_OK)
, fHTMLForm (NULL)
, fEndPointIPv4 (0)
, fEndPointPort (0)
{
}


VHTTPRequest::~VHTTPRequest()
{
	if (NULL != fAuthenticationInfos)
	{
		delete fAuthenticationInfos;
		fAuthenticationInfos = NULL;
	}

	if (NULL != fHTMLForm)
	{
		delete fHTMLForm;
		fHTMLForm = NULL;
	}
}


void VHTTPRequest::Reset()
{
	inherited::Clear();

	fRequestMethod = HTTP_UNKNOWN;
	fHTTPVersion = VERSION_UNKNOWN;
	fParsingState = PS_Undefined;
	fParsingError = XBOX::VE_OK;
	fRequestLine.Clear();
	fURL.Clear();
	fRawURL.Clear();
	fURLPath.Clear();
	fURLQuery.Clear();
	fHost.Clear();

	if (NULL != fAuthenticationInfos)
	{
		delete fAuthenticationInfos;
		fAuthenticationInfos = NULL;
	}

	if (NULL != fHTMLForm)
	{
		delete fHTMLForm;
		fHTMLForm = NULL;
	}
}


/* private */
VHTTPRequest& VHTTPRequest::operator = (const VHTTPRequest& inHTTPRequest)
{
	if (&inHTTPRequest != this)
	{
		fRequestMethod = inHTTPRequest.fRequestMethod;
		fRequestLine.FromString (inHTTPRequest.fRequestLine);
		fURL.FromString (inHTTPRequest.fURL);
		fRawURL.FromString (inHTTPRequest.fRawURL);
		fURLPath.FromString (inHTTPRequest.fURLPath);
		fURLQuery.FromString (inHTTPRequest.fURLQuery);
		fHost.FromString (inHTTPRequest.fHost);
		fHTTPVersion = inHTTPRequest.fHTTPVersion;
		fAuthenticationInfos = new VAuthenticationInfos (*inHTTPRequest.fAuthenticationInfos);
		fParsingState = inHTTPRequest.fParsingState;
		fParsingError = inHTTPRequest.fParsingError;
	}

	return *this;
}


XBOX::VError VHTTPRequest::ReadFromEndPoint (XBOX::VEndPoint& inEndPoint, uLONG inTimeout)
{
	// Read Request-Line and extract Method, URL and HTTP version
#define	MAX_BUFFER_LENGTH	8192

	sLONG			bufferSize = MAX_BUFFER_LENGTH;
	char *			buffer = (char *)XBOX::vMalloc (bufferSize, 0);

	if (NULL == buffer)
		return XBOX::VE_MEMORY_FULL;

	XBOX::VError	endPointError = XBOX::VE_OK;
	sLONG			bufferOffset = 0;
	sLONG			unreadBytes = 0;
	sLONG			lineLen = 0;
	bool			hostHeaderFound = false;
	XBOX::VString	header;
	XBOX::VString	value;
	char *			startLinePtr = NULL;
	char *			endLinePtr = NULL;
	char *			endHeaderPtr = NULL;
	const sLONG		endLineSize = sizeof (HTTP_CRLF) - 1;
	const sLONG		endHeaderSize = sizeof (HTTP_CRLFCRLF) - 1;
	sLONG			contentLength = 0;
	bool			contentLengthFound = false;
	void *			bodyContentBuffer = NULL;
	XBOX::VSize		bodyContentSize = 0;
	const sLONG		READ_TIMEOUT = (inTimeout != 0) ? inTimeout : 10000;
	bool			stopReadingSocket = false;
	bool			wasCRLF = false;
	bool			requestEntityTooLarge = false;

	fParsingState = PS_Undefined;
	fRequestLine.Clear();

#if WITH_DEPRECATED_IPV4_API
	fEndPointIPv4 = dynamic_cast<VTCPEndPoint &>(inEndPoint).GetIPv4HostOrder();
	fEndPointPort = dynamic_cast<VTCPEndPoint &>(inEndPoint).GetPort();
#elif DEPRECATED_IPV4_API_SHOULD_NOT_COMPILE
	#error NEED AN IP V6 UPDATE
#endif
	
	XBOX::StErrorContextInstaller errorContext (VE_SRVR_TOO_MANY_SOCKETS_FOR_SELECT_IO,
												VE_SRVR_READ_FAILED,
												VE_SRVR_CONNECTION_FAILED,
												VE_SRVR_CONNECTION_BROKEN,
												VE_SOCK_PEER_OVER,
												VE_SRVR_NOTHING_TO_READ,
												XBOX::VE_OK); // YT 07-Dec-2009 - ACI0063626
#if LOG_IN_CONSOLE
	VDebugTimer readExactlyTimer;
#endif

	// Read the first 5 bytes (minimalist request is : "GET /"
	unreadBytes = bufferOffset = 5;
	endPointError = inEndPoint.ReadExactly (buffer, unreadBytes, READ_TIMEOUT);
	
#if LOG_IN_CONSOLE
	readExactlyTimer.DebugMsg ("***VHTTPRequest::ReadFromEndPoint\n\tVTCPEndPoint::ReadExactly()");
#endif

	if (XBOX::VE_OK == endPointError)
		fParsingState = PS_ReadingRequestLine;

	while ((XBOX::VE_OK == endPointError) && !stopReadingSocket)
	{
		if (0 == unreadBytes)
			bufferOffset = 0;

		bufferSize = MAX_BUFFER_LENGTH - bufferOffset;
#if VERSIONDEBUG
		if (bufferSize <= 0)
			assert (false);
#endif

#if LOG_IN_CONSOLE
		VDebugTimer readTimer;
#endif
		endPointError = inEndPoint.Read (buffer + bufferOffset, (uLONG *)&bufferSize);

		//jmo - tentatives de fix pour ne plus lire en boucle une socket sur laquelle le client a fait un shutdown...
		if(VE_OK==endPointError && 0==bufferSize)
		{
			//jmo - on ne devrait plus passer par là.
			xbox_assert(false);
			stopReadingSocket=true;
		}
		
		//jmo - erreur qu'on recupere sur un shutdown cote client
		if(VE_SRVR_NOTHING_TO_READ==endPointError)
			stopReadingSocket=true;
		
#if LOG_IN_CONSOLE
		readTimer.DebugMsg ("\tVTCPEndPoint::Read()");
#endif

#if LOG_IN_CONSOLE
		if ((XBOX::VE_OK != endPointError) && (VE_SRVR_RESOURCE_TEMPORARILY_UNAVAILABLE != endPointError))
			::DebugMsg ("* VTCPEndPoint::Read() returns error %d\n", ERRCODE_FROM_VERROR(endPointError));
#endif
		unreadBytes = (bufferSize + bufferOffset);
#if VERSIONDEBUG
		if (unreadBytes > MAX_BUFFER_LENGTH)
			assert(false);
#endif

		bufferOffset = 0;

		while ((unreadBytes > 0) && (XBOX::VE_OK == fParsingError))
		{
			if (fParsingState <= PS_ReadingHeaders)
			{
				startLinePtr = buffer + bufferOffset;
				/*
					YT 14-Jun-2011 - TestCase HttpMsg_MsgType02 (see RFC2616 - Section 4.1)
					In the interest of robustness, servers SHOULD ignore any empty
					line(s) received where a Request-Line is expected. In other words, if
					the server is reading the protocol stream at the beginning of a
					message and receives a CRLF first, it should ignore the CRLF.
				*/
				if ((PS_ReadingRequestLine == fParsingState) && (NULL != startLinePtr))
				{
					while ((*startLinePtr == HTTP_CR) || (*startLinePtr == HTTP_LF))
					{
						++startLinePtr;
						++bufferOffset;
						--unreadBytes;
					}
				}
				endLinePtr = strstr (startLinePtr, HTTP_CRLF);

				// YT 14-Jun-2011 - TestCase HttpMsg_MsgHeader03
				// Check if CRLF is immediately followed by SP or HT (aka LWS: allowed in Headers)
				if ((PS_ReadingHeaders == fParsingState) && (NULL != endLinePtr))
				{
					char * crlfPtr = endLinePtr + 2; // Skip CRLF
					// Do we have a LWS ?
					if (NULL != crlfPtr && ((*crlfPtr == HTTP_SP) || (*crlfPtr == HTTP_HT)))
					{
						while (isspace (*crlfPtr))
							++crlfPtr;

						// Relocate the true end of line
						endLinePtr = strstr (crlfPtr, HTTP_CRLF);
					}
				}

				if ((NULL != endLinePtr) && (NULL == endHeaderPtr))
				{
					endHeaderPtr = strstr (startLinePtr, HTTP_CRLFCRLF);

					if ((NULL == endHeaderPtr) && wasCRLF && (*startLinePtr == HTTP_CR) && (*(startLinePtr+1) == HTTP_LF))
						endHeaderPtr = startLinePtr;
				}
			}

			/* Start to parse the Status-Line */
			switch (fParsingState)
			{
				case PS_ReadingRequestLine:
				{
					if (NULL != startLinePtr)
					{
						XBOX::VSize chunkSize = (NULL != endLinePtr) ? XBOX::VSize (endLinePtr - startLinePtr) : XBOX::VSize (unreadBytes);
						fRequestLine.AppendBlock (startLinePtr, chunkSize, XBOX::VTC_UTF_8);
						
						/* We cannot find the end of requestLine (it's bigger than buffer or we cannot read enough data from VTCPEndPoint) */
						if (NULL == endLinePtr)
						{
							bufferOffset = unreadBytes = 0;
						}
						else
						{
#if CHECK_REQUEST_LINE_VALIDITY // disabled, cause pattern used causes crashes..
							if (HTTPProtocol::IsValidRequestLine (fRequestLine))
#endif
							{
								const UniChar *	lineBuffer = fRequestLine.GetCPointer();
								const sLONG		lineLength = fRequestLine.GetLength();

								fRequestMethod = HTTPProtocol::GetMethodFromRequestLine (lineBuffer, lineLength);
								if (fRequestMethod != HTTP_UNKNOWN)
								{
									if (HTTPProtocol::GetRequestURIFromRequestLine (lineBuffer, lineLength, fRawURL, false))
									{
										fURL.FromString (fRawURL);
										XBOX::VURL::Decode (fURL);
										fHTTPVersion = HTTPProtocol::GetVersionFromRequestLine (lineBuffer, lineLength);
										if (fHTTPVersion == VERSION_UNSUPPORTED)
										{
											fParsingError = VE_HTTP_PROTOCOL_HTTP_VERSION_NOT_SUPPORTED;
											break;
										}

										XBOX::VURL url;
										url.FromString (fURL, false);
										url.GetPath (fURLPath, eURL_POSIX_STYLE, false);
										url.GetQuery (fURLQuery, false);

										fParsingState = PS_ReadingHeaders;
									}
									else
									{
										fParsingError = VHTTPServer::ThrowError (VE_HTTP_PROTOCOL_BAD_REQUEST, CVSTR ("Request Line parsing failed !!"));
										break;
									}
								}
								else
								{
									/*
									YT 02-May-2011 - RFC2616 compliance
									Reference: RFC2616 chapter: 5.1.1 Method
									[...]
									An origin server SHOULD return the status code 405 (Method Not Allowed)
									if the method is known by the origin server but not allowed for the
									requested resource, and 501 (Not Implemented) if the method is
									unrecognized or not implemented by the origin server.
									*/
									fParsingError = VE_HTTP_PROTOCOL_NOT_IMPLEMENTED;
									break;
								}
							}
#if CHECK_REQUEST_LINE_VALIDITY // disabled, cause pattern used causes crashes..
							else
							{
								fParsingError = VHTTPServer::ThrowError (VE_HTTP_PROTOCOL_BAD_REQUEST, CVSTR ("Request Line validation failed !!"));
								break;
							}
#endif
						}
					}
					else
					{
						fParsingError = VHTTPServer::ThrowError (VE_HTTP_PROTOCOL_BAD_REQUEST, CVSTR ("Empty Request Line !!"));
						break;
					}
					break;
				}
				case PS_ReadingHeaders:
				{
					if (NULL != endLinePtr)
					{
						if (startLinePtr != endHeaderPtr)
						{
							char *colonPtr = strchr (startLinePtr, HTTP_COLON);

							if ((NULL != colonPtr) && (colonPtr < endLinePtr))
							{
								header.FromBlock (startLinePtr, colonPtr - startLinePtr, XBOX::VTC_UTF_8);
								if (!header.IsEmpty())
								{
									++colonPtr; // skip colon
									while ((colonPtr < endLinePtr) && isspace (*colonPtr))
										++colonPtr;
									value.FromBlock (colonPtr, (endLinePtr - colonPtr), XBOX::VTC_UTF_8);

									GetHeaders().SetHeaderValue (header, value, false);

									if (!hostHeaderFound && HTTPServerTools::EqualASCIIVString (header, STRING_HEADER_HOST))
									{
										fHost.FromString(value);
										hostHeaderFound = true;
									}
								}
							}
						}

						if ((NULL != endHeaderPtr) && (endLinePtr == endHeaderPtr))
						{
							/*
								Do not read socket anymore when header is complete for GET, HEAD, TRACE & OPTIONS requests. Body is not allowed in theses cases.
							*/
							 
							if ((HTTP_GET == fRequestMethod) || (HTTP_HEAD == fRequestMethod) || (HTTP_TRACE == fRequestMethod) || (HTTP_OPTIONS == fRequestMethod))
							{
								fParsingState = PS_ParsingFinished;
								stopReadingSocket = true;
							}
							else
							{
								fParsingState = PS_ReadingBody;
								
								XBOX::VString contentLengthString;
								if ((contentLengthFound = GetHeaders().GetHeaderValue (STRING_HEADER_CONTENT_LENGTH, contentLengthString)))
								{
									contentLength = HTTPServerTools::GetLongFromString (contentLengthString);	
								}
							}
						}
					}
					break;
				}

				case PS_ReadingBody:
				{
					if (!requestEntityTooLarge && (NULL == bodyContentBuffer))
					{
						// There's no Content-Length field in header
						if (0 == contentLength)
						{
							bodyContentBuffer = XBOX::vMalloc (bufferSize, 0);
							bodyContentSize = 0;
						}
						// There's one Content-Length, just check it match limits
						else if ((contentLength > 0) && (_AcceptIncomingDataSize (contentLength)))
						{
							bodyContentBuffer = XBOX::vMalloc (contentLength, 0);
							bodyContentSize = 0;
						}
					}

					if ((NULL != bodyContentBuffer) && _AcceptIncomingDataSize (bodyContentSize + unreadBytes))
					{
						sLONG  nbBytesToCopy = unreadBytes;
						if (bodyContentSize + nbBytesToCopy > contentLength)
						{
							void * moreBodyContentBuffer = XBOX::vRealloc (bodyContentBuffer, bodyContentSize + nbBytesToCopy);
							
							if (NULL != moreBodyContentBuffer)
							{
								bodyContentBuffer = moreBodyContentBuffer;
							}
							else
							{
								XBOX::vFree (bodyContentBuffer);
								bodyContentBuffer = NULL;
// 								fParsingError = VE_HTTP_PROTOCOL_REQUEST_ENTITY_TOO_LARGE;
								requestEntityTooLarge = true;
							}
						}

						if (NULL != bodyContentBuffer)
						{
							memcpy ((char *)(bodyContentBuffer) + bodyContentSize, buffer + bufferOffset, unreadBytes);
							bodyContentSize += unreadBytes;
							bufferOffset = unreadBytes = 0;

							if (contentLength > 0 && bodyContentSize == contentLength)
								stopReadingSocket = true;
						}
					}
					else
					{
// 						fParsingError = VE_HTTP_PROTOCOL_REQUEST_ENTITY_TOO_LARGE;
						/*	YT 04-Nov-2001 - ACI0071407 
							Do NOT interrupt reads on TCPEndPoint (it causes a RESET when we answer using it later).
							Just simply read the whole body (without saving it in Request object).
							I'll see later how to stop socket reading properly...
						*/
						requestEntityTooLarge = true;

						bodyContentSize += unreadBytes;
						bufferOffset = unreadBytes = 0;

						if (contentLength > 0 && bodyContentSize == contentLength)
							stopReadingSocket = true;

						if (NULL != bodyContentBuffer)
						{
							XBOX::vFree (bodyContentBuffer);
							bodyContentBuffer = NULL;
						}
					}
					break;
				}

				case PS_ParsingFinished:
					stopReadingSocket = true;
					break;
			}

			if (XBOX::VE_OK != fParsingError)
				break;

			if (NULL != endLinePtr)
			{
				lineLen = (endLinePtr - startLinePtr) + (((endHeaderPtr != NULL) && (endLinePtr == endHeaderPtr)) ? endHeaderSize : endLineSize); // to skip CRLF or CRLFCRLF;
				bufferOffset += lineLen;
				unreadBytes -= lineLen;
				startLinePtr = endLinePtr = NULL;

				wasCRLF = true;

				/* YT 26-Aug-2010 - Optimization: When there's no Content-Length header, no more data in buffer,
									so there's probably no body... In that case do NOT read socket anymore which
									is too penalizing, because VTCPEndPoint::Read() uses a timeout of 100ms (with select)
									to determine if socket is readable...
				*/

				if (PS_ReadingBody == fParsingState)
				{
					if ((fRequestMethod != HTTP_GET) && (fRequestMethod != HTTP_HEAD) && (fRequestMethod != HTTP_TRACE) && (!contentLengthFound)) // YT 03-Oct-2011 - WAK0073323 - Test if Content-Length was found (even if equals 0) - YT 06-Jul-2011 - TestCase MessageLength02 (RFC2616 4.4)
					{
						fParsingError = VE_HTTP_PROTOCOL_LENGTH_REQUIRED; 
						stopReadingSocket = true;
					}
					else if ((contentLengthFound) && (contentLength == 0))
					{
						stopReadingSocket = true;
					}
				}
			}
			else
			{
				if (bufferOffset > 0)
				{
					memmove (buffer, buffer + bufferOffset, unreadBytes);
					buffer[unreadBytes] = 0;
				}
				bufferOffset = unreadBytes;
				break;
			}
		}

		if (XBOX::VE_OK != fParsingError)
			break;
	}

	if (VE_SRVR_RESOURCE_TEMPORARILY_UNAVAILABLE == endPointError)
	{
		assert (false); // Should never happend in Blocking Mode
		endPointError = XBOX::VE_OK;
	}

	if (requestEntityTooLarge)
		fParsingError = VE_HTTP_PROTOCOL_REQUEST_ENTITY_TOO_LARGE;

	if ((XBOX::VE_OK == fParsingError) && (XBOX::VE_OK == endPointError))
	{
		XBOX::VString	headerValue;
		XBOX::CharSet	charSet = XBOX::VTC_UNKNOWN;

		if (NULL != bodyContentBuffer)
		{
			if ((fRequestMethod != HTTP_GET) && (fRequestMethod != HTTP_HEAD) && (fRequestMethod != HTTP_TRACE) && (!contentLengthFound)) // YT 03-Oct-2011 - WAK0073323 - Test if Content-Length was found (even if equals 0) - YT 06-Jul-2011 - TestCase MessageLength02 (RFC2616 4.4)
			{
				fParsingError = VE_HTTP_PROTOCOL_LENGTH_REQUIRED; 
			}
			else
			{
				//assert (bodyContentSize == contentLength);
				GetBody().SetDataPtr (bodyContentBuffer, bodyContentSize);

				// YT 16-Feb-2012 - ACI0075553 - Automatically decompress body (when applicable)
				if (GetHeaders().GetHeaderValue (STRING_HEADER_CONTENT_ENCODING, headerValue))
				{
					HTTPCompressionMethod compressionMethod = HTTPProtocol::NegotiateEncodingMethod (headerValue);
					if ((compressionMethod > COMPRESSION_NONE) && (compressionMethod <= COMPRESSION_X_COMPRESS))
					{
							fParsingError = HTTPServerTools::DecompressStream (GetBody());
					}
					else if (compressionMethod != COMPRESSION_IDENTITY)
					{
						fParsingError = VE_HTTP_PROTOCOL_UNSUPPORTED_MEDIA_TYPE; // YT 06-Jul-2011 - TestCase ContentCoding10 (see RFC2616 Section 14.11)
					}
				}

				/* Set VStream charset according to content-type header other else set default charset to UTF-8 */
				if ((!GetHeaders().GetContentType (headerValue, &charSet)) || (XBOX::VTC_UNKNOWN == charSet))
					charSet = XBOX::VTC_UTF_8;
				GetBody().SetCharSet (charSet);
			}
		}

		fParsingState = PS_ParsingFinished;

		// HTTP/1.1 requests must include Host header
		if ((fHTTPVersion == VERSION_1_1) && fHost.IsEmpty())
		{
			if (!GetHeaders().GetHeaderValue (STRING_HEADER_HOST, fHost))
				fParsingError = VHTTPServer::ThrowError (VE_HTTP_PROTOCOL_BAD_REQUEST, CVSTR ("HTTP/1.1 requests must include Host header"));
		}

		// YT 12-Oct-2009 - ACI0063558 - Request MUST include CRLF+CRLF at the end of header
		if ((XBOX::VE_OK == fParsingError) && (NULL == endHeaderPtr))
			fParsingError = VHTTPServer::ThrowError (VE_HTTP_PROTOCOL_BAD_REQUEST, CVSTR ("Request header is not followed by CRLF+CRLF !!"));

		// YT 08-Jul-2011 - TestCase CharSet01 (RFC2616 14.2)
		if ((XBOX::VE_OK == fParsingError) && (GetHeaders().IsHeaderSet (STRING_HEADER_ACCEPT_CHARSET)))
		{
			XBOX::VString acceptCharsetValues;

			GetHeaders().GetHeaderValue (STRING_HEADER_ACCEPT_CHARSET, acceptCharsetValues);
			XBOX::CharSet charset = HTTPProtocol::NegotiateCharSet (acceptCharsetValues);
			if (XBOX::VTC_UNKNOWN == charset)
				fParsingError = VE_HTTP_PROTOCOL_NOT_ACCEPTABLE;
		}

		// Check if body is compressed and if compression method is supported
		if ((XBOX::VE_OK == fParsingError) && (GetHeaders().IsHeaderSet (STRING_HEADER_ACCEPT_ENCODING)))
		{
			if (GetHeaders().GetHeaderValue (STRING_HEADER_ACCEPT_ENCODING, headerValue))
			{
				HTTPCompressionMethod compressionMethod = HTTPProtocol::NegotiateEncodingMethod (headerValue);
				if ((compressionMethod < COMPRESSION_NONE) || (compressionMethod > COMPRESSION_LAST_SUPPORTED_METHOD))
					fParsingError = VE_HTTP_PROTOCOL_NOT_ACCEPTABLE; // YT 14-Jun-2011 - TestCase Entities_CC03 (see RFC2616 Section 14.3)
			}
		}
	}
	else
	{
		if (NULL != bodyContentBuffer)
			XBOX::vFree (bodyContentBuffer);
	}

#if LOG_IN_CONSOLE
	if (XBOX::VE_OK != endPointError)
		::DebugMsg ("* VHTTPRequest::ReadFromEndPoint() returns error %d\n", ERRCODE_FROM_VERROR (endPointError));
#endif

	XBOX::vFree (buffer);

	return endPointError;

#undef MAX_BUFFER_LENGTH
}


XBOX::VError VHTTPRequest::ReadFromStream (XBOX::VStream& inStream)
{
	return inherited::_ReadFromStream ( inStream,
										CVSTR (""),
										&fRequestLine,
										&fRequestMethod,
										&fHTTPVersion,
										&fHost,
										&fRawURL,
										&fURL,
										&fURLPath,
										&fURLQuery,
										&fParsingState,
										&fParsingError);
}


void VHTTPRequest::GetRequestMethodString (XBOX::VString& outMethodString) const
{
	HTTPProtocol::MakeHTTPMethodString (fRequestMethod, outMethodString);
}


void VHTTPRequest::GetRequestHTTPVersionString (XBOX::VString& outVersionString) const
{
	HTTPProtocol::MakeHTTPVersionString (fHTTPVersion, outVersionString);
}


IAuthenticationInfos *VHTTPRequest::GetAuthenticationInfos()
{
	if (NULL == fAuthenticationInfos)
		fAuthenticationInfos = new VAuthenticationInfos();

	return fAuthenticationInfos;
}


IAuthenticationInfos *VHTTPRequest::GetAuthenticationInfos() const
{
	if (NULL == fAuthenticationInfos)
		fAuthenticationInfos = new VAuthenticationInfos();

	return fAuthenticationInfos;
}


void VHTTPRequest::GetContentTypeHeader (XBOX::VString& outContentType, XBOX::CharSet *outCharSet) const
{
	GetHeaders().GetContentType (outContentType, outCharSet);
}


MimeTypeKind VHTTPRequest::GetContentTypeKind() const
{
	XBOX::VString contentType;

	GetHeaders().GetContentType (contentType, NULL);

	return VMimeTypeManager::GetMimeTypeKind (contentType);
}


bool VHTTPRequest::GetCookies (std::vector<IHTTPCookie *>& outCookies) const
{
	XBOX::VectorOfVString values;

	outCookies.clear();
	if (GetHeaders().GetHeaderValues (HEADER_COOKIE, values))
	{
		for (XBOX::VectorOfVString::const_iterator it = values.begin(); it != values.end(); ++it)
		{
			VHTTPCookie * cookie = new VHTTPCookie (*it);

			if (NULL != cookie)
				outCookies.push_back (dynamic_cast<IHTTPCookie *>(cookie));
		}
	}

	return !outCookies.empty();
}


bool VHTTPRequest::GetCookie (const XBOX::VString& inName, XBOX::VString& outValue) const
{
	return GetHeaders().GetCookie (inName, outValue);
}


bool VHTTPRequest::_ExtractAuthenticationInfos ()
{
	if (NULL != fAuthenticationInfos)
		delete fAuthenticationInfos;

	fAuthenticationInfos = VAuthenticationManager::CreateAuthenticationInfosFromHeader (GetHeaders(), fRequestMethod);

	return (NULL != fAuthenticationInfos);
}


inline
bool VHTTPRequest::_AcceptIncomingDataSize (XBOX::VSize inSize)
{
	if ((0 == fMaxIncomingDataSize) || ((fMaxIncomingDataSize > 0) && (inSize <= fMaxIncomingDataSize)))
		return true;
	
	return false;
}


const IHTMLForm *VHTTPRequest::GetHTMLForm() const
{
	if (NULL == fHTMLForm)
	{
		fHTMLForm = new VHTMLForm();
		fHTMLForm->Load (*this);
	}

	return fHTMLForm;
}
