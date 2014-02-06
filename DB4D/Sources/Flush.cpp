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


#if VERSIONDEBUG
uBOOL __okdeletesousbt = 0;
uBOOL fJFlush = 0;
#endif

#if debuglr
uBOOL debug_tools_started = 0;
#endif

sLONG sGlobalNbFlushObjects = 0;

VCriticalSection gModifyInFlushEncoursMutex;

vector<BtreeFlush*> BtreeFlush::fPreAllocatedStack;
VCriticalSection BtreeFlush::fAllocateMutex;

vector<BtreeFlush*> BtreeFlush::fPreAllocatedStackRegular;
VCriticalSection BtreeFlush::fAllocateRegularMutex;

BtreeFlush* BtreeFlush::GetNewBtreeFlush()
{
	BtreeFlush* res = nil;
	if (IsCurTaskFlushing())
	{
		VTaskLock lock(&fAllocateMutex); // ne devrait pas etre necessaire, mais je le mets pour l'instant

		if (!fPreAllocatedStack.empty())
		{
			res = fPreAllocatedStack.back();
			fPreAllocatedStack.pop_back();
		}
	}
	else
	{
		VTaskLock lock(&fAllocateRegularMutex);

		if (!fPreAllocatedStackRegular.empty())
		{
			res = fPreAllocatedStackRegular.back();
			fPreAllocatedStackRegular.pop_back();
		}
	}


	xbox_assert (res != nil);
	return res;

}


void BtreeFlush::AllocateBtreeFlush()
{
	sLONG i;

	SetCurTaskFlushing(true);

	VTaskLock lock(&fAllocateMutex); // ne devrait pas etre necessaire, mais je le mets pour l'instant

	for (i = (sLONG)fPreAllocatedStack.size(); i < MaxPreAllocatedBtreeFlush; i++)
	{
		BtreeFlush* page = new BtreeFlush;
		if (page == nil)
		{
			break;
		}
		else
			fPreAllocatedStack.push_back(page);
	}

	SetCurTaskFlushing(false);

}


void BtreeFlush::AllocateBtreeFlushRegular()
{
	sLONG i;

	VTaskLock lock(&fAllocateRegularMutex);

	for (i = (sLONG)fPreAllocatedStackRegular.size(); i < MaxPreAllocatedBtreeFlush; i++)
	{
		BtreeFlush* page = new BtreeFlush;
		if (page == nil)
		{
			break;
		}
		else
			fPreAllocatedStackRegular.push_back(page);
	}

}

void BtreeFlush::RecupereBtreeFlush(BtreeFlush* page)
{
	if (IsCurTaskFlushing())
	{
		VTaskLock lock(&fAllocateMutex); // ne devrait pas etre necessaire, mais je le mets pour l'instant
		if ((sLONG)fPreAllocatedStack.size() < MaxPreAllocatedBtreeFlush)
		{
			page->init();
			fPreAllocatedStack.push_back(page);
		}
		else
			delete page;
	}
	else
	{
		if (fAllocateRegularMutex.TryToLock())
		{
			if ((sLONG)fPreAllocatedStackRegular.size() < MaxPreAllocatedBtreeFlush)
			{
				page->init();
				fPreAllocatedStackRegular.push_back(page);
			}
			else
				delete page;
		}
		else
			delete page;
	}
}


void BtreeFlush::SetForNew( const BTitemFlush& u, BtreeFlush *sousbt)
{
	tabsous[0]=sousbt;
	tabsous[1]=u.sousBT;
	tabmem[0]=u.obj;
	oldaddr[0]=u.addr;
	nkeys=1;
}


BtreeFlush::~BtreeFlush()
{
#if VERSIONDEBUG
	Boolean toutvide = true;
	for (sLONG i = 0; i<=nkeys; i++)
	{
		if (tabsous[i] != nil)
		{
			toutvide = false;
		}
	}

	if (!__okdeletesousbt)
	{
		DebugMsg("attention delete BtreeFlush au mauvais moment");
	}
#endif
}

#if autocheckobj
uBOOL BtreeFlush::CheckObjInMem(void)
{
	sLONG i;
	CheckAssert(IsValidFastPtr(this));
	CheckAssert(nkeys>=0 && nkeys<=32);
	for (i=0;(i<=nkeys);i++)
	{
		CheckAssert(IsValidFastPtr(tabsous[i]));
	}
	return(true);
}
#endif


