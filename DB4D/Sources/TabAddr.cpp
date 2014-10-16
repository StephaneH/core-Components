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




TreeMem::TreeMem(TreeMemHeader* owner)
{
	fOwner = owner;
	std::fill(&TabMem[0], &TabMem[kNbElemTreeMem], (void*)nil);
#if debugTreeMem_Strong
	fOwner->debug_AddPage(this);
#endif
#if debugLeakCheck_TreeMem
	if (debug_candumpleaks)
		DumpStackCrawls();
	if (debug_canRegisterLeaks)
		RegisterStackCrawl(this);
#endif
}


#if debugTreeMem_Strong || debugLeakCheck_TreeMem || debuglr
TreeMem::~TreeMem()
{
	for (sLONG i =0; i < kNbElemTreeMem; i++)
	{
		if (TabMem[i] != nil)
		{
			xbox_assert(TabMem[i] == nil);
		}
	}
#if debugTreeMem_Strong
	fOwner->debug_DelPage(this);
#endif

#if debugLeakCheck_TreeMem
	UnRegisterStackCrawl(this);
#endif

}
#endif


void* TreeMem::DelFrom(sLONG nbelem, sLONG n, OccupableStack* curstack)
{
	sLONG n1,xn2;
	TreeMem* sous;
	void* result = nil;

	Occupy(curstack, false);
	gOccPool.EndWaitForMemoryManager(curstack);

	n1=n>>kRatioTabAddr;
	xn2=n & (kNbElemTreeMem-1);

	if (nbelem>=kNbElemTreeMem)
	{
		if (nbelem>=kNbElemTreeMem2)
		{
			n1=n1>>kRatioTreeMem;
			xn2=n & (kNbElemTreeMem2-1);
			nbelem=kNbElemTreeMem2-1;
		}
		else
		{
			nbelem=kNbElemTreeMem-1;
		}
		gOccPool.WaitForMemoryManager(curstack);
		sous=(TreeMem*)(TabMem[n1]);
		if (sous!=nil)
		{
			result = sous->DelFrom(nbelem, xn2, curstack);
		}
		else
		{
			gOccPool.EndWaitForMemoryManager(curstack);
			result = nil;
		}
	}
	else
	{
		//gOccPool.WaitForMemoryManager(curstack);
		result = TabMem[xn2];
		TabMem[xn2] = nil;
		//gOccPool.EndWaitForMemoryManager(curstack);
	}

	Free(curstack, true);

	return result;
}



void TreeMem::PurgeFrom(sLONG nbelem, sLONG n, OccupableStack* curstack)
{
	sLONG n1,xn2;
	TreeMem* sous;
	void* result = nil;

	Occupy(curstack, false);
	gOccPool.EndWaitForMemoryManager(curstack);

	n1=n>>kRatioTabAddr;
	xn2=n & (kNbElemTreeMem-1);

	if (nbelem>=kNbElemTreeMem)
	{
		if (nbelem>=kNbElemTreeMem2)
		{
			n1=n1>>kRatioTreeMem;
			xn2=n & (kNbElemTreeMem2-1);
			nbelem=kNbElemTreeMem2-1;
		}
		else
		{
			nbelem=kNbElemTreeMem-1;
		}
		gOccPool.WaitForMemoryManager(curstack);
		sous=(TreeMem*)(TabMem[n1]);
		if (sous!=nil)
		{
			sous->PurgeFrom(nbelem, xn2, curstack);
		}
		else
		{
			result = nil;
			gOccPool.EndWaitForMemoryManager(curstack);
		}
	}
	else
	{
		//gOccPool.WaitForMemoryManager(curstack);
		result = TabMem[xn2];
		if (result != nil)
			fOwner->KillElem(result);
		TabMem[xn2] = nil;
		//gOccPool.EndWaitForMemoryManager(curstack);
	}

	Free(curstack, true);

}


bool TreeMem::TryToPurge(sLONG nbelem, OccupableStack* curstack)
{
	bool purged = true;
	sLONG i,n,xn2;
	TreeMem* sous;

	Occupy(curstack, false);
	gOccPool.EndWaitForMemoryManager(curstack);

	if (nbelem>=kNbElemTreeMem)
	{
		if (nbelem>=kNbElemTreeMem2)
		{
			n = (nbelem+kNbElemTreeMem2-1) >> kRatioTreeMem2;
			for (i=0;i<n;i++)
			{
				xn2 = kNbElemTreeMem2 - 1;

				gOccPool.WaitForMemoryManager(curstack);
				sous = (TreeMem*)(TabMem[i]);
				if (sous!=nil)
				{
					if (sous->TryToPurge(xn2, curstack))
					{
						delete sous;
						TabMem[i] = nil;
					}
					else
					{
						purged = false;
					}
				}
				else
					gOccPool.EndWaitForMemoryManager(curstack);

			}
		}
		else
		{
			n = (nbelem+kNbElemTreeMem-1) >> kRatioTreeMem;
			for (i=0;i<n;i++)
			{
				xn2 = kNbElemTreeMem-1;

				gOccPool.WaitForMemoryManager(curstack);
				sous = (TreeMem*)(TabMem[i]);
				if (sous!=nil)
				{
					if (sous->TryToPurge(xn2, curstack))
					{
						delete sous;
						TabMem[i] = nil;
					}
					else
					{
						purged = false;
					}
				}
				else
					gOccPool.EndWaitForMemoryManager(curstack);

			}
		}
	}
	else
	{
		gOccPool.WaitForMemoryManager(curstack);
		for (i=0;i<=nbelem;i++) 
		{
			void *obj = TabMem[i];

			if (obj != nil)
			{
				purged = purged && fOwner->IsElemPurged(obj);
			}
		}
		gOccPool.EndWaitForMemoryManager(curstack);
	}

	Free(curstack, true);

	return purged;
}


void TreeMem::LiberePourClose(sLONG nbelem, OccupableStack* curstack)
{
	sLONG i,n,xn2;
	TreeMem* sous;

	Occupy(curstack, false);
	gOccPool.EndWaitForMemoryManager(curstack);

	if (nbelem>=kNbElemTreeMem)
	{
		if (nbelem>=kNbElemTreeMem2)
		{
			n = (nbelem+kNbElemTreeMem2-1) >> kRatioTreeMem2;
			for (i=0;i<n;i++)
			{
				xn2 = kNbElemTreeMem2-1;

				gOccPool.WaitForMemoryManager(curstack);
				sous = (TreeMem*)(TabMem[i]);
				if (sous!=nil)
				{
					sous->LiberePourClose(xn2, curstack);
					delete sous;
					TabMem[i] = nil;
				}
				else
					gOccPool.EndWaitForMemoryManager(curstack);
			}
		}
		else
		{
			n = (nbelem+kNbElemTreeMem-1) >> kRatioTreeMem;
			for (i=0;i<n;i++)
			{
				xn2 = kNbElemTreeMem-1;

				gOccPool.WaitForMemoryManager(curstack);
				sous = (TreeMem*)(TabMem[i]);
				if (sous!=nil)
				{
					sous->LiberePourClose(xn2, curstack);
					delete sous;
					TabMem[i] = nil;
				}
				else
					gOccPool.EndWaitForMemoryManager(curstack);
			}
		}
	}
	else
	{
		//gOccPool.WaitForMemoryManager(curstack);
		for (i=0;i<=nbelem;i++) 
		{
			void *obj = TabMem[i];
			TabMem[i]=nil;

			if (obj != nil)
			{
				fOwner->KillElem(obj);
			}
		}
		//gOccPool.EndWaitForMemoryManager(curstack);
	}

	Free(curstack, true);

}


void* TreeMem::GetFrom(sLONG nbelem, sLONG n, OccupableStack* curstack)
{
	sLONG n1,xn2;
	TreeMem* sous;
	void* result;

	xbox_assert( n >= 0 && n <= nbelem); // L.E. 23/01/01 actually don't know if 0 is ok

	Occupy(curstack, false);
	gOccPool.EndWaitForMemoryManager(curstack);

	result=nil;

	if (n >= 0 && n <= nbelem)
	{
		n1=n>>kRatioTabAddr;
		xn2=n & (kNbElemTreeMem-1);

		if (nbelem>=kNbElemTreeMem)
		{
			if (nbelem>=kNbElemTreeMem2)
			{
				n1=n1>>kRatioTreeMem;
				xn2=n & (kNbElemTreeMem2-1);
				nbelem=kNbElemTreeMem2-1;
			}
			else
			{
				nbelem=kNbElemTreeMem-1;
			}

			gOccPool.WaitForMemoryManager(curstack);
			sous=(TreeMem*)(TabMem[n1]);
			if (sous!=nil)
			{
				result=sous->GetFrom(nbelem,xn2,curstack);
			}
			else
				gOccPool.EndWaitForMemoryManager(curstack);
		}
		else
		{
		//	gOccPool.WaitForMemoryManager(curstack);
			result=TabMem[xn2];
		//	gOccPool.EndWaitForMemoryManager(curstack);
		}
	}

	Free(curstack, true);

	return(result);
}



void* TreeMem::GetFromAndOccupy(sLONG nbelem, sLONG n, OccupableStack* curstack)
{
	sLONG n1,xn2;
	TreeMem* sous;
	void* result;

	xbox_assert( n >= 0 && n <= nbelem); // L.E. 23/01/01 actually don't know if 0 is ok

	Occupy(curstack, false);
	gOccPool.EndWaitForMemoryManager(curstack);

	result=nil;

	if (n >= 0 && n <= nbelem)
	{
		n1=n>>kRatioTabAddr;
		xn2=n & (kNbElemTreeMem-1);

		if (nbelem>=kNbElemTreeMem)
		{
			if (nbelem>=kNbElemTreeMem2)
			{
				n1=n1>>kRatioTreeMem;
				xn2=n & (kNbElemTreeMem2-1);
				nbelem=kNbElemTreeMem2-1;
			}
			else
			{
				nbelem=kNbElemTreeMem-1;
			}

			gOccPool.WaitForMemoryManager(curstack);
			sous=(TreeMem*)(TabMem[n1]);
			if (sous!=nil)
			{
				result=sous->GetFromAndOccupy(nbelem,xn2,curstack);
			}
			else
				gOccPool.EndWaitForMemoryManager(curstack);
		}
		else
		{
			//gOccPool.WaitForMemoryManager(curstack);
			result=TabMem[xn2];
			if (result != nil)
				fOwner->OccupyElem(result, curstack);
			
			//gOccPool.EndWaitForMemoryManager(curstack);
		}
	}

	Free(curstack, true);

	return(result);
}



void* TreeMem::RetainFrom(sLONG nbelem, sLONG n, OccupableStack* curstack)
{
	sLONG n1,xn2;
	TreeMem* sous;
	void* result;

	xbox_assert( n >= 0 && n <= nbelem); // L.E. 23/01/01 actually don't know if 0 is ok

	Occupy(curstack, false);
	gOccPool.EndWaitForMemoryManager(curstack);

	result=nil;

	if (n >= 0 && n <= nbelem)
	{
		n1=n>>kRatioTabAddr;
		xn2=n & (kNbElemTreeMem-1);

		if (nbelem>=kNbElemTreeMem)
		{
			if (nbelem>=kNbElemTreeMem2)
			{
				n1=n1>>kRatioTreeMem;
				xn2=n & (kNbElemTreeMem2-1);
				nbelem=kNbElemTreeMem2-1;
			}
			else
			{
				nbelem=kNbElemTreeMem-1;
			}
			gOccPool.WaitForMemoryManager(curstack);
			sous=(TreeMem*)(TabMem[n1]);
			if (sous!=nil)
			{
				result=sous->RetainFrom(nbelem,xn2,curstack);
			}
			else
				gOccPool.EndWaitForMemoryManager(curstack);
		}
		else
		{
			//gOccPool.WaitForMemoryManager(curstack);
			result=TabMem[xn2];
			if (result != nil)
				fOwner->RetainElem(result);
			//gOccPool.EndWaitForMemoryManager(curstack);
		}
	}

	Free(curstack, true);

	return(result);
}


VError TreeMem::PutInto(sLONG nbelem, sLONG n, void* data, OccupableStack* curstack)
{
	sLONG n1,xn2;
	VError err = VE_OK;
	TreeMem* sous;

	Occupy(curstack, false);
	gOccPool.EndWaitForMemoryManager(curstack);

	n1=n>>kRatioTabAddr;
	xn2=n & (kNbElemTreeMem-1);

	if (nbelem>=kNbElemTreeMem)
	{
		if (nbelem>=kNbElemTreeMem2)
		{
			n1=n1>>kRatioTreeMem;
			xn2=n & (kNbElemTreeMem2-1);
			nbelem=kNbElemTreeMem2-1;
		}
		else
		{
			nbelem=kNbElemTreeMem-1;
		}

		bool recheck;
		do
		{
			recheck = false;
			gOccPool.WaitForMemoryManager(curstack);
			sous=(TreeMem*)(TabMem[n1]);
			if (sous==nil)
			{
				recheck = true;
				gOccPool.EndWaitForMemoryManager(curstack);
				{
					VTaskLock lock(&fAlterTreeMutex);
					sous = (TreeMem*)(TabMem[n1]); // peut avoir change car n'etait pas encore protege par mutex
					
					if (sous == nil)
					{
						sous=fOwner->CreTreeMem();
						if (sous==nil)
						{
							err = ThrowBaseError(memfull, DBaction_StoringObjectInCache);
							recheck = false;
						}
						else
						{
							xbox_assert(TabMem[n1] == nil);
							TabMem[n1]=(void*)sous;
						}
					}
				}

			}
		} while (recheck);

		if (err==VE_OK)
		{
			err=sous->PutInto(nbelem,xn2,data,curstack);
		}

	}
	else
	{
		// 64-bit assignment is guaranteed atomic only if aligned on 64-bit bounday
		assert_compile( offsetof( TreeMem, TabMem) % sizeof(void*) == 0);
		xbox_assert( ((char*)&TabMem[0] - (char*)0) % sizeof(void*) == 0);
		TabMem[xn2]=data;
	}

	Free(curstack, true);

	return(err);
}



VError TreeMem::ReplaceInto(sLONG nbelem, sLONG n, void* data, OccupableStack* curstack)
{
	sLONG n1,xn2;
	VError err = VE_OK;
	TreeMem* sous;

	Occupy(curstack, false);
	gOccPool.EndWaitForMemoryManager(curstack);

	n1=n>>kRatioTabAddr;
	xn2=n & (kNbElemTreeMem-1);

	if (nbelem>=kNbElemTreeMem)
	{
		if (nbelem>=kNbElemTreeMem2)
		{
			n1=n1>>kRatioTreeMem;
			xn2=n & (kNbElemTreeMem2-1);
			nbelem=kNbElemTreeMem2-1;
		}
		else
		{
			nbelem=kNbElemTreeMem-1;
		}

		bool recheck;
		do
		{
			recheck = false;
			gOccPool.WaitForMemoryManager(curstack);
			sous=(TreeMem*)(TabMem[n1]);
			if (sous==nil)
			{
				recheck = true;
				gOccPool.EndWaitForMemoryManager(curstack);
				{
					VTaskLock lock(&fAlterTreeMutex);
					sous = (TreeMem*)(TabMem[n1]); // peut avoir change car n'etait pas encore protege par mutex
					
					if (sous == nil)
					{
						sous=fOwner->CreTreeMem();
						if (sous==nil)
						{
							err = ThrowBaseError(memfull, DBaction_StoringObjectInCache);
							recheck = false;
						}
						else
						{
							xbox_assert(TabMem[n1] == nil);
							TabMem[n1]=(void*)sous;
						}
					}
				}

			}
		} while (recheck);

		if (err==VE_OK)
		{
			err=sous->ReplaceInto(nbelem,xn2,data,curstack);
		}
	}
	else
	{
		//gOccPool.WaitForMemoryManager(curstack);
		void* old = TabMem[xn2];
		if (data != old)
		{
			if (data != nil)
			{
				fOwner->RetainElem(data);
			}
			if (old != nil)
			{
				fOwner->KillElem(old);
			}
			TabMem[xn2]=data;
		}
		//gOccPool.EndWaitForMemoryManager(curstack);
	}

	Free(curstack, true);

	return(err);
}



bool TreeMem::TryToFreeMem(sLONG allocationBlockNumber, sLONG nbelem, VSize combien, VSize& outFreed)
{
	bool allfreed = true;
	VSize tot;
	bool isoccupied = IsOccupied();

	tot = 0;

	//if (!IsOccupied())
	{
		if (combien == 0) 
			combien = kMaxPositif;

		if (nbelem >= kNbElemTreeMem)
		{
			sLONG n, xn2;

			if (nbelem>=kNbElemTreeMem2)
			{
				n = (nbelem+kNbElemTreeMem2-1) >> kRatioTreeMem2;
				xn2 = kNbElemTreeMem2-1;

				if (xn2 < kNbElemTreeMem)
					xn2 = kNbElemTreeMem; // celui ci est il necessaire ?
			}
			else
			{
				n = (nbelem+kNbElemTreeMem-1) >> kRatioTreeMem;
				xn2 = kNbElemTreeMem-1;
			}

			for (sLONG i=0;i<n;i++)
			{
				TreeMem* sous = (TreeMem*)(TabMem[i]);
				if (sous!=nil)
				{
					VSize tot2;
					if (sous->TryToFreeMem(allocationBlockNumber, xn2, combien-tot, tot2) && !isoccupied)
					{
						tot = tot + sizeof(TreeMem);
						delete sous;
						//TabMem[i] = nil;
						VInterlocked::ExchangePtr(&(TabMem[i]), (void*)nil);
						/*
						if (tot >= combien)
						{
							if (i != n-1)
								allfreed = false;
							break;
						}
						*/
					}
					else
					{
						allfreed = false;
						/*
						if (tot >= combien)
							break;
							*/
					}
					tot = tot + tot2;
				}
			}
		}
		else
		{
			if (!isoccupied)
			{
				for (sLONG i=0;i<=nbelem;i++) {
					void *obj = TabMem[i];

					if (obj != nil)
					{
						VSize tot2;
						if (fOwner->TryToFreeElem(allocationBlockNumber, obj, tot2))
						{
							tot = tot + tot2;
							//TabMem[i] = nil;
							VInterlocked::ExchangePtr(&(TabMem[i]), (void*)nil);
							/*
							if (tot >= combien)
							{
								if (i != nbelem)
									allfreed = false;
								break;
							}
							*/
						}
						else
							allfreed = false;
					}
				}
			}
			else allfreed = false;
		}
	}
	outFreed = tot;

	if (IsOccupied())
		allfreed = false;

	return allfreed;
}



VError TreeMem::PerformRech(Bittab* ReelFiltre, sLONG nbelem, RechNode* rn, Bittab *cursel, Bittab *filtre, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, 
							  DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, sLONG& nbfound, Bittab &Nulls, Bittab* dejalocked, OccupableStack* curstack, LocalEntityModel* model)
{
	VError err = VE_OK;
	sLONG i,n,xn2;
	TreeMem* sous;
	Occupy(curstack, false);
	gOccPool.EndWaitForMemoryManager(curstack);

	if (nbelem>=kNbElemTreeMem)
	{
		if (nbelem>=kNbElemTreeMem2)
		{
			n = (nbelem+kNbElemTreeMem2-1) >> kRatioTreeMem2;
			for (i=0;i<n && err == VE_OK;i++)
			{
				xn2 = kNbElemTreeMem2-1;

				gOccPool.WaitForMemoryManager(curstack);
				sous = (TreeMem*)(TabMem[i]);
				if (sous!=nil)
				{
					err = sous->PerformRech(ReelFiltre, xn2, rn, cursel, filtre, InProgress, context, HowToLock, exceptions, limit, nbfound, Nulls, dejalocked, curstack, model);
					if (limit>0 && nbfound>=limit)
						break;
				}
				else
					gOccPool.EndWaitForMemoryManager(curstack);
			}
		}
		else
		{
			n = (nbelem+kNbElemTreeMem-1) >> kRatioTreeMem;
			for (i=0;i<n && err == VE_OK;i++)
			{
				xn2 = kNbElemTreeMem-1;

				gOccPool.WaitForMemoryManager(curstack);
				sous = (TreeMem*)(TabMem[i]);
				if (sous!=nil)
				{
					err = sous->PerformRech(ReelFiltre, xn2, rn, cursel, filtre, InProgress, context, HowToLock, exceptions, limit, nbfound, Nulls, dejalocked, curstack, model);
					if (limit>0 && nbfound>=limit)
						break;
				}
				else
					gOccPool.EndWaitForMemoryManager(curstack);
			}
		}
	}
	else
	{
		for (i=0;i<=nbelem && err == VE_OK;i++) 
		{
			gOccPool.WaitForMemoryManager(curstack);
			FicheOnDisk *obj = (FicheOnDisk*)(TabMem[i]);

			if (obj!=nil)
			{
				StOccupy lock(obj, curstack, false);
				gOccPool.EndWaitForMemoryManager(curstack);
				if (ReelFiltre == nil || ReelFiltre->isOn(obj->getnumfic()))
				{
					filtre->Set(obj->getnumfic());
					if (InProgress != nil)
					{
						InProgress->Increment();
					}
					Boolean ok = rn->PerformSeq(obj, context, InProgress, context, HowToLock, exceptions, limit, model);
					if (ok == 2)
					{
						err = Nulls.Set(obj->getnumfic(), true);
					}
					else
					{
						if (ok)
						{
							if (HowToLock == DB4D_Keep_Lock_With_Transaction)
							{
								if (!obj->GetOwner()->LockRecord(obj->getnumfic(), context, DB4D_Keep_Lock_With_Transaction))
								{
									if (exceptions != nil)
										exceptions->Set(obj->getnumfic(), true);
									err =  obj->ThrowError(VE_DB4D_RECORDISLOCKED, DBaction_ExecutingQuery);
								}
							}

							if (err == VE_OK)
							{
								cursel->Set(obj->getnumfic());
								nbfound++;
								if (limit>0 && nbfound>=limit)
								{
									break;
								}
							}
						}
					}
				}
			}
			else
				gOccPool.EndWaitForMemoryManager(curstack);

		}
	}

	Free(curstack, true);
	return err;
}



VError TreeMem::PerformRech(Bittab* ReelFiltre, sLONG nbelem, SimpleQueryNode* rn, Bittab *cursel, Bittab *filtre, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, 
							  DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, sLONG& nbfound, Bittab &Nulls, Bittab* dejalocked, OccupableStack* curstack)
{
	VError err = VE_OK;
	sLONG i,n,xn2;
	TreeMem* sous;
	Occupy(curstack, false);
	gOccPool.EndWaitForMemoryManager(curstack);

	if (nbelem>=kNbElemTreeMem)
	{
		if (nbelem>=kNbElemTreeMem2)
		{
			n = (nbelem+kNbElemTreeMem2-1) >> kRatioTreeMem2;
			for (i=0;i<n && err == VE_OK;i++)
			{
				xn2 = kNbElemTreeMem2-1;

				gOccPool.WaitForMemoryManager(curstack);
				sous = (TreeMem*)(TabMem[i]);
				if (sous!=nil)
				{
					err = sous->PerformRech(ReelFiltre, xn2, rn, cursel, filtre, InProgress, context, HowToLock, exceptions, limit, nbfound, Nulls, dejalocked, curstack);
					if (limit>0 && nbfound>=limit)
						break;
				}
				else
					gOccPool.EndWaitForMemoryManager(curstack);
			}
		}
		else
		{
			n = (nbelem+kNbElemTreeMem-1) >> kRatioTreeMem;
			for (i=0;i<n && err == VE_OK;i++)
			{
				xn2 = kNbElemTreeMem-1;

				gOccPool.WaitForMemoryManager(curstack);
				sous = (TreeMem*)(TabMem[i]);
				if (sous!=nil)
				{
					err = sous->PerformRech(ReelFiltre, xn2, rn, cursel, filtre, InProgress, context, HowToLock, exceptions, limit, nbfound, Nulls, dejalocked, curstack);
					if (limit>0 && nbfound>=limit)
						break;
				}
				else
					gOccPool.EndWaitForMemoryManager(curstack);
			}
		}
	}
	else
	{
		for (i=0;i<=nbelem && err == VE_OK;i++) 
		{
			gOccPool.WaitForMemoryManager(curstack);
			FicheOnDisk *obj = (FicheOnDisk*)(TabMem[i]);

			if (obj!=nil)
			{
				StOccupy lock(obj, curstack, false);
				{
					gOccPool.EndWaitForMemoryManager(curstack);
					if (ReelFiltre == nil || ReelFiltre->isOn(obj->getnumfic()))
					{
						filtre->Set(obj->getnumfic());
						if (InProgress != nil)
						{
							InProgress->Increment();
						}
						Boolean ok = rn->PerformSeq(obj, context, InProgress, context, HowToLock, exceptions, limit);
						if (ok == 4)
						{
							err = -1;
						}
						else
						{
							if (ok == 2)
							{
								err = Nulls.Set(obj->getnumfic(), true);
							}
							else
							{
								if (ok)
								{
									if (HowToLock == DB4D_Keep_Lock_With_Transaction)
									{
										if (!obj->GetOwner()->LockRecord(obj->getnumfic(), context, DB4D_Keep_Lock_With_Transaction))
										{
											if (exceptions != nil)
												exceptions->Set(obj->getnumfic(), true);
											err = obj->ThrowError(VE_DB4D_RECORDISLOCKED, DBaction_ExecutingQuery);
											break;
										}
									}
									cursel->Set(obj->getnumfic());
									nbfound++;
									if (limit>0 && nbfound>=limit)
									{
										break;
									}
								}
							}
						}
					}

				}
			}
			else
				gOccPool.EndWaitForMemoryManager(curstack);
		}
	}

	Free(curstack, true);
	return err;
}


void TreeMem::CalculateFormulaFromCache(sLONG nbelem, ColumnFormulas* Formulas, Bittab* filtre, Bittab& outDeja, BaseTaskInfo* context, OccupableStack* curstack)
{
	sLONG i,n,xn2;
	TreeMem* sous;
	Occupy(curstack, false);
	gOccPool.EndWaitForMemoryManager(curstack);

	if (nbelem>=kNbElemTreeMem)
	{
		if (nbelem>=kNbElemTreeMem2)
		{
			n = (nbelem+kNbElemTreeMem2-1) >> kRatioTreeMem2;
			for (i=0;i<n;i++)
			{
				xn2 = kNbElemTreeMem2-1;

				gOccPool.WaitForMemoryManager(curstack);
				sous = (TreeMem*)(TabMem[i]);
				if (sous!=nil)
				{
					sous->CalculateFormulaFromCache(xn2, Formulas, filtre, outDeja, context, curstack );
				}
				else
					gOccPool.EndWaitForMemoryManager(curstack);
			}
		}
		else
		{
			n = (nbelem+kNbElemTreeMem-1) >> kRatioTreeMem;
			for (i=0;i<n;i++)
			{
				xn2 = kNbElemTreeMem-1;

				gOccPool.WaitForMemoryManager(curstack);
				sous = (TreeMem*)(TabMem[i]);
				if (sous!=nil)
				{
					sous->CalculateFormulaFromCache(xn2, Formulas, filtre, outDeja, context, curstack );
				}
				else
					gOccPool.EndWaitForMemoryManager(curstack);
			}
		}
	}
	else
	{
		for (i=0; i<=nbelem; i++) 
		{
			gOccPool.WaitForMemoryManager(curstack);
			FicheOnDisk *obj = (FicheOnDisk*)(TabMem[i]);

			if (obj!=nil)
			{
				StOccupy lock(obj, curstack, false);
				{
					gOccPool.EndWaitForMemoryManager(curstack);
					if (filtre->isOn(obj->getnumfic()))
					{
						if (Formulas->ExecuteOnOneRec(obj, context))
							outDeja.Set(obj->getnumfic());
					}
				}
			}
			else
				gOccPool.EndWaitForMemoryManager(curstack);
		}
	}

	Free(curstack, true);
}


