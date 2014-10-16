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


const uLONG DEFAULT_KEEP_ALIVE_TIMEOUT = 3000;		// 3 secs
const sLONG DEFAULT_KEEP_ALIVE_MAX_REQUESTS = 1000;


VHTTPServerSession::VHTTPServerSession (VHTTPResponse* inEndPoint)
: fStartTime (0)
, fKeepAliveTimeOut (DEFAULT_KEEP_ALIVE_TIMEOUT)
, fHTTPResponse (NULL)
, fRequestCount (0)
, fKeepAlive (true)
, fMaxRequest (DEFAULT_KEEP_ALIVE_MAX_REQUESTS)
{
	fHTTPResponse = dynamic_cast<VHTTPResponse *>(inEndPoint);
	fStartTime = XBOX::VSystem::GetCurrentTime();
}


VHTTPServerSession::~VHTTPServerSession()
{
	Close();
}


void VHTTPServerSession::Reset()
{
	fStartTime = XBOX::VSystem::GetCurrentTime();

	if (NULL != fHTTPResponse)
		fHTTPResponse->Reset();	// YT 31-Aug-2010 - Use VHTTPResponse::Reset() instead of VHTTPMessage::Clear() to free VHTTPRequest object members too...
}


XBOX::VError VHTTPServerSession::Close()
{
	if (NULL != fHTTPResponse)
	{
		if (NULL != fHTTPResponse->GetEndPoint())
			fHTTPResponse->GetEndPoint()->Close();
		delete fHTTPResponse;
		fHTTPResponse = NULL;
	}

	return XBOX::VE_OK;
}


XBOX::VError VHTTPServerSession::HandleTRACE()
{
	if (NULL == fHTTPResponse)
		return VHTTPServer::ThrowError (VE_HTTP_PROTOCOL_INTERNAL_SERVER_ERROR, STRING_ERROR_INVALID_PARAMETER);

	XBOX::VString bodyString;

	// Build Header
	fHTTPResponse->AddResponseHeader (STRING_HEADER_PRAGMA, STRING_HEADER_VALUE_NO_CACHE);
	fHTTPResponse->AddResponseHeader (STRING_HEADER_CONTENT_TYPE, STRING_CONTENT_TYPE_MESSAGE);
	fHTTPResponse->SetExpiresHeader (GMT_NOW);

	// Build Body-message
	bodyString.FromString (fHTTPResponse->GetRequest().GetRequestLine());
	bodyString.AppendCString (HTTP_CRLF);

	XBOX::VNameValueCollection requestHeader = fHTTPResponse->GetRequestHeader().GetHeaderList();
	for (XBOX::VNameValueCollection::Iterator it = requestHeader.begin(); it != requestHeader.end(); ++it)
	{
		if ((*it).first.GetLength() > 0)
		{
			bodyString.AppendString ((*it).first);
			bodyString.AppendUniChar (CHAR_COLON);
			bodyString.AppendUniChar (CHAR_SPACE);
			bodyString.AppendString ((*it).second);
			bodyString.AppendCString (HTTP_CRLF);
		}
	}

	XBOX::StStringConverter<char> buffer (bodyString, XBOX::VTC_UTF_8);
	fHTTPResponse->SetResponseBody (buffer.GetCPointer(), buffer.GetSize());

	return XBOX::VE_OK;
}


