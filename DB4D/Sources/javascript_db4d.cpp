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

#include "JavaScript/VJavaScript.h"

#include "javascript_db4d.h"



USING_TOOLBOX_NAMESPACE
using namespace std;






CDB4DContext *GetDB4DContextFromJSContext( const XBOX::VJSContext& inContext)
{
	return static_cast<CDB4DContext*>( inContext.GetGlobalObjectPrivateInstance()->GetSpecific( 'db4d'));
}


void SetDB4DContextInJSContext( const XBOX::VJSContext& inContext, CDB4DContext* inDB4DContext)
{
	if (inContext.GetGlobalObjectPrivateInstance()->SetSpecific( 'db4d', inDB4DContext, VJSSpecifics::DestructorReleaseCComponent))
	{
		if (inDB4DContext != NULL)
			inDB4DContext->Retain();
	}
}


CDB4DBaseContext *GetDB4DBaseContextFromJSContext( const XBOX::VJSContext& inContext, CDB4DBase* inBase)
{
	CDB4DContext *db4dContext = GetDB4DContextFromJSContext( inContext);
	if (db4dContext != NULL)
		return db4dContext->RetainDataBaseContext( inBase, true, false);

	return NULL;
}


inline CDB4DBaseContext* GetDB4DBaseContextFromJSContext( const XBOX::VJSContext& inContext, CDB4DTable* inTable)
{
	return GetDB4DBaseContextFromJSContext( inContext, inTable->GetOwner());
}


inline CDB4DBaseContext* GetDB4DBaseContextFromJSContext( const XBOX::VJSContext& inContext, CDB4DEntityModel* em)
{
	CDB4DBase* base = em->RetainDataBase();
	CDB4DBaseContext* result = GetDB4DBaseContextFromJSContext( inContext, base);
	base->Release();
	return result;
}


inline CDB4DBaseContext* GetDB4DBaseContextFromJSContext( const XBOX::VJSContext& inContext, CDB4DSelection* inSel)
{
	return GetDB4DBaseContextFromJSContext( inContext, inSel->GetBaseRef());
}


template<class T>
static CDB4DBaseContext* GetDB4DBaseContextFromJSContext( const VJSParms_withContext& inParms, T* inParam)
{
	return GetDB4DBaseContextFromJSContext( inParms.GetContext(), inParam);
}



sLONG _GetStringAsIndexType(const VString& propname)
{
	sLONG result = 0;
	if (propname == L"Btree")
		result = DB4D_Index_Btree;
	else if (propname == L"Cluster")
		result = DB4D_Index_BtreeWithCluster;
	else if (propname == L"Btree_Cluster")
		result = DB4D_Index_BtreeWithCluster;
	return result;
}


VError getQParams(VJSParms_callStaticFunction& ioParms, sLONG firstparam, QueryParamElementVector& outParams, CDB4DQuery* inQuery)
{
	VError err = VE_OK;
	SearchTab* query = VImpCreator<VDB4DQuery>::GetImpObject(inQuery)->GetSearchTab();
	for (sLONG i = firstparam, nb = ioParms.CountParams(); i <= nb; i++)
	{
		if (ioParms.IsObjectParam(i))
		{
			if (!ioParms.IsNullParam(i))
			{
				VJSObject obj(ioParms.GetParamValue(i).GetObject());
				if (obj.IsArray())
				{
					VJSArray arr(obj, false);
					outParams.push_back(QueryParamElement(arr));
				}
				else
				{
					if (obj.HasProperty("allowJavascript"))
					{
						bool allowjs = obj.GetPropertyAsBool("allowJavascript", nil, nil);
						query->AllowJSCode(allowjs);
					}
					if (obj.HasProperty("queryPlan"))
					{
						bool queryplan = obj.GetPropertyAsBool("queryPlan", nil, nil);
					}
					if (obj.HasProperty("queryPath"))
					{
						bool querypath = obj.GetPropertyAsBool("queryPath", nil, nil);	
					}

				}
			}
		}
		else if (!ioParms.IsNullParam(i))
		{
			VJSValue jsval(ioParms.GetParamValue(i));
			VValueSingle* cv = jsval.CreateVValue();
			if (cv == nil)
			{
				cv = new VString();
				cv->SetNull(true);
			}
			outParams.push_back(QueryParamElement(cv));
		}
	}
	return err;
}


VError FillQueryWithParams(CDB4DQuery* query, VJSParms_callStaticFunction& ioParms, sLONG firstparam)
{
	vector<VString> ParamNames;
	QueryParamElementVector ParamValues;
	
	VError err = query->GetParams(ParamNames, ParamValues);
	if (err == VE_OK)
	{
		QueryParamElementVector::iterator curvalue = ParamValues.begin();
		for (vector<VString>::iterator curname = ParamNames.begin(), endname = ParamNames.end(); curname != endname; curname++, curvalue++)
		{
			const VString& s = *curname;
			bool isAParam = false;
			if (s.GetLength() == 1)
			{
				UniChar c = s[0];
				if (c >= '1' && c <= '9')
				{
					isAParam = true;
					sLONG paramnum = s.GetLong() - 1 + firstparam;
					if (ioParms.CountParams() >= paramnum)
					{
						VJSValue jsval(ioParms.GetParamValue(paramnum));
						if (jsval.IsArray())
						{
							VJSArray jarr(jsval, false);
							curvalue->Dispose();
							*curvalue = QueryParamElement(jarr);
						}
						else
						{
							VValueSingle* cv = jsval.CreateVValue();
							if (cv != nil)
							{
								curvalue->Dispose();
								*curvalue = QueryParamElement(cv);
							}
						}
					}
				}
			}
			if (!isAParam)
			{
				curvalue->Dispose();
				VValueSingle* cv;
				ioParms.EvaluateScript(*curname, &cv);
				*curvalue = QueryParamElement(cv);
			}
		}
		err = query->SetParams(ParamNames, ParamValues);
	}
		
	return err;
}


sLONG GetAttributeListParams(XBOX::VJSParms_callStaticFunction& ioParms, sLONG StartParam, EntityAttributeSortedSelection& outList)
{
	if (ioParms.IsStringParam(StartParam))
	{
		VString s;
		ioParms.GetStringParam(StartParam, s);
		outList.BuildFromString(s, ConvertContext( GetDB4DBaseContextFromJSContext(ioParms, outList.GetModel())), true, false, nil);
		StartParam++;
	}
	else
	{
		while (StartParam <= ioParms.CountParams())
		{
			CDB4DEntityAttribute* att = ioParms.GetParamObjectPrivateData<VJSEntityAttribute>(StartParam);
			if (att != nil)
			{
				EntityAttribute* xatt = VImpCreator<EntityAttribute>::GetImpObject(att);
				outList.AddAttribute(xatt, nil);
				StartParam++;
			}
			else
				break;
		}

	}
	return StartParam;
}





//======================================================



EntitySelectionIterator::EntitySelectionIterator(CDB4DSelection* inSel, bool inReadOnly, bool inAutoSave, CDB4DBaseContext* inContext, CDB4DEntityModel* inModel)
{
	fReadOnly = inReadOnly;
	fAutoSave = inAutoSave;
	fSel = RetainRefCountable(inSel);
	fCurRec = nil;
	fCurPos = -1;
	fSelSize = fSel->CountRecordsInSelection(inContext);
	fModel = RetainRefCountable(inModel);
}


EntitySelectionIterator::EntitySelectionIterator(CDB4DEntityRecord* inRec, CDB4DBaseContext* inContext)
{
	fReadOnly = false;
	fAutoSave = true;
	fSel = nil;
	fCurRec = RetainRefCountable(inRec);
	fCurPos = -1;
	fSelSize = 0;
	fModel = RetainRefCountable(inRec->GetModel());
}

EntitySelectionIterator::EntitySelectionIterator(const EntitySelectionIterator& from)
{
	fReadOnly = from.fReadOnly;
	fAutoSave = from.fAutoSave;
	fSel = RetainRefCountable(from.fSel);
	fCurRec = RetainRefCountable(from.fCurRec);
	fCurPos = from.fCurPos;
	fSelSize = from.fSelSize;
	fModel = RetainRefCountable(from.fModel);
}

EntitySelectionIterator::~EntitySelectionIterator()
{
	ReleaseCurCurec(false);
	QuickReleaseRefCountable(fCurRec);
	QuickReleaseRefCountable(fSel);
	QuickReleaseRefCountable(fModel);
}


CDB4DEntityRecord* EntitySelectionIterator::GetCurRec(CDB4DBaseContext* inContext)
{
	if (fSel != nil)
	{
		if (fCurPos == -2)
		{
			ReleaseCurCurec(true);
			fCurPos = 0;
			if (fCurPos >= fSelSize)
				fCurPos = -1;
			if (fCurPos != -1)
				fCurRec = fSel->LoadEntity(fCurPos+1, /*fReadOnly ? DB4D_Do_Not_Lock : DB4D_Keep_Lock_With_Record*/DB4D_Do_Not_Lock, inContext);		
		}
	}
	
	return fCurRec;
}


VError EntitySelectionIterator::ReLoadCurRec(CDB4DBaseContext* inContext, bool readonly, bool canautosave)
{
	VError err = VE_OK;
	
	if (fSel != nil)
	{
		ReleaseCurCurec(canautosave);
		if (fCurPos == -2)
		{
			fCurPos = 0;
			if (fCurPos >= fSelSize)
				fCurPos = -1;
		}
		if (fCurPos != -1)
			fCurRec = fSel->LoadEntity(fCurPos+1, /*readonly ? DB4D_Do_Not_Lock : DB4D_Keep_Lock_With_Record*/DB4D_Do_Not_Lock, inContext);		
	}
	else
	{
		if (fCurRec != nil)
		{
			sLONG curnum = fCurRec->GetNum();
			if (curnum >= 0)
			{
				fCurRec->Release();
				fCurRec = fModel->LoadEntity(curnum, err, DB4D_Do_Not_Lock, inContext, false);
			}
		}
	}
	
	return err ;	
}


sLONG EntitySelectionIterator::GetCurRecID()
{
	if (fSel == nil)
		return fCurRec->GetNum();
	else
	{
		if (fCurPos == -2)
		{
			if (fSelSize == 0)
				return -1;
			else
				return fSel->GetSelectedRecordID(1, nil);
		}
		else
		{
			if (fCurPos == -1)
				return -1;
			else
				return fSel->GetSelectedRecordID(fCurPos+1, nil);
		}
	}
}


void EntitySelectionIterator::ReleaseCurCurec(bool canautosave)
{
	//if (fSel != nil)
	{
		if (fCurRec != nil)
		{
			if (/*!fReadOnly && */fAutoSave && canautosave)
			{
				if (fCurRec->IsModified())
				{
					StErrorContextInstaller errs(false);
					sLONG stamp = fCurRec->GetModificationStamp();
					if (stamp == 0)
						stamp = 1;
					fCurRec->Save(stamp);
				}
			}
			
			//fCurRec->ReleaseExtraDatas();
			fCurRec->Release();
			fCurRec = nil;
		}
	}
}


