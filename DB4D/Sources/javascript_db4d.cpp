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

#include "Backup.h"


USING_TOOLBOX_NAMESPACE
using namespace std;






CDB4DContext *GetDB4DContextFromJSContext( const XBOX::VJSContext& inContext)
{
	return static_cast<CDB4DContext*>( inContext.GetGlobalObjectPrivateInstance()->GetSpecific( 'db4d'));
}


void SetDB4DContextInJSContext( const XBOX::VJSContext& inContext, CDB4DContext* inDB4DContext)
{
	if (inContext.GetGlobalObjectPrivateInstance()->SetSpecific( 'db4d', inDB4DContext, VJSSpecifics::DestructorReleaseIRefCountable))
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
	EntityModel* xem = dynamic_cast<EntityModel*>(em);
	CDB4DBase* base = xem->GetCatalog()->GetOwner()->RetainBaseX();
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



BaseTaskInfo *GetBaseTaskInfoFromJSContext( const XBOX::VJSContext& inContext, Base4D* inBase)
{
	CDB4DContext *db4dContext = GetDB4DContextFromJSContext( inContext);
	if (db4dContext != NULL)
		return dynamic_cast<VDB4DContext*>(db4dContext)->RetainDataBaseContext( inBase, true, false);

	return NULL;
}


BaseTaskInfo *GetBaseTaskInfoFromJSContext( const XBOX::VJSContext& inContext, EntityModel* inModel)
{
	return GetBaseTaskInfoFromJSContext(inContext, inModel->GetCatalog()->GetOwner());
}


BaseTaskInfo *GetBaseTaskInfoFromJSContext( const XBOX::VJSContext& inContext, EntityCollection* inSel)
{
	return GetBaseTaskInfoFromJSContext(inContext, inSel->GetModel());
}


template<class T>
static BaseTaskInfo* GetBaseTaskInfoFromJSContext( const VJSParms_withContext& inParms, T* inParam)
{
	return GetBaseTaskInfoFromJSContext( inParms.GetContext(), inParam);
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

/*
VError getQParams(VJSParms_callStaticFunction& ioParms, sLONG firstparam, QueryParamElementVector& outParams, SearchTab* inQuery)
{
	VError err = VE_OK;
	SearchTab* query = inQuery;
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


VError FillQueryWithParams(SearchTab* query, VJSParms_callStaticFunction& ioParms, sLONG firstparam)
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
*/

sLONG GetAttributeListParams(XBOX::VJSParms_callStaticFunction& ioParms, sLONG StartParam, EntityAttributeSortedSelection& outList)
{
	if (ioParms.IsStringParam(StartParam))
	{
		VString s;
		ioParms.GetStringParam(StartParam, s);
		outList.BuildFromString(s, GetBaseTaskInfoFromJSContext(ioParms, outList.GetModel()), true, false, nil);
		StartParam++;
	}
	else
	{
		while (StartParam <= ioParms.CountParams())
		{
			EntityAttribute* xatt = ioParms.GetParamObjectPrivateData<VJSEntityAttribute>(StartParam);
			if (xatt != nil)
			{
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



EntitySelectionIterator::EntitySelectionIterator(EntityCollection* inSel, bool inReadOnly, bool inAutoSave, BaseTaskInfo* inContext, EntityModel* inModel)
{
	fReadOnly = inReadOnly;
	fAutoSave = inAutoSave;
	fSel = RetainRefCountable(inSel);
	fCurRec = nil;
	fCurPos = -1;
	fSelSize = fSel->GetLength(inContext);
	fModel = RetainRefCountable(inModel);
}


EntitySelectionIterator::EntitySelectionIterator(EntityRecord* inRec, BaseTaskInfo* inContext)
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


EntityRecord* EntitySelectionIterator::GetCurRec(BaseTaskInfo* inContext)
{
	if (fSel != nil)
	{
		if (fCurPos == -2)
		{
			ReleaseCurCurec(true);
			fCurPos = 0;
			if (fCurPos >= fSelSize)
				fCurPos = -1;
			VError err = VE_OK;
			if (fCurPos != -1)
				fCurRec = fSel->LoadEntity(fCurPos, inContext, err, DB4D_Do_Not_Lock);		
		}
	}
	
	return fCurRec;
}


VError EntitySelectionIterator::ReLoadCurRec(BaseTaskInfo* inContext, bool readonly, bool canautosave)
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
			fCurRec = fSel->LoadEntity(fCurPos, inContext, err, DB4D_Do_Not_Lock);		
	}
	else
	{
		if (fCurRec != nil)
		{
			err = fCurRec->Reload();
		}
	}
	
	return err ;	
}

/*
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
*/

void EntitySelectionIterator::ReleaseCurCurec(bool canautosave)
{
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
			
			fCurRec->Release();
			fCurRec = nil;
		}
	}
}


void EntitySelectionIterator::NextNotNull(BaseTaskInfo* inContext)
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

		if (fCurPos != -1)
			fCurPos = fSel->NextNotNull(inContext, fCurPos);
	}
}


void EntitySelectionIterator::First(BaseTaskInfo* inContext)
{
	ReleaseCurCurec(true);
	fCurPos = -2;
	NextNotNull(inContext);
	ReLoadCurRec(inContext, fReadOnly, fAutoSave); 
	
}

void EntitySelectionIterator::Next(BaseTaskInfo* inContext)
{
	if (fSel != nil)
	{
		ReleaseCurCurec(true);
		NextNotNull(inContext);

		{
			if (fCurPos >= fSelSize || fCurPos == -1)
			{
				fCurPos = -1;
			}
			else
			{
				VError err = VE_OK;
				fCurRec = fSel->LoadEntity(fCurPos, inContext, err, DB4D_Do_Not_Lock);			
			}
		}
	}
}




//======================================================


JSCollectionManager::JSCollectionManager(JS4D::ContextRef inContext, bool simpleDate)
{
	fContextRef = inContext;
	fSize = 0;
	fSimpleDate = simpleDate;
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
	outValue = val.CreateVValue(nil, fSimpleDate);
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




void VJSTable::Initialize( const VJSParms_initialize& inParms, Table* inTable)
{
	inTable->Retain();
}


void VJSTable::Finalize( const VJSParms_finalize& inParms, Table* inTable)
{
	inTable->Release();
}



void VJSTable::GetPropertyNames( VJSParms_getPropertyNames& ioParms, Table* inTable)
{
	sLONG nb = inTable->GetNbCrit();
	for (sLONG i = 1; i <= nb; i++)
	{
		VString fieldname;
		Field* ff = inTable->RetainField(i);
		if (ff != nil)
		{
			ff->GetName(fieldname);
			ioParms.AddPropertyName(fieldname);
			ff->Release();
		}
	}
}


void VJSTable::GetProperty( VJSParms_getProperty& ioParms, Table* inTable)
{
	Field* ff = nil;
	
	sLONG num;
	if (ioParms.GetPropertyNameAsLong( &num))
	{
		ff = inTable->RetainField(num);
	}
	else
	{
		VString propname;
		ioParms.GetPropertyName( propname);
		ff = inTable->FindAndRetainFieldRef(propname);
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


void VJSTable::_GetID(VJSParms_callStaticFunction& ioParms, Table* inTable)
{
	ioParms.ReturnNumber(inTable->GetNum());
}


void VJSTable::_GetName(VJSParms_callStaticFunction& ioParms, Table* inTable)
{
	XBOX::VString name;
	inTable->GetName( name);

	ioParms.ReturnString( name);
}


void VJSTable::_SetName(VJSParms_callStaticFunction& ioParms, Table* inTable)
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


void VJSTable::_CountFields(VJSParms_callStaticFunction& ioParms, Table* inTable)
{
	ioParms.ReturnNumber(inTable->GetNbCrit());
}


void VJSTable::_Drop(VJSParms_callStaticFunction& ioParms, Table* inTable)
{
	VError err = inTable->Drop(nil, nil);
}


void VJSTable::_keepSyncInfo(VJSParms_callStaticFunction& ioParms, Table* inTable)
{
	bool keepinfo;
	VError err = VE_OK;
	if (ioParms.GetBoolParam(1, &keepinfo))
	{
		err = inTable->SetKeepRecordSyncInfo(nil, keepinfo, nil);
	}
}


void VJSTable::_dropPrimaryKey(VJSParms_callStaticFunction& ioParms, Table* inTable)
{
	NumFieldArray empty;
	VError err = inTable->SetPrimaryKey(empty, nil, false, nil, nil);
}


void VJSTable::_setPrimaryKey(VJSParms_callStaticFunction& ioParms, Table* inTable)
{
	NumFieldArray fields;
	XBOX::VString s = L"PK"+XBOX::ToString(inTable->GetNum());

	sLONG nbparam = (sLONG) ioParms.CountParams();
	for (sLONG i = 1; i <= nbparam; i++)
	{
		Field* field = ioParms.GetParamObjectPrivateData<VJSField>(i);
		if (field != nil)
		{
			fields.Add(field->GetPosInRec());
		}
	}

	VError err = inTable->SetPrimaryKey(fields, nil, false, nil, &s);
}


void VJSTable::_CreateField(VJSParms_callStaticFunction& ioParms, Table* inTable)
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
				
			//VError err = inTable->AddField(fieldname, fieldtype, fieldsize, attribute);
		}
	}
	
}


void VJSTable::_setAutoSeqValue(XBOX::VJSParms_callStaticFunction& ioParms, Table* inTable)
{
	Real val;
	if (ioParms.GetRealParam(1, &val))
	{
		AutoSeqNumber* autoseq = inTable->GetSeqNum(nil);
		if (autoseq != nil)
		{
			autoseq->SetCurrentValue((sLONG8)val);
			//autoseq->Release();
		}
	}
}


void VJSTable::_isEntityModel(VJSParms_callStaticFunction& ioParms, Table* inTable)
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

VJSObject VJSBackupSettings::CreateInstance( JS4D::ContextRef inContext, IBackupSettings *inRetainedBackupSettings)
{
	return VJSObject( inContext, inherited::CreateInstance( inContext, inRetainedBackupSettings));
}

void VJSBackupSettings::Initialize( const XBOX::VJSParms_initialize& inParms, IBackupSettings* inSettings)
{
	if (inSettings)
		inSettings->Retain();
}

void VJSBackupSettings::Finalize( const XBOX::VJSParms_finalize& inParms, IBackupSettings* inSettings)
{
	if (inSettings)
		inSettings->Release();
}

void VJSBackupSettings::GetDefinition( ClassDefinition& outDefinition)
{
	static inherited::StaticFunction functions[] = 
	{
		{ "setDestination", js_callStaticFunction<_setDestination>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "isValid", js_callStaticFunction<_isValid>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ 0, 0, 0}
	};

	static inherited::StaticValue values[] =
	{
		{ "destination", js_getProperty<_getDestination>, NULL,JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontDelete },
		{ 0, 0, 0, 0}
	};

	outDefinition.className = "BackupSettings";
	outDefinition.callAsConstructor = js_callAsConstructor<CallAsConstructor>;
	outDefinition.initialize = js_initialize<Initialize>;
	outDefinition.finalize = js_finalize<Finalize>;
	outDefinition.staticFunctions = functions;
	outDefinition.staticValues = values;
}

void	VJSBackupSettings::CallAsConstructor(VJSParms_callAsConstructor& ioParms)
{
	bool constructed = false;
	VDBMgr *mgr = VDBMgr::GetManager();
	if(mgr)
	{
		IBackupSettings* settings = mgr->CreateBackupSettings();
		if(settings)
		{
			ioParms.ReturnConstructedObject(VJSBackupSettings::CreateInstance(ioParms.GetContext(),settings));
			constructed = true;
		}
		ReleaseRefCountable(&settings);
	}
	if(!constructed)
		ioParms.ReturnUndefined();
}

void	VJSBackupSettings::_getDestination(XBOX::VJSParms_getProperty& ioParms, IBackupSettings* inBackupSettings)
{
	VFilePath path;
	bool valid = false;
	valid = inBackupSettings->GetDestination(path);
	if (valid)
	{
		VString pathStr;
		path.GetPosixPath(pathStr);
		path.FromFullPath(pathStr,FPS_POSIX);
		ioParms.ReturnFilePathAsFileOrFolder(path);
	}
	else
	{
		ioParms.ReturnNullValue();
	}
}

void	VJSBackupSettings::_setDestination(XBOX::VJSParms_callStaticFunction& ioParms, IBackupSettings* inBackupSettings)
{
	//TODO: inspect parameter and check if it is:
	// - a string
	// - a javascript object e.g. Folder
	// - something else
	// - if none of these apply then throw

	VFilePath path;
	VString pathString;
	if (ioParms.CountParams() == 0)
	{
		vThrowError(VE_INVALID_PARAMETER);
		ioParms.ReturnNullValue();
	}
	else if (ioParms.IsStringParam(1))
	{
		ioParms.GetStringParam(1,pathString);
		if (!pathString.BeginsWith( CVSTR( "./")) && !pathString.BeginsWith( CVSTR( "../")))
		{
			path.FromFullPath(pathString,FPS_POSIX);
			if (path.IsEmpty() || !path.IsValid())
			{
				VString part;
				VURL url(pathString,true);

				path.Clear();
				url.GetScheme(part);
				part.ToLowerCase();
				//Ensure we're dealing with a file:// URL
				if(part == CVSTR("file"))
				{
					//Ensure we're dealing with a localhost/127.0.0.1 or empty host address
					url.GetHostName(part,false);
					part.ToLowerCase();
					if (part.GetLength() == 0 || part == CVSTR("127.0.0.1") || part == CVSTR("localhost"))
					{
						//Ensure we clear the network location part so that VUrl retrieves
						//the filepath with no host part
						url.SetNetWorkLocation(CVSTR(""),false);
						if (!url.GetFilePath(path))
						{
							path.Clear();
						}
					}
				}
			}
		}
	}
	else //Assume object so a Folder or a URI
	{
		VFolder *folder = NULL;
		VJSValue value = ioParms.GetParamValue(1);
		folder = value.GetFolder();
		if (folder)
		{
			folder->GetPath(path);
			folder = NULL;//obtained from GetFolder() so no need to release
		}
		else
		{
			value.GetString( pathString);
			if (!pathString.BeginsWith( CVSTR( "./")) && !pathString.BeginsWith( CVSTR( "../")))
			{
				VURL url;
				VString part;
				url.GetScheme(part);
				part.ToLowerCase();
				if(part == CVSTR("file"))
				{
					//accept only localhost or 127.0.0.1
					url.GetHostName(part,false);
					part.ToLowerCase();
					if (part.GetLength() == 0 || part == CVSTR("127.0.0.1") || part == CVSTR("localhost"))
					{
						//Ensure we clear the network location part so that VUrl retrieves
						//the filepath with no host part
						url.SetNetWorkLocation(CVSTR(""),false);
						if (value.GetURL( url))
						{
							if (!url.GetFilePath(path))
							{
								path.Clear();
							}
						}
					}
				}
			}
		}
	}

	if (path.IsEmpty() || !path.IsFolder() || !path.IsValid())
	{
		vThrowError(VE_INVALID_PARAMETER);
	}
	else
	{
		inBackupSettings->SetDestination(path);
	}
	ioParms.ReturnNullValue();
}

