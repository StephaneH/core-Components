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
#ifndef __TRANSAC__
#define __TRANSAC__

class DataTable;
class FicheOnDisk;
class AddrTableHeader;
class Base4D;
class IndexInfo;
class BTitemIndex;
class Transaction;






inline bool operator < (const RecordIDInTrans& v1, const RecordIDInTrans& v2) 
{ 
	sLONG num1 = abs(v1.fTableID);
	sLONG num2 = abs(v2.fTableID);

	if (num1 == num2)
	{
		return v1.fNumRec < v2.fNumRec;
	}
	else
		return num1 < num2;
}


class BaseTaskInfo;


class RecTempHeader
{
	public:
		sLONG fDataLen;
		sLONG fNumfic;
		sLONG fAntelen;
		uLONG fModificationStamp;
		sWORD fNbCrit;
		uLONG8 fTimeStamp;
		DataAddr4D fAddr;
		uBOOL fNewInTrans;

};


class BlobTempHeader
{
	public:
		DataAddr4D fAnteAddr;
		DataAddr4D fAddr;
		sLONG fType;
		sLONG fDataLen;
		sLONG fNumblob;
		sLONG fAntelen;
		uBOOL fNewInTrans;
};


class FicheInTrans
{
	public:
		inline FicheInTrans(FicheOnDisk* rec)
		{
			fAddr = -1;
			//fLen = 0;
			fRec = rec;
		};

		~FicheInTrans()
		{
			if (fRec != nil)
			{
				/*
				fRec->occupe();
				fRec->FreeAfterUse();
				*/
				fRec->Release();
			}
		}

		FicheInTrans(const FicheInTrans& inOther)
		{
			fRec = inOther.fRec;
			if (fRec != nil)
				fRec->Retain();
				//fRec->use();
			fAddr = inOther.fAddr;
			//fLen = inOther.fLen;
		}

		FicheInTrans& operator = ( const FicheInTrans& inOther)
		{
			FicheOnDisk* oldrec = fRec;
			fRec = inOther.fRec;
			if (fRec != nil)
				fRec->Retain();
				//fRec->use();
			fAddr = inOther.fAddr;
			//fLen = inOther.fLen;

			if (oldrec != nil)
			{
				oldrec->Release();
				/*
				oldrec->occupe();
				oldrec->FreeAfterUse();
				*/
			}

			return *this;
		}


		FicheOnDisk* RetainRecord(sLONG TableID, Transaction* trans);
		FicheOnDisk* GetRecord(sLONG TableID, Transaction* trans);
		VError SaveToTemp(sLONG allocationBlockNumber, Transaction* trans, sLONG& totfreed);
		VError LoadFromTemp(sLONG TableID, Transaction* trans);

		FicheOnDisk* fRec;
		DataAddr4D fAddr;
		//sLONG fLen;
};


class BlobInTrans
{
	public:
		inline BlobInTrans(Blob4D* blob)
		{
			fAddr = -1;
			//fLen = 0;
			fBlob = blob;
		};

		~BlobInTrans()
		{
			if (fBlob != nil)
			{
				fBlob->Release();
			}
		}

		BlobInTrans(const BlobInTrans& inOther)
		{
			fBlob = inOther.fBlob;
			if (fBlob != nil)
				fBlob->Retain();
			fAddr = inOther.fAddr;
			//fLen = inOther.fLen;
		}

		BlobInTrans& operator = ( const BlobInTrans& inOther)
		{
			Blob4D* oldblob = fBlob;
			fBlob = inOther.fBlob;
			if (fBlob != nil)
				fBlob->Retain();
			fAddr = inOther.fAddr;
			//fLen = inOther.fLen;

			if (oldblob != nil)
			{
				oldblob->Release();
			}

			return *this;
		}

		Blob4D* RetainBlob(sLONG TableID, Transaction* trans);
		VError SaveToTemp(sLONG allocationBlockNumber, Transaction* trans, sLONG& totfreed);
		VError LoadFromTemp(sLONG TableID, Transaction* trans);

		Blob4D* fBlob;
		DataAddr4D fAddr;
		//sLONG fLen;

};

