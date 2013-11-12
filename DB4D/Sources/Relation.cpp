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



FieldNuplet::FieldNuplet(sLONG maxfield, uBOOL rien)
{
	sLONG i;
	nbfield = maxfield;
	fields = (FieldDef*)gCppMem->NewPtr(nbfield*sizeof(FieldDef), false, 'fnup');
	fieldarr.SetAllocatedSize(0);
	fieldarr.AddNSpaces(nbfield, true);
	/*
	for (i=0; i<nbfield; i++)
	{
		fieldarr[i].crit = nil;
		fieldarr[i].fic = nil;
	}
	*/
}


FieldNuplet::FieldNuplet(FieldNuplet* from)
{
	nbfield = 0;
	fields = nil;
	CopyFrom(from);
}


FieldNuplet::~FieldNuplet()
{
	sLONG i;
	for (i=0; i<nbfield; i++)
	{
		FieldRef ref = fieldarr[i];
		if (ref.crit != nil) ref.crit->Release();
		if (ref.fic != nil) ref.fic->Release();
	}
	if (fields)
		gCppMem->DisposePtr((void *) fields);
}


#if autocheckobj
uBOOL FieldNuplet::CheckObjInMem(void)
{
	sLONG i;
	
	if (fields != nil)
	{
		CheckAssert(IsValidPtr(fields));
		CheckAssert(nbfield>=0);
		for (i = 0; i<nbfield; i++)
		{
			CheckAssert(IsFieldValid(fields[i].numfile, fields[i].numfield, TheBase));
		}
		
	}
	return(true);
}

#endif


void FieldNuplet::CopyFrom(FieldNuplet* from)
{
	sLONG i;
	FieldDef *p1,*p2;

	if (fields !=nil)
	{
		gCppMem->DisposePtr((tPtr)fields);
	}
	nbfield = from->nbfield;
	fields = (FieldDef*)gCppMem->NewPtr(nbfield*sizeof(FieldDef), false, 'fnup');
	
	p1=from->fields;
	p2=fields;
	for (i=nbfield; i>0; --i)
	{
		*p2++ = *p1++;
	}
	fieldarr.CopyFrom(from->fieldarr);
	for (i=0; i<nbfield; i++)
	{
		FieldRef ref = fieldarr[i];
		if (ref.crit != nil) ref.crit->Retain();
		if (ref.fic != nil) ref.fic->Retain();
	}
}


void FieldNuplet::SetNthField(sLONG n, Field* cri)
{
	assert(n<=nbfield );
	
	if (cri == nil)
	{
		fieldarr[n-1].fic = nil;
		fieldarr[n-1].crit = nil;
		fields[n-1].numfile = 0;
		fields[n-1].numfield = 0;
	}
	else
	{
		fieldarr[n-1].fic = cri->GetOwner();
		fieldarr[n-1].crit = cri;
		cri->GetOwner()->Retain();
		cri->Retain();
		fields[n-1].numfile = cri->GetOwner()->GetNum();
		fields[n-1].numfield = cri->GetPosInRec();
	}
}

		
VError FieldNuplet::UpdateCritFic(Base4D *bd)
{
	FieldDef *p1;
	sLONG i;
	Table *fic;
	Field *crit;
	VError err = VE_OK;
	
	for (i=0; i<nbfield; i++)
	{
		FieldRef ref = fieldarr[i];
		if (ref.crit != nil) ref.crit->Release();
		if (ref.fic != nil) ref.fic->Release();
	}

	fieldarr.SetAllocatedSize(0);
	fieldarr.AddNSpaces(nbfield, false);
	
	p1=fields;
	for (i=0;i<nbfield;i++)
	{
		crit = nil;
		fic=bd->RetainTable( p1->numfile);
		if (fic == nil)
			err = VE_DB4D_WRONGTABLEREF;
		else
		{
			crit=fic->RetainField(p1->numfield);
			if (crit == nil)
				err = VE_DB4D_WRONGFIELDREF;
		}

		fieldarr[i].fic = fic;
		fieldarr[i].crit = crit;
		p1++;
	}

	return err;
}


uBOOL FieldNuplet::Match(const FieldNuplet* other) const
{
	const FieldDef *p1,*p2;
	sLONG i;
	
	if (nbfield!=other->nbfield)
	{
		return(false);
	}
	else
	{
		p1=fields;
		p2=other->fields;
		for (i=0;i<nbfield;i++)
		{
			if ( (p1->numfile != p2->numfile) || (p1->numfield != p2->numfield) )
			{
				return(false);
			}
			p1++;
			p2++;
		}
	}
	
	return(true);
}


uBOOL FieldNuplet::Match(const NumFieldArray& xother, sLONG numTable) const
{
	const FieldDef *p1;
	const sLONG *p2;
	sLONG i;

	if (nbfield!=xother.GetCount())
	{
		return(false);
	}
	else
	{
		p1=fields;
		p2=xother.First();
		for (i=0;i<nbfield;i++)
		{
			if ( (p1->numfile != numTable) || (p1->numfield != *p2) )
			{
				return(false);
			}
			p1++;
			p2++;
		}
	}

	return(true);
}


void FieldDef::SwapBytes()
{
	ByteSwap(&numfield);
	ByteSwap(&numfile);
}


void FieldNuplet::SwapBytes()
{
	FieldDef *p = fields, *end = fields + nbfield;

	while (p != end)
	{
		p->SwapBytes();
		p++;
	}

}


VError FieldNuplet::PutInto( VStream &inStream)
{
	VError error = VE_OK;

	error = inStream.PutLong( nbfield);
	if (error == VE_OK && nbfield > 0)
		error = inStream.PutData( fields, nbfield*sizeof(FieldDef));
	
	return error;
}


FieldNuplet* FieldNuplet::CreateFrom( VStream &inStream, Base4D *inBase, VError &outError)
{
	sLONG fieldCount = 0;
	FieldNuplet *result = NULL;

	outError = inStream.GetLong( fieldCount);
	if (outError == VE_OK)
	{
		result = new FieldNuplet( fieldCount, true);
		if (result == NULL)
		{
			outError = VE_MEMORY_FULL;
		}
		else if ((fieldCount > 0) && (result->fields != NULL))
		{
			outError = inStream.GetData( result->fields, fieldCount*sizeof(FieldDef));
			if (outError == VE_OK)
			{
				if (inStream.NeedSwap())
					result->SwapBytes();
				outError = result->UpdateCritFic( inBase);
			}
		}
	}
	return result;
}


		/* ************************************************************************************************ */


Relation::Relation(const VString* name, const VString* nameopposite, Field* source, Field* dest, sLONG inPosInList):
	fExtra(&fExtraAddr, &fExtraLen, nil, -1, -1, DBOH_ExtraRelationProperties, false)
{
	fState = RelationState_FromSource;
	fAutoLoadNto1 = false;
	fAutoLoad1toN = false;
	fID.Regenerate();
	bd = nil;
	fExtraLen = 0;
	fExtraAddr = 0;
	fCanSave = true;

	fSource = source;
	if (fSource != nil)
	{
		fSource->Retain();
		bd = fSource->GetOwner()->GetOwner();
		fExtra.SetDB(bd->GetStructure());
		fSource->Retain();
		fSources.Add(fSource);
	}

	fDestination = dest;
	if (fDestination != nil)
	{
		fDestination->Retain();
		fDestination->Retain();
		fDestinations.Add(fDestination);
	}

	fName = *name;
	fOppositeName = *nameopposite;
	fIsValid = true;
	fRelVar1toN = nil;
	fRelVarNto1 = nil;
	fWithReferentialIntegrity = false;
	fAutoDelete = false;
	fIsForSubTable = false;
	fIsForeignKey = false;
	if (fSource != nil && fDestination != nil)
	{
//		if (fSource->GetTyp() == DB4D_SubTable && fDestination->GetTyp() == DB4D_SubTableKey && fType == Relation_1ToN)
		if (fDestination->GetTyp() == DB4D_SubTable && fSource->GetTyp() == DB4D_SubTableKey)
		{
			fIsForSubTable = true;
		}
	}

	fStamp = 10;
	fPosInBase = inPosInList;
	fIsRemote = false;
}


Relation::~Relation()
{
	//UnRegisterForLang();
	if (fSource != nil)
	{
		assert(bd != nil);
		fSource->Release();
	}

	if (fDestination != nil)
	{
		assert(bd != nil);
		fDestination->Release();
	}

	for (FieldArray::Iterator cur = fSources.First(), end = fSources.End(); cur != end; cur++)
	{
		if (*cur != nil)
			(*cur)->Release();
	}
	for (FieldArray::Iterator cur = fDestinations.First(), end = fDestinations.End(); cur != end; cur++)
	{
		if (*cur != nil)
			(*cur)->Release();
	}
}


