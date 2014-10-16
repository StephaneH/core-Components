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
#include "HTTPServerCache.h"


namespace NSCacheManager
{
	CREATE_BAGKEY (cacheUsage);
	CREATE_BAGKEY (numOfLoads);
	CREATE_BAGKEY (currentSize);
	CREATE_BAGKEY (maxSize);
	CREATE_BAGKEY (objectMaxSize);
	CREATE_BAGKEY (enabled);
	CREATE_BAGKEY (nbCachedObjects);
	CREATE_BAGKEY (nbFilteredCachedObjects);
	CREATE_BAGKEY (cachedObjects);
	namespace Rule
	{
		CREATE_BAGKEY (pattern);
		CREATE_BAGKEY (timeInCache);
	}
}


namespace NSCachedObject
{
	CREATE_BAGKEY (url);
	CREATE_BAGKEY (isStatic);
	CREATE_BAGKEY (encoding);
	CREATE_BAGKEY (nbLoads);
	CREATE_BAGKEY (fullPath);
	CREATE_BAGKEY (lastModified);
	CREATE_BAGKEY (dataSize);
	CREATE_BAGKEY (mimeType);
	CREATE_BAGKEY (entryDate);
	CREATE_BAGKEY (maxAge);
	CREATE_BAGKEY (expirationDate);
}


//--------------------------------------------------------------------------------------------------


inline
void _MakeCachedObjectKeyValueString (const XBOX::VString& inHostString, const XBOX::VString& inURL, XBOX::VString& outValueKeyString)
{
	/*
		CachedObject entry in map should look like "2FDE984A90C98546BE2834538D376D64/index.html"
		UUID identifies the Virtual Host
	*/
	outValueKeyString = inHostString + inURL;
}


//--------------------------------------------------------------------------------------------------


VCachedObject::VCachedObject()
: fLastModified()
, fFullPath()
, fUrl()
, fMimeType()
, fIsStaticUrl (false)
, fNbLoads (1)
, fCompressionMode (COMPRESSION_NONE)
, fCachedContent()
, fEntryDate()
, fMaxAge (0)	// -1: Unlimited
, fExpirationDate()
{
	XBOX::VTime::Now (fEntryDate);
}


VCachedObject::~VCachedObject()
{
	fCachedContent.Clear();
}


void VCachedObject::Clear()
{
	fLastModified.Clear();
	fIsStaticUrl = false;
	fNbLoads = 1;

	fFullPath.Clear();
	fUrl.Clear();
	fMimeType.Clear();

	fCachedContent.Clear();

	fCompressionMode = COMPRESSION_NONE;

	fEntryDate.Clear();

	fMaxAge = 0;
}


void VCachedObject::GetCachedContent (void **outContent, XBOX::VSize& outContentSize)
{
	if (NULL == outContent)
		return;

	*outContent = fCachedContent.GetDataPtr();
	outContentSize = fCachedContent.GetDataSize();
}


void VCachedObject::SetCachedContent (void *inContent, const XBOX::VSize inContentSize)
{
	if ((NULL == inContent) || (inContent <= 0))
		return;

	fCachedContent.PutData (0, inContent, inContentSize);
}


void VCachedObject::SaveToBag (VValueBag &ioValueBag)
{
	XBOX::VTime expirationDate;
	
	if (IsVTimeValid (fExpirationDate))
	{
		expirationDate.FromTime (fExpirationDate);
	}
	else
	{
		expirationDate.FromTime (fEntryDate);
		expirationDate.AddSeconds (fMaxAge);
	}

	ioValueBag.SetString (NSCachedObject::url, fUrl);
	ioValueBag.SetString (NSCachedObject::mimeType, fMimeType);

	if (COMPRESSION_NONE != fCompressionMode)
	{
		VString encoding;
		HTTPProtocol::GetEncodingMethodName (fCompressionMode, encoding);
		ioValueBag.SetString (NSCachedObject::encoding, encoding);
	}

	ioValueBag.SetBool (NSCachedObject::isStatic, fIsStaticUrl);
	ioValueBag.SetLong (NSCachedObject::nbLoads, fNbLoads);
	ioValueBag.SetTime (NSCachedObject::lastModified, fLastModified);
	ioValueBag.SetLong (NSCachedObject::dataSize, fCachedContent.GetDataSize());
	ioValueBag.SetTime (NSCachedObject::entryDate, fEntryDate);
	ioValueBag.SetLong (NSCachedObject::maxAge, fMaxAge);
	ioValueBag.SetTime (NSCachedObject::expirationDate, expirationDate);
}


