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

#include <map>


VFolder* DB4DJSRuntimeDelegate::RetainScriptsFolder()
{
	return RetainRefCountable(fOwner->GetBase()->GetStructFolder());
}





					// ============================================================================



uLONG8 BaseTaskInfo::sGlobID = 0;
VCriticalSection BaseTaskInfo::sGlobIDMutex;
StructLockerMap BaseTaskInfo::sStructLockers;
VCriticalSection BaseTaskInfo::sStructLockersMutex;
ContextCollection BaseTaskInfo::sAllContexts;

void BaseTaskInfo::xinit()
{
	{
		VTaskLock lock(&sGlobIDMutex);
		sGlobID++;
		if (sGlobID <= 0)
			sGlobID = 1;
		fID = sGlobID;
		sAllContexts.insert(this);
	}
	fKeepForRestTransaction = false;
	fPermErrorHappened = false;
	fSyncHelperContext = nil;
	fLastQueryPath = nil;
	fLastQueryPlan = nil;
	fRemoteMaxRecordStamp = 0;
	fAutoRelStamp = 0;
	fRemoteAutoRelStamp = 0;
	fRemoteTransactionLevel = 0;
	fParent = nil;
	fRetainsParent = false;
	#if WITH_ASSERT
	fBuildQueryPlan = false;
	fBuildQueryPath = false;
	#else
	fBuildQueryPlan = false; //(bd->GetStructure() != nil);
	fBuildQueryPath = false;
	#endif

	fWaitForLockTimer = 0;
	curtrans = nil;
	curtransForNeedsByte = nil;
	locker = new LockEntity(this);
	xbox_assert(locker != nil);
	fAllRels1toNAreAuto = false;
	fAllRelsNto1AreAuto = false;
	fIsCreationWrittenToLog = false;
	fUserID.Clear();
	fUserID.SetNull(true);
	fUserSession = nil;
	fThreadPrivileges = nil;
	fFeatures = 0;
	fFeaturesInited = 0;

	if (bd->GetIntlMgr() != nil)
	{
		fIntlMgr = bd->GetIntlMgr()->Clone();
	}
	else
	{
		Base4D *base = bd->GetStructure();

		if ((base != nil) && (base->GetIntlMgr() != nil))
		{
			fIntlMgr = base->GetIntlMgr()->Clone();
		}
		else
		{
			//xbox_assert(false);
			fIntlMgr = NULL;
		}
	}
	fDB4DNetManager = nil;
	fJSDelegate = nil;
	fCachedProgressIndicator  = NULL;
}


BaseTaskInfo::BaseTaskInfo(Base4D *xbd, CUAGSession* inUserSession, VJSGlobalContext* inJSContext, CDB4DBase* owner, bool islocal)
{
	bd=xbd;
	fOwner = RetainRefCountable(owner);
	//fEncapsuleur = encapsuleur;
	fRemoteBase = nil;
	fIsRemote = xbd->IsRemote();
	fIsLocal = islocal;
	xinit();
	fUserSession = RetainRefCountable(inUserSession);
	if (inJSContext == kJSContextCreator)
	{

		fJSDelegate = new DB4DJSRuntimeDelegate(this);
		fJSContext = VJSGlobalContext::Create( fJSDelegate);
	}
	else
		fJSContext = inJSContext;	// sc 24/08/2009 no more retain on JavaScript context
}


BaseTaskInfo::BaseTaskInfo(Base4DRemote *xbdremote, CUAGSession* inUserSession, VJSGlobalContext* inJSContext, CDB4DBase* owner, bool islocal)
{
	fRemoteBase = xbdremote;
	fOwner = RetainRefCountable(owner);
	bd=xbdremote->GetLocalDB();
	//fEncapsuleur = encapsuleur;
	fIsRemote = true;
	fIsLocal = islocal;
	xinit();
	fUserSession = RetainRefCountable(inUserSession);
	if (inJSContext == kJSContextCreator)
	{
		fJSDelegate = new DB4DJSRuntimeDelegate(this);
		fJSContext = VJSGlobalContext::Create( fJSDelegate);
	}
	else
		fJSContext = inJSContext;	// sc 24/08/2009 no more retain on JavaScript context

}


void BaseTaskInfo::DoCallOnRefCount0()
{
	if (fRetainsParent)
	{
		fRetainsParent = false;
		if (fParent != nil)
		{
			if (fParent->GetRefCount() > 1)
			{
				dynamic_cast<VDB4DContext*>(fParent)->NowOwns(this);
			}
			fParent->Release();
		}
	}
}


BaseTaskInfo::~BaseTaskInfo()
{
	Base4D* oldbd;

	ClearAllExtraData();

	FreeAllJSMethods();

	QuickReleaseRefCountable(fSyncHelperContext);

	VDBMgr::GetManager()->ForgetServerKeptRecords(this);
	// toute les fiches qui sont dans fTreeRecs ne sont pas retained
	// comme fTreeRecs deleteOnRemove alors les noeuds sont detruits

	{
		VTaskLock lock(&sGlobIDMutex);
		sAllContexts.erase(this);
	}

#if 0
	for (vector<RemoteRecordCache*>::iterator cur = fRemoteRecordCaches.begin(), end = fRemoteRecordCaches.end(); cur != end; cur++)
	{
		(*cur)->Release();
	}
#endif

	SendlastRemoteInfo();

	WriteDeletionToLog();

	for (RecRefMap::iterator cur = fPushedRecordIDs.begin(), end = fPushedRecordIDs.end(); cur != end; cur++)
	{
		sLONG recnum = cur->first.second;
		DataTable* df = cur->first.first;
		sLONG nbpush = cur->second;
		for (sLONG i = 0; i < nbpush; i++)
		{
			df->UnMarkRecordAsPushed(recnum);
		}
	}

	{
		VTaskLock lock(&sStructLockersMutex);
		for (StructLockerCollection::const_iterator cur = fStructLockers.begin(), end = fStructLockers.end(); cur != end; cur++)
		{
			sStructLockers.erase(*cur);
		}
	}

	if (locker != nil)
	{
		locker->SetOwner(nil);
		locker->Release();
	}

	Selection* sel = fLockingSels.GetFirst();
	while (sel != nil)
	{
		sel->UnLockRecords(this,-2, false);
		Selection* sel2 = sel->GetNext();
		fLockingSels.Remove(sel);
		sel->Release();
		sel = sel2;
	}

	for (RecordsInMemMap::iterator cur = fMapRecs.begin(), end = fMapRecs.end(); cur != end; cur++)
	{
		FicheInMem* rec = cur->second;
		rec->ClearContext();
	}
	
	if (bd != nil)
	{
		// L.E. 10/12/1999 on verifie que la base n'a pas ete detruite avant....
		xbox_assert( gCppMem->CheckVObject( bd));
		//bd->ClearLockCount(this);
		xSetBaseToNil();
	}
	
	ReleaseRefCountable( &fCachedProgressIndicator);
	
	ReleaseRefCountable( &fIntlMgr);

	QuickReleaseRefCountable(fLastQueryPlan);
	QuickReleaseRefCountable(fLastQueryPath);

	if (fDB4DNetManager != nil)
	{
		fDB4DNetManager->RemoveContext(GetEncapsuleur());
	}

	QuickReleaseRefCountable(fThreadPrivileges);
	QuickReleaseRefCountable(fUserSession);
	QuickReleaseRefCountable(fOwner);

	if (fJSDelegate != nil)
	{
		QuickReleaseRefCountable(fJSContext);
		delete fJSDelegate;
	}

}


void BaseTaskInfo::SetCurrentUser(const VUUID& inUserID, CUAGSession* inSession)
{
	fUserID = inUserID;
	CopyRefCountable(&fUserSession, inSession);
	//fUserSession = RetainRefCountable(inSession);
}


void BaseTaskInfo::GetCurrentUserName(VString& outName)
{
	if (fUserSession == nil)
		outName.Clear();
	else
	{
		CUAGUser* user = fUserSession->RetainUser();
		if (user == nil)
			outName.Clear();
		else
		{
			user->GetName(outName);
			user->Release();
		}
	}
}


void BaseTaskInfo::GetCurrentUserID(VUUID& outID)
{
	if (fUserSession == nil)
		outID.Clear();
	else
	{
		CUAGUser* user = fUserSession->RetainUser();
		if (user == nil)
			outID.Clear();
		else
		{
			user->GetID(outID);
			user->Release();
		}
	}
}


void BaseTaskInfo::SendlastRemoteInfo()
{
	if (IsRemoteLike() && (!fRemoteTransactions.empty() || !fClientReleasedRecIDs.empty()))
	{
		IRequest *req = bd->CreateRequest( GetEncapsuleur(), Req_SendLastRemoteInfo + kRangeReqDB4D);
		if (req != NULL)
		{
			req->PutBaseParam( bd);
			req->PutThingsToForget( VDBMgr::GetManager(), this);
			VError err = req->GetLastError();
			if (err == VE_OK)
			{
				err = req->Send();
			}
			req->Release();
		}

	}
}


void BaseTaskInfo::xSetBaseToNil()
{
	sLONG i;
	Transaction* trans;

	if (bd != nil)
	{
		while (curtrans != nil)
		{
			trans = curtrans->GetMother();
			curtrans->RollBack();
			delete curtrans;
			curtrans = trans;
		}
		
		// penser a supprimer les lock sur les fiches
		bd = nil;
	}
}


Transaction* BaseTaskInfo::GetCurrentTransaction(void) const
{ 
	Transaction* trans;

	//fTransactionMutex.Lock();
	
	trans = curtrans;
	while (trans != nil && trans->EndOfTrans())
	{
		trans = trans->GetMother();
	}

	//fTransactionMutex.Unlock();
	
	return(trans); 
}

void BaseTaskInfo::SetCachedProgressIndicator(XBOX::VProgressIndicator* inProgressToCache)
{
	XBOX::CopyRefCountable(&fCachedProgressIndicator,inProgressToCache);
}

XBOX::VProgressIndicator* BaseTaskInfo::GetCachedProgressIndicator()
{
	if(fCachedProgressIndicator == NULL)
	{
		IDB4D_ApplicationIntf* application = VDBMgr::GetManager()->GetApplicationInterface();
		
		//FIXME: on Wakanda, application is NULL
		//xbox_assert(application != NULL);
		if(application != NULL)
		{
			fCachedProgressIndicator = application->CreateProgressIndicator();
		}
	}
	return fCachedProgressIndicator;
}

Transaction* BaseTaskInfo::GetCurrentTransactionForNeedsByte(void) const
{ 
	VTaskLock lock(&fcurtransForNeedsByteMutex);
	if (curtransForNeedsByte != nil)
		curtransForNeedsByte->occupe();
	return curtransForNeedsByte;
}


VError BaseTaskInfo::AddLogEntryToTrans(sLONG8 inLogPos)
{
	VError err = VE_OK;
	Transaction* trans = curtrans;
	if (trans != nil)
		trans = trans->GetBaseTrans();
	if (trans != nil)
	{
		err = trans->AddLogEntryToTrans(inLogPos);
	}
	return err;
}


