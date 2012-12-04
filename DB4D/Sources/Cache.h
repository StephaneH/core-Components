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
#ifndef __CACHE__
#define __CACHE__

#include <map>

class BtreeCache;
class VDBMgr;

struct BTitemCache
{
	ObjCache *obj;
	BtreeCache *sousBT;
	sLONG ordre;
	uBOOL pris;
};

#if VERSIONDEBUG
#define CHECKCACHE 	VDBMgr::GetCacheManager()->Check()
#define CHECKCACHEDUP 	VDBMgr::GetCacheManager()->CheckDup()
#define CHECKCACHEFIND(x)	VDBMgr::GetCacheManager()->DebugFindPage(x)
#else
#define CHECKCACHE
#define CHECKCACHEDUP
#define CHECKCACHEFIND(x)
#endif


#if VERSIONDEBUG
/*
class debug_CacheTypObjRef
{
	public:
		inline debug_CacheTypObjRef() { typobj = 0; count = 0; };

		inline Boolean operator == ( const debug_CacheTypObjRef &other ) const { return typobj == other.typobj; };
		inline Boolean operator  > ( const debug_CacheTypObjRef &other ) const { return typobj > other.typobj; };
		inline Boolean operator  < ( const debug_CacheTypObjRef &other ) const { return typobj < other.typobj; };
		inline Boolean operator >= ( const debug_CacheTypObjRef &other ) const { return typobj >= other.typobj; };
		inline Boolean operator <= ( const debug_CacheTypObjRef &other ) const { return typobj <= other.typobj; };
		inline Boolean operator != ( const debug_CacheTypObjRef &other ) const { return typobj != other.typobj; };

		sLONG typobj;
		sLONG count;
};
*/


#endif

#if VERSIONDEBUG
typedef std::map<sLONG, sLONG> debug_CacheObjRef;
#endif

const VSize kMaxNeedMem = 0xFFFFFFFF;

const sLONG kNbElemInTreeCache = 32;
const sLONG kHalfTreeCache = 16;

extern sLONG gCacheIsWaitingFlush;

class BtreeCache : public ObjInCacheMemory
{
public:
	BtreeCache(void) {nkeys=0;tabsous[0]=nil;};

	void search(uBOOL *h, BTitemCache *v, sLONG level);
	
	uBOOL ViderCache(sLONG allocationBlockNumber, VSize need, VSize *tot, sLONG level, sLONG& ioCountFreedObjects, sLONG& ioDebugcurcompteobj, sLONG& outRemaining,
					Boolean& BigObjectsNeedToBeFreedByOtherTask);
	void SubCopyInto(BtreeCache* into, sLONG& startPos);
	void Equilibre(void);
	
	sLONG CompteObj(void);

	#if VERSIONDEBUG
	void DebugFindPage(BtreeCache *sbt);
	void CheckPage(sLONG *curval);
	void CheckDupPage(void);
	virtual uBOOL CheckObjInMem(void);
	sLONG CompteAddr(DataAddr4D xaddr);
	void DisplayTree(void);
	void CheckPageIndexFixSize(void);
	void BuildCacheStat(debug_CacheObjRef& stats);
	#endif

	void xInitFromItem( const BTitemCache& u, BtreeCache *sousbt);
	
protected:
	ObjCache* tabmem[kNbElemInTreeCache];
	BtreeCache* tabsous[kNbElemInTreeCache+1];
	sLONG tabordre[kNbElemInTreeCache];
	uBOOL tabpris[kNbElemInTreeCache];
	sLONG nkeys;

	#if VERSIONDEBUG
	static Boolean fCheckCacheWithFlush;
	#endif
};


typedef list<IObjToFree*> CacheList;



class VDBCacheMgr : public VObject
{
public:
	enum{
		kMaxFreePages=20,
		//kMinimumCache = 33554432	/*32 Mo*/
		kMinimumCache = 100*1024*1024	/*100 Mo*/
	};

	static VDBCacheMgr *NewCacheManager( VDBMgr *inManager);
	virtual ~VDBCacheMgr();
	Boolean Init();

	void PutObject(ObjCache *inObject, Boolean checkatomic=true);
	void RemoveObject(ObjCache *inObject);

	void PutObject(IObjToFree *inObject, Boolean checkatomic=true);
	void RemoveObject(IObjToFree *inObject);

