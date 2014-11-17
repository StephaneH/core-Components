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


struct VVirtualHostAcceptPortFunctor
{
	VVirtualHostAcceptPortFunctor (const PortNumber inPort)
		: fPort (inPort)
	{
	}

	bool operator() (const std::pair<XBOX::VRegexMatcher *, VVirtualHost *>& inValueType) const
	{
		return inValueType.second->AcceptConnectionsOnPort (fPort);
	}

private:
	const PortNumber		fPort;
};


struct VVirtualHostAcceptAddressFunctor
{
	VVirtualHostAcceptAddressFunctor (const XBOX::VString& inIPAddress)
	: fIPAddress (inIPAddress)
	{
	}
	
	bool operator() (const std::pair<XBOX::VRegexMatcher *, VVirtualHost *>& inValueType) const
	{
		return inValueType.second->AcceptConnectionsOnAddress (fIPAddress);
	}

private:

	XBOX::VString			fIPAddress;
};


//--------------------------------------------------------------------------------------------------


VVirtualHostManager::VVirtualHostManager ()
: fHostPatternsMap ()
#if HTTP_SERVER_USE_PROJECT_PATTERNS
, fURLPatternsMap ()
#endif
, fVirtualHosts()
, fLock()
{
}


VVirtualHostManager::~VVirtualHostManager()
{
	fHostPatternsMap.erase (fHostPatternsMap.begin(), fHostPatternsMap.end());
#if HTTP_SERVER_USE_PROJECT_PATTERNS
	fURLPatternsMap.erase (fURLPatternsMap.begin(), fURLPatternsMap.end());
#endif
	fVirtualHosts.erase (fVirtualHosts.begin(), fVirtualHosts.end());
}


VVirtualHost *VVirtualHostManager::GetMatchingVirtualHost (const VHTTPRequest& inRequest)
{
	VVirtualHost *		virtualHost = NULL;
	XBOX::VTaskLock		lock (&fLock);

#if HTTP_SERVER_USE_PROJECT_PATTERNS
	// Try first to find a VirtualHost matching both host header & project application keyword (if any)
	for (VVirtualHostMap::const_iterator it = fURLPatternsMap.begin(); it != fURLPatternsMap.end(); ++it)
	{
		if (MatchRegex (it->first, inRequest.GetURL()))
		{
			if (it->second->MatchHostPattern (inRequest.GetHost()))
			{
				virtualHost = it->second;
				break;
			}
		}
	}
#endif
	
	if ((NULL == virtualHost) && (fVirtualHosts.size() > 0))
	{
		// We have one VirtualHost only... OK it's the easiest case --> just return it (no matter what host header contains)
		if (fVirtualHosts.size() == 1)
		{
			virtualHost = fVirtualHosts.at (0);
		}
		else
		{
			// Try to find a Virtual Host matching the port (no matter what address the request match)
			virtualHost = _FindMatchingVirtualHost (fHostPatternsMap, STRING_EMPTY, inRequest.GetLocalPort());
			// Other else try to find a VirtualHost matching the Host
			if (NULL == virtualHost)
			{
				virtualHost = _FindMatchingVirtualHost (fHostPatternsMap, inRequest.GetHost());

				if ((virtualHost != NULL) && !virtualHost->AcceptConnectionsOnPort(inRequest.GetLocalPort())) // YT 27-May-2014 - WAK0087800
					virtualHost = NULL;
			}
		}
	}

	return virtualHost;
}