VError TreeMem::FillCollection(sLONG recnum, sLONG nbelem, DB4DCollectionManager& collection, Bittab& dejapris, SelPosIterator& PosIter, BaseTaskInfo* context, VArrayRetainedOwnedPtrOf<Field*>& cols, OccupableStack* curstack)
{
	VError err = VE_OK;
	sLONG i,n,xn2;
	TreeMem* sous;
	Occupy(curstack, false);
	gOccPool.EndWaitForMemoryManager(curstack);

	if (nbelem>=kNbElemTreeMem)
	{
		if (nbelem>=kNbElemTreeMem2)
		{
			n = (nbelem+kNbElemTreeMem2-1) >> kRatioTreeMem2;
			for (i=0; i<n && err == VE_OK; i++)
			{
				xn2 = kNbElemTreeMem2-1;
				if (PosIter.GetCurRecNum() <= (recnum + (i+1) * kNbElemTreeMem2))
				{
					gOccPool.WaitForMemoryManager(curstack);
					sous = (TreeMem*)(TabMem[i]);
					if (sous!=nil)
					{
						err = sous->FillCollection(recnum + i * kNbElemTreeMem2, xn2, collection, dejapris, PosIter, context, cols, curstack);
						if (PosIter.ReachedEnd())
							break;
					}
					else
						gOccPool.EndWaitForMemoryManager(curstack);
				}
			}
		}
		else
		{
			n = (nbelem+kNbElemTreeMem-1) >> kRatioTreeMem;
			for (i=0; i<n && err == VE_OK; i++)
			{
				xn2 = kNbElemTreeMem-1;
				if (PosIter.GetCurRecNum() <= (recnum + (i+1) * kNbElemTreeMem))
				{
					gOccPool.WaitForMemoryManager(curstack);
					sous = (TreeMem*)(TabMem[i]);
					if (sous!=nil)
					{
						err = sous->FillCollection(recnum + i * kNbElemTreeMem, xn2, collection, dejapris, PosIter, context, cols, curstack);
						if (PosIter.ReachedEnd())
							break;
					}
					else
						gOccPool.EndWaitForMemoryManager(curstack);
				}
			}
		}
	}
	else // we are in a leaf
	{
		sLONG nbcol = cols.GetCount();

		for (i=0; i<=nbelem && err == VE_OK; i++, recnum++) 
		{
			if (PosIter.ReachedEnd())
				break;
			sLONG nextrec = PosIter.GetCurRecNum();
			while (recnum > nextrec)
			{
				PosIter.SkippedOne();
				PosIter.NextPos();
				nextrec = PosIter.GetCurRecNum();
				if (PosIter.ReachedEnd())
					break;
			}
			if (recnum == nextrec)
			{
				Boolean okpris = false;
				gOccPool.WaitForMemoryManager(curstack);
				FicheOnDisk *obj = (FicheOnDisk*)(TabMem[i]);

				if (obj!=nil)
				{
					StOccupy lock(obj, curstack, false);
					{
						gOccPool.EndWaitForMemoryManager(curstack);
						sLONG posinarray = PosIter.GetCurSelPos();
						bool reject = false;

						for (sLONG j = 0; (j < nbcol) && (err == VE_OK) && !reject; j++)
						{
							Field* cri = cols[j];
							if (cri == nil || cri->GetPosInRec() == 0)
							{
								err = collection.SetNthElementRawData(posinarray+1, j+1, &recnum, VK_LONG, &reject);
							}
							else
							{
								sLONG fieldDataType;
								tPtr fieldData = obj->GetDataPtrForQuery( cri->GetPosInRec(), &fieldDataType, true, false);
								if (fieldDataType == VK_TEXT) // les blobs ne sont pas supportes par GetDataPtrForQuery
									reject = true;
								else
									err = collection.SetNthElementRawData(posinarray+1, j+1, fieldData, (ValueKind) fieldDataType, &reject);
							}
						}

						if (!reject)
						{
							dejapris.Set(nextrec);
							okpris = true;
						}
					}
				}
				else
					gOccPool.EndWaitForMemoryManager(curstack);

				PosIter.NextPos();
				if (!PosIter.ReachedEnd() && PosIter.GetCurRecNum() == recnum)
				{
					--i;
					--recnum;
				}
				if (!okpris)
					PosIter.SkippedOne();
			}
		}
	}

	Free(curstack, true);
	return err;
}


void TreeMem::FillArrayFromCache(sLONG nbelem, void* &into, sLONG sizeelem, Bittab* filtre, Bittab& outDeja, const xMultiFieldDataOffsets& criterias, OccupableStack* curstack, Bittab* nulls)
{
	sLONG i,n,xn2;
	TreeMem* sous;
	Occupy(curstack, false);
	gOccPool.EndWaitForMemoryManager(curstack);

	if (nbelem>=kNbElemTreeMem)
	{
		if (nbelem>=kNbElemTreeMem2)
		{
			n = (nbelem+kNbElemTreeMem2-1) >> kRatioTreeMem2;
			for (i=0;i<n;i++)
			{
				xn2 = kNbElemTreeMem2-1;
				gOccPool.WaitForMemoryManager(curstack);
				sous = (TreeMem*)(TabMem[i]);
				if (sous!=nil)
				{
					sous->FillArrayFromCache(xn2, into, sizeelem, filtre, outDeja, criterias, curstack, nulls);
				}
				else
					gOccPool.EndWaitForMemoryManager(curstack);
			}
		}
		else
		{
			n = (nbelem+kNbElemTreeMem-1) >> kRatioTreeMem;
			for (i=0;i<n;i++)
			{
				xn2 = kNbElemTreeMem-1;
				gOccPool.WaitForMemoryManager(curstack);
				sous = (TreeMem*)(TabMem[i]);
				if (sous!=nil)
				{
					sous->FillArrayFromCache(xn2, into, sizeelem, filtre, outDeja, criterias, curstack, nulls);
				}
				else
					gOccPool.EndWaitForMemoryManager(curstack);
			}
		}
	}
	else // we are in a leaf
	{
		if (criterias.IsMulti())
		{
			TypeSortElem<xMultiFieldData> *into2 = (TypeSortElem<xMultiFieldData>*)into;
			for (i=0;i<=nbelem;i++) 
			{
				gOccPool.WaitForMemoryManager(curstack);
				FicheOnDisk *obj = (FicheOnDisk*)(TabMem[i]);

				if (obj!=nil)
				{
					StOccupy lock(obj, curstack, false);
					{
						gOccPool.EndWaitForMemoryManager(curstack);
						sLONG numrec = obj->getnumfic();
						if (filtre->isOn(numrec))
						{
							outDeja.Set(numrec);
							into2->recnum = numrec;
							into2->value.SetHeader(&criterias);

							bool isnull = false;
							xMultiFieldDataOffset_constiterator cur = criterias.Begin(), end = criterias.End();
							for (; cur != end; cur++)
							{
								sLONG requestedType = cur->GetDataType(), numfield = cur->GetNumCrit(), size = cur->GetSize();
								char* posdata = into2->value.GetDataPtr(cur->GetOffset());
								sLONG TypeOnDisk;
								void* data = (void*)(obj->GetDataPtrForQuery(numfield, &TypeOnDisk));
								if (data == nil)
								{
									isnull = true;
									break;
								}
								else
								{
									switch (requestedType)
									{
										case VK_STRING:
											{
												xString* xs = (xString*)posdata;
												if (data != nil)
												{
													if (TypeOnDisk != requestedType)
													{
														ValPtr cv = NewValPtr(requestedType, data, TypeOnDisk, obj->GetOwner(), nil);
														if (cv != nil)
														{
															xs->SetSize(size);
															xs->CopyFromString((VString*)cv);
															delete cv;
														}
														else
															memset( xs, 0, size);
													}
													else
													{
														xs->SetSize(size);
														xs->CopyFrom(data);
													}
												}
												else
												{
													memset(xs, 0, size);
												}
											}
											break;

										case VK_STRING_UTF8:
											{
												xStringUTF8* xs = (xStringUTF8*)posdata;
												if (data != nil)
												{
													if (TypeOnDisk != requestedType)
													{
														ValPtr cv = NewValPtr(requestedType, data, TypeOnDisk, obj->GetOwner(), nil);
														if (cv != nil)
														{
															xs->SetSize(size);
															xs->CopyFromString((VString*)cv);
															delete cv;
														}
														else
															memset( xs, 0, size);
													}
													else
													{
														xs->SetSize(size);
														xs->CopyFrom(data);
													}
												}
												else
												{
													memset(xs, 0, size);
												}
											}
											break;

										case VK_BOOLEAN:
										case VK_BYTE:
											{
												if (data != nil)
												{
													if (TypeOnDisk == requestedType)
														*(uCHAR*)posdata = *((uCHAR*)data);
													else
													{
														ValPtr cv = NewValPtr(requestedType, data, TypeOnDisk, obj->GetOwner(), nil);
														if (cv != nil)
														{
															cv->WriteToPtr(posdata);
															delete cv;
														}
														else
															*(uCHAR*)posdata = 0;
													}
												}
												else
												{
													*(uCHAR*)posdata = 0;
												}
											}
											break;

										case VK_WORD:
											{
												if (data != nil)
												{
													if (TypeOnDisk == requestedType)
														*(sWORD*)posdata = *((sWORD*)data);
													else
													{
														ValPtr cv = NewValPtr(requestedType, data, TypeOnDisk, obj->GetOwner(), nil);
														if (cv != nil)
														{
															cv->WriteToPtr(posdata);
															delete cv;
														}
														else
															*(sWORD*)posdata = 0;
													}
												}
												else
												{
													*(sWORD*)posdata = 0;
												}
											}
											break;

										case VK_LONG:
											{
												if (data != nil)
												{
													if (TypeOnDisk == requestedType)
														*(sLONG*)posdata = *((sLONG*)data);
													else
													{
														ValPtr cv = NewValPtr(requestedType, data, TypeOnDisk, obj->GetOwner(), nil);
														if (cv != nil)
														{
															cv->WriteToPtr(posdata);
															delete cv;
														}
														else
															*(sLONG*)posdata = 0;
													}
												}
												else
												{
													*(sLONG*)posdata = 0;
												}
											}
											break;

										case VK_UUID:
											{
												if (data != nil)
												{
													if (TypeOnDisk == requestedType)
														*(VUUIDBuffer*)posdata = *((VUUIDBuffer*)data);
													else
													{
														ValPtr cv = NewValPtr(requestedType, data, TypeOnDisk, obj->GetOwner(), nil);
														if (cv != nil)
														{
															cv->WriteToPtr(posdata);
															delete cv;
														}
														else
															((VUUIDBuffer*)posdata)->FromLong(0);
													}
												}
												else
												{
													((VUUIDBuffer*)posdata)->FromLong(0);
												}
											}
											break;
												
										case VK_LONG8:
										case VK_DURATION:
										case VK_TIME:
										case VK_REAL:
										case VK_SUBTABLE:
										case VK_SUBTABLE_KEY:
											{
												if (data != nil)
												{
													if (TypeOnDisk == requestedType)
														*(sLONG8*)posdata = *((sLONG8*)data);
													else
													{
														ValPtr cv = NewValPtr(requestedType, data, TypeOnDisk, obj->GetOwner(), nil);
														if (cv != nil)
														{
															cv->WriteToPtr(posdata);
															delete cv;
														}
														else
															*(sLONG8*)posdata = 0;
													}
												}
												else
												{
													*(sLONG8*)posdata = 0;
												}
											}
											break;

									}

								}

							}
							if (isnull)
								nulls->Set(numrec);
							else
								into2 = TypeSortElemArray<xMultiFieldData>::NextDataElem(into2, sizeelem - 4);
						}

					}
				}
				else
					gOccPool.EndWaitForMemoryManager(curstack);

			}
			into = (void*)into2;

		}
		else
		{
			const xMultiFieldDataOffset* cur = criterias.GetOffset(0);
			sLONG requestedType = cur->GetDataType(), numfield = cur->GetNumCrit();

			if (requestedType == VK_STRING || requestedType == VK_STRING_UTF8)
			{
				if (requestedType == VK_STRING)
				{
					TypeSortElem<xString> *into2 = (TypeSortElem<xString>*)into;
					for (i=0;i<=nbelem;i++) 
					{
						gOccPool.WaitForMemoryManager(curstack);
						FicheOnDisk *obj = (FicheOnDisk*)(TabMem[i]);

						if (obj!=nil)
						{
							StOccupy lock(obj, curstack, false);
							{
								gOccPool.EndWaitForMemoryManager(curstack);
								sLONG numrec = obj->getnumfic();
								if (filtre->isOn(numrec))
								{
									outDeja.Set(numrec);
									into2->recnum = numrec;
									sLONG TypeOnDisk;
									void* data = (void*)(obj->GetDataPtrForQuery(numfield, &TypeOnDisk));
									if (data != nil)
									{
										if (TypeOnDisk != requestedType)
										{
											ValPtr cv = NewValPtr(requestedType, data, TypeOnDisk, obj->GetOwner(), nil);
											if (cv != nil)
											{
												into2->value.SetSize(sizeelem - 4);
												into2->value.CopyFromString((VString*)cv);
												delete cv;
											}
											else
												memset((((char*)into2) + 4), 0, sizeelem - 4);
										}
										else
										{
											into2->value.SetSize(sizeelem - 4);
											into2->value.CopyFrom(data);
										}
										into2 = TypeSortElemArray<xString>::NextDataElem(into2, sizeelem - 4);
									}
									else
									{
										if (nulls != nil)
											nulls->Set(numrec);
										else
										{
											memset((((char*)into2) + 4), 0, sizeelem - 4);
											into2 = TypeSortElemArray<xString>::NextDataElem(into2, sizeelem - 4);
										}

									}
								}

							}
						}
						else
							gOccPool.EndWaitForMemoryManager(curstack);

					}
					into = (void*)into2;
				}
				else
				{
					TypeSortElem<xStringUTF8> *into2 = (TypeSortElem<xStringUTF8>*)into;
					for (i=0;i<=nbelem;i++) 
					{
						gOccPool.WaitForMemoryManager(curstack);
						FicheOnDisk *obj = (FicheOnDisk*)(TabMem[i]);

						if (obj!=nil)
						{
							StOccupy lock(obj, curstack, false);
							{
								gOccPool.EndWaitForMemoryManager(curstack);
								sLONG numrec = obj->getnumfic();
								if (filtre->isOn(numrec))
								{
									outDeja.Set(numrec);
									into2->recnum = numrec;
									sLONG TypeOnDisk;
									void* data = (void*)(obj->GetDataPtrForQuery(numfield, &TypeOnDisk));
									if (data != nil)
									{
										if (TypeOnDisk != requestedType)
										{
											ValPtr cv = NewValPtr(requestedType, data, TypeOnDisk, obj->GetOwner(), nil);
											if (cv != nil)
											{
												into2->value.SetSize(sizeelem - 4);
												into2->value.CopyFromString((VString*)cv);
												delete cv;
											}
											else
												memset((((char*)into2) + 4), 0, sizeelem - 4);
										}
										else
										{
											into2->value.SetSize(sizeelem - 4);
											into2->value.CopyFrom(data);
										}
										into2 = TypeSortElemArray<xStringUTF8>::NextDataElem(into2, sizeelem - 4);
									}
									else
									{
										if (nulls != nil)
											nulls->Set(numrec);
										else
										{
											memset((((char*)into2) + 4), 0, sizeelem - 4);
											into2 = TypeSortElemArray<xStringUTF8>::NextDataElem(into2, sizeelem - 4);
										}
									}
								}

							}
						}
						else
							gOccPool.EndWaitForMemoryManager(curstack);

					}
					into = (void*)into2;
				}
			}
			else
			{
				switch (sizeelem - 4)
				{
				case 2:
					{
						TypeSortElem<sWORD> *into2 = (TypeSortElem<sWORD>*)into;
						for (i=0;i<=nbelem;i++) 
						{
							gOccPool.WaitForMemoryManager(curstack);
							FicheOnDisk *obj = (FicheOnDisk*)(TabMem[i]);

							if (obj!=nil)
							{
								StOccupy lock(obj, curstack, false);
								{
									gOccPool.EndWaitForMemoryManager(curstack);
									sLONG numrec = obj->getnumfic();
									if (filtre->isOn(numrec))
									{
										outDeja.Set(numrec);
										into2->recnum = numrec;
										sLONG TypeOnDisk;
										void* data = (void*)(obj->GetDataPtrForQuery(numfield, &TypeOnDisk));
										if (data != nil)
										{
											if (TypeOnDisk == requestedType)
												into2->value = *((sWORD*)data);
											else
											{
												ValPtr cv = NewValPtr(requestedType, data, TypeOnDisk, obj->GetOwner(), nil);
												if (cv != nil)
												{
													cv->WriteToPtr(&(into2->value));
													delete cv;
												}
												else
													into2->value = 0;
											}
											into2++;
										}
										else
										{
											if (nulls != nil)
												nulls->Set(numrec);
											else
											{
												into2->value = 0;
												into2++;
											}
										}
									}

								}
							}
							else
								gOccPool.EndWaitForMemoryManager(curstack);

						}
						into = (void*)into2;
					}
					break;

				case 4:
					{
						TypeSortElem<sLONG> *into2 = (TypeSortElem<sLONG>*)into;
						for (i=0;i<=nbelem;i++) 
						{
							gOccPool.WaitForMemoryManager(curstack);
							FicheOnDisk *obj = (FicheOnDisk*)(TabMem[i]);

							if (obj!=nil)
							{
								StOccupy lock(obj, curstack, false);
								{
									gOccPool.EndWaitForMemoryManager(curstack);
									sLONG numrec = obj->getnumfic();
									if (filtre->isOn(numrec))
									{
										outDeja.Set(numrec);
										into2->recnum = numrec;
										sLONG TypeOnDisk;
										void* data = (void*)(obj->GetDataPtrForQuery(numfield, &TypeOnDisk));
										if (data != nil)
										{
											if (TypeOnDisk == requestedType)
												into2->value = *((sLONG*)data);
											else
											{
												ValPtr cv = NewValPtr(requestedType, data, TypeOnDisk, obj->GetOwner(), nil);
												if (cv != nil)
												{
													cv->WriteToPtr(&(into2->value));
													delete cv;
												}
												else
													into2->value = 0;
											}
											into2++;
										}
										else
										{
											if (nulls != nil)
												nulls->Set(numrec);
											else
											{
												into2->value = 0;
												into2++;
											}
										}
									}

								}
							}
							else
								gOccPool.EndWaitForMemoryManager(curstack);

						}
						into = (void*)into2;
					}
					break;

				case 8:
					{
						TypeSortElem<sLONG8> *into2 = (TypeSortElem<sLONG8>*)into;
						for (i=0;i<=nbelem;i++) 
						{
							gOccPool.WaitForMemoryManager(curstack);
							FicheOnDisk *obj = (FicheOnDisk*)(TabMem[i]);

							if (obj!=nil)
							{
								StOccupy lock(obj, curstack, false);
								{
									gOccPool.EndWaitForMemoryManager(curstack);
									sLONG numrec = obj->getnumfic();
									if (filtre->isOn(numrec))
									{
										outDeja.Set(numrec);
										into2->recnum = numrec;
										sLONG TypeOnDisk;
										void* data = (void*)(obj->GetDataPtrForQuery(numfield, &TypeOnDisk));
										if (data != nil)
										{
											if (TypeOnDisk == requestedType)
												into2->value = *((sLONG8*)data);
											else
											{
												ValPtr cv = NewValPtr(requestedType, data, TypeOnDisk, obj->GetOwner(), nil);
												if (cv != nil)
												{
													cv->WriteToPtr(&(into2->value));
													delete cv;
												}
												else
													into2->value = 0;
											}
											into2++;
										}
										else
										{
											if (nulls != nil)
												nulls->Set(numrec);
											else
											{
												into2->value = 0;
												into2++;
											}
										}
									}

								}
							}
							else
								gOccPool.EndWaitForMemoryManager(curstack);

						}
						into = (void*)into2;
					}
					break;

				default:
					for (i=0;i<=nbelem;i++) 
					{
						gOccPool.WaitForMemoryManager(curstack);
						FicheOnDisk *obj = (FicheOnDisk*)(TabMem[i]);

						if (obj!=nil)
						{
							StOccupy lock(obj, curstack, false);
							{
								gOccPool.EndWaitForMemoryManager(curstack);
								sLONG numrec = obj->getnumfic();
								if (filtre->isOn(numrec))
								{
									outDeja.Set(numrec);
									*((sLONG*)into) = numrec;
									sLONG TypeOnDisk;
									void* data = (void*)(obj->GetDataPtrForQuery(numfield, &TypeOnDisk));
									if (data != nil)
									{
										if (TypeOnDisk == requestedType)
											vBlockMove(data, (void*) (((char*)into) + 4), sizeelem - 4);
										else
										{
											ValPtr cv = NewValPtr(requestedType, data, TypeOnDisk, obj->GetOwner(), nil);
											if (cv != nil)
											{
												cv->WriteToPtr((void*) (((char*)into) + 4));
												delete cv;
											}
											else
												memset((((char*)into) + 4), 0, sizeelem - 4);
										}
										into = (void*) (((char*)into) + sizeelem);
									}
									else
									{
										if (nulls != nil)
											nulls->Set(numrec);
										else
										{
											memset((((char*)into) + 4), 0, sizeelem - 4);
											into = (void*) (((char*)into) + sizeelem);
										}
									}
								}

							}
						}
						else
							gOccPool.EndWaitForMemoryManager(curstack);

					}
					break;
				}
			}
		}
	}

	Free(curstack, true);
}



VError TreeMem::GetLockerInfo(RecIDType curpos, sLONG nbelem, VJSONArray* ioArr, OccupableStack* curstack)
{
	VError err = VE_OK;
	sLONG i,n,xn2;
	TreeMem* sous;
	Occupy(curstack, false);
	gOccPool.EndWaitForMemoryManager(curstack);

	if (nbelem>=kNbElemTreeMem)
	{
		if (nbelem>=kNbElemTreeMem2)
		{
			n = (nbelem+kNbElemTreeMem2-1) >> kRatioTreeMem2;
			for (i=0;i<n && err == VE_OK;i++)
			{
				xn2 = kNbElemTreeMem2-1;

				gOccPool.WaitForMemoryManager(curstack);
				sous = (TreeMem*)(TabMem[i]);
				if (sous!=nil)
				{
					err = sous->GetLockerInfo(curpos + (i * kNbElemTreeMem2), xn2, ioArr, curstack);
				}
				else
					gOccPool.EndWaitForMemoryManager(curstack);
			}
		}
		else
		{
			n = (nbelem+kNbElemTreeMem-1) >> kRatioTreeMem;
			for (i=0;i<n && err == VE_OK;i++)
			{
				xn2 = kNbElemTreeMem-1;

				gOccPool.WaitForMemoryManager(curstack);
				sous = (TreeMem*)(TabMem[i]);
				if (sous!=nil)
				{
					err = sous->GetLockerInfo(curpos + (i * kNbElemTreeMem), xn2, ioArr, curstack);
				}
				else
					gOccPool.EndWaitForMemoryManager(curstack);
			}
		}
	}
	else
	{
		for (i=0;i<=nbelem && err == VE_OK; ++i, ++curpos) 
		{
			gOccPool.WaitForMemoryManager(curstack);
			LockEntity *obj = (LockEntity*)(TabMem[i]);

			if (obj != nil)
			{
				gOccPool.EndWaitForMemoryManager(curstack);
				if (obj->GetOwner() != nil)
				{
					VJSONObject* lockinfo = obj->GetOwner()->BuildLockInfo();
					if (lockinfo != nil)
					{
						lockinfo->SetPropertyAsNumber("recordNumber", curpos);
						ioArr->Push(VJSONValue(lockinfo));
						lockinfo->Release();
					}
				}
			}
			else
				gOccPool.EndWaitForMemoryManager(curstack);

		}
	}

	Free(curstack, true);
	return err;
}


#if debugTreeMem_Strong

sLONG TreeMem::debug_CheckTreeComptePages(sLONG nbelem)
{
	sLONG tot = 1;
	if (nbelem >= kNbElemTreeMem)
	{
		sLONG n, xn2;

		if (nbelem>=kNbElemTreeMem2)
		{
			n = (nbelem+kNbElemTreeMem2-1) >> kRatioTreeMem2;
			xn2 = kNbElemTreeMem2-1;
		}
		else
		{
			n = (nbelem+kNbElemTreeMem-1) >> kRatioTreeMem;
			xn2 = kNbElemTreeMem-1;
		}

		for (sLONG i=0;i<n;i++)
		{
			TreeMem* sous = (TreeMem*)(TabMem[i]);
			if (sous!=nil)
			{
				tot = tot + sous->debug_CheckTreeComptePages(xn2);
			}
		}
	}
	else
	{
	}
	return tot;
}


void TreeMem::debug_CheckTree(sLONG nbelem, debug_treememSet& dansTree)
{
	dansTree.insert(this);
	if (nbelem >= kNbElemTreeMem)
	{
		sLONG n, xn2;

		if (nbelem>=kNbElemTreeMem2)
		{
			n = (nbelem+kNbElemTreeMem2-1) >> kRatioTreeMem2;
			xn2 = kNbElemTreeMem2-1;
		}
		else
		{
			n = (nbelem+kNbElemTreeMem-1) >> kRatioTreeMem;
			xn2 = kNbElemTreeMem-1;
		}

		for (sLONG i=0;i<n;i++)
		{
			TreeMem* sous = (TreeMem*)(TabMem[i]);
			if (sous!=nil)
			{
				sous->debug_CheckTree(xn2, dansTree);
			}
		}
	}
	else
	{
	}
}

#endif





/* ----------------------------------------------- */


TreeMemHeader::TreeMemHeader()
{
	tmem=nil;
	nbelem=0;
}


TreeMemHeader::~TreeMemHeader()
{
	// LibereEspaceMem(); // sera appeler dans le destructeur de chaque classe derivee

}

void TreeMemHeader::LibereEspaceMem(OccupableStack* curstack)
{
	Occupy(curstack, false);
	gOccPool.WaitForMemoryManager(curstack);

#if debugTreeMem_Strong
	debug_CheckTree();
#endif

	if (tmem!=nil)
	{
		sLONG xnbelem = nbelem;
		if (xnbelem < kNbElemTreeMem)
			xnbelem--;
		tmem->LiberePourClose(xnbelem, curstack);
		delete tmem;
		tmem = nil;
	}
	else
		gOccPool.EndWaitForMemoryManager(curstack);

#if debugTreeMem_Strong
	debug_CheckTree();
#endif

	Free(curstack, true);
	nbelem = 0;
}


bool TreeMemHeader::TryToPurge(OccupableStack* curstack)
{
	Occupy(curstack, false);
	gOccPool.WaitForMemoryManager(curstack);

#if debugTreeMem_Strong
	debug_CheckTree();
#endif

	bool purged = true;
	if (tmem!=nil)
	{
		sLONG xnbelem = nbelem;
		if (xnbelem < kNbElemTreeMem)
			xnbelem--;
		if (tmem->TryToPurge(xnbelem, curstack))
		{
			delete tmem;
			tmem = nil;
			nbelem = 0;
		}
		else 
			purged = false;
	}
	else
	{
		nbelem = 0;
		gOccPool.EndWaitForMemoryManager(curstack);
	}

#if debugTreeMem_Strong
	debug_CheckTree();
#endif

	Free(curstack, true);

	return purged;
}