VError BaseTaskInfo::StealLogEntries(LogEntriesCollection& outEntries)
{
	VError err = VE_OK;
	Transaction* trans = curtrans;
	if (trans != nil)
		trans = trans->GetBaseTrans();
	if (trans != nil)
	{
		err = trans->StealLogEntries(outEntries);
	}
	return err;
}


VError BaseTaskInfo::WriteLog(DB4D_LogAction action, bool SignificantAction)
{
	VError err = bd->WriteLog(action, this, SignificantAction);

	return err;
}


VError BaseTaskInfo::WriteCreationToLog()
{
	VError err = VE_OK;
	if (bd != nil && !fIsCreationWrittenToLog)
	{
		fIsCreationWrittenToLog = true;
		VStream *log;

		const VValueBag *extra = getContextOwner()->GetExtraData();
		if (extra != NULL)
		{
			VPtrStream tempStream;
			tempStream.SetLittleEndian();
			tempStream.OpenWriting();
			extra->WriteToStream( &tempStream);
			err = tempStream.CloseWriting();
			if (err == VE_OK)
				err = bd->StartWriteLog(DB4D_Log_CreateContextWithExtra, (sLONG)tempStream.GetDataSize(), this, log, false);
			if (err == VE_OK)
			{
				if (log != nil)
				{
					err = log->PutData( tempStream.GetDataPtr(), tempStream.GetDataSize());
				}
				VError err2 = bd->EndWriteLog((sLONG)tempStream.GetDataSize());
				if (err == VE_OK)
					err = err2;
			}
		}
		else
		{
			err = bd->StartWriteLog(DB4D_Log_CreateContextWithUserUUID, sizeof(VUUIDBuffer), this, log, false);
			if (err == VE_OK)
			{
				if (log != nil)
				{
					err = fUserID.WriteToStream(log);
				}
				VError err2 = bd->EndWriteLog(sizeof(VUUIDBuffer));
				if (err == VE_OK)
					err = err2;
			}
		}
	}
	return err;
}


VError BaseTaskInfo::WriteDeletionToLog()
{
	VError err = VE_OK;
	if (bd != nil && fIsCreationWrittenToLog)
	{
		err = bd->WriteLog(DB4D_Log_CloseContext, this, false);
	}
	return err;
}


Transaction*  BaseTaskInfo::StartTransaction(VError& err, sLONG WaitForLockTimer)
{
	Transaction* trans = nil;
	
	err = VE_OK;

	if (IsRemoteLike())
	{
		fRemoteTransactions.push_back(remote_starttrans);
		fRemoteTransactionLevel++;
	}
	else
	{
		bd->StartDataModif(this);
		if (bd->OkToUpdate(err))
		{
			// WriteLog(DB4D_Log_StartTrans, true);

			//fTransactionMutex.Lock();

			if (curtrans == nil)
			{
				curtrans = new Transaction(this, nil, WaitForLockTimer);
				if (curtrans == nil)
					err = ThrowBaseError(memfull, DBaction_StartingTransaction);
			}
			else
			{
				curtrans = curtrans->StartSubTransaction(err, WaitForLockTimer);
			}
			trans = curtrans;

			//fTransactionMutex.Unlock();

			{
				VTaskLock lock(&fcurtransForNeedsByteMutex);
				curtransForNeedsByte = trans;
			}

			if (fSyncHelperContext != nil)
				fSyncHelperContext->StartTransaction();

			bd->ClearUpdating();
		}
		bd->EndDataModif(this);
	}

	if (err != VE_OK)
		err = ThrowBaseError(VE_DB4D_CANNOT_START_TRANSACTION, DBaction_StartingTransaction);

	return(trans);
}


VError BaseTaskInfo::CommitTransaction(void)
{
	Transaction* trans;
	VError err = bd->CommitTransactionOnRemoteDataStores(this);
	
	if (IsRemoteLike())
	{
		fRemoteTransactions.push_back(remote_committrans);
		if (fRemoteTransactionLevel > 0)
			fRemoteTransactionLevel--;
	}
	else
	{
		//WriteLog(DB4D_Log_Commit); // fait dans le commit

		{
			VTaskLock lock(&fcurtransForNeedsByteMutex);
			if (curtrans != nil)
				curtransForNeedsByte = curtrans->GetMother();
		}

		Boolean tryagain = false;

		sLONG sleeptime = 5;
		do 
		{
			//fTransactionMutex.Lock();
			if (curtrans == nil)
			{
				err = VE_OK;
			}
			else
			{
				trans = curtrans->GetMother();
				err = curtrans->Commit(tryagain);
				if (!tryagain)
				{
					delete curtrans;
					curtrans = trans;
				}
			}
			//fTransactionMutex.Unlock();

			if (tryagain)
			{
				VTaskMgr::Get()->GetCurrentTask()->Sleep(sleeptime);
				sleeptime = sleeptime * 2;
				if (sleeptime > 100)
					sleeptime = 100;			
			}

		} while (tryagain);
		if (fSyncHelperContext != nil)
			fSyncHelperContext->CommitTransaction();
	}

	if (fOwner != nil)
		fOwner->SyncThingsToForget(this);

	if (err != VE_OK)
		err = ThrowBaseError(VE_DB4D_CANNOT_COMMIT_TRANSACTION, DBaction_CommittingTransaction);

	return(err);
}


VError BaseTaskInfo::RollBackTransaction(void)
{
	Transaction* trans;
	sLONG nb,i;

	VError err = bd->RollBackTransactionOnRemoteDataStores(this);

	if (IsRemoteLike())
	{
		fRemoteTransactions.push_back(remote_rollbacktrans);
		if (fRemoteTransactionLevel > 0)
			fRemoteTransactionLevel--;
	}
	else
	{
		bd->StartDataModif(this);
		//WriteLog(DB4D_Log_RollBack, true);

		{
			VTaskLock lock(&fcurtransForNeedsByteMutex);
			if (curtrans != nil)
				curtransForNeedsByte = curtrans->GetMother();
		}

		//fTransactionMutex.Lock();
		if (curtrans == nil)
		{
			// err = VE_OK;
		}
		else
		{	
			trans = curtrans->GetMother();
			curtrans->RollBack();
			delete curtrans;
			curtrans = trans;
		}

		if (fSyncHelperContext != nil)
			fSyncHelperContext->RollBackTransaction();

		bd->EndDataModif(this);
		//fTransactionMutex.Unlock();
	}

	if (fOwner != nil)
		fOwner->SyncThingsToForget(this);

	return VE_OK;
}


sLONG BaseTaskInfo::CurrentTransactionLevel() const
{
	if (IsRemoteLike())
	{
		return fRemoteTransactionLevel;
	}
	else
	{
		//VTaskLock lock(&fTransactionMutex);
		if (curtrans == nil)
			return 0;
		else
			return curtrans->CurrentTransactionLevel();
	}
}



Boolean BaseTaskInfo::AddLockingSelection(Selection* sel)
{
	Boolean ok = false;

#if 0
	xbox_assert(sel != nil);
	xbox_assert(sel->GetParentFile()->GetDB() == bd);

	if (sel->GetTypSel() == sel_bitsel)
	{
		BitSel* xsel = dynamic_cast<BitSel*>(sel);
		Bittab* b = xsel->GetBittab();
		DataTable* df = sel->GetParentFile();
		if (df != nil)
		{
			if (df->AddLockSel(b, sel, this))
			{
				ok = true;
			}
		}
		
	}
	else
	{
		ok = true;
	}

	if (ok)
	{
		sel->Retain();
		fLockingSels.AddTail(sel);
	}
#endif

	return ok;
}


void BaseTaskInfo::FinaliseLock(Selection* sel, DB4D_Way_of_Locking HowToLock)
{
	xbox_assert(sel != nil);
	xbox_assert(sel->GetParentFile()->GetDB() == bd);

	sel->Retain();
	fLockingSels.AddTail(sel);
}


Boolean BaseTaskInfo::RemoveLockingSelection(Selection* sel, Boolean RemoveFromList)
{
	xbox_assert(sel != nil);
	xbox_assert(sel->GetParentFile()->GetDB() == bd);
	Boolean ok = true;

#if 0
	occupe();

	xbox_assert(sel != nil);
	if (sel->GetTypSel() == sel_bitsel)
	{
		BitSel* xsel = dynamic_cast<BitSel*>(sel);
		Bittab* b = xsel->GetBittab();
		DataTable* df = sel->GetParentFile();
		if (df != nil)
		{
			if (df->RemoveLockSel(b, sel, this))
			{
				ok = true;
			}
		}

	}

	if (ok)
	{
		fLockingSels.Remove(sel);
		sel->Release();
	}

	libere();
#endif

	return ok;
}


Boolean BaseTaskInfo::IsRecInContext(DataTable* DF, sLONG n)
{
	xbox_assert(DF != nil);
	xbox_assert(DF->GetDB() == bd);
	//VObjLock lock(this);
	RecRef x(DF, n);
	RecordsInMemMap::iterator found = fMapRecs.find(x);
	return found != fMapRecs.end();
}


FicheInMem* BaseTaskInfo::RetainRecAlreadyIn(DataTable* DF, sLONG n)
{
	FicheInMem *fic = nil;
	xbox_assert(DF != nil);
	xbox_assert(DF->GetDB() == bd);

	if (!DF->IsRecordMarkedAsPushed(n))
	{
		//occupe();
		RecRef x(DF, n);
		RecordsInMemMap::iterator found = fMapRecs.find(x);
		if (found != fMapRecs.end())
		{
			fic = found->second;
			fic->Retain();
		}
	}

	//libere();

	return fic;
}


VError BaseTaskInfo::PutRecIn(FicheInMem* fic)
{
	xbox_assert(fic != nil);
	xbox_assert(fic->GetDF()->GetDB() == bd);

	VError err = VE_OK;
	RecRef x(fic->GetDF(), fic->GetNum());


	//occupe();

	fMapRecs.insert(std::pair<RecRef, FicheInMem*>(x, fic));
	//libere();

	return err;
}


void BaseTaskInfo::RemoveRecFrom(DataTable* DF, sLONG n, Boolean SetRecordBackToNew, Boolean CanBeMissing)
{
	xbox_assert(DF != nil);
	xbox_assert(DF->GetDB() == bd);

	//occupe();
	RecRef x(DF, n);
	RecordsInMemMap::iterator found = fMapRecs.find(x);
	if (found != fMapRecs.end())
	{
		if (SetRecordBackToNew)
		{
			found->second->SetBackToNew();
			found->second->ClearSeqNumToken(this);
		}
		fMapRecs.erase(found);
	}
	else
	{
		xbox_assert(CanBeMissing);
	}
	//libere();

}


