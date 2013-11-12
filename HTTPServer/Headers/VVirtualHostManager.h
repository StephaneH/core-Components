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
#ifndef __VIRTUAL_HOST_MANAGER_INCLUDED__
#define __VIRTUAL_HOST_MANAGER_INCLUDED__


class VVirtualHost;


class VVirtualHostManager : public XBOX::VObject, public IRefCountable
{
public:
								VVirtualHostManager();
	virtual						~VVirtualHostManager();

	VVirtualHost *				GetMatchingVirtualHost (const VHTTPRequest& inRequest);

	XBOX::VError				AddVirtualHost (const VVirtualHost *inVirtualHost);
	XBOX::VError				RemoveVirtualHost (const VVirtualHost *inVirtualHost);

	VVirtualFolder *			RetainVirtualFolder (const XBOX::VString& inLocationPath, const XBOX::VString& inIndexPage, const XBOX::VString& inKeyword, XBOX::VFileSystemNamespace *inFileSystemNameSpace = NULL);

	void						SaveToBag (XBOX::VValueBag& outBag);

	typedef XBOX::VRefPtr<VVirtualHost>											VVirtualHostRefPtr;
	typedef std::map<XBOX::VRefPtr<XBOX::VRegexMatcher>, VVirtualHostRefPtr>	VVirtualHostMap;
	typedef std::vector<VVirtualHostRefPtr>										VVirtualHostVector;	

protected:
	XBOX::VError				_AddVirtualHost (VVirtualHostMap& inMap, VVirtualHost *inVirtualHost, const XBOX::VString& inPattern);
	VVirtualHost *				_FindMatchingVirtualHost (const VVirtualHostMap& inMap, const XBOX::VString& inPattern);

#if WITH_DEPRECATED_IPV4_API
	VVirtualHost *				_FindMatchingVirtualHost (const VVirtualHostMap& inMap, IP4 /*done*/ inIPv4Address, PortNumber inPort);
#else
	VVirtualHost *				_FindMatchingVirtualHost (const VVirtualHostMap& inMap, const XBOX::VString& inIPAddress, PortNumber inPort);
#endif
	
	bool						_VirtualHostAlreadyExist (const VVirtualHostMap& inMap, const XBOX::VString& inPattern);
	bool						_VirtualHostMatchPattern (const VVirtualHostMap& inMap, const XBOX::VString& inPattern);

private:
	VVirtualHostMap				fHostPatternsMap;
	VVirtualHostMap				fURLPatternsMap;
	VVirtualHostVector			fVirtualHosts;	
	VCriticalSection			fLock;
};


#endif // __VIRTUAL_HOST_MANAGER_INCLUDED__
