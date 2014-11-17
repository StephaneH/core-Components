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

inline sLONG BitTabChunkSize(sLONG siz) { return (siz+127L) & (-128L); }

Bittab::Bittab(uBOOL isATransPart)
{
	nbbit=0;
	nbpage=0;
	nbpageToWrite = 0;
	tabmem = nil;
	tabplein = nil;
	TabPage=nil;
	equisel=-1;
	decompte=-1;
	SelisCluster=false;
	db = nil;
	isempty = 2;
	fIsRemote = false;
	//fRemoteContext = nil;
	fIDHasBeenGenerated = false;
	fIsASel = false;
	fAutoPurge = true;
	fNeedPurgeCount = 0;
#if debug_bittab_threadsafe
	fDebugTaskOwner = 0;
#endif

#if debugLeakCheck_Bittab
	if (debug_candumpleaks)
		DumpStackCrawls();
	if (debug_canRegisterLeaks)
		RegisterStackCrawl(this);
#endif
}

Bittab::~Bittab()
{	
#if debugLeakCheck_Bittab
	UnRegisterStackCrawl(this);
#endif
	vide();

	if (fIsRemote && !fIsASel)
	{
		VDBMgr::GetManager()->AddReleasedSelID(fID);
	}			

	/*
	if (fRemoteContext != nil)
		fRemoteContext->Release();
		*/
}


VError Bittab::ThrowError( VError inErrCode, ActionDB4D inAction) const
{
	if (db != nil)
	{
		return db->ThrowError(inErrCode, inAction);
	}
	else
	{
		VErrorDB4DBase *err = new VErrorDB4DBase(inErrCode, inAction);
		VTask::GetCurrent()->PushRetainedError( err);
	}

	return inErrCode;
}


#if autocheckobj
uBOOL Bittab::CheckObjInMem(void)
{
	sLONG i;
	
	CheckAssert(IsValidFastPtr(tabmem));
	CheckAssert(IsValidFastPtr(tabplein));
	CheckAssert(IsValidFastPtr(TabPage));
	CheckAssert(nbpage>=0);
	if (tabmem!=nil)
	{
		for (i=0;i<nbpage;i++)
		{
			if (tabplein[i] == kBitPageNonVide )
			{
				bitsetptr p=tabmem[i];
				if (p!=nil)
				{
					CheckAssert(IsValidFastPtr(p));
				}
			}
		}
	}
	
	return(true);
}
#endif

void Bittab::vide(void)
{
	sLONG i;
	
	if (tabmem!=nil)
	{
		for (i=0;i<nbpage;i++)
		{
			if (tabplein[i] == kBitPageNonVide )
			{
				bitsetptr p = tabmem[i];
				if (p!=nil && p != (bitsetptr)-1)
				{
					FreeFastMem(p);
				}
			}
		}
		
		FreeFastMem( tabmem);
		tabmem=nil;
	}
	if (tabplein!=nil)
	{
		FreeFastMem(tabplein);
		tabplein=nil;
	}
	if (TabPage!=nil)
	{
		FreeFastMem( TabPage);
		TabPage = nil;
	}
#if debug_bittab_threadsafe
	debugCheckOwner();
#endif
	equisel = -1;
	decompte = -1;
}


sLONG Bittab::PourLibereMem(sLONG combien, uBOOL tout)
{
	sLONG i,tot;
	
	tot=0;
	if (tabmem!=nil)
	{
		for (i=0;i<nbpage;i++)
		{
			if (tabplein[i] == kBitPageNonVide )
			{
				bitsetptr p = tabmem[i];
				if (p!=nil && p!=(bitsetptr)-1)
				{
					if (p[kNbLongParSet]==0)
					{
						tot += kSizeSetMem;
						FreeFastMem(p);
						tabmem[i] = nil;
					}
				}
			}
		}
	}
	
	if (tot==0) tot=1;
	return(tot);
}


uBOOL Bittab::PourOkdel(void)
{
	uBOOL ok;
	sLONG i;
	
	ok=true;
	if (tabmem != nil)
	{
		for (i=0;i<nbpage;i++)
		{
			if (tabplein[i] == kBitPageNonVide )
			{
				bitsetptr p = tabmem[i];
				if (p!=nil && p!= (bitsetptr)-1)
				{
					if (p[kNbLongParSet]!=0)
					{
						ok=false;
					}
				}
			}
		}
	}
	
	return(ok);
}

/*
VError Bittab::SaveBits(BaseTaskInfo* context)
{
	sLONG i;
	DataAddr4D lll;
	VError err = VE_OK;

	if (SelisCluster)
	{

		for (i=0;i<nbpage && err == VE_OK;i++)
		{
			lll=TabPage[i];
			bitsetptr p = tabmem[i];
			
			if ( p != (bitsetptr)(-1) )
			{
				if ( tabplein[i] == kBitPageNonVide )
				{
					if (lll < 2)
					{
						lll=db->findplace(kSizeSetDisk+kSizeDataBaseObjectHeader, context, 0, (ObjAlmostInCache*)this);
						if (lll == -1)
							err = DerniereErreur();
						else
							TabPage[i]=lll;
					}
				}
				else
				{
					if (lll > 1)
					{
						db->libereplace(lll, kSizeSetDisk+kSizeDataBaseObjectHeader, context, (ObjAlmostInCache*)this);
						TabPage[i]=tabplein[i];
						err = DerniereErreur();
					}
				}
			}
		}
	}
	
	if (err != VE_OK)
		err = db->ThrowError(VE_DB4D_CANNOTSAVESET, DBaction_SavingSet);
	return(err);
}
*/

sLONG Bittab::PourSaveobj(DataAddr4D xaddr)
{
	sLONG i,tot;
	uLONG lll;
	DataAddr4D ou;
	VError err = VE_OK;

#if debuglogwrite
	VString wherefrom(L"Bittab PourSaveObj");
	VString* whx = &wherefrom;
#else
	VString* whx = nil;
#endif

	xbox_assert(xaddr == SelAddr);

	sLONG xnbpage = VInterlocked::AtomicGet(&nbpageToWrite);

#if debugIndexOverlap_strong
	if (xnbpage < nbpage-1)
	{
		xnbpage = xnbpage; // break here #index
	}
#endif

	tot=0;
	//if (tabmem!=nil)
	{

#if WITH_ASSERT
		if (tabmem == nil)
			xbox_assert(nbbit == 0);
#endif

		lll=sel_bitsel;
		DataBaseObjectHeader tag(TabPage, xnbpage<<3, DBOH_SetDiskTable, -1, -1);
		err = tag.WriteInto(db, SelAddr, whx);
		if (err == VE_OK) 
			err = db->writelong(&lll,4,SelAddr,kSizeDataBaseObjectHeader, whx);
		if (err == VE_OK) 
			err = db->writelong(&nbbit,4,SelAddr,kSizeDataBaseObjectHeader+4, whx);
		if (err == VE_OK) 
			err = db->writelong(&decompte,4,SelAddr,kSizeDataBaseObjectHeader+8, whx);
		if (err == VE_OK) 
			err = db->writelong( TabPage, xnbpage<<3, SelAddr, kSizeDataBaseObjectHeader+12, whx);
		
#if debuglr == 114
		VString s(L" BitSel saveobj : ");
		VString s2;
		s2.FromLong8(SelAddr);
		s+=s2;
		s+=L" , ";
		s2.FromLong(nbpage<<3+12+kSizeDataBaseObjectHeader);
		s+=s2;
		s+=L"\n";
		DebugMsg(s);
#endif

		if (err == VE_OK && tabmem != nil)
		{
			if (xnbpage > nbpage)
				xnbpage = nbpage;
			for (i=0;i<xnbpage && err == VE_OK;i++)
			{
				bitsetptr p = tabmem[i];
				if (p!=(bitsetptr)(-1))
				{
					if (tabplein[i] == kBitPageNonVide )
					{
						if (p!=nil)
						{
							if (p[kNbLongParSet]!=0)
							{
								tot += kSizeSetDisk;
								ou=TabPage[i];
								DataBaseObjectHeader tag(p, kSizeSetDisk, DBOH_SetDisk, i, -1);
								err = tag.WriteInto(db, ou, whx);
								if (err == VE_OK)
									err = db->writelong(p,kSizeSetDisk,ou,kSizeDataBaseObjectHeader, whx);
								if (err == VE_OK)
									p[kNbLongParSet] = 0;

#if debuglr
								uLONG xp2[kNbLongParSet+1];
								bitsetptr p2 = &xp2[0];
								{
									DataBaseObjectHeader tag2;
									VError err2 = tag2.ReadFrom(db, ou, nil);
									if (err2 == VE_OK)
										err2 = tag2.ValidateTag(DBOH_SetDisk, i, -1);
									if (err2 == VE_OK)
										err2 = db->readlong(p2,kSizeSetDisk, ou, kSizeDataBaseObjectHeader);
									if (err2 == VE_OK)
										err2 = tag2.ValidateCheckSum(p2, kSizeSetDisk);
									if (err2 != VE_OK)
									{
										sLONG xbreak = 1; // put a break here
										xbox_assert(err2 == VE_OK);
									}
								}
#endif
							}
						}
					}
				}
			}
		}
	}
	
	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTSAVESET, DBaction_SavingBitSel);
	if (tot==0) tot=1;
	return(tot);
}


void Bittab::SetAddr(DataAddr4D addr, Base4D *xdb)
{
	SelAddr=addr;
	SelisCluster=true;
	db=xdb;
}


VError Bittab::LoadBitSel(DataBaseObjectHeader& tag)
{
	sLONG nb,i;
	VError err;
	
	err=db->readlong(&nb,4,SelAddr,4+kSizeDataBaseObjectHeader);
	if (err==VE_OK)
	{
		if (tag.NeedSwap())
			nb = SwapLong(nb);
		err=aggrandit(nb);
	}
	if (err==VE_OK)
	{
		err=db->readlong(&decompte,4,SelAddr,8+kSizeDataBaseObjectHeader);
		if (err==VE_OK)
		{
			if (tag.NeedSwap())
				decompte = SwapLong(decompte);
		}

		// TabPage est deja alloue dans le aggrandit normalement
		if (TabPage == nil)
			TabPage = (DataAddr4D*)GetFastMem( BitTabChunkSize(nbpage<<3), false, 'Bse1');

		if (TabPage != nil)
		{
			err=db->readlong( TabPage,nbpage<<3,SelAddr,12+kSizeDataBaseObjectHeader);
			if (err == VE_OK)
				err = tag.ValidateCheckSum(TabPage, nbpage<<3);
			if (err == VE_OK)
			{
				if (tag.NeedSwap())
					ByteSwap(TabPage, nbpage);
			}

			for (i=0;i<nbpage;i++)
			{
				tabmem[i] = (bitsetptr)(-1);
			}
			antelen=8+(nbpage<<3);

		}
		else 
			err = ThrowError(memfull, DBaction_LoadingBitSel);
	}
	
	if (err != VE_OK )
		err = ThrowError(VE_DB4D_CANNOTLOADBITSEL, DBaction_LoadingBitSel);
		
	return(err);
}


bitsetptr Bittab::loadmemAndBuildIfEmpty(sLONG n, VError& err)
{
	err = VE_OK;
	bitsetptr p = loadmem(n, err);
	if (err == VE_OK && p == nil)
	{
		p=(bitsetptr)GetFastMem(kSizeSetMem, false, 'bsel');
		if (p==nil)
		{
			err=ThrowError(memfull, DBaction_ModifyingSet);
		}
		else
		{
			if (SelisCluster)
			{
				DataAddr4D lll=db->findplace(kSizeSetDisk+kSizeDataBaseObjectHeader, nil, err, -kIndexSegNum, (ObjAlmostInCache*)this);
				if (err == VE_OK)
				{
					TabPage[n]=lll;
#if debugIndexOverlap_strong
					di_IndexOverLap::AddClusterPart(nil, -1, n, lll, kSizeSetDisk+kSizeDataBaseObjectHeader);
#endif
				}
			}
			if (tabplein[n])
			{
				_rau(p,kSizeSetMem);
			}
			else
			{
				_raz(p,kSizeSetMem);
			}
			tabmem[n]=p;
			tabplein[n] = kBitPageNonVide;
		}
	}

	if (err != VE_OK)
		err=ThrowError(VE_DB4D_CANNOTMODIFYSET, DBaction_ModifyingSet);

	return p;
}


