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



LocalEntityCollection::LocalEntityCollection(LocalEntityModel* inOwner, Selection* inSel): EntityCollection((EntityModel*)inOwner)
{
	if (inSel == nil)
	{
		fSel = new PetiteSel(inOwner->GetMainTable()->GetDF());
	}
	else
		fSel = RetainRefCountable(inSel);
}


VError LocalEntityCollection::AddEntity(EntityRecord* entity, bool atTheEnd)
{
	VError err = VE_OK;
	LocalEntityRecord* localEntity = dynamic_cast<LocalEntityRecord*>(entity);
	if ((localEntity == nil) || (localEntity->GetModel() != fOwner))
		err = ThrowBaseError(VE_DB4D_ENTITY_IS_NOT_OF_THE_RIGHT_MODEL, fOwner->GetName());
	else
	{
		RecIDType num = localEntity->GetNum();
		if (num >= 0)
		{
			if (!atTheEnd && fSel->GetTypSel() == sel_bitsel)
				fSel->AddToSel(num);
			else
			{
				Selection* newsel = fSel->AddToSelection(num, err);
				if (newsel != fSel)
				{
					fSel->Release();
					fSel = newsel;
				}
			}
		}
	}
	return err;
}


VError LocalEntityCollection::AddCollection(EntityCollection* other, BaseTaskInfo* context, bool atTheEnd)
{
	VError err = VE_OK;
	LocalEntityCollection* othercol = dynamic_cast<LocalEntityCollection*>(other);
	if (other != nil)
	{
		Selection* othersel = othercol->fSel;
		if (fSel->GetTypSel() == sel_bitsel && othersel->GetTypSel() == sel_bitsel && !atTheEnd)
		{
			((BitSel*)fSel)->GetBittab()->Or(((BitSel*)othersel)->GetBittab());
			((BitSel*)fSel)->Touch();
		}
		else
		{
			sLONG nbelem = othersel->GetQTfic();
			for (sLONG i = 0; i < nbelem && err == VE_OK; i++)
			{
				RecIDType x = othersel->GetFic(i);
				Selection* newsel = fSel->AddToSelection(x, err);
				if (newsel != fSel)
				{
					fSel->Release();
					fSel = newsel;
				}
			}
		}

	}
	else
		err = ThrowBaseError(VE_UNKNOWN_ERROR);

	return err;
}


EntityRecord* LocalEntityCollection::LoadEntity(RecIDType posInCol, BaseTaskInfo* context, VError& outError, DB4D_Way_of_Locking HowToLock)
{
	LocalEntityModel* localModel = (LocalEntityModel*)fOwner;
	EntityRecord* result = nil;
	outError = VE_OK;
	sLONG recnum = fSel->GetFic(posInCol);
	if (recnum != -1)
	{
		result = localModel->LoadEntity(recnum, outError,  HowToLock, context);
	}
	return result;
}


RecIDType LocalEntityCollection::GetLength(BaseTaskInfo* context)
{
	return fSel->GetQTfic();
}


EntityCollection* LocalEntityCollection::SortSel(VError& err, EntityAttributeSortedSelection* sortingAtt, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, WafSelection* inWafSel, WafSelection* outWafSel)
{
	EntityCollection* result = nil;
	if (sortingAtt == nil || sortingAtt->empty() || GetLength(context) < 2)
	{
		result = this;
		result->Retain();
	}
	else
	{
		Selection* newsel = fSel->SortSel(err, (LocalEntityModel*)fOwner, sortingAtt, context, InProgress, inWafSel, outWafSel);
		if (newsel != nil)
		{
			result = new LocalEntityCollection((LocalEntityModel*)fOwner, newsel);
			newsel->Release();
		}
	}

	return result;
}


VError LocalEntityCollection::DropEntities(BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, EntityCollection* *outLocked)
{
	VError err = VE_OK;
	Table* target = ((LocalEntityModel*)fOwner)->GetMainTable();
	Base4D* db = target->GetOwner();
	DataTable* df = target->GetDF();

	if (okperm(context, fOwner, DB4D_EM_Delete_Perm))
	{
		bool DeleteDejaFait = false;
		if (!fOwner->HasDeleteEvent())
		{
			if (!target->HasSyncInfo() && !(db->ReferentialIntegrityEnabled() && target->AtLeastOneReferentialIntegrity()) )
			{
				df->LockRead();
				bool mustunlock = true;
				if (fSel->IsAllRecords(context) && GetCurrentTransaction(context) == nil)
				{
					if (!db->IsThereATrigger(DB4D_DeleteRecord_Trigger, target))
					{
						DeleteDejaFait = true;
						err = target->Truncate(context, InProgress, true, mustunlock);
						// attention le Unlock peut avoir ete fait dans le Truncate ( 2 fois en tout)
						if (err == -2)
						{
							DeleteDejaFait = false;
							err = VE_OK;
						}
					}
				}
				if (mustunlock)
					df->Unlock();

				if (!DeleteDejaFait)
				{
					if (!target->CanBeLogged() && !target->AtLeastOneBlob() && GetCurrentTransaction(context) == nil && !db->IsThereATrigger(DB4D_DeleteRecord_Trigger, target))
					{
						if (target->NonIndexed() || (df->GetNbRecords(context, false)/3) < fSel->GetQTfic())
						{
							DeleteDejaFait = true;
							sLONG i;
							Boolean first = true;
							SelectionIterator itersel(fSel);

							sLONG currec = itersel.LastRecord();
							sLONG count = 0;
							StErrorContextInstaller errorContext( true);	// true means all errors are kept

							if (InProgress != nil)
							{
								InProgress->BeginSession(fSel->GetQTfic(), GetString(1005,23),true);
							}

#if trackModif
							trackDebugMsg("before quick delete selection of records\n");
#endif
							target->DelayIndexes(false);

#if trackModif
							trackDebugMsg("before quick delete selection of records, after index invalidation\n");
#endif

							BitSel* bsel = nil;

							while (currec != -1)
							{
								VTask::Yield();
								if (InProgress != nil)
								{
									if (!InProgress->Progress(count))
									{
										err = fSel->ThrowError(VE_DB4D_ACTIONCANCELEDBYUSER, DBaction_DeletingRecord);
										break;
									}
								}

								count++;
								sLONG n = currec;
								VError err2;
								err2 = df->QuickDelRecord(n, context);
								if (err2 != VE_OK)
								{
									err = VE_DB4D_CANNOT_DELETE_ALL_RECORDS_IN_SEL;
									if (outLocked != nil)
									{
										if (*outLocked == nil)
										{
											bsel = new BitSel(df);
											*outLocked = new LocalEntityCollection((LocalEntityModel*)fOwner, bsel);
											bsel->Release();
										}
										bsel->AddToSel(n);
									}
								}

								// any error that has been thrown in the loop is pushed in the previous error context.
								// then the current error context is flushed and vGetLastError() returns VE_OK.
								errorContext.MergeAndFlush();

								currec = itersel.PreviousRecord();
							}

							QuickReleaseRefCountable(bsel);

							if (InProgress != nil)
								InProgress->EndSession();
							
#if trackModif
							trackDebugMsg("after quick delete selection of records, before index rebuilding\n");
#endif
							target->AwakeIndexes(nil, nil);

#if trackModif
							trackDebugMsg("after quick delete selection of records\n");
#endif
						}
					}
					target->GetOwner()->ReleaseLogMutex();
				}
			}
		}

		if (!DeleteDejaFait)
		{
			bool withModel = false;
			if (fOwner->HasDeleteEvent())
				withModel = true;

			sLONG i;
			Boolean first = true;
			SelectionIterator itersel(fSel);

			sLONG currec = itersel.LastRecord();
			sLONG count = 0;

			StErrorContextInstaller errorContext( true);	// true means all errors are kept

			if (InProgress != nil)
			{
				InProgress->BeginSession(GetLength(context), GetString(1005,23),true);
			}
			
			BitSel* bsel = nil;

			while (currec != -1)
			{
				VTask::Yield();
				if (InProgress != nil)
				{
					if (!InProgress->Progress(count))
					{
						err = fSel->ThrowError(VE_DB4D_ACTIONCANCELEDBYUSER, DBaction_DeletingRecord);
						break;
					}
				}

				count++;
				sLONG n = currec;
				VError err2 = VE_OK;

				if (withModel)
				{					
					EntityRecord* erec = ((LocalEntityModel*)fOwner)->LoadEntity(n, err2, DB4D_Do_Not_Lock, context);
					if (erec != nil)
					{
						err2 = erec->Drop();
						erec->Release();
					}
				}
				else
				{
					FicheInMem* rec = df->LoadRecord(n, err2, DB4D_Keep_Lock_With_Record, context, true);
					if (rec != nil)
					{
						err2 = df->DelRecord(rec, context);
						rec->Release();
					}
				}

				if (err2 != VE_OK)
				{
					err = VE_DB4D_CANNOT_DELETE_ALL_RECORDS_IN_SEL;
					if (outLocked != nil)
					{
						if (*outLocked == nil)
						{
							bsel = new BitSel(df);
							*outLocked = new LocalEntityCollection((LocalEntityModel*)fOwner, bsel);
							bsel->Release();
						}
						bsel->AddToSel(n);
					}
				}

				// any error that has been thrown in the loop is pushed in the previous error context.
				// then the current error context is flushed and vGetLastError() returns VE_OK.
				errorContext.MergeAndFlush();

				currec = itersel.PreviousRecord();

			}

			QuickReleaseRefCountable(bsel);

			if (InProgress != nil)
				InProgress->EndSession();
		}
	}
	else
	{
		context->SetPermError();
		err = fOwner->ThrowError(VE_DB4D_NO_PERM_TO_DELETE);
	}

	return err;
}



RecIDType LocalEntityCollection::NextNotNull(BaseTaskInfo* context, RecIDType startFrom)
{
	RecIDType qt = fSel->GetQTfic();
	bool stop = false;
	do 
	{
		if (startFrom != -1)
		{
			RecIDType recnum = fSel->GetFic(startFrom);
			if (recnum >= 0)
			{
				sLONG len = 0;
				VError err = VE_OK;
				DataAddr4D ou = ((LocalEntityModel*)fOwner)->GetMainTable()->GetDF()->GetRecordPos(recnum, len, err);
				if (ou > 0)
				{
					stop = true;
				}
			}
		}
		else
			stop = true;

		if (!stop)
		{
			++startFrom;
			if (startFrom >= qt)
			{
				startFrom = -1;
				stop = true;
			}
		}

	} while (!stop);

	return startFrom;

}



VError LocalEntityCollection::ComputeOnOneAttribute(const EntityAttribute* att, DB4D_ColumnFormulae action, VJSValue& outVal, BaseTaskInfo* context, JS4D::ContextRef jscontext)
{
	bool okresult = false;
	LocalEntityModel* em = (LocalEntityModel*)fOwner;
	VJSContext vjsContext(jscontext);
	VJSValue resultVal(vjsContext);
	resultVal.SetNull();
	VError err = VE_OK;

	if (att->GetModel() != fOwner)
	{
		VString s = em->GetName()+"."+att->GetName();
		err = ThrowBaseError(VE_DB4D_ENTITY_ATTRIBUTE_IS_FROM_ANOTHER_DATACLASS, s);
	}
	else
	{
		if (action == DB4D_Count || action == DB4D_Count_distinct || action == DB4D_Min || action == DB4D_Max)
		{
			if (!att->IsStatable())
			{
				err = att->ThrowError(VE_DB4D_WRONG_ATTRIBUTE_KIND);
			}
		}
		else
		{
			if (!att->IsSummable())
			{
				err = att->ThrowError(VE_DB4D_WRONG_ATTRIBUTE_KIND);
			}
		}
	}

	if (err == VE_OK)
	{
		if (att->GetKind() == eattr_storage)
		{
			Field* cri = ((db4dEntityAttribute*)att)->RetainDirectField();
			if (cri != nil)
			{
				ColumnFormulas formules(em->GetMainTable());
				formules.AddAction(action, cri);
				VError err = formules.Execute(fSel, context, nil, nil, (action >= DB4D_Count_distinct) && (action <= DB4D_Sum_distinct));
				if (err == VE_OK)
				{
					VValueSingle* result = formules.GetResult(0);
					if (result != nil)
					{
						if (result->IsNull() && (action == DB4D_Sum || action == DB4D_Count))
							resultVal.SetNumber(0);
						else
							resultVal.SetVValue(*result);
					}
				}
				cri->Release();
			}
		}
		else
		{
			err = EntityCollection::ComputeOnOneAttribute(att, action, resultVal, context, jscontext);
		}
	}

	outVal = resultVal;
	return err;
}


