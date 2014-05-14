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

using namespace std;
#include <algorithm>



Selection::Selection(DataTable *xDF, Base4D* remoteDB)
{
	xInit();
	fIsRemote = false;
	parentfile=xDF;
	if (remoteDB != nil)
		db = remoteDB;
	else
		db=xDF->GetDB();
}


Selection::Selection(Base4D* remoteDB, CDB4DBaseContext* inContext)
{
	xInit();
	fIsRemote = (inContext != (CDB4DBaseContext*)-1);
	parentfile=nil;
	db=remoteDB;
}


void Selection::xInit()
{
	//fStopWrite = 0;
	qtfic=0;
	SelisCluster=false;
	antelen=0;
	nbuse = 1;
	fModificationCounter = 0;
	fRequestedFieldsMaster = nil;

	// fix FM
	fKeepRecordLocked = false;
	fLockingContext = NULL;
	fQueryPath = nil;
	fQueryPlan = nil;
}



void Selection::Kill()
{
	xbox_assert(SelisCluster);
	Release();
}


void Selection::Dispose(void)
{
	if (SelisCluster)
	{
		Release();
	}
	else
	{
		unuse();
		if (nbuse == 0)
		{
			Release();
		}
	}
}


Boolean Selection::IsOwnerRemote() const
{
	if (db == nil)
		return false;
	else
		return db->IsRemote();
}


bool Selection::SaveObj(VSize& outSizeSaved)
{
	outSizeSaved = 0;
	return true;
}


void Selection::setmodif(uBOOL xismodif, Base4D* bd, BaseTaskInfo* context)
{
	/*if (xismodif)
		fStopWrite = 0;*/
	IObjToFlush::setmodif(xismodif, bd, context);
}


VError Selection::ThrowError( VError inErrCode, ActionDB4D inAction) const
{
	if (parentfile != nil)
		return parentfile->ThrowError(inErrCode, inAction);
	else
	{
		if (db != nil)
			return db->ThrowError(inErrCode, inAction);
		else
			return ThrowBaseError(inErrCode, inAction);
	}
}


sLONG Selection::GetNextRecord(sLONG fromRecord, sLONG FromIndice)
{
	xbox_assert(FromIndice>=0);

	if (FromIndice >= qtfic)
		return -1;
	else
		return GetFic(FromIndice);
}



sLONG Selection::GetPreviousRecord(sLONG fromRecord, sLONG FromIndice)
{
	if (FromIndice < 0 || qtfic == 0)
		return -1;
	else
	{
		if (FromIndice >= qtfic)
			return GetFic(qtfic-1);
		else
			return GetFic(FromIndice);
	}
}


sLONG Selection::GetLastRecord()
{
	if (qtfic > 0)
		return GetFic(qtfic-1);
	else
		return -1;
}


void Selection::CheckFieldsForCache()
{
	if (fRequestedFieldsMaster != nil)
	{
		if (parentfile != nil)
		{
			fRequestedFieldsMaster->AddSel(parentfile->GetNum());
		}
	}
}

sLONG Selection::GetQTfic(void)
{ 
	CheckFieldsForCache();
	return(qtfic);
}


Boolean Selection::IsEmpty()
{
	CheckFieldsForCache();
	return (qtfic == 0);
}


VError Selection::PutFicWithWafSelection(sLONG n, sLONG r, Bittab* inWafSelbits, WafSelection* outWafSel)
{
	if (inWafSelbits != nil && outWafSel!= nil)
	{
		if (inWafSelbits->isOn(r))
			outWafSel->AddElem();
		else
			outWafSel->SkipElem();
	}
	return PutFic(n, r);
}


void Selection::SetSelAddr(DataAddr4D addr)
{		
	SelisCluster=true;
	setaddr(addr);
}


VError Selection::AddToBittab(Bittab *b, BaseTaskInfo* context)
{
	return(VE_DB4D_NOTIMPLEMENTED);
}


VError Selection::AddToSel(sLONG n)
{
	return(VE_DB4D_NOTIMPLEMENTED);
}


VError Selection::DelFromSel(sLONG n)
{
	return(VE_DB4D_NOTIMPLEMENTED);
}


VError Selection::DelFromSel(Bittab* b)
{
	return(VE_DB4D_NOTIMPLEMENTED);
}


VError Selection::FillArray(sLONG* tab, sLONG maxelem)
{
	return(VE_DB4D_NOTIMPLEMENTED);
}

/*
void Selection::OktoSave(BaseTaskInfo* context)
{
}
*/

VError Selection::AddSelToSel(sLONG& curfic, sLONG maxnb, Bittab* filtre, Selection* sel)
{
	sLONG i,qt,n;
	VError err = VE_OK;
	
	qt=GetQTfic();
	for (i=0; (i<qt) && (curfic<maxnb) && (err == VE_OK); i++)
	{
		n = GetFic(i);
		if ( (n>=0) && (filtre == nil || filtre->isOn(n)) )
		{
			err = sel->PutFic(curfic,n);
			++curfic;
		}
	}
	Touch();
	return err;
}


VError Selection::AddSelToSelWithWafSelection(sLONG& curfic, sLONG maxnb, Bittab* filtre, Selection* sel, Bittab* inWafSelbits, WafSelection* outWafSel)
{
	sLONG i,qt,n;
	VError err = VE_OK;

	qt=GetQTfic();
	for (i=0; (i<qt) && (curfic<maxnb) && (err == VE_OK); i++)
	{
		n = GetFic(i);
		if ( (n>=0) && (filtre == nil || filtre->isOn(n)) )
		{
			err = sel->PutFicWithWafSelection(curfic,n, inWafSelbits, outWafSel);
			++curfic;
		}
	}
	Touch();
	return err;
}



void Selection::BaseCopyInto(Selection* newsel)
{
	newsel->db = db;
	newsel->qtfic = qtfic;
	newsel->Touch();
}


VError Selection::DeleteRecords(BaseTaskInfo* context, Bittab* NotDeletedOnes, Relation* RefIntRel, VDB4DProgressIndicator* InProgress, Table* inRemoteTable)
{
	VError err = VE_OK;
	if (fIsRemote || IsOwnerRemote())
	{
		if (testAssert(inRemoteTable != nil))
		{
			IRequest* req = db->CreateRequest(context->GetEncapsuleur(), Req_DeleteRecordsInSelection + kRangeReqDB4D, db->IsThereATrigger(DB4D_DeleteRecord_Trigger, inRemoteTable, true) );
			if (req == nil)
				err = ThrowBaseError(memfull, noaction);
			else
			{
				req->PutBaseParam(db);
				req->PutThingsToForget( VDBMgr::GetManager(), context);
				req->PutTableParam(inRemoteTable);
				req->PutSelectionParam(this, context->GetEncapsuleur());
				req->PutBooleanParam(NotDeletedOnes != nil);
				req->PutProgressParam(InProgress);

				err = req->GetLastError();

				if (err == VE_OK)
				{
					err = req->Send();
					if (err== VE_OK)
						err = req->GetUpdatedInfo(db, context);
					if (err == VE_OK)
					{
						Bittab* lockedset = req->RetainSetReply(db, inRemoteTable, err, context->GetEncapsuleur());
						if (lockedset != nil)
						{
							NotDeletedOnes->Or(lockedset);
							lockedset->Release();
							err = VE_DB4D_CANNOT_DELETE_ALL_RECORDS_IN_SEL;
						}
					}
				}
				req->Release();
			}
		}
	}
	else
	{
		{
			Boolean DeleteDejaFait = false;
	#if trackModif
		trackDebugMsg("before delete selection of records\n");
	#endif
			if (parentfile != nil)
			{
				Table* target = parentfile->GetTable();
				bool willBeLogged = target->CanBeLogged();
				db->ReleaseLogMutex();

				if (!willBeLogged && !target->HasSyncInfo() && !(target->GetOwner()->ReferentialIntegrityEnabled() && target->AtLeastOneReferentialIntegrity()) )
				{
					parentfile->LockRead();
					bool mustunlock = true;
					if (IsAllRecords(context) && GetCurrentTransaction(context) == nil)
					{
						if (!db->IsThereATrigger(DB4D_DeleteRecord_Trigger, parentfile->GetTable()))
						{
							DeleteDejaFait = true;
							err = parentfile->GetTable()->Truncate(context, InProgress, true, mustunlock);
							// attention le Unlock peut avoir ete fait dans le Truncate ( 2 fois en tout)
							if (err == -2)
							{
								DeleteDejaFait = false;
								err = 0;
							}
						}
					}
					if (mustunlock)
						parentfile->Unlock();

					if (!DeleteDejaFait)
					{
						if (!willBeLogged && !target->HasSyncInfo() && (!parentfile->GetTable()->AtLeastOneBlob()) && GetCurrentTransaction(context) == nil && !db->IsThereATrigger(DB4D_DeleteRecord_Trigger, parentfile->GetTable()))
						{
							if (target->NonIndexed() || (parentfile->GetNbRecords(context, false)/3) < GetQTfic())
							{
								//parentfile->LockQuickDel();

								DeleteDejaFait = true;
								sLONG i;
								Boolean first = true;
								SelectionIterator itersel(this);

								sLONG currec = itersel.LastRecord();
								sLONG count = 0;
								StErrorContextInstaller errorContext( true);	// true means all errors are kept

								if (InProgress != nil)
								{
									InProgress->BeginSession(GetQTfic(), GetString(1005,23),true);
								}

	#if trackModif
								trackDebugMsg("before quick delete selection of records\n");
	#endif
								/*
								IndexArray indexdeps;
								target->CopyIndexDep(indexdeps);
								for (IndexArray::Iterator cur = indexdeps.First(), end = indexdeps.End(); cur != end; cur++)
								{
									IndexInfo* ind = *cur;
									if (ind != nil)
										ind->SetInvalidOnDisk();
								}
								*/
								target->DelayIndexes(false);

	#if trackModif
								trackDebugMsg("before quick delete selection of records, after index invalidation\n");
	#endif

								while (currec != -1)
								{
									VTask::Yield();
									if (InProgress != nil)
									{
										if (!InProgress->Progress(count))
										{
											err = ThrowError(VE_DB4D_ACTIONCANCELEDBYUSER, DBaction_DeletingRecord);
											break;
										}
									}

									count++;
									sLONG n = currec;
									VError err2;
									err2 = parentfile->QuickDelRecord(n, context);
									if (err2 != VE_OK)
									{
										err = VE_DB4D_CANNOT_DELETE_ALL_RECORDS_IN_SEL;
										if (NotDeletedOnes != nil)
										{
											if (first)
											{
												first = false;
												err = NotDeletedOnes->aggrandit(parentfile->GetMaxRecords(context));
												if (err != VE_OK)
													break; // en cas de probleme pour aggrandir l'ensemble des fiches non detruite, il vaut mieux arreter tout de suite
											}
											NotDeletedOnes->Set(n);
										}
									}

									// any error that has been thrown in the loop is pushed in the previous error context.
									// then the current error context is flushed and vGetLastError() returns VE_OK.
									errorContext.MergeAndFlush();

									currec = itersel.PreviousRecord();
								}

								if (InProgress != nil)
									InProgress->EndSession();
								
	#if trackModif
								trackDebugMsg("after quick delete selection of records, before index rebuilding\n");
	#endif
								/*
								target->occupe();
								target->RebuildAllRelatedIndex(context->GetEncapsuleur(), InProgress);
								target->libere();
								*/
								target->AwakeIndexes(nil, nil);

	#if trackModif
								trackDebugMsg("after quick delete selection of records\n");
	#endif
								//parentfile->UnLockQuickDel();
							}
						}
					}
				}
			}

			if (!DeleteDejaFait)
			{
				sLONG i;
				Boolean first = true;
				SelectionIterator itersel(this);

				sLONG currec = itersel.LastRecord();
				//sLONG currec = itersel.FirstRecord();
				sLONG count = 0;

				//sLONG qt = GetQTfic();
				//for (i = qt-1; i>=0; i--)
				
				// L.E. install a new error context.
				// vGetLastError() will now return VE_OK.
				StErrorContextInstaller errorContext( true);	// true means all errors are kept

				
				bool endSession = false;
				if (InProgress != nil && (currec != -1))
				{
					//Ensure there are sme records to delete before creating a session
					InProgress->BeginSession(GetQTfic(), GetString(1005,23),true);
					endSession = true;
				}
				
				while (currec != -1)
				{
					VTask::Yield();
					if (InProgress != nil)
					{
						if (!InProgress->Progress(count))
						{
							err = ThrowError(VE_DB4D_ACTIONCANCELEDBYUSER, DBaction_DeletingRecord);
							break;
						}
					}

					count++;
					//sLONG n = GetFic(i);
					sLONG n = currec;
					VError err2 = VE_OK;

					{
						FicheInMem* rec = parentfile->LoadRecord(n, err2, DB4D_Keep_Lock_With_Record, context, true);
						if (rec != nil)
						{
							if (RefIntRel != nil)
								context->MustNotCheckRefInt(RefIntRel, rec);
							err2 = parentfile->DelRecord(rec, context,-1,InProgress);
							if (RefIntRel != nil)
								context->CheckRefIntAgain(RefIntRel, rec);
							rec->Release();
						}
					}

					if (err2 != VE_OK)
					{
						err = VE_DB4D_CANNOT_DELETE_ALL_RECORDS_IN_SEL;
						if (NotDeletedOnes != nil)
						{
							if (first)
							{
								first = false;
								VError err3 = NotDeletedOnes->aggrandit(parentfile->GetMaxRecords(context));
								if (err3 != VE_OK)
									break; // en cas de probleme pour aggrandir l'ensemble des fiches non detruite, il vaut mieux arreter tout de suite
							}
							NotDeletedOnes->Set(n);
						}
					}

					// any error that has been thrown in the loop is pushed in the previous error context.
					// then the current error context is flushed and vGetLastError() returns VE_OK.
					errorContext.MergeAndFlush();

					currec = itersel.PreviousRecord();
					//currec = itersel.NextRecord();

				}

				if ( (InProgress != nil) && endSession)
					InProgress->EndSession();
			}

	#if trackModif
			trackDebugMsg("after delete selection of records\n");
	#endif
		}
	}

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOT_DELETE_ALL_RECORDS_IN_SEL, DBaction_DeletingSelectionOfRecord);
	return err;
}


Boolean Selection::IsAllRecords(BaseTaskInfo* context)
{
	if (fIsRemote || IsOwnerRemote())
		return false;
	else
		return GetQTfic() == parentfile->GetNbRecords(context);
}


inline Boolean CompStringPtrLess(VString* v1, VString* v2) { return *v1 < *v2; };



VError Selection::GetDistinctValues(db4dEntityAttribute* att, DB4DCollectionManager& outCollection, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, const VCompareOptions& inOptions)
{
	VError err = VE_OK;

	if (fIsRemote || IsOwnerRemote())
	{
		xbox_assert(false); // a faire
		err = -1; 
	}
	else
	{
		bool dejafait = false;
		if (att->GetKind() == eattr_storage && !att->HasEvent(dbev_load))
		{
			Field* cri = att->RetainDirectField();
			if (cri != nil)
			{
				dejafait = true;
				err = GetDistinctValues(cri, outCollection, context, InProgress, inOptions);
				cri->Release();
			}
		}

		if (!dejafait)
		{
			TryToFastGetDistinctValues(err, att, outCollection, context, InProgress, inOptions);
		}
	}

	return err;
}