bool VCachedObject::Expired() const
{
	XBOX::VTime currentTime;
	XBOX::VTime expirationDate;
	XBOX::VTime::Now (currentTime);

	if (IsVTimeValid (fExpirationDate))
	{
		expirationDate.FromTime (fExpirationDate);
	}
	else if (fMaxAge >= 0)
	{
		expirationDate.FromTime (fEntryDate);
		expirationDate.AddSeconds (fMaxAge);
	}

	if (IsVTimeValid (fExpirationDate) && (expirationDate.CompareTo (currentTime) == XBOX::CR_SMALLER))
	{
		return true;
	}

	return false;
}


void VCachedObject::SetMaxAge (sLONG inValue)
{
	fMaxAge = inValue;
}


void VCachedObject::SetExpirationDate (const XBOX::VTime& inValue)
{
	fExpirationDate.FromTime (inValue);
	/*
	XBOX::VTime expirationDate = inValue;
	if (fEntryDate.CompareTo (expirationDate) == XBOX::CR_SMALLER)
		fMaxAge = sLONG8(expirationDate.GetMilliseconds() - fEntryDate.GetMilliseconds()) / 1000;
	*/
}


sLONG VCachedObject::GetAge() const
{
	XBOX::VTime currentTime;
	XBOX::VTime::Now (currentTime);

	return ((currentTime.GetMilliseconds() - fEntryDate.GetMilliseconds()) / 1000);
}


bool VCachedObject::GetExpirationDate (XBOX::VTime& outExpirationDate) const
{
	if (IsVTimeValid (fExpirationDate))
	{
		outExpirationDate.FromTime (fExpirationDate);
		return true;
	}
	else if (fMaxAge > 0)
	{
		outExpirationDate.FromTime (fEntryDate);
		outExpirationDate.AddSeconds (fMaxAge);

		return true;
	}

	return false;
}


inline
bool VCachedObject::LoadComparator (const VCachedObject *inValue1, const VCachedObject *inValue2)
{
	return (inValue1->GetNbLoads() > inValue2->GetNbLoads());
}


inline
bool VCachedObject::AgeComparator (const VCachedObject *inValue1, const VCachedObject *inValue2)
{
	return (inValue1->GetEntryDate().GetMilliseconds() < inValue2->GetEntryDate().GetMilliseconds());
}


inline
bool VCachedObject::SizeComparator (const VCachedObject *inValue1, const VCachedObject *inValue2)
{
	return (inValue1->GetSize() < inValue2->GetSize());
}


//--------------------------------------------------------------------------------------------------


VCacheManager::VCacheManager ()
: fCurrentSize (0)
, fNbLoads (0)
, fEnableDataCache (HTTP_DEFAULT_CACHE_ENABLED)
, fCacheMaxSize (HTTP_DEFAULT_CACHE_MAX_SIZE)					//	Default value: 10 mo
, fCachedObjectMaxSize (HTTP_DEFAULT_CACHED_OBJECT_MAX_SIZE)	//	Default value: 512 ko
, fCacheMapLock()
{
	fCacheMap = new MapOfVCachedObject();
}


VCacheManager::~VCacheManager()
{
	ClearCache();

	delete fCacheMap;
	fCacheMap = NULL;

	fCacheRulesMap.clear();
}


void VCacheManager::ClearCache()
{
	if (NULL != fCacheMap)
	{
		XBOX::VTaskLock locker (&fCacheMapLock);

		// free the contents of the table
		for (MapOfVCachedObject::iterator it = fCacheMap->begin(); it != fCacheMap->end(); ++it)
			it->second->Release();

		// then free the table itself:
		fCacheMap->clear();

		fCurrentSize = 0;
		fNbLoads = 0;
	}
}