bitsetptr Bittab::loadmem(sLONG n, VError& err)
{
	DataAddr4D lll;
	err = VE_OK;
	
	// L.E. en fait ce devrait etre < nbpage mais...
	xbox_assert( n >= 0 && n <= nbpage);

	// L.E. 12/03/02
	if (tabmem == nil)
		return nil;

	bitsetptr p = tabmem[n];
	if (p==(bitsetptr)(-1))
	{
		lll=TabPage[n];
		if (lll<2)
			tabplein[n]=lll;
		else
			tabplein[n]=kBitPageNonVide;
		p=nil;
	}
	if (p != nil && p != (bitsetptr)(-2))
		return p;
	
	if ( tabplein[n] == kBitPageNonVide )
	{
		if (p==nil)
		{
			if (fIsRemote)
			{
				p=(bitsetptr)GetFastMem(kSizeSetMem, false, 'bsel');
				if (p!=nil)
				{
					IRequest* req = db->CreateRequest(nil, Req_AskForASelectionPart + kRangeReqDB4D);
					if (req != nil)
					{
						req->PutBaseParam(db);
						VStream* reqstream = req->GetOutputStream();
						//reqstream->PutLong(sel_bitsel);
						ToServer(reqstream, nil, false);
						req->PutLongParam(n);
						VError err = req->GetLastError();
						if (err == VE_OK)
						{
							err = req->Send();
							if (err == VE_OK)
							{
								uBYTE fillpage = req->GetByteReply(err);
								if (fillpage == kBitPageAllFalse)
									tabplein[n] = kBitPageAllFalse;
								else if (fillpage == kBitPageAllTrue)
									tabplein[n] = kBitPageAllTrue;
								else
								{
									tabplein[n] = kBitPageNonVide;
									err = req->GetArrayLongReply((sLONG*)p, kNbLongParSet);
								}
							}
						}
						req->Release();
					}
					else
						err = ThrowError(memfull, noaction);

					if (err != VE_OK )
					{
						FreeFastMem(p);
					}
					else
					{
						if (tabplein[n] == kBitPageNonVide)
						{
							tabmem[n] = p;
							p[kNbLongParSet]=0;
						}
						else
						{
							FreeFastMem(p);
							tabmem[n] = nil;
						}
					}
				}
				else
					err = ThrowError(memfull, DBaction_LoadingSet);
			}
			else
			{
				xbox_assert(SelisCluster);
				p=(bitsetptr)GetFastMem(kSizeSetMem, false, 'bsel');
				if (p!=nil)
				{
					lll=TabPage[n];
					DataBaseObjectHeader tag;
					err = tag.ReadFrom(db, lll, nil);
					if (err == VE_OK)
						err = tag.ValidateTag(DBOH_SetDisk, n, -1);
					if (err == VE_OK)
						err = db->readlong(p,kSizeSetDisk, lll, kSizeDataBaseObjectHeader);
					if (err == VE_OK)
					{
						if (tag.NeedSwap())
							ByteSwap(p, kNbLongParSet);
					}
					if (err != VE_OK )
					{
						FreeFastMem(p);
					}
					else
					{
						tabmem[n] = p;
						p[kNbLongParSet]=0;
					}
				}
				else
					err = ThrowError(memfull, DBaction_LoadingSet);
			}
		}
	}
	else p=nil;
	
	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTLOADSET, DBaction_LoadingSet);

	return(p);
}


sLONG Bittab::CalcLenOnDisk(void)
{
	antelen=(nbpage<<3)+12;
	return(antelen);
}


uBOOL Bittab::isOn(sLONG n)
{
	uBOOL res;
	sLONG npage,n1,nL;
	bitsetptr p;
	VError err;

	if ( (n>=nbbit) || (n<0) )
	{
		res=false;
	}
	else
	{
		npage=n>>kRatioSet;
		p=loadmem(npage, err);
		if (p == nil)
		{
			res=tabplein[npage];
		}
		else
		{
			n1=n & (kNbBitParSet-1);
			nL=n1 >> 5;
			n1=n1 & 31;
			if ( ((uLONG)(1<<n1)) & (*(p+nL)) )
			{
				res=true;
			}
			else
			{
				res=false;
			}
		}
	}

	return(res);
}


VError Bittab::SetRange(uBOOL b, sLONG pagenum)
{
	if (pagenum < nbpage)
	{
		if (tabplein[pagenum] == kBitPageNonVide)
		{
			if (tabmem[pagenum] != nil)
				FreeFastMem(tabmem[pagenum]);
			if (SelisCluster)
			{
#if debugIndexOverlap_strong
				di_IndexOverLap::RemoveClusterPart(nil, -1, pagenum, TabPage[pagenum], kSizeSetDisk+kSizeDataBaseObjectHeader);
#endif
				db->libereplace(TabPage[pagenum], kSizeSetDisk+kSizeDataBaseObjectHeader, nil, (ObjAlmostInCache*)this);
			}
			tabmem[pagenum]=(bitsetptr)(-2);
		}
		tabplein[pagenum] = b;
		if (SelisCluster)
			TabPage[pagenum] = tabplein[pagenum];
#if debug_bittab_threadsafe
		debugCheckOwner();
#endif
		decompte = -1;
		equisel = -1;
	}
	return VE_OK;
}


VError Bittab::SetRange(bitsetptr autre, sLONG pagenum)
{
	VError err = VE_OK;
	if (pagenum < nbpage)
	{
		bitsetptr p = nil;

		if (tabplein[pagenum] != kBitPageNonVide)
		{
			p=(bitsetptr)GetFastMem(kSizeSetMem, false, 'bsel');
			if (p==nil)
			{
				err=ThrowError(memfull, DBaction_ModifyingSet);
			}
			else
			{
				if (tabplein[pagenum])
				{
					_rau(p,kSizeSetMem);
				}
				else
				{
					_raz(p,kSizeSetMem);
				}
				if (SelisCluster)
				{
					DataAddr4D lll=db->findplace(kSizeSetDisk+kSizeDataBaseObjectHeader, nil, err, -kIndexSegNum, (ObjAlmostInCache*)this);
					if (err == VE_OK)
					{
#if debugIndexOverlap_strong
						di_IndexOverLap::AddClusterPart(nil, -1, pagenum, lll, kSizeSetDisk+kSizeDataBaseObjectHeader);
#endif
						TabPage[pagenum]=lll;
					}
				}
				tabmem[pagenum]=p;
				tabplein[pagenum] = kBitPageNonVide;
			}
		}
		else
		{
			p = loadmem(pagenum, err);
		}

		if (p != nil)
		{
			bitsetptr p2 = p;
			for (sLONG i = 0; i < kNbLongParSet; i++, p++, autre++)
			{
				*p = *p | *autre;
			}
			//std::copy(autre, autre + kNbLongParSet, p);
			p2[kNbLongParSet] = -1;
#if debug_bittab_threadsafe
			debugCheckOwner();
#endif
			decompte = -1;
			equisel = -1;
		}

	}

	if (err != VE_OK)
		err=ThrowError(VE_DB4D_CANNOTMODIFYSET, DBaction_ModifyingSet);

	return err;
}



VError Bittab::ClearOrSet(sLONG n, uBOOL b, uBOOL cangrow)
{
	sLONG npage,n1,nL;
	bitsetptr p;
	uBOOL testbits = false;
	VError err;
	
	if (fNeedPurgeCount > 10000 && fAutoPurge && !SelisCluster)
		Epure();

	err=VE_OK;
	if (n<0)
	{
		if (SelisCluster)
		{
			n=-n;
			testbits=true;
		}
		else
		{
			n=-n;
			//return ThrowError(VE_DB4D_CANNOTMODIFYSET, DBaction_ModifyingSet);
		}
	}
	else testbits=false;
#if debug_bittab_threadsafe
	debugCheckOwner();
#endif
	equisel = -1;
	//decompte=-1;
	if (n>=nbbit && cangrow)
	{
		err = aggrandit(n+1/*+1024*/);
	}
	if (n<nbbit)
	{
		if (fAutoPurge)
			fNeedPurgeCount++;
		npage=n>>kRatioSet;
		
		p=loadmem(npage, err);
		
		if (p==nil)
		{
			if (err != VE_OK)
			{
				//err=ThrowError(VE_DB4D_CANNOTMODIFYSET, DBaction_ModifyingSet);
			}
			else
			{
				if (b != tabplein[npage])
				{
					p=(bitsetptr)GetFastMem(kSizeSetMem, false, 'bsel');
					if (p==nil)
					{
						err=ThrowError(memfull, DBaction_ModifyingSet);
					}
					else
					{
						if (SelisCluster)
						{
							DataAddr4D lll=db->findplace(kSizeSetDisk+kSizeDataBaseObjectHeader, nil, err, -kIndexSegNum, (ObjAlmostInCache*)this);
							if (err == VE_OK)
							{
#if debugIndexOverlap_strong
								di_IndexOverLap::AddClusterPart(nil, -1, npage, lll, kSizeSetDisk+kSizeDataBaseObjectHeader);
#endif
								TabPage[npage]=lll;
							}
						}
						if (tabplein[npage])
						{
							_rau(p,kSizeSetMem);
						}
						else
						{
							_raz(p,kSizeSetMem);
						}
						tabmem[npage]=p;
						tabplein[npage] = kBitPageNonVide;
					}
				}
			}
		}
		
		if (p != nil)
		{
			n1=n & (kNbBitParSet-1);
			nL=n1 >> 5;
			n1=n1 & 31;
			p[kNbLongParSet] = -1;
			if (b)
			{
				if (decompte != -1 && !(((uLONG)(1<<n1)) & (*(p+nL))) )
				{
					decompte++;
				}
				*(p+nL)=*(p+nL) | (1<<n1);
				if (testbits && AllFF(p,kSizeSetDisk))
				{
					if (SelisCluster)
					{
#if debugIndexOverlap_strong
						di_IndexOverLap::RemoveClusterPart(nil, -1, npage, TabPage[npage], kSizeSetDisk+kSizeDataBaseObjectHeader);
#endif
						db->libereplace(TabPage[npage], kSizeSetDisk+kSizeDataBaseObjectHeader, nil, (ObjAlmostInCache*)this);
					}
					FreeFastMem(p);
					tabmem[npage]=nil;
					tabplein[npage]=kBitPageAllTrue;
					TabPage[npage] = tabplein[npage];
				}
			}
			else
			{
				if (decompte != -1 && (((uLONG)(1<<n1)) & (*(p+nL))) )
				{
					decompte--;
				}
				*(p+nL)=*(p+nL) & (ALLFF ^ (1<<n1));
				if (testbits && All00(p,kSizeSetDisk))
				{
					if (SelisCluster)
					{
#if debugIndexOverlap_strong
						di_IndexOverLap::RemoveClusterPart(nil, -1, npage, TabPage[npage], kSizeSetDisk+kSizeDataBaseObjectHeader);
#endif
						db->libereplace(TabPage[npage], kSizeSetDisk+kSizeDataBaseObjectHeader, nil, (ObjAlmostInCache*)this);
					}
					FreeFastMem(p);
					tabmem[npage]=nil;
					tabplein[npage]=false;
					TabPage[npage] = tabplein[npage];
				}
			}
		}
	}

	if (err != VE_OK)
		err=ThrowError(VE_DB4D_CANNOTMODIFYSET, DBaction_ModifyingSet);
	return(err);
}


VError Bittab::aggranditParBlock(sLONG nbbits)
{
	if (nbbit <= nbbits)
		return aggrandit(nbbits+1024);
	else
		return VE_OK;
}

VError Bittab::Reduit(sLONG nbrec)
{
	if (nbrec<Compte())
	{
		sLONG nieme = PlaceDuNiemeBitAllume(nbrec);
		ClearFrom(nieme);
	}

	return VE_OK;
}

