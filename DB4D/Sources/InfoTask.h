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
#ifndef __INFOTASK__
#define __INFOTASK__

#include "Sort4D.h"
#include "Rech4D.h"
#include <map>

typedef FicheInMem* FicheInMemptr;

class BaseTaskInfo;

typedef V0ArrayOf<FicheInMemPtr> FicheInMemArray;

class SortContext;
class SortElem : public ObjInCacheMemory
{
	public:
		void Activate(sLONG n, SortContext* xtri);
		void DeActivate();
		ValPtr GetNthValue(sLONG n, VError& err);

protected:
		FicheInMem *rec;
		SortContext* tri;
		Vx1ArrayOf<ValPtr, 5> vals;
		sLONG numrec;
};

typedef V0ArrayOf<ValPtr> SortCol;
typedef V1ArrayOf<SortCol*> SortColArray;

class RelationPath;

class SortContext : public ObjInCacheMemory
{
public:
	sLONG qt;
	SortTab *st;
	SortElem pivot;
	SortElem f1,f2;
	VDB4DProgressIndicator* fProgress;
	sLONG8 fNbComp;
	BaseTaskInfo *context;
	SortColArray fVals;
	Selection* fSel;
	VArrayPtrOf<RelationPath*> RelPaths;
	Boolean fTestUnicite;
	Boolean fWasNotUnique;
};



class FieldList;

class EntityModel;
class EntityAttribute;
class EntityMethod;


class JSEntityMethodReference
{
	public:

		bool operator < (const JSEntityMethodReference& other) const
		{
			bool result = false;

			if (fDataClass == other.fDataClass)
			{
				if (fMethod == other.fMethod)
				{
					if (fAttribute == other.fAttribute)
					{
						if (fEvent == other.fEvent)
						{
							if (fMethodKind == other.fMethodKind)
							{
								result = fCalculatedScript < other.fCalculatedScript;
							}
							else
								result = fMethodKind < other.fMethodKind;
						}
						else
							result = fEvent < other.fEvent;
					}
					else
						result = fAttribute < other.fAttribute;
				}
				else
					result = fMethod < other.fMethod;
			}
			else
				result = fDataClass < other.fDataClass;

			return result;
		}


		void init()
		{
			fDataClass = nil;
			fAttribute = nil;
			fMethod = nil;
			fMethodKind = emeth_none;
			fEvent = dbev_none;
			fCalculatedScript = script_attr_none;
		}

		JSEntityMethodReference(const EntityModel* inDataClass, const EntityMethod* inMethod, EntityMethodKind inMethodKind) // for dataClass, entity or entityCollection methods
		{
			init();
			fDataClass = inDataClass;
			fMethod = inMethod;
			fMethodKind = inMethodKind;
		}

		JSEntityMethodReference(const EntityModel* inDataClass, const EntityAttribute* inAttribute, script_attr inCalculatedScript) // for computed attributes scripts
		{
			init();
			fDataClass = inDataClass;
			fAttribute = inAttribute;
			fCalculatedScript = inCalculatedScript;
		}

		JSEntityMethodReference(const EntityModel* inDataClass, DBEventKind inEventKind, const EntityAttribute* inAttribute) // for dataClass or attributes events 
		{
			init();
			fDataClass = inDataClass;
			fAttribute = inAttribute;
			fEvent = inEventKind;
		}

		JSEntityMethodReference() // for methodCall glue
		{
			init();
		}

		JSEntityMethodReference(sLONG ref) // for RemoteFunctionCall glue
		{
			init();
			fMethod = (const EntityMethod*)(ref); // will not be used as a pointer
		}

	private:
		const EntityModel* fDataClass;
		const EntityAttribute* fAttribute;
		const EntityMethod* fMethod;
		DBEventKind fEvent;
		EntityMethodKind fMethodKind;
		script_attr fCalculatedScript;
};


class JSEntityMethod
{
	public:
		JSEntityMethod():fFuncObj(nil)
		{
		}

		JSEntityMethod(const VJSObject& inFuncObj):fFuncObj(inFuncObj)
		{
		}

		VJSObject fFuncObj;
};

typedef map<JSEntityMethodReference, JSEntityMethod> MapOfJSMethods;

							// -----------------------
							
class Transaction;

#if 0
class RelationInfoInContext : public ITreeable<RelationInfoInContext>
{
public:
	inline RelationInfoInContext(Boolean mustrelease, const Relation* rel) { fMustRelease = mustrelease; fRel = const_cast<Relation*>(rel); };
	//RelationInfoInContext(const Relation* rel);
	RelationInfoInContext(const Relation* rel, Boolean autoload);
	virtual ~RelationInfoInContext();