Boolean BtreeFlush::search(uBOOL *h, BTitemFlush *v, uBOOL pourdelete)
{
	BtreeFlush *sousBT;
	sLONG l,k,r,i;
	uBOOL isegal;
	IObjToFlush* obj2,*obj;
	BTitemFlush u;
	BtreeFlush **p,**pb;
	IObjToFlush  **p2,**p2b;
	DataAddr4D  *p3,*p3b;
	Boolean res = false;
	StAllocateInCache alloc;
	CompareResult comp;

	occupe(true);
	l=1;
	r=nkeys;
	isegal=false;
	obj=v->obj;

	while ( (!isegal) && (r>=l) )
	{
		k=(r+l)>>1;
		obj2=tabmem[k-1];
		if (obj2==obj)
		{
			isegal=true;
			res = true;
			if (pourdelete)
			{
				oldaddr[k-1]=obj->getaddr();
				tabmem[k-1]=nil;
				obj->Release();
				VInterlocked::Decrement(&sGlobalNbFlushObjects);
			}
		}
		else
		{
			if (obj2==nil)
			{
				comp = obj->CompAddr(oldaddr[k-1]);
			}
			else
			{
				comp = obj->CompAddr(obj2);
			}

			if (comp == CR_EQUAL)
			{
				if (pourdelete)
				{
					Boolean jusquaubout = true;
					for (sLONG ik = l; ik <= r; ik++)
					{
						if (tabmem[ik-1] == obj)
						{
							isegal = true;
							res = true;
							oldaddr[ik-1]=obj->getaddr();
							VInterlocked::Decrement(&sGlobalNbFlushObjects);
							tabmem[ik-1] = nil;
							jusquaubout = false;
							obj->Release();
							break;
						}
						else
						{
							CompareResult comp2;
							if (tabmem[ik-1] == nil)
								comp2 = obj->CompAddr(oldaddr[ik-1]);
							else
								comp2 = obj->CompAddr(tabmem[ik-1]);

							if (comp2 == CR_EQUAL)
							{
								sousBT = tabsous[ik-1];
								if (sousBT != nil )
								{
									u=*v;
									res = res || sousBT->search(h,&u,pourdelete);
									if (res)
									{
										isegal = true;
										jusquaubout = false;
									}
								}
							}
							else
							{
								jusquaubout = false;
								break;
							}
						}
					}
					if (jusquaubout)
					{
						sousBT=tabsous[r];
						if (sousBT!=nil)
						{
							u=*v;
							res = res || sousBT->search(h,&u,pourdelete);
							if (res)
								isegal = true;
						}
					}
				}
				else
				{
					if (obj2 == nil)
					{
						tabmem[k-1] = obj;
						isegal = true;
						VInterlocked::Increment(&sGlobalNbFlushObjects);
					}
				}
			}

			if (!isegal)
			{
				if (comp == CR_BIGGER)
				{
					l=k+1;
				}
				else
				{
					r=k-1;
				}
			}
		}
	}

	if (isegal)
	{
		l=l; /* ne sert a rien mais permet un break point */
	}
	else
	{
		sousBT=tabsous[r];
		if (sousBT!=nil)
		{
			u=*v;
			/*/ ?? faut il remettre u.sousBT a nil */
			res = res || sousBT->search(h,&u,pourdelete);
		}
		else
		{
			if (!pourdelete)
				VInterlocked::Increment(&sGlobalNbFlushObjects);
			*h=true;
			u=*v;
		}
		if (*h && !pourdelete)
		{
			if (nkeys<32)
			{
				*h=false;
				p=tabsous+nkeys+1;
				p2=tabmem+nkeys;
				p3=oldaddr+nkeys;
				for (i=r;i<nkeys;i++)
				{
					*p=*(p-1);
					p--;
					*p2=*(p2-1);
					--p2;
					*p3=*(p3-1);
					--p3;
				}
				tabsous[r+1]=u.sousBT;
				tabmem[r]=u.obj;
				oldaddr[r]=u.addr;
				//if (u.obj != nil) u.obj->SetTransOwner(u.owner);
				nkeys++;
			}
			else
			{
				sousBT=BtreeFlush::GetNewBtreeFlush();
				if (r<=16)
				{
					if (r==16)
					{
						*v=u;
					}
					else
					{
						v->addr=oldaddr[15];
						v->obj=tabmem[15];
						v->sousBT=tabsous[16];
						//if (v->obj != nil) v->owner = v->obj->GetTransOwner();
						p=tabsous+16;
						p2=tabmem+15;
						p3=oldaddr+15;

						for (i=r+1;i<16;i++)
						{
							*p=*(p-1);
							p--;
							*p2=*(p2-1);
							--p2;
							*p3=*(p3-1);
							--p3;
						}

						tabsous[r+1]=u.sousBT;
						tabmem[r]=u.obj;
						oldaddr[r]=u.addr;
						//if (u.obj != nil) u.obj->SetTransOwner(u.owner);
					}

					p=tabsous+17;
					p2=tabmem+16;
					p3=oldaddr+16;
					pb=sousBT->tabsous+1;
					p2b=sousBT->tabmem;
					p3b=sousBT->oldaddr;
					for (i=1;i<=16;i++)
					{
						*pb++=*p++;
						*p2b++=*p2++;
						*p3b++=*p3++;
					}
				}
				else
				{
					r=r-16;
					v->obj=tabmem[16];
					v->addr=oldaddr[16];
					v->sousBT=tabsous[17];
					//if (v->obj != nil) v->owner = v->obj->GetTransOwner();

					p=tabsous+18;
					p2=tabmem+17;
					p3=oldaddr+17;
					pb=sousBT->tabsous+1;
					p2b=sousBT->tabmem;
					p3b=sousBT->oldaddr;
					for (i=1;i<r;i++)
					{
						*pb++=*p++;
						*p2b++=*p2++;
						*p3b++=*p3++;
					}

					sousBT->tabsous[r]=u.sousBT;
					sousBT->tabmem[r-1]=u.obj;
					sousBT->oldaddr[r-1]=u.addr;
					//if (u.obj != nil) u.obj->SetTransOwner(u.owner);

					p=tabsous+16+r+1;
					p2=tabmem+16+r;
					p3=oldaddr+16+r;
					pb=sousBT->tabsous+1+r;
					p2b=sousBT->tabmem+r;
					p3b=sousBT->oldaddr+r;
					for (i=r+1;i<=16;i++)
					{
						*pb++=*p++;
						*p2b++=*p2++;
						*p3b++=*p3++;
					}
				}

				nkeys=16;
				sousBT->nkeys=16;
				sousBT->tabsous[0]=v->sousBT;
				v->sousBT=sousBT;
			}

		}
	}

	libere();
	return res;
}




#if VERSIONDEBUG
uBOOL fCheckFlushMore = 0;
#endif

uBOOL BtreeFlush::ViderFlush(VSize *tot, Boolean *outSomethingFlushed, FlushEventList* events, VCriticalSection* FlushEventListMutext, 
							 VProgressIndicator* progress, FlushProgressInfo& fpi, DataAddrSetVector& AllSegsFullyDeletedObjects)
{
	BtreeFlush *sousbt;
	uBOOL toutvide;
	IObjToFlush *obj;
	sLONG i;
	VSize tot2;

	toutvide=true;

	sousbt=tabsous[0];
	if (sousbt!=nil)
	{
		if (sousbt->ViderFlush(tot, outSomethingFlushed, events, FlushEventListMutext, progress, fpi, AllSegsFullyDeletedObjects))
		{
			VTaskLock lock(&gModifyInFlushEncoursMutex);
#if VERSIONDEBUG
			__okdeletesousbt = true;
#endif
			//delete sousbt;
			BtreeFlush::RecupereBtreeFlush(sousbt);
#if VERSIONDEBUG
			__okdeletesousbt = false;
#endif
			tabsous[0]=nil;
		}
		else
		{
			toutvide=false;
		}
	}

	for (i=0;i<nkeys;i++)
	{
		bool keepGoingIfLocked = true;
		while (keepGoingIfLocked)
		{
			VTaskLock lock(&gModifyInFlushEncoursMutex);
			keepGoingIfLocked = false;
			obj=tabmem[i];
			sousbt=tabsous[i+1];
			if (obj!=nil)
			{
				DataAddr4D addrobj = obj->getaddr();
				sLONG segnum = addrobj & (kMaxSegData-1);
				addrobj = addrobj & (DataAddr4D)(-kMaxSegData);

				DataAddrSetForFlush* px = &(AllSegsFullyDeletedObjects[segnum]);
				if (!px->fAddrs.empty())
				{
					sLONG nullzero = 0;
					while (px->fCurrent != px->fLast && (*(px->fCurrent) <= addrobj) )
					{
						fpi.currentBD->writelong(&nullzero, 4, *(px->fCurrent), 0);
						px->fCurrent++;
					}
				}

				if (obj->TryWriteLock())
				{					
					BaseFlushInfoIndex oldflushinfo = obj->SwitchToSaving();

					bool cont, oksave = true;

					if (oksave)
					{
						if (!obj->IsValidForWrite())
							oksave = false;
						else
						{
							do
							{
								cont = false;
								sLONG stamp = obj->GetOccupiedStamp();

		#if debugFindPlace_log
								DebugMsg(L"SaveObj : "+ToString(obj->getaddr())+L"  ,  obj = "+ToString((sLONG8)obj)+"\n\n");
		#endif

		#if debugFindPlace_strong
								fpi.currentBD->Debug_CheckAddrOverWrite(obj);
		#endif

								if (!obj->IsAddrAllowed())
								{
									oksave = true; // break here
									cont = false;
									tot2 = 0;
									xbox_assert(obj->IsAddrAllowed());
								}
								else
								{
									oksave = obj->SaveObj(tot2);
									if (stamp != obj->GetOccupiedStampBarrier())
									{
										if (obj->MustCheckOccupiedStamp())
											cont = true;
									}
								}
							} while (cont && oksave);
						}
					}

					if (oksave)
					{
						fpi.totbytes = fpi.totbytes + tot2;
						if (progress != nil)
						{
							uLONG curtime = VSystem::GetCurrentTime();
							if ((curtime - fpi.lasttime)  > 400)
							{
								fpi.lasttime = curtime;
								VStr<100> s;
								s = fpi.message;
								s+=L" (";
								VStr<80> s2;
								uLONG debit = fpi.totbytes / (curtime - fpi.starttime);
								if (debit > 10000)
								{
									s2.FromLong(debit/1000);
									s+=s2;
									s+=L" M/s)";

								}
								else
								{
									s2.FromLong(debit);
									s+=s2;
									s+=L" K/s)";
								}
								progress->SetMessage(s);
							}
							progress->Progress(fpi.curObjectNum);
						}
						fpi.curObjectNum++;
						*outSomethingFlushed = true;

						*tot=*tot+tot2;
						//obj->setmodif(false, nil, nil);
						obj->FinishSaving();
						VInterlocked::Decrement(&sGlobalNbFlushObjects);
						tabmem[i]=nil;

#if debuglr
						if (obj->GetRefCount() == 1)
						{
							obj = obj; // break here
						}
#endif

						obj->WriteUnLock();
						obj->Release();  // c'est le code final
						
						// je place ici du code intermediaire
						//obj->clearfromflush();

						{
							VTaskLock lock2(FlushEventListMutext);
							for (FlushEventList::iterator cur = events->begin(), end = events->end(); cur != end; )
							{
								Boolean next = true;
								if (cur->needmem)
								{
									cur->FreedMem += tot2;
									if (cur->FreedMem >= cur->NeededMem)
									{
										cur->flag->Unlock();
										cur->flag->Release();
										FlushEventList::iterator theone = cur;
										cur++;
										next = false;
										events->erase(theone);
									}
								}
								if (next)
									cur++;
							}
						}

						//xbox_assert(!VDBMgr::GetFlushManager()->FindObjInFlush(obj));
					}
					else
					{
						if (obj->IsValidForWrite())
							obj->FailedSaving(oldflushinfo);
						obj->WriteUnLock();
						toutvide = false;
					}
				
				}
				else
				{
					if (gCacheIsWaitingFlush == 0)
						keepGoingIfLocked = true;
					else
					{
						toutvide = false;
					}
				}
			}
		}

		if (sousbt!=nil)
		{
			if (sousbt->ViderFlush(tot, outSomethingFlushed, events, FlushEventListMutext, progress, fpi, AllSegsFullyDeletedObjects))
			{
				VTaskLock lock(&gModifyInFlushEncoursMutex);
#if VERSIONDEBUG
				__okdeletesousbt = true;
#endif
				//delete sousbt;
				BtreeFlush::RecupereBtreeFlush(sousbt);
#if VERSIONDEBUG
				__okdeletesousbt = false;
#endif
				tabsous[i+1]=nil;
			}
			else
			{
				toutvide=false;
			}
		}
	}

	return(toutvide);
}