bool TreeMemHeader::TryToFreeMem(sLONG allocationBlockNumber, VSize combien, VSize& outFreed)
{
	bool purged = true;
	outFreed = 0;

#if debugTreeMem_Strong
	debug_CheckTree();
#endif

	if (tmem!=nil)
	{
		sLONG xnbelem = nbelem;
		if (xnbelem < kNbElemTreeMem)
			xnbelem--;
		if (tmem->TryToFreeMem(allocationBlockNumber, xnbelem, combien, outFreed) && !IsOccupied())
		{
			delete tmem;
			tmem = nil;
			//nbelem = 0;
		}
		else 
			purged = false;
	}
	/*
	else
		nbelem = 0;
		*/

#if debugTreeMem_Strong
	debug_CheckTree();
#endif

	return purged;
}



VError TreeMemHeader::PerformRech(Bittab* ReelFiltre, RechNode* rn, Bittab *cursel, Bittab *filtre, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, 
									DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, sLONG& nbfound, Bittab &Nulls, Bittab* dejalocked, OccupableStack* curstack, LocalEntityModel* model)
{
	VError err = VE_OK;
	Occupy(curstack, true);
	gOccPool.WaitForMemoryManager(curstack);
	if (tmem!=nil)
		err = tmem->PerformRech(ReelFiltre, nbelem, rn, cursel, filtre, InProgress, context, HowToLock, exceptions, limit, nbfound, Nulls, dejalocked, curstack, model);
	else
		gOccPool.EndWaitForMemoryManager(curstack);
	Free(curstack, true);
	return err;
}


VError TreeMemHeader::PerformRech(Bittab* ReelFiltre, SimpleQueryNode* rn, Bittab *cursel, Bittab *filtre, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, 
									DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, sLONG& nbfound, Bittab &Nulls, Bittab* dejalocked, OccupableStack* curstack)
{
	VError err = VE_OK;
	Occupy(curstack, true);
	gOccPool.WaitForMemoryManager(curstack);
	if (tmem!=nil)
		err = tmem->PerformRech(ReelFiltre, nbelem, rn, cursel, filtre, InProgress, context, HowToLock, exceptions, limit, nbfound, Nulls, dejalocked, curstack);
	else
		gOccPool.EndWaitForMemoryManager(curstack);
	Free(curstack, true);
	return err;
}


void TreeMemHeader::FillArrayFromCache(void* &into, sLONG sizeelem, Bittab* filtre, Bittab& outDeja, const xMultiFieldDataOffsets& criterias, OccupableStack* curstack, Bittab* nulls)
{
	Occupy(curstack, true);
	gOccPool.WaitForMemoryManager(curstack);
	if (tmem != nil)
		tmem->FillArrayFromCache(nbelem, into, sizeelem, filtre, outDeja, criterias, curstack, nulls);
	else
		gOccPool.EndWaitForMemoryManager(curstack);
	Free(curstack, true);
}


void TreeMemHeader::CalculateFormulaFromCache(ColumnFormulas* Formulas, Bittab* filtre, Bittab& outDeja, BaseTaskInfo* context, OccupableStack* curstack)
{
	Occupy(curstack, true);
	gOccPool.WaitForMemoryManager(curstack);
	if (tmem != nil)
		tmem->CalculateFormulaFromCache(nbelem, Formulas, filtre, outDeja, context, curstack);
	else
		gOccPool.EndWaitForMemoryManager(curstack);
	Free(curstack, true);
}


VError TreeMemHeader::FillCollection(DB4DCollectionManager& collection, Bittab& dejapris, SelPosIterator& PosIter, BaseTaskInfo* context, VArrayRetainedOwnedPtrOf<Field*>& cols, OccupableStack* curstack)
{
	Occupy(curstack, true);
	gOccPool.WaitForMemoryManager(curstack);
	VError err = VE_OK;
	if (tmem != nil)
		err = tmem->FillCollection(0, nbelem, collection, dejapris, PosIter, context, cols, curstack);
	else
		gOccPool.EndWaitForMemoryManager(curstack);
	Free(curstack, true);
	return err;
}


VError TreeMemHeader::GetLockerInfo(VJSONArray* ioArr, OccupableStack* curstack)
{
	Occupy(curstack, true);
	gOccPool.WaitForMemoryManager(curstack);
	VError err = VE_OK;
	if (tmem != nil)
		err = tmem->GetLockerInfo(0, nbelem, ioArr, curstack);
	else
		gOccPool.EndWaitForMemoryManager(curstack);
	Free(curstack, true);
	return err;
}




void TreeMemHeader::Init(sLONG NbElements)
{
	nbelem=NbElements;
}


VError TreeMemHeader::PutIntoTreeMem(sLONG NewMax, sLONG n, void *data, OccupableStack* curstack)
{
	VError err = VE_OK;
	TreeMem *sous;

	Occupy(curstack, true);
	
	bool recheck;

	do
	{
		recheck = false;
		gOccPool.WaitForMemoryManager(curstack);

	#if debugTreeMem_Strong
		debug_CheckTree();
	#endif

		sous=tmem;
		if (sous==nil)
		{
			recheck = true;
			gOccPool.EndWaitForMemoryManager(curstack);

			{
				VTaskLock lock(&fAlterTreeMutex);
				sous=tmem;
				if (sous == nil)
				{
					sous=CreTreeMem();
					if (sous==nil)
					{
						err = ThrowBaseError(memfull, DBaction_StoringObjectInCache);
						recheck = false;
					}
					else
					{
						gOccPool.WaitForMemoryManager(curstack);
						tmem=sous;
						gOccPool.EndWaitForMemoryManager(curstack);
					}
				}
			}

		}
	} while (recheck);

	if (nbelem<NewMax && err == VE_OK)
	{
		if ((NewMax==kNbElemTreeMem) || (NewMax==kNbElemTreeMem2))
		{
			gOccPool.EndWaitForMemoryManager(curstack);
			sous=CreTreeMem();
			if (sous==nil)
			{
				err = ThrowBaseError(memfull, DBaction_StoringObjectInCache);
			}
			else
			{
				gOccPool.WaitForMemoryManager(curstack);
				(*sous)[0]=tmem;
				tmem=sous;
			}
		}
	}


	if (err==VE_OK)
	{
		err=sous->PutInto(NewMax,n,data,curstack);
	}

	if ( (err==VE_OK) && (NewMax>nbelem) ) 
		nbelem=NewMax;

#if debugTreeMem_Strong
	debug_CheckTree();
#endif

	Free(curstack, true);

	return(err);
}



VError TreeMemHeader::ReplaceIntoTreeMem(sLONG NewMax, sLONG n, void *data, OccupableStack* curstack)
{
	VError err = VE_OK;
	TreeMem *sous;

	Occupy(curstack, true);

#if debugTreeMem_Strong
	debug_CheckTree();
#endif

	bool recheck;

	do
	{
		recheck = false;
		gOccPool.WaitForMemoryManager(curstack);

	#if debugTreeMem_Strong
		debug_CheckTree();
	#endif

		sous=tmem;
		if (sous==nil)
		{
			recheck = true;
			gOccPool.EndWaitForMemoryManager(curstack);

			{
				VTaskLock lock(&fAlterTreeMutex);
				sous=tmem;
				if (sous == nil)
				{
					sous=CreTreeMem();
					if (sous==nil)
					{
						err = ThrowBaseError(memfull, DBaction_StoringObjectInCache);
						recheck = false;
					}
					else
					{
						gOccPool.WaitForMemoryManager(curstack);
						tmem=sous;
						gOccPool.EndWaitForMemoryManager(curstack);
					}
				}
			}

		}
	} while (recheck);

	if (nbelem<NewMax && err == VE_OK)
	{
		if ((NewMax==kNbElemTreeMem) || (NewMax==kNbElemTreeMem2))
		{
			gOccPool.EndWaitForMemoryManager(curstack);
			sous=CreTreeMem();
			if (sous==nil)
			{
				err = ThrowBaseError(memfull, DBaction_StoringObjectInCache);
			}
			else
			{
				gOccPool.WaitForMemoryManager(curstack);
				(*sous)[0]=tmem;
				tmem=sous;
			}
		}
	}

	if (err==VE_OK)
	{
		err=sous->ReplaceInto(NewMax,n,data,curstack);
	}

	if ( (err==VE_OK) && (NewMax>nbelem) ) 
		nbelem=NewMax;

#if debugTreeMem_Strong
	debug_CheckTree();
#endif

	Free(curstack, true);

	return(err);
}


void* TreeMemHeader::GetFromTreeMem(sLONG n, OccupableStack* curstack)
{
	void* result;

	gOccPool.WaitForMemoryManager(curstack);
	Occupy(curstack, false);
	result=nil;
	if (tmem!=nil)
	{
		result = tmem->GetFrom(nbelem,n,curstack);
	}
	else
		gOccPool.EndWaitForMemoryManager(curstack);

	Free(curstack, true);
	return(result);
}



void* TreeMemHeader::GetFromTreeMemAndOccupy(sLONG n, OccupableStack* curstack)
{
	void* result;

	gOccPool.WaitForMemoryManager(curstack);
	Occupy(curstack, false);
	result=nil;
	if (tmem!=nil)
	{
		result = tmem->GetFromAndOccupy(nbelem,n,curstack);
	}
	else
		gOccPool.EndWaitForMemoryManager(curstack);

	Free(curstack, true);
	return(result);
}


void* TreeMemHeader::RetainFromTreeMem(sLONG n, OccupableStack* curstack)
{
	void* result;

	gOccPool.WaitForMemoryManager(curstack);
	Occupy(curstack, false);
	result=nil;
	if (tmem!=nil)
	{
		result = tmem->RetainFrom(nbelem,n, curstack);
	}
	else
		gOccPool.EndWaitForMemoryManager(curstack);

	Free(curstack, true);
	return(result);
}


VError TreeMemHeader::Aggrandit(sLONG newMax, OccupableStack* curstack)
{
	VError err = VE_OK;
	Occupy(curstack, true);

	sLONG i = nbelem;
	while (newMax > nbelem && err == VE_OK)
	{
		err = PutIntoTreeMem(i+1, i, nil, curstack);
		i++;
	}

	Free(curstack, true);
	return err;
}


VError TreeMemHeader::AggranditSiNecessaire(sLONG newMax, OccupableStack* curstack)
{
	VError err = VE_OK;
	Occupy(curstack, true);

	if (tmem != nil)
	{
		sLONG i = nbelem;
		while (newMax > nbelem && err == VE_OK)
		{
			err = PutIntoTreeMem(i+1, i, nil, curstack);
			i++;
		}
	}
	else
		nbelem = newMax;

	Free(curstack, true);
	return err;
}



void* TreeMemHeader::DelFromTreeMem(sLONG n, OccupableStack* curstack)
{
	void* result = nil;
	gOccPool.WaitForMemoryManager(curstack);
	Occupy(curstack, false);
	if (tmem!=nil)
	{
		result = tmem->DelFrom(nbelem,n,curstack);
	}
	else
	{
		gOccPool.EndWaitForMemoryManager(curstack);
		result = nil;
	}
	Free(curstack, true);
	return result;
}


void TreeMemHeader::PurgeFromTreeMem(sLONG n, OccupableStack* curstack)
{
	gOccPool.WaitForMemoryManager(curstack);
	Occupy(curstack, false);
	if (tmem!=nil)
	{
		tmem->PurgeFrom(nbelem,n,curstack);
	}
	else
		gOccPool.EndWaitForMemoryManager(curstack);

	Free(curstack, true);
}


TreeMem* TreeMemHeader::CreTreeMem()
{
	return new TreeMem(this);
}


#if debugTreeMem_Strong

void TreeMemHeader::debug_AddPage(TreeMem* obj)
{
	debug_pages.insert(obj);
}


void TreeMemHeader::debug_DelPage(TreeMem* obj)
{
	debug_pages.erase(obj);
}

void TreeMemHeader::debug_CheckTree()
{
	sLONG xnbelem = nbelem;
	if (xnbelem < kNbElemTreeMem)
		xnbelem--;
	sLONG nbpage = 0;
	if (tmem != nil)
		nbpage = tmem->debug_CheckTreeComptePages(xnbelem);
	if (nbpage != debug_pages.size())
	{
		xnbelem = xnbelem; // put a break here
		xbox_assert(false);

	}

	/*
	debug_treememSet dansTree;
	if (tmem != nil)
		tmem->debug_CheckTree(xnbelem, dansTree);
	for (debug_treememSet::iterator cur = debug_pages.begin(), end = debug_pages.end(); cur != end; cur++)
	{
		if (dansTree.find(*cur) == dansTree.end())
		{
			xnbelem = xnbelem; // put a break here
			xbox_assert(false);
		}
	}
	*/
}

#endif




				// ==========================================================

#if debugTabAddr

VCriticalSection AddressTable::debugKeepTablesMutex;
map<DataAddr4D, AddressTable*> AddressTable::debugKeepTables;


void AddressTable::DebugKeepPage(AddressTable* addrtable)
{
	VTaskLock lock(&debugKeepTablesMutex);
	DataAddr4D addr = addrtable->getaddr();
	map<DataAddr4D, AddressTable*>::iterator found = debugKeepTables.find(addr);
	if (found != debugKeepTables.end())
	{
		xbox_assert(found == debugKeepTables.end());
		addr = addr; //put a break here
	}
	debugKeepTables[addr] = addrtable;
}


void AddressTable::DebugForgetPage(AddressTable* addrtable)
{
	VTaskLock lock(&debugKeepTablesMutex);
	DataAddr4D addr = addrtable->getaddr();
	map<DataAddr4D, AddressTable*>::iterator found = debugKeepTables.find(addr);

	if (found == debugKeepTables.end())
	{
		xbox_assert(found != debugKeepTables.end());
		addr = addr; //put a break here
	}
	else
	{
		if (found->second != addrtable)
		{
			xbox_assert(found->second == addrtable);
			addr = addr; //put a break here
		}
	}

	debugKeepTables.erase(addr);
}


#endif




AddressTable::AddressTable(Base4D *xdb, sLONG xprefseg, bool inKeepStamps, bool DebugCheckOnDelete)
{
	db=xdb;
	std::fill(&fTabmem[0], &fTabmem[kNbElemTabAddr], (AddressTable*)nil);
	prefseg = xprefseg;
	fIsLoaded = false;
	posdansparent = -5;
	fKeepStamps = inKeepStamps;
	if (fKeepStamps)
	{
		fStamps = (StampsArray*)GetFastMem(sizeof(StampsArray), false, 'stam');
	}
	else
		fStamps = nil;
	fDebugCheckOnDelete = DebugCheckOnDelete;
}


AddressTable::~AddressTable()
{
#if debugTabAddr
	if (fDebugCheckOnDelete)
	{
		DebugForgetPage(this);

		for (sLONG i = 0; i < kNbElemTabAddr; i++)
		{
			if (fTabmem[i] != nil)
			{
				xbox_assert(fTabmem[i] == nil);
				i = i; // put a break here
			}
		}

		if (false)
		{
			AddrDBObj disktab[kNbElemTabAddr];
			DataAddr4D ou = getaddr();
			if (ou > 0 && !db->IsClosing())
			{
				db->readlong(disktab,kSizeTabAddr,ou,kSizeDataBaseObjectHeader);
				for (sLONG i = 0; i < kNbElemTabAddr; i++)
				{
					if (testAssert(disktab[i].addr == tab[i].addr && disktab[i].len == tab[i].len))
					{
						// c'est bon
					}
					else
					{
						// put a break here;
						i = i;
					}
				}
			}
		}

	}
#endif

	if (fStamps != nil)
		FreeFastMem(fStamps);
#if debuglr
	if (modifie())
	{
		xbox_assert(!modifie());
	}
#endif
}


VError AddressTable::init(void)
{
	AddrDBObj filler;
	filler.addr = 0;
	filler.len = 0;
	std::fill(&tab[0], &tab[kNbElemTabAddr], filler);
	if (fStamps != nil)
		std::fill(&(*fStamps)[0], &(*fStamps)[kNbElemTabAddr], 0);
	fIsLoaded = true;
	return VE_OK;
}


VError AddressTable::loadobj(DataAddr4D xaddr, sLONG inposdansparent)
{
	VError err = VE_OK;

	if (!fIsLoaded)
	{
		VTaskLock lock(&fLoadMutex);
		if (!fIsLoaded)
		{
			if (inposdansparent == -1)
				inposdansparent = posdansparent;
			if (xaddr!=0) 
				setaddr(xaddr);
			DataBaseObjectHeader tag;
			err = tag.ReadFrom(db, getaddr(), nil);
			if (err == VE_OK)
				err = tag.ValidateTag(fKeepStamps ? DBOH_TableAddressWithStamps : DBOH_TableAddress, inposdansparent, -1);
			if (err == VE_OK)
				err=db->readlong(tab,kSizeTabAddr,getaddr(),kSizeDataBaseObjectHeader);
			if (fKeepStamps && err == VE_OK)
			{
				if (fStamps == nil)
					err = ThrowBaseError(memfull);
				else
				{
					err=db->readlong(fStamps, sizeof(StampsArray), getaddr(), kSizeDataBaseObjectHeader+kSizeTabAddr);
				}
			}
			if (err == VE_OK)
				err = tag.ValidateCheckSum(tab, kSizeTabAddr);
	#if debuglr
			if (err != VE_OK)
				sLONG xdebug = 1; // put a break here
	#endif
			if (err == VE_OK)
			{
				fIsLoaded = true;
				if (tag.NeedSwap())
				{
					ByteSwapCollection(tab, kNbElemTabAddr);
				}
			}
		}
	}
/*
	if (tabmem==nil && err == VE_OK)
	{
		AddressTable* *xtabmem =(AddressTable* *)GetFastMem(kSizeTabAddrMem, false, 'tMem');
		if (xtabmem==nil)
		{
			err = ThrowBaseError(memfull, DBaction_LoadingAddrTable);
		}
		else
		{
			_raz(xtabmem,kSizeTabAddrMem);
			tabmem = xtabmem;
		}
	}
*/
	if (err != VE_OK)
		err = ThrowBaseError(VE_DB4D_CANNOT_LOAD_ADDRTABLE, DBaction_LoadingAddrTable);

	return(err);
}


bool AddressTable::SaveObj(VSize& outSizeSaved)
{
#if debuglogwrite
	VString wherefrom(L"AddrTable SaveObj");
	VString* whx = &wherefrom;
#else
	VString* whx = nil;
#endif

	VError err;

	DataBaseObjectHeader tag(tab, kSizeTabAddr, fKeepStamps ? DBOH_TableAddressWithStamps : DBOH_TableAddress, posdansparent, -1);
	err = tag.WriteInto(db, getaddr(), whx);
	if (err == VE_OK)
		err=db->writelong(tab,kSizeTabAddr,getaddr(),kSizeDataBaseObjectHeader, whx);
	if (err == VE_OK && fStamps != nil)
		err=db->writelong(fStamps, sizeof(StampsArray), getaddr(), kSizeDataBaseObjectHeader+kSizeTabAddr, whx);

	outSizeSaved = kSizeTabAddr;
	if (fStamps != nil)
		outSizeSaved = outSizeSaved + sizeof(StampsArray);

	return(true);
}


bool AddressTable::TryToFreeMem(sLONG allocationBlockNumber, sLONG nbelem, VSize combien, VSize& outFreed)
{
	bool allfreed = true;
	VSize tot = 0;

	sLONG i,n,xn2;
	AddressTable* sous;

	bool isoccupied = IsOccupied();
	//if (tabmem != nil)
	{
		if (nbelem>kNbElemTabAddr)
		{
			if (nbelem>kNbElemTabAddr2)
			{
				n = (nbelem+kNbElemTabAddr2-1) >> kRatioTabAddr2;
				for (i=0;i<n;i++)
				{
					if (i==(n-1))
					{
						xn2 = nbelem & (kNbElemTabAddr2-1);
						if (xn2 <= kNbElemTabAddr)
							xn2 = kNbElemTabAddr+1;
					}
					else
					{
						xn2 = kNbElemTabAddr2-1;
					}

					sous = fTabmem[i];
					if (sous!=nil)
					{
						VSize tot2;
						if (sous->TryToFreeMem(allocationBlockNumber, xn2, combien-tot, tot2) && !isoccupied)
						{
							tot = tot + sizeof(AddressTable);
							if (sous->fStamps != nil)
								tot = tot + sizeof(StampsArray);
							sous->Release();
							fTabmem[i] = nil;
							/*
							if (tot >= combien)
							{
								if (i != n-1)
									allfreed = false;
								break;
							}
							*/
						}
						else
						{
							allfreed = false;
							/*
							if (tot >= combien)
								break;
								*/
						}
						tot = tot + tot2;
					}
				}
			}
			else
			{
				n = (nbelem+kNbElemTabAddr-1) >> kRatioTabAddr;
				for (i=0;i<n;i++)
				{
					if (i==(n-1))
					{
						xn2 = nbelem & (kNbElemTabAddr-1);
					}
					else
					{
						xn2 = kNbElemTabAddr-1;
					}

					sous = fTabmem[i];
					if (sous!=nil)
					{
						VSize tot2;
						if (sous->TryToFreeMem(allocationBlockNumber, xn2, combien-tot, tot2) && !isoccupied)
						{
							tot = tot + sizeof(AddressTable);
							if (sous->fStamps != nil)
								tot = tot + sizeof(StampsArray);
							sous->Release();
							fTabmem[i] = nil;
							/*
							if (tot >= combien)
							{
								if (i != n-1)
									allfreed = false;
								break;
							}
							*/
						}
						else
						{
							allfreed = false;
							/*
							if (tot >= combien)
								break;
								*/
						}
						tot = tot + tot2;
					}
				}
			}
		}
		else
		{
		}
	}

	outFreed = tot;

	if (IsOccupied() || modifie() || !OKAllocationNumber(this, allocationBlockNumber))
		allfreed = false;

	return allfreed;
}


VError AddressTable::loadmem(sLONG n, sLONG n1, sLONG *xn2, sLONG *n3, AddressTable* *psous, OccupableStack* curstack)
{
	VError err;
	AddressTable* sousADT = nil;

	/*
	Occupy(curstack, true);
	gOccPool.EndWaitForMemoryManager(curstack);
	*/

	err=loadobj();
#if 0
	// deja fait dans loadobj
	if (tabmem==nil)
	{
		tabmem=(AddressTable* *)GetFastMem(kSizeTabAddrMem, false, 'tMem');
		if (tabmem==nil)
		{
			err = ThrowBaseError(memfull, DBaction_LoadingAddrTable);
		}
		else
		{
			_raz(tabmem,kSizeTabAddrMem);
		}
	}
#endif

	if (psous!=nil)
	{
		if (err==VE_OK)
		{
			if (*n3>kNbElemTabAddr2)
			{
				n1=n1>>kRatioTabAddr;
				*xn2=n & (kNbElemTabAddr2-1);
				*n3=kNbElemTabAddr2-1;
			}
			else
			{
				*n3=kNbElemTabAddr-1;
			}

			gOccPool.WaitForMemoryManager(curstack);
			sousADT=fTabmem[n1];
			if (sousADT==nil)
			{
				gOccPool.EndWaitForMemoryManager(curstack);

				fLoadMutex.Lock();
				gOccPool.WaitForMemoryManager(curstack);
				sousADT=fTabmem[n1];
				if (sousADT == nil)
				{
					gOccPool.EndWaitForMemoryManager(curstack);
					sousADT=new AddressTable(db, prefseg, fKeepStamps, fDebugCheckOnDelete);
					if (sousADT==nil)
					{
						err = ThrowBaseError(memfull, DBaction_LoadingAddrTable);
					}
					else
					{
						err=sousADT->loadobj(tab[n1].addr, n1);
						if (err==VE_OK)
						{
							gOccPool.WaitForMemoryManager(curstack);
							sousADT->posdansparent = n1;
							fTabmem[n1]=sousADT;
						}
						else
						{
							sousADT->Release();
							sousADT=nil;
						}
					}
				}
				fLoadMutex.Unlock();
			}
		}

		*psous=sousADT;
	}

	//Free(curstack, true);

	return(err);
}


void AddressTable::LiberePourClose(sLONG nbelem, OccupableStack* curstack)
{
	sLONG i,n,xn2;
	AddressTable* sous;

	Occupy(curstack, false);
	gOccPool.EndWaitForMemoryManager(curstack);

	if (nbelem>kNbElemTabAddr)
	{
		if (nbelem>kNbElemTabAddr2)
		{
			n = (nbelem+kNbElemTabAddr2-1) >> kRatioTabAddr2;
			for (i=0;i<n;i++)
			{
				if (i==(n-1))
				{
					xn2 = nbelem & (kNbElemTabAddr2-1);
				}
				else
				{
					xn2 = kNbElemTabAddr2-1;
				}

				gOccPool.WaitForMemoryManager(curstack);
				sous = fTabmem[i];
				if (sous!=nil)
				{
					sous->LiberePourClose(xn2, curstack);
					sous->Release();
					fTabmem[i] = nil;
				}
				else
					gOccPool.EndWaitForMemoryManager(curstack);
			}
		}
		else
		{
			n = (nbelem+kNbElemTabAddr-1) >> kRatioTabAddr;
			for (i=0;i<n;i++)
			{
				if (i==(n-1))
				{
					xn2 = nbelem & (kNbElemTabAddr-1);
				}
				else
				{
					xn2 = kNbElemTabAddr-1;
				}

				gOccPool.WaitForMemoryManager(curstack);
				sous = fTabmem[i];
				if (sous!=nil)
				{
					sous->LiberePourClose(xn2, curstack);
					sous->Release();
					fTabmem[i] = nil;
				}
				else
					gOccPool.EndWaitForMemoryManager(curstack);
			}
		}
	}
	else
	{
	}

	Free(curstack, true);
}


DataAddr4D AddressTable::GetAddrFromTable(sLONG nbelem, sLONG n, VError& err, OccupableStack* curstack, sLONG* outLen, uLONG* outStamp)
{
	DataAddr4D result;
	sLONG n1,xn2;
	AddressTable* sousADT;

	result=0;
	Occupy(curstack, false);
	gOccPool.EndWaitForMemoryManager(curstack);

	err=loadobj();

	if (err==VE_OK)
	{
		n1=n>>kRatioTabAddr;
		xn2=n & (kNbElemTabAddr-1);

		if (nbelem>kNbElemTabAddr)
		{
			err=loadmem(n,n1,&xn2,&nbelem,&sousADT, curstack);
			if (err==VE_OK)
			{
				result=sousADT->GetAddrFromTable(nbelem, xn2, err, curstack, outLen, outStamp);
			}
		}
		else
		{
			if (err==VE_OK) 
			{
//				gOccPool.WaitForMemoryManager(curstack);
				result=tab[xn2].addr;
				if (outLen != nil)
					*outLen = tab[xn2].len;
				if (outStamp != nil)
				{
					if (fStamps != nil)
						*outStamp = (*fStamps)[xn2];
					else
						*outStamp = 0;
				}
//				gOccPool.EndWaitForMemoryManager(curstack);
			}
		}
	}

	Free(curstack, true);
	return(result);
}


