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


static
bool _RemoveProjectPatternFromURL (const XBOX::VString& inProjectPattern, XBOX::VString& ioURL)
{
	if (inProjectPattern.IsEmpty())
		return false;

	XBOX::VString url (ioURL);
	sLONG pos = HTTPServerTools::FindASCIIVString (url, inProjectPattern);
	if (pos == 2) // Takes the starting CHAR_SOLIDUS into account
	{
		url.GetSubString (pos + inProjectPattern.GetLength(), url.GetLength() - inProjectPattern.GetLength() - 1, ioURL);
		return true;
	}

	return false;
}


static 
bool _GetCheckRFC822DateValue (const VHTTPHeader& inHeader, const XBOX::VString& inHeaderName, XBOX::VTime& outDateTimeValue, bool bCheckDateValidity = false)
{
	XBOX::VString dateTimeString;

	outDateTimeValue.Clear();

	if (inHeader.GetHeaderValue (inHeaderName, dateTimeString) && !dateTimeString.IsEmpty())
	{
		outDateTimeValue.FromRfc822String (dateTimeString);

		bool bIsValid = IsVTimeValid (outDateTimeValue);

		/*
		 *	Check VTime validity. Here, just make an RFC822 Date string and compare to 
		 *	original header value (More efficient than using a regex pattern...I guess)
		 */
		if (bIsValid && bCheckDateValidity)
		{
			XBOX::VString tempString;
			outDateTimeValue.GetRfc822String (tempString);

			if (!HTTPServerTools::EqualASCIIVString (dateTimeString, tempString))
			{
				outDateTimeValue.Clear();
				bIsValid = false;
			}
		}

		return bIsValid;
	}

	return false;
}


//--------------------------------------------------------------------------------------------------


VHTTPResponse::VHTTPResponse (VHTTPServer *inServer, XBOX::VTCPEndPoint* inEndPoint, XBOX::VTCPSelectIOPool* inIOPool)
: fEndPoint (inEndPoint)
, fHTTPServer (inServer)
, fVirtualHost (NULL)
, fRequest (NULL)
, fResponseStatusCode (HTTP_UNDEFINED)
, fCanCacheBody (false)
, fHTTPVersion (VERSION_1_1)
, fCanCompressBody (false)
, fCompressionMode (COMPRESSION_NONE)
, fWantedAuthMethod (AUTH_NONE)
, fFileToSend (NULL)
, fIsChunked (false)
, fNumOfChunkSent (0)
, fMinCompressionThreshold (-1) // -1 means: Use Default Threshold defined in project settings
, fMaxCompressionThreshold (-1) // -1 means: Use Default Threshold defined in project settings
, fHeaderSent (false)
, fForceCloseSession (false)
, fUseDefaultCharset (true)
{
	fStartRequestTime = XBOX::VSystem::GetCurrentTime();
}


VHTTPResponse::~VHTTPResponse()
{
	if (NULL != fRequest)
	{
		delete fRequest;
		fRequest = NULL;
	}

	if (NULL != fEndPoint)
	{
		fEndPoint->Close();
		XBOX::ReleaseRefCountable( &fEndPoint);
	}

	XBOX::ReleaseRefCountable (&fVirtualHost);
	XBOX::ReleaseRefCountable (&fFileToSend);
}


void VHTTPResponse::Reset()
{
	inherited::Clear();

	XBOX::ReleaseRefCountable (&fVirtualHost);
	if (NULL != fRequest)
		fRequest->Reset();
	fResponseStatusCode  = HTTP_UNDEFINED;
	fCanCacheBody = false;
	fHTTPVersion = VERSION_1_1;
	fCanCompressBody = false;
	fCompressionMode = COMPRESSION_NONE;
	fWantedAuthMethod = AUTH_NONE;
	fStartRequestTime = XBOX::VSystem::GetCurrentTime();
	XBOX::ReleaseRefCountable (&fFileToSend);
	fIsChunked = false;
	fNumOfChunkSent = 0;
	fMinCompressionThreshold = -1;
	fMaxCompressionThreshold = -1;
	fHeaderSent = false;
	fUseDefaultCharset = true;
}


XBOX::VError VHTTPResponse::SetResponseBody (const void *inData, XBOX::VSize inDataSize)
{
	XBOX::VError error = XBOX::VE_OK;

	if (XBOX::VE_OK == (error = GetBody().OpenWriting()))
	{
		error = GetBody().PutData (inData, inDataSize);
		GetBody().CloseWriting();
	}

	return error;
}


XBOX::VError VHTTPResponse::SetFileToSend (XBOX::VFile *inFileToSend)
{
	if (NULL != fFileToSend)
		XBOX::QuickReleaseRefCountable (fFileToSend);

	fFileToSend = XBOX::RetainRefCountable (inFileToSend);

	return XBOX::VE_OK;
}


bool VHTTPResponse::HasFileToSend()
{
	return (NULL != fFileToSend);
}


XBOX::VError VHTTPResponse::ReadRequestFromEndPoint (uLONG inTimeout)
{
	XBOX::VError error = VE_HTTP_INVALID_INTERNAL_STATE;

	if (NULL == fRequest)
		fRequest = new VHTTPRequest();

	if (NULL != fRequest)
	{
		error = fRequest->ReadFromEndPoint (*fEndPoint, inTimeout);

		if (XBOX::VE_OK == error)
			fRequest->_ExtractAuthenticationInfos();
	}

	return error;
}


