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
#ifndef __TABADDR__
#define __TABADDR__

#ifndef __BITTABLE__
#include BitTable.h
#endif


const sLONG kNbElemTabAddr = 1024;
const sLONG kSizeTabAddr = kNbElemTabAddr * sizeof(AddrDBObj);
const sLONG kSizeTabAddrMem = kNbElemTabAddr * sizeof(void*);
const sLONG kRatioTabAddr = 10;
const sLONG kNbElemTabAddr2 = kNbElemTabAddr * kNbElemTabAddr;
const sLONG kRatioTabAddr2 = 20;

const sLONG kNbElemTreeMem = 1024;
const sLONG kSizeTreeMem = kNbElemTreeMem * sizeof(void*);
const sLONG kRatioTreeMem = 10;
const sLONG kNbElemTreeMem2 = kNbElemTreeMem * kNbElemTreeMem;
const sLONG kRatioTreeMem2 = 20;


class TreeInMem;

class BaseTaskInfo;

class RechNode; 
class SimpleQueryNode;

typedef TreeInMem* TreeInMemPtr;

class ColumnFormulas;




class TreeMemHeader;
class TreeMem;

#if debugTreeMem_Strong
typedef set<TreeMem*> debug_treememSet;
#endif


class TreeMem : public ObjInCacheMem, public IOccupable
{
	public:
		TreeMem(TreeMemHeader* owner);
#if debugTreeMem_Strong || debugLeakCheck_TreeMem || debuglr
		virtual ~TreeMem();
#endif

		inline void*& operator[](sLONG pIndex) { return (TabMem[pIndex]); };

		VError ReplaceInto(sLONG nbelem, sLONG n, void* data, OccupableStack* curstack);
		VError PutInto(sLONG nbelem, sLONG n, void* data, OccupableStack* curstack);
		void* RetainFrom(sLONG nbelem, sLONG n, OccupableStack* curstack);
		void* GetFrom(sLONG nbelem, sLONG n, OccupableStack* curstack);
		void* GetFromAndOccupy(sLONG nbelem, sLONG n, OccupableStack* curstack);
		void* DelFrom(sLONG nbelem, sLONG n, OccupableStack* curstack);
		void PurgeFrom(sLONG nbelem, sLONG n, OccupableStack* curstack);
		void LiberePourClose(sLONG nbelem, OccupableStack* curstack);
		bool TryToPurge(sLONG nbelem, OccupableStack* curstack);
		bool TryToFreeMem(sLONG allocationBlockNumber, sLONG nbelem, VSize combien, VSize& outFreed);

		VError PerformRech(Bittab* ReelFiltre, sLONG nbelem, RechNode* rn, Bittab *cursel, Bittab *filtre, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, 
			DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, sLONG& nbfound, Bittab &Nulls, Bittab* dejalocked, OccupableStack* curstack, LocalEntityModel* model);

		VError PerformRech(Bittab* ReelFiltre, sLONG nbelem, SimpleQueryNode* rn, Bittab *cursel, Bittab *filtre, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, 
			DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, sLONG& nbfound, Bittab &Nulls, Bittab* dejalocked, OccupableStack* curstack);

		void FillArrayFromCache(sLONG nbelem, void* &into, sLONG sizeelem, Bittab* filtre, Bittab& outDeja, const xMultiFieldDataOffsets& criterias, OccupableStack* curstack, Bittab* nulls);
		void CalculateFormulaFromCache(sLONG nbelem, ColumnFormulas* Formulas, Bittab* filtre, Bittab& outDeja, BaseTaskInfo* context, OccupableStack* curstack);

		VError FillCollection(sLONG recnum, sLONG nbelem, DB4DCollectionManager& collection, Bittab& dejapris, SelPosIterator& PosIter, BaseTaskInfo* context, VArrayRetainedOwnedPtrOf<Field*>& cols, OccupableStack* curstack);

#if debugTreeMem_Strong
		void debug_CheckTree(sLONG nbelem, debug_treememSet& dansTree);
		sLONG debug_CheckTreeComptePages(sLONG nbelem);
#endif