VError Selection::GetDistinctValues(Field* cri, DB4DCollectionManager& outCollection, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, const VCompareOptions& inOptions)
{
	VError err = VE_OK;

	if (fIsRemote || IsOwnerRemote())
	{
		xbox_assert(false); // a faire
		err = -1; 
	}
	else
	{
		xbox_assert(cri != nil);
		Boolean lexico = false;
		bool forPict = false;
		VCompareOptions Options = inOptions;
		Options.SetIntlManager(GetContextIntl(context));

		cri->Retain();

		Table* tab = cri->GetOwner();
		tab->Retain();
		
		IndexInfo* ind = cri->FindAndRetainIndexSimple(true, true, context);

		if (ind == nil)
		{
			ind = cri->FindAndRetainIndexLexico(false,context);
			if (ind != nil)
			{
				lexico = true;
				if (! ind->AskForValid(context))
				{
					ind->Release();
					ind = nil;
				}
			}
		}

		if (ind == nil && cri->GetTyp() == VK_IMAGE)
		{
			lexico = true;
			forPict = true;
		}

		if (ind != nil)
		{
			if (((sLONG8)GetQTfic() * tab->GetSeqRatioCorrector()) < ind->GetNBDiskAccessForAScan(!lexico))
			{
				ind->ReleaseValid();
				ind->Release();
				ind = nil;
			}
		}

		if (ind != nil)
		{
			if (IsAllRecords(context))
			{
				err = ind->GetDistinctValues(outCollection, context, nil, InProgress, Options);
			}
			else
			{
				if (typsel == sel_bitsel)
				{
					err = ind->GetDistinctValues(outCollection, context, ((BitSel*)this)->GetTB(), InProgress, Options);

				}
				else
				{
					Bittab b;
					err = AddToBittab(&b, context);
					if (err == VE_OK)
					{	
						err = ind->GetDistinctValues(outCollection, context, &b, InProgress, Options);
					}
				}
			}
			ind->ReleaseValid();
			ind->Release();
		}
		else
		{
			if (lexico || !TryToFastGetDistinctValues(err, cri, outCollection, context, InProgress, Options))
			{
				if (cri->GetTyp() == VK_TEXT || lexico)
				{
					DB4DKeyWordList allkeys;
					allkeys.SetSizeIncrement(16384);
					SelectionIterator itersel(this);
					sLONG ficnum = itersel.FirstRecord();
					while (ficnum!= -1)
					{
						Boolean add = false;
						FicheInMem* fic = parentfile->LoadRecord(ficnum, err, DB4D_Do_Not_Lock, context, false);
						if (fic != nil)
						{
							ValPtr cv = fic->GetFieldValue(cri, err, false, true);
							if (cv != nil)
							{
								ValueKind vk = cv->GetValueKind();
								if (vk == VK_TEXT || vk == VK_STRING || vk == VK_STRING_UTF8 || vk == VK_TEXT_UTF8 || vk == VK_IMAGE)
								{
									DB4DKeyWordList keys;
									if (vk == VK_IMAGE)
									{
#if !VERSION_LINUX   // Postponed Linux Implementation !
										VString sKeys;
										((VPicture*)cv)->GetKeywords(sKeys);
										err = BuildKeyWords(sKeys, keys, Options);
#endif
									}
									else
										BuildKeyWords(*((VString*)cv), keys, Options);
									sLONG i,nb = keys.GetCount();
									for (i=0; i<nb; i++)
									{
										if (allkeys.Add(keys[i]))
										{
											keys[i] = nil;
										}
										else
										{
											err = ThrowError(memfull, DBaction_GetDistinctValues);
											break;
										}
									}
								}
							}
							fic->Release();
						}
						if (err != VE_OK)
							break;
						ficnum = itersel.NextRecord();
					}

					if (err == VE_OK)
					{
						sLONG nb = allkeys.GetCount();
						sort(allkeys.First(), allkeys.First() + nb, CompStringPtrLess);
						sLONG i;
						VString* curvalue;
						if (nb>0)
						{
							curvalue = allkeys[0];
							err = outCollection.AddOneElement(1, *curvalue);
							if ( (err == VE_OK) && (nb > 1) )
							{
								VString* *p = &allkeys[1];
								for (i = 1; i<nb; i++)
								{
									VString* s = *p;
									if (*s != *curvalue)
									{
										curvalue = s;
										err = outCollection.AddOneElement(1, *curvalue);
										if (err != VE_OK)
											break;
									}
									p++;
								}
							}
						}
					}

				}
				else
				{
					SortTab sortcri(cri->GetOwner()->GetOwner());
					sortcri.AddTriLineField(cri->GetOwner()->GetNum(), cri->GetPosInRec(), true);
					Selection* sel = SortSel(err, &sortcri, context, InProgress);
					if (sel != nil)
					{
						ValPtr curvalue = nil;
						sLONG curvaluepos = 0;
						SelectionIterator itersel(sel);
						sLONG ficnum = itersel.FirstRecord();
						while (ficnum!= -1)
						{
							Boolean add = false;
							FicheInMem* fic = parentfile->LoadRecord(ficnum, err, DB4D_Do_Not_Lock, context, false);
							if (fic != nil)
							{
								ValPtr cv = fic->GetFieldValue(cri, err, false, true);
								if (cv != nil)
								{
									if (curvalue == nil)
									{
										add = true;
										curvalue = (ValPtr)cv->Clone();
									}
									else
									{
										if (cv->CompareToSameKindWithOptions(curvalue, Options) != CR_EQUAL)
										{
											add = true;
											delete curvalue;
											curvalue = (ValPtr)cv->Clone();
										}
									}
								}
								if (add)
								{
									if (curvalue == nil)
										err = err = ThrowError(memfull, DBaction_GetDistinctValues);
									else
									{
										/*
										err = outCollection.SetCollectionSize(curvaluepos+1);
										if (err == VE_OK)
										{
											err = outCollection.SetNthElement(curvaluepos+1, 1, *curvalue);
											curvaluepos++;
										}
										*/
										err = outCollection.AddOneElement(1, *curvalue);
										curvaluepos++;
										
									}
								}
								fic->Release();
							}
							if (err != VE_OK)
								break;
							ficnum = itersel.NextRecord();
						}
						sel->Release();
					}
				}
			}
		}

		tab->Release();
		cri->Release();
	}

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOT_CANNOT_COMPLETE_DISTINCT_VALUES, DBaction_GetDistinctValues);

	return err;
}



VError Selection::QuickGetDistinctValues(Field* cri, QuickDB4DArrayOfValues *outCollection, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, const VCompareOptions& inOptions)
{
	VError err = VE_OK;

	if (fIsRemote || IsOwnerRemote())
	{
		xbox_assert(false); // a faire
		err = -1; 
	}
	else
	{
		xbox_assert(cri != nil);
		Boolean lexico = false;
		VCompareOptions Options = inOptions;
		Options.SetIntlManager(GetContextIntl(context));

		cri->Retain();

		Table* tab = cri->GetOwner();
		tab->Retain();

		IndexInfo* ind = cri->FindAndRetainIndexSimple(true, true, context);


		if (ind != nil)
		{
			if (((sLONG8)GetQTfic() * tab->GetSeqRatioCorrector()) < ind->GetNBDiskAccessForAScan(true))
			{
				ind->ReleaseValid();
				ind->Release();
				ind = nil;
			}
		}

		if (ind != nil)
		{
			if (IsAllRecords(context))
			{
				err = ind->QuickGetDistinctValues(outCollection, context, nil, InProgress, Options);
			}
			else
			{
				if (typsel == sel_bitsel)
				{
					err = ind->QuickGetDistinctValues(outCollection, context, ((BitSel*)this)->GetTB(), InProgress, Options);

				}
				else
				{
					Bittab b;
					err = AddToBittab(&b, context);
					if (err == VE_OK)
					{	
						err = ind->QuickGetDistinctValues(outCollection, context, &b, InProgress, Options);
					}
				}
			}
			ind->ReleaseValid();
			ind->Release();
		}
		else
		{
			if (!TryToFastQuickGetDistinctValues(err, cri, outCollection, context, InProgress, Options))
			{
				xbox_assert(false);
				// on ne devrait jamais passer par la
			}
		}

		tab->Release();
		cri->Release();
	}

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOT_CANNOT_COMPLETE_DISTINCT_VALUES, DBaction_GetDistinctValues);

	return err;
}


Boolean Selection::IntersectBittab(Bittab* b, BaseTaskInfo* context, VError& err)
{
	err = VE_DB4D_NOTIMPLEMENTED;
	return false;
}

VError Selection::FillWithArray(DB4DCollectionManager& inCollection)
{
	VError err = VE_OK;

	if (GetTypSel() == t_bitsel)
		ClearSel();

	sLONG nb = inCollection.GetCollectionSize();
	if (FixFic(nb))
	{
		for (sLONG i = 1; i <= nb; i++)
		{
			ConstValPtr cv;
			bool disposeIt = false;
			err = inCollection.GetNthElement(i, 1, cv, &disposeIt);
			if (err == VE_OK)
			{
				err = PutFic(i-1, cv->GetLong());
				if (disposeIt)
					delete cv;
			}
			if (err != VE_OK)
				break;
		}
	}
	else
		err = ThrowError(memfull, DBaction_BuildingSelection);

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTBUILDSELECTION, DBaction_BuildingSelection);

	return err;
}



VError Selection::DelFromSelRemote(sLONG n, CDB4DBaseContext* inContext)
{
	VError err = VE_OK;
	if (testAssert(fIsRemote))
	{
		IRequest* req = db->CreateRequest(inContext, Req_DelRecFromSel + kRangeReqDB4D);
		if (req == nil)
			err = ThrowBaseError(memfull, noaction);
		else
		{
			req->PutBaseParam(db);
			req->PutThingsToForget( VDBMgr::GetManager(), ConvertContext(inContext));
			req->PutSelectionParam(this, inContext);
			req->PutLongParam(n);

			err = req->GetLastError();

			if (err == VE_OK)
			{
				err = req->Send();
				if (err == VE_OK)
				{
					sLONG count = req->GetLongReply(err);
					if (err == VE_OK)
					{
						ClearRemoteCache(req->GetInputStream());
						SetCompte(count);
					}
				}
			}
			req->Release();
		}
	}
	return err;
}


VError Selection::DelFromSelRemote(Bittab* b, CDB4DBaseContext* inContext)
{
	VError err = VE_OK;
	if (testAssert(fIsRemote))
	{
		IRequest* req = db->CreateRequest(inContext, Req_DelSetFromSel + kRangeReqDB4D);
		if (req == nil)
			err = ThrowBaseError(memfull, noaction);
		else
		{
			req->PutBaseParam(db);
			req->PutThingsToForget( VDBMgr::GetManager(), ConvertContext(inContext));
			req->PutSelectionParam(this, inContext);
			req->PutSetParam(b, inContext);

			err = req->GetLastError();

			if (err == VE_OK)
			{
				err = req->Send();
				if (err == VE_OK)
				{
					sLONG count = req->GetLongReply(err);
					if (err == VE_OK)
					{
						ClearRemoteCache(req->GetInputStream());
						SetCompte(count);
					}
				}
			}
			req->Release();
		}
	}
	return err;
}


VError Selection::RemoveSelectedRangeRemote(sLONG inRecordIndex1, sLONG inRecordIndex2, CDB4DBaseContext* inContext)
{
	VError err = VE_OK;
	if (testAssert(fIsRemote))
	{
		IRequest* req = db->CreateRequest(inContext, Req_DelRangeFromSel + kRangeReqDB4D);
		if (req == nil)
			err = ThrowBaseError(memfull, noaction);
		else
		{
			req->PutBaseParam(db);
			req->PutThingsToForget( VDBMgr::GetManager(), ConvertContext(inContext));
			req->PutSelectionParam(this, inContext);
			req->PutLongParam(inRecordIndex1);
			req->PutLongParam(inRecordIndex2);

			err = req->GetLastError();

			if (err == VE_OK)
			{
				err = req->Send();
				if (err == VE_OK)
				{
					sLONG count = req->GetLongReply(err);
					if (err == VE_OK)
					{
						ClearRemoteCache(req->GetInputStream());
						SetCompte(count);
					}
				}
			}
			req->Release();
		}
	}
	return err;
}



VError Selection::FillArray(DB4DCollectionManager& outCollection)
{
	VError err = VE_OK;

	VLong vl;
	err = outCollection.SetCollectionSize(GetQTfic());
	if (err == VE_OK)
	{
		SelectionIterator itersel(this);
		for (sLONG i = itersel.FirstRecord(), cur = 1; i != -1; i = itersel.NextRecord(), cur++)
		{
			vl.FromLong(i);
			err = outCollection.SetNthElement(cur, 1, vl);
			if (err != VE_OK)
				break;
		}
	}

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOT_BUILD_ARRAYOFVALUES, DBaction_BuildingArrayOfValues);


	return err;
}


Base4D* Selection::GetDB()
{
	if (parentfile != nil)
		return parentfile->GetDB();
	else
		return db;
}


VError Selection::FillBittabWithWafSel(WafSelection* inWafSel, Bittab& outWafSelBits)
{
	VError err = VE_OK;
	if (parentfile != nil)
		outWafSelBits.aggrandit(parentfile->GetMaxRecords(nil));
	for (vector<sLONG>::iterator cur = inWafSel->rows.begin(), end = inWafSel->rows.end(); cur != end; cur++)
	{
		sLONG n = GetFic(*cur);
		outWafSelBits.Set(n);					
	}

	for (WafSelectionRangeVector::iterator cur = inWafSel->ranges.begin(), end = inWafSel->ranges.end(); cur != end; cur++)
	{
		sLONG debut = cur->start;
		sLONG fin = cur->end;
		for (sLONG i = debut; i <= fin; i++)
		{
			sLONG n = GetFic(i);
			outWafSelBits.Set(n);
		}

	}

	for (vector<sLONG>::iterator cur = inWafSel->butRows.begin(), end = inWafSel->butRows.end(); cur != end; cur++)
	{
		sLONG n = GetFic(*cur);
		outWafSelBits.Clear(n);					
	}

	return err;
}



