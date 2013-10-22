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
#ifndef __FICHE__
#define __FICHE__


typedef Vx1ArrayOf<ChampHeader, 20> ChampHeaderArray;


class Table;


class DataTable;

//enum { f_Free = 0 , f_LectureSeule, f_LectureEcriture, f_LectureEcriture_LockedInSel, f_MustFreeBitLockOnly };

class FicheInMem;
class RecTempHeader;


class FicheInMem;
class FicheOnDisk;

#if debugFicheOnDisk

class FicheRetainerDebug
{
public:
	typedef set<FicheInMem*> FicheSet;

	void CheckAssocData(FicheOnDisk* ficD);

	void AddFiche(FicheInMem* rec);

	void RemoveFiche(FicheInMem* rec);

protected:
	FicheSet fRecords;
	VCriticalSection fMutex;

};


extern FicheRetainerDebug gDebugFiches;

#endif



class FicheOnDisk : public ObjInCacheMem, public IObjToFlush
{
	public:
		inline FicheOnDisk(void) { fQueryValuesMutex = 0;};

		virtual bool SaveObj(VSize& outSizeSaved);

		FicheOnDisk(DataTable* owner, const RecTempHeader& From);
		void PutHeaderInto(RecTempHeader& into);

		FicheOnDisk(const FicheOnDisk* from, uBOOL pourdouble=true); // appelee par DuplicateRecord
		static FicheOnDisk* DuplicateRecord(const FicheOnDisk* from, uBOOL pourdouble);

		FicheOnDisk(DataTable *xDF, sLONG xnumfic, DataAddr4D xaddr, RecordHeader& tag, bool allowForbiddenAddr = false); // appelee par BuildRecordFromLoad
		FicheOnDisk(DataTable *xDF, sLONG xnumfic, void* dataptr, sLONG datalen, sLONG nbfield, Boolean needswap); // appele apres avoir charge la fiche dand BuildRecordFromLog

		static FicheOnDisk* BuildRecordFromLoad(VError& err, DataTable *xDF, sLONG xnumfic, DataAddr4D xaddr, ReadAheadBuffer* buffer = nil);
		static FicheOnDisk* BuildRecordFromLog(VError& err, DataTable *xDF, sLONG xnumfic, void* dataptr, sLONG datalen, sLONG nbfield, Boolean needswap );

		static FicheOnDisk* BuildRecordFromLoad(VError& err, sLONG DataTableNum, sLONG xnumfic, DataAddr4D xaddr, VFileDesc* f,  DataTable *xDF);

		FicheOnDisk(Base4D *xdb, DataTable *xDF, Table* crit); // pour le Create Record

		/*
		void use(void) { nbuse++; };
		void unuse(void) { assert(nbuse>0); nbuse--; };
		uBOOL isused(void) { return(nbuse>0); };
		void FreeAfterUse();

		inline sLONG GetNbUse(void) const { return(nbuse); };
		inline void SetNbUse(sLONG n) { nbuse=n; };
		*/

		tPtr GetDataPtr(sLONG n, sLONG* WhatType = nil);
		tPtr GetDataPtrForQuery(sLONG n, sLONG* WhatType = nil, bool forceNonNull = true, bool checkForStyle = true);
		sLONG calclen(void);
		inline sLONG GetAntelen(void) const { return(antelen); };
		inline void SetAntelen(sLONG l) { antelen=l; };
		inline sLONG GetNbCrit(void) const { return(/*chd.GetCount()*/fNbCrit); };
		inline sLONG getnumfic(void) const { return(numfic); };
		inline void setnumfic(sLONG l) { numfic=l; };
		inline sLONG GetTyp(sLONG n) { return(GetChampHeader(n)->typ); };
		inline Boolean IsFieldNull(sLONG n) const { return (n > fNbCrit || GetChampHeader(n)->typ == DB4D_NullType || GetChampHeader(n)->typ == DB4D_NoType); };

		
		inline FicheOnDisk* GetxOldOne(BaseTaskInfo* context) { return /*((FicheOnDisk*)GetOldOne(context))*/ this; };
		
		uLONG8 GetTimeStamp(void) const { return TimeStamp; };
		FicheOnDisk* ReajusteData(FicheInMem *owner, VError& err, Boolean MayKeepOriginal);
		sLONG GetActualFieldLen(sLONG n);
		VError ThrowError( VError inErrCode, ActionDB4D inAction) const;