EntityCollection* LocalEntityCollection::_Mix(EntityCollection* other, VError& err, BaseTaskInfo* context, DB4DConjunction action)
{
	err = VE_OK;
	EntityCollection* result = nil;
	LocalEntityCollection* locOther = dynamic_cast<LocalEntityCollection*>(other);
	if (locOther != nil)
	{
		Bittab* b1 = fSel->GenereBittab(context, err);
		Bittab* b2 = locOther->GetSel()->GenereBittab(context, err);

		if (b1 != nil && b2 != nil)
		{
			if (action == DB4D_Intersect)
			{
				if (b1->Intersect(b2, err))
					result = (EntityCollection*)1;
				else
					result = nil;
			}
			else
			{
				Bittab* b = b1->Clone(err);
				if (b != nil)
				{
					switch (action)
					{
						case DB4D_And:
							err = b->And(b2);
							break;
						case DB4D_OR:
							err = b->Or(b2);
							break;
						case DB4D_Except:
							err = b->moins(b2);
							break;
						default:
							err = ThrowBaseError(VE_UNKNOWN_ERROR);
							break;
					}
					if (err == VE_OK)
					{
						BitSel* bsel = new BitSel(fSel->GetParentFile(), b);
						result = new LocalEntityCollection((LocalEntityModel*)fOwner, bsel);
						QuickReleaseRefCountable(bsel);

					}
					QuickReleaseRefCountable(b);
				}
			}
		}
		else
			err = ThrowBaseError(VE_DB4D_COLLECTION_ON_INCOMPATIBLE_DATACLASSES);
		QuickReleaseRefCountable(b1);
		QuickReleaseRefCountable(b2);
	}
	else
		err = ThrowBaseError(VE_DB4D_COLLECTION_ON_INCOMPATIBLE_DATACLASSES);

	return result;
}


EntityCollection* LocalEntityCollection::And(EntityCollection* other, VError& err, BaseTaskInfo* context)
{
	return _Mix(other, err, context, DB4D_And);
}


EntityCollection* LocalEntityCollection::Or(EntityCollection* other, VError& err, BaseTaskInfo* context)
{
	return _Mix(other, err, context, DB4D_OR);
}


EntityCollection* LocalEntityCollection::Minus(EntityCollection* other, VError& err, BaseTaskInfo* context)
{
	return _Mix(other, err, context, DB4D_Except);
}


bool LocalEntityCollection::Intersect(EntityCollection* other, VError& err, BaseTaskInfo* context)
{
	EntityCollection* fakecol = _Mix(other, err, context, DB4D_Intersect); // 0 or 1
	if (fakecol == nil)
		return false;
	else
		return true;
}


EntityCollectionIterator* LocalEntityCollection::NewIterator(BaseTaskInfo* context)
{
	return new LocalEntityCollectionIterator(this, context);
}


VError LocalEntityCollection::GetDistinctValues(EntityAttribute* att, DB4DCollectionManager& outCollection, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, const VCompareOptions& inOptions)
{
	VError err;
	err = fSel->GetDistinctValues((db4dEntityAttribute*)att, outCollection, context, InProgress, inOptions);
	/*
	{
		bool dejafait = false;
		if (att->GetKind() == eattr_storage && !att->HasEvent(dbev_load))
		{
			Field* cri = ((LocalEntityModel*)fOwner)->RetainDirectField(att);
			if (cri != nil)
			{
				dejafait = true;
				err = fSel->GetDistinctValues(cri, outCollection, context, InProgress, inOptions);
				cri->Release();
			}
		}

		if (!dejafait)
		{
			fSel->TryToFastGetDistinctValues(err, (db4dEntityAttribute*)att, outCollection, context, InProgress, inOptions);
		}
	}
	*/

	return err;

}


VError LocalEntityCollection::RemoveSelectedRange(sLONG inRecordIndex1, sLONG inRecordIndex2, BaseTaskInfo* context)
{
	return fSel->RemoveSelectedRange(inRecordIndex1, inRecordIndex2, context);
}


EntityCollection* LocalEntityCollection::NewFromWafSelection(WafSelection* wafsel, BaseTaskInfo* context)
{
	EntityCollection* result = nil;
	LocalEntityModel* locmodel = (LocalEntityModel*)fOwner;
	if (fSel->GetTypSel() == sel_bitsel)
	{
		Bittab filter;
		filter.aggrandit(locmodel->GetMainTable()->GetDF()->GetMaxRecords(context));

		for (vector<sLONG>::iterator cur = wafsel->rows.begin(), end = wafsel->rows.end(); cur != end; cur++)
		{
			sLONG n = fSel->GetFic(*cur);
			filter.Set(n);					
		}

		for (WafSelectionRangeVector::iterator cur = wafsel->ranges.begin(), end = wafsel->ranges.end(); cur != end; cur++)
		{
			sLONG debut = cur->start;
			sLONG fin = cur->end;
			for (sLONG i = debut; i <= fin; i++)
			{
				sLONG n = fSel->GetFic(i);
				filter.Set(n);
			}

		}

		for (vector<sLONG>::iterator cur = wafsel->butRows.begin(), end = wafsel->butRows.end(); cur != end; cur++)
		{
			sLONG n = fSel->GetFic(*cur);
			filter.Clear(n);					
		}

		Bittab* bresult = new Bittab();
		Bittab* bsel = ((BitSel*)fSel)->GetBittab();
		bresult->Or(bsel);
		bresult->And(&filter);
		Selection* sel = new BitSel(locmodel->GetMainTable()->GetDF(), bresult);
		bresult->Release();
		result = new LocalEntityCollection(locmodel, sel);
		sel->Release();

	}
	else
	{
		Bittab filter;
		Selection* newsel;
		sLONG nbelemInWafSel = wafsel->count();
		if (nbelemInWafSel > kNbElemInSel)
		{
			newsel=new LongSel(locmodel->GetMainTable()->GetDF(), nil);
			((LongSel*)newsel)->PutInCache();
		}
		else
		{
			newsel=new PetiteSel(locmodel->GetMainTable()->GetDF(), nil);
		}

		for (vector<sLONG>::iterator cur = wafsel->butRows.begin(), end = wafsel->butRows.end(); cur != end; cur++)
		{
			sLONG n = fSel->GetFic(*cur);
			filter.Set(n);					
		}


		for (vector<sLONG>::iterator cur = wafsel->rows.begin(), end = wafsel->rows.end(); cur != end; cur++)
		{
			sLONG n = fSel->GetFic(*cur);
			if (!filter.isOn(n))
			{
				newsel->AddToSel(n);
			}
		}

		for (WafSelectionRangeVector::iterator cur = wafsel->ranges.begin(), end = wafsel->ranges.end(); cur != end; cur++)
		{
			sLONG debut = cur->start;
			sLONG fin = cur->end;
			for (sLONG i = debut; i <= fin; i++)
			{
				sLONG n = fSel->GetFic(i);
				if (!filter.isOn(n))
					newsel->AddToSel(n);
			}

		}

		result = new LocalEntityCollection(locmodel, newsel);
		newsel->Release();
	}
	return result;
}


RecIDType LocalEntityCollection::FindKey(VectorOfVString& vals, BaseTaskInfo* context, VError& err)
{
	RecIDType result = -1;
	LocalEntityModel* locmodel = (LocalEntityModel*)fOwner;

	RecIDType n = locmodel->getEntityNumWithPrimKey(vals,context, err);
	if (n != -1)
		result = fSel->GetRecordPos(n, context);
	return result;
}

/*
EntityCollection* LocalEntityCollection::ProjectCollection(EntityAttribute* att, VError& err, BaseTaskInfo* context)
{
	EntityCollection* result = nil;

	EntityRelation* relpath = att->GetRelPath();
	EntityModel* relmodel = att->GetSubEntityModel();

	if (testAssert(relpath != nil) && relmodel != nil)
	{
		const vector<EntityRelation*>& path = relpath->GetPath();
		SearchTab query(relmodel);
		map<const EntityModel*, sLONG> dejainstance;

		bool first = true;
		for (vector<EntityRelation*>::const_iterator cur = path.begin(), end = path.end(); cur != end; cur++)
		{
			EntityModel* sourcemodel = (*cur)->GetSourceAtt()->GetOwner();
			EntityModel* destmodel = (*cur)->GetDestAtt()->GetOwner();
			sLONG instancedest = dejainstance[destmodel];
			if (sourcemodel == destmodel)
				dejainstance[destmodel]++;
			sLONG instancesource = dejainstance[sourcemodel];
			query.AddSearchLineJoinEm((*cur)->GetSourceAtt(), DB4D_Equal, (*cur)->GetDestAtt(), false, instancesource, instancedest);
			query.AddSearchLineBoolOper(DB4D_And);
		}

		query.AddSearchLineEmSel(this, dejainstance[GetModel()]);

		result = relmodel->executeQuery(&query, context, nil, nil, DB4D_Do_Not_Lock, 0, nil, &err);
	}
	
	return result;
}

*/


VError LocalEntityCollection::InstallBittab(Bittab* b)
{
	LocalEntityModel* locmodel = (LocalEntityModel*)fOwner;
	BitSel* bsel = new BitSel(locmodel->GetMainTable()->GetDF(), b);
	QuickReleaseRefCountable(fSel);
	fSel = bsel;
	return VE_OK;
}


bool LocalEntityCollection::MatchAllocationNumber(sLONG allocationNumber)
{
	return fSel->MatchAllocationNumber(allocationNumber);
}


size_t LocalEntityCollection::CalcLenOnDisk(void)
{
	return (size_t)fSel->CalcLenOnDisk();
}





				// -------------------------------------


LocalEntityCollectionIterator::LocalEntityCollectionIterator(LocalEntityCollection* collection, BaseTaskInfo* context):EntityCollectionIterator(collection, context),fSelIter(collection->GetSel())
{
	
}


EntityRecord* LocalEntityCollectionIterator::First(VError& err)
{
	EntityRecord* result = nil;
	err = VE_OK;
	LocalEntityModel* locmodel = (LocalEntityModel*)(fCollection->GetModel());
	RecIDType recnum = fSelIter.FirstRecord();
	if (recnum != -1)
	{
		result = locmodel->LoadEntity(recnum, err, DB4D_Do_Not_Lock, fContext);
	}
	return result;
}


EntityRecord* LocalEntityCollectionIterator::Next(VError& err)
{
	EntityRecord* result = nil;
	err = VE_OK;
	LocalEntityModel* locmodel = (LocalEntityModel*)(fCollection->GetModel());
	RecIDType recnum = fSelIter.NextRecord();
	if (recnum != -1)
	{
		result = locmodel->LoadEntity(recnum, err, DB4D_Do_Not_Lock, fContext);
	}
	return result;
}


RecIDType LocalEntityCollectionIterator::GetCurPos() const
{
	return fSelIter.GetCurPos();
}


EntityRecord* LocalEntityCollectionIterator::SetCurPos(RecIDType pos, VError& err)
{
	EntityRecord* result = nil;
	err = VE_OK;
	LocalEntityModel* locmodel = (LocalEntityModel*)(fCollection->GetModel());
	RecIDType recnum = fSelIter.SetCurrentRecord(pos);
	if (recnum != -1)
	{
		result = locmodel->LoadEntity(recnum, err, DB4D_Do_Not_Lock, fContext);
	}
	return result;
}






// -----------------------------------------------------------------------------------------------------------------------


LocalEntityModel::LocalEntityModel(LocalEntityModelCatalog* inOwner, Table* inMainTable): EntityModel((EntityModelCatalog*)inOwner)
{
	fMainTable = inMainTable;
	fDB = inOwner->GetDB();

}


EntityRecord* LocalEntityModel::LoadEntity(sLONG n, VError& err, DB4D_Way_of_Locking HowToLock, BaseTaskInfo* context)
{
	if (okperm(context, fPerms[DB4D_EM_Read_Perm]) /*|| okperm(context, fPerms[DB4D_EM_Update_Perm]) || okperm(context, fPerms[DB4D_EM_Delete_Perm])*/ )
	{
		FicheInMem* rec = fMainTable->GetDF()->LoadRecord(n, err, HowToLock, context);
		if (rec == nil)
			return nil;
		else
		{
			EntityRecord* erec = new LocalEntityRecord(this, rec, context);
			rec->Release();
			CallDBEvent(dbev_load, erec, context);
			return erec;
		}
	}
	else
	{
		err = ThrowError(VE_DB4D_NO_PERM_TO_READ);
		context->SetPermError();
		return nil;
	}
}




sLONG LocalEntityModel::countEntities(BaseTaskInfo* inContext)
{
	sLONG result = 0;
	if (okperm(inContext, fPerms[DB4D_EM_Read_Perm]) /*|| okperm(inContext, fPerms[DB4D_EM_Update_Perm]) || okperm(inContext, fPerms[DB4D_EM_Delete_Perm])*/ )
	{
		if (WithRestriction())
		{
			VError err = VE_OK;
			EntityCollection* sel = BuildRestrictingSelection(inContext, err);
			if (sel != nil)
			{
				result = sel->GetLength(inContext);
				sel->Release();
			}
		}
		else if (WithRestrictingQuery())
		{
			Base4D *db;
			DataTable *DF;
			DF=fMainTable->GetDF();
			db=DF->GetDB();
			OptimizedQuery rech;
			// db->LockIndexes(); // will be done at another level when necessary
			SearchTab locsearch(this);
			locsearch.From(*fRestrictingQuery);
			VError err = rech.AnalyseSearch(&locsearch, inContext);
			// db->UnLockIndexes();
			if (err == VE_OK)
			{
				Selection* sel = rech.Perform((Bittab*)nil, nil, inContext, err, DB4D_Do_Not_Lock, 0, nil);
				if (sel != nil)
				{
					result = sel->GetQTfic();
					sel->Release();
				}
			}
		}
		else
			result = fMainTable->GetDF()->GetNbRecords(inContext);
	}
	else
		result = 0;
	return result;
}