VError Bittab::aggrandit(sLONG nbbits)
{
	sLONG newnbpage;
	sLONG i,start,end,nbit;
	bitsetptr p;
	char* xp;
	VError err;
	
	occupe();
	err=VE_OK;
	
	if (nbbits>nbbit)
	{
		newnbpage=(nbbits+kNbBitParSet-1)>>kRatioSet;
		if (tabmem == nil)
		{
			tabmem = (bitsetptr*)GetFastMem(BitTabChunkSize(sizeof(bitsetptr)*newnbpage), false, 'Bse2');
			if (tabmem == nil) 
				err = ThrowError(memfull, DBaction_IncreasingSetSize);
		}
		else
		{
			if (BitTabChunkSize(sizeof(bitsetptr)*newnbpage) >  BitTabChunkSize(sizeof(bitsetptr)*nbpage) )
			{
				xp = (char*)GetFastMem(BitTabChunkSize(sizeof(bitsetptr)*newnbpage), false, 'Bse3');
				if (xp == nil)
					err = ThrowError(memfull, DBaction_IncreasingSetSize);
				else
				{
					move4block(tabmem, xp, sizeof(bitsetptr)*nbpage);
					FreeFastMem(tabmem);
					tabmem = (bitsetptr*)xp;
				}
			}
		}

#if debugIndexOverlap_strong
		if ((newnbpage > nbpage + 1) && SelisCluster && nbpage != 0)
		{
			err = err; // break here #index
		}
#endif

		if (err==VE_OK)
		{
		
			if (tabplein == nil)
			{
				tabplein = (uBOOL*)GetFastMem(BitTabChunkSize(newnbpage), false, 'Bse4');
				if (tabplein == nil) 
					err = ThrowError(memfull, DBaction_IncreasingSetSize);
			}
			else
			{
				if (BitTabChunkSize(newnbpage) >  BitTabChunkSize(nbpage) )
				{
					xp = (char*)GetFastMem(BitTabChunkSize(newnbpage), false, 'Bse5');
					if (xp == nil)
						err = ThrowError(memfull, DBaction_IncreasingSetSize);
					else
					{
						vBlockMove(tabplein, xp, nbpage);
						FreeFastMem(tabplein);
						tabplein = (uBOOL*)xp;
					}
				}
			}
		
			if (err==VE_OK)
			{
				if (SelisCluster)
				{
					if (TabPage == nil)
					{
						TabPage = (DataAddr4D*)GetFastMem(BitTabChunkSize(newnbpage<<3), false, 'Bse6');
						if (TabPage == nil)
							err = ThrowError(memfull, DBaction_IncreasingSetSize);
					}
					else
					{
						if (BitTabChunkSize(newnbpage<<3) >  BitTabChunkSize(nbpage<<3) )
						{
							xp = (char*)GetFastMem(BitTabChunkSize(newnbpage<<3), false, 'Bse7');
							if (xp == nil)
								err = ThrowError(memfull, DBaction_IncreasingSetSize);
							else
							{
								move4block(TabPage, xp, nbpage<<3);
								FreeFastMem(TabPage);
								TabPage = (DataAddr4D*)xp;
							}
						}
					}
				}
			}
		}
		if (err==VE_OK)
		{
			for (i=nbpage; (i<newnbpage); i++)
			{
				tabmem[i]=(bitsetptr)(-2);
				tabplein[i]=false;
				if (SelisCluster )
					TabPage[i]=0;
			}
			
			sLONG oldnbbit = nbbit;
			sLONG oldnbpage = nbpage;

			nbbit=nbbits;
			nbpage=newnbpage;

			if (oldnbpage == newnbpage)
			{
				p=loadmemAndBuildIfEmpty(newnbpage-1, err);
				if (p!=nil && err == VE_OK)
				{
					start=oldnbbit & (kNbBitParSet-1);
					end=(nbbits-1) & (kNbBitParSet-1);
					
					nbit=start & 31;
					p=p+(start>>5);
					for (i=start; i<=end; i++)
					{
						*p=*p & (ALLFF ^ (1<<nbit));
						nbit++;
						if (nbit>31)
						{
							nbit=0;
							p++;
						}
					}
				}
			}
			
			
		}
	}
	else
	{
		//ClearFrom(nbbits);
	}
	
	libere();
	if (err != VE_OK)
		err=ThrowError(VE_DB4D_CANNOTMODIFYSET, DBaction_IncreasingSetSize);
	return(err);
}


void Bittab::ClearOrSetAll(uBOOL b)
{
	sLONG i;
	bitsetptr p;
	
	occupe();
#if debug_bittab_threadsafe
	debugCheckOwner();
#endif
	decompte = -1;
	equisel=-1;
	
	for (i=0;i<nbpage;i++)
	{
		if (tabplein[i] == kBitPageNonVide )
		{
			p=tabmem[i]; 
			if (p!=nil)
			{
				FreeFastMem(p);
				tabmem[i]=(bitsetptr)(-2);
			}
		}
		tabplein[i]=b;
	}

	libere();
}


void Bittab::ClearFrom(sLONG n)
{
	sLONG i,npage, n1 ,nL, n2;
	bitsetptr p;
	uBOOL		res;
	VError err;
	
	if ((n>=0) && (n<nbbit))
	{
		occupe();
#if debug_bittab_threadsafe
		debugCheckOwner();
#endif
		decompte = -1;
		equisel=-1;
		
		npage=n>>kRatioSet;
		p=loadmem(npage, err);

		// L.E. 12/03/02
		if (tabplein != nil) {
			res=tabplein[npage];
			if (res!=kBitPageNonVide)
			{
				if (tabplein[npage])
				{
					p=(bitsetptr)GetFastMem(kSizeSetMem, false, 'bsel');
					if (p==nil)
					{
						ThrowError(memfull, DBaction_ModifyingSet);
					}
					else
					{
						if (SelisCluster)
						{
							DataAddr4D lll=db->findplace(kSizeSetDisk+kSizeDataBaseObjectHeader, nil, err, -kIndexSegNum, (ObjAlmostInCache*)this);
							if (err == VE_OK)
							{
#if debugIndexOverlap_strong
								di_IndexOverLap::AddClusterPart(nil, -1, npage, lll, kSizeSetDisk+kSizeDataBaseObjectHeader);
#endif
								TabPage[npage]=lll;
							}
						}
						_rau(p,kSizeSetMem);
						tabmem[npage]=p;
						tabplein[npage] = kBitPageNonVide;
					}
				}
				else p = nil;
			}
		}
		
		if (p != nil)
		{
			n1=n & (kNbBitParSet-1);
			nL=n1 >> 5;
			n2=n1 & 31;
			
			for (i = n2; i<32; i++)
			{
				*(p+nL)=*(p+nL) & (ALLFF ^ (1<<i));
			}
			
			for (i = nL+1; i<kNbLongParSet; i++)
			{
				*(p+i) = 0;
			}
		}

		for (i=npage+1;i<nbpage;i++)
		{
			p=tabmem[i];
			if (tabplein[i] == kBitPageNonVide )
			{
				if (p!=nil)
				{
					FreeFastMem(p);
					tabmem[i]=(bitsetptr)(-2);
				}
			}
			tabplein[i]=false;
		}
		
		libere();
	}
}


Boolean Bittab::Intersect(Boolean autre, sLONG pagenum, VError& err)
{
	Boolean res = false;
#if debug_bittab_threadsafe
	debugUse();
#endif

	if (autre)
	{
		if (pagenum < nbpage)
		{
			uBOOL b = tabplein[pagenum];
			if (b == kBitPageNonVide)
			{
				sLONG i;
				bitsetptr p = loadmem(pagenum, err);
				if (err == VE_OK && p != nil)
				{
					for (i = 0; i < kNbLongParSet; i++, p++)
					{
						if (*p != 0)
						{
							res = true;
							break;
						}
					}
				}
			}
			else
			{
				if (b)
					res = true;
			}
		}
	}
#if debug_bittab_threadsafe
	debugUnuse();
#endif

	return res;
}


Boolean Bittab::Intersect(bitsetptr autre, sLONG pagenum, VError& err, vector<sLONG>& outCollisions, sLONG maxCollisions)
{
	Boolean res = false;
#if debug_bittab_threadsafe
	debugUse();
#endif

	if (autre != nil)
	{
		if (pagenum < nbpage)
		{
			uBOOL b = tabplein[pagenum];
			if (b == kBitPageNonVide)
			{
				sLONG i;
				bitsetptr p = loadmem(pagenum, err);
				if (err == VE_OK && p != nil)
				{
					for (i = 0; i < kNbLongParSet; i++, p++, autre++)
					{
						if ((*p & *autre) != 0)
						{
							res = true;
							if (maxCollisions < outCollisions.size())
							{
								for (sLONG k = 0; k < 32 && maxCollisions < outCollisions.size(); k++)
								{
									if ( (((uLONG)(1<<k)) & *p ) != 0 && (((uLONG)(1<<k)) & *autre ) != 0)
									{
										sLONG num = i * 32 + k;
										outCollisions.push_back(num);
									}
								}
							}
							else
								break;
						}
					}
				}
			}
			else
			{
				if (b)
				{
					bitsetptr p = autre;
					for (sLONG i = 0; i < kNbLongParSet; i++, p++)
					{
						if (*p != 0)
						{
							res = true;
							if (maxCollisions < outCollisions.size())
							{
								for (sLONG k = 0; k < 32 && maxCollisions < outCollisions.size(); k++)
								{
									if ( (((uLONG)(1<<k)) & *p ) != 0 )
									{
										sLONG num = i * 32 + k;
										outCollisions.push_back(num);
									}
								}
							}
							else
								break;
						}
					}
				}
			}
		}
	}
#if debug_bittab_threadsafe
	debugUnuse();
#endif

	return res;
}



Boolean Bittab::Intersect(Bittab *autre, VError& err)
{
	Boolean inter = false;
	sLONG i,j;
	bitsetptr p,p2;
	uBOOL b1,b2;
	sLONG nb2;

	//use();
	//autre->use();
	err=VE_OK;
	/*
	equisel=-1;
	decompte = -1;
	*/
#if debug_bittab_threadsafe
	debugUse();
#endif

	//if (!inter && err == VE_OK)
	{
		/*if (nbbit<autre->nbbit)
			err =  aggrandit(autre->nbbit);
		else*/ // L.R le 26 mars 2010
		{
			// on ne doit plus alterer l'autre bitsel L.R le 23 mars 2010
			/*
			if (nbbit > autre->nbbit)
				err = autre->aggrandit(nbbit);
				*/
		}

		nb2 = minl(nbpage, autre->nbpage);

		for (i=0;(i<nb2) && (err==VE_OK) && (!inter);i++)
		{
			p=loadmem(i, err);
			if (err == VE_OK)
			{
				p2=autre->loadmem(i, err);
			}

			if (err == VE_OK)
			{
				if ((p!=nil) && (p2!=nil))
				{
					for (j=0; j<kNbLongParSet && !inter; j++)
					{
						if ( (*p & *p2) != 0)
						{
							inter = true;
							break;
						}
						p++;
						p2++;
					}
				}
				else
				{
					if (p==nil)
					{
						b1=tabplein[i];
						if (p2==nil)
						{
							b2=autre->tabplein[i];
							if (b1 && b2)
								inter = true;
						}
						else
						{
							if (b1)
							{
								for (j=0; j<kNbLongParSet && !inter; j++)
								{
									if ( *p2 != 0)
									{
										inter = true;
										break;
									}
									p2++;
								}
							}
						}
					}
					else
					{
						if (p2 == nil)
						{
							b2 = autre->tabplein[i];
							if (b2)
							{
								for (j=0; j<kNbLongParSet && !inter; j++)
								{
									if ( *p != 0)
									{
										inter = true;
										break;
									}
									p++;
								}
							}
						}
					}
				}
			}
		}
	}

#if debug_bittab_threadsafe
	debugUnuse();
#endif

	if (err != VE_OK)
	{
		err = ThrowError(VE_DB4D_CANNOTMODIFYSET, DBaction_ModifyingSet);
	}

	//autre->unuse();
	//unuse();
	return(inter);
}


VError Bittab::And(Bittab *autre, Boolean cangrow)
{
	VError err;
	sLONG i,j;
	bitsetptr p,p2;
	uBOOL b1,b2;
	sLONG nb2;
	
	//use();
	//autre->use();
	err=VE_OK;
#if debug_bittab_threadsafe
	debugCheckOwner();
#endif
	equisel = -1;
	decompte = -1;

	if (err == VE_OK)
	{
		if (nbbit<autre->nbbit && cangrow)
			err =  aggrandit(autre->nbbit);
		else
		{
			if (nbbit>autre->nbbit)
			{
				ClearFrom(autre->nbbit);
			}
		}
	}

	nb2 = minl(nbpage, autre->nbpage);

	for (i=0;(i<nb2) && (err==VE_OK);i++)
	{
		p=loadmem(i, err);
		if (err == VE_OK)
		{
			p2=autre->loadmem(i, err);
		}
		
		if (err == VE_OK)
		{
			if ((p!=nil) && (p2!=nil))
			{
				p[kNbLongParSet] = -1;
				for (j=0;j<kNbLongParSet;j++)
				{
					*p &= *p2;
					p++;
					p2++;
				}
			}
			else
			{
				if (p==nil)
				{
					b1=tabplein[i];
					if (p2==nil)
					{
						b2=autre->tabplein[i];
						tabplein[i] = b1 && b2;
						tabmem[i] = (bitsetptr)(-2);
					}
					else
					{
						if (b1)
						{
							p=(bitsetptr)GetFastMem(kSizeSetMem, false, 'bsel');
							if (p==nil)
							{
								err=ThrowError(memfull, DBaction_ModifyingSet);
							}
							else
							{
								if (SelisCluster)
								{
									DataAddr4D lll=db->findplace(kSizeSetDisk+kSizeDataBaseObjectHeader, nil, err, -kIndexSegNum, (ObjAlmostInCache*)this);
									if (err == VE_OK)
									{
#if debugIndexOverlap_strong
										di_IndexOverLap::AddClusterPart(nil, -1, i, lll, kSizeSetDisk+kSizeDataBaseObjectHeader);
#endif
										TabPage[i]=lll;
									}
								}
								p[kNbLongParSet] = -1;
								move4block(p2,p,kSizeSetDisk);
								tabmem[i]=p;
								tabplein[i]=kBitPageNonVide;
							}
						}
					}
				}
				else
				{
					b2=autre->tabplein[i];
					if (!b2)
					{
						if ((i == (nbpage-1)) && (nbbit > autre->nbbit))
						{
							xbox_assert(nbbit - autre->nbbit < kNbBitParSet);
							p[kNbLongParSet] = -1;
							sLONG remainbits = (autre->nbbit % kNbBitParSet);
							if (remainbits == 0)
								remainbits = kNbBitParSet;
							sLONG nblong = remainbits / 32;
							for (sLONG k = 0; k < nblong; k++)
							{
								*p = 0;
								++p;
							}
							remainbits = remainbits % 32;
							for (sLONG k = 0; k < remainbits; k++)
							{
								*p = *p & (ALLFF ^ (1<<k));
							}
						}
						else
						{
							FreeFastMem(p);
							tabmem[i]=(bitsetptr)(-2);
							tabplein[i]=false;
						}
					}
				}
			}
		}
	}
	
	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTMODIFYSET, DBaction_ModifyingSet);

	//autre->unuse();
	//unuse();
	return(err);
}