const IHTTPRequest& VHTTPResponse::GetRequest() const
{
	return (const IHTTPRequest&)*fRequest;
}


const HTTPVersion VHTTPResponse::GetRequestHTTPVersion() const
{
	return fRequest->GetHTTPVersion();
}


const VHTTPHeader& VHTTPResponse::GetRequestHeader() const
{
	return fRequest->GetHTTPHeaders();
}


const XBOX::VString& VHTTPResponse::GetRequestURL() const
{
	return fRequest->GetURL();
}


void VHTTPResponse::SetRequestURLPath (const XBOX::VString& inURLPath)
{
	fRequest->fURLPath.FromString (inURLPath);
}


const XBOX::VString& VHTTPResponse::GetRequestRawURL() const
{
	return fRequest->GetRawURL();
}


const XBOX::VString& VHTTPResponse::GetRequestURLPath() const
{
	return fRequest->GetURLPath();
}


const XBOX::VString& VHTTPResponse::GetRequestURLQuery() const
{
	return fRequest->GetURLQuery();
}


HTTPRequestMethod VHTTPResponse::GetRequestMethod() const
{
	return fRequest->GetRequestMethod();
}


void VHTTPResponse::GetRequestMethodName (XBOX::VString& outMethodName) const
{
	HTTPProtocol::MakeHTTPMethodString (fRequest->GetRequestMethod(), outMethodName);
}


bool VHTTPResponse::AddResponseHeader (const HTTPCommonHeaderCode inCode, const XBOX::VString& inValue, bool inOverride)
{
	return GetHeaders().SetHeaderValue (inCode, inValue, inOverride);
}


bool VHTTPResponse::AddResponseHeader (const XBOX::VString& inName, const XBOX::VString& inValue, bool inOverride)
{
	return GetHeaders().SetHeaderValue (inName, inValue, inOverride);
}


bool VHTTPResponse::AddResponseHeader (const XBOX::VString& inName, sLONG inValue, bool inOverride)
{
	return GetHeaders().SetHeaderValue (inName, inValue, inOverride);
}


bool VHTTPResponse::AddResponseHeader (const XBOX::VString& inName, const XBOX::VTime& inValue, bool inOverride)
{
	return GetHeaders().SetHeaderValue (inName, inValue, inOverride);
}


bool VHTTPResponse::SetContentLengthHeader (const sLONG8 inValue)
{
	XBOX::VString string;
	string.FromLong8 (inValue);
	return GetHeaders().SetHeaderValue (STRING_HEADER_CONTENT_LENGTH, string, true);
}


bool VHTTPResponse::SetExpiresHeader (const sLONG inValue)
{
	XBOX::VString dateString;
	HTTPProtocol::MakeRFC822GMTDateString (inValue, dateString);
	return GetHeaders().SetHeaderValue (STRING_HEADER_EXPIRES, dateString, true);
}


bool VHTTPResponse::SetExpiresHeader (const XBOX::VTime& inValue)
{
	XBOX::VString dateString;
	inValue.GetRfc822String (dateString);
	return GetHeaders().SetHeaderValue (STRING_HEADER_EXPIRES, dateString, true);
}


bool VHTTPResponse::IsResponseHeaderSet (const HTTPCommonHeaderCode inCode) const
{
	return GetHeaders().IsHeaderSet (inCode);
}


bool VHTTPResponse::IsResponseHeaderSet (const XBOX::VString& inName) const
{
	return GetHeaders().IsHeaderSet (inName);
}


bool VHTTPResponse::GetResponseHeader (const HTTPCommonHeaderCode inCode, XBOX::VString& outValue) const
{
	return GetHeaders().GetHeaderValue (inCode, outValue);
}


bool VHTTPResponse::GetResponseHeader (const XBOX::VString& inName, XBOX::VString& outValue) const
{
	return GetHeaders().GetHeaderValue (inName, outValue);
}


bool VHTTPResponse::SetContentTypeHeader (const XBOX::VString& inValue, const XBOX::CharSet inCharSet)
{
	return GetHeaders().SetContentType (inValue, inCharSet);
}


bool VHTTPResponse::GetContentTypeHeader (XBOX::VString& outValue, XBOX::CharSet *outCharSet) const
{
	return GetHeaders().GetContentType (outValue, outCharSet);
}


MimeTypeKind VHTTPResponse::GetContentTypeKind() const
{
	XBOX::VString contentType;

	GetHeaders().GetContentType (contentType, NULL);

	return VMimeTypeManager::GetMimeTypeKind (contentType);
}


bool VHTTPResponse::GetIfModifiedSinceHeader (XBOX::VTime& outTime)
{
	return _GetCheckRFC822DateValue (fRequest->GetHTTPHeaders(), STRING_HEADER_IF_MODIFIED_SINCE, outTime);
}


bool VHTTPResponse::GetIfUnmodifiedSinceHeader (XBOX::VTime& outTime)
{
	return _GetCheckRFC822DateValue (fRequest->GetHTTPHeaders(), STRING_HEADER_IF_UNMODIFIED_SINCE, outTime, true); // YT 18-Oct-2012 - WAK0078811
}