EntityCollection* LocalEntityModel::SelectAllEntities(BaseTaskInfo* context, VErrorDB4D* outErr, DB4D_Way_of_Locking HowToLock, EntityCollection* outLockSet, bool allowRestrict)
{
	VError err = VE_OK;
	Selection* sel = nil;
	if (okperm(context, fPerms[DB4D_EM_Read_Perm]) /*|| okperm(context, fPerms[DB4D_EM_Update_Perm]) || okperm(context, fPerms[DB4D_EM_Delete_Perm])*/)
	{
		if (WithRestrictingQuery())
		{
			Base4D *db;
			DataTable *DF;
			DF=fMainTable->GetDF();
			db=DF->GetDB();
			OptimizedQuery rech;
			// db->LockIndexes(); // will be done at another level when necessary
			SearchTab locsearch(this);
			locsearch.From(*fRestrictingQuery);
			err = rech.AnalyseSearch(&locsearch, context);
			// db->UnLockIndexes();
			if (err == VE_OK)
			{
				sel = rech.Perform((Bittab*)nil, nil, context, err, HowToLock, 0, nil);
				if (err == VE_OK)
				{
					if (WithRestriction() && allowRestrict)
					{
						Selection* sel2 = nil;
						EntityCollection* collec = BuildRestrictingSelection(context, err);
						if (collec != nil)
						{
							LocalEntityCollection* localCollec = dynamic_cast<LocalEntityCollection*>(collec);
							if (localCollec != nil)
							{
								sel2 = RetainRefCountable(localCollec->GetSel());
							}
						}

						if (err == VE_OK && sel2 != nil && sel != nil)
						{
							Bittab* b1 = sel->GenereBittab(context, err);
							Bittab* b2 = sel2->GenereBittab(context, err);

							if (b1 != nil && b2 != nil)
							{
								Bittab* b3 = b1->Clone(err);
								if (b3 != nil)
								{
									err = b3->And(b2);
									if (err == VE_OK)
									{
										Selection* sel3 = new BitSel(sel->GetParentFile(), b3);
										QuickReleaseRefCountable(sel);
										sel = sel3;
									}
								}
								QuickReleaseRefCountable(b3);
							}

							QuickReleaseRefCountable(b1);
							QuickReleaseRefCountable(b2);

						}
						if (err != VE_OK)
							ReleaseRefCountable(&sel);
						QuickReleaseRefCountable(sel2);
						QuickReleaseRefCountable(collec);
					}
				}
			}
		}
		else if (WithRestriction() && allowRestrict)
		{
			EntityCollection* collec = BuildRestrictingSelection(context, err);
			if (collec != nil)
			{
				LocalEntityCollection* localCollec = dynamic_cast<LocalEntityCollection*>(collec);
				if (localCollec != nil)
				{
					sel = RetainRefCountable(localCollec->GetSel());
				}
				QuickReleaseRefCountable(collec);
			}
		}
		else
			sel = fMainTable->GetDF()->AllRecords(context, err);
	}
	else
	{
		err = ThrowError(VE_DB4D_NO_PERM_TO_READ);
		context->SetPermError();
	}
	if (outErr != nil)
		*outErr = err;

	EntityCollection* result = nil;
	if (sel != nil)
	{
		result = new LocalEntityCollection(this, sel);
		sel->Release();
	}
	return result;
}



EntityCollection* LocalEntityModel::executeQuery( SearchTab* querysearch, BaseTaskInfo* context, EntityCollection* Filter, 
									 VDB4DProgressIndicator* InProgress, DB4D_Way_of_Locking HowToLock, 
									 sLONG limit, EntityCollection* outLockSet, VErrorDB4D *outErr, EntityAttributeSortedSelection* sortingAtt)
{
	VError err = VE_OK;
	Selection *sel = nil;
	if (okperm(context, fPerms[DB4D_EM_Read_Perm]) /*|| okperm(context, fPerms[DB4D_EM_Update_Perm]) || okperm(context, fPerms[DB4D_EM_Delete_Perm])*/)
	{
		Base4D *db;
		DataTable *DF;
		OptimizedQuery rech;

		{
			if (testAssert( querysearch != nil))
			{

				DF=fMainTable->GetDF();
				db=DF->GetDB();

				SearchTab* xsearch;
				SearchTab locsearch(fMainTable);
				SearchTab restsearch(fMainTable);
				bool alreadyloc = false;

				if (AddRestrictionToQuery(restsearch, context, err))
				{
					if (!alreadyloc)
					{
						locsearch.From(*querysearch);
						xsearch = &locsearch;
					}
					locsearch.Add(restsearch);
					alreadyloc = true;
				}
				if (WithRestrictingQuery())
				{
					if (!alreadyloc)
					{
						locsearch.From(*querysearch);
						xsearch = &locsearch;
					}
					locsearch.Add(*fRestrictingQuery);
					alreadyloc = true;
				}
				if (!alreadyloc)
					xsearch = querysearch;

				// db->LockIndexes(); // will be done at another level when necessary
				err = rech.AnalyseSearch(xsearch, context);
				// db->UnLockIndexes();
				if (err == VE_OK)
				{
					Selection* filter = nil;
					if (Filter != nil)
					{
						LocalEntityCollection* xfilter = dynamic_cast<LocalEntityCollection*>(Filter);
						if (xfilter != nil)
							filter = xfilter->GetSel();
					}
					Bittab* locked = nil;
					if (outLockSet != nil)
						locked = new Bittab();
					sel = rech.Perform(filter, InProgress, context, err, HowToLock, 0, locked);
					if (locked != nil)
					{
						LocalEntityCollection* lockedCol = dynamic_cast<LocalEntityCollection*>(outLockSet);
						if (lockedCol != nil)
						{
							lockedCol->InstallBittab(locked);
						}
						locked->Release();
					}
					if (sel != nil && err == VE_OK && sortingAtt != nil)
					{
						Selection* sel2 = sel->SortSel(err, this, sortingAtt, context, InProgress);
						QuickReleaseRefCountable(sel);
						sel = sel2;
					}
				}


			}
		}
	}
	else
	{
		context->SetPermError();
		err = ThrowError(VE_DB4D_NO_PERM_TO_READ);
	}

	EntityCollection* result = nil;
	if (sel != nil)
	{
		result = new LocalEntityCollection(this, sel);
		sel->Release();
	}

	if (outErr != nil)
		*outErr = err;
	return result;

}


EntityCollection* LocalEntityModel::executeQuery( const VString& queryString, VJSParms_callStaticFunction& ioParms, BaseTaskInfo* context, EntityCollection* filter, 
									   VDB4DProgressIndicator* InProgress, DB4D_Way_of_Locking HowToLock, 
									   sLONG limit, EntityCollection* outLockSet, VError *outErr)
{
	EntityCollection* sel = nil;
	SearchTab query(this);
	VString orderby;
	QueryParamElementVector qparams;
	VError err = getQParams(ioParms, 2, qparams, &query);
	err = query.BuildFromString(queryString, orderby, context, this, false, &qparams);
	if (err == VE_OK)
	{
		err = FillQueryWithParams(&query, ioParms, 2);
		sel = executeQuery(&query, context, filter, InProgress, HowToLock, 0, nil, &err);
		if (sel != nil && !orderby.IsEmpty())
		{
			EntityCollection* newsel = sel->SortCollection(orderby, context, err, InProgress);
			CopyRefCountable(&sel, newsel);
			QuickReleaseRefCountable(newsel);
		}
	}


	if (outErr != nil)
		*outErr = err;

	return sel;
}


EntityCollection* LocalEntityModel::executeQuery( const VString& queryString, VJSONArray* params, BaseTaskInfo* context, EntityCollection* filter, 
												 VDB4DProgressIndicator* InProgress, DB4D_Way_of_Locking HowToLock, 
												 sLONG limit, EntityCollection* outLockSet, VError *outErr)
{
	xbox_assert(false); // shold not be called;
	VError err = ThrowBaseError(VE_UNKNOWN_ERROR);
	if (outErr != nil)
		*outErr = err;
	return nil;
}


EntityCollection* LocalEntityModel::executeQuery( VJSObject queryObj, BaseTaskInfo* context, EntityCollection* filter, 
									   VDB4DProgressIndicator* InProgress, DB4D_Way_of_Locking HowToLock, 
									   sLONG limit, EntityCollection* outLockSet, VError *outErr)
{
	VError err = VE_OK;
	bool first = true;
	EntityCollection* sel = nil;

	vector<VString> props;
	queryObj.GetPropertyNames(props);

	SearchTab query(this);

	VCompareOptions options;
	options.SetDiacritical(false);

	for (vector<VString>::const_iterator cur = props.begin(), end = props.end(); cur != end && err == VE_OK; cur++)
	{
		VJSValue jsval(queryObj.GetContext());
		jsval = queryObj.GetProperty(*cur);
		AttributePath path(this, *cur);
		bool simpleDate = false;
		if (path.IsValid())
			simpleDate = path.LastPart()->fAtt->isSimpleDate();
		VValueSingle* cv = jsval.CreateVValue(simpleDate);
		if (cv != nil)
		{
			if (!first)
				query.AddSearchLineBoolOper(DB4D_And);
			ObjectPath objpath;
			if (path.Count() == 1)
			{
				query.AddSearchLineEmSimple(path.LastPart()->fAtt, objpath, DB4D_Like, cv, options);
			}
			else
			{
				query.AddSearchLineEm(*cur, objpath, DB4D_Like, cv, false);
			}
			delete cv;
			first = false;
		}

	}

	sel = executeQuery(&query, context, filter, InProgress, HowToLock, 0, nil, &err);
	if (outErr != nil)
		*outErr = err;

	return sel;

}


EntityCollection* LocalEntityModel::BuildCollectionFromJSON(const VJSONObject* from, VError& outErr)
{
	EntityCollection* result = nil;
	outErr = VE_OK;
	VString datasetname;;
	from->GetPropertyAsString("collectionID", datasetname);
	VUUID datasetID;
	datasetID.FromString(datasetname);
	DataSet* dataset = VDBMgr::GetManager()->RetainKeptDataSet(datasetID);
	if (dataset != nil)
	{
		result = RetainRefCountable(dataset->GetSel());
		dataset->Release();
	}
	else
	{
		outErr = ThrowBaseError(rest::dataset_not_found, datasetname);
	}

	return result;
}


EntityCollection* LocalEntityModel::NewCollection(bool ordered, bool safeRef) const
{
	Selection* sel = nil;
	EntityCollection* result = nil;
	DB4D_SelectionType inSelectionType = DB4D_Sel_Bittab;

	if (ordered)
		inSelectionType = DB4D_Sel_SmallSel;

	switch (inSelectionType)
	{
	case DB4D_Sel_OneRecSel:
	case DB4D_Sel_SmallSel:
		sel = new PetiteSel(fMainTable->GetDF(), fMainTable->GetOwner(), fMainTable->GetNum());
		break;

	case DB4D_Sel_LongSel:
		sel = new LongSel(fMainTable->GetDF(), fMainTable->GetOwner());
		((LongSel*)sel)->PutInCache();
		break;

	case DB4D_Sel_Bittab:
		sel = new BitSel(fMainTable->GetDF(), fMainTable->GetOwner());
		break;
	}

	if (sel != nil)
	{
		result = new LocalEntityCollection((LocalEntityModel*)this, sel);
		sel->Release();
	}

	return result;
}


EntityCollection* LocalEntityModel::NewCollection(const VectorOfVString& primKeys, VError& err, BaseTaskInfo* context) const
{
	EntityCollection* result = nil;
	BitSel* bsel = new BitSel(fMainTable->GetDF(), fMainTable->GetOwner());
	for (VectorOfVString::const_iterator cur = primKeys.begin(), end = primKeys.end(); cur != end && err == VE_OK; ++cur)
	{
		RecIDType recnum = ((LocalEntityModel*)this)->getEntityNumWithPrimKey(*cur, context, err);
		if (recnum != -1)
			bsel->GetBittab()->Set(recnum);
	}
	result = new LocalEntityCollection((LocalEntityModel*)this, bsel);
	bsel->Release();
	return result;
}



EntityRecord* LocalEntityModel::newEntity(BaseTaskInfo* inContext, bool forClone) const
{
	VError err;
	if (forClone)
	{
		EntityRecord* erec = new LocalEntityRecord((LocalEntityModel*)this, nil, inContext);
		return erec;
	}
	else
	{
		FicheInMem* rec = fMainTable->GetDF()->NewRecord(err, inContext);
		if (rec == nil)
			return nil;
		else
		{
			EntityRecord* erec = new LocalEntityRecord((LocalEntityModel*)this, rec, inContext);
			rec->Release();
			erec->CallInitEvent(inContext);
			return erec;
		}
	}
}





RecIDType LocalEntityModel::getEntityNumWithPrimKey(const VString& primkey, BaseTaskInfo* context, VError& err)
{
	VectorOfVString xprimkey;
	Wordizer ww(primkey);
	ww.ExctractStrings(xprimkey, true, '&', false);
	return getEntityNumWithPrimKey(xprimkey, context, err);
}