VError AddressTable::PutAddrIntoTable(sLONG nbelem, sLONG n, DataAddr4D addr, sLONG len, BaseTaskInfo* context, OccupableStack* curstack, uLONG inStamp)
{
	sLONG n1,xn2;
	AddressTable* sousADT;
	VError err;

	Occupy(curstack, false);
	gOccPool.EndWaitForMemoryManager(curstack);
	err = loadobj();

	if (err==VE_OK)
	{
		n1=n>>kRatioTabAddr;
		xn2=n & (kNbElemTabAddr-1);

		if (nbelem>kNbElemTabAddr)
		{

			err=loadmem(n,n1,&xn2,&nbelem,&sousADT, curstack);
			if (err==VE_OK)
			{
				err=sousADT->PutAddrIntoTable(nbelem,xn2,addr,len, context, curstack, inStamp);
			}

		}
		else
		{
			if (err==VE_OK) 
			{
				//gOccPool.WaitForMemoryManager(curstack);
				tab[xn2].addr=addr;
				tab[xn2].len=len;
				if (fStamps != nil && inStamp != 0)
					(*fStamps)[xn2] = inStamp;
				//gOccPool.EndWaitForMemoryManager(curstack);
			}
			setmodif(true, db, context);
		}
	}

	Free(curstack, true);

	if (err != VE_OK)
		err = ThrowBaseError(VE_DB4D_CANNOT_MODIFY_ADDRTABLE, DBaction_LoadingAddrTable);
	return(err);
}


VError AddressTable::InitAndSetSize(sLONG maxelem, BaseTaskInfo* context, sLONG inPosDansParent, OccupableStack* curstack)
{
	VError err = VE_OK;
	sLONG i,nb,last,sousmax, maxdone = -1;
	DataAddr4D ou = db->findplace(kSizeTabAddr+kSizeDataBaseObjectHeader+(fKeepStamps ? sizeof(StampsArray) : 0), context, err, prefseg, this);
	if (err == VE_OK)
	{
		xbox_assert(ou > 0);
		setaddr(ou,false);

		_raz(tab,kSizeTabAddr);
		if (maxelem > kNbElemTabAddr)
		{
			AddressTable* soust = new AddressTable(db, prefseg, fKeepStamps, fDebugCheckOnDelete);
			if (soust == nil)
				err = ThrowBaseError(memfull, DBaction_BuildingAddrTable);
			else
			{
				//soust->Occupy(curstack, true);
				soust->init();

				if (maxelem > kNbElemTabAddr2)
				{
					nb = (maxelem+kNbElemTabAddr2-1) / kNbElemTabAddr2;
					last = (maxelem & (kNbElemTabAddr2-1))+1;
					if (last < 1025)
						last = 1025;
					sousmax = kNbElemTabAddr2;
				}
				else
				{
					nb = (maxelem+kNbElemTabAddr-1) / kNbElemTabAddr;
					last = (maxelem & (kNbElemTabAddr-1))+1;
					sousmax = kNbElemTabAddr;
				}

				for (i = 0; i<nb && err == VE_OK; i++)
				{
					if (soust != nil)
					{
						err = soust->InitAndSetSize(i == nb -1 ? last : sousmax, context, i, curstack);
						if (err == VE_OK)
						{
							tab[i].addr = soust->getaddr();
							tab[i].len = kSizeTabAddr /*+kSizeDataBaseObjectHeader*/ +(fKeepStamps ? sizeof(StampsArray) : 0);
							maxdone = i;
						}
					}
					else
						err = ThrowBaseError(memfull, DBaction_BuildingAddrTable);
				}
				//soust->Free(curstack, true);
				soust->Release();
			}
		}

		if (err == VE_OK)
		{
			DataBaseObjectHeader tag(tab, kSizeTabAddr, fKeepStamps ? DBOH_TableAddressWithStamps : DBOH_TableAddress, inPosDansParent, -1);
			err = tag.WriteInto(db, getaddr(), nil);
			if (err == VE_OK)
				err=db->writelong(tab,kSizeTabAddr,getaddr(),kSizeDataBaseObjectHeader, nil);
			if (err == VE_OK && fStamps != nil)
			{
				err=db->writelong(fStamps, sizeof(StampsArray), getaddr(),kSizeDataBaseObjectHeader + kSizeTabAddr, nil);

			}
		}

		if (err != VE_OK)
		{
			if (maxdone != -1)
			{
				if (maxelem > kNbElemTabAddr2)
				{
					AddressTable* soust = new AddressTable(db, prefseg, fKeepStamps, fDebugCheckOnDelete);
					if (soust != nil)
					{
						soust->init();
						for (i=0; i<=maxdone; i++)
						{
							soust->SubLiberePlace(tab[i].addr, context);
						}
						soust->Release();
					}
				}
				for (i=0; i<=maxdone; i++)
				{
					db->libereplace(tab[i].addr, kSizeTabAddr+kSizeDataBaseObjectHeader + (fKeepStamps ? sizeof(StampsArray) : 0), context, this);
				}
			}
			db->libereplace(ou, kSizeTabAddr+kSizeDataBaseObjectHeader + (fKeepStamps ? sizeof(StampsArray) : 0), context, this);
		}
	}

	if (err != VE_OK)
		err = ThrowBaseError(VE_DB4D_CANNOT_MODIFY_ADDRTABLE, DBaction_BuildingAddrTable);
	return err;
}


void AddressTable::SubLiberePlace(DataAddr4D ou, BaseTaskInfo* context)
{
	VError err = db->readlong(tab, kSizeTabAddr, ou, kSizeDataBaseObjectHeader);
	if (err == VE_OK)
	{
		sLONG i;
		for (i=0; i<kNbElemTabAddr; i++)
		{
			if (tab[i].addr > 0)
				db->libereplace(tab[i].addr, kSizeDataBaseObjectHeader + kSizeTabAddr + (fKeepStamps ? sizeof(StampsArray) : 0), context, this);
		}
	}
}


AddressTable* AddressTable::AddToTable(sLONG nbelem, BaseTaskInfo* context, VError& err, OccupableStack* curstack)
{
	AddressTable *result, *deux, *sous;
	sLONG n,xn2;
	DataAddr4D ou;
	uBOOL pasdeux;

	Occupy(curstack, false);
	gOccPool.EndWaitForMemoryManager(curstack);

	result=this;
	err=VE_OK;
	deux=nil;
	if (nbelem<0)
	{
		pasdeux=true;
		nbelem=-nbelem;
	}
	else
	{
		pasdeux=false;
	}

	if ( (nbelem==kNbElemTabAddr) || (nbelem==kNbElemTabAddr2))
	{
		result=new AddressTable(db, prefseg, fKeepStamps, fDebugCheckOnDelete);
		result->Occupy(curstack, true);

		ou=db->findplace(kSizeTabAddr+kSizeDataBaseObjectHeader + (fKeepStamps ? sizeof(StampsArray) : 0), context, err, prefseg, result);
#if debugplace
		shex.FromULong((uLONG)ou,kHexaDecimal);
		UnivStrDebug("FindPlace AddrTab = ",shex);
#endif
		if (ou>0 && err == VE_OK)
		{
			err=result->init();
			if (err==VE_OK)
			{
#if debug_Addr_Overlap
				db->SetDBObjRef(ou, kSizeTabAddr+kSizeDataBaseObjectHeader + (fKeepStamps ? sizeof(StampsArray) : 0), new debug_AddrTableRef(-1, fKeepStamps), true);
#endif

				result->setaddr(ou);
				result->setmodif(true, db, context);
				if (pasdeux)
				{
					if (nbelem==kNbElemTabAddr2)
					{
						err=result->loadmem(0,0,nil,nil,nil, curstack);
						if (err==VE_OK)
						{
							gOccPool.WaitForMemoryManager(curstack);
							sous=result->AddToTable(-kNbElemTabAddr, context, err, curstack);
							if (sous!=nil)
							{
								gOccPool.WaitForMemoryManager(curstack);
								sous->posdansparent = 0;
								result->fTabmem[0]=sous;
								result->tab[0].addr=sous->getaddr();
								result->tab[0].len=kSizeTabAddr+(fKeepStamps ? sizeof(StampsArray) : 0);
								gOccPool.EndWaitForMemoryManager(curstack);
								result->setmodif(true, db, context);
								sous->Free(curstack, true);
							}
						}
					}
				}
				else
				{
					err=result->loadmem(0,0,nil,nil,nil, curstack);
					if (err==VE_OK)
					{
						deux=new AddressTable(db, prefseg, fKeepStamps, fDebugCheckOnDelete);
						deux->Occupy(curstack, true);

						gOccPool.WaitForMemoryManager(curstack);
						result->tab[0].addr=getaddr();
						result->tab[0].len=kSizeTabAddr+(fKeepStamps ? sizeof(StampsArray) : 0);
						posdansparent = 0;
						result->fTabmem[0]=this;
						result->posdansparent = 0;
						gOccPool.EndWaitForMemoryManager(curstack);

						ou=db->findplace(kSizeTabAddr+kSizeDataBaseObjectHeader+ (fKeepStamps ? sizeof(StampsArray) : 0), context, err, prefseg, deux);
#if debugplace
						shex.FromULong((uLONG)ou,kHexaDecimal);
						UnivStrDebug("FindPlace AddrTab = ",shex);
#endif
						if (ou>0 && err == VE_OK)
						{
							err=deux->init();
							if (err==VE_OK)
							{
#if debug_Addr_Overlap
								db->SetDBObjRef(ou, kSizeTabAddr+kSizeDataBaseObjectHeader + (fKeepStamps ? sizeof(StampsArray) : 0), new debug_AddrTableRef(-1, fKeepStamps), true);
#endif
								deux->setaddr(ou);
								/*## ?? err=deux->loadmem(0,0,nil,nil,nil); */
								deux->setmodif(true, db, context);
								if (err==VE_OK)
								{
									gOccPool.WaitForMemoryManager(curstack);
									result->tab[1].addr=ou;
									result->tab[1].len=kSizeTabAddr+(fKeepStamps ? sizeof(StampsArray) : 0);
									deux->posdansparent = 1;
									result->fTabmem[1]=deux;
									gOccPool.EndWaitForMemoryManager(curstack);
									result->setmodif(true, db, context);
									if (nbelem==kNbElemTabAddr2)
									{
										gOccPool.WaitForMemoryManager(curstack);
										sous=deux->AddToTable(-kNbElemTabAddr, context, err, curstack);
										{
											err=deux->loadmem(0,0,nil,nil,nil, curstack);
											if (err == VE_OK)
											{
												gOccPool.WaitForMemoryManager(curstack);
												sous->posdansparent = 0;
												deux->tab[0].addr=sous->getaddr();
												deux->tab[0].len=kSizeTabAddr+(fKeepStamps ? sizeof(StampsArray) : 0);
												deux->fTabmem[0]=sous;
												gOccPool.EndWaitForMemoryManager(curstack);
												deux->setmodif(true, db, context);
											}
											sous->Free(curstack, true);
										}
									}

								}
							}

						}
						deux->Free(curstack, true);
					}
				}
			}
		}

		/*
		if (result!=nil)
			result->Free(curstack, true);
			// c'est maintenant fait par l'appelant
		*/

		if (err!=VE_OK)
		{
			if (result!=nil) 
			{
				result->Free(curstack, true);
				result->Release();
			}
			result=nil;
			if (deux!=nil) 
				deux->Release();
			deux=nil;
		}
	}
	else
	{
		if (nbelem>kNbElemTabAddr2)
		{
			n=nbelem>>(kRatioTabAddr<<1);
			xn2=nbelem & (kNbElemTabAddr2-1);

			if ((xn2==0) && (n>1))
			{
				gOccPool.WaitForMemoryManager(curstack);
				sous=AddToTable(-kNbElemTabAddr2, context, err, curstack);
				if (sous!=nil)
				{
					err = loadobj();
					if (err == VE_OK)
					{
						gOccPool.WaitForMemoryManager(curstack);
						sous->posdansparent = n;
						tab[n].addr=sous->getaddr();
						tab[n].len=kSizeTabAddr+(fKeepStamps ? sizeof(StampsArray) : 0);
						fTabmem[n]=sous;
						gOccPool.EndWaitForMemoryManager(curstack);
						setmodif(true, db, context);
					}
					else result = nil;
					sous->Free(curstack, true);
				}
				else result = nil;
			}
			else
			{
				sLONG bidon,bidon2;
				bidon = nbelem;
				bidon2 = xn2;

				err=loadmem(nbelem,nbelem>>kRatioTabAddr,&bidon2,&bidon,&deux, curstack);

				deux->Occupy(curstack, false);
				gOccPool.EndWaitForMemoryManager(curstack);

				if ((xn2 & (kNbElemTabAddr-1)) == 0)
				{
					sLONG xn1 = xn2 >> kRatioTabAddr;
					gOccPool.WaitForMemoryManager(curstack);
					sous=deux->AddToTable(-kNbElemTabAddr, context, err, curstack);
					if (sous!=nil)
					{
						err = loadobj();
						if (err == VE_OK)
						{
							err = deux->loadmem(0,0,nil,nil,nil, curstack);
							gOccPool.WaitForMemoryManager(curstack);
							sous->posdansparent = xn1;
							deux->tab[xn1].addr=sous->getaddr();
							deux->tab[xn1].len=kSizeTabAddr+(fKeepStamps ? sizeof(StampsArray) : 0);
							deux->fTabmem[xn1]=sous;
							gOccPool.EndWaitForMemoryManager(curstack);
							deux->setmodif(true, db, context);
						}
						else result = nil;
						sous->Free(curstack, true);
					}
					else result = nil;
				}	

				deux->Free(curstack, true);
			}

		}
		else
		{
			n=nbelem>>kRatioTabAddr;
			xn2=nbelem & (kNbElemTabAddr-1);
			if ((xn2==0) && (n>0))
			{
				gOccPool.WaitForMemoryManager(curstack);
				sous=AddToTable(-kNbElemTabAddr, context, err, curstack);
				if (sous!=nil)
				{
					err = loadobj();
					if (err == VE_OK)
					{
						err = loadmem(0,0,nil,nil,nil, curstack);
						gOccPool.WaitForMemoryManager(curstack);
						sous->posdansparent = n;
						tab[n].addr=sous->getaddr();
						tab[n].len=kSizeTabAddr+(fKeepStamps ? sizeof(StampsArray) : 0);
						fTabmem[n]=sous;
						gOccPool.EndWaitForMemoryManager(curstack);
						setmodif(true, db, context);
					}
					else result = nil;
					sous->Free(curstack, true);
				}
				else
				{
					result=nil;
				}
			}
		}
	}

	if (result == this)
		result->Occupy(curstack, true);

	Free(curstack, true);

	if (err != VE_OK)
		err = ThrowBaseError(VE_DB4D_CANNOT_MODIFY_ADDRTABLE, DBaction_BuildingAddrTable);
	return(result);
}



#if debug_Addr_Overlap
void AddressTable::FillDebug_DBObjRef(sLONG nbelem, OccupableStack* curstack, sLONG tablenum, bool isrectable)
{
	Occupy(curstack, false);
	gOccPool.EndWaitForMemoryManager(curstack);

	if (nbelem>kNbElemTabAddr)
	{
		loadobj();
		sLONG oldnbelem=nbelem;
		nbelem=(nbelem+kNbElemTabAddr-1)>>kRatioTabAddr;
		if (nbelem>kNbElemTabAddr)
		{
			sLONG xn2=oldnbelem & (kNbElemTabAddr2-1);
			nbelem=(nbelem+kNbElemTabAddr-1)>>kRatioTabAddr;
			for (sLONG i=0;i<nbelem;i++)
			{
				sLONG n;
				if (i==(nbelem-1)) 
					n=xn2; 
				else 
					n=kNbElemTabAddr2-1;
				sLONG bidon1 = 0, bidon2 = 0, bidon3 = 0;
				AddressTable* soustable = nil;
				loadmem(bidon1, i, &bidon2, &bidon3, &soustable, curstack);
				if (soustable != nil)
					soustable->FillDebug_DBObjRef(n, curstack, tablenum, isrectable);
			}
		}
		else
		{
			for (sLONG i=0;i<nbelem;i++)
			{
				db->SetDBObjRef(tab[i].addr, tab[i].len, new debug_AddrTableRef(tablenum, isrectable));
			}
		}
	}
	else
	{
		// rien au dernier niveau
	}
	Free(curstack, true);
}

#endif


VError AddressTable::SubLibereEspaceDisk(sLONG nbelem, VDB4DProgressIndicator* InProgress, OccupableStack* curstack)
{
	sLONG i,n,xn2,oldnbelem;
	AddressTable *soustable;
	VError err = VE_OK;

	Occupy(curstack, false);
	gOccPool.EndWaitForMemoryManager(curstack);

	if (nbelem>kNbElemTabAddr)
	{
		loadobj();
		oldnbelem=nbelem;
		nbelem=(nbelem+kNbElemTabAddr-1)>>kRatioTabAddr;
		if (nbelem>kNbElemTabAddr)
		{
			xn2=oldnbelem & (kNbElemTabAddr2-1);
			nbelem=(nbelem+kNbElemTabAddr-1)>>kRatioTabAddr;
			for (i=0;i<nbelem;i++)
			{
				gOccPool.WaitForMemoryManager(curstack);
				{
					//gOccPool.WaitForMemoryManager(curstack);
					soustable=fTabmem[i];
					fTabmem[i] = nil;
					//gOccPool.EndWaitForMemoryManager(curstack);
				}
				if (soustable==nil)
				{
					gOccPool.EndWaitForMemoryManager(curstack);
					soustable=new AddressTable(db, prefseg, fKeepStamps, fDebugCheckOnDelete);
					soustable->loadobj(tab[i].addr, i);
					gOccPool.WaitForMemoryManager(curstack);

				}
				if (soustable!=nil)
				{
					if (i==(nbelem-1)) 
						n=xn2; 
					else 
						n=kNbElemTabAddr2-1;
					soustable->SubLibereEspaceDisk(n, InProgress, curstack);
					soustable->Release();
				}
			}
		}
		else
		{
			//if (tabmem!=nil)
			{
				for (i=0;i<nbelem;i++)
				{
					//gOccPool.WaitForMemoryManager(curstack);
					soustable=fTabmem[i];
					//gOccPool.EndWaitForMemoryManager(curstack);
					if (soustable != nil)
						soustable->setmodif(false, db, nil);
#if debug_Addr_Overlap
					db->DelDBObjRef(tab[i].addr, kSizeTabAddr+kSizeDataBaseObjectHeader+ (fKeepStamps ? sizeof(StampsArray) : 0));
#endif
					db->libereplace(tab[i].addr, kSizeTabAddr+kSizeDataBaseObjectHeader+ (fKeepStamps ? sizeof(StampsArray) : 0), nil, soustable);
					/*
					if (soustable!=nil) 
						soustable->Release();
						*/
#if debugplace
					shex.FromULong((uLONG)(tab[i].addr),kHexaDecimal);
					UnivStrDebug("LiberePlace Tabaddr = ",shex);
#endif
					vYield();
				}
			}
		}
	}

	setmodif(false, db, nil);
	if (getaddr() > 0)
	{
#if debug_Addr_Overlap
		db->DelDBObjRef(getaddr(), kSizeTabAddr+kSizeDataBaseObjectHeader+ (fKeepStamps ? sizeof(StampsArray) : 0));
#endif
		db->libereplace(getaddr(), kSizeTabAddr+kSizeDataBaseObjectHeader + (fKeepStamps ? sizeof(StampsArray) : 0), nil, this);
	}

	Free(curstack, true);

#if debugplace
	shex.FromULong((uLONG)getaddr(),kHexaDecimal);
	UnivStrDebug("LiberePlace Tabaddr = ",shex);
#endif

	return err;
}

/*
AddressTable* AddressTable::NewTable(Base4D *xdb)
{
	AddressTable* sous;

	sous=new AddressTable(xdb, prefseg);
	return(sous);
}
*/


DataAddr4D AddressTable::ConvertAddrTable(sLONG nbelem, bool inKeepStamps, VDB4DProgressIndicator* InProgress, OccupableStack* curstack, BaseTaskInfo* context, VError& err)
{
	Occupy(curstack, false);
	gOccPool.EndWaitForMemoryManager(curstack);
	err = loadobj();

	DataAddr4D result;
	if (err == VE_OK)
		result = db->findplace(kSizeTabAddr+kSizeDataBaseObjectHeader+(inKeepStamps ? sizeof(StampsArray) : 0), context, err, prefseg, this);

	if (err == VE_OK)
	{
		AddressTable* copy = new AddressTable(db, prefseg, inKeepStamps, fDebugCheckOnDelete);
		if (copy == nil)
			err = ThrowBaseError(memfull, noaction);
		else
		{
			copy->init();
			copy->setaddr(result);
			if (nbelem > kNbElemTabAddr)
			{
				for (sLONG i = 0; i < kNbElemTabAddr && err == VE_OK; i++)
				{
					gOccPool.WaitForMemoryManager(curstack);
					AddressTable* sous = fTabmem[i];
					if (sous == nil && tab[i].addr != 0)
					{
						gOccPool.EndWaitForMemoryManager(curstack);
						sous = new AddressTable(db, prefseg, fKeepStamps, fDebugCheckOnDelete);
						sous->posdansparent = i;
						err = sous->loadobj(tab[i].addr, i);
						if (err == VE_OK)
						{
							fTabmem[i] = sous;
						}
						else
						{
							sous->Release();
							sous = nil;
						}
					}
					else
						gOccPool.EndWaitForMemoryManager(curstack);
					
					if (sous != nil)
					{
						gOccPool.WaitForMemoryManager(curstack);
						fTabmem[i] = sous;
						copy->tab[i].addr = sous->ConvertAddrTable(nbelem / kNbElemTabAddr, inKeepStamps, InProgress, curstack, context, err);
						copy->tab[i].len = kSizeTabAddr+/*kSizeDataBaseObjectHeader+*/(inKeepStamps ? sizeof(StampsArray) : 0);
					}
				}
			}
			else
			{
				std::copy(&tab[0], &tab[kNbElemTabAddr], &(copy->tab[0]));
			}

			VSize totsaved;
			copy->posdansparent = posdansparent;
			copy->SaveObj(totsaved);
			copy->Release();
		}
	}

	Free(curstack, false);
	return result;
}


#if debugOverlaps_strong

Boolean AddressTable::Debug_CheckAddrOverlap(sLONG nbelem, sLONG curelem, DataAddr4D addrToCheck, sLONG lenToCheck, sLONG nomobjToCheck)
{
	Boolean result = false;
	Occupy();
	{
		sLONG n, xn2, i;
		AddrTable* sous;

		if (nbelem>kNbElemTabAddr)
		{
			if (nbelem>kNbElemTabAddr2)
			{
				n = (nbelem+kNbElemTabAddr2-1) >> kRatioTabAddr2;
				for (i=0;i<n;i++)
				{
					if (i==(n-1))
					{
						xn2 = nbelem & (kNbElemTabAddr2-1);
					}
					else
					{
						xn2 = kNbElemTabAddr2-1;
					}

					sous = (AddressTable*)(fTabmem[i]);
					if (sous!=nil)
					{
						result = result || sous->Debug_CheckAddrOverlap(xn2, curelem, addrToCheck, lenToCheck, nomobjToCheck);
					}
					curelem = curelem + kNbElemTabAddr2;
				}
			}
			else
			{
				n = (nbelem+kNbElemTabAddr-1) >> kRatioTabAddr;
				for (i=0;i<n;i++)
				{
					if (i==(n-1))
					{
						xn2 = nbelem & (kNbElemTabAddr-1);
					}
					else
					{
						xn2 = kNbElemTabAddr-1;
					}

					sous = (AddrTable*)(fTabmem[i]);
					if (sous!=nil)
					{
						result = result || sous->Debug_CheckAddrOverlap(xn2, curelem, addrToCheck, lenToCheck, nomobjToCheck);
					}
					curelem = curelem + kNbElemTabAddr;
				}
			}
		}
		else
		{
			for (i = 0; i <=nbelem; i++)
			{
				DataAddr4D addrobj = tab[i].addr;
				sLONG lenobj = tab[i].len;
				if ( (addrobj >= addrToCheck && addrobj < (addrToCheck+lenToCheck)) 
					|| (addrToCheck >= addrobj && addrToCheck < (addrobj+lenobj)) )
				{
					if (curelem != nomobjToCheck)
					{
						lenobj = lenobj; // put a break here;
						xbox_assert(! ((addrobj >= addrToCheck && addrobj < (addrToCheck+lenToCheck)) || (addrToCheck >= addrobj && addrToCheck < (addrobj+lenobj))) );
					}
				}
				curelem++;
			}

		}

		Free();
	}
	return result;
}

#endif





/* ----------------------------------------------- */


#if debugLogEventTabAddr

class dbgTabAddr
{
	public:
		inline dbgTabAddr()
		{
			fValue = 0;
			fNbelem = 0;
			fDebutDesTrous = 0;
		}

		inline dbgTabAddr(sLONG inValue, sLONG inNbelem, DataAddr4D inDebutDesTrous)
		{
			fValue = inValue;
			fNbelem = inNbelem;
			fDebutDesTrous = inDebutDesTrous;
		}

	sLONG fValue;
	sLONG fNbelem;
	DataAddr4D fDebutDesTrous;

};


EventLogger<dbgTabAddr, 1000> gDebugTabAddr;

#endif


sLONG UniqTableHeader = 0;


AddressTableHeader::AddressTableHeader()
{
	++UniqTableHeader;
	if (UniqTableHeader>2000000000)
		UniqTableHeader = 1;
	fIDUniq = UniqTableHeader;
	FirstPage=nil;
	db=nil;
	AddrFirstPage=nil;
	owner=nil;
	DebutTrou=nil;
	nbelem=nil;
	//original = nil;
	prefseg = 0;
	fKeepStamps = false;
	fDebugCheckOnDelete = false;
#if debugTabAddr
	fDebugMess = false;
#endif

}


AddressTableHeader::~AddressTableHeader()
{
	OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
	LibereEspaceMem(curstack);
}


void AddressTableHeader::LibereEspaceMem(OccupableStack* curstack)
{
	Occupy(curstack, true);
	gOccPool.WaitForMemoryManager(curstack);
	if (FirstPage != nil)
	{
		FirstPage->LiberePourClose(*nbelem, curstack);
		FirstPage->Release();
		FirstPage = nil;
	}
	else
		gOccPool.EndWaitForMemoryManager(curstack);
	Free(curstack, true);
}



bool AddressTableHeader::SaveObj(VSize& outSizeSaved)
{
	xbox_assert(false);
	outSizeSaved = 0;
	//owner->setmodif(true, db, nil);
	return true;
}


void AddressTableHeader::Init(Base4D *xdb, IObjToFlush *TheOwner, DataAddr4D* AddrOfFirst, DataAddr4D* AddrDebutTrou,
							  sLONG *AddrNBelem, sLONG xprefseg, bool inKeepStamps)
{
	db=xdb;
	AddrFirstPage=AddrOfFirst;
	owner=TheOwner;
	DebutTrou=AddrDebutTrou;
	nbelem=AddrNBelem;
	prefseg = xprefseg;
	fKeepStamps = inKeepStamps;
}


void AddressTableHeader::setmodif(uBOOL xismodif, Base4D* bd, BaseTaskInfo* context)
{
	owner->setmodif(xismodif,bd,context);
}


AddressTable* AddressTableHeader::NewTable(Base4D *xdb)
{
	AddressTable* sous;

	sous=new AddressTable(xdb, prefseg, fKeepStamps, fDebugCheckOnDelete);
	return(sous);
}