VError BaseTaskInfo::SetRelationAutoLoadNto1(const Relation* rel, DB4D_Rel_AutoLoadState state)
{
	VError err = VE_OK;

	xbox_assert(rel->GetOwner() == bd);

	//occupe();

	RelationAutoInfoMap::iterator deja = fRelsNto1.find(rel->GetUUIDBuffer());

	if (state == DB4D_Rel_AutoLoad_SameAsStructure)
	{
		if (deja != fRelsNto1.end())
		{
			fAutoRelStamp++;
			fRelsNto1.erase(rel->GetUUIDBuffer());
		}
	}
	else
	{
		if (deja == fRelsNto1.end())
		{
			try
			{
				fAutoRelStamp++;
				fRelsNto1.insert(make_pair(rel->GetUUIDBuffer(), state == DB4D_Rel_AutoLoad));
			}
			catch (...)
			{
				err = rel->ThrowError(memfull, DBaction_ModifyingRelation);
			}
		}
		else
		{
			if (deja->second)
			{
				if (state != DB4D_Rel_AutoLoad)
					fAutoRelStamp++;
			}
			else
			{
				if (state == DB4D_Rel_AutoLoad)
					fAutoRelStamp++;
			}
			deja->second = (state == DB4D_Rel_AutoLoad);
		}
	}

	//libere();

	return err;
}


Boolean BaseTaskInfo::IsRelationAutoLoadNto1(const Relation* rel, Boolean& unknown) const
{
	Boolean res = false;

	xbox_assert(rel->GetOwner() == bd);

	//occupe();

	Table* dest = nil;
	Field* fdest = rel->GetDest();
	if (fdest != nil)
		dest = fdest->GetOwner();

	TableCounterMap::const_iterator found = fExcludedTables.find(dest);
	if (found != fExcludedTables.end() && found->second > 0)
	{
		// comme la table cible est excluse alors la relation n'est pas auto
		RelationAutoInfoMap::const_iterator deja = fRelsNto1.find(rel->GetUUIDBuffer());

		if (deja == fRelsNto1.end())
			unknown = true;
		else
			unknown = false;
	}
	else
	{
		if (fAllRelsNto1AreAuto)
		{
			unknown = false;
			res = true;
		}
		else
		{
			RelationAutoInfoMap::const_iterator deja = fRelsNto1.find(rel->GetUUIDBuffer());
			if (deja == fRelsNto1.end())
			{
				res = rel->IsAutoLoadNto1(nil);
				unknown = true;
			}
			else
			{
				unknown = false;
				res = deja->second;
			}
		}
	}

	//libere();

	return res;
}




VError BaseTaskInfo::SetRelationAutoLoad1toN(const Relation* rel, DB4D_Rel_AutoLoadState state)
{
	VError err = VE_OK;

	xbox_assert(rel->GetOwner() == bd);

	//occupe();

	RelationAutoInfoMap::iterator deja = fRels1toN.find(rel->GetUUIDBuffer());

	if (state == DB4D_Rel_AutoLoad_SameAsStructure)
	{
		if (deja != fRels1toN.end())
		{
			fAutoRelStamp++;
			fRels1toN.erase(rel->GetUUIDBuffer());
		}
	}
	else
	{
		if (deja == fRels1toN.end())
		{
			try
			{
				fAutoRelStamp++;
				fRels1toN.insert(make_pair(rel->GetUUIDBuffer(), state == DB4D_Rel_AutoLoad));
			}
			catch (...)
			{
				err = rel->ThrowError(memfull, DBaction_ModifyingRelation);
			}
		}
		else
		{
			if (deja->second)
			{
				if (state != DB4D_Rel_AutoLoad)
					fAutoRelStamp++;
			}
			else
			{
				if (state == DB4D_Rel_AutoLoad)
					fAutoRelStamp++;
			}
			deja->second = (state == DB4D_Rel_AutoLoad);
		}
	}

	//libere();

	return err;
}


Boolean BaseTaskInfo::IsRelationAutoLoad1toN(const Relation* rel, Boolean& unknown) const
{
	Boolean res = false;

	xbox_assert(rel->GetOwner() == bd);

	//occupe();

	Table* dest = nil;
	Field* fdest = rel->GetDest();
	if (fdest != nil)
		dest = fdest->GetOwner();

	TableCounterMap::const_iterator found = fExcludedTables.find(dest);
	if (found != fExcludedTables.end() && found->second > 0)
	{
		// comme la table cible est excluse alors la relation n'est pas auto
		RelationAutoInfoMap::const_iterator deja = fRels1toN.find(rel->GetUUIDBuffer());

		if (deja == fRels1toN.end())
			unknown = true;
		else
			unknown = false;
	}
	else
	{
		if (fAllRels1toNAreAuto)
		{
			unknown = false;
			res = true;
		}
		else
		{
			RelationAutoInfoMap::const_iterator deja = fRels1toN.find(rel->GetUUIDBuffer());
			if (deja == fRels1toN.end())
			{
				res = rel->IsAutoLoad1toN(nil);
				unknown = true;
			}
			else
			{
				unknown = false;
				res = deja->second;
			}
		}
	}

	//libere();

	return res;
}


void BaseTaskInfo::ExcludeTableFromAutoRelationDestination(Table* inTableToExclude)
{
	xbox_assert(inTableToExclude->GetOwner() == bd);
	//occupe();
	fAutoRelStamp++;
	fExcludedTables[inTableToExclude]++;
	//libere();
}


void BaseTaskInfo::IncludeBackTableToAutoRelationDestination(Table* inTableToInclude)
{
	xbox_assert(inTableToInclude->GetOwner() == bd);
	//occupe();
	TableCounterMap::iterator found = fExcludedTables.find(inTableToInclude);
	if (found == fExcludedTables.end())
	{
		xbox_assert(false);
		//fExcludedTables[inTableToInclude]--;
	}
	else
	{
		fAutoRelStamp++;
		found->second--;
		if (found->second == 0)
		{
			fExcludedTables.erase(found);
		}
	}
	//libere();
}


Boolean BaseTaskInfo::IsTableExcludedFromAutoRelationDestination(Table* inTableToCheck) const
{
	xbox_assert(inTableToCheck->GetOwner() == bd);
	Boolean res = false;
	//occupe();
	TableCounterMap::const_iterator found = fExcludedTables.find(inTableToCheck);
	if (found != fExcludedTables.end())
	{
		res = found->second > 0;
	}
	//libere();
	return res;
}


const VValueBag* BaseTaskInfo::RetainExtraData() const
{
	if (fParent == nil)
		return nil;
	return getContextOwner()->RetainExtraData();
}


VJSONObject* BaseTaskInfo::BuildLockInfo()
{
	VJSONObject* result = new VJSONObject();
	VError err = VE_OK;
	VString s;

	if (fParent != nil)
	{
		fParent->GetID().GetString(s);
		result->SetPropertyAsString("contextID", s);
	}
	if (!fUserID.IsNull())
	{
		fUserID.GetString(s);
		result->SetPropertyAsString("userID", s);
	}
	if (!fUser4DSessionID.IsNull())
	{
		fUser4DSessionID.GetString(s);
		result->SetPropertyAsString("sessionID", s);
	}
	const VValueBag* extradata = RetainExtraData();
	if (extradata != nil)
	{
		VJSONObject* obj = extradata->BuildJSONObject(err);
		if (obj != nil)
		{
			result->SetProperty("contextAttributes", VJSONValue(obj));
			obj->Release();
		}
		extradata->Release();
	}
	return result;
}



void BaseTaskInfo::MustNotCheckRefInt(const Relation* rel, const FicheInMem* rec)
{
	RefIntID ref(rel, rec);
	try
	{
		fMustNotCheck.insert(ref);
	}
	catch (...)
	{
	}
}

void BaseTaskInfo::CheckRefIntAgain(const Relation* rel, const FicheInMem* rec)
{
	RefIntID ref(rel, rec);
	fMustNotCheck.erase(ref);
}


Boolean BaseTaskInfo::CanCheckRefInt(const Relation* rel, const FicheInMem* rec)
{
	RefIntID ref(rel, rec);
	return fMustNotCheck.find(ref) == fMustNotCheck.end();
}



void BaseTaskInfo::MustNotCheckRefIntOnForeignKey(const Relation* rel, const FicheInMem* rec)
{
	RefIntID ref(rel, rec);
	try
	{
		fMustNotCheckForeignKey.insert(ref);
	}
	catch (...)
	{
	}
}

void BaseTaskInfo::CheckRefIntAgainOnForeignKey(const Relation* rel, const FicheInMem* rec)
{
	RefIntID ref(rel, rec);
	fMustNotCheckForeignKey.erase(ref);
}


Boolean BaseTaskInfo::CanCheckRefIntOnForeignKey(const Relation* rel, const FicheInMem* rec)
{
	RefIntID ref(rel, rec);
	return fMustNotCheckForeignKey.find(ref) == fMustNotCheckForeignKey.end();
}


StructObjLockPtr BaseTaskInfo::TryToLock(const StructObjRef& obj, sLONG TimeOut, const VValueBag **outLockerExtraData)
{
	Boolean ForReadOnly = obj.IsForReadOnly();
	Boolean ok = false;
	Boolean stop = false;
	sLONG sleeptime = 5;
	uLONG startmillisec = VSystem::GetCurrentTime();

	while (!stop)
	{
		{
			VTaskLock lock(&sStructLockersMutex);
			StructLockerMap::iterator found;

			if (obj.fBaseOwner != nil)
			{
				StructObjRef baseowner(obj.fBaseOwner);
				found = sStructLockers.find(baseowner);
				if (found != sStructLockers.end())
				{
					if (found->first.GetOwner() != obj.GetOwner())
					{
						if (found->first.IsForReadOnly() && ForReadOnly)
						{
						}
						else
						{
							stop = true;
							if (outLockerExtraData != NULL)
							{
								const VValueBag *bag = found->first.GetOwner()->RetainExtraData();
								CopyRefCountable( outLockerExtraData, bag);
								ReleaseRefCountable( &bag);
							}
						}
					}
				}
			}

			if ((!stop) && obj.fTableOwner != nil)
			{
				StructObjRef tableowner(obj.fTableOwner);
				found = sStructLockers.find(tableowner);
				if (found != sStructLockers.end())
				{
					if (found->first.GetOwner() != obj.GetOwner())
					{
						if (found->first.AlsoLockFields())
						{
							if (found->first.IsForReadOnly() && ForReadOnly)
							{
							}
							else
							{
								stop = true;
								if (outLockerExtraData != NULL)
								{
									const VValueBag *bag = found->first.GetOwner()->RetainExtraData();
									CopyRefCountable( outLockerExtraData, bag);
									ReleaseRefCountable( &bag);
								}
							}
						}
					}
				}
			}

			if (!stop)
			{
				found = sStructLockers.find(obj);
				if (found == sStructLockers.end())
				{
					try
					{
						sStructLockers.insert(make_pair(obj, 1));
						ok = true;
						obj.GetOwner()->AddStructLocker(obj);
						if (outLockerExtraData != NULL)
						{
							const VValueBag *bag = obj.GetOwner()->RetainExtraData();
							CopyRefCountable( outLockerExtraData, bag);
							ReleaseRefCountable( &bag);
						}
					}
					catch (...)
					{
						ok = false;
					}
				}
				else
				{
					if (found->first.GetOwner() == obj.GetOwner())
					{
						found->second++;
						ok =true;
						xbox_assert(found->first.fLockerType == obj.fLockerType);
					}
					else
					{
						if (found->first.IsForReadOnly() && ForReadOnly)
						{
							found->second++;
							ok =true;
						}
					}

					if (outLockerExtraData != NULL)
					{
						const VValueBag *bag = found->first.GetOwner()->RetainExtraData();
						CopyRefCountable( outLockerExtraData, bag);
						ReleaseRefCountable( &bag);
					}
				}
			}
		}

		if (ok)
			stop = true;
		else
		{
			stop = false;
			if (TimeOut == 0)
				stop = true;
			else
			{
				if (TimeOut == -1)
				{
					VTaskMgr::Get()->GetCurrentTask()->Sleep(sleeptime);
					sleeptime = sleeptime * 2;
					if (sleeptime > 100)
						sleeptime = 100;
				}
				else
				{
					uLONG currentime = VSystem::GetCurrentTime();
					sLONG passedtime = currentime - startmillisec;
					if ( passedtime < TimeOut)
					{
						sLONG remaintime = TimeOut - passedtime;
						if (sleeptime > remaintime)
							sleeptime = remaintime;
						VTaskMgr::Get()->GetCurrentTask()->Sleep(sleeptime);
						sleeptime = sleeptime * 2;
						if (sleeptime > 100)
							sleeptime = 100;
					}
					else
						stop = true;
				}

			}
		}

	}

	if (ok)
	{
		return (StructObjLockPtr)obj.fStructObj;
	}
	else
		return nil;
}