	protected:
		//virtual void DeleteElem( void *inObject);

		void* TabMem[kNbElemTreeMem];
		TreeMemHeader* fOwner;
		mutable VCriticalSection fAlterTreeMutex;

};


class TreeMemHeader : public ObjInCacheMem, public IOccupable
{
	public:
		TreeMemHeader();
		virtual ~TreeMemHeader();
		virtual void PreDestructor(OccupableStack* curstack) = 0; // necessaire car les classe derivees peuvent appeler des fonction virtuelles dans le destructeur de la classe de base

		void LibereEspaceMem(OccupableStack* curstack);
		void Init(sLONG NbElements);
		VError ReplaceIntoTreeMem(sLONG NewMax, sLONG n, void *data, OccupableStack* curstack);
		VError PutIntoTreeMem(sLONG NewMax, sLONG n, void *data, OccupableStack* curstack);
		void* GetFromTreeMemAndOccupy(sLONG n, OccupableStack* curstack);
		void* GetFromTreeMem(sLONG n, OccupableStack* curstack);
		void* RetainFromTreeMem(sLONG n, OccupableStack* curstack);
		void* DelFromTreeMem(sLONG n, OccupableStack* curstack);
		void PurgeFromTreeMem(sLONG n, OccupableStack* curstack);
		bool TryToPurge(OccupableStack* curstack);
		bool TryToFreeMem(sLONG allocationBlockNumber, VSize combien, VSize& outFreed);
		inline Boolean IsEmpty() const { return tmem == nil; };

		VError PerformRech(Bittab* ReelFiltre, RechNode* rn, Bittab *cursel, Bittab *filtre, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, 
			DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, sLONG& nbfound, Bittab &Nulls, Bittab* dejalocked, OccupableStack* curstack, LocalEntityModel* model);
		VError PerformRech(Bittab* ReelFiltre, SimpleQueryNode* rn, Bittab *cursel, Bittab *filtre, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, 
			DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, sLONG& nbfound, Bittab &Nulls, Bittab* dejalocked, OccupableStack* curstack);

		void FillArrayFromCache(void* &into, sLONG sizeelem, Bittab* filtre, Bittab& outDeja, const xMultiFieldDataOffsets& criterias, OccupableStack* curstack, Bittab* nulls);
		void CalculateFormulaFromCache(ColumnFormulas* Formulas, Bittab* filtre, Bittab& outDeja, BaseTaskInfo* context, OccupableStack* curstack);

		VError FillCollection(DB4DCollectionManager& collection, Bittab& dejapris, SelPosIterator& PosIter, BaseTaskInfo* context, VArrayRetainedOwnedPtrOf<Field*>& cols, OccupableStack* curstack);

		VError Aggrandit(sLONG newMax, OccupableStack* curstack);
		VError AggranditSiNecessaire(sLONG newMax, OccupableStack* curstack);

		TreeMem* CreTreeMem();

		virtual void KillElem(void* inObject) = 0;
		virtual bool IsElemPurged(void* InObject) = 0;
		virtual bool TryToFreeElem(sLONG allocationBlockNumber, void* InObject, VSize& outFreed) = 0;
		virtual void RetainElem(void* inObject) = 0;
		virtual void OccupyElem(void* inObject, OccupableStack* curstack) = 0;

#if debugTreeMem_Strong
		void debug_AddPage(TreeMem* obj);
		void debug_DelPage(TreeMem* obj);
		void debug_CheckTree();
#endif

	protected:
		TreeMem *tmem;
		sLONG nbelem;
		mutable VCriticalSection fAlterTreeMutex;

#if debugTreeMem_Strong
		debug_treememSet debug_pages;
#endif
};



				// ====================================================


class AddressTableHeader;

typedef uLONG StampsArray[kNbElemTabAddr];