VError Selection::ExportToSQL(CDB4DSelection* xsel, Table* target, BaseTaskInfo* context, VFolder* inBaseFolder, VProgressIndicator* inProgress, ExportOption& options)
{
	VError err = VE_OK;
	if (GetQTfic() > 0)
	{
		if (inProgress != nil)
		{
			XBOX::VString session_title;
			VValueBag bag;
			VStr<64> tname;
			target->GetName(tname);
			bag.SetString("TableName", tname);
			bag.SetString("curValue", L"{curValue}");
			bag.SetString("maxValue", L"{maxValue}");
			gs(1005,31,session_title);
			session_title.Format(&bag);
			inProgress->BeginSession(GetQTfic(), session_title, true);
		}

		sLONG maxblobs;
		if (IsRemoteLike())
			maxblobs = GetQTfic()*3;
		else
			maxblobs = parentfile->GetMaxBlobs(context);

		ExportJob job(target, inBaseFolder, inProgress, maxblobs, options);
		err = job.StartJob(GetQTfic());
		if (err == VE_OK)
		{
			if (IsRemoteLike())
			{
				VDB4DFieldCacheCollection *fieldcache = new VDB4DFieldCacheCollection(target);
				FieldsForCache* fields = fieldcache->GetFieldsCache();
				for (sLONG i = 1,nb = target->GetNbCrit(); i <= nb; i++)
				{
					Field* ff = target->RetainField(i);
					if (ff != nil)
						fields->AddField(ff);
					QuickReleaseRefCountable(ff);
				}
				CDB4DRemoteRecordCache* remotecache = nil;
				sLONG count = GetQTfic();
				for (sLONG i = 0; i < count && err == VE_OK; i++)
				{
					if (i%100 == 0)
					{
						sLONG i2 = i + 100;
						if (i2 >= count)
							i2 = count - 1;
						QuickReleaseRefCountable(remotecache);
						vector<uBYTE> none;
						remotecache = context->GetEncapsuleur()->StartCachingRemoteRecords(xsel, i, i2, fieldcache, none);
						if (remotecache == nil)
							err = ThrowError(VE_DB4D_CANNOTLOADRECORD, noaction);
					}
					if (remotecache != nil)
					{
						vector<CachedRelatedRecord> relatedrecs;
						CDB4DRecord* xfic = nil;
						err = remotecache->RetainCacheRecord(i, xfic, relatedrecs);
						if (xfic != nil)
						{
							FicheInMem* fic = dynamic_cast<VDB4DRecord*>(xfic)->GetRec();
							if (fic != nil)
							{
								err = job.WriteOneRecord(fic, context);
							}
						}
						QuickReleaseRefCountable(xfic);
					}
				}
				QuickReleaseRefCountable(remotecache);
			}
			else
			{
				SelectionIterator itersel(this);

				ReadAheadBufferActivator buf(parentfile->GetDB());
				sLONG currec = itersel.FirstRecord();
				while (currec != -1 && err == VE_OK)
				{
					FicheInMem* fic = parentfile->LoadRecord(currec, err, DB4D_Do_Not_Lock, context, false, false, buf.GetBuffer());
					if (fic != nil)
					{
						err = job.WriteOneRecord(fic, context);
						fic->Release();
					}

					currec = itersel.NextRecord();
				}
			}
			err = job.StopJob();
		}

		if (inProgress != nil)
		{
			inProgress->EndSession();
		}
	}
	return err;
}



/* ------------------------------------- */



PetiteSel::PetiteSel(DataTable *xDF, Base4D* remoteDB, sLONG numtableRemote) : Selection(xDF, remoteDB)
{
	fTabMem=nil;
	typsel=sel_petitesel;
	fIsRemote = false;
	fNumTableRemote = numtableRemote;
	fQTFicToWrite = 0;
#if debugLeakCheck_PetiteSel
	if (debug_candumpleaks)
		DumpStackCrawls();
	if (debug_canRegisterLeaks)
		RegisterStackCrawl(this);
#endif
}


PetiteSel::PetiteSel(Base4D* remoteDB, CDB4DBaseContext* inContext, sLONG numtableRemote) : Selection(remoteDB, inContext)
{
	fTabMem=nil;
	typsel=sel_petitesel;
	fIsRemote = false;
	fNumTableRemote = numtableRemote;
	fQTFicToWrite = 0;
#if debugLeakCheck_PetiteSel
	if (debug_candumpleaks)
		DumpStackCrawls();
	if (debug_canRegisterLeaks)
		RegisterStackCrawl(this);
#endif
}


PetiteSel::~PetiteSel()
{
#if debugLeakCheck_PetiteSel
	UnRegisterStackCrawl(this);
#endif
	if (fKeepRecordLocked)
	{
		xbox_assert(!fIsRemote && !IsOwnerRemote());
		UnLockRecords(fLockingContext);
	}

	if (fTabMem!=nil)
	{
		FreeFastMem( fTabMem);
		fTabMem=nil;
	}
}


void PetiteSel::CheckFieldsForCache()
{
	if (fRequestedFieldsMaster != nil)
	{
		if (parentfile != nil)
		{
			fRequestedFieldsMaster->AddSel(parentfile->GetNum());
		}
		else
		{
			fRequestedFieldsMaster->AddSel(fNumTableRemote);
		}
	}
}



sLONG PetiteSel::CalcLenOnDisk(void)
{
	return(8+(qtfic<<2));
}


VError PetiteSel::LoadSel(DataBaseObjectHeader& tag)
{
	sLONG nbb;
	VError err;

	xbox_assert(!fIsRemote);
	err=db->readlong(&nbb,4,getaddr(),4+kSizeDataBaseObjectHeader);
	if (err==VE_OK)
	{
		if (tag.NeedSwap())
			nbb = SwapLong(nbb);
		if (FixFic(nbb))
		{
			if (nbb > 0)
			{
				err=db->readlong( fTabMem,nbb<<2,getaddr(),8+kSizeDataBaseObjectHeader);
				if (err == VE_OK)
					err = tag.ValidateCheckSum(fTabMem,nbb<<2);
				if (err == VE_OK)
				{
					if (tag.NeedSwap())
						ByteSwap(fTabMem, nbb);
				}
			}
			antelen=8+(nbb<<2);
		}
		else
		{
			err = ThrowError(memfull, DBaction_LoadingSelection);
		}
	}

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOT_LOAD_SEL, DBaction_LoadingSelection);

	return(err);
}

void PetiteSel::setmodif(uBOOL xismodif, Base4D* bd, BaseTaskInfo* context)
{
	if (xismodif)
	{
		fQTFicToWrite = qtfic;
	}
	Selection::setmodif(xismodif, bd, context);
}

bool PetiteSel::SaveObj(VSize& outSizeSaved)
{
#if debuglogwrite
	VString wherefrom(L"PetiteSel Saveobj");
	VString* whx = &wherefrom;
#else
	VString* whx = nil;
#endif

	sLONG tot,len;

	if (SelisCluster)
	{
		xbox_assert(!fIsRemote);
		/*
		if (VInterlocked::AtomicGet(&fStopWrite) != 0)
		{
			outSizeSaved = 0;
			return false;
		}
		else
		*/
		{
			tot=0;
			if (fTabMem!=nil)
			{
				sLONG xqt = VInterlocked::AtomicGet(&fQTFicToWrite);
				len=xqt<<2;
				DataBaseObjectHeader tag(fTabMem, len, DBOH_PetiteSel, -1, -1 /*GetParentFile()->GetNum()*/);
				tag.WriteInto(db, getaddr(), whx);
				db->writelong(&typsel,4,getaddr(),kSizeDataBaseObjectHeader, whx);
				db->writelong(&xqt,4,getaddr(),kSizeDataBaseObjectHeader+4, whx);
				db->writelong(fTabMem,len,getaddr(),kSizeDataBaseObjectHeader+8, whx);
				tot=len+8;
			}
			outSizeSaved = tot;
		}
	}
	else
	{
		outSizeSaved = 0;
	}

	return true;
}

/*
void PetiteSel::OktoSave(BaseTaskInfo* context)
{
	sLONG len;
	DataAddr4D ou;
	VError err;

	len=8+(qtfic<<2);
	if ( (antelen==0) || ( adjuste(len+kSizeDataBaseObjectHeader) != adjuste(antelen+kSizeDataBaseObjectHeader) ) )
	{
		DataAddr4D oldaddr = getaddr();
		sLONG oldantelen = antelen;
		ou=db->findplace(len+kSizeDataBaseObjectHeader, context, err, -kIndexSegNum, this);
		ChangeAddr(ou, db, context);
		antelen=len;
		if (oldantelen!=0) 
			db->libereplace(oldaddr ,oldantelen+kSizeDataBaseObjectHeader, context, this);
	}
}
*/


sLONG PetiteSel::GetFic(sLONG n, Boolean canlock)
{
	sLONG result;
	if ( (n<0) || (n>=qtfic) )
	{
		result = -1;
	}
	else
	{
		result = fTabMem[n];
	}

	return (result);
}


VError PetiteSel::PutFic(sLONG n, sLONG r, Boolean canlock)
{
	VError err = VE_OK;

	if ( testAssert(n>=0) && (n<qtfic) )
	{
		fTabMem[n]=r;
	}

	Touch();
	return(err);
}


uBOOL PetiteSel::FixFic(sLONG nbb, Boolean ForARemoteReceive)
{
	uBOOL ok;
	VError err = VE_OK;

	if (nbb == 0)
	{
		if (fTabMem!=nil)
			FreeFastMem(fTabMem);
		fTabMem = nil;
	}
	else
	{
		if (fTabMem!=nil)
		{
			if (adjuste(nbb*4) > adjuste(qtfic*4))
			{
				fTabMem = (sLONG*)ResizePtr(fTabMem, adjuste(qtfic*4), adjuste(nbb*4));
				if (fTabMem == nil)
					err = ThrowError(memfull, DBaction_BuildingSelection);
			}
		}
		else
		{
			fTabMem = (sLONG*)GetFastMem( adjuste(nbb*4), false, 'psel');
			if (fTabMem == nil)
				err = ThrowError(memfull, DBaction_BuildingSelection);
		}
	}
	ok=(err==VE_OK);
	if (ok)
	{
		qtfic=nbb;
	}

	Touch();
	return(ok);
}


VError PetiteSel::UpdateFromBittab(Bittab *b)
{
	sLONG nb,i,ou;
	VError err;

	err=VE_OK;
	nb=b->Compte();
	if (FixFic(nb))
	{
		if (nb>0)
		{
			err = b->FillArray(fTabMem, nb);
		}
	}
	else 
		err = ThrowError(memfull, DBaction_BuildingSelection);

	Touch();

	//## generer exception ici
	return(err);
}


sLONG PetiteSel::FindInSel(sLONG n)
{
	sLONG i,res;

	res=-1;

	if (qtfic>0)
	{
		sLONG *p = fTabMem;
		for (i=0;i<qtfic;i++)
		{
			if (*p==n)
			{
				res=i;
				break;
			}
			++p;
		}
	}

	return(res);
}


VError PetiteSel::FillArray(sLONG* tab, sLONG maxelem)
{
	if (qtfic <= maxelem)
		vBlockMove(fTabMem, tab, qtfic*sizeof(sLONG));
	else
		return ThrowError(VE_DB4D_ARRAYLIMIT_IS_EXCEEDED, DBaction_BuildingArrayOfValues);
	return VE_OK;
}


VError PetiteSel::FillPosInfoArray(SelPosInfo* tab, sLONG fromindice, sLONG toindice)
{
	sLONG* source = fTabMem+fromindice;
	SelPosInfo* dest = tab;
	for (sLONG i = fromindice; i <= toindice; i++)
	{
		dest->fRecNum = *source;
		dest->fSelPos = i - fromindice;
		dest++;
		source++;
	}
	return VE_OK;
}



VError PetiteSel::AddToSel(sLONG n)
{
	sLONG deja;
	VError err = VE_OK;

	deja=FindInSel(n);
	if (deja==-1)
	{
		if (FixFic(qtfic+1))
		{
			fTabMem[qtfic-1]=n;
		}
		else 
			err = ThrowError(memfull, DBaction_BuildingSelection);
	}
	Touch();

	//## generer exception ici
	return(err);
}


Selection* PetiteSel::AddToSelection(sLONG recID, VError& err)
{
	Selection* res = this;
	err = VE_OK;

	if (qtfic < kNbElemInSel)
	{
		if (FixFic(qtfic+1))
		{
			fTabMem[qtfic-1] = recID;
		}
		else 
			err = ThrowError(memfull, DBaction_BuildingSelection);
	}
	else
	{
		res = new LongSel(parentfile, db);
		if (res == nil)
			err = ThrowError(memfull, DBaction_BuildingSelection);
		else
		{
			((LongSel*)res)->PutInCache();
			if (res->FixFic(qtfic+1))
			{
				sLONG i;
				sLONG* p = fTabMem;
				for (i = 0; i < qtfic && err == VE_OK; i++)
				{
					err = res->PutFic(i,*p);
					p++;
				}
			}
			else
				err = ThrowError(memfull, DBaction_BuildingSelection);

			if (err == VE_OK)
			{
				err = res->PutFic(qtfic, recID);
			}

			if (err != VE_OK)
			{
				res->Release();
				res = this;
			}

		}
	}
	Touch();

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTBUILDSELECTION, DBaction_BuildingSelection);

	return res;
}


VError PetiteSel::DelFromSel(Bittab* b)
{
	VError err = VE_OK;

	sLONG nb = qtfic;

	if (b->Compte() > 0 && qtfic > 0)
	{
		sLONG* pdest = fTabMem;
		sLONG* pcur = fTabMem;
		sLONG i;
		for (i=0; i<nb; i++)
		{
			if (b->isOn(*pcur))
			{
				qtfic--;
			}
			else
			{
				*pdest = *pcur;
				pdest++;
			}
			pcur++;
		}
	}

	Touch();

	return err;
}


VError PetiteSel::DelFromSel(sLONG n)
{
	sLONG deja,i;
	VError err = VE_OK;

	deja=FindInSel(n);

	if (deja!=-1)
	{
		sLONG *p = fTabMem;
		p += deja;
		for (i=qtfic-deja-1;i>0;i--)
		{
			*p=*(p+1);
			p++;
		}
		qtfic--;
	}

	Touch();
	return(err);
}


Bittab* PetiteSel::GenereBittab(BaseTaskInfo* context, VError& err)
{
	Bittab* bb;
	sLONG nb,i;
	err = VE_OK;

	bb = new Bittab;
	if (bb!=nil)
	{
		if (parentfile != nil)
			err=bb->aggrandit(parentfile->GetMaxRecords(context));
		if (err==VE_OK)
		{
			sLONG *p = fTabMem;

			nb=qtfic;
			for (i=0;i<nb;i++)
			{
				bb->Set(*p, true);
				p++;
			}

			bb->Epure();
		}
		else
		{
			bb->Release();
			bb=nil;
		}
	}
	else
		err = ThrowError(memfull, DBaction_BuildingSelection);

	return(bb);
}


VError PetiteSel::PutIntoBittab(Bittab *bb)
{
	sLONG nb,i,tot,lll;
	VError err;

	err=VE_OK;
	if (bb!=nil)
	{
		tot = 0;
		sLONG *p = fTabMem + qtfic;
		nb=qtfic;
		for (i=0;(i<nb) && (err==VE_OK);i++)
		{
			p--;
			lll=*p;
			err = bb->Set(lll, true);
		}

	}

	return(err);
}


VError PetiteSel::AddToBittab(Bittab *b, BaseTaskInfo* context)
{
	return(PutIntoBittab(b));
}