XBOX::VError VHTTPResponse::_CompressData()
{
	if (fCompressionMode != COMPRESSION_NONE)
		return XBOX::VE_OK;

	XBOX::VError	error = XBOX::VE_OK;
	bool			isOK = false;

	if (fCanCompressBody)
	{
		// Negotiate compression Method according to the request header

		XBOX::VString encoding;
		if (GetRequestHeader().GetHeaderValue (STRING_HEADER_ACCEPT_ENCODING, encoding))
		{
			fCompressionMode = HTTPProtocol::NegotiateEncodingMethod (encoding);

			if (COMPRESSION_NONE != fCompressionMode)
			{
#if HTTP_SERVER_VERBOSE_MODE || LOG_IN_CONSOLE
				VDebugTimer compressionTimer;
#endif
				error = HTTPServerTools::CompressStream (GetBody(), fCompressionMode);

				if (XBOX::VE_OK == error)
				{
					XBOX::VString contentEncoding;
					HTTPProtocol::GetEncodingMethodName (fCompressionMode, contentEncoding);

					/* Tell that Content-Encoding was negotiated */
					GetHeaders().SetHeaderValue (STRING_HEADER_VARY, STRING_HEADER_CONTENT_ENCODING, false);
					GetHeaders().SetHeaderValue (STRING_HEADER_CONTENT_ENCODING, contentEncoding);

					/* And of course change the Content-Length */
					SetContentLengthHeader (GetBody().GetSize());

					isOK = true;
				}

#if HTTP_SERVER_VERBOSE_MODE
				GetHeaders().SetHeaderValue (CVSTR ("X-CompressionTime"), compressionTimer.GetElapsedTime());
#endif
#if LOG_IN_CONSOLE
				compressionTimer.DebugMsg ("* VHTTPResponse::_CompressData()");
#endif
			}
		}
	}

	if (!isOK)
		fCompressionMode = COMPRESSION_NONE;

	return error;
}


bool VHTTPResponse::_NormalizeResponseHeader()
{
	/*
	 *	Do NOT send additional header with 100-Continue Status
	 */
	if (fResponseStatusCode == HTTP_CONTINUE)
	{
		GetHeaders().Clear();
		return true;
	}

	/*
	 *	HTTP automatic fixes: to mimic Apache's behavior for CGIs
	 */
	XBOX::VString fieldValue;

	/*
	 *	Write Server Header
	 */
	HTTPProtocol::MakeServerString (fieldValue, false, false);
	GetHeaders().SetHeaderValue (STRING_HEADER_SERVER, fieldValue, true);

	/*
	 *	Write Date Header
	 */
	if (!GetHeaders().IsHeaderSet (STRING_HEADER_DATE))
	{
		HTTPProtocol::MakeRFC822GMTDateString (GMT_NOW, fieldValue, false);
		GetHeaders().SetHeaderValue (STRING_HEADER_DATE, fieldValue);
	}

	/*
	 *	Special CGI case: the CGI author can use the Status or X-Status headers to set the response code
	 */
	if (GetHeaders().IsHeaderSet (STRING_HEADER_STATUS))
	{
		if (GetHeaders().GetHeaderValue (STRING_HEADER_STATUS, fieldValue))
		{
			SetResponseStatusCode ((HTTPStatusCode)HTTPServerTools::GetLongFromString (fieldValue));
			GetHeaders().RemoveHeader (STRING_HEADER_STATUS);
		}
	}

	/*
	 *	We still support some legacy, non standard special token:
	 */
	if (GetHeaders().IsHeaderSet (STRING_HEADER_X_STATUS))
	{
		if (GetHeaders().GetHeaderValue (STRING_HEADER_X_STATUS, fieldValue))
		{
			SetResponseStatusCode ((HTTPStatusCode)HTTPServerTools::GetLongFromString (fieldValue));
			GetHeaders().RemoveHeader (STRING_HEADER_X_STATUS);
		}
	}

	if (GetHeaders().IsHeaderSet (STRING_HEADER_X_VERSION))
	{
		if (GetHeaders().GetHeaderValue (STRING_HEADER_X_VERSION, fieldValue))
		{
			if (HTTPServerTools::EqualASCIICString (fieldValue, "http/1.0"))
				SetHTTPVersion (VERSION_1_0);
			else
				SetHTTPVersion (VERSION_1_1);
			GetHeaders().RemoveHeader (STRING_HEADER_X_VERSION);
		}
	}

	/*
	 *	Classic Redirect trick
	 */
	if (GetHeaders().IsHeaderSet (STRING_HEADER_LOCATION) && (((sLONG)fResponseStatusCode / 100) != 3))
		SetResponseStatusCode (HTTP_FOUND);


	/*
	 *	Best practice see: http://developer.yahoo.com/performance/rules.html#expires
	 */
	/*
	if (fCanCacheBody && (fResponseStatusCode == 200) && !GetHeaders().IsHeaderSet (STRING_HEADER_EXPIRES))
	{
		HTTPProtocol::MakeRFC822GMTDateString (GMT_FAR_FUTURE, fieldValue);
		GetHeaders().SetHeaderValue (STRING_HEADER_EXPIRES, fieldValue);
	}
	*/

	/*
	 *	Fix Content-Length when missing
	 */
	if (!GetHeaders().IsHeaderSet (STRING_HEADER_CONTENT_LENGTH) && (GetBody().GetDataSize() >= 0)) // YT 30-Nov-2012 - ACI0079288 - Set Content-Length even if body is empty (to prevent browser waiting a nonexistent body)
	{
		fieldValue.FromLong8 (GetBody().GetDataSize());
		GetHeaders().SetHeaderValue (STRING_HEADER_CONTENT_LENGTH, fieldValue);
	}

	/*
	 *	Fix the generic "application/octet-stream" Content-Type when missing
	 */
	if (!GetHeaders().IsHeaderSet (STRING_HEADER_CONTENT_TYPE) && GetBody().GetDataSize())
		GetHeaders().SetHeaderValue (STRING_HEADER_CONTENT_TYPE, STRING_CONTENT_TYPE_BINARY);

	return true;
}