RecIDType LocalEntityModel::getEntityNumWithPrimKey(const VectorOfVString& primkey, BaseTaskInfo* context, VError& err)
{
	RecIDType result = -1;
	err = VE_OK;
	if (!fPrimaryAtts.empty())
	{
		if (fPrimaryAtts.size() == primkey.size())
		{
			SearchTab query(fMainTable);
			VectorOfVString::const_iterator curkey = primkey.begin();
			bool first = true;
			for (IdentifyingAttributeCollection::iterator cur = fPrimaryAtts.begin(), end = fPrimaryAtts.end(); cur != end; cur++, curkey++)
			{
				if (first)
					first = false;
				else
					query.AddSearchLineBoolOper(DB4D_And);
				query.AddSearchLineSimple(fMainTable->GetNum(), cur->fAtt->GetFieldPos(), DB4D_Equal, &(*curkey));
			}
			EntityCollection* sel = executeQuery(&query, context, nil, nil, DB4D_Do_Not_Lock, 0, nil, &err);
			if (sel != nil)
			{
				if (sel->GetLength(context) > 0)
				{
					Selection* xsel = ((LocalEntityCollection*)sel)->GetSel();
					result = xsel->GetFic(0);
				}
				sel->Release();
			}
		}
		else
			err = ThrowError(VE_DB4D_PRIMKEY_MALFORMED);
	}
	else
	{
		//err = ThrowError(VE_DB4D_ENTITY_HAS_NO_PRIMKEY);
		if (primkey.size() == 1)
		{
			const VString& s = primkey[0];
			if (!s.IsEmpty())
				result = s.GetLong();

		}
		else
			err = ThrowError(VE_DB4D_PRIMKEY_MALFORMED);
	}

	return result;
}


RecIDType LocalEntityModel::getEntityNumWithPrimKey(const VValueBag& primkey, BaseTaskInfo* context, VError& err, bool ErrOnNull)
{
	err = VE_OK;
	RecIDType result = -1;
	if (!fPrimaryAtts.empty())
	{
		SearchTab query(fMainTable);
		bool first = true;
		for (IdentifyingAttributeCollection::iterator cur = fPrimaryAtts.begin(), end = fPrimaryAtts.end(); cur != end && err == VE_OK; cur++)
		{
			EntityAttribute* att = cur->fAtt;
			const VValueSingle* cv = primkey.GetAttribute(att->GetName());
			if (cv != nil)
			{
				if (first)
					first = false;
				else
					query.AddSearchLineBoolOper(DB4D_And);
				query.AddSearchLineSimple(fMainTable->GetNum(), cur->fAtt->GetFieldPos(), DB4D_Equal, cv);
			}
			else
			{
				if (ErrOnNull)
					err = ThrowError(VE_DB4D_PRIMKEY_MALFORMED);
				else
					err = VE_DB4D_PRIMKEY_MALFORMED;
			}
		}
		if (err == VE_OK)
		{
			EntityCollection* sel = executeQuery(&query, context, nil, nil, DB4D_Do_Not_Lock, 0, nil, &err);
			if (sel != nil)
			{
				if (sel->GetLength(context) > 0)
				{
					Selection* xsel = ((LocalEntityCollection*)sel)->GetSel();
					result = xsel->GetFic(0);
				}
				sel->Release();
			}
		}
	}
	else
	{
		if (ErrOnNull)
			err = ThrowError(VE_DB4D_ENTITY_HAS_NO_PRIMKEY);
	}

	return result;
}



RecIDType LocalEntityModel::getEntityNumWithPrimKey(const VectorOfVValue& primkey, BaseTaskInfo* context, VError& err)
{
	RecIDType result = -1;
	err = VE_OK;
	if (!fPrimaryAtts.empty())
	{
		if (fPrimaryAtts.size() == primkey.size())
		{
			SearchTab query(fMainTable);
			VectorOfVValue::const_iterator curkey = primkey.begin();
			bool first = true;
			for (IdentifyingAttributeCollection::iterator cur = fPrimaryAtts.begin(), end = fPrimaryAtts.end(); cur != end; cur++, curkey++)
			{
				if (first)
					first = false;
				else
					query.AddSearchLineBoolOper(DB4D_And);
				query.AddSearchLineSimple(fMainTable->GetNum(), cur->fAtt->GetFieldPos(), DB4D_Equal, *curkey);
			}
			EntityCollection* sel = executeQuery(&query, context, nil, nil, DB4D_Do_Not_Lock, 0, nil, &err);
			if (sel != nil)
			{
				if (sel->GetLength(context) > 0)
				{
					Selection* xsel = ((LocalEntityCollection*)sel)->GetSel();
					result = xsel->GetFic(0);
				}
				sel->Release();
			}
		}
		else
			err = ThrowError(VE_DB4D_PRIMKEY_MALFORMED);
	}
	else
	{
		//err = ThrowError(VE_DB4D_ENTITY_HAS_NO_PRIMKEY);
		if (primkey.size() == 1)
		{
			const VValueSingle* cv = primkey[0];
			if (cv != nil && !cv->IsNull())
			{
				if (cv->GetValueKind() == VK_STRING)
				{
					if ( !((VString*)cv)->IsEmpty())
						result = cv->GetLong();
				}
				else
					result = cv->GetLong();
			}

		}
		else
			err = ThrowError(VE_DB4D_PRIMKEY_MALFORMED);	}

	return result;
}


RecIDType LocalEntityModel::getEntityNumWithPrimKey(VJSObject objkey, BaseTaskInfo* context, VError& err)
{
	RecIDType result = -1;
	VJSValue jsval(objkey.GetContext());
	jsval = objkey.GetProperty("__RECID");
	if (jsval.IsNumber())
	{
		jsval.GetLong(&result);
	}
	else
	{
		if (!fPrimaryAtts.empty())
		{
			SearchTab query(fMainTable);
			bool first = true;
			bool okcont = true;
			for (IdentifyingAttributeCollection::iterator cur = fPrimaryAtts.begin(), end = fPrimaryAtts.end(); cur != end && okcont ; cur++)
			{
				EntityAttribute* att = cur->fAtt;
				jsval = objkey.GetProperty(att->GetName());
				if (!jsval.IsUndefined() && !jsval.IsNull())
				{
					VValueSingle* cv = jsval.CreateVValue();
					if (cv != nil)
					{
						if (first)
							first = false;
						else
							query.AddSearchLineBoolOper(DB4D_And);
						query.AddSearchLineSimple(fMainTable->GetNum(), cur->fAtt->GetFieldPos(), DB4D_Equal, cv);
					}
					else
						okcont = false;
				}
				else
					okcont = false;
			}

			if (okcont)
			{
				EntityCollection* sel = executeQuery(&query, context, nil, nil, DB4D_Do_Not_Lock, 0, nil, &err);
				if (sel != nil)
				{
					if (sel->GetLength(context) > 0)
					{
						Selection* xsel = ((LocalEntityCollection*)sel)->GetSel();
						result = xsel->GetFic(0);
					}
					sel->Release();
				}
			}

		}
	}
	return result;
}



RecIDType LocalEntityModel::getEntityNumWithIdentifyingAtts(const VectorOfVString& idents, BaseTaskInfo* context, VError& err)
{
	RecIDType result = -1;
	err = VE_OK;
	if (!fIdentifyingAtts.empty())
	{
		SearchTab query(fMainTable);
		bool enough = true;
		VectorOfVString::const_iterator curkey = idents.begin();
		bool first = true;
		for (IdentifyingAttributeCollection::iterator cur = fIdentifyingAtts.begin(), end = fIdentifyingAtts.end(); cur != end; cur++, curkey++)
		{
			if (curkey == idents.end())
			{
				if (!cur->fOptionnel)
					enough = false;
				break;
			}
			if (first)
				first = false;
			else
				query.AddSearchLineBoolOper(DB4D_And);
			query.AddSearchLineSimple(fMainTable->GetNum(), cur->fAtt->GetFieldPos(), DB4D_Equal, &(*curkey));
		}
		if (enough)
		{
			EntityCollection* sel = executeQuery(&query, context, nil, nil, DB4D_Do_Not_Lock, 0, nil, &err);
			if (sel != nil)
			{
				if (sel->GetLength(context) > 0)
				{
					Selection* xsel = ((LocalEntityCollection*)sel)->GetSel();
					result = xsel->GetFic(0);
				}
				sel->Release();
			}
		}
		else
			err = ThrowError(VE_DB4D_IDENTKEY_MALFORMED);
	}
	else
		err = ThrowError(VE_DB4D_ENTITY_HAS_NO_IDENTKEY);

	return result;
}


RecIDType LocalEntityModel::getEntityNumWithIdentifyingAtts(const VValueBag& bagData, BaseTaskInfo* context, VError& err, bool ErrOnNull)
{
	RecIDType result = -1;
	bool enough = true;
	const IdentifyingAttributeCollection* idents = GetIdentifyingAtts();
	if (!idents->empty())
	{
		SearchTab xquery(fMainTable);
		bool first = true;
		for (IdentifyingAttributeCollection::const_iterator cur = idents->begin(), end = idents->end(); cur != end; cur++)
		{
			const EntityAttribute* att = cur->fAtt;
			const VValueSingle* cv = bagData.GetAttribute(att->GetName());
			if (cv != nil)
			{
				if (first)
					first = false;
				else
					xquery.AddSearchLineBoolOper(DB4D_And);
				xquery.AddSearchLineSimple(fMainTable->GetNum(), att->GetFieldPos(), DB4D_Like, cv);
			}
			else
			{
				if (!cur->fOptionnel)
				{
					enough = false;
					break;
				}
			}
		}
		if (enough)
		{
			EntityCollection* sel = executeQuery(&xquery, context, nil, nil, DB4D_Do_Not_Lock, 0, nil, &err);
			if (sel != nil)
			{
				if (sel->GetLength(context) > 0)
				{
					Selection* xsel = ((LocalEntityCollection*)sel)->GetSel();
					result = xsel->GetFic(0);
				}
				sel->Release();
			}
		}
	}
	return result;
}


RecIDType LocalEntityModel::getEntityNumWithIdentifyingAtts(const VectorOfVValue& idents, BaseTaskInfo* context, VError& err)
{
	RecIDType result = -1;
	err = VE_OK;
	if (!fIdentifyingAtts.empty())
	{
		SearchTab query(fMainTable);
		bool enough = true;
		VectorOfVValue::const_iterator curkey = idents.begin();
		bool first = true;
		for (IdentifyingAttributeCollection::iterator cur = fIdentifyingAtts.begin(), end = fIdentifyingAtts.end(); cur != end; cur++, curkey++)
		{
			if (curkey == idents.end())
			{
				if (!cur->fOptionnel)
					enough = false;
				break;
			}
			if (first)
				first = false;
			else
				query.AddSearchLineBoolOper(DB4D_And);
			query.AddSearchLineSimple(fMainTable->GetNum(), cur->fAtt->GetFieldPos(), DB4D_Equal, *curkey);
		}
		if (enough)
		{
			EntityCollection* sel = executeQuery(&query, context, nil, nil, DB4D_Do_Not_Lock, 0, nil, &err);
			if (sel != nil)
			{
				if (sel->GetLength(context) > 0)
				{
					Selection* xsel = ((LocalEntityCollection*)sel)->GetSel();
					result = xsel->GetFic(0);
				}
				sel->Release();
			}
		}
		else
			err = ThrowError(VE_DB4D_IDENTKEY_MALFORMED);
	}
	else
		err = ThrowError(VE_DB4D_ENTITY_HAS_NO_IDENTKEY);

	return result;
}





EntityRecord* LocalEntityModel::findEntityWithPrimKey(VJSObject objkey, BaseTaskInfo* context, VError& err, DB4D_Way_of_Locking HowToLock)
{
	err = VE_OK;
	EntityRecord* erec = nil;
	RecIDType numrec = getEntityNumWithPrimKey(objkey, context, err);
	if (numrec != -1)
	{
		erec = LoadEntity(numrec, err, HowToLock, context);
	}

	return erec;
}


EntityRecord* LocalEntityModel::findEntityWithPrimKey(const VString& primkey, BaseTaskInfo* context, VError& err, DB4D_Way_of_Locking HowToLock)
{
	err = VE_OK;
	EntityRecord* erec = nil;
	RecIDType numrec = getEntityNumWithPrimKey(primkey, context, err);
	if (numrec != -1)
	{
		erec = LoadEntity(numrec, err, HowToLock, context);
	}

	return erec;
}


EntityRecord* LocalEntityModel::findEntityWithPrimKey(const VectorOfVString& primkey, BaseTaskInfo* context, VError& err, DB4D_Way_of_Locking HowToLock)
{
	err = VE_OK;
	EntityRecord* erec = nil;
	RecIDType numrec = getEntityNumWithPrimKey(primkey, context, err);
	if (numrec != -1)
	{
		erec = LoadEntity(numrec, err, HowToLock, context);
	}

	return erec;
}

EntityRecord* LocalEntityModel::findEntityWithPrimKey(const VValueBag& primkey, BaseTaskInfo* context, VError& err, DB4D_Way_of_Locking HowToLock)
{
	err = VE_OK;
	EntityRecord* erec = nil;
	RecIDType numrec = getEntityNumWithPrimKey(primkey, context, err);
	if (numrec != -1)
	{
		erec = LoadEntity(numrec, err, HowToLock, context);
	}

	return erec;
}


EntityRecord* LocalEntityModel::findEntityWithPrimKey(const VectorOfVValue& primkey, BaseTaskInfo* context, VError& err, DB4D_Way_of_Locking HowToLock)
{
	err = VE_OK;
	EntityRecord* erec = nil;
	RecIDType numrec = getEntityNumWithPrimKey(primkey, context, err);
	if (numrec != -1)
	{
		erec = LoadEntity(numrec, err, HowToLock, context);
	}

	return erec;
}




EntityRecord* LocalEntityModel::findEntityWithIdentifyingAtts(const VectorOfVString& idents, BaseTaskInfo* context, VError& err, DB4D_Way_of_Locking HowToLock)
{
	err = VE_OK;
	EntityRecord* erec = nil;
	RecIDType numrec = getEntityNumWithIdentifyingAtts(idents, context, err);
	if (numrec != -1)
	{
		erec = LoadEntity(numrec, err, HowToLock, context);
	}

	return erec;
}