VError Relation::ExtendRelationFields(const CDB4DFieldArray& inSourceFields, const CDB4DFieldArray& inDestinationFields)
{
	assert(inSourceFields.GetCount() == inDestinationFields.GetCount() && inSourceFields.GetCount() > 1);

	VError err = VE_OK;
	for (CDB4DFieldArray::ConstIterator cur = inSourceFields.First()+1, end = inSourceFields.End(), curdest = inDestinationFields.First()+1;
				cur != end; cur++, curdest++)
	{
		if (*cur != nil && *curdest != nil)
		{
			Field* source = dynamic_cast<VDB4DField*>(*cur)->GetField();
			Field* dest = dynamic_cast<VDB4DField*>(*curdest)->GetField();
			if (source->GetTyp() != dest->GetTyp())
				err = VE_DB4D_FIELDTYPENOTMATCHING;
			else
			{
				source->Retain();
				fSources.Add(source);
				dest->Retain();
				fDestinations.Add(dest);
			}
		}
		else
			err = VE_DB4D_FIELDTYPENOTMATCHING;

		if (err != VE_OK)
			break;
	}
	return err;
}

VError Relation::ActivateManyToOne(FicheInMem* rec, FicheInMem* &result, BaseTaskInfo* context, Boolean OldOne, Boolean NoCache, DB4D_Way_of_Locking HowToLock)
{
	ValPtr cv;
	VError err = VE_OK;

	result = nil;

	if (rec == nil)
	{
		err = ThrowError(VE_DB4D_RECORDISEMPTY, DBaction_ActivatingManyToOneRelation);
	}
	else
	{
		if (fSource == nil || fDestination == nil || fSources.GetCount() != fDestinations.GetCount())
		{
			err = ThrowError(VE_DB4D_WRONGFIELDREF, DBaction_ActivatingManyToOneRelation);
		}
		else
		{
			if (!NoCache)
				result = rec->RetainCachedRelatedRecord(this, OldOne);
			if (result == nil)
			{
				Table* t = fDestination->GetOwner();
				SearchTab rech(t, true);

				for (FieldArray::ConstIterator cur = fSources.First(), end = fSources.End(), curdest = fDestinations.First(); cur != end; cur++, curdest++)
				{
					Field* source = *cur;
					Field* dest = *curdest;
					if (OldOne)
						cv = rec->GetFieldOldValue(source, err, true);
					else
						cv = rec->GetFieldValue(source, err, false, true);
					if (cv != nil)
					{
						if (cur != fSources.First())
							rech.AddSearchLineBoolOper(DB4D_And);
						rech.AddSearchLineSimple(dest, DB4D_Equal, cv);
					}
					if (err != VE_OK)
						break;
				}

				if (err == VE_OK)
				{
					OptimizedQuery query;
					err = query.AnalyseSearch(&rech, context);
					if (err == VE_OK)
					{
						Selection *sel = query.Perform((Bittab*)nil, nil, context, err, DB4D_Do_Not_Lock);
						if (sel != nil)
						{
							if (sel->GetQTfic()>0)
							{
								sLONG numrec = sel->GetFic(0);
								DataTable* df = t->GetDF();
								if (df != nil)
								{
									result = df->LoadRecord(numrec, err, HowToLock, context, true);
									if (result != nil && !NoCache)
									{
										rec->SetCachedRelatedRecord(this, result, OldOne);
									}
								}
							}
							sel->Release();
						}
					}
				}

			}
		}
	}

	return err;
}


VError Relation::ActivateManyToOneS(FicheInMem* rec, Selection* &result, BaseTaskInfo* context, Boolean OldOne, DB4D_Way_of_Locking HowToLock)
{
	ValPtr cv;
	VError err = VE_OK;

	result = nil;

	if (rec == nil)
	{
		err = ThrowError(VE_DB4D_RECORDISEMPTY, DBaction_ActivatingManyToOneRelation);
	}
	else
	{
		if (fSource == nil || fDestination == nil || fSources.GetCount() != fDestinations.GetCount())
		{
			err = ThrowError(VE_DB4D_WRONGFIELDREF, DBaction_ActivatingManyToOneRelation);
		}
		else
		{
			Table* t = fDestination->GetOwner();
			SearchTab rech(t, true);

			for (FieldArray::ConstIterator cur = fSources.First(), end = fSources.End(), curdest = fDestinations.First(); cur != end; cur++, curdest++)
			{
				Field* source = *cur;
				Field* dest = *curdest;

				if (OldOne)
					cv = rec->GetFieldOldValue(source, err, true);
				else
					cv = rec->GetFieldValue(source, err, false, true);
				if (cv != nil)
				{
					if (cur != fSources.First())
						rech.AddSearchLineBoolOper(DB4D_And);
					rech.AddSearchLineSimple(dest, DB4D_Like, cv);
				}

				if (err != VE_OK)
					break;
			}

			if (err == VE_OK)
			{
				OptimizedQuery query;
				err = query.AnalyseSearch(&rech, context);
				if (err == VE_OK)
				{
					result = query.Perform((Bittab*)nil, nil, context, err, HowToLock);
				}
			}
		}
	}

	return err;
}


VError Relation::ActivateOneToMany(FicheInMem* rec, Selection* &result, BaseTaskInfo* context, Boolean OldOne, Boolean NoCache, DB4D_Way_of_Locking HowToLock)
{
	ValPtr cv;
	VError err = VE_OK;

	result = nil;

	if (rec == nil)
	{
		err = ThrowError(VE_DB4D_RECORDISEMPTY, DBaction_ActivatingManyToOneRelation);
	}
	else
	{
		if (fSource == nil || fDestination == nil || fSources.GetCount() != fDestinations.GetCount())
		{
			err = ThrowError(VE_DB4D_WRONGFIELDREF, DBaction_ActivatingManyToOneRelation);
		}
		else
		{
			if (!NoCache)
				result = rec->RetainCachedRelatedSelection(this, OldOne);
			if (result == nil)
			{
				Table* t = fSource->GetOwner();
				SearchTab rech(t, true);

				for (FieldArray::ConstIterator cur = fDestinations.First(), end = fDestinations.End(), curdest = fSources.First(); cur != end; cur++, curdest++)
				{
					Field* source = *cur;
					Field* dest = *curdest;

					if (OldOne)
						cv = rec->GetFieldOldValue(source, err, true);
					else
						cv = rec->GetFieldValue(source, err, false, true);
					if (cv != nil)
					{
						if (cur != fDestinations.First())
							rech.AddSearchLineBoolOper(DB4D_And);
						rech.AddSearchLineSimple(dest, DB4D_Equal, cv);
					}

					if (err != VE_OK)
						break;
				}

				if (err == VE_OK)
				{
					OptimizedQuery query;
					err = query.AnalyseSearch(&rech, context);
					if (err == VE_OK)
					{
						result = query.Perform((Bittab*)nil, nil, context, err, HowToLock);
						if (result != nil && !NoCache)
						{
							DataTable* df = t->GetDF();
							if (df != nil)
							{
								rec->SetCachedRelatedSelection(this, result, OldOne);
							}
							else
							{
								result->Release();
								result = nil;
							}
						}
					}
				}

			}
		}
	}

	return err;
}


/*
VError Relation::ActivateDestToSource(FicheInMem* rec, Selection* &result, BaseTaskInfo* context, Boolean OldOne, DB4D_Way_of_Locking HowToLock)
{
	return ActivateOneToMany(rec, result, context, OldOne, true, HowToLock);
	
	ValPtr cv;
	VError err = VE_OK;

	result = nil;

	if (rec == nil)
	{
		err = ThrowError(VE_DB4D_RECORDISEMPTY, DBaction_ActivatingManyToOneRelation);
	}
	else
	{
		if (fSource == nil || fDestination == nil || fSources.GetCount() != fDestinations.GetCount())
		{
			err = ThrowError(VE_DB4D_WRONGFIELDREF, DBaction_ActivatingManyToOneRelation);
		}
		else
		{
			if (result == nil)
			{
				Table* t = fSource->GetOwner();
				SearchTab rech(t, true);

				for (FieldArray::ConstIterator cur = fDestinations.First(), end = fDestinations.End(), curdest = fSources.First(); cur != end; cur++, curdest++)
				{
					Field* source = *cur;
					Field* dest = *curdest;

					if (OldOne)
						cv = rec->GetFieldOldValue(source, err, true);
					else
						cv = rec->GetFieldValue(source, err, false, true);
					if (cv != nil)
					{
						if (cur != fDestinations.First())
							rech.AddSearchLineBoolOper(DB4D_And);
						rech.AddSearchLineSimple(dest, DB4D_Equal, cv);
					}

					if (err != VE_OK)
						break;
				}

				if (err == VE_OK)
				{
					OptimizedQuery query;
					err = query.AnalyseSearch(&rech, context);
					if (err == VE_OK)
					{
						result = query.Perform((Bittab*)nil, nil, context, err, Locking);
					}
				}

			}
		}
	}

	return err;
	
}
*/