bool VCacheManager::AddPageToCache(	const XBOX::VString& inURL,
									const XBOX::VString& inHost,
									const XBOX::VString& inContentType,
									void *inData,
									const XBOX::VSize inDataSize,
									const XBOX::VFilePath& inFullPath,
									XBOX::VTime& inLastChecked,
									XBOX::VTime &inLastModified,
									bool isStaticURL,
									HTTPCompressionMethod compressionMode,
									VCachedObject **outCachedObject)
{
	bool onlyCacheURL = true;
	bool isOK = true;

	if (inURL.IsEmpty())
		return false;
	
	if ((0 == fCacheMaxSize) || !fEnableDataCache)
		onlyCacheURL = true;
	else
		onlyCacheURL = false;

	fCacheMapLock.Lock();
	if (inData && !onlyCacheURL && ((inDataSize + fCurrentSize) > fCacheMaxSize))
	{
		if (_MakeRoom())
		{
			onlyCacheURL = false;
		}
		else
		{
			onlyCacheURL = true;
		}
	}

	if (!onlyCacheURL && !inURL.IsEmpty() && (NULL != inData) && !inFullPath.IsEmpty())	// if we cache both data and URLs...
	{
		try
		{	
			//	we look if the url already exists
			XBOX::VString keyValueString;
			_MakeCachedObjectKeyValueString (inHost, inURL, keyValueString);
			MapOfVCachedObject::iterator it = fCacheMap->find (keyValueString);
			VCachedObject *cachedObject = NULL;

			// if it exists, we replace it
			if (it != fCacheMap->end())
			{
				cachedObject = it->second;
				fCurrentSize -= cachedObject->GetSize();
				fNbLoads -= cachedObject->GetNbLoads();

				cachedObject->SetLastModified (inLastModified);
				cachedObject->SetFullPath (inFullPath);
				cachedObject->SetIsStaticURL (isStaticURL);
				cachedObject->IncrementLoad();
				cachedObject->SetCachedContent (inData, inDataSize);
				cachedObject->SetCompressionMode (compressionMode);
				cachedObject->SetMimeType (inContentType);
			}
			else
			{
				cachedObject = new VCachedObject();

				cachedObject->SetURL (inURL);
				cachedObject->SetLastModified (inLastModified);
				cachedObject->SetFullPath (inFullPath);
				cachedObject->SetIsStaticURL (isStaticURL);
				cachedObject->SetCachedContent (inData, inDataSize);
				cachedObject->SetCompressionMode (compressionMode);
				cachedObject->SetMimeType (inContentType);

				VHTTPResource *resource = FindMatchingResource (inURL);
				if (NULL != resource)
				{
					XBOX::VTime expires (resource->GetExpires());

					if (IsVTimeValid (expires))
						cachedObject->SetExpirationDate (expires);
					else if (resource->GetLifeTime() > 0)
						cachedObject->SetMaxAge (resource->GetLifeTime());
				}
				/*
				else
				{
					cachedObject->SetMaxAge (1200);	// TODO: retrieve default value from settings
				}
				*/

				MapOfVCachedObjectValueType cachedObjectValue (keyValueString, cachedObject);
				fCacheMap->insert (cachedObjectValue);
			}

			fCurrentSize += inDataSize;
			++fNbLoads;

			if (NULL != outCachedObject)
				*outCachedObject = XBOX::RetainRefCountable (cachedObject);
		}
		catch (...)
		{	
			// Failed to insert in the cache
			isOK = false;
			assert (false);
		}
	}
	else
	{
		;
		/*
			... TODO: the web cache should be able to cache both URLs and files ...
		*/
	}

	fCacheMapLock.Unlock();

	return isOK;
}