/*
typedef map<sLONG, sLONG, less<sLONG> , cache_allocator<pair<const sLONG, sLONG> > > mapNewNum;
typedef map<RecordIDInTrans, FicheInTrans, less<RecordIDInTrans>, cache_allocator<pair<const RecordIDInTrans, FicheInTrans> > > mapRecsInTrans;
typedef map<sLONG, Bittab*, less<sLONG> , cache_allocator<pair<const sLONG, Bittab*> > > mapRecIDs;
typedef map<RecordIDInTrans, BlobInTrans, less<RecordIDInTrans> , cache_allocator<pair<const RecordIDInTrans, BlobInTrans> > > mapBlobsInTrans;
typedef map<IndexInfo*, mapIndexKeys*, less<IndexInfo*> , cache_allocator<pair<IndexInfo*const, mapIndexKeys*> > > mapIndexInfo;
typedef set<RecordIDInTrans, less<RecordIDInTrans>, cache_allocator<RecordIDInTrans> > SetRecId;
*/

class OutsideDeletedBlobRef
{
	public:
		VString fPath;
		DataTable* fDF;
};

typedef map<sLONG, sLONG> mapNewNum;
typedef map<RecordIDInTrans, FicheInTrans> mapRecsInTrans;
typedef map<sLONG, Bittab*> mapRecIDs;
typedef map<RecordIDInTrans, BlobInTrans> mapBlobsInTrans;
typedef map<IndexInfo*, /* mapIndexKeys* */void* > mapIndexInfo;
typedef map<IndexInfo*, Bittab*> mapNullsIndexInfo;
typedef set<RecordIDInTrans> SetRecId;
typedef map<DataTable*, Bittab*> mapRecIDsByDataTable;

typedef pair<sLONG, VString> BlobPathID;
typedef map<BlobPathID, BlobInTrans> mapOutsideBlobsInTrans;
typedef map<VString, OutsideDeletedBlobRef> mapBlobPaths;

typedef pair<sLONG, sLONG> recref; // numtable, numrec   or numtable, numblob
typedef pair<PrimKey*, sLONG> insideblobref; // primkey, numfield

typedef map<recref, PrimKey*> mapPrimKeys;
typedef map<recref, insideblobref> mapPrimKeysForBlobs;


const sLONG kTransNewBlobNum = 1200000000L;

typedef map<sLONG, Transaction*> TransactionCollection;

class Transaction : public ObjCache
{
friend class BtreeFlush;
	public:
		Transaction(BaseTaskInfo* owner, Transaction* Mother = nil, sLONG WaitForLockTimer = 0);
		virtual ~Transaction();


		virtual sLONG liberemem(sLONG allocationBlockNumber, sLONG combien=-1, uBOOL tout=false);

		virtual uBOOL okdel(void)
		{
			return false;
		}

		virtual uBOOL okToLiberemem()
		{
			return true;
		}

		virtual uBOOL isBigObject()
		{
			if (fBigObjectCacheStamp == VDBMgr::GetCacheManager()->GetWaitingForOtherToFreeBigObjectsStamp())
			{
				return fLastFreedMem > 0;
			}
			else
				return true;
		};

		void Placer(ObjAlmostInCache* obj);
		void Detruire(ObjAlmostInCache* obj);
		Transaction* StartSubTransaction(VError& err, sLONG WaitForLockTimer);
		VError Commit(Boolean &outTryAgain);
		VError RollBack(void);
		void DeleteAll(void); 

		VError FreeAddrTableNewEntries();
		inline sLONG GetLockTimer() const { return fWaitForLockTimer; };
		
		inline Transaction* GetMother() const { return(mere); };
		inline Transaction* GetBaseTrans() const { return(fBaseTrans); };

		inline sLONG CurrentTransactionLevel() const { return fCurrentLevel; };
		
		VError AddNewBlobNum(sLONG TableID, sLONG numblob);
		VError AddNewRecNum(sLONG TableID, sLONG numrec);
		VError DelNewRecNum(sLONG TableID, sLONG numrec);
				
		uBOOL EndOfTrans(void) { return(closing); };