	inline Boolean IsEqualTo(RelationInfoInContext* other) const { return (sLONG8)fRel == (sLONG8)(other->fRel); };
	inline Boolean IsLessThan(RelationInfoInContext* other) const { return (sLONG8)fRel < (sLONG8)(other->fRel); };
	inline Relation* GetRel() { return fRel; };
	inline Boolean isAutoLoad() { return fAutoLoad; };
	inline void SetAutoLoad(Boolean autoload) { fAutoLoad = autoload; };

protected:
	Relation* fRel;
	Boolean fAutoLoad, fMustRelease;
};

#endif

class RefIntID
{
	public:

		inline RefIntID(const Relation* rel, const FicheInMem* rec)
		{
			fRel = rel;
			fRec = rec;
		};

		inline bool operator <(const RefIntID& other) const
		{
			if (fRel == other.fRel)
				return fRec < other.fRec;
			else
				return fRel < other.fRel;
		}

		const Relation* fRel;
		const FicheInMem* fRec;
};

typedef std::pair<DataTable*, sLONG> RecRef;

//typedef std::set<RefIntID, less<RefIntID>, cache_allocator<RefIntID> > SetOfRefIntID;
//typedef std::map<RecRef, FicheInMem*, less<RecRef>, cache_allocator<pair<const RecRef, FicheInMem*> > > RecordsInMemMap;
//typedef std::map<Table*, sLONG, less<Table*>, cache_allocator<pair<Table * const, sLONG> > > TableCounterMap;

typedef std::set<RefIntID> SetOfRefIntID;
typedef std::map<RecRef, FicheInMem*> RecordsInMemMap;
typedef std::map<Table*, sLONG> TableCounterMap;

typedef map<RecRef, sLONG> RecRefMap;


#pragma pack(push, 2)

const uWORD locker_table = 1;
const uWORD locker_field = 2;
const uWORD locker_relation = 3;
const uWORD locker_index = 4;
const uWORD locker_base = 5;

const uBYTE lock_readonly = 1;
const uBYTE lock_readwrite = 0;

class StructObjRef
{
	public:
		inline StructObjRef(const void* obj)
		{
			fStructObj = obj;
		};

		inline StructObjRef(const void* obj, uWORD lockertype, uBYTE extra, BaseTaskInfo* owner, Boolean ForReadOnly)
		{
			fStructObj = obj;
			fLockerType = lockertype;
			fExtraProp = extra;
			fOwner = owner;
			fTableOwner = nil;
			fBaseOwner = nil;
			fLockType = ForReadOnly ? 1 : 0;

		};

		inline StructObjRef(const Base4D* base, BaseTaskInfo* owner, Boolean ForReadOnly)
		{
			fStructObj = base;
			fLockerType = locker_base;
			fExtraProp = 0;
			fOwner = owner;
			fTableOwner = nil;
			fBaseOwner = nil;
			fLockType = ForReadOnly ? 1 : 0;
		};

		inline StructObjRef(const Table* table, Boolean withFields, BaseTaskInfo* owner, const Base4D* baseowner, Boolean ForReadOnly)
		{
			fStructObj = table;
			fLockerType = locker_table;
			if (withFields)
				fExtraProp = 1;
			else
				fExtraProp = 0;
			fOwner = owner;
			fBaseOwner = baseowner;
			fTableOwner = nil;
			fLockType = ForReadOnly ? 1 : 0;
		};

		inline StructObjRef(const Field* cri, BaseTaskInfo* owner, const Table* tableowner, const Base4D* baseowner, Boolean ForReadOnly)
		{
			fStructObj = cri;
			fLockerType = locker_field;
			fExtraProp = 0;
			fOwner = owner;
			fBaseOwner = baseowner;
			fTableOwner = tableowner;
			fLockType = ForReadOnly ? 1 : 0;
		};

		inline StructObjRef(const Relation* rel, BaseTaskInfo* owner, const Base4D* baseowner, Boolean ForReadOnly)
		{
			fStructObj = rel;
			fLockerType = locker_relation;
			fExtraProp = 0;
			fOwner = owner;
			fBaseOwner = baseowner;
			fTableOwner = nil;
			fLockType = ForReadOnly ? 1 : 0;
		};

		inline StructObjRef(const IndexInfo* ind, BaseTaskInfo* owner, const Base4D* baseowner, Boolean ForReadOnly)
		{
			fStructObj = ind;
			fLockerType = locker_index;
			fExtraProp = 0;
			fOwner = owner;
			fBaseOwner = baseowner;
			fTableOwner = nil;
			fLockType = ForReadOnly ? 1 : 0;
		};

		inline Boolean AlsoLockFields() const { return fExtraProp == 1; };
		inline Boolean IsForReadOnly() const { return fLockType == lock_readonly; };