VError BaseTaskInfo::TryToUnLock(const StructObjRef& obj)
{
	if (testAssert(obj.fStructObj != nil))
	{
		Boolean ok = false;
		{
			VTaskLock lock(&sStructLockersMutex);
			StructLockerMap::iterator found = sStructLockers.find(obj);
			if (testAssert(found != sStructLockers.end()))
			{
				if ((found->first.GetOwner() == obj.GetOwner()) || found->first.IsForReadOnly())
				{
					found->second--;
					xbox_assert(found->second >= 0);
					if (found->second <= 0)
					{
						obj.GetOwner()->RemoveStructLocker(obj);
						sStructLockers.erase(found);
					}
					ok = true;
				}
				else
					return VE_DB4D_WRONG_OWNER;
			}
			else
				return VE_DB4D_OBJECT_WAS_NOT_LOCKED;		
		}
		return VE_OK;
	}
	else
		return VE_DB4D_OBJECT_IS_NULL;
}


VError BaseTaskInfo::AddStructLocker(const StructObjRef& obj)
{
	VTaskLock lock(&fStructLockersMutex);
	try
	{
		xbox_assert(fStructLockers.find(obj) == fStructLockers.end());
		fStructLockers.insert(obj);
		return VE_OK;
	}
	catch (...)
	{
		return bd->ThrowError(memfull, DBaction_AddingALock);
	}
}


VError BaseTaskInfo::RemoveStructLocker(const StructObjRef& obj)
{
	VTaskLock lock(&fStructLockersMutex);
	xbox_assert(fStructLockers.find(obj) != fStructLockers.end());
	fStructLockers.erase(obj);
	return VE_OK;
}



StructObjLockPtr BaseTaskInfo::LockDataBaseDef( const Base4D* inBase, sLONG inTimeOut, Boolean ForReadOnly, const VValueBag **outLockerExtraData)
{
	StructObjLockPtr lockPtr = NULL;

	if (inBase != NULL)
	{
		if(bd->IsRemote())
		{
			//if (fEncapsuleur != NULL)
			{
				IRequest *req = bd->CreateRequest( this, Req_LockDataBaseDef + kRangeReqDB4D);
				if (req != NULL)
				{
					VValueBag lockDef;
					DB4DRemoteBagKeys::lock_timeout.Set( &lockDef, inTimeOut);
					DB4DRemoteBagKeys::lock_for_readonly.Set( &lockDef, ForReadOnly);
					
					req->PutBaseParam( inBase);
					req->PutValueBagParam( lockDef);
			
					VError err = req->GetLastError();
					if (err == VE_OK)
					{
						err = req->Send();
						if (err == VE_OK)
						{
							//SH C4312 sLONG --> LONG_PTR
							sLONG_PTR lockRef = req->GetLongReply( err);
							if (err == VE_OK)
							{
								lockPtr = (StructObjLockPtr) lockRef;
								// read the locker extra data
								Boolean haveBag = req->GetBooleanReply( err);
								if (err == VE_OK && haveBag)
								{
									const VValueBag *bag = req->RetainValueBagReply( err);
									if (err == VE_OK && outLockerExtraData != NULL)
										CopyRefCountable( outLockerExtraData, bag);
									ReleaseRefCountable( &bag);
								}
							}
						}
					}
					req->Release();
				}
			}
		}
		else
		{
			StructObjRef locker(inBase, this, ForReadOnly);
			lockPtr = TryToLock(locker, inTimeOut, outLockerExtraData);
		}
	}
	return lockPtr;
}


StructObjLockPtr BaseTaskInfo::LockTableDef(const Table* inTable, Boolean inWithFields, sLONG inTimeOut, Boolean ForReadOnly, const VValueBag **outLockerExtraData)
{
	StructObjLockPtr lockPtr = NULL;

	if (inTable != NULL)
	{
		if (bd->IsRemote())
		{
			//if (fEncapsuleur != NULL)
			{
				IRequest *req = bd->CreateRequest( this, Req_LockTableDef + kRangeReqDB4D);
				if (req != NULL)
				{
					VValueBag lockDef;
					DB4DRemoteBagKeys::lock_with_fields.Set( &lockDef, inWithFields);
					DB4DRemoteBagKeys::lock_timeout.Set( &lockDef, inTimeOut);
					DB4DRemoteBagKeys::lock_for_readonly.Set( &lockDef, ForReadOnly);
					
					req->PutBaseParam( inTable->GetOwner());
					req->PutTableParam( inTable);
					req->PutValueBagParam( lockDef);
			
					VError err = req->GetLastError();
					if (err == VE_OK)
					{
						err = req->Send();
						if (err == VE_OK)
						{
							//SH C4312 sLONG --> LONG_PTR
							sLONG_PTR lockRef = req->GetLongReply( err);
							if (err == VE_OK)
							{
								lockPtr = (StructObjLockPtr) lockRef;
								// read the locker extra data
								Boolean haveBag = req->GetBooleanReply( err);
								if (err == VE_OK && haveBag)
								{
									const VValueBag *bag = req->RetainValueBagReply( err);
									if (err == VE_OK && outLockerExtraData != NULL)
										CopyRefCountable( outLockerExtraData, bag);
									ReleaseRefCountable( &bag);
								}
							}
						}
					}
					req->Release();
				}
			}
		}
		else
		{
			StructObjRef locker(inTable, inWithFields, this, inTable->GetOwner(), ForReadOnly);
			lockPtr = TryToLock(locker, inTimeOut, outLockerExtraData);
		}
	}
	return lockPtr;
}


StructObjLockPtr BaseTaskInfo::LockFieldDef(const Field* inField, sLONG inTimeOut, Boolean ForReadOnly, const VValueBag **outLockerExtraData)
{
	StructObjLockPtr lockPtr = NULL;

	if (inField != NULL)
	{
		if (bd->IsRemote())
		{
			//if (fEncapsuleur != NULL)
			{
				IRequest *req = bd->CreateRequest( this, Req_LockFieldDef + kRangeReqDB4D);
				if (req != NULL)
				{
					VValueBag lockDef;
					DB4DRemoteBagKeys::lock_timeout.Set( &lockDef, inTimeOut);
					DB4DRemoteBagKeys::lock_for_readonly.Set( &lockDef, ForReadOnly);
					
					req->PutBaseParam( inField->GetOwner()->GetOwner());
					req->PutTableParam( inField->GetOwner());
					req->PutFieldParam( inField);
					req->PutValueBagParam( lockDef);
			
					VError err = req->GetLastError();
					if (err == VE_OK)
					{
						err = req->Send();
						if (err == VE_OK)
						{
							//SH C4312 sLONG --> LONG_PTR
							sLONG_PTR lockRef = req->GetLongReply( err);
							if (err == VE_OK)
							{
								lockPtr = (StructObjLockPtr) lockRef;
								// read the locker extra data
								Boolean haveBag = req->GetBooleanReply( err);
								if (err == VE_OK && haveBag)
								{
									const VValueBag *bag = req->RetainValueBagReply( err);
									if (err == VE_OK && outLockerExtraData != NULL)
										CopyRefCountable( outLockerExtraData, bag);
									ReleaseRefCountable( &bag);
								}
							}
						}
					}
					req->Release();
				}
			}
		}
		else
		{
			StructObjRef locker(inField, this, inField->GetOwner(), inField->GetOwner()->GetOwner(), ForReadOnly);
			lockPtr = TryToLock(locker, inTimeOut, outLockerExtraData);
		}
	}
	return lockPtr;
}