VError Bittab::Or(Bittab *autre, Boolean cangrow)
{
	sLONG i,j;
	bitsetptr p,p2;
	uBOOL b1,b2;
	VError err;
	
	//use();
	//autre->use();
	err=0;
#if debug_bittab_threadsafe
	debugCheckOwner();
#endif
	equisel = -1;
	decompte = -1;
	
	if (err == VE_OK)
	{
		if (nbbit<autre->nbbit && cangrow)
			err = aggrandit(autre->nbbit);
		else
		{
			// on ne doit plus alterer l'autre bitsel L.R le 23 mars 2010
			/*
			if (nbbit > autre->nbbit)
				err = autre->aggrandit(nbbit);
				*/
		}
	}

	sLONG nbpagetogo = nbpage;
	if (nbpagetogo> autre->nbpage)
		nbpagetogo = autre->nbpage;
	
	for (i=0;(i<nbpagetogo) && (err==VE_OK);i++)
	{
		p=loadmem(i, err);
		if (err == VE_OK)
		{
			p2=autre->loadmem(i, err);
		}
		
		if (err == VE_OK)
		{
			if ((p!=nil) && (p2!=nil))
			{
				p[kNbLongParSet] = -1;
				if (i == (autre->nbpage - 1) && nbbit != autre->nbbit)
				{
					sLONG remainbits = (autre->nbbit % kNbBitParSet);
					if (remainbits == 0)
						remainbits = kNbBitParSet;
					sLONG nblong = remainbits / 32;
					for (sLONG k = 0; k < nblong; k++)
					{
						*p = *p | *p2;
						p++;
						p2++;
					}
					remainbits = remainbits % 32;
					for (sLONG k = 0; k < remainbits; k++)
					{
						if ( ((uLONG)(1<<k)) & *p2 )
							*p = *p | (1<<k);
					}
				}
				else
				{
					for (j=0;j<kNbLongParSet;j++)
					{
						*p = *p | *p2;
						p++;
						p2++;
					}
				}
			}
			else
			{
				if (p==nil)
				{
					b1=tabplein[i];
					if (p2==nil)
					{
						b2=autre->tabplein[i];
						tabplein[i]=b1 || b2;
						tabmem[i]=(bitsetptr)(-2);
					}
					else
					{
						if (!b1)
						{
							p=(bitsetptr)GetFastMem(kSizeSetMem, false, 'bsel');
							if (p==nil)
							{
								err=ThrowError(memfull, DBaction_ModifyingSet);
							}
							else
							{
								if (SelisCluster)
								{
									DataAddr4D lll=db->findplace(kSizeSetDisk+kSizeDataBaseObjectHeader, nil, err, -kIndexSegNum, (ObjAlmostInCache*)this);
									if (err == VE_OK)
									{
#if debugIndexOverlap_strong
										di_IndexOverLap::AddClusterPart(nil, -1, i, lll, kSizeSetDisk+kSizeDataBaseObjectHeader);
#endif
										TabPage[i]=lll;
									}
								}

								if (i == (autre->nbpage - 1) && nbbit != autre->nbbit)
								{
									bitsetptr endp = p+kNbLongParSet, p3 = p;
									sLONG remainbits = (autre->nbbit % kNbBitParSet);
									if (remainbits == 0)
										remainbits = kNbBitParSet;
									sLONG nblong = remainbits / 32;
									for (sLONG k = 0; k < nblong; k++)
									{
										*p3 = *p2;
										p3++;
										p2++;
									}
									remainbits = remainbits % 32;
									*p3 = 0;
									for (sLONG k = 0; k < remainbits; k++)
									{
										if ( ((uLONG)(1<<k)) & *p2 )
											*p3 = *p3 | (1<<k);
									}
									if (endp > (p3 + 1))
									{
										std::fill(p3+1, endp, 0);
									}
								}
								else
								{
									move4block(p2,p,kSizeSetDisk);
								}
								p[kNbLongParSet] = -1;
								tabmem[i]=p;
								tabplein[i]=kBitPageNonVide;
							}
						}
					}
				}
				else
				{
					b2=autre->tabplein[i];
					if (b2)
					{
						if ((i == (nbpage-1)) && (nbbit > autre->nbbit))
						{
							xbox_assert(nbbit - autre->nbbit < kNbBitParSet);
							p[kNbLongParSet] = -1;
							sLONG remainbits = (autre->nbbit % kNbBitParSet);
							if (remainbits == 0)
								remainbits = kNbBitParSet;
							sLONG nblong = remainbits / 32;
							for (sLONG k = 0; k < nblong; k++)
							{
								*p = ALLFF;
								++p;
							}
							remainbits = remainbits % 32;
							for (sLONG k = 0; k < remainbits; k++)
							{
								*p = *p | (1<<k);
							}
						}
						else
						{
							FreeFastMem(p);
							tabmem[i]=(bitsetptr)(-2);
							tabplein[i]=kBitPageAllTrue;
						}
					}
				}
			}
		}
	}
	
	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTMODIFYSET, DBaction_ModifyingSet);
	
	//autre->unuse();
	//unuse();
	return(err);
}


VError Bittab::moins(Bittab *autre, Boolean cangrow)
{
	sLONG i,j;
	bitsetptr p,p2;
	uBOOL b1,b2;
	sLONG nb2;
	VError err = VE_OK;
	
	//use();
	//autre->use();
	err=0;
#if debug_bittab_threadsafe
	debugCheckOwner();
#endif
	equisel = -1;
	decompte = -1;
	
	/*
	if (err == VE_OK)
	{
		if (nbbit<autre->nbbit && cangrow) 
			aggrandit(autre->nbbit);
	}
	*/

	// on ne doit plus alterer l'autre bitsel L.R le 23 mars 2010
	/*
	if (nbbit > autre->nbbit)
		err = autre->aggrandit(nbbit);
		*/

	nb2 = minl(nbpage, autre->nbpage);

	for (i=0;(i<nb2) && (err==VE_OK);i++)
	{
		p=loadmem(i, err);
		if (err == VE_OK)
		{
			p2=autre->loadmem(i, err);
		}
		
		if (err == VE_OK)
		{		
			uLONG tempbits[kNbLongParSet+1];
			if (p2==nil)
			{
				b2=autre->tabplein[i];
				if (b2 && (nbbit > autre->nbbit) && (i == (autre->nbpage-1)) )
				{
					p2 = &tempbits[0];
					std::fill(&tempbits[0], &tempbits[kNbLongParSet], 0);
					bitsetptr p3 = p2;

					sLONG remainbits = (autre->nbbit % kNbBitParSet);
					if (remainbits == 0)
						remainbits = kNbBitParSet;
					sLONG nblong = remainbits / 32;
					for (sLONG k = 0; k < nblong; k++)
					{
						*p3 = ALLFF;
						++p3;
					}
					remainbits = remainbits % 32;
					for (sLONG k = 0; k < remainbits; k++)
					{
						*p3 = *p3 | (1<<k);
					}
				}
			}

			if ((p!=nil) && (p2!=nil))
			{
				p[kNbLongParSet] = -1;
				if (i == (autre->nbpage - 1) && nbbit != autre->nbbit)
				{
					sLONG remainbits = (autre->nbbit % kNbBitParSet);
					if (remainbits == 0)
						remainbits = kNbBitParSet;
					sLONG nblong = remainbits / 32;
					for (sLONG k = 0; k < nblong; k++)
					{
						*p = *p & ((uLONG)(0xFFFFFFFF) ^ *p2);
						p++;
						p2++;
					}
					remainbits = remainbits % 32;
					for (sLONG k = 0; k < remainbits; k++)
					{
						if ( ((uLONG)(1<<k)) & *p2 )
							*p = *p & (ALLFF ^ (1<<k));
					}
				}
				else
				{
					for (j=0;j<kNbLongParSet;j++)
					{
						*p = *p & ((uLONG)(0xFFFFFFFF) ^ *p2);
						p++;
						p2++;
					}
				}
			}
			else
			{

				if (p==nil)
				{
					b1=tabplein[i];
					if (p2==nil)
					{
						b2=autre->tabplein[i];
						tabplein[i]=b1 && (!b2);
						tabmem[i]=(bitsetptr)(-2);
					}
					else
					{
						if (b1)
						{
							p=(bitsetptr)GetFastMem(kSizeSetMem, false, 'bsel');
							if (p==nil)
							{
								err=ThrowError(memfull, DBaction_ModifyingSet);
							}
							else
							{
								if (SelisCluster)
								{
									DataAddr4D lll=db->findplace(kSizeSetDisk+kSizeDataBaseObjectHeader, nil, err, -kIndexSegNum, (ObjAlmostInCache*)this);
									if (err == VE_OK)
									{
#if debugIndexOverlap_strong
										di_IndexOverLap::AddClusterPart(nil, -1, i, lll, kSizeSetDisk+kSizeDataBaseObjectHeader);
#endif
										TabPage[i]=lll;
									}
								}
								p[kNbLongParSet] = -1;
								bitsetptr p3 = p;
								for (j=0;j<kNbLongParSet;j++)
								{
									*p3 = (uLONG)(0xFFFFFFFF) ^ *p2;
									p3++;
									p2++;
								}
								tabmem[i]=p;
								tabplein[i]=kBitPageNonVide;
							}
						}
					}
				}
				else
				{
					b2=autre->tabplein[i];
					if (b2)
					{
						if ((i == (nbpage-1)) && (nbbit > autre->nbbit))
						{
							xbox_assert(nbbit - autre->nbbit < kNbBitParSet);
							p[kNbLongParSet] = -1;
							sLONG remainbits = (autre->nbbit % kNbBitParSet);
							if (remainbits == 0)
								remainbits = kNbBitParSet;
							sLONG nblong = remainbits / 32;
							for (sLONG k = 0; k < nblong; k++)
							{
								*p = 0;
								++p;
							}
							remainbits = remainbits % 32;
							for (sLONG k = 0; k < remainbits; k++)
							{
								*p = *p & (ALLFF ^ (1<<k));
							}
						}
						else
						{
							FreeFastMem(p);
							tabmem[i]=(bitsetptr)(-2);
							tabplein[i]=false;
			
						}
					}
				}
			}
		}
	}
	
	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTMODIFYSET, DBaction_ModifyingSet);
	
	//autre->unuse();
	//unuse();
	return(err);
}


VError Bittab::Invert(void)
{
	sLONG i,j;
	bitsetptr p;
	VError err;
	
	//use();
	err=0;
#if debug_bittab_threadsafe
	debugCheckOwner();
#endif
	equisel = -1;
	decompte = -1;

	for (i=0;(i<nbpage) && (err==VE_OK);i++)
	{
		p=loadmem(i, err);

		if (err == VE_OK)
		{
			if (p==nil)
			{
				tabplein[i] = ! tabplein[i];
				tabmem[i] = (bitsetptr)(-2);
			}
			else
			{
				for (j=0;j<kNbLongParSet;j++)
				{
					*p = (uLONG)(0xFFFFFFFF) ^ *p;
					p++;
				}
			}
		}
	}
	
	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTMODIFYSET, DBaction_ModifyingSet);
	
	//unuse();
	return(err);
}


sLONG Bittab::FindNextBit(sLONG start)
{
	sLONG curpage,stb,endb,nbit;
	sLONG res;
	bitsetptr p;
	uLONG u;
	VError err;
	
#if debug_bittab_threadsafe
	debugUse();
#endif
	curpage = start >> kRatioSet;
	res=-1;
	
	if (testAssert(start >= 0))
	{
		if (start<nbbit)
			p=loadmem(curpage, err);
		else
			p = nil;
		
		while ((start<nbbit) && (res==-1))
		{
			if (p==nil)
			{
				if (tabplein[curpage])
				{
					res=start;
				}
				else
				{
					start=(start+kNbBitParSet) & (-kNbBitParSet);
					curpage++;
					if (start<nbbit) // L.E. 05/01/01 sinon crash avec db == nil
						p=loadmem(curpage, err);
				}
			}
			else
			{
				stb=start & (kNbBitParSet-1);
				p=p+(stb>>5);
				if (curpage==(nbpage-1))
				{
					endb=(nbbit-1) & (kNbBitParSet-1);
				}
				else
				{
					endb=kNbBitParSet-1;
				}
				nbit=stb & 31;
				u=*p;
				while ((stb<=endb) && (res==-1))
				{
					if (nbit==0)
					{
						while ((u==0) && (stb<=endb))
						{
							p++;
							u=*p;
							stb=stb+32;
							start=start+32;
						}
						if (stb<=endb)
						{
							if (start>=nbbit)
							{
								start=nbbit-1;
							}
							else
							{
								if ( ((uLONG)(1<<nbit)) & u )
								{
									res=start;
								}
							}
						}
						
					}
					else
					{
						if ( ((uLONG)(1<<nbit)) & u )
						{
							res=start;
						}
					}

					if (stb<=endb)
					{
						nbit++;
						stb++;
						start++;
						if (nbit==32)
						{
							nbit=0;
							p++;
							u=*p;
						}
					}
				} // du  while ((stb<=endb) && (res==-1))
				
				if ((start<nbbit) && (res==-1))
				{
					curpage++;
					if (curpage<nbpage) p=loadmem(curpage, err);
				}
			}
		} // du  while ((start<nbbit) && (res==-1))
	}
#if debug_bittab_threadsafe
	debugUnuse();
#endif

	return(res);
}