		inline bool operator <(const StructObjRef& other) const
		{
			return fStructObj < other.fStructObj;
		};

		inline bool operator ==(const StructObjRef& other) const
		{
			return fStructObj == other.fStructObj;
		};

		inline BaseTaskInfo* GetOwner() const { return fOwner; };

		const void* fStructObj;
		BaseTaskInfo* fOwner;
		uWORD fLockerType;
		uBYTE fExtraProp;
		uBYTE fLockType;
		const void* fTableOwner;
		const void* fBaseOwner;
};

typedef set<StructObjRef> StructLockerCollection;
typedef map<StructObjRef, sLONG> StructLockerMap;

typedef set<BaseTaskInfo*> ContextCollection;
typedef vector<sLONG8> LogEntriesCollection;

#pragma pack(pop)
typedef void* StructObjLockPtr;
typedef map<sLONG, StructObjLockPtr> StructLockRefMap;

typedef map<VUUIDBuffer, Boolean> RelationAutoInfoMap;

class Base4DRemote;
class DB4DNetManager;
class CDB4DContext;
class VDB4DContext;


const uBYTE remote_starttrans = 1;
const uBYTE remote_rollbacktrans = 2;
const uBYTE remote_committrans = 3;

//class RemoteRecordCache;

typedef set<const EntityModel*> SetOfModels;


class DB4DJSRuntimeDelegate : public IJSRuntimeDelegate
{
	public:

		DB4DJSRuntimeDelegate(BaseTaskInfo* inOwner)
		{
			fOwner = inOwner;
		}

		virtual	VFolder* RetainScriptsFolder();

		virtual VProgressIndicator* CreateProgressIndicator( const VString& inTitle)
		{
			return nil;
		}

	protected:
		BaseTaskInfo* fOwner;
};


class RemoteEntityCollection;


class ENTITY_API RemotePage
{
	public:

		RemotePage()
		{
			fStart = 0;
			fLen = -1;
		}

		void Clear()
		{
			fStart = 0;
			fLen = -1;
			fKeys.clear();
		}

		//protected:
		RecIDType fStart;
		RecIDType fLen;
		VectorOfVString fKeys;
};


//typedef map<RemoteEntityCollection*, RemotePage> RemotePageMap;


class ENTITY_API BaseTaskInfo : /*public ObjInCacheMemory,*/ public CDB4DBaseContext
{
	public:
		BaseTaskInfo(Base4D *xbd, CUAGSession* inUserSession, VJSGlobalContext* inJSContext, CDB4DBase* owner, bool islocal = false);
		BaseTaskInfo(Base4DRemote *xbdremote, CUAGSession* inUserSession, VJSGlobalContext* inJSContext, CDB4DBase* owner, bool islocal = false);
		void xinit();
		virtual ~BaseTaskInfo();
		inline Base4D* GetBase() const { return(bd); };
		void xSetBaseToNil();

		inline bool IsLocal() const
		{
			return fIsLocal;
		}

		virtual void DoCallOnRefCount0();

		
		Transaction* GetCurrentTransactionForNeedsByte(void) const;
		Transaction* GetCurrentTransaction(void) const;
		Transaction*  StartTransaction(VError& err, sLONG WaitForLockTimer = 0);
		//VError CommitTransaction(void);
		//void RollBackTransaction(void);
		uBOOL InTransaction(void) const {return (curtrans!=nil); };
		//sLONG CurrentTransactionLevel() const;

		inline sLONG GetLockTimer() const { return fWaitForLockTimer; };
		inline void SetLockTimer(sLONG WaitForLockTimer) { fWaitForLockTimer = WaitForLockTimer; };
		inline sLONG GetCurrentLockTimer() const;
		inline void SetTimeOutForOthersPendingLock(sLONG inTimeOut)
		{
			locker->SetLockOthersTimeOut(inTimeOut);
		};
		
		LockEntity* GetLockEntity(void) { return(locker); };

		Boolean AddLockingSelection(Selection* sel);
		void FinaliseLock(Selection* sel, DB4D_Way_of_Locking HowToLock);
		Boolean RemoveLockingSelection(Selection* sel, Boolean RemoveFromList);

		FicheInMem* RetainRecAlreadyIn(DataTable* DF, sLONG n);
		VError PutRecIn(FicheInMem* fic);
		void RemoveRecFrom(DataTable* DF, sLONG n, Boolean SetRecordBackToNew = false, Boolean CanBeMissing = false);
		Boolean IsRecInContext(DataTable* DF, sLONG n);

		inline CDB4DBaseContextPtr GetEncapsuleur() const 
		{ 
			const CDB4DBaseContext* const res = this;
			return (CDB4DBaseContextPtr)res;
		};