void BtreeFlush::Transfert(void)
{
	BtreeFlush *sousbt;
	IObjToFlush *obj;
	sLONG i;

	//CHECKFLUSH;

	sousbt=tabsous[0];
	if (sousbt!=nil)
	{
		sousbt->Transfert();
#if VERSIONDEBUG
		__okdeletesousbt = true;
		tabsous[0] = nil;
#endif

		//delete sousbt;
		BtreeFlush::RecupereBtreeFlush(sousbt);

#if VERSIONDEBUG
		__okdeletesousbt = false;
#endif
	}

	for (i=0;i<nkeys;i++)
	{
		obj=tabmem[i];
		sousbt=tabsous[i+1];
		if (obj!=nil)
		{
			VDBMgr::PutObjectInFlush(obj, false, false, true);
			obj->Release();
		}
		if (sousbt!=nil)
		{
			sousbt->Transfert();
#if VERSIONDEBUG
			__okdeletesousbt = true;
			tabsous[i+1] = nil;
#endif
			//delete sousbt;
			BtreeFlush::RecupereBtreeFlush(sousbt);
#if VERSIONDEBUG
			__okdeletesousbt = false;
#endif
		}
	}

	//CHECKFLUSH;
}


/* ---------------------------------------------- */







#if VERSIONDEBUG


void BtreeFlush::FindSousBT(BtreeFlush* mere, BtreeFlush* fille)
{
	BtreeFlush *sousbt;
	sLONG i;

	for (i=0;(i<=nkeys);i++)
	{
		sousbt=tabsous[i];
		if (sousbt!=nil)
		{
			if (mere != this && fille == sousbt)
			{
				DebugMsg("duplicate souspage in flush");
			}
			sousbt->FindSousBT(mere, fille);
		}
	}

}


void BtreeFlush::CheckFlushPage(void)
{
	BtreeFlush *sousbt;
	sLONG i;
	IObjToFlush *obj;

	sousbt=tabsous[0];
	if (sousbt!=nil)
	{
		if (fCheckFlushMore) FindSousBT(this, sousbt);
		sousbt->CheckFlushPage();
	}

	for (i=0;(i<nkeys);i++)
	{
		if (tabmem[i]!=nil)
		{
			obj = tabmem[i];
			if ( *((sLONG*)obj) < 128000L )
			{
				DebugMsg("bad obj in flush");
			}
		}
		sousbt=tabsous[i+1];
		if (sousbt!=nil)
		{
			if (fCheckFlushMore) FindSousBT(this, sousbt);

			/*
			for (j=0;j<=i;j++)
			{
			if (tabsous[j] == sousbt)
			{
			DebugMsg("duplicate souspage in flush");
			}
			}
			*/

			sousbt->CheckFlushPage();
		}
	}
}


#endif


uBOOL BtreeFlush::FindObj(IObjToFlush *obj2)
{
	BtreeFlush *sousbt;
	IObjToFlush *obj;
	sLONG i;

	sousbt=tabsous[0];
	if (sousbt!=nil)
	{
		if (sousbt->FindObj(obj2)) return(true);
	}

	for (i=0;i<nkeys;i++)
	{
		obj=tabmem[i];
		sousbt=tabsous[i+1];
		if (obj==obj2)
		{
			return(true);
		}
		if (sousbt!=nil)
		{
			if (sousbt->FindObj(obj2)) return(true);
		}
	}

	return(false);
}



sLONG BtreeFlush::CompteObj(void)
{
	sLONG res;
	BtreeFlush *sousbt;
	sLONG i;

	res=0;

	sousbt=tabsous[0];
	if (sousbt!=nil)
	{
		res=res+sousbt->CompteObj();
	}

	for (i=0;(i<nkeys);i++)
	{
		if (tabmem[i]!=nil)
		{
			res++;
		}
		sousbt=tabsous[i+1];
		if (sousbt!=nil)
		{
			res=res+sousbt->CompteObj();
		}
	}

	return(res);
}

#if VERSIONDEBUG

void BtreeFlush::DisplayTree(void)
{
	BtreeFlush *sousbt;
	IObjToFlush *obj;
	sLONG i;

	sousbt=tabsous[0];
	if (sousbt!=nil)
	{
		sousbt->DisplayTree();
	}

	for (i=0;i<nkeys;i++)
	{
		obj=tabmem[i];
		sousbt=tabsous[i+1];
		if (obj!=nil)
		{
			DebugMsg("obj = "/*,(sLONG)obj*/);
			DebugMsg("     addr    = "/*,obj->getaddr()*/);
			DebugMsg("     oldaddr = "/*,oldaddr[i]*/);
		}
		if (sousbt!=nil)
		{
			sousbt->DisplayTree();
		}
	}
}


Boolean BtreeFlush::FindObjInFlushByAddr( DataAddr4D addr)
{
	BtreeFlush *sousbt;
	IObjToFlush *obj;
	sLONG i;
	Boolean res = false;

	if (!occupe())
	{
		sousbt=tabsous[0];
		if (sousbt!=nil)
		{
			res = res || sousbt->FindObjInFlushByAddr(addr);
		}

		for (i=0;i<nkeys;i++)
		{
			obj=tabmem[i];
			sousbt=tabsous[i+1];
			if (obj!=nil)
			{
				if (obj->getaddr() == addr)
				{
					xbox_assert(false);
					res = true;
				}
			}
			if (sousbt!=nil)
			{
				res = res || sousbt->FindObjInFlushByAddr(addr);
			}
		}
		libere();
	}

	return res;
}



