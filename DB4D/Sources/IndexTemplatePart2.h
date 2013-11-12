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
#ifndef __INDEXTEMPLATEPART2__
#define __INDEXTEMPLATEPART2__




template <class Type, sLONG MaxCles>
BtreePage<Type, MaxCles>::BtreePage(IndexInfo* xentete, sLONG xnum, Boolean iscluster, sWORD DefaultAccess, BtreePageData<Type, MaxCles>* fromBTP)/*:ObjCacheInTree(DefaultAccess)*/
{
	entete=xentete;
	num=xnum;
//	encours=false;
	IsCluster = iscluster;
	//original = nil;
	//origaddr = 0;
	btp.nkeys = 0;
	btp.souspage0 = -1;
	//fStopWrite = 0;
	tabmem[0] = nil;
	fDebugVerify = true;
	if (fromBTP != nil)
		btp = *fromBTP;
}

#if debugCheckIndexPageOnDiskInDestructor

template <class Type, sLONG MaxCles>
void BtreePage<Type, MaxCles>::DebugCheckPageOnDisk()
{
	Base4D *db = entete->GetDB();
	DataAddr4D addr = getaddr();
	if (addr != 0)
	{
		if (!modifie())
		{
			VError err = VE_OK;
			DataBaseObjectHeader tag;
			err = tag.ReadFrom(db, addr, nil);
			if (err == VE_OK)
			{
				err = tag.ValidateTag(BtreePageDataHeader<Type>::DBOH_BtreePageTemplate, num, -2);
			}
			assert(err == VE_OK);
			if (err != VE_OK)
			{
				addr = addr; // put a break here
			}
		}
	}

}

#endif

template <class Type, sLONG MaxCles>
BtreePage<Type, MaxCles>::~BtreePage()
{

#if debug_checkRelaseIndexPage
	if (gOccPool.FindObjectInStack(dynamic_cast<IOccupable*>(this)))
	{
		sLONG xdebug = 1; // put a break here
		xbox_assert(false);
	}

#endif

#if debugCheckIndexPageOnDiskInDestructor
	Base4D *db = entete->GetDB();
	DataAddr4D addr = getaddr();
	if (addr != 0)
	{
		VError err = VE_OK;
		DataBaseObjectHeader tag;
		err = tag.ReadFrom(db, addr, nil);
		if (err == VE_OK)
		{
			err = tag.ValidateTag(BtreePageDataHeader<Type>::DBOH_BtreePageTemplate, num, -2);
		}
		assert(err == VE_OK);
		if (err != VE_OK)
		{
			addr = addr; // put a break here
		}
	}
#endif
#if debuglr
	if (btp.souspage0 != -1 && fDebugVerify)
	{
		for (sLONG i = 0; i <= btp.nkeys; i++)
		{
			if (tabmem[i] != nil)
			{
				assert(tabmem[i] == nil);
			}
		}
	}
#endif
}

template <class Type, sLONG MaxCles>
VError BtreePage<Type, MaxCles>::ThrowError( VError inErrCode, ActionDB4D inAction)
{
	if (entete != nil)
		return entete->ThrowError(inErrCode, inAction);
	else
		return inErrCode;
}



template <class Type, sLONG MaxCles>
VError BtreePage<Type, MaxCles>::loadobj(DataAddr4D xaddr, sLONG len)
{
	Base4D *db;
	VError err = VE_OK;
	
	db=entete->GetDB();
	setaddr(xaddr);
		
	assert(db->IsAddrValid(getaddr()));
	
	sLONG datalen = len - kSizeDataBaseObjectHeader;
	
	if (testAssert((size_t)datalen == sizeof(btp)))
	{

		DataBaseObjectHeader tag;
		err = tag.ReadFrom(db, getaddr(), nil);
		if (err == VE_OK)
		{
			err = tag.ValidateTag(BtreePageDataHeader<Type>::DBOH_BtreePageTemplate, num, -2);
			if (datalen != tag.GetLen())
				err = ThrowError(VE_DB4D_WRONG_TAG_HEADER, noaction);
		}
		if (err == VE_OK)
			err=db->readlong( &btp, datalen, getaddr(), kSizeDataBaseObjectHeader);
		if (err == VE_OK)
			err = tag.ValidateCheckSum(	&btp, datalen);

		if (tag.NeedSwap())
		{
			btp.SwapBytes();
		}

		assert( (btp.nkeys>0 && btp.nkeys <= MaxCles) || err != VE_OK) ;

		fill(&tabmem[0], &tabmem[btp.nkeys+1], (BtreePage<Type, MaxCles>*)nil);
		
		
	}
	else
		err = ThrowError(VE_DB4D_WRONG_TAG_HEADER, noaction);
		
	return(err);
}




template <class Type, sLONG MaxCles>
bool BtreePage<Type, MaxCles>::SaveObj(VSize& outSizeSaved)
{
#if debuglogwrite
	VString wherefrom(L"BtreePageIndex Saveobj");
	VString* whx = &wherefrom;
#else
	VString* whx = nil;
#endif
	sLONG tot;
	Base4D *db;
	
	/*
	if (VInterlocked::AtomicGet(&fStopWrite) != 0)
	{
		outSizeSaved = 0;
		return false;
	}
	else
	*/
	{
#if debugOverWrite_strong
		debug_SetCurrentWritingPage(getaddr());
#endif

		VError err = VE_OK;
		db = entete->GetDB();
		tot = sizeof(btp);
		assert(db->IsAddrValid(getaddr()));
		DataBaseObjectHeader tag(&btp, tot, BtreePageDataHeader<Type>::DBOH_BtreePageTemplate, num, -2);
		err = tag.WriteInto(db, getaddr(), whx);
		if (err == VE_OK) 
			err=db->writelong( &btp ,tot, getaddr(), kSizeDataBaseObjectHeader, whx);

#if debugCheckIndexPageOnDiskInDestructor
		sLONG8 memaddr = (sLONG8)this;
		DebugMsg(L"Index page: "+ToString(memaddr)+L" , num: "+ToString(num)+L" , disk addr: "+ToString(getaddr())+L"\n\n");
#endif

	#if debugOverWrite_strong
		debug_ClearCurrentWritingPage();
	#endif

		outSizeSaved = tot;
		return(true);
	}
}


template <class Type, sLONG MaxCles>
VError BtreePage<Type, MaxCles>::savepage(OccupableStack* curstack, BaseTaskInfo* context)
{
	DataAddr4D ou;
	Base4D *db;
	IndexHeader *head;
	VError err;
	
	err=VE_OK;
	
#if debuglr == 112
	AddPageInfoDebug(this, false, true);
#endif

#if debuglr

	sLONG nb = btp.nkeys-1;
	for (sLONG i = 0; i < nb; i++)
	{
		CleIndex<Type>* val = GetItemPtr(i);
		sLONG rescomp = CompareKeysStrict(i+1, val);
		assert(rescomp != CR_SMALLER);
	}
#endif

	if (getaddr() == 0)
	{
		db=entete->GetDB();
		ou=db->findplace(sizeof(btp)+kSizeDataBaseObjectHeader, context, err, -kIndexSegNum, this);

#if debugOverWrite_strong
		debug_SetPageRef(entete, num, ou, sizeof(btp)+kSizeDataBaseObjectHeader);
#endif
		ChangeAddr(ou, db, context);
		if (err == VE_OK)
		{
			head=entete->GetHeader();
			num=head->PutInd(curstack, num,ou, sizeof(btp)+kSizeDataBaseObjectHeader, context, err);
			setmodif(true, entete->GetDB(), context);
#if debugIndexOverlap_strong
			di_IndexOverLap::AddIndexPage(entete, num, ou, sizeof(btp)+kSizeDataBaseObjectHeader);
#endif

		}
	}
	else
	{
		setmodif(true, entete->GetDB(), context);
	}
	
#if debuglr == 112
	checktabmem();
#endif
	
	return(err);
}


template <class Type, sLONG MaxCles>
void BtreePage<Type, MaxCles>::GetItem(OccupableStack* curstack, sLONG n, FullCleIndex<Type, MaxCles>& outItem, Boolean chargesous, BaseTaskInfo* context, VError& err)
{
	outItem = *(GetItemPtr(n));
	if (chargesous)
	{
		if (outItem.souspage != -1)
		{
			gOccPool.WaitForMemoryManager(curstack);
			BtreePage<Type, MaxCles> *sous = tabmem[n+1];
			if (sous == nil)
			{
				gOccPool.EndWaitForMemoryManager(curstack);
				/*  pas besoin car GetItem n'est utilisee que pour les acces exclusifs

				VTaskLock lock(&fLoadPageMutex);
				sous = tabmem[n+1];
				if (sous == nil)
				*/
					sous = LoadPage(curstack, outItem.souspage, entete, context, err);
				tabmem[n+1]=sous;
				outItem.sousBT = sous;
			}
			else
			{
				outItem.sousBT = sous;
				sous->Occupy(curstack, false);
				gOccPool.EndWaitForMemoryManager(curstack);
			}
		}
	}
}


template <class Type, sLONG MaxCles>
void BtreePage<Type, MaxCles>::LiberePage(OccupableStack* curstack, BaseTaskInfo* context)
{
	Base4D *db;
	
#if debuglr == 112
	DelPageInfoDebug(this);
#endif

	db=entete->GetDB();
	setmodif(false, db, context);

#if debugOverWrite_strong
	debug_ClearPageRef(getaddr(), sizeof(btp) + kSizeDataBaseObjectHeader, entete);
#endif
#if debugIndexOverlap_strong
	di_IndexOverLap::RemoveIndexPage(entete, num, getaddr(), sizeof(btp) + kSizeDataBaseObjectHeader);
#endif

	db->libereplace(getaddr(), sizeof(btp) + kSizeDataBaseObjectHeader, context, this);
	entete->GetHeader()->LiberePage(curstack, num, context);
	DoNotVerify();

}



template <class Type, sLONG MaxCles>
BtreePage<Type, MaxCles>* BtreePage<Type, MaxCles>::LoadPage(OccupableStack* curstack, sLONG n, IndexInfo* xentete, BaseTaskInfo* context, VError& err)
{
	BtreePage<Type, MaxCles> *page = nil;
	DataAddr4D ou;
	sLONG len;
	err = VE_OK;
	
#if debug
	assert(n>=0 && n < ((IndexHeaderBTree*)(xentete->GetHeader()))->GetNbPage() );
#endif
	ou=xentete->GetHeader()->GetInd(curstack, n, context, err, &len);

	if (ou > 0 && len > 0 && err == VE_OK)
	{
		page = new BtreePage<Type, MaxCles>(xentete, n, xentete->GetHeader()->GetRealType() == DB4D_Index_BtreeWithCluster);
		if (page != nil)
		{
			page->Occupy(curstack, true);
			err = page->loadobj(ou, len);
			//page->libere();
			if (err != VE_OK)
			{
				page->Free(curstack, true);
				page->Release();
				page = nil;
			}
		}
		else
			err = xentete->ThrowError(memfull, DBaction_AllocatingIndexPageInMem);
	}
	else
	{
		err = xentete->ThrowError(VE_DB4D_WRONG_TAG_HEADER, DBaction_AllocatingIndexPageInMem);
	}

	if (err != VE_OK)
		err = xentete->ThrowError(VE_DB4D_CANNOTLOADINDEXPAGE, DBaction_LoadingIndexPage);
	
	return(page);
}


template <class Type, sLONG MaxCles>
BtreePage<Type, MaxCles>* BtreePage<Type, MaxCles>::AllocatePage(OccupableStack* curstack, IndexInfo* xentete, BaseTaskInfo* context, VError& err)
{
	BtreePage<Type, MaxCles> *page;
	sLONG len;
	err = VE_OK;
	
	page = new BtreePage<Type, MaxCles>(xentete, -1, xentete->GetHeader()->GetRealType() == DB4D_Index_BtreeWithCluster);
	if (page != nil)
	{
		page->Occupy(curstack, true);
		err = page->savepage(curstack, context);
		//page->libere();
		if (err != VE_OK)
		{
			page->Free(curstack, true);
			page->Release();
			page = nil;
		}
	}
	else
		err = xentete->ThrowError(memfull, DBaction_AllocatingIndexPageInMem);
	
	return(page);
}


template <class Type, sLONG MaxCles>
void BtreePage<Type, MaxCles>::SetSousPage(sLONG n, BtreePage<Type, MaxCles>* sousBT)
{
	//ForbidWrite();
	tabmem[n]=sousBT;
	if (n == 0)
	{
		if (sousBT!=nil)
		{
			btp.souspage0 = sousBT->GetNum();
			if (btp.souspage0 == -1)
				btp.souspage0 = -2;
		}
		else
		{
			btp.souspage0 = -1;
		}
	}
	else
	{
		CleIndex<Type>* item = GetItemPtr(n-1);
		if (sousBT!=nil)
		{
			item->souspage = sousBT->GetNum();
		}
		else
		{
			item->souspage = -1;
		}
	}
	//InvalidateNbElemTab();
}


template <class Type, sLONG MaxCles>
BtreePage<Type, MaxCles>* BtreePage<Type, MaxCles>::GetSousPage(OccupableStack* curstack, sLONG n, VError &err, BaseTaskInfo* context)
{
	err = VE_OK;
	if (btp.souspage0 == -1)
		return nil;
	else
	{
		gOccPool.WaitForMemoryManager(curstack);
		BtreePage<Type, MaxCles>* sous = tabmem[n];
		
		if (sous==nil)
		{
			gOccPool.EndWaitForMemoryManager(curstack);
			sLONG p = GetSousPageNum(n);
			if (p != -1)
			{
				VTaskLock lock(&fLoadPageMutex);
				sous = tabmem[n];
				if (sous == nil)
				{
					sous=LoadPage(curstack, p, entete, context, err);
					tabmem[n]=sous;
				}
				else
					sous->Occupy(curstack, true);
			}
		}
		else
		{
			sous->Occupy(curstack, false);
			gOccPool.EndWaitForMemoryManager(curstack);
		}
		return(sous);
	}
		
}


template <class Type, sLONG MaxCles>
void BtreePage<Type, MaxCles>::MoveAndAddItems(sLONG from, sLONG howmany, BtreePage<Type, MaxCles>* into)
{
	sLONG i,j,nb = from + howmany;
	assert(into->btp.nkeys+howmany <= MaxCles);

	for (i = from, j = into->btp.nkeys; i < nb; i++, j++)
	{
		CleIndex<Type>* p = GetItemPtr(i);
		into->InsertKey(j, p);
		BtreePage<Type, MaxCles>* sousBT = tabmem[i+1];
		into->tabmem[j+1] = sousBT;
	}

	//InvalidateNbElemTab();
}


template <class Type, sLONG MaxCles>
void BtreePage<Type, MaxCles>::MoveAndInsertItemsAtBegining(sLONG from, sLONG howmany, BtreePage<Type, MaxCles>* into)
{
	sLONG i,j,nb = from + howmany;
	assert(into->btp.nkeys+howmany <= MaxCles);

	for (i = from, j = 0; i < nb; i++, j++)
	{
		CleIndex<Type>* p = GetItemPtr(i);
		into->InsertKey(j, p);
		BtreePage<Type, MaxCles>* sousBT = tabmem[i+1];
		into->tabmem[j+1] = sousBT;
	}

	//InvalidateNbElemTab();
}


template <class Type, sLONG MaxCles>
void BtreePage<Type, MaxCles>::InsertKey(sLONG n, FullCleIndex<Type, MaxCles>& u, Boolean AvecSousPage)
{
	//ForbidWrite();

	sLONG nb = btp.nkeys;
	assert(nb < MaxCles);
	
	if (n < nb)
	{
		sLONG i;
		{
			CleIndex<Type> *p = &(btp.cles[nb-1]), *p2 = &(btp.cles[nb]);

			for (i = n; i < nb; i++)
			{
				*p2 = *p;
				p2--;
				p--;
			}
		}

		if (btp.souspage0 != -1) // quand la page n'est pas une feuille finale
		{
			sLONG nn = nb+1;
			BtreePage<Type, MaxCles>* *tp = &(tabmem[nb]);
			BtreePage<Type, MaxCles>* *tp2 = &(tabmem[nb+1]);
			for (i = n; i < nb; i++)
			{
				BtreePage<Type, MaxCles>* sousBT = *tp;
				*tp2 = sousBT;
				tp2--;
				tp--;
				nn--;
			}
		}
		else
		{
			tabmem[nb+1] = nil;
		}
	}

	btp.nkeys = nb + 1;
	CleIndex<Type>* x = GetItemPtr(n);
	u.CopyTo(*x);
	if (AvecSousPage)
	{
		SetSousPage(n+1, u.sousBT);
	}
	else
		tabmem[n+1] = nil;

}


template <class Type, sLONG MaxCles>
void BtreePage<Type, MaxCles>::InsertKey(sLONG n, CleIndex<Type>* u)
{
	//ForbidWrite();

	sLONG nb = btp.nkeys;
	assert(nb < MaxCles);
	
	if (n < nb)
	{
		sLONG i;
		{
			CleIndex<Type> *p = &(btp.cles[nb-1]), *p2 = &(btp.cles[nb]);

			for (i = n; i < nb; i++)
			{
				*p2 = *p;
				p2--;
				p--;
			}
		}

		if (btp.souspage0 != -1) // quand la page n'est pas une feuille finale
		{
			sLONG nn = nb+1;
			BtreePage<Type, MaxCles>* *tp = &(tabmem[nb]);
			BtreePage<Type, MaxCles>* *tp2 = &(tabmem[nb+1]);
			for (i = n; i < nb; i++)
			{
				BtreePage<Type, MaxCles>* sousBT = *tp;
				*tp2 = sousBT;
				tp2--;
				tp--;
				nn--;
			}
			//InvalidateNbElemTab();
		}
		else
		{
			tabmem[nb+1] = nil;
		}
	}

	btp.nkeys = nb + 1;
	CleIndex<Type>* x = GetItemPtr(n);
	*x = *u;
}


template <class Type, sLONG MaxCles>
void BtreePage<Type, MaxCles>::SetKey(sLONG n, FullCleIndex<Type, MaxCles>& u, Boolean AvecSousPage)
{
	//ForbidWrite();

	CleIndex<Type>* x = GetItemPtr(n);
	u.CopyTo(*x);
	if (AvecSousPage)
	{
		SetSousPage(n+1, u.sousBT);
	}
	else
		tabmem[n+1] = nil;	
}


template <class Type, sLONG MaxCles>
void BtreePage<Type, MaxCles>::AddKey(Type key, sLONG qui, BtreePage<Type, MaxCles>* sousBT)
{
	//ForbidWrite();

	assert(btp.nkeys < MaxCles);
	sLONG n = btp.nkeys;
	btp.nkeys++;
	CleIndex<Type>* x = GetItemPtr(n);
	x->qui = qui;
	x->cle = key;
	if (sousBT == nil)
	{
		tabmem[n+1] = nil;
		x->souspage = -1;
	}
	else
		SetSousPage(n+1,sousBT);
}


template <class Type, sLONG MaxCles>
void BtreePage<Type, MaxCles>::SupKey(sLONG n)
{
	//ForbidWrite();

	sLONG nb = btp.nkeys;
	assert(nb <= MaxCles);
	sLONG i;
	
	{
		CleIndex<Type> *p = &(btp.cles[n+1]), *p2 = &(btp.cles[n]);

		for (i = n; i < nb; i++)
		{
			*p2 = *p;
			p2++;
			p++;
		}
	}

	if (btp.souspage0 != -1) // si la page n'est pas une feuille finale
	{
		BtreePage<Type, MaxCles>* *tp = &(tabmem[n+2]);
		BtreePage<Type, MaxCles>* *tp2 = &(tabmem[n+1]);

		for (i = n; i < (nb-1); i++)
		{
			BtreePage<Type, MaxCles>* sousBT = *tp;
			*tp2 = sousBT;
			tp2++;
			tp++;
		}
	}

	btp.nkeys = nb - 1;
	//InvalidateNbElemTab();
}


template <class Type, sLONG MaxCles>
void BtreePage<Type, MaxCles>::SupKeys(sLONG n, sLONG howmany)
{
	//ForbidWrite();

	sLONG nb = btp.nkeys;
	assert(nb <= MaxCles);
	sLONG i;
	
	{
		CleIndex<Type> *p = &(btp.cles[n+howmany]), *p2 = &(btp.cles[n]);

		for (i = n; i <= (nb-howmany); i++)
		{
			*p2 = *p;
			p2++;
			p++;
		}
	}

	if (btp.souspage0 != -1) // si la page n'est pas une feuille finale
	{
		BtreePage<Type, MaxCles>* *tp = &(tabmem[n+1+howmany]);
		BtreePage<Type, MaxCles>* *tp2 = &(tabmem[n+1]);

		for (i = n; i <= (nb-1-howmany); i++)
		{
			BtreePage<Type, MaxCles>* sousBT = *tp;
			*tp2 = sousBT;
			tp2++;
			tp++;
		}
	}

	btp.nkeys = nb - howmany;
	//InvalidateNbElemTab();
}


template <class Type, sLONG MaxCles>
void BtreePage<Type, MaxCles>::DelKeys(sLONG from)
{
	//ForbidWrite();

	btp.nkeys = from;
	//InvalidateNbElemTab();
}



