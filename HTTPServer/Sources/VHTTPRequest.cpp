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
, fLocalAddress()
, fPeerAddress()
, fIsSSL (false)
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
	fParsingError = XBOX::VE_OK;

	/*
	 *	For Keep-Alive reset previous request's infos except when Server sent 100-Continue.
	 *	In that case keep headers and just prepare to receive message's body
	 */
	if (fParsingState != PS_WaitingForBody)
	{
		inherited::Clear();

		fRequestMethod = HTTP_UNKNOWN;
		fHTTPVersion = VERSION_UNKNOWN;
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
}


XBOX::VError VHTTPRequest::ReadFromEndPoint (XBOX::VTCPEndPoint& inEndPoint, uLONG inTimeout)
{
	// Read Request-Line and extract Method, URL and HTTP version
#define	MAX_BUFFER_LENGTH					16384

	sLONG			bufferSize = MAX_BUFFER_LENGTH;
	char *			buffer = (char *)XBOX::vMalloc (bufferSize + 1, 0);

	if (NULL == buffer)
		return XBOX::VE_MEMORY_FULL;

	XBOX::VError	endPointError = XBOX::VE_OK;
	sLONG			bufferOffset = 0;
	sLONG			unreadBytes = 0;
	sLONG			lineLen = 0;
	bool			hostHeaderFound = false;
	bool			expectContinueFound = false;
	XBOX::VString	expectValueString;
	XBOX::VString	header;
	XBOX::VString	value;
	char *			startLinePtr = NULL;
	char *			endLinePtr = NULL;
	char *			endHeaderPtr = NULL;
	const sLONG		endLineSize = sizeof (HTTP_CRLF) - 1;
	const sLONG		endHeaderSize = sizeof (HTTP_CRLFCRLF) - 1;
	XBOX::VSize		contentLength = 0;
	bool			contentLengthFound = false;
	void *			bodyContentBuffer = NULL;
	XBOX::VSize		bodyContentSize = 0;
	const sLONG		READ_TIMEOUT = (inTimeout != 0) ? inTimeout : 10000;
	bool			stopReadingSocket = false;
	bool			wasCRLF = false;
	bool			requestEntityTooLarge = false;
	bool			skipRequestLineAndHeader = false;

	if (fParsingState != PS_WaitingForBody)
	{
		fParsingState = PS_Undefined;
		fRequestLine.Clear();

		fLocalAddress.FromLocalAddr (inEndPoint.GetRawSocket());
		fPeerAddress.FromPeerAddr (inEndPoint.GetRawSocket());
		fIsSSL = inEndPoint.IsSSL();
	}
	/*
	 *	Server previously sent 100 - Continue status code, let's now read request body
	 */
	else
	{
		skipRequestLineAndHeader = true;
		fParsingState = PS_ReadingBody;
		contentLengthFound = GetHeaders().GetContentLength (contentLength);
	}

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

	if (XBOX::VE_OK == endPointError && !skipRequestLineAndHeader)
		fParsingState = PS_ReadingRequestLine;

	while ((XBOX::VE_OK == endPointError) && !stopReadingSocket)
	{
		if ((0 == unreadBytes) || (bufferOffset >= MAX_BUFFER_LENGTH))
			bufferOffset = 0;

		bufferSize = MAX_BUFFER_LENGTH - bufferOffset;
#if VERSIONDEBUG
		if (bufferSize <= 0)
			assert (false);
#endif

#if LOG_IN_CONSOLE
		VDebugTimer readTimer;
#endif
		
		endPointError = inEndPoint.ReadWithTimeout (buffer + bufferOffset, (uLONG *)&bufferSize, 5000 /*timeout ms*/);
		
		if (fParsingState <= PS_ReadingHeaders)
			buffer[bufferOffset + bufferSize] = '\0'; //  YT 29-May-2012 - ACI0076811

		//jmo - tentatives de fix pour ne plus lire en boucle une socket sur laquelle le client a fait un shutdown...
		if(VE_OK==endPointError && 0==bufferSize)
		{
			//jmo - on ne devrait plus passer par là.
			xbox_assert(false);
			stopReadingSocket=true;
#if VERSIONWIN && VERSIONDEBUG
			::OutputDebugStringW (L"VEndPoint.Read() returned Zero Bytes !!");
#endif
		}
		
		//jmo - erreur qu'on recupere sur un shutdown cote client
		if(VE_SRVR_NOTHING_TO_READ==endPointError)
		{
			stopReadingSocket=true;
#if VERSIONWIN && VERSIONDEBUG
			::OutputDebugStringW (L"VEndPoint.Read() returned VE_SRVR_NOTHING_TO_READ !!");
#endif
		}
		
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
							fParsingError = HTTPProtocol::ReadRequestLine (fRequestLine, fRequestMethod, fRawURL, fHTTPVersion, true /*verifyRequestLineValidity*/);
							if (XBOX::VE_OK != fParsingError)
								break;

							HTTPProtocol::ParseURL (fRawURL, fURL, fURLPath, fURLQuery);

							fParsingState = PS_ReadingHeaders;

							if ((NULL != endHeaderPtr) && (endLinePtr == endHeaderPtr))
								stopReadingSocket = true;
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
									/*
									 *	YT 09-Nov-2012 - WAK0079099
									 *	If Expect header explicitly contains "100-Continue" then check later message body size
									 *	other else send 417 - Expectation Failed and exit now.
									 */
									else if (!expectContinueFound && HTTPServerTools::EqualASCIIVString (header, STRING_HEADER_EXPECT))
									{
										XBOX::VString expectValueString (value);

										if (!expectValueString.IsEmpty() && HTTPServerTools::EqualASCIIVString (expectValueString, STRING_HEADER_VALUE_100_CONTINUE))
										{
											expectContinueFound = true;
										}
										else
										{
											fParsingError = VE_HTTP_PROTOCOL_EXPECTATION_FAILED;
											break;
										}
									}
								}
							}
						}

						if ((NULL != endHeaderPtr) && (endLinePtr == endHeaderPtr))
						{
							/*
								Do not read socket anymore when header is complete for GET, HEAD, TRACE & OPTIONS requests. Body is not allowed in theses cases.
							*/
							 
							if ((HTTP_GET == fRequestMethod) || (HTTP_HEAD == fRequestMethod) || (HTTP_TRACE == fRequestMethod) || (HTTP_OPTIONS == fRequestMethod) || (HTTP_DELETE == fRequestMethod))
							{
								fParsingState = PS_ParsingFinished;
								stopReadingSocket = true;
							}
							else
							{
								fParsingState = PS_ReadingBody;
								
								contentLengthFound = GetHeaders().GetContentLength (contentLength);

								/*
								 *	Check "Expect: 100-Continue" request header and properly respond according to Content-Length limit
								 */
								if ((expectContinueFound) && (fHTTPVersion == VERSION_1_1) && ((fRequestMethod == HTTP_POST) || (fRequestMethod == HTTP_PUT)))
								{
									if (contentLengthFound && _AcceptIncomingDataSize (contentLength))
									{
										fParsingState = PS_WaitingForBody;
										fParsingError = VE_HTTP_PROTOCOL_CONTINUE;
									}
									else
									{
										requestEntityTooLarge = true;
										stopReadingSocket = true;
									}
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

							if ((contentLength > 0) && (bodyContentSize >= contentLength))
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

						if ((contentLength > 0) && (bodyContentSize == contentLength))
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
					if ((!contentLengthFound) && (fRequestMethod != HTTP_UNKNOWN) &&
						((fRequestMethod == HTTP_PUT) || (fRequestMethod == HTTP_POST))) // YT 03-Oct-2012 - WAK0073323 & WAK0072822 - Test if Content-Length was found (even if equals 0) - YT 06-Jul-2011 - TestCase MessageLength02 (RFC2616 4.4)
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

	if (((XBOX::VE_OK == fParsingError) || (VE_HTTP_PROTOCOL_CONTINUE == fParsingError)) && (XBOX::VE_OK == endPointError))
	{
		XBOX::VString	headerValue;
		XBOX::CharSet	charSet = XBOX::VTC_UNKNOWN;

		if (NULL != bodyContentBuffer)
		{
			if ((!contentLengthFound) && (fRequestMethod != HTTP_UNKNOWN) &&
				((fRequestMethod == HTTP_PUT) || (fRequestMethod == HTTP_POST))) // YT 03-Oct-2012 - WAK0073323 & WAK0072822 - Test if Content-Length was found (even if equals 0) - YT 06-Jul-2011 - TestCase MessageLength02 (RFC2616 4.4)
			{
				fParsingError = VE_HTTP_PROTOCOL_LENGTH_REQUIRED; 
			}
			else
			{
				/*
				 *	YT 05-Oct-2012 - WAK0072934 & WAK0072913
				 *		[HTTP-RFC] Server must use the content-length header when the request includes an identity transfer-encoding.
				 *		[HTTP-RFC] Server should reply with 501 when receives a request with a transfer-encoding it does not understand. 
				 */
				if (GetHeaders().GetHeaderValue (STRING_HEADER_TRANSFER_ENCODING, headerValue))
				{
					HTTPCompressionMethod	transferEncodingMethod = HTTPProtocol::NegotiateEncodingMethod (headerValue);
					if ((bodyContentSize > contentLength) && (COMPRESSION_IDENTITY == transferEncodingMethod))
						bodyContentSize = contentLength;
					else if (COMPRESSION_UNKNOWN == transferEncodingMethod)
						fParsingError = VE_HTTP_PROTOCOL_NOT_IMPLEMENTED; 
				}

				if ((XBOX::VE_OK == fParsingError) || (VE_HTTP_PROTOCOL_CONTINUE == fParsingError))
				{
					GetBody().SetDataPtr (bodyContentBuffer, bodyContentSize);

					// YT 16-Feb-2012 - ACI0075553 - Automatically decompress body (when applicable)
					if (GetHeaders().GetHeaderValue (STRING_HEADER_CONTENT_ENCODING, headerValue))
					{
						HTTPCompressionMethod	compressionMethod = HTTPProtocol::NegotiateEncodingMethod (headerValue);
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
		}

		if (PS_WaitingForBody != fParsingState)
			fParsingState = PS_ParsingFinished;

		/*
		 *	HTTP/1.1 requests must include Host header
		 */
		if ((fHTTPVersion == VERSION_1_1) && fHost.IsEmpty())
		{
			if (!GetHeaders().GetHeaderValue (STRING_HEADER_HOST, fHost))
				fParsingError = VHTTPServer::ThrowError (VE_HTTP_PROTOCOL_BAD_REQUEST, CVSTR ("HTTP/1.1 requests must include Host header"));
		}
		/*
		 *	YT 05-Oct-2012 - WAK0076702
		 *	Because HTTP/1.0 proxies do not understand the Connection header, 
		 *	however, HTTP/1.1 imposes an additional rule. If a Connection header 
		 *	is received in an HTTP/1.0 message, then it must have been incorrectly 
		 *	forwarded by an HTTP/1.0 proxy. Therefore, all of the headers it lists 
		 *	were also incorrectly forwarded, and must be ignored.
		 */
		else if ((fHTTPVersion == VERSION_1_0) && GetHeaders().GetHeaderValue (STRING_HEADER_CONNECTION, headerValue))
		{
			XBOX::VectorOfVString	values;
			headerValue.GetSubStrings (CHAR_COMMA, values, false, true);
			for (XBOX::VectorOfVString::const_iterator it = values.begin(); it != values.end(); ++it)
			{
				if (!HTTPServerTools::EqualASCIIVString (*it, STRING_HEADER_VALUE_CLOSE) &&
					!HTTPServerTools::EqualASCIIVString (*it, STRING_HEADER_VALUE_KEEP_ALIVE) &&
					GetHeaders().IsHeaderSet (*it))
					GetHeaders().RemoveHeader (*it);
			}
		}

		// YT 12-Oct-2009 - ACI0063558 - Request MUST include CRLF+CRLF at the end of header
		if (!skipRequestLineAndHeader && ((XBOX::VE_OK == fParsingError) || (VE_HTTP_PROTOCOL_CONTINUE == fParsingError)) && (NULL == endHeaderPtr))
			fParsingError = VHTTPServer::ThrowError (VE_HTTP_PROTOCOL_BAD_REQUEST, CVSTR ("Request header is not followed by CRLF+CRLF !!"));

		// YT 08-Jul-2011 - TestCase CharSet01 (RFC2616 14.2)
		if (((XBOX::VE_OK == fParsingError) || (VE_HTTP_PROTOCOL_CONTINUE == fParsingError)) && (GetHeaders().IsHeaderSet (STRING_HEADER_ACCEPT_CHARSET)))
		{
			XBOX::VString acceptCharsetValues;

			GetHeaders().GetHeaderValue (STRING_HEADER_ACCEPT_CHARSET, acceptCharsetValues);
			XBOX::CharSet charset = HTTPProtocol::NegotiateCharSet (acceptCharsetValues);
			if (XBOX::VTC_UNKNOWN == charset)
				fParsingError = VE_HTTP_PROTOCOL_NOT_ACCEPTABLE;
		}

		// Check if body is compressed and if compression method is supported
		if (((XBOX::VE_OK == fParsingError) || (VE_HTTP_PROTOCOL_CONTINUE == fParsingError)) && (GetHeaders().IsHeaderSet (STRING_HEADER_ACCEPT_ENCODING)))
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
	assert (false); // TODO: Cleanup, Dead Code, should never be called
// 	return inherited::_ReadFromStream ( inStream,
// 										CVSTR (""),
// 										&fRequestLine,
// 										&fRequestMethod,
// 										&fHTTPVersion,
// 										&fHost,
// 										&fRawURL,
// 										&fURL,
// 										&fURLPath,
// 										&fURLQuery,
// 										&fParsingState,
// 										&fParsingError);
	return XBOX::VE_OK;
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


bool VHTTPRequest::GetCookies (XBOX::VectorOfCookie& outCookies) const
{
	XBOX::VectorOfVString values;

	outCookies.clear();
	if (GetHeaders().GetHeaderValues (HEADER_COOKIE, values))
	{
		for (XBOX::VectorOfVString::const_iterator it = values.begin(); it != values.end(); ++it)
		{
			XBOX::VHTTPCookie * cookie = new XBOX::VHTTPCookie (*it);

			if (NULL != cookie)
				outCookies.push_back (cookie);
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


const XBOX::VMIMEMessage *VHTTPRequest::GetHTMLForm() const
{
	if (NULL == fHTMLForm)
	{
		XBOX::VString	contentType;
		XBOX::VString	rawQueryString;

		fHTMLForm = new XBOX::VMIMEMessage();
		GetHeaders().GetHeaderValue (STRING_HEADER_CONTENT_TYPE, contentType);

		// YT 17-Dec-2012 - ACI0079673 - Use RawQueryURL (names & values will be individually decoded in VMIMEMessage::_ReadUrl())
		sLONG questionMarkPos = fRawURL.FindUniChar (CHAR_QUESTION_MARK);
		if (questionMarkPos > 0)
			fRawURL.GetSubString (questionMarkPos + 1, fRawURL.GetLength() - questionMarkPos, rawQueryString);

		fHTMLForm->Load (((fRequestMethod == HTTP_POST) || (fRequestMethod == HTTP_PUT)), contentType, rawQueryString, GetBody());
	}

	return fHTMLForm;
}