Boolean BtreeFlush::FindObjInFlush(IObjToFlush *obj2)
{
	BtreeFlush *sousbt;
	IObjToFlush *obj;
	sLONG i;
	Boolean res = false;

	if (!occupe())
	{
		sousbt=tabsous[0];
		if (sousbt!=nil)
		{
			res = res || sousbt->FindObjInFlush(obj2);
		}

		for (i=0;i<nkeys;i++)
		{
			obj=tabmem[i];
			sousbt=tabsous[i+1];
			if (obj==obj2)
			{
				xbox_assert(false);
				res = true;
			}
			if (sousbt!=nil)
			{
				res = res || sousbt->FindObjInFlush(obj2);
			}
		}
		libere();
	}

	return res;
}


sLONG BtreeFlush::CompteAddr(DataAddr4D xaddr)
{
	BtreeFlush *sousbt;
	IObjToFlush *obj;
	sLONG i;
	sLONG tot;

	tot=0;
	sousbt=tabsous[0];
	if (sousbt!=nil)
	{
		tot=tot+sousbt->CompteAddr(xaddr);
	}

	for (i=0;i<nkeys;i++)
	{
		obj=tabmem[i];
		sousbt=tabsous[i+1];
		if (obj!=nil)
		{
			if (obj->getaddr()==xaddr)
			{
				tot++;
			}
		}
		if (sousbt!=nil)
		{
			tot=tot+sousbt->CompteAddr(xaddr);
		}
	}

	return(tot);
}


void BtreeFlush::CheckFlushObjs(DataAddr4D &curaddr)
{
	BtreeFlush *sousbt;
	IObjToFlush *obj;
	sLONG i;
	DataAddr4D xaddr;

	//if (!occupe())
	{

		for (i=0;i<nkeys;i++)
		{
			sousbt = tabsous[i];
			if (sousbt!=nil)
				sousbt->CheckFlushObjs(curaddr);

			obj=tabmem[i];
			if (obj!=nil)
			{
				xbox_assert(obj->modifie());
				xaddr = obj->getaddr();
				xbox_assert (obj->getaddr() >= 0);
				xbox_assert (obj->getaddr() == oldaddr[i]);
			}
			else
				xaddr = oldaddr[i];

			uLONG numseg1 = ((uLONG)xaddr) & (kMaxSegData-1);
			uLONG numseg2 = ((uLONG)curaddr) & (kMaxSegData-1);

			if (numseg1 == numseg2)
			{
				if (xaddr > 512)
					xbox_assert(xaddr > curaddr);
				else
					xbox_assert(xaddr >= curaddr);
			}
			else
			{
				xbox_assert(numseg1 >= numseg2);
			}
			curaddr = xaddr;
		}

		sousbt = tabsous[nkeys];
		if (sousbt!=nil)
			sousbt->CheckFlushObjs(curaddr);

		//libere();
	}
}


sLONG BtreeFlush::CompteObjOccurence(IObjToFlush *obj2)
{
	BtreeFlush *sousbt;
	IObjToFlush *obj;
	sLONG i;
	sLONG res = 0;

	if (!occupe())
	{
		sousbt=tabsous[0];
		if (sousbt!=nil)
		{
			res = res + sousbt->CompteObjOccurence(obj2);
		}

		for (i=0;i<nkeys;i++)
		{
			obj=tabmem[i];
			sousbt=tabsous[i+1];
			if (obj==obj2)
			{
				res++;
			}
			if (sousbt!=nil)
			{
				res = res + sousbt->CompteObjOccurence(obj2);
			}
		}
		libere();
	}

	return res;
}


void BtreeFlush::CheckDuplicatesInFlush()
{
	BtreeFlush *sousbt;
	IObjToFlush *obj;
	sLONG i;

	if (!occupe())
	{

		for (i=0;i<nkeys;i++)
		{
			obj=tabmem[i];
			if (obj!=nil)
			{
				xbox_assert (obj->getaddr() >= 0);
			}
		}

		for (i=0;i<=nkeys;i++)
		{
			sousbt = tabsous[i];
			if (sousbt!=nil)
				sousbt->CheckDuplicatesInFlush();
		}

		libere();
	}
}



#endif



/* ---------------------------------------------- */


BaseFlushInfo::BaseFlushInfo(Base4D* xbd, VDBFlushMgr* pere)
{
	left = nil;
	right = nil;
	fCurTree = nil;
	fFlushingTree = nil;
	bd = xbd;
	parent = pere;
	doisdelete = false;
	isflushing = false;
	fOutOfList = true;
	fWaitToInsertNewObject = 0;
	fNbModifStamp = 0;
	fRequestForInvalid = 0;
}


BaseFlushInfo::~BaseFlushInfo()
{
	xbox_assert(fCurTree == nil && fFlushingTree == nil && fOutOfList);
}


void BaseFlushInfo::Dispose(void)
{
	Boolean busy;
	sLONG count = 0;

	parent->xDeleteBaseFlushInfo(this);
	do
	{
		{
			VTaskLock Lock(&TreeMutex);
			busy = (fCurTree != nil) || (fFlushingTree != nil) || (!fOutOfList);
			++count;
		}
		if (busy)
		{
			VTask::GetCurrent()->Sleep(100);
			VDBMgr::GetFlushManager()->SetDirty(true);
			VDBMgr::GetFlushManager()->Flush(false);
		}
	} while (busy);

	VDBFlushMgr::FreeFlushInfoID(fID);
	//delete this;
	Release();
}

/* ---------------------------------------------- */



BaseFlushInfo* VDBFlushMgr::NewBaseFlushInfo(Base4D* bd)
{
	VTaskLock lock(&ListFlushInfoMutext);

	BaseFlushInfo* fi = new BaseFlushInfo(bd, this);
	BaseFlushInfoIndex xid = sTableFlushinfo.AddInAHole(fi);
	fi->SetID(xid);
	fi->fOutOfList = false;
	if (ListFlushInfo != nil) ListFlushInfo->left = fi;
	fi->right = ListFlushInfo;
	ListFlushInfo = fi;
	return fi;
}


void VDBFlushMgr::DeleteBaseFlushInfo(BaseFlushInfo* info)
{
	// VTaskLock lock(&(info->TreeMutex)); // mutex fait par l'appelant

	if (info->fCurTree == nil && info->fFlushingTree == nil)
	{
		if (info->right != nil) info->right->left = info->left;
		if (info->left != nil) info->left->right = info->right;
		if (info == ListFlushInfo) ListFlushInfo = info->right;
		//delete info;
		info->fOutOfList = true;
		//info->GetOwner()->ValidateHeader();
		fNeedValidate.erase(info);

	}

}


void VDBFlushMgr::xDeleteBaseFlushInfo(BaseFlushInfo* info)
{
	VTaskLock lock(&(info->TreeMutex));

	info->doisdelete = true;
	if (info->fCurTree != nil || info->fFlushingTree != nil)
	{
		// on demarre un flush
		Flush(false);
	}

}


void VDBFlushMgr::SetDirty( Boolean inIsDirty )
{
	VTaskLock lock(&DirtyMutext);

	fIsDirty = inIsDirty;
}