template <class Type, sLONG MaxCles>
VError BtreePage<Type, MaxCles>::Place(OccupableStack* curstack, uBOOL *h, FullCleIndex<Type, MaxCles>& v, BaseTaskInfo* context)
{
	BtreePage<Type, MaxCles> *sousBT;
	sLONG l,k,r,i,rescomp;
	FullCleIndex<Type, MaxCles> u;
	uBOOL egal,comefromplace,EgalPourCluster;
	VError err = VE_OK;

#if debuglr == 112
	checktabmem();
#endif

	if (AccesModifOK(context))
	{
		
		l=1;
		r=btp.nkeys;
		
		egal=false;
		EgalPourCluster=false;
		while ( (r>=l) && !egal && err == VE_OK)
		{
			k=(r+l)>>1;

			rescomp = CompareKeysStrict(v, k-1);

			if (rescomp==CR_EQUAL)
			{
				if (entete->IsUniqueKey() && entete->GetDB()->CheckUniquenessEnabled())
				{
					err = ThrowError(VE_DB4D_DUPLICATED_KEY, DBaction_InsertingKeyIntoIndex);
					break;
				}

				if (IsCluster)
				{
					EgalPourCluster=true;
					egal=true;
				}
				else
				{
					sLONG xqui = GetQui(k-1);
					if (xqui == v.qui)
					{
						egal = true;
					}
					else
					{
						if (v.qui > xqui)
							rescomp = CR_BIGGER;
						else
							rescomp = CR_SMALLER;
					}
				}
			}
			
			if (rescomp==CR_BIGGER) 
			{
				l=k+1;
			}
			else
			{
				r=k-1;
			}
		}
		
		if (err == VE_OK)
		{
			if (EgalPourCluster)
			{
				sLONG oldqui = GetQui(k-1);
				l=entete->GetClusterSel(curstack)->AddToCluster(curstack, oldqui, v.qui, context, err);
				if (err == VE_OK)
				{
					if (oldqui != l)
					{
						WriteLock();
						SetQui(k-1, l);
						err = savepage(curstack, context);
						WriteUnLock();
					}
				}
			}
			else
			{
				if (!egal)
				{
					comefromplace=false;
					sousBT=GetSousPage(curstack, r, err, context);
					
					if (err == VE_OK)
					{
						if (sousBT!=nil)
						{
							u=v;
							err = sousBT->Place(curstack, h, u, context);
							sousBT->Free(curstack, true);
							comefromplace=true;
						}
						else
						{
							*h=true;
							u=v;
						}
						
						if (*h)
						{
							if (btp.nkeys<MaxCles)
							{
								*h=false;
		
								WriteLock();
								InsertKey(r, u, true);
		
								if (u.sousBT!=nil) 
								{
#if debuglr
									checkPosSousBT(u.sousBT);
#endif
									//u.sousBT->SetEncours(false);
									u.sousBT->Free(curstack, true); 
								}
								if (err == VE_OK)
									err = savepage(curstack, context);
								WriteUnLock();

							}
							else
							{
								sousBT=AllocatePage(curstack, entete, context, err);
								if (err == VE_OK)
								{
									WriteLock();
									sousBT->WriteLock();

									if (r<=(MaxCles / 2))
									{
										if (r==(MaxCles / 2))
										{
											v = u;
										}
										else
										{
											GetItem(curstack, (MaxCles / 2)-1, v, true, context, err);
											if (err == VE_OK)
											{
												SupKey((MaxCles / 2)-1);
												InsertKey(r, u, true);
												if (u.sousBT!=nil)
												{
#if debuglr
													checkPosSousBT(u.sousBT);
#endif
													//u.sousBT->SetEncours(false);
													u.sousBT->Free(curstack, true); 
												}
											}
										}
										
										// move (MaxCles / 2) keys from this to sousBT
										if (err == VE_OK)
											MoveAndAddItems((MaxCles / 2), (MaxCles / 2), sousBT);
									}
									else
									{
										r=r-(MaxCles / 2);
										GetItem(curstack, (MaxCles / 2), v, true, context, err);
										if (err == VE_OK)
										{
											MoveAndAddItems((MaxCles / 2)+1, r-1, sousBT);
											sousBT->InsertKey(sousBT->GetNKeys(), u, true);
											if (u.sousBT!=nil)
											{
#if debuglr
												sousBT->checkPosSousBT(u.sousBT);
#endif
												//u.sousBT->SetEncours(false);
												u.sousBT->Free(curstack, true);
											}
											MoveAndAddItems((MaxCles / 2)+r, (MaxCles / 2)-r, sousBT);
										}
									}
									
									DelKeys((MaxCles / 2));
									sousBT->SetSousPage(0, v.sousBT);
									if (v.sousBT!=nil)
										v.sousBT->Free(curstack, true);
									v.sousBT=sousBT;
									if (v.sousBT != nil)
										v.souspage = sousBT->GetNum();
									else
										v.souspage = -1;
									
#if debuglr
									//checkPosSousBT(sousBT);
#endif
									err = savepage(curstack, context);
									if (err == VE_OK)
										err = sousBT->savepage(curstack, context);
									//sousBT->SetEncours(true);
									// sousBT->Free(curstack);  // ##  il ne faut pas le liberer ici car il sera libere plus tard en tant que u.sousBT ou v->sousBT

									sousBT->WriteUnLock();
									WriteUnLock();

#if debugCheckIndexPageOnDiskInDestructor
									sousBT->DebugCheckPageOnDisk();
#endif
								}
								else
								{
									// si l'allocation a echoue
									if (sousBT != nil)
										sousBT->Release();
								}
							}
						}
					}
				}
			}
		}
	}
	else
	{
		err = ThrowError(VE_DB4D_TRANSACTIONCONFLICT, DBaction_InsertingKeyIntoIndex);
	}
	
#if debugCheckIndexPageOnDiskInDestructor
	DebugCheckPageOnDisk();
#endif

#if debuglr == 112
	checktabmem();
#endif

	/* pas la peine de generer une exception car c'est fait au retour du premier niveau d'appel */
	return(err);
}



			
template <class Type, sLONG MaxCles>
void BtreePage<Type, MaxCles>::underflow(OccupableStack* curstack, BtreePage<Type, MaxCles>* pc, sLONG s, uBOOL *h, uBOOL *doisdel, BaseTaskInfo* context)
{
	BtreePage<Type, MaxCles>* pb;
	sLONG mc,mb,k,i;
	FullCleIndex<Type, MaxCles> etamp;
	VError err;
	
	*doisdel=false;

	mc=pc->btp.nkeys;
	if (s<mc) // { b est la page de droite de this }
	{
		++s;
		pc->GetItem(curstack, s-1, etamp, true, context, err);
		pb=etamp.sousBT;
		
		mb=pb->btp.nkeys;
		k=(mb-(MaxCles / 2)+1)/2;
		
		WriteLock();
		etamp.sousBT = pb->GetSousPage(curstack, 0, err, context);
		InsertKey((MaxCles / 2)-1,etamp, true);
		if (etamp.sousBT!=nil)
			etamp.sousBT->Free(curstack, true);
			
		if (k>0)   // deplace k items de b en this
		{
			pb->WriteLock();
			pc->WriteLock();
			pb->MoveAndAddItems(0, k-1, this);

			pb->GetItem(curstack, k-1, etamp, true, context, err);
			pb->SetSousPage(0, etamp.sousBT);
			if (etamp.sousBT!=nil)
				etamp.sousBT->Free(curstack, true);
			etamp.sousBT = pb;

			pc->SetKey(s-1, etamp, true);

			mb=mb-k;
			pb->SupKeys(0, k);
			*h=false;
			

#if debuglr
			pc->checkPosSousBT(this);
			pc->checkPosSousBT(pb);
#endif
			savepage(curstack, context);
			pb->savepage(curstack, context);
			pc->savepage(curstack, context);
			pb->Free(curstack, true);

			pb->WriteUnLock();
			pc->WriteUnLock();
		}
		else // fusione les pages this et b dans this
		{
			pb->WriteLock();
			pc->WriteLock();
			pb->MoveAndAddItems(0, (MaxCles / 2), this);

			pc->SupKey(s-1);
			*h=(pc->btp.nkeys < (MaxCles / 2));
			
			pb->SetValidForWrite(false);
			pb->WriteUnLock();

			//pb->SetParent(nil,0);
			pb->LiberePage(curstack, context);
			pb->Free(curstack, true);
			//pb->PrepareToDelete();
			pb->Release();
			pb = nil;

#if debuglr
			pc->checkPosSousBT(this);
#endif
			savepage(curstack, context);
			pc->savepage(curstack, context);

			pc->WriteUnLock();
		}

		WriteUnLock();
	}
	else  // b est la page de gauche de this
	{
		pb=pc->GetSousPage(curstack, s-1, err, context);
		
		mb=pb->btp.nkeys+1;
		k=(mb-(MaxCles / 2)+1)/2;
		
		if (pb->btp.nkeys > (MaxCles / 2)) // deplace k items de b en this
		{
			WriteLock();
			pb->WriteLock();
			pc->WriteLock();

			pc->GetItem(curstack, s-1, etamp, true, context, err);
			if (etamp.sousBT!=nil)
				etamp.sousBT->Free(curstack, true);
			etamp.sousBT = GetSousPage(curstack, 0, err, context);

			InsertKey(0, etamp, true);
			if (etamp.sousBT!=nil)
				etamp.sousBT->Free(curstack, true);
			mb=mb-k;

			pb->MoveAndInsertItemsAtBegining(mb, k-1, this);

			pb->GetItem(curstack, mb-1, etamp, true, context, err);
			SetSousPage(0,etamp.sousBT);
			if (etamp.sousBT!=nil)
				etamp.sousBT->Free(curstack, true);
			etamp.sousBT=this;
			pc->SetKey(s-1,etamp, true);
			
			pb->DelKeys(mb-1);
			*h=false;
			
#if debuglr
			pc->checkPosSousBT(this);
			pc->checkPosSousBT(pb);
#endif
			savepage(curstack, context);
			pb->savepage(curstack, context);
			pc->savepage(curstack, context);

			pc->WriteUnLock();
			pb->WriteUnLock();
			WriteUnLock();

			pb->Free(curstack, true);
		}
		else // fusionne les pages this et b dans b
		{
			WriteLock();
			pb->WriteLock();
			pc->WriteLock();

			pc->GetItem(curstack, s-1, etamp, true, context, err);
			if (etamp.sousBT!=nil)
				etamp.sousBT->Free(curstack, true);
			etamp.sousBT=GetSousPage(curstack, 0, err, context);

			pb->InsertKey(mb-1,etamp, true);
			if (etamp.sousBT!=nil)
				etamp.sousBT->Free(curstack, true);
			
			MoveAndAddItems(0, (MaxCles / 2)-1, pb);

			// pc->DelKeys(mc-1);  // hautement suspect
			pc->SupKey(s-1); // me semble plus approprie

			//SetParent(nil,0);
#if debuglr
			pc->checkPosSousBT(pb);
#endif
			pb->savepage(curstack, context);
			pc->savepage(curstack, context);
			*h=( pc->GetNKeys() < (MaxCles / 2) );
			
			pc->WriteUnLock();
			pb->WriteUnLock();

			SetValidForWrite(false);
			WriteUnLock();

			*doisdel=true;
			LiberePage(curstack, context);

			pb->Free(curstack, true);
		}
	}

}


template <class Type, sLONG MaxCles>
void BtreePage<Type, MaxCles>::del(OccupableStack* curstack, BtreePage<Type, MaxCles>* from, sLONG k, uBOOL *h, uBOOL *doisdel, BaseTaskInfo* context)
{
	BtreePage<Type, MaxCles> *sousBT;
	sLONG r;
	BtreePage<Type, MaxCles> *old;
	FullCleIndex<Type, MaxCles> u;
	uBOOL locdoisdel;
	VError err;

	*doisdel=false;
	
	r = GetNKeys();
	sousBT = GetSousPage(curstack, r, err, context); 

	if (sousBT!=nil)
	{
		sousBT->del(curstack, from, k, h, &locdoisdel, context); 
		if (*h) 
			sousBT->underflow(curstack, this, r, h, &locdoisdel, context);
		sousBT->Free(curstack, true);
		if (locdoisdel) 
		{
			//sousBT->PrepareToDelete();
			sousBT->Release();
		}
	}
	else
	{
		old = from->GetSousPage(curstack, k, err, context);
		GetItem(curstack, r-1, u, true, context, err);

		WriteLock();
		from->WriteLock();

		if (old==nil) 
			u.souspage=-1; 
		else 
			u.souspage=old->GetNum();

		u.sousBT=old;

		if (old != nil) 
			old->Free(curstack, true);

		from->SetKey(k-1,u,true);
		DelKeys(r-1);
		
#if debuglr
		from->checkPosSousBT(u.sousBT);
#endif
		from->savepage(curstack, context);
		savepage(curstack, context);

		from->WriteUnLock();
		WriteUnLock();

		*h=(GetNKeys()<(MaxCles / 2));
	}

}


template <class Type, sLONG MaxCles>
VError BtreePage<Type, MaxCles>::Detruire(OccupableStack* curstack, uBOOL *h, const FullCleIndex<Type, MaxCles>& v, uBOOL *doisdel, BaseTaskInfo* context)
{
	VError err = VE_OK;
	
	BtreePage<Type, MaxCles> *sousBT;
	sLONG l,k,r,p,rescomp;
	uBOOL egal,locdoisdel,EgalPourCluster = false;

	// il faut faire deux passes lors d'une transaction
	
	*doisdel=false;
	
	//if (AccesModifOK())
	{		
		l=1;
		r=GetNKeys();
		
		egal=false;
		while (r>=l)
		{
			k=(r+l)>>1;
			rescomp = CompareKeysStrict(v, k-1);

			if (rescomp==CR_EQUAL)
			{
				if (IsCluster)
				{
					EgalPourCluster=true;
					egal=true;
				}
				else
				{
					sLONG xqui = GetQui(k-1);
					if (xqui == v.qui)
					{
						egal = true;
					}
					else
					{
						if (v.qui > xqui)
							rescomp = CR_BIGGER;
						else
							rescomp = CR_SMALLER;
					}
				}
			}

			if ( (rescomp==CR_BIGGER) || egal)  // val > val2
			{
				l=k+1;
			}
			if ( (rescomp==CR_SMALLER) || egal)  // val < val2
			{
				r=k-1;
			}

		} // du while
		
		if (EgalPourCluster)
		{
			sLONG oldqui = GetQui(k-1);
			sLONG clustval=entete->GetClusterSel(curstack)->DelFromCluster(curstack, oldqui, v.qui, context, err);
			if (clustval != oldqui)
			{
				WriteLock();
				SetQui(k-1, clustval);
				savepage(curstack, context);
				WriteUnLock();
			}
			if (clustval==-1)
			{
				EgalPourCluster=false;
			}
		}
		
		if (!EgalPourCluster)
		{
			sousBT=GetSousPage(curstack, r, err, context);
			
			if ( (l-r) > 1 )
			{
				if (sousBT==nil)
				{
					WriteLock();
					CleIndex<Type>* x = GetItemPtr(k-1);
					p=x->souspage;
					SupKey(k-1);
					*h=(GetNKeys()<(MaxCles / 2));
#if debugindex_strong
					if (*h)
						((IndexInfoScalar<Type, MaxCles>*)entete)->debug_mustcheck = true;
#endif
					savepage(curstack, context);
					WriteUnLock();
				}
				else
				{
#if debugindex_strong
					((IndexInfoScalar<Type, MaxCles>*)entete)->debug_mustcheck = true;
#endif				
					sousBT->del(curstack, this, k, h, &locdoisdel, context);
					if (*h) 
						sousBT->underflow(curstack, this, r, h, &locdoisdel, context);
					sousBT->Free(curstack, true);
					if (locdoisdel) 
					{
//						sousBT->PrepareToDelete();
						sousBT->Release();
					}
				}
			}
			else
			{
				if (sousBT==nil)
				{
					*h=false;
				}
				else
				{
					sousBT->Detruire(curstack, h, v, &locdoisdel, context);
					if (*h) 
						sousBT->underflow(curstack, this, r, h, &locdoisdel, context);
					sousBT->Free(curstack, true);
					if (locdoisdel) 
					{
//						sousBT->PrepareToDelete();
						sousBT->Release();
					}
				}
			}
		}
		
	}
	
	
	return(err);
}



														
template <class Type, sLONG MaxCles>
void BtreePage<Type, MaxCles>::LibereEspaceMem(OccupableStack* curstack)
{
	sLONG nb,i;
	BtreePage<Type, MaxCles> *sous;

	Occupy(curstack, true);
	
	if (btp.souspage0 != -1)
	{
		nb = GetNKeys();
		
		assert(nb>=0 && nb <= MaxCles);
				
		for (i=0;i<=nb;i++)
		{
			sous=tabmem[i];
			if (sous!=nil)
			{
				sous->LibereEspaceMem(curstack);
				sous->Release();
				tabmem[i] = nil;
			}
		}
	}

	Free(curstack, true);
	//InvalidateNbElemTab();
}							



template <class Type, sLONG MaxCles>
void BtreePage<Type, MaxCles>::DelFromFlush(OccupableStack* curstack)
{
	sLONG nb,i;
	BtreePage<Type, MaxCles> *sous;

	Occupy(curstack, true);

	if (btp.souspage0 != -1)
	{
		nb = GetNKeys();

		assert(nb>=0 && nb <= MaxCles);

		for (i=0;i<=nb;i++)
		{
			sous=tabmem[i];
			if (sous!=nil)
			{
				sous->DelFromFlush(curstack);
			}
		}
	}

	setmodif(false, entete->GetDB(), nil);
	Free(curstack, true);
	//InvalidateNbElemTab();
}							

template <class Type, sLONG MaxCles>
VError BtreePage<Type, MaxCles>::SelectAllKeys(OccupableStack* curstack, Bittab* b, BaseTaskInfo* context, ProgressEncapsuleur* InProgress, FullCleIndex<Type, MaxCles>* outVal)
{
	VError err = VE_OK;
	VTask::Yield();
	if (!InProgress->Increment())
		err = ThrowError(VE_DB4D_ACTIONCANCELEDBYUSER, noaction);
	if (err == VE_OK)
	{
		bool canCont = true;
		sLONG nb = btp.nkeys;
		assert(nb>=0 && nb<= MaxCles);
		for (sLONG i = 0; i < nb && canCont && err == VE_OK; ++i)
		{
			BtreePage<Type, MaxCles>* sousBT=GetSousPage(curstack, i, err, context);
			if (sousBT!=nil)
			{
				err = sousBT->SelectAllKeys(curstack, b, context, InProgress, outVal);
				sousBT->Free(curstack, true);
			}

			if (IsCluster)
			{
				entete->GetClusterSel(curstack)->AddSelToBittab(curstack, GetQui(i), b, context, err);
			}
			else
			{
				err = b->Set(GetQui(i), false);
			}
			if (outVal != nil)
			{
				canCont = false;
				GetItem(curstack, i, *outVal, false, context, err);	
			}
		}
		if (canCont && err == VE_OK)
		{
			BtreePage<Type, MaxCles>* sousBT=GetSousPage(curstack, nb, err, context);
			if (sousBT!=nil)
			{
				err = sousBT->SelectAllKeys(curstack, b, context, InProgress, outVal);
				sousBT->Free(curstack, true);
			}
		}

	}
	return err;
}

template <class Type, sLONG MaxCles>
VError BtreePage<Type, MaxCles>::FourcheScalar(OccupableStack* curstack, Bittab* b, const FullCleIndex<Type, MaxCles>& val1, uBOOL xstrict1, const FullCleIndex<Type, MaxCles>& val2, 
				uBOOL xstrict2, BaseTaskInfo* context, ProgressEncapsuleur* InProgress, const VCompareOptions& inOptions, FullCleIndex<Type, MaxCles>* outVal)
{
	BtreePage<Type, MaxCles> *sousBT;
	sLONG i,nb,rescomp;
	uBOOL egal;
	VError err = VE_OK;
	Boolean canCont = true;
	
	VTask::Yield();
	if (!InProgress->Increment())
		err = ThrowError(VE_DB4D_ACTIONCANCELEDBYUSER, noaction);

	if (err == VE_OK)
	{
		nb=btp.nkeys;
		
		assert(nb>=0 && nb<= MaxCles);

		if (gTempNewIndexFourche) // new algorithm
		{
			sLONG mostleft = 0, mostright = 0;
			bool mostleftIsEqual = false, mostrightIsEqual = false;

			if (!val1.IsNull())
			{
				bool cont = true;
				sLONG left = 0;
				sLONG right = nb;
				sLONG lastequal = -2;
				while (cont)
				{
					if ((right-left) <= 1)
					{
						cont = false;
						rescomp = CompareKeys(mostleft, val1, inOptions);

						if (rescomp == CR_BIGGER)
						{
							--mostleft;
							if (mostleft == 0)
							{
								rescomp = CompareKeys(mostleft, val1, inOptions);

								if (rescomp == CR_BIGGER)
									--mostleft;
								else if ((rescomp == CR_EQUAL) && !xstrict1)
									mostleftIsEqual= true;
							}

						}
						else if (rescomp == CR_SMALLER || ((rescomp == CR_EQUAL) && xstrict1))
						{
							if (lastequal != -2)
							{
								mostleft = lastequal;
								mostleftIsEqual = true;
							}
						}
						else
						{
							mostleftIsEqual = true;
							if (mostleft > 0)
							{
								rescomp = CompareKeys(mostleft-1, val1, inOptions);

								if (rescomp == CR_EQUAL && !xstrict1)
									--mostleft;
							}
						}
					}
					else
					{
						mostleft = left + ((right - left + 1) / 2);
						rescomp = CompareKeys( mostleft, val1, inOptions);

						if (((rescomp == CR_EQUAL) && !xstrict1) || rescomp == CR_BIGGER)
						{
							right = mostleft;
							if (rescomp == CR_EQUAL && !xstrict1)
								lastequal = mostleft;
						}
						else
						{
							left = mostleft;
						}
					}
				}

			}
			else
			{
				mostleft = -1;
				mostleftIsEqual = false;
			}


			if (!val2.IsNull())
			{
				bool cont = true;
				sLONG left = 0;
				sLONG right = nb;
				sLONG lastequal = -2;
				while (cont)
				{
					if ((right-left) <= 1)
					{
						cont = false;
						rescomp = CompareKeys(mostright, val2, inOptions);

						if (rescomp == CR_SMALLER)
						{
							++mostright;
							if (mostright == (nb-1))
							{
								rescomp = CompareKeys(mostright, val2, inOptions);

								if (rescomp == CR_SMALLER)
									++mostright;
								else if ((rescomp == CR_EQUAL) && !xstrict2)
									mostrightIsEqual= true;
							}

						}
						else if (rescomp == CR_BIGGER || ((rescomp == CR_EQUAL) && xstrict2))
						{
							if (mostright == 1)
							{
								rescomp = CompareKeys(mostright-1, val2, inOptions);
								if (rescomp == CR_BIGGER || rescomp == CR_EQUAL)
								{
									--mostright;
									if (rescomp == CR_EQUAL && !xstrict2)
										mostrightIsEqual = true;
								}
							}
							else if (lastequal != -2)
							{
								mostright = lastequal;
								mostrightIsEqual = true;
							}
						}
						else
						{
							mostrightIsEqual = true;
							if (mostright < (nb-1))
							{
								rescomp = CompareKeys(mostright+1, val2, inOptions);

								if (rescomp == CR_EQUAL)
									++mostright;
							}
						}
					}
					else
					{
						mostright = left + ((right - left + 1) / 2);
						rescomp = CompareKeys(mostright, val2, inOptions);

						if ( ((rescomp == CR_EQUAL) && !xstrict2) || rescomp == CR_SMALLER)
						{
							if (rescomp == CR_EQUAL && !xstrict2)
								lastequal = mostright;
							left = mostright;
						}
						else
						{
							right = mostright;
						}
					}
				}			
			}
			else
			{
				mostright = nb;
				mostrightIsEqual = false;
			}

			sLONG needFourcheRight = -2;
			sLONG begin = mostleft, end = mostright;

			if (!mostleftIsEqual)
				++begin;

			if (mostrightIsEqual)
				++end;

			for (i = begin; i <=  mostright && canCont && err == VE_OK; ++i)
			{
				if ((i != mostright) || mostrightIsEqual)
				{
					if (IsCluster)
					{
						entete->GetClusterSel(curstack)->AddSelToBittab(curstack, GetQui(i), b, context, err);
					}
					else
					{
						err = b->Set(GetQui(i), false);
					}
					if (outVal != nil)
					{
						canCont = false;
						GetItem(curstack, i, *outVal, false, context, err);	
					}
				}
				sousBT=GetSousPage(curstack, i, err, context);
				if (sousBT!=nil)
				{
					if (i > begin && i < end)
					{
						err = sousBT->SelectAllKeys(curstack, b, context, InProgress, outVal);
					}
					else
					{
						err = sousBT->FourcheScalar(curstack, b, val1, xstrict1, val2, xstrict2, context, InProgress, inOptions, outVal);
					}
					sousBT->Free(curstack, true);
				}
			}

			if (mostrightIsEqual && canCont && err == VE_OK)
			{
				if (testAssert(mostright < nb))
				{
					sousBT=GetSousPage(curstack, mostright+1, err, context);
					if (sousBT!=nil)
					{
						err = sousBT->FourcheScalar(curstack, b, val1, xstrict1, val2, xstrict2, context, InProgress, inOptions, outVal);
						sousBT->Free(curstack, true);
					}

				}
			}

		}
		else // old algorithm
		{
 			for (i=0;i<nb && canCont;i++)
			{
				if (val1.IsNull())
				{
					rescomp=CR_BIGGER;
				}
				else
				{
					rescomp = CompareKeys(i, val1, inOptions);
				}
				
				egal=((rescomp==CR_EQUAL) && !xstrict1) || (rescomp==CR_BIGGER);
				if ( (rescomp==CR_BIGGER) || ( (rescomp==CR_EQUAL) && !xstrict1) )
				{
					sousBT=GetSousPage(curstack, i, err, context);
					if (sousBT!=nil)
					{
						err = sousBT->FourcheScalar(curstack, b, val1, xstrict1, val2, xstrict2, context, InProgress, inOptions, outVal);
						sousBT->Free(curstack, true);
					}
				}
				
				if (err != VE_OK)
					break;

				if (val2.IsNull())
				{
					rescomp=CR_SMALLER;
				}
				else
				{
					rescomp = CompareKeys(i, val2, inOptions);
				}
				
				if ( (((rescomp==CR_EQUAL) && !xstrict2) || (rescomp==CR_SMALLER)) && egal )
				{
					if (IsCluster)
					{
						entete->GetClusterSel(curstack)->AddSelToBittab(curstack, GetQui(i), b, context, err);
					}
					else
					{
						err = b->Set(GetQui(i), false);
					}
					if (outVal != nil)
					{
						canCont = false;
						GetItem(curstack, i, *outVal, false, context, err);
					}
				}
				
				if (i==(nb-1) && err == VE_OK && canCont)
				{
					if ( ((rescomp==CR_EQUAL) && !xstrict2) || (rescomp==CR_SMALLER) )
					{
						sousBT=GetSousPage(curstack, i+1, err, context);
						if (sousBT!=nil)
						{
							err = sousBT->FourcheScalar(curstack, b, val1, xstrict1, val2, xstrict2, context, InProgress, inOptions, outVal);
							sousBT->Free(curstack, true);
						}
					}
				}
				
				if (err != VE_OK)
					break;
				if (rescomp==CR_BIGGER) 
					break; // sort de la boucle
			}
		}
	}

	return err;
}


