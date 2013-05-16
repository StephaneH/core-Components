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
#include "4DDBHeaders.h"


TransactionCollection Transaction::sAllTrans;
VCriticalSection Transaction::sAllTransMutex;
sLONG Transaction::sTransID = 0;



RecordIDInTrans::RecordIDInTrans(const FicheOnDisk* ficD)
{
	assert(ficD != nil);
	fNumRec = ficD->getnumfic();
	assert(fNumRec>=0);
	fTableID = ficD->GetOwner()->GetNum();
}


RecordIDInTrans::RecordIDInTrans(const Blob4D* blob)
{
	assert(blob != nil);
	fNumRec = blob->GetNum();
	assert(fNumRec>=0);
	fTableID = blob->GetDF()->GetNum();
}


		/* -------------------------- */


FicheOnDisk* FicheInTrans::RetainRecord(sLONG TableID, Transaction* trans)
{
	LoadFromTemp(TableID, trans);
	if (fRec != nil)
	{
		fRec->Retain();
	}
	return fRec;
}


FicheOnDisk* FicheInTrans::GetRecord(sLONG TableID, Transaction* trans)
{
	LoadFromTemp(TableID, trans);
	return fRec;
}


VError FicheInTrans::SaveToTemp(sLONG allocationBlockNumber, Transaction* trans, sLONG& totfreed)
{
	VError err = VE_OK;
	Boolean oktosave = false;
	totfreed = 0;
	if (fRec != nil && OKAllocationNumber(fRec,allocationBlockNumber))
	{
		if (fRec->GetRefCount() == 1)
		{
			sLONG len = fRec->calclen() + sizeof(RecTempHeader);
			if (fAddr == -1)
			{
				/*
				if (len <= fLen)
				{
					oktosave = true;
				}
				else
				*/
				{
					fAddr = trans->FindTempSpace(len, err);
				}
				if (err == VE_OK)
				{
#if debugtrans_temp
					DebugMsg(L" Fiche Save To Temp : "+ToString(fAddr)+L" , len = "+ToString(len)+L"\n");
#endif
					RecTempHeader header;
					fRec->PutHeaderInto(header);
					err = trans->PutTempData(&header, sizeof(header), fAddr);
					if (err == VE_OK)
						err = trans->PutTempData(fRec->GetDataBegining(), fRec->calclen(), fAddr + sizeof(header));
				}
			}
			if (err == VE_OK)
			{
				totfreed = len;
				//fLen = len;
				//fRec->FreeAfterUse();
				fRec->Release();
				fRec = nil;
			}
			else
			{
				fAddr = -1;
				//fLen = 0;
			}
		}
	}

	if (err != VE_OK)
		err = ThrowBaseError(VE_DB4D_CANNOT_SAVE_TRANS_RECORD, DBaction_SavingRecordIntoTemp);
	return err;
}


VError FicheInTrans::LoadFromTemp(sLONG TableID, Transaction* trans)
{
	VError err = VE_OK;
	if (fRec == nil)
	{
		if (testAssert(fAddr >= 0))
		{
			RecTempHeader header;
			err = trans->GetTempData(&header, sizeof(header), fAddr);
			if (err == VE_OK)
			{
				VSize leninmem = FicheOnDisk::ComputeRecordSizeInMem(header.fDataLen, header.fNbCrit);
				void* p = GetFastMem(leninmem, true, 'ficD');
				if (p == nil)
					err = ThrowBaseError(memfull, DBaction_LoadingRecordFromTemp);
				else
				{
					Table* tt = trans->GetOwner()->GetBase()->RetainTable(TableID);
					assert(tt != nil);
					fRec = new (p) FicheOnDisk(tt->GetDF(), header);
					tt->Release();
					err = trans->GetTempData(fRec->GetDataBegining(), fRec->calclen(), fAddr + sizeof(header));
					if (err != VE_OK)
					{
						fRec->Release();
						fRec = nil;
					}
					/*
					else
					{
						fRec->use();
						fRec->libere();
					}
					*/
				}
			}
		}
		else
			err = ThrowBaseError(VE_DB4D_TRANSACTIONISINVALID, DBaction_ExecutingTransaction);
	}

	if (err != VE_OK)
		err = ThrowBaseError(VE_DB4D_CANNOT_LOAD_TRANS_RECORD, DBaction_LoadingRecordFromTemp);

	return err;
}


		/* -------------------------- */


Blob4D* BlobInTrans::RetainBlob(sLONG TableID, Transaction* trans)
{
	LoadFromTemp(TableID, trans);
	if (fBlob != nil)
	{
		fBlob->Retain();
	}
	return fBlob;
}


VError BlobInTrans::SaveToTemp(sLONG allocationBlockNumber, Transaction* trans, sLONG& totfreed)
{
	VError err = VE_OK;
	Boolean oktosave = false;
	totfreed = 0;
	if (fBlob != nil && OKAllocationNumber(fBlob,allocationBlockNumber))
	{
		if (fBlob->GetRefCount() == 1 && !fBlob->IsOutsidePath()) // a faire les outsidepath
		{
			sLONG len = fBlob->calclen();
			if (len > 0)
				len = len - 4;
			sLONG totlen = len + sizeof(BlobTempHeader);
			if (fAddr == -1)
			{
				/*
				if (totlen <= fLen)
				{
					oktosave = true;
				}
				else
				*/
				{
					fAddr = trans->FindTempSpace(totlen, err);
				}
				if (err == VE_OK)
				{
#if debugtrans_temp
					DebugMsg(L" Blob Save To Temp : "+ToString(fAddr)+L" , len = "+ToString(len)+L"\n");
#endif
#if debugOverlaps_strong
					debug_AddBlobAddrInTrans(RecordIDInTrans(fBlob->GetDF()->GetNum(), fBlob->GetNum()), fBlob->getaddr());
#endif
					BlobTempHeader header;
					fBlob->PutHeaderInto(header);
					err = trans->PutTempData(&header, sizeof(header), fAddr);
					if (err == VE_OK && len > 0)
					{
						void* p = VObject::GetMainMemMgr()->Malloc(len, false, 'temp');
						if (p == nil)
							err = ThrowBaseError(memfull, DBaction_SavingRecordIntoTemp);
						else
						{
							err = fBlob->PutInto(p);
							err = trans->PutTempData(p, len, fAddr + sizeof(header) );
							VObject::GetMainMemMgr()->Free(p);
						}
					}
				}
			}
			else
			{
				err = err; // break here
			}
			if (err == VE_OK)
			{
				//fLen = totlen;
				totfreed = totlen;
				fBlob->Release();
				fBlob = nil;
			}
			else
			{
				fAddr = -1;
				//fLen = 0;
			}
		}
	}

	if (err != VE_OK)
		err = ThrowBaseError(VE_DB4D_CANNOT_SAVE_TRANS_BLOB, DBaction_SavingBlobIntoTemp);

	return err;
}


VError BlobInTrans::LoadFromTemp(sLONG TableID, Transaction* trans)
{
	VError err = VE_OK;
	if (fBlob == nil)
	{
		if (testAssert(fAddr >= 0))
		{
			BlobTempHeader header;
			err = trans->GetTempData(&header, sizeof(header), fAddr);
			if (err == VE_OK)
			{
				Table* tt = trans->GetOwner()->GetBase()->RetainTable(TableID);
				assert(tt != nil);
				Blob4D* blob;
				if (header.fType == kBlobTypeWithPtr)
					blob = new BlobWithPtr(tt->GetDF());
				else
					blob = new BlobText(tt->GetDF());
				sLONG len = header.fDataLen;
				if (len > 0)
					len = len - 4;
				blob->SetNewInTrans(header.fNewInTrans);
				blob->SetInTrans(true);
				blob->SetNum(header.fNumblob);
				blob->SetAnteFromTrans(header.fAnteAddr, header.fAntelen);
				blob->setaddr(header.fAddr, false);

				if (len > 0)
				{
					void* p = GetFastMem(len, false, 'temp');
					if (p == nil)
						err = ThrowBaseError(memfull, DBaction_ExecutingTransaction);
					else
					{
						err = trans->GetTempData(p, len, fAddr + sizeof(header));
						if (err != VE_OK)
						{
							blob->Release();
							fBlob = nil;
						}
						else
						{
							err = blob->GetFrom(p, len);
							if (err != VE_OK)
							{
								blob->Release();
								fBlob = nil;
							}
							else
								fBlob = blob;
						}
						FreeFastMem( p);
					}
				}
				else
					fBlob = blob;
			}
		}
		else
			err = ThrowBaseError(VE_DB4D_TRANSACTIONISINVALID, DBaction_ExecutingTransaction);
	}

	if (err != VE_OK)
		err = ThrowBaseError(VE_DB4D_CANNOT_LOAD_TRANS_BLOB, DBaction_LoadingBlobFromTemp);
	return err;
}


		/* ------------------------------------------------------------------------------------------- */
		
		

Transaction::Transaction(BaseTaskInfo* owner, Transaction *Mother, sLONG WaitForLockTimer):ObjCache(DoNotPutInCacheAcces, false)
{ 
	fOwner = owner;
	mere = Mother;

	if (mere == nil)
	{
		fID = Transaction::GetNewID(this);
		fStartingIndexesStamp = IndexInfo::GetCurIndStamp();
	}
	else
	{
		fID = mere->fID;
		fStartingIndexesStamp = mere->fStartingIndexesStamp;
		Transaction::RegisterTrans(fID, this);
	}

	if (WaitForLockTimer == -2)
	{
		if (mere == nil)
			fWaitForLockTimer = owner->GetLockTimer();
		else
			fWaitForLockTimer = mere->fWaitForLockTimer;
	}
	else
	{
		fWaitForLockTimer = WaitForLockTimer;
	}
	if (mere == nil)
	{
		fBaseTrans = this;
		fCurrentLevel = 1;
	}
	else
	{
		fBaseTrans = mere->GetBaseTrans();
		fCurrentLevel = mere->fCurrentLevel + 1;
	}

	fille = nil; 
	closing = false;
	fIsValid = true;
	fPossibleUsedDiskSize = 0;
	fKeepLogEntries = true;

	fTempFile = nil;
	fTempFileDesc = nil;
	fEndOfTempFile = 0;
#if debugtrans_temp
	debug_Previous_EndOfTempFile = 0;
#endif

	fBigObjectCacheStamp = -1;
	fLastFreedMem = 0;
	hasfille = false;

	if (mere != nil)
	{
		fBeeingDestructed = true;
		mere->hasfille = true;
	}
	else
		fBeeingDestructed = false;

	AddToCache(TransactionAccess);

}


sLONG Transaction::liberemem(sLONG allocationBlockNumber, sLONG combien, uBOOL tout)
{
	sLONG tot = 0;
	if (fLibereMemMutex.TryToLock())
	{
		if (! hasfille && !fBeeingDestructed)
		{
			VSize total = 0;
			NeedsBytes(allocationBlockNumber, (VSize)combien, total);
			tot = (sLONG)total;
		}
		fLibereMemMutex.Unlock();
	}
	fBigObjectCacheStamp = VDBMgr::GetCacheManager()->GetWaitingForOtherToFreeBigObjectsStamp();
	fLastFreedMem = tot;
	return tot;
}


inline void ClearSelInTrans(mapRecIDs::value_type& x)
{
	if (x.second != nil)
		x.second->Release();
}


inline void ClearSel2InTrans(mapRecIDsByDataTable::value_type& x)
{
	if (x.second != nil)
		x.second->Release();
}


class ClearRecInTrans
{
	public:
		inline ClearRecInTrans(BaseTaskInfo* context) { fContext = context; };

		inline void operator()(mapRecsInTrans::value_type& x) 
		{
			FicheOnDisk* ficD = x.second.fRec;
			if (ficD != nil)
			{
				sLONG n = ficD->getnumfic();
				{
					if (ficD->IsNewInTrans())
					{
						fContext->RemoveRecFrom(ficD->GetOwner(), n, true, true);
						//ficD->GetOwner()->UnlockRecord(n, fContext, false);
						//le unlock record est maintenant fait dans le corps principal du rollback : ref 16/11/2006
						ficD->setnumfic(-3);
					}
					else
					{
						if (!fContext->IsRecInContext(ficD->GetOwner(), n))
						{
							ficD->GetOwner()->UnlockRecord(n, fContext, /*DB4D_Keep_Lock_With_Transaction*/ DB4D_Keep_Lock_With_Record);
						}
						else
						{
							n = n; // put a break here for debug
						}
					}
				}

				ficD->SetNewInTrans(false);
				ficD->SetInTrans(false);
#if debuglr
				ficD->SetCommited(false);
#endif
				ficD->Release();
				/*
				ficD->occupe();
				ficD->FreeAfterUse();
				*/
				x.second.fRec = nil;
			}
			else
			{
				if (x.second.fAddr != -4)
				{
					Table* tt = fContext->GetBase()->RetainTable(x.first.fTableID);
					if (tt != nil)
					{
						sLONG n = x.first.fNumRec;
						DataTable* df = tt->GetDF();
						if (df != nil)
						{
							bool newintrans = false;
							FicheInMem* fic = fContext->RetainRecAlreadyIn(df, n);
							if (fic != nil)
							{
								newintrans = fic->GetAssoc()->IsNewInTrans();
								fic->Release();
							}
							if (newintrans)
							{
								fContext->RemoveRecFrom(df, n, true, true);
							}
							else
							{
								if (!fContext->IsRecInContext(df, n))
								{
									df->UnlockRecord(n, fContext, DB4D_Keep_Lock_With_Record);
								}
							}
							
						}
						tt->Release();
					}
				}
			}
		}