Boolean VDBFlushMgr::IsTimeToFlush()
{
	return xAbs<sLONG>( VSystem::GetCurrentTime() - fLastFlushTime) > fFlushPeriod;
}


VSyncEvent* VDBFlushMgr::NewFlushEnoughMemEvent(VSize inNeededBytes)
{
	FlushEvent fev;
	fev.NeededMem = inNeededBytes;
	fev.FreedMem = 0;
	fev.flag = new VSyncEvent();
	fev.flag->Retain();
	fev.needmem = true;
	fev.waitForAllWritten = false;

	{
		VTaskLock lock(&FlushEventMutex);
		fev.FlushCycle = fCurrentFlushCycle;
		events.push_back(fev);
	}
	return fev.flag;
}


VSyncEvent* VDBFlushMgr::NewFlushEndEvent()
{
	FlushEvent fev;
	fev.NeededMem = 0;
	fev.FreedMem = 0;
	fev.flag = new VSyncEvent();
	fev.flag->Retain();
	fev.needmem = false;
	fev.waitForAllWritten = false;

	{
		VTaskLock lock(&FlushEventMutex);
		fev.FlushCycle = fCurrentFlushCycle;
		events.push_back(fev);
	}
	return fev.flag;
}


VSyncEvent* VDBFlushMgr::NewFlushAllWrittenEvent()
{
	FlushEvent fev;
	fev.NeededMem = 0;
	fev.FreedMem = 0;
	fev.flag = new VSyncEvent();
	fev.flag->Retain();
	fev.needmem = false;
	fev.waitForAllWritten = true;

	{
		VTaskLock lock(&FlushEventMutex);
		fev.FlushCycle = fCurrentFlushCycle;
		events.push_back(fev);
	}
	return fev.flag;
}



void VDBFlushMgr::Flush(Base4D* target, Boolean inSynchronous)
{
	Flush(inSynchronous);
}


void VDBFlushMgr::Flush( Boolean inSynchronous, Boolean onlyForAllWritten)
{
#if debuglr == 101
	if (fFlushProgress != nil && !fFlushProgress->IsManagerValid())
	{
		sLONG xdebug = 1; // put a break here
	}
#endif

	if (fFlushTask != nil) 
	{
		if (inSynchronous)
		{	
#if trackClose
			trackDebugMsg("call flush synchronous\n");
#endif
			sLONG	lastStamp = StartFlushStamp;
			VSyncEvent* wait = nil;
			{
				{
					VTaskLock lock(&StartFlushMutex);
					if (lastStamp == StartFlushStamp)
					{
						if (onlyForAllWritten)
							wait = NewFlushAllWrittenEvent();
						else
							wait = NewFlushEndEvent();
#if trackClose
							trackDebugMsg("call flush synchronous after mutex\n");
#endif						
							Flush( false);
					}
				}

				if (wait != nil)
				{
					wait->Lock();
					wait->Release();
					{
						VTaskLock lock(&StartFlushMutex);
						StartFlushStamp++;
					}
				}
#if trackClose
				trackDebugMsg("end of call flush synchronous\n");
#endif			
			}
		}
		else // pas synchrone
		{
#if trackClose
			trackDebugMsg("call flush asynchronous\n");
#endif			
			if (StartFlushMutex.TryToLock())
			{
#if trackClose
				trackDebugMsg("call flush asynchronous after mutex\n");
#endif						
				{
					VTaskLock lock(&DirtyMutext);
					fIsAskingFlush = true;
					//fFlushTask->WakeUp();
				}
				WakeUp();
				StartFlushMutex.Unlock();
#if trackClose
				trackDebugMsg("release mutext for call flush asynchronous\n");
#endif						
			}
		}
	}
}


Boolean VDBFlushMgr::SomethingToFlush(void)
{
	VTaskLock lock1(&ListFlushInfoMutext);
	BaseFlushInfo* curinfo;
	Boolean result = false;

	curinfo = ListFlushInfo;

	while (curinfo != nil)
	{
		VTaskLock lock2(&(curinfo->TreeMutex));
		if (curinfo->fCurTree != nil) result = true;
		curinfo = curinfo->right;
	}

	return result;

}

Boolean VDBFlushMgr::NeedsBytes( VSize inNeededBytes)
{
	Boolean isOK = false;
	if (VTask::GetCurrent() != fFlushTask)
	{
		sLONG	lastStamp = StartFlushStamp;
		VTaskLock lock(&StartFlushMutex);
		if (lastStamp == StartFlushStamp)
		{
			{
				VSyncEvent* wait = NewFlushEnoughMemEvent(inNeededBytes);

				Flush( false);


				fNbWaitingClients++;
				wait->Lock();
				fNbWaitingClients--;
				isOK = true;
				StartFlushStamp++;
				wait->Release();
			}
		}
		else
			isOK = true; // si le stamp a change, cela veut dire qu'une autre tache a deja demande au flush de liberer sur disk de l'espace
	}
	return isOK;
}

#if VERSIONDEBUG

void VDBFlushMgr::CheckFlushObjs(void)
{
	//	VTaskLock lock1(&ListFlushInfoMutext);
	BaseFlushInfo* curinfo;
	Boolean result = false;

	curinfo = ListFlushInfo;

	while (curinfo != nil)
	{
		VTaskLock lock2(&(curinfo->TreeMutex));
		DataAddr4D curaddr = 0;
		if (curinfo->fCurTree != nil) 
			curinfo->fCurTree->CheckFlushObjs(curaddr);
		curinfo = curinfo->right;
	}

}


void VDBFlushMgr::CheckDuplicatesInFlush(void)
{
	//	VTaskLock lock1(&ListFlushInfoMutext);
	BaseFlushInfo* curinfo;
	Boolean result = false;

	curinfo = ListFlushInfo;

	while (curinfo != nil)
	{
		VTaskLock lock2(&(curinfo->TreeMutex));
		if (curinfo->fCurTree != nil) curinfo->fCurTree->CheckDuplicatesInFlush();
		curinfo = curinfo->right;
	}

}


void VDBFlushMgr::CheckDuplicatesInFlushEnCours(void)
{
	//	VTaskLock lock1(&ListFlushInfoMutext);
	BaseFlushInfo* curinfo;
	Boolean result = false;

	curinfo = ListFlushInfo;

	while (curinfo != nil)
	{
		VTaskLock lock2(&(curinfo->TreeMutex));
		if (curinfo->fFlushingTree != nil) curinfo->fFlushingTree->CheckDuplicatesInFlush();
		curinfo = curinfo->right;
	}

}


void VDBFlushMgr::FindSousBT( BtreeFlush* mere, BtreeFlush* fille)
{
	/*
	if (fCurTree!=nil)
	fCurTree->FindSousBT(mere, fille);
	if (fFlushingTree!=nil)
	fFlushingTree->FindSousBT(mere, fille);
	*/
}


void VDBFlushMgr::TestFlushAddr(DataAddr4D xaddr)
{
	/*
	if ( (fCurTree!=nil) && (xaddr!=0) ) {
	sLONG n=fCurTree->CompteAddr(xaddr);
	xbox_assert(n<=1);
	}
	*/
}


Boolean VDBFlushMgr::FindObjInFlush( IObjToFlush *obj)
{
	Boolean res = false;
	//	VTaskLock lock1(&ListFlushInfoMutext);
	BaseFlushInfo* curinfo;
	Boolean result = false;

	curinfo = ListFlushInfo;

	while (curinfo != nil)
	{
		VTaskLock lock2(&(curinfo->TreeMutex));
		if (curinfo->fCurTree != nil) 
			res = res || curinfo->fCurTree->FindObjInFlush(obj);
		curinfo = curinfo->right;
	}

	return res;
}



