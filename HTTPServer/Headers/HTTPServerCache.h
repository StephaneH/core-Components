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
#ifndef __HTTP_SERVER_CACHE_INCLUDED__
#define __HTTP_SERVER_CACHE_INCLUDED__

#include <algorithm>
#include <map>
#include "HTTPServerTools.h"


USING_TOOLBOX_NAMESPACE

const bool	HTTP_DEFAULT_CACHE_ENABLED = true;
const sLONG HTTP_DEFAULT_CACHE_MAX_SIZE = 10 * 1024 * 1024;
const sLONG HTTP_DEFAULT_CACHED_OBJECT_MAX_SIZE = 512 * 1024;
const sLONG HTTP_DEFAULT_TIME_BETWEEN_CHECKS = 1 * 1000;


class VCachedObject;
class VHTTPResource;


struct hash_VCachedObject
{
	unsigned long operator() (const XBOX::VString& inString) const
	{
		unsigned long hash = 5381;
		const UniChar *charPtr = inString.GetCPointer();

		while (*charPtr != '\0')
		{
			UniChar c = *charPtr;

			if ((c >= CHAR_LATIN_CAPITAL_LETTER_A) && (c <= CHAR_LATIN_CAPITAL_LETTER_Z))
				c += 0x0020;

			/* hash = hash*33 + c */
			hash = ((hash << 5) + hash) + c;
			charPtr++;
		}

		return hash;
	}

	bool operator() (const XBOX::VString& inString1, const XBOX::VString& inString2) const
	{
		return (inString1.CompareToString (inString2) == XBOX::CR_SMALLER);
	}

	enum
	{	// parameters for visualstudio hash table
		bucket_size = 4,
		min_buckets = 8
	};
};

typedef	STL_HASH_MAP::hash_map<XBOX::VString, VCachedObject *, hash_VCachedObject>	MapOfVCachedObject;
typedef MapOfVCachedObject::iterator MapOfVCachedObjectIterator;
typedef MapOfVCachedObject::value_type MapOfVCachedObjectValueType;


typedef std::vector<XBOX::VRefPtr<VCachedObject> > VCachedObjectVector;
typedef VCachedObjectVector::iterator VCachedObjectVectorIterator;


class VCachedObject : public XBOX::VObject, public XBOX::IRefCountable
{
public:
							VCachedObject();
	virtual					~VCachedObject();
	void					Clear();


	// Last Modified is alwyas GMT
	const XBOX::VTime&		GetLastModified() const { return fLastModified; }
	void					SetLastModified (XBOX::VTime& inLastModified) { fLastModified = inLastModified; }

	sLONG					GetMaxAge() const { return fMaxAge; }
	void					SetMaxAge (sLONG inValue);

	sLONG					GetAge() const;

	bool					GetExpirationDate (XBOX::VTime& outExpirationValue) const;
	void					SetExpirationDate (const XBOX::VTime& inValue);

	const XBOX::VTime&		GetEntryDate() const { return fEntryDate; }

	const bool				GetIsStaticURL() const { return fIsStaticUrl; }
	void					SetIsStaticURL (bool inStaticURL) { fIsStaticUrl = inStaticURL; }

	const bool				IsCompressed() const { return fCompressionMode != COMPRESSION_NONE; }
	HTTPCompressionMethod	GetCompressionMode() const { return fCompressionMode; }
	void					SetCompressionMode (HTTPCompressionMethod inValue) { fCompressionMode = inValue; }

	const XBOX::VString&	GetMimeType() const { return fMimeType; }
	void					SetMimeType (const XBOX::VString& inValue) { fMimeType = inValue; }

	const uLONG				GetNbLoads() const { return fNbLoads; }
	void					IncrementLoad() { ++fNbLoads; }

	const XBOX::VSize		GetSize() const { return fCachedContent.GetDataSize(); }
			
	const XBOX::VFilePath&	GetFullPath() const { return fFullPath; }
	void					SetFullPath (const XBOX::VFilePath& inPath) { fFullPath.FromFilePath (inPath); }

	const XBOX::VString&	GetURL() const { return fUrl; }
	void					SetURL (const XBOX::VString& inURL) { fUrl.FromString (inURL); }

	void					GetCachedContent (void **outContent, XBOX::VSize& outContentSize);
	void					SetCachedContent (void *inContent, const XBOX::VSize inContentSize);

	void					SaveToBag (XBOX::VValueBag &ioValueBag);

	bool					Expired() const;

	static	bool			LoadComparator (const VCachedObject *inValue1, const VCachedObject *inValue2);
	static	bool			AgeComparator (const VCachedObject *inValue1, const VCachedObject *inValue2);
	static	bool			SizeComparator (const VCachedObject *inValue1, const VCachedObject *inValue2);

private:
	XBOX::VTime				fEntryDate;			//	Entry date in cache (usefull to calculate object's Age in cache)
	XBOX::VTime				fLastModified;		//	File Last Modification Date
	sLONG					fMaxAge;			//	Max age of the resource in cache
	XBOX::VTime				fExpirationDate;
	XBOX::VFilePath			fFullPath;			//	VFilePath of the resource in cache
	XBOX::VString			fUrl;				//	The URL used to get this page
	bool					fIsStaticUrl;		//	To be changed in some enum to cached the URL parsing
	HTTPCompressionMethod	fCompressionMode;	//	Cached Object Data compression mode (COMPRESSION_NONE, COMPRESSION_GZIP, COMPRESSION_DEFLATE)
	XBOX::VString			fMimeType;			//	Cached Object Mime-Type
	XBOX::VMemoryBuffer<>	fCachedContent;		//	Data of the cached file (only if VWebManager::fEnableDataCache == True)
	uLONG					fNbLoads;			//	Nb loads of this entry from the cache
};