		inline void* GetDataBegining() const { return (void*) &fData; };
		inline ChampHeader* GetChampHeader(sLONG numcrit) const { return ((ChampHeader*) (((char*)&fData)+fDataLen) ) + (numcrit - 1); };
		inline Base4D* GetDB() const { return DF->GetDB(); };
		inline Boolean IsNew() const { return numfic == -3; };
		inline sLONG GetChampHeaderSize() const { return fNbCrit * sizeof(ChampHeader); };
		inline sLONG ComputeSizeInMem() const { return GetChampHeaderSize() + fDataLen + sizeof(FicheOnDisk) - sizeof(void*); };
		inline sLONG GetDataLen() const { return fDataLen; };
		inline DataTable* GetOwner() const { return DF; };

		static inline size_t ComputeRecordSizeInMem(sLONG DataLen, sLONG Nbcrit) { return (Nbcrit * sizeof(ChampHeader)) + DataLen + sizeof(FicheOnDisk) - sizeof(void*); };
	
		VError WriteSaveToLog(BaseTaskInfo* context, Boolean newone);
		VError WriteDeleteToLog(BaseTaskInfo* context);

		FicheOnDisk* Clone(VError& err, Boolean ForPush = false) const;
		//VError Detach(BaseTaskInfo* Context);
		//BaseTaskInfo* WhoLockedIt(DB4D_KindOfLock& outLockType, BaseTaskInfo* context, const VValueBag **outLockingContextRetainedExtraData) const;

		void SwapBytes();

		//tools only
		inline void SetOwner( DataTable* inDF) { DF = inDF; };

		inline void SetNewInTrans(uBOOL b)
		{
			fNewInTrans = b;
		}

		inline uBOOL IsNewInTrans() const
		{
			return fNewInTrans;
		}

#if debuglr
		inline void SetCommited(uBOOL b = true)
		{
			fCommited = b;
		}

		virtual void setmodif(uBOOL xismodif, Base4D* bd, BaseTaskInfo* context)
		{
			if (fInTrans && !fCommited && xismodif)
				assert(!fInTrans);
			IObjToFlush::setmodif(xismodif, bd, context);

		}
#endif

		inline void SetInTrans(uBOOL b = true)
		{
#if debuglr
			if (fModified && b)
				assert(!fModified);
#endif
			fInTrans = b;
		}

		inline uBOOL InTrans() const
		{
			return fInTrans;
		}

		inline void SetModificationStamp(uLONG inStamp)
		{
			fModificationStamp = inStamp;
		}

		inline uLONG GetModificationStamp() const
		{
			return fModificationStamp;
		}

		
#if debugLeaksAll
		virtual bool OKToTrackDebug() const
		{
			return false;
		}

		virtual void GetDebugInfo(VString& outText) const
		{
			outText = "record : "+ToString(numfic);
		}

#endif

	protected:
		virtual ~FicheOnDisk();

		DataTable *DF;
		sLONG fDataLen;
		sLONG numfic;
		sLONG antelen;
		//sWORD nbuse;
		sWORD fNbCrit;
		uBOOL fNewInTrans, fInTrans;
#if debuglr
		uBOOL fCommited;
#endif
		uLONG fModificationStamp;
		uLONG8 TimeStamp;
		vector<void*> fQueryValues;
		SpinLockType fQueryValuesMutex;
		void* fData;
};

	
																
typedef Vx1ArrayOf<ValPtr, 20> ChampArray;

typedef vector<ValPtr> ChampVector;
typedef vector<uLONG> StampVector;

class Relation;
class Selection;

class RelationDep : public VObject
{
	public:
		inline RelationDep() { fRel = nil; fRec = nil; fSel = nil; fStamp = 0; fOldOne = false;};

		inline RelationDep(Relation* rel, FicheInMem* rec, uLONG stamp, Boolean OldOne = false) 
		{ fRel = rel; fRec = rec; fSel = nil; fStamp = stamp; fOldOne = OldOne; };

		inline RelationDep(Relation* rel, Selection* sel, uLONG stamp, Boolean OldOne = false) 
		{ fRel = rel; fRec = nil; fSel = sel; fStamp = stamp; fOldOne = OldOne; };