sLONG Bittab::FindPreviousBit(sLONG start)
{
	sLONG curpage,stb,endb,nbit;
	sLONG res;
	bitsetptr p;
	uLONG u;
	VError err;

#if debug_bittab_threadsafe
	debugUse();
#endif

	curpage=start>>kRatioSet;
	res=-1;

	if (start >= nbbit)
		start = nbbit-1;
	if (start >= 0)
	{
		p=loadmem(curpage, err);

		while ((start>=0) && (res==-1))
		{
			if (p==nil)
			{
				if (tabplein[curpage])
				{
					res=start;
				}
				else
				{
					curpage--;
					//start = start - kNbBitParSet;
					start = curpage << kRatioSet;
					start = start + kNbBitParSet - 1; 
					if (curpage>=0)
						p=loadmem(curpage, err);
				}
			}
			else
			{
				stb=start & (kNbBitParSet-1);
				p=p+(stb>>5);
				nbit=stb & 31;
				u=*p;
				while ((stb>=0) && (res==-1))
				{
					if (nbit==31)
					{
						while ((u==0) && (stb>=0))
						{
							p--;
							u=*p;
							stb=stb-32;
							start=start-32;
						}
						if (stb>=0)
						{
							if ( ((uLONG)(1<<nbit)) & u )
							{
								res=start;
							}
						}

					}
					else
					{
						if ( ((uLONG)(1<<nbit)) & u )
						{
							res=start;
						}
					}

					if (stb>=0)
					{
						nbit--;
						stb--;
						start--;
						if (nbit==-1)
						{
							nbit=31;
							p--;
							u=*p;
						}
					}
				} 

				if ((start>=0) && (res==-1))
				{
					curpage--;
					if (curpage>=0) 
						p=loadmem(curpage, err);
				}
			}
		} // du  while ((start<nbbit) && (res==-1))
	}
#if debug_bittab_threadsafe
	debugUnuse();
#endif

	return(res);
}


void Bittab::CalcEquiSel()
{
	Boolean cont;
	sLONG j;
	sLONG i;
	bitsetptr p;
	VError err = VE_OK;

	if (equisel==-1)
	{
		cont=true;
		equisel=0;
		for (j=0;(j<nbpage) && (cont) && (err == VE_OK);j++)
		{
			p=loadmem(j, err);
			if (p==nil)
			{
				if (tabplein[j])
				{
					if (j==(nbpage-1))
					{
						equisel=nbbit;
					}
					else
					{
						equisel=(j+1)<<kRatioSet;
					}
				}
				else
				{
					cont=false;
				}
			}
			else
			{
				for (i=0;(i<kNbLongParSet) && (cont);i++)
				{
					if (*p++!=-1) cont=false; else equisel=equisel+32;
				}
			}
		}
	}

}


Bittab* Bittab::GenerateSetFromRange(sLONG inRecordIndex1, sLONG inRecordIndex2, VError& err)
{
	sLONG inRecordID = PlaceDuNiemeBitAllume(inRecordIndex1);
	return GenerateSetFromRecordID(inRecordID, inRecordIndex2, err);
}


Bittab* Bittab::GenerateSetFromRecordID(sLONG inRecordID, sLONG inRecordIndex2, VError& err)
{
	err = VE_OK;

	sLONG inRecordID2 = PlaceDuNiemeBitAllume(inRecordIndex2);

	if (inRecordID2 < 0)
		inRecordID2 = 0;
	if (inRecordID2 > nbbit)
		inRecordID2 = nbbit - 1;

	if (inRecordID < 0)
		inRecordID = 0;
	if (inRecordID > nbbit)
		inRecordID = nbbit - 1;

	if (inRecordID>inRecordID2)
	{
		sLONG temp = inRecordID;
		inRecordID = inRecordID2;
		inRecordID2 = temp;
	}

	sLONG numpage = inRecordID >> kRatioSet;
	sLONG numbit = inRecordID & (kNbBitParSet-1);
	sLONG numlong = numbit >> 5;
	numbit = numbit & 0x0000001F;

	sLONG numpageEnd = inRecordID2 >> kRatioSet;
	sLONG numbitEnd = inRecordID2 & (kNbBitParSet-1);
	sLONG numlongEnd = numbitEnd >> 5;
	numbitEnd = numbitEnd & 0x0000001F;

	Bittab* res = new Bittab;
	if (res != nil)
	{
		err = res->aggrandit(nbbit);
		if (err == VE_OK && nbbit > 0)
		{
			sLONG npage;

			for (npage = numpage; npage <= numpageEnd && err == VE_OK; npage++)
			{
				if (npage == numpage || npage == numpageEnd)
				{
					if (tabplein[npage] != 0)
					{
						bitsetptr p2 = nil;
						bitsetptr p = loadmem(npage, err);
						if (err == VE_OK)
						{
							p2 = (bitsetptr)GetFastMem(kSizeSetMem, false, 'bsel');
							if (p2 == nil)
								err = ThrowError(memfull, DBaction_ModifyingSet);
							else
							{
								_raz((void*) p2, kSizeSetMem);
								res->tabplein[npage] = kBitPageNonVide;
								res->tabmem[npage] = p2;

								bitsetptr p3;
								bitsetptr p4;

								if (npage == numpage)
								{
									Boolean oktouchenumbit = true;
									if (npage == numpageEnd)
									{
										if (numlong == numlongEnd)
										{
											oktouchenumbit = false;

											p3 = p + numlong;
											uLONG u = 0xFFFFFFFF >> (31-numbitEnd);
											uLONG u2;
											if (numbit == 0)
												u2 = 0;
											else
												u2 = 0xFFFFFFFF >> (31-numbit+1);
											u = u ^ u2;
											if (p != nil)
												u = u & *p3;
											p4 = p2 + numlong;
											*p4 = u;
											numlong++;
										}
										else
										{
											if (numbitEnd != 31)
											{
												p3 = p + numlongEnd, p4 = p2 + numlongEnd;
												uLONG u = 0xFFFFFFFF >> (31-numbitEnd);
												if (p != nil)
													u = u & *p3;
												*p4 = u;
												numlongEnd--;
											}
										}
									}
									
									if (numbit != 0 && oktouchenumbit)
									{
										p3 = p + numlong, p4 = p2 + numlong;
										uLONG u = 0xFFFFFFFF >> (31-numbit+1);
										u = 0xFFFFFFFF ^ u;
										if (p != nil)
											u = u & *p3;
										*p4 = u;
										numlong++;
									}
									
								}
								else
								{
									if (npage == numpageEnd)
									{
										if (numbitEnd != 31)
										{
											p3 = p + numlongEnd, p4 = p2 + numlongEnd;
											uLONG u = 0xFFFFFFFF >> (31-numbitEnd);
											if (p != nil)
												u = u & *p3;
											*p4 = u;
											numlongEnd--;
										}
									}
								}

								sLONG start, end, nlong;
								if (npage == numpage)
									start = numlong;
								else
									start = 0;

								if (npage == numpageEnd)
									end = numlongEnd;
								else
									end = kNbLongParSet-1;

								if (p == nil)
								{
									p4 = p2 + start;
									for (nlong = start; nlong <= end; nlong++)
									{
										(*p4++) = 0xFFFFFFFF;
									}
								}
								else
								{
									p3 = p + start;
									p4 = p2 + start;
									for (nlong = start; nlong <= end; nlong++)
									{
										(*p4++) = *p3++;
									}
								}

							}
						}
					}
				}
				else
				{
					res->tabplein[npage] = tabplein[npage];
					if (tabplein[npage] == kBitPageNonVide)
					{
						bitsetptr p2 = nil;
						bitsetptr p = loadmem(npage, err);
						if (err == VE_OK)
						{
							if (testAssert(p != nil))
							{
								p2 = (bitsetptr)GetFastMem(kSizeSetMem, false, 'bsel');
								if (p2 == nil)
								{
									err = ThrowError(memfull, DBaction_ModifyingSet);
								}
								move4block((void*)p, (void*)p2, kSizeSetDisk);
								p2[kNbLongParSet] = -1;
							}

						}
						if (err != VE_OK && p2 != nil)
						{
							FreeFastMem((void*)p2);
							p2 = (bitsetptr)-2;
							res->tabplein[npage] = 0;
						}
						res->tabmem[npage] = p2;
					}
					else
						res->tabmem[npage] = (bitsetptr)-2;
				}

			}
		}
		//res->libere();
	}
	else
		err = ThrowError(memfull, DBaction_ModifyingSet);

	if (err != VE_OK && res != nil)
	{
		delete res;
		res = nil;
	}

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTMODIFYSET, DBaction_ModifyingSet);

	return res;
}


sLONG Bittab::CountHowManyBitAllumeAt(sLONG pos)
{
	sLONG res = 0;
	VError err = VE_OK;

	//use();
	if (pos<nbbit)
	{
		CalcEquiSel();
		if (pos<equisel)
		{
			res = pos;
		}
		else
		{
			res = equisel-1;
			sLONG numpage = equisel >> kRatioSet;
			sLONG numbit = equisel & (kNbBitParSet-1);
			sLONG numlong = numbit >> 5;
			numbit = numbit & 0x0000001F;

			sLONG numpageEnd = pos >> kRatioSet;
			sLONG numbitEnd = pos & (kNbBitParSet-1);
			sLONG numlongEnd = numbitEnd >> 5;
			numbitEnd = numbitEnd & 0x0000001F;

			sLONG i, start, end, j;

			for (i=numpage; i<=numpageEnd && err == VE_OK; i++)
			{
				if (i == numpage)
					start = numlong;
				else
					start = 0;

				if (i == numpageEnd)
					end = numlongEnd;
				else
				{
					end = kNbLongParSet-1;
				}

				if (tabplein[i] != kBitPageNonVide)
				{
					if (tabplein[i])
					{
						res = res + (end-start+1)*32;
						if (i == numpageEnd)
							res = res + numbitEnd;
					}
				}
				else
				{
					bitsetptr p = loadmem(i, err);
					if (testAssert(p != nil))
					{
						if (i == numpage)
							p = p + numlong;
						for (j = start; j <= end; j++, p++)
						{
							uLONG u = *p;
							sLONG startbit,endbit;

							if (u == 0xffffffff)
							{
								if (j == start)
								{
									if (j == end)
										res = res + numbitEnd - numbit + 1;
									else
										res = res + 32 - numbit;
								}
								else
								{
									if (j == end)
									{
										res = res + numbitEnd + 1;
									}
									else
										res = res + 32;
								}
							}
							else
							{
								if (u == 0)
								{
								}
								else
								{
									if (j == start)
										startbit = numbit;
									else
										startbit = 0;

									if (j == end)
										endbit = numbitEnd;
									else
										endbit = 31;

									sLONG k;
									for (k = startbit; k <= endbit; k++)
									{
										if (((uLONG)(1<<k)) & u) 
											res++;
									}
								}
							}
						}
					}
					else
					{
						res = -1;
					}
				}
			}

		}
	}
	else
		res = -1;

	//unuse();

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOT_COMPUTE_SET, DBaction_ComputingSet);

	return res;
}