class AddressTable : public ObjInCacheMem, public IObjToFlush
{
	public:
		AddressTable(Base4D *xdb, sLONG xprefseg, bool inKeepStamps, bool DebugCheckOnDelete);
		inline AddrDBObj& operator[](sLONG ind) {return( tab[ind] ); };
		inline const AddrDBObj& operator[](sLONG ind) const {return( tab[ind] ); };
		VError init(void);
		VError loadobj(DataAddr4D xaddr=0, sLONG inposdansparent = -1);

		virtual bool SaveObj(VSize& outSizeSaved);

		DataAddr4D GetAddrFromTable(sLONG nbelem, sLONG n, VError& err, OccupableStack* curstack, sLONG* outLen = nil, uLONG* outStamp = nil);
		VError PutAddrIntoTable(sLONG nbelem, sLONG n, DataAddr4D addr, sLONG len, BaseTaskInfo* context, OccupableStack* curstack, uLONG inStamp = 0);
		VError loadmem(sLONG n, sLONG n1, sLONG *xn2, sLONG *n3, AddressTable* *psous, OccupableStack* curstack);
		AddressTable* AddToTable(sLONG nbelem, BaseTaskInfo* context, VError& err, OccupableStack* curstack);
		virtual VError LibereEspaceDisk(sLONG nbelem, VDB4DProgressIndicator* InProgress, OccupableStack* curstack);
		//virtual AddressTable* NewTable(Base4D *xdb);
		void LiberePourClose(sLONG nbelem, OccupableStack* curstack);
		bool TryToFreeMem(sLONG allocationBlockNumber, sLONG nbelem, VSize combien, VSize& outFreed);

		VError InitAndSetSize(sLONG maxelem, BaseTaskInfo* context, sLONG inPosDansParent, OccupableStack* curstack);
		void SubLiberePlace(DataAddr4D ou, BaseTaskInfo* context);

		inline void SetPosDansParent(sLONG n)
		{
			posdansparent = n;
		}

		DataAddr4D ConvertAddrTable(sLONG nbelem, bool inKeepStamps, VDB4DProgressIndicator* InProgress, OccupableStack* curstack, BaseTaskInfo* context, VError& err);


	#if debugOverlaps_strong
		Boolean Debug_CheckAddrOverlap(sLONG nbelem, sLONG curelem, DataAddr4D addrToCheck, sLONG lenToCheck, sLONG nomobjToCheck = -1);
	#endif

#if debug_Addr_Overlap
		void FillDebug_DBObjRef(sLONG nbelem, OccupableStack* curstack, sLONG tablenum, bool isrectable);
#endif

#if debugTabAddr
		static void DebugKeepPage(AddressTable* addrtable);
		static void DebugForgetPage(AddressTable* addrtable);

		inline void SetDebugCheckOnDelete(bool xdebugcheck)
		{
			fDebugCheckOnDelete = xdebugcheck;
		}

		inline bool GetDebugCheckOnDelete() const
		{
			return fDebugCheckOnDelete;
		}

		virtual void setaddr(DataAddr4D addr, bool checknew = true) 
		{ 
			IObjToFlush::setaddr(addr, checknew);
			if (fDebugCheckOnDelete)
			{
				DebugKeepPage(this);
			}
		};
#endif

	protected:
		virtual ~AddressTable();

		AddrDBObj tab[kNbElemTabAddr];
		AddressTable* fTabmem[kNbElemTabAddr];
		StampsArray* fStamps;
		Base4D *db;
		sLONG prefseg;
		sLONG posdansparent;
		VCriticalSection fLoadMutex;
		bool fIsLoaded, fKeepStamps;
		bool fDebugCheckOnDelete;

#if debugTabAddr
		static VCriticalSection debugKeepTablesMutex;
		static map<DataAddr4D, AddressTable*> debugKeepTables;
#endif

};


class AddressTableHeader : public ObjInCacheMem, public IObjToFlush
{
	public:

	#if debuglr
		typedef set<sLONG> debug_MapOfTempAddr;
	#endif