EntityRecord* LocalEntityModel::findEntityWithIdentifyingAtts(const VValueBag& idents, BaseTaskInfo* context, VError& err, DB4D_Way_of_Locking HowToLock)
{
	err = VE_OK;
	EntityRecord* erec = nil;
	RecIDType numrec = getEntityNumWithIdentifyingAtts(idents, context, err);
	if (numrec != -1)
	{
		erec = LoadEntity(numrec, err, HowToLock, context);
	}

	return erec;
}


EntityRecord* LocalEntityModel::findEntityWithIdentifyingAtts(const VectorOfVValue& idents, BaseTaskInfo* context, VError& err, DB4D_Way_of_Locking HowToLock)
{
	err = VE_OK;
	EntityRecord* erec = nil;
	RecIDType numrec = getEntityNumWithIdentifyingAtts(idents, context, err);
	if (numrec != -1)
	{
		erec = LoadEntity(numrec, err, HowToLock, context);
	}

	return erec;
}




bool LocalEntityModel::MatchPrimKeyWithDataSource() const
{
	if (fMatchPrimKey == 2)
	{
		if (fPrimaryAtts.empty())
			fMatchPrimKey = 0;
		else
		{
			FieldArray primkeyfields;
			fMainTable->RetainPrimaryKey(primkeyfields);
			if (primkeyfields.GetCount() == 0 || primkeyfields.GetCount() != fPrimaryAtts.size())
				fMatchPrimKey = 0;
			else
			{
				fMatchPrimKey = 1;
				IdentifyingAttributeCollection::const_iterator curatt = fPrimaryAtts.begin();
				for (FieldArray::Iterator cur = primkeyfields.First(), end = primkeyfields.End(); cur != end; cur++, curatt++)
				{
					if ((*cur)->GetPosInRec() != curatt->fAtt->GetFieldPos())
						fMatchPrimKey = 0;
				}

			}
			for (FieldArray::Iterator cur = primkeyfields.First(), end = primkeyfields.End(); cur != end; cur++)
				(*cur)->Release();
		}
	}
	return fMatchPrimKey == 1;
}

/*

VError LocalEntityModel::compute(EntityAttributeSortedSelection& atts, EntityCollection* collec, VJSObject& outObj, BaseTaskInfo* context, JS4D::ContextRef jscontext)
{
	outObj.MakeEmpty();

	vector<computeResult> results;
	results.resize(atts.size());
	VError err = VE_OK;
	if (okperm(context, fPerms[DB4D_EM_Read_Perm]) || okperm(context, fPerms[DB4D_EM_Update_Perm]) || okperm(context, fPerms[DB4D_EM_Delete_Perm]))
	{
		if (collec == nil)
			collec = SelectAllEntities(context, &err);
		else
			collec->Retain();

		LocalEntityCollection* localCollec = dynamic_cast<LocalEntityCollection*>(collec);
		Selection* sel = nil;
		if (localCollec != nil)
			sel = localCollec->GetSel();

		if (sel != nil && err == VE_OK)
		{
			SelectionIterator itersel(sel);
			sLONG currec = itersel.FirstRecord();

			while (currec != -1 && err == VE_OK)
			{
				EntityRecord* rec = LoadEntity(currec, err, DB4D_Do_Not_Lock, context, false);
				if (rec != nil)
				{
					sLONG respos = 0;
					for (EntityAttributeSortedSelection::const_iterator cur = atts.begin(), end = atts.end(); cur != end && err == VE_OK; cur++, respos++)
					{
						computeResult& result = results[respos];
						EntityAttributeValue* eval = rec->getAttributeValue(cur->fAttribute, err, context);
						if (err == VE_OK)
						{
							if (eval->GetAttributeKind() == eav_vvalue)
							{
								VValueSingle* cv = eval->getVValue();
								if (cv != nil && !cv->IsNull())
								{
									result.fCount++;
									result.fSum += cv->GetReal();
									if (result.first)
									{
										result.fMinVal = cv->Clone();
										result.fMaxVal = cv->Clone();
										result.first = false;
									}
									else
									{
										if (result.fMinVal->CompareToSameKind(cv, true) == CR_BIGGER)
											result.fMinVal->FromValueSameKind(cv);
										if (result.fMaxVal->CompareToSameKind(cv, true) == CR_SMALLER)
											result.fMaxVal->FromValueSameKind(cv);
									}
								}
							}
						}
					}
					QuickReleaseRefCountable(rec);
				}
				currec = itersel.NextRecord();
			}


		}

		sLONG respos = 0;
		for (EntityAttributeSortedSelection::const_iterator cur = atts.begin(), end = atts.end(); cur != end && err == VE_OK; cur++, respos++)
		{
			computeResult& result = results[respos];
			VJSObject subobject(outObj, cur->fAttribute->GetName());
			subobject.SetProperty("count", result.fCount);
			if (!result.first)
			{
				if (cur->fAttribute->IsSummable())
				{
					subobject.SetProperty("sum", result.fSum);
					if (result.fCount != 0)
						subobject.SetProperty("average", result.fSum / (Real)result.fCount);
					else
						subobject.SetNullProperty("average");
				}
				subobject.SetProperty("min", result.fMinVal);
				subobject.SetProperty("max", result.fMaxVal);
			}
			else
			{	
				if (cur->fAttribute->IsSummable())
				{
					subobject.SetNullProperty("average");
					subobject.SetProperty("sum", 0.0);
				}
				subobject.SetNullProperty("min");
				subobject.SetNullProperty("max");
			}
		}

		QuickReleaseRefCountable(collec);
	}
	else
	{
		context->SetPermError();
		err = ThrowError(VE_DB4D_NO_PERM_TO_READ);
	}
	return err;
}
*/

VJSValue LocalEntityModel::call_Method(const VString& inMethodName,VJSArray& params, VJSObject& thisObj, BaseTaskInfo* context, JS4D::ContextRef jscontext, VError& err)
{
	VJSContext vjsContext(jscontext);
	VJSValue nullret(vjsContext);
	nullret.SetNull();
	return nullret;
}


VError LocalEntityModel::SetAutoSequenceNumber(sLONG8 newnum, BaseTaskInfo* context)
{
	AutoSeqNumber* seq = fMainTable->GetSeqNum(context->GetEncapsuleur());
	if (seq != nil)
	{
		seq->SetCurrentValue(newnum);
	}
	//QuickReleaseRefCountable(seq);
	return VE_OK;
}


bool LocalEntityModel::IsIndexed(const EntityAttribute* att) const
{
	bool result = false;
	Field* cri = RetainField(att, true);
	if (cri != nil)
		result = cri->IsIndexed();
	QuickReleaseRefCountable(cri);
	return result;
}


bool LocalEntityModel::IsFullTextIndexed(const EntityAttribute* att) const
{
	bool result = false;
	Field* cri = RetainField(att, true);
	if (cri != nil)
		result = cri->IsFullTextIndexed();
	QuickReleaseRefCountable(cri);
	return result;
}



bool LocalEntityModel::IsIndexable(const EntityAttribute* att) const
{
	bool result = false;
	Field* cri = RetainField(att, true);
	if (cri != nil)
		result = cri->IsIndexable();
	QuickReleaseRefCountable(cri);
	return result;
}


bool LocalEntityModel::IsFullTextIndexable(const EntityAttribute* att) const
{
	bool result = false;
	Field* cri = RetainField(att, true);
	if (cri != nil)
		result = cri->IsFullTextIndexable();
	QuickReleaseRefCountable(cri);
	return result;
}


Field* LocalEntityModel::RetainDirectField(const EntityAttribute* att) const
{
	if (att->GetKind() == eattr_storage)
		return RetainField(att);
	else
		return nil;
}


Field* LocalEntityModel::RetainField(const EntityAttribute* att, bool includingForeignKey) const
{
	Field* result = nil;
	switch (att->GetKind())
	{
		case eattr_storage:
			if (fMainTable != nil)
				result = fMainTable->RetainField(att->GetFieldPos());
			break;

		case eattr_alias:
			{
				EntityModel* flat = att->getFlattenLastDest();
				if (flat != nil)
				{
					const EntityAttribute* flatatt = flat->getAttribute(att->getFlattenAttributeName());
					if (flatatt != nil)
					{
						result = ((db4dEntityAttribute*)flatatt)->RetainField(flatatt);
					}
				}
			}
			break;

		case eattr_relation_Nto1:
			{
				EntityRelation* rel = att->GetRelPath();
				if (rel != nil && rel->IsSimple() && includingForeignKey)
				{
					result = fMainTable->RetainField(att->GetFieldPos());
				}
			}
			break;

		default:
			result = nil;
			break;
	}
	return result;
}


Table* LocalEntityModel::GetBaseTable()
{
	if (fBaseEm != nil)
	{
		LocalEntityModel* locBaseEm = dynamic_cast<LocalEntityModel*>(fBaseEm);
		return locBaseEm->GetBaseTable();
	}
	else
		return fMainTable;
}


VError LocalEntityModel::MatchExtendedTable()
{
	VError err = VE_OK;
	fDB = ((LocalEntityModelCatalog*)fCatalog)->GetDB();

	if (fBaseEm != nil)
	{
		LocalEntityModel* locBaseEm = dynamic_cast<LocalEntityModel*>(fBaseEm);
		fMainTable = locBaseEm->GetBaseTable();
		if (fMainTable != nil)
		{
			for (EntityAttributeCollection::iterator cur = fAttributes.begin(), end = fAttributes.end(); cur != end && err == VE_OK; ++cur)
			{
				db4dEntityAttribute* entAtt = (db4dEntityAttribute*)*cur;
				VString fieldname = entAtt->GetName();
				fieldname.Truncate(31);
				Field* cri = fMainTable->FindAndRetainFieldRef(fieldname);
				if (cri != nil)
				{
					entAtt->SetFieldPos(cri->GetPosInRec());
					cri->Release();
				}
				else
				{
				}
			}
		}
	}
	return err;
}



VError LocalEntityModel::MatchWithTable()
{
	VError err = VE_OK;
	fDB = ((LocalEntityModelCatalog*)fCatalog)->GetDB();

	if (fBaseEm == nil)
	{
		fMainTable = nil;
		VString tname = fName;
		tname.Truncate(31);

		for (sLONG i = (sLONG)fAlternateTableNames.size(); (i > 0) && (fMainTable == nil); --i)
		{
			VString tablename = fAlternateTableNames[i-1];
			fMainTable = fDB->FindAndRetainTableRef(tablename);
		}

		if (fMainTable == nil)
			fMainTable = fDB->FindOrCreateTableRef(tname, err, fDB4DUUID);

		if (fMainTable != nil)
		{
			fMainTable->ResetAttProps();
			for (EntityAttributeCollection::iterator cur = fAttributes.begin(), end = fAttributes.end(); cur != end && err == VE_OK; ++cur)
			{
				db4dEntityAttribute* entAtt = (db4dEntityAttribute*)*cur;
				err = entAtt->MatchField(fMainTable);
			}

			if ((err == VE_OK) && !fPrimaryAtts.empty())
			{
				NumFieldArray prim;
				prim.SetCount((sLONG)fPrimaryAtts.size());
				sLONG i = 1;
				for (IdentifyingAttributeCollection::iterator cur = fPrimaryAtts.begin(), end = fPrimaryAtts.end(); cur != end; cur++, i++)
				{
					prim[i] = cur->fAtt->GetFieldPos();
				}
				err = fMainTable->SetPrimaryKeySilently(prim, nil);
			}


			if (err == VE_OK)
			{
				fMainTable->ReCalc();
				fMainTable->save();
				VUUID xid;
				fMainTable->GetUUID(xid);
				fDB->AddObjectID(objInBase_Table, fMainTable, xid);
			}
		}
	}
	return err;
}




VError LocalEntityModel::BuildIndexes(bool dataWasAlreadyThere)
{
	VError err = VE_OK;
	if (fMainTable != nil)
	{
		for (IdentifyingAttributeCollection::iterator cur = fPrimaryAtts.begin(), end = fPrimaryAtts.end(); cur != end; cur++)
		{
			cur->fAtt->SetIndexKind("btree");
		}

		for (EntityAttributeCollection::iterator cur = fAttributes.begin(), end = fAttributes.end(); cur != end && err == VE_OK; ++cur)
		{
			db4dEntityAttribute* entAtt = (db4dEntityAttribute*)*cur;
			err = entAtt->BuildIndexes(fMainTable, dataWasAlreadyThere);
		}

		if (err == VE_OK)
			fMainTable->save(true);
	}
	return err;
}