template <class Type, sLONG MaxCles>
VError BtreePage<Type, MaxCles>::ScanIndex(OccupableStack* curstack, Selection* into, sLONG& currec, sLONG inMaxRecords, Boolean KeepSorted, Boolean Ascent, BaseTaskInfo* context, Bittab* filtre, ProgressEncapsuleur* InProgress)
{
	VError err = VE_OK;
	BtreePage<Type, MaxCles>* sousBT;
	BTitemIndex* x;

	sLONG nb = btp.nkeys;

	VTask::Yield();
	if (!InProgress->Increment())
		err = ThrowError(VE_DB4D_ACTIONCANCELEDBYUSER, noaction);

	if (err == VE_OK)
	{
		assert(nb>=0 && nb<= MaxCles);
		if (Ascent)
		{
			sousBT=GetSousPage(curstack, 0, err, context);
			if (sousBT != nil)
			{
				err = sousBT->ScanIndex(curstack, into, currec, inMaxRecords, KeepSorted, Ascent, context, filtre, InProgress);
				sousBT->Free(curstack, true);
			}

			for (sLONG i = 0; i<nb && err == VE_OK && currec < inMaxRecords; i++)
			{
				sLONG l = GetQui(i);
				if (IsCluster)
				{
					err = entete->GetClusterSel(curstack)->AddSelToSel(curstack, currec, inMaxRecords, filtre, into, l, context);
				}
				else
				{
					if (filtre == nil || filtre->isOn(l))
					{
						err = into->PutFic(currec, l);
						if (err == VE_OK)
							currec++;
					}

				}
				
				if (err == VE_OK && currec < inMaxRecords)
				{
					sousBT=GetSousPage(curstack, i+1, err, context);
					if (sousBT != nil)
					{
						err = sousBT->ScanIndex(curstack, into, currec, inMaxRecords, KeepSorted, Ascent, context, filtre, InProgress);
						sousBT->Free(curstack, true);
					}
				}
			}
		}
		else
		{
			for (sLONG i = nb-1; i>=0 && err == VE_OK && currec < inMaxRecords; i--)
			{
				sousBT=GetSousPage(curstack, i+1, err, context);
				if (sousBT != nil)
				{
					err = sousBT->ScanIndex(curstack, into, currec, inMaxRecords, KeepSorted, Ascent, context, filtre, InProgress);
					sousBT->Free(curstack, true);
				}

				if (err == VE_OK && currec < inMaxRecords)
				{
					sLONG l = GetQui(i);
					if (IsCluster)
					{
						err = entete->GetClusterSel(curstack)->AddSelToSel(curstack, currec, inMaxRecords, filtre, into, l, context);
					}
					else
					{
						if (filtre == nil || filtre->isOn(l))
						{
							err = into->PutFic(currec, l);
							if (err == VE_OK)
								currec++;
						}

					}
				}
			}

			if (err == VE_OK && currec < inMaxRecords)
			{
				sousBT=GetSousPage(curstack, 0, err, context);
				if (sousBT != nil)
				{
					err = sousBT->ScanIndex(curstack, into, currec, inMaxRecords, KeepSorted, Ascent, context, filtre, InProgress);
					sousBT->Free(curstack, true);
				}

			}
		}
	}

	return err;
}


template <class Type, sLONG MaxCles>
VError BtreePage<Type, MaxCles>::CalculateColumnFormulas(OccupableStack* curstack, ColumnFormulas* formules, BaseTaskInfo* context, Bittab* filtre, sLONG &curpage, 
											   ProgressEncapsuleur* InProgress, Boolean& stop)
{
	BtreePage<Type, MaxCles> *sousBT;
	sLONG i,nb,l;
	VError err = VE_OK;

	if (InProgress != nil)
	{
		if (!InProgress->Progress(curpage))
			err = ThrowError(VE_DB4D_ACTIONCANCELEDBYUSER, DBaction_CalculateFomulasWithIndex);
	}

	nb = btp.nkeys;

	if (err == VE_OK)
	{
		sousBT = GetSousPage(curstack, 0, err, context);
		if (sousBT != nil && err == VE_OK)
		{
			curpage++;
			err = sousBT->CalculateColumnFormulas(curstack, formules, context, filtre, curpage, InProgress, stop);
			sousBT->Free(curstack, true);
		}

		if (!stop)
		{
			for (i=0;i<nb && err == VE_OK;i++)
			{
				Type curkey = GetItemPtr(i)->cle;
				l = GetQui(i);
				if (IsCluster)
				{
					if (l>=0)
					{
						if (filtre == nil || filtre->isOn(l))
						{
							err = formules->ExecuteOnOneKey<Type>(entete, curkey, l, stop, context);
							if (stop)
								break;
						}
					}
					else
					{
						Selection* sel = entete->GetClusterSel(curstack)->GetSel(curstack, l, context, err);
						if (sel != nil)
						{
							SelectionIterator itersel(sel);
							sLONG currec = itersel.FirstRecord();
							while (currec != -1)
							{
								if (filtre == nil || filtre->isOn(currec))
									err = formules->ExecuteOnOneKey<Type>(entete, curkey, currec, stop, context);
								if (stop)
									break;
								currec = itersel.NextRecord();
							}

							sel->Release();
						}
						if (stop)
							break;
					}
				}
				else
				{
					if (filtre == nil || filtre->isOn(l))
					{
						err = formules->ExecuteOnOneKey<Type>(entete, curkey, l, stop, context);
						if (stop)
							break;
					}

				}

				if (!stop)
				{
					sousBT = GetSousPage(curstack, i+1, err, context);
					if (sousBT != nil && err == VE_OK)
					{
						curpage++;
						err = sousBT->CalculateColumnFormulas(curstack, formules, context, filtre, curpage, InProgress, stop);
						sousBT->Free(curstack, true);
						if (stop)
							break;
					}
				}

			}

		}
	}

	return err;
}

/*

template <class Type, sLONG MaxCles, class ResultType>
VError BtreePage<Type, MaxCles>::CalculateSum<ResultType>(OccupableStack* curstack, BaseTaskInfo* context, Bittab* filtre, sLONG &curpage, 
														 ProgressEncapsuleur* InProgress, Boolean& stop, sLONG& outCount, ResultType& outSum)
{
	BtreePage<Type, MaxCles> *sousBT;
	sLONG i,nb,l;
	VError err = VE_OK;

	if (InProgress != nil)
	{
		if (!InProgress->Progress(curpage))
			err = ThrowError(VE_DB4D_ACTIONCANCELEDBYUSER, DBaction_CalculateFomulasWithIndex);
	}

	nb = btp.nkeys;

	if (err == VE_OK)
	{
		sousBT = GetSousPage(curstack, 0, err, context);
		if (sousBT != nil && err == VE_OK)
		{
			curpage++;
			err = sousBT->CalculateSum(curstack, context, filtre, curpage, InProgress, stop, outCount, outSum);
			sousBT->Free(curstack, true);
		}

		if (!stop)
		{
			for (i=0;i<nb && err == VE_OK;i++)
			{
				Type curkey = GetItemPtr(i)->cle;
				l = GetQui(i);
				if (IsCluster)
				{
					if (l>=0)
					{
						if (filtre == nil || filtre->isOn(l))
						{
							outCount++;
							outSum += curkey;
							if (stop)
								break;
						}
					}
					else
					{
						Selection* sel = entete->GetClusterSel(curstack)->GetSel(curstack, l, context, err);
						if (sel != nil)
						{
							SelectionIterator itersel(sel);
							sLONG currec = itersel.FirstRecord();
							while (currec != -1)
							{
								if (filtre == nil || filtre->isOn(currec))
								{
									outCount++;
									outSum += curkey;
								}
								if (stop)
									break;
								currec = itersel.NextRecord();
							}

							sel->Release();
						}
						if (stop)
							break;
					}
				}
				else
				{
					if (filtre == nil || filtre->isOn(l))
					{
						outCount++;
						outSum += curkey;
						if (stop)
							break;
					}

				}

				if (!stop)
				{
					sousBT = GetSousPage(curstack, i+1, err, context);
					if (sousBT != nil && err == VE_OK)
					{
						curpage++;
						err = sousBT->CalculateSum(curstack, context, filtre, curpage, InProgress, stop, outCount, outSum);
						sousBT->Free(curstack, true);
						if (stop)
							break;
					}
				}

			}

		}
	}

	return err;
}

*/


template <class Type, sLONG MaxCles>
VError BtreePage<Type, MaxCles>::FindKeyInArrayScalar(OccupableStack* curstack, Bittab* b, DB4DArrayOfConstDirectValues<Type>* values, const Type* &CurVal, BaseTaskInfo* context, const VCompareOptions& inOptions, ProgressEncapsuleur* InProgress)
{
	BtreePage<Type, MaxCles> *sousBT;
	sLONG i,nb,rescomp;
	uBOOL egal;
	VError err = VE_OK;

	nb=btp.nkeys;
	Boolean feuillefinale = btp.souspage0 == -1;

	VTask::Yield();
	if (!InProgress->Increment())
		err = ThrowError(VE_DB4D_ACTIONCANCELEDBYUSER, noaction);

	assert(nb>=0 && nb<= MaxCles);
	for (i=0;i<nb && CurVal != nil && err == VE_OK;i++)
	{
		Boolean trynextval;
		do
		{
			trynextval = false;
			rescomp = CompareKeysPtr(i, CurVal, inOptions);
			if (rescomp == CR_BIGGER)
			{
				if (feuillefinale)
					trynextval = true;
				else
				{
					sousBT=GetSousPage(curstack, i, err, context);
					if (sousBT!=nil)
					{
						err = sousBT->FindKeyInArrayScalar(curstack, b, values, CurVal, context, inOptions, InProgress);
						sousBT->Free(curstack, true);

						if (CurVal != nil)
						{
							rescomp = CompareKeysPtr(i, CurVal, inOptions);
							if (rescomp == CR_EQUAL)
							{
								if (IsCluster)
								{
									entete->GetClusterSel(curstack)->AddSelToBittab(curstack, GetQui(i), b, context, err);
								}
								else
								{
									err = b->Set(GetQui(i), false);
								}
							}
							else
							{
								if (rescomp == CR_BIGGER)
								{
									trynextval = true;
								}
							}
						}
					}
				}
			}
			else
			{
				if (rescomp == CR_EQUAL)
				{
					if (IsCluster)
					{
						entete->GetClusterSel(curstack)->AddSelToBittab(curstack, GetQui(i), b, context, err);
					}
					else
					{
						err = b->Set(GetQui(i), false);
					}

					if (!feuillefinale)
					{
						sousBT=GetSousPage(curstack, i, err, context);
						if (sousBT!=nil)
						{
							err = sousBT->FindKeyInArrayScalar(curstack, b, values, CurVal, context, inOptions, InProgress);
							sousBT->Free(curstack, true);
						}
					}
				}
			}

			if (trynextval)
			{
				CurVal = values->Next();
				if (CurVal == nil)
					trynextval = false;
			}
		} while (trynextval && err == VE_OK);

	}

	if (CurVal != nil && (rescomp == CR_EQUAL || rescomp == CR_SMALLER) && !feuillefinale && err == VE_OK)
	{
		sousBT=GetSousPage(curstack, btp.nkeys, err, context);
		if (sousBT!=nil)
		{
			err = sousBT->FindKeyInArrayScalar(curstack, b, values, CurVal, context, inOptions, InProgress);
			sousBT->Free(curstack, true);
		}
	}

	return err;
}


template <class Type, sLONG MaxCles>
VError BtreePage<Type, MaxCles>::FindKeyInArray(OccupableStack* curstack, Bittab* b, DB4DArrayOfValues* values, BaseTaskInfo* context, const VCompareOptions& inOptions, ProgressEncapsuleur* InProgress)
{
	assert(false); // ne devrait plus jamais passer par la
#if 0
	BtreePage<Type, MaxCles> *sousBT;
	sLONG i,nb,rescomp, rescomp2;
	CleIndex<Type> *val;
	VError err = VE_OK;
	if (values->Count() == 0)
	{
		// rien a faire
	}
	else
	{
		const VValueSingle* val1 = values->GetFirst();
		const VValueSingle* val2 = values->GetLast();
		FullCleIndex<Type, MaxCles> xval1(val1) ,xval2(val2);
		void* data;

		Occupy();

		nb=btp.nkeys;

		assert(nb>=0 && nb <= MaxCles);
		for (i=0;i<nb;i++)
		{
			val = GetItemPtr(i);
			data = &(val->cle);

			rescomp = CompareKeys(xval1, i, inOptions);
			rescomp2 = CompareKeys(xval2, i, inOptions);

			//rescomp = XBOX::VValue::InvertCompResult(val1->Swap_CompareToSameKindPtrWithOptions(data, inOptions));
			//rescomp2 = XBOX::VValue::InvertCompResult(val2->Swap_CompareToSameKindPtrWithOptions(data, inOptions));


			if ((rescomp==CR_SMALLER) || (rescomp==CR_EQUAL))
			{
				sousBT=GetSousPage(curstack, i, err, context);
				if (sousBT!=nil)
				{
					err = sousBT->FindKeyInArray(b, values, context, inOptions);
					sousBT->Free(curstack);
				}
			}

			if ( ((rescomp==CR_SMALLER) || (rescomp==CR_EQUAL)) && ((rescomp2==CR_BIGGER) || (rescomp2==CR_EQUAL)) )
			{
				if (values->FindWithDataPtr(data, inOptions))
				{
					if (IsCluster)
					{
						entete->GetClusterSel()->AddSelToBittab(GetQui(i), b, context, err);
					}
					else
					{
						err = b->Set(GetQui(i), false);
					}
				}
			}

			if (i==(nb-1) && err == VE_OK)
			{
				if ( (rescomp2==CR_EQUAL) || (rescomp2==CR_BIGGER) )
				{
					sousBT=GetSousPage(curstack, i+1, err, context);
					if (sousBT!=nil)
					{
						err = sousBT->FindKeyInArray(b, values, context, inOptions);
						sousBT->Free(curstack);
					}
				}
			}

			if (err != VE_OK)
				break;

			if (rescomp2==CR_SMALLER) break; // sort de la boucle
		}

		Free(curstack);
	}

	return err;
#endif
	return VE_DB4D_NOTIMPLEMENTED;
}


template <class Type, sLONG MaxCles>
VError BtreePage<Type, MaxCles>::GetDistinctValues(OccupableStack* curstack, DB4DCollectionManager& outCollection, BaseTaskInfo* context, Bittab* filtre, 
						 sLONG &curpage, sLONG &curvalue, Type &curkey, Boolean& dejaval, ProgressEncapsuleur* InProgress, 
						 VCompareOptions& inOptions, distinctvalue_iterator& xx)
{
	VTask::Yield();

	BtreePage<Type, MaxCles> *sousBT;
	sLONG i,nb,l;
	VError err = VE_OK;
	Boolean feuillefinale = btp.souspage0 == -1;

	if (InProgress != nil)
	{
		if (!InProgress->Progress(curpage))
			err = ThrowError(VE_DB4D_ACTIONCANCELEDBYUSER, DBaction_GetDistinctValuesWithIndex);
	}

	nb = btp.nkeys;

	if (err == VE_OK)
	{
		if (!feuillefinale)
		{
			sousBT = GetSousPage(curstack, 0, err, context);
			if (sousBT != nil && err == VE_OK)
			{
				curpage++;
				err = sousBT->GetDistinctValues(curstack, outCollection, context, filtre, curpage, curvalue, curkey, dejaval, InProgress, inOptions, xx);
				sousBT->Free(curstack, true);
			}
		}

		for (i=0;i<nb && err == VE_OK;i++)
		{
			Boolean add = false;

			l = GetQui(i);
			if (IsCluster)
			{
				if (l>=0)
				{
					if (filtre == nil || filtre->isOn(l))
					{
						add = true;
					}
				}
				else
				{
					if (filtre == nil)
						add = true;
					else
					{
						if (entete->GetClusterSel(curstack)->IntersectSel(curstack, l, filtre, context, err))
						{
							add = true;
						}
					}
				}

				if (add && !inOptions.IsDiacritical())
				{
					add = false;
					if (dejaval)
					{
						if (CompareKeysPtr(i, &curkey, inOptions) != CR_EQUAL)
						{
							curkey = GetItemPtr(i)->cle;
							add = true;
						}
					}
					else
					{
						dejaval = true;
						add = true;
						curkey = GetItemPtr(i)->cle;
					}
				}

				if (add)
				{
					err = AddElemIntoCollection<Type>(curkey, outCollection, ((IndexInfoFromField*)entete)->GetDataKind(), xx);
				}

			}
			else
			{
				if (filtre == nil || filtre->isOn(l))
				{
					if (dejaval)
					{
						if (CompareKeysPtr(i, &curkey, inOptions) != CR_EQUAL)
						{
							curkey = GetItemPtr(i)->cle;
							add = true;
						}
					}
					else
					{
						dejaval = true;
						add = true;
						curkey = GetItemPtr(i)->cle;
					}

				}

				if (add)
				{
					err = AddElemIntoCollection<Type>(curkey, outCollection, ((IndexInfoFromField*)entete)->GetDataKind(), xx);
				}

			}

			if (!feuillefinale)
			{
				sousBT = GetSousPage(curstack, i+1, err, context);
				if (sousBT != nil && err == VE_OK)
				{
					curpage++;
					err = sousBT->GetDistinctValues(curstack, outCollection, context, filtre, curpage, curvalue, curkey, dejaval, InProgress, inOptions, xx);
					sousBT->Free(curstack, true);
				}
			}

		}

	}

	return err;
}


template <class Type, sLONG MaxCles>
VError BtreePage<Type, MaxCles>::QuickGetDistinctValues(OccupableStack* curstack, DB4DArrayOfDirectValues<Type> *outCollection, BaseTaskInfo* context, Bittab* filtre, 
							  sLONG &curpage, sLONG &curvalue, Type &curkey, Boolean& dejaval, ProgressEncapsuleur* InProgress, VCompareOptions& inOptions)
{
	VTask::Yield();

	BtreePage<Type, MaxCles> *sousBT;
	sLONG i,nb,l;
	VError err = VE_OK;
	Boolean feuillefinale = btp.souspage0 == -1;

	if (InProgress != nil)
	{
		if (!InProgress->Progress(curpage))
			err = ThrowError(VE_DB4D_ACTIONCANCELEDBYUSER, DBaction_GetDistinctValuesWithIndex);
	}

	nb = btp.nkeys;

	if (err == VE_OK)
	{
		if (!feuillefinale)
		{
			sousBT = GetSousPage(curstack, 0, err, context);
			if (sousBT != nil && err == VE_OK)
			{
				curpage++;
				err = sousBT->QuickGetDistinctValues(curstack, outCollection, context, filtre, curpage, curvalue, curkey, dejaval, InProgress, inOptions);
				sousBT->Free(curstack, true);
			}
		}

		for (i=0;i<nb && err == VE_OK;i++)
		{
			Boolean add = false;

			l = GetQui(i);
			if (IsCluster)
			{
				if (l>=0)
				{
					if (filtre == nil || filtre->isOn(l))
					{
						add = true;
					}
				}
				else
				{
					if (filtre == nil)
						add = true;
					else
					{
						if (entete->GetClusterSel(curstack)->IntersectSel(curstack, l, filtre, context, err))
						{
							add = true;
						}
					}
				}

				if (add && !inOptions.IsDiacritical())
				{
					add = false;
					if (dejaval)
					{
						if (CompareKeysPtr(i, &curkey, inOptions) != CR_EQUAL)
						{
							curkey = GetItemPtr(i)->cle;
							add = true;
						}
					}
					else
					{
						dejaval = true;
						add = true;
						curkey = GetItemPtr(i)->cle;
					}
				}

				if (add)
				{
					if (! outCollection->AddOneElem(curkey) )
						err = memfull;
				}

			}
			else
			{
				if (filtre == nil || filtre->isOn(l))
				{
					if (dejaval)
					{
						if (CompareKeysPtr(i, &curkey, inOptions) != CR_EQUAL)
						{
							curkey = GetItemPtr(i)->cle;
							add = true;
						}
					}
					else
					{
						dejaval = true;
						add = true;
						curkey = GetItemPtr(i)->cle;
					}

				}

				if (add)
				{
					if (! outCollection->AddOneElem(curkey) )
						err = memfull;
				}

			}

			if (!feuillefinale)
			{
				sousBT = GetSousPage(curstack, i+1, err, context);
				if (sousBT != nil && err == VE_OK)
				{
					curpage++;
					err = sousBT->QuickGetDistinctValues(curstack, outCollection, context, filtre, curpage, curvalue, curkey, dejaval, InProgress, inOptions);
					sousBT->Free(curstack, true);
				}
			}

		}

	}

	return err;
}


template <class Type, sLONG MaxCles>
VError BtreePage<Type, MaxCles>::SortSel(OccupableStack* curstack, sLONG& curfic, sLONG maxnb, Bittab* filtre, Selection* sel, uBOOL ascent, BaseTaskInfo* context, ProgressEncapsuleur* InProgress, Bittab* inWafSelbits, WafSelection* outWafSel)
{
	VTask::Yield();
	BtreePage<Type, MaxCles> *sousBT;
	sLONG i,nb,l;
	VError err = VE_OK;


	if (InProgress != nil)
	{
		if (!InProgress->Progress(curfic))
			err = ThrowError(VE_DB4D_ACTIONCANCELEDBYUSER, DBaction_SortingWithIndex);
	}

	nb = btp.nkeys;

	if (err == VE_OK)
	{
		if (ascent)
		{
			sousBT = GetSousPage(curstack, 0, err, context);
			if (sousBT != nil && err == VE_OK)
			{
				err = sousBT->SortSel(curstack, curfic, maxnb, filtre, sel, ascent, context, InProgress, inWafSelbits, outWafSel);
				sousBT->Free(curstack, true);
			}

			for (i=0;i<nb && err == VE_OK;i++)
			{

				l = GetQui(i);
				if (IsCluster)
				{
					err = entete->GetClusterSel(curstack)->AddSelToSel(curstack, curfic, maxnb, filtre, sel, l, context, inWafSelbits, outWafSel);
				}
				else
				{
					if (filtre->isOn(l))
					{
						if (curfic<maxnb)
						{
							err = sel->PutFicWithWafSelection(curfic,l, inWafSelbits, outWafSel);
							if (err == VE_OK) 
								++curfic;
							else
							{
								curfic = curfic;
								// put a break here
							}
						}
						else
						{
							curfic = curfic;
							// put a break here
						}
					}
					else
					{
						curfic = curfic;
						// put a break here
					}
				}

				if (err == VE_OK )
				{
					sousBT = GetSousPage(curstack, i+1, err, context);
					if (sousBT != nil)
					{
						err = sousBT->SortSel(curstack, curfic, maxnb, filtre, sel, ascent, context, InProgress, inWafSelbits, outWafSel);
						sousBT->Free(curstack, true);
					}
				}
			}
		}
		else
		{
			for (i=nb-1;i>=0 && err == VE_OK;i--)
			{
				//l = GetQui(i);
				sousBT = GetSousPage(curstack, i+1, err, context);
				if (sousBT != nil)
				{
					err = sousBT->SortSel(curstack, curfic, maxnb, filtre, sel, ascent, context, InProgress, inWafSelbits, outWafSel);
					sousBT->Free(curstack, true);
				}

				if (err == VE_OK)
				{
					l = GetQui(i);
					if (IsCluster)
					{
						err = entete->GetClusterSel(curstack)->AddSelToSel(curstack, curfic, maxnb, filtre, sel, l, context, inWafSelbits, outWafSel);
					}
					else
					{
						if (filtre->isOn(l))
						{
							if (curfic<maxnb)
							{
								err = sel->PutFicWithWafSelection(curfic,l, inWafSelbits, outWafSel);
								if (err == VE_OK) 
									++curfic;
								else
								{
									curfic = curfic;
									// put a break here
								}
							}
							else
							{
								curfic = curfic;
								// put a break here
							}
						}
						else
						{
							curfic = curfic;
							// put a break here
						}
					}
				}

			}

			if (err == VE_OK )
			{
				sousBT = GetSousPage(curstack, 0, err, context);
				if (sousBT != nil)
				{
					err = sousBT->SortSel(curstack, curfic, maxnb, filtre, sel, ascent, context, InProgress, inWafSelbits, outWafSel);
					sousBT->Free(curstack, true);
				}
			}

		}
	}

	if (err != VE_OK)
	{
		curfic = curfic;
		// put a break here
	}
	return err;
}



template <class Type, sLONG MaxCles>
Boolean BtreePage<Type, MaxCles>::FreePageMem(sLONG allocationBlockNumber, VSize combien, VSize& tot)
{
	Boolean oktodelete = true;
	if (btp.souspage0 != -1)
	{
		for (BtreePage<Type, MaxCles> **cur = &(tabmem[0]), **end = &(tabmem[btp.nkeys+1]); cur != end; cur++)
		{
			BtreePage<Type, MaxCles>* souspage = *cur;
			if (souspage != nil)
			{
				if (souspage->FreePageMem(allocationBlockNumber, combien, tot) && !IsOccupied())
				{
					tot = tot + sizeof(BtreePage<Type, MaxCles>);
					souspage->Release();
					//*cur = nil;
					VInterlocked::ExchangePtr(&(*cur), (BtreePage<Type, MaxCles>*)nil);
					/*
					if (tot >= combien)
						break;
						*/
				}
				else
					oktodelete = false;
			}
		}
	}

	/*
	if (tot >= combien)
		oktodelete = false;
	else
	*/
	{
		if (IsOccupied() || modifie() || !OKAllocationNumber(this, allocationBlockNumber))
			oktodelete = false;
	}

	//if (oktodelete)
		//tot = tot + sizeof(BtreePage<Type, MaxCles>);

	return oktodelete;
}


#if debugindex_strong