bool VCacheManager::_MakeRoom()
{
#ifdef LOG_IN_CONSOLE
	VDebugTimer timer;
#endif

	bool	isOK = false;
	uLONG	nbLoadsAverage = 0;

	if ((fCacheMap->size() > 1) && (fNbLoads > 1))
	{
		try
		{
			/* 
			This operation can be quite long for a web cache. 
			So, to be efficient, we will free the half of the cache max size
			*/
			while ((fCurrentSize > (fCacheMaxSize / 2)) && (fCacheMap->size() > 1))
			{
				MapOfVCachedObjectIterator it = fCacheMap->begin();
				nbLoadsAverage = fNbLoads / (uLONG)fCacheMap->size();
				while (it != fCacheMap->end())
				{
					VCachedObject *cachedObject = it->second;

					/*
						We have average number of loads per CachedObjects.
						Let's delete all the objects that are less loaded than this value
					*/  
					if (cachedObject->Expired() || (cachedObject->GetNbLoads() <= nbLoadsAverage) || (cachedObject->GetSize() > fCachedObjectMaxSize))
					{
						fCurrentSize -= cachedObject->GetSize();
						fNbLoads -= cachedObject->GetNbLoads();
						cachedObject->Release();
						fCacheMap->erase (it++);
					}
					else
						++it;

					if (fCurrentSize < (fCacheMaxSize / 2))
						break;
				}

				if (fCurrentSize > (fCacheMaxSize / 2)) // YT 12-Mar-2013 - ACI0080851
					++nbLoadsAverage;
			}
			isOK = true;
		}
		catch (...)
		{	
			/*
			There was a problem during cleanup... I think we should clear all the cache, 
			but we need to be sure of the context when we call the method
			*/
			isOK = false;
		}
	}
	else
	{
		ClearCache();
		isOK = true;
	}

#ifdef LOG_IN_CONSOLE
	if (isOK)
		timer.DebugMsg ("* VCacheManager::_MakeRoom() exit with no error");
	else
		timer.DebugMsg ("* VCacheManager::_MakeRoom() exit with error");
#endif

	return isOK;
}


#if HTTP_SERVER_USE_SYSTEM_NOTIFICATIONS
VCachedObject *VCacheManager::RetainPageFromCache (const XBOX::VString& inURL, const XBOX::VString& inHost, const XBOX::VString& inAcceptEncodingHeader)
#else
VCachedObject *VCacheManager::RetainPageFromCache (const XBOX::VString& inURL, const XBOX::VString& inHost, const XBOX::VString& inAcceptEncodingHeader, XBOX::VTime *inCheckLastModifiedTime, XBOX::VTime *inCheckLastUnmodifiedTime)
#endif
{
	if (NULL == fCacheMap)	
        return NULL;

#if LOG_IN_CONSOLE
	VDebugTimer timer;
#endif

	XBOX::VTaskLock	lock (&fCacheMapLock);
	VCachedObject *	cachedObject = NULL;

	try
	{
		XBOX::VString keyValueString;
		_MakeCachedObjectKeyValueString (inHost, inURL, keyValueString);
		MapOfVCachedObjectIterator it = fCacheMap->find (keyValueString);

		if (it != fCacheMap->end())
		{
			VCachedObject *foundObject = it->second;

			if (foundObject->Expired())
			{
				fCurrentSize -= foundObject->GetSize();
				fNbLoads -= foundObject->GetNbLoads();
				foundObject->Release();
				fCacheMap->erase (it);
				cachedObject = NULL;
			}
			else
			{
				bool objectChanged = false;

#if !HTTP_SERVER_USE_SYSTEM_NOTIFICATIONS
				XBOX::VTime modificationDate;
				if (HTTPServerTools::GetFileInfos (foundObject->GetFullPath(), &modificationDate, NULL) == XBOX::VE_FILE_NOT_FOUND)
				{
					objectChanged = true; // File does NOT exist anymore...
				}
				else
				{
					if (((NULL != inCheckLastModifiedTime) && (*inCheckLastModifiedTime != modificationDate)) ||
						((NULL != inCheckLastUnmodifiedTime) && (*inCheckLastUnmodifiedTime == modificationDate)) ||
						(foundObject->GetLastModified() != modificationDate))
					{
						objectChanged = true;
					}
				}
#endif

				// We need to remove the first '/' to use GetWebFile
				if (objectChanged || !HTTPProtocol::AcceptEncodingMethod (inAcceptEncodingHeader, foundObject->GetCompressionMode()))
				{
					// if the file has changed, remove the entry and act as if there was no hit...
					// the normal runtime path will recreate the correct entry
					fCurrentSize -= foundObject->GetSize();
					fNbLoads -= foundObject->GetNbLoads();
					foundObject->Release();
					fCacheMap->erase (it);
					cachedObject = NULL;
				}
				else	// not modified
				{
					foundObject->IncrementLoad();
					foundObject->Retain();
					cachedObject = foundObject;
				}

				++fNbLoads;
			}
		}
	}
	catch(...)
	{
		cachedObject = NULL;
	}

#if LOG_IN_CONSOLE
	timer.DebugMsg ("* VCacheManager::RetainPageFromCache()");
#endif

	return cachedObject;
}


