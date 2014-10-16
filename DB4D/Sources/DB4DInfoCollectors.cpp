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
#include "DB4DInfoCollectors.h"

USING_TOOLBOX_NAMESPACE


#if WITH_RTM_DETAILLED_ACTIVITY_INFO

namespace OperationCaption
{
	const XBOX::VString& Unknown("???");
	const XBOX::VString& Query("query");
	const XBOX::VString& DeleteRecords("deleteRecords");
	const XBOX::VString& DistinctValues("distinctValues");
	const XBOX::VString& SelectionToArray("selectionToArray");
	const XBOX::VString& ArrayToSelection("arrayToSelection");
	const XBOX::VString& Sorting("sorting");
};

namespace Properties
{
	const XBOX::VString& OperationType("operationType");
	const XBOX::VString& DbContextInfo("dbContextInfo");
	const XBOX::VString& DbOperationDetails("dbOperationDetails");
	const XBOX::VString& Index("index");
	const XBOX::VString& IndexInfo("indexInfo");
	const XBOX::VString& Fields("fields");
	const XBOX::VString& Field("field");
	const XBOX::VString& Formula("formula");
	const XBOX::VString& Ascending("ascending");
	const XBOX::VString& QueryPlan("queryPlan");
	const XBOX::VString& SortArguments("sortParameters");
	const XBOX::VString& Table("table");
	
};



VDB4DBasicInfoCollector::VDB4DBasicInfoCollector(CDB4DBaseContext* inBaseContext) :
fBaseContext(inBaseContext)
{
}

VDB4DBasicInfoCollector::~VDB4DBasicInfoCollector()
{
}

XBOX::VJSONObject*	VDB4DBasicInfoCollector::CollectInfo(VProgressIndicator * /*inIndicator*/, bool /*inForRootSession*/)
{
	XBOX::VJSONObject* info = nil;

	if (fBaseContext != nil)
	{
		BaseTaskInfo* ctxt = ConvertContext(fBaseContext);
		if (ctxt != nil)
		{
			info = new XBOX::VJSONObject();
			if (info != nil)
			{
				XBOX::VJSONObject* o = ctxt->BuildLockInfo();
				if (o != NULL)
				{
					_BuildDbContextInfoProperty(o, info);
					o->Release();
				}
			}
		}
	}
	return info;
}

void VDB4DBasicInfoCollector::_BuildDbContextInfoProperty(const VJSONObject* inBaseContextLockInfo, VJSONObject* outContainerObject)
{
	XBOX::VJSONValue objValue;
	objValue = inBaseContextLockInfo->GetProperty(CVSTR("contextAttributes"));
	if (objValue.IsObject())
	{
		XBOX::VJSONObject *copy = new VJSONObject();
		if (copy != nil)
		{
			copy->MergeWith(objValue.GetObject());
			outContainerObject->SetProperty(Properties::DbContextInfo, XBOX::VJSONValue(copy));
		}
		XBOX::ReleaseRefCountable(&copy);
	}
}