XBOX::VError VVirtualHostManager::AddVirtualHost (const VVirtualHost *inVirtualHost)
{
	XBOX::VError error = XBOX::VE_INVALID_PARAMETER;

	if (NULL != inVirtualHost)
	{
		XBOX::VString	pattern;
		VVirtualHost *	virtualHost = const_cast<VVirtualHost *>(inVirtualHost);

		pattern.FromString (virtualHost->GetHostPattern());
		if (!pattern.IsEmpty())
		{
			/* Severals project can use the same hostPattern but they can NOT have the same projectPattern */
			error = _AddVirtualHost (fHostPatternsMap, virtualHost, pattern);
		}

#if HTTP_SERVER_USE_PROJECT_PATTERNS
		pattern.FromString (virtualHost->GetURLPattern());
		if (!pattern.IsEmpty())
		{
			if (!_VirtualHostAlreadyExist (fURLPatternsMap, pattern))
			{
				error = _AddVirtualHost (fURLPatternsMap, virtualHost, pattern);
			}
			else
			{
				error = VE_HTTP_SERVER_VIRTUAL_HOST_ALREADY_EXIST;
			}
		}
#endif
		
		if (error == VE_OK)
		{
			fVirtualHosts.push_back (virtualHost);
		}
	}

	return error;
} 


XBOX::VError VVirtualHostManager::RemoveVirtualHost (const VVirtualHost *inVirtualHost)
{
	if (NULL == inVirtualHost)
		return XBOX::VE_OK;

	XBOX::VTaskLock lock (&fLock);
	VVirtualHostMap::iterator it;

	for (it = fHostPatternsMap.begin(); it != fHostPatternsMap.end(); ++it)
	{
		if (it->second.Get() == inVirtualHost)
		{
			fHostPatternsMap.erase (it);
			break;
		}
	}

#if HTTP_SERVER_USE_PROJECT_PATTERNS
	for (it = fURLPatternsMap.begin(); it != fURLPatternsMap.end(); ++it)
	{
		if (it->second.Get() == inVirtualHost)
		{
			fURLPatternsMap.erase (it);
			break;
		}
	}
#endif
	
	VVirtualHostVector::iterator iter = std::find (fVirtualHosts.begin(), fVirtualHosts.end(), inVirtualHost);
	if (iter != fVirtualHosts.end())
		fVirtualHosts.erase (iter);

	return XBOX::VE_OK;
}


XBOX::VError VVirtualHostManager::_AddVirtualHost (VVirtualHostMap& inMap, VVirtualHost *inVirtualHost, const XBOX::VString& inPattern)
{
	XBOX::VError			error = XBOX::VE_OK;
	XBOX::VTaskLock			lock (&fLock);
	XBOX::VRegexMatcher *	matcher = XBOX::VRegexMatcher::Create (inPattern, &error);

	if (matcher && (XBOX::VE_OK == error))
	{
		inMap[matcher] = inVirtualHost;
	}

	XBOX::QuickReleaseRefCountable (matcher);

	return error;
}


VVirtualHost *VVirtualHostManager::_FindMatchingVirtualHost (const VVirtualHostMap& inMap, const XBOX::VString& inPattern)
{
	if (inPattern.IsEmpty() || inMap.empty())
		return NULL;

	VVirtualHost *	matchingHost = NULL;
	XBOX::VTaskLock	lock (&fLock);

	VVirtualHostMap::const_iterator it = std::find_if (inMap.begin(), inMap.end(), MatchVRegexMatcherFunctor<VVirtualHostRefPtr> (inPattern));
	if (it != inMap.end())
		matchingHost = it->second;

	return matchingHost;
}

VVirtualHost *VVirtualHostManager::_FindMatchingVirtualHost (const VVirtualHostMap& inMap, const XBOX::VString& inIPAddress, PortNumber inPort)
{
	if (inMap.empty())
		return NULL;

	VVirtualHost *	matchingHost = NULL;
	XBOX::VTaskLock	lock (&fLock);

	if (std::count_if (inMap.begin(), inMap.end(), VVirtualHostAcceptPortFunctor (inPort)) == 1)
	{
		VVirtualHostMap::const_iterator it = std::find_if (inMap.begin(), inMap.end(), VVirtualHostAcceptPortFunctor (inPort));
		if (it != inMap.end())
			matchingHost = it->second;
	}
	else if (!inIPAddress.IsEmpty() &&	// YT 08-Oct-2012 - WAK0078200
			!HTTPServerTools::EqualASCIIVString (inIPAddress, VNetAddress::GetAnyIP()) &&
			(std::count_if (inMap.begin(), inMap.end(), VVirtualHostAcceptAddressFunctor (inIPAddress)) == 1))
	{
		VVirtualHostMap::const_iterator it = std::find_if (inMap.begin(), inMap.end(), VVirtualHostAcceptAddressFunctor (inIPAddress));
		if (it != inMap.end())
			matchingHost = it->second;
	}

	return matchingHost;
}