	protected:
		BaseTaskInfo* fContext;

};

inline void ClearOutsideBlobInTrans(mapOutsideBlobsInTrans::value_type& x)
{
	if (x.second.fBlob != nil)
	{
		x.second.fBlob->SetNewInTrans(false);
		x.second.fBlob->SetInTrans(false);
		x.second.fBlob->Release();
		x.second.fBlob = nil;
	}
}



inline void ClearBlobInTrans(mapBlobsInTrans::value_type& x)
{
#if debugOverlaps_strong
	if (x.second.fAddr != -2)
	{
		debug_DelBlobAddrInTrans(x.first);
	}
#endif
	Blob4D* blob = x.second.fBlob;
	if (blob != nil)
	{
		if (blob->getaddr() == 0)
		{
			assert(blob->IsNewInTrans());
			blob->SetNum(-1);
		}
		else
		{
			assert(!blob->IsNewInTrans());
		}
		blob->SetNewInTrans(false);
		blob->SetInTrans(false);
		blob->Release();
		x.second.fBlob = nil;
	}
}


inline void ClearIndexInfo(mapIndexInfo::value_type& x)
{
	const IndexInfo* ind = x.first;
	
	if (ind->IsScalar())
	{
		ind->DeleteMapKeys((void*)x.second);
	}
	else
	{
		mapIndexKeys* vals = (mapIndexKeys*)x.second;

		if (vals != nil)
			delete vals; 
	}

	// for_each(vals->begin(), vals->end(), ClearKeyInTrans);
	ind->Release(); // ind peut etre detruit par le release mais son pointer ne change pas, donc la stl::map reste coherente
}


inline void ClearNullsIndexInfo(mapNullsIndexInfo::value_type& x)
{
	const IndexInfo* ind = x.first;
	Bittab* vals = x.second;

	if (vals != nil)
		vals->Release(); 

	ind->Release(); // ind peut etre detruit par le release mais son pointer ne change pas, donc la stl::map reste coherente
}


Transaction::~Transaction()
{
	fLibereMemMutex.Lock();
	fBeeingDestructed = true;

	if (fID != 0)
		Transaction::UnregisterTrans(fID);
		
	for_each(fKeepLockIDs.begin(), fKeepLockIDs.end(), ClearSel2InTrans);
	fKeepLockIDs.clear();
	for_each(fDeletedBlobIDs.begin(), fDeletedBlobIDs.end(), ClearSelInTrans);
	for_each(fDeletedRecordIDs.begin(), fDeletedRecordIDs.end(), ClearSelInTrans);
	fDeletedRecordIDs.clear();
	for_each(fSavedRecordIDs.begin(), fSavedRecordIDs.end(), ClearSelInTrans);
	fSavedRecordIDs.clear();
	for_each(fRecords.begin(), fRecords.end(), ClearRecInTrans(fOwner));
	for_each(fBlobs.begin(), fBlobs.end(), ClearBlobInTrans);
	for_each(fOutsideBlobs.begin(), fOutsideBlobs.end(), ClearOutsideBlobInTrans);
	for_each(fSavedKeys.begin(), fSavedKeys.end(), ClearIndexInfo);
	for_each(fDeletedKeys.begin(), fDeletedKeys.end(), ClearIndexInfo);
	for_each(fNullKeys.begin(), fNullKeys.end(), ClearNullsIndexInfo);
	for_each(fDeletedNullKeys.begin(), fDeletedNullKeys.end(), ClearNullsIndexInfo);

	for_each(fMotherKeepLockIDs.begin(), fMotherKeepLockIDs.end(), ClearSel2InTrans);
	fMotherKeepLockIDs.clear();

	if (fTempFile != nil)
	{
		if (fTempFileDesc != nil)
		{
			delete fTempFileDesc;
			fTempFile->Delete();
		}
		fTempFile->Release();
	}
	if (mere != nil)
		mere->hasfille = false;

#if debug_BTItemIndex
	if (sAllTrans.empty())
	{
		if (!BTitemIndex::sdebug_BTitemIndexSet.empty())
		{
			fID = fID; // put a break here
		}
	}
	{
		VTaskLock lock(&sAllTransMutex);

		sLONG totiKeys = 0;
		for (TransactionCollection::iterator curt = sAllTrans.begin(), endt = sAllTrans.end(); curt != endt; curt++)
		{
			Transaction* curtrans = curt->second;
			for (mapIndexInfo::iterator curi = curtrans->fSavedKeys.begin(), endi = curtrans->fSavedKeys.end(); curi != endi; curi++)
			{
				IndexInfo* ind = curi->first;
				if (!ind->IsScalar())
				{
					mapIndexKeys* vals = (mapIndexKeys*)curi->second;
					if (vals != nil)
					{
						for (mapIndexKeys::const_iterator cur = vals->begin(), end = vals->end(); cur != end; cur++)
						{
							if (cur->GetKeyForDebug() != nil)
								totiKeys++;
						}
					}		
				}
			}

			for (mapIndexInfo::iterator curi = curtrans->fDeletedKeys.begin(), endi = curtrans->fDeletedKeys.end(); curi != endi; curi++)
			{
				IndexInfo* ind = curi->first;
				if (!ind->IsScalar())
				{
					mapIndexKeys* vals = (mapIndexKeys*)curi->second;
					if (vals != nil)
					{
						for (mapIndexKeys::const_iterator cur = vals->begin(), end = vals->end(); cur != end; cur++)
						{
							if (cur->GetKeyForDebug() != nil)
								totiKeys++;
						}
					}
				}
			}

		}

		if (totiKeys != BTitemIndex::sdebug_BTitemIndexSet.size())
		{
			fID = fID; // put a break here
		}
	}
#endif
	fLibereMemMutex.Unlock();
}

Transaction* Transaction::StartSubTransaction(VError& err, sLONG WaitForLockTimer)
{
	Transaction* trans;
	
	err = VE_OK;

	occupe();

	
	fille = new Transaction(fOwner, this, WaitForLockTimer);
	trans = fille;

	trans->fTempFile = fTempFile;
	if (fTempFile != nil)
		fTempFile->Retain();
	trans->fTempFileDesc = fTempFileDesc;
	trans->fEndOfTempFile = fEndOfTempFile;

	fille->fBeeingDestructed = false;

	// 1er passage qui prend les bittab servant a garder les lock dans la transaction, les merge eventuellement avec celles de la transactions mere,
	// puis les recopie dans le fMotherKeepLockIDs de la transaction fille

	for (mapRecIDsByDataTable::iterator cur = fKeepLockIDs.begin(), end = fKeepLockIDs.end(); cur != end && err == VE_OK; cur++)
	{
		mapRecIDsByDataTable::iterator grandmerekeep = fMotherKeepLockIDs.find(cur->first);
		Bittab* b = new Bittab;
		if (b != nil)
		{
			err = b->Or(cur->second);
			if (err == VE_OK && grandmerekeep != fMotherKeepLockIDs.end())
				err = b->Or(grandmerekeep->second);
			if (err == VE_OK)
			{
				try
				{
					trans->fMotherKeepLockIDs.insert(make_pair(cur->first, b));
				}
				catch (...)
				{
					err = ThrowBaseError(memfull, DBaction_ExecutingTransaction);
				}
			}
		}
		else
			err = ThrowBaseError(memfull, DBaction_ExecutingTransaction);
		if (err != VE_OK)
			ReleaseRefCountable(&b);
	}

	// 2e passage qui prend les bittab qui etait uniquement dans la transaction mere et n'avait pas ete vues dans le passage precedant
	for (mapRecIDsByDataTable::iterator cur = fMotherKeepLockIDs.begin(), end = fMotherKeepLockIDs.end(); cur != end && err == VE_OK; cur++)
	{
		mapRecIDsByDataTable::iterator dejakeep = fKeepLockIDs.find(cur->first);
		if (dejakeep == fKeepLockIDs.end())
		{
			Bittab* b = new Bittab;
			if (b != nil)
			{
				err = b->Or(cur->second);
				if (err == VE_OK)
				{
					try
					{
						trans->fMotherKeepLockIDs.insert(make_pair(cur->first, b));
					}
					catch (...)
					{
						err = ThrowBaseError(memfull, DBaction_ExecutingTransaction);
					}
				}
			}
			else
				err = ThrowBaseError(memfull, DBaction_ExecutingTransaction);
			if (err != VE_OK )
				ReleaseRefCountable(&b);
		}
	}

	// puis on garde, dans la fille, les bittab des fiches detruites dans la transaction
	for (mapRecIDs::iterator cur = fDeletedRecordIDs.begin(), end = fDeletedRecordIDs.end(); cur != end && err == VE_OK; cur++)
	{
		Bittab* b = new Bittab;
		if (b != nil)
		{
			err = b->Or(cur->second);
			if (err == VE_OK)
			{
				try
				{
					trans->fDeletedRecordIDs.insert(make_pair(cur->first, b));
				}
				catch (...)
				{
					err = ThrowBaseError(memfull, DBaction_ExecutingTransaction);
				}
			}

			//b->libere();
		}
		else
			err = ThrowBaseError(memfull, DBaction_ExecutingTransaction);
		if (err != VE_OK)
			ReleaseRefCountable(&b);
	}

	// puis on garde, dans la fille, les bittab des fiches sauvees dans la transaction
	for (mapRecIDs::iterator cur = fSavedRecordIDs.begin(), end = fSavedRecordIDs.end(); cur != end && err == VE_OK; cur++)
	{
		Bittab* b = new Bittab;
		if (b != nil)
		{
			err = b->Or(cur->second);
			if (err == VE_OK)
			{
				try
				{
					trans->fSavedRecordIDs.insert(make_pair(cur->first, b));
				}
				catch (...)
				{
					err = ThrowBaseError(memfull, DBaction_ExecutingTransaction);
				}
			}

			//b->libere();
		}
		else
			err = ThrowBaseError(memfull, DBaction_ExecutingTransaction);
		if (err != VE_OK)
			ReleaseRefCountable(&b);
	}

	// puis on garde, dans la fille, les bittab des blobs detruits dans la transaction
	for (mapRecIDs::iterator cur = fDeletedBlobIDs.begin(), end = fDeletedBlobIDs.end(); cur != end && err == VE_OK; cur++)
	{
		Bittab* b = new Bittab;
		if (b != nil)
		{
			err = b->Or(cur->second);
			if (err == VE_OK)
			{
				try
				{
					trans->fDeletedBlobIDs.insert(make_pair(cur->first, b));
				}
				catch (...)
				{
					err = ThrowBaseError(memfull, DBaction_ExecutingTransaction);
				}
			}

			//b->libere();
		}
		else
			err = ThrowBaseError(memfull, DBaction_ExecutingTransaction);
		if (err != VE_OK )
			ReleaseRefCountable(&b);
	}

	if (err != VE_OK)
		trans->SetValid(false);
	
	libere();

	if (!trans->IsValid())
	{
		fille = nil;
		delete trans;
		trans = nil;
	}

	return(trans);
}

#if debug
extern uBOOL __okdeletesousbt;
#endif


VError Transaction::DetruireCleAll(IndexInfo* ind, void* xvals)
{
	VError err = VE_OK;

	if (ind->IsScalar())
	{
		ind->DetruireAllCleForTrans(xvals, fOwner);
	}
	else
	{
		ind->WaitEndOfQuickBuilding();
		ind->Open(index_write);
		OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
		mapIndexKeys* vals = (mapIndexKeys*)xvals;
		for (mapIndexKeys::iterator cur = vals->begin(), end = vals->end(); cur != end && err == VE_OK; cur++)
		{
			BTitemIndex* val = const_cast<BTitemIndexHolder*>( &*cur)->StealKey();

			if (val != nil)
			{
				//const_cast<BTitemIndexHolder*>(&*cur)->fKey = nil; // val sera delete par DetruireCleForTrans
				VError err2;
				err2 = ind->DetruireCleForTrans(curstack, val, fOwner);
			}
		}
		ind->Close();
	}

	return err;
}


