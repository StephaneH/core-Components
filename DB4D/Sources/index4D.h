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
#ifndef __INDEX4D__
#define __INDEX4D__




									/* -----------------------------------------------  */
const sLONG kSpecialClustNulls = 2000000001;

class FieldNuplet;
class FieldRefArray;

									/* -----------------------------------------------  */
									
class ColumnFormulas;

class IndexHeader;

class IndexInfo;

class BaseTaskInfo;

const sLONG kFakeSousPageForNull = -5;

const sLONG KeyPreAllocSize = 2048;

extern bool gTempNewIndexFourche;



									/* -----------------------------------------------  */

class ClusterSel;
class BTitemIndex;

#if debug_BTItemIndex
typedef set<BTitemIndex*> debug_BTitemIndexSet;
#endif

class BTitemIndex
{
public:
	inline BTitemIndex() 
	{ 
		sousBT = nil; 
		souspage = -1; 
		qui = -1;
#if debug_BTItemIndex
		sdebug_BTitemIndexSet.insert(this);		
#endif
	};

#if debug_BTItemIndex
	inline ~BTitemIndex()
	{
		sdebug_BTitemIndexSet.erase(this);	
	}
#endif

#if debug_BTItemIndex
	inline void debug_dispose()
	{
		sdebug_BTitemIndexSet.erase(this);
	}
#endif

	BTreePageIndex *sousBT;
	mutable sLONG souspage;
	sLONG qui;
	void* data;

#if debug_BTItemIndex
	static debug_BTitemIndexSet sdebug_BTitemIndexSet;
#endif

	inline void SetInd(IndexInfo* ind) { sousBT = (BTreePageIndex*)ind; };
	inline IndexInfo* GetInd() const { return (IndexInfo*)sousBT; }; 
	// attention a n'utiliser que pour les BTitemIndex stockes dans les transactions 
	// car sousBT n'est pas utilisee comme souspage et sert a stocker la reference a l'index

	inline void StartUse() { souspage = 1; };
	inline sLONG GetNbUse() const { return souspage; };
	inline void Use() const { souspage++; };
	void Unuse() const;
	// attention a n'utiliser que pour les BTitemIndex stockes dans les transactions 
	// car souspage n'est pas utilisee et sert a stocker un compteur d'utilisation

	
	inline void SetQueryAttribute(Boolean isBeginWith, Boolean isLike)
	{
		if (isBeginWith)
			qui = kTransFakeKeyNumForBeginWith;
		else if (isLike)
			qui = kTransFakeKeyNumForIsLike;
		else
			qui = -1;
	};

	inline sLONG GetQui() const
	{
		return qui;
	};

	inline void SetQui(sLONG xqui)
	{
		qui = xqui;
	};

	inline Boolean IsNull() const 
	{ 
		return souspage == kFakeSousPageForNull;
	};

	inline void SetNull() 
	{ 
		souspage = kFakeSousPageForNull;
	};

	inline static size_t CalculateEmptyLen() { return sizeof(BTitemIndex) - sizeof(void*) - sizeof(BTreePageIndex*); };
	void GetDebugString(IndexInfo* ind, VString& outString);
};


class IndexKeyArray
{
	public:
		inline IndexKeyArray(IndexInfo* owner) { nbval = 0; entete = owner;};
		virtual ~IndexKeyArray();
		void AddKey(BTitemIndex* val) { xbox_assert(nbval < 30); vals[nbval] = val; nbval++; };

	protected:
		IndexInfo* entete;
		sLONG nbval;
		BTitemIndex* vals[30];
};


class VDB4DIndexKey;
class WafSelection;

class IndexHeader : public /*ObjAlmostInCacheInTree*/ ObjInCacheMem, public IObjToFlush
{
	friend class DataTools;

	public:
		IndexHeader();
		virtual ~IndexHeader();
		virtual void setmodif(uBOOL xismodif, Base4D* bd, BaseTaskInfo* context);

		virtual uBOOL MayBeSorted(void);
		virtual sLONG GetLen(void)=0;
		virtual VError PutInto(VStream& buf);
		virtual VError GetFrom(VStream& buf);
		virtual void SetAssoc(IndexInfo* x); // va aussi initialiser les TabAddr Header et TreeInMem Header Associes
		inline IndexInfo* GetAssoc(void) { return(assoc); };
		DataAddr4D GetInd(OccupableStack* curstack, sLONG n, BaseTaskInfo* context, VError& err, sLONG* outLen = nil);
		sLONG PutInd(OccupableStack* curstack, sLONG n, DataAddr4D r, sLONG len, BaseTaskInfo* context, VError& err);
		void LiberePage(OccupableStack* curstack, sLONG numpage, BaseTaskInfo* context);
		sLONG GetTyp(void) const;

		inline	sLONG GetRealType() const { return typindex; };
		
		/*
		static VError Reloader(AddrTableHeader* obj, ObjAlmostInCache* Owner);
		static VError ReloaderForKill(ObjAlmostInCache* Owner);
		void SaveIHDForTrans(BaseTaskInfo* context);
		*/

		virtual Bittab* FindKeyInArray(OccupableStack* curstack, DB4DArrayOfValues* values, BaseTaskInfo* context, VError& err, VDB4DProgressIndicator* InProgress, const VCompareOptions& inOptions, Bittab* dejasel=nil)
		{
			xbox_assert(false);
			return nil;
		}

		virtual Bittab* Fourche(OccupableStack* curstack, BTitemIndex* val1, uBOOL xstrict1, BTitemIndex* val2, uBOOL xstrict2, BaseTaskInfo* context, VError& err, 
														VDB4DProgressIndicator* InProgress, const VCompareOptions& inOptions, Bittab* dejasel=nil, BTitemIndex** outVal = nil)
		{
			xbox_assert(false);
			return nil;
		}

		virtual sLONG FindKey(OccupableStack* curstack, BTitemIndex* val, BaseTaskInfo* context, const VCompareOptions& inOptions, Bittab* dejasel, BTitemIndex** outVal)
		{
			xbox_assert(false);
			return -1;
		}
		

		virtual VError PlaceCle(OccupableStack* curstack, BTitemIndex *cle, sLONG xqui, IndexKeyArray& tempkeys, BaseTaskInfo* context)
		{
			xbox_assert(false);
			return VE_DB4D_NOTIMPLEMENTED;
		}
		
		virtual VError DetruireCle(OccupableStack* curstack, BTitemIndex *cle, sLONG xqui, IndexKeyArray& tempkeys, BaseTaskInfo* context)
		{
			xbox_assert(false);
			return VE_DB4D_NOTIMPLEMENTED;
		}
		
		virtual void TryToFreeMem(sLONG allocationBlockNumber, VSize combien, VSize& tot) = 0;

		virtual VError LibereEspaceDisk(OccupableStack* curstack, VDB4DProgressIndicator* InProgress = nil);
		virtual void LibereEspaceMem(OccupableStack* curstack);

		virtual Selection* SortSel(OccupableStack* curstack, Selection* sel, uBOOL ascent, BaseTaskInfo* context, VError& err, 
										VDB4DProgressIndicator* InProgress = nil, Boolean TestUnicite = false, Bittab* inWafSelbits = nil, WafSelection* outWafSel = nil) = 0;
		
		VError ThrowError( VError inErrCode, ActionDB4D inAction);

		virtual ClusterSel* GetClusterSel(OccupableStack* curstack) { return nil ;};
		virtual const ClusterSel* GetClusterSel(OccupableStack* curstack) const { return nil ;};

		virtual BTreePageIndex* CrePage(void) = 0;

		inline IndexHeaderDISK* GetIndexHeader(void) { return (&IHD) ;}; // J.A. Tools

		virtual VError GetDistinctValues(OccupableStack* curstack, DB4DCollectionManager& outCollection, BaseTaskInfo* context, Bittab* filtre, 
										VDB4DProgressIndicator* InProgress, VCompareOptions& inOptions) = 0;

		virtual VError QuickGetDistinctValues(OccupableStack* curstack, QuickDB4DArrayOfValues *outCollection, BaseTaskInfo* context, Bittab* filtre, 
										VDB4DProgressIndicator* InProgress, VCompareOptions& inOptions)
		{
			return VE_DB4D_NOTIMPLEMENTED; 
		}

		virtual VError CalculateColumnFormulas(OccupableStack* curstack, ColumnFormulas* formules, BaseTaskInfo* context, Bittab* filtre, VDB4DProgressIndicator* InProgress)
		 { 
			return VE_DB4D_NOTIMPLEMENTED; 
		};

		virtual VError CalculateMin(OccupableStack* curstack, BaseTaskInfo* context, Bittab* filtre, VDB4DProgressIndicator* InProgress, VValueSingle* &result) 
		{ 
			result = nil; return VE_DB4D_NOTIMPLEMENTED; 
		};

		virtual VError CalculateMax(OccupableStack* curstack, BaseTaskInfo* context, Bittab* filtre, VDB4DProgressIndicator* InProgress, VValueSingle* &result) 
		{ 
			result = nil; return VE_DB4D_NOTIMPLEMENTED; 
		};

		virtual VError FindKeyAndBuildPath(OccupableStack* curstack, BTitemIndex* val, const VCompareOptions& inOptions, BaseTaskInfo* context, VDB4DIndexKey* outKey) = 0;
		virtual VError NextKey(OccupableStack* curstack, const VDB4DIndexKey* inKey, BaseTaskInfo* context, VDB4DIndexKey* outKey) = 0;
		
		virtual VError ScanIndex(OccupableStack* curstack, Selection* into, sLONG inMaxRecords, Boolean KeepSorted, Boolean Ascent, BaseTaskInfo* context, Bittab* filtre,sLONG &nbtrouves, VDB4DProgressIndicator* InProgress) = 0;

		virtual sLONG8 GetNBDiskAccessForAScan(Boolean WithSelFilter) = 0;

		virtual VError InitTablesAddr(OccupableStack* curstack);
		virtual VError NormalizeTablesAddr(OccupableStack* curstack);

		virtual VError SetClusterAddr(OccupableStack* curstack, sLONG numclust, DataAddr4D SelAddr, sLONG SelLen)
		{
			if (numclust == kSpecialClustNulls)
			{
				IHD.AddrNULLs = SelAddr;
				IHD.lenNULLs = SelLen;
				return VE_OK;
			}
			else
				return VE_DB4D_NOTIMPLEMENTED;
		}

		VError SetPageAddr(OccupableStack* curstack, sLONG numpage, DataAddr4D PageAddr, sLONG PageLen)
		{
			return taddr.PutxAddr(numpage, PageAddr, PageLen, nil, curstack);
		}