StructObjLockPtr BaseTaskInfo::LockRelationDef(const Relation* inRelation, Boolean inWithRelatedFields, sLONG inTimeOut, Boolean ForReadOnly, const VValueBag **outLockerExtraData)
{
	StructObjLockPtr lockret = nil;

	if (inRelation != NULL)
	{
		if (bd->IsRemote())
		{
			//if (fEncapsuleur != NULL)
			{
				IRequest *req = bd->CreateRequest( this, Req_LockRelationDef + kRangeReqDB4D);
				if (req != NULL)
				{
					VValueBag lockDef;
					DB4DRemoteBagKeys::lock_with_relatedfields.Set( &lockDef, inWithRelatedFields);
					DB4DRemoteBagKeys::lock_timeout.Set( &lockDef, inTimeOut);
					DB4DRemoteBagKeys::lock_for_readonly.Set( &lockDef, ForReadOnly);
					
					req->PutBaseParam( inRelation->GetOwner());
					req->PutRelationParam( inRelation);
					req->PutValueBagParam( lockDef);
			
					VError err = req->GetLastError();
					if (err == VE_OK)
					{
						err = req->Send();
						if (err == VE_OK)
						{
							//SH C4312 sLONG --> LONG_PTR
							sLONG_PTR lockRef = req->GetLongReply( err);
							if (err == VE_OK)
							{
								lockret = (StructObjLockPtr) lockRef;
								// read the locker extra data
								Boolean haveBag = req->GetBooleanReply( err);
								if (err == VE_OK && haveBag)
								{
									const VValueBag *bag = req->RetainValueBagReply( err);
									if (err == VE_OK && outLockerExtraData != NULL)
										CopyRefCountable( outLockerExtraData, bag);
									ReleaseRefCountable( &bag);
								}
							}
						}
					}
					req->Release();
				}
			}
		}
		else
		{
			StructObjLockPtr locker1 = nil;
			StructObjLockPtr locker2 = nil;
			Boolean ok = false;

			if (inWithRelatedFields)
			{
				const VValueBag *sourceLockerExtra = NULL, *destLockerExtra = NULL;
				locker1 = LockFieldDef(inRelation->GetSource(), inTimeOut, ForReadOnly, &sourceLockerExtra);
				locker2 = LockFieldDef(inRelation->GetDest(), inTimeOut, ForReadOnly, &destLockerExtra);
				ok = locker1 != nil && locker2 != nil;
				if (!ok)
				{
					if (locker1 != nil)
					{
						UnLockStructObject(locker1);
					}
					else if (outLockerExtraData != NULL)
					{
						CopyRefCountable( outLockerExtraData, sourceLockerExtra);
					}

					if (locker2 != nil)
					{
						UnLockStructObject(locker2);
					}
					else if(outLockerExtraData != NULL && *outLockerExtraData != NULL)
					{
						CopyRefCountable( outLockerExtraData, destLockerExtra);
					}
				}
				ReleaseRefCountable( &sourceLockerExtra);
				ReleaseRefCountable( &destLockerExtra);
			}
			else
			{
				ok = true;
			}

			if (ok)
			{
				StructObjRef locker(inRelation, this, inRelation->GetOwner(), ForReadOnly);
				lockret = TryToLock(locker, inTimeOut, outLockerExtraData);
				if (lockret == nil && inWithRelatedFields)
				{
					UnLockStructObject(locker1);
					UnLockStructObject(locker2);
				}
			}
		}
	}
	return lockret;
}


StructObjLockPtr BaseTaskInfo::LockIndexDef(const IndexInfo* inIndex, sLONG inTimeOut, Boolean ForReadOnly, const VValueBag **outLockerExtraData)
{
	StructObjLockPtr lockret = nil;

	if (inIndex != NULL)
	{
		if (bd->IsRemote())
		{
			//if (fEncapsuleur != NULL)
			{
				IRequest *req = bd->CreateRequest( this, Req_LockIndexDef + kRangeReqDB4D);
				if (req != NULL)
				{
					VValueBag lockDef;
					DB4DRemoteBagKeys::lock_timeout.Set( &lockDef, inTimeOut);
					DB4DRemoteBagKeys::lock_for_readonly.Set( &lockDef, ForReadOnly);
					
					req->PutBaseParam( inIndex->GetDB());
					req->PutIndexParam( inIndex);
					req->PutValueBagParam( lockDef);
			
					VError err = req->GetLastError();
					if (err == VE_OK)
					{
						err = req->Send();
						if (err == VE_OK)
						{
							//SH C4312 sLONG --> LONG_PTR
							sLONG_PTR lockRef = req->GetLongReply( err);
							if (err == VE_OK)
							{
								lockret = (StructObjLockPtr) lockRef;
								// read the locker extra data
								const VValueBag *extraData = req->RetainValueBagReply( err);
								if (err == VE_OK)
									CopyRefCountable( outLockerExtraData, extraData);
								ReleaseRefCountable( &extraData);
							}
						}
					}
					req->Release();
				}
			}
		}
		else
		{
			StructObjRef locker(inIndex, this, inIndex->GetDB(), ForReadOnly);
			return TryToLock(locker, inTimeOut, outLockerExtraData);
		}
	}
	return lockret;
}


VError BaseTaskInfo::UnLockStructObject(StructObjLockPtr inLocker)
{
	VError error = VE_OK;
	
	if (bd->IsRemote())
	{
		//if (fEncapsuleur != NULL)
		{
			IRequest *req = bd->CreateRequest( this, Req_UnlockObjectDef + kRangeReqDB4D);
			if (req != NULL)
			{
				//SH C4312 sLONG --> LONG_PTR
				sLONG_PTR lockRef = (sLONG_PTR) inLocker;

				req->PutBaseParam( bd);
				req->PutLongParam( lockRef);
		
				VError err = req->GetLastError();
				if (err == VE_OK)
				{
					err = req->Send();
					if (err == VE_OK)
					{
						sBYTE reply = req->GetByteReply( err);
						if (err == VE_OK)
						{
							if (reply == 0)
								err = VE_DB4D_CANNOT_UNLOCK_STRUCT_OBJECTDEF;
						}
					}
				}
				req->Release();
			}
		}
	}
	else
	{
		StructObjRef locker((void*)inLocker, 0, 0, this, false);
		error = TryToUnLock(locker);
	}
	return error;
}


VError BaseTaskInfo::LockDataBaseDefWithBag( const Base4D* inBase, const VValueBag& inLockDefinition, sLONG& outLockRef, const VValueBag **outLockerExtraData)
{
	VError error = VE_OK;
	StructObjLockPtr lockPtr = NULL;

	if (inBase != NULL)
	{
		sLONG timeOut = DB4DRemoteBagKeys::lock_timeout.Get( &inLockDefinition);
		Boolean forReadOnly = DB4DRemoteBagKeys::lock_for_readonly.Get( &inLockDefinition);
		lockPtr = LockDataBaseDef( inBase, timeOut, forReadOnly, outLockerExtraData);
	}

	if (lockPtr == NULL)
	{
		error = VE_DB4D_CANNOT_LOCK_STRUCT_OBJECTDEF;
	}
	else
	{
		outLockRef = _GetNextStructLockRef();
		fStructLockRefMap[outLockRef] = lockPtr;
	}
	return error;
}


VError BaseTaskInfo::LockTableDefWithBag( const Table* inTable, const VValueBag& inLockDefinition, sLONG& outLockRef, const VValueBag **outLockerExtraData)
{
	VError error = VE_OK;
	StructObjLockPtr lockPtr = NULL;

	if (inTable != NULL)
	{
		Boolean withFields = DB4DRemoteBagKeys::lock_with_fields.Get( &inLockDefinition);
		sLONG timeOut = DB4DRemoteBagKeys::lock_timeout.Get( &inLockDefinition);
		Boolean forReadOnly = DB4DRemoteBagKeys::lock_for_readonly.Get( &inLockDefinition);
		lockPtr = LockTableDef( inTable, withFields, timeOut, forReadOnly, outLockerExtraData);
	}

	if (lockPtr == NULL)
	{
		error = VE_DB4D_CANNOT_LOCK_STRUCT_OBJECTDEF;
	}
	else
	{
		outLockRef = _GetNextStructLockRef();
		fStructLockRefMap[outLockRef] = lockPtr;
	}
	return error;
}


VError BaseTaskInfo::LockFieldDefWithBag( const Field* inField, const VValueBag& inLockDefinition, sLONG& outLockRef, const VValueBag **outLockerExtraData)
{
	VError error = VE_OK;
	StructObjLockPtr lockPtr = NULL;

	if (inField != NULL)
	{
		sLONG timeOut = DB4DRemoteBagKeys::lock_timeout.Get( &inLockDefinition);
		Boolean forReadOnly = DB4DRemoteBagKeys::lock_for_readonly.Get( &inLockDefinition);
		lockPtr = LockFieldDef( inField, timeOut, forReadOnly, outLockerExtraData);
	}

	if (lockPtr == NULL)
	{
		error = VE_DB4D_CANNOT_LOCK_STRUCT_OBJECTDEF;
	}
	else
	{
		outLockRef = _GetNextStructLockRef();
		fStructLockRefMap[outLockRef] = lockPtr;
	}
	return error;
}


VError BaseTaskInfo::LockRelationDefWithBag( const Relation* inRelation, const VValueBag& inLockDefinition, sLONG& outLockRef, const VValueBag **outLockerExtraData)
{
	VError error = VE_OK;
	StructObjLockPtr lockPtr = NULL;

	if (inRelation != NULL)
	{
		Boolean withRelatedFields = DB4DRemoteBagKeys::lock_with_relatedfields.Get( &inLockDefinition);
		sLONG timeOut = DB4DRemoteBagKeys::lock_timeout.Get( &inLockDefinition);
		Boolean forReadOnly = DB4DRemoteBagKeys::lock_for_readonly.Get( &inLockDefinition);
		lockPtr = LockRelationDef( inRelation, withRelatedFields, timeOut, forReadOnly, outLockerExtraData);
	}

	if (lockPtr == NULL)
	{
		error = VE_DB4D_CANNOT_LOCK_STRUCT_OBJECTDEF;
	}
	else
	{
		outLockRef = _GetNextStructLockRef();
		fStructLockRefMap[outLockRef] = lockPtr;
	}
	return error;
}


VError BaseTaskInfo::LockIndexDefWithBag( const IndexInfo* inIndex, const VValueBag& inLockDefinition, sLONG& outLockRef, const VValueBag **outLockerExtraData)
{
	VError error = VE_OK;
	StructObjLockPtr lockPtr = NULL;

	if (inIndex != NULL)
	{
		sLONG timeOut = DB4DRemoteBagKeys::lock_timeout.Get( &inLockDefinition);
		Boolean forReadOnly = DB4DRemoteBagKeys::lock_for_readonly.Get( &inLockDefinition);
		lockPtr = LockIndexDef( inIndex, timeOut, forReadOnly, outLockerExtraData);
	}

	if (lockPtr == NULL)
	{
		error = VE_DB4D_CANNOT_LOCK_STRUCT_OBJECTDEF;
	}
	else
	{
		outLockRef = _GetNextStructLockRef();
		fStructLockRefMap[outLockRef] = lockPtr;
	}
	return error;
}


VError BaseTaskInfo::UnLockStructObject( sLONG inLockRef)
{
	VError error = VE_OK;
	StructObjLockPtr lockPtr = NULL;

	StructLockRefMap::iterator iter = fStructLockRefMap.find( inLockRef);
	if (iter != fStructLockRefMap.end())
		lockPtr = iter->second;

	if (lockPtr == NULL)
	{
		error = VE_DB4D_OBJECT_WAS_NOT_LOCKED;
	}
	else
	{
		error = UnLockStructObject( lockPtr);
		if (error == VE_OK)
			iter->second = NULL;
	}
	return error;
}


const sLONG kFirstStructLockRef = 0x100;

sLONG BaseTaskInfo::_GetNextStructLockRef() const
{
	sLONG lockRef = 0;

	// find a free slot
	StructLockRefMap::const_iterator iter = fStructLockRefMap.begin();
	while (iter != fStructLockRefMap.end())
	{
		if (iter->second == NULL)
			break;
		++iter;
	}
	if (iter == fStructLockRefMap.end())
	{
		StructLockRefMap::const_reverse_iterator r_iter = fStructLockRefMap.rbegin();	// take the last element
		if (r_iter != fStructLockRefMap.rend())
			lockRef = r_iter->first + 1;
		else
			lockRef = kFirstStructLockRef;
	}
	else
	{
		lockRef = iter->first;
	}
	return lockRef;
}


