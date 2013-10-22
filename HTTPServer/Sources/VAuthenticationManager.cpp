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
void ExtractFieldValuePair (const XBOX::VString& inHeader, const XBOX::VString& inFieldName, XBOX::VString& outFieldValue)
{
	/* This is the kind of string we receive :
		Digest username="toto", realm="test web.4DB", nonce="1068215347:7307f04f919115ff129c7f2d9fb0214c",
		uri="/dfghgdfhfffrdfgh", response="6179a3695580ec07375545dc7c03f960", algorithm="MD5",
		cnonce="750c89c5ce15dee24dffffa38703b492", nc=00000008, qop="auth"
	*/

	outFieldValue.Clear();

	if  ((inHeader.GetLength() < inFieldName.GetLength()) || (0 == inFieldName.GetLength()))
		return;

	XBOX::VString fieldName (inFieldName);
	fieldName.AppendUniChar (CHAR_EQUALS_SIGN);

	XBOX::VIndex startValue = HTTPServerTools::FindASCIIVString (inHeader, fieldName);
	if (startValue > 0)
	{
		XBOX::VIndex endValue = 0;

		startValue += (fieldName.GetLength() - 1);
		endValue = inHeader.FindUniChar (CHAR_COMMA, (startValue + 1));

		if (endValue > startValue)
			--endValue;
		else
			endValue = inHeader.GetLength();

		outFieldValue.AppendUniChars (inHeader.GetCPointer() + startValue, endValue - startValue);

		// Clean-up value String
		HTTPServerTools::TrimUniChar (outFieldValue, CHAR_QUOTATION_MARK);
		HTTPServerTools::TrimUniChar (outFieldValue, CHAR_SPACE);
	}
}


//--------------------------------------------------------------------------------------------------


VAuthenticationInfos::VAuthenticationInfos()
: fAuthenticationMethod (AUTH_NONE)
, fUserName()
, fUserGroup()
, fPassword()
, fKerberosTicket()
, fUAGSession(NULL)
, fRealm()
, fHTTPRequestMethod (HTTP_UNKNOWN)
, fNonce()
, fOpaque()
, fCnonce()
, fQop()
, fNonceCount()
, fAlgorithm()
, fResponse()
, fURI()
, fDomain()
{
	fNeedAddUAGSession = false;
}


VAuthenticationInfos::VAuthenticationInfos (const VAuthenticationInfos& inAuthenticationInfos)
{
	fUAGSession = NULL;
	*this = inAuthenticationInfos;
}


VAuthenticationInfos::~VAuthenticationInfos()
{
	XBOX::QuickReleaseRefCountable(fUAGSession);
}


VAuthenticationInfos& VAuthenticationInfos::operator = (const VAuthenticationInfos& inAuthenticationInfos)
{
	if (&inAuthenticationInfos != this)
	{
		fAuthenticationMethod = inAuthenticationInfos.fAuthenticationMethod;
		fUserName.FromString (inAuthenticationInfos.fUserName);
		fUserGroup.FromString (inAuthenticationInfos.fUserGroup);
		fPassword.FromString (inAuthenticationInfos.fPassword);
		fKerberosTicket.FromString (inAuthenticationInfos.fKerberosTicket);
		XBOX::CopyRefCountable(&fUAGSession, inAuthenticationInfos.fUAGSession);
		fAlreadyParsed = inAuthenticationInfos.fAlreadyParsed;
		fRealm.FromString (inAuthenticationInfos.fRealm);
		fHTTPRequestMethod = inAuthenticationInfos.fHTTPRequestMethod;
		fNonce.FromString (inAuthenticationInfos.fNonce);
		fOpaque.FromString (inAuthenticationInfos.fOpaque);
		fCnonce.FromString (inAuthenticationInfos.fCnonce);
		fQop.FromString (inAuthenticationInfos.fQop);
		fNonceCount.FromString (inAuthenticationInfos.fNonceCount);
		fAlgorithm.FromString (inAuthenticationInfos.fAlgorithm);
		fResponse.FromString (inAuthenticationInfos.fResponse);
		fURI.FromString (inAuthenticationInfos.fURI);
		fDomain.FromString (inAuthenticationInfos.fDomain);
		fNeedAddUAGSession = inAuthenticationInfos.fNeedAddUAGSession;
	}

	return *this;
}


bool VAuthenticationInfos::IsValid() const
{
	return (!fUserName.IsEmpty() || !fPassword.IsEmpty());
}