		AddressTableHeader();
		virtual ~AddressTableHeader();
		void LibereEspaceMem(OccupableStack* curstack);
		void Init(Base4D *xdb, IObjToFlush *TheOwner, DataAddr4D *AddrOfFirst, DataAddr4D* AddrDebutTrou, sLONG *AddrNBelem, sLONG xprefseg, bool inKeepStamps);
		//inline AddressTable* GetFirstPage(BaseTaskInfo* context) { ChargeTabAddr(context);return(FirstPage); };
		DataAddr4D GetxAddr(sLONG n, BaseTaskInfo* context, VError& err, OccupableStack* curstack, sLONG* outLen = nil, uLONG* outStamp = nil);
		VError PutxAddr(sLONG n, DataAddr4D ValAddr, sLONG len, BaseTaskInfo* context, OccupableStack* curstack, uLONG inStamp = 0, const char* debugmess = nil);

		AddressTable* NewTable(Base4D *xdb);

		VError InitAndSetSize(sLONG maxelem, BaseTaskInfo* context, OccupableStack* curstack); // attention n'utiliser que quand la table d'adresse est nouvellement creee
		VError Normalize(BaseTaskInfo* context, OccupableStack* curstack);
		VError SetDebutDesTrous(sLONG val, BaseTaskInfo* context);

		VError ChargeTabAddr(BaseTaskInfo* context, OccupableStack* curstack);
		sLONG findtrou(BaseTaskInfo* context, VError& err, OccupableStack* curstack);
		VError liberetrou(sLONG n, BaseTaskInfo* context, OccupableStack* curstack, uLONG inStamp = 0);
		VError TakeOffBits(Bittab *tb, BaseTaskInfo* context, OccupableStack* curstack, bool UseAlternateDebut = false, DataAddr4D AlternateDebut = 0);
		virtual VError LibereEspaceDisk(VDB4DProgressIndicator* InProgress, OccupableStack* curstack);

		sLONG GetNbElem(void) const { return(*nbelem); };

		DataAddr4D* GetAddrFirstPage(void) const { return(AddrFirstPage); };
		DataAddr4D* GetDebutTrou(void) const { return(DebutTrou); };

		virtual void setmodif(uBOOL xismodif, Base4D* bd, BaseTaskInfo* context);

		virtual bool SaveObj(VSize& outSizeSaved);

		void SetValues(DataAddr4D AddrFirstPageVal, DataAddr4D DebutTrouVal, sLONG nbelemVal) 
		{ *AddrFirstPage = AddrFirstPageVal; *DebutTrou = DebutTrouVal; *nbelem = nbelemVal; };

		Base4D *GetBase() const { return db; };

		//virtual Obj4D* GetThis() const { return (Obj4D*)this); };

		sLONG GetPrefSeg() { return prefseg; };

		VError TakeAddrTemporary(sLONG n, BaseTaskInfo* context, sLONG* debuttroutrans, OccupableStack* curstack);
		VError GiveBackAddrTemporary(sLONG n, BaseTaskInfo* context, sLONG* debuttroutrans, OccupableStack* curstack);

		VError TakeAddr(sLONG n, BaseTaskInfo* context, OccupableStack* curstack);

		virtual Boolean OnlyHasOneChild() const
		{
			return true;
		}

		bool TryToFreeMem(sLONG allocationBlockNumber, VSize combien, VSize& outFreed);

		VError ConvertAddrTable(bool inKeepStamps, VDB4DProgressIndicator* InProgress, OccupableStack* curstack);

		VError GetFragmentation(sLONG8& outTotalRec, sLONG8& outFrags);


	#if debugOverlaps_strong
		Boolean Debug_CheckAddrOverlap(DataAddr4D addrToCheck, sLONG lenToCheck, sLONG nomobjToCheck = -1);
	#endif

	#if debugTabAddr
		inline void SetDebugMess(bool xdebugmess)
		{
			fDebugMess = xdebugmess;
		}

		inline bool GetDebugMess() const
		{
			return fDebugMess;
		}

		inline void SetDebugCheckOnDelete(bool xdebugcheck)
		{
			fDebugCheckOnDelete = xdebugcheck;
		}