VError PetiteSel::LibereEspaceDisk(OccupableStack* curstack, VDB4DProgressIndicator* InProgress)
{
	sLONG len;
	DataAddr4D ou;
	VError err = VE_OK;

	if (SelisCluster)
	{
		ou = getaddr();
		if (ou>0)
		{
			len = CalcLenOnDisk();
			setmodif(false, db, nil);
#if debugIndexOverlap_strong
			di_IndexOverLap::RemoveCluster(nil, -1, ou, len+kSizeDataBaseObjectHeader);
#endif
			db->libereplace(ou, len+kSizeDataBaseObjectHeader, nil, this);
		}
	}

	return err;
}


void PetiteSel::DupliqueInto(SelectionPtr& newsel)
{
	PetiteSel *newpetite;

	newpetite = new PetiteSel(parentfile, db);
	// newpetite->occupe();
//	xbox_assert(newpetite->FixFic(qtfic));
	BaseCopyInto(newpetite);
	if (fTabMem == nil || qtfic == 0)
		newpetite->fTabMem = nil;
	else
	{
		newpetite->fTabMem = (sLONG*)GetFastMem(adjuste(qtfic*4), false, 'psel');
		vBlockMove(fTabMem, newpetite->fTabMem, qtfic*sizeof(sLONG));
	}

	newsel = newpetite;
	newsel->Touch();
}


Boolean PetiteSel::LockRecords(BaseTaskInfo* context)
{
	Boolean ok = true;
	sLONG i;

	xbox_assert(!fIsRemote && !IsOwnerRemote());
	if (!fKeepRecordLocked)
	{
		if (fTabMem != nil)
		{
			for (i=0; i<qtfic; i++)
			{
				if (! parentfile->LockRecord(fTabMem[i], context))
				{
					UnLockRecords(context, i-1);
					ok = false;
					break;
				}
			}
		}

		if (ok)
		{
			context->AddLockingSelection(this);
			fKeepRecordLocked = true;
		}
	}

	return ok;
}


Boolean PetiteSel::UnLockRecords(BaseTaskInfo* context, sLONG upto, Boolean RemoveFromContext)
{
	sLONG i;
	if (upto == -2)
		upto = qtfic-1;

	xbox_assert(!fIsRemote && !IsOwnerRemote());
	if (fKeepRecordLocked)
	{
		if (fTabMem != nil)
		{
			for (i=0; i<=upto; i++)
			{
				parentfile->UnlockRecord(fTabMem[i],context);
			}
		}
		fKeepRecordLocked= false;
		context->RemoveLockingSelection(this, RemoveFromContext);
	}

	return true;
}


VError PetiteSel::FillWith(const void* data, sLONG sizeElem, sLONG nbElem, Boolean ascent, sLONG maxrecords, sLONG startFrom ,bool useIndirection, Bittab* inWafSel, WafSelection* outWafSel)
{
	VError err;

//	xbox_assert(fTabMem != nil);

	if (FixFic(nbElem+startFrom))
	{
		err = VE_OK;
		sLONG i;
		sLONG *p = fTabMem+startFrom;
		if (ascent)
		{
			const char  *p2 = (const char*)data;
			for (i = 0; i < nbElem; i++)
			{
				sLONG recnum;
				if (useIndirection)
					recnum = **((sLONG**)p2);
				else
					recnum = *((sLONG*)p2);
				if (recnum >= 0 && recnum < maxrecords)
				{
					if (inWafSel != nil && outWafSel != nil)
					{
						if (inWafSel->isOn(recnum))
							outWafSel->AddElem();
						else
							outWafSel->SkipElem();
					}
					*p = recnum;
					p++;
					p2 = p2 + sizeElem;
				}
				else
				{
					err = ThrowBaseError(VE_DB4D_WRONGRECORDID, DBaction_BuildingSelection);
					break;
				}
			}
		}
		else
		{
			const char  *p2 = (const char*)data;
			p2 = p2 + (sizeElem * (nbElem-1));
			for (i = 0; i < nbElem; i++)
			{
				sLONG recnum;
				if (useIndirection)
					recnum = **((sLONG**)p2);
				else
					recnum = *((sLONG*)p2);
				if (recnum >= 0 && recnum < maxrecords)
				{
					if (inWafSel != nil && outWafSel != nil)
					{
						if (inWafSel->isOn(recnum))
							outWafSel->AddElem();
						else
							outWafSel->SkipElem();
					}
					*p = recnum;
					p++;
					p2 = p2 - sizeElem;
				}
				else
				{
					err = ThrowBaseError(VE_DB4D_WRONGRECORDID, DBaction_BuildingSelection);
					break;
				}
			}
		}
	}
	else
	{
		err = ThrowBaseError( memfull, DBaction_BuildingSelection);
	}

	Touch();
	return err;
}


Boolean PetiteSel::IntersectBittab(Bittab* b, BaseTaskInfo* context, VError& err)
{
	Boolean res = false;
	err = VE_OK;
	sLONG i;
	sLONG *p = fTabMem;

	xbox_assert(fTabMem != nil);
	if (p != nil)
	{
		for (i = 0; i < qtfic; i++)
		{
			if (b->isOn(*p))
			{
				res = true;
				break;
			}
			p++;
		}
	}

	return res;

}


VError PetiteSel::BuildWithOneRec(sLONG inRecNum)
{

	VError err = VE_OK;
	if (fTabMem == nil)
	{
		fTabMem = (sLONG*)GetFastMem( adjuste(4), false, 'psel');
		if (fTabMem == nil)
			err = ThrowError(memfull, DBaction_BuildingSelection);
	}
	if (fTabMem != nil)
	{
		qtfic = 1;
		fTabMem[0] = inRecNum;
	}
	Touch();
	return err;
}


sLONG PetiteSel::GetRecordPos(sLONG inRecordID, BaseTaskInfo* context)
{
	if (fTabMem == nil)
		return -1;
	else
	{
		sLONG* p = fTabMem;
		sLONG i;
		for (i=0; i<qtfic; i++, p++)
		{
			if (*p == inRecordID)
				return i;
		}
	}
	return -1;
}


Bittab* PetiteSel::GenerateSetFromRange(sLONG inRecordIndex1, sLONG inRecordIndex2, VError& err, BaseTaskInfo* context)
{
	err = VE_OK;
	Bittab* res = new Bittab;
	if (res == nil)
		err = ThrowError(memfull, DBaction_BuildingSelection);
	else
	{
		if (fTabMem != nil)
		{
			if (inRecordIndex1>inRecordIndex2)
			{
				sLONG temp = inRecordIndex1;
				inRecordIndex1 = inRecordIndex2;
				inRecordIndex2 = temp;
			}
			if (inRecordIndex1<0)
				inRecordIndex1 = 0;
			if (inRecordIndex2>=qtfic)
				inRecordIndex2 = qtfic-1;
			err = res->FillFromArray((char*)(fTabMem+inRecordIndex1), sizeof(sLONG), inRecordIndex2-inRecordIndex1+1, 0x7FFFFFFF, false);
		}
	}

	if (err != VE_OK)
	{
		ReleaseRefCountable(&res);
	}

	return res;
}


Bittab* PetiteSel::GenerateSetFromRecordID(sLONG inRecordID, sLONG inRecordIndex2, VError& err, BaseTaskInfo* context)
{
	Bittab* res = nil;
	sLONG inRecordIndex1 = GetRecordPos(inRecordID, context);
	if (inRecordIndex1 != -1)
		res = GenerateSetFromRange(inRecordIndex1, inRecordIndex2, err, context);
	return res;
}


VError PetiteSel::RemoveSelectedRange(sLONG inRecordIndex1, sLONG inRecordIndex2, BaseTaskInfo* context)
{
	VError err = VE_OK;

	if (inRecordIndex1<0)
		inRecordIndex1 = 0;
	if (inRecordIndex2>=qtfic)
		inRecordIndex2 = qtfic-1;
	if (inRecordIndex2-inRecordIndex1+1 >= qtfic)
	{
		qtfic = 0;
	}
	else
	{
		if (inRecordIndex2 >= (qtfic-1))
		{
			qtfic = inRecordIndex1;
		}
		else
		{
			if (fTabMem != nil)
			{
				sLONG* p = fTabMem + inRecordIndex1;
				sLONG* p2 = fTabMem + inRecordIndex2+1;
				sLONG* end = fTabMem + qtfic;
				while (p2 < end)
				{
					*p = *p2;
					p++;
					p2++;
				}
				qtfic = qtfic - (inRecordIndex2-inRecordIndex1+1);
			}
		}
	}

	return err;
}



VError PetiteSel::FromServer(VStream* from, CDB4DBaseContext* inContext)
{
	VError err = VE_OK;

	err = from->GetLong(fNumTableRemote);
	sLONG nb = 0;
	if (err == VE_OK)
		err = from->GetLong(nb);
	if (err == VE_OK)
	{
		if (FixFic(nb))
		{
			if (nb > 0)
			{
				err = from->GetLongs(fTabMem, &nb);
				if (nb != qtfic)
				{
					err = ThrowError(VE_DB4D_CANNOTBUILDSELECTION, noaction);
				}
			}
		}
		else
			err = ThrowBaseError(memfull, noaction);
	}

	return err;

}

VError PetiteSel::ToClient(VStream* into, CDB4DBaseContext* inContext)
{
	VError err = VE_OK;

	err = into->PutLong(GetTypSel());
	if (err == VE_OK)
		err = into->PutLong(parentfile->GetNum());
	if (err == VE_OK)
		err = into->PutLong(qtfic);
	if (err == VE_OK && qtfic > 0)
		err = into->PutLongs(fTabMem, qtfic);

	return err;
}



VError PetiteSel::FromClient(VStream* from, CDB4DBaseContext* inContext)
{
	VError err = VE_OK;

	sLONG numtable = 0;

	err = from->GetLong(numtable);

	Table* tt = db->RetainTable(numtable);
	if (tt != nil)
	{
		parentfile = tt->GetDF();
		tt->Release();
	}

	if (parentfile == nil)
		err = ThrowError(VE_DB4D_CANNOTBUILDSELECTION, noaction);
	else
	{
		sLONG nb = 0;
		if (err == VE_OK)
			err = from->GetLong(nb);
		if (err == VE_OK)
		{
			if (FixFic(nb))
			{
				if (nb > 0)
				{
					err = from->GetLongs(fTabMem, &nb);
					if (nb != qtfic)
					{
						err = ThrowError(VE_DB4D_CANNOTBUILDSELECTION, noaction);
					}
				}
			}
			else
				err = ThrowBaseError(memfull, noaction);
		}
	}
	return err;

}


VError PetiteSel::ToServer(VStream* into, CDB4DBaseContext* inContext)
{
	VError err = VE_OK;

	err = into->PutLong(GetTypSel());
	err = into->PutLong(fNumTableRemote);
	if (err == VE_OK)
		err = into->PutLong(qtfic);
	if (err == VE_OK && qtfic > 0)
		err = into->PutLongs(fTabMem, qtfic);

	return err;
}


bool PetiteSel::MatchAllocationNumber(sLONG allocationNumber)
{
	if (allocationNumber == -1)
		return true;
	if (GetAllocationNumber(this) == allocationNumber)
		return true;
	if (fTabMem != nil && GetAllocationNumber(fTabMem) == allocationNumber)
		return true;

	return false;
}






												/* ------------------------------------- */



sLONG ConstVectorSel::FindInSel(sLONG n)
{
	sLONG i,res;

	res=-1;

	if (qtfic>0)
	{
		const sLONG *p = &fVector[0];
		for (i=0;i<qtfic;i++)
		{
			if (*p==n)
			{
				res=i;
				break;
			}
			++p;
		}
	}

	return(res);
}



VError ConstVectorSel::FillArray(sLONG* tab, sLONG maxelem)
{
	if (qtfic <= maxelem)
		std::copy(fVector.begin(), fVector.end(), tab);
	else
		return ThrowError(VE_DB4D_ARRAYLIMIT_IS_EXCEEDED, DBaction_BuildingArrayOfValues);
	return VE_OK;
}


VError ConstVectorSel::FillPosInfoArray(SelPosInfo* tab, sLONG fromindice, sLONG toindice)
{
	const sLONG* source = &fVector[fromindice];
	SelPosInfo* dest = tab;
	for (sLONG i = fromindice; i <= toindice; i++)
	{
		dest->fRecNum = *source;
		dest->fSelPos = i - fromindice;
		dest++;
		source++;
	}
	return VE_OK;
}



Bittab* ConstVectorSel::GenereBittab(BaseTaskInfo* context, VError& err)
{
	Bittab* bb;
	sLONG nb,i;
	err = VE_OK;

	bb = new Bittab;
	if (bb!=nil)
	{
		if (parentfile != nil)
			err=bb->aggrandit(parentfile->GetMaxRecords(context));
		if (err==VE_OK)
		{
			const sLONG *p = &fVector[0];

			nb=qtfic;
			for (i=0;i<nb;i++)
			{
				bb->Set(*p, true);
				p++;
			}

			bb->Epure();
		}
		else
		{
			bb->Release();
			bb=nil;
		}
	}
	else
		err = ThrowError(memfull, DBaction_BuildingSelection);

	return(bb);
}


VError ConstVectorSel::PutIntoBittab(Bittab *bb)
{
	sLONG nb,i,tot,lll;
	VError err;

	err=VE_OK;
	if (bb!=nil)
	{
		tot = 0;
		const sLONG *p = &fVector[0];
		p = p + qtfic;
		nb=qtfic;
		for (i=0;(i<nb) && (err==VE_OK);i++)
		{
			p--;
			lll=*p;
			err = bb->Set(lll, true);
		}

	}

	return(err);
}


VError ConstVectorSel::AddToBittab(Bittab *b, BaseTaskInfo* context)
{
	return(PutIntoBittab(b));
}



Boolean ConstVectorSel::LockRecords(BaseTaskInfo* context)
{
	Boolean ok = true;
	sLONG i;

	xbox_assert(!fIsRemote && !IsOwnerRemote());
	if (!fKeepRecordLocked)
	{
		{
			for (i=0; i<qtfic; i++)
			{
				if (! parentfile->LockRecord(fVector[i], context))
				{
					UnLockRecords(context, i-1);
					ok = false;
					break;
				}
			}
		}

		if (ok)
		{
			context->AddLockingSelection(this);
			fKeepRecordLocked = true;
		}
	}

	return ok;
}


Boolean ConstVectorSel::UnLockRecords(BaseTaskInfo* context, sLONG upto, Boolean RemoveFromContext)
{
	sLONG i;
	if (upto == -2)
		upto = qtfic-1;

	xbox_assert(!fIsRemote && !IsOwnerRemote());
	if (fKeepRecordLocked)
	{
		{
			for (i=0; i<=upto; i++)
			{
				parentfile->UnlockRecord(fVector[i],context);
			}
		}
		fKeepRecordLocked= false;
		context->RemoveLockingSelection(this, RemoveFromContext);
	}

	return true;
}