		inline void SetSelection(Selection* sel, uLONG stamp) { fSel = sel; fStamp = stamp; };
		inline void SetRecord(FicheInMem* rec, uLONG stamp) { fRec = rec; fStamp = stamp; };

		inline Relation* GetRelation() { return fRel; };
		Selection* GetSelection(uLONG stamp);
		FicheInMem* GetRecord(uLONG stamp);

		inline Boolean IsOld() const { return fOldOne; };

		virtual ~RelationDep();

	private:
		Relation* fRel;
		FicheInMem* fRec;
		Selection* fSel;
		uLONG fStamp;
		Boolean fOldOne;
}; 


class FieldsForCacheOnOneTable;
class FieldsForCache;
class RemoteRecordCache;
class EntityAttribute;

class FicheInMem : public VObject, public IDebugRefCountable
{
	public:
		FicheInMem(void) 
		{ 	fRecNumPreAllocated = -1; fRecNumReserved = -1; fRecParent = nil; isSaving = false; isModified = false; fToken = 0; fWasJustDeleted = false; 
				fReadOnly = true; fWasClonedForPush = 0; fIsRemote = false; fIsIDGenerated = false; fLockingContextExtraData = nil; fRemoteRecNum = -3;
				RemoteAskedForReservedRecordNumber = false; fNumFieldParent = 0; fRequestedFields = nil; fDejaRequested = false; fRequestedFieldsMaster = nil; isnew = false;
				fDisplayCacheMaster = nil;
				fModificationStamp = 0;
				fCheckAllIntegrity = true;
				fIsASubRecordOnClient = false;
				fUsedAsSubRecordOnServer = false;
				fAllAlwaysNull = false;
				fMustClearIfReloadFromContext = true;
				fAddressAllreadyTaken = false;
				fAlreadyAskedSeqNum = -2;
#if debugLeakCheck_NbLoadedRecords
				if (debug_candumpleaks)
					DumpStackCrawls();
				if (debug_canRegisterLeaks)
					RegisterStackCrawl(this);
#endif
#if debugFicheOnDisk
				gDebugFiches.AddFiche(this);
#endif

		};
		FicheInMem(BaseTaskInfo* Context, Base4D* xdb, Table* crit, VError& err);  // equivalent du initmem
		FicheInMem(BaseTaskInfo* Context, Table* crit, FicheOnDisk* ff);  // equivalent du chargehte
		virtual ~FicheInMem(); // equivalent du delmem
		// void RollBack(void);
		void UnLockRecord();
		Boolean IsFieldValid(const Field* cri, sLONG* ncrit = nil);

		void SetAssocData(FicheOnDisk* newdata);

		VError GetNthFieldBlob(sLONG n, VBlob& outBlob, Boolean CanCacheData = true);
		ValPtr GetNthField(sLONG n, VError& err, bool pictAsRaw = false, bool forQuery = false);
		//ValPtr GetNthFieldForModif(sLONG n);
		//ValPtr GetNthFieldNew(sLONG n);
		ValPtr GetNthFieldOld(sLONG n, VError& err, bool forQuery = false);

		ValPtr GetFieldValue(const Field* cri, VError& err, bool pictAsRaw = false, bool forQuery = false);
		ValPtr GetFieldOldValue(const Field* cri, VError& err, bool forQuery = false);

		ValPtr GetFieldValue(const EntityAttribute* att, VError& err, bool touch = false);

		uLONG GetFieldModificationStamp( sLONG inFieldID);
		uLONG GetFieldModificationStamp( const Field* inField);
		VError GetFieldModificationStamps(StampVector& outStamps);

		void Touch(sLONG inFieldID);
		void Touch(const Field* inField);

		uBOOL IsModif(const Field* cri);
		uBOOL IsModif(sLONG n);

		inline void SetID(const VUUIDBuffer& newid)
		{
			fID = newid;
			fIsIDGenerated = true;
		}

		inline void SetRemoteSeqNum(sLONG8 seqnum)
		{
			fRemoteSeqNum = seqnum;
		}

		inline tPtr GetDataPtr(sLONG n)
		{
			if (assocData == nil)
				return nil;
			else
				return assocData->GetDataPtr(n,nil);
		};

		inline tPtr GetDataPtrForQuery(sLONG n)
		{
			if (assocData == nil)
				return nil;
			else
				return assocData->GetDataPtrForQuery(n,nil);
		};