sLONG Bittab::PlaceDuNiemeBitAllume(sLONG place)
{
	sLONG i,j,res,n,n1,xn2;
	bitsetptr p;
	uBOOL cont;
	uLONG u;
	VError err = VE_OK;

#if debug_bittab_threadsafe
	debugUse();
#endif

	//use();
	CalcEquiSel();
	if (decompte == -1)
		Compte();
	if (place >= decompte)
		res = -1;
	else if (place<equisel)
	{
		res=place;
	}
	else
	{
		res=-1;
		place=place-equisel;
		// n1=(equisel & (-kNbBitParSet)) & (-32);  -->  L.E
		n1=(equisel & (kNbBitParSet-1)) & (-32);
		
		
		for (j=equisel>>kRatioSet;(place>=0) && (j<nbpage);j++)
		{
			if (j==(nbpage-1)) n=((nbbit-1) & (kNbBitParSet-1))+1; else n=kNbBitParSet;
			p=loadmem(j, err);
			if (tabplein[j] != kBitPageNonVide)
			{
				if (tabplein[j]) // L.E. 10/12/1999 was ((*tabplein[j]))
				{
					if (place<n)
					{
						res=(j<<kRatioSet)+place;
					}
					place=place-n;
				}
			}
			else
			{
				p=p+(n1>>5);
				u=*p;
				xn2=0;
				// i=0;  --> L.E
				i=n1;
				while ( (place>=0) && (i<n) )
				{
					if ((i & 31) == 0)
					{
						if (u==0)
						{
							p++;
							u=*p;
							i=i+32;
						}
						else
						{
							if (u==0xFFFFFFFF)
							{
								if (place<32)
								{
									res=(j<<kRatioSet)+i+place;
								}
								p++;
								u=*p;
								i=i+32;
								place=place-32;
							}
							else
							{
								if ( ((uLONG)(1<<xn2)) & u )
								{
									place--;
									if (place<0)
									{
										res=(j<<kRatioSet)+i;
									}
								}
								i++;
								xn2++;
								if (xn2==32)
								{
									p++;
									u=*p;
									xn2=0;
								}
							}
						}
					}
					else
					{
						if ( ((uLONG)(1<<xn2)) & u )
						{
							place--;
							if (place<0)
							{
								res=(j<<kRatioSet)+i;
							}
						}
						i++;
						xn2++;
						if (xn2==32)
						{
							p++;
							u=*p;
							xn2=0;
						}
					}
				} // du while
			}
			n1=0;
		} // du for j
	}
#if debug_bittab_threadsafe
	debugUnuse();
#endif

	//unuse();
	return(res);
}


Boolean Bittab::IsEmpty()
{
	sLONG k,j,i,n,xn2,n3;
	uLONG u;
	bitsetptr p;
	VError err = VE_OK;
	
	if (isempty != 2) 
	{
		return isempty;
	}
	else
	{
		isempty = true;
		for (i=0;i<nbpage;i++)
		{
			p=loadmem(i, err);
			if (err == VE_OK)
			{
				if (p==nil)
				{
					if (tabplein[i])
					{
						isempty = false;
						return isempty;
					}
				}
				else
				{
					if (i==(nbpage-1)) 
						n=((nbbit-1) & (kNbBitParSet-1))+1; 
					else 
						n=kNbBitParSet;

					xn2=n>>5;
					for (j=0;j<xn2;j++)
					{
						u=*p++;
						if (u!=0)
						{
							isempty = false;
							return isempty;
						}
					}
					
					n3=n & 31;
					u=*p;
					for (k=0;k<n3;k++)
					{
						if ( ((uLONG)(1<<k)) & u)
						{ 
							isempty = false;
							return isempty;
						}
					}

				}
			}
			else
			{
				isempty = false;
				return isempty;
			}
		}
		
		return isempty;
	}
}


sLONG Bittab::Compte(void)
{
	sLONG k,j,i,n,res,xn2,n3;
	uLONG u;
	bitsetptr p;
	VError err = VE_OK;
	
	//use();
	if (decompte==-1)
	{
		if (fNeedPurgeCount > 5000 && fAutoPurge && !SelisCluster)
			Epure();

		res=0;
		for (i=0;i<nbpage;i++)
		{
			p=loadmem(i, err);
			if (err == VE_OK)
			{
				if (i==(nbpage-1)) 
					n=((nbbit-1) & (kNbBitParSet-1))+1; 
				else 
					n=kNbBitParSet;
					
				if (p==nil)
				{
					if (tabplein[i])
					{
						res=res+n;
					}
				}
				else
				{
					xn2=n>>5;
					for (j=0;j<xn2;j++)
					{
						u=*p++;
						if (u!=0)
						{
							if (u==-1)
							{
								res=res+32;
							}
							else
							{
								for (k=0;k<32;k++)
								{
									if ( ((uLONG)(1<<k)) & u) res++;
								}
							}
						}
					}
					n3=n & 31;
					u=*p;
					for (k=0;k<n3;k++)
					{
						if ( ((uLONG)(1<<k)) & u) res++;
					}
				}
			}
			else
				break;
		}
		decompte=res;
	}
	else
	{
		res=decompte;
	}

	//unuse();
	return(res);
}

void Bittab::Epure(void)
{
	bitsetptr p,p2;
	sLONG i,j,k,xn2,n;
	uLONG u;
	uBOOL okepure;
	VError err = VE_OK;
	
	//use();

	for (i=0;i<(nbpage-1) && err == VE_OK;i++)
	{
		okepure=false;
		p=loadmem(i, err);
		if (err == VE_OK)
		{
			if (p!=nil)
			{
				if (i==(nbpage-1)) n=((nbbit-1) & (kNbBitParSet-1))+1; else n=kNbBitParSet;
				u=*p;
				if (u==0)
				{
					p2 = p; // L.E. 26/01/07 ACI0048489
					okepure = true;
					xn2=n>>5;
					for (j=0;j<xn2;j++)
					{
						u=*p2++;
						if (u!=0)
						{
							okepure = false;
							break;
						}
					}
					xn2=n & 31;
					u=*p2;
					for (k=0;k<xn2;k++)
					{
						if ( ((uLONG)(1<<k)) & u) okepure = false;
					}
					
					if (okepure)
					{
						// equisel = -1;
						FreeFastMem(p); // L.E. 26/01/07 ACI0048489
						tabmem[i]=(bitsetptr)(-2);
						tabplein[i]=false;
					}
				}
				else
				{
					if (u == -1)
					{
						okepure = true;
						xn2=n>>5;
						p2 = p;
						for (j=0;j<xn2;j++)
						{
							u=*p2++;
							if (u!=0xFFFFFFFF)
							{
								okepure = false;
								break;
							}
						}
						xn2=n & 31;
						u=*p2;
						for (k=0;k<xn2;k++)
						{
							if ( !(((uLONG)(1<<k)) & u) ) okepure = false;
						}
						
						if (okepure)
						{
							// equisel = -1;
							FreeFastMem(p);
							tabmem[i]=(bitsetptr)(-2);
							tabplein[i]=kBitPageAllTrue;
						}
					}
				}
			}
		}
	}

	fNeedPurgeCount = 0;

	//unuse(); // L.E. 10/12/1999 added
}


VError Bittab::LibereEspaceDisk(OccupableStack* curstack, VDB4DProgressIndicator* InProgress)
{
	sLONG i,len;
	DataAddr4D ou;
	VError err = VE_OK;
	
	use();
	if (SelisCluster)
	{
		for (i=0;i<nbpage;i++)
		{
			ou=TabPage[i];
			if (ou>2)
			{
#if debugIndexOverlap_strong
				di_IndexOverLap::RemoveClusterPart(nil, -1, i, ou, kSizeSetDisk+kSizeDataBaseObjectHeader);
#endif
				db->libereplace(ou, kSizeSetDisk+kSizeDataBaseObjectHeader, nil, (ObjAlmostInCache*)this);
				err = PullLastError();
			}
		}
		
		ou = SelAddr;
		if (ou>0)
		{
			len = CalcLenOnDisk();
#if debugIndexOverlap_strong
			di_IndexOverLap::RemoveCluster(nil, -1, ou, len+kSizeDataBaseObjectHeader);
#endif
			db->libereplace(ou, len+kSizeDataBaseObjectHeader, nil, (ObjAlmostInCache*)this);
			err = PullLastError();
		}
	}
	unuse();
	
	if (err != VE_OK)
		err = ThrowError(err, DBaction_DropingSet);
		
	return err;
}


VError Bittab::FillFromArray(const char* tab, sLONG sizeElem, sLONG nbelem, sLONG maxrecords, bool useIndirection)
{
	VError err = VE_OK;
	const char* p = tab + (nbelem*sizeElem);

	while (nbelem>0 && err == VE_OK)
	{
		p = p - sizeElem;
		sLONG n;
		if (useIndirection)
			n = **((sLONG**)p);
		else
			n = *((sLONG*)p);
		if (n>=0 && n<maxrecords)
		{
			err = ClearOrSet(n, true, true);
		}
		else
			err = ThrowBaseError(VE_DB4D_WRONGRECORDID);

		nbelem--;
	}

	return err;
}


VError Bittab::PutIntoLongSel(LongSel* sel)
{
	VError err;
	sLONG k,j,i,n,xn2,n3,cur;
	uLONG u;
	bitsetptr p;
	
	
	err = VE_OK;
	cur = 0;

	sLONG curpage = 0;
	sLONG* tab = sel->loadMem(curpage, true);
	if (tab != nil)
		sel->ModifMem(tab);
	sLONG* tab2 = tab;
	sLONG maxlongselpage = (sel->GetQTfic()+kNbElemInSel-1) / kNbElemInSel;
	sLONG curelem = 0;
	{
		for (i=0;(i<nbpage) && (err == VE_OK);i++)
		{
			p=loadmem(i, err);
			
			if (err == VE_OK)
			{
				if (i==(nbpage-1)) n=((nbbit-1) & (kNbBitParSet-1))+1; else n=kNbBitParSet;
				if (p==nil)
				{
					if (tabplein[i])
					{
						for (j=0;j<n;j++)
						{
							*tab2++ = cur;
							curelem++;
							if (curelem >= kNbElemInSel)
							{
								sel->ReleaseMem(tab);
								curpage++;
								if (curpage < maxlongselpage)
								{
									tab = sel->loadMem(curpage, true);
									if (tab != nil)
										sel->ModifMem(tab);
								}
								else
									tab = nil;
								tab2 = tab;
								if (tab == nil)
									break;
								curelem = 0;
							}
							++cur;
						}
					}
					else
					{
						cur = cur + n;
					}
				}
				else
				{
					xn2=n>>5;
					for (j=0;j<xn2;j++)
					{
						u=*p++;
						if (u!=0)
						{
							if (u==-1)
							{
								for (k=0;k<32;k++)
								{
									*tab2++ = cur;
									++cur;
									curelem++;
									if (curelem >= kNbElemInSel)
									{
										sel->ReleaseMem(tab);
										curpage++;
										if (curpage < maxlongselpage)
										{
											tab = sel->loadMem(curpage, true);
											if (tab != nil)
												sel->ModifMem(tab);
										}
										else
											tab = nil;
										tab2 = tab;
										curelem = 0;
										if (tab == nil)
											break;
									}
								}
							}
							else
							{
								for (k=0;k<32;k++)
								{
									if ( ((uLONG)(1<<k)) & u)
									{
										*tab2++ = cur;
										curelem++;
										if (curelem >= kNbElemInSel)
										{
											sel->ReleaseMem(tab);
											curpage++;
											if (curpage < maxlongselpage)
											{
												tab = sel->loadMem(curpage, true);
												if (tab != nil)
													sel->ModifMem(tab);
											}
											else
												tab = nil;
											tab2 = tab;
											curelem = 0;
											if (tab == nil)
												break;
										}
									}
									++cur;
		
								}
							}
						}
						else
						{
							cur = cur + 32;
						}
						if (tab == nil)
							break;
					}
					n3=n & 31;
					u=*p;
					for (k=0;k<n3;k++)
					{
						if ( ((uLONG)(1<<k)) & u)
						{
							*tab2++ = cur;
							curelem++;
							if (curelem >= kNbElemInSel)
							{
								sel->ReleaseMem(tab);
								curpage++;
								if (curpage < maxlongselpage)
								{
									tab = sel->loadMem(curpage, true);
									if (tab != nil)
										sel->ModifMem(tab);
								}
								else
									tab = nil;
								tab2 = tab;
								curelem = 0;
								if (tab == nil)
									break;
							}
						}
						++cur;
					}
				}
			}
			if (tab == nil)
				break;
		}
		
		if (tab != nil)
			sel->ReleaseMem(tab);

//		xbox_assert(sel->qtfic == ( (curpage * kNbElemInSel) + curelem ));

		if (err != VE_OK)
			err = ThrowError(VE_DB4D_CANNOTMODIFYSET, DBaction_ModifyingSet);
	}
	
	
	return(err);

}