VError AddressTableHeader::ChargeTabAddr(BaseTaskInfo* context, OccupableStack* curstack)
{
	VError err = VE_OK;

	gOccPool.WaitForMemoryManager(curstack);
	if (FirstPage==nil)
	{
		gOccPool.EndWaitForMemoryManager(curstack);

		fLoadMutex.Lock();
		gOccPool.WaitForMemoryManager(curstack);
		if (FirstPage == nil)
		{
			gOccPool.EndWaitForMemoryManager(curstack);
			AddressTable* temp = NewTable(db);
			if (temp == nil)
				err = ThrowBaseError(memfull, DBaction_LoadingAddrTable);
			else
			{
				gOccPool.WaitForMemoryManager(curstack);
				temp->setaddr(*AddrFirstPage);
				temp->SetPosDansParent(0);
				FirstPage = temp;
			}
		}
		fLoadMutex.Unlock();

	}

	return err;
}


sLONG AddressTableHeader::findtrou(BaseTaskInfo* context, VError& err, OccupableStack* curstack)
{
	sLONG result;
	DataAddr4D ou;
	err = VE_OK;

	Occupy(curstack, true);

#if debugLogEventTabAddr
	gDebugTabAddr.AddMessage(L"Avant Findtrou", dbgTabAddr(-2, *nbelem, *DebutTrou));
#endif

	if (*DebutTrou==kFinDesTrou)
	{
		result=-1;
	}
	else
	{
		result=-*DebutTrou;
#if VERSIONDEBUG
		if (result < 0 || result > *nbelem)
		{
			sLONG xdebug = 1; // put a break here
			xbox_assert(result >= 0 && result <= *nbelem);
		}
#endif
		err = ChargeTabAddr(context, curstack);
		ou=FirstPage->GetAddrFromTable(*nbelem, result, err, curstack);

		if (err == VE_OK)
		{
			*DebutTrou = ou;
#if VERSIONDEBUG
			if (ou != kFinDesTrou)
			{
				DataAddr4D xou = -ou;
				if (xou < 0 || xou > *nbelem)
				{
					sLONG xdebug = 1; // put a break here
					xbox_assert(xou >= 0 && xou <= *nbelem);
				}
			}
#endif
			setmodif(true, db, context);
		}
		else
			result = -1;
	}

#if debugLogEventTabAddr
	gDebugTabAddr.AddMessage(L"Apres Findtrou", dbgTabAddr(result, *nbelem, *DebutTrou));
#endif

	Free(curstack, true);

	if (err != VE_OK)
		err = ThrowBaseError(VE_DB4D_CANNOT_ALLOCATE_NEW_ENTRY, DBaction_ModifyingAddrTable);
	return(result);
}


VError AddressTableHeader::liberetrou(sLONG n, BaseTaskInfo* context, OccupableStack* curstack, uLONG inStamp)
{
	VError err;
	DataAddr4D ou;

	Occupy(curstack, true);
#if debugLogEventTabAddr
	gDebugTabAddr.AddMessage(L"Avant liberetrou", dbgTabAddr(n, *nbelem, *DebutTrou));
#endif

	err = ChargeTabAddr(context, curstack);
	if (err == VE_OK)
	{
		//if (FirstPage->GetAddrFromTable(*nbelem, n, err) >= 0 && err == VE_OK)
		if (FirstPage != nil)
		{
			ou = *DebutTrou;
#if debugTabAddr
			if (fDebugMess)
			{
				VString smess("liberetrou");
				VString sdebugtaskid = L"Task ID "+ToString(VTask::GetCurrentID())+ L"  :  "+smess+"  :  ";
				DebugMsg(sdebugtaskid + L"PutxAddr "+ToString(ou)+L"  at "+ToString(n)+L"  ,  nbelem = "+ToString(*nbelem)+L"\n\n");
			}
#endif
#if VERSIONDEBUG
			if (ou != kFinDesTrou)
			{
				DataAddr4D xou = -ou;
				if (xou < 0 || xou > *nbelem)
				{
					sLONG xdebug = 1; // put a break here
					xbox_assert(xou >= 0 && xou <= *nbelem);
				}
			}
#endif
			err=FirstPage->PutAddrIntoTable(*nbelem,n,ou,-1,context, curstack, inStamp);
			*DebutTrou = -n;
			setmodif(true,db, context);
		}
		else
			gOccPool.EndWaitForMemoryManager(curstack);
	}
	else
		gOccPool.EndWaitForMemoryManager(curstack);

#if debugLogEventTabAddr
	gDebugTabAddr.AddMessage(L"Apres liberetrou", dbgTabAddr(n, *nbelem, *DebutTrou));
#endif

	Free(curstack, true);

	if (err != VE_OK)
		err = ThrowBaseError(VE_DB4D_CANNOT_FREE_ENTRY, DBaction_ModifyingAddrTable);

	return err;
}


VError AddressTableHeader::TakeAddrTemporary(sLONG n, BaseTaskInfo* context, sLONG* debuttroutrans, OccupableStack* curstack)
{
	Occupy(curstack, true);

#if debuglr
	xbox_assert (xTempAddr.find(n) == xTempAddr.end());
	xTempAddr.insert(n);
#endif

	xbox_assert(*debuttroutrans < 0);
	VError err = VE_OK;
	sLONG n2 = *debuttroutrans;
	if (n2 != kFinDesTrou)
	{
		n2 = n2 + 1;
		sLONG nprec = -n2;
		sLONG lenprec;
		DataAddr4D ou = GetxAddr(nprec, context, err, curstack, &lenprec);
		xbox_assert(lenprec == kDebutDesTrou);
		err = PutxAddr(nprec, ou, -n, context, curstack, 0, "TakeAddrTemporary");

	}
	if (err == VE_OK)
	{
		err = PutxAddr(n, n2, kDebutDesTrou, context, curstack, 0,  "TakeAddrTemporary");
	}
	if (err == VE_OK)
		*debuttroutrans = -n - 1;
	xbox_assert(*debuttroutrans < 0);

#if debugLogEventTabAddr
	gDebugTabAddr.AddMessage(L"TakeAddrTemporary", dbgTabAddr(n, *nbelem, *DebutTrou));
#endif

	Free(curstack, true);

	return err;
}


VError AddressTableHeader::GiveBackAddrTemporary(sLONG n, BaseTaskInfo* context, sLONG* debuttroutrans, OccupableStack* curstack)
{
	Occupy(curstack, true);
	xbox_assert(*debuttroutrans < 0);

#if debuglr
	xbox_assert (xTempAddr.find(n) != xTempAddr.end());
	xTempAddr.erase(n);
#endif

	VError err = VE_OK;
	sLONG precedant;
	DataAddr4D suivant = GetxAddr(n, context, err, curstack, &precedant);
	if (err == VE_OK && suivant<=0 && precedant <= 0) // il se peut que l'addresse temporaire ait deja ete rendue
	{
		sLONG nsuivant = (sLONG)suivant;
		sLONG lendontuse;

		if (nsuivant != kFinDesTrou)
		{
			DataAddr4D oudontuse = GetxAddr(-nsuivant, context, err, curstack, &lendontuse);
			xbox_assert(lendontuse == -n);
			err = PutxAddr(-nsuivant, oudontuse, precedant, context, curstack, 0, "GiveBackAddrTemporary");
		}

		if (precedant == kDebutDesTrou)
		{
			xbox_assert( *debuttroutrans == -n - 1);
			if (nsuivant == kFinDesTrou)
				*debuttroutrans = kFinDesTrou;
			else
				*debuttroutrans = nsuivant - 1;
		}
		else
		{
			sLONG anteprecedant;
			DataAddr4D ouprecedant = GetxAddr(-precedant, context, err, curstack, &anteprecedant);
			xbox_assert ((sLONG)-ouprecedant == n);
			sLONG n2;
			if (nsuivant == kFinDesTrou)
				n2 = kFinDesTrou;
			else
				n2 = nsuivant;
			err = PutxAddr(-precedant, n2, anteprecedant, context, curstack, 0, "GiveBackAddrTemporary");
		}

	}

	xbox_assert(*debuttroutrans < 0);

#if debuglr
	if (*debuttroutrans == kFinDesTrou)
		xbox_assert(xTempAddr.size() == 0);
#endif

#if debugLogEventTabAddr
	gDebugTabAddr.AddMessage(L"GiveBackAddrTemporary", dbgTabAddr(n, *nbelem, *DebutTrou));
#endif
	Free(curstack, true);

	return err;
}


VError AddressTableHeader::InitAndSetSize(sLONG maxelem, BaseTaskInfo* context, OccupableStack* curstack)
{
	VError err = VE_OK;
	Occupy(curstack, true);
	if (*nbelem == 0)
	{
		if (maxelem > 0)
		{
			AddressTable* soust = new AddressTable(db, prefseg, fKeepStamps, fDebugCheckOnDelete);
			if (soust != nil)
			{
				soust->init();
				err = soust->InitAndSetSize(maxelem, context, 0, curstack);
				*AddrFirstPage = soust->getaddr();
				soust->Release();
				if (err == VE_OK)
					*nbelem = maxelem;
			}
		}
	}
	else
		err = -2;
		//err = ThrowBaseError(VE_DB4D_NOTIMPLEMENTED, DBaction_ModifyingAddrTable);

	Free(curstack, true);

	if (err != VE_OK && err != -2)
		err = ThrowBaseError(VE_DB4D_CANNOT_MODIFY_ADDRTABLE, DBaction_ModifyingAddrTable);

	return err;
}


VError AddressTableHeader::Normalize(BaseTaskInfo* context, OccupableStack* curstack)
{
	sLONG debuttrou = kFinDesTrou;
	sLONG i,nb = *nbelem;
	VError err = VE_OK;

	Occupy(curstack, true);

	for (i = 0; i < nb && err == VE_OK; i++)
	{
		DataAddr4D ou = GetxAddr(i, context, err, curstack);
		if (err == VE_OK)
		{
			if (ou <= 0)
			{
				err = PutxAddr(i, debuttrou, -1, context, curstack, 0, "Normalize");
				if (err == VE_OK)
				{
					debuttrou = -i;
				}
			}
		}
	}

	*DebutTrou = debuttrou;
	setmodif(true, db, context);

	Free(curstack, true);

	if (err != VE_OK)
		err = ThrowBaseError(VE_DB4D_CANNOT_MODIFY_ADDRTABLE, DBaction_ModifyingAddrTable);

	return err;
}


VError AddressTableHeader::SetDebutDesTrous(sLONG val, BaseTaskInfo* context)
{
	*DebutTrou = val;
	setmodif(true, db, context);

	return VE_OK;
}


VError AddressTableHeader::LibereEspaceDisk(OccupableStack* curstack, VDB4DProgressIndicator* InProgress)
{
	//gOccPool.WaitForMemoryManager(curstack);
	Occupy(curstack, true);
	//gOccPool.EndWaitForMemoryManager(curstack);
#if debugLogEventTabAddr
	gDebugTabAddr.AddMessage(L"avant LibereEspaceDisk", dbgTabAddr(-1, *nbelem, *DebutTrou));
#endif

	VError err = VE_OK;
	if (InProgress != nil)
	{
		XBOX::VString session_title;
		gs(1005,21,session_title);	//  Deleting Address Table: %curValue of %maxValue Entries
		InProgress->BeginSession(*nbelem,session_title);
	}

	err = ChargeTabAddr(nil, curstack);
	if (FirstPage != nil && err == VE_OK)
	{
		err = FirstPage->SubLibereEspaceDisk(*nbelem, InProgress, curstack);
		FirstPage->SetPosDansParent(0);
		FirstPage->Release();
		FirstPage = nil;
	}

	if (err == VE_OK)
	{
		*AddrFirstPage = 0;
		*DebutTrou = kFinDesTrou;
		*nbelem = 0;
	}

	if (InProgress != nil)
	{
		InProgress->EndSession();
	}

	LibereEspaceMem(curstack);

#if debugLogEventTabAddr
	gDebugTabAddr.AddMessage(L"apres LibereEspaceDisk", dbgTabAddr(-1, *nbelem, *DebutTrou));
#endif

	Free(curstack, true);

	return err;
}


VError AddressTableHeader::TakeOffBits(Bittab *tb, BaseTaskInfo* context, OccupableStack* curstack, bool UseAlternateDebut, DataAddr4D AlternateDebut)
{
	VError err;
	DataAddr4D ou;
	Bittab dejapass;

	err=0;
	Occupy(curstack, true);

	if (UseAlternateDebut)
		ou = AlternateDebut;
	else
		ou=*DebutTrou;

	// L.E. 17/10/08 ACI0059051 remove deleted records that are not in free list
	tb->ClearFrom( GetNbElem());

	// ClearError();
	while ((ou!=kFinDesTrou) && (err==VE_OK))
	{
		ou = -ou;
		if (dejapass.isOn(ou))
			err = ThrowBaseError(VE_DB4D_CIRCULAR_REF_IN_REC_ADDR_TABLE);
		else
		{
			err = dejapass.Set(ou);
			if (err == VE_OK)
			{
				tb->ClearOrSet(ou,false);
				ou = GetxAddr(ou, context, err, curstack);
				/*
				err = ChargeTabAddr(context);
				if (err == VE_OK)
				{
					ou = FirstPage->GetAddrFromTable(*nbelem,ou, err);
				}
				*/
			}
		}
	}

	Free(curstack, true);
	return(err);
}


DataAddr4D AddressTableHeader::GetxAddr(sLONG n, BaseTaskInfo* context, VError& err, OccupableStack* curstack, sLONG* outLen, uLONG* outStamp)
{
	DataAddr4D ou = 0;

	err = VE_OK;

	//gOccPool.WaitForMemoryManager(curstack);
	Occupy(curstack, true);
	//gOccPool.EndWaitForMemoryManager(curstack);
	if (n < *nbelem)
	{
		err = ChargeTabAddr(context, curstack);
		if (err == VE_OK)
		{
			ou=FirstPage->GetAddrFromTable(*nbelem,n, err, curstack, outLen, outStamp);
		}
	}
	Free(curstack, true);

	xbox_assert(ou < 0 || db->IsAddrValid(ou));

	return(ou);
}


VError AddressTableHeader::PutxAddr(sLONG n, DataAddr4D ValAddr, sLONG len, BaseTaskInfo* context, OccupableStack* curstack, uLONG inStamp, const char* debugmess )
{
	AddressTable* sousADT;
	VError err = VE_OK;

	xbox_assert(ValAddr <= 0 || db->IsAddrValid(ValAddr));		// mbucatariu, vu avec L.E., pour le compactage, on copie des trous, context = NULL
	// et la les addresses ne sont pas des adresses valides																 
	Occupy(curstack, true);

#if debugLogEventTabAddr
	gDebugTabAddr.AddMessage(L"avant PutxAddr", dbgTabAddr(n, *nbelem, *DebutTrou));
#endif

#if debuglr == 1027
	if (ValAddr>0 && db->GetStructure() != nil)
	{
		VString sdebugtaskid = L"Task ID "+ToString(VTask::GetCurrentID())+ L"  :  ";
		DebugMsg(sdebugtaskid + L"Enter PutxAddr "+ToString(ValAddr)+L"  at "+ToString(n)+L"  ,  nbelem = "+ToString(*nbelem)+L"\n\n");
	}
#endif
#if debugTabAddr
	if (fDebugMess)
	{
		VString smess;
		if (debugmess != nil)
		{
			VString s(debugmess);
			smess = s;
		}
		VString sdebugtaskid = L"Task ID "+ToString(VTask::GetCurrentID())+ L"  :  "+smess+"  :  ";
		DebugMsg(sdebugtaskid + L"PutxAddr "+ToString(ValAddr)+L"  at "+ToString(n)+L"  ,  nbelem = "+ToString(*nbelem)+L"\n\n");
	}
#endif

	if (*nbelem==0)
	{
		if (AccesModifOK(context))
		{
			sousADT=NewTable(db);
			gOccPool.WaitForMemoryManager(curstack);
			AddressTable* temp = sousADT->AddToTable(-kNbElemTabAddr, context, err, curstack);
			sousADT->Release();
			gOccPool.WaitForMemoryManager(curstack);
			FirstPage = temp;
			if (err == VE_OK)
			{
				xbox_assert(FirstPage != nil);
				FirstPage->SetPosDansParent(0);
				*AddrFirstPage=FirstPage->getaddr();
				gOccPool.EndWaitForMemoryManager(curstack);
				setmodif(true,db,context);
				gOccPool.WaitForMemoryManager(curstack);
				FirstPage->Free(curstack, true);
			}
			else
				gOccPool.EndWaitForMemoryManager(curstack);
		}
		else 
			err = VE_DB4D_TRANSACTIONCONFLICT;
	}
	else
	{
		err = ChargeTabAddr(context, curstack);
	}

	if ((err==0) && (n>=*nbelem))
	{
		if (AccesModifOK(context))
		{
			if ( ((((*nbelem)/*+1*/) & (kNbElemTabAddr-1)) == 0) && (*nbelem != 0) )
			{
				sousADT=FirstPage->AddToTable((*nbelem)/*+1*/, context, err, curstack);
				if (err == VE_OK)
				{
					xbox_assert(sousADT != nil);
					gOccPool.WaitForMemoryManager(curstack);
					FirstPage=sousADT;
					*AddrFirstPage=FirstPage->getaddr();
					*nbelem=*nbelem+1;
					gOccPool.EndWaitForMemoryManager(curstack);
					setmodif(true,db,context);
					sousADT->Free(curstack, true);
				}
			}
			else
			{
				gOccPool.EndWaitForMemoryManager(curstack);
				*nbelem=*nbelem+1;
				setmodif(true,db,context);
			}
		}
		else 
		{
			gOccPool.EndWaitForMemoryManager(curstack);
			err = ThrowBaseError(VE_DB4D_TRANSACTIONCONFLICT, DBaction_ModifyingAddrTable);
		}
		if (err == VE_OK)
			gOccPool.WaitForMemoryManager(curstack);
	}	


	if (err == VE_OK)
	{
		err=FirstPage->PutAddrIntoTable(*nbelem,n,ValAddr,len,context, curstack, inStamp);
	}

#if debuglr == 1027
	if (ValAddr>0 && db->GetStructure() != nil)
	{
		VString sdebugtaskid = L"Task ID "+ToString(VTask::GetCurrentID())+ L"  :  ";
		DebugMsg(sdebugtaskid + L"Exit PutxAddr "+ToString(ValAddr)+L"  at "+ToString(n)+L"  ,  nbelem = "+ToString(*nbelem)+L"\n\n");
	}
#endif

#if debugLogEventTabAddr
	gDebugTabAddr.AddMessage(L"apres PutxAddr", dbgTabAddr(n, *nbelem, *DebutTrou));
#endif

	Free(curstack, true);
	return(err);
}


VError AddressTableHeader::TakeAddr(sLONG n, BaseTaskInfo* context, OccupableStack* curstack)
{
	VError err = VE_OK;

	Occupy(curstack, true);

#if debugLogEventTabAddr
	gDebugTabAddr.AddMessage(L"avant TakeAddr", dbgTabAddr(n, *nbelem, *DebutTrou));
#endif

	if (n >= *nbelem)
	{
		sLONG curtrou = *nbelem;
		while (n > *nbelem && err == VE_OK)
		{
			DataAddr4D ou = *DebutTrou;
			err = PutxAddr(curtrou, ou, -1, context, curstack, 0, "TakeAddr");
			if (err == VE_OK)
				*DebutTrou = -curtrou;
			curtrou++;
		}
		if (err == VE_OK)
		{
			err = PutxAddr(curtrou, 0, 0, context, curstack, 0, "TakeAddr");
		}
	}
	else
	{
		Boolean found = false;
		sLONG ou = *DebutTrou;
		sLONG previous = -1;
		// ClearError();
		while ((ou != kFinDesTrou) && (err==VE_OK))
		{
			if ((-ou) == n)
			{
				DataAddr4D ou2 = GetxAddr(n, context, err, curstack);
				if (testAssert(ou2 <= 0))
				{
					ou = (sLONG)ou2;
					if (err == VE_OK)
					{
						if (previous == -1)
						{
							*DebutTrou = ou;
#if VERSIONDEBUG
							if (ou != kFinDesTrou)
							{
								DataAddr4D xou = -ou;
								if (xou < 0 || xou > *nbelem)
								{
									sLONG xdebug = 1; // put a break here
									xbox_assert(xou >= 0 && xou <= *nbelem);
								}
							}
#endif
						}
						else
							err = PutxAddr(previous, ou, -1, context, curstack, 0, "TakeAddr");
					}
				}
				else
					err = ThrowBaseError(VE_DB4D_ADDR_ENTRY_IS_NOT_EMPTY, DBactionFinale);

				if (err == VE_OK)
				{
					found = true;
					break;
				}

			}
			else
			{
				ou = -ou;
				previous = ou;
				DataAddr4D ou2 = GetxAddr(ou, context, err, curstack);
				xbox_assert(ou2 <= 0);
				ou = (sLONG)ou2;
			}
		}

		if (!found)
			err = ThrowBaseError(VE_DB4D_ADDR_ENTRY_IS_NOT_EMPTY, DBactionFinale);
	}

#if debugLogEventTabAddr
	gDebugTabAddr.AddMessage(L"apres TakeAddr", dbgTabAddr(n, *nbelem, *DebutTrou));
#endif

	Free(curstack, true);

	if (err != VE_OK)
		err = ThrowBaseError(VE_DB4D_CANNOT_PREALLOCATE_ADDR, DBactionFinale);

#if debuglr == 214
	{
		sLONG ou = *DebutTrou;
		sLONG previous = -1;
		while ((ou != kFinDesTrou))
		{
			ou = -ou;
			xbox_assert(ou != n);
			if (ou == n)
			{
				ou = ou; // break here
			}
			DataAddr4D ou2 = GetxAddr(ou, context, err);
			if (testAssert(ou2 <= 0))
			{
			}
			else
			{
				ou = ou; // break here
			}
			ou = (sLONG)ou2;
		}
	}
#endif

	return err;
}



bool AddressTableHeader::TryToFreeMem(sLONG allocationBlockNumber, VSize combien, VSize& outFreed)
{
	bool purged = true;
	outFreed = 0;

	if (FirstPage!=nil)
	{
		if (FirstPage->TryToFreeMem(allocationBlockNumber, *nbelem, combien, outFreed) && !IsOccupied())
		{
			FirstPage->Release();
			FirstPage = nil;
		}
		else 
			purged = false;
	}

	return purged ;
}


VError AddressTableHeader::ConvertAddrTable(bool inKeepStamps, VDB4DProgressIndicator* InProgress, OccupableStack* curstack)
{
	VError err = VE_OK;
	xbox_assert(fKeepStamps != inKeepStamps);
	BaseTaskInfo* context = new BaseTaskInfo(db, nil, nil, nil);
	Occupy(curstack, true);
	if (*nbelem > 0)
	{
		err = ChargeTabAddr(context, curstack);
		if (err == VE_OK && FirstPage != nil)
		{
			DataAddr4D oldaddr = *AddrFirstPage;

			DataAddr4D newaddr = FirstPage->ConvertAddrTable(*nbelem, inKeepStamps, InProgress, curstack, context, err);

			if (err == VE_OK)
			{
				gOccPool.WaitForMemoryManager(curstack);
				FirstPage->SubLibereEspaceDisk(*nbelem, InProgress, curstack);
				gOccPool.WaitForMemoryManager(curstack);
				AddressTable* oldfirst = FirstPage;
				FirstPage = nil;
				oldfirst->LiberePourClose(*nbelem, curstack);
				*AddrFirstPage = newaddr;
				fKeepStamps = inKeepStamps;
				setmodif(true, db, context);
			}
			else
			{
				AddressTable* newfirst = new AddressTable(db, prefseg, inKeepStamps, fDebugCheckOnDelete);
				newfirst->loadobj(newaddr, 0);
				gOccPool.WaitForMemoryManager(curstack);
				newfirst->SubLibereEspaceDisk(*nbelem, InProgress, curstack);
				newfirst->Release();

				*AddrFirstPage = oldaddr;

			}
		}
	}
	Free(curstack, true);
	context->Release();
	return err;
}



VError AddressTableHeader::GetFragmentation(sLONG8& outTotalRec, sLONG8& outFrags)
{
	VError err = VE_OK;
	OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
	Occupy(curstack, true);
	
	sLONG totalrec = 0;
	sLONG frags = 0;
	DataAddr4D oldou = 0;
	sLONG oldlen;
	sLONG nb = GetNbElem();
	for (sLONG i = 0; i < nb && err == VE_OK; i++)
	{
		sLONG len;
		DataAddr4D ou = GetxAddr(i, nil, err, curstack, &len);
		if (ou != 0)
		{
			len = len + kSizeRecordHeader;
			ou=ou & (DataAddr4D)(-kMaxSegData);
			if (oldou != 0)
			{
				totalrec++;
				if ( (oldou + (DataAddr4D)(adjuste(oldlen))) != ou )
				{
					frags++;
				}
			}
			oldou = ou;
			oldlen = len;
		}
	}

	outTotalRec = totalrec;
	outFrags = frags;

	Free(curstack, true);
	return err;
}




#if debugOverlaps_strong

Boolean AddressTableHeader::Debug_CheckAddrOverlap(DataAddr4D addrToCheck, sLONG lenToCheck, sLONG nomobjToCheck)
{
	Boolean result = false;
	Occupy()
	
	if (FirstPage != nil)
	{
		result = FirstPage->Debug_CheckAddrOverlap(*nbelem, 0, addrToCheck, lenToCheck, nomobjToCheck);
	}
	Free();
	
	return result;
}

#endif


#if debug_Addr_Overlap

void AddressTableHeader::FillDebug_DBObjRef(sLONG tablenum, bool isrectable)
{
	OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
	Occupy(curstack, true);
	if (*nbelem > 0)
	{
		VError err = ChargeTabAddr(nil, curstack);
		if (FirstPage != nil)
		{
			FirstPage->FillDebug_DBObjRef(*nbelem, curstack, tablenum, isrectable);
		}
	}
	Free(curstack, true);
}

#endif









// old code --------------------------------------------------------------------------------------------------------------




#define wrongway_to_do 0


TreeInMem::TreeInMem(sWORD DefaultAccess, typobjcache typ, uBOOL needlock):ObjCacheInTree(DefaultAccess)
{
#if debuglrWithTypObj
	typobj=t_treeinmem;
#endif
	fNeedLock = needlock;
	ContientQuoi = typ;
	FeuilleFinaleContientQuoi = typ;

	tabmem = &xTabMem[0];
	std::fill(&xTabMem[0], &xTabMem[kNbElemTreeMem], (ObjCacheInTree*)nil);
	/*
	tabmem=(ObjCacheInTree**)GetFastMem(kSizeTreeMem, false, 'tmem');
	if (tabmem!=nil)
	{
		_raz(tabmem,kSizeTreeMem);
	}
	*/

}

TreeInMem::~TreeInMem()
{
#if 0 && VERSIONDEBUG
	if (tabmem != nil)
	{
		for (sLONG i = 0; i < kNbElemTreeMem; i++)
		{
			xbox_assert(tabmem[i] == nil);
		}
	}
#endif
	/*
	if (tabmem!=nil)
	{
		FreeFastMem(tabmem);
	}
	*/
}


