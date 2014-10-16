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
Boolean debug_DisplayCacheTree = false;

const char* NamesOfType[] = {"t_Obj4D", "t_ObjCache", "t_Base4D", "t_Datafile", "t_bittable", "t_addrtab", "t_addrdefictable",
"t_ficheondisk", "t_treeinmem", "t_treedeficinmem", "t_segdata", "t_addrindtable", "t_btreepageindex",
"t_IndexHeaderBTree", "t_addrdeblobtable", "t_blob4d", "t_treedeblobinmem",
"t_addrdeselclustertable", "t_treedeselclusterinmem", "t_segdatapagesec", "t_btreepagefixsizeindex",
"t_IndexHeaderBTreeFixSize", "t_locker", "t_tabledef", "t_relationdef", "t_petitesel", "t_longsel", "t_bitsel"};

#endif

#if 0
ObjCache** BtreeCache::switchobj(ObjCache* obj, sLONG k)
{
	ObjCache **result, **obj2addr;
	uBOOL sup;
	
	occupe();
	
	if (k==0)
	{
		result=nil;
	}
	else
	{
		result=tabmem+(k-1);
	}
	
	k++;
	sup=true;
	while ((k<nkeys) && sup)
	{
		if (tabmem[k-1]==nil)
		{
			if (! obj->InfAcces(tabordre[k-1]))
			{
				sup=false;
				if (tabsous[k-1]!=nil)
				{
					obj2addr=tabsous[k-1]->switchobj(obj,0);
					if (obj2addr!=nil) result=obj2addr;
				}
			}
			else
			{
				result=tabmem+(k-1);
			}
		}
		else
		{
			if (tabmem[k-1]->SupAcces(obj))
			{
				sup=false;
				if (tabsous[k-1]!=nil)
				{
					obj2addr=tabsous[k-1]->switchobj(obj,0);
					if (obj2addr!=nil) result=obj2addr;
				}
			}
			else
			{
				result=tabmem+(k-1);
			}
		}
		k++;
	}
	
	libere();
	return(result);
}
#endif


sLONG gCacheIsWaitingFlush = 0;