XBOX::VError VHTTPResponse::_WriteChunkSize (XBOX::VSize inChunkSize)
{
	char	chunkBuffer[32] = {0};
	uLONG	chunkBufferSize = 0;

	if (inChunkSize > 0)
	{
		if (0 == fNumOfChunkSent)	// first body chunk
			chunkBufferSize = sprintf (chunkBuffer, "%lX\r\n", inChunkSize);
		else
			chunkBufferSize = sprintf (chunkBuffer, "\r\n%lX\r\n", inChunkSize);

		++fNumOfChunkSent;
	}
	else
	{
		// Special ending line for chunked encoding
		chunkBufferSize = sprintf (chunkBuffer, "\r\n0\r\n\r\n");
	}

	return _WriteToSocket (chunkBuffer, &chunkBufferSize);
}


XBOX::VError VHTTPResponse::_WriteToSocket (void *inBuffer, uLONG *ioBytes)
{
#if LOG_IN_CONSOLE
	VDebugTimer timer;
#endif

	XBOX::VError error = XBOX::VE_OK;
	XBOX::StErrorContextInstaller errorContext (VE_SRVR_WRITE_FAILED,
												VE_SRVR_CONNECTION_BROKEN,
												VE_SOCK_WRITE_FAILED,
												XBOX::VE_OK); // YT 07-Dec-2009 - ACI0063626

	error = fEndPoint->WriteExactly (inBuffer, *ioBytes, 10000 /*10s*/);

#if LOG_IN_CONSOLE
	timer.DebugMsg ("* VHTTPResponse::_WriteToSocket()");
#endif

	return error;
}


XBOX::VError VHTTPResponse::_SendResponseHeader()
{
	if (fHeaderSent)
		return XBOX::VE_OK; // Not sure we have to report the error

	_NormalizeResponseHeader();

	XBOX::VString string;

	// Make Status-Line
	HTTPProtocol::MakeStatusLine (fHTTPVersion, fResponseStatusCode, string);

	// Write Status-Line + Headers
	GetHeaders().ToString (string);
	string.AppendCString (HTTP_CRLF);

	XBOX::StStringConverter<char>	converter (string, XBOX::VTC_UTF_8);
	char *							buffer = (char *)converter.GetCPointer();
	uLONG							bufferSize = (uLONG)converter.GetSize();
	XBOX::VError					error = _WriteToSocket (buffer, &bufferSize);

	fHeaderSent = (XBOX::VE_OK == error);

	return error;
}


XBOX::VError VHTTPResponse::_SendResponseBody()
{
	XBOX::VError error = XBOX::VE_OK;

	if ((GetBody().GetDataSize() > 0) && GetBody().GetDataPtr())
	{
		uLONG	bufferSize = (uLONG)GetBody().GetDataSize();
		void *	buffer = GetBody().GetDataPtr();
#if VERSIONDEBUG
		if ((bufferSize <= 0) || (NULL == buffer))
			assert (false);
#endif
		error = _WriteToSocket (buffer, &bufferSize);
#if VERSIONDEBUG
		if (bufferSize != (uLONG)GetBody().GetDataSize())
			assert (false);
#endif
	}

	return error;
}


XBOX::VError VHTTPResponse::_SendResponseWithStatusCode (HTTPStatusCode inStatusCode)
{
	XBOX::VError error = XBOX::VE_OK;

	if (XBOX::VE_OK == (error = ReplyWithStatusCode (inStatusCode)))
	{
		if (XBOX::VE_OK == (error = _SendResponseHeader()))
			error = _SendResponseBody();
	}

	return error;
}