#if autocheckobj
uBOOL TreeInMem::CheckObjInMem(void)
{
	sLONG i,nb;
	
	CheckAssert(ObjCacheInTree::CheckObjInMem());
	
	CheckAssert(IsValidFastPtr(tabmem));
	
	nb = 0;
	if (tabmem != nil)
	{
		for (i=0; i<kNbElemTreeMem; i++)
		{
			if (tabmem[i] != nil)
			{
				nb++;
			}
			CheckAssert(IsValidFastPtr(tabmem[i]));
		}
	}
	
	CheckAssert(nb == nbelemtab);
	
	return(true);
}
#endif


TreeInMem* TreeInMem::CreTreeInMem(void)
{
	sWORD _access = TreeInMemAccess;
	if (FeuilleFinaleContientQuoi == t_ficheondisk)
		_access = RecordAccess;
	return(new TreeInMem(_access, FeuilleFinaleContientQuoi, fNeedLock));
}


void* TreeInMem::DelFrom(sLONG nbelem, sLONG n)
{
	sLONG n1,xn2;
	TreeInMem* sous;
	
	VObjLockPotentiel lock(this, fNeedLock);
	//occupe();
	
	n1=n>>kRatioTabAddr;
	xn2=n & (kNbElemTreeMem-1);
	
	if (nbelem>=kNbElemTreeMem)
	{
		if (nbelem>=kNbElemTreeMem2)
		{
			n1=n1>>kRatioTreeMem;
			xn2=n & (kNbElemTreeMem2-1);
			nbelem=kNbElemTreeMem2-1;
		}
		else
		{
			nbelem=kNbElemTreeMem-1;
		}
		sous=(TreeInMem*)(tabmem[n1]);
		if (sous!=nil)
		{
			return sous->DelFrom(nbelem,xn2);
		}
		else
			return nil;
	}
	else
	{
		void* obj = tabmem[xn2];
		InvalidateNbElemTab();
		tabmem[xn2]=nil;
		return obj;
	}
	
	//libere();
}


Boolean TreeInMem::TryToPurge(sLONG nbelem)
{
	Boolean purged = true;
	sLONG i,n,xn2;
	TreeInMem* sous;
	
	VObjLockPotentiel lock(this, fNeedLock);
	//occupe();
	
	if (nbelem>=kNbElemTreeMem)
	{
		if (nbelem>=kNbElemTreeMem2)
		{
			n = (nbelem+kNbElemTreeMem2-1) >> kRatioTreeMem2;
			for (i=0;i<n;i++)
			{
#if wrongway_to_do
				if (i==(n-1))
				{
					xn2 = nbelem & (kNbElemTreeMem2-1);
				}
				else
				{
					xn2 = kNbElemTreeMem2-1;
				}
#endif
				xn2 = kNbElemTreeMem2 - 1;
				
				sous = (TreeInMem*)(tabmem[i]);
				if (sous!=nil)
				{
					if (sous->TryToPurge(xn2))
					{
						delete sous;
						tabmem[i] = nil;
					}
					else
						purged = false;
				}
			}
		}
		else
		{
			n = (nbelem+kNbElemTreeMem-1) >> kRatioTreeMem;
			for (i=0;i<n;i++)
			{
#if wrongway_to_do
				if (i==(n-1))
				{
					xn2 = nbelem & (kNbElemTreeMem-1);
				}
				else
				{
					xn2 = kNbElemTreeMem-1;
				}
#endif
				xn2 = kNbElemTreeMem-1;

				sous = (TreeInMem*)(tabmem[i]);
				if (sous!=nil)
				{
					if (sous->TryToPurge(xn2))
					{
						delete sous;
						tabmem[i] = nil;
					}
					else
						purged = false;
				}
			}
		}
	}
	else
	{
		for (i=0;i<=nbelem;i++) 
		{
			ObjCacheInTree *obj = tabmem[i];

			if (obj!=nil)
			{
				if (FeuilleFinaleContientQuoi == t_locker)
				{
					LockEntity* lle = (LockEntity*)obj;
					if (lle->GetOwner() != nil)
						purged = false;
				}
				else
					purged = false;
			}
		}
	}
	
	//libere();
	return purged;
}


void TreeInMem::LiberePourClose(sLONG nbelem, uBOOL DoisDisposeElem)
{
	sLONG i,n,xn2;
	TreeInMem* sous;
	
	VObjLockPotentiel lock(this, fNeedLock);
	//occupe();
	
	if (nbelem>=kNbElemTreeMem)
	{
		if (nbelem>=kNbElemTreeMem2)
		{
			n = (nbelem+kNbElemTreeMem2-1) >> kRatioTreeMem2;
			for (i=0;i<n;i++)
			{
#if wrongway_to_do
				if (i==(n-1))
				{
					xn2 = nbelem & (kNbElemTreeMem2-1);
				}
				else
				{
					xn2 = kNbElemTreeMem2-1;
				}
#endif
				xn2 = kNbElemTreeMem2-1;

				sous = (TreeInMem*)(tabmem[i]);
				if (sous!=nil)
				{
					sous->LiberePourClose(xn2,DoisDisposeElem);
					delete sous;
					tabmem[i] = nil;
				}
			}
		}
		else
		{
			n = (nbelem+kNbElemTreeMem-1) >> kRatioTreeMem;
			for (i=0;i<n;i++)
			{
#if wrongway_to_do
				if (i==(n-1))
				{
					xn2 = nbelem & (kNbElemTreeMem-1);
				}
				else
				{
					xn2 = kNbElemTreeMem-1;
				}
#endif
				xn2 = kNbElemTreeMem-1;

				sous = (TreeInMem*)(tabmem[i]);
				if (sous!=nil)
				{
					sous->LiberePourClose(xn2,DoisDisposeElem);
					delete sous;
					tabmem[i] = nil;
				}
			}
		}
	}
	else
	{
		if (DoisDisposeElem)
		{
			for (i=0;i<=nbelem;i++) {
				ObjCacheInTree *obj = tabmem[i];
				tabmem[i]=nil;

				if (obj != nil)
				{
					if (ContientQuoi == t_ficheondisk)
					{
						FicheOnDisk* ficD = (FicheOnDisk*)obj;
						ficD->setmodif(false, ficD->GetDB(), nil);
						ficD->Release();
						/*
						if (ficD->IsOkToOccupe())
						{
							ficD->use();
							ficD->setmodif(false, ficD->GetDB(), nil);
							ficD->SetDansCache(false);
							ficD->FreeAfterUse();
						}
						else
							ficD->SetDansCache(false);
							*/
					}
					else
					{
						if (ContientQuoi == t_blob4d)
						{
							Blob4D* blob = (Blob4D*)obj;
							//if (blob->IsOkToOccupe())
							{
								blob->setmodif(false, blob->GetDF()->GetDB(), nil);
								//blob->SetDansCache(false);
								blob->SetNotExisting();
								blob->Kill();
							}
						}
						else
						{
							obj->Kill();
						}
					}
				}
			}
		}
	}
	
	InvalidateNbElemTab();
	//libere();
}


ObjCacheInTree* TreeInMem::GetFrom(sLONG nbelem, sLONG n)
{
	sLONG n1,xn2;
	TreeInMem* sous;
	ObjCacheInTree* result;
	
	xbox_assert( n >= 0 && n <= nbelem); // L.E. 23/01/01 actually don't know if 0 is ok
	//xbox_assert( nbelem > 0);

	result=nil;
	VObjLockPotentiel lock(this, fNeedLock);
	//occupe();
	
	if (n >= 0 && n <= nbelem)
	{
		n1=n>>kRatioTabAddr;
		xn2=n & (kNbElemTreeMem-1);
		
		if (nbelem>=kNbElemTreeMem)
		{
			if (nbelem>=kNbElemTreeMem2)
			{
				n1=n1>>kRatioTreeMem;
				xn2=n & (kNbElemTreeMem2-1);
				nbelem=kNbElemTreeMem2-1;
			}
			else
			{
				nbelem=kNbElemTreeMem-1;
			}
			sous=(TreeInMem*)(tabmem[n1]);
			if (sous!=nil)
			{
				result=sous->GetFrom(nbelem,xn2);
			}
		}
		else
		{
			result=tabmem[xn2];
		}
	}
	
	//libere();
	return(result);
}



ObjCacheInTree* TreeInMem::GetFromAndOccupe(sLONG nbelem, sLONG n, Boolean& dejaoccupe)
{
	sLONG n1,xn2;
	TreeInMem* sous;
	ObjCacheInTree* result;

	xbox_assert( n >= 0 && n <= nbelem); // L.E. 23/01/01 actually don't know if 0 is ok
	xbox_assert( nbelem > 0);

	result=nil;
	occupe();

	if (n >= 0 && n <= nbelem)
	{
		n1=n>>kRatioTabAddr;
		xn2=n & (kNbElemTreeMem-1);

		if (nbelem>=kNbElemTreeMem)
		{
			if (nbelem>=kNbElemTreeMem2)
			{
				n1=n1>>kRatioTreeMem;
				xn2=n & (kNbElemTreeMem2-1);
				nbelem=kNbElemTreeMem2-1;
			}
			else
			{
				nbelem=kNbElemTreeMem-1;
			}
			sous=(TreeInMem*)(tabmem[n1]);
			if (sous!=nil)
			{
				result=sous->GetFromAndOccupe(nbelem,xn2,dejaoccupe);
			}
		}
		else
		{
			result=tabmem[xn2];
			if (result != nil)
			{
				if ( !((Obj4D*)result)->IsOkToOccupe('GFAO') )
				{
					dejaoccupe = true;
				}
			}
		}
	}

	libere();
	return(result);
}


VError TreeInMem::PutInto(sLONG nbelem, sLONG n, ObjCacheInTree* data, uBOOL ContientObjAutre)
{
	sLONG n1,xn2;
	VError err = VE_OK;
	TreeInMem* sous;
	
	VObjLockPotentiel lock(this, fNeedLock);
	//occupe();
	n1=n>>kRatioTabAddr;
	xn2=n & (kNbElemTreeMem-1);
	
	if (nbelem>=kNbElemTreeMem)
	{
		ContientQuoi = t_treeinmem;
		if (nbelem>=kNbElemTreeMem2)
		{
			n1=n1>>kRatioTreeMem;
			xn2=n & (kNbElemTreeMem2-1);
			nbelem=kNbElemTreeMem2-1;
		}
		else
		{
			nbelem=kNbElemTreeMem-1;
		}

		sous=(TreeInMem*)(tabmem[n1]);
		if (sous==nil)
		{
			sous=CreTreeInMem();
			if (sous==nil)
			{
				err = ThrowBaseError(memfull, DBaction_StoringObjectInCache);
			}
			else
			{
				if (sous->okcree())
				{
					tabmem[n1]=(ObjCacheInTree*)sous;
					sous->SetParent(this,n1);
					sous->libere();
				}
				else
				{
					err = ThrowBaseError(memfull, DBaction_StoringObjectInCache);
					sous->libere();
					delete sous;
				}
			}
			
		}
		if (err==VE_OK)
		{
			err=sous->PutInto(nbelem,xn2,data, ContientObjAutre);
		}
	}
	else
	{
		tabmem[xn2]=data;
		InvalidateNbElemTab();
		if (data != nil)
		{
			if (!ContientObjAutre) 
				data->SetParent(this,xn2);
		}
	}
	
	//libere();
	
	return(err);
}


uBOOL TreeInMem::okdel(void)
{
	return(GetNbElemTab()==0 || ForceDeleted() );
}


sLONG TreeInMem::saveobj(void)
{
	return(0);
}


void TreeInMem::DelFromParent(sLONG n, ObjCacheInTree* enfant)
{
	if (tabmem!=nil)
	{
		if (tabmem[n]!=nil)
		{
			xbox_assert(tabmem[n] == enfant);
			InvalidateNbElemTab();
			tabmem[n]=nil;
		}
	}
}


void TreeInMem::RecalcNbElemTab()
{
	nbelemtab = 0;
	if (tabmem != nil)
	{
		ObjCacheInTree** p = &tabmem[0];
		for (sLONG i = 0; i<kNbElemTreeMem; i++)
		{
			if (*p != nil)
				nbelemtab++;
			p++;
		}
	}
}


sLONG TreeInMem::liberemem(sLONG allocationBlockNumber, sLONG combien, uBOOL tout)
{
	sLONG tot, tot2;
	ObjAlmostInCache* obj;
	
	tot=0;
	if ((GetParent()==nil) || (!GetParent()->occupe()))
	{
		if (GetParent()->GetCountOccupe() == 1)
		{
			if (combien==-1) combien=kMaxPositif;
			
			if (tabmem!=nil && (ContientQuoi == t_ficheondisk || ContientQuoi == t_blob4d))
			{
				sLONG i;
				for (i=0; i<kNbElemTreeMem; i++)
				{
					obj = (ObjAlmostInCache*)tabmem[i];
					if (obj != nil)
					{
						if (!obj->occupe(false, 'libm'))
						{
							if (!obj->modifie() && (obj->GetCountOccupe() == 1))
							{
								if (ContientQuoi == t_blob4d)
								{
									((Blob4D*)obj)->SetOKToDelete(true);
								}
								tot2=obj->liberemem(allocationBlockNumber, combien,false);
								if (tot2>2)
								{
									tot = tot + tot2;
								}
								if ((tot2>0) && obj->okdel())
								{
	#if debuglr == 121
									if (ContientQuoi == t_ficheondisk)
									{
										FicheOnDisk* rec = ((FicheOnDisk*)obj);
										if (VDBMgr::GetFlushManager()->FindObjInFlush(rec))
										{
											rec = rec; // put a break here
										}
										if (VDBMgr::GetFlushManager()->FindObjInFlushByAddr(rec->getaddr()))
										{
											rec = rec; // put a break here
										}
									}
	#endif

									tabmem[i] = nil;
									obj->libere(); 
									obj->Kill();
									InvalidateNbElemTab();
								}
								else
								{
									if (ContientQuoi == t_blob4d)
									{
										((Blob4D*)obj)->SetOKToDelete(false);
									}
									obj->libere();
								}
							}
							else 
							{
								obj->libere();
							}
						}
					}
				}
				
			}

			if (GetNbElemTab()==0)
			{
				tot = tot + kSizeTreeMem;
				//FreeFastMem(tabmem);
				//tabmem=nil;
				if (GetParent()!=nil) 
					GetParent()->DelFromParent(posdansparent, this);
				// CHECKMEM;
			}
			else tot = -tot;
		}

		if (GetParent()!=nil) 
			GetParent()->libere();
	}
	return(tot);
}


void TreeInMem::DeleteElem( ObjCacheInTree *inObject)
{
	delete inObject;
}


#if debuglr

void TreeInMem::CheckObjRef(sLONG nbelem, ObjCacheInTree* objtocheck)
{
	sLONG i,n,xn2;
	TreeInMem* sous;

	occupe();

	if (nbelem>=kNbElemTreeMem)
	{
		if (nbelem>=kNbElemTreeMem2)
		{
			n = (nbelem+kNbElemTreeMem2-1) >> kRatioTreeMem2;
			for (i=0;i<n;i++)
			{
				xn2 = kNbElemTreeMem2-1;

				sous = (TreeInMem*)(tabmem[i]);
				if (sous!=nil)
				{
					sous->CheckObjRef(xn2,objtocheck);
				}
			}
		}
		else
		{
			n = (nbelem+kNbElemTreeMem-1) >> kRatioTreeMem;
			for (i=0;i<n;i++)
			{
				xn2 = kNbElemTreeMem-1;

				sous = (TreeInMem*)(tabmem[i]);
				if (sous!=nil)
				{
					sous->CheckObjRef(xn2,objtocheck);
				}
			}
		}
	}
	else
	{
		for (i=0;i<=nbelem;i++) 
		{
			ObjCacheInTree *obj = tabmem[i];

			if (obj == objtocheck)
			{
				sLONG xdebug = 1; // put a break here
				xbox_assert(false);
			}
		}
	}

	libere();
}

#endif



								/* ----------------------------------------------- */


TreeInMemHeader::TreeInMemHeader()
{
	tmem=nil;
	nbelem=0;
	fNeedLock = true;
}


TreeInMemHeader::~TreeInMemHeader()
{
	/*
	if (tmem!=nil)
	{
		tmem->SetParent(nil,0);
		tmem->LiberePourClose(nbelem,!ContientObjetAutre);
		delete tmem;
	}
	*/
	LibereEspaceMem();
	
}

void TreeInMemHeader::LibereEspaceMem()
{
	if (tmem!=nil)
	{
		tmem->SetParent(nil,0);
		sLONG xnbelem = nbelem;
		if (xnbelem < kNbElemTreeMem)
			xnbelem--;
		tmem->LiberePourClose(xnbelem, true /*!ContientObjetAutre*/);
		delete tmem;
		tmem = nil;
	}
	nbelem = 0;
}


Boolean TreeInMemHeader::TryToPurge()
{
	VObjLockPotentiel lock(this, fNeedLock);
	//occupe();
	
	Boolean purged = true;
	if (tmem!=nil)
	{
		sLONG xnbelem = nbelem;
		if (xnbelem < kNbElemTreeMem)
			xnbelem--;
		if (tmem->TryToPurge(xnbelem))
		{
			tmem->SetParent(nil,0);
			delete tmem;
			tmem = nil;
			nbelem = 0;
		}
		else 
			purged = false;
	}
	else
		nbelem = 0;
	
	//libere();
	return purged;
}


void TreeInMemHeader::DelFromParent(sLONG n, ObjCacheInTree* enfant)
{
	xbox_assert(n == 0);
	xbox_assert(tmem == enfant);
	tmem=nil;
	InvalidateNbElemTab();
}


void TreeInMemHeader::RecalcNbElemTab()
{
	if (tmem == nil)
	{
		nbelemtab = 0;
	}
	else
		nbelemtab = 1;
}


void TreeInMemHeader::Init(sLONG NbElements, uBOOL ContientObjAutre)
{
	nbelem=NbElements;
	ContientObjetAutre = ContientObjAutre;
}


VError TreeInMemHeader::PutIntoTreeMem(sLONG NewMax, sLONG n, ObjCacheInTree *data)
{
	VError err = VE_OK;
	TreeInMem *sous;
	
	VObjLockPotentiel lock(this, fNeedLock);
	//occupe();
	sous=tmem;
	if (sous==nil)
	{
		sous=CreTreeInMem();
		if (sous==nil)
		{
			err = ThrowBaseError(memfull, DBaction_StoringObjectInCache);
		}
		else
		{
			if (sous->okcree())
			{
				tmem=sous;
				sous->SetParent(this,0);
				err=sous->PutInto(NewMax,n,data, ContientObjetAutre);
				sous->libere();
			}
			else
			{
				sous->libere();
				delete sous;
				err = ThrowBaseError(memfull, DBaction_StoringObjectInCache);
			}
		}
		
	}
	else  // sous is not nil
	{
		if (nbelem<NewMax)
		{
			if ((NewMax==kNbElemTreeMem) || (NewMax==kNbElemTreeMem2))
			{
				sous=CreTreeInMem();
				if (sous==nil)
				{
					err = ThrowBaseError(memfull, DBaction_StoringObjectInCache);
				}
				else
				{
					if (sous->okcree())
					{
						(*sous)[0]=tmem;
						tmem->SetParent(sous,0);
						sous->SetParent(this,0);
						tmem=sous;
						sous->libere();
					}
					else
					{
						sous->libere();
						delete sous;
						err = ThrowBaseError(memfull, DBaction_StoringObjectInCache);
					}
				}
			}
		}
		
		if (err==VE_OK)
		{
			err=sous->PutInto(NewMax,n,data,ContientObjetAutre);
		}
	}
	
	if ( (err==VE_OK) && (NewMax>nbelem) ) nbelem=NewMax;
	
	//libere();
	
	return(err);
}


ObjCacheInTree* TreeInMemHeader::GetFromTreeMem(sLONG n)
{
	ObjCacheInTree* result;
	
	VObjLockPotentiel lock(this, fNeedLock);
	//occupe();
	result=nil;
	if (tmem!=nil)
	{
		result=tmem->GetFrom(nbelem,n);
	}
	//libere();
	return(result);
}


VError TreeInMemHeader::Aggrandit(sLONG newMax)
{
	VError err = VE_OK;
	VObjLockPotentiel lock(this, fNeedLock);
	//occupe();

	sLONG i = nbelem;
	while (newMax > nbelem && err == VE_OK)
	{
		err = PutIntoTreeMem(i+1, i, nil);
		i++;
	}

	//libere();
	return err;
}


VError TreeInMemHeader::AggranditSiNecessaire(sLONG newMax)
{
	VError err = VE_OK;
	VObjLockPotentiel lock(this, fNeedLock);
	//occupe();

	if (tmem != nil)
	{
		sLONG i = nbelem;
		while (newMax > nbelem && err == VE_OK)
		{
			err = PutIntoTreeMem(i+1, i, nil);
			i++;
		}
	}
	else
		nbelem = newMax;

	//libere();
	return err;
}




ObjCacheInTree* TreeInMemHeader::GetFromTreeMemAndOccupe(sLONG n, Boolean wait)
{
	ObjCacheInTree* result;
	Boolean dejaoccupe;

	do 
	{
		dejaoccupe = false;
		occupe();
		result=nil;
		if (tmem!=nil)
		{
			result=tmem->GetFromAndOccupe(nbelem,n, dejaoccupe);
		}
		libere();
		if (dejaoccupe)
			VTask::GetCurrent()->YieldNow();
	}
	while (dejaoccupe && wait);
	if (dejaoccupe)
		result = (ObjCacheInTree*)-1;
	return(result);
}


void* TreeInMemHeader::DelFromTreeMem(sLONG n)
{
	VObjLockPotentiel lock(this, fNeedLock);
	//occupe();
	if (tmem!=nil)
	{
		return tmem->DelFrom(nbelem,n);
	}
	else
		return nil;
	//libere();
}

#pragma segment Main
TreeInMem* TreeInMemHeader::CreTreeInMem()
{
	sWORD _access = ObjCache::TreeInMemAccess;
	if (FeuilleFinaleContientQuoi == t_ficheondisk)
		_access = ObjCache::RecordAccess;
	return new TreeInMem(_access, FeuilleFinaleContientQuoi, fNeedLock);
}





								/* ----------------------------------------------- */


AddrTable::AddrTable(Base4D *xdb, sLONG xprefseg, sWORD DefaultAccess):ObjCacheInTree(DefaultAccess)
{
#if debuglrWithTypObj
	typobj=t_addrtab;
#endif
	db=xdb;
	tab=nil;
	tabmem=nil;
	//original = nil;
	//origaddr = 0;
	prefseg = xprefseg;
}


AddrTable::~AddrTable()
{
	occupe();
	if (tab!=nil) FreeFastMem(tab);
	tab=nil;
	if (tabmem!=nil) FreeFastMem(tabmem);
	tabmem=nil;
	FreeOldOne();
	libere();
}



#if autocheckobj
uBOOL AddrTable::CheckObjInMem(void)
{
	sLONG i;
	
	CheckAssert(ObjCacheInTree::CheckObjInMem());
	
	CheckAssert(IsValidFastPtr(tab));
	CheckAssert(IsValidFastPtr(tabmem));
	
	if (tab != nil)
	{
		for (i=0; i<kNbElemTabAddr; i++)
		{
			CheckAssert(IsValidDiskAddr(tab[i].addr));
		}
	}

	if (tabmem != nil)
	{
		for (i=0; i<kNbElemTabAddr; i++)
		{
			CheckAssert(IsValidFastPtr(tabmem[i]));
		}
	}
	
	return(true);
}
#endif


VError AddrTable::init(void)
{
	VError err = VE_OK;
	
	tab=(AddrDBObj*)GetFastMem(kSizeTabAddr, false, 'tadr');
	if (tab==nil)
	{
		err = ThrowBaseError(memfull, DBaction_BuildingAddrTable);
	}
	if (err==VE_OK)
	{
		_raz(tab,kSizeTabAddr);
	}
	return(err);
}


VError AddrTable::loadobj(DataAddr4D xaddr, sLONG inposdansparent)
{
	VError err = VE_OK;
	
	if (tab==nil)
	{
		if (inposdansparent == -1)
			inposdansparent = posdansparent;
		tab=(AddrDBObj*)GetFastMem(kSizeTabAddr, false, 'tadr');
		xbox_assert(tab != nil);
		if (tab==nil)
		{
			err = ThrowBaseError(memfull, DBaction_LoadingAddrTable);
		}
	
		if (err==VE_OK)
		{
			if (xaddr!=0) setaddr(xaddr);
			DataBaseObjectHeader tag;
			err = tag.ReadFrom(db, getaddr(), nil);
			if (err == VE_OK)
				err = tag.ValidateTag(DBOH_TableAddress, inposdansparent, -1);
			if (err == VE_OK)
				err=db->readlong(tab,kSizeTabAddr,getaddr(),kSizeDataBaseObjectHeader);
			if (err == VE_OK)
				err = tag.ValidateCheckSum(tab, kSizeTabAddr);
			if (err!=VE_OK)
			{
				FreeFastMem(tab);
				tab=nil;
			}
			else
			{
				if (tag.NeedSwap())
				{
					ByteSwapCollection(tab, kNbElemTabAddr);
				}
			}
		}
	}

	if (tabmem==nil && err == VE_OK)
	{
		tabmem=(AddrTable* *)GetFastMem(kSizeTabAddrMem, false, 'tMem');
		if (tabmem==nil)
		{
			err = ThrowBaseError(memfull, DBaction_LoadingAddrTable);
		}
		else
		{
			_raz(tabmem,kSizeTabAddrMem);
		}
	}

	if (err != VE_OK)
		err = ThrowBaseError(VE_DB4D_CANNOT_LOAD_ADDRTABLE, DBaction_LoadingAddrTable);
	
	return(err);
}


sLONG AddrTable::saveobj(void)
{
#if debuglogwrite
	VString wherefrom(L"AddrTable SaveObj");
	VString* whx = &wherefrom;
#else
	VString* whx = nil;
#endif

	VError err;
	
#if 0 && debugindex
	VStr63 s;
	
	if (typobj==t_addrindtable)
	{
		s.FromULong(getaddr(),kHexaDecimal);
		UnivStrDebug("savepage addr : ",s);
		s.FromULong((uLONG)this,kHexaDecimal);
		UnivStrDebug("   inmem = ",s);
	}
#endif

	DataBaseObjectHeader tag(tab,kSizeTabAddr, DBOH_TableAddress, posdansparent, -1);
	err = tag.WriteInto(db, getaddr(), whx);
	if (err == VE_OK)
		err=db->writelong(tab,kSizeTabAddr,getaddr(),kSizeDataBaseObjectHeader, whx);
	
	return(kSizeTabAddr);
}


void AddrTable::DelFromParent(sLONG n, ObjCacheInTree* enfant)
{
	if (tabmem!=nil)
	{
		if (tabmem[n]!=nil)
		{
			xbox_assert(tabmem[n] == enfant);
			InvalidateNbElemTab();
			tabmem[n]=nil;
		}
	}
}


void AddrTable::RecalcNbElemTab()
{
	nbelemtab = 0;
	if (tabmem != nil)
	{
		AddrTable** p = &tabmem[0];
		for (sLONG i = 0; i<kNbElemTabAddr; i++)
		{
			if (*p != nil)
				nbelemtab++;
			p++;
		}
	}
}


