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
#ifndef __FLUSH__
#define __FLUSH__

#if FORMAC
#pragma segment Flush
#endif


class FlushEvent
{
public:
	VSyncEvent* flag;
	Boolean needmem;
	Boolean waitForAllWritten;
	VSize FreedMem;
	VSize NeededMem;
	sLONG FlushCycle;
};

typedef list<FlushEvent> FlushEventList;

class BtreeFlush;

#if VERSIONDEBUG
#define CHECKFLUSH VDBMgr::GetFlushManager()->CheckFlushInMem()
#define CHECKFLUSHOBJS VDBMgr::GetFlushManager()->CheckFlushObjs()
#else
#define CHECKFLUSH
#define CHECKFLUSHOBJS
#endif

class Transaction;

struct BTitemFlush
{
	IObjToFlush *obj;
	BtreeFlush *sousBT;
	DataAddr4D addr;
	//TransactionIndex owner;
};

const sLONG MaxPreAllocatedBtreeFlush = 120;


class FlushProgressInfo
{
public:
	sLONG curObjectNum;
	uLONG8 totbytes;
	uLONG starttime;
	uLONG lasttime;
	VString message;
	Base4D* currentBD;

};

class BtreeFlush : public ObjInCacheMemory
{
	friend class Transaction;
public:
	BtreeFlush() { init(); };
	void SetForNew( const BTitemFlush& u, BtreeFlush *sousbt);
	virtual ~BtreeFlush();
	Boolean search(uBOOL *h, BTitemFlush *v, uBOOL pourdelete=false);

	void init() { nkeys=0; tabsous[0]=nil; };

	uBOOL ViderFlush(VSize *tot, Boolean *outSomethingFlushed, FlushEventList* events, VCriticalSection* FlushEventListMutext, 
		VProgressIndicator* progress, FlushProgressInfo& fpi, DataAddrSetVector& AllSegsFullyDeletedObjects);
	void Transfert(void);
	//VError TransfertTransaction(Transaction* newtrans);
	uBOOL FindObj(IObjToFlush *obj2);
	//void DeletePage(void);
#if autocheckobj
	virtual uBOOL CheckObjInMem(void);
#endif

	sLONG CompteObj(void);

#if VERSIONDEBUG
	void CheckFlushObjs(DataAddr4D &curaddr);
	void DisplayTree(void);
	Boolean FindObjInFlush(IObjToFlush *obj2);
	Boolean FindObjInFlushByAddr(DataAddr4D addr);
	sLONG CompteAddr(DataAddr4D xaddr);
	void FindSousBT(BtreeFlush* mere, BtreeFlush* fille);
	void CheckFlushPage(void);
	void CheckDuplicatesInFlush();
	sLONG CompteObjOccurence(IObjToFlush *obj2);
#endif

	static BtreeFlush* GetNewBtreeFlush();
	static void AllocateBtreeFlushRegular();
	static void AllocateBtreeFlush();
	static void RecupereBtreeFlush(BtreeFlush* page);

protected:
	IObjToFlush* tabmem[32];
	DataAddr4D oldaddr[32];
	BtreeFlush* tabsous[33];
	sLONG nkeys;

	//static Boolean fNeedAllocateNewPage;
	//static Boolean fFirstTimeAllocateNewPage;
	//static BtreeFlush* fPreAllocated[MaxPreAllocatedBtreeFlush];
	static vector<BtreeFlush*> fPreAllocatedStack;
	static VCriticalSection fAllocateMutex;

	static vector<BtreeFlush*> fPreAllocatedStackRegular;
	static VCriticalSection fAllocateRegularMutex;
};

class VDBFlushMgr;

class BaseFlushInfo : public ObjInCacheMemory, public IRefCountable
{
	friend class VDBFlushMgr;

public:
	void Dispose(void);
	inline BaseFlushInfoIndex GetID() const { return fID; };
	inline void SetID(BaseFlushInfoIndex xID) { fID = xID; };
	inline Base4D* GetOwner() const { return bd; };
	inline uLONG GetDelayToInsert() const { return fWaitToInsertNewObject; };

	inline void DelayForFlush()
	{
		if (fWaitToInsertNewObject != 0)
		{
			fWaitToInsertNewObjectTimer.Wait(fWaitToInsertNewObject);
		}
	}

	inline void RequestForInvalideHeader()
	{
		VInterlocked::Increment(&fRequestForInvalid);
	}

	inline void EndOfRequestForInvalideHeader()
	{
		VInterlocked::Decrement(&fRequestForInvalid);
	}

protected:
	BaseFlushInfo(Base4D* xbd, VDBFlushMgr* pere);
	virtual ~BaseFlushInfo();

	BaseFlushInfo* left;
	BaseFlushInfo* right;

	BtreeFlush *fCurTree;	// arbre principal
	BtreeFlush *fFlushingTree; // arbre en cours de flush
	Base4D* bd;

	mutable VCriticalSection TreeMutex;
	mutable VCriticalSection TransfertMutex;
	uLONG fWaitToInsertNewObject;
	uLONG fLastTimeIncrease;
	sLONG fNbModifStamp;
	sLONG fRequestForInvalid;
	mutable vxSyncEvent fWaitToInsertNewObjectTimer;
	Boolean doisdelete;
	Boolean isflushing;
	Boolean fOutOfList;
	VDBFlushMgr* parent;

	BaseFlushInfoIndex fID;

};


class VDBFlushTask;

typedef set<BaseFlushInfo*> SetOfBaseFlushInfo;