Boolean ConstVectorSel::IntersectBittab(Bittab* b, BaseTaskInfo* context, VError& err)
{
	Boolean res = false;
	err = VE_OK;
	sLONG i;
	const sLONG *p = &fVector[0];

	if (p != nil)
	{
		for (i = 0; i < qtfic; i++)
		{
			if (b->isOn(*p))
			{
				res = true;
				break;
			}
			p++;
		}
	}

	return res;

}



sLONG ConstVectorSel::GetRecordPos(sLONG inRecordID, BaseTaskInfo* context)
{
	{
		const sLONG* p = &fVector[0];
		sLONG i;
		for (i=0; i<qtfic; i++, p++)
		{
			if (*p == inRecordID)
				return i;
		}
	}
	return -1;
}


Bittab* ConstVectorSel::GenerateSetFromRange(sLONG inRecordIndex1, sLONG inRecordIndex2, VError& err, BaseTaskInfo* context)
{
	err = VE_OK;
	Bittab* res = new Bittab;
	if (res == nil)
		err = ThrowError(memfull, DBaction_BuildingSelection);
	else
	{
		const sLONG* fTabMem = &fVector[0];
		if (fTabMem != nil)
		{
			if (inRecordIndex1>inRecordIndex2)
			{
				sLONG temp = inRecordIndex1;
				inRecordIndex1 = inRecordIndex2;
				inRecordIndex2 = temp;
			}
			if (inRecordIndex1<0)
				inRecordIndex1 = 0;
			if (inRecordIndex2>=qtfic)
				inRecordIndex2 = qtfic-1;
			err = res->FillFromArray((const char*)(fTabMem+inRecordIndex1), sizeof(sLONG), inRecordIndex2-inRecordIndex1+1, 0x7FFFFFFF, false);
		}
	}

	if (err != VE_OK)
	{
		ReleaseRefCountable(&res);
	}

	return res;
}


Bittab* ConstVectorSel::GenerateSetFromRecordID(sLONG inRecordID, sLONG inRecordIndex2, VError& err, BaseTaskInfo* context)
{
	Bittab* res = nil;
	sLONG inRecordIndex1 = GetRecordPos(inRecordID, context);
	if (inRecordIndex1 != -1)
		res = GenerateSetFromRange(inRecordIndex1, inRecordIndex2, err, context);
	return res;
}



												/* ------------------------------------- */




LongSel::LongSel(DataTable *xDF, Base4D* remoteDB) : Selection(xDF, remoteDB)
{
	fTabMem=nil;
	fFile=nil;
	fFileDesc=nil;
	typsel=sel_longsel;
	//fRemoteContext = nil; /* Sergiy - August 20, 2007 - Uninitializaed class member was causing a crash in the destructor. */
	fIDHasBeenGenerated = false;
	fCurLockPage = -1;
}


LongSel::LongSel(Base4D* remoteDB, CDB4DBaseContext* inContext) : Selection(remoteDB, inContext)
{
	/*
	fRemoteContext = inContext;
	if (fRemoteContext != nil)
		fRemoteContext->Retain();
		*/
	fTabMem=nil;
	fFile=nil;
	fFileDesc=nil;
	typsel=sel_longsel;
	fIDHasBeenGenerated = false;
	fCurLockPage = -1;
}


LongSel::~LongSel()
{	
	RemoveFromCache();
	if (fKeepRecordLocked)
	{
		xbox_assert(!fIsRemote);
		UnLockRecords(fLockingContext);
	}
	DeleteSel();
	if ( fFileDesc != nil )
	{
		delete fFileDesc;
		fFileDesc = NULL;
	}
	if ( fFile )
	{
		fFile->Delete();
		fFile->Release();
	}
	/*
	if (fRemoteContext != nil)
		fRemoteContext->Release();
		*/
}


void LongSel::DeleteSel()
{
	if (fTabMem!=nil)
	{
		sLONG nb = (qtfic+kNbElemInSel-1) >> kRatioSel;
		sLONG* *p = fTabMem;
		
		for ( sLONG i=0;i<nb;i++)
		{
			if (*p != nil && *p != (sLONG*)-1)
				FreeFastMem( *p);
			++p;
		}
		FreeFastMem( fTabMem);
		fTabMem=nil;
		qtfic=0;
	}
	
}


uBOOL LongSel::OpenSelFile(uBOOL doiscreate)
{
	VStr63 s;
	sLONG_PTR n;
	uBOOL ok = false;
	sLONG nb;
	VError err = VE_OK;
	
	if (fFile==nil)
	{
		n=reinterpret_cast<sLONG_PTR>(this);
		n=n>>4;
		s.FromLong( n);	//s.FromULong(n,NF_HEXA8);
		s += ".tmp";
		
		//delete fFileDesc;
		fFileDesc = NULL;

		VFolder *tempfolder = db->RetainTemporaryFolder( true, &err);
		if (tempfolder != nil)
		{
			fFile=new VFile(*tempfolder, s);
			tempfolder->Release();

			if (doiscreate)
			{
				fFile->Delete(); 
				err=fFile->Create( true);
			}
		}
		
		if (err == VE_OK)
		{
			err=fFile->Open( FA_READ_WRITE, &fFileDesc );
			if (err==VE_OK)
			{
				ok=true;
			}
		}
		
		if (ok)
		{
			nb=(qtfic+kNbElemInSel-1) >> kRatioSel;
			err=fFileDesc->SetSize(nb*kSizePageSelDisk);
			if (err!=VE_OK)
			{
				ok=false;
				delete fFileDesc;
				fFileDesc = NULL;
				fFile->Delete();
			}
		}
		
		if (!ok)
		{
			XBOX::ReleaseRefCountable( &fFile);
		}
	}
	else
	{
		ok=true;
	}
	
	return(ok);
}


sLONG* LongSel::loadMem(sLONG n1, Boolean canCreate, Boolean canlock)
{
	SetCurLockPage(n1);
	sLONG* tsh = fTabMem[n1];
	
	if (tsh == nil)
	{
		if (fIsRemote)
		{
			IRequest* req = db->CreateRequest(nil /*fRemoteContext*/, Req_AskForASelectionPart + kRangeReqDB4D);
			if (req != nil)
			{
				req->PutBaseParam(db);
				req->PutSelectionParam(this, nil);
				req->PutLongParam(n1);
				VError err = req->GetLastError();
				if (err == VE_OK)
				{
					err = req->Send();
					if (err == VE_OK)
					{
						tsh = (sLONG*)GetFastMem( kSizePageSel, false, 'lsel');
						if (tsh != nil)
						{
							err = req->GetArrayLongReply(tsh, kNbElemInSel);
							if (err != VE_OK)
							{
								FreeFastMem(tsh);
								tsh = nil;
							}
							else
								tsh[kNbElemInSel]=2;
						}
						
					}
				}
				req->Release();
			}

		}
		else
		{
			if (SelisCluster)
			{
				tsh = (sLONG*)GetFastMem( kSizePageSel, false, 'lsel');
				if (tsh!=nil)
				{
					db->readlong( tsh, kSizePageSelDisk,getaddr(),8+(n1*kSizePageSelDisk)+kSizeDataBaseObjectHeader);
					tsh[kNbElemInSel]=2;
				}
			}
			else
			{
				if (OpenSelFile(false))
				{
					tsh = (sLONG*)GetFastMem(kSizePageSel, false, 'lsel');
					if (tsh!=nil)
					{
						VError err = fFileDesc->GetData( tsh, kSizePageSelDisk, n1*kSizePageSelDisk);
						if (err != VE_OK)
						{
							FreeFastMem(tsh);
							tsh = nil;
						}
						else
							tsh[kNbElemInSel]=2;
					}
				}
			}
		}
		
		if (tsh!=nil)
		{
			fTabMem[n1] = tsh;
		}
	}
	else
	{
		if (tsh == (sLONG*)-1)
		{
			if (canCreate)
			{
				tsh = (sLONG*)GetFastMem( kSizePageSel, false, 'lsel');
				if (tsh != nil)
				{
					tsh[kNbElemInSel] = 2;
					fTabMem[n1] = tsh;
				}
			}
			else
				tsh = nil;
		}
		else
			RetainMem(tsh);
	}
	
	ClearCurLockPage();

	return(tsh);
}


bool LongSel::FreeMem(sLONG allocationBlockNumber, VSize combien, VSize& outSizeFreed)
{
	sLONG i,nb;
	VError err;
	bool okfree = false;

	VSize tot=0;

	if (combien==0) 
		combien=kMaxPositif;

	sLONG curlock = VInterlocked::AtomicGet(&fCurLockPage);
	if (curlock != -2)
	{
		if (fTabMem!=nil)
		{
			sLONG* *tshp = fTabMem;

			nb=(qtfic+kNbElemInSel-1)>>kRatioSel;

			if (SelisCluster)
			{
				for (i=0;(i<nb)/* && (tot<combien)*/;i++)
				{
					sLONG curlock = VInterlocked::AtomicGet(&fCurLockPage);
					if (curlock != i)
					{
						if (*tshp!=nil)
						{
							sLONG *p = *tshp;
							if (VInterlocked::AtomicGet(&(p[kNbElemInSel]))==0 && OKAllocationNumber(p, allocationBlockNumber))
							{
								tot += kSizePageSel;
								FreeFastMem( p);
								*tshp=nil;
							}
						}
					}
					tshp++;
				}
			}
			else
			{
				for (i=0;(i<nb) /*&& (tot<combien)*/;i++)
				{
					sLONG curlock = VInterlocked::AtomicGet(&fCurLockPage);
					if (curlock != i)
					{
						if (*tshp!=nil && *tshp != (sLONG*)-1)
						{
							if (fIsRemote)
							{
								sLONG *p = *tshp;
								sLONG val = VInterlocked::AtomicGet(&(p[kNbElemInSel]));
								if (!isMemRetained(val) && !isMemModified(val) && OKAllocationNumber(p, allocationBlockNumber))
								{
									tot += kSizePageSel;
									FreeFastMem( p);
									*tshp=nil;
								}
							}
							else
							{
								if (OpenSelFile())
								{
									sLONG *p = *tshp;
									sLONG val = VInterlocked::AtomicGet(&(p[kNbElemInSel]));
									if (!isMemRetained(val) && OKAllocationNumber(p, allocationBlockNumber))
									{
										if (isMemModified(val))
										{
											err=fFileDesc->PutData(p, kSizePageSelDisk, i*kSizePageSelDisk);
										}
										tot += kSizePageSel;
										FreeFastMem( p);
										*tshp=nil;
									}
								}
							}
						}
					}
					tshp++;
				}
			}

		}
		okfree = true;
		ClearMemRequest();
	}

	outSizeFreed = tot;

	return(okfree);
}


bool LongSel::SaveObj(VSize& outSizeSaved)
{
#if debuglogwrite
	VString wherefrom(L"LongSel SaveObj");
	VString* whx = &wherefrom;
#else
	VString* whx = nil;
#endif

	sLONG tot = 0;
	sLONG i,nb;
	Boolean allsaved = true;
	
	if (SelisCluster)
	{
		/*
		if (VInterlocked::AtomicGet(&fStopWrite) != 0)
		{
			outSizeSaved = 0;
			return false;
		}
		else
		*/
		{
			xbox_assert(!fIsRemote);
			if (fTabMem!=nil)
			{
				DataBaseObjectHeader tag(nil, 0, DBOH_LongSel, -1, -1 /*GetParentFile()->GetNum()*/);
				tag.WriteInto(db, getaddr(), whx);
				db->writelong(&typsel,4,getaddr(),kSizeDataBaseObjectHeader, whx);
				db->writelong(&qtfic,4,getaddr(),kSizeDataBaseObjectHeader+4, whx);
				sLONG* *tshp = fTabMem;
				nb=(qtfic+kNbElemInSel-1)>>kRatioSel;
				
				for (i=0;i<nb;i++)
				{
					if (*tshp!=nil)
					{
						sLONG *p = *tshp;
						if (isMemModified(p))
						{
							if (isMemRetained(p))
							{
								allsaved = false;
							}
							else
							{
								db->writelong(p,kSizePageSelDisk,getaddr(),kSizeDataBaseObjectHeader+8+(i*kSizePageSelDisk), whx);
								tot += kSizePageSelDisk;
								p[kNbElemInSel] = 0;
							}
						}
						tshp++;
					}
				}
			}
		}
	}

	outSizeSaved = tot;
	return true;
}


sLONG LongSel::CalcLenOnDisk(void)
{
	sLONG nb;
	
	nb=(qtfic+kNbElemInSel-1) >> kRatioSel;
	return(8+nb*kSizePageSelDisk);
}


VError LongSel::LoadSel(DataBaseObjectHeader& tag)
{
	VError err;

	sLONG nbb;
	err=db->readlong(&nbb,4,getaddr(),4+kSizeDataBaseObjectHeader);
	if (err==VE_OK)
	{
		if (FixFic(nbb))
		{
		}
		else
		{
			err = ThrowError(memfull, DBaction_LoadingSelection);
		}
	}
	
	if (err != VE_OK)
		err = err = ThrowError(VE_DB4D_CANNOT_LOAD_SEL, DBaction_LoadingSelection);

	return(err);
}
		

sLONG LongSel::GetFic(sLONG n, Boolean canlock)
{
	sLONG n1,xn2,result;
		
	if ( (n<0) || (n>=qtfic) )
	{
		result = -1;
	}
	else
	{
		n1 = n>>kRatioSel;
		xn2 = n & (kNbElemInSel-1);

		sLONG* tsh = loadMem(n1, false, canlock);
		if (tsh!=nil)
		{
			result = tsh[xn2];
			ReleaseMem(tsh);
		}
		else
		{
			result=-1;
		}
	}
	
	return(result);
}


VError LongSel::PutFic(sLONG n, sLONG r, Boolean canlock)
{
	sLONG n1,xn2;
	VError err = VE_OK;
	

	if ( testAssert(n>=0) && (n<qtfic) )
	{
		n1=n>>kRatioSel;
		xn2=n & (kNbElemInSel-1);

		sLONG* tsh = loadMem(n1, true, canlock);
		
		if (tsh!=nil)
		{
			tsh[xn2]=r;
			ModifMem(tsh);
			ReleaseMem(tsh);
		}
		else
		{
			err = ThrowError(memfull, DBaction_BuildingSelection);
		}
	}
	
	Touch();	
	
	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTBUILDSELECTION, DBaction_BuildingSelection);

	return(err);
}
		

void LongSel::ClearRemoteCache(VStream* fromServer)
{
	if (fTabMem != nil)
	{
		LockAllPages();
		sLONG nb = (qtfic+kNbElemInSel-1) >> kRatioSel;
		for (sLONG **cur = fTabMem, **end = fTabMem+nb; cur != end; cur++)
		{
			sLONG* p = *cur;
			if (p != nil && p != (sLONG*)-1 && p != (sLONG*)-2)
			{
				FreeFastMem(p);
				*cur = nil;
			}
		}
		ClearCurLockPage();
	}
}


