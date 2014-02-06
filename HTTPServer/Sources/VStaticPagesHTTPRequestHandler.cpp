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
#include "VStaticPagesHTTPRequestHandler.h"


//--------------------------------------------------------------------------------------------------


XBOX::VError VStaticPagesHTTPRequestHandler::GetPatterns (XBOX::VectorOfVString *outPatterns) const
{
	if (outPatterns)
		outPatterns->clear();

	return VE_OK;
}


XBOX::VError VStaticPagesHTTPRequestHandler::HandleRequest (IHTTPResponse *ioResponse)
{
	VHTTPResponse *	httpResponse = dynamic_cast<VHTTPResponse *>(ioResponse);

	if (NULL == httpResponse)
		return VHTTPServer::ThrowError (VE_HTTP_PROTOCOL_INTERNAL_SERVER_ERROR, STRING_ERROR_INVALID_PARAMETER);

	VVirtualHost *	virtualHost = dynamic_cast<VVirtualHost *>(httpResponse->GetVirtualHost());

	if (NULL == virtualHost)
		return VE_HTTP_PROTOCOL_NOT_FOUND;

	XBOX::VError			error = VE_OK;
	XBOX::VFilePath			filePath;
	const XBOX::VString		URL (httpResponse->GetRequestURLPath());
	const HTTPRequestMethod	method = httpResponse->GetRequestMethod();

	if ((method == HTTP_GET) || (method == HTTP_HEAD))
	{
		VCacheManager *	cacheManager = NULL;
		VCachedObject *	cachedObject = NULL;
		bool			loadPageFromDisk = true;
		XBOX::VString	acceptEncodingHeaderValue;
		XBOX::VTime		ifModifiedDate;
		XBOX::VTime		ifUnmodifiedDate;
		bool			ifModifiedField = false;
		bool			ifUnmodifiedField = false;
		bool			isPartialContent = false;

#if HTTP_SERVER_HANDLE_PARTIAL_CONTENT
		XBOX::VString	rangeHeaderValue;
		XBOX::VIndex	startByte = 0;
		XBOX::VIndex	endByte = 0;

		if (httpResponse->GetRequestHeader().GetHeaderValue (STRING_HEADER_RANGE, rangeHeaderValue))
		{
			std::vector< std::pair<XBOX::VIndex, xbox::VIndex> > indexes;

			if (!HTTPProtocol::ParseRangeHeaderValue (rangeHeaderValue, indexes))
				return VE_HTTP_PROTOCOL_REQUESTED_RANGE_NOT_SATISFIABLE;

			if (!indexes.empty())
			{
				isPartialContent = true;

				/*
				 *	Handle only Single Range-Byte (multiple will be supported later)
				 */
				startByte = indexes[0].first;
				endByte = indexes[0].second;
			}
		}
#endif

		if (!isPartialContent)
		{
			httpResponse->GetRequestHeader().GetHeaderValue (STRING_HEADER_ACCEPT_ENCODING, acceptEncodingHeaderValue);

			if ((ifModifiedField = httpResponse->GetIfModifiedSinceHeader (ifModifiedDate)) == false)
				ifUnmodifiedField = httpResponse->GetIfUnmodifiedSinceHeader (ifUnmodifiedDate); // YT 05-Oct-2012 - WAK0072943 - Oops silly typo was: GetIfModifiedSinceHeader

			cacheManager = virtualHost->GetProject()->GetHTTPServer()->GetCacheManager();
			if ((NULL != cacheManager) && cacheManager->GetEnableDataCache())
			{
	#if HTTP_SERVER_USE_SYSTEM_NOTIFICATIONS
				cachedObject = cacheManager->RetainPageFromCache(URL, virtualHost->GetUUIDString(), acceptEncodingHeaderValue);
	#else
				cachedObject = cacheManager->RetainPageFromCache(URL, virtualHost->GetUUIDString(), acceptEncodingHeaderValue, ifModifiedField ? &ifModifiedDate : NULL);
	#endif
			}

			if (NULL != cachedObject)
			{
				if (ifModifiedField && (cachedObject->GetLastModified() == ifModifiedDate))
				{
					loadPageFromDisk = false;
					/* Send 304 - Not Modified */
					error = VE_HTTP_PROTOCOL_NOT_MODIFIED;
					httpResponse->AddResponseHeader (STRING_HEADER_CONTENT_TYPE, cachedObject->GetMimeType());
				}
				else if (ifUnmodifiedField && (cachedObject->GetLastModified() != ifUnmodifiedDate))
				{
					loadPageFromDisk = false;
					/* Send 412 - Precondition Failed */
					error = VE_HTTP_PROTOCOL_PRECONDITION_FAILED;
				}
				else
				{
					void *cacheBuffer = NULL;
					XBOX::VSize cacheBufferSize = 0;
					cachedObject->GetCachedContent (&cacheBuffer, cacheBufferSize);
					if (cacheBuffer)
					{
						/*
							Retrieve compression infos & content-type from cache to set the correct headers
						*/
						if (cachedObject->IsCompressed())
						{
							XBOX::VString contentEncoding;
							HTTPProtocol::GetEncodingMethodName (cachedObject->GetCompressionMode(), contentEncoding);
							httpResponse->AddResponseHeader (STRING_HEADER_VARY, STRING_HEADER_CONTENT_ENCODING, false);
							httpResponse->AddResponseHeader (STRING_HEADER_CONTENT_ENCODING, contentEncoding);
						}

						if (!cachedObject->GetMimeType().IsEmpty())
							httpResponse->AddResponseHeader (STRING_HEADER_CONTENT_TYPE, cachedObject->GetMimeType());

						sLONG		maxAge = cachedObject->GetMaxAge();
						XBOX::VTime	expirationDate;

						if (maxAge > 0)
						{
							XBOX::VString	string;

							string.FromCString ("max-age=");
							string.AppendLong (maxAge);
							httpResponse->AddResponseHeader (STRING_HEADER_CACHE_CONTROL, string);
							httpResponse->AddResponseHeader (STRING_HEADER_AGE, cachedObject->GetAge());
						}
						else if (cachedObject->GetExpirationDate(expirationDate))
						{
							httpResponse->AddResponseHeader (STRING_HEADER_EXPIRES, expirationDate);
						}
						else
						{
							XBOX::VString lastModifiedString;
							HTTPProtocol::MakeRFC822GMTDateString (cachedObject->GetLastModified(), lastModifiedString);
							httpResponse->AddResponseHeader (STRING_HEADER_LAST_MODIFIED, lastModifiedString);
						}

						if (method == HTTP_GET)
						{
							/*
							 *	Do NOT dispose body in ~VHTTPMessage(), the data belongs to VCacheManager
							 */
							httpResponse->SetDisposeBody (false);

							/*
							 *	YT 03-Oct-2012 - WAK0078597 & WAK0078598
							 *	SetIsStaticResource to avoid charset parameter to be automatically set in Content-Type response header
							 */
							httpResponse->SetUseDefaultCharset (false);
							httpResponse->GetResponseBody().SetDataPtr (cacheBuffer, cacheBufferSize);
						}
						else if(method == HTTP_HEAD)
						{
							httpResponse->SetContentLengthHeader (cachedObject->GetSize());
						}

						loadPageFromDisk = false;
					}
				}

				XBOX::QuickReleaseRefCountable (cachedObject);
			}
		}
		else
		{
			loadPageFromDisk = isPartialContent;
		}
		
		if (loadPageFromDisk && (XBOX::VE_OK == error))
		{
			XBOX::VString	locationPath;
			sLONG8			fileSize = 0;

			error = virtualHost->GetFilePathFromURL (URL, locationPath);
			if (XBOX::VE_OK == error)
				filePath.FromFullPath (locationPath);

			if ((XBOX::VE_OK == error) && filePath.IsFile())
			{
				XBOX::VFile *file = new XBOX::VFile (filePath);

				if (NULL != file)
				{
					if (file->Exists())
					{
						XBOX::VTime modificationDate;
						file->GetTimeAttributes (&modificationDate);

						if (ifModifiedField || ifUnmodifiedField)
						{
							if (ifModifiedField && (modificationDate == ifModifiedDate))
							{
								/* Send 304 - Not Modified */
								error = VE_HTTP_PROTOCOL_NOT_MODIFIED;
							}
							else if (ifUnmodifiedField && (modificationDate != ifUnmodifiedDate))
							{
								/* Send 412 - Precondition Failed */
								error = VE_HTTP_PROTOCOL_PRECONDITION_FAILED;
							}
						}

						if (XBOX::VE_OK == error)
						{
							if (NULL != cacheManager)
							{
								VHTTPResource *resource = cacheManager->FindMatchingResource (URL);
								if (NULL != resource)
								{
									XBOX::VTime expires (resource->GetExpires());

									if (IsVTimeValid (expires))
									{
										httpResponse->AddResponseHeader (STRING_HEADER_EXPIRES, expires);
									}
									else if ((resource->GetLifeTime() > 0) && cacheManager->GetEnableDataCache())
									{
										XBOX::VString	string;

										string.FromCString ("max-age=");
										string.AppendLong (resource->GetLifeTime());
										httpResponse->AddResponseHeader (STRING_HEADER_CACHE_CONTROL, string);
										httpResponse->AddResponseHeader (STRING_HEADER_AGE, 0);
									}
								}
							}

							XBOX::VString lastModifiedString;
							HTTPProtocol::MakeRFC822GMTDateString (modificationDate, lastModifiedString);
							httpResponse->AddResponseHeader (STRING_HEADER_LAST_MODIFIED, lastModifiedString);
							
							/*
							 *	YT 03-Oct-2012 - WAK0078597 & WAK0078598
							 *	SetIsStaticResource to avoid charset parameter to be automatically set in Content-Type response header
							 */
							httpResponse->SetUseDefaultCharset (false);

							XBOX::VFileStream fileStream (file);
							if (testAssert (fileStream.OpenReading() == VE_OK))
							{
								if ((method == HTTP_GET) || (method == HTTP_HEAD))
								{
									const sLONG8	MAX_FILE_SIZE = 10 * 1024 * 1024; //kMAX_sLONG
									sLONG8			chunkSize = 0;
									bool			chunkSizeOK = false;

									file->GetSize (&fileSize);

#if HTTP_SERVER_HANDLE_PARTIAL_CONTENT
									if (isPartialContent && (fileSize > 0))
									{
										if (!HTTPProtocol::AcceptUpdateRangeIndexes (startByte, endByte, fileSize))
											//error = VE_HTTP_PROTOCOL_REQUESTED_RANGE_NOT_SATISFIABLE;
											isPartialContent = false; // YT 11-Jun-2013 - WAK0082160 - Do NOT send 416 anymore, simply ignore partial-content request
									}

									if (isPartialContent && (endByte >= fileSize))
										endByte = (fileSize - 1);	// Not sure we have to be so strict... error = VE_HTTP_PROTOCOL_REQUESTED_RANGE_NOT_SATISFIABLE;

									chunkSize = isPartialContent ? ((endByte - startByte) + 1): fileSize;
									chunkSizeOK = isPartialContent ? ((chunkSize >= 0) && (chunkSize < kMAX_sLONG)) : ((chunkSize >= 0) && (chunkSize < MAX_FILE_SIZE));
#else
									chunkSize = fileSize;
									chunkSizeOK = ((chunkSize >= 0) && (chunkSize < MAX_FILE_SIZE));
#endif
									if ((error == XBOX::VE_OK) && (chunkSizeOK))
									{
										/* HEAD: Just retrieve the file size and set the Content-Length header */
										if (method == HTTP_HEAD)
										{
											httpResponse->SetCacheBodyMessage (false);
#if HTTP_SERVER_HANDLE_PARTIAL_CONTENT
											if (isPartialContent)
											{
												XBOX::VString contentRangeValue;
												HTTPProtocol::MakeRangeHeaderValue (startByte, endByte, fileSize, contentRangeValue);
												httpResponse->SetResponseStatusCode (HTTP_PARTIAL_CONTENT);
												httpResponse->AddResponseHeader (STRING_HEADER_ACCEPT_RANGES, STRING_HEADER_VALUE_BYTES);
												httpResponse->AddResponseHeader (STRING_HEADER_CONTENT_RANGE, contentRangeValue);
												httpResponse->SetContentLengthHeader (chunkSize);
											}
											else
#endif
											{
												httpResponse->SetContentLengthHeader (fileSize);
											}
										}
										/* GET: Standard Stuff */
										else
										{
											if (chunkSize > 0)
											{
												XBOX::VSize bufferSize = (XBOX::VSize)chunkSize;
												void *buffer = XBOX::vMalloc (bufferSize, 0);
												if (NULL != buffer)
												{
													if (isPartialContent)
														fileStream.SetPos (startByte);

													fileStream.GetBytes (buffer, &bufferSize);
													error = httpResponse->SetResponseBody (buffer, bufferSize);
													XBOX::vFree (buffer);
													buffer = NULL;
#if HTTP_SERVER_HANDLE_PARTIAL_CONTENT
													if (isPartialContent)
													{
														XBOX::VString contentRangeValue;
														HTTPProtocol::MakeRangeHeaderValue (startByte, endByte, fileSize, contentRangeValue);
														httpResponse->SetResponseStatusCode (HTTP_PARTIAL_CONTENT);
														httpResponse->AddResponseHeader (STRING_HEADER_ACCEPT_RANGES, STRING_HEADER_VALUE_BYTES);
														httpResponse->AddResponseHeader (STRING_HEADER_CONTENT_RANGE, contentRangeValue);
														httpResponse->SetContentLengthHeader (chunkSize);
													}
													else
#endif
													{

														/* Set the page cachable */
														httpResponse->SetCacheBodyMessage (true);
													}

													/* Set the body compressible */
													httpResponse->AllowCompression (true);
												}
												else
												{
													httpResponse->SetCacheBodyMessage (false);
													httpResponse->AllowCompression (false);
													httpResponse->SetContentLengthHeader (fileSize);
													error = httpResponse->SetFileToSend (file);
												}
											}
											else
											{
												httpResponse->SetContentLengthHeader (chunkSize);
											}
										}
									}
									else if (XBOX::VE_OK == error)
									{
										httpResponse->SetCacheBodyMessage (false);
										httpResponse->AllowCompression (false);
										httpResponse->SetContentLengthHeader (fileSize);
										error = httpResponse->SetFileToSend (file);
									}
								}

								fileStream.CloseReading();
							}
						}
					}
					else
					{
						error = VE_HTTP_PROTOCOL_NOT_FOUND;
					}
				}
				else
				{
					error = VHTTPServer::ThrowError (VE_HTTP_PROTOCOL_INTERNAL_SERVER_ERROR, STRING_ERROR_ALLOCATION_FAILED);
				}

				XBOX::QuickReleaseRefCountable (file);
			}
			/* Set Location Header (when applicable) */
			else if (error == VE_HTTP_PROTOCOL_FOUND)
			{
				httpResponse->AddResponseHeader (STRING_HEADER_LOCATION, locationPath);
			}
			else if (error != VE_HTTP_PROTOCOL_FORBIDDEN)
			{
				error = VE_HTTP_PROTOCOL_NOT_FOUND;
			}
		}

#if HTTP_SERVER_HANDLE_PARTIAL_CONTENT
		/* Set Content-Range (when applicable) */
		if (error == VE_HTTP_PROTOCOL_REQUESTED_RANGE_NOT_SATISFIABLE)
		{
			sLONG8 fileSize = 0;
			HTTPServerTools::GetFileInfos (filePath, NULL, &fileSize);

			if (fileSize > 0)
			{
				XBOX::VString contentRangeValue;
				HTTPProtocol::MakeRangeHeaderValue (-1, -1, fileSize, contentRangeValue);
				httpResponse->AddResponseHeader (STRING_HEADER_CONTENT_RANGE, contentRangeValue);
			}
		}
#endif
	}
	/* Temp for Lot 1 */ 
	else if (	method == HTTP_PUT ||
				method == HTTP_POST ||
				method == HTTP_DELETE ||
				method == HTTP_OPTIONS)
	{
		/* Send 405 - Method Not Allowed */
		error = VE_HTTP_PROTOCOL_METHOD_NOT_ALLOWED;
		httpResponse->AddResponseHeader (STRING_HEADER_ALLOW, CVSTR ("GET, HEAD, TRACE"));
	}
	else
	{
		/* Send 501 - Not Implemented */
		error = VE_HTTP_PROTOCOL_NOT_IMPLEMENTED;
	}

	if ((error != VE_HTTP_PROTOCOL_NOT_FOUND) && filePath.IsFile() && !httpResponse->IsResponseHeaderSet (HEADER_CONTENT_TYPE))
	{
		XBOX::VString contentType;
		XBOX::VString extension;
		filePath.GetExtension (extension);
		VMimeTypeManager::FindContentType (extension, contentType);
		httpResponse->AddResponseHeader (STRING_HEADER_CONTENT_TYPE, contentType);
	}

	return error;
}