bool VAuthenticationInfos::IsValidForKerberos() const
{
	return (!fKerberosTicket.IsEmpty());	//jmo - todo : un peu light...
}


void VAuthenticationInfos::Clear()
{
	fAuthenticationMethod = AUTH_NONE;
	fUserName.Clear();
	fUserGroup.Clear();
	fPassword.Clear();
	XBOX::ReleaseRefCountable(&fUAGSession);
	fRealm.Clear();
	fHTTPRequestMethod = HTTP_UNKNOWN;
	fNonce.Clear();
	fOpaque.Clear();
	fCnonce.Clear();
	fQop.Clear();
	fNonceCount.Clear();
	fAlgorithm.Clear();
	fResponse.Clear();
	fURI.Clear();
	fDomain.Clear();
}


void VAuthenticationInfos::GetHTTPRequestMethodName (XBOX::VString& outValue) const
{
	outValue.Clear();
	if (fHTTPRequestMethod != HTTP_UNKNOWN)
		HTTPProtocol::MakeHTTPMethodString (fHTTPRequestMethod, outValue);
}


//--------------------------------------------------------------------------------------------------


VAuthenticationManager::VAuthenticationManager (const XBOX::VValueBag *inSettings)
: fSecurityManager (NULL), fAuthenticationDelegate(NULL)
{
	fAuthenticationReferee = new VAuthenticationReferee (inSettings);
}


VAuthenticationManager::~VAuthenticationManager()
{
	XBOX::ReleaseRefCountable (&fSecurityManager);
	XBOX::ReleaseRefCountable (&fAuthenticationReferee);
	XBOX::ReleaseRefCountable( &fAuthenticationDelegate);
}


/* static */
VAuthenticationInfos *VAuthenticationManager::CreateAuthenticationInfosFromHeader (const VHTTPHeader& inHeader, const HTTPRequestMethod inMethod)
{
	VAuthenticationInfos *	result = NULL;
	XBOX::VString			headerValue;

	if (!inHeader.GetHeaderValue (STRING_HEADER_AUTHORIZATION, headerValue) || headerValue.IsEmpty())
		return NULL;

	const UniChar *	startPtr = headerValue.GetCPointer();
	const UniChar *	endPtr = startPtr + headerValue.GetLength();
	sLONG			pos = 0;
	XBOX::VString	string;

	if ((headerValue.GetLength() >= STRING_AUTHENTICATION_BASIC.GetLength()) &&
		((pos = HTTPServerTools::FindASCIIVString (startPtr, STRING_AUTHENTICATION_BASIC)) > 0))
	{
		startPtr += (pos + STRING_AUTHENTICATION_BASIC.GetLength() - 1);

		// skip linear spaces
		while ((startPtr < endPtr) && std::isspace (*startPtr))
			++startPtr;

		while ((startPtr < endPtr) && std::isspace (*endPtr))
			--endPtr;

		string.FromBlock (startPtr, (endPtr - startPtr) * sizeof(UniChar), XBOX::VTC_UTF_16);

		if (XBOX::VE_OK == HTTPServerTools::Base64Decode (string))
		{
			pos = string.FindUniChar (CHAR_COLON);
			if (pos > 0)
			{
				XBOX::VString userName;
				XBOX::VString password;

				if (pos > 1)
					string.GetSubString (1, pos - 1, userName);

				XBOX::VIndex length = (string.GetLength() - userName.GetLength() - 1);
				if (length >= 1)
					string.GetSubString (pos + 1, length, password);

				result = new VAuthenticationInfos();
				if (NULL != result)
				{
					result->SetAuthenticationMethod (AUTH_BASIC);
					result->SetUserName (userName);
					result->SetPassword (password);
				}
			}
		}
	}
	else if ((headerValue.GetLength() >= STRING_AUTHENTICATION_NEGOTIATE.GetLength()) &&
			((pos = HTTPServerTools::FindASCIIVString (startPtr, STRING_AUTHENTICATION_NEGOTIATE)) > 0))
	{
		startPtr += (pos + STRING_AUTHENTICATION_NEGOTIATE.GetLength() - 1);

		// skip linear spaces
		while ((startPtr < endPtr) && std::isspace (*startPtr))
			++startPtr;

		while ((startPtr < endPtr) && std::isspace (*endPtr))
			--endPtr;

		string.FromBlock (startPtr, (endPtr - startPtr) * sizeof(UniChar), XBOX::VTC_UTF_16);

		if (!string.IsEmpty())
		{
			result = new VAuthenticationInfos();

			if (NULL != result)
			{
				result->SetAuthenticationMethod (AUTH_KERBEROS);
				result->SetKerberosTicket (string);
			}
		}
	}
	else if ((headerValue.GetLength() > STRING_AUTHENTICATION_DIGEST.GetLength()) &&
			((pos = HTTPServerTools::FindASCIIVString (startPtr, STRING_AUTHENTICATION_DIGEST)) > 0))
	{
		startPtr += (pos + STRING_AUTHENTICATION_DIGEST.GetLength() - 1);

		// skip linear spaces
		while ((startPtr < endPtr) && std::isspace (*startPtr))
			++startPtr;

		while ((startPtr < endPtr) && std::isspace (*endPtr))
			--endPtr;

		result = new VAuthenticationInfos();
		if (NULL != result)
		{
			XBOX::VString nonceString;

			_PopulateAuthenticationParameters (headerValue, *result);
			result->SetAuthenticationMethod (AUTH_DIGEST);
			result->GetNonce (nonceString);
			VNonce::ValidNonceAndCleanPile (nonceString);
		}
	}

	if (NULL != result)
		result->SetHTTPRequestMethod (inMethod); // Useful only with DIGEST Authentication, but who knows ?

	return result;
}