VError Transaction::PlaceCleAll(IndexInfo* ind, void* xvals)
{
	VError err = VE_OK;

	if (ind->IsScalar())
	{
		ind->PlaceCleAllForTrans(xvals, fOwner);
	}
	else
	{
		ind->WaitEndOfQuickBuilding();
		ind->Open(index_write);
		OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
		mapIndexKeys* vals = (mapIndexKeys*)xvals;
		for (mapIndexKeys::iterator cur = vals->begin(), end = vals->end(); cur != end && err == VE_OK; cur++)
		{
			BTitemIndex* val = const_cast<BTitemIndexHolder*>( &*cur)->StealKey();

			if (val != nil)
			{
				//const_cast<BTitemIndexHolder*>(&*cur)->fKey = nil; // val sera delete par PlaceCleForTrans
				VError err2;
				err2 = ind->GetHeader()->DelFromNulls(curstack, val->GetQui(), fOwner);
				err2 = ind->PlaceCleForTrans(curstack, val, fOwner);
			}
		}
		ind->Close();
	}

	return err;
}


VError Transaction::Commit(Boolean &outTryAgain)
{
	VError err = VE_OK;
	outTryAgain = false;
	occupe();

	if (IsValid())
	{
		//UnlockAllKeptLocks();

		if (mere == nil)  // c'est la transaction de base le commit va reellement se faire
		{
			fOwner->GetBase()->DelayForFlush();
			if (fOwner->GetBase()->OkToUpdate(err))
			{
				if (fOwner->GetBase()->TryToStartDataModif(fOwner))
				{
					// destruction pour tous les index des cles qui sont conservees dans l'ensembles des cles a detruire
					fOwner->WriteLog(DB4D_Log_Commit);
					UnlockAllKeptLocks(false);

					OccupableStack* curstack = gOccPool.GetStackForCurrentThread();

					for (mapIndexInfo::iterator cur = fDeletedKeys.begin(), end = fDeletedKeys.end(); cur != end && err == VE_OK; cur++)
					{
						IndexInfo* ind = cur->first;
						void* vals = cur->second;
						if (vals != nil)
						{
							DetruireCleAll(ind, vals);
						}
					}

					for (mapNullsIndexInfo::iterator cur = fDeletedNullKeys.begin(), end = fDeletedNullKeys.end(); cur != end && err == VE_OK; cur++)
					{
						IndexInfo* ind = cur->first;
						Bittab* vals = cur->second;
						if (vals != nil)
						{
							ind->GetHeader()->DelFromNulls(curstack, vals, fOwner);
						}
					}

					for (mapNullsIndexInfo::iterator cur = fNullKeys.begin(), end = fNullKeys.end(); cur != end && err == VE_OK; cur++)
					{
						IndexInfo* ind = cur->first;
						Bittab* vals = cur->second;
						if (vals != nil)
						{
							ind->GetHeader()->AddToNulls(curstack, vals, fOwner);
						}
					}

					// Ajout pour tous les index des cles qui sont conservees dans l'ensembles des cles a ajouter
					for (mapIndexInfo::iterator cur = fSavedKeys.begin(), end = fSavedKeys.end(); cur != end && err == VE_OK; cur++)
					{
						IndexInfo* ind = cur->first;
						void* vals = cur->second;
						if (vals != nil)
						{
							PlaceCleAll(ind, vals);
						}
					}

					// suppression des blobs qui sont references par les bittab de la transaction
					for (mapRecIDs::iterator cur = fDeletedBlobIDs.begin(), end = fDeletedBlobIDs.end(); cur != end && err == VE_OK; cur++)
					{
						Bittab* b = cur->second;
						sLONG tableid = cur->first;

						Table* tt = fOwner->GetBase()->RetainTable(tableid);
						if (tt != nil)
						{
							tt->occupe();
							DataTable* df = tt->GetDF();
							/*
							if (df != nil)
								df->occupe();
								*/
							tt->libere();
							if (df != nil)
							{
								sLONG i = b->FindNextBit(0);
								assert(i < kTransNewBlobNum);
								while (i != -1 && i < kTransNewBlobNum)
								{
									df->DelBlobForTrans(i, fOwner,true);
									i = b->FindNextBit(i+1);
									assert(i < kTransNewBlobNum);
								}

								//df->libere();
							}
							tt->Release();
						}

					}

					for (mapBlobPaths::iterator cur = fOutsideDeletedBlobs.begin(), end = fOutsideDeletedBlobs.end(); cur != end && err == VE_OK; cur++)
					{
						cur->second.fDF->MarkOutsideBlobToDelete(cur->first, cur->second.fPath);
					}

					for (mapOutsideBlobsInTrans::iterator cur = fOutsideBlobs.begin(), end = fOutsideBlobs.end(); cur != end && err == VE_OK; cur++)
					{
						
						Blob4D* blob = cur->second.RetainBlob(cur->first.first, this);
						if (testAssert(blob != nil)) // ne devrait jamais se produire
						{
							DataTable* df = blob->GetDF();
							df->SaveBlobForTrans(blob, fOwner);
							blob->Release();
						}
						
					}

					// Sauvegarde des blobs qui sont dans la map de la transaction
					for (mapBlobsInTrans::iterator cur = fBlobs.begin(), end = fBlobs.end(); cur != end && err == VE_OK; cur++)
					{
						Blob4D* blob = cur->second.RetainBlob(cur->first.fTableID, this);
						if (testAssert(blob != nil)) // ne devrait jamais se produire
						{
							DataTable* df = blob->GetDF();
							sLONG tableID = cur->first.fTableID;
							sLONG numrec = cur->first.fNumRec;
							assert(numrec == blob->GetNum());
							assert(tableID == df->GetNum());
							df->SaveBlobForTrans(blob, fOwner);
							blob->Release();
						}
					}

					// suppression des fiches qui sont referencees par les bittab de la transaction
					for (mapRecIDs::iterator cur = fDeletedRecordIDs.begin(), end = fDeletedRecordIDs.end(); cur != end && err == VE_OK; cur++)
					{
						Bittab* b = cur->second;
						sLONG tableid = cur->first;
						
						Table* tt = fOwner->GetBase()->RetainTable(tableid);
						if (tt != nil)
						{
							tt->occupe();
							DataTable* df = tt->GetDF();
							/*
							if (df != nil)
								df->occupe();
								*/
							tt->libere();
							if (df != nil)
							{
								sLONG i = b->FindNextBit(0);
								while (i != -1)
								{
									df->DelRecordForTrans(i, fOwner);
									i = b->FindNextBit(i+1);
								}

								//df->libere();
							}
							tt->Release();
						}
						
					}

					// Sauvegarde des fiches qui sont dans la map de la transaction
					for (mapRecsInTrans::iterator cur = fRecords.begin(), end = fRecords.end(); cur != end && err == VE_OK; cur++)
					{
						FicheOnDisk* ficD = cur->second.RetainRecord(cur->first.fTableID, this);
						if (testAssert(ficD != nil)) // ne devrait jamais se produire
						{
							DataTable* df = ficD->GetOwner();
							sLONG tableID = cur->first.fTableID;
							sLONG numrec = cur->first.fNumRec;
							assert(numrec == ficD->getnumfic());
							assert(tableID == df->GetNum());
							df->SaveRecordInTrans(ficD, fOwner);
							//ficD->FreeAfterUse();
							ficD->Release();
						}
					}

					fOwner->GetBase()->EndDataModif(fOwner);

				}
				else
					outTryAgain = true;
				fOwner->GetBase()->ClearUpdating();
			}
			else
			{
				SetValid(false);
				err = RollBack();
			}
			
		}
		else  // on merge la sous transaction avec sa mere
		{
			fOwner->WriteLog(DB4D_Log_Commit);

			UnlockAllKeptLocks(false);

			Boolean somerecswheredeleted = fDeletedRecordIDs.size() != 0;
			Boolean someblobswheredeleted = fDeletedBlobIDs.size() != 0;

			// recopie des cles detruites pour chaque index dans la transaction mere
			for (mapIndexInfo::iterator cur = fDeletedKeys.begin(), end = fDeletedKeys.end(); cur != end && err == VE_OK; cur++)
			{
				err = mere->RecopieIndexes(cur, mere->fDeletedKeys);
			}

			// recopie des cles ajoutees pour chaque index dans la transaction mere
			for (mapIndexInfo::iterator cur = fSavedKeys.begin(), end = fSavedKeys.end(); cur != end && err == VE_OK; cur++)
			{
				err = mere->RecopieIndexes(cur, mere->fSavedKeys);
			}

			for (mapNullsIndexInfo::iterator cur = fDeletedNullKeys.begin(), end = fDeletedNullKeys.end(); cur != end && err == VE_OK; cur++)
			{
				err = mere->RecopieNullIndexes(cur, mere->fDeletedNullKeys);
			}

			for (mapNullsIndexInfo::iterator cur = fNullKeys.begin(), end = fNullKeys.end(); cur != end && err == VE_OK; cur++)
			{
				err = mere->RecopieNullIndexes(cur, mere->fNullKeys);
			}

			mapBlobPaths& mereOutsideDeletedBlobs = mere->fOutsideDeletedBlobs;
			for (mapBlobPaths::iterator cur = fOutsideDeletedBlobs.begin(), end = fOutsideDeletedBlobs.end(); cur != end && err == VE_OK; cur++)
			{
				// Ajout de l'ensemble des path de blob detruits de la fille dans l'ensemble des path de blob detruits de la mere
				mereOutsideDeletedBlobs[cur->first] = cur->second;
			}

			// Merge de l'ensemble des outside blob sauves de la fille dans l'ensemble des outside blob sauves de la mere
			for (mapOutsideBlobsInTrans::iterator cur = fOutsideBlobs.begin(), end = fOutsideBlobs.end(); cur != end && err == VE_OK; cur++)
			{
				mapOutsideBlobsInTrans::iterator found = mere->fOutsideBlobs.find(cur->first);
				if (found == mere->fOutsideBlobs.end())
				{
					try
					{
						mere->fOutsideBlobs.insert(*cur);
					}
					catch (...)
					{
						err = ThrowBaseError(memfull, DBaction_CommittingTransaction);
					}
				}
				else
				{
					found->second = cur->second;
				}
				if (err == VE_OK)
				{
					if (cur->second.fBlob != nil)
					{
						cur->second.fBlob->Release();
						cur->second.fBlob = nil; // important pour que le ClearOutsideBlobInTrans ne touche pas a ce blob qui vient de passer dans la transaction mere
#if debugOverlaps_strong
						cur->second.fAddr = -2;
#endif
					}
				}
			}

			// Ajout de l'ensemble des ID de blob detruits de la fille dans l'ensemble des ID de blob detruits de la mere
			for (mapRecIDs::iterator cur = fDeletedBlobIDs.begin(), end = fDeletedBlobIDs.end(); cur != end && err == VE_OK; cur++)
			{
				Bittab* b = cur->second;
				sLONG tableid = cur->first;

				Bittab* b2 = mere->GetDeletedBlobIDs(tableid, err, true);
				if (b2 != nil)
				{
					err = b2->Or(b);
					b->Release();
					cur->second = nil;
				}
			}

			if (err == VE_OK)
			{
				// Merge de l'ensemble des blob sauves de la fille dans l'ensemble des blob sauves de la mere
				for (mapBlobsInTrans::iterator cur = fBlobs.begin(), end = fBlobs.end(); cur != end && err == VE_OK; cur++)
				{
					mapBlobsInTrans::iterator found = mere->fBlobs.find(cur->first);
					if (found == mere->fBlobs.end())
					{
						try
						{
							mere->fBlobs.insert(*cur);
						}
						catch (...)
						{
							err = ThrowBaseError(memfull, DBaction_CommittingTransaction);
						}
					}
					else
					{
						found->second = cur->second;
					}
					if (err == VE_OK)
					{
						if (cur->second.fBlob != nil)
						{
							cur->second.fBlob->Release();
							cur->second.fBlob = nil; // important pour que le ClearBlobInTrans ne touche pas a ce blob qui vient de passer dans la transaction mere
#if debugOverlaps_strong
							cur->second.fAddr = -2;
#endif
						}
					}
				}
			}

			if (err == VE_OK)
			{
				// si il y a des blobs detruits dans la fille, il faut verifier qu'ils ne sont pas de l'ensemble des blob sauves de la mere
				// et les en retirer si c'est le cas
				if (someblobswheredeleted)
				{
					Bittab* b = nil;
					sLONG curtableID = 0;
					vector<RecordIDInTrans> ToDelete;
					for (mapBlobsInTrans::iterator cur = mere->fBlobs.begin(), end = mere->fBlobs.end(); cur != end && err == VE_OK; cur++)
					{
						if (cur->first.fTableID != curtableID)
						{
							curtableID = cur->first.fTableID;
							b = mere->GetDeletedBlobIDs(curtableID, err, false);
						}
						if (b!=nil && b->isOn(cur->first.fNumRec))
						{
							//Blob4D* blob = cur->second->RetainBlob(curtableID, this);
							//if (blob != nil)
							{
								RecordIDInTrans id(curtableID, cur->first.fNumRec);
								SetRecId::iterator found = mere->fNewBlobNums.find(id);
								if (found != mere->fNewBlobNums.end())
								{ // si le blob vient d'etre cree dans cette sous transaction
									Table* tt = fOwner->GetBase()->RetainTable(id.fTableID);
									if (tt != nil)
									{
										DataTable* df = tt->GetDF();
										if (df != nil)
										{
											df->LockWrite();
											OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
											df->LibereAddrBlob(curstack, id.fNumRec, fOwner);
											df->CheckForMemRequest();
											df->Unlock();

											b->Clear(id.fNumRec); // on retire le blob de la bittab des blobs a supprimer de la mere, puisqu'on vient de le faire.
										}
										tt->Release();
									}
									mere->fNewBlobNums.erase(found);
								}
							}

							/*
							cur->second = nil;
							blob->Release();
							*/
							try
							{
								ToDelete.push_back(cur->first);
							}
							catch (...)
							{
								err = ThrowBaseError(memfull, DBaction_CommittingTransaction);
							}
						}
					}
					if (err == VE_OK)
					{
						for (vector<RecordIDInTrans>::iterator cur = ToDelete.begin(), end = ToDelete.end(); cur != end; cur++)
						{
#if debugOverlaps_strong
							debug_DelBlobAddrInTrans(*cur);
#endif
							mere->fBlobs.erase(*cur);
						}
					}
				}
			}


			if (err == VE_OK)
			{
				// Ajout de l'ensemble des ID de fiches sauvees de la fille dans l'ensemble des ID de fiches sauvees de la mere
				for (mapRecIDs::iterator cur = fSavedRecordIDs.begin(), end = fSavedRecordIDs.end(); cur != end && err == VE_OK; cur++)
				{
					Bittab* b = cur->second;
					sLONG tableid = cur->first;
					Bittab* b2 = mere->GetSavedRecordIDs(tableid, err, true);
					if (b2 != nil)
					{
						err = b2->Or(b);
						b->Release();
						cur->second = nil;
					}
				}
			}

			// Ajout de l'ensemble des ID de fiches detruites de la fille dans l'ensemble des ID de fiches detruites de la mere
			// et suppression dans l'ensemble des ID de fiches sauvees dans la mere
			for (mapRecIDs::iterator cur = fDeletedRecordIDs.begin(), end = fDeletedRecordIDs.end(); cur != end && err == VE_OK; cur++)
			{
				Bittab* b = cur->second;
				sLONG tableid = cur->first;
				Bittab* b3 = mere->GetSavedRecordIDs(tableid, err, true);
				if (b3 != nil)
				{
					err = b3->moins(b);
				}

				if (err == VE_OK)
				{
					Bittab* b2 = mere->GetDeletedRecordIDs(tableid, err, true);
					if (b2 != nil)
					{
						err = b2->Or(b);
						b->Release();
						cur->second = nil;
					}
				}
			}

			if (err == VE_OK)
			{
				// Merge de l'ensemble des fiches sauvees de la fille dans l'ensemble des fiches sauvees de la mere
				for (mapRecsInTrans::iterator cur = fRecords.begin(), end = fRecords.end(); cur != end && err == VE_OK; cur++)
				{
					mapRecsInTrans::iterator found = mere->fRecords.find(cur->first);
					if (found == mere->fRecords.end())
					{
						try
						{
							mere->fRecords.insert(*cur);
						}
						catch (...)
						{
							err = ThrowBaseError(memfull, DBaction_CommittingTransaction);
						}
					}
					else
					{
						found->second = cur->second;
					}
					if (err == VE_OK)
					{
						if (cur->second.fRec != nil)
						{
							//cur->second.fRec->FreeAfterUse();
							cur->second.fRec->Release();
							cur->second.fRec = nil; // important pour que le ClearRecInTrans ne touche pas a cette fiche qui vient de passer dans la transaction mere
						}
						cur->second.fAddr = -4; // precise a ClearRecInTrans de ne pas essayer de delocker la fiche
					}
					
				}
			}

			if (err == VE_OK)
			{
				// si il y a des fiches detruites dans la fille, il faut verifier qu'elles ne sont pas de l'ensemble des fiches sauvees de la mere
				// et les en retirer si c'est le cas
				if (somerecswheredeleted)
				{
					Bittab* b = nil;
					sLONG curtableID = 0;
					vector<RecordIDInTrans> ToDelete;
					for (mapRecsInTrans::iterator cur = mere->fRecords.begin(), end = mere->fRecords.end(); cur != end && err == VE_OK; cur++)
					{
						if (cur->first.fTableID != curtableID)
						{
							curtableID = cur->first.fTableID;
							b = mere->GetDeletedRecordIDs(curtableID, err, false);
						}
						if (b!=nil && b->isOn(cur->first.fNumRec))
						{
							//FicheOnDisk* ficD = cur->second.RetainRecord(cur->first.fTableID, this);
							{
								RecordIDInTrans id(cur->first.fTableID, cur->first.fNumRec);
								SetRecId::iterator found = mere->fNewRecNums.find(id);
								if (found != mere->fNewRecNums.end())
								{ // si la fiche vient d'etre creee dans cette sous transaction
									Table* tt = fOwner->GetBase()->RetainTable(id.fTableID);
									if (tt != nil)
									{
										DataTable* df = tt->GetDF();
										if (df != nil)
										{
											df->LockWrite();
											OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
											df->LibereAddrRec(curstack, id.fNumRec, fOwner);
											df->UnlockRecord(id.fNumRec, fOwner, DB4D_Do_Not_Lock /* mode special qui ne verifie que le lock de la fiche et non pas si elle est dans les differents ensembles de la transaction*/);
											df->CheckForMemRequest();
											df->Unlock();

											b->Clear(id.fNumRec); // on retire la fiche de la bittab des fiches a supprimer de la mere, puisqu'on vient de le faire.
										}
										tt->Release();
									}
									mere->fNewRecNums.erase(found);
									// mere->DecNewRecs(id.fTableID);
								}
							}

							/* L.R : le 7 mai 2007, Je ne vois pas pourquoi il faut delocker la fiche detruite dans la sous transaction

							sLONG n = ficD->getnumfic();
							ficD->GetOwner()->UnlockRecord(n, fOwner,  DB4D_Keep_Lock_With_Record);
							ficD->FreeAfterUse();
							*/

							try
							{
								ToDelete.push_back(cur->first);
							}
							catch (...)
							{
								err = ThrowBaseError(memfull, DBaction_CommittingTransaction);
							}
						}
					}
					if (err == VE_OK)
					{
						for (vector<RecordIDInTrans>::iterator cur = ToDelete.begin(), end = ToDelete.end(); cur != end; cur++)
						{
							mere->fRecords.erase(*cur);
						}
					}
				}
			}

			if (err == VE_OK)
			{
				for (SetRecId::const_iterator cur = fNewRecNums.begin(), end = fNewRecNums.end(); cur != end && err == VE_OK; cur++)
				{
					mere->fNewRecNums.insert(*cur);
					//mere->IncNewRecs(cur->fTableID);
					sLONG* p = (sLONG*) &(cur->fTableID);
					*p = -(*p);
				}
				for (SetRecId::const_iterator cur = fNewBlobNums.begin(), end = fNewBlobNums.end(); cur != end && err == VE_OK; cur++)
				{
					mere->fNewBlobNums.insert(*cur);
					sLONG* p = (sLONG*) &(cur->fTableID);
					*p = -(*p);
				}
			}

			if (err != VE_OK)
			{
				mere->SetValid(false);
				RollBack();
			}

			if (mere->fTempFile == nil)
			{
				mere->fTempFile = fTempFile;
				if (fTempFile != nil)
					fTempFile->Retain();
			}
			mere->fEndOfTempFile = fEndOfTempFile;
			mere->fTempFileDesc = fTempFileDesc;
			fTempFileDesc = nil;

			Transaction::RegisterTrans(fID, mere);
			fID = 0;
		}

	}
	else
	{
		err = RollBack();
		err = ThrowBaseError(VE_DB4D_TRANSACTIONISINVALID, DBaction_CommittingTransaction);
	}

	if (mere != nil)
		mere->fille = nil;

	libere();
	return(err);
}