Boolean VDBFlushMgr::FindObjInFlushByAddr( DataAddr4D addr)
{
	Boolean res = false;
	//	VTaskLock lock1(&ListFlushInfoMutext);
	BaseFlushInfo* curinfo;
	Boolean result = false;

	curinfo = ListFlushInfo;

	while (curinfo != nil)
	{
		VTaskLock lock2(&(curinfo->TreeMutex));
		if (curinfo->fCurTree != nil) 
			res = res || curinfo->fCurTree->FindObjInFlushByAddr(addr);
		curinfo = curinfo->right;
	}

	return res;
}


sLONG VDBFlushMgr::CompteObjOccurence( IObjToFlush *obj)
{
	sLONG res = 0;
	//	VTaskLock lock1(&ListFlushInfoMutext);
	BaseFlushInfo* curinfo;
	Boolean result = false;

	curinfo = ListFlushInfo;

	while (curinfo != nil)
	{
		VTaskLock lock2(&(curinfo->TreeMutex));
		if (curinfo->fFlushingTree != nil) 
			res = res + curinfo->fFlushingTree->CompteObjOccurence(obj);
		curinfo = curinfo->right;
	}

	return res;
}


void VDBFlushMgr::CheckFlushInMem()
{
	/*
	if (fCheckFlush) {
	if (fCurTree!=nil)
	fCurTree->CheckFlushPage();
	if (fFlushingTree!=nil)
	fFlushingTree->CheckFlushPage();
	}
	*/
}

#endif


void VDBFlushMgr::xDisposeFlushingTree( BaseFlushInfo *info, Boolean inWithTransfert)
{
	// attention: section critique !

	VTaskLock lock(&(info->TreeMutex));

	BtreeFlush *tree = info->fFlushingTree;
	info->fFlushingTree = nil;

	if (inWithTransfert) {
		// attention: Transfert doit etre atomique
		VTaskLock lock2(&(info->TransfertMutex));

#if VERSIONDEBUG
		if (fJFlush)
		{
			WriteLnLogStr(".... entre dans transfert");
		}
#endif

		tree->Transfert();

#if VERSIONDEBUG
		if (fJFlush)
		{
			WriteLnLogStr(".... sort de transfert");
		}
#endif
	}

#if VERSIONDEBUG
	__okdeletesousbt = true;
#endif
	//delete tree;
	BtreeFlush::RecupereBtreeFlush(tree);
#if VERSIONDEBUG
	__okdeletesousbt = false;
#endif
}

#if VERSIONDEBUG

void VDBFlushMgr::CheckIfObjectAfterValidate(IObjToFlush *inObject)
{
	BaseFlushInfo *info = inObject->GetBaseFlushInfo();
	if (info == nil)
	{
		info = info; // put a break here
	}
	else
	{
		if (fNeedValidate.find(info) == fNeedValidate.end())
		{
			info = info; // put a break here
		}
	}
}
#endif


void VDBFlushMgr::PutObject(IObjToFlush *inObject, Boolean inSetCacheDirty, Boolean inForDelete, Boolean NoWait)
{
	BaseFlushInfo *info;
	StAllocateInCache alloc;

	//CHECKFLUSH;
#if debuglr == 117
	if (debug_tools_started)
	{
		*((sLONG*)0) = 0xffff; // crash on purpose
	}
#endif

#if debuglr == 101
	if (fFlushProgress != nil && !fFlushProgress->IsManagerValid())
	{
		sLONG xdebug = 1; // put a break here
	}
#endif

	if (!inForDelete)
		inObject->Retain();

	info = inObject->GetBaseFlushInfo();
	if ((info == nil) && inForDelete)
	{
		info = nil; // put a break here
		// cela peut etre normal pour un objet charge du disque et par encore modifie , on ne fait rien
	}
	else
	{
		BTitemFlush u;
		u.obj=inObject;
		u.sousBT=nil;
		u.addr=inObject->getaddr();
		//		u.owner = DefaultFlushTransac;
		uBOOL h = false;

		if (info == nil)
		{
			inSetCacheDirty = false;
			// put a break here
			u.obj->ClearModifFlag();
		}
		else
		{
			/*
			uLONG waittime = info->fWaitToInsertNewObject;
			if (waittime > 0 && !inForDelete && !NoWait)
			VTask::GetCurrent()->Sleep(waittime);
			*/
			if (!IsCurTaskFlushing())
				BtreeFlush::AllocateBtreeFlushRegular();
			{
				VTaskLock lock(&(info->TreeMutex));
				if (!inForDelete)
					info->fNbModifStamp++;
				if (info->fCurTree == nil)
					info->fCurTree = BtreeFlush::GetNewBtreeFlush();

#if debugflush
				DataAddr4D curaddr = 0;
				if (info->fCurTree != nil)
					info->fCurTree->CheckFlushObjs(curaddr);
#endif


#if debugflush
#endif
				Boolean isFound = info->fCurTree->search(&h,&u,inForDelete);

				if (h && !inForDelete)
				{
					BtreeFlush* newBT = BtreeFlush::GetNewBtreeFlush();
					newBT->SetForNew(u, info->fCurTree);
					//BtreeFlush *newBT = new BtreeFlush( u, info->fCurTree);
					/*
					if (u.obj != nil)
					u.obj->SetTransOwner(u.owner);
					*/
					info->fCurTree=newBT;
				}

#if debugflush
				curaddr = 0;
				if (info->fCurTree != nil)
					info->fCurTree->CheckFlushObjs(curaddr);
#endif
				if (inForDelete && !isFound)
				{
					VTaskLock lock(&gModifyInFlushEncoursMutex);
#if debugflush
					curaddr = 0;
					if (info->fCurTree != nil)
						info->fFlushingTree->CheckFlushObjs(curaddr);
#endif
					Boolean isFound2 = false;
					if (info->fFlushingTree != nil)
						isFound2 = info->fFlushingTree->search(&h,&u,inForDelete);
#if debugflush
					curaddr = 0;
					if (info->fCurTree != nil)
						info->fFlushingTree->CheckFlushObjs(curaddr);
#endif
#if debuglr
					isFound = isFound2; // ne sert a rien mais permet de mettre un break point
#endif
				}
			}
		}
	}

	if (inSetCacheDirty)
		SetDirty();
}


void VDBFlushMgr::RemoveObject(IObjToFlush *inObject )
{
	PutObject( inObject,false, true, false);
}








void VDBFlushMgr::xFlushEnd()
{
	VTaskLock lock(&DirtyMutext);

	for (SetOfBaseFlushInfo::iterator cur = fNeedValidate.begin(), end = fNeedValidate.end(); cur != end; cur++)
	{
		(*cur)->GetOwner()->ValidateHeader();
	}

	xbox_assert( fIsFlushing);
	fIsFlushing = false;
	fIsAskingFlush = false;
	fLastFlushTime = VSystem::GetCurrentTime();
	++fCountFlushes;

	{
		VTaskLock lock2(&FlushEventMutex);
		for (FlushEventList::iterator cur = events.begin(), end = events.end(); cur != end; cur++)
		{
			cur->flag->Unlock();
			cur->flag->Release();
		}
		events.clear();
	}
}


Boolean VDBFlushMgr::GetDirty()
{
	VTaskLock lock(&DirtyMutext);

	return fIsDirty;
}