template <class Type, sLONG MaxCles>
void BtreePage<Type, MaxCles>::debug_checkKeyOrder(CleIndex<Type>* &curkey)
{
	for (sLONG i = 0; i < btp.nkeys; i++)
	{
		if (tabmem[i] != nil)
			tabmem[i]->debug_checkKeyOrder(curkey);
		CleIndex<Type>* x = GetItemPtr(i);
		if (curkey != nil)
		{
			sLONG comp = CompareKeysStrict(curkey, i);
			if (comp == CR_EQUAL)
			{
				if (curkey->qui > x->qui)
				{
					comp = CR_BIGGER;
				}
			}
			assert(comp != CR_BIGGER);
			if (comp == CR_BIGGER)
			{
				comp = comp; // put a break here
			}
		}
		curkey = x;
	}
	if (tabmem[btp.nkeys] != nil)
		tabmem[btp.nkeys]->debug_checkKeyOrder(curkey);
}

#endif


#if debuglr


template <class Type, sLONG MaxCles>
void BtreePage<Type, MaxCles>::checkWrongRecordRef(sLONG limit, CleIndex<Type>* &curval)
{
	DataTable* df = entete->GetTargetTable()->GetDF();
	sLONG nb = btp.nkeys;
	for (sLONG i = 0; i < nb; i++)
	{
		BtreePage<Type, MaxCles>* sous = tabmem[i];
		if (sous != nil)
			sous->checkWrongRecordRef(limit, curval);

		CleIndex<Type>* x = GetItemPtr(i);
		if (curval != nil)
		{
			sLONG result = CompareKeysStrict(curval, x);
			if (result == CR_EQUAL)
				assert(curval->qui < x->qui);
			else
				assert(result = CR_SMALLER);
		}
		curval = x;
		//assert(! df->isRecordDeleted(x->qui));
		assert(x->qui < limit);
	}

	BTreePageIndex* sous = tabmem[nb];
	if (sous != nil)
		sous->checkWrongRecordRef(limit, curval);
}


template <class Type, sLONG MaxCles>
void BtreePage<Type, MaxCles>::CheckPageOwner()
{
	if ( this->GetParent() != nil)
		this->GetParent()->CheckDansParent(this->posdansparent, this);

	sLONG nb = btp.nkeys;
	for (sLONG i = 0; i <= nb; i++)
	{
		BtreePage<Type, MaxCles>* sous = tabmem[i];
		if (sous != nil)
			sous->CheckPageOwner();
	}

}


template <class Type, sLONG MaxCles>
void BtreePage<Type, MaxCles>::CheckPageKeys()
{
	sLONG nb = btp.nkeys-1;
	for (sLONG i = 0; i < nb; i++)
	{
		CleIndex<Type>* val = GetItemPtr(i);
		sLONG rescomp = CompareKeysStrict(i+1, val);
		assert(rescomp != CR_SMALLER);
	}
	nb = btp->nkeys;
	for (sLONG i = 0; i <= nb; i++)
	{
		BtreePage<Type, MaxCles>* sous = tabmem[i];
		if (sous != nil)
			sous->CheckPageKeys();
	}

}


template <class Type, sLONG MaxCles>
void BtreePage<Type, MaxCles>::checkPosSousBT(BtreePage<Type, MaxCles>* sousBT)
{
	assert(sousBT != nil);
	sLONG numsous = sousBT->GetNum();
	sLONG nb = btp.nkeys;
	sLONG found = -1;
	CleIndex<Type>* val = nil;


	for (sLONG i = 0; i <= nb; i++)
	{
		sLONG num = GetSousPageNum(i);
		if (num == numsous)
		{
			found = i;
			break;
		}
	}
	if (testAssert(found != -1))
	{
		CleIndex<Type>* sousBTMin = sousBT->GetItemPtr(0);
		CleIndex<Type>* sousBTMax = sousBT->GetItemPtr(sousBT->btp.nkeys-1);
		if (found != 0)
		{
			assert(CompareKeysStrict(found-1, sousBTMin) != CR_BIGGER);
		}
		if (found != nb)
		{
			assert(CompareKeysStrict(found, sousBTMax) != CR_SMALLER);
		}
	}
}


template <class Type, sLONG MaxCles>
void BtreePage<Type, MaxCles>::DisplayKeys(const VString& message)
{
	DebugMsg(message+L"  :  PageNum = "+ToString(num)+L" , NbKeys = "+ToString(btp->nkeys)+L"\n\n");
	VError err = VE_OK;

	sLONG nb = btp.nkeys;
	for (sLONG i = 0; i < nb; i++)
	{
		CleIndex<Type>* val = GetItemPtr(i);
		ValPtr cv = ConvertKeyToVValue(val->cle, err);
		VString s;
		cv->GetString(s);
		delete cv;
		DebugMsg(s+L" : " + ToString(val->qui)+ L"\n");
	}

	DebugMsg(L"\n\n\n");
}

		
template <class Type, sLONG MaxCles>
void BtreePage<Type, MaxCles>::Display(void)
{
	/*
	BTreePageIndex *sousBT;
	sLONG i,nb;
	IndexValue *val;
	VError err;
	VStr255 s;

	Occupy();
	
	nb=btp->nkeys;
	
	sousBT=GetSousPage(0, err);
	if (sousBT!=nil)
	{
		sousBT->Display();
		sousBT->Free(curstack);
	}
	for (i=0;i<nb;i++)
	{
		val=loadval(i);
		val->ChangeToText(&s);
//		UnivStrDebug(s);
		
		sousBT=GetSousPage(i+1, err);
		if (sousBT!=nil)
		{
			sousBT->Display();
			sousBT->Free(curstack);
		}
	}

	Free(curstack);
	*/
	
}
	
#if debuglr == 112

template <class Type, sLONG MaxCles>
void BtreePage<Type, MaxCles>::checktabmem(void)
{
	sLONG i,nb;
	BtreePage<Type, MaxCles>* sousBT;
	
	nb=btp->nkeys;
	for (i=0;i<=nb;i++)
	{
		sousBT=tabmem[i];
		if (sousBT!=nil)
		{
			if ( (sousBT->posdansparent!=i) || (sousBT->parent!=this) )
			{
				assert(false);
				i=i;
			}
		}
	}
}
#endif

#endif



						/* -------------------------------------------------------------- */



template <class Type, sLONG MaxCles>
void IndexHeaderBTreeScalar<Type,MaxCles>::Update(Table* inTable)
{
	cls.SetDataTable(inTable->GetDF());
}


template <class Type, sLONG MaxCles>
VError IndexHeaderBTreeScalar<Type,MaxCles>::PlaceCle(OccupableStack* curstack, FullCleIndex<Type, MaxCles>& Key, sLONG xqui, BaseTaskInfo* context)
{
	BtreePage<Type, MaxCles> *pag, *first;
	uBOOL h;
	VError err;
	
	Occupy(curstack, true);
	
	if (Key.IsNull())
		err = AddToNulls(curstack, xqui, context);
	else
	{
		Key.qui = xqui;
		h = false;
		err = ChargeFirstPage(curstack, context, true);
		if (err == VE_OK)
		{
			first = firstpage;
			err = first->Place(curstack, &h, Key, context);
			if (err != VE_OK)
				first->Free(curstack, true);
			else
			{
				if (h)
				{
					pag = BtreePage<Type, MaxCles>::AllocatePage(curstack, entete, context, err);
					if (pag != nil)
					{
						pag->SetSousPage(0, first);
						pag->InsertKey(0, Key, true);
						//Key.sousBT->SetEncours(false);
						Key.sousBT->Free(curstack, true);
						first->Free(curstack, true);
						
						first=pag;
						//pag->SetParent(this,0);
						HBT.FirstPage=pag->GetNum();
						taddr.setmodif(true, entete->GetDB(), context);
						err = pag->savepage(curstack, context);
						pag->Free(curstack, true);
						
						firstpage = first;
					}
				}
				else
				{
					first->Free(curstack, true);
				}
			}
		}
	}
	
	Free(curstack, true);

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTINSERTKEYINTOINDEX, DBaction_InsertingKeyIntoIndex);

	return err;
}


template <class Type, sLONG MaxCles>
VError IndexHeaderBTreeScalar<Type,MaxCles>::DetruireCle(OccupableStack* curstack, FullCleIndex<Type, MaxCles>& Key, sLONG xqui, BaseTaskInfo* context)
{
	uBOOL h;
	VError err;
	uBOOL locdoisdel;

	err=VE_OK;
	Occupy(curstack, true);

	if (Key.IsNull())
	{
		err = DelFromNulls(curstack, xqui, context);
	}
	else
	{
		Key.qui = xqui;
		h=false;

#if debugindex_strong
		((IndexInfoScalar<Type, MaxCles>*)entete)->debug_mustcheck = false;
#endif
		err = ChargeFirstPage(curstack, context, false);

		if (firstpage!=nil)
		{
			if (err == VE_OK) 
				err = firstpage->Detruire(curstack, &h, Key, &locdoisdel, context);

			if (err != VE_OK)
			{
				firstpage->Free(curstack, true);
			}
			else
			{
				if (h)
				{
					if (firstpage->GetNKeys()==0)
					{
						BtreePage<Type, MaxCles>* sousbt = firstpage->GetSousPage(curstack, 0, err, context);
						firstpage->LiberePage(curstack, context);
//						firstpage->PrepareToDelete();
						firstpage->Free(curstack, true);
						firstpage->Release();
						firstpage=sousbt;
						if (firstpage != nil)
						{
//							firstpage->SetParent(this, 0);
							HBT.FirstPage = firstpage->GetNum();
							firstpage->Free(curstack, true);
						}
						else
							HBT.FirstPage = -1;
						taddr.setmodif(true, entete->GetDB(), context);
					}
					else
					{
						firstpage->Free(curstack, true);
					}
				}	
				else
				{
					firstpage->Free(curstack, true);
				}
			}
		}
#if debugindex_strong
		if (firstpage != nil && ((IndexInfoScalar<Type, MaxCles>*)entete)->debug_mustcheck)
		{
			CleIndex<Type>* curkey = nil;
			firstpage->debug_checkKeyOrder(curkey);
		}
#endif
	}

	Free(curstack, true);

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTDELETEKEYFROMINDEX, DBaction_DeletingKeyFromIndex);
	return(err);
}



template <class Type, sLONG MaxCles>
VError IndexHeaderBTreeScalar<Type,MaxCles>::ChargeFirstPage(OccupableStack* curstack, BaseTaskInfo* context, uBOOL doiscreer)
{
	VError err = VE_OK;
	
	gOccPool.WaitForMemoryManager(curstack);
	if (firstpage==nil)
	{
		gOccPool.EndWaitForMemoryManager(curstack);
		if ((IHD.nbpage==0) || (HBT.FirstPage==-1)) 
		{
			if (doiscreer)
			{
				firstpage = BtreePage<Type, MaxCles>::AllocatePage(curstack, entete, context, err);
				if (firstpage != nil)
				{
					HBT.FirstPage=firstpage->GetNum();
					taddr.setmodif(true, entete->GetDB(), context);
				}
			}
		}
		else
		{
			VTaskLock lock(&fLoadPageMutex);
			if (firstpage == nil)
				firstpage = BtreePage<Type, MaxCles>::LoadPage(curstack, HBT.FirstPage, entete, context, err);
			else
				firstpage->Occupy(curstack, true);
		}
	}
	else
	{
		firstpage->Occupy(curstack, false);
		gOccPool.EndWaitForMemoryManager(curstack);
	}
	
	return err;
}


template <class Type, sLONG MaxCles>
void IndexHeaderBTreeScalar<Type,MaxCles>::SetFirstPage(BtreePage<Type, MaxCles>* first, BaseTaskInfo* context) 
{ 
	firstpage = first;
	if (first == nil)
		HBT.FirstPage = -1;
	else
		HBT.FirstPage = first->GetNum();
	taddr.setmodif(true, entete->GetDB(), context);
}



template <class Type, sLONG MaxCles>
Bittab* IndexHeaderBTreeScalar<Type,MaxCles>::FourcheScalar(OccupableStack* curstack, const FullCleIndex<Type, MaxCles>& val1, uBOOL xstrict1, const FullCleIndex<Type, MaxCles>& val2, uBOOL xstrict2, BaseTaskInfo* context, VError& err, 
									VDB4DProgressIndicator* InProgress, const VCompareOptions& inOptions, Bittab* dejasel, FullCleIndex<Type, MaxCles>* outVal)
{
	Bittab* b;
	sLONG nb;

#if debug_checkIndexFourche
	bool checktwice = true;
	if (err == -3)
	{
		err = VE_OK;
		checktwice = false;
	}
#endif

	err = VE_OK;

	Occupy(curstack, true);

	b = new Bittab;
	if (b!=nil)
	{
		nb=entete->GetMaxNB(context);
		err=b->aggrandit(nb);
		
		if (err==VE_OK)
		{
			err = ChargeFirstPage(curstack, context, false);
			if (firstpage!=nil && err == VE_OK)
			{
				VString session_title;
				gs(1005,28,session_title);
				ProgressEncapsuleur progress(InProgress, IHD.nbpage, session_title, true);
				err = firstpage->FourcheScalar(curstack, b, val1, xstrict1, val2, xstrict2, context, &progress, inOptions, outVal);
				firstpage->Free(curstack, true);
			}
			if (dejasel != nil)
				err = b->And(dejasel, false);
		}
		
		if (err != VE_OK)
		{
			ReleaseRefCountable(&b);
		}
	}
	else
	{
		err = ThrowError(memfull, DBaction_SearchingInIndex);
	}
	
	Free(curstack, true);

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTCOMPLETESCANOFINDEX, DBaction_SearchingInIndex);

#if debug_checkIndexFourche
	if (checktwice && err == 0 && b != nil && outVal == nil)
	{
		bool oldTempNewIndexFourche = gTempNewIndexFourche;
		gTempNewIndexFourche = !gTempNewIndexFourche;
		err = -3;

		Bittab* b2 = FourcheScalar(curstack, val1, xstrict1, val2, xstrict2, context, err, InProgress, inOptions, dejasel, outVal);

		if (b2 != nil)
		{
			Bittab* b3 = b->Clone(err);
			b3->moins(b2);
			if (!b3->IsEmpty())
			{
				sLONG xdebug = 1; // put a break here
				assert(b3->IsEmpty());
			}

			b2->moins(b);
			if (!b2->IsEmpty())
			{
				sLONG xdebug = 1; // put a break here
				assert(b2->IsEmpty());
			}

			QuickReleaseRefCountable(b3);
			QuickReleaseRefCountable(b2);
		}

		gTempNewIndexFourche = oldTempNewIndexFourche;
	}
#endif

	return(b);
}


template <class Type, sLONG MaxCles>
VError IndexHeaderBTreeScalar<Type,MaxCles>::ScanIndex(OccupableStack* curstack, Selection* into, sLONG inMaxRecords, Boolean KeepSorted, Boolean Ascent, BaseTaskInfo* context, Bittab* filtre,sLONG& nbtrouves, VDB4DProgressIndicator* InProgress)
{
	Occupy(curstack, true);
	VError err = ChargeFirstPage(curstack, context, false);
	nbtrouves = 0;
	if (firstpage!=nil && err == VE_OK)
	{
		VString session_title;
		gs(1005,29,session_title);
		ProgressEncapsuleur progress(InProgress, IHD.nbpage, session_title, true);

		err = firstpage->ScanIndex(curstack, into, nbtrouves, inMaxRecords, KeepSorted, Ascent, context, filtre, &progress);
		firstpage->Free(curstack, true);
	}
	Free(curstack, true);
	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTCOMPLETESCANOFINDEX, DBaction_ScanningIndex);
	return err;
}



template <class Type, sLONG MaxCles>
Selection* IndexHeaderBTreeScalar<Type,MaxCles>::SortSel(OccupableStack* curstack, Selection* sel, uBOOL ascent, BaseTaskInfo* context, VError& err, 
														 VDB4DProgressIndicator* InProgress, Boolean TestUnicite, Bittab* inWafSelbits, WafSelection* outWafSel)
{
	Bittab *bb;
	Selection* sel2;
	sLONG nb, curfic;
	err = VE_OK;

	sel2 = nil;

	bb=sel->GenereBittab(context, err);

	Occupy(curstack, true);
	if (bb != nil)
	{
		nb = sel->GetQTfic();
		if (nb > kNbElemInSel)
		{
			sel2 = new LongSel(sel->GetParentFile());
			((LongSel*)sel2)->PutInCache();
		}
		else
		{
			sel2 = new PetiteSel(sel->GetParentFile());
		}

		if (sel2 != nil)
		{
			if (! sel2->FixFic(nb))
			{
				err = ThrowError(memfull, DBaction_SortingOnIndex);
				sel2->Release();
				sel2 = nil;
			}
		}
		else
		{
			err = ThrowError(memfull, DBaction_SortingOnIndex);
		}

		if (sel2 != nil)
		{
			curfic = 0;

			BitSel* nulls = RetainNulls(curstack, err, context);
			if (nulls != nil && ascent)
			{
				sLONG n = nulls->GetTB()->FindNextBit(0);
				while (n != -1 && err == VE_OK)
				{
					if (bb->isOn(n) && curfic < nb)
					{
						err = sel2->PutFicWithWafSelection(curfic, n, inWafSelbits, outWafSel);
						curfic++;
					}
					n = nulls->GetTB()->FindNextBit(n+1);
				}
			}


			Transaction* trans = GetCurrentTransaction(context);
			if (TestUnicite || (trans != nil && trans->ConcernIndex(entete)))
			{
				XBOX::VString session_title;
				gs(1005,8,session_title);	// Indexed Sort: %curValue of %maxValue Keys
				IndexIterator<Type, MaxCles> iter((IndexInfoScalar<Type, MaxCles>*)entete, bb, context, InProgress, session_title, curstack);
				const Type* curkey = nil;

				while (iter.NextKey(context, err, &curfic))
				{
					if (TestUnicite)
					{
						if (curkey != nil)
						{
							if (*curkey == *(iter.GetKey()))
							{
								err = VE_DB4D_DUPLICATED_KEY;
								break;
							}
						}
						curkey = iter.GetKey();
						if (curkey == nil)
						{
							err = ThrowError(memfull, DBaction_SortingOnIndex);
							break;
						}
					}
					if (ascent)
						err = sel2->PutFicWithWafSelection(curfic, iter.GetRecNum(), inWafSelbits, outWafSel);
					else
						err = sel2->PutFicWithWafSelection(nb-curfic-1, iter.GetRecNum(), inWafSelbits, outWafSel);
					if (err != VE_OK)
						break;
					curfic++;
					if (curfic >= nb)
						break;
				}

				assert(curfic <= nb);

				if (nulls != nil && !ascent)
				{
					sLONG n = nulls->GetTB()->FindNextBit(0);
					while (n != -1 && err == VE_OK)
					{
						if (bb->isOn(n) && curfic < nb)
						{
							err = sel2->PutFicWithWafSelection(curfic, n, inWafSelbits, outWafSel);
							curfic++;
						}
						n = nulls->GetTB()->FindNextBit(n+1);
					}
				}

				if (curfic < nb)
				{
					if (err == VE_OK)
					{
						if (ascent)
							sel2->FixFic(curfic);
						else
							sel2->RemoveSelectedRange(0,nb-curfic-1,context);
					}
				}
			}
			else
			{
				err = ChargeFirstPage(curstack, context, false);
				if (firstpage!=nil && err == VE_OK)
				{
					XBOX::VString session_title;
					gs(1005,8,session_title);	// Indexed Sort: %curValue of %maxValue Keys
					ProgressEncapsuleur progress(InProgress, nb, session_title, true);
					err = firstpage->SortSel(curstack, curfic,nb,bb,sel2,ascent, context, &progress, inWafSelbits, outWafSel);
					firstpage->Free(curstack, true);
				}
				assert(curfic <= nb);
				if (nulls != nil && !ascent)
				{
					sLONG n = nulls->GetTB()->FindNextBit(0);
					while (n != -1 && err == VE_OK)
					{
						if (bb->isOn(n) && curfic < nb)
						{
							err = sel2->PutFicWithWafSelection(curfic, n, inWafSelbits, outWafSel);
							curfic++;
						}
						n = nulls->GetTB()->FindNextBit(n+1);
					}
				}
				if (curfic < nb)
				{
					sel2->FixFic(curfic);
				}

			}

			QuickReleaseRefCountable(nulls);
		}

		ReleaseRefCountable(&bb);
	}

	Free(curstack, true);

	if (err != VE_OK)
	{
		if (!TestUnicite)
			err = ThrowError(VE_DB4D_CANNOTSORTONINDEX, DBaction_SortingOnIndex);
		if (sel2 != nil)
			sel2->Release();
		sel2 = nil;
	}
	return(sel2);
}



template <class Type, sLONG MaxCles>
Bittab* IndexHeaderBTreeScalar<Type,MaxCles>::FindKeyInArray(OccupableStack* curstack, DB4DArrayOfValues* values, BaseTaskInfo* context, VError& err, VDB4DProgressIndicator* InProgress, const VCompareOptions& inOptions, Bittab* dejasel)
{
	Bittab* b;
	err = VE_OK;
	sLONG nb;

	Occupy(curstack, true);
	b = new Bittab;
	if (b!=nil)
	{
		nb=entete->GetMaxNB(context);
		err=b->aggrandit(nb);

		if (err==VE_OK)
		{
			err = ChargeFirstPage(curstack, context, false);
			if (firstpage!=nil && err == VE_OK)
			{
				if (values->GetSignature() == 'cons')
				{
					VString session_title;
					gs(1005,28,session_title);
					ProgressEncapsuleur progress(InProgress, IHD.nbpage, session_title, true);

					const Type* curval = ((DB4DArrayOfConstDirectValues<Type>*)values)->First();
					err = firstpage->FindKeyInArrayScalar(curstack, b, (DB4DArrayOfConstDirectValues<Type>*)values, curval, context, inOptions, &progress);
				}
				else
				{
					VString session_title;
					gs(1005,28,session_title);
					ProgressEncapsuleur progress(InProgress, IHD.nbpage, session_title, true);
					assert(false); // ne devrait plus passer par la
					err = firstpage->FindKeyInArray(curstack, b, values, context, inOptions, &progress);
				}
				firstpage->Free(curstack, true);
			}
			if (dejasel != nil)
				err = b->And(dejasel, false);
		}

		if (err != VE_OK)
		{
			b->Release();
			b=nil;
		}
	}
	else
	{
		err = ThrowError(memfull, DBaction_SearchingInIndex);
	}
	Free(curstack, true);

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTCOMPLETESCANOFINDEX, DBaction_SearchingInIndex);
	return(b);
}


template <class Type, sLONG MaxCles>
VError IndexHeaderBTreeScalar<Type,MaxCles>::GetDistinctValues(OccupableStack* curstack, DB4DCollectionManager& outCollection, BaseTaskInfo* context, Bittab* filtre, 
								 VDB4DProgressIndicator* InProgress, VCompareOptions& inOptions)
{
	VError err = VE_OK;
	Type curkey;
	Boolean dejaval = false;
	sLONG curpage = 0, curvalue = 0;

	Occupy(curstack, true);

	if (err == VE_OK)
		err = ChargeFirstPage(curstack, context, false);

	if (firstpage!=nil && err == VE_OK)
	{
		distinctvalue_iterator xx;
		XBOX::VString session_title;
		gs(1005,7,session_title);	// Indexed Distinct Values: %curValue of %maxValue Keys
		ProgressEncapsuleur progress(InProgress, IHD.nbpage,session_title,true);
		err = firstpage->GetDistinctValues(curstack, outCollection, context, filtre, curpage, curvalue, curkey, dejaval, &progress, inOptions, xx);
		firstpage->Free(curstack, true);
		outCollection.SetCollectionSize(xx.nbelem, false);
	}

	Free(curstack, true);

	return err;
}



template <class Type, sLONG MaxCles>
VError IndexHeaderBTreeScalar<Type,MaxCles>::QuickGetDistinctValues(OccupableStack* curstack, QuickDB4DArrayOfValues *outCollection, BaseTaskInfo* context, Bittab* filtre, 
														VDB4DProgressIndicator* InProgress, VCompareOptions& inOptions)
{
	VError err = VE_OK;
	Type curkey;
	Boolean dejaval = false;
	sLONG curpage = 0, curvalue = 0;

	Occupy(curstack, true);

	if (err == VE_OK)
		err = ChargeFirstPage(curstack, context, false);

	if (firstpage!=nil && err == VE_OK)
	{
		XBOX::VString session_title;
		gs(1005,7,session_title);	// Indexed Distinct Values: %curValue of %maxValue Keys
		ProgressEncapsuleur progress(InProgress, IHD.nbpage,session_title,true);
		err = firstpage->QuickGetDistinctValues(curstack, (DB4DArrayOfDirectValues<Type>*)outCollection, context, filtre, curpage, curvalue, curkey, dejaval, &progress, inOptions);
		firstpage->Free(curstack, true);
	}

	Free(curstack, true);

	return err;
}