void Relation::RegisterForLang()
{
#if 0
	CDB4D_Lang* lang4D = VDBMgr::GetLang4D();

	if (fSource != nil && fDestination != nil)
	{
		Table* tab = fSource->GetOwner();
		Table* desttab = fDestination->GetOwner();
		if ( (lang4D != nil) && (tab->GetRecordRef() != nil) )
		{
			if (fRelVarNto1 == nil)
			{
				if (!fName.IsEmpty())
				{
					fRelVarNto1 = tab->GetRecordRef()->AddVariable( fName, fID, eVK_Class, desttab->GetRecordRef(), NULL, -2 );
					if (fRelVarNto1 != nil) 
						fRelVarNto1->SetExpandable(false);
				}
			}
			else
			{
				if (fName.IsEmpty())
				{
					fRelVarNto1->Drop();
					fRelVarNto1->Release();
					fRelVarNto1 = nil;
				}
				else
					fRelVarNto1->SetName(fName);
			}

			if (fRelVar1toN == nil)
			{
				if (!fOppositeName.IsEmpty())
				{
					VUUID xUID;
					VUUIDBuffer buf;
					fID.ToBuffer(buf);
					buf.IncFirst8();
					xUID.FromBuffer(buf);
					
					fRelVar1toN = desttab->GetRecordRef()->AddVariable(fOppositeName, xUID, eVK_Class, tab->GetSelectionRef(), NULL, -3 );
					if (fRelVar1toN != nil) 
						fRelVar1toN->SetExpandable(false);
				}
			}
			else
			{
				if (fOppositeName.IsEmpty())
				{
					fRelVar1toN->Drop();
					fRelVar1toN->Release();
					fRelVar1toN = nil;
				}
				else
					fRelVar1toN->SetName(fOppositeName);
			}

		}
		
	}
#endif
}


void Relation::UnRegisterForLang()
{
#if 0
	CDB4D_Lang* lang4D = VDBMgr::GetLang4D();
	if ( (lang4D != NULL) && (fRelVarNto1 != NULL))
	{
		fRelVarNto1->Drop();
		fRelVarNto1->Release();
		fRelVarNto1 = nil;
	}
	if ( (lang4D != NULL) && (fRelVar1toN != NULL))
	{
		fRelVar1toN->Drop();
		fRelVar1toN->Release();
		fRelVar1toN = nil;
	}
#endif
}


void RelOnDisk::SwapBytes()
{
	//ByteSwap(&type);
	ID.SwapBytes();
	SourceField.SwapBytes();
	DestField.SwapBytes();
	ByteSwap(&ExtraAddr);
	ByteSwap(&ExtraLen);
}


void oldRelOnDisk::SwapBytes()
{
	ByteSwap(&type);
	ID.SwapBytes();
	SourceField.SwapBytes();
	DestField.SwapBytes();
	ByteSwap(&ExtraAddr);
	ByteSwap(&ExtraLen);
}


Boolean Relation::IsAutoLoadNto1(BaseTaskInfo* context) const
{
	if (fIsForSubTable)
		return false;

	if (context == nil)
	{
		return fAutoLoadNto1;
	}
	else
	{
		Boolean unknown;
		Boolean res = context->IsRelationAutoLoadNto1(this, unknown);
		/*
		if (unknown)
			res = fAutoLoadNto1;
		*/
		return res;
	}
}


Boolean Relation::IsAutoLoad1toN(BaseTaskInfo* context) const
{
	if (fIsForSubTable)
		return false;

	if (context == nil)
	{
		return fAutoLoad1toN;
	}
	else
	{
		Boolean unknown;
		Boolean res = context->IsRelationAutoLoad1toN(this, unknown);
		/*
		if (unknown)
		res = fAutoLoad1toN;
		*/
		return res;
	}
}


VError Relation::SetAutoNto1Load(Boolean state, CDB4DBaseContext* inContext)
{
	VError err = VE_OK;
	
	if (fIsRemote)
	{
		VValueBag bag;
		DB4DRemoteBagKeys::rm_auto_load_Nto1.Set( &bag, state);
		err = SendReqSetAttributes( bag, inContext);
		if (err == VE_OK)
		{
			fAutoLoadNto1 = state;
		}
	}
	else
	{
		fAutoLoadNto1 = state;
		Touch();
	}
	return err;
}


VError Relation::SetAuto1toNLoad(Boolean state, CDB4DBaseContext* inContext)
{
	VError err = VE_OK;
	
	if (fIsRemote)
	{
		VValueBag bag;
		DB4DRemoteBagKeys::rm_auto_load_1toN.Set( &bag, state);
		err = SendReqSetAttributes( bag, inContext);
		if (err == VE_OK)
			fAutoLoad1toN = state;
	}
	else
	{
		fAutoLoad1toN = state;
		Touch();
	}
	return err;
}


VError Relation::SetReferentialIntegrity(Boolean ReferentialIntegrity, Boolean AutoDeleteRelatedRecords, CDB4DBaseContext* inContext)
{
	VError err = VE_OK;
	
	if (fIsRemote)
	{
		VValueBag bag;
		if (ReferentialIntegrity)
		{
			if (AutoDeleteRelatedRecords)
				DB4DRemoteBagKeys::rm_integrity.Set( &bag,CVSTR( "delete"));
			else
				DB4DRemoteBagKeys::rm_integrity.Set( &bag,CVSTR( "reject"));
		}
		else
		{
			DB4DRemoteBagKeys::rm_integrity.Set( &bag,CVSTR( "none"));
		}
		err = SendReqSetAttributes( bag, inContext);
		if (err == VE_OK)
		{
			fWithReferentialIntegrity = ReferentialIntegrity;
			fAutoDelete = AutoDeleteRelatedRecords;
			RecalcDepIntegrity();
		}
	}
	else
	{
		fWithReferentialIntegrity = ReferentialIntegrity;
		fAutoDelete = AutoDeleteRelatedRecords;
		RecalcDepIntegrity();
		Touch();
	}
	return err;
}


VError Relation::SetForeignKey(Boolean on, CDB4DBaseContext* inContext)
{
	VError err = VE_OK;
	
	if (fIsRemote)
	{
		VValueBag bag;
		DB4DRemoteBagKeys::rm_foreign_key.Set( &bag, true);
		err = SendReqSetAttributes( bag, inContext);
		if (err == VE_OK)
		{
			fIsForeignKey = true;
			fWithReferentialIntegrity = true;
			fAutoDelete = false;
			RecalcDepIntegrity();
		}
	}
	else
	{
		fIsForeignKey = true;
		fWithReferentialIntegrity = true;
		fAutoDelete = false;
		RecalcDepIntegrity();
		Touch();
	}
	return err;
}


VError Relation::_LoadAttributesFromBag( const VValueBag &inBag)
{
	VError err = VE_OK;	
	
	// Get Name
	inBag.GetString( DB4DBagKeys::name_Nto1, fName);
	if (!IsValid4DName( fName))
		err = VE_DB4D_INVALIDRELATIONNAME;
	
	// Get Opposite Name
	if (err == VE_OK)
	{
		inBag.GetString( DB4DBagKeys::name_1toN, fOppositeName);
		if (!IsValid4DName( fOppositeName))
			err = VE_DB4D_INVALIDRELATIONNAME;
	}
	
	if (err == VE_OK)
	{
		// Get ReferentialIntegrity
		VString integrity;
		if (inBag.GetString( DB4DBagKeys::integrity, integrity) && !integrity.IsEmpty() && !integrity.EqualToString( CVSTR( "none")) )
		{
			if (integrity == CVSTR("reject"))
			{
				fWithReferentialIntegrity = true;
				fAutoDelete = false;
			}
			else if (testAssert( integrity == CVSTR("delete")))
			{
				fWithReferentialIntegrity = true;
				fAutoDelete = true;
			}
		}
		else
		{
			fWithReferentialIntegrity = false;
			fAutoDelete = false;
		}

		fAutoLoadNto1 = DB4DBagKeys::auto_load_Nto1( &inBag);
		fAutoLoad1toN = DB4DBagKeys::auto_load_1toN( &inBag);
		fIsForeignKey = DB4DBagKeys::foreign_key( &inBag);

		// Get State
		sLONG val = 0;
		inBag.GetLong( DB4DBagKeys::state, val);
		if (val != RelationState_FromSource && val != RelationState_ReadOnly && val != RelationState_ReadWrite)
		{
			err = VE_INVALID_PARAMETER;
		}
		else
		{
			fState = (DB4D_RelationState) val;
		}
	}
	return err;
}


VError Relation::_SaveAttributesToBag( VValueBag &ioBag) const
{
	VError err = VE_OK;
	
	ioBag.SetString( DB4DBagKeys::name_Nto1, fName);
	ioBag.SetString( DB4DBagKeys::name_1toN, fOppositeName);
	ioBag.SetBoolean( DB4DBagKeys::auto_load_Nto1, fAutoLoadNto1);
	ioBag.SetBoolean( DB4DBagKeys::auto_load_1toN, fAutoLoad1toN);
	ioBag.SetBoolean( DB4DBagKeys::foreign_key, fIsForeignKey);
	ioBag.SetLong( DB4DBagKeys::state, fState);
	
	if (fWithReferentialIntegrity)
	{
		if (fAutoDelete)
			ioBag.SetString( DB4DBagKeys::integrity, CVSTR( "delete"));
		else
			ioBag.SetString( DB4DBagKeys::integrity, CVSTR( "reject"));
	}
	return err;
}