		inline BaseTaskInfo* GetOwner() const { return fOwner; };
		inline Bittab* GetSavedRecordIDs(sLONG TableID, VError& err, Boolean BuildIfMissing = true) { return GetxRecordIDs(TableID, err, 0, BuildIfMissing); };
		inline Bittab* GetDeletedRecordIDs(sLONG TableID, VError& err, Boolean BuildIfMissing = true) { return GetxRecordIDs(TableID, err, 1, BuildIfMissing); };

		inline Bittab* GetDeletedBlobIDs(sLONG TableID, VError& err, Boolean BuildIfMissing = true) { return GetxRecordIDs(TableID, err, 2, BuildIfMissing); };

		VError SaveRecord(FicheOnDisk* ficD, VSize RecSize);
		FicheOnDisk* GetRecord(sLONG TableID, sLONG RecID, VError& err);
		FicheOnDisk* RetainRecord(sLONG TableID, sLONG RecID, VError& err);
		Boolean IsRecordDeleted(sLONG TableID, sLONG RecID);
		VError DelRecord(sLONG TableID, sLONG RecID, PrimKey* primkey);

		VError SaveBlob(Blob4D* blob);
		Blob4D* GetBlob(sLONG TableID, sLONG BlobID, VError& err);
		Boolean IsBlobDeleted(sLONG TableID, sLONG BlobID);
		VError DelBlob(sLONG TableID, sLONG RecID, PrimKey* primkey, sLONG fieldnum);
		VError DelOutsideBlob(Blob4D* blob);
		VError DelOldBlobPath(Blob4D* blob);

		mapIndexKeys* NewIndexKeysMap();
		void* GetDeletedKeys(IndexInfo* ind, Boolean BuildIfMissing, VError& err);
		void* GetSavedKeys(IndexInfo* ind, Boolean BuildIfMissing, VError& err);
		Bittab* GetNullKeys(IndexInfo* ind, Boolean BuildIfMissing, VError& err);
		Bittab* GetDeletedNullKeys(IndexInfo* ind, Boolean BuildIfMissing, VError& err);

		VError RecopieNullIndexes(mapNullsIndexInfo::iterator& from, mapNullsIndexInfo& into);
		VError RecopieIndexes(mapIndexInfo::iterator& from, mapIndexInfo& into);

		VError RemoveDeleteKeysFromFourche(IndexInfo* ind, const BTitemIndex* val1, uBOOL xstrict1, const BTitemIndex* val2, uBOOL xstrict2,
											const VCompareOptions& inOptions, Bittab* dejasel);

		VError Fourche(IndexInfo* ind, const BTitemIndex* val1, uBOOL xstrict1, const BTitemIndex* val2, uBOOL xstrict2, 
						const VCompareOptions& inOptions, Bittab* dejasel, Bittab* into, BTitemIndex** outVal);

		VError ParseAllIndex(IndexInfo* ind, const BTitemIndex* val1, const VCompareOptions& inOptions, Bittab* into);

		VError PlaceCle(IndexInfo* ind, BTitemIndex* val, sLONG numrec);
		VError DetruireCle(IndexInfo* ind, BTitemIndex* val, sLONG numrec);

		VError DetruireCleAll(IndexInfo* ind, void* vals);
		VError PlaceCleAll(IndexInfo* ind, void* vals);

		inline uBOOL IsValid() const { return fIsValid; };
		inline void SetValid(uBOOL state) { fIsValid = state; };

		void IncNewRecs(sLONG TableID);
		void DecNewRecs(sLONG TableID);
		sLONG GetHowManyNewRecs(sLONG TableID);

		VError CreateTempFile();

		VError PutTempData(const void* data, sLONG len, DataAddr4D ou);
		VError GetTempData(void* data, sLONG len, DataAddr4D ou);

		void FreeTempSpace(DataAddr4D ou, sLONG len);
		DataAddr4D FindTempSpace(sLONG len, VError& err);

		void NeedsBytes(sLONG allocationBlockNumber, VSize needed, VSize& total);

		sLONG GetID() const { return fID; };

		VError AddLogEntryToTrans(sLONG8 inLogPos);
		VError StealLogEntries(LogEntriesCollection& outEntries);