VError Transaction::FreeAddrTableNewEntries()
{
	VError err = VE_OK;

	// liberation des adresses des blobs crees dans la transaction
	Table* curTable = nil;
	sLONG curTableId = 0;
	for (SetRecId::const_iterator cur = fNewBlobNums.begin(), end = fNewBlobNums.end(); cur != end; cur++)
	{
		sLONG TableID = cur->fTableID;
		sLONG numrec = cur->fNumRec;
		if (TableID > 0)
		{
			if (TableID != curTableId)
			{
				if (curTable != nil)
					curTable->Release();
				curTableId = TableID;
				curTable = fOwner->GetBase()->RetainTable(curTableId);
			}
			if (curTable != nil)
			{
				DataTable* df = curTable->GetDF();
				df->LockWrite();
				OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
				df->LibereAddrBlob(curstack, numrec, fOwner);
				df->CheckForMemRequest();
				df->Unlock();
			}
		}
	}

	if (curTable != nil)
		curTable->Release();
	fNewBlobNums.clear();


	// liberation des adresses des fiches creees dans la transaction
	curTable = nil;
	curTableId = 0;
	for (SetRecId::const_iterator cur = fNewRecNums.begin(), end = fNewRecNums.end(); cur != end; cur++)
	{
		sLONG TableID = cur->fTableID;
		sLONG numrec = cur->fNumRec;
		if (TableID > 0)
		{
			if (TableID != curTableId)
			{
				if (curTable != nil)
					curTable->Release();
				curTableId = TableID;
				curTable = fOwner->GetBase()->RetainTable(curTableId);
			}
			if (curTable != nil)
			{
				DataTable* df = curTable->GetDF();
				// le unlock record qui se trouvait dans [ for_each(fRecords) ClearRecInTrans ] est maintenant ici : ref 16/11/2006
				df->LockWrite();
				OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
				df->UnlockRecord(numrec, fOwner, DB4D_Do_Not_Lock /* mode special qui ne verifie que le lock de la fiche et non pas si elle est dans les differents ensembles de la transaction*/);
				df->LibereAddrRec(curstack, numrec, fOwner);
				df->CheckForMemRequest();
				df->Unlock();
			}
		}
	}

	if (curTable != nil)
		curTable->Release();
	fNewRecNums.clear();

	return err;
}


VError Transaction::RollBack(void)
{
	VError err = VE_OK;
	
	occupe();
	
	if (fille != nil)
	{
		fille->RollBack();
		delete fille;
		fille = nil;
	}

	closing = true;

	UnlockAllKeptLocks(true);

	FreeAddrTableNewEntries();

	if (mere != nil)
	{
		for (mapRecsInTrans::iterator cur = fRecords.begin(), end = fRecords.end(); cur != end; cur++)
		{
			bool foundHigher = false;
			Transaction* trans = mere;
			while (trans != nil && !foundHigher)
			{
				if (trans->fRecords.find(cur->first) != trans->fRecords.end())
					foundHigher = true;
				trans = trans->mere;
			}

			if (foundHigher)
			{
				if (cur->second.fRec != nil)
				{
					cur->second.fRec->Release();
					cur->second.fRec = nil; // pour que ClearRecInTrans n'y touche pas, car la fiche est referencee dans une transaction mere
				}
				cur->second.fAddr = -4; // precise a ClearRecInTrans de ne pas essayer de delocker la fiche
			}
		}


		for (mapBlobsInTrans::iterator cur = fBlobs.begin(), end = fBlobs.end(); cur != end; cur++)
		{
			bool foundHigher = false;
			Transaction* trans = mere;
			while (trans != nil && !foundHigher)
			{
				if (trans->fBlobs.find(cur->first) != trans->fBlobs.end())
					foundHigher = true;
				trans = trans->mere;
			}

			if (foundHigher)
			{
				if (cur->second.fBlob != nil)
				{
					cur->second.fBlob->Release();
					cur->second.fBlob = nil; // pour que ClearBlobInTrans n'y touche pas, car le blob est reference dans une transaction mere
				}
			}
		}
	}

	DeleteAll();

	if (mere != nil)
	{
		if (mere->fTempFile == nil)
		{
			mere->fTempFile = fTempFile;
			if (fTempFile != nil)
				fTempFile->Retain();
		}
		mere->fEndOfTempFile = fEndOfTempFile;
		mere->fTempFileDesc = fTempFileDesc;
		fTempFileDesc = nil;

		mere->fille = nil;

		Transaction::RegisterTrans(fID, mere);
		fID = 0;
	}
	
	libere();
	return(err);
}

void Transaction::InvalidateTransaction()
{
	VObjLock lock(this);

	FreeAddrTableNewEntries();
	SetValid(false);

	if (mere != nil)
		mere->InvalidateTransaction();

}



void Transaction::DeleteAll(void)
{
	occupe();
	
	// rien a faire, tout est fait ailleur
	
	libere();
}

/*
sLONG Transaction::GetNewXNum(mapNewNum& fNums, sLONG StartNewNum, sLONG TableID)
{
	sLONG result;

	try
	{
		mapNewNum::iterator found = fNums.find(TableID);
		if (found == fNums.end())
		{
			fNums.insert(make_pair(TableID, StartNewNum));
			result = StartNewNum;
		}
		else
		{
			result = ++(found->second);
		}
	}
	catch (...)
	{
		result = -1;
	}

	return(result);
}
*/