VDBFlushMgr::VDBFlushMgr( VDBMgr *inManager)
{
	fManager = inManager;
	fCurrentFlushCycle = 0;

	fIsDirty = false;
	fIsFlushing = false;
	fIsAskingFlush = false;
	fIsFlushTaskReady = false;
	fFlushPeriod = 10*1000  ; // 10 sec par defaut
	fLastFlushTime = VSystem::GetCurrentTime();
	fCountFlushes = 0;
	fNbWaitingClients = 0;
	ListFlushInfo = nil;
	StartFlushStamp = 0;
	fFlushProgress = nil;
#if VERSIONDEBUG
	fCheckFlush = false;
#endif
}

VDBFlushMgr::~VDBFlushMgr()
{
	if (fFlushTask != nil) {
		fFlushTask->Kill();
		fDemandeDeFlush.Unlock();
		fFlushTask->WaitForDeath( 10000);
		::CopyRefCountable( &fFlushTask, (VDBFlushTask *) nil);
	}
}


VDBFlushMgr *VDBFlushMgr::NewFlushManager( VDBMgr *inManager)
{
	VDBFlushMgr *manager = new VDBFlushMgr( inManager);
	if (!manager->Init()) {
		delete manager;
		manager = nil;
	}
	return manager;
}


Boolean VDBFlushMgr::Init()
{
#if debuglr
	debug_tools_started = false;
#endif
	if (sTableFlushinfo.GetCount() == 0)
		sTableFlushinfo.Add(nil);

	BtreeFlush::AllocateBtreeFlush();
	BtreeFlush::AllocateBtreeFlushRegular();

	fFlushTask = new VDBFlushTask( this);
	fFlushTask->SetName( CVSTR( "DB4D Flush"));
	fFlushTask->Run();
	return true;
}


void VDBFlushMgr::MainLoop()
{
	VDebugContext& context = VTask::GetCurrent()->GetDebugContext();
	context.StartLowLevelMode();

	sLONG nbpass = 1;
	Boolean AuMoinsUnFlushTreeNonVide;

	fIsFlushing = true;

	if (VDBMgr::GetActivityManager() != nil)
		VDBMgr::GetActivityManager()->SetActivityWrite(true, -1);
	/*
	VGoldfingerSysTrayServer* systray = VGoldfingerSysTrayServer::GetSysTray();
	if ( NULL != systray )
	systray->SetActivityWrite_Toggle(true);
	*/

	VCacheLogQuotedEntry	logEntry(eCLEntryKind_Flush, false, true);

#if trackClose
	trackDebugMsg("debut flush MainLoop\n");
#endif						

	while (nbpass<2)
	{
		uLONG WaitOthers = 1;

		//fFlushedBytes = 0;
		AuMoinsUnFlushTreeNonVide = true;

		FlushProgressInfo fpi;
		fpi.curObjectNum = 0;
		fpi.totbytes = 0;
		fpi.starttime = VSystem::GetCurrentTime();
		fpi.lasttime = fpi.starttime;
		fpi.currentBD = nil;

#if debuglr == 101
		if (fFlushProgress != nil && !fFlushProgress->IsManagerValid())
		{
			sLONG xdebug = 1; // put a break here
		}

		if (fFlushProgress != nil)
			fFlushProgress->Release();
#endif

		fFlushProgress = VDBMgr::GetManager()->RetainDefaultProgressIndicator_For_DataCacheFlushing();
		if (fFlushProgress != nil)
		{
			XBOX::VString session_title;
			gs(1005,22,session_title); // Flushing Data
			fpi.message = session_title;
			fFlushProgress->BeginSession(sGlobalNbFlushObjects,session_title,false);
		}

		Boolean RAZTimers = true;

		SetCurTaskFlushing(true);
		while( AuMoinsUnFlushTreeNonVide) 
		{

			Boolean AllisSomethingFlushed = false;
			Boolean toutvide = true, waitForRequestForInvalid = false;

			SetDirty( false);
			//xSetFlushCancelled( false);

			BaseFlushInfo *curinfo;

			{
				VTaskLock Lock(&ListFlushInfoMutext);
				curinfo = ListFlushInfo;
			}

			while (curinfo != nil)
			{
				Boolean isSomethingFlushed = false;
				BtreeFlush *newbtflush;
				Boolean mustincreasetimer = false;
				sLONG NbModifStamp = 0;
				DataAddrSetVector AllSegsFullyDeletedObjects;

				{
					VTaskLock Lock(&(curinfo->TreeMutex));
					if (RAZTimers)
					{
						curinfo->fWaitToInsertNewObject = 0;
						curinfo->fWaitToInsertNewObjectTimer.Unlock();
					}
					newbtflush = curinfo->fCurTree;
					curinfo->fFlushingTree = newbtflush;
					curinfo->fCurTree = nil;
					if (newbtflush != nil)
					{
						fpi.currentBD = curinfo->GetOwner();
						curinfo->GetOwner()->SwapAllSegsFullyDeletedObjects(AllSegsFullyDeletedObjects);
						if (fNeedValidate.find(curinfo) == fNeedValidate.end())
						{
							curinfo->GetOwner()->InvalidateHeader();
							fNeedValidate.insert(curinfo);
						}
					}
					else
					{
						if (curinfo->fRequestForInvalid > 0)
						{
							waitForRequestForInvalid = true;
							if (fNeedValidate.find(curinfo) == fNeedValidate.end())
							{
								curinfo->GetOwner()->InvalidateHeader();
								fNeedValidate.insert(curinfo);
							}
						}
					}
					NbModifStamp = curinfo->fNbModifStamp;
				}

				if (newbtflush != nil)
				{
					VSize curtot = 0;
					if (newbtflush->ViderFlush( &curtot, &isSomethingFlushed, &events, &FlushEventMutex, fFlushProgress, fpi, AllSegsFullyDeletedObjects))
					{
						// comme tout l'arbre a ete flushe, on peut le supprimer
						xDisposeFlushingTree( curinfo, false);
						newbtflush=nil;
					}
					else toutvide = false;

					for (DataAddrSetVector::iterator cur = AllSegsFullyDeletedObjects.begin(), end = AllSegsFullyDeletedObjects.end(); cur != end; cur++)
					{
						DataAddrSetForFlush* px = &(*cur);
						if (!px->fAddrs.empty())
						{
							sLONG nullzero = 0;
							while (px->fCurrent != px->fLast )
							{
								fpi.currentBD->writelong(&nullzero, 4, *(px->fCurrent), 0);
								px->fCurrent++;
							}
						}
					}

					if (NbModifStamp != curinfo->fNbModifStamp)
						mustincreasetimer = true;

					if (newbtflush!=nil)
					{
						// comme certains objets n'ont pu etre flushe, on recopie le residu d'arbre pour un prochain essai
						xDisposeFlushingTree( curinfo, true);
						newbtflush=nil;
						toutvide = false;
					}

					AllisSomethingFlushed = AllisSomethingFlushed || isSomethingFlushed;

					{
						VTaskLock Lock(&(curinfo->TreeMutex));
						if (curinfo->fCurTree == nil)
						{
							if (curinfo->fRequestForInvalid > 0)
							{
								waitForRequestForInvalid = true;
							}
							else
							{
								fNeedValidate.erase(curinfo);
								curinfo->GetOwner()->ValidateHeader();
							}
							curinfo->fWaitToInsertNewObject = 0;
							curinfo->fWaitToInsertNewObjectTimer.Unlock();
						}
						else
						{
							if (mustincreasetimer)
							{
								if (curinfo->fWaitToInsertNewObject == 0)
								{
									curinfo->fWaitToInsertNewObject = 1;
									curinfo->fLastTimeIncrease = VSystem::GetCurrentTime();
									curinfo->fWaitToInsertNewObjectTimer.ResetAndLock();
								}
								else
								{
									uLONG curtime = VSystem::GetCurrentTime();
									if ((uLONG)abs((sLONG)(curtime - curinfo->fLastTimeIncrease)) > curinfo->fWaitToInsertNewObject)
									{
										if (curinfo->fWaitToInsertNewObject < 8192)
											curinfo->fWaitToInsertNewObject = curinfo->fWaitToInsertNewObject * 2;
										curinfo->fLastTimeIncrease = curtime;
									}
								}
							}
							else // decrease
							{
								if (curinfo->fWaitToInsertNewObject != 0)
								{
									if (curinfo->fWaitToInsertNewObject == 1)
									{
										curinfo->fWaitToInsertNewObject = 0;
										curinfo->fWaitToInsertNewObjectTimer.Unlock();
									}
									else
									{
										uLONG curtime = VSystem::GetCurrentTime();
										if ((uLONG)abs((sLONG)(curtime - curinfo->fLastTimeIncrease)) > curinfo->fWaitToInsertNewObject)
										{
											curinfo->fWaitToInsertNewObject = curinfo->fWaitToInsertNewObject / 2;
											curinfo->fLastTimeIncrease = curtime;
										}
									}
								}
							}

							toutvide = false;
						}
					}

				}

				{
					VTaskLock Lock(&(curinfo->TreeMutex));
					curinfo = curinfo->right;
				}
			}

			// on libere toute les demandes de flush pour memoire qui ont, au moins, fait un cycle complet
			{
				VTaskLock lock2(&FlushEventMutex);
				for (FlushEventList::iterator cur = events.begin(), end = events.end(); cur != end; )
				{
					Boolean next = true;
					if (cur->needmem)
					{
						if (cur->FlushCycle!= fCurrentFlushCycle)
						{
							cur->flag->Unlock();
							cur->flag->Release();
							FlushEventList::iterator theone = cur;
							cur++;
							next = false;
							events.erase(theone);
						}
					}
					if (next)
						cur++;
				}

				fCurrentFlushCycle++;
			}

			// maintenant on supprime les BaseFlushInfo des bases qu'on a ferme
			{
				VTaskLock Lock(&ListFlushInfoMutext);
				curinfo = ListFlushInfo;

				while (curinfo != nil)
				{
					Boolean doisdelete = false;
					BaseFlushInfo* suivant;
					{
						VTaskLock Lock2(&(curinfo->TreeMutex));
						suivant = curinfo->right;

						doisdelete = (curinfo->doisdelete);
						if (doisdelete) 
							DeleteBaseFlushInfo(curinfo);
					}

					curinfo = suivant;
				}
			}

			{
				VTaskLock lock2(&FlushEventMutex);
				for (FlushEventList::iterator cur = events.begin(), end = events.end(); cur != end; )
				{
					Boolean next = true;
					if (cur->waitForAllWritten)
					{
						cur->flag->Unlock();
						cur->flag->Release();
						FlushEventList::iterator theone = cur;
						cur++;
						next = false;
						events.erase(theone);
					}
					if (next)
						cur++;
				}
			}
			if (toutvide)
			{
				if (waitForRequestForInvalid)
					AuMoinsUnFlushTreeNonVide = true;
				else
					AuMoinsUnFlushTreeNonVide = false; // on peut quitter la boucle car tous les arbres ont ete flushes
			}
			else
			{
				if (WaitOthers < 2)
					WaitOthers = WaitOthers * 2;
				VTask::GetCurrent()->Sleep(WaitOthers);
				//vYieldNow();
				//on donne une chance aux autres process de liberer les objets lockes
			}

			RAZTimers = false;

			/*  moved to DB4D CRON
			VDBMgr::GetCacheManager()->ClearWaitListForBigObject();
			VDBMgr::GetManager()->PurgeExpiredDataSets();
			*/
			VDBMgr::GetManager()->DeleteDeadBlobPaths();

		}

		if (fFlushProgress != nil)
		{
			fFlushProgress->EndSession();
#if debuglr == 101
			// when debugging, keep the progress indicator
#else
			fFlushProgress->Release();
			fFlushProgress = nil;
#endif
		}

		SetCurTaskFlushing(false);
		//SetDirty( false);

		++nbpass;
	}

#if trackClose
	trackDebugMsg("fin flush MainLoop\n");
#endif						

	xFlushEnd();

#if trackClose
	trackDebugMsg("apres xFlushEnd\n");
#endif						


	if (VDBMgr::GetActivityManager() != nil)
		VDBMgr::GetActivityManager()->SetActivityWrite(false, 0);
	/*
	if ( NULL != systray )
	systray->SetActivityWrite_Toggle(false);
	*/

	VDBMgr::GetManager()->FlushAllData();

	context.StopLowLevelMode();

}