bool VCacheManager::RemoveCachedObject (const XBOX::VFilePath& inFilePath)
{
	if (NULL == fCacheMap)	
        return false;

	XBOX::VTaskLock	lock (&fCacheMapLock);

	try
	{
		for (MapOfVCachedObject::iterator it = fCacheMap->begin(); it != fCacheMap->end(); ++it)
		{
			if (it->second->GetFullPath() == inFilePath)
			{
				fCurrentSize -= it->second->GetSize();
				fNbLoads -= it->second->GetNbLoads();
				it->second->Release();
				fCacheMap->erase (it);
				return true;
				break;
			}
		}
	}
	catch(...)
	{
		return false;
	}

	return false;
}


void VCacheManager::GetSortedVectorCacheElements (VCachedObjectVector &outVector, HTTPServerCacheSortOption sortOption)
{
#if LOG_IN_CONSOLE
	VDebugTimer timer;
#endif

	XBOX::VTaskLock	lock (&fCacheMapLock);

	for (MapOfVCachedObjectIterator it = fCacheMap->begin(); it != fCacheMap->end(); ++it)
		outVector.push_back (it->second);

	switch (sortOption)
	{
	case STATS_SORT_BY_LOADS:
		std::sort (outVector.begin(), outVector.end(), VCachedObject::LoadComparator);
		break;
	case STATS_SORT_BY_AGE:
		std::sort (outVector.begin(), outVector.end(), VCachedObject::AgeComparator);
		break;
	case STATS_SORT_BY_SIZE:
		std::sort (outVector.begin(), outVector.end(), VCachedObject::SizeComparator);
		break;
    default:
        break;
	}

#if LOG_IN_CONSOLE
	timer.DebugMsg ("* VCacheManager::GetSortedVectorCacheElements()");
#endif
}


/* Cache Statistics */
sLONG VCacheManager::GetPercentCacheUsed() const
{
	if (0 == fCacheMaxSize)
		return 0;

	// pour eviter l'overflow
	XBOX::VSize	maxSize = 0;
	XBOX::VSize	curSize = (fCurrentSize >> 10);

	curSize	*= 100;
	maxSize = fCacheMaxSize >> 10;

	if (!maxSize)
		return 0;

	return sLONG ((float)curSize / (float)maxSize);
}


void VCacheManager::SetCacheMaxSize (const XBOX::VSize inSize)
{
	if (inSize < (fCachedObjectMaxSize * 2))
		return;

	fCacheMaxSize = inSize;
	
	if (inSize < fCurrentSize)
	{
		XBOX::VTaskLock lock (&fCacheMapLock);
		_MakeRoom();
	}
}


void VCacheManager::SetCachedObjectMaxSize(const XBOX::VSize inSize)
{
	if (inSize > (fCacheMaxSize / 2))
		return;

	bool isSmaller = (inSize < fCachedObjectMaxSize);

	fCachedObjectMaxSize = inSize;

	if (isSmaller)
	{
		XBOX::VTaskLock lock (&fCacheMapLock);
		_MakeRoom();
	}
}