VError Bittab::FillArray(sLONG* tab, sLONG maxelem)
{
	VError err;
	sLONG k,j,i,n,xn2,n3,cur;
	uLONG u;
	bitsetptr p;
	
	
	err = VE_OK;
	cur = 0;

	if (maxelem < Compte())
	{
		err = ThrowError(VE_DB4D_ARRAYLIMIT_IS_EXCEEDED, DBaction_ModifyingSet);
	}
	else
	{
		//use();
		for (i=0;(i<nbpage) && (err == VE_OK);i++)
		{
			p=loadmem(i, err);
			
			if (err == VE_OK)
			{
				if (i==(nbpage-1)) n=((nbbit-1) & (kNbBitParSet-1))+1; else n=kNbBitParSet;
				if (p==nil)
				{
					if (tabplein[i])
					{
						for (j=0;j<n;j++)
						{
							*tab++ = cur;
							++cur;
						}
					}
					else
					{
						cur = cur + n;
					}
				}
				else
				{
					xn2=n>>5;
					for (j=0;j<xn2;j++)
					{
						u=*p++;
						if (u!=0)
						{
							if (u==-1)
							{
								for (k=0;k<32;k++)
								{
									*tab++ = cur;
									++cur;
								}
							}
							else
							{
								for (k=0;k<32;k++)
								{
									if ( ((uLONG)(1<<k)) & u) *tab++ = cur;
									++cur;
		
								}
							}
						}
						else
						{
							cur = cur + 32;
						}
					}
					n3=n & 31;
					u=*p;
					for (k=0;k<n3;k++)
					{
						if ( ((uLONG)(1<<k)) & u) *tab++ = cur;
						++cur;
					}
				}
			}
		}

		if (err != VE_OK)
			err = ThrowError(VE_DB4D_CANNOTMODIFYSET, DBaction_ModifyingSet);

		//unuse();
	}
	
	
	return(err);
}


VError Bittab::FillArrayOfBits(void* outArray, sLONG inMaxElements)
{
	VError err;
	sLONG k,j,i,n,xn2,n3,cur;
	uLONG u;
	bitsetptr p;
	uCHAR* out = (uCHAR*)outArray;
	sLONG curbit;

	err = VE_OK;
	cur = 0;

	sLONG xnbbit = nbbit;
	if (xnbbit > inMaxElements)
		xnbbit = inMaxElements;
	sLONG xnbpage = (xnbbit+kNbBitParSet-1)>>kRatioSet;

	//use();
	
	// L.E. 15/05/09 first raz extra bytes in output buffer
	if (inMaxElements > nbbit)
	{
		sLONG firstOffset = (nbbit > 0) ? nbbit / 8 : 0;
		sLONG lastOffset = (inMaxElements > 0) ? (inMaxElements - 1) / 8 : 0;
		::memset( (char*) outArray + firstOffset, 0, (lastOffset - firstOffset) + 1);
	}

	for (i=0;(i<xnbpage) && (err == VE_OK);i++)
	{
		xbox_assert(((out - (uCHAR*)outArray) & 4) == 0);
		p=loadmem(i, err);

		if (err == VE_OK)
		{
			if (i==(xnbpage-1)) n=((xnbbit-1) & (kNbBitParSet-1))+1; else n=kNbBitParSet;
			if (p==nil)
			{
				sLONG nblongs = n / 32;
				sLONG nboctets_restants = (n / 8) & 3;	// L.E. 22/03/06 was n & 4
				sLONG nbbits_restants = n & 7;	// L.E. 22/03/06 was n & 8
				if (tabplein[i])
				{
					for (j=0;j<nblongs;j++)
					{
						*((sLONG*)out) = -1;
						out = out + 4;
					}
					for (j=0;j<nboctets_restants;j++)
					{
						*out = 255;
						out++;
					}
					for (j=0;j<nbbits_restants;j++)
					{
						*out = *out | (1 << j);
					}
				}
				else
				{
					for (j=0;j<nblongs;j++)
					{
						*((sLONG*)out) = 0;
						out = out + 4;
					}
					for (j=0;j<nboctets_restants;j++)
					{
						*out = 0;
						out++;
					}
					for (j=0;j<nbbits_restants;j++)
					{
						*out = *out & (255 ^ (1<<j));
					}
				}
			}
			else
			{
				xn2=n>>5;
				for (j=0;j<xn2;j++)
				{
					u=*p++;
					if (u!=0)
					{
						if (u==-1)
						{
							for (k=0;k<4;k++)
							{
								*out = 255;
								out++;
							}
						}
						else
						{
							curbit = 0;
							for (k=0; k<32; k++)
							{
								if ( ((uLONG)(1<<k)) & u)
									*out = *out | (1 << curbit);
								else
									*out = *out & (255 ^ (1 << curbit));
								++curbit;
								if (curbit == 8)
								{
									curbit = 0;
									out++;
								}
							}
						}
					}
					else
					{
						for (k=0; k<4; k++)
						{
							*out = 0;
							out++;
						}
					}
				}
				n3=n & 31;
				u=*p;
				curbit = 0;
				for (k=0; k<n3; k++)
				{
					if ( ((uLONG)(1<<k)) & u)
						*out = *out | (1 << curbit);
					else
						*out = *out & (255 ^ (1 << curbit));
					++curbit;
					if (curbit == 8)
					{
						curbit = 0;
						out++;
					}
				}
			}
		}
	}

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTMODIFYSET, DBaction_ModifyingSet);

	//unuse();

	return(err);
}


VError Bittab::FillFromArrayOfBits(const void* inArray, sLONG inNbElements)
{
	VError err = aggrandit(inNbElements);

	if (err == VE_OK)
	{
		const uCHAR* pIN = (const uCHAR*) inArray;
		sLONG nblongs = (inNbElements / 32);
		sLONG nbbits_restants = inNbElements & 31;	// L.E. 22/03/06 was inNbElements & 32

		sLONG curpage = 0, curlong = 0;
		bitsetptr p = loadmemAndBuildIfEmpty(curpage, err);

		sLONG j;

		if (err == VE_OK)
		{
			for (sLONG i = 0; i < nblongs; i++)
			{
				if (*(const sLONG*)pIN == 0)
				{
					*p = 0;
					pIN += 4;
				}
				else
				{
					if (*(const sLONG*)pIN == -1)
					{
						*p = ALLFF;
						pIN += 4;
					}
					else
					{
						*p = 0;
						sLONG curbit = 0;
						uCHAR u = *pIN;
						for (j = 0; j < 32; j++)
						{
							if (u & ( 1 << curbit))
							{
								*p = *p | ( 1 << j);
							}
							curbit++;
							if (curbit == 8)
							{
								pIN++;
								u = *pIN;
								curbit = 0;	// L.E. 05/01/07 ACI0047451
							}
						}
					}
				}
				
				// L.E. 05/01/07 ACI0047451 increment p & curlong in each case
				p++;
				curlong++;

				if (curlong == kNbLongParSet)
				{
					curlong = 0;
					curpage++;
					p = loadmemAndBuildIfEmpty(curpage, err);	// 	// L.E. 05/01/07 ACI0047451 curpage instead of 0
					if (err != VE_OK)
					{
						break;
					}
				}
			}

			if (err == VE_OK)
			{
				*p = 0;
				sLONG curbit = 0;
				uCHAR u = *pIN;
				for (j = 0; j < nbbits_restants; j++)
				{
					if (u & ( 1 << curbit))
					{
						*p = *p | ( 1 << j);
					}
					curbit++;
					if (curbit == 8)
					{
						pIN++;
						u = *pIN;
						curbit = 0;	// L.E. 05/01/07 ACI0047451
					}
				}
			}

		}
	}

	if (err == VE_OK)
		Epure();

	return(err);
}



VError Bittab::AddBittabToSel(sLONG& curfic, sLONG maxnb, Bittab* filtre, Selection* sel)
{
	VError err = VE_OK;
	sLONG k,j,i,n,xn2,n3,cur;
	uLONG u;
	bitsetptr p;
	
	
	cur = 0;

	//use();
	for (i=0;i<nbpage && err == VE_OK;i++)
	{
		p=loadmem(i, err);
		
		if (err == VE_OK)
		{
			if (i==(nbpage-1)) n=((nbbit-1) & (kNbBitParSet-1))+1; else n=kNbBitParSet;
			if (p==nil)
			{
				if (tabplein[i])
				{
					for (j=0;j<n && err == VE_OK;j++)
					{
						if (filtre == nil || filtre->isOn(cur))
						{
							err = sel->PutFic(curfic,cur);
							++curfic;
						}
						++cur;
					}
				}
				else
				{
					cur = cur + kNbBitParSet;
				}
			}
			else
			{
				xn2=n>>5;
				for (j=0;j<xn2 && err == VE_OK;j++)
				{
					u=*p++;
					if (u!=0)
					{
						if (u==-1)
						{
							for (k=0;k<32 && err == VE_OK;k++)
							{
								if (filtre == nil || filtre->isOn(cur))
								{
									err = sel->PutFic(curfic,cur);
									++curfic;
								}
								++cur;
							}
						}
						else
						{
							for (k=0;k<32 && err == VE_OK;k++)
							{
								if ( ((uLONG)(1<<k)) & u)
								{
									if (filtre == nil || filtre->isOn(cur))
									{
										err = sel->PutFic(curfic,cur);
										++curfic;
									}
								}
								++cur;
	
							}
						}
					}
					else
					{
						cur = cur + 32;
					}
				}
				n3=n & 31;
				u=*p;
				for (k=0;k<n3 && err == VE_OK;k++)
				{
					if ( ((uLONG)(1<<k)) & u)
					{
						if (filtre == nil || filtre->isOn(cur))
						{
							err = sel->PutFic(curfic,cur);
							++curfic;
						}
					}
					++cur;
				}
			}
		}
	}

	//unuse();
	
	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTMODIFYSET, DBaction_ModifyingSet);
	
	return(err);
}


VError Bittab::FillWithSel(Selection* sel, BaseTaskInfo* context)
{
	VError err;
	raz();
	err = sel->AddToBittab(this, context);
	Epure();
	
	return err;
}

VError Bittab::RemoveSelectedRange(sLONG inRecordIndex1, sLONG inRecordIndex2)
{
	VError err = VE_OK;
	//use();

	if (inRecordIndex1<0)
		inRecordIndex1 = 0;

	if (inRecordIndex2 >= (Compte()-1))
	{
		sLONG first = PlaceDuNiemeBitAllume(inRecordIndex1);
		Clear(first);
		Reduit(first);
	}
	else
	{
		sLONG cur = PlaceDuNiemeBitAllume(inRecordIndex1);
		sLONG i = inRecordIndex1;
		while (i<=inRecordIndex2 && cur != -1)
		{
			Clear(cur);
			cur = FindNextBit(cur+1);
			i++;
		}
	}

	//unuse();
	return err;
}


VError Bittab::AddSel(Selection* sel, BaseTaskInfo* context)
{
	VError err;
	
	err = sel->AddToBittab(this, context);
	Epure();

	return err;
}


VError Bittab::GetFrom(VStream& buf)
{
	VError err = VE_OK;
	vide();
	sLONG newnbbit = 0;
	sLONG newnbpage = 0;
	err = buf.GetLong(newnbbit);
	if (err == VE_OK)
		err = buf.GetLong(newnbpage);
	if (err == VE_OK)
	{
		if (newnbbit >= 0 && newnbpage == (newnbbit+kNbBitParSet-1)>>kRatioSet)
		{
			tabplein = (uBOOL*)GetFastMem(BitTabChunkSize(newnbpage), false, 'Bse8');
			if (tabplein == nil)
				ThrowError(memfull, DBaction_LoadingSet);
			else
			{
				tabmem = (bitsetptr*)GetFastMem(BitTabChunkSize(sizeof(bitsetptr)*newnbpage), false, 'Bse9');
				if (tabmem == nil)
					ThrowError(memfull, DBaction_LoadingSet);
				else
				{
					memset(tabplein, 0 /*kBitPageNonVide*/, newnbpage);
					_raz(tabmem, sizeof(bitsetptr)*newnbpage);
					nbpage = newnbpage;
					nbbit = newnbbit;

					err = buf.GetData(tabplein, newnbpage);
					if (err != VE_OK)
					{
						//memset(tabplein, kBitPageNonVide, newnbpage); // pourquoi ?
					}
					else
					{
						sLONG i;
						for (i = 0; i < nbpage; i++)
						{
							if (tabplein[i] == kBitPageNonVide)
							{
								bitsetptr p = (bitsetptr)GetFastMem(kSizeSetMem, false, 'bsel');
								if (p == nil)
								{
									ThrowError(memfull, DBaction_LoadingSet);
									break;
								}
								else
								{
									tabmem[i] = p;
									sLONG nbread = kNbLongParSet;
									err = buf.GetLongs(p, &nbread);
									if (nbread != kNbLongParSet)
										err = VE_DB4D_CANNOTLOADSET;
									p[kNbLongParSet] = 0;
									if (err != VE_OK)
										break;
								}
							}
						}
					}

				}
			}
			
			if (err != VE_OK)
				vide();
		}
		else
			err = VE_DB4D_CANNOTLOADSET;
	}
	
	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTLOADSET, DBaction_LoadingSet);

	return err;
}


VError Bittab::PutInto(VStream& buf)
{
	VError err = VE_OK;
	Epure();
	err = buf.PutLong(nbbit);
	if (err == VE_OK)
		err = buf.PutLong(nbpage);

	if (nbpage > 0 && err == VE_OK)
	{
		err = buf.PutData(tabplein, nbpage);
		if (err == VE_OK)
		{
			sLONG i;
			for (i = 0; i < nbpage; i++)
			{
				bitsetptr p = loadmem(i, err);
				if (err == VE_OK)
				{
					if (tabplein[i] == kBitPageNonVide)
					{
						err = buf.PutLongs(p, kNbLongParSet);
						if (err != VE_OK)
							break;
					}
				}
			}
		}
	}

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTSAVESET, DBaction_SavingSet);

	return err;
}