XBOX::VError VHTTPResponse::SendResponse()
{
	XBOX::VError error = XBOX::VE_OK;

	if (fIsChunked)
	{
		// First send buffered data that was not already sent...
		if (XBOX::VE_OK == (error = _SendResponseBody()))
		{
			// Then send special ending line for chunked encoding
			error = _WriteChunkSize (0);
		}
	}
	else
	{
		XBOX::VString		contentType;
		XBOX::VString		contentEncoding;
		XBOX::CharSet		charset = XBOX::VTC_UNKNOWN;
		XBOX::MimeTypeKind	mimeTypeKind = MIMETYPE_UNDEFINED;

		if (GetHeaders().GetContentType (contentType, &charset))
		{
			mimeTypeKind = VMimeTypeManager::GetMimeTypeKind (contentType);
			/*
			 *	Check if Content-Type charset parameter is missing, and for MIMETYPE_TEXT only, 
			 *	automatically set charset using VHTTPMessage.fBody's charset... (not sure I'm clear :)
			 */
			if (fUseDefaultCharset && (XBOX::MIMETYPE_TEXT == mimeTypeKind) && (XBOX::VTC_UNKNOWN == charset))
			{
				charset = GetBody().GetCharSet();
				GetHeaders().SetContentType (contentType, charset);
			}
		}

		if (GetHeaders().GetHeaderValue (HEADER_CONTENT_ENCODING, contentEncoding) && !contentEncoding.IsEmpty())
		{
			if (HTTPProtocol::NegotiateEncodingMethod (contentEncoding) != COMPRESSION_UNKNOWN)
				fCanCompressBody = false;
		}

		if (HTTP_UNDEFINED == fResponseStatusCode)
			fResponseStatusCode = HTTP_OK;

		VVirtualHost *virtualHost = dynamic_cast<VVirtualHost *>(GetVirtualHost());

		if (NULL != virtualHost)
		{
			// Compress HTTP Message body when applicable
			bool compressionEnabled = virtualHost->GetSettings()->GetEnableCompression();
			if (fCanCompressBody && compressionEnabled)
			{
				sLONG size = (sLONG)GetBody().GetSize();
				sLONG minThreshold = (fMinCompressionThreshold == -1) ? virtualHost->GetSettings()->GetCompressionMinThreshold() : fMinCompressionThreshold;
				sLONG maxThreshold = (fMaxCompressionThreshold == -1) ? virtualHost->GetSettings()->GetCompressionMaxThreshold() : fMaxCompressionThreshold;

				if ((size > minThreshold) && (size <= maxThreshold))
				{
					if (!contentType.IsEmpty() && (XBOX::MIMETYPE_TEXT == mimeTypeKind))
					{
						error = _CompressData();
					}
				}
			}
		}

		// Put HTTP Message body in cache when applicable
		if ((NULL != virtualHost) && fCanCacheBody && (fResponseStatusCode == HTTP_OK))
		{
			VCacheManager *	cacheManager = virtualHost->GetProject()->GetHTTPServer()->GetCacheManager();
			XBOX::VFilePath	filePath;
			XBOX::VString	locationPath;
			XBOX::VTime		lastModified;
			XBOX::VString	lastModifiedString;
			XBOX::VError	fileError = XBOX::VE_OK;
			bool			staticFile = false;

			if (XBOX::VE_OK == (fileError = virtualHost->GetFilePathFromURL (fRequest->GetURLPath(), locationPath)))
			{
				filePath.FromFullPath (locationPath);
				if (filePath.IsFile() && (XBOX::VE_OK == HTTPServerTools::GetFileInfos (filePath, &lastModified)))
				{
					staticFile = true;
					HTTPProtocol::MakeRFC822GMTDateString (lastModified, lastModifiedString);
				}
			}

			if ((XBOX::VE_OK == fileError) && (NULL != cacheManager) && cacheManager->GetEnableDataCache())
			{
				uLONG	bufferSize = (uLONG)GetBody().GetSize();
				if (bufferSize <= cacheManager->GetCachedObjectMaxSize())
				{
					void *			buffer = GetBody().GetDataPtr();
					XBOX::VTime		lastChecked;
					VCachedObject *	cachedObject = NULL;

					XBOX::VTime::Now (lastChecked);

					bool ok = cacheManager->AddPageToCache (fRequest->GetURLPath(),
															virtualHost->GetUUIDString(),
															contentType,
															buffer,
															bufferSize,
															filePath,
															lastChecked,
															lastModified,
															staticFile,
															fCompressionMode,
															&cachedObject);

					if (ok)
					{
						if (NULL != cachedObject)
						{
							XBOX::VTime expirationDate;
							sLONG maxAge = cachedObject->GetMaxAge();

							if (maxAge > 0)
							{
								XBOX::VString	string;

								string.FromCString ("max-age=");
								string.AppendLong (maxAge);
								AddResponseHeader (STRING_HEADER_CACHE_CONTROL, string);
								AddResponseHeader (STRING_HEADER_AGE, cachedObject->GetAge());
								if (cachedObject->GetExpirationDate (expirationDate))
									AddResponseHeader (STRING_HEADER_EXPIRES, expirationDate);
							}
							else if (cachedObject->GetExpirationDate (expirationDate) && IsVTimeValid (expirationDate))
							{
								AddResponseHeader (STRING_HEADER_EXPIRES, expirationDate);
							}

							XBOX::QuickReleaseRefCountable (cachedObject);
						}
					}
				}
			}

			if (!lastModifiedString.IsEmpty())
				AddResponseHeader (STRING_HEADER_LAST_MODIFIED, lastModifiedString);
		}

		if (HTTP_OK == fResponseStatusCode)
		{
			if (NULL != fFileToSend)
			{
				if (fFileToSend->Exists())
				{
					sLONG8	fileSize = 0;
					
					fFileToSend->GetSize (&fileSize);
					SetContentLengthHeader (fileSize); // YT 18-Jul-2011 - ACI0072287

					if (!IsResponseHeaderSet (STRING_HEADER_CONTENT_TYPE))
					{
						XBOX::VString fileExtension;
						fFileToSend->GetPath().GetExtension (fileExtension);
						VMimeTypeManager::FindContentType (fileExtension, contentType);
						AddResponseHeader (STRING_HEADER_CONTENT_TYPE, contentType);
					}

					if (XBOX::VE_OK == (error = _SendResponseHeader()))
					{
						const sLONG CHUNK_BUFFER_SIZE = 0xFFFF;
						char *	chunkBuffer = (char *)XBOX::vMalloc (CHUNK_BUFFER_SIZE, 0);
						if (testAssert (NULL != chunkBuffer))
						{
							XBOX::VFileDesc *fileDesc = NULL;
							if ((XBOX::VE_OK == fFileToSend->Open (XBOX::FA_READ, &fileDesc, XBOX::FO_SequentialScan)) && (NULL != fileDesc))
							{
								uLONG			chunkSize = 0;
								XBOX::VSize		readBytes = 0;
								XBOX::VError	fileError = XBOX::VE_OK;
								sLONG8			unreadSize = 0;

								unreadSize = fileSize;
								fileDesc->SetPos (0, true);

								while ((XBOX::VE_OK == fileError) && (unreadSize > 0))
								{
									chunkSize = (unreadSize > CHUNK_BUFFER_SIZE) ? CHUNK_BUFFER_SIZE : unreadSize;
									fileError = fileDesc->GetDataAtPos (chunkBuffer, chunkSize, 0, &readBytes);
									unreadSize -= (sLONG8)readBytes;

									if ((XBOX::VE_OK == fileError) || (XBOX::VE_STREAM_EOF == fileError))
									{
										error = _WriteToSocket (chunkBuffer, &chunkSize);
										if (XBOX::VE_OK != error)
											break;
									}
								}

								delete fileDesc;
								fileDesc = NULL;
							}
							else
							{
								error = _SendResponseWithStatusCode (HTTP_INTERNAL_SERVER_ERROR);
							}

							XBOX::vFree (chunkBuffer);
							chunkBuffer = NULL;
						}
						else
						{
							error = _SendResponseWithStatusCode (HTTP_INTERNAL_SERVER_ERROR);
						}
					}
				}
				else
				{
					error = _SendResponseWithStatusCode (HTTP_NOT_FOUND);
				}

				XBOX::ReleaseRefCountable (&fFileToSend);
			}
			else if (GetBody().GetDataSize() >= 0)
			{
				if (XBOX::VE_OK == (error = _SendResponseHeader()))
				{
					if (NULL != GetBody().GetDataPtr())
						error = _SendResponseBody();
				}
			}
			else
			{
				error = _SendResponseWithStatusCode (HTTP_INTERNAL_SERVER_ERROR);
			}
		}
		else
		{
			error = _SendResponseWithStatusCode (fResponseStatusCode);
		}
	}

	return error;
}