template <class Type, sLONG MaxCles>
VError IndexHeaderBTreeScalar<Type,MaxCles>::CalculateColumnFormulas(OccupableStack* curstack, ColumnFormulas* formules, BaseTaskInfo* context, Bittab* filtre, VDB4DProgressIndicator* InProgress)
{
	Boolean stop = false;
	sLONG curpage = 0;
	VError err = VE_OK;
	bool okoptime = true;

	for (sLONG i = 0, nb = formules->GetNbActions(); i < nb; i++)
	{
		ColumnFormulae* formule = formules->GetAction(i);
		DB4D_ColumnFormulae action = formule->GetAction();
		if (!(action == DB4D_Sum || action == DB4D_Average || action == DB4D_Count))
		{
			okoptime = false;
		}
	}

	Occupy(curstack, true);

	err = ChargeFirstPage(curstack, context, false);

	if (firstpage!=nil && err == VE_OK)
	{
		XBOX::VString session_title;
		gs(1005,6,session_title);	// Indexed Distinct Values: %curValue of %maxValue Keys
		ProgressEncapsuleur progress(InProgress, IHD.nbpage,session_title,true);
		if (okoptime)
		{
			bool okcurrec = false;
			sLONG currec = formules->GetCurrec();
			Bittab newfiltre;
			if (filtre != nil && currec != -1 && filtre->isOn(currec))
			{
				newfiltre.Or(filtre);
				newfiltre.Clear(currec);
				filtre = &newfiltre;
				okcurrec = true;
			}
			sLONG count = 0;
			sLONG datakind = ((IndexInfoScalar<Type, MaxCles>*)entete)->GetDataKind();
			if (datakind == VK_REAL)
			{
				Real resultsum = 0;
				err = firstpage->CalculateSum(curstack, context, filtre, curpage, &progress, stop, count, resultsum);
				for (sLONG i = 0, nb = formules->GetNbActions(); i < nb; i++)
				{
					ColumnFormulae* formule = formules->GetAction(i);
					DB4D_ColumnFormulae action = formule->GetAction();
					VValueSingle* cv = formule->GetCurrecValue();
					if (okcurrec && cv != nil)
					{
						Real r = cv->GetReal();
						formule->FirstTime();
						count++;
						resultsum += r;
					}

					switch(action)
					{
						case DB4D_Sum:
							if (count > 0)
								formule->SetResult(resultsum, count);
							break;

						case DB4D_Average:
							if (count > 0)
								formule->SetResult(resultsum/* / (Real)count*/, count);
							break;

						case DB4D_Count:
							formule->SetResult((sLONG8)count, count);
							break;
					}
				}
			}
			else
			{
				sLONG8 resultsum = 0;
				err = firstpage->CalculateSum(curstack, context, filtre, curpage, &progress, stop, count, resultsum);
				for (sLONG i = 0, nb = formules->GetNbActions(); i < nb; i++)
				{
					ColumnFormulae* formule = formules->GetAction(i);
					DB4D_ColumnFormulae action = formule->GetAction();
					VValueSingle* cv = formule->GetCurrecValue();
					if (okcurrec && cv != nil)
					{
						sLONG8 l = cv->GetLong8();
						formule->FirstTime();
						count++;
						resultsum += l;
					}
					switch(action)
					{
						case DB4D_Sum:
							if (count > 0)
								formule->SetResult(resultsum, count);
							break;

						case DB4D_Average:
							if (count > 0)
								formule->SetResult(resultsum/* / (Real)count*/, count);
							break;

						case DB4D_Count:
							formule->SetResult((sLONG8)count, count);
							break;
					}
				}
			}
		}
		else
			err = firstpage->CalculateColumnFormulas(curstack, formules, context, filtre, curpage, &progress, stop);
		firstpage->Free(curstack, true);
	}

	Free(curstack, true);

	return err;
}


template <class Type, sLONG MaxCles>
void IndexHeaderBTreeScalar<Type,MaxCles>::TryToFreeMem(sLONG allocationBlockNumber, VSize combien, VSize& tot)
{
	tot = 0;
	if (fIsCluster)
		cls.TryToFreeMem(allocationBlockNumber, combien, tot);

	if (/*tot < combien && */firstpage != nil)
	{
		VSize tot2 = 0;
		if (firstpage->FreePageMem(allocationBlockNumber, combien-tot, tot2) && !IsOccupied())
		{
			tot2 = tot2 + sizeof(BtreePage<Type, MaxCles>);
			assert(firstpage->GetRefCount() == 1);
			firstpage->Release();
			firstpage = nil;
		}
		tot = tot + tot2;
		//if (tot < combien)
		{
			taddr.TryToFreeMem(allocationBlockNumber, combien-tot, tot2);
			tot = tot + tot2;
		}
	}
}



						/* -------------------------------------------------------------- */



template <class Type, sLONG MaxCles>
VError IndexInfoScalar<Type,MaxCles>::TransPlaceCle(Transaction* trans, FullCleIndex<Type, MaxCles>& val, sLONG numrec)
{
	VError err = VE_OK;
	Boolean KeyWasFoundInDeletedMap = false;

	if (val.IsNull())
	{
		Bittab* b = trans->GetNullKeys(this, true, err);
		if (err == VE_OK && b != nil)
		{
			err = b->Set(numrec, true);
		}
		Bittab* b2 = trans->GetDeletedNullKeys(this, false, err);
		if (err == VE_OK && b2 != nil)
		{
			err = b2->Clear(numrec, false);
		}
	}
	else
	{
		TransCleIndex<Type> Key;
		val.CopyTo(Key);

		Key.qui = numrec;

		mapKeys* vals = (mapKeys*)trans->GetDeletedKeys(this, true, err);
		if (err == VE_OK && vals != nil)
		{
			typename mapKeys::iterator foundkey = vals->find(Key);
			if (foundkey != vals->end())
			{
				KeyWasFoundInDeletedMap = true;
				vals->erase(foundkey);
			}
		}

		if (err == VE_OK && !KeyWasFoundInDeletedMap)
		{
			mapKeys* vals = (mapKeys*)trans->GetSavedKeys(this, true, err);
			if (err == VE_OK)
			{
				assert(vals != nil);

				try
				{
					vals->insert(Key);
				}
				catch (...)
				{
					err = ThrowBaseError(memfull, DBaction_ExecutingTransaction);
				}
			}
		}

	}
	if (err != VE_OK)
		trans->SetValid(false);
	return err;
}


template <class Type, sLONG MaxCles>
VError IndexInfoScalar<Type,MaxCles>::TransDetruireCle(Transaction* trans, FullCleIndex<Type, MaxCles>& val, sLONG numrec)
{
	VError err = VE_OK;
	Boolean KeyWasFoundInSavedMap = false;

	if (val.IsNull())
	{
		Bittab* b = trans->GetNullKeys(this, false, err);
		if (b != nil && err == VE_OK)
		{
			err = b->Clear(numrec, false);
		}
		Bittab* b2 = trans->GetDeletedNullKeys(this, true, err);
		if (b2 != nil && err == VE_OK)
		{
			err = b2->Set(numrec, true);
		}
	}
	else
	{
		TransCleIndex<Type> Key;
		val.CopyTo(Key);

		Key.qui = numrec;


		mapKeys* vals = (mapKeys*)trans->GetSavedKeys(this, true, err);
		if (err == VE_OK && vals != nil)
		{
			typename mapKeys::iterator foundkey = vals->find(Key);
			if (foundkey != vals->end())
			{
				KeyWasFoundInSavedMap = true;
				vals->erase(foundkey);
			}
		}

		if (err == VE_OK && !KeyWasFoundInSavedMap)
		{
			mapKeys* vals = (mapKeys*)trans->GetDeletedKeys(this, true, err);
			if (err == VE_OK)
			{
				assert(vals != nil);

				try
				{
					vals->insert(Key);
				}
				catch (...)
				{
					err = ThrowBaseError(memfull, DBaction_ExecutingTransaction);
				}
			}
		}

	}

	if (err != VE_OK)
		trans->SetValid(false);
	return err;
}



template <class Type, sLONG MaxCles>
void IndexInfoScalar<Type,MaxCles>::BuildFullCle(FicheInMem *rec, FullCleIndex<Type, MaxCles>& outKey, VError &err, Boolean OldOne)
{
	ValPtr cv;
	//err = VE_OK;
	assert(rec != nil);
	if (OldOne)
		cv = rec->GetNthFieldOld(crit->GetPosInRec(), err, true);
	else
		cv = rec->GetNthField(crit->GetPosInRec(), err, false, true);
		
	if (cv == nil || cv->IsNull())
		outKey.SetNull(true);
	else
		cv->WriteToPtr(&outKey.cle, false);
}
						
						
template <class Type, sLONG MaxCles>
VError IndexInfoScalar<Type,MaxCles>::PlaceCle(OccupableStack* curstack, FicheInMem *rec, BaseTaskInfo* context)
{
	VError err = VE_OK;
	if (fIsRemote)
		err = ThrowError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
	else
	{
		FullCleIndex<Type, MaxCles> val;
		BuildFullCle(rec, val, err, false);

		if (err == VE_OK)
		{
			Transaction* trans = GetCurrentTransaction(context);
			if (trans == nil)
			{
				err = GetHeaderScalar()->PlaceCle(curstack, val, rec->GetAssoc()->getnumfic(), context);
			}
			else
			{
				err = TransPlaceCle(trans, val, rec->GetAssoc()->getnumfic());
			}
		}
	}
	return err;
}


template <class Type, sLONG MaxCles>
VError IndexInfoScalar<Type,MaxCles>::DetruireCle(OccupableStack* curstack, FicheInMem *rec, BaseTaskInfo* context)
{
	VError err = VE_OK;
	if (fIsRemote)
		err = ThrowError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
	else
	{
		FullCleIndex<Type, MaxCles> val;
		BuildFullCle(rec, val, err, true);

		if (err == VE_OK)
		{
			Transaction* trans = GetCurrentTransaction(context);
			if (trans == nil)
			{
				err = GetHeaderScalar()->DetruireCle(curstack, val, rec->GetAssoc()->getnumfic(), context);
			}
			else
			{
				err = TransDetruireCle(trans, val, rec->GetAssoc()->getnumfic());
			}
		}
	}
	return err;
}

						
						
template <class Type, sLONG MaxCles>
Boolean IndexInfoScalar<Type,MaxCles>::GenereQuickIndex(VError &err, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, sLONG MaxElemsInPage, sLONG RequiredElemsInPage)
{
	Boolean result = true;
	TypeSortElemArray<Type> tempsort;
	Selection* into = nil;
	tempsort.SetBreakOnNonUnique(false);

	xMultiFieldDataOffsets off(crit, fDataKind, true, sizeof(Type));
	result = tempsort.TryToSort((Selection*)nil, into, err, off, context, InProgress, fUniqueKeys, sizeof(Type));
	if (result)
	{
		if (fUniqueKeys && !tempsort.OKUnique())
		{
			fUniqueKeys = false;
			crit->SetUnique(false, InProgress, context == nil ? nil : context->GetEncapsuleur());
		}
		if (!GetHeaderScalar()->IsCluster())
			err = tempsort.GenereIndexScalar(this, MaxCles, MaxCles, context, InProgress, fDataKind, sizeof(Type));
		else
			err = tempsort.GenereClusterIndexScalar(this, MaxCles, MaxCles, context, InProgress, fDataKind, sizeof(Type), off);
	}
	
	return result;
}


template <class Type, sLONG MaxCles>
VError IndexInfoScalar<Type,MaxCles>::GenereIndex(BaseTaskInfo* context, VDB4DProgressIndicator* InProgress)
{
	DataTable *FD;
	sLONG i,nb;
	VError err = VE_OK;
	FicheInMem *rec;
	ValPtr cv;
	
	if (fIsRemote)
		err = ThrowError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
	else
	{
		FD=fic->GetDF();
		nb=FD->GetMaxRecords(context);
		SetMaxElemEnCreation(nb);
		
		if (InProgress != nil)
		{
			XBOX::VString session_title;
			gs(1005,10,session_title);	// Building Index on one Field: %curValue of %maxValue Records
			InProgress->BeginSession(nb,session_title,false);
		}
		
		ReadAheadBuffer* buf = fic->GetOwner()->NewReadAheadBuffer();

		OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
		for (i=0;i<nb && err == VE_OK;i++)
		{
			if (InProgress != nil)
			{
				if (!InProgress->Progress(i))
					 err = ThrowError(VE_DB4D_ACTIONCANCELEDBYUSER, DBaction_BuildingIndex);
			}
				
			rec=FD->LoadRecord(i, err, DB4D_Do_Not_Lock, context, false, false, buf);
			if (rec!=nil)
			{
				FullCleIndex<Type, MaxCles> val;
				BuildFullCle(rec, val, err, false);
				if (err == VE_OK)
				{
					Open(index_write);
					err=GetHeaderScalar()->PlaceCle(curstack, val, i, context);
					EnCreation(i);
					Close();
				}
				rec->Release();
			}

		}

		if (buf != nil)
			buf->Release();
		
		LockValidity();
		FinCreation();
		UnLockValidity();

		if (InProgress != nil)
		{
			InProgress->EndSession();
		}
	}

	return(err);
}


template <class Type, sLONG MaxCles>
VError IndexInfoScalar<Type,MaxCles>::TransFourche(Transaction* trans, const FullCleIndex<Type, MaxCles>& val1, uBOOL xstrict1, const FullCleIndex<Type, MaxCles>& val2,  
													uBOOL xstrict2, const VCompareOptions& inOptions, Bittab* dejasel, Bittab* into, FullCleIndex<Type, MaxCles>* outVal)
{
	VError err = VE_OK;

	mapKeys* vals;
	if (outVal == nil)
	{
		vals = (mapKeys*)trans->GetDeletedKeys(this, false, err);
		if (vals != nil)
		{
			if (val1.IsNull())
			{
				if (!val2.IsNull())
				{
					typename mapKeys::const_iterator cur = vals->begin(), end = vals->end();
					Boolean upperbound = false;
					while (cur != end && !upperbound && err == VE_OK)
					{
						if (CompareKeys(*cur, val2, inOptions) == CR_BIGGER)
							upperbound = true;
						else
						{
							sLONG numfic = cur->qui;
							{
								err = into->Clear(numfic);
							}
						}
						cur++;
					}
				}
			}
			else
			{
				typename mapKeys::const_iterator cur = vals->lower_bound(val1.ConvertToTransCle()), end = vals->end();
				Boolean upperbound = false;
				Boolean lowerbound = false;

				
				if (fDataKind == VK_REAL && cur != end)
				{
					typename mapKeys::const_reverse_iterator cur2 = typename mapKeys::const_reverse_iterator(cur);
					typename mapKeys::const_reverse_iterator end2 = vals->rend();

					while (cur2 != end2 && !lowerbound && err == VE_OK)
					{
						if (CompareKeys(*cur2, val1, inOptions) != CR_EQUAL)
						{
							lowerbound = true;
						}
						else
						{
							sLONG numfic = cur2->qui;
							{
								err = into->Clear(numfic);
							}
						}
						cur2++;
					}
				}

				while (cur != end && !upperbound && err == VE_OK)
				{
					if ((!val2.IsNull()) && CompareKeys(*cur, val2, inOptions) == CR_BIGGER)
						upperbound = true;
					else
					{
						sLONG numfic = cur->qui;
						{
							err = into->Clear(numfic);
						}
					}
					cur++;
				}
			}
		}
	}

	if (err == VE_OK)
	{
		vals = (mapKeys*)trans->GetSavedKeys(this, false, err);
		if (vals != nil)
		{
			if (val1.IsNull())
			{
				if (!val2.IsNull())
				{
					typename mapKeys::const_iterator cur = vals->begin(), end = vals->end();
					Boolean upperbound = false;
					while (cur != end && !upperbound && err == VE_OK)
					{
						sLONG comp2 = CompareKeys(*cur, val2, inOptions);
						if (comp2 == CR_BIGGER || (comp2 == CR_EQUAL && xstrict2))
							upperbound = true;
						else
						{
							sLONG numfic = cur->qui;
							if (dejasel == nil || dejasel->isOn(numfic))
							{
								err = into->Set(numfic);
								if (outVal != nil)
								{
									*outVal = *cur;
									break;
								}
							}
						}
						cur++;
					}
				}
			}
			else
			{
				typename mapKeys::const_iterator cur = vals->lower_bound(val1.ConvertToTransCle()), end = vals->end();
				Boolean upperbound = false;
				Boolean lowerbound = false;

				if (fDataKind == VK_REAL && cur != end)
				{
					typename mapKeys::const_reverse_iterator cur2 = typename mapKeys::const_reverse_iterator(cur);
					typename mapKeys::const_reverse_iterator end2 = vals->rend();


					while (cur2 != end2 && !lowerbound && err == VE_OK)
					{
						if (CompareKeys(*cur2, val1, inOptions) != CR_EQUAL)
						{
							lowerbound = true;
						}
						else
						{
							if (!xstrict1)
							{
								sLONG numfic = cur2->qui;
								if (dejasel == nil || dejasel->isOn(numfic))
								{
									err = into->Set(numfic);
									if (outVal != nil)
									{
										*outVal = *cur2;
										upperbound = true;
										break;
									}
								}
							}
						}
						cur2++;
					}
				}

				while (cur != end && !upperbound && err == VE_OK)
				{
					bool ok = true;
					if (xstrict1 && CompareKeys(*cur, val1, inOptions) == CR_EQUAL)
						ok = false;

					if ((!val2.IsNull()) && CompareKeys(*cur, val2, inOptions) == CR_BIGGER)
						upperbound = true;
					else
					{
						if (ok)
						{
							sLONG numfic = cur->qui;
							if (dejasel == nil || dejasel->isOn(numfic))
							{
								err = into->Set(numfic);
								if (outVal != nil)
								{
									*outVal = *cur;
									break;
								}
							}
						}
					}
					cur++;
				}
			}
		}
	}

	return err;
}


template <class Type, sLONG MaxCles>
Bittab* IndexInfoScalar<Type,MaxCles>::Fourche(BTitemIndex* val1, uBOOL xstrict1, BTitemIndex* val2, uBOOL xstrict2, BaseTaskInfo* context, VError& err, 
													 VDB4DProgressIndicator* InProgress, const VCompareOptions& inOptions, Bittab* dejasel, BTitemIndex** outVal)
{
	Bittab* b = nil;

	if (fIsRemote)
		err = ThrowError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
	else
	{
		FullCleIndex<Type, MaxCles> fullkey1(val1), fullkey2(val2), *outResult, result;
		if (outVal == nil)
			outResult = nil;
		else
		{
			outResult = &result;
			result.SetNull(true);
		}
		
		Open(index_read);
		OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
		VCompareOptions options = inOptions;
		options.SetIntlManager(GetContextIntl(context));
		if (Invalid)
			//SetError(kErr4DInvalidIndex);
			ThrowError(VE_DB4D_INVALIDINDEX, DBaction_SearchingInIndex);
		else
		{
			Open(index_read);
			b = GetHeaderScalar()->FourcheScalar(curstack, fullkey1, xstrict1, fullkey2, xstrict2, context, err, InProgress, options, dejasel, outResult);
			Close();
		}
		Close();

		Transaction* trans = GetCurrentTransaction(context);
		if (trans != nil && err == VE_OK)
		{
			assert(b != nil);
			err = TransFourche(trans, fullkey1, xstrict1, fullkey2, xstrict2, options, dejasel, b, outResult);
		}

		if (err == VE_OK && outResult != nil && !outResult->IsNull())
		{
			*outVal = outResult->BuildBTItem(this);
		}
	}

	return(b);
}


template <class Type, sLONG MaxCles>
VError IndexInfoScalar<Type,MaxCles>::RemoveDeleteKeysFromFourche(Transaction* trans, const FullCleIndex<Type, MaxCles>& val1, uBOOL xstrict1, const FullCleIndex<Type, MaxCles>& val2, 
																	uBOOL xstrict2, const VCompareOptions& inOptions, Bittab* dejasel)
{
	VError err = VE_OK;

	mapKeys* vals = (mapKeys*)trans->GetDeletedKeys(this, false, err);
	if (vals != nil)
	{
		typename mapKeys::const_iterator cur = vals->lower_bound(val1.ConvertToTransCle()), end = vals->end();
		Boolean upperbound = false;
		Boolean lowerbound = false;

		if (fDataKind != VK_REAL && cur != end)
		{
			//mapIndexKeys::const_reverse_iterator cur2 = reverse_bidirectional_iterator<mapIndexKeys::iterator, BTitemIndexHolder, mapIndexKeys::reference, mapIndexKeys::difference_type>(cur);
			typename mapKeys::const_reverse_iterator cur2 = typename mapKeys::const_reverse_iterator(cur);
			typename mapKeys::const_reverse_iterator end2 = vals->rend();

			while (cur2 != end2 && !lowerbound && err == VE_OK)
			{
				if (CompareKeys(*cur2, val1, inOptions) != CR_EQUAL)
				{
					lowerbound = true;
				}
				else
				{
					sLONG numfic = cur2->qui;
					{
						err = dejasel->Set(numfic, true);
					}
				}
				cur2++;
			}
		}

		while (cur != end && !upperbound && err == VE_OK)
		{
			if (CompareKeys(*cur, val2, inOptions) == CR_BIGGER)
			//if (cur->cle > val2.cle)
				upperbound = true;
			else
			{
				sLONG numfic = cur->qui;
				{
					err = dejasel->Set(numfic, true);
				}
			}
			cur++;
		}
	}
	return err;
}


template <class Type, sLONG MaxCles>
sLONG IndexInfoScalar<Type,MaxCles>::FindKey(BTitemIndex* val, BaseTaskInfo* context, const VCompareOptions& inOptions, Bittab* dejasel, BTitemIndex** outVal) 
{ 
	VCompareOptions options = inOptions;
	options.SetIntlManager(GetContextIntl(context));
	FullCleIndex<Type, MaxCles> fullkey(val), *outResult, result;
	Bittab dejasel2;
	VError err = VE_OK;
	Bittab* b;
	sLONG res = -1;
	
	if (fIsRemote)
		err = ThrowError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
	else
	{
		Transaction* trans = GetCurrentTransaction(context);
		if (trans != nil && outVal != nil)
		{
			RemoveDeleteKeysFromFourche(trans, fullkey, false, fullkey, false, inOptions, &dejasel2);
		}

		if (outVal == nil)
			outResult = nil;
		else
		{
			outResult = &result;
			result.SetNull(true);
		}

		Open(index_read);
		OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
		b = GetHeaderScalar()->FourcheScalar(curstack, fullkey, false, fullkey, false, context, err, nil, options, dejasel, outResult);
		Close();
		
		if (err == VE_OK)
		{
			if (trans != nil)
			{			
				assert(b != nil);
				err = TransFourche(trans, fullkey, false, fullkey, false, inOptions, dejasel, b, outResult);
				if (outVal != nil)
				{
					err = b->moins(&dejasel2);
				}
			}
			res = b->FindNextBit(0);
		}

		ReleaseRefCountable(&b);
		
		if (err == VE_OK && outResult != nil && !outResult->IsNull())
		{
			*outVal = outResult->BuildBTItem(this);
		}
	}
	
	return res;
}



template <class Type, sLONG MaxCles>
VError IndexInfoScalar<Type,MaxCles>::PlaceCleAllForTrans(void* xvals, BaseTaskInfo* context)
{
	VError err = VE_OK;
	if (fIsRemote)
		err = ThrowError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
	else
	{
		mapKeys* vals = (mapKeys*)xvals;
		
		Open(index_write);
		OccupableStack* curstack = gOccPool.GetStackForCurrentThread();

		for (typename mapKeys::iterator cur = vals->begin(), end = vals->end(); cur != end; cur++)
		{
	//		if (!cur->IsNull())
			{
				FullCleIndex<Type, MaxCles> key(*cur);
				GetHeaderScalar()->DelFromNulls(curstack, key.qui, context);
				GetHeaderScalar()->PlaceCle(curstack, key, key.qui, context);
			}
		}

		Close();
	}
	return err;
}
	
			
template <class Type, sLONG MaxCles>
VError IndexInfoScalar<Type,MaxCles>::DetruireAllCleForTrans(void* xvals, BaseTaskInfo* context)
{
	VError err = VE_OK;
	if (fIsRemote)
		err = ThrowError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
	else
	{
		mapKeys* vals = (mapKeys*)xvals;
		
		Open(index_write);
		OccupableStack* curstack = gOccPool.GetStackForCurrentThread();

		for (typename mapKeys::iterator cur = vals->begin(), end = vals->end(); cur != end; cur++)
		{
	//		if (!cur->IsNull())
			{
				FullCleIndex<Type, MaxCles> key(*cur);
				GetHeaderScalar()->DetruireCle(curstack, key, key.qui, context);
			}
		}

		Close();
	}
	return err;
}