void VCacheManager::SaveToBag (XBOX::VValueBag &ioValueBag, const XBOX::VString& inFilter, HTTPServerCacheSortOption sortOption)
{
	XBOX::VTaskLock lock (&fCacheMapLock);

	ioValueBag.SetLong (NSCacheManager::cacheUsage, VCacheManager::GetPercentCacheUsed());
	ioValueBag.SetLong (NSCacheManager::numOfLoads, fNbLoads);
	ioValueBag.SetLong (NSCacheManager::currentSize, fCurrentSize);
	ioValueBag.SetLong (NSCacheManager::maxSize, fCacheMaxSize);
	ioValueBag.SetLong (NSCacheManager::objectMaxSize, fCachedObjectMaxSize);
	ioValueBag.SetBool (NSCacheManager::enabled, fEnableDataCache);
	ioValueBag.SetLong (NSCacheManager::nbCachedObjects, fCacheMap->size());

	if (STATS_SORT_NONE == sortOption)
	{
		if (fCacheMap->size())
		{
			bool emptyFilter = (inFilter.IsEmpty() || inFilter.EqualToUSASCIICString ("*"));
			XBOX::VBagArray *cachedObjects = new VBagArray();
			for (MapOfVCachedObjectIterator it = fCacheMap->begin(); it != fCacheMap->end(); ++it)
			{
				if (emptyFilter || (HTTPServerTools::FindASCIIVString (it->second->GetMimeType(), inFilter)))
				{
					XBOX::VValueBag *valueBag = new XBOX::VValueBag();
					it->second->SaveToBag (*valueBag);
					cachedObjects->AddTail (valueBag);
					valueBag->Release();
				}
			}

			if (!inFilter.IsEmpty())
				ioValueBag.SetLong (NSCacheManager::nbFilteredCachedObjects, fCacheMap->size() - cachedObjects->GetCount());

			if (cachedObjects->GetCount())
				ioValueBag.SetElements (NSCacheManager::cachedObjects, cachedObjects);

			cachedObjects->Release();
		}
	}
	else
	{
		VCachedObjectVector sortedVector;

		GetSortedVectorCacheElements (sortedVector, sortOption);

		if (sortedVector.size())
		{
			bool emptyFilter = (inFilter.IsEmpty() || inFilter.EqualToUSASCIICString ("*"));
			XBOX::VBagArray *cachedObjects = new VBagArray();
			for (VCachedObjectVector::const_iterator it = sortedVector.begin(); it != sortedVector.end(); ++it)
			{
				if (emptyFilter || (HTTPServerTools::FindASCIIVString (it->Get()->GetMimeType(), inFilter)))
				{
					XBOX::VValueBag *valueBag = new XBOX::VValueBag();
					it->Get()->SaveToBag (*valueBag);
					cachedObjects->AddTail (valueBag);
					valueBag->Release();
				}
			}

			if (!inFilter.IsEmpty())
				ioValueBag.SetLong (NSCacheManager::nbFilteredCachedObjects, fCacheMap->size() - cachedObjects->GetCount());

			if (cachedObjects->GetCount())
				ioValueBag.SetElements (NSCacheManager::cachedObjects, cachedObjects);

			cachedObjects->Release();
		}
	}
}


void VCacheManager::ToJSONString (XBOX::VString& outJSONString, const XBOX::VString& inFilter, bool prettyFormatting, HTTPServerCacheSortOption sortOption)
{
	XBOX::VValueBag bag;
	SaveToBag (bag, inFilter, sortOption);
	bag.GetJSONString (outJSONString, prettyFormatting ? JSON_PrettyFormatting : JSON_Default);
}