		inline bool GetDebugCheckOnDelete() const
		{
			return fDebugCheckOnDelete;
		}
#endif

#if debug_Addr_Overlap
		void FillDebug_DBObjRef(sLONG tablenum, bool isrectable);
#endif

	protected:
		AddressTable *FirstPage;
		Base4D *db;
		DataAddr4D* AddrFirstPage;
		DataAddr4D* DebutTrou;
		sLONG* nbelem;
		IObjToFlush *owner;
		sLONG fIDUniq;
		sLONG prefseg;
		VCriticalSection fLoadMutex;
		bool fKeepStamps;

		bool fDebugCheckOnDelete;
	#if debugTabAddr
		bool fDebugMess;
	#endif

	#if debuglr
		debug_MapOfTempAddr xTempAddr;
	#endif

};




// old code
                                // -------------------------------------------------------------------------




class TreeInMem : public ObjCacheInTree
{
	public:
		TreeInMem(sWORD DefaultAccess, typobjcache typ, uBOOL needlock);
		virtual ~TreeInMem();
	#if autocheckobj
		virtual uBOOL CheckObjInMem(void);
	#endif
		uBOOL okcree(void) { return(tabmem!=nil); };
		VError PutInto(sLONG nbelem, sLONG n, ObjCacheInTree* data, uBOOL ContientObjAutre);
		ObjCacheInTree* GetFrom(sLONG nbelem, sLONG n);
		ObjCacheInTree* GetFromAndOccupe(sLONG nbelem, sLONG n, Boolean& dejaoccupe);
		void* DelFrom(sLONG nbelem, sLONG n);
		virtual uBOOL okdel(void);
		virtual sLONG saveobj(void);
		virtual sLONG liberemem(sLONG allocationBlockNumber, sLONG combien=-1, uBOOL tout=false);
		virtual void RecalcNbElemTab();
		virtual void DelFromParent(sLONG n, ObjCacheInTree* enfant);
		inline TreeInMemPtr& operator[](sLONG pIndex) { return (((TreeInMemPtr*)tabmem)[pIndex]); };
		virtual TreeInMem* CreTreeInMem(void);
		void LiberePourClose(sLONG nbelem, uBOOL DoisDisposeElem);
		Boolean TryToPurge(sLONG nbelem);

		inline void SetContientQuoi(typobjcache typ) { ContientQuoi = typ; FeuilleFinaleContientQuoi = typ; };
		
		virtual void CheckDansParent(sLONG n, ObjCacheInTree* enfant) const
		{
			if (testAssert(tabmem != nil))
			{
				xbox_assert(tabmem[n] == enfant);
			}
		}

#if debuglr
		void CheckObjRef(sLONG nbelem, ObjCacheInTree* objtocheck);
#endif

	protected:
		virtual void DeleteElem( ObjCacheInTree *inObject);
		ObjCacheInTree* xTabMem[kNbElemTreeMem];
		ObjCacheInTree* *tabmem;
		uBOOL ContientObjAutre, fNeedLock;
		typobjcache ContientQuoi; 
		typobjcache FeuilleFinaleContientQuoi; 
	
};


class TreeInMemHeader : public ObjAlmostInCacheInTree
{
	public:
		TreeInMemHeader();
		virtual ~TreeInMemHeader();
		void LibereEspaceMem();
		void Init(sLONG NbElements, uBOOL ContientObjAutre = false);
		virtual void RecalcNbElemTab();
		virtual void DelFromParent(sLONG n, ObjCacheInTree* enfant);
		VError PutIntoTreeMem(sLONG NewMax, sLONG n, ObjCacheInTree *data);
		ObjCacheInTree* GetFromTreeMem(sLONG n);
		ObjCacheInTree* GetFromTreeMemAndOccupe(sLONG n, Boolean wait = true);
		void* DelFromTreeMem(sLONG n);
		virtual void Kill(void) { ; };
		Boolean TryToPurge();
		inline void SetContientQuoi(typobjcache typ) { FeuilleFinaleContientQuoi = typ; };
		inline void SetNeedLock(uBOOL needlock) { fNeedLock = needlock; };
		inline Boolean IsEmpty() const { return tmem == nil; };