void BtreeCache::search(uBOOL *h, BTitemCache *v, sLONG level)
{
	BtreeCache *sousBT;
	sLONG l,k=0,r,i;
	uBOOL remplace,isegal;
	ObjCache* obj2,*obj;
	BTitemCache u;
	BtreeCache **p,**pb;
	ObjCache  **p2,**p2b;
	sLONG *p3,*p3b;
	uBOOL *p4,*p4b;
	
	occupe();
	
	if (level>20)
	{
		VDBMgr::GetCacheManager()->xDoisEquilibrer();
		xbox_assert( level <= 45);
	}
	
	l=1;
	r=nkeys;
	isegal=false;
	remplace=false;
	obj=v->obj;
	
	while ( (!isegal) && (r>=l) )
	{
		k=(r+l)>>1;
		if (tabpris[k-1])
		{
			obj2=tabmem[k-1];
		}
		else
		{
			obj2=nil;
		}
		if (obj2==obj)
		{
			isegal=true;
		}
		else
		{
			if (obj2==nil)
			{
				if (obj->EgalAccesPtr(tabordre[k-1],tabmem[k-1]))
				{
					isegal=true;
					remplace=true;
					*h=false;
					xbox_assert( k != 0);
					tabmem[k-1]=obj;
					tabordre[k-1]=obj->GetNbAcces();
					tabpris[k-1]=true;
				}
				else
				{
					if (obj->SupAccesPtr(tabordre[k-1],tabmem[k-1]))
					{
						l=k+1;
					}
					else
					{
						r=k-1;
					}
				}
			}
			else
			{
				if (obj->SupAcces(obj2))
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
		if (obj->IsPourDeleteCache())
		{
			tabordre[k-1]=obj->GetNbAcces();
			// tabmem[k-1]=nil; 
			// il faut garder l'adresse pour le test des sup ptr
			tabpris[k-1]=false;
		}
		else
		{
			if (!remplace)
			{
				xbox_assert( false);	// "pourquoi on remplace dans cache"
				#if 0
				vDebugStr("pourquoi on remplace dans cache");
				#endif

			#if 0
				oldacces=obj->GetNbAcces();
				obj->IncNbAcces();
				if (obj->SupAcces(oldacces))
				{
					obj2addr=switchobj(obj,k);
					obj2=*obj2addr;
					*obj2addr=obj;
					tabmem[k-1]=obj2;
					if (obj2==nil)
					{
						tabordre[k-1]=oldacces;
					}
					else
					{
						tabordre[k-1]=obj2->GetNbAcces();
					}
				}
				else
				{
					tabordre[k-1]=obj->GetNbAcces();
				}
			#endif

			}
		}
	}
	else
	{
		sousBT=tabsous[r];
		if (sousBT!=nil)
		{
			u=*v;
			/*/ ?? faut il remettre u.sousBT a nil */
			sousBT->search(h,&u,level+1);
		}
		else
		{
			if (obj->IsPourDeleteCache())
			{
				*h=false;
			}
			else
			{
				if ((k>0) && (k<=nkeys) && !tabpris[k-1])
				{
					remplace=true;
					*h=false;
					xbox_assert( k != 0);
					tabmem[k-1]=obj;
					tabordre[k-1]=obj->GetNbAcces();
					tabpris[k-1]=true;
				}
				else
				{
					*h=true;
					u=*v;
				}
			}
		}
		
		if (*h)
		{
			if (nkeys<kNbElemInTreeCache)
			{
				*h=false;
				p=tabsous+nkeys+1;
				p2=tabmem+nkeys;
				p3=tabordre+nkeys;
				p4=tabpris+nkeys;
				for (i=r;i<nkeys;i++)
				{
					*p=*(p-1);
					p--;
					xbox_assert(p2 >= tabmem);
					*p2=*(p2-1);
					--p2;
					*p3=*(p3-1);
					--p3;
					*p4=*(p4-1);
					--p4;
				}
				tabsous[r+1]=u.sousBT;
				xbox_assert( r >= 0);
				tabmem[r]=u.obj;
				tabordre[r]=u.ordre;
				tabpris[r]=u.pris;
				nkeys++;
			}
			else
			{
				sousBT = VDBMgr::GetCacheManager()->xNextFreeCachePage();
				if (r<=kHalfTreeCache)
				{
					if (r==kHalfTreeCache)
					{
						*v=u;
					}
					else
					{
						v->obj=tabmem[kHalfTreeCache-1];
						v->sousBT=tabsous[kHalfTreeCache];
						v->ordre=tabordre[kHalfTreeCache-1];
						v->pris=tabpris[kHalfTreeCache-1];
						p=tabsous+kHalfTreeCache;
						p2=tabmem+kHalfTreeCache-1;
						p3=tabordre+kHalfTreeCache-1;
						p4=tabpris+kHalfTreeCache-1;
						
						for (i=r+1;i<kHalfTreeCache;i++)
						{
							*p=*(p-1);
							p--;

							xbox_assert( p2 >= tabmem);

							*p2=*(p2-1);
							--p2;
							*p3=*(p3-1);
							--p3;
							*p4=*(p4-1);
							--p4;
						}
						
						xbox_assert( r >= 0);

						tabsous[r+1]=u.sousBT;
						tabmem[r]=u.obj;
						tabordre[r]=u.ordre;
						tabpris[r]=u.pris;
					}
					
					p=tabsous+kHalfTreeCache+1;
					p2=tabmem+kHalfTreeCache;
					p3=tabordre+kHalfTreeCache;
					p4=tabpris+kHalfTreeCache;
					pb=sousBT->tabsous+1;
					p2b=sousBT->tabmem;
					p3b=sousBT->tabordre;
					p4b=sousBT->tabpris;
					for (i=1;i<=kHalfTreeCache;i++)
					{
						*pb++=*p++;
						*p2b++=*p2++;
						*p3b++=*p3++;
						*p4b++=*p4++;
					}
				}
				else
				{
					r=r-kHalfTreeCache;
					v->obj=tabmem[kHalfTreeCache];
					v->sousBT=tabsous[kHalfTreeCache+1];
					v->ordre=tabordre[kHalfTreeCache];
					v->pris=tabpris[kHalfTreeCache];
					
					p=tabsous+kHalfTreeCache+2;
					p2=tabmem+kHalfTreeCache+1;
					p3=tabordre+kHalfTreeCache+1;
					p4=tabpris+kHalfTreeCache+1;
					pb=sousBT->tabsous+1;
					p2b=sousBT->tabmem;
					p3b=sousBT->tabordre;
					p4b=sousBT->tabpris;
					for (i=1;i<r;i++)
					{
						*pb++=*p++;
						*p2b++=*p2++;
						*p3b++=*p3++;
						*p4b++=*p4++;
					}
					
					xbox_assert( r >= 0);
					sousBT->tabsous[r]=u.sousBT;
					sousBT->tabmem[r-1]=u.obj;
					sousBT->tabordre[r-1]=u.ordre;
					sousBT->tabpris[r-1]=u.pris;
					
					p=tabsous+kHalfTreeCache+r+1;
					p2=tabmem+kHalfTreeCache+r;
					p3=tabordre+kHalfTreeCache+r;
					p4=tabpris+kHalfTreeCache+r;
					pb=sousBT->tabsous+1+r;
					p2b=sousBT->tabmem+r;
					p3b=sousBT->tabordre+r;
					p4b=sousBT->tabpris+r;
					for (i=r+1;i<=16;i++)
					{
						*pb++=*p++;
						*p2b++=*p2++;
						*p3b++=*p3++;
						*p4b++=*p4++;
					}
				}
				
				nkeys=kHalfTreeCache;
				sousBT->nkeys=kHalfTreeCache;
				sousBT->tabsous[0]=v->sousBT;
				v->sousBT=sousBT;

				// CHECKCACHEDUP;
			}
		}
	}
	libere();
}


void BtreeCache::Equilibre(void)
{
	BtreeCache *sousbt;
	sLONG i;
	
	sousbt=tabsous[0];
	if (sousbt!=nil)
	{
		sousbt->Equilibre();
		tabsous[0]=nil;
		delete sousbt;
	}

	for (i=0;(i<nkeys);i++)
	{
		if (tabpris[i])
		{
			VDBMgr::PutObjectInCache(tabmem[i],false);
		}
		sousbt=tabsous[i+1];
		if (sousbt!=nil)
		{
			sousbt->Equilibre();
			tabsous[i+1]=nil;
			delete sousbt;
		}
	}
}

uBOOL BtreeCache::ViderCache(sLONG allocationBlockNumber, VSize need, VSize *tot, sLONG level, sLONG& ioCountFreedObjects, sLONG& ioDebugcurcompteobj, sLONG& outRemaining,
							 Boolean& BigObjectsNeedToBeFreedByOtherTask)
{
	BtreeCache *sousbt;
	uBOOL toutvide;
	ObjCache *obj;
	sLONG tot2,i;
	Boolean aumoinsunenfant = false, aumoinsuntrou = false;
	BtreeCache avanttouche = *this;

	Boolean debug_1 = false, debug_2 = false;
	
	sLONG subremaining[kNbElemInTreeCache+1];
	xbox_assert( level <= 45);

	toutvide=true;
	std::fill(&subremaining[0], &subremaining[nkeys+1], 0);
	
	//CHECKCACHE;

	sousbt=tabsous[0];
	if (sousbt!=nil)
	{
		if (sousbt->ViderCache(allocationBlockNumber, need,tot,level+1,ioCountFreedObjects,ioDebugcurcompteobj, subremaining[0], BigObjectsNeedToBeFreedByOtherTask))
		{
			xbox_assert(subremaining[0] == 0);
			//CHECKCACHE;
			tabsous[0]=nil;
			//CHECKCACHEFIND(sousbt);
			delete sousbt;
		}
		else
		{
			outRemaining = outRemaining + subremaining[0];
			aumoinsunenfant = true;
			toutvide=false;
		}
	}

	for (i=0;(i<nkeys) /*&& (*tot<need)*/;i++)
	{
		if (tabpris[i])
		{
			obj=tabmem[i];
		}
		else
		{
			obj=nil;
			aumoinsuntrou = true;
		}
		sousbt=tabsous[i+1];
		if (obj!=nil)
		{
			if (!obj->IsOkToFreeMem())
			{
				if (obj->isBigObject())
					BigObjectsNeedToBeFreedByOtherTask = true;
				toutvide=false;
				outRemaining++;
			}
			else
			{
				if (obj->okToLiberemem())
				{
					#if journalcache
					Jmess((uCHAR*)"Liberemem : ",obj);
					#endif
					
					tot2=obj->liberemem(allocationBlockNumber, /*(sLONG)need*/ kMaxPositif,false);
					//CHECKCACHE;
					if (tot2<0)
					{
						toutvide=false;
						outRemaining++;
						obj->ReleaseFreeMem();
						*tot = *tot + (VSize)(-tot2);
					}
					else
					{
						if (tot2>2)
						{
							*tot=*tot+tot2;
						}
						if ((tot2>0) && obj->okdel())
						{
							ioCountFreedObjects++;
							tabordre[i]=obj->GetNbAcces();
							obj->PourDeleteCache(true);
							obj->ReleaseFreeMem(); // on peut le liberer car il n'est plus reference par son parent (liberemem s'en est charge)
							obj->Kill();
							tabpris[i]=false;
							aumoinsuntrou = true;
							#if VERSIONDEBUG
							ioDebugcurcompteobj--;
							#endif
							//CHECKCACHE;
						}
						else
						{
							toutvide=false;
							outRemaining++;
							obj->ReleaseFreeMem();
						}
					}
				}
				else 
				{
					toutvide=false;
					outRemaining++;
					obj->ReleaseFreeMem();
				}
			}
		}
		if (sousbt!=nil)
		{
			if (sousbt->ViderCache(allocationBlockNumber, need,tot,level+1,ioCountFreedObjects,ioDebugcurcompteobj, subremaining[i+1], BigObjectsNeedToBeFreedByOtherTask))
			{
				//CHECKCACHE;
				xbox_assert(subremaining[i+1] == 0);
				tabsous[i+1]=nil;
				//xbox_assert( (ioDebugcurcompteobj-20) <= VDBMgr::GetCacheManager()->CompteObj());
				//CHECKCACHEFIND(sousbt);
				delete sousbt;
			}
			else
			{
				outRemaining = outRemaining + subremaining[i+1];
				aumoinsunenfant = true;
				//CHECKCACHE;
				toutvide=false;
			}
		}
	}

#if 0 && VERSIONDEBUG
	sLONG oldsubremaining[kNbElemInTreeCache+1];
	vBlockMove(subremaining, oldsubremaining, sizeof(subremaining));
#endif

	if (!toutvide && aumoinsuntrou)
	{
		debug_1 = true;
		sLONG newnkeys = 0;
		for (i = 0; i<nkeys; i++)
		{
			if (tabpris[i] || (tabsous[i+1] != nil))
			{
				tabpris[newnkeys] = tabpris[i];
				tabordre[newnkeys] = tabordre[i];
				tabmem[newnkeys] = tabmem[i];
				tabsous[newnkeys+1] = tabsous[i+1];
				subremaining[newnkeys+1] = subremaining[i+1];
				newnkeys++;
			}
		}
		nkeys = newnkeys;
	}

	if (aumoinsunenfant && nkeys < kNbElemInTreeCache)
	{
		debug_2 = true;
		for (i=0; i<=nkeys; i++)
		{
			sousbt = tabsous[i];
			sLONG subremain = subremaining[i];
			if (sousbt != nil && subremain > 0)
			{
				sLONG plusun = 0;
				if (i>0)
				{
					if (!tabpris[i-1])
					{
						plusun = 1;
					}
				}
				if (subremain <= (kNbElemInTreeCache+plusun-nkeys))
				{
					sLONG off = subremain-plusun, i2 = i-plusun;
					std::copy_backward(&tabmem[i2], &tabmem[nkeys], &tabmem[nkeys+off]);
					std::copy_backward(&tabordre[i2], &tabordre[nkeys], &tabordre[nkeys+off]);
					std::copy_backward(&tabpris[i2], &tabpris[nkeys], &tabpris[nkeys+off]);
					std::copy_backward(&tabsous[i2+1], &tabsous[nkeys+1], &tabsous[nkeys+1+off]);
					std::copy_backward(&subremaining[i2+1], &subremaining[nkeys+1], &subremaining[nkeys+1+off]);
					tabsous[i] = nil;
					nkeys = nkeys + off; // la limite de la boucle for est volontairement repoussee
					sLONG pos = i2;
					sousbt->SubCopyInto(this, pos);
					xbox_assert((pos-i2) == subremain);
					delete sousbt;
				}
			}
		}
	}

	xbox_assert(nkeys <= kNbElemInTreeCache);
#if VERSIONDEBUG
	if (!toutvide && *tot < need)
	{
		sLONG fincompte = 0;
		for (i=0; i<nkeys; i++)
		{
			if (tabpris[i] || tabsous[i+1] != nil)
				fincompte++;
		}
		xbox_assert(fincompte == nkeys);
	}
#endif

#if 0
	VString debug_s;
	DebugMsg(L"sortie pour ");
	debug_s.FromLong((sLONG )this);
	DebugMsg(debug_s);
	DebugMsg(L"  :  nkeys = ");
	debug_s.FromLong(nkeys);
	DebugMsg(debug_s);
	DebugMsg(L"  :  remaining objects = ");
	debug_s.FromLong(outRemaining);
	DebugMsg(debug_s);
	DebugMsg(L"  :  freed bytes = ");
	debug_s.FromLong(*tot);
	DebugMsg(debug_s);
	DebugMsg(L"\n");
#endif

	//CHECKCACHE;
	if (*tot>=need)
	{
		toutvide=false;
		outRemaining = kMaxNeedMem;
	}
	return(toutvide);
}


void BtreeCache::SubCopyInto(BtreeCache* into, sLONG& startPos)
{
	BtreeCache* sousBT = tabsous[0];

	if (sousBT != nil)
	{
		sousBT->SubCopyInto(into, startPos);
		delete sousBT;
	}

	for (sLONG i = 0; i < nkeys; i++)
	{
		sousBT = tabsous[i+1];
		if (tabpris[i])
		{
			xbox_assert(startPos < into->nkeys);
			into->tabpris[startPos] = true;
			into->tabordre[startPos] = tabordre[i];
			into->tabmem[startPos] = tabmem[i];
			into->tabsous[startPos+1] = nil;
			startPos++;
		}

		if (sousBT != nil)
		{
			sousBT->SubCopyInto(into, startPos);
			delete sousBT;
		}

	}

}




sLONG BtreeCache::CompteObj(void)
{
	sLONG res;
	BtreeCache *sousbt;
	sLONG i;
	
	res=0;
	
	sousbt=tabsous[0];
	if (sousbt!=nil)
	{
		res=res+sousbt->CompteObj();
	}

	for (i=0;(i<nkeys);i++)
	{
		if (tabpris[i])
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

void BtreeCache::BuildCacheStat(debug_CacheObjRef& stats)
{
	sLONG i;

	for (i=0;(i<nkeys);i++)
	{
		ObjCache* obj = nil;
		if (tabpris[i])
		{
			obj=tabmem[i];
		}
		if (obj != nil)
		{
			stats[(sLONG)obj->GetType()]++;
		}
	}

	for (i=0;(i<=nkeys);i++)
	{
		BtreeCache* soust = tabsous[i];
		if (soust != nil)
			soust->BuildCacheStat(stats);
	}
}


uBOOL BtreeCache::CheckObjInMem(void)
{
	sLONG i;
	
	testAssert(IsValidFastPtr(this));
	testAssert(nkeys>=0 && nkeys<=kNbElemInTreeCache);
	for (i=0;(i<=nkeys);i++)
	{
		testAssert(IsValidFastPtr(tabsous[i]));
	}
	return(true);
}


Boolean BtreeCache::fCheckCacheWithFlush = false;

void BtreeCache::CheckPage(sLONG *curval)
{
	BtreeCache *sousbt;
	ObjCache *obj;
	sLONG i;
	VName vs;
	
	xbox_assert( *((sLONG*)this) != 0);
	
	sousbt=tabsous[0];
	if (sousbt!=nil)
	{
		sousbt->CheckPage(curval);
	}

	for (i=0;(i<nkeys);i++)
	{
		if (tabpris[i])
		{
			obj=tabmem[i];
		}
		else
		{
			obj=nil;
		}
		
		if (obj != nil)
		{
			if (fCheckCacheWithFlush)
			{
				if (!obj->occupe())
				{
					if (obj->modifieonly())
					{
						xbox_assert(VDBMgr::GetFlushManager()->FindObjInFlush(obj));
					}
					obj->libere();
				}
			}
			
			if (!obj->CheckObjInMem())
			{
				obj->DebugGetClassName(vs);
				WriteLogStr("Bad Object in mem : ");
				WriteLnLogVStr(vs);
			}
		}
		
		/*
		if (obj!=nil)
		{
			if (!obj->EgalAcces(tabordre[i]))
			{
				vBreak();
			}
		}
		if ( (tabordre[i]>>5) < ((*curval)>>5) )
		{
				vBreak();
		}
		*curval=tabordre[i];
		*/
		sousbt=tabsous[i+1];
		if (sousbt!=nil)
		{
			sousbt->CheckPage(curval);
		}
	}
}

void BtreeCache::DisplayTree(void)
{
	BtreeCache *sousbt;
	ObjCache *obj;
	sLONG i;
	VName vs;
	
	sousbt=tabsous[0];
	if (sousbt!=nil)
	{
		sousbt->DisplayTree();
	}

	for (i=0;(i<nkeys);i++)
	{
		if (tabpris[i])
		{
			obj=tabmem[i];
		}
		else
		{
			obj=nil;
		}
		
		if (obj != nil)
		{
			obj->DebugGetClassName(vs);
			WriteLnLogVStr(vs);
		}
		sousbt=tabsous[i+1];
		if (sousbt!=nil)
		{
			sousbt->DisplayTree();
		}
	}
}


void BtreeCache::CheckPageIndexFixSize(void)
{
	BtreeCache *sousbt;
	ObjCache *obj;
	sLONG i;
	VName vs;
	
	sousbt=tabsous[0];
	if (sousbt!=nil)
	{
		sousbt->CheckPageIndexFixSize();
	}

	for (i=0;(i<nkeys);i++)
	{
		if (tabpris[i])
		{
			obj=tabmem[i];
		}
		else
		{
			obj=nil;
		}
		
		if (obj != nil)
		{
#if debuglrWithTypObj
			if (obj->GetType() == t_btreepagefixsizeindex)
			{
				//((BTreePageFixSizeIndex*)obj)->checktabmem();
			}
#endif
		}
		sousbt=tabsous[i+1];
		if (sousbt!=nil)
		{
			sousbt->CheckPageIndexFixSize();
		}
	}
}


void BtreeCache::DebugFindPage(BtreeCache *sbt)
{
	BtreeCache *sousbt;
	sLONG i;
	
	if (this == sbt)
	{
		DebugMsg( "found!");
	}
	
	sousbt=tabsous[0];
	if (sousbt!=nil)
	{
		sousbt->DebugFindPage(sbt);
	}

	for (i=0;(i<nkeys);i++)
	{
		sousbt=tabsous[i+1];
		if (sousbt!=nil)
		{
			sousbt->DebugFindPage(sbt);
		}
	}
}




void BtreeCache::CheckDupPage(void)
{
	BtreeCache *sousbt;
	sLONG i;
	
	sousbt=tabsous[0];
	if (sousbt!=nil)
	{
		tabsous[0]=nil;
		DebugFindPage( sousbt);
		tabsous[0]=sousbt;
		sousbt->CheckDupPage();
	}

	for (i=0;(i<nkeys);i++)
	{
		sousbt=tabsous[i+1];
		if (sousbt!=nil)
		{
			tabsous[i+1]=nil;
			DebugFindPage( sousbt);
			tabsous[i+1]=sousbt;
			sousbt->CheckDupPage();
		}
	}
}


sLONG BtreeCache::CompteAddr(DataAddr4D xaddr)
{
	sLONG res;
	BtreeCache *sousbt;
	sLONG i;
	ObjCache *obj;
	
	res=0;
	
	sousbt=tabsous[0];
	if (sousbt!=nil)
	{
		res=res+sousbt->CompteAddr(xaddr);
	}

	for (i=0;(i<nkeys);i++)
	{
		if (tabpris[i])
		{
			obj=tabmem[i];
			if (obj->getaddr()==xaddr)
			{
				res++;
			}
		}
		sousbt=tabsous[i+1];
		if (sousbt!=nil)
		{
			res=res+sousbt->CompteAddr(xaddr);
		}
	}
	
	return(res);
}
#endif

void BtreeCache::xInitFromItem( const BTitemCache& u, BtreeCache *sousbt)
{
	tabsous[0]=sousbt;
	tabsous[1]=u.sousBT;
	tabmem[0]=u.obj;
	tabordre[0]=u.ordre;
	tabpris[0]=u.pris;
	nkeys=1;
}

VDBCacheMgr::VDBCacheMgr( VDBMgr *inManager):fCache( VCppMemMgr::kAllocator_xbox, false, false)
{	
	ObjAlmostInCache::InitMasks();
	#if VERSIONDEBUG
	fCheckCache = false;
	#endif
	
	fManager = inManager;

	fPeutEquilibrer = false;
	fDoisEquilibrer = false;

	fNeedsFreePages = true;
	memset( fTabFreePages, 0, sizeof( fTabFreePages));
		
	fTree = nil;

	fMaxAlloc = kMinimumCache;
	fMinSizeToFlush = (fMaxAlloc / 100) * 10;
	fCache.SetPurgeHandlerProc( CallNeedsBytes);
	fCache.AddVirtualAllocation(fMaxAlloc, NULL, false);
	fCache.SetWithLeaksCheck( false); // force it to false since there's currently no way to dump the cache leaks
	fCache.SetAutoAllocationState( false);
	
	xbox_assert( gAlternateCppMem == NULL);
	gAlternateCppMem = &fCache;
	VObject::SetAlternateAllocator(gAlternateCppMem);

	//fNeedsByteSyncEvent = nil;
	fWaitingForOtherToFreeBigObjects = nil;
	fWaitingForOtherToFreeBigObjectsStamp = 0;
	fFreedMem = -1;
	fNeededMem = -1;
	fCurBigObjToFreeisValid = false;
}

VDBCacheMgr::~VDBCacheMgr()
{
	xbox_assert( gAlternateCppMem == &fCache);
	gAlternateCppMem = NULL;
}


VError VDBCacheMgr::SetMaxSize( VSize inSize, bool inPhysicalMemOnly, VSize *outMaxSize)
{
	VError err = VE_OK;
#if ARCH_32
	if (inSize > (VSize)2500000000)
		inSize = (VSize)2500000000;
#endif
	if (inSize > fMaxAlloc)
	{
		bool ok = fCache.AddVirtualAllocation( inSize - fMaxAlloc, NULL, inPhysicalMemOnly);
		fCache.SetAutoAllocationState( false);
		fMaxAlloc = fCache.GetAllocatedMem();
		if (!ok)
		{
			err = ThrowBaseError(memfull);
		}
	}
	if (outMaxSize)
		*outMaxSize = fMaxAlloc;

	return err;
}

/*
VError VDBCacheMgr::SetMaxSize( VSize inSize, bool inPhysicalMemOnly, VSize *outMaxSize)
{
	VError err = VE_OK;
#if ARCH_32
	if (inSize > (VSize)2500000000)
		inSize = (VSize)2500000000;
#endif
	if (inSize > fMaxAlloc)
	{
		// first try allocation in one block
		bool ok = fCache.AddVirtualAllocation( inSize - fMaxAlloc, NULL, inPhysicalMemOnly);
		if (ok)
		{
			fMaxAlloc = inSize;
			fCache.SetAutoAllocationState( false);
		}
		else
		{
			bool retry;
			
			// min size for cache blocks.
			const VSize kMinAllocationSize = 50*1024*1024;	// 50 Mo
			
			// no more than 10 cache blocks
			const VIndex kMaxAllocationCount = 10;
			
			VIndex allocationCount = fCache.CountVirtualAllocations();
			
			do
			{
				retry = false;
				
				// get all available VM blocks
				typedef std::multimap<VSize,const void*> MapOfAddressBySize;
				MapOfAddressBySize vm_mapBySize;
				const void *address = NULL;
				VSize cumulatedFreeVirtualSize = 0;
				
				do {
					VSize size;
					const void *baseAddress;
					VMStatus status;
					if (!VSystem::VirtualQuery( address, &size, &status, &baseAddress))
						break;
					if (status & VMS_FreeMemory)
					{
						cumulatedFreeVirtualSize += size;
						vm_mapBySize.insert( MapOfAddressBySize::value_type( size, baseAddress));
					}
					address = (const char*) baseAddress + size;
				} while( true);
				
				#if VERSIONDEBUG
				DebugMsg( "Virtual Memory Free Blocks Dump\n");
				for( MapOfAddressBySize::const_iterator i = vm_mapBySize.begin() ; i != vm_mapBySize.end() ; ++i)
				{
					DebugMsg( "0x%p\t%ld\n", i->second, i->first);
				}
				DebugMsg( "Cumulated free virtual space = %ld\n", cumulatedFreeVirtualSize);
				#endif
				
				// allocate all blocks in decreasing order
				while( (fMaxAlloc < inSize) && (allocationCount < kMaxAllocationCount) )
				{
					// look for first block greater than what is needed
					VSize allocationSize = 0;
					const void *allocationAddress = NULL;
					
					VSize needed = inSize - fMaxAlloc;
					MapOfAddressBySize::iterator i = vm_mapBySize.lower_bound( needed);
					if (i != vm_mapBySize.end())
					{
						allocationSize = i->first;
						allocationAddress = i->second;
						vm_mapBySize.erase( i);
					}
					else if (!vm_mapBySize.empty())
					{
						// no big enough block is available. Let's take the biggest one.
						// there's no std::map::back()
						i = vm_mapBySize.end();
						--i;
						allocationSize = i->first;
						allocationAddress = i->second;
						vm_mapBySize.erase( i);
					}
					
					if (allocationSize > kMinAllocationSize)	// min size for allocations
					{
						// if the block is bigger than what is needed, truncate it.
						// but if the remaining block is small (< 1Mo) let's take it.
						if (allocationSize > needed + 1024*1204)
							allocationSize = needed;

						do
						{
							ok = fCache.AddVirtualAllocation( allocationSize, allocationAddress, inPhysicalMemOnly);
							if (ok)
							{
								fMaxAlloc += allocationSize;
								allocationCount++;
								break;
							}
							else
							{
								// the block has been advertized as free but its allocation failed.
								// this may due to the fact that the system had to allocate a small bit of our block in the mean time.
								// retry a bit further.
								VSize chunk = VSystem::GetVMPageSize();
								if (allocationSize < kMinAllocationSize)
								{
									break;
								}

								allocationSize -= chunk;
								allocationAddress = (char*) allocationAddress + chunk;
							}
						} while( allocationCount < kMaxAllocationCount);
					}
					else
					{
						break;
					}
				}
			} while( retry);
			
			if (fMaxAlloc < inSize)
				err = ThrowBaseError(memfull, DBaction_ChangeCacheSize);
			else
				fCache.SetAutoAllocationState( false);
		}
	}
	
	fCache.SetAutoAllocationState( false);
	if (outMaxSize)
		*outMaxSize = fMaxAlloc;
	
	return err;
}
*/


VError VDBCacheMgr::SetMinSizeToFlush( VSize inMinSizeToFlush)
{
	if ( (inMinSizeToFlush>0) )
	{
		if (inMinSizeToFlush<0x10000)
			inMinSizeToFlush = 0x10000;
		fMinSizeToFlush = inMinSizeToFlush;
	}
	return VE_OK;
}


VDBCacheMgr *VDBCacheMgr::NewCacheManager( VDBMgr *inManager)
{
	VDBCacheMgr *manager = new VDBCacheMgr( inManager);
	/*  deplace plus loin pour que VDBMgr::fCacheMgr soit bien initialise   L.R le 6 fev 2004

	if (!manager->Init()) {
		delete manager;
		manager = nil;
	}
	*/
	return manager;
}


Boolean VDBCacheMgr::Init()
{
	xCheckFreeCachePages();

	return true;
}

BtreeCache* VDBCacheMgr::xNextFreeCachePage()
{
	BtreeCache* bc = nil;
	
	fNeedsFreePages = true;
	
	for( sLONG i=0;i<kMaxFreePages;i++)
	{
		if (fTabFreePages[i]!=nil)
		{
			bc=fTabFreePages[i];
			fTabFreePages[i]=nil;
			break;
		}
	}
	xbox_assert( bc != nil);	// "Grave : plus de FreeCachePage"
	return bc;
}


#if VERSIONDEBUG
void VDBCacheMgr::xTestAddr(DataAddr4D inAddr)
{
	
	if ( (fTree!=nil) && (inAddr!=0) )
	{
		sLONG n = fTree->CompteAddr(inAddr);
		xbox_assert( n <= 1);
	}
}

void VDBCacheMgr::Dump()
{
	VTaskLock lock( &fMutex);
	if (fTree != nil)
		fTree->DisplayTree();
}


void VDBCacheMgr::Check()
{
	if (fCheckCache) {
		VTaskLock lock( &fMutex);
		sLONG val=0;
		if (fTree!=nil)
			fTree->CheckPage(&val);
	}
}

void VDBCacheMgr::CheckDup()
{
	if (fCheckCache) {
		VTaskLock lock( &fMutex);
		if (fTree!=nil)
			fTree->CheckDupPage();
	}
}


void VDBCacheMgr::CheckPageIndexFixSize()
{
		VTaskLock lock( &fMutex);
		if (fTree!=nil)
			fTree->CheckPageIndexFixSize();
}

void VDBCacheMgr::DebugFindPage(BtreeCache *inPage)
{
	VTaskLock lock( &fMutex);
	if (fTree!=nil)
		fTree->DebugFindPage(inPage);
}



sLONG VDBCacheMgr::CompteObj()
{
	VTaskLock lock( &fMutex);
	return (fTree == nil) ? 0 : fTree->CompteObj();
}


void VDBCacheMgr::DisplayCacheStat(Boolean before)
{
	if (debug_DisplayCacheTree)
	{
		VTaskLock lock( &fMutex);
		debug_CacheObjRef stats;

		DebugMsg(L"\n");
		DebugMsg(L"\n");
		if (before)
			DebugMsg(L"Avant Free cache \n");
		else
			DebugMsg(L"Apres Free cache \n");


		if (fTree != nil)
			fTree->BuildCacheStat(stats);

		debug_CacheObjRef::iterator p = stats.begin();
		debug_CacheObjRef::iterator theend = stats.end();
		while (p != theend)
		{
			VString s;
			s.FromCString(NamesOfType[(*p).first]);
			DebugMsg(s);
			DebugMsg(L" : ");
			s.FromLong((*p).second);
			DebugMsg(s);
			DebugMsg(L"\n");
			p++;
		}
		DebugMsg(L"\n");
		DebugMsg(L"\n");
	}
}


#endif


void* VDBCacheMgr::NewPtr( VSize inNbBytes, Boolean inIsVObject, sLONG inTag, sLONG preferedBlock)
{
	return fCache.NewPtr( inNbBytes, inIsVObject, inTag, preferedBlock);
}


void VDBCacheMgr::DisposePtr( void *inBlock)	
{
	fCache.DisposePtr( (VPtr) inBlock);
}


void VDBCacheMgr::xCheckFreeCachePages()
{
	StAllocateInCache alloc;

	if (fNeedsFreePages)
	{
		fNeedsFreePages = false;
		for (sLONG i=0;i<kMaxFreePages;i++)
		{
			if (fTabFreePages[i]==nil)
				fTabFreePages[i] = new BtreeCache;
		}
	}
}

void VDBCacheMgr::ClearWaitListForBigObject()
{
	fNeedsByteMutex.Lock();
	if (fWaitingForOtherToFreeBigObjects != nil)
	{
		fWaitingForOtherToFreeBigObjects->Unlock();
		fWaitingForOtherToFreeBigObjects = nil;
	}
	fNeedsByteMutex.Unlock();
}


void VDBCacheMgr::CallFreeMemObj(sLONG allocationBlockNumber, VSize inNeededBytes, IObjToFree* obj) // devrait etre obsolete
{
	bool cont = true;
	do
	{
		fNeedsByteMutex.Lock();
		if (fNeedsByteSyncEvent == nil)
		{
			cont = false;
			fNeedsByteSyncEvent = new VSyncEvent;
			fNeedsByteMutex.Unlock();

			gOccPool.StartGarbageCollection();

			VSize freed;
			obj->FreeMem(allocationBlockNumber, inNeededBytes, freed);

			gOccPool.EndGarbageCollection();

			fNeedsByteMutex.Lock();
			VSyncEvent* ev = fNeedsByteSyncEvent;
			fNeedsByteSyncEvent = nil;
			ev->Unlock();
			ev->Release();
			fNeedsByteMutex.Unlock();
		}
		else
		{
			VSyncEvent* ev = fNeedsByteSyncEvent;
			ev->Retain();
			fNeedsByteMutex.Unlock();
			ev->Lock();
			ev->Release();
		}
	} while (cont);
}


VSize VDBCacheMgr::NeedsBytes( sLONG allocationBlockNumber, VSize requiredBytes, bool withFlush)
{
	VCacheLogQuotedEntry	logEntry(eCLEntryKind_NeedsBytes, requiredBytes, true);

//	JS4D::GarbageCollect( NULL);

	// code sale en attendant quelque chose de Stephane
	//if (global_ApplicationRef != nil)
	{
		IDB4D_ApplicationIntf *applicationIntf = VDBMgr::GetManager()->GetApplicationInterface();
		if (applicationIntf != nil)
		{
			applicationIntf->JS4DGarbageCollect();
		}
	}
	// fin du code sale

	if (withFlush)
	{
		VInterlocked::Increment(&gCacheIsWaitingFlush);
		VDBMgr::GetManager()->FlushCache(true, true);
		VInterlocked::Decrement(&gCacheIsWaitingFlush);
	}

	//fNeedsByteMutex.Lock();
	//if (fNeedsByteSyncEvent == nil)
	{
		Boolean BigObjectsNeedToBeFreedByOtherTask = false;
		//fNeedsByteSyncEvent = new VSyncEvent;
		//fNeedsByteMutex.Unlock();
		VSize tot;
		sLONG compte;
		
		tot=0;
		compte=0;
		
		/* juste pour mettre un break
		if (IsCurTaskFlushing())
		{
			tot = compte;
		}
		*/
		//xbox_assert(!IsCurTaskFlushing());

		/*
		if (inNeededBytes<fMinSizeToFlush)
			inNeededBytes = fMinSizeToFlush;
		
		if (fDoisEquilibrer)
		{
			inNeededBytes=kMaxNeedMem;
		}
		*/

		//do
		{
			compte++;
			
			{
				VTaskLock lock( &fMutex);
				
				VTaskMgr::Get()->SetDirectUpdate(false);
				SetCurTaskFaisPlace(true);

				if (fTree!=nil)
				{
					sLONG countFreedObjects = 0;
					//do
					{
	#if 0 && VERSIONDEBUG
						sLONG debugcurcompteobj=fTree->CompteObj();
	#else
						sLONG debugcurcompteobj = 0;
	#endif
						countFreedObjects = 0;
	#if VERSIONDEBUG
						DisplayCacheStat(true);
	#endif

						sLONG remainingobjs = 0;
						if (fTree->ViderCache(allocationBlockNumber, requiredBytes,&tot,0, countFreedObjects, debugcurcompteobj, remainingobjs, BigObjectsNeedToBeFreedByOtherTask))
						{
							xbox_assert(remainingobjs == 0);
							delete fTree;
							fTree=nil;
						}
	#if VERSIONDEBUG
						DisplayCacheStat(false);
	#endif
					}
					//while (countFreedObjects > 0 && (tot<inNeededBytes) && (fTree!=nil));
				}
				
				//if (tot<inNeededBytes)
				{
					gOccPool.StartGarbageCollection();
					CacheList::iterator cur, end ;
					VSize nbbigobj = fBigObjs.size();

					//bool stop = tot>=inNeededBytes;

					bool firstpass = true;

					//do
					{
						/*
						if (firstpass)
						{
							if (!fCurBigObjToFreeisValid)
								fCurBigObjToFree = fBigObjs.begin();
							fCurBigObjToFreeisValid = true;
							cur = fCurBigObjToFree;
							end = fBigObjs.end();
						}
						else
						{
							cur = fBigObjs.begin();
							end = fCurBigObjToFree;
						}
						*/
						cur = fBigObjs.begin();
						end = fBigObjs.end();

						while (cur != end /*&& !stop*/)
						{
							CacheList::iterator next = cur;
							++next;

							IObjToFree* obj = *cur;
							VSize tot2;
							if (obj->FreeMem(allocationBlockNumber, 0, tot2))
							{
								tot = tot + tot2;
							}
							else
							{
								BigObjectsNeedToBeFreedByOtherTask = true;
								obj->RequestMem(allocationBlockNumber, 0);
							}

							/*if (tot >= inNeededBytes)
								stop = true;*/

							cur = next;
						}

						/*
						if (!stop)
						{
							if (firstpass)
								firstpass = false;
							else
								stop = true;
						}
						*/

					} 
					//while (!stop);

					if (cur == end)
						fCurBigObjToFree = fBigObjs.begin();
					else
						fCurBigObjToFree = cur;

					gOccPool.EndGarbageCollection();
				}

				VTaskMgr::Get()->SetDirectUpdate(true);
			
				SetCurTaskFaisPlace(false);
				
			}
			
#if 0
			if ((tot<inNeededBytes) && (compte<2))
			{
				if (IsCurTaskFlushing())
				{
					compte=2;
				}
				else
				{
					VCacheLogQuotedEntry	logEntry(eCLEntryKind_FlushFromNeedsBytes);
					if (!VDBMgr::GetFlushManager()->NeedsBytes( inNeededBytes-tot))
					{
						compte=2;
					}
				}
			}
#if 0
			if ((tot<inNeededBytes) && compte == 2)
			{
				VSize subtot = 0;
				BaseTaskInfo::NeedsBytes(inNeededBytes-tot, subtot);
				tot = tot + subtot;
			}
#endif

			if (tot>=inNeededBytes)
			{
				compte=3;
			}
#endif
		}
		//while (compte<3);
		
		if (fDoisEquilibrer)
		{
			fPeutEquilibrer=true;
			fDoisEquilibrer=false;
		}
		
#if 0
		fNeedsByteMutex.Lock();
		VSyncEvent* ev = fNeedsByteSyncEvent;
		fNeedsByteSyncEvent = nil;
		ev->Unlock();
		ev->Release();
		if (fWaitingForOtherToFreeBigObjects != nil)
		{
			fFreedMem = fFreedMem + tot;
			if (fFreedMem >= fNeededMem)
			{
				fWaitingForOtherToFreeBigObjects->Unlock();
				//fWaitingForOtherToFreeBigObjects->Release();
				fWaitingForOtherToFreeBigObjects = nil;
			}
			else
			{
				if (tot == 0 && !BigObjectsNeedToBeFreedByOtherTask)
				{
					fWaitingForOtherToFreeBigObjects->Unlock();
					//fWaitingForOtherToFreeBigObjects->Release();
					fWaitingForOtherToFreeBigObjects = nil;
				}
			}

		}
		fNeedsByteMutex.Unlock();
		if (BigObjectsNeedToBeFreedByOtherTask && tot < inNeededBytes)
		{
			fNeedsByteMutex.Lock();
			if (fWaitingForOtherToFreeBigObjects == nil)
			{
				fWaitingForOtherToFreeBigObjectsStamp++;
				fWaitingForOtherToFreeBigObjects = new vxSyncEvent;
				fNeededMem = inNeededBytes;
				fFreedMem = tot;
			}
			else
				fWaitingForOtherToFreeBigObjects->Retain();
			vxSyncEvent* ev2 = fWaitingForOtherToFreeBigObjects;
			fNeedsByteMutex.Unlock();

			ev2->Wait(1000);

			fNeedsByteMutex.Lock();
			if (ev2 == fWaitingForOtherToFreeBigObjects) // peut avoir ete change par un autre process
			{
				fWaitingForOtherToFreeBigObjects = nil;
			}
			ev2->Release();
			fNeedsByteMutex.Unlock();

			return 1000000;
		}
		else
#endif
			return(tot);
	}
#if 0
	else // si le cache est deja en train d'etre libere par un autre process, on en attend la fin et on retourne une valeur arbitraire.
	{
		VSyncEvent* ev = fNeedsByteSyncEvent;
		ev->Retain();
		fNeedsByteMutex.Unlock();
		ev->Lock();
		ev->Release();
		return 1000000;
	}
#endif
}

VSize VDBCacheMgr::CallNeedsBytes( sLONG allocationBlockNumber, VSize inNeededBytes, bool withFlush)
{
	VDBCacheMgr *manager = VDBMgr::GetCacheManager();

	if(manager == nil)
	{
		return 0;
	}
	else
	{
		return manager->NeedsBytes( allocationBlockNumber, inNeededBytes, withFlush);
	}
}







void VDBCacheMgr::PutObject(ObjCache *inObject, Boolean checkatomic)
{
	StAllocateInCache alloc;

#if debug == 6
	WriteLogStr("VDBCacheMgr::PutObject : ");
	if (obj->Classname != nil ) WriteLogStr(obj->Classname);
	WriteLogStr(" , ");
	WriteLogHexa((sLONG)obj);
	WriteLnLogStr("");
#endif

	BTitemCache u;
	uBOOL h;
	BtreeCache* newBT;
	BtreeCache *oldglobbtcache;

	if (!inObject->IsPourDeleteCache()) {
		// before locking the cache
		xCheckFreeCachePages(); // L.E. 23/01/01  added. don't know if this is the right place
	}
	
	u.obj=inObject;
	u.sousBT=nil;
	u.ordre=inObject->GetNbAcces();
	u.pris=true;
	h=false;

	if (checkatomic)
	{
		fMutex.Lock();
	}
	//CHECKCACHE;
	
	if (fTree == nil)
		fTree = new BtreeCache;
	
	if (fPeutEquilibrer)
	{
		fPeutEquilibrer=false;
		oldglobbtcache=fTree;
		fTree=new BtreeCache;
		oldglobbtcache->Equilibre();
		delete oldglobbtcache;
	}

	fTree->search(&h,&u,0);
	if (h)
	{
		newBT = xNextFreeCachePage();
		newBT->xInitFromItem( u, fTree);
		fTree=newBT;
	}

	//CHECKCACHE;

	//CHECKCACHEDUP;
	
	if (checkatomic)
	{
		fMutex.Unlock();
	}
}


void VDBCacheMgr::RemoveObject(ObjCache *inObject)
{ 
#if debug == 6
	WriteLogStr("VDBCacheMgr::RemoveObject : ");
	if (obj->Classname != nil ) WriteLogStr(inObject->Classname);
	WriteLogStr(" , ");
	WriteLogHexa((sLONG)inObject);
	WriteLnLogStr("");
#endif
	inObject->PourDeleteCache(true);
	PutObject( inObject); 
}




void VDBCacheMgr::PutObject(IObjToFree *inObject, Boolean checkatomic)
{
	if (checkatomic)
	{
		fMutex.Lock();
	}

	fBigObjs.push_back(inObject);
	fCurBigObjToFreeisValid = false;

	if (checkatomic)
	{
		fMutex.Unlock();
	}
}


void VDBCacheMgr::RemoveObject(IObjToFree *inObject)
{ 
	VTaskLock lock(&fMutex);
	CacheList::iterator found = std::find(fBigObjs.begin(), fBigObjs.end(), inObject);
	if (found != fBigObjs.end())
	{
		fCurBigObjToFreeisValid = false;
		fBigObjs.erase(found);
	}
}


