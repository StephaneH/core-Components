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
	VVirtualHostAcceptAddressFunctor (const IP4 inIPv4Address)
		: fIPv4Address (inIPv4Address)
	{
	}

	bool operator() (const std::pair<XBOX::VRegexMatcher *, VVirtualHost *>& inValueType) const
	{
		return inValueType.second->AcceptConnectionsOnAddress (fIPv4Address);
	}

private:
	const IP4				fIPv4Address;
};


//--------------------------------------------------------------------------------------------------


VVirtualHostManager::VVirtualHostManager ()
: fSignal_DidStart (VSignal::ESM_Asynchonous)
, fSignal_DidStop (VSignal::ESM_Asynchonous)
, fHostPatternsMap ()
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


VVirtualHost *VVirtualHostManager::RetainMatchingVirtualHost (const VHTTPRequest& inRequest)
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
#if 0 //VERSIONDEBUG
			/*
				TODO: See with Julien & Sergiy if there's a way to retrieve the Server's IP/Port used for this endPoint
							and, then try to find a VirtualHost matching ipv4address and/or port
			*/
			XBOX::VString	ipv4AddrString;
			IP4			ipv4Addr = inRequest.GetEndPointIPv4(); // Peer Address (i need the Server address)
			PortNumber	port = inRequest.GetEndPointPort();		// Server Port

			HTTPServerTools::MakeIPv4String (ipv4Addr, ipv4AddrString);
#endif
			// Try to find a Virtual Host matching the port (no matter what address the request match)
			virtualHost = _FindMatchingVirtualHost (fHostPatternsMap, 0, inRequest.GetEndPointPort());

			// Other else try to find a VirtualHost matching the Host
			if (NULL == virtualHost)
				virtualHost = _FindMatchingVirtualHost (fHostPatternsMap, inRequest.GetHost());
		}
	}

	if (NULL != virtualHost)
		virtualHost->Retain();

	return virtualHost;
}


void VVirtualHostManager::Notify_DidStart()
{
	fSignal_DidStart();
}


void VVirtualHostManager::Notify_DidStop()
{
	fSignal_DidStop();
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


VVirtualHost *VVirtualHostManager::_FindMatchingVirtualHost (const VVirtualHostMap& inMap, IP4 inIPv4Address, PortNumber inPort)
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
	else if ((inIPv4Address != 0) && (std::count_if (inMap.begin(), inMap.end(), VVirtualHostAcceptAddressFunctor (inPort)) == 1))
	{
		VVirtualHostMap::const_iterator it = std::find_if (inMap.begin(), inMap.end(), VVirtualHostAcceptAddressFunctor (inIPv4Address));
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


VVirtualFolder * VVirtualHostManager::RetainVirtualFolder (const XBOX::VString& inLocationPath, const XBOX::VString& inIndexPage, const XBOX::VString& inKeyword)
{
	VVirtualFolder * resultVirtualFolder = NULL;
	XBOX::VTaskLock	lock (&fLock);

	for (VVirtualHostVector::const_iterator it = fVirtualHosts.begin(); it != fVirtualHosts.end(); ++it)
	{
		if (NULL != (resultVirtualFolder = (*it)->RetainVirtualFolder (inLocationPath, inIndexPage, inKeyword)))
			break;
	}
	
	if (NULL == resultVirtualFolder)
		resultVirtualFolder = new VVirtualFolder (inLocationPath, inIndexPage, inKeyword, CVSTR (""));

	return resultVirtualFolder;
}