		VError Aggrandit(sLONG newMax);
		VError AggranditSiNecessaire(sLONG newMax);

		virtual void CheckDansParent(sLONG n, ObjCacheInTree* enfant) const
		{
			xbox_assert(n == 0);
			xbox_assert(tmem == enfant);
		}
		virtual Boolean OnlyHasOneChild() const
		{
			return true;
		}

#if debuglr
		void CheckObjRef(ObjCacheInTree* objtocheck)
		{
			VObjLock lock(this);
			if (tmem != nil)
				tmem->CheckObjRef(nbelem, objtocheck);
		}
#endif

	protected:
		virtual TreeInMem* CreTreeInMem();

		TreeInMem *tmem;
		sLONG nbelem;
		uBOOL ContientObjetAutre, fNeedLock;
		typobjcache FeuilleFinaleContientQuoi; 
};





											/* ======================================== */
											
											
	
class CheckAndRepairAgent;		
																			
class AddrTable : public ObjCacheInTree
{
	public:
		AddrTable(Base4D *xdb, sLONG xprefseg, sWORD DefaultAccess = TableAddressAccess	);
		virtual ~AddrTable();
	#if autocheckobj
		virtual uBOOL CheckObjInMem(void);
	#endif
		inline uBOOL isokdel(void) { return( (tab==nil) && (tabmem==nil) ); };
		inline AddrDBObj& operator[](sLONG ind) {return( tab[ind] ); };
		inline const AddrDBObj& operator[](sLONG ind) const {return( tab[ind] ); };
		VError init(void);
		VError loadobj(DataAddr4D xaddr=0, sLONG inposdansparent = -1);
		virtual uBOOL okdel(void);
		virtual sLONG saveobj(void);
		virtual void RecalcNbElemTab();
		virtual sLONG liberemem(sLONG allocationBlockNumber, sLONG combien=-1, uBOOL tout=false);
		virtual void DelFromParent(sLONG n, ObjCacheInTree* enfant);
		DataAddr4D GetAddrFromTable(sLONG nbelem, sLONG n, VError& err, sLONG* outLen = nil);
		VError PutAddrIntoTable(sLONG nbelem, sLONG n, DataAddr4D addr, sLONG len, BaseTaskInfo* context);
		VError loadmem(sLONG n, sLONG n1, sLONG *xn2, sLONG *n3, AddrTable* *psous);
		AddrTable* AddToTable(sLONG nbelem, BaseTaskInfo* context, VError& err, AddrTableHeader *ParentF = nil);
		virtual VError LibereEspaceDisk(sLONG nbelem, VDB4DProgressIndicator* InProgress = nil);
		virtual AddrTable* NewTable(Base4D *xdb);
		void LiberePourClose(sLONG nbelem);
		inline AddrTable* GetxOldOne(BaseTaskInfo* context) { return /*((AddrTable*)GetOldOne(context))*/ this; };

		static VError CheckTable(CheckAndRepairAgent* inCheck, DataAddr4D adr, sLONG nbelem, sLONG maxelems, DataBaseObjectType WhatType, sLONG PosInParent, sLONG Parent);
		
		VError InitAndSetSize(sLONG maxelem, BaseTaskInfo* context, sLONG inPosDansParent);
		void SubLiberePlace(DataAddr4D ou, BaseTaskInfo* context);

		virtual void CheckDansParent(sLONG n, ObjCacheInTree* enfant) const
		{
			if (testAssert(tabmem != nil))
			{
				xbox_assert(tabmem[n] == enfant);
			}
		}

#if debugOverlaps_strong
		Boolean Debug_CheckAddrOverlap(sLONG nbelem, sLONG curelem, DataAddr4D addrToCheck, sLONG lenToCheck, sLONG nomobjToCheck = -1);
#endif

	protected:
		AddrDBObj *tab;
		//sLONG *tablen;
		AddrTable* *tabmem;
		Base4D *db;
		//DataAddr4D origaddr;
		//AddrTable *original;
		sLONG prefseg;
};

class AddrTableHeader;