bool VAuthenticationManager::_ValidateAuthentication (IAuthenticationInfos *ioAuthenticationInfos, const IHTTPRequest *inRequest)
{
	VAuthenticationInfos *	authenticationInfos = dynamic_cast <VAuthenticationInfos *>(ioAuthenticationInfos);
	if ((NULL == authenticationInfos) || (NULL == fSecurityManager))
		return false;

	bool					isOK = false;
	XBOX::VError			error = XBOX::VE_OK;
	XBOX::VString			userGroup;
	CUAGSession*			uagSession = NULL;


	if (XBOX::VE_OK == error)
	{
		switch (authenticationInfos->GetAuthenticationMethod())
		{
		case AUTH_NONE:
			isOK = true; // What else ??
			break;

		case AUTH_BASIC:
			if (authenticationInfos->IsValid())
			{
				XBOX::VString userName;
				XBOX::VString password;

				authenticationInfos->GetUserName (userName);
				authenticationInfos->GetPassword (password);

				CUAGDirectory *directory = fSecurityManager->GetUserDirectory();
				if ((directory != NULL) && directory->HasLoginListener())
				{
					if (testAssert(fAuthenticationDelegate != NULL))
					{
						// sc 22/06/2012, custom JavaScript authentication support
						XBOX::VJSGlobalContext *globalContext = fAuthenticationDelegate->RetainJSContext( NULL, true, inRequest);
						assert(globalContext != NULL);
						error = fSecurityManager->ValidateBasicAuthentication (userName, password, &isOK, uagSession, globalContext);
						fAuthenticationDelegate->ReleaseJSContext( globalContext);
					}
				}
				else
				{
					error = fSecurityManager->ValidateBasicAuthentication (userName, password, &isOK, uagSession, NULL);
				}
			}
			break;

		case AUTH_KERBEROS:
			if (authenticationInfos->IsValidForKerberos())
			{
				XBOX::VString outAuthData;
				XBOX::VString krbTicket;

				authenticationInfos->GetKerberosTicket (krbTicket);

				error = fSecurityManager->ValidateKerberosAuthentication (krbTicket, &isOK, &outAuthData, uagSession);
			}
			break;

		case AUTH_DIGEST:
			{
				if (authenticationInfos->IsValid())
				{
					XBOX::VString user, algorithm, realm, nonce, cnonce, qop, nonceCount, uri, method, response;

					authenticationInfos->GetUserName (user);
					authenticationInfos->GetAlgorithm (algorithm);
					authenticationInfos->GetRealm (realm);
					authenticationInfos->GetNonce (nonce);
					authenticationInfos->GetCnonce (cnonce);
					authenticationInfos->GetQop (qop);
					authenticationInfos->GetNonceCount (nonceCount);
					authenticationInfos->GetURI (uri);
					authenticationInfos->GetHTTPRequestMethodName (method);
					authenticationInfos->GetResponse (response);

					CUAGDirectory *directory = fSecurityManager->GetUserDirectory();
					if ((directory != NULL) && directory->HasLoginListener())
					{
						if (testAssert(fAuthenticationDelegate != NULL))
						{
							// sc 22/06/2012, custom JavaScript authentication support
							XBOX::VJSGlobalContext *globalContext = fAuthenticationDelegate->RetainJSContext( NULL, true, inRequest);
							assert(globalContext != NULL);
							error = fSecurityManager->ValidateDigestAuthentication (user, algorithm, realm, nonce, cnonce, qop, nonceCount, uri, method, response, &isOK, uagSession, globalContext);
							fAuthenticationDelegate->ReleaseJSContext( globalContext);
						}
					}
					else
					{
						error = fSecurityManager->ValidateDigestAuthentication (user, algorithm, realm, nonce, cnonce, qop, nonceCount, uri, method, response, &isOK, uagSession, NULL);
					}
				}
				else
				{
					isOK = false;
				}
			}
			break;

		case AUTH_NTLM:
			break;
		}

		authenticationInfos->SetUAGSession (uagSession);
		authenticationInfos->UAGSessionHasChanged();

		if (uagSession != NULL && isOK)
		{ 
			authenticationInfos->GetUserGroup (userGroup);
			if (!userGroup.IsEmpty())
			{
				CUAGDirectory* directory = fSecurityManager->GetUserDirectory();

				if (directory == NULL)
					isOK = false;
				else
				{
					CUAGGroup* group = directory->RetainGroup(userGroup);
					if (group != NULL)
					{
						isOK = uagSession->BelongsTo(group);
						group->Release();
					}
				}
			}

/*			if (!isOK)
			{
				// dois je remettre l'uag session a null ?
				sLONG xdebug = 1; // put a break here
			}
*/
		}

		XBOX::QuickReleaseRefCountable(uagSession);
	}

	return isOK;
}