template <class Type, sLONG MaxCles>
VError IndexInfoScalar<Type,MaxCles>::GetDistinctValues(DB4DCollectionManager& outCollection, BaseTaskInfo* context, Bittab* filtre, 
														VDB4DProgressIndicator* InProgress, const VCompareOptions& inOptions)
{
	VError err = VE_OK;

	if (fIsRemote)
		err = ThrowError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
	else
	{
		distinctvalue_iterator xx;

		Open(index_read);
		OccupableStack* curstack = gOccPool.GetStackForCurrentThread();

		Transaction* curtrans = GetCurrentTransaction(context);
		if (curtrans != nil && curtrans->ConcernIndex(this))
		{
			XBOX::VString session_title;
			gs(1005,7,session_title);	// Indexed Distinct Values: %curValue of %maxValue Keys

			IndexIterator<Type,MaxCles> iter(this, filtre, context, InProgress, session_title, curstack);
			const Type* curkey = nil;

			sLONG curvalue = 0;
			while (iter.NextKey(context, err) && err == VE_OK)
			{
				Boolean add = false;
				if (curkey != nil)
				{
					//if (entete->CompareKeys(iter.GetKey(), curkey, inOptions) != CR_EQUAL)
					if (*curkey != *(iter.GetKey()))
					{
						curkey = iter.GetKey();
						add = true;
					}
				}
				else
				{
					add = true;
					curkey = iter.GetKey();
				}

				if (add)
				{
					err = AddElemIntoCollection<Type>(*curkey, outCollection, fDataKind, xx);
					curvalue++;
				}
			}
			outCollection.SetCollectionSize(xx.nbelem, false);

		}
		else 
		{ 
			VCompareOptions options = inOptions;
			options.SetIntlManager(GetContextIntl(context));
			err = GetHeaderScalar()->GetDistinctValues(curstack, outCollection, context, filtre, InProgress, options);
		}

		Close();
	}

	return err;
}



template <class Type, sLONG MaxCles>
VError IndexInfoScalar<Type,MaxCles>::QuickGetDistinctValues(QuickDB4DArrayOfValues *outCollection, BaseTaskInfo* context, Bittab* filtre, 
														VDB4DProgressIndicator* InProgress, const VCompareOptions& inOptions)
{
	VError err = VE_OK;

	if (fIsRemote)
		err = ThrowError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
	else
	{
		distinctvalue_iterator xx;
		Open(index_read);
		OccupableStack* curstack = gOccPool.GetStackForCurrentThread();

		Transaction* curtrans = GetCurrentTransaction(context);
		if (curtrans != nil && curtrans->ConcernIndex(this))
		{
			XBOX::VString session_title;
			gs(1005,7,session_title);	// Indexed Distinct Values: %curValue of %maxValue Keys

			IndexIterator<Type,MaxCles> iter(this, filtre, context, InProgress, session_title, curstack);
			const Type* curkey = nil;

			sLONG curvalue = 0;
			while (iter.NextKey(context, err) && err == VE_OK)
			{
				Boolean add = false;
				if (curkey != nil)
				{
					//if (entete->CompareKeys(iter.GetKey(), curkey, inOptions) != CR_EQUAL)
					if (*curkey != *(iter.GetKey()))
					{
						curkey = iter.GetKey();
						add = true;
					}
				}
				else
				{
					add = true;
					curkey = iter.GetKey();
				}

				if (add)
				{
					if (! ((DB4DArrayOfDirectValues<Type>*)outCollection)->AddOneElem(*curkey) )
						err = memfull;
					curvalue++;
				}
			}

		}
		else
		{
			VCompareOptions options = inOptions;
			options.SetIntlManager(GetContextIntl(context));
			err = GetHeaderScalar()->QuickGetDistinctValues(curstack, outCollection, context, filtre, InProgress, options);
		}

		Close();
	}

	return err;
}



template <class Type, sLONG MaxCles>
Bittab* IndexInfoScalar<Type,MaxCles>::FindKeyInArray(DB4DArrayOfValues* values, BaseTaskInfo* context, VError& err, 
											VDB4DProgressIndicator* InProgress, const VCompareOptions& inOptions, Bittab* dejasel)
{
	Bittab* b = nil;

	if (fIsRemote)
		err = ThrowError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
	else
	{
		Open(index_read);
		OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
		VCompareOptions options = inOptions;
		options.SetIntlManager(GetContextIntl(context));
		if (Invalid)
			//SetError(kErr4DInvalidIndex);
			err = ThrowError(VE_DB4D_INVALIDINDEX, DBaction_SearchingInIndex);
		else
			b = GetHeaderScalar()->FindKeyInArray(curstack, values, context, err, InProgress, options, dejasel);
		Close();

		if (err == VE_OK && b != nil)
		{
			Transaction* trans = GetCurrentTransaction(context);
			if (trans != nil)
			{
				if (values->GetSignature() == 'cons')
				{
					DB4DArrayOfConstDirectValues<Type>* xvalues = (DB4DArrayOfConstDirectValues<Type>*)values;

					mapKeys* vals;
					vals = (mapKeys*)trans->GetDeletedKeys(this, false, err);
					if (vals != nil)
					{
						for (typename mapKeys::const_iterator cur = vals->begin(), end = vals->end(); cur != end; cur++)
						{
							if (xvalues->FindDirect(&(cur->cle), inOptions))
							{
								b->Clear(cur->qui);
							}
						}
					}

					vals = (mapKeys*)trans->GetSavedKeys(this, false, err);
					if (vals != nil)
					{
						for (typename mapKeys::const_iterator cur = vals->begin(), end = vals->end(); cur != end; cur++)
						{
							if (xvalues->FindDirect(&(cur->cle), inOptions))
							{
								b->Set(cur->qui);
							}
						}
					}
				}
				else
				{
					assert(false);
					// ne devrait plus arriver
				}

			}

		}

	}
	return(b);
}




template <class Type, sLONG MaxCles>
VError IndexInfoScalar<Type,MaxCles>::CalculateColumnFormulas(ColumnFormulas* formules, BaseTaskInfo* context, Bittab* filtre, VDB4DProgressIndicator* InProgress)
{
	VError err = VE_OK;
	sLONG curpage = 0;
	Boolean stop = false;

	if (fIsRemote)
		err = ThrowError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
	else
	{
		Open(index_read);
		OccupableStack* curstack = gOccPool.GetStackForCurrentThread();

		Transaction* curtrans = GetCurrentTransaction(context);
		if (curtrans != nil && curtrans->ConcernIndex(this))
		{
			XBOX::VString session_title;
			gs(1005,27,session_title);	// Indexed Calculate Formulas: %curValue of %maxValue Pages
			IndexIterator<Type,MaxCles> iter(this, filtre, context, InProgress, session_title, curstack);

			while (iter.NextKey(context, err))
			{
				err = formules->ExecuteOnOneKey<Type>(this, *(iter.GetKey()), iter.GetRecNum(), stop, context);
				if (stop || (err != VE_OK))
					break;
			}
		}
		else 
		{ 
			GetHeaderScalar()->CalculateColumnFormulas(curstack, formules, context, filtre, InProgress);
		}

		Close();
	}

	return err;
}


template <class Type, sLONG MaxCles>
VError IndexInfoScalar<Type,MaxCles>::CalculateMin(BaseTaskInfo* context, Bittab* filtre, VDB4DProgressIndicator* InProgress, VValueSingle* &result)
{
	VError err = VE_OK;
	sLONG curpage = 0;
	Boolean stop = false;

	if (fIsRemote)
		err = ThrowError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
	else
	{
		Open(index_read);
		OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
		{
			XBOX::VString session_title;
			gs(1005,27,session_title);	// Indexed Calculate Formulas: %curValue of %maxValue Pages
			IndexIterator<Type,MaxCles> iter(this, filtre, context, InProgress, session_title, curstack);

			if (iter.NextKey(context, err))
			{
				result = CreateVValueWithType<Type>(*(iter.GetKey()), fDataKind);
				if (result == nil)
					err = ThrowError(memfull, noaction);
			}
		}
		Close();
	}

	return err;
}


template <class Type, sLONG MaxCles>
VError IndexInfoScalar<Type,MaxCles>::CalculateMax(BaseTaskInfo* context, Bittab* filtre, VDB4DProgressIndicator* InProgress, VValueSingle* &result)
{
	VError err = VE_OK;
	sLONG curpage = 0;
	Boolean stop = false;

	if (fIsRemote)
		err = ThrowError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
	else
	{
		Open(index_read);
		OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
		{
			XBOX::VString session_title;
			gs(1005,27,session_title);	// Indexed Calculate Formulas: %curValue of %maxValue Pages
			IndexIterator<Type,MaxCles> iter(this, filtre, context, InProgress, session_title, curstack);

			if (iter.PreviousKey(context, err))
			{
				result = CreateVValueWithType<Type>(*(iter.GetKey()), fDataKind);
				if (result == nil)
					err = ThrowError(memfull, noaction);
			}
		}
		Close();
	}

	return err;
}



template <class Type, sLONG MaxCles>
Selection* IndexInfoScalar<Type,MaxCles>::ScanIndex(sLONG inMaxRecords, Boolean KeepSorted, Boolean Ascent, BaseTaskInfo* context, VError& err, Selection* filtre, VDB4DProgressIndicator* InProgress)
{
	Selection* result = nil;
	err = VE_OK;
	if (fIsRemote)
		err = ThrowError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
	else
	{
		Table* target = GetTargetTable();
		sLONG nbrecsInTable = target->GetDF()->GetNbRecords(context, false);
		Open(index_read);
		OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
		if (Invalid)
			err = ThrowError(VE_DB4D_INVALIDINDEX, DBaction_ScanningIndex);
		else
		{
			if (inMaxRecords > nbrecsInTable)
				inMaxRecords = nbrecsInTable;
			if (KeepSorted)
			{
				if (inMaxRecords > kNbElemInSel)
				{
					result = new LongSel(target->GetDF());
					((LongSel*)result)->PutInCache();
				}
				else
				{
					result = new PetiteSel(target->GetDF());
				}
			}
			else
			{
				if (inMaxRecords > 16)
				{
					result = new BitSel(target->GetDF());
				}
				else
				{
					result = new PetiteSel(target->GetDF());
				}
			}

			if (result == nil)
				err = ThrowError(memfull, DBaction_BuildingSelection);
			else
			{
				Bittab* bfiltre = nil;
				if (result->FixFic(inMaxRecords))
				{
					if (filtre != nil)
					{
						bfiltre = filtre->GenereBittab(context, err);
					}
				}
				else
				{
					err = ThrowError(memfull, DBaction_ScanningIndex);
				}

				if (err == VE_OK)
				{
					sLONG nbtrouves = 0;


					Transaction* curtrans = GetCurrentTransaction(context);
					if (curtrans != nil && curtrans->ConcernIndex(this))
					{
						VString session_title;
						gs(1005,28,session_title);
						IndexIterator<Type,MaxCles> iter(this, bfiltre, context, InProgress, session_title, curstack);

						if (Ascent)
						{
							while ((nbtrouves < inMaxRecords) && iter.NextKey(context, err, &nbtrouves))
							{
								err = result->PutFic(nbtrouves, iter.GetRecNum());
								if (err != VE_OK)
									break;
								nbtrouves++;
							}
						}
						else
						{
							while ((nbtrouves < inMaxRecords) && iter.PreviousKey(context, err))
							{
								err = result->PutFic(nbtrouves, iter.GetRecNum());
								if (err != VE_OK)
									break;
								nbtrouves++;
							}
						}
					}
					else
						err = header->ScanIndex(curstack, result, inMaxRecords, KeepSorted, Ascent, context, bfiltre,nbtrouves, InProgress);

					if (nbtrouves < inMaxRecords)
					{
						if (result->FixFic(nbtrouves) == false)
							err = ThrowError(memfull, DBaction_ScanningIndex);
					}
				}

				ReleaseRefCountable(&bfiltre);

				if (err != VE_OK)
				{
					result->Release();
					result = nil;
				}
			}

		}

		Close();
	}

	if (err != VE_OK)
	{
		err = ThrowError(VE_DB4D_CANNOTBUILDSELECTION, DBaction_ScanningIndex);
	}

	return result;
}



template <class Type, sLONG MaxCles>
VError IndexInfoScalar<Type,MaxCles>::JoinWithOtherIndex(IndexInfo* other, Bittab* filtre1, Bittab* filtre2,ComplexSelection* result, BaseTaskInfo* context, 
														 VCompareOptions& inOptions, VProgressIndicator* inProgress, bool leftjoin, bool rightjoin)
{
	VError err = VE_OK;

	if (fIsRemote)
		err = ThrowError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
	else
	{
		Open(index_read);
		other->Open(index_read);
		OccupableStack* curstack = gOccPool.GetStackForCurrentThread();

		IndexInfoScalar<Type,MaxCles>* xother = (IndexInfoScalar<Type,MaxCles>*)other;

		IndexIterator<Type,MaxCles> iter1(this, filtre1, context, nil, L"", curstack);
		IndexIterator<Type,MaxCles> iter2(xother, filtre2, context, nil, L"", curstack);

		Boolean cont1 = iter1.NextKey(context, err);
		Boolean cont2 = false;
		if (err == VE_OK)
		{
			cont2 = iter2.NextKey(context, err);
		}

		ComplexSelRow selrow;
		selrow.resize(2, -1);
		while (cont1 && cont2 && err == VE_OK)
		{
			VTask::Yield();
			if (*(iter1.GetKey()) == *(iter2.GetKey()))
			{
				vector<RecIDType> recordsNumsLeft;
				Type val = *(iter1.GetKey());
				RecIDType recnum1 = iter1.GetRecNum();
				selrow[0] = recnum1;

				vector<RecIDType> recordsNums;
				try
				{
					RecIDType recnum2 = iter2.GetRecNum();
					recordsNums.push_back(recnum2);
					selrow[1] = recnum2;
					err = result->AddRow(selrow);
					Boolean matching = true;
					while (matching && err == VE_OK)
					{
						VTask::Yield();
						matching = false;
						cont2 = iter2.NextKey(context,err);
						if (cont2)
						{
							if (*(iter2.GetKey()) == val)
							{
								recnum2 = iter2.GetRecNum();
								matching = true;
								recordsNums.push_back(recnum2);
								selrow[1] = recnum2;
								err = result->AddRow(selrow);
							}
						}
					}

					matching = true;
					while (matching && err == VE_OK)
					{
						VTask::Yield();
						matching = false;
						cont1 = iter1.NextKey(context,err);
						if (cont1)
						{
							if (*(iter1.GetKey()) == val)
							{
								recnum1 = iter1.GetRecNum();
								matching = true;
								selrow[0] = recnum1;
								for (vector<RecIDType>::iterator cur = recordsNums.begin(), end = recordsNums.end(); cur != end && err == VE_OK; cur++)
								{
									selrow[1] = *cur;
									err = result->AddRow(selrow);
								}
							}
						}
					}
				}
				catch (...)
				{
					err = ThrowBaseError(memfull, noaction);
				}

			}
			else
			{
				if (*(iter1.GetKey()) < *(iter2.GetKey()))
				{
					if (leftjoin)
					{
						if (filtre2 == nil)
						{
							selrow[0] = iter1.GetRecNum();
							selrow[1] = -2;
							err = result->AddRow(selrow);
						}
					}
					cont1 = iter1.NextKey(context, err);
				}
				else
				{
					if (rightjoin)
					{
						if (filtre1 == nil)
						{
							selrow[0] = -2;
							selrow[1] = iter2.GetRecNum();
							err = result->AddRow(selrow);
						}
					}
					cont2 = iter2.NextKey(context, err);
				}

			}
		}
		if (err == VE_OK)
		{
			if (leftjoin && filtre2 == nil)
			{
				while (cont1)
				{
					selrow[0] = iter1.GetRecNum();
					selrow[1] = -2;
					err = result->AddRow(selrow);
					
					cont1 = iter1.NextKey(context,err);
				}
			}
			if (rightjoin && filtre1 == nil)
			{
				while (cont2)
				{
					selrow[0] = -2;
					selrow[1] = iter2.GetRecNum();
					err = result->AddRow(selrow);

					cont2 = iter2.NextKey(context,err);
				}
			}
		}
		Close();
		other->Close();
	}

	return err;
}


template <class Type, sLONG MaxCles>
VError IndexInfoScalar<Type,MaxCles>::JoinWithOtherIndexNotNull(IndexInfo* other, Bittab* filtre1, Bittab* filtre2, Bittab* result, BaseTaskInfo* context, VCompareOptions& inOptions, VProgressIndicator* inProgress)
{
	VError err = VE_OK;

	if (fIsRemote)
		err = ThrowError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
	else
	{
		Open(index_read);
		other->Open(index_read);
		OccupableStack* curstack = gOccPool.GetStackForCurrentThread();

		IndexInfoScalar<Type,MaxCles>* xother = (IndexInfoScalar<Type,MaxCles>*)other;

		IndexIterator<Type,MaxCles> iter1(this, filtre1, context, nil, L"", curstack);
		IndexIterator<Type,MaxCles> iter2(xother, filtre2, context, nil, L"", curstack);

		Boolean cont1 = iter1.NextKey(context, err);
		Boolean cont2 = false;
		if (err == VE_OK)
		{
			cont2 = iter2.NextKey(context, err);
		}

		while (cont1 && cont2 && err == VE_OK)
		{
			if (*(iter1.GetKey()) == *(iter2.GetKey()))
			{
				cont1 = iter1.NextKey(context, err);
				result->Set(iter1.GetRecNum());
			}
			else
			{
				if (*(iter1.GetKey()) < *(iter2.GetKey()))
				{
					cont1 = iter1.NextKey(context, err);
				}
				else
				{
					cont2 = iter2.NextKey(context, err);
				}

			}
		}
		Close();
		other->Close();
	}

	return err;
}




				// -------------------------------------------------------------------------- //
				
				


template <class Type>
void TypeSortElemArray<Type>::GenerePageIndexScalar(OccupableStack* curstack, BtreePage<Type, kNbKeysForScalar> *page, VError &err, sLONG &curelem, sLONG maxlevel,
													 sLONG level, IndexInfo* ind, 
													 sLONG MaxElemsInPage, sLONG RequiredElemsInPage, BaseTaskInfo* context, 
													 VDB4DProgressIndicator* InProgress, sLONG typ, sLONG TypeSize, sLONG maxElemsToCompute, Boolean CheckRemain, Boolean islast)
{
	err = VE_OK;
	sLONG subrequired, submaxtocompute = -1;
	Boolean subcheckremain = false, issublast = false;
	BtreePage<Type, kNbKeysForScalar> *sousBT = nil;
	sLONG nbkey = 0, nbkeytobuild = RequiredElemsInPage;
	if (level == maxlevel)
		nbkeytobuild = MaxElemsInPage;

	if (InProgress != nil)
	{
		if (! InProgress->Progress(curelem))
			err = ind->ThrowError(VE_DB4D_ACTIONCANCELEDBYUSER, DBaction_BuildingIndex);
	}

	if (err == VE_OK)
	{
		{
			sLONG nbkeys_in_last = nbkeytobuild, nbkeys_in_antelast = nbkeytobuild;
			sLONG pos_of_last = -1, pos_of_antelast = -1;
			sLONG maxtocompute_in_last = -1, maxtocompute_in_antelast = -1;

			if (level > 1)
			{
				sLONG maxforthislevel = ind->ComputeMaxKeyToFit(level, MaxElemsInPage, MaxElemsInPage, false);
				if (count - curelem < maxforthislevel)
				{
					CheckRemain = true;
					islast = true;
				}
			}

			if (level > 1 && CheckRemain)
			{
				//if (level != maxlevel)
				{
					sLONG maxkeys_level_moins_un = ind->ComputeMaxKeyToFit(level-1, MaxElemsInPage, MaxElemsInPage, false);
					sLONG maxkeys_level_moins_deux = ind->ComputeMaxKeyToFit(level - 2, MaxElemsInPage, MaxElemsInPage, false);
					if (islast)
					{
						maxElemsToCompute = count - curelem;
					}
					sLONG nbkeys = maxElemsToCompute / maxkeys_level_moins_un;
					sLONG maxelemsmoins = maxElemsToCompute-nbkeys;
					nbkeys = maxelemsmoins / maxkeys_level_moins_un;
					sLONG remain = maxElemsToCompute - (maxkeys_level_moins_un * nbkeys + nbkeys);
					assert(remain >= 0);
					//sLONG nbkey_for_remain = (remain + maxkeys_level_moins_deux - 1) / maxkeys_level_moins_deux;
					sLONG nbkey_for_remain = remain / maxkeys_level_moins_deux;
					if (level > 2)
						nbkey_for_remain = (remain - nbkey_for_remain) / maxkeys_level_moins_deux;
					
					if (nbkey_for_remain < (MaxElemsInPage / 2))
					{
						nbkeys_in_last = (MaxElemsInPage + nbkey_for_remain) / 2;
						nbkeys_in_antelast = MaxElemsInPage + nbkey_for_remain - nbkeys_in_last;
						pos_of_last = nbkeys;
						pos_of_antelast = pos_of_last - 1;
						sLONG remain2 = remain + maxkeys_level_moins_un;
						maxtocompute_in_antelast = nbkeys_in_antelast * maxkeys_level_moins_deux;
						if (maxkeys_level_moins_deux > 1)
						{
							maxtocompute_in_antelast = maxtocompute_in_antelast 
															+ nbkeys_in_antelast /* pour le nb de cle dans la page mere */ 
															+ maxkeys_level_moins_deux /* pour le nombre de cles dans les pages filles de l'element 0*/;
						}
						maxtocompute_in_last = remain2 - maxtocompute_in_antelast;
					}
					nbkeytobuild = (maxElemsToCompute + maxkeys_level_moins_un - 1)/maxkeys_level_moins_un;
				}
			}

			// on va d'abord calculer le nombre de cles a ce niveau
			if (level > 1)
			{
				if (pos_of_antelast == 0)
				{
					subrequired = nbkeys_in_antelast;
					subcheckremain = false;
					submaxtocompute = maxtocompute_in_antelast;
				}
				else
				{
					subrequired = MaxElemsInPage;
					subcheckremain = false;
					submaxtocompute = -1;
				}

				sousBT = BtreePage<Type, kNbKeysForScalar>::AllocatePage(curstack, ind, context, err);
				if (sousBT == nil)
					err = ind->ThrowError(memfull, noaction);
				else
				{
					page->SetSousPage(0, sousBT);
					GenerePageIndexScalar(curstack, sousBT, err, curelem, maxlevel, level-1, ind, MaxElemsInPage, subrequired, context, InProgress, typ, TypeSize, submaxtocompute, subcheckremain, false);
					page->SetSousPage(0, sousBT);
					if (sousBT != nil)
					{
						assert(sousBT->GetNKeys() >= (MaxElemsInPage / 2));
						sousBT->Free(curstack, true);
					}
				}

			}

			while (curelem < count && nbkey < nbkeytobuild && err == VE_OK)
			{
				sLONG numrec;
				Type TempVal;

				if (filedata != nil)
				{
					err = filedata->GetLong(numrec);
					if (err == VE_OK)
					{
						sLONG len = 0;
						err = filedata->GetLong(len);
						if (err == VE_OK)
						{
							err = filedata->GetData(&TempVal, len); 
						}
					}
					if (err != VE_OK)
						break;
				}
				else
				{
					TypeSortElem<Type>* elem = GetDataElem(curelem, TypeSize);
					TempVal = elem->value;
					numrec = elem->recnum;
				}

				page->AddKey(TempVal, numrec, nil);
				
				curelem++;
				
				if (level > 1)
				{
					if (pos_of_antelast == nbkey + 1)
					{
						subrequired = nbkeys_in_antelast;
						submaxtocompute = maxtocompute_in_antelast;
						subcheckremain = false;
					}
					else
					{
						if (pos_of_last == nbkey + 1)
						{
							issublast = true;
							subrequired = nbkeys_in_last;
							submaxtocompute = maxtocompute_in_last;
							subcheckremain = true;
					}
						else
						{
							subrequired = MaxElemsInPage;
							subcheckremain = false;
							submaxtocompute = -1;
						}
					}
					sousBT = BtreePage<Type, kNbKeysForScalar>::AllocatePage(curstack, ind, context, err);
					if (sousBT == nil)
					{
						err = ind->ThrowError(memfull, noaction);
					}
					else
					{
						page->SetSousPage(page->GetNKeys(), sousBT);
						GenerePageIndexScalar(curstack, sousBT, err, curelem, maxlevel, level-1, ind, MaxElemsInPage, subrequired, context, InProgress, typ, TypeSize, submaxtocompute, subcheckremain, issublast);
						page->SetSousPage(page->GetNKeys(), sousBT);
						assert(sousBT->GetNKeys() >= (MaxElemsInPage / 2));
						sousBT->Free(curstack, true);
					}
				}


				nbkey++;


			}
		}

		if (err == VE_OK)
		{
			//ind->GetDB()->DelayForFlush();
			err = page->savepage(curstack, context);
		}

	}
		
}