		inline FicheOnDisk* GetAssoc(void) const { return(assocData); };
		//virtual FicheInMem* CopyMem(void);
		//virtual FicheInMem* Duplique(void);
		//inline FicheInMem* GetDouble(void) { return(LeDouble); };
		virtual VError stockcrit(BaseTaskInfo* context);
		inline sLONG NBCrit(void) { return(maxnbcrit); };
		virtual VError ReajusteData(BaseTaskInfo* context);
		Boolean ReadOnlyState() const { return fReadOnly; };
		void SetReadOnlyState(Boolean state) { fReadOnly = state;};
		inline uBOOL IsNew(void) const { return(isnew); };
		DataTable* GetDF(void);
		virtual sLONG GetNum(void);

		void ClearAllDirty(void);
		Boolean IsRecordModified(void) { return(isnew || isModified ); };
		//sLONG GetTyp(sLONG n) { if ((n>0) && (n<=assocData->GetNbCrit())) return (assocData->GetTyp(n)); else return(DB4D_NoType); };
		
		BaseTaskInfo* GetContext(void) { return context; };

		void ClearContext()
		{
			context = nil;
		}
		
		sLONG GetMaxCV() { return (sLONG)fch.size(); };
		
		VError ThrowError( VError inErrCode, ActionDB4D inAction) const;
		inline Table* GetOwner() const { return asscrit; };

		FicheInMem* RetainCachedRelatedRecord(Relation* rel, Boolean OldOne = false);
		Selection* RetainCachedRelatedSelection(Relation* rel, Boolean OldOne = false);

		void SetCachedRelatedRecord(Relation* rel, FicheInMem* rec, Boolean OldOne = false);
		void SetCachedRelatedSelection(Relation* rel, Selection* sel, Boolean OldOne = false);

		Boolean SetSaving();
		void ClearSaving();

#if debuglr == 111
		virtual	sLONG		Retain(const char* DebugInfo = 0) const;
		virtual	sLONG		Release(const char* DebugInfo = 0) const;
#endif

		void ValidateAutoSeqToken(BaseTaskInfo* Context);
		sLONG8 GetAutoSeqValue();

		virtual FicheInMem* CloneOnlyModifiedValues(BaseTaskInfo* Context, VError& err) const;
		virtual VError RevertModifiedValues(BaseTaskInfo* Context, FicheInMem* From);
		virtual FicheInMem* Clone(BaseTaskInfo* Context, VError& err, Boolean ForPush = false) const;
		virtual VError Detach(BaseTaskInfo* Context, Boolean BlobFieldsCanBeEmptied = false);
		virtual void ClearSeqNumToken(BaseTaskInfo* context);
		virtual	void TransferSeqNumToken( BaseTaskInfo* context, FicheInMem *inDestination);

		void WhoLockedIt(DB4D_KindOfLock& outLockType, const VValueBag **outLockingContextRetainedExtraData) const;
		VError FillAllFieldsEmpty();

		inline void MarkAsDeleted() { fWasJustDeleted = true; };

		VError SetNthField(sLONG n, ValPtr cv, Boolean AutoMaxField = true);
		VError SetNthFieldOld(sLONG n, ValPtr cv, Boolean AutoMaxField = true);

		inline void SetRecParent(FicheInMem* parent, sLONG NumFieldParent = 0) // second parameter is only used on the client  
		{
			fRecParent = parent;
			fNumFieldParent = NumFieldParent;
			//fRecParent->Retain();
		};

		inline sLONG GetRecordNumberReserved() const { return fRecNumReserved; };
		inline void ClearRecordNumberReserved() { fRecNumReserved = -1; };

		virtual VError ReservedRecordNumber(BaseTaskInfo* context);

		inline void SetBackToNew()
		{
			isnew = true;
		};

		inline void ClearNew()
		{
			isnew = false;
		};

		inline void ClearModified()
		{
			isModified = false;
		};


		virtual FicheInMem* CloneForPush(BaseTaskInfo* Context, VError& err) const;
		virtual void RestoreFromPop();

		virtual uLONG GetModificationStamp() const
		{
			return fModificationStamp;
		}

		inline void _SetModificationStamp(uLONG inStamp)
		{
			fModificationStamp = inStamp;
		}

		virtual Boolean IsSystem() const { return false; };