		VError AddToNulls(OccupableStack* curstack, Bittab* b, BaseTaskInfo* context);
		VError DelFromNulls(OccupableStack* curstack, Bittab* b, BaseTaskInfo* context);
		VError AddToNulls(OccupableStack* curstack, sLONG numrec, BaseTaskInfo* context);
		VError DelFromNulls(OccupableStack* curstack, sLONG numrec, BaseTaskInfo* context);
		BitSel* RetainNulls(OccupableStack* curstack, VError& err, BaseTaskInfo* context);
		VError ModifyNulls(OccupableStack* curstack, BaseTaskInfo* context);

		Boolean HasNulls(OccupableStack* curstack);

		virtual Bittab* ParseAll(OccupableStack* curstack, BTitemIndex* val1, BaseTaskInfo* context, VError& err, const VCompareOptions& inOptions, VDB4DProgressIndicator* InProgress)
		{
			err = ThrowError(VE_DB4D_NOTIMPLEMENTED, noaction);
			return nil;

		}

		virtual void Update(Table* inTable) = 0;

		virtual bool IsInvalidOnDisk();
		virtual void SetInvalidOnDisk();
		virtual void SetValidOnDisk();


		void MeasureRead(sLONG nbread, VSize howManyBytes);
		void MeasureWrite(sLONG nbwrite, VSize howManyBytes);
		void HitCache(sLONG nbhit, VSize howManyBytes);
		void MissCache(sLONG nbmiss, VSize howManyBytes);

		VJSONObject* RetainMeasures(VJSONObject* options = nil, JSONPath* path = nil);
		void SetMeasureInterval(uLONG interval);


#if debuglr
		virtual void CheckAllPageKeys() { ; };
		virtual void CheckAllPageOwner() { ; };
#endif

	protected:
		sLONG typindex;
		IndexHeaderDISK IHD;
		IndexInfo* assoc;
		AddressTableHeader taddr;
		BitSel* fNulls;

		MeasureCollection fDiskReadBytes;
		MeasureCollection fDiskReadCount;
		MeasureCollection fDiskWriteBytes;
		MeasureCollection fDiskWriteCount;

		MeasureCollection fCacheHitBytes;
		MeasureCollection fCacheHitCount;
		MeasureCollection fCacheMissBytes;
		MeasureCollection fCacheMissCount;

		mutable VCriticalSection fMeasureMutex;

		//IndexHeaderDISK IHD_SavedByTrans;

	private:
		/*
		uBOOL occupe(Boolean ForceWaitEvenDringFlush = false, sLONG signature = 0) const { return false; };
		uBOOL IsOkToOccupe(sLONG signature = 0) const { return false; };
		void libere(sLONG signature = 0) const { ; };

		void occupe() { ; };
		void libere() { ; };
		*/


};


									/* -----------------------------------------------  */
	


#if debugLogEventPageIndex

class dbgIndexEvent
{
public:
	inline dbgIndexEvent()
	{
		fPageLen = 0;
		fPageNum = 0;
		fPageAddr = 0;
		fOwner = nil;
	}

	inline dbgIndexEvent(sLONG inPageLen, sLONG inPageNum, DataAddr4D inPageAddr, IndexInfo* inOwner)
	{
		fPageLen = inPageLen;
		fPageNum = inPageNum;
		fPageAddr = inPageAddr;
		fOwner = inOwner;
	}

	sLONG fPageLen;
	sLONG fPageNum;
	DataAddr4D fPageAddr;
	IndexInfo* fOwner;

};


extern EventLogger<dbgIndexEvent, 1000> gDebugIndexEvent;

#endif


class distinctvalue_iterator;									

const sLONG HalfPage=128/2;

class BTreePageIndex : public ObjInCacheMem, public IObjToFlush
{
	public:
		BTreePageIndex(IndexInfo* xentete, sLONG datasize, sLONG xnum, sWORD DefaultAccess = ObjCache::PageIndexAccess);
		VError loadobj(DataAddr4D xaddr, sLONG len);
		VError savepage(OccupableStack* curstack, BaseTaskInfo* context);

		Boolean FreePageMem(sLONG allocationBlockNumber, VSize combien, VSize& tot);
		virtual bool SaveObj(VSize& outSizeSaved);

		virtual bool MustCheckOccupiedStamp() const
		{
			return false; // use the new algorithm to check if the object is valid while flushing
		}

		virtual void setmodif(uBOOL xismodif, Base4D* bd, BaseTaskInfo* context)
		{
			if (xismodif)
			{
				fCurrentSizeToWrite = btp->CurrentSizeInMem;
				//fStopWrite = 0;
			}
#if debugLogEventPageIndex
			gDebugIndexEvent.AddMessage(L"setmodif", dbgIndexEvent(fCurrentSizeToWrite, num, getaddr(), entete));
#endif
			IObjToFlush::setmodif(xismodif, bd, context);
		}

		inline void ReleaseAndCheck(bool SkipCheck = false) const
		{
		//	xbox_assert(GetRefCount() == 1);
#if debuglr
			if (!SkipCheck)
			{
				if (GetRefCount() == 1 || (GetRefCount() == 2 && modifie()))
				{
				}
				else
				{
					sLONG a = 1;
				}
			}
#endif
			Release();
		}

		static BTreePageIndex* LoadPage(OccupableStack* curstack, sLONG n, IndexInfo* xentete, BaseTaskInfo* context, VError& err);
		static BTreePageIndex* AllocatePage(OccupableStack* curstack, IndexInfo* xentete, BaseTaskInfo* context, VError& err);
		VError Place(OccupableStack* curstack, uBOOL *h, BTitemIndex* &v, IndexKeyArray& tempkeys, BaseTaskInfo* context, const VCompareOptions& inOptions);
		VError Detruire(OccupableStack* curstack, uBOOL *h, BTitemIndex *v, uBOOL *doisdel, IndexKeyArray& tempkeys, BaseTaskInfo* context, const VCompareOptions& inOptions);
		inline sLONG GetNum(void) { return(num); };
		BTitemIndex* CopyKey(sLONG n, void* prealloc = nil);
		BTitemIndex* GetItem(OccupableStack* curstack, sLONG n, Boolean chargesous, IndexKeyArray &tempkeys, BaseTaskInfo* context, VError& err);
		BTitemIndex* GetItemPtr(sLONG n);
		VError MoveAndAddItems(sLONG from, sLONG howmany, BTreePageIndex* into);
		VError MoveAndInsertItemsAtBegining(sLONG from, sLONG howmany, BTreePageIndex* into);
		VError InsertKey(sLONG n, BTitemIndex* u, Boolean AvecSousPage = false);
		VError SetKey(sLONG n, BTitemIndex* u, Boolean AvecSousPage = false);
		VError AddKey(void* p, sLONG qui, BTreePageIndex* sousBT);
		sLONG GetNKeys() const;
		void SetSousPage(sLONG n, BTreePageIndex* sousBT);
		BTreePageIndex* GetSousPage(OccupableStack* curstack, sLONG n, VError &err, BaseTaskInfo* context);
		sLONG GetSousPageNum(sLONG n);
		sLONG GetQui(sLONG n);
		void SetQui(sLONG n, sLONG xqui);
		inline void SetEncours(uBOOL b) { encours=b; };
		void SupKey(sLONG n);
		void SupKeys(sLONG n, sLONG howmany);
		void DelKeys(sLONG from);
		void LibereEspaceMem(OccupableStack* curstack);
		void DelFromFlush(OccupableStack* curstack);
		void LiberePage(OccupableStack* curstack, BaseTaskInfo* context);
		void underflow(OccupableStack* curstack, BTreePageIndex* pc, sLONG s, uBOOL *h, uBOOL *doisdel, IndexKeyArray& tempkeys, BaseTaskInfo* context);
		void del(OccupableStack* curstack, BTreePageIndex* from, sLONG k, uBOOL *h, uBOOL *doisdel, IndexKeyArray& tempkeys, BaseTaskInfo* context);

		VError ParseAll(OccupableStack* curstack, Bittab* b, BTitemIndex* val1, BaseTaskInfo* context, const VCompareOptions& inOptions, ProgressEncapsuleur* InProgress);

		VError Fourche(OccupableStack* curstack, Bittab* b, BTitemIndex* val1, uBOOL xstrict1, BTitemIndex* val2, uBOOL xstrict2, BaseTaskInfo* context, 
										ProgressEncapsuleur* InProgress, const VCompareOptions& inOptions, BTitemIndex** outVal = nil, bool findOnlyOne = false);

		VError SelectAllKeys(OccupableStack* curstack, Bittab* b, BaseTaskInfo* context, ProgressEncapsuleur* InProgress, BTitemIndex** outVal = nil);

		VError FindKeyInArray(OccupableStack* curstack, Bittab* b, DB4DArrayOfConstValues* values, const void* &CurVal, BaseTaskInfo* context, const VCompareOptions& inOptions, ProgressEncapsuleur* InProgress);
													

		VError SortSel(OccupableStack* curstack, sLONG& curfic, sLONG maxnb, Bittab* filtre, Selection* sel, uBOOL ascent, BaseTaskInfo* context, 
						ProgressEncapsuleur* InProgress, Boolean TestUnicite, BTitemIndex* &curkey, const VCompareOptions& inOptions, Bittab* inWafSelbits, WafSelection* outWafSel);
		//inline BTreePageIndex* GetxOldOne(BaseTaskInfo* context) { return /*((BTreePageIndex*)GetOldOne(context))*/ this; };

		VError ThrowError( VError inErrCode, ActionDB4D inAction);

		sLONG CompareKeys(const BTitemIndex *val, sLONG inIndex, const VCompareOptions& inOptions);
		sLONG CompareKeys(sLONG inIndex, const BTitemIndex *val, const VCompareOptions& inOptions);

		inline static sLONG CalculateEmptyLenOnDisk() 
		{ 
			//return sizeof(BTreePageIndexDisk)-sizeof(sLONG); 
			return offsetof(BTreePageIndexDisk, suite);
		};

		inline static sLONG CalculateEmptyLen() 
		{ 
			//return sizeof(BTreePageIndex)-sizeof(sLONG);
			// return offsetof(BTreePageIndex, fBTPdata.suite);	GCC complains offsetof is only authorized on POD types
			BTreePageIndex *p = (BTreePageIndex*)0;
			return (char*) &p->fBTPdata.suite - (char*) p;
		};

		static sLONG CalculateDefaultLen(IndexInfo *xentete);
		Boolean CheckFit(sLONG len);
		void Reorganize();

		VError GetDistinctValues(OccupableStack* curstack, DB4DCollectionManager& outCollection, BaseTaskInfo* context, Bittab* filtre, 
								sLONG &curpage, sLONG &curvalue, BTitemIndex* &curkey, ProgressEncapsuleur* InProgress, VCompareOptions& inOptions, 
								void* prealloc, distinctvalue_iterator& xx);

		VError QuickGetDistinctValues(OccupableStack* curstack, QuickDB4DArrayOfValues *outCollection, BaseTaskInfo* context, Bittab* filtre, 
								sLONG &curpage, sLONG &curvalue, BTitemIndex* &curkey, ProgressEncapsuleur* InProgress, VCompareOptions& inOptions, void* prealloc);