sLONG AddrTable::liberemem(sLONG allocationBlockNumber, sLONG combien, uBOOL tout)
{
	sLONG tot;
	
	tot=0;
	if ((GetParent()==nil) || (!GetParent()->occupe()))
	{
		if (GetParent()->GetCountOccupe() == 1)
		{
			if (combien==-1) combien=kMaxPositif;
			
			if (tab!=nil)
			{
				tot=tot+kSizeTabAddr;
				FreeFastMem(tab);
				tab=nil;
			}
			
			if (tabmem!=nil)
			{
				if (GetNbElemTab()==0)
				{
					tot += kSizeTabAddrMem;
					FreeFastMem(tabmem);
					tabmem=nil;
				}
			}
				
			if (okdel())
			{
				if (GetParent()!=nil) 
					GetParent()->DelFromParent(posdansparent, this);
				if (tot==0) tot=1;
			}
		}
		if (GetParent()!=nil) 
			GetParent()->libere();
	}
	
	return(tot);
}



uBOOL AddrTable::okdel(void)
{
	return(((tab==nil) && (tabmem==nil)) || ForceDeleted() );
}


VError AddrTable::loadmem(sLONG n, sLONG n1, sLONG *xn2, sLONG *n3, AddrTable* *psous)
{
	VError err;
	AddrTable* sousADT = nil;
	AddrTable* ceci;
	

	err=loadobj();
	if (tabmem==nil)
	{
		tabmem=(AddrTable* *)GetFastMem(kSizeTabAddrMem, false, 'tMem');
		if (tabmem==nil)
		{
			err = ThrowBaseError(memfull, DBaction_LoadingAddrTable);
		}
		else
		{
			_raz(tabmem,kSizeTabAddrMem);
		}
	}
	
	if (psous!=nil)
	{
		if (err==VE_OK)
		{
			if (*n3>kNbElemTabAddr2)
			{
				n1=n1>>kRatioTabAddr;
				*xn2=n & (kNbElemTabAddr2-1);
				*n3=kNbElemTabAddr2-1;
			}
			else
			{
				*n3=kNbElemTabAddr-1;
			}
			sousADT=tabmem[n1];
			if (sousADT==nil)
			{
				ceci=this;
				//sousADT=new AddrDeFicTable(db);
				sousADT=ceci->NewTable(db);
				if (sousADT==nil)
				{
					err = ThrowBaseError(memfull, DBaction_LoadingAddrTable);
				}
				else
				{
					// sousADT->occupe();
					err=sousADT->loadobj(tab[n1].addr, n1);
					if (err==VE_OK)
					{
						tabmem[n1]=sousADT;
						sousADT->SetParent(this,n1);
						sousADT->libere();
					}
					else
					{
						sousADT->libere();
						delete sousADT;
						sousADT=nil;
					}
				}
			}
		}
		
		*psous=sousADT;
	}
	
	return(err);
}


void AddrTable::LiberePourClose(sLONG nbelem)
{
	sLONG i,n,xn2;
	AddrTable* sous;
	
	occupe();
	
	if (nbelem>kNbElemTabAddr)
	{
		if (nbelem>kNbElemTabAddr2)
		{
			n = (nbelem+kNbElemTabAddr2-1) >> kRatioTabAddr2;
			for (i=0;i<n;i++)
			{
				if (i==(n-1))
				{
					xn2 = nbelem & (kNbElemTabAddr2-1);
				}
				else
				{
					xn2 = kNbElemTabAddr2-1;
				}
				
				sous = (AddrTable*)(tabmem[i]);
				if (sous!=nil)
				{
					sous->LiberePourClose(xn2);
					delete sous;
					tabmem[i] = nil;
				}
			}
		}
		else
		{
			n = (nbelem+kNbElemTabAddr-1) >> kRatioTabAddr;
			for (i=0;i<n;i++)
			{
				if (i==(n-1))
				{
					xn2 = nbelem & (kNbElemTabAddr-1);
				}
				else
				{
					xn2 = kNbElemTabAddr-1;
				}
				
				sous = (AddrTable*)(tabmem[i]);
				if (sous!=nil)
				{
					sous->LiberePourClose(xn2);
					delete sous;
					tabmem[i] = nil;
				}
			}
		}
	}
	else
	{
	}
	
	InvalidateNbElemTab();
	libere();
}


DataAddr4D AddrTable::GetAddrFromTable(sLONG nbelem, sLONG n, VError& err, sLONG* outLen)
{
	DataAddr4D result;
	sLONG n1,xn2;
	AddrTable* sousADT;
	
	result=0;
	occupe();
	// IncNbAcces();
	
	err=loadobj();
	
	if (err==VE_OK)
	{
		n1=n>>kRatioTabAddr;
		xn2=n & (kNbElemTabAddr-1);
		
		if (nbelem>kNbElemTabAddr)
		{
			err=loadmem(n,n1,&xn2,&nbelem,&sousADT);
			if (err==VE_OK)
			{
				result=sousADT->GetAddrFromTable(nbelem, xn2, err, outLen);
			}
		}
		else
		{
			if (err==VE_OK) 
			{
				result=tab[xn2].addr;
				if (outLen != nil)
					*outLen = tab[xn2].len;
			}
			/* ?? setmodif(true); */
		}
	}
	
	libere();
	return(result);
}


VError AddrTable::PutAddrIntoTable(sLONG nbelem, sLONG n, DataAddr4D addr, sLONG len, BaseTaskInfo* context)
{
	sLONG n1,xn2;
	AddrTable* sousADT;
	VError err;

	occupe();
	// IncNbAcces();
	err = loadobj();
			
	if (err==VE_OK)
	{
		n1=n>>kRatioTabAddr;
		xn2=n & (kNbElemTabAddr-1);
		
		if (nbelem>kNbElemTabAddr)
		{
			
			err=loadmem(n,n1,&xn2,&nbelem,&sousADT);
			if (err==VE_OK)
			{
				err=sousADT->PutAddrIntoTable(nbelem,xn2,addr,len, context);
			}
			
		}
		else
		{
			if (err==VE_OK) 
			{
				tab[xn2].addr=addr;
				tab[xn2].len=len;
			}
			setmodif(true, db, context);
		}
	}
	
	libere();

	if (err != VE_OK)
		err = ThrowBaseError(VE_DB4D_CANNOT_MODIFY_ADDRTABLE, DBaction_LoadingAddrTable);
	return(err);
}


VError AddrTable::InitAndSetSize(sLONG maxelem, BaseTaskInfo* context, sLONG inPosDansParent)
{
	VError err = VE_OK;
	sLONG i,nb,last,sousmax, maxdone = -1;
	DataAddr4D ou = db->findplace(kSizeTabAddr+kSizeDataBaseObjectHeader, context, err, prefseg, this);
	if (err == VE_OK)
	{
		xbox_assert(ou > 0);
		setaddr(ou,false);
		
		_raz(tab,kSizeTabAddr);
		if (maxelem > kNbElemTabAddr)
		{
			AddrTable* soust = new AddrTable(db, prefseg);
			if (soust == nil)
				err = ThrowBaseError(memfull, DBaction_BuildingAddrTable);
			else
			{
				soust->init();

				if (maxelem > kNbElemTabAddr2)
				{
					nb = (maxelem+kNbElemTabAddr2-1) / kNbElemTabAddr2;
					last = (maxelem & (kNbElemTabAddr2-1))+1;
					if (last < 1025)
						last = 1025;
					sousmax = kNbElemTabAddr2;
				}
				else
				{
					nb = (maxelem+kNbElemTabAddr-1) / kNbElemTabAddr;
					last = (maxelem & (kNbElemTabAddr-1))+1;
					sousmax = kNbElemTabAddr;
				}

				for (i = 0; i<nb && err == VE_OK; i++)
				{
					if (soust != nil)
					{
						err = soust->InitAndSetSize(i == nb -1 ? last : sousmax, context, i);
						if (err == VE_OK)
						{
							tab[i].addr = soust->getaddr();
							tab[i].len = kSizeTabAddr /*+kSizeDataBaseObjectHeader*/;
							maxdone = i;
						}
					}
					else
						err = ThrowBaseError(memfull, DBaction_BuildingAddrTable);
				}
				soust->libere();
				delete soust;
			}
		}

		if (err == VE_OK)
		{
			DataBaseObjectHeader tag(tab,kSizeTabAddr, DBOH_TableAddress, inPosDansParent, -1);
			err = tag.WriteInto(db, getaddr(), nil);
			if (err == VE_OK)
				err=db->writelong(tab,kSizeTabAddr,getaddr(),kSizeDataBaseObjectHeader, nil);
		}

		if (err != VE_OK)
		{
			if (maxdone != -1)
			{
				if (maxelem > kNbElemTabAddr2)
				{
					AddrTable* soust = new AddrTable(db, prefseg);
					if (soust != nil)
					{
						soust->init();
						for (i=0; i<=maxdone; i++)
						{
							soust->SubLiberePlace(tab[i].addr, context);
						}
						soust->libere();
						delete soust;
					}
				}
				for (i=0; i<=maxdone; i++)
				{
					db->libereplace(tab[i].addr, kSizeTabAddr+kSizeDataBaseObjectHeader, context, this);
				}
			}
			db->libereplace(ou, kSizeTabAddr+kSizeDataBaseObjectHeader, context, this);
		}
	}

	if (err != VE_OK)
		err = ThrowBaseError(VE_DB4D_CANNOT_MODIFY_ADDRTABLE, DBaction_BuildingAddrTable);
	return err;
}


void AddrTable::SubLiberePlace(DataAddr4D ou, BaseTaskInfo* context)
{
	VError err = db->readlong(tab, kSizeTabAddr, ou, kSizeDataBaseObjectHeader);
	if (err == VE_OK)
	{
		sLONG i;
		for (i=0; i<kNbElemTabAddr; i++)
		{
			if (tab[i].addr > 0)
				db->libereplace(tab[i].addr, kSizeDataBaseObjectHeader + kSizeTabAddr, context, this);
		}
	}
}


AddrTable* AddrTable::AddToTable(sLONG nbelem, BaseTaskInfo* context, VError& err, AddrTableHeader *ParentF)
{
	AddrTable *result, *deux, *sous, *ceci;
	sLONG n,xn2;
	DataAddr4D ou;
	uBOOL pasdeux;
	
	occupe();
	// IncNbAcces();

	result=this;
	err=VE_OK;
	deux=nil;
	if (nbelem<0)
	{
		pasdeux=true;
		nbelem=-nbelem;
	}
	else
	{
		pasdeux=false;
	}
	
	if ( (nbelem==kNbElemTabAddr) || (nbelem==kNbElemTabAddr2))
	{
		ceci=this;
		result=ceci->NewTable(db);
		//result=new AddrDeFicTable(db);
		// result->occupe();
		ou=db->findplace(kSizeTabAddr+kSizeDataBaseObjectHeader, context, err, prefseg, result);
	#if debugplace
		shex.FromULong((uLONG)ou,kHexaDecimal);
		UnivStrDebug("FindPlace AddrTab = ",shex);
	#endif
		if (ou>0 && err == VE_OK)
		{
			err=result->init();
			if (err==VE_OK)
			{
				result->setaddr(ou);
				result->setmodif(true, db, context);
				if (pasdeux)
				{
					if (nbelem==kNbElemTabAddr2)
					{
						err=result->loadmem(0,0,nil,nil,nil);
						if (err==VE_OK)
						{
							sous=result->AddToTable(-kNbElemTabAddr, context, err);
							if (sous!=nil)
							{
								result->tabmem[0]=sous;
								sous->SetParent(result,0);
								result->tab[0].addr=sous->getaddr();
								result->tab[0].len=kSizeTabAddr;
								result->setmodif(true, db, context);
							}
						}
					}
				}
				else
				{
					err=result->loadmem(0,0,nil,nil,nil);
					if (err==VE_OK)
					{
						ceci=this;
						deux=ceci->NewTable(db);

						result->tab[0].addr=getaddr();
						result->tab[0].len=kSizeTabAddr;
						result->tabmem[0]=this;
						result->SetParent(ParentF,0);
						SetParent(result,0);
					
						//deux=new AddrDeFicTable(db);
						//deux->occupe();
						ou=db->findplace(kSizeTabAddr+kSizeDataBaseObjectHeader, context, err, prefseg, deux);
					#if debugplace
						shex.FromULong((uLONG)ou,kHexaDecimal);
						UnivStrDebug("FindPlace AddrTab = ",shex);
					#endif
						if (ou>0 && err == VE_OK)
						{
							err=deux->init();
							if (err==VE_OK)
							{
								deux->setaddr(ou);
								/*## ?? err=deux->loadmem(0,0,nil,nil,nil); */
								deux->setmodif(true, db, context);
								if (err==VE_OK)
								{
									result->tab[1].addr=ou;
									result->tab[1].len=kSizeTabAddr;
									result->tabmem[1]=deux;
									deux->SetParent(result,1);
									result->setmodif(true, db, context);
									if (nbelem==kNbElemTabAddr2)
									{
										sous=deux->AddToTable(-kNbElemTabAddr, context, err);
										{
											err=deux->loadmem(0,0,nil,nil,nil);
											if (err == VE_OK)
											{
												deux->tabmem[0]=sous;
												sous->SetParent(deux,0);
												deux->tab[0].addr=sous->getaddr();
												deux->tab[0].len=kSizeTabAddr;
												deux->setmodif(true, db, context);
											}
										}
									}
									
								}
							}
							
						}
						deux->libere();
					}
				}
			}
			if (result!=nil)
			{
				result->InvalidateNbElemTab();
			}
		}
		
		if (result!=nil)
			result->libere();
		if (err!=VE_OK)
		{
			if (result!=nil) delete result;
			result=nil;
			if (deux!=nil) delete deux;
			deux=nil;
		}
	}
	else
	{
		if (nbelem>kNbElemTabAddr2)
		{
			n=nbelem>>(kRatioTabAddr<<1);
			xn2=nbelem & (kNbElemTabAddr2-1);
			
			if ((xn2==0) && (n>1))
			{
				sous=AddToTable(-kNbElemTabAddr2, context, err);
				if (sous!=nil)
				{
					err = loadobj();
					if (err == VE_OK)
					{
						tabmem[n]=sous;
						sous->SetParent(this,n);
						tab[n].addr=sous->getaddr();
						tab[n].len=kSizeTabAddr;
						setmodif(true, db, context);
					}
					else result = nil;
				}
				else result = nil;
			}
			else
			{
				sLONG bidon,bidon2;
				bidon = nbelem;
				bidon2 = xn2;
				
				err=loadmem(nbelem,nbelem>>kRatioTabAddr,&bidon2,&bidon,&deux);
				
				deux->occupe();
			
				if ((xn2 & (kNbElemTabAddr-1)) == 0)
				{
					sLONG xn1 = xn2 >> kRatioTabAddr;
					sous=deux->AddToTable(-kNbElemTabAddr, context, err);
					if (sous!=nil)
					{
						err = loadobj();
						if (err == VE_OK)
						{
							err = deux->loadmem(0,0,nil,nil,nil);
							deux->tabmem[xn1]=sous;
							sous->SetParent(deux,xn1);
							deux->tab[xn1].addr=sous->getaddr();
							deux->tab[xn1].len=kSizeTabAddr;
							deux->setmodif(true, db, context);
						}
						else result = nil;
					}
					else result = nil;
				}	

				deux->libere();
			}

		}
		else
		{
			n=nbelem>>kRatioTabAddr;
			xn2=nbelem & (kNbElemTabAddr-1);
			if ((xn2==0) && (n>0))
			{
				sous=AddToTable(-kNbElemTabAddr, context, err);
				if (sous!=nil)
				{
					err = loadobj();
					if (err == VE_OK)
					{
						err = loadmem(0,0,nil,nil,nil);
						tabmem[n]=sous;
						sous->SetParent(this,n);
						tab[n].addr=sous->getaddr();
						tab[n].len=kSizeTabAddr;
						setmodif(true, db, context);
					}
					else result = nil;
				}
				else
				{
					result=nil;
				}
			}
		}
	}
	
	libere();
	// SetError(err);
	if (err != VE_OK)
		err = ThrowBaseError(VE_DB4D_CANNOT_MODIFY_ADDRTABLE, DBaction_BuildingAddrTable);
	return(result);
}


VError AddrTable::SubLibereEspaceDisk(sLONG nbelem, VDB4DProgressIndicator* InProgress)
{
	sLONG i,n,xn2,oldnbelem;
	AddrTable *soustable;
	VError err = VE_OK;
	
	occupe();

	if (nbelem>kNbElemTabAddr)
	{
		loadobj();
		oldnbelem=nbelem;
		nbelem=(nbelem+kNbElemTabAddr-1)>>kRatioTabAddr;
		if (nbelem>kNbElemTabAddr)
		{
			xn2=oldnbelem & (kNbElemTabAddr2-1);
			nbelem=(nbelem+kNbElemTabAddr-1)>>kRatioTabAddr;
			for (i=0;i<nbelem;i++)
			{
				if (tabmem==nil) soustable=nil; else soustable=tabmem[i];
				if (soustable==nil)
				{
					soustable=new AddrTable(db, prefseg);
					soustable->loadobj(tab[i].addr, i);
				}
				else
					soustable->occupe();
				if (soustable!=nil)
				{
					if (i==(nbelem-1)) n=xn2; else n=kNbElemTabAddr2-1;
					soustable->SubLibereEspaceDisk(n, InProgress);
					soustable->libere();
					delete soustable;
				}
			}
		}
		else
		{
			if (tabmem!=nil)
			{
				for (i=0;i<nbelem;i++)
				{
					soustable=tabmem[i];
					if (soustable != nil)
						soustable->setmodif(false, db, nil);
					db->libereplace(tab[i].addr, kSizeTabAddr+kSizeDataBaseObjectHeader, nil, soustable);
					soustable=tabmem[i]; // volontairement, car peut avoir ete libere par un liberemem
					if (soustable!=nil) 
						delete soustable;
					else
					{
						sLONG xdebug = 1; // put a break here
					}
				#if debugplace
					shex.FromULong((uLONG)(tab[i].addr),kHexaDecimal);
					UnivStrDebug("LiberePlace Tabaddr = ",shex);
				#endif
					vYield();
				}
			}
		}
	}
	
	// vYield();
	setmodif(false, db, nil);
	if (getaddr() > 0)
		db->libereplace(getaddr(), kSizeTabAddr+kSizeDataBaseObjectHeader, nil, this);

	libere();
#if debugplace
	shex.FromULong((uLONG)getaddr(),kHexaDecimal);
	UnivStrDebug("LiberePlace Tabaddr = ",shex);
#endif

	return err;
}


#if 0 & debugindex

void AddrTable::check(sLONG nbelem)
{
	sLONG i,n,xn2,oldnbelem;
	AddrTable *soustable;
	
	if (nbelem>kNbElemTabAddr)
	{
		oldnbelem=nbelem;
		nbelem=(nbelem+kNbElemTabAddr-1)>>kRatioTabAddr;
		if (nbelem>kNbElemTabAddr)
		{
			xn2=oldnbelem & (kNbElemTabAddr2-1);
			nbelem=(nbelem+kNbElemTabAddr-1)>>kRatioTabAddr;
			if (tabmem!=nil)
			{
				for (i=0;i<nbelem;i++)
				{
					soustable=tabmem[i];
					if (soustable!=nil)
					{
						if (i==(nbelem-1)) n=xn2; else n=kNbElemTabAddr2-1;
						soustable->check(n);
					}
				}
			}
		}
		else
		{
			xn2=oldnbelem & (kNbElemTabAddr-1);
			if (tabmem!=nil)
			{
				for (i=0;i<nbelem;i++)
				{
					soustable=tabmem[i];
					if (soustable!=nil)
					{
						if (i==(nbelem-1)) n=xn2; else n=kNbElemTabAddr-1;
						soustable->check(n);
					}
				}
			}
		}
	}
	else
	{
		if (tab!=nil)
		{
			for (i=0;i<(nbelem-1);i++)
			{
				if (tab[i].addr==0)
				{
					vBreak();
					i=i;
				}
			}
			
			if (!modifie())
			{
				db->readlong(&xxbuf,kSizeTabAddr,getaddr(),kSizeDataBaseObjectHeader);
				for (i=0;i<kNbElemTabAddr;i++)
				{
					if (tab[i].addr!=xxbuf[i])
					{
						vBreak();
					}
				}
			}
			
		}
	}
}

#endif


AddrTable* AddrTable::NewTable(Base4D *xdb)
{
	AddrTable* sous;
	
	sous=new AddrTable(xdb, prefseg);
	return(sous);
}


VError AddrTable::CheckTable(CheckAndRepairAgent* inCheck, DataAddr4D adr, sLONG nbelem, sLONG maxelems, DataBaseObjectType WhatType, sLONG PosInParent, sLONG Parent)
{
	VError err = VE_OK;
	sLONG i;
	AddrDBObj xtab[kNbElemTabAddr];

	Base4D* db = inCheck->GetTarget();

	if (! db->ValidAddress(adr,kSizeTabAddr+kSizeDataBaseObjectHeader))
	{
		err = VE_DB4D_ADDRESSISINVALID;
	}
	else
	{
		DataBaseObjectHeader tag;

		err = tag.ReadFrom(db, adr);
		if (err == VE_OK)
		{
			err = tag.ValidateTag(WhatType, PosInParent, Parent);
			if (err == VE_OK)
			{
				err = db->readlong(xtab, kSizeTabAddr, adr, kSizeDataBaseObjectHeader);
				
				err = tag.ValidateCheckSum(xtab, kSizeTabAddr);
				if (err == VE_OK)
				{
					if (nbelem < kNbElemTabAddr)
					{
						for (i = 0; i < nbelem; i++)
						{
							DataAddr4D xadr = xtab[i].addr;
							sLONG len = xtab[i].len;
							if (xadr != kFinDesTrou)
							{
								if (xadr<0)
								{
									xadr = -xadr;
									if (xadr > (DataAddr4D)maxelems)
									{
										err = VE_DB4D_ADDRESSISINVALID;
									}
								}
								else
								{
									if (! db->ValidAddress(xadr,len))
									{
										err = VE_DB4D_ADDRESSISINVALID;
									}
								}
							}
						}
					}
					else
					{

						sLONG nbsouspage;
						sLONG i,nrest;

						if (nbelem <kNbElemTabAddr2)
						{
							nbsouspage = (nbelem+kNbElemTabAddr-1)>>kRatioTabAddr;
							nrest = nbelem & (kNbElemTabAddr-1);

						}
						else
						{
							nbsouspage = (nbelem+kNbElemTabAddr2-1)>>kRatioTabAddr2;
							nrest = ((nbelem+kNbElemTabAddr-1)>>kRatioTabAddr) & (kNbElemTabAddr-1);
						}

						for (i = 0; i < nbsouspage; i++)
						{
							AddrTable* soustable;
							sLONG n;

							if (i == (nbsouspage-1)) n = nrest; else n = kNbElemTabAddr - 1;

							err = CheckTable(inCheck, xtab[i].addr, n, maxelems, WhatType, i, -1);
						}
					}
				}
			}
		}
	}

	return err;
}



#if debugOverlaps_strong

Boolean AddrTable::Debug_CheckAddrOverlap(sLONG nbelem, sLONG curelem, DataAddr4D addrToCheck, sLONG lenToCheck, sLONG nomobjToCheck)
{
	Boolean result = false;
	if (IsOkToOccupe())
	{
		sLONG n, xn2, i;
		AddrTable* sous;

		if (nbelem>kNbElemTabAddr)
		{
			if (nbelem>kNbElemTabAddr2)
			{
				n = (nbelem+kNbElemTabAddr2-1) >> kRatioTabAddr2;
				for (i=0;i<n;i++)
				{
					if (i==(n-1))
					{
						xn2 = nbelem & (kNbElemTabAddr2-1);
					}
					else
					{
						xn2 = kNbElemTabAddr2-1;
					}

					sous = (AddrTable*)(tabmem[i]);
					if (sous!=nil)
					{
						result = result || sous->Debug_CheckAddrOverlap(xn2, curelem, addrToCheck, lenToCheck, nomobjToCheck);
					}
					curelem = curelem + kNbElemTabAddr2;
				}
			}
			else
			{
				n = (nbelem+kNbElemTabAddr-1) >> kRatioTabAddr;
				for (i=0;i<n;i++)
				{
					if (i==(n-1))
					{
						xn2 = nbelem & (kNbElemTabAddr-1);
					}
					else
					{
						xn2 = kNbElemTabAddr-1;
					}

					sous = (AddrTable*)(tabmem[i]);
					if (sous!=nil)
					{
						result = result || sous->Debug_CheckAddrOverlap(xn2, curelem, addrToCheck, lenToCheck, nomobjToCheck);
					}
					curelem = curelem + kNbElemTabAddr;
				}
			}
		}
		else
		{
			for (i = 0; i <=nbelem; i++)
			{
				DataAddr4D addrobj = tab[i].addr;
				sLONG lenobj = tab[i].len;
				if ( (addrobj >= addrToCheck && addrobj < (addrToCheck+lenToCheck)) 
					|| (addrToCheck >= addrobj && addrToCheck < (addrobj+lenobj)) )
				{
					if (curelem != nomobjToCheck)
					{
						lenobj = lenobj; // put a break here;
						xbox_assert(! ((addrobj >= addrToCheck && addrobj < (addrToCheck+lenToCheck)) || (addrToCheck >= addrobj && addrToCheck < (addrobj+lenobj))) );
					}
				}
				curelem++;
			}

		}

		libere();
	}
	return result;
}

#endif





								/* ----------------------------------------------- */




AddrTableHeader::AddrTableHeader()
{
	++UniqTableHeader;
	if (UniqTableHeader>2000000000)
		UniqTableHeader = 1;
	fIDUniq = UniqTableHeader;
	FirstPage=nil;
	db=nil;
	AddrFirstPage=nil;
	owner=nil;
	DebutTrou=nil;
	nbelem=nil;
	//original = nil;
	prefseg = 0;
}


AddrTableHeader::~AddrTableHeader()
{
	LibereEspaceMem();
	/*
	if (FirstPage != nil)
	{
		FirstPage->LiberePourClose(*nbelem);
		delete FirstPage;
	}
	if (original != nil)
	{
		delete original;
	}
	*/
}


void AddrTableHeader::LibereEspaceMem()
{
	if (FirstPage != nil)
	{
		FirstPage->LiberePourClose(*nbelem);
		AddrTable* oldfirst = FirstPage;
		FirstPage->SetParent(nil, 0);
		FirstPage = nil;
		delete oldfirst;
	}
	/*
	if (original != nil)
	{
		delete original;
		original = nil;
	}
	*/
}



#if autocheckobj
uBOOL AddrTableHeader::CheckObjInMem(void)
{
	sLONG xx;
	
	CheckAssert(ObjCacheInTree::CheckObjInMem());
	CheckAssert(IsValidFastPtr(FirstPage));
	
	if (AddrFirstPage != nil) xx = *AddrFirstPage;
	if (DebutTrou != nil) xx = *DebutTrou;
	if (nbelem != nil) xx = *nbelem;
	
	return(true);
}
#endif


sLONG AddrTableHeader::saveobj(void)
{

	owner->setmodif(true, db, nil);
	return(1); // nb octets sauves (doit etre != 0)
}


void AddrTableHeader::Init(Base4D *xdb, ObjAlmostInCache *TheOwner, DataAddr4D* AddrOfFirst, DataAddr4D* AddrDebutTrou, sLONG *AddrNBelem,
							 AddrTableHeaderReLoaderProc* Loader, AddrTableHeaderReLoaderForKillProc* LoaderForKill, sLONG xprefseg)
{
	db=xdb;
	AddrFirstPage=AddrOfFirst;
	owner=TheOwner;
	DebutTrou=AddrDebutTrou;
	nbelem=AddrNBelem;
	/*
	reloader = Loader;
	reloaderForKill = LoaderForKill;
	*/
	prefseg = xprefseg;
}