		VError SetRelationAutoLoadNto1(const Relation* rel, DB4D_Rel_AutoLoadState state);
		Boolean IsRelationAutoLoadNto1(const Relation* rel, Boolean& unknown) const;

		VError SetRelationAutoLoad1toN(const Relation* rel, DB4D_Rel_AutoLoadState state);
		Boolean IsRelationAutoLoad1toN(const Relation* rel, Boolean& unknown) const;

		uLONG8 GetID() const { return fID; };

		VError WriteLog(DB4D_LogAction action, bool SignificantAction);

		void ExcludeTableFromAutoRelationDestination(Table* inTableToExclude);
		void IncludeBackTableToAutoRelationDestination(Table* inTableToInclude);
		Boolean IsTableExcludedFromAutoRelationDestination(Table* inTableToCheck) const;

		virtual void SetAllRelationsToAutomatic(Boolean RelationsNto1, Boolean ForceAuto) 
		{
			if (RelationsNto1)
			{
				if (fAllRelsNto1AreAuto != ForceAuto)
					fAutoRelStamp++;
				fAllRelsNto1AreAuto = ForceAuto;
			}
			else
			{
				if (fAllRels1toNAreAuto != ForceAuto)
					fAutoRelStamp++;
				fAllRels1toNAreAuto = ForceAuto;
			}
		};

		virtual Boolean IsAllRelationsToAutomatic(Boolean RelationsNto1) 
		{ 
			return RelationsNto1 ? fAllRelsNto1AreAuto : fAllRels1toNAreAuto; 
		};

		//VError SetExtraData( const VValueBag* inExtra);
		//const VValueBag* RetainExtraData() const;

		void MustNotCheckRefInt(const Relation* rel, const FicheInMem* rec); 
		void CheckRefIntAgain(const Relation* rel, const FicheInMem* rec);
		Boolean CanCheckRefInt(const Relation* rel, const FicheInMem* rec);

		void MustNotCheckRefIntOnForeignKey(const Relation* rel, const FicheInMem* rec); 
		void CheckRefIntAgainOnForeignKey(const Relation* rel, const FicheInMem* rec);
		Boolean CanCheckRefIntOnForeignKey(const Relation* rel, const FicheInMem* rec);

		StructObjLockPtr LockDataBaseDef(const Base4D* inBase, sLONG inTimeOut = 0, Boolean ForReadOnly = false, const VValueBag **outLockerExtraData = NULL);
		StructObjLockPtr LockTableDef(const Table* inTable, Boolean inWithFields, sLONG inTimeOut = 0, Boolean ForReadOnly = false, const VValueBag **outLockerExtraData = NULL);
		StructObjLockPtr LockFieldDef(const Field* inField, sLONG inTimeOut = 0, Boolean ForReadOnly = false, const VValueBag **outLockerExtraData = NULL);
		StructObjLockPtr LockRelationDef(const Relation* inRelation, Boolean inWithRelatedFields, sLONG inTimeOut = 0, Boolean ForReadOnly = false, const VValueBag **outLockerExtraData = NULL);
		StructObjLockPtr LockIndexDef(const IndexInfo* inIndex, sLONG inTimeOut = 0, Boolean ForReadOnly = false, const VValueBag **outLockerExtraData = NULL);
		VError UnLockStructObject(StructObjLockPtr inLocker);
		
		VError	LockDataBaseDefWithBag( const Base4D* inBase, const VValueBag& inLockDefinition, sLONG& outLockRef, const VValueBag **outLockerExtraData);
		VError	LockTableDefWithBag( const Table* inTable, const VValueBag& inLockDefinition, sLONG& outLockRef, const VValueBag **outLockerExtraData);
		VError	LockFieldDefWithBag( const Field* inField, const VValueBag& inLockDefinition, sLONG& outLockRef, const VValueBag **outLockerExtraData);
		VError	LockRelationDefWithBag( const Relation* inRelation, const VValueBag& inLockDefinition, sLONG& outLockRef, const VValueBag **outLockerExtraData);
		VError	LockIndexDefWithBag( const IndexInfo* inIndex, const VValueBag& inLockDefinition, sLONG& outLockRef, const VValueBag **outLockerExtraData);
		VError	UnLockStructObject( sLONG inLockRef);

		VError AddStructLocker(const StructObjRef& obj);
		VError RemoveStructLocker(const StructObjRef& obj);

		static StructObjLockPtr TryToLock(const StructObjRef& obj, sLONG TimeOut, const VValueBag **outLockerExtraData);
		static VError TryToUnLock(const StructObjRef& obj);

		VError WriteCreationToLog();
		VError WriteDeletionToLog();

