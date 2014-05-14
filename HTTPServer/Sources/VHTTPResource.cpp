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
#include "HTTPServerSettings.h"


//--------------------------------------------------------------------------------------------------


static
void _UpdateResourceURLPattern (XBOX::VString& ioURLRegexString)
{
	/*
		URL in resource settings may be a simple way to define a regex...
	
		When user write "/myPath/", we shall understand "(?i)^/myPath/"
		When user write "/myPath" (without SOLIDUS at the end of URL), we shall understand "(?i)^/myPath$"
		When user write "myPath" (without SOLIDUS in the beginning & at the end of URL), we shall understand "(?i)myPath$"
		When user write "myPath/" (without SOLIDUS in the beginning of URL), we shall understand "(?i)myPath/"

		When user set urlMatch in resource settings, we simply use it's regex without any transformation
	*/
	if (!ioURLRegexString.IsEmpty())
	{
		if (ioURLRegexString.GetUniChar (1) == CHAR_SOLIDUS)
			ioURLRegexString.Insert (CHAR_CIRCUMFLEX_ACCENT, 1);

		if (!HTTPServerTools::FindASCIICString (ioURLRegexString, "(?i)"))
			ioURLRegexString.Insert (CVSTR ("(?i)"), 1);

		if (ioURLRegexString.GetUniChar (ioURLRegexString.GetLength()) != CHAR_SOLIDUS)
			ioURLRegexString.AppendUniChar (CHAR_DOLLAR_SIGN);
	}
}


VHTTPResource::VHTTPResource()
: fURL()
, fURLMatch()
, fAllowedMethods (0)
, fDisallowedMethods (0)
, fGroup()
, fAuthType (AUTH_NONE)
, fRealm()
, fExpires()
, fLifeTime (0)
{
}


VHTTPResource::~VHTTPResource()
{
}


const XBOX::VTime& VHTTPResource::GetExpires() const
{
	return fExpires;
}


sLONG VHTTPResource::GetLifeTime() const
{
	if (!fLifeTime && (fExpires.CompareTo (TIME_EMPTY_DATE) != XBOX::CR_EQUAL))
	{
		XBOX::VTime currentTime;
		XBOX::VTime::Now (currentTime);

		return ((fExpires.GetMilliseconds() - currentTime.GetMilliseconds()) / 1000);
	}
	else
	{
		return fLifeTime;
	}
}


void VHTTPResource::LoadFromBag (const XBOX::VValueBag& inValueBag)
{
	/*
	 *	YT 27-Dec-2012 - WAK0085534 - Keep supporting "location" and "locationsMatch" properties for authentication mechanism in .waSettings
	 */
	inValueBag.GetString (RIASettingsKeys::Resources::urlPath, fURL);
	if (fURL.IsEmpty())
		inValueBag.GetString (RIASettingsKeys::Resources::location, fURL);
	inValueBag.GetString (RIASettingsKeys::Resources::urlRegex, fURLMatch);
	if (fURLMatch.IsEmpty())
		inValueBag.GetString (RIASettingsKeys::Resources::locationMatch, fURLMatch);
	inValueBag.GetString (RIASettingsKeys::Resources::group, fGroup);
	inValueBag.GetString (RIASettingsKeys::Resources::realm, fRealm);

	XBOX::VString	authType;
	inValueBag.GetString (RIASettingsKeys::Resources::authType, authType);
	fAuthType = HTTPServerTools::GetAuthenticationMethodFromName (authType);

	XBOX::VString allowString;
	inValueBag.GetString (RIASettingsKeys::Resources::allow, allowString);
	fAllowedMethods = _ParseMethodNamesString (allowString);

	XBOX::VString disallowString;
	inValueBag.GetString (RIASettingsKeys::Resources::disallow, disallowString);
	fDisallowedMethods = _ParseMethodNamesString (disallowString);

	XBOX::VString expires;
	inValueBag.GetString (RIASettingsKeys::Resources::expire, expires);
	if (!expires.IsEmpty())
		fExpires.FromXMLString (expires);

	inValueBag.GetLong (RIASettingsKeys::Resources::lifeTime, fLifeTime);

	_UpdateResourceURLPattern (fURL);
}