VError Transaction::AddNewRecNum(sLONG TableID, sLONG numrec)
{
	VObjLock lock(this);
	VError err = VE_OK;
	try
	{
		RecordIDInTrans x(TableID, numrec);
		fNewRecNums.insert(x);
	}
	catch (...)
	{
		err = ThrowBaseError(memfull, DBaction_ExecutingTransaction);
	}
	return err;
}



VError Transaction::DelNewRecNum(sLONG TableID, sLONG numrec)
{
	VObjLock lock(this);
	VError err = VE_OK;
	RecordIDInTrans x(TableID, numrec);
	fNewRecNums.erase(x);
	return err;
}


VError Transaction::AddNewBlobNum(sLONG TableID, sLONG numblob)
{
	VObjLock lock(this);
	VError err = VE_OK;
	try
	{
		RecordIDInTrans x(TableID, numblob);
		fNewBlobNums.insert(x);
	}
	catch (...)
	{
		err = ThrowBaseError(memfull, DBaction_ExecutingTransaction);
	}
	return err;
}


Bittab* Transaction::GetxRecordIDs(sLONG TableID, VError& err, sLONG x, Boolean BuildIfMissing) 
// x == 0 means SavedRecordIDs, x == 1 means DeletedRecordIDs, x== 2 means DeletedBlobIDs
{
	VObjLock lock(this);

	Bittab* b;
	try
	{
		mapRecIDs* recs = &fDeletedRecordIDs;
		if (x == 0)
			recs = &fSavedRecordIDs;
		if (x == 2)
			recs = &fDeletedBlobIDs;

		mapRecIDs::iterator y = recs->find(TableID);
		if (y == recs->end())
		{
			if (BuildIfMissing)
			{
				b = new Bittab;
				if (b == nil)
					err = ThrowBaseError(memfull, DBaction_ExecutingTransaction);
				else
				{
					recs->insert(make_pair(TableID, b));
				}
			}
			else
				b = nil;
		}
		else
		{
			b = y->second;
			//b->occupe();
		}

	}

	catch (...)
	{
		err = ThrowBaseError(memfull, DBaction_ExecutingTransaction);
		b = nil;
	}

	return b;
}


VError Transaction::SaveRecord(FicheOnDisk* ficD, VSize RecSize)
{
	VError err = VE_OK;

	VObjLock lock(this);

	fPossibleUsedDiskSize += RecSize;

	if (IsValid())
	{
		RecordIDInTrans id(ficD);

		err = KeepRecordLocked(ficD->GetOwner(), id.fNumRec);

		if (err == VE_OK)
		{
			Bittab* b = GetSavedRecordIDs(id.fTableID, err);
			if (err == VE_OK)
			{
				err = b->Set(id.fNumRec, true);
				//b->libere();
			}
		}

		if (err == VE_OK)
		{
			try
			{
				mapRecsInTrans::iterator found = fRecords.find(id);
				if (found == fRecords.end())
				{
					/*
					ficD->occupe();
					ficD->use();
					ficD->libere();
					*/
					ficD->SetInTrans();
					ficD->Retain();
					ficD->SetModificationStamp(ficD->GetOwner()->GetRecordStampInAddressTable(id.fNumRec));
					fRecords.insert(make_pair(id, FicheInTrans(ficD)));
				}
				else
				{
					FicheOnDisk* old = found->second.fRec;
					/*
					ficD->occupe();
					ficD->use();
					ficD->libere();
					*/
					ficD->SetInTrans();
					ficD->Retain();
					if (old != nil)
						ficD->SetModificationStamp(old->GetModificationStamp());
					found->second.fRec = ficD;
					found->second.fAddr = -1;
					if (old != nil)
					{
						/*
						old->occupe();
						old->FreeAfterUse();
						*/
						old->Release();
					}
				}
			}
			catch (...)
			{
				err = ThrowBaseError(memfull, DBaction_ExecutingTransaction);
			}
		}

		if (err != VE_OK)
			SetValid(false);
	}
	else
		err = ThrowBaseError(VE_DB4D_TRANSACTIONISINVALID, DBaction_ExecutingTransaction);

	if (err != VE_OK)
		err = ThrowBaseError(VE_DB4D_CANNOTSAVERECORD_NO_PARAM, DBaction_ExecutingTransaction);

	return err;
}


VError Transaction::DelRecord(sLONG TableID, sLONG RecID)
{
	VError err = VE_OK;

	VObjLock lock(this);

	if (IsValid())
	{
		RecordIDInTrans id(TableID, RecID);

		Bittab* b = GetSavedRecordIDs(id.fTableID, err, false);
		if (err == VE_OK)
		{
			if (b != nil)
			{
				err = b->Clear(id.fNumRec, false);
				//b->libere();
			}
		}

		if (err == VE_OK)
		{
			SetRecId::iterator found = fNewRecNums.find(id);
			if (found == fNewRecNums.end())
			{
				b = GetDeletedRecordIDs(id.fTableID, err, true);
				if (err == VE_OK)
				{
					err = b->Set(id.fNumRec, true);
					//b->libere();
				}
				if (err == VE_OK)
				{
					Table* tt = fOwner->GetBase()->RetainTable(TableID);
					if (tt != nil)
					{
						DataTable* df = tt->GetDF();
						if (df != nil)
						{
							err = KeepRecordLocked(df, RecID);
						}
						tt->Release();
					}
				}
			}
			else 
			{ // si la fiche vient d'etre creee dans cette sous transaction
				Table* tt = fOwner->GetBase()->RetainTable(TableID);
				if (tt != nil)
				{
					DataTable* df = tt->GetDF();
					if (df != nil)
					{
						df->LockWrite();
						OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
						df->UnlockRecord(RecID, fOwner, DB4D_Keep_Lock_With_Transaction);
						df->LibereAddrRec(curstack, RecID, fOwner);
						df->CheckForMemRequest();
						df->Unlock();
					}
					tt->Release();
				}
				fNewRecNums.erase(found);
			}
		}

		if (err == VE_OK)
		{
			mapRecsInTrans::iterator found = fRecords.find(id);
			if (found != fRecords.end())
			{
				//FicheOnDisk* ficD = found->second.fRec;
				fRecords.erase(found);
				/*
				if (ficD != nil)
				{
					ficD->occupe();
					ficD->FreeAfterUse();
				}
				*/
			}
		}

		if (err != VE_OK)
			SetValid(false);
	}
	else
		err = ThrowBaseError(VE_DB4D_TRANSACTIONISINVALID, DBaction_ExecutingTransaction);

	if (err != VE_OK)
		err = ThrowBaseError(VE_DB4D_CANNOTDELETERECORD, DBaction_ExecutingTransaction);

	return err;
}


FicheOnDisk* Transaction::GetRecord(sLONG TableID, sLONG RecID, VError& err)
{
	VObjLock lock(this);

	RecordIDInTrans id(TableID, RecID);
	mapRecsInTrans::iterator found = fRecords.find(id);
	if (found == fRecords.end())
	{
		if (mere == nil)
			return nil;
		else
			return mere->GetRecord(TableID, RecID, err);
	}
	else
	{
		FicheOnDisk* ficD = found->second.GetRecord(TableID, this);
		return ficD;
	}
}


FicheOnDisk* Transaction::RetainRecord(sLONG TableID, sLONG RecID, VError& err)
{
	VObjLock lock(this);

	RecordIDInTrans id(TableID, RecID);
	mapRecsInTrans::iterator found = fRecords.find(id);
	if (found == fRecords.end())
	{
		if (mere == nil)
			return nil;
		else
			return mere->RetainRecord(TableID, RecID, err);
	}
	else
	{
		FicheOnDisk* ficD = found->second.RetainRecord(TableID, this);
		return ficD;
	}
}




VError Transaction::SaveBlob(Blob4D* blob)
{
	VError err = VE_OK;

	VObjLock lock(this);

	if (IsValid())
	{
		if (blob->IsOutsidePath())
		{
			blob->_CreatePathIfEmpty();
			VString id;
			blob->GetOutsideID(id);
			BlobPathID fullID = make_pair(blob->GetDF()->GetNum(), id);

			mapOutsideBlobsInTrans::iterator found = fOutsideBlobs.find(fullID);
			if (found == fOutsideBlobs.end())
			{
				blob->Retain();
				blob->SetInTrans();
				fOutsideBlobs.insert(make_pair(fullID, blob));
			}
			else
			{
				Blob4D* old = found->second.fBlob;
				blob->Retain();
				blob->SetInTrans();
				found->second.fBlob = blob;
				found->second.fAddr = -1;
				if (old != nil)
					old->Release();
			}
		}
		else
		{
			RecordIDInTrans id(blob);
			{
				try
				{
					mapBlobsInTrans::iterator found = fBlobs.find(id);
					if (found == fBlobs.end())
					{
						blob->Retain();
						blob->SetInTrans();
						fBlobs.insert(make_pair(id, blob));
					}
					else
					{
						Blob4D* old = found->second.fBlob;
						blob->Retain();
						blob->SetInTrans();
						found->second.fBlob = blob;
						found->second.fAddr = -1;
						if (old != nil)
							old->Release();
					}
				}
				catch (...)
				{
					err = ThrowBaseError(memfull, DBaction_ExecutingTransaction);
				}
			}
		}

		if (err != VE_OK)
			SetValid(false);
	}
	else
		err = ThrowBaseError(VE_DB4D_TRANSACTIONISINVALID, DBaction_ExecutingTransaction);

	if (err != VE_OK)
		err = ThrowBaseError(VE_DB4D_CANNOTSAVEBLOB, DBaction_ExecutingTransaction);

	return err;
}


VError Transaction::DelOldBlobPath(Blob4D* blob)
{
	VError err = VE_OK;

	VObjLock lock(this);

	if (IsValid())
	{
		if (blob->IsPathRelative())
		{
			VString oldID;
			blob->GetOldOutsideID(oldID);
			if (!oldID.IsEmpty())
			{
				OutsideDeletedBlobRef* p = &(fOutsideDeletedBlobs[oldID]);
				p->fPath = oldID;
				p->fDF = blob->GetDF();
			}
		}
	}

	return err;
}


VError Transaction::DelOutsideBlob(Blob4D* blob)
{
	VError err = VE_OK;

	VObjLock lock(this);

	if (IsValid())
	{
		
		if (blob->IsPathRelative())
		{
			VString oldID;
			blob->GetOldOutsideID(oldID);
			if (!oldID.IsEmpty())
			{
				OutsideDeletedBlobRef* p = &(fOutsideDeletedBlobs[oldID]);
				p->fPath = oldID;
				p->fDF = blob->GetDF();
			}

			blob->_CreatePathIfEmpty();
			VString id;
			blob->GetOutsideID(id);
			BlobPathID fullID = make_pair(blob->GetDF()->GetNum(), id);
			mapOutsideBlobsInTrans::iterator found = fOutsideBlobs.find(fullID);
			if (found == fOutsideBlobs.end())
			{
				OutsideDeletedBlobRef* p = &(fOutsideDeletedBlobs[id]);
				p->fPath = blob->GetComputedPath();
				p->fDF = blob->GetDF();
			}
			else
			{
				fOutsideBlobs.erase(fullID);
			}
		}
	}
	else
		err = ThrowBaseError(VE_DB4D_TRANSACTIONISINVALID, DBaction_ExecutingTransaction);

	if (err != VE_OK)
		err = ThrowBaseError(VE_DB4D_CANNOTDELETEBLOBS, DBaction_ExecutingTransaction);
	return err;
}


VError Transaction::DelBlob(sLONG TableID, sLONG RecID)
{
	VError err = VE_OK;

	VObjLock lock(this);

	if (IsValid())
	{
		RecordIDInTrans id(TableID, RecID);

		SetRecId::iterator found = fNewBlobNums.find(id);
		if (found == fNewBlobNums.end())
		{
			Bittab* b = GetDeletedBlobIDs(id.fTableID, err, true);
			if (err == VE_OK)
			{
				err = b->Set(id.fNumRec, true);
				//b->libere();
			}
		}
		else 
		{ // si le blob vient d'etre cree dans cette sous transaction
			Table* tt = fOwner->GetBase()->RetainTable(TableID);
			if (tt != nil)
			{
				DataTable* df = tt->GetDF();
				if (df != nil)
				{
					df->LockWrite();
					OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
					df->LibereAddrBlob(curstack, RecID, fOwner);
					df->CheckForMemRequest();
					df->Unlock();
				}
				tt->Release();
			}
			fNewBlobNums.erase(found);
		}

		if (err == VE_OK)
		{
			mapBlobsInTrans::iterator found = fBlobs.find(id);
			if (found != fBlobs.end())
			{
				//Blob4D* blob = found->second;
				fBlobs.erase(found);
				/*
				if (blob != nil)
				{
					blob->Release();
				}
				*/
			}
		}

		if (err != VE_OK)
			SetValid(false);
	}
	else
		err = ThrowBaseError(VE_DB4D_TRANSACTIONISINVALID, DBaction_ExecutingTransaction);

	if (err != VE_OK)
		err = ThrowBaseError(VE_DB4D_CANNOTDELETEBLOBS, DBaction_ExecutingTransaction);

	return err;
}