VError Relation::PutInto(VStream& buf, const Base4D* bd) const
{
	VError err = VE_OK;
	RelOnDisk dd;

	//dd.type = (sLONG)fType;
	fID.ToBuffer(dd.ID);
	VUUID xid;

	if (fSource == nil)
		_raz(&dd.SourceField,sizeof(dd.SourceField));
	else
	{
		fSource->GetUUID(xid);
		xid.ToBuffer(dd.SourceField);
	}

	if (fDestination == nil)
		_raz(&dd.DestField,sizeof(dd.DestField));
	else
	{
		fDestination->GetUUID(xid);
		xid.ToBuffer(dd.DestField);
	}

	if (fAutoLoadNto1)
		dd.RelNto1IsAuto = 1;
	else
		dd.RelNto1IsAuto = 0;

	if (fAutoLoad1toN)
		dd.Rel1toNIsAuto = 1;
	else
		dd.Rel1toNIsAuto = 0;

	/*if (fCheckRefIntOnModify)
		dd.CheckRefIntOnModify = 1;
	else*/
		dd.CheckRefIntOnModify = 0;

	if (fWithReferentialIntegrity)
		dd.WithRefInt = 1;
	else
		dd.WithRefInt = 0;

	if (fAutoDelete)
		dd.AutoDel = 1;
	else
		dd.AutoDel = 0;

	if (fIsForeignKey)
		dd.ForeignKey = 1;
	else
		dd.ForeignKey = 0;

	if (fSources.GetCount() > 1 || fDestinations.GetCount() > 1)
		dd.MultipleFields = 1;
	else
		dd.MultipleFields = 0;

	dd.RelState = (uCHAR)fState;

	dd.ExtraLen = fExtraLen;
	dd.ExtraAddr = fExtraAddr;

	err = buf.PutData(&dd, sizeof(dd));
	if (err == VE_OK)
		err = fName.WriteToStream(&buf);
	if (err == VE_OK)
		err = fOppositeName.WriteToStream(&buf);
	if (err == VE_OK && (dd.MultipleFields == 1))
	{
		sLONG nbfields = fSources.GetCount();
		err = buf.PutLong(nbfields);
		if (err == VE_OK)
		{
			for (FieldArray::ConstIterator cur = fSources.First()+1, end = fSources.End(); cur != end; cur++)
			{
				VUUID xid;
				(*cur)->GetUUID(xid);
				VUUIDBuffer bufid;
				xid.ToBuffer(bufid);
				err = buf.PutData(bufid.fBytes, sizeof(bufid.fBytes));
				if (err != VE_OK)
					break;
			}
		}

		nbfields = fDestinations.GetCount();
		if (err == VE_OK)
		err = buf.PutLong(nbfields);

		if (err == VE_OK)
		{
			for (FieldArray::ConstIterator cur = fDestinations.First()+1, end = fDestinations.End(); cur != end; cur++)
			{
				VUUID xid;
				(*cur)->GetUUID(xid);
				VUUIDBuffer bufid;
				xid.ToBuffer(bufid);
				err = buf.PutData(bufid.fBytes, sizeof(bufid.fBytes));
				if (err != VE_OK)
					break;
			}
		}

	}

	return err;
}


VError Relation::GetFrom(VStream& buf, const Base4D* bd)
{
	VError err = VE_OK;
	RelOnDisk dd;
	VUUID xid;

	err = buf.GetData(&dd, (uLONG)sizeof(dd));
	if (err == VE_OK)
	{
		if (buf.NeedSwap())
			dd.SwapBytes();
		(*this).bd = const_cast<Base4D*>(bd);
		//fType = (DB4D_RelationType)dd.type;
		fID.FromBuffer(dd.ID);

		xid.FromBuffer(dd.SourceField);
		Field *sourceField = bd->FindAndRetainFieldRef( xid);
		if (fSource == NULL && sourceField != NULL)		// sc 16/05/2008 ACI0057889
		{
			CopyRefCountable( &fSource, sourceField);

			// add the new source to the source array
			assert(fSources.IsEmpty());
			fSource->Retain();
			fSources.AddHead( fSource);
		}
		else
		{
			// sc 14/12/2010 should never occur
			assert(fSource == sourceField);
			assert(fSources.GetNth(1) == sourceField);
		}
			
		ReleaseRefCountable( &sourceField);

		xid.FromBuffer(dd.DestField);
		Field *destField = bd->FindAndRetainFieldRef(xid);
		if (fDestination == NULL && destField != NULL)	// sc 16/05/2008 ACI0057889
		{
			CopyRefCountable( &fDestination, destField);

			// add the new destination to the destination array
			assert(fDestinations.IsEmpty());
			fDestination->Retain();
			fDestinations.AddHead( fDestination);
		}
		else
		{
			// sc 14/12/2010 should never occur
			assert(fDestination == destField);
			assert(fDestinations.GetNth(1) == destField);
		}

		ReleaseRefCountable( &destField);

		fState = (DB4D_RelationState)dd.RelState;
		fAutoLoadNto1 = (dd.RelNto1IsAuto == 1);
		fAutoLoad1toN = (dd.Rel1toNIsAuto == 1);
		fAutoDelete = (dd.AutoDel == 1);
		fIsForeignKey = (dd.ForeignKey == 1);
		fWithReferentialIntegrity = (dd.WithRefInt == 1);
		//fCheckRefIntOnModify = (dd.CheckRefIntOnModify == 1);

		fExtraLen = dd.ExtraLen;
		fExtraAddr = dd.ExtraAddr;

		if (fSource != nil && fDestination != nil && dd.MultipleFields == 0)
		{
			//if (fSource->GetTyp() == DB4D_SubTable && fDestination->GetTyp() == DB4D_SubTableKey && fType == Relation_1ToN)
			if (fDestination->GetTyp() == DB4D_SubTable && fSource->GetTyp() == DB4D_SubTableKey)
			{
				fIsForSubTable = true;
			}
		}

		err = fName.ReadFromStream(&buf);
		if (err == VE_OK)
			err = fOppositeName.ReadFromStream(&buf);

		if (dd.MultipleFields == 1 && err == VE_OK)
		{
			assert(fSource != NULL && fDestination != NULL);
			
			sLONG nbfields;

			err = buf.GetLong(nbfields);
			if (err == VE_OK)
			{
				bool onlyUpdate = (fSources.GetCount() == nbfields);	// sc 16/05/2008 ACI0057889

				for (sLONG i = 1; i < nbfields; i++)  // on part de 1 car le premier element est deja dedans (fSource)
				{
					VUUID xid;
					VUUIDBuffer bufid;
					err = buf.GetData(bufid.fBytes, sizeof(bufid.fBytes));
					if (err == VE_OK)
					{
						xid.FromBuffer(bufid);
						Field* ff = bd->FindAndRetainFieldRef(xid);
						if (ff != nil)
						{
							if (!onlyUpdate)
							{
								fSources.Add( ff);
							}
							else
							{
								// sc 14/12/2010 should never occur
								assert(fSources.FindPos( ff) != -1);
								ff->Release();
							}
						}
					}
				
					if (err != VE_OK)
						break;
				}
			}

			if (err == VE_OK)
				err = buf.GetLong(nbfields);

			if (err == VE_OK)
			{
				bool onlyUpdate = (fDestinations.GetCount() == nbfields);	// sc 16/05/2008 ACI0057889
				
				for (sLONG i = 1; i < nbfields; i++)  // on part de 1 car le premier element est deja dedans (fDestination)
				{
					VUUID xid;
					VUUIDBuffer bufid;
					err = buf.GetData(bufid.fBytes, sizeof(bufid.fBytes));
					if (err == VE_OK)
					{
						xid.FromBuffer(bufid);
						Field* ff = bd->FindAndRetainFieldRef(xid);
						if (ff != nil)
						{
							if (!onlyUpdate)
							{
								fDestinations.Add(ff);
							}
							else
							{
								// sc 14/12/2010 should never occur
								assert(fDestinations.FindPos( ff) != -1);
								ff->Release();
							}
						}
					}

					if (err != VE_OK)
						break;
				}
			}
		}
	}

	return err;
}