		inline sLONG GetRecordNumberPreAllocated() const { return fRecNumPreAllocated; };
		inline void SetRecordNumberPreAllocated(sLONG num) { fRecNumPreAllocated = num; };

		VError FromClientAsSubRecord(VStream* from, BaseTaskInfo* inContext);
		VError ToClientAsSubRecord(VStream* into, BaseTaskInfo* inContext);
		VError FromServerAsSubRecord(VStream* from, BaseTaskInfo* inContext, Table* inTable);
		VError ToServerAsSubRecord(VStream* into, BaseTaskInfo* inContext);

		VError FromClient(VStream* from, BaseTaskInfo* inContext);
		VError ToClient(VStream* into, BaseTaskInfo* inContext);
		VError FromServer(VStream* from, BaseTaskInfo* inContext, Table* inTable);
		VError ToServer(VStream* into, BaseTaskInfo* inContext);
		VError ToServerMinimal(VStream* into, BaseTaskInfo* inContext);
		VError SendModifiedFields(VStream* into, BaseTaskInfo* inContext, StampVector* oldstamps);
		VError ReceiveModifiedFields(VStream* from, BaseTaskInfo* inContext, Table* inTable);

		inline Boolean IsRemote() const { return fIsRemote; };

		void SetRemoteRecordNumber(sLONG recnum)
		{
			fRemoteRecNum = recnum;
		}

		inline VUUIDBuffer& GetID() { return fID; };
		inline const VUUIDBuffer& GetID() const { return fID; };

		inline DB4D_KindOfLock* GetKindOfLockPtr()
		{
			return &fKindOfLock;
		}

		inline const VValueBag** GetLockingContextExtraDataPtr()
		{
			return &fLockingContextExtraData;
		}

		inline void SetWhoLocked(DB4D_KindOfLock inKindOfLock, const VValueBag* inLockingContextExtraData)
		{
			fKindOfLock = inKindOfLock;
			CopyRefCountable(&fLockingContextExtraData, inLockingContextExtraData);
		}

		VError CheckNthFieldRemote(sLONG n);

		void PrepareForCacheDisplay(FieldsForCache* RequestedFieldsMaster, sLONG recnum, BaseTaskInfo* xcontext, Table* inTable, RemoteRecordCache* DisplayCacheMaster);

		void ClearCacheDisplay()
		{
			fDisplayCacheMaster = nil;
			fRequestedFieldsMaster = nil;
			ReleaseRefCountable(&fRequestedFields);
		}

		void RevertToReadOnly(BaseTaskInfo* context);

		VSize PossibleUsedDiskSize();

		void ClearModifiedFields();

		inline Boolean MustCheckIntegrity() const
		{
			return fCheckAllIntegrity;
		}

		inline void DoNotCheckIntegrity()
		{
			fCheckAllIntegrity = false;
		}


		inline bool IsASubRecordOnClient() const
		{
			return fIsASubRecordOnClient;
		}

		inline void SetAsSubRecordOnClient()
		{
			fIsASubRecordOnClient = true;
		}


		inline bool IsUsedAsSubRecordOnServer() const
		{
			return fUsedAsSubRecordOnServer;
		}

		inline void MarkAsUsedAsSubRecordOnServer()
		{
			fUsedAsSubRecordOnServer = true;
		}

		inline void UnMarkAsUsedAsSubRecordOnServer()
		{
			fUsedAsSubRecordOnServer = false;
		}

		VError SetSyncInfo(Sync_Action action, BaseTaskInfo* context, VectorOfVValue* oldkey, uBOOL wasnew);
		uLONG8 GetSyncInfo() const;

		VError GetFullSyncInfo(uLONG8& outSyncstamp, VTime& outTimestamp, Sync_Action& outAction) const;

		inline void SetAllAlwaysNull()
		{
			fAllAlwaysNull = true;
		}

		void GetOldPrimKeyValue(VectorOfVValue& outKeyval);

		inline bool MustClearIfReloadFromContext() const
		{
			return fMustClearIfReloadFromContext;
		}

		inline void DoNotClearIfReloadFromContext()
		{
			fMustClearIfReloadFromContext = false;
		}

		inline void SetAddressAllreadyTaken()
		{
			fAddressAllreadyTaken = true;
		}