XBOX::VError VHTTPResponse::ReplyWithStatusCode (HTTPStatusCode inValue, XBOX::VString *inExplanationString)
{
	if (HTTP_UNDEFINED == fResponseStatusCode)
		fResponseStatusCode = inValue;

	if ((HTTP_UNAUTHORIZED == inValue) || (HTTP_NOT_MODIFIED == inValue) || ((3 == (sLONG)inValue / 100)) || (HTTP_CONTINUE == inValue))
	{
		if (HTTP_UNAUTHORIZED == inValue)
		{
			GetVirtualHost()->GetProject()->GetAuthenticationManager()->SetAuthorizationHeader (this);
		}
	}
	else
	{
		if (!HTTPProtocol::IsValidStatusCode (fResponseStatusCode))
			fResponseStatusCode = HTTP_INTERNAL_SERVER_ERROR;

		if (0 == GetBody().GetDataSize() && !HasFileToSend())
		{
			GetHeaders().SetHeaderValue (STRING_HEADER_CONTENT_TYPE, STRING_CONTENT_TYPE_HTML);
			HTTPProtocol::MakeErrorResponse (inValue, GetBody(), inExplanationString);
		}
	}

	fCanCacheBody = false;
	fCanCompressBody = false;

	return XBOX::VE_OK;
}


XBOX::VError VHTTPResponse::SendData (void *inData, XBOX::VSize inDataSize, bool isChunked)
{
	if ((NULL == inData) || (inDataSize <= 0))
		return VE_HTTP_INVALID_ARGUMENT;

	XBOX::VError	error = XBOX::VE_OK;
	XBOX::VSize		bodyOffset = 0;
	void *			buffer = inData;
	XBOX::VSize		bufferSize = inDataSize;

	if (!fNumOfChunkSent)
	{
		// Check if the data chunk contains a StatusLine or/and and HTTP Header (used only for 4D's backward compatibility)
		if (_InitResponseHeaderFromBuffer (buffer, bufferSize, bodyOffset)) // YT 17-Jul-2012 - ACI0075655
		{
			if ((bufferSize - bodyOffset) > 0)
			{
				buffer = (((char *)inData) + bodyOffset);
				bufferSize = (inDataSize - bodyOffset);
			}
			else
			{
				buffer = NULL;
				bufferSize = 0;
			}
		}

		// Check if chunked data is possible (that's not the case before HTTP/1.1)
		if (isChunked && (GetRequestHTTPVersion() == VERSION_1_1))
		{
			fIsChunked = AddResponseHeader (STRING_HEADER_TRANSFER_ENCODING, STRING_HEADER_VALUE_CHUNKED);
			/* For instance, for simplification, no compression allowed in chunked mode */
			AllowCompression (false);
		}

		SetCacheBodyMessage (false);
		if (!IsResponseHeaderSet (STRING_HEADER_EXPIRES))
			SetExpiresHeader (GMT_NOW);

		if (fIsChunked)
		{
			SetResponseStatusCode (HTTP_OK);
			error = _SendResponseHeader();
		}
	}

	if ((XBOX::VE_OK == error) && (NULL != buffer) && (bufferSize > 0))
	{
		if (fIsChunked)
		{
			uLONG	size = (uLONG)bufferSize;

			error = _WriteChunkSize (bufferSize);
			if (XBOX::VE_OK == error)
				error = _WriteToSocket (buffer, &size);
		}
		else
		{
			/* Buffered response: Append data to body VPtrStream */
			error = SetResponseBody (buffer, bufferSize);
			AllowCompression (true);
		}
	}

	return error;
}