bool VVirtualHostManager::_VirtualHostAlreadyExist (const VVirtualHostMap& inMap, const XBOX::VString& inPattern)
{
	if (inPattern.IsEmpty())
		return false;

	VVirtualHost *	matchingHost = NULL;
	XBOX::VTaskLock	lock (&fLock);

	VVirtualHostMap::const_iterator it = std::find_if (inMap.begin(), inMap.end(), EqualVRegexMatcherFunctor<VVirtualHostRefPtr> (inPattern));
	if (it != inMap.end())
		matchingHost = it->second;

	return (NULL != matchingHost);
}


bool VVirtualHostManager::_VirtualHostMatchPattern (const VVirtualHostMap& inMap, const XBOX::VString& inPattern)
{
	XBOX::VTaskLock	lock (&fLock);

	VVirtualHostMap::const_iterator it = std::find_if (inMap.begin(), inMap.end(), MatchVRegexMatcherFunctor<VVirtualHostRefPtr> (inPattern));
	if (it != inMap.end())
		return true;
	
	return false;
}


void VVirtualHostManager::SaveToBag (XBOX::VValueBag& outBag)
{
	outBag.SetLong (L"refCount", GetRefCount());

	XBOX::VBagArray *bagArray = new XBOX::VBagArray();
	if (bagArray)
	{
		XBOX::VTaskLock lock (&fLock);
		for (VVirtualHostVector::const_iterator it = fVirtualHosts.begin(); it != fVirtualHosts.end(); ++it)
		{
			XBOX::VValueBag *virtualHostBag = new XBOX::VValueBag();
			if (virtualHostBag)
			{
				(*it)->SaveToBag (*virtualHostBag);
				bagArray->AddTail (virtualHostBag);
				XBOX::QuickReleaseRefCountable (virtualHostBag);
			}
		}

		outBag.SetElements (L"virtualHosts", bagArray);
		XBOX::QuickReleaseRefCountable (bagArray);
	}
}


VVirtualFolder * VVirtualHostManager::RetainVirtualFolder (const XBOX::VString& inLocationPath, const XBOX::VString& inIndexPage, const XBOX::VString& inKeyword, XBOX::VFileSystemNamespace *inFileSystemNameSpace)
{
	VVirtualFolder * resultVirtualFolder = NULL;
	XBOX::VTaskLock	lock (&fLock);

	for (VVirtualHostVector::const_iterator it = fVirtualHosts.begin(); it != fVirtualHosts.end(); ++it)
	{
		if (NULL != (resultVirtualFolder = (*it)->RetainVirtualFolder (inLocationPath, inIndexPage, inKeyword, inFileSystemNameSpace)))
			break;
	}
	
	if (NULL == resultVirtualFolder)
	{
		resultVirtualFolder = new VVirtualFolder (inLocationPath, inIndexPage, inKeyword, CVSTR (""), false, inFileSystemNameSpace);

		if (resultVirtualFolder->IsLocalFolder())
		{
			xbox_assert (resultVirtualFolder->GetFolder() != NULL);

			if ((NULL != resultVirtualFolder->GetFolder())&& !resultVirtualFolder->GetFolder()->Exists())
			{
				XBOX::VString errorString;
				XBOX::VString folderPath (resultVirtualFolder->GetFolder()->GetPath().GetPath());
				errorString.Printf ("Virtual Folder (%S) does NOT exist !!", &folderPath);

				VHTTPServer::ThrowError (VE_VIRTUAL_FOLDER_DOES_NOT_EXIST, errorString);

				XBOX::ReleaseRefCountable (&resultVirtualFolder);
			}
		}
	}

	return resultVirtualFolder;
}