		static Boolean NeedsBytes(sLONG allocationBlockNumber, VSize needed, VSize& total);
		inline static ContextCollection& GetAndLockListContexts() 
		{ 
			sGlobIDMutex.Lock();
			return sAllContexts; 
		};

		inline static void UnlockListContexts()
		{
			sGlobIDMutex.Unlock();
		};

		VError AddLogEntryToTrans(sLONG8 inLogPos);
		VError StealLogEntries(LogEntriesCollection& outEntries);

		void InvalidateTransaction();
		static void InvalidateAllTransactions(Base4D* bd);

		VError GetLoadedRecords(DataTable* df, Bittab& outLoadedOnes, Bittab* filtre = nil);

		static void AddDeletedIndexIDInAllContexts(const VUUIDBuffer& inID);

#if debuglr == 12
		void Debug_LookForRec(FicheInMem* rec);
#endif

		// inherited from CDB4DBaseContext
		virtual VIntlMgr*	GetIntlMgr() const	{ return fIntlMgr;}
		
		//inline Boolean		IsRemote() const {return fIsRemote;}

		inline void SetRemoteConnection(DB4DNetManager* connection)
		{
			fDB4DNetManager = connection;
		}

		inline DB4DNetManager* GetRemoteConnection() const
		{
			return fDB4DNetManager;
		}

		inline void MustDescribeQuery(Boolean describe)
		{
			fMustDescribeQuery = describe;
		}

		inline Boolean ShouldDescribeQuery()
		{
			return fMustDescribeQuery;
		}

		inline void GetLastQueryDescription(VString& outResult)
		{
			VTaskLock lock(&fDescriptionMutex);
			outResult = fLastQueryDescription;
		}

		inline void SetQueryDescription(const VString& s)
		{
			VTaskLock lock(&fDescriptionMutex);
			fLastQueryDescription = s;
		}

		inline void GetLastQueryExecution(VString& outResult)
		{
			VTaskLock lock(&fDescriptionMutex);
			outResult = fLastQueryExecution;
		}

		inline void SetQueryExecution(const VString& s)
		{
			VTaskLock lock(&fDescriptionMutex);
			fLastQueryExecution = s;
		}

		inline void GetLastQueryExecutionXML(VString& outResult)
		{
			VTaskLock lock(&fDescriptionMutex);
			outResult = fLastQueryExecutionXML;
		}

		inline void SetQueryExecutionXML(const VString& s)
		{
			VTaskLock lock(&fDescriptionMutex);
			fLastQueryExecutionXML = s;
		}

		inline void SetQueryPlan(VValueBag* plan)
		{
			CopyRefCountable(&fLastQueryPlan, plan);
		}

		inline void SetQueryPath(VValueBag* path)
		{
			CopyRefCountable(&fLastQueryPath, path);
		}

		virtual VValueBag* GetLastQueryPlan() const
		{
			return fLastQueryPlan;
		}

		virtual VValueBag* GetLastQueryPath() const
		{
			return fLastQueryPath;
		}

		void StealDeletedIndexesID(vector<VUUIDBuffer>& outIDs)
		{
			VTaskLock lock(&fDescriptionMutex);
			outIDs.clear();
			fDeletedIndexIDs.swap(outIDs);
		}

		void AddDeletedIndexID(const VUUIDBuffer& inID)
		{
			VTaskLock lock(&fDescriptionMutex);
			fDeletedIndexIDs.push_back(inID);
		}

		VDB4DContext* getContextOwner() const;
		virtual CDB4DContext* GetContextOwner();

		inline void SetContextOwner(CDB4DContext* parent)
		{
			fParent = parent;
		}

		inline void StealRemoteTransactions(vector<uBYTE>& outList)
		{
			outList.clear();
			outList.swap(fRemoteTransactions);
		}

		inline Boolean IsRemoteLike() const
		{
			return fIsRemote || bd->IsRemote();
		}
		
		inline Boolean HasRemoteTransPending() const
		{
			return !fRemoteTransactions.empty();
		}

		//void SendlastRemoteInfo();

		//FicheInMem* RetainRemoteCachedRecord(Table* inTable, RecIDType recnum);

		VError PutAutoRelInfoToStream(VStream* into);
		VError GetAutoRelInfoFromStream(VStream* from);
		void AutoRelInfoWasSent();
		bool NeedsToSendAutoRelInfo() const		{ return fRemoteAutoRelStamp != fAutoRelStamp;}

		inline sLONG GetRemoteMaxrecordStamp() const
		{
			return fRemoteMaxRecordStamp;
		}

		inline void SetRemoteMaxrecordStamp(sLONG stamp)
		{
			fRemoteMaxRecordStamp = stamp;
		}

		void UnlockAllLoadedRecs(DataTable* df);