void VHTTPResource::LoadFromJSONValue (const XBOX::VJSONValue& inJSONValue)
{
	if (inJSONValue.IsObject())
	{
		if (!inJSONValue.GetProperty ("urlPath").IsUndefined())
			inJSONValue.GetProperty ("urlPath").GetString (fURL);
		else if (!inJSONValue.GetProperty ("urlRegex").IsUndefined())
			inJSONValue.GetProperty ("urlRegex").GetString (fURLMatch);

		inJSONValue.GetProperty ("group").GetString (fGroup);
		inJSONValue.GetProperty ("realm").GetString (fRealm);

		if (!inJSONValue.GetProperty ("maxAge").IsUndefined())
		{
			if (inJSONValue.GetProperty ("maxAge").IsNumber())
			{
				fLifeTime = static_cast<sLONG>(inJSONValue.GetProperty ("maxAge").GetNumber());
			}
			else
			{
				XBOX::VString expires;
				inJSONValue.GetProperty ("maxAge").GetString (expires);
				if (!expires.IsEmpty())
				{
					/*
					 *	Is "maxAge" attribute a valid RFC3339 Date ?
					 */
					if ((expires.GetLength() > 11 && (expires.GetUniChar (11) == 'T')) || (expires.FindUniChar (CHAR_COLON) >0 ) || (expires.FindUniChar (CHAR_SOLIDUS) > 0))
					{
						fExpires.FromXMLString (expires);
					}
					else
					{
						/*
						 *	Otherwise try to retrieve a lifeTime
						 */
						fLifeTime = static_cast<sLONG>(inJSONValue.GetProperty ("maxAge").GetNumber());
					}
				}
			}
		}

		/*
		 *	Following attributes are not used anymore.. Keep for backward compatibility ?!?!
		 */
		XBOX::VString	authType;
		inJSONValue.GetProperty ("authType").GetString (authType);
		fAuthType = HTTPServerTools::GetAuthenticationMethodFromName (authType);

		XBOX::VString allowString;
		inJSONValue.GetProperty ("allow").GetString (allowString);
		fAllowedMethods = _ParseMethodNamesString (allowString);

		XBOX::VString disallowString;
		inJSONValue.GetProperty ("disallow").GetString (disallowString);
		fDisallowedMethods = _ParseMethodNamesString (disallowString);

		_UpdateResourceURLPattern (fURL);
	}
}


void VHTTPResource::SaveToBag (XBOX::VValueBag& ioValueBag)
{
	if (!fURL.IsEmpty())
		ioValueBag.SetString (RIASettingsKeys::Resources::urlPath, fURL);

	if (!fURLMatch.IsEmpty())
		ioValueBag.SetString (RIASettingsKeys::Resources::urlRegex, fURLMatch);

	if (!fGroup.IsEmpty())
		ioValueBag.SetString (RIASettingsKeys::Resources::group, fGroup);

	if (!fRealm.IsEmpty())
		ioValueBag.SetString (RIASettingsKeys::Resources::realm, fRealm);

	if (fAuthType != AUTH_NONE)
	{
		XBOX::VString authType;
		HTTPProtocol::GetAuthenticationType (fAuthType, authType);
		ioValueBag.SetString (RIASettingsKeys::Resources::authType, authType);
	}

	if (fExpires.CompareTo (TIME_EMPTY_DATE) != XBOX::CR_EQUAL)
	{
		XBOX::VString expires;
		fExpires.GetXMLString (expires, XBOX::XSO_Time_UTC);
		ioValueBag.SetString (RIASettingsKeys::Resources::expire, expires);
	}

	if (fLifeTime > 0)
		ioValueBag.SetLong (RIASettingsKeys::Resources::lifeTime, fLifeTime);

	if (fAllowedMethods > 0)
	{
		XBOX::VString allowString;
		_BuildMethodNamesString (fAllowedMethods, true, allowString);
		ioValueBag.SetString (RIASettingsKeys::Resources::allow, allowString);
	}

	if (fDisallowedMethods > 0)
	{
		XBOX::VString disallowString;
		_BuildMethodNamesString (fDisallowedMethods, false, disallowString);
		ioValueBag.SetString (RIASettingsKeys::Resources::disallow, disallowString);
	}
}


bool VHTTPResource::IsAllowedMethod (HTTPRequestMethod inMethod) const
{
	/* GET & HEAD methods are always allowed except when they are explicitly forbidden in settings */
	if ((inMethod == HTTP_GET) || (inMethod == HTTP_HEAD))
	{
		return (!IsDisallowedMethod (inMethod));
	}
	else
	{
		return ((fAllowedMethods & (1 << (sLONG)inMethod)) && !(IsDisallowedMethod (inMethod)));
	}
}


bool VHTTPResource::IsDisallowedMethod (HTTPRequestMethod inMethod) const
{
	return (fDisallowedMethods & (1 << (sLONG)inMethod)) ? true : false;
}


sLONG VHTTPResource::_ParseMethodNamesString (const XBOX::VString& inMethodNamesString)
{
	sLONG result = 0;

	if (!inMethodNamesString.IsEmpty())
	{
		XBOX::VectorOfVString vector;
		inMethodNamesString.GetSubStrings (CHAR_COMMA, vector, false, true);

		for (XBOX::VectorOfVString::const_iterator it = vector.begin(); it != vector.end(); ++it)
		{
			sLONG method = HTTPProtocol::GetMethodFromName (*it);
			if (method != HTTP_UNKNOWN)
				result |= (1 << (sLONG)method);
		}
	}

	return result;
}


void VHTTPResource::_BuildMethodNamesString (const sLONG inMethods, bool inAllow, XBOX::VString& outMethodNamesString)
{
	XBOX::VString	method;

	outMethodNamesString.Clear();

	for (sLONG i = (sLONG)HTTP_GET; i <= (sLONG)HTTP_OPTIONS; ++i)
	{
		if ((inAllow && IsAllowedMethod ((HTTPRequestMethod)i)) ||
			(!inAllow && IsDisallowedMethod ((HTTPRequestMethod)i)))
		{
			HTTPProtocol::MakeHTTPMethodString ((HTTPRequestMethod)i, method);
			if (!outMethodNamesString.IsEmpty())
				outMethodNamesString.AppendUniChar (CHAR_COMMA);
			outMethodNamesString.AppendString (method);
		}
	}
}