VError Relation::oldGetFrom(VStream& buf, const Base4D* bd)
{
	VError err = VE_OK;
	oldRelOnDisk dd;
	VUUID xid;

	err = buf.GetData(&dd, (uLONG)sizeof(dd));
	if (err == VE_OK)
	{
		if (buf.NeedSwap())
			dd.SwapBytes();
		(*this).bd = const_cast<Base4D*>(bd);
		fOldType = dd.type;
		fID.FromBuffer(dd.ID);

		xid.FromBuffer(dd.SourceField);
		fSource = bd->FindAndRetainFieldRef(xid);

		xid.FromBuffer(dd.DestField);
		fDestination = bd->FindAndRetainFieldRef(xid);

		fState = (DB4D_RelationState)dd.RelState;
		fAutoLoadNto1 = (dd.RelIsAuto == 1);
		fAutoLoad1toN = false;
		fAutoDelete = (dd.AutoDel == 1);
		fIsForeignKey = (dd.ForeignKey == 1);
		fWithReferentialIntegrity = (dd.WithRefInt == 1);
		//fCheckRefIntOnModify = (dd.CheckRefIntOnModify == 1);

		fExtraLen = dd.ExtraLen;
		fExtraAddr = dd.ExtraAddr;

		if (fSource != nil && fDestination != nil && dd.MultipleFields == 0)
		{
			//if (fSource->GetTyp() == DB4D_SubTable && fDestination->GetTyp() == DB4D_SubTableKey && fType == Relation_1ToN)
			if (fSource->GetTyp() == DB4D_SubTable && fDestination->GetTyp() == DB4D_SubTableKey)
			{
				fIsForSubTable = true;
			}
		}

		if (fSource != nil)
			fSource->Retain();
		fSources.Add(fSource);

		if (fDestination != nil)
			fDestination->Retain();
		fDestinations.Add(fDestination);

		err = fName.ReadFromStream(&buf);

		if (dd.MultipleFields == 1 && err == VE_OK)
		{
			sLONG nbfields;

			err = buf.GetLong(nbfields);
			if (err == VE_OK)
			{
				for (sLONG i = 1; i < nbfields; i++)  // on part de 1 car le premier element est deja dedans (fSource)
				{
					VUUID xid;
					VUUIDBuffer bufid;
					err = buf.GetData(bufid.fBytes, sizeof(bufid.fBytes));
					if (err == VE_OK)
					{
						xid.FromBuffer(bufid);
						Field* ff = bd->FindAndRetainFieldRef(xid);
						if (ff != nil)
						{
							fSources.Add(ff);
						}
					}

					if (err != VE_OK)
						break;
				}
			}

			if (err == VE_OK)
				err = buf.GetLong(nbfields);

			if (err == VE_OK)
			{
				for (sLONG i = 1; i < nbfields; i++)  // on part de 1 car le premier element est deja dedans (fDestination)
				{
					VUUID xid;
					VUUIDBuffer bufid;
					err = buf.GetData(bufid.fBytes, sizeof(bufid.fBytes));
					if (err == VE_OK)
					{
						xid.FromBuffer(bufid);
						Field* ff = bd->FindAndRetainFieldRef(xid);
						if (ff != nil)
						{
							fDestinations.Add(ff);
						}
					}

					if (err != VE_OK)
						break;
				}
			}

		}
	}

	return err;
}

VError Relation::LoadFromBagWithLoader( const VValueBag& inBag, VBagLoader *inLoader, void* inContext)
{
	assert( bd != NULL);
	VError err = VE_OK;

	// get UUID
	inLoader->GetUUID( inBag, fID);
	err = _LoadAttributesFromBag( inBag);		// sc 05/09/2007 factorisation
	if (err != VE_OK)
		err = ThrowError( err, DBaction_CreatingRelation);

	fSource = nil;
	fDestination = nil;
	Field* cri;
	
	// get fields
	VUUID uuid;
	if (err == VE_OK)
	{
		const VBagArray *fields = inBag.RetainElements( DB4DBagKeys::related_field);
		for (sLONG index = fields->GetCount(); index > 0; index--)
		{
			const VValueBag *relatedField = fields->RetainNth( index);
			VString kind;
			if (relatedField->GetString( DB4DBagKeys::kind, kind))
			{
				const VValueBag *fieldRef = relatedField->RetainUniqueElement( DB4DBagKeys::field_ref);
				if (fieldRef != NULL)
				{
					if (kind == L"source")
					{
						cri = bd->FindAndRetainFieldRef( *fieldRef, inLoader);
						if (cri != nil)
						{
							if (fSource == nil)
							{
								fSource = cri;
								fSource->Retain();
							}
							fSources.Add(cri);
						}
					}
					else if (kind == L"destination")
					{
						cri = bd->FindAndRetainFieldRef( *fieldRef, inLoader);
						if (cri != nil)
						{
							if (fDestination == nil)
							{
								fDestination = cri;
								fDestination->Retain();
							}
							fDestinations.Add(cri);
						}
					}
					fieldRef->Release();
				}
			}
			relatedField->Release();
		}
		fields->Release();

		if (fSource == NULL)
			err = ThrowError( VE_DB4D_WRONGSOURCEFIELD, DBaction_CreatingRelation);
		else if (fDestination == NULL)
			err = ThrowError( VE_DB4D_WRONGDESTINATIONFIELD, DBaction_CreatingRelation);
	}
	
	fIsForSubTable = false;
	// check field types
	if (err == VE_OK)
	{
		if (fDestinations.GetCount() != fSources.GetCount())
			err = ThrowError( VE_DB4D_FIELDTYPENOTMATCHING, DBaction_CreatingRelation);
		else
		{
			if ( (fSource != NULL) && (fDestination != NULL) )
			{
				/*
				if (fSource->GetTyp() == DB4D_SubTable && fDestination->GetTyp() == DB4D_SubTableKey && fType == Relation_1ToN)
				{
					fIsForSubTable = true;
				}
				else if (fSource->GetTyp() == DB4D_SubTableKey && fDestination->GetTyp() == DB4D_SubTable && fType == Relation_NTo1)
				{
					// reversed subtable link
				}
				*/

				if (fDestination->GetTyp() == DB4D_SubTable && fSource->GetTyp() == DB4D_SubTableKey)
					fIsForSubTable = true;
				else
				{
					if (fSource->GetTyp() != fDestination->GetTyp())
						err = ThrowError( VE_DB4D_FIELDTYPENOTMATCHING, DBaction_CreatingRelation);
				}
			}
		}
	}

	if (err == VE_OK)
	{
		err = GetExtraPropertiesFromBag( DB4DBagKeys::relation_extra, this, inBag, false, (CDB4DBaseContext*)inContext, inLoader->IsLoadOnly());
	}

	return err;
}


VError Relation::SaveToBag( VValueBag& ioBag, VString& outKind) const
{
	VError err = VE_OK;

	// set properties
	// as a general guideline, all property identifiers must be exclusively lowercase
	// and words separated by '_', no space.
	
	ioBag.SetVUUID( DB4DBagKeys::uuid, fID);
	err = _SaveAttributesToBag( ioBag);		// sc 05/09/2007 factorisation
	
	for (FieldArray::ConstIterator cur = fSources.First(), end = fSources.End(); cur != end; cur++)
	{
		Field* cri = *cur;
		if (cri != nil)
		{
			VValueBag *related_field = new VValueBag;
			if (related_field != NULL)
			{
				related_field->SetString( DB4DBagKeys::kind, CVSTR( "source"));

				VValueBag *ref = cri->CreateReferenceBag( true);
				if (ref != NULL)
				{
					related_field->AddElement( DB4DBagKeys::field_ref, ref);
					ref->Release();
				}
				ioBag.AddElement( DB4DBagKeys::related_field, related_field);
				related_field->Release();
			}
		}
	}

	for (FieldArray::ConstIterator cur = fDestinations.First(), end = fDestinations.End(); cur != end; cur++)
	{
		Field* cri = *cur;
		if (cri != nil)
		{
			VValueBag *related_field = new VValueBag;
			if (related_field != NULL)
			{
				related_field->SetString( DB4DBagKeys::kind, CVSTR( "destination"));

				VValueBag *ref = cri->CreateReferenceBag( true);
				if (ref != NULL)
				{
					related_field->AddElement( DB4DBagKeys::field_ref, ref);
					ref->Release();
				}
				ioBag.AddElement( DB4DBagKeys::related_field, related_field);
				related_field->Release();
			}
		}
	}

	/*
	if (fSource != NULL)
	{
		VValueBag *related_field = new VValueBag;
		if (related_field != NULL)
		{
			related_field->SetString( DB4DBagKeys::kind, CVSTR( "source"));

			VValueBag *ref = fSource->CreateReferenceBag( true);
			if (ref != NULL)
			{
				related_field->AddElement( DB4DBagKeys::field_ref, ref);
				ref->Release();
			}
			ioBag.AddElement( DB4DBagKeys::related_field, related_field);
			related_field->Release();
		}
	}

	if (fDestination != NULL)
	{
		VValueBag *related_field = new VValueBag;
		if (related_field != NULL)
		{
			related_field->SetString( DB4DBagKeys::kind, CVSTR( "destination"));

			VValueBag *ref = fDestination->CreateReferenceBag( true);
			if (ref != NULL)
			{
				related_field->AddElement( DB4DBagKeys::field_ref, ref);
				ref->Release();
			}
			ioBag.AddElement( DB4DBagKeys::related_field, related_field);
			related_field->Release();
		}
	}
	*/
	
	
	// insert extra properties.
	err = PutExtraPropertiesInBag( DB4DBagKeys::relation_extra, const_cast<Relation*>( this), ioBag, nil);
	

	outKind = L"relation";
	
	return err;
}


VError Relation::SetName(const VString& inName, CDB4DBaseContext* inContext)
{
	VError err = VE_OK;

	if (fIsRemote)
	{
		VValueBag bag;
		DB4DRemoteBagKeys::rm_name_Nto1.Set( &bag, inName);
		err = SendReqSetAttributes( bag, inContext);
		if (err == VE_OK)
		{
			fName = inName;
			RegisterForLang();
		}
	}
	else
	{
		Relation* other;
		if (bd == nil) 
			other = nil;
		else 
			other = bd->FindAndRetainRelationByName(inName);

		if (inName.IsEmpty() || (inName != fOppositeName && IsValid4DName(inName) && ((other==nil) || (other == this))) )
		{
			fName = inName;
			Touch();
			RegisterForLang();
		}
		else
			err = ThrowError(VE_DB4D_INVALIDRELATIONNAME, DBaction_ModifyingRelation);

		if (other != nil)
			other->Release();
	}
	return err;
}