		VError CalculateColumnFormulas(OccupableStack* curstack, ColumnFormulas* formules, BaseTaskInfo* context, Bittab* filtre, sLONG &curpage, 
																	 ProgressEncapsuleur* InProgress, Boolean& stop);

		VError CalculateMin(OccupableStack* curstack, BaseTaskInfo* context, Bittab* filtre, sLONG &curpage, 
												ProgressEncapsuleur* InProgress, Boolean& stop, VValueSingle* &result);

		VError CalculateMax(OccupableStack* curstack, BaseTaskInfo* context, Bittab* filtre, sLONG &curpage, 
												ProgressEncapsuleur* InProgress, Boolean& stop, VValueSingle* &result);

		VError CalculateColumnFormulasOnOneKey(ColumnFormulas* formules, BTitemIndex* u, Boolean& stop, BaseTaskInfo* context);

		inline IndexInfo* GetEntete() { return entete; };

		VError FindKeyAndBuildPath(OccupableStack* curstack, BTitemIndex* val, const VCompareOptions& inOptions, BaseTaskInfo* context, VDB4DIndexKey* outKey, Boolean& trouve);
		VError NextKey(OccupableStack* curstack, const VDB4DIndexKey* inKey, sLONG level, BaseTaskInfo* context, VDB4DIndexKey* outKey, Boolean& outlimit);
		VError GetFirstKey(OccupableStack* curstack, BaseTaskInfo* context, VDB4DIndexKey* outKey);

		VError ScanIndex(OccupableStack* curstack, Selection* into, sLONG& currec, sLONG inMaxRecords, Boolean KeepSorted, Boolean Ascent, BaseTaskInfo* context, Bittab* filtre, ProgressEncapsuleur* InProgress);

		inline VSize GetSizeToFree() const
		{
			return (VSize)(fDataSize + (BTreePageIndex::CalculateEmptyLen() - BTreePageIndex::CalculateEmptyLenOnDisk()));
		}

		inline void DoNotVerify()
		{
			fDebugVerify = false;
		}

		void SetBTP(BTreePageIndexDisk* newBTP)
		{
			btp = newBTP;
			fCurrentSizeToWrite = btp->CurrentSizeInMem;
		}

		VSize GetSize() const
		{
			return btp->TrueDataSize;
		}

		/*
		inline void ForbidWrite()
		{
			if (fStopWrite == 0)
				VInterlocked::Increment(&fStopWrite);
		}
		*/


#if debuglr
		Boolean CheckPageLength()
		{
			xbox_assert(oldlen == btp->CurrentSizeInMem);
			return true;
		}

		void Display(void);
		void checktabmem(void);

		void DisplayKeys(const VString& message);

		void checkPosSousBT(BTreePageIndex* sousBT);

		void CheckPageKeys();
		void CheckPageOwner();

		void checkWrongRecordRef(sLONG limit, BTitemIndex* &curval);

#endif
#if debugCheckIndexPageOnDiskInDestructor
		void DebugCheckPageOnDisk();
#endif
	
	protected:
		virtual ~BTreePageIndex();
		//BTreePageIndex* original;
		BTreePageIndex* tabmem[kNbKeyParPageIndex+1];
		sLONG fCurrentSizeToWrite;
		sLONG fDataSize; // taille alloue en memoire
		sLONG fSizeKeyRemoved; // la somme de toutes les cles qui ont ete retirees
		sLONG num;
		sLONG oldlen;
		//sLONG fStopWrite;
		IndexInfo *entete;
		BTreePageIndexDisk* btp;
		uBOOL encours;
		uBOOL IsCluster;
		uBOOL fDebugVerify;
		uBOOL filler1;
		//DataAddr4D origaddr;
		//sLONG origlen;
		VCriticalSection fLoadPageMutex;
		BTreePageIndexDisk fBTPdata;
};

class IndexHeaderBTreeRoot : public IndexHeader
{
	public:
		IndexHeaderBTreeRoot(IndexInfo *xentete);
		
		//inline	sLONG	GetFirstPage(void) const { return(HBT.FirstPage); };			// J.A. DataTools
		VError ThrowError( VError inErrCode, ActionDB4D inAction);

		virtual sLONG GetLen(void);
		virtual VError PutInto(VStream& buf);
		virtual VError GetFrom(VStream& buf);

		virtual bool SaveObj(VSize& outSizeSaved);

		virtual VError LibereEspaceDisk(OccupableStack* curstack, VDB4DProgressIndicator* InProgress = nil);

	protected:
		HeaderBtreeDisk HBT;
		IndexInfo *entete;
		VCriticalSection fLoadPageMutex;
};


class IndexHeaderBTree : public IndexHeaderBTreeRoot
{
	public:
		inline IndexHeaderBTree(IndexInfo *xentete):IndexHeaderBTreeRoot(xentete) 
		{ 
			firstpage = nil;
		};
		
		
		virtual BTreePageIndex* CrePage(void);
		virtual BTreePageIndex* LoadPage(OccupableStack* curstack, DataAddr4D addr, sLONG len, BaseTaskInfo* context, VError& err, sLONG xnum);
		
		virtual void TryToFreeMem(sLONG allocationBlockNumber, VSize combien, VSize& tot);

		virtual VError PlaceCle(OccupableStack* curstack, BTitemIndex *cle, sLONG xqui, IndexKeyArray& tempkeys, BaseTaskInfo* context);
		virtual VError DetruireCle(OccupableStack* curstack, BTitemIndex *cle, sLONG xqui, IndexKeyArray& tempkeys, BaseTaskInfo* context);
		virtual VError LibereEspaceDisk(OccupableStack* curstack, VDB4DProgressIndicator* InProgress = nil);
		virtual void LibereEspaceMem(OccupableStack* curstack);
		
		VError ChargeFirstPage(OccupableStack* curstack, BaseTaskInfo* context, uBOOL doiscreer=false);

		virtual Bittab* FindKeyInArray(OccupableStack* curstack, DB4DArrayOfValues* values, BaseTaskInfo* context, VError& err, VDB4DProgressIndicator* InProgress, const VCompareOptions& inOptions, Bittab* dejasel=nil);

		virtual Bittab* Fourche(OccupableStack* curstack, BTitemIndex* val1, uBOOL xstrict1, BTitemIndex* val2, uBOOL xstrict2, BaseTaskInfo* context, VError& err, 
														VDB4DProgressIndicator* InProgress, const VCompareOptions& inOptions, Bittab* dejasel=nil, BTitemIndex** outVal = nil);

		virtual Bittab* ParseAll(OccupableStack* curstack, BTitemIndex* val1, BaseTaskInfo* context, VError& err, const VCompareOptions& inOptions, VDB4DProgressIndicator* InProgress);

		virtual sLONG FindKey(OccupableStack* curstack, BTitemIndex* val, BaseTaskInfo* context, const VCompareOptions& inOptions, Bittab* dejasel, BTitemIndex** outVal);
		
		virtual Selection* SortSel(OccupableStack* curstack, Selection* sel, uBOOL ascent, BaseTaskInfo* context, VError& err, 
										VDB4DProgressIndicator* InProgress = nil, Boolean TestUnicite = false, Bittab* inWafSelbits = nil, WafSelection* outWafSel = nil);

		virtual VError GetDistinctValues(OccupableStack* curstack, DB4DCollectionManager& outCollection, BaseTaskInfo* context, Bittab* filtre, 
											VDB4DProgressIndicator* InProgress, VCompareOptions& inOptions);

		virtual VError QuickGetDistinctValues(OccupableStack* curstack, QuickDB4DArrayOfValues *outCollection, BaseTaskInfo* context, Bittab* filtre, 
											VDB4DProgressIndicator* InProgress, VCompareOptions& inOptions);

		void	SetFirstPage(BTreePageIndex* first, BaseTaskInfo* context);

		virtual VError CalculateColumnFormulas(OccupableStack* curstack, ColumnFormulas* formules, BaseTaskInfo* context, Bittab* filtre, VDB4DProgressIndicator* InProgress);

		virtual VError CalculateMin(OccupableStack* curstack, BaseTaskInfo* context, Bittab* filtre, VDB4DProgressIndicator* InProgress, VValueSingle* &result);
		
		virtual VError CalculateMax(OccupableStack* curstack, BaseTaskInfo* context, Bittab* filtre, VDB4DProgressIndicator* InProgress, VValueSingle* &result);

		virtual VError FindKeyAndBuildPath(OccupableStack* curstack, BTitemIndex* val, const VCompareOptions& inOptions, BaseTaskInfo* context, VDB4DIndexKey* outKey);
		virtual VError NextKey(OccupableStack* curstack, const VDB4DIndexKey* inKey, BaseTaskInfo* context, VDB4DIndexKey* outKey);

		virtual VError ScanIndex(OccupableStack* curstack, Selection* into, sLONG inMaxRecords, Boolean KeepSorted, Boolean Ascent, BaseTaskInfo* context, Bittab* filtre,sLONG &nbtrouves, VDB4DProgressIndicator* InProgress);

		virtual sLONG8 GetNBDiskAccessForAScan(Boolean WithSelFilter) { return IHD.nbpage; };

		inline BTreePageIndex* _GetFirstPage() const { return firstpage; };

		virtual void Update(Table* inTable)
		{
		}


#if debuglr
		inline sLONG GetNbPage(void) { return(IHD.nbpage); };
		virtual void CheckAllPageKeys();
		virtual void CheckAllPageOwner();
#endif
		
	protected:
		BTreePageIndex *firstpage;
};
							
									
IndexHeader* CreIndexHeaderBTree(IndexInfo *xentete);

const sLONG kMaxPageLevelInKeyIterator = 20;

class IndexKeyPathItem
{
	public:
		BTreePageIndex* fPage;
		sLONG fPos;

};


						/* ------------------------------------------------------------------ */


class BTitemIndexHolder
{
public:
	inline BTitemIndexHolder(const BTitemIndex* val, IndexInfo* ind, Boolean isBeginWith, Boolean isLike, BaseTaskInfo* context) 
	{ 
		fContext = context;
		fAddr = -2;
		fKey = (BTitemIndex*)val;
		fTransID = 0;
		if (fKey != nil)
		{
			((BTitemIndex*)fKey)->SetInd(ind);
			((BTitemIndex*)fKey)->SetQueryAttribute(isBeginWith, isLike);
			fKey->souspage = 2; 
		}
	};

	inline BTitemIndexHolder(BTitemIndex* val, BaseTaskInfo* context) 
	{ 
		fContext = context;
		fAddr = -1;
		fKey = val;
		fTransID = 0;
		if (fKey != nil)
			fKey->Use();
	};

