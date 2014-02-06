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
#ifndef __HTTP_RESOURCE_INCLUDED__
#define __HTTP_RESOURCE_INCLUDED__


class VHTTPResource : public XBOX::VObject, public XBOX::IRefCountable
{
public:
										VHTTPResource();
										~VHTTPResource();

	void								LoadFromBag (const XBOX::VValueBag& inValueBag);
	void								LoadFromJSONValue (const XBOX::VJSONValue& inJSONValue);
	void								SaveToBag (XBOX::VValueBag& ioValueBag);

	const XBOX::VString&				GetURL() const { return fURL; }
	const XBOX::VString&				GetURLMatch() const { return fURLMatch; }
	const XBOX::VString&				GetRealm() const { return fRealm; }
	const XBOX::VString&				GetGroup() const { return fGroup; }
	HTTPAuthenticationMethod			GetAuthType() const { return fAuthType; }
	sLONG								GetAllowedMethods() const { return fAllowedMethods; }
	sLONG								GetDisallowedMethods() const { return fDisallowedMethods; }
	const XBOX::VTime&					GetExpires() const;
	sLONG								GetLifeTime() const;

	bool								IsAllowedMethod (HTTPRequestMethod inMethod) const;
	bool								IsDisallowedMethod (HTTPRequestMethod inMethod) const;

private:
	sLONG								_ParseMethodNamesString (const XBOX::VString& inMethodNamesString);
	void								_BuildMethodNamesString (const sLONG inMethods, bool inAllow, XBOX::VString& outMethodNamesString);

private:
	XBOX::VString						fURL;
	XBOX::VString						fURLMatch;
	sLONG								fAllowedMethods;
	sLONG								fDisallowedMethods;
	XBOX::VString						fGroup;
	HTTPAuthenticationMethod			fAuthType;
	XBOX::VString						fRealm;
	mutable XBOX::VTime					fExpires;
	mutable sLONG						fLifeTime;
};


#endif	// __HTTP_RESOURCE_INCLUDED__