VError Relation::SetOppositeName(const VString& inName, CDB4DBaseContext* inContext)
{
	VError err = VE_OK;

	if (fIsRemote)
	{
		VValueBag bag;
		DB4DRemoteBagKeys::rm_name_1toN.Set( &bag, inName);
		err = SendReqSetAttributes( bag, inContext);
		if (err == VE_OK)
		{
			fOppositeName = inName;
			RegisterForLang();
		}
	}
	else
	{
		Relation* other;
		if (bd == nil) 
			other = nil;
		else 
			other = bd->FindAndRetainRelationByName(inName);

		if (inName.IsEmpty() || (inName != fName && IsValid4DName(inName) && ((other==nil) || (other == this))) )
		{
			fOppositeName = inName;
			Touch();
			RegisterForLang();
		}
		else
			err = ThrowError(VE_DB4D_INVALIDRELATIONNAME, DBaction_ModifyingRelation);

		if (other != nil)
			other->Release();
	}
	return err;
}


VError Relation::ThrowError( VError inErrCode, ActionDB4D inAction) const
{
	if (bd != nil)
	{		
		VErrorDB4D_OnRelation *err = new VErrorDB4D_OnRelation(inErrCode, inAction, bd);
		VTask::GetCurrent()->PushRetainedError( err);
	}
	
	return inErrCode;
}


void Relation::RecalcDepIntegrity()
{
	if (fDestination != nil)
	{
		fDestination->GetOwner()->DelRefIntegrityDep(this);

		if (fWithReferentialIntegrity)
		{
			fDestination->GetOwner()->AddRefIntegrityDep(this);
		}
	}
	if (fSource != nil)
	{
		if (fWithReferentialIntegrity)
		{
			fSource->GetOwner()->AddRefIntegrityDepForeign(this);
		}
	}
	/*
	if (fSource != nil)
	{
		fSource->GetOwner()->DelRefIntegrityDep(this);

		if (fWithReferentialIntegrity)
		{
			fSource->GetOwner()->AddRefIntegrityDep(this);
		}
	}
	if (fDestination != nil)
	{
		if (fWithReferentialIntegrity)
		{
			fDestination->GetOwner()->AddRefIntegrityDepForeign(this);
		}
	}
	*/
}


void Relation::CalculDependence()
{
	if (fSource != nil)
	{
		for (FieldArray::ConstIterator cur = fSources.First(), end = fSources.End(); cur != end; cur++)
		{
			Field* source = *cur;
			if (source != nil)
				source->AddRelationNto1Dep(this);
		}

		fSource->GetOwner()->AddRelationNto1Dep(this);
	}

	if (fDestination != nil)
	{
		for (FieldArray::ConstIterator cur = fDestinations.First(), end = fDestinations.End(); cur != end; cur++)
		{
			Field* dest = *cur;
			if (dest != nil)
				dest->AddRelation1toNDep(this);
		}

		fDestination->GetOwner()->AddRelation1toNDep(this);
	}

	if (fDestination != nil)
	{
		if (fWithReferentialIntegrity)
		{
			fDestination->GetOwner()->AddRefIntegrityDep(this);
		}
	}

	if (fSource != nil)
	{
		if (fWithReferentialIntegrity)
		{
			fSource->GetOwner()->AddRefIntegrityDepForeign(this);
		}
	}

	/*
	if (fSource != nil)
	{
		for (FieldArray::ConstIterator cur = fSources.First(), end = fSources.End(); cur != end; cur++)
		{
			Field* source = *cur;
			if (source != nil)
				source->AddRelationDep(this);
		}

		fSource->GetOwner()->AddRelationDep(this);

		if (fWithReferentialIntegrity)
		{
			fSource->GetOwner()->AddRefIntegrityDep(this);
		}
	}

	if (fDestination != nil)
	{
		if (fWithReferentialIntegrity)
		{
			fDestination->GetOwner()->AddRefIntegrityDepForeign(this);
		}
	}
	*/
}


void Relation::RemoveFromDependence()
{
	if (fSource != nil)
	{
		for (FieldArray::ConstIterator cur = fSources.First(), end = fSources.End(); cur != end; cur++)
		{
			Field* source = *cur;
			if (source != nil)
				source->DelRelationNto1Dep(this);
		}
		fSource->GetOwner()->DelRelationNto1Dep(this);
	}

	if (fDestination != nil)
	{
		for (FieldArray::ConstIterator cur = fDestinations.First(), end = fDestinations.End(); cur != end; cur++)
		{
			Field* dest = *cur;
			if (dest != nil)
				dest->DelRelation1toNDep(this);
		}
		fDestination->GetOwner()->DelRelation1toNDep(this);
	}

	if (fDestination != nil)
	{
		if (fWithReferentialIntegrity)
		{
			fDestination->GetOwner()->DelRefIntegrityDep(this);
		}
	}

	if (fSource != nil)
	{
		if (fWithReferentialIntegrity)
		{
			fSource->GetOwner()->DelRefIntegrityDepForeign(this);
		}
	}

	/*
	if (fSource != nil)
	{
		for (FieldArray::ConstIterator cur = fSources.First(), end = fSources.End(); cur != end; cur++)
		{
			Field* source = *cur;
			if (source != nil)
				source->DelRelationDep(this);
		}
		fSource->GetOwner()->DelRelationDep(this);

		if (fWithReferentialIntegrity)
		{
			fSource->GetOwner()->DelRefIntegrityDep(this);
		}
	}

	if (fDestination != nil)
	{
		if (fWithReferentialIntegrity)
		{
			fDestination->GetOwner()->DelRefIntegrityDepForeign(this);
		}
	}
	*/
}


VError Relation::SetExtraProperties(VValueBag* inExtraProperties, bool inNotify, CDB4DBaseContext* inContext, bool loadonly)
{ 
	VError err = VE_OK;

	if (fIsRemote)
	{
		if (inExtraProperties != NULL)
		{
			err = SendReqSetExtraProperties( *inExtraProperties, inContext);
		}
		else
		{
			VValueBag emptyBag;
			err = SendReqSetExtraProperties( emptyBag, inContext);
		}
		if (err == VE_OK)
			err = fExtra.SetExtraProperties( inExtraProperties);
	}
	else
	{
		err = fExtra.SetExtraProperties(inExtraProperties, loadonly);
		//bd->SaveRelations();
		if (!loadonly)
		{
			Touch();
		}
	}

	if (err == VE_OK && inNotify && (VDBMgr::GetCommandManager() != nil))
	{
		VUUID UID_base = bd->GetUUID();
		VDBMgr::GetCommandManager()->Tell_UpdateRelation( UID_base, fID, TUR_ExtraPropertiesChanged);
	}

	return err;
}


VError Relation::Load(StructElemDef* e)
{
	Boolean mustlibere = false;
	VError err = VE_OK;
	if (e == nil)
	{
		e = bd->LoadRelationDef(fPosInBase, err);
		mustlibere = true;
	}

	if (e != nil)
	{
		VConstPtrStream buf(e->GetDataPtr(), (VSize)e->GetDataLen());
		err = buf.OpenReading();
		if (err == VE_OK)
		{
			buf.SetNeedSwap(e->NeedSwap());
			err = GetFrom(buf, bd);

			buf.CloseReading();
		}
		if (mustlibere)
			e->libere();
	}
	return err;
}


VError Relation::Save()
{
	VError err = VE_OK;
	if (fCanSave)
	{
		if (bd->StoredAsXML())
		{
			bd->TouchXML();
		}
		else
		{
			xbox::VPtrStream buf;

			//if (fPosInBase > 0)
			{
				err = buf.OpenWriting();
				buf.SetNeedSwap(false);
				if (err == VE_OK)
				{
					occupe();
					err = PutInto(buf, bd);
					libere();

					if (err == VE_OK)
					{
						Boolean isnew = false;
						StructElemDef* e;
						if (fPosInBase == -1)
						{
							e = new StructElemDef(bd->GetStructure(), -1, DBOH_RelationDefElem);
							if (e == nil)
								err = memfull;
							isnew = true;
						}
						else
							e = bd->LoadRelationDef(fPosInBase, err);
						if (err == VE_OK)
						{
							if (e == nil)
							{
								e = new StructElemDef(bd->GetStructure(), fPosInBase, DBOH_RelationDefElem);
								isnew = true;
							}
							err = e->CopyData(buf.GetDataPtr(), buf.GetSize());
							if (err == VE_OK)
							{
								err = bd->SaveRelationDef(e);
								if (err == VE_OK)
								{
									if (fPosInBase == -1)
										fPosInBase = e->GetNum();
								}
								e->libere();
							}
							else
							{
								if (isnew)
									delete e;
							}
						}
					}

					buf.CloseWriting(false);

				}
			}
		}
	}

	return err;
}