Blob4D* Transaction::GetBlob(sLONG TableID, sLONG BlobID, VError& err)
{
	VObjLock lock(this);

	RecordIDInTrans id(TableID, BlobID);
	mapBlobsInTrans::iterator found = fBlobs.find(id);
	if (found == fBlobs.end())
	{
		if (mere == nil)
			return nil;
		else
			return mere->GetBlob(TableID, BlobID, err);
	}
	else
	{
		Blob4D* blob = found->second.RetainBlob(TableID, this);

		//if (blob != nil)
		{
			//blob->libere(); // sera fait par l'appelant
			//blob->Release();
		}
		return blob;
	}
}


Boolean Transaction::IsRecordDeleted(sLONG TableID, sLONG RecID)
{
	VObjLock lock(this);
	VError err = VE_OK;
	{
		Bittab* b = GetDeletedRecordIDs(TableID, err, false);
		if (b == nil)
			return false;
		else
		{
			return b->isOn(RecID);
		}
	}
}


Boolean Transaction::IsBlobDeleted(sLONG TableID, sLONG BlobID)
{
	VObjLock lock(this);
	VError err = VE_OK;
	if (BlobID >= kTransNewBlobNum)
		return false;
	else
	{
		Bittab* b = GetDeletedBlobIDs(TableID, err, false);
		if (b == nil)
			return false;
		else
		{
			return b->isOn(BlobID);
		}
	}
}


mapIndexKeys* Transaction::NewIndexKeysMap()
{
	mapIndexKeys* res = nil;
#if 0
	void* p = GetFastMem((sLONG)sizeof(mapIndexKeys));
	if (p != nil)
	{
		res = new (p) mapIndexKeys;
	}
#else
	res = new mapIndexKeys;
#endif
	return res;
}


void* Transaction::GetDeletedKeys(IndexInfo* ind, Boolean BuildIfMissing, VError& err)
{
//	VObjLock lock(this);

	mapIndexInfo::iterator found = fDeletedKeys.find(ind);
	if (found == fDeletedKeys.end())
	{
		void* parent;

		if (mere == nil)
			parent = nil;
		else
			parent = mere->GetDeletedKeys(ind, false, err);

		if (BuildIfMissing)
		{
			Boolean isscalar = ind->IsScalar();
			void* vals = nil;
			try
			{
				if (isscalar)
					vals = ind->NewMapKeys();
				else
					vals = NewIndexKeysMap();

				if (vals == nil)
					err = ThrowBaseError(memfull, DBaction_ExecutingTransaction);
				else
				{
					if (parent != nil)
					{
						if (isscalar)
						{
							ind->CopyMapKeys(parent, vals);
						}
						else
						{
							*((mapIndexKeys*)vals) = *((mapIndexKeys*)parent);  // recopie l'ensemble des cles de la transaction mere
						}
					}

					fDeletedKeys.insert(make_pair(ind, vals));
					ind->Retain();
				}
			}
			catch (...)
			{
				err = ThrowBaseError(memfull, DBaction_ExecutingTransaction);
			}

			if (err != VE_OK)
			{
				if (isscalar)
					ind->DeleteMapKeys(vals);
				else
					delete (mapIndexKeys*)vals;
				vals = nil;
				SetValid(false);
			}

			return vals;
		}
		else
			return parent;
	}
	else
	{
		return found->second;
	}
}


void* Transaction::GetSavedKeys(IndexInfo* ind, Boolean BuildIfMissing, VError& err)
{
//	VObjLock lock(this);

	mapIndexInfo::iterator found = fSavedKeys.find(ind);
	if (found == fSavedKeys.end())
	{
		void* parent;

		if (mere == nil)
			parent = nil;
		else
			parent = mere->GetSavedKeys(ind, false, err);

		if (BuildIfMissing)
		{
			Boolean isscalar = ind->IsScalar();
			void* vals = nil;
			try
			{
				if (isscalar)
					vals = ind->NewMapKeys();
				else
					vals = NewIndexKeysMap();

				if (vals == nil)
					err = ThrowBaseError(memfull, DBaction_ExecutingTransaction);
				else
				{
					if (parent != nil)
					{
						if (isscalar)
						{
							ind->CopyMapKeys(parent, vals);
						}
						else
						{
							*((mapIndexKeys*)vals) = *((mapIndexKeys*)parent);  // recopie l'ensemble des cles de la transaction mere
						}
					}

					fSavedKeys.insert(make_pair(ind, vals));
					ind->Retain();
				}
			}
			catch (...)
			{
				err = ThrowBaseError(memfull, DBaction_ExecutingTransaction);
			}

			if (err != VE_OK)
			{
				if (isscalar)
					ind->DeleteMapKeys(vals);
				else
					delete (mapIndexKeys*)vals;
				vals = nil;
				SetValid(false);
			}

			return vals;
		}
		else
			return parent;
	}
	else
	{
		return found->second;
	}
}

Bittab* Transaction::GetNullKeys(IndexInfo* ind, Boolean BuildIfMissing, VError& err)
{
//	VObjLock lock(this);

	mapNullsIndexInfo::iterator found = fNullKeys.find(ind);
	if (found == fNullKeys.end())
	{
		Bittab* parent;

		if (mere == nil)
			parent = nil;
		else
			parent = mere->GetNullKeys(ind, false, err);

		if (BuildIfMissing)
		{
			Bittab* vals = nil;
			try
			{
				vals = new Bittab();

				if (vals == nil)
					err = ThrowBaseError(memfull, DBaction_ExecutingTransaction);
				else
				{
					if (parent != nil)
						err = vals->Or(parent, true);  // recopie l'ensemble des nulls de la transaction mere
					if (err == VE_OK)
					{
						fNullKeys.insert(make_pair(ind, vals));
						ind->Retain();
					}
				}
			}
			catch (...)
			{
				err = ThrowBaseError(memfull, DBaction_ExecutingTransaction);
			}

			if (err != VE_OK)
			{
				ReleaseRefCountable(&vals);
				SetValid(false);
			}

			return vals;
		}
		else
			return parent;
	}
	else
	{
		return found->second;
	}
}



Bittab* Transaction::GetDeletedNullKeys(IndexInfo* ind, Boolean BuildIfMissing, VError& err)
{
	//	VObjLock lock(this);

	mapNullsIndexInfo::iterator found = fDeletedNullKeys.find(ind);
	if (found == fDeletedNullKeys.end())
	{
		Bittab* parent;

		if (mere == nil)
			parent = nil;
		else
			parent = mere->GetDeletedNullKeys(ind, false, err);

		if (BuildIfMissing)
		{
			Bittab* vals = nil;
			try
			{
				vals = new Bittab();

				if (vals == nil)
					err = ThrowBaseError(memfull, DBaction_ExecutingTransaction);
				else
				{
					if (parent != nil)
						err = vals->Or(parent, true);  // recopie l'ensemble des nulls de la transaction mere
					if (err == VE_OK)
					{
						fDeletedNullKeys.insert(make_pair(ind, vals));
						ind->Retain();
					}
				}
			}
			catch (...)
			{
				err = ThrowBaseError(memfull, DBaction_ExecutingTransaction);
			}

			if (err != VE_OK)
			{
				ReleaseRefCountable(&vals);
				SetValid(false);
			}

			return vals;
		}
		else
			return parent;
	}
	else
	{
		return found->second;
	}
}


Boolean Transaction::ConcernIndex(IndexInfo* ind)
{
	VError err;
	return ( (GetSavedKeys(ind, false, err) != nil) || (GetDeletedKeys(ind, false, err) != nil) );
}



VError Transaction::RecopieIndexes(mapIndexInfo::iterator& from, mapIndexInfo& into)
{
	VError err = VE_OK;
	IndexInfo* ind = from->first;
	void* vals = from->second;

	if (vals != nil)
	{
		mapIndexInfo::iterator found = into.find(ind);
		if (found == into.end())
		{
			try
			{
				into.insert(make_pair(ind, vals));
				ind->Retain();
				from->second = nil;  // on vole le pointer de mapIndexKeys et on le remplace par nil.
				// quand cette transaction fille sera detruite, la mapIndexKeys ne le sera pas car il y aura nil.
			}
			catch (...)
			{
				err = ThrowBaseError(memfull, DBaction_ExecutingTransaction);
			}
		}
		else
		{
			void* vals2 = found->second;
			if (vals2 != nil)
			{
				if (ind->IsScalar())
					ind->DeleteMapKeys(vals2);
				else
				delete (mapIndexKeys*)vals2;
			}
			found->second = vals;
			from->second = nil;  // on vole le pointer de mapIndexKeys et on le remplace par nil.
			// quand cette transaction fille sera detruite, la mapIndexKeys ne le sera pas car il y aura nil.			
		}
	}
	return err;
}



VError Transaction::RecopieNullIndexes(mapNullsIndexInfo::iterator& from, mapNullsIndexInfo& into)
{
	VError err = VE_OK;
	IndexInfo* ind = from->first;
	Bittab* vals = from->second;

	if (vals != nil)
	{
		mapNullsIndexInfo::iterator found = into.find(ind);
		if (found == into.end())
		{
			try
			{
				into.insert(make_pair(ind, vals));
				ind->Retain();
				from->second = nil;  // on vole le pointer de bittab et on le remplace par nil.
				// quand cette transaction fille sera detruite, la bittab ne le sera pas car il y aura nil.
			}
			catch (...)
			{
				err = ThrowBaseError(memfull, DBaction_ExecutingTransaction);
			}
		}
		else
		{
			Bittab* vals2 = found->second;
			ReleaseRefCountable(&vals2);
			found->second = vals;
			from->second = nil;  // on vole le pointer de bittab et on le remplace par nil.
			// quand cette transaction fille sera detruite, la bittab ne le sera pas car il y aura nil.			
		}
	}
	return err;
}


VError Transaction::RemoveDeleteKeysFromFourche(IndexInfo* ind, const BTitemIndex* val1, uBOOL xstrict1, const BTitemIndex* val2, uBOOL xstrict2,
												const VCompareOptions& inOptions, Bittab* dejasel)
{
	VError err = VE_OK;

	mapIndexKeys* vals = (mapIndexKeys*)GetDeletedKeys(ind, false, err);
	if (vals != nil)
	{
		mapIndexKeys::const_iterator cur = vals->lower_bound(BTitemIndexHolder(val1, ind, inOptions.IsBeginsWith(), inOptions.IsLike(), fOwner)), end = vals->end();
		Boolean upperbound = false;
		Boolean lowerbound = false;

		if (cur != end)
		{
			//mapIndexKeys::const_reverse_iterator cur2 = reverse_bidirectional_iterator<mapIndexKeys::iterator, BTitemIndexHolder, mapIndexKeys::reference, mapIndexKeys::difference_type>(cur);
			mapIndexKeys::const_reverse_iterator cur2 = mapIndexKeys::const_reverse_iterator(cur);
			mapIndexKeys::const_reverse_iterator end2 = vals->rend();

			while (cur2 != end2 && !lowerbound && err == VE_OK)
			{
				BTitemIndex* key = cur2->RetainKey();
				if (key != nil)
				{
					if (ind->CompareKeys(key, val1, inOptions) != CR_EQUAL)
					{
						lowerbound = true;
					}
					else
					{
						sLONG numfic = key->GetQui();
						{
							err = dejasel->Set(numfic, true);
						}
					}
					key->Unuse();
				}
				else
				{
					err = ThrowBaseError(VE_DB4D_TRANSACTIONISINVALID, DBaction_ExecutingTransaction);
					break;
				}
				cur2++;
			}
		}

		while (cur != end && !upperbound && err == VE_OK)
		{
			BTitemIndex* key = cur->RetainKey();
			if (key != nil)
			{
				if (ind->CompareKeys(key, val2, inOptions) == CR_BIGGER)
					upperbound = true;
				else
				{
					sLONG numfic = key->GetQui();
					{
						err = dejasel->Set(numfic, true);
					}
				}
				key->Unuse();
			}
			else
			{
				err = ThrowBaseError(VE_DB4D_TRANSACTIONISINVALID, DBaction_ExecutingTransaction);
				break;
			}
			cur++;
		}
	}
	return err;
}