		void AddReleasedRecID(const DB4D_RecordRef& inID);
		void StealListOfReleasedRecIDs(vector<DB4D_RecordRef>& outList); // to be called on client
		bool HasSomeReleasedObjects();

		void	ClearIsCreationWrittenToLog() { fIsCreationWrittenToLog = false;}

		virtual void SetCurrentUser(const VUUID& inUserID, CUAGSession* inSession);

		void GetCurrentUserName(VString& outName);
		void GetCurrentUserID(VUUID& outID);

		virtual CUAGSession* GetCurrentUserSession()
		{
			return fUserSession;
		}

		virtual void DescribeQueryExecution(Boolean on);
		virtual Boolean ShouldDescribeQueryExecution();
		virtual void GetLastQueryDescription(XBOX::VString& outResult, DB4D_QueryDescriptionFormat inFormat);
		virtual void GetLastQueryExecution(XBOX::VString& outResult, DB4D_QueryDescriptionFormat inFormat);
		virtual void SetLastQueryExecution(XBOX::VString& inExecution, DB4D_QueryDescriptionFormat inFormat);

		virtual void SetTimerOnRecordLocking(sLONG WaitForLockTimer);
		virtual sLONG GetTimerOnRecordLocking() const;

		virtual CDB4DBase* GetOwner(void) const
		{
			return fOwner;
		}

		virtual Boolean MatchBaseInContext(CDB4DBaseContextPtr InBaseContext) const; 

		virtual VError StartTransaction(sLONG WaitForLockTimer = 0);

		virtual VError CommitTransaction();
		virtual VError RollBackTransaction();
		virtual sLONG CurrentTransactionLevel() const;

		virtual VError ReleaseFromConsistencySet(CDB4DSelection* InSel);
		virtual VError ReleaseFromConsistencySet(CDB4DSet* InSet);
		virtual VError ReleaseFromConsistencySet(RecIDType inRecordID);
		virtual VError ReleaseFromConsistencySet(CDB4DRecord* inRec);
		virtual VError SetConsistency(Boolean isOn);

		//virtual CDB4DTableContext *NewTableContext( DB4D_TableID inTableID);
		virtual CDB4DSqlQuery *NewSqlQuery(VString& request, VError& err);

		virtual VError SetRelationAutoLoadNto1(const CDB4DRelation* inRel, DB4D_Rel_AutoLoadState inAutoLoadState);
		virtual VError SetRelationAutoLoad1toN(const CDB4DRelation* inRel, DB4D_Rel_AutoLoadState inAutoLoadState);
		//	virtual VError SetRelationAutoLoad(sLONG inTableSourceID, sLONG inFieldSourceID, sLONG inTableDestID, sLONG inFieldDestID , DB4D_Rel_AutoLoadState inAutoLoadState);
		//	virtual VError SetRelationAutoLoad(const CDB4DField* inSource, const CDB4DField* inDest, DB4D_Rel_AutoLoadState inAutoLoadState);

		virtual DB4D_Rel_AutoLoadState GetRelationAutoLoadNto1State(const CDB4DRelation* inRel) const;
		virtual DB4D_Rel_AutoLoadState GetRelationAutoLoad1toNState(const CDB4DRelation* inRel) const;
		//	virtual DB4D_Rel_AutoLoadState GetRelationAutoLoadState(sLONG inTableSourceID, sLONG inFieldSourceID, sLONG inTableDestID, sLONG inFieldDestID) const;
		//	virtual DB4D_Rel_AutoLoadState GetRelationAutoLoadState(const CDB4DField* inSource, const CDB4DField* inDest) const;

		virtual Boolean IsRelationAutoLoadNto1(const CDB4DRelation* inRel) const;
		virtual Boolean IsRelationAutoLoad1toN(const CDB4DRelation* inRel) const;

		virtual void ExcludeTableFromAutoRelationDestination(CDB4DTable* inTableToExclude);
		virtual void IncludeBackTableToAutoRelationDestination(CDB4DTable* inTableToInclude);
		virtual Boolean IsTableExcludedFromAutoRelationDestination(CDB4DTable* inTableToCheck) const;

		//virtual void SetAllRelationsToAutomatic(Boolean RelationsNto1, Boolean ForceAuto);
		//virtual Boolean IsAllRelationsToAutomatic(Boolean RelationsNto1);

		// extra data are now owned by VDB4DContext
		const VValueBag* RetainExtraData() const;