void VCacheManager::ToXMLString (XBOX::VString& outXMLString, const XBOX::VString& inFilter, bool prettyFormatting, HTTPServerCacheSortOption sortOption)
{
	XBOX::VValueBag bag;
	SaveToBag (bag, inFilter, sortOption);
	outXMLString.FromCString ("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	bag.DumpXML (outXMLString,  L"cacheManagerStatistics", prettyFormatting);
}


XBOX::VError VCacheManager::AddResource (const XBOX::VString& inPattern, VHTTPResource *inResource)
{
	XBOX::VError error = XBOX::VE_OK;

	if (!inPattern.IsEmpty())
	{
		XBOX::VTaskLock lock (&fCacheRulesMapLock);

		CacheRulesMap::const_iterator cacheRule = std::find_if (fCacheRulesMap.begin(), fCacheRulesMap.end(), EqualVRegexMatcherFunctor<XBOX::VRefPtr<VHTTPResource> > (inPattern));
		if (cacheRule == fCacheRulesMap.end())
		{
			XBOX::VRegexMatcher *matcher = XBOX::VRegexMatcher::Create (inPattern, &error);

			if ((NULL != matcher) && (error == VE_OK))
			{
				fCacheRulesMap[matcher] = inResource;
			}

			XBOX::QuickReleaseRefCountable (matcher);
		}
		else
		{
			error = VE_CACHE_RULE_ALREADY_EXISTS;
		}
	}

	return error;
}


XBOX::VError VCacheManager::RemoveResource (const XBOX::VString& inPattern)
{
	XBOX::VError error = XBOX::VE_OK;

	if (!inPattern.IsEmpty())
	{
		XBOX::VTaskLock lock (&fCacheRulesMapLock);

		CacheRulesMap::iterator cacheRule = std::find_if (fCacheRulesMap.begin(), fCacheRulesMap.end(), EqualVRegexMatcherFunctor<XBOX::VRefPtr<VHTTPResource> > (inPattern));
		if (cacheRule != fCacheRulesMap.end())
		{
			fCacheRulesMap.erase (cacheRule);
		}
		else
		{
			error = VE_CACHE_RULE_DOES_NOT_EXIST;
		}
	}

	return error;
}


VHTTPResource *VCacheManager::FindMatchingResource (const XBOX::VString& inURL)
{
	VHTTPResource *	result = NULL;
	XBOX::VTaskLock	lock (&fCacheRulesMapLock);

	CacheRulesMap::const_iterator it = std::find_if (fCacheRulesMap.begin(), fCacheRulesMap.end(), MatchVRegexMatcherFunctor<XBOX::VRefPtr<VHTTPResource> > (inURL));
	if (it != fCacheRulesMap.end())
	{
		result = (*it).second;
	}

	return result;
}


XBOX::VError VCacheManager::LoadRulesFromBag (const XBOX::VValueBag *inValueBag)
{
	if (NULL == inValueBag)
		return XBOX::VE_INVALID_PARAMETER;

	XBOX::VError error = XBOX::VE_OK;

	/* Resources settings */
	const XBOX::VBagArray *resourcesSettings = RetainMultipleSettings (inValueBag, RIASettingsKeys::Resources::kXmlElement);
	if (NULL != resourcesSettings)
	{
		XBOX::VString	patternString;

		for (XBOX::VIndex i = 1; i <= resourcesSettings->GetCount(); ++i)
		{
			const XBOX::VValueBag *bag = resourcesSettings->GetNth (i);
			if (NULL != bag)
			{
				VHTTPResource *resource = new VHTTPResource();
				if (NULL != resource)
				{
					resource->LoadFromBag (*bag);

					if (!resource->GetURLMatch().IsEmpty())
						patternString.FromString (resource->GetURLMatch());
					else
						patternString.FromString (resource->GetURL());

					if (!patternString.IsEmpty() && (resource->GetLifeTime() > 0))
					{
						error = AddResource (patternString, resource);
						if ((XBOX::VE_OK != error) && (VE_CACHE_RULE_ALREADY_EXISTS != error))
							break;
					}

					resource->Release();
				}
			}
		}

		XBOX::QuickReleaseRefCountable (resourcesSettings);
	}

	return error;
}


void VCacheManager::GetStatistics (std::vector<XBOX::VString>& outPages, std::vector<sLONG>& outHits, sLONG& outUsage)
{
	outPages.clear();
	outHits.clear();
	
	VCachedObjectVector cachedObjects;

	GetSortedVectorCacheElements (cachedObjects, STATS_SORT_BY_LOADS);

	if (cachedObjects.size())
	{
		for (VCachedObjectVector::const_iterator it = cachedObjects.begin(); it != cachedObjects.end(); ++it)
		{
			outPages.push_back ((*it)->GetURL());
			outHits.push_back ((*it)->GetNbLoads());
		}
	}


	outUsage = GetPercentCacheUsed();
}


//--------------------------------------------------------------------------------------------------


XBOX::VError VCacheManagerHTTPRequestHandler::GetPatterns (XBOX::VectorOfVString *outPatterns) const
{
	if (NULL == outPatterns)
		return VE_HTTP_INVALID_ARGUMENT;

	outPatterns->clear();
	outPatterns->push_back (CVSTR ("(?i)/cache$"));
	return XBOX::VE_OK;
}


XBOX::VError VCacheManagerHTTPRequestHandler::HandleRequest (IHTTPResponse *ioResponse)
{
	if ((NULL == ioResponse) || (NULL == dynamic_cast<VHTTPResponse *>(ioResponse)))
		return VHTTPServer::ThrowError (VE_HTTP_PROTOCOL_INTERNAL_SERVER_ERROR, STRING_ERROR_INVALID_PARAMETER);

	VHTTPResponse *				response = dynamic_cast<VHTTPResponse *>(ioResponse);
	VVirtualHost *				virtualHost = (NULL != response) ? dynamic_cast<VVirtualHost *>(response->GetVirtualHost()) : NULL;
	VAuthenticationManager *	authenticationManager = (NULL != virtualHost) ? dynamic_cast<VAuthenticationManager *>(virtualHost->GetProject()->GetAuthenticationManager()) : NULL;

	if (NULL != authenticationManager && (authenticationManager->CheckAdminAccessGranted (ioResponse) != XBOX::VE_OK))
		return VE_HTTP_PROTOCOL_UNAUTHORIZED;

	VCacheManager *	cacheManager = (NULL != virtualHost) ? virtualHost->GetProject()->GetHTTPServer()->GetCacheManager() : NULL;

	if (NULL != cacheManager)
	{
		const XBOX::VString	stringURL = ioResponse->GetRequest().GetURLQuery();
		HTTPRequestMethod	method = ioResponse->GetRequest().GetRequestMethod();
		const UniChar *		stringPtr = stringURL.GetCPointer();
		const sLONG			stringLen = stringURL.GetLength();

		/* Allow Compression of response body */
		ioResponse->AllowCompression (true);

		if ((HTTP_GET == method) || (HTTP_HEAD == method))
		{
			sLONG						posFilter = 0;
			sLONG						posSort = 0;
			HTTPServerCacheSortOption	sortOption = STATS_SORT_NONE;
			XBOX::VValueBag				bag;
			XBOX::VString				filterString;

			if ((posFilter = HTTPServerTools::FindASCIICString (stringPtr, "filter=")) > 0)
			{
				posFilter += 6;
				sLONG endPos = HTTPServerTools::FindASCIICString (stringPtr + posFilter, "&");
				if (endPos <= 0)
					endPos = stringLen;
				else
					endPos += (posFilter - 1);

				if (endPos > posFilter)
					HTTPServerTools::GetSubString (stringURL, posFilter, endPos - 1, filterString);
			}

			if ((posSort = HTTPServerTools::FindASCIICString (stringPtr, "sort=")) > 0)
			{
				XBOX::VString sortString;

				posSort += 4;
				sLONG endPos = HTTPServerTools::FindASCIICString (stringPtr + posSort, "&");
				if (endPos <= 0)
					endPos = stringLen;
				else
					endPos += (posSort - 1);

				if (endPos > posSort)
					HTTPServerTools::GetSubString (stringURL, posSort, endPos - 1, sortString);

				if (!sortString.IsEmpty())
				{
					if (HTTPServerTools::EqualASCIICString (sortString, "load"))
						sortOption = STATS_SORT_BY_LOADS;
					else if (HTTPServerTools::EqualASCIICString (sortString, "age"))
						sortOption = STATS_SORT_BY_AGE;
					else if (HTTPServerTools::EqualASCIICString (sortString, "size"))
						sortOption = STATS_SORT_BY_SIZE;
				}
			}

			cacheManager->SaveToBag (bag, filterString, sortOption);

			HTTPServerTools::SendValueBagResponse (*ioResponse, bag, CVSTR ("cacheManagerStatistics"));
		}
		else if (HTTP_POST == method)
		{
			sLONG	posAction = 0;

			if ((posAction = HTTPServerTools::FindASCIICString (stringPtr, "action=")) > 0)
			{
				posAction += 6;
				if (HTTPServerTools::FindASCIICString (stringPtr + posAction, "clear") > 0)
					cacheManager->ClearCache();
				else if (HTTPServerTools::FindASCIICString (stringPtr + posAction, "disable") > 0)
					cacheManager->SetEnableDataCache(false);
				else if (HTTPServerTools::FindASCIICString (stringPtr + posAction, "enable") > 0)
					cacheManager->SetEnableDataCache(true);
				
				ioResponse->ReplyWithStatusCode (HTTP_OK);
			}
			else
			{
				ioResponse->ReplyWithStatusCode (HTTP_BAD_REQUEST);
			}
		}
		else
		{
			ioResponse->ReplyWithStatusCode (HTTP_BAD_REQUEST);
		}

		return VE_OK;
	}

	return VHTTPServer::ThrowError (VE_HTTP_PROTOCOL_INTERNAL_SERVER_ERROR, STRING_EMPTY);
}