	inline BTitemIndexHolder(const BTitemIndexHolder& other) 
	{ 
		fContext = other.fContext;
		fAddr = other.fAddr;
		fKey = other.fKey;
		fTransID = other.fTransID;
		if (fKey != nil)
			fKey->Use();
	};

	inline BTitemIndexHolder& operator =(const BTitemIndexHolder& other) 
	{ 
		fContext = other.fContext;
		fAddr = other.fAddr;
		fKey = other.fKey; 
		fTransID = other.fTransID;
		fKey->Use();
		return *this;
	};

	inline ~BTitemIndexHolder() 
	{ 
		if (fKey != nil)
			fKey->Unuse(); 
	};


	inline BTitemIndex* GetKeyForDebug() const
	{
		return fKey;
	}

	inline BaseTaskInfo* GetContext() const
	{
		return fContext;
	}

	BTitemIndex* StealKey();
	BTitemIndex* RetainKey() const;
	VError SaveToTemp(sLONG allocationBlockNumber, Transaction* trans, sLONG& totfreed);
	VError LoadFromTemp() const;

	DataAddr4D fAddr;
	sLONG fTransID;

private:
	BaseTaskInfo* fContext;
	mutable BTitemIndex* fKey;
};

bool operator < (const BTitemIndexHolder& v1, const BTitemIndexHolder& v2);

//typedef multiset<BTitemIndexHolder, less<BTitemIndexHolder>, cache_allocator<BTitemIndexHolder> > mapIndexKeys;

typedef multiset<BTitemIndexHolder> mapIndexKeys;


						/* ------------------------------------------------------------------ */



class IndexKeyIterator : public ObjAlmostInCache
{
	public:
	
		IndexKeyIterator(IndexInfo* ind, Bittab* filter, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, const VString& inMess, OccupableStack* curstack );
		~IndexKeyIterator();

		Boolean FirstKey(BaseTaskInfo* context, VError& err, sLONG* CurElemToProgress = nil);
		Boolean NextKey(BaseTaskInfo* context, VError& err, sLONG* CurElemToProgress = nil);

		Boolean LastKey(BaseTaskInfo* context, VError& err, sLONG* CurElemToProgress = nil) { return false; /* a faire */ };
		Boolean PreviousKey(BaseTaskInfo* context, VError& err, sLONG* CurElemToProgress = nil)  { return false; /* a faire */ };

		sLONG GetRecNum() const;
		const BTitemIndex* GetKey() const;

	protected:

		VError _IncCurPos(sLONG* CurElemToProgress = nil);
		void _DisposePages();

		void _DisposeSel();
		VError _SetSel(Selection* sel);

		void _DisposeKey();

		void _PosToFirstLeft(BTreePageIndex* sousBT, BaseTaskInfo* context, VError& err);

		void _MatchAddWithCurKey();
		Boolean _MatchDelWithCurKey();

		void _RetainKey();

		IndexInfo *fInd;
		BTitemIndex* fKey;
		IndexKeyPathItem fPagePath[kMaxPageLevelInKeyIterator];
		sLONG fNbLevel;
		Selection* fSel;
		SelectionIterator fSelIter;
		sLONG fRecNum;
		Boolean fWasJustReset, fCleAddEnTrans, fCleDelEnTrans;
		Boolean fCurKeyInTrans;
		Bittab* fFilter;
		BaseTaskInfo* fContext;
		ProgressEncapsuleur fProgress;
		sLONG fCurProgressPos;
		VCompareOptions fStrictOptions;

		mapIndexKeys* fSavedKeys;
		mapIndexKeys* fDeletedKeys;
		mapIndexKeys::iterator fCurSavedKey, fCurDeletedKey;
		mapIndexKeys::iterator fEndSavedKey, fEndDeletedKey;

		BTitemIndex* fKeyInTrans;
		OccupableStack* fCurstack;

		char keyprealloc[KeyPreAllocSize];


};
									
									/* -----------------------------------------------  */
									
									
class Base4D;

class Field;

class BTreePageIndex;

class ComplexSelection;

#if debugIndexPage
typedef set<BTreePageIndex*> debug_ListOfPages;
#endif

typedef enum
{
	index_read = 0,
	index_write = 1
} index_access_right;

class IndexInfo : public ObjInCacheMemory, public IObjToFlush, public IObjToFree, public IBaggable
{
	public:
		inline IndexInfo(Boolean UniqueKeys = false)
		{ fDelayRequestCount = 0; encreation=false; Invalid = false; MaxElemEnCreation = 0; fUniqueKeys = UniqueKeys; 
			/*sourcelang_isinited = false; */PosInMap = -1; PosInMapInStruct = -1; fIsOKtoSave = false; /*fIsQuickBuilding = true; */
			fIDisChosen = false;
			fDebugDeja = false;
			fIsAuto = false; 
			header = nil;
			fBuildError = VE_OK;
			fStamp = GetCurIndStamp();
			//fIntlMgr = nil;
			fUpdateOptions.SetDiacritical(true);
			fBigObjectCacheStamp = -1;
			fLastFreedMem = 0;
			fIsRemote = false;
			fValidityRequestCount = 0;
			fValidityWaitingEvent = nil;
			fStructureStamp = 0;
#if debugIndexPage
			debug_CanAddPage = true;
#endif
		};

		void FinalizeForRemote()
		{
			fDelayRequestCount = 0; 
			encreation=false; 
			Invalid = false;
			fIsOKtoSave = false;
			fIsRemote = true;
		}

		inline sLONG GetTyp(void) const { return(InfoTyp); };

		Boolean MatchType(const IndexInfo* other) const
		{
			return (ReduceType(InfoTyp) == ReduceType(other->InfoTyp));
		}

		Boolean MatchType(sLONG typ) const
		{
			return (ReduceType(InfoTyp) == ReduceType(typ));
		}

		static inline sLONG ReduceType(sLONG xtyp)
		{
			if (xtyp >= DB4D_Index_OnOneField_Scalar_Range_Begin && xtyp <= DB4D_Index_OnOneField_Scalar_Range_End)
				return DB4D_Index_OnOneField;
			else
				return xtyp;
		}

		virtual void setmodif(uBOOL xismodif, Base4D* bd, BaseTaskInfo* context)
		{
			xbox_assert(false); // ne devrait pas etre appelee
			Save();
		}

		virtual bool SaveObj(VSize& outSizeSaved)
		{
			xbox_assert(false); // ne devrait pas passer par la
			outSizeSaved = 0;
			return true;

		}

		virtual bool FreeMem(sLONG allocationBlockNumber, VSize combien, VSize& outSizeFreed);


		inline sLONG GetReducedType() const
		{
			return ReduceType(InfoTyp);
		}

		inline void SetTyp(sLONG typ) { InfoTyp=typ; };
		inline void SetUnique(Boolean UniqueKeys) { fUniqueKeys = UniqueKeys; };
		inline const VString& GetName() const { return fName; };
		VError SetName(const VString& name, CDB4DBaseContext* inContext);
		//virtual sLONG GetLen(void);
		virtual VError PutInto(VStream& buf, Base4D* xdb, Boolean WithHeader = true);
		virtual VError GetFrom(VStream& buf, Base4D* xdb, Boolean WithHeader = true, Boolean oldformat = false);
		void ModifyIt(BaseTaskInfo* context);
		inline Base4D* GetDB(void) const { return(bd); };
		
		virtual sLONG GetScalarKind() const { return -1; };

		virtual Boolean CanBeScalarConverted() const { return false; };

		virtual IndexInfo* ConvertToScalar() const
		{
			xbox_assert(false);
			return nil;
		}
		
		inline Boolean IsScalar() const
		{
			return GetScalarKind() != -1;
		}

		virtual sLONG GetTemplateType() const
		{
			return 0;
		}

		virtual Boolean MatchOtherDataKind(IndexInfo* ind) const
		{
			return false;
		};
		
		virtual void* NewMapKeys() const
		{
			xbox_assert(false);
			return nil;
		}
		
		virtual void DeleteMapKeys(void* mapkey) const
		{
			xbox_assert(false);
		}
		
		virtual void CopyMapKeys(const void* source, void* dest) const
		{
			xbox_assert(false);
		}
		
		virtual VError PlaceCleAllForTrans(void* xvals, BaseTaskInfo* context)
		{
			xbox_assert(false);
			return VE_DB4D_NOTIMPLEMENTED;
		}
		
		virtual VError DetruireAllCleForTrans(void* xvals, BaseTaskInfo* context)
		{
			xbox_assert(false);
			return VE_DB4D_NOTIMPLEMENTED;
		}

		inline void SetDB(Base4D *db) 
		{ 
			bd=db;
			//fIntlMgr = db->GetIntlMgr(); 
			//fUpdateOptions.SetIntlManager(fIntlMgr);
			fUpdateOptions.SetDiacritical(true);
		};

		inline IndexHeader* GetHeader(void) { return(header); };
		inline ClusterSel* GetClusterSel(OccupableStack* curstack) { return(header->GetClusterSel(curstack)); };
		//inline VIntlMgr* GetIntlMgr() const { return fIntlMgr; };
		inline const VCompareOptions& GetStrictOptions() const { return fUpdateOptions; };
		inline void SetHeader(IndexHeader* head) { header=head; };
		virtual BTitemIndex* CreateIndexKey(const void* p)=0;
		virtual void CalculDependence(void)=0;
		virtual void DeleteFromDependencies(void)=0;
		virtual void TouchDependencies() = 0;
		virtual VError PlaceCle(OccupableStack* curstack, FicheInMem *rec, BaseTaskInfo* context)=0;
		virtual VError DetruireCle(OccupableStack* curstack, FicheInMem *rec, BaseTaskInfo* context)=0;
		virtual VError PlaceCleForTrans(OccupableStack* curstack, BTitemIndex* val, BaseTaskInfo* context)=0;
		virtual VError DetruireCleForTrans(OccupableStack* curstack, BTitemIndex* val, BaseTaskInfo* context)=0;

		inline void Open(index_access_right access) const 
		{ 
			fAccess.Lock(access == index_read ? RWS_ReadOnly :  RWS_WriteExclusive);
		}

		inline void Close() const 
		{ 
			fAccess.Unlock(); 
		}

		inline bool TryToOpen(index_access_right access) const
		{
			return fAccess.TryToLock(access == index_read ? RWS_ReadOnly :  RWS_WriteExclusive);
		}

		Boolean WaitEndOfQuickBuilding();
		virtual uBOOL Egal(IndexInfo* autre)=0;

		virtual uBOOL MayBeSorted(void);

		virtual Boolean NeedToRebuildIndex(VIntlMgr* NewIntlMgr, bool stringAlgoHasChanged, bool keywordAlgoHasChanged) = 0;

		virtual bool IsOkToFreeMem() const
		{
			return TryToOpen(index_write);
		}

		virtual void ReleaseFreeMem() const
		{
			Close();
		}

		inline void LockValidity() const
		{
			fValidityMutex.Lock();
		}