void AddrTableHeader::setmodif(uBOOL xismodif, Base4D* bd, BaseTaskInfo* context)
{
	owner->setmodif(xismodif,bd,context);
}


AddrTable* AddrTableHeader::NewTable(Base4D *xdb)
{
	AddrTable* sous;
	
	sous=new AddrTable(xdb, prefseg);
	return(sous);
}


void AddrTableHeader::DelFromParent(sLONG n, ObjCacheInTree* enfant)
{
	occupe();
	xbox_assert(FirstPage == enfant);
	FirstPage=nil;
	InvalidateNbElemTab();
	libere();
}


void AddrTableHeader::RecalcNbElemTab()
{
	if (FirstPage == nil)
		nbelemtab = 0;
	else
		nbelemtab = 1;
}


VError AddrTableHeader::ChargeTabAddr(BaseTaskInfo* context)
{
	VError err = VE_OK;
	occupe();
	
	if (FirstPage==nil)
	{
		FirstPage=NewTable(db);
		if (FirstPage == nil)
			err = ThrowBaseError(memfull, DBaction_LoadingAddrTable);
		else
		{
			FirstPage->setaddr(*AddrFirstPage);
			FirstPage->SetParent(this,0);
		}
	}
	else FirstPage->occupe();
	
	libere();
	return err;
}


sLONG AddrTableHeader::findtrou(BaseTaskInfo* context, VError& err)
{
	sLONG result;
	DataAddr4D ou;
	err = VE_OK;
	
	occupe();
	if (*DebutTrou==kFinDesTrou)
	{
		result=-1;
	}
	else
	{
		result=-*DebutTrou;
		err = ChargeTabAddr(context);
		ou=FirstPage->GetAddrFromTable(*nbelem, result, err);
		FirstPage->libere();
		
		if (err == VE_OK)
		{
			*DebutTrou = ou;
			setmodif(true, db, context);
		}
		else
			result = -1;
	}
	
	libere();

	if (err != VE_OK)
		err = ThrowBaseError(VE_DB4D_CANNOT_ALLOCATE_NEW_ENTRY, DBaction_ModifyingAddrTable);
	return(result);
}


VError AddrTableHeader::liberetrou(sLONG n, BaseTaskInfo* context)
{
	VError err;
	DataAddr4D ou;
	
	occupe();
	err = ChargeTabAddr(context);
	if (err == VE_OK)
	{
		//if (FirstPage->GetAddrFromTable(*nbelem, n, err) >= 0 && err == VE_OK)
		{
			ou = *DebutTrou;
			err=FirstPage->PutAddrIntoTable(*nbelem,n,ou,-1,context);
			*DebutTrou = -n;
			setmodif(true,db, context);
		}
		FirstPage->libere();
	}
	libere();

	if (err != VE_OK)
		err = ThrowBaseError(VE_DB4D_CANNOT_FREE_ENTRY, DBaction_ModifyingAddrTable);

	return err;
}


VError AddrTableHeader::TakeAddrTemporary(sLONG n, BaseTaskInfo* context, sLONG* debuttroutrans)
{
	occupe();

#if debuglr
	xbox_assert (xTempAddr.find(n) == xTempAddr.end());
	xTempAddr.insert(n);
#endif

	xbox_assert(*debuttroutrans < 0);
	VError err = VE_OK;
	sLONG n2 = *debuttroutrans;
	if (n2 != kFinDesTrou)
	{
		n2 = n2 + 1;
		sLONG nprec = -n2;
		sLONG lenprec;
		DataAddr4D ou = GetxAddr(nprec, context, err, &lenprec);
		xbox_assert(lenprec == kDebutDesTrou);
		err = PutxAddr(nprec, ou, -n, context);

	}
	if (err == VE_OK)
	{
		err = PutxAddr(n, n2, kDebutDesTrou, context);
	}
	if (err == VE_OK)
		*debuttroutrans = -n - 1;
	xbox_assert(*debuttroutrans < 0);
	libere();

	return err;
}


VError AddrTableHeader::GiveBackAddrTemporary(sLONG n, BaseTaskInfo* context, sLONG* debuttroutrans)
{
	occupe();
	xbox_assert(*debuttroutrans < 0);

#if debuglr
	xbox_assert (xTempAddr.find(n) != xTempAddr.end());
	xTempAddr.erase(n);
#endif

	VError err = VE_OK;
	sLONG precedant;
	DataAddr4D suivant = GetxAddr(n, context, err, &precedant);
	if (err == VE_OK && suivant<=0 && precedant <= 0) // il se peut que l'addresse temporaire ait deja ete rendue
	{
		sLONG nsuivant = (sLONG)suivant;
		sLONG lendontuse;

		if (nsuivant != kFinDesTrou)
		{
			DataAddr4D oudontuse = GetxAddr(-nsuivant, context, err, &lendontuse);
			xbox_assert(lendontuse == -n);
			err = PutxAddr(-nsuivant, oudontuse, precedant, context);
		}

		if (precedant == kDebutDesTrou)
		{
			xbox_assert( *debuttroutrans == -n - 1);
			if (nsuivant == kFinDesTrou)
				*debuttroutrans = kFinDesTrou;
			else
				*debuttroutrans = nsuivant - 1;
		}
		else
		{
			sLONG anteprecedant;
			DataAddr4D ouprecedant = GetxAddr(-precedant, context, err, &anteprecedant);
			xbox_assert ((sLONG)-ouprecedant == n);
			sLONG n2;
			if (nsuivant == kFinDesTrou)
				n2 = kFinDesTrou;
			else
				n2 = nsuivant;
			err = PutxAddr(-precedant, n2, anteprecedant, context);
		}
				
	}

	xbox_assert(*debuttroutrans < 0);

#if debuglr
	if (*debuttroutrans == kFinDesTrou)
		xbox_assert(xTempAddr.size() == 0);
#endif

	libere();

	return err;
}


VError AddrTableHeader::InitAndSetSize(sLONG maxelem, BaseTaskInfo* context)
{
	VError err = VE_OK;
	if (testAssert(*nbelem == 0))
	{
		if (maxelem > 0)
		{
			AddrTable* soust = new AddrTable(db, prefseg);
			if (soust != nil)
			{
				soust->init();
				err = soust->InitAndSetSize(maxelem, context, 0);
				*AddrFirstPage = soust->getaddr();
				soust->libere();
				delete soust;
				if (err == VE_OK)
					*nbelem = maxelem;
			}
		}
	}
	else
		err = ThrowBaseError(VE_DB4D_NOTIMPLEMENTED, DBaction_ModifyingAddrTable);

	if (err != VE_OK)
		err = ThrowBaseError(VE_DB4D_CANNOT_MODIFY_ADDRTABLE, DBaction_ModifyingAddrTable);

	return err;
}


VError AddrTableHeader::Normalize(BaseTaskInfo* context)
{
	sLONG debuttrou = kFinDesTrou;
	sLONG i,nb = *nbelem;
	VError err = VE_OK;

	for (i = 0; i < nb && err == VE_OK; i++)
	{
		DataAddr4D ou = GetxAddr(i, context, err);
		if (err == VE_OK)
		{
			if (ou <= 0)
			{
				err = PutxAddr(i, debuttrou, -1, context);
				if (err == VE_OK)
				{
					debuttrou = -i;
				}
			}
		}
	}

	*DebutTrou = debuttrou;
	setmodif(true, db, context);

	if (err != VE_OK)
		err = ThrowBaseError(VE_DB4D_CANNOT_MODIFY_ADDRTABLE, DBaction_ModifyingAddrTable);

	return err;
}


VError AddrTableHeader::SetDebutDesTrous(sLONG val, BaseTaskInfo* context)
{
	occupe();
	*DebutTrou = val;
	setmodif(true, db, context);
	libere();

	return VE_OK;
}


VError AddrTableHeader::LibereEspaceDisk(OccupableStack* curstack, VDB4DProgressIndicator* InProgress)
{
	occupe();
	VError err = VE_OK;
	if (InProgress != nil)
	{
		XBOX::VString session_title;
		gs(1005,21,session_title);	//  Deleting Address Table: %curValue of %maxValue Entries
		InProgress->BeginSession(*nbelem,session_title);
	}

	err = ChargeTabAddr(nil);
#if debugObjInTree_Strong
	ObjCacheInTree::Debug_CheckParentEnfantsRelations();
#endif
	if (FirstPage != nil && err == VE_OK)
	{
		err = FirstPage->SubLibereEspaceDisk(*nbelem, InProgress);
		FirstPage->libere();
		AddrTable* oldfirst = FirstPage;
		FirstPage->SetParent(nil, 0);
		FirstPage = nil;
		delete oldfirst;
	}

	if (err == VE_OK)
	{
		*AddrFirstPage = 0;
		*DebutTrou = kFinDesTrou;
		*nbelem = 0;
	}

	if (InProgress != nil)
	{
		InProgress->EndSession();
	}

	LibereEspaceMem();

	libere();

	return err;
}


VError AddrTableHeader::TakeOffBits(Bittab *tb, BaseTaskInfo* context)
{
	VError err;
	DataAddr4D ou;
	Bittab dejapass;
	
	err=0;
	occupe();
	
	ou=*DebutTrou;
	// ClearError();
	while ((ou!=kFinDesTrou) && (err==VE_OK))
	{
		ou = -ou;
		if (dejapass.isOn(ou))
			err = ThrowBaseError(VE_DB4D_CIRCULAR_REF_IN_REC_ADDR_TABLE);
		else
		{
			err = dejapass.Set(ou);
			if (err == VE_OK)
			{
				tb->ClearOrSet(ou,false);
				err = ChargeTabAddr(context);
				if (err == VE_OK)
				{
					ou = FirstPage->GetAddrFromTable(*nbelem,ou, err);
					FirstPage->libere();
				}
			}
		}
	}
	
	libere();
	return(err);
}


DataAddr4D AddrTableHeader::GetxAddr(sLONG n, BaseTaskInfo* context, VError& err, sLONG* outLen)
{
	DataAddr4D ou = -1;
	
	err = VE_OK;

	occupe();
	err = ChargeTabAddr(context);
	if (err == VE_OK)
	{
		ou=FirstPage->GetxOldOne(context)->GetAddrFromTable(*nbelem,n, err, outLen);
		FirstPage->libere();
	}
	libere();
	
	xbox_assert(ou < 0 || db->IsAddrValid(ou));

	return(ou);
}


VError AddrTableHeader::PutxAddr(sLONG n, DataAddr4D ValAddr, sLONG len, BaseTaskInfo* context )
{
	AddrTable* sousADT;
	VError err = VE_OK;

	xbox_assert(ValAddr <= 0 || db->IsAddrValid(ValAddr));		// mbucatariu, vu avec L.E., pour le compactage, on copie des trous, context = NULL
																// et la les addresses ne sont pas des adresses valides																 
	occupe();
	
#if debuglr == 1027
	if (ValAddr>0 && db->GetStructure() != nil)
	{
		VString sdebugtaskid = L"Task ID "+ToString(VTask::GetCurrentID())+ L"  :  ";
		DebugMsg(sdebugtaskid + L"Enter PutxAddr "+ToString(ValAddr)+L"  at "+ToString(n)+L"  ,  nbelem = "+ToString(*nbelem)+L"\n\n");
	}
#endif

	if (*nbelem==0)
	{
		if (AccesModifOK(context))
		{
			sousADT=NewTable(db);
			FirstPage=sousADT->AddToTable(-kNbElemTabAddr, context, err);
			sousADT->libere();
			delete sousADT;
			if (err == VE_OK)
			{
				xbox_assert(FirstPage != nil);
				FirstPage->occupe();
				FirstPage->SetParent(this,0);
				*AddrFirstPage=FirstPage->getaddr();
				setmodif(true,db,context);
			}
		}
		else 
			err = VE_DB4D_TRANSACTIONCONFLICT;
	}
	else
	{
		err = ChargeTabAddr(context);
	}

	if ((err==0) && (n>=*nbelem))
	{
		if (AccesModifOK(context))
		{
			if ( ((((*nbelem)/*+1*/) & (kNbElemTabAddr-1)) == 0) && (*nbelem != 0) )
			{
				sousADT=FirstPage->AddToTable((*nbelem)/*+1*/, context, err, this);
				if (err == VE_OK)
				{
					xbox_assert(sousADT != nil);
					FirstPage->libere();
					FirstPage=sousADT;
					FirstPage->occupe();
					*AddrFirstPage=FirstPage->getaddr();
					*nbelem=*nbelem+1;
					setmodif(true,db,context);

#if debugObjInTree_Strong
					ObjCacheInTree::Debug_CheckParentEnfantsRelations();
#endif
				}
			}
			else
			{
				*nbelem=*nbelem+1;
				setmodif(true,db,context);
			}
		}
		else 
			err = ThrowBaseError(VE_DB4D_TRANSACTIONCONFLICT, DBaction_ModifyingAddrTable);

	}
	
	if (err==0)
	{
		err=FirstPage->GetxOldOne(context)->PutAddrIntoTable(*nbelem,n,ValAddr,len,context);
	}
	
	FirstPage->libere();

#if debuglr == 1027
	if (ValAddr>0 && db->GetStructure() != nil)
	{
		VString sdebugtaskid = L"Task ID "+ToString(VTask::GetCurrentID())+ L"  :  ";
		DebugMsg(sdebugtaskid + L"Exit PutxAddr "+ToString(ValAddr)+L"  at "+ToString(n)+L"  ,  nbelem = "+ToString(*nbelem)+L"\n\n");
	}
#endif

	libere();
	return(err);
}


VError AddrTableHeader::CheckAddresses(CheckAndRepairAgent* inCheck)
{
	VError err = VE_OK;

	err = AddrTable::CheckTable(inCheck, *AddrFirstPage, *nbelem, *nbelem, DBOH_TableAddress, 0, -1);

	return err;
}


VError AddrTableHeader::TakeAddr(sLONG n, BaseTaskInfo* context)
{
	VError err = VE_OK;

	occupe();

	if (n >= *nbelem)
	{
		sLONG curtrou = *nbelem;
		while (n > *nbelem && err == VE_OK)
		{
			DataAddr4D ou = *DebutTrou;
			err = PutxAddr(curtrou, ou, -1, context);
			if (err == VE_OK)
				*DebutTrou = -curtrou;
			curtrou++;
		}
		if (err == VE_OK)
		{
			err = PutxAddr(curtrou, 0, 0, context);
		}
	}
	else
	{
		Boolean found = false;
		sLONG ou = *DebutTrou;
		sLONG previous = -1;
		// ClearError();
		while ((ou != kFinDesTrou) && (err==VE_OK))
		{
			if ((-ou) == n)
			{
				DataAddr4D ou2 = GetxAddr(n, context, err);
				if (testAssert(ou2 <= 0))
				{
					ou = (sLONG)ou2;
					if (err == VE_OK)
					{
						if (previous == -1)
							*DebutTrou = ou;
						else
							err = PutxAddr(previous, ou, -1, context);
					}
				}
				else
					err = ThrowBaseError(VE_DB4D_ADDR_ENTRY_IS_NOT_EMPTY, DBactionFinale);

				if (err == VE_OK)
				{
					found = true;
					break;
				}

			}
			else
			{
				ou = -ou;
				previous = ou;
				DataAddr4D ou2 = GetxAddr(ou, context, err);
				xbox_assert(ou2 <= 0);
				ou = (sLONG)ou2;
			}
		}

		if (!found)
			err = ThrowBaseError(VE_DB4D_ADDR_ENTRY_IS_NOT_EMPTY, DBactionFinale);
	}

	libere();

	if (err != VE_OK)
		err = ThrowBaseError(VE_DB4D_CANNOT_PREALLOCATE_ADDR, DBactionFinale);

#if debuglr == 214
	{
		sLONG ou = *DebutTrou;
		sLONG previous = -1;
		while ((ou != kFinDesTrou))
		{
			ou = -ou;
			xbox_assert(ou != n);
			if (ou == n)
			{
				ou = ou; // break here
			}
			DataAddr4D ou2 = GetxAddr(ou, context, err);
			if (testAssert(ou2 <= 0))
			{
			}
			else
			{
				ou = ou; // break here
			}
			ou = (sLONG)ou2;
		}
	}
#endif

	return err;
}



#if debugOverlaps_strong

Boolean AddrTableHeader::Debug_CheckAddrOverlap(DataAddr4D addrToCheck, sLONG lenToCheck, sLONG nomobjToCheck)
{
	Boolean result = false;
	if (IsOkToOccupe())
	{
		if (FirstPage != nil)
		{
			result = FirstPage->Debug_CheckAddrOverlap(*nbelem, 0, addrToCheck, lenToCheck, nomobjToCheck);
		}
		libere();
	}
	return result;
}

#endif




								/* ----------------------------------------------- */


AddrTableHeader_NotOpened::AddrTableHeader_NotOpened(Base4D_NotOpened* xdb)
{
	db = xdb;
	nbelem = -1;
	AddrFirstPage = -1;
	DebutTrou = 0;
}


								/* ----------------------------------------------- */

#if 0

AddrTableHeader2::AddrTableHeader2(AddrTableHeader* from)
{
	AddrTableHeader2* xfrom = (AddrTableHeader2*)from;
	
	FirstPage=xfrom->FirstPage;
	db=xfrom->db;
	AddrFirstPage=&dup_AddrFirstPage;
	owner=xfrom->owner;
	DebutTrou=&dup_DebutTrou;
	nbelem=&dup_nbelem;
	/*
	reloader = nil;
	reloaderForKill = nil;
	original = nil;
	*/
}

#endif



								/* ----------------------------------------------- */
		

ExtraElement::ExtraElement(ExtraElementTable* inOwner, DataAddr4D xaddr, sLONG len, sLONG pos)
{
	fOwner = inOwner;
	fPos = pos;
	fLenOnDisk = len;
	if (xaddr > 0)
	{
		setaddr(xaddr);
	}
}


bool ExtraElement::SaveObj(VSize& outSizeSaved)
{
	if (fMutex.TryToLock())
	{
#if debuglogwrite
		VString wherefrom(L"ExtraElement SaveObj");
		VString* whx = &wherefrom;
#else
		VString* whx = nil;
#endif
		Base4D* db = GetDB();
		DataBaseObjectHeader tag(fDataOnDisk.GetDataPtr(), fLenOnDisk, DBOH_ExtraElement, fPos, -1);
		VError err = tag.WriteInto(db, getaddr(), whx);
		if (err == VE_OK)
			err = db->writelong(fDataOnDisk.GetDataPtr(), fLenOnDisk ,getaddr(), kSizeDataBaseObjectHeader, whx);

		fMutex.Unlock();
		return true;
	}
	else
		return false;
}


VError ExtraElement::load(bool* needSwap)
{
	VError err = VE_OK;

	Base4D* db = GetDB();
	DataAddr4D xaddr = getaddr();
	if (xaddr > 0 && fLenOnDisk)
	{
		if (fData.SetSize(fLenOnDisk))
		{
			DataBaseObjectHeader tag;
			err = tag.ReadFrom(db, xaddr, nil);
			if (err == VE_OK)
				err = tag.ValidateTag(DBOH_ExtraElement, fPos, -1);
			if (err == VE_OK)
				err = db->readlong(fData.GetDataPtr(), fLenOnDisk, xaddr, kSizeDataBaseObjectHeader);

			if (err == VE_OK)
				err = tag.ValidateCheckSum(fData.GetDataPtr(), fLenOnDisk);

			if (needSwap != nil)
				*needSwap = tag.NeedSwap();
		}
		else
			err = ThrowBaseError(VE_MEMORY_FULL);
		
	}

	return err;
}

VError ExtraElement::save(OccupableStack* curstack)
{
	VError err = VE_OK;
	DataAddr4D xaddr = getaddr();
	DataAddr4D oldaddr = xaddr;
	sLONG newlen = (sLONG)fData.GetDataSize();
	sLONG oldlen = fLenOnDisk;
	Base4D* db = GetDB();

	if (db->IsWakandaRunningMode())
	{
		if (oldaddr <= 0 || adjuste(newlen+kSizeDataBaseObjectHeader) != adjuste(fLenOnDisk+kSizeDataBaseObjectHeader))
		{
			xaddr = db->findplace(newlen+kSizeDataBaseObjectHeader, nil, err);
			if (xaddr >= 0)
			{
				{
					VTaskLock lock(&fMutex);
					VSize allocSize = fData.GetAllocatedSize();
					VSize size = fData.GetDataSize();
					void* dataptr = fData.GetDataPtr();
					fDataOnDisk.SetDataPtr(dataptr, size, allocSize);
					fData.ForgetData();
					fLenOnDisk = newlen;

					if (oldaddr <= 0)
					{
						setaddr(xaddr);
						setmodif(true, db, nil);
					}
					else
						ChangeAddr(xaddr, db, nil);
				}
				err = fOwner->SetElementAddress(fPos, xaddr, newlen, curstack);
				if (oldaddr > 0)
					db->libereplace(oldaddr, oldlen+kSizeDataBaseObjectHeader, nil);
			}
		}
		else
		{
			VTaskLock lock(&fMutex);
			VSize allocSize = fData.GetAllocatedSize();
			VSize size = fData.GetDataSize();
			void* dataptr = fData.GetDataPtr();
			fDataOnDisk.SetDataPtr(dataptr, size, allocSize);
			fData.ForgetData();
			fLenOnDisk = newlen;
			setmodif(true, db, nil);
		}
	}

	return err;
}




								/* ----------------------------------------------- */


ExtraElementTable::ExtraElementTable(Base4D* inOwner, DataAddr4D* inAddrPtr, sLONG* inNbElemPtr)
{
	fAddrPtr = inAddrPtr;
	fNbElemPtr = inNbElemPtr;
	fOwner = inOwner;
	AddrDBObj empty;
	empty.addr = 0;
	empty.len = 0;
	fNbElemsOnDisk = *fNbElemPtr;
	std::fill(&fAddrs[0], &fAddrs[kMaxNbExtraElements], empty);
	std::fill(&fElems[0], &fElems[kMaxNbExtraElements], (ExtraElementPtr)nil);
}


ExtraElementTable::~ExtraElementTable()
{
	for (ExtraElement **cur = &fElems[0], **end = &fElems[kMaxNbExtraElements]; cur != end; ++cur)
	{
		QuickReleaseRefCountable(*cur);
	}
}


bool ExtraElementTable::SaveObj(VSize& outSizeSaved)
{
	sLONG len = fNbElemsOnDisk * sizeof(AddrDBObj);
#if debuglogwrite
	VString wherefrom(L"ExtraElementTable SaveObj");
	VString* whx = &wherefrom;
#else
	VString* whx = nil;
#endif

	DataBaseObjectHeader tag(&fAddrs[0], len, DBOH_TableExtraElements, -1, -1);
	VError err = tag.WriteInto(fOwner, getaddr(), whx);
	if (err == VE_OK)
		err = fOwner->writelong(&fAddrs[0], len ,getaddr(), kSizeDataBaseObjectHeader, whx);

	return true;
}


VError ExtraElementTable::load()
{
	VError err = VE_OK;

	if (fOwner->IsWakandaRunningMode())
	{
		if ((*fAddrPtr != 0) && (fNbElemsOnDisk != 0))
		{
			bool canCheckSum = true;
			DataAddr4D xaddr = *fAddrPtr;
			sLONG nbelems = fNbElemsOnDisk;
			if (nbelems > kMaxNbExtraElements) // if we read a database written by a newer version of DB4D
			{
				nbelems = kMaxNbExtraElements;
				canCheckSum = false;
			}
			sLONG len = nbelems * sizeof(AddrDBObj);

			DataBaseObjectHeader tag;
			err = tag.ReadFrom(fOwner, xaddr, nil);
			if (err == VE_OK)
				err = tag.ValidateTag(DBOH_TableExtraElements, -1, -1);
			if (err == VE_OK)
				err=fOwner->readlong(&fAddrs[0], len, xaddr, kSizeDataBaseObjectHeader);

			if (err == VE_OK && canCheckSum)
				err = tag.ValidateCheckSum(&fAddrs[0], len);
			if (err == VE_OK)
			{
				if (tag.NeedSwap())
				{
					ByteSwapCollection(&fAddrs[0], nbelems);
				}
			}


			if (err == VE_OK && nbelems < kMaxNbExtraElements && !fOwner->IsWriteProtected()) // if we read a database written by an older version of DB4D
			{
				sLONG newlen = kMaxNbExtraElements * sizeof(AddrDBObj);
				if (adjuste(newlen+kSizeDataBaseObjectHeader) != adjuste(len+kSizeDataBaseObjectHeader))
				{
					DataAddr4D newaddr = fOwner->findplace(newlen+kSizeDataBaseObjectHeader, nil, err);
					if (newaddr != -1)
					{
						err = fOwner->libereplace(xaddr, len+kSizeDataBaseObjectHeader, nil);
						*fAddrPtr = newaddr;
						xaddr = newaddr;
						*fNbElemPtr = kMaxNbExtraElements;
						nbelems = kMaxNbExtraElements;
						fOwner->setmodif(true, fOwner, nil);
						ChangeAddr(xaddr, fOwner, nil);
						/*
						setmodif(true, fOwner, nil);
						*/

					}
				}
				else
				{
					*fNbElemPtr = kMaxNbExtraElements;
					nbelems = kMaxNbExtraElements;
					fOwner->setmodif(true, fOwner, nil);
					setmodif(true, fOwner, nil);
				}
			}

			fNbElemsOnDisk = nbelems;
			setaddr(xaddr);
		}
		else
		{
			if (!fOwner->IsWriteProtected())
			{
				sLONG newlen = kMaxNbExtraElements * sizeof(AddrDBObj);
				DataAddr4D newaddr = fOwner->findplace(newlen+kSizeDataBaseObjectHeader, nil, err);
				if (newaddr != -1)
				{
					*fAddrPtr = newaddr;
					*fNbElemPtr = kMaxNbExtraElements;
					fNbElemsOnDisk = kMaxNbExtraElements;
					setaddr(newaddr);
					fOwner->setmodif(true, fOwner, nil);
					setmodif(true, fOwner, nil);
				}
			}
		}
	}

	return err;
}


VError ExtraElementTable::save()
{
	VError err = VE_OK;
	if (fOwner->IsWriteProtected())
		err = fOwner->ThrowError(VE_DB4D_DATABASEISWRITEPROTECTED, noaction);
	else
		setmodif(true, fOwner, nil);

	return VE_OK;
}


ExtraElement* ExtraElementTable::RetainElement(sLONG n, OccupableStack* curstack, VError& err, bool* outNeedSwap)
{
	err = VE_OK;
	ExtraElement* result = nil;
	if (n < kMaxNbExtraElements)
	{
		Occupy(curstack, true);
		result = fElems[n];
		if (result == nil)
		{
			result = new ExtraElement(this, fAddrs[n].addr, fAddrs[n].len, n);
			err = result->load(outNeedSwap);
            fElems[n] = result;
		}
		result->Retain();
		Free(curstack, true);
	}
	else
	{
		err = ThrowBaseError(VE_UNIMPLEMENTED);
	}
	return(result);

}

VError ExtraElementTable::SetElementAddress(sLONG n, DataAddr4D addr, sLONG len, OccupableStack* curstack)
{
	VError err = VE_OK;
	if (n < kMaxNbExtraElements)
	{
		Occupy(curstack, true);
		fAddrs[n].addr = addr;
		fAddrs[n].len = len;
		setmodif(true, fOwner, nil);
		Free(curstack, true);
	}
	else
	{
		err = ThrowBaseError(VE_UNIMPLEMENTED);
	}
	return err;
}