VError LocalEntityModel::BuildLocalEntityModelRelations(LocalEntityModelCatalog* catalog)
{
	VError err = VE_OK;
	Table* from = fMainTable;
	LocalEntityModel* em = this;

	DepRelationArrayIncluded* relsNto1 = from->GetRelNto1Deps();
	for (DepRelationArrayIncluded::Iterator cur = relsNto1->First(), end = relsNto1->End(); cur != end; cur++)
	{
		Relation* rel = *cur;
		VString relname = rel->GetName();
		if (!relname.IsEmpty())
		{
			Field* source = rel->GetSource();
			Field* dest = rel->GetDest();
			if (source != nil && dest != nil && !source->GetHideInRest() && !dest->GetHideInRest() 
				&& !source->GetOwner()->GetHideInRest() && !dest->GetOwner()->GetHideInRest() )
			{
				EntityAttribute* att = new EntityAttribute(em);
				att->SetKind(eattr_relation_Nto1);
				att->SetName(relname);
				att->SetOverWrite(true);
				att->SetRelation(rel, erel_Nto1, catalog);

				em->fAttributes.push_back(att);
				em->fAttributesByName[att->GetName()] = att;
				att->SetPosInOwner((sLONG)em->fAttributes.size());
			}
		}
	}

	//if (false)
	{
		DepRelationArrayIncluded* rels1toN = from->GetRel1toNDeps();
		for (DepRelationArrayIncluded::Iterator cur = rels1toN->First(), end = rels1toN->End(); cur != end; cur++)
		{
			Relation* rel = *cur;
			VString relname = rel->GetOppositeName();
			if (!relname.IsEmpty())
			{
				Field* source = rel->GetSource();
				Field* dest = rel->GetDest();
				if (source != nil && dest != nil && !source->GetHideInRest() && !dest->GetHideInRest() 
					&& !source->GetOwner()->GetHideInRest() && !dest->GetOwner()->GetHideInRest() )
				{
					EntityAttribute* att = new EntityAttribute(em);
					att->SetKind(eattr_relation_1toN);
					att->SetName(relname);
					att->SetOverWrite(true);
					att->SetRelation(rel, erel_1toN, catalog);
					att->SetReversePath(true);

					em->fAttributes.push_back(att);
					em->fAttributesByName[att->GetName()] = att;
					att->SetPosInOwner((sLONG)em->fAttributes.size());
				}
			}
		}
	}
	return err;
}


			// --------------------------------------------------------------------



VError db4dEntityAttribute::MatchField(Table* tt)
{
	VError err = VE_OK;
	sLONG typ = 0;

	if (fKind == eattr_storage)
	{
		typ = ComputeScalarType();
		if (typ == 0)
			err = ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_TYPE_MISSING);
	}
	else if ((fKind == eattr_relation_Nto1) && fIsForeignKey)
	{
		EntityModel* otherEntity = fSubEntity;
		if (otherEntity != nil)
		{
			const IdentifyingAttributeCollection* otherPrim = otherEntity->GetPrimaryAtts();
			if (otherPrim != nil)
			{
				const EntityAttribute* otherAtt = (*otherPrim)[0].fAtt;
				typ = otherAtt->ComputeScalarType();
				if (typ == 0)
					err = otherAtt->ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_TYPE_MISSING);

			}
		}
	}

	if (typ != 0 && err == VE_OK)
	{
		//if (typ == VK_STRING)
		//typ = VK_TEXT;
		VString fieldname = fName;
		fieldname.Truncate(31);
		Field* cri = nil;
		if (!fAlternateFieldNames.empty())
		{
			for (sLONG i = (sLONG)fAlternateFieldNames.size(); (i > 0) && (cri == nil); --i)
			{
				VString s = fAlternateFieldNames[i-1];
				cri = tt->FindAndRetainFieldRef(s);
			}
		}
		if (cri == nil)
			cri = tt->FindAndRetainFieldRef(fieldname);
		if (cri == nil)
		{
			sLONG pos = tt->FindNextFieldFree();
			if (pos == -1)
				pos = tt->GetNbCrit()+1;
			SetFieldPos(pos);
			cri = new Field(typ, pos, tt);
			VUUID xid;
			xid.Regenerate();
			cri->SetUUID(xid);
			err = tt->AddFieldSilently(cri, pos);

		}
		else
		{
			if (typ != cri->GetTyp())
			{
				cri->SetTyp(typ);
			}
		}

		Critere* xcri = cri->GetCritere();
		xcri->SetName(fName);
		fFieldPos = cri->GetPosInRec();
		if ((typ == VK_STRING) || (typ == VK_STRING_UTF8))
			xcri->SetLimitingLen(fLimitingLen);
		xcri->SetAutoSeq(fAutoSeq);
		xcri->SetAutoGenerate(fAutoGen);
		xcri->SetNot_Null(fNotNull);
		xcri->SetNeverNull(fNullToEmpty);
		xcri->SetUnique(fUnique);
		xcri->SetOutsideData(fOuterBlob);
		xcri->SetStyledText(fStyledText);
		if ((typ == VK_TEXT) || (typ == VK_TEXT_UTF8))
			xcri->SetTextSwitchSize(fBlobSwitchSize);
		if ((typ == VK_BLOB) || (typ == VK_BLOB_DB4D) || (typ == VK_IMAGE))
			xcri->SetBlobSwitchSize(fBlobSwitchSize);
		if (typ == VK_BLOB_OBJ)
			xcri->SetBlobSwitchSize(0x7FFFFFFF);

		VUUID xid;
		cri->GetUUID(xid);
		tt->GetOwner()->AddObjectID(objInBase_Field, cri, xid);

		VString fref = fOwner->GetName()+"."+fName;
		sLONG smallid = tt->GetOwner()->GetSmallID(fref, err);
		if (smallid != 0)
		{
			tt->AssociateField(smallid, cri);
			cri->SetSmallIDInRec(smallid);
		}

		QuickReleaseRefCountable(cri);
	}

	return err;
}


VError db4dEntityAttribute::BuildIndexes(Table* tt, bool dataWasAlreadyThere)
{
	VError err = VE_OK;

	Base4D* db = tt->GetOwner();

	if (fKind == eattr_storage)
	{
		xbox_assert(fFieldPos > 0);
		VectorOfVString indexKinds;
		if (fIndexKind.IsEmpty() && fUnique)
			fIndexKind = EIndexKinds[DB4D_Index_Btree];
		fIndexKind.GetSubStrings(',', indexKinds, false, true);
		for (VectorOfVString::iterator cur = indexKinds.begin(), end = indexKinds.end(); cur != end; cur++)
		{
			sLONG indextype = EIndexKinds[*cur];
			if (indextype == 9) // objectPath
			{
				if (dataWasAlreadyThere)
				{
					err = db->CreIndexOnField(tt->GetNum(), fFieldPos, DB4D_Index_OnObj, false, nil, nil);
				}
				else
				{
					IndexInfoFromFieldObj* ind = new IndexInfoFromFieldObj(db, tt->GetNum(), fFieldPos, DB4D_Index_Btree);
					db->AddXMLIndex(ind);
					ind->Release();
				}
			}
			else if (indextype == 8) //  keywords
			{
				if (dataWasAlreadyThere)
				{
					err = db->CreIndexOnField(tt->GetNum(), fFieldPos, DB4D_Index_OnKeyWords, false, nil, nil);
				}
				else
				{
					IndexInfoFromFieldLexico* ind = new IndexInfoFromFieldLexico(db, tt->GetNum(), fFieldPos, DB4D_Index_Btree);
					db->AddXMLIndex(ind);
					ind->Release();
				}
			}
			else
			{
				if (indextype == DB4D_Index_Btree || indextype == DB4D_Index_BtreeWithCluster || indextype == DB4D_Index_AutoType)
				{
					if (dataWasAlreadyThere)
					{
						err = db->CreIndexOnField(tt->GetNum(), fFieldPos, DB4D_Index_Btree, fUnique, nil, nil);
					}
					else
					{
						IndexInfoFromField* ind = new IndexInfoFromField(db, tt->GetNum(), fFieldPos, indextype, false, true);
						db->AddXMLIndex(ind);
						ind->Release();
					}
				}
			}
		}
	}
	else if ((fKind == eattr_relation_Nto1) && fIsForeignKey)
	{
		xbox_assert(fFieldPos > 0);

		if (dataWasAlreadyThere)
		{
			err = db->CreIndexOnField(tt->GetNum(), fFieldPos, DB4D_Index_BtreeWithCluster, false, nil, nil);
		}
		else
		{
			IndexInfoFromField* ind = new IndexInfoFromField(db, tt->GetNum(), fFieldPos, DB4D_Index_BtreeWithCluster, false, true);
			db->AddXMLIndex(ind);
			ind->Release();
		}
	}

	return err;
}



// ---------------------------------------------------------------------------------------------------------------



EntityRecord* LocalEntityRecord::do_LoadRelatedEntity(const EntityAttribute* inAttribute, const EntityAttribute* relatedAttribute, BaseTaskInfo* context, VError& err, DB4D_Way_of_Locking HowToLock)
{
	err = VE_OK;
	EntityRecord* result = nil;
	db4dEntityAttribute* att = (db4dEntityAttribute*)inAttribute;

	Field* cri = att->RetainField(true);
	if (cri != nil && relatedAttribute != nil)
	{
		VValueSingle* cv = fMainRec->GetFieldValue(cri, err);
		if (cv != nil && !cv->IsNull())
		{
			if (inAttribute->RelIsNotOnPrimKey())
			{
				result = relatedAttribute->GetModel()->findEntityWithAttribute(inAttribute, cv, context, err, HowToLock);
			}
			else
			{
				VectorOfVValue primkey;
				primkey.push_back(cv);
				result = relatedAttribute->GetModel()->findEntityWithPrimKey(primkey, context, err, HowToLock);
			}
		}
	}
	else
		err = ThrowError(VE_DB4D_WRONG_ATTRIBUTE_KIND);

	QuickReleaseRefCountable(cri);
	return result;
}


PrimKey* LocalEntityRecord::RetainRelatedAttributeKey(const EntityAttribute* inAttribute, VError& err)
{
	PrimKey* result = nil;
	err = VE_OK;
	LocalEntityModel* locmodel = dynamic_cast<LocalEntityModel*>(fModel);
	if (locmodel != nil && inAttribute->GetKind() == eattr_relation_Nto1)
	{
		Field* cri = locmodel->RetainField(inAttribute, true);
		if (cri != nil)
		{
			VValueSingle* cv = fMainRec->GetFieldValue(cri, err);
			if (cv != nil && !cv->IsNull())
			{
				result = new PrimKey();
				result->AddPart(cv);
			}
			cri->Release();
		}
		else
		{
			SubEntityCache* cache = GetSubEntityCache(inAttribute->GetPathID(), err, true, inAttribute->GetSubEntityModel(), fContext);
			if (cache != nil)
			{
				EntityRecord* erec = cache->GetErec();
				if (erec != nil)
				{
					LocalEntityRecord* locrec = dynamic_cast<LocalEntityRecord*>(erec);
					result = locrec->fMainRec->RetainPrimKey(err, false);
				}
			}
		}
	}
	return result;
}