void VDBFlushMgr::SleepFor(sLONG nbmilliseconds)
{
	fDemandeDeFlush.ResetAndLock();
	fDemandeDeFlush.Wait(nbmilliseconds);
}


void VDBFlushMgr::WakeUp()
{
	fDemandeDeFlush.Unlock();
}




VArrayWithHolesOf<BaseFlushInfo*> VDBFlushMgr::sTableFlushinfo;

BaseFlushInfo* VDBFlushMgr::GetFlushInfo(BaseFlushInfoIndex n)
{
	return sTableFlushinfo[n];
}

void VDBFlushMgr::FreeFlushInfoID(BaseFlushInfoIndex n)
{
	sTableFlushinfo.FreeHole(n);
}


//=================

void VDBFlushTask::DoInit()
{
	timeupdate = VSystem::GetCurrentTicks();
	fFlushMgr->xSetFlushTaskReady( true);

	// on attend que notre createur soit completement initialise
	Sleep( 500L);
}

void VDBFlushTask::DoDeInit()
{
	fFlushMgr->xSetFlushTaskReady( false);
}


void VDBFlushTask::DoPrepareToDie()
{
	fFlushMgr->WakeUp();
}



Boolean VDBFlushTask::DoRun()
{
	Boolean cont = true;

	VDBMgr::GetManager()->StoreStructuresAsXML();

	/*  moved to DB4D CRON
	VDBMgr::GetCacheManager()->ClearWaitListForBigObject();
	VDBMgr::GetManager()->PurgeExpiredDataSets();
	VDBMgr::GetManager()->ReleaseExpiredUAGSessions();
	*/
	if (fFlushMgr->IsAskingFlush())
	{
		fFlushMgr->MainLoop();
		if (GetState() < TS_DYING)
		{
			//Sleep(1000);
			fFlushMgr->SleepFor(1000);
		}
	}
	else
	{
		if( GetState() < TS_DYING || fFlushMgr->GetDirty()) 
		{
			if ( fFlushMgr->GetDirty() && (fFlushMgr->IsTimeToFlush() || (GetState() >= TS_DYING)) ) 
			{
				fFlushMgr->MainLoop();
			}
			else
			{
				BtreeFlush::AllocateBtreeFlush();
			}
			if (GetState() < TS_DYING)
			{
				//Sleep(1000);
				fFlushMgr->SleepFor(1000);
			}
		}
		else 
			cont = false;
	}

	return cont;
}

