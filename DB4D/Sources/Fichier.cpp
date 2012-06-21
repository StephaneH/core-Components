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
#include "../Headers/DB4DBagKeys.h"
//#include "DB4DLang/DB4DLang.h"
#include <algorithm>

// #include "..\db4dlang\CDB4D_Lang.h"

#if debuglr 
sLONG xxbuf[kNbElemTabAddr];
#endif

AddrTable *filletable=0;


									/* -----------------------------------------------  */

Boolean operator ==(const FieldArray& x1, const FieldArray& x2)
{
	if (x1.GetCount() != x2.GetCount())
		return false;

	for (FieldArray::ConstIterator cur = x1.First(), end = x1.End(), cur2 = x2.First(); cur != end; cur++, cur2++)
	{
		if (*cur != *cur2)
			return false;
	}
	return true;
}

									/* -----------------------------------------------  */
									
									
VError Field::ThrowError( VError inErrCode, ActionDB4D inAction)
{
	Base4D* owner = Owner->GetOwner();

	VErrorDB4D_OnTable *err = new VErrorDB4D_OnTable(inErrCode, inAction, owner, Owner->GetNum());
	VTask::GetCurrent()->PushRetainedError( err);
	
	return inErrCode;
}

void Field::xInit()
{
	fCRD.nom[0] = 0;
	fCRD.ExtraAddr = 0;
	fCRD.UnusedLong1 = 0;
	fCRD.ReUsedNumber = 0;
	fCRD.UnusedAddr2 = 0;
	fCRD.ExtraLen = 0;
	fCRD.LimitingLength = 0;
	fCRD.typ = 0;
	fCRD.fStamp = 1;
	fCRD.not_null = 0;
	fCRD.unique = 0;
	fCRD.fOuterData = 0;
	fCRD.fTextIsWithStyle = 0;
	fCRD.fUnusedChar3 = 0;
	fCRD.fAutoSeq = 0;
	fCRD.fAutoGenerate = 0;
	fCRD.fStoreAsUTF8 = 0;
	fCRD.fNeverNull = 0;
	fCRD.fDefaultValue = 0;
}
									
Field::Field( Table *inTable, Boolean isremote)
: fExtra(&fCRD.ExtraAddr, &fCRD.ExtraLen, inTable->GetOwner()->GetStructure(), inTable->GetNum(), -1, DBOH_ExtraFieldProperties, isremote)
{
	fIsRemote = isremote;
	xInit();
	cri = NULL;
	PosInRec = 0;
	Owner = inTable;
	FieldVar = NULL;
	FieldRefVar = NULL;
	fRegistered = false;
	oldtypEnregistre = -1;
	if (inTable != NULL)
		inTable->Retain();
	util = nil;
	fEmptyValPtr = nil;
	fStamp = 1;
}

									
Field::Field(sLONG typ, sLONG pos, Table* owner, Boolean isremote)
: fExtra(&fCRD.ExtraAddr, &fCRD.ExtraLen, owner->GetOwner()->GetStructure(), owner->GetNum(), pos, DBOH_ExtraFieldProperties, isremote)
{
	fIsRemote = isremote;
	xInit();
	cri = CreateCritere(typ, &fCRD);
	PosInRec = pos;
	Owner = owner;
	FieldVar = nil;
	FieldRefVar = nil;
	fRegistered = false;
	oldtypEnregistre = -1;
	if (Owner != nil)
		Owner->Retain();

	if (cri != nil)
	{
		CreateEmptyValue();
		fStamp = cri->GetStamp();
	}
	else
		ThrowBaseError(VE_DB4D_UNKNOWN_FIELD_TYPE);
}


Field::Field( const CritereDISK *criD, sLONG pos, Table* owner, Boolean isremote)
: fExtra(&fCRD.ExtraAddr, &fCRD.ExtraLen, owner->GetOwner()->GetStructure(), owner->GetNum(), pos, DBOH_ExtraFieldProperties, isremote)
{
	fIsRemote = isremote;
	xInit();
	cri = CreateCritere(criD, &fCRD);
	PosInRec = pos;
	Owner = owner;
	FieldVar = nil;
	fRegistered = false;
	FieldRefVar = nil;
	oldtypEnregistre = -1;
	if (Owner != nil)
		Owner->Retain();

	if (cri != nil)
	{
		CreateEmptyValue();
		fStamp = cri->GetStamp();
	}
	else
		ThrowBaseError(VE_DB4D_UNKNOWN_FIELD_TYPE);
}


Field::Field( const CritereDISK *criD, VValueBag *inExtraProperties, sLONG pos, Table* owner)
: fExtra(&fCRD.ExtraAddr, &fCRD.ExtraLen, owner->GetOwner()->GetStructure(), owner->GetNum(), pos, DBOH_ExtraFieldProperties, true)
{
	fIsRemote = true;
	xInit();
	cri = CreateCritere(criD, &fCRD);
	PosInRec = pos;
	Owner = owner;
	FieldVar = nil;
	fRegistered = false;
	FieldRefVar = nil;
	oldtypEnregistre = -1;
	if (Owner != nil)
		Owner->Retain();

	if (cri != nil)
	{
		CreateEmptyValue();
		fStamp = cri->GetStamp();
	}
	else
		ThrowBaseError(VE_DB4D_UNKNOWN_FIELD_TYPE);
	fExtra.SetExtraProperties( inExtraProperties);
}


Field::~Field()
{
	UnRegisterForLang();
	if (cri != nil)
		delete cri;
	if (Owner != nil)
		Owner->Release();
}


void Field::Touch() 
{ 
//	EntityModel::ClearCacheTableEM();
	if (testAssert(!fIsRemote))
	{
		fStamp++;
		if (fStamp == 0)
			fStamp = 1;
		if(cri != nil)
			cri->SetStamp(fStamp);
		if (Owner != nil) 
			Owner->Touch(); 
	}
}


void Field::CreateEmptyValue()
{
	//util = CreVValueFromNil( cri->GetTyp(), -1);
	util = Table::xGetEmptyVal(cri->GetTyp());
	fEmptyValPtr = Table::xGetEmptyPtr(cri->GetTyp());

	fValueInfo = (util != NULL) ? util->GetValueInfo() : NULL;
}

void Field::SetPosInRec(sLONG p) 
{ 
	PosInRec = p; 
	fExtra.InitLater(Owner->GetNum(), p); 
}


sLONG Field::GetMaxLenForSort() const
{
	sLONG lenmax = GetLimitingLen();
	if (lenmax<=0)
		lenmax = 20;
	if (lenmax>kMaxXStringLen)
		lenmax = kMaxXStringLen;
	if (lenmax < (sizeof(void*)/2))
		lenmax = (sizeof(void*)/2);
	return lenmax * 2;
}


sLONG Field::GetSwitchSize() const
{
	if (Owner->GetOwner()->AlwaysStoreBlobsInRecs())
		return 0x7FFFFFFF;
	else
		return cri->GetBlobSwitchSize();
}


VError Field::SetTextSwitchSize(sLONG len, CDB4DBaseContext* inContext)
{
	VError err = VE_OK;

	if (fIsRemote)
	{
		IRequest *req = GetOwner()->GetOwner()->CreateRequest( inContext, Req_SetFieldTextSwitchSize + kRangeReqDB4D);
		if (req == NULL)
		{
			err = ThrowBaseError(memfull, DBaction_ChangingFieldProperties);
		}
		else
		{
			req->PutBaseParam( GetOwner()->GetOwner());
			req->PutTableParam( GetOwner());
			req->PutFieldParam( this);
			req->PutLongParam( len);
		
			err = req->GetLastError();
			if (err == VE_OK)
			{
				err = req->Send();
				if (err == VE_OK)
				{
					occupe();
					cri->SetTextSwitchSize( len);
					libere();
				}
			}
			req->Release();
		}
	}
	else
	{
		sLONG oldlen = cri->GetTextSwitchSize();
		cri->SetTextSwitchSize(len);
		if (oldlen != len)
		{
			Touch();
			err = Owner->save();
		}
	}
	return err;
}


VError Field::SetBlobSwitchSize(sLONG len, CDB4DBaseContext* inContext)
{
	VError err = VE_OK;

	if (fIsRemote)
	{
		IRequest *req = GetOwner()->GetOwner()->CreateRequest( inContext, Req_SetFieldTextSwitchSize + kRangeReqDB4D);
		if (req == NULL)
		{
			err = ThrowBaseError(memfull, DBaction_ChangingFieldProperties);
		}
		else
		{
			req->PutBaseParam( GetOwner()->GetOwner());
			req->PutTableParam( GetOwner());
			req->PutFieldParam( this);
			req->PutLongParam( len);

			err = req->GetLastError();
			if (err == VE_OK)
			{
				err = req->Send();
				if (err == VE_OK)
				{
					occupe();
					cri->SetBlobSwitchSize( len);
					libere();
				}
			}
			req->Release();
		}
	}
	else
	{
		sLONG oldlen = cri->GetBlobSwitchSize();
		cri->SetBlobSwitchSize(len);
		if (oldlen != len)
		{
			Touch();
			err = Owner->save();
		}
	}
	return err;
}


void Field::SetLimitingLen(sLONG len, CDB4DBaseContext* inContext) 
{ 
	if (fIsRemote)
	{
	}
	else
	{
		sLONG oldlen = cri->GetLimitingLen();
		cri->SetLimitingLen(len);
		if (oldlen != len)
		{
			Touch();
			Owner->save();
		}
	}
}


bool Field::IsPrimKey()
{
	if (Owner != nil)
	{
		if (Owner->IsPrimKey(this))
			return true;
	}
	return false;
}


void Field::SetStyledText(Boolean x, CDB4DBaseContext* inContext, VDB4DProgressIndicator* InProgress)
{
	if (fIsRemote)
	{
		IRequest *req = GetOwner()->GetOwner()->CreateRequest( inContext, Req_SetFieldStyledText + kRangeReqDB4D);
		if (req == NULL)
		{
			ThrowBaseError( memfull, DBaction_ChangingFieldProperties);
		}
		else
		{
			req->PutBaseParam( GetOwner()->GetOwner());
			req->PutTableParam( GetOwner());
			req->PutFieldParam( this);
			req->PutBooleanParam( x);
			req->PutProgressParam( InProgress);

			VError err = req->GetLastError();
			if (err == VE_OK)
			{
				err = req->Send();
				if (err == VE_OK)
				{
					occupe();
					cri->SetStyledText( x);
					libere();
				}
			}
			req->Release();
		}
	}
	else
	{
		Boolean old = cri->GetStyledText();
		cri->SetStyledText(x);
		if (old != x)
		{
			Touch();
			Owner->save();
			RebuildAllRelatedIndex(inContext, 0, InProgress);
		}
	}
}


void Field::SetOutsideData(Boolean x, CDB4DBaseContext* inContext)
{
	if (fIsRemote)
	{
		IRequest *req = GetOwner()->GetOwner()->CreateRequest( inContext, Req_SetOutsideData + kRangeReqDB4D);
		if (req == NULL)
		{
			ThrowBaseError( memfull, DBaction_ChangingFieldProperties);
		}
		else
		{
			req->PutBaseParam( GetOwner()->GetOwner());
			req->PutTableParam( GetOwner());
			req->PutFieldParam( this);
			req->PutBooleanParam( x);

			VError err = req->GetLastError();
			if (err == VE_OK)
			{
				err = req->Send();
				if (err == VE_OK)
				{
					occupe();
					cri->SetOutsideData( x);
					libere();
				}
			}
			req->Release();
		}
	}
	else
	{
		Boolean old = cri->GetOutsideData();
		cri->SetOutsideData(x);
		if (old != x)
		{
			Touch();
			Owner->save();
		}
	}
}

    
VError Field::SetUnique(Boolean x, VProgressIndicator* progress, CDB4DBaseContext* inContext) 
{ 
	VError err = VE_OK;
	if (fIsRemote)
	{
	}
	else
	{
		if (IsPrimKey())
		{
		}
		else
		{
			Boolean	old = cri->GetUnique();
			if (cri->CanBeUnique())
			{
				cri->SetUnique(x); 
				if (x)
				{
					if (!old)
					{
						err = TryUnique(progress);
						if (err != VE_OK)
						{
							cri->SetUnique(old); 
							x = old;
						}
						else
							Owner->SetAtLeastOneUnique(this);
					}
				}
				else
				{
					if (old)
					{
						RemoveUniqueFromIndexes();
						Owner->RecalcAtLeastOneUnique();
					}
				}
				if (x != old)
				{
					Touch();
					Owner->save();
				}
			}
			else
			{
				if (x)
					err = ThrowError(VE_DB4D_FIELDTYPE_DOES_NOT_SUPPORT_UNIQUE, DBaction_ChangingFieldProperties);
			}
		}
	}
	return err;
}


VError Field::SetNeverNull(Boolean x, CDB4DBaseContext* inContext)
{
	VError err = VE_OK;

	if (fIsRemote)
	{
		// sc 11/06/2008
		IRequest *req = GetOwner()->GetOwner()->CreateRequest( inContext, Req_SetFieldNeverNull + kRangeReqDB4D);
		if (req == NULL)
		{
			err = ThrowBaseError(memfull, DBaction_ChangingFieldProperties);
		}
		else
		{
			req->PutBaseParam( GetOwner()->GetOwner());
			req->PutTableParam( GetOwner());
			req->PutFieldParam( this);
			req->PutBooleanParam( x);
		
			err = req->GetLastError();
			if (err == VE_OK)
			{
				err = req->Send();
				if (err == VE_OK)
				{
					occupe();
					cri->SetNeverNull( x);
					libere();
				}
			}
			req->Release();
		}
	}
	else
	{
		Boolean old = cri->IsNeverNull();
		if (old != x)
		{
			cri->SetNeverNull(x);
			Touch();
			err = Owner->save();
			RebuildAllRelatedIndex(inContext, x ? 1 : 2, nil);
		}
	}
	return err;
}


void Field::SetAutoSeq(Boolean x, CDB4DBaseContext* inContext) 
{ 
	if (fIsRemote)
	{
	}
	else
	{
		Boolean	old = cri->GetAutoSeq();
		cri->SetAutoSeq(x); 
		if (x)
		{
			if (!old)
				Owner->SetAtLeastOneAutoSeq(this);
		}
		else
		{
			if (old)
				Owner->RecalcAtLeastOneAutoSeq();
		}
		if (x != old)
		{
			Touch();
			Owner->save();
		}
	}
}


void Field::SetAutoGenerate(Boolean x, CDB4DBaseContext* inContext) 
{ 
	if (fIsRemote)
	{
	}
	else
	{
		Boolean	old = cri->GetAutoGenerate();
		cri->SetAutoGenerate(x); 
		if (x)
		{
			if (!old)
				Owner->SetAtLeastOneAutoGenerate(this);
		}
		else
		{
			if (old)
				Owner->RecalcAtLeastOneAutoGenerate();
		}
		if (x != old)
		{
			Touch();
			Owner->save();
		}
	}
}



VError Field::SetNot_Null(Boolean x, VProgressIndicator* progress, CDB4DBaseContext* inContext)
{	
	VError err = VE_OK;
	if (fIsRemote)
	{
	}
	else
	{
		Boolean	old = cri->GetNot_Null();
		if (cri->CanBeNot_Null())
		{
			cri->SetNot_Null(x);
			if (x)
			{
				if (!old)
				{
					err = TryNot_Null(progress);
					if (err != VE_OK)
					{
						x = old;
						cri->SetNot_Null(old);
					}
					else
						Owner->SetAtLeastOneNot_Null(this);
				}
			}
			else
			{
				if (old)
					Owner->RecalcAtLeastOneNot_Null();
			}
			if (x != old)
			{
				Touch();
				Owner->save();
			}
		}
		else
		{
			err = ThrowError(VE_DB4D_FIELDTYPE_DOES_NOT_SUPPORT_NEVER_NULL, DBaction_ChangingFieldProperties);
		}
	}

	return err;
}


sLONG Field::CalcAvgSize() const
{
	if (fValueInfo == nil)
		return 0;
	else
		return (sLONG) fValueInfo->GetAvgSpace();
}


sLONG Field::CalcDataSize(const void* p) const
{
	if (fValueInfo == nil)
		return 0;
	else
		return (sLONG) fValueInfo->GetSizeOfValueDataPtr(p);
}


CompareResult Field::CompareKeys(const void *val1, const void *val2, const VCompareOptions& inOptions)
{
	if (fValueInfo == nil)
		return CR_EQUAL;
	else
	{
		return fValueInfo->CompareTwoPtrToDataWithOptions(val1, val2, inOptions);
		/*
		if (isLike)
		{
			if (IsBeginWith)
				return fValueInfo->CompareTwoPtrToDataBegining_Like(val1, val2, xstrict);
			else
				return fValueInfo->CompareTwoPtrToData_Like(val1, val2, xstrict);
		}
		else
		{
			if (IsBeginWith)
				return fValueInfo->CompareTwoPtrToDataBegining(val1, val2, xstrict);
			else
				return fValueInfo->CompareTwoPtrToData(val1, val2, xstrict);
		}
		*/
	}
}

/*
sLONG Field::CalcAvgSize() const
{
	if (util == nil)
		return 0
	else
		return util->GetAvgSpace();
}


sLONG Field::CalcDataSize(const void* p) const
{
	if (util == nil)
		return 0;
	else
		return util->GetSizeOfValueDataPtr(p);
}
*/


void Field::GetUUID(VUUID& outID) const
{
	cri->GetUUID(outID);
}


void Field::SetUUID(const VUUID& inID)
{
	cri->SetUUID(inID);
}


void Field::RegisterForLang(void)
{
//	CDB4D_Lang* lang4D = VDBMgr::GetLang4D();
//	CLanguage* language = VDBMgr::GetLanguage();

//	VVariableKind kind;
	
	if (Owner->CanRegister())
	{
		if (!Owner->IsNotifying())
		{
			if (VDBMgr::GetCommandManager() != nil)
			{
				VUUID UID_base = Owner->GetOwner()->GetUUID();

				VUUID UID_table;
				Owner->GetUUID( UID_table);

				VUUID UID_field;
				GetUUID( UID_field);

				if (fRegistered)
				{
					ETellUpdateFieldOptions what = TUF_NameChanged;
					if (oldtypEnregistre != cri->GetTyp())
						what |= TUF_TypeChanged;
					VDBMgr::GetCommandManager()->Tell_UpdateField( UID_base, UID_table, UID_field, Owner->GetNum(), PosInRec, what);
				}
				else
				{
					VDBMgr::GetCommandManager()->Tell_AddField( UID_base, UID_table, UID_field, Owner->GetNum(), PosInRec);
				}
			}
		}

		fRegistered = true;
	}
#if 0
	if ( (language != nil) && (lang4D != nil) && (Owner->GetRecordRef() != nil) )
	{
		VString	name;
		GetName(name);
		
		VUUID xUID;
		VUUIDBuffer buf;

		if ( (FieldVar == nil) || (oldtypEnregistre != cri->GetTyp()) )
		{
			cri->GetUUID(xUID);
			oldtypEnregistre = cri->GetTyp();
			if (FieldVar != nil)
			{
				FieldVar->Drop();
				FieldVar->Release();
			}
			FieldVar = Owner->GetRecordRef()->AddVariable(name, xUID, cri->GetLangKind(), cri->GetLangKindClass(), NULL, PosInRec);
		}
		else
		{
			FieldVar->SetName(name);
		}

		if (FieldRefVar == nil)
		{
			cri->GetUUID(xUID);
			xUID.ToBuffer(buf);
			buf.IncFirst8();
			xUID.FromBuffer(buf);
			FieldRefVar = Owner->GetTableRef()->AddVariable(name, xUID, eVK_Class, lang4D->GetFieldClassRef(), NULL, PosInRec);
		}
		else
		{
			FieldRefVar->SetName(name);
		}
		
	}
#endif
}


void Field::UnRegisterForLang(void)
{
//	CDB4D_Lang* lang4D = VDBMgr::GetLang4D();
	sLONG i,nb;
	
	if (!Owner->IsNotifying())
	{
		if (fRegistered)
		{
			if (VDBMgr::GetCommandManager() != nil)
			{
				VUUID UID_base = Owner->GetOwner()->GetUUID();

				VUUID UID_table;
				Owner->GetUUID( UID_table);

				VUUID UID_field;
				GetUUID( UID_field);

				VDBMgr::GetCommandManager()->Tell_DelField( UID_base, UID_table, UID_field, Owner->GetNum(), PosInRec);
			}
		}
	}
	fRegistered = false;
#if 0
	if (FieldVar != nil)
	{
		if ( lang4D != nil )
		{
			FieldVar->Drop();
			FieldVar->Release();
			FieldVar = nil;
			
			if (FieldRefVar != nil)
			{
				FieldRefVar->Drop();
				FieldRefVar->Release();
				FieldRefVar = nil;
			}
			
		}
		
	}
#endif	
}


VError Field::SetName(const VString& name, CDB4DBaseContext* inContext) 
{ 
	VError err = VE_OK;

	if (fIsRemote)
	{
		IRequest *req = GetOwner()->GetOwner()->CreateRequest( inContext, Req_SetFieldName + kRangeReqDB4D);
		if (req == NULL)
		{
			err = ThrowBaseError(memfull, DBaction_ChangingFieldName);
		}
		else
		{
			req->PutBaseParam( GetOwner()->GetOwner());
			req->PutTableParam( GetOwner());
			req->PutFieldParam( this);
			req->PutStringParam( name);
		
			err = req->GetLastError();
			if (err == VE_OK)
			{
				err = req->Send();
				if (err == VE_OK)
				{
					occupe();
					cri->SetName( name);
					libere();
				}
			}
			req->Release();
		}
	}
	else
	{
		if (name.GetLength() <= kMaxFieldNameLength && IsValid4DFieldName(name))
		{
			Field* other = Owner->FindAndRetainFieldRef(name);
			
			if (other==nil || other == this)
			{
				err = cri->SetName(name);
				if (err == VE_OK) 
				{
					Touch();
					RegisterForLang();
					Owner->save();	// YT 29-Aug-2005
				}
			}
			else
			{
				err = ThrowError(VE_DB4D_FIELDNAMEDUPLICATE, DBaction_ChangingFieldName);	// sc 28/03/2007
			}
			
			if (other != nil)
			{
				other->Release();
			}
		}
		else
		{
			err = ThrowError(VE_DB4D_INVALIDFIELDNAME, DBaction_ChangingFieldName);
		}
	}
	return err;
}


VError Field::TryUnique(VProgressIndicator* progress)
{
	VError err = VE_OK;
	DataTable* df = Owner->GetDF();
	if (df != nil)
	{
		CDB4DBase* basex = df->GetDB()->RetainBaseX();
		if (basex != nil)
		{
			CDB4DBaseContext* inContext = basex->NewContext(nil, nil);
			BaseTaskInfo* context = ConvertContext(inContext);
			if (df->LockTable(context))
			{
				NumFieldArray fields;
				fields.Add(GetPosInRec());
				if (df->CheckForNonUniqueField(fields, progress, err, inContext))
					err = ThrowBaseError(VE_DB4D_DUPLICATED_KEY);
				if (err == VE_OK)
				{
					occupe();
					for (IndexArrayIncluded::Iterator cur = IndexDep.First(), end = IndexDep.End(); cur != end; cur++)
					{
						IndexInfo* ind = *cur;
						if (ind != nil)
						{
							if (ind->MatchType(DB4D_Index_OnOneField))
							{
								ind->SetUnique(true);
								ind->Save();
								ind->SaveInStruct();
							}
						}
					}
					libere();
				}
				df->UnLockTable(context);
			}
			else
				err = VE_DB4D_CANNOT_GET_EXCLUSIVEACCESS_ON_TABLE;

			inContext->Release();
			basex->Release();
		}
	}
	return err;
}


void Field::RemoveUniqueFromIndexes()
{
	occupe();
	for (IndexArrayIncluded::Iterator cur = IndexDep.First(), end = IndexDep.End(); cur != end; cur++)
	{
		IndexInfo* ind = *cur;
		if (ind != nil)
		{
			if (ind->MatchType(DB4D_Index_OnOneField))
			{
				ind->SetUnique(false);
				ind->Save();
				ind->SaveInStruct();
			}
		}
	}
	libere();
}


VError Field::TryNot_Null(VProgressIndicator* progress)
{
	VError err = VE_OK;
	DataTable* df = Owner->GetDF();
	if (df != nil)
	{
		BaseTaskInfo context(Owner->GetOwner(), nil, nil, nil);
		if (df->LockTable(&context))
		{
			VString cv2;
			cv2.SetNull(true);

			SearchTab rech(Owner, true);
			rech.AddSearchLineSimple(Owner->GetNum(), GetPosInRec(), DB4D_Equal, &cv2);
			OptimizedQuery query;
			err = query.AnalyseSearch(&rech, &context);
			if (err == VE_OK)
			{
				Selection *sel = query.Perform((Bittab*)nil, nil, &context, err, DB4D_Do_Not_Lock);
				if (sel != nil)
				{
					if (sel->GetQTfic() > 0)
					{
						err = VE_DB4D_CANNOT_SET_FIELD_TO_Not_Null;
					}
					sel->Release();
				}
			}

			df->UnLockTable(&context);
		}
		else
			err = VE_DB4D_CANNOT_GET_EXCLUSIVEACCESS_ON_TABLE;
	}
	return err;
}


VError Field::UpdateField(const CritereDISK *crd1, VValueBag *inExtraProperties)
{
	occupe();

	VUUIDBuffer oldid = cri->getCRD()->ID;

	if (GetTyp() != crd1->typ) 
	{
		Critere* oldcri = cri;
		cri = CreateCritere(crd1, &fCRD);
		if (cri == nil)
			cri = oldcri;
		else
		{
			delete oldcri;
			CreateEmptyValue();
		}
	}
	else
		cri->setCRD(crd1);

	fStamp = cri->GetStamp();
	if (crd1->ID != oldid)
	{
		VUUID xoldid(oldid);
		VUUID xid(crd1->ID);
		Owner->GetOwner()->DelObjectID(objInBase_Field, this, xoldid);
		Owner->GetOwner()->AddObjectID(objInBase_Field, this, xid);
	}

	fExtra.SetExtraProperties( inExtraProperties);
	RegisterForLang();

	libere();

	return VE_OK;
	
}


VError Field::setCRD( const CritereDISK *crd1, CDB4DBaseContext* inContext, Boolean inNotify, VProgressIndicator* progress)
{
	VError err = VE_OK;
	if (fIsRemote)
	{
		assert(false);
		//err = UpdateField(crd1);
	}
	else
	{
		sLONG oldtt = GetTyp();
		DataTable* df;

		if (crd1->not_null)
		{
			if (!GetNot_Null())
				err = TryNot_Null(progress);
		}
		
		if (err == VE_OK)
		{
			if (IsPrimKey())
				((CritereDISK*)crd1)->unique = true;

			if (crd1->unique)
			{
				if (!GetUnique())
					err = TryUnique(progress);
			}
			else
			{
				if (GetUnique())
					RemoveUniqueFromIndexes();
			}
		}

		if (err == VE_OK)
		{
			occupe();

			Critere* oldcri = cri;
			cri = CreateCritere(crd1, &fCRD);
			if (cri == nil)
				cri = oldcri;
			else
				delete oldcri;
			
			Boolean TypeHasChanged = false;
			Touch();
			if (inNotify) 
				RegisterForLang();
			if (oldtt != cri->GetTyp()) 
			{
				TypeHasChanged = true;
				if (oldtt == DB4D_SubTable || cri->GetTyp() == DB4D_SubTable)
					Owner->RecalcAtLeastOneSubTable();
				CreateEmptyValue();
			}
			Owner->ReCalc();
			libere();

			if (!Owner->GetOwner()->StoredAsXML())
			{
				Owner->save();
			}

			if (TypeHasChanged) 
				RebuildAllRelatedIndex(inContext, 0, progress);
		}
	}
	return err;
}


VError Field::CopyFromField(Field *other, CDB4DBaseContext* inContext, VProgressIndicator* progress)
{
	return setCRD(other->getCRD(), inContext, true, progress);
}

/*
sLONG Field::CalcAvgSize()
{
	sLONG res = 0;

	ValPtr cv = CreVValueFromNil(cri->GetTyp(), 0, false);
	if (cv != nil)
	{
		res = cv->GetAvgSpace();
		delete cv;
	}

	return res;
}
*/

void Field::AddIndexDep(IndexInfo* ind)
{
	ind->Retain();
	IndexDep.Add(ind);
	
	if (!fIsRemote && !Owner->GetOwner()->StoredAsXML() )
		Touch();
		
}

void Field::DelIndexDep(IndexInfo* ind)
{
	sLONG i;
	
	for (i=1;i<=IndexDep.GetCount();i++)
	{
		if (ind==IndexDep[i])
		{
			
			if (!fIsRemote && !Owner->GetOwner()->StoredAsXML())
				Touch();
				
			IndexDep.DeleteNth(i);
			break;
		}
	}
}

void Field::ClearIndexDep(void)
{
	IndexInfo* ind;
	IndexDep.SetAllocatedSize(0);
}


uBOOL Field::IsIndexed(void) const
{
	uBOOL res = false;
	occupe();
	IndexArrayIncluded::ConstIterator cur = IndexDep.First(), end = IndexDep.End();
	for (;cur != end; cur++)
	{
		IndexInfo* ind = *cur;
		if (ind != nil && ind->MatchType(DB4D_Index_OnOneField))
		{
			res = true;
			break;
		}
	}
	libere();
	return res;
}

uBOOL Field::IsFullTextIndexed(void) const
{
	uBOOL res = false;
	occupe();
	IndexArrayIncluded::ConstIterator cur = IndexDep.First(), end = IndexDep.End();
	for (;cur != end; cur++)
	{
		IndexInfo* ind = *cur;
		if (ind != nil && ind->MatchType(DB4D_Index_OnKeyWords))
		{
			res = true;
			break;
		}
	}
	libere();
	return res;
}



void Field::RebuildAllRelatedIndex(CDB4DBaseContext* inContext, sLONG NeverNullChange, VProgressIndicator* progress)
{
	IndexArrayIncluded IndexDepCopy;
	IndexDepCopy.CopyAndRetainFrom(IndexDep);
	IndexDep.SetAllocatedSize(0);

	sLONG i, nb = IndexDepCopy.GetCount();

	Base4D* bd = Owner->GetOwner();

	OccupableStack* curstack = gOccPool.GetStackForCurrentThread();


	for (i=nb; i>0; i--)
	{
		Boolean okrebuild = true;

		IndexInfo* ind = IndexDepCopy[i];
		if (ind->MatchType(DB4D_Index_OnOneField))
		{
			if (NeverNullChange != 0)
			{
				if (NeverNullChange == 1) // on demande nevernull pour le champ
				{
					if (!ind->HasNulls(curstack))
						okrebuild = false;
				}
				if (NeverNullChange == 2) // on retire nevernull pour le champ
				{
					okrebuild = true; // on peut pas savoir s'il y a de vraies valeurs nulles pour ce champs donc il faut tout reconstruire
				}
			}

			if (okrebuild)
			{
				sLONG headerType = ind->GetHeaderType();
				VString name = ind->GetName();
				Boolean uniqkeys = ind->IsUniqueKey();
				bd->DeleteIndexByRef(ind, inContext, progress);
				bd->CreIndexOnField(this, headerType, uniqkeys, inContext, progress, &name);
			}
			else
			{
				ind->Retain();
				IndexDep.Add(ind);
			}
		}
		else
			bd->RebuildIndexByRef(ind, inContext, progress);
	}
}


void Field::DropAllRelations(CDB4DBaseContext* inContext)
{
	sLONG i, nb = RelDepNto1.GetCount();
	
	for (i=nb; i>0; i--)
	{
		Relation* rel = RelDepNto1[i];
		RelDepNto1[i] = nil;
		if (rel != nil)
			Owner->GetOwner()->DeleteRelation(rel, inContext);
	}
	RelDepNto1.SetAllocatedSize(0);

	nb = RelDep1toN.GetCount();

	for (i=nb; i>0; i--)
	{
		Relation* rel = RelDep1toN[i];
		RelDep1toN[i] = nil;
		if (rel != nil)
			Owner->GetOwner()->DeleteRelation(rel, inContext);
	}
	RelDep1toN.SetAllocatedSize(0);
}



void Field::DropAllRelatedIndex(CDB4DBaseContext* inContext, VProgressIndicator* progress)
{
	sLONG i, nb = IndexDep.GetCount();

	for (i=nb; i>0; i--)
	{
		IndexInfo* ind = IndexDep[i];
		IndexDep[i] = nil;
		Owner->GetOwner()->DeleteIndexByRef(ind, inContext, progress);
	}
	IndexDep.SetAllocatedSize(0);

}


void Field::CalcDependences(Table* fic, sLONG numfield)
{
	cri->CalcDependences(fic, numfield);
}


VError Field::SetExtraProperties(VValueBag* inExtraProperties, bool inNotify, CDB4DBaseContext* inContext, bool loadonly) 
{ 
	VError err = VE_OK;

	if (fIsRemote)
	{
		if (inExtraProperties != NULL)
		{
			IRequest *req = GetOwner()->GetOwner()->CreateRequest( inContext, Req_SetFieldExtraProperties + kRangeReqDB4D);
			if (req == NULL)
			{
				err = ThrowBaseError(memfull, DBaction_ChangingFieldName);
			}
			else
			{
				req->PutBaseParam( GetOwner()->GetOwner());
				req->PutTableParam( GetOwner());
				req->PutFieldParam( this);
				req->PutValueBagParam( *inExtraProperties);
			
				err = req->GetLastError();
				if (err == VE_OK)
				{
					err = req->Send();
					if (err == VE_OK)
					{
						occupe();
						err = fExtra.SetExtraProperties( inExtraProperties);
						libere();
					}
				}
				req->Release();
			}
		}
	}
	else
	{
		if (loadonly || Owner->GetOwner()->OkToUpdate(err))
		{
			err = fExtra.SetExtraProperties(inExtraProperties, loadonly);

			if (!loadonly)
			{
				if (!Owner->GetOwner()->StoredAsXML())
				{
					Owner->save();
				}
				Owner->GetOwner()->ClearUpdating();
				Touch();

				if (inNotify && (VDBMgr::GetCommandManager() != nil))
				{
					VUUID UID_base = Owner->GetOwner()->GetUUID();

					VUUID UID_table;
					Owner->GetUUID( UID_table);

					VUUID UID_field;
					GetUUID( UID_field);

					VDBMgr::GetCommandManager()->Tell_UpdateField( UID_base, UID_table, UID_field, Owner->GetNum(), PosInRec, TUF_ExtraPropertiesChanged);
				}
			}
		}
	}
	return err;
}


VError Field::LoadFromBagWithLoader( const VValueBag& inBag, VBagLoader *inLoader, void* inContext)
{
	VError err = VE_OK;

	assert( cri == NULL);
	sLONG val = 0;
	inBag.GetLong( DB4DBagKeys::type, val);
	if ((val == VK_STRING || val == VK_TEXT) && (DB4DBagKeys::store_as_UUID( &inBag)))
		val = VK_UUID;
	if (val == VK_STRING  && (DB4DBagKeys::store_as_utf8( &inBag)))
		val = VK_STRING_UTF8;
	if (val == VK_TEXT  && (DB4DBagKeys::store_as_utf8( &inBag)))
		val = VK_TEXT_UTF8;
	cri = CreateCritere( val, &fCRD);
	if (cri == NULL)
		err = ThrowError( VE_DB4D_UNKNOWN_FIELD_TYPE, DBaction_AddingField);

	// get name
	if (err == VE_OK)
	{
		VString s;
		DB4DBagKeys::name.Get( &inBag, s);
		if (!inLoader->WithNamesCheck() || ( s.GetLength() <= kMaxFieldNameLength && IsValid4DFieldName( s)))
			err = cri->SetName( s);
		else
			err = ThrowError(VE_DB4D_INVALIDFIELDNAME, DBaction_AddingField);
	}
	
	// get UUID
	if (err == VE_OK)
	{
		VUUID uuid;
		inLoader->GetUUID( inBag, uuid);
		cri->SetUUID( uuid);
	}

	if (err == VE_OK)
	{
		sLONG typ = cri->GetTyp();
		if ((typ == VK_STRING) || (typ == VK_STRING_UTF8))
			cri->SetLimitingLen( DB4DBagKeys::limiting_length( &inBag));
		cri->SetAutoSeq( DB4DBagKeys::autosequence( &inBag));
		cri->SetAutoGenerate( DB4DBagKeys::autogenerate( &inBag));
		cri->SetStoreUTF8( DB4DBagKeys::store_as_utf8( &inBag));
		cri->SetNot_Null( DB4DBagKeys::not_null( &inBag));
		cri->SetNeverNull( DB4DBagKeys::never_null( &inBag));
		cri->SetUnique( DB4DBagKeys::unique( &inBag));
		cri->SetOutsideData(DB4DBagKeys::outside_blob( &inBag));
		cri->SetStyledText(DB4DBagKeys::styled_text( &inBag));
		if ((typ == VK_TEXT) || (typ == VK_TEXT_UTF8))
			cri->SetTextSwitchSize( DB4DBagKeys::text_switch_size( &inBag));
		if ((typ == VK_BLOB) || (typ == VK_BLOB_DB4D) || (typ == VK_IMAGE))
			cri->SetBlobSwitchSize( DB4DBagKeys::blob_switch_size( &inBag));

	}

	if (err == VE_OK)
	{
		CreateEmptyValue();
	}

	if (err == VE_OK)
	{
		err = GetExtraPropertiesFromBag( DB4DBagKeys::field_extra, this, inBag, false, (CDB4DBaseContext*)inContext, inLoader->IsLoadOnly());
	}

	return err;
}


VError Field::SaveToBag( VValueBag& ioBag, VString& outKind) const
{
	VError err = VE_OK;
	
	occupe();

	VString s;
	GetName(s);
	DB4DBagKeys::name.Set( &ioBag, s);

	VUUID uuid;
	GetUUID( uuid);
	DB4DBagKeys::uuid.Set( &ioBag, uuid);

	sLONG typ = GetTyp();
	if (typ == VK_UUID)
		typ = VK_STRING;
	if (typ == VK_STRING_UTF8)
		typ = VK_STRING;
	if (typ == VK_TEXT_UTF8)
		typ = VK_TEXT;
		
	ioBag.SetLong( DB4DBagKeys::type, typ);
	if (typ == DB4D_StrFix)
		DB4DBagKeys::limiting_length.Set( &ioBag, cri->GetLimitingLen());
	DB4DBagKeys::unique.Set( &ioBag, cri->getCRD()->unique);
	DB4DBagKeys::autosequence.Set( &ioBag, cri->getCRD()->fAutoSeq);
	DB4DBagKeys::autogenerate.Set( &ioBag, cri->getCRD()->fAutoGenerate);
	DB4DBagKeys::store_as_utf8.Set( &ioBag, GetTyp() == VK_STRING_UTF8 || GetTyp() == VK_TEXT_UTF8);
	DB4DBagKeys::store_as_UUID.Set( &ioBag, GetTyp() == VK_UUID);

	if ((typ == VK_TEXT) || (typ == VK_TEXT_UTF8))
		DB4DBagKeys::text_switch_size.Set( &ioBag, cri->GetTextSwitchSize());
	if (typ == VK_BLOB || typ == VK_BLOB_DB4D || typ == VK_IMAGE)
		DB4DBagKeys::blob_switch_size.Set( &ioBag, cri->GetBlobSwitchSize());

	// this one now means NOT NULL
	DB4DBagKeys::not_null.Set( &ioBag, cri->getCRD()->not_null);

	DB4DBagKeys::never_null.Set( &ioBag, cri->getCRD()->fNeverNull);

	DB4DBagKeys::id.Set( &ioBag, GetPosInRec());

	DB4DBagKeys::outside_blob.Set( &ioBag, cri->getCRD()->fOuterData);

	DB4DBagKeys::styled_text.Set( &ioBag, cri->getCRD()->fTextIsWithStyle);

	// insert extra properties.
	err = PutExtraPropertiesInBag( DB4DBagKeys::field_extra, const_cast<Field*>( this), ioBag, nil);

	libere();
	
	outKind = L"field";
	
	return err;
}


VValueBag *Field::CreateReferenceBag( bool inForExport) const
{
	VValueBag *bag = new VValueBag;
	if (bag != NULL)
	{
		occupe();
		
		VUUID uuid;
		GetUUID( uuid);
		DB4DBagKeys::uuid.Set( bag, uuid);
		
		if (inForExport)
		{
			VString s;
			GetName( s);
			DB4DBagKeys::name.Set( bag, s);

			VValueBag *table_ref = Owner->CreateReferenceBag( inForExport);
			if (table_ref != NULL)
			{
				bag->AddElement( DB4DBagKeys::table_ref, table_ref);
				table_ref->Release();
			}
		}
		
		libere();
	}
	return bag;
}


void Field::ClearAllDependencies()
{
	ClearRelationNto1Dep();
	ClearRelation1toNDep();
	ClearIndexDep();
}


void Field::AddRelationNto1Dep(Relation* rel)
{
	rel->Retain();
	RelDepNto1.Add(rel);
}


void Field::DelRelationNto1Dep(Relation* rel)
{
	sLONG i;

	for (i=1;i<=RelDepNto1.GetCount();i++)
	{
		if (rel==RelDepNto1[i])
		{
			RelDepNto1.DeleteNth(i);
			break;
		}
	}
}

void Field::ClearRelationNto1Dep(void)
{
	RelDepNto1.SetAllocatedSize(0);
}



void Field::AddRelation1toNDep(Relation* rel)
{
	rel->Retain();
	RelDep1toN.Add(rel);
}


void Field::DelRelation1toNDep(Relation* rel)
{
	sLONG i;

	for (i=1;i<=RelDep1toN.GetCount();i++)
	{
		if (rel==RelDep1toN[i])
		{
			RelDep1toN.DeleteNth(i);
			break;
		}
	}
}

void Field::ClearRelation1toNDep(void)
{
	RelDep1toN.SetAllocatedSize(0);
}


Relation* Field::GetSubTableRel()
{
	Relation* result = nil;
	sLONG i,nb = RelDep1toN.GetCount();
	for (i=1; i<=nb; i++)
	{
		Relation* rel = RelDep1toN[i];
		if (rel != nil)
		{
			if (rel->IsForSubtable())
			{
				result = rel;
				break;
			}
		}
	}
	return result;
}



VError Field::RetainIndexes(ArrayOf_CDB4DIndex& outIndexes)
{
	outIndexes.ReduceCount(0);
	VObjLock lock(this);

	if (outIndexes.SetAllocatedSize(IndexDep.GetCount()))
	{
		for (IndexArrayIncluded::Iterator cur = IndexDep.First(), end = IndexDep.End(); cur != end; cur++)
		{
			IndexInfo* ind = *cur;
			if (ind != nil)
			{
				VDB4DIndex* xind = new VDB4DIndex(ind);
				if (xind == nil)
					return ThrowError(memfull, DBaction_BuildingListOfIndexes);
				else
				{
					outIndexes.Add(xind);
				}
			}
		}
	}
	else
		return ThrowError(memfull, DBaction_BuildingListOfIndexes);

	return VE_OK;
}



IndexInfo* Field::FindAndRetainIndexLexico(Boolean MustBeValid, BaseTaskInfo* context) const
{
	sLONG i;
	sLONG nb;
	IndexInfo *ind,*ind2;

	occupe();
	nb=IndexDep.GetCount();
	ind2=nil;
	for (i=1;i<=nb;i++)
	{
		ind=IndexDep[i];
		if (ind!=nil)
		{
			if (ind->MatchType(DB4D_Index_OnKeyWords))
			{
				//if (MustBeValid && ind->isValid())
				{
					//ind->UseForQuery();
					ind2=ind;
					ind2->Retain();
					break;
				}			
			}
		}
	}

	libere();

	if (MustBeValid && ind2 != nil)
	{
		if (! ind2->AskForValid(context))
		{
			ind2->Release();
			ind2 = nil;
		}
	}

	return(ind2);
}


IndexInfo* Field::FindAndRetainIndexSimple(uBOOL sortable, Boolean MustBeValid, BaseTaskInfo* context) const
{
	sLONG i;
	sLONG nb;
	IndexInfo *ind,*ind2;

	occupe();
	nb=IndexDep.GetCount();
	ind2=nil;
	for (i=1;i<=nb;i++)
	{
		ind=IndexDep[i];
		if (ind!=nil)
		{
			if (ind->MatchType(DB4D_Index_OnOneField))
			{
				if (sortable)
				{
					if (ind->MayBeSorted())
					{			
						ind2=ind;
						ind2->Retain();
						break;
					}
				}
				else
				{
					ind2=ind;
					ind2->Retain();
					break;
				}
			}
		}
	}

	libere();

	if (MustBeValid && ind2 != nil)
	{
		if (! ind2->AskForValid(context))
		{
			ind2->Release();
			ind2 = nil;
		}
	}

	return(ind2);
}




									
									/* -----------------------------------------------  */



void Critere::setCRD( const CritereDISK *crd1)
{ 
	*CRD=*crd1;
};


void Critere::GetUUID(VUUID& outID) const
{
	outID.FromBuffer(CRD->ID);
}


void Critere::SetUUID(const VUUID& inID)
{
	inID.ToBuffer(CRD->ID);
}


void Critere::GetName(VString& s) const
{ 
	s.FromBlock( &CRD->nom[1], CRD->nom[0] * sizeof( UniChar), VTC_UTF_16);
}


VError Critere::SetName(const VString& s)
{
	s.ToBlock( CRD->nom, sizeof( CRD->nom), VTC_UTF_16, false, true);
	
	return VE_OK;
}


void Critere::CalcDependences(Table* fic, sLONG numfield)
{
}



																								/* ------------ */
/*																								
VVariableKind CritereAlpha::GetLangKind(void)
{
	return eVK_Class;
}


CClass* CritereAlpha::GetLangKindClass(void)
{
	return VDBMgr::GetLanguage()->GetKernelClass( eKC_String );
}
	

VVariableKind CritereAlphaUTF8::GetLangKind(void)
{
	return eVK_Class;
}


CClass* CritereAlphaUTF8::GetLangKindClass(void)
{
	return VDBMgr::GetLanguage()->GetKernelClass( eKC_String );
}
*/

/*
sLONG CritereAlpha::GetMaxLen()
{
	return ((CRD.lenalpha<<1)+4+3)&(-4);
}
*/


																								/* ------------ */

/*
VVariableKind CritereFloat::GetLangKind(void)
{
	return eVK_Float;
}


CClass* CritereFloat::GetLangKindClass(void)
{
	return NULL;
}
*/				
/*
sLONG CritereFloat::GetMaxLen()
{
	return 12;
}

*/
																								/* ------------ */

/*
VVariableKind CritereReel::GetLangKind(void)
{
	return eVK_Real;
}


CClass* CritereReel::GetLangKindClass(void)
{
	return NULL;
}
*/

/*
sLONG CritereReel::GetMaxLen()
{
	return sizeof(freal);
}
*/

																								/* ------------ */

/*
VVariableKind CritereShort::GetLangKind(void)
{
	return eVK_Long;
}


CClass* CritereShort::GetLangKindClass(void)
{
	return NULL;
}
*/				
/*
sLONG CritereShort::GetMaxLen()
{
	return sizeof(sWORD);
}
*/

																								/* ------------ */

/*
VVariableKind CritereLong::GetLangKind(void)
{
	return eVK_Long;
}


CClass* CritereLong::GetLangKindClass(void)
{
	return NULL;
}
*/				
/*
sLONG CritereLong::GetMaxLen()
{
	return sizeof(sLONG);
}
*/

																								/* ------------ */

/*
VVariableKind CritereLong8::GetLangKind(void)
{
	return eVK_Long8;
}


CClass* CritereLong8::GetLangKindClass(void)
{
	return NULL;
}
*/				
/*
sLONG CritereLong8::GetMaxLen()
{
	return 8;
}
*/

																								/* ------------ */

/*
VVariableKind CritereMoney::GetLangKind(void)
{
	return eVK_Float;
}


CClass* CritereMoney::GetLangKindClass(void)
{
	return NULL;
}
*/

/*
sLONG CritereMoney::GetMaxLen()
{
	return 20;
}
*/

																								/* ------------ */

/*
VVariableKind CritereDuration::GetLangKind(void)
{
	return eVK_Class;
}


CClass* CritereDuration::GetLangKindClass(void)
{
	return VDBMgr::GetLanguage()->GetKernelClass( eKC_Duration );
}
*/				
/*
sLONG CritereDuration::GetMaxLen()
{
	return(8);
}
*/

																								/* ------------ */
																								
/*
VVariableKind CritereTime::GetLangKind(void)
{
	return eVK_Class;
}


CClass* CritereTime::GetLangKindClass(void)
{
	return VDBMgr::GetLanguage()->GetKernelClass( eKC_Time );
}
*/				
/*
sLONG CritereTime::GetMaxLen()
{
	return(8);
}
*/

																								/* ------------ */

/*
VVariableKind CritereBool::GetLangKind(void)
{
	return eVK_Boolean;
}


CClass* CritereBool::GetLangKindClass(void)
{
	return NULL;
}
*/				
/*
sLONG CritereBool::GetMaxLen()
{
	return(1);
}
*/


																								/* ------------ */

/*
VVariableKind CritereByte::GetLangKind(void)
{
	return eVK_Byte;
}


CClass* CritereByte::GetLangKindClass(void)
{
	return NULL;
}
*/

																								/* ------------ */

/*
VVariableKind CritereUUID::GetLangKind(void)
{
	return eVK_Class;
}


CClass* CritereUUID::GetLangKindClass(void)
{
	return VDBMgr::GetLanguage()->GetKernelClass( eKC_String );
}
*/				
/*
sLONG CritereUUID::GetMaxLen()
{
	return 16;
}
*/

																								/* ------------ */
/*
VVariableKind CritereText::GetLangKind(void)
{
	return eVK_Class;
}


CClass* CritereText::GetLangKindClass(void)
{
	return VDBMgr::GetLanguage()->GetKernelClass( eKC_String );
}
*/				

																								/* ------------ */

/*
VVariableKind CritereImage::GetLangKind(void)
{
	return eVK_Class;
}


CClass* CritereImage::GetLangKindClass(void)
{
	
	// return VDBMgr::GetLanguage()->GetKernelClass( eKC_Image );
	// Il faudra le faire ailleurs
	
	return VDBMgr::GetLanguage()->GetKernelClass( eKC_Blob );
}
*/				

																								/* ------------ */
/*
VVariableKind CritereBlob::GetLangKind(void)
{
	return eVK_Class;
}


CClass* CritereBlob::GetLangKindClass(void)
{
	return VDBMgr::GetLanguage()->GetKernelClass( eKC_Blob );
}
*/
				
/*
sLONG CritereBlob::GetMaxLen()
{
	return sizeof(sLONG);
}
*/

void CritereBlob::CalcDependences(Table* fic, sLONG numfield)
{
	fic->AddBlobDep(numfield);
}

																								/* ------------ */


CodeReg *cri_CodeReg;

void InitCritere()
{
	assert(cri_CodeReg ==nil);
	cri_CodeReg = new CodeReg;
	RegisterCri( DB4D_StrFix,CritereAlpha::NewCritere);
	RegisterCri( DB4D_Text, CritereText::NewCritere);
	RegisterCri( DB4D_Real, CritereReel::NewCritere);
	RegisterCri( DB4D_Float, CritereFloat::NewCritere);
	RegisterCri( DB4D_Duration, CritereDuration::NewCritere);
	RegisterCri( DB4D_Time, CritereTime::NewCritere);
	RegisterCri( DB4D_Integer16, CritereShort::NewCritere);
	RegisterCri( DB4D_Integer32, CritereLong::NewCritere);
	RegisterCri( DB4D_Boolean, CritereBool::NewCritere);
	RegisterCri( DB4D_Byte, CritereByte::NewCritere);
	//RegisterCri( DB4D_Date, CritereDate::NewCritere);
	RegisterCri( DB4D_Integer64, CritereLong8::NewCritere);
	//RegisterCri( DB4D_Money, CritereMoney::NewCritere);
	RegisterCri( DB4D_Picture, CritereImage::NewCritere);
	RegisterCri( DB4D_Blob, CritereBlob::NewCritere);
	RegisterCri( DB4D_SubTable, CritereSubTable::NewCritere);
	RegisterCri( DB4D_SubTableKey, CritereSubTableKey::NewCritere);
	RegisterCri( DB4D_UUID, CritereUUID::NewCritere);
	RegisterCri( VK_BLOB_DB4D, CritereBlob::NewCritere);
	RegisterCri( VK_STRING_UTF8, CritereAlphaUTF8::NewCritere);
	RegisterCri( VK_TEXT_UTF8, CritereTextUTF8::NewCritere);
	//RegisterCri( VK_STRING_DB4D, CritereAlpha::NewCritere);
}


void DeInitCritere()
{
	if (cri_CodeReg != nil) {
		delete cri_CodeReg;
		cri_CodeReg = nil;
	}
}

																								/* ------------ */


Critere* CreateCritere( const CritereDISK *criD, CritereDISK *crd)
{
	CreCritere_Code Code = FindCri(criD->typ);
	Critere *cri= (Code != NULL) ? (*Code)(crd) : NULL;
	if (cri != nil)
		cri->setCRD(criD);
	return cri;
}


Critere* CreateCritere(sLONG typ, CritereDISK *crd)
{
	CreCritere_Code Code=FindCri(typ);
	Critere *cri= (Code != NULL) ? (*Code)(crd) : NULL;
	
	return(cri);
}



				/* -----------------------------------------------  */
								 

#if autocheckobj
uBOOL Table::CheckObjInMem(void)
{
	sLONG i;
	
	CheckAssert(IsValidFastPtr(this));
	
	CheckAssert(FID.nbcrit>=0 && FID.nbcrit<MaxFields);
	for (i = 1; i<= FID.nbcrit; i++)
	{
		CheckAssert(IsValidFastPtr(tc[i]));
	}
	
	return(true);
}
#endif


Boolean Table::CanRegister() 
{ 
	return Owner->CanRegister(); 
}


#if debuglr == 111
sLONG Table::Retain(const char* DebugInfo) const
{
	return IRefCountable::Retain(DebugInfo);
}

sLONG Table::Release(const char* DebugInfo) const
{
	return IRefCountable::Release(DebugInfo);
}
#endif


void Table::Touch() 
{ 
	if (fTableCanBesaved)
	{
		if (testAssert(!fIsRemote))
		{
			fStamp++; 
			if (fStamp == 0)
				fStamp = 1;
			if (Owner != nil) 
				Owner->Touch(); 
		}
	}
}



MapOfEmptyVal Table::sEmptyVals;

const VValueSingle* Table::xGetEmptyVal(sLONG typ)
{
	MapOfEmptyVal::iterator found = sEmptyVals.find(typ);
	if (found == sEmptyVals.end())
	{
		xEmptyVal val;
		val.fEmptyVal = CreVValueFromNil( typ, -1, false);
		if (val.fEmptyVal == nil)
			val.fEmptyPtr = nil;
		else
		{
			val.fEmptyPtr = GetFastMem(30, false, 'valE');
			if (typ == VK_TEXT)
				*((sLONG*)val.fEmptyPtr) = -1;
			else
				val.fEmptyVal->WriteToPtr(val.fEmptyPtr);
		}
		sEmptyVals.insert(make_pair(typ,val));
		return val.fEmptyVal;
	}
	else
	{
		return found->second.fEmptyVal;
	}
}


const void* Table::xGetEmptyPtr(sLONG typ)
{
	MapOfEmptyVal::iterator found = sEmptyVals.find(typ);
	if (found == sEmptyVals.end())
	{
		xEmptyVal val;
		val.fEmptyVal = CreVValueFromNil( typ, -1, false);
		if (val.fEmptyVal == nil)
			val.fEmptyPtr = nil;
		else
		{
			val.fEmptyPtr = GetFastMem(30, false, 'valE');
			if (typ == VK_TEXT)
				*((sLONG*)val.fEmptyPtr) = -1;
			else
				val.fEmptyVal->WriteToPtr(val.fEmptyPtr);
		}
		sEmptyVals.insert(make_pair(typ,val));
		return val.fEmptyPtr;
	}
	else
	{
		return found->second.fEmptyPtr;
	}
}



VError Table::WriteSeqNumToLog(BaseTaskInfo* context, sLONG8 curnum)
{
	VError err = VE_OK;
	VStream* log;
	Base4D* db = GetOwner();
	sLONG len = 8 /*curnum*/ + sizeof(VUUIDBuffer) /*numtable*/;

	err = db->StartWriteLog(DB4D_Log_SaveSeqNum, len, context, log);
	if (err == VE_OK)
	{
		if (log != nil)
		{
			err = log->PutLong8(curnum);
			if (err == VE_OK)
			{
				VUUID xid;
				GetUUID(xid);
				err = xid.WriteToStream(log);
			}
		}
		VError err2 = db->EndWriteLog(len);
		if (err == VE_OK)
			err = err2;
	}

	return err;
}


VError Table::RetainIndexes(ArrayOf_CDB4DIndex& outIndexes)
{
	outIndexes.ReduceCount(0);
	VObjLock lock(this);

	if (outIndexes.SetAllocatedSize(IndexDep.GetCount()))
	{
		for (IndexArrayIncluded::Iterator cur = IndexDep.First(), end = IndexDep.End(); cur != end; cur++)
		{
			IndexInfo* ind = *cur;
			if (ind != nil)
			{
				VDB4DIndex* xind = new VDB4DIndex(ind);
				if (xind == nil)
					return ThrowError(memfull, DBaction_BuildingListOfIndexes);
				else
				{
					outIndexes.Add(xind);
				}
			}
		}
	}
	else
		return ThrowError(memfull, DBaction_BuildingListOfIndexes);

	return VE_OK;
}


void Table::GetFieldsStamps(StampsVector& outStamps) const
{
	occupe();
	outStamps.clear();
	try
	{
		outStamps.reserve(tc.GetCount()+1);
		for (FieldArray::ConstIterator cur = tc.First(), end = tc.End(); cur != end; cur++)
		{
			if (*cur == nil)
				outStamps.push_back(0);
			else
				outStamps.push_back((*cur)->GetStamp());
		}
	}
	catch (...)
	{
	}

	libere();
}


void Table::SetAtLeastOneAutoSeq(Field* cri) 
{ 
	occupe();
	AutoSeqDep.Add(cri->GetPosInRec());
	libere(); 
}


void Table::RecalcAtLeastOneAutoSeq()
{
	occupe();
	sLONG i,nb = tc.GetCount();

	AutoSeqDep.SetCount(0);
	for (i = 1; i <= nb; i++)
	{
		Field* cri = tc[i];
		if (cri != nil)
		{
			if (cri->GetAutoSeq())
			{
				AutoSeqDep.Add(i);
			}
		}
	}
	libere();
}



void Table::SetAtLeastOneAutoGenerate(Field* cri) 
{ 
	occupe();
	AutoGenerateDep.Add(cri->GetPosInRec());
	libere(); 
}


void Table::RecalcAtLeastOneAutoGenerate()
{
	occupe();
	sLONG i,nb = tc.GetCount();
	
	AutoGenerateDep.SetCount(0);
	for (i = 1; i <= nb; i++)
	{
		Field* cri = tc[i];
		if (cri != nil)
		{
			if (cri->GetAutoGenerate())
			{
				AutoGenerateDep.Add(i);
			}
		}
	}
	libere();
}



void Table::SetAtLeastOneUnique(Field* cri) 
{ 
	occupe();
	fAtLeastOneUnique = true;
	UniqueDep.Add(cri->GetPosInRec());
	libere(); 
}


void Table::RecalcAtLeastOneUnique()
{
	occupe();
	sLONG i,nb = tc.GetCount();
	Boolean b = false;

	UniqueDep.SetCount(0);
	for (i = 1; i <= nb; i++)
	{
		Field* cri = tc[i];
		if (cri != nil)
		{
			if (cri->GetUnique())
			{
				b = true;
				UniqueDep.Add(i);
			}
		}
	}
	fAtLeastOneUnique = b;
	libere();

}


void Table::SetAtLeastOneNot_Null(Field* cri) 
{ 
	occupe();
	fAtLeastOneNot_Null = true;
	Not_NullDep.Add(cri->GetPosInRec());
	libere(); 
}


void Table::RecalcAtLeastOneNot_Null()
{
	occupe();
	sLONG i,nb = tc.GetCount();
	Boolean b = false;

	Not_NullDep.SetCount(0);
	for (i = 1; i <= nb; i++)
	{
		Field* cri = tc[i];
		if (cri != nil)
		{
			if (cri->GetNot_Null())
			{
				b = true;
				Not_NullDep.Add(i);
			}
		}
	}
	fAtLeastOneNot_Null = b;
	libere();
}


void Table::RecalcAtLeastOneSubTable()
{
	occupe();
	sLONG i,nb = tc.GetCount();

	SubTableDep.SetCount(0);
	for (i = 1; i <= nb; i++)
	{
		Field* cri = tc[i];
		if (cri != nil)
		{
			if (cri->GetTyp() == DB4D_SubTable)
			{
				SubTableDep.Add(i);
			}
		}
	}
	libere();
}


Boolean Table::CheckUniqueness(FicheInMem* fic, const NumFieldArray& deps, BaseTaskInfo* context, sLONG beginprimkey)
{
	VError err = VE_OK;;
	Boolean result = false;

	if (testAssert(!fIsRemote))
	{
		if (!Owner->CheckUniquenessEnabled())
		{
			result = true;
		}
		else
		{
			SearchTab rech(this, true);
			sLONG i,nb = deps.GetCount();
			Boolean	deja = false, avecparent = false;
			if (beginprimkey == 0)
				beginprimkey = nb+1;
			for (i = 1; i <= nb && err == VE_OK; i++)
			{
				ValPtr cv = fic->GetNthField(deps[i], err, false, true);
				if (cv != nil)
				{
					if (i >= beginprimkey)
					{
						if (i == beginprimkey)
						{
							if (deja)
							{
								rech.AddSearchLineBoolOper(DB4D_OR);
								avecparent = true;
								rech.AddSearchLineOpenParenthesis();
							}
						}
						else
						{
							rech.AddSearchLineBoolOper(DB4D_And);
						}
					}
					else
					{
						if (deja)
							rech.AddSearchLineBoolOper(DB4D_OR);
						else
							deja = true;
					}
					rech.AddSearchLineSimple(GetNum(), deps[i], DB4D_Equal, cv);
				}
			}
			if (avecparent)
				rech.AddSearchLineCloseParenthesis();

			OptimizedQuery query;
			if (err == VE_OK)
				err = query.AnalyseSearch(&rech, context);
			if (err == VE_OK)
			{
				Selection *sel = query.Perform((Bittab*)nil, nil, context, err, DB4D_Do_Not_Lock);
				if (sel != nil)
				{
					if (sel->GetQTfic() == 0)
					{
						result = true;
					}
					else
					{
						if (sel->GetQTfic() == 1)
						{
							if (sel->GetFic(0) == fic->GetNum())
								result = true;
						}
					}
					sel->Release();
				}
			}
		}
	}

	return result;
}

/*
static CDB4DBaseContextPtr GetContextFromTable(CLanguageContext* inContext, Table* table)
{
	CLanguage* sLanguage = VDBMgr::GetLanguage();
	CDB4DBaseContextPtr xcontext = nil;

	CDB4DContext* context = (CDB4DContext*)inContext->GetCustomData(0);
	if (testAssert(context != nil))
	{
		CDB4DBase* xbase = table->GetOwner()->RetainBaseX("GetContextFromTable");
		xcontext = context->RetainDataBaseContext(xbase, true);
		xbase->Release("GetContextFromTable");
		assert(xcontext != nil);
	}

	return xcontext;
}
*/


#if 0
VError Table::RecordPushMember (CLanguageContext* inContext, VClassInstance& ioClassInstance, CVariable* inVariable, StackPtr* ioStack)
{
	VError err = VE_OK;
	CDB4D_Lang* lang4D = VDBMgr::GetLang4D();
	Boolean okpushed = false;
	sLONG nc = inVariable->GetID();
	CDB4DRecord* rec = (CDB4DRecord*)ioClassInstance;
	VValueSingle *cv = nil;
	FicheInMem *fiche;
	
	if (nc > 0)
	{
		if (rec != nil)
		{
			cv = rec->GetFieldValue(nc, false, &err);
			if (cv != nil)
			{
				inContext->PushVariableFromValue(ioStack, inVariable, cv);
				okpushed = true;
			}
		}

		if (!okpushed && err == VE_OK)
		{
			CreVValue_Code Code;
			Field* cri = nil;
			FicheInMem *fic;
			if (rec != nil)
			{
				fic = (FicheInMem*)rec->GetFicheInMem();
				Table* assoctable = fic->GetDF()->RetainTable();
				if (assoctable != nil)
				{
					cri = assoctable->RetainField(nc);
					assoctable->Release();
				}
			}
			if (cri != nil)
			{
				Code=FindCV(cri->GetTyp());
				if (Code == nil)
					err = fic->ThrowError(VE_DB4D_FIELDDEFCODEMISSING, DBaction_AccessingField);
				else
				{
					cv=(*Code)(fic->GetDF(),cri->GetLimitingLen(),nil,false,false,NULL);
					cri->Release();
					if (cv == nil)
						err = fic->ThrowError(memfull, DBaction_AccessingField);
				}
			}
			else
			{
				cv = new VString();
				if (cv == nil)
					err = fic->ThrowError(memfull, DBaction_AccessingField);
			}
			
			if (cv != nil)
			{
				cv->SetNull(true);
				inContext->PushVariableFromValue(ioStack, inVariable, cv);
				delete cv;
			}
		}
	}
	else
	{
		if (nc == -2 || nc == -3) // c'est une relation
		{
			if (rec != nil)
			{
				FicheInMem* fic = (FicheInMem*)rec->GetFicheInMem();	
				Base4D* bd = fic->GetOwner()->GetOwner();
				Relation* rel = bd->FindAndRetainRelationByRelVar(inVariable);
				if (rel!=nil)
				{
					ContextAuto context(GetContextFromTable(inContext, fic->GetOwner()));
					CDB4DBase* xbase = bd->RetainBaseX("RecordPushMember");
					CDB4DRelation* xrel = new VDB4DRelation(xbase, rel);
					xbase->Release("RecordPushMember");
					if (rel->GetRelVar1toN() == inVariable) // lien retour
					{
						CDB4DSelection*  sel = nil;
						err = xrel->ActivateOneToMany(rec, sel, context.GetContext());
						lang_selection4d *xsel = new lang_selection4d(sel);	
						inContext->PushClassInstance(ioStack, (VClassInstance)xsel, inVariable);
					}
					else
					{
						if (rel->GetRelVarNto1() == inVariable) // lien aller
						{	
							CDB4DRecord* result;
							err = xrel->ActivateManyToOne(rec, result, context.GetContext());
							inContext->PushClassInstance(ioStack, (VClassInstance)result, inVariable);
						}
					}

					xrel->Release();
					rel->Release();
				}
			}
			else
				inContext->PushClassInstance(ioStack, (VClassInstance)nil, inVariable, true);
		}
	}
	return(err);
}
#endif



Boolean Table::IsNotifying() 
{ 
	return fIsNotifying || Owner->IsNotifying(); 
}

#if 0
VError Table::RecordPopMember (CLanguageContext* inContext, VClassInstance& ioClassInstance, VVariableStamp* ptStamp, CVariable* inVariable, StackPtr* ioStack)
{
	VError err = VE_OK;
	Boolean pasfait = true;
	
	sLONG nc = inVariable->GetID();
	if (nc > 0)
	{
		CDB4DRecord* rec = (CDB4DRecord*)ioClassInstance;
		
		if (rec != nil)
		{
			VValueSingle *cv = rec->GetFieldValue(nc, false, &err);
			if (cv != nil)
			{
				inContext->PopVariableAndSetValue(ioStack, inVariable, cv);
				rec->Touch(nc);
				pasfait = false;
			}
		}
	}
	
	if (pasfait)
	{
		// retire de la stack la valeur
		VValueSingle* value = inContext->PopVariableAndCreateValue(ioStack, inVariable);
		delete value;
	}
	return(err);
}
#endif


#if 0
VError Table::TablePushMember (CLanguageContext* inContext, VClassInstance& ioClassInstance, CVariable* inVariable, StackPtr* ioStack)
{
	VError err = VE_OK;
	CDB4D_Lang* lang4D = VDBMgr::GetLang4D();
	sLONG nc = inVariable->GetID();
	CDB4DTable* table = (CDB4DTable*)ioClassInstance;
	
	if (table != nil)
	{
		if (nc < 0)
		{
			if (nc == -1)
			{
				CDB4DRecord* currec = (*(VDBMgr::GetManager()->Get_GetCurrentRecordFromContextMethod()))(inContext, table->GetID());
				inContext->PushClassInstance(ioStack, (VClassInstance)currec, inVariable, currec == nil);
			}
			if (nc == -2)
			{
				void* cursel = (*(VDBMgr::GetManager()->Get_GetCurrentSelectionFromContextMethod()))(inContext, table->GetID());
				inContext->PushClassInstance(ioStack, (VClassInstance)cursel, inVariable, cursel == nil);
			}
		}
		else
		{
			CDB4DField* field = table->RetainNthField(nc);
			inContext->PushClassInstance(ioStack, (VClassInstance)field, inVariable, false);
		}
	}
	else
	{
		inContext->PushClassInstance(ioStack, (VClassInstance)nil, inVariable, true);
	}
	return(err);
}
#endif


#if 0
VError Table::TablePopMember (CLanguageContext* inContext, VClassInstance& ioClassInstance, VVariableStamp* ptStamp, CVariable* inVariable, StackPtr* ioStack)
{
	sLONG nc = inVariable->GetID();
	Boolean isNull;
	CVariable* var;
	CDB4DField* dummy = (CDB4DField*)inContext->PopClassInstance( ioStack, isNull, var );
	if (nc >= 0)
	{
		if (dummy != nil)
			dummy->Release();
	}
	return(VE_OK);
}
#endif

// CNameSpace* Table::GetNameSpace(void) { return Owner->GetNameSpace(); };

/*
VError Table::FakeRecCreatorMethod (CLanguageContext* inContext, CMethod* inMethod, StackPtr inStack)
{
	CDB4DBase *xbd;
	VError err;
	VVariableStamp* stamp;

	err = inMethod->RetainThisClassInstance(inContext, inStack, (VClassInstance&) xbd, stamp);
	if ( VE_OK == err )
	{
		if (xbd != nil)
		{
			Base4D* b = VImpCreator<VDB4DBase>::GetImpObject(xbd)->GetBase();
			Table* t = b->FindAndRetainTableByRecordCreator(inMethod);
			if (t != nil)
			{
				sLONG nbparm = inMethod->GetNbPassedParameters(inStack);
				sLONG i;
				SearchTab query(t);
				for (i = 0; i < nbparm; i++)
				{
					Field* cri = t->RetainField(i+1);
					if (cri != nil)
					{
						VValueSingle* cv = inMethod->CreateValueFromVariantParameter(inStack, i);
						if (cv != nil)
						{
							query.AddSearchLineSimple(cri,DB4D_Equal,cv);
							if (i < (nbparm-1))
								query.AddSearchLineBoolOper(DB4D_And);
							delete cv;
						}
						cri->Release();
					}
				}
				OptimizedQuery q;

				BaseTaskInfo* context = nil;
				CDB4DBaseContext* xcontext = nil;

				CDB4DContext* globcontext = (CDB4DContext*)inContext->GetCustomData(ID_4D_Lang);
				if (globcontext != nil)
				{
					CDB4DBase* xbase = b->RetainBaseX("FakeRecCreatorMethod");
					CDB4DBaseContext* xcontext = globcontext->RetainDataBaseContext(xbase);
					xbase->Release("FakeRecCreatorMethod");
					if (xcontext != nil)
						context = VImpCreator<VDB4DBaseContext>::GetImpObject(xcontext)->GetBaseTaskInfo();
				}

				if (context != nil)
					err = q.AnalyseSearch(&query, context);

				if (err == VE_OK)
				{

					if (context != nil)
					{
						Selection* sel = q.Perform((Bittab*)nil, nil, context, err, false);
						if (sel != nil)
						{
							if (sel->GetQTfic() > 0)
							{
								FicheInMem* rec = t->GetDF()->LoadRecord(sel->GetFic(0), err, false, context, true, false);
								if (rec == nil)
									rec = new FicheInMem(context, b, t, err);
								if (rec != nil)
								{
									CDB4DRecord* xrec = new VDB4DRecord(VDBMgr::GetManager(), rec, xcontext);
									CDB4DRecord* *rd = (CDB4DRecord**)inMethod->GetReturnedClassInstancePtr(inStack);
									CHECK_RETURN_VALUE_NIL(rd);
									*rd = xrec;
									inMethod->ReturnValue(inStack);
								}
							}
							sel->Release();
						}
					}
				}

				t->Release();
			}
		}
	}
	
	return err;
}
*/


void Table::RegisterForLang(Boolean inWithFields)
{
//	CLanguage* sLanguage = VDBMgr::GetLanguage();
	//CDB4D_Lang* lang4D = VDBMgr::GetLang4D();
	occupe();
#if 0
	if ( (lang4D != nil) && (sLanguage != nil)  )
	{
		if ((Owner->GetNameSpace() != nil))
		{
			VString	s(L"Record of ");
			VString srec;
			GetRecordName(srec);
			if (srec.IsEmpty())
			{
				GetName(srec);
				s += srec;
			}
			else
				s = srec;
			/*
			if (FID.RecordName[0] == 0)
			{
				s.AppendBlock(&FID.nom[1],FID.nom[0] * sizeof( UniChar), VTC_UTF_16);
			}
			else
			{
				GetRecordName(s);
			}
			*/

			VString s3;
			GetName(s3);

			VString	s2(L"Table ");
			s2 += s3;

			VString	s4(L"Selection of ");
			s4 += s3;

			/*
			VString	s2(L"Table ");
			s2.AppendBlock(&FID.nom[1],FID.nom[0] * sizeof( UniChar), VTC_UTF_16);

			VString	s3;
			s3.AppendBlock(&FID.nom[1],FID.nom[0] * sizeof( UniChar), VTC_UTF_16);

			VString	s4(L"Selection of ");
			s4.AppendBlock(&FID.nom[1],FID.nom[0] * sizeof( UniChar), VTC_UTF_16);
			*/

			if (TableRef == nil)
			{
				VUUID xUID;
				VUUIDBuffer buf;
				sLONG i,nb;
				
				VCreatorMethod creatorMethod;
				VDestructorMethod destructorMethod;
				VPushMemberMethod pushMemberMethod;
				VPopMemberMethod popMemberMethod;
				VPushArrayElementMethod pushArrayElementMethod;
				VPopArrayElementMethod popArrayElementMethod;
				VGetArrayCountMethod getArrayCountMethod;

				lang4D->GetRecordClassRef()->GetMethods( creatorMethod, destructorMethod, pushMemberMethod, popMemberMethod );

				GetUUID(xUID);
				RecordRef = Owner->GetNameSpace()->CreateClass( s, xUID, lang4D->GetRecordClassRef(), creatorMethod, destructorMethod, RecordPushMember, RecordPopMember );
				RecordRef->SetCanEditDefaultValue(false, false);
				RecordRef->AllowDynamicCast(true);

				lang4D->GetTableClassRef()->GetMethods( creatorMethod, destructorMethod, pushMemberMethod, popMemberMethod );

				xUID.ToBuffer(buf);
				buf.IncFirst8();
				xUID.FromBuffer(buf);
				
				TableRef = Owner->GetNameSpace()->CreateClass(s2, xUID, lang4D->GetTableClassRef(), creatorMethod, destructorMethod, TablePushMember, TablePopMember );
				TableRef->SetCanEditDefaultValue(false, false);
				TableRef->AllowDynamicCast(true);
				
				// a l'ouverture de la base, le context de la library n'est pas encore cree
				// c'est dommage mais il faut creer un base context supplementaire

				CDB4DBase* xxbase = Owner->RetainBaseX("Table::RegisterForLang");
				VDB4DBase* xbase = VImpCreator<VDB4DBase>::GetImpObject(xxbase);
				VDB4DTable *xtable = new VDB4DTable(VDBMgr::GetManager(), xbase, this);
				CDB4DTable *xxtable = xtable;
				xxbase->Release("Table::RegisterForLang");
				
				TableRef->SetCustomData(xxtable);

				buf.IncFirst8();
				xUID.FromBuffer(buf);

				TableVar = Owner->GetBaseRef()->AddVariable( s3, xUID, eVK_Class, TableRef, NULL, numfile );

				//buf.IncFirst8();
				//xUID.FromBuffer(buf);

				// FM : Pas d'ID pour cette variable ???? bug ???
				//TableGVar = Owner->GetNameSpace()->GetNameSpaceClass()->AddVariable( s3, xUID, eVK_Class, TableRef, numfile );
				// on ne cree plus une variable avec le nom de la table dans le namespace

				lang4D->GetSelectionClassRef()->GetMethods(creatorMethod, destructorMethod, pushMemberMethod, popMemberMethod );
				lang4D->GetSelectionClassRef()->GetArrayMethods( pushArrayElementMethod, popArrayElementMethod, getArrayCountMethod );

				buf.IncFirst8();
				xUID.FromBuffer(buf);

				SelectionRef = Owner->GetNameSpace()->CreateClass(s4, xUID, lang4D->GetSelectionClassRef(), creatorMethod, destructorMethod, pushMemberMethod, popMemberMethod );
				SelectionRef->SetCanEditDefaultValue(false, false);
				SelectionRef->AllowDynamicCast(true);
				SelectionRef->SetArrayDefinition( eVK_Class, RecordRef );
				SelectionRef->SetArrayMethods( pushArrayElementMethod, popArrayElementMethod, getArrayCountMethod );

				/*
				buf.IncFirst8();
				xUID.FromBuffer(buf);
				fFakeRecCreator = Owner->GetNameSpace()->GetNameSpaceClass()->AddMethod(s, xUID, FakeRecCreatorMethod);
				fFakeRecCreator->AddParameter( CVSTR( "data" ), eVK_Variant );
				fFakeRecCreator->SetNbVariableParameters( 1 );
				fFakeRecCreator->SetReturnValue( eVK_Class, RecordRef );
				*/

				buf.IncFirst8();
				xUID.FromBuffer(buf);
				CVariable* TableCurrentRecordVar = TableRef->AddVariable(L"Current Record", xUID, eVK_Class, RecordRef, NULL, -1);
				TableCurrentRecordVar->Release();

				buf.IncFirst8();
				xUID.FromBuffer(buf);
				CVariable* TableCurrentSelectionVar = TableRef->AddVariable(L"Current Selection", xUID, eVK_Class, SelectionRef, NULL, -2);
				TableCurrentSelectionVar->Release();
			}
			else
			{
				SelectionRef->SetName(s4);
				RecordRef->SetName(s);
				//fFakeRecCreator->SetName(s);
				TableRef->SetName(s2);
				TableVar->SetName(s3);
				//TableGVar->SetName(s3);
			}
		}
	}
#endif
	if (Owner->CanRegister())
	{
		if (!Owner->IsNotifying())
		{
			if (VDBMgr::GetCommandManager() != nil)
			{
				VUUID UID_base = Owner->GetUUID();
				
				VUUID UID_table;
				GetUUID( UID_table);
				
				if (fRegistered)
				{
					VDBMgr::GetCommandManager()->Tell_UpdateTable( UID_base, UID_table, numfile, TUT_NameChanged);
				}
				else
				{
					VDBMgr::GetCommandManager()->Tell_AddTable( UID_base, UID_table, numfile);
				}
			}
		}

		fRegistered = true;
		if (inWithFields)
		{
			sLONG i,nb = tc.GetCount();
			
			for (i = 1; i <= nb; i++)
			{
				Field* cri = tc[i];
				if (cri != nil)
					cri->RegisterForLang();
			}
		}
	}

	libere();
}


void Table::UnRegisterForLang(void)
{
	//CLanguage* sLanguage = VDBMgr::GetLanguage();
	//CDB4D_Lang* lang4D = VDBMgr::GetLang4D();
	sLONG i,nb;
	
	occupe();
	SetNotifyState(true);
	
	if (fRegistered)
	{
		fRegistered = false;

		if (!Owner->IsNotifying())
		{
			if (VDBMgr::GetCommandManager() != nil)
			{
				VUUID UID_base = Owner->GetUUID();
				
				VUUID UID_table;
				GetUUID( UID_table);

				VDBMgr::GetCommandManager()->Tell_DelTable( UID_base, UID_table, numfile);
			}
		}

		nb = tc.GetCount();
		for (i=1; i<=nb; i++)
		{
			Field *cri;
			cri = tc[i];
			if (cri != nil)
			{
				cri->UnRegisterForLang();
			}
			
		}
	}
#if 0
	if (TableRef != nil)
	{
		if ( (lang4D != nil) && (sLanguage != nil) )
		{
			/*
			TableGVar->Drop();
			TableGVar->Release();
			TableGVar = nil;
			*/

			CDB4DTable* xxtable = (CDB4DTable*)TableRef->GetCustomData();
			if (xxtable != nil)
				xxtable->Release();
			
			TableVar->Drop();
			TableVar->Release();
			TableVar = nil;
			

			TableRef->Drop();
			TableRef->Release();
			TableRef = nil;

			RecordRef->Drop();
			RecordRef->Release();
			RecordRef = nil;

			SelectionRef->Drop();
			SelectionRef->Release();
			SelectionRef = nil;

			/*
			fFakeRecCreator->Drop();
			fFakeRecCreator->Release();
			fFakeRecCreator = nil;
			*/
						
		}
	}
#endif
	SetNotifyState(false);
	libere();
}


void Table::AddIndexDep(IndexInfo* ind)
{
	occupe();
	ind->Retain();
	IndexDep.Add(ind);
	libere();
}

void Table::DelIndexDep(IndexInfo* ind)
{
	sLONG i;
	
	occupe();
	for (i=1;i<=IndexDep.GetCount();i++)
	{
		if (ind==IndexDep[i])
		{
			IndexDep.DeleteNth(i);
			break;
		}
	}
	libere();
}

void Table::ClearIndexDep(void)
{
	occupe();
	IndexDep.SetAllocatedSize(0);
	libere();
}


VError Table::AddBlobDep(sLONG fieldnum)
{
	VError err = VE_OK;
	occupe();
	if (!BlobDep.Add(fieldnum))
		err = ThrowError(memfull, DBaction_AddingBlobDependancy);
	libere();
	
	return err;
}

void Table::DelBlobDep(sLONG fieldnum)
{
	sLONG i;
	
	occupe();
	for (i=1;i<=BlobDep.GetCount();i++)
	{
		if (fieldnum==BlobDep[i])
		{
			BlobDep.DeleteNth(i);
			break;
		}
	}
	libere();
}

void Table::ClearBlobDep(void)
{
	occupe();
	BlobDep.SetCount(0);
	libere();
}


void Table::CalcDependencesField(void)
{
	sLONG nb,j;
	
	occupe();
	
	nb=GetNbCrit();
	ClearBlobDep();
	Not_NullDep.SetCount(0);
	for (j = 1; j <= nb; j++)
	{
		Field* cri = tc[j];
		if (cri != nil)
		{
			if (cri->GetNot_Null())
				Not_NullDep.Add(j);
			cri->CalcDependences(this,j);
		}
	}
	libere();
}


void Table::ReleaseAllFields()
{
	sLONG i,nb;

	occupe();

	nb = tc.GetCount();
	for (i=1;i<=nb;i++)
	{
		Field* cri = tc[i];
		if (cri !=nil)
		{
			/*
			VUUID xid;
			cri->GetUUID(xid);
			Owner->DelObjectID(objInBase_Field, cri, xid);
			*/
			// pas necessaire car appelle uniquement a la fermeture de la base
			cri->Release();
		}
		tc[i] = nil;
	}

	libere();
}


Table::~Table()
{
	sLONG i;
	FieldPtr cri;
								
	//VUUID xid;
	//GetUUID(xid);
	//Owner->DelObjectID(objInBase_Table, this, xid);

	UnRegisterForLang();
	
	for (i=1;i<=tc.GetCount();i++)
	{
		cri = tc[i];
		if (cri !=nil) 
			cri->Release();
	}

	QuickReleaseRefCountable(fPseudoRecNumField);
	
	if (DF != nil)
	{
		DF->ClearAssoc();
		DF->Release();
	}
}



void Table::DelayIndexes()
{
	VTaskLock lock(&fAllIndexDelayRequestCountMutex);
	{
		VObjLock lock(this);
		for (IndexArray::Iterator cur = IndexDep.First(), end = IndexDep.End(); cur != end; cur++)
		{
			if (*cur != nil)
				(*cur)->DelayIndex();
		}
	}
	{
		fAllIndexDelayRequestCount++;
	}
}


void Table::AwakeIndexes(VDB4DProgressIndicator* inProgress, vector<IndexInfo*> *outIndexList)
{
	if (outIndexList != nil)

		outIndexList->clear();
	VTaskLock lock(&fAllIndexDelayRequestCountMutex);

	{
		VObjLock lock(this);
		for (IndexArrayIncluded::Iterator cur = IndexDep.First(), end = IndexDep.End(); cur != end; cur++)
		{
			IndexInfo* ind = *cur;
			if (ind != nil)
			{
				if (outIndexList != nil)
				{
					ind->Retain();
					outIndexList->push_back(ind);
				}
				ind->AwakeIndex(inProgress);
			}
		}
	}
	{
		fAllIndexDelayRequestCount--;
	}
}


Boolean Table::AreIndexesDelayed() const
{
	VTaskLock lock(&fAllIndexDelayRequestCountMutex);
	return fAllIndexDelayRequestCount > 0;
}


void Table::AddOneIndexToDelay(IndexInfo* indexToAdd)
{
	VTaskLock lock(&fAllIndexDelayRequestCountMutex);
	if (fAllIndexDelayRequestCount > 0)
		indexToAdd->SetDelayIndex(fAllIndexDelayRequestCount);
}


Table::Table(Base4D *owner, sLONG xnres, Boolean canBeSaved, Boolean isRemote)
{ 
	sLONG i;
	
	fRemoteMaxRecords = -1;
	//fIsIndexDelayed = false;
	fAllIndexDelayRequestCount = 0;
	fIsNotifying = false;
//	TableVar = nil;
//	TableRef = nil;
//	RecordRef = nil;
	//fFakeRecCreator = nil;
//	SelectionRef = nil;
	Owner = owner;
	fRegistered = false;
	nbuse=0;
	nres=xnres;
	numfile = xnres;
	fParent = nil;
	
	fTableCanBesaved = canBeSaved;
	fAtLeastOneUnique = false;
	fAtLeastOneNot_Null = false;
	DF = nil;
	fStamp = 1;

	fIsRemote = isRemote;
	fPseudoRecNumField = nil;

}


AutoSeqNumber* Table::GetSeqNum(CDB4DBaseContext* inContext) 
{ 
	if (fIsRemote)
	{
		return nil;
	}
	else
	{
		if (DF == nil) 
			return nil; 
		else 
			return DF->GetSeqNum(inContext); 
	}
}


VError Table::ThrowError( VError inErrCode, ActionDB4D inAction) const
{
	Base4D* owner = GetOwner();

	VErrorDB4D_OnTable *err = new VErrorDB4D_OnTable(inErrCode, inAction, owner, numfile);
	VTask::GetCurrent()->PushRetainedError( err);
	
	return inErrCode;
}


void Table::ReCalc(void)
{
	sLONG nb, i;
	Field *pc;
	
	occupe();
	nb = tc.GetCount();
	RecalcAtLeastOneNot_Null();
	RecalcAtLeastOneSubTable();
	RecalcAtLeastOneUnique();
	RecalcAtLeastOneAutoSeq();
	RecalcAtLeastOneAutoGenerate();
	CalcDependencesField();
	libere();
	
}


Field* Table::RetainPseudoField(DB4D_Pseudo_Field_Kind kind)
{
	if (kind == DB4D_Pseudo_Field_RecordNum)
	{
		VObjLock lock(this);
		if (fPseudoRecNumField == nil)
		{
			fPseudoRecNumField = new Field(VK_LONG, 0, this);
		}
		return RetainRefCountable(fPseudoRecNumField);
	}
	else
		return nil;
}


sLONG8 Table::GetSeqRatioCorrector() const
{
	if (DF != nil)
		return DF->GetSeqRatioCorrector();
	else
		return 20;
}



sLONG Table::PosInParent(void)
{
	sLONG i,nb,result = 0;
	Field *pc;
	
	nb = fParent->tc.GetCount();
	for (i = 1; i<=nb ;i++)
	{
		pc = fParent->tc[i];
		if (pc != nil)
		{
			if (pc->GetSubFile() == this) result = i;
			
		}
	}
	
	return(result);
}


void FichierDISK::SwapBytes()
{
	ID.SwapBytes();
	ByteSwap(&nom[0], 32);
	ByteSwap(&RecordName[0], 32);
	ByteSwap(&ExtraAddr);
	ByteSwap(&UnusedAddr1);
	ByteSwap(&UnusedAddr2);
	ByteSwap(&ExtraLen);
	ByteSwap(&UnusedLen1);
	ByteSwap(&UnusedLen2);
	ByteSwap(&typ);
	ByteSwap(&nbcrit);
	ByteSwap(&fStamp);
	ByteSwap(&fSchemaID);
	ByteSwap(&Perms[0], maxPermFile);
}


void CritereDISK::SwapBytes()
{
	ID.SwapBytes();
	ByteSwap(&nom[0], 32);
	ByteSwap(&ExtraAddr);
	ByteSwap(&UnusedLong1);
	ByteSwap(&ReUsedNumber);
	ByteSwap(&UnusedAddr2);
	ByteSwap(&ExtraLen);
	ByteSwap(&LimitingLength);
	ByteSwap(&typ);
	ByteSwap(&fStamp);
}


Boolean Table::IsDataDirect(sLONG numfield) const
{
	VObjLock lock(this);
	Boolean res = false;

	if (numfield>0 && numfield <= tc.GetCount())
	{
		Field* cri = tc[numfield];
		if (cri != nil)
			res = cri->IsDataDirect();
	}
	return res;
}


Boolean Table::IsNeverNull(sLONG numfield) const
{
	VObjLock lock(this);
	Boolean res = false;

	if (numfield>0 && numfield <= tc.GetCount())
	{
		Field* cri = tc[numfield];
		if (cri != nil)
			res = cri->IsNeverNull();
	}
	return res;
}


IndexInfo* Table::FindAndRetainIndexLexico(sLONG nc, Boolean MustBeValid, BaseTaskInfo* context) const
{
	sLONG i;
	sLONG nb;
	IndexInfo *ind,*ind2;

	occupe();
	nb=IndexDep.GetCount();
	ind2=nil;
	for (i=1;i<=nb;i++)
	{
		ind=IndexDep[i];
		if (ind!=nil)
		{
			if (ind->MatchType(DB4D_Index_OnKeyWords))
			{
				if (((IndexInfoFromFieldLexico*)ind)->MatchField(numfile,nc))
				{
					{
						ind2=ind;
						ind2->Retain();
						break;
					}
				}
			}
		}
	}

	libere();

	if (MustBeValid && ind2 != nil)
	{
		if (! ind2->AskForValid(context))
		{
			ind2->Release();
			ind2 = nil;
		}
	}

	return(ind2);
}


IndexInfo* Table::FindAndRetainIndexSimple(sLONG nc, uBOOL sortable, Boolean MustBeValid, BaseTaskInfo* context) const
{
	sLONG i;
	sLONG nb;
	IndexInfo *ind,*ind2;
	
	occupe();
	nb=IndexDep.GetCount();
	ind2=nil;
	for (i=1;i<=nb;i++)
	{
		ind=IndexDep[i];
		if (ind!=nil)
		{
			if (ind->MatchType(DB4D_Index_OnOneField))
			{
				if (((IndexInfoFromField*)ind)->MatchField(numfile,nc))
				{
					if (sortable)
					{
						if (ind->MayBeSorted())
						{
							{
								ind2=ind;
								ind2->Retain();
								break;
							}
						}
					}
					else
					{
						{
							ind2=ind;
							ind2->Retain();
							break;
						}
					}
				}
			}
		}
	}
	
	libere();

	if (MustBeValid && ind2 != nil)
	{
		if (! ind2->AskForValid(context))
		{
			ind2->Release();
			ind2 = nil;
		}
	}

	return(ind2);
}


VError Table::GetListOfTablesForCascadingDelete(set<sLONG>& outSet)
{
	if (outSet.find(GetNum()) == outSet.end())
	{
		outSet.insert(GetNum());
		DepRelationArray rels;

		occupe();
		rels.CopyAndRetainFrom(RefIntegrityDep);
		libere();

		sLONG i,nb = rels.GetCount();

		for (i=1; i <= nb; i++)
		{
			Relation* rel = rels[i];
			if (rel->AutoDeleteRelatedRecords())
			{
				Table* dest = rel->GetSourceTable();
				dest->GetListOfTablesForCascadingDelete(outSet);
			}
		}

	}
	return VE_OK;
}


VError Table::CheckReferentialIntegrity(FicheInMem *fic, BaseTaskInfo* context, Boolean OnModify)
{
	VError err = VE_OK;
	
	if (testAssert(!fIsRemote))
	{
		occupe();
		if (Owner->ReferentialIntegrityEnabled() && AtLeastOneReferentialIntegrity())
		{
			DepRelationArray rels;
			rels.CopyAndRetainFrom(RefIntegrityDep);
			libere();

			sLONG i,nb = rels.GetCount();

			for (i=1; i <= nb; i++)
			{
				Relation* rel = rels[i];
				assert(rel != nil);
				assert(rel->WithReferentialIntegrity());

				Boolean okcheck = context->CanCheckRefInt(rel, fic);

				if (okcheck && OnModify)
				{
					Boolean match = true;
					const FieldArray& fields = rel->GetDestinations();
					for (FieldArray::ConstIterator cur = fields.First(), end = fields.End(); cur != end; cur++)
					{
						if (*cur != nil)
						{
							ValPtr vnew;
							ValPtr vold = fic->GetFieldOldValue(*cur, err, true);
							if (err == VE_OK)
							{
								vnew = fic->GetFieldValue(*cur, err, false, true);
							}
							if (err == VE_OK && vnew != nil && vold != nil)
								match = vnew->EqualToSameKind(vold, false);
							if (err != VE_OK || !match)
								break;
						}
					}
					okcheck = !match && (err == VE_OK);
				}

				if (okcheck)
				{
					if (rel->AutoDeleteRelatedRecords())
					{
						Transaction *trans = context->StartTransaction(err);

						if (err == VE_OK)
						{
							//if (rel->GetType() == Relation_1ToN)
							{
								Selection* sel;
								err = rel->ActivateOneToMany(fic, sel, context, OnModify, false, DB4D_Keep_Lock_With_Transaction);
								if (err == VE_OK)
								{
									if (sel != nil)
									{
										if (OnModify)
										{
											SelectionIterator itersel(sel);
											sLONG cur = itersel.FirstRecord();
											while (cur != -1 && err == VE_OK)
											{
												FicheInMem* rec = sel->GetParentFile()->LoadRecord(cur, err, DB4D_Keep_Lock_With_Record, context);
												if (rec != nil)
												{
													if (err == VE_OK)
													{
														if (rec->ReadOnlyState())
															err = ThrowError(VE_DB4D_RECORDISLOCKED, DBaction_CheckingReferentialIntegrity);
														else
															err = rel->CopyFields(fic,rec);
														if (err == VE_OK)
														{
															context->MustNotCheckRefInt(rel, rec);
															err = sel->GetParentFile()->SaveRecord(rec, context);
															context->CheckRefIntAgain(rel, rec);
														}
													}
													rec->Release();
												}
												cur = itersel.NextRecord();
											}
										}
										else
										{
											if (sel->DeleteRecords(context, nil, rel) != VE_OK)
											{
												err = fic->ThrowError(VE_DB4D_SOME_RELATED_RECORDS_STILL_EXIST, DBaction_CheckingReferentialIntegrity);
											}
										}
										sel->Release();
									}
								}
								else
									err = ThrowError(VE_DB4D_CANNOT_CHECK_REFERENCIAL_INTEGRITY, DBaction_CheckingReferentialIntegrity);
							}
							/*
							else
							{
								FicheInMem *other;
								err = rel->ActivateManyToOne(fic, other, context, OnModify, false, true);
								if (err == VE_OK)
								{
									if (other != nil)
									{
										if (OnModify)
										{
											if (other->ReadOnlyState())
												err = ThrowError(VE_DB4D_RECORDISLOCKED, DBaction_CheckingReferentialIntegrity);
											else
												err = rel->CopyFields(fic,other);
											if (err == VE_OK)
											{
												context->MustNotCheckRefInt(rel, other);
												other->GetDF()->SaveRecord(other, context);
												context->CheckRefIntAgain(rel, other);
											}
										}
										else
										{
											context->MustNotCheckRefInt(rel, other);
											err = other->GetDF()->DelRecord(other, context);
											context->CheckRefIntAgain(rel, other);
											if (err != VE_OK)
											{
												err = fic->ThrowError(VE_DB4D_SOME_RELATED_RECORDS_STILL_EXIST, DBaction_CheckingReferentialIntegrity);
											}
										}
										other->Release();
									}
								}
								else
									err = ThrowError(VE_DB4D_CANNOT_CHECK_REFERENCIAL_INTEGRITY, DBaction_CheckingReferentialIntegrity);

							}

							*/
						}
							
						if (err == VE_OK)
						{
							err = context->CommitTransaction();
							if (err != VE_OK)
							{
								err = ThrowError(VE_DB4D_CANNOT_CHECK_REFERENCIAL_INTEGRITY, DBaction_CheckingReferentialIntegrity);
							}
						}
						else
							context->RollBackTransaction();
					}
					else
					{
						//if (rel->GetType() == Relation_1ToN)

						if ( !OnModify || rel->IsForeignKey() )/*Sergiy - 26 April 2007 - For compatibility with 2004*/
						{
							Selection* sel;
							err = rel->ActivateOneToMany(fic, sel, context, OnModify);
							if (err == VE_OK)
							{
								if (sel != nil)
								{
									if (sel->GetQTfic() > 0)
									{
										err = fic->ThrowError(VE_DB4D_SOME_RELATED_RECORDS_STILL_EXIST, DBaction_CheckingReferentialIntegrity);
									}
									sel->Release();
								}
							}
							else
								err = ThrowError(VE_DB4D_CANNOT_CHECK_REFERENCIAL_INTEGRITY, DBaction_CheckingReferentialIntegrity);
						}

						/*
						else
						{
							FicheInMem *other;
							err = rel->ActivateManyToOne(fic, other, context, OnModify);
							if (err == VE_OK)
							{
								if (other != nil)
								{
									err = fic->ThrowError(VE_DB4D_SOME_RELATED_RECORDS_STILL_EXIST, DBaction_CheckingReferentialIntegrity);
									other->Release();
								}
							}
							else
								err = ThrowError(VE_DB4D_CANNOT_CHECK_REFERENCIAL_INTEGRITY, DBaction_CheckingReferentialIntegrity);

						}
						*/

					}
				}
				if (err != VE_OK)
					break;
			}
		}
		else
			libere();
	}

	return err;
}



VError Table::CheckReferentialIntegrityForForeignKey(FicheInMem *fic, BaseTaskInfo* context, Boolean OnModify)
{
	VError err = VE_OK;

	if (testAssert(!fIsRemote))
	{
		occupe();
		if (Owner->ReferentialIntegrityEnabled() && AtLeastOneReferentialIntegrityForeign())
		{
			DepRelationArray rels;
			rels.CopyAndRetainFrom(RefIntegrityDepForeign);
			libere();

			sLONG i,nb = rels.GetCount();

			for (i=1; i <= nb; i++)
			{
				Relation* rel = rels[i];
				assert(rel != nil);
				assert(rel->WithReferentialIntegrity());

				Boolean okcheck = context->CanCheckRefInt(rel, fic);

				if (okcheck)
				{
					const FieldArray& fields = rel->GetSources();
					for (FieldArray::ConstIterator cur = fields.First(), end = fields.End(); cur != end; cur++)
					{
						if (*cur != nil)
						{
							ValPtr cv = fic->GetFieldValue(*cur, err, false, true);
							if (cv == nil || cv->IsNull())
								okcheck = false;
						}
					}
				}

				if (okcheck && OnModify)
				{
					Boolean match = true;
					const FieldArray& fields = rel->GetSources();
					for (FieldArray::ConstIterator cur = fields.First(), end = fields.End(); cur != end; cur++)
					{
						if (*cur != nil)
						{
							ValPtr vnew;
							ValPtr vold = fic->GetFieldOldValue(*cur, err, true);
							if (err == VE_OK)
							{
								vnew = fic->GetFieldValue(*cur, err, false, true);
							}
							if (err == VE_OK && vnew != nil && vold != nil)
								match = vnew->EqualToSameKind(vold, false);
							if (err != VE_OK || !match)
								break;
						}
					}
					okcheck = !match && (err == VE_OK);
				}

				if (okcheck && rel->IsForeignKey() )/*Sergiy - 26 April 2007 - For compatibility with 2004*/
				{
					Selection* sel;
					//err = rel->ActivateDestToSource(fic, sel, context);
					err = rel->ActivateManyToOneS(fic, sel, context, false, DB4D_Do_Not_Lock);
					if (err == VE_OK)
					{
						if (sel != nil)
						{
							if (sel->GetQTfic() == 0)
							{
								bool okinside = false;
								if (rel->GetDestTable() == this)
								{
									bool isequal = true;
									const FieldArray& sources = rel->GetSources();
									const FieldArray& destinations = rel->GetDestinations();
									for (FieldArray::ConstIterator cur = sources.First(), end = sources.End(), cur2 = destinations.First(); cur != end && isequal; cur++, cur2++)
									{
										ValPtr cv =fic->GetFieldValue(*cur, err, false, true);
										ValPtr cv2 =fic->GetFieldValue(*cur2, err, false, true);
										if (cv != nil && cv2 != nil)
										{
											if (cv->IsNull() || cv2->IsNull())
											{
												isequal = false;
											}
											else
											{
												if (cv->GetValueKind() == cv2->GetValueKind())
												{
													if (!cv2->EqualToSameKind(cv))
														isequal = false;
												}
												else
												{
													ValPtr cv3 = cv2->ConvertTo(cv->GetValueKind());
													if (!cv3->EqualToSameKind(cv))
														isequal = false;
													delete cv3;
												}
											}
										}
										else
											isequal = false;
									}
									if (isequal)
										okinside = true;
								}
								if (! okinside)
									err = ThrowError(VE_DB4D_NO_PRIMARYKEY_MATCHING_THIS_FOREIGNKEY, DBaction_CheckingReferentialIntegrity);
							}
							sel->Release();
						}
						else
							err = ThrowError(VE_DB4D_NO_PRIMARYKEY_MATCHING_THIS_FOREIGNKEY, DBaction_CheckingReferentialIntegrity);
					}
					else
						err = ThrowError(VE_DB4D_CANNOT_CHECK_REFERENCIAL_INTEGRITY, DBaction_CheckingReferentialIntegrity);
				}
				if (err != VE_OK)
					break;
			}

		}
		else
			libere();
	}

	return err;
}


VError Table::UpdateIndexKeyForTrans(sLONG n, FicheOnDisk *ficD, BaseTaskInfo* context)
{
	sLONG i,nb;
	IndexInfo *ind;
	IndexArray deps;
	VError err;
	Boolean deleteoldkey = !ficD->IsNewInTrans();
	FicheInMem *fic = nil, *oldfic = nil;

	err=VE_OK;
	if (testAssert(!fIsRemote))
	{
		occupe();
		deps.CopyAndRetainFrom(IndexDep);
		libere();

		OccupableStack* curstack;

		nb=deps.GetCount();
		if (nb > 0 || HasSyncInfo())
			 curstack = gOccPool.GetStackForCurrentThread();

		/* plus besoin de le faire ici, car les SyncInfo sont maintenant maintenues dans une transaction
		if (HasSyncInfo())
		{
			fic = new FicheInMem(context, this, ficD);
			if (fic != nil)
				err = fic->SetSyncInfo(Sync_Update);
		}
		*/

		for (i=1;(i<=nb) && (err == VE_OK);i++)
		{
			ind=deps[i];
			{
				ind->WaitEndOfQuickBuilding();
				if (ind->AskForValidOutsideContext(context, false))  // si l'index est valid hors transaction, 
																				// mais pas pour cette transaction, alors c'est qu'il a ete cree apres le debut de la transaction
																				// dans ce cas, il faut mettre a jour les cle d'index
				{
					ind->Open(index_write);
					sLONG curelem = ind->CurElemEnCreation();
					if (curelem == -2 || curelem>=n || ind->GetMaxElemEnCreation()<=n)
					{
						if (deleteoldkey)
						{
							if (oldfic == nil)
							{
								oldfic = GetDF()->LoadRecord(n, err, DB4D_Do_Not_Lock, nil /* c'est expres que le context est nil */);
							}
							if (oldfic != nil)
							{
								err=ind->DetruireCle(curstack, oldfic, /*context*/ nil /* c'est expres que le context est nil */);
							}
						}
						if (err == VE_OK)
						{
							if (fic == nil)
							{
								fic = new FicheInMem(context, this, ficD);
							}
							if (fic != nil)
							{
								err=ind->PlaceCle(curstack, fic, /*context*/ nil /* c'est expres que le context est nil */);
							}
						}
					}
					ind->Close();
					ind->ReleaseValid();
				}
			}
		}

		if (err != VE_OK)
			err = ThrowError(VE_DB4D_CANNOTUPDATEINDEX, DBaction_UpdatingIndexKeyForRecord);
	}

	if (oldfic != nil)
		oldfic->Release();
	if (fic != nil)
		fic->Release();

	return(err);
}


VError Table::UpdateIndexKey(FicheInMem *fic, BaseTaskInfo* context)
{
	sLONG i,nb;
	IndexInfo *ind;
	IndexArray deps;
	VError err;
	Boolean deleteoldkey = !fic->IsNew();
	
	err=VE_OK;
	if (testAssert(!fIsRemote))
	{
		occupe();
		deps.CopyAndRetainFrom(IndexDep);
		libere();
		
		OccupableStack* curstack;

		nb=deps.GetCount();
		if (nb > 0)
			curstack = gOccPool.GetStackForCurrentThread();

		for (i=1;(i<=nb) && (err == VE_OK);i++)
		{
			ind=deps[i];
			if (ind->NeedUpdate(fic, context))
			{
				ind->WaitEndOfQuickBuilding();
				if (ind->AskForValid(context, false)) 
				{
					ind->Open(index_write);
					sLONG curelem = ind->CurElemEnCreation();
					sLONG n = fic->GetNum();
					if (curelem == -2 || curelem>=n || ind->GetMaxElemEnCreation()<=n)
					{
						if (deleteoldkey)
						{
							err=ind->DetruireCle(curstack, fic, context);
						}
						if (err == VE_OK) 
							err=ind->PlaceCle(curstack, fic, context);
					}
					ind->Close();
					ind->ReleaseValid();
				}
			}
		}
		
		if (err != VE_OK)
			err = ThrowError(VE_DB4D_CANNOTUPDATEINDEX, DBaction_UpdatingIndexKeyForRecord);
	}
		
	return(err);
}


VError Table::DeleteIndexKeyForTrans(sLONG n, BaseTaskInfo* context)
{
	sLONG i,nb;
	IndexInfo *ind;
	IndexArray deps;
	VError err;
	FicheInMem* fic = nil;

	err=VE_OK;
	if (testAssert(!fIsRemote))
	{
		occupe();
		deps.CopyAndRetainFrom(IndexDep);
		libere();

		OccupableStack* curstack;

		nb=deps.GetCount();
		if (nb > 0 || HasSyncInfo())
			curstack = gOccPool.GetStackForCurrentThread();

		/* plus besoin de le faire ici, car les SyncInfo sont maintenant maintenues dans une transaction
		if (HasSyncInfo())
		{
			fic = GetDF()->LoadRecord(n, err, DB4D_Do_Not_Lock, nil ); // c'est expres que le context est nil

			if (fic != nil)
				err = fic->.SetSyncInfo(Sync_Delete);
		}
		*/

		for (i=1;(i<=nb) && (err==VE_OK);i++)
		{
			ind=deps[i];
			ind->WaitEndOfQuickBuilding();
			if (ind->AskForValidOutsideContext(context, false))
			{
				ind->Open(index_write);
				sLONG curelem = ind->CurElemEnCreation();
				if (curelem == -2 || curelem>=n || ind->GetMaxElemEnCreation()<=n)
				{
					if (fic == nil)
					{
						fic = GetDF()->LoadRecord(n, err, DB4D_Do_Not_Lock, nil /* c'est expres que le context est nil*/);
					}

					if (fic != nil)
						err=ind->DetruireCle(curstack, fic, context);
				}
				ind->Close();
				ind->ReleaseValid();
			}
		}

		if (err != VE_OK)
			err = ThrowError(VE_DB4D_CANNOTUPDATEINDEX, DBaction_DeletingIndexKeyForRecord);
	}

	if (fic != nil)
		fic->Release();

	return(err);
}



VError Table::DeleteIndexKey(FicheInMem *fic, BaseTaskInfo* context)
{
	sLONG i,nb;
	IndexInfo *ind;
	IndexArray deps;
	VError err;
	
	err=VE_OK;
	if (testAssert(!fIsRemote))
	{
		occupe();
		deps.CopyAndRetainFrom(IndexDep);
		libere();
		
		OccupableStack* curstack;

		nb=deps.GetCount();
		if (nb > 0)
			curstack = gOccPool.GetStackForCurrentThread();
		for (i=1;(i<=nb) && (err==VE_OK);i++)
		{
			ind=deps[i];
			ind->WaitEndOfQuickBuilding();
			if (ind->AskForValid(context, false)) 
			{
				ind->Open(index_write);
				sLONG curelem = ind->CurElemEnCreation();
				sLONG n = fic->GetNum();
				if (curelem == -2 || curelem>=n || ind->GetMaxElemEnCreation()<=n)
				{
					err=ind->DetruireCle(curstack, fic, context);
				}
				ind->Close();
				ind->ReleaseValid();
			}
		}
		
		if (err != VE_OK)
			err = ThrowError(VE_DB4D_CANNOTUPDATEINDEX, DBaction_DeletingIndexKeyForRecord);
	}
		
	return(err);
}


void Table::SetRecNumHintForBlobs(FicheInMem *fic, BaseTaskInfo* context, sLONG numrec)
{
	sLONG i,nb,n;
	VError err;
	ValPtr cv;

	err=VE_OK;
	if (testAssert(!fIsRemote))
	{
		occupe();

		NumFieldArray xBlobDep;
		xBlobDep.CopyFrom(BlobDep);
		libere();

		nb=xBlobDep.GetCount();
		for (i=1; i<=nb; i++)
		{
			n=xBlobDep[i];
			cv=fic->GetNthField(-n, err);
			if (cv != nil)
			{
				cv->SetHintRecNum(GetNum(), n, numrec);
			}
		}
	}

}


VError Table::UpdateBlobs(FicheInMem *fic, BaseTaskInfo* context)
{
	sLONG i,nb,n;
	VError err;
	ValPtr cv;
	
	err=VE_OK;
	if (testAssert(!fIsRemote))
	{
		occupe();
		
		NumFieldArray xBlobDep;
		xBlobDep.CopyFrom(BlobDep);
		libere();
		
		nb=xBlobDep.GetCount();
		for (i=1;(i<=nb) && (err==VE_OK);i++)
		{
			n=xBlobDep[i];
			if (fic->IsModif(n))
			{
				cv=fic->GetNthField(-n, err);
				if (cv != nil)
				{

#if debugblobs
					sLONG oldblobnum = -2;
					sLONG oldblobnumindata = -2;

					sLONG* curp = (sLONG*)fic->GetAssoc()->GetDataPtr(n);
					if (curp != nil)
						oldblobnumindata = *curp;

					if (cv->GetValueKind() == VK_BLOB)
					{
						BlobWithPtr* blob = ((VBlob4DWithPtr*)cv)->GetBlob4D();
						if (blob != nil)
						{
							oldblobnum = blob->GetNum();
						}
					}
#endif
					err = cv->Flush(fic->GetDataPtr(n), context);
#if debugblobs
					if (cv->GetValueKind() == VK_BLOB)
					{
						BlobWithPtr* blob = ((VBlob4DWithPtr*)cv)->GetBlob4D();
						if (blob != nil)
						{
							sLONG blobnum = blob->GetNum();
							if (blobnum >=0)
							{
								if (fic->GetAssoc() != nil)
								{
									DataTableRegular* df = (DataTableRegular*)(fic->GetDF());
									df->debug_putblobnum(blobnum, fic->GetNum());
								}
							}
						}
					}

#endif

				}
			}
		}
		
		
		if (err != VE_OK)
			err = ThrowError(VE_DB4D_CANNOTUPDATEBLOBS, DBaction_UpdatingBlobForRecord);
	}

	return(err);
}


VError Table::DeleteBlobs(FicheInMem *fic, BaseTaskInfo* context)
{
	sLONG i,nb,n;
	ValPtr cv;
	Blob4D* theBlob = nil;
	VError err;
	
	err=VE_OK;
	if (testAssert(!fIsRemote))
	{
		
		occupe();
		
		NumFieldArray xBlobDep;
		xBlobDep.CopyFrom(BlobDep);
		libere();
		
		nb=xBlobDep.GetCount();
		for (i=1;i<=nb && err == VE_OK;i++)
		{
			n=xBlobDep[i];
			cv=fic->GetNthField(n, err);
			if (cv != nil)
			{
				ValueKind typblob = cv->GetValueKind();
				if (typblob == VK_TEXT || typblob == VK_STRING || typblob == VK_STRING_UTF8 || typblob == VK_TEXT_UTF8)
				{
					VBlobText *blobtext;
#if VERSIONDEBUG
					blobtext = dynamic_cast<VBlobText*>(cv);
#else
					blobtext = (VBlobText*)cv;
#endif
					if (testAssert(blobtext != nil))
					{
						Blob4D *theBlob = blobtext->GetBlob4D(context, true);
						if (theBlob != nil)
							err=DF->DelBlob(theBlob, context);
					}
				}
				else
				{
					if (typblob == VK_IMAGE)
					{
						VBlob4DWithPtr *blobhandle;
#if VERSIONDEBUG
						blobhandle = dynamic_cast<VBlob4DWithPtr*>(cv->GetDataBlob());
#else
						blobhandle = (VBlob4DWithPtr*)(cv->GetDataBlob());
#endif
						if ( testAssert(blobhandle != nil)) 
						{
							Blob4D *theBlob = blobhandle->GetBlob4D();
							if (theBlob != nil)
								err=DF->DelBlob(theBlob, context);
						}
					}
					else
					{
						VBlob4DWithPtr *blobhandle;
#if VERSIONDEBUG
						blobhandle = dynamic_cast<VBlob4DWithPtr*>( cv);
#else
						blobhandle = (VBlob4DWithPtr*)cv;
#endif
						if ( testAssert(blobhandle != nil)) {
							Blob4D *theBlob = blobhandle->GetBlob4D();
							if (theBlob != nil)
								err=DF->DelBlob(theBlob, context);
						}
					}
				}
			}
		}
				
		if (err != VE_OK)
			err = ThrowError(VE_DB4D_CANNOTDELETEBLOBS, DBaction_DeletingBlobForRecord);
	}

	return(err);
}


sLONG Table::FindField( const VString& pFieldName) const
{
	sLONG result,i,nb;
	FieldPtr cri;
	VString s;
	
	result = 0;
	nb = tc.GetCount();
	for (i=1; i<=nb; i++)
	{
		cri = tc[i];
		if (cri != nil)
		{
			cri->GetName(s);
			if (s == pFieldName)
			{
				result = i;
				break;
			}
		}
	}
	
	return(result);
}


sLONG Table::FindField( const VUUID& inFieldID) const
{
	sLONG result,i,nb;
	FieldPtr cri;
	VUUID xid;

	result = 0;
	nb = tc.GetCount();
	for (i=1; i<=nb; i++)
	{
		cri = tc[i];
		if (cri != nil)
		{
			cri->GetUUID(xid);
			if (xid == inFieldID)
			{
				result = i;
				break;
			}
		}
	}

	return(result);
}


Field* Table::FindAndRetainFieldRef(const VString& pFieldName) const
{
	sLONG i,nb;
	FieldPtr cri;
	VString s;
	FieldPtr result;
	
	result = nil;
	nb = tc.GetCount();
	for (i=1; i<=nb; i++)
	{
		cri = tc[i];
		if (cri != nil)
		{
			cri->GetName(s);
			if (s == pFieldName)
			{
				result = cri;
				cri->Retain();
				break;
			}
		}
	}
	
	return(result);
}



Field* Table::FindAndRetainFieldRef(const VUUID& pRefID) const
{
	VObjLock lock(this);
	// L.E. 28/09/05 shark dit qu'on passe 20% du temps ici en ouverture de base avec 238 tables
	// peut-etre qu'une map globale (dans Base) serait opportune.
	// L.R ok c'est fait le 8/2/06
	FieldPtr result;
	result = (FieldPtr)Owner->GetObjectByID(objInBase_Field, pRefID);
	if (result != nil)
		result->Retain();
	return(result);
}


Field* Table::RetainField(sLONG nc) const
{
	Field* cri = nil;

	occupe();
	if (nc>0 && nc<=tc.GetCount())
	{
		cri = tc[nc];
		if (cri != nil)
			cri->Retain();
	}

	libere();

	return cri;
}


VError Table::RetainExistingFields(vector<CDB4DField*>& outFields)
{
	VError err = VE_OK;
	outFields.clear();
	occupe();
	for (FieldArray::Iterator cur = tc.First(), end = tc.End(); cur != end && err == VE_OK; cur++)
	{
		if (*cur != nil)
		{
			CDB4DField* xfield = new VDB4DField(*cur);
			if (xfield == nil)
				err = ThrowError(memfull, DBaction_BuildingListOfFields);
			else
			{
				try
				{
					outFields.push_back(xfield);
				}
				catch (...)
				{
					err = ThrowError(memfull, DBaction_BuildingListOfFields);
					xfield->Release();
				}
			}
		}
	}

	libere();

	if (err != VE_OK)
	{
		for (vector<CDB4DField*>::iterator cur = outFields.begin(), end = outFields.end(); cur != end; cur++)
		{
			if (*cur != nil)
				(*cur)->Release();
		}
		outFields.clear();
	}

	return err;
}


VError Table::AddConstraint()
{
	VObjLock lock(this);

	if (PrimaryKey.GetCount() > 0)
		return Owner->AddConstraint(this);
	else
		return VE_OK;
}


VError Table::AddConstraintCols()
{
	VObjLock lock(this);
	VError err = VE_OK;
	if (PrimaryKey.GetCount() > 0)
	{
		sLONG n = 1;
		for (NumFieldArray::Iterator cur = PrimaryKey.First(), end = PrimaryKey.End(); cur != end && err == VE_OK; cur++, n++)
		{
			err = Owner->AddConstraintCol(this, tc[*cur], n);
		}
	}
	return err;
}


VError Table::DelConstraint()
{
	VObjLock lock(this);

	if (PrimaryKey.GetCount() > 0)
		return Owner->DelConstraint(this);
	else
		return VE_OK;
}


VError Table::DelConstraintCols()
{
	VObjLock lock(this);

	if (PrimaryKey.GetCount() > 0)
		return Owner->DelConstraintCol(this);
	else
		return VE_OK;
}


void Table::WaitToBuildIndex()
{
	//fBuildIndexMutex.Lock(RWS_WriteExclusive);
	if (DF != nil)
		DF->LockRead();
}

void Table::FinishedBuildingIndex()
{
	if (DF != nil)
		DF->Unlock();
	//fBuildIndexMutex.Unlock();
}


bool Table::GetFieldsNum(vector<sLONG>& outFieldsNum)
{
	bool ok = true;
	try
	{
		VObjLock lock(this);
		outFieldsNum.reserve(tc.GetCount());
		sLONG i = 0;
		for (FieldArray::Iterator cur = tc.First(), end = tc.End(); cur != end; cur++)
		{
			i++;
			if (*cur != nil)
				outFieldsNum.push_back(i);
		}

	}
	catch (...)
	{
		ok = false;
	}
	return ok;
}


sLONG Table::CountExistingFields()
{
	sLONG tot = 0;

	occupe();
	for (FieldArray::Iterator cur = tc.First(), end = tc.End(); cur != end; cur++)
	{
		if (*cur != nil)
			tot++;
	}

	libere();

	return tot;
}



sLONG Table::FindNextFieldFree()
{
	sLONG i, res = -1, nb;
	
	// pas besoin de mutex ici car l'appelant doit le faire (AddField)
	
	nb = tc.GetCount();
	for (i = 1; i<=nb; i++)
	{
		if (tc[i] == nil)
		{
			res = i;
			break;
		}
	}
	
	return res;
}


IndexInfo* Table::FindAndRetainIndex(FieldNuplet* fn, uBOOL sortable, Boolean MustBeValid, BaseTaskInfo* context) const
{
	sLONG i;
	sLONG nb;
	IndexInfo *ind,*ind2;
	
	if ((fn->GetNbField()==1) /* && ( (fn->li[0].numfile==0) || (fn->li[0].numfile==numfile) )*/ )
	{
		ind2=FindAndRetainIndexSimple(fn->GetFieldNum(0), sortable, MustBeValid);
	}
	else
	{
		occupe();
		nb=IndexDep.GetCount();
		ind2=nil;
		for (i=1;i<=nb;i++)
		{
			ind=IndexDep[i];
			if (ind!=nil)
			{
				if (ind->MatchType(DB4D_Index_OnMultipleFields))
				{
					if (((IndexInfoFromMultipleField*)ind)->MatchFields(fn))
					{
						if (sortable)
						{
							if (ind->MayBeSorted())
							{
								{
									ind2=ind;
									ind2->Retain();
									break;
								}
							}
						}
						else
						{
							{
								ind2=ind;
								ind2->Retain();
								break;
							}
						}
					}
				}
			}
		}
		libere();

		if (MustBeValid && ind2 != nil)
		{
			if (! ind2->AskForValid(context))
			{
				ind2->Release();
				ind2 = nil;
			}
		}
	}

	return(ind2);
}



IndexInfo* Table::FindAndRetainIndex(const NumFieldArray& fields, uBOOL sortable, Boolean MustBeValid, BaseTaskInfo* context) const
{
	sLONG i;
	sLONG nb;
	IndexInfo *ind,*ind2;

	if ((fields.GetCount() == 1) )
	{
		ind2=FindAndRetainIndexSimple(fields[1], sortable, MustBeValid);
	}
	else
	{
		occupe();
		nb=IndexDep.GetCount();
		ind2=nil;
		for (i=1;i<=nb;i++)
		{
			ind=IndexDep[i];
			if (ind!=nil)
			{
				if (ind->MatchType(DB4D_Index_OnMultipleFields))
				{
					if (((IndexInfoFromMultipleField*)ind)->MatchFields(fields, numfile))
					{
						if (sortable)
						{
							if (ind->MayBeSorted())
							{
								{
									ind2=ind;
									ind2->Retain();
									break;
								}
							}
						}
						else
						{
							{
								ind2=ind;
								ind2->Retain();
								break;
							}
						}
					}
				}
			}
		}
		libere();

		if (MustBeValid && ind2 != nil)
		{
			if (! ind2->AskForValid(context))
			{
				ind2->Release();
				ind2 = nil;
			}
		}
	}


	return(ind2);
}


Boolean Table::IsIndexed(const NumFieldArray& fields) const
{
	Boolean result = false;
	IndexInfo* ind = FindAndRetainIndex(fields, false);
	if (ind != nil)
	{
		ind->ReleaseValid();
		ind->Release();
		result = true;
	}
	return result;
}


VError Table::DropAllRelatedIndex(CDB4DBaseContext* inContext, VProgressIndicator* progress)
{
	VError err = VE_OK;
	sLONG i, nb = IndexDep.GetCount();

	if (testAssert(!fIsRemote))
	{	
		for (i=nb; i>0; i--)
		{
			IndexInfo* ind = IndexDep[i];
			IndexDep[i] = nil;
			Owner->DeleteIndexByRef(ind, inContext, progress);
		}
		IndexDep.SetAllocatedSize(0);
	}
	
	return err;
}


void Table::RebuildAllRelatedIndex(CDB4DBaseContext* inContext, VProgressIndicator* progress)
{
	sLONG i, nb = IndexDep.GetCount();

	if (testAssert(!fIsRemote))
	{	
		for (i=nb; i>0; i--)
		{
			IndexInfo* ind = IndexDep[i];
			Owner->RebuildIndexByRef(ind, inContext, progress, nil);
		}
	}
}


VError Table::InitDataFileOfFields()
{
	VObjLock lock(this);
	VError err = VE_OK;

	FieldArray::Iterator cur = tc.First(), end = tc.End();
	for (; cur != end; cur++)
	{
		Field* cri = *cur;
		if (cri != nil)
		{
			err = Owner->AddFieldRec(GetNum(), cri->GetPosInRec());
			if (err != VE_OK)
				break;
		}
	}
	return err;
}


void Table::ClearAllDependencies()
{
	ClearRelationNto1Dep();
	ClearRelation1toNDep();
	ClearRefIntegrityDep();
	ClearRefIntegrityDepForeign();
	ClearIndexDep();

	FieldArray::Iterator cur = tc.First(), end = tc.End();
	for (; cur != end; cur++)
	{
		Field* cri = *cur;
		if (cri != nil)
			cri->ClearAllDependencies();
	}

}


void Table::AddRelationNto1Dep(Relation* rel)
{
	rel->Retain();
	RelDepNto1.Add(rel);
}


void Table::DelRelationNto1Dep(Relation* rel)
{
	sLONG i;

	for (i=1;i<=RelDepNto1.GetCount();i++)
	{
		if (rel==RelDepNto1[i])
		{
			RelDepNto1.DeleteNth(i);
			break;
		}
	}
}

void Table::ClearRelationNto1Dep(void)
{
	RelDepNto1.SetAllocatedSize(0);
}



void Table::AddRelation1toNDep(Relation* rel)
{
	rel->Retain();
	RelDep1toN.Add(rel);
}


void Table::DelRelation1toNDep(Relation* rel)
{
	sLONG i;

	for (i=1;i<=RelDep1toN.GetCount();i++)
	{
		if (rel==RelDep1toN[i])
		{
			RelDep1toN.DeleteNth(i);
			break;
		}
	}
}

void Table::ClearRelation1toNDep(void)
{
	RelDep1toN.SetAllocatedSize(0);
}


void Table::AddRefIntegrityDep(Relation* rel)
{
	rel->Retain();
	RefIntegrityDep.Add(rel);
}


void Table::DelRefIntegrityDep(Relation* rel)
{
	sLONG i;

	for (i=1;i<=RefIntegrityDep.GetCount();i++)
	{
		if (rel==RefIntegrityDep[i])
		{
			RefIntegrityDep.DeleteNth(i);
			break;
		}
	}
}

void Table::ClearRefIntegrityDep(void)
{
	RefIntegrityDep.SetAllocatedSize(0);
}


void Table::AddRefIntegrityDepForeign(Relation* rel)
{
	rel->Retain();
	RefIntegrityDepForeign.Add(rel);
}


void Table::DelRefIntegrityDepForeign(Relation* rel)
{
	sLONG i;

	for (i=1;i<=RefIntegrityDepForeign.GetCount();i++)
	{
		if (rel==RefIntegrityDepForeign[i])
		{
			RefIntegrityDepForeign.DeleteNth(i);
			break;
		}
	}
}

void Table::ClearRefIntegrityDepForeign(void)
{
	RefIntegrityDepForeign.SetAllocatedSize(0);
}


VError Table::SetPrimaryKeySilently(const NumFieldArray& PrimKey, VString* inPrimaryKeyName) // utilisee lors de l'ouverture d'une base en XML
{
	VError err = VE_OK;

	NumFieldArray::ConstIterator cur = PrimKey.First(), end = PrimKey.End();
	for (;cur != end && err == VE_OK; cur++)
	{
		sLONG numcrit = *cur;
		if (numcrit<=0 || numcrit>tc.GetCount())
		{
			err = ThrowError(VE_DB4D_WRONGFIELDREF_IN_TABLE, DBaction_SettingPrimaryKey);
		}
		else
		{
			Field* cri = tc[numcrit];
			if (cri != nil)
			{
				if (!cri->IsSortable() || !cri->CanBeUnique() || !cri->CanBeNot_Null())
					err = ThrowError(VE_DB4D_WRONGFIELDREF_IN_TABLE, DBaction_SettingPrimaryKey);
			}
			else
				err = ThrowError(VE_DB4D_WRONGFIELDREF_IN_TABLE, DBaction_SettingPrimaryKey);
		}
	}

	if (err == VE_OK)
	{
		if (PrimKey.GetCount() == 1)
		{
			Field* cri = RetainField(PrimKey[1]);
			if (cri != nil)
			{
				cri->GetCritere()->SetUnique(true);
				cri->Release();
			}
		}
		if (inPrimaryKeyName != nil)
			fPrimaryKeyName = *inPrimaryKeyName;
		else
			fPrimaryKeyName.Clear();
		if (!PrimaryKey.CopyFrom(PrimKey))
			err = ThrowError(memfull, DBaction_SettingPrimaryKey);
		AddConstraint();
		AddConstraintCols();
	}
	return err;
}


VError Table::SetPrimaryKey(const NumFieldArray& PrimKey, VProgressIndicator* InProgress, Boolean CanReplaceExistingOne, CDB4DBaseContext* inContext, VString* inPrimaryKeyName)
{
	VError err = VE_OK;
	
	if (fIsRemote)
	{
		// sc 09/03/2010 ACI0065041
		IRequest *req = GetOwner()->CreateRequest( inContext, Req_SetTablePrimaryKey + kRangeReqDB4D);
		if (req == NULL)
		{
			err = ThrowBaseError( memfull, DBaction_ChangingTableProperties);
		}
		else
		{
			req->PutBaseParam( GetOwner());
			req->PutTableParam( this);
			req->PutNumFieldArrayParam( PrimKey);
			req->PutStringParam( inPrimaryKeyName);
			req->PutBooleanParam( CanReplaceExistingOne);
			req->PutProgressParam( InProgress);
			
			err = req->GetLastError();
			if (err == VE_OK)
			{
				err = req->Send();
				if (err == VE_OK)
				{
					occupe();

					NumFieldArray numField;
					VString name;
					
					err = req->GetNumFieldArrayReply( numField);
					if (err == VE_OK)
						err = req->GetStringReply( name);
					if (err == VE_OK)
					{
						fPrimaryKeyName = name;
						PrimaryKey.CopyFrom( numField);
					}

					libere();
				}
			}
			req->Release();
		}
	}
	else
	{
		occupe();

		if (PrimKey.GetCount() > 0)
		{
			if (HasSyncInfo())
				CanReplaceExistingOne = false;
			if (PrimaryKey.GetCount() > 0 && !CanReplaceExistingOne)
				err = -1; // le bon message est plus bas
				//err = ThrowError(VE_DB4D_CANNOT_CHANGE_PRIMARYKEY_DEFINITION, DBaction_SettingPrimaryKey);
			else
			{
				NumFieldArray::ConstIterator cur = PrimKey.First(), end = PrimKey.End();
				for (;cur != end && err == VE_OK; cur++)
				{
					sLONG numcrit = *cur;
					if (numcrit<=0 || numcrit>tc.GetCount())
					{
						err = ThrowError(VE_DB4D_WRONGFIELDREF_IN_TABLE, DBaction_SettingPrimaryKey);
					}
					else
					{
						Field* cri = tc[numcrit];
						if (cri != nil)
						{
							if (!cri->IsSortable() || !cri->CanBeUnique() || !cri->CanBeNot_Null())
								err = ThrowError(VE_DB4D_WRONGFIELDREF_IN_TABLE, DBaction_SettingPrimaryKey);
						}
						else
							err = ThrowError(VE_DB4D_WRONGFIELDREF_IN_TABLE, DBaction_SettingPrimaryKey);
					}
				}
			}

			if (err == VE_OK && DF != nil)
			{
				BaseTaskInfo* context;
				if (inContext == nil)
					context = new BaseTaskInfo(Owner, nil, nil, nil);
				else
					context = ConvertContext(inContext);

				//BaseTaskInfo context(Owner, nil);
				if (DF->LockTable(context))
				{
					if (DF->CheckForNonUniqueField(PrimKey, InProgress, err, inContext))
						err = ThrowError(VE_DB4D_DUPLICATED_KEY, DBaction_SettingPrimaryKey);

					if (err == VE_OK)
					{
						//BaseTaskInfo context(Owner, nil);
						VString cv2;
						cv2.SetNull(true);

						SearchTab rech(this, true);
						NumFieldArray::ConstIterator cur = PrimKey.First(), end = PrimKey.End();
						for (;cur != end && err == VE_OK; cur++)
						{
							if (cur != PrimKey.First())
								rech.AddSearchLineBoolOper(DB4D_And);
							sLONG numcrit = *cur;
							rech.AddSearchLineSimple(GetNum(), numcrit, DB4D_Equal, &cv2);
						}
						OptimizedQuery query;
						err = query.AnalyseSearch(&rech, context);
						if (err == VE_OK)
						{
							Selection *sel = query.Perform((Bittab*)nil, nil, context, err, DB4D_Do_Not_Lock);
							if (sel != nil)
							{
								if (sel->GetQTfic() > 0)
								{
									err = ThrowError(VE_DB4D_CANNOT_SET_FIELD_TO_Not_Null, DBaction_SettingPrimaryKey);
								}
								sel->Release();
							}
						}

					}

					if (err == VE_OK)
					{
						occupe();
						FieldArray fields;
						
						for (NumFieldArray::ConstIterator cur = PrimKey.First(), end = PrimKey.End(); cur != end && err == VE_OK; cur++)
						{
							fields.Add(tc[*cur]);
						}

						for (IndexArrayIncluded::Iterator cur = IndexDep.First(), end = IndexDep.End(); cur != end; cur++)
						{
							IndexInfo* ind = *cur;
							if (ind != nil)
							{
								if (ind->MatchType(DB4D_Index_OnOneField) || ind->MatchType(DB4D_Index_OnMultipleFields))
								{
									if (ind->MatchIndex(&fields, fields.GetCount()))
									{
										ind->SetUnique(true);
										ind->Save();
										ind->SaveInStruct();
									}
								}
							}
						}
						libere();
					}
					DF->UnLockTable(context);
				}
				else
					err = ThrowError(VE_DB4D_CANNOT_GET_EXCLUSIVEACCESS_ON_TABLE, DBaction_SettingPrimaryKey);

				if (inContext == nil && context != nil)
					context->Release();
			}
		}
		else // check if there are still foreign key to it
		{
			if (HasSyncInfo())
				err = ThrowError(VE_DB4D_FOREIGN_KEY_CONSTRAINT_LIST_NOT_EMPTY, DBaction_SettingPrimaryKey);

			for (DepRelationArrayIncluded::Iterator cur = RefIntegrityDep.First(), end = RefIntegrityDep.End(); cur != end && err == VE_OK; cur++)
			{
				Relation* rel = *cur;
				if (rel->IsForeignKey())
				{
					err = ThrowError(VE_DB4D_FOREIGN_KEY_CONSTRAINT_LIST_NOT_EMPTY, DBaction_SettingPrimaryKey);
				}
			}
		}

		if (err == VE_OK)
		{
			if (PrimKey.GetCount() == 1)
			{
				Field* cri = RetainField(PrimKey[1]);
				if (cri != nil)
				{
					cri->GetCritere()->SetUnique(true);
					cri->Release();
				}
			}
			StErrorContextInstaller errs(false);
			DelConstraint();
			DelConstraintCols();
			if (inPrimaryKeyName != nil)
				fPrimaryKeyName = *inPrimaryKeyName;
			else
				fPrimaryKeyName.Clear();
			if (!PrimaryKey.CopyFrom(PrimKey))
				err = ThrowError(memfull, DBaction_SettingPrimaryKey);
			AddConstraint();
			AddConstraintCols();
			Touch();
			save();
		}

		if (err != VE_OK)
			err = ThrowError(VE_DB4D_CANNOT_CHANGE_PRIMARYKEY_DEFINITION, DBaction_SettingPrimaryKey);

		libere();
	}
	return err;
}


VError Table::RetainPrimaryKey(FieldArray& outPrimKey) const
{
	VError err = VE_OK;
	occupe();

	outPrimKey.SetCount(0);
	NumFieldArray::ConstIterator cur = PrimaryKey.First(), end = PrimaryKey.End();
	for (;cur != end && err == VE_OK; cur++)
	{
		Field* cri = tc[*cur];
		if (testAssert(cri != nil))
		{
			if (outPrimKey.Add(cri))
			{
				cri->Retain();
			}
			else
				err = ThrowError(memfull, DBaction_BuildingListOfFields);
		}
		else
			err = ThrowError(memfull, DBaction_BuildingListOfFields);
	}
	
	libere();
	return err;
}


void Table::CopyIndexDep(IndexArray& outDeps) const
{
	occupe();
	outDeps.CopyAndRetainFrom(IndexDep);
	libere();
}


class tempAutoRelInfo
{
public:
	VRefPtr<FicheInMem> fRecord;
	Table* fTable;;
};

typedef vector<tempAutoRelInfo> tempAutoRelInfoArray;

class tempAutoRel_1_N_Info
{
public:
	VRefPtr<FicheInMem> fRecord;
	VRefPtr<Selection> fSel;
	Table* fTable;
};

typedef vector<tempAutoRel_1_N_Info> tempAutoRel_1_N_InfoArray;


VError FillAutoRelInfo(Table* onTable, FicheInMem* rec, tempAutoRelInfoArray& result, BaseTaskInfo* context, SmallBittabForTables& deja, const vector<uBYTE>& inWayOfLocking)
{
	VError err = VE_OK;
	DepRelationArrayIncluded rels;
	onTable->CopyRelNto1Deps(rels);
	for (DepRelationArrayIncluded::Iterator cur = rels.First(), end = rels.End(); cur != end && err == VE_OK; cur++)
	{
		Relation* rel = *cur;
		Table* dest = rel->GetDestTable();
		if (dest != nil)
		{
			sLONG tablenum = dest->GetNum();
			bool okToDo = true;
			if (tablenum <= inWayOfLocking.size() && inWayOfLocking[tablenum-1] == 255)
				okToDo = false;
				
			if (okToDo && !deja.isOn(tablenum))
			{
				if (rel->IsAutoLoadNto1(context))
				{
					tempAutoRelInfo x;
					x.fTable = dest;
					if (rec == nil)
						x.fRecord = nil;
					else
					{
						FicheInMem* rec2;
						err = rel->ActivateManyToOne(rec, rec2, context, false, true, (tablenum <= inWayOfLocking.size()) ? (DB4D_Way_of_Locking)inWayOfLocking[tablenum-1] : DB4D_Do_Not_Lock);
						x.fRecord.Adopt(rec2);
					}

					if (err == VE_OK)
					{
						deja.Set(tablenum);
						result.push_back(x);
					}
				}
			}
		}

	}
	return err;
}


VError Table::ActivateAutomaticRelations_N_To_1(FicheInMem* mainrec, vector<CachedRelatedRecord>& outResult, BaseTaskInfo* context, const vector<uBYTE>& inWayOfLocking)
{
	VError err = VE_OK;

	try
	{
		SmallBittabForTables deja;
		deja.Set(GetNum());
		tempAutoRelInfoArray records, temprecs;

		VRefPtr<CDB4DBase> xbase(Owner->RetainBaseX(), false);

		err = FillAutoRelInfo(this, mainrec, temprecs, context, deja, inWayOfLocking);
		if (err == VE_OK)
		{
			while ((!temprecs.empty()) && err == VE_OK)
			{
				tempAutoRelInfoArray temprecs2;
				copy(temprecs.begin(), temprecs.end(), back_insert_iterator<tempAutoRelInfoArray>(records));
				for (tempAutoRelInfoArray::iterator cur = temprecs.begin(), end = temprecs.end(); cur != end && err == VE_OK; cur++)
				{
					err = FillAutoRelInfo(cur->fTable, cur->fRecord, temprecs2, context, deja, inWayOfLocking);
				}
				temprecs.clear();
				temprecs.swap(temprecs2);
			}

			if (err == VE_OK)
			{
				outResult.resize(records.size());
				sLONG i = 0;
				for (tempAutoRelInfoArray::iterator cur = records.begin(), end = records.end(); cur != end; cur++, i++)
				{
					Table* tt = cur->fTable;
					FicheInMem* rec = cur->fRecord;
					CDB4DRecord* xrec = nil;
					CDB4DSelection* xsel = nil;
					CachedRelatedRecord* p = &(outResult[i]);
					deja.Set(tt->GetNum());
					//Selection* sel = new PetiteSel(tt->GetDF(), GetOwner());
					Selection* sel;
					if (tt->GetDF() == nil)
						sel = new PetiteSel(nil, tt->GetOwner(), tt->GetNum());
					else
						sel = new PetiteSel(tt->GetDF());

					if (sel == nil)
						err = ThrowBaseError(memfull);
					else
					{
						xsel = new VDB4DSelection(VDBMgr::GetManager(), VImpCreator<VDB4DBase>::GetImpObject(xbase), tt, sel);
						if (rec != nil)
						{
							sel->FixFic(1);
							sel->PutFic(0, rec->GetNum());
							xrec = new VDB4DRecord(VDBMgr::GetManager(), rec, context->GetEncapsuleur());
							if (xrec != nil)
								rec->Retain();
						}
						p->AdoptRecord(xrec);
						p->AdoptSelection(xsel);
						p->SetTableNum(tt->GetNum());
					}

				}
			}
		}
	}
	catch (...)
	{
		err = ThrowBaseError(memfull);
	}

	return err;
}


VError Table::ActivateAutomaticRelations_1_To_N(FicheInMem* mainrec, vector<CachedRelatedRecord>& outResult, BaseTaskInfo* context, 
												const vector<uBYTE>& inWayOfLocking, Field* onField, Boolean oldvalues,
												Boolean AutomaticOnly, Boolean OneLevelOnly)
{
	VError err = VE_OK;
	DepRelationArrayIncluded rels;
	CopyRel1toNDeps(rels);
	VRefPtr<CDB4DBase> xbase(Owner->RetainBaseX(), false);

	SmallBittabForTables deja;
	deja.Set(GetNum());

	tempAutoRel_1_N_InfoArray firstlevel;

	for (DepRelationArrayIncluded::Iterator cur = rels.First(), end = rels.End(); cur != end && err == VE_OK; cur++)
	{
		Relation* rel = *cur;
		Table* dest = rel->GetSourceTable();
		if (dest != nil && dest != this && (onField == nil || onField == rel->GetDest()))
		{
			sLONG tablenum = dest->GetNum();
			{
				bool okToDo = true;
				if (tablenum <= inWayOfLocking.size() && inWayOfLocking[tablenum-1] == 255)
					okToDo = false;

				if (okToDo && !deja.isOn(tablenum) && ((!AutomaticOnly) || rel->IsAutoLoad1toN(context)) )
				{
					tempAutoRel_1_N_Info temp;
					Selection* sel = nil;
					FicheInMem* rec = nil;
					if (mainrec != nil)
					{
						DB4D_Way_of_Locking wayOfLocking = ((sLONG)inWayOfLocking.size() >= tablenum) ? (DB4D_Way_of_Locking)inWayOfLocking[tablenum-1] : DB4D_Do_Not_Lock;
						err = rel->ActivateOneToMany(mainrec, sel, context, oldvalues, true, DB4D_Do_Not_Lock);
						if (sel != nil && !sel->IsEmpty())
						{
							rec = dest->GetDF()->LoadRecord(sel->GetFic(0), err, wayOfLocking, context, true);
						}
					}
					else
					{
						//sel = new PetiteSel(dest->GetDF(), GetOwner());
						if (dest->GetDF() == nil)
							sel = new PetiteSel(nil, dest->GetOwner(), dest->GetNum());
						else
							sel = new PetiteSel(dest->GetDF());
					}
					deja.Set(tablenum);
					temp.fRecord.Adopt(rec);
					temp.fSel.Adopt(sel);
					temp.fTable = dest;
					firstlevel.push_back(temp);
				}
			}
		}
	}

	tempAutoRelInfoArray records, temprecs;

	if (!OneLevelOnly)
	{
		for (tempAutoRel_1_N_InfoArray::iterator cur = firstlevel.begin(), end = firstlevel.end(); cur != end && err == VE_OK; cur++)
		{
			tempAutoRelInfoArray temprecs2;
			err = FillAutoRelInfo(cur->fTable, cur->fRecord, temprecs2, context, deja, inWayOfLocking);
			copy(temprecs2.begin(), temprecs2.end(), back_insert_iterator<tempAutoRelInfoArray>(temprecs));
		}

		while ((!temprecs.empty()) && err == VE_OK)
		{
			tempAutoRelInfoArray temprecs2;
			copy(temprecs.begin(), temprecs.end(), back_insert_iterator<tempAutoRelInfoArray>(records));
			for (tempAutoRelInfoArray::iterator cur = temprecs.begin(), end = temprecs.end(); cur != end && err == VE_OK; cur++)
			{
				err = FillAutoRelInfo(cur->fTable, cur->fRecord, temprecs2, context, deja, inWayOfLocking);
			}
			temprecs.clear();
			temprecs.swap(temprecs2);
		}
	}

	sLONG nb =(sLONG) (firstlevel.size() + records.size());

	outResult.resize(nb);
	sLONG i = 0;

	for (tempAutoRel_1_N_InfoArray::iterator cur = firstlevel.begin(), end = firstlevel.end(); cur != end && err == VE_OK; cur++, i++)
	{
		CachedRelatedRecord* p = &(outResult[i]);

		Table* tt = cur->fTable;

		Selection* sel = cur->fSel;
		RetainRefCountable(sel);

		FicheInMem* rec = cur->fRecord;
		RetainRefCountable(rec);

		CDB4DRecord* xrec = nil;
		CDB4DSelection* xsel = nil;

		xsel = new VDB4DSelection(VDBMgr::GetManager(), VImpCreator<VDB4DBase>::GetImpObject(xbase), tt, sel);
		if (rec != nil)
		{
			xrec = new VDB4DRecord(VDBMgr::GetManager(), rec, context->GetEncapsuleur());
		}
		p->AdoptRecord(xrec);
		p->AdoptSelection(xsel);
		p->SetTableNum(tt->GetNum());
	}

	if (!OneLevelOnly)
	{
		for (tempAutoRelInfoArray::iterator cur = records.begin(), end = records.end(); cur != end && err == VE_OK; cur++, i++)
		{
			CachedRelatedRecord* p = &(outResult[i]);

			Table* tt = cur->fTable;

			Selection* sel;
			if (tt->GetDF() == nil)
				sel = new PetiteSel(nil, tt->GetOwner(), tt->GetNum());
			else
				sel = new PetiteSel(tt->GetDF());
			//RetainRefCountable(sel);

			FicheInMem* rec = cur->fRecord;
			RetainRefCountable(rec);

			CDB4DRecord* xrec = nil;
			CDB4DSelection* xsel = nil;

			xsel = new VDB4DSelection(VDBMgr::GetManager(), VImpCreator<VDB4DBase>::GetImpObject(xbase), tt, sel);
			if (rec != nil)
			{
				sel->FixFic(1);
				sel->PutFic(0, rec->GetNum());
				xrec = new VDB4DRecord(VDBMgr::GetManager(), rec, context->GetEncapsuleur());
			}
			p->AdoptRecord(xrec);
			p->AdoptSelection(xsel);
			p->SetTableNum(tt->GetNum());
		}
	}

	return err;
}


RecIDType Table::GetMaxRecords(BaseTaskInfo* context)
{
	if (fIsRemote)
	{
		return GetRemoteMaxRecordsInTable();
	}
	else
		return DF->GetMaxRecords(context);
}


#if AllowSyncOnRecID

VError Table::GetOneRow(VStream& inStream, BaseTaskInfo* context, sBYTE& outAction, uLONG& outStamp, RecIDType& outRecID, vector<VValueSingle*>& outValues)
{
	VError err = VE_OK;

	err = inStream.GetByte(outAction);
	if (outAction != 0)
	{
		if (err == VE_OK)
			err = inStream.GetLong(outRecID);
		if (err == VE_OK)
			err = inStream.GetLong(outStamp);

		if (err == VE_OK)
		{
			if (outAction == DB4D_SyncAction_Update)
			{
				sLONG nbcol;
				err = inStream.GetLong(nbcol);
				outValues.resize(nbcol, nil);
				fill(&outValues[0], &outValues[nbcol-1], (VValueSingle*)nil);
				for (sLONG i = 0; i < nbcol && err == VE_OK; i++)
				{
					VValueSingle* cv = nil;
					uWORD typ;
					err = inStream.GetWord(typ);
					if (err == VE_OK)
					{
						if (typ != VK_EMPTY)
						{
							cv = (VValueSingle*)VValue::NewValueFromValueKind(typ);
							if (cv != nil)
							{
								outValues[i] = cv;
								err = cv->ReadFromStream(&inStream);
							}
							else
								err = ThrowBaseError(VE_DB4D_NOT_FOUND);
						}
					}
				}

			}
		}
	}
	else
	{
		outRecID = -1;
		outStamp = 0;
	}

	return err;
}

#endif


VError Table::GetOneRow(IReplicationInputFormatter& inFormatter, BaseTaskInfo* context, sBYTE& outAction, uLONG8& outStamp, VTime& outTimeStamp, vector<VValueSingle*>& outPrimKey, vector<VValueSingle*>& outValues)
{
	VError err = VE_OK;

	err = inFormatter.GetAction(outAction);
	if (outAction != 0)
	{
		sBYTE bNbPrimVal;
		if (err == VE_OK)
			err = inFormatter.GetPrimaryKeyCount(bNbPrimVal);
		if (err == VE_OK)
		{
			outPrimKey.resize(bNbPrimVal, nil);
			assert(bNbPrimVal != 0);
			if (bNbPrimVal != 0)
				fill(&outPrimKey[0], &outPrimKey[bNbPrimVal-1], (VValueSingle*)nil);
			for (sLONG i = 0; i < bNbPrimVal; i++)
			{
				VValueSingle* cv = nil;
				uWORD typ;
				err = inFormatter.GetPrimaryKeyType(typ);
				if (err == VE_OK)
				{
					if (typ != VK_EMPTY)
					{
						cv = (VValueSingle*)VValue::NewValueFromValueKind(typ);
						if (cv != nil)
						{
							outPrimKey[i] = cv;
							err = inFormatter.GetVValue(*cv);
						}
						else
							err = ThrowBaseError(VE_DB4D_NOT_FOUND);
					}
				}
			}
		}
		if (err == VE_OK)
			err = inFormatter.GetStamp(outStamp);
		if (err == VE_OK)
			err = inFormatter.GetTimeStamp(outTimeStamp);

		if (err == VE_OK)
		{
			if (outAction == DB4D_SyncAction_Update)
			{
				sLONG nbcol;
				err = inFormatter.GetFieldCount(nbcol);
				outValues.resize(nbcol, nil);
				fill(&outValues[0], &outValues[nbcol-1], (VValueSingle*)nil);
				for (sLONG i = 0; i < nbcol && err == VE_OK; i++)
				{
					VValueSingle* cv = nil;
					uWORD typ;
					err = inFormatter.GetFieldType(typ);
					if (err == VE_OK)
					{
						if (typ != VK_EMPTY)
						{
							cv = (VValueSingle*)VValue::NewValueFromValueKind(typ);
							if (cv != nil)
							{
								outValues[i] = cv;
								err = inFormatter.GetVValue(*cv);
							}
							else
								err = ThrowBaseError(VE_DB4D_NOT_FOUND);
						}
					}
				}

			}
		}
	}
	else
	{
		outPrimKey.clear();
		outStamp = 0;
	}

	return err;
}



VError Table::GetFragmentation(sLONG8& outTotalRec, sLONG8& outFrags, CDB4DBaseContext* inContext)
{
	VError err = VE_OK;
	if (fIsRemote)
	{
		IRequest *req = GetOwner()->CreateRequest( inContext, Req_GetTableFragmentation + kRangeReqDB4D);
		if (req == NULL)
		{
			err = ThrowBaseError(memfull, noaction);
		}
		else
		{
			req->PutBaseParam(GetOwner());
			req->PutTableParam(this);

			err = req->GetLastError();
			if (err == VE_OK)
			{
				err = req->Send();
				if (err == VE_OK)
				{
					outTotalRec = req->GetLong8Reply(err);
					outFrags = req->GetLong8Reply(err);
				}
			}
			req->Release();
		}
	}
	else
	{
		if (DF == nil)
		{
			outTotalRec = 0;
			outFrags = 0;
		}
		else
			err = DF->GetFragmentation(outTotalRec, outFrags);
	}
	return err;
}








								/* ----------------------------------------------- */


TableRegular::TableRegular(Base4D *owner, sLONG xnres, Boolean canBeSaved, Boolean isRemote) 
: Table(owner, xnres, canBeSaved, isRemote), fExtra(&FID.ExtraAddr, &FID.ExtraLen, owner->GetStructure(), xnres, -1, DBOH_ExtraTableProperties, isRemote)
{ 
	sLONG i;
	FID.typ=0;
	//FID.fInvisible = false;
	FID.fHasPrimaryKey = false;
	FID.fNotFullyDeleteRecords = true;
	FID.nbcrit = 0;
	FID.RecordName[0] = 0;
	FID.ExtraAddr = 0;
	FID.ExtraLen = 0;

	FID.UnusedAddr1 = 0;
	FID.UnusedLen1 = 0;
	FID.UnusedAddr2 = 0;
	FID.UnusedLen2 = 0;
	FID.fStamp = 1;

	FID.fSchemaID = 0;
	FID.fKeepStamps = kDefaultKeepStamp;
	if (owner->GetStructure() == nil || !canBeSaved)
		FID.fKeepStamps = 0;
	FID.fKeepSyncInfo = 0;
	for (i = 0; i<maxTrigFile; i++) { FID.AllowTrigger[i] = false; }
	for (i = 0; i<maxPermFile; i++) { FID.Perms[i] = 0; }
}


TableRegular::~TableRegular()
{
}


VError TableRegular::save(uBOOL inDatas, bool cantouch)
{
	CritereDISK *p,*p2;
	sLONG i;
	VError err = VE_OK;
	StructElemDef* tabdef = nil;
	void* data = nil;
	Boolean newone = false;

	sLONG sizename = 0;
	sLONG nbprim = PrimaryKey.GetCount();
	if (nbprim > 0)
		sizename = kMaxTableNameNbWords*2;
	sLONG sizeblock = tc.GetCount()*sizeof(CritereDISK)+sizeof(FichierDISK)+(sizeof(sLONG)*(nbprim+1))+sizename;

	if (testAssert(!fIsRemote))
	{
		occupe();
		sLONG numresindata = 0;
		Boolean oktosave = true;
		if (inDatas)
		{
			if (DF == nil)
				oktosave = false;
			else
				numresindata = DF->GetTableDefNumInData();
		}
		if (fTableCanBesaved && oktosave) // resmap cannot be saved
		{
			if (Owner->StoredAsXML() && !inDatas)
			{
				fName.ToBlock( &FID.nom, sizeof( FID.nom), VTC_UTF_16, false, true);
				fRecordName.ToBlock( &FID.RecordName, sizeof( FID.RecordName), VTC_UTF_16, false, true);
				FID.fStamp = fStamp;
				FID.fHasPrimaryKey = nbprim > 0;
				if (cantouch)
					Owner->TouchXML();
				save(true, cantouch);
			}
			else
			{
				if (err == VE_OK)
				{
					if (inDatas ? numresindata != 0 : nres != 0)
					{
						if (inDatas)
							tabdef = Owner->LoadTableDefInDatas(numresindata-1, err);
						else
							tabdef = Owner->LoadTableDef(nres-1, err);

						if (tabdef == nil && err== VE_OK)
						{
							if (inDatas)
								numresindata = 0;
							else
								nres = 0;
							newone = true;
						}
						else
						{
							if (err == VE_OK)
							{
								if (tabdef->GetDataLen() == sizeblock)
								{
									data = tabdef->GetDataPtr();
								}
							}
						}
					}
					else
						newone = true;

					if (err == VE_OK)
					{
						if (data == nil)
						{
							data = GetFastMem(sizeblock, false, 'TabD');
							if (data == nil)
							{
								err = ThrowError(memfull, DBaction_SavingTableDef);
							}
						}
						
						if (err == VE_OK)
						{
							fName.ToBlock( &FID.nom, sizeof( FID.nom), VTC_UTF_16, false, true);
							fRecordName.ToBlock( &FID.RecordName, sizeof( FID.RecordName), VTC_UTF_16, false, true);
							FID.fStamp = fStamp;
							FID.fHasPrimaryKey = nbprim > 0;
							CritereDISK nullfield;
							nullfield.nom[0] = 0;
							nullfield.typ = DB4D_NoType;
							*((FichierDISK*)data) = FID;
							if (inDatas)
							{
								((FichierDISK*)data)->ExtraAddr = 0;
								((FichierDISK*)data)->ExtraLen = 0;
							}
							p = (CritereDISK*)(((char*)data)+sizeof(FichierDISK));
							for (i=1;i<=tc.GetCount();i++)
							{
								if (tc[i] != nil)
									p2=tc[i]->getCRD();
								else
									p2 = &nullfield;
								*p = *p2;
								if (inDatas)
								{
									p->ExtraLen = 0;
									p->ExtraAddr = 0;
								}
								p++;
							}

							sLONG* pl = (sLONG*)p;
							*pl++ = -nbprim;
							for (NumFieldArray::Iterator cur = PrimaryKey.First(), end = PrimaryKey.End(); cur != end; cur++)
							{
								*pl++ = *cur;
							}
							if (nbprim > 0)
							{
								fPrimaryKeyName.ToBlock( pl, sizename, VTC_UTF_16, false, true);
							}

							
							if (inDatas ? numresindata == 0 : nres == 0)
							{
								if (inDatas)
									tabdef = new StructElemDef(Owner, -1, DBOH_TableDefElem, ObjCache::TableDefAccess);
								else
									tabdef = new StructElemDef(Owner->GetStructure(), -1, DBOH_TableDefElem, ObjCache::TableDefAccess);

								if (tabdef == nil)
								{
									err = ThrowError(memfull, DBaction_SavingTableDef);
									FreeFastMem(data);
								}
							}

							if (err == VE_OK)
							{
								tabdef->SetData(data, sizeblock);
								if (inDatas)
									err = Owner->SaveTableRefInDatas(tabdef);
								else
									err = Owner->SaveTableRef(tabdef);

								if (err != VE_OK)
								{
									if (newone)
									{
										tabdef->libere();
										delete tabdef;
										tabdef = nil;
									}
								}
							}
						}
					}
				}
				
				if (tabdef != nil)
				{
					if (inDatas)
						DF->SetTableDefNumInData(tabdef->GetNum()+1);
					else
						nres = tabdef->GetNum()+1;
					tabdef->libere();
				}

				if (!inDatas)
				{
					save(true, cantouch);
				}
			}
		}

		libere();
		
		if (err != VE_OK)
			err = ThrowError(VE_DB4D_CANNOTSAVETABLEDEF, DBaction_SavingTableDef);
	}


	return(err);
}


void TableRegular::GetUUID(VUUID& outID) const
{
	outID.FromBuffer(FID.ID);
}


void TableRegular::SetUUID(const VUUID& inID)
{
	inID.ToBuffer(FID.ID);
}


void TableRegular::GetName(VString& s) const
{ 
	//s.FromBlock(&FID.nom[1],FID.nom[0] * sizeof( UniChar), VTC_UTF_16);
	s = fName;
}


VError TableRegular::SetFullyDeleteRecords(Boolean FullyDeleteState, CDB4DBaseContextPtr inContext)
{
	VError err = VE_OK;

	if (fIsRemote)
	{
		IRequest* req = Owner->CreateRequest(inContext, Req_SetFullyDeleteRecords + kRangeReqDB4D);
		if (req == nil)
			err = ThrowError(memfull, noaction);
		else
		{
			req->PutBaseParam(Owner);
			req->PutTableParam(this);
			req->PutBooleanParam(FullyDeleteState);

			err = req->GetLastError();
			if (err == VE_OK)
			{
				err = req->Send();
				if (err == VE_OK)
				{
					occupe();
					FID.fNotFullyDeleteRecords = !FullyDeleteState;
					libere();
				}
			}
			req->Release();
		}
	}
	else
	{
		if (Owner->OkToUpdate(err))
		{
			occupe();
			FID.fNotFullyDeleteRecords = !FullyDeleteState;
			Touch();
			err = save();
			libere();
			Owner->ClearUpdating();
		}
	}

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTUPDATETABLEDEF, DBaction_ChangingTableName);
	return err;
}


Boolean TableRegular::GetFullyDeleteRecordsState() const
{
	return !FID.fNotFullyDeleteRecords;
}


VError TableRegular::SetName(const VString& s, CDB4DBaseContext* inContext, Boolean inNotify, Boolean inWithNameCheck)
{
	VError err = VE_OK;

	if (fIsRemote)
	{
		IRequest* req = Owner->CreateRequest(inContext, Req_SetTableName + kRangeReqDB4D);
		if (req == nil)
			err = ThrowError(memfull, DBaction_ChangingTableName);
		else
		{
			VStream* reqsend = req->GetOutputStream();
			Owner->GetUUID().WriteToStream(reqsend);
			reqsend->PutLong(GetNum());
			s.WriteToStream(reqsend);
			reqsend->PutWord( (inWithNameCheck) ? 1 : 0);		// sc 11/04/2007 write check name mode
			err = reqsend->GetLastError();
			if (err == VE_OK)
			{
				err = req->Send();
				if (err == VE_OK)
				{
					occupe();

					//s.ToBlock( &FID.nom, sizeof( FID.nom), VTC_UTF_16, false, true);
					fName = s;
					if (inNotify) 
						RegisterForLang();	
					libere();
				}
			}
			req->Release();
		}
	}
	else
	{
		if (!fTableCanBesaved || Owner->OkToUpdate(err))
		{
			Table* other = Owner->FindAndRetainTableRef(s);

			occupe();

			if ( !inWithNameCheck || (s.GetLength() <= kMaxTableNameLength && IsValid4DTableName(s)) )
			{
				if ( !inWithNameCheck || (other == nil || other == this) )
				{
					//s.ToBlock( &FID.nom, sizeof( FID.nom), VTC_UTF_16, false, true);
					fName = s;
					Touch();
					if (inNotify) 
						RegisterForLang();
					err = save();
				}
				else
				{
					err = ThrowError(VE_DB4D_TABLENAMEDUPLICATE, DBaction_ChangingTableName);	// sc 28/03/2007
				}

			}
			else
			{
				err = ThrowError(VE_DB4D_INVALIDTABLENAME, DBaction_ChangingTableName);
			}

			if (other != nil)
				other->Release();

			libere();
			if (fTableCanBesaved)
				Owner->ClearUpdating();
		}
	}

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTUPDATETABLEDEF, DBaction_ChangingTableName);
	return err;

}


VError TableRegular::SetSchema( DB4D_SchemaID inSchemaID, CDB4DBaseContext* inContext)
{
	VError err = VE_OK;

	if (fIsRemote)
	{
		IRequest *req = GetOwner()->CreateRequest( inContext, Req_SetTableSchema + kRangeReqDB4D);
		if (req == NULL)
		{
			err = ThrowBaseError(memfull, noaction);
		}
		else
		{
			req->PutBaseParam( GetOwner());
			req->PutTableParam( this);
			req->PutLongParam( inSchemaID);
		
			err = req->GetLastError();
			if (err == VE_OK)
			{
				err = req->Send();
				if (err == VE_OK)
				{
					occupe();
					FID.fSchemaID = inSchemaID;
					libere();
				}
			}
			req->Release();
		}
	}
	else
	{
		if (Owner->OkToUpdate(err))
		{
			occupe();
			FID.fSchemaID = inSchemaID;
			Touch();
			err = save();
			libere();
			Owner->ClearUpdating();
		}
	}
	return err;
}



VError TableRegular::SetNameSilently( const VString& inName)
{
	if (testAssert(!fIsRemote && !fRegistered))
		fName = inName;

	return VE_OK;
}


void TableRegular::Touch() 
{ 
//	EntityModel::ClearCacheTableEM();
	if (fTableCanBesaved)
	{
		if (testAssert(!fIsRemote))
		{
			fStamp++; 
			if (fStamp == 0)
				fStamp = 1;
			FID.fStamp = fStamp;
			if (Owner != nil) 
				Owner->Touch(); 
		}
	}
}



void TableRegular::GetRecordName(VString& s) const
{ 
	//s.FromBlock(&FID.RecordName[1],FID.RecordName[0] * sizeof( UniChar), VTC_UTF_16);
	s = fRecordName;
}



VError TableRegular::SetRecordName(const VString& s, CDB4DBaseContext* inContext, Boolean inNotify)
{
	VError err = VE_OK;
	if (fIsRemote)
	{
	}
	else
	{
		if (Owner->OkToUpdate(err))
		{
			Table* other = Owner->FindAndRetainTableRefByRecordName(s);
			occupe();

			if (s.GetLength() <= kMaxTableNameLength && IsValid4DName(s) && (other == nil || other == this))
			{
				//s.ToBlock( &FID.RecordName, sizeof( FID.RecordName), VTC_UTF_16, false, true);
				fRecordName = s;
				Touch();
				if (inNotify) 
					RegisterForLang();
				err = save();

			}
			else
				err = ThrowError(VE_DB4D_INVALIDRECORDNAME, DBaction_ChangingTableName);

			if (other != nil)
				other->Release();

			libere();
			Owner->ClearUpdating();
		}
	}

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTUPDATETABLEDEF, DBaction_ChangingTableName);
	return err;

}


void TableRegular::InitExtraPropHeaderLaterForFields()
{
	occupe();
	for (FieldArray::Iterator cur = tc.First(), end = tc.End(); cur != end; cur++)
	{
		if (*cur != nil)
			(*cur)->SetPosInRec((*cur)->GetPosInRec());
	}
	libere();
}


const VValueBag* TableRegular::RetainExtraProperties(VError &err, CDB4DBaseContext* inContext)
{
	const VValueBag* bag = fExtra.RetainExtraProperties(err);
	return bag;
}


VError TableRegular::SetExtraProperties(VValueBag* inExtraProperties, bool inNotify, CDB4DBaseContext* inContext, bool loadonly)
{
	VError err = VE_OK;

	if (fIsRemote)
	{
		if (inExtraProperties != NULL)
		{
			IRequest *req = Owner->CreateRequest( inContext, Req_SetTableExtraProperties + kRangeReqDB4D);
			if (req == NULL)
			{
				err = ThrowBaseError(memfull, noaction);
			}
			else
			{
				req->PutBaseParam( GetOwner());
				req->PutTableParam( this);
				req->PutValueBagParam( *inExtraProperties);
				
				err = req->GetLastError();
				if (err == VE_OK)
				{
					err = req->Send();
					if (err == VE_OK)
					{
						err = fExtra.SetExtraProperties( inExtraProperties);
					}
				}
				req->Release();
			}
		}
	}
	else
	{
		if (loadonly || Owner->OkToUpdate(err))
		{
			err = fExtra.SetExtraProperties(inExtraProperties, loadonly);

			// L.E. if the table has not been saved yet, don't save it now.
			// (creating a db from xml)
			if (!loadonly)
			{
				if (nres != 0 && !Owner->StoredAsXML())
					save();

				Owner->ClearUpdating();
				Touch();

				if (inNotify && (VDBMgr::GetCommandManager() != nil))
				{
					VUUID UID_base = Owner->GetUUID();

					VUUID UID_table;
					GetUUID( UID_table);

					VDBMgr::GetCommandManager()->Tell_UpdateTable( UID_base, UID_table, numfile, TUT_ExtraPropertiesChanged);
				}
			}
		}
	}
	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTUPDATETABLEDEF, DBaction_ChangingTableName);
	return err;
}


VError TableRegular::Update(VStream* dataget)
{
	VError err = VE_OK;
	occupe();
	VUUIDBuffer oldid = FID.ID;
	FichierDISK xfid;
	VValueBag *tableExtra = NULL;

	err = dataget->GetData(&xfid, sizeof(xfid));
	if (err == VE_OK)
	{
		tableExtra = new VValueBag();
		err = tableExtra->ReadFromStream( dataget);		// sc 22/08/2007 read the extra properties
	}
	if (err == VE_OK)
	{
		if (dataget->NeedSwap())
			xfid.SwapBytes();
		if (xfid.ID != oldid)
		{
			UnRegisterForLang();
		}
		FID = xfid;
		fExtra.SetExtraProperties( tableExtra);
		if (FID.nbcrit >= 0 && FID.nbcrit <= kMaxFields)
		{
			if (FID.nbcrit > tc.GetCount())
			{
				if (!tc.SetCount(FID.nbcrit, nil))
					err = ThrowError(memfull, DBaction_UpdatingTableDef);
			}
			else
			{
				if (FID.nbcrit < tc.GetCount())
				{
					for (sLONG i = FID.nbcrit+1; i<=tc.GetCount(); i++)
					{
						Field* cri = tc[i];
						if (cri != nil)
							DeleteField(cri, nil, nil, true);
					}
					tc.SetCount(FID.nbcrit);
				}
			}
			if (err == VE_OK)
			{
				for (sLONG i = 1; i <= FID.nbcrit && err == VE_OK; i++)
				{
					CritereDISK crd;
					VValueBag *fieldExtra = NULL;
					err = dataget->GetData(&crd, sizeof(crd));
					if (err == VE_OK)
					{
						fieldExtra = new VValueBag();
						err = fieldExtra->ReadFromStream( dataget);		// sc 23/08/2007 read the extra properties
					}
					if (err == VE_OK)
					{
						if (dataget->NeedSwap())
							crd.SwapBytes();
						Field* cri = tc[i];
						if (crd.typ == DB4D_NoType)
						{
							if (cri != nil)
							{
								DeleteField(cri, nil, nil, true);
							}
						}
						else
						{
							if (cri == nil)
							{
								cri=new Field(&crd, fieldExtra, i, this);
								if (cri == nil)
									err = ThrowError(memfull, DBaction_UpdatingTableDef);
								else
								{
									tc[i]=cri;
									VUUID xid;
									cri->GetUUID(xid);
									Owner->AddObjectID(objInBase_Field, cri, xid);
									Owner->AddFieldRec(GetNum(), i);
									cri->RegisterForLang();
								}

							}
							else
							{
								err = cri->UpdateField( &crd, fieldExtra);
							}
						}

					}
					ReleaseRefCountable( &fieldExtra);
				}
			}
		}
		else
			err = ThrowError(VE_DB4D_INVALID_FIELDNUM, DBaction_UpdatingTableDef);
	}

	if (err == VE_OK)
	{
		// sc 09/03/2010 ACI0065041, read the primary key
		err = fPrimaryKeyName.ReadFromStream( dataget);
		if (err == VE_OK)
		{
			sLONG count = 0;
			err = dataget->GetLong( count);
			if (err == VE_OK)
			{
				PrimaryKey.Destroy();
				
				for (VIndex i = 0 ; i < count && err == VE_OK ; ++i)
				{
					sLONG fieldNum = 0;
					err = dataget->GetLong( fieldNum);
					if (err == VE_OK)
						PrimaryKey.Add( fieldNum);
				}
			}
		}
	}
	
	if (err == VE_OK)
	{
		fStamp = FID.fStamp;
		fName.FromBlock(&FID.nom[1],FID.nom[0] * sizeof( UniChar), VTC_UTF_16);
		fRecordName.FromBlock(&FID.RecordName[1],FID.RecordName[0] * sizeof( UniChar), VTC_UTF_16);
		RegisterForLang(false);		// sc 30/11/2007 fields have been already registered before
		if (FID.ID != oldid)
		{
			VUUID xoldid(oldid);
			VUUID xid(FID.ID);
			Owner->DelObjectID(objInBase_Table, this, xoldid);
			Owner->AddObjectID(objInBase_Table, this, xid);
		}
	}

	ReleaseRefCountable( &tableExtra);

	ReCalc();

	libere();

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTUPDATETABLEDEF, DBaction_ChangingTableName);

	return err;
}


VError TableRegular::SendToClient(VStream* datasend)
{
	VError err = VE_OK;
	occupe();
	FichierDISK fid = FID;
	sLONG nb = fid.nbcrit;

	assert(nb == tc.GetCount());

	VValueBag *tableExtra = fExtra.RetainExtraProperties( err);
	if (err != VE_OK)
		ReleaseRefCountable( &tableExtra);
	if (tableExtra == NULL)
		tableExtra = new VValueBag();
		
	libere();
	err = datasend->PutLong(numfile);
	if (err == VE_OK)
		err = datasend->PutData(&FID, sizeof(FID));
	if (err == VE_OK)
		err = tableExtra->WriteToStream( datasend);		// sc 22/08/2007 send the extra properties
	ReleaseRefCountable( &tableExtra);
	
	CritereDISK crd;
	if (err == VE_OK)
	{
		for (sLONG i = 1; i <= nb && err == VE_OK; i++)
		{
			const VValueBag *fieldExtra = NULL;
			occupe();
			Field* ff = tc[i];
			if (ff == nil)
			{
				_raz(&crd, sizeof(crd));
				crd.typ = DB4D_NoType;
			}
			else
			{
				ff->occupe();
				crd = *(ff->getCRD());
				fieldExtra = ff->RetainExtraProperties( err, NULL);
				if (err != VE_OK)
					ReleaseRefCountable( &fieldExtra);
				ff->libere();
			}
			libere();
			
			if (fieldExtra == NULL)
				fieldExtra = new VValueBag();
			err = datasend->PutData(&crd, sizeof(crd));
			if (err == VE_OK)
				err = fieldExtra->WriteToStream( datasend);		// sc 23/08/2007 send the extra properties
			ReleaseRefCountable( &fieldExtra);
		}
	}
	
	if (err == VE_OK)
	{
		// sc 09/03/2010, ACI0065041 send the primary key
		err = fPrimaryKeyName.WriteToStream( datasend);
		if (err == VE_OK)
			err = datasend->PutLong( PrimaryKey.GetCount());
		if (err == VE_OK)
		{
			for (NumFieldArray::Iterator iter = PrimaryKey.First(), end = PrimaryKey.End() ; iter != end && err == VE_OK ; ++iter)
			{
				err = datasend->PutLong( *iter);
			}
		}
	}
	
	return err;
}



VError TableRegular::change( const CritereDISK* p, sLONG nbcrit, sLONG start, Boolean inWithNamesCheck, CDB4DBaseContext* inContext, 
							Boolean inNotify, VProgressIndicator* progress, Boolean isTrueStructField, bool cantouch)
{
	sLONG i,ii,oldnbc,newnbcrit;
	VError err = VE_OK;

	if (fIsRemote)
	{
		assert(false);
	}
	else
	{
		if (!fTableCanBesaved || Owner->OkToUpdate(err))
		{
			Field* cri;
			const CritereDISK* p2 = p;
			Boolean DejaOccupeDF = false;

			VArrayOf<VString> deja;

			deja.SetOwnership(true);
			deja.SetUnique(true);

			occupe();

			for (i=1; (i<=nbcrit) && (err == VE_OK) ;i++)
			{
				if (p2->typ != DB4D_NoType)
				{
					VString s;

					s.FromBlock( &(p2->nom[1]), p2->nom[0] * sizeof( UniChar), VTC_UTF_16);

					if (p2->not_null)
					{
						if (DF != nil)
						{
							BaseTaskInfo context(GetOwner(), nil, nil, nil);
							if (!DejaOccupeDF)
							{
								DF->LockRead();
								DejaOccupeDF = true;
							}
							if (DF->GetNbRecords(&context) != 0)
							{
								err = ThrowError(VE_DB4D_CANNOT_SET_FIELD_TO_Not_Null, DBaction_UpdatingTableDef);
								break;
							}
						}
					}

					if (inWithNamesCheck)
					{
						if (s.GetLength() <= kMaxFieldNameLength && IsValid4DFieldName(s))
						{
							Field* other = FindAndRetainFieldRef(s);
							if (other == nil)
							{
								if (!deja.Add(s))
									err = ThrowError(VE_DB4D_FIELDNAMEDUPLICATE, DBaction_UpdatingTableDef);
							}
							else
							{
								other->Release();
								err = ThrowError(VE_DB4D_FIELDNAMEDUPLICATE, DBaction_UpdatingTableDef);
							}
						}
						else
						{
							err = ThrowError(VE_DB4D_INVALIDFIELDNAME, DBaction_UpdatingTableDef);
						}
					}
				}

				p2++;
			}

			if (err == VE_OK)
			{
				oldnbc=tc.GetCount();
				newnbcrit=nbcrit+start;
				if (p!=nil)
				{
					if (newnbcrit>oldnbc)
					{
						if (!tc.AddNSpaces(newnbcrit-oldnbc, true)) 
						{
							err = ThrowError(memfull, DBaction_UpdatingTableDef);
						}
					}

					if (err==VE_OK)
					{
						if (newnbcrit>FID.nbcrit) FID.nbcrit = newnbcrit;
						for (i=1; i<=nbcrit && err == VE_OK ;i++)
						{
							ii=i+start;

							if (p->typ == DB4D_NoType)
							{
								tc[ii] = nil;
							}
							else
							{
								if (ii>=oldnbc)
								{
									cri=new Field( p, ii, this);
									if (cri == nil)
										err = ThrowError(memfull, DBaction_UpdatingTableDef);
									else
									{
										if (!cri->IsValid())
										{
											err = ThrowError(VE_DB4D_INVALIDFIELD, noaction);
										}
										else
										{
											tc[ii]=cri;
											if (isTrueStructField)
											{
												VUUID xid;
												cri->GetUUID(xid);
												Owner->AddObjectID(objInBase_Field, cri, xid);
												Owner->AddFieldRec(GetNum(), ii);
												if (inNotify) 
													cri->RegisterForLang();
											}
										}
									}
								}
								else
								{
									cri=tc[ii];
									if (cri == nil)
									{
										cri=new Field( p, ii, this);
										if (cri == nil)
											err = ThrowError(memfull, DBaction_UpdatingTableDef);
										else
										{
											if (!cri->IsValid())
											{
												err = ThrowError(VE_DB4D_INVALIDFIELD, noaction);
											}
											else
											{
												tc[ii]=cri;
												if (isTrueStructField)
												{
													VUUID xid;
													cri->GetUUID(xid);
													Owner->AddObjectID(objInBase_Field, cri, xid);
													Owner->AddFieldRec(GetNum(), ii);
													if (inNotify) 
														cri->RegisterForLang();
												}
											}
										}
									}
									else
										err = cri->setCRD(p, inContext, inNotify && isTrueStructField, progress);
								}
							}

							p++;
						}
					}
				}

				ReCalc();
			}

			if (DejaOccupeDF)
			{
				DF->Unlock();
			}

			libere();
			if (fTableCanBesaved)
				Owner->ClearUpdating();
			if (cantouch)
				Touch();
		}


		if (err != VE_OK)
			err = ThrowError(VE_DB4D_CANNOTUPDATETABLEDEF, DBaction_UpdatingTableDef);
	}
	return(err);
}



VError TableRegular::load(Boolean inDatas, sLONG nresindata, bool FromDataToStruct)
{
	VError err = VE_OK;

	if (testAssert(!fIsRemote))
	{
		occupe();

		StructElemDef* tabdef = inDatas ? Owner->LoadTableDefInDatas(nresindata-1, err) : Owner->LoadTableDef(nres-1, err);
		if (tabdef != nil && err == VE_OK )
		{
			void* data = tabdef->GetDataPtr();
			Boolean needswap = tabdef->NeedSwap();

			if (data != nil)
			{
				FID = *(FichierDISK*) data;
				if (needswap)
					FID.SwapBytes();

				fName.FromBlock(&FID.nom[1],FID.nom[0] * sizeof( UniChar), VTC_UTF_16);
				fRecordName.FromBlock(&FID.RecordName[1],FID.RecordName[0] * sizeof( UniChar), VTC_UTF_16);

				fStamp = FID.fStamp;

				if (FromDataToStruct || !inDatas)
					RegisterForLang();
				CritereDISK *p2 = nil;
				CritereDISK *p=(CritereDISK*)(((char*)data)+sizeof(FichierDISK));
				if (needswap)
				{
					p2=(CritereDISK*)GetFastMem(sizeof(CritereDISK)*FID.nbcrit, false, 'tmp2');
					if (p2 == nil)
						err = ThrowError(memfull, DBaction_LoadingTableDef);
					else
					{
						vBlockMove((void*)p, (void*)p2, sizeof(CritereDISK)*FID.nbcrit);
						p = p2+FID.nbcrit;
						while (p!=p2)
						{
							p--;
							p->SwapBytes();
						}
						p = p2;
					}
				}
				CritereDISK* p3 = p + FID.nbcrit;
				while (p!=p3)
				{
					p3--;
					if (p3->fStamp == 0)
						p3->fStamp = 1;
				}

				if (err == VE_OK)
				{
					err=change(p,FID.nbcrit, 0, false, nil, FromDataToStruct || !inDatas, nil, FromDataToStruct || !inDatas, false);	// no names check when loading
				}

				if (p2 != nil)
					FreeFastMem((void*)p2);

				if (FID.fHasPrimaryKey && err == VE_OK)
				{
					sLONG* pl = (sLONG*)(((char*)data)+sizeof(FichierDISK)+sizeof(CritereDISK)*FID.nbcrit);
					sLONG nbprim = *pl++;
					if (needswap)
						ByteSwap(&nbprim);
					bool withname = false;
					if (nbprim < 0)
					{
						nbprim = -nbprim;
						withname = true;
					}
					PrimaryKey.SetCount(nbprim);
					bool wrongPrimKey = false;
					for (sLONG i = 1; i <= nbprim; i++)
					{
						sLONG numfield = *pl++;
						if (needswap)
							ByteSwap(&numfield);
						PrimaryKey[i] = numfield;
						if (numfield > 0 && numfield <= FID.nbcrit)
						{
							if (tc[numfield] == nil)
								wrongPrimKey = true;
						}
						else
							wrongPrimKey = true;
					}
					if (withname)
					{
						UniChar* pl2 = (UniChar*)pl;
						fPrimaryKeyName.FromBlock( &pl2[1], pl2[0]*2, VTC_UTF_16);
					}
					if (wrongPrimKey)
						PrimaryKey.SetCount(0);


				}

			}

			tabdef->libere();
		}

		libere();

		if (err != VE_OK)
			err = ThrowError(VE_DB4D_CANNOTLOADTABLEDEF, DBaction_LoadingTableDef);
	}
	return(err);
}



VError TableRegular::SetCrit(sLONG n, const VValueBag& inFieldDefinition, CDB4DBaseContext* inContext, VProgressIndicator* progress)
{
	VStr255	name;
	CritereDISK cri;
	Field *fld;
	VError err = VE_OK;

	if (fIsRemote)
	{
		fld = RetainField(n);
		if (fld != NULL)
		{
			IRequest *req = GetOwner()->CreateRequest( inContext, Req_SetFieldAttributes + kRangeReqDB4D);
			if (req == NULL)
			{
				err = ThrowBaseError(memfull, DBaction_ChangingFieldProperties);
			}
			else
			{
				req->PutBaseParam( GetOwner());
				req->PutTableParam( this);
				req->PutFieldParam( fld);
				req->PutValueBagParam( inFieldDefinition);
				req->PutProgressParam(progress);
			
				err = req->GetLastError();
				if (err == VE_OK)
				{
					err = req->Send();
					if (err == VE_OK)
					{
						VValueBag *extraBag = NULL;
						err = req->GetInputStream()->GetData( &cri, sizeof(cri));
						if (err == VE_OK)
						{
							extraBag = req->RetainValueBagReply( err);
							if (err != VE_OK)
								ReleaseRefCountable( &extraBag);
						}
						if (err == VE_OK)
						{
							if (req->GetInputStream()->NeedSwap())
								cri.SwapBytes();
							err = fld->UpdateField( &cri, extraBag);
						}
						ReleaseRefCountable( &extraBag);
					}
				}
				req->Release();
			}
		}
	}
	else
	{
		if (Owner->OkToUpdate(err))
		{
			occupe();

			fld = RetainField(n);
			if (fld != nil)
			{
				cri = *(fld->getCRD());

				if (inFieldDefinition.GetString( DB4DBagKeys::name, name))
					name.ToBlock( cri.nom, sizeof( cri.nom), VTC_UTF_16, false, true);

				inFieldDefinition.GetLong( DB4DBagKeys::type, cri.typ);

				Boolean xstoreuuid;
				if (inFieldDefinition.GetBoolean( DB4DBagKeys::store_as_UUID, xstoreuuid))
				{
					if ( (cri.typ == VK_STRING || cri.typ == VK_TEXT || cri.typ == VK_STRING_UTF8 || cri.typ == VK_TEXT_UTF8) && xstoreuuid)
						cri.typ = VK_UUID;
				}

				sLONG limitlen;

				if (inFieldDefinition.GetLong( DB4DBagKeys::limiting_length, limitlen) && (cri.typ == VK_STRING || cri.typ == VK_STRING_UTF8) )
				{
					cri.LimitingLength = limitlen;
					if (cri.LimitingLength < 0)
						cri.LimitingLength = 0;
				}

				if (inFieldDefinition.GetLong( DB4DBagKeys::text_switch_size, limitlen) && (cri.typ == VK_TEXT || cri.typ == VK_TEXT_UTF8))
				{
					cri.LimitingLength = limitlen;
					if (cri.LimitingLength < 0)
						cri.LimitingLength = 0;
					else
						cri.LimitingLength = -cri.LimitingLength;
				}

				if (inFieldDefinition.GetLong( DB4DBagKeys::blob_switch_size, limitlen) && (cri.typ == VK_IMAGE || cri.typ == VK_BLOB || cri.typ == VK_BLOB_DB4D))
				{
					cri.LimitingLength = limitlen;
					if (cri.LimitingLength < 0)
						cri.LimitingLength = 0;
					else
						cri.LimitingLength = -cri.LimitingLength;
				}

				Boolean xautoseq;
				if (inFieldDefinition.GetBoolean( DB4DBagKeys::autosequence, xautoseq))
					cri.fAutoSeq = xautoseq ? 1 : 0;

				Boolean xautogen;
				if (inFieldDefinition.GetBoolean( DB4DBagKeys::autogenerate, xautogen))
					cri.fAutoGenerate = xautogen ? 1 : 0;

				Boolean xstoreutf8;
				if (inFieldDefinition.GetBoolean( DB4DBagKeys::store_as_utf8, xstoreutf8))
				{
					cri.fStoreAsUTF8 = xstoreutf8 ? 1 : 0;
					if (cri.typ == VK_STRING && xstoreutf8)
						cri.typ = VK_STRING_UTF8;
					if (cri.typ == VK_TEXT && xstoreutf8)
						cri.typ = VK_TEXT_UTF8;
				}

				Boolean xnot_null;
				if (inFieldDefinition.GetBoolean( DB4DBagKeys::not_null, xnot_null))
					cri.not_null = xnot_null ? 1 : 0;

				Boolean xnevernull;
				if (inFieldDefinition.GetBoolean( DB4DBagKeys::never_null, xnevernull))
					cri.fNeverNull = xnevernull ? 1 : 0;

				Boolean xunique;
				if (inFieldDefinition.GetBoolean( DB4DBagKeys::unique, xunique))
					cri.unique = xunique ? 1 : 0;

				Boolean xouterdata;
				if (inFieldDefinition.GetBoolean( DB4DBagKeys::outside_blob, xouterdata))
					cri.fOuterData = xouterdata ? 1 : 0;

				Boolean xstyledtext;
				if (inFieldDefinition.GetBoolean( DB4DBagKeys::styled_text, xstyledtext))
					cri.fTextIsWithStyle = xstyledtext ? 1 : 0;

				err = fld->setCRD( &cri, inContext, true, progress);

				if (err == VE_OK)
					err = save();

				fld->Release();
			}

			libere();

			Owner->ClearUpdating();
		}
	}
	return err;
}



VError TableRegular::LoadFromBagWithLoader( const VValueBag& inBag, VBagLoader *inLoader, void* inContext)
{
	VError err = VE_OK;

	// get UUID
	VUUID uuid;
	inLoader->GetUUID( inBag, uuid);
	uuid.ToBuffer(FID.ID);

	VString s;
	inBag.GetString( DB4DBagKeys::name, s);
	if (!inLoader->WithNamesCheck() || (s.GetLength() <= kMaxTableNameLength && IsValid4DTableName( s)))
	{
		//s.ToBlock( &FID.nom, sizeof( FID.nom), VTC_UTF_16, false, true);
		fName = s;
	}
	else
		err = ThrowError( VE_DB4D_INVALIDTABLENAME, DBaction_CreatingTable);

	if (err == VE_OK)
	{
		FID.fNotFullyDeleteRecords = DB4DBagKeys::leave_tag_on_delete.Get( &inBag);
	}
	
	if (err == VE_OK)
	{
		FID.fKeepStamps = DB4DBagKeys::keep_record_stamps.Get( &inBag) ? 1 : 0;
	}

	if (err == VE_OK)
	{
		FID.fKeepSyncInfo = DB4DBagKeys::keep_record_sync_info.Get( &inBag) ? 1 : 0;
	}

	if (err == VE_OK)
	{
		FID.fSchemaID = DB4DBagKeys::sql_schema_id.Get( &inBag);
		CDB4DSchema *schema = Owner->RetainSchema( FID.fSchemaID);
		if (schema == NULL)
			FID.fSchemaID = 1;
		ReleaseRefCountable( &schema);
	}

	if (err == VE_OK)
	{
		err = GetExtraPropertiesFromBag( DB4DBagKeys::table_extra, this, inBag, false, (CDB4DBaseContext*)inContext, inLoader->IsLoadOnly());
	}

	fName.ToBlock( &FID.nom, sizeof( FID.nom), VTC_UTF_16, false, true);
	fRecordName.ToBlock( &FID.RecordName, sizeof( FID.RecordName), VTC_UTF_16, false, true);

	return err;
}


VError TableRegular::LoadPrimKeyFromBagWithLoader( const VValueBag& inBag, VBagLoader *inLoader, void* inContext)
{
	return ExtractPrimKeyFromBagWithLoader( PrimaryKey, inBag, inLoader, inContext);
}


VError TableRegular::ExtractPrimKeyFromBagWithLoader( NumFieldArray& outPrimaryKey, const VValueBag& inBag, VBagLoader *inLoader, void* inContext) const
{
	VError err = VE_OK;

	outPrimaryKey.SetCount(0);

	{
		const VBagArray* primkeys = inBag.GetElements(DB4DBagKeys::primary_key);
		if (primkeys != nil)
		{
			for (VIndex i = 1, nb = primkeys->GetCount(); i <= nb; i++)
			{
				const VValueBag* primbag = primkeys->GetNth(i);
				if (primbag != nil)
				{
					VString fieldname;
					VUUID fieldid;
					sLONG fieldnum = 0;
					if (primbag->GetVUUID(DB4DBagKeys::field_uuid, fieldid))
					{
						fieldnum = FindField(fieldid);
					}
					if (fieldnum == 0)
					{
						if (primbag->GetString(DB4DBagKeys::field_name, fieldname))
						{
							fieldnum = FindField(fieldname);
						}
					}
					if (fieldnum != 0)
						outPrimaryKey.Add(fieldnum);
					else
						err = ThrowError(VE_DB4D_WRONGFIELDREF, noaction);
				}
			}
		}
	}

	sLONG primkeyfield = 0;
	if (inBag.GetLong(L"primkey", primkeyfield))
	{
		outPrimaryKey.Add(primkeyfield);
	}

	return err;
}


VError TableRegular::SaveToBag( VValueBag& ioBag, VString& outKind) const
{
	VError err = VE_OK;

	occupe();

	// set properties
	VString name;
	GetName( name);
	DB4DBagKeys::name.Set( &ioBag, name);

	VUUID uuid;
	GetUUID( uuid);
	DB4DBagKeys::uuid.Set( &ioBag, uuid);

	DB4DBagKeys::id.Set( &ioBag, GetNum());

	DB4DBagKeys::leave_tag_on_delete.Set( &ioBag, FID.fNotFullyDeleteRecords);
	DB4DBagKeys::keep_record_stamps.Set( &ioBag, FID.fKeepStamps == 1);
	DB4DBagKeys::keep_record_sync_info.Set( &ioBag, FID.fKeepSyncInfo == 1);

	DB4DBagKeys::sql_schema_id.Set( &ioBag, FID.fSchemaID);

	// insert fields definition
	sLONG count = GetNbCrit();
	for( sLONG i = 1 ; (i <= count) && (err == VE_OK) ; ++i)
	{
		const Field *thefield = RetainField( i);
		if (thefield != NULL)
		{
			err = ioBag.AddElement( thefield);
			thefield->Release();
		}
	}

	FieldArray primkey;
	err = RetainPrimaryKey(primkey);
	if (primkey.GetCount() != 0)
	{
		for (FieldArray::Iterator cur = primkey.First(), end = primkey.End(); cur != end; cur++)
		{
			Field* crit = *cur;
			VString fieldname;
			VUUID fieldid;
			crit->GetName(fieldname);
			crit->GetUUID(fieldid);
			BagElement primbag(ioBag, DB4DBagKeys::primary_key);
			primbag->SetString(DB4DBagKeys::field_name, fieldname);
			primbag->SetVUUID(DB4DBagKeys::field_uuid, fieldid);

			crit->Release();
		}
	}

	// insert extra properties.
	err = PutExtraPropertiesInBag( DB4DBagKeys::table_extra, const_cast<TableRegular*>( this), ioBag, nil);

	libere();

	outKind = L"table";

	return err;
}


VValueBag *TableRegular::CreateReferenceBag( bool inForExport) const
{
	VValueBag *bag = new VValueBag;
	if (bag != NULL)
	{
		occupe();

		VUUID uuid;
		GetUUID( uuid);
		DB4DBagKeys::uuid.Set( bag, uuid);

		if (inForExport)
		{
			VString name;
			GetName( name);
			DB4DBagKeys::name.Set( bag, name);
		}

		libere();
	}
	return bag;
}



Boolean TableRegular::DeleteField(Field* inFieldToDelete, CDB4DBaseContext* inContext, VProgressIndicator* progress, Boolean inOnlyLocal)
{
	Boolean ok = false;
	sLONG n;
	VError err = VE_OK;


	if (fIsRemote && !inOnlyLocal)
	{
		if (inFieldToDelete->GetOwner() == this)
		{
			IRequest *req = GetOwner()->CreateRequest( inContext, Req_DropField + kRangeReqDB4D);
			if (req == NULL)
			{
				err = ThrowBaseError(memfull, DBaction_DeletingField);
			}
			else
			{
				req->PutBaseParam( GetOwner());
				req->PutTableParam( this);
				req->PutFieldParam( inFieldToDelete);
				req->PutProgressParam(progress);
			
				err = req->GetLastError();
				if (err == VE_OK)
				{
					err = req->Send();
					if (err == VE_OK)
					{
						sLONG numTable = req->GetLongReply( err);
						if (err==VE_OK && numTable!=0 && numTable==numfile)
						{
							err = Update( req->GetInputStream());
							ok = true;
						}
					}
				}
				req->Release();
			}
		}
	}
	else
	{
		if (fIsRemote || Owner->OkToUpdate(err))
		{
			if (inFieldToDelete->GetOwner() == this)
			{
				occupe();
				n = inFieldToDelete->GetPosInRec();
				bool isPartOfPrimKey = false;
				for (NumFieldArray::Iterator cur = PrimaryKey.First(), end = PrimaryKey.End(); cur != end; cur++)
				{
					if (*cur == n)
						isPartOfPrimKey = true;
				}
				if (isPartOfPrimKey)
				{
					err = ThrowError(VE_DB4D_CANNOT_CHANGE_PRIMARYKEY_DEFINITION, noaction);
				}
				else
				{
					if (tc[n] == inFieldToDelete)
					{
						tc[n] = nil;
						VUUID xid;
						inFieldToDelete->GetUUID(xid);
						Owner->DelObjectID(objInBase_Field, inFieldToDelete, xid);
						Owner->DelFieldRec(GetNum(), n);
						//inFieldToDelete->DropAllRelations();
						inFieldToDelete->GetOwner()->GetOwner()->DropAllRelationOnOneField(inFieldToDelete, inContext, inOnlyLocal);	// sc 25/06/2010 ACI0066147, pass inOnlyLocal parameter
						if (!fIsRemote)
							inFieldToDelete->DropAllRelatedIndex(inContext, progress);	// sc 25/06/2010 ACI0066147, only for local database
						inFieldToDelete->UnRegisterForLang();
						inFieldToDelete->SetExtraProperties(nil, false, inContext);
						inFieldToDelete->Release();
						ok = true;
					}

					if (!fIsRemote)
					{
						ReCalc();
						VError err = save();
					}
				}
				libere();
			}
			if (!fIsRemote)
				Owner->ClearUpdating();
		}
	}
	return ok;
}


sLONG TableRegular::AddField(const VString &name, sLONG fieldtyp, sLONG fieldlen, DB4DFieldAttributes inAttributes, VError& err, 
							 CDB4DBaseContext* inContext, VProgressIndicator* progress)
{
	CritereDISK cd;
	sLONG nc;
	VUUID x;

	if (fIsRemote)
	{
	}
	else
	{
		if (Owner->OkToUpdate(err))
		{
			occupe();

			nc = FindNextFieldFree();

			name.ToBlock( cd.nom, sizeof( cd.nom), VTC_UTF_16, false, true);
			cd.typ=fieldtyp;
			/*
			if (fieldlen<0)
				fieldlen = 0;
				*/
			cd.LimitingLength=fieldlen;
			cd.not_null = (inAttributes & DB4D_Not_Null) == 0 ? 0 : 1;
			cd.unique = (inAttributes & DB4D_Unique) == 0 ? 0 : 1;
			cd.fStamp = 1; // sc 09/11/2006, ACI0046812, was 0
			cd.fAutoSeq = (inAttributes & DB4D_AutoSeq) == 0 ? 0 : 1;
			cd.fAutoGenerate = (inAttributes & DB4D_AutoGenerate) == 0 ? 0 : 1;
			cd.fStoreAsUTF8 = (inAttributes & DB4D_StoreAsUTF8) == 0 ? 0 : 1;
			
			if ((inAttributes & DB4D_StoreAsUUID) && (cd.typ == VK_STRING || cd.typ == VK_TEXT))
				cd.typ = VK_UUID;
			if ((inAttributes & DB4D_StoreAsUTF8))
			{
				if (fieldtyp == VK_STRING)
					cd.typ = VK_STRING_UTF8;
				else if (fieldtyp == VK_TEXT)
					cd.typ = VK_TEXT_UTF8;
			}

			cd.ExtraAddr = 0;
			cd.ExtraLen = 0;

			cd.UnusedLong1 = 0;
			cd.ReUsedNumber = 0;
			cd.UnusedAddr2 = 0;
			cd.fNeverNull = 0;
			cd.fDefaultValue = 0;
			cd.fOuterData = 0;
			cd.fTextIsWithStyle = 0;
			cd.fUnusedChar3 = 0;

			x.Regenerate();
			x.ToBuffer(cd.ID);

			if (nc == -1)
			{
				err = change(&cd, 1, GetNbCrit(), true, inContext, true, progress);
				nc = GetNbCrit();
			}
			else
				err = change(&cd, 1, nc-1, true, inContext, true, progress);

			ReCalc();

			if (err != VE_OK)
				nc = -1;

			libere();
			Owner->ClearUpdating();
		}

		if (err != VE_OK)
			err = ThrowError(VE_DB4D_CANNOTADDFIELD, DBaction_AddingField);
	}

	return(nc);
}


sLONG TableRegular::AddField(Field* cri, VError& err, CDB4DBaseContext* inContext, VProgressIndicator* progress)
{
	sLONG nc;
	VUUID x;

	if (fIsRemote)
	{
	}
	else
	{
		if (Owner->OkToUpdate(err))
		{
			occupe();
			x.Regenerate();
			cri->SetUUID(x);

			nc = FindNextFieldFree();

			if (nc == -1)
			{
				err = change(cri->getCRD(), 1, GetNbCrit(), true, inContext, true, progress);
				nc = GetNbCrit();
			}
			else
				err = change(cri->getCRD(), 1, nc-1, true, inContext, true, progress);
			ReCalc();

			if (err != VE_OK)
				nc = -1;

			libere();
			Owner->ClearUpdating();
		}

		if (err != VE_OK)
			err = ThrowError(VE_DB4D_CANNOTADDFIELD, DBaction_AddingField);
	}

	return(nc);
}


VError TableRegular::AddFieldSilently( Field* cri, sLONG pos)
{
	if (pos > tc.GetCount())
	{
		tc.SetCount(pos, nil);
	}
	tc[pos] = cri;
	FID.nbcrit = tc.GetCount();
	cri->Retain();
	return VE_OK;
}



sLONG TableRegular::AddFields(CritereDISK* cd, sLONG nbFieldsToAdd, VError& err, CDB4DBaseContext* inContext, VProgressIndicator* progress)
{
	sLONG nc,i;
	CritereDISK *p = cd;
	VUUID x;

	if (fIsRemote)
	{
	}
	else
	{
		if (!fTableCanBesaved || Owner->OkToUpdate(err))
		{
			for (i=0; i<nbFieldsToAdd; i++)
			{
				x.Regenerate();
				x.ToBuffer(p->ID);
				p++;
			}


			occupe();

			nc = FindNextFieldFree();
			if (nc == -1 || nbFieldsToAdd > 1)
			{
				nc = GetNbCrit() + 1;
				err = change(cd, nbFieldsToAdd, GetNbCrit(), true, inContext, true, progress);
			}
			else
			{
				err = change(cd,1,nc-1, true, inContext, true, progress);
			}
			ReCalc();

			if (err != VE_OK)
				nc = -1;

			libere();
			if (fTableCanBesaved)
				Owner->ClearUpdating();
		}

		if (err != VE_OK)
			err = ThrowError(VE_DB4D_CANNOTADDFIELD, DBaction_AddingField);
	}

	return(nc);
}




VError TableRegular::LoadFields( const VBagArray *inFieldsBags, VBagLoader *inLoader, CDB4DBaseContext* inContext)
{
	VError err = VE_OK;

	sLONG count = inFieldsBags->GetCount();

	for( sLONG i = 1 ; (i <= count) && (err == VE_OK) ; ++i)
	{
		const VValueBag *fieldBag = inFieldsBags->GetNth( i);
		Field *field = new Field( this);
		if (field != NULL)
		{
			sLONG num = 0;
			if (!fieldBag->GetLong(DB4DBagKeys::id, num))
			{
				num = tc.GetCount()+1;
			}
			if (num > tc.GetCount())
				tc.SetCount(num, nil);
			err = field->LoadFromBagWithLoader( *fieldBag, inLoader, inContext);
			if (err == VE_OK)
			{
				field->SetPosInRec(num);
				tc[num] = field;
				VUUID xid;
				field->GetUUID(xid);
				Owner->AddObjectID(objInBase_Field, field, xid);
				FID.nbcrit = tc.GetCount();
			}
			else
				field->Release();
		}
		else
		{
			err = ThrowError(VE_DB4D_CANNOTADDFIELD, DBaction_AddingField);
		}
	}

	return err;
}



VError TableRegular::CreateFields( const VBagArray *inFieldsBags, sLONG *outFirstAddedFieldNo, VBagLoader *inLoader, CDB4DBaseContext* inContext, VProgressIndicator* progress)
{
	/*
	Ou bien tous les champs on pu etre ajoute ou bien aucun.
	*/

	VError err = VE_OK;
	sLONG firstAddedFieldNo = -1;

	if (fIsRemote)
	{
		IRequest *req = GetOwner()->CreateRequest( inContext, Req_AddFields_With_BagArray + kRangeReqDB4D);
		if (req == NULL)
		{
			err = ThrowBaseError(memfull, DBaction_AddingField);
		}
		else
		{
			req->PutBaseParam( GetOwner());
			req->PutTableParam( this);
			req->PutValueBagArrayParam( *inFieldsBags);
			req->PutProgressParam(progress);
		
			err = req->GetLastError();
			if (err == VE_OK)
			{
				err = req->Send();
				if (err == VE_OK)
				{
					firstAddedFieldNo = req->GetLongReply( err);
					sLONG numTable = req->GetLongReply( err);
					if (err==VE_OK && numTable==numfile)
						err = Update( req->GetInputStream());
				}
			}
			req->Release();
		}
	}
	else
	{
		if (Owner->OkToUpdate(err))
		{
			sLONG count = inFieldsBags->GetCount();

			std::vector<Field*> fields( count);

			for( sLONG i = 1 ; (i <= count) && (err == VE_OK) ; ++i)
			{
				const VValueBag *fieldBag = inFieldsBags->GetNth( i);
				Field *field = new Field( this);
				if (field != NULL)
				{
					err = field->LoadFromBagWithLoader( *fieldBag, inLoader, inContext);
					if (err == VE_OK)
					{
						fields[i-1] = field;
					}
					else
					{
						field->Release();
					}
				}
				else
				{
					err = ThrowError(VE_DB4D_CANNOTADDFIELD, DBaction_AddingField);
				}
			}

			//		if (err == VE_OK)
			if ((err == VE_OK)&&((sLONG)fields.size() > 0))
				err = AddFields( &fields[0], (sLONG)fields.size(), &firstAddedFieldNo, inLoader->WithNamesCheck(), inContext, progress);

			for( std::vector<Field*>::iterator f = fields.begin() ; f != fields.end() ; ++f)
			{
				if (*f != NULL)
					(*f)->Release();
			}

			if (err == VE_OK)
			{
				err = save();
			}
			Owner->ClearUpdating();
		}
	}

	if (outFirstAddedFieldNo != nil)
		*outFirstAddedFieldNo = firstAddedFieldNo;

	return err;
}


VError TableRegular::AddFields( Field **inFirst, sLONG inCount, sLONG *outFirstAddedFieldNo, Boolean inWithNamesCheck, CDB4DBaseContext* inContext, VProgressIndicator* progress)
{
	/*
	Ou bien tous les champs on pu etre ajoute ou bien aucun.
	*/

	VError err = VE_OK;
	
	sLONG firstAddedFieldNo = -1;

	if (fIsRemote)
	{
	}
	else
	{
		Field **afterLast = inFirst + inCount;

		if (!fTableCanBesaved || Owner->OkToUpdate(err))
		{
			occupe();

			// check for name duplicates

			if (inWithNamesCheck)
			{
				for( Field **oneField = inFirst ; (oneField != afterLast) && (err == VE_OK) ; ++oneField)
				{
					VStr31 name;
					(*oneField)->GetName( name);
					Field *other = FindAndRetainFieldRef( name);
					if (other != NULL)
					{
						err = ThrowError( VE_DB4D_FIELDNAMEDUPLICATE, DBaction_AddingField);
						other->Release();
					}
				}
			}

			// take Fields
			if (err == VE_OK)
			{
				sLONG oldCount = this->tc.GetCount();
				sLONG toAdd = afterLast - inFirst;
				sLONG nc = (toAdd == 1) ? FindNextFieldFree() : -1;
				if ( (toAdd == 1) && (nc != -1) )
				{
					firstAddedFieldNo = nc;
					this->tc[nc] = *inFirst;
					(*inFirst)->Retain();
					(*inFirst)->SetPosInRec( nc);
				}
				else
				{
					if (!this->tc.AddNSpaces( toAdd, false)) 
					{
						err = ThrowError( memfull, DBaction_AddingField);
					}
					else
					{
						firstAddedFieldNo = this->FID.nbcrit + 1;
						for( Field **oneField = inFirst ; (oneField != afterLast) && (err == VE_OK) ; ++oneField)
						{
							this->tc[++this->FID.nbcrit] = *oneField;
							(*oneField)->Retain();
							(*oneField)->SetPosInRec( this->FID.nbcrit);
						}
					}
				}
			} 

			ReCalc();

			libere();

			// now register for language
			if (err == VE_OK)
			{
				occupe(); // utile ?
				for( Field **oneField = inFirst ; (oneField != afterLast) && (err == VE_OK) ; ++oneField)
				{
					VUUID xid;
					(*oneField)->GetUUID(xid);
					Owner->AddObjectID(objInBase_Field, *oneField, xid);
					Owner->AddFieldRec(GetNum(), (*oneField)->GetPosInRec());
					(*oneField)->RegisterForLang();
				}
				libere();
			}
			if (fTableCanBesaved)
				Owner->ClearUpdating();
		}
	}

	if (outFirstAddedFieldNo != nil)
		*outFirstAddedFieldNo = firstAddedFieldNo;
	
	return err;
}



VError TableRegular::Drop(CDB4DBaseContext* inContext, VProgressIndicator* progress)
{
	VError err = VE_OK;

	{
		occupe();

		if (!fIsRemote)
		{
			err = DropAllRelatedIndex(inContext, progress);
			GetOwner()->DropAllRelationOnOneTable(this, inContext); // ACI0045896
		}


		if (err == VE_OK)
		{
			SetNotifyState(true);
			Owner->DelFieldRec(GetNum(), 0);
			sLONG nb = tc.GetCount();
			for (sLONG i = 1; i <= nb; i++)
			{
				Field* cri = tc[i];
				if (cri != nil)
				{
					tc[i] = nil;
					VUUID xid;
					cri->GetUUID(xid);
					Owner->DelObjectID(objInBase_Field, cri, xid);
					cri->UnRegisterForLang();
					cri->Release();
				}
			}
			SetNotifyState(false);

			if (!fIsRemote)
				err = Owner->DeleteTableDef(nres-1);
		}

		libere();
	}
	return err;
}


VError TableRegular::Truncate(BaseTaskInfo* context, VProgressIndicator* progress, Boolean ForADeleteSelection, bool& mustunlock)
{
	VError err = VE_OK;
	if (testAssert(!fIsRemote))
	{
#if trackTruncate
		trackDebugMsg("before truncate\n");
#endif
		bool ok;
		if (ForADeleteSelection)
			//ok = fBuildIndexMutex.TryToLock(RWS_ReadOnly);
			ok = DF->TryToLockWrite();
		else
		{
			//fBuildIndexMutex.Lock(RWS_ReadOnly);
			DF->LockWrite();
			ok = true;
		}
		if (ok)
		{
#if trackTruncate
			trackDebugMsg("before truncate, after LockWrite\n");
#endif
			Base4D* db = GetOwner();
			
			IndexArray indexdeps;
			/*
			CopyIndexDep(indexdeps);
			*/
			err = DF->Truncate(context, progress, ForADeleteSelection, indexdeps);
			if (err == VE_OK)
			{
				VStream* log;
				sLONG len = 4 + sizeof(VUUIDBuffer);

				err = db->StartWriteLog(DB4D_Log_TruncateTable, len, context, log);
				if (err == VE_OK)
				{
					if (log != nil)
					{
						err = log->PutLong(-1);
						if (err == VE_OK)
						{
							VUUID xid;
							GetUUID(xid);
							//err = log->PutLong(DF->GetTrueNum());
							err = xid.WriteToStream(log);
						}
					}
					VError err2 = db->EndWriteLog(len);
					if (err == VE_OK)
						err = err2;
				}
			}
			//fBuildIndexMutex.Unlock();
			DF->CheckForMemRequest();
			DF->Unlock();

			if (ForADeleteSelection)
			{
				DF->Unlock(); // 2eme unlock car l'appelant a locke DF en read avant d'appeler truncate
				mustunlock = false;
			}

			if (err != -2)
			{
				if (db->OkToUpdate(err))
				{
					AwakeIndexes(nil, nil);
					/*
					occupe();
					RebuildAllRelatedIndex(context->GetEncapsuleur(), progress);
					libere();
					*/
					db->ClearUpdating();
				}
			}

		}
		else
			err = -2;
#if trackTruncate
		trackDebugMsg("after truncate\n");
#endif
	}
	else
		err = ThrowBaseError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE);
	return err;
}


VError TableRegular::SetKeepStamp( CDB4DBaseContext* inContext, bool inKeepStamp, VDB4DProgressIndicator* inProgress)
{
	VError err = VE_OK;

	if (fIsRemote)
	{
		// sc 24/04/2009 ACI0061742
		IRequest *req = GetOwner()->CreateRequest( inContext, Req_SetTableKeepStamp + kRangeReqDB4D);
		if (req == NULL)
		{
			err = ThrowBaseError( memfull, DBaction_ChangingTableProperties);
		}
		else
		{
			req->PutBaseParam( GetOwner());
			req->PutTableParam( this);
			req->PutBooleanParam( inKeepStamp);
			req->PutProgressParam( inProgress);
		
			err = req->GetLastError();
			if (err == VE_OK)
			{
				err = req->Send();
				if (err == VE_OK)
				{
					occupe();
					FID.fKeepStamps = inKeepStamp ? 1 : 0;
					libere();
				}
			}
			req->Release();
		}
	}
	else
	{
		FID.fKeepStamps = inKeepStamp ? 1 : 0;
		if (DF != nil)
		{
			err = DF->SetKeepStamp(FID.fKeepStamps, inProgress);
		}
		Touch();
		err = save();
	}
	return err;
}


bool TableRegular::GetKeepRecordSyncInfo() const
{
	return FID.fKeepSyncInfo == 1;
}

VError TableRegular::SetKeepRecordSyncInfo( CDB4DBaseContext* inContext, bool inKeepSyncInfo, VDB4DProgressIndicator* inProgress)
{
	VError err = VE_OK;

	if (HasPrimKey() || !inKeepSyncInfo)
	{
		if (fIsRemote)
		{
			// sc 24/04/2009 ACI0061742
			IRequest *req = GetOwner()->CreateRequest( inContext, Req_SetTableKeepRecordSyncInfo + kRangeReqDB4D);
			if (req == NULL)
			{
				err = ThrowBaseError( memfull, DBaction_ChangingTableProperties);
			}
			else
			{
				req->PutBaseParam( GetOwner());
				req->PutTableParam( this);
				req->PutBooleanParam( inKeepSyncInfo);
				req->PutProgressParam( inProgress);
			
				err = req->GetLastError();
				if (err == VE_OK)
				{
					err = req->Send();
					if (err == VE_OK)
					{
						occupe();
						FID.fKeepSyncInfo = inKeepSyncInfo ? 1 : 0;
						libere();
					}
				}
				req->Release();
			}
		}
		else
		{
			if (!inKeepSyncInfo && HasSyncInfo())
			{
				SynchroBaseHelper* sync = Owner->GetSyncHelper(true);
				if (sync != nil)
				{
					sync->Lock();
					Table* tsync = sync->GetTableSync(this, err, false);
					if (tsync != nil)
					{
						Base4D* bsync = sync->GetBase(err, false);
						sync->RemoveTable(this);
						bsync->DeleteTable(tsync, nil);
					}
					sync->Unlock();
				}

			}
			FID.fKeepSyncInfo = inKeepSyncInfo ? 1 : 0;
			Touch();
			err = save();
		}
	}
	else
	{
		err = ThrowError( VE_DB4D_PRIMKEY_IS_NEEDED, DBaction_ChangingTableProperties);
	}

	return err;
}


bool TableRegular::HasSyncInfo() const
{
	return HasPrimKey() && GetKeepRecordSyncInfo();
}


void TableRegular::MarkRecordAsPushed(sLONG numrec)
{
	if (fIsRemote)
	{
		VDBMgr::GetManager()->AddMarkRecordAsPushed(GetNum(), numrec);
	}
	else
	{
		if (DF != nil)
			DF->MarkRecordAsPushed(numrec);
	}
}

void TableRegular::UnMarkRecordAsPushed(sLONG numrec)
{
	if (fIsRemote)
	{
		VDBMgr::GetManager()->AddUnMarkRecordAsPushed(GetNum(), numrec);
	}
	else
	{
		if (DF != nil)
			DF->UnMarkRecordAsPushed(numrec);
	}
}





VError TableRegular::ImportRecords(VFolder* folder, BaseTaskInfo* context, VDB4DProgressIndicator* inProgress, ExportOption& options)
{
	VError err = VE_OK;
	options.Import = true;

	ExportJob job(this, folder, inProgress, 0, options);

	err = job.StartJob(0);
	if (err == VE_OK)
	{
		if (options.DelayIndexes)
			DelayIndexes();
		if (options.ChangeIntegrityRules)
			Owner->StopDataConversion();
		sLONG nbficToImport = job.GetNbFicToImport();
		if (inProgress != nil)
		{
			XBOX::VString session_title;
			VValueBag bag;
			VStr<64> tname;
			GetName(tname);
			bag.SetString("TableName", tname);
			bag.SetString("curValue", L"{curValue}");
			bag.SetString("maxValue", L"{maxValue}");
			gs(1005,31,session_title);
			session_title.Format(&bag);
			inProgress->BeginSession(nbficToImport, session_title, true);
		}

		bool cont = true;
		sLONG8 curfic = 0;
		do 
		{
			if (inProgress != nil)
			{
				if (!inProgress->Progress(curfic))
				{
					err = ThrowError(VE_DB4D_ACTIONCANCELEDBYUSER, noaction);
				}
			}

			if (err == VE_OK)
			{
				FicheInMem* fic = new FicheInMem(context, Owner, this, err);
				cont = job.ReadOneRecord(fic, context, err);
				if (cont)
				{
					err = DF->SaveRecord(fic, context);
				}
				fic->Release();
			}
			++curfic;
		} while (cont && err == VE_OK); 

		if (inProgress != nil)
		{
			inProgress->EndSession();
		}

		if (options.ChangeIntegrityRules)
			Owner->StopDataConversion();
		if (options.DelayIndexes)
		{
			//vector<IndexInfo*> indexes;
			AwakeIndexes(nil, nil);
			/*
			for (vector<IndexInfo*>::iterator cur = indexes.begin(), end = indexes.end(); cur != end; cur++)
				(*cur)->Release();
				*/
		}
	}
	job.StopJob();

	return err;
}






									/* ----------------------------------------------- */













TableSystem::TableSystem(Base4D *owner, sLONG num, const VString& name, const VUUID& id, const VString& recordname):Table(owner, num, false)
{
	fName = name;
	fID = id;
	fRecordName = recordname;
	owner->AddObjectID(objInBase_Table, this, id);
}


TableSystem::~TableSystem()
{
	Owner->DelObjectID(objInBase_Table, this, fID);
}


VError TableSystem::SaveToBag( VValueBag& ioBag, VString& outKind) const
{
	VError err = VE_OK;

	occupe();

	// set properties
	VString name;
	GetName( name);
	ioBag.SetString( L"name", name);

	VUUID uuid;
	GetUUID( uuid);
	ioBag.SetVUUID( L"uuid", uuid);

	// insert fields definition
	sLONG count = GetNbCrit();
	for( sLONG i = 1 ; (i <= count) && (err == VE_OK) ; ++i)
	{
		const Field *thefield = RetainField( i);
		if (thefield != NULL)
		{
			err = ioBag.AddElement( thefield);
			thefield->Release();
		}
	}

	libere();

	outKind = L"table";

	return err;
}


VValueBag *TableSystem::CreateReferenceBag( bool inForExport) const
{
	VValueBag *bag = new VValueBag;
	if (bag != NULL)
	{
		occupe();

		VUUID uuid;
		GetUUID( uuid);
		bag->SetVUUID( L"uuid", uuid);

		if (inForExport)
		{
			VString name;
			GetName( name);
			bag->SetString( L"name", name);
		}

		libere();
	}
	return bag;
}


void TableSystem::System_AddField(Field* cri)
{
	tc.Add(cri);
	cri->Retain();
}


sLONG TableSystem::System_AddField(sLONG typ, const VString& name)
{
	sLONG num = tc.GetCount() + 1;
	Field* cri = new Field(typ, num, this);
	cri->SetName(name, nil);
	tc.Add(cri);
	return num;
}

								/* ----------------------------------------------- */


TableOfTable::TableOfTable(Base4D* owner):TableSystem(owner, -1, L"_USER_TABLES", VUUID((VUUIDBuffer&)"Sys_DB4D_000001"), L"_USER_TABLE")
{
	id_Table_Name = System_AddField(DB4D_StrFix, L"TABLE_NAME");
	id_Temporary = System_AddField(DB4D_Boolean, L"TEMPORARY");
	id_Table_ID = System_AddField(DB4D_Integer64, L"TABLE_ID");
	id_Schema_ID = System_AddField(DB4D_Integer32, L"SCHEMA_ID");
	id_Replication = System_AddField(DB4D_Boolean, L"REPLICATION");
}


								/* ----------------------------------------------- */


TableOfField::TableOfField(Base4D* owner):TableSystem(owner, -2, L"_USER_COLUMNS", VUUID((VUUIDBuffer&)"Sys_DB4D_000002"), L"_USER_COLUMN")
{
	id_Table_Name = System_AddField(DB4D_StrFix, L"TABLE_NAME");
	id_Column_Name = System_AddField(DB4D_StrFix, L"COLUMN_NAME");
	id_Data_Type = System_AddField(DB4D_Integer32, L"DATA_TYPE");
	id_Data_Length = System_AddField(DB4D_Integer32, L"DATA_LENGTH");
	id_Nullable = System_AddField(DB4D_Boolean, L"NULLABLE");
	id_Table_ID = System_AddField(DB4D_Integer64, L"TABLE_ID");
	id_Column_ID = System_AddField(DB4D_Integer64, L"COLUMN_ID");
	id_Old_Data_Type = System_AddField(DB4D_Integer32, L"OLD_DATA_TYPE");
}


								/* ----------------------------------------------- */


TableOfIndexes::TableOfIndexes(Base4D* owner):TableSystem(owner, -3, L"_USER_INDEXES", VUUID((VUUIDBuffer&)"Sys_DB4D_000003"), L"_USER_INDEXES")
{
	id_Index_UUID = System_AddField(DB4D_StrFix, L"INDEX_UUID");
	id_Index_Name = System_AddField(DB4D_StrFix, L"INDEX_NAME");
	id_Index_Type = System_AddField(DB4D_Integer32, L"INDEX_TYPE");
	id_Table_Name = System_AddField(DB4D_StrFix, L"TABLE_NAME");
	id_Uniq = System_AddField(DB4D_Boolean, L"UNIQUENESS");
	id_Table_ID = System_AddField(DB4D_Integer64, L"TABLE_ID");
	id_Index_ID = System_AddField(DB4D_Integer64, L"INDEX_ID");
}


								/* ----------------------------------------------- */


TableOfIndexCols::TableOfIndexCols(Base4D* owner):TableSystem(owner, -4, L"_USER_IND_COLUMNS", VUUID((VUUIDBuffer&)"Sys_DB4D_000004"), L"_USER_IND_COLUMNS")
{
	id_Index_UUID = System_AddField(DB4D_StrFix, L"INDEX_UUID");
	id_Index_Name = System_AddField(DB4D_StrFix, L"INDEX_NAME");
	id_Table_Name = System_AddField(DB4D_StrFix, L"TABLE_NAME");
	id_Column_Name = System_AddField(DB4D_StrFix, L"COLUMN_NAME");
	id_Column_Position = System_AddField(DB4D_Integer32, L"COLUMN_POSITION");
	id_Table_ID = System_AddField(DB4D_Integer64, L"TABLE_ID");
	id_Column_ID = System_AddField(DB4D_Integer64, L"COLUMN_ID");
	id_Index_ID = System_AddField(DB4D_Integer64, L"INDEX_ID");
}


								/* ----------------------------------------------- */


TableOfConstraints::TableOfConstraints(Base4D* owner):TableSystem(owner, -5, L"_USER_CONSTRAINTS", VUUID((VUUIDBuffer&)"Sys_DB4D_000005"), L"_USER_CONSTRAINTS")
{
	id_Constraint_ID = System_AddField(DB4D_StrFix, L"CONSTRAINT_ID");
	id_Constraint_Name = System_AddField(DB4D_StrFix, L"CONSTRAINT_NAME");
	id_Constraint_Type = System_AddField(DB4D_StrFix, L"CONSTRAINT_TYPE");
	id_Table_Name = System_AddField(DB4D_StrFix, L"TABLE_NAME");
	id_Table_ID = System_AddField(DB4D_Integer64, L"TABLE_ID");
	id_Delete_Rule = System_AddField(DB4D_StrFix, L"DELETE_RULE");
	id_Related_Table = System_AddField(DB4D_StrFix, L"RELATED_TABLE_NAME");
	id_Related_Table_ID = System_AddField(DB4D_Integer64, L"RELATED_TABLE_ID");
}


								/* ----------------------------------------------- */


TableOfConstraintCols::TableOfConstraintCols(Base4D* owner):TableSystem(owner, -6, L"_USER_CONS_COLUMNS", VUUID((VUUIDBuffer&)"Sys_DB4D_000006"), L"_USER_CONS_COLUMNS")
{
	id_Constraint_ID = System_AddField(DB4D_StrFix, L"CONSTRAINT_ID");
	id_Constraint_Name = System_AddField(DB4D_StrFix, L"CONSTRAINT_NAME");
	id_Table_Name = System_AddField(DB4D_StrFix, L"TABLE_NAME");
	id_Table_ID = System_AddField(DB4D_Integer64, L"TABLE_ID");
	id_Column_Name  = System_AddField(DB4D_StrFix, L"COLUMN_NAME");
	id_Column_ID  = System_AddField(DB4D_Integer64, L"COLUMN_ID");
	id_Position  = System_AddField(DB4D_Integer32, L"COLUMN_POSITION");
	id_Related_Column_Name = System_AddField(DB4D_StrFix, L"RELATED_COLUMN_NAME");
	id_Related_Column_ID = System_AddField(DB4D_Integer32, L"RELATED_COLUMN_ID");
}


								/* ----------------------------------------------- */


TableOfSchemas::TableOfSchemas(Base4D* owner):TableSystem(owner, -7, L"_USER_SCHEMAS", VUUID((VUUIDBuffer&)"Sys_DB4D_000007"), L"_USER_SCHEMAS")
{
	id_Schema_ID = System_AddField(DB4D_Integer32, L"SCHEMA_ID");
	id_Schema_Name = System_AddField(DB4D_StrFix, L"SCHEMA_NAME");
	id_Read_Group_ID = System_AddField(DB4D_Integer32, L"READ_GROUP_ID");
	id_Read_Group_Name = System_AddField(DB4D_StrFix, L"READ_GROUP_NAME");
	id_Read_Write_Group_ID = System_AddField(DB4D_Integer32, L"READ_WRITE_GROUP_ID");
	id_Read_Write_Group_Name = System_AddField(DB4D_StrFix, L"READ_WRITE_GROUP_NAME");
	id_All_Group_ID = System_AddField(DB4D_Integer32, L"ALL_GROUP_ID");
	id_All_Group_Name = System_AddField(DB4D_StrFix, L"ALL_GROUP_NAME");
}


								/* ----------------------------------------------- */


#if 0

TreeInMem *FullyDeletedRecordInfoTreeInMem::CreTreeInMem()
{
	return new FullyDeletedRecordInfoTreeInMem(FeuilleFinaleContientQuoi);
}



TreeInMem *FullyDeletedRecordInfoTreeInMemHeader::CreTreeInMem()
{
	return new FullyDeletedRecordInfoTreeInMem(FeuilleFinaleContientQuoi);
}

#endif

								/* ----------------------------------------------- */


#if 0
TreeInMem *BlobTreeInMem::CreTreeInMem()
{
	return new BlobTreeInMem(RecordAccess, FeuilleFinaleContientQuoi);
}


void BlobTreeInMem::DeleteElem( ObjCacheInTree *inObject)
{
	Blob4D *theBlob = dynamic_cast<Blob4D*>( inObject);
	if (theBlob != nil)
		theBlob->Release();
}


TreeInMem *BlobTreeInMemHeader::CreTreeInMem()
{
	return new BlobTreeInMem(ObjCache::RecordAccess, FeuilleFinaleContientQuoi);
}


								/* ----------------------------------------------- */
			
													
TreeInMem *LockEntityTreeInMem::CreTreeInMem()
{
	return new LockEntityTreeInMem(FeuilleFinaleContientQuoi);
}



TreeInMem *LockEntityTreeInMemHeader::CreTreeInMem()
{
	return new LockEntityTreeInMem(FeuilleFinaleContientQuoi);
}

			
#endif
								
								/* ----------------------------------------------- */
								



DataTable::~DataTable()
{
	if (crit != nil) 
	{
		crit->setDF(nil);
		if (fDefIsInDataPart)
			crit->Release();
	}
}


DataTable::DataTable()
{
	fRealNum = -1;
	fTableDefNumInData = 0;
	crit = nil;
	db = nil;
	//pastouchefiche=false;
	fIsDeleted = false;
	fIsInvalid = false;
	fDefIsInDataPart = false;
	fNotDeletedRequest = 0;
	fSeqRatioCorrector = 20;
}


DataTable::DataTable(Base4D *xdb, Table* xcrit, sLONG xnum)
{
	// il est possible que xcrit soit nil
	// cela veut dire que l'on a charge un datafile avec une table detruite
	db=xdb;
	crit=xcrit;
	/*
	if (crit != nil) 
		crit->Retain();
		*/
	fRealNum = xnum;
	fNotDeletedRequest = 0;
	fIsDeleted = false;
	//pastouchefiche=false;
	fIsInvalid = false;
	fDefIsInDataPart = false;
	if (crit != nil)
		crit->setDF(this);
	fSeqRatioCorrector = 20;

}


void DataTable::SetAssociatedTable(Table* tt, sLONG TableDefNumInData)
{	
	crit = tt;

	fTableDefNumInData = TableDefNumInData;

	if (crit != nil)
	{
		crit->setDF(this);
	}
}


VError DataTable::ThrowError( VError inErrCode, ActionDB4D inAction) const
{
	// L.E. 10/09/02 sometimes, db has no owner

	VErrorDB4D_OnTable *err = new VErrorDB4D_OnTable(inErrCode, inAction, db, GetNum());
	VTask::GetCurrent()->PushRetainedError( err);

	return inErrCode;
}

Table* DataTable::RetainTable() const
{
	VTaskLock lock(&fAdminAccess);
	if (crit != nil)
		crit->Retain();
	return crit;
}


typedef map<Table*, FicheInMem*> RecordByTableMap;

VError DataTable::DataToCollection(Selection* sel, DB4DCollectionManager& collection, sLONG FromRecInSel, sLONG ToRecInSel,
																	BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, bool pictAsRaw,
																	bool& FullyCompleted, sLONG& maxElems, bool TestLimit )
{
	FullyCompleted = true;
	VError err = VE_OK;
	VSize totmem = 0;
	if (NotDeleted(err))
	{
		SelectionIterator itersel(sel);
		VArrayRetainedOwnedPtrOf<Field*> cols;

		VArrayPtrOf<RelationPath*> RelPaths;
		RelPaths.SetOwnership(true);	// to avoid mem. leaks

		sLONG nbcol = collection.GetNumberOfColumns();
		sLONG rawnumber = -1;

		VLong RecNumber;

		sLONG qt = sel->GetQTfic();
		if (qt > 0)
		{
			if (FromRecInSel < 0)
				FromRecInSel = 0;
			if (ToRecInSel < 0 || ToRecInSel >= qt)
				ToRecInSel = qt - 1;
			qt = ToRecInSel - FromRecInSel + 1;
			maxElems = qt;

			err = collection.SetCollectionSize(qt);

			if (err == VE_OK)
			{
				if (qt > 0)	// do nothing if 0
				{
					Bittab dejapris;

					if (cols.SetCount(nbcol, nil) && RelPaths.SetCount(nbcol, nil))
					{
						RecordByTableMap RecsInTable;

						bool tryWithRawRecord = collection.AcceptRawData() && AcceptNotFullRecord();
						
						sLONG i;
						for (i = 0; i < nbcol && err == VE_OK; i++)
						{
							CDB4DField* f = collection.GetColumnRef(i+1);
							if (f != nil)
							{
								Field* cri = (VImpCreator<VDB4DField>::GetImpObject(f))->GetField();
								if (cri != nil)
								{
									cri->Retain();
									cols[i] = cri;
									
									if (crit != cri->GetOwner())
									{
										tryWithRawRecord = false;
										vector<uBYTE> none;
										RelationPath* rp = new RelationPath;
										if (rp->BuildPath(context, crit, cri->GetOwner(), err, true, true, none))
										{
											RelPaths[i] = rp;
										}
										else
											delete rp;
									}
								}
							}
						}

						Boolean TousDejaPris = false;

						if (err == VE_OK && tryWithRawRecord)
						{
							StErrorContextInstaller errs(false);
							VError err2 = VE_OK;
							SelPosIterator positer;
							err2 = positer.InitWithSel(sel, FromRecInSel, ToRecInSel);
							if (err2 == VE_OK)
							{
								StLockerRead lock(this);
								OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
								err2 = FillCollection(collection, dejapris, positer, context, cols, curstack);
								TousDejaPris = positer.AllTaken();
							}
							Transaction* trans = GetCurrentTransaction(context);

							if (trans != nil)
							{
								Bittab* b = trans->GetSavedRecordIDs(GetNum(), err, false);
								if (b != nil)
								{
									TousDejaPris = false;
									dejapris.moins(b);
								}

								b = trans->GetDeletedRecordIDs(GetNum(), err, false);
								if (b != nil)
								{
									TousDejaPris = false;
									dejapris.moins(b);
								}
							}
							
						}

						if (err == VE_OK && !TousDejaPris)
						{
							sLONG curfic;
							if (FromRecInSel == 0)
								curfic = itersel.FirstRecord();
							else
								curfic = itersel.SetCurrentRecord(FromRecInSel);

							SmallBittab<32, 65536> dejaTable;

							if (InProgress != nil)
							{
								XBOX::VString session_title;
								gs(1005,25, session_title);
								InProgress->BeginSession(qt, session_title);
							}

							ReadAheadBufferActivator buf(db);
							while ((curfic != -1) && (rawnumber < (qt-1)) && FullyCompleted)
							{
								rawnumber++;

								if (!dejapris.isOn(curfic))
								{

									if (InProgress != nil)
									{
										InProgress->Progress(rawnumber);
									}

									bool tryWithFicInMem = true;
									if (tryWithRawRecord)
									{
										FicheOnDisk* ficD = LoadNotFullRecord(curfic, err, DB4D_Do_Not_Lock /* readonly*/ , context, false /*BitLockOnly*/, buf.GetBuffer(), nil /*couldlock*/, nil /*notenoughmem*/);
										if (ficD != nil)
										{
											tryWithFicInMem = false;
											for (i = 0; (i < nbcol) && (err == VE_OK) && !tryWithFicInMem; i++)
											{
												Field* cri = cols[i];
												if (cri == nil || cri->GetPosInRec() == 0)
												{
													sLONG recnum = ficD->getnumfic();
													err = collection.SetNthElementRawData(rawnumber+1, i+1, &recnum, VK_LONG, &tryWithFicInMem);
												}
												else
												{
													sLONG fieldDataType;
													tPtr fieldData = ficD->GetDataPtrForQuery( cri->GetPosInRec(), &fieldDataType, true, false);
													if (fieldDataType == VK_TEXT) // les blobs ne sont pas supportes par GetDataPtrForQuery
														tryWithFicInMem = true;
													else
														err = collection.SetNthElementRawData(rawnumber+1, i+1, fieldData, (ValueKind) fieldDataType, &tryWithFicInMem);
												}
											}
											ficD->Release();
										}
										else
										{
											if (err != VE_OK)
												break;
										}
									}

									if (tryWithFicInMem)
									{
										dejaTable.ClearAll();

										FicheInMem* fic = LoadRecord(curfic, err, DB4D_Do_Not_Lock, context, false, false, buf.GetBuffer());
										if (fic == nil)
										{
											if (err != VE_OK)
												break;
										}
										else
										{
											for (i = 0; i < nbcol; i++)
											{

												Field* cri = cols[i];
												if (cri == nil)
												{
													RecNumber.FromLong(fic->GetNum());
													err = collection.SetNthElement(rawnumber+1, i+1, RecNumber);
													totmem += RecNumber.GetFullSpaceOnDisk();

												}
												else
												{
													if (cri->GetOwner() == crit)
													{
														ValPtr cv = fic->GetFieldValue(cri, err, pictAsRaw);
														if (err != VE_OK)
															break;
														if (cv != nil)
														{
															totmem += cv->GetFullSpaceOnDisk();
															err = collection.SetNthElement(rawnumber+1, i+1, *cv);
														}
														else
														{
															totmem += 6;
															err = collection.SetNthElement(rawnumber+1, i+1, *(cri->GetEmptyValue()));
														}
													}
													else
													{
														FicheInMem* destfic = nil;

														Table* destTable = cri->GetOwner();
														sLONG numTable = destTable->GetNum();
														if (dejaTable.isOn(numTable))
														{
															destfic = RecsInTable[destTable];
														}
														else
														{
															RelationPath* rp = RelPaths[i];
															if (rp != nil)
																err = rp->ActivateRelation(fic, destfic, context, true);
															if (err == VE_OK)
															{
																dejaTable.Set(numTable);
																RecsInTable[destTable] = destfic;
															}
														}
														if (err == VE_OK)
														{
															if (destfic != nil)
															{
																ValPtr cv = destfic->GetFieldValue(cri, err, pictAsRaw);
																if (err == VE_OK)
																{
																	if (cv != nil)
																	{
																		totmem += cv->GetFullSpaceOnDisk();
																		err = collection.SetNthElement(rawnumber+1, i+1, *cv);
																	}
																	else
																	{
																		totmem += 6;
																		err = collection.SetNthElement(rawnumber+1, i+1, *(cri->GetEmptyValue()));
																	}
																}
															}
															else
															{
																err = collection.SetNthElement(rawnumber+1, i+1, *(cri->GetEmptyValue()));
																totmem += 6;
															}
														}
														if (err != VE_OK)
															break;
													}
												}
											}

											for (RecordByTableMap::iterator cur = RecsInTable.begin(), end = RecsInTable.end(); cur != end; cur++)
											{
												if (cur->second != nil)
													cur->second->Release();
											}

											fic->Release();
											if (err != VE_OK)
												break;
										}
									}
								}

								if (totmem > 10000000 && TestLimit)
									FullyCompleted = false;
								maxElems = rawnumber + 1;
								curfic = itersel.NextRecord();
							}

							if (InProgress != nil)
							{
								InProgress->EndSession();
							}

						}

					}
					else
						err = ThrowError(memfull, DBaction_DataToCollection);
				}
			}
		}
		else
			err = collection.SetCollectionSize(0);

		LibereDelete();
	}

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOT_COMPLETE_DATA_TO_COLLECTION, DBaction_DataToCollection);

	return err;
}


VError DataTable::CollectionToData(Selection* sel, DB4DCollectionManager& collection, BaseTaskInfo* context,
																	Boolean AddToSel, Boolean CreateAlways, Selection* &outSel, Bittab* &outLockSet, VDB4DProgressIndicator* InProgress)
{
	VError err = VE_OK;
	if (NotDeleted(err))
	{
		SelectionIterator itersel(sel);
		VArrayRetainedOwnedPtrOf<Field*> cols;
		sLONG nbcol = collection.GetNumberOfColumns();
		sLONG nbraw = collection.GetCollectionSize();
		sLONG rawnumber = 0;
		outLockSet = nil;
		outSel = sel;

		if (cols.SetCount(nbcol, nil))
		{
			sLONG i;
			for (i = 0; i < nbcol; i++)
			{
				CDB4DField* f = collection.GetColumnRef(i+1);
				if (f != nil)
				{
					Field* cri = (VImpCreator<VDB4DField>::GetImpObject(f))->GetField();
					if (cri != nil)
					{
						cri->Retain();
						cols[i] = cri;
					}
				}
			}

			if (InProgress != nil)
			{
				XBOX::VString session_title;
				gs(1005,26, session_title);
				InProgress->BeginSession(nbraw, session_title, false);
			}


			sLONG curfic = itersel.FirstRecord();
			for (rawnumber = 0; rawnumber < nbraw; rawnumber++)
			{

				if (InProgress != nil)
				{
					InProgress->Progress(rawnumber);
				}

				Boolean newone = false, mustreplace = false;
				FicheInMem* fic;
				if (CreateAlways || (curfic == -1))
				{
					fic = NewRecord(err, context);
					newone = true;
					if (fic == nil)
					{
						break;
					}
				}
				else
				{
					fic = LoadRecord(curfic, err, DB4D_Keep_Lock_With_Record, context, false, false, nil);
					if (fic == nil)
					{
						if (err != VE_OK)
							break;
						fic = NewRecord(err, context);
						mustreplace = true;
						newone = true;
						if (fic == nil)
						{
							err = ThrowError(memfull, DBaction_CollectionToData);
							break;
						}
					}
				}

				if (fic->ReadOnlyState())
				{
					if (testAssert(!newone))
					{
						if (outLockSet == nil)
						{
							outLockSet = new Bittab();
							err = outLockSet->aggrandit(GetMaxRecords(context));
						}
						if (err == VE_OK)
						{
							err = outLockSet->Set(fic->GetNum());
						}
					}
				}
				else
				{

					for (i = 0; i < nbcol; i++)
					{
						Field* cri = cols[i];
						if (cri != nil)
						{
							ValPtr cv = fic->GetFieldValue(cri, err);
							ConstValPtr from;
							bool disposeIt = false;
							if (err == VE_OK)
								err = collection.GetNthElement(rawnumber+1, i+1, from, &disposeIt);
							if (err != VE_OK)
								break;
							if (from != nil)
							{
								from->GetValue(*cv);
								fic->Touch(cri);
								if (disposeIt)
									delete from;
							}
						}
					}

					if (err != VE_OK)
						break;
					err = SaveRecord(fic, context);
					if (err == VE_OK)
					{
						if (newone && AddToSel)
						{
							Selection* newsel = outSel->AddToSelection(fic->GetNum(), err);
							if (newsel != outSel)
							{
								if (outSel != sel)
									outSel->Release();
								outSel = newsel;
							}
						}
					}
				}

				fic->Release();
				if (err != VE_OK)
					break;

				curfic = itersel.NextRecord();
			}

			if (InProgress != nil)
			{
				InProgress->EndSession();
			}

		}
		else
			err = ThrowError(memfull, DBaction_DataToCollection);
		LibereDelete();
	}

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOT_COMPLETE_COLLECTION_TO_DATA, DBaction_CollectionToData);
	return err;
}



Bittab* DataTable::Search(VError& err, RechNode* rn, Bittab *cursel, VDB4DProgressIndicator* InProgress, 
						 BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, Bittab &Nulls, EntityModel* model)
{
	//CheckUseCount();

	uBOOL oktot;
	Bittab *sel = nil, *filtre = nil;
	sLONG i,nb,counterr;
	BaseTaskInfo *tfb = context;
	sLONG nf;
	Boolean NeedToLoadFullRec = !AcceptNotFullRecord(), mustlimit = limit > 0;
	sLONG nbfound = 0;

	NeedToLoadFullRec = NeedToLoadFullRec || rn->NeedToLoadFullRec();

	err=VE_OK;

	if (NotDeleted(err))
	{

		counterr=0;
		sel=new Bittab;
		if (sel!=nil)
		{
			nb=GetMaxRecords(context);
			err=sel->aggrandit(nb);
			if (cursel!=nil) 
				nb=cursel->GetMaxBit();
			if (err==VE_OK)
			{

				if (InProgress != nil)
				{
					XBOX::VString session_title;
					VValueBag bag;
					VStr<64> tname;
					if (crit != nil)
						crit->GetName(tname);
					bag.SetString("TableName", tname);
					bag.SetString("curValue", L"{curValue}");
					bag.SetString("maxValue", L"{maxValue}");
					gs(1005,4,session_title);	// Sequential Search: %curValue of %maxValue Records
					session_title.Format(&bag);
					InProgress->BeginSession( (cursel == nil) ? nb : cursel->Compte(), session_title);
				}

				nf = GetNum();
				{
					Transaction* trans = GetCurrentTransaction(context);

					filtre = new Bittab;
					err=filtre->aggrandit(nb);
					Bittab* dejalocked = nil;
					if (trans != nil && HowToLock == DB4D_Keep_Lock_With_Transaction)
						dejalocked = trans->GetKeptLocks(this, err, false);

					if (cursel == nil && !NeedToLoadFullRec)
					{
						StLockerRead lock(this);
						OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
						err = PerformRech(cursel, rn, sel, filtre, InProgress, context, HowToLock, exceptions, limit, nbfound, Nulls, dejalocked, curstack, model);
					}

					if (err == VE_OK)
					{
						filtre->Invert();

						if (trans != nil)
						{
							Bittab* b = trans->GetSavedRecordIDs(GetNum(), err, false);
							if (b != nil)
							{
								sel->moins(b);
								filtre->Or(b);
								//	b->libere();
							}

							// ce deuxieme moins n'est normalement pas necessaire
							b = trans->GetDeletedRecordIDs(GetNum(), err, false);
							if (b != nil)
							{
								sel->moins(b);
								filtre->moins(b);
								//	b->libere();
							}
						}

						if (cursel != nil)
						{
							filtre->And(cursel);
							sel->And(cursel);
						}

						ReadAheadBuffer* buf = db->NewReadAheadBuffer();

						if (NeedToLoadFullRec)
						{
							FicheInMem *fic;

							if (filtre!=nil) i=filtre->FindNextBit(0);
							else i=0;

							while ( (i<nb) && (i!=-1) && (counterr<MaxErrSeq) && (err!=VE_DB4D_ACTIONCANCELEDBYUSER) )
							{
								if (mustlimit && nbfound>=limit)
									break;

								Boolean waskept = dejalocked != nil && dejalocked->isOn(i);
								fic=LoadRecord(i, err, HowToLock, tfb, false, false, buf);
								if (InProgress != nil)
									InProgress->Increment(); 

								if (MustStopTask(InProgress))
									err = ThrowError(VE_DB4D_ACTIONCANCELEDBYUSER, DBaction_SeqSearchingInDataTable);

								if (err == VE_OK)
								{
									if (fic!=nil)
									{
										oktot=rn->PerformSeq(nf,fic,tfb, nil, context, HowToLock, exceptions, limit, model);

										if (oktot == 0 || oktot == 2)
										{
											if (!waskept && trans != nil)
											{
												trans->DoNotKeepRecordLocked(this, i);
											}
										}

										if (oktot == 2)
										{
											err = Nulls.Set(nf, true);
										}
										else
										{
											if (oktot)
											{
												if (HowToLock > DB4D_Keep_Lock_With_Record && fic->ReadOnlyState())
												{
													if (exceptions != nil)
														exceptions->Set(i, true);
													err  = ThrowError(VE_DB4D_RECORDISLOCKED, DBaction_SeqSearchingInDataTable);
													counterr=MaxErrSeq;
												}
												else
												{
													err=sel->Set(i);
													if (err!=VE_OK)
													{
														counterr=MaxErrSeq;
													}
													else nbfound++;
												}
											}
										}
									}

								} // du if fic!=nil
								else
								{
									counterr++;
									if (counterr<MaxErrSeq)
									{
										err = PullLastError();
										err = VE_OK;
									}
								}

								if (fic != nil)
									fic->Release();

								if (filtre==nil) ++i;
								else i=filtre->FindNextBit(i+1);

							} // du for i
						}
						else
						{
							FicheOnDisk *ficD;

							if (filtre!=nil) i=filtre->FindNextBit(0);
							else i=0;

							while ( (i<nb) && (i!=-1) && (counterr<MaxErrSeq) && (err!=VE_DB4D_ACTIONCANCELEDBYUSER) )
							{
								if (mustlimit && nbfound>=limit)
									break;

								Boolean waskept = dejalocked != nil && dejalocked->isOn(i);
								Boolean couldlock;
								ficD=LoadNotFullRecord(i, err, HowToLock, tfb, false, buf, &couldlock);

								if (err == VE_OK)
								{
									if (InProgress != nil)
										InProgress->Increment();
									if (MustStopTask( InProgress))
										err = ThrowError(VE_DB4D_ACTIONCANCELEDBYUSER, DBaction_SeqSearchingInDataTable);
								}

								if (err == VE_OK)
								{
									if (ficD!=nil)
									{
										oktot=rn->PerformSeq(ficD,tfb, nil, context, HowToLock, exceptions, limit, model);

										if (oktot == 0 || oktot == 2)
										{
											if (!waskept && trans != nil)
											{
												trans->DoNotKeepRecordLocked(this, i);
											}
										}

										if (oktot == 2)
										{
											err = Nulls.Set(nf, true);
										}
										else
										{
											if (oktot)
											{
												if (HowToLock > DB4D_Keep_Lock_With_Record && !couldlock)
												{
													if (exceptions != nil)
														exceptions->Set(i, true);
													err  = ThrowError(VE_DB4D_RECORDISLOCKED, DBaction_SeqSearchingInDataTable);
													counterr=MaxErrSeq;
												}
												else
												{
													err=sel->Set(i);
													if (err!=VE_OK)
													{
														counterr=MaxErrSeq;
													}
													else
														nbfound++;
												}
											}
										}
									}

								} // du if fic!=nil
								else
								{
									counterr++;
									if (counterr<MaxErrSeq)
									{
										err = PullLastError();
										err = VE_OK;
									}
								}

								if (ficD != nil)
								{
									ficD->Release();
								}

								if (filtre==nil) ++i;
								else i=filtre->FindNextBit(i+1);

							} // du for i
						}


						if (buf != nil)
							buf->Release();
					}

					ReleaseRefCountable(&filtre);

					//delete tfb;
				}

				if (InProgress != nil)
				{
					InProgress->EndSession();
				}
				//delete indic;

				// if ((counterr<MaxErrSeq) && (err!=kActionCanceled) ) err=VE_OK;
			} // du if err==0

		} // du if sel!=nil
		else
		{
			err = ThrowError(memfull, DBaction_SeqSearchingInDataTable);
		}
		LibereDelete();
	}
	else
		err = ThrowError(err, DBaction_SeqSearchingInDataTable);

	if (err!=VE_OK)
	{
		if ( sel != 0 )
			ReleaseRefCountable(&sel);
		err = ThrowError(VE_DB4D_SEARCHCOULDNOTCOMPLETE, DBaction_SeqSearchingInDataTable);
	}

	//CheckUseCount();

	return(sel);
}




Bittab* DataTable::Search(VError& err, SimpleQueryNode* rn, Bittab *cursel, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, 
						 DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, Bittab &Nulls)
{
	//CheckUseCount();

	uBOOL oktot;
	Bittab *sel, *filtre = nil;
	sLONG i,nb,counterr;
	BaseTaskInfo *tfb = context;
	sLONG nf;
	Boolean NeedToLoadFullRec = !AcceptNotFullRecord(), mustlimit = limit > 0;
	sLONG nbfound = 0;

	//NeedToLoadFullRec = NeedToLoadFullRec || rn->NeedToLoadFullRec();

	err=VE_OK;

	if (NotDeleted(err))
	{
		counterr=0;
		sel=new Bittab;
		if (sel!=nil)
		{
			nb=GetMaxRecords(context);
			err=sel->aggrandit(nb);
			if (cursel!=nil) nb=cursel->GetMaxBit();
			if (err==VE_OK)
			{

				if (InProgress != nil)
				{
					XBOX::VString session_title;
					VValueBag bag;
					VStr<64> tname;
					if (crit != nil)
						crit->GetName(tname);
					bag.SetString("TableName", tname);
					bag.SetString("curValue", L"{curValue}");
					bag.SetString("maxValue", L"{maxValue}");
					gs(1005,4,session_title);	// Sequential Search: %curValue of %maxValue Records
					session_title.Format(&bag);
					InProgress->BeginSession(nb,session_title);
				}

				nf = GetNum();
				{
					Transaction* trans = GetCurrentTransaction(context);
					filtre = new Bittab;
					err=filtre->aggrandit(nb);
					Bittab* dejalocked = nil;
					if (trans != nil && HowToLock == DB4D_Keep_Lock_With_Transaction)
						dejalocked = trans->GetKeptLocks(this, err, false);

					if (cursel == nil)
					{
						StLockerRead lock(this);
						OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
						PerformRech(cursel, rn, sel, filtre, InProgress, context, HowToLock, exceptions, limit, nbfound, Nulls, dejalocked, curstack);
					}

					if (err == VE_OK)
					{
						filtre->Invert();

						if (trans != nil)
						{
							Bittab* b = trans->GetSavedRecordIDs(GetNum(), err, false);
							if (b != nil)
							{
								sel->moins(b);
								filtre->Or(b);
								//	b->libere();
							}

							// ce deuxieme mois n'est normalement pas necessaire
							b = trans->GetDeletedRecordIDs(GetNum(), err, false);
							if (b != nil)
							{
								sel->moins(b);
								filtre->moins(b);
								//	b->libere();
							}
						}

						if (cursel != nil)
						{
							filtre->And(cursel);
							sel->And(cursel);
						}

						ReadAheadBuffer* buf = db->NewReadAheadBuffer();

						if (NeedToLoadFullRec)
						{
							FicheInMem *fic;

							if (filtre!=nil) i=filtre->FindNextBit(0);
							else i=0;

							while ( (i<nb) && (i!=-1) && (counterr<MaxErrSeq) && (err!=VE_DB4D_ACTIONCANCELEDBYUSER) )
							{
								if (mustlimit && nbfound>=limit)
									break;

								Boolean waskept = dejalocked != nil && dejalocked->isOn(i);
								fic=LoadRecord(i, err, HowToLock, tfb, false, false, buf);
								if (InProgress != nil)
									InProgress->Increment();

								if (MustStopTask(InProgress))
									err = ThrowError(VE_DB4D_ACTIONCANCELEDBYUSER, DBaction_SeqSearchingInDataTable);

								if (err == VE_OK)
								{
									if (fic!=nil)
									{
										oktot=rn->PerformSeq(fic,tfb, nil, context, HowToLock, exceptions, limit);

										if (oktot == 4)
										{
											err = VE_DB4D_ACTIONCANCELEDBYUSER;
											if (!waskept && trans != nil)
											{
												trans->DoNotKeepRecordLocked(this, i);
											}
										}
										else
										{
											if (oktot == 0 || oktot == 2)
											{
												if (!waskept && trans != nil)
												{
													trans->DoNotKeepRecordLocked(this, i);
												}
											}

											if (oktot == 2)
											{
												err = Nulls.Set(nf, true);
											}
											else
											{
												if (oktot)
												{
													if (HowToLock > DB4D_Keep_Lock_With_Record && fic->ReadOnlyState())
													{
														if (exceptions != nil)
															exceptions->Set(i, true);
														err  = ThrowError(VE_DB4D_RECORDISLOCKED, DBaction_SeqSearchingInDataTable);
														counterr=MaxErrSeq;
													}
													else
													{
														err=sel->Set(i);
														if (err!=VE_OK)
														{
															counterr=MaxErrSeq;
														}
														else nbfound++;
													}
												}
											}
										}
									}

								} // du if fic!=nil
								else
								{
									counterr++;
									if (counterr<MaxErrSeq)
									{
										err = PullLastError();
										err = VE_OK;
									}
								}

								if (fic != nil)
									fic->Release();

								if (filtre==nil) ++i;
								else i=filtre->FindNextBit(i+1);

							} // du for i
						}
						else
						{
							FicheOnDisk *ficD;

							if (filtre!=nil) i=filtre->FindNextBit(0);
							else i=0;

							while ( (i<nb) && (i!=-1) && (counterr<MaxErrSeq) && (err!=VE_DB4D_ACTIONCANCELEDBYUSER) )
							{
								if (mustlimit && nbfound>=limit)
									break;

								Boolean waskept = dejalocked != nil && dejalocked->isOn(i);
								Boolean couldlock;
								ficD=LoadNotFullRecord(i, err, HowToLock, tfb, false, buf, &couldlock);

								if (err == VE_OK)
								{
									if (InProgress != nil)
										InProgress->Increment();

									if (MustStopTask(InProgress))
										err = ThrowError(VE_DB4D_ACTIONCANCELEDBYUSER, DBaction_SeqSearchingInDataTable);
								}

								if (err == VE_OK)
								{
									if (ficD!=nil)
									{
										oktot=rn->PerformSeq(ficD,tfb, nil, context, HowToLock, exceptions, limit);

										if (oktot == 4)
										{
											err = VE_DB4D_ACTIONCANCELEDBYUSER;
											if (!waskept && trans != nil)
											{
												trans->DoNotKeepRecordLocked(this, i);
											}
										}
										else
										{
											if (oktot == 0 || oktot == 2)
											{
												if (!waskept && trans != nil)
												{
													trans->DoNotKeepRecordLocked(this, i);
												}
											}

											if (oktot == 2)
											{
												err = Nulls.Set(nf, true);
											}
											else
											{
												if (oktot)
												{
													if (HowToLock > DB4D_Keep_Lock_With_Record && !couldlock)
													{
														if (exceptions != nil)
															exceptions->Set(i, true);
														err  = ThrowError(VE_DB4D_RECORDISLOCKED, DBaction_SeqSearchingInDataTable);
														counterr=MaxErrSeq;
													}
													else
													{
														err=sel->Set(i);
														if (err!=VE_OK)
														{
															counterr=MaxErrSeq;
														}
														else
															nbfound++;
													}
												}
											}
										}
									}

								} // du if fic!=nil
								else
								{
									counterr++;
									if (counterr<MaxErrSeq)
									{
										err = PullLastError();
										err = VE_OK;
									}
								}

								if (ficD != nil)
								{
									ficD->Release();
								}

								if (filtre==nil) ++i;
								else i=filtre->FindNextBit(i+1);

							} // du for i
						}


						if (buf != nil)
							buf->Release();
					}

					ReleaseRefCountable(&filtre);

				}

				if (InProgress != nil)
				{
					InProgress->EndSession();
				}
				//delete indic;

				// if ((counterr<MaxErrSeq) && (err!=kActionCanceled) ) err=VE_OK;
			} // du if err==0

		} // du if sel!=nil
		else
		{
			err = ThrowError(memfull, DBaction_SeqSearchingInDataTable);
		}
		LibereDelete();
	}
	else
		err = ThrowError(err, DBaction_SeqSearchingInDataTable);

	if (err!=VE_OK)
	{
		ReleaseRefCountable(&sel);
		err = ThrowError(VE_DB4D_SEARCHCOULDNOTCOMPLETE, DBaction_SeqSearchingInDataTable);
	}

	//CheckUseCount();

	return(sel);
}


Boolean DataTable::NotDeleted(VError& outErr, ActionDB4D action) const
{
	//if (db->IsLogValid())
	{
		VInterlocked::Increment(&fNotDeletedRequest);
		if (!fIsDeleted)
			return true;
		else
		{
			VInterlocked::Decrement(&fNotDeletedRequest);
			outErr = VE_DB4D_DATATABLE_HAS_BEEN_DELETED;
			if (action != (ActionDB4D)-1)
				ThrowError(outErr, action);
			return false;
		}
	}
	/*
	else
	{
		outErr = VE_DB4D_CURRENT_JOURNAL_IS_INVALID;
		if (action != (ActionDB4D)-1)
			ThrowError(outErr, action);
		return false;
	}
	*/
};


VError DataTable::ExecuteQuery(SearchTab* query, QueryOptions* options, QueryResult& outResult, BaseTaskInfo* context, VDB4DProgressIndicator* inProgress)
{
	VError outError;
	Selection *sel;
	OptimizedQuery rech;

	Boolean olddescribe = false;
	if (context != nil)
	{
		olddescribe = context->ShouldDescribeQuery();
		context->MustDescribeQuery(options->ShouldDescribeQuery());
	}

	Selection* oldsel = options->GetFilter();
	Bittab* lockedset = nil;
	if (options->GetWantsLockedSet())
	{
		lockedset = new Bittab;
		lockedset->SetOwner(db);
	}

	db->LockIndexes();
	outError = rech.AnalyseSearch(query, context);
	db->UnLockIndexes();
	if (outError == VE_OK)
	{
		sel = rech.Perform(oldsel, inProgress, context, outError, options->GetWayOfLocking(), (sLONG)options->GetLimit(), lockedset);

		if (sel != nil)
		{
			{
				DB4D_QueryDestination dest = options->GetDestination();

				if (context!= nil && context->ShouldDescribeQuery())
				{
					context->GetLastQueryDescription(outResult.GetQueryDescription());
					context->GetLastQueryExecution(outResult.GetQueryExecution());
					context->GetLastQueryExecutionXML(outResult.GetQueryExecutionXML());
				}

				if ((dest & DB4D_QueryDestination_Count) != 0)
					outResult.SetCount(sel->GetQTfic());

				if ((dest & DB4D_QueryDestination_Selection) != 0)
					outResult.SetSelection(sel);

				if ((dest & DB4D_QueryDestination_Sets) != 0)
				{
					Bittab* b = sel->StealBittab(context, outError);
					outResult.SetSet(b);
					b->Release();
				}

				if (options->GetWantsFirstRecord() && !sel->IsEmpty())
				{
					FicheInMem* rec = LoadRecord(sel->GetFic(0), outError, options->GetRecordWayOfLocking(), context, true);
					outResult.SetFirstRecord(rec);
					ReleaseRefCountable(&rec);

				}

			}
		}

		if (lockedset != nil && lockedset->Compte() != 0)
		{
			outResult.SetLockedSet(lockedset);
		}

		ReleaseRefCountable(&sel);
	}

	ReleaseRefCountable(&lockedset);

	if (context != nil)
	{
		context->MustDescribeQuery(olddescribe);
	}

	return outError;
}


Selection* DataTable::AllRecords(BaseTaskInfo* context, VError& err)
{
	err = VE_OK;
	Selection* result = nil;

	if (NotDeleted(err))
	{
		result = new BitSel(this);
		if (result == nil)
			err = ThrowError(memfull, DBaction_SeqSearchingInDataTable);
		else
		{
			Bittab* bb = ((BitSel*)result)->GetBittab();

			sLONG nbrec = GetMaxRecords(context);
			err = bb->aggrandit(nbrec);
			if (err == VE_OK)
			{
				bb->ClearOrSetAll(true);
				err = TakeOffBits(bb, context);
			}
			if (err == VE_OK)
			{
				result->Touch();
			}
			else
			{
				result->Release();
				result = nil;
			}
		}
		LibereDelete();
	}

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_SEARCHCOULDNOTCOMPLETE, DBaction_SeqSearchingInDataTable);
	return result;
}


Selection* DataTable::SortSel(SortTab& inSort, Selection* inSel, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, VError& err)
{
	sLONG i,nb;
	FieldNuplet *fn;
	SortLine p1;
	FieldDef *p2;
	IndexInfo *ind = nil;
	Base4D *bd;
	Selection *newsel = nil;
	Boolean ascent = true;
	SortContext tri;
	Boolean mustreleaseinsel = false;

	err=VE_OK;
	if (NotDeleted(err))
	{
		nb=inSort.GetNbLine();
		fn=new FieldNuplet(nb,false);
		if (fn==nil)
		{
			err = ThrowError(memfull, DBaction_SortingSelection);
		}
		else
		{
			bool okToLookForIndex = true; 
			p2=fn->GetDataPtr();
			for (i=1;i<=nb;i++)
			{
				inSort.GetTriLine(i, &p1);
				if (i == 1) 
					ascent = p1.ascendant;
				else
				{
					if (p1.ascendant != ascent)
						okToLookForIndex = false;
				}
				p2->numfield=p1.numfield;
				p2->numfile=p1.numfile;
				p2++;
			}

			bd=GetDB();

			fn->UpdateCritFic(bd);

			sLONG count;

			if (inSel == nil)
				count = GetNbRecords(context);
			else
				count = inSel->GetQTfic();

			// bd->LockIndexes();

			Table* assoctable = RetainTable();
			if (assoctable != nil)
			{
				if (okToLookForIndex)
					ind=assoctable->FindAndRetainIndex(fn, true);
				assoctable->Release();
			}
			if (ind != nil)
			{
				if ((sLONG8)count * assoctable->GetSeqRatioCorrector() < (sLONG8)ind->GetNBDiskAccessForAScan(true))
				{
					ind->ReleaseValid();
					ind->Release();
					ind = nil;
				}
			}

			if (ind!=nil)
			{
				ind->Open(index_read);
				// bd->UnLockIndexes();
				OccupableStack* curstack = gOccPool.GetStackForCurrentThread();

				newsel=ind->GetHeader()->SortSel(curstack, inSel, ascent, context, err, InProgress, false);
				ind->Close();
				ind->ReleaseValid();
				ind->Release();
			}
			else
			{
				if (inSel == nil)
				{
					inSel = new BitSel(this);
					if (inSel == nil)
						err = ThrowError(memfull, DBaction_SortingSelection);
					else
					{
						mustreleaseinsel = true;
						Bittab *bb = ((BitSel*)inSel)->GetBittab();
						err = bb->aggrandit(GetMaxRecords(context));
						if (err == VE_OK)
						{
							bb->ClearOrSetAll(true);
							err = TakeOffBits(bb, context);
						}
					}
				}
				// bd->UnLockIndexes();
				if (inSel!=nil && err == VE_OK)
				{
					newsel = inSel->SortSel(err, &inSort, context, InProgress, false);
					if (newsel == nil && err == VE_OK)
					{
						err = ThrowError(memfull, DBaction_SortingSelection);
					}
				}

				if (mustreleaseinsel && inSel != nil)
					inSel->Release();
			}

			delete fn;
		}
		LibereDelete();
	}

	// ClearError();

	if (err != VE_OK)
	{
		if (newsel != nil)
			newsel->Release();
		newsel = nil;
	}

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTCOMPLETESORTOFINDEX, DBaction_SortingSelection);
	return newsel;
}




													/* ************************************************************* */
/*

void ReadWriteMutexTable::Lock(sLONG n)
{
	Boolean stop = false;
	while (!stop)
	{
		fMapMutex.Lock();
		ReadWriteMutexMap::iterator found = fMap.find(n);
		if (found == fMap.end())
		{
			fMap[n] = ReadWriteMutex();
			fMapMutex.Unlock();
			stop =true;
		}
		else
		{
			VSyncEvent* wait = found->second.fWaitingEvent;
			if (wait == nil)
			{
				wait = new VSyncEvent();
				found->second.fWaitingEvent = wait;
			}
			wait->Retain();
			fMapMutex.Unlock();
			wait->Lock();
			wait->Release();

			stop = false;
		}
	}
}


void ReadWriteMutexTable::Unlock(sLONG n)
{
	fMapMutex.Lock();
	ReadWriteMutexMap::iterator found = fMap.find(n);
	if (testAssert(found != fMap.end()))
	{
		VSyncEvent* wait = found->second.fWaitingEvent;
		if (wait != nil)
		{
			wait->Unlock();
			wait->Release();
		}
		fMap.erase(found);
	}
	fMapMutex.Unlock();
}

*/


													/* ************************************************************* */



void RecordTreeMemHeader::KillElem(void* inObject)
{
	((FicheOnDisk*)inObject)->setmodif(false, ((FicheOnDisk*)inObject)->GetDB(), nil);
	((FicheOnDisk*)inObject)->Release();
}


void RecordTreeMemHeader::RetainElem(void* inObject)
{
	((FicheOnDisk*)inObject)->Retain();
}


void RecordTreeMemHeader::OccupyElem(void* inObject, OccupableStack* curstack)
{
	((FicheOnDisk*)inObject)->Occupy(curstack, true);
}


bool RecordTreeMemHeader::TryToFreeElem(sLONG allocationBlockNumber, void* InObject, VSize& outFreed)
{
	FicheOnDisk* ficD = (FicheOnDisk*)InObject;
	if (!ficD->IsOccupied() && !ficD->modifie() && OKAllocationNumber(ficD, allocationBlockNumber))
	{
		outFreed = ficD->ComputeSizeInMem();
		ficD->Release();
		return true;
	}
	else
		return false;
}


					// --------------------------------------------


void BlobTreeMemHeader::KillElem(void* inObject)
{
	((Blob4D*)inObject)->setmodif(false, ((Blob4D*)inObject)->GetDF()->GetDB(), nil);
	((Blob4D*)inObject)->Release();
}


void BlobTreeMemHeader::RetainElem(void* inObject)
{
	((Blob4D*)inObject)->Retain();
}


void BlobTreeMemHeader::OccupyElem(void* inObject, OccupableStack* curstack)
{
	((Blob4D*)inObject)->Occupy(curstack, true);
}


bool BlobTreeMemHeader::TryToFreeElem(sLONG allocationBlockNumber, void* InObject, VSize& outFreed)
{
	Blob4D* blob = (Blob4D*)InObject;
	if (blob->MatchAllocationNumber(allocationBlockNumber))
	{
		if (!blob->IsOccupied() && !blob->modifie())
		{
			outFreed = blob->calclen();
			blob->Release();
			return true;
		}
		else
			return false;
	}
	else
		return false;
}





													/* ************************************************************* */


DataTableRegular::~DataTableRegular()
{
	assert(NbLoadedRecords==0);

#if debugLeakCheck_NbLoadedRecords
	if (NbLoadedRecords > 0)
		DumpStackCrawls();
#endif

	if (fSeq != nil && fSeq != (AutoSeqNumber*)-1)
		fSeq->Release();

	/*
	if (fLockEvent != nil)
	{
		if (fWholeTableIsLockedBy != nil)
			fLockEvent->Unlock();
		fLockEvent->Release();
	}
	*/
}


DataTableRegular::DataTableRegular()
{
	//fLockEvent = nil;
	fBusyDeleting = false;
	fAllRecordsInited = false;
	//fFullyDeletedRecords.libere();
	//fFullyDeletedRecords.SetContientQuoi(t_fullydeletedrecinfo);
	fWholeTableIsLockedBy = nil;
	fLockCount = 0;
	fCountDataModif = 0;
	fTotalLocks = 0;
	NbLoadedRecords = 0;
	fSeq = nil;
	fRemoteMaxRecordsStamp = 1;
	fMustFullyDeleteForLibereEspaceDisk = false;
}

void oldDataFileDISK::SwapBytes()
{
	ByteSwap(&nbfic);
	ByteSwap(&newnum);
	ByteSwap(&debuttrou);
	ByteSwap(&addrtabaddr);
	ByteSwap(&segref);
	ByteSwap(&debutBlobTrou);
	ByteSwap(&addrBlobtabaddr);
	ByteSwap(&nbBlob);
	SeqNum_ID.SwapBytes();
}


void DataTableDISK::SwapBytes()
{
	ByteSwap(&nbfic);
	ByteSwap(&newnum);
	ByteSwap(&debuttrou);
	ByteSwap(&addrtabaddr);
	ByteSwap(&segref);
	ByteSwap(&debutBlobTrou);
	ByteSwap(&addrBlobtabaddr);
	ByteSwap(&nbBlob);
	SeqNum_ID.SwapBytes();
	TableDefID.SwapBytes();
	ByteSwap(&filler8);
	ByteSwap(&fLastRecordSync);
	ByteSwap(&debutTransRecTrou);
	ByteSwap(&debutTransBlobTrou);
	ByteSwap(&fLastRecordStamp);
}


DataTableRegular::DataTableRegular(Base4D *xdb, Table* xcrit, sLONG xnum, DataAddr4D ou, Boolean ForInit, DataTableDISK* dejaDFD)
	:DataTable(xdb, xcrit, xnum)
{
	VError err = VE_OK;
	// il est possible que xcrit soit nil
	// cela veut dire que l'on a charge un datafile avec une table detruite
	//fLockEvent = nil;
	fBusyDeleting = false;
	fAllRecordsInited = false;
	fMustFullyDeleteForLibereEspaceDisk = false;
	fRemoteMaxRecordsStamp = 1;
	setaddr(ou);
	if (dejaDFD != nil)
		DFD = *dejaDFD;
	else
	{
		if (!ForInit)
		{
			DataBaseObjectHeader tag;
			err = tag.ReadFrom(db, ou);
			if (err==VE_OK)
			{
				err = tag.ValidateTag(DBOH_DataTable, xnum, -3);
				if (err == VE_OK)
				{
					err = db->readlong(&DFD,sizeof(DFD),ou,kSizeDataBaseObjectHeader);
					if (err == VE_OK)
					{
						err = tag.ValidateCheckSum(&DFD, sizeof(DFD));
						if (err == VE_OK && tag.NeedSwap())
							DFD.SwapBytes();
					}
				}
			}
			if (err != VE_OK)
			{
				ForInit = true;
				fIsInvalid = true;
			}
		}

		if (ForInit)
		{
			DFD.filler8 = 0;
			DFD.fLastRecordSync = 0;
			std::fill(&DFD.SeqNum_ID.fBytes[0], &DFD.SeqNum_ID.fBytes[16], 0);
			DFD.nbfic=0;
			DFD.newnum=0;
			DFD.debuttrou=kFinDesTrou;
			DFD.addrtabaddr=0;
			DFD.segref=0;
			DFD.nbBlob=0;
			DFD.debutBlobTrou=kFinDesTrou;
			DFD.addrBlobtabaddr=0;
			DFD.debutTransRecTrou = 0;
			DFD.debutTransBlobTrou = 0;
			DFD.fKeepStamps = kDefaultKeepStamp;
			if (xdb->GetStructure() == nil)
				DFD.fKeepStamps = 0;
			DFD.fillerByte1 = 0;
			DFD.fillerByte2 = 0;
			DFD.fillerByte3 = 0;
			DFD.fLastRecordStamp = 0;
		}
	}

	if (DFD.debutTransRecTrou == 0)
		DFD.debutTransRecTrou = kFinDesTrou;

	if (DFD.debutTransBlobTrou == 0)
		DFD.debutTransBlobTrou = kFinDesTrou;

	VUUID xid;
	if (xcrit == nil)
	{
		xid.FromBuffer(DFD.TableDefID);
		xcrit = xdb->FindAndRetainTableRef(xid);
		if (xcrit == nil)
		{
			//xcrit = xdb->FindAndRetainTableRef_InDataPart(xid);
			if (xcrit != nil)
				fDefIsInDataPart = true;
		}
		else
			xcrit->Release();
	}
	else
	{
		xcrit->GetUUID(xid);
		xid.ToBuffer(DFD.TableDefID);
	}
	crit = xcrit;

	fTableDefNumInData = xdb->GetNumOfTableRefInData(xid);
	if (fTableDefNumInData != 0)
		xdb->AssociateTableRefInDataWithDataTable(fTableDefNumInData, xnum);

	if (crit != nil)
	{
		crit->setDF(this);
	}


	FicTabAddr.Init(xdb,this,&DFD.addrtabaddr,&DFD.debuttrou,&DFD.nbfic,DFD.segref, DFD.fKeepStamps == 1);
#if debugTabAddr
	if (crit != nil && crit->GetOwner()->GetStructure() != nil)
	{
		//FicTabAddr.SetDebugMess(true);
		FicTabAddr.SetDebugCheckOnDelete(true);
	}
#endif

	BlobTabAddr.Init(xdb,this,&DFD.addrBlobtabaddr,&DFD.debutBlobTrou,&DFD.nbBlob,DFD.segref, false);

	FicInMem.Init(DFD.nbfic);
	BlobInMem.Init(DFD.nbBlob);
	LockRec.Init(DFD.nbfic);
	nbrecord=-1;
	fWholeTableIsLockedBy = nil;
	fLockCount = 0;
	fCountDataModif = 0;
	fTotalLocks = 0;
	NbLoadedRecords = 0;
	fSeq = nil;

	/*
	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTALLOCATETABLEINMEM, DBaction_ConstructingTableInMem);
		*/
}


bool DataTableRegular::FreeMem(sLONG allocationBlockNumber, VSize combien, VSize& outSizeFreed)
{
	bool okfree = false;

	if (combien==0)
		combien=kMaxPositif;

	VSize tot = 0, tot2 = 0;
	FreeOutsideBlobCache(allocationBlockNumber, combien, tot2);
	tot = tot2;

#if FreeMemWaitOnLockWrite
	if (TryToLockFreeMem())
#endif
	{
		sLONG nb;
		
		bool okdel = true;

		//if (tot < combien)
		{
			LockRec.TryToFreeMem(allocationBlockNumber, combien-tot, tot2);
			tot = tot + tot2;
		}

		//if (tot < combien)
		{
			BlobInMem.TryToFreeMem(allocationBlockNumber, combien-tot, tot2);
			tot += tot2;
		}

		//if (tot < combien)
		{
			FicInMem.TryToFreeMem(allocationBlockNumber, combien-tot, tot2);
			tot += tot2;
		}

		//if (tot < combien)
		{
			BlobTabAddr.TryToFreeMem(allocationBlockNumber, combien-tot, tot2);
			tot += tot2;
		}

		//if (tot < combien)
		{
			FicTabAddr.TryToFreeMem(allocationBlockNumber, combien-tot, tot2);
			tot += tot2;
		}

		ClearMemRequest();
		outSizeFreed = tot;
		okfree = true;
#if FreeMemWaitOnLockWrite
		Unlock(true);
#endif
	}

	return okfree;

}


Blob4D* DataTableRegular::LoadBlobFromOutsideCache(const void* from, CreBlob_Code Code, VError& err, BaseTaskInfo* context)
{
	Blob4D* result = nil;
	err = VE_OK;
	VString path;
	sLONG* p = (sLONG*)from;
	bool isrelative = true;
	if (*p == -2 || *p == -3)
	{
		if (*p == -3)
			isrelative = false;
		p++;
		sLONG len = *p;
		p++;
		path.FromBlock(p, len * sizeof(UniChar), VTC_UTF_16);
	}
	if (!path.IsEmpty())
	{
		result = RetainOutsideBlobFromCache(path);
	}
	if (result == nil)
	{
		result = (*Code)(this);
		if (path.IsEmpty())
			path = "*";
		result->SetOutsidePath(path, isrelative);
		err = result->LoadDataFromPath();
		if (err == VE_OK)
			CacheOutsideBlob(result);
	}
	return result;
}



Blob4D* DataTableRegular::LoadBlob(sLONG n, CreBlob_Code Code, uBOOL formodif, VError& err, BaseTaskInfo* context)
{
	//CheckUseCount();

	Blob4D *theBlob = nil, *blobold;
	err = VE_OK;
	
	LockRead();
	OccupableStack* curstack = gOccPool.GetStackForCurrentThread();

	if (NotDeleted(err, DBaction_LoadingBlob))
	{
		Transaction* trans = GetCurrentTransaction(context);

		if (trans != nil)
		{
			theBlob = trans->GetBlob(GetNum(), n, err);
		}

		if (theBlob == nil && err == VE_OK)
		{
			theBlob=FindBlob(n, curstack);
			if (theBlob==nil) 
			{
				VTaskLock lock(&fLoadMutex);
				theBlob=FindBlob(n, curstack);
				if (theBlob == nil)
				{
					theBlob=(*Code)(this);

					if (theBlob == nil)
					{
						err = ThrowError(memfull, DBaction_LoadingBlob);
					}
					else
					{
						DataAddr4D ou = BlobTabAddr.GetxAddr(n, context, err, curstack);
						if (ou > 0)
						{
							theBlob->SetNum(n);
							//err = theBlob->loadobj(ou);
							theBlob = theBlob->loadobj(ou, err);
							if (err == VE_OK) 
							{
								StErrorContextInstaller errs(true);
								//theBlob->Retain();
								err = BlobInMem.PutIntoTreeMem(DFD.nbBlob,n,theBlob, curstack);
								if (err != VE_OK)
								{
									errs.Flush();
									err = VE_OK;
									theBlob->Release();
								}
							}
							/*
							else
							{
								theBlob->Release();
								theBlob = nil;
							}
							*/
						}
					}
				}
			} 
			
		}
		LibereDelete();
	}
	Unlock();
	
	//CheckUseCount();

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTLOADBLOB, DBaction_LoadingBlob);
	return theBlob;
}


VError DataTableRegular::SaveRawBlob(OccupableStack* curstack, DataBaseObjectType inType,sLONG numblob, void* rawdata, sLONG lendata, BaseTaskInfo* context)
{
	VError err = VE_OK;
	sLONG len = lendata + 4;
	DataAddr4D ou=db->findplace(len+kSizeDataBaseObjectHeader, context, err, 0, nil);
	if (ou != -1)
	{
		err = BlobTabAddr.PutxAddr(numblob, ou, len, context, curstack);
		if (err == VE_OK)
		{
			DataBaseObjectHeader tag(rawdata, lendata, inType, numblob, GetTrueNum());
			err = tag.WriteInto(db, ou, nil, -1);
			if (err==VE_OK)
			{
				err=db->writelong(&lendata, 4 , ou, kSizeDataBaseObjectHeader, nil, -1);
				err=db->writelong(rawdata, len , ou, kSizeDataBaseObjectHeader+4, nil, -1);
			}
		}
	}

	return err;
}


VError DataTableRegular::SaveBlobForTrans(Blob4D *inBlob, BaseTaskInfo* context)
{
	VError err = VE_OK;
	sLONG n,len;
	DataAddr4D ou;
	uBOOL newone;

	LockWrite();
	OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
	inBlob->Occupy(curstack, false);

	if (inBlob->IsOutsidePath())
	{
		inBlob->UnMarkDeleteOutsidePath();
		newone = inBlob->IsNewInTrans();
		inBlob->SetInProtectedArea();
		CacheOutsideBlob(inBlob);
		inBlob->setmodif(true, db, context);
		if (newone)
			inBlob->SetNewInTrans(false);
	}
	else if (NotDeleted(err))
	{
		n=inBlob->GetNum();
		newone = inBlob->IsNewInTrans();

		if (newone)
		{
			inBlob->Retain();
			err=BlobInMem.PutIntoTreeMem(DFD.nbBlob,n,inBlob,curstack);
			if (err == VE_OK)
			{
				//inBlob->SetDansCache(true);
				//inBlob->Retain();
			}
			else
				inBlob->Release();
		}
		else
		{
			Blob4D* oldblob = (Blob4D*)BlobInMem.GetFromTreeMem(n, curstack);
			if (oldblob == inBlob)
			{
				assert(false);
				//assert(inBlob->IsDansCache());
				//oldblob->libere();
			}
			else
			{
				BlobInMem.ReplaceIntoTreeMem(DFD.nbBlob,n,inBlob,curstack);
			}
		}

		if ((n>=0) && (err==VE_OK))
		{
			len=inBlob->calclen();
			if (newone)
			{
				err = BlobTabAddr.GiveBackAddrTemporary(n, context, &DFD.debutTransBlobTrou, curstack);

				ou=db->findplace(len+kSizeDataBaseObjectHeader, context, err, 0, inBlob);
				if (ou>0)
				{
#if debugOverlaps_strong
					db->Debug_CheckAddrOverlap(ou, len+kSizeDataBaseObjectHeader, GetTrueNum(), n, -1);
#endif
#if debug_Addr_Overlap
					db->SetDBObjRef(ou, len+kSizeDataBaseObjectHeader, new debug_BlobRef(crit->GetNum(), n), true);
#endif
					err=BlobTabAddr.PutxAddr(n,ou, len, context, curstack);
					inBlob->setaddr(ou);
				}
			}
			else
			{
				if (adjuste(len+kSizeDataBaseObjectHeader)==adjuste(inBlob->GetAntelen()+kSizeDataBaseObjectHeader))
				{
					if (len != inBlob->GetAntelen())
					{
#if debugOverlaps_strong
						db->Debug_CheckAddrOverlap(inBlob->getaddr(), len+kSizeDataBaseObjectHeader, GetTrueNum(), n, -1);
#endif
#if debug_Addr_Overlap
						db->ChangeDBObjRefLength(inBlob->getaddr(), len+kSizeDataBaseObjectHeader, debug_BlobRef(crit->GetNum(), n));
#endif
						
						err=BlobTabAddr.PutxAddr(n, inBlob->getaddr(), len, context, curstack);
					}
				}
				else
				{
					ou=db->findplace(len+kSizeDataBaseObjectHeader, context, err, 0, inBlob);
					if (ou>0)
					{
#if debugOverlaps_strong
						VError err2;
						DataAddr4D xoldaddr = BlobTabAddr.GetxAddr(n, context, err2);
						if (xoldaddr != inBlob->getaddr())
						{
							n = n; // break here;
							assert(xoldaddr == inBlob->getaddr());
						}

						db->Debug_CheckAddrOverlap(ou, len+kSizeDataBaseObjectHeader, GetTrueNum(), n, -1);
#endif
						DataAddr4D oldaddr = inBlob->getaddr();
						sLONG oldlen = inBlob->GetAntelen();
						if (err == VE_OK)
						{
#if debug_Addr_Overlap
							db->DelDBObjRef(oldaddr, oldlen+kSizeDataBaseObjectHeader);
							db->SetDBObjRef(ou, len+kSizeDataBaseObjectHeader, new debug_BlobRef(crit->GetNum(), n), true);
#endif
							err=BlobTabAddr.PutxAddr(n, ou, len, context, curstack);
							inBlob->ChangeAddr(ou, db, context);
						}

#if debugOverlaps_strong
						db->Debug_CheckAddrOverlap(oldaddr, oldlen+kSizeDataBaseObjectHeader, GetTrueNum(), n, -1);
#endif
						err = db->libereplace(oldaddr, oldlen+kSizeDataBaseObjectHeader, context, inBlob);
						if (err != VE_OK)
						{
							err = VE_OK;
							PullLastError();
						}
					}
				}
			}
			if (err==VE_OK)
			{
				inBlob->SetAntelen(len);
				inBlob->SetNum(n);
				//inBlob->SetInTrans(false);
				inBlob->setmodif(true, db, context);
				if (newone)
					inBlob->SetNewInTrans(false);
			}
		}

		LibereDelete();
	}

	inBlob->Free(curstack, false);
	CheckForMemRequest();
	Unlock();
	//CheckUseCount();


	return err;
}


VError DataTableRegular::SaveBlob(Blob4D *inBlob, BaseTaskInfo* context, bool forcePathIfEmpty)
{
	VError err = VE_OK;
	sLONG n,len;
	DataAddr4D ou;
	uBOOL newone;
	
	//CheckUseCount();

	LockWrite();
	OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
	inBlob->Occupy(curstack, false);

	if (NotDeleted(err, DBaction_SavingBlob))
	{
		Transaction* trans = GetCurrentTransaction(context);	

		if (trans != nil)
		{
			if (inBlob->IsEmpty() && !forcePathIfEmpty)
			{
				if (inBlob->SomethingToDelete())
				{
					err=DelBlob(inBlob, context);
				}
			}
			else if (inBlob->IsOutsidePath())
			{
				err = inBlob->CheckIfMustDeleteOldBlobID(context, trans);
				if (err == VE_OK)
				{
					newone = inBlob->GetComputedPath(false).IsEmpty();
					err = trans->SaveBlob(inBlob);
					if (err == VE_OK)
						inBlob->WriteSaveToLog(context, newone);
					if (newone)
						inBlob->SetNewInTrans(true);
				}
			}
			else
			{
				inBlob->CheckIfMustDeleteOldPath(context, trans);
				n=inBlob->GetNum();
				if (n<0)
				{
					newone=true;

					{
						n = inBlob->GetReservedBlobNumber();
						if (n < 0)
						{
							n = BlobTabAddr.findtrou(context, err, curstack);
							if (err == VE_OK)
							{
								if (n < 0)
								{
									if (DFD.nbBlob < kMaxBlobsInTable)
									{
										err=BlobInMem.PutIntoTreeMem(DFD.nbBlob+1, DFD.nbBlob, nil, curstack);
										if (err == VE_OK)
										{
											n = DFD.nbBlob;
										}
									}
									else
										err = ThrowError(VE_DB4D_MAXBLOBS_REACHED, DBaction_SavingBlob);
								}
							}
						}
						else
						{
							err = BlobTabAddr.TakeAddr(n, context, curstack);
							if (err == VE_OK)
							{
								err = BlobInMem.Aggrandit(max(DFD.nbBlob,n+1), curstack);
								if (err != VE_OK)
								{
									BlobTabAddr.liberetrou(n, context, curstack);
								}
							}
						}

						if (err == VE_OK)
						{
							err = BlobTabAddr.TakeAddrTemporary(n, context, &DFD.debutTransBlobTrou, curstack);
							//err=BlobTabAddr.PutxAddr(n, 0, 0, context);
							if (trans != nil)
							{
								trans->AddNewBlobNum(GetNum(), n);
								//MarkAddrAsNewForTrans(n, context);
							}
						}

					}

					if (n == -1)
					{
						err = ThrowError(memfull, DBaction_SavingBlob);
					}
					else
					{
						inBlob->SetNum(n);
						inBlob->SetNewInTrans(true);
					}
				}
				else
					newone = false;

				if (err == VE_OK)
				{
					err = trans->SaveBlob(inBlob);
					if (err == VE_OK)
						inBlob->WriteSaveToLog(context, newone);
				}
			}
		}
		else
		{
			
			if (inBlob->IsEmpty() && !forcePathIfEmpty)
			{
				if (inBlob->SomethingToDelete())
				{
					err=DelBlob(inBlob, context);
				}
			}
			else if (inBlob->IsOutsidePath())
			{
				err = inBlob->CheckIfMustDeleteOldBlobID(context, nil);
				if (err == VE_OK)
				{
					inBlob->UnMarkDeleteOutsidePath();
					newone = inBlob->GetComputedPath(false).IsEmpty();
					inBlob->_CreatePathIfEmpty();
					inBlob->SetInProtectedArea();
					CacheOutsideBlob(inBlob);
					inBlob->setmodif(true, db, context);
					inBlob->WriteSaveToLog(context, newone);	
				}
			}
			else
			{
				inBlob->CheckIfMustDeleteOldPath(context, nil);
				if (err==VE_OK)
				{
					n=inBlob->GetNum();
					if (n<0)
					{
						newone=true;
						{
							n = inBlob->GetReservedBlobNumber();
							if (n < 0)
							{
								n=BlobTabAddr.findtrou(context, err, curstack);
								if (err == VE_OK)
								{
									if (n<0)
									{
										inBlob->Retain();
										err=BlobInMem.PutIntoTreeMem(DFD.nbBlob+1,DFD.nbBlob, inBlob, curstack);
										if (err==VE_OK)
										{
											n=DFD.nbBlob;
											err=BlobTabAddr.PutxAddr(n,0, 0, context, curstack);
										}
										else
											inBlob->Release();

									}
									else
									{
										inBlob->Retain();
										err=BlobInMem.PutIntoTreeMem(DFD.nbBlob,n, inBlob, curstack);
										if (err==VE_OK) {
											//inBlob->SetDansCache(true);
											//inBlob->Retain();
										}
										else
											inBlob->Release();								
									}
								}
							}
							else
							{
								err = BlobTabAddr.TakeAddr(n, context, curstack);
								if (err == VE_OK)
								{
									err = BlobInMem.Aggrandit(max(DFD.nbBlob,n+1), curstack);
									if (err != VE_OK)
									{
										BlobTabAddr.liberetrou(n, context, curstack);
									}
									else
									{
										inBlob->Retain();
										err=BlobInMem.PutIntoTreeMem(DFD.nbBlob,n,(ObjCacheInTree*)inBlob, curstack);
										if (err==VE_OK) 
										{
											//inBlob->SetDansCache(true);
											//inBlob->Retain();
										}
										else
											inBlob->Release();
									}
								}
							}

						}
					}
					else
					{
						newone=false;
						BlobInMem.ReplaceIntoTreeMem(DFD.nbBlob,n,inBlob, curstack);
					}
					
					if ((n>=0) && (err==VE_OK))
					{
						len=inBlob->calclen();
						if (newone)
						{
							ou=db->findplace(len+kSizeDataBaseObjectHeader, context, err, 0, inBlob);
							if (ou>0)
							{
#if debugOverlaps_strong
								db->Debug_CheckAddrOverlap(ou, len+kSizeDataBaseObjectHeader, GetTrueNum(), n, -1);
#endif
#if debug_Addr_Overlap
								db->SetDBObjRef(ou, len+kSizeDataBaseObjectHeader, new debug_BlobRef(crit->GetNum(), n), true);
#endif
								err=BlobTabAddr.PutxAddr(n,ou, len, context, curstack);
								inBlob->setaddr(ou);
							}
						}
						else
						{
							if (adjuste(len+kSizeDataBaseObjectHeader)==adjuste(inBlob->GetAntelen()+kSizeDataBaseObjectHeader))
							{
								if (len != inBlob->GetAntelen())
								{
#if debugOverlaps_strong
									db->Debug_CheckAddrOverlap(inBlob->getaddr(), len+kSizeDataBaseObjectHeader, GetTrueNum(), n, -1);
#endif
#if debug_Addr_Overlap
									db->ChangeDBObjRefLength(inBlob->getaddr(), len+kSizeDataBaseObjectHeader, debug_BlobRef(crit->GetNum(), n));
#endif
									err=BlobTabAddr.PutxAddr(n, inBlob->getaddr(), len, context, curstack);
								}
							}
							else
							{
								ou=db->findplace(len+kSizeDataBaseObjectHeader, context, err, 0, inBlob);
								if (ou>0)
								{		
#if debugOverlaps_strong
									VError err2;
									DataAddr4D xoldaddr = BlobTabAddr.GetxAddr(n, context, err2);
									if (xoldaddr != inBlob->getaddr())
									{
										n = n; // break here;
										assert(xoldaddr == inBlob->getaddr());
									}
									db->Debug_CheckAddrOverlap(ou, len+kSizeDataBaseObjectHeader, GetTrueNum(), n, -1);
#endif
									DataAddr4D oldaddr = inBlob->getaddr();
									sLONG oldlen = inBlob->GetAntelen();
									if (err == VE_OK)
									{
#if debug_Addr_Overlap
										db->DelDBObjRef(oldaddr, oldlen+kSizeDataBaseObjectHeader);
										db->SetDBObjRef(ou, len+kSizeDataBaseObjectHeader, new debug_BlobRef(crit->GetNum(), n), true);
#endif
										err=BlobTabAddr.PutxAddr(n, ou, len, context, curstack);						
										inBlob->ChangeAddr(ou, db, context);
									}
#if debugOverlaps_strong
									db->Debug_CheckAddrOverlap(oldaddr, oldlen+kSizeDataBaseObjectHeader, GetTrueNum(), n, -1);
#endif
									err = db->libereplace(oldaddr,oldlen+kSizeDataBaseObjectHeader, context, inBlob);
									if (err != VE_OK)
									{
										err = VE_OK;
										PullLastError();
									}					
								}
							}
						}
						if (err==VE_OK)
						{
							inBlob->SetAntelen(len);
							inBlob->SetNum(n);
							inBlob->setmodif(true, db, context);
							inBlob->WriteSaveToLog(context, newone);
						}
					}
				}
			}
			
		}
		LibereDelete();
	}

	inBlob->Free(curstack, false);
	CheckForMemRequest();
	Unlock();
	//CheckUseCount();

	if (err != VE_OK)
		err = inBlob->ThrowError(VE_DB4D_CANNOTSAVEBLOB, DBaction_SavingBlob);
	return(err);
}


Boolean DataTableRegular::isRecordDeleted(sLONG recordnum)
{
	Boolean res = false;
	VError err;
	LockRead();
	OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
	if (NotDeleted(err))
	{
		if (FicTabAddr.GetxAddr(recordnum, nil, err, curstack) < 0)
			res = true;
		LibereDelete();
	}
	Unlock();
	return res;
}


VError DataTableRegular::SaveRawRecord(OccupableStack* curstack, sLONG numrec, void* rawdata, sLONG lendata, sLONG nbfields, BaseTaskInfo* context)
{
	VError err = VE_OK;
	sLONG len = lendata + (nbfields* sizeof(ChampHeader));
	DataAddr4D ou=db->findplace(len+kSizeRecordHeader, context, err, 0, nil);
	if (ou != -1)
	{
		err = FicTabAddr.PutxAddr(numrec, ou, len, context, curstack, 0, "SaveRawRecord");
		if (err == VE_OK)
		{
			RecordHeader tag(rawdata, lendata, DBOH_Record, numrec, GetTrueNum(), nbfields);
			err = tag.WriteInto(db, ou, nil, -1);
			if (err==VE_OK)
			{
				err=db->writelong(rawdata, len , ou, kSizeRecordHeader, nil, -1);
			}
		}
	}

	return err;
}


VError DataTableRegular::SaveRecordInTrans(FicheOnDisk *ficD, BaseTaskInfo* context)
{
	VError err = VE_OK;
	Boolean newone = false;
	sLONG n;

	//CheckUseCount();

	VTask::Yield();
	//crit->WaitToAlterRecord();
	///LockWrite();
	OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
	ficD->Occupy(curstack, false);

	if (NotDeleted(err))
	{
		db->DelayForFlush();
		StartDataModif(context);

		if (db->OkToUpdate(err))
		{
			LockWrite();
			db->RequestForInvalideHeader();
			crit->UpdateIndexKeyForTrans(ficD->getnumfic(), ficD, context);

			n = ficD->getnumfic();
			newone = ficD->IsNewInTrans();

			bool withStamp = DFD.fKeepStamps == 1;

			if (crit->HasSyncInfo())
			{
				VectorOfVValue oldkey(false), *oldkeyptr = nil;
				FicheInMem* fic = nil, *oldfic = nil;
				if (!newone)
				{
					VError err2;
					StErrorContextInstaller errs(false);
					oldfic = LoadRecord(n, err2, DB4D_Do_Not_Lock, context);
					if (oldfic != nil)
					{
						oldkeyptr = &oldkey;
						oldfic->GetOldPrimKeyValue(oldkey);
					}
				}			
				fic = new FicheInMem(context, crit, ficD);
				err = fic->SetSyncInfo(Sync_Update, context, oldkeyptr, newone);
				fic->Release();
				QuickReleaseRefCountable(oldfic);
			}

			if (!newone)
			{
				err = FicInMem.ReplaceIntoTreeMem(DFD.nbfic,n,(ObjCacheInTree*)ficD, curstack);
			}

			sLONG len;
			DataAddr4D ou;

			if (err==VE_OK)
			{
				{
					len=ficD->calclen();
					if (newone)
					{
						WriteLogAction(L"Before Allocate record Space", n);
						ou=db->findplace(len+kSizeRecordHeader, context, err, 0, ficD);
						WriteLogAction(L"Allocated record Space", ou);
						if (ou>0)
						{
#if debugOverlaps_strong
							db->Debug_CheckAddrOverlap(ou, len+kSizeRecordHeader, GetTrueNum(), -1, n);
#endif
							ficD->setaddr(ou);
							ficD->Retain();

							VError err2 = FicInMem.PutIntoTreeMem(DFD.nbfic, n, (ObjCacheInTree*)ficD, curstack);

							if (err2 != VE_OK)
								ficD->Release();

							if (err == VE_OK)
							{
#if debug_Addr_Overlap
								db->SetDBObjRef(ou, len+kSizeRecordHeader, new debug_FicheRef(crit->GetNum(), n), true);
#endif
								WriteLogAction(L"Before Putaddr", n);
								uLONG stamp = 0;
								if (withStamp)
									stamp = ficD->GetModificationStamp();

								err = FicTabAddr.GiveBackAddrTemporary(n, context, &DFD.debutTransRecTrou, curstack);
								err = FicTabAddr.PutxAddr(n, ou, len, context, curstack, stamp, "SaveRecordInTrans");
								UnMarkAddrAsNewForTrans(n, context);
								FicTabAddr.setmodif(true, db, context);
								setmodif(true, db, context);
								WriteLogAction(L"After Putaddr", n);
							}

							if (err != VE_OK)
							{
								ficD->setmodif(false, db, context);
								//ficD->SetDansCache(false);
								ficD->Release();
								if (n>=0)
									FicInMem.DelFromTreeMem(n, curstack);
#if debugOverlaps_strong
								db->Debug_CheckAddrOverlap(ou, len+kSizeRecordHeader, GetTrueNum(), -1, n);
#endif
#if debug_Addr_Overlap
								db->DelDBObjRef(ou, len+kSizeRecordHeader);
#endif
								db->libereplace(ou, len+kSizeRecordHeader, context, ficD);
							}
						}

						if (err != VE_OK)
						{
							if (n>=0)
							{
								FicTabAddr.GiveBackAddrTemporary(n, context, &DFD.debutTransRecTrou, curstack);
								VError err2 = FicTabAddr.liberetrou(n, context, curstack);
								if (err2 != VE_OK)
									err2 = PullLastError();
							}
						}
					}
					else
					{
						if (adjuste(len+kSizeRecordHeader)==adjuste(ficD->GetAntelen()+kSizeRecordHeader))
						{
							if (len != ficD->GetAntelen() || withStamp )
							{
								/*
								uLONG stamp = 0;
								if (withStamp)
								{
									VError err2;
									FicTabAddr.GetxAddr(n, context, err2, curstack, nil, &stamp);
									if (err2 == VE_OK)
										stamp++;
									if (stamp == 0)
										stamp = 1;
								}
#if debugOverlaps_strong
								db->Debug_CheckAddrOverlap(ficD->getaddr(), len+kSizeRecordHeader, GetTrueNum(), -1, n);
#endif
								*/
								uLONG stamp = 0;
								if (withStamp)
									stamp = ficD->GetModificationStamp();
#if debug_Addr_Overlap
								db->ChangeDBObjRefLength(ficD->getaddr(), len+kSizeRecordHeader, debug_FicheRef(crit->GetNum(), n));
#endif
								err=FicTabAddr.PutxAddr(n,ficD->getaddr(),len,context, curstack, stamp, "SaveRecordInTrans");
								ficD->SetModificationStamp(stamp);
							}
						}
						else
						{
							uLONG stamp = 0;
							if (withStamp)
								stamp = ficD->GetModificationStamp();

							ou=db->findplace(len+kSizeRecordHeader, context, err, 0, ficD);
							if (ou>0)
							{
								ficD->SetModificationStamp(stamp);

#if debugOverlaps_strong
								db->Debug_CheckAddrOverlap(ou, len+kSizeRecordHeader, GetTrueNum(), -1, n);
#endif
#if debug_Addr_Overlap
								db->DelDBObjRef(ficD->getaddr(), ficD->GetAntelen()+kSizeRecordHeader);
								db->SetDBObjRef(ou, len+kSizeRecordHeader, new debug_FicheRef(crit->GetNum(), n), true);
#endif
								{
#if debugOverlaps_strong
									db->Debug_CheckAddrOverlap(ficD->getaddr(),ficD->GetAntelen()+kSizeRecordHeader, GetTrueNum(), -1, n);
#endif
									StErrorContextInstaller errs(true);
									if (MustFullyDeleteRecords())
										db->MarkBlockToFullyDeleted(ficD->getaddr(),ficD->GetAntelen()+kSizeRecordHeader);
									err = db->libereplace(ficD->getaddr(),ficD->GetAntelen()+kSizeRecordHeader, context, ficD);
									if (err != VE_OK)
									{
										err = VE_OK;
										errs.Flush();
									}
								}
								ficD->ChangeAddr(ou, db, context);
								err=FicTabAddr.PutxAddr(n,ou,len,context, curstack, stamp, "SaveRecordInTrans");
							}
						}
					}
				}
			}

			if (err == VE_OK)
			{
				ficD->SetAntelen(len);

				//ficD->SetInTrans(false);
#if debuglr
				ficD->SetCommited(true);
#endif
				ficD->setmodif(true, db, context);

				if (newone) 
				{
					ficD->SetNewInTrans(false);
					{
						IncNbRecords(n);
						/*
						if (nbrecord != -1)
							nbrecord++;
							*/
					}
				}
			}

			db->EndOfRequestForInvalideHeader();

			CheckForMemRequest();
			Unlock();
			db->ClearUpdating();
		}

		EndDataModif(context);
		//CheckUseCount();
		LibereDelete();
	}


	ficD->Free(curstack, false);
	//CheckForMemRequest();
	//Unlock();
	//crit->FinishAlterRecord();


	return err;
}


sLONG DataTableRegular::ReserveNewRecAddr(BaseTaskInfo* context, VError& err)
{
	sLONG res = -1;
	err = VE_OK;

	LockWrite();
	OccupableStack* curstack = gOccPool.GetStackForCurrentThread();

	if (NotDeleted(err))
	{
		res = FicTabAddr.findtrou(context, err, curstack);
		if (err == VE_OK)
		{
			if (res < 0)
			{
				if (DFD.nbfic < kMaxRecordsInTable)
				{
					err = FicInMem.PutIntoTreeMem(DFD.nbfic+1, DFD.nbfic, nil, curstack);
					if (err == VE_OK)
					{
						fRemoteMaxRecordsStamp++;
						db->IncRemoteMaxRecordsStamp();
						res = DFD.nbfic;
					}
				}
				else
					err = VE_DB4D_MAXRECORDS_REACHED;
			}
		}

		if (err == VE_OK)
		{
			err = FicTabAddr.TakeAddrTemporary(res, context, &DFD.debutTransRecTrou, curstack);
		}
		LibereDelete();
	}
	CheckForMemRequest();
	Unlock();

	return res;
}


VError DataTableRegular::LibereAddrRec(OccupableStack* curstack, sLONG numrec, BaseTaskInfo* context)
{
	VError err = VE_OK;

	//CheckUseCount();

	// LockWrite(); // est fait par les appelants
	if (NotDeleted(err))
	{
		FicTabAddr.GiveBackAddrTemporary(numrec, context, &DFD.debutTransRecTrou, curstack);
		UnMarkAddrAsNewForTrans(numrec, context);
		err = FicTabAddr.liberetrou(numrec, context, curstack);
		setmodif(true, GetDB(), context);
		LibereDelete();
	}
	// Unlock();

	//CheckUseCount();

	return err;
}


VError DataTableRegular::LibereAddrBlob(OccupableStack* curstack, sLONG numblob, BaseTaskInfo* context)
{
	VError err = VE_OK;

	//CheckUseCount();

	// LockWrite(); // est fait par les appelants
#if debugblobs
	debug_delblobnum(numblob, -2);
#endif

	if (NotDeleted(err))
	{
		//UnMarkAddrAsNewForTrans(numblob, context);
		BlobTabAddr.GiveBackAddrTemporary(numblob, context, &DFD.debutTransBlobTrou, curstack);
		err = BlobTabAddr.liberetrou(numblob, context, curstack);
		LibereDelete();
	}

	// Unlock();

	//CheckUseCount();

	return err;
}


VError DataTableRegular::EnoughDiskSpaceForRecord(BaseTaskInfo* context, Transaction* trans, VSize recSize)
{
	VError err = VE_OK;
	VSize possibleSize = 0;

	if (trans != nil)
	{
		possibleSize = trans->PossibleUsedDiskSize();
	}

	possibleSize = possibleSize + recSize;

	if (!db->CanFindDiskSpace(possibleSize))
		err = db->ThrowError(VE_DB4D_NOT_ENOUGH_FREE_SPACE_ON_DISK, noaction);

	return err;
}


VError DataTableRegular::SaveRecord(FicheInMem *fic, BaseTaskInfo* context, uLONG stamp, bool allowOverrideStamp)
{
	VError err = VE_OK, errsub = VE_OK;
	sLONG n,len;
	DataAddr4D ou;
	uBOOL newone;
	FicheOnDisk *ficD, *ficDx;
	Boolean RecordNumberWasCreated = false;
	Boolean RecordHasBeenLocked = false;
	Boolean SaveSubRecLater = false;
	uLONG curstamp = 0xFFFFFFFF;

	//CheckUseCount();

#if trackModif
	trackDebugMsg("before save record\n");
#endif

	VTask::Yield();
	//crit->WaitToAlterRecord();
	//LockWrite();
	OccupableStack* curstack = gOccPool.GetStackForCurrentThread();

	if (NotDeleted(err))
	{
		StartDataModif(context);
#if trackModif
		trackDebugMsg("before save record, after StartDataModif\n");
#endif
		LockWrite();
#if trackModif
		trackDebugMsg("before save record, after LockWrite\n");
#endif

		Transaction *subtrans = nil;
		
		WriteLogAction(L"Enter Saverecord", fic->GetNum());

		if (stamp == 0)
		{
			if (fic->ReadOnlyState())
			{
				err = ThrowError(VE_DB4D_RECORDISLOCKED, DBaction_SavingRecord);
			}
		}
		else
		{
			if (!fic->IsNew())
			{
				sLONG recnum = fic->GetNum();
				if (fic->ReadOnlyState())
				{
					if (!LockRecord(recnum, context, DB4D_Keep_Lock_With_Record, fic->GetKindOfLockPtr(), fic->GetLockingContextExtraDataPtr()))
					{
						err = ThrowError(VE_DB4D_RECORDISLOCKED, DBaction_SavingRecord);
					}
					else
					{
						RecordHasBeenLocked = true;
						fic->SetReadOnlyState(false);
						if (context != nil)
							context->PutRecIn(fic);
					}
				}
				if (err == VE_OK)
				{
					sLONG curlen = 0;
					FicTabAddr.GetxAddr(recnum, context, err, curstack, &curlen, &curstamp);
					if (curstamp != stamp && curstamp != 0 && !allowOverrideStamp)
					{
						err = fic->ThrowError(VE_DB4D_RECORD_STAMP_NOT_MATCHING, DBaction_SavingRecord);
					}
				}
				if (err != VE_OK)
				{
					if (RecordHasBeenLocked)
					{
						RecordHasBeenLocked = false;
						if (context != nil)
							context->RemoveRecFrom(this, recnum);
						UnlockRecord(recnum, context, DB4D_Keep_Lock_With_Transaction);
						fic->SetReadOnlyState(true);
					}
				}
			}
		}

		if (err == VE_OK && db->OkToUpdate(err))
		{
			if (crit->AtLeastOneSubTable())
			{
				if (db->IsThereATrigger(DB4D_SaveNewRecord_Trigger, this) || db->IsThereATrigger(DB4D_SaveExistingRecord_Trigger, this) )
				{
					SaveSubRecLater = true;
					if (context != nil)
						subtrans = context->StartTransaction(err);
				}
				else
				{
					NumFieldArray subfields;
					crit->CopySubTableDep(subfields);
					sLONG i,nb = subfields.GetCount();
					for (i=1; i<=nb && errsub == VE_OK; i++)
					{
						sLONG nc = subfields[i];
						Field *cri = crit->RetainField(nc);
						assert(cri != nil);
						if (cri != nil)
						{
							if (testAssert(cri->GetTyp() == DB4D_SubTable))
							{
								ValPtr cv = fic->GetNthField(nc, errsub);
								if (testAssert(cv != nil))
								{
									assert(cv->GetTrueValueKind() == VK_SUBTABLE);
									VSubTableDB4D* sub = (VSubTableDB4D*)cv;
									errsub = sub->SaveSubRecords(cri, context, subtrans);
								}
							}
						}
					}
				}
			}

			Transaction *trans = GetCurrentTransaction(context);
			Boolean enTransaction = trans != nil;

			if (!enTransaction)
				db->DelayForFlush();


			if (fic->SetSaving())
			{
				ficD=fic->GetAssoc();

				bool withStamp = DFD.fKeepStamps == 1;

				// premiere partie du save, recherche d'un numero pour la fiche si celle ci est nouvelle

				n = fic->GetRecordNumberReserved();
				if (n != -1)
				{
					newone = true;
					if (!LockRecord(n, context, DB4D_Keep_Lock_With_Record, fic->GetKindOfLockPtr(), fic->GetLockingContextExtraDataPtr()))
					{
						err = ThrowError(VE_DB4D_RECORDISLOCKED, DBaction_SavingRecord);
					}
					else
					{
						RecordHasBeenLocked = true;
						if (enTransaction)
						{
							trans->AddNewRecNum(GetNum(), n);
							MarkAddrAsNewForTrans(n, context);
						}
						else
						{
							/*
							if (nbrecord != -1)
								nbrecord++;
								*/
							IncNbRecords(n);
							DFD.newnum++;
						}
						RecordNumberWasCreated = true;
						fic->ClearRecordNumberReserved();
						ficD->setnumfic(n);
						if (enTransaction)
							ficD->SetNewInTrans(true);
					}
				}
				else
				{
					n = ficD->getnumfic();
					if (n < 0)
					{
						newone = true;
						{
							n = fic->GetRecordNumberPreAllocated();
							if (n >= 0)
							{
								if (!fic->AddressAllreadyTaken())
									err = FicTabAddr.TakeAddr(n, context, curstack);
								if (err == VE_OK)
								{
									err = FicInMem.Aggrandit(max(DFD.nbfic,n+1), curstack);
									if (err == VE_OK)
									{
										err = FicTabAddr.TakeAddrTemporary(n, context, &DFD.debutTransRecTrou, curstack);
									}

									if (err != VE_OK)
									{
										FicTabAddr.liberetrou(n, context, curstack);
									}
								}

							}
							else
							{								
								n = FicTabAddr.findtrou(context, err, curstack);
								if (err == VE_OK)
								{
									//occupe();
									if (n < 0)
									{
										if (DFD.nbfic < kMaxRecordsInTable)
										{
											sLONG oldnbfic = DFD.nbfic;
											err=FicInMem.PutIntoTreeMem(DFD.nbfic+1, DFD.nbfic, nil, curstack);
											if (err == VE_OK)
											{
												n = DFD.nbfic;
												fRemoteMaxRecordsStamp++;
												db->IncRemoteMaxRecordsStamp();
											}
											assert(oldnbfic == DFD.nbfic);
										}
										else
											err = ThrowError(VE_DB4D_MAXRECORDS_REACHED, DBaction_SavingRecord);
									}

									if (err == VE_OK)
										FicTabAddr.TakeAddrTemporary(n, context, &DFD.debutTransRecTrou, curstack);

								}
							}

							if (err == VE_OK)
							{
								if (!LockRecord(n, context, DB4D_Keep_Lock_With_Record, fic->GetKindOfLockPtr(), fic->GetLockingContextExtraDataPtr()))
								{
									FicTabAddr.GiveBackAddrTemporary(n, context, &DFD.debutTransRecTrou, curstack);
									FicTabAddr.liberetrou(n, context, curstack);
									err = ThrowError(VE_DB4D_RECORDISLOCKED, DBaction_SavingRecord);
								}
								else
									RecordHasBeenLocked = true;
							}

							if (err == VE_OK)
							{
								RecordNumberWasCreated = true;
								if (!enTransaction)
								{
									/*
									if (nbrecord != -1)
										nbrecord++;
										*/
									IncNbRecords(n);
									DFD.newnum++;
								}

								if (enTransaction)
								{
									trans->AddNewRecNum(GetNum(), n);
									MarkAddrAsNewForTrans(n, context);
								}
							}

						}

						if (err == VE_OK)
						{
							ficD->setnumfic(n);
							if (enTransaction)
								ficD->SetNewInTrans(true);
						}

					}
					else
						newone = false;

				}



				//libere();

				// deuxieme partie du save, appel du trigger si il y en a
				if (err == VE_OK)
				{
					DB4D_TriggerType trig;
					if (newone)
						trig = DB4D_SaveNewRecord_Trigger;
					else
						trig = DB4D_SaveExistingRecord_Trigger;

					if (fic->MustCheckIntegrity() && db->IsThereATrigger(trig, this))
					{
						Unlock();

						fic->Retain();
						CDB4DRecord* encapsule = new VDB4DRecord(VDBMgr::GetManager(), fic, context->GetEncapsuleur());
						err = db->GetTriggerManager()->CallTrigger(trig, encapsule, context->GetEncapsuleur());
						encapsule->Release();

						LockWrite();
					}

					if (SaveSubRecLater)
					{
						if (crit->AtLeastOneSubTable())
						{
							NumFieldArray subfields;
							crit->CopySubTableDep(subfields);
							sLONG i,nb = subfields.GetCount();
							for (i=1; i<=nb && errsub == VE_OK; i++)
							{
								sLONG nc = subfields[i];
								Field *cri = crit->RetainField(nc);
								assert(cri != nil);
								if (cri != nil)
								{
									if (testAssert(cri->GetTyp() == DB4D_SubTable))
									{
										ValPtr cv = fic->GetNthField(nc, errsub);
										if (testAssert(cv != nil))
										{
											assert(cv->GetTrueValueKind() == VK_SUBTABLE);
											VSubTableDB4D* sub = (VSubTableDB4D*)cv;
											errsub = sub->SaveSubRecords(cri, context, subtrans);
										}
									}
								}
							}
						}
					}

					if (err == VE_OK && !newone && fic->MustCheckIntegrity()) // puis verification de l'integrite referentielle sur une fiche modifiée
					{
						err = crit->CheckReferentialIntegrity(fic, context, true);
					}

					if (err == VE_OK && fic->MustCheckIntegrity())  // puis verification de l'integrite referentielle pour les foreign keys
						err = crit->CheckReferentialIntegrityForForeignKey(fic, context, !newone);

					VSize recSize = fic->PossibleUsedDiskSize();
					if (err == VE_OK) // puis verification de l'espace disque
						err = EnoughDiskSpaceForRecord(context, trans, recSize);

					// troisieme partie du save, mise a jour des index et blob relatifs a cette fiche
					// puis reajustement des datas pour tenir en un seul block

					if (err == VE_OK)
					{						
						if (!enTransaction)
							db->RequestForInvalideHeader();

						uBOOL isnew = fic->IsNew();
						VectorOfVValue oldkey(false);
						if (crit->HasSyncInfo())
							fic->GetOldPrimKeyValue(oldkey);

						crit->SetRecNumHintForBlobs(fic, context, n);
						err = fic->stockcrit(context);

						/* L.R le 22/8/2011 je dois deplacer le ReajusteData a l'interieur de stockcrit avant la mise a jour des index
							car ReajusteData peut echouer sur un probleme de memoire et dans ce cas les cles d'index ont ete mises a jour 
							alors que la fiche ne sera pas stockee
						if (err == VE_OK)
							err = fic->ReajusteData(context);  // penser a forcer le changement de ficD si Transaction && ficD est dans cache
							*/

						if (err == VE_OK)
							err = crit->UpdateBlobs(fic, context);
						if (err == VE_OK && !enTransaction)
							err = fic->SetSyncInfo(Sync_Update, context, &oldkey, isnew);

						ficD=fic->GetAssoc();

						if (ficD != nil)
							ficD->Occupy(curstack, false);

						// quatrieme partie du save, recherche de l'espace pour stoquer les datas de la fiche

						if (err==VE_OK)
						{
							fic->GetAutoSeqValue();
							if (enTransaction)
							{
								//if (trans->EnoughDiskSpaceForRecord(ficD, err))
								{
									err = trans->SaveRecord(ficD, recSize);
								}
							}
							else
							{
								if (!newone)
								{
									FicInMem.ReplaceIntoTreeMem(DFD.nbfic,n,ficD,curstack);
								}
								
								if (err==VE_OK)
								{
									{
										len=ficD->calclen();
										if (newone)
										{
											WriteLogAction(L"Before Allocate record Space", n);
											ou=db->findplace(len+kSizeRecordHeader, context, err, 0, ficD);
											WriteLogAction(L"Allocated record Space", ou);
											if (ou>0)
											{
#if debugOverlaps_strong
												db->Debug_CheckAddrOverlap(ou, len+kSizeRecordHeader, GetTrueNum(), -1, n);
#endif
#if debug_Addr_Overlap
												db->SetDBObjRef(ou, len+kSizeRecordHeader, new debug_FicheRef(crit->GetNum(), n), true);
#endif
												ficD->setaddr(ou);
												ficD->Retain();

												VError err2 = FicInMem.PutIntoTreeMem(DFD.nbfic, n, (ObjCacheInTree*)ficD, curstack);
												if (err2 != VE_OK)
													ficD->Release();

												if (err == VE_OK)
												{
													WriteLogAction(L"Before Putaddr", n);
													err = FicTabAddr.GiveBackAddrTemporary(n, context, &DFD.debutTransRecTrou, curstack);

													uLONG stamp = 0;
													/*
													if (withStamp)
													{
														VError err2;
														FicTabAddr.GetxAddr(n, context, err2, curstack, nil, &stamp);
														if (err2 == VE_OK)
															stamp++;
														if (stamp == 0)
															stamp = 1;
													}
													*/
													if (withStamp)
													{
														if (curstamp == 0xFFFFFFFF)
														{
															VError err2;
															FicTabAddr.GetxAddr(n, context, err2, curstack, nil, &stamp);
															stamp++;
														}
														else
															stamp = curstamp + 1;
													}

													err = FicTabAddr.PutxAddr(n, ou, len, context, curstack, stamp, "SaveRecord");
													ficD->SetModificationStamp(stamp);
													fic->_SetModificationStamp(stamp);

													FicTabAddr.setmodif(true, db, context);
													setmodif(true, db, context);
													WriteLogAction(L"After Putaddr", n);
												}

												if (err != VE_OK)
												{
													ficD->setmodif(false, db, context);
													//ficD->SetDansCache(false);
													ficD->Release();
													if (n>=0)
														FicInMem.DelFromTreeMem(n, curstack);
#if debug_Addr_Overlap
													db->DelDBObjRef(ou, len+kSizeRecordHeader);
#endif
													db->libereplace(ou, len+kSizeRecordHeader, context, ficD);
												}
											}
											
											if (err != VE_OK)
											{
												if (n>=0)
												{
													FicTabAddr.GiveBackAddrTemporary(n, context, &DFD.debutTransRecTrou, curstack);
													VError err2 = FicTabAddr.liberetrou(n, context, curstack);
													if (err2 != VE_OK)
														err2 = PullLastError();
												}
											}
										}
										else
										{
											if (adjuste(len+kSizeRecordHeader)==adjuste(ficD->GetAntelen()+kSizeRecordHeader))
											{
												if (len != ficD->GetAntelen() || withStamp)
												{
													uLONG stamp = 0;
#if debugOverlaps_strong
													db->Debug_CheckAddrOverlap(ficD->getaddr(), len+kSizeRecordHeader, GetTrueNum(), -1, n);
#endif
													if (withStamp)
													{
														if (curstamp == 0xFFFFFFFF)
														{
															VError err2;
															FicTabAddr.GetxAddr(n, context, err2, curstack, nil, &stamp);
															stamp++;
														}
														else
															stamp = curstamp + 1;
													}
#if debug_Addr_Overlap
													db->ChangeDBObjRefLength(ficD->getaddr(), len+kSizeRecordHeader, debug_FicheRef(crit->GetNum(), n));
#endif
													err=FicTabAddr.PutxAddr(n,ficD->getaddr(),len,context, curstack, stamp, "SaveRecord");
													ficD->SetModificationStamp(stamp);
													fic->_SetModificationStamp(stamp);
												}
											}
											else
											{
												uLONG stamp = 0;
												if (withStamp)
												{
													if (curstamp == 0xFFFFFFFF)
													{
														VError err2;
														FicTabAddr.GetxAddr(n, context, err2, curstack, nil, &stamp);
														stamp++;
													}
													else
														stamp = curstamp + 1;
												}

												ou=db->findplace(len+kSizeRecordHeader, context, err, 0, ficD);
												if (ou>0)
												{
													ficD->SetModificationStamp(stamp);
													fic->_SetModificationStamp(stamp);
#if debugOverlaps_strong
													db->Debug_CheckAddrOverlap(ou, len+kSizeRecordHeader, GetTrueNum(), -1, n);
#endif
													DataAddr4D oldaddr = ficD->getaddr();
													sLONG oldlen = ficD->GetAntelen();
#if debug_Addr_Overlap
													db->DelDBObjRef(ficD->getaddr(), ficD->GetAntelen()+kSizeRecordHeader);
													db->SetDBObjRef(ou, len+kSizeRecordHeader, new debug_FicheRef(crit->GetNum(), n), true);
#endif
													ficD->ChangeAddr(ou, db, context);
													err=FicTabAddr.PutxAddr(n, ou, len, context, curstack, stamp, "SaveRecord");
#if debugOverlaps_strong
													db->Debug_CheckAddrOverlap(oldaddr, oldlen+kSizeRecordHeader, GetTrueNum(), -1, n);
#endif
													{
														StErrorContextInstaller errs(true);
														if (MustFullyDeleteRecords())
															db->MarkBlockToFullyDeleted(oldaddr,oldlen+kSizeRecordHeader);
														err = db->libereplace(oldaddr,oldlen+kSizeRecordHeader, context, ficD);
														if (err != VE_OK)
														{
															err = VE_OK;
															errs.Flush();
														}
													}
												}
											}
										}
									}
								}
							}
						}

						// cinquieme partie du save, finalisation, place une entree dans le log et lock la fiche si elle nouvelle

						if (err==VE_OK)
						{
							if (newone)
								fic->ValidateAutoSeqToken(context);

							if (!enTransaction)
								ficD->SetAntelen(len);
							//ficD->setnumfic(n);

							ficD->WriteSaveToLog(context, newone);

							if (!enTransaction)
								ficD->setmodif(true, db, context);
							if (newone) 
							{
								//LockRecord(n, context, false);
								if (context != nil)
								{
									context->PutRecIn(fic);
								}
							}

						}

						if (!enTransaction)
							db->EndOfRequestForInvalideHeader();

						if (ficD != nil)
							ficD->Free(curstack, false);

						/* maintenant cela est fait plus haut
						if ((err==VE_OK) && (!enTransaction) && (newone) && (nbrecord!=-1)) 
							nbrecord++;
						*/

					}
				}

				if (err != VE_OK)
				{
					if (RecordNumberWasCreated)
					{
						FicTabAddr.GiveBackAddrTemporary(n, context, &DFD.debutTransRecTrou, curstack);
						FicTabAddr.liberetrou(n, context, curstack);
						if (ficD != nil)
							ficD->setnumfic(-3);
						if (fic != nil)
							fic->SetBackToNew();
						if (!enTransaction)
						{
							/*
							if (nbrecord != -1)
								nbrecord--;
								*/
							DecNbRecords(n);
						}

						if (enTransaction)
						{
							if (ficD != nil)
								ficD->SetNewInTrans(false);
							trans->DelNewRecNum(GetNum(), n);
							UnMarkAddrAsNewForTrans(n, context);
						}
					}
					if (RecordHasBeenLocked)
						UnlockRecord(n, context);

				}
					
				fic->ClearSaving();

				if (err == VE_OK)
					err = errsub;

			}

			if (subtrans != nil)
			{
				assert(context != nil);
				if (err != VE_OK)
				{
					context->RollBackTransaction();
				}
				else
				{
					err = context->CommitTransaction();
				}
			}

			db->ClearUpdating();
		}

		if (stamp != 0 && RecordHasBeenLocked)
		{
			fic->UnLockRecord();
			fic->SetReadOnlyState(true);
		}

		CheckForMemRequest();
		Unlock();
		EndDataModif(context);
		LibereDelete();
	}

#if trackModif
	trackDebugMsg("after save record, n = "+ToString(n)+"\n");
#endif

	//CheckForMemRequest();
	//Unlock();
	//crit->FinishAlterRecord();

	if (err != VE_OK)
	{
		if (fic->IsNew())
			err = fic->ThrowError(VE_DB4D_CANNOTSAVENEWRECORD, DBaction_SavingRecord);
		else
			err = fic->ThrowError(VE_DB4D_CANNOTSAVERECORD, DBaction_SavingRecord);
	}


	WriteLogAction(L"Exit Saverecord", fic->GetNum());

#if debugLeakCheck_TreeMem
	if (debug_candumpleaks)
	{
		void* p = GetFastMem(2000000000, false, 0);
		DumpStackCrawls();
	}
#endif

	//CheckUseCount();

	return(err);
}


FicheInMem* DataTableRegular::NewRecord(VError& err, BaseTaskInfo* Context)
{
	FicheInMem* fic = nil;
	
	// occupe();
	//CheckUseCount();

	if (NotDeleted(err))
	{
		fic=new FicheInMem(Context, db,crit, err);
		if (fic==nil)
		{
			err = ThrowError(memfull, DBaction_AllocatingRecordInMem);
		}
		else
		{
			if (err == VE_OK)
			{
				if (crit->AtLeastOneSubTable())
				{
					NumFieldArray subfields;
					crit->CopySubTableDep(subfields);
					sLONG i,nb = subfields.GetCount();
					for (i=1; i<=nb && err == VE_OK; i++)
					{
						sLONG nc = subfields[i];
						Field *cri = crit->RetainField(nc);
						assert(cri != nil);
						if (cri != nil)
						{
							if (testAssert(cri->GetTyp() == DB4D_SubTable))
							{
								ValPtr cv = fic->GetNthField(nc, err);
								if (testAssert(cv != nil))
								{
									assert(cv->GetTrueValueKind() == VK_SUBTABLE);
									VSubTableDB4D* sub = (VSubTableDB4D*)cv;
									Boolean SomeLocked = false;
									err = sub->InitForNewRecord(fic, cri, Context);
								}
							}
							cri->Release();
						}
					}
				}
			}

			if (err!=VE_OK)
			{
				fic->Release();
				fic=nil;
			}
		}
		LibereDelete();
	}
	
	//CheckUseCount();
	// libere();
	return(fic);
}

#if 0
sLONG DataTableRegular::GetAutoSequence(void)
{
	sLONG lll;
	
	//CheckUseCount();

	occupe();
	lll = DFD.newnum;
	DFD.newnum++;
	setmodif(true, db, nil);
	libere();
	
	//CheckUseCount();

	return(lll);
}
#endif

Blob4D* DataTableRegular::FindBlob(sLONG numblob, OccupableStack* curstack)
{
	Blob4D *theBlob=(Blob4D*)(BlobInMem.RetainFromTreeMem(numblob, curstack));
	
	return theBlob;
}
	

DataAddr4D DataTableRegular::GetRecordPos(sLONG numrec, sLONG& outOldLen, VError& err)
{
	err = VE_OK;
	DataAddr4D ou = 0;
	outOldLen = -1;

	LockRead();
	OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
	if (NotDeleted(err))
	{
		ou=FicTabAddr.GetxAddr(numrec, nil, err, curstack, &outOldLen);
		LibereDelete();
	}
	Unlock();

	return ou;

}



FicheInMem* DataTableRegular::LoadRecord(sLONG n, VError& err, DB4D_Way_of_Locking HowToLock, BaseTaskInfo* Context, Boolean WithSubRecords, 
										Boolean BitLockOnly, ReadAheadBuffer* buffer, Boolean* outEnoughMem)
{
	FicheInMem* fic = nil;
	FicheOnDisk *ficD;
	FicheOnDisk *ficDx;
	DataAddr4D ou;
	uBOOL locked;
	StAllocateInCache	alloc;
	Boolean wasnotlocked = true;
	Transaction *trans = GetCurrentTransaction(Context);
	Boolean comesfromtrans = false;
	Boolean enoughmem = true;
	Boolean DoNotLoadSubRecs = false;

	DB4D_KindOfLock xKindOfLock = DB4D_LockedByNone;
	const VValueBag* xLockingContextExtraData = nil;

	
	//CheckUseCount();

	err = VE_OK;
	VTask::Yield();
	LockRead();
	OccupableStack* curstack = gOccPool.GetStackForCurrentThread();


	if (NotDeleted(err))
	{
		if ( trans==nil || !trans->IsRecordDeleted(GetNum(), n)) 
		{
			if (n>=0 && n<DFD.nbfic)
			{
				// ClearError();
				
				fic=nil;
				ficD=nil;
						
				if (HowToLock != DB4D_Do_Not_Lock)
				{
					if (Context != nil)
					{
						fic = Context->RetainRecAlreadyIn(this,n);
						if (fic != nil && fic->MustClearIfReloadFromContext())
						{
							fic->ClearModifiedFields();
						}
						DoNotLoadSubRecs = true;
						locked = false;
					}
					if (fic == nil)
						locked = ! LockRecord(n, Context, HowToLock, &xKindOfLock, &xLockingContextExtraData);
				}
				else locked = true;
				
				if (fic == nil)
				{

					if (trans != nil)
					{
						ficD = trans->RetainRecord(GetNum(), n, err);
						if (ficD != nil)
						{
							wasnotlocked = false;
							comesfromtrans = true;
						}
					}
					
					if (ficD == nil && err == VE_OK)
					{
						ficD=(FicheOnDisk*)(FicInMem.RetainFromTreeMem(n, curstack));
					
						if (ficD==nil)
						{
							VTaskLock lock(&fLoadMutex);
							ficD=(FicheOnDisk*)(FicInMem.RetainFromTreeMem(n, curstack));
							if (ficD == nil)
							{
								uLONG stamp = 0;
								ou=FicTabAddr.GetxAddr(n, Context, err, curstack, nil, &stamp);
								if (err == memfull)
									enoughmem = false;
								if ((err==VE_OK) && (ou>0))
								{
									ficD = FicheOnDisk::BuildRecordFromLoad(err, this, n, ou, buffer);
									if (err == memfull)
										enoughmem = false;
									if (err != VE_OK)
										err = ThrowError(err, DBaction_LoadingRecord);
									if (ficD != nil)
									{
										if (err != VE_OK) 
										{
											ficD->Release();
											ficD=nil;
										}
										else
										{
											StErrorContextInstaller errs(true);
											ficD->SetModificationStamp(stamp);
											// ficD->occupe();
											ficD->Retain();
											err=FicInMem.PutIntoTreeMem(DFD.nbfic,n,(ObjCacheInTree*)ficD, curstack);
											if (err != VE_OK)
												ficD->Release();
												//ficD->SetDansCache(true);
											else
											{
												// si on ne peut pas la mettre dans le cache, ce n'est pas grave
												errs.Flush();
												err=VE_OK;
											}
										}
									}
								}
							}
						}


					}

					if ((err==VE_OK) && (ficD!=nil))
					{
						fic=new FicheInMem(Context, crit,ficD);
						if (fic==nil)
						{
							enoughmem = false;
							err=ThrowError(memfull, DBaction_LoadingRecord);
						}
						else
						{
							fic->SetReadOnlyState(HowToLock == DB4D_Do_Not_Lock || locked);
							fic->SetWhoLocked(xKindOfLock, xLockingContextExtraData);
						}
					}
					
					if (ficD!=nil) 
					{
						ficD->Release();
					}
					
					if ( (fic == nil) && wasnotlocked && (!locked) && !BitLockOnly)
					{
						UnlockRecord(n, Context);
					}
				
					if ((fic != nil) && (!locked) && (Context != nil) )
					{
						Context->PutRecIn(fic);
					}

				}

				if (err == VE_OK && fic != nil)
				{
					if (WithSubRecords && crit->AtLeastOneSubTable())
					{
						Boolean mustreload = false;

						NumFieldArray subfields;
						crit->CopySubTableDep(subfields);
						sLONG i,nb = subfields.GetCount();
						for (i=1; i<=nb && err == VE_OK && !mustreload; i++)
						{
							sLONG nc = subfields[i];
							Field *cri = crit->RetainField(nc);
							assert(cri != nil);
							if (cri != nil)
							{
								if (testAssert(cri->GetTyp() == DB4D_SubTable))
								{
									ValPtr cv = fic->GetNthField(nc, err);
									if (testAssert(cv != nil))
									{
										assert(cv->GetTrueValueKind() == VK_SUBTABLE);
										VSubTableDB4D* sub = (VSubTableDB4D*)cv;
										Boolean SomeLocked = false;
										if (DoNotLoadSubRecs && sub->HasBeenLoaded())
										{
											err = sub->AllSubRecords(Context);
										}
										else
											err = sub->LoadSubRecords(fic, cri, locked ? DB4D_Do_Not_Lock : HowToLock, Context, SomeLocked, outEnoughMem);
										if (err == VE_OK)
										{
											if (SomeLocked)
											{
												mustreload = true;
											}
										}
									}
								}
							}
						}

						if (err == VE_OK && mustreload)
						{
							fic->Release();
							fic = LoadRecord(n, err, DB4D_Do_Not_Lock, Context, true, false, buffer, outEnoughMem);
						}
					}
				}

			}
			/*
			else
				err = ThrowError(VE_DB4D_WRONGRECORDID, DBaction_LoadingRecord);
				*/
		}
		LibereDelete();
	}

	Unlock();

	//CheckUseCount();

	ReleaseRefCountable(&xLockingContextExtraData);

	if (err != VE_OK)
	{
		if (VTaskMgr::Get()->GetCurrentTask()->FailedForMemory())
			enoughmem = false;
		err = ThrowError(VE_DB4D_CANNOTLOADRECORD, DBaction_LoadingRecord);
	}

	if (outEnoughMem != nil)
		*outEnoughMem = enoughmem;
	return(fic);
}




FicheOnDisk* DataTableRegular::LoadNotFullRecord(sLONG n, VError& err, DB4D_Way_of_Locking HowToLock, BaseTaskInfo* Context, Boolean BitLockOnly, 
												ReadAheadBuffer* buffer, Boolean* CouldLock, Boolean* outEnoughMem)
{
	FicheOnDisk *ficD = nil;
	FicheOnDisk *ficDx;
	DataAddr4D ou;
	uBOOL locked;
	StAllocateInCache	alloc;
	Boolean wasnotlocked = true;
	Transaction *trans = GetCurrentTransaction(Context);
	Boolean comesfromtrans = false;
	Boolean enoughmem = true;

	if (CouldLock != nil)
		*CouldLock = false;

	//CheckUseCount();

	VTask::Yield();
	err = VE_OK;
	LockRead();
	OccupableStack* curstack = gOccPool.GetStackForCurrentThread();

	if (NotDeleted(err))
	{
		if ( trans==nil || !trans->IsRecordDeleted(GetNum(), n)) 
		{
			if (n>=0)
			{
				// ClearError();
				if (HowToLock != DB4D_Do_Not_Lock)
				{
					FicheInMem* fic;
					if (Context != nil)
					{
						fic = Context->RetainRecAlreadyIn(this,n);
					}
					if (fic == nil)
						locked = ! LockRecord(n, Context, HowToLock);
					else
					{
						ficD = fic->GetAssoc();
						ficD->Retain();
						locked = false;
						if (CouldLock != nil)
							*CouldLock = true;
					}
				}
				else locked = true;

				if (ficD == nil)
				{

					if (trans != nil)
					{
						ficD = trans->RetainRecord(GetNum(), n, err);
						if (ficD != nil)
						{
							if (CouldLock != nil)
								*CouldLock = true;
							wasnotlocked = false;
							//ficD->occupe();  // fait dans le getrecord
							comesfromtrans = true;
						}
					}
				}

				if (ficD == nil && err == VE_OK)
				{
					ficD=(FicheOnDisk*)(FicInMem.RetainFromTreeMem(n, curstack));

					if (ficD==nil)
					{
						VTaskLock lock(&fLoadMutex);
						ficD=(FicheOnDisk*)(FicInMem.RetainFromTreeMem(n, curstack));
						if (ficD == nil)
						{
							uLONG stamp = 0;
							ou=FicTabAddr.GetxAddr(n, Context, err, curstack, nil, &stamp);
							if (err == memfull)
								enoughmem = false;
							if ((err==VE_OK) && (ou>0))
							{
								ficD = FicheOnDisk::BuildRecordFromLoad(err, this, n, ou, buffer);
								if (err == memfull)
									enoughmem = false;
								if (err != VE_OK)
									err = ThrowError(err, DBaction_LoadingRecord);
								if (ficD != nil)
								{
									if (err != VE_OK) 
									{
										ficD->Release();
										ficD=nil;
									}
									else
									{
										ficD->SetModificationStamp(stamp);
										ficD->Retain();
										err=FicInMem.PutIntoTreeMem(DFD.nbfic,n,(ObjCacheInTree*)ficD, curstack);
										
										if (err != VE_OK)
											ficD->Release();
											
										else
										{
											// si on ne peut pas la mettre dans le cache, ce n'est pas grave
											err = PullLastError();
											err=VE_OK;
										}
										if (HowToLock == DB4D_Do_Not_Lock || locked)
										{
											if (CouldLock != nil)
												*CouldLock = false;
										}
										else
										{
											{
												if (CouldLock != nil)
													*CouldLock = true;
											}

										}
									}
								}
							}
						}
					}
					else
					{
						if (HowToLock == DB4D_Do_Not_Lock || locked) 
						{
							if (CouldLock != nil)
								*CouldLock = false;
						}
						else 
						{
							{
								if (CouldLock != nil)
									*CouldLock = true;
							}

						}

					}

				}

				if ( (ficD == nil) && wasnotlocked && (!locked))
				{
					UnlockRecord(n, Context, HowToLock);
				}

			}
		}
		LibereDelete();
	}

	Unlock();

	//CheckUseCount();

	if (err != VE_OK)
	{
		if (VTaskMgr::Get()->GetCurrentTask()->FailedForMemory())
		err = ThrowError(VE_DB4D_CANNOTLOADRECORD, DBaction_LoadingRecord);
	}
	if (outEnoughMem != nil)
		*outEnoughMem = enoughmem;
	return(ficD);
}



bool DataTableRegular::SaveObj(VSize& outSizeSaved)
{
#if debuglogwrite
	VString wherefrom(L"DataTable SaveObj");
	VString* whx = &wherefrom;
#else
	VString* whx = nil;
#endif

	outSizeSaved = 0;
	VError err = VE_OK;

	outSizeSaved=sizeof(DataTableDISK) + kSizeDataBaseObjectHeader;
	DataBaseObjectHeader tag(&DFD, sizeof(DataTableDISK), DBOH_DataTable, fRealNum, -3);
	err = tag.WriteInto(db, getaddr(), whx);
	err = db->writelong(&DFD, sizeof(DataTableDISK), getaddr(),kSizeDataBaseObjectHeader, whx);
	if (err != VE_OK)
	{
		err = ThrowError(VE_DB4D_CANNOTSAVETABLEHEADER, DBaction_SavingTableHeader);
	}

	return true;
}


sLONG DataTableRegular::GetNbRecords(BaseTaskInfo* context, bool lockread)
{
	DataAddr4D ou;
	VError err = VE_OK;
	sLONG result = 0;
	
	//CheckUseCount();

	if (lockread)
	{
		LockRead();
		OccupableStack* curstack = gOccPool.GetStackForCurrentThread();

		if (NotDeleted(err))
		{
			if (nbrecord==-1)
			{
				Bittab b;
				nbrecord=DFD.nbfic;
				ou=DFD.debuttrou;
				while (ou!=kFinDesTrou)
				{
					ou = -ou;
					nbrecord--;
					if (ou<0 || b.isOn(ou))
						err = ThrowError(VE_DB4D_CIRCULAR_REF_IN_REC_ADDR_TABLE, noaction);
					else
					{
						b.Set(ou);
						ou=FicTabAddr.GetxAddr(ou, context, err, curstack);
					}
					if (err != VE_OK)
						break;
				}
			}
			assert(err == VE_OK);
			result = nbrecord;

			Transaction* trans = GetCurrentTransaction(context);
			if (trans != nil)
			{
				Bittab* b = trans->GetDeletedRecordIDs(GetNum(), err, false);
				if (b != nil)
					result = result - b->Compte();
				result = result + trans->GetHowManyNewRecs(GetNum());
			}
			LibereDelete();
		}
		Unlock();
	}
	else
	{
		result = nbrecord;
		if (result == -1)
			result = DFD.nbfic;
	}

	//CheckUseCount();

	return(result);
}


sLONG DataTableRegular::GetMaxRecords(BaseTaskInfo* context) const
{
	VError err;
	if (NotDeleted(err))
	{
		sLONG result = FicTabAddr.GetNbElem();
		LibereDelete();
		return result;
	}
	else
		return 0;
}


VError DataTableRegular::QuickDelRecord(sLONG n, BaseTaskInfo* context)
{
	VError err = VE_OK;
	VTask::Yield();
	//LockWrite();
	OccupableStack* curstack = gOccPool.GetStackForCurrentThread();

#if trackModif
	trackDebugMsg("before quick delete record\n");
#endif
	if (NotDeleted(err))
	{
		db->DelayForFlush();
		StartDataModif(context);

#if trackModif
		trackDebugMsg("before quick delete record, after StartDataModif\n");
#endif
		LockWrite();

#if trackModif
		trackDebugMsg("before quick delete record, after LockWrite\n");
#endif
		DataAddr4D ou;
		sLONG len;
		if (db->OkToUpdate(err))
		{
			if (n < DFD.nbfic)
			{
				if (LockRecord(n, context, DB4D_Keep_Lock_With_Record))
				{
					FicheOnDisk* ficD = (FicheOnDisk*)FicInMem.GetFromTreeMem(n, curstack);
					if (ficD != nil)
					{
						FicInMem.DelFromTreeMem(n, curstack);
						ficD->setmodif(false, db, context);
						ficD->Release();
						/*
						ficD->setmodif(false, db, context);
						ficD->SetDansCache(false);
						if (ficD->okdel2()) // si la fiche sur disque n'est plus reference par d'autre ficheinmem (en lecture seulement)
						{
							ficD->libere();
							delete ficD;
						}
						*/
					}

					uLONG curstamp;
					ou = FicTabAddr.GetxAddr(n, context, err, curstack, &len, &curstamp);

					if (ou > 0)
					{
#if debug_Addr_Overlap
						db->DelDBObjRef(ou, len+kSizeRecordHeader);
#endif
						err = FicTabAddr.liberetrou(n, context, curstack, curstamp+1);
						UnlockRecord(n, context, DB4D_Do_Not_Lock);

						if (err == VE_OK)
						{
							/*
							if (nbrecord != -1)
								nbrecord--;
								*/
							DecNbRecords(n);
						}
						if (ou > 0)
						{
							if (MustFullyDeleteRecords())
								db->MarkBlockToFullyDeleted(ou, len + kSizeRecordHeader);
							err = db->libereplace(ou, len + kSizeRecordHeader, context, nil);
						}
						WriteDeleteToLog(context, n);
					}
					else
					{
						// ne devrait pas passer par la
						// assert(false);
						UnlockRecord(n, context, DB4D_Do_Not_Lock);
					}
				}
				else
					err = ThrowError(VE_DB4D_RECORDISLOCKED, DBaction_DeletingRecord);
			}

			db->ClearUpdating();
		}
		CheckForMemRequest();
		Unlock();
		EndDataModif(context);
		LibereDelete();
	}

#if trackModif
	trackDebugMsg("after quick delete record, n = "+ToString(n)+"\n");
#endif
	//CheckForMemRequest();
	//Unlock();

	return err;
}


VError DataTableRegular::WriteDeleteToLog(BaseTaskInfo* context, sLONG numfic)
{
	VError err = VE_OK;
	VStream* log;
	Base4D* db = GetDB();
	sLONG len = 4 /*numrecord*/ + sizeof(VUUIDBuffer) /*numtable*/;
	Table* tt = GetTable();

	err = db->StartWriteLog(DB4D_Log_DeleteRecord, len, context, log);
	if (err == VE_OK)
	{
		if (log != nil)
		{
			err = log->PutLong(numfic);
			if (err == VE_OK)
			{
				VUUID xid;
				tt->GetUUID(xid);
				err = xid.WriteToStream(log);
			}
		}
		VError err2 = db->EndWriteLog(len);
		if (err == VE_OK)
			err = err2;
	}

	return err;
}


VError DataTableRegular::DelRecordForTrans(sLONG n, BaseTaskInfo* context)
{
	//CheckUseCount();
	VError err = VE_OK;

	VTask::Yield();
	//LockWrite();
	OccupableStack* curstack = gOccPool.GetStackForCurrentThread();

	if (NotDeleted(err))
	{
		db->DelayForFlush();
		StartDataModif(context);
		LockWrite();

		DataAddr4D ou;
		sLONG len;
		if (db->OkToUpdate(err))
		{
			db->RequestForInvalideHeader();
			//crit->WaitToAlterRecord();
			crit->DeleteIndexKeyForTrans(n, context);
			uLONG curstamp;

			if (crit->HasSyncInfo())
			{
				VError err2 = VE_OK;
				StErrorContextInstaller errs(false);
				FicheInMem* fic = LoadRecord(n, err, DB4D_Do_Not_Lock, nil /*context*/ /* on retire le context pour charger la fiche hors transaction */);
				if (fic != nil)
				{
					err2 = fic->SetSyncInfo(Sync_Delete, context, nil, false);
					fic->Release();
				}

			}

			FicheOnDisk* ficD = (FicheOnDisk*)FicInMem.GetFromTreeMem(n, curstack);
			if (ficD != nil)
			{
				ou = ficD->getaddr();
				len = ficD->GetAntelen() + kSizeRecordHeader;
				FicInMem.DelFromTreeMem(n, curstack);
				ficD->setmodif(false, db, context);
				ficD->Release();

				/*
				ficD->setmodif(false, db, context);
				ficD->SetDansCache(false);
				if (ficD->okdel2()) // si la fiche sur disque n'est plus reference par d'autre ficheinmem (en lecture seulement)
				{
					ficD->libere();
					delete ficD;
					ficD = nil;
				}
				else
				{
					ficD->libere();
				}
				*/
				sLONG len2;
				DataAddr4D ou2 = FicTabAddr.GetxAddr(n, context, err, curstack, &len2, &curstamp);
			}
			else
			{
				ou = FicTabAddr.GetxAddr(n, context, err, curstack, &len, &curstamp);
				len = len + kSizeRecordHeader;
			}

#if debug_Addr_Overlap
			db->DelDBObjRef(ou, len+kSizeRecordHeader);
#endif
			err = FicTabAddr.liberetrou(n, context, curstack, curstamp+1);
			UnlockRecord(n, context, DB4D_Do_Not_Lock);  // deplace ici , le 16 mai 2007

			if (err == VE_OK)
			{
				/*
				if (nbrecord != -1)
					nbrecord--;
					*/
				DecNbRecords(n);
			}
			if (ou > 0)
			{
				if (MustFullyDeleteRecords())
					db->MarkBlockToFullyDeleted(ou, len);
				err = db->libereplace(ou, len, context, nil);
			}
			
			//crit->FinishAlterRecord();
			db->EndOfRequestForInvalideHeader();
			db->ClearUpdating();
		}

		//CheckUseCount();
		CheckForMemRequest();
		Unlock();
		EndDataModif(context);
		LibereDelete();
	}

	//CheckForMemRequest();
	//Unlock();

	return err;
}


VError DataTableRegular::DelRecord(FicheInMem *rec, BaseTaskInfo* context, uLONG stamp)
{
	sLONG n;
	VError err = VE_OK;
	FicheOnDisk *ficD;
	bool RecordHasBeenLocked = false;
	
	//CheckUseCount();

#if trackModif
	trackDebugMsg("before delete record\n");
#endif

	VTask::Yield();
	//LockWrite();
	OccupableStack* curstack = gOccPool.GetStackForCurrentThread();

	if (NotDeleted(err))
	{
		Transaction* trans = GetCurrentTransaction(context);
		Boolean EnTransaction = trans != nil;
		if (!EnTransaction)
			db->DelayForFlush();

		StartDataModif(context);
#if trackModif
		trackDebugMsg("before delete record, after StartDataModif\n");
#endif

		if (db->OkToUpdate(err))
		{
			LockWrite();
			
#if trackModif
			trackDebugMsg("before delete record, after LockWrite\n");
#endif
			ficD=rec->GetAssoc();

			if (stamp != 0xFFFFFFFF)
			{
				if (!rec->IsNew())
				{
					sLONG recnum = rec->GetNum();
					if (rec->ReadOnlyState())
					{
						if (!LockRecord(recnum, context, DB4D_Keep_Lock_With_Record, rec->GetKindOfLockPtr(), rec->GetLockingContextExtraDataPtr()))
						{
							err = ThrowError(VE_DB4D_RECORDISLOCKED, DBaction_SavingRecord);
						}
						else
						{
							RecordHasBeenLocked = true;
							rec->SetReadOnlyState(false);
							if (context != nil)
								context->PutRecIn(rec);
						}
					}
					if (err == VE_OK)
					{
						uLONG curstamp = -1;
						sLONG curlen = 0;
						FicTabAddr.GetxAddr(recnum, context, err, curstack, &curlen, &curstamp);
						if (curstamp != stamp)
						{
							err = rec->ThrowError(VE_DB4D_RECORD_STAMP_NOT_MATCHING, DBaction_SavingRecord);
						}
					}
					if (err != VE_OK)
					{
						if (RecordHasBeenLocked)
						{
							RecordHasBeenLocked = false;
							if (context != nil)
								context->RemoveRecFrom(this, recnum);
							UnlockRecord(recnum, context, DB4D_Keep_Lock_With_Transaction);
							rec->SetReadOnlyState(true);
						}
					}
				}
			}
			
			if (rec->ReadOnlyState())
			{
				if (err == VE_OK)
					err = ThrowError(VE_DB4D_RECORDISLOCKED, DBaction_DeletingRecord);
			}
			else
			{
				n=ficD->getnumfic();
				if (n>=0)
				{
					
					if (rec->MustCheckIntegrity() && db->IsThereATrigger(DB4D_DeleteRecord_Trigger, this))
					{
						Unlock();

						rec->Retain();
						CDB4DRecord* encapsule = new VDB4DRecord(VDBMgr::GetManager(), rec, context->GetEncapsuleur());
						err = db->GetTriggerManager()->CallTrigger(DB4D_DeleteRecord_Trigger, encapsule, context->GetEncapsuleur());
						encapsule->Release();

						LockWrite();
					}
					
					xbox_assert( ficD->getnumfic() == n);	// check if the trigger has changed the current record number (using RestoreFromPop?)

					if (err == VE_OK && rec->MustCheckIntegrity())
					{
						err = crit->CheckReferentialIntegrity(rec, context, false);
					}

					xbox_assert( ficD->getnumfic() == n);

					db->RequestForInvalideHeader();

					if (err == VE_OK)
						err = crit->DeleteIndexKey(rec, context);

					if (err == VE_OK)
					{
						if (!EnTransaction)
							err = rec->SetSyncInfo(Sync_Delete, context, nil, false);
						DataAddr4D oldaddr = ficD->getaddr();
						sLONG oldlen = ficD->GetAntelen();
						if (err == VE_OK)
						{
							StErrorContextInstaller errs(false);
							crit->DeleteBlobs(rec, context);

							if (EnTransaction)
							{
								trans->DelRecord(GetNum(), n);
							}
							else
							{
#if debug_Addr_Overlap
								db->DelDBObjRef(ficD->getaddr(), ficD->GetAntelen()+kSizeRecordHeader);
#endif
								FicInMem.PurgeFromTreeMem(n, curstack);
								//FicInMem.DelFromTreeMem(n);  // deplace ici pour etre avant le unlockrecord, le 16 mai 2007
								uLONG curstamp;
								sLONG len2;
								DataAddr4D ou2 = FicTabAddr.GetxAddr(n, context, err, curstack, &len2, &curstamp);
								err = FicTabAddr.liberetrou(n, context, curstack, curstamp+1);
								if (!EnTransaction)
									UnlockRecord(n, context); // deplace ici pour etre protege par le FicTabAddr.occupe(), le 16 mai 2007
								if (err == VE_OK)
								{
									/*
									if (nbrecord != -1)
										nbrecord--;
										*/
									DecNbRecords(n);
								}
								err = VE_OK;
								ficD->setmodif(false, db, context);
								//ficD->SetDansCache(false);
							}
							
							if (context != nil)
							{
								context->RemoveRecFrom(this, n);
							}
							/* deplace un peu plus haut, le 16 mai 2007
							if (!EnTransaction)
								UnlockRecord(n, context);
								*/

							rec->MarkAsDeleted();

							ficD->WriteDeleteToLog(context);
						}
						if (!EnTransaction)
						{
							if (MustFullyDeleteRecords())
								db->MarkBlockToFullyDeleted(oldaddr, oldlen+kSizeRecordHeader);
							err = db->libereplace(oldaddr, oldlen+kSizeRecordHeader, context, ficD);
						}
					}
					else
					{
						if (stamp != 0 && RecordHasBeenLocked)
						{
							rec->UnLockRecord();
							/*
							if (context != nil)
								context->RemoveRecFrom(this, rec->GetNum(), true);
								*/
							rec->SetReadOnlyState(true);
						}
					}
					
					db->EndOfRequestForInvalideHeader();
					//ficD->libere();
				}
			}
			
			//libere();

			//crit->FinishAlterRecord();
			CheckForMemRequest();
			Unlock();
			db->ClearUpdating();
		}
		
		//CheckUseCount();

		EndDataModif(context);
		LibereDelete();
	}

	//CheckForMemRequest();
	//Unlock();

#if trackModif
	trackDebugMsg("after delete record, n = "+ToString(n)+"\n");
#endif
	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTDELETERECORD, DBaction_DeletingRecord);
	return(err);
}


VError DataTableRegular::DelBlob(Blob4D *inBlob, BaseTaskInfo* context)
{
	sLONG n;
	VError err = VE_OK;
	
#if debugblobs
	if (inBlob != nil)
		debug_delblobnum(inBlob->GetNum(), -2);
#endif

	//CheckUseCount();

	LockWrite();
	OccupableStack* curstack = gOccPool.GetStackForCurrentThread();

	if (NotDeleted(err))
	{
		Transaction* trans = GetCurrentTransaction(context);
		if (trans != nil)
		{
			if (inBlob->IsOutsidePath())
			{
				trans->DelOutsideBlob(inBlob);
			}
			else
			{
				n=inBlob->GetNum();
				if (n>=0)
				{
					err = trans->DelBlob(GetNum(), n);
					if (err == VE_OK)
					{
						inBlob->WriteDeleteToLog(context);
						inBlob->SetNotExisting();  // le blob a normalement ete duplique avant (soit dans VBobText::Flush, soit dans CreVBLobWithPtr ou CreVBlobPicture juste apres le loadblob)
					}
				}
			}
		}
		else
		{
					
			if (inBlob != nil)
			{	
				inBlob->CheckIfMustDeleteOldPath(context, nil);
				if (inBlob->IsOutsidePath())
				{
					inBlob->MarkDeleteOutsidePath();
				}
				else
				{
					n=inBlob->GetNum();
					if (n>=0)
					{
						DataAddr4D oldaddr = inBlob->getaddr();
						sLONG oldlen = inBlob->GetAntelen();
	#if debug_Addr_Overlap
						db->DelDBObjRef(oldaddr, oldlen+kSizeDataBaseObjectHeader);
	#endif

						if (err == VE_OK)
						{
							inBlob->WriteDeleteToLog(context);
							err = BlobTabAddr.liberetrou(n, context, curstack);
							if (err != VE_OK)
							{
								// on continue meme s'il y a une erreur a ce niveau, il est trop tard pout faire marche arriere
								err = PullLastError();
								err = VE_OK;
							}

							inBlob->SetNotExisting();
							BlobInMem.PurgeFromTreeMem(n, curstack);
						}
						db->libereplace(oldaddr, oldlen+kSizeDataBaseObjectHeader, context, inBlob);
					}
				}
			}
			
		}
		LibereDelete();
	}
		
	CheckForMemRequest();
	Unlock();

	//CheckUseCount();

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTDELETEBLOBS, DBaction_DeletingBlob);
	return(err);
}



VError DataTableRegular::DelBlobForTrans(sLONG n, BaseTaskInfo* context, bool alterModifState)
{
	VError err = VE_OK;

	DataAddr4D ou;
	sLONG len;

	//CheckUseCount();

	LockWrite();
	OccupableStack* curstack = gOccPool.GetStackForCurrentThread();

	if (NotDeleted(err))
	{
		Blob4D* inBlob = (Blob4D*)BlobInMem.GetFromTreeMem(n, curstack);

		if (inBlob != nil)
		{
			//inBlob->occupe();
			ou = inBlob->getaddr();
			len = inBlob->GetAntelen()+kSizeDataBaseObjectHeader;
			BlobInMem.DelFromTreeMem(n, curstack);
			inBlob->setmodif(false, db, context);
			//inBlob->SetDansCache(false);
			inBlob->Release();
		}
		else
		{
			ou = BlobTabAddr.GetxAddr(n, context, err, curstack, &len);
		}

		/*
			L.E. 16/07/09 while repairing a database containing an orphaned and damaged blob,
			the blob is not inserted in the target database but we are being called to delete it.
			That's why ou may be < 0 (a hole).
		*/
		if ( (err == VE_OK) && (ou > 0) )
		{
#if debug_Addr_Overlap
			db->DelDBObjRef(ou, len+kSizeDataBaseObjectHeader);
#endif
			// il faut d'abord liberer l'espace occupe par le blob
			err = db->libereplace(ou, len, context, nil);
			if (err == VE_OK)
			{
				err = BlobTabAddr.liberetrou(n, context, curstack);
				if (err != VE_OK)
				{
					// on continue meme s'il y a une erreur a ce niveau, il est trop tard pout faire marche arriere
					err = PullLastError();
					err = VE_OK;
				}
			}
		}

		LibereDelete();
	}

	CheckForMemRequest();
	Unlock();

	//CheckUseCount();

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTDELETEBLOBS, DBaction_DeletingBlob);
	return(err);
}


void DataTableRegular::IncDataModif(BaseTaskInfo* context)
{
	if (testAssert(fDataModifCounts.IncCount(context)))
		fCountDataModif++;
	assert(fCountDataModif>0);
}


void DataTableRegular::DecDataModif(BaseTaskInfo* context)
{
	if (testAssert(fDataModifCounts.DecCount(context)))
		fCountDataModif--;
	assert(fCountDataModif>=0);
}


sLONG DataTableRegular::GetDataModifCount(BaseTaskInfo* context)
{
	return fDataModifCounts.GetCountFor(context);
}


void DataTableRegular::StartDataModif(BaseTaskInfo* context)
{
	LockEntity* newlock;
	if (context == nil)
	{
		newlock = vGetLockEntity();
	}
	else newlock = context->GetLockEntity();

	Boolean stop = false;
	sLONG sleeptime = 5;
	while (!stop)
	{
		db->PoseVerrouDataLocker();
		LockEntity* dblocker = db->GetDBLocker();
		if (dblocker == nil || dblocker == newlock || dblocker->GetOwner() == nil || dblocker->GetLockOthersTimeOut() != -1)
		{
			if (fWholeTableIsLockedBy == nil || fWholeTableIsLockedBy == newlock || fWholeTableIsLockedBy->GetOwner() == nil || fWholeTableIsLockedBy->GetLockOthersTimeOut() != -1)
			{
				stop = true;
			}
			else
			{
				if (db->GetDataModifCount(context) > 0) // c'est volontairement db
				{
					stop = true;
				}
			}
		}
		else
		{
			if (db->GetDataModifCount(context) > 0)
			{
				stop = true;
			}
		}

		if (stop)
		{
			db->IncDataModif(context);
			IncDataModif(context);
		}

		db->RetireVerrouDataLocker();

		if (!stop)
		{
			VTaskMgr::Get()->GetCurrentTask()->Sleep(sleeptime);
			sleeptime = sleeptime * 2;
			if (sleeptime > 100)
				sleeptime = 100;			
		}
	}
}


void DataTableRegular::EndDataModif(BaseTaskInfo* context)
{
	db->PoseVerrouDataLocker();
	db->DecDataModif(context);
	DecDataModif(context);
	db->RetireVerrouDataLocker();
}


void DataTableRegular::ClearLockCount(BaseTaskInfo* context)
{
	fTotalLocks = fTotalLocks - fAllLocks.RemoveContext(context);
}


Boolean DataTableRegular::NotDeleted(VError& outErr, ActionDB4D action) const
{
	//if (db->IsLogValid())
	{
		VInterlocked::Increment(&fNotDeletedRequest);
		if (!fIsDeleted && !fBusyDeleting)
			return true;
		else
		{
			VInterlocked::Decrement(&fNotDeletedRequest);
			outErr = VE_DB4D_DATATABLE_HAS_BEEN_DELETED;
			if (action != (ActionDB4D)-1)
				ThrowError(outErr, action);
			return false;
		}
	}
	/*
	else
	{
		outErr = VE_DB4D_CURRENT_JOURNAL_IS_INVALID;
		if (action != (ActionDB4D)-1)
			ThrowError(outErr, action);
		return false;
	}
	*/
};


VError DataTableRegular::PutMaxRecordRemoteInfo(sLONG curstamp, VStream* outstream)
{
	LockRead();
	if (crit == nil)
	{
		Unlock();
		return VE_OK;
	}
	sLONG numtable = crit->GetNum();
	sLONG stamp = fRemoteMaxRecordsStamp;
	sLONG nbfic = DFD.nbfic;
	Unlock();

	VError err = VE_OK;
	if (stamp > curstamp || curstamp == 0)
	{
		err = outstream->PutLong(numtable);
		if (err == VE_OK)
			err = outstream->PutLong(nbfic);
	}
	return err;
}


uBOOL DataTableRegular::LockTable(BaseTaskInfo* Context, Boolean WaitForEndOfRecordLocks, sLONG TimeToWaitForEndOfRecordLocks)
{
	uBOOL canlock = true;
	LockEntity *newlock;
	Boolean stop = false;

	{
		if (Context == nil)
		{
			newlock = vGetLockEntity();
		}
		else newlock = Context->GetLockEntity();
		uLONG startmillisec;
		Boolean AlreadyAskedTime = false;
		sLONG sleeptime = 5;

		while (!stop)
		{
			canlock = true;
			sLONG counttotlock;
			Boolean SomeOtherLock = false, SomeDataModif = false;
			LockEntity* DejaLocker = nil;

			db->PoseVerrouDataLocker();
			LockEntity* dblocker = db->GetDBLocker();
			if (dblocker == nil || dblocker == newlock || dblocker->GetOwner() == nil)
			{
				if (fWholeTableIsLockedBy == nil || fWholeTableIsLockedBy == newlock || fWholeTableIsLockedBy->GetOwner() == nil)
				{
					if (newlock->GetLockOthersTimeOut() == -1)
					{
						if ((fCountDataModif-GetDataModifCount(Context)) > 0)
							SomeDataModif = true;
					}
					if (WaitForEndOfRecordLocks)
					{
						counttotlock = fTotalLocks - fAllLocks.GetCountFor(Context);
						SomeOtherLock = (counttotlock != 0);
					}
					if (SomeOtherLock && TimeToWaitForEndOfRecordLocks == 0)
						canlock = false;
					else
					{
						fLockCount++;
						fWholeTableIsLockedBy = newlock;
					}
				}
				else
				{
					canlock = false;
					DejaLocker = fWholeTableIsLockedBy;
				}
			}
			else
			{
				canlock = false;
				DejaLocker = dblocker;
			}
			db->RetireVerrouDataLocker();

			if (canlock)
			{
				if (SomeDataModif)
				{
					sLONG sleeptime = 5;
					Boolean stop2 = false;
					while (!stop2)
					{
						VTaskMgr::Get()->GetCurrentTask()->Sleep(sleeptime);
						sleeptime = sleeptime * 2;
						if (sleeptime > 100)
							sleeptime = 100;

						db->PoseVerrouDataLocker();
						if (WaitForEndOfRecordLocks)
						{
							counttotlock = fTotalLocks - fAllLocks.GetCountFor(Context);
							SomeOtherLock = (counttotlock != 0);
						}
						if ((fCountDataModif-GetDataModifCount(Context)) <= 0)
							stop2 = true;
						db->RetireVerrouDataLocker();
					}
				}

				if (SomeOtherLock)
				{
					sLONG sleeptime = 5;
					Boolean stop2 = false;
					while (!stop2)
					{
						if (TimeToWaitForEndOfRecordLocks == -1)
						{
							VTaskMgr::Get()->GetCurrentTask()->Sleep(sleeptime);
							sleeptime = sleeptime * 2;
							if (sleeptime > 100)
								sleeptime = 100;
						}
						else
						{
							uLONG currentime = VSystem::GetCurrentTime();
							if (!AlreadyAskedTime)
							{
								AlreadyAskedTime = true;
								startmillisec = currentime;
							}
							sLONG passedtime = currentime - startmillisec;
							if ( passedtime < TimeToWaitForEndOfRecordLocks)
							{
								sLONG remaintime = TimeToWaitForEndOfRecordLocks - passedtime;
								if (sleeptime > remaintime)
									sleeptime = remaintime;
								VTaskMgr::Get()->GetCurrentTask()->Sleep(sleeptime);
								sleeptime = sleeptime * 2;
								if (sleeptime > 100)
									sleeptime = 100;
							}
							else
								stop2 = true;
						}

						if (!stop2)
						{
							db->PoseVerrouDataLocker();
							counttotlock = fTotalLocks - fAllLocks.GetCountFor(Context);
							SomeOtherLock = (counttotlock != 0);
							db->RetireVerrouDataLocker();
							if (!SomeOtherLock)
								stop2 = true;
						}
					}

					if (SomeOtherLock)
					{
						db->PoseVerrouDataLocker();
						fLockCount--;
						if (fLockCount == 0)
							fWholeTableIsLockedBy = nil;			
						db->RetireVerrouDataLocker();
						canlock = false;
					}
				}
			}

			if (canlock)
				stop = true;
			else
			{
				if (SomeOtherLock)
					stop = true;
				else
				{
					sLONG timeout = DejaLocker->GetLockOthersTimeOut();
					sLONG timeout2 = Context->GetLockTimer();
					if (timeout == -1 || timeout2 == -1)
					{
						VTaskMgr::Get()->GetCurrentTask()->Sleep(sleeptime);
						sleeptime = sleeptime * 2;
						if (sleeptime > 100)
							sleeptime = 100;
						stop = false;
					}
					else
					{
						if (timeout == 0 && timeout2 == 0)
						{
							stop = true;
						}
						else
						{
							if (timeout < timeout2)
								timeout = timeout2;
							uLONG currentime = VSystem::GetCurrentTime();
							if (!AlreadyAskedTime)
							{
								AlreadyAskedTime = true;
								startmillisec = currentime;
							}
							sLONG passedtime = currentime - startmillisec;
							if ( passedtime < timeout)
							{
								sLONG remaintime = timeout - passedtime;
								if (sleeptime > remaintime)
									sleeptime = remaintime;
								VTaskMgr::Get()->GetCurrentTask()->Sleep(sleeptime);
								sleeptime = sleeptime * 2;
								if (sleeptime > 100)
									sleeptime = 100;
								stop = false;
							}
							else
								stop = true;
						}
					}
				}
			}
		}
	}

	return canlock;
}


void DataTableRegular::UnLockTable(BaseTaskInfo* Context)
{
	//CheckUseCount();

	LockEntity *newlock;

	if (Context == nil)
	{
		newlock = vGetLockEntity();
	}
	else newlock = Context->GetLockEntity();
	
	if (Context == nil)
	{
		newlock = vGetLockEntity();
	}
	else newlock = Context->GetLockEntity();

	db->PoseVerrouDataLocker();
	if (fWholeTableIsLockedBy == newlock)
	{
		fLockCount--;
		if (fLockCount == 0)
			fWholeTableIsLockedBy = nil;
	}

	db->RetireVerrouDataLocker();

	//CheckUseCount();

}


#if 1

void DataTableRegular::WhoLockedRecord(sLONG n, VError& err, BaseTaskInfo* Context, DB4D_KindOfLock *outLockType, const VValueBag **outLockingContextRetainedExtraData)
{
	Transaction *trans = GetCurrentTransaction(Context);

	DB4D_KindOfLock xKindOfLock = DB4D_LockedByNone;
	const VValueBag* xLockingContextExtraData = nil;

	LockRead();
	OccupableStack* curstack = gOccPool.GetStackForCurrentThread();

	if (NotDeleted(err))
	{
		if ( trans==nil || !trans->IsRecordDeleted(GetNum(), n)) 
		{
			if (n>=0 && n<DFD.nbfic)
			{
				uBOOL locked = ! LockRecord(n, Context, DB4D_Do_Not_Lock, &xKindOfLock, &xLockingContextExtraData);
			}
			else
			{
				xKindOfLock = DB4D_LockedNoCurrentRecord;
			}
		}
		else
		{
			xKindOfLock = DB4D_LockedNoCurrentRecord;
		}
		LibereDelete();
	}
	else
	{
		xKindOfLock = DB4D_LockedNoCurrentRecord;
	}
	Unlock();

	if (outLockType != nil)
		*outLockType = xKindOfLock;
	if (outLockingContextRetainedExtraData != nil)
		*outLockingContextRetainedExtraData = xLockingContextExtraData;
	else
		QuickReleaseRefCountable( xLockingContextExtraData);
}

#else

BaseTaskInfo* DataTableRegular::WhoLockedRecord(sLONG n, DB4D_KindOfLock& outLockType, BaseTaskInfo* Context, const VValueBag **outLockingContextRetainedExtraData)
{
	BaseTaskInfo* result = nil;
	outLockType = DB4D_LockedByNone;
	LockEntity *obj = nil;
	VError err = VE_OK;
	
	//CheckUseCount();

	if (outLockingContextRetainedExtraData != nil)
		*outLockingContextRetainedExtraData = nil;

	if (testAssert(n>=0 && n<DFD.nbfic))
	{
		if (FicTabAddr.GetxAddr(n, Context, err) > 0)
		{
			db->PoseVerrouDataLocker();
			if (db->GetDBLocker() == nil)
			{
				if (fWholeTableIsLockedBy == nil)
				{
					obj=(LockEntity*)LockRec.GetFromTreeMem(n);
					if (obj != nil)
					{
						if (obj->GetOwner() == nil)
							obj = nil;
					}
					if (obj != nil)
					{
						outLockType = DB4D_LockedByRecord;
					}

					/*
					//occupe();
					if (fCacheLockSel.isOn(n))
					{
						CoupleSelAndContext* xsel = fLockingSels.GetFirst();
						while (xsel != nil)
						{
							BitSel* bsel = (BitSel*)(xsel->GetSel());
							if (bsel->GetBittab()->isOn(n))
							{
								outLockType = DB4D_LockedBySelection;
								obj = xsel->GetContext()->GetLockEntity();
								break;
							}
							xsel =xsel->GetNext();
						}

						if (obj == nil)
						{
							outLockType = DB4D_LockedByRecord;
							obj=(LockEntity*)LockRec.GetFromTreeMem(n);
						}
						assert(obj != nil);
					}
					//libere();
					*/
				}
				else
				{
					outLockType = DB4D_LockedByTable;
					obj = fWholeTableIsLockedBy;
				}
			}
			else
			{
				outLockType = DB4D_LockedByDataBase;
				obj = fWholeTableIsLockedBy;
			}

			if (obj != nil)
			{
				result = obj->GetOwner();
				if (outLockingContextRetainedExtraData != nil)
					*outLockingContextRetainedExtraData = result->RetainExtraData();
			}

			db->RetireVerrouDataLocker();
		}
		else
			outLockType = DB4D_LockedByDeletedRecord;
	}
	else
		outLockType = DB4D_LockedByDeletedRecord;



	//CheckUseCount();

	return result;
}
#endif


uBOOL DataTableRegular::LockRecord(sLONG n, BaseTaskInfo* Context, DB4D_Way_of_Locking HowToLock, DB4D_KindOfLock* outLockType, const VValueBag **outLockingContextRetainedExtraData)
{
	VError err = VE_OK;
	LockEntity *obj = nil, *newlock;
	Boolean stop = false;
	sLONG sleeptime = 5;

	//CheckUseCount();

	if (NotDeleted(err))
	{
		uLONG startmillisec;
		Boolean AlreadyAskedTime = false;
		
		if (Context == nil)
		{
			newlock = vGetLockEntity();
		}
		else newlock = Context->GetLockEntity();

		while (!stop)
		{
			if (outLockType != nil)
				*outLockType = DB4D_LockedByNone;

			if (outLockingContextRetainedExtraData != nil)
				ReleaseRefCountable(outLockingContextRetainedExtraData);

			//occupe();
			db->PoseVerrouDataLocker();
			LockEntity* dblocker = db->GetDBLocker();
			Boolean vasy = true;

			if (!(dblocker == nil || dblocker == newlock || dblocker->GetOwner() == nil))
			{
				vasy = false;
				if (dblocker->GetSpecialFlushAndLock())
					vasy = true;
				else if (dblocker->GetLockOthersTimeOut() == -1)
				{
					// ceci ce produit quand un autre context a demande a locker toute la base, mais qu'il y avait des modif en cours dans le context en cours
					// alors on permet au context en cours de continuer et de locker sa fiche
					// l'autre context attend que toutes les modif soient terminees
					if (db->GetDataModifCount(Context) > 0)
						vasy = true;
				}
			}

			if (vasy)
			{
				if (!(fWholeTableIsLockedBy == nil || fWholeTableIsLockedBy == newlock || fWholeTableIsLockedBy->GetOwner() == nil))
				{
					vasy = false;
					if (fWholeTableIsLockedBy->GetLockOthersTimeOut() == -1)
					{
						// ceci ce produit quand un autre context a demande a locker toute la table, mais qu'il y avait des modif en cours dans le context en cours
						// alors on permet au context en cours de continuer et de locker sa fiche
						// l'autre context attend que toutes les modif soient terminees
						if (db->GetDataModifCount(Context) > 0)
							vasy = true;
					}
				}
				if (vasy)
				{
					OccupableStack* curstack = gOccPool.GetStackForCurrentThread();

					err = LockRec.Aggrandit(max(DFD.nbfic,n+1), curstack);
					if (err == VE_OK)
					{
						obj=(LockEntity*)LockRec.GetFromTreeMem(n, curstack);
						if ((obj==nil) || obj->GetOwner() == nil)
						{
							if (HowToLock != DB4D_Do_Not_Lock)	// on nous demande juste de tester le verrou sans en mettre un
							{
								err=LockRec.PutIntoTreeMem(DFD.nbfic, n, (ObjCacheInTree*)newlock, curstack );
								if (err != VE_OK)
									err = ThrowError(VE_DB4D_CANNOTLOCKRECORD, DBaction_LockingRecord);
								else
								{
									if ((obj == nil || obj->GetOwner() == nil) && (newlock != nil) )
									{
										assert(fAllLocks.IncCount(Context));
										assert(db->IncLockCount(Context));
										fTotalLocks++;
									}
									if (newlock != nil)
										newlock->Retain();
								}
							}
						}
						else
						{
							if (outLockType != nil)
								*outLockType = DB4D_LockedByRecord;
							if (outLockingContextRetainedExtraData != nil)
								*outLockingContextRetainedExtraData = obj->GetOwner()->RetainExtraData();
						}

					}
				}
				else 
				{
					obj = fWholeTableIsLockedBy;
					if (outLockType != nil)
						*outLockType = DB4D_LockedByTable;
					if (outLockingContextRetainedExtraData != nil)
						*outLockingContextRetainedExtraData = obj->GetOwner()->RetainExtraData();
				}
					
			}
			else
			{
				obj = dblocker;
				if (outLockType != nil)
					*outLockType = DB4D_LockedByDataBase;
				if (outLockingContextRetainedExtraData != nil)
					*outLockingContextRetainedExtraData = obj->GetOwner()->RetainExtraData();
			}

			db->RetireVerrouDataLocker();

			//libere();

			if (err == VE_OK)
			{
				if ((obj==nil) || (obj==newlock) || obj->GetOwner() == nil || (HowToLock == DB4D_Do_Not_Lock) )
					stop =true;
				else
				{
					sLONG timeout = obj->GetLockOthersTimeOut();
					sLONG timeout2 = Context->GetLockTimer();
					if (timeout == -1 || timeout2 == -1)
					{
						VTaskMgr::Get()->GetCurrentTask()->Sleep(sleeptime);
						sleeptime = sleeptime * 2;
						if (sleeptime > 100)
							sleeptime = 100;
						stop = false;
					}
					else
					{
						if (timeout == 0 && timeout2 == 0)
						{
							stop = true;
						}
						else
						{
							if (timeout < timeout2)
								timeout = timeout2;
							uLONG currentime = VSystem::GetCurrentTime();
							if (!AlreadyAskedTime)
							{
								AlreadyAskedTime = true;
								startmillisec = currentime;
							}
							sLONG passedtime = currentime - startmillisec;
							if ( passedtime < timeout)
							{
								sLONG remaintime = timeout - passedtime;
								if (sleeptime > remaintime)
									sleeptime = remaintime;
								VTaskMgr::Get()->GetCurrentTask()->Sleep(sleeptime);
								sleeptime = sleeptime * 2;
								if (sleeptime > 100)
									sleeptime = 100;
								stop = false;
							}
							else
								stop = true;
						}
					}
				}
			}
			else
				stop =true;
		}

		//CheckUseCount();

		Boolean oklock = ((obj==nil) || (obj==newlock) || obj->GetOwner() == nil) && (err==VE_OK);

		if (oklock && IsRecordMarkedAsPushed(n))
		{
			oklock = false;
			if (outLockType != nil)
				*outLockType = DB4D_LockedByRecord;
			if (outLockingContextRetainedExtraData != nil && Context != nil)
				*outLockingContextRetainedExtraData = Context->RetainExtraData();
		}

		if (oklock && HowToLock == DB4D_Keep_Lock_With_Transaction && Context != nil)
		{
			Transaction* trans = Context->GetCurrentTransaction();
			if (trans != nil)
				trans->KeepRecordLocked(this, n);
		}

		if (oklock && (HowToLock != DB4D_Do_Not_Lock) )
		{
			if (outLockType != nil)
				*outLockType = DB4D_LockedByRecord;
			if (outLockingContextRetainedExtraData != nil && Context != nil)
			{
				ReleaseRefCountable(outLockingContextRetainedExtraData);
				*outLockingContextRetainedExtraData = Context->RetainExtraData();
			}
		}

		LibereDelete();
		return oklock;
	}
	else
		return false;

}


void DataTableRegular::UnlockRecord(sLONG n, BaseTaskInfo* Context, DB4D_Way_of_Locking HowToLock)
{
	LockEntity *obj, *thelock;
	VError err;
	Boolean canUnlock = true;
	//CheckUseCount();

	if (NotDeleted(err))
	{
		if (Context == nil)
		{
			thelock = vGetLockEntity();
		}
		else
		{
			thelock = Context->GetLockEntity();
			if (HowToLock == DB4D_Do_Not_Lock)
			{
				// appele par Transaction::UnlockSel, rien a faire d'autre
			}
			else
			{
				Transaction* trans = Context->GetCurrentTransaction();
				if (trans != nil)
				{
					if (HowToLock == DB4D_Keep_Lock_With_Transaction)
					{
						trans->DoNotKeepRecordLocked(this, n);
					}
					if (trans->IsRecordLockKept(this, n))
					{
						canUnlock = false;
					}
				}
			}

		}

		if (canUnlock)
	/*
		if (BitLockOnly)
		{
			CoupleSelAndContext* xsel = fLockingSels.GetFirst();
			Boolean mustclear = true;
			while (xsel != nil)
			{
				if (xsel->GetContext()->GetLockEntity() == thelock)
				{
					BitSel* bsel = dynamic_cast<BitSel*>(xsel->GetSel());
					if (bsel->GetBittab()->isOn(n))
					{
						mustclear = false;
						break;
					}
				}
				xsel =xsel->GetNext();
			}

			obj=(LockEntity*)LockRec.GetFromTreeMem(n);
			if (obj == thelock || (obj != nil && obj->GetOwner() == nil) )
			{
				mustclear = false;
			}
			else
				assert(obj == nil);

			if (mustclear) // on libere le lock si fiche pas deja dans une autre locking sel (du meme context, bien sur)
			{
				VError err = fCacheLockSel.ClearOrSet(n,false);
				assert(err == VE_OK);
			}
		}
		else
		*/
		{
			db->PoseVerrouDataLocker();

			/*
			CoupleSelAndContext* xsel = fLockingSels.GetFirst();
			Boolean mustclear = true;
			while (xsel != nil)
			{
				if (xsel->GetContext()->GetLockEntity() == thelock)
				{
					BitSel* bsel = (BitSel*)(xsel->GetSel());
					if (bsel->GetBittab()->isOn(n))
					{
						mustclear = false;
						break;
					}
				}
				xsel =xsel->GetNext();
			}
			*/

			OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
			obj=(LockEntity*)LockRec.GetFromTreeMem(n, curstack);
			if ( (obj == thelock) /* || (obj != nil && obj->GetOwner() == nil)*/ )
			{
				LockRec.DelFromTreeMem(n, curstack);
				if (obj != nil)
				{
					obj->Release();
					fTotalLocks--;
					assert(fAllLocks.DecCount(Context));
					assert(db->DecLockCount(Context));
				}

			}
			/*
			else
				mustclear = false;
				*/

			/*
			if (mustclear)
			{
				VError err = fCacheLockSel.ClearOrSet(n,false);
				assert(err == VE_OK);
			}
			*/

			db->RetireVerrouDataLocker();
		}
		LibereDelete();
	}
	//CheckUseCount();
}


/*
void DataTableRegular::UnlockBitSel(Bittab *b, BaseTaskInfo* Context, DB4D_Way_of_Locking HowToLock)
{
	VError err = VE_OK;
	LockEntity *obj, *thelock;

	//CheckUseCount();

	if (NotDeleted(err))
	{
		if (Context == nil)
		{
			thelock = vGetLockEntity();
		}
		else thelock = Context->GetLockEntity();

		Bittab b2;
		b2.Or(b);

		db->PoseVerrouDataLocker();

		CoupleSelAndContext* xsel = fLockingSels.GetFirst();
		while (xsel != nil)
		{
			if (xsel->GetContext()->GetLockEntity() == thelock)
			{
				BitSel* bsel = dynamic_cast<BitSel*>(xsel->GetSel());
				b2.moins(bsel->GetBittab());
			}
			xsel =xsel->GetNext();
		}

		sLONG i = b2.FindNextBit(0);
		while (i != -1)
		{
			obj = (LockEntity*)LockRec.GetFromTreeMem(i);
			if (obj != nil)
			{
				assert(obj == thelock);
				err = b2.ClearOrSet(i, false);
				assert(err == VE_OK);
			}
			i = b2.FindNextBit(i+1);
		}

		fCacheLockSel.moins(&b2);

		db->RetireVerrouDataLocker();
		LibereDelete();
	}
	//CheckUseCount();
}
*/


Boolean DataTableRegular::IsBusyDeleting()
{
	Boolean busy;
	
	fAdminAccess.Lock();
	busy = fBusyDeleting;
	fAdminAccess.Unlock();
	
	return busy;
}


bool DataTableRegular::StartDeleting()
{
	bool ok;
	fAdminAccess.Lock();
	fBusyDeleting = true;
	VTask::YieldNow();
	bool noRequest = (fNotDeletedRequest == 0);
	fAdminAccess.Unlock();
	if (noRequest)
	{
		LockWrite();

		Table* oldcrit = crit;
		crit = nil;

		OccupableStack* curstack = gOccPool.GetStackForCurrentThread();

		if (LibereAllLock(curstack, true))
		{
			if (fDefIsInDataPart && oldcrit != nil)
				oldcrit->Release();
			ok = true;
		}
		else
		{
			crit = oldcrit;
			Unlock();
			fAdminAccess.Lock();
			fBusyDeleting = false;
			fAdminAccess.Unlock();
			ok = false;
		}
	}
	else
	{
		fAdminAccess.Lock();
		fBusyDeleting = false;
		fAdminAccess.Unlock();
		ok = false;
	}
	return ok;
}

void DataTableRegular::StopDeleting()
{
	Unlock();
}



VError DataTableRegular::LibereEspaceDisk(OccupableStack* curstack, VDB4DProgressIndicator* InProgress)
{
	VError err = VE_OK, err2;
	Boolean AuMoinsUneErreur = false;
	Boolean PremiereErreur = true;
	DataAddr4D ou;
	sLONG len,i;
	
	Boolean mustfullydel = MustFullyDeleteRecords() || fMustFullyDeleteForLibereEspaceDisk;
	// LockWrite();
	
	for (i = 0; i < DFD.nbfic; i++)
	{
		ou = FicTabAddr.GetxAddr(i, nil, err2, curstack, &len);
		if (err2 != VE_OK)
		{
			if (PremiereErreur)
				PremiereErreur = false;
			else
				err2 = PullLastError();
			AuMoinsUneErreur = false;
		}
		else
		{
			if (ou > 0)
			{
				if (mustfullydel)
					db->MarkBlockToFullyDeleted(ou, len+kSizeRecordHeader);
				err2 = db->libereplace(ou, len+kSizeRecordHeader, nil, this);
			#if debugplace
				shex.FromULong((uLONG)ou,kHexaDecimal);
				UnivStrDebug("LiberePlace record = ",shex);
			#endif
				if (err2 != VE_OK)
				{
					if (PremiereErreur)
						PremiereErreur = false;
					else
						err2 = PullLastError();
					AuMoinsUneErreur = false;
				}
			}
		}
	}

	for (i = 0; i < DFD.nbBlob; i++)
	{
		ou = BlobTabAddr.GetxAddr(i, nil, err2, curstack, &len);
		if (err2 != VE_OK)
		{
			if (PremiereErreur)
				PremiereErreur = false;
			else
				err2 = PullLastError();
			AuMoinsUneErreur = false;
		}
		else
			{
			if (ou > 0)
			{
				err2 = db->libereplace(ou, len+kSizeDataBaseObjectHeader, nil, this);
				if (err2 != VE_OK)
				{
					if (PremiereErreur)
						PremiereErreur = false;
					else
						err2 = PullLastError();
					AuMoinsUneErreur = false;
				}
			}
		}
	}
	
	err2 = BlobTabAddr.LibereEspaceDisk(InProgress, curstack);
	if (err2 != VE_OK)
	{
		if (PremiereErreur)
			PremiereErreur = false;
		else
			err2 = PullLastError();
		AuMoinsUneErreur = false;
	}
	err2 = FicTabAddr.LibereEspaceDisk(InProgress, curstack);
	if (err2 != VE_OK)
	{
		if (PremiereErreur)
			PremiereErreur = false;
		else
			err2 = PullLastError();
		AuMoinsUneErreur = false;
	}
	
	// Unlock();
	
	if (AuMoinsUneErreur)
	{
		err = ThrowError(VE_DB4D_COULDNOTCOMPLETELYDELETETABLE, DBaction_DeletingTable);
	}
	return err;
}


void DataTableRegular::LibereEspaceMem(OccupableStack* curstack)
{
	FicInMem.LibereEspaceMem(curstack);
	BlobInMem.LibereEspaceMem(curstack);
	
	FicTabAddr.LibereEspaceMem(curstack);
	BlobTabAddr.LibereEspaceMem(curstack);
}


Boolean DataTableRegular::LibereAllLock(OccupableStack* curstack, Boolean CheckAlsoLoadedRecords)
{
	Boolean ok = true;
			
	if (NbLoadedRecords==0 || !CheckAlsoLoadedRecords)
	{
		if (LockRec.TryToPurge(curstack))
		{
			// comme il ne reste plus de fiche lockees sur ce fichier, on peut le detruire
		}
		else
			ok = false;
	}
	else
		ok = false;
		
	return ok;
}


void DataTableRegular::IncNbLoadedRecords()
{
	fAdminAccess.Lock();
	NbLoadedRecords++;
	fAdminAccess.Unlock();
}


void DataTableRegular::DecNbLoadedRecords()
{
	fAdminAccess.Lock();
	NbLoadedRecords--;
	fAdminAccess.Unlock();
}

VError DataTableRegular::DeleteAll(VDB4DProgressIndicator* InProgress, VSyncEvent* event, bool mustFullyDelete, sLONG numtable)
{
	VError err = VE_OK;
	
	if (db->OkToUpdate(err))
	{
		LockWrite();
		OccupableStack* curstack = gOccPool.GetStackForCurrentThread();

		if (LibereAllLock(curstack, true))
		{
			fAdminAccess.Lock();
			fIsDeleted = true;
			setmodif(false, db, nil);
			fAdminAccess.Unlock();
			VUUID ID(DFD.SeqNum_ID);
			db->DeleteSeqNum(ID, nil);
			fSeq = nil;

			FicInMem.LibereEspaceMem(curstack);
			BlobInMem.LibereEspaceMem(curstack);
			fMustFullyDeleteForLibereEspaceDisk = mustFullyDelete;
			err = LibereEspaceDisk(curstack, InProgress);
			mustFullyDelete = false;
			
			if (numtable != -1)
			{
				VFolder* parent = db->RetainBlobsFolder();
				if (parent != nil)
				{
					VString s;
					s = "Table "+ToString(numtable);
					VFolder xf(*parent, s);
					if (xf.Exists())
					{
						xf.DeleteContents(true);
						xf.Delete(true);
					}
					parent->Release();
				}
			}

			
			std::fill(&DFD.SeqNum_ID.fBytes[0], &DFD.SeqNum_ID.fBytes[16], 0);
			std::fill(&DFD.TableDefID.fBytes[0], &DFD.TableDefID.fBytes[16], 0);
			DFD.filler8 = 0;
			DFD.fLastRecordSync = 0;
			DFD.nbfic=0;
			DFD.newnum=0;
			DFD.debuttrou=kFinDesTrou;
			DFD.addrtabaddr=0;
			DFD.segref=0;
			DFD.nbBlob=0;
			DFD.debutBlobTrou=kFinDesTrou;
			DFD.addrBlobtabaddr=0;

			if (crit != nil && fDefIsInDataPart)
				crit->Release();
			crit = nil;
			fBusyDeleting = false;
		}
		else
		{
			//StartDeleting(InProgress, event);
			err = ThrowError(VE_DB4D_BUSY_OBJECT, noaction);
		}
		Unlock();

		db->ClearUpdating();
	}
	return err;
}


VError DataTableRegular::Truncate(BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, Boolean ForADeleteSelection, IndexArray& indexdeps)
{
	VError err = VE_OK;

	assert(GetCurrentTransaction(context) == nil);

	StartDataModif(context);
	if (db->OkToUpdate(err))
	{
		LockWrite();
		OccupableStack* curstack = gOccPool.GetStackForCurrentThread();

		if (context != nil)
			context->UnlockAllLoadedRecs(this);
		if (LibereAllLock(curstack, false))
		{
			/*
			for (IndexArray::Iterator cur = indexdeps.First(), end = indexdeps.End(); cur != end; cur++)
			{
				IndexInfo* ind = *cur;
				if (ind != nil)
					ind->SetInvalidOnDisk();
			}
			*/
			GetTable()->DelayIndexes();

			FicInMem.LibereEspaceMem(curstack);
			BlobInMem.LibereEspaceMem(curstack);
			err = LibereEspaceDisk(curstack, InProgress);
			if (err == VE_OK)
			{
				if (crit != nil)
				{
					VFolder* parent = db->RetainBlobsFolder();
					if (parent != nil)
					{
						VString s;
						s = "Table "+ToString(crit->GetNum());
						VFolder xf(*parent, s);
						if (xf.Exists())
						{
							xf.DeleteContents(true);
							xf.Delete(true);
						}
						parent->Release();
					}
				}

				nbrecord = 0;
				fAllRecords.ClearFrom(0);
				fAllRecordsInited = true;

				DFD.nbfic=0;
				DFD.debuttrou=kFinDesTrou;
				DFD.addrtabaddr=0;
				DFD.nbBlob=0;
				DFD.debutBlobTrou=kFinDesTrou;
				DFD.addrBlobtabaddr=0;
				setmodif(true, db, context);
			}
		}
		else
		{
			if (ForADeleteSelection)
			{
				err = -2;
			}
			else
				err = ThrowError(VE_DB4D_SOME_RECORDS_ARE_STILL_LOCKED, noaction);
		}
		Unlock();
		db->ClearUpdating();
	}
	EndDataModif(context);
	return err;
}


VError DataTableRegular::CheckData(CheckAndRepairAgent* inCheck)
{
	VError err = VE_OK;

	//err = FicTabAddr.CheckAddresses(inCheck);
	OccupableStack* curstack = gOccPool.GetStackForCurrentThread();

	if (err == VE_OK)
	{
		sLONG nb = DFD.nbfic;
		sLONG i,len;

		for (i=0; i<nb; i++)
		{
			DataAddr4D ou=FicTabAddr.GetxAddr(i, nil, err, curstack, &len);
			if (err == VE_OK)
			{
				RecordHeader tag;
				err = tag.ReadFrom(db, ou);
				if (err==VE_OK)
				{
					if (tag.ValidateTag(DBOH_Record,i,GetTrueNum()) == VE_OK)
					{
						sLONG nbc = tag.GetNbFields();
						uLONG8 TimeStamp = tag.GetTimeStamp();
						sLONG	lenx=tag.GetLen();
						void* fData=GetFastMem(lenx, false, 'tmp4');
						ChampHeaderArray chd;

						if (fData!=nil)
						{
							err=db->readlong(fData,lenx,ou,kSizeRecordHeader);
							if (err==VE_OK)
							{
								err = tag.ValidateCheckSum(fData,lenx);
								if (err == VE_OK)
								{
									if (!chd.AddNSpaces(nbc,true))
									{
										// err = ThrowError(memfull, DBaction_LoadingRecord);
									}
									else
									{
										err=db->readlong(chd.First(), sizeof(ChampHeader)*nbc, ou, kSizeRecordHeader+lenx);
									}
								}
							}
							FreeFastMem(fData);
						}
						else
						{
							// err = ThrowError(memfull, DBaction_LoadingRecord);
						}
					}
					else
					{
						err = VE_DB4D_WRONGRECORDHEADER; // DBaction_LoadingRecord);
					}
				}
			}
		}

	}


	return err;
}

/*
CoupleSelAndContext* DataTableRegular::FindLockingSel(Selection* sel, BaseTaskInfo* context)
{
	CoupleSelAndContext *result = nil, *xsel;

	//CheckUseCount();
	xsel = fLockingSels.GetFirst();
	while (xsel != nil)
	{
		if (xsel->GetSel() == sel && xsel->GetContext() == context)
		{
			result = xsel;
			break;
		}
		xsel = xsel->GetNext();
	}

	//CheckUseCount();
	return result;
}


Boolean DataTableRegular::AddLockSel(Bittab* b, Selection* sel, BaseTaskInfo* context)
{
	Boolean ok = true, dejasel = false;
	VError err = VE_OK;
	CoupleSelAndContext* xsel;
	
	//CheckUseCount();
	occupe();

	xsel = FindLockingSel(sel, context);
	if (xsel != nil)
	{
		dejasel = true;
	}

	if (!dejasel)
	{
		xsel = new CoupleSelAndContext(sel, context);
		if (xsel == nil)
		{
			err = ThrowError(memfull, DBaction_LockingSelection);
			ok = false;
		}
		else
		{
			fLockingSels.AddTail(xsel);
			ok = fCacheLockSel.Intersect(b,err);
			if (err != VE_OK)
				ok = false;
			else
			{
				if (ok)
				{
					err = fCacheLockSel.Or(b);
				}
			}
		}
	}
	libere();

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOT_LOCK_SEL, DBaction_LockingSelection);
	//CheckUseCount();
	return ok;
}

Boolean DataTableRegular::RemoveLockSel(Bittab* b, Selection* sel, BaseTaskInfo* context)
{
	VError err = VE_OK;

	//CheckUseCount();
	LockEntity *obj, *thelock;

	if (context == nil)
	{
		thelock = vGetLockEntity();
	}
	else thelock = context->GetLockEntity();

	occupe();

	CoupleSelAndContext* xsel = FindLockingSel(sel, context);
	assert(xsel != nil);
	if (xsel != nil)
	{
		fLockingSels.Remove(xsel);
	}

	Bittab b2;
	b2.Or(b);

	xsel = fLockingSels.GetFirst();
	while (xsel != nil)
	{
		if (xsel->GetContext()->GetLockEntity() == thelock)
		{
			BitSel* bsel = dynamic_cast<BitSel*>(xsel->GetSel());
			b2.moins(bsel->GetBittab());
		}
		xsel =xsel->GetNext();
	}

	sLONG i = b2.FindNextBit(0);
	while (i!=-1)
	{
		obj = (LockEntity*)LockRec.GetFromTreeMem(i);
		if (obj != nil)
		{
			assert(obj == thelock);
			err = b2.ClearOrSet(i, false);
			assert(err == VE_OK);
		}
		i = b2.FindNextBit(i+1);
	}

	err = fCacheLockSel.moins(&b2);

	libere();

	//CheckUseCount();
	return true;
}
*/


void DataTableRegular::DecNbRecords(sLONG n)
{
	if (nbrecord != -1)
		nbrecord--;
	if (fAllRecordsInited)
		fAllRecords.Clear(n, true);
}


void DataTableRegular::IncNbRecords(sLONG n)
{
	if (nbrecord != -1)
		nbrecord++;
	if (fAllRecordsInited)
		fAllRecords.Set(n, true);

}



VError DataTableRegular::MarkAddrAsNewForTrans(sLONG numrec, BaseTaskInfo* context) 
{ 
	//CheckUseCount();
	Transaction* trans = GetCurrentTransaction(context);
	if (testAssert(trans != nil))
	{
		trans->IncNewRecs(GetNum());
	}
	//CheckUseCount();
	return fNewRecsInTransID.Set(numrec, true); 
}


VError DataTableRegular::UnMarkAddrAsNewForTrans(sLONG numrec, BaseTaskInfo* context) 
{ 
	//CheckUseCount();
	Transaction* trans = GetCurrentTransaction(context);
	if ((trans != nil))
	{
		trans->DecNewRecs(GetNum());
	}
	//CheckUseCount();
	return fNewRecsInTransID.Clear(numrec, false); 
}


VError DataTableRegular::CheckForNonEmptyTransHolesList()
{
	VError err = VE_OK;
	Boolean modif = false;

	LockWrite();
	OccupableStack* curstack = gOccPool.GetStackForCurrentThread();

	if (DFD.debutTransRecTrou == 0)
		DFD.debutTransRecTrou = kFinDesTrou;

	if (DFD.debutTransRecTrou != kFinDesTrou)
	{
		StErrorContextInstaller errs(false);
		Bittab dejapass;

		while (DFD.debutTransRecTrou != kFinDesTrou && err == VE_OK)
		{
			sLONG n = -DFD.debutTransRecTrou - 1;
			sLONG len;

			if (dejapass.isOn(n))
				err = ThrowError(VE_DB4D_CIRCULAR_REF_IN_REC_ADDR_TABLE, noaction);
			else
			{
				dejapass.Set(n);
				DataAddr4D ou = FicTabAddr.GetxAddr(n, nil, err, curstack, &len);
				if (testAssert(len <= 0 && ou <= 0) && err == VE_OK)
				{
					FicTabAddr.liberetrou(n, nil, curstack);
					sLONG n2 = (sLONG)ou;
					if (n2 != kFinDesTrou)
						n2 = n2 - 1;
					DFD.debutTransRecTrou = n2;
					modif = true;
				}
				else
					err = ThrowError(VE_DB4D_CIRCULAR_REF_IN_REC_ADDR_TABLE, noaction);
			}
		}

		DFD.debutTransRecTrou = kFinDesTrou;
		modif = true;
	}

	err = VE_OK;

	if (DFD.debutTransBlobTrou == 0)
		DFD.debutTransBlobTrou = kFinDesTrou;

	if (DFD.debutTransBlobTrou != kFinDesTrou)
	{
		StErrorContextInstaller errs(false);
		Bittab dejapass;
		while (DFD.debutTransBlobTrou != kFinDesTrou && err == VE_OK)
		{
			sLONG n = -DFD.debutTransBlobTrou - 1;
			sLONG len;

			if (dejapass.isOn(n))
				err = ThrowError(VE_DB4D_CIRCULAR_REF_IN_BLOB_ADDR_TABLE, noaction);
			else
			{
				dejapass.Set(n);
				DataAddr4D ou = BlobTabAddr.GetxAddr(n, nil, err, curstack, &len);
				if (testAssert(len <=0 && ou <= 0) && err == VE_OK)
				{
					BlobTabAddr.liberetrou(n, nil, curstack);
					sLONG n2 = (sLONG)ou;
					if (n2 != kFinDesTrou)
						n2 = n2 - 1;
					DFD.debutTransBlobTrou = n2;
					modif = true;
				}
				else
					err = ThrowError(VE_DB4D_CIRCULAR_REF_IN_BLOB_ADDR_TABLE, noaction);
			}
		}

		DFD.debutTransBlobTrou = kFinDesTrou;
		modif = true;
	}

	err = VE_OK;

	if (modif)
		setmodif(true, GetDB(), nil);

	Unlock();

	return err;
}


VError DataTableRegular::TakeOffBits(Bittab *tb, BaseTaskInfo* context) 
{ 
	VError err = VE_OK;

	//CheckUseCount();
	LockRead();
	OccupableStack* curstack = gOccPool.GetStackForCurrentThread();

	if (NotDeleted(err))
	{
		Bittab bAll;
		if (fAllRecordsInited)
		{
			bAll.Or(&fAllRecords, true);
		}
		else
		{
			bAll.aggrandit(GetMaxRecords(context));
			bAll.ClearOrSetAll(true);

			if (err == VE_OK)
				err = FicTabAddr.TakeOffBits(&bAll, context, curstack);

			if (err == VE_OK)
			{
				fAllRecordsInited = true;
				fAllRecords.Or(&bAll, true);
			}
		}

		if (err == VE_OK && DFD.debutTransRecTrou != kFinDesTrou)
			err = FicTabAddr.TakeOffBits(&bAll, context, curstack, true, DFD.debutTransRecTrou+1);

		Transaction* trans = GetCurrentTransaction(context);
		if (trans != nil)
		{
			if (err == VE_OK)
			{
				Bittab* b;

				b = trans->GetSavedRecordIDs(GetNum(), err, false);
				if (b != nil)
				{
					err = bAll.Or(b, true);
				}
				if (err == VE_OK)
				{
					b = trans->GetDeletedRecordIDs(GetNum(), err, false);
					if (b != nil)
					{
						err = bAll.moins(b, false);
					}
				}
			}
		}

		if (err == VE_OK)
		{
			err = tb->And(&bAll, true);
		}

		LibereDelete();
	}
	Unlock();
	
	//CheckUseCount();
	return err;
}


Boolean DataTableRegular::CheckForNonUniqueField(const NumFieldArray& fields, VProgressIndicator* progress, VError &err, CDB4DBaseContext* inContext )
{
	SortTab xsort(GetDB());
	err = VE_OK;
	CDB4DBase* basex = GetDB()->RetainBaseX();
	if (basex != nil)
	{
		CDB4DBaseContext* context = nil;
		if (inContext == nil)
			context = basex->NewContext(nil, nil);
		else
		{
			context = inContext;
			context->Retain();
		}
		if (context != nil)
		{
			Selection* allrecs = AllRecords(ConvertContext(context), err);
			if (allrecs != nil)
			{
				CDB4DSelection* alls = new VDB4DSelection(VDBMgr::GetManager(), VImpCreator<VDB4DBase>::GetImpObject(basex), GetTable(), allrecs);

				if (alls != nil)
				{
					for (NumFieldArray::ConstIterator cur = fields.First(), end = fields.End(); cur != end; cur++)
					{
						xsort.AddTriLineField(GetNum(), *cur, true);
					}

					if (context != nil)
					{
						(VImpCreator<VDB4DSelection>::GetImpObject(alls))->SortSelection(&xsort, progress, context, err, true, true);
					}
					else
						err = ThrowError(memfull, DBaction_CheckingUniqueness);

					alls->Release();
				}
				else
					err = ThrowError(memfull, DBaction_CheckingUniqueness);
			}
			else
				err = ThrowError(memfull, DBaction_CheckingUniqueness);

			context->Release();
		}
		else
			err = ThrowError(memfull, DBaction_CheckingUniqueness);

		basex->Release();
	}
	else
		err = ThrowError(memfull, DBaction_CheckingUniqueness);

	return err != VE_OK;
}


AutoSeqNumber* DataTableRegular::GetSeqNum(CDB4DBaseContext* inContext)
{
	AutoSeqNumber* result = nil;
	fAdminAccess.Lock();

	//CheckUseCount();
	if (fSeq == nil)
	{
		VUUID ID(DFD.SeqNum_ID);
		fSeq = db->RetainSeqNum(ID);
		if (fSeq == nil)
		{
			VError err;
			fSeq = db->AddSeqNum(DB4D_AutoSeq_Simple, err, inContext);
			if (fSeq != nil)
			{
				fSeq->GetID().ToBuffer(DFD.SeqNum_ID);
				setmodif(true, db, nil);
			}
			else
				fSeq = (AutoSeqNumber*)-1;
		}
	}

	if (fSeq == (AutoSeqNumber*)-1)
		result = nil;
	else
		result = fSeq;

	fAdminAccess.Unlock();

	//CheckUseCount();
	return result;
}


void DataTableRegular::Debug_CheckBlobsAddr(BaseTaskInfo* context)
{
	LockRead();
	OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
	sLONG nb = BlobTabAddr.GetNbElem();

	for (sLONG i = 0; i<nb; i++)
	{
		VError err;
		DataAddr4D ou = BlobTabAddr.GetxAddr(i, context, err, curstack);
		assert(err == VE_OK);
		if (ou > 0)
		{
			Blob4D* blob = FindBlob(i, curstack);
			if (blob == nil)
			{
				assert(false);
			}
		}
	}
	Unlock();
}


uLONG DataTableRegular::GetRecordStampInAddressTable(sLONG numrec)
{
	uLONG stamp;
	if (numrec < FicTabAddr.GetNbElem())
	{
		OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
		VError err;
		sLONG len;
		DataAddr4D ou = FicTabAddr.GetxAddr(numrec, nil, err, curstack, &len, &stamp);
		return stamp;
	}
	else
		return 0;
}


VError DataTableRegular::ResizeTableBlobs(OccupableStack* curstack, sLONG nbblobs)
{
	return BlobTabAddr.InitAndSetSize(nbblobs, nil, curstack);
}


VError DataTableRegular::NormalizeTableBlobs(OccupableStack* curstack)
{
	return BlobTabAddr.Normalize(nil, curstack);
}

VError DataTableRegular::ResizeTableRecs(OccupableStack* curstack, sLONG nbrecs)
{
	return FicTabAddr.InitAndSetSize(nbrecs, nil, curstack);
}

VError DataTableRegular::NormalizeTableRecs(OccupableStack* curstack)
{
	return FicTabAddr.Normalize(nil, curstack);
}

VError DataTableRegular::SetRecordEntryAsFree(sLONG numrec, BaseTaskInfo* context)
{
	OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
	return FicTabAddr.liberetrou(numrec, context, curstack);
}


VError DataTableRegular::SetKeepStamp(uBYTE inKeepStamp, VDB4DProgressIndicator* InProgress)
{
	VError err = VE_OK;
	if (inKeepStamp != DFD.fKeepStamps)
	{
		LockWrite();
		OccupableStack* curstack = gOccPool.GetStackForCurrentThread();

		if (NotDeleted(err))
		{
			if (db->OkToUpdate(err))
			{

				err = FicTabAddr.ConvertAddrTable(inKeepStamp == 1, InProgress, curstack);
				if (err == VE_OK)
				{
					DFD.fKeepStamps = inKeepStamp;
					setmodif(true, db, nil);
				}
				db->ClearUpdating();
			}

			//CheckUseCount();
			LibereDelete();
		}

		Unlock();
	}

	return err;
}


#if AllowSyncOnRecID
VError DataTableRegular::GetModificationsSinceStamp(uLONG stamp, VStream& outStream, uLONG& outLastStamp, sLONG& outNbRows, BaseTaskInfo* context, vector<sLONG>& cols)
{
	VError err = VE_OK;

	LockRead();
	OccupableStack* curstack = gOccPool.GetStackForCurrentThread();

	Bittab changed;
	changed.aggrandit(FicTabAddr.GetNbElem());
	//err = FicTabAddr.ScanForGreaterStamp(stamp, changed, curstack);

	if (cols.empty())
		crit->GetFieldsNum(cols);
	sLONG nbcol = (sLONG)cols.size();

	sLONG nb = FicTabAddr.GetNbElem();
	for (sLONG i = 0; i < nb && err == VE_OK; i++)
	{
		uLONG stampx;
		sLONG len;
		DataAddr4D addr = FicTabAddr.GetxAddr(i, context, err, curstack, &len, &stampx);
		if (err == VE_OK)
		{
			if (stampx > stamp)
			{
				err = changed.Set(i);
			}
		}
	}

	if (err == VE_OK)
	{
		outNbRows = changed.Compte();
		err = outStream.PutLong(outNbRows);
		outLastStamp = DFD.fLastRecordStamp-1;
		sLONG numrec = changed.FindNextBit(0);
		while (numrec != -1 && err == VE_OK)
		{
			uLONG stampx;
			sLONG len;
			sBYTE action = 0;
			DataAddr4D addr = FicTabAddr.GetxAddr(numrec, context, err, curstack, &len, &stampx);
			if (err == VE_OK)
			{
				if (addr > 0)
				{
					action = DB4D_SyncAction_Update;
				}
				else
				{
					action = DB4D_SyncAction_Delete;
				}

				err = outStream.PutByte(action);
				err = outStream.PutLong(numrec);
				err = outStream.PutLong(stampx);

				if (action == DB4D_SyncAction_Update && err == VE_OK)
				{
					err = outStream.PutLong(nbcol);
					FicheInMem* rec = LoadRecord(numrec, err, DB4D_Do_Not_Lock, context);
					if (err == VE_OK && rec != nil)
					{
						for (sLONG i = 0; (i < nbcol) && err == VE_OK; i++)
						{
							VValueSingle* cv = rec->GetNthField(cols[i], err);
							if (err == VE_OK)
							{
								if (cv == nil || cv->IsNull())
								{
									outStream.PutWord(VK_EMPTY);
								}
								else
								{
									err = outStream.PutWord(cv->GetValueKind());
									if (err == VE_OK)
										err = cv->WriteToStream(&outStream);
								}
							}
						}
					}
					QuickReleaseRefCountable(rec);
				}

				numrec = changed.FindNextBit(numrec+1);
			}
		}

		if (err == VE_OK)
			err = outStream.PutByte(0);
	}

	Unlock();

	return err;
}

VError DataTableRegular::IntegrateModifications(VStream& inStream, BaseTaskInfo* context, vector<sLONG>& cols)
{
	VError err = VE_OK;
	sLONG rowCount = 0;
	err = inStream.GetLong(rowCount);
	if ( err != VE_OK )
		return err;

	LockWrite();
	OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
	if (cols.empty())
		crit->GetFieldsNum(cols);
	sLONG nbcol = (sLONG)cols.size();

	sBYTE action;

	do 
	{
		uLONG stampx;
		sLONG numrec;
		vector<VValueSingle*> values;
		err = crit->GetOneRow(inStream, context, action, stampx, numrec, values);
		if (err == VE_OK && action != 0)
		{
			if (action == DB4D_SyncAction_Delete)
			{
				if (FicTabAddr.GetxAddr(numrec, context, err, curstack) > 0)
				{
					FicheInMem* rec = LoadRecord(numrec, err, DB4D_Keep_Lock_With_Record, context);
					if (err == VE_OK)
					{
						err = DelRecord(rec, context);
					}
					QuickReleaseRefCountable(rec);
				}
			}
			else if (action == DB4D_SyncAction_Update)
			{
				FicheInMem* rec;
				if (numrec >= FicTabAddr.GetNbElem())
					rec = NewRecord(err, context);
				else
				{
					if (FicTabAddr.GetxAddr(numrec, context, err, curstack) > 0)
						rec = LoadRecord(numrec, err, DB4D_Keep_Lock_With_Record, context);
					else
					{
						rec = NewRecord(err, context);
						rec->SetRecordNumberPreAllocated(numrec);
					}
				}
				if (err == VE_OK)
				{
					sLONG nbcol2 = (sLONG)cols.size();
					if (nbcol2 > nbcol)
						nbcol2 = nbcol;
					for (sLONG i = 0; i < nbcol2; i++)
					{
						VValueSingle* cv = rec->GetNthField(cols[i], err);
						if (cv != nil)
						{
							VValueSingle* cv2 = values[i];
							if (cv2 == nil)
							{
								cv->Clear();
								cv->SetNull(true);
							}
							else
							{
								cv->FromValue(*cv2);
							}
							rec->Touch(cols[i]);
						}
					}
					rec->DoNotCheckIntegrity();
					err = SaveRecord(rec, context);
				}
				QuickReleaseRefCountable(rec);

			}
		}

		for (vector<VValueSingle*>::iterator cur = values.begin(), end = values.end(); cur != end; cur++)
			delete *cur;

	} while(err == VE_OK && action != 0);
	
	CheckForMemRequest();
	Unlock();

	return err;
}

#endif


VError DataTableRegular::SendModificationValue ( VValueSingle* inValue, VStream& inOutputStream, bool inWithRawImages, std::vector<XBOX::VString*>* inImageFormats)
{
	VError			err = VE_OK;

	if (inValue == nil || inValue->IsNull())
	{
		err = inOutputStream.PutWord(VK_EMPTY);
	}
	else
	{
		err = inOutputStream.PutWord(inValue->GetValueKind());
		if (err == VE_OK)
		{
			if ( inValue->GetValueKind() == VK_IMAGE && inWithRawImages && inImageFormats != nil && VDBMgr::GetManager()->GetGraphicsInterface() != nil )
			{
				err = VDBMgr::GetManager()->GetGraphicsInterface()-> ConvertImageToPreferredFormat ( *inValue, *inImageFormats );
				if ( err == VE_OK )
				{
					VBlobWithPtr			vblbPict;
					inValue-> GetValue ( vblbPict );
					err = vblbPict. WriteToStream ( &inOutputStream );
				}
				else
					inOutputStream.PutWord(VK_EMPTY);
			}
			else
				err = inValue->WriteToStream(&inOutputStream);
		}
	}

	return err;
}


VError DataTableRegular::GetModificationsSinceStamp(uLONG8 stamp, VStream& outStream, uLONG8& outLastStamp, sLONG& outNbRows, BaseTaskInfo* context, vector<sLONG>& cols,
													Selection* filter, sLONG8 skip, sLONG8 top, std::vector<XBOX::VString*>* inImageFormats)
{
	VError err = VE_OK;

	outLastStamp = 0;
	outNbRows = 0;
	bool nbrowwaswritten = false;

	bool								bBinary = true;
	if ( inImageFormats != 0 )
	{
		vector<VString*>::iterator		iter = inImageFormats-> begin ( );
		while ( iter != inImageFormats-> end ( ) )
		{
			if ( ( *iter ) != 0 && ( *iter )-> EqualToString ( CVSTR ( ".json" ) ) )
			{
				bBinary = false;

				break;
			}
			iter++;
		}
	}

	bool bWithRawImages = ( inImageFormats != 0 && inImageFormats-> size ( ) > 0 );

	IReplicationOutputFormatter*		formatter = 0;
	if ( bBinary )
		formatter = new ReplicationOutputBinaryFormatter ( outStream, bWithRawImages, inImageFormats );
	else
	{
		formatter = new ReplicationOutputJSONFormatter ( outStream, bWithRawImages, inImageFormats );
		formatter-> SetFields ( crit, cols );
	}

	if (crit->HasSyncInfo())
	{
		LockRead();
		
		OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
		SynchroBaseHelper* synchelp = db->GetSyncHelper();

		Table* tsync = synchelp->GetTableSync(crit, err, false);
		if (tsync != nil || stamp == -1)
		{
			FieldArray primkeyfields;
			crit->RetainPrimaryKey(primkeyfields);
			sBYTE bNbKeyFields = primkeyfields.GetCount();
			formatter-> SetPrimaryKey ( primkeyfields );

			if (cols.empty())
				crit->GetFieldsNum(cols);
			sLONG nbcol = (sLONG)cols.size();

			if (stamp == -1) // on veut toutes les fiches, on ne tient pas compte des modifications
			{
				Selection* sel;
				if (filter == nil)
				{
					sel = AllRecords(context, err);
				}
				else
					sel = RetainRefCountable(filter);

				SelectionIterator itersel(sel);
				sLONG numrec;
				sLONG xtop = (sLONG)top;
				sLONG xskip = (sLONG)skip;

				outNbRows = sel->GetQTfic();
				if (xskip >= outNbRows)
				{
					outNbRows = 0;
				}
				else
				{
					if ( xskip > 0 )
						outNbRows = outNbRows - xskip;
				}
				if (xtop > 0)
				{
					if (xtop < outNbRows)
						outNbRows = xtop;
				}

				if (xskip <=0)
					numrec = itersel.FirstRecord();
				else
					numrec = itersel.SetCurrentRecord(xskip);

				/*if (xskip > 0)
					outNbRows = outNbRows - skip;*/
				//err = outStream.PutLong(outNbRows);
				err = formatter->PutActionCount(outNbRows);
				nbrowwaswritten = true;
				outLastStamp = DFD.fLastRecordSync;
				if ( err == VE_OK )
					err = formatter-> PutLatestStamp ( outLastStamp );

				bool mustcheckNbRows = (xtop > 0);
				sLONG nbrow = 0;

				while (numrec != -1 && err == VE_OK)
				{
					nbrow++;
					FicheInMem* rec = LoadRecord(numrec, err, DB4D_Do_Not_Lock, context);
					if (rec != nil)
					{
						uLONG8 Syncstamp;
						VTime Timestamp;
						Sync_Action Action;
						err = rec->GetFullSyncInfo(Syncstamp, Timestamp, Action);
						if (err == VE_OK)
						{
							if (Action == Sync_None)
								Action = Sync_Update;
							//err = outStream.PutByte(Action);
							//err = outStream.PutByte(bNbKeyFields);
							err = formatter->PutAction(Action);
							err = formatter->PutPrimaryKeyCount(bNbKeyFields);

							for (FieldArray::Iterator cur = primkeyfields.First(), end = primkeyfields.End(); cur != end; cur++)
							{
								Field* cri = *cur;
								VValueSingle* cv = rec->GetFieldValue(cri, err);
								if (err == VE_OK)
								{
									//err = SendModificationValue ( cv, outStream, bWithRawImages, inImageFormats );
									err = formatter->PutVValue ( cv );
								}								
							}

							//err = outStream.PutLong8(Syncstamp);
							//err = Timestamp.WriteToStream(&outStream);
							err = formatter->PutStamp(Syncstamp);
							err = formatter->PutTimeStamp(Timestamp);

							if (err == VE_OK)
							{
								//err = outStream.PutLong(nbcol);
								err = formatter->PutFieldCount(nbcol);

								if (err == VE_OK)
								{
									for (sLONG i = 0; (i < nbcol) && err == VE_OK; i++)
									{
										VValueSingle* cv = rec->GetNthField(cols[i], err);
										if (err == VE_OK)
										{
											//err = SendModificationValue ( cv, outStream, bWithRawImages, inImageFormats );
											err = formatter->PutVValue ( cv );
										}
									}
								}
							}


							
						}
					}
					QuickReleaseRefCountable(rec);
					numrec = itersel.NextRecord();

					if (mustcheckNbRows && numrec != -1)
					{
						if (nbrow >= xtop)
							numrec = -1;
					}
				}

				QuickReleaseRefCountable(sel);
				//outStream.PutByte(0);
				formatter->PutAction(0);
			}
			else // on recherche les modification
			{
				SearchTab query(tsync);
				bool onlyDeleted = false;
				if (stamp == -2)
				{
					stamp = 0;
					onlyDeleted = true;
				}
				VLong8 valstamp(stamp);
				query.AddSearchLineSimple(tsync->GetNum(), 2, DB4D_GreaterOrEqual, &valstamp, false);
				if (onlyDeleted)
				{
					VWord valaction(Sync_Delete);
					query.AddSearchLineBoolOper(DB4D_And);
					query.AddSearchLineSimple(tsync->GetNum(), 1, DB4D_Like, &valaction, false);
				}
				OptimizedQuery xquery;
				BaseTaskInfo* syncHelperContext = context->GetOrBuildSyncHelperContext(synchelp->GetBase(err, false));
				xquery.AnalyseSearch(&query, syncHelperContext);
				Selection* sel = xquery.Perform((Bittab*)nil, nil, syncHelperContext, err, DB4D_Do_Not_Lock);
				if (sel != nil)
				{
					if (filter != nil)
					{
						SortTab sortorder(tsync->GetOwner());
						for (sLONG i = 4; i < 4+bNbKeyFields; i++)
							sortorder.AddTriLineField(tsync->GetNum(), i, true);
						Selection* actionsel = tsync->GetDF()->SortSel(sortorder, sel, context, nil, err);

						SortTab sortorder2(db);
						for (FieldArray::Iterator cur = primkeyfields.First(), end = primkeyfields.End(); cur != end; cur++)
						{
							Field* cri = *cur;
							sortorder2.AddTriLineField(cri, true);
						}
						Selection* filtersorted = SortSel(sortorder2, filter, context, nil, err);

						if (filtersorted != nil && actionsel != nil)
						{
							BitSel* result = new BitSel(tsync->GetDF());
							Bittab *b = result->GetBittab();
							FicheInMem* rec1 = nil;
							FicheInMem* rec2 = nil;
							sLONG numrec2;

							sLONG i1 = 0, i2 = 0, nb1 = filtersorted->GetQTfic(), nb2 = actionsel->GetQTfic();
							while (i1 < nb1 && i2 < nb2 && err == VE_OK)
							{
								while (rec1 == nil && i1 < nb1 && err == VE_OK)
								{
									rec1 = LoadRecord(filtersorted->GetFic(i1), err, DB4D_Do_Not_Lock, context);
									if (rec1 == nil)
										i1++;
								}
								while (rec2 == nil && i2 < nb2 && err == VE_OK)
								{
									numrec2 = actionsel->GetFic(i2);
									rec2 = tsync->GetDF()->LoadRecord(numrec2, err, DB4D_Do_Not_Lock, syncHelperContext);
									if (rec2 == nil)
										i2++;
								}
								if (rec1 != nil && rec2 != nil)
								{
									VValueSingle* cvaction = rec2->GetNthField(1, err);
									if (cvaction != nil && cvaction->GetByte() == Sync_Delete)
									{
										b->Set(numrec2);
										ReleaseRefCountable(&rec2);
										i2++;
									}
									else
									{
										CompareResult compres = CR_EQUAL;

										sLONG i =4;
										for (FieldArray::Iterator cur = primkeyfields.First(), end = primkeyfields.End(); cur != end && err == VE_OK; cur++, i++)
										{
											Field* cri = *cur;
											VValueSingle* cv1 = rec1->GetFieldValue(cri, err);
											VValueSingle* cv2 = rec2->GetNthField(i, err);
											CompareResult compres2 = cv1->CompareToSameKind(cv2, false);
											if (compres2 != CR_EQUAL)
											{
												compres = compres2;
												break;
											}
										}
										if (compres == CR_EQUAL)
										{
											b->Set(numrec2);
											ReleaseRefCountable(&rec1);
											i1++;
											ReleaseRefCountable(&rec2);
											i2++;
										}
										else
										{
											if (compres == CR_SMALLER)
											{
												ReleaseRefCountable(&rec1);
												i1++;
											}
											else
											{
												ReleaseRefCountable(&rec2);
												i2++;
											}
										}
									}
								}
							}

							QuickReleaseRefCountable(rec1);
							while (i2 < nb2 && err == VE_OK)
							{
								while (rec2 == nil && i2 < nb2 && err == VE_OK)
								{
									numrec2 = actionsel->GetFic(i2);
									rec2 = tsync->GetDF()->LoadRecord(numrec2, err, DB4D_Do_Not_Lock, syncHelperContext);
									if (rec2 == nil)
										i2++;
								}
								if (rec2 != nil)
								{	
									VValueSingle* cvaction = rec2->GetNthField(1, err);
									if (cvaction != nil && cvaction->GetByte() == Sync_Delete)
									{
										b->Set(numrec2);
									}
									ReleaseRefCountable(&rec2);
									i2++;
								}
							}

							if (err == VE_OK)
							{
								actionsel->Touch();
								sel->Release();
								sel = result;
								actionsel = nil;
							}
						}
						else
							err = -1;

						QuickReleaseRefCountable(actionsel);
						QuickReleaseRefCountable(filtersorted);

					}

					SelectionIterator itersel(sel);
					sLONG numrec;
					sLONG xtop = (sLONG)top;
					sLONG xskip = (sLONG)skip;
					
					outNbRows = sel->GetQTfic();
					if (xskip >= outNbRows)
					{
						outNbRows = 0;
					}
					else
					{
						if ( xskip > 0 )
							outNbRows = outNbRows - xskip;
					}
					if (xtop > 0)
					{
						if (xtop < outNbRows)
							outNbRows = xtop;
					}

					if (xskip <=0)
						numrec = itersel.FirstRecord();
					else
						numrec = itersel.SetCurrentRecord(xskip);

				/*	if (xskip > 0)
						outNbRows = outNbRows - skip;*/
					err = formatter->PutActionCount(outNbRows);
					nbrowwaswritten = true;
					outLastStamp = DFD.fLastRecordSync;
					if ( err == VE_OK )
						err = formatter-> PutLatestStamp ( outLastStamp );

					bool mustcheckNbRows = (xtop > 0);
					sLONG nbrow = 0;

					while (numrec != -1 && err == VE_OK)
					{
						nbrow++;
						FicheInMem* recsync = tsync->GetDF()->LoadRecord(numrec, err, DB4D_Do_Not_Lock, syncHelperContext);
						
						if (err == VE_OK)
						{
							uLONG8 stampx;
							sBYTE action = 0;
							VValueSingle* cvtimestamp;
							VValueSingle* cvstamp;
							VValueSingle* cvaction;

							cvaction = recsync->GetNthField(1, err);
							action = cvaction->GetByte();

							cvstamp = recsync->GetNthField(2, err);
							cvtimestamp = recsync->GetNthField(3, err);

							err = formatter->PutAction(action);
							err = formatter->PutPrimaryKeyCount(bNbKeyFields);
							for (sLONG i = 4; i < bNbKeyFields+4 && err == VE_OK; i++)
							{
								VValueSingle* cv = recsync->GetNthField(i, err);
								if (err == VE_OK)
								{
									err = formatter->PutVValue ( cv );
								}
							}

							err = formatter->PutStamp(cvstamp->GetLong8());
							err = formatter->PutTimeStamp(*cvtimestamp);

							if (action == DB4D_SyncAction_Update && err == VE_OK)
							{
								err = formatter->PutFieldCount(nbcol);

								SearchTab keyquery(crit);
								bool first = true;
								sLONG i = 4;
								for (FieldArray::Iterator cur = primkeyfields.First(), end = primkeyfields.End(); cur != end; cur++, i++)
								{
									Field* cri = *cur;
									if (first)
										first = false;
									else
										keyquery.AddSearchLineBoolOper(DB4D_And);

									VValueSingle* val = recsync->GetNthField(i, err);
									VString empty;
									if (val == nil)
										val = &empty;

									keyquery.AddSearchLineSimple(crit->GetNum(), cri->GetPosInRec(), DB4D_Equal, val, true);
								}

								OptimizedQuery xkeyquery;
								xkeyquery.AnalyseSearch(&keyquery, context);
								Selection* keysel = xkeyquery.Perform((Bittab*)nil, nil, context, err, DB4D_Do_Not_Lock);
								FicheInMem* rec = nil;
								if (keysel != nil && keysel->GetQTfic() > 0)
								{
									rec = LoadRecord(keysel->GetFic(0), err, DB4D_Do_Not_Lock, context);
								}
								QuickReleaseRefCountable(keysel);

								if (err == VE_OK && rec != nil)
								{
									for (sLONG i = 0; (i < nbcol) && err == VE_OK; i++)
									{
										VValueSingle* cv = rec->GetNthField(cols[i], err);
										if (err == VE_OK)
										{
											err = formatter->PutVValue(cv);
										}
									}
								}
								else
									err = ThrowError(VE_DB4D_NO_PRIMARYKEY_MATCHING_THIS_FOREIGNKEY, noaction);
								QuickReleaseRefCountable(rec);
							}
						}
						QuickReleaseRefCountable(recsync);
						numrec = itersel.NextRecord();

						if (mustcheckNbRows && numrec != -1)
						{
							if (nbrow >= xtop)
								numrec = -1;
						}
					}
				
					formatter->PutAction(0);
					
					sel->Release();
				}
			}

			for (FieldArray::Iterator cur = primkeyfields.First(), end = primkeyfields.End(); cur != end; cur++)
				(*cur)->Release();
		}

		Unlock();
	}

	if (!nbrowwaswritten)
	{
		formatter-> PutEmptyResponse ( );
	}

	delete formatter;

	return err;
}




VError DataTableRegular::IntegrateModificationsWithPrimKey(VStream& inStream, BaseTaskInfo* context, vector<sLONG>& cols, bool sourceOverDest,
																uLONG8& ioFirstDestStampToCheck, uLONG8& outErrorStamp, bool inBinary)
{
	VError err = VE_OK;
	outErrorStamp = 0;
	
	SynchroBaseHelper* synchelp = db->GetSyncHelper();

	Table* tsync = nil;

	if (crit->HasSyncInfo())
		tsync = synchelp->GetTableSync(crit, err, false);

	if (err == VE_OK)
	{
		IReplicationInputFormatter*		irFormatter = NULL;
		if ( inBinary )
			irFormatter = new ReplicationInputBinaryFormatter ( inStream );
		else
		{
			irFormatter = new ReplicationInputJSONFormatter ( inStream );
			irFormatter-> SetFields ( crit, cols );
		}

		sLONG rowCount = 0;
		err = irFormatter-> GetRowCount ( rowCount );
		if ( err != VE_OK )
		{
			delete irFormatter;

			return err;
		}

		FieldArray primkeyfields;
		crit->RetainPrimaryKey(primkeyfields);
		irFormatter-> SetPrimaryKey ( primkeyfields );


		LockWrite();
		OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
		if (cols.empty())
			crit->GetFieldsNum(cols);
		sLONG nbcol = (sLONG)cols.size();
		sBYTE action;
		uLONG8 stampx;
		VTime timestamp;
		vector<VValueSingle*> values;
		vector<VValueSingle*> primkey;

		do 
		{
			sLONG numrec;
			err = crit->GetOneRow(*irFormatter, context, action, stampx, timestamp, primkey, values);
			outErrorStamp = stampx;
			if (err == VE_OK && action != 0)
			{

				SearchTab keyquery(crit);
				bool first = true;
				vector<VValueSingle*>::iterator curval = primkey.begin();

				for (FieldArray::Iterator cur = primkeyfields.First(), end = primkeyfields.End(); cur != end; cur++, curval++)
				{
					Field* cri = *cur;
					if (first)
						first = false;
					else
					{
						keyquery.AddSearchLineBoolOper(DB4D_And);
					}

					VValueSingle* val = *curval;
					keyquery.AddSearchLineSimple(crit->GetNum(), cri->GetPosInRec(), DB4D_Equal, val, true);
				}

				bool canUpdate = true;
				uLONG8 dejaStamp = 0;
				Sync_Action dejaAction = Sync_None;
				VTime dejaTimeStamp;

				if (tsync != nil && !sourceOverDest)
				{
					synchelp->GetSynchroStamp(crit, primkey, dejaStamp, dejaTimeStamp, dejaAction, context);
					if (dejaAction != Sync_None && dejaStamp > ioFirstDestStampToCheck)
					{
						canUpdate = false;
					}
				}

				if (canUpdate)
				{
					FicheInMem* rec = nil;
					OptimizedQuery xkeyquery;
					xkeyquery.AnalyseSearch(&keyquery, context);
					Selection* keysel = xkeyquery.Perform((Bittab*)nil, nil, context, err, DB4D_Do_Not_Lock);
					if (keysel != nil && keysel->GetQTfic() > 0)
					{
						rec = LoadRecord(keysel->GetFic(0), err, DB4D_Keep_Lock_With_Record, context);
					}
					QuickReleaseRefCountable(keysel);

					if (action == DB4D_SyncAction_Delete)
					{
						if (err == VE_OK && rec != nil)
						{
							rec->DoNotCheckIntegrity();
							err = DelRecord(rec, context);
						}
					}
					else if (action == DB4D_SyncAction_Update)
					{
						bool newone = false;
						if (rec == nil)
						{
							rec = NewRecord(err, context);
							newone = true;
						}
						if (err == VE_OK)
						{
							bool toutegal = !newone;
							sLONG nbcol2 = (sLONG)cols.size();
							if (nbcol2 > nbcol)
								nbcol2 = nbcol;
							for (sLONG i = 0; i < nbcol2; i++)
							{
								VValueSingle* cv = rec->GetNthField(cols[i], err);
								if (cv != nil)
								{
									VValueSingle* cv2 = values[i];
									if (cv2 == nil)
									{
										if (!cv->IsNull())
											toutegal = false;
										cv->Clear();
										cv->SetNull(true);
									}
									else
									{
										if (toutegal)
										{
											if (cv->GetValueKind() == cv2->GetValueKind())
												toutegal = cv->EqualToSameKind(cv2, true);
											else
												toutegal = false;
										}
										cv->FromValue(*cv2);
									}
									rec->Touch(cols[i]);
								}
							}

							vector<VValueSingle*>::iterator curval = primkey.begin();
							for (FieldArray::Iterator cur = primkeyfields.First(), end = primkeyfields.End(); cur != end; cur++, curval++)
							{
								Field* cri = *cur;
								VValueSingle* val = *curval;
								VValueSingle* cv = rec->GetFieldValue(cri, err);
								if (cv != nil && val != nil)
									cv->FromValue(*val);
								rec->Touch(cri);
							}
							rec->DoNotCheckIntegrity();
							if (!toutegal)
								err = SaveRecord(rec, context);
						}
					}

					QuickReleaseRefCountable(rec);
				}
			}

			for (vector<VValueSingle*>::iterator cur = values.begin(), end = values.end(); cur != end; cur++)
				delete *cur;

			values. clear ( );

			for (vector<VValueSingle*>::iterator curPK = primkey.begin(), endPK = primkey.end(); curPK != endPK; curPK++)
				delete *curPK;

			primkey. clear ( );

		} while(err == VE_OK && action != 0);

		ioFirstDestStampToCheck = DFD.fLastRecordSync;

		if ( err != VE_OK && action != 0 )
		{
			VError		vReadError = VE_OK;
			while ( vReadError == VE_OK && action != 0 )
			{
				values. clear ( );
				vReadError = crit->GetOneRow(*irFormatter, context, action, stampx, timestamp, primkey, values);
				for (vector<VValueSingle*>::iterator cur = values.begin(), end = values.end(); cur != end; cur++)
					delete *cur;
			}
		}

		for (FieldArray::Iterator cur = primkeyfields.First(), end = primkeyfields.End(); cur != end; cur++)
			(*cur)->Release();

		CheckForMemRequest();
		Unlock();

		delete irFormatter;
	}

	return err;
}




void DataTableRegular::MarkRecordAsPushed(sLONG numrec)
{
	VTaskLock lock(&fPushedRecordIDsMutex);
	fPushedRecordIDs[numrec]++;
}

void DataTableRegular::UnMarkRecordAsPushed(sLONG numrec)
{
	VTaskLock lock(&fPushedRecordIDsMutex);
	map<sLONG, sLONG>::iterator found = fPushedRecordIDs.find(numrec);
	if (testAssert(found != fPushedRecordIDs.end()))
	{
		assert(found->second > 0);
		if (found->second == 1)
			fPushedRecordIDs.erase(found);
		else
			found->second--;
	}

}


bool DataTableRegular::IsRecordMarkedAsPushed(sLONG numrec)
{
	VTaskLock lock(&fPushedRecordIDsMutex);
	map<sLONG, sLONG>::iterator found = fPushedRecordIDs.find(numrec);
	if (found != fPushedRecordIDs.end())
		return true;
	else
		return false;
}


void DataTableRegular::SetAssociatedTable(Table* tt, sLONG TableDefNumInData)
{
	DataTable::SetAssociatedTable(tt, TableDefNumInData);

	VUUID xid;
	tt->GetUUID(xid);
	xid.ToBuffer(DFD.TableDefID);
}


VError DataTableRegular::GetFragmentation(sLONG8& outTotalRec, sLONG8& outFrags)
{
	return FicTabAddr.GetFragmentation(outTotalRec, outFrags);
}



void DataTableRegular::CacheOutsideBlob(Blob4D* inBlob)
{
	try
	{
		if (!inBlob->GetOutsidePath().IsEmpty())
		{
			VString path = inBlob->GetOutsidePath();
			path += inBlob->GetOutsideSuffixe();
			VTaskLock lock(&fCacheOutsideBlobMutex);
			cacheOutsideBlob::iterator found = fCacheOutsideBlob.find(path);
			if (found == fCacheOutsideBlob.end())
			{
				fCacheOutsideBlob[path] = inBlob;
				inBlob->Retain();
			}
			else
			{
				Blob4D* oldBlob = found->second;
				if (inBlob != oldBlob)
				{
					inBlob->Retain();
					oldBlob->setmodif(false, db, nil);
					oldBlob->Release();
					found->second = inBlob;
				}
			}
		}
	}
	catch (...)
	{
	}
}


void DataTableRegular::MarkOutsideBlobToDelete(const VString& inID, const VString& inPath)
{
	UnCacheOutsideBlob(inID);
	if (!inPath.IsEmpty())
		VDBMgr::GetManager()->AddBlobPathToDelete(inPath);
}


void DataTableRegular::UnMarkOutsideBlobToDelete(const VString& inPath)
{
	if (!inPath.IsEmpty())
		VDBMgr::GetManager()->RemoveBlobPathToDelete(inPath);
}

void DataTableRegular::UnCacheOutsideBlob(const VString& inPath)
{
	VTaskLock lock(&fCacheOutsideBlobMutex);
	cacheOutsideBlob::iterator found = fCacheOutsideBlob.find(inPath);
	if (found != fCacheOutsideBlob.end())
	{
		fCacheOutsideBlob.erase(found);
	}

}

void DataTableRegular::FreeOutsideBlobCache(sLONG allocationBlockNumber, VSize combien, VSize& outSizeFreed)
{
	outSizeFreed = 0;
	VTaskLock lock(&fCacheOutsideBlobMutex);
	VectorOfVString ftempcache;
	for (cacheOutsideBlob::iterator cur = fCacheOutsideBlob.begin(), end = fCacheOutsideBlob.end(); cur != end /*&& outSizeFreed < combien*/; cur++)
	{
		if (cur->second->MatchAllocationNumber(allocationBlockNumber))
		{
			VSize size = cur->second->GetDataLength();
			outSizeFreed += size;
			ftempcache.push_back(cur->first);
			cur->second->Release();
		}
	}
	for (VectorOfVString::iterator cur = ftempcache.begin(), end = ftempcache.end(); cur != end; cur++)
	{
		fCacheOutsideBlob.erase(*cur);
	}
}


Blob4D* DataTableRegular::RetainOutsideBlobFromCache(const VString& inPath)
{
	Blob4D* result = nil;
	VTaskLock lock(&fCacheOutsideBlobMutex);
	cacheOutsideBlob::iterator found = fCacheOutsideBlob.find(inPath);
	if (found != fCacheOutsideBlob.end())
	{
		result = found->second;
		result->Retain();
	}
	return result;
}


#if debugblobs

void DataTableRegular::debug_putblobnum(sLONG blobnum, sLONG recnum)
{
	if (blobnum >= 0 && recnum >= 0)
	{
		debug_MapOfBlobNums::iterator found = fDebug_MapOfBlobs.find(blobnum);
		if (found != fDebug_MapOfBlobs.end())
		{
			if (found->second != recnum)
			{
				recnum = recnum; // put a break here
				assert(false);
			}
			found->second = recnum;
		}
		else
			fDebug_MapOfBlobs[blobnum] = recnum;
	}
}

void DataTableRegular::debug_delblobnum(sLONG blobnum, sLONG recnum)
{
	fDebug_MapOfBlobs.erase(blobnum);
}


#endif




											/* ---------------------------------------------------------- */



DataTableSystem::DataTableSystem(Base4D *xdb, Table* xcrit):DataTable(xdb, xcrit, xcrit->GetNum())
{

}


DataTableSystem::~DataTableSystem()
{
}


FicheOnDisk* DataTableSystem::LoadNotFullRecord(sLONG n, VError& err, DB4D_Way_of_Locking HowToLock, BaseTaskInfo* Context, Boolean BitLockOnly, 
									   ReadAheadBuffer* buffer, Boolean* CouldLock, Boolean* outEnoughMem)
{ 
	FicheOnDisk* result = nil;
	err = VE_OK;
	if (CouldLock != nil)
		*CouldLock = false;
	FicheOnDisk* temp = new FicheOnDisk(crit->GetOwner(), crit->GetDF(), crit);
	if (temp == nil)
		err = ThrowError(memfull, DBactionFinale);
	else
	{
		//temp->use();
		FicheInMem* fic = LoadRecord(n, err, DB4D_Do_Not_Lock, Context, false, false, buffer, outEnoughMem);
		if (fic != nil)
		{
			result = temp->ReajusteData(fic, err, false);
			if (result != nil)
			{
				result->setnumfic(n);
			}
			fic->Release();
		}
		//temp->FreeAfterUse();
		temp->Release();
	} 


	return result; 
};


#if debugOverlaps_strong

Boolean DataTableRegular::Debug_CheckAddrOverlap(DataAddr4D addrToCheck, sLONG lenToCheck, sLONG numblobToCheck, sLONG numrecToCheck)
{
	return FicTabAddr.Debug_CheckAddrOverlap(addrToCheck, lenToCheck, numrecToCheck) || BlobTabAddr.Debug_CheckAddrOverlap(addrToCheck, lenToCheck, numblobToCheck);
}

Boolean DataTableRegular::Debug_CheckBlobAddrMatching(DataAddr4D addrToCheck, sLONG blobID)
{
	if (blobID>=0 && addrToCheck != 0)
	{
		VError err;
		DataAddr4D danstable = BlobTabAddr.GetxAddr(blobID, nil, err);
		if (danstable != addrToCheck)
		{
			err = err; // break here
			assert(danstable == addrToCheck);
			return true;
		}
	}
	return false;
}

#endif


#if debug_Addr_Overlap
void DataTableRegular::FillDebug_DBObjRef()
{
	OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
	if (crit != nil)
	{
		sLONG tablenum = crit->GetNum();
		FicTabAddr.FillDebug_DBObjRef(tablenum, true);
		BlobTabAddr.FillDebug_DBObjRef(tablenum, false);
		for (sLONG i = 0, nb = FicTabAddr.GetNbElem(); i < nb; i++)
		{
			VError err = VE_OK;
			sLONG len;
			DataAddr4D addr = FicTabAddr.GetxAddr(i, nil, err, curstack, &len);
			if (addr > 0 && err == VE_OK)
				db->SetDBObjRef(addr, len+kSizeRecordHeader, new debug_FicheRef(tablenum, i));
		}

		for (sLONG i = 0, nb = BlobTabAddr.GetNbElem(); i < nb; i++)
		{
			VError err = VE_OK;
			sLONG len;
			DataAddr4D addr = BlobTabAddr.GetxAddr(i, nil, err, curstack, &len);
			if (addr > 0 && err == VE_OK)
				db->SetDBObjRef(addr, len+kSizeDataBaseObjectHeader, new debug_BlobRef(tablenum, i));
		}

	}
}

#endif







											/* ---------------------------------------------------------- */







DataTableOfTables::DataTableOfTables(Base4D *xdb, Table* xcrit):DataTableSystem(xdb, xcrit)
{

}


DataTableOfTables::~DataTableOfTables()
{
}


FicheInMem* DataTableOfTables::LoadRecord(sLONG n, VError& err, DB4D_Way_of_Locking HowToLock, BaseTaskInfo* Context, 
										Boolean WithSubRecords, Boolean BitLockOnly, ReadAheadBuffer* buffer, Boolean* outEnoughMem)
{
	err = VE_OK;
	FicheInMemSystem* fic = nil;

	Table* tt = db->RetainTable(n+1);
	if (tt != nil)
	{
		TableOfTable* tcrit = (TableOfTable*)crit;
		fic = new FicheInMemSystem(Context, db, crit, err);
		if (fic == nil)
			err = ThrowError(memfull, DBaction_LoadingRecord);
		else
			fic->SetNumRec(n);

		if (err == VE_OK)
		{
			VString* sname = new VString();
			if (sname == nil)
				err = ThrowError(memfull, DBaction_LoadingRecord);
			if (err == VE_OK)
			{
				tt->GetName(*sname);
				err = fic->SetNthField(tcrit->id_Table_Name, sname);
				if (err != VE_OK)
					delete sname;
			}
		}

		if (err == VE_OK)
		{
			VBoolean* stemp = new VBoolean(false);
			if (stemp == nil)
				err = ThrowError(memfull, DBaction_LoadingRecord);
			else
			{
				err = fic->SetNthField(tcrit->id_Temporary, stemp);
				if (err != VE_OK)
					delete stemp;
			}
		}

		if (err == VE_OK)
		{
			VLong8* sID = new VLong8(tt->GetNum());
			if (sID == nil)
				err = ThrowError(memfull, DBaction_LoadingRecord);
			else
			{
				err = fic->SetNthField(tcrit->id_Table_ID, sID);
				if (err != VE_OK)
					delete sID;
			}
		}

		if (err == VE_OK)
		{
			VLong* sID = new VLong(tt->GetSchema());
			if (sID == nil)
				err = ThrowError(memfull, DBaction_LoadingRecord);
			else
			{
				err = fic->SetNthField(tcrit->id_Schema_ID, sID);
				if (err != VE_OK)
					delete sID;
			}
		}

		if (err == VE_OK)
		{
			VBoolean* sreplicatable = new VBoolean(tt->GetKeepRecordSyncInfo());
			if (sreplicatable == nil)
				err = ThrowError(memfull, DBaction_LoadingRecord);
			else
			{
				err = fic->SetNthField(tcrit->id_Replication, sreplicatable);
				if (err != VE_OK)
					delete sreplicatable;
			}
		}

		tt->Release();
	}

	if (err != VE_OK)
	{
		err = ThrowError(VE_DB4D_CANNOTLOADRECORD, DBaction_LoadingRecord);
		if (fic != nil)
			fic->Release();
		fic = nil;
	}

	return fic;
}


sLONG DataTableOfTables::GetNbRecords(BaseTaskInfo* context, bool lockread)
{
	return db->GetNBTableWithoutDeletedTables();
}


sLONG DataTableOfTables::GetMaxRecords(BaseTaskInfo* context) const
{
	return db->GetNBTable();
}


VError DataTableOfTables::TakeOffBits(Bittab *tb, BaseTaskInfo* context)
{
	return db->TakeOffDeletedTables(tb);
}



											/* ---------------------------------------------------------- */



DataTableOfFields::DataTableOfFields(Base4D *xdb, Table* xcrit):DataTableSystem(xdb, xcrit)
{
	fFields_inited = false;
	fFirstTrou = -1;
	fError = VE_OK;
}


DataTableOfFields::~DataTableOfFields()
{
}


VError DataTableOfFields::InitFieldRec()
{
	if (!fFields_inited && fError == VE_OK)
	{
		StLockerWrite lock(this);

		VError err = db->InitDataFileOfFields();

		if (err == VE_OK)
		{
			fFields_inited = true;
		}
		else
		{
			fFields.clear();
			fError = err;
			fFields_inited = false;
		}
		return err;
	}
	else
		return fError;
}


VError DataTableOfFields::AddFieldRec(sLONG tablenum, sLONG fieldnum)
{
	if ( tablenum == 0 )
		return VE_OK;

	VError err = VE_OK;
	StLockerWrite lock(this);
	bool exists = false;

	FieldRec fr(tablenum, fieldnum);
	if (fFieldsSorted.find(fr) != fFieldsSorted.end())
		exists = true;

#if 0
	// Verify if all fields have been updated.
	if (!fFields_inited)
	{
		// Do nothing if it already exists in this table.
		for (FieldRecArray::iterator cur = fFields.begin(), end = fFields.end(); cur != end && !exists; cur++)
			exists= (tablenum == cur->fTableNum && fieldnum == cur->fFieldNum);
	}
#endif

	if (!exists)
	{

		fFieldsSorted.insert(fr);
		if (fFirstTrou == -1)
		{
			try
			{
				fFields.push_back(fr);
			}
			catch (...)
			{
				err = ThrowError(memfull, DBaction_SavingRecord);
			}
		}
		else
		{
			sLONG nexttrou = fFields[fFirstTrou].fFieldNum;
			fFields[fFirstTrou] = fr;
			fFirstTrou = nexttrou;
		}

		if (err == VE_OK)
			fFields_inited = false;
	}

	return err;
}


VError DataTableOfFields::DelFieldRec(sLONG tablenum, sLONG fieldnum)
{
	VError err = VE_OK;
	StLockerWrite lock(this);
	for (FieldRecArray::iterator cur = fFields.begin(), end = fFields.end(); cur != end; cur++)
	{
		if (tablenum == cur->fTableNum)
		{
			if (fieldnum == 0 || fieldnum == cur->fFieldNum)
			{
				fFieldsSorted.erase(*cur);

				sLONG pos = cur - fFields.begin();
				cur->fTableNum = -1;
				cur->fFieldNum = fFirstTrou;
				fFirstTrou = pos;
				fFields_inited = false;
			}
		}
	}
	return err;
}



sLONG DataTableOfFields::GetNbRecords(BaseTaskInfo* context, bool lockread)
{
	StLockerRead lock(this);
	if (InitFieldRec() == VE_OK)
	{
#if 0
		sLONG nbrec = (sLONG)fFields.size();
		sLONG cur = fFirstTrou;
		while (cur != -1)
		{
			nbrec--;
			assert(nbrec >= 0);
			cur = fFields[cur].fFieldNum;
		}
#endif
		sLONG nbrec = (sLONG)fFieldsSorted.size();
		return nbrec;
	}
	else
		return 0;
}


sLONG DataTableOfFields::GetMaxRecords(BaseTaskInfo* context) const
{
	StLockerRead lock(this);
	if (((DataTableOfFields*)this)->InitFieldRec() == VE_OK)
	{
		return (sLONG)fFields.size();
	}
	else
		return 0;
}


VError DataTableOfFields::TakeOffBits(Bittab *tb, BaseTaskInfo* context)
{
	StLockerRead lock(this);
	VError err = InitFieldRec();
	if (err == VE_OK)
	{
		sLONG cur = fFirstTrou;
		while (cur != -1)
		{
			err = tb->Clear(cur);
			if (err != VE_OK)
				break;
			cur = fFields[cur].fFieldNum;
		}
	}
	return err;
}



FicheInMem* DataTableOfFields::LoadRecord(sLONG n, VError& err, DB4D_Way_of_Locking HowToLock, BaseTaskInfo* Context, 
										Boolean WithSubRecords, Boolean BitLockOnly, ReadAheadBuffer* buffer, Boolean* outEnoughMem)
{
	StLockerRead lock(this);
	err = InitFieldRec();
	FicheInMemSystem* fic = nil;
	if (err == VE_OK)
	{
		if (n>=0 && n<(sLONG)fFields.size())
		{
			sLONG ntable = fFields[n].fTableNum;
			sLONG nfield = fFields[n].fFieldNum;
			if (ntable > 0)
			{
				Field* ff = db->RetainField(ntable, nfield);
				if (ff != nil)
				{
					TableOfField* tcrit = (TableOfField*)crit;
					fic = new FicheInMemSystem(Context, db, crit, err);
					if (fic == nil)
						err = ThrowError(memfull, DBaction_LoadingRecord);
					else
						fic->SetNumRec(n);

					if (err == VE_OK)
					{
						VString* sname = new VString();
						if (sname == nil)
							err = ThrowError(memfull, DBaction_LoadingRecord);
						if (err == VE_OK)
						{
							ff->GetName(*sname);
							err = fic->SetNthField(tcrit->id_Column_Name, sname);
							if (err != VE_OK)
								delete sname;
						}
					}

					if (err == VE_OK)
					{
						Table* tt = db->RetainTable(ntable);
						if (tt != nil)
						{
							VString* sname = new VString();
							if (sname == nil)
								err = ThrowError(memfull, DBaction_LoadingRecord);
							if (err == VE_OK)
							{
								tt->GetName(*sname);
								err = fic->SetNthField(tcrit->id_Table_Name, sname);
								if (err != VE_OK)
									delete sname;
							}

							if (err == VE_OK)
							{
								VLong8* sID = new VLong8(tt->GetNum());
								if (sID == nil)
									err = ThrowError(memfull, DBaction_LoadingRecord);
								else
								{
									err = fic->SetNthField(tcrit->id_Table_ID, sID);
									if (err != VE_OK)
										delete sID;
								}
							}

							tt->Release();
						}
					}

					if (err == VE_OK)
					{
						VLong8* sID = new VLong8(ff->GetPosInRec());
						if (sID == nil)
							err = ThrowError(memfull, DBaction_LoadingRecord);
						else
						{
							err = fic->SetNthField(tcrit->id_Column_ID, sID);
							if (err != VE_OK)
								delete sID;
						}
					}

					if (err == VE_OK)
					{
						sLONG size = ff->GetTypeSize();
						VLong* slen = new VLong(size);
						if (slen == nil)
							err = ThrowError(memfull, DBaction_LoadingRecord);
						else
						{
							err = fic->SetNthField(tcrit->id_Data_Length, slen);
							if (err != VE_OK)
								delete slen;
						}
					}

					if (err == VE_OK)
					{
						VLong* styp = new VLong(ff->GetTyp());
						if (styp == nil)
							err = ThrowError(memfull, DBaction_LoadingRecord);
						else
						{
							err = fic->SetNthField(tcrit->id_Data_Type, styp);
							if (err != VE_OK)
								delete styp;
						}
					}

					if (err == VE_OK)
					{
						sLONG oldtype = -1;
						switch(ff->GetTyp())
						{
							case VK_BOOLEAN: 
								oldtype = 6;
								break;

							case VK_BYTE:
								oldtype =  -2;
								break;

							case VK_WORD:
								oldtype = 8;
								break;

							case VK_LONG:
								oldtype = 9;
								break;

							case VK_LONG8:
								oldtype = -5;
								break;

							case VK_REAL:
								oldtype = 1;
								break;

							case VK_FLOAT:
								oldtype = -7;
								break;

							case VK_TIME:
								oldtype = 4;
								break;

							case VK_DURATION:
								oldtype = 11;
								break;

							case VK_UUID:
								oldtype = 0;
								break;

							case VK_STRING:
							case VK_STRING_UTF8:
								if (ff->GetLimitingLen() == 0)
									oldtype = 2;
								else
									oldtype = 0;
								break;

							case VK_BLOB:
								oldtype = 30;
								break;

							case VK_IMAGE:
								oldtype = 3;
								break;

							case VK_TEXT:
							case VK_TEXT_UTF8:
								oldtype = 2;
								break;
						}

						VLong* styp = new VLong(oldtype);
						if (styp == nil)
							err = ThrowError(memfull, DBaction_LoadingRecord);
						else
						{
							err = fic->SetNthField(tcrit->id_Old_Data_Type, styp);
							if (err != VE_OK)
								delete styp;
						}
					}

					if (err == VE_OK)
					{
						VBoolean* snullable = new VBoolean(!ff->GetNot_Null());
						if (snullable == nil)
							err = ThrowError(memfull, DBaction_LoadingRecord);
						else
						{
							err = fic->SetNthField(tcrit->id_Nullable, snullable);
							if (err != VE_OK)
								delete snullable;
						}
					}

					ff->Release();
				}
			}
		}
	}

	if (err != VE_OK)
	{
		err = ThrowError(VE_DB4D_CANNOTLOADRECORD, DBaction_LoadingRecord);
		if (fic != nil)
			fic->Release();
		fic = nil;
	}
	return fic;
}



								/* ----------------------------------------------------------------- */


DataTableOfIndexes::DataTableOfIndexes(Base4D *xdb, Table* xcrit):DataTableSystem(xdb, xcrit)
{
	fIndexRefs_inited = false;
	fError = VE_OK;
	fLastindexNum = -1;
}


DataTableOfIndexes::~DataTableOfIndexes()
{
}



VError DataTableOfIndexes::InitIndexRefs()
{
	if (!fIndexRefs_inited && fError == VE_OK)
	{
		StLockerWrite lock(this);
		fIndexRefs_inited = true;

		VError err = db->InitDataFileOfIndexes();

		if (err != VE_OK)
		{
			fIndexRefs.clear();
			fIndexRefs_inited = false;
			fError = err;
		}
		return err;
	}
	else
		return fError;
}


FicheInMem* DataTableOfIndexes::LoadRecord(sLONG n, VError& err, DB4D_Way_of_Locking HowToLock, BaseTaskInfo* Context, Boolean WithSubRecords, 
										Boolean BitLockOnly, ReadAheadBuffer* buffer, Boolean* outEnoughMem)
{
	StLockerRead lock(this);
	err = InitIndexRefs();
	FicheInMemSystem* fic = nil;

	if (err == VE_OK)
	{
		MapOfIndexRef::iterator found = fIndexRefs.find(n);
		if (found != fIndexRefs.end())
		{
			VUUID xid(found->second);
			IndexInfo* ind = db->FindAndRetainIndexByUUID(xid, false, Context);
			if (ind != nil)
			{
				TableOfIndexes* tcrit = (TableOfIndexes*)crit;
				fic = new FicheInMemSystem(Context, db, crit, err);
				if (fic == nil)
					err = ThrowError(memfull, DBaction_LoadingRecord);
				else
					fic->SetNumRec(n);

				if (err == VE_OK)
				{
					VString *sid = new VString();
					if (sid == nil)
						err = ThrowError(memfull, DBaction_LoadingRecord);
					if (err == VE_OK)
					{
						xid.GetString(*sid);
						err = fic->SetNthField(tcrit->id_Index_UUID, sid);
						if (err != VE_OK)
							delete sid;
					}
				}

				if (err == VE_OK)
				{
					VLong8* sindexid = new VLong8();
					if (sindexid == nil)
						err = ThrowError(memfull, DBaction_LoadingRecord);
					if (err == VE_OK)
					{
						sindexid->FromLong(ind->GetPlaceInStruct());
						err = fic->SetNthField(tcrit->id_Index_ID, sindexid);
						if (err != VE_OK)
							delete sindexid;
					}
				}

				if (err == VE_OK)
				{
					VString* sname = new VString();
					if (sname == nil)
						err = ThrowError(memfull, DBaction_LoadingRecord);
					if (err == VE_OK)
					{
						*sname = ind->GetName();
						err = fic->SetNthField(tcrit->id_Index_Name, sname);
						if (err != VE_OK)
							delete sname;
					}
				}

				if (err == VE_OK)
				{
					VLong* stype = new VLong();
					if (stype == nil)
						err = ThrowError(memfull, DBaction_LoadingRecord);
					if (err == VE_OK)
					{
						stype->FromLong(ind->GetHeaderType());
						err = fic->SetNthField(tcrit->id_Index_Type, stype);
						if (err != VE_OK)
							delete stype;
					}
				}

				Table* target = ind->GetTargetTable();
				if (target != nil)
				{
					if (err == VE_OK)
					{
						VLong8* stableid = new VLong8();
						if (stableid == nil)
							err = ThrowError(memfull, DBaction_LoadingRecord);
						if (err == VE_OK)
						{
							stableid->FromLong(target->GetNum());
							err = fic->SetNthField(tcrit->id_Table_ID, stableid);
							if (err != VE_OK)
								delete stableid;
						}
					}

					if (err == VE_OK)
					{
						VString* sname = new VString();
						if (sname == nil)
							err = ThrowError(memfull, DBaction_LoadingRecord);
						if (err == VE_OK)
						{
							target->GetName(*sname);
							err = fic->SetNthField(tcrit->id_Table_Name, sname);
							if (err != VE_OK)
								delete sname;
						}
					}
				}

				if (err == VE_OK)
				{
					VBoolean* cv = new VBoolean(ind->IsUniqueKey());
					if (cv == nil)
						err = ThrowError(memfull, DBaction_LoadingRecord);
					if (err == VE_OK)
					{
						err = fic->SetNthField(tcrit->id_Uniq, cv);
						if (err != VE_OK)
							delete cv;
					}
				}

				ind->Release();
			}
		}
	}

	if (err != VE_OK)
	{
		err = ThrowError(VE_DB4D_CANNOTLOADRECORD, DBaction_LoadingRecord);
		if (fic != nil)
			fic->Release();
		fic = nil;
	}
	return fic;
}


sLONG DataTableOfIndexes::GetNbRecords(BaseTaskInfo* context, bool lockread)
{
	StLockerRead lock(this);
	if (InitIndexRefs() == VE_OK)
		return (sLONG)fIndexRefs.size();
	else
		return 0;
}


sLONG DataTableOfIndexes::GetMaxRecords(BaseTaskInfo* context) const
{
	StLockerRead lock(this);
	if (((DataTableOfIndexes*)this)->InitIndexRefs() == VE_OK)
	{
		sLONG result = 0;
		if (fIndexRefs.size() > 0)
		{
			MapOfIndexRef::const_iterator last = fIndexRefs.end();
			last--;
			result = last->first + 1;
		}
		return result;
	}
	else
		return 0;
}


VError DataTableOfIndexes::TakeOffBits(Bittab *tb, BaseTaskInfo* context)
{
	StLockerRead lock(this);
	VError err = InitIndexRefs();

	assert(tb != nil);
	if (err == VE_OK && tb != nil)
	{
		Bittab dedans;
		for (MapOfIndexRef::iterator cur = fIndexRefs.begin(), end = fIndexRefs.end(); cur != end && err == VE_OK; cur++)
		{
			err = dedans.Set(cur->first);
		}
		if (err == VE_OK)
		{
			err = dedans.Invert();
			if (err == VE_OK)
				err = tb->moins(&dedans);
		}
	}

	return err;
}



VError DataTableOfIndexes::AddIndexRef(IndexInfo* ind)
{
	VError err = VE_OK;
	StLockerWrite lock(this);
	if (fIndexRefs_inited)
	{
		try
		{
			if (ind->GetPlaceInStruct() == -1 && ind->GetDB()->StoredAsXML())
			{
				fLastindexNum--;
				ind->SetPlaceInStruct(fLastindexNum);
			}
			fIndexRefs[ind->GetPlaceInStruct()] = ind->GetID().GetBuffer();
		}
		catch (...)
		{
			err = memfull;
		}
	}

	return err;
}


VError DataTableOfIndexes::DelIndexRef(IndexInfo* ind)
{
	VError err = VE_OK;
	StLockerWrite lock(this);
	if (fIndexRefs_inited)
	{
		try
		{
			fIndexRefs.erase(ind->GetPlaceInStruct());
		}
		catch (...)
		{
			err = memfull;
		}
	}

	return err;
}





								/* ---------------------------------------------------------- */


DataTableOfIndexCols::DataTableOfIndexCols(Base4D *xdb, Table* xcrit):DataTableSystem(xdb, xcrit)
{
	fIndexCols_inited = false;
	fError = VE_OK;
	fFirstTrou = -1;
}


DataTableOfIndexCols::~DataTableOfIndexCols()
{
}



VError DataTableOfIndexCols::InitIndexCols()
{
	if (!fIndexCols_inited && fError == VE_OK)
	{
		StLockerWrite lock(this);
		fIndexCols_inited = true;

		VError err = db->InitDataFileOfIndexCols();

		if (err != VE_OK)
		{
			fIndexCols.clear();
			fIndexCols_inited = false;
			fError = err;
		}
		return err;
	}
	else
		return fError;
}


VError DataTableOfIndexCols::AddIndexCol(IndexInfo* ind, const VUUID& fieldid, sLONG pos)
{
	VError err = VE_OK;
	StLockerWrite lock(this);
	if (fIndexCols_inited)
	{
		{
			IndexColRec fr(ind->GetID().GetBuffer(), fieldid.GetBuffer(), pos);

			if (fFirstTrou == -1)
			{
				try
				{
					fIndexCols.push_back(fr);
				}
				catch (...)
				{
					err = ThrowError(memfull, DBaction_SavingRecord);
				}
			}
			else
			{
				sLONG nexttrou = fIndexCols[fFirstTrou].fPos;
				fIndexCols[fFirstTrou] = fr;
				fFirstTrou = nexttrou;
			}

		}
	}
	return err;
}


VError DataTableOfIndexCols::DelIndexCol(IndexInfo* ind)
{
	VError err = VE_OK;
	StLockerWrite lock(this);
	if (fIndexCols_inited)
	{
		for (IndexColRecArray::iterator cur = fIndexCols.begin(), end = fIndexCols.end(); cur != end; cur++)
		{
			if (ind->GetID().GetBuffer() == cur->fIndexId)
			{
				sLONG pos = cur - fIndexCols.begin();
				cur->fPos = fFirstTrou;
				cur->fIndexId.FromLong(0);
				cur->fFieldId.FromLong(0);
				fFirstTrou = pos;
				
			}
		}
	}
	return err;
}



sLONG DataTableOfIndexCols::GetNbRecords(BaseTaskInfo* context, bool lockread)
{
	StLockerRead lock(this);
	if (InitIndexCols() == VE_OK)
	{
		sLONG nbrec = (sLONG)fIndexCols.size();
		sLONG cur = fFirstTrou;
		while (cur != -1)
		{
			nbrec--;
			assert(nbrec >= 0);
			cur = fIndexCols[cur].fPos;
		}
		return nbrec;
	}
	else
		return 0;
}


sLONG DataTableOfIndexCols::GetMaxRecords(BaseTaskInfo* context) const
{
	StLockerRead lock(this);
	if (((DataTableOfIndexCols*)this)->InitIndexCols() == VE_OK)
	{
		return (sLONG)fIndexCols.size();
	}
	else
		return 0;
}


VError DataTableOfIndexCols::TakeOffBits(Bittab *tb, BaseTaskInfo* context)
{
	StLockerRead lock(this);
	VError err = InitIndexCols();
	if (err == VE_OK)
	{
		sLONG cur = fFirstTrou;
		while (cur != -1)
		{
			err = tb->Clear(cur);
			if (err != VE_OK)
				break;
			cur = fIndexCols[cur].fPos;
		}
	}
	return err;
}



FicheInMem* DataTableOfIndexCols::LoadRecord(sLONG n, VError& err, DB4D_Way_of_Locking HowToLock, BaseTaskInfo* Context, 
										 Boolean WithSubRecords, Boolean BitLockOnly, ReadAheadBuffer* buffer, Boolean* outEnoughMem)
{
	StLockerRead lock(this);
	err = InitIndexCols();
	FicheInMemSystem* fic = nil;
	if (err == VE_OK)
	{
		if (n>=0 && n<(sLONG)fIndexCols.size())
		{
			VUUID xid(fIndexCols[n].fIndexId);
			IndexInfo* ind = nil;
			sLONG xpos = fIndexCols[n].fPos;
			//if (xpos > 0)
			{
				ind = db->FindAndRetainIndexByUUID(xid, false);
			}
			if (ind != nil)
			{

				TableOfIndexCols* tcrit = (TableOfIndexCols*)crit;
				fic = new FicheInMemSystem(Context, db, crit, err);
				if (fic == nil)
					err = ThrowError(memfull, DBaction_LoadingRecord);
				else
					fic->SetNumRec(n);

				if (err == VE_OK)
				{
					VString* sid = new VString();
					if (sid == nil)
						err = ThrowError(memfull, DBaction_LoadingRecord);
					if (err == VE_OK)
					{
						xid.GetString(*sid);
						err = fic->SetNthField(tcrit->id_Index_UUID, sid);
						if (err != VE_OK)
							delete sid;
					}
				}

				if (err == VE_OK)
				{
					VString* sname = new VString();
					if (sname == nil)
						err = ThrowError(memfull, DBaction_LoadingRecord);
					if (err == VE_OK)
					{
						*sname = ind->GetName();
						err = fic->SetNthField(tcrit->id_Index_Name, sname);
						if (err != VE_OK)
							delete sname;
					}
				}


				if (err == VE_OK)
				{
					VLong* cv = new VLong();
					if (cv == nil)
						err = ThrowError(memfull, DBaction_LoadingRecord);
					if (err == VE_OK)
					{
						cv->FromLong(xpos);
						err = fic->SetNthField(tcrit->id_Column_Position, cv);
						if (err != VE_OK)
							delete cv;
					}
				}

				if (err == VE_OK)
				{
					VUUID fieldID(fIndexCols[n].fFieldId);
					Field* cri = db->FindAndRetainFieldRef(fieldID);
					if (cri != nil)
					{
						if (err == VE_OK)
						{
							VString* cv = new VString();
							if (cv == nil)
								err = ThrowError(memfull, DBaction_LoadingRecord);
							if (err == VE_OK)
							{
								cri->GetName(*cv);
								err = fic->SetNthField(tcrit->id_Column_Name, cv);
								if (err != VE_OK)
									delete cv;
							}
						}
						
						if (err == VE_OK)
						{
							VLong8* cv = new VLong8();
							if (cv == nil)
								err = ThrowError(memfull, DBaction_LoadingRecord);
							if (err == VE_OK)
							{
								cv->FromLong(cri->GetPosInRec());
								err = fic->SetNthField(tcrit->id_Column_ID, cv);
								if (err != VE_OK)
									delete cv;
							}
						}
						cri->Release();
					}
				}

				Table* target = ind->GetTargetTable();
				if (target != nil)
				{
					if (err == VE_OK)
					{
						VLong8* stableid = new VLong8();
						if (stableid == nil)
							err = ThrowError(memfull, DBaction_LoadingRecord);
						if (err == VE_OK)
						{
							stableid->FromLong(target->GetNum());
							err = fic->SetNthField(tcrit->id_Table_ID, stableid);
							if (err != VE_OK)
								delete stableid;
						}
					}

					if (err == VE_OK)
					{
						VString* sname = new VString();
						if (sname == nil)
							err = ThrowError(memfull, DBaction_LoadingRecord);
						if (err == VE_OK)
						{
							target->GetName(*sname);
							err = fic->SetNthField(tcrit->id_Table_Name, sname);
							if (err != VE_OK)
								delete sname;
						}
					}
				}

				if (err == VE_OK)
				{
					VLong8* sindexid = new VLong8();
					if (sindexid == nil)
						err = ThrowError(memfull, DBaction_LoadingRecord);
					if (err == VE_OK)
					{
						sindexid->FromLong(ind->GetPlaceInStruct());
						err = fic->SetNthField(tcrit->id_Index_ID, sindexid);
						if (err != VE_OK)
							delete sindexid;
					}
				}


				//ind->ReleaseValid();
				ind->Release();
			}
		}
	}

	if (err != VE_OK)
	{
		err = ThrowError(VE_DB4D_CANNOTLOADRECORD, DBaction_LoadingRecord);
		if (fic != nil)
			fic->Release();
		fic = nil;
	}
	return fic;
}


								/* ---------------------------------------------------------- */



DataTableOfConstraints::DataTableOfConstraints(Base4D *xdb, Table* xcrit):DataTableSystem(xdb, xcrit)
{
	fConstraints_inited = false;
	fError = VE_OK;
	fFirstTrou = -1;
}


DataTableOfConstraints::~DataTableOfConstraints()
{
}


VError DataTableOfConstraints::InitConstraints()
{
	if (!fConstraints_inited && fError == VE_OK)
	{
		StLockerWrite lock(this);
		fConstraints_inited = true;

		VError err = db->InitDataFileOfConstraints();

		if (err != VE_OK)
		{
			fConstraints.clear();
			fConstraints_inited = false;
			fError = err;
		}
		return err;
	}
	else
		return fError;
}


VError DataTableOfConstraints::_AddConstraint(const VUUIDBuffer& id, Boolean isprim)
{
	VError err = VE_OK;
	StLockerWrite lock(this);
	if (fConstraints_inited)
	{
		//VError err = InitFieldRec();
		//if (err == VE_OK)
		{
			ConstraintRec fr(id, isprim);

			if (fFirstTrou == -1)
			{
				try
				{
					fConstraints.push_back(fr);
				}
				catch (...)
				{
					err = ThrowError(memfull, DBaction_SavingRecord);
				}
			}
			else
			{
				sLONG nexttrou = fConstraints[fFirstTrou].next;
				fConstraints[fFirstTrou] = fr;
				fFirstTrou = nexttrou;
			}

		}
	}
	return err;
}


VError DataTableOfConstraints::_DelConstraint(const VUUIDBuffer& id, Boolean isprim)
{
	VError err = VE_OK;
	StLockerWrite lock(this);
	if (fConstraints_inited)
	{
		//VError err = InitFieldRec();
		//if (err == VE_OK)
		{
			for (ConstraintRecArray::iterator cur = fConstraints.begin(), end = fConstraints.end(); cur != end; cur++)
			{
				if (id == cur->fID)
				{
					if (isprim == cur->fIsPrimKey)
					{
						sLONG pos = cur - fConstraints.begin();
						cur->fID.FromLong(0);
						cur->next = fFirstTrou;
						fFirstTrou = pos;
					}
				}
			}
		}
	}
	return err;
}


VError DataTableOfConstraints::AddConstraint(Table* tab)
{
	VUUID id;
	tab->GetUUID(id);
	return _AddConstraint(id.GetBuffer(), true);
}

VError DataTableOfConstraints::DelConstraint(Table* tab)
{
	VUUID id;
	tab->GetUUID(id);
	return _DelConstraint(id.GetBuffer(), true);
}


VError DataTableOfConstraints::AddConstraint(Relation* rel)
{
	return _AddConstraint(rel->GetUUID().GetBuffer(), false);
}


VError DataTableOfConstraints::DelConstraint(Relation* rel)
{
	return _DelConstraint(rel->GetUUID().GetBuffer(), false);
}



sLONG DataTableOfConstraints::GetNbRecords(BaseTaskInfo* context, bool lockread)
{
	StLockerRead lock(this);
	if (InitConstraints() == VE_OK)
	{
		sLONG nbrec = (sLONG)fConstraints.size();
		sLONG cur = fFirstTrou;
		while (cur != -1)
		{
			nbrec--;
			assert(nbrec >= 0);
			cur = fConstraints[cur].next;
		}
		return nbrec;
	}
	else
		return 0;
}


sLONG DataTableOfConstraints::GetMaxRecords(BaseTaskInfo* context) const
{
	StLockerRead lock(this);
	if (((DataTableOfConstraints*)this)->InitConstraints() == VE_OK)
	{
		return (sLONG)fConstraints.size();
	}
	else
		return 0;
}


VError DataTableOfConstraints::TakeOffBits(Bittab *tb, BaseTaskInfo* context)
{
	StLockerRead lock(this);
	VError err = InitConstraints();
	if (err == VE_OK)
	{
		sLONG cur = fFirstTrou;
		while (cur != -1)
		{
			err = tb->Clear(cur);
			if (err != VE_OK)
				break;
			cur = fConstraints[cur].next;
		}
	}
	return err;
}



FicheInMem* DataTableOfConstraints::LoadRecord(sLONG n, VError& err, DB4D_Way_of_Locking HowToLock, BaseTaskInfo* Context, 
											Boolean WithSubRecords, Boolean BitLockOnly, ReadAheadBuffer* buffer, Boolean* outEnoughMem)
{
	StLockerRead lock(this);
	err = InitConstraints();
	FicheInMemSystem* fic = nil;
	if (err == VE_OK)
	{
		if (n>=0 && n<(sLONG)fConstraints.size())
		{
			TableOfConstraints* tcrit = (TableOfConstraints*)crit;
			Boolean isprim = fConstraints[n].fIsPrimKey;
			VUUID xid(fConstraints[n].fID);
			if (isprim)
			{
				Table* tab = db->FindAndRetainTableRef(xid);
				if (tab != nil)
				{
					fic = new FicheInMemSystem(Context, db, crit, err);
					if (fic == nil)
						err = ThrowError(memfull, DBaction_LoadingRecord);
					else
						fic->SetNumRec(n);

					if (err == VE_OK)
					{
						VLong8* cv = new VLong8();
						if (cv == nil)
							err = ThrowError(memfull, DBaction_LoadingRecord);
						if (err == VE_OK)
						{
							cv->FromLong(tab->GetNum());
							err = fic->SetNthField(tcrit->id_Table_ID, cv);
							if (err != VE_OK)
								delete cv;
						}
					}

					if (err == VE_OK)
					{
						VString* cv = new VString();
						if (cv == nil)
							err = ThrowError(memfull, DBaction_LoadingRecord);
						if (err == VE_OK)
						{
							tab->GetName(*cv);
							err = fic->SetNthField(tcrit->id_Table_Name, cv);
							if (err != VE_OK)
								delete cv;
						}
					}

					if (err == VE_OK)
					{
						VString* cv = new VString();
						if (cv == nil)
							err = ThrowError(memfull, DBaction_LoadingRecord);
						if (err == VE_OK)
						{
							xid.GetString(*cv);
							err = fic->SetNthField(tcrit->id_Constraint_ID, cv);
							if (err != VE_OK)
								delete cv;
						}
					}

					if (err == VE_OK)
					{
						VString* cv = new VString();
						if (cv == nil)
							err = ThrowError(memfull, DBaction_LoadingRecord);
						if (err == VE_OK)
						{
							*cv = tab->GetPrimaryKeyName();
							err = fic->SetNthField(tcrit->id_Constraint_Name, cv);
							if (err != VE_OK)
								delete cv;
						}
					}

					if (err == VE_OK)
					{
						VString* cv = new VString();
						if (cv == nil)
							err = ThrowError(memfull, DBaction_LoadingRecord);
						if (err == VE_OK)
						{
							*cv = L"P";
							err = fic->SetNthField(tcrit->id_Constraint_Type, cv);
							if (err != VE_OK)
								delete cv;
						}
					}

					tab->Release();
				}
			}
			else
			{
				Relation* rel = db->FindAndRetainRelationByUUID(xid);
				if (rel != nil)
				{
					Table* tab = rel->GetSource()->GetOwner();

					fic = new FicheInMemSystem(Context, db, crit, err);
					if (fic == nil)
						err = ThrowError(memfull, DBaction_LoadingRecord);
					else
						fic->SetNumRec(n);

					if (err == VE_OK)
					{
						VLong8* cv = new VLong8();
						if (cv == nil)
							err = ThrowError(memfull, DBaction_LoadingRecord);
						if (err == VE_OK)
						{
							cv->FromLong(tab->GetNum());
							err = fic->SetNthField(tcrit->id_Table_ID, cv);
							if (err != VE_OK)
								delete cv;
						}
					}

					if (err == VE_OK)
					{
						VString* cv = new VString();
						if (cv == nil)
							err = ThrowError(memfull, DBaction_LoadingRecord);
						if (err == VE_OK)
						{
							tab->GetName(*cv);
							err = fic->SetNthField(tcrit->id_Table_Name, cv);
							if (err != VE_OK)
								delete cv;
						}
					}

					if (err == VE_OK)
					{
						VString* cv = new VString();
						if (cv == nil)
							err = ThrowError(memfull, DBaction_LoadingRecord);
						if (err == VE_OK)
						{
							xid.GetString(*cv);
							err = fic->SetNthField(tcrit->id_Constraint_ID, cv);
							if (err != VE_OK)
								delete cv;
						}
					}

					if (err == VE_OK)
					{
						VString* cv = new VString();
						if (cv == nil)
							err = ThrowError(memfull, DBaction_LoadingRecord);
						if (err == VE_OK)
						{
							*cv = rel->GetName();
							err = fic->SetNthField(tcrit->id_Constraint_Name, cv);
							if (err != VE_OK)
								delete cv;
						}
					}

					if (err == VE_OK)
					{
						VString* cv = new VString();
						if (cv == nil)
							err = ThrowError(memfull, DBaction_LoadingRecord);
						if (err == VE_OK)
						{
							if (rel->IsForeignKey())
								*cv = L"R";
							else
								*cv = L"4DR";
							err = fic->SetNthField(tcrit->id_Constraint_Type, cv);
							if (err != VE_OK)
								delete cv;
						}
					}

					if (err == VE_OK)
					{
						VString* cv = new VString();
						if (cv == nil)
							err = ThrowError(memfull, DBaction_LoadingRecord);
						if (err == VE_OK)
						{
							rel->GetDest()->GetOwner()->GetName(*cv);
							err = fic->SetNthField(tcrit->id_Related_Table, cv);
							if (err != VE_OK)
								delete cv;
						}
					}

					if (err == VE_OK)
					{
						VLong8* cv = new VLong8();
						if (cv == nil)
							err = ThrowError(memfull, DBaction_LoadingRecord);
						if (err == VE_OK)
						{
							cv->FromLong(rel->GetDest()->GetOwner()->GetNum());
							err = fic->SetNthField(tcrit->id_Related_Table_ID, cv);
							if (err != VE_OK)
								delete cv;
						}
					}

					if (err == VE_OK)
					{
						VString* cv = new VString();
						if (cv == nil)
							err = ThrowError(memfull, DBaction_LoadingRecord);
						if (err == VE_OK)
						{
							if (rel->WithReferentialIntegrity())
							{
								if (rel->AutoDeleteRelatedRecords())
								{
									*cv = L"CASCADE";
								}
								else
									*cv = L"RESTRICT";
								{
								}
							}
							else
								*cv = L"";
							err = fic->SetNthField(tcrit->id_Delete_Rule, cv);
							if (err != VE_OK)
								delete cv;
						}
					}

					rel->Release();
				}
			}
		}
	}

	if (err != VE_OK)
	{
		err = ThrowError(VE_DB4D_CANNOTLOADRECORD, DBaction_LoadingRecord);
		if (fic != nil)
			fic->Release();
		fic = nil;
	}
	return fic;
}



								/* ---------------------------------------------------------- */


FicheInMem* DataTableOfConstraintCols::LoadRecord(sLONG n, VError& err, DB4D_Way_of_Locking HowToLock, BaseTaskInfo* Context, 
											  Boolean WithSubRecords, Boolean BitLockOnly, ReadAheadBuffer* buffer, Boolean* outEnoughMem)
{
	StLockerRead lock(this);
	err = InitConstraints();
	FicheInMemSystem* fic = nil;
	if (err == VE_OK)
	{
		if (n>=0 && n<(sLONG)fConstraints.size())
		{
			TableOfConstraintCols* tcrit = (TableOfConstraintCols*)crit;
			Boolean isprim = fConstraints[n].fIsPrimKey;
			VUUID xid(fConstraints[n].fID);
			if (isprim)
			{
				Table* tab = db->FindAndRetainTableRef(xid);
				if (tab != nil)
				{
					fic = new FicheInMemSystem(Context, db, crit, err);
					if (fic == nil)
						err = ThrowError(memfull, DBaction_LoadingRecord);
					else
						fic->SetNumRec(n);

					if (err == VE_OK)
					{
						VLong8* cv = new VLong8();
						if (cv == nil)
							err = ThrowError(memfull, DBaction_LoadingRecord);
						if (err == VE_OK)
						{
							cv->FromLong(tab->GetNum());
							err = fic->SetNthField(tcrit->id_Table_ID, cv);
							if (err != VE_OK)
								delete cv;
						}
					}

					if (err == VE_OK)
					{
						VString* cv = new VString();
						if (cv == nil)
							err = ThrowError(memfull, DBaction_LoadingRecord);
						if (err == VE_OK)
						{
							tab->GetName(*cv);
							err = fic->SetNthField(tcrit->id_Table_Name, cv);
							if (err != VE_OK)
								delete cv;
						}
					}

					if (err == VE_OK)
					{
						VString* cv = new VString();
						if (cv == nil)
							err = ThrowError(memfull, DBaction_LoadingRecord);
						if (err == VE_OK)
						{
							*cv = tab->GetPrimaryKeyName();
							err = fic->SetNthField(tcrit->id_Constraint_Name, cv);
							if (err != VE_OK)
								delete cv;
						}
					}

					tab->Release();
				}
			}
			else
			{
				Relation* rel = db->FindAndRetainRelationByUUID(xid);
				if (rel != nil)
				{
					Table* tab = rel->GetSource()->GetOwner();

					fic = new FicheInMemSystem(Context, db, crit, err);
					if (fic == nil)
						err = ThrowError(memfull, DBaction_LoadingRecord);
					else
						fic->SetNumRec(n);

					if (err == VE_OK)
					{
						VLong8* cv = new VLong8();
						if (cv == nil)
							err = ThrowError(memfull, DBaction_LoadingRecord);
						if (err == VE_OK)
						{
							cv->FromLong(tab->GetNum());
							err = fic->SetNthField(tcrit->id_Table_ID, cv);
							if (err != VE_OK)
								delete cv;
						}
					}

					if (err == VE_OK)
					{
						VString* cv = new VString();
						if (cv == nil)
							err = ThrowError(memfull, DBaction_LoadingRecord);
						if (err == VE_OK)
						{
							tab->GetName(*cv);
							err = fic->SetNthField(tcrit->id_Table_Name, cv);
							if (err != VE_OK)
								delete cv;
						}
					}

					if (err == VE_OK)
					{
						VString* cv = new VString();
						if (cv == nil)
							err = ThrowError(memfull, DBaction_LoadingRecord);
						if (err == VE_OK)
						{
							*cv = rel->GetName();
							err = fic->SetNthField(tcrit->id_Constraint_Name, cv);
							if (err != VE_OK)
								delete cv;
						}
					}

					rel->Release();
				}
			}

			if (fic != nil)
			{

				if (err == VE_OK)
				{
					VLong* cv = new VLong();
					if (cv == nil)
						err = ThrowError(memfull, DBaction_LoadingRecord);
					if (err == VE_OK)
					{
						cv->FromLong(fConstraints[n].fPos);
						err = fic->SetNthField(tcrit->id_Position, cv);
						if (err != VE_OK)
							delete cv;
					}
				}

				if (err == VE_OK)
				{
					VString* cv = new VString();
					if (cv == nil)
						err = ThrowError(memfull, DBaction_LoadingRecord);
					if (err == VE_OK)
					{
						xid.GetString(*cv);
						err = fic->SetNthField(tcrit->id_Constraint_ID, cv);
						if (err != VE_OK)
							delete cv;
					}
				}

				VUUID fieldid(fConstraints[n].fFieldID);
				Field* cri = db->FindAndRetainFieldRef(fieldid);
				if (cri != nil)
				{
					if (err == VE_OK)
					{
						VString* cv = new VString();
						if (cv == nil)
							err = ThrowError(memfull, DBaction_LoadingRecord);
						if (err == VE_OK)
						{
							cri->GetName(*cv);
							err = fic->SetNthField(tcrit->id_Column_Name, cv);
							if (err != VE_OK)
								delete cv;
						}
					}

					if (err == VE_OK)
					{
						VLong8* cv = new VLong8();
						if (cv == nil)
							err = ThrowError(memfull, DBaction_LoadingRecord);
						if (err == VE_OK)
						{
							cv->FromLong(cri->GetPosInRec());
							err = fic->SetNthField(tcrit->id_Column_ID, cv);
							if (err != VE_OK)
								delete cv;
						}
					}

					cri->Release();
				}

				VUUID relatedfieldid(fConstraints[n].fRelatedID);
				cri = db->FindAndRetainFieldRef(relatedfieldid);
				if (cri != nil)
				{
					if (err == VE_OK)
					{
						VString* cv = new VString();
						if (cv == nil)
							err = ThrowError(memfull, DBaction_LoadingRecord);
						if (err == VE_OK)
						{
							cri->GetName(*cv);
							err = fic->SetNthField(tcrit->id_Related_Column_Name, cv);
							if (err != VE_OK)
								delete cv;
						}
					}

					if (err == VE_OK)
					{
						VLong* cv = new VLong();
						if (cv == nil)
							err = ThrowError(memfull, DBaction_LoadingRecord);
						if (err == VE_OK)
						{
							cv->FromLong(cri->GetPosInRec());
							err = fic->SetNthField(tcrit->id_Related_Column_ID, cv);
							if (err != VE_OK)
								delete cv;
						}
					}

					cri->Release();
				}

			}
		}
	}

	if (err != VE_OK)
	{
		err = ThrowError(VE_DB4D_CANNOTLOADRECORD, DBaction_LoadingRecord);
		if (fic != nil)
			fic->Release();
		fic = nil;
	}
	return fic;
}


DataTableOfConstraintCols::DataTableOfConstraintCols(Base4D *xdb, Table* xcrit):DataTableSystem(xdb, xcrit)
{
	fConstraints_inited = false;
	fError = VE_OK;
	fFirstTrou = -1;
}


DataTableOfConstraintCols::~DataTableOfConstraintCols()
{
}


VError DataTableOfConstraintCols::InitConstraints()
{
	if (!fConstraints_inited && fError == VE_OK)
	{
		StLockerWrite lock(this);
		fConstraints_inited = true;

		VError err = db->InitDataFileOfConstraintCols();

		if (err != VE_OK)
		{
			fConstraints.clear();
			fConstraints_inited = false;
			fError = err;
		}
		return err;
	}
	else
		return fError;
}


VError DataTableOfConstraintCols::_AddConstraintCol(const VUUIDBuffer& id, Boolean isprim, const VUUIDBuffer& fieldid, sLONG pos, const VUUIDBuffer& relatedid)
{
	VError err = VE_OK;
	StLockerWrite lock(this);
	if (fConstraints_inited)
	{
		//VError err = InitFieldRec();
		//if (err == VE_OK)
		{
			ConstraintColRec fr(id, isprim, fieldid, pos, relatedid);

			if (fFirstTrou == -1)
			{
				try
				{
					fConstraints.push_back(fr);
				}
				catch (...)
				{
					err = ThrowError(memfull, DBaction_SavingRecord);
				}
			}
			else
			{
				sLONG nexttrou = fConstraints[fFirstTrou].next;
				fConstraints[fFirstTrou] = fr;
				fFirstTrou = nexttrou;
			}

		}
	}
	return err;
}


VError DataTableOfConstraintCols::_DelConstraintCol(const VUUIDBuffer& id, Boolean isprim)
{
	VError err = VE_OK;
	StLockerWrite lock(this);
	if (fConstraints_inited)
	{
		//VError err = InitFieldRec();
		//if (err == VE_OK)
		{
			for (ConstraintColRecArray::iterator cur = fConstraints.begin(), end = fConstraints.end(); cur != end; cur++)
			{
				if (id == cur->fID)
				{
					if (isprim == cur->fIsPrimKey)
					{
						sLONG pos = cur - fConstraints.begin();
						cur->fID.FromLong(0);
						cur->fFieldID.FromLong(0);
						cur->fPos = -1;
						cur->next = fFirstTrou;
						fFirstTrou = pos;
					}
				}
			}
		}
	}
	return err;
}


VError DataTableOfConstraintCols::AddConstraintCol(Table* tab, Field* cri, sLONG pos)
{
	VUUID id;
	tab->GetUUID(id);
	VUUID fieldid, emptyid;
	cri->GetUUID(fieldid);
	emptyid.FromLong(0);
	return _AddConstraintCol(id.GetBuffer(), true, fieldid.GetBuffer(), pos, emptyid.GetBuffer());
}

VError DataTableOfConstraintCols::DelConstraintCol(Table* tab)
{
	VUUID id;
	tab->GetUUID(id);
	return _DelConstraintCol(id.GetBuffer(), true);
}


VError DataTableOfConstraintCols::AddConstraintCol(Relation* rel, Field* cri, sLONG pos, Field* relatedcri)
{
	VUUID fieldid, relatedid;
	cri->GetUUID(fieldid);
	relatedcri->GetUUID(relatedid);
	return _AddConstraintCol(rel->GetUUID().GetBuffer(), false, fieldid.GetBuffer(), pos, relatedid.GetBuffer());
}


VError DataTableOfConstraintCols::DelConstraintCol(Relation* rel)
{
	return _DelConstraintCol(rel->GetUUID().GetBuffer(), false);
}



sLONG DataTableOfConstraintCols::GetNbRecords(BaseTaskInfo* context, bool lockread)
{
	StLockerRead lock(this);
	if (InitConstraints() == VE_OK)
	{
		sLONG nbrec = (sLONG)fConstraints.size();
		sLONG cur = fFirstTrou;
		while (cur != -1)
		{
			nbrec--;
			assert(nbrec >= 0);
			cur = fConstraints[cur].next;
		}
		return nbrec;
	}
	else
		return 0;
}


sLONG DataTableOfConstraintCols::GetMaxRecords(BaseTaskInfo* context) const
{
	StLockerRead lock(this);
	if (((DataTableOfConstraintCols*)this)->InitConstraints() == VE_OK)
	{
		return (sLONG)fConstraints.size();
	}
	else
		return 0;
}


VError DataTableOfConstraintCols::TakeOffBits(Bittab *tb, BaseTaskInfo* context)
{
	StLockerRead lock(this);
	VError err = InitConstraints();
	if (err == VE_OK)
	{
		sLONG cur = fFirstTrou;
		while (cur != -1)
		{
			err = tb->Clear(cur);
			if (err != VE_OK)
				break;
			cur = fConstraints[cur].next;
		}
	}
	return err;
}



						/* ---------------------------------------------------------- */





DataTableOfSchemas::DataTableOfSchemas(Base4D *xdb, Table* xcrit):DataTableSystem(xdb, xcrit)
{

}


DataTableOfSchemas::~DataTableOfSchemas()
{
}


FicheInMem* DataTableOfSchemas::LoadRecord(sLONG n, VError& err, DB4D_Way_of_Locking HowToLock, BaseTaskInfo* Context, Boolean WithSubRecords,
									   Boolean BitLockOnly, ReadAheadBuffer* buffer, Boolean* outEnoughMem)
{
	err = VE_OK;
	FicheInMemSystem* fic = nil;

	std::vector<VRefPtr<CDB4DSchema> >		vctrSchemas; /* Self-releases all in the end of the method */
	err = db->RetainAllSchemas(vctrSchemas);
	if (err == VE_OK && vctrSchemas.size() >= n)
	{
		TableOfSchemas* tcrit = (TableOfSchemas*)crit;
		fic = new FicheInMemSystem(Context, db, crit, err);
		if (fic == nil)
			err = ThrowError(memfull, DBaction_LoadingRecord);
		else
			fic->SetNumRec(n);

		if (err == VE_OK)
		{
			VString* sname = new VString();
			if (sname == nil)
				err = ThrowError(memfull, DBaction_LoadingRecord);
			if (err == VE_OK)
			{
				const VString&	vstrName = vctrSchemas [n]->GetName();
				sname->FromString(vstrName);
				err = fic->SetNthField(tcrit->id_Schema_Name, sname);
				if (err != VE_OK)
					delete sname;
			}
		}

		if (err == VE_OK)
		{
			VLong* sID = new VLong(vctrSchemas[n]->GetID());
			if (sID == nil)
				err = ThrowError(memfull, DBaction_LoadingRecord);
			else
			{
				err = fic->SetNthField(tcrit->id_Schema_ID, sID);
				if (err != VE_OK)
					delete sID;
			}
		}

		IDB4D_ApplicationIntf *applicationIntf = VDBMgr::GetManager()->GetApplicationInterface();
		if (err == VE_OK && applicationIntf != nil)
		{
			sLONG			outReadGroupID, outReadWriteGroupID, outAllGroupID;
			XBOX::VString	outReadGroupName, outReadWriteGroupName, outAllGroupName;
			applicationIntf->DescribePermissionsForSchema (
													vctrSchemas[n]->GetID(),
													outReadGroupID, outReadGroupName,
													outReadWriteGroupID, outReadWriteGroupName,
													outAllGroupID, outAllGroupName );
			if (err == VE_OK)
			{
				VLong* sRID = new VLong(outReadGroupID);
				if (sRID == nil)
					err = ThrowError(memfull, DBaction_LoadingRecord);
				else
				{
					err = fic->SetNthField(tcrit->id_Read_Group_ID, sRID);
					if (err != VE_OK)
						delete sRID;
				}
			}

			if (err == VE_OK)
			{
				VString* sRname = new VString();
				if (sRname == nil)
					err = ThrowError(memfull, DBaction_LoadingRecord);
				if (err == VE_OK)
				{
					sRname->FromString(outReadGroupName);
					err = fic->SetNthField(tcrit->id_Read_Group_Name, sRname);
					if (err != VE_OK)
						delete sRname;
				}
			}

			if (err == VE_OK)
			{
				VLong* sRWID = new VLong(outReadWriteGroupID);
				if (sRWID == nil)
					err = ThrowError(memfull, DBaction_LoadingRecord);
				else
				{
					err = fic->SetNthField(tcrit->id_Read_Write_Group_ID, sRWID);
					if (err != VE_OK)
						delete sRWID;
				}
			}

			if (err == VE_OK)
			{
				VString* sRWname = new VString();
				if (sRWname == nil)
					err = ThrowError(memfull, DBaction_LoadingRecord);
				if (err == VE_OK)
				{
					sRWname->FromString(outReadWriteGroupName);
					err = fic->SetNthField(tcrit->id_Read_Write_Group_Name, sRWname);
					if (err != VE_OK)
						delete sRWname;
				}
			}

			if (err == VE_OK)
			{
				VLong* sAllID = new VLong(outAllGroupID);
				if (sAllID == nil)
					err = ThrowError(memfull, DBaction_LoadingRecord);
				else
				{
					err = fic->SetNthField(tcrit->id_All_Group_ID, sAllID);
					if (err != VE_OK)
						delete sAllID;
				}
			}

			if (err == VE_OK)
			{
				VString* sAllname = new VString();
				if (sAllname == nil)
					err = ThrowError(memfull, DBaction_LoadingRecord);
				if (err == VE_OK)
				{
					sAllname->FromString(outAllGroupName);
					err = fic->SetNthField(tcrit->id_All_Group_Name, sAllname);
					if (err != VE_OK)
						delete sAllname;
				}
			}

		}
	}

	if (err != VE_OK)
	{
		err = ThrowError(VE_DB4D_CANNOTLOADRECORD, DBaction_LoadingRecord);
		if (fic != nil)
			fic->Release();
		fic = nil;
	}

	return fic;
}


sLONG DataTableOfSchemas::GetNbRecords(BaseTaskInfo* context, bool lockread)
{
	db->BuildFirstSchemaIfNeeded();
	return GetMaxRecords(context);
}


sLONG DataTableOfSchemas::GetMaxRecords(BaseTaskInfo* context) const
{
	db->BuildFirstSchemaIfNeeded();
	return db->CountSchemas();
}


VError DataTableOfSchemas::TakeOffBits(Bittab *tb, BaseTaskInfo* context)
{
	return VE_OK;
}



						/* ---------------------------------------------------------- */





DataFile_NotOpened::DataFile_NotOpened(Base4D_NotOpened* xbd, DataAddr4D addr, sLONG numtable)
{
	bd = xbd;
	fAddr = addr;
	fNumTable = numtable;
	fNumTableDef = 0;
	fIsLoaded = false;
	fBlockIsValid = false;
	fBlockIsFullyValid = false;
	fNbFicIsValid = false;
	fTabRecAddrIsValid = false;
	fTabTrouRecIsValid = false;
	fNbBlobIsValid = false;
	fTabBlobAddrIsValid = false;
	fTabTrouBlobIsValid = false;
	fSeqIDIsValid = false;
	fNbRecords = -1;

	fBlobsHaveBeenChecked = false;
	fBlobsTrouHaveBeenChecked = false;
	fBlobsOrphanHaveBeenChecked = false;
	fBlobMapIsValid = false;
	fBlobsInMap = nil;
	fBlobsInRec = nil;
	fBlobsDeleted = nil;
	fBlobsOrphan = nil;

	fRecordsHaveBeenChecked = false;
	fRecordsTrouHaveBeenChecked = false;
	fRecordMapIsValid = false;
	//fRecordsInMap = nil;
	fRecordsDeleted = nil;

	fTempBlobsDeleted = nil;
	fTempRecordsDeleted = nil;

	fTempBlobAddrCache = nil;
	fTempRecAddrCache = nil;

	fWithStamps = false;

	libere();
}


DataFile_NotOpened::~DataFile_NotOpened()
{
	if (fTempBlobAddrCache != nil)
		delete fTempBlobAddrCache;
	if (fTempRecAddrCache != nil)
		delete fTempRecAddrCache;
	ReleaseRefCountable(&fBlobsInRec);
	ReleaseRefCountable(&fBlobsInMap);
	ReleaseRefCountable(&fBlobsDeleted);
	ReleaseRefCountable(&fBlobsOrphan);
	ReleaseRefCountable(&fRecordsDeleted);
}


VError DataFile_NotOpened::Load(ToolLog* log)
{
	VError errexec = VE_OK;
	if (!fIsLoaded)
	{
		fIsLoaded = true;

		DataBaseObjectHeader tag;
		VError err = tag.ReadFrom(bd, fAddr);
		if (err != VE_OK)
		{
			DataTableProblem pb(TOP_DataTableHeader_PhysicalDataIsInvalid, fNumTable);
			errexec = log->Add(pb);
		}

		if (err == VE_OK && errexec == VE_OK)
		{
			err = tag.ValidateTag(DBOH_DataTable, fNumTable, -3);
			if (err != VE_OK)
			{
				DataTableProblem pb(TOP_DataTableHeader_TagIsInvalid, fNumTable);
				errexec = log->Add(pb);
			}
		}

		if (err == VE_OK && errexec == VE_OK)
		{
			err = bd->readlong(&DFD, sizeof(DFD), fAddr, kSizeDataBaseObjectHeader);
			if (err == VE_OK)
			{
				err = tag.ValidateCheckSum(&DFD, sizeof(DFD));
				if (err == VE_OK)
				{
					if (tag.NeedSwap())
						DFD.SwapBytes();
				}
				else
				{
					DataTableProblem pb(TOP_DataTableHeader_PhysicalDataIsInvalid, fNumTable);
					errexec = log->Add(pb);
				}
			}
			else
			{
				DataTableProblem pb(TOP_DataTableHeader_PhysicalDataIsInvalid, fNumTable);
				errexec = log->Add(pb);
			}
		}

		if (err == VE_OK && errexec == VE_OK)
		{
			fWithStamps = DFD.fKeepStamps;
			fBlockIsValid = true;
			fNumTableDef = -1;

			if (!bd->ExistUUID(DFD.TableDefID, tuuid_Table, nil, nil, &fNumTableDef))
			{
				// ## a faire : chercher dans les TableDef dans data
			}

			if (fNumTableDef>0)
			{
				bd->SetDataTableByTableRefNum(this, fNumTableDef);
			}

			if (DFD.nbfic >= 0 && DFD.nbfic <= kMaxRecordsInTable)
				fNbFicIsValid = true;
			else
			{
				DataTableProblem pb(TOP_Table_NBficIsInvalid, fNumTable, fNumTableDef);
				errexec = log->Add(pb);
			}

			if (errexec == VE_OK)
			{
				if (DFD.addrtabaddr == 0 || bd->IsAddrValid(DFD.addrtabaddr))
					fTabRecAddrIsValid = true;
				else
				{
					DataTableProblem pb(TOP_Table_AddrOfPrimaryTableOfRecordsIsInvalid, fNumTable, fNumTableDef);
					errexec = log->Add(pb);
				}
			}

			if (errexec == VE_OK)
			{
				if ( (DFD.debuttrou <= 0 && (-DFD.debuttrou) <= kMaxRecordsInTable) || DFD.debuttrou == kFinDesTrou )
					fTabTrouRecIsValid = true;
				else
				{
					DataTableProblem pb(TOP_Table_ListOfDeletedRecordsIsInvalid, fNumTable, fNumTableDef);
					errexec = log->Add(pb);
				}
			}
				
			if (errexec == VE_OK)
			{
				if (DFD.nbBlob >= 0 && DFD.nbBlob <= kMaxRecordsInTable)
					fNbBlobIsValid = true;
				else
				{
					DataTableProblem pb(TOP_Table_NBblobsIsInvalid, fNumTable, fNumTableDef);
					errexec = log->Add(pb);
				}
			}

			if (errexec == VE_OK)
			{
				if (DFD.addrBlobtabaddr == 0 || bd->IsAddrValid(DFD.addrBlobtabaddr))
					fTabBlobAddrIsValid = true;
				else
				{
					DataTableProblem pb(TOP_Table_AddrOfPrimaryTableOfBlobsIsInvalid, fNumTable, fNumTableDef);
					errexec = log->Add(pb);
				}
			}

			if (errexec == VE_OK)
			{
				if ( (DFD.debutBlobTrou <= 0 && (-DFD.debutBlobTrou) <= kMaxBlobsInTable) || DFD.debutBlobTrou == kFinDesTrou )
					fTabTrouBlobIsValid = true;
				else
				{
					DataTableProblem pb(TOP_Table_ListOfDeletedBlobsIsInvalid, fNumTable, fNumTableDef);
					errexec = log->Add(pb);
				}
			}

			fBlockIsFullyValid = fNbFicIsValid && fTabRecAddrIsValid && fTabTrouRecIsValid && fNbBlobIsValid && fTabBlobAddrIsValid  && fTabTrouBlobIsValid;

			if (errexec == VE_OK)
			{
				if ( *((uLONG8*) &DFD.SeqNum_ID.fBytes[0]) == 0 && *((uLONG8*) &DFD.SeqNum_ID.fBytes[8]) == 0 )
					fSeqIDIsValid = true;
				else
				{
					if (!bd->ExistUUID(DFD.SeqNum_ID, tuuid_AutoSeqNum, nil))
					{
						DataTableProblem pb(TOP_SeqNumIDIsInvalid, fNumTable, fNumTableDef);
						errexec = log->Add(pb);
						Base4D* target = nil;
						DataTable* df = nil;
						if (log->IsCompacting())
						{
							target = log->GetTargetCompact();
							if (bd->GetStruct() == nil)
								target = target->GetStructure();
							df = target->RetainDataTableByUUID(DFD.TableDefID);
							if (df != nil)
							{
								errexec = df->ResetSeqNum();
							}
						}
					}
				}
			}

		}
	}
	return errexec;
}


VError DataFile_NotOpened::CheckOneBlob(DataAddr4D ou, sLONG len, sLONG numblob, ToolLog* log)
{
	VError errexec = VE_OK;
	VError err = VE_OK;

	if (bd->IsAddrValid(ou))
	{
		log->MarkAddr_Blob(ou, len + kSizeDataBaseObjectHeader, fNumTable, numblob);

		DataBaseObjectHeader tag;
		err = tag.ReadFrom(bd, ou);
		if (err != VE_OK)
		{
			BlobProblem pb(TOP_BlobPhysicalDataIsInvalid, fNumTable, numblob);
			errexec = log->Add(pb);
		}

		DataBaseObjectType whichtype = DBOH_Blob;

		if (err == VE_OK && errexec == VE_OK)
		{
			err = tag.ValidateTag(whichtype,numblob, fNumTable);
			if (err != VE_OK)
			{
				whichtype = DBOH_BlobText;
				err = tag.ValidateTag(whichtype, numblob, fNumTable);
			}
			if (err != VE_OK)
			{
				whichtype = DBOH_BlobPict;
				err = tag.ValidateTag(whichtype, numblob, fNumTable);
			}
			if (err != VE_OK)
			{
				BlobProblem pb(TOP_BlobTagIsInvalid, fNumTable, numblob);
				errexec = log->Add(pb);
			}
		}

		sLONG lll;
		if (err == VE_OK && errexec == VE_OK)
		{
			err=bd->readlong(&lll, 4, ou, kSizeDataBaseObjectHeader);
			if (err != VE_OK)
			{
				BlobProblem pb(TOP_BlobPhysicalDataIsInvalid, fNumTable, numblob);
				errexec = log->Add(pb);
			}
		}
		if (err == VE_OK && errexec == VE_OK)
		{
			if (tag.NeedSwap())
				lll = SwapLong(lll);
		}

		if (err == VE_OK && errexec == VE_OK)
		{
			if (lll>0)
			{
				bool ismalloc = false;
				void* p = GetFastMem(lll, false, 'blbT');
				if(p==nil)//[MI] le 05/10/2009 ACI0062909 : la memoire peut etre fragmenter, GetFastMem echoue.
				{
					p=malloc(lll);
					ismalloc = true;
				}

				if (p!=nil)
				{
					err=bd->readlong( p, lll, ou, kSizeDataBaseObjectHeader+4);
					if (err != VE_OK)
					{
						BlobProblem pb(TOP_BlobPhysicalDataIsInvalid, fNumTable, numblob);
						errexec = log->Add(pb);
					}

					if (err == VE_OK && errexec == VE_OK)
					{
						err = tag.ValidateCheckSum(p, lll);
						if (err != VE_OK)
						{
							BlobProblem pb(TOP_BlobCheckSumIsInvalid, fNumTable, numblob);
							errexec = log->Add(pb);
						}
					}

					if (err == VE_OK && errexec == VE_OK)
					{
						if (log->IsCompacting())
						{
							DataTable* df = log->GetTargetDataTableCompact();
							if (df != nil)
							{
								if (tag.NeedSwap())
								{
									if (whichtype == DBOH_BlobText)
									{
										ByteSwap((UniChar *) p, lll/2);
									}
								}
								OccupableStack* curstack = gOccPool.GetStackForCurrentThread();

								err = df->SaveRawBlob(curstack, whichtype,numblob, p, lll, nil);
							}
						}
						else
						{
							if (adjuste(lll+4) != adjuste(len))
							{
								BlobProblem pb(TOP_BlobLengthDoesNotMatchTabAddr, fNumTable, numblob);
								errexec = log->Add(pb);
								err = VE_DB4D_CANNOTLOADBLOB;
							}
						}
					}

					if(ismalloc)
						free(p);
					else
						FreeFastMem(p);
				}
				else
					errexec = memfull;
			}
			else
			{
				BlobProblem pb(TOP_BlobLengthIsInvalid, fNumTable, numblob);
				errexec = log->Add(pb);
			}
		}
	}
	else
	{
		BlobProblem pb(TOP_BlobAddrIsInvalid, fNumTable, numblob);
		errexec = log->Add(pb);
		fBlobMapIsValid = false;
		err = VE_DB4D_CANNOTLOADBLOB;
	}

	if (errexec == VE_OK && err == VE_OK )
	{
		errexec = fBlobsInMap->Set(numblob, true);
		nbBlobsInMap++;
		if (nbBlobsInMap > 1000)
		{
			nbBlobsInMap = 0;
			fBlobsInMap->Epure();
		}
		
	}

	return errexec;
}


VError DataFile_NotOpened::CheckBlobs(DataAddr4D ou, sLONG nbblobmax, sLONG nbblobstocheck, sLONG pos1, sLONG pos2, ToolLog* log, sLONG mastermultiplicateur)
{
	VError errexec = VE_OK;
	VError err = VE_OK;

	TabAddrDisk taddr;
	log->MarkAddr_TabAddr_Blob(ou, kSizeTabAddr+kSizeDataBaseObjectHeader, fNumTable, -1, -1);
	errexec = bd->ReadAddressTable(ou, taddr, pos1, pos2, log, this, err, 2);
	if (errexec == VE_OK)
	{
		if (err == VE_OK)
		{
			if (nbblobmax > kNbElemTabAddr)
			{
				sLONG diviseur = kNbElemTabAddr;
				if (nbblobmax > kNbElemTabAddr2)
					diviseur = kNbElemTabAddr2;

				sLONG nbparents = (nbblobstocheck+diviseur-1) / diviseur;
				for (sLONG i=0; i<nbparents && errexec == VE_OK; i++)
				{
					sLONG nbremains = diviseur;
					if (i == (nbparents-1))
						nbremains = nbblobstocheck & (diviseur-1);
					if (nbremains == 0)
						nbremains = diviseur;
					if (taddr[i].len != kSizeTabAddr)
					{
						DataTable_BlobTabAddrProblem pb(TOP_LenghOfAddressTableIsInvalid, fNumTable, mastermultiplicateur + i);
						errexec = log->Add(pb);
						fBlobMapIsValid = false;
					}
					if (errexec == VE_OK)
						errexec = CheckBlobs(taddr[i].addr, diviseur - 1, nbremains, i, -1, log, mastermultiplicateur + i * diviseur);
				}
			}
			else
			{
				for (sLONG i=0; i<nbblobstocheck && errexec == VE_OK; i++)
				{
					errexec = log->Progress(mastermultiplicateur + i);
					if (errexec == VE_OK)
					{
						DataAddr4D addr = taddr[i].addr;
						if (fTempBlobAddrCache != nil)
							fTempBlobAddrCache->AddAddr(mastermultiplicateur + i, addr, taddr[i].len);
						if (addr <= 0)
						{
							DataAddr4D addr2 = -addr;
							if (addr2 < DFD.nbBlob || addr == kFinDesTrou)
							{
								errexec = fBlobsDeleted->Set(mastermultiplicateur + i, true);
								if (errexec == VE_OK)
								{
									nbDeletedBlobInserted++;
									if (nbDeletedBlobInserted > 1000)
									{
										nbDeletedBlobInserted = 0;
										fBlobsDeleted->Epure();
									}
								}

							}
							else
							{
								BlobProblem pb(TOP_BlobAddrIsInvalid, fNumTable, mastermultiplicateur + i);
								errexec = log->Add(pb);
								fBlobMapIsValid = false;
							}
						}
						else
						{
							errexec = CheckOneBlob(addr, taddr[i].len, mastermultiplicateur + i, log );
						}
					}
				}
			}
		}
		else
		{
			fBlobMapIsValid = false;
		}
	}
	else
	{
		fBlobMapIsValid = false;
	}

	if (errexec != VE_OK)
		fBlobsHaveBeenChecked = false;

	return errexec;
}


VError DataFile_NotOpened::CheckAllBlobs(ToolLog* log)
{
	VError errexec = VE_OK;
	VError err;
	fBlobsHaveBeenChecked = true;

	OccupableStack* curstack = gOccPool.GetStackForCurrentThread();

	if (fNbBlobIsValid && fTabBlobAddrIsValid)
	{
		fBlobsInMap = new Bittab();
		fBlobsDeleted = new Bittab();
		if (fBlobsDeleted == nil || fBlobsInMap == nil)
		{
			errexec = memfull;
			fBlobsHaveBeenChecked = false;
		}
		else
		{
			fTempBlobAddrCache = new TabAddrCache(bd, DFD.addrBlobtabaddr, DFD.nbBlob);
			fBlobMapIsValid = true;
			nbDeletedBlobInserted = 0;
			nbBlobsInMap = 0;

			VString s;
			log->GetVerifyOrCompactString(14, s);// Checking Blobs on Table {p1}

			VString tablename;
			bd->ExistTable(fNumTable, &tablename);
			FormatStringWithParamsStrings(s, &tablename);
			errexec = log->OpenProgressSession(s, DFD.nbBlob);
			if (errexec == VE_OK)
			{
				if (DFD.addrBlobtabaddr != 0)
				{
					Base4D* target = nil;
					DataTable* df = nil;
					if (log->IsCompacting())
					{
						target = log->GetTargetCompact();
						if (bd->GetStruct() == nil)
							target = target->GetStructure();
						df = target->RetainDataTableByUUID(DFD.TableDefID);
						assert(df != nil);
						log->SetDataTableBeCompacted(df);
						if (df != nil)
						{
							df->ResizeTableBlobs(curstack, DFD.nbBlob);
						}
					}

					sLONG nbmax = DFD.nbBlob;
					/*
					if (nbmax == kNbElemTabAddr || nbmax == kNbElemTabAddr2)
						nbmax++;
						*/
					errexec = CheckBlobs(DFD.addrBlobtabaddr, nbmax, DFD.nbBlob, 0, -1, log, 0);
					{
						if (log->IsCompacting() && errexec == VE_OK)
						{
							if (df != nil)
							{
								df->NormalizeTableBlobs(curstack);
								df->Release();
							}
						}
					}

				}
				log->CloseProgressSession();
			}
			if (errexec == VE_OK && fTabTrouBlobIsValid && !log->IsCompacting())
			{
				log->GetVerifyOrCompactString(15,s); // Checking List of Deleted Blobs on Table {p1}
				VString tablename;
				bd->ExistTable(fNumTable, &tablename);
				FormatStringWithParamsStrings(s, &tablename);
				errexec = log->OpenProgressSession(s, DFD.nbBlob);
				if (DFD.addrBlobtabaddr != 0 && DFD.debutBlobTrou != kFinDesTrou)
					errexec = CheckBlobsTrous(DFD.debutBlobTrou, log);
				log->CloseProgressSession();
			}
			fBlobsDeleted->Epure();
			fBlobsInMap->Epure();

			if (fTempBlobAddrCache != nil)
				delete fTempBlobAddrCache;
			fTempBlobAddrCache = nil;
		}
	}

	return errexec;
}


VError DataFile_NotOpened::CheckBlobsTrous(sLONG numtrou, ToolLog* log)
{
	VError errexec = VE_OK;

	fBlobsTrouHaveBeenChecked = true;

	if (fTabTrouBlobIsValid)
	{
		if (fTabBlobAddrIsValid && fNbBlobIsValid)
		{
			sLONG nbSets = 0;
			fTempBlobsDeleted = new Bittab();
			if (fTempBlobsDeleted != nil)
			{
				DataAddr4D n = DFD.debutBlobTrou;
				Boolean ChaineIsValid = true;
				Boolean ReferenceCirculaire = false;
				Boolean WrongReference = false;
				
				while (n != kFinDesTrou && ChaineIsValid && errexec == VE_OK)
				{
					DataAddr4D n2 = -n;
					if (n2>=0 && n2<DFD.nbBlob)
					{
						if (fTempBlobsDeleted->isOn(n2))
						{
							ChaineIsValid = false;
							ReferenceCirculaire = true;
						}
						else
						{
							errexec = fTempBlobsDeleted->Set(n2, true);
							nbSets++;
							if (nbSets>1000)
							{
								fTempBlobsDeleted->Epure();
								nbSets = 0;
							}
							if (errexec == VE_OK)
							{
								VError err = VE_OK;
								n = bd->GetAddrFromTable(fTempBlobAddrCache, (sLONG)n2, DFD.addrBlobtabaddr, DFD.nbBlob, err);
								if (err != VE_OK)
								{
									ChaineIsValid = false;
									WrongReference = true;
								}
							}
						}
					}
					else
					{
						WrongReference = true;
						ChaineIsValid = false;
					}
				}

				if (errexec == VE_OK)
				{
					if (!ChaineIsValid)
					{
						fTabTrouBlobIsValid = false;
						if (WrongReference)
						{
							DataTableProblem pb(TOP_Table_ChainOfDeletedBlobsIsInvalid, fNumTable, fNumTableDef);
							errexec = log->Add(pb);
						}
						else
						{
							DataTableProblem pb(TOP_Table_ChainOfDeletedBlobsIsCircular, fNumTable, fNumTableDef);
							errexec = log->Add(pb);
						}
					}
				}

				ReleaseRefCountable(&fTempBlobsDeleted);
			}
			else
				errexec = memfull;
		}
		else
			fTabTrouBlobIsValid = false;
	}


	if (errexec != VE_OK)
		fBlobsTrouHaveBeenChecked = false;

	return errexec;
}


VError DataFile_NotOpened::CheckOneRecord(DataAddr4D ou, sLONG len, sLONG numrec, ToolLog* log)
{
	VError errexec = VE_OK;
	VError err;
	RecordHeader tag;
	vector<sLONG> blobsToMark;

	if (bd->IsAddrValid(ou))
	{
		log->MarkAddr_Record(ou, len + kSizeRecordHeader, fNumTable, numrec);
		err = tag.ReadFrom(bd, ou);
		if (err==VE_OK)
		{
			err = tag.ValidateTag(DBOH_Record, numrec, fNumTable);
			if (err == VE_OK)
			{		
				sLONG nbc = tag.GetNbFields();
				sLONG lenx=tag.GetLen();
				if (lenx >= 0)
				{
					if (nbc >= 0 && nbc <= kMaxFields)
					{
						void* p = GetFastMem(lenx+(nbc*sizeof(ChampHeader)), false, 'ficT');
						if (p!=nil)
						{
							err = bd->readlong(p, lenx+(nbc*sizeof(ChampHeader)),ou, kSizeRecordHeader);
							if (err==VE_OK)
							{
								err = tag.ValidateCheckSum(p,lenx);
								if (err == VE_OK)
								{
									/*
									if (log->IsCompacting())
									{
										DataTable* df = log->GetTargetDataTableCompact();
										if (df != nil)
										{
											err = df->SaveRawRecord(numrec, p, lenx, nbc, nil);
										}
									}
									else
									*/
									{
										Boolean cansaverecord = true;

										ChampHeader* cur = (ChampHeader*)( ((char*)p)+lenx );
										ChampHeader* start = cur;
										ChampHeader* end = cur + nbc;
										if (tag.NeedSwap())
											ByteSwapCollection(cur, nbc);
										for (; cur != end && errexec == VE_OK; cur++)
										{
											sLONG curoff = cur->offset;
											if (curoff < 0 || curoff > lenx)
											{
												cansaverecord = false;
												FieldProblem pb(TOP_FieldDataOffsetIsInvalid, fNumTable, numrec, (cur-start)+1);
												errexec = log->Add(pb);
											}
											else
											{
												sLONG lenfield;
												if (cur == (end-1))
												{
													lenfield = lenx - curoff;
												}
												else
												{
													lenfield = (cur+1)->offset - curoff;
												}

												if (lenfield < 0 || lenfield > (lenx - curoff))
												{
													cansaverecord = false;
													FieldProblem pb(TOP_FieldDataLengthIsInvalid, fNumTable, numrec, (cur-start)+1);
													errexec = log->Add(pb);
												}
												else
												{
													char* pfield = ((char*)p) + curoff;
													Boolean badlen = false;

													switch (cur->typ)
													{
														case -1:
															if (lenfield != 0)
																badlen = true;
															break;

														case VK_BOOLEAN:
														case VK_BYTE:
															if (lenfield != 0 && lenfield != 1)
																badlen = true;
															break;

														case VK_WORD:
															if (lenfield != 0 && lenfield != 2)
																badlen = true;
															else
															{
																if (tag.NeedSwap())
																	ByteSwapWord((sWORD*)pfield);
															}
															break;

														case VK_LONG:
															if (lenfield != 0 && lenfield != 4)
																badlen = true;
															else
															{
																if (tag.NeedSwap())
																	ByteSwapLong((sLONG*)pfield);
															}
															break;

														case VK_UUID:
															if (lenfield != 0 && lenfield != sizeof(VUUIDBuffer))
																badlen = true;
															else
															{
																if (tag.NeedSwap())
																{
																	const VValueInfo* xinfo = VValue::ValueInfoFromValueKind(VK_UUID);
																	if (xinfo != nil)
																		xinfo->ByteSwapPtr((void*)pfield, false);
																}
															}
															break;
															

														case VK_SUBTABLE: 
														case VK_SUBTABLE_KEY: 
														case VK_TIME:
														case VK_DURATION:
														case VK_LONG8:
															if (lenfield != 0 && lenfield != 8)
																badlen = true;
															else
															{
																if (tag.NeedSwap())
																{
																	ByteSwapLong8((sLONG8*)pfield);
																}
															}
															break;

														case VK_REAL:
															if (lenfield != 0 && lenfield != sizeof(Real))
																badlen = true;
															else
															{
																if (tag.NeedSwap())
																{
																	const VValueInfo* xinfo = VValue::ValueInfoFromValueKind(VK_REAL);
																	if (xinfo != nil)
																		xinfo->ByteSwapPtr((void*)pfield, false);
																}
															}
															break;

														case VK_FLOAT:
															if (lenfield != 0)
															{
																if (lenfield >= 9 )
																{
																	// float start is 4 + 1 + 4 (long + byte + long)
																	sLONG lenfloat = *((sLONG*)(pfield + 5));
																	if (tag.NeedSwap())
																		ByteSwapLong(&lenfloat);

																	if (lenfloat+9 != lenfield)
																		badlen = true;
																	else
																	{
																		if (tag.NeedSwap())
																		{
																			const VValueInfo* xinfo = VValue::ValueInfoFromValueKind(VK_FLOAT);
																			if (xinfo != nil)
																				xinfo->ByteSwapPtr((void*)pfield, false);
																		}
																	}
																}
																else
																	badlen = true;
															}
															break;

														case VK_STRING_UTF8:
															if (lenfield != 0)
															{
																if (lenfield >= 4)
																{
																	sLONG lenstring = *((sLONG*)pfield);
																	if (tag.NeedSwap())
																		ByteSwapLong(&lenstring);
																	if (lenstring+4 != lenfield)
																		badlen = true;
																	else
																	{
																		if (tag.NeedSwap())
																		{
																			const VValueInfo* xinfo = VValue::ValueInfoFromValueKind(VK_STRING_UTF8);
																			if (xinfo != nil)
																				xinfo->ByteSwapPtr((void*)pfield, false);
																		}
																	}
																}
																else
																	badlen = true;
															}
															break;

														case VK_STRING:
															if (lenfield != 0)
															{
																if (lenfield >= 4)
																{
																	sLONG lenstring = *((sLONG*)pfield);
																	if (tag.NeedSwap())
																		ByteSwapLong(&lenstring);
																	if ((lenstring*2)+4 != lenfield)
																		badlen = true;
																	else
																	{
																		if (tag.NeedSwap())
																		{
																			const VValueInfo* xinfo = VValue::ValueInfoFromValueKind(VK_STRING);
																			if (xinfo != nil)
																				xinfo->ByteSwapPtr((void*)pfield, false);
																		}
																	}
																}
																else
																	badlen = true;
															}
															break;

														case VK_TEXT_UTF8:
														case VK_TEXT:
														case VK_BLOB:
														case VK_BLOB_DB4D:
														case VK_IMAGE:
															if (lenfield != 0)
															{
																if (lenfield == 4)
																{
																	Boolean mustchangeblobnum = false;

																	if (tag.NeedSwap())
																		ByteSwapLong((sLONG*)pfield);
																	sLONG blobnum = *((sLONG*)pfield);
																	if (blobnum >=0 && blobnum < DFD.nbBlob)
																	{
																		if (fBlobsDeleted->isOn(blobnum))
																		{
																			mustchangeblobnum = true;
																			FieldProblem pb(TOP_FieldBlobIsDeleted, fNumTable, numrec, (cur-start)+1);
																			errexec = log->Add(pb);
																		}
																		else
																		{
																			if (!fBlobsInMap->isOn(blobnum))
																			{
																				mustchangeblobnum = true;
																				FieldProblem pb(TOP_FieldBlobIsInvalid, fNumTable, numrec, (cur-start)+1);
																				errexec = log->Add(pb);
																			}
																			else
																			{
																				if (fBlobsInRec->isOn(blobnum))
																				{
																					mustchangeblobnum = true;
																					FieldProblem pb(TOP_FieldBlobRefIsAlreadyUsed, fNumTable, numrec, (cur-start)+1);
																					errexec = log->Add(pb);
																				}
																				else
																				{
																					blobsToMark.push_back(blobnum);
																				}
																			}
																		}

																	}
																	else
																	{
																		if (blobnum != -1)
																		{
																			mustchangeblobnum = true;
																			FieldProblem pb(TOP_FieldBlobNumIsInvalid, fNumTable, numrec, (cur-start)+1);
																			errexec = log->Add(pb);
																		}
																	}

																	if (mustchangeblobnum)
																	{
																		blobnum = -1;
																		*((sLONG*)pfield) = blobnum;
																	}
																}
																else
																{
																	if (lenfield >= 4)
																	{
																		sLONG lentext = *((sLONG*)pfield);
																		if (tag.NeedSwap())
																			ByteSwapLong(&lentext);
																		if (lentext <= -10) // blob dans la fiche
																		{
																			lentext = - lentext - 10;
																			if (cur->typ == VK_BLOB || cur->typ == VK_BLOB_DB4D || cur->typ == VK_IMAGE)
																			{
																				sLONG lenblob = lentext;
																				if ((lenblob + 4) != lenfield)
																					badlen = true;
																			}
																			else
																			{
																				if (((lentext*2) + 4) != lenfield)
																					badlen = true;
																			}
																			if (!badlen)
																			{
																				if (tag.NeedSwap())
																				{
																					const VValueInfo* xinfo = VValue::ValueInfoFromValueKind(cur->typ);
																					if (xinfo != nil)
																						xinfo->ByteSwapPtr((void*)pfield, false);
																				}
																			}
																		}
																		else if (lentext == -2 || lentext == -3) // blob dans outside path
																		{
																			sLONG* pblob = (sLONG*)pfield;
																			pblob++;
																			sLONG pathlen = *pblob;
																			if ((pathlen*2 + 4 + 4) == lenfield)
																			{
																				pblob++;
																				bool isrelatif = (lentext == -2);
																				VString pathstring;
																				pathstring.FromBlock(pblob, pathlen * sizeof(UniChar), VTC_UTF_16);
																				if (isrelatif)
																				{
																					const VFile* dfile = bd->GetDataFile();
																					if (dfile != nil)
																					{
																						VFolder* parent = dfile->RetainParentFolder();
																						VFilePath path;
																						VString s;
																						dfile->GetNameWithoutExtension(s);
																						s += ".ExternalData/";
																						path.FromRelativePath(*parent, s+pathstring, FPS_POSIX);
																						VFile blobfile(path);
																						if (!blobfile.Exists())
																						{
																							FieldProblem pb(TOP_FieldExternalBlobIsMissing, fNumTable, numrec, (cur-start)+1);
																							pb.SetErrorLevel(TO_ErrorLevel_Warning);
																							errexec = log->Add(pb);
																						}
																						else
																						{
																							if (log->IsCompacting())
																							{
																								const VFile* destdFile = log->GetTargetCompact()->GetDataSegs()->GetFirst()->GetFile();
																								if (destdFile != nil)
																								{
																									VFolder* destparent = destdFile->RetainParentFolder();
																									VFilePath destpath;
																									destdFile->GetNameWithoutExtension(s);
																									s += ".ExternalData/";
																									destpath.FromRelativePath(*destparent, s+pathstring, FPS_POSIX);
																									VFile destFile(destpath);	

																									VFolder* destFolder = destFile.RetainParentFolder();
																									if (destFolder != nil)
																									{
																										destFolder->CreateRecursive();
																										destFolder->Release();
																									}

																									errexec = blobfile.CopyTo(destFile);
																									destparent->Release();
																								}
																							}
																						}
																						parent->Release();
																					}
																				}
																				else
																				{
																				}
																			}
																			else
																				badlen = true;

																		}
																		else
																			badlen = true;
																	}
																	else
																		badlen = true;
																}
															}
															break;


														default:
															{
																FieldProblem pb(TOP_FieldDataTypeIsInvalid, fNumTable, numrec, (cur-start)+1);
																errexec = log->Add(pb);
																if (len == 1)
																	cur->typ = VK_BYTE;
																else if (len == 2)
																	cur->typ = VK_WORD;
																else if (len == 4)
																	cur->typ = VK_LONG;
																else if (len == 8)
																	cur->typ = VK_LONG8;
																else if (len == 16)
																	cur->typ = VK_UUID;
																else if (len == 0)
																	cur->typ = -1;
																else
																	cansaverecord = false;
															}
													} // du switch

													if (badlen)
													{
														cansaverecord = false;
														FieldProblem pb(TOP_FieldDataLengthIsInvalid, fNumTable, numrec, (cur-start)+1);
														errexec = log->Add(pb);
													}
												}
											}
										}

										if ( adjuste(lenx+(nbc*sizeof(ChampHeader))) != adjuste(len))
										{
											cansaverecord = false;
											RecordProblem pb(TOP_RecordLengthDoesNotMatchTabAddr, fNumTable, numrec);
											errexec = log->Add(pb);
										}

										if (log->IsCompacting() && !cansaverecord)
										{
										}
										else
										{
											for (vector<sLONG>::iterator cur = blobsToMark.begin(), end = blobsToMark.end(); cur != end && errexec == VE_OK; cur++)
											{
												sLONG blobnum = *cur;
#if debug_BlobsRefsWithTools
												fMapBlobs[blobnum] = numrec;
#endif
												errexec = fBlobsInRec->Set(blobnum, true);

												nbBlobsInRec++;
												if (false && nbBlobsInRec > 1000)
												{
													nbBlobsInRec = 0;
													fBlobsInRec->Epure();
												}
											}
										}


										if (log->IsCompacting() && cansaverecord)
										{
											OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
											DataTable* df = log->GetTargetDataTableCompact();
											if (df != nil)
											{
												err = df->SaveRawRecord(curstack, numrec, p, lenx, nbc, nil);
											}
										}

									}
								}
								else
								{
									RecordProblem pb(TOP_RecordCheckSumIsInvalid, fNumTable, numrec);
									errexec = log->Add(pb);
								}
							}
							else
							{
								RecordProblem pb(TOP_RecordPhysicalDataIsInvalid, fNumTable, numrec);
								errexec = log->Add(pb);
							}

							FreeFastMem(p);
						}
						else
						{
							errexec = memfull;
						}
					}
					else
					{
						RecordProblem pb(TOP_RecordNumberOfFieldsIsInvalid, fNumTable, numrec);
						errexec = log->Add(pb);
					}
				}
				else
				{
					RecordProblem pb(TOP_RecordLengthIsInvalid, fNumTable, numrec);
					errexec = log->Add(pb);
				}
				
			}
			else
			{
				RecordProblem pb(TOP_RecordTagIsInvalid, fNumTable, numrec);
				errexec = log->Add(pb);
			}
		}
		else
		{
			RecordProblem pb(TOP_RecordPhysicalDataIsInvalid, fNumTable, numrec);
			errexec = log->Add(pb);
		}
	}
	else
	{
		RecordProblem pb(TOP_RecordAddrIsInvalid, fNumTable, numrec);
		errexec = log->Add(pb);
		fRecordMapIsValid = false;
	}

	return errexec;
}

VError DataFile_NotOpened::CheckRecords(DataAddr4D ou, sLONG nbficmax, sLONG nbfictocheck, sLONG pos1, sLONG pos2, ToolLog* log, sLONG mastermultiplicateur)
{
	VError errexec = VE_OK;
	VError err = VE_OK;

	TabAddrDisk taddr;
	log->MarkAddr_TabAddr_Record(ou, kSizeTabAddr+kSizeDataBaseObjectHeader, fNumTable, -1, -1);
	errexec = bd->ReadAddressTable(ou, taddr, pos1, pos2, log, this, err, 1, true, fWithStamps);
	if (errexec == VE_OK)
	{
		if (err == VE_OK)
		{
			if (nbficmax > kNbElemTabAddr)
			{
				sLONG diviseur = kNbElemTabAddr;
				if (nbficmax > kNbElemTabAddr2)
					diviseur = kNbElemTabAddr2;

				sLONG nbparents = (nbfictocheck+diviseur-1) / diviseur;
				for (sLONG i=0; i<nbparents && errexec == VE_OK; i++)
				{
					sLONG nbremains = diviseur;
					if (i == (nbparents-1))
						nbremains = nbfictocheck & (diviseur-1);
					if (nbremains == 0)
						nbremains = diviseur;
					sLONG truelen = kSizeTabAddr;
					if (DFD.fKeepStamps == 1)
						truelen += sizeof(StampsArray);
					if (taddr[i].len != truelen)
					{
						DataTable_RecTabAddrProblem pb(TOP_LenghOfAddressTableIsInvalid, fNumTable, mastermultiplicateur + i);
						errexec = log->Add(pb);
						fRecordMapIsValid = false;
					}
					if (errexec == VE_OK)
						errexec = CheckRecords(taddr[i].addr, diviseur-1, nbremains, i, -1, log, mastermultiplicateur + i * diviseur);
				}
			}
			else
			{
				for (sLONG i=0; i<nbfictocheck && errexec == VE_OK; i++)
				{
					errexec = log->Progress(mastermultiplicateur + i);
					if (errexec == VE_OK)
					{
						DataAddr4D addr = taddr[i].addr;
						if (fTempRecAddrCache != nil)
							fTempRecAddrCache->AddAddr(mastermultiplicateur + i, addr, taddr[i].len);
						if (addr <= 0)
						{
							DataAddr4D addr2 = -addr;
							if (addr2 < DFD.nbfic || addr == kFinDesTrou)
							{
								errexec = fRecordsDeleted->Set(mastermultiplicateur + i, true);
								if (errexec == VE_OK)
								{
									nbDeletedRecordInserted++;
									if (nbDeletedRecordInserted > 1000)
									{
										nbDeletedRecordInserted = 0;
										fRecordsDeleted->Epure();
									}
								}

							}
							else
							{
								RecordProblem pb(TOP_RecordAddrIsInvalid, fNumTable, mastermultiplicateur + i);
								errexec = log->Add(pb);
								fRecordMapIsValid = false;
							}
						}
						else
						{
							errexec = CheckOneRecord(addr, taddr[i].len, mastermultiplicateur + i, log );
						}
					}
				}
			}
		}
		else
		{
			fRecordMapIsValid = false;
		}
	}
	else
	{
		fRecordMapIsValid = false;
	}

	if (errexec != VE_OK)
		fRecordsHaveBeenChecked = false;

	return errexec;
}


VError DataFile_NotOpened::CheckRecordsTrous(sLONG numtrou, ToolLog* log)
{
	VError errexec = VE_OK;

	fRecordsTrouHaveBeenChecked = true;

	if (fTabTrouRecIsValid)
	{
		if (fTabRecAddrIsValid && fNbFicIsValid)
		{
			sLONG nbSets = 0;
			fTempRecordsDeleted = new Bittab();
			if (fTempRecordsDeleted != nil)
			{
				DataAddr4D n;
				Boolean ChaineIsValid = true;
				Boolean ReferenceCirculaire = false;
				Boolean WrongReference = false;

				for (sLONG npass = 1; npass <= 2 && ChaineIsValid && errexec == VE_OK; npass++)
				{
					if (npass == 1)
						n = DFD.debuttrou;
					else
					{
						n = DFD.debutTransRecTrou;
						if (n != kFinDesTrou)
							n++;
					}

					while (n != kFinDesTrou && ChaineIsValid && errexec == VE_OK)
					{
						DataAddr4D n2 = -n;
						if (n2>=0 && n2<DFD.nbfic)
						{
							if (fTempRecordsDeleted->isOn(n2))
							{
								ChaineIsValid = false;
								ReferenceCirculaire = true;
							}
							else
							{
								errexec = fTempRecordsDeleted->Set(n2, true);
								nbSets++;
								if (nbSets>1000)
								{
									fTempRecordsDeleted->Epure();
									nbSets = 0;
								}
								if (errexec == VE_OK)
								{
									VError err = VE_OK;
									n = bd->GetAddrFromTable(fTempRecAddrCache, (sLONG)n2, DFD.addrtabaddr, DFD.nbfic, err, true, nil, fWithStamps);
									if (err != VE_OK)
									{
										ChaineIsValid = false;
										WrongReference = true;
									}
								}
							}
						}
						else
						{
							WrongReference = true;
							ChaineIsValid = false;
						}
					}
				}

				if (errexec == VE_OK)
				{
					if (!ChaineIsValid)
					{
						fTabTrouRecIsValid = false;
						if (WrongReference)
						{
							DataTableProblem pb(TOP_Table_ChainOfDeletedRecordsIsInvalid, fNumTable, fNumTableDef);
							errexec = log->Add(pb);
						}
						else
						{
							DataTableProblem pb(TOP_Table_ChainOfDeletedRecordsIsCircular, fNumTable, fNumTableDef);
							errexec = log->Add(pb);
						}
					}
					else
					{
						fNbRecords = DFD.nbfic - fTempRecordsDeleted->Compte();
					}
				}

				ReleaseRefCountable(&fTempRecordsDeleted);
			}
			else
				errexec = memfull;
		}
		else
			fTabTrouRecIsValid = false;
	}


	if (errexec != VE_OK)
		fRecordsTrouHaveBeenChecked = false;

	return errexec;
}


VError DataFile_NotOpened::CountRecords(ToolLog* log, sLONG& result, Boolean& outInfoIsValid)
{
	VError errexec = VE_OK;
	outInfoIsValid = false;
	result = -1;
	if (fIsLoaded && fNbFicIsValid && fTabRecAddrIsValid && fTabTrouRecIsValid)
	{
		if (!fRecordsTrouHaveBeenChecked)
			CheckRecordsTrous(DFD.debuttrou, log);
		if (fTabTrouRecIsValid)
		{
			outInfoIsValid = true;
			result = fNbRecords;
		}
	}
	return errexec;
}


VError DataFile_NotOpened::CheckAllRecords(ToolLog* log)
{
	VError errexec = VE_OK;
	VError err;
	fRecordsHaveBeenChecked = true;
	OccupableStack* curstack = gOccPool.GetStackForCurrentThread();

	if (fNbFicIsValid && fTabRecAddrIsValid)
	{
		//fRecordsInMap = new Bittab();
		fBlobsInRec = new Bittab();
		fRecordsDeleted = new Bittab();
		if (fRecordsDeleted == nil || fBlobsInRec == nil /* || fRecordsInMap == nil */)
		{
			errexec = memfull;
			fRecordsHaveBeenChecked = false;
		}
		else
		{
			fTempRecAddrCache = new TabAddrCache(bd, DFD.addrtabaddr, DFD.nbfic);

			nbBlobsInRec = 0;
			fRecordMapIsValid = true;
			nbDeletedRecordInserted = 0;
			VString s;
			log->GetVerifyOrCompactString(16,s);	// Checking Records on Table {p1}
			VString tablename;
			bd->ExistTable(fNumTable, &tablename);
			FormatStringWithParamsStrings(s, &tablename);
			errexec = log->OpenProgressSession(s, DFD.nbfic);
			if (errexec == VE_OK)
			{
				if (DFD.addrtabaddr != 0)
				{
					Base4D* target = nil;
					DataTable* df = nil;
					if (log->IsCompacting())
					{
						target = log->GetTargetCompact();
						if (bd->GetStruct() == nil)
							target = target->GetStructure();
						df = target->RetainDataTableByUUID(DFD.TableDefID);
						assert(df != nil);
						log->SetDataTableBeCompacted(df);
						if (df != nil)
						{
							df->ResizeTableRecs(curstack, DFD.nbfic);
						}
					}

					/* Bug fix for ACI0049937 - Sergiy - 9 April 2007
					Detection of phantom records - records that are deleted and are viewed as such by DFD
					but high-level classes like CDB4DTable and CDB4DSelection think that these records are
					there and are not deleted. */
					sLONG			nNonDeletedRecordCount = 0;												//
					Boolean			bInfoIsValid = false;													//
					VError			vE = CountRecords ( log, nNonDeletedRecordCount, bInfoIsValid );		//
					/*-------------ACI0049937---------------------------------------------------------------*/

					sLONG nbmax = DFD.nbfic;
					/*
					if (nbmax == kNbElemTabAddr || nbmax == kNbElemTabAddr2)
						nbmax++;
						*/
					errexec = CheckRecords(DFD.addrtabaddr, nbmax, DFD.nbfic, 0, -1, log, 0);

					/* Bug fix for ACI0049937 - Sergiy - 9 April 2007
					Detection of phantom records - records that are deleted and are viewed as such by DFD
					but high-level classes like CDB4DTable and CDB4DSelection think that these records are
					there and are not deleted. */
					sLONG			nDeletedRecordCount = fRecordsDeleted-> Compte ( );						//
					sLONG			nTotalRecordCount = DFD. nbfic;											//
					if ( nNonDeletedRecordCount + nDeletedRecordCount > nTotalRecordCount )					//
					{																						//
						DataTableProblem		pb ( TOP_Table_NBficIsInvalid, fNumTable, fNumTableDef );	//
						log-> Add ( pb );																	//
					}																						//
					/*-------------ACI0049937---------------------------------------------------------------*/

					if (log->IsCompacting() && errexec == VE_OK)
					{
						if (df != nil)
						{
							df->NormalizeTableRecs(curstack);
							df->Release();
						}
					}
				}

				if (errexec == VE_OK)
				{
					if (fBlobsHaveBeenChecked)
					{
						fBlobsOrphanHaveBeenChecked = true;
						if (fNbBlobIsValid && fTabBlobAddrIsValid)
						{
							fBlobsOrphan = new Bittab();
							if (fBlobsOrphan == nil)
								errexec = memfull;
							else
							{
								errexec = fBlobsOrphan->Or(fBlobsInMap);
								if (errexec == VE_OK)
									errexec = fBlobsOrphan->moins(fBlobsInRec);
								if (errexec == VE_OK)
								{
									if (fBlobsOrphan->Compte() > 0)
									{
										DataTableProblem pb(TOP_Table_SomeBlobsAreOrphan, fNumTable, fNumTableDef);
										errexec = log->Add(pb);
										if (log->IsCompacting() && errexec == VE_OK)
										{
											DataTable* df = log->GetTargetDataTableCompact();
											sLONG n = fBlobsOrphan->FindNextBit(0);
											while (n != -1 && errexec == VE_OK)
											{
												errexec = df->DelBlobForTrans(n, nil, true);
												n = fBlobsOrphan->FindNextBit(n+1);
											}
										}
									}
								}
							}

							if (errexec != VE_OK)
							{
								ReleaseRefCountable(&fBlobsOrphan);
								fBlobsOrphanHaveBeenChecked = false;
							}
						}
					}
				}
				log->CloseProgressSession();
			}

			if (errexec == VE_OK && fTabTrouRecIsValid && !log->IsCompacting())
			{
				log->GetVerifyOrCompactString(17,s);	// Checking List of Deleted Records on Table {p1}
				VString tablename;
				bd->ExistTable(fNumTable, &tablename);
				FormatStringWithParamsStrings(s, &tablename);
				errexec = log->OpenProgressSession(s, DFD.nbBlob);
				if (DFD.addrtabaddr != 0 && DFD.debuttrou != kFinDesTrou && errexec == VE_OK)
					errexec = CheckRecordsTrous(DFD.debuttrou, log);
				log->CloseProgressSession();
			}
			fRecordsDeleted->Epure();
			fBlobsInRec->Epure();

			if (fTempRecAddrCache != nil)
				delete fTempRecAddrCache;
			fTempRecAddrCache = nil;
		}
	}

	return errexec;
}



VError DataFile_NotOpened::ReportInvalidTabAddrAddr(ToolLog* log, sLONG selector)
{
	if (selector == 1)
	{
		DataTableProblem pb(TOP_Table_AddrOfRecordAddressTableIsInvalid, fNumTable, fNumTableDef);
		return log->Add(pb);
	}
	else
	{
		DataTableProblem pb(TOP_Table_AddrOfBlobAddressTableIsInvalid, fNumTable, fNumTableDef);
		return log->Add(pb);
	}

}


VError DataFile_NotOpened::ReportInvalidTabAddrTag(ToolLog* log, sLONG selector)
{
	if (selector == 1)
	{
		DataTableProblem pb(TOP_Table_TagOfRecordAddressTableIsInvalid, fNumTable, fNumTableDef);
		return log->Add(pb);
	}
	else
	{
		DataTableProblem pb(TOP_Table_TagOfBlobAddressTableIsInvalid, fNumTable, fNumTableDef);
		return log->Add(pb);
	}
}


VError DataFile_NotOpened::ReportInvalidTabAddrRead(ToolLog* log, sLONG selector)
{
	if (selector == 1)
	{
		DataTableProblem pb(TOP_Table_PhysicalDataOfRecordAddressTableIsInvalid, fNumTable, fNumTableDef);
		return log->Add(pb);
	}
	else
	{
		DataTableProblem pb(TOP_Table_PhysicalDataOfBlobAddressTableIsInvalid, fNumTable, fNumTableDef);
		return log->Add(pb);
	}
}


VError DataFile_NotOpened::ReportInvalidTabAddrChecksum(ToolLog* log, sLONG selector)
{
	if (selector == 1)
	{
		DataTableProblem pb(TOP_Table_CheckSumOfRecordAddressTableIsInvalid, fNumTable, fNumTableDef);
		return log->Add(pb);
	}
	else
	{
		DataTableProblem pb(TOP_Table_CheckSumOfBlobAddressTableIsInvalid, fNumTable, fNumTableDef);
		return log->Add(pb);
	}
}


ReplicationOutputBinaryFormatter::ReplicationOutputBinaryFormatter ( VStream& inStream, bool inUseRawImages, vector<VString*> const * inImageFormats ) :
																			fStream ( inStream ), fUseRawImages ( inUseRawImages )
{
	if ( inImageFormats != 0 )
	{
		fImageFormats. insert ( fImageFormats. end ( ), inImageFormats-> begin ( ), inImageFormats-> end ( ) );
		vector<VString*>::iterator		iter = fImageFormats. begin ( );
		while ( iter != fImageFormats. end ( ) )
		{
			if ( ( *iter ) != 0 && ( *iter )-> EqualToString ( CVSTR ( ".json" ) ) )
			{
				iter = fImageFormats. erase ( iter );
			}
			else
				iter++;
		}
	}
}

ReplicationOutputBinaryFormatter::~ReplicationOutputBinaryFormatter ( )
{
}

VError ReplicationOutputBinaryFormatter::PutVValue ( VValueSingle* inValue )
{
	VError			err = VE_OK;

	if (inValue == nil || inValue->IsNull())
	{
		err = fStream.PutWord(VK_EMPTY);
	}
	else
	{
		err = fStream.PutWord(inValue->GetValueKind());
		if (err == VE_OK)
		{
			if ( inValue->GetValueKind() == VK_IMAGE && fUseRawImages && fImageFormats. size ( ) > 0 && VDBMgr::GetManager()->GetGraphicsInterface() != nil )
			{
				err = VDBMgr::GetManager()->GetGraphicsInterface()-> ConvertImageToPreferredFormat ( *inValue, fImageFormats );
				if ( err == VE_OK )
				{
					VBlobWithPtr			vblbPict;
					inValue-> GetValue ( vblbPict );
					err = vblbPict. WriteToStream ( &fStream );
				}
				else
					fStream.PutWord(VK_EMPTY);
			}
			else
				err = inValue->WriteToStream(&fStream);
		}
	}

	return err;
}

VError ReplicationOutputBinaryFormatter::PutEmptyResponse ( )
{
	VError			vError = fStream. PutLong ( 0 );
	xbox_assert ( vError == VE_OK );
	if ( vError == VE_OK )
	{
		vError = fStream. PutByte ( 0  );
		xbox_assert ( vError == VE_OK );
	}

	return vError;
}

ReplicationOutputJSONFormatter::ReplicationOutputJSONFormatter ( VStream& inStream, bool inUseRawImages, vector<VString*> const * inImageFormats ) :
																			fStream ( inStream ), fUseRawImages ( inUseRawImages )
{
	if ( inImageFormats != 0 )
	{
		fImageFormats. insert ( fImageFormats. end ( ), inImageFormats-> begin ( ), inImageFormats-> end ( ) );
		vector<VString*>::iterator		iter = fImageFormats. begin ( );
		while ( iter != fImageFormats. end ( ) )
		{
			if ( ( *iter ) != 0 && ( *iter )-> EqualToString ( CVSTR ( ".json" ) ) )
			{
				iter = fImageFormats. erase ( iter );
			}
			else
				iter++;
		}
	}

	XBOX::VError		vError = fStream. SetCharSet ( VTC_UTF_8 );
	xbox_assert ( vError == VE_OK );
	fStream. SetCarriageReturnMode ( eCRM_CRLF );

	fIsFirstAction = true;
	fCurrentAction = 0;
	fPrimaryKeyIndex = -1;
	fFieldIndex = -1;
}

ReplicationOutputJSONFormatter::~ReplicationOutputJSONFormatter ( )
{
}

void ReplicationOutputJSONFormatter::SetPrimaryKey ( FieldArray const & inPK )
{
	for ( VIndex i = 1; i <= inPK. GetCount ( ); i++ )
	{
		VString			vstrName;
		inPK [ i ]-> GetName ( vstrName );
		EscapeJSONString ( vstrName );
		fPrimaryKey. push_back ( vstrName );
	}
}

void ReplicationOutputJSONFormatter::SetFields ( Table* inTable, vector<sLONG> const & inFieldIDs )
{
	if ( inTable == 0 )
		return;

	Field*								fld = 0;
	vector<sLONG>::const_iterator		iter = inFieldIDs. begin ( );
	while ( iter != inFieldIDs. end ( ) )
	{
		fld = inTable-> RetainField ( *iter );
		if ( fld != 0 )
		{
			VString						vstrName;
			fld-> GetName ( vstrName );
			EscapeJSONString ( vstrName );
			fFields. push_back ( vstrName );
			fld-> Release ( );
		}
		iter++;
	}
}

VError ReplicationOutputJSONFormatter::PutActionCount ( sLONG inCount )
{
	VString			vstr;
	vstr. AppendCString ( "{\"__SENT\":" );
	vstr. AppendLong ( inCount );
	vstr. AppendCString ( "," );
	VError			vError = fStream. PutText ( vstr );

	return vError;
}

VError ReplicationOutputJSONFormatter::PutLatestStamp ( sLONG8 inStamp )
{
	VString			vstr;
	vstr. AppendCString ( "\"__LATEST_STAMP\":\"" );
	vstr. AppendLong8 ( inStamp );
	vstr. AppendCString ( "\",\"__ACTIONS\":[" );
	VError			vError = fStream. PutText ( vstr );

	return vError;
}

VError ReplicationOutputJSONFormatter::PutAction ( sBYTE inAction )
{
	VString			vstr;
	if ( inAction == 0 )
		vstr. AppendCString ( "]}" );
	else
	{
		if ( !fIsFirstAction )
			vstr. AppendCString ( "," );

		vstr. AppendCString ( "{\"__ACTION\":\"" );
		if ( inAction == 1 )
			vstr. AppendCString ( "update" );
		else if ( inAction == 2 )
			vstr. AppendCString ( "delete" );
		else
			vstr. AppendCString ( "unknown" );

		vstr. AppendCString ( "\"," );

		fIsFirstAction = false;
		fCurrentAction = inAction;
	}

	VError			vError = fStream. PutText ( vstr );

	return vError;
}

VError ReplicationOutputJSONFormatter::PutPrimaryKeyCount ( sBYTE inCount )
{
	VString			vstr;
	vstr. AppendCString ( "\"__PRIMARY_KEY_COUNT\":" );
	vstr. AppendLong ( inCount );
	vstr. AppendCString ( "," );
	VError			vError = fStream. PutText ( vstr );

	fPrimaryKeyIndex = 0;

	return vError;
}

VError ReplicationOutputJSONFormatter::PutVValue ( VValueSingle* inValue )
{
	VError			vError = VE_OK;

	VString			vstr;
	if ( fPrimaryKeyIndex >= 0 )
	{
		if ( fPrimaryKeyIndex == 0 )
			vstr. AppendCString ( "\"__PRIMARY_KEY\":{\"" );
		else
			vstr. AppendCString ( ",\"" );

		vstr. AppendString ( fPrimaryKey [ fPrimaryKeyIndex ] );
		vstr. AppendCString ( "\":" );
		vError = GetJSONValue ( inValue, vstr );
		xbox_assert ( vError == VE_OK );
		fPrimaryKeyIndex++;
		if ( fPrimaryKeyIndex == fPrimaryKey. size ( ) )
		{
			vstr. AppendCString ( "}," );
			fPrimaryKeyIndex = -1;
		}
	}
	else if ( fFieldIndex >= 0 )
	{
		if ( fFieldIndex == 0 )
			vstr. AppendCString ( "\"__VALUES\":{\"" );
		else
			vstr. AppendCString ( ",\"" );

		vstr. AppendString ( fFields [ fFieldIndex ] );
		vstr. AppendCString ( "\":" );
		vError = GetJSONValue ( inValue, vstr );
		xbox_assert ( vError == VE_OK );
		fFieldIndex++;
		if ( fFieldIndex == fFields. size ( ) )
		{
			vstr. AppendCString ( "}}" );
			fFieldIndex = -1;
		}
	}

	vError = fStream. PutText ( vstr );

	return vError;
}

VError ReplicationOutputJSONFormatter::GetJSONValue ( VValueSingle* vval, VString& outValue )
{
	VError			vError = VE_OK;
	if ( vval == 0 || vval-> IsNull ( ) )
	{
		outValue. AppendCString ( "null" );

		return VE_OK;
	}

	ValueKind		vKind = vval-> GetValueKind ( );
	switch ( vKind )
	{
		case VK_BOOLEAN:
			if ( vval-> GetBoolean ( ) )
				outValue. AppendCString ( "true" );
			else
				outValue. AppendCString ( "false" );

			break;
		case VK_BYTE:
		case VK_WORD:
		case VK_LONG:
		{
			sLONG		nVal = vval-> GetLong ( );
			outValue. AppendLong ( nVal );
			break;
		}
		case VK_UUID:
		{
			VString		vstr;
			vval-> GetString ( vstr );
			outValue. AppendCString ( "\"" );
			outValue. AppendString ( vstr );
			outValue. AppendCString ( "\"" );

			break;
		}
		case VK_TIME:
		{
			VTime*			vT = dynamic_cast<VTime*> ( vval );
			if ( vT == 0 )
				outValue. AppendCString ( "null" );
			else
			{
				VString		vstrTime;
				vT-> GetRfc822String ( vstrTime );
				outValue. AppendCString ( "\"" );
				outValue. AppendString ( vstrTime );
				outValue. AppendCString ( "\"" );
			}

			break;
		}
		case VK_DURATION:
		{
			VString		vstr;
			vval-> GetString ( vstr );
			outValue. AppendCString ( "\"" );
			outValue. AppendString ( vstr );
			outValue. AppendCString ( "\"" );

			break;
		}
		case VK_LONG8:
		{
			sLONG8		nVal = vval-> GetLong8 ( );
			outValue. AppendLong ( nVal );
			break;
		}
		case VK_REAL:
		{
			Real		nVal = vval-> GetReal ( );
			outValue. AppendReal ( nVal );
			break;
		}
		case VK_FLOAT:
		{
			VString		vstr;
			vval-> GetString ( vstr );
			outValue. AppendCString ( "\"" );
			outValue. AppendString ( vstr );
			outValue. AppendCString ( "\"" );

			break;
		}
		case VK_STRING:
		case VK_TEXT:
		{
			outValue. AppendCString ( "\"" );
			VString		vstr;
			vstr. FromValue ( *vval );
			EscapeJSONString ( vstr );
			outValue. AppendValue ( vstr );
			outValue. AppendCString ( "\"" );
			break;
		}
		case VK_BLOB:
			xbox_assert ( false );
			outValue. AppendCString ( "UNSUPPORTED TYPE" );

			break;
		case VK_BLOB_DB4D:
			xbox_assert ( false );
			outValue. AppendCString ( "UNSUPPORTED TYPE" );

			break;
		case VK_IMAGE:
			xbox_assert ( false );
			outValue. AppendCString ( "UNSUPPORTED TYPE" );

			break;
		default:
			xbox_assert ( false );
			outValue. AppendCString ( "UNKNOWN TYPE" );

			break;
	}

	return vError;
}

VError ReplicationOutputJSONFormatter::PutStamp ( uLONG8 inStamp )
{
	VString			vstr;
	vstr. AppendCString ( "\"__STAMP\":" );
	vstr. AppendLong ( inStamp );
	vstr. AppendCString ( "," );

	VError			vError = fStream. PutText ( vstr );

	return vError;
}

VError ReplicationOutputJSONFormatter::PutTimeStamp ( VValueSingle& inStamp )
{
	VString			vstr;
	vstr. AppendCString ( "\"__TIMESTAMP\":" );
	VTime*			vT = dynamic_cast<VTime*> ( &inStamp );
	if ( vT == 0 )
		vstr. AppendCString ( "null" );
	else
	{
		VString		vstrTime;
		vT-> GetRfc822String ( vstrTime );
		vstr. AppendCString ( "\"" );
		vstr. AppendString ( vstrTime );
		vstr. AppendCString ( "\"" );
	}

	if ( fCurrentAction == DB4D_SyncAction_Update )
		vstr. AppendCString ( "," );
	else if ( fCurrentAction == DB4D_SyncAction_Delete )
		vstr. AppendCString ( "}" );

	VError			vError = fStream. PutText ( vstr );

	return vError;
}

VError ReplicationOutputJSONFormatter::PutFieldCount ( sLONG inCount )
{
	VString			vstr;
	vstr. AppendCString ( "\"__FIELD_COUNT\":" );
	vstr. AppendLong ( inCount );
	vstr. AppendCString ( "," );
	VError			vError = fStream. PutText ( vstr );

	fFieldIndex = 0;

	return vError;
}

VError ReplicationOutputJSONFormatter::PutEmptyResponse ( )
{
	VError			vError = fStream. PutText ( CVSTR ( "{\"__SENT\":0,\"__LATEST_STAMP\":\"0\",\"__ACTIONS\":[]}" ) );

	return vError;
}

void ReplicationOutputJSONFormatter::EscapeJSONString ( VString& ioValue )
{
	ioValue. ExchangeAll ( CVSTR ( "\\" ), CVSTR ( "\\\\" ) );
	ioValue. ExchangeAll ( CVSTR ( "\"" ), CVSTR ( "\\\"" ) );
	ioValue. ExchangeAll ( CVSTR ( "\n" ),"\\n");
	ioValue. ExchangeAll ( CVSTR ( "\r" ),"\\r");
	ioValue. ExchangeAll ( CVSTR ( "\t" ),"\\t");

	//Replace all non printable chars (0-31) with \xdd except 0x0a, 0x0d and 0x09
	VIndex	len = ioValue.GetLength();
	for(long i =len-1;i >=0;i--)
	{
		UniChar car = ioValue[i];
		if(car >=0 && car < 32 && car != 0x09 && car != 0x0a && car != 0x0D)
		{
			//ioValue.Remove(i+1,1);
			char buf[5];	
			sprintf(buf,"\\x%02d",car);
			VString FormatedStr(buf);

			ioValue.Replace(FormatedStr,i+1,1);
		}
	}
}


ReplicationInputBinaryFormatter::ReplicationInputBinaryFormatter ( VStream& inStream ) :
																					fStream ( inStream )
{
	;
}

ReplicationInputBinaryFormatter::~ReplicationInputBinaryFormatter ( )
{
	;
}

ReplicationInputJSONFormatter::ReplicationInputJSONFormatter ( VStream& inStream ) :
																				fJSON ( )
{
	inStream. GetText ( fJSON );
	fjsonImporter = new VJSONImporter ( fJSON );
	fActionCount = 0;
	fCurrentAction = 0;
	fCurrentPK = 0;
	fCurrentField = 0;
}

ReplicationInputJSONFormatter::~ReplicationInputJSONFormatter ( )
{
	delete fjsonImporter;
}

void ReplicationInputJSONFormatter::SetPrimaryKey ( FieldArray const & inPK )
{
	for ( VIndex i = 1; i <= inPK. GetCount ( ); i++ )
	{
		/*VString			vstrName;
		inPK [ i ]-> GetName ( vstrName );
		EscapeJSONString ( vstrName );
		fPrimaryKey. push_back ( vstrName );*/
		fPKTypes. push_back ( inPK [ i ]-> GetTyp ( ) );
	}
}

void ReplicationInputJSONFormatter::SetFields ( Table* inTable, vector<sLONG> const & inFieldIDs )
{
	if ( inTable == 0 )
		return;

	Field*								fld = 0;
	vector<sLONG>::const_iterator		iter = inFieldIDs. begin ( );
	while ( iter != inFieldIDs. end ( ) )
	{
		fld = inTable-> RetainField ( *iter );
		if ( fld != 0 )
		{
			fFieldTypes. push_back ( fld-> GetTyp ( ) );
			fld-> Release ( );
		}
		iter++;
	}
}

VError ReplicationInputJSONFormatter::GetRowCount ( sLONG& outCount )
{
	VError							vError = VE_OK;
	VString							vstrToken;
	bool							bWithQuotes = false;
	VJSONImporter::JsonToken		jsonToken = VJSONImporter::jsonNone;

	jsonToken = fjsonImporter-> GetNextJSONToken ( ); // jsonBeginObject
	xbox_assert ( jsonToken == VJSONImporter::jsonBeginObject );

	vstrToken. Clear ( );
	jsonToken = fjsonImporter-> GetNextJSONToken ( vstrToken, &bWithQuotes ); // jsonString __SENT
	xbox_assert ( jsonToken == VJSONImporter::jsonString );
	xbox_assert ( vstrToken. EqualToString ( CVSTR ( "__SENT" ) ) );

	jsonToken = fjsonImporter-> GetNextJSONToken ( ); // jsonAssigne
	xbox_assert ( jsonToken == VJSONImporter::jsonAssigne );

	vstrToken. Clear ( );
	jsonToken = fjsonImporter-> GetNextJSONToken ( vstrToken, &bWithQuotes ); // Action count
	xbox_assert ( jsonToken == VJSONImporter::jsonString );
	fActionCount = vstrToken. GetLong ( );
	outCount = fActionCount;

	jsonToken = fjsonImporter-> GetNextJSONToken ( vstrToken, &bWithQuotes ); // jsonSeparator
	xbox_assert ( jsonToken == VJSONImporter::jsonSeparator );

	// Skip the latest stamp value; not used at the moment.
	vstrToken. Clear ( );
	jsonToken = fjsonImporter-> GetNextJSONToken ( vstrToken, &bWithQuotes ); // jsonString __LATEST_STAMP
	xbox_assert ( jsonToken == VJSONImporter::jsonString );
	xbox_assert ( vstrToken. EqualToString ( CVSTR ( "__LATEST_STAMP" ) ) );

	vstrToken. Clear ( );
	jsonToken = fjsonImporter-> GetNextJSONToken ( ); // jsonAssigne
	xbox_assert ( jsonToken == VJSONImporter::jsonAssigne );

	vstrToken. Clear ( );
	jsonToken = fjsonImporter-> GetNextJSONToken ( ); // jsonString Latest stamp value
	xbox_assert ( jsonToken == VJSONImporter::jsonString );

	vstrToken. Clear ( );
	jsonToken = fjsonImporter-> GetNextJSONToken ( ); // jsonSeparator
	xbox_assert ( jsonToken == VJSONImporter::jsonSeparator );

	vstrToken. Clear ( );
	jsonToken = fjsonImporter-> GetNextJSONToken ( vstrToken, &bWithQuotes ); // jsonString __ACTIONS
	xbox_assert ( jsonToken == VJSONImporter::jsonString );
	xbox_assert ( vstrToken. EqualToString ( CVSTR ( "__ACTIONS" ) ) );

	vstrToken. Clear ( );
	jsonToken = fjsonImporter-> GetNextJSONToken ( ); // jsonAssigne
	xbox_assert ( jsonToken == VJSONImporter::jsonAssigne );

	vstrToken. Clear ( );
	jsonToken = fjsonImporter-> GetNextJSONToken ( ); // jsonBeginArray
	xbox_assert ( jsonToken == VJSONImporter::jsonBeginArray );

	return vError;
}

VError ReplicationInputJSONFormatter::GetAction ( sBYTE& outAction )
{
	VError							vError = VE_OK;

	if ( fCurrentAction == fActionCount )
	{
		outAction = 0;

		return VE_OK;
	}

	fCurrentAction++;

	VString							vstrToken;
	bool							bWithQuotes = false;
	VJSONImporter::JsonToken		jsonToken = VJSONImporter::jsonNone;

	if ( fCurrentAction > 1 )
	{
		jsonToken = fjsonImporter-> GetNextJSONToken ( ); // jsonSeparator
		xbox_assert ( jsonToken == VJSONImporter::jsonSeparator );
	}

	jsonToken = fjsonImporter-> GetNextJSONToken ( ); // jsonBeginObject
	xbox_assert ( jsonToken == VJSONImporter::jsonBeginObject );

	jsonToken = fjsonImporter-> GetNextJSONToken ( vstrToken, &bWithQuotes ); // jsonString __ACTION
	xbox_assert ( jsonToken == VJSONImporter::jsonString );
	xbox_assert ( vstrToken. EqualToString ( CVSTR ( "__ACTION" ) ) );

	vstrToken. Clear ( );
	jsonToken = fjsonImporter-> GetNextJSONToken ( ); // jsonAssigne
	xbox_assert ( jsonToken == VJSONImporter::jsonAssigne );

	vstrToken. Clear ( );
	jsonToken = fjsonImporter-> GetNextJSONToken ( vstrToken, &bWithQuotes ); // jsonString ( update or delete )
	xbox_assert ( jsonToken == VJSONImporter::jsonString );
	xbox_assert ( vstrToken. EqualToString ( CVSTR ( "update" ) ) || vstrToken. EqualToString ( CVSTR ( "delete" ) ) );
	if ( vstrToken. EqualToString ( CVSTR ( "update" ) ) )
		outAction = DB4D_SyncAction_Update;
	else
		outAction = DB4D_SyncAction_Delete;

	jsonToken = fjsonImporter-> GetNextJSONToken ( ); // jsonSeparator
	xbox_assert ( jsonToken == VJSONImporter::jsonSeparator );

	return vError;
}

VError ReplicationInputJSONFormatter::GetPrimaryKeyCount ( sBYTE& outCount )
{
	VError							vError = VE_OK;

	VString							vstrToken;
	bool							bWithQuotes = false;
	VJSONImporter::JsonToken		jsonToken = VJSONImporter::jsonNone;

	jsonToken = fjsonImporter-> GetNextJSONToken ( vstrToken, &bWithQuotes ); // jsonString __PRIMARY_KEY_COUNT
	xbox_assert ( jsonToken == VJSONImporter::jsonString );
	xbox_assert ( vstrToken. EqualToString ( CVSTR ( "__PRIMARY_KEY_COUNT" ) ) );

	jsonToken = fjsonImporter-> GetNextJSONToken ( ); // jsonAssigne
	xbox_assert ( jsonToken == VJSONImporter::jsonAssigne );

	vstrToken. Clear ( );
	jsonToken = fjsonImporter-> GetNextJSONToken ( vstrToken, &bWithQuotes ); // jsonString PK count
	xbox_assert ( jsonToken == VJSONImporter::jsonString );
	outCount = vstrToken. GetLong ( );

	jsonToken = fjsonImporter-> GetNextJSONToken ( ); // jsonSeparator
	xbox_assert ( jsonToken == VJSONImporter::jsonSeparator );

	vstrToken. Clear ( );
	jsonToken = fjsonImporter-> GetNextJSONToken ( vstrToken, &bWithQuotes ); // jsonString __PRIMARY_KEY
	xbox_assert ( jsonToken == VJSONImporter::jsonString );
	xbox_assert ( vstrToken. EqualToString ( CVSTR ( "__PRIMARY_KEY" ) ) );

	jsonToken = fjsonImporter-> GetNextJSONToken ( ); // jsonAssigne
	xbox_assert ( jsonToken == VJSONImporter::jsonAssigne );

	jsonToken = fjsonImporter-> GetNextJSONToken ( ); // jsonBeginObject
	xbox_assert ( jsonToken == VJSONImporter::jsonBeginObject );

	return vError;
}

VError ReplicationInputJSONFormatter::GetPrimaryKeyType ( uWORD& outType )
{
	VString							vstrToken;
	bool							bWithQuotes = false;
	VJSONImporter::JsonToken		jsonToken = VJSONImporter::jsonNone;

	jsonToken = fjsonImporter-> GetNextJSONToken ( vstrToken, &bWithQuotes ); // jsonString PK field name
	xbox_assert ( jsonToken == VJSONImporter::jsonString );

	jsonToken = fjsonImporter-> GetNextJSONToken ( ); // jsonAssigne
	xbox_assert ( jsonToken == VJSONImporter::jsonAssigne );

	fCurrentValue. Clear ( );
	jsonToken = fjsonImporter-> GetNextJSONToken ( fCurrentValue, &bWithQuotes ); // jsonString PK field value
	xbox_assert ( jsonToken == VJSONImporter::jsonString );

	outType = fPKTypes [ fCurrentPK ];
	fCurrentPK++;
	if ( fCurrentPK == fPKTypes. size ( ) )
	{
		fCurrentPK = 0;

		jsonToken = fjsonImporter-> GetNextJSONToken ( ); // jsonEndObject
		xbox_assert ( jsonToken == VJSONImporter::jsonEndObject );
	}
	jsonToken = fjsonImporter-> GetNextJSONToken ( ); // jsonSeparator
	xbox_assert ( jsonToken == VJSONImporter::jsonSeparator );

	return VE_OK;
}

VError ReplicationInputJSONFormatter::GetVValue ( VValueSingle& outValue )
{
	if ( outValue. GetValueKind ( ) == VK_TIME )
		( ( VTime& ) outValue ). FromRfc822String ( fCurrentValue );
	else
		outValue. FromString ( fCurrentValue );

	return VE_OK;
}

VError ReplicationInputJSONFormatter::GetStamp ( uLONG8& outStamp )
{
	VString							vstrToken;
	bool							bWithQuotes = false;
	VJSONImporter::JsonToken		jsonToken = VJSONImporter::jsonNone;

	jsonToken = fjsonImporter-> GetNextJSONToken ( vstrToken, &bWithQuotes ); // jsonString __STAMP
	xbox_assert ( jsonToken == VJSONImporter::jsonString );
	xbox_assert ( vstrToken. EqualToString ( CVSTR ( "__STAMP" ) ) );

	jsonToken = fjsonImporter-> GetNextJSONToken ( ); // jsonAssigne
	xbox_assert ( jsonToken == VJSONImporter::jsonAssigne );

	vstrToken. Clear ( );
	jsonToken = fjsonImporter-> GetNextJSONToken ( vstrToken, &bWithQuotes ); // jsonString Stamp value
	xbox_assert ( jsonToken == VJSONImporter::jsonString );
	outStamp = vstrToken. GetLong8 ( );

	jsonToken = fjsonImporter-> GetNextJSONToken ( ); // jsonSeparator
	xbox_assert ( jsonToken == VJSONImporter::jsonSeparator );

	return VE_OK;
}

VError ReplicationInputJSONFormatter::GetTimeStamp ( VTime& outTimeStamp )
{
	VString							vstrToken;
	bool							bWithQuotes = false;
	VJSONImporter::JsonToken		jsonToken = VJSONImporter::jsonNone;

	jsonToken = fjsonImporter-> GetNextJSONToken ( vstrToken, &bWithQuotes ); // jsonString __TIMESTAMP
	xbox_assert ( jsonToken == VJSONImporter::jsonString );
	xbox_assert ( vstrToken. EqualToString ( CVSTR ( "__TIMESTAMP" ) ) );

	jsonToken = fjsonImporter-> GetNextJSONToken ( ); // jsonAssigne
	xbox_assert ( jsonToken == VJSONImporter::jsonAssigne );

	vstrToken. Clear ( );
	jsonToken = fjsonImporter-> GetNextJSONToken ( vstrToken, &bWithQuotes ); // jsonString Timestamp value
	xbox_assert ( jsonToken == VJSONImporter::jsonString );
	outTimeStamp. FromRfc822String ( vstrToken );

	jsonToken = fjsonImporter-> GetNextJSONToken ( ); // jsonSeparator
	xbox_assert ( jsonToken == VJSONImporter::jsonSeparator || jsonToken == VJSONImporter::jsonEndObject );

	return VE_OK;
}

VError ReplicationInputJSONFormatter::GetFieldCount ( sLONG& outCount )
{
	VString							vstrToken;
	bool							bWithQuotes = false;
	VJSONImporter::JsonToken		jsonToken = VJSONImporter::jsonNone;

	jsonToken = fjsonImporter-> GetNextJSONToken ( vstrToken, &bWithQuotes ); // jsonString __FIELD_COUNT
	xbox_assert ( jsonToken == VJSONImporter::jsonString );
	xbox_assert ( vstrToken. EqualToString ( CVSTR ( "__FIELD_COUNT" ) ) );

	jsonToken = fjsonImporter-> GetNextJSONToken ( ); // jsonAssigne
	xbox_assert ( jsonToken == VJSONImporter::jsonAssigne );

	vstrToken. Clear ( );
	jsonToken = fjsonImporter-> GetNextJSONToken ( vstrToken, &bWithQuotes ); // jsonString Field count value
	xbox_assert ( jsonToken == VJSONImporter::jsonString );
	outCount = vstrToken. GetLong ( );

	jsonToken = fjsonImporter-> GetNextJSONToken ( ); // jsonSeparator
	xbox_assert ( jsonToken == VJSONImporter::jsonSeparator );

	vstrToken. Clear ( );
	jsonToken = fjsonImporter-> GetNextJSONToken ( vstrToken, &bWithQuotes ); // jsonString __VALUES
	xbox_assert ( jsonToken == VJSONImporter::jsonString );
	xbox_assert ( vstrToken. EqualToString ( CVSTR ( "__VALUES" ) ) );

	jsonToken = fjsonImporter-> GetNextJSONToken ( ); // jsonAssigne
	xbox_assert ( jsonToken == VJSONImporter::jsonAssigne );

	vstrToken. Clear ( );
	jsonToken = fjsonImporter-> GetNextJSONToken ( ); // jsonBeginObject
	xbox_assert ( jsonToken == VJSONImporter::jsonBeginObject );

	return VE_OK;
}

VError ReplicationInputJSONFormatter::GetFieldType ( uWORD& outType )
{
	VString							vstrToken;
	bool							bWithQuotes = false;
	VJSONImporter::JsonToken		jsonToken = VJSONImporter::jsonNone;

	jsonToken = fjsonImporter-> GetNextJSONToken ( vstrToken, &bWithQuotes ); // jsonString Column's name
	xbox_assert ( jsonToken == VJSONImporter::jsonString );

	jsonToken = fjsonImporter-> GetNextJSONToken ( ); // jsonAssigne
	xbox_assert ( jsonToken == VJSONImporter::jsonAssigne );

	fCurrentValue. Clear ( );
	jsonToken = fjsonImporter-> GetNextJSONToken ( fCurrentValue, &bWithQuotes ); // jsonString First column's value
	xbox_assert ( jsonToken == VJSONImporter::jsonString );

	if ( vstrToken. EqualToString ( CVSTR ( "null" ) ) && !bWithQuotes )
	{
		outType = VK_EMPTY;
		fCurrentValue. Clear ( );
	}
	else
	{
		outType = fFieldTypes [ fCurrentField ];
		//TODO: Remove quotes if present
	}

	bWithQuotes = false;
	fCurrentField++;
	if ( fCurrentField == fFieldTypes. size ( ) )
	{
		fCurrentField = 0;

		jsonToken = fjsonImporter-> GetNextJSONToken ( ); // jsonEndObject
		xbox_assert ( jsonToken == VJSONImporter::jsonEndObject );

		jsonToken = fjsonImporter-> GetNextJSONToken ( ); // jsonEndObject
		xbox_assert ( jsonToken == VJSONImporter::jsonEndObject );
	}
	else
	{
		jsonToken = fjsonImporter-> GetNextJSONToken ( ); // jsonSeparator
		xbox_assert ( jsonToken == VJSONImporter::jsonSeparator );
	}

	/*
	vstrToken. Clear ( );
	jsonToken = fjsonImporter-> GetNextJSONToken ( vstrToken, &bWithQuotes ); // jsonSeparator
	vstrToken. Clear ( );
	jsonToken = fjsonImporter-> GetNextJSONToken ( vstrToken, &bWithQuotes ); // jsonString Second column's name
	vstrToken. Clear ( );
	jsonToken = fjsonImporter-> GetNextJSONToken ( vstrToken, &bWithQuotes ); // jsonAssigne

	vstrToken. Clear ( );
	jsonToken = fjsonImporter-> GetNextJSONToken ( vstrToken, &bWithQuotes ); // jsonString Second column's value
	vstrToken. Clear ( );
	jsonToken = fjsonImporter-> GetNextJSONToken ( vstrToken, &bWithQuotes ); // jsonEndObject
	vstrToken. Clear ( );
	jsonToken = fjsonImporter-> GetNextJSONToken ( vstrToken, &bWithQuotes ); // jsonEndObject
	vstrToken. Clear ( );
	jsonToken = fjsonImporter-> GetNextJSONToken ( vstrToken, &bWithQuotes ); // jsonSeparator


	vstrToken. Clear ( );
	jsonToken = fjsonImporter-> GetNextJSONToken ( vstrToken, &bWithQuotes ); // jsonBeginObject
	*/

	return VE_OK;
}