VError Relation::Update( VStream *inDataGet)
{
	VError err = VE_OK;
	VValueBag *extraBag = NULL;
	VUUID oldID = fID;

	occupe();
	err = GetFrom( *inDataGet, bd);
	if (err == VE_OK)
		err = inDataGet->GetLong( fStamp);	// sc 16/05/2008 read the stamp
	if (err == VE_OK)
	{
		extraBag = new VValueBag();
		err = extraBag->ReadFromStream( inDataGet);
	}
	if (err == VE_OK)
		fExtra.SetExtraProperties( extraBag);
	libere();
	ReleaseRefCountable( &extraBag);

	if (err == VE_OK)
	{
		if (fID != oldID)
		{
			bd->DelObjectID( objInBase_Relation, this, oldID);
			bd->AddObjectID( objInBase_Relation, this, fID);
		}
		RegisterForLang();
	#if 0 // sc 14/12/2010 ACI0068541, now it's done in Base4D::UpdateRelation()
		CalculDependence();
	#endif
	}
	return err;
}


VError Relation::SendToClient( VStream *inDataSend)
{
	VError err = VE_OK;
	
	occupe();
	VValueBag *extraBag = fExtra.RetainExtraProperties( err);
	if (err != VE_OK)
		ReleaseRefCountable( &extraBag);
	if (extraBag == NULL)
		extraBag = new VValueBag();
	err = PutInto( *inDataSend, bd);
	if (err == VE_OK)
		inDataSend->PutLong( fStamp);	// sc 16/05/2008 pass the stamp
	libere();
	if (err == VE_OK)
		err = extraBag->WriteToStream( inDataSend);
	
	ReleaseRefCountable( &extraBag);

	return err;
}


VError Relation::SendReqSetAttributes( const VValueBag& inBag, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;

	if (testAssert(fIsRemote && (bd!=NULL) && bd-> GetNetAccess ( ) ))
	{
		IRequest *req = bd->CreateRequest( inContext, Req_SetRelationAttributes + kRangeReqDB4D);
		if (req == NULL)
		{
			err = ThrowBaseError(memfull, DBaction_ModifyingRelation);
		}
		else
		{
			req->PutBaseParam( bd);
			req->PutRelationParam( this);
			req->PutValueBagParam( inBag);
		
			err = req->GetLastError();
			if (err == VE_OK)
			{
				err = req->Send();
			}
			req->Release();
		}
	}
	return err;
}


VError Relation::SendReqSetExtraProperties( const VValueBag& inBag, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;

	if (testAssert(fIsRemote && (bd!=NULL)))
	{
		IRequest *req = bd->CreateRequest( inContext, Req_SetRelationExtraProperties + kRangeReqDB4D);
		if (req == NULL)
		{
			err = ThrowBaseError(memfull, DBaction_ModifyingExtraProperty);
		}
		else
		{
			req->PutBaseParam( bd);
			req->PutRelationParam( this);
			req->PutValueBagParam( inBag);
		
			err = req->GetLastError();
			if (err == VE_OK)
			{
				err = req->Send();
			}
			req->Release();
		}
	}
	return err;
}


void Relation::ExecReqSetAttributes( IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	
	Base4D *base = inRequest->RetainBaseParam( err);
	if (err == VE_OK && base != NULL)
	{
		Relation *relation = inRequest->RetainRelationParam( base, err);
		if (err == VE_OK && relation != NULL)
		{
			VValueBag *bag = inRequest->RetainValueBagParam( err);
			if (err == VE_OK && bag != NULL)
			{
				ObjLocker locker( inContext, relation);
				if (locker.CanWork())
				{
					VString stringVal;
					sLONG longVal;
					Boolean booleanVal;
					
					if (bag->GetString( DB4DRemoteBagKeys::rm_name_Nto1, stringVal))
						err = relation->SetName( stringVal, inContext);

					if (err == VE_OK && bag->GetString( DB4DRemoteBagKeys::rm_name_1toN, stringVal))
						err = relation->SetOppositeName( stringVal, inContext);

					if (err == VE_OK && bag->GetBoolean( DB4DRemoteBagKeys::rm_auto_load_Nto1, booleanVal))
						err = relation->SetAutoNto1Load( booleanVal, inContext);

					if (err == VE_OK && bag->GetBoolean( DB4DRemoteBagKeys::rm_auto_load_1toN, booleanVal))
						err = relation->SetAuto1toNLoad( booleanVal, inContext);

					if (err == VE_OK && bag->GetString( DB4DRemoteBagKeys::rm_integrity, stringVal))
					{
						if (stringVal.EqualToString( CVSTR( "none")))
						{
							err = relation->SetReferentialIntegrity( false, false, inContext);
						}
						else if (stringVal == CVSTR("reject"))
						{
							err = relation->SetReferentialIntegrity( true, false, inContext);
						}
						else if (stringVal == CVSTR("delete"))
						{
							err = relation->SetReferentialIntegrity( true, true, inContext);
						}
						else
						{
							err = VE_INVALID_PARAMETER;
						}
					}

					if (err == VE_OK && bag->GetBoolean( DB4DRemoteBagKeys::rm_foreign_key, booleanVal))
						err = relation->SetForeignKey( booleanVal, inContext);

					const VValueSingle *value = bag->GetAttribute(DB4DRemoteBagKeys::rm_state);
					if (err == VE_OK && value != NULL)
					{
						sLONG state = value->GetLong();
						if (state != RelationState_FromSource && state != RelationState_ReadOnly && state != RelationState_ReadWrite)
						{
							err = VE_INVALID_PARAMETER;
						}
						else
						{
							err = relation->SetState( (DB4D_RelationState) state);
						}
					}

					if (err == VE_OK)
					{
						inRequest->InitReply(0);
						alreadysend = true;
					}
				}
				else
					err = base->ThrowError( VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT, DBaction_ModifyingRelation);
			}
			ReleaseRefCountable( &bag);
		}
		ReleaseRefCountable( &relation);
	}
	ReleaseRefCountable( &base);

	if (!alreadysend)
		SendError( inRequest, inContext, err);
}


void Relation::ExecReqSetExtraProperties( IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	
	Base4D *base = inRequest->RetainBaseParam( err);
	if (err == VE_OK && base != NULL)
	{
		Relation *relation = inRequest->RetainRelationParam( base, err);
		if (err == VE_OK && relation != NULL)
		{
			VValueBag *bag = inRequest->RetainValueBagParam( err);
			if (err == VE_OK && bag != NULL)
			{
				ObjLocker locker( inContext, relation);
				if (locker.CanWork())
				{
					err = relation->SetExtraProperties( bag, false, inContext);
					if (err == VE_OK)
					{
						inRequest->InitReply(0);
						alreadysend = true;
					}
				}
				else
					err = base->ThrowError( VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT, DBaction_ModifyingExtraProperty);
			}
			ReleaseRefCountable( &bag);
		}
		ReleaseRefCountable( &relation);
	}
	ReleaseRefCountable( &base);

	if (!alreadysend)
		SendError( inRequest, inContext, err);
}


Boolean Relation::Match(const FieldArray& sources, const FieldArray& dests)
{
	return (sources == fSources && dests == fDestinations);
}


void Relation::SwapSourceDest()
{
	assert(fSources.GetCount() == fDestinations.GetCount());

	FieldArray temp;
	temp.CopyFrom(fSources);
	fSources.CopyFrom(fDestinations);
	fDestinations.CopyFrom(temp);
}


void Relation::Touch() 
{ 
//	EntityModel::ClearCacheTableEM();
	fStamp++;
	if (fStamp == 0)
		fStamp = 1;
	if (bd != nil) 
		bd->Touch();
	Save();
};



VError Relation::CopyFields(FicheInMem* fichesource, FicheInMem* fichedest, Boolean OldOne)
{
	VError err = VE_OK;
	for (FieldArray::ConstIterator cur = fSources.First(), end = fSources.End(), curdest = fDestinations.First(); cur != end; cur++, curdest++)
	{
		Field* source = *cur;
		Field* dest = *curdest;

		VValueSingle* cv;
		if (OldOne)
			cv = fichesource->GetFieldOldValue(dest, err, true);
		else
			cv = fichesource->GetFieldValue(dest, err, false, true);
		if (cv != nil)
		{
			VValueSingle* cv2 = fichedest->GetFieldValue(source, err, false, true);
			if (cv2 != nil)
			{
				cv2->FromValue(*cv);
				fichedest->Touch(source);
			}
		}

		if (err != VE_OK)
			break;
	}

	return err;
}


VError Relation::AddConstraint()
{
	return bd->AddConstraint(this);
}


VError Relation::AddConstraintCols()
{
	VError err = VE_OK;
	sLONG n = 1;
	for (FieldArray::Iterator cur = fSources.First(), end = fSources.End(); cur != end && err == VE_OK; cur++, n++)
	{
		err = bd->AddConstraintCol(this, *cur, n, fDestinations[n]);
	}
	return err;
}


VError Relation::DelConstraint()
{
	return bd->DelConstraint(this);
}


VError Relation::DelConstraintCols()
{
	return bd->DelConstraintCol(this);
}



/* ************************************************************************************************ */


RelationPath::~RelationPath()
{
	/*
	RelationInList* xrel = fPath.GetFirst();
	while (xrel != nil)
	{
		RelationInList* xrel2 = xrel->GetNext();
		fPath.Remove(xrel);
		delete xrel;
		xrel = xrel2;
	}
	*/
}


