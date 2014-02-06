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
#ifndef __AUTHENTICATION_REFEREE_INCLUDED__
#define __AUTHENTICATION_REFEREE_INCLUDED__


class VHTTPResource;


class VAuthenticationReferee : public XBOX::VObject, public IAuthenticationReferee
{
public:
										VAuthenticationReferee();
	virtual								~VAuthenticationReferee();

	VHTTPResource *						FindMatchingResource (const XBOX::VString& inURL, const HTTPRequestMethod& inHTTPMethod) const;

	XBOX::VError						AddRetainReferee (const XBOX::VString& inRegEx, VHTTPResource *inResource);

	XBOX::VError						LoadFromBag (const XBOX::VValueBag *inValueBag);
	XBOX::VError						LoadFromJSONFile (const XBOX::VFilePath& inFilePath);
	XBOX::VError						SaveToBag (XBOX::VValueBag *outBag);

	typedef std::pair<XBOX::VRefPtr<XBOX::VRegexMatcher>, XBOX::VRefPtr<VHTTPResource> >		RefereeMapEntry;
	typedef std::vector<RefereeMapEntry>														RefereeMap;

private:
	RefereeMap							fMap;
	mutable XBOX::VCriticalSection		fMapLock;
};


#endif	// __AUTHENTICATION_REFEREE_INCLUDED__