		inline void UnLockValidity() const
		{
			fValidityMutex.Unlock();
		}

		inline void EnCreation(sLONG n) { encreation=true; CurElemCreate=n; };
		inline void FinCreation(void) { SetStamp(); encreation=false; MaxElemEnCreation = 0; };
		inline sLONG CurElemEnCreation() { if (encreation) return CurElemCreate; else return -2; };
		inline sLONG GetMaxElemEnCreation() { return MaxElemEnCreation; };
		inline void SetMaxElemEnCreation(sLONG n) { MaxElemEnCreation = n; };
		inline Boolean IsBuilding() const { fValidityMutex.Lock(); Boolean res = encreation; fValidityMutex.Unlock(); return res; };
		
		virtual VError GenereIndex(BaseTaskInfo* context, VDB4DProgressIndicator* InProgress)=0;


		virtual Bittab* FindKeyInArray(DB4DArrayOfValues* values, BaseTaskInfo* context, VError& err, VDB4DProgressIndicator* InProgress, const VCompareOptions& inOptions, Bittab* dejasel=nil);
		
		virtual Bittab* Fourche(BTitemIndex* val1, uBOOL xstrict1, BTitemIndex* val2, uBOOL xstrict2, BaseTaskInfo* context, VError& err, 
										VDB4DProgressIndicator* InProgress, const VCompareOptions& inOptions, Bittab* dejasel=nil, BTitemIndex** outVal = nil);

		virtual Bittab* ParseAll(BTitemIndex* val1, BaseTaskInfo* context, VError& err, const VCompareOptions& inOptions, VDB4DProgressIndicator* InProgress);

		virtual sLONG FindKey(BTitemIndex* val, BaseTaskInfo* context, const VCompareOptions& inOptions, Bittab* dejasel, BTitemIndex** outVal) 
		{ 
			if (fIsRemote)
			{
				xbox_assert(!fIsRemote);
				return -1;
			}
			else
			{
				VCompareOptions options = inOptions;
				options.SetIntlManager(GetContextIntl(context));
				Open(index_read);
				OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
				sLONG result = header->FindKey(curstack, val, context, options, dejasel, outVal);
				Close();
				return result;
			}
		}


		virtual sLONG GetMaxNB(BaseTaskInfo* context)=0;
		virtual uBOOL NeedUpdate(FicheInMem *rec, BaseTaskInfo* context)=0;
		virtual Boolean MatchIndex(FieldsArray* fields, sLONG NbFields);
		
		virtual void GetDebugString(VString& s, sLONG numinstance) = 0;
		virtual void IdentifyIndex(VString& outString, Boolean WithTableName, Boolean WithIndexName, bool simple) const = 0;
		
		void SetInvalid();
		void SetValid();

		Boolean AskForValid(BaseTaskInfo* context, Boolean CheckAlsoEnCreation = true) const;
		Boolean AskForValidOutsideContext(BaseTaskInfo* context, Boolean CheckAlsoEnCreation = true) const;
		void ReleaseValid();
		void IncAskForValid();
		
		// from IBaggable
		virtual VError	LoadFromBagWithLoader( const VValueBag& inBag, VBagLoader *inLoader, void* inContext = nil);
		virtual VError	SaveToBag( VValueBag& ioBag, VString& outKind) const;

		VError ThrowError( VError inErrCode, ActionDB4D inAction);

		virtual sLONG CalculateDefaultSizeKey() = 0;
		virtual sLONG CalulateKeyLength(const BTitemIndex* u) = 0;
		virtual sLONG CalulateDataPtrKeyLength(void* data) = 0;
		virtual sLONG CalulateFullKeyLengthInMem(const BTitemIndex* u) = 0;
		virtual sLONG CompareKeys(const BTitemIndex *val1, const BTitemIndex *val2, const VCompareOptions& inOptions) = 0;
		BTitemIndex* AllocateKey(sLONG len);
		BTitemIndex* CopyKey(const BTitemIndex* key, void* prealloc = nil);

		inline void FreeKey(BTitemIndex* u, void* prealloc = nil) const 
		{ 
			xbox_assert(u != nil);
#if debug_BTItemIndex
			u->debug_dispose();
#endif
			if ((void*)u != prealloc)
				FreeFastMem((void*)u);
		};

		virtual BTitemIndex* BuildKey(FicheInMem *rec, VError &err, Boolean OldOne = false) = 0;
		virtual ValPtr CreateVValueWithKey(const BTitemIndex* key, VError &err);
		virtual void SwapByteKeyData(BTitemIndex* key) = 0;

		sLONG8 ComputeMaxKeyToFit(sLONG level, sLONG8 MaxElemsInPage, sLONG8 RequiredElemsInPage, Boolean CheckAllTree = true);
		sLONG CalculateGenerationsLevels(sLONG8 nbrec, sLONG MaxElemsInPage, sLONG RequiredElemsInPage);
		VError GenereFullIndex(VDB4DProgressIndicator* InProgress, sLONG MaxElemsInPage, sLONG RequiredElemsInPage);
		virtual Boolean GenereQuickIndex(VError &err, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, sLONG MaxElemsInPage, sLONG RequiredElemsInPage) = 0;

		inline Boolean IsUniqueKey() const { return fUniqueKeys; };

		virtual VError GetDistinctValues(DB4DCollectionManager& outCollection, BaseTaskInfo* context, Bittab* filtre, 
											VDB4DProgressIndicator* InProgress, const VCompareOptions& inOptions);

		virtual VError QuickGetDistinctValues(QuickDB4DArrayOfValues *outCollection, BaseTaskInfo* context, Bittab* filtre, 
											VDB4DProgressIndicator* InProgress, const VCompareOptions& inOptions)
		{
			return VE_DB4D_NOTIMPLEMENTED;
		}

		virtual VError CalculateColumnFormulas(ColumnFormulas* formules, BaseTaskInfo* context, Bittab* filtre, VDB4DProgressIndicator* InProgress) { return VE_DB4D_NOTIMPLEMENTED; };

		virtual VError CalculateMin(BaseTaskInfo* context, Bittab* filtre, VDB4DProgressIndicator* InProgress, VValueSingle* &result) 
		{ result = nil; return VE_DB4D_NOTIMPLEMENTED; };

		virtual VError CalculateMax(BaseTaskInfo* context, Bittab* filtre, VDB4DProgressIndicator* InProgress, VValueSingle* &result) 
		{ result = nil; return VE_DB4D_NOTIMPLEMENTED; };

//		virtual VArrayStruct* GetSourceLang() = 0;

		virtual VError FindKeyAndBuildPath(BTitemIndex* val, const VCompareOptions& inOptions, BaseTaskInfo* context, VDB4DIndexKey* outKey);
		virtual VError NextKey(const VDB4DIndexKey* inKey, BaseTaskInfo* context, VDB4DIndexKey* outKey);

		virtual Table* GetTargetTable() = 0;
		virtual Field* GetTargetField()
		{
			return nil;
		}


		inline Boolean IsDelayed() const { return fDelayRequestCount > 0; }; // pas de occupe / libere , c'est voulu
		virtual void CheckIfIsDelayed() = 0;
		virtual void DelayIndex() = 0;
		virtual void SetDelayIndex(sLONG inAlreadyDelayRequestCount) = 0;
		virtual void AwakeIndex(VDB4DProgressIndicator* inProgress) = 0;

		virtual bool IsOnPrimKey() = 0;

		//inline void UseForQuery() { ; /* a faire */ };
		//inline void FinishedUseForQuery() { ; /* a faire */ };

		inline void SetPlace(sLONG place) { PosInMap = place; };
		inline void SetPlaceInStruct(sLONG place) { PosInMapInStruct = place; };
		inline sLONG GetPlace() const { return PosInMap; };
		inline sLONG GetPlaceInStruct() const { return PosInMapInStruct; };

		VError Save();
		VError SaveInStruct(Boolean ForceSave = false);

		inline uBOOL GetDebugDeja() { uBOOL x = fDebugDeja; fDebugDeja = true; return x; };

		inline VUUID& GetID() { return fID; };
		inline const VUUID& GetID() const { return fID; };
		inline void GenID() 
		{ 
			if (!fIDisChosen)
			{
				fID.Regenerate(); 
				fIDisChosen = true;
			}
		};

		inline void SetID(const VUUID& id)
		{
			xbox_assert(!fIDisChosen);
			fID = id;
			fIDisChosen = true;
		};

		inline void SetOKtoSave() { fIsOKtoSave = true; };

		virtual void CreateHeader() = 0;

		virtual Selection* ScanIndex(sLONG inMaxRecords, Boolean KeepSorted, Boolean Ascent, BaseTaskInfo* context, VError& err, Selection* filtre, VDB4DProgressIndicator* InProgress);

		inline Boolean IsAuto() const { return fIsAuto; };
		inline void SetAuto(Boolean xauto) { fIsAuto = xauto; };

		inline sLONG8 GetNBDiskAccessForAScan(Boolean WithSelFilter)
		{
			xbox_assert(!fIsRemote);
			if (header == nil)
				return -1;
			else
				return header->GetNBDiskAccessForAScan(WithSelFilter);
		};

		VError InitTablesAddr(OccupableStack* curstack) { xbox_assert(!fIsRemote); return header->InitTablesAddr(curstack); };
		VError NormalizeTablesAddr(OccupableStack* curstack) { xbox_assert(!fIsRemote); return header->NormalizeTablesAddr(curstack); };

		virtual VError ValidateIndexInfo() = 0;

		inline VError GetBuildError() const { return fBuildError; };
		inline void SetBuildError(VError err) { fBuildError = err; };

		inline void ClearQuickBuilding()
		{
			//VTaskLock lock(&fQuickBuildingMutex);
			//fIsQuickBuilding = false;
			fQuickBuildingWaitingEvent.Unlock();
		}

		inline void SetQuickBuilding()
		{
			//VTaskLock lock(&fQuickBuildingMutex);
			//fIsQuickBuilding = false;
			fQuickBuildingWaitingEvent.ResetAndLock();
		}

		inline void SetStamp()
		{
			xbox_assert(!fIsRemote);
			fStamp = GetNewIndStamp();
		}

		static uLONG GetNewIndStamp();
		static uLONG GetCurIndStamp();

		inline void SetStructureStamp( uLONG inStructureStamp)
		{
			xbox_assert(!fIsRemote);
			fStructureStamp = inStructureStamp;
		}

		inline uLONG GetStructureStamp() const
		{
			return fStructureStamp;
		}

		virtual VError ClearFromSystemTableIndexCols() = 0;
		virtual VError AddToSystemTableIndexCols() = 0;

		inline sLONG GetHeaderType() const
		{
			if (fIsAuto)
				return DB4D_Index_AutoType;
			else
				return typindex;
		};

		virtual Boolean LockTargetTables(BaseTaskInfo* context) = 0;
		virtual void UnLockTargetTables(BaseTaskInfo* context) = 0;