		static sLONG GetNewID(Transaction* trans);

		static void UnregisterTrans(sLONG id);
		static void RegisterTrans(sLONG id, Transaction* trans);

		static Transaction* GetTransFromID(sLONG id);

		void InvalidateTransaction();

		inline uLONG GetIndexStartingStamp() const 
		{ 
			return fStartingIndexesStamp; 
		};
		
		VError DoNotKeepRecordLocked(DataTable* df, sLONG recnum);
		VError KeepRecordLocked(DataTable* df, sLONG recnum);
		Boolean IsRecordLockKept(DataTable* df, sLONG recnum);
		Boolean WasLockKeptInThatTransaction(DataTable* df, sLONG recnum);

		VError TryToLockSel(DataTable* df, Bittab* b, Bittab* lockedset);
		VError UnlockSel(DataTable* df, Bittab* b, bool unlockDeleted);

		VError UnlockAllKeptLocks(bool unlockDeleted);

		Bittab* GetKeptLocks(DataTable* df, VError& err, Boolean BuildIfMissing = true);

		Boolean ConcernIndex(IndexInfo* ind);

		VSize PossibleUsedDiskSize();

		void DoNotKeepEntries()
		{
			fKeepLogEntries = false;
		}

		void KeepEntries()
		{
			fKeepLogEntries = true;
		}

	protected:
//		sLONG GetNewXNum(mapNewNum& fNums, sLONG StartNewNum, sLONG TableID);
		Bittab* GetxRecordIDs(sLONG TableID, VError& err, sLONG x, Boolean BuildIfMissing = true);

		Bittab* GetxMotherKeptLocks(DataTable* df);
		

		BaseTaskInfo* fOwner;
		Transaction* fBaseTrans;
		Transaction* mere;
		Transaction* fille;

		SetRecId fNewRecNums;
		mapNewNum fHowManyNewRecs;

		mapRecsInTrans fRecords;
		mapRecIDs fSavedRecordIDs;
		mapRecIDs fDeletedRecordIDs;
		mapPrimKeys fDeletedPrimKeys;

		mapRecIDsByDataTable fKeepLockIDs;
		mapRecIDsByDataTable fMotherKeepLockIDs;

		mapOutsideBlobsInTrans fOutsideBlobs;
		mapBlobPaths fOutsideDeletedBlobs;
		mapBlobsInTrans fBlobs;
		mapRecIDs fDeletedBlobIDs;
		SetRecId fNewBlobNums;
		mapPrimKeysForBlobs fDeletedBlobsPrimKeys;

		mapIndexInfo fSavedKeys;
		mapIndexInfo fDeletedKeys;
		mapNullsIndexInfo fNullKeys;
		mapNullsIndexInfo fDeletedNullKeys;

		sLONG fWaitForLockTimer;
		sLONG fCurrentLevel;

		uBOOL closing, fIsValid;

		VFile* fTempFile;
		VFileDesc* fTempFileDesc;
		DataAddr4D fEndOfTempFile;
		sLONG fID;
		uLONG fStartingIndexesStamp;

		LogEntriesCollection fLogEntries;

		sLONG fBigObjectCacheStamp;
		VSize fLastFreedMem;
		VSize fPossibleUsedDiskSize;
		Boolean hasfille, fBeeingDestructed;
		bool fKeepLogEntries;

		VCriticalSection fLibereMemMutex;

#if debugtrans_temp
		DataAddr4D debug_Previous_EndOfTempFile;
#endif

		static TransactionCollection sAllTrans;
		static VCriticalSection sAllTransMutex;
		static sLONG sTransID;
};

inline sLONG BaseTaskInfo::GetCurrentLockTimer() const { return curtrans == nil ? fWaitForLockTimer : curtrans->GetLockTimer(); };

inline sLONG GetCurrentLockTimer(const BaseTaskInfo* owner) { return owner == nil ? 0 : owner->GetCurrentLockTimer(); };

void InitTransactionManager(void);
void DeInitTransactionManager(void);

#define DefaultFlushTransac (TransactionIndex)-1


			/* -------------------------------------------------------------------------------------- */

#endif