XBOX::VError VHTTPServerSession::HandleRequest()
{
	VHTTPServer *httpServer = dynamic_cast<VHTTPServer *>(fHTTPResponse->GetHTTPServer());

	if ((NULL == fHTTPResponse) || (NULL == httpServer))
		return VHTTPServer::ThrowError (VE_HTTP_PROTOCOL_INTERNAL_SERVER_ERROR, STRING_ERROR_INVALID_PARAMETER);

	/* Start parsing request */
	uLONG			startTime = XBOX::VSystem::GetCurrentTime();
	XBOX::VError	error = XBOX::VE_OK;

	// Reinit Session infos
	if (fRequestCount > 0)
		Reset();
	
#if HTTP_SERVER_VERBOSE_MODE && HTTP_SERVER_DETAILED_OPERATIONS_TIMES
	VDebugTimer readRequestTimer;
#endif

	error = fHTTPResponse->ReadRequestFromEndPoint (fKeepAlive ? fKeepAliveTimeOut : 0); 

#if HTTP_SERVER_VERBOSE_MODE && HTTP_SERVER_DETAILED_OPERATIONS_TIMES
	sLONG8 readRequestTime = readRequestTimer.GetElapsedTime();
#endif

	// close session and exit
	if (VE_HTTP_PROTOCOL_REQUEST_TIMEOUT == error)
		return error;

	/* A network error occur and Parsing did not complete, return error, other else try to answer */
	if ((XBOX::VE_OK != error) && (!fHTTPResponse->GetRequest().IsParsingComplete()))
		return error;

	if (XBOX::VE_OK == error)
		error = fHTTPResponse->GetRequest().GetParsingError();

	VVirtualHost *	virtualHost = dynamic_cast<VVirtualHost *>(fHTTPResponse->GetVirtualHost());

	if ((XBOX::VE_OK == error) && (NULL == virtualHost))
		error = VE_HTTP_PROTOCOL_CONFLICT;

	/* Uses Keep-Alive Project Settings */
	if ((0 == fRequestCount) && (NULL != virtualHost))
	{
#if HTTP_SERVER_GLOBAL_SETTINGS
		fKeepAlive = httpServer->GetSettings()->GetEnableKeepAlive();
		fMaxRequest = httpServer->GetSettings()->GetKeepAliveMaxConnections();
		fKeepAliveTimeOut = httpServer->GetSettings()->GetKeepAliveTimeout() * 1000;
#else
		fKeepAlive = virtualHost->GetProject()->GetSettings()->GetEnableKeepAlive();
		fMaxRequest = virtualHost->GetProject()->GetSettings()->GetKeepAliveMaxConnections();
		fKeepAliveTimeOut = virtualHost->GetProject()->GetSettings()->GetKeepAliveTimeout() * 1000;
#endif
	}

#if LOG_IN_CONSOLE
	::DebugMsg ("* Request: %S\n", &fHTTPResponse->GetRequestURLPath());
	::DebugMsg ("\tElapsed time parsing request: %d\n", XBOX::VSystem::GetCurrentTime() - startTime);
#endif

	IRequestLogger *		requestLogger = httpServer->GetRequestLogger();
	XBOX::VString			headerValue;

#if LOG_IN_CONSOLE || (HTTP_SERVER_VERBOSE_MODE && HTTP_SERVER_DETAILED_OPERATIONS_TIMES)
	VDebugTimer registeringRequestTimer;
#endif

	if (XBOX::VE_OK == error)
	{
		if (NULL != virtualHost)
		{
			if (!virtualHost->GetProject()->GetAcceptIncommingRequests() || !virtualHost->IsAvailable())
				error = VE_HTTP_PROTOCOL_SERVICE_UNAVAILABLE;

			if (XBOX::VE_OK == error)
			{
				virtualHost->GetProject()->IncreaseHitsCount();
				virtualHost->GetProject()->RegisterRequest (fHTTPResponse);
			}
		}
		else
		{
			/*
			 *	Send 409 - Conflict when we were unable to determine the proper VirtualHost
			 *	May happens when, for example, 2 or many VirtualHosts use the same IPAddress/Port pair and doesn't use hostnames
			 */
			error = VE_HTTP_PROTOCOL_CONFLICT;
		}
	}

#if LOG_IN_CONSOLE
	registeringRequestTimer.DebugMsg ("\tRegistering Request");
#endif
#if HTTP_SERVER_VERBOSE_MODE && HTTP_SERVER_DETAILED_OPERATIONS_TIMES
	sLONG8 registeringRequestTime = registeringRequestTimer.GetElapsedTime();
#endif
	
	HTTPRequestMethod requestMethod = fHTTPResponse->GetRequestMethod();

#if HTTP_SERVER_VERBOSE_MODE && HTTP_SERVER_DETAILED_OPERATIONS_TIMES
	VDebugTimer checkingAuthTimer;
#endif

	// Verify Authentication
	XBOX::VString				uagSessionID;
	IAuthenticationDelegate *	authDelegate = NULL;

	if (XBOX::VE_OK == error)
	{
		authDelegate = (NULL != virtualHost) ? virtualHost->GetProject()->GetAuthenticationDelegate() : NULL;

		// TODO: Move the following code into CheckAndValidateAuthentication... Later
		// We have a SessionID, no need to validate Authorization HTTP Header, just retrieve the corresponding CUAGSession Object
		if ((NULL != authDelegate) && fHTTPResponse->GetRequest().GetCookie (STRING_UAG_SESSION_ID, uagSessionID) && !uagSessionID.IsEmpty())
		{
			CUAGSession *	uagSession = (NULL != authDelegate) ? authDelegate->CopyUAGSession (uagSessionID) : NULL;
			if (NULL != uagSession)
			{
				/*
				CUAGSession* copySession = uagSession->Clone();
				dynamic_cast <VAuthenticationInfos *>(fHTTPResponse->GetRequest().GetAuthenticationInfos())->SetUAGSession (copySession);
				QuickReleaseRefCountable(copySession);
				*/
				IAuthenticationInfos* authInfo = fHTTPResponse->GetRequest().GetAuthenticationInfos();
				VString username, sessionUsername;
				authInfo->GetUserName(username);
				bool keepsession = true;
				HTTPAuthenticationMethod authMethod = authInfo->GetAuthenticationMethod();
				if (authMethod == AUTH_BASIC || authMethod == AUTH_DIGEST)
				{
					CUAGUser* user = uagSession->RetainUser();
					keepsession = false;
					if (user != nil)
					{
						user->GetName(sessionUsername);
						if (sessionUsername == username)
							keepsession = true;
					}
					else
					{
						if (username.IsEmpty())
							keepsession = true;
					}
					QuickReleaseRefCountable(user);
				}
				if (keepsession)
					dynamic_cast <VAuthenticationInfos *>(authInfo)->SetUAGSession (uagSession);
				else
				{
					ReleaseRefCountable(&uagSession);
				}
			}

			if (NULL == uagSession)
				uagSessionID.Clear();

			XBOX::QuickReleaseRefCountable (uagSession);
		}

		if ((NULL != virtualHost) && uagSessionID.IsEmpty())
			error = virtualHost->GetProject()->GetAuthenticationManager()->CheckAndValidateAuthentication (fHTTPResponse);
	}

#if HTTP_SERVER_VERBOSE_MODE && HTTP_SERVER_DETAILED_OPERATIONS_TIMES
	sLONG8 checkingAuthTime = checkingAuthTimer.GetElapsedTime();
#endif

#if HTTP_SERVER_VERBOSE_MODE && HTTP_SERVER_DETAILED_OPERATIONS_TIMES
	VDebugTimer handlingRequestTimer;
#endif
	if (XBOX::VE_OK == error)
	{
		if (HTTP_TRACE == requestMethod)
		{
			error = HandleTRACE();
		}
		else
		{
			/* Temp for Lot 1 */
			if ((HTTP_POST == requestMethod) || (HTTP_PUT == requestMethod))
			{
				if (fHTTPResponse->GetRequest().GetHTTPHeaders().GetHeaderValue (STRING_HEADER_TRANSFER_ENCODING, headerValue))
					if (HTTPServerTools::EqualASCIIVString (headerValue, STRING_HEADER_VALUE_CHUNKED))
						error = VE_HTTP_PROTOCOL_NOT_IMPLEMENTED;
			}

			if (XBOX::VE_OK == error)
			{
				bool	bIsWebSocket;

				// To determine if it is a WebSocket connection request, just check the header for an entry "Upgrade" with "websocket" value.
				
				bIsWebSocket 
					= fHTTPResponse->GetRequest().GetHTTPHeaders().GetHeaderValue(STRING_HEADER_UPGRADE, headerValue) 
					&& HTTPServerTools::EqualASCIIVString(headerValue, STRING_HEADER_VALUE_WEBSOCKET);

				/* Call Pre Process method (when applicable, in particular bypass if it is a WebSocket connection request). */

				if (!bIsWebSocket)
				{
					IPreProcessingHandler *preProcessingHandler = virtualHost->GetProject()->RetainPreProcessingHandler (fHTTPResponse->GetRequestURLPath());

					if (NULL != preProcessingHandler)
					{
						error = preProcessingHandler->HandleRequest (fHTTPResponse);
						XBOX::QuickReleaseRefCountable (preProcessingHandler);
					}
				}

				IHTTPRequestHandler *	handler = NULL;

				if (XBOX::VE_OK == error)
				{		
					if (bIsWebSocket) {

						if (fHTTPResponse->GetRequestMethod() != XBOX::HTTP_GET
						|| fHTTPResponse->GetRequestHTTPVersion() != VERSION_1_1
						|| XBOX::VWebSocketListener::IsRequestOk(&fHTTPResponse->GetRequestHeader())) {

							// Do some basic checking of the request. 
							// In particular, check that all required header entries are present.
							// Validity of the request will be further checked later.

							error = VE_HTTP_PROTOCOL_BAD_REQUEST;

						} else {

							// Find and call the registered handler if any.
							// Query is part of the resource name (path).

							XBOX::VString		resourceName		= fHTTPResponse->GetRequest().GetURL();
							IWebSocketHandler	*webSocketHandler	= virtualHost->GetProject()->RetainMatchingWebSocketHandler(resourceName);

							if (webSocketHandler != NULL && webSocketHandler->GetEnable()) 
							
								error = webSocketHandler->HandleRequest(fHTTPResponse);

							else {

								// If a handler isn't found, don't consider as "forbidden" but as not (no handler) "available". 
							
								error = VE_HTTP_PROTOCOL_SERVICE_UNAVAILABLE;

							}

							XBOX::ReleaseRefCountable<IWebSocketHandler>(&webSocketHandler);

						}

					}

					if ((XBOX::VE_OK == error) && !bIsWebSocket)
					{
						/*
						 *	Call RequestHandler
						 */
						if (XBOX::VE_OK == error)
						{
							handler = virtualHost->GetProject()->RetainMatchingHTTPRequestHandler (fHTTPResponse->GetRequestURLPath());

							if ((NULL == handler) && virtualHost->GetProject()->GetDefaultRequestHandler())
							{
								handler = XBOX::RetainRefCountable (virtualHost->GetProject()->GetDefaultRequestHandler());
							}

							if (NULL != handler)
							{
								if (handler->GetEnable())
								{
									error = handler->HandleRequest (fHTTPResponse);
								}
								else
								{
									error = VE_HTTP_PROTOCOL_SERVICE_UNAVAILABLE;
								}
							}
							else
							{
								error = VHTTPServer::ThrowError (VE_HTTP_PROTOCOL_INTERNAL_SERVER_ERROR, STRING_ERROR_INVALID_REQUEST_HANDLER);
							}
						}

						/* Call PostProcess method (when applicable) */
						if ((XBOX::VE_OK == error) && !bIsWebSocket && (NULL != dynamic_cast<VVirtualHost *>(fHTTPResponse->GetVirtualHost()))) // YT 12-Nov-2013 - ACI0084838
						{
							IPostProcessingHandler *postProcessingHandler = dynamic_cast<VVirtualHost *>(fHTTPResponse->GetVirtualHost())->GetProject()->RetainPostProcessingHandler (fHTTPResponse->GetRequestURLPath());
					
							if (NULL != postProcessingHandler)
							{
								error = postProcessingHandler->HandleRequest (fHTTPResponse);
								XBOX::QuickReleaseRefCountable (postProcessingHandler);
							}
						}
					}
				}

				/* Call ErrorProcess method (when applicable) */
				if ((XBOX::VE_OK != error) && (NULL != virtualHost->GetProject()->GetErrorProcessingHandler()))
				{
					error = virtualHost->GetProject()->GetErrorProcessingHandler()->ProcessError (error, handler, fHTTPResponse);					
				}

				XBOX::QuickReleaseRefCountable (handler);
			}
		}
	}

	virtualHost = dynamic_cast<VVirtualHost *>(fHTTPResponse->GetVirtualHost());	// YT 03-Sep-2013 - ACI0083675 - Check VirtualHost is still valid

	if (!fHTTPResponse->IsValidEndPoint())
	{
		if (NULL != virtualHost)
			virtualHost->GetProject()->UnregisterRequest (fHTTPResponse);
		return XBOX::VE_OK;
	}

#if HTTP_SERVER_VERBOSE_MODE && HTTP_SERVER_DETAILED_OPERATIONS_TIMES
	sLONG8 handlingRequestTime = handlingRequestTimer.GetElapsedTime();
#endif
	

#if HTTP_SERVER_VERBOSE_MODE && HTTP_SERVER_DETAILED_OPERATIONS_TIMES
	VDebugTimer buildingResponseTimer;
#endif

	/* Set UAGSessionID in response header when applicable */
	if ((NULL != authDelegate) && (XBOX::VE_OK == error) && (NULL != fHTTPResponse->GetRequest().GetAuthenticationInfos()->GetUAGSession()))
	{
		authDelegate->SetUAGSessionAndCookie (fHTTPResponse);
	}

	HTTPStatusCode	statusCode = fHTTPResponse->GetResponseStatusCode();
	bool			bIsOK = ((XBOX::VE_OK == error) && ((HTTP_UNDEFINED == statusCode) || (HTTP_OK == statusCode)));

	if (bIsOK && fHTTPResponse->GetRequest().GetHTTPHeaders().GetHeaderValue (STRING_HEADER_ACCEPT, headerValue))
	{
		XBOX::VString contentType;
		fHTTPResponse->GetContentTypeHeader (contentType);
		if (!contentType.IsEmpty() && !HTTPProtocol::IsAcceptableMediaType (headerValue, contentType))
		{
			error = VE_HTTP_PROTOCOL_NOT_ACCEPTABLE; // YT 21-Jul-2011 - TestCase ContentCoding10 (see RFC2616 Section 14.1)
			/* Clear Response Body */
			fHTTPResponse->GetBody().Clear();	// YT 21-Feb-2014 - WAK0086762
			
			/* Clear Response Content-Length */
			fHTTPResponse->GetHeaders().RemoveHeader(STRING_HEADER_CONTENT_LENGTH);
		}
	}

	/* Prepare Response */
	if (XBOX::VE_OK != error)
	{
		if (COMPONENT_FROM_VERROR (error) != CHTTPServer::Component_Type)
		{
			XBOX::VString errorString;
			XBOX::VString componentSignature;
			componentSignature.FromOsType ((OsType)COMPONENT_FROM_VERROR (error));
			errorString.Printf ("Component \'%S\' throw an error: %d", &componentSignature, ERRCODE_FROM_VERROR (error));
			error = VHTTPServer::ThrowError (VE_HTTP_PROTOCOL_INTERNAL_SERVER_ERROR, errorString);
		}

		error = fHTTPResponse->ReplyWithStatusCode ((HTTPStatusCode)ERRCODE_FROM_VERROR (error)); // YT 31-Aug-2010 - Reset error value (to prevent VTCPEndPoint to be closed)
	}
	else
	{
		if (HTTP_UNDEFINED == fHTTPResponse->GetResponseStatusCode())
			fHTTPResponse->SetResponseStatusCode (HTTP_OK);
		else if (!HTTPProtocol::IsValidStatusCode (fHTTPResponse->GetResponseStatusCode()))
			fHTTPResponse->SetResponseStatusCode (HTTP_INTERNAL_SERVER_ERROR);

#if HTTP_SERVER_HANDLE_PARTIAL_CONTENT
		fHTTPResponse->AddResponseHeader (STRING_HEADER_ACCEPT_RANGES, STRING_HEADER_VALUE_BYTES);
#else
		/* Temp for Lot 1 - Does not handle Partial-Content for the moment*/
		if (HTTP_OK == fHTTPResponse->GetResponseStatusCode())
			fHTTPResponse->AddResponseHeader (STRING_HEADER_ACCEPT_RANGES, STRING_HEADER_VALUE_NONE);
#endif
	}

	++fRequestCount;

	/*
	 *	YT 17-May-2013 - WAK0079087
	 *	Prevent connection to be closed with requests containing "Expect: 100-Continue"
	 *	regardless the Connection header value
	 */
	statusCode = fHTTPResponse->GetResponseStatusCode();

	if (statusCode == HTTP_CONTINUE)
	{
		fKeepAlive = true;
	}
	else
	{
		/* No Keep-Alive when ForceCloseSession flag is set (when Server is shutting down for example) */
		if (fHTTPResponse->GetForceCloseSession())
			fKeepAlive = false;

		/* No Keep-Live for redirections, authentications or erroneous responses */
		if (fKeepAlive)
		{
			fKeepAlive = ((1 == (sLONG)statusCode / 100) || (2 == (sLONG)statusCode / 100));
		}

		/* Reset Keep-Live when limit is reached */
		if (IsMaxConnectionsReached())
			fKeepAlive = false;

		/* Reset Keep-Alive flag:
		*	- with HTTP/1.0 when Connection header is NOT explicitly "Keep-Alive"
		*	- with HTTP/1.1 when Connection header is explicitly "close"
		*/
		if (fKeepAlive)
		{
			XBOX::VString	connectionValue;

			/*	YT 20-Oct-2011 - WAK0072937 - According to RFC2616 - Chapter 8.1.2.1 Negotiation:
			*	An HTTP/1.1 server MAY assume that HTTP/1.1 client intends to maintain a persistent connection
			*	unless a Connection header including the connection-token "close" was sent in the request.
			*/

			fHTTPResponse->GetRequestHeader().GetHeaderValue (STRING_HEADER_CONNECTION, connectionValue);

			if (fHTTPResponse->GetRequestHTTPVersion() < VERSION_1_1)
			{
				/*	YT 05-Apr-2012 - We support Keep-Alive for HTTP/1.0 clients who explicitly 
				*	ask for it (using header "Connection: Keep-Alive")
			 */
				fKeepAlive = (HTTPServerTools::FindASCIIVString (connectionValue, STRING_HEADER_VALUE_KEEP_ALIVE) > 0);
			}
			else if (HTTPServerTools::FindASCIIVString (connectionValue, STRING_HEADER_VALUE_CLOSE) > 0)
			{
				fKeepAlive = false;
			}
		}

		/* Set Connection header */
		if (fKeepAlive)
		{
			fHTTPResponse->AddResponseHeader (STRING_HEADER_CONNECTION, STRING_HEADER_VALUE_KEEP_ALIVE, false);

#if HTTP_SERVER_VERBOSE_MODE
			/* The Keep-Alive field is NOT a standard HTTP header field and may not be supported by all clients. */
			XBOX::VString stringValue;
			stringValue.Printf ("timeout=%d, max=%d", (fKeepAliveTimeOut / 1000), (fMaxRequest - fRequestCount));
			fHTTPResponse->AddResponseHeader (STRING_HEADER_KEEP_ALIVE, stringValue);
#endif
		}
		else
		{
			fHTTPResponse->AddResponseHeader (STRING_HEADER_CONNECTION, STRING_HEADER_VALUE_CLOSE, false);
		}
	}

#if HTTP_SERVER_VERBOSE_MODE && HTTP_SERVER_DETAILED_OPERATIONS_TIMES
	sLONG8 buildingResponseTime = buildingResponseTimer.GetElapsedTime();
#endif
	
#if HTTP_SERVER_VERBOSE_MODE
	/* For DEBUG purposes */

	XBOX::VString infoString;

	infoString.Printf (	"timeMS=%d; reqCount=%d; socket=%d",
						(XBOX::VSystem::GetCurrentTime() - fHTTPResponse->GetStartRequestTime()),
						fRequestCount,
						fHTTPResponse->GetEndPoint()->GetRawSocket());

	fHTTPResponse->AddResponseHeader (CVSTR ("X-Debug-Infos"), infoString);
#endif
	
#if HTTP_SERVER_VERBOSE_MODE && HTTP_SERVER_DETAILED_OPERATIONS_TIMES
	infoString.Clear();
	infoString.AppendCString ("readReq=");
	infoString.AppendLong8 (readRequestTime);
	infoString.AppendCString ("; regReq=");
	infoString.AppendLong8 (registeringRequestTime);
	infoString.AppendCString ("; checkAuth=");
	infoString.AppendLong8 (checkingAuthTime);
	infoString.AppendCString ("; handleReq=");
	infoString.AppendLong8 (handlingRequestTime);
	infoString.AppendCString ("; buildResp=");
	infoString.AppendLong8 (buildingResponseTime);

	fHTTPResponse->AddResponseHeader (CVSTR ("X-Detailed-Times"), infoString);
#endif

	if (VERSION_UNKNOWN == fHTTPResponse->GetHTTPVersion())
		fHTTPResponse->SetHTTPVersion (VERSION_1_1);

	// YT 17-Mar-2011 - ACI0070300 (is this really a bug ?) - Set X-WA-Pattern header for WAF
#if HTTP_SERVER_USE_PROJECT_PATTERNS
	if (NULL != virtualHost)
	{
		if (!virtualHost->GetSettings()->GetProjectPattern().IsEmpty())
			fHTTPResponse->AddResponseHeader (STRING_HEADER_X_WA_PATTERN, virtualHost->GetSettings()->GetProjectPattern());
	}
#endif
	
	/* Send Response */
	fHTTPResponse->SendResponse();

	if (NULL != virtualHost)
	{
		/* Log Request */
		VHTTPServerLog *serverLog = virtualHost->GetServerLog();
		if (NULL != serverLog)
		{
			serverLog->Retain();
			serverLog->LogRequest (*fHTTPResponse);
			serverLog->Release();
		}

#if LOG_IN_CONSOLE
		VDebugTimer unregisteringTimer;
#endif

		virtualHost->GetProject()->UnregisterRequest (fHTTPResponse);

#if LOG_IN_CONSOLE
		unregisteringTimer.DebugMsg ("\tUnregistering Request");
#endif
	}

	if (!fKeepAlive)
	{
		Close();
	}

	if (NULL != requestLogger)
	{
		XBOX::VString logMessage;
		if (XBOX::VE_OK == error)
		{
			logMessage.AppendCString ("OK");
		}
		else
		{
			logMessage.AppendCString ("Error: ");
			logMessage.AppendLong (ERRCODE_FROM_VERROR (error));
			logMessage.AppendCString (" Component: \'");
			logMessage.AppendOsType ((OsType)COMPONENT_FROM_VERROR (error));
			logMessage.AppendCString ("\'");

#if LOG_IN_CONSOLE
			::DebugMsg ("\tVHTTPNetworkConnection::HandleRequest() returns error %d\n", ERRCODE_FROM_VERROR(error));
#endif
		}

		LogMessage (logMessage, XBOX::VSystem::GetCurrentTime() - startTime);
	}
	
	return error;
}


bool VHTTPServerSession::IsTimedOut() const
{
	return  (XBOX::VSystem::GetCurrentTime() - fStartTime) > fKeepAliveTimeOut;
}


bool VHTTPServerSession::IsMaxConnectionsReached() const
{
	return  (fRequestCount >= fMaxRequest);
}


void VHTTPServerSession::LogMessage (const XBOX::VString& inMessageString, sLONG inElapsedTime)
{
	VHTTPServer		*httpServer = dynamic_cast<VHTTPServer *>(fHTTPResponse->GetHTTPServer());
	IRequestLogger	*requestLogger = httpServer->GetRequestLogger();

	if (NULL != requestLogger)
		requestLogger->Log (CHTTPServer::Component_Type, NULL, inMessageString, inElapsedTime);
}