typedef VError (AddrTableHeaderReLoaderProc)(AddrTableHeader* obj, ObjAlmostInCache* Owner);
// en passant nil dans obj on force l'Owner a remettre a zero la copie des entetes (typiquement -1 dans la copie de nbelem)

typedef VError (AddrTableHeaderReLoaderForKillProc)(ObjAlmostInCache* Owner);


//class AddrTableHeader2;

class AddrTableHeader : public ObjAlmostInCacheInTree
{
	public:

#if debuglr
		typedef set<sLONG> debug_MapOfTempAddr;
#endif


		AddrTableHeader();
		virtual ~AddrTableHeader();
		void LibereEspaceMem();
	#if autocheckobj
		virtual uBOOL CheckObjInMem(void);
	#endif
		void Init(Base4D *xdb, ObjAlmostInCache *TheOwner, DataAddr4D *AddrOfFirst, DataAddr4D* AddrDebutTrou, sLONG *AddrNBelem,
					 AddrTableHeaderReLoaderProc* Loader, AddrTableHeaderReLoaderForKillProc* LoaderForKill, sLONG xprefseg);
		virtual AddrTable* NewTable(Base4D *xdb);
		virtual void RecalcNbElemTab();
		virtual void DelFromParent(sLONG n, ObjCacheInTree* enfant);
		//inline AddrTable* GetFirstPage(BaseTaskInfo* context) { ChargeTabAddr(context);return(FirstPage); };
		inline void LibereFirst(void) { if (FirstPage!=nil) FirstPage->libere(); };
		DataAddr4D GetxAddr(sLONG n, BaseTaskInfo* context, VError& err, sLONG* outLen = nil);
		VError PutxAddr(sLONG n, DataAddr4D ValAddr, sLONG len, BaseTaskInfo* context);

		VError InitAndSetSize(sLONG maxelem, BaseTaskInfo* context); // attention n'utiliser que quand la table d'adresse est nouvellement creee
		VError Normalize(BaseTaskInfo* context);
		VError SetDebutDesTrous(sLONG val, BaseTaskInfo* context);

		VError ChargeTabAddr(BaseTaskInfo* context);
		sLONG findtrou(BaseTaskInfo* context, VError& err);
		VError liberetrou(sLONG n, BaseTaskInfo* context);
		VError TakeOffBits(Bittab *tb, BaseTaskInfo* context);
		virtual VError LibereEspaceDisk(VDB4DProgressIndicator* InProgress = nil);
		
		sLONG GetNbElem(void) const { return(*nbelem); };
		
		DataAddr4D* GetAddrFirstPage(void) const { return(AddrFirstPage); };
		DataAddr4D* GetDebutTrou(void) const { return(DebutTrou); };

		inline AddrTableHeader* GetxOldOne(BaseTaskInfo* context) { return /*((AddrTableHeader*)GetOldOne(context))*/ this; };

		virtual void setmodif(uBOOL xismodif, Base4D* bd, BaseTaskInfo* context);
		virtual sLONG saveobj(void);

		void SetValues(DataAddr4D AddrFirstPageVal, DataAddr4D DebutTrouVal, sLONG nbelemVal) 
			{ *AddrFirstPage = AddrFirstPageVal; *DebutTrou = DebutTrouVal; *nbelem = nbelemVal; };
		
		VError CheckAddresses(CheckAndRepairAgent* inCheck);
	
		Base4D *GetBase() const { return db; };

		//virtual Obj4D* GetThis() const { return (Obj4D*)this); };

		sLONG GetPrefSeg() { return prefseg; };

		VError TakeAddrTemporary(sLONG n, BaseTaskInfo* context, sLONG* debuttroutrans);
		VError GiveBackAddrTemporary(sLONG n, BaseTaskInfo* context, sLONG* debuttroutrans);

		VError TakeAddr(sLONG n, BaseTaskInfo* context);

		virtual Boolean OnlyHasOneChild() const
		{
			return true;
		}