XBOX::VError VAuthenticationManager::CheckAndValidateAuthentication (IHTTPResponse *ioResponse)
{
	XBOX::VError				error = XBOX::VE_OK;
	HTTPRequestMethod			requestMethod = ioResponse->GetRequestMethod();
	VHTTPResource *				matchingResource = GetAuthenticationReferee()->FindMatchingResource (ioResponse->GetRequest().GetURLPath(), requestMethod);
	VVirtualHost *				virtualHost = dynamic_cast<VVirtualHost *>(ioResponse->GetVirtualHost());
	VAuthenticationInfos *		authInfos = dynamic_cast<VAuthenticationInfos *>(ioResponse->GetRequest().GetAuthenticationInfos());
	HTTPAuthenticationMethod	foundAuthMethod = authInfos->GetAuthenticationMethod();
	HTTPAuthenticationMethod	wantedAuthMethod = AUTH_NONE;
	XBOX::VString				wantedAuthRealm;


	if (NULL != matchingResource)
	{
		if (!matchingResource->IsDisallowedMethod (requestMethod))
		{
			wantedAuthMethod = matchingResource->GetAuthType();
			wantedAuthRealm = matchingResource->GetRealm();

			if (AUTH_NONE == wantedAuthMethod)
				wantedAuthMethod = virtualHost->GetProject()->GetSettings()->GetDefaultAuthType();

			if (wantedAuthRealm.IsEmpty())
				wantedAuthRealm.FromString (virtualHost->GetProject()->GetSettings()->GetDefaultRealm());

			if ((wantedAuthMethod != AUTH_NONE) && (foundAuthMethod != wantedAuthMethod))
			{
				error = VE_HTTP_PROTOCOL_UNAUTHORIZED;
				ioResponse->SetWantedAuthMethod (wantedAuthMethod);
				ioResponse->SetWantedAuthRealm (wantedAuthRealm);
			}
			else
			{
				if (!matchingResource->GetGroup().IsEmpty())
					authInfos->SetUserGroup (matchingResource->GetGroup());

				if (!_ValidateAuthentication (authInfos, &ioResponse->GetRequest()))
				{
					switch (foundAuthMethod)
					{
					case AUTH_KERBEROS:
						error = VE_HTTP_PROTOCOL_FORBIDDEN;
						break;

					default:
						error = VE_HTTP_PROTOCOL_UNAUTHORIZED;
						break;
					}

					ioResponse->SetWantedAuthMethod (wantedAuthMethod);
					ioResponse->SetWantedAuthRealm (wantedAuthRealm);
				}
			}
		}
		else
		{
			error = VE_HTTP_PROTOCOL_UNAUTHORIZED;
		}
	}
	else
	{
		if (AUTH_NONE != foundAuthMethod)
		{
			wantedAuthMethod = virtualHost->GetProject()->GetSettings()->GetDefaultAuthType();
			wantedAuthRealm.FromString (virtualHost->GetProject()->GetSettings()->GetDefaultRealm());

			if ((wantedAuthMethod != AUTH_NONE) && (foundAuthMethod != wantedAuthMethod))
			{
				error = VE_HTTP_PROTOCOL_UNAUTHORIZED;
				ioResponse->SetWantedAuthMethod (wantedAuthMethod);
				ioResponse->SetWantedAuthRealm (wantedAuthRealm);
			}
			else
			{
				if (!_ValidateAuthentication (authInfos, &ioResponse->GetRequest()))
				{
					switch (foundAuthMethod)
					{
					case AUTH_KERBEROS:
						error = VE_HTTP_PROTOCOL_FORBIDDEN;
						break;

					default:
						error = VE_HTTP_PROTOCOL_UNAUTHORIZED;
						break;
					}

					ioResponse->SetWantedAuthMethod (wantedAuthMethod);
					ioResponse->SetWantedAuthRealm (wantedAuthRealm);
				}
			}
		}
	}

	return error;
}