VError Transaction::ParseAllIndex(IndexInfo* ind, const BTitemIndex* val1, const VCompareOptions& inOptions, Bittab* into)
{
	VError err = VE_OK;
	mapIndexKeys* vals  = (mapIndexKeys*)GetDeletedKeys(ind, false, err);
	if (vals != nil)
	{
		for (mapIndexKeys::const_iterator cur = vals->begin(), end = vals->end(); cur != end && err == VE_OK; cur++)
		{
			BTitemIndex* key = cur->RetainKey();
			if (key != nil)
			{
				if (ind->CompareKeys(key, val1, inOptions) == CR_EQUAL)
				{
					err = into->Clear(key->GetQui());
				}
				key->Unuse();
			}

		}
	}

	if (err == VE_OK)
	{
		vals = (mapIndexKeys*)GetSavedKeys(ind, false, err);
		if (vals != nil)
		{
			for (mapIndexKeys::const_iterator cur = vals->begin(), end = vals->end(); cur != end && err == VE_OK; cur++)
			{
				BTitemIndex* key = cur->RetainKey();
				if (key != nil)
				{
					if (ind->CompareKeys(key, val1, inOptions) == CR_EQUAL)
					{
						err = into->Set(key->GetQui());
					}
					key->Unuse();
				}
			}
		}
	}

	return err;
}


VError Transaction::Fourche(IndexInfo* ind, const BTitemIndex* val1, uBOOL xstrict1, const BTitemIndex* val2, uBOOL xstrict2, 
							const VCompareOptions& inOptions, Bittab* dejasel, Bittab* into, BTitemIndex** outVal)
{
	VError err = VE_OK;

	VObjLock lock(this);

	mapIndexKeys* vals;
	if (outVal == nil)
	{
		vals = (mapIndexKeys*)GetDeletedKeys(ind, false, err);
		if (vals != nil)
		{
			if (val1 == nil)
			{
				if (val2 != nil)
				{
					mapIndexKeys::const_iterator cur = vals->begin(), end = vals->end();
					Boolean upperbound = false;
					while (cur != end && !upperbound && err == VE_OK)
					{
						BTitemIndex* key = cur->RetainKey();
						if (key != nil)
						{
							if (ind->CompareKeys(key, val2, inOptions) == CR_BIGGER)
								upperbound = true;
							else
							{
								sLONG numfic = key->GetQui();
								// if (dejasel == nil || dejasel->isOn(numfic))
								{
									err = into->Clear(numfic);
								}
							}
							key->Unuse();
						}
						else
						{
							err = ThrowBaseError(VE_DB4D_TRANSACTIONISINVALID, DBaction_ExecutingTransaction);
							break;
						}
						cur++;
					}
				}
			}
			else
			{
				mapIndexKeys::const_iterator cur = vals->lower_bound(BTitemIndexHolder(val1, ind, inOptions.IsBeginsWith(), inOptions.IsLike(), fOwner)), end = vals->end();
				Boolean upperbound = false;
				Boolean lowerbound = false;

				if (cur != end)
				{
					mapIndexKeys::const_reverse_iterator cur2 = mapIndexKeys::const_reverse_iterator(cur);
					mapIndexKeys::const_reverse_iterator end2 = vals->rend();

					while (cur2 != end2 && !lowerbound && err == VE_OK)
					{
						BTitemIndex* key = cur2->RetainKey();
						if (key != nil)
						{
							if (ind->CompareKeys(key, val1, inOptions) != CR_EQUAL)
							{
								lowerbound = true;
							}
							else
							{
								sLONG numfic = key->GetQui();
								{
									err = into->Clear(numfic);
								}
							}
							key->Unuse();
						}
						else
						{
							err = ThrowBaseError(VE_DB4D_TRANSACTIONISINVALID, DBaction_ExecutingTransaction);
							break;
						}
						cur2++;
					}
				}

				while (cur != end && !upperbound && err == VE_OK)
				{
					BTitemIndex* key = cur->RetainKey();
					if (key != nil)
					{
						if (val2 != nil && ind->CompareKeys(key, val2, inOptions) == CR_BIGGER)
							upperbound = true;
						else
						{
							sLONG numfic = key->GetQui();
							// if (dejasel == nil || dejasel->isOn(numfic))
							{
								err = into->Clear(numfic);
							}
						}
						key->Unuse();
					}
					else
					{
						err = ThrowBaseError(VE_DB4D_TRANSACTIONISINVALID, DBaction_ExecutingTransaction);
						break;
					}
					cur++;
				}
			}
		}
	}

	if (err == VE_OK)
	{
		vals = (mapIndexKeys*)GetSavedKeys(ind, false, err);
		if (vals != nil)
		{
			if (val1 == nil)
			{
				if (val2 != nil)
				{
					mapIndexKeys::const_iterator cur = vals->begin(), end = vals->end();
					Boolean upperbound = false;
					while (cur != end && !upperbound && err == VE_OK)
					{
						BTitemIndex* key = cur->RetainKey();
						if (key != nil)
						{
							sLONG comp2 = ind->CompareKeys(key, val2, inOptions);
							if (comp2 == CR_BIGGER || (comp2 == CR_EQUAL && xstrict2))
								upperbound = true;
							else
							{
								sLONG numfic = key->GetQui();
								if (dejasel == nil || dejasel->isOn(numfic))
								{
									err = into->Set(numfic);
									if (outVal != nil)
									{
										*outVal = ind->CopyKey(key);
										break;
									}
								}
							}
							key->Unuse();
						}
						else
						{
							err = ThrowBaseError(VE_DB4D_TRANSACTIONISINVALID, DBaction_ExecutingTransaction);
							break;
						}
						cur++;
					}
				}
			}
			else
			{
				mapIndexKeys::const_iterator cur = vals->lower_bound(BTitemIndexHolder(val1, ind, inOptions.IsBeginsWith(), inOptions.IsLike(), fOwner)), end = vals->end();
				Boolean upperbound = false;
				Boolean lowerbound = false;

				if (cur != end)
				{
					mapIndexKeys::const_reverse_iterator cur2 = mapIndexKeys::const_reverse_iterator(cur);
					mapIndexKeys::const_reverse_iterator end2 = vals->rend();

					while (cur2 != end2 && !lowerbound && err == VE_OK)
					{
						BTitemIndex* key = cur2->RetainKey();
						if (key != nil)
						{
							if (ind->CompareKeys(key, val1, inOptions) != CR_EQUAL)
							{
								lowerbound = true;
							}
							else
							{
								if (!xstrict1)
								{
									sLONG numfic = key->GetQui();
									if (dejasel == nil || dejasel->isOn(numfic))
									{
										err = into->Set(numfic);
										if (outVal != nil)
										{
											*outVal = ind->CopyKey(key);
											upperbound = true;
											break;
										}
									}
								}
							}
							key->Unuse();
						}
						else
						{
							err = ThrowBaseError(VE_DB4D_TRANSACTIONISINVALID, DBaction_ExecutingTransaction);
							break;
						}
						cur2++;
					}
				}

				while (cur != end && !upperbound && err == VE_OK)
				{
					bool ok = true;
					BTitemIndex* key = cur->RetainKey();
					if (key != nil)
					{
						if (xstrict1 && ind->CompareKeys(key, val1, inOptions) == CR_EQUAL)
							ok = false;

						if (val2 != nil && ind->CompareKeys(key, val2, inOptions) == CR_BIGGER)
							upperbound = true;
						else
						{
							if (ok)
							{
								sLONG numfic = key->GetQui();
								if (dejasel == nil || dejasel->isOn(numfic))
								{
									err = into->Set(numfic);
									if (outVal != nil)
									{
										*outVal = ind->CopyKey(key);
										break;
									}
								}
							}
						}
						key->Unuse();
					}
					else
					{
						err = ThrowBaseError(VE_DB4D_TRANSACTIONISINVALID, DBaction_ExecutingTransaction);
						break;
					}
					cur++;
				}
			}
		}
	}

	return err;
}


VError Transaction::PlaceCle(IndexInfo* ind, BTitemIndex* val, sLONG numrec)
{
	VObjLock lock(this);

	VError err = VE_OK;
	Boolean KeyWasFoundInDeletedMap = false;

	if (val->IsNull())
	{
		Bittab* b = GetNullKeys(ind, true, err);
		if (err == VE_OK && b != nil)
		{
			err = b->Set(numrec, true);
		}
		Bittab* b2 = GetDeletedNullKeys(ind, false, err);
		if (err == VE_OK && b2 != nil)
		{
			err = b2->Clear(numrec, false);
		}
		ind->FreeKey(val);
	}
	else
	{
		ind->Retain();
		val->SetInd(ind);
		val->SetQui(numrec);
		val->StartUse();


		mapIndexKeys* vals = (mapIndexKeys*)GetDeletedKeys(ind, true, err);
		if (err == VE_OK && vals != nil)
		{
			mapIndexKeys::iterator foundkey = vals->find(BTitemIndexHolder(val, fOwner));
			if (foundkey != vals->end())
			{
				KeyWasFoundInDeletedMap = true;
				vals->erase(foundkey);
			}
		}

		if (err == VE_OK && !KeyWasFoundInDeletedMap)
		{
			mapIndexKeys* vals = (mapIndexKeys*)GetSavedKeys(ind, true, err);
			if (err == VE_OK)
			{
				assert(vals != nil);

				try
				{
					vals->insert(BTitemIndexHolder(val, fOwner));
				}
				catch (...)
				{
					err = ThrowBaseError(memfull, DBaction_ExecutingTransaction);
				}
			}
		}

		val->Unuse();
		ind->Release();
	}
	if (err != VE_OK)
		SetValid(false);
	return err;
}


VError Transaction::DetruireCle(IndexInfo* ind, BTitemIndex* val, sLONG numrec)
{
	VObjLock lock(this);

	VError err = VE_OK;
	Boolean KeyWasFoundInSavedMap = false;

	if (val->IsNull())
	{
		Bittab* b = GetNullKeys(ind, false, err);
		if (b != nil && err == VE_OK)
		{
			err = b->Clear(numrec, false);
		}
		Bittab* b2 = GetDeletedNullKeys(ind, true, err);
		if (b2 != nil && err == VE_OK)
		{
			err = b2->Set(numrec, true);
		}
		ind->FreeKey(val);
	}
	else
	{
		ind->Retain();
		val->SetInd(ind);
		val->SetQui(numrec);
		val->StartUse();

		mapIndexKeys* vals = (mapIndexKeys*)GetSavedKeys(ind, true, err);
		if (err == VE_OK && vals != nil)
		{
			mapIndexKeys::iterator foundkey = vals->find(BTitemIndexHolder(val, fOwner));
			if (foundkey != vals->end())
			{
				KeyWasFoundInSavedMap = true;
				vals->erase(foundkey);
			}
		}

		if (err == VE_OK && !KeyWasFoundInSavedMap)
		{
			mapIndexKeys* vals = (mapIndexKeys*)GetDeletedKeys(ind, true, err);
			if (err == VE_OK)
			{
				assert(vals != nil);

				try
				{
					vals->insert(BTitemIndexHolder(val, fOwner));
				}
				catch (...)
				{
					err = ThrowBaseError(memfull, DBaction_ExecutingTransaction);
				}
			}
		}

		val->Unuse();
		ind->Release();
	}

	if (err != VE_OK)
		SetValid(false);
	return err;
}


void Transaction::IncNewRecs(sLONG TableID)
{
	VObjLock lock(this);
	try
	{
		fBaseTrans->fHowManyNewRecs[TableID]++;
	}
	catch (...)
	{
		assert(false);
	}
}


void Transaction::DecNewRecs(sLONG TableID)
{
	VObjLock lock(this);
	mapNewNum::iterator found = fBaseTrans->fHowManyNewRecs.find(TableID);
	if (testAssert(found != fBaseTrans->fHowManyNewRecs.end()))
	{
		found->second--;
		assert(found->second >= 0);
	}
}


sLONG Transaction::GetHowManyNewRecs(sLONG TableID)
{
	VObjLock lock(this);
	mapNewNum::iterator found = fBaseTrans->fHowManyNewRecs.find(TableID);
	if (found == fBaseTrans->fHowManyNewRecs.end())
		return 0;
	else
		return found->second;
}



VError Transaction::CreateTempFile()
{
	VError err = VE_OK;
	VStr63 s;
	sLONG_PTR n;

	if (fTempFile == nil)
	{
		fEndOfTempFile = 0;
		n=reinterpret_cast<sLONG_PTR>(this);
		//n=n>>4;
		s.FromLong( n);
		s += ".trans";

		fTempFileDesc = NULL;

		VFolder *tempfolder = fOwner->GetBase()->RetainTemporaryFolder( true, &err);
		if (tempfolder != nil)
		{
			fTempFile=new VFile(*tempfolder, s);
			tempfolder->Release();

			fTempFile->Delete(); 
			err=fTempFile->Create( true);
		}

		if (err == VE_OK)
		{
			err=fTempFile->Open( FA_READ_WRITE, &fTempFileDesc );
		}

		if (err != VE_OK)
		{
			if (fTempFileDesc != nil)
				delete fTempFileDesc;
			fTempFileDesc = nil;
			fTempFile->Release();
			fTempFile = nil;
		}
	}

	return err;
}


VError Transaction::PutTempData(const void* data, sLONG len, DataAddr4D ou)
{
	VError err = CreateTempFile();

	if (err == VE_OK)
	{
		err = fTempFileDesc->PutData(data, (VSize)len, ou);
	}

	return err;
}