		virtual LockPtr LockDataBaseDef(const CDB4DBase* inBase, sLONG inTimeOut = 0, Boolean inForReadOnly = false, const VValueBag **outLockerExtraData = NULL);
		virtual LockPtr LockTableDef(const CDB4DTable* inTable, Boolean inWithFields, sLONG inTimeOut = 0, Boolean inForReadOnly = false, const VValueBag **outLockerExtraData = NULL);
		virtual LockPtr LockFieldDef(const CDB4DField* inField, sLONG inTimeOut = 0, Boolean inForReadOnly = false, const VValueBag **outLockerExtraData = NULL);
		virtual LockPtr LockRelationDef(const CDB4DRelation* inRelation, Boolean inWithRelatedFields, sLONG inTimeOut = 0, Boolean inForReadOnly = false, const VValueBag **outLockerExtraData = NULL);
		virtual LockPtr LockIndexDef(const CDB4DIndex* inIndex, sLONG inTimeOut = 0, Boolean inForReadOnly = false, const VValueBag **outLockerExtraData = NULL);

		virtual Boolean LockMultipleTables(const vector<const CDB4DTable*>& inTables, vector<LockPtr>& outLocks, sLONG inTimeOut = 0, Boolean inForReadOnly = false);
		virtual void UnLockMultipleTables(const vector<LockPtr>& inLocks);

		virtual VErrorDB4D UnLockStructObject(LockPtr inLocker);

		virtual void SetClientRequestStreams(VStream* OutputStream, VStream* InputStream);
		virtual void SetServerRequestStreams(VStream* InputStream, VStream* OutputStream);

		virtual Boolean IsRemote() const { return fIsRemote; };

		//virtual CDB4DContext* GetContextOwner() const;

		virtual void SendlastRemoteInfo();

		virtual CDB4DRemoteRecordCache* StartCachingRemoteRecords(CDB4DSelection* inSel, RecIDType FromRecIndex, RecIDType ToRecIndex,  CDB4DFieldCacheCollection* inWhichFields, const vector<uBYTE>& inWayOfLocking);
		virtual void StopCachingRemoteRecords(CDB4DRemoteRecordCache* inCacheInfo);

		virtual VErrorDB4D SetIdleTimeOut(uLONG inMilliseconds);

		virtual void FreeAllJSFuncs();

		virtual VJSGlobalContext* GetJSContext() const
		{
			return fJSContext;
		}

		virtual void SetJSContext(VJSGlobalContext* inJSContext, bool propagateToParent = true)
		{
			fJSContext = inJSContext;	// sc 24/08/2009 no more retain on JavaScript context
			if ((fParent != nil) && propagateToParent)
				fParent->SetJSContext(inJSContext);
		}

		void MarkRecordAsPushed(DataTable*df, sLONG numrec);

		void UnMarkRecordAsPushed(DataTable*df, sLONG numrec);

		sLONG	GetServerVersion() const;
		sLONG	GetClientVersion() const;

		bool	CheckFeature( bool inServer, const Feature& inFeature);

		bool	CheckServerFeature( const Feature& inFeature)		{ return CheckFeature( true, inFeature);}
		bool	CheckClientFeature( const Feature& inFeature)		{ return CheckFeature( false, inFeature);}

		BaseTaskInfo* GetOrBuildSyncHelperContext(Base4D* base);

		void	SetPermError()
		{
			fPermErrorHappened = true;
		}

		void	ClearPermError()
		{
			fPermErrorHappened = false;
		}

		bool	PermErrorHappened() const
		{
			return fPermErrorHappened;
		}

		bool GetJSMethod(const JSEntityMethodReference& methRef, VJSObject& outObjFunc);
		void SetJSMethod(const JSEntityMethodReference& methRef, const VJSObject& inObjFunc);
		void FreeAllJSMethods();
		
		bool AlreadyCalledRestrict(const EntityModel* model);
		void ReleaseCalledRestrict(const EntityModel* model);

		virtual void CleanUpForReuse();

		//RemotePage* GetRemotePage(RemoteEntityCollection* sel);

		IRefCountable* GetExtraData(void* selector);
		void SetExtraData(void* selector, IRefCountable* data);
		void RemoveExtraData(void* selector);
		void ClearAllExtraData();

		bool GetKeepForRestTransaction() const
		{
			return fKeepForRestTransaction;
		}

		void SetKeepForRestTransaction(bool b)
		{
			fKeepForRestTransaction = b;
		}


	protected:

		typedef map<void*, IRefCountable*> DataMap;

		sLONG		_GetNextStructLockRef() const;

		VUUID fUserID;
		VUUID	fUser4DSessionID;
		CUAGSession* fUserSession;
		VJSGlobalContext* fJSContext;
		MapOfJSMethods fJSMethods;
		BaseTaskInfo* fSyncHelperContext;
		uLONG8 fID;
		Base4D *bd;
		CDB4DBase* fOwner;
		Boolean fAllRelsNto1AreAuto;
		Boolean fAllRels1toNAreAuto;
		LockEntity *locker;
		Transaction* curtrans;
		Transaction* curtransForNeedsByte;
		VChainOf<Selection> fLockingSels;
		RecordsInMemMap fMapRecs;
		SetOfRefIntID fMustNotCheck;
		SetOfRefIntID fMustNotCheckForeignKey;