void VDB4DBasicInfoCollector::_GetOperationTypeCaption(OperationType inType, XBOX::VString& outCaption)const
{
	switch (inType)
	{
	case kDeleteRecords:
		outCaption = OperationCaption::DeleteRecords;
		break;
	case kDistinctValues:
		outCaption = OperationCaption::DistinctValues;
		break;
	case kSelectionToArray:
		outCaption = OperationCaption::SelectionToArray;
		break;
	case kArrayToSelection:
		outCaption = OperationCaption::ArrayToSelection;
		break;
	case kOrderBy:
		outCaption = OperationCaption::Sorting;
		break;
	case kQuery:
		outCaption = OperationCaption::Query;
		break;

	case kLast:
	default:
		outCaption = OperationCaption::Unknown;
		break;
	}
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

VDB4DQueryInfoCollector::VDB4DQueryInfoCollector(CDB4DBaseContext* inContext, OptimizedQuery* inQuery, VProgressIndicator* ioProgressIndicator) :
VDB4DBasicInfoCollector(inContext),
fCollectedInfo(nil),
fQuery(inQuery),
fProgressIndicator(ioProgressIndicator)
{
	if (fProgressIndicator != NULL)
		fProgressIndicator->SetDefaultInfoCollector(this);

}

VDB4DQueryInfoCollector::~VDB4DQueryInfoCollector()
{
	if (fProgressIndicator != NULL)
		fProgressIndicator->SetDefaultInfoCollector(NULL);
	XBOX::ReleaseRefCountable(&fCollectedInfo);
}


XBOX::VJSONObject* VDB4DQueryInfoCollector::CollectInfo(VProgressIndicator *inIndicator, bool inForRootSession)
{
	if (fCollectedInfo == nil)
	{
		if (inForRootSession)
		{
			fCollectedInfo = VDB4DBasicInfoCollector::CollectInfo(inIndicator, inForRootSession);
		}

		if (fCollectedInfo == NULL)
		{
			fCollectedInfo = new XBOX::VJSONObject();
		}

		if ((fBaseContext != nil) && (fCollectedInfo != nil))
		{
			XBOX::VString tmp;
			XBOX::VJSONObject* info = new XBOX::VJSONObject();
			if (info != nil)
			{
				_GetOperationTypeCaption(kQuery, tmp);
				info->SetPropertyAsString(Properties::OperationType, tmp);

				BaseTaskInfo* context = ConvertContext(fBaseContext);
				Base4D* bd = context->GetBase();
				if (bd != nil && fQuery != nil)
				{
					fQuery->GetRoot()->FullyDescribe(bd, tmp);
					info->SetPropertyAsString(Properties::QueryPlan, tmp);
				}
				fCollectedInfo->SetProperty(Properties::DbOperationDetails, XBOX::VJSONValue(info));
			}
			XBOX::ReleaseRefCountable(&info);
		}
	}
	if (fCollectedInfo != nil)
	{
		fCollectedInfo->Retain();
	}
	return fCollectedInfo;
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////


VDB4DSortInfoCollector::VDB4DSortInfoCollector(CDB4DBaseContext* inBaseContext, Table* inTable, SortTab* inSortLines, XBOX::VProgressIndicator* inProgressIndicator) :
VDB4DBasicInfoCollector(inBaseContext),
fTable(inTable),
fSortLines(inSortLines),
fCollectedInfo(nil),
fProgressIndicator(inProgressIndicator)
{
	if (fProgressIndicator != nil)
	{
		fProgressIndicator->SetDefaultInfoCollector(this);
	}
}

VDB4DSortInfoCollector::~VDB4DSortInfoCollector()
{
	if (fProgressIndicator != nil)
	{
		fProgressIndicator->SetDefaultInfoCollector(NULL);
	}
	XBOX::ReleaseRefCountable(&fCollectedInfo);
}

XBOX::VJSONObject*	VDB4DSortInfoCollector::CollectInfo(VProgressIndicator *inIndicator, bool inForRootSession)
{
	if (fCollectedInfo == nil)
	{
		if (inForRootSession)
		{
			fCollectedInfo = VDB4DBasicInfoCollector::CollectInfo(inIndicator, inForRootSession);
		}
		if (fCollectedInfo != nil)
		{
			XBOX::VString tmp;
			XBOX::VJSONObject* details = new XBOX::VJSONObject();
			if (details)
			{
				_GetOperationTypeCaption(kOrderBy,tmp);
				details->SetPropertyAsString(Properties::OperationType, tmp);
				fTable->GetName(tmp);
				details->SetPropertyAsString(Properties::Table, tmp);
				if (fSortLines != nil)
				{
					sLONG nb = fSortLines->GetNbLine();
					if (nb >= 1)
					{
						XBOX::VJSONArray* fields = new XBOX::VJSONArray();
						if (fields != nil)
						{
							for (sLONG i = 1; i <= nb; ++i)
							{
								const SortLine* l = fSortLines->GetTriLineRef(i);
								XBOX::VJSONObject* param = nil;
								if (l->isfield)
								{
									Field* cri = fTable->RetainField(l->numfield);
									param = new XBOX::VJSONObject();
									if (param != nil)
									{
										cri->GetName(tmp);
										param->SetPropertyAsString(Properties::Field, tmp);
									}
								}
								else if (l->expression != NULL)
								{
									param = new XBOX::VJSONObject();
									if (param != nil)
									{
										l->expression->GetDescription(tmp);
										param->SetPropertyAsString(Properties::Formula, tmp);
									}
								}
								if (param != nil)
								{
									param->SetPropertyAsBool(Properties::Ascending, l->ascendant);
									fields->Push(XBOX::VJSONValue(param));
								}
								XBOX::ReleaseRefCountable(&param);
							}
							details->SetProperty(Properties::SortArguments, XBOX::VJSONValue(fields));
						}
						XBOX::ReleaseRefCountable(&fields);
					}
				}
				fCollectedInfo->SetProperty(Properties::DbOperationDetails, XBOX::VJSONValue(details));
			}
			XBOX::ReleaseRefCountable(&details);
		}
	}
	if (fCollectedInfo != nil)
	{
		//Keep one more tab on fCollectedInfo, we don't want to compute it for each call
		fCollectedInfo->Retain();
	}
	return fCollectedInfo;
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

VDB4DSelectionInfoCollector::VDB4DSelectionInfoCollector(CDB4DBaseContext* inContext, DataTable* inTargetTable, OperationType inOperationType) :
	VDB4DBasicInfoCollector(inContext),
	fCollectionManager(NULL),
	fTargetTable(NULL),
	fTargetField(NULL),
	fTargetIndex(NULL),
	fCollectedInfo(NULL),
	fProgressIndicator(NULL),
	fDB4DOperation(inOperationType)
{
	if (inTargetTable != NULL)
	{
		fTargetTable = inTargetTable->RetainTable();
	}
}

VDB4DSelectionInfoCollector::VDB4DSelectionInfoCollector(CDB4DBaseContext* inContext, Selection* inSelection, DB4DCollectionManager* inCollectionManager, OperationType inOperationType) :
VDB4DBasicInfoCollector(inContext),
fCollectionManager(inCollectionManager),
fTargetTable(NULL),
fTargetField(NULL),
fTargetIndex(NULL),
fCollectedInfo(NULL),
fProgressIndicator(NULL),
fDB4DOperation(inOperationType)
{
	if (inSelection != NULL)
	{
		fTargetTable = inSelection->GetParentFile()->RetainTable();
	}
	//Avoid retain/release on collection manager: allows to avoid the 90 cycle penalty on cpu core (interlock increment).
	//The collection mgr object lifespan SHOULD largely overlap ours so we don't need to retain/release
	//XBOX::RetainRefCountable(fCollectionManager);
	
}	



VDB4DSelectionInfoCollector::VDB4DSelectionInfoCollector(CDB4DBaseContext* inContext, Field* inTargetField, IndexInfo* inIndexInfo, XBOX::VProgressIndicator * inIndicator, OperationType inOperationType) :
	VDB4DBasicInfoCollector(inContext),
	fCollectionManager(NULL),
	fTargetTable(NULL),
	fTargetField(inTargetField),
	fTargetIndex(inIndexInfo),
	fCollectedInfo(NULL),
	fProgressIndicator(inIndicator),
	fDB4DOperation(inOperationType)
{
	if (fProgressIndicator != nil)
	{
		fProgressIndicator->SetDefaultInfoCollector(this);
	}
}

VDB4DSelectionInfoCollector::~VDB4DSelectionInfoCollector()
{
	if (fProgressIndicator != nil)
	{
		fProgressIndicator->SetDefaultInfoCollector(NULL);
	}
	
	//c.f. constructor why we avoid retain/release
	//XBOX::ReleaseRefCountable(&fCollectionManager);
	
	XBOX::ReleaseRefCountable(&fCollectedInfo);
	XBOX::ReleaseRefCountable(&fTargetTable);
}

VJSONObject*	VDB4DSelectionInfoCollector::CollectInfo(VProgressIndicator * inIndicator, bool inForRootSession)
{
	if (fCollectedInfo == NULL)
	{
		if (inForRootSession)
		{
			fCollectedInfo = VDB4DBasicInfoCollector::CollectInfo(inIndicator, inForRootSession);
		}

		if (fCollectedInfo == NULL)
		{
			fCollectedInfo = new XBOX::VJSONObject();
		}

		if ((fCollectedInfo != NULL) && ((fTargetField != nil) || (fTargetTable != NULL) || (fTargetIndex != NULL)))
		{
			XBOX::VJSONObject* info = new XBOX::VJSONObject();
			if (info != NULL)
			{
				XBOX::VString tmp;
				
				_GetOperationTypeCaption(fDB4DOperation, tmp);
				info->SetPropertyAsString(Properties::OperationType, tmp);

				if (fTargetTable != NULL)
				{
					fTargetTable->GetName(tmp);
					info->SetPropertyAsString(Properties::Table, tmp);
				}
				if (fTargetField != NULL)
				{
					fTargetField->GetOwner()->GetName(tmp);
					info->SetPropertyAsString(Properties::Table, tmp);
					fTargetField->GetName(tmp);
					info->SetPropertyAsString(Properties::Field, tmp);
				}
				else if (fCollectionManager != NULL)
				{
					XBOX::VJSONArray* fields = new XBOX::VJSONArray();
					if (fields != nil)
					{
						for (sLONG i = 1; i <= fCollectionManager->GetNumberOfColumns(); i++)
						{
							XBOX::VString name;
							fCollectionManager->GetColumnRef(i)->GetName(name);
							fields->Push(VJSONValue(name));
						}
						info->SetProperty(Properties::Fields, VJSONValue(fields));
					}
					XBOX::ReleaseRefCountable(&fields);
					
				}
				if (fTargetIndex != NULL)
				{
					if (fTargetIndex->GetName().IsEmpty())
						info->SetPropertyAsString(Properties::Index, CVSTR("no-name"));
					else
						info->SetPropertyAsString(Properties::Index, fTargetIndex->GetName());
				}
				fCollectedInfo->SetProperty(Properties::DbOperationDetails, XBOX::VJSONValue(info));
			}
			XBOX::ReleaseRefCountable(&info);
		}
	}
	if (fCollectedInfo != nil)
	{
		fCollectedInfo->Retain();
	}
	return fCollectedInfo;
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

VIndexInfoProvider::VIndexInfoProvider(IndexAction& inIndexAction) :
VDB4DBasicInfoCollector(nil),
fIndexInfo(nil),
fCollectedInfo(nil),
fLockInfo(inIndexAction.fIndexRequestOriginatorInfo)
{
	fIndexInfo = inIndexAction.Ind;
}

VIndexInfoProvider::~VIndexInfoProvider() 
{
	XBOX::ReleaseRefCountable(&fCollectedInfo);
}


XBOX::VJSONObject*	VIndexInfoProvider::CollectInfo(VProgressIndicator* inIndicator, bool inForRootSession)
{
	if (fCollectedInfo == nil)
	{
		fCollectedInfo = new XBOX::VJSONObject();
		if (inForRootSession && (fLockInfo != nil))
		{
			_BuildDbContextInfoProperty(fLockInfo, fCollectedInfo);
		}
		if (fIndexInfo != nil)
		{
			XBOX::VJSONObject* extraInfo = new XBOX::VJSONObject();
			if (extraInfo != nil)
			{
				if (fCollectedInfo != nil) 
				{
					XBOX::VString tmp;
					fIndexInfo->IdentifyIndex(tmp,true,true,false);
					extraInfo->SetPropertyAsString(Properties::IndexInfo, tmp);
					fCollectedInfo->SetProperty(Properties::DbOperationDetails, XBOX::VJSONValue(extraInfo));
				}
				XBOX::ReleaseRefCountable(&extraInfo);
			}
		}
	}
	if (fCollectedInfo != nil)
	{
		fCollectedInfo->Retain();
	}
	return fCollectedInfo;
}
#else

VDB4DQueryInfoCollector::VDB4DQueryInfoCollector(CDB4DBaseContext* inContext,OptimizedQuery* inQuery) :
fBaseContext(inContext),
fQuery(inQuery),
fCollectedInfo(nil)
{
}

VDB4DQueryInfoCollector::~VDB4DQueryInfoCollector()
{
	XBOX::ReleaseRefCountable(&fCollectedInfo);
}

XBOX::VJSONObject* VDB4DQueryInfoCollector::CollectInfo(VProgressIndicator *inIndicator)
{
	if (fCollectedInfo == NULL)
	{
		if (fBaseContext != nil && fQuery != nil)
		{
			fCollectedInfo = new XBOX::VJSONObject();
			XBOX::VString tmp;
			BaseTaskInfo* context = ConvertContext(fBaseContext);
			Base4D* bd = context->GetBase();
			if (bd != nil && fQuery != nil)
			{
				fQuery->GetRoot()->FullyDescribe(bd, tmp);
				fCollectedInfo->SetPropertyAsString(CVSTR("queryPlan"), tmp);
			}
		}
	}
	if (fCollectedInfo != NULL)
	{
		//Keep one more tab on fCollectedInfo, we don't want to compute it for each call
		fCollectedInfo->Retain();
	}
	return fCollectedInfo;
}

bool VDB4DQueryInfoCollector::LoadMessageString(XBOX::VString& /*ioMessage*/)
{
	return false;
}

/////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////

VIndexInfoProvider::VIndexInfoProvider(IndexAction& inIndexAction) :
fIndexInfo(nil)
{
	fIndexInfo = inIndexAction.Ind;
}


XBOX::VJSONObject*	VIndexInfoProvider::CollectInfo(VProgressIndicator * /*inIndicator*/)
{
	XBOX::VJSONObject* extraInfo = nil;
	if (fIndexInfo != nil)
	{
		XBOX::VString tmp;
		extraInfo = new XBOX::VJSONObject();
		extraInfo->SetPropertyAsString(CVSTR("indexName"), fIndexInfo->GetName());

		fIndexInfo->GetDebugString(tmp, 0);
		extraInfo->SetPropertyAsString(CVSTR("debug"), tmp);
	}
	return extraInfo;
}

#endif //WITH_RTM_DETAILLED_ACTIVITY_INFO