XBOX::VError VAuthenticationManager::CheckAdminAccessGranted (IHTTPResponse *ioResponse)
{
	XBOX::VError				error = XBOX::VE_OK;
	HTTPAuthenticationMethod	wantedAuthMethod = AUTH_NONE;
	XBOX::VString				wantedAuthRealm;

	if (NULL != fSecurityManager)
	{
		VHTTPResponse *		response = dynamic_cast<VHTTPResponse *>(ioResponse);
		CUAGDirectory *		uagDirectory = fSecurityManager->GetUserDirectory();
		CUAGGroup *			adminGroup = (uagDirectory) ? uagDirectory->RetainSpecialGroup (CUAGDirectory::AdminGroup) : NULL;
		CUAGSession *		uagSession = (NULL != response) ? ioResponse->GetRequest().GetAuthenticationInfos()->GetUAGSession() : NULL;

		if ((NULL != uagDirectory) && (NULL != adminGroup) && ((NULL == uagSession) || !uagSession->BelongsTo (adminGroup)))
		{
			VVirtualHost *	virtualHost = dynamic_cast<VVirtualHost *>(ioResponse->GetVirtualHost());

			wantedAuthMethod = virtualHost->GetProject()->GetSettings()->GetDefaultAuthType();
			wantedAuthRealm.FromString (virtualHost->GetProject()->GetSettings()->GetDefaultRealm());

			if (wantedAuthMethod == AUTH_NONE)
				wantedAuthMethod = AUTH_BASIC;

			if (wantedAuthRealm.IsEmpty())
				wantedAuthRealm.FromCString ("Wakanda");

			ioResponse->SetWantedAuthMethod (wantedAuthMethod);
			ioResponse->SetWantedAuthRealm (wantedAuthRealm);

			error = VE_HTTP_PROTOCOL_UNAUTHORIZED;
		}

		XBOX::ReleaseRefCountable (&adminGroup);
	}

	return error;
}


XBOX::VError VAuthenticationManager::SetAuthorizationHeader (IHTTPResponse *ioResponse)
{
	VHTTPResponse *	response = dynamic_cast<VHTTPResponse *>(ioResponse);
	if (NULL == response)
		return VE_HTTP_INVALID_ARGUMENT;

	XBOX::VString	authHeaderValue;

	if (!response->GetRequestHeader().GetHeaderValue (STRING_HEADER_WWW_AUTHENTICATE, authHeaderValue))
	{
		XBOX::VString				realm;
		XBOX::VString				domain (response->GetRequest().GetURLPath());
		VVirtualHost *				virtualHost = dynamic_cast<VVirtualHost *>(ioResponse->GetVirtualHost());
		HTTPAuthenticationMethod	wantedAuthMethod = ioResponse->GetWantedAuthMethod(); // YT 03-Oct-2011 - WAK0072836

		if (AUTH_NONE == wantedAuthMethod)
			wantedAuthMethod = virtualHost->GetProject()->GetSettings()->GetDefaultAuthType();

		// Is there a realm already defined ?
		response->GetWantedAuthRealm (realm);

		// If none, let's retrieve the default one (set in Solution settings).
		if (realm.IsEmpty())
		{
			if (!virtualHost->GetProject()->GetSettings()->GetDefaultRealm().IsEmpty())
				realm.FromString (virtualHost->GetProject()->GetSettings()->GetDefaultRealm());
			else
				realm.FromCString ("Wakanda");
		}

		if (HTTPProtocol::MakeHTTPAuthenticateHeaderValue (wantedAuthMethod, realm, domain, authHeaderValue))
			response->GetHeaders().SetHeaderValue (STRING_HEADER_WWW_AUTHENTICATE, authHeaderValue);
	}

	return XBOX::VE_OK;
}