		virtual VError JoinWithOtherIndex(IndexInfo* other, Bittab* filtre1, Bittab* filtre2,ComplexSelection* result, BaseTaskInfo* context, 
											VCompareOptions& inOptions, VProgressIndicator* inProgress = nil, bool leftjoin = nil, bool rightjoin = nil)
		{
			return VE_DB4D_NOTIMPLEMENTED;
		}

		virtual VError JoinWithOtherIndexNotNull(IndexInfo* other, Bittab* filtre1, Bittab* filtre2, Bittab* result, BaseTaskInfo* context, VCompareOptions& inOptions, VProgressIndicator* inProgress = nil)
		{
			return VE_DB4D_NOTIMPLEMENTED;
		}

		inline Boolean IsRemote() const
		{
			return fIsRemote;
		}

		Boolean HasNulls(OccupableStack* curstack)
		{
			if (header == nil)
				return false;
			else
				return header->HasNulls(curstack);
		}

		virtual Boolean SourceIsABlob() const
		{
			return false;
		}

		virtual bool MustBeRebuild() = 0;

		bool IsInvalidOnDisk();
		void SetInvalidOnDisk();
		void SetValidOnDisk();


#if debugLeaksAll
		virtual bool OKToTrackDebug() const
		{
			return false;
		}
#endif

#if debugIndexPage
		void Can_Debug_AddPage() { debug_CanAddPage = true; };

		void Cannot_Debug_AddPage() { debug_CanAddPage = false; };

		void debug_ClearPageList() { debug_fPageList.clear(); };

		void debug_AddPage(BTreePageIndex* page)
		{
			if (debug_CanAddPage)
			{
				try
				{
					debug_fPageList.insert(page);
				}
				catch (...)
				{
					
				}
			}
		}

		void debug_DelPage(BTreePageIndex* page)
		{
			debug_fPageList.erase(page);
		}

		debug_ListOfPages* debug_GetPageList() { return &debug_fPageList; };
#endif

	protected:
		virtual ~IndexInfo();

		/*
		uBOOL occupe(Boolean ForceWaitEvenDringFlush = false, sLONG signature = 0) const { return false; };
		uBOOL IsOkToOccupe(sLONG signature = 0) const { return false; };
		void libere(sLONG signature = 0) const { ; };

		void occupe() { ; };
		void libere() { ; };
		*/

		VStr<32> fName;
		VUUID fID;
		VError fBuildError;
		sLONG typindex;
		sLONG InfoTyp;
		sLONG PosInMap;
		sLONG PosInMapInStruct;
		IndexHeader *header;
		Base4D *bd;
		sLONG CurElemCreate;
		sLONG MaxElemEnCreation;
		uBOOL encreation, Invalid, fUniqueKeys, /*sourcelang_isinited, */fIsDelayed, fIsOKtoSave,/* fIsQuickBuilding, */ fIDisChosen;
		Boolean fIsRemote;
//		VArrayStruct fSourceLang;
		uBOOL fDebugDeja;
		uBOOL fIsAuto;
		vxSyncEvent fQuickBuildingWaitingEvent;
		VNonVirtualCriticalSection fQuickBuildingMutex;
		vxSyncEvent* fValidityWaitingEvent;
		mutable VNonVirtualCriticalSection fValidityMutex;
		uLONG fStamp;
		sLONG fDelayRequestCount;
		mutable VCriticalSection fDelayRequestCountMutex;
		mutable sLONG fValidityRequestCount;
		mutable ReadWriteSemaphore fAccess;
		//VIntlMgr* fIntlMgr;
		VCompareOptions fUpdateOptions;
		sLONG fBigObjectCacheStamp;
		VSize fLastFreedMem;
		uLONG fStructureStamp;

		static uLONG sIndexInfoStamp;
		static VCriticalSection sIndexInfoStampMutex;

#if debugIndexPage
		debug_ListOfPages debug_fPageList;
		Boolean debug_CanAddPage;
#endif
};

inline void BTitemIndex::Unuse() const { souspage--; if (souspage<=0) GetInd()->FreeKey((BTitemIndex*)this); };


class IndexInfoFromField : public IndexInfo
{
typedef IndexInfo inherited;
	public:
		inline IndexInfoFromField(void) { InfoTyp=DB4D_Index_OnOneField; crit = nil; fic = nil; };
		IndexInfoFromField(Base4D* db, sLONG xnumfile, sLONG xnumfield, sLONG xtypindex, Boolean UniqueKeys, Boolean CanBeScalar);
		//virtual sLONG GetLen(void);

		virtual Boolean CanBeScalarConverted() const;

		virtual IndexInfo* ConvertToScalar() const;

		virtual Boolean MatchOtherDataKind(IndexInfo* ind) const
		{
			if (ind->GetTyp() == DB4D_Index_OnOneField)
			{
				return (GetDataKind() == ((IndexInfoFromField*)ind)->GetDataKind());
			}
			else
				return false;
		};

		virtual VError PutInto(VStream& buf, Base4D* xdb, Boolean WithHeader = true);
		virtual VError GetFrom(VStream& buf, Base4D* xdb, Boolean WithHeader = true, Boolean oldformat = false);
		virtual BTitemIndex* CreateIndexKey(const void* p);
		virtual void CalculDependence(void);
		virtual void DeleteFromDependencies(void);
		virtual void TouchDependencies();
		virtual VError PlaceCle(OccupableStack* curstack, FicheInMem *rec, BaseTaskInfo* context);
		virtual VError DetruireCle(OccupableStack* curstack, FicheInMem *rec, BaseTaskInfo* context);
		virtual VError PlaceCleForTrans(OccupableStack* curstack, BTitemIndex* val, BaseTaskInfo* context);
		virtual VError DetruireCleForTrans(OccupableStack* curstack, BTitemIndex* val, BaseTaskInfo* context);
		virtual uBOOL NeedUpdate(FicheInMem *rec, BaseTaskInfo* context);
		virtual uBOOL Egal(IndexInfo* autre);
		virtual VError GenereIndex(BaseTaskInfo* context, VDB4DProgressIndicator* InProgress);
		virtual sLONG GetMaxNB(BaseTaskInfo* context);
		inline uBOOL MatchField(sLONG nf, sLONG nc) { return( crit == nil || fic == nil ? false : (fic->GetNum()==nf) && (crit->GetPosInRec()==nc) ); }
		virtual Boolean MatchIndex(FieldsArray* fields, sLONG NbFields);

		virtual void GetDebugString(VString& s, sLONG numinstance);
		virtual void IdentifyIndex(VString& outString, Boolean WithTableName, Boolean WithIndexName, bool simple) const;

		// from IBaggable
		virtual VError	LoadFromBagWithLoader( const VValueBag& inBag, VBagLoader *inLoader, void* inContext = nil);
		virtual VError	SaveToBag( VValueBag& ioBag, VString& outKind) const;
		
		inline Table* GetTable() { return fic; };
		inline Field* GetField() { return crit; };

		virtual sLONG CalculateDefaultSizeKey();
		virtual sLONG CalulateKeyLength(const BTitemIndex* u);
		virtual sLONG CalulateDataPtrKeyLength(void* data);
		virtual sLONG CalulateFullKeyLengthInMem(const BTitemIndex* u);
		virtual sLONG CompareKeys(const BTitemIndex *val1, const BTitemIndex *val2, const VCompareOptions& inOptions);
		virtual BTitemIndex* BuildKey(FicheInMem *rec, VError &err, Boolean OldOne = false);
		virtual BTitemIndex* BuildKeyFromVValue(const ValPtr cv, VError &err);
		virtual ValPtr CreateVValueWithKey(const BTitemIndex* key, VError &err);
		virtual void SwapByteKeyData(BTitemIndex* key);

		//VError ThrowError( VError inErrCode, ActionDB4D inAction);
		Boolean TryToBuildIndexLong(VError& err, sLONG MaxElemsInPage, sLONG RequiredElemsInPage, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, sLONG typ);
		Boolean TryToBuildIndexTime(VError& err, sLONG MaxElemsInPage, sLONG RequiredElemsInPage, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, sLONG typ);
		Boolean TryToBuildIndexLong8(VError& err, sLONG MaxElemsInPage, sLONG RequiredElemsInPage, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, sLONG typ);
		Boolean TryToBuildIndexShort(VError& err, sLONG MaxElemsInPage, sLONG RequiredElemsInPage, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, sLONG typ);
		Boolean TryToBuildIndexBoolean(VError& err, sLONG MaxElemsInPage, sLONG RequiredElemsInPage, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, sLONG typ);
		Boolean TryToBuildIndexByte(VError& err, sLONG MaxElemsInPage, sLONG RequiredElemsInPage, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, sLONG typ);
		Boolean TryToBuildIndexReal(VError& err, sLONG MaxElemsInPage, sLONG RequiredElemsInPage, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, sLONG typ);
		Boolean TryToBuildIndexUUID(VError& err, sLONG MaxElemsInPage, sLONG RequiredElemsInPage, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, sLONG typ);
		Boolean TryToBuildIndexAlpha(VError& err, Field* cri, sLONG MaxElemsInPage, sLONG RequiredElemsInPage, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, sLONG typ);
		Boolean TryToBuildIndexAlphaUTF8(VError& err, Field* cri, sLONG MaxElemsInPage, sLONG RequiredElemsInPage, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, sLONG typ);

		virtual Boolean GenereQuickIndex(VError &err, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, sLONG MaxElemsInPage, sLONG RequiredElemsInPage);

		virtual VError GetDistinctValues(DB4DCollectionManager& outCollection, BaseTaskInfo* context, Bittab* filtre, 
											VDB4DProgressIndicator* InProgress, const VCompareOptions& inOptions);

		virtual VError QuickGetDistinctValues(QuickDB4DArrayOfValues *outCollection, BaseTaskInfo* context, Bittab* filtre, 
											VDB4DProgressIndicator* InProgress, const VCompareOptions& inOptions);

		virtual VError CalculateColumnFormulas(ColumnFormulas* formules, BaseTaskInfo* context, Bittab* filtre, VDB4DProgressIndicator* InProgress);

		virtual VError CalculateMin(BaseTaskInfo* context, Bittab* filtre, VDB4DProgressIndicator* InProgress, VValueSingle* &result);

		virtual VError CalculateMax(BaseTaskInfo* context, Bittab* filtre, VDB4DProgressIndicator* InProgress, VValueSingle* &result);

//		virtual VArrayStruct* GetSourceLang();

		virtual Table* GetTargetTable();
		virtual Field* GetTargetField()
		{
			return crit;
		}

		virtual void CheckIfIsDelayed();
		virtual void DelayIndex();
		virtual void SetDelayIndex(sLONG inAlreadyDelayRequestCount);
		virtual void AwakeIndex(VDB4DProgressIndicator* inProgress);

		virtual bool IsOnPrimKey();