EntityAttributeValue* LocalEntityRecord::do_getAttributeValue(const EntityAttribute* inAttribute, VError& err, BaseTaskInfo* context, bool restrictValue)
{
	err = VE_OK;
	EntityAttributeValue* result = nil;
	xbox_assert(inAttribute != nil);
	if (inAttribute == nil)
		return nil;
	xbox_assert(fModel != nil && fModel->isExtendedFrom(inAttribute->GetOwner()));
	//xbox_assert(inAttribute->GetOwner() == fModel);

	sLONG pos = inAttribute->GetPosInOwner();
	xbox_assert(pos > 0 && pos <= fValues.size());

	result = fValues[pos-1];
	EntityAttributeKind kind = inAttribute->GetKind();

	if (result == nil || kind == eattr_computedField || kind == eattr_alias)
	{
		bool mustCallEvent = (result == nil);

		switch(kind)
		{
			case eattr_storage:
			case eattr_alias:
				{
					if (kind == eattr_alias)
					{
						bool needReload = true /*SubEntityCacheNeedsActivation(inAttribute->GetPathID()) ||  result == nil */;
						if (needReload)
						{
							VValueSingle* cv3 = nil;
							if (result == nil)
								result = new EntityAttributeValue(this, inAttribute, eav_vvalue, nil, true);
							SubEntityCache* cache = GetSubEntityCache(inAttribute->GetPathID(), err, true, inAttribute->GetSubEntityModel(), context);
							if (cache != nil)
							{
								EntityRecord* erec = cache->GetErec();
								if (erec != nil)
								{
									EntityAttributeValue* result2 = erec->getAttributeValue(inAttribute->getFlattenAttributeName(), err, context, restrictValue);
									if (result2 != nil)
									{
										VValueSingle* cv2 = result2->getVValue();
										if (cv2 != nil)
											cv3 = cv2->Clone();
									}
								}
							}
							result->setVValue(cv3);
						}
					}
					else
					{
						FicheInMem* rec = fMainRec;
						if (rec != nil)
						{
							sLONG fieldtyp = 0;
							db4dEntityAttribute* att = (db4dEntityAttribute*)inAttribute;

							fieldtyp = att->GetTable()->GetFieldType(att->GetFieldPos());

							if (restrictValue && (fieldtyp == VK_IMAGE || fieldtyp == VK_BLOB || fieldtyp == VK_BLOB_DB4D))
							{
								if (!ContainsBlobData(inAttribute, context))
									result = nil;
								else
								{
									if (fieldtyp == VK_IMAGE)
										result = (EntityAttributeValue*)-2;
									else
										result = (EntityAttributeValue*)-3;
								}
							}
							else
							{
								VValueSingle* cv = rec->GetNthField(att->GetFieldPos(), err);
								if (err == VE_OK && cv != nil)
								{
									result = new EntityAttributeValue(this, inAttribute, eav_vvalue, cv);
									if (result == nil)
										err = ThrowBaseError(memfull);
								}
							}
						}
						else
						{
							sLONG xdebug = 1;
							/*
							if (att->IsAutoSeq())
							{
								VValueSingle* cv = res->getVValue();
								if (cv != nil)
								{
									if (cv->IsNull())
									{
										StErrorContextInstaller errs(false);
										AutoSeqNumber* seq = asscrit->GetSeqNum(nil);
										if (seq != nil)
										{
											sLONG8 val = seq->GetNewValue(fToken);
											if (val != -1)
												chp->FromLong8(val);
										}
									}
								}
							}
							*/
						}
					}
				}
				break;

			case eattr_relation_Nto1:
				{
					if (restrictValue)
						result = (EntityAttributeValue*)-4;
					else
					{
						SubEntityCache* cache = GetSubEntityCache(inAttribute->GetPathID(), err, true, inAttribute->GetSubEntityModel(), context);
						if (cache != nil)
						{
							EntityRecord* erec = cache->GetErec();
							result = new EntityAttributeValue(this, inAttribute, eav_subentity, erec, inAttribute->GetSubEntityModel());
							if (result == nil)
								err = ThrowBaseError(memfull);
						}
					}
				}
				break;

			case eattr_relation_1toN:
				{
					if (restrictValue)
						result = (EntityAttributeValue*)-5;
					else
					{
						EntityCollection* sel = nil;
						SubEntityCache* cache = GetSubEntityCache(inAttribute->GetPathID(), err, false, inAttribute->GetSubEntityModel(), context);
						if (cache != nil)
							sel = cache->GetSel();
						result = new EntityAttributeValue(this, inAttribute, eav_selOfSubentity, sel, inAttribute->GetSubEntityModel());
						if (result == nil)
							err = ThrowBaseError(memfull);
					}

				}
				break;

			case eattr_composition:
				{
					EntityCollection* sel = nil;
					SubEntityCache* cache = GetSubEntityCache(inAttribute->GetPathID(), err, false, inAttribute->GetSubEntityModel(), context);
					if (cache != nil)
						sel = cache->GetSel();
					result = new EntityAttributeValue(this, inAttribute, eav_composition, sel, inAttribute->GetSubEntityModel());
					if (result == nil)
						err = ThrowBaseError(memfull);
				}
				break;

			case eattr_computedField:
				{
					if (result == nil)
						result = new EntityAttributeValue(this, inAttribute, eav_vvalue, nil, true);
					VValueSingle* cv = nil;

					if (inAttribute->GetScriptKind() == script_db4d)
					{
						OptimizedQuery* script = inAttribute->GetScriptDB4D(context);
						if (script != nil)
						{
							cv = script->Compute(this, context, err);
						}
					}
					else
					{
						if (context->GetJSContext() != nil)
						{
							VJSContext jscontext(context->GetJSContext());
							VJSObject therecord(VJSEntitySelectionIterator::CreateInstance(jscontext, new EntitySelectionIterator(this, context)));
							VJSValue result(jscontext);
							VJSObject* objfunc = nil;
							VJSObject localObjFunc(jscontext);
							if (inAttribute->GetScriptObjFunc(script_attr_get, context->GetJSContext(), context, localObjFunc))
								objfunc = &localObjFunc;
							if (objfunc == nil)
								objfunc = context->getContextOwner()->GetJSFunction(inAttribute->GetScriptNum(script_attr_get), inAttribute->GetScriptStatement(script_attr_get), nil);
							
							//JSObjectSetProperty(*jscontext, therecord, name, funcref, kJSPropertyAttributeDontEnum, nil);
							XBOX::VJSException excep;
							if (objfunc != nil && therecord.CallFunction(*objfunc, nil, &result, &excep))
							{
								cv = result.CreateVValue(inAttribute->isSimpleDate());
							}

							if (!excep.IsEmpty())
							{
								ThrowJSExceptionAsError(jscontext, excep);
								err = ThrowError(VE_DB4D_JS_ERR);
							}

						}
					}

					if (cv != nil)
					{
						if (cv->GetValueKind() != inAttribute->ComputeScalarType())
						{
							VValueSingle* cv2 = cv->ConvertTo(inAttribute->ComputeScalarType());
							result->setVValue(cv2);
						}
						else
						{
							result->setVValue(cv);
						}
					}
					else
						result->setVValue(cv);

				}
				break;

			default:
				result = nil;
				break;

		}

		if (result != nil && result != (EntityAttributeValue*)-2 && result != (EntityAttributeValue*)-3 && result != (EntityAttributeValue*)-4 && result != (EntityAttributeValue*)-5)
		{
			if (mustCallEvent)
			{
				inAttribute->CallDBEvent(dbev_load, this, context, true);
			}

			result->AllowModifications(inAttribute->CanBeModified());
			fValues[pos-1] = result;
		}
	}


	return result;
}



VError LocalEntityRecord::do_setAttributeValue(const EntityAttribute* inAttribute, const VValueSingle* inValue)
{
	VError err = VE_OK;

	//BaseTaskInfo* context = ConvertContext(fContext);
	BaseTaskInfo* context = fContext;
	EntityAttributeValue* val = getAttributeValue(inAttribute, err, context);
	if (err == VE_OK && val != nil)
	{
		if (val->GetAttributeKind() == eav_vvalue)
		{
			if (val->CanBeModified())
			{
				if (inAttribute->GetKind() == eattr_computedField)
				{
					if (inAttribute->GetScriptKind() == script_javascript)
					{
						//BaseTaskInfo* context = ConvertContext(fContext);
						if (context->GetJSContext() != nil)
						{
							VJSContext jscontext(context->GetJSContext());
							VJSObject therecord(VJSEntitySelectionIterator::CreateInstance(jscontext, new EntitySelectionIterator(this, fContext)));
							VJSValue result(jscontext);
							VectorOfVString params;
							params.push_back(L"paramValue");

							VJSObject* objfunc = nil;
							VJSObject localObjFunc(jscontext);
							if (inAttribute->GetScriptObjFunc(script_attr_set, context->GetJSContext(), context, localObjFunc))
								objfunc = &localObjFunc;
							if (objfunc == nil)
								objfunc = context->getContextOwner()->GetJSFunction(inAttribute->GetScriptNum(script_attr_set), inAttribute->GetScriptStatement(script_attr_set), &params);

							//JSObjectSetProperty(*jscontext, therecord, name, funcref, kJSPropertyAttributeDontEnum, nil);
							if (objfunc != nil)
							{
								vector<VJSValue> paramvalues;
								VJSValue valjs(jscontext);
								if (inValue == nil)
									valjs.SetNull(nil);
								else
									valjs.SetVValue(*inValue, nil);
								paramvalues.push_back(valjs);

								XBOX::VJSException excep;
								if (therecord.CallFunction(*objfunc, &paramvalues, &result, &excep))
								{

								}

								if (!excep.IsEmpty())
								{
									ThrowJSExceptionAsError(jscontext, excep);
									err = ThrowError(VE_DB4D_JS_ERR);
								}
								else
									err = inAttribute->CallDBEvent(dbev_set, this, context, true);
							}
						}
					}

				}
				else
				{
					VValueSingle* cv = val->getVValue();
					if (cv != nil)
					{
						if (cv != inValue)
						{
							if (inValue == nil || inValue->IsNull())
								cv->SetNull(true);
							else
							{
								ValueKind existingType = cv->GetValueKind();
								ValueKind importingType = inValue->GetValueKind();
								if (importingType == VK_STRING || importingType == VK_TEXT)
								{
									if (existingType == VK_IMAGE || existingType == VK_BLOB || existingType == VK_BLOB_DB4D)
									{
										VString path;
										inValue->GetString(path);
										cv->SetOutsidePath(path);
										VString computedPath;
										cv->GetOutsidePath(computedPath);
										VFile xfile(fModel->GetFileSystemNamespace(), computedPath);
										if (xfile.Exists())
											cv->ReloadFromOutsidePath();
									}
									else if (existingType !=  VK_STRING && existingType != VK_TEXT)
									{
										if (((VString*)inValue)->GetLength() == 0)
											cv->SetNull(true);
										else
											cv->FromValue(*inValue);
									}
									else
										cv->FromValue(*inValue);
								}
								else if (importingType == VK_STRING && existingType == VK_IMAGE)
								{
									const VPicture* inPict = dynamic_cast<const VPicture*>(inValue);
									VPicture* cvpict = dynamic_cast<VPicture*>(cv);
									cvpict->FromVPicture_Retain(inPict, false, true);
								}
								else
									cv->FromValue(*inValue);
							}
						}
						val->Touch(context);
						//err = CallDBEvent(dbev_set, ConvertContext(fContext));
					}
					else
						err = inAttribute->ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_IS_NOT_A_VVALUE);
				}
			}
			else
				err = inAttribute->ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_IS_READ_ONLY);
		}
		else if (val->GetAttributeKind() == eav_subentity)
		{
			if (inValue == nil || inValue->IsNull())
				err = setAttributeValue(inAttribute, (EntityRecord*)nil);
			else
			{
				VectorOfVValue key(false);
				key.push_back(inValue);
				err = setAttributeValue(inAttribute, key);
			}
			ClearSubEntityCache(inAttribute->GetPathID());
		}
		else if (val->GetAttributeKind() == eav_composition)
		{
			if (inValue == nil || inValue->IsNull())
			{
				BaseTaskInfo* context = ConvertContext(fContext);
				if (context->GetJSContext() != nil)
				{
					VJSContext jscontext(context->GetJSContext());
					VJSArray jsemptyArray(jscontext);
					val->SetJSObject(jsemptyArray);
				}
				val->Touch(context);
				//err = CallDBEvent(dbev_set, context);
			}
		}
		else
			err = inAttribute->ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_IS_NOT_A_VVALUE);
	}

	return err;
}


void LocalEntityRecord::TouchAttribute(const EntityAttribute* att)
{
	Field* cri = ((db4dEntityAttribute*)att)->RetainField(true);
	if (cri != nil)
	{
		fMainRec->Touch(cri);
	}
	QuickReleaseRefCountable(cri);
}


bool LocalEntityRecord::ContainsBlobData(const EntityAttribute* inAttribute, BaseTaskInfo* context)
{
	bool result = false;

	EntityAttributeKind kind = inAttribute->GetKind();
	if (kind == eattr_storage)
	{
		FicheInMem* rec = fMainRec;
		if (rec != nil)
		{
			FicheOnDisk* assoc = rec->GetAssoc();
			if (assoc != nil)
			{
				void* p = assoc->GetDataPtr(inAttribute->GetFieldPos());
				if (p != nil)
				{
					if (*((sLONG*)p) != -1)
						result = true;
				}
			}
		}
	}
	else if (kind == eattr_alias)
	{
		VError err;
		SubEntityCache* cache = GetSubEntityCache(inAttribute->GetPathID(), err, true, inAttribute->GetSubEntityModel(), context);
		if (cache != nil)
		{
			EntityRecord* other = cache->GetErec();
			if (other != nil)
			{
				return other->ContainsBlobData(inAttribute->getFlattenAttribute(), context);
			}
		}
	}

	return result;
}



/*
VError LocalEntityRecord::Validate(BaseTaskInfo* context)
{
	VError err = VE_OK;
	bool validFail = false;

	for (EntityAttributeCollection::const_iterator cur = fModel->getAllAttributes().begin(), end = fModel->getAllAttributes().end(); cur != end; ++cur)
	{
		const EntityAttribute* att = *cur;
		if (att->NeedValidation())
		{
			EntityAttributeValue* val = getAttributeValue(att, err, context, false);
			if (err == VE_OK)
			{
				err = val->Validate(context);
			}
			if (err != VE_OK)
				validFail = true;
		}
	}


	if (!validFail)
	{
		err = CallDBEvent(dbev_validate, context);
	}

	if (validFail)
		err = ThrowError(VE_DB4D_ENTITY_RECORD_FAILS_VALIDATION);

	return err;
}
*/


VErrorDB4D LocalEntityRecord::Save(uLONG stamp, bool allowOverrideStamp, bool refreshOnly)
{
	return Save(nil, ConvertContext(fContext), stamp, allowOverrideStamp, refreshOnly);
}

/*
VError LocalEntityRecord::Validate()
{
	return Validate(ConvertContext(fContext));
}
*/