		inline bool AddressAllreadyTaken() const
		{
			return fAddressAllreadyTaken;
		}


	protected:
		RelationDep* FindRelationDep(Relation* rel, Boolean OldOne);
		VError CopyAllBlobs(FicheInMem* into, BaseTaskInfo* Context, bool ForPush = false);

		mutable VUUIDBuffer fID;
		sLONG8 fRemoteSeqNum;
		sLONG maxnbcrit;
		uLONG fModificationStamp;
		ChampVector fch;
		ChampVector fchold;
		vector<ValPtr> fchForQuery;
		vector<ValPtr> fcholdForQuery;
		StampVector fFieldsStamps;
		Table* asscrit;
		FicheOnDisk* assocData;
		VLong fValueRecNum;
		//FicheInMem *LeDouble;
		uBOOL isnew;
		Boolean isSaving;
		Boolean isModified;
		Boolean fTokenIsValidated;
		Boolean fWasJustDeleted;
		BaseTaskInfo* context;
		Vx0ArrayOf<RelationDep*, 10> RelDeps;
		DB4D_AutoSeqToken fToken;
		FicheInMem* fRecParent;
		sLONG fNumFieldParent;
		sLONG fRemoteRecNum;
		sLONG fRecNumReserved;
		sLONG fRecNumPreAllocated;
		sLONG8 fAlreadyAskedSeqNum;
		mutable sLONG fWasClonedForPush;
		DB4D_KindOfLock fKindOfLock;
		const VValueBag* fLockingContextExtraData;
		FieldsForCacheOnOneTable* fRequestedFields;
		FieldsForCache* fRequestedFieldsMaster;
		RemoteRecordCache* fDisplayCacheMaster;
		Boolean fIsRemote;
		mutable Boolean fIsIDGenerated;
		Boolean fReadOnly, RemoteAskedForReservedRecordNumber;
		Boolean fDejaRequested, fCheckAllIntegrity;
		bool fIsASubRecordOnClient, fUsedAsSubRecordOnServer;
		bool fAllAlwaysNull, fMustClearIfReloadFromContext;
		bool fAddressAllreadyTaken;
};

typedef FicheInMem* FicheInMemPtr;


															/* ------------------------------------------------ */



class FicheInMemSystem : public FicheInMem
{
	public:
		FicheInMemSystem(BaseTaskInfo* Context, Base4D* xdb, Table* crit, VError& err);  // equivalent du initmem
		virtual ~FicheInMemSystem();

		inline void SetNumRec(sLONG num) { fNumRec = num; };

		//virtual FicheInMem* CopyMem(void);
		//virtual FicheInMem* Duplique(void);
		virtual VError stockcrit(BaseTaskInfo* context) { return VE_DB4D_TABLEISLOCKED; };
		virtual VError ReajusteData(BaseTaskInfo* context) { return VE_DB4D_TABLEISLOCKED; };
		virtual sLONG GetNum(void) { return(fNumRec); };
		virtual VError RevertModifiedValues(BaseTaskInfo* Context, FicheInMem* From) { return VE_DB4D_TABLEISLOCKED; };
		virtual FicheInMem* CloneOnlyModifiedValues(BaseTaskInfo* Context, VError& err) const { err = VE_DB4D_TABLEISLOCKED; return nil; };
		virtual FicheInMem* Clone(BaseTaskInfo* Context, VError& err, Boolean ForPush = false) const { err = VE_DB4D_TABLEISLOCKED; return nil; };
		virtual VError Detach(BaseTaskInfo* Context, Boolean BlobFieldsCanBeEmptied = false) { return VE_DB4D_TABLEISLOCKED; };
		virtual VError ReservedRecordNumber(BaseTaskInfo* context) { return VE_DB4D_TABLEISLOCKED; };
		virtual void ClearSeqNumToken(BaseTaskInfo* context) { ; };
		virtual	void TransferSeqNumToken( BaseTaskInfo* context, FicheInMem *inDestination) {}

		virtual FicheInMem* CloneForPush(BaseTaskInfo* Context, VError& err) const  { err = VE_DB4D_TABLEISLOCKED; return nil; };
		virtual void RestoreFromPop()  { ; };
		virtual uLONG GetModificationStamp() const
		{
			return 0;
		}

		virtual Boolean IsSystem() const { return true; };

	protected:
		sLONG fNumRec;
};


#endif