Boolean BaseTaskInfo::NeedsBytes(sLONG allocationBlockNumber, VSize needed, VSize& total)
{
	VTaskLock lock(&sGlobIDMutex);
	for (ContextCollection::iterator cur = sAllContexts.begin(), end = sAllContexts.end(); cur != end/* && total < needed*/; cur++)
	{
		BaseTaskInfo* context = *cur;
		Transaction* trans = context->GetCurrentTransactionForNeedsByte();
		if (trans != nil)
		{
			trans->NeedsBytes(allocationBlockNumber, needed, total);
			trans->libere();
		}
	}

	return /*total > needed*/ true;
}


void BaseTaskInfo::AddDeletedIndexIDInAllContexts(const VUUIDBuffer& inID)
{
	VTaskLock lock(&sGlobIDMutex);
	for (ContextCollection::iterator cur = sAllContexts.begin(), end = sAllContexts.end(); cur != end; cur++)
	{
		BaseTaskInfo* context = *cur;
		context->AddDeletedIndexID(inID);
	}
}


void BaseTaskInfo::InvalidateTransaction()
{
	//fTransactionMutex.Lock();
	if (curtrans != nil)
		curtrans->InvalidateTransaction();

	//fTransactionMutex.Unlock();
}

void BaseTaskInfo::InvalidateAllTransactions(Base4D* bd)
{
	VTaskLock lock(&sGlobIDMutex);
	for (ContextCollection::iterator cur = sAllContexts.begin(), end = sAllContexts.end(); cur != end; cur++)
	{
		BaseTaskInfo* context = *cur;
		if (context->GetBase() == bd)
			context->InvalidateTransaction();
	}

}



VError BaseTaskInfo::GetLoadedRecords(DataTable* df, Bittab& outLoadedOnes, Bittab* filtre)
{
	VError err = VE_OK;

	for (RecordsInMemMap::iterator cur = fMapRecs.begin(), end = fMapRecs.end(); cur != end && err == VE_OK; cur++)
	{
		if (cur->first.first == df)
		{
			if (filtre != nil && filtre->isOn(cur->first.second))
				err = outLoadedOnes.Set(cur->first.second, true);
		}
	}

	return err;
}


#if 0
FicheInMem* BaseTaskInfo::RetainRemoteCachedRecord(Table* inTable, RecIDType recnum)
{
	FicheInMem* result = nil;

	for (vector<RemoteRecordCache*>::iterator cur = fRemoteRecordCaches.begin(), end = fRemoteRecordCaches.end(); cur != end; cur++)
	{
		//result = (*cur)->RetainRecord(inTable, recnum);
		if (result != nil)
			break;
	}
	return result;
}
#endif


VError BaseTaskInfo::PutAutoRelInfoToStream(VStream* into)
{
	VError err = into->PutByte(fAllRelsNto1AreAuto ? 1 : 0);
	if (err == VE_OK)
		err = into->PutByte(fAllRels1toNAreAuto ? 1 : 0);
	if (err == VE_OK)
		err = into->PutLong((sLONG)fRelsNto1.size());
	for (RelationAutoInfoMap::iterator cur = fRelsNto1.begin(), end = fRelsNto1.end(); cur != end && err == VE_OK; cur++)
	{
		VUUID xid(cur->first);
		err = xid.WriteToStream(into);
		if (err == VE_OK)
			err = into->PutByte(cur->second ? 1 : 0);
	}

	if (err == VE_OK)
		err = into->PutLong((sLONG)fRels1toN.size());
	for (RelationAutoInfoMap::iterator cur = fRels1toN.begin(), end = fRels1toN.end(); cur != end && err == VE_OK; cur++)
	{
		VUUID xid(cur->first);
		err = xid.WriteToStream(into);
		if (err == VE_OK)
			err = into->PutByte(cur->second ? 1 : 0);
	}

	if (err == VE_OK)
		err = into->PutLong((sLONG)fExcludedTables.size());
	for (TableCounterMap::iterator cur = fExcludedTables.begin(), end = fExcludedTables.end(); cur != end && err == VE_OK; cur++)
	{
		VUUID xid;
		cur->first->GetUUID(xid);
		err = xid.WriteToStream(into);
		if (err == VE_OK)
			err = into->PutLong(cur->second);
	}

	return err;
}



VError BaseTaskInfo::GetAutoRelInfoFromStream(VStream* from)
{
	uBYTE cc;
	VError err = from->GetByte(cc);
	fAllRelsNto1AreAuto = cc == 1;
	if (err == VE_OK)
		err = from->GetByte(cc);
	fAllRels1toNAreAuto = cc == 1;

	fRelsNto1.clear();
	fRels1toN.clear();
	fExcludedTables.clear();
	VUUID xid;
	uBYTE bb;
	sLONG nb = 0;
	if (err == VE_OK)
		err = from->GetLong(nb);
	for (sLONG i = 0; i< nb && err == VE_OK; i++)
	{
		err = xid.ReadFromStream(from);
		if (err == VE_OK)
			err = from->GetByte(bb);
		if (err == VE_OK)
		{
			try
			{
				fRelsNto1[xid.GetBuffer()] = (bb == 1);
			}
			catch (...)
			{
				err = ThrowBaseError(memfull, noaction);
			}
		}

	}

	if (err == VE_OK)
		err = from->GetLong(nb);
	for (sLONG i = 0; i< nb && err == VE_OK; i++)
	{
		err = xid.ReadFromStream(from);
		if (err == VE_OK)
			err = from->GetByte(bb);
		if (err == VE_OK)
		{
			try
			{
				fRels1toN[xid.GetBuffer()] = (bb == 1);
			}
			catch (...)
			{
				err = ThrowBaseError(memfull, noaction);
			}
		}

	}

	if (err == VE_OK)
		err = from->GetLong(nb);
	for (sLONG i = 0; i< nb && err == VE_OK; i++)
	{
		sLONG count;
		err = xid.ReadFromStream(from);
		if (err == VE_OK)
			err = from->GetLong(count);
		if (err == VE_OK)
		{
			Table* tt = bd->FindAndRetainTableRef(xid);
			if (tt != nil)
			{
				try
				{
					fExcludedTables[tt] = count;
				}
				catch (...)
				{
					err = ThrowBaseError(memfull, noaction);
				}
				tt->Release();
			}
		}

	}

	return err;
}


void BaseTaskInfo::AutoRelInfoWasSent()
{
	fRemoteAutoRelStamp = fAutoRelStamp;
}


void BaseTaskInfo::UnlockAllLoadedRecs(DataTable* df)
{
	vector<RecRef> toRemove;

	for (RecordsInMemMap::iterator cur = fMapRecs.begin(), end = fMapRecs.end(); cur != end; cur++)
	{
		if (cur->first.first == df)
		{
			FicheInMem* rec = cur->second;
			rec->RevertToReadOnly(this);
			toRemove.push_back(cur->first);
		}
	}

	for (vector<RecRef>::iterator cur = toRemove.begin(), end = toRemove.end(); cur != end; cur++)
	{
		fMapRecs.erase(*cur);
	}
}


void BaseTaskInfo::AddReleasedRecID(const DB4D_RecordRef& inID)
{
	VTaskLock lock(&fServerKeptRecordsMutex);
	fClientReleasedRecIDs.push_back(inID);
}


void BaseTaskInfo::StealListOfReleasedRecIDs(vector<DB4D_RecordRef>& outList)
{
	VTaskLock lock(&fServerKeptRecordsMutex);
	outList = fClientReleasedRecIDs;
	fClientReleasedRecIDs.clear();
}


bool BaseTaskInfo::HasSomeReleasedObjects()
{
	VTaskLock lock(&fServerKeptRecordsMutex);
	return HasRemoteTransPending() || !fClientReleasedRecIDs.empty();
}




VIntlMgr* GetContextIntl(const BaseTaskInfo* context)
{
	if (context == nil)
		return VTask::GetCurrentIntlManager();
	else
		return context->GetIntlMgr();
}


UniChar GetWildChar(const BaseTaskInfo* context)
{
	VIntlMgr* intl = GetContextIntl(context);
	return intl->GetCollator()->GetWildChar();
}







Boolean BaseTaskInfo::MatchBaseInContext(CDB4DBaseContextPtr InBaseContext) const
{
	if (GetBase() == dynamic_cast<BaseTaskInfo*>(InBaseContext)->GetBase()) 
		return true;
	else 
		return false;
}


void BaseTaskInfo::SetTimerOnRecordLocking(sLONG WaitForLockTimer)
{
	SetLockTimer(WaitForLockTimer);
}


sLONG BaseTaskInfo::GetTimerOnRecordLocking() const
{
	return GetLockTimer();
}


VError BaseTaskInfo::StartTransaction(sLONG WaitForLockTimer)
{
	VError err;
	err = bd->StartTransactionOnRemoteDataStores(this);
	StartTransaction(err, WaitForLockTimer);
	return err;
}

/*
VError VDB4DBaseContext::CommitTransaction()
{		
	VError err = fBase.CommitTransaction();
	fOwner->SyncThingsToForget(this);
	return err;
}


VError VDB4DBaseContext::RollBackTransaction()
{
	fBase.RollBackTransaction();
	fOwner->SyncThingsToForget(this);
	return VE_OK;
}
*/

/*
sLONG BaseTaskInfo::CurrentTransactionLevel() const
{
	return CurrentTransactionLevel();
}
*/

VError BaseTaskInfo::ReleaseFromConsistencySet(CDB4DSelection* InSel)
{

	return VE_OK;
}


VError BaseTaskInfo::ReleaseFromConsistencySet(CDB4DSet* InSet)
{

	return VE_OK;
}


VError BaseTaskInfo::ReleaseFromConsistencySet(RecIDType inRecordID)
{

	return VE_OK;
}


VError BaseTaskInfo::ReleaseFromConsistencySet(CDB4DRecord* inRec)
{

	return VE_OK;
}


VError BaseTaskInfo::SetConsistency(Boolean isOn)
{

	return VE_OK;
}


CDB4DSqlQuery* BaseTaskInfo::NewSqlQuery(VString& request, VError& err)
{
	CDB4DSqlQuery* sq = new VDB4DSqlQuery(VDBMgr::GetManager(), GetBase(), this, request, err);
	return sq;
}


VError BaseTaskInfo::SetRelationAutoLoadNto1(const CDB4DRelation* inRel, DB4D_Rel_AutoLoadState inAutoLoadState)
{
	if (inRel == nil)
		return VE_DB4D_WRONG_RELATIONREF;
	else
	{
		const Relation* rel = dynamic_cast<const VDB4DRelation*>(inRel)->GetRel();
		return SetRelationAutoLoadNto1(rel, inAutoLoadState);
	}
}