uBOOL LongSel::FixFic(sLONG nbb, Boolean ForARemoteReceive)
{
	sLONG nb, oldnb;
	uBOOL ok;
	
	LockAllPages();
	VError err = VE_OK;
	if (nbb == 0)
	{
		if (fTabMem != nil)
		{
			oldnb = (qtfic+kNbElemInSel-1) >> kRatioSel;
			if (oldnb>0)
			{
				sLONG* *tshp = fTabMem;
				sLONG i;
				if (ForARemoteReceive)
				{
					for (i = 0; i<oldnb; i++)
					{
						if (*tshp != nil && (*tshp != (sLONG*)-1))
							FreeFastMem(*tshp);
						*tshp = nil;
						++tshp;
					}
				}
				else
				{
					for (i = 0; i<oldnb; i++)
					{
						if (*tshp != nil && (*tshp != (sLONG*)-1))
							FreeFastMem(*tshp);
						*tshp = (sLONG*)-1;
						++tshp;
					}
				}
			}
		}
	}
	else
	{
		oldnb = (qtfic+kNbElemInSel-1) >> kRatioSel;
		nb=(nbb+kNbElemInSel-1) >> kRatioSel;
		if (fTabMem==nil)
		{
			fTabMem = (sLONG**)GetFastMem( adjuste(nb*sizeof(sLONG*)), false, 'Lsel');
			if (fTabMem == nil)
				err = ThrowError(memfull, DBaction_BuildingSelection);
			else
			{
				if (ForARemoteReceive)
					_raz(fTabMem, nb*sizeof(sLONG*) );
				else
					_rau(fTabMem, nb*sizeof(sLONG*) );
			}
		}
		else
		{
			if ( adjuste(oldnb*sizeof(sLONG*)) < adjuste(nb*sizeof(sLONG*)) )
			{
				fTabMem = (sLONG**)ResizePtr(fTabMem, adjuste(oldnb*sizeof(sLONG*)), adjuste(nb*sizeof(sLONG*)) );
			}
			
			if (nb > oldnb)
			{
				sLONG* *tshp = fTabMem+oldnb;
				sLONG i;
				if (ForARemoteReceive)
				{
					for (i = oldnb+1; i<=nb; i++)
					{
						*tshp = nil;
						++tshp;
					}
				}
				else
				{
					for (i = oldnb+1; i<=nb; i++)
					{
						*tshp = (sLONG*)-1;
						++tshp;
					}
				}
			}
			else
			{
				if (nb<oldnb)
				{
					sLONG* *tshp = fTabMem+nb;
					sLONG i;
					if (ForARemoteReceive)
					{
						for (i = nb+1; i<=oldnb; i++)
						{
							if (*tshp != nil && *tshp != (sLONG*)-1)
								FreeFastMem(*tshp);
							*tshp = nil;
							++tshp;
						}
					}
					else
					{
						for (i = nb+1; i<=oldnb; i++)
						{
							if (*tshp != nil && *tshp != (sLONG*)-1)
								FreeFastMem(*tshp);
							*tshp = (sLONG*)-1;
							++tshp;
						}
					}
				}
			}
			
		}
	}
	
	ClearCurLockPage();

	ok=(err==VE_OK);
	if (ok)
	{
		qtfic=nbb;
	}
	
	Touch();
	
	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTBUILDSELECTION, DBaction_BuildingSelection);
	return(ok);
}


VError LongSel::FillPosInfoArray(SelPosInfo* tab, sLONG fromindice, sLONG toindice)
{
	SelPosInfo* dest = tab;

	VError err = VE_OK;
	sLONG* *tshp = fTabMem;
	sLONG i, nb, nb2, pos = 0;

	nb=(toindice + 1+kNbElemInSel-1) >> kRatioSel;
	sLONG startpage = fromindice >> kRatioSel;
	sLONG startj = fromindice & (kNbElemInSel-1);
	for (i=startpage; i<nb; i++)
	{
		sLONG* tsh = loadMem(i);;

		if (tsh!=nil)
		{
			if (i==(nb-1))
			{
				nb2 = (toindice+1) & (kNbElemInSel-1);
				if (nb2 == 0)
					nb2 = kNbElemInSel;
			}
			else
				nb2 = kNbElemInSel;

			sLONG* source = tsh+startj;
			for (sLONG j= startj; j < nb2; j++)
			{
				dest->fRecNum = *source;
				dest->fSelPos = pos;
				dest++;
				source++;
				pos++;
			}
			startj = 0;
			ReleaseMem(tsh);
		}
		else
		{
			err = ThrowError(memfull, DBaction_BuildingArrayOfValues);
		}
	}

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOT_BUILD_ARRAYOFVALUES, DBaction_BuildingArrayOfValues);

	return err;
}


VError LongSel::FillArray(sLONG* tab, sLONG maxelem)
{

	VError err = VE_OK;
	sLONG* *tshp = fTabMem;
	sLONG i, nb, nb2;

	if (qtfic > maxelem)
		return VE_DB4D_ARRAYLIMIT_IS_EXCEEDED;

	nb=(qtfic+kNbElemInSel-1) >> kRatioSel;
	for (i=0;i<nb;i++)
	{
		sLONG* tsh = loadMem(i);;

		if (tsh!=nil)
		{
			if (i==(nb-1))
			{
				nb2 = qtfic & (kNbElemInSel-1);
				if (nb2 == 0)
					nb2 = kNbElemInSel;
			}
			else
				nb2 = kNbElemInSel;
			//vBlockMove(tsh, tab, nb2*sizeof(sLONG));
			std::copy(tsh, tsh+nb2, tab);
			tab = tab + nb2;
			ReleaseMem(tsh);
		}
		else
		{
			err = ThrowError(memfull, DBaction_BuildingArrayOfValues);
		}
	}

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOT_BUILD_ARRAYOFVALUES, DBaction_BuildingArrayOfValues);

	return err;
}


VError LongSel::UpdateFromBittab(Bittab *b)
{
	sLONG nb,i,ou;
	VError err = VE_OK;
	
	nb=b->Compte();
	if (FixFic(nb))
	{
		if (nb>0)
		{
			/*
			ou=-1;
			for (i=0;(i<nb) && (err==VE_OK);i++)
			{
				ou=b->FindNextBit(ou+1);
				err=PutFic(i,ou, false);
			}
			*/
			err = b->PutIntoLongSel(this);
		}
	}
	else 
		err = ThrowError(memfull, DBaction_BuildingSelection);
	
	Touch();

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTBUILDSELECTION, DBaction_BuildingSelection);

	return(err);
}


Bittab* LongSel::GenereBittab(BaseTaskInfo* context, VError& err)
{
	Bittab* bb;
	sLONG nb,i,j,nb2;
	err = VE_OK;
	
	bb = new Bittab;
	if (bb!=nil)
	{
		if (parentfile != nil)
			err=bb->aggrandit(parentfile->GetMaxRecords(context));
		if (err==VE_OK)
		{
			bb->raz();
			sLONG* *tshp = fTabMem;
			
			nb=(qtfic+kNbElemInSel-1) >> kRatioSel;
			for (i=0;i<nb;i++)
			{
				sLONG* tsh = loadMem(i);

				if (tsh!=nil)
				{
					if (i==nb-1)
					{
						nb2=qtfic & (kNbElemInSel-1);
						if (nb2 == 0)
							nb2 = kNbElemInSel;
					}
					else
						nb2 = kNbElemInSel;
					sLONG *p = tsh;
					for (j=0; j<nb2; j++)
					{
						err = bb->Set(*p, true);
						if (err != VE_OK)
							break;
						p++;
					}
					ReleaseMem(tsh);
				}
				else
				{
					err = ThrowError(memfull, DBaction_BuildingSelection);
					break;
				}
			}
			
			bb->Epure();
		}
		else
		{
			ReleaseRefCountable(&bb);
		}
	}
	else
		err = ThrowError(memfull, DBaction_BuildingSelection);

	if (err != VE_OK && bb != nil)
	{
		bb->Release();
		bb = nil;
	}

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTBUILDSELECTION, DBaction_BuildingSelection);

	return(bb);
}


VError LongSel::AddToBittab(Bittab *b, BaseTaskInfo* context)
{
	sLONG nb,i,j,nb2;
	
	VError err = VE_OK;
	if (b!=nil)
	{
		if (parentfile != nil)
			err=b->aggrandit(parentfile->GetMaxRecords(context));
		if (err==VE_OK)
		{
			b->use();
			sLONG* *tshp = fTabMem;
			
			nb=(qtfic+kNbElemInSel-1) >> kRatioSel;
			for (i=0;i<nb;i++)
			{
				sLONG* tsh = loadMem(i);

				if (tsh!=nil)
				{
					if (i==nb-1)
					{
						nb2=qtfic & (kNbElemInSel-1);
						if (nb2 == 0)
							nb2 = kNbElemInSel;
					}
					else
						nb2=kNbElemInSel; 
					sLONG *p = tsh;
					for (j=0; j<nb2; j++)
					{
						err = b->Set(*p, true);
						if (err != VE_OK)
							break;
						p++;
					}
					ReleaseMem(tsh);
				}
				else
				{
					err = ThrowError(memfull, DBaction_BuildingSelection);
					break;
				}
			}
			
			b->unuse();
		}
	}
	
	return(err);
}


void LongSel::DupliqueInto(SelectionPtr& newsel)
{
	sLONG i;
	VError err = VE_OK;
	
	LongSel *newlongue = new LongSel(parentfile, db);
	
	if (newlongue == nil)
		err = ThrowError(memfull, DBaction_BuildingSelection);
	else
	{
		((LongSel*)newlongue)->PutInCache();
		if (newlongue->FixFic(qtfic))
		{
			BaseCopyInto(newlongue);
			sLONG nb=(qtfic+kNbElemInSel-1) >> kRatioSel;
			
			for (i=0; i<nb; i++)
			{
				sLONG* tshsource = loadMem(i);
				if (tshsource != nil)
				{
					sLONG* tsh = newlongue->loadMem(i, true);
					
					if (tsh != nil)
					{
						vBlockMove((void*)tshsource, (void*)tsh, (VSize)kSizePageSelDisk);
						ModifMem(tsh);
						ReleaseMem(tsh);
					}
					else
					{
						err = ThrowError(memfull, DBaction_BuildingSelection);
						break;
					}
					ReleaseMem(tshsource);
				}
				else
				{
					err = ThrowError(memfull, DBaction_BuildingSelection);
					break;
				}
			}
		}
		else
			err = ThrowError(memfull, DBaction_BuildingSelection);

		if (err != VE_OK)
		{
			newlongue->Release();
			newlongue = nil;
		}
	}
	
	newsel = newlongue;
	newsel->Touch();
}



Boolean LongSel::LockRecords(BaseTaskInfo* context)
{
	Boolean ok = true;
	sLONG i;

	if (!fKeepRecordLocked)
	{
		xbox_assert(!fIsRemote && !IsOwnerRemote());
		for (i=0; i<qtfic; i++)
		{
			if (! parentfile->LockRecord(GetFic(i), context))
			{
				UnLockRecords(context, i-1);
				ok = false;
				break;
			}
		}

		if (ok)
		{
			context->AddLockingSelection(this);
			fKeepRecordLocked = true;
		}
	}

	return ok;
}


Boolean LongSel::UnLockRecords(BaseTaskInfo* context, sLONG upto, Boolean RemoveFromContext)
{
	sLONG i;
	if (upto == -2)
		upto = qtfic-1;

	if (fKeepRecordLocked)
	{
		xbox_assert(!fIsRemote && !IsOwnerRemote());
		for (i=0; i<=upto; i++)
		{
			parentfile->UnlockRecord(GetFic(i),context);
		}

		fKeepRecordLocked = false;
		context->RemoveLockingSelection(this, RemoveFromContext);
	}

	return true;
}


Selection* LongSel::AddToSelection(sLONG recID, VError& err)
{
	err = VE_OK;

	if (FixFic(qtfic+1))
	{
		err = PutFic(qtfic-1, recID);
	}
	else
		err = ThrowError(memfull, DBaction_BuildingSelection);

	Touch();

	return this;
}


VError LongSel::xDelFromSel(Bittab* b, sLONG n)
{
	VError err = VE_OK;

	//occupe();

	sLONG oldqtfic = qtfic;
	sLONG newqtfic = qtfic;

	if (b->Compte() > 0 && newqtfic > 0)
	{
		sLONG* *tpdest = fTabMem;
		sLONG* *tpcur = fTabMem;

		sLONG* pdest = loadMem(0);
		sLONG* pcur = loadMem(0);

		sLONG pagedest = 0, ndest = 0;
		sLONG pagecur = 0, ncur = 0;

		sLONG i;
		for (i=0; i<oldqtfic; i++)
		{
			Boolean match = false;
			if (b == nil)
			{
				match = (pcur[ncur] == n);
			}
			else
			{
				match = b->isOn(pcur[ncur]);
			}

			if (match)
			{
				newqtfic--;
			}
			else
			{
				if (pagecur == pagedest && ncur == ndest)
				{
					// rien a faire car on a pas encore decale un seul
				}
				else
				{
					pdest[ndest] = pcur[ncur];
					//pdest[kNbElemInSel] = pdest[kNbElemInSel] | 1;
					ModifMem(pdest);
				}
				ndest++;
				if (ndest >= kNbElemInSel)
				{
					ndest = 0;
					pagedest++;
					if (pdest != pcur)
						ReleaseMem(pdest);
					pdest = loadMem(pagedest);
				}
			}
			ncur++;
			if (ncur >= kNbElemInSel)
			{
				ncur = 0;
				pagecur++;
				if (pcur != pdest)
					ReleaseMem(pcur);
				pcur = loadMem(pagecur);
			}
		}

		ReleaseMem(pdest);
		if (pcur != pdest)
			ReleaseMem(pcur);

		qtfic = newqtfic;
		sLONG nbpage = (qtfic+kNbElemInSel-1) >> kRatioSel;
		sLONG oldnbpage = (oldqtfic+kNbElemInSel-1) >> kRatioSel;
		for (i=nbpage; i<oldnbpage; i++)
		{
			pcur = fTabMem[i];
			if (pcur != nil)
				FreeFastMem((void*)pcur);
			fTabMem[i] = nil;
		}
	}

	Touch();
	//libere();

	return err;
}


VError LongSel::DelFromSel(sLONG n)
{
	return(xDelFromSel(nil, n));
}


VError LongSel::DelFromSel(Bittab* b)
{
	return(xDelFromSel(b, -1));
}