VError Transaction::GetTempData(void* data, sLONG len, DataAddr4D ou)
{
	VError err;
	if (fTempFileDesc == nil)
	{
		assert(fTempFileDesc != nil);
		err = ThrowBaseError(VE_DB4D_TRANSACTIONISINVALID, DBaction_ExecutingTransaction);
	}
	else
		err = fTempFileDesc->GetData(data, (VSize)len, ou);

	if (err != VE_OK)
	{
		err = ThrowBaseError(VE_DB4D_CANNOT_GET_TEMP_DATA_FOR_TRANS, DBaction_ExecutingTransaction);
	}

	return err;
}


void Transaction::FreeTempSpace(DataAddr4D ou, sLONG len)
{
	// rien pour l'instant
}


DataAddr4D Transaction::FindTempSpace(sLONG len, VError& err)
{
	DataAddr4D result = -1;
	err = CreateTempFile();
	if (err == VE_OK)
	{
#if debugtrans_temp
		assert(debug_Previous_EndOfTempFile <= fEndOfTempFile);
		if (debug_Previous_EndOfTempFile > fEndOfTempFile)
		{
			result = result;
		}
#endif		
		result = fEndOfTempFile;
		assert(len > 0);
#if debuglr
		if (len <= 0)
		{
			len = len;
		}
#endif
		fEndOfTempFile = fEndOfTempFile + len;
#if debugtrans_temp
		debug_Previous_EndOfTempFile = fEndOfTempFile;
#endif		
	}
	return result;
}


void Transaction::NeedsBytes(sLONG allocationBlockNumber, VSize needed, VSize& total)
{
	StErrorContextInstaller errors(false);

	VObjLock lock(this);

	VError err = VE_OK;
	for (mapOutsideBlobsInTrans::iterator cur = fOutsideBlobs.begin(), end = fOutsideBlobs.end(); cur != end && err == VE_OK; cur++)
	{
		sLONG tot = 0;
		err = cur->second.SaveToTemp(allocationBlockNumber, this, tot);
		total = total + tot;
	}

	for (mapBlobsInTrans::iterator cur = fBlobs.begin(), end = fBlobs.end(); cur != end && err == VE_OK; cur++)
	{
		sLONG tot = 0;
		err = cur->second.SaveToTemp(allocationBlockNumber, this, tot);
		total = total + tot;
	}

	for (mapRecsInTrans::iterator cur = fRecords.begin(), end = fRecords.end(); cur != end && err == VE_OK; cur++)
	{
		sLONG tot = 0;
		err = cur->second.SaveToTemp(allocationBlockNumber, this, tot);
		total = total + tot;
	}

	for (mapIndexInfo::iterator cur = fDeletedKeys.begin(), end = fDeletedKeys.end(); cur != end && err == VE_OK; cur++)
	{
		if (!cur->first->IsScalar())
		{			
			mapIndexKeys* keys = (mapIndexKeys*)cur->second;

			for (mapIndexKeys::const_iterator curx = keys->begin(), endx = keys->end(); curx != endx && err == VE_OK; curx++)
			{
				sLONG tot = 0;
				BTitemIndexHolder* it = (BTitemIndexHolder*) &(*curx);
				err = it->SaveToTemp(allocationBlockNumber, this, tot);
				total = total + tot;
			}
		}
	}

	for (mapIndexInfo::iterator cur = fSavedKeys.begin(), end = fSavedKeys.end(); cur != end && err == VE_OK; cur++)
	{
		if (!cur->first->IsScalar())
		{			
			mapIndexKeys* keys = (mapIndexKeys*)cur->second;

			for (mapIndexKeys::const_iterator curx = keys->begin(), endx = keys->end(); curx != endx && err == VE_OK; curx++)
			{
				sLONG tot = 0;
				BTitemIndexHolder* it = (BTitemIndexHolder*) &(*curx);
				err = it->SaveToTemp(allocationBlockNumber, this, tot);
				total = total + tot;
			}
		}
	}
}


VError Transaction::AddLogEntryToTrans(sLONG8 inLogPos)
{
	VObjLock lock(this);
	VError err = VE_OK;

	if (fKeepLogEntries)
	{
		try
		{
			fLogEntries.push_back(inLogPos);
		}
		catch (...)
		{
			err = memfull;
		}
	}

	return err;
}


VError Transaction::StealLogEntries(LogEntriesCollection& outEntries)
{
	VObjLock lock(this);

	VError err = VE_OK;
	try
	{
		fLogEntries.swap(outEntries);
	}
	catch (...)
	{
		err = memfull;
	}
	return err;
}



sLONG Transaction::GetNewID(Transaction* trans)
{
	sLONG result = 0;
	VTaskLock lock(&sAllTransMutex);
	do
	{
		sTransID++;
		if (sTransID != 0)
		{
			TransactionCollection::iterator found = sAllTrans.find(sTransID);
			if (found == sAllTrans.end())
			{
				sAllTrans[sTransID] = trans;
				result = sTransID;
			}
		}
	} while (result == 0);

	return result;
}


void Transaction::UnregisterTrans(sLONG id)
{
	VTaskLock lock(&sAllTransMutex);
	sAllTrans.erase(id);
}


void Transaction::RegisterTrans(sLONG id, Transaction* trans)
{
	VTaskLock lock(&sAllTransMutex);
	sAllTrans[id] = trans;
}


Transaction* Transaction::GetTransFromID(sLONG id)
{
	Transaction* result = nil;
	VTaskLock lock(&sAllTransMutex);
	if (id != 0)
	{
		TransactionCollection::iterator found = sAllTrans.find(id);
		if (found != sAllTrans.end())
		{
			result = found->second;
		}
	}

	return result;
}



Bittab* Transaction::GetKeptLocks(DataTable* df, VError& err, Boolean BuildIfMissing)
{
	Bittab* b;
	try
	{
		mapRecIDsByDataTable* recs = &fKeepLockIDs;

		mapRecIDsByDataTable::iterator x = recs->find(df);
		if (x == recs->end())
		{
			if (BuildIfMissing)
			{
				b = new Bittab;
				if (b == nil)
					err = ThrowBaseError(memfull, DBaction_ExecutingTransaction);
				else
				{
					recs->insert(make_pair(df, b));
				}
			}
			else
				b = nil;
		}
		else
		{
			b = x->second;
			//b->occupe();
		}

	}

	catch (...)
	{
		err = ThrowBaseError(memfull, DBaction_ExecutingTransaction);
		b = nil;
	}

	return b;
}


Bittab* Transaction::GetxMotherKeptLocks(DataTable* df)
{
	Bittab* b;
	mapRecIDsByDataTable* recs = &fMotherKeepLockIDs;

	mapRecIDsByDataTable::iterator x = recs->find(df);
	if (x == recs->end())
	{
		b = nil;
	}
	else
	{
		b = x->second;
	}

	return b;
}


VError Transaction::DoNotKeepRecordLocked(DataTable* df, sLONG recnum)
{
	VError err = VE_OK;

	if (testAssert(recnum>=0))
	{
		mapRecIDsByDataTable::iterator found = fKeepLockIDs.find(df);
		if (found != fKeepLockIDs.end())
		{
			Bittab* b = found->second;
			if (b != nil)
			{
				err = b->Clear(recnum);
			}
		}
	}

	return err;
}


VError Transaction::KeepRecordLocked(DataTable* df, sLONG recnum)
{
	VError err = VE_OK;

	if (testAssert(recnum>=0))
	{
		Bittab* b = GetKeptLocks(df, err, true);
		if (b != nil)
		{
			err = b->Set(recnum, true);
		}
	}

	return err;
}


Boolean Transaction::WasLockKeptInThatTransaction(DataTable* df, sLONG recnum)
{
	Boolean res = false;

	if (testAssert(recnum>=0))
	{
		VError err;
		Bittab* locks = GetKeptLocks(df, err, false);
		if (locks != nil)
		{
			res = locks->isOn(recnum);
		}
	}
	return res;
}


Boolean Transaction::IsRecordLockKept(DataTable* df, sLONG recnum)
{
	Boolean res = false;

	if (testAssert(recnum>=0))
	{
		VError err;
		Bittab* motherkept = GetxMotherKeptLocks(df);
		if (motherkept != nil && motherkept->isOn(recnum))
			res = true;
		else
		{
			Bittab* delrecs = GetDeletedRecordIDs(df->GetNum(), err, false);
			if (delrecs != nil && delrecs->isOn(recnum))
				res = true;
			else
			{
				Bittab* savedrec = GetSavedRecordIDs(df->GetNum(), err, false);
				if (savedrec != nil && savedrec->isOn(recnum))
					res = true;
				else
				{
					mapRecIDsByDataTable::iterator found = fKeepLockIDs.find(df);
					if (found != fKeepLockIDs.end())
					{
						Bittab* b = found->second;
						if (b != nil)
						{
							res = b->isOn(recnum);
						}
					}
				}
			}
		}
	}

	return res;
}


VError Transaction::TryToLockSel(DataTable* df, Bittab* b, Bittab* lockedset)
{
	VError err = VE_OK;
	if (df != nil && b != nil)
	{
		df->Retain();

		Bittab* keptlocked = GetKeptLocks(df, err, true);
		if (keptlocked != nil)
		{
			Bittab alocker;
			err = alocker.Or(b);
			if (err == VE_OK)
				err = alocker.moins(keptlocked);
			if (err == VE_OK)
			{
				sLONG n = alocker.FindNextBit(0);
				while (n != -1 && err == VE_OK)
				{
					if (!df->LockRecord(n, fOwner, DB4D_Keep_Lock_With_Record))
					{
						if (lockedset != nil)
							lockedset->Set(n, true);
						alocker.ClearFrom(n);
						{
							StErrorContextInstaller errs(false);
							UnlockSel(df, &alocker, false);
						}
						err = ThrowBaseError(VE_DB4D_RECORDISLOCKED, DBaction_ExecutingTransaction);
					}
					else
					{
						err = keptlocked->Set(n, true);
						if (err != VE_OK)
						{
							df->UnlockRecord(n, fOwner, DB4D_Do_Not_Lock);
							alocker.ClearFrom(n);
							{
								StErrorContextInstaller errs(false);
								UnlockSel(df, &alocker, false);
							}
						}
					}
					n = alocker.FindNextBit(n+1);
				}
			}
		}

		df->Release();
	}

	return err;
}


VError Transaction::UnlockSel(DataTable* df, Bittab* b, bool unlockDeleted) // ne delocke pas les fiches lockes dans la transaction mere, ni les fiches detruites ou modifiees
{
	VError err = VE_OK;
	if (df != nil && b != nil)
	{
		df->Retain();

		Bittab* keptlocked = GetKeptLocks(df, err, false);
		if (keptlocked != nil)
		{
			StErrorContextInstaller errs(false);
			Bittab* xb;
			Bittab adelocker;
			DB4D_Way_of_Locking howtounlock;
			/*
			if (b == keptlocked)
			{
				xb = b;
			}
			else
			*/
			{
				VError err2 = adelocker.Or(b);
				if (err2 == VE_OK)
					err2 = adelocker.And(keptlocked);
				if (err2 == VE_OK && !unlockDeleted)
				{
					Bittab* delrec = GetDeletedRecordIDs(df->GetNum(), err2, false);
					if (delrec != nil)
						err2 = adelocker.moins(delrec);
				}
				if (err2 == VE_OK)
				{
					Bittab* savedrec = GetSavedRecordIDs(df->GetNum(), err2, false);
					if (savedrec != nil)
						err2 = adelocker.moins(savedrec);
				}

				if (err2 == VE_OK)
				{
					Bittab* motherkept = GetxMotherKeptLocks(df);
					if (motherkept != nil)
						err2 = adelocker.moins(motherkept);
				}

				if (err2 == VE_OK)
				{
					howtounlock = DB4D_Do_Not_Lock; // special pour acceler le unlock
					xb = &adelocker;
				}
				else
				{
					howtounlock = DB4D_Keep_Lock_With_Transaction;
					xb = b;
				}
			}

			sLONG n = xb->FindNextBit(0);
			while (n != -1)
			{
				sLONG n2 = xb->FindNextBit(n+1);
				df->UnlockRecord(n, fOwner, howtounlock);
				n = n2;
			}

			keptlocked->moins(b);
		}
		df->Release();
	}

	return err;
}

VError Transaction::UnlockAllKeptLocks(bool unlockDeleted)
{
	VError err = VE_OK;
	VError err2 = VE_OK;

	for (mapRecIDsByDataTable::iterator cur = fKeepLockIDs.begin(), end = fKeepLockIDs.end(); cur != end; cur++)
	{
		err = UnlockSel(cur->first, cur->second, unlockDeleted); // ceci ne supprimera que les lock des fiches qui n'ont ni ete modifies ni ete detruites
		if (err != VE_OK)
			err2 = err;
	}

	return err2;
}


VSize Transaction::PossibleUsedDiskSize()
{
	return fPossibleUsedDiskSize;
}





			/* -------------------------------------------------------------------------------------- */


void InitTransactionManager(void)
{
}


void DeInitTransactionManager(void)
{
}







			/* -------------------------------------------------------------------------------------- */