		virtual void CreateHeader();

		virtual VError ValidateIndexInfo();

		virtual VError ClearFromSystemTableIndexCols();
		virtual VError AddToSystemTableIndexCols();

		virtual Boolean LockTargetTables(BaseTaskInfo* context);
		virtual void UnLockTargetTables(BaseTaskInfo* context);

		virtual Boolean NeedToRebuildIndex(VIntlMgr* NewIntlMgr, bool stringAlgoHasChanged, bool keywordAlgoHasChanged);

		inline Field* GetTargetField() const { return crit; };

		inline sLONG GetDataKind() const { return fDataKind; };

		virtual VError JoinWithOtherIndex(IndexInfo* other, Bittab* filtre1, Bittab* filtre2,ComplexSelection* result, BaseTaskInfo* context, 
											VCompareOptions& inOptions, VProgressIndicator* inProgress = nil, bool leftjoin = nil, bool rightjoin = nil);

		virtual VError JoinWithOtherIndexNotNull(IndexInfo* other, Bittab* filtre1, Bittab* filtre2, Bittab* result, BaseTaskInfo* context, VCompareOptions& inOptions, VProgressIndicator* inProgress = nil);

		virtual Boolean SourceIsABlob() const;

		virtual bool MustBeRebuild();

#if debugLeaksAll
		virtual void GetDebugInfo(VString& outText) const;
#endif

	protected:
		virtual ~IndexInfoFromField();
		//IndexInfoFromFieldOnDisk IHD;
		Table *fic;
		Field *crit;
		sLONG fDataKind;
};

IndexInfo* CreIndexInfoFromField(void);


									/* -----------------------------------------------  */

class IndexInfoFromFieldLexico : public IndexInfoFromField
{
	public:
		inline IndexInfoFromFieldLexico(void) { InfoTyp=DB4D_Index_OnKeyWords; crit = nil; fic = nil; };
		IndexInfoFromFieldLexico(Base4D* db, sLONG xnumfile, sLONG xnumfield, sLONG xtypindex):IndexInfoFromField(db, xnumfile, xnumfield, xtypindex, false, false)
		{ 
			InfoTyp=DB4D_Index_OnKeyWords;
			if (crit != nil)
				fDataKind = crit->GetTypeForLexicalIndex(); 
			else
				fDataKind = -1;
		};

		virtual uBOOL MayBeSorted(void);

		virtual BTitemIndex* CreateIndexKey(const void* p);

		virtual VError PlaceCle(OccupableStack* curstack, FicheInMem *rec, BaseTaskInfo* context);
		virtual VError DetruireCle(OccupableStack* curstack, FicheInMem *rec, BaseTaskInfo* context);

		virtual void GetDebugString(VString& s, sLONG numinstance);
		virtual void IdentifyIndex(VString& outString, Boolean WithTableName, Boolean WithIndexName, bool simple) const;

		virtual VError GenereIndex(BaseTaskInfo* context, VDB4DProgressIndicator* InProgress);
		virtual Boolean GenereQuickIndex(VError &err, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, sLONG MaxElemsInPage, sLONG RequiredElemsInPage);

		virtual sLONG CalculateDefaultSizeKey();
		virtual sLONG CalulateKeyLength(const BTitemIndex* u);
		virtual sLONG CalulateDataPtrKeyLength(void* data);
		virtual sLONG CalulateFullKeyLengthInMem(const BTitemIndex* u);
		virtual sLONG CompareKeys(const BTitemIndex *val1, const BTitemIndex *val2, const VCompareOptions& inOptions);
		virtual BTitemIndex* BuildKey(FicheInMem *rec, VError &err, Boolean OldOne = false);
		virtual BTitemIndex* BuildKeyFromVValue(const ValPtr cv, VError &err);
		virtual ValPtr CreateVValueWithKey(const BTitemIndex* key, VError &err);
		virtual void SwapByteKeyData(BTitemIndex* key);

		virtual VError ValidateIndexInfo();

		virtual Boolean NeedToRebuildIndex(VIntlMgr* NewIntlMgr, bool stringAlgoHasChanged, bool keywordAlgoHasChanged)
		{
			return stringAlgoHasChanged || keywordAlgoHasChanged;
		};

		virtual bool MustBeRebuild();

};

IndexInfo* CreIndexInfoFromFieldLexico(void);


								/* -----------------------------------------------  */



class IndexInfoFromMultipleField : public IndexInfo
{
typedef IndexInfo inherited;
	public:
		inline IndexInfoFromMultipleField(void) { InfoTyp=DB4D_Index_OnMultipleFields; fields = nil; frefs = nil; };
		IndexInfoFromMultipleField(Base4D* db, FieldNuplet* from, sLONG xtypindex, Boolean UniqueKeys);
		//virtual sLONG GetLen(void);
		virtual VError PutInto(VStream& buf, Base4D* xdb, Boolean WithHeader = true);
		virtual VError GetFrom(VStream& buf, Base4D* xdb, Boolean WithHeader = true, Boolean oldformat = false);
		virtual BTitemIndex* CreateIndexKey(const void* p);
		virtual void CalculDependence(void);
		virtual void DeleteFromDependencies(void);
		virtual void TouchDependencies();
		virtual VError PlaceCle(OccupableStack* curstack, FicheInMem *rec, BaseTaskInfo* context);
		virtual VError DetruireCle(OccupableStack* curstack, FicheInMem *rec, BaseTaskInfo* context);
		virtual VError PlaceCleForTrans(OccupableStack* curstack, BTitemIndex* val, BaseTaskInfo* context);
		virtual VError DetruireCleForTrans(OccupableStack* curstack, BTitemIndex* val, BaseTaskInfo* context);
		virtual uBOOL NeedUpdate(FicheInMem *rec, BaseTaskInfo* context);
		virtual uBOOL Egal(IndexInfo* autre);
		virtual VError GenereIndex(BaseTaskInfo* context, VDB4DProgressIndicator* InProgress);
		virtual sLONG GetMaxNB(BaseTaskInfo* context);
		uBOOL MatchFields(IndexInfo* OtherIndex);
		uBOOL MatchFields(FieldNuplet *OtherFields);
		uBOOL MatchFields(const NumFieldArray& OtherFields, sLONG numTable);
		virtual Boolean MatchIndex(FieldsArray* fields, sLONG NbFields);

		virtual void GetDebugString(VString& s, sLONG numinstance);
		virtual void IdentifyIndex(VString& outString, Boolean WithTableName, Boolean WithIndexName, bool simple) const;

		// from IBaggable
		virtual VError	LoadFromBagWithLoader( const VValueBag& inBag, VBagLoader *inLoader, void* inContext = nil);
		virtual VError	SaveToBag( VValueBag& ioBag, VString& outKind) const;
		
		inline FieldRefArray* GetFields() { return frefs; };
		inline const FieldRefArray* GetFields() const { return frefs; };

		inline FieldNuplet* GetFieldNums() { return fields; };
		inline const FieldNuplet* GetFieldNums() const { return fields; };

		virtual sLONG CalculateDefaultSizeKey();
		virtual sLONG CalulateKeyLength(const BTitemIndex* u);
		virtual sLONG CalulateDataPtrKeyLength(void* data);
		virtual sLONG CalulateFullKeyLengthInMem(const BTitemIndex* u);
		virtual sLONG CompareKeys(const BTitemIndex *val1, const BTitemIndex *val2, const VCompareOptions& inOptions);
		virtual BTitemIndex* BuildKey(FicheInMem *rec, VError &err, Boolean OldOne = false);
		BTitemIndex* BuildKeyFromVValues(const ListOfValues& values, VError &err);
		VError BuildVValuesFromKey(const BTitemIndex* key, ListOfValues& outValues);
		virtual void SwapByteKeyData(BTitemIndex* key);
		virtual ValPtr CreateVValueWithKey(const BTitemIndex* key, VError &err);

		virtual Boolean GenereQuickIndex(VError &err, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, sLONG MaxElemsInPage, sLONG RequiredElemsInPage);
	//VError ThrowError( VError inErrCode, ActionDB4D inAction);
	//	virtual VArrayStruct* GetSourceLang();

		virtual Table* GetTargetTable();

		Boolean TryToBuildIndexMulti(VError& err, sLONG MaxElemsInPage, sLONG RequiredElemsInPage, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress);

		virtual void CheckIfIsDelayed();
		virtual void DelayIndex();
		virtual void SetDelayIndex(sLONG inAlreadyDelayRequestCount);
		virtual void AwakeIndex(VDB4DProgressIndicator* inProgress);
		virtual bool IsOnPrimKey();

		virtual void CreateHeader();

		virtual VError ValidateIndexInfo();

		virtual VError ClearFromSystemTableIndexCols();
		virtual VError AddToSystemTableIndexCols();

		virtual Boolean LockTargetTables(BaseTaskInfo* context);
		virtual void UnLockTargetTables(BaseTaskInfo* context);

		virtual Boolean NeedToRebuildIndex(VIntlMgr* NewIntlMgr, bool stringAlgoHasChanged, bool keywordAlgoHasChanged);

		virtual bool MustBeRebuild();

		virtual VError JoinWithOtherIndex(IndexInfo* other, Bittab* filtre1, Bittab* filtre2,ComplexSelection* result, BaseTaskInfo* context, 
											VCompareOptions& inOptions, VProgressIndicator* inProgress = nil, bool leftjoin = nil, bool rightjoin = nil);

#if debugLeaksAll
		virtual void GetDebugInfo(VString& outText) const;
#endif


	protected:
		virtual ~IndexInfoFromMultipleField();

		FieldNuplet *fields;
		FieldRefArray *frefs;
		vector<sLONG> fDataKinds;
		
};

IndexInfo* CreIndexInfoFromMultipleField(void);



									/* -----------------------------------------------  */



IndexInfo* CreateIndexInfo(sLONG ll);

IndexHeader* CreateIndexHeader(sLONG ll, IndexInfo *xentete, sLONG xDataKind = -1);



									/* -----------------------------------------------  */


enum { pasindexcom=0L, indexajoute, indexsupprime, indexrebuild, datafilesupprime };

class IndexAction : public VObject
{
	public:
		void SetAction(sLONG xindexcommande, Base4D *xdb, const VUUID& id, VDB4DProgressIndicator* InProgress);
		void SetAction(sLONG xindexcommande, Base4D *xdb, sLONG n, VDB4DProgressIndicator* InProgress);
		void SetAction(sLONG xindexcommande, IndexInfo* ind, VDB4DProgressIndicator* InProgress);
		//~IndexAction();

		void Dispose(Boolean ProgressAlso);
				
		sLONG indexcommande;
		IndexInfo *Ind;
		VUUID IndexID;
		sLONG num;
		Base4D *db;
		VDB4DProgressIndicator* fProgress;
		VSyncEvent* fEvent;
		uLONG8 fStamp;
		Boolean fCanBeStopped;
};


