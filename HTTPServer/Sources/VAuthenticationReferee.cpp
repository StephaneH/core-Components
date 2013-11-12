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


namespace NSReferee
{
	CREATE_BAGKEY (pattern);
	CREATE_BAGKEY (authenticationMethod);
}


//--------------------------------------------------------------------------------------------------


VAuthenticationReferee::VAuthenticationReferee()
{
}


VAuthenticationReferee::~VAuthenticationReferee()
{
	fMap.erase (fMap.begin(), fMap.end());
}


VHTTPResource *VAuthenticationReferee::FindMatchingResource (const XBOX::VString& inURL, const HTTPRequestMethod& inHTTPMethod) const
{
	VHTTPResource *	result = NULL;
	XBOX::VTaskLock	lock (&fMapLock);

	for (RefereeMap::const_iterator it = fMap.begin(); it != fMap.end(); ++it)
	{
		if (MatchRegex (it->first, inURL) && it->second->IsAllowedMethod (inHTTPMethod))
		{
			result = it->second.Get();
			break;
		}
	}

	return result;
}


XBOX::VError VAuthenticationReferee::AddRetainReferee (const XBOX::VString& inPattern, VHTTPResource *inResource)
{
	XBOX::VError error = XBOX::VE_OK;

	if (!inPattern.IsEmpty())
	{
		XBOX::VTaskLock lock (&fMapLock);

		RefereeMap::const_iterator foundMatcher = std::find_if (fMap.begin(), fMap.end(), EqualVRegexMatcherFunctor<XBOX::VRefPtr<VHTTPResource> > (inPattern));
		if (foundMatcher == fMap.end())
		{
			XBOX::VRegexMatcher *matcher = XBOX::VRegexMatcher::Create (inPattern, &error);

			if ((NULL != matcher) && (XBOX::VE_OK == error))
			{
				fMap.push_back (std::make_pair (matcher,  inResource));
			}

			XBOX::QuickReleaseRefCountable (matcher);
		}
		else
		{
			error = VE_AUTHENTICATION_REFEREE_ALREADY_EXISTS;
		}
	}

	return error;
}


XBOX::VError VAuthenticationReferee::LoadFromBag (const XBOX::VValueBag *inValueBag)
{
	if (NULL == inValueBag)
		return XBOX::VE_INVALID_PARAMETER;

	XBOX::VError error = XBOX::VE_OK;

	/* Resources settings */
	const XBOX::VBagArray *bagArray = RetainMultipleSettings (inValueBag, RIASettingsKeys::Resources::kXmlElement);
	if (bagArray)
	{
		if (fMapLock.Lock())
		{
			XBOX::VString	patternString;
			
			fMap.clear();
			fMapLock.Unlock();

			for (XBOX::VIndex i = 1; i <= bagArray->GetCount(); ++i)
			{
				const XBOX::VValueBag *bag = bagArray->GetNth (i);
				if (bag)
				{
					VHTTPResource *resource = new VHTTPResource();
					if (resource)
					{
						resource->LoadFromBag (*bag);

						if (!resource->GetURLMatch().IsEmpty())
							patternString.FromString (resource->GetURLMatch());
						else
							patternString.FromString (resource->GetURL());

						if (!patternString.IsEmpty() && (resource->GetDisallowedMethods() || resource->GetAllowedMethods()))
						{
							error = AddRetainReferee (patternString, resource);
							if (XBOX::VE_OK != error)
								break;
						}

						resource->Release();
					}
				}
			}
		}

		XBOX::QuickReleaseRefCountable (bagArray);
	}

	return error;
}


XBOX::VError VAuthenticationReferee::LoadFromJSONFile (const XBOX::VFilePath& inFilePath)
{
	if (inFilePath.IsEmpty() || !inFilePath.IsValid())
		return XBOX::VE_INVALID_PARAMETER;

	XBOX::VError	error = XBOX::VE_OK;
	XBOX::VFile		file (inFilePath);

	if (file.Exists())
	{
		VJSONValue		jsonValue;
		XBOX::VString	patternString;

		error = VJSONImporter::ParseFile (&file, jsonValue, VJSONImporter::EJSI_Strict);

		if ((XBOX::VE_OK == error) && jsonValue.IsArray())
		{

			if (fMapLock.Lock())
			{
				fMap.clear();
				fMapLock.Unlock();

				VJSONArray *jsonArray = jsonValue.GetArray();
				for (sLONG i = 0; i < jsonArray->GetCount(); ++i)
				{
					VJSONValue elem = (*jsonArray)[i];
					if (elem.IsObject())
					{
						VHTTPResource *resource = new VHTTPResource();
						if (NULL != resource)
						{
							resource->LoadFromJSONValue (elem);

							if (!resource->GetURLMatch().IsEmpty())
								patternString.FromString (resource->GetURLMatch());
							else
								patternString.FromString (resource->GetURL());

							if (!patternString.IsEmpty() && (resource->GetDisallowedMethods() || resource->GetAllowedMethods()))
							{
								error = AddRetainReferee (patternString, resource);
								if (XBOX::VE_OK != error)
									break;
							}

							resource->Release();
						}
					}
				}
			}
		}
	}

	return error;
}


XBOX::VError VAuthenticationReferee::SaveToBag (XBOX::VValueBag *outBag)
{

	XBOX::VBagArray *bagArray = new XBOX::VBagArray ();
	if (NULL != bagArray)
	{
		XBOX::VTaskLock	lock (&fMapLock);

		for (RefereeMap::const_iterator it = fMap.begin(); it != fMap.end(); ++it)
		{
			if (!it->second.IsNull())
			{
				XBOX::VValueBag *bag = new XBOX::VValueBag ();
				if (NULL != bag)
				{
					(*it).second->SaveToBag (*bag);
					bagArray->AddTail (bag);
					bag->Release();
				}
			}
		}

		if (bagArray->GetCount())
			outBag->SetElements (RIASettingsKeys::Resources::kXmlElement, bagArray);

		XBOX::ReleaseRefCountable (&bagArray);
	}

#if 0 /*VERSIONDEBUG*/
	XBOX::VString xmlString ("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	outBag->DumpXML (xmlString, CVSTR ("settings"), true);
#endif

	return XBOX::VE_OK;
}