VError LocalEntityRecord::Save(Transaction* *trans, BaseTaskInfo* context, uLONG stamp, bool allowOverrideStamp, bool refreshOnly)
{
	VError err = VE_OK;
	LocalEntityModel* locmodel = (LocalEntityModel*)fModel;

	if (!fAlreadySaving)
	{
		fAlreadySaving = true;
		VUUID xGroupID;
		bool forced;
		bool isnew = false;
		if (fMainRec == nil || fMainRec->IsNew())
		{
			locmodel->GetPermission(DB4D_EM_Create_Perm, xGroupID, forced);
			isnew = true;
		}
		else
			locmodel->GetPermission(DB4D_EM_Update_Perm, xGroupID, forced);

		if (refreshOnly || okperm(context, xGroupID))
		{
			bool firstlevel = false;
			bool needfinalcommit = false;
			Transaction* localtrans = nil;
			Transaction* origtrans = context->GetCurrentTransaction();
			if (trans == nil)
			{
				firstlevel = true;
				trans = &localtrans;
			}

			err = Validate(context);

			if (err == VE_OK)
			{
				err = CallDBEvent(refreshOnly ? dbev_clientrefresh : dbev_save, context);
			}

			if (context != nil)
			{
				Transaction* curtrans = context->GetCurrentTransaction();
				if (curtrans != nil && curtrans != origtrans)
				{
					if (*trans != curtrans)
					{
						needfinalcommit = true;
						if (*trans == nil)
						{
							*trans = curtrans;
						}
						else
						{
							context->CommitTransaction();
						}
					}
				}
			}

			if (err == VE_OK)
			{
				for (EntityAttributeValueCollection::iterator cur = fValues.begin(), end = fValues.end(); cur != end && err == VE_OK; cur++)
				{
					EntityAttributeValue* val = *cur;
					if (val != nil)
					{
						err = val->Save(*trans, context, refreshOnly);

						if (context != nil)
						{
							Transaction* curtrans = context->GetCurrentTransaction();
							if (curtrans != nil && curtrans != origtrans)
							{
								if (*trans != curtrans)
								{
									needfinalcommit = true;
									if (*trans == nil)
									{
										*trans = curtrans;
									}
									else
									{
										context->CommitTransaction();
									}
								}
							}
						}
					}
				}
			}

			if (err == VE_OK)
			{
				if (!refreshOnly && locmodel->HasPrimKey() && !locmodel->MatchPrimKeyWithDataSource())
				{
					const IdentifyingAttributeCollection* primkey = locmodel->GetPrimaryAtts();
					VValueBag keyval;
					for (IdentifyingAttributeCollection::const_iterator cur = primkey->begin(), end = primkey->end(); cur != end && err == VE_OK; cur++)
					{
						EntityAttributeValue* val = getAttributeValue(cur->fAtt, err, context);
						if (err == VE_OK)
						{
							if (val == nil || val->getVValue() == nil || val->GetVValue()->IsNull())
							{
								if (GetNum() >= 0)
									err = ThrowError(VE_DB4D_PRIMKEY_IS_NULL);
								else
									err = ThrowError(VE_DB4D_PRIMKEY_IS_NULL_2);
							}
							else
							{
								keyval.SetAttribute(cur->fAtt->GetName(), val->getVValue()->Clone());
							}
						}
					}
					if (err == VE_OK)
					{
						sLONG otherrecid = locmodel->getEntityNumWithPrimKey(keyval, context, err);
						if (otherrecid != -1 && otherrecid != GetNum())
						{
							VString skey;
							bool first = true;
							for (IdentifyingAttributeCollection::const_iterator cur = primkey->begin(), end = primkey->end(); cur != end && err == VE_OK; cur++)
							{
								EntityAttributeValue* val = getAttributeValue(cur->fAtt, err, context);
								VString s;
								val->getVValue()->GetString(s);
								if (first)
									first = false;
								else
									skey += L";";
								skey += s;
							}
							err = ThrowError(VE_DB4D_PRIMKEY_IS_NOT_UNIQUE, &skey);
						}
					}
				}
				if (err == VE_OK && !refreshOnly)
					err = fMainRec->GetDF()->SaveRecord(fMainRec, context, stamp, allowOverrideStamp);
				if (err == VE_OK && !refreshOnly)
				{
					fStamp = 0;
					for (EntityAttributeValueCollection::iterator cur = fValues.begin(), end = fValues.end(); cur != end; cur++)
					{
						EntityAttributeValue* val = *cur;
						if (val != nil)
							val->UnTouch();
					}
				}
			}

			if (*trans != nil && firstlevel && needfinalcommit)
			{
				if (err == VE_OK /* && !refreshOnly*/)
					err = context->CommitTransaction();
				else
					context->RollBackTransaction();
			}
		}
		else
		{
			context->SetPermError();
			if (isnew)
				err = ThrowError(VE_DB4D_NO_PERM_TO_CREATE);
			else
				err = ThrowError(VE_DB4D_NO_PERM_TO_UPDATE);
		}
		fAlreadySaving = false;
	}

	if (err != VE_OK)
	{
		if (GetNum() >= 0)
			err = ThrowError(VE_DB4D_ENTITY_RECORD_CANNOT_BE_SAVED);
		else
			err = ThrowError(VE_DB4D_NEW_ENTITY_RECORD_CANNOT_BE_SAVED);
	}
	return err;
}


Boolean LocalEntityRecord::IsNew() const
{
	return fMainRec->IsNew();
}


Boolean LocalEntityRecord::IsProtected() const
{
	return fMainRec->ReadOnlyState();
}


Boolean LocalEntityRecord::IsModified() const
{
	return fStamp != 0;
}


VError LocalEntityRecord::do_Reload()
{
	VError err = VE_OK;
	if (!IsNew())
	{
		sLONG num = fMainRec->GetNum();
		if (num >= 0)
		{
			LocalEntityModel* model = dynamic_cast<LocalEntityModel*>(fModel);
			FicheInMem* newrec = model->GetMainTable()->GetDF()->LoadRecord(num, err, DB4D_Do_Not_Lock, fContext);
			if (newrec != nil)
			{
				CopyRefCountable(&fMainRec, newrec);
				fModificationStamp = fMainRec->GetModificationStamp();
				fStamp = 0;
				newrec->Release();
			}
		}
	}
	return err;
}


void LocalEntityRecord::GetTimeStamp(VTime& outValue)
{
	uLONG8 quand = fMainRec->GetAssoc()->GetTimeStamp();
	outValue.FromStamp(quand);
}


VErrorDB4D LocalEntityRecord::Drop()
{
	BaseTaskInfo* context = ConvertContext(fContext);
	VError err = VE_OK;
	if (!fAlreadyDeleting)
	{
		fAlreadyDeleting = true;
		if (okperm(context, fModel, DB4D_EM_Delete_Perm))
		{
			err = ValidateRemove(context);
			if (err == VE_OK)
				err = CallDBEvent(dbev_remove, context);
			if (err == VE_OK)
			{
				if (fModel->HasDeleteEvent(true))
					err = fModel->CallAttributesDBEvent(dbev_remove, this, context);
				if (err == VE_OK)
					err = fMainRec->GetDF()->DelRecord(fMainRec, context, fModificationStamp);
			}
		}
		else
		{
			err =  ThrowError(VE_DB4D_NO_PERM_TO_DELETE);
			context->SetPermError();
		}
		fAlreadyDeleting = false;
	}
	return err;
}


VError LocalEntityRecord::SetForeignKey(const EntityAttribute* att, EntityRecord* relatedEntity, const VValueSingle* inForeignKey)
{
	VError err = VE_OK;
	FicheInMem* rec = fMainRec;
	EntityRelation* rel = att->GetRelPath();
	if (rec != nil)
	{
		VValueSingle* cv = rec->GetFieldValue(rel->GetSourceAtt(), err, true);

		FicheInMem* relatedRec = nil;
		if (relatedEntity != nil)
		{
			LocalEntityRecord* localrec = dynamic_cast<LocalEntityRecord*>(relatedEntity);
			if (localrec != nil)
				relatedRec = localrec->getRecord();
		}
		if (relatedRec == nil)
		{
			if (inForeignKey != nil)
			{
				cv->FromValue(*inForeignKey);
			}
			else
			{
				cv->Clear();
				cv->SetNull(true);
			}
		}
		else
		{
			VValueSingle* relatedCV = relatedRec->GetFieldValue(rel->GetDestAtt(), err);
			if (relatedCV == nil)
			{
				cv->Clear();
				cv->SetNull(true);
			}
			else
			{
				cv->FromValue(*relatedCV);
			}
		}
		//rec->Touch(rel->GetSourceAtt());

	}
	return err;
}


bool LocalEntityRecord::do_GetPrimKeyValueAsObject(VJSObject& outObj)
{
	const IdentifyingAttributeCollection* primatts = fModel->GetPrimaryAtts();
	if (primatts->empty())
	{
		outObj.MakeEmpty();
		RecIDType numrec = GetNum();
		if (numrec >= 0)
		{
			VJSValue jsval(outObj.GetContext());
			jsval.SetNumber(numrec);
			outObj.SetProperty("__RECID", jsval, JS4D::PropertyAttributeNone);
		}
		VJSValue jsval2(outObj.GetContext());
		sLONG stamp = GetModificationStamp();
		jsval2.SetNumber(stamp);
		outObj.SetProperty("__STAMP", jsval2, JS4D::PropertyAttributeNone);
		return true;
	}
	else
		return false;
}

bool LocalEntityRecord::do_GetPrimKeyValueAsBag(VValueBag& outBagKey)
{
	const IdentifyingAttributeCollection* primatts = fModel->GetPrimaryAtts();
	if (primatts->empty())
	{
		RecIDType numrec = GetNum();
		if (numrec >= 0)
		{
			outBagKey.SetLong8("__RECID", numrec);
		}
		return true;
	}
	else
		return false;
}

bool LocalEntityRecord::do_GetPrimKeyValueAsString(VString& outKey, bool autoQuotes)
{
	const IdentifyingAttributeCollection* primatts = fModel->GetPrimaryAtts();
	if (primatts->empty())
	{
		RecIDType numrec = GetNum();
		if (numrec >= 0)
		{
			outKey.FromLong8(numrec);
		}
		else
			outKey.Clear();
		return true;
	}
	else
		return false;
}

bool LocalEntityRecord::do_GetPrimKeyValueAsVValues(VectorOfVValue& outPrimkey)
{
	const IdentifyingAttributeCollection* primatts = fModel->GetPrimaryAtts();
	if (primatts->empty())
	{
		RecIDType numrec = GetNum();
		if (numrec >= 0)
		{
			fPseudoKey.FromLong8(numrec);
			outPrimkey.push_back(&fPseudoKey);

		}
		return true;
	}
	else
		return false;
}



VError LocalEntityRecord::DuplicateInto(EntityRecord* otherRec)
{
	VError err = EntityRecord::DuplicateInto(otherRec);
	LocalEntityRecord* localOther = dynamic_cast<LocalEntityRecord*>(otherRec);
	
	if (localOther != nil)
	{
		localOther->fMainRec = RetainRefCountable(fMainRec);
		localOther->fModificationStamp = fModificationStamp;
		localOther->fStamp = fStamp;
		localOther->fPseudoKey = fPseudoKey;
	}

	return err;
}




			// --------------------------------------------------------------------------------------


void LocalEntityModelCatalog::GetName(VString& outName) const
{
	fOwner->GetName(outName);
}


EntityModel* LocalEntityModelCatalog::NewModel()
{
	return new LocalEntityModel(this, nil);
}


VError LocalEntityModelCatalog::do_LoadEntityModels(const VValueBag& bagEntities, ModelErrorReporter* errorReporter, unordered_map_VString<VRefPtr<VFile> >* outIncludedFiles, BaseTaskInfo* context)
{
	VError err = VE_OK;

	return err;
}


VError LocalEntityModelCatalog::do_SecondPassLoadEntityModels(ModelErrorReporter* errorReporter, bool dataWasAlreadyThere)
{
	Base4D* db = GetDB();
	VError err = VE_OK;
	if (!dataWasAlreadyThere)
		err = db->BuildTablesFromDataTables();

	VTaskLock lock(&fEntityModelMutex);
	for (EntityModelMap::iterator cur = fEntityModels.begin(), end = fEntityModels.end(); cur != end && err == VE_OK; cur++)
	{
		LocalEntityModel* model = dynamic_cast<LocalEntityModel*>(cur->second);
		if (testAssert(model != nil))
		{
			err = model->MatchWithTable();
		}
	}

	for (EntityModelMap::iterator cur = fEntityModels.begin(), end = fEntityModels.end(); cur != end && err == VE_OK; cur++)
	{
		LocalEntityModel* model = dynamic_cast<LocalEntityModel*>(cur->second);
		if (testAssert(model != nil))
		{
			err = model->MatchExtendedTable();
		}
	}

	for (EntityModelMap::iterator cur = fEntityModels.begin(), end = fEntityModels.end(); cur != end && err == VE_OK; cur++)
	{
		LocalEntityModel* model = dynamic_cast<LocalEntityModel*>(cur->second);
		if (testAssert(model != nil))
		{
			err = model->BuildIndexes(dataWasAlreadyThere);
		}
	}

	db->SaveSmallIDsInRec();

	return err;
}


VError LocalEntityModelCatalog::BuildLocalEntityModelRelations()
{
	VError err = VE_OK;
	VTaskLock lock(&fEntityModelMutex);
	for (EntityModelMap::iterator cur = fEntityModels.begin(), end = fEntityModels.end(); cur != end && err == VE_OK; cur++)
	{
		LocalEntityModel* model = dynamic_cast<LocalEntityModel*>(cur->second);
		if (testAssert(model != nil))
		{
			err = model->BuildLocalEntityModelRelations(this);
		}
	}
	return err;
}


VError LocalEntityModelCatalog::SetLogFile(VFile* logfile, bool append, VJSONObject* options)
{
	VError err = EntityModelCatalog::SetLogFile(logfile, append, options);
	EntityModelCatalogCollection outsideCats;
	fOwner->GetOutsideCatalogs(outsideCats);
	for (EntityModelCatalogCollection::iterator cur = outsideCats.begin(), end = outsideCats.end(); cur != end; ++cur)
	{
		EntityModelCatalog* cat = *cur;
		cat->AcceptLogger(fLogger);
		cat->Release();
	}
	return err;
}


EntityModel* LocalEntityModelCatalog::RetainEntityModelByTableNum(sLONG tablenum)
{
	EntityModel* result = nil;
	for (EntityModelMap::iterator cur = fEntityModels.begin(), end = fEntityModels.end(); cur != end && result == nil; ++cur)
	{
		LocalEntityModel* em = (LocalEntityModel*)(cur->second);
		if (em->GetMainTable() != nil && em->GetMainTable()->GetNum() == tablenum)
			result = RetainRefCountable(cur->second);
	}

	return result;
}