VError BaseTaskInfo::SetRelationAutoLoad1toN(const CDB4DRelation* inRel, DB4D_Rel_AutoLoadState inAutoLoadState)
{
	if (inRel == nil)
		return VE_DB4D_WRONG_RELATIONREF;
	else
	{
		const Relation* rel = dynamic_cast<const VDB4DRelation*>(inRel)->GetRel();
		return SetRelationAutoLoad1toN(rel, inAutoLoadState);
	}
}


Boolean BaseTaskInfo::IsRelationAutoLoadNto1(const CDB4DRelation* inRel) const
{
	Boolean res = false;
	if (inRel != nil)
	{
		Boolean unknown;
		const Relation* rel = dynamic_cast<const VDB4DRelation*>(inRel)->GetRel();
		res = IsRelationAutoLoadNto1(rel, unknown);
	}

	return res;
}


Boolean BaseTaskInfo::IsRelationAutoLoad1toN(const CDB4DRelation* inRel) const
{
	Boolean res = false;
	if (inRel != nil)
	{
		Boolean unknown;
		const Relation* rel = dynamic_cast<const VDB4DRelation*>(inRel)->GetRel();
		res = IsRelationAutoLoad1toN(rel, unknown);
	}

	return res;
}


DB4D_Rel_AutoLoadState BaseTaskInfo::GetRelationAutoLoadNto1State(const CDB4DRelation* inRel) const
{
	DB4D_Rel_AutoLoadState res = DB4D_Rel_AutoLoad_SameAsStructure;

	if (inRel != nil)
	{
		Boolean unknown;
		const Relation* rel = dynamic_cast<const VDB4DRelation*>(inRel)->GetRel();
		Boolean res2 = IsRelationAutoLoadNto1(rel, unknown);
		if (!unknown)
		{
			if (res2)
				res = DB4D_Rel_AutoLoad;
			else
				res = DB4D_Rel_Not_AutoLoad;
		}
	}

	return res;
}


DB4D_Rel_AutoLoadState BaseTaskInfo::GetRelationAutoLoad1toNState(const CDB4DRelation* inRel) const
{
	DB4D_Rel_AutoLoadState res = DB4D_Rel_AutoLoad_SameAsStructure;

	if (inRel != nil)
	{
		Boolean unknown;
		const Relation* rel = dynamic_cast<const VDB4DRelation*>(inRel)->GetRel();
		Boolean res2 = IsRelationAutoLoad1toN(rel, unknown);
		if (!unknown)
		{
			if (res2)
				res = DB4D_Rel_AutoLoad;
			else
				res = DB4D_Rel_Not_AutoLoad;
		}
	}

	return res;
}


void BaseTaskInfo::ExcludeTableFromAutoRelationDestination(CDB4DTable* inTableToExclude)
{
	ExcludeTableFromAutoRelationDestination(dynamic_cast<VDB4DTable*>(inTableToExclude)->GetTable());
}


void BaseTaskInfo::IncludeBackTableToAutoRelationDestination(CDB4DTable* inTableToInclude)
{
	IncludeBackTableToAutoRelationDestination(dynamic_cast<VDB4DTable*>(inTableToInclude)->GetTable());
}


Boolean BaseTaskInfo::IsTableExcludedFromAutoRelationDestination(CDB4DTable* inTableToCheck) const
{
	return IsTableExcludedFromAutoRelationDestination(dynamic_cast<VDB4DTable*>(inTableToCheck)->GetTable());
}

/*
void BaseTaskInfo::SetAllRelationsToAutomatic(Boolean RelationsNto1, Boolean ForceAuto)
{
	SetAllRelationsToAutomatic(RelationsNto1, ForceAuto);
}
*/
/*
Boolean BaseTaskInfo::IsAllRelationsToAutomatic(Boolean RelationsNto1)
{
	return IsAllRelationsToAutomatic(RelationsNto1);
}
*/
/*
void VDB4DBaseContext::SetExtraData( const VValueBag* inExtra)
{
	fBase.SetExtraData(inExtra);
}


const VValueBag* VDB4DBaseContext::RetainExtraData() const
{
	return fBase.RetainExtraData();
}
*/

LockPtr BaseTaskInfo::LockDataBaseDef(const CDB4DBase* inBase, sLONG inTimeOut, Boolean inForReadOnly, const VValueBag **outLockerExtraData)
{
	if (testAssert(inBase != nil))
		return (LockPtr)LockDataBaseDef(dynamic_cast<const VDB4DBase*>(inBase)->GetBase(), inTimeOut, inForReadOnly, outLockerExtraData);
	else
		return nil;
}


LockPtr BaseTaskInfo::LockTableDef(const CDB4DTable* inTable, Boolean inWithFields, sLONG inTimeOut, Boolean inForReadOnly, const VValueBag **outLockerExtraData)
{
	if (testAssert(inTable != nil))
		return (LockPtr)LockTableDef(dynamic_cast<const VDB4DTable*>(inTable)->GetTable(), inWithFields, inTimeOut, inForReadOnly, outLockerExtraData);
	else
		return nil;
}


LockPtr BaseTaskInfo::LockFieldDef(const CDB4DField* inField, sLONG inTimeOut, Boolean inForReadOnly, const VValueBag **outLockerExtraData)
{
	if (testAssert(inField != nil))
		return (LockPtr)LockFieldDef(dynamic_cast<const VDB4DField*>(inField)->GetField(), inTimeOut, inForReadOnly, outLockerExtraData);
	else
		return nil;
}


LockPtr BaseTaskInfo::LockRelationDef(const CDB4DRelation* inRelation, Boolean inWithRelatedFields, sLONG inTimeOut, Boolean inForReadOnly, const VValueBag **outLockerExtraData)
{
	if (testAssert(inRelation != nil))
		return (LockPtr)LockRelationDef(dynamic_cast<const VDB4DRelation*>(inRelation)->GetRel(), inWithRelatedFields, inTimeOut, inForReadOnly, outLockerExtraData);
	else
		return nil;
}


LockPtr BaseTaskInfo::LockIndexDef(const CDB4DIndex* inIndex, sLONG inTimeOut, Boolean inForReadOnly, const VValueBag **outLockerExtraData)
{
	if (testAssert(inIndex != nil))
		return (LockPtr)LockIndexDef(dynamic_cast<const VDB4DIndex*>(inIndex)->GetInd(), inTimeOut, inForReadOnly, outLockerExtraData);
	else
		return nil;
}


Boolean BaseTaskInfo::LockMultipleTables(const vector<const CDB4DTable*>& inTables, vector<LockPtr>& outLocks, sLONG inTimeOut, Boolean inForReadOnly)
{
	Boolean result = true;
	uLONG startmillisec = VSystem::GetCurrentTime();

	try
	{
		outLocks.resize(inTables.size(), nil);
	}
	catch (...)
	{
		result = false;
	}
	vector<LockPtr>::iterator curlock = outLocks.begin();
	for (vector<const CDB4DTable*>::const_iterator cur = inTables.begin(), end = inTables.end(); cur != end && result; cur++, curlock++)
	{
		LockPtr xlock = LockTableDef(*cur, true, inTimeOut, inForReadOnly);
		if (xlock != nil)
		{
			*curlock = xlock;
			if (inTimeOut != -1 && inTimeOut != 0)
			{
				uLONG currentime = VSystem::GetCurrentTime();
				sLONG passedtime = currentime - startmillisec;
				if (passedtime < inTimeOut)
					inTimeOut = inTimeOut - passedtime;
				else
					inTimeOut = 0;

			}
		}
		else
			result = false;
	}

	if (!result)
		UnLockMultipleTables(outLocks);

	return result;
}


void BaseTaskInfo::UnLockMultipleTables(const vector<LockPtr>& inLocks)
{
	for (vector<LockPtr>::const_iterator cur = inLocks.begin(), end = inLocks.end(); cur != end; cur++)
	{
		if (*cur != nil)
			UnLockStructObject(*cur);
	}
}


VErrorDB4D BaseTaskInfo::UnLockStructObject(LockPtr inLocker)
{
	return UnLockStructObject((StructObjLockPtr)inLocker);
}



void BaseTaskInfo::MarkRecordAsPushed(DataTable*df, sLONG numrec)
{
	fPushedRecordIDs[make_pair(df, numrec)]++;
}

void BaseTaskInfo::UnMarkRecordAsPushed(DataTable*df, sLONG numrec)
{
	RecRefMap::iterator found = fPushedRecordIDs.find(make_pair(df, numrec));
	if (testAssert(found != fPushedRecordIDs.end()))
	{
		xbox_assert(found->second > 0);
		if (found->second == 1)
			fPushedRecordIDs.erase(found);
		else
			found->second--;
	}

}


sLONG BaseTaskInfo::GetServerVersion() const
{
	return testAssert( (IsRemote() && (bd != nil) && (bd->GetLegacyNetAccess() != nil))) ? bd->GetLegacyNetAccess()->GetServerVersion() : 0;
}


sLONG BaseTaskInfo::GetClientVersion() const
{
	const VValueBag *extra = getContextOwner()->GetExtraData();
	xbox_assert( !IsRemote() && (extra != nil) );
	sLONG version = DB4DBagKeys::client_version( extra);
	return version;
}


bool BaseTaskInfo::CheckFeature( bool inServer, const Feature& inFeature)
{
	if (fFeaturesInited & inFeature.fFeatureFlag)
		return fFeatures & inFeature.fFeatureFlag;

	bool ok;
	sLONG version = inServer ? GetServerVersion() : GetClientVersion();
	switch(version & 0xff00)
	{
		case 0x1100:	ok = VersionGreaterOrEqual( version, inFeature.fMinVersionFor11); break;
		case 0x1200:	ok = VersionGreaterOrEqual( version, inFeature.fMinVersionFor12); break;
		case 0x1300:	ok = VersionGreaterOrEqual( version, inFeature.fMinVersionFor13); break;
		default:		ok = version != 0; break;
	}
	fFeaturesInited |= inFeature.fFeatureFlag;
	if (ok)
		fFeatures |= inFeature.fFeatureFlag;
	return ok;
}


void BaseTaskInfo::SetClientRequestStreams(VStream* OutputStream, VStream* InputStream)
{
	//## a faire
}

void BaseTaskInfo::SetServerRequestStreams(VStream* InputStream, VStream* OutputStream)
{
	//## a faire
}

/*
void VDB4DBaseContext::SendlastRemoteInfo()
{
	fBase.SendlastRemoteInfo();
}
*/
/*
CDB4DContext* VDB4DBaseContext::GetContextOwner() const
{
	return fBase.GetContextOwner();
}
*/

void BaseTaskInfo::DescribeQueryExecution(Boolean on)
{
	BuildQueryPath(on);
}

Boolean BaseTaskInfo::ShouldDescribeQueryExecution()
{
	return ShouldDescribeQuery();
}

void BaseTaskInfo::BuildQueryPlan(Boolean on)
{
	fBuildQueryPlan = on;
}