void VAuthenticationManager::SetSecurityManager (CSecurityManager *inSecurityManager)
{
	if (NULL != fSecurityManager)
		XBOX::QuickReleaseRefCountable (fSecurityManager);

	fSecurityManager = XBOX::RetainRefCountable (inSecurityManager);
}


void VAuthenticationManager::SetAuthenticationDelegate( IAuthenticationDelegate *inAuthenticationDelegate)
{
	CopyRefCountable( &fAuthenticationDelegate, inAuthenticationDelegate);
}


/* Used for DIGEST-MD5 */
void VAuthenticationManager::_PopulateAuthenticationParameters (const XBOX::VString& inAuthorizationHTTPField, VAuthenticationInfos& outAuthenticationInfos)
{
	ExtractFieldValuePair (inAuthorizationHTTPField, STRING_AUTH_FIELD_REALM, outAuthenticationInfos.fRealm);
	ExtractFieldValuePair (inAuthorizationHTTPField, STRING_AUTH_FIELD_NONCE, outAuthenticationInfos.fNonce);
	ExtractFieldValuePair (inAuthorizationHTTPField, STRING_AUTH_FIELD_OPAQUE, outAuthenticationInfos.fOpaque);
	ExtractFieldValuePair (inAuthorizationHTTPField, STRING_AUTH_FIELD_CNONCE, outAuthenticationInfos.fCnonce);
	ExtractFieldValuePair (inAuthorizationHTTPField, STRING_AUTH_FIELD_QOP, outAuthenticationInfos.fQop);
	ExtractFieldValuePair (inAuthorizationHTTPField, STRING_AUTH_FIELD_NC, outAuthenticationInfos.fNonceCount);
	ExtractFieldValuePair (inAuthorizationHTTPField, STRING_AUTH_FIELD_ALGORITHM, outAuthenticationInfos.fAlgorithm);
	ExtractFieldValuePair (inAuthorizationHTTPField, STRING_AUTH_FIELD_RESPONSE, outAuthenticationInfos.fResponse);
	ExtractFieldValuePair (inAuthorizationHTTPField, STRING_AUTH_FIELD_USERNAME, outAuthenticationInfos.fUserName);
	ExtractFieldValuePair (inAuthorizationHTTPField, STRING_AUTH_FIELD_URI, outAuthenticationInfos.fURI);
}


/* Used for DIGEST-MD5 */
bool VAuthenticationManager::_IsValidNonce (const VAuthenticationInfos& inAuthenticationInfos)
{
	/*
	 - The fNonce field looks like :  1068215347:7307f04f919115ff129c7f2d9fb0214c
	 - The first part (before ':') of the nonce is the time in seconds when the nonce was generated.
	 We consider the the nonce has a life time of 2 minutes.
	 - The second part is random values.
	 */

	bool	result = false;

	if (!inAuthenticationInfos.fNonce.IsEmpty())	//the authentication is not valid.
	{
		XBOX::VString	nonceTimeValue;
		XBOX::VString	nonceRandomValue;
		
		if (VNonce::ParseNonceValue (inAuthenticationInfos.fNonce, nonceTimeValue, nonceRandomValue))
		{
			sLONG8		nowTime = 0, clientTime = 0;
			XBOX::VTime	curTime;
			
			curTime.FromSystemTime();
			nowTime = curTime.GetMilliseconds();
			clientTime = nonceTimeValue.GetLong8();
			
			if ((clientTime >= (nowTime - CONST_NONCE_TIMEOUT)) && (clientTime <= nowTime)) // only valid for 2 minutes
			{
				if (VNonce::ValidNonceAndCleanPile (inAuthenticationInfos.fNonce))
					result = true;
			}						
		}
	}
	
	return result;
}