	VSize NeedsBytes( sLONG allocationBlockNumber, VSize requiredBytes, bool withFlush);
	static VSize CallNeedsBytes( sLONG allocationBlockNumber, VSize inNeededBytes, bool withFlush);
	
	VError SetMaxSize(VSize inSize, bool inPhysicalMemOnly, VSize *outMaxSize);
	VError SetMinSizeToFlush( VSize inMinSizeToFlush);

	sLONG CompteObj();
	
	BtreeCache* xNextFreeCachePage();
	void xCheckFreeCachePages();
	void xDoisEquilibrer() 	{fDoisEquilibrer = true;}

	#if VERSIONDEBUG
	void xTestAddr(DataAddr4D inAddr);
	void Dump();
	void Check();
	void CheckDup();
	void DebugFindPage(BtreeCache *inPage);
	void CheckPageIndexFixSize();
	void DisplayCacheStat(Boolean before);
	#endif
	
	void* NewPtr( VSize inNbBytes, Boolean inIsVObject, sLONG inTag, sLONG preferedBlock = -1);
	void DisposePtr( void *inBlock);
	inline VSize GetPtrSize( const void *inBlock)	{return fCache.GetPtrSize( (VPtr) inBlock);}
	inline void*	SetPtrSize (void* ioPtr, VSize inNbBytes) { return (void*)fCache.SetPtrSize((VPtr)ioPtr, inNbBytes); };

	VCppMemMgr *GetMemoryManager()	{return &fCache;}
	inline VSize GetMaxSize() { return fMaxAlloc; };
	inline VSize GetMinSizeToFlush() { return fMinSizeToFlush; };

	inline sLONG GetWaitingForOtherToFreeBigObjectsStamp()
	{
		VTaskLock lock(&fNeedsByteMutex);
		return fWaitingForOtherToFreeBigObjectsStamp;
	}

	void ClearWaitListForBigObject();

	void GetMemUsageInfo(VSize& outTotalMem, VSize& outUsedMem)
	{
		fCache.GetMemUsageInfo(outTotalMem, outUsedMem);
	}

	void CallFreeMemObj(sLONG allocationBlockNumber, VSize inNeededBytes, IObjToFree* obj);


private:
	VDBCacheMgr( VDBMgr *inManager);
	
	VDBMgr	*fManager;
	
	VCppMemMgr	fCache;

	CacheList fBigObjs;

	#if VERSIONDEBUG
	Boolean	fCheckCache;
	#endif
	
	Boolean fNeedsFreePages;
	Boolean fPeutEquilibrer;
	Boolean fDoisEquilibrer;

	VCriticalSection fMutex;
	VSyncEvent* fNeedsByteSyncEvent;
	VCriticalSection fNeedsByteMutex;
	
	BtreeCache *fTree;
	BtreeCache *fTabFreePages[kMaxFreePages];
	
	VSize fMaxAlloc;
	VSize fMinSizeToFlush;

	VSize fNeededMem;
	VSize fFreedMem;
	vxSyncEvent* fWaitingForOtherToFreeBigObjects;
	sLONG fWaitingForOtherToFreeBigObjectsStamp;
	CacheList::iterator fCurBigObjToFree;
	bool fCurBigObjToFreeisValid;
};


#if debug_nonmemmgr
inline void *GetFastMem(VSize inNbBytes, Boolean ForAnObject, sLONG inTag) { return malloc(inNbBytes); };
inline void FreeFastMem(void *inBlock) { free(inBlock); };
inline VSize GetFastMemSize( const void *inBlock) { return 200; };
inline void* SetFastMemSize(void *inBlock, VSize inNbBytes) { return realloc(inBlock,inNbBytes); };
#else
inline void *GetFastMem(VSize inNbBytes, Boolean ForAnObject, sLONG inTag, sLONG preferedBlock = -1) { return VDBMgr::GetManager()->GetCacheManager()->NewPtr((inNbBytes+31) & -32, ForAnObject, inTag, preferedBlock); };
inline void FreeFastMem(void *inBlock) { return VDBMgr::GetManager()->GetCacheManager()->DisposePtr( inBlock); };
inline VSize GetFastMemSize( const void *inBlock) { return VDBMgr::GetManager()->GetCacheManager()->GetPtrSize( inBlock); };
inline void* SetFastMemSize(void *inBlock, VSize inNbBytes) { return VDBMgr::GetManager()->GetCacheManager()->SetPtrSize(inBlock, inNbBytes); };
#endif

