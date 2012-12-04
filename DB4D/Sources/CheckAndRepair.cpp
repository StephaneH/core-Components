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


CheckAndRepairAgent::CheckAndRepairAgent(Base4D *target)
{
	fTarget = target;
	fTarget->Retain();

	fCheckTables = true;
	fCheckAllTables = true;
	fCheckBlobs = true;
	fTablesToCheck.SetUnique(true);

	fCheckIndexes = true;
	fCheckAllIndexes = true;
	fIndexesToCheck.SetUnique(true);

	fCheckAllocatedSpace = true;

	fRecoverByTags = false;
	fTextLog = nil;
	fTextLogIsOwned = false;

}


CheckAndRepairAgent::~CheckAndRepairAgent()
{
	sLONG i,nb;

	nb = fTablesToCheck.GetCount();
	for (i=0; i<nb; i++)
	{
		Table* crit = fTablesToCheck[i];
		if (crit != nil)
		{
			crit->Release();
		}
	}

		/*
	nb = fIndexesToCheck.GetCount();
	for (i=0; i<nb; i++)
	{
		IndexInfo* ind = fIndexesToCheck[i];
		if (ind != nil)
		{
			ind->Release();
		}
	}
	*/

	if (fTextLog != nil && fTextLogIsOwned)
	{
		delete fTextLog;
	}

	fTarget->Release();

}


VError CheckAndRepairAgent::ThrowError( VError inErrCode, ActionDB4D inAction)
{
	VErrorDB4D_OnCheckAndRepair *err = new VErrorDB4D_OnCheckAndRepair(inErrCode, inAction, fTarget);
	VTask::GetCurrent()->PushRetainedError( err);
	
	return inErrCode;
}


void CheckAndRepairAgent::SetFullCheck()
{
	fCheckTables = true;
	fCheckAllTables = true;
	fCheckBlobs = true;

	fCheckIndexes = true;
	fCheckAllIndexes = true;

	fCheckAllocatedSpace = true;

	fRecoverByTags = false;
}


VError CheckAndRepairAgent::CheckOneTable(Table* target)
{
	VError err = VE_OK;
	DataTableRegular* df;

	df = (DataTableRegular*)target->GetDF();
	if (df != nil)
	{
		Boolean ok = df->LockTable(fContext);
		if (ok)
		{
			err = df->CheckData(this);

			df->UnLockTable(fContext);
		}
		else
		{
			err = ThrowError(VE_DB4D_TABLEISLOCKED, DBaction_CheckingTable);
		}
	}

	return err;
}


VError CheckAndRepairAgent::RunCheckTables()
{
	VError err = VE_OK;
	sLONG i,nb;

	if (fCheckAllTables)
	{
		nb = fTarget->GetNBTable();
		for (i=1; i<=nb && err == VE_OK; i++)
		{
			Table* crit = fTarget->RetainTable(i);
			if (crit != nil)
			{
				err = CheckOneTable(crit);
			}
		}
	}
	else
	{
		nb = fTablesToCheck.GetCount();
		for (i=0; i<nb && err == VE_OK; i++)
		{
			Table* crit = fTablesToCheck[i];
			if (crit != nil)
			{
				err = CheckOneTable(crit);
			}
		}
	}

	return err;
}


VError CheckAndRepairAgent::Run(VStream* outMsg, ListOfErrors& OutList, BaseTaskInfo* Context, VDB4DProgressIndicator* InProgress)
{
	VError err = VE_OK;

	fProgress = InProgress;
	fContext = Context;
	fContext->Retain();

	if (fTextLog != nil && fTextLogIsOwned)
	{
		delete fTextLog;
	}

	fTextLog = outMsg;
	if (fTextLog == nil)
	{
		fTextLogIsOwned = true;
		fTextLog = new xbox::VPtrStream();
	}

	fErrors = &OutList;
	
	fTextLog->OpenWriting();

	if (fTarget->Lock(Context, false))
	{
		// err = fTarget->WaitForAllPendingLocked(time, WhatIsStillLocked);
		if (err == VE_OK)
		{
			// err = fTarget->ForceFlush();
			if (err == VE_OK)
			{
				if (fCheckTables)
				{
					err = RunCheckTables();
				}
			}
		}
		fTarget->UnLock(Context);
	}
	else
		// message dans log pas possible car base locked

	fTextLog->CloseWriting();

	fContext->Release();
	return err;
}