void EntitySelectionIterator::NextNotNull(CDB4DBaseContext* inContext)
{
	VError err;
	if (fSel != nil)
	{
		if (fCurPos == -2)
			fCurPos = 0;
		else if (fCurPos != -1)
			++fCurPos;
		if (fCurPos >= fSelSize)
			fCurPos = -1;
		bool stop = false;
		EntityModel* em = VImpCreator<EntityModel>::GetImpObject(fModel);
		do 
		{
			if (fCurPos != -1)
			{
				sLONG recnum = fSel->GetSelectedRecordID(fCurPos+1, inContext);
				if (recnum >= 0)
				{
					sLONG len = 0;
					DataAddr4D ou = em->GetMainTable()->GetDF()->GetRecordPos(recnum, len, err);
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
				++fCurPos;
				if (fCurPos >= fSelSize)
				{
					fCurPos = -1;
					stop = true;
				}
			}

		} while (!stop);
	}
}


void EntitySelectionIterator::First(CDB4DBaseContext* inContext)
{
	ReleaseCurCurec(true);
	/*
	 if (fSelSize > 0)
	 {
	 fCurPos = 0;
	 fCurRec = fSel->LoadSelectedRecord(fCurPos+1, fReadOnly ? DB4D_Do_Not_Lock : DB4D_Keep_Lock_With_Record, inContext, false);
	 }
	 else
	 */
	fCurPos = -2;
	NextNotNull(inContext);
	ReLoadCurRec(inContext, fReadOnly, fAutoSave); 
	
}

void EntitySelectionIterator::Next(CDB4DBaseContext* inContext)
{
	if (fSel != nil)
	{
		ReleaseCurCurec(true);
		/*
		if (fCurPos == -2)
			fCurPos = 0;
		else
		{
			if (fCurPos != -1)
				fCurPos++;
		}
		*/
		NextNotNull(inContext);

		{
			if (fCurPos >= fSelSize || fCurPos == -1)
			{
				fCurPos = -1;
			}
			else
			{
				fCurRec = fSel->LoadEntity(fCurPos+1, /*fReadOnly ? DB4D_Do_Not_Lock : DB4D_Keep_Lock_With_Record*/DB4D_Do_Not_Lock, inContext);			
			}
		}
	}
}




//======================================================


JSCollectionManager::JSCollectionManager(JS4D::ContextRef inContext)
{
	fContextRef = inContext;
	fSize = 0;
}


JSCollectionManager::~JSCollectionManager()
{
	for (vector<VJSArray>::iterator cur = fValues.begin(), end = fValues.end(); cur != end; cur++)
	{
		JS4D::UnprotectValue(fContextRef, *cur);
	}
}


VErrorDB4D JSCollectionManager::SetCollectionSize(RecIDType size, Boolean ClearData)
{
	for (vector<VJSArray>::iterator cur = fValues.begin(), end = fValues.end(); cur != end; cur++)
	{
		if (size > fSize)
		{
			/*
			VJSValue nullval(fContextRef);
			nullval.SetNull();
			cur->SetValueAt(size, nullval);
			*/
		}
		else if (size < fSize)
		{
			if (size == 0)
			{
				cur->Clear();
			}
			else
			{
				cur->Remove(size, fSize-size);
			}
		}
	}
	fSize = size;
	return VE_OK;
}


RecIDType JSCollectionManager::GetCollectionSize()
{
	return fSize;
}


sLONG JSCollectionManager::GetNumberOfColumns()
{
	return (sLONG)fValues.size();
}


bool JSCollectionManager::AcceptRawData()
{
	return false;
}

CDB4DField* JSCollectionManager::GetColumnRef(sLONG ColumnNumber)
{
	return nil;
}

VErrorDB4D JSCollectionManager::SetNthElement(RecIDType ElemNumber, sLONG ColumnNumber, const XBOX::VValueSingle& inValue)
{
	VJSValue val(fContextRef);
	val.SetVValue(inValue);
	fValues[ColumnNumber-1].SetValueAt(ElemNumber-1, val);
	return VE_OK;
}

XBOX::VSize JSCollectionManager::GetNthElementSpace( RecIDType inIndex, sLONG inColumnNumber, XBOX::ValueKind inRawValueKind, bool *outRejected, XBOX::VError& outError)
{
	return 0;
}

void* JSCollectionManager::WriteNthElementToPtr( RecIDType inIndex, sLONG inColumnNumber, void *outRawValue, XBOX::ValueKind inRawValueKind, bool *outRejected, XBOX::VError& outError)
{
	return nil;
}

// set *outRejected to true if you are not pleased with given raw data and want to get called with SetNthElement instead for this row (already initialized to false)
VErrorDB4D JSCollectionManager::SetNthElementRawData(RecIDType ElemNumber, sLONG ColumnNumber, const void *inRawValue, XBOX::ValueKind inRawValueKind, bool *outRejected)
{
	if (outRejected != nil)
		*outRejected = true;
	return VE_OK;
}

VErrorDB4D JSCollectionManager::GetNthElement(RecIDType ElemNumber, sLONG ColumnNumber, const XBOX::VValueSingle*& outValue, bool *outDisposeIt)
{
	VJSValue val(fValues[ColumnNumber-1].GetValueAt(ElemNumber-1));
	outValue = val.CreateVValue();
	return VE_OK;
}


VErrorDB4D JSCollectionManager::AddOneElement(sLONG ColumnNumber, const XBOX::VValueSingle& inValue)
{
	VJSValue val(fContextRef);
	val.SetVValue(inValue);
	fValues[ColumnNumber-1].PushValue(val);
	fSize++;
	return VE_OK;
}


sLONG JSCollectionManager::GetSignature()
{
	return 'jsAr';
}


XBOX::VError JSCollectionManager::PutInto(XBOX::VStream& outStream)
{
	return ThrowBaseError(VE_DB4D_NOTIMPLEMENTED);
}


void JSCollectionManager::SetNumberOfColumn(sLONG nb)
{
	for (sLONG i =0; i < nb; i++)
	{
		VJSArray arr(fContextRef);
		fValues.push_back(arr);
		JS4D::ProtectValue(fContextRef, fValues[i]);
	}
}




//======================================================




void VJSTable::Initialize( const VJSParms_initialize& inParms, CDB4DTable* inTable)
{
	inTable->Retain();
}


void VJSTable::Finalize( const VJSParms_finalize& inParms, CDB4DTable* inTable)
{
	inTable->Release();
}



void VJSTable::GetPropertyNames( VJSParms_getPropertyNames& ioParms, CDB4DTable* inTable)
{
	sLONG nb = inTable->CountFields();
	for (sLONG i = 1; i <= nb; i++)
	{
		VString fieldname;
		CDB4DField* ff = inTable->RetainNthField(i);
		if (ff != nil)
		{
			ff->GetName(fieldname);
			ioParms.AddPropertyName(fieldname);
			ff->Release();
		}
	}
}


void VJSTable::GetProperty( VJSParms_getProperty& ioParms, CDB4DTable* inTable)
{
	CDB4DField* ff = nil;
	
	sLONG num;
	if (ioParms.GetPropertyNameAsLong( &num))
	{
		ff = inTable->RetainNthField(num);
	}
	else
	{
		VString propname;
		ioParms.GetPropertyName( propname);
		ff = inTable->FindAndRetainField(propname);
	}
	if (ff != nil)
	{
		ioParms.ReturnValue(VJSField::CreateInstance(ioParms.GetContextRef(), ff));
		ff->Release();
	}
	else
	{
		//ioParms.ReturnValue(nil);
	}
}


void VJSTable::_GetID(VJSParms_callStaticFunction& ioParms, CDB4DTable* inTable)
{
	ioParms.ReturnNumber(inTable->GetID());
}


void VJSTable::_GetName(VJSParms_callStaticFunction& ioParms, CDB4DTable* inTable)
{
	XBOX::VString name;
	inTable->GetName( name);

	ioParms.ReturnString( name);
}


void VJSTable::_SetName(VJSParms_callStaticFunction& ioParms, CDB4DTable* inTable)
{
	XBOX::VString name;

	if (ioParms.CountParams() > 0)
	{
		if (ioParms.GetStringParam(1, name))
		{
			CDB4DBaseContext* context = NULL; // GetContext From IoParm
			VError err = inTable->SetName(name, context);
		}
	}

}


void VJSTable::_CountFields(VJSParms_callStaticFunction& ioParms, CDB4DTable* inTable)
{
	ioParms.ReturnNumber(inTable->CountFields());
}


void VJSTable::_Drop(VJSParms_callStaticFunction& ioParms, CDB4DTable* inTable)
{
	VError err = inTable->Drop(nil, nil);
}


void VJSTable::_keepSyncInfo(VJSParms_callStaticFunction& ioParms, CDB4DTable* inTable)
{
	bool keepinfo;
	VError err = VE_OK;
	if (ioParms.GetBoolParam(1, &keepinfo))
	{
		err = inTable->SetKeepRecordSyncInfo(keepinfo, nil, nil);
	}
}


void VJSTable::_dropPrimaryKey(VJSParms_callStaticFunction& ioParms, CDB4DTable* inTable)
{
	CDB4DFieldArray empty;
	VError err = inTable->SetPrimaryKey(empty, nil, false, nil, nil);
}


void VJSTable::_setPrimaryKey(VJSParms_callStaticFunction& ioParms, CDB4DTable* inTable)
{
	CDB4DFieldArray fields;
	XBOX::VString s = L"PK"+XBOX::ToString(inTable->GetID());

	sLONG nbparam = (sLONG) ioParms.CountParams();
	for (sLONG i = 1; i <= nbparam; i++)
	{
		CDB4DField* field = ioParms.GetParamObjectPrivateData<VJSField>(i);
		if (field != nil)
		{
			fields.Add(field);
		}
	}

	VError err = inTable->SetPrimaryKey(fields, nil, false, nil, &s);
}


void VJSTable::_CreateField(VJSParms_callStaticFunction& ioParms, CDB4DTable* inTable)
{
	sLONG nbparam = (sLONG) ioParms.CountParams();
	if (nbparam > 1)
	{
		VString fieldname;
		sLONG fieldtype;
		sLONG fieldsize = 30;
		bool isuniq = false;
		bool isnotnull = false;
		bool isautoinc = false;
		if (ioParms.GetStringParam(1, fieldname) && ioParms.GetLongParam(2, &fieldtype))
		{
			if (nbparam > 2)
			{
				sLONG xfieldsize;
				if (ioParms.GetLongParam(3, &xfieldsize))
					fieldsize = xfieldsize;
			}
			if (nbparam > 3)
			{
				for (sLONG i = 4; i <= nbparam; i++)
				{
					VString s;
					if (ioParms.GetStringParam(i, s))
					{
						if (s == L"Unique")
							isuniq = true;
						else if (s == L"Auto Increment")
							isautoinc = true;
						else if (s == L"Not Null")
							isnotnull = true;
					}
				}
			}
			
			sLONG attribute = 0;
			if (isuniq)
				attribute = attribute | DB4D_Unique;
			if (isautoinc)
				attribute = attribute | DB4D_AutoSeq;
			if (isnotnull)
				attribute = attribute | DB4D_Not_Null;
				
			VError err = inTable->AddField(fieldname, fieldtype, fieldsize, attribute);
		}
	}
	
}


void VJSTable::_setAutoSeqValue(XBOX::VJSParms_callStaticFunction& ioParms, CDB4DTable* inTable)
{
	Real val;
	if (ioParms.GetRealParam(1, &val))
	{
		CDB4DAutoSeqNumber* autoseq = inTable->RetainAutoSeqNumber(GetDB4DBaseContextFromJSContext(ioParms, inTable));
		if (autoseq != nil)
		{
			autoseq->SetCurrentValue((sLONG8)val);
			autoseq->Release();
		}
	}
}


void VJSTable::_isEntityModel(VJSParms_callStaticFunction& ioParms, CDB4DTable* inTable)
{
	ioParms.ReturnBool(false);
}



void VJSTable::GetDefinition( ClassDefinition& outDefinition)
{
	static inherited::StaticFunction functions[] = 
	{
		//{ "isEntityModel", js_callStaticFunction<_isEntityModel>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "getID", js_callStaticFunction<_GetID>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "getName", js_callStaticFunction<_GetName>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "setName", js_callStaticFunction<_SetName>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "countFields", js_callStaticFunction<_CountFields>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "drop", js_callStaticFunction<_Drop>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "remove", js_callStaticFunction<_Drop>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "createField", js_callStaticFunction<_CreateField>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "keepSyncInfo", js_callStaticFunction<_keepSyncInfo>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "removePrimaryKey", js_callStaticFunction<_dropPrimaryKey>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "dropPrimaryKey", js_callStaticFunction<_dropPrimaryKey>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "setPrimaryKey", js_callStaticFunction<_setPrimaryKey>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
/*
		{ "allRecords", js_callStaticFunction<_AllRecords>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "all", js_callStaticFunction<_AllRecords>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "query", js_callStaticFunction<_Query>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "find", js_callStaticFunction<_Find>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "newRecord", js_callStaticFunction<_NewRecord>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "newSelection", js_callStaticFunction<_NewSelection>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		*/
		{ "setAutoSeqValue", js_callStaticFunction<_setAutoSeqValue>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },

		{ 0, 0, 0}
	};

	outDefinition.className = "Table";
	outDefinition.initialize = js_initialize<Initialize>;
	outDefinition.finalize = js_finalize<Finalize>;
	outDefinition.getProperty = js_getProperty<GetProperty>;
	outDefinition.getPropertyNames = js_getPropertyNames<GetPropertyNames>;
	outDefinition.staticFunctions = functions;
}




//======================================================



void VJSField::Initialize( const VJSParms_initialize& inParms, CDB4DField* inField)
{
	inField->Retain();
}


void VJSField::Finalize( const VJSParms_finalize& inParms, CDB4DField* inField)
{
	inField->Release();
}


void VJSField::_GetName(VJSParms_callStaticFunction& ioParms, CDB4DField* inField)
{
	VString name;
	inField->GetName( name);

	ioParms.ReturnString( name);
}


void VJSField::_SetName(VJSParms_callStaticFunction& ioParms, CDB4DField* inField)
{
	VString name;

	if (ioParms.CountParams() > 0)
	{
		if (ioParms.GetStringParam(1, name))
		{
			CDB4DBaseContext* context = NULL; // GetContext From IoParm
			VError err = inField->SetName(name, context);
		}
	}

}


void VJSField::_Drop(VJSParms_callStaticFunction& ioParms, CDB4DField* inField)
{
	VError err = inField->Drop(nil, nil);
}



void VJSField::_CreateIndex(VJSParms_callStaticFunction& ioParms, CDB4DField* inField)
{
	VString indexname, indextype;
	sLONG indtype = DB4D_Index_AutoType;
	if (ioParms.GetStringParam(1, indextype))
	{
		indtype = _GetStringAsIndexType(indextype);
		if (indtype == 0)
			indtype = DB4D_Index_AutoType;
	}
	ioParms.GetStringParam(2, indexname);
	{
		VSyncEvent* indexevent = new VSyncEvent();
		VError err = inField->GetOwner()->GetOwner()->CreateIndexOnOneField(inField, indtype, false, nil, &indexname, nil, true, indexevent);
		indexevent->Lock();
		indexevent->Release();
	}
}


void VJSField::_DropIndex(VJSParms_callStaticFunction& ioParms, CDB4DField* inField)
{
	sLONG indtype = DB4D_Index_AutoType;
	VString indextype;
	if (ioParms.GetStringParam(1, indextype))
	{
		indtype = _GetStringAsIndexType(indextype);
		if (indtype == 0)
			indtype = DB4D_Index_AutoType;
	}	
	VSyncEvent* indexevent = new VSyncEvent();
	VError err = inField->GetOwner()->GetOwner()->DropIndexOnOneField(inField, indtype, nil, indexevent);
	indexevent->Lock();
	indexevent->Release();
}


void VJSField::_Create_FullText_Index(VJSParms_callStaticFunction& ioParms, CDB4DField* inField)
{
	VString indexname;
	ioParms.GetStringParam(1, indexname);
	VSyncEvent* indexevent = new VSyncEvent();
	VError err = inField->GetOwner()->GetOwner()->CreateFullTextIndexOnOneField(inField, nil, &indexname, nil, true, indexevent);
	indexevent->Lock();
	indexevent->Release();
	
}


void VJSField::_Drop_FullText_Index(VJSParms_callStaticFunction& ioParms, CDB4DField* inField)
{
	VSyncEvent* indexevent = new VSyncEvent();
	VError err = inField->GetOwner()->GetOwner()->DropFullTextIndexOnOneField(inField, nil, indexevent);
	indexevent->Lock();
	indexevent->Release();
}


void VJSField::_SetType(VJSParms_callStaticFunction& ioParms, CDB4DField* inField)
{
	if (ioParms.CountParams() > 0)
	{
		sLONG fieldtype;
		if (ioParms.GetLongParam(1, &fieldtype))
		{
			VError err = inField->SetType(fieldtype, nil, nil);
		}
	}
}


void VJSField::_GetType(VJSParms_callStaticFunction& ioParms, CDB4DField* inField)
{
	ioParms.ReturnNumber(inField->GetType(nil));
}


void VJSField::_GetID(VJSParms_callStaticFunction& ioParms, CDB4DField* inField)
{
	ioParms.ReturnNumber(inField->GetID(nil));
}


void VJSField::_IsIndexed(VJSParms_callStaticFunction& ioParms, CDB4DField* inField)
{
	ioParms.ReturnBool(inField->IsIndexed(nil));
}


void VJSField::_IsUnique(VJSParms_callStaticFunction& ioParms, CDB4DField* inField)
{
	ioParms.ReturnBool(inField->IsUnique(nil));
}


void VJSField::_Is_FullText_Indexed(VJSParms_callStaticFunction& ioParms, CDB4DField* inField)
{
	ioParms.ReturnBool(inField->IsFullTextIndexed(nil));
}


void VJSField::_IsNotNull(VJSParms_callStaticFunction& ioParms, CDB4DField* inField)
{
	ioParms.ReturnBool(inField->IsNot_Null(nil));
}


void VJSField::_IsAutoIncrement(VJSParms_callStaticFunction& ioParms, CDB4DField* inField)
{
	ioParms.ReturnBool(inField->IsAutoSequence(nil));
}


void VJSField::_SetNotNull(VJSParms_callStaticFunction& ioParms, CDB4DField* inField)
{
	if (ioParms.CountParams() > 0)
	{
		bool b;
		if (ioParms.GetBoolParam(1, &b))
		{
			VError err = inField->SetNot_Null(b, nil, nil);
		}
	}
}


void VJSField::_SetAutoIncrement(VJSParms_callStaticFunction& ioParms, CDB4DField* inField)
{
	if (ioParms.CountParams() > 0)
	{
		bool b;
		if (ioParms.GetBoolParam(1, &b))
		{
			VError err = inField->SetAutoSequence(b, nil);
		}
	}
}


void VJSField::_SetUnique(VJSParms_callStaticFunction& ioParms, CDB4DField* inField)
{
	if (ioParms.CountParams() > 0)
	{
		bool b;
		if (ioParms.GetBoolParam(1, &b))
		{
			VError err = inField->SetUnique(b, nil, nil);
		}
	}
}


void VJSField::_SetAutoGenerate(VJSParms_callStaticFunction& ioParms, CDB4DField* inField)
{
	if (ioParms.CountParams() > 0)
	{
		bool b;
		if (ioParms.GetBoolParam(1, &b))
		{
			VError err = inField->SetAutoGenerate(b, nil);
		}
	}
}


void VJSField::_SetStoredAsUUID(VJSParms_callStaticFunction& ioParms, CDB4DField* inField)
{
	if (ioParms.CountParams() > 0)
	{
		bool b;
		if (ioParms.GetBoolParam(1, &b))
		{
			VError err = inField->SetStoreAsUUID(b, nil);
		}
	}
}


void VJSField::_IsAutoGenerate(VJSParms_callStaticFunction& ioParms, CDB4DField* inField)
{
	ioParms.ReturnBool(inField->IsAutoGenerate(nil));
}


void VJSField::_IsStoredAsUUID(VJSParms_callStaticFunction& ioParms, CDB4DField* inField)
{
	ioParms.ReturnBool(inField->IsStoreAsUUID(nil));
}



void VJSField::_isStoredAsUTF8(VJSParms_callStaticFunction& ioParms, CDB4DField* inField)
{
	ioParms.ReturnBool(inField->IsStoreAsUTF8(nil));
}


void VJSField::_SetStoredAsUTF8(VJSParms_callStaticFunction& ioParms, CDB4DField* inField)
{
	if (ioParms.CountParams() > 0)
	{
		bool b;
		if (ioParms.GetBoolParam(1, &b))
		{
			VError err = inField->SetStoreAsUTF8(b, nil);
		}
	}
}


void VJSField::_isStoredOutside(VJSParms_callStaticFunction& ioParms, CDB4DField* inField)
{
	ioParms.ReturnBool(inField->IsStoreOutside(nil));
}


void VJSField::_setStoreOutside(VJSParms_callStaticFunction& ioParms, CDB4DField* inField)
{
	if (ioParms.CountParams() > 0)
	{
		bool b;
		if (ioParms.GetBoolParam(1, &b))
		{
			VError err = inField->SetStoreOutside(b, nil, nil);
		}
	}
}


void VJSField::GetDefinition( ClassDefinition& outDefinition)
{
	static inherited::StaticFunction functions[] = 
	{
		{ "getID", js_callStaticFunction<_GetID>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "getName", js_callStaticFunction<_GetName>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "setName", js_callStaticFunction<_SetName>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "remove", js_callStaticFunction<_Drop>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "drop", js_callStaticFunction<_Drop>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "createIndex", js_callStaticFunction<_CreateIndex>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "removeIndex", js_callStaticFunction<_DropIndex>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "dropIndex", js_callStaticFunction<_DropIndex>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "create_FullText_Index", js_callStaticFunction<_Create_FullText_Index>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "drop_FullText_Index", js_callStaticFunction<_Drop_FullText_Index>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "remove_FullText_Index", js_callStaticFunction<_Drop_FullText_Index>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "setType", js_callStaticFunction<_SetType>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "getType", js_callStaticFunction<_GetType>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "isIndexed", js_callStaticFunction<_IsIndexed>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "is_FullText_Indexed", js_callStaticFunction<_Is_FullText_Indexed>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "isUnique", js_callStaticFunction<_IsUnique>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "isNotNull", js_callStaticFunction<_IsNotNull>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "isAutoIncrement", js_callStaticFunction<_IsAutoIncrement>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "setNotNull", js_callStaticFunction<_SetNotNull>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "setAutoIncrement", js_callStaticFunction<_SetAutoIncrement>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "setUnique", js_callStaticFunction<_SetUnique>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "isAutoGenerate", js_callStaticFunction<_IsAutoGenerate>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "setAutoGenerate", js_callStaticFunction<_SetAutoGenerate>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "isStoredAsUUID", js_callStaticFunction<_IsStoredAsUUID>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "setStoredAsUUID", js_callStaticFunction<_SetStoredAsUUID>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "isStoredAsUTF8", js_callStaticFunction<_isStoredAsUTF8>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "setStoredAsUTF8", js_callStaticFunction<_SetStoredAsUTF8>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "isStoredOutside", js_callStaticFunction<_isStoredOutside>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "setStoreOutside", js_callStaticFunction<_setStoreOutside>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ 0, 0, 0}
	};

	outDefinition.className = "Field";
	outDefinition.initialize = js_initialize<Initialize>;
	outDefinition.finalize = js_finalize<Finalize>;
	outDefinition.staticFunctions = functions;
}



//======================================================


void VJSDatabase::Initialize( const VJSParms_initialize& inParms, CDB4DBase* inDatabase)
{
	inDatabase->Retain();
}


void VJSDatabase::Finalize( const VJSParms_finalize& inParms, CDB4DBase* inDatabase)
{
	inDatabase->Release();
}


VJSObject VJSDatabase::CreateJSEMObject( const VString& emName, const VJSContext& inContext, CDB4DBaseContext *inBaseContext)
{
	CDB4DBase* inDatabase = inBaseContext->GetOwner();
	CDB4DEntityModel* em = inDatabase->RetainEntityModel(emName, false);
	VJSObject result(inContext);
	if (em != nil)
	{
		EntityModel* xem = VImpCreator<EntityModel>::GetImpObject(em);
		if (xem->publishAsGlobal(inBaseContext))
			result = VJSEntityModel::CreateInstance(inContext, em);
		else
			result.SetUndefined();
		em->Release();
	}
	else
	{
		result.SetUndefined();
	}
	return result;
}


void VJSDatabase::PutAllModelsInGlobalObject(VJSObject& globalObject, CDB4DBase* inDatabase, CDB4DBaseContext* context)
{
	vector<VRefPtr<CDB4DEntityModel> > entities;
	inDatabase->RetainAllEntityModels(entities, context, false);
	for (vector<VRefPtr<CDB4DEntityModel> >::iterator cur = entities.begin(), end = entities.end(); cur != end; cur++)
	{
		CDB4DEntityModel* em = *cur;
		EntityModel* xem = VImpCreator<EntityModel>::GetImpObject(em);
		if (xem->publishAsGlobal(context))
		{
			VJSValue emVal = VJSEntityModel::CreateInstance(globalObject.GetContextRef(), em);
			globalObject.SetProperty(em->GetEntityName(), emVal);
		}
	}
}


void VJSDatabase::GetPropertyNames( VJSParms_getPropertyNames& ioParms, CDB4DBase* inDatabase)
{
	//set<VString> dejaName;
	//vector<CDB4DEntityModel*> entities;
	vector<VRefPtr<CDB4DEntityModel> > entities;
	inDatabase->RetainAllEntityModels(entities, GetDB4DBaseContextFromJSContext( ioParms, inDatabase), false);
	for (vector<VRefPtr<CDB4DEntityModel> >::iterator cur = entities.begin(), end = entities.end(); cur != end; cur++)
	{
		CDB4DEntityModel* em = *cur;
		ioParms.AddPropertyName(em->GetEntityName());
		//dejaName.insert(em->GetEntityName());
	}
	
#if AllowDefaultEMBasedOnTables
	sLONG nb = inDatabase->CountTables();
	for (sLONG i = 1; i <= nb; i++)
	{
		VString tablename;
		CDB4DTable* tt = inDatabase->RetainNthTable(i);
		if (tt != nil)
		{
			tt->GetName(tablename);
			if (!tablename.IsEmpty())
			{
				/*if (dejaName.find(tablename) == dejaName.end())
					ioParms.AddPropertyName(tablename);*/
				XBOX::VString entitytablename = L"$"+tablename;
				ioParms.AddPropertyName(entitytablename);
			}
			tt->Release();
		}
	}
#endif

}


void VJSDatabase::GetProperty( VJSParms_getProperty& ioParms, CDB4DBase* inDatabase)
{
	CDB4DTable* tt = nil;
	CDB4DEntityModel* em = nil;
	
	VString propname;
	{
		ioParms.GetPropertyName(propname);
		CDB4DEntityModel* em = inDatabase->RetainEntityModel(propname, false);
		if (em != nil)
		{
			ioParms.ReturnValue(VJSEntityModel::CreateInstance(ioParms.GetContextRef(), em));
			em->Release();
		}
		else
		{
			/*
			CDB4DEntityModel* em = inDatabase->RetainEntityModelBySingleEntityName(propname);
			if (em != nil)
			{
				ioParms.ReturnValue(VJSEntityModel::CreateInstance(ioParms.GetContextRef(), em));
				em->Release();
			}
			*/
		}
	}
	
}


void VJSDatabase::_setSortMaxMem(XBOX::VJSParms_callStaticFunction& ioParms, CDB4DBase* inDatabase)
{
	CDB4DManager* db4D = XBOX::VComponentManager::RetainComponentOfType<CDB4DManager>();
	if (db4D != nil)
	{
		Real memsize;
		if (ioParms.GetRealParam(1, &memsize))
		{
			db4D->SetLimitPerSort(memsize);
		}
		db4D->Release();
	}
}


void VJSDatabase::_setCacheSize(XBOX::VJSParms_callStaticFunction& ioParms, CDB4DBase* inDatabase)
{
	CDB4DManager* db4D = XBOX::VComponentManager::RetainComponentOfType<CDB4DManager>();
	if (db4D != nil)
	{
		Real cachesize;
		if (ioParms.GetRealParam(1, &cachesize))
		{
			XBOX::VSize deja;
			db4D->SetCacheSize(cachesize, ioParms.GetBoolParam(2, L"InPhysicalMemory", L""), &deja);
		}
		db4D->Release();
	}
}

void VJSDatabase::_getCacheSize(XBOX::VJSParms_callStaticFunction& ioParms, CDB4DBase* inDatabase)
{
	CDB4DManager* db4D = XBOX::VComponentManager::RetainComponentOfType<CDB4DManager>();
	if (db4D != nil)
	{
		ioParms.ReturnNumber(db4D->GetCacheSize());
		db4D->Release();
	}
}


void VJSDatabase::_loadModelsDefinition(XBOX::VJSParms_callStaticFunction& ioParms, CDB4DBase* inDatabase)
{
	VFile* file = ioParms.RetainFileParam(1);
	inDatabase->ReLoadEntityModels(file); // file peut etre null, c'est permis
	QuickReleaseRefCountable(file);
}


void VJSDatabase::_GetName(VJSParms_callStaticFunction& ioParms, CDB4DBase* inDatabase)
{
	VString name;
	inDatabase->GetName( name);
	
	ioParms.ReturnString( name);
}


void VJSDatabase::_SetName(VJSParms_callStaticFunction& ioParms, CDB4DBase* inDatabase)
{
	VString name;
	/*
	if (ioParms.CountParams() > 0)
	{
		if (ioParms.GetStringParam(1, name))
		{
			CDB4DBaseContext* context = NULL; // GetContext From IoParm
			VError err = inDatabase->SetName(name, context);
			if (err != VE_OK)
			{
				ioParms.CreateExceptionFromErrorStack();
			}
		}
	}
	 */
}


void VJSDatabase::_CountTables(VJSParms_callStaticFunction& ioParms, CDB4DBase* inDatabase)
{
	ioParms.ReturnNumber(inDatabase->CountTables(nil));
}


void VJSDatabase::_GetPath(VJSParms_callStaticFunction& ioParms, CDB4DBase* inDatabase)
{
	VString pathname;
	XBOX::VFilePath path;
	inDatabase->GetBasePath(path, nil);
	path.GetPath(pathname);
	ioParms.ReturnString(pathname);
}


void VJSDatabase::_CreateTable(VJSParms_callStaticFunction& ioParms, CDB4DBase* inDatabase)
{
	VString tablename;
	
	if (ioParms.CountParams() > 0)
	{
		if (ioParms.GetStringParam(1, tablename))
		{
			CDB4DTable* table;
			VValueBag def;
			def.SetString( L"name", tablename);
			VError err = inDatabase->CreateTable(def, &table);
			if (err != VE_OK)
			{
				//ioParms.ReturnValue(nil);
			}
			else
			{
				ioParms.ReturnValue(VJSTable::CreateInstance(ioParms.GetContextRef(), table));
			}
			QuickReleaseRefCountable(table);
			
		}
	}
			
}


void VJSDatabase::_CreateIndex(VJSParms_callStaticFunction& ioParms, CDB4DBase* inDatabase)
{
}


void VJSDatabase::_DropIndex(VJSParms_callStaticFunction& ioParms, CDB4DBase* inDatabase)
{
}


void VJSDatabase::_StartTransaction(VJSParms_callStaticFunction& ioParms, CDB4DBase* inDatabase)
{
	CDB4DBaseContext* context = GetDB4DBaseContextFromJSContext(ioParms, inDatabase);
	if (context != nil)
	{
		VError err = context->StartTransaction();
	}
}


void VJSDatabase::_Commit(VJSParms_callStaticFunction& ioParms, CDB4DBase* inDatabase)
{
	CDB4DBaseContext* context = GetDB4DBaseContextFromJSContext(ioParms, inDatabase);
	if (context != nil)
	{
		VError err = context->CommitTransaction();
	}
}


void VJSDatabase::_RollBack(VJSParms_callStaticFunction& ioParms, CDB4DBase* inDatabase)
{
	CDB4DBaseContext* context = GetDB4DBaseContextFromJSContext(ioParms, inDatabase);
	if (context != nil)
	{
		VError err = context->RollBackTransaction();
	}
}


void VJSDatabase::_TransactionLevel(VJSParms_callStaticFunction& ioParms, CDB4DBase* inDatabase)
{
	CDB4DBaseContext* context = GetDB4DBaseContextFromJSContext(ioParms, inDatabase);
	if (context != nil)
	{
		ioParms.ReturnNumber(context->CurrentTransactionLevel());
	}
}


void VJSDatabase::_GetStructure(VJSParms_callStaticFunction& ioParms, CDB4DBase* inDatabase)
{
	CDB4DBase* structure = inDatabase->RetainStructDatabase(nil);
	if (structure != nil)
	{
		ioParms.ReturnValue(VJSDatabase::CreateInstance(ioParms.GetContextRef(), structure));
		structure->Release();
	}
}


void VJSDatabase::_GetSyncInfo(VJSParms_callStaticFunction& ioParms, CDB4DBase* inDatabase)
{
	CDB4DBase* structure = inDatabase->RetainSyncDataBase();
	if (structure != nil)
	{
		ioParms.ReturnValue(VJSDatabase::CreateInstance(ioParms.GetContextRef(), structure));
		structure->Release();
	}
}


void VJSDatabase::_FlushCache(VJSParms_callStaticFunction& ioParms, CDB4DBase* inDatabase)
{
	bool waitUntilDone = false;
	ioParms.GetBoolParam(1, &waitUntilDone);
	inDatabase->Flush(waitUntilDone);
}


void VJSDatabase::_ExportAsSQL(VJSParms_callStaticFunction& ioParms, CDB4DBase* inDatabase)
{
	VError err = VE_OK;
	VFolder* folder = ioParms.RetainFolderParam( 1);
	if (folder != nil)
	{
		sLONG nbBlobsPerLevel = 200;
		Real maxSQLFileSize = 0;
		ioParms.GetLongParam(2, &nbBlobsPerLevel);
		ioParms.GetRealParam(3, &maxSQLFileSize);
		if (nbBlobsPerLevel <= 0)
			nbBlobsPerLevel = 200;
		ExportOption options;
		options.NbBlobsPerLevel = nbBlobsPerLevel;
		options.MaxSQLTextSize = maxSQLFileSize;
		err = inDatabase->ExportToSQL( GetDB4DBaseContextFromJSContext(ioParms, inDatabase), folder, nil, options);
		folder->Release();
	}
}


void VJSDatabase::_clearErrs(VJSParms_callStaticFunction& ioParms, CDB4DBase* inDatabase)
{
	XBOX::VTask::GetCurrent()->FlushErrors();
}


void VJSDatabase::_getTables( XBOX::VJSParms_getProperty& ioParms, CDB4DBase* inDatabase)
{
	ioParms.ReturnValue(VJSDatabaseTableEnumerator::CreateInstance(ioParms.GetContextRef(), inDatabase));

}


void VJSDatabase::_getEntityModels( XBOX::VJSParms_getProperty& ioParms, CDB4DBase* inDatabase)
{
	ioParms.ReturnValue(VJSDatabaseEMEnumerator::CreateInstance(ioParms.GetContextRef(), inDatabase));

}


void VJSDatabase::_getTempFolder( XBOX::VJSParms_callStaticFunction& ioParms, CDB4DBase* inDatabase)
{
	VFolder* ff = inDatabase->RetainTemporaryFolder(true);
	ioParms.ReturnFolder( ff);
	ReleaseRefCountable( &ff);
}



void VJSDatabase::_getDataFolder( XBOX::VJSParms_callStaticFunction& ioParms, CDB4DBase* inDatabase)
{
	VFolder* ff = inDatabase->RetainDataFolder();
	ioParms.ReturnFolder( ff);
	ReleaseRefCountable( &ff);
}


void VJSDatabase::_getCatalogFolder( XBOX::VJSParms_callStaticFunction& ioParms, CDB4DBase* inDatabase)
{
	VFolder* ff = inDatabase->RetainStructFolder();
	ioParms.ReturnFolder( ff);
	ReleaseRefCountable( &ff);
}


void VJSDatabase::_getCacheInfo( XBOX::VJSParms_callStaticFunction& ioParms, CDB4DBase* inDatabase)
{
	VMemStats stats;
	VDBMgr::GetManager()->GetCacheManager()->GetMemoryManager()->GetStatistics(stats);
	VJSObject result(ioParms.GetContextRef());
	result.MakeEmpty();
	VJSValue jsval(ioParms.GetContextRef());
	jsval.SetNumber(stats.fUsedMem);
	result.SetProperty(L"usedMem", jsval, JS4D::PropertyAttributeNone);
	jsval.SetNumber(stats.fFreeMem);
	result.SetProperty(L"freeMem", jsval, JS4D::PropertyAttributeNone);
	VJSArray objarr(ioParms.GetContextRef());
	for( VMapOfObjectInfo::const_iterator i = stats.fObjectInfo.begin() ; i != stats.fObjectInfo.end() ; ++i)
	{
		const std::type_info* typobj = i->first;
		VStr255 s;
		VSystem::DemangleSymbol( typobj->name(), s);
		VJSObject elem(ioParms.GetContextRef());
		elem.MakeEmpty();
		VJSValue elemval(ioParms.GetContextRef());
		elemval.SetString(s);
		elem.SetProperty("id", elemval, JS4D::PropertyAttributeNone);
		elemval.SetNumber(i->second.fCount);
		elem.SetProperty("count", elemval, JS4D::PropertyAttributeNone);
		elemval.SetNumber(i->second.fTotalSize);
		elem.SetProperty("size", elemval, JS4D::PropertyAttributeNone);
		objarr.PushValue(elem);
	}
	result.SetProperty("objects", objarr, JS4D::PropertyAttributeNone);

	VJSArray blockarr(ioParms.GetContextRef());
	for( VMapOfBlockInfo::const_iterator i = stats.fBlockInfo.begin() ; i != stats.fBlockInfo.end() ; ++i)
	{
		VStr4 s;
		s.FromOsType( i->first);
		VJSObject elem(ioParms.GetContextRef());
		elem.MakeEmpty();
		VJSValue elemval(ioParms.GetContextRef());
		elemval.SetString(s);
		elem.SetProperty("id", elemval, JS4D::PropertyAttributeNone);
		elemval.SetNumber(i->second.fCount);
		elem.SetProperty("count", elemval, JS4D::PropertyAttributeNone);
		elemval.SetNumber(i->second.fTotalSize);
		elem.SetProperty("size", elemval, JS4D::PropertyAttributeNone);
		blockarr.PushValue(elem);
	}
	result.SetProperty("blocks", blockarr, JS4D::PropertyAttributeNone);

	ioParms.ReturnValue(result);

}


void VJSDatabase::_getDBList( XBOX::VJSParms_callStaticFunction& ioParms, CDB4DBase* inDatabase)
{
	VJSArray arr(ioParms.GetContextRef());
	for (sLONG i = 1, nb = VDBMgr::GetManager()->CountBases(); i <= nb; i++)
	{
		CDB4DBase* base = VDBMgr::GetManager()->RetainNthBase(i);
		if (base != nil)
		{
			arr.PushValue(VJSDatabase::CreateInstance(ioParms.GetContextRef(), base));
			QuickReleaseRefCountable( base);
		}
	}
	ioParms.ReturnValue(arr);
}


void VJSDatabase::_freeCacheMem( XBOX::VJSParms_callStaticFunction& ioParms, CDB4DBase* inDatabase)
{
	VDBMgr::GetManager()->GetCacheManager()->GetMemoryManager()->PurgeMem();
}


void VJSDatabase::_getIndices( XBOX::VJSParms_callStaticFunction& ioParms, CDB4DBase* inDatabase)
{
	VJSArray arr(ioParms.GetContextRef());
	Base4D* base = VImpCreator<VDB4DBase>::GetImpObject(inDatabase)->GetBase();
	base->GetIndices(arr);
	ioParms.ReturnValue(arr);
}


void VJSDatabase::_close( XBOX::VJSParms_callStaticFunction& ioParms, CDB4DBase* inDatabase)
{
	bool simpleClose = true;
	if (ioParms.IsStringParam(1))
	{
		VString syncEventName;
		ioParms.GetStringParam(1, syncEventName);
		if (!syncEventName.IsEmpty())
		{
			jsSyncEvent* sync = jsSyncEvent::RetainSyncEvent(syncEventName);
			if (sync != nil)
			{
				simpleClose = false;
				VSyncEvent* waitclose = &(sync->fSync);
				inDatabase->Close(waitclose);
				ioParms.ReturnValue(VJSSyncEvent::CreateInstance(ioParms.GetContextRef(), sync));
				sync->Release();
			}
		}

	}

	if (simpleClose)
	{
		inDatabase->Close();
		ioParms.ReturnNullValue();
	}
}



class JSTOOLSIntf : public IDB4D_DataToolsIntf
{
	public :
		JSTOOLSIntf(JS4D::ContextRef jscontext, VJSObject& paramObj):fAddProblemFunc(jscontext), fOpenProgressionFunc(jscontext), 
			fCloseProgressionFunc(jscontext), fProgressFunc(jscontext), fSetProgressTitleFunc(jscontext), fParamObj(jscontext)
		{
			fJsContext = jscontext;
			fParamObj = paramObj;
			VJSValue v1(paramObj.GetProperty("addProblem"));
			if (v1.IsFunction())
			{
				fAddProblemFunc = v1.GetObject();
			}

			v1 = paramObj.GetProperty("openProgress");
			if (v1.IsFunction())
			{
				fOpenProgressionFunc = v1.GetObject();
			}

			v1 = paramObj.GetProperty("closeProgress");
			if (v1.IsFunction())
			{
				fCloseProgressionFunc = v1.GetObject();
			}

			v1 = paramObj.GetProperty("progress");
			if (v1.IsFunction())
			{
				fProgressFunc = v1.GetObject();
			}

			v1 = paramObj.GetProperty("setProgressTitle");
			if (v1.IsFunction())
			{
				fSetProgressTitleFunc = v1.GetObject();
			}
		}

		virtual VErrorDB4D AddProblem(const VValueBag& inProblemBag)
		{
			VError err = VE_OK;
			if (fAddProblemFunc.GetObjectRef() != nil)
			{
				VJSObject problem(fJsContext);
				problem.MakeEmpty();
				for (sLONG i = 1, nb = inProblemBag.GetAttributesCount(); i <= nb; i++)
				{
					VString attname;
					const VValueSingle* cv = inProblemBag.GetNthAttribute(i, &attname);
					if (cv != nil)
					{
						VJSValue jsval(fJsContext);
						jsval.SetVValue(*cv);
						problem.SetProperty(attname, jsval, JS4D::PropertyAttributeNone);
					}
				}

				JS4D::ExceptionRef excep;
				VJSValue result(fJsContext);
				vector<VJSValue> params;
				params.push_back(problem);
				fParamObj.CallFunction(fAddProblemFunc, &params, &result, &excep);
			}
			return err;
		}


		virtual VErrorDB4D OpenProgression(const VString inProgressTitle, sLONG8 inMaxElems)  // inMaxElems == -1 means non linear progression
		{
			VError err = VE_OK;
			if (fOpenProgressionFunc.GetObjectRef() != nil)
			{
				JS4D::ExceptionRef excep;
				VJSValue result(fJsContext);
				vector<VJSValue> params;
				VJSValue p1(fJsContext);
				p1.SetString(inProgressTitle);
				VJSValue p2(fJsContext);
				p2.SetNumber(inMaxElems);
				params.push_back(p1);
				params.push_back(p2);
				fParamObj.CallFunction(fOpenProgressionFunc, &params, &result, &excep);
			}
			return err;
		}


		virtual VErrorDB4D CloseProgression()  // OpenProgression can be called in nested levels
		{
			VError err = VE_OK;
			if (fCloseProgressionFunc.GetObjectRef() != nil)
			{
				JS4D::ExceptionRef excep;
				VJSValue result(fJsContext);
				vector<VJSValue> params;
				fParamObj.CallFunction(fCloseProgressionFunc, &params, &result, &excep);
			}
			return err;
		}


		virtual VErrorDB4D Progress(sLONG8 inCurrentValue, sLONG8 inMaxElems)
		{
			VError err = VE_OK;
			if (fProgressFunc.GetObjectRef() != nil)
			{
				JS4D::ExceptionRef excep;
				VJSValue result(fJsContext);
				vector<VJSValue> params;
				VJSValue p1(fJsContext);
				p1.SetNumber(inCurrentValue);
				VJSValue p2(fJsContext);
				p2.SetNumber(inMaxElems);
				params.push_back(p1);
				params.push_back(p2);
				fParamObj.CallFunction(fProgressFunc, &params, &result, &excep);
			}
			return err;
		}


		virtual VErrorDB4D SetProgressTitle(const VString inProgressTitle)
		{
			VError err = VE_OK;
			if (fSetProgressTitleFunc.GetObjectRef() != nil)
			{
				JS4D::ExceptionRef excep;
				VJSValue result(fJsContext);
				vector<VJSValue> params;
				VJSValue p1(fJsContext);
				p1.SetString(inProgressTitle);
				params.push_back(p1);
				fParamObj.CallFunction(fSetProgressTitleFunc, &params, &result, &excep);
			}
			return err;
		}



	protected:
		JS4D::ContextRef fJsContext;
		VJSObject fParamObj;
		VJSObject fAddProblemFunc;
		VJSObject fOpenProgressionFunc;
		VJSObject fCloseProgressionFunc;
		VJSObject fProgressFunc;
		VJSObject fSetProgressTitleFunc;
};



IDB4D_DataToolsIntf* VJSDatabase::CreateJSDataToolsIntf(VJSContext& jscontext, VJSObject& paramObj)
{
	return new JSTOOLSIntf(jscontext, paramObj);
}



void VJSDatabase::_verify( XBOX::VJSParms_callStaticFunction& ioParms, CDB4DBase* inDatabase)
{
	StErrorContextInstaller errs(false);
	bool ok = false;

	CDB4DManager* db4D = VComponentManager::RetainComponentOfType<CDB4DManager>();
	VError err = VE_OK;
	VJSObject paramObj(ioParms.GetContextRef());
	if (ioParms.IsObjectParam(1))
	{
		ioParms.GetParamObject(1, paramObj);
	}
	else
		paramObj.MakeEmpty();

	JSTOOLSIntf toolintf(ioParms.GetContextRef(), paramObj);
	CDB4DRawDataBase* dataDB = inDatabase->OpenRawDataBase(&toolintf, err);

	if (dataDB != nil)
	{
		err = dataDB->CheckAll(&toolintf);
		ok = err == VE_OK;
		dataDB->Release();
	}

	ioParms.ReturnBool(ok);
}



void VJSDatabase::_queryOptions( XBOX::VJSParms_callStaticFunction& ioParms, CDB4DBase* inDatabase)
{
	if (ioParms.IsObjectParam(1))
	{
		CDB4DBaseContext* context = GetDB4DBaseContextFromJSContext(ioParms, inDatabase);
		VJSObject options(ioParms.GetContextRef());
		ioParms.GetParamObject(1, options);
		bool exists;
		bool queryplan = options.GetPropertyAsBool("queryPlan", nil, &exists);
		bool querypath = options.GetPropertyAsBool("queryPath", nil, &exists);
		context->DescribeQueryExecution(queryplan || querypath);
	}
}

void VJSDatabase::GetDefinition( ClassDefinition& outDefinition)
{
	static inherited::StaticFunction functions[] = 
	{
		{ "getName", js_callStaticFunction<_GetName>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		//{ "setName", js_callStaticFunction<_SetName>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "countTables", js_callStaticFunction<_CountTables>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "getPath", js_callStaticFunction<_GetPath>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "createTable", js_callStaticFunction<_CreateTable>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "createIndex", js_callStaticFunction<_CreateIndex>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "dropIndex", js_callStaticFunction<_DropIndex>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "removeIndex", js_callStaticFunction<_DropIndex>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "startTransaction", js_callStaticFunction<_StartTransaction>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "commit", js_callStaticFunction<_Commit>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "rollBack", js_callStaticFunction<_RollBack>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "transactionLevel", js_callStaticFunction<_TransactionLevel>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "getStructure", js_callStaticFunction<_GetStructure>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "flushCache", js_callStaticFunction<_FlushCache>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "exportAsSQL", js_callStaticFunction<_ExportAsSQL>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "clearErrs", js_callStaticFunction<_clearErrs>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "getSyncInfo", js_callStaticFunction<_GetSyncInfo>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "loadModelDefinition", js_callStaticFunction<_loadModelsDefinition>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "setCacheSize", js_callStaticFunction<_setCacheSize>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "getCacheSize", js_callStaticFunction<_getCacheSize>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "getDataFolder", js_callStaticFunction<_getDataFolder>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "getModelFolder", js_callStaticFunction<_getCatalogFolder>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "getTempFolder", js_callStaticFunction<_getTempFolder>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "freeCacheMem", js_callStaticFunction<_freeCacheMem>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "getCacheInfo", js_callStaticFunction<_getCacheInfo>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "getAllDBs", js_callStaticFunction<_getDBList>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "getIndices", js_callStaticFunction<_getIndices>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "verify", js_callStaticFunction<_verify>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "queryOptions", js_callStaticFunction<_queryOptions>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "setSortMaxMem", js_callStaticFunction<_setSortMaxMem>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "close", js_callStaticFunction<_close>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ 0, 0, 0}
	};
 
	static inherited::StaticValue values[] = 
	{
		{ "tables", js_getProperty<_getTables>, nil, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontDelete },
		{ "dataClasses", js_getProperty<_getEntityModels>, nil, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ 0, 0, 0, 0}
	};

	outDefinition.className = "Datastore";
	outDefinition.initialize = js_initialize<Initialize>;
	outDefinition.finalize = js_finalize<Finalize>;
	outDefinition.getProperty = js_getProperty<GetProperty>;
	outDefinition.getPropertyNames = js_getPropertyNames<GetPropertyNames>;
	outDefinition.staticFunctions = functions;
	outDefinition.staticValues = values;
}


VJSObject VJSDatabase::CreateInstance( JS4D::ContextRef inContext, CDB4DBase *inDatabase)
{
	return VJSObject( inContext, inherited::CreateInstance( inContext, inDatabase));
}


//======================================================




void VJSDatabaseTableEnumerator::Initialize( const VJSParms_initialize& inParms, CDB4DBase* inDatabase)
{
	inDatabase->Retain();
}


void VJSDatabaseTableEnumerator::Finalize( const VJSParms_finalize& inParms, CDB4DBase* inDatabase)
{
	inDatabase->Release();
}

void VJSDatabaseTableEnumerator::GetPropertyNames( VJSParms_getPropertyNames& ioParms, CDB4DBase* inDatabase)
{
	sLONG nb = inDatabase->CountTables();
	for (sLONG i = 1; i <= nb; i++)
	{
		VString tablename;
		CDB4DTable* tt = inDatabase->RetainNthTable(i);
		if (tt != nil)
		{
			tt->GetName(tablename);
			if (!tablename.IsEmpty())
			{
				ioParms.AddPropertyName(tablename);
			}
			tt->Release();
		}
	}
}


void VJSDatabaseTableEnumerator::GetProperty( VJSParms_getProperty& ioParms, CDB4DBase* inDatabase)
{
	CDB4DTable* tt = nil;
	CDB4DEntityModel* em = nil;

	VString propname;
	sLONG num;
	if (ioParms.GetPropertyNameAsLong( &num))
	{
		tt = inDatabase->RetainNthTable(num-1);
	}
	else
	{
		ioParms.GetPropertyName(propname);
		tt = inDatabase->FindAndRetainTable(propname);
		if (tt != nil)
		{
			ioParms.ReturnValue(VJSTable::CreateInstance(ioParms.GetContextRef(), tt));
			tt->Release();
		}
	}
}



void VJSDatabaseTableEnumerator::GetDefinition( ClassDefinition& outDefinition)
{
	outDefinition.className = "DatabaseTableEnumerator";
	outDefinition.initialize = js_initialize<Initialize>;
	outDefinition.finalize = js_finalize<Finalize>;
	outDefinition.getProperty = js_getProperty<GetProperty>;
	outDefinition.getPropertyNames = js_getPropertyNames<GetPropertyNames>;
}


VJSObject VJSDatabaseTableEnumerator::CreateInstance( JS4D::ContextRef inContext, CDB4DBase *inDatabase)
{
	return VJSObject( inContext, inherited::CreateInstance( inContext, inDatabase));
}



//======================================================




void VJSDatabaseEMEnumerator::Initialize( const VJSParms_initialize& inParms, CDB4DBase* inDatabase)
{
	inDatabase->Retain();
}


void VJSDatabaseEMEnumerator::Finalize( const VJSParms_finalize& inParms, CDB4DBase* inDatabase)
{
	inDatabase->Release();
}

void VJSDatabaseEMEnumerator::GetPropertyNames( VJSParms_getPropertyNames& ioParms, CDB4DBase* inDatabase)
{
	//vector<CDB4DEntityModel*> entities;
	vector<VRefPtr<CDB4DEntityModel> > entities;
	inDatabase->RetainAllEntityModels(entities, GetDB4DBaseContextFromJSContext( ioParms, inDatabase), false);
	for (vector<VRefPtr<CDB4DEntityModel> >::iterator cur = entities.begin(), end = entities.end(); cur != end; cur++)
	{
		CDB4DEntityModel* em = *cur;
		ioParms.AddPropertyName(em->GetEntityName());
		//dejaName.insert(em->GetEntityName());
	}

#if AllowDefaultEMBasedOnTables

	sLONG nb = inDatabase->CountTables();
	for (sLONG i = 1; i <= nb; i++)
	{
		VString tablename;
		CDB4DTable* tt = inDatabase->RetainNthTable(i);
		if (tt != nil)
		{
			tt->GetName(tablename);
			if (!tablename.IsEmpty())
			{
				XBOX::VString entitytablename = L"$"+tablename;
				ioParms.AddPropertyName(entitytablename);
			}
			tt->Release();
		}
	}
#endif
}


void VJSDatabaseEMEnumerator::GetProperty( VJSParms_getProperty& ioParms, CDB4DBase* inDatabase)
{
	VString propname;
	
	ioParms.GetPropertyName(propname);
	CDB4DEntityModel* em = inDatabase->RetainEntityModel(propname, false);
	if (em != nil)
	{
		ioParms.ReturnValue(VJSEntityModel::CreateInstance(ioParms.GetContextRef(), em));
		em->Release();
	}
}



void VJSDatabaseEMEnumerator::GetDefinition( ClassDefinition& outDefinition)
{
	outDefinition.className = "DataClassEnumerator";
	outDefinition.initialize = js_initialize<Initialize>;
	outDefinition.finalize = js_finalize<Finalize>;
	outDefinition.getProperty = js_getProperty<GetProperty>;
	outDefinition.getPropertyNames = js_getPropertyNames<GetPropertyNames>;
}


VJSObject VJSDatabaseEMEnumerator::CreateInstance( JS4D::ContextRef inContext, CDB4DBase *inDatabase)
{
	return VJSObject( inContext, inherited::CreateInstance( inContext, inDatabase));
}


//======================================================




void VJSEntityAttributeEnumerator::Initialize( const VJSParms_initialize& inParms, CDB4DEntityModel* inModel)
{
	inModel->Retain();
}


void VJSEntityAttributeEnumerator::Finalize( const VJSParms_finalize& inParms, CDB4DEntityModel* inModel)
{
	inModel->Release();
}


void VJSEntityAttributeEnumerator::GetPropertyNames( VJSParms_getPropertyNames& ioParms, CDB4DEntityModel* inModel)
{
	sLONG nb = inModel->CountAttributes();
	for (sLONG i = 1; i <= nb; i++)
	{
		CDB4DEntityAttribute* att = inModel->GetAttribute(i);
		if (att != nil)
		{
			ioParms.AddPropertyName(att->GetAttibuteName());
		}
	}
}


void VJSEntityAttributeEnumerator::GetProperty( VJSParms_getProperty& ioParms, CDB4DEntityModel* inModel)
{
	CDB4DEntityAttribute* att = nil;

	sLONG num;
	VString propname;
	ioParms.GetPropertyName( propname);
	if (ioParms.GetPropertyNameAsLong( &num))
	{
		att = inModel->GetAttribute(num-1);
	}
	else
	{
		att = inModel->GetAttribute(propname);
	}
	if (att != nil)
	{
		ioParms.ReturnValue(VJSEntityAttribute::CreateInstance(ioParms.GetContextRef(), att));
	}
}




void VJSEntityAttributeEnumerator::GetDefinition( ClassDefinition& outDefinition)
{
	outDefinition.className = "EntityAttributeEnumerator";
	outDefinition.initialize = js_initialize<Initialize>;
	outDefinition.finalize = js_finalize<Finalize>;
	outDefinition.getProperty = js_getProperty<GetProperty>;
	outDefinition.getPropertyNames = js_getPropertyNames<GetPropertyNames>;
}



//======================================================

/*
void VJSEntityConstructor::Initialize( const VJSParms_initialize& inParms, CDB4DEntityModel* inModel)
{
	inModel->Retain();
}


void VJSEntityConstructor::Finalize( const VJSParms_finalize& inParms, CDB4DEntityModel* inModel)
{
	inModel->Release();
}




void VJSEntityConstructor::CallAsFunction(VJSParms_callAsFunction& ioParms)
{
	CDB4DEntityModel* inModel = ioParms.GetObject().GetPrivateData<VJSEntityModel>();
	XBOX::VError err = XBOX::VE_OK;
	CDB4DEntityRecord* erec = inModel->NewEntity(GetBaseContext( ioParms), DB4D_Do_Not_Lock);
	if (erec != nil)
	{
		if (ioParms.IsObjectParam(1))
		{
			VJSObject obj(ioParms.GetContextRef());
			ioParms.GetParamObject(1, obj);
			err = VImpCreator<EntityRecord>::GetImpObject(erec)->convertFromJSObj(obj);
		}
		EntitySelectionIterator* iter = new EntitySelectionIterator(erec, GetBaseContext( ioParms));
		ioParms.ReturnValue(VJSEntitySelectionIterator::CreateInstance(ioParms.GetContextRef(), iter));
		erec->Release();
	}
}


void VJSEntityConstructor::CallAsConstructor(VJSParms_callAsConstructor& ioParms)
{
	CDB4DEntityModel* inModel = ioParms.GetObject().GetPrivateData<VJSEntityModel>();
	XBOX::VError err = XBOX::VE_OK;
	CDB4DEntityRecord* erec = inModel->NewEntity(GetBaseContext( ioParms), DB4D_Do_Not_Lock);

	if (erec != nil)
	{
		if (ioParms.IsObjectParam(1))
		{
			VJSObject obj(ioParms.GetContextRef());
			ioParms.GetParamObject(1, obj);
			err = VImpCreator<EntityRecord>::GetImpObject(erec)->convertFromJSObj(obj);
		}

		EntitySelectionIterator* iter = new EntitySelectionIterator(erec, GetBaseContext( ioParms));
		ioParms.ReturnConstructedObject(VJSEntitySelectionIterator::CreateInstance(ioParms.GetContextRef(), iter));
		erec->Release();
	}

}



void VJSEntityConstructor::GetDefinition( ClassDefinition& outDefinition)
{
	outDefinition.className = "EntityConstructor";
	outDefinition.initialize = js_initialize<Initialize>;
	outDefinition.finalize = js_finalize<Finalize>;
	outDefinition.callAsFunction = js_callAsFunction<CallAsFunction>;
	outDefinition.callAsConstructor = js_callAsConstructor<CallAsConstructor>;
}

*/


//======================================================




void VJSEntityModel::Initialize( const VJSParms_initialize& inParms, CDB4DEntityModel* inModel)
{
	inModel->Retain();
}


void VJSEntityModel::Finalize( const VJSParms_finalize& inParms, CDB4DEntityModel* inModel)
{
	inModel->Release();
}


void VJSEntityModel::_isEntityModel(VJSParms_callStaticFunction& ioParms, CDB4DEntityModel* inModel)
{
	ioParms.ReturnBool(true);
}



void VJSEntityModel::GetPropertyNames( VJSParms_getPropertyNames& ioParms, CDB4DEntityModel* inModel)
{
	sLONG nb = inModel->CountAttributes();
	for (sLONG i = 1; i <= nb; i++)
	{
		CDB4DEntityAttribute* att = inModel->GetAttribute(i);
		if (att != nil)
		{
			ioParms.AddPropertyName(att->GetAttibuteName());
		}
	}
}


void VJSEntityModel::GetProperty( VJSParms_getProperty& ioParms, CDB4DEntityModel* inModel)
{
	CDB4DEntityAttribute* att = nil;
	
	sLONG num;
	VString propname;
	ioParms.GetPropertyName( propname);
	if (ioParms.GetPropertyNameAsLong( &num))
	{
		att = inModel->GetAttribute(num);
	}
	else
	{
		att = inModel->GetAttribute(propname);
	}
	if (att != nil)
	{
		ioParms.ReturnValue(VJSEntityAttribute::CreateInstance(ioParms.GetContextRef(), att));
	}
	else
	{
		CDB4DEntityMethod* meth = inModel->GetMethod(propname);
		if (meth != nil && meth->GetMethodKind() == emeth_static)
		{
			VUUID permid, promoteid;
			EntityMethod* xmeth = VImpCreator<EntityMethod>::GetImpObject(meth);
			//xmeth->GetPermission(DB4D_EM_Execute_Perm, permid);
			//xmeth->GetPermission(DB4D_EM_Promote_Perm, promoteid);
			CDB4DBaseContext* context = GetDB4DBaseContextFromJSContext(ioParms, inModel);
			//if (okperm(context, permid))
			{
				//bool waspromoted = promoteperm(basecontext, promoteid);
				VJSObject localObjFunc(ioParms.GetContextRef());
				VJSObject* objfunc = meth->GetFuncObject(context, localObjFunc);
				if (objfunc != nil)
				{
					ioParms.ReturnValue(*objfunc);
				}
			}
		}
	}
}


void VJSEntityModel::CallAsFunction(VJSParms_callAsFunction& ioParms)
{
	if (ioParms.CountParams() > 0)
	{
		VError err = VE_OK;

		CDB4DEntityModel* inModel = ioParms.GetObject().GetPrivateData<VJSEntityModel>();
		if (ioParms.IsObjectParam(1))
		{
			VJSObject objParam(ioParms.GetContextRef());
			ioParms.GetParamObject(1, objParam);

			bool first = true;

			vector<VString> props;
			objParam.GetPropertyNames(props);

			CDB4DQuery* query = inModel->NewQuery();
			CDB4DBaseContext* context = GetDB4DBaseContextFromJSContext(ioParms, inModel);

			VString queryString;

			for (vector<VString>::const_iterator cur = props.begin(), end = props.end(); cur != end && err == VE_OK; cur++)
			{
				VJSValue jsval(objParam.GetContextRef());
				jsval = objParam.GetProperty(*cur);
				VValueSingle* cv = jsval.CreateVValue();
				if (cv != nil)
				{
					if (!first)
						query->AddLogicalOperator(DB4D_And);
					query->AddEmCriteria(*cur, DB4D_Like, *cv, false);
					delete cv;
					first = false;
				}

			}

			CDB4DSelection* sel = inModel->ExecuteQuery(query, context, nil, nil, DB4D_Do_Not_Lock, 0, nil, &err);
			if (sel != nil)
			{
				EntitySelectionIterator* itersel = new EntitySelectionIterator(sel, false, true, context, inModel);
				itersel->First(context);
				if (itersel->GetCurRec(context) != nil)
				{
					ioParms.ReturnValue(VJSEntitySelectionIterator::CreateInstance(ioParms.GetContextRef(), itersel));
				}
				else
				{
					delete itersel;
					ioParms.ReturnNullValue();
				}
				sel->Release();
			}
			query->Release();
		}
		else
		{
			VectorOfVValue key(true);
			for (VSize i = 1, nbparm = ioParms.CountParams(); i <= nbparm; i++)
			{
				VValueSingle* cv = ioParms.CreateParamVValue(i);
				if (cv != nil)
					key.push_back(cv);
			}
			
			CDB4DEntityRecord* erec = nil;

			erec = inModel->FindEntityWithPrimKey(key, GetDB4DBaseContextFromJSContext(ioParms, inModel), err, DB4D_Do_Not_Lock);

			if (erec != nil)
			{
				EntitySelectionIterator* iter = new EntitySelectionIterator( erec, GetDB4DBaseContextFromJSContext(ioParms, inModel));
				ioParms.ReturnValue(VJSEntitySelectionIterator::CreateInstance(ioParms.GetContextRef(), iter));
				erec->Release();
			}
		}
	}

#if 0
	CDB4DEntityModel* inModel = ioParms.GetObject().GetPrivateData<VJSEntityModel>();
	XBOX::VError err = XBOX::VE_OK;
	CDB4DEntityRecord* erec = inModel->NewEntity(GetBaseContext( ioParms), /*DB4D_Keep_Lock_With_Record*/DB4D_Do_Not_Lock);
	if (erec != nil)
	{
		if (ioParms.IsObjectParam(1))
		{
			VJSObject obj(ioParms.GetContextRef());
			ioParms.GetParamObject(1, obj);
			err = VImpCreator<EntityRecord>::GetImpObject(erec)->convertFromJSObj(obj);
		}
		EntitySelectionIterator* iter = new EntitySelectionIterator(erec, GetBaseContext( ioParms));
		ioParms.ReturnValue(VJSEntitySelectionIterator::CreateInstance(ioParms.GetContextRef(), iter));
		erec->Release();
	}
#endif
}


void VJSEntityModel::CallAsConstructor(VJSParms_callAsConstructor& ioParms)
{
	CDB4DEntityModel* inModel = ioParms.GetObject().GetPrivateData<VJSEntityModel>();
	XBOX::VError err = XBOX::VE_OK;
	CDB4DEntityRecord* erec = inModel->NewEntity( GetDB4DBaseContextFromJSContext(ioParms, inModel), DB4D_Do_Not_Lock);

	if (erec != nil)
	{
		if (ioParms.IsObjectParam(1))
		{
			VJSObject obj(ioParms.GetContextRef());
			ioParms.GetParamObject(1, obj);
			err = VImpCreator<EntityRecord>::GetImpObject(erec)->convertFromJSObj(obj);
		}

		EntitySelectionIterator* iter = new EntitySelectionIterator( erec, GetDB4DBaseContextFromJSContext(ioParms, inModel));
		ioParms.ReturnConstructedObject(VJSEntitySelectionIterator::CreateInstance(ioParms.GetContextRef(), iter));
		erec->Release();
	}

}


void VJSEntityModel::_getName(VJSParms_callStaticFunction& ioParms, CDB4DEntityModel* inModel)
{
	ioParms.ReturnString(inModel->GetEntityName());
}


void VJSEntityModel::_getScope(VJSParms_callStaticFunction& ioParms, CDB4DEntityModel* inModel)
{
	ioParms.ReturnString(EScopeKinds[inModel->GetScope()]);
}


void VJSEntityModel::_getDataStore(VJSParms_callStaticFunction& ioParms, CDB4DEntityModel* inModel)
{
	CDB4DBase* owner = inModel->RetainDataBase();
	if (owner != nil)
		ioParms.ReturnValue(VJSDatabase::CreateInstance(ioParms.GetContextRef(), owner));
	QuickReleaseRefCountable(owner);
}


void VJSEntityModel::_AllEntities(VJSParms_callStaticFunction& ioParms, CDB4DEntityModel* inModel)
{
	VError err = VE_OK;
	CDB4DSelection* sel = inModel->SelectAllEntities(GetDB4DBaseContextFromJSContext(ioParms, inModel), &err);
	if (sel != nil)
	{
		ioParms.ReturnValue(VJSEntitySelection::CreateInstance(ioParms.GetContextRef(), sel));
		sel->Release();
	}
}


void QueryJS(VJSParms_callStaticFunction& ioParms, CDB4DEntityModel* inModel, CDB4DSelection* filter)
{
	VString querystring;
	VError err = VE_OK;
	if (ioParms.GetStringParam(1, querystring))
	{
		bool withlock = false;
		//ioParms.GetBoolParam(2, &withlock);
		CDB4DQuery* query = inModel->NewQuery();
		if (query != nil)
		{
			VString orderby;
			CDB4DBaseContext* context = GetDB4DBaseContextFromJSContext(ioParms, inModel);
			QueryParamElementVector qparams;
			err = getQParams(ioParms, 2, qparams, query);
			err = query->BuildFromString(querystring, orderby, context, inModel, &qparams);
			if (err == VE_OK)
			{
				err = FillQueryWithParams(query, ioParms, 2);

				if (err == VE_OK)
				{	 
					CDB4DSelection* sel = inModel->ExecuteQuery(query, context, filter, nil, withlock ? DB4D_Keep_Lock_With_Transaction : DB4D_Do_Not_Lock, 0, nil, &err);
					if (sel != nil && !orderby.IsEmpty())
					{
						bool ok = sel->SortSelection(orderby, nil, context);
						if (!ok)
						{
							ReleaseRefCountable(&sel);
						}
					}
					if (sel != nil)
					{
						sel->SetQueryPlan(context->GetLastQueryPlan());
						sel->SetQueryPath(context->GetLastQueryPath());
						ioParms.ReturnValue(VJSEntitySelection::CreateInstance(ioParms.GetContextRef(), sel));
						sel->Release();
					}
				}
			}
			query->Release();
		}
	}
}

void VJSEntityModel::_Query(VJSParms_callStaticFunction& ioParms, CDB4DEntityModel* inModel)
{
	QueryJS(ioParms, inModel, nil);
}


void FindJS(VJSParms_callStaticFunction& ioParms, CDB4DEntityModel* inModel, CDB4DSelection* filter)
{
	VString querystring;
	VError err = VE_OK;
	bool okresult = false;
	if (ioParms.GetStringParam(1, querystring))
	{
		bool withlock = false;
		//ioParms.GetBoolParam(2, &withlock);
		CDB4DQuery* query = inModel->NewQuery();
		if (query != nil)
		{
			CDB4DBaseContext* context = GetDB4DBaseContextFromJSContext(ioParms, inModel);
			VString orderby;
			QueryParamElementVector qparams;
			err = getQParams(ioParms, 2, qparams, query);
			err = query->BuildFromString(querystring, orderby, context, inModel, &qparams);
			if (err == VE_OK)
				err = FillQueryWithParams(query, ioParms, 2);
			if (err == VE_OK)
			{
				CDB4DSelection* sel = inModel->ExecuteQuery(query, context, filter, nil, withlock ? DB4D_Keep_Lock_With_Transaction : DB4D_Do_Not_Lock, 0, nil, &err);
				if (sel != nil && !orderby.IsEmpty())
				{
					bool ok = sel->SortSelection(orderby, nil, context);
					if (!ok)
					{
						ReleaseRefCountable(&sel);
					}
				}
				if (sel != nil)
				{
					EntitySelectionIterator* itersel = new EntitySelectionIterator(sel, false, true, context, inModel);
					itersel->First(context);
					if (itersel->GetCurRec(context) != nil)
					{
						ioParms.ReturnValue(VJSEntitySelectionIterator::CreateInstance(ioParms.GetContextRef(), itersel));
						okresult = true;
					}
					else
						delete itersel;
					sel->Release();
				}
			}
			query->Release();
		}
	}
	if (!okresult)
		ioParms.ReturnNullValue();
}

void VJSEntityModel::_Find(VJSParms_callStaticFunction& ioParms, CDB4DEntityModel* inModel)
{
	FindJS(ioParms, inModel, nil);
}



void VJSEntityModel::_NewEntity(VJSParms_callStaticFunction& ioParms, CDB4DEntityModel* inModel)
{
	CDB4DBaseContext* context = GetDB4DBaseContextFromJSContext(ioParms, inModel);
	CDB4DEntityRecord* rec = inModel->NewEntity(context, /*DB4D_Keep_Lock_With_Record*/DB4D_Do_Not_Lock);
	if (rec != nil)
	{
		EntitySelectionIterator* iter = new EntitySelectionIterator(rec, context);
		ioParms.ReturnValue(VJSEntitySelectionIterator::CreateInstance(ioParms.GetContextRef(), iter));
		rec->Release();
	}
}


void VJSEntityModel::_NewSelection(VJSParms_callStaticFunction& ioParms, CDB4DEntityModel* inModel)
{
	bool keepsorted = ioParms.GetBoolParam( 1, L"KeepSorted", L"AnyOrder");
	CDB4DSelection* sel = inModel->NewSelection(keepsorted ? DB4D_Sel_SmallSel : DB4D_Sel_Bittab);
	if (sel != nil)
	{
		ioParms.ReturnValue(VJSEntitySelection::CreateInstance(ioParms.GetContextRef(), sel));
		sel->Release();
	}
}


void VJSEntityModel::_First(VJSParms_callStaticFunction& ioParms, CDB4DEntityModel* inModel)
{
	VError err = VE_OK;
	CDB4DSelection* sel = inModel->SelectAllEntities(GetDB4DBaseContextFromJSContext(ioParms, inModel), &err);
	if (sel != nil)
	{
		VJSEntitySelection::_First(ioParms, sel);
		sel->Release();
	}
}

void VJSEntityModel::_Count(VJSParms_callStaticFunction& ioParms, CDB4DEntityModel* inModel)
{
	if (ioParms.CountParams() > 0)
	{
		VError err = VE_OK;
		CDB4DSelection* sel = inModel->SelectAllEntities(GetDB4DBaseContextFromJSContext(ioParms, inModel), &err);
		if (sel != nil)
		{
			VJSEntitySelection::_Count(ioParms, sel);
			sel->Release();
		}
	}
	else
	ioParms.ReturnNumber(inModel->CountEntities(GetDB4DBaseContextFromJSContext(ioParms, inModel)));
}

void VJSEntityModel::_OrderBy(VJSParms_callStaticFunction& ioParms, CDB4DEntityModel* inModel)
{
	VError err = VE_OK;
	CDB4DSelection* sel = inModel->SelectAllEntities(GetDB4DBaseContextFromJSContext(ioParms, inModel), &err);
	if (sel != nil)
	{
		VJSEntitySelection::_OrderBy(ioParms, sel);
		sel->Release();
	}
}

void VJSEntityModel::_Each(VJSParms_callStaticFunction& ioParms, CDB4DEntityModel* inModel)
{
	VError err = VE_OK;
	CDB4DSelection* sel = inModel->SelectAllEntities(GetDB4DBaseContextFromJSContext(ioParms, inModel), &err);
	if (sel != nil)
	{
		VJSEntitySelection::_Each(ioParms, sel);
		sel->Release();
	}
}

void VJSEntityModel::_dropEntities(VJSParms_callStaticFunction& ioParms, CDB4DEntityModel* inModel)
{
	VError err = VE_OK;
	CDB4DSelection* sel = inModel->SelectAllEntities(GetDB4DBaseContextFromJSContext(ioParms, inModel), &err);
	if (sel != nil)
	{
		VJSEntitySelection::_dropEntities(ioParms, sel);
		sel->Release();
	}
}

void VJSEntityModel::_distinctValues(VJSParms_callStaticFunction& ioParms, CDB4DEntityModel* inModel)
{
	VError err = VE_OK;
	CDB4DSelection* sel = inModel->SelectAllEntities(GetDB4DBaseContextFromJSContext(ioParms, inModel), &err);
	if (sel != nil)
	{
		VJSEntitySelection::_distinctValues(ioParms, sel);
		sel->Release();
	}
}


void VJSEntityModel::_toArray(VJSParms_callStaticFunction& ioParms, CDB4DEntityModel* inModel)
{
	VError err = VE_OK;
	CDB4DSelection* sel = inModel->SelectAllEntities(GetDB4DBaseContextFromJSContext(ioParms, inModel), &err);
	if (sel != nil)
	{
		VJSEntitySelection::_toArray(ioParms, sel);
		sel->Release();
	}
}


void VJSEntityModel::_sum(VJSParms_callStaticFunction& ioParms, CDB4DEntityModel* inModel)
{
	VError err = VE_OK;
	CDB4DSelection* sel = inModel->SelectAllEntities(GetDB4DBaseContextFromJSContext(ioParms, inModel), &err);
	if (sel != nil)
	{
		VJSEntitySelection::_sum(ioParms, sel);
		sel->Release();
	}
}


void VJSEntityModel::_min(VJSParms_callStaticFunction& ioParms, CDB4DEntityModel* inModel)
{
	VError err = VE_OK;
	CDB4DSelection* sel = inModel->SelectAllEntities(GetDB4DBaseContextFromJSContext(ioParms, inModel), &err);
	if (sel != nil)
	{
		VJSEntitySelection::_min(ioParms, sel);
		sel->Release();
	}
}


void VJSEntityModel::_max(VJSParms_callStaticFunction& ioParms, CDB4DEntityModel* inModel)
{
	VError err = VE_OK;
	CDB4DSelection* sel = inModel->SelectAllEntities(GetDB4DBaseContextFromJSContext(ioParms, inModel), &err);
	if (sel != nil)
	{
		VJSEntitySelection::_max(ioParms, sel);
		sel->Release();
	}
}


void VJSEntityModel::_average(VJSParms_callStaticFunction& ioParms, CDB4DEntityModel* inModel)
{
	VError err = VE_OK;
	CDB4DSelection* sel = inModel->SelectAllEntities(GetDB4DBaseContextFromJSContext(ioParms, inModel), &err);
	if (sel != nil)
	{
		VJSEntitySelection::_average(ioParms, sel);
		sel->Release();
	}
}


void VJSEntityModel::_compute(VJSParms_callStaticFunction& ioParms, CDB4DEntityModel* inModel)
{
	VError err = VE_OK;
	CDB4DSelection* sel = inModel->SelectAllEntities(GetDB4DBaseContextFromJSContext(ioParms, inModel), &err);
	if (sel != nil)
	{
		VJSEntitySelection::_compute(ioParms, sel);
		sel->Release();
	}
}


void VJSEntityModel::_fromArray(VJSParms_callStaticFunction& ioParms, CDB4DEntityModel* inModel)
{
	VError err = VE_OK;
	bool okresult = false;
	CDB4DBaseContext* context = GetDB4DBaseContextFromJSContext(ioParms, inModel);
	BaseTaskInfo* basecontext = ConvertContext(context);
	if (ioParms.IsArrayParam(1))
	{
		CDB4DSelection* xsel = inModel->NewSelection(DB4D_Sel_Bittab);
		VJSArray arr(ioParms.GetContextRef(), nil,  true);
		ioParms.GetParamArray(1, arr);
		sLONG nbelem = arr.GetLength();
		VJSValue jsval(ioParms.GetContextRef());
		Bittab* bsel = ((BitSel*)(VImpCreator<VDB4DSelection>::GetImpObject(xsel)->GetSel()))->GetBittab();
		EntityModel* model = VImpCreator<EntityModel>::GetImpObject(inModel);

		for (sLONG i = 0; i < nbelem && err == VE_OK; i++)
		{
			jsval = arr.GetValueAt(i);
			if (jsval.IsObject())
			{
				VJSObject objrec(ioParms.GetContextRef());
				jsval.GetObject(objrec);
				sLONG stamp = 0;
				
				EntityRecord* rec = nil;
				jsval = objrec.GetProperty("__KEY");
				if (jsval.IsObject())
				{
					VJSObject objkey(ioParms.GetContextRef());
					jsval.GetObject(objkey);

					jsval = objkey.GetProperty("__STAMP");
					if (jsval.IsNumber())
						jsval.GetLong(&stamp);

					rec = model->findEntityWithPrimKey(objkey, basecontext,err, DB4D_Do_Not_Lock);
					if (err == VE_OK)
					{
						if (rec == nil)
						{
							rec = model->NewEntity(basecontext, DB4D_Do_Not_Lock);
							if (rec != nil)
							{
								rec->setPrimKey(objkey);
							}
						}
					}
				}
				else
				{
					rec = model->NewEntity(basecontext, DB4D_Do_Not_Lock);
				}

				if (rec != nil)
				{
					err = rec->convertFromJSObj(objrec);
				}
				if (err == VE_OK)
				{
					StErrorContextInstaller errs(false);
					VError err2 = rec->Save(stamp);
					if (err2 == VE_OK)
					{
						sLONG numrec = rec->GetNum();
						if (numrec >= 0)
						bsel->Set(numrec);
					}
				}

				QuickReleaseRefCountable(rec);
			}

		}

		okresult = true;
		ioParms.ReturnValue(VJSEntitySelection::CreateInstance(ioParms.GetContextRef(), xsel));
		xsel->Release();

	}
	if (!okresult)
		ioParms.ReturnNullValue();
}


void VJSEntityModel::_setAutoSequenceNumber(VJSParms_callStaticFunction& ioParms, CDB4DEntityModel* inModel)
{
	VError err = VE_OK;
	bool okresult = false;
	CDB4DBaseContext* context = GetDB4DBaseContextFromJSContext(ioParms, inModel);

	CDB4DTable* tt = inModel->RetainTable();
	if (tt != nil)
	{
		CDB4DAutoSeqNumber* seq = tt->RetainAutoSeqNumber(context);
		if (seq != nil)
		{
			Real newnum;
			if (ioParms.IsNumberParam(1))
			{
				ioParms.GetRealParam(1, &newnum);
				seq->SetCurrentValue((sLONG8)newnum);
			}
		}
		QuickReleaseRefCountable(seq);
	}
	QuickReleaseRefCountable(tt);

}


void VJSEntityModel::_getAttributes( XBOX::VJSParms_getProperty& ioParms, CDB4DEntityModel* inModel)
{
	ioParms.ReturnValue(VJSEntityAttributeEnumerator::CreateInstance(ioParms.GetContextRef(), inModel));
}


void VJSEntityModel::_getLength( XBOX::VJSParms_getProperty& ioParms, CDB4DEntityModel* inModel)
{
	ioParms.ReturnNumber(inModel->CountEntities(GetDB4DBaseContextFromJSContext(ioParms, inModel)));
}


void VJSEntityModel::GetDefinition( ClassDefinition& outDefinition)
{
	static inherited::StaticFunction functions[] = 
	{
		{ "getDataStore", js_callStaticFunction<_getDataStore>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "getName", js_callStaticFunction<_getName>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "getScope", js_callStaticFunction<_getScope>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "toString", js_callStaticFunction<_getName>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		//{ "isEntityModel", js_callStaticFunction<_isEntityModel>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		
		{ "all", js_callStaticFunction<_AllEntities>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "allEntities", js_callStaticFunction<_AllEntities>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "query", js_callStaticFunction<_Query>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "filter", js_callStaticFunction<_Query>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "find", js_callStaticFunction<_Find>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "createEntity", js_callStaticFunction<_NewEntity>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "createEntityCollection", js_callStaticFunction<_NewSelection>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },

		{ "first", js_callStaticFunction<_First>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "count", js_callStaticFunction<_Count>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "orderBy", js_callStaticFunction<_OrderBy>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "sort", js_callStaticFunction<_OrderBy>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "forEach", js_callStaticFunction<_Each>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "drop", js_callStaticFunction<_dropEntities>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "remove", js_callStaticFunction<_dropEntities>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "distinctValues", js_callStaticFunction<_distinctValues>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "toArray", js_callStaticFunction<_toArray>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "fromArray", js_callStaticFunction<_fromArray>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "setAutoSequenceNumber", js_callStaticFunction<_setAutoSequenceNumber>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },

		{ "sum", js_callStaticFunction<_sum>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "min", js_callStaticFunction<_min>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "max", js_callStaticFunction<_max>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "average", js_callStaticFunction<_average>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "compute", js_callStaticFunction<_compute>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ 0, 0, 0}
	};

	static inherited::StaticValue values[] = 
	{
		{ "attributes", js_getProperty<_getAttributes>, nil, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "length", js_getProperty<_getLength>, nil, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ 0, 0, 0, 0}
	};

	outDefinition.className = "dataClass";
	outDefinition.initialize = js_initialize<Initialize>;
	outDefinition.finalize = js_finalize<Finalize>;
	outDefinition.getProperty = js_getProperty<GetProperty>;
	outDefinition.getPropertyNames = js_getPropertyNames<GetPropertyNames>;
	outDefinition.callAsFunction = js_callAsFunction<CallAsFunction>;
	outDefinition.callAsConstructor = js_callAsConstructor<CallAsConstructor>;
	outDefinition.staticValues = values;
	outDefinition.staticFunctions = functions;
}





//======================================================



void VJSEntityAttribute::Initialize( const VJSParms_initialize& inParms, CDB4DEntityAttribute* inAttribute)
{
	inAttribute->Retain();
}


void VJSEntityAttribute::Finalize( const VJSParms_finalize& inParms, CDB4DEntityAttribute* inAttribute)
{
	inAttribute->Release();
}


void VJSEntityAttribute::_getName(VJSParms_callStaticFunction& ioParms, CDB4DEntityAttribute* inAttribute)
{
	ioParms.ReturnString(inAttribute->GetAttibuteName());
}


void VJSEntityAttribute::_getPropName( XBOX::VJSParms_getProperty& ioParms, CDB4DEntityAttribute* inAttribute)
{
	ioParms.ReturnString(inAttribute->GetAttibuteName());
}


void VJSEntityAttribute::_getKind( XBOX::VJSParms_getProperty& ioParms, CDB4DEntityAttribute* inAttribute)
{
	ioParms.ReturnString(EattTypes[inAttribute->GetAttributeKind()]);
}


void VJSEntityAttribute::_getScope( XBOX::VJSParms_getProperty& ioParms, CDB4DEntityAttribute* inAttribute)
{
	ioParms.ReturnString(EScopeKinds[inAttribute->GetScope()]);
}


void VJSEntityAttribute::_getType( XBOX::VJSParms_getProperty& ioParms, CDB4DEntityAttribute* inAttribute)
{
	VString s;
	EntityAttributeKind kind = inAttribute->GetAttributeKind();
	if (kind == eattr_composition || kind == eattr_relation_1toN)
	{
		CDB4DEntityModel* rel = inAttribute->GetRelatedEntityModel();
		if (rel != nil)
		{
			EntityModel* xrel = VImpCreator<EntityModel>::GetImpObject(rel);
			s = xrel->GetCollectionName();
		}
	}
	else if (kind == eattr_relation_Nto1)
	{
		CDB4DEntityModel* rel = inAttribute->GetRelatedEntityModel();
		if (rel != nil)
		{
			EntityModel* xrel = VImpCreator<EntityModel>::GetImpObject(rel);
			s = xrel->GetName();
		}
		}
	else
	{
		EntityAttribute* xatt = VImpCreator<EntityAttribute>::GetImpObject(inAttribute);
		s = EValPredefinedTypes[xatt->GetDataKind()];
	}
	ioParms.ReturnString(s);
}


void VJSEntityAttribute::_getIndexType( XBOX::VJSParms_getProperty& ioParms, CDB4DEntityAttribute* inAttribute)
{
	VString s;
	EntityAttribute* xatt = VImpCreator<EntityAttribute>::GetImpObject(inAttribute);
	s = xatt->GetIndexKind();
	ioParms.ReturnString(s);
}


void VJSEntityAttribute::_getIndexed( XBOX::VJSParms_getProperty& ioParms, CDB4DEntityAttribute* inAttribute)
{
	bool res = false;
	EntityAttribute* xatt = VImpCreator<EntityAttribute>::GetImpObject(inAttribute);
	Field* ff = xatt->RetainDirectField();
	if (ff != nil)
		res = ff->IsIndexed();
	QuickReleaseRefCountable(ff);
	ioParms.ReturnBool(res);
}


void VJSEntityAttribute::_getFullTextIndexed( XBOX::VJSParms_getProperty& ioParms, CDB4DEntityAttribute* inAttribute)
{
	bool res = false;
	EntityAttribute* xatt = VImpCreator<EntityAttribute>::GetImpObject(inAttribute);
	Field* ff = xatt->RetainDirectField();
	if (ff != nil)
		res = ff->IsFullTextIndexed();
	QuickReleaseRefCountable(ff);
	ioParms.ReturnBool(res);
}



void VJSEntityAttribute::_getDataClass( XBOX::VJSParms_getProperty& ioParms, CDB4DEntityAttribute* inAttribute)
{
	CDB4DEntityModel* res = inAttribute->GetModel();
	if (res == nil)
		ioParms.ReturnNullValue();
	else
		ioParms.ReturnValue(VJSEntityModel::CreateInstance(ioParms.GetContextRef(), res));
}


void VJSEntityAttribute::_getRelatedDataClass( XBOX::VJSParms_getProperty& ioParms, CDB4DEntityAttribute* inAttribute)
{
	CDB4DEntityModel* res = inAttribute->GetRelatedEntityModel();
	if (res == nil)
		ioParms.ReturnNullValue();
	else
		ioParms.ReturnValue(VJSEntityModel::CreateInstance(ioParms.GetContextRef(), res));
}


void VJSEntityAttribute::GetDefinition( ClassDefinition& outDefinition)
{
	static inherited::StaticFunction functions[] = 
	{
		{ "getName", js_callStaticFunction<_getName>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "toString", js_callStaticFunction<_getName>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ 0, 0, 0}
	};

	static inherited::StaticValue values[] = 
	{
		{ "name", js_getProperty<_getPropName>, nil, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontDelete },
		{ "kind", js_getProperty<_getKind>, nil, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontDelete },
		{ "type", js_getProperty<_getType>, nil, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontDelete },
		{ "scope", js_getProperty<_getScope>, nil, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontDelete },
		{ "indexType", js_getProperty<_getIndexType>, nil, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontDelete },
		{ "indexed", js_getProperty<_getIndexed>, nil, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontDelete },
		{ "fullTextIndexed", js_getProperty<_getFullTextIndexed>, nil, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontDelete },
		{ "dataClass", js_getProperty<_getDataClass>, nil, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontDelete },
		{ "relatedDataClass", js_getProperty<_getRelatedDataClass>, nil, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontDelete },
		{ 0, 0, 0, 0}
	};

	outDefinition.className = "EntityAttribute";
	outDefinition.initialize = js_initialize<Initialize>;
	outDefinition.finalize = js_finalize<Finalize>;
	outDefinition.staticFunctions = functions;
	outDefinition.staticValues = values;
}



//======================================================





void VJSEntitySelectionIterator::Initialize( const VJSParms_initialize& inParms, EntitySelectionIterator* inSelIter)
{
	// rien a faire pour l'instant
}


void VJSEntitySelectionIterator::Finalize( const VJSParms_finalize& inParms, EntitySelectionIterator* inSelIter)
{
	delete inSelIter;
}



void VJSEntitySelectionIterator::GetPropertyNames( VJSParms_getPropertyNames& ioParms, EntitySelectionIterator* inSelIter)
{
	CDB4DEntityModel* em = inSelIter->GetModel();
	sLONG nb = em->CountAttributes();
	for (sLONG i = 1; i <= nb; i++)
	{
		CDB4DEntityAttribute* att = em->GetAttribute(i);
		if (att != nil)
		{
			ioParms.AddPropertyName(att->GetAttibuteName());
		}
	}
}

void VJSEntitySelectionIterator::GetProperty( VJSParms_getProperty& ioParms, EntitySelectionIterator* inSelIter)
{
	CDB4DEntityModel* em = inSelIter->GetModel();
	CDB4DBaseContext* context = GetDB4DBaseContextFromJSContext(ioParms, em);
	VString propname;
	ioParms.GetPropertyName(propname);
	CDB4DEntityAttribute* att = em->GetAttribute(propname);
	if (att != nil)
	{
		CDB4DEntityRecord* rec = inSelIter->GetCurRec(context);
		if (rec != nil)
		{
			VError err = VE_OK;
			CDB4DEntityAttributeValue* emval = rec->GetAttributeValue(att, err);
			if (emval != nil)
			{
				switch (emval->GetAttributeKind()) {
					case eav_vvalue:
						{
							VValueSingle* cv = emval->GetVValue();
							if (cv != nil)
							{
#if 0
								if (cv->GetValueKind() == VK_IMAGE)
								{
#if !VERSION_LINUX   // Postponed Linux Implementation !

									/*
									void* extradata = emval->GetExtraData();
									if (extradata == nil)
									{
										db4dJSPictContainer* xpic = new db4dJSPictContainer(cv, ioParms.GetContextRef(), rec, att);
										ioParms.ReturnValue(VJSImage::CreateInstance(ioParms.GetContextRef(), xpic));
										emval->SetExtraData(xpic);
										xpic->Release();
									}
									else
									{
										ioParms.ReturnValue(VJSImage::CreateInstance(ioParms.GetContextRef(), (db4dJSPictContainer*)extradata));
									}
									*/
#endif
								}
								else
#endif
									ioParms.ReturnVValue(*cv);
							}
						}
						break;
						
					case eav_subentity:
						{
							CDB4DEntityRecord* subrec = emval->GetRelatedEntity();
							if (subrec != nil)
							{
								EntitySelectionIterator* subiter = new EntitySelectionIterator(subrec, context);
								ioParms.ReturnValue(VJSEntitySelectionIterator::CreateInstance(ioParms.GetContextRef(), subiter));
							}
						}
						break;
						
					case eav_selOfSubentity:
						{
							CDB4DSelection* sel = emval->GetRelatedSelection();
							if (sel != nil)
							{
								ioParms.ReturnValue(VJSEntitySelection::CreateInstance(ioParms.GetContextRef(), sel));								
							}
						}
						break;

					case eav_composition:
						{
							/*
							VString jsonvalue;
							emval->GetJsonString(jsonvalue);
							VJSJSON json(ioParms.GetContextRef());
							VJSValue result(json.Parse(jsonvalue));
							ioParms.ReturnValue(result);
							*/
							VJSObject result(ioParms.GetContextRef());
							err = emval->GetJSObject(result);
							if (err != VE_OK)
								result.SetUndefined();
							ioParms.ReturnValue(result);
						}
						
					default:
						break;
				}
			}
		}
	}
	else
	{
		CDB4DEntityMethod* meth = em->GetMethod(propname);
		if (meth != nil && meth->GetMethodKind() == emeth_rec)
		{
			VJSObject localObjFunc(ioParms.GetContextRef());
			VJSObject* objfunc = meth->GetFuncObject(context, localObjFunc);
			if (objfunc != nil)
			{
				ioParms.ReturnValue(*objfunc);
			}
		}
	}
}



bool VJSEntitySelectionIterator::SetProperty( VJSParms_setProperty& ioParms, EntitySelectionIterator* inSelIter)
{
	CDB4DEntityModel* em = inSelIter->GetModel();
	CDB4DBaseContext* context = GetDB4DBaseContextFromJSContext(ioParms, em);
	VString propname;
	ioParms.GetPropertyName(propname);
	CDB4DEntityAttribute* att = em->GetAttribute(propname);
	if (att != nil)
	{
		CDB4DEntityRecord* rec = inSelIter->GetCurRec(context);
		if (rec != nil)
		{
			if (att->GetAttributeKind() == eattr_composition)
			{
				const VJSValue& val = ioParms.GetPropertyValue();
				VJSObject jsobj(ioParms.GetContextRef());
				if (val.IsObject())
					val.GetObject(jsobj);
				else
					jsobj.MakeEmpty();
					
				/*
				VJSJSON json(ioParms.GetContextRef());
				VString jsonvalue;
				json.Stringify(val, jsonvalue);
				*/
				rec->SetAttributeValue(att, jsobj);
			}
			else
			{
				EntitySelectionIterator* xrelatedRec = ioParms.GetPropertyObjectPrivateData<VJSEntitySelectionIterator>();
				if (xrelatedRec == nil)
				{
					const VJSValue& val = ioParms.GetPropertyValue();
					if (val.IsInstanceOf(L"Array"))
					{
						VJSObject valobj(ioParms.GetContextRef());
						val.GetObject(valobj);
						VJSArray varr(valobj);
						VSize nbelem = varr.GetLength();
						VectorOfVValue vals(true);
						for (VSize i = 0; i < nbelem; i++)
						{
							VJSValue elem(varr.GetValueAt(i));
							VValueSingle* cv = elem.CreateVValue();
							if (cv != nil)
							{
								vals.push_back(cv);
							}
						}
						rec->SetAttributeValue(att, &vals);
					}
					else
					{
						bool isMeta = false;
						if (att->GetDataKind() == VK_IMAGE)
						{
							VJSValue propval(ioParms.GetPropertyValue());
							if (propval.IsObject())
							{
								VJSObject propobj(propval.GetObject());
								if (propobj.HasProperty("IPTC") || propobj.HasProperty("EXIF") || propobj.HasProperty("GPS") || propobj.HasProperty("TIFF"))
								{
									isMeta = true;
									VError err = VE_OK;
									CDB4DEntityAttributeValue* emval = rec->GetAttributeValue(att, err);
									if (emval != nil)
									{
										VValueSingle* cvpict = emval->GetVValue();
										if (cvpict != nil)
										{
#if !VERSION_LINUX   // Postponed Linux Implementation !
											VPicture* pict = dynamic_cast<VPicture*>(cvpict);
											if (pict != nil)
											{
												VString jsonString;
												VJSJSON json(ioParms.GetContextRef());
												json.Stringify(propobj, jsonString);
												VValueBag* bag = new VValueBag();
												bag->FromJSONString(jsonString);
												const VPictureData* picdata = pict->RetainNthPictData(1);
												if (picdata != nil)
												{
													VString codecIdentifier = picdata->GetEncoderID();
													VString pictpath;
													bool isrelative;
													pict->GetOutsidePath(pictpath, &isrelative);

													VPicture newpict;
													VValueBag* pictureSettings = new VValueBag();
													ImageEncoding::stWriter settingsWriter(pictureSettings);
													settingsWriter.SetCodecIdentifier( codecIdentifier);
													VValueBag *bagRetained = settingsWriter.CreateOrRetainMetadatas( bag);
													if (bagRetained) 
														bagRetained->Release(); 

													VPictureCodecFactoryRef fact;
													err = fact->Encode( *pict, codecIdentifier, newpict, pictureSettings);

													const VPictureData* newpictdata = newpict.RetainNthPictData(1);
													if (newpictdata != nil)
													{
														pict->FromVPictureData(newpictdata);
														newpictdata->Release();
														rec->TouchAttributeValue(att);
													}

													QuickReleaseRefCountable(pictureSettings);

													pict->GetOutsidePath(pictpath, &isrelative);

													picdata->Release();
												}

												QuickReleaseRefCountable(bag);
											}
#endif
										}
									}
								}
							}
						}

						if (!isMeta)
						{
							VValueSingle* cv = ioParms.CreatePropertyVValue();
							//if (cv != nil)
							{
								rec->SetAttributeValue(att, cv);
								if (cv != nil)
									delete cv;
							}
						}
					}
				}
				else
				{
					rec->SetAttributeValue(att, xrelatedRec->GetCurRec(context));
				}
			}
		}
	}
	return true;
}


void VJSEntitySelectionIterator::_Save(VJSParms_callStaticFunction& ioParms, EntitySelectionIterator* inSelIter)
{
	VError err = VE_OK;
	bool saved = false;
	CDB4DEntityRecord* inRecord = inSelIter->GetCurRec(GetDB4DBaseContextFromJSContext(ioParms, inSelIter->GetModel()));
	if (inRecord != nil)
	{
		sLONG stamp = inRecord->GetModificationStamp();
		if (stamp == 0)
			stamp = 1;
		err = inRecord->Save(stamp);
		saved = err == VE_OK;
	}
	ioParms.ReturnBool(saved);
}


void VJSEntitySelectionIterator::_validate(VJSParms_callStaticFunction& ioParms, EntitySelectionIterator* inSelIter)
{
	VError err = VE_OK;
	bool validated = false;
	CDB4DEntityRecord* inRecord = inSelIter->GetCurRec(GetDB4DBaseContextFromJSContext(ioParms, inSelIter->GetModel()));
	if (inRecord != nil)
	{
		err = inRecord->Validate();
		validated = err == VE_OK;
	}
	ioParms.ReturnBool(validated);
}


void VJSEntitySelectionIterator::_Valid(VJSParms_callStaticFunction& ioParms, EntitySelectionIterator* inSelIter)
{
	ioParms.ReturnBool(inSelIter->GetCurPos() != -1);
}


void VJSEntitySelectionIterator::_GetModel(VJSParms_callStaticFunction& ioParms, EntitySelectionIterator* inSelIter)
{
	ioParms.ReturnValue(VJSEntityModel::CreateInstance(ioParms.GetContextRef(), inSelIter->GetModel()));
}


void VJSEntitySelectionIterator::_Next(VJSParms_callStaticFunction& ioParms, EntitySelectionIterator* inSelIter)
{
	EntitySelectionIterator* newIter = new EntitySelectionIterator(*inSelIter);
	CDB4DBaseContext* context = GetDB4DBaseContextFromJSContext(ioParms, inSelIter->GetModel());
	newIter->Next(context);
	if (newIter->GetCurPos() != -1)
	{
		ioParms.ReturnValue(VJSEntitySelectionIterator::CreateInstance(ioParms.GetContextRef(), newIter));
	}
	else
	{
		ioParms.ReturnNullValue();
		delete newIter;
	}
}


void VJSEntitySelectionIterator::_Loaded(VJSParms_callStaticFunction& ioParms, EntitySelectionIterator* inSelIter)
{
	ioParms.ReturnBool(inSelIter->GetCurRec(GetDB4DBaseContextFromJSContext(ioParms, inSelIter->GetModel())) != nil);
}


void VJSEntitySelectionIterator::_GetID(VJSParms_callStaticFunction& ioParms, EntitySelectionIterator* inSelIter)
{
	CDB4DEntityRecord* inRecord = inSelIter->GetCurRec(GetDB4DBaseContextFromJSContext(ioParms, inSelIter->GetModel()));
	if (inRecord == nil)
		ioParms.ReturnNumber(-1);
	else
		ioParms.ReturnNumber(inRecord->GetNum());
}


void VJSEntitySelectionIterator::_IsModified(VJSParms_callStaticFunction& ioParms, EntitySelectionIterator* inSelIter)
{
	CDB4DEntityRecord* inRecord = inSelIter->GetCurRec(GetDB4DBaseContextFromJSContext(ioParms, inSelIter->GetModel()));
	if (inRecord == nil)
		ioParms.ReturnBool(false);
	else
		ioParms.ReturnBool(inRecord->IsModified());
}


void VJSEntitySelectionIterator::_IsNew(VJSParms_callStaticFunction& ioParms, EntitySelectionIterator* inSelIter)
{
	CDB4DEntityRecord* inRecord = inSelIter->GetCurRec(GetDB4DBaseContextFromJSContext(ioParms, inSelIter->GetModel()));
	if (inRecord == nil)
		ioParms.ReturnBool(false);
	else
		ioParms.ReturnBool(inRecord->IsNew());
}


void VJSEntitySelectionIterator::_Drop(VJSParms_callStaticFunction& ioParms, EntitySelectionIterator* inSelIter)
{
	VError err = VE_OK;
	CDB4DEntityRecord* inRecord = inSelIter->GetCurRec(GetDB4DBaseContextFromJSContext(ioParms, inSelIter->GetModel()));
	if (inRecord != nil)
		err = inRecord->Drop();
	inSelIter->ReleaseCurCurec(false);
}


void VJSEntitySelectionIterator::_Release(VJSParms_callStaticFunction& ioParms, EntitySelectionIterator* inSelIter)
{
	inSelIter->ReleaseCurCurec(false);
}


void VJSEntitySelectionIterator::_Reload(VJSParms_callStaticFunction& ioParms, EntitySelectionIterator* inSelIter)
{
	VError err = VE_OK;
	bool readonly = ioParms.GetBoolParam( 1, L"ReadOnly", L"ReadWrite");
	err = inSelIter->ReLoadCurRec(GetDB4DBaseContextFromJSContext(ioParms, inSelIter->GetModel()), readonly, false);
}


void VJSEntitySelectionIterator::_getTimeStamp(VJSParms_callStaticFunction& ioParms, EntitySelectionIterator* inSelIter)
{
	VError err = VE_OK;
	CDB4DEntityRecord* inRecord = inSelIter->GetCurRec(GetDB4DBaseContextFromJSContext(ioParms, inSelIter->GetModel()));
	if (inRecord != nil)
	{
		VTime stamp;
		inRecord->GetTimeStamp(stamp);
		ioParms.ReturnTime(stamp);
	}
}


void VJSEntitySelectionIterator::_getKey(XBOX::VJSParms_callStaticFunction& ioParms, EntitySelectionIterator* inSelIter)
{
	VError err = VE_OK;
	
	CDB4DEntityRecord* inRecord = inSelIter->GetCurRec(GetDB4DBaseContextFromJSContext(ioParms, inSelIter->GetModel()));
	if (inRecord != nil)
	{
		VectorOfVValue key;
		inRecord->GetPrimKeyValue(key);
		if (!key.empty())
		{
			if (key.size() == 1)
			{
				ioParms.ReturnVValue(*(key[0]));
			}
			else
			{
				VJSArray arr(ioParms.GetContextRef());
				arr.PushValues(key);
				ioParms.ReturnValue(arr);
			}
		}
	}
}


void VJSEntitySelectionIterator::_getStamp(XBOX::VJSParms_callStaticFunction& ioParms, EntitySelectionIterator* inSelIter)
{
	VError err = VE_OK;
	CDB4DEntityRecord* inRecord = inSelIter->GetCurRec(GetDB4DBaseContextFromJSContext(ioParms, inSelIter->GetModel()));
	if (inRecord != nil)
	{
		ioParms.ReturnNumber(inRecord->GetModificationStamp());
	}
}


void EntityRecToString(CDB4DEntityRecord* inRecord, CDB4DBaseContext* context, VString& result)
{
	result.Clear();
	VError err = VE_OK;
	if (inRecord != nil)
	{
		CDB4DEntityModel* em = inRecord->GetModel();
		bool first = true;
		sLONG nbatt = em->CountAttributes();
		for (sLONG i = 1; i <= nbatt; i++)
		{
			CDB4DEntityAttribute* att = em->GetAttribute(i);
			if (att != nil)
			{
				if (first)
					first = false;
				else
					result += L"  ,  ";
				result += att->GetAttibuteName()+L" : ";
				CDB4DEntityAttributeValue* val = inRecord->GetAttributeValue(att, err);
				if (val == nil)
					result += L"null";
				else
				{
					switch (val->GetAttributeKind())
					{
					case eav_vvalue:
						{
							XBOX::VValueSingle* cv = val->GetVValue();
							if (cv == nil)
								result += L"null";
							else
							{
								VString s;
								cv->GetJSONString(s);
								result += s;
							}

						}
						break;

					case eav_subentity:
						{
							CDB4DEntityRecord* rec = val->GetRelatedEntity();
							if (rec == nil)
								result += L"<<null entity>>";
							else
								result += L"1 entity";
						}
						break;

					case eav_selOfSubentity:
						{
							CDB4DSelection* sel = val->GetRelatedSelection();
							if (sel == nil)
								result += L"<<null selection>>";
							else
							{
								sLONG nbent = sel->CountRecordsInSelection(context);
								result += ToString(nbent)+ ((nbent == 1)?L" entity":L" entities");
							}
						}
						break;;
					}
				}
			}
		}
	}
	else
	{
		result = "<<null Entity>>";
	}
}


void VJSEntitySelectionIterator::_toString(VJSParms_callStaticFunction& ioParms, EntitySelectionIterator* inSelIter)
{
	VString result;
	VError err = VE_OK;
	CDB4DEntityModel* em = inSelIter->GetModel();
	CDB4DBaseContext* context = GetDB4DBaseContextFromJSContext(ioParms, em);
	CDB4DEntityRecord* inRecord = inSelIter->GetCurRec(context);
	EntityRecToString(inRecord, context, result);
	ioParms.ReturnString(result);
}


void VJSEntitySelectionIterator::_toObject(VJSParms_callStaticFunction& ioParms, EntitySelectionIterator* inSelIter)
{
	CDB4DEntityModel* em = inSelIter->GetModel();
	CDB4DBaseContext* context = GetDB4DBaseContextFromJSContext(ioParms, em);
	CDB4DEntityRecord* inRecord = inSelIter->GetCurRec(context);
	if (inRecord != nil)
	{
		EntityRecord* rec = VImpCreator<EntityRecord>::GetImpObject(inRecord);
		VJSObject obj(ioParms.GetContextRef());
		obj.MakeEmpty();
		EntityAttributeSortedSelection atts(rec->GetOwner());
		EntityAttributeSelection expand(rec->GetOwner());
		EntityAttributeSortedSelection sortingatts(rec->GetOwner());
		sLONG curparam = 1;
		buildAttributeListFromParams(ioParms, curparam, rec->GetOwner(), atts, expand, sortingatts, rec->getContext());
		rec->ConvertToJSObject(obj, atts, &expand, &sortingatts, true, false);
		ioParms.ReturnValue(obj);
		
	}
}


void VJSEntitySelectionIterator::_toJSON(VJSParms_callStaticFunction& ioParms, EntitySelectionIterator* inSelIter)
{
	CDB4DEntityModel* em = inSelIter->GetModel();
	CDB4DBaseContext* context = GetDB4DBaseContextFromJSContext(ioParms, em);
	CDB4DEntityRecord* inRecord = inSelIter->GetCurRec(context);
	if (inRecord != nil)
	{
		EntityRecord* rec = VImpCreator<EntityRecord>::GetImpObject(inRecord);
		VJSObject obj(ioParms.GetContextRef());
		obj.MakeEmpty();
		EntityAttributeSortedSelection atts(rec->GetOwner());
		rec->ConvertToJSObject(obj, atts, nil, nil, true, false);
		/*
		VJSJSON json(ioParms.GetContextRef());
		VString s;
		json.Stringify(obj, s);
		ioParms.ReturnString(s);
		*/
		ioParms.ReturnValue(obj);
	}
}



void VJSEntitySelectionIterator::GetDefinition( ClassDefinition& outDefinition)
{
	static inherited::StaticFunction functions[] = 
	{
		{ "next", js_callStaticFunction<_Next>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
	//	{ "valid", js_callStaticFunction<_Valid>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "isLoaded", js_callStaticFunction<_Loaded>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "save", js_callStaticFunction<_Save>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "validate", js_callStaticFunction<_validate>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "getID", js_callStaticFunction<_GetID>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "refresh", js_callStaticFunction<_Reload>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "isModified", js_callStaticFunction<_IsModified>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "isNew", js_callStaticFunction<_IsNew>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "drop", js_callStaticFunction<_Drop>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "remove", js_callStaticFunction<_Drop>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "release", js_callStaticFunction<_Release>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "getTimeStamp", js_callStaticFunction<_getTimeStamp>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "getDataClass", js_callStaticFunction<_GetModel>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "toString", js_callStaticFunction<_toString>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "toJSON", js_callStaticFunction<_toJSON>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "toObject", js_callStaticFunction<_toObject>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "getKey", js_callStaticFunction<_getKey>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "getStamp", js_callStaticFunction<_getStamp>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ 0, 0, 0}
	};
	
	outDefinition.className = "Entity";
	outDefinition.initialize = js_initialize<Initialize>;
	outDefinition.finalize = js_finalize<Finalize>;
	outDefinition.getProperty = js_getProperty<GetProperty>;
	outDefinition.setProperty = js_setProperty<SetProperty>;
	outDefinition.getPropertyNames = js_getPropertyNames<GetPropertyNames>;
	outDefinition.staticFunctions = functions;
}




//======================================================



void VJSEntitySelection::Initialize( const VJSParms_initialize& inParms, CDB4DSelection* inSelection)
{
	inSelection->Retain();
}


void VJSEntitySelection::Finalize( const VJSParms_finalize& inParms, CDB4DSelection* inSelection)
{
	inSelection->Release();
}



void VJSEntitySelection::GetProperty( VJSParms_getProperty& ioParms, CDB4DSelection* inSelection)
{
	
	sLONG num;
	if (ioParms.GetPropertyNameAsLong( &num) && (num >= 0) )
	{
		CDB4DBaseContext* context = GetDB4DBaseContextFromJSContext( ioParms, inSelection);
		CDB4DEntityRecord* rec = inSelection->LoadEntity(num+1, /*DB4D_Keep_Lock_With_Record*/DB4D_Do_Not_Lock, context);
		if (rec != nil)
		{
			EntitySelectionIterator* iter = new EntitySelectionIterator(rec, context);
			ioParms.ReturnValue(VJSEntitySelectionIterator::CreateInstance(ioParms.GetContextRef(), iter));
			rec->Release();
		}
		
	}
	else
	{
		CDB4DEntityModel* em = inSelection->GetModel();
		if (em != nil)
		{
			VString propname;
			ioParms.GetPropertyName(propname);
			CDB4DEntityMethod* meth = em->GetMethod(propname);
			if (meth != nil)
			{
				if (meth->GetMethodKind() == emeth_sel)
				{
					CDB4DBaseContext* context = GetDB4DBaseContextFromJSContext( ioParms, em);
					VJSObject localObjFunc(ioParms.GetContextRef());
					VJSObject* objfunc = meth->GetFuncObject(context, localObjFunc);
					if (objfunc != nil)
					{
						ioParms.ReturnValue(*objfunc);
					}
				}
			}
			else
			{
				CDB4DEntityAttribute* att = em->GetAttribute(propname);
				if (att != nil)
				{
					CDB4DBaseContext* context = GetDB4DBaseContextFromJSContext( ioParms, em);
					EntityAttributeKind kind = att->GetAttributeKind();
					if (kind == eattr_relation_Nto1 || kind == eattr_relation_1toN || kind == eattr_composition)
					{
						VError err = VE_OK;
						CDB4DSelection* result = em->ProjectSelection(inSelection, att, err, context);
						if (result != nil)
							ioParms.ReturnValue(VJSEntitySelection::CreateInstance(ioParms.GetContextRef(), result));
					}
					else
					{
						CDB4DBaseContext* context = GetDB4DBaseContextFromJSContext( ioParms, inSelection);
						sLONG nbelem = inSelection->CountRecordsInSelection(context);
						VJSArray result(ioParms.GetContextRef());
						VError err = VE_OK;
						for (sLONG i = 1; i <= nbelem && err == VE_OK; i++)
						{
							CDB4DEntityRecord* erec = inSelection->LoadEntity(i, DB4D_Do_Not_Lock, context);
							if (erec != nil)
							{
								CDB4DEntityAttributeValue* xval = erec->GetAttributeValue(att, err);
								if (err == VE_OK && xval != nil)
								{
									VValueSingle* cv = xval->GetVValue();
									result.PushValue(*cv);
								}
								//QuickReleaseRefCountable(xval);
								erec->Release();
							}
						}
						if (err == VE_OK)
							ioParms.ReturnValue(result);
					}
				}
			}
		}
		else
			ThrowBaseError(VE_DB4D_NOT_AN_ENTITY_COLLECTION);
	}

}


void computeStats(XBOX::VJSParms_callStaticFunction& ioParms, CDB4DSelection* inSelection, DB4D_ColumnFormulae action)
{
	bool okresult = false;
	CDB4DEntityModel* model = inSelection->GetModel();

	if (model == nil)
		ThrowBaseError(VE_DB4D_NOT_AN_ENTITY_COLLECTION);
	else
	{
		EntityModel* em = VImpCreator<EntityModel>::GetImpObject(model);
		CDB4DBaseContext* context = GetDB4DBaseContextFromJSContext( ioParms, inSelection);

		CDB4DEntityAttribute* xatt = ioParms.GetParamObjectPrivateData<VJSEntityAttribute>(1);
		if (xatt == nil)
		{
			if (ioParms.IsStringParam(1))
			{
				VString s;
				ioParms.GetStringParam(1, s);
				xatt = model->GetAttribute(s);

				if (xatt == nil)
				{
					em->ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_NOT_FOUND, &s);
				}
			}
			else
				vThrowError(VE_JVSC_WRONG_PARAMETER_TYPE_ATTRIBUTE, "1");
		}

		if (xatt != nil)
		{
			if (xatt->GetModel() != model)
			{
				VString s = em->GetName()+"."+xatt->GetAttibuteName();
				ThrowBaseError(VE_DB4D_ENTITY_ATTRIBUTE_IS_FROM_ANOTHER_DATACLASS, s);
				xatt = nil;
			}
			else
			{
				EntityAttribute* att = VImpCreator<EntityAttribute>::GetImpObject(xatt);
				if (action == DB4D_Count || action == DB4D_Count_distinct || action == DB4D_Min || action == DB4D_Max)
				{
					if (!att->IsStatable())
					{
						att->ThrowError(VE_DB4D_WRONG_ATTRIBUTE_KIND);
					}
				}
				else
				{
					if (!att->IsSummable())
					{
						att->ThrowError(VE_DB4D_WRONG_ATTRIBUTE_KIND);
					}
				}
			}
		}

		if (xatt != nil)
		{
			EntityAttribute* att = VImpCreator<EntityAttribute>::GetImpObject(xatt);
			Selection* sel = VImpCreator<VDB4DSelection>::GetImpObject(inSelection)->GetSel();
			Selection* selsorted = nil;

			if (action >= DB4D_Count_distinct && action <=DB4D_Sum_distinct)
			{
				EntityAttributeSortedSelection sortingAtt(em);
				sortingAtt.AddAttribute(att, nil);
				VError err = VE_OK;
				selsorted = sel->SortSel(err, em, &sortingAtt, ConvertContext(context));
				if (selsorted != nil)
					sel = selsorted;
			}

			if (att->GetKind() == eattr_storage /*|| att->GetKind() == eattr_field*/)
			{
				Field* cri = att->RetainDirectField();
				if (cri != nil)
				{
					ColumnFormulas formules(em->GetMainTable());
					formules.AddAction(action, cri);
					VError err = formules.Execute(sel, ConvertContext(context), nil, nil, sel == selsorted);
					if (err == VE_OK)
					{
						VValueSingle* result = formules.GetResult(0);
						if (result != nil)
						{
							if (result->IsNull() && (action == DB4D_Sum || action == DB4D_Count))
								ioParms.ReturnNumber(0);
							else
								ioParms.ReturnVValue(*result);
							okresult = true;
						}
					}
					cri->Release();
				}
			}
			else
			{
				EntityAttributeSortedSelection attlist(em);
				attlist.AddAttribute(att, nil);

				VJSObject result(ioParms.GetContextRef());

				VError err  = em->compute(attlist, sel, result, ConvertContext(context), ioParms.GetContextRef());
				if (err == VE_OK)
				{
					VJSValue subatt(ioParms.GetContextRef());
					subatt = result.GetProperty(att->GetName());
					if (subatt.IsObject())
					{
						VJSObject subattobj(ioParms.GetContextRef());
						subatt.GetObject(subattobj);

						VJSValue subresult(ioParms.GetContextRef());
						switch (action)
						{
							case DB4D_Sum:
								subresult = subattobj.GetProperty("sum");
								break;
							case DB4D_Average:
								subresult = subattobj.GetProperty("average");
								break;
							case DB4D_Min:
								subresult = subattobj.GetProperty("min");
								break;
							case DB4D_Max:
								subresult = subattobj.GetProperty("max");
								break;
							case DB4D_Count:
								subresult = subattobj.GetProperty("count");
								break;
						}
						if ( (subresult.IsUndefined() || subresult.IsNull()) && (action == DB4D_Sum || action == DB4D_Count) )
							ioParms.ReturnNumber(0);
						else
						{
							if (subresult.IsUndefined())
								ioParms.ReturnNullValue();
							else
								ioParms.ReturnValue(subresult);
						}
						okresult = true;
					}
				}
			}
			QuickReleaseRefCountable(selsorted);
		}
	}

	if (!okresult)
	{
		ioParms.ReturnNullValue();
	}

}


void VJSEntitySelection::_compute(XBOX::VJSParms_callStaticFunction& ioParms, CDB4DSelection* inSelection)
{
	VError err = VE_OK;
	bool okresult = false;
	CDB4DBaseContext* context = GetDB4DBaseContextFromJSContext( ioParms, inSelection);
	CDB4DEntityModel* model = inSelection->GetModel();
	if (model == nil)
		err = ThrowBaseError(VE_DB4D_NOT_AN_ENTITY_COLLECTION);
	else
	{
		EntityModel* em = VImpCreator<EntityModel>::GetImpObject(model);
		EntityAttributeSortedSelection attlist(em);
		sLONG curparam = GetAttributeListParams(ioParms, 1, attlist);
		bool withdistinct = ioParms.GetBoolParam(curparam, L"distinct", L"");
		DB4D_ColumnFormulae lastaction = DB4D_Count;
		if (withdistinct)
			lastaction = DB4D_Sum_distinct;

		bool allAttributesAreDirect = true;

		if (!attlist.empty())
		{
			Selection* sel = VImpCreator<VDB4DSelection>::GetImpObject(inSelection)->GetSel();
			Selection* selsorted = nil;
			if (withdistinct)
			{
				EntityAttributeSortedSelection sortingAtt(em);
				sortingAtt.AddAttribute(attlist[0].fAttribute, nil);
				VError err = VE_OK;
				selsorted = sel->SortSel(err, em, &sortingAtt, ConvertContext(context));
				if (selsorted != nil)
					sel = selsorted;
			}

			ColumnFormulas formules(em->GetMainTable());
			for (EntityAttributeSortedSelection::const_iterator cur = attlist.begin(), end = attlist.end(); cur != end && err == VE_OK; cur++)
			{
				const EntityAttribute* att = cur->fAttribute;
				if (!att->IsStatable())
				{
					err = att->ThrowError(VE_DB4D_WRONG_ATTRIBUTE_KIND);
				}
				else if (att->GetOwner() != em)
				{
					VString s = att->GetOwner()->GetName()+"."+att->GetAttibuteName();
					err = ThrowBaseError(VE_DB4D_ENTITY_ATTRIBUTE_IS_FROM_ANOTHER_DATACLASS, s);
				}
				else
				{
					if (/*att->GetKind() != eattr_field &&*/ att->GetKind() != eattr_storage)
					{
						allAttributesAreDirect = false;
					}
					else
					{
						Field* cri = att->RetainDirectField();
						if (cri != nil)
						{
							for (DB4D_ColumnFormulae action = DB4D_Sum; action <= lastaction; action = (DB4D_ColumnFormulae)((sLONG)action+1))
								formules.AddAction(action, cri);
							cri->Release();
						}
					}
				}
			}

			if (err == VE_OK)
			{
				if (allAttributesAreDirect)
				{
					err = formules.Execute(sel, ConvertContext(context), nil, nil, sel == selsorted);
					if (err == VE_OK)
					{
						VJSObject result(ioParms.GetContextRef());
						result.MakeEmpty();

						sLONG curcol = 0;
						for (EntityAttributeSortedSelection::const_iterator cur = attlist.begin(), end = attlist.end(); cur != end; cur++)
						{
							const EntityAttribute* att = cur->fAttribute;
							if (/*att->GetKind() != eattr_field &&*/ att->GetKind() != eattr_storage)
							{
								allAttributesAreDirect = false;
							}
							else
							{
								Field* cri = att->RetainDirectField();
								if (cri != nil)
								{
									VJSObject oneatt(ioParms.GetContextRef());
									oneatt.MakeEmpty();

									for (DB4D_ColumnFormulae action = DB4D_Sum; action <= lastaction; action = (DB4D_ColumnFormulae)((sLONG)action+1), curcol++)
									{
										if ( (action == DB4D_Sum || action == DB4D_Average || action == DB4D_Average_distinct || action == DB4D_Sum_distinct) && !att->IsSummable())
										{
											// nothing to do here
										}
										else
										{
											VValueSingle* cv = formules.GetResult(curcol);
											if (cv != nil)
											{
												VJSValue jsval(ioParms.GetContextRef());
												jsval.SetVValue(*cv);
												VString actionstring;
												switch (action)
												{
													case DB4D_Sum:
														actionstring = L"sum";
														break;
													case DB4D_Average:
														actionstring = L"average";
														break;
													case DB4D_Min:
														actionstring = L"min";
														break;
													case DB4D_Max:
														actionstring = L"max";
														break;
													case DB4D_Count:
														actionstring = L"count";
														break;
													case DB4D_Count_distinct:
														actionstring = L"count_distinct";
														break;
													case DB4D_Average_distinct:
														actionstring = L"average_distinct";
														break;
													case DB4D_Sum_distinct:
														actionstring = L"sum_distinct";
														break;
												}
												oneatt.SetProperty(actionstring, jsval, JS4D::PropertyAttributeNone);
											}
										}
									}
									cri->Release();

									result.SetProperty(att->GetName(), oneatt, JS4D::PropertyAttributeNone);
								}
							}
						}

						ioParms.ReturnValue(result);
						okresult = true;
					}

				}
				else
				{
					VJSObject result(ioParms.GetContextRef());
					err  = em->compute(attlist, sel, result, ConvertContext(context), ioParms.GetContextRef());
					if (err == VE_OK)
					{
						ioParms.ReturnValue(result);
						okresult = true;
					}
				}
			}
			QuickReleaseRefCountable(selsorted);
		}
	}

	if (!okresult)
	{
		ioParms.ReturnNullValue();
	}
}


static void _opersel(XBOX::VJSParms_callStaticFunction& ioParms, CDB4DSelection* inSelection, DB4DConjunction oper)
{
	bool okresult = false;
	CDB4DBaseContext* context = GetDB4DBaseContextFromJSContext( ioParms, inSelection);
	CDB4DEntityModel* model = inSelection->GetModel();
	if (model == nil)
		ThrowBaseError(VE_DB4D_NOT_AN_ENTITY_COLLECTION);
	else
	{
		CDB4DSelection* otherSelection = ioParms.GetParamObjectPrivateData<VJSEntitySelection>(1);
		if (otherSelection == nil)
			ThrowBaseError(VE_DB4D_NOT_AN_ENTITY_COLLECTION);
		else
		{
			CDB4DEntityModel* otherModel = otherSelection->GetModel();
			if (otherModel == nil)
				ThrowBaseError(VE_DB4D_NOT_AN_ENTITY_COLLECTION);
			else
			{
				EntityModel* em = VImpCreator<EntityModel>::GetImpObject(model);
				EntityModel* otherem = VImpCreator<EntityModel>::GetImpObject(otherModel);
				if (otherem->isExtendedFrom(em))
				{
					VError err = VE_OK;
					Selection* sel  = VImpCreator<VDB4DSelection>::GetImpObject(inSelection)->GetSel();
					Selection* othersel  = VImpCreator<VDB4DSelection>::GetImpObject(otherSelection)->GetSel();
					BaseTaskInfo* xcontext = ConvertContext(context);
					Bittab* b1 = sel->GenereBittab(xcontext, err);
					Bittab* b2 = othersel->GenereBittab(xcontext, err);
					if (b1 != nil && b2 != nil && err == VE_OK)
					{
						Bittab* bresult = b1->Clone(err);
						if (err == VE_OK && bresult != nil)
						{
							okresult = true;
							switch (oper)
							{
								case DB4D_OR:
									bresult->Or(b2);
									break;
								case DB4D_And:
									bresult->And(b2);
									break;
								case DB4D_Except:
									bresult->moins(b2);
									break;
							}
							Selection* newsel = new BitSel(sel->GetParentFile(), bresult);
							CDB4DSelection* resultsel = new VDB4DSelection(VDBMgr::GetManager(), VImpCreator<VDB4DBase>::GetImpObject(inSelection->GetBaseRef()), em->GetMainTable(), newsel, em);
							ioParms.ReturnValue(VJSEntitySelection::CreateInstance(ioParms.GetContextRef(), resultsel));
							QuickReleaseRefCountable(resultsel);
						}
						QuickReleaseRefCountable(bresult);
					}
					QuickReleaseRefCountable(b1);
					QuickReleaseRefCountable(b2);
				}
				else
					ThrowBaseError(VE_DB4D_COLLECTION_ON_INCOMPATIBLE_DATACLASSES);

			}
		}
	}
	if (!okresult)
		ioParms.ReturnNullValue();
}

void VJSEntitySelection::_and(XBOX::VJSParms_callStaticFunction& ioParms, CDB4DSelection* inSelection)
{
	_opersel(ioParms, inSelection, DB4D_And);
}

void VJSEntitySelection::_or(XBOX::VJSParms_callStaticFunction& ioParms, CDB4DSelection* inSelection)
{
	_opersel(ioParms, inSelection, DB4D_OR);
}

void VJSEntitySelection::_minus(XBOX::VJSParms_callStaticFunction& ioParms, CDB4DSelection* inSelection)
{
	_opersel(ioParms, inSelection, DB4D_Except);
}



void VJSEntitySelection::_Count(VJSParms_callStaticFunction& ioParms, CDB4DSelection* inSelection)
{
	if (ioParms.CountParams() > 0)
		computeStats(ioParms, inSelection, ioParms.GetBoolParam(2,L"distinct",L"") ? DB4D_Count_distinct : DB4D_Count);
	else
		ioParms.ReturnNumber(inSelection->CountRecordsInSelection( GetDB4DBaseContextFromJSContext( ioParms, inSelection)));
}


void VJSEntitySelection::_getLength( XBOX::VJSParms_getProperty& ioParms, CDB4DSelection* inSelection)
{
	ioParms.ReturnNumber(inSelection->CountRecordsInSelection( GetDB4DBaseContextFromJSContext( ioParms, inSelection)));
}


void VJSEntitySelection::_getQueryPlan( XBOX::VJSParms_getProperty& ioParms, CDB4DSelection* inSelection)
{
	const VValueBag* queryplan = inSelection->GetQueryPlan();
	if (queryplan != nil)
	{
		VString jsons;
		queryplan->GetJSONString(jsons, JSON_UniqueSubElementsAreNotArrays);
		VJSJSON json(ioParms.GetContextRef());

		VJSValue result(json.Parse(jsons));

		if (!result.IsUndefined())
		{
			ioParms.ReturnValue(VJSValue(ioParms.GetContextRef(), result));
		}
		else
			ioParms.ReturnNullValue();
	}
}


void VJSEntitySelection::_getQueryPath( XBOX::VJSParms_getProperty& ioParms, CDB4DSelection* inSelection)
{
	const VValueBag* queryplan = inSelection->GetQueryPath();
	if (queryplan != nil)
	{
		VString jsons;
		queryplan->GetJSONString(jsons, JSON_UniqueSubElementsAreNotArrays);
		VJSJSON json(ioParms.GetContextRef());

		VJSValue result(json.Parse(jsons));

		if (!result.IsUndefined())
		{
			ioParms.ReturnValue(VJSValue(ioParms.GetContextRef(), result));
		}
		else
			ioParms.ReturnNullValue();
	}
}




void VJSEntitySelection::_First(VJSParms_callStaticFunction& ioParms, CDB4DSelection* inSelection)
{
	bool readonly = false;
	if (ioParms.CountParams() > 0 && ioParms.IsStringParam(1))
	{
		VString s;
		ioParms.GetStringParam(1, s);
		if (s == L"ReadOnly")
			readonly = true;
	}
	else
		ioParms.GetBoolParam(1, &readonly);
	
	CDB4DBaseContext* context = GetDB4DBaseContextFromJSContext( ioParms, inSelection);
	EntitySelectionIterator* iter = new EntitySelectionIterator(inSelection, readonly, true, context, inSelection->GetModel());
	iter->First(context);
	if (iter->GetCurRec(context) != nil)
	{
		ioParms.ReturnValue(VJSEntitySelectionIterator::CreateInstance(ioParms.GetContextRef(), iter));
	}
	else
	{
		ioParms.ReturnNullValue();
		delete iter;
	}
}


void VJSEntitySelection::_OrderBy(VJSParms_callStaticFunction& ioParms, CDB4DSelection* inSelection)
{
	CDB4DEntityModel* model = inSelection->GetModel();
	bool ok = false;
	VError err = VE_OK;

	if (model == nil)
	{
		ThrowBaseError(VE_DB4D_NOT_AN_ENTITY_COLLECTION);
		ioParms.ReturnNullValue();
	}
	else
	{
		VString sortorder;
		if (ioParms.IsStringParam(1))
		{
			ioParms.GetStringParam(1, sortorder);
		}
		else
		{
			for (sLONG i = 1, nb = ioParms.CountParams(); i <= nb && err == VE_OK; i++)
			{
				if (ioParms.IsStringParam(i))
				{
					VString s;
					ioParms.GetStringParam(i, s);
					if (s == "asc" || s == "desc")
						sortorder += L" "+s;
					else
					{
						if (i != 1)
							s += L",";
						sortorder += s;
					}
				}
				else
				{
					CDB4DEntityAttribute* att = ioParms.GetParamObjectPrivateData<VJSEntityAttribute>(i);
					if (att != nil)
					{
						if (i != 1)
							sortorder += L",";
						if (att->GetModel() == model)
							sortorder += att->GetAttibuteName();
						else
						{
							VString s = att->GetModel()->GetEntityName()+L"."+att->GetAttibuteName();
							ThrowBaseError(VE_DB4D_ENTITY_ATTRIBUTE_IS_FROM_ANOTHER_DATACLASS, s);
							//sortorder += att->GetModel()->GetEntityName()+L"."+att->GetAttibuteName();
						}
					}
					else
					{
						err = vThrowError(VE_JVSC_WRONG_PARAMETER_TYPE_ATTRIBUTE, "1");
					}
				}
			}
		}
		if (!sortorder.IsEmpty())
		{
			ok = inSelection->SortSelection(sortorder, nil, GetDB4DBaseContextFromJSContext( ioParms, inSelection));
		}
		if (ok)
			ioParms.ReturnValue(VJSEntitySelection::CreateInstance(ioParms.GetContextRef(), inSelection));
		else
			ioParms.ReturnNullValue();
	}
}


void VJSEntitySelection::_Add(VJSParms_callStaticFunction& ioParms, CDB4DSelection* inSelection)
{
	VError err = VE_OK;
	if (ioParms.CountParams() > 0)
	{
		CDB4DBaseContext* context = GetDB4DBaseContextFromJSContext( ioParms, inSelection);
		bool atTheEnd = ioParms.GetBoolParam( 2, L"AtTheEnd", L"AnyWhere");
		CDB4DSelection* sel = ioParms.GetParamObjectPrivateData<VJSEntitySelection>(1);
		if (sel != nil)
		{
			for (sLONG i = 0, nb = sel->CountRecordsInSelection(nil); i < nb && err == VE_OK; i++)
			{
				err = inSelection->AddRecordID(sel->GetSelectedRecordID(i+1, nil), atTheEnd, context);
			}
		}
		else
		{
			{
				EntitySelectionIterator* seliter = ioParms.GetParamObjectPrivateData<VJSEntitySelectionIterator>(1);
				if (seliter != nil)
				{
					sLONG recid = seliter->GetCurRecID();
					if (recid >= 0)
						err = inSelection->AddRecordID(recid, atTheEnd, context);
				}
				else
				{
					if (ioParms.IsNumberParam(1))
					{
						sLONG recid;
						ioParms.GetLongParam(1, &recid);
						if (recid >= 0)
							err = inSelection->AddRecordID(recid, atTheEnd, context);
					}
				}
			}
		}
	}
	ioParms.ReturnThis();
}



void VJSEntitySelection::_GetModel(VJSParms_callStaticFunction& ioParms, CDB4DSelection* inSelection)
{
	ioParms.ReturnValue(VJSEntityModel::CreateInstance(ioParms.GetContextRef(), inSelection->GetModel()));
}


void VJSEntitySelection::_Each(XBOX::VJSParms_callStaticFunction& ioParms, CDB4DSelection* inSelection)
{
	VJSValue valfunc(ioParms.GetParamValue(1));
	vector<VJSValue> params;
	if (!valfunc.IsUndefined() && valfunc.IsObject())
	{
		VJSObject objfunc(ioParms.GetContextRef());
		valfunc.GetObject(objfunc);
		if (objfunc.IsFunction())
		{
			VJSValue elemVal(ioParms.GetContextRef());
			params.push_back(elemVal);
			VJSValue indiceVal(ioParms.GetContextRef());
			params.push_back(indiceVal);
			params.push_back(ioParms.GetThis());

			VJSValue thisParamVal(ioParms.GetParamValue(2));
			VJSObject thisParam(thisParamVal.GetObject());

			CDB4DBaseContext* context = GetDB4DBaseContextFromJSContext( ioParms, inSelection);
			bool cont = true;
			for (sLONG i = 1, nb = inSelection->CountRecordsInSelection(context); i <= nb && cont; i++)
			{
				CDB4DEntityRecord* erec = inSelection->LoadEntity(i, DB4D_Do_Not_Lock, context);
				if (erec != nil)
				{
					params[1].SetNumber(i-1);
					EntitySelectionIterator* iter = new EntitySelectionIterator(erec, context);
					VJSObject thisobj(VJSEntitySelectionIterator::CreateInstance(ioParms.GetContextRef(), iter));
					params[0] = thisobj;
					VJSValue result(ioParms.GetContextRef());
					JS4D::ExceptionRef except = nil;
					thisParam.CallFunction(objfunc, &params, &result, &except);
					if (except != nil)
					{
						ioParms.SetException(except);
						cont = false;
					}
					else
					{
						if (erec->IsModified())
						{
							sLONG stamp = erec->GetModificationStamp();
							if (stamp == 0)
								stamp = 1;
							erec->Save(stamp);
						}
					}
					erec->Release();
				}
			}
		}
	}
}


void VJSEntitySelection::_dropEntities(XBOX::VJSParms_callStaticFunction& ioParms, CDB4DSelection* inSelection)
{
	bool okreturned = false;
	CDB4DBaseContext* context = GetDB4DBaseContextFromJSContext( ioParms, inSelection);
	CDB4DTable* tt = inSelection->GetBaseRef()->RetainNthTable(inSelection->GetTableRef(), context);
	CDB4DSet* locked = nil;
	if (tt != nil)
		locked = tt->NewSet();
	VError err = inSelection->DeleteRecords(context, locked, nil);
	if (locked != nil)
	{
		VError err;
		CDB4DSelection* result = locked->ConvertToSelection(err, context);
		if (result != nil)
			ioParms.ReturnValue(VJSEntitySelection::CreateInstance(ioParms.GetContextRef(), result));
		QuickReleaseRefCountable(result);
	}
	QuickReleaseRefCountable(locked);
	QuickReleaseRefCountable(tt);
	if (!okreturned)
		ioParms.ReturnNullValue();
}


void VJSEntitySelection::_toString(XBOX::VJSParms_callStaticFunction& ioParms, CDB4DSelection* inSelection)
{
	VString out;
	CDB4DBaseContext* context = GetDB4DBaseContextFromJSContext( ioParms, inSelection);
	CDB4DEntityModel* model = inSelection->GetModel();
	if (model != nil)
	{
		sLONG count = 0;
		sLONG selcount = inSelection->CountRecordsInSelection(context);
		ioParms.GetLongParam(1, &count);
		if (count == 0)
			count = 100;
		if (count == -1 || count > selcount)
			count = selcount;
		VError err = VE_OK;
		for (sLONG i = 0; i < count; i++)
		{
			//CDB4DEntityRecord* rec = model->LoadEntity(inSelection->getrecordid(i), err, DB4D_Do_Not_Lock, context, false);
			CDB4DEntityRecord* rec = inSelection->LoadEntity( i+1, DB4D_Do_Not_Lock, context, nil);
			if (rec != nil)
			{
				VString s;
				EntityRecToString(rec, context, s);
				out += s;
				out += L"\n";
				rec->Release();
			}
		}
	}
	ioParms.ReturnString(out);
}


void VJSEntitySelection::_distinctValues(XBOX::VJSParms_callStaticFunction& ioParms, CDB4DSelection* inSelection)
{
	CDB4DBaseContext* context = GetDB4DBaseContextFromJSContext( ioParms, inSelection);
	CDB4DEntityModel* model = inSelection->GetModel();
	CDB4DEntityAttribute* att = ioParms.GetParamObjectPrivateData<VJSEntityAttribute>(1);
	bool okresult = false;
	if (att == nil)
	{
		VString s;
		ioParms.GetStringParam(1, s);
		att = model->GetAttribute(s);
	}
	EntityAttribute* xatt = nil;
	if (att != nil)
	{
		xatt = VImpCreator<EntityAttribute>::GetImpObject(att);
		EntityModel* em = VImpCreator<EntityModel>::GetImpObject(model);
		if (xatt->GetOwner() != em)
		{
			VString s = xatt->GetOwner()->GetName()+"."+xatt->GetAttibuteName();
			ThrowBaseError(VE_DB4D_ENTITY_ATTRIBUTE_IS_FROM_ANOTHER_DATACLASS, s);
		}
	}
	if (att != nil)
	{
		EntityAttributeKind kind = att->GetAttributeKind();
		if (kind == eattr_storage || kind == eattr_computedField || kind == eattr_alias)
		{
			JSCollectionManager collection(ioParms.GetContextRef());
			collection.SetNumberOfColumn(1);
			VError err = inSelection->GetDistinctValues(att, collection, context, nil);
			if (err == VE_OK)
			{
				okresult = true;
				VJSArray result(collection.getArrayRef(1));
				ioParms.ReturnValue(result);
			}
		}
		else
			xatt->ThrowError(VE_DB4D_WRONG_ATTRIBUTE_KIND);

		/*
		EntityAttribute* xatt = VImpCreator<EntityAttribute>::GetImpObject(att);
		sLONG fieldpos = xatt->GetFieldPos();
		if (fieldpos > 0)
		{
			CDB4DTable* table = model->RetainTable();
			if (table != nil)
			{
				CDB4DField* field = table->RetainNthField(fieldpos, context);
				if (field != nil)
				{
					JSCollectionManager collection(ioParms.GetContextRef());
					collection.SetNumberOfColumn(1);
					VError err = inSelection->GetDistinctValues(field, collection, context, nil);
					if (err == VE_OK)
					{
						okresult = true;
						VJSArray result(collection.getArrayRef(1));
						ioParms.ReturnValue(result);
					}
					field->Release();
				}
				table->Release();
			}
		}
		*/
	}
	if (!okresult)
		ioParms.ReturnNullValue();
}


void buildAttributeListFromParams(XBOX::VJSParms_callStaticFunction& ioParms, sLONG& StartParam, EntityModel* model, EntityAttributeSortedSelection& outList, 
								  EntityAttributeSelection& outExpand, EntityAttributeSortedSelection& outSortingAtts, BaseTaskInfo* context)
{
	if (ioParms.IsStringParam(StartParam))
	{
		VString s;
		ioParms.GetStringParam(StartParam, s);
		model->BuildListOfAttributes(s, outExpand, false, nil);
		model->BuildListOfSortedAttributes(s, outList, context, false, false, nil);
		StartParam++;
		if (ioParms.IsStringParam(StartParam))
		{
			ioParms.GetStringParam(StartParam, s);
			if (s != "withkey")
			{
				model->BuildListOfSortedAttributes(s, outSortingAtts, context, false, true, nil);
				StartParam++;
			}
		}
		/*
		for (EntityAttributeSelection::iterator cur = outExpand.begin(), end = outExpand.end(); cur != end; cur++)
		{
			EntityAttribute* att = cur->fAttribute;
			if (att != nil)
				outList.AddAttribute(att);
		}
		*/
	}
	else
	{
		while (StartParam <= ioParms.CountParams())
		{
			CDB4DEntityAttribute* att = ioParms.GetParamObjectPrivateData<VJSEntityAttribute>(StartParam);
			if (att != nil)
			{
				EntityAttribute* xatt = VImpCreator<EntityAttribute>::GetImpObject(att);
				if (xatt->GetOwner() == model)
				{
					outList.AddAttribute(xatt, nil);
				}
				StartParam++;
			}
			else
				break;
		}

	}
}


void VJSEntitySelection::_toArray(XBOX::VJSParms_callStaticFunction& ioParms, CDB4DSelection* inSelection)
{
	bool okresult = false;
	CDB4DBaseContext* context = GetDB4DBaseContextFromJSContext( ioParms, inSelection);
	CDB4DEntityModel* model = inSelection->GetModel();
	EntityModel* xmodel = VImpCreator<EntityModel>::GetImpObject(model);
	EntityAttributeSortedSelection attlist(xmodel);
	EntityAttributeSelection expand(xmodel);
	EntityAttributeSortedSelection sortingatts(xmodel);
	sLONG curparam = 1;
	sLONG startvalue = 0;
	sLONG nbvalue = -1;
	VError err = VE_OK;
	VJSObject objfunc(ioParms.GetContextRef());
	bool withfunc = false;
	BaseTaskInfo* basecontext = ConvertContext(context);
	
	VJSObject thisParam(ioParms.GetContextRef());
	VJSValue valfunc(ioParms.GetParamValue(curparam));
	vector<VJSValue> params;
	if (!valfunc.IsUndefined() && valfunc.IsObject())
	{
		valfunc.GetObject(objfunc);
		if (objfunc.IsFunction())
		{
			VJSValue elemVal(ioParms.GetContextRef());
			params.push_back(elemVal);
			VJSValue indiceVal(ioParms.GetContextRef());
			params.push_back(indiceVal);
			params.push_back(ioParms.GetThis());

			thisParam.MakeEmpty();
			withfunc = true;
			curparam++;
		}
	}

	if (!withfunc)
		buildAttributeListFromParams(ioParms, curparam, xmodel, attlist, expand, sortingatts, basecontext);

	size_t newparam = curparam;
	bool withkey = ioParms.GetBoolParam(curparam, "withKey", "withoutKey", &newparam);
	curparam = newparam;

	if (ioParms.IsNumberParam(curparam))
	{
		ioParms.GetLongParam(curparam, &startvalue);
		curparam++;
	}
	if (ioParms.IsNumberParam(curparam))
	{
		ioParms.GetLongParam(curparam, &nbvalue);
		curparam++;
	}

	VJSArray arr(ioParms.GetContextRef());

	sLONG count = inSelection->CountRecordsInSelection(context);
	if (startvalue < 0)
		startvalue = 0;
	count = count-startvalue;
	if (count < 0)
		count = 0;
	if (count > nbvalue && nbvalue != -1)
		count = nbvalue;
	sLONG lastvalue = startvalue + count;

	if (withfunc)
	{
		sLONG curindice = 0;
		bool cont = true;
		for (sLONG i = startvalue; i < lastvalue && cont; i++, curindice++)
		{
			CDB4DEntityRecord* erec = inSelection->LoadEntity(i+1, DB4D_Do_Not_Lock, context);
			if (erec != nil)
			{
				params[1].SetNumber(curindice);
				EntitySelectionIterator* iter = new EntitySelectionIterator(erec, context);
				VJSObject thisobj(VJSEntitySelectionIterator::CreateInstance(ioParms.GetContextRef(), iter));
				params[0] = thisobj;
				VJSValue result(ioParms.GetContextRef());
				JS4D::ExceptionRef except = nil;
				thisParam.CallFunction(objfunc, &params, &result, &except);
				if (except != nil)
				{
					ioParms.SetException(except);
					cont = false;
				}
				arr.PushValue(result);		
			}
			QuickReleaseRefCountable(erec);
		}
	}
	else
	{
		Selection* sel = VImpCreator<VDB4DSelection>::GetImpObject(inSelection)->GetSel();
		err = SelToJSObject(basecontext, arr, xmodel, sel, attlist, &expand, &sortingatts, withkey, false, startvalue, nbvalue);
		if (err != VE_OK)
			okresult = false;
	}

	okresult = true;
	ioParms.ReturnValue(arr);

	if (!okresult)
		ioParms.ReturnNullValue();
}


void VJSEntitySelection::_Query(XBOX::VJSParms_callStaticFunction& ioParms, CDB4DSelection* inSelection)
{
	CDB4DEntityModel* model = inSelection->GetModel();
	QueryJS(ioParms, model, inSelection);
}


void VJSEntitySelection::_Find(XBOX::VJSParms_callStaticFunction& ioParms, CDB4DSelection* inSelection)
{
	CDB4DEntityModel* model = inSelection->GetModel();
	FindJS(ioParms, model, inSelection);
}


void VJSEntitySelection::_sum(XBOX::VJSParms_callStaticFunction& ioParms, CDB4DSelection* inSelection)
{
	computeStats(ioParms, inSelection, ioParms.GetBoolParam(2,L"distinct",L"") ? DB4D_Sum_distinct : DB4D_Sum);
}


void VJSEntitySelection::_min(XBOX::VJSParms_callStaticFunction& ioParms, CDB4DSelection* inSelection)
{
	computeStats(ioParms, inSelection, DB4D_Min);
}


void VJSEntitySelection::_max(XBOX::VJSParms_callStaticFunction& ioParms, CDB4DSelection* inSelection)
{
	computeStats(ioParms, inSelection, DB4D_Max);
}


void VJSEntitySelection::_average(XBOX::VJSParms_callStaticFunction& ioParms, CDB4DSelection* inSelection)
{
	computeStats(ioParms, inSelection, ioParms.GetBoolParam(2,L"distinct",L"") ? DB4D_Average_distinct : DB4D_Average);
}


void VJSEntitySelection::_toJSON(XBOX::VJSParms_callStaticFunction& ioParms, CDB4DSelection* inSelection)
{
	CDB4DBaseContext* context = GetDB4DBaseContextFromJSContext( ioParms, inSelection);
	CDB4DEntityModel* model = inSelection->GetModel();
	EntityModel* xmodel = VImpCreator<EntityModel>::GetImpObject(model);
	Selection* sel = VImpCreator<VDB4DSelection>::GetImpObject(inSelection)->GetSel();
	
	VJSArray obj(ioParms.GetContextRef());
	EntityAttributeSortedSelection atts(xmodel);
	SelToJSObject(ConvertContext(context), obj, xmodel, sel, atts, nil, nil, true, false, -1, -1);
	/*
	VJSJSON json(ioParms.GetContextRef());
	VString s;
	json.Stringify(obj, s);
	ioParms.ReturnString(s);
	*/
	ioParms.ReturnValue(obj);
}



void VJSEntitySelection::GetDefinition( ClassDefinition& outDefinition)
{
	static inherited::StaticFunction functions[] = 
	{
		{ "first", js_callStaticFunction<_First>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "count", js_callStaticFunction<_Count>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "sort", js_callStaticFunction<_OrderBy>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "orderBy", js_callStaticFunction<_OrderBy>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "add", js_callStaticFunction<_Add>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "getDataClass", js_callStaticFunction<_GetModel>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "forEach", js_callStaticFunction<_Each>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
//		{ "each", js_callStaticFunction<_Each>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "remove", js_callStaticFunction<_dropEntities>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "drop", js_callStaticFunction<_dropEntities>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "toString", js_callStaticFunction<_toString>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "distinctValues", js_callStaticFunction<_distinctValues>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "toArray", js_callStaticFunction<_toArray>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "find", js_callStaticFunction<_Find>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "query", js_callStaticFunction<_Query>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "filter", js_callStaticFunction<_Query>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "sum", js_callStaticFunction<_sum>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "min", js_callStaticFunction<_min>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "max", js_callStaticFunction<_max>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "average", js_callStaticFunction<_average>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "compute", js_callStaticFunction<_compute>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "toJSON", js_callStaticFunction<_toJSON>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "and", js_callStaticFunction<_and>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "or", js_callStaticFunction<_or>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "minus", js_callStaticFunction<_minus>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ 0, 0, 0}
	};

	static inherited::StaticValue values[] = 
	{
		{ "length", js_getProperty<_getLength>, nil, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "queryPlan", js_getProperty<_getQueryPlan>, nil, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "queryPath", js_getProperty<_getQueryPath>, nil, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ 0, 0, 0, 0}
	};

	outDefinition.className = "EntityCollection";
	outDefinition.initialize = js_initialize<Initialize>;
	outDefinition.finalize = js_finalize<Finalize>;
	outDefinition.getProperty = js_getProperty<GetProperty>;
	outDefinition.staticFunctions = functions;
	outDefinition.staticValues = values;
}





// --------------------------------------------------------------------------------------------------------------------------------------------------------------------


void VJSCacheManager::Initialize( const XBOX::VJSParms_initialize& inParms, VDBMgr* inMgr)
{
}


void VJSCacheManager::Finalize( const XBOX::VJSParms_finalize& inParms, VDBMgr* inMgr)
{
}


void VJSCacheManager::_getObjects(XBOX::VJSParms_callStaticFunction& ioParms, VDBMgr* inMgr)
{
	VMemStats stats;
	VDBMgr::GetManager()->GetCacheManager()->GetMemoryManager()->GetStatistics(stats);
	VJSObject result(ioParms.GetContextRef());
	result.MakeEmpty();
	VJSValue jsval(ioParms.GetContextRef());
	jsval.SetNumber(stats.fUsedMem);
	result.SetProperty(L"usedMem", jsval, JS4D::PropertyAttributeNone);
	jsval.SetNumber(stats.fFreeMem);
	result.SetProperty(L"freeMem", jsval, JS4D::PropertyAttributeNone);
	result.SetProperty(L"pageCount", stats.fNbPages);
	result.SetProperty(L"objectCount", stats.fNbObjects);
	result.SetProperty(L"smallBlockCount", stats.fNbSmallBlocks);
	result.SetProperty(L"smallFreeBlockCount", stats.fNbSmallBlocksFree);
	result.SetProperty(L"smallUsedBlockCount", stats.fNbSmallBlocksUsed);
	result.SetProperty(L"largeBlockCount", stats.fNbBigBlocks);
	result.SetProperty(L"largeFreeBlockCount", stats.fNbBigBlocksFree);
	result.SetProperty(L"largeUsedBlockCount", stats.fNbBigBlocksUsed);
	result.SetProperty(L"largestBlock", (sLONG8)stats.fBiggestBlock);
	result.SetProperty(L"largestFreeBlock", (sLONG8)stats.fBiggestBlockFree);

	VJSArray objarr(ioParms.GetContextRef());
	for( VMapOfObjectInfo::const_iterator i = stats.fObjectInfo.begin() ; i != stats.fObjectInfo.end() ; ++i)
	{
		const std::type_info* typobj = i->first;
		VStr255 s;
		VSystem::DemangleSymbol( typobj->name(), s);
		VJSObject elem(ioParms.GetContextRef());
		elem.MakeEmpty();
		VJSValue elemval(ioParms.GetContextRef());
		elemval.SetString(s);
		elem.SetProperty("id", elemval, JS4D::PropertyAttributeNone);
		elemval.SetNumber(i->second.fCount);
		elem.SetProperty("count", elemval, JS4D::PropertyAttributeNone);
		elemval.SetNumber(i->second.fTotalSize);
		elem.SetProperty("size", elemval, JS4D::PropertyAttributeNone);
		objarr.PushValue(elem);
	}
	result.SetProperty("objects", objarr, JS4D::PropertyAttributeNone);

	VJSArray blockarr(ioParms.GetContextRef());
	for( VMapOfBlockInfo::const_iterator i = stats.fBlockInfo.begin() ; i != stats.fBlockInfo.end() ; ++i)
	{
		VStr4 s;
		s.FromOsType( i->first);
		VJSObject elem(ioParms.GetContextRef());
		elem.MakeEmpty();
		VJSValue elemval(ioParms.GetContextRef());
		elemval.SetString(s);
		elem.SetProperty("id", elemval, JS4D::PropertyAttributeNone);
		elemval.SetNumber(i->second.fCount);
		elem.SetProperty("count", elemval, JS4D::PropertyAttributeNone);
		elemval.SetNumber(i->second.fTotalSize);
		elem.SetProperty("size", elemval, JS4D::PropertyAttributeNone);
		blockarr.PushValue(elem);
	}
	result.SetProperty("blocks", blockarr, JS4D::PropertyAttributeNone);

	ioParms.ReturnValue(result);
}


void VJSCacheManager::_getGarbageCollectInterval( XBOX::VJSParms_getProperty& ioParms, VDBMgr* inMgr)
{
	ioParms.ReturnNumber(VDBMgr::GetManager()->GetGarbageTask()->GetJSGarbageInterval() / 1000);
}


bool VJSCacheManager::_setGarbageCollectInterval( XBOX::VJSParms_setProperty& ioParms, VDBMgr* inMgr)
{
	const VJSValue& val = ioParms.GetPropertyValue();
	sLONG xvalue = 0;
	if (val.IsNumber())
	{
		val.GetLong(&xvalue);
		xvalue = xvalue * 1000;
		VDBMgr::GetManager()->GetGarbageTask()->SetJSGarbageInterval(xvalue);
		return true;
	}
	else
		return false;
}




void VJSCacheManager::GetDefinition( ClassDefinition& outDefinition)
{
	static inherited::StaticFunction functions[] = 
	{
		{ "getFullInfo", js_callStaticFunction<_getObjects>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ 0, 0, 0}
	};

	static inherited::StaticValue values[] = 
	{
		{ "garbageCollectInterval", js_getProperty<_getGarbageCollectInterval>, js_setProperty<_setGarbageCollectInterval>, JS4D::PropertyAttributeDontDelete },
		{ 0, 0, 0, 0}
	};

	outDefinition.className = "CacheManager";
	outDefinition.initialize = js_initialize<Initialize>;
	outDefinition.finalize = js_finalize<Finalize>;
	outDefinition.staticFunctions = functions;
	outDefinition.staticValues = values;
}



// --------------------------------------------------------------------------------------------------------------------------------------------------------------------



/*

	static functions for global object class

*/
void do_OpenBase(VJSParms_callStaticFunction& ioParms, VJSGlobalObject*)
{
	XBOX::VFile* structfile = ioParms.RetainFileParam( 1);
	XBOX::VFile* datafile = ioParms.RetainFileParam( 2);
	
	if (structfile != NULL)
	{
		if (datafile == NULL)
		{
			XBOX::VString s;
			structfile->GetNameWithoutExtension(s);
			s = s + kDataFileExt;
			XBOX::VFolder* parent = structfile->RetainParentFolder();
			if (parent != NULL)
			{
				datafile = new XBOX::VFile(*parent, s);
			}
		}
		
		if (datafile != NULL)
		{
			CDB4DManager* db4D = XBOX::VComponentManager::RetainComponentOfType<CDB4DManager>();
			XBOX::VError err = XBOX::VE_OK;
			CDB4DBase* newdb = db4D->OpenBase(*structfile, DB4D_Open_WithSeparateIndexSegment | DB4D_Open_As_XML_Definition | DB4D_Open_No_Respart, &err);
			if (err == XBOX::VE_OK)
			{
				CDB4DBaseContext* dbcontext = NULL;
				CDB4DContext* ownercontext = GetDB4DContextFromJSContext( ioParms.GetContext());
				if (ownercontext != NULL)
					dbcontext = ownercontext->RetainDataBaseContext(newdb, true);
					
				newdb->OpenData(*datafile, DB4D_Open_AllDefaultParamaters, dbcontext, &err);
				QuickReleaseRefCountable(dbcontext);
				if (err == XBOX::VE_OK)
				{
					ioParms.ReturnValue(VJSDatabase::CreateInstance(ioParms.GetContextRef(), newdb));
				}
			}
			QuickReleaseRefCountable( newdb);
			XBOX::ReleaseRefCountable( &db4D);
		}
	}
	XBOX::QuickReleaseRefCountable(structfile);
	XBOX::QuickReleaseRefCountable(datafile);
	
}


void do_Open4DBase(VJSParms_callStaticFunction& ioParms, VJSGlobalObject*)
{
	XBOX::VFile* structfile = ioParms.RetainFileParam( 1);
	XBOX::VFile* datafile = ioParms.RetainFileParam( 2);

	if (structfile != NULL)
	{
		if (datafile == NULL)
		{
			XBOX::VString s;
			structfile->GetNameWithoutExtension(s);
			s = s + kDataFileExt;
			XBOX::VFolder* parent = structfile->RetainParentFolder();
			if (parent != NULL)
			{
				datafile = new XBOX::VFile(*parent, s);
			}
		}

		if (datafile != NULL)
		{
			CDB4DManager* db4D = XBOX::VComponentManager::RetainComponentOfType<CDB4DManager>();
			XBOX::VError err = XBOX::VE_OK;
			CDB4DBase* newdb = db4D->OpenBase(*structfile, DB4D_Open_WithSeparateIndexSegment | DB4D_Open_BuildAutoEm, &err);
			if (err == XBOX::VE_OK)
			{
				CDB4DBaseContext* dbcontext = NULL;
				CDB4DContext* ownercontext = GetDB4DContextFromJSContext( ioParms.GetContext());
				if (ownercontext != NULL)
					dbcontext = ownercontext->RetainDataBaseContext(newdb, true);

				newdb->OpenData(*datafile, DB4D_Open_AllDefaultParamaters, dbcontext, &err);
				QuickReleaseRefCountable(dbcontext);
				if (err == XBOX::VE_OK)
				{
					ioParms.ReturnValue(VJSDatabase::CreateInstance(ioParms.GetContextRef(), newdb));
				}
			}
			QuickReleaseRefCountable( newdb);
			XBOX::ReleaseRefCountable( &db4D);
		}
	}
	XBOX::QuickReleaseRefCountable(structfile);
	XBOX::QuickReleaseRefCountable(datafile);

}


void do_CreateBase(VJSParms_callStaticFunction& ioParms, VJSGlobalObject*)
{
	XBOX::VFile* structfile = ioParms.RetainFileParam( 1);
	XBOX::VFile* datafile = ioParms.RetainFileParam( 2);
	
	if (structfile != NULL)
	{
		if (datafile == NULL)
		{
			XBOX::VString s;
			structfile->GetNameWithoutExtension(s);
			s = s + kDataFileExt;
			XBOX::VFolder* parent = structfile->RetainParentFolder();
			if (parent != NULL)
			{
				datafile = new XBOX::VFile(*parent, s);
			}
		}
		
		if (datafile != NULL)
		{
			CDB4DManager* db4D = XBOX::VComponentManager::RetainComponentOfType<CDB4DManager>();
			XBOX::VError err = XBOX::VE_OK;
			CDB4DBase* newdb = db4D->CreateBase(*structfile, DB4D_Create_As_XML_Definition | DB4D_Create_No_Respart | DB4D_Create_WithSeparateIndexSegment, XBOX::VTask::GetCurrentIntlManager(), &err);
			if (err == XBOX::VE_OK)
			{
				CDB4DBaseContext* dbcontext = NULL;
				CDB4DContext* ownercontext = GetDB4DContextFromJSContext( ioParms.GetContext());
				if (ownercontext != NULL)
					dbcontext = ownercontext->RetainDataBaseContext(newdb, true);
				
				newdb->CreateData(*datafile, DB4D_Create_AllDefaultParamaters, NULL, dbcontext, &err);
				QuickReleaseRefCountable(dbcontext);
				if (err == XBOX::VE_OK)
				{
					ioParms.ReturnValue(VJSDatabase::CreateInstance(ioParms.GetContextRef(), newdb));
				}
			}
			QuickReleaseRefCountable(newdb);
			XBOX::ReleaseRefCountable( &db4D);
		}
	}
	XBOX::QuickReleaseRefCountable(structfile);
	XBOX::QuickReleaseRefCountable(datafile);
	
}


void do_GetBase(VJSParms_callStaticFunction& ioParms, VJSGlobalObject*)
{
	XBOX::VString basename;
	if (ioParms.IsNumberParam(1))
	{
		CDB4DManager* db4D = XBOX::VComponentManager::RetainComponentOfType<CDB4DManager>();
		sLONG i = 0;
		ioParms.GetLongParam(1, &i);
		CDB4DBase* db = db4D->RetainNthBase(i);
		if (db != NULL)
		{
			ioParms.ReturnValue(VJSDatabase::CreateInstance(ioParms.GetContextRef(), db));
			db->Release();
		}
		XBOX::ReleaseRefCountable( &db4D);
	}
	else if (ioParms.GetStringParam(1, basename))
	{
		CDB4DManager* db4D = XBOX::VComponentManager::RetainComponentOfType<CDB4DManager>();
		for (sLONG i = 1, nb = db4D->CountBases(); i <= nb; i++)
		{
			CDB4DBase* db = db4D->RetainNthBase(i);
			if (db != NULL)
			{
				XBOX::VString name;
				db->GetName(name);
				if (name == basename)
					ioParms.ReturnValue(VJSDatabase::CreateInstance(ioParms.GetContextRef(), db));
				db->Release();
			}
		}
		XBOX::ReleaseRefCountable( &db4D);
	}
}


void do_OpenRemoteBase(VJSParms_callStaticFunction& ioParms, VJSGlobalObject*)
{
	
}

void do_GetCacheManager(VJSParms_callStaticFunction& ioParms, VJSGlobalObject*)
{
	ioParms.ReturnValue(VJSCacheManager::CreateInstance(ioParms.GetContextRef(), VDBMgr::GetManager()));
}




//======================================================



void CreateGlobalDB4DClasses()
{
	VJSDatabase::Class();
	VJSDatabaseTableEnumerator::Class();
	//VJSSelection::Class();
	//VJSRecord::Class();
	VJSField::Class();
	VJSTable::Class();
//	VJSSelectionIterator::Class();

	VJSGlobalClass::AddStaticFunction( "open4DBase", VJSGlobalClass::js_callStaticFunction<do_Open4DBase>, JS4D::PropertyAttributeNone);
	VJSGlobalClass::AddStaticFunction( "openDataStore", VJSGlobalClass::js_callStaticFunction<do_OpenBase>, JS4D::PropertyAttributeNone);
	VJSGlobalClass::AddStaticFunction( "createDataStore", VJSGlobalClass::js_callStaticFunction<do_CreateBase>, JS4D::PropertyAttributeNone);
	//VJSGlobalClass::AddStaticFunction( "openRemoteBase", VJSGlobalClass::js_callStaticFunction<do_OpenRemoteBase>, JS4D::PropertyAttributeNone); // DataBase : OpenRemoteBase(string : servername, long : portnum);
	VJSGlobalClass::AddStaticFunction( "getDataStore", VJSGlobalClass::js_callStaticFunction<do_GetBase>, JS4D::PropertyAttributeNone);

	VJSGlobalClass::AddStaticFunction( "getCacheManager", VJSGlobalClass::js_callStaticFunction<do_GetCacheManager>, JS4D::PropertyAttributeNone);
}