		//CDB4DBaseContextPtr fEncapsuleur;
		//VBinTreeOf<RelationInfoInContext> fRelsNto1;
		//VBinTreeOf<RelationInfoInContext> fRels1toN;
		RelationAutoInfoMap fRelsNto1;
		RelationAutoInfoMap fRels1toN;

		TableCounterMap fExcludedTables;

		sLONG fWaitForLockTimer;

		StructLockerCollection fStructLockers;
		VCriticalSection fStructLockersMutex;

		StructLockRefMap fStructLockRefMap;		// used only on the server

		Boolean fMustDescribeQuery;
		Boolean fIsCreationWrittenToLog;
		Boolean fIsRemote;
		bool	fPermErrorHappened;
		bool fIsLocal;
		sLONG fRemoteTransactionLevel;
		vector<uBYTE> fRemoteTransactions;
		Base4DRemote* fRemoteBase;
		DB4DNetManager* fDB4DNetManager;
		//vector<RemoteRecordCache*> fRemoteRecordCaches;

		VString fLastQueryDescription;
		VString fLastQueryExecutionXML;
		VString fLastQueryExecution;
		VValueBag* fLastQueryPlan;
		VValueBag* fLastQueryPath;
		VCriticalSection fDescriptionMutex;

		VIntlMgr* fIntlMgr;
		vector<VUUIDBuffer> fDeletedIndexIDs;

		CDB4DContext* fParent;
		bool fRetainsParent, fKeepForRestTransaction;
		sLONG fAutoRelStamp;
		sLONG fRemoteAutoRelStamp;
		sLONG fRemoteMaxRecordStamp;

		vector<DB4D_RecordRef> fClientReleasedRecIDs;
		VCriticalSection fServerKeptRecordsMutex;

		DB4DJSRuntimeDelegate* fJSDelegate;

		RecRefMap fPushedRecordIDs;

		sLONG	fFeaturesInited;
		sLONG	fFeatures;

		SetOfModels fAlreadyCalledRestrict;

		//RemotePageMap fRemotePages;

		DataMap fDataMap;

		//mutable VCriticalSection fTransactionMutex;
		mutable VCriticalSection fcurtransForNeedsByteMutex;

		static ContextCollection sAllContexts;
		static uLONG8 sGlobID;
		static VCriticalSection sGlobIDMutex;

		static StructLockerMap sStructLockers;
		static VCriticalSection sStructLockersMutex;

};

typedef BaseTaskInfo *BaseTaskInfoPtr;

							// -----------------------

inline Transaction* GetCurrentTransaction(const BaseTaskInfo* owner) { return owner == nil ? nil : owner->GetCurrentTransaction(); };

ENTITY_API BaseTaskInfo* ConvertContext(CDB4DBaseContext *inContext);




class ObjLocker
{
	public:
		ObjLocker(CDB4DBaseContextPtr inContext, Base4D* base, ObjLocker* dejalocker = nil);

		ObjLocker(CDB4DBaseContextPtr inContext, Table* table, Boolean withfields = false, ObjLocker* dejalocker = nil);

		ObjLocker(CDB4DBaseContextPtr inContext, Field* cri, ObjLocker* dejalocker = nil);

		ObjLocker(CDB4DBaseContextPtr inContext, Relation* rel, Boolean withfields = false, ObjLocker* dejalocker = nil);

		ObjLocker(CDB4DBaseContextPtr inContext, IndexInfo* ind, ObjLocker* dejalocker = nil);

		inline ~ObjLocker()
		{
			if (fLocker != nil)
				fContext->UnLockStructObject(fLocker);
			fContext->Release("ObjLocker");
		};

		inline void xInit(CDB4DBaseContextPtr inContext, Base4D* base, ObjLocker* dejalocker = nil)
		{
			assert(fContext == NULL);

			if (inContext == nil && dejalocker != nil)
			{
				fContext = dejalocker->fContext;
				fContext->Retain("ObjLocker");
			}
			else
			{
				fContext = ConvertContext(inContext);
				if (fContext == nil)
				{
					fContext = new BaseTaskInfo(base, nil, nil, (CDB4DBase*)nil);
				}
				else
				{
					assert(fContext->GetBase() == base);
					fContext->Retain("ObjLocker");
				}
			}
		};

		inline Boolean CanWork() const
		{
			if (fContext->IsRemote())
				return true;
			else
				return (fLocker != nil);
		};

	protected:
		BaseTaskInfo* fContext;
		StructObjLockPtr fLocker;
};



#endif