class VCacheManager : public XBOX::VObject, public ICacheManager
{
public:
							VCacheManager ();
	virtual					~VCacheManager();

	virtual void			SetCacheMaxSize (const XBOX::VSize inSize);
	virtual XBOX::VSize		GetCacheMaxSize() const { return fCacheMaxSize; }

	virtual void			SetCachedObjectMaxSize (const XBOX::VSize inSize);
	virtual XBOX::VSize		GetCachedObjectMaxSize() const { return fCachedObjectMaxSize; }

	virtual void			SetEnableDataCache (bool inAllowToCacheData) { fEnableDataCache = inAllowToCacheData; }
	virtual bool			GetEnableDataCache() const { return fEnableDataCache; }

	virtual void			ClearCache();

	XBOX::VSize				GetCurrentSize() const { return fCurrentSize; }

	sLONG					GetPercentCacheUsed() const;

	/*	Add a page to WebCache. A URL and its data will be stored if fEnableDataCache == true, else only the URL	*/
	bool					AddPageToCache (const XBOX::VString& inURL,
											const XBOX::VString& inHost,
											const XBOX::VString& inContentType,
											void *inData,
											const XBOX::VSize inDataSize,
											const XBOX::VFilePath& inFullPath,
											XBOX::VTime &inLastChecked,
											XBOX::VTime &inLastModified,
											bool isStaticURL,
											HTTPCompressionMethod compressionMode = COMPRESSION_NONE,
											VCachedObject **outCachedObject = NULL);

	/*	To get any static page or cached page. You need to release the VCachedObject returned	*/
#if HTTP_SERVER_USE_SYSTEM_NOTIFICATIONS
	VCachedObject *			RetainPageFromCache (const XBOX::VString& inURL, const XBOX::VString& inHost, const XBOX::VString& inAcceptEncodingHeader);
#else
	VCachedObject *			RetainPageFromCache (	const XBOX::VString& inURL,
													const XBOX::VString& inHost,
													const XBOX::VString& inAcceptEncodingHeader,
													XBOX::VTime *inCheckLastModifiedTime = NULL,
													XBOX::VTime *inCheckLastUnmodifiedTime = NULL);
#endif

	bool					RemoveCachedObject (const XBOX::VFilePath& inFilePath);

	// Get a sorted list of the cache elems according to the number of hits
	// useful for stats
	// the caller is responsible for releasing the entries of the list
	void					GetSortedVectorCacheElements (VCachedObjectVector &outVector, HTTPServerCacheSortOption sortOption);

	void					SaveToBag (XBOX::VValueBag &ioValueBag, const XBOX::VString& inFilter, HTTPServerCacheSortOption sortOption = STATS_SORT_NONE);
	void					ToJSONString (XBOX::VString& outJSONString, const XBOX::VString& inFilter, bool prettyFormatting = false, HTTPServerCacheSortOption sortOption = STATS_SORT_NONE);
	void					ToXMLString (XBOX::VString& outXMLString, const XBOX::VString& inFilter, bool prettyFormatting = false, HTTPServerCacheSortOption sortOption = STATS_SORT_NONE);

	XBOX::VError			AddRetainResource (const XBOX::VString& inPattern, VHTTPResource *inResource);
	XBOX::VError			RemoveReleaseResource (const XBOX::VString& inPattern);
	VHTTPResource *			FindMatchingResource (const XBOX::VString& inURL);
	XBOX::VError			LoadRulesFromBag (const XBOX::VValueBag *inValueBag);

	/* Used for 4D GET WEB STATISTICS command only */
	void					GetStatistics (std::vector<XBOX::VString>& outPages, std::vector<sLONG>& outHits, sLONG& outUsage);

	/* Deal with caching settings */
	typedef std::map<XBOX::VRefPtr<XBOX::VRegexMatcher>, XBOX::VRefPtr<VHTTPResource> >	CacheRulesMap;

private:
	bool					_MakeRoom();			//	Quite expensive method (in time & load) to free the half of the cache memory

	XBOX::VSize				fCurrentSize;			//	The sum of the size of all CachedObjects
	bool					fEnableDataCache;		//	This allows to cache both URLs and data
	uLONG					fNbLoads;				//	The nb of hits on cachedObjects (i.e sum of the nbLoads of each page)

	MapOfVCachedObject *	fCacheMap;
	XBOX::VCriticalSection	fCacheMapLock;			//	Provides synchronous access to fCacheMap

	CacheRulesMap			fCacheRulesMap;
	XBOX::VCriticalSection	fCacheRulesMapLock;

	/*	!!	Please make sure that  at least (fCachedObjectMaxSize * 2) > fCacheMaxSize	!!	*/
	XBOX::VSize				fCacheMaxSize;			//	10 Mo as default value
	XBOX::VSize				fCachedObjectMaxSize;	//	512 Ko as default value. Must be added to the prefs
};


class VCacheManagerHTTPRequestHandler : public IHTTPRequestHandler
{
public:
							VCacheManagerHTTPRequestHandler() {};
	virtual XBOX::VError	GetPatterns (XBOX::VectorOfVString *outPatterns) const;
	virtual XBOX::VError	HandleRequest (IHTTPResponse *ioResponse);
};


#endif	// __HTTP_SERVER_CACHE_INCLUDED__