Boolean RelationPath::BuildPath(BaseTaskInfo* context, Table* source, Table* dest, VError& err, Boolean Only_N_To_1, Boolean Only_Auto, const vector<uBYTE>& inWayOfLocking)
{
	Boolean res = false;

	fContainsSubTables = false;

	if (source == dest)
	{
		err = VE_OK;
		res = true;
	}
	else
	{
		Bittab deja;

		if (OKTable(inWayOfLocking, dest))
		{
			err = deja.aggrandit(source->GetOwner()->GetNBTable() + 30);
			if (err == VE_OK)
			{
				res = BuildSubPath(context, &deja, source, dest, err, Only_N_To_1, Only_Auto, 0, inWayOfLocking);
			}
		}
	}

	for (ListOfRelation::iterator cur = fPath.begin(), end = fPath.end(); cur != end; cur++)
	{
		if (cur->GetRel()->IsForSubtable())
			fContainsSubTables = true;
	}

	return res;
}


Boolean RelationPath::BuildPath(BaseTaskInfo* context, sLONG source, sLONG dest, VError& err, Boolean Only_N_To_1, Boolean Only_Auto, const vector<uBYTE>& inWayOfLocking)
{
	Boolean result = false;

	fContainsSubTables = false;
	Base4D* db = context->GetBase();
	Table * Tsource = db->RetainTable(source);
	Table * Tdest = db->RetainTable(dest);
	if (Tsource != nil && Tdest != nil)
		result = BuildPath(context, Tsource, Tdest, err, Only_N_To_1, Only_Auto, inWayOfLocking);
	
	if (Tsource != nil)
		Tsource->Release();
	else
		err = VE_DB4D_WRONGTABLEREF;

	if (Tdest != nil)
		Tdest->Release();
	else
		err = VE_DB4D_WRONGTABLEREF;

	return result;
}


Boolean RelationPath::BuildSubPath(BaseTaskInfo* context, Bittab* deja, Table* source_intermediaire, Table* finaldest, VError& err, Boolean Only_N_To_1, Boolean Only_Auto, sLONG curlevel, const vector<uBYTE>& inWayOfLocking)
{
	Boolean res = false;

	if (curlevel < 8/* && OKTable(inWayOfLocking, source_intermediaire)*/)
	{
		deja->Set(source_intermediaire->GetNum());
		source_intermediaire->occupe();

		DepRelationArrayIncluded* reldep = source_intermediaire->GetRelNto1Deps();
		sLONG i,nb = reldep->GetCount();

		ListOfRelation GoodPath = fPath;

		Boolean dejaun = false;

		// premiere passe a un niveau

		for (i=1; i<=nb; i++)
		{
			Relation* rel2 = (*reldep)[i];
			if (rel2 != nil && (!Only_Auto || rel2->IsAutoLoadNto1(context)))
			{
				fPath.clear();
				if (BuildSubPathNto1(context, deja, rel2, finaldest, err, Only_N_To_1, Only_Auto, -1, inWayOfLocking))
				{
					GoodPath = fPath;
					dejaun = true;
					res = true;
					break;
				}
			}
		}

		if (!res && !Only_N_To_1)
		{
			reldep = source_intermediaire->GetRel1toNDeps();
			nb = reldep->GetCount();

			for (i=1; i<=nb; i++)
			{
				Relation* rel2 = (*reldep)[i];
				if (rel2 != nil && (!Only_Auto || rel2->IsAutoLoad1toN(context)))
				{
					fPath.clear();
					if (BuildSubPath1toN(context, deja, rel2, finaldest, err, Only_N_To_1, Only_Auto, -1, inWayOfLocking))
					{
						GoodPath = fPath;
						dejaun = true;
						res = true;
						break;
					}
				}
			}

		}

		if (!res)
		{
			// deuxieme passe plus en profondeur
			reldep = source_intermediaire->GetRelNto1Deps();
			nb = reldep->GetCount();

			for (i=1; i<=nb; i++)
			{
				Relation* rel2 = (*reldep)[i];
				if (rel2 != nil && (!Only_Auto || rel2->IsAutoLoadNto1(context)))
				{
					fPath.clear();
					if (BuildSubPathNto1(context, deja, rel2, finaldest, err, Only_N_To_1, Only_Auto, curlevel+1, inWayOfLocking))
					{
						if (!dejaun)
							GoodPath = fPath;
						else
						{
							if (GoodPath.size() > fPath.size())
								GoodPath = fPath;
						}
						dejaun = true;
						res = true;
						//break;
					}
				}
			}

			if (!Only_N_To_1)
			{
				reldep = source_intermediaire->GetRel1toNDeps();
				nb = reldep->GetCount();

				for (i=1; i<=nb; i++)
				{
					Relation* rel2 = (*reldep)[i];
					if (rel2 != nil && (!Only_Auto || rel2->IsAutoLoad1toN(context)))
					{
						fPath.clear();
						if (BuildSubPath1toN(context, deja, rel2, finaldest, err, Only_N_To_1, Only_Auto, curlevel+1, inWayOfLocking))
						{
							if (!dejaun)
								GoodPath = fPath;
							else
							{
								if (GoodPath.size() > fPath.size())
									GoodPath = fPath;
							}
							dejaun = true;
							res = true;
							//break;
						}
					}
				}

			}
		}

		if (res)
			fPath = GoodPath;

		source_intermediaire->libere();
		deja->Clear(source_intermediaire->GetNum());
	}

	return res;
}


Boolean RelationPath::BuildSubPathNto1(BaseTaskInfo* context, Bittab* deja, Relation* Rel, Table* finaldest, VError& err, Boolean Only_N_To_1, Boolean Only_Auto, sLONG curlevel, const vector<uBYTE>& inWayOfLocking)
{
	Boolean res = false;
	Table* dest =   Rel->GetDest() == nil ? nil : Rel->GetDest()->GetOwner();
	if (dest != nil)
	{
		if (dest == finaldest)
		{
			res = true;
		}
		else
		{
			if (curlevel >= 0 && OKTable(inWayOfLocking, dest) && !deja->isOn(dest->GetNum()))
			{
				res = BuildSubPath(context, deja, dest, finaldest, err, Only_N_To_1, Only_Auto, curlevel, inWayOfLocking);
			}

		}
	}
	if (res)
	{
		fPath.push_front(RelationInList(Rel, true));
	}

	return res;
}



Boolean RelationPath::BuildSubPath1toN(BaseTaskInfo* context, Bittab* deja, Relation* Rel, Table* finaldest, VError& err, Boolean Only_N_To_1, Boolean Only_Auto, sLONG curlevel, const vector<uBYTE>& inWayOfLocking)
{
	Boolean res = false;
	Table* dest =   Rel->GetSource() == nil ? nil : Rel->GetSource()->GetOwner();
	if (dest != nil)
	{
		if (dest == finaldest)
		{
			res = true;
		}
		else
		{
			if (curlevel >= 0 && OKTable(inWayOfLocking, dest) && !deja->isOn(dest->GetNum()))
			{
				res = BuildSubPath(context, deja, dest, finaldest, err, Only_N_To_1, Only_Auto, curlevel, inWayOfLocking);
			}

		}
	}
	if (res)
	{
		fPath.push_front(RelationInList(Rel, false));
	}

	return res;
}


VError RelationPath::ActivateRelation(FicheInMem* source, FicheInMem* &dest, BaseTaskInfo* context, Boolean readonly)
{
	VError err = VE_OK;
	dest = nil;

	if (source != nil)
	{
		FicheInMem* currentsource = source;
		currentsource->Retain();

		ListOfRelation::iterator xrel = fPath.begin();

		while (xrel != fPath.end())
		{
			Relation* rel = xrel->GetRel();
			FicheInMem* subdest = nil;
			err = rel->ActivateManyToOne(currentsource, subdest, context, false, false, readonly ? DB4D_Do_Not_Lock : DB4D_Keep_Lock_With_Record);
			currentsource->Release();
			if (err != VE_OK)
			{
				if (subdest != nil)
					subdest->Release();
				subdest = nil;
			}
			currentsource = subdest;
			if (currentsource == nil)
			{
				break;
			}
			xrel++;
		}

		dest = currentsource;
	}

	return err;
}


VError RelationPath::CopyInto(JoinPath& outPath)
{
	ListOfRelation::reverse_iterator xrel = fPath.rbegin();
	VError err = VE_OK;

	while (xrel != fPath.rend() && err == VE_OK)
	{
		Relation* rel = xrel->GetRel();

		/*
		err = outPath.AddJoin(rel->GetSource()->GetOwner()->GetNum(), rel->GetSource()->GetPosInRec(),
													rel->GetDest()->GetOwner()->GetNum(), rel->GetDest()->GetPosInRec());
		*/
		if (xrel->IsNto1())
			err = outPath.AddJoin(rel->GetSources(), rel->GetDestinations(), 0, 0);
		else
			err = outPath.AddJoin(rel->GetDestinations(), rel->GetSources(), 0, 0);
		xrel++;
	}

	return err;
}
