inline
XBOX::VError VHTTPResponse::SendResponseHeader()
{
	return _SendResponseHeader();
}


bool VHTTPResponse::IsCookieSet (const XBOX::VString& inName) const
{
	XBOX::VectorOfVString cookieValues;

	if (GetHeaders().GetHeaderValues (HEADER_SET_COOKIE, cookieValues))
	{
		XBOX::VString cookieName (inName);
		cookieName.AppendUniChar (CHAR_EQUALS_SIGN);

		for (XBOX::VectorOfVString::const_iterator it = cookieValues.begin(); it != cookieValues.end(); ++it)
		{
			if (HTTPServerTools::FindASCIIVString ((*it), cookieName) > 0)
				return true;
		}
	}

	return false;
}


bool VHTTPResponse::IsCookieSet (const VHTTPCookie& inCookie) const
{
	XBOX::VectorOfVString	cookieValues;
	
	if (GetHeaders().GetHeaderValues (HEADER_SET_COOKIE, cookieValues))
	{
		XBOX::VString cookieName (inCookie.GetName());
		cookieName.AppendUniChar (CHAR_EQUALS_SIGN);
		
		for (XBOX::VectorOfVString::const_iterator it = cookieValues.begin(); it != cookieValues.end(); ++it)
		{
			XBOX::VectorOfVString multipleCookieValues;

			(*it).GetSubStrings (CHAR_SEMICOLON, multipleCookieValues, false, true);

			XBOX::VectorOfVString::const_iterator found = std::find_if (multipleCookieValues.begin(), multipleCookieValues.end(), FindVStringFunctor (cookieName));
			if (found != multipleCookieValues.end())
			{
				VHTTPCookie cookie (*found);
				if (cookie == inCookie)
					return true;
			}
		}
	}

	return false;
}


bool VHTTPResponse::GetCookie (const XBOX::VString& inName, XBOX::VString& outValue) const
{
	return GetHeaders().GetCookie (inName, outValue);
}


bool VHTTPResponse::SetCookie (const XBOX::VString& inName, const XBOX::VString& inValue)
{
	return GetHeaders().SetCookie (inName, inValue);
}


bool VHTTPResponse::AddCookie (const XBOX::VString& inName, const XBOX::VString& inValue, const XBOX::VString& inComment, const XBOX::VString& inDomain, const XBOX::VString& inPath, bool inSecure, bool inHTTPOnly, sLONG inMaxAge, bool inAlwaysUseExpires)
{
	VHTTPCookie cookie;

	cookie.SetVersion (1);
	cookie.SetName (inName);
	cookie.SetValue (inValue);
	cookie.SetComment (inComment);
	cookie.SetDomain (inDomain);
	cookie.SetPath (inPath);
	cookie.SetSecure (inSecure);
	cookie.SetMaxAge (inMaxAge);
	cookie.SetHttpOnly (inHTTPOnly);

	if (cookie.IsValid() && !IsCookieSet (inName))
		return GetHeaders().SetHeaderValue (HEADER_SET_COOKIE, cookie.ToString (inAlwaysUseExpires), false);

	return false;
}


bool VHTTPResponse::DropCookie (const XBOX::VString& inName)
{
	return GetHeaders().DropCookie (inName);
}


IVirtualHost *VHTTPResponse::GetVirtualHost()
{
	if ((NULL == fVirtualHost) && (NULL != fRequest))
	{
		//	YT 14-Jun-2001 - TestCase HttpMsg_Version05
		//	HTTP/1.0: Host header is optionnal but we need it to determine VirtualHost, so let's emulate it.
		if ((fRequest->GetHTTPVersion() == VERSION_1_0) && fRequest->GetHost().IsEmpty())
		{
#if WITH_DEPRECATED_IPV4_API			
			HTTPServerTools::MakeIPv4String (fEndPoint->GetIPv4HostOrder(), fRequest->fHost);
#else
			fEndPoint->GetIP (fRequest->fHost);
#endif
			fRequest->fHost.AppendUniChar (CHAR_COLON);
			fRequest->fHost.AppendLong (fEndPoint->GetPort());
		}

		fVirtualHost = fHTTPServer->GetVirtualHostManager()->RetainMatchingVirtualHost (*fRequest);
#if HTTP_SERVER_USE_PROJECT_PATTERNS
		if (NULL != fVirtualHost)
			_UpdateRequestURL (fVirtualHost->GetProject()->GetSettings()->GetProjectPattern());	// YT 26-Nov-2010 - ACI0068972
#endif

		if (NULL != fVirtualHost)
		{
			/*
			 *	Set VHTTPMessage.fBody's charset using project's default charset.
			 */
			XBOX::CharSet charset = fVirtualHost->GetSettings()->GetDefaultCharSet();
			if (!GetBody().IsReadOnly() && !GetBody().IsWriteOnly())
				GetBody().SetCharSet (charset);
		}
	}

	return fVirtualHost;
}