void BaseTaskInfo::BuildQueryPath(Boolean on)
{
	fBuildQueryPath = on;
}

Boolean BaseTaskInfo::ShouldBuildQueryPlan()const
{
	return fBuildQueryPlan;
}

Boolean BaseTaskInfo::ShouldBuildQueryPath()const
{
	return fBuildQueryPath;
}

void BaseTaskInfo::GetLastQueryDescription(XBOX::VString& outResult, DB4D_QueryDescriptionFormat inFormat)
{
	GetLastQueryDescription(outResult);
}


void BaseTaskInfo::GetLastQueryExecution(XBOX::VString& outResult, DB4D_QueryDescriptionFormat inFormat)
{
	if (inFormat == DB4D_QueryDescription_XML)
	{
		GetLastQueryExecutionXML(outResult);
	}
	else
	{
		GetLastQueryExecution(outResult);
	}
}

void BaseTaskInfo::SetLastQueryExecution(XBOX::VString& inExecution, DB4D_QueryDescriptionFormat inFormat)
{
	if (inFormat == DB4D_QueryDescription_XML)
	{
		SetQueryExecutionXML(inExecution);
	}
	else
	{
		SetQueryExecution(inExecution);
	}
}



CDB4DRemoteRecordCache* BaseTaskInfo::StartCachingRemoteRecords(CDB4DSelection* inSel, RecIDType FromRecIndex, RecIDType ToRecIndex,  CDB4DFieldCacheCollection* inWhichFields, const vector<uBYTE>& inWayOfLocking)
{
	VDB4DRemoteRecordCache* result = nil;
	StErrorContextInstaller errors(false);
	VError err = VE_OK;

	if (testAssert(inSel != nil))
	{
		result = new VDB4DRemoteRecordCache(inWhichFields, this);

		if (result != nil)
		{
			RemoteRecordCache* remoterecords = result->GetRemoteRecordCache();

			err = remoterecords->StartCachingRemoteRecords(dynamic_cast<VDB4DSelection*>(inSel)->GetSel(), FromRecIndex, ToRecIndex, inWayOfLocking);
		}

		if (err != VE_OK)
			ReleaseRefCountable(&result);
	}

	return result;
}


void BaseTaskInfo::StopCachingRemoteRecords(CDB4DRemoteRecordCache* inCacheInfo)
{
	// pourra etre utile, un jour
}


VErrorDB4D BaseTaskInfo::SetIdleTimeOut(uLONG inMilliseconds)
{
	VError err = VE_OK;
	DB4DNetManager* net = GetRemoteConnection();
	if (net != nil)
	{
		err = net->SetContextIdleTimeOut(this, inMilliseconds);
	}

	return err;
}



CDB4DContext* BaseTaskInfo::GetContextOwner()
{
	if (fParent == nil)
	{
		VDB4DContext* xcontext = new VDB4DContext(fUserSession, fJSContext, fIsLocal);
		VDBMgr::GetManager()->RegisterContext(xcontext);
		
		fParent = xcontext;
		//BaseTaskInfo* thethis = const_cast<BaseTaskInfo*>(this);
		xcontext->ContainButDoesNotOwn(this);
		fRetainsParent = true;
		
	}
	return fParent;
}


void BaseTaskInfo::FreeAllJSFuncs()
{
	if (fParent != nil)
		fParent->FreeAllJSFuncs();
}


VDB4DContext* BaseTaskInfo::getContextOwner() const
{
	if (fParent == nil)
		const_cast<BaseTaskInfo*>( this)->GetContextOwner();
	if (fParent == nil)
		return nil;
	else
		return dynamic_cast<VDB4DContext*>(fParent);
}


BaseTaskInfo* BaseTaskInfo::GetOrBuildSyncHelperContext(Base4D* base)
{
	if (fSyncHelperContext == nil)
	{
		fSyncHelperContext = new BaseTaskInfo(base, nil, nil, nil);
		sLONG translevel = CurrentTransactionLevel();
		for (sLONG i = 0; i < translevel; i++)
		{
			fSyncHelperContext->StartTransaction();
		}
	}
	return fSyncHelperContext;
}


bool BaseTaskInfo::GetJSMethod(const JSEntityMethodReference& methRef, VJSObject& outObjFunc)
{
	bool result = false;
	MapOfJSMethods::iterator found = fJSMethods.find(methRef);
	if (found != fJSMethods.end())
	{
		outObjFunc = found->second.fFuncObj;
		result = true;
	}
	return result;
}


void BaseTaskInfo::SetJSMethod(const JSEntityMethodReference& methRef, const VJSObject& inObjFunc)
{
	if (fJSContext != nil)
	{
		VJSContext jscontext(fJSContext);
		inObjFunc.Protect();
		fJSMethods[methRef] = JSEntityMethod(inObjFunc);
	}
}

void BaseTaskInfo::FreeAllJSMethods()
{
	for (MapOfJSMethods::iterator cur = fJSMethods.begin(), end = fJSMethods.end(); cur != end; ++cur)
	{
		VJSContext jscontext(fJSContext);
		VJSObject* funcobj = &(cur->second.fFuncObj);
		funcobj->Unprotect();
	}
	fJSMethods.clear();
}


bool BaseTaskInfo::AlreadyCalledRestrict(const EntityModel* model)
{
	SetOfModels::iterator found = fAlreadyCalledRestrict.find(model);
	if (found == fAlreadyCalledRestrict.end())
	{
		fAlreadyCalledRestrict.insert(model);
		return false;
	}
	else
		return true;
}

void BaseTaskInfo::ReleaseCalledRestrict(const EntityModel* model)
{
	fAlreadyCalledRestrict.erase(model);
}

void BaseTaskInfo::CleanUpForReuse()
{
	StErrorContextInstaller errs(false);
	while (curtrans != nil)
		RollBackTransaction();
	//fRemotePages.clear();
	ClearAllExtraData();
	ReleaseRefCountable(&fUserSession);
	ReleaseRefCountable(&fThreadPrivileges);

}

/*
RemotePage* BaseTaskInfo::GetRemotePage(RemoteEntityCollection* sel)
{
	// no need for a mutex
	return &(fRemotePages[sel]);
}
*/


void BaseTaskInfo::ClearAllExtraData()
{
	for (DataMap::iterator cur = fDataMap.begin(), end = fDataMap.end(); cur != end; ++cur)
	{
		IRefCountable* session = cur->second;
		if (session == nil)
		{
			//xbox_assert(false);
			sLONG xdebug = 1; // put a break here
		}
		else
			session->Release();
	}
	fDataMap.clear();
}


IRefCountable* BaseTaskInfo::GetExtraData(void* selector)
{
	return fDataMap[selector];
}


void BaseTaskInfo::SetExtraData(void* selector, IRefCountable* data)
{
	fDataMap[selector] = data;
}


void BaseTaskInfo::RemoveExtraData(void* selector)
{
	DataMap::iterator found = fDataMap.find(selector);
	if (found != fDataMap.end())
	{
		QuickReleaseRefCountable(found->second);
		fDataMap.erase(found);
	}
}
 






													

													// ======================================

#if 0
/*
RelationInfoInContext::RelationInfoInContext(const Relation* rel)
{ 
	fRel = const_cast<Relation*>(rel);
	fAutoLoad = false;
	fMustRelease = true;
	if (testAssert(fRel != nil))
	{
		fRel->Retain();
		fAutoLoad = rel->IsAutoLoad();
	}
}
*/

RelationInfoInContext::RelationInfoInContext(const Relation* rel, Boolean autoload)
{ 
	fRel = const_cast<Relation*>(rel);
	fAutoLoad = autoload;
	fMustRelease = true;
	if (testAssert(fRel != nil))
	{
		fRel->Retain();
	}
}


RelationInfoInContext::~RelationInfoInContext()
{
	if (fRel != nil && fMustRelease)
		fRel->Release();
}

#endif

														// ======================================
														


Boolean LockerCount::IncCount(const BaseTaskInfo* context)
{
	try
	{
		fCounts[context]++;
	}
	catch (...)
	{
		return false;
	}
	return true;
}


Boolean LockerCount::DecCount(const BaseTaskInfo* context)
{
	try
	{
		mapofcount::iterator found = fCounts.find(context);
		if (testAssert(found != fCounts.end()))
		{
			xbox_assert(found->second > 0);
			found->second--;
			if (found->second <= 0)
			{
				fCounts.erase(found);
			}
		}
	}
	catch (...)
	{
		return false;
	}
	return true;
}


sLONG LockerCount::GetCountFor(const BaseTaskInfo* context)
{
	mapofcount::iterator found = fCounts.find(context);
	if (found == fCounts.end())
		return 0;
	else
		return found->second;
}


sLONG LockerCount::RemoveContext(const BaseTaskInfo* context)
{
	mapofcount::iterator found = fCounts.find(context);
	if (found == fCounts.end())
		return 0;
	else
	{
		sLONG result = found->second;
		fCounts.erase(found);
		return result;
	}
}



BaseTaskInfo* ConvertContext(CDB4DBaseContext *inContext)
{
	BaseTaskInfo* context = nil;
	if (inContext != nil)
	{
		context = (dynamic_cast<BaseTaskInfo*>(inContext));
	}

	return context;
}



ObjLocker::ObjLocker(CDB4DBaseContextPtr inContext, Base4D* base, ObjLocker* dejalocker) : fContext( NULL), fLocker( NULL)
{
	xInit(inContext, base, dejalocker);
	if (!fContext->IsRemote())
		fLocker = fContext->LockDataBaseDef(base);
}


ObjLocker::ObjLocker(CDB4DBaseContextPtr inContext, Table* table, Boolean withfields, ObjLocker* dejalocker) : fContext( NULL), fLocker( NULL)
{
	xInit(inContext,table->GetOwner(), dejalocker);
	if (!fContext->IsRemote())
		fLocker = fContext->LockTableDef(table, withfields);
}


ObjLocker::ObjLocker(CDB4DBaseContextPtr inContext, Field* cri, ObjLocker* dejalocker) : fContext( NULL), fLocker( NULL)
{
	xInit(inContext, cri->GetOwner()->GetOwner(), dejalocker);
	if (!fContext->IsRemote())
		fLocker = fContext->LockFieldDef(cri);
}


ObjLocker::ObjLocker(CDB4DBaseContextPtr inContext, Relation* rel, Boolean withfields, ObjLocker* dejalocker) : fContext( NULL), fLocker( NULL)
{
	xInit(inContext, rel->GetOwner(), dejalocker);
	if (!fContext->IsRemote())
		fLocker = fContext->LockRelationDef(rel, withfields);
}


ObjLocker::ObjLocker(CDB4DBaseContextPtr inContext, IndexInfo* ind, ObjLocker* dejalocker) : fContext( NULL), fLocker( NULL)
{
	xInit(inContext, ind->GetDB(), dejalocker);
	if (!fContext->IsRemote())
		fLocker = fContext->LockIndexDef(ind);
}