void	VJSBackupSettings::_isValid(XBOX::VJSParms_callStaticFunction& ioParms, IBackupSettings* inBackupSettings)
{
	VError err = VE_OK;
	if (inBackupSettings)
		err = inBackupSettings->CheckValidity();
	ioParms.ReturnBool(err == VE_OK);
}

//======================================================



void VJSField::Initialize( const VJSParms_initialize& inParms, Field* inField)
{
	inField->Retain();
}


void VJSField::Finalize( const VJSParms_finalize& inParms, Field* inField)
{
	inField->Release();
}


void VJSField::_GetName(VJSParms_callStaticFunction& ioParms, Field* inField)
{
	VString name;
	inField->GetName( name);

	ioParms.ReturnString( name);
}


void VJSField::_SetName(VJSParms_callStaticFunction& ioParms, Field* inField)
{
	VString name;

	if (ioParms.CountParams() > 0)
	{
		if (ioParms.GetStringParam(1, name))
		{
			VError err = inField->SetName(name, nil);
		}
	}

}


void VJSField::_Drop(VJSParms_callStaticFunction& ioParms, Field* inField)
{
	//VError err = inField->Drop(nil, nil);
}



void VJSField::_CreateIndex(VJSParms_callStaticFunction& ioParms, Field* inField)
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
		//VError err = inField->GetOwner()->GetOwner()->CreateIndexOnOneField(inField, indtype, false, nil, &indexname, nil, true, indexevent);
		indexevent->Lock();
		indexevent->Release();
	}
}


void VJSField::_DropIndex(VJSParms_callStaticFunction& ioParms, Field* inField)
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
	//VError err = inField->GetOwner()->GetOwner()->DropIndexOnOneField(inField, indtype, nil, indexevent);
	indexevent->Lock();
	indexevent->Release();
}


void VJSField::_Create_FullText_Index(VJSParms_callStaticFunction& ioParms, Field* inField)
{
	VString indexname;
	ioParms.GetStringParam(1, indexname);
	VSyncEvent* indexevent = new VSyncEvent();
	//VError err = inField->GetOwner()->GetOwner()->CreateFullTextIndexOnOneField(inField, nil, &indexname, nil, true, indexevent);
	indexevent->Lock();
	indexevent->Release();
	
}


void VJSField::_Drop_FullText_Index(VJSParms_callStaticFunction& ioParms, Field* inField)
{
	VSyncEvent* indexevent = new VSyncEvent();
//	VError err = inField->GetOwner()->GetOwner()->DropFullTextIndexOnOneField(inField, nil, indexevent);
	//indexevent->Lock();
	indexevent->Release();
}


void VJSField::_SetType(VJSParms_callStaticFunction& ioParms, Field* inField)
{
	if (ioParms.CountParams() > 0)
	{
		sLONG fieldtype;
		if (ioParms.GetLongParam(1, &fieldtype))
		{
			//VError err = inField->SetType(fieldtype, nil, nil);
		}
	}
}


void VJSField::_GetType(VJSParms_callStaticFunction& ioParms, Field* inField)
{
	ioParms.ReturnNumber(inField->GetTyp());
}


void VJSField::_GetID(VJSParms_callStaticFunction& ioParms, Field* inField)
{
	ioParms.ReturnNumber(inField->GetPosInRec());
}


void VJSField::_IsIndexed(VJSParms_callStaticFunction& ioParms, Field* inField)
{
	ioParms.ReturnBool(inField->IsIndexed());
}


void VJSField::_IsUnique(VJSParms_callStaticFunction& ioParms, Field* inField)
{
	ioParms.ReturnBool(inField->GetUnique());
}


void VJSField::_Is_FullText_Indexed(VJSParms_callStaticFunction& ioParms, Field* inField)
{
	ioParms.ReturnBool(inField->IsFullTextIndexed());
}


void VJSField::_IsNotNull(VJSParms_callStaticFunction& ioParms, Field* inField)
{
	ioParms.ReturnBool(inField->GetNot_Null());
}


void VJSField::_IsAutoIncrement(VJSParms_callStaticFunction& ioParms, Field* inField)
{
	ioParms.ReturnBool(inField->GetAutoSeq());
}


void VJSField::_SetNotNull(VJSParms_callStaticFunction& ioParms, Field* inField)
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


void VJSField::_SetAutoIncrement(VJSParms_callStaticFunction& ioParms, Field* inField)
{
	if (ioParms.CountParams() > 0)
	{
		bool b;
		if (ioParms.GetBoolParam(1, &b))
		{
			inField->SetAutoSeq(b, nil);
		}
	}
}


void VJSField::_SetUnique(VJSParms_callStaticFunction& ioParms, Field* inField)
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


void VJSField::_SetAutoGenerate(VJSParms_callStaticFunction& ioParms, Field* inField)
{
	if (ioParms.CountParams() > 0)
	{
		bool b;
		if (ioParms.GetBoolParam(1, &b))
		{
			inField->SetAutoGenerate(b, nil);
		}
	}
}


void VJSField::_SetStoredAsUUID(VJSParms_callStaticFunction& ioParms, Field* inField)
{
	if (ioParms.CountParams() > 0)
	{
		bool b;
		if (ioParms.GetBoolParam(1, &b))
		{
			//VError err = inField->set(b, nil);
		}
	}
}


void VJSField::_IsAutoGenerate(VJSParms_callStaticFunction& ioParms, Field* inField)
{
	ioParms.ReturnBool(inField->GetAutoGenerate());
}


void VJSField::_IsStoredAsUUID(VJSParms_callStaticFunction& ioParms, Field* inField)
{
	//ioParms.ReturnBool(inField->IsStoreAsUUID(nil));
}



void VJSField::_isStoredAsUTF8(VJSParms_callStaticFunction& ioParms, Field* inField)
{
	ioParms.ReturnBool(inField->GetStoreUTF8());
}


void VJSField::_SetStoredAsUTF8(VJSParms_callStaticFunction& ioParms, Field* inField)
{
	if (ioParms.CountParams() > 0)
	{
		bool b;
		if (ioParms.GetBoolParam(1, &b))
		{
			inField->SetStoreUTF8(b, nil);
		}
	}
}


void VJSField::_isStoredOutside(VJSParms_callStaticFunction& ioParms, Field* inField)
{
//	ioParms.ReturnBool(inField->IsStoreOutside(nil));
}


