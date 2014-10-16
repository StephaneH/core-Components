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
	CDB4DBase* base = xem->GetCatalog()->GetAssocBase()->RetainBaseX();
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
	if (inBase != NULL)
	{
		CDB4DContext *db4dContext = GetDB4DContextFromJSContext( inContext);
		if (db4dContext != NULL)
			return dynamic_cast<VDB4DContext*>(db4dContext)->RetainDataBaseContext( inBase, true, false);
	}

	return NULL;
}



BaseTaskInfo *GetBaseTaskInfoFromJSContext( const XBOX::VJSContext& inContext, EntityModelCatalog* inCatalog)
{
	return GetBaseTaskInfoFromJSContext(inContext, inCatalog->GetAssocBase());
}


BaseTaskInfo *GetBaseTaskInfoFromJSContext( const XBOX::VJSContext& inContext, EntityModel* inModel)
{
	return GetBaseTaskInfoFromJSContext(inContext, inModel->GetCatalog());
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


JSCollectionManager::JSCollectionManager(JS4D::ContextRef inContext, bool simpleDate) : fContextRef(inContext)
{
	fSize = 0;
	fSimpleDate = simpleDate;
}


JSCollectionManager::~JSCollectionManager()
{
	for (vector<VJSArray>::iterator cur = fValues.begin(), end = fValues.end(); cur != end; cur++)
	{
		cur->Unprotect();
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
	outValue = val.CreateVValue(fSimpleDate);
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
		fValues[i].Protect();
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
		ioParms.ReturnValue(VJSField::CreateInstance(ioParms.GetContext(), ff));
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
		em->Release();
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
			VJSValue emVal = VJSEntityModel::CreateInstance(globalObject.GetContext(), em);
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
			ioParms.ReturnValue(VJSEntityModel::CreateInstance(ioParms.GetContext(), em));
			em->Release();
		}
		else
		{
			/*
			EntityModel* em = inDatabase->RetainEntityModelBySingleEntityName(propname);
			if (em != nil)
			{
				ioParms.ReturnValue(VJSEntityModel::CreateInstance(ioParms.GetContext(), em));
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
		VJSContext	vjsContext(ioParms.GetContext());
		err = catalog->SaveEntityModels(*outBag, true, true);
		if (err == VE_OK)
		{
			VString s;
			err = outBag->GetJSONString(s);
			if (err == VE_OK)
			{
				VJSJSON		json(vjsContext);
				VJSValue	retVal(vjsContext);
				json.Parse(retVal,s);
				ioParms.ReturnValue(retVal);
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
				ioParms.ReturnValue(VJSTable::CreateInstance(ioParms.GetContext(), table));
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
		ioParms.ReturnValue(VJSDatabase::CreateInstance(ioParms.GetContext(), structure));
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
		ioParms.ReturnValue(VJSDatabase::CreateInstance(ioParms.GetContext(), structure));
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


typedef enum { exportAs_SQL, exportAs_Binary, exportAs_JSON } exportAs_type;


void __import(VJSParms_callStaticFunction& ioParms, Base4D* inDatabase, exportAs_type exportType)
{
	VError err = VE_OK;
	VFolder* folder = ioParms.RetainFolderParam( 1);
	if (folder != nil)
	{
		ExportOption options;
		if (exportType == exportAs_JSON)
			options.JSONExport = true;
		else if (exportType == exportAs_Binary)
			options.BinaryExport = true;
		options.CreateFolder = false;
		err = inDatabase->ImportRecords( GetBaseTaskInfoFromJSContext(ioParms, inDatabase), folder, nil, options);
		folder->Release();
	}
}

void VJSDatabase::_ImportFromJSON(VJSParms_callStaticFunction& ioParms, Base4D* inDatabase)
{
	__import(ioParms, inDatabase, exportAs_JSON);
}


void __export(VJSParms_callStaticFunction& ioParms, Base4D* inDatabase, exportAs_type exportType)
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
		if (exportType == exportAs_JSON)
			options.JSONExport = true;
		else if (exportType == exportAs_Binary)
			options.BinaryExport = true;

		sLONG blobthreshold = 0;
		ioParms.GetLongParam(4, &blobthreshold);
		options.BlobThresholdSize = blobthreshold;

		options.CreateFolder = false;
		options.NbBlobsPerLevel = nbBlobsPerLevel;
		options.MaxSQLTextSize = maxSQLFileSize;
		err = inDatabase->ExportToSQL( GetBaseTaskInfoFromJSContext(ioParms, inDatabase), folder, nil, options);
		folder->Release();
	}
}

void VJSDatabase::_ExportAsSQL(VJSParms_callStaticFunction& ioParms, Base4D* inDatabase)
{
	__export(ioParms, inDatabase, exportAs_SQL);
}


void VJSDatabase::_ExportAsJSON(VJSParms_callStaticFunction& ioParms, Base4D* inDatabase)
{
	__export(ioParms, inDatabase, exportAs_JSON);
}


void VJSDatabase::_clearErrs(VJSParms_callStaticFunction& ioParms, Base4D* inDatabase)
{
	XBOX::VTask::GetCurrent()->FlushErrors();
}


void VJSDatabase::_getTables( XBOX::VJSParms_getProperty& ioParms, Base4D* inDatabase)
{
	ioParms.ReturnValue(VJSDatabaseTableEnumerator::CreateInstance(ioParms.GetContext(), inDatabase));

}


void VJSDatabase::_getEntityModels( XBOX::VJSParms_getProperty& ioParms, Base4D* inDatabase)
{
	ioParms.ReturnValue(VJSDatabaseEMEnumerator::CreateInstance(ioParms.GetContext(), inDatabase));

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
	VJSObject result(ioParms.GetContext());
	result.MakeEmpty();
	VJSValue jsval(ioParms.GetContext());
	jsval.SetNumber(stats.fUsedMem);
	result.SetProperty(L"usedMem", jsval, JS4D::PropertyAttributeNone);
	jsval.SetNumber(stats.fFreeMem);
	result.SetProperty(L"freeMem", jsval, JS4D::PropertyAttributeNone);
	VJSArray objarr(ioParms.GetContext());
	for( VMapOfObjectInfo::const_iterator i = stats.fObjectInfo.begin() ; i != stats.fObjectInfo.end() ; ++i)
	{
		const std::type_info* typobj = i->first;
		VStr255 s;
		VSystem::DemangleSymbol( typobj->name(), s);
		VJSObject elem(ioParms.GetContext());
		elem.MakeEmpty();
		VJSValue elemval(ioParms.GetContext());
		elemval.SetString(s);
		elem.SetProperty("id", elemval, JS4D::PropertyAttributeNone);
		elemval.SetNumber(i->second.fCount);
		elem.SetProperty("count", elemval, JS4D::PropertyAttributeNone);
		elemval.SetNumber(i->second.fTotalSize);
		elem.SetProperty("size", elemval, JS4D::PropertyAttributeNone);
		objarr.PushValue(elem);
	}
	result.SetProperty("objects", objarr, JS4D::PropertyAttributeNone);

	VJSArray blockarr(ioParms.GetContext());
	for( VMapOfBlockInfo::const_iterator i = stats.fBlockInfo.begin() ; i != stats.fBlockInfo.end() ; ++i)
	{
		VStr4 s;
		s.FromOsType( i->first);
		VJSObject elem(ioParms.GetContext());
		elem.MakeEmpty();
		VJSValue elemval(ioParms.GetContext());
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
	VJSArray arr(ioParms.GetContext());
	for (sLONG i = 1, nb = VDBMgr::GetManager()->CountBases(); i <= nb; i++)
	{
		Base4D* base = VDBMgr::GetManager()->RetainNthBase(i);
		if (base != nil)
		{
			arr.PushValue(VJSDatabase::CreateInstance(ioParms.GetContext(), base));
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
	VJSArray arr(ioParms.GetContext());
	inDatabase->GetIndices(arr);
	ioParms.ReturnValue(arr);
}


/*void VJSDatabase::_fixForV4(XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase)
{
	inDatabase->_fixForWakandaV4();
}
*/


void VJSDatabase::_resurectGhostTables(XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase)
{
	inDatabase->ResurectGhostTables();
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
				ioParms.ReturnValue(VJSSyncEvent::CreateInstance(ioParms.GetContext(), sync));
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
		JSTOOLSIntf(JS4D::ContextRef jscontext, VJSObject& paramObj):
			fJsContext(jscontext),
			fAddProblemFunc(fJsContext),
			fOpenProgressionFunc(fJsContext), 
			fCloseProgressionFunc(fJsContext),
			fProgressFunc(fJsContext), fSetProgressTitleFunc(fJsContext), fParamObj(fJsContext)
		{
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
			if (fAddProblemFunc.HasRef())
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

				VJSException excep;
				VJSValue result(fJsContext);
				vector<VJSValue> params;
				params.push_back(problem);
				fParamObj.CallFunction(fAddProblemFunc, &params, &result, excep);
			}
			return err;
		}


		virtual VErrorDB4D OpenProgression(const VString inProgressTitle, sLONG8 inMaxElems)  // inMaxElems == -1 means non linear progression
		{
			VError err = VE_OK;
			if (fOpenProgressionFunc.HasRef())
			{
				VJSException excep;
				VJSValue result(fJsContext);
				vector<VJSValue> params;
				VJSValue p1(fJsContext);
				p1.SetString(inProgressTitle);
				VJSValue p2(fJsContext);
				p2.SetNumber(inMaxElems);
				params.push_back(p1);
				params.push_back(p2);
				fParamObj.CallFunction(fOpenProgressionFunc, &params, &result, excep);
			}
			return err;
		}


		virtual VErrorDB4D CloseProgression()  // OpenProgression can be called in nested levels
		{
			VError err = VE_OK;
			if (fCloseProgressionFunc.HasRef())
			{
				VJSException excep;
				VJSValue result(fJsContext);
				vector<VJSValue> params;
				fParamObj.CallFunction(fCloseProgressionFunc, &params, &result, excep);
			}
			return err;
		}


		virtual VErrorDB4D Progress(sLONG8 inCurrentValue, sLONG8 inMaxElems)
		{
			VError err = VE_OK;
			if (fProgressFunc.HasRef())
			{
				VJSException excep;
				VJSValue result(fJsContext);
				vector<VJSValue> params;
				VJSValue p1(fJsContext);
				p1.SetNumber(inCurrentValue);
				VJSValue p2(fJsContext);
				p2.SetNumber(inMaxElems);
				params.push_back(p1);
				params.push_back(p2);
				fParamObj.CallFunction(fProgressFunc, &params, &result, excep);
			}
			return err;
		}


		virtual VErrorDB4D SetProgressTitle(const VString inProgressTitle)
		{
			VError err = VE_OK;
			if (fSetProgressTitleFunc.HasRef())
			{
				VJSException excep;
				VJSValue result(fJsContext);
				vector<VJSValue> params;
				VJSValue p1(fJsContext);
				p1.SetString(inProgressTitle);
				params.push_back(p1);
				fParamObj.CallFunction(fSetProgressTitleFunc, &params, &result, excep);
			}
			return err;
		}



	protected:
		VJSContext fJsContext;
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
			VJSObject settingsObj(ioParms.GetContext());
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
	VJSObject jsBackupProgressObs(ioParms.GetContext());
	XBOX::VFilePath journalPath;
	if (ioParms.IsObjectParam(2))
	{
		ioParms.GetParamObject(2,jsBackupProgressObs);
	}
	else
	{
		jsBackupProgressObs.MakeEmpty();
	}
	JSTOOLSIntf backupToolInterface(ioParms.GetContext(), jsBackupProgressObs);

	//Ensure a valid journal file is specified
	if (ioParms.IsObjectParam(1))
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
	VJSObject jsBackupProgressObs(ioParms.GetContext());
	IBackupSettings* workingBackupSettings = NULL;

	//Optional object which will contain the actual backup config used, for debug
	bool dryRun = false;
	VJSObject dryRunConfigObject (ioParms.GetContext());
	
	

	CDB4DBase* inDatabase = base->RetainBaseX();
	context = GetDB4DBaseContextFromJSContext(ioParms, inDatabase);
	
	//Command syntax:
	//ds.backup([config: object] [,options: object])
	//examples:
	//ds.backup(null,{some progress object});                         -> uses default backup config from database and a progress indicator
	//ds.backup({destination:Folder(SomePath),useUniqueNames:true});  -> uses custom config and no progress options
	//ds.backup();                                                    -> uses default backup config from database and no progress options

	//Obtain backup progress callbacks from options parameter if applicable
	if (ioParms.IsObjectParam(2))
	{
		ioParms.GetParamObject(2,jsBackupProgressObs);
	}
	else
	{
		jsBackupProgressObs.MakeEmpty();
	}
	JSTOOLSIntf backupToolInterface(ioParms.GetContext(), jsBackupProgressObs);

	//Create working settings initialized to the database backup settings which are our default settings
	const IBackupSettings* defaultSettings = inDatabase->RetainBackupSettings();
	workingBackupSettings = defaultSettings->Clone();
	XBOX::ReleaseRefCountable(&defaultSettings);

	//Check if param 1 is there because it can be null if param 2 is defined
	if (ioParms.IsObjectParam(1))
	{
		//First parameter is specified as JS object, parse it as backup settings
		//and override the working backups ettings with that

		VJSObject altBackupConfig(ioParms.GetContext());
		ioParms.GetParamObject(1,altBackupConfig);

		if (altBackupConfig.GetPropertyAsBool(CVSTR("dryRunUnitTest"),NULL,NULL))
		{
			//Testing, we won't perform backup, only the configuration subsitution
			dryRun = true;
			dryRunConfigObject.MakeEmpty();
		}

		error = workingBackupSettings->FromJSObject(altBackupConfig);

		if(error != VE_OK)
		{
			//The new configuration is not valid so clear it we'll use a default one
			XBOX::ReleaseRefCountable(&workingBackupSettings);
		}
	}
	else
	{
		error = workingBackupSettings->CheckValidity();
		if (error != VE_OK)
		{
			XBOX::ReleaseRefCountable(&workingBackupSettings);
		}
	}

	bool backupSucceeded = false;
	VFilePath manifestPath;
	
	if (errorContext.GetLastError() == VE_OK)
	{
		if (workingBackupSettings != NULL)
		{
			if (!dryRun)
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
				backupSucceeded = true;
				workingBackupSettings->ToJSObject(dryRunConfigObject);
			}
		}
		else
		{
			error = vThrowError(VE_INVALID_BACKUP_SETTINGS);
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
		if (!dryRun)
		{
		xbox_assert(!manifestPath.IsEmpty() && manifestPath.IsValid() && manifestPath.IsFile());
		ioParms.ReturnFilePathAsFileOrFolder(manifestPath);
	}
	else
	{
			ioParms.ReturnValue(dryRunConfigObject);
		}
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
	VJSObject paramObj(ioParms.GetContext());
	if (ioParms.IsObjectParam(1))
	{
		ioParms.GetParamObject(1, paramObj);
	}
	else
		paramObj.MakeEmpty();

	JSTOOLSIntf toolintf(ioParms.GetContext(), paramObj);
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
		VJSObject options(ioParms.GetContext());
		ioParms.GetParamObject(1, options);
		bool exists;
		bool queryplan = options.GetPropertyAsBool("queryPlan", nil, &exists);
		bool querypath = options.GetPropertyAsBool("queryPath", nil, &exists);
		context->DescribeQueryExecution(querypath);
	}
}

void VJSDatabase::_setLogFile( XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase)
{
	EntityModelCatalog* cat = inDatabase->GetEntityCatalog(true);
	if (cat != nil)
	{
		VFile* file = ioParms.RetainFileParam(1, true);
		if (file != nil)
		{
			VJSONObject* options = nil;
			bool append = ioParms.GetBoolParam(2, "append", "clear");
			if (ioParms.IsObjectParam(3))
			{
				VJSObject obj = ioParms.GetObject();
				VJSJSON json(ioParms.GetContext());
				VString s;
				json.Stringify(obj, s);
				VJSONValue val;
				val.ParseFromString(s);
				options = RetainRefCountable(val.GetObject());
			}
			VError err = cat->SetLogFile(file, append, options);
			if (err == VE_OK)
				err = cat->StartLogging();
		}
		else
			vThrowError(VE_JVSC_WRONG_PARAMETER_TYPE_FILE, "1");
	}
}


void VJSDatabase::_startLogging( XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase)
{
	EntityModelCatalog* cat = inDatabase->GetEntityCatalog(true);
	if (cat != nil)
	{
		cat->StartLogging();
	}
}

void VJSDatabase::_stopLogging( XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase)
{
	EntityModelCatalog* cat = inDatabase->GetEntityCatalog(true);
	if (cat != nil)
	{
		cat->StopLogging();
	}
}


void VJSDatabase::_flushLog( XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase)
{
	EntityModelCatalog* cat = inDatabase->GetEntityCatalog(true);
	if (cat != nil)
	{
		cat->FlushLog();
	}
}

void VJSDatabase::_getStats( XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase)
{
	VJSContext	vjsContext(ioParms.GetContext());
	VJSJSON json(vjsContext);
	VJSONObject* options = nil;
	if (ioParms.IsObjectParam(1))
	{
		VJSObject objparam(vjsContext);
		ioParms.GetParamObject(1, objparam);
		VString s;
		json.Stringify(objparam, s);
		VJSONValue val;
		val.ParseFromString(s);
		if (val.IsObject())
			options = RetainRefCountable(val.GetObject());

	}
	JSONPath jsonpath;
	JSONPath* xpath = nil;
	if (options != nil)
	{
		VString path;
		if (options->GetPropertyAsString("path", path) && !path.IsEmpty())
		{
			jsonpath.FromString(path);
			xpath = &jsonpath;
		}
	}
	VJSONObject* stats = inDatabase->RetainMeasures(options, xpath); 
	if (stats != nil)
	{
		VString s;
		VJSONValue(stats).Stringify(s);
		stats->Release();
		VJSJSON		json(vjsContext);
		VJSValue	retVal(vjsContext);
		json.Parse(retVal,s);
		ioParms.ReturnValue(retVal);
	}
}

void VJSDatabase::_setStatsInterval( XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase)
{
	uLONG interval;
	if (ioParms.IsNumberParam(1))
	{
		sLONG ll = 0;
		ioParms.GetLongParam(1, &ll);
		interval = ll;
		inDatabase->SetMeasureInterval(interval);
	}
}


void VJSDatabase::_requireModel(XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase)
{
	EntityModelCatalog* cat = inDatabase->GetEntityCatalog(true);
	if (cat != nil)
	{
		VError err = VE_OK;
		VFile* modelfile = ioParms.RetainFileParam(1);
		if (modelfile != nil)
		{
			EntityModelCatalog* subcat = cat->AddSubProject(modelfile, err);
			if (subcat != nil)
			{
				vector<VFile*> files;
				BaseTaskInfo* context = GetBaseTaskInfoFromJSContext(ioParms, inDatabase);
				subcat->EvaluateModelScript(ioParms.GetContext().GetGlobalObject(), context, nil, files);
				for (vector<VFile*>::iterator cur = files.begin(), end = files.end(); cur != end; ++cur)
				{
					(*cur)->Release();
				}
				ioParms.ReturnValue(VJSEntityModelCatalog::CreateInstance(ioParms.GetContext(), subcat));
			}
			modelfile->Release();
		}
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
		{ "exportAsJSON", js_callStaticFunction<_ExportAsJSON>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "importFromJSON", js_callStaticFunction<_ImportFromJSON>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
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
		{ "setLogFile", js_callStaticFunction<_setLogFile>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "startLogging", js_callStaticFunction<_startLogging>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "stopLogging", js_callStaticFunction<_stopLogging>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "flushLog", js_callStaticFunction<_flushLog>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "getMeasures", js_callStaticFunction<_getStats>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "setMeasureInterval", js_callStaticFunction<_setStatsInterval>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "revealGhostTables", js_callStaticFunction<_resurectGhostTables>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "requireModel", js_callStaticFunction<_requireModel>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },


		//{ "fixForV4", js_callStaticFunction<_fixForV4>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
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


VJSObject VJSDatabase::CreateInstance( const VJSContext& inContext, Base4D *inDatabase)
{
	return inherited::CreateInstance( inContext, inDatabase);
}


//======================================================


void VJSEntityModelCatalog::Initialize( const VJSParms_initialize& inParms, EntityModelCatalog* inCatalog)
{
	inCatalog->Retain();
}


void VJSEntityModelCatalog::Finalize( const VJSParms_finalize& inParms, EntityModelCatalog* inCatalog)
{
	inCatalog->Release();
}



void VJSEntityModelCatalog::GetPropertyNames( VJSParms_getPropertyNames& ioParms, EntityModelCatalog* inCatalog)
{
	//set<VString> dejaName;
	//vector<EntityModel*> entities;
	vector<VRefPtr<EntityModel> > entities;
	inCatalog->RetainAllEntityModels(entities);
	for (vector<VRefPtr<EntityModel> >::iterator cur = entities.begin(), end = entities.end(); cur != end; cur++)
	{
		EntityModel* em = *cur;
		ioParms.AddPropertyName(em->GetEntityName());
	}
}


void VJSEntityModelCatalog::GetProperty( VJSParms_getProperty& ioParms, EntityModelCatalog* inCatalog)
{
	EntityModel* em = nil;
	
	VString propname;
	{
		ioParms.GetPropertyName(propname);
		EntityModel* em = inCatalog->RetainEntity(propname);
		if (em != nil)
		{
			ioParms.ReturnValue(VJSEntityModel::CreateInstance(ioParms.GetContext(), em));
			em->Release();
		}
	}
	
}



void VJSEntityModelCatalog::_GetName(XBOX::VJSParms_callStaticFunction& ioParms, EntityModelCatalog* inCatalog)
{
	VString s;
	inCatalog->GetName(s);
	ioParms.ReturnString(s);
}

void VJSEntityModelCatalog::_StartTransaction(XBOX::VJSParms_callStaticFunction& ioParms, EntityModelCatalog* inCatalog)
{
	BaseTaskInfo* context = GetBaseTaskInfoFromJSContext(ioParms, inCatalog);
	if (context != nil)
	{
		VError err = inCatalog->StartTransaction(context);
	}
}

void VJSEntityModelCatalog::_Commit(XBOX::VJSParms_callStaticFunction& ioParms, EntityModelCatalog* inCatalog)
{
	BaseTaskInfo* context = GetBaseTaskInfoFromJSContext(ioParms, inCatalog);
	if (context != nil)
	{
		VError err = inCatalog->CommitTransaction(context);
	}
}

void VJSEntityModelCatalog::_RollBack(XBOX::VJSParms_callStaticFunction& ioParms, EntityModelCatalog* inCatalog)
{
	BaseTaskInfo* context = GetBaseTaskInfoFromJSContext(ioParms, inCatalog);
	if (context != nil)
	{
		VError err = inCatalog->RollBackTransaction(context);
	}
}

void VJSEntityModelCatalog::_getIndices(XBOX::VJSParms_callStaticFunction& ioParms, EntityModelCatalog* inCatalog)
{
}

void VJSEntityModelCatalog::_close(XBOX::VJSParms_callStaticFunction& ioParms, EntityModelCatalog* inCatalog)
{
}


void VJSEntityModelCatalog::_setLogFile(XBOX::VJSParms_callStaticFunction& ioParms, EntityModelCatalog* inCatalog)
{
	EntityModelCatalog* cat = inCatalog;
	if (cat != nil)
	{
		VFile* file = ioParms.RetainFileParam(1, true);
		if (file != nil)
		{
			VJSONObject* options = nil;
			bool append = ioParms.GetBoolParam(2, "append", "clear");
			if (ioParms.IsObjectParam(3))
			{
				VJSObject obj = ioParms.GetObject();
				VJSJSON json(ioParms.GetContext());
				VString s;
				json.Stringify(obj, s);
				VJSONValue val;
				val.ParseFromString(s);
				options = RetainRefCountable(val.GetObject());
			}
			VError err = cat->SetLogFile(file, append, options);
			if (err == VE_OK)
				err = cat->StartLogging();
		}
		else
			vThrowError(VE_JVSC_WRONG_PARAMETER_TYPE_FILE, "1");
	}
}

void VJSEntityModelCatalog::_startLogging(XBOX::VJSParms_callStaticFunction& ioParms, EntityModelCatalog* inCatalog)
{
	inCatalog->StartLogging();
}

void VJSEntityModelCatalog::_stopLogging(XBOX::VJSParms_callStaticFunction& ioParms, EntityModelCatalog* inCatalog)
{
	inCatalog->StopLogging();
}


void VJSEntityModelCatalog::_flushLog(XBOX::VJSParms_callStaticFunction& ioParms, EntityModelCatalog* inCatalog)
{
	inCatalog->FlushLog();
}


void VJSEntityModelCatalog::_queryOptions(XBOX::VJSParms_callStaticFunction& ioParms, EntityModelCatalog* inCatalog)
{
}


void VJSEntityModelCatalog::_getModelDefinition(XBOX::VJSParms_callStaticFunction& ioParms, EntityModelCatalog* inCatalog)
{
}


void VJSEntityModelCatalog::_getEntityModels( XBOX::VJSParms_getProperty& ioParms, EntityModelCatalog* inCatalog)
{
	ioParms.ReturnValue(VJSCatalogEMEnumerator::CreateInstance(ioParms.GetContext(), inCatalog));
}




void VJSEntityModelCatalog::GetDefinition( ClassDefinition& outDefinition)
{
	static inherited::StaticFunction functions[] = 
	{
		{ "getName", js_callStaticFunction<_GetName>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "startTransaction", js_callStaticFunction<_StartTransaction>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "commit", js_callStaticFunction<_Commit>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "rollBack", js_callStaticFunction<_RollBack>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
//		{ "transactionLevel", js_callStaticFunction<_TransactionLevel>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "getModelDefinition", js_callStaticFunction<_getModelDefinition>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "getIndices", js_callStaticFunction<_getIndices>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "queryOptions", js_callStaticFunction<_queryOptions>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "close", js_callStaticFunction<_close>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "setLogFile", js_callStaticFunction<_setLogFile>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "startLogging", js_callStaticFunction<_startLogging>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "stopLogging", js_callStaticFunction<_stopLogging>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "flushLog", js_callStaticFunction<_flushLog>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },

		{ 0, 0, 0}
	};

	static inherited::StaticValue values[] = 
	{
		{ "dataClasses", js_getProperty<_getEntityModels>, nil, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ 0, 0, 0, 0}
	};

	outDefinition.className = "OutsideDatastore";
	outDefinition.initialize = js_initialize<Initialize>;
	outDefinition.finalize = js_finalize<Finalize>;
	outDefinition.getProperty = js_getProperty<GetProperty>;
	outDefinition.getPropertyNames = js_getPropertyNames<GetPropertyNames>;
	outDefinition.staticFunctions = functions;
	outDefinition.staticValues = values;
}



VJSObject VJSEntityModelCatalog::CreateInstance( const VJSContext& inContext, EntityModelCatalog* inCatalog)
{
	return inherited::CreateInstance( inContext, inCatalog);
}


//======================================================



void VJSCatalogEMEnumerator::Initialize( const VJSParms_initialize& inParms, EntityModelCatalog* inCatalog)
{
	inCatalog->Retain();
}


void VJSCatalogEMEnumerator::Finalize( const VJSParms_finalize& inParms, EntityModelCatalog* inCatalog)
{
	inCatalog->Release();
}

void VJSCatalogEMEnumerator::GetPropertyNames( VJSParms_getPropertyNames& ioParms, EntityModelCatalog* inCatalog)
{
	vector<VRefPtr<EntityModel> > entities;
	inCatalog->RetainAllEntityModels(entities);
	for (vector<VRefPtr<EntityModel> >::iterator cur = entities.begin(), end = entities.end(); cur != end; cur++)
	{
		EntityModel* em = *cur;
		ioParms.AddPropertyName(em->GetEntityName());
	}
}


void VJSCatalogEMEnumerator::GetProperty( VJSParms_getProperty& ioParms, EntityModelCatalog* inCatalog)
{
	VString propname;

	ioParms.GetPropertyName(propname);
	EntityModel* em = inCatalog->RetainEntity(propname);
	if (em != nil)
	{
		ioParms.ReturnValue(VJSEntityModel::CreateInstance(ioParms.GetContext(), em));
		em->Release();
	}
}



void VJSCatalogEMEnumerator::GetDefinition( ClassDefinition& outDefinition)
{
	outDefinition.className = "CatalogDataClassEnumerator";
	outDefinition.initialize = js_initialize<Initialize>;
	outDefinition.finalize = js_finalize<Finalize>;
	outDefinition.getProperty = js_getProperty<GetProperty>;
	outDefinition.getPropertyNames = js_getPropertyNames<GetPropertyNames>;
}


VJSObject VJSCatalogEMEnumerator::CreateInstance( const VJSContext& inContext, EntityModelCatalog* inCatalog)
{
	return inherited::CreateInstance( inContext, inCatalog);
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
			ioParms.ReturnValue(VJSTable::CreateInstance(ioParms.GetContext(), tt));
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


VJSObject VJSDatabaseTableEnumerator::CreateInstance( const VJSContext& inContext, Base4D *inDatabase)
{
	return inherited::CreateInstance( inContext, inDatabase);
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
		ioParms.ReturnValue(VJSEntityModel::CreateInstance(ioParms.GetContext(), em));
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


VJSObject VJSDatabaseEMEnumerator::CreateInstance( const VJSContext& inContext, Base4D *inDatabase)
{
	return inherited::CreateInstance( inContext, inDatabase);
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
		ioParms.ReturnValue(VJSEntityAttribute::CreateInstance(ioParms.GetContext(), att));
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
		ioParms.ReturnValue(VJSEntityAttribute::CreateInstance(ioParms.GetContext(), att));
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
				VJSObject localObjFunc(ioParms.GetContext());
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
			VJSObject objParam(ioParms.GetContext());
			ioParms.GetParamObject(1, objParam);
			BaseTaskInfo* context = GetBaseTaskInfoFromJSContext(ioParms, inModel);
			
			EntityCollection* sel = inModel->executeQuery(objParam, context, nil, nil, DB4D_Do_Not_Lock, 0, nil, &err);
			if (sel != nil)
			{
				EntitySelectionIterator* itersel = new EntitySelectionIterator(sel, false, true, context, inModel);
				itersel->First(context);
				if (itersel->GetCurRec(context) != nil)
				{
					ioParms.ReturnValue(VJSEntitySelectionIterator::CreateInstance(ioParms.GetContext(), itersel));
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
				ioParms.ReturnValue(VJSEntitySelectionIterator::CreateInstance(ioParms.GetContext(), iter));
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
			VJSObject obj(ioParms.GetContext());
			ioParms.GetParamObject(1, obj);
			err = erec->convertFromJSObj(obj);
		}

		EntitySelectionIterator* iter = new EntitySelectionIterator( erec, context);
		ioParms.ReturnConstructedObject(VJSEntitySelectionIterator::CreateInstance(ioParms.GetContext(), iter));
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
	LocalEntityModel* localModel = dynamic_cast<LocalEntityModel*>(inModel);
	if (localModel != nil)
	{
		Base4D* owner = localModel->GetDB();
		if (owner != nil)
			ioParms.ReturnValue(VJSDatabase::CreateInstance(ioParms.GetContext(), owner));
	}
	else
	{
		ioParms.ReturnValue(VJSEntityModelCatalog::CreateInstance(ioParms.GetContext(), inModel->GetCatalog()));
	}
}


void VJSEntityModel::_AllEntities(VJSParms_callStaticFunction& ioParms, EntityModel* inModel)
{
	VError err = VE_OK;
	EntityCollection* sel = inModel->SelectAllEntities(GetBaseTaskInfoFromJSContext(ioParms, inModel), &err);
	if (sel != nil)
	{
		ioParms.ReturnValue(VJSEntitySelection::CreateInstance(ioParms.GetContext(), sel));
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
			ioParms.ReturnValue(VJSEntitySelection::CreateInstance(ioParms.GetContext(), sel));
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
						ioParms.ReturnValue(VJSEntitySelection::CreateInstance(ioParms.GetContext(), sel));
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
				ioParms.ReturnValue(VJSEntitySelectionIterator::CreateInstance(ioParms.GetContext(), itersel));
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
						ioParms.ReturnValue(VJSEntitySelectionIterator::CreateInstance(ioParms.GetContext(), itersel));
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
		ioParms.ReturnValue(VJSEntitySelectionIterator::CreateInstance(ioParms.GetContext(), iter));
		rec->Release();
	}
}


void VJSEntityModel::_NewSelection(VJSParms_callStaticFunction& ioParms, EntityModel* inModel)
{
	bool keepsorted = ioParms.GetBoolParam( 1, L"KeepSorted", L"AnyOrder");
	EntityCollection* sel = inModel->NewCollection(keepsorted);
	if (sel != nil)
	{
		ioParms.ReturnValue(VJSEntitySelection::CreateInstance(ioParms.GetContext(), sel));
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


void VJSEntityModel::_exportAsSQL(VJSParms_callStaticFunction& ioParms, EntityModel* inModel)
{
	VError err = VE_OK;
	EntityCollection* sel = inModel->SelectAllEntities(GetBaseTaskInfoFromJSContext(ioParms, inModel), &err);
	if (sel != nil)
	{
		VJSEntitySelection::_exportAsSQL(ioParms, sel);
		sel->Release();
	}
}

void VJSEntityModel::_exportAsJSON(VJSParms_callStaticFunction& ioParms, EntityModel* inModel)
{
	VError err = VE_OK;
	EntityCollection* sel = inModel->SelectAllEntities(GetBaseTaskInfoFromJSContext(ioParms, inModel), &err);
	if (sel != nil)
	{
		VJSEntitySelection::_exportAsJSON(ioParms, sel);
		sel->Release();
	}
}

void __importTable(VJSParms_callStaticFunction& ioParms, EntityModel* inModel, exportAs_type exportType)
{
	VError err = VE_OK;
	VFolder* folder = ioParms.RetainFolderParam( 1);
	if (folder != nil)
	{
		ExportOption options;
		if (exportType == exportAs_JSON)
			options.JSONExport = true;
		else if (exportType == exportAs_Binary)
			options.BinaryExport = true;
		options.CreateFolder = false;
		LocalEntityModel* localmodel = dynamic_cast<LocalEntityModel*>(inModel);
		if (localmodel != nil)
		{
			if (localmodel->GetMainTable() != nil)
				err = localmodel->GetMainTable()->ImportRecords(folder, GetBaseTaskInfoFromJSContext(ioParms, inModel), nil, options);
			else
				err = ThrowBaseError(VE_UNKNOWN_ERROR);
		}
		else
			err = ThrowBaseError(VE_UNKNOWN_ERROR);
		folder->Release();
	}
}


void VJSEntityModel::_ImportFromJSON(VJSParms_callStaticFunction& ioParms, EntityModel* inModel)
{
	__importTable(ioParms, inModel, exportAs_JSON);
}


void VJSEntityModel::_allowAttribute(VJSParms_callStaticFunction& ioParms, EntityModel* inModel)
{
	BaseTaskInfo* context = GetBaseTaskInfoFromJSContext(ioParms, inModel);
	VString permStr;
	DB4D_EM_Perm perm = DB4D_EM_None_Perm;
	ioParms.GetStringParam(2, permStr);
	if (permStr == "read")
		perm = DB4D_EM_Read_Perm;
	else if (permStr == "update")
		perm = DB4D_EM_Update_Perm;
	else if (permStr == "create")
		perm = DB4D_EM_Create_Perm;

	EntityAttribute* att = nil;
	if (ioParms.IsStringParam(1))
	{
		VString s;
		ioParms.GetStringParam(1, s);
		att = inModel->getAttribute(s);
	}
	else
	{
		att = ioParms.GetParamObjectPrivateData<VJSEntityAttribute>(1);
	}

	if (att == nil)
	{
		vThrowError(VE_JVSC_WRONG_PARAMETER_TYPE_ATTRIBUTE, "1");
	}
	else
	{
		if (perm != DB4D_EM_None_Perm)
		{
			ioParms.ReturnBool(att->permissionMatch(perm, context));
		}
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
			VJSArray arr(ioParms.GetContext(), nil,  true);
			ioParms.GetParamArray(2, arr);
			if (ioParms.IsObjectParam(3))
			{
				VJSObject thisobj(ioParms.GetContext());
				ioParms.GetParamObject(3, thisobj);
				VError err = VE_OK;
				ioParms.ReturnValue(inModel->call_Method(funcname, arr, thisobj, basecontext, ioParms.GetContext(), err));
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
		VJSArray arr(ioParms.GetContext(), nil,  true);
		ioParms.GetParamArray(1, arr);

		EntityCollection* xsel = inModel->FromArray(arr, basecontext, err, nil);

		if (xsel != nil)
		{
			okresult = true;
			ioParms.ReturnValue(VJSEntitySelection::CreateInstance(ioParms.GetContext(), xsel));
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
	ioParms.ReturnValue(VJSEntityAttributeEnumerator::CreateInstance(ioParms.GetContext(), inModel));
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

		{ "exportAsSQL", js_callStaticFunction<_exportAsSQL>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "exportAsJSON", js_callStaticFunction<_exportAsJSON>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "importFromJSON", js_callStaticFunction<_ImportFromJSON>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },

		{ "allowAttribute", js_callStaticFunction<_allowAttribute>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },

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


void VJSEntityAttribute::_getUUID( XBOX::VJSParms_getProperty& ioParms, EntityAttribute* inAttribute)
{
	VString s;
	VUUID id;
	id = inAttribute->GetUUID();
	if (id.IsNull())
		ioParms.ReturnNullValue();
	else
	{
		id.GetString(s);
		ioParms.ReturnString(s);
	}
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
		ioParms.ReturnValue(VJSEntityModel::CreateInstance(ioParms.GetContext(), res));
}


void VJSEntityAttribute::_getRelatedDataClass( XBOX::VJSParms_getProperty& ioParms, EntityAttribute* inAttribute)
{
	EntityModel* res = inAttribute->GetSubEntityModel();
	if (res == nil)
		ioParms.ReturnNullValue();
	else
		ioParms.ReturnValue(VJSEntityModel::CreateInstance(ioParms.GetContext(), res));
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
		{ "uuid", js_getProperty<_getUUID>, nil, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontDelete },
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
							bool simpleDate = false;
							VValueSingle* cv = emval->GetVValue();
							if (cv != nil)
							{
								ioParms.ReturnVValue(*cv, att->isSimpleDate());
							}
						}
						break;
						
					case eav_subentity:
						{
							EntityRecord* subrec = emval->getRelatedEntity();
							if (subrec != nil)
							{
								EntitySelectionIterator* subiter = new EntitySelectionIterator(subrec, context);
								ioParms.ReturnValue(VJSEntitySelectionIterator::CreateInstance(ioParms.GetContext(), subiter));
							}
						}
						break;
						
					case eav_selOfSubentity:
						{
							EntityCollection* sel = emval->getRelatedSelection();
							if (sel != nil)
							{
								ioParms.ReturnValue(VJSEntitySelection::CreateInstance(ioParms.GetContext(), sel));								
							}
						}
						break;

					case eav_composition:
						{
							/*
							VString jsonvalue;
							emval->GetJsonString(jsonvalue);
							VJSJSON json(ioParms.GetContext());
							VJSValue result(json.Parse(jsonvalue));
							ioParms.ReturnValue(result);
							*/
							VJSObject result(ioParms.GetContext());
							err = emval->GetJSObject(result);
							if (err != VE_OK)
								result.ClearRef();
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
			VJSObject localObjFunc(ioParms.GetContext());
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
				VJSObject jsobj(ioParms.GetContext());
				if (val.IsObject())
					jsobj = val.GetObject();
				else
					jsobj.MakeEmpty();
					
				/*
				VJSJSON json(ioParms.GetContext());
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
						VJSObject valobj = val.GetObject();
						VJSArray varr(valobj);
						VSize nbelem = varr.GetLength();
						VectorOfVValue vals(true);
						for (VSize i = 0; i < nbelem; i++)
						{
							VJSValue elem(varr.GetValueAt(i));
							VValueSingle* cv = elem.CreateVValue(att->isSimpleDate());
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
												VJSJSON json(ioParms.GetContext());
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
						else if (att->GetDataKind() == VK_BLOB_OBJ)
						{
							VJSValue propval(ioParms.GetPropertyValue());
							VJSONValue val;
							//VJSJSON json(ioParms.GetContext());;
							VJSException excep;

							if (propval.GetJSONValue(val, &excep))
							{
								/*
								VString s;
								json.Stringify(propval, s, &excep);

								val.ParseFromString(s, true);
								*/
#if debuglr
								ObjectNodeCollection nodes;
								ObjectParser parser(val, nodes);
#endif
								VBlobObj blobobj(nil, (BlobWithPtr*)-1);
								blobobj.FromJSONValue(val);
								rec->setAttributeValue(att, &blobobj);
							}
							else
							{
								ioParms.SetException(excep);
							}
							isMeta = true;
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
	ioParms.ReturnValue(VJSEntityModel::CreateInstance(ioParms.GetContext(), inSelIter->GetModel()));
}


void VJSEntitySelectionIterator::_Next(VJSParms_callStaticFunction& ioParms, EntitySelectionIterator* inSelIter)
{
	EntitySelectionIterator* newIter = new EntitySelectionIterator(*inSelIter);
	BaseTaskInfo* context = GetBaseTaskInfoFromJSContext(ioParms, inSelIter->GetModel());
	newIter->Next(context);
	if (newIter->GetCurPos() != -1)
	{
		ioParms.ReturnValue(VJSEntitySelectionIterator::CreateInstance(ioParms.GetContext(), newIter));
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
				ioParms.ReturnVValue(*(key[0]), false);
			}
			else
			{
				VJSArray arr(ioParms.GetContext());
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
	VJSArray arr(ioParms.GetContext());
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


void VJSEntitySelectionIterator::_getAttributeProperty(XBOX::VJSParms_callStaticFunction& ioParms, EntitySelectionIterator* inSelIter)
{
	VError err = VE_OK;
	BaseTaskInfo* context = GetBaseTaskInfoFromJSContext(ioParms, inSelIter->GetModel());
	EntityRecord* inRecord = inSelIter->GetCurRec(context);
	if (inRecord != nil)
	{
		EntityAttribute* att = nil;
		if (ioParms.IsStringParam(1))
		{
			VString s;
			ioParms.GetStringParam(1, s);
			att = inRecord->GetModel()->getAttribute(s);
		}
		else
		{
			att = ioParms.GetParamObjectPrivateData<VJSEntityAttribute>(1);
		}

		if (att == nil)
		{
			vThrowError(VE_JVSC_WRONG_PARAMETER_TYPE_ATTRIBUTE, "1");
		}
		else
		{
			VString propS;
			ioParms.GetStringParam(2, propS);
			AttributeProperty prop = (AttributeProperty)EAttributeProperty[propS];
			if (prop == attprop_none)
			{
				vThrowError(VE_JVSC_WRONG_PARAMETER_TYPE_ATTRIBUTE_PROPERTY, "2", att->GetName());
			}
			else
			{
				EntityAttributeValue* val = inRecord->getAttributeValue(att, err, context);
				if (val != nil)
				{
					switch (prop)
					{
						case attprop_path:
							{
								VValueSingle* cv = val->getVValue();
								if (cv != nil)
								{
									VString path;
									cv->GetOutsidePath(path);
									ioParms.ReturnString(path);
								}
							}
							break;
						default:
							break;
					}
				}
			}
		}
	}
}


void VJSEntitySelectionIterator::_setAttributeProperty(XBOX::VJSParms_callStaticFunction& ioParms, EntitySelectionIterator* inSelIter)
{
	VError err = VE_OK;
	BaseTaskInfo* context = GetBaseTaskInfoFromJSContext(ioParms, inSelIter->GetModel());
	EntityRecord* inRecord = inSelIter->GetCurRec(context);
	if (inRecord != nil)
	{
		EntityAttribute* att = nil;
		if (ioParms.IsStringParam(1))
		{
			VString s;
			ioParms.GetStringParam(1, s);
			att = inRecord->GetModel()->getAttribute(s);
		}
		else
		{
			att = ioParms.GetParamObjectPrivateData<VJSEntityAttribute>(1);
		}

		if (att == nil)
		{
			vThrowError(VE_JVSC_WRONG_PARAMETER_TYPE_ATTRIBUTE, "1");
		}
		else
		{
			VString propS;
			ioParms.GetStringParam(2, propS);
			AttributeProperty prop = (AttributeProperty)EAttributeProperty[propS];
			if (prop == attprop_none)
			{
				vThrowError(VE_JVSC_WRONG_PARAMETER_TYPE_ATTRIBUTE_PROPERTY, "2", att->GetName());
			}
			else
			{
				EntityAttributeValue* val = inRecord->getAttributeValue(att, err, context);
				if (val != nil)
				{
					switch (prop)
					{
					case attprop_path:
						{
							VValueSingle* cv = val->getVValue();
							if (cv != nil)
							{
								VString path;
								ioParms.GetStringParam(3, path);
								cv->SetOutsidePath(path);
								val->Touch(context);
							}
						}
						break;
					default:
						break;
					}
				}
			}
		}
	}
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
					default:
						break;
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
		VJSObject obj(ioParms.GetContext());
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
		VJSObject obj(ioParms.GetContext());
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
		{ "getAttributeProperty", js_callStaticFunction<_getAttributeProperty>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "setAttributeProperty", js_callStaticFunction<_setAttributeProperty>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
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
			ioParms.ReturnValue(VJSEntitySelectionIterator::CreateInstance(ioParms.GetContext(), iter));
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
					VJSObject localObjFunc(ioParms.GetContext());
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
							ioParms.ReturnValue(VJSEntitySelection::CreateInstance(ioParms.GetContext(), result));
					}
					else
					{
						BaseTaskInfo* context = GetBaseTaskInfoFromJSContext( ioParms, inSelection);
						/*
						sLONG nbelem = inSelection->GetLength();
						VJSArray result(ioParms.GetContext());
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
						VJSValue result(inSelection->ProjectAttribute(att, err, context, ioParms.GetContext()));
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

			VJSValue result(ioParms.GetContext());
			inSelection->ComputeOnOneAttribute(xatt, action, result, context, ioParms.GetContext());
			ioParms.ReturnValue(result);
			okresult = true;

			/*
			{
				EntityAttributeSortedSelection attlist(em);
				attlist.AddAttribute(att, nil);

				VJSObject result(ioParms.GetContext());

				VError err  = inSelection->Compute(attlist, result, context, ioParms.GetContext());
				if (err == VE_OK)
				{
					VJSValue subatt(ioParms.GetContext());
					subatt = result.GetProperty(att->GetName());
					if (subatt.IsObject())
					{
						VJSObject subattobj(ioParms.GetContext());
						subatt.GetObject(subattobj);

						VJSValue subresult(ioParms.GetContext());
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
				VJSObject result(ioParms.GetContext());
				err  = inSelection->Compute(attlist, result, context, ioParms.GetContext(), withdistinct);
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
				VJSFunction func("ReportingDB4D.report", ioParms.GetContext());
				func.AddParam(VJSEntitySelection::CreateInstance(ioParms.GetContext(), inSelection));
				func.AddParam(attlistString);
				func.AddParam(groupbyString);
				if (func.Call())
				{
					okresult = true;
					ioParms.ReturnValue(func.GetResult());
				}
				else
				{
					VJSException	except;
					func.GetException(except);
					ioParms.SetException(except);
				}
			}
		}
	}
	

	if (!okresult)
	{
		ioParms.ReturnNullValue();
	}
}


void __exportSel(XBOX::VJSParms_callStaticFunction& ioParms, EntityCollection* inSelection, exportAs_type exportType)
{
	VError err = VE_OK;
	bool okresult = false;
	BaseTaskInfo* context = GetBaseTaskInfoFromJSContext( ioParms, inSelection);
	EntityModel* model = inSelection->GetModel();
	if (model == nil)
		err = ThrowBaseError(VE_DB4D_NOT_AN_ENTITY_COLLECTION);
	else
	{
		LocalEntityCollection* localsel = dynamic_cast<LocalEntityCollection*>(inSelection);
		if (localsel != nil)
		{
			Selection* sel = localsel->GetSel();
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
				if (exportType == exportAs_JSON)
					options.JSONExport = true;
				else if (exportType == exportAs_Binary)
					options.BinaryExport = true;

				sLONG blobthreshold = 0;
				ioParms.GetLongParam(4, &blobthreshold);
				options.BlobThresholdSize = blobthreshold;

				options.CreateFolder = false;
				options.NbBlobsPerLevel = nbBlobsPerLevel;
				options.MaxSQLTextSize = maxSQLFileSize;
				LocalEntityModel* localmodel = dynamic_cast<LocalEntityModel*>(model);
				err = sel->ExportToSQL(nil, localmodel->GetMainTable(), context, folder, nil, options);
				folder->Release();
			}
		}
	}
}


void VJSEntitySelection::_exportAsJSON(XBOX::VJSParms_callStaticFunction& ioParms, EntityCollection* inSelection)
{
	__exportSel(ioParms, inSelection, exportAs_JSON);
}

void VJSEntitySelection::_exportAsSQL(XBOX::VJSParms_callStaticFunction& ioParms, EntityCollection* inSelection)
{
	__exportSel(ioParms, inSelection, exportAs_SQL);
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
						default:
							break;
					}
					if (result != nil)
					{
						okresult = true;
						ioParms.ReturnValue(VJSEntitySelection::CreateInstance(ioParms.GetContext(), result));
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
		VJSContext	vjsContext(ioParms.GetContext());
		VJSJSON json(vjsContext);

		VJSValue result(vjsContext);
		json.Parse(result,jsons);

		if (!result.IsUndefined())
		{
			ioParms.ReturnValue(result);
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
		VJSContext	vjsContext(ioParms.GetContext());
		VJSJSON json(vjsContext);

		VJSValue result(vjsContext);
		json.Parse(result,jsons);

		if (!result.IsUndefined())
		{
			ioParms.ReturnValue(result);
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
		ioParms.ReturnValue(VJSEntitySelectionIterator::CreateInstance(ioParms.GetContext(), iter));
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
			ioParms.ReturnValue(VJSEntitySelection::CreateInstance(ioParms.GetContext(), newcol));
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
	ioParms.ReturnValue(VJSEntityModel::CreateInstance(ioParms.GetContext(), inSelection->GetModel()));
}


void VJSEntitySelection::_Each(XBOX::VJSParms_callStaticFunction& ioParms, EntityCollection* inSelection)
{
	VJSValue valfunc(ioParms.GetParamValue(1));
	vector<VJSValue> params;
	if (!valfunc.IsUndefined() && valfunc.IsObject())
	{
		VJSObject objfunc = valfunc.GetObject();
		if (objfunc.IsFunction())
		{
			VJSValue elemVal(ioParms.GetContext());
			params.push_back(elemVal);
			VJSValue indiceVal(ioParms.GetContext());
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
					VJSValue indiceVal2(ioParms.GetContext());
					indiceVal2.SetNumber(i);
					params[1] = indiceVal2;
					//params[1].SetNumber(i);
					EntitySelectionIterator* iter = new EntitySelectionIterator(erec, context);
					VJSObject thisobj(VJSEntitySelectionIterator::CreateInstance(ioParms.GetContext(), iter));
					params[0] = thisobj;
					VJSValue result(ioParms.GetContext());
					VJSException except;
					thisParam.CallFunction(objfunc, &params, &result, except);
					if (!except.IsEmpty())
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
		ioParms.ReturnValue(VJSEntitySelection::CreateInstance(ioParms.GetContext(), locked));
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
			JSCollectionManager collection(ioParms.GetContext(), att->isSimpleDate());
			collection.SetNumberOfColumn(1);
			VCompareOptions options;
			options.SetDiacritical(true);
			options.SetIntlManager(GetContextIntl(context));
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
	VJSObject objfunc(ioParms.GetContext());
	bool withfunc = false;
	//BaseTaskInfo* basecontext = ConvertContext(context);
	
	VJSObject thisParam(ioParms.GetContext());
	VJSValue valfunc(ioParms.GetParamValue(curparam));
	vector<VJSValue> params;
	if (!valfunc.IsUndefined() && valfunc.IsObject())
	{
		objfunc = valfunc.GetObject();
		if (objfunc.IsFunction())
		{
			VJSValue elemVal(ioParms.GetContext());
			params.push_back(elemVal);
			VJSValue indiceVal(ioParms.GetContext());
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

	VJSArray arr(ioParms.GetContext());

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
				VJSValue indiceVal2(ioParms.GetContext());
				indiceVal2.SetNumber(curindice);
				params[1] = indiceVal2;
				//params[1].SetNumber(curindice);
				EntitySelectionIterator* iter = new EntitySelectionIterator(erec, context);
				VJSObject thisobj(VJSEntitySelectionIterator::CreateInstance(ioParms.GetContext(), iter));
				params[0] = thisobj;
				VJSValue result(ioParms.GetContext());
				VJSException except;
				thisParam.CallFunction(objfunc, &params, &result, except);
				if (!except.IsEmpty())
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
		arr = inSelection->ToJsArray(context, ioParms.GetContext(), attlist, &expand, &sortingatts, withkey, false, startvalue, nbvalue, err, nil);
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
	
	VJSArray obj(inSelection->ToJsArray(context, ioParms.GetContext(), atts, nil, nil, true, false, -1, -1, err, nil));
	
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
		{ "exportAsSQL", js_callStaticFunction<_exportAsSQL>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "exportAsJSON", js_callStaticFunction<_exportAsJSON>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
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
	VJSObject result(ioParms.GetContext());
	result.MakeEmpty();
	VJSValue jsval(ioParms.GetContext());
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

	VJSArray objarr(ioParms.GetContext());
	for( VMapOfObjectInfo::const_iterator i = stats.fObjectInfo.begin() ; i != stats.fObjectInfo.end() ; ++i)
	{
		const std::type_info* typobj = i->first;
		VStr255 s;
		VSystem::DemangleSymbol( typobj->name(), s);
		VJSObject elem(ioParms.GetContext());
		elem.MakeEmpty();
		VJSValue elemval(ioParms.GetContext());
		elemval.SetString(s);
		elem.SetProperty("id", elemval, JS4D::PropertyAttributeNone);
		elemval.SetNumber(i->second.fCount);
		elem.SetProperty("count", elemval, JS4D::PropertyAttributeNone);
		elemval.SetNumber(i->second.fTotalSize);
		elem.SetProperty("size", elemval, JS4D::PropertyAttributeNone);
		objarr.PushValue(elem);
	}
	result.SetProperty("objects", objarr, JS4D::PropertyAttributeNone);

	VJSArray blockarr(ioParms.GetContext());
	for( VMapOfBlockInfo::const_iterator i = stats.fBlockInfo.begin() ; i != stats.fBlockInfo.end() ; ++i)
	{
		VStr4 s;
		s.FromOsType( i->first);
		VJSObject elem(ioParms.GetContext());
		elem.MakeEmpty();
		VJSValue elemval(ioParms.GetContext());
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
void _OpenRemoteStore(VJSParms_callStaticFunction& ioParms, VJSGlobalObject*, bool addAlso)
{
	VError err = VE_OK;
	sLONG startParam = 1;
	VString catname;
	if (addAlso)
	{
		if (ioParms.IsStringParam(startParam))
		{
			ioParms.GetStringParam(startParam, catname);
		}
		else
			err = vThrowError(VE_JVSC_WRONG_PARAMETER_TYPE_STRING, ToString(startParam));
		++startParam;
	}
	if (err == VE_OK)
	{
		VJSONObject* params = nil;
		VString url, username, password;
		if (ioParms.IsObjectParam(startParam))
		{
			VJSObject objparams(ioParms.GetContext());
			ioParms.GetParamObject(startParam, objparams);
			VJSONValue inutile;
			((VJSValue)objparams).GetJSONValue(inutile);
			params = RetainRefCountable(inutile.GetObject());
		}
		else
		{
			ioParms.GetStringParam(startParam, url);
			ioParms.GetStringParam(startParam+1, username);
			ioParms.GetStringParam(startParam+2, password);
			params = new VJSONObject();
			params->SetPropertyAsString(khostname, url);
			params->SetPropertyAsString(kuser, username);
			params->SetPropertyAsString(kpassword, password);
			params->SetPropertyAsBool(kSQL, false);
		}
		/*
		VFolder* folder = VDBMgr::GetManager()->RetainResourceFolder();
		VFile xfile(*folder, "remote");
		CDB4DBase* xbd = VDBMgr::GetManager()->CreateBase(xfile, DB4D_Create_Empty_Catalog | DB4D_Create_As_XML_Definition | DB4D_Create_No_Respart | DB4D_Create_WithSeparateIndexSegment, XBOX::VTask::GetCurrentIntlManager(), &err);
		Base4D* bd = dynamic_cast<VDB4DBase*>(xbd)->GetBase();
		*/

		Base4D* bd = nil;

		VJSObject globobj = ioParms.GetContext().GetGlobalObject();
		VJSValue dsval = globobj.GetProperty("ds");
		if (dsval.IsObject())
		{
			bd = dsval.GetObject().GetPrivateData<VJSDatabase>();
		}
		
		if (bd == nil)
		{
			vThrowError(VE_DB4D_CANNOT_USE_ADD_REMOTE);
		}
		else
		{
			EntityModelCatalog* remotecat = VDBMgr::GetManager()->OpenRemoteCatalog(catname, params, bd, nil, true, err);
			if (remotecat == nil)
			{
				// throw err;
			}
			else
			{
				//err = bd->AddOutsideCatalog(remotecat);
				VJSContext jscontext(ioParms.GetContext());
				void* ProjectRef = jscontext.GetGlobalObjectPrivateInstance()->GetSpecific('db4x');
				VJSValue retcat(VJSEntityModelCatalog::CreateInstance(ioParms.GetContext(), remotecat));
				if (ProjectRef != nil)
				{
					VDBMgr::GetManager()->AddOutsideCatalog(catname, remotecat, ProjectRef);

					jscontext.GetGlobalObject().SetProperty(catname, retcat, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontDelete);
				}
				ioParms.ReturnValue(retcat);
			}

			QuickReleaseRefCountable(remotecat);
		}
	}
}


void do_OpenRemoteStore(VJSParms_callStaticFunction& ioParms, VJSGlobalObject* globobj)
{
	_OpenRemoteStore(ioParms, globobj, false);
}


void do_AddRemoteStore(VJSParms_callStaticFunction& ioParms, VJSGlobalObject* globobj)
{
	_OpenRemoteStore(ioParms, globobj, true);
}

void do_OpenBase(VJSParms_callStaticFunction& ioParms, VJSGlobalObject*)
{
	VFile* structfile = ioParms.RetainFileParam( 1);
	VFile* datafile = ioParms.RetainFileParam( 2);
	
	if (structfile != NULL)
	{
		if (datafile == NULL)
		{
			VString s;
			structfile->GetNameWithoutExtension(s);
			s += kDataFileExt;
			VFolder* parent = structfile->RetainParentFolder();
			if (parent != NULL)
			{
				datafile = new VFile(*parent, s);
			}
			ReleaseRefCountable( &parent);
		}
		
		if (datafile != NULL)
		{
			CDB4DManager* db4D = CDB4DManager::RetainManager();
			VError err = VE_OK;
			CDB4DBase* newdb = db4D->OpenBase(*structfile, DB4D_Open_WithSeparateIndexSegment | DB4D_Open_As_XML_Definition | DB4D_Open_No_Respart, &err);
			if (err == VE_OK)
			{
				CDB4DBaseContext* dbcontext = NULL;
				CDB4DContext* ownercontext = GetDB4DContextFromJSContext( ioParms.GetContext());
				if (ownercontext != NULL)
					dbcontext = ownercontext->RetainDataBaseContext(newdb, true);
					
				newdb->OpenData(*datafile, DB4D_Open_AllDefaultParamaters, dbcontext, &err);
				QuickReleaseRefCountable(dbcontext);
				if (err == VE_OK)
				{
					ioParms.ReturnValue(VJSDatabase::CreateInstance(ioParms.GetContext(), dynamic_cast<VDB4DBase*>(newdb)->GetBase()));
				}
			}
			QuickReleaseRefCountable( newdb);
			ReleaseRefCountable( &db4D);
		}
	}
	QuickReleaseRefCountable(structfile);
	QuickReleaseRefCountable(datafile);
	
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
					ioParms.ReturnValue(VJSDatabase::CreateInstance(ioParms.GetContext(), dynamic_cast<VDB4DBase*>(newdb)->GetBase()));
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
					ioParms.ReturnValue(VJSDatabase::CreateInstance(ioParms.GetContext(), dynamic_cast<VDB4DBase*>(newdb)->GetBase()));
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
			ioParms.ReturnValue(VJSDatabase::CreateInstance(ioParms.GetContext(), db));
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
			ioParms.ReturnValue(VJSDatabase::CreateInstance(ioParms.GetContext(), db));
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
					ioParms.ReturnValue(VJSDatabase::CreateInstance(ioParms.GetContext(), db));
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
	ioParms.ReturnValue(VJSCacheManager::CreateInstance(ioParms.GetContext(), VDBMgr::GetManager()));
}


void do_DynamicObj(VJSParms_callStaticFunction& ioParms, VJSGlobalObject*)
{
	DynamicObj* obj = new DynamicObj();
	ioParms.ReturnValue(VJSDynamicObj::CreateInstance(ioParms.GetContext(), obj));
}

/*
int dprint(int* dd, int n, int m){
	int i, j;
	for (i = 0; i < n + 2; i++){
		for (j = 0; j < m + 2; j++){
			printf("%02d ", d(i, j));
		}
		printf("\n");
	}
	printf("\n");
	return 0;
}
*/

#define MAX_VAR_SIZE 256

void do_editDist(VJSParms_callStaticFunction& ioParms, VJSGlobalObject*)
{
#define d(i,j) dd[(i) * (m+2) + (j) ]
#define min(x,y) ((x) < (y) ? (x) : (y))
#define min3(a,b,c) ((a)< (b) ? min((a),(c)) : min((b),(c)))
#define min4(a,b,c,d) ((a)< (b) ? min3((a),(c),(d)) : min3((b),(c),(d)))

	VString s1, s2;

	AsciiBuffer* obj1 = ioParms.GetParamObjectPrivateData<VJSAsciiStr>(1);
	AsciiBuffer* obj2 = ioParms.GetParamObjectPrivateData<VJSAsciiStr>(2);

	char  buf_s[MAX_VAR_SIZE + 1],
		buf_t[MAX_VAR_SIZE + 1];


	char *s;
	char* t;
	int n;
	int m;

	if (obj1 == nil)
	{
		s = &buf_s[0];
		ioParms.GetStringParam(1, s1);
		s1.ToUpperCase(true);
		s1.TrimeSpaces();
		s1.Truncate(MAX_VAR_SIZE);
		s1.ToBlock(&buf_s[0], MAX_VAR_SIZE, VTC_US_ASCII, false, false);
		n = s1.GetLength();
	}
	else
	{
		s = obj1->getData();
		n = obj1->getLength();
	}

	if (obj2 == nil)
	{
		t = &buf_t[0];
		ioParms.GetStringParam(2, s2);
		s2.ToUpperCase(true);
		s2.TrimeSpaces();
		s2.Truncate(MAX_VAR_SIZE);
		s2.ToBlock(&buf_t[0], MAX_VAR_SIZE, VTC_US_ASCII, false, false);
		m = s2.GetLength();
	}
	else
	{
		t = obj2->getData();
		m = obj2->getLength();
	}

	int *dd, *DA;
	int i, j, cost, k, i1, j1, DB;
	int maxpossible = n + m;

	int buf_DA[MAX_VAR_SIZE];
	int buf_dd[MAX_VAR_SIZE*MAX_VAR_SIZE];

	// DA = (int*)malloc(256 * sizeof(int));
	// dd = (int*)malloc((n + 2)*(m + 2)*sizeof(int));

	DA = &buf_DA[0];
	dd = &buf_dd[0];

	d(0, 0) = maxpossible;
	for (i = 0; i < n + 1; i++) {
		d(i + 1, 1) = i;
		d(i + 1, 0) = maxpossible;
	}
	for (j = 0; j < m + 1; j++) {
		d(1, j + 1) = j;
		d(0, j + 1) = maxpossible;
	}
	//dprint(dd, n, m);
	//for (k = 0; k < 256; k++) DA[k] = 0;
	std::fill(&DA[0], &DA[MAX_VAR_SIZE], 0);
	for (i = 1; i < n + 1; i++) {
		DB = 0;
		for (j = 1; j < m + 1; j++) {
			i1 = DA[t[j - 1]];
			j1 = DB;
			cost = ((s[i - 1] == t[j - 1]) ? 0 : 1);
			if (cost == 0) 
				DB = j;
			d(i + 1, j + 1) =
				min4(d(i, j) + cost,
				d(i + 1, j) + 1,
				d(i, j + 1) + 1,
				d(i1, j1) + (i - i1 - 1) + 1 + (j - j1 - 1));
		}
		DA[s[i - 1]] = i;
		//dprint(dd, n, m);
	}
	cost = d(n + 1, m + 1);
	//free(dd);
		
	ioParms.ReturnNumber(cost);
}

//#include <ctype.h>
//#include <string.h>

#define NOTNUM(c)	((c>57) || (c<48))
#define INRANGE(c)      ((c>0)  && (c<91))


void do_StringSymetry(VJSParms_callStaticFunction& ioParms, VJSGlobalObject*)
{
	Real result = 0.0;

	VString s1, s2;

	AsciiBuffer* obj1 = ioParms.GetParamObjectPrivateData<VJSAsciiStr>(1);
	AsciiBuffer* obj2 = ioParms.GetParamObjectPrivateData<VJSAsciiStr>(2);

	char  buf_ying_hold[MAX_VAR_SIZE + 1],
		buf_yang_hold[MAX_VAR_SIZE + 1],
		ying_flag[MAX_VAR_SIZE + 1],
		yang_flag[MAX_VAR_SIZE + 1];

	char* ying_hold;
	char* yang_hold;
	long ying_length;
	long yang_length;

	if (obj1 == nil)
	{
		ying_hold = &buf_ying_hold[0];
		ioParms.GetStringParam(1, s1);
		s1.ToUpperCase(true);
		s1.TrimeSpaces();
		s1.Truncate(MAX_VAR_SIZE);
		s1.ToBlock(&ying_hold[0], MAX_VAR_SIZE, VTC_US_ASCII, false, false);
		ying_length = s1.GetLength();
		ying_hold[ying_length] = 0;
	}
	else
	{
		ying_hold = obj1->getData();
		ying_length = obj1->getLength();
	}

	if (obj2 == nil)
	{
		yang_hold = &buf_yang_hold[0];
		ioParms.GetStringParam(2, s2);
		s2.ToUpperCase(true);
		s2.TrimeSpaces();
		s2.Truncate(MAX_VAR_SIZE);
		s2.ToBlock(&yang_hold[0], MAX_VAR_SIZE, VTC_US_ASCII, false, false);
		yang_length = s2.GetLength();
		yang_hold[yang_length] = 0;
	}
	else
	{
		yang_hold = obj2->getData();
		yang_length = obj2->getLength();
	}

	/* strcmp95.c   Version 2						      */

	/* The strcmp95 function returns a double precision value from 0.0 (total
	disagreement) to 1.0 (character-by-character agreement).  The returned
	value is a measure of the similarity of the two strings.                   */

	/* Date of Release:  Jan. 26, 1994					      */
	/* Modified: April 24, 1994  Corrected the processing of the single length
	character strings.
	Authors:  This function was written using the logic from code written by
	Bill Winkler, George McLaughlin and Matt Jaro with modifications
	by Maureen Lynch.
	Comment:  This is the official string comparator to be used for matching
	during the 1995 Test Census.                                     */


	bool higherProba = true;

	{
		/* Arguments:

		ying and yang are pointers to the 2 strings to be compared.  The strings
		need not be NUL-terminated strings because the length is passed.

		y_length is the length of the strings.

		higherProba Increase the probability of a match when the number of matched
		characters is large.  This option allows for a little more
		tolerance when the strings are large.  It is not an appropriate
		test when comparing fixed length fields such as phone and
		social security numbers.
		 
		*/

		static	int	pass = 0, adjwt[91][91];
		static	char	sp[39][2] =
		{ 'A', 'E', 'A', 'I', 'A', 'O', 'A', 'U', 'B', 'V', 'E', 'I', 'E', 'O', 'E', 'U',
		'I', 'O', 'I', 'U', 'O', 'U', 'I', 'Y', 'E', 'Y', 'C', 'G', 'E', 'F',
		'W', 'U', 'W', 'V', 'X', 'K', 'S', 'Z', 'X', 'S', 'Q', 'C', 'U', 'V',
		'M', 'N', 'L', 'I', 'Q', 'O', 'P', 'R', 'I', 'J', '2', 'Z', '5', 'S',
		'8', 'B', '1', 'I', '1', 'L', '0', 'O', '0', 'Q', 'C', 'K', 'G', 'J',
		'E', ' ', 'Y', ' ', 'S', ' ' };


		double  weight = 0.0, Num_sim;

		long    minv, search_range, lowlim,
			hilim, N_trans, Num_com;

		int	yl1, yi_st, N_simi;

		register        int     i, j, k;

		/* Initialize the adjwt array on the first call to the function only.
		The adjwt array is used to give partial credit for characters that
		may be errors due to known phonetic or character recognition errors.
		A typical example is to match the letter "O" with the number "0"           */
		if (!pass) {
			pass++;
			for (i = 0; i<91; i++) for (j = 0; j<91; j++) adjwt[i][j] = 0;
			for (i = 0; i<36; i++) {
				adjwt[sp[i][0]][sp[i][1]] = 3;
				adjwt[sp[i][1]][sp[i][0]] = 3;
			}
		}

		if (ying_length > yang_length) {
			search_range = ying_length;
			minv = yang_length;
		}
		else {
			search_range = yang_length;
			minv = ying_length;
		}

		/* If either string is blank - return                                         */

		/* Blank out the flags														  */
		std::fill(&ying_flag[0], &ying_flag[MAX_VAR_SIZE], ' ');
		std::fill(&yang_flag[0], &yang_flag[MAX_VAR_SIZE], ' ');
		ying_flag[MAX_VAR_SIZE] = 0;
		yang_flag[MAX_VAR_SIZE] = 0;
		search_range = (search_range / 2) - 1;
		if (search_range < 0) search_range = 0;   /* added in version 2               */

		/* Convert all lower case characters to upper case.                           */

		/* Looking only within the search range, count and flag the matched pairs.    */
		Num_com = 0;
		yl1 = yang_length - 1;
		for (i = 0; i < ying_length; i++) {
			lowlim = (i >= search_range) ? i - search_range : 0;
			hilim = ((i + search_range) <= yl1) ? (i + search_range) : yl1;
			for (j = lowlim; j <= hilim; j++)  {
				if ((yang_flag[j] != '1') && (yang_hold[j] == ying_hold[i])) {
					yang_flag[j] = '1';
					ying_flag[i] = '1';
					Num_com++;
					break;
				}
			}
		}

		/* If no characters in common - return                                        */
		if (Num_com != 0)
		{
			/* Count the number of transpositions                                         */
			k = N_trans = 0;
			for (i = 0; i < ying_length; i++) {
				if (ying_flag[i] == '1') {
					for (j = k; j < yang_length; j++) {
						if (yang_flag[j] == '1') {
							k = j + 1;
							break;
						}
					}
					if (ying_hold[i] != yang_hold[j]) N_trans++;
				}
			}
			N_trans = N_trans / 2;

			/* adjust for similarities in nonmatched characters                           */
			N_simi = 0;
			if (minv > Num_com) {
				for (i = 0; i < ying_length; i++) {
					if (ying_flag[i] == ' ' && INRANGE(ying_hold[i])) {
						for (j = 0; j < yang_length; j++) {
							if (yang_flag[j] == ' ' && INRANGE(yang_hold[j])) {
								if (adjwt[ying_hold[i]][yang_hold[j]] > 0) {
									N_simi += adjwt[ying_hold[i]][yang_hold[j]];
									yang_flag[j] = '2';
									break;
								}
							}
						}
					}
				}
			}
			Num_sim = ((double)N_simi) / 10.0 + Num_com;

			/* Main weight computation.						      */
			weight = Num_sim / ((double)ying_length) + Num_sim / ((double)yang_length)
				+ ((double)(Num_com - N_trans)) / ((double)Num_com);
			weight = weight / 3.0;

			/* Continue to boost the weight if the strings are similar                    */
			if (weight > 0.7) {

				/* Adjust for having up to the first 4 characters in common                 */
				j = (minv >= 4) ? 4 : minv;
				for (i = 0; ((i<j) && (ying_hold[i] == yang_hold[i]) && (NOTNUM(ying_hold[i]))); i++);
				if (i) weight += i * 0.1 * (1.0 - weight);

				/* Optionally adjust for long strings.                                      */
				/* After agreeing beginning chars, at least two more must agree and
				the agreeing characters must be > .5 of remaining characters.          */
				if ((higherProba) && (minv>4) && (Num_com > i + 1) && (2 * Num_com >= minv + i))
				if (NOTNUM(ying_hold[0]))
					weight += (double)(1.0 - weight) *
					((double)(Num_com - i - 1) / ((double)(ying_length + yang_length - i * 2 + 2)));
			}

		}

		result = weight;
	} /* strcmp95 */


	ioParms.ReturnNumber(result);
}



void do_AsciiStr(VJSParms_callStaticFunction& ioParms, VJSGlobalObject*)
{
	VString s;
	bool toupper = false, trim = false;
	ioParms.GetStringParam(1, s);
	ioParms.GetBoolParam(2, &toupper);
	ioParms.GetBoolParam(3, &trim);

	AsciiBuffer* obj = new AsciiBuffer(s, toupper, trim);
	ioParms.ReturnValue(VJSAsciiStr::CreateInstance(ioParms.GetContext(), obj));
}




//======================================================


AsciiBuffer::AsciiBuffer(const VString& from, bool toUpperCase, bool trimSpaces)
{
	VString s = from;
	if (toUpperCase)
		s.ToUpperCase(true);

	if (trimSpaces)
		s.TrimeSpaces();

	s.Truncate(max_char_ascii_buffer);
	s.ToBlock(&fData[0], max_char_ascii_buffer, VTC_US_ASCII, false, false);
	fLength = s.GetLength();
	fData[fLength] = 0;
}



void VJSAsciiStr::Initialize(const VJSParms_initialize& inParms, AsciiBuffer* obj)
{
	// rien a faire pour l'instant
}


void VJSAsciiStr::Finalize(const VJSParms_finalize& inParms, AsciiBuffer* obj)
{
	delete obj;
}


void VJSAsciiStr::_toString(VJSParms_callStaticFunction& ioParms, AsciiBuffer* obj)
{
	VString s;
	s.FromBlock(obj->getData(), obj->getLength(), VTC_US_ASCII);
	ioParms.ReturnString(s);
}


void VJSAsciiStr::GetDefinition(ClassDefinition& outDefinition)
{
	
	static inherited::StaticFunction functions[] =
	{
		{ "toString", js_callStaticFunction<_toString>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },

		{ 0, 0, 0 }
	};
	

	outDefinition.staticFunctions = functions;
	

	outDefinition.className = "AsciiStr";
	outDefinition.initialize = js_initialize<Initialize>;
	outDefinition.finalize = js_finalize<Finalize>;
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
	VJSDynamicObj::Class();
//	VJSSelectionIterator::Class();

	//VJSGlobalClass::AddStaticFunction( "open4DBase", VJSGlobalClass::js_callStaticFunction<do_Open4DBase>, JS4D::PropertyAttributeNone);
	//VJSGlobalClass::AddStaticFunction( "openDataStore", VJSGlobalClass::js_callStaticFunction<do_OpenBase>, JS4D::PropertyAttributeNone);
	//VJSGlobalClass::AddStaticFunction( "createDataStore", VJSGlobalClass::js_callStaticFunction<do_CreateBase>, JS4D::PropertyAttributeNone);
	//VJSGlobalClass::AddStaticFunction( "openRemoteBase", VJSGlobalClass::js_callStaticFunction<do_OpenRemoteBase>, JS4D::PropertyAttributeNone); // DataBase : OpenRemoteBase(string : servername, long : portnum);
	VJSGlobalClass::AddStaticFunction( "openRemoteStore", VJSGlobalClass::js_callStaticFunction<do_OpenRemoteStore>, JS4D::PropertyAttributeNone);
	VJSGlobalClass::AddStaticFunction( "addRemoteStore", VJSGlobalClass::js_callStaticFunction<do_AddRemoteStore>, JS4D::PropertyAttributeNone);
	//VJSGlobalClass::AddStaticFunction( "getDataStore", VJSGlobalClass::js_callStaticFunction<do_GetBase>, JS4D::PropertyAttributeNone);

	VJSGlobalClass::AddStaticFunction( "getCacheManager", VJSGlobalClass::js_callStaticFunction<do_GetCacheManager>, JS4D::PropertyAttributeNone);
	VJSGlobalClass::AddStaticFunction( "DynamicObj", VJSGlobalClass::js_callStaticFunction<do_DynamicObj>, JS4D::PropertyAttributeNone);

	VJSGlobalClass::AddStaticFunction("strSym", VJSGlobalClass::js_callStaticFunction<do_StringSymetry>, JS4D::PropertyAttributeNone);
	VJSGlobalClass::AddStaticFunction("AsciiStr", VJSGlobalClass::js_callStaticFunction<do_AsciiStr>, JS4D::PropertyAttributeNone);
	VJSGlobalClass::AddStaticFunction("strEditDist", VJSGlobalClass::js_callStaticFunction<do_editDist>, JS4D::PropertyAttributeNone);
	
}


//======================================================


DynamicObj::~DynamicObj()
{
	for (MapOfJSValues::iterator cur = fValues.begin(), end = fValues.end(); cur != end; ++cur)
	{
		//JS4D::UnprotectValue(fContextRef, cur->second);
	}
}




void VJSDynamicObj::Initialize( const VJSParms_initialize& inParms, DynamicObj* obj)
{
	// rien a faire pour l'instant
	obj->SetContextRef(inParms.GetContext());
}


void VJSDynamicObj::Finalize( const VJSParms_finalize& inParms, DynamicObj* obj)
{
	delete obj;
}



void VJSDynamicObj::GetPropertyNames( VJSParms_getPropertyNames& ioParms, DynamicObj* obj)
{
	/*
	MapOfJSValues& values = obj->GetValues();

	for (MapOfJSValues::iterator cur = values.begin(), end = values.end(); cur != end; ++cur)
	{
		ioParms.AddPropertyName(cur->first);
	}
	*/

	VectorOfVString& props = obj->GetPropsChrono();
	for (VectorOfVString::iterator cur = props.begin(), end = props.end(); cur != end; ++cur)
	{
		ioParms.AddPropertyName(*cur);
	}
}

void VJSDynamicObj::GetProperty( VJSParms_getProperty& ioParms, DynamicObj* obj)
{
	MapOfJSValues& values = obj->GetValues();
	VectorOfVString& props = obj->GetPropsChrono();
	VString propname;
	ioParms.GetPropertyName(propname);
	if (! propname.EqualToUSASCIICString("toJSON"))
	{
		MapOfJSValues::iterator found = values.find(propname);
		if (found != values.end())
		{
			// is found->second in the same VJSContext as ioParms.GetContext()  ?????
			// --> showed to and validated by LR -> all VJSContexts of the maps are refering to the same GlobalContext
			ioParms.ReturnValue(found->second);
		}
		else
		{
			if (ioParms.GetContext().GetGlobalObjectPrivateInstance()->GetSpecific( 'db4x') == (void*)-1)
			{
				ioParms.ReturnUndefinedValue();
			}
			else
			{
				if (propname.EqualToUSASCIICString("addEventListener"))
				{
					VJSValue val(ioParms.GetContext());
					ioParms.GetContext().EvaluateScript("DataClass.prototype.addEventListener", nil, &val, nil, nil);
					val.Protect();
					values.insert(make_pair(propname, val));
					props.push_back(propname);
					ioParms.ReturnValue(val);
				}
				else
				{
				DynamicObj* subobj = new DynamicObj();
				VJSValue val(VJSDynamicObj::CreateInstance(ioParms.GetContext(), subobj));
				val.Protect();
				values.insert(make_pair(propname, val));
				props.push_back(propname);
				ioParms.ReturnValue(val);
			}
		}
	}
}
}



bool VJSDynamicObj::SetProperty( VJSParms_setProperty& ioParms, DynamicObj* obj)
{
	MapOfJSValues& values = obj->GetValues();
	VString propname;
	VectorOfVString& props = obj->GetPropsChrono();
	ioParms.GetPropertyName(propname);
	const VJSValue& val = ioParms.GetPropertyValue();
	MapOfJSValues::iterator found = values.find(propname);
	val.Protect();
	if (found != values.end())
	{
		// found->second.Unprotect(); // I get rid of the unprotect but it will not create a true memory leak because this is used at load time and the js context is going to be released
		found->second = val;
	}
	else
	{
		values.insert(make_pair(propname, val));
		props.push_back(propname);
	}
	return true;
}


void VJSDynamicObj::GetDefinition( ClassDefinition& outDefinition)
{
	/*
	static inherited::StaticFunction functions[] = 
	{
		{ "setAutoSeqValue", js_callStaticFunction<_setAutoSeqValue>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },

		{ 0, 0, 0}
	};

	outDefinition.staticFunctions = functions;
	*/

	outDefinition.className = "DynamicObject";
	outDefinition.initialize = js_initialize<Initialize>;
	outDefinition.finalize = js_finalize<Finalize>;
	outDefinition.getProperty = js_getProperty<GetProperty>;
	outDefinition.setProperty = js_setProperty<SetProperty>;
	outDefinition.getPropertyNames = js_getPropertyNames<GetPropertyNames>;
}