template <class Type>
VError TypeSortElemArray<Type>::GenereIndexScalar(IndexInfo* ind, sLONG MaxElemsInPage, sLONG RequiredElemsInPage, BaseTaskInfo* context, 
											VDB4DProgressIndicator* InProgress, sLONG typ, sLONG TypeSize)
{
	VError err = VE_OK;
	BtreePage<Type, kNbKeysForScalar>* first;
		
	if (count > 0)
	{
		sLONG curelem = 0;

		if (InProgress != nil)
		{
			XBOX::VString session_title;
			gs(1005,15,session_title);	// Generating Index Pages
			InProgress->BeginSession(count,session_title,false);
		}

		ind->Open(index_write);
		OccupableStack* curstack = gOccPool.GetStackForCurrentThread();

		sLONG nblevel = ind->CalculateGenerationsLevels(count, MaxElemsInPage, RequiredElemsInPage);
		first = BtreePage<Type, kNbKeysForScalar>::AllocatePage(curstack, ind, context, err);

		if (first == nil)
		{
			err = ind->ThrowError(memfull, noaction);
			if (filedata != nil)
				filedata->CloseReading();
		}
		else
		{
			((IndexHeaderBTreeScalar<Type, kNbKeysForScalar>*)(ind->GetHeader()))->SetFirstPage(first, context);
			GenerePageIndexScalar(curstack, first, err, curelem, nblevel, nblevel, ind, MaxElemsInPage, RequiredElemsInPage, context, InProgress, typ, TypeSize, count, true, true);
			((IndexHeaderBTreeScalar<Type, kNbKeysForScalar>*)(ind->GetHeader()))->SetFirstPage(first, context);
			if (err == VE_OK && fNulls != nil && fNulls->Compte() != 0)
				err = ind->GetHeader()->AddToNulls(curstack, fNulls, context);
			ind->GetHeader()->setmodif(true, ind->GetDB(), context);
			if (filedata != nil)
				filedata->CloseReading();
			if (err == VE_OK)
			{
				first->Free(curstack, true);
			}
			else
			{
				// l'index est mal genere, il faut le detruire
				first->LibereEspaceMem(curstack);
				((IndexHeaderBTreeScalar<Type, kNbKeysForScalar>*)(ind->GetHeader()))->SetFirstPage(nil, context);
				first->Free(curstack, true);
				first->Release();
				((IndexHeaderBTreeScalar<Type, kNbKeysForScalar>*)(ind->GetHeader()))->LibereEspaceDisk(curstack, InProgress);
			}
		}

		ind->Close();

		if (InProgress != nil)
		{
			InProgress->EndSession();
		}

	}
	else
	{
		OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
		if (fNulls != nil && fNulls->Compte() != 0)
			err = ind->GetHeader()->AddToNulls(curstack, fNulls, context);
		ind->GetHeader()->setmodif(true, ind->GetDB(), context);
	}

	if (err != VE_OK)
		err = ind->ThrowError(VE_DB4D_CANNOT_BUILD_QUICK_INDEX, DBaction_BuildingIndex);

	return err;
}




template <class Type>
void TypeSortElemArray<Type>::GenerePageClusterIndexScalar(OccupableStack* curstack, BtreePage<Type, kNbKeysForScalar> *page, VFileStream* keysdata, VError &err, sLONG &curelem, sLONG &curcluster, sLONG maxlevel,
												 sLONG level, IndexInfo* ind, 
												 sLONG MaxElemsInPage, sLONG RequiredElemsInPage, BaseTaskInfo* context, 
												 VDB4DProgressIndicator* InProgress, sLONG typ, sLONG TypeSize, sLONG maxElemsToCompute, Boolean CheckRemain, Boolean islast)
{
	err = VE_OK;
	sLONG subrequired, submaxtocompute = -1;
	Boolean subcheckremain = false, issublast = false;
	BtreePage<Type, kNbKeysForScalar> *sousBT = nil;
	sLONG nbkey = 0, nbkeytobuild = RequiredElemsInPage;
	if (level == maxlevel)
		nbkeytobuild = MaxElemsInPage;

	if (InProgress != nil)
	{
		if (! InProgress->Progress(curelem))
			err = ind->ThrowError(VE_DB4D_ACTIONCANCELEDBYUSER, DBaction_BuildingIndex);
	}
	if (err == VE_OK)
	{
		{
			sLONG nbkeys_in_last = nbkeytobuild, nbkeys_in_antelast = nbkeytobuild;
			sLONG pos_of_last = -1, pos_of_antelast = -1;
			sLONG maxtocompute_in_last = -1, maxtocompute_in_antelast = -1;

			if (level > 1)
			{
				sLONG maxforthislevel = ind->ComputeMaxKeyToFit(level, MaxElemsInPage, MaxElemsInPage, false);
				if (count - curelem < maxforthislevel)
				{
					CheckRemain = true;
					islast = true;
				}
			}

			if (level > 1 && CheckRemain)
			{
				//if (level != maxlevel)
				{
					sLONG maxkeys_level_moins_un = ind->ComputeMaxKeyToFit(level-1, MaxElemsInPage, MaxElemsInPage, false);
					sLONG maxkeys_level_moins_deux = ind->ComputeMaxKeyToFit(level - 2, MaxElemsInPage, MaxElemsInPage, false);
					if (islast)
					{
						maxElemsToCompute = count - curelem;
					}
					sLONG nbkeys = maxElemsToCompute / maxkeys_level_moins_un;
					sLONG maxelemsmoins = maxElemsToCompute-nbkeys;
					nbkeys = maxelemsmoins / maxkeys_level_moins_un;
					sLONG remain = maxElemsToCompute - (maxkeys_level_moins_un * nbkeys + nbkeys);
					assert(remain >= 0);
					//sLONG nbkey_for_remain = (remain + maxkeys_level_moins_deux - 1) / maxkeys_level_moins_deux;
					sLONG nbkey_for_remain = remain / maxkeys_level_moins_deux;
					if (level > 2)
						nbkey_for_remain = (remain - nbkey_for_remain) / maxkeys_level_moins_deux;

					if (nbkey_for_remain < (MaxElemsInPage / 2))
					{
						nbkeys_in_last = (MaxElemsInPage + nbkey_for_remain) / 2;
						nbkeys_in_antelast = MaxElemsInPage + nbkey_for_remain - nbkeys_in_last;
						pos_of_last = nbkeys;
						pos_of_antelast = pos_of_last - 1;
						sLONG remain2 = remain + maxkeys_level_moins_un;
						maxtocompute_in_antelast = nbkeys_in_antelast * maxkeys_level_moins_deux;
						if (maxkeys_level_moins_deux > 1)
						{
							maxtocompute_in_antelast = maxtocompute_in_antelast 
								+ nbkeys_in_antelast /* pour le nb de cle dans la page mere */ 
								+ maxkeys_level_moins_deux /* pour le nombre de cles dans les pages filles de l'element 0*/;
						}
						maxtocompute_in_last = remain2 - maxtocompute_in_antelast;
					}
					nbkeytobuild = (maxElemsToCompute + maxkeys_level_moins_un - 1)/maxkeys_level_moins_un;
				}
			}

			// on va d'abord calculer le nombre de cles a ce niveau
			if (level > 1)
			{
				if (pos_of_antelast == 0)
				{
					subrequired = nbkeys_in_antelast;
					subcheckremain = false;
					submaxtocompute = maxtocompute_in_antelast;
				}
				else
				{
					subrequired = MaxElemsInPage;
					subcheckremain = false;
					submaxtocompute = -1;
				}

				sousBT = BtreePage<Type, kNbKeysForScalar>::AllocatePage(curstack, ind, context, err);
				if (sousBT == nil)
					err = ind->ThrowError(memfull, noaction);
				else
				{
					page->SetSousPage(0, sousBT);
					GenerePageClusterIndexScalar(curstack, sousBT, keysdata, err, curelem, curcluster, maxlevel, level-1, ind, MaxElemsInPage, subrequired, context, InProgress, typ, TypeSize, submaxtocompute, subcheckremain, false);
					page->SetSousPage(0, sousBT);
					assert(sousBT->GetNKeys() >= (MaxElemsInPage / 2));
					sousBT->Free(curstack, true);
				}
			}

			while (curelem < count && nbkey < nbkeytobuild && err == VE_OK)
			{
				sLONG numrec = -1;
				Type TempVal;

				err = keysdata->GetData(&TempVal, sizeof(TempVal));
				sLONG qt = 0;
				if (err == VE_OK)
					err = keysdata->GetLong(qt);
				if (err == VE_OK && qt == 1)
					err = keysdata->GetLong(numrec);
				else
				{
					numrec = -(curcluster+2);
					curcluster++;
				}
				if (err == VE_OK)
				{
					page->AddKey(TempVal, numrec, nil);
				}
				if (err != VE_OK)
					break;

				curelem++;

				if (level > 1)
				{
					if (pos_of_antelast == nbkey + 1)
					{
						subrequired = nbkeys_in_antelast;
						submaxtocompute = maxtocompute_in_antelast;
						subcheckremain = false;
					}
					else
					{
						if (pos_of_last == nbkey + 1)
						{
							issublast = true;
							subrequired = nbkeys_in_last;
							submaxtocompute = maxtocompute_in_last;
							subcheckremain = true;
						}
						else
						{
							subrequired = MaxElemsInPage;
							subcheckremain = false;
							submaxtocompute = -1;
						}
					}
					sousBT = BtreePage<Type, kNbKeysForScalar>::AllocatePage(curstack, ind, context, err);
					if (sousBT == nil)
						err = ind->ThrowError(memfull, noaction);
					else
					{
						page->SetSousPage(page->GetNKeys(), sousBT);
						GenerePageClusterIndexScalar(curstack, sousBT, keysdata, err, curelem, curcluster, maxlevel, level-1, ind, MaxElemsInPage, subrequired, context, InProgress, typ, TypeSize, submaxtocompute, subcheckremain, issublast);
						page->SetSousPage(page->GetNKeys(), sousBT);
						assert(sousBT->GetNKeys() >= (MaxElemsInPage / 2));
						sousBT->Free(curstack, true);
					}
				}

				nbkey++;

			}
		}

		if (err == VE_OK)
		{
			//ind->GetDB()->DelayForFlush();
			err = page->savepage(curstack, context);
		}

	}
}



template <class Type>
VError TypeSortElemArray<Type>::GenereClusterIndexScalar(IndexInfo* ind, sLONG MaxElemsInPage, sLONG RequiredElemsInPage, BaseTaskInfo* context, 
														VDB4DProgressIndicator* InProgress, sLONG typ, sLONG TypeSize, xMultiFieldDataOffsets& criterias)
{
	VError err = VE_OK;
	BtreePage<Type, kNbKeysForScalar>* first;
#if debugIndexPage
	ind->Cannot_Debug_AddPage();
#endif

	if (count > 0)
	{
		sLONG curelem = 0;
		sLONG nbrealkeys = 0;
		sLONG debugcountsel = 0;

		VFileStream* keysdata = CreateTempFile(ind->GetDB(), nil, 0);
		VFileStream* selsdata = CreateTempFile(ind->GetDB(), nil, 0);
		if (keysdata != nil && selsdata != nil)
		{
			keysdata->OpenWriting();
			keysdata->SetBufferSize(512000);
			selsdata->OpenWriting();
			selsdata->SetBufferSize(512000);
			// premiere partie : regouper les numero de fiche des cles egales 

			if (err == VE_OK)
			{
				Boolean dejadata = false;
				Type CurData;
				Bittab* cursel = nil;
				sLONG qtsel = 0;
				sLONG curficseule = -1;

				if (InProgress != nil)
				{
					XBOX::VString session_title;
					gs(1005,16,session_title);	// Generating Clusters
					InProgress->BeginSession(count,session_title,false);
				}

				sLONG maxrecord = ind->GetTargetTable()->GetDF()->GetMaxRecords(nil);
				sLONG maxrecordforpetitesel = maxrecord/32;
				for (sLONG i = 0; i<=count && err == VE_OK; i++)
				{
					if (InProgress != nil)
					{
						if (! InProgress->Progress(i))
						{
							err = VE_DB4D_ACTIONCANCELEDBYUSER;
							break;
						}
					}

					sLONG numrec = -1;
					Type CurRead;
					Boolean AtTheEnd = false;

					if (i != count)
					{
						if (filedata != nil)
						{
							err = filedata->GetLong(numrec);
							if (err == VE_OK)
							{
								sLONG len = 0;
								err = filedata->GetLong(len);
								if (err == VE_OK)
								{
									err = filedata->GetData(&CurRead, len);
								}
							}
						}
						else
						{
							TypeSortElem<Type>* elem = GetDataElem(i, TypeSize);
							numrec = elem->recnum;
							CurRead = elem->value;
						}
					}
					else
					{
						AtTheEnd = true;
					}

					if (err == VE_OK)
					{
						if (!dejadata || AtTheEnd || CurRead != CurData)
						{
							if (!AtTheEnd )
								nbrealkeys++;
/*
							DebugMsg(L"\n");
							DebugMsg(L"-------------------------------------\n");
*/
							if (dejadata)
							{
								err = keysdata->PutData(&CurData, sizeof(CurData));
								if (err == VE_OK)
								{
									sLONG qt = qtsel;
									assert(qt > 0);
									err = keysdata->PutLong(qt);
									if (err == VE_OK)
									{
										if (qt == 1)
										{
											err = keysdata->PutLong(curficseule);
										}
										else
										{
											cursel->Epure();
											assert (qt == cursel->Compte());
											debugcountsel++;
											if (qt <= maxrecordforpetitesel)
											{
												err = selsdata->PutLong(sel_petitesel);
												if (err == VE_OK)
													err = selsdata->PutLong(qt);
												if (err == VE_OK)
												{
													sLONG debugcount = 0;
													sLONG x = cursel->FindNextBit(0);
													while (x != -1 && err == VE_OK)
													{
														debugcount++;
														err = selsdata->PutLong(x);
														x = cursel->FindNextBit(x+1);
													}
													assert(debugcount == qt);
												}
											}
											else
											{
												err = selsdata->PutLong(sel_bitsel);
												if (err == VE_OK)
													err = cursel->PutInto(*selsdata);
											}
										}
									}
								}

								ReleaseRefCountable(&cursel);
								qtsel = 0;

							}

							dejadata = true;
							if (!AtTheEnd)
							{
								CurData = CurRead;
								qtsel = 1;
								curficseule = numrec;
							}
						}
						else
						{
							if (cursel == nil)
								cursel = new Bittab;
							if (cursel == nil)
								err = ind->ThrowError(memfull, DBaction_BuildingIndex);
							else
							{
								err = cursel->aggrandit(maxrecord);
								if (err == VE_OK)
									err = cursel->Set(numrec, true);
								if (err == VE_OK)
									err = cursel->Set(curficseule, true);
								qtsel++;
							}
						}
					}
				}

				if (InProgress != nil)
				{
					InProgress->EndSession();
				}

				ReleaseRefCountable(&cursel);

			}

			keysdata->CloseWriting();
			selsdata->CloseWriting();

			if (err == VE_OK)
			{
				count = nbrealkeys;
				sLONG curcluster = 0;

				keysdata->OpenReading();
				selsdata->OpenReading();

				if (InProgress != nil)
				{
					XBOX::VString session_title;
					gs(1005,15,session_title);	// Generating Index Pages
					InProgress->BeginSession(nbrealkeys,session_title,false);
				}

				ind->Open(index_write);

				OccupableStack* curstack = gOccPool.GetStackForCurrentThread();

				sLONG nblevel = ind->CalculateGenerationsLevels(nbrealkeys, MaxElemsInPage, RequiredElemsInPage);
				first = BtreePage<Type, kNbKeysForScalar>::AllocatePage(curstack, ind, context, err);

				if (first == nil)
				{
					err = ind->ThrowError(memfull, noaction);
				}
				else
				{
					((IndexHeaderBTreeScalar<Type, kNbKeysForScalar>*)(ind->GetHeader()))->SetFirstPage(first, context);
					GenerePageClusterIndexScalar(curstack, first, keysdata, err, curelem, curcluster, nblevel, nblevel, ind, MaxElemsInPage, RequiredElemsInPage, context, InProgress, typ, TypeSize, count, true, true);
					((IndexHeaderBTreeScalar<Type, kNbKeysForScalar>*)(ind->GetHeader()))->SetFirstPage(first, context);
					if (err == VE_OK && fNulls != nil && fNulls->Compte() != 0)
						err = ind->GetHeader()->AddToNulls(curstack, fNulls, context);
					ind->GetHeader()->setmodif(true, ind->GetDB(), context);
					if (err != VE_OK)
					{
						// l'index est mal genere, il faut le detruire
						first->LibereEspaceMem(curstack);
						((IndexHeaderBTreeScalar<Type, kNbKeysForScalar>*)(ind->GetHeader()))->SetFirstPage(nil, context);
						first->Free(curstack, true);
						first->Release();
						first = nil;
						((IndexHeaderBTreeScalar<Type, kNbKeysForScalar>*)(ind->GetHeader()))->LibereEspaceDisk(curstack, InProgress);
					}
				}

				if (InProgress != nil)
				{
					InProgress->EndSession();
				}

				if (first != nil)
				{
					first->Free(curstack, true);

					if (InProgress != nil)
					{
						XBOX::VString session_title;
						gs(1005,18,session_title); // Generating Index Clusters
						InProgress->BeginSession(curcluster,session_title,false);
					}
					for (sLONG i = 0; i < curcluster && err == VE_OK; i++)
					{
						sLONG typsel = 0;
						if (InProgress != nil)
						{
							if (! InProgress->Progress(i))
								err = VE_DB4D_ACTIONCANCELEDBYUSER;
						}
						if (err == VE_OK)
							err = selsdata->GetLong(typsel);
						if (err == VE_OK)
						{
							if (typsel == sel_petitesel)
							{
								sLONG qt = 0;
								err = selsdata->GetLong(qt);
								if (err == VE_OK)
								{
									PetiteSel* sel = new PetiteSel(ind->GetTargetTable()->GetDF());
									if (sel == nil)
										err = ind->ThrowError(memfull, DBaction_BuildingIndex);
									else
									{
										sel->Occupy(curstack, true);
										if (sel->FixFic(qt))
										{
											sLONG qt2 = qt;
											err = selsdata->GetLongs(sel->GetTabMem(), &qt2);
											assert(qt == qt2);
										}
										else
											err = ind->ThrowError(memfull, DBaction_BuildingIndex);

										if (err == VE_OK)
										{
											err = ind->GetClusterSel(curstack)->AddSel(curstack, sel, context);
										}

										sel->Free(curstack, true);
										sel->Release();
									}
								}
							}
							else
							{
								if (typsel == sel_bitsel)
								{
									BitSel* sel = new BitSel(ind->GetTargetTable()->GetDF());
									if (sel == nil)
										err = ind->ThrowError(memfull, DBaction_BuildingIndex);
									{
										sel->Occupy(curstack, true);
										Bittab* b = sel->GetBittab();
										err = b->GetFrom(*selsdata);
										if (err == VE_OK)
										{
											sel->Touch();
											sel->GetQTfic();
											err = ind->GetClusterSel(curstack)->AddSel(curstack, sel, context);
										}
										sel->Free(curstack, true);
										sel->Release();
									}
								}
								else
									assert(false); // wrong selection type
							}
						}
					}


					if (InProgress != nil)
					{
						InProgress->EndSession();
					}

					if (err != VE_OK)
					{
						((IndexHeaderBTreeScalar<Type, kNbKeysForScalar>*)(ind->GetHeader()))->LibereEspaceMem(curstack);
						first = nil;
						((IndexHeaderBTreeScalar<Type, kNbKeysForScalar>*)(ind->GetHeader()))->LibereEspaceDisk(curstack, InProgress);
					}

					ind->Close();
				}
				keysdata->CloseReading();
				selsdata->CloseReading();
			
			}

		}
		else
			err = ind->ThrowError(memfull, DBaction_BuildingIndex);

		if (keysdata != nil)
		{
			const VFile* ff = keysdata->GetVFile();
			if (ff != nil)
				ff->Delete();
			delete keysdata;
		}
		if (selsdata != nil)
		{
			const VFile* ff = selsdata->GetVFile();
			if (ff != nil)
				ff->Delete();
			delete selsdata;
		}

	}
	else
	{
		OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
		if (fNulls != nil && fNulls->Compte() != 0)
			err = ind->GetHeader()->AddToNulls(curstack, fNulls, context);
		ind->GetHeader()->setmodif(true, ind->GetDB(), context);
	}


	if (filedata != nil)
		filedata->CloseReading();

#if debugIndexPage
	ind->Can_Debug_AddPage();
#endif

	if (err != VE_OK)
		err = ind->ThrowError(VE_DB4D_CANNOT_BUILD_QUICK_INDEX, DBaction_BuildingIndex);

	return err;
}



				

											/* -----------------------------------------------  */




template <class Type, sLONG MaxCles>
IndexIterator<Type, MaxCles>::IndexIterator(IndexInfoScalar<Type, MaxCles>* ind, Bittab* filter, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, const VString& inMess, OccupableStack* curstack):fProgress(InProgress)
{
	fWasJustReset = true;
	fInd = ind;
	if (testAssert(ind != nil))
		fInd->Retain();

	fContext = context;
	if (fContext != nil)
		fContext->Retain();
	fSel = nil;
	fKey = nil;
	fNbLevel = 0;
	fRecNum = -1;
	fFilter = filter;
	fCleAddEnTrans = false;
	fCleDelEnTrans = false;
	fCurKeyInTrans = false;
	fCurStack = curstack;

	//fProgress = InProgress;
	fCurProgressPos = 0;
	fKeyInTrans = nil;

	//if (InProgress != nil)
	{
		sLONG nMax = -1;
		if (fInd->MatchType(DB4D_Index_OnKeyWords))
			nMax = -1;
		else
		{
			Table* tt = fInd->GetTargetTable();
			if (tt != nil)
			{
				DataTable* df = tt->GetDF();
				nMax = df->GetNbRecords(context, false);
			}

		}
		fProgress.BeginSession(nMax, inMess,true);
	}

}


template <class Type, sLONG MaxCles>
IndexIterator<Type, MaxCles>::~IndexIterator()
{
	//if (fProgress != nil)
	{
		fProgress.EndSession();
	}

	_DisposeSel();
	_DisposePages();
	if (fInd != nil)
		fInd->Release();
	if (fContext != nil)
		fContext->Release();
}


template <class Type, sLONG MaxCles>
void IndexIterator<Type, MaxCles>::_DisposePages()
{
	for (sLONG i = 0; i<fNbLevel; i++)
		fPagePath[i].fPage->Free(fCurStack, true);
	fNbLevel = 0;
}



template <class Type, sLONG MaxCles>
VError IndexIterator<Type, MaxCles>::_SetSel(Selection* sel)
{
	VError err = VE_OK;
	Boolean mustretain = true;
	_DisposeSel();
	if (fFilter != nil && sel != nil)
	{
		BitSel* bsel = new BitSel(sel->GetParentFile());
		if (bsel != nil)
		{
			Bittab* b = sel->GenereBittab(fContext, err);
			if (err == VE_OK)
				err = bsel->GetBittab()->Or(b, true);
			if (err == VE_OK)
				err = bsel->GetBittab()->And(fFilter, false);
			bsel->Touch();
			ReleaseRefCountable(&b);
			sel->Release();
			if (err != VE_OK)
			{
				bsel->Release();
				bsel = nil;
			}
			sel = bsel;
			mustretain = false;
		}
		else
			err = ThrowBaseError(memfull, DBaction_IndexKeyIterator);
	}
	fSel = sel;
	if (sel != nil && mustretain)
		sel->Retain();
	fSelIter.Reset(sel);

	return err;

}


template <class Type, sLONG MaxCles>
sLONG IndexIterator<Type, MaxCles>::GetRecNum() const 
{ 
	sLONG result = -1;
	if (fCurKeyInTrans)
	{
//		if (fKeyInTrans != nil)
			result = fKeyInTrans->qui;
	}
	else
		result = fRecNum; 
	return result;
}


template <class Type, sLONG MaxCles>
const Type* IndexIterator<Type, MaxCles>::GetKey() const
{
	if (fCurKeyInTrans)
	{
		return &(fKeyInTrans->cle);
	}
	else
	{
		assert(fKey != nil);
		return &(fKey->cle);
	}
}