		virtual void CheckDansParent(sLONG n, ObjCacheInTree* enfant) const
		{
			xbox_assert(n == 0);
			xbox_assert(FirstPage == enfant);
		}

#if debugOverlaps_strong
		Boolean Debug_CheckAddrOverlap(DataAddr4D addrToCheck, sLONG lenToCheck, sLONG nomobjToCheck = -1);
#endif




	protected:

		AddrTable *FirstPage;
		Base4D *db;
		DataAddr4D* AddrFirstPage;
		DataAddr4D* DebutTrou;
		sLONG* nbelem;
		ObjAlmostInCache *owner;
		sLONG fIDUniq;
		/*
		AddrTableHeader2* original;
		AddrTableHeaderReLoaderProc *reloader;
		AddrTableHeaderReLoaderForKillProc *reloaderForKill;
		*/
		sLONG prefseg;

		
#if debuglr
		debug_MapOfTempAddr xTempAddr;
#endif
	
};


class Base4D_NotOpened;

class AddrTableHeader_NotOpened : public ObjAlmostInCache
{
	public:
		AddrTableHeader_NotOpened(Base4D_NotOpened* xdb);
		inline void Init(sLONG xnbelem, DataAddr4D xAddrFirstPage, DataAddr4D xDebutTrou)
		{
			nbelem = xnbelem;
			AddrFirstPage = xAddrFirstPage;
			DebutTrou = xDebutTrou;
		};

	protected:
		sLONG nbelem;
		DataAddr4D AddrFirstPage;
		DataAddr4D DebutTrou;
		Base4D_NotOpened* db;

};


#if 0

class AddrTableHeader2:public AddrTableHeader
{
	public:
		AddrTableHeader2(AddrTableHeader* from);
		
	protected:
		DataAddr4D dup_AddrFirstPage, dup_DebutTrou;
		sLONG dup_nbelem;
		
};



class AddrTableRoot
{
	public:
		inline AddrTableRoot(void) { parentroot=nil; };
		inline void SetParentRoot(ObjCache *par) { parentroot=par; };
		inline ObjCache* GetParentRoot(void) { return(parentroot); };
		
	private:
		ObjCache *parentroot;
		
};

#endif


								/* ======================================== */

class ExtraElementTable;

class ExtraElement : public IObjToFlush
{
	public:
		ExtraElement(ExtraElementTable* inOwner, DataAddr4D addr, sLONG len, sLONG pos);

		virtual bool SaveObj(VSize& outSizeSaved);

		VError load(bool* needSwap);
		VError save(OccupableStack* curstack);

		Base4D* GetDB() const;

		VMemoryBuffer<>& GetData()
		{
			return fData;
		}

		const VMemoryBuffer<>& GetData() const
		{
			return fData;
		}

	protected:
		sLONG fLenOnDisk;
		sLONG fPos;
		VMemoryBuffer<> fDataOnDisk;
		VMemoryBuffer<> fData;
		ExtraElementTable* fOwner;
		VCriticalSection fMutex;
		

};

typedef ExtraElement* ExtraElementPtr;


class ExtraElementTable : public IObjToFlush
{
	public:
		ExtraElementTable(Base4D* inOwner, DataAddr4D* inAddrPtr, sLONG* inNbElemPtr);
		virtual ~ExtraElementTable();

		virtual bool SaveObj(VSize& outSizeSaved);

		VError load();
		VError save();

		ExtraElement* RetainElement(sLONG n, OccupableStack* curstack, VError& err, bool* outNeedSwap);
		VError SetElementAddress(sLONG n, DataAddr4D addr, sLONG len, OccupableStack* curstack);

		Base4D* GetOwner() const
		{
			return fOwner;
		}

	protected:
		AddrDBObj fAddrs[kMaxNbExtraElements];
		ExtraElementPtr fElems[kMaxNbExtraElements];
		sLONG fNbElemsOnDisk;

		Base4D* fOwner;
		DataAddr4D* fAddrPtr;
		sLONG* fNbElemPtr;



};


inline Base4D* ExtraElement::GetDB() const
{
	return fOwner->GetOwner();
}





#endif