typedef list<IndexAction> IndexActionContainer;
typedef IndexActionContainer::iterator IndexActionRef;

void AddDeleteDataFile(Base4D *xdb, sLONG n, VDB4DProgressIndicator* InProgress, VSyncEvent* event);  // obsolete

void AddDeleteIndex(Base4D *xdb, const VUUID& id, VDB4DProgressIndicator* InProgress, VSyncEvent* event);

void AddRebuildIndex(IndexInfo* Ind, VDB4DProgressIndicator* InProgress, VSyncEvent* event);

void AddCreateIndex(IndexInfo* Ind, VDB4DProgressIndicator* InProgress, VSyncEvent* event);

Boolean GetNextIndexAction(IndexAction& outAction, uLONG8& startingstamp, sLONG& outRemain);

Boolean SomeIndexesPending(const Base4D* onBase);

void RemoveIndexActionsForDB(Base4D* db);

Boolean StillIndexingOn(Base4D* db);
void SetIndexEncours(IndexInfo* ind);

//Boolean IsIndexBuilding();

/*
extern IndexAction* IndexActionRoot;
extern IndexAction* IndexActionLast;

extern Obj4D GlobListIndex;
*/

									/* -----------------------------------------------  */

class VIndexInfoProvider: public XBOX::VObject, public XBOX::IProgressInfoCollector
{
public:
	VIndexInfoProvider();
	
	void SetIndexInfo(IndexInfo* inInfo){fIndexInfo=inInfo;}
	virtual	XBOX::VJSONObject*	CollectInfo(VProgressIndicator *inIndicator);

	bool					LoadMessageString(XBOX::VString& ioMessage){return false;}
private:
	VIndexInfoProvider(const VIndexInfoProvider&);

private:
	IndexInfo* fIndexInfo;
};


class IndexBuilderTask : public VTask
{
	public:
		inline IndexBuilderTask(VDBMgr* db4d):VTask(nil,512*1024L,eTaskStylePreemptive,NULL) 
		{ 
			fManager = db4d;
			progress = nil;
			startingstamp = 0;
			nbaction = 0;
			curaction = 0;

			SetKind( kDB4DTaskKind_IndexBuilder);
		};
		virtual Boolean DoRun();
		virtual void DoInit();

		virtual void DoPrepareToDie()
		{
			WakeUp();
		}

		void SleepFor(sLONG nbmilliseconds)
		{
			fDemandeDeIndex.ResetAndLock();
			fDemandeDeIndex.Wait(nbmilliseconds);
		}

		void WakeUp()
		{
			fDemandeDeIndex.Unlock();
		}


	protected:
		VProgressIndicator* progress;
		sLONG nbaction, curaction;;
		uLONG8 startingstamp;
		VDBMgr* fManager;
		mutable vxSyncEvent fDemandeDeIndex;
};

extern IndexBuilderTask* gIndexBuilder;
extern bool AllowWakeUpAfterIndexAction;


									/* -----------------------------------------------  */

typedef IndexHeader* (*CreHeader_Code)(IndexInfo *xentete);
typedef IndexInfo* (*CreIndex_Code)(void);



void InitIndex(VDBMgr* db4d);
void DeInitIndex();

extern CodeReg *Index_CodeReg;

inline void RegisterIndex(uLONG id, CreIndex_Code Code) { Index_CodeReg->Register(id,(void*)Code); };
inline CreIndex_Code FindIndex(uLONG id) { return( (CreIndex_Code)(Index_CodeReg->GetCodeFromID(id)) ); };


extern CodeReg *Header_CodeReg;

inline void RegisterHeader(uLONG id, CreHeader_Code Code) { Header_CodeReg->Register(id,(void*)Code); };
inline CreHeader_Code FindHeader(uLONG id) { return( (CreHeader_Code)(Header_CodeReg->GetCodeFromID(id)) ); };


IndexInfoFromField* CreateIndexInfoField(Base4D* owner, sLONG nf, sLONG nc, sLONG typindex, Boolean UniqueKeys, sLONG datakind);
IndexInfoFromField* CreateEmptyIndexInfoFieldScalar(sLONG datakind);


									/* -----------------------------------------------  */

class Base4D_NotOpened;
class ToolLog;

template <class Type, sLONG MaxCles> class FullCleIndex;

class Index_NonOpened : public ObjAlmostInCache, public IProblemReporterIntf
{
	public:
		Index_NonOpened(Base4D_NotOpened* owner, sLONG typ, VUUID& id, VString& name, sLONG numindex, Boolean isInStruct);
		virtual ~Index_NonOpened();
		inline sLONG GetNum() const { return fNum; };
		VError GetFields(std::vector<sLONG>& outFields);
		inline const VUUID& GetUUID() const { return fID; }
		
		VError AddField(sLONG numtable, sLONG numfield, ToolLog* log);
		VError AddField(const VUUID& inFieldID, sLONG datakind, ToolLog* log);
		inline Boolean IsSourceValid() const { return fSourceValid; };

		VError SetHeaderType(sLONG IndexHeaderType, ToolLog* log);
		inline void SetUniqueKeys(Boolean uniq) { fUniqueKeys = uniq; };

		VError ReadHeader(VStream& buf, ToolLog* log);
		inline void SetHeaderInvalid() { fHeaderIsValid = false; };

		Boolean IsOKPageNum(sLONG pagenum, sLONG numpageparent, VError& errexec, ToolLog* log);
		VError SwapByteKeyData(Boolean needswap, BTitemIndex* item, sLONG maxlen, Boolean& ok, sLONG numpage, ToolLog* log, CompareResult& comp);

		bool OpenCheckKeysProgressSession(ToolLog* log, VError *outError);
		
		VError CheckKeys(ToolLog* log);
		VError CheckPage(sLONG numpage, ToolLog* log, sLONG &curpage);

		template <class Type, sLONG MaxCles>
		VError CheckScalarKeys(ToolLog* log);

		template <class Type, sLONG MaxCles>
		VError CheckScalarPage(sLONG numpage, ToolLog* log, sLONG &curpage, FullCleIndex<Type, MaxCles>& currentKey);

		VError CheckPagesTrous(ToolLog* log);
		VError CheckTabAddr(ToolLog* log);
		VError CheckPagesAddrs(DataAddr4D ou, sLONG nbpagesmax, sLONG nbpagestocheck, sLONG pos1, sLONG pos2, ToolLog* log, sLONG mastermultiplicateur);

		VError CheckOneCluster(DataAddr4D ou, sLONG len, sLONG numclust, ToolLog* log);
		VError CheckClusterTrous(ToolLog* log);
		VError CheckAllClusters(ToolLog* log);
		VError CheckClusters(DataAddr4D ou, sLONG nbclustmax, sLONG nbclusttocheck, sLONG pos1, sLONG pos2, ToolLog* log, sLONG mastermultiplicateur);

		VError CheckAll(ToolLog* log);

		inline sLONG GetTarget() const { return fTable; };
		Boolean MatchField(sLONG fieldnum);

		void CleanTempData();

		void GetName(VString& outName) const;

		// from IProblemReporterIntf
		virtual VError ReportInvalidTabAddrAddr(ToolLog* log, sLONG selector);
		virtual VError ReportInvalidTabAddrTag(ToolLog* log, sLONG selector);
		virtual VError ReportInvalidTabAddrRead(ToolLog* log, sLONG selector);
		virtual VError ReportInvalidTabAddrChecksum(ToolLog* log, sLONG selector);

	protected:
		Base4D_NotOpened* fOwner;
		sLONG fType;
		sLONG fNum;
		VUUID fID;
		VString fName;

		sLONG fIndexHeaderType;
		sLONG fTable;
		vector<sLONG> fFields, fFieldTypes;
		vector<const VValueInfo*> fFieldInfos;
		Boolean fUniqueKeys;

		Boolean fDefIsValid;
		Boolean fSourceValid;
		Boolean fHeaderIsValid;
		Boolean fTabTrouIsValid;
		Boolean fTabTrouHasBeenChecked;
		Boolean fTabTrouClustIsValid;
		Boolean fTabTrouClustHasBeenChecked;
		Boolean fPagesMapIsValid;
		Boolean fPagesMapHasBeenChecked;
		Boolean fClustMapIsValid;
		Boolean fClustMapHasBeenChecked;
		Boolean fSomePagesAreInvalid;

		IndexHeaderDISK IHD;
		HeaderBtreeDisk HBT;
		ClusterDISK IHCLUST;

		TabAddrCache* fTempCachePageAddr;
		TabAddrCache* fTempCacheClusterAddr;

		Bittab* fPagesInMap;
		Bittab* fClustInMap;
		Bittab* fRecordsInKeys;
		Bittab* fDejaPages;

		void* fCurrentKey;
		sLONG fCurrentKeyLen;
		sLONG fCurrentKeyQui;
		Boolean fFirstKeyToCheck;
		Boolean fIsIndexInStruct;


};



#if debugIndexOverlap_strong

class ClusterSel;

typedef enum { di_none, di_pageIndex, di_cluster, di_clusterpart, di_nulls} di_type;

class di_IndexOverLap
{
	public:
		di_IndexOverLap()
		{
			fType = di_none;
			fInd = nil;
			fCluster = nil;
			fAddr = -1;
			fLen = -1;
			fPageNum = -1;
			fClusterNum = -1;
			fClusterPartNum = -1;
		}

		di_type fType;
		IndexInfo* fInd;
		ClusterSel* fCluster;
		DataAddr4D fAddr;
		sLONG fLen;
		sLONG fPageNum;
		sLONG fClusterNum;
		sLONG fClusterPartNum;

	typedef map<DataAddr4D, di_IndexOverLap> di_mapOverLap;

	static di_mapOverLap sMapOverLaps;
	static VCriticalSection sdi_mutex;

	static void AddIndexPage(IndexInfo* ind, sLONG pagenum, DataAddr4D ou, sLONG len);
	static void AddCluster(ClusterSel* cluster, sLONG clusternum, DataAddr4D ou, sLONG len);
	static void AddClusterPart(ClusterSel* cluster, sLONG clusternum, sLONG ClusterPartNum, DataAddr4D ou, sLONG len);
	static void AddNullsCluster(IndexInfo* ind, DataAddr4D ou, sLONG len);

	static void RemoveIndexPage(IndexInfo* ind, sLONG pagenum, DataAddr4D ou, sLONG len);
	static void RemoveCluster(ClusterSel* cluster, sLONG clusternum, DataAddr4D ou, sLONG len);
	static void RemoveClusterPart(ClusterSel* cluster, sLONG clusternum, sLONG ClusterPartNum, DataAddr4D ou, sLONG len);
	static void RemoveNullsCluster(IndexInfo* ind, DataAddr4D ou, sLONG len);

	static void add(di_IndexOverLap& di);
	static void remove(di_IndexOverLap& di);

	static void stopInMap();
};

#endif


#endif