VError LongSel::FillWith(const void* data, sLONG sizeElem, sLONG nbElem, Boolean ascent, sLONG maxrecords, sLONG startFrom, bool useIndirection, Bittab* inWafSel, WafSelection* outWafSel)
{
	//VObjLock lock(this);

	VError err;
//	occupe();

	if (FixFic(nbElem+startFrom))
	{
		err = VE_OK;
		sLONG i, count = startFrom % kNbElemInSel, counttsh = startFrom / kNbElemInSel;
		sLONG *tsh = nil;
		sLONG *p;
		const char  *p2 = (const char*)data;
		if (!ascent)
			p2 = p2 + (sizeElem * (nbElem-1));
		for (i = 0; i < nbElem; i++)
		{
			if (tsh == nil)
			{
				tsh = loadMem(counttsh, true);
				if (tsh == nil)
				{
					err = ThrowBaseError( memfull, DBaction_BuildingSelection);
					break;
				}
				ModifMem(tsh);
				p = tsh+count;
			}
			sLONG recnum;
			if (useIndirection)
				recnum = **((sLONG**)p2);
			else
				recnum = *((sLONG*)p2);
			if (recnum >= 0 && recnum < maxrecords)
			{
				if (inWafSel != nil && outWafSel != nil)
				{
					if (inWafSel->isOn(recnum))
						outWafSel->AddElem();
					else
						outWafSel->SkipElem();
				}
				*p = recnum;
			}
			else
			{
				err = ThrowBaseError(VE_DB4D_WRONGRECORDID, DBaction_BuildingSelection);
				break;
			}
			p++;
			if (ascent)
				p2 = p2 + sizeElem;
			else
				p2 = p2 - sizeElem;
			count++;
			if (count == kNbElemInSel)
			{
				if (tsh != nil)
					ReleaseMem(tsh);
				count = 0;
				counttsh++;
				tsh = nil;
			}
		}

		if (tsh != nil)
		{
			ReleaseMem(tsh);
		}
	}
	else
	{
		err = ThrowBaseError( memfull, DBaction_BuildingSelection);
	}

	Touch();
	//libere();

	return err;
}


Boolean LongSel::IntersectBittab(Bittab* b, BaseTaskInfo* context, VError& err)
{
	err = VE_DB4D_NOTIMPLEMENTED;
	return false;
}




sLONG LongSel::GetRecordPos(sLONG inRecordID, BaseTaskInfo* context)
{
	//VObjLock lock(this);

	VError err = VE_OK;
	sLONG* *tshp = fTabMem;
	sLONG i, nb, nb2;

	nb=(qtfic+kNbElemInSel-1) >> kRatioSel;
	for (i=0;i<nb;i++)
	{
		sLONG* tsh = loadMem(i);

		if (tsh!=nil)
		{
			if (i==nb-1)
				nb2 = qtfic & (kNbElemInSel-1);
			else
				nb2 = kNbElemInSel;

			sLONG* p = tsh;
			sLONG j;
			for (j=0; j<nb2; j++, p++)
			{
				if (*p == inRecordID)
				{
					ReleaseMem(tsh);
					return (i*kNbElemInSel)+j;
				}
			}
			ReleaseMem(tsh);
		}
	}

	return -1;
}


Bittab* LongSel::GenerateSetFromRange(sLONG inRecordIndex1, sLONG inRecordIndex2, VError& err, BaseTaskInfo* context)
{
	//VObjLock lock(this);

	err = VE_OK;
	Bittab* res = new Bittab;
	if (res == nil)
		err = ThrowError(memfull, DBaction_BuildingSelection);
	else
	{
		res->occupe();
		if (inRecordIndex1>inRecordIndex2)
		{
			sLONG temp = inRecordIndex1;
			inRecordIndex1 = inRecordIndex2;
			inRecordIndex2 = temp;
		}
		if (inRecordIndex1<0)
			inRecordIndex1 = 0;
		if (inRecordIndex2>=qtfic)
			inRecordIndex2 = qtfic-1;

		sLONG startpage = inRecordIndex1>>kRatioSel;
		sLONG startindice = inRecordIndex1 & (kNbElemInSel-1);
		sLONG endpage = inRecordIndex2>>kRatioSel;
		sLONG endindice = inRecordIndex2 & (kNbElemInSel-1);
		sLONG i, start, end;

		sLONG* *tshp = fTabMem;

		for (i=startpage; i<=endpage && err == VE_OK; i++)
		{
			if (i==startpage)
				start = startindice;
			else
				start = 0;

			if (i == endpage)
				end = endindice;
			else
				end = kNbElemInSel-1;

			sLONG* tsh = loadMem(i);

			if (tsh!=nil)
			{
				err = res->FillFromArray((char*)(tsh+start), sizeof(sLONG), end-start+1, 0x7FFFFFFF, false);
				ReleaseMem(tsh);
			}
			else
			{
				err = ThrowError(memfull, DBaction_BuildingSelection);
			}
		}
		res->libere();
	}

	if (err != VE_OK)
	{
		if (res != nil)
			delete res;
		res = nil;
	}

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTBUILDSELECTION, DBaction_BuildingSelection);

	return res;
}


Bittab* LongSel::GenerateSetFromRecordID(sLONG inRecordID, sLONG inRecordIndex2, VError& err, BaseTaskInfo* context)
{
	Bittab* res = nil;
	sLONG inRecordIndex1 = GetRecordPos(inRecordID, context);
	if (inRecordIndex1 != -1)
		res = GenerateSetFromRange(inRecordIndex1, inRecordIndex2, err, context);
	return res;
}



VError LongSel::RemoveSelectedRange(sLONG inRecordIndex1, sLONG inRecordIndex2, BaseTaskInfo* context)
{
	//VObjLock lock(this);
	VError err = VE_OK;
	sLONG newqtfic = qtfic;

	if (inRecordIndex1<0)
		inRecordIndex1 = 0;
	if (inRecordIndex2>=qtfic)
		inRecordIndex2 = qtfic-1;
	if (inRecordIndex2-inRecordIndex1+1 >= qtfic)
	{
		newqtfic = 0;
	}
	else
	{
		if (inRecordIndex2 >= (qtfic-1))
		{
			newqtfic = inRecordIndex1;
		}
		else
		{
			if (fTabMem != nil)
			{
				sLONG page1 = inRecordIndex1>>kRatioSel;
				sLONG indice1 = inRecordIndex1 & (kNbElemInSel-1);
				sLONG page2 = inRecordIndex2>>kRatioSel;
				sLONG indice2 = inRecordIndex2 & (kNbElemInSel-1);

				sLONG maxpage = (qtfic + kNbElemInSel - 1) >> kRatioSel;

				sLONG* tsh1 = loadMem(page1);
				sLONG* tsh2 = loadMem(page2);
				sLONG* p1 = tsh1;
				sLONG* p2 = tsh2;

				if (p1 == nil || p2 == nil)
					err = ThrowError(memfull, DBaction_BuildingSelection);
				else
				{
					p1 = p1 + indice1;
					p2 = p2 + indice2 + 1;
					sLONG* p1End = p1 + kNbElemInSel;
					sLONG* p2End = p2 + kNbElemInSel;

					for (sLONG i = inRecordIndex2+1; i < qtfic; i++)
					{
						*p1 = *p2;
						p1++;
						if (p1>=p1End)
						{
							xbox_assert(p1 == p1End);
							*p1 = 1;
							page1++;
							ModifMem(tsh1);
							if (tsh1 != tsh2)
								ReleaseMem(tsh1);

							tsh1 = loadMem(page1);
							p1 = tsh1;
							if (p1 == nil)
							{
								err = ThrowError(memfull, DBaction_BuildingSelection);
								break;
							}
							p1End = p1 + kNbElemInSel;
						}

						p2++;
						if (p2>=p2End)
						{
							xbox_assert(p2 == p2End);
							page2++;
							if (page2 < maxpage)
							{
								if (tsh1 != tsh2)
									ReleaseMem(tsh2);
								tsh2 = loadMem(page2);
								p2 = tsh2;
								if (p2 == nil)
								{
									err = ThrowError(memfull, DBaction_BuildingSelection);
									break;
								}
								p2End = p2 + kNbElemInSel;
							}
						}
					}

					if (tsh1 != nil)
					{
						ModifMem(tsh1);
						ReleaseMem(tsh1);
					}
					if (tsh2 != tsh1 && tsh2 != nil)
						ReleaseMem(tsh2);
				}

				newqtfic = qtfic - (inRecordIndex2-inRecordIndex1+1);
			}
		}
	}

	if (err == VE_OK)
		FixFic(newqtfic);
	else
		err = ThrowError(VE_DB4D_CANNOTBUILDSELECTION, DBaction_BuildingSelection);

	return err;
}




VError LongSel::FromServer(VStream* from, CDB4DBaseContext* inContext)
{
	VError err = VE_OK;

	VUUID xid;
	err = xid.ReadFromStream(from);
	xid.ToBuffer(fID);
	fIDHasBeenGenerated = true;
	sLONG nb = 0;
	if (err == VE_OK)
		err = from->GetLong(nb);
	if (err == VE_OK)
	{
		if (FixFic(nb, true))
		{
			if (nb > 0)
			{
				sLONG* tsh = loadMem(0, true);
				if (tsh == nil)
					err = ThrowBaseError(memfull, noaction);
				else
				{
					sLONG nb = qtfic;
					if (nb > kNbElemInSel)
						nb = kNbElemInSel;
					sLONG nb2 = nb;
					err = from->GetLongs(tsh, &nb);
					if (nb != nb2)
					{
						err = ThrowError(VE_DB4D_CANNOTBUILDSELECTION, noaction);
					}
					ReleaseMem(tsh);
				}
			}
		}
		else
			err = ThrowBaseError(memfull, noaction);
	}

	return err;

}


VError LongSel::ToClient(VStream* into, CDB4DBaseContext* inContext)
{
	VError err = VE_OK;

	err = into->PutLong(GetTypSel());
	if (!fIDHasBeenGenerated)
	{
		GenerateID();
		fIDHasBeenGenerated = true;
	}
	VUUID xid(fID);
	err = xid.WriteToStream(into);
	if (err == VE_OK)
		err = into->PutLong(qtfic);
	if (qtfic > 0)
	{
		if (err == VE_OK)
		{
			sLONG* tsh = loadMem(0);
			if (tsh == nil)
				err = ThrowBaseError(memfull, noaction);
			else
			{
				sLONG nb = qtfic;
				if (nb > kNbElemInSel)
					nb = kNbElemInSel;
				err = into->PutLongs(tsh, nb);
				ReleaseMem(tsh);
			}
		}
	}

	if (err == VE_OK)
	{
		VDBMgr::GetManager()->KeepSelectionOnServer(this);
	}
	return err;
}



VError LongSel::FromClient(VStream* from, CDB4DBaseContext* inContext)
{
	// est appele dans le cas ou la selection a ete construite sur le client
	VError err = VE_OK;

	VUUID newid;
	err = newid.ReadFromStream(from);
	if (err == VE_OK)
	{
		fIDHasBeenGenerated = true;
		fID = newid.GetBuffer();
	}

	sLONG newqtfic;
	if (err == VE_OK)
		err = from->GetLong(newqtfic);
	if (err == VE_OK)
	{
		if (!FixFic(newqtfic))
			err = ThrowBaseError(memfull, noaction);
	}
	if (qtfic > 0 && err == VE_OK)
	{
		sLONG nbpage = (qtfic+kNbElemInSel-1) >> kRatioSel;
		for (sLONG i = 0; i < nbpage && err == VE_OK; i++)
		{
			sLONG* tsh = loadMem(i, true, true);
			if (tsh == nil)
				err = ThrowBaseError(memfull, noaction);
			else
			{
				sLONG nb = kNbElemInSel;
				if (i == (nbpage - 1))
				{
					nb = qtfic & (kNbElemInSel-1);
					if (nb == 0)
						nb = kNbElemInSel;
				}
				err = from->GetLongs(tsh, &nb);
				ModifMem(tsh);
				ReleaseMem(tsh);
			}
		}
	}

	return err;

}


VError LongSel::ToServer(VStream* into, CDB4DBaseContext* inContext)
{
	VError err = VE_OK;

	if (fIsRemote)
	{
		err = into->PutLong(GetTypSel());
		xbox_assert(fIDHasBeenGenerated);
		VUUID xid(fID);
		err = xid.WriteToStream(into);
	}
	else
	{
		err = into->PutLong(sel_longsel_fullfromclient);
		VUUID newid;
		newid.Regenerate();
		fID = newid.GetBuffer();
		fIDHasBeenGenerated = true;
		err = newid.WriteToStream(into);

		if (err == VE_OK)
			err = into->PutLong(qtfic);
		if (qtfic > 0)
		{
			sLONG nbpage = (qtfic+kNbElemInSel-1) >> kRatioSel;
			for (sLONG i = 0; i < nbpage && err == VE_OK; i++)
			{
				sLONG* tsh = loadMem(i);
				if (tsh == nil)
					err = ThrowBaseError(memfull, noaction);
				else
				{
					sLONG nb = kNbElemInSel;
					if (i == (nbpage - 1))
					{
						nb = qtfic & (kNbElemInSel-1);
						if (nb == 0)
							nb = kNbElemInSel;
					}
					err = into->PutLongs(tsh, nb);
					ReleaseMem(tsh);
				}
			}
		}

		if (err == VE_OK)
			fIsRemote = true;

	}
	return err;
}


VError LongSel::GetPartReply(sLONG numpart, IRequestReply *inRequest)
{
	VError err = VE_OK;
	sLONG nb=(qtfic+kNbElemInSel-1)>>kRatioSel;
	if (numpart<0 || numpart >= nb)
		err = ThrowError(VE_DB4D_INVALID_SELECTION_PART, noaction);
	else
	{
		sLONG* tsh = loadMem(numpart);
		if (tsh != nil)
		{
			err = inRequest->PutArrayLongReply(tsh, kNbElemInSel);
			ReleaseMem(tsh);
		}
		else
			err = ThrowError(VE_DB4D_INVALID_SELECTION_PART, noaction);
	}
	return err;
}


bool LongSel::MatchAllocationNumber(sLONG allocationNumber)
{
	if (allocationNumber == -1)
		return true;
	if (GetAllocationNumber(this) == allocationNumber)
		return true;
	if (fTabMem != nil && GetAllocationNumber(fTabMem) == allocationNumber)
		return true;
	if (fTabMem != nil)
	{
		sLONG nbpage = (qtfic+kNbElemInSel-1) >> kRatioSel;
		for (sLONG i = 0; i < nbpage; i++)
		{
			sLONG* tsh = fTabMem[i];
			if (tsh != nil && tsh != (sLONG*)-1 && tsh != (sLONG*)-2)
			{
				if (GetAllocationNumber(tsh) == allocationNumber)
					return true;
			}
		}

	}

	return false;
}





												/* ------------------------------------- */



void BitSel::setmodif(uBOOL xismodif, Base4D* bd, BaseTaskInfo* context)
{
	if (xismodif)
	{
		pTB->PourSetModif();
	}
	Selection::setmodif(xismodif, bd, context);
}


bool BitSel::SaveObj(VSize& outSizeSaved)
{
	if (SelisCluster)
	{
		/*
		if (VInterlocked::AtomicGet(&fStopWrite) != 0)
		{
			outSizeSaved = 0;
			return false;
		}
		else
		*/
			outSizeSaved = pTB->PourSaveobj(getaddr());
	}
	else
	{
		outSizeSaved = 0;
	}
	return true;
}