inline sLONG GetAllocationNumber(void* inBlock)
{
	return VDBMgr::GetManager()->GetCacheManager()->GetMemoryManager()->GetAllocationBlockNumber(inBlock);
}

inline bool OKAllocationNumber(void* inBlock, sLONG allocnum)
{
	return (allocnum == -1) || (GetAllocationNumber(inBlock) == allocnum);
}


#if VERSIONDEBUG
inline bool CheckFastMem() { return VDBMgr::GetManager()->GetCacheManager()->GetMemoryManager()->CheckNow(); };
#else
inline bool CheckFastMem() { return true; };
#endif



#if 0
class StAllocateInCache
{
public:
	StAllocateInCache():fTask(VTask::GetCurrent())
	{
		fWasAlternate = fTask->IsAlternateCurrentAllocator();
		//fTask->SetCurrentAllocator( true);
		fTask->SetCurrentAllocator(false);
	}

	~StAllocateInCache()
	{
		fTask->SetCurrentAllocator( fWasAlternate);
	}

private:
	Boolean fWasAlternate;
	VTask* fTask;
};
#endif

typedef sLONG StAllocateInCache;


inline void* GetFastMemForSTL(size_t len, Boolean ForAnObject, sLONG inTag)
{
	void* p = GetFastMem((sLONG)len, ForAnObject, inTag);
	if (p == nil)
	{
		throw std::bad_alloc();
	}
	return p;
}

inline void FreeFastMemForSTL(void* p, size_t = 0)
{
	if (p != nil)
		FreeFastMem(p);
}

template <typename T> class cache_allocator;

template <> class cache_allocator<void>
{
public:
	typedef void* pointer;
	typedef const void* const_pointer;
	// reference to void members are impossible.
	typedef void value_type;
	template <class U> 
	struct rebind { typedef cache_allocator<U> other; };
};    

template <typename T>
class cache_allocator
{
public:
	typedef size_t size_type;
	typedef ptrdiff_t difference_type;
	typedef T* pointer;
	typedef const T* const_pointer;
	typedef T& reference;
	typedef const T& const_reference;
	typedef T value_type;

	template <class U> 
	struct rebind { typedef cache_allocator<U> other; };
	cache_allocator(){}
	pointer address(reference x) const {return &x;}
	const_pointer address(const_reference x) const {return &x;}
	pointer allocate(size_type size, cache_allocator<void>::const_pointer hint = 0)
	{
		return static_cast<pointer>(GetFastMemForSTL(size*sizeof(T), false, 'STL '));
	}
	//for Dinkumware:
	char *_Charalloc(size_type n){return static_cast<char*>(GetFastMemForSTL(n, false, 'STLc'));}
	// end Dinkumware

	template <class U> cache_allocator(const cache_allocator<U>&){}
	void deallocate(pointer p, size_type n)
	{
		FreeFastMemForSTL(p, n);
	}
	void deallocate(void *p, size_type n)
	{
		FreeFastMemForSTL(p, n);
	}
	size_type max_size() const throw() {return size_t(-1) / sizeof(value_type);}
	void construct(pointer p, const T& val)
	{
		new(static_cast<void*>(p)) T(val);
	}
	void construct(pointer p)
	{
		new(static_cast<void*>(p)) T();
	}
	void destroy(pointer p){p->~T();}

};

template <> inline void cache_allocator<char>::destroy(char* p) { ; };
template <> inline void cache_allocator<unsigned char>::destroy(unsigned char* p) { ; };
template <> inline void cache_allocator<short>::destroy(short* p) { ; };
template <> inline void cache_allocator<sLONG>::destroy(sLONG* p) { ; };
template <> inline void cache_allocator<unsigned short>::destroy(unsigned short* p) { ; };
template <> inline void cache_allocator<uLONG>::destroy(uLONG* p) { ; };
template <> inline void cache_allocator<double>::destroy(double* p) { ; };
template <> inline void cache_allocator<sLONG8>::destroy(sLONG8* p) { ; };
template <> inline void cache_allocator<uLONG8>::destroy(uLONG8* p) { ; };


template <typename T, typename U>
inline bool operator==(const cache_allocator<T>&, const cache_allocator<U>){return true;}

template <typename T, typename U>
inline bool operator!=(const cache_allocator<T>&, const cache_allocator<U>){return false;}

#endif