VError Bittab::TransformIntoCluster(DataAddr4D addr, Base4D *xdb)
{
	VError err = VE_OK;
	TabPage = (DataAddr4D*)GetFastMem( BitTabChunkSize(nbpage<<3), false, 'Bse0');
	if (TabPage != nil)
	{
		_raz(TabPage, nbpage<<3);
		SetAddr(addr, xdb);
		for (sLONG i = 0; i < nbpage; i++)
		{
			if (tabplein[i] == kBitPageNonVide)
			{
				bitsetptr p = tabmem[i];
				xbox_assert(p != nil);
				p[kNbLongParSet] = -1;
				DataAddr4D lll=db->findplace(kSizeSetDisk+kSizeDataBaseObjectHeader, nil, err, -kIndexSegNum, (ObjAlmostInCache*)this);
				if (err == VE_OK && lll>0)
				{
#if debugIndexOverlap_strong
					di_IndexOverLap::AddClusterPart(nil, -1, nbpage, lll, kSizeSetDisk+kSizeDataBaseObjectHeader);
#endif
					TabPage[i]=lll;
				}
				else
					break;
			}
			else
				TabPage[i] = tabplein[i];
		}
		if (err != VE_OK)
		{
			for (sLONG i = 0; i < nbpage; i++)
			{
				if (TabPage[i] != 0)
				{
#if debugIndexOverlap_strong
					di_IndexOverLap::RemoveClusterPart(nil, -1, i, TabPage[i], kSizeSetDisk+kSizeDataBaseObjectHeader);
#endif
					db->libereplace(TabPage[i], kSizeSetDisk+kSizeDataBaseObjectHeader, nil, (ObjAlmostInCache*)this);
				}
			}
		}
	}
	else
		err = ThrowError(memfull, DBaction_ModifyingSet);

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTMODIFYSET, DBaction_ModifyingSet);

	return err;
}


void Bittab::cut()
{
	nbbit = 0;
	nbpage = 0;
	SelAddr = 0;
	db = nil;
	tabmem = nil;
	tabplein = nil;
	TabPage = nil;
	SelisCluster = false;
	decompte = 0;
	equisel = -1;
}


Bittab* Bittab::Clone(VError& err) const
{
	Bittab* b = new Bittab;
	if (b == nil)
		err = ThrowError(memfull, noaction);
	else
	{
		err = b->Or((Bittab*)this);
	}
	return b;
}



VError Bittab::FromServer(VStream* from, CDB4DBaseContext* inContext)
{
	VError err = VE_OK;

	VUUID xid;
	err = xid.ReadFromStream(from);
	xid.ToBuffer(fID);
	fIDHasBeenGenerated = true;
	sLONG nb = 0;
	if (err == VE_OK)
		err = from->GetLong(nb);
	if (err == VE_OK)
		err = from->GetLong(decompte);
	if (err == VE_OK)
	{
		err = aggrandit(nb);
		if (err == VE_OK)
		{
			if (nb > 0)
			{
				VSize nb2;
				err = from->GetData(tabplein, nbpage, &nb2);
				if ((sLONG)nb2 != nbpage)
					err = ThrowError(VE_DB4D_CANNOTBUILDSELECTION, noaction);
				else
				{
					Boolean first = true;
					for (sLONG i = 0; i < nbpage; i++)
					{
						tabmem[i] = nil;
						if (tabplein[i] == kBitPageNonVide && first)
						{
							first = false;
							bitsetptr tb = (bitsetptr)GetFastMem(kSizeSetMem, false, 'bsel');
							if (tb == nil)
								err = ThrowError(memfull, noaction);
							else
							{
								sLONG nb3 = kNbLongParSet;
								err = from->GetLongs(tb, &nb3);
								if (nb3 != kNbLongParSet)
									err = ThrowError(VE_DB4D_CANNOTBUILDSELECTION, noaction);
								if (err == VE_OK)
								{
									tabmem[i] = tb;
									tb[kNbLongParSet] = 0;

								}
								else
									FreeFastMem(tb);
							}
						}
					}
				}
			}
		}
	}

	//decompte = -1;
	equisel = -1;
	fIsRemote = true;
	return err;

}


VError Bittab::ToClient(VStream* into, CDB4DBaseContext* inContext)
{
	VError err = VE_OK;

	if (!fIDHasBeenGenerated)
	{
		GenerateID();
		fIDHasBeenGenerated = true;
	}
	VUUID xid(fID);
	err = xid.WriteToStream(into);
	if (err == VE_OK)
		err = into->PutLong(nbbit);
	if (err == VE_OK)
	{
		Compte();
		err = into->PutLong(decompte);
	}
	if (nbbit > 0)
	{
		if (err == VE_OK)
		{
			err = into->PutData(tabplein, nbpage);
		}
		if (err == VE_OK)
		{
			for (sLONG i = 0; i < nbpage; i++)
			{
				if (tabplein[i] == kBitPageNonVide)
				{
					bitsetptr tb = loadmem(i, err);
					if (tb != nil)
					{
						err = into->PutLongs(tb, kNbLongParSet);
					}
					break;
				}
			}
		}
	}

	if (err == VE_OK)
	{
		if (!fIsASel)
			VDBMgr::GetManager()->KeepSetOnServer(this);
	}

	return err;
}



VError Bittab::FromClient(VStream* from, CDB4DBaseContext* inContext)
{
	// est appele dans le cas ou la selection a ete construite sur le client
	VError err = VE_OK;

	VUUID newid;
	err = newid.ReadFromStream(from);
	fID = newid.GetBuffer();
	fIDHasBeenGenerated = true;
	sLONG newnbbit;
	if (err == VE_OK)
		err = from->GetLong(newnbbit);
	if (err == VE_OK)
		err = from->GetLong(decompte);

	vide();

	if (err == VE_OK)
		err = aggrandit(newnbbit);

	if (nbbit > 0 && err == VE_OK)
	{
		if (err == VE_OK)
		{
			err = from->GetData(tabplein, nbpage);
		}
		if (err == VE_OK)
		{
			for (sLONG i = 0; i < nbpage && err == VE_OK; i++)
			{
				if (tabplein[i] == kBitPageNonVide)
				{
					bitsetptr p=(bitsetptr)GetFastMem(kSizeSetMem, false, 'bsel');
					if (p == nil)
					{
						err = ThrowBaseError(memfull, noaction);
					}
					else
					{
						tabmem[i] = p;
						sLONG nblong = kNbLongParSet;
						err = from->GetLongs((sLONG*)p, &nblong);
						p[kNbLongParSet] = 0;
					}
				}
			}
		}
	}

	return err;
}

VError Bittab::ToServer(VStream* into, CDB4DBaseContext* inContext, bool inKeepOnServer)
{
	VError err = VE_OK;

	if (fIsRemote)
	{
		err = into->PutLong(sel_bitsel);
		VUUID xid(fID);
		xbox_assert(fIDHasBeenGenerated);
		err = xid.WriteToStream(into);
	}
	else
	{
		err = into->PutLong( inKeepOnServer ? sel_bitsel_fullfromclient_keepOnServer : sel_bitsel_fullfromclient);
		VUUID newid;
		if (fIDHasBeenGenerated)
			newid = fID;
		else
		{
			newid.Regenerate();
			fID = newid.GetBuffer();
			fIDHasBeenGenerated = true;
		}
		err = newid.WriteToStream(into);
		if (err == VE_OK)
			err = into->PutLong(nbbit);
		if (err == VE_OK)
			err = into->PutLong(decompte);
		if (nbbit > 0)
		{
			if (err == VE_OK)
			{
				err = into->PutData(tabplein, nbpage);
			}
			if (err == VE_OK)
			{
				for (sLONG i = 0; i < nbpage && err == VE_OK; i++)
				{
					if (tabplein[i] == kBitPageNonVide)
					{
						bitsetptr tb = loadmem(i, err);
						if (tb != nil && err == VE_OK)
						{
							err = into->PutLongs(tb, kNbLongParSet);
						}
					}
				}
			}
		}

		if ((err == VE_OK) && inKeepOnServer)
			fIsRemote = true;
	}
	return err;
}



VError Bittab::GetPartReply(sLONG numpart, IRequestReply *inRequest)
{
	VError err = VE_OK;
	if (numpart<0 || numpart >= nbpage)
		err = ThrowError(VE_DB4D_INVALID_SELECTION_PART, noaction);
	else
	{
		bitsetptr tsh = loadmem(numpart, err);
		if (tsh != nil && err == VE_OK)
		{
			inRequest->PutByteReply(kBitPageNonVide);
			err = inRequest->PutArrayLongReply((sLONG*)tsh, kNbLongParSet);
		}
		else if (err == VE_OK)
		{
			inRequest->PutByteReply(tabplein[numpart]);
		}
		else
			err = ThrowError(VE_DB4D_INVALID_SELECTION_PART, noaction);
	}
	return err;
}


void Bittab::ClearRemoteCache(VStream* fromServer)
{
	if (fIsRemote)
	{
		if (nbpage > 0)
		{
			for (bitsetptr *cur = tabmem, *end = tabmem+nbpage; cur != end; cur++)
			{
				bitsetptr p = *cur;
				if (p != nil && p != (bitsetptr)-1 && p != (bitsetptr)-2)
				{
					FreeFastMem(p);
				}
				*cur = nil;
			}
			sLONG newnbbit;
			fromServer->GetLong(newnbbit);
			sLONG newnbpage = (newnbbit+kNbBitParSet-1)>>kRatioSet;
			if (BitTabChunkSize(sizeof(bitsetptr)*newnbpage) != BitTabChunkSize(sizeof(bitsetptr)*nbpage))
			{
				FreeFastMem(tabmem);
				tabmem = nil;
				if (newnbpage > 0)
				{
					tabmem = (bitsetptr*) GetFastMem(BitTabChunkSize(sizeof(bitsetptr)*newnbpage), false, 'bsel');
					fill(&tabmem[0], &tabmem[nbpage], (bitsetptr)nil);
				}
			}
			if (BitTabChunkSize(newnbpage) != BitTabChunkSize(nbpage))
			{
				FreeFastMem(tabplein);
				tabplein = nil;
				if (newnbpage > 0)
				{
					tabplein = (uBOOL*)GetFastMem(BitTabChunkSize(newnbpage), false, 'bsel');
				}
			}

			nbbit = newnbbit;
			nbpage = newnbpage;
			fromServer->GetData(tabplein, nbpage);

			uBOOL *p2 = tabplein;
			for (bitsetptr *cur = tabmem, *end = tabmem+nbpage; cur != end; cur++, p2++)
			{
				if ((*p2) != kBitPageNonVide)
					*cur = (bitsetptr)-2;
				else
					*cur = nil;
			}
		}
	}
}


void Bittab::PutClearCacheInfo(VStream* toClient)
{
	if (nbpage > 0)
	{
		toClient->PutLong(nbbit);
		toClient->PutData(tabplein, nbpage);
	}
}


bool Bittab::MatchAllocationNumber(sLONG allocationNumber, void* owner)
{
	if (allocationNumber == -1)
		return true;
	if (owner != nil && GetAllocationNumber(owner) == allocationNumber)
		return true;
	if (tabmem != nil && GetAllocationNumber(tabmem) == allocationNumber)
		return true;
	if (tabplein != nil && GetAllocationNumber(tabplein) == allocationNumber)
		return true;
	if (TabPage != nil && GetAllocationNumber(TabPage) == allocationNumber)
		return true;

	if (tabmem != nil)
	{
		for (sLONG i = 0; i < nbpage; i++)
		{
			bitsetptr p = tabmem[i];
			if (p != nil && p != (bitsetptr)-1 && p != (bitsetptr)-2)
			{
				if (GetAllocationNumber(p) == allocationNumber)
					return true;
			}
		}
	}

	return false;
}



#if debug_bittab_threadsafe

void Bittab::debugCheckOwner()
{
	if (fDebugTaskOwner != 0)
	{
		VTaskID tid = VTask::GetCurrentID();
		if (tid != fDebugTaskOwner)
		{
			sLONG xdebug = 1; // put a break here
			assert(tid == fDebugTaskOwner);
		}
	}

}

void Bittab::debugUse()
{
	VTaskID tid = VTask::GetCurrentID();
	if (fDebugTaskOwner != 0)
	{
		if (tid != fDebugTaskOwner)
		{
			sLONG xdebug = 1; // put a break here
			assert(tid == fDebugTaskOwner);
		}
	}
	else
	{
		fDebugTaskOwner = tid;
	}
}

void Bittab::debugUnuse()
{
	VTaskID tid = VTask::GetCurrentID();
	if (tid != fDebugTaskOwner)
	{
		sLONG xdebug = 1; // put a break here
		assert(tid == fDebugTaskOwner);
	}
	else
		fDebugTaskOwner = 0;
}


#endif


