/*
void BitSel::OktoSave(BaseTaskInfo* context)
{
	sLONG len;
	DataAddr4D ou;
	VError err;

	len=pTB->CalcLenOnDisk();
	if ( (antelen==0) || (adjuste(len+kSizeDataBaseObjectHeader) != adjuste(antelen+kSizeDataBaseObjectHeader)) )
	{
		DataAddr4D oldaddr = getaddr();
		sLONG oldantelen = antelen;

		ou=db->findplace(len+kSizeDataBaseObjectHeader, context, err, -kIndexSegNum, this);
		ChangeAddr(ou, db, context);
		pTB->SetAddr(ou,db);
		antelen=len;

		if (oldantelen!=0) 
			db->libereplace(oldaddr,oldantelen+kSizeDataBaseObjectHeader, context, this);
	}
}
*/

sLONG BitSel::GetFic(sLONG n, Boolean canlock)
{
	return(pTB->PlaceDuNiemeBitAllume(n));
}

VError BitSel::PutFic(sLONG n, sLONG r, Boolean canlock)
{
	VError err = VE_OK;
	pTB->ClearOrSet(r, true, true);
	//recalcnbfic=true;
	Touch();
	return(err);
}


void BitSel::ReduceSel(sLONG nb)
{
	pTB->Reduit(nb);
	//recalcnbfic=true;
	Touch();
}


uBOOL BitSel::FixFic(sLONG nbb, Boolean ForARemoteReceive)
{
	VError err;
	
	err=pTB->aggrandit(nbb);
	//recalcnbfic=true;
	Touch();
	return(err==VE_OK);
}

sLONG BitSel::GetQTfic(void)
{ 
	CheckFieldsForCache();
	if (recalcnbfic)
	{
		qtfic=pTB->Compte();
		recalcnbfic=false;
	}
	return(qtfic);
};


Boolean BitSel::IsEmpty()
{
	CheckFieldsForCache();
	return pTB->IsEmpty();
}


sLONG BitSel::GetNextRecord(sLONG fromRecord, sLONG FromIndice)
{
	return pTB->FindNextBit(fromRecord);
}


sLONG BitSel::GetPreviousRecord(sLONG fromRecord, sLONG FromIndice)
{
	return pTB->FindPreviousBit(fromRecord);
}


sLONG BitSel::GetLastRecord()
{
	return pTB->FindPreviousBit(pTB->GetMaxBit()-1);
}


VError BitSel::UpdateFromBittab(Bittab *b)
{
	//occupe();
	
	pTB->vide();
	pTB->Release();
	pTB = b;
	pTB->Retain();
	/*
	*pTB=*b;
	b->cut();
	*/
	//recalcnbfic=true;
	
	Touch();
	//libere();
	return(VE_OK);
}


void BitSel::SetSelAddr(DataAddr4D addr)
{
	Selection::SetSelAddr(addr);
	pTB->SetAddr(addr,db);
}


void BitSel::ChangeAddr(DataAddr4D addr, Base4D* bd, BaseTaskInfo* context)
{
	Selection::ChangeAddr(addr, bd, context);
	pTB->SetAddr(addr, db);
}


sLONG BitSel::CalcLenOnDisk(void)
{
	return(pTB->CalcLenOnDisk());
}


VError BitSel::LoadSel(DataBaseObjectHeader& tag)
{
	VError err;
	
	err=pTB->LoadBitSel(tag);
	antelen=pTB->GetLenDisk();
	
	//## generer exception ici
	return(err);
}


VError BitSel::AddToSel(sLONG n)
{
	VError err;
	pTB->use();
	pTB->aggranditParBlock(n);
	err=pTB->Set(-n, true);
	pTB->unuse();
	//recalcnbfic=true;
	Touch();
	return(err);
}


Selection* BitSel::AddToSelection(sLONG recID, VError& err)
{
	err = VE_OK;
	Selection* res = CreFromBittab_nobitsel(parentfile, pTB, db);
	if (res == nil)
	{
		err = ThrowError(memfull, DBaction_BuildingSelection);
		res = this;
	}
	else
	{
		res = res->AddToSelection(recID, err);
		if (err != VE_OK)
		{
			res->Release();
			res = this;
		}
	}
	//recalcnbfic=true;
	Touch();
	return res;
}


VError BitSel::AddToBittab(Bittab *b, BaseTaskInfo* context)
{
	VError err;
	err = b->Or(pTB);
	//Touch();	L.E. c'est b qui est modifie
	return(err);
}


VError BitSel::DelFromSel(sLONG n)
{
	VError err;
	
	pTB->use();
	err=pTB->Clear(-n);
	pTB->unuse();
	
	Touch();
	return(err);
}


VError BitSel::DelFromSel(Bittab* b)
{
	VError err;

	pTB->use();
	err=pTB->moins(b);
	pTB->unuse();

	Touch();
	return(err);
}


Bittab* BitSel::GenereBittab(BaseTaskInfo* context, VError& err)
{
	err = VE_OK;
	pTB->Retain();
	return(pTB);
}


Bittab* BitSel::StealBittab(BaseTaskInfo* context, VError& err)
{
	Bittab* result;
	if (pTB == &Owned_TB)
	{
		result = pTB->Clone(err);
	}
	else
	{
		result = pTB;
		pTB->FreeFromSel();
		pTB = &Owned_TB;
		pTB->Retain();
		pTB->SetAsSel();
	}

	if (result != nil)
		result->SetOwner(parentfile->GetDB());
	return result;
}



VError BitSel::LibereEspaceDisk(OccupableStack* curstack, VDB4DProgressIndicator* InProgress)
{
	if (SelisCluster)
	{
		return pTB->LibereEspaceDisk(curstack, InProgress);
	}
	
	return VE_OK;
}


VError BitSel::FillArray(sLONG* tab, sLONG maxelem)
{
	return(pTB->FillArray(tab, maxelem));
}


VError BitSel::AddSelToSel(sLONG& curfic, sLONG maxnb, Bittab* filtre, Selection* sel)
{
	VError err = pTB->AddBittabToSel(curfic, maxnb, filtre, sel);
	Touch();
	return err;
}


void BitSel::DupliqueInto(SelectionPtr& newsel)
{
	newsel = CreFromBittab(parentfile, pTB, db);
	newsel->Touch();
}


Boolean BitSel::LockRecords(BaseTaskInfo* context)
{
	Boolean ok =true;

	if (!fKeepRecordLocked)
	{
		if (context != nil)
		{
			if (!context->AddLockingSelection(this))
				ok = false;
		}

		if (ok)
			fKeepRecordLocked = true;
	}

	return true;
}

Boolean BitSel::UnLockRecords(BaseTaskInfo* context, sLONG upto, Boolean RemoveFromContext)
{
	if (fKeepRecordLocked)
	{
		context->RemoveLockingSelection(this, RemoveFromContext);
	}
	return true;
}


VError BitSel::FillWith(const void* data, sLONG sizeElem, sLONG nbElem, Boolean ascent, sLONG maxrecords, sLONG startFrom, bool useIndirection, Bittab* inWafSel, WafSelection* outWafSel)
{
	recalcnbfic=true;
	return pTB->FillFromArray((const char*)data, sizeElem, nbElem, maxrecords, useIndirection);
}


Boolean BitSel::IntersectBittab(Bittab* b, BaseTaskInfo* context, VError& err)
{
	err = VE_OK;
	return pTB->Intersect(b, err);
}



sLONG BitSel::GetRecordPos(sLONG inRecordID, BaseTaskInfo* context)
{
	return pTB->CountHowManyBitAllumeAt(inRecordID);
}


Bittab* BitSel::GenerateSetFromRange(sLONG inRecordIndex1, sLONG inRecordIndex2, VError& err, BaseTaskInfo* context)
{
	Bittab* res = pTB->GenerateSetFromRange(inRecordIndex1, inRecordIndex2, err);
	return res;
}


Bittab* BitSel::GenerateSetFromRecordID(sLONG inRecordID, sLONG inRecordIndex2, VError& err, BaseTaskInfo* context)
{
	Bittab* res = pTB->GenerateSetFromRecordID(inRecordID, inRecordIndex2, err);
	return res;
}


VError BitSel::RemoveSelectedRange(sLONG inRecordIndex1, sLONG inRecordIndex2, BaseTaskInfo* context)
{
	VError err = pTB->RemoveSelectedRange(inRecordIndex1, inRecordIndex2);
	Touch();
	return err;
}


VError BitSel::TransformIntoCluster(DataAddr4D addr)
{
	VError err = pTB->TransformIntoCluster(addr, parentfile->GetDB());
	if (err == VE_OK)
		SetSelAddr(addr);
	return err;
}

#if debugLeaksAll

void BitSel::GetDebugInfo(VString& outText) const
{
	VString s;
	outText = "bitsel : ";
	try
	{
		VString s;
		((Selection*)this)->GetParentFile()->GetTable()->GetName(s);
		outText += " for table : "+s;
	}
	catch (...)
	{
		outText += " for unknown table ";
	}
}

#endif


					/* ================================================= */

SelectionIterator::SelectionIterator(Selection* sel)
{
	fSel = sel;
	fCurRecord = -1;
	fCurIndice = 0;
	fWasJustReset = true;
}


SelectionIterator::SelectionIterator(const SelectionIterator& other)
{
	fSel = other.fSel;
	fCurRecord = other.fCurRecord;
	fCurIndice = other.fCurIndice;
	fWasJustReset = other.fWasJustReset;;
}


SelectionIterator& SelectionIterator::operator =(const SelectionIterator& other)
{
	fSel = other.fSel;
	fCurRecord = other.fCurRecord;
	fCurIndice = other.fCurIndice;
	fWasJustReset = other.fWasJustReset;;
	return *this;
}


void SelectionIterator::Reset(Selection* sel)
{
	fSel = sel;
	fCurRecord = -1;
	fCurIndice = 0;
	fWasJustReset = true;
}


RecIDType SelectionIterator::NextRecord()
{
	if (fWasJustReset)
		return FirstRecord();
	else
	{
		if (fCurIndice != -1)
		{
			fCurRecord = fSel->GetNextRecord(fCurRecord+1,fCurIndice+1);
			if (fCurRecord == -1)
				fCurIndice = -1;
			else
				fCurIndice++;
		}

		return fCurRecord;
	}

}

RecIDType SelectionIterator::PreviousRecord()
{
	if (fWasJustReset)
		return LastRecord();
	else
	{
		if (fCurIndice != -1)
		{
			fCurRecord = fSel->GetPreviousRecord(fCurRecord-1,fCurIndice-1);
			if (fCurRecord == -1)
				fCurIndice = -1;
			else
				fCurIndice--;
		}
		else
		{
			fCurRecord = -1;
		}

		return fCurRecord;
	}
}


RecIDType SelectionIterator::LastRecord()
{
	fWasJustReset = false;
	fCurRecord = fSel->GetLastRecord();
	if (fCurRecord == -1)
		fCurIndice = -1;
	else
		fCurIndice = fSel->GetQTfic()-1;

	return fCurRecord;
}



RecIDType SelectionIterator::FirstRecord()
{
	fWasJustReset = false;
	fCurRecord = fSel->GetNextRecord(0,0);
	if (fCurRecord == -1)
		fCurIndice = -1;
	else
		fCurIndice = 0;

	return fCurRecord;
}


RecIDType SelectionIterator::SetCurrentRecord(sLONG PosInSel)
{
	fWasJustReset = false;
	if (PosInSel >= fSel->GetQTfic())
	{
		fCurRecord = -1;
		fCurIndice = -1;
	}
	else
	{
		fCurRecord = fSel->GetFic(PosInSel);
		fCurIndice = PosInSel;
	}
	return fCurRecord;
}



					/* ================================================= */
					


Selection* CreFromBittabPtr(DataTable *DF, Bittab* b, Base4D* remoteDB)
{
	Selection* sel=new BitSel(DF, b, remoteDB);
	if (sel == nil)
		ThrowBaseError(memfull,noaction);
	return sel;
}


Selection* CreFromBittab(DataTable *DF, Bittab* b, Base4D* remoteDB)
{
	VError err;
	Selection* sel = nil;
	Bittab* b2 = b->Clone(err);

	if (b2 != nil)
	{
		sel=new BitSel(DF, b2, remoteDB);
		if (sel == nil)
			ThrowBaseError(memfull,noaction);
		b2->Release();
	}
	return sel;
}


Selection* CreFromBittab_nobitsel(DataTable *DF, Bittab* b, Base4D* remoteDB)
{
	Selection* sel = nil;
	sLONG nb=b->Compte();
	if (nb>kNbElemInSel)
	{
		sel=new LongSel(DF, remoteDB);
		((LongSel*)sel)->PutInCache();
	}
	else
	{
		sel=new PetiteSel(DF, remoteDB);
	}
	if (sel == nil)
		ThrowBaseError(memfull,noaction);
	else
	{
		VError err = sel->UpdateFromBittab(b);
		if (err != VE_OK)
		{
			sel->Dispose();
			sel=nil;
		}
	}

	return sel;
}


	/* ---------------------------------------------------- */



VError SelPosIterator::InitWithBittab(Bittab* b, sLONG fromindice, sLONG toindice)
{
	fSet = b;
	fSet->Retain();
	//sLONG i = fSet->FindNextBit(0);
	sLONG i = fSet->PlaceDuNiemeBitAllume(fromindice);
	fCurRecNum = i;
	fCurSelPos = 0;
	fCurPos = 0;
	fMax = toindice - fromindice + 1;
	return VE_OK;
}


VError SelPosIterator::InitWithSel(Selection* sel, sLONG fromindice, sLONG toindice)
{
	VError err = VE_OK;

	if (sel->GetTypSel() == sel_bitsel)
	{
		return InitWithBittab(((BitSel*)sel)->GetBittab(), fromindice, toindice);
	}
	else
	{
		//sLONG qt = sel->GetQTfic();
		//fMax = qt;
		sLONG qt = toindice - fromindice + 1;
		fMax = qt;
		if (qt > 0)
		{
			fPosInfos = (SelPosInfo*)GetTempMem((VSize)qt * (VSize)sizeof(SelPosInfo), false, 'slps');
			if (fPosInfos != nil)
			{
				err = sel->FillPosInfoArray(fPosInfos, fromindice, toindice);
				if (err == VE_OK)
				{
					sort(fPosInfos, fPosInfos+qt);
				}
			}
			else
				err = ThrowBaseError(memfull);

			if (err == VE_OK)
			{
				fCurRecNum = fPosInfos->fRecNum;
				fCurSelPos = fPosInfos->fSelPos;
			}
		}
		else
		{
			fCurRecNum = -1;
			fCurSelPos = 0;
		}
	}


	return err;
}


void SelPosIterator::NextPos()
{
	fCurPos++;
	if (fSet != nil)
	{
		if (fCurPos < fMax)
		{
			sLONG i = fSet->FindNextBit(fCurRecNum+1);
			fCurRecNum = i;
			fCurSelPos = fCurPos;
		}
		else
		{
			fCurRecNum = -1;
			fCurSelPos = -1;			
		}
	}
	else
	{
		if (fCurPos < fMax)
		{
			SelPosInfo* curpos = fPosInfos + fCurPos;
			fCurRecNum = curpos->fRecNum;
			fCurSelPos = curpos->fSelPos;
		}
		else
		{
			fCurRecNum = -1;
			fCurSelPos = -1;
		}
	}
}