bool VHTTPResponse::IsSSL() const
{
	if (NULL != fEndPoint)
		return fEndPoint->IsSSL();

	return false;
}


void VHTTPResponse::SetWantedAuthRealm (const XBOX::VString& inValue)
{
	dynamic_cast<VAuthenticationInfos *>(fRequest->GetAuthenticationInfos())->SetRealm (inValue);
}


void VHTTPResponse::GetWantedAuthRealm (XBOX::VString& outValue) const
{
	fRequest->GetAuthenticationInfos()->GetRealm (outValue);
}


sLONG VHTTPResponse::GetRawSocket() const
{
	if (NULL != fEndPoint)
		return fEndPoint->GetRawSocket();

	return -1;
}


bool VHTTPResponse::_UpdateRequestURL (const XBOX::VString& inProjectPattern)
{
	// URL Start with project pattern ?
	if (!inProjectPattern.IsEmpty())
	{
		_RemoveProjectPatternFromURL (inProjectPattern, fRequest->fURL);
		_RemoveProjectPatternFromURL (inProjectPattern, fRequest->fURLPath);
		_RemoveProjectPatternFromURL (inProjectPattern, fRequest->fRawURL);

		return true;
	}

	return false;
}


inline
void VHTTPResponse::AllowCompression (bool inValue, sLONG inMinThreshold, sLONG inMaxThreshold)
{
	fCanCompressBody = inValue;

	if ((inMinThreshold > 0) && ((inMinThreshold < inMaxThreshold) || (inMaxThreshold == -1)))
		fMinCompressionThreshold = inMinThreshold;

	if ((inMaxThreshold > 0) && (inMinThreshold < inMaxThreshold))
		fMaxCompressionThreshold = inMaxThreshold;
}

#if WITH_DEPRECATED_IPV4_API
inline
IP4 /*done*/ VHTTPResponse::GetIPv4() const
{
	if (NULL != fEndPoint)
		return fEndPoint->GetIPv4HostOrder();

	return 0;
}
#else
inline
void VHTTPResponse::GetIP (XBOX::VString& outIP) const
{
	if (NULL != fEndPoint)
		fEndPoint->GetIP (outIP);
	else
		outIP.Clear();
}
#endif


bool VHTTPResponse::_InitResponseHeaderFromBuffer (void *inBuffer, XBOX::VSize inBufferSize, XBOX::VSize& outBodyOffset)
{
	outBodyOffset = 0;

	if ((inBuffer == NULL) || (inBufferSize == 0))
		return false;

	char *		buffer = (char *)inBuffer;
	char *		bufferEnd = buffer + inBufferSize;

	if ((inBufferSize > 8)
		&& ((buffer[0] == 'H') || (buffer[0] == 'h'))
		&& ((buffer[1] == 'T') || (buffer[1] == 't'))
		&& ((buffer[2] == 'T') || (buffer[2] == 't'))
		&& ((buffer[3] == 'P') || (buffer[3] == 'p'))
		&& ((buffer[4] == '/')))
	{
		// there is a first line to be parsed:
		// typically: HTTP/1.1 200 Ok
		if ((buffer[5] == '1') && (buffer[6] == '.') && (buffer[7] == '1'))
		{
			buffer+=8;
			fHTTPVersion = VERSION_1_1;
		}
		else if ((buffer[5] == '1') && (buffer[6] == '.') && (buffer[7] == '0'))
		{
			buffer+=8;
			fHTTPVersion = VERSION_1_0;
		}

		while ((buffer < bufferEnd) && std::isspace (*buffer))
			++buffer;

		if ((buffer + 3) < bufferEnd)
		{
			// parse status code
			if ((buffer[0] >= '0') && (buffer[0] <= '9')
				&& (buffer[1] >= '0') && (buffer[1] <= '9')
				&& (buffer[2] >= '0') && (buffer[2] <= '9'))
			{
				fResponseStatusCode = HTTPStatusCode(((buffer[0] - '0') * 100) + ((buffer[1] - '0') * 10) + (buffer[2] - '0'));
				buffer +=3;
			}
		}

		const sLONG HTTP_CRLF_SIZE = (sizeof (HTTP_CRLF) - 1);
		char *endLinePtr = strstr (buffer, HTTP_CRLF);
		if ((endLinePtr != NULL) && ((endLinePtr + HTTP_CRLF_SIZE) <= bufferEnd))
			buffer = (endLinePtr + HTTP_CRLF_SIZE);

		outBodyOffset += (buffer - (char *)inBuffer);
	}

	const sLONG	HTTP_CRLFCRLF_SIZE = (sizeof (HTTP_CRLFCRLF) - 1);
	char *		startHeaderPtr = buffer;
	char *		endHeaderPtr = strstr (startHeaderPtr, HTTP_CRLFCRLF);
	if ((endHeaderPtr != NULL) && ((endHeaderPtr + HTTP_CRLFCRLF_SIZE) <= bufferEnd))
	{
		XBOX::VString headerString;
		headerString.FromBlock (buffer, endHeaderPtr - startHeaderPtr, XBOX::VTC_UTF_8);
		GetResponseHeader().FromString (headerString);
		buffer = (endHeaderPtr + HTTP_CRLFCRLF_SIZE);
		outBodyOffset += (buffer - startHeaderPtr);
	}

	return (outBodyOffset != 0);
}