template <class Type, sLONG MaxCles>
Boolean IndexIterator<Type, MaxCles>::FirstKey(BaseTaskInfo* context, VError& err, sLONG* CurElemToProgress)
{
	Boolean ok = false;
	err = VE_OK;
	_DisposeSel();
	_DisposePages();
	_DisposeKey();
	fCurKeyInTrans = false;
	if (fInd != nil)
	{
		Transaction* trans = GetCurrentTransaction(context);
		if (trans != nil)
		{
			assert(fInd->IsScalar());
			fSavedKeys = (typename IndexInfoScalar<Type, MaxCles>::mapKeys*)trans->GetSavedKeys(fInd, false, err);
			fDeletedKeys = (typename IndexInfoScalar<Type, MaxCles>::mapKeys*)trans->GetDeletedKeys(fInd, false, err);
			if (fSavedKeys != nil)
			{
				fCurSavedKey = fSavedKeys->begin();
				fEndSavedKey = fSavedKeys->end();
				fCleAddEnTrans = true;
				_RetainKey();
			}
			if (fDeletedKeys != nil)
			{
				fCurDeletedKey = fDeletedKeys->begin();
				fEndDeletedKey = fDeletedKeys->end();
				fCleDelEnTrans = true;
			}
		}

		IndexHeaderBTreeScalar<Type, MaxCles>* header = fInd->GetHeaderScalar();
		if (header != nil)
		{
			err = header->ChargeFirstPage(fCurStack, fContext, false);
			if (err == VE_OK)
			{
				BtreePage<Type, MaxCles>* page = header->_GetFirstPage();
				if (page != nil)
				{
					_PosToFirstLeft(page, context, err);
					if (err == VE_OK)
					{
						fWasJustReset = false;
						ok = NextKey(context, err, CurElemToProgress);
					}
				}
				else
				{
					// pas de cle dans l'index btree mais peut etre dans la transaction
					fWasJustReset = false;
					ok = NextKey(context, err, CurElemToProgress);
				}
			}
		}

	}
	return ok;
}


template <class Type, sLONG MaxCles>
void IndexIterator<Type, MaxCles>::_PosToFirstLeft(BtreePage<Type, MaxCles>* newpage, BaseTaskInfo* context, VError& err)
{
	fNbLevel++;
	fPagePath[fNbLevel-1].fPage = newpage;
	CleIndex<Type>* key = newpage->GetItemPtr(0);
	if (key->souspage != -1)
	{
		fPagePath[fNbLevel-1].fPos = -1;
		BtreePage<Type, MaxCles>* sousBT = newpage->GetSousPage(fCurStack, 0, err, context);
		if (err == VE_OK)
		{
			_PosToFirstLeft(sousBT, context, err);
		}
	}
	else
	{
		fPagePath[fNbLevel-1].fPos = -1;
		_DisposeSel();
		_DisposeKey();
	}
}


template <class Type, sLONG MaxCles>
Boolean IndexIterator<Type, MaxCles>::_MatchDelWithCurKey()
{
	Boolean match = false;

	if (fCleDelEnTrans && fKey != nil)
	{
		Boolean OneMoreTime;
		do
		{
			OneMoreTime = false;
			if (fCurDeletedKey != fEndDeletedKey)
			{
				sLONG res = CompareKeysStrictWithRecNum(*fKey, *fCurDeletedKey );

				if (res == CR_EQUAL)
				{
					match = true;
				}
				else
				{
					if (res == CR_BIGGER)
					{
						OneMoreTime = true;
						fCurDeletedKey++;
					}
				}
			}
			else
				fCleDelEnTrans = false;
		} while (OneMoreTime);
	}

	return match;
}


template <class Type, sLONG MaxCles>
void IndexIterator<Type, MaxCles>::_MatchAddWithCurKey()
{
	fCurKeyInTrans = false;
	if (fCleAddEnTrans)
	{
		Boolean OneMoreTime;
		do
		{
			OneMoreTime = false;
			if (fCurSavedKey != fEndSavedKey)
			{
				//const TransCleIndex<Type>* keyintrans = &(*fKeyInTrans);
				if (fFilter != nil && !fFilter->isOn(fKeyInTrans->qui))
				{
					OneMoreTime = true;
					fCurSavedKey++;
					_RetainKey();
				}
				else
				{
					sLONG res;
					if (fKey == nil)
					{
						res = CR_BIGGER;
					}
					else
					{
						res = CompareKeysStrictWithRecNum(*fKey, *fKeyInTrans );
					}

					if (res == CR_EQUAL || res == CR_SMALLER)
					{
						assert (res != CR_EQUAL);
						if (res == CR_EQUAL) // ne devrait pas arriver
						{
							fCurSavedKey++;
							_RetainKey();
						}
						fCurKeyInTrans = false;
					}
					else
					{
						fCurKeyInTrans = true;
					}
				}
			}
			else
				fCleAddEnTrans = false;
		} while (OneMoreTime);
	}
}


template <class Type, sLONG MaxCles>
VError IndexIterator<Type, MaxCles>::_IncCurPos(sLONG* CurElemToProgress)
{
	VError err = VE_OK;
	fCurProgressPos++;
	//if (fProgress != nil)
	{
		if (!fProgress.Progress(CurElemToProgress == nil ? fCurProgressPos : *CurElemToProgress))
			err = VE_DB4D_ACTIONCANCELEDBYUSER;
	}
	return err;
}


template <class Type, sLONG MaxCles>
Boolean IndexIterator<Type, MaxCles>::NextKey(BaseTaskInfo* context, VError& err, sLONG* CurElemToProgress)
{
	if (fWasJustReset)
		return FirstKey(context, err, CurElemToProgress);
	else
	{
		Boolean OneMoreTime;
		Boolean ok = false;
		Boolean fromSubLevel = false;
		do
		{
			OneMoreTime = false;
			err = VE_OK;
			Boolean nextpos = true;

			if (fCurKeyInTrans)
			{
				//err = _IncCurPos(CurElemToProgress);
				//if (err == VE_OK)
				{
					fCurSavedKey++;
					_RetainKey();
					_MatchAddWithCurKey();
					ok = fCurKeyInTrans || (fKey != nil);
				}
			}
			else
			{
				if (fSel != nil)
				{
					Boolean subloopcluster;
					do
					{
						/*
						err = _IncCurPos(CurElemToProgress);
						if (err != VE_OK)
						{
							nextpos = false;
							break;
						}
						*/
						subloopcluster = false;
						fRecNum = fSelIter.NextRecord();
						if (fRecNum != -1)
						{
							if (_MatchDelWithCurKey())
								subloopcluster = true;
							else
							{
								if (testAssert(fKey != nil))
									fKey->qui = fRecNum;
								_MatchAddWithCurKey();
								nextpos = false;
								ok = true;
							}
						}
					} while (subloopcluster);
				}

				if (nextpos)
				{
					_DisposeSel();
					//err = _IncCurPos(CurElemToProgress);
					//if (err == VE_OK)
					{
						BtreePage<Type, MaxCles>* curpage = nil;
						sLONG curpos = -1;
						if (fNbLevel > 0)
						{
							curpage = fPagePath[fNbLevel-1].fPage;
							curpos = fPagePath[fNbLevel-1].fPos;
						}

						if (curpage != nil)
						{
							if (curpos < (curpage->GetNKeys()-1))
							{
								curpos++;
								fPagePath[fNbLevel-1].fPos = curpos;
								CleIndex<Type>* key = curpage->GetItemPtr(curpos);
								if (key->souspage != -1 && !fromSubLevel)
								{
									fromSubLevel = false;
									fPagePath[fNbLevel-1].fPos = curpos-1;
									VTask::Yield();
									BtreePage<Type, MaxCles>* sousBT = curpage->GetSousPage(fCurStack, curpos, err, context);
									if (err == VE_OK)
									{
										err = _IncCurPos(CurElemToProgress);
										if (err == VE_OK)
										{
											_PosToFirstLeft(sousBT, context, err);
											OneMoreTime = true;
										}
									}
								}
								else
								{
									fromSubLevel = false;
									_DisposeKey();
									fKey = curpage->GetItemPtr(curpos);
									sLONG qui = fKey->qui;
									if (qui >= 0)
									{		
										if (fFilter == nil || fFilter->isOn(qui))
										{
											fRecNum = qui;
											if (_MatchDelWithCurKey())
												OneMoreTime = true;
											else
											{
												_MatchAddWithCurKey();
												ok = true;
											}
										}
										else
											OneMoreTime = true;

									}
									else
									{
										fAuxKeyForCluster = *fKey;
										fKey = &fAuxKeyForCluster;
										Selection* sel = fInd->GetClusterSel(fCurStack)->GetSel(fCurStack, qui, context, err);
										if (err == VE_OK)
											err = _SetSel(sel);
										if (err == VE_OK)
										{
											OneMoreTime = true;
										}
									}
								}

							}
							else
							{
								Boolean Remonte = true;
								if (curpos == (curpage->GetNKeys()-1))
								{
									fPagePath[fNbLevel-1].fPos = curpos+1;
									VTask::Yield();
									BtreePage<Type, MaxCles>* sousBT = curpage->GetSousPage(fCurStack, curpos+1, err, context);
									if (err == VE_OK)
									{
										if (sousBT != nil)
										{
											err = _IncCurPos(CurElemToProgress);
											if (err == VE_OK)
											{
												_PosToFirstLeft(sousBT, context, err);
												OneMoreTime = true;
												Remonte = false;
											}
										}
									}
									else
										Remonte = false;
								}

								if (Remonte)
								{
									curpage->Free(fCurStack, true);
									fNbLevel--;
									if (fNbLevel > 0)
									{
										fromSubLevel = true;
										OneMoreTime = true;
									}
								}
							}

						}
					}
				}
			}
		} while (OneMoreTime && err == VE_OK);

		if (err != VE_OK)
			ok = false;

		if (!ok)
		{
			_DisposeKey();
			fRecNum = -1;
			if (err == VE_OK)
			{
				if (fCleAddEnTrans)
				{
					if (fCurSavedKey != fEndSavedKey)
					{
						_RetainKey();
						_MatchAddWithCurKey();
						ok = fCurKeyInTrans || (fKey != nil);
					}
				}
			}
		}

		return ok;
	}


}


							/* -----------------------------------------------  */


template <class Type>
VError ColumnFormulas::ExecuteOnOneKey(IndexInfo* ind, Type data, sLONG numrec, Boolean& stop, BaseTaskInfo* context)
{
	VError err = VE_OK;

	FicheInMem* dejaChargee = nil;
	if (context != nil)
	{
		dejaChargee = context->RetainRecAlreadyIn(ind->GetTargetTable()->GetDF(), numrec);
	}
	sLONG i,nb = fActions.GetCount();
	for (i = 0; i<nb && err == VE_OK; i++)
	{
		ColumnFormulae* formule = &fActions[i];
		Field* cri = formule->GetField();
		sLONG typondisk = cri->GetTyp();
		DB4D_ColumnFormulae action = formule->GetAction();
		ValueKind typresult = formule->GetResultType();
		Boolean isnull = false;

		if (numrec == fCurrec)
		{
			ValPtr cv = formule->GetCurrecValue();
			if (cv == nil || cv->IsNull())
				isnull = true;
			if (!isnull)
			{
				ValPtr convertedValue = cv->ConvertTo( ind->GetScalarKind());
				if (convertedValue != nil)
					convertedValue->WriteToPtr(&data);
				else
					isnull = true;
				delete convertedValue;
			}
		}
		else
		{
			if (dejaChargee != nil)
			{
				ValPtr cv = dejaChargee->GetFieldValue(cri, err, false, true);
				if (cv == nil || cv->IsNull())
					isnull = true;
				if (!isnull)
				{
					cv->WriteToPtr(&data);
				}
			}
		}

		if (isnull || err != VE_OK /* tester si la cle est une valeur nulle*/)
		{
		}
		else // la cle n'est pas une valeur nulle
		{
			formule->AddCount();
			if (action != DB4D_Count)
			{
				switch (action)
				{
				case DB4D_Max:
					switch (typresult)
					{
					case VK_DURATION:
						*(formule->GetResultDurationPtr()) = VDuration((sLONG8)data);
						break;

					case VK_FLOAT:
						*(formule->GetResultFloatPtr()) = VFloat((sLONG8)data);
						break;

					case VK_REAL:
						*(formule->GetResultRealPtr()) = data;
						break;

					case VK_SUBTABLE:
					case VK_SUBTABLE_KEY:
					case VK_LONG8:
						*(formule->GetResultLong8Ptr()) = (sLONG8)data;
						break;

					case VK_TIME:
						VTime dd(data);
						//dd.FromMilliseconds((sLONG8)data);
						formule->SetGenericValue(&dd);
						break;
					}
					break;

				case DB4D_Min:
					if (formule->FirstTime())
					{
						switch (typresult)
						{
						case VK_DURATION:
							*(formule->GetResultDurationPtr()) = VDuration((sLONG8)data);
							break;

						case VK_FLOAT:
							*(formule->GetResultFloatPtr()) = VFloat((sLONG8)data);
							break;

						case VK_REAL:
							*(formule->GetResultRealPtr()) = data;
							break;

						case VK_SUBTABLE:
						case VK_SUBTABLE_KEY:
						case VK_LONG8:
							*(formule->GetResultLong8Ptr()) = (sLONG8)data;
							break;

						case VK_TIME:
							VTime dd(data);
							//dd.FromMilliseconds((sLONG8)data);
							formule->SetGenericValue(&dd);
							break;

						}
					}
					break;

				case DB4D_Average:
				case DB4D_Sum:
					switch (typresult)
					{
					case VK_DURATION:
						formule->GetResultDurationPtr()->Add(VDuration((sLONG8)data));
						break;

					case VK_FLOAT:
						formule->GetResultFloatPtr()->Add(*(formule->GetResultFloatPtr()), VFloat((sLONG8)data));
						break;

					case VK_REAL:
						*(formule->GetResultRealPtr()) = *(formule->GetResultRealPtr()) + data;
						break;

					case VK_SUBTABLE:
					case VK_SUBTABLE_KEY:
					case VK_LONG8:
						*(formule->GetResultLong8Ptr()) = *(formule->GetResultLong8Ptr()) + (sLONG8)data;
						break;

					}
					break;

				case DB4D_Average_distinct:
				case DB4D_Sum_distinct:
				case DB4D_Count_distinct:
					switch (typresult)
					{
					case VK_DURATION:
						formule->AddDistinctDuration(VDuration((sLONG8)data));
						break;

					case VK_FLOAT:
						formule->AddDistinctFloat(VFloat((sLONG8)data));
						break;

					case VK_REAL:
						formule->AddDistinctReal(data);
						break;

					case VK_SUBTABLE:
					case VK_SUBTABLE_KEY:
					case VK_LONG8:
						formule->AddDistinctLong8((sLONG8)data);
						break;

					case VK_TIME:
						VTime dd(data);
						//dd.FromMilliseconds((sLONG8)data);
						formule->AddDistinctGeneric(&dd);
						break;

					}
					break;

				} // end of switch action
			
				

			}

			formule->FirstTime();
		}

	}

	if (dejaChargee != nil)
		dejaChargee->Release();

	return err;
}



template <>
inline VError ColumnFormulas::ExecuteOnOneKey<VUUIDBuffer>(IndexInfo* ind, VUUIDBuffer data, sLONG numrec, Boolean& stop, BaseTaskInfo* context)
{
	VError err = VE_OK;
	
	FicheInMem* dejaChargee = nil;
	if (context != nil)
	{
		dejaChargee = context->RetainRecAlreadyIn(ind->GetTargetTable()->GetDF(), numrec);
	}
	sLONG i,nb = fActions.GetCount();
	for (i = 0; i<nb && err == VE_OK; i++)
	{
		ColumnFormulae* formule = &fActions[i];
		Field* cri = formule->GetField();
		sLONG typondisk = cri->GetTyp();
		DB4D_ColumnFormulae action = formule->GetAction();
		ValueKind typresult = formule->GetResultType();
		Boolean isnull = false;
		
		if (numrec == fCurrec)
		{
			ValPtr cv = formule->GetCurrecValue();
			if (cv == nil || cv->IsNull())
				isnull = true;
			if (!isnull)
			{
				xbox_assert( cv->GetValueKind() == VK_UUID);
				cv->WriteToPtr(&data);
			}
		}
		else
		{
			if (dejaChargee != nil)
			{
				ValPtr cv = dejaChargee->GetFieldValue(cri, err, false, true);
				if (cv == nil || cv->IsNull())
					isnull = true;
				if (!isnull)
				{
					cv->WriteToPtr(&data);
				}
			}
		}
		
		if (isnull || err != VE_OK /* tester si la cle est une valeur nulle*/)
		{
		}
		else // la cle n'est pas une valeur nulle
		{
			formule->AddCount();
			formule->FirstTime();
		}
	}
	return err;
}




							/* -----------------------------------------------  */


template <class Type, sLONG MaxCles>
VError Index_NonOpened::CheckScalarKeys(ToolLog* log)
{
	VError errexec = VE_OK;

	if (fHeaderIsValid && (fPagesMapIsValid || !fPagesMapHasBeenChecked))
	{
		fSomePagesAreInvalid = false;

		if (HBT.FirstPage != -1)
		{
			fCurrentKey = nil;
			FullCleIndex<Type, MaxCles> xCurrentKey;

			fFirstKeyToCheck = true;
			sLONG curpage = 0;
			fDejaPages = new Bittab;

			errexec = CheckScalarPage(HBT.FirstPage, log, curpage, xCurrentKey);
		}
	}

	return errexec;
}


template <class Type, sLONG MaxCles>
VError Index_NonOpened::CheckScalarPage(sLONG numpage, ToolLog* log, sLONG &curpage, FullCleIndex<Type, MaxCles>& currentKey)
{
	VError errexec = VE_OK;
	VError err = VE_OK;

	OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
	if (numpage < 0 || numpage > IHD.nbpage)
	{
		assert(false);
	}
	else
	{
		if (fDejaPages->isOn(numpage))
		{
			IndexPageProblem pb(TOP_IndexPage_PageUsedInCircularReference, fNum, numpage);
			errexec = log->Add(pb);			
		}
		else
		{
			curpage++;
			DataAddr4D addr;
			sLONG len;
			addr = fOwner->GetAddrFromTable(fTempCachePageAddr, numpage, IHD.addrprim, IHD.nbpage, err, false, &len);
			errexec = fDejaPages->Set(numpage, true);
			if (errexec == VE_OK)
				errexec = log->Progress(curpage);
			if (errexec == VE_OK && err == VE_OK)
			{
				if (fOwner->IsAddrValid(addr, false))
				{
					if (len >0 && len < 10000000)
					{
						log->MarkAddr_Index(addr, len, fNum, numpage);
						sLONG taglen = 0;
						DataBaseObjectHeader tag;
						err = tag.ReadFrom(fOwner, addr);
						if (err != VE_OK)
						{
							IndexPageProblem pb(TOP_PhysicalDataIsInvalid, fNum, numpage);
							errexec = log->Add(pb);
						}
						if (err == VE_OK && errexec == VE_OK)
						{
							err = tag.ValidateTag(BtreePageDataHeader<Type>::DBOH_BtreePageTemplate, numpage, -2);
							if (err != VE_OK)
							{
								IndexPageProblem pb(TOP_TagIsInvalid, fNum, numpage);
								errexec = log->Add(pb);
							}
						}

						if (err == VE_OK && errexec == VE_OK)
						{
							BtreePageData<Type, MaxCles> btp;
							taglen = tag.GetLen();
							if ((len == ((sLONG)sizeof(btp)+kSizeDataBaseObjectHeader)) && (taglen == (sLONG)sizeof(btp)) )
							{
								err=fOwner->readlong( &btp, taglen, addr, kSizeDataBaseObjectHeader);
								if (err != VE_OK)
								{
									IndexPageProblem pb(TOP_PhysicalDataIsInvalid, fNum, numpage);
									errexec = log->Add(pb);									
								}

								if (err == VE_OK && errexec == VE_OK)
								{
									err = tag.ValidateCheckSum(	&btp, taglen);
									if (err != VE_OK)
									{
										IndexPageProblem pb(TOP_CheckSumIsInvalid, fNum, numpage);
										errexec = log->Add(pb);									
									}
								}

								if (tag.NeedSwap())
								{
									btp.SwapBytes();
								}

								if (err == VE_OK && errexec == VE_OK)
								{
									if (btp.nkeys<=0 || btp.nkeys > MaxCles)
									{
										IndexPageProblem pb(TOP_IndexPage_NbKeysIsInvalid, fNum, numpage);
										errexec = log->Add(pb);									
									}

									if (errexec == VE_OK)
									{
										if (btp.souspage0 != -1)
										{
											if (IsOKPageNum(btp.souspage0, numpage, errexec, log))
												errexec = CheckScalarPage(btp.souspage0, log, curpage, currentKey);
										}
									}

									sLONG maxrecnum = kMaxRecordsInTable;
									Bittab* DeletedRecords = nil;
									DataFile_NotOpened* df = fOwner->GetDataTableWithTableDefNum(fTable);
									if (df != nil)
									{
										if (df->RecordsHaveBeenChecked())
											DeletedRecords = df->RecordsDeleted();
										if (df->NbFicIsValid())
											maxrecnum = df->GetMaxRecords();
									}

									if (errexec == VE_OK)
									{
										CleIndex<Type> *curp = &btp.cles[0];
										for (sLONG i=0;i<btp.nkeys && errexec == VE_OK;i++, curp++)
										{
											Boolean ok = true;
											CompareResult comp;

											if (fFirstKeyToCheck)
												comp = CR_SMALLER;
											else
											{
												if (curp->cle == currentKey.cle)
												{
													if (curp->qui == currentKey.qui)
														comp = CR_EQUAL;
													else
													{
														if (currentKey.qui < curp->qui )
															comp = CR_SMALLER;
														else
															comp = CR_BIGGER;
													}
												}
												else
												{
													if (currentKey.cle < curp->cle )
														comp = CR_SMALLER;
													else
														comp = CR_BIGGER;
												}
											}

											fFirstKeyToCheck = false;

											if (errexec == VE_OK)
											{
												if (comp == CR_BIGGER || comp == CR_EQUAL)
												{
													IndexPageProblem pb(TOP_IndexPage_KeyOrderIsInvalid, fNum, numpage);
													errexec = log->Add(pb);;
												}
											}

											currentKey = *curp;

											if (errexec == VE_OK)
											{
												if (curp->qui > kMaxRecordsInTable)
												{
													IndexPageProblem pb(TOP_IndexPage_InvalidRecordNumber, fNum, numpage);
													errexec = log->Add(pb);;
												}
												else
												{
													if (curp->qui < 0)
													{
														if (fIndexHeaderType != DB4D_Index_BtreeWithCluster)
														{
															IndexPageProblem pb(TOP_IndexPage_InvalidRecordNumber, fNum, numpage);
															errexec = log->Add(pb);;
														}
													}
													else
													{
														if (fRecordsInKeys != nil)
														{
															if (fType != DB4D_Index_OnKeyWords)
															{
																if (fRecordsInKeys->isOn(curp->qui))
																{
																	IndexPageProblem pb(TOP_IndexPage_DuplicatedRecordNumber, fNum, numpage);
																	errexec = log->Add(pb);;
																}
															}
															if (errexec == VE_OK)
																errexec = fRecordsInKeys->Set(curp->qui, true);
														}
														if (errexec == VE_OK)
														{
															if (DeletedRecords != nil)
															{
																if (DeletedRecords->isOn(curp->qui))
																{
																	IndexPageProblem pb(TOP_IndexPage_DeletedRecordNumber, fNum, numpage);
																	errexec = log->Add(pb);;
																}
															}
														}

													}

												}
											}

											if (errexec == VE_OK)
											{
												if (curp->souspage != -1)
												{
													if (IsOKPageNum(curp->souspage, numpage, errexec, log))
														errexec = CheckScalarPage(curp->souspage, log, curpage, currentKey);
												}
											}
										}
									}

								}

								if (errexec == VE_OK && log->IsCompacting())
								{
									Base4D* target = log->GetTargetCompact();
									if (fOwner->GetStruct() == nil)
										target = target->GetStructure();

									DataAddr4D pageaddr = target->findplace(taglen + kSizeDataBaseObjectHeader, nil, err, -kIndexSegNum, nil);

									BtreePage<Type, MaxCles>* page = new BtreePage<Type, MaxCles>(log->GetTargetIndexCompact(), numpage, fIndexHeaderType == DB4D_Index_BtreeWithCluster, ObjCache::PageIndexAccess, &btp);
									page->setaddr(pageaddr, false);
									page->setmodif(true, log->GetTargetIndexCompact()->GetDB(), nil);
									page->Release();

									DataBaseObjectHeader tag2(&btp, taglen, BtreePageDataHeader<Type>::DBOH_BtreePageTemplate, numpage, -2);
									err = tag2.WriteInto(target, pageaddr, nil, -1);
									if (err == VE_OK) 
										err=target->writelong( &btp, taglen, pageaddr, kSizeDataBaseObjectHeader, nil, -1);


									log->GetTargetIndexCompact()->GetHeader()->SetPageAddr(curstack, numpage, pageaddr, taglen + kSizeDataBaseObjectHeader);

								}

							}
							else
							{
								IndexPageProblem pb(TOP_LengthIsInvalid, fNum, numpage);
								errexec = log->Add(pb);
							}
						}
					}
					else
					{
						IndexPageProblem pb(TOP_LengthIsInvalid, fNum, numpage);
						errexec = log->Add(pb);
					}
				}
				else
				{
					IndexPageProblem pb(TOP_AddrIsInvalid, fNum, numpage);
					errexec = log->Add(pb);
				}
			}
		}
	}
	return errexec;
}


							/* -----------------------------------------------  */



							/* -----------------------------------------------  */



#endif