class VDBFlushMgr : public VObject
{
	friend class BaseFlushInfo;

public:
	static VDBFlushMgr *NewFlushManager( VDBMgr *inManager);
	virtual ~VDBFlushMgr();

	void SetDirty( Boolean inIsDirty = true);
	Boolean GetDirty();

	Boolean IsTimeToFlush();
	Boolean IsAskingFlush() {
		VTaskLock lock(&DirtyMutext);
		return fIsAskingFlush;
	}
	Boolean IsFlushing() {return fIsFlushing;}

	void Flush(Base4D* target, Boolean inSynchronous);
	void Flush( Boolean inSynchronous, Boolean onlyForAllWritten = false);
	Boolean NeedsBytes( VSize inNeededBytes);

	void SetFlushPeriod( sLONG inPeriod) {fFlushPeriod = inPeriod;}

	void PutObject(IObjToFlush *inObject, Boolean inSetCacheDirty = false, Boolean inForDelete = false, Boolean NoWait = false);
	void RemoveObject(IObjToFlush *inObject );

	BaseFlushInfo* NewBaseFlushInfo(Base4D* bd);
	void DeleteBaseFlushInfo(BaseFlushInfo* info);
	void xDeleteBaseFlushInfo(BaseFlushInfo* info);

#if VERSIONDEBUG
	void CheckFlushObjs();
	void CheckFlushInMem();
	Boolean FindObjInFlush( IObjToFlush *obj);
	Boolean FindObjInFlushByAddr(DataAddr4D addr);
	void FindSousBT( BtreeFlush* mere, BtreeFlush* fille);
	void TestFlushAddr( DataAddr4D xaddr);
	void CheckDuplicatesInFlush();
	void CheckDuplicatesInFlushEnCours();
	sLONG CompteObjOccurence( IObjToFlush *obj);
	void CheckIfObjectAfterValidate(IObjToFlush *inObject);
#endif

	// BtreeFlush *xFlushBegin();
	void xDisposeFlushingTree(BaseFlushInfo *info, Boolean inWithTransfert);
	void xSetFlushTaskReady( Boolean inIsReady) {fIsFlushTaskReady = inIsReady;}
	void xFlushEnd();
	void MainLoop();

	sLONG xGetWaitingClients() {return fNbWaitingClients;}
	//void xSetFlushCancelled(Boolean inIsCancelled) {fIsFlushCancelled = inIsCancelled;}
	//Boolean xIsFlushCancelled() {return fIsFlushCancelled;}
	//void xSetFlushedBytes( sLONG inFlushedBytes) {fFlushedBytes = inFlushedBytes;}
	Boolean SomethingToFlush(void);

	static BaseFlushInfo* GetFlushInfo(BaseFlushInfoIndex n);
	static void FreeFlushInfoID(BaseFlushInfoIndex n);

	VSyncEvent* NewFlushEnoughMemEvent(VSize inNeededBytes);
	VSyncEvent* NewFlushEndEvent();
	VSyncEvent* NewFlushAllWrittenEvent();
	
	void SleepFor(sLONG nbmilliseconds);
	void WakeUp();

	inline VProgressIndicator* GetProgress() const { return fFlushProgress; };

private:
	VDBFlushMgr( VDBMgr *inManager);
	Boolean Init();

	VDBMgr	*fManager;
	mutable VCriticalSection DirtyMutext; // protege fIsDirty
	mutable VCriticalSection ListFlushInfoMutext; // protege ListFlushInfo

	Boolean fIsDirty;
	Boolean fIsFlushTaskReady;
	Boolean fIsAskingFlush;
	Boolean fIsFlushing;
	//Boolean fIsFlushCancelled; // there was nothing to flush in respond to NeedsBytes
#if VERSIONDEBUG
	Boolean fCheckFlush;
#endif

	sLONG	fLastFlushTime;
	sLONG	fFlushPeriod;
	sLONG	fCountFlushes;
	sLONG	fNbWaitingClients; // nbr d'appels a NeedsBytes en attente
	//sLONG	fFlushedBytes;
	//sLONG	fNeededBytes;

	VDBFlushTask *fFlushTask;
	BaseFlushInfo *ListFlushInfo;

	/*
	VSyncEvent WaitForFlushToEnd;
	VSyncEvent WaitForFlushToStart;
	VSyncEvent WaitForFlushToFreeEnoughMem;
	*/
	VCriticalSection StartFlushMutex;
	sLONG StartFlushStamp;
	/*
	Boolean WaitForFlushToFreeEnoughMemMustBeUnlocked;
	Boolean WaitForFlushToStartMustBeUnlocked;
	Boolean WaitForFlushToEndMustBeUnlocked;
	*/

	VCriticalSection FlushEventMutex;
	FlushEventList events;
	static VArrayWithHolesOf<BaseFlushInfo*> sTableFlushinfo;

	sLONG fCurrentFlushCycle;

	SetOfBaseFlushInfo fNeedValidate;

	mutable vxSyncEvent fDemandeDeFlush;

	VProgressIndicator* fFlushProgress;

};


class VDBFlushTask : public VTask
{
public:
	VDBFlushTask( VDBFlushMgr *inFlushMgr):VTask(inFlushMgr,512*1024L,eTaskStylePreemptive,NULL),fFlushMgr(inFlushMgr)
	{
		SetKind( kDB4DTaskKind_FlushCache);
	}

protected:
	virtual void DoInit();
	virtual void DoDeInit();
	virtual Boolean DoRun();
	virtual void DoPrepareToDie();


	VDBFlushMgr *fFlushMgr;
	sLONG timeupdate;
};


#if debuglr
extern uBOOL debug_tools_started;
#endif

#endif