void VJSField::_setStoreOutside(VJSParms_callStaticFunction& ioParms, Field* inField)
{
	if (ioParms.CountParams() > 0)
	{
		bool b;
		if (ioParms.GetBoolParam(1, &b))
		{
			//VError err = inField->SetStoreOutside(b, nil, nil);
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


void VJSDatabase::Initialize( const VJSParms_initialize& inParms, Base4D* inDatabase)
{
	inDatabase->Retain();
}


void VJSDatabase::Finalize( const VJSParms_finalize& inParms, Base4D* inDatabase)
{
	inDatabase->Release();
}


VJSObject VJSDatabase::CreateJSEMObject( const VString& emName, const VJSContext& inContext, BaseTaskInfo *inBaseContext)
{
	Base4D* inDatabase = inBaseContext->GetBase();
	EntityModel* em = inDatabase->RetainEntity(emName, false);
	VJSObject result(inContext);
	if (em != nil)
	{
		EntityModel* xem = em;
		if (xem->publishAsGlobal(nil))
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


void VJSDatabase::PutAllModelsInGlobalObject(VJSObject& globalObject, Base4D* inDatabase, BaseTaskInfo* context)
{
	vector<VRefPtr<EntityModel> > entities;
	inDatabase->RetainAllEntityModels(entities, false);
	for (vector<VRefPtr<EntityModel> >::iterator cur = entities.begin(), end = entities.end(); cur != end; cur++)
	{
		EntityModel* em = *cur;
		if (em->publishAsGlobal(context))
		{
			VJSValue emVal = VJSEntityModel::CreateInstance(globalObject.GetContextRef(), em);
			globalObject.SetProperty(em->GetEntityName(), emVal);
		}
	}
}


void VJSDatabase::GetPropertyNames( VJSParms_getPropertyNames& ioParms, Base4D* inDatabase)
{
	//set<VString> dejaName;
	//vector<EntityModel*> entities;
	vector<VRefPtr<EntityModel> > entities;
	inDatabase->RetainAllEntityModels(entities, false);
	for (vector<VRefPtr<EntityModel> >::iterator cur = entities.begin(), end = entities.end(); cur != end; cur++)
	{
		EntityModel* em = *cur;
		ioParms.AddPropertyName(em->GetEntityName());
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


void VJSDatabase::GetProperty( VJSParms_getProperty& ioParms, Base4D* inDatabase)
{
	EntityModel* em = nil;
	
	VString propname;
	{
		ioParms.GetPropertyName(propname);
		EntityModel* em = inDatabase->RetainEntity(propname, false);
		if (em != nil)
		{
			ioParms.ReturnValue(VJSEntityModel::CreateInstance(ioParms.GetContextRef(), em));
			em->Release();
		}
		else
		{
			/*
			EntityModel* em = inDatabase->RetainEntityModelBySingleEntityName(propname);
			if (em != nil)
			{
				ioParms.ReturnValue(VJSEntityModel::CreateInstance(ioParms.GetContextRef(), em));
				em->Release();
			}
			*/
		}
	}
	
}


void VJSDatabase::_setSortMaxMem(XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase)
{
	CDB4DManager* db4D = CDB4DManager::RetainManager();
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


void VJSDatabase::_setCacheSize(XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase)
{
	CDB4DManager* db4D = CDB4DManager::RetainManager();
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

void VJSDatabase::_getCacheSize(XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase)
{
	CDB4DManager* db4D = CDB4DManager::RetainManager();
	if (db4D != nil)
	{
		ioParms.ReturnNumber(db4D->GetCacheSize());
		db4D->Release();
	}
}


void VJSDatabase::_getModelDefinition(XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase)
{
	VError err = VE_OK;
	VValueBag* outBag = new VValueBag();;
	EntityModelCatalog* catalog = inDatabase->GetGoodEntityCatalog(true);
	if (catalog != nil)
	{
		err = catalog->SaveEntityModels(*outBag, true, true);
		if (err == VE_OK)
		{
			VString s;
			err = outBag->GetJSONString(s);
			if (err == VE_OK)
			{
				VJSJSON json(ioParms.GetContextRef());
				ioParms.ReturnValue(json.Parse(s));
			}
		}
	}
	QuickReleaseRefCountable(outBag);	
}


void VJSDatabase::_loadModelsDefinition(XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase)
{
	VFile* file = ioParms.RetainFileParam(1);
	inDatabase->ReLoadEntityModels(file); // file peut etre null, c'est permis
	QuickReleaseRefCountable(file);
}


void VJSDatabase::_GetName(VJSParms_callStaticFunction& ioParms, Base4D* inDatabase)
{
	VString name;
	inDatabase->GetName( name);
	
	ioParms.ReturnString( name);
}


void VJSDatabase::_SetName(VJSParms_callStaticFunction& ioParms, Base4D* inDatabase)
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


void VJSDatabase::_CountTables(VJSParms_callStaticFunction& ioParms, Base4D* inDatabase)
{
	ioParms.ReturnNumber(inDatabase->GetNBTable());
}


void VJSDatabase::_GetPath(VJSParms_callStaticFunction& ioParms, Base4D* inDatabase)
{
	VString pathname;
	XBOX::VFilePath path;
	inDatabase->GetDataSegPath(1, path);
	path.GetPath(pathname);
	ioParms.ReturnString(pathname);
}


void VJSDatabase::_CreateTable(VJSParms_callStaticFunction& ioParms, Base4D* inDatabase)
{
#if 0
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
#endif
}


void VJSDatabase::_CreateIndex(VJSParms_callStaticFunction& ioParms, Base4D* inDatabase)
{
}


void VJSDatabase::_DropIndex(VJSParms_callStaticFunction& ioParms, Base4D* inDatabase)
{
}


void VJSDatabase::_StartTransaction(VJSParms_callStaticFunction& ioParms, Base4D* inDatabase)
{
	BaseTaskInfo* context = GetBaseTaskInfoFromJSContext(ioParms, inDatabase);
	if (context != nil)
	{
		VError err = context->StartTransaction();
	}
}


void VJSDatabase::_Commit(VJSParms_callStaticFunction& ioParms, Base4D* inDatabase)
{
	BaseTaskInfo* context = GetBaseTaskInfoFromJSContext(ioParms, inDatabase);
	if (context != nil)
	{
		VError err = context->CommitTransaction();
	}
}


void VJSDatabase::_RollBack(VJSParms_callStaticFunction& ioParms, Base4D* inDatabase)
{
	BaseTaskInfo* context = GetBaseTaskInfoFromJSContext(ioParms, inDatabase);
	if (context != nil)
	{
		VError err = context->RollBackTransaction();
	}
}


void VJSDatabase::_TransactionLevel(VJSParms_callStaticFunction& ioParms, Base4D* inDatabase)
{
	BaseTaskInfo* context = GetBaseTaskInfoFromJSContext(ioParms, inDatabase);
	if (context != nil)
	{
		ioParms.ReturnNumber(context->CurrentTransactionLevel());
	}
}


void VJSDatabase::_GetStructure(VJSParms_callStaticFunction& ioParms, Base4D* inDatabase)
{
	/*
	Base4D* structure = inDatabase->RetainStructDatabase(nil);
	if (structure != nil)
	{
		ioParms.ReturnValue(VJSDatabase::CreateInstance(ioParms.GetContextRef(), structure));
		structure->Release();
	}
	*/
}


void VJSDatabase::_GetSyncInfo(VJSParms_callStaticFunction& ioParms, Base4D* inDatabase)
{
	/*
	Base4D* structure = inDatabase->RetainSyncDataBase();
	if (structure != nil)
	{
		ioParms.ReturnValue(VJSDatabase::CreateInstance(ioParms.GetContextRef(), structure));
		structure->Release();
	}
	*/
}


void VJSDatabase::_FlushCache(VJSParms_callStaticFunction& ioParms, Base4D* inDatabase)
{
	bool waitUntilDone = false;
	ioParms.GetBoolParam(1, &waitUntilDone);
	VDBMgr::GetManager()->FlushCache(inDatabase, waitUntilDone);
	//inDatabase->Flush(waitUntilDone);
}


void VJSDatabase::_ExportAsSQL(VJSParms_callStaticFunction& ioParms, Base4D* inDatabase)
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
		err = inDatabase->ExportToSQL( GetBaseTaskInfoFromJSContext(ioParms, inDatabase), folder, nil, options);
		folder->Release();
	}
}


void VJSDatabase::_clearErrs(VJSParms_callStaticFunction& ioParms, Base4D* inDatabase)
{
	XBOX::VTask::GetCurrent()->FlushErrors();
}


void VJSDatabase::_getTables( XBOX::VJSParms_getProperty& ioParms, Base4D* inDatabase)
{
	ioParms.ReturnValue(VJSDatabaseTableEnumerator::CreateInstance(ioParms.GetContextRef(), inDatabase));

}


void VJSDatabase::_getEntityModels( XBOX::VJSParms_getProperty& ioParms, Base4D* inDatabase)
{
	ioParms.ReturnValue(VJSDatabaseEMEnumerator::CreateInstance(ioParms.GetContextRef(), inDatabase));

}


void VJSDatabase::_getTempFolder( XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase)
{
	VFolder* ff = inDatabase->RetainTemporaryFolder(true);
	ioParms.ReturnFolder( ff);
	ReleaseRefCountable( &ff);
}



void VJSDatabase::_getDataFolder( XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase)
{
	VFolder* ff = inDatabase->RetainDataFolder();
	ioParms.ReturnFolder( ff);
	ReleaseRefCountable( &ff);
}


void VJSDatabase::_getCatalogFolder( XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase)
{
	VFolder* ff = RetainRefCountable(inDatabase->GetStructFolder());
	ioParms.ReturnFolder( ff);
	ReleaseRefCountable( &ff);
}


void VJSDatabase::_getCacheInfo( XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase)
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


void VJSDatabase::_getDBList( XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase)
{
	/*
	VJSArray arr(ioParms.GetContextRef());
	for (sLONG i = 1, nb = VDBMgr::GetManager()->CountBases(); i <= nb; i++)
	{
		Base4D* base = VDBMgr::GetManager()->RetainNthBase(i);
		if (base != nil)
		{
			arr.PushValue(VJSDatabase::CreateInstance(ioParms.GetContextRef(), base));
			QuickReleaseRefCountable( base);
		}
	}
	ioParms.ReturnValue(arr);
	*/
}


void VJSDatabase::_freeCacheMem( XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase)
{
	VDBMgr::GetManager()->GetCacheManager()->GetMemoryManager()->PurgeMem();
}


void VJSDatabase::_getIndices( XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase)
{
	VJSArray arr(ioParms.GetContextRef());
	inDatabase->GetIndices(arr);
	ioParms.ReturnValue(arr);
}


void VJSDatabase::_fixForV4(XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase)
{
	inDatabase->_fixForWakandaV4();
}


void VJSDatabase::_close( XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase)
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
				inDatabase->UnRegisterForLang();
				inDatabase->BeginClose(waitclose);
				VDBMgr::GetManager()->UnRegisterBase(inDatabase);
				ioParms.ReturnValue(VJSSyncEvent::CreateInstance(ioParms.GetContextRef(), sync));
				sync->Release();
			}
		}

	}

	if (simpleClose)
	{
		inDatabase->UnRegisterForLang();
		inDatabase->BeginClose();
		VDBMgr::GetManager()->UnRegisterBase(inDatabase);
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


void	VJSDatabase::_GetJournalFile(XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase)
{
	VError error = VE_OK;
	XBOX::VFile* journalFile = NULL;

	//context = GetBaseTaskInfoFromJSContext(ioParms, inDatabase);
	
	journalFile = inDatabase->RetainJournalFile();

	if (journalFile)
	{
		ioParms.ReturnFile(journalFile);
	}
	else
	{
		ioParms.ReturnNullValue();
	}
	XBOX::ReleaseRefCountable(&journalFile);
}

void	VJSDatabase::_IsJournalEnabled(XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase)
{
	bool journalEnabled = false;
	XBOX::VFile* journalFile = NULL;

	//context = GetBaseTaskInfoFromJSContext(ioParms, inDatabase);
	
	journalFile = inDatabase->RetainJournalFile();
	journalEnabled = (journalFile != NULL);
	XBOX::ReleaseRefCountable(&journalFile);

	ioParms.ReturnBool(journalEnabled);
}

void	VJSDatabase::_DisableJournal(XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase)
{
	Base4D* base = NULL;
	VError error = VE_OK;

	BaseTaskInfo* context = GetBaseTaskInfoFromJSContext(ioParms, inDatabase);

	error = JournalUtils::DisableJournal(inDatabase, context->GetEncapsuleur());
	ioParms.ReturnBool(error == VE_OK);
}

void	VJSDatabase::_GetBackupSettings(XBOX::VJSParms_callStaticFunction& ioParms, Base4D* base)
{
	StErrorContextInstaller errorContext;
	bool done =false;
	CDB4DBase* inDatabase = base->RetainBaseX();

	if (inDatabase)
	{
		const IBackupSettings* settings =  inDatabase->RetainBackupSettings();
		if(settings)
		{
			VJSObject settingsObj(ioParms.GetContextRef());
			settingsObj.MakeEmpty();
			settings->ToJSObject(settingsObj);
			ioParms.ReturnValue(settingsObj);
			done = true;
		}
	}
	if(!done)
		ioParms.ReturnNullValue();

	QuickReleaseRefCountable(inDatabase);
}

/**
 * Command syntax:
 * ds.backupAndChangeJournal(newJournalFile: object [,options: object])
 * examples:
 * <code>
 * var newJournal = new File(xxx); 
 * ds.backupAndChangeJournal(newJournal);                 // specifies a new journal and no progress option
 * ds.backupAndChangeJournal(null,{some options});		  // fails you have to specify a journal 
 * ds.backupAndChangeJournal('some path',{some options}); // fails you have to specify a journal as a File object
 */
void	VJSDatabase::_BackupAndChangeJournal(XBOX::VJSParms_callStaticFunction& ioParms, Base4D* base)
{
	StErrorContextInstaller errorContext;
	CDB4DBaseContext* context = NULL;
	XBOX::VFilePath manifestPath;
	XBOX::VError error = VE_OK;

	CDB4DBase* inDatabase = base->RetainBaseX();
	context = GetDB4DBaseContextFromJSContext(ioParms, inDatabase);
		
	//Retrieve progress callbacks from options if applicable
	VJSObject jsBackupProgressObs(ioParms.GetContextRef());
	XBOX::VFilePath journalPath;
	if (ioParms.IsObjectParam(2))
	{
		ioParms.GetParamObject(2,jsBackupProgressObs);
	}
	else
	{
		jsBackupProgressObs.MakeEmpty();
	}
	JSTOOLSIntf backupToolInterface(ioParms.GetContextRef(), jsBackupProgressObs);

	//Ensure a valid journal file is specified
	if (!ioParms.IsNullParam(1) && ioParms.IsObjectParam(1))
	{
		VFile*	journalFile = NULL;
		journalFile = ioParms.RetainFileParam(1);
		if (journalFile )
		{
			journalFile->GetPath(journalPath);
		}
		XBOX::ReleaseRefCountable(&journalFile);
	}

	bool backupSucceeded = false;
	if (!journalPath.IsFile() || !journalPath.IsValid() || journalPath.IsEmpty() )
	{
		backupSucceeded = false;
		error = VE_INVALID_PARAMETER;
		vThrowError(error,CVSTR("Invalid journal file path"));
	}
	else
	{
		IBackupTool* backupTool = NULL;
		const IBackupSettings* backupSettings = NULL;
		backupSettings = inDatabase->RetainBackupSettings();
		backupTool = VDBMgr::CreateBackupTool();

		if (journalPath.IsFile() && journalPath.IsValid() && !journalPath.IsEmpty() )
		{
			backupSucceeded = backupTool->BackupDatabaseAndChangeJournal(inDatabase,context,*backupSettings,&journalPath,&manifestPath,&backupToolInterface);
		}
	
		delete backupTool;backupTool = NULL;
		XBOX::ReleaseRefCountable(&backupSettings);
	}
	
	if (!backupSucceeded)
	{
		error =	errorContext.GetLastError();
	}
	
	if (error != VE_OK || !backupSucceeded)
	{
		VValueBag errorBag;
		VErrorBase* errorBase = NULL;
		errorBase = errorContext.GetContext()->GetLast();
		errorBag.SetLong(L"ErrorLevel",2);//normal error
		if(errorBase)
		{
			VString errorString,temp;
			errorBase->GetErrorString(errorString);
			temp.Clear();
			errorBase->GetLocation(temp);
			if(temp.GetLength() > 0)
			{
				errorString.AppendCString(", ");
				errorString.AppendString(temp);
			}
			temp.Clear();
			errorBase->GetErrorDescription(temp);
			if(temp.GetLength() > 0)
			{
				errorString.AppendCString(", ");
				errorString.AppendString(temp);
			}
			temp.Clear();
			errorBase->GetActionDescription(temp);
			if(temp.GetLength() > 0)
			{
				errorString.AppendCString(", ");
				errorString.AppendString(temp);
			}
			errorBag.SetString(L"ErrorText",errorString);
		}
		else
		{
			errorBag.SetString(L"ErrorText",CVSTR("Backup failed"));
			errorBag.SetLong(L"ErrorNumber",ERRCODE_FROM_VERROR(error));
		}
		backupToolInterface.AddProblem(errorBag);
		ioParms.ReturnNullValue();
	}
	else
	{
		ioParms.ReturnFilePathAsFileOrFolder(manifestPath);
	}
	
	QuickReleaseRefCountable(inDatabase);
}

/**
 * \brief Backs up an active/opened database
 * \details
 * Command syntax:
 * ds.backup([config: object] [,options: object])
 * examples:
 * <code>
 * ds.backup(null,{some progress object});                         -> uses default backup config from database and a progress indicator
 * ds.backup({destination:Folder(SomePath),useUniqueNames:true});  -> uses custom config and no progress options
 * ds.backup();                                                    -> uses default backup config from database and no progress options
 * </code>
 */

void	VJSDatabase::_Backup(XBOX::VJSParms_callStaticFunction& ioParms, Base4D* base)
{
	StErrorContextInstaller errorContext;

	CDB4DBaseContext* context = NULL;
	VError error = VE_OK;
	VJSObject jsBackupProgressObs(ioParms.GetContextRef());
	IBackupSettings* workingBackupSettings = NULL;

	CDB4DBase* inDatabase = base->RetainBaseX();
	context = GetDB4DBaseContextFromJSContext(ioParms, inDatabase);
	
	//Command syntax:
	//ds.backup([config: object] [,options: object])
	//examples:
	//ds.backup(null,{some progress object});                         -> uses default backup config from database and a progress indicator
	//ds.backup({destination:Folder(SomePath),useUniqueNames:true});  -> uses custom config and no progress options
	//ds.backup();                                                    -> uses default backup config from database and no progress options

	//Obtain backup progress callbacks from options parameter if applicable
	if (!ioParms.IsNullParam(2) && ioParms.IsObjectParam(2))
	{
		ioParms.GetParamObject(2,jsBackupProgressObs);
	}
	else
	{
		jsBackupProgressObs.MakeEmpty();
	}
	JSTOOLSIntf backupToolInterface(ioParms.GetContextRef(), jsBackupProgressObs);

	//Check if param 1 is there because it can be null if param 2 is defined
	if (!ioParms.IsNullParam(1) && ioParms.IsObjectParam(1))
	{
		//First parameter is specified as JS object, parse it as backup settings
		const IBackupSettings* parentSettings = inDatabase->RetainBackupSettings();

		VJSObject altBackupConfig(ioParms.GetContextRef());
		ioParms.GetParamObject(1,altBackupConfig);

		//Create working settings and chain them to database settings
		//We need to do this before initializing workingBackupSettings because
		//for max flexibility, altBackupConfig may contain a subset of backup parameters (e.g. destination folder only)
		//so for those undefined parameters we must refer to the parent config
		workingBackupSettings = VDBMgr::CreateBackupSettings();
		workingBackupSettings->SetRetainedParent(parentSettings);

		bool configIsValid = workingBackupSettings->FromJSObject(altBackupConfig);

		if(!configIsValid)
		{
			//The new configuration is not valid so clear it we'll use a default one
			XBOX::ReleaseRefCountable(&workingBackupSettings);
			error = VE_INVALID_PARAMETER;
			vThrowError(error,CVSTR("backup configuration is not valid"));
		}
		XBOX::ReleaseRefCountable(&parentSettings);
	}

	bool backupSucceeded = false;
	VFilePath manifestPath;
	
	if (errorContext.GetLastError() == VE_OK)
	{
		//If no settings were specified then use default ones
		if (workingBackupSettings == NULL)
		{
			workingBackupSettings = const_cast<IBackupSettings*>(inDatabase->RetainBackupSettings());
		}
		if (testAssert(workingBackupSettings != NULL))
		{
			//Now backup takes place
			IBackupTool* backupTool = NULL;
			backupTool = VDBMgr::CreateBackupTool();
			backupSucceeded = backupTool->BackupDatabase(inDatabase,context,*workingBackupSettings,&manifestPath,&backupToolInterface);
			delete backupTool;
			backupTool = NULL;
		}
		else
		{
			error = VE_INVALID_PARAMETER;
			vThrowError(error,CVSTR("no backup configuration defined"));
		}
	}
	XBOX::ReleaseRefCountable(&workingBackupSettings);
	


	//Backup error processing
	if (!backupSucceeded || errorContext.GetLastError() != VE_OK)
	{
		VValueBag errorBag;
		VErrorBase* errorBase = NULL;
		error =	errorContext.GetLastError();
		errorBase = errorContext.GetContext()->GetLast();
		errorBag.SetLong(L"ErrorLevel",2);//normal error
		if(errorBase)
		{
			VString errorString,temp;
			errorBase->GetErrorString(errorString);
			temp.Clear();
			errorBase->GetLocation(temp);
			if(temp.GetLength() > 0)
			{
				errorString.AppendCString(", ");
				errorString.AppendString(temp);
			}
			temp.Clear();
			errorBase->GetErrorDescription(temp);
			if(temp.GetLength() > 0)
			{
				errorString.AppendCString(", ");
				errorString.AppendString(temp);
			}
			temp.Clear();
			errorBase->GetActionDescription(temp);
			if(temp.GetLength() > 0)
			{
				errorString.AppendCString(", ");
				errorString.AppendString(temp);
			}
			errorBag.SetString(L"ErrorText",errorString);
		}
		else
		{
			errorBag.SetString(L"ErrorText",CVSTR("Backup failed"));
			errorBag.SetLong(L"ErrorNumber",ERRCODE_FROM_VERROR(error));
		}
		backupToolInterface.AddProblem(errorBag);
	}

	if (error == VE_OK)
	{
		xbox_assert(!manifestPath.IsEmpty() && manifestPath.IsValid() && manifestPath.IsFile());
		ioParms.ReturnFilePathAsFileOrFolder(manifestPath);
	}
	else
	{
		ioParms.ReturnNullValue();
	}
	QuickReleaseRefCountable(inDatabase);
}


void VJSDatabase::_tempSetIndexNewFourche( XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase)
{
	gTempNewIndexFourche = ioParms.GetBoolParam(1, L"new", L"old");
}


void VJSDatabase::_verify( XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase)
{
	StErrorContextInstaller errs(false);
	bool ok = false;

	CDB4DManager* db4D = CDB4DManager::RetainManager();
	VError err = VE_OK;
	VJSObject paramObj(ioParms.GetContextRef());
	if (ioParms.IsObjectParam(1))
	{
		ioParms.GetParamObject(1, paramObj);
	}
	else
		paramObj.MakeEmpty();

	JSTOOLSIntf toolintf(ioParms.GetContextRef(), paramObj);
	CDB4DBase* basex = inDatabase->RetainBaseX();
	if (basex != nil)
	{
		CDB4DRawDataBase* dataDB = basex->OpenRawDataBase(&toolintf, err);
		if (dataDB != nil)
		{
			err = dataDB->CheckAll(&toolintf);
			ok = err == VE_OK;
			dataDB->Release();
		}
		basex->Release();
	}
	ioParms.ReturnBool(ok);
}


void VJSDatabase::_queryOptions( XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase)
{
	if (ioParms.IsObjectParam(1))
	{
		BaseTaskInfo* context = GetBaseTaskInfoFromJSContext(ioParms, inDatabase);
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
		{ "getModelDefinition", js_callStaticFunction<_getModelDefinition>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
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
		{ "backup", js_callStaticFunction<_Backup>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "getJournalFile", js_callStaticFunction<_GetJournalFile>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "isJournalEnabled", js_callStaticFunction<_IsJournalEnabled>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "disableJournal", js_callStaticFunction<_DisableJournal>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "backupAndChangeJournal", js_callStaticFunction<_BackupAndChangeJournal>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "fixForV4", js_callStaticFunction<_fixForV4>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "tempSetIndexNewFourche", js_callStaticFunction<_tempSetIndexNewFourche>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		
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


VJSObject VJSDatabase::CreateInstance( JS4D::ContextRef inContext, Base4D *inDatabase)
{
	return VJSObject( inContext, inherited::CreateInstance( inContext, inDatabase));
}


//======================================================




void VJSDatabaseTableEnumerator::Initialize( const VJSParms_initialize& inParms, Base4D* inDatabase)
{
	inDatabase->Retain();
}


void VJSDatabaseTableEnumerator::Finalize( const VJSParms_finalize& inParms, Base4D* inDatabase)
{
	inDatabase->Release();
}

void VJSDatabaseTableEnumerator::GetPropertyNames( VJSParms_getPropertyNames& ioParms, Base4D* inDatabase)
{
	sLONG nb = inDatabase->GetNBTable();
	for (sLONG i = 1; i <= nb; i++)
	{
		VString tablename;
		Table* tt = inDatabase->RetainTable(i);
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


void VJSDatabaseTableEnumerator::GetProperty( VJSParms_getProperty& ioParms, Base4D* inDatabase)
{
	Table* tt = nil;
	EntityModel* em = nil;

	VString propname;
	sLONG num;
	if (ioParms.GetPropertyNameAsLong( &num))
	{
		tt = inDatabase->RetainTable(num-1);
	}
	else
	{
		ioParms.GetPropertyName(propname);
		tt = inDatabase->FindAndRetainTableRef(propname);
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


VJSObject VJSDatabaseTableEnumerator::CreateInstance( JS4D::ContextRef inContext, Base4D *inDatabase)
{
	return VJSObject( inContext, inherited::CreateInstance( inContext, inDatabase));
}



//======================================================




void VJSDatabaseEMEnumerator::Initialize( const VJSParms_initialize& inParms, Base4D* inDatabase)
{
	inDatabase->Retain();
}


void VJSDatabaseEMEnumerator::Finalize( const VJSParms_finalize& inParms, Base4D* inDatabase)
{
	inDatabase->Release();
}

void VJSDatabaseEMEnumerator::GetPropertyNames( VJSParms_getPropertyNames& ioParms, Base4D* inDatabase)
{
	//vector<EntityModel*> entities;
	vector<VRefPtr<EntityModel> > entities;
	inDatabase->RetainAllEntityModels(entities, false);
	for (vector<VRefPtr<EntityModel> >::iterator cur = entities.begin(), end = entities.end(); cur != end; cur++)
	{
		EntityModel* em = *cur;
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


void VJSDatabaseEMEnumerator::GetProperty( VJSParms_getProperty& ioParms, Base4D* inDatabase)
{
	VString propname;
	
	ioParms.GetPropertyName(propname);
	EntityModel* em = inDatabase->RetainEntity(propname, false);
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


VJSObject VJSDatabaseEMEnumerator::CreateInstance( JS4D::ContextRef inContext, Base4D *inDatabase)
{
	return VJSObject( inContext, inherited::CreateInstance( inContext, inDatabase));
}


//======================================================




void VJSEntityAttributeEnumerator::Initialize( const VJSParms_initialize& inParms, EntityModel* inModel)
{
	inModel->Retain();
}


void VJSEntityAttributeEnumerator::Finalize( const VJSParms_finalize& inParms, EntityModel* inModel)
{
	inModel->Release();
}


void VJSEntityAttributeEnumerator::GetPropertyNames( VJSParms_getPropertyNames& ioParms, EntityModel* inModel)
{
	sLONG nb = inModel->CountAttributes();
	for (sLONG i = 1; i <= nb; i++)
	{
		EntityAttribute* att = inModel->getAttribute(i);
		if (att != nil)
		{
			ioParms.AddPropertyName(att->GetAttibuteName());
		}
	}
}


void VJSEntityAttributeEnumerator::GetProperty( VJSParms_getProperty& ioParms, EntityModel* inModel)
{
	EntityAttribute* att = nil;

	sLONG num;
	VString propname;
	ioParms.GetPropertyName( propname);
	if (ioParms.GetPropertyNameAsLong( &num))
	{
		att = inModel->getAttribute(num-1);
	}
	else
	{
		att = inModel->getAttribute(propname);
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



void VJSEntityModel::Initialize( const VJSParms_initialize& inParms, EntityModel* inModel)
{
	inModel->Retain();
}


void VJSEntityModel::Finalize( const VJSParms_finalize& inParms, EntityModel* inModel)
{
	inModel->Release();
}


void VJSEntityModel::_isEntityModel(VJSParms_callStaticFunction& ioParms, EntityModel* inModel)
{
	ioParms.ReturnBool(true);
}



void VJSEntityModel::GetPropertyNames( VJSParms_getPropertyNames& ioParms, EntityModel* inModel)
{
	sLONG nb = inModel->CountAttributes();
	for (sLONG i = 1; i <= nb; i++)
	{
		EntityAttribute* att = inModel->getAttribute(i);
		if (att != nil)
		{
			ioParms.AddPropertyName(att->GetAttibuteName());
		}
	}
}


void VJSEntityModel::GetProperty( VJSParms_getProperty& ioParms, EntityModel* inModel)
{
	EntityAttribute* att = nil;
	
	sLONG num;
	VString propname;
	ioParms.GetPropertyName( propname);
	if (ioParms.GetPropertyNameAsLong( &num))
	{
		att = inModel->getAttribute(num);
	}
	else
	{
		att = inModel->getAttribute(propname);
	}
	if (att != nil)
	{
		ioParms.ReturnValue(VJSEntityAttribute::CreateInstance(ioParms.GetContextRef(), att));
	}
	else
	{
		EntityMethod* meth = inModel->getMethod(propname);
		if (meth != nil && meth->GetMethodKind() == emeth_static)
		{
			VUUID permid, promoteid;
			BaseTaskInfo* context = GetBaseTaskInfoFromJSContext(ioParms, inModel);
			//if (okperm(context, permid))
			{
				//bool waspromoted = promoteperm(basecontext, promoteid);
				VJSObject localObjFunc(ioParms.GetContextRef());
				VJSObject* objfunc = meth->getFuncObject(context, localObjFunc);
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

		EntityModel* inModel = ioParms.GetObject().GetPrivateData<VJSEntityModel>();
		if (ioParms.IsObjectParam(1))
		{
			VJSObject objParam(ioParms.GetContextRef());
			ioParms.GetParamObject(1, objParam);
			BaseTaskInfo* context = GetBaseTaskInfoFromJSContext(ioParms, inModel);

			/*
			bool first = true;

			vector<VString> props;
			objParam.GetPropertyNames(props);

			SearchTab query(inModel);

			VString queryString;

			for (vector<VString>::const_iterator cur = props.begin(), end = props.end(); cur != end && err == VE_OK; cur++)
			{
				VJSValue jsval(objParam.GetContextRef());
				jsval = objParam.GetProperty(*cur);
				VValueSingle* cv = jsval.CreateVValue();
				if (cv != nil)
				{
					if (!first)
						query.AddSearchLineBoolOper(DB4D_And);
					query.AddSearchLineEm(*cur, DB4D_Like, cv, false);
					delete cv;
					first = false;
				}

			}

			EntityCollection* sel = inModel->executeQuery(&query, context, nil, nil, DB4D_Do_Not_Lock, 0, nil, &err);
			*/
			
			EntityCollection* sel = inModel->executeQuery(objParam, context, nil, nil, DB4D_Do_Not_Lock, 0, nil, &err);
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
			
			EntityRecord* erec = nil;

			BaseTaskInfo* context = GetBaseTaskInfoFromJSContext(ioParms, inModel);

			erec = inModel->findEntityWithPrimKey(key, context, err, DB4D_Do_Not_Lock);

			if (erec != nil)
			{
				EntitySelectionIterator* iter = new EntitySelectionIterator( erec, context);
				ioParms.ReturnValue(VJSEntitySelectionIterator::CreateInstance(ioParms.GetContextRef(), iter));
				erec->Release();
			}
		}
	}
}


void VJSEntityModel::CallAsConstructor(VJSParms_callAsConstructor& ioParms)
{
	EntityModel* inModel = ioParms.GetObject().GetPrivateData<VJSEntityModel>();
	XBOX::VError err = XBOX::VE_OK;
	BaseTaskInfo* context = GetBaseTaskInfoFromJSContext(ioParms, inModel);
	EntityRecord* erec = inModel->newEntity( context);

	if (erec != nil)
	{
		if (ioParms.IsObjectParam(1))
		{
			VJSObject obj(ioParms.GetContextRef());
			ioParms.GetParamObject(1, obj);
			err = erec->convertFromJSObj(obj);
		}

		EntitySelectionIterator* iter = new EntitySelectionIterator( erec, context);
		ioParms.ReturnConstructedObject(VJSEntitySelectionIterator::CreateInstance(ioParms.GetContextRef(), iter));
		erec->Release();
	}

}


void VJSEntityModel::_getName(VJSParms_callStaticFunction& ioParms, EntityModel* inModel)
{
	ioParms.ReturnString(inModel->GetEntityName());
}


void VJSEntityModel::_getScope(VJSParms_callStaticFunction& ioParms, EntityModel* inModel)
{
	ioParms.ReturnString(EScopeKinds[inModel->GetScope()]);
}


void VJSEntityModel::_getDataStore(VJSParms_callStaticFunction& ioParms, EntityModel* inModel)
{
	Base4D* owner = inModel->GetCatalog()->GetOwner();
	if (owner != nil)
		ioParms.ReturnValue(VJSDatabase::CreateInstance(ioParms.GetContextRef(), owner));
	QuickReleaseRefCountable(owner);
}


void VJSEntityModel::_AllEntities(VJSParms_callStaticFunction& ioParms, EntityModel* inModel)
{
	VError err = VE_OK;
	EntityCollection* sel = inModel->SelectAllEntities(GetBaseTaskInfoFromJSContext(ioParms, inModel), &err);
	if (sel != nil)
	{
		ioParms.ReturnValue(VJSEntitySelection::CreateInstance(ioParms.GetContextRef(), sel));
		sel->Release();
	}
}


void QueryJS(VJSParms_callStaticFunction& ioParms, EntityModel* inModel, EntityCollection* filter)
{
	VString querystring;
	VError err = VE_OK;
	if (ioParms.GetStringParam(1, querystring))
	{
		BaseTaskInfo* context = GetBaseTaskInfoFromJSContext(ioParms, inModel);
		EntityCollection* sel = inModel->executeQuery(querystring, ioParms, context, filter, nil, DB4D_Do_Not_Lock, 0, nil, &err);
		if (sel != nil)
		{
			sel->SetQueryPlan(context->GetLastQueryPlan());
			sel->SetQueryPath(context->GetLastQueryPath());
			ioParms.ReturnValue(VJSEntitySelection::CreateInstance(ioParms.GetContextRef(), sel));
			sel->Release();
		}
	}
	
		/*
	VString querystring;
	VError err = VE_OK;
	if (ioParms.GetStringParam(1, querystring))
	{
		bool withlock = false;
		//ioParms.GetBoolParam(2, &withlock);
		SearchTab query(inModel);
		{
			VString orderby;
			BaseTaskInfo* context = GetBaseTaskInfoFromJSContext(ioParms, inModel);
			QueryParamElementVector qparams;
			err = getQParams(ioParms, 2, qparams, &query);
			err = query.BuildFromString(querystring, orderby, context, inModel, false, &qparams);
			if (err == VE_OK)
			{
				err = FillQueryWithParams(&query, ioParms, 2);

				if (err == VE_OK)
				{	 
					EntityCollection* sel = inModel->executeQuery(&query, context, filter, nil, withlock ? DB4D_Keep_Lock_With_Transaction : DB4D_Do_Not_Lock, 0, nil, &err);
					if (sel != nil && !orderby.IsEmpty())
					{
						bool ok = sel->SortCollection(orderby, nil, context);
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
		}
	}
	*/
}

void VJSEntityModel::_Query(VJSParms_callStaticFunction& ioParms, EntityModel* inModel)
{
	QueryJS(ioParms, inModel, nil);
}


void FindJS(VJSParms_callStaticFunction& ioParms, EntityModel* inModel, EntityCollection* filter)
{
	VString querystring;
	VError err = VE_OK;
	bool okresult = false;
	if (ioParms.GetStringParam(1, querystring))
	{
		BaseTaskInfo* context = GetBaseTaskInfoFromJSContext(ioParms, inModel);
		EntityCollection* sel = inModel->executeQuery(querystring, ioParms, context, filter, nil, DB4D_Do_Not_Lock, 0, nil, &err);
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
	if (!okresult)
		ioParms.ReturnNullValue();


/*
	VString querystring;
	VError err = VE_OK;
	bool okresult = false;
	if (ioParms.GetStringParam(1, querystring))
	{
		bool withlock = false;
		//ioParms.GetBoolParam(2, &withlock);
		SearchTab query(inModel);
		{
			BaseTaskInfo* context = GetBaseTaskInfoFromJSContext(ioParms, inModel);
			VString orderby;
			QueryParamElementVector qparams;
			err = getQParams(ioParms, 2, qparams, &query);
			err = query.BuildFromString(querystring, orderby, context, inModel, false, &qparams);
			if (err == VE_OK)
				err = FillQueryWithParams(&query, ioParms, 2);
			if (err == VE_OK)
			{
				EntityCollection* sel = inModel->ExecuteQuery(&query, context, filter, nil, withlock ? DB4D_Keep_Lock_With_Transaction : DB4D_Do_Not_Lock, 0, nil, &err);
				if (sel != nil && !orderby.IsEmpty())
				{
					bool ok = sel->SortCollection(orderby, nil, context);
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
		}
	}
	if (!okresult)
		ioParms.ReturnNullValue();
*/
}

void VJSEntityModel::_Find(VJSParms_callStaticFunction& ioParms, EntityModel* inModel)
{
	FindJS(ioParms, inModel, nil);
}



void VJSEntityModel::_NewEntity(VJSParms_callStaticFunction& ioParms, EntityModel* inModel)
{
	BaseTaskInfo* context = GetBaseTaskInfoFromJSContext(ioParms, inModel);
	EntityRecord* rec = inModel->newEntity(context);
	if (rec != nil)
	{
		EntitySelectionIterator* iter = new EntitySelectionIterator(rec, context);
		ioParms.ReturnValue(VJSEntitySelectionIterator::CreateInstance(ioParms.GetContextRef(), iter));
		rec->Release();
	}
}


void VJSEntityModel::_NewSelection(VJSParms_callStaticFunction& ioParms, EntityModel* inModel)
{
	bool keepsorted = ioParms.GetBoolParam( 1, L"KeepSorted", L"AnyOrder");
	EntityCollection* sel = inModel->NewCollection(keepsorted);
	if (sel != nil)
	{
		ioParms.ReturnValue(VJSEntitySelection::CreateInstance(ioParms.GetContextRef(), sel));
		sel->Release();
	}
}


void VJSEntityModel::_First(VJSParms_callStaticFunction& ioParms, EntityModel* inModel)
{
	VError err = VE_OK;
	EntityCollection* sel = inModel->SelectAllEntities(GetBaseTaskInfoFromJSContext(ioParms, inModel), &err);
	if (sel != nil)
	{
		VJSEntitySelection::_First(ioParms, sel);
		sel->Release();
	}
}

void VJSEntityModel::_Count(VJSParms_callStaticFunction& ioParms, EntityModel* inModel)
{
	if (ioParms.CountParams() > 0)
	{
		VError err = VE_OK;
		EntityCollection* sel = inModel->SelectAllEntities(GetBaseTaskInfoFromJSContext(ioParms, inModel), &err);
		if (sel != nil)
		{
			VJSEntitySelection::_Count(ioParms, sel);
			sel->Release();
		}
	}
	else
	ioParms.ReturnNumber(inModel->countEntities(GetBaseTaskInfoFromJSContext(ioParms, inModel)));
}

void VJSEntityModel::_OrderBy(VJSParms_callStaticFunction& ioParms, EntityModel* inModel)
{
	VError err = VE_OK;
	EntityCollection* sel = inModel->SelectAllEntities(GetBaseTaskInfoFromJSContext(ioParms, inModel), &err);
	if (sel != nil)
	{
		VJSEntitySelection::_OrderBy(ioParms, sel);
		sel->Release();
	}
}

void VJSEntityModel::_Each(VJSParms_callStaticFunction& ioParms, EntityModel* inModel)
{
	VError err = VE_OK;
	EntityCollection* sel = inModel->SelectAllEntities(GetBaseTaskInfoFromJSContext(ioParms, inModel), &err);
	if (sel != nil)
	{
		VJSEntitySelection::_Each(ioParms, sel);
		sel->Release();
	}
}

void VJSEntityModel::_dropEntities(VJSParms_callStaticFunction& ioParms, EntityModel* inModel)
{
	VError err = VE_OK;
	EntityCollection* sel = inModel->SelectAllEntities(GetBaseTaskInfoFromJSContext(ioParms, inModel), &err);
	if (sel != nil)
	{
		VJSEntitySelection::_dropEntities(ioParms, sel);
		sel->Release();
	}
}

void VJSEntityModel::_distinctValues(VJSParms_callStaticFunction& ioParms, EntityModel* inModel)
{
	VError err = VE_OK;
	EntityCollection* sel = inModel->SelectAllEntities(GetBaseTaskInfoFromJSContext(ioParms, inModel), &err);
	if (sel != nil)
	{
		VJSEntitySelection::_distinctValues(ioParms, sel);
		sel->Release();
	}
}


void VJSEntityModel::_toArray(VJSParms_callStaticFunction& ioParms, EntityModel* inModel)
{
	VError err = VE_OK;
	EntityCollection* sel = inModel->SelectAllEntities(GetBaseTaskInfoFromJSContext(ioParms, inModel), &err);
	if (sel != nil)
	{
		VJSEntitySelection::_toArray(ioParms, sel);
		sel->Release();
	}
}


void VJSEntityModel::_sum(VJSParms_callStaticFunction& ioParms, EntityModel* inModel)
{
	VError err = VE_OK;
	EntityCollection* sel = inModel->SelectAllEntities(GetBaseTaskInfoFromJSContext(ioParms, inModel), &err);
	if (sel != nil)
	{
		VJSEntitySelection::_sum(ioParms, sel);
		sel->Release();
	}
}


void VJSEntityModel::_min(VJSParms_callStaticFunction& ioParms, EntityModel* inModel)
{
	VError err = VE_OK;
	EntityCollection* sel = inModel->SelectAllEntities(GetBaseTaskInfoFromJSContext(ioParms, inModel), &err);
	if (sel != nil)
	{
		VJSEntitySelection::_min(ioParms, sel);
		sel->Release();
	}
}


void VJSEntityModel::_max(VJSParms_callStaticFunction& ioParms, EntityModel* inModel)
{
	VError err = VE_OK;
	EntityCollection* sel = inModel->SelectAllEntities(GetBaseTaskInfoFromJSContext(ioParms, inModel), &err);
	if (sel != nil)
	{
		VJSEntitySelection::_max(ioParms, sel);
		sel->Release();
	}
}


void VJSEntityModel::_average(VJSParms_callStaticFunction& ioParms, EntityModel* inModel)
{
	VError err = VE_OK;
	EntityCollection* sel = inModel->SelectAllEntities(GetBaseTaskInfoFromJSContext(ioParms, inModel), &err);
	if (sel != nil)
	{
		VJSEntitySelection::_average(ioParms, sel);
		sel->Release();
	}
}


void VJSEntityModel::_compute(VJSParms_callStaticFunction& ioParms, EntityModel* inModel)
{
	VError err = VE_OK;
	EntityCollection* sel = inModel->SelectAllEntities(GetBaseTaskInfoFromJSContext(ioParms, inModel), &err);
	if (sel != nil)
	{
		VJSEntitySelection::_compute(ioParms, sel);
		sel->Release();
	}
}


void VJSEntityModel::_callMethod(XBOX::VJSParms_callStaticFunction& ioParms, EntityModel* inModel)
{
	BaseTaskInfo* basecontext = GetBaseTaskInfoFromJSContext(ioParms, inModel);
	if (ioParms.IsStringParam(1))
	{
		VString funcname;
		ioParms.GetStringParam(1, funcname);
		if (ioParms.IsArrayParam(2))
		{
			VJSArray arr(ioParms.GetContextRef(), nil,  true);
			ioParms.GetParamArray(2, arr);
			if (ioParms.IsObjectParam(3))
			{
				VJSObject thisobj(ioParms.GetContextRef());
				ioParms.GetParamObject(3, thisobj);
				VError err = VE_OK;
				ioParms.ReturnValue(inModel->call_Method(funcname, arr, thisobj, basecontext, ioParms.GetContextRef(), err));
			}
			else
				vThrowError(VE_JVSC_WRONG_PARAMETER_TYPE_OBJECT, "3");
		}
		else
			vThrowError(VE_JVSC_WRONG_PARAMETER_TYPE_ARRAY, "2");
	}
	else
		vThrowError(VE_JVSC_WRONG_PARAMETER_TYPE_STRING, "1");
}


void VJSEntityModel::_fromArray(VJSParms_callStaticFunction& ioParms, EntityModel* inModel)
{
	VError err = VE_OK;
	bool okresult = false;
	BaseTaskInfo* basecontext = GetBaseTaskInfoFromJSContext(ioParms, inModel);
	if (ioParms.IsArrayParam(1))
	{
		VJSArray arr(ioParms.GetContextRef(), nil,  true);
		ioParms.GetParamArray(1, arr);

		EntityCollection* xsel = inModel->FromArray(arr, basecontext, err, nil);

		if (xsel != nil)
		{
			okresult = true;
			ioParms.ReturnValue(VJSEntitySelection::CreateInstance(ioParms.GetContextRef(), xsel));
			xsel->Release();
		}

	}
	if (!okresult)
		ioParms.ReturnNullValue();
}


void VJSEntityModel::_setAutoSequenceNumber(VJSParms_callStaticFunction& ioParms, EntityModel* inModel)
{
	VError err = VE_OK;
	bool okresult = false;
	BaseTaskInfo* context = GetBaseTaskInfoFromJSContext(ioParms, inModel);
	Real newnum;
	if (ioParms.IsNumberParam(1))
	{
		ioParms.GetRealParam(1, &newnum);
		err = inModel->SetAutoSequenceNumber((sLONG8)newnum, context);
	}
}


void VJSEntityModel::_getFragmentation(VJSParms_callStaticFunction& ioParms, EntityModel* inModel)
{
	/*
	VError err = VE_OK;
	bool okresult = false;
	CDB4DBaseContext* context = GetDB4DBaseContextFromJSContext(ioParms, inModel);

	CDB4DTable* tt = inModel->RetainTable();
	if (tt != nil)
	{
		Real result = 0.0;
		sLONG8 total = 0, frags = 0;
		err = tt->GetFragmentation(total, frags, context);
		if (total > 0)
			result = (Real)frags / (Real)total;
		ioParms.ReturnNumber(result);
	}
	QuickReleaseRefCountable(tt);
	*/

}



void VJSEntityModel::_getAttributes( XBOX::VJSParms_getProperty& ioParms, EntityModel* inModel)
{
	ioParms.ReturnValue(VJSEntityAttributeEnumerator::CreateInstance(ioParms.GetContextRef(), inModel));
}


void VJSEntityModel::_getLength( XBOX::VJSParms_getProperty& ioParms, EntityModel* inModel)
{
	ioParms.ReturnNumber(inModel->countEntities(GetBaseTaskInfoFromJSContext(ioParms, inModel)));
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
		{ "getFragmentation", js_callStaticFunction<_getFragmentation>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },

		{ "sum", js_callStaticFunction<_sum>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "min", js_callStaticFunction<_min>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "max", js_callStaticFunction<_max>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "average", js_callStaticFunction<_average>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "compute", js_callStaticFunction<_compute>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },

		{ "callMethod", js_callStaticFunction<_callMethod>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
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



void VJSEntityAttribute::Initialize( const VJSParms_initialize& inParms, EntityAttribute* inAttribute)
{
	inAttribute->Retain();
}


void VJSEntityAttribute::Finalize( const VJSParms_finalize& inParms, EntityAttribute* inAttribute)
{
	inAttribute->Release();
}


void VJSEntityAttribute::_getName(VJSParms_callStaticFunction& ioParms, EntityAttribute* inAttribute)
{
	ioParms.ReturnString(inAttribute->GetName());
}


void VJSEntityAttribute::_getPropName( XBOX::VJSParms_getProperty& ioParms, EntityAttribute* inAttribute)
{
	ioParms.ReturnString(inAttribute->GetName());
}


void VJSEntityAttribute::_getKind( XBOX::VJSParms_getProperty& ioParms, EntityAttribute* inAttribute)
{
	ioParms.ReturnString(EattTypes[inAttribute->GetKind()]);
}


void VJSEntityAttribute::_getScope( XBOX::VJSParms_getProperty& ioParms, EntityAttribute* inAttribute)
{
	ioParms.ReturnString(EScopeKinds[inAttribute->getScope()]);
}


void VJSEntityAttribute::_getType( XBOX::VJSParms_getProperty& ioParms, EntityAttribute* inAttribute)
{
	VString s;
	EntityAttributeKind kind = inAttribute->GetKind();
	if (kind == eattr_composition || kind == eattr_relation_1toN)
	{
		EntityModel* rel = inAttribute->GetSubEntityModel();
		if (rel != nil)
		{
			s = rel->GetCollectionName();
		}
	}
	else if (kind == eattr_relation_Nto1)
	{
		EntityModel* rel = inAttribute->GetSubEntityModel();
		if (rel != nil)
		{
			s = rel->GetName();
		}
		}
	else
	{
		s = EValPredefinedTypes[inAttribute->GetDataKind()];
	}
	ioParms.ReturnString(s);
}


void VJSEntityAttribute::_getIndexType( XBOX::VJSParms_getProperty& ioParms, EntityAttribute* inAttribute)
{
	VString s;
	s = inAttribute->GetIndexKind();
	ioParms.ReturnString(s);
}


void VJSEntityAttribute::_getIndexed( XBOX::VJSParms_getProperty& ioParms, EntityAttribute* inAttribute)
{
	bool res = inAttribute->IsIndexed();
	ioParms.ReturnBool(res);
}


void VJSEntityAttribute::_getFullTextIndexed( XBOX::VJSParms_getProperty& ioParms, EntityAttribute* inAttribute)
{
	bool res = inAttribute->IsFullTextIndexed();
	ioParms.ReturnBool(res);
}



void VJSEntityAttribute::_getDataClass( XBOX::VJSParms_getProperty& ioParms, EntityAttribute* inAttribute)
{
	EntityModel* res = inAttribute->GetModel();
	if (res == nil)
		ioParms.ReturnNullValue();
	else
		ioParms.ReturnValue(VJSEntityModel::CreateInstance(ioParms.GetContextRef(), res));
}


void VJSEntityAttribute::_getRelatedDataClass( XBOX::VJSParms_getProperty& ioParms, EntityAttribute* inAttribute)
{
	EntityModel* res = inAttribute->GetSubEntityModel();
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
	EntityModel* em = inSelIter->GetModel();
	sLONG nb = em->CountAttributes();
	for (sLONG i = 1; i <= nb; i++)
	{
		EntityAttribute* att = em->getAttribute(i);
		if (att != nil)
		{
			ioParms.AddPropertyName(att->GetAttibuteName());
		}
	}
}

void VJSEntitySelectionIterator::GetProperty( VJSParms_getProperty& ioParms, EntitySelectionIterator* inSelIter)
{
	EntityModel* em = inSelIter->GetModel();
	BaseTaskInfo* context = GetBaseTaskInfoFromJSContext(ioParms, em);
	VString propname;
	ioParms.GetPropertyName(propname);
	EntityAttribute* att = em->getAttribute(propname);
	if (att != nil)
	{
		EntityRecord* rec = inSelIter->GetCurRec(context);
		if (rec != nil)
		{
			VError err = VE_OK;
			EntityAttributeValue* emval = rec->getAttributeValue(att, err, context);
			if (emval != nil)
			{
				switch (emval->GetAttributeKind()) {
					case eav_vvalue:
						{
							VValueSingle* cv = emval->GetVValue();
							if (cv != nil)
							{
									ioParms.ReturnVValue(*cv);
							}
						}
						break;
						
					case eav_subentity:
						{
							EntityRecord* subrec = emval->getRelatedEntity();
							if (subrec != nil)
							{
								EntitySelectionIterator* subiter = new EntitySelectionIterator(subrec, context);
								ioParms.ReturnValue(VJSEntitySelectionIterator::CreateInstance(ioParms.GetContextRef(), subiter));
							}
						}
						break;
						
					case eav_selOfSubentity:
						{
							EntityCollection* sel = emval->getRelatedSelection();
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
		EntityMethod* meth = em->getMethod(propname);
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
	EntityModel* em = inSelIter->GetModel();
	BaseTaskInfo* context = GetBaseTaskInfoFromJSContext(ioParms, em);
	VString propname;
	ioParms.GetPropertyName(propname);
	EntityAttribute* att = em->getAttribute(propname);
	if (att != nil)
	{
		EntityRecord* rec = inSelIter->GetCurRec(context);
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
				rec->setAttributeValue(att, jsobj);
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
							VValueSingle* cv = elem.CreateVValue(nil, att->isSimpleDate());
							if (cv != nil)
							{
								vals.push_back(cv);
							}
						}
						rec->setAttributeValue(att, &vals);
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
									CDB4DEntityAttributeValue* emval = rec->getAttributeValue(att, err, context);
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
														rec->touchAttributeValue(att);
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
							VValueSingle* cv = ioParms.CreatePropertyVValue(att->isSimpleDate());
							//if (cv != nil)
							{
								rec->setAttributeValue(att, cv);
								if (cv != nil)
									delete cv;
							}
						}
					}
				}
				else
				{
					rec->setAttributeValue(att, xrelatedRec->GetCurRec(context));
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
	EntityRecord* inRecord = inSelIter->GetCurRec(GetBaseTaskInfoFromJSContext(ioParms, inSelIter->GetModel()));
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
	EntityRecord* inRecord = inSelIter->GetCurRec(GetBaseTaskInfoFromJSContext(ioParms, inSelIter->GetModel()));
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
	BaseTaskInfo* context = GetBaseTaskInfoFromJSContext(ioParms, inSelIter->GetModel());
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
	ioParms.ReturnBool(inSelIter->GetCurRec(GetBaseTaskInfoFromJSContext(ioParms, inSelIter->GetModel())) != nil);
}



void VJSEntitySelectionIterator::_IsModified(VJSParms_callStaticFunction& ioParms, EntitySelectionIterator* inSelIter)
{
	EntityRecord* inRecord = inSelIter->GetCurRec(GetBaseTaskInfoFromJSContext(ioParms, inSelIter->GetModel()));
	if (inRecord == nil)
		ioParms.ReturnBool(false);
	else
		ioParms.ReturnBool(inRecord->IsModified());
}


void VJSEntitySelectionIterator::_IsNew(VJSParms_callStaticFunction& ioParms, EntitySelectionIterator* inSelIter)
{
	EntityRecord* inRecord = inSelIter->GetCurRec(GetBaseTaskInfoFromJSContext(ioParms, inSelIter->GetModel()));
	if (inRecord == nil)
		ioParms.ReturnBool(false);
	else
		ioParms.ReturnBool(inRecord->IsNew());
}


void VJSEntitySelectionIterator::_Drop(VJSParms_callStaticFunction& ioParms, EntitySelectionIterator* inSelIter)
{
	VError err = VE_OK;
	EntityRecord* inRecord = inSelIter->GetCurRec(GetBaseTaskInfoFromJSContext(ioParms, inSelIter->GetModel()));
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
	err = inSelIter->ReLoadCurRec(GetBaseTaskInfoFromJSContext(ioParms, inSelIter->GetModel()), readonly, false);
}


void VJSEntitySelectionIterator::_getTimeStamp(VJSParms_callStaticFunction& ioParms, EntitySelectionIterator* inSelIter)
{
	VError err = VE_OK;
	EntityRecord* inRecord = inSelIter->GetCurRec(GetBaseTaskInfoFromJSContext(ioParms, inSelIter->GetModel()));
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
	
	EntityRecord* inRecord = inSelIter->GetCurRec(GetBaseTaskInfoFromJSContext(ioParms, inSelIter->GetModel()));
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
	EntityRecord* inRecord = inSelIter->GetCurRec(GetBaseTaskInfoFromJSContext(ioParms, inSelIter->GetModel()));
	if (inRecord != nil)
	{
		ioParms.ReturnNumber(inRecord->GetModificationStamp());
	}
}


void VJSEntitySelectionIterator::_getModifiedAttributes(XBOX::VJSParms_callStaticFunction& ioParms, EntitySelectionIterator* inSelIter)
{
	VError err = VE_OK;
	EntityRecord* inRecord = inSelIter->GetCurRec(GetBaseTaskInfoFromJSContext(ioParms, inSelIter->GetModel()));
	VJSArray arr(ioParms.GetContextRef());
	if (inRecord != nil)
	{
		EntityAttributeCollection result;
		inRecord->GetModifiedAttributes(result);
		for (EntityAttributeCollection::iterator cur = result.begin(), end = result.end(); cur != end; ++cur)
		{
			arr.PushString((*cur)->GetName());
		}
	}
	ioParms.ReturnValue(arr);
}


void EntityRecToString(EntityRecord* inRecord, BaseTaskInfo* context, VString& result)
{
	result.Clear();
	VError err = VE_OK;
	if (inRecord != nil)
	{
		EntityModel* em = inRecord->GetModel();
		bool first = true;
		sLONG nbatt = em->CountAttributes();
		for (sLONG i = 1; i <= nbatt; i++)
		{
			EntityAttribute* att = em->getAttribute(i);
			if (att != nil)
			{
				if (first)
					first = false;
				else
					result += L"  ,  ";
				result += att->GetAttibuteName()+L" : ";
				EntityAttributeValue* val = inRecord->getAttributeValue(att, err, context);
				if (val == nil)
					result += L"null";
				else
				{
					switch (val->GetAttributeKind())
					{
					case eav_vvalue:
						{
							XBOX::VValueSingle* cv = val->getVValue();
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
							EntityRecord* rec = val->getRelatedEntity();
							if (rec == nil)
								result += L"<<null entity>>";
							else
								result += L"1 entity";
						}
						break;

					case eav_selOfSubentity:
						{
							EntityCollection* sel = val->getRelatedSelection();
							if (sel == nil)
								result += L"<<null selection>>";
							else
							{
								sLONG nbent = sel->GetLength(context);
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
	EntityModel* em = inSelIter->GetModel();
	BaseTaskInfo* context = GetBaseTaskInfoFromJSContext(ioParms, em);
	EntityRecord* inRecord = inSelIter->GetCurRec(context);
	EntityRecToString(inRecord, context, result);
	ioParms.ReturnString(result);
}


void VJSEntitySelectionIterator::_toObject(VJSParms_callStaticFunction& ioParms, EntitySelectionIterator* inSelIter)
{
	EntityModel* em = inSelIter->GetModel();
	BaseTaskInfo* context = GetBaseTaskInfoFromJSContext(ioParms, em);
	EntityRecord* inRecord = inSelIter->GetCurRec(context);
	if (inRecord != nil)
	{
		EntityRecord* rec = inRecord;
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
	EntityModel* em = inSelIter->GetModel();
	BaseTaskInfo* context = GetBaseTaskInfoFromJSContext(ioParms, em);
	EntityRecord* inRecord = inSelIter->GetCurRec(context);
	if (inRecord != nil)
	{
		EntityRecord* rec = inRecord;
		VJSObject obj(ioParms.GetContextRef());
		obj.MakeEmpty();
		EntityAttributeSortedSelection atts(rec->GetOwner());
		rec->ConvertToJSObject(obj, atts, nil, nil, true, false);
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
		{ "getModifiedAttributes", js_callStaticFunction<_getModifiedAttributes>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
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



void VJSEntitySelection::Initialize( const VJSParms_initialize& inParms, EntityCollection* inSelection)
{
	inSelection->Retain();
}


void VJSEntitySelection::Finalize( const VJSParms_finalize& inParms, EntityCollection* inSelection)
{
	inSelection->Release();
}



void VJSEntitySelection::GetProperty( VJSParms_getProperty& ioParms, EntityCollection* inSelection)
{
	
	sLONG num;
	if (ioParms.GetPropertyNameAsLong( &num) && (num >= 0) )
	{
		VError err = VE_OK;
		BaseTaskInfo* context = GetBaseTaskInfoFromJSContext( ioParms, inSelection);
		EntityRecord* rec = inSelection->LoadEntity(num, context, err, DB4D_Do_Not_Lock);
		if (rec != nil)
		{
			EntitySelectionIterator* iter = new EntitySelectionIterator(rec, context);
			ioParms.ReturnValue(VJSEntitySelectionIterator::CreateInstance(ioParms.GetContextRef(), iter));
			rec->Release();
		}
		
	}
	else
	{
		EntityModel* em = inSelection->GetModel();
		if (em != nil)
		{
			VString propname;
			ioParms.GetPropertyName(propname);
			EntityMethod* meth = em->getMethod(propname);
			if (meth != nil)
			{
				if (meth->GetMethodKind() == emeth_sel)
				{
					BaseTaskInfo* context = GetBaseTaskInfoFromJSContext( ioParms, em);
					VJSObject localObjFunc(ioParms.GetContextRef());
					VJSObject* objfunc = meth->getFuncObject(context, localObjFunc);
					if (objfunc != nil)
					{
						ioParms.ReturnValue(*objfunc);
					}
				}
			}
			else
			{
				EntityAttribute* att = em->getAttribute(propname);
				if (att != nil)
				{
					BaseTaskInfo* context = GetBaseTaskInfoFromJSContext( ioParms, em);
					EntityAttributeKind kind = att->GetAttributeKind();
					if (kind == eattr_relation_Nto1 || kind == eattr_relation_1toN || kind == eattr_composition)
					{
						VError err = VE_OK;
						EntityCollection* result = inSelection->ProjectCollection(att, err, context);
						if (result != nil)
							ioParms.ReturnValue(VJSEntitySelection::CreateInstance(ioParms.GetContextRef(), result));
					}
					else
					{
						BaseTaskInfo* context = GetBaseTaskInfoFromJSContext( ioParms, inSelection);
						/*
						sLONG nbelem = inSelection->GetLength();
						VJSArray result(ioParms.GetContextRef());
						VError err = VE_OK;
						for (sLONG i = 0; i < nbelem && err == VE_OK; i++)
						{
							EntityRecord* erec = inSelection->LoadEntity(i, context, err, DB4D_Do_Not_Lock);
							if (erec != nil)
							{
								EntityAttributeValue* xval = erec->getAttributeValue(att, err);
								if (err == VE_OK && xval != nil)
								{
									VValueSingle* cv = xval->getVValue();
									result.PushValue(*cv);
								}
								//QuickReleaseRefCountable(xval);
								erec->Release();
							}
						}
						*/
						VError err = VE_OK;
						VJSValue result(inSelection->ProjectAttribute(att, err, context, ioParms.GetContextRef()));
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


void computeStats(XBOX::VJSParms_callStaticFunction& ioParms, EntityCollection* inSelection, DB4D_ColumnFormulae action)
{
	bool okresult = false;
	EntityModel* model = inSelection->GetModel();

	if (model == nil)
		ThrowBaseError(VE_DB4D_NOT_AN_ENTITY_COLLECTION);
	else
	{
		EntityModel* em = model;
		BaseTaskInfo* context = GetBaseTaskInfoFromJSContext( ioParms, inSelection);

		EntityAttribute* xatt = ioParms.GetParamObjectPrivateData<VJSEntityAttribute>(1);
		if (xatt == nil)
		{
			if (ioParms.IsStringParam(1))
			{
				VString s;
				ioParms.GetStringParam(1, s);
				xatt = model->getAttribute(s);

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
				VString s = xatt->GetModel()->GetName()+"."+xatt->GetAttibuteName();
				ThrowBaseError(VE_DB4D_ENTITY_ATTRIBUTE_IS_FROM_ANOTHER_DATACLASS, s);
				xatt = nil;
			}
			else
			{
				EntityAttribute* att = xatt;
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
			EntityAttribute* att = xatt;
			EntityCollection* selsorted = nil;

			if (action >= DB4D_Count_distinct && action <=DB4D_Sum_distinct)
			{
				EntityAttributeSortedSelection sortingAtt(em);
				sortingAtt.AddAttribute(att, nil);
				VError err = VE_OK;
				selsorted = inSelection->SortSel(err, &sortingAtt, context);
				if (selsorted != nil)
					inSelection = selsorted;
			}

			VJSValue result(ioParms.GetContextRef());
			inSelection->ComputeOnOneAttribute(xatt, action, result, context, ioParms.GetContextRef());
			ioParms.ReturnValue(result);
			okresult = true;

			/*
			{
				EntityAttributeSortedSelection attlist(em);
				attlist.AddAttribute(att, nil);

				VJSObject result(ioParms.GetContextRef());

				VError err  = inSelection->Compute(attlist, result, context, ioParms.GetContextRef());
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
			*/

			QuickReleaseRefCountable(selsorted);
		}
	}

	if (!okresult)
	{
		ioParms.ReturnNullValue();
	}

}


void VJSEntitySelection::_compute(XBOX::VJSParms_callStaticFunction& ioParms, EntityCollection* inSelection)
{
	VError err = VE_OK;
	bool okresult = false;
	BaseTaskInfo* context = GetBaseTaskInfoFromJSContext( ioParms, inSelection);
	EntityModel* model = inSelection->GetModel();
	if (model == nil)
		err = ThrowBaseError(VE_DB4D_NOT_AN_ENTITY_COLLECTION);
	else
	{
		EntityModel* em =model;
		EntityAttributeSortedSelection attlist(em);
		EntityAttributeSortedSelection groupByList(em);
		sLONG curparam = GetAttributeListParams(ioParms, 1, attlist);
		bool withdistinct = false;
		VString s;
		if (ioParms.GetStringParam(curparam, s) && (s == "distinct"))
			withdistinct = ioParms.GetBoolParam(curparam, L"distinct", L"");
		else
		{
			curparam = GetAttributeListParams(ioParms, curparam, groupByList);
			withdistinct = ioParms.GetBoolParam(curparam, L"distinct", L"");
		}

		if (attlist.empty())
		{
			err = vThrowError(VE_JVSC_WRONG_PARAMETER_TYPE_ATTRIBUTE, "1");
		}

		for (EntityAttributeSortedSelection::const_iterator cur = attlist.begin(), end = attlist.end(); cur != end && err == VE_OK; ++cur)
		{
			const EntityAttribute* att = cur->fAttribute;
			if (att->GetModel() != em )
			{
				VString s = att->GetModel()->GetName()+"."+att->GetAttibuteName();
				err = ThrowBaseError(VE_DB4D_ENTITY_ATTRIBUTE_IS_FROM_ANOTHER_DATACLASS, s);
			}
			else if (!att->IsStatable())
			{
				err = att->ThrowError(VE_DB4D_WRONG_ATTRIBUTE_KIND);
			}
		}

		for (EntityAttributeSortedSelection::const_iterator cur = groupByList.begin(), end = groupByList.end(); cur != end && err == VE_OK; ++cur)
		{
			const EntityAttribute* att = cur->fAttribute;
			if (att->GetModel() != em )
			{
				VString s = att->GetModel()->GetName()+"."+att->GetAttibuteName();
				err = ThrowBaseError(VE_DB4D_ENTITY_ATTRIBUTE_IS_FROM_ANOTHER_DATACLASS, s);
			}
			else if (!att->IsStatable())
			{
				err = att->ThrowError(VE_DB4D_WRONG_ATTRIBUTE_KIND);
			}
		}


		if (err == VE_OK)
		{
			if (groupByList.empty())
			{
				VJSObject result(ioParms.GetContextRef());
				err  = inSelection->Compute(attlist, result, context, ioParms.GetContextRef(), withdistinct);
				if (err == VE_OK)
				{
					ioParms.ReturnValue(result);
					okresult = true;
				}
			}
			else
			{
				VString attlistString, groupbyString;
				attlist.ToString(attlistString);
				groupByList.ToString(groupbyString);
				VJSFunction func("ReportingDB4D.report", ioParms.GetContextRef());
				func.AddParam(VJSEntitySelection::CreateInstance(ioParms.GetContextRef(), inSelection));
				func.AddParam(attlistString);
				func.AddParam(groupbyString);
				if (func.Call())
				{
					okresult = true;
					ioParms.ReturnValue(func.GetResult());
				}
				else
				{
					ioParms.SetException(func.GetException());
				}
			}
		}
	}
	

	if (!okresult)
	{
		ioParms.ReturnNullValue();
	}
}


static void _opersel(XBOX::VJSParms_callStaticFunction& ioParms, EntityCollection* inSelection, DB4DConjunction oper)
{
	bool okresult = false;
	BaseTaskInfo* context = GetBaseTaskInfoFromJSContext( ioParms, inSelection);
	EntityModel* model = inSelection->GetModel();
	if (model == nil)
		ThrowBaseError(VE_DB4D_NOT_AN_ENTITY_COLLECTION);
	else
	{
		EntityCollection* otherSelection = ioParms.GetParamObjectPrivateData<VJSEntitySelection>(1);
		if (otherSelection == nil)
			ThrowBaseError(VE_DB4D_NOT_AN_ENTITY_COLLECTION);
		else
		{
			EntityModel* otherModel = otherSelection->GetModel();
			if (otherModel == nil)
				ThrowBaseError(VE_DB4D_NOT_AN_ENTITY_COLLECTION);
			else
			{
				EntityModel* em = model;
				EntityModel* otherem = otherModel;
				if (otherem->isExtendedFrom(em))
				{
					VError err = VE_OK;
					EntityCollection* result = nil;
					switch (oper)
					{
						case DB4D_OR:
							result = inSelection->Or(otherSelection, err, context);
							break;
						case DB4D_And:
							result = inSelection->And(otherSelection, err, context);
							break;
						case DB4D_Except:
							result = inSelection->Minus(otherSelection, err, context);
							break;
					}
					if (result != nil)
					{
						okresult = true;
						ioParms.ReturnValue(VJSEntitySelection::CreateInstance(ioParms.GetContextRef(), result));
					}
				}
				else
					ThrowBaseError(VE_DB4D_COLLECTION_ON_INCOMPATIBLE_DATACLASSES);

			}
		}
	}
	if (!okresult)
		ioParms.ReturnNullValue();
}

void VJSEntitySelection::_and(XBOX::VJSParms_callStaticFunction& ioParms, EntityCollection* inSelection)
{
	_opersel(ioParms, inSelection, DB4D_And);
}

void VJSEntitySelection::_or(XBOX::VJSParms_callStaticFunction& ioParms, EntityCollection* inSelection)
{
	_opersel(ioParms, inSelection, DB4D_OR);
}

void VJSEntitySelection::_minus(XBOX::VJSParms_callStaticFunction& ioParms, EntityCollection* inSelection)
{
	_opersel(ioParms, inSelection, DB4D_Except);
}



void VJSEntitySelection::_Count(VJSParms_callStaticFunction& ioParms, EntityCollection* inSelection)
{
	if (ioParms.CountParams() > 0)
		computeStats(ioParms, inSelection, ioParms.GetBoolParam(2,L"distinct",L"") ? DB4D_Count_distinct : DB4D_Count);
	else
		ioParms.ReturnNumber(inSelection->GetLength(GetBaseTaskInfoFromJSContext( ioParms, inSelection)));
}


void VJSEntitySelection::_getLength( XBOX::VJSParms_getProperty& ioParms, EntityCollection* inSelection)
{
	ioParms.ReturnNumber(inSelection->GetLength(GetBaseTaskInfoFromJSContext( ioParms, inSelection)));
}


void VJSEntitySelection::_getQueryPlan( XBOX::VJSParms_getProperty& ioParms, EntityCollection* inSelection)
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


void VJSEntitySelection::_getQueryPath( XBOX::VJSParms_getProperty& ioParms, EntityCollection* inSelection)
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




void VJSEntitySelection::_First(VJSParms_callStaticFunction& ioParms, EntityCollection* inSelection)
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
	
	BaseTaskInfo* context = GetBaseTaskInfoFromJSContext( ioParms, inSelection);
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


void VJSEntitySelection::_OrderBy(VJSParms_callStaticFunction& ioParms, EntityCollection* inSelection)
{
	EntityModel* model = inSelection->GetModel();
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
			for (sLONG i = 1, nb = (sLONG)ioParms.CountParams(); i <= nb && err == VE_OK; i++)
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
					EntityAttribute* att = ioParms.GetParamObjectPrivateData<VJSEntityAttribute>(i);
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
		EntityCollection* newcol = nil;
		if (!sortorder.IsEmpty())
		{
			newcol = inSelection->SortCollection(sortorder, GetBaseTaskInfoFromJSContext( ioParms, inSelection), err, nil);
		}
		if (newcol != nil)
			ioParms.ReturnValue(VJSEntitySelection::CreateInstance(ioParms.GetContextRef(), newcol));
		else
			ioParms.ReturnNullValue();
		QuickReleaseRefCountable(newcol);
	}
}


void VJSEntitySelection::_Add(VJSParms_callStaticFunction& ioParms, EntityCollection* inSelection)
{
	VError err = VE_OK;
	if (ioParms.CountParams() > 0)
	{
		BaseTaskInfo* context = GetBaseTaskInfoFromJSContext( ioParms, inSelection);
		bool atTheEnd = ioParms.GetBoolParam( 2, L"AtTheEnd", L"AnyWhere");
		EntityCollection* sel = ioParms.GetParamObjectPrivateData<VJSEntitySelection>(1);
		if (sel != nil)
		{
			err = inSelection->AddCollection(sel, context, atTheEnd);
		}
		else
		{
			{
				EntitySelectionIterator* seliter = ioParms.GetParamObjectPrivateData<VJSEntitySelectionIterator>(1);
				if (seliter != nil && seliter->GetCurRec(context) != nil)
				{
					err = inSelection->AddEntity(seliter->GetCurRec(context), atTheEnd);
				}
			}
		}
	}
	ioParms.ReturnThis();
}



void VJSEntitySelection::_GetModel(VJSParms_callStaticFunction& ioParms, EntityCollection* inSelection)
{
	ioParms.ReturnValue(VJSEntityModel::CreateInstance(ioParms.GetContextRef(), inSelection->GetModel()));
}


void VJSEntitySelection::_Each(XBOX::VJSParms_callStaticFunction& ioParms, EntityCollection* inSelection)
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

			BaseTaskInfo* context = GetBaseTaskInfoFromJSContext( ioParms, inSelection);
			bool cont = true;
			for (sLONG i = 0, nb = inSelection->GetLength(context); i < nb && cont; i++)
			{
				VError err = VE_OK;
				EntityRecord* erec = inSelection->LoadEntity(i, context, err, DB4D_Do_Not_Lock);
				if (erec != nil)
				{
					VJSValue indiceVal2(ioParms.GetContextRef());
					indiceVal2.SetNumber(i);
					params[1] = indiceVal2;
					//params[1].SetNumber(i);
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
				else
				{
					// generate an exception on damaged entity ?
				}
			}
		}
	}
}


void VJSEntitySelection::_dropEntities(XBOX::VJSParms_callStaticFunction& ioParms, EntityCollection* inSelection)
{
	EntityCollection* locked = nil;
	BaseTaskInfo* context = GetBaseTaskInfoFromJSContext( ioParms, inSelection);
	VError err = inSelection->DropEntities(context, nil, &locked);
	if (locked != nil)
		ioParms.ReturnValue(VJSEntitySelection::CreateInstance(ioParms.GetContextRef(), locked));
	else
		ioParms.ReturnNullValue();
	QuickReleaseRefCountable(locked);
}


void VJSEntitySelection::_toString(XBOX::VJSParms_callStaticFunction& ioParms, EntityCollection* inSelection)
{
	VString out;
	BaseTaskInfo* context = GetBaseTaskInfoFromJSContext( ioParms, inSelection);
	EntityModel* model = inSelection->GetModel();
	if (model != nil)
	{
		sLONG count = 0;
		sLONG selcount = inSelection->GetLength(context);
		ioParms.GetLongParam(1, &count);
		if (count == 0)
			count = 100;
		if (count == -1 || count > selcount)
			count = selcount;
		VError err = VE_OK;
		for (sLONG i = 0; i < count; i++)
		{
			EntityRecord* rec = inSelection->LoadEntity( i, context, err, DB4D_Do_Not_Lock );
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


void VJSEntitySelection::_distinctValues(XBOX::VJSParms_callStaticFunction& ioParms, EntityCollection* inSelection)
{
	BaseTaskInfo* context = GetBaseTaskInfoFromJSContext( ioParms, inSelection);
	EntityModel* model = inSelection->GetModel();
	EntityAttribute* att = ioParms.GetParamObjectPrivateData<VJSEntityAttribute>(1);
	bool okresult = false;
	if (att == nil)
	{
		VString s;
		ioParms.GetStringParam(1, s);
		att = model->getAttribute(s);
	}
	if (att != nil)
	{
		EntityModel* em = dynamic_cast<EntityModel*>(model);
		if (att->GetOwner() != em)
		{
			VString s = att->GetOwner()->GetName()+"."+att->GetAttibuteName();
			ThrowBaseError(VE_DB4D_ENTITY_ATTRIBUTE_IS_FROM_ANOTHER_DATACLASS, s);
		}
	}
	if (att != nil)
	{
		EntityAttributeKind kind = att->GetAttributeKind();
		if (kind == eattr_storage || kind == eattr_computedField || kind == eattr_alias)
		{
			JSCollectionManager collection(ioParms.GetContextRef(), att->isSimpleDate());
			collection.SetNumberOfColumn(1);
			VCompareOptions options;
			options.SetDiacritical(true);
			
			VError err = inSelection->GetDistinctValues(att, collection, context, nil, options);
			if (err == VE_OK)
			{
				okresult = true;
				VJSArray result(collection.getArrayRef(1));
				ioParms.ReturnValue(result);
			}
			
		}
		else
			att->ThrowError(VE_DB4D_WRONG_ATTRIBUTE_KIND);

	}
	else
		vThrowError(VE_JVSC_WRONG_PARAMETER_TYPE_ATTRIBUTE, "1");

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
			EntityAttribute* att = ioParms.GetParamObjectPrivateData<VJSEntityAttribute>(StartParam);
			if (att != nil)
			{
				if (att->GetOwner() == model)
				{
					outList.AddAttribute(att, nil);
				}
				StartParam++;
			}
			else
				break;
		}

	}
}


void VJSEntitySelection::_toArray(XBOX::VJSParms_callStaticFunction& ioParms, EntityCollection* inSelection)
{
	bool okresult = false;
	BaseTaskInfo* context = GetBaseTaskInfoFromJSContext( ioParms, inSelection);
	EntityModel* model = inSelection->GetModel();
	EntityModel* xmodel = dynamic_cast<EntityModel*>(model);
	EntityAttributeSortedSelection attlist(xmodel);
	EntityAttributeSelection expand(xmodel);
	EntityAttributeSortedSelection sortingatts(xmodel);
	sLONG curparam = 1;
	sLONG startvalue = 0;
	sLONG nbvalue = -1;
	VError err = VE_OK;
	VJSObject objfunc(ioParms.GetContextRef());
	bool withfunc = false;
	//BaseTaskInfo* basecontext = ConvertContext(context);
	
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
		buildAttributeListFromParams(ioParms, curparam, xmodel, attlist, expand, sortingatts, context);

	size_t newparam = curparam;
	bool withkey = ioParms.GetBoolParam(curparam, "withKey", "withoutKey", &newparam);
	curparam = (sLONG)newparam;

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

	sLONG count = inSelection->GetLength(context);
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
			EntityRecord* erec = inSelection->LoadEntity(i, context, err, DB4D_Do_Not_Lock);
			if (erec != nil)
			{
				VJSValue indiceVal2(ioParms.GetContextRef());
				indiceVal2.SetNumber(curindice);
				params[1] = indiceVal2;
				//params[1].SetNumber(curindice);
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
		arr = inSelection->ToJsArray(context, ioParms.GetContextRef(), attlist, &expand, &sortingatts, withkey, false, startvalue, nbvalue, err);
		if (err != VE_OK)
			okresult = false;
	}

	okresult = true;
	ioParms.ReturnValue(arr);

	if (!okresult)
		ioParms.ReturnNullValue();
}


void VJSEntitySelection::_Query(XBOX::VJSParms_callStaticFunction& ioParms, EntityCollection* inSelection)
{
	EntityModel* model = inSelection->GetModel();
	QueryJS(ioParms, model, inSelection);
}


void VJSEntitySelection::_Find(XBOX::VJSParms_callStaticFunction& ioParms, EntityCollection* inSelection)
{
	EntityModel* model = inSelection->GetModel();
	FindJS(ioParms, model, inSelection);
}


void VJSEntitySelection::_sum(XBOX::VJSParms_callStaticFunction& ioParms, EntityCollection* inSelection)
{
	computeStats(ioParms, inSelection, ioParms.GetBoolParam(2,L"distinct",L"") ? DB4D_Sum_distinct : DB4D_Sum);
}


void VJSEntitySelection::_min(XBOX::VJSParms_callStaticFunction& ioParms, EntityCollection* inSelection)
{
	computeStats(ioParms, inSelection, DB4D_Min);
}


void VJSEntitySelection::_max(XBOX::VJSParms_callStaticFunction& ioParms, EntityCollection* inSelection)
{
	computeStats(ioParms, inSelection, DB4D_Max);
}


void VJSEntitySelection::_average(XBOX::VJSParms_callStaticFunction& ioParms, EntityCollection* inSelection)
{
	computeStats(ioParms, inSelection, ioParms.GetBoolParam(2,L"distinct",L"") ? DB4D_Average_distinct : DB4D_Average);
}


void VJSEntitySelection::_toJSON(XBOX::VJSParms_callStaticFunction& ioParms, EntityCollection* inSelection)
{
	BaseTaskInfo* context = GetBaseTaskInfoFromJSContext( ioParms, inSelection);
	EntityModel* model = inSelection->GetModel();
	EntityAttributeSortedSelection atts(model);
	VError err = VE_OK;
	
	VJSArray obj(inSelection->ToJsArray(context, ioParms.GetContextRef(), atts, nil, nil, true, false, -1, -1, err));
	
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
void do_OpenRemoteStore(VJSParms_callStaticFunction& ioParms, VJSGlobalObject*)
{
	VError err = VE_OK;
	if (ioParms.IsStringParam(1))
	{
		VJSONObject* params = nil;
		VString url, username, password;
		if (ioParms.IsObjectParam(1))
		{
			VJSObject objparams(ioParms.GetContextRef());
			ioParms.GetParamObject(1, objparams);
			VJSONValue inutile;
			((VJSValue)objparams).GetJSONValue(inutile);
			params = RetainRefCountable(inutile.GetObject());
		}
		else
		{
			ioParms.GetStringParam(1, url);
			ioParms.GetStringParam(2, username);
			ioParms.GetStringParam(3, password);
			params = new VJSONObject();
			params->SetPropertyAsString("hostname", url);
			params->SetPropertyAsString("user", username);
			params->SetPropertyAsString("password", password);
		}
		VFolder* folder = VDBMgr::GetManager()->RetainResourceFolder();
		VFile xfile(*folder, "remote");
		CDB4DBase* xbd = VDBMgr::GetManager()->CreateBase(xfile, DB4D_Create_Empty_Catalog | DB4D_Create_As_XML_Definition | DB4D_Create_No_Respart | DB4D_Create_WithSeparateIndexSegment, XBOX::VTask::GetCurrentIntlManager(), &err);
		Base4D* bd = dynamic_cast<VDB4DBase*>(xbd)->GetBase();

		EntityModelCatalog* remotecat = EntityModelCatalog::NewCatalog(kRemoteCatalogFactory, bd, params, err);
		if (remotecat == nil)
		{
			// throw err;
		}
		else
		{
			err = bd->AddOutsideCatalog(remotecat);
			ioParms.ReturnValue(VJSDatabase::CreateInstance(ioParms.GetContextRef(), bd));
		}

		QuickReleaseRefCountable(xbd);
	}
}


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
			CDB4DManager* db4D = CDB4DManager::RetainManager();
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
					ioParms.ReturnValue(VJSDatabase::CreateInstance(ioParms.GetContextRef(), dynamic_cast<VDB4DBase*>(newdb)->GetBase()));
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
			CDB4DManager* db4D = CDB4DManager::RetainManager();
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
					ioParms.ReturnValue(VJSDatabase::CreateInstance(ioParms.GetContextRef(), dynamic_cast<VDB4DBase*>(newdb)->GetBase()));
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
			CDB4DManager* db4D = CDB4DManager::RetainManager();
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
					ioParms.ReturnValue(VJSDatabase::CreateInstance(ioParms.GetContextRef(), dynamic_cast<VDB4DBase*>(newdb)->GetBase()));
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
	VString basename;
	if (ioParms.GetStringParam(1, basename))
	{
		Base4D* db = VDBMgr::GetManager()->RetainBase4DByName(basename);
		if (db != NULL)
		{
			ioParms.ReturnValue(VJSDatabase::CreateInstance(ioParms.GetContextRef(), db));
			db->Release();
		}
		QuickReleaseRefCountable(db);
	}

	/*
	XBOX::VString basename;
	if (ioParms.IsNumberParam(1))
	{
		CDB4DManager* db4D = CDB4DManager::RetainManager();
		sLONG i = 0;
		ioParms.GetLongParam(1, &i);
		Base4D* db = db4D->RetainNthBase(i);
		if (db != NULL)
		{
			ioParms.ReturnValue(VJSDatabase::CreateInstance(ioParms.GetContextRef(), db));
			db->Release();
		}
		XBOX::ReleaseRefCountable( &db4D);
	}
	else if (ioParms.GetStringParam(1, basename))
	{
		CDB4DManager* db4D = CDB4DManager::RetainManager();
		for (sLONG i = 1, nb = db4D->CountBases(); i <= nb; i++)
		{
			Base4D* db = db4D->RetainNthBase(i);
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
	*/
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
	VJSGlobalClass::AddStaticFunction( "openRemoteStore", VJSGlobalClass::js_callStaticFunction<do_OpenRemoteStore>, JS4D::PropertyAttributeNone);
	VJSGlobalClass::AddStaticFunction( "getDataStore", VJSGlobalClass::js_callStaticFunction<do_GetBase>, JS4D::PropertyAttributeNone);

	VJSGlobalClass::AddStaticFunction( "getCacheManager", VJSGlobalClass::js_callStaticFunction<do_GetCacheManager>, JS4D::PropertyAttributeNone);
}


//======================================================







