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

#include <algorithm>


#if debugLogEventPageIndex
EventLogger<dbgIndexEvent, 1000> gDebugIndexEvent;
#endif

#if debuglr_page
BTreePageIndex* debug_tabpage[30000];
sLONG debug_nbpage = 0;
VCriticalSection debug_mutex;

void debug_addpage(BTreePageIndex* p)
{
	VTaskLock lock(&debug_mutex);
	assert(debug_nbpage<30000);
	debug_tabpage[debug_nbpage] = p;
	debug_nbpage++;
}

sLONG debug_findpage(BTreePageIndex* p)
{
	sLONG i, res = -1;
	for (i = 0; i < debug_nbpage; i++)
	{
		if (debug_tabpage[i] == p)
		{
			res = i;
			break;
		}
	}
	return res;
}

void debug_suppage(BTreePageIndex* p)
{
	VTaskLock lock(&debug_mutex);
	sLONG i,n = debug_findpage(p);
	assert(n>=0);
	if (n>=0)
	{
		BTreePageIndex* *tp = &(debug_tabpage[n]);
		BTreePageIndex* *tp2 = &(debug_tabpage[n+1]);

		for (i = n+1; i  < debug_nbpage; i++)
		{
			*tp = *tp2;
			tp++;
			tp2++;
		}
		debug_nbpage--;
	}
}

void debug_checkpages()
{
	VTaskLock lock(&debug_mutex);
	sLONG i;
	for (i = 0; i < debug_nbpage; i++)
	{
		BTreePageIndex* sousBT = debug_tabpage[i];
		assert(sousBT->GetParent()->FindPosOfChild(sousBT) == sousBT->GetPosInParent());
	}
}

#endif

#if debuglr
Boolean debug_underflow_has_been_executed = false;
#endif

#if debuglr == 112

Boolean OKTOCHECKTABMEM1 = true;

class pageindexinfo
{
	public:
		Boolean operator == (const pageindexinfo& other) const { return page == other.page; };

		BTreePageIndex* page;
		Boolean lenpageHasChanged;
		Boolean pageWasSaved;
};

VArrayOf<pageindexinfo> pagesinfodebug;

void ClearPageInfoDebug()
{
	pagesinfodebug.SetAllocatedSize(0);
}


void AddPageInfoDebug(BTreePageIndex* xpage, Boolean xlenpageHasChanged, Boolean xpageWasSaved)
{
	pageindexinfo p;
	p.page = xpage;
	sLONG n = pagesinfodebug.FindPos(p);
	if (n>0)
	{
		p.lenpageHasChanged = xlenpageHasChanged;
		p.pageWasSaved = xpageWasSaved;
		pagesinfodebug[n-1] = p;
	}
	else
	{
		p.lenpageHasChanged = p.lenpageHasChanged || xlenpageHasChanged;
		p.pageWasSaved = p.pageWasSaved || xpageWasSaved;
		pagesinfodebug.Add(p);
	}
}

void DelPageInfoDebug(BTreePageIndex* xpage)
{
	pageindexinfo p;
	p.page = xpage;
	pagesinfodebug.Delete(p);
}


void CheckPageInfoDebug()
{
	sLONG i,nb;
	
	nb = pagesinfodebug.GetCount();
	
	for (i=0;i<nb;i++)
	{
		pageindexinfo p = pagesinfodebug[i];
		if (p.lenpageHasChanged)
			assert(p.pageWasSaved);
	}
}

#endif



IndexKeyArray::~IndexKeyArray()
{
	sLONG i;
	for (i=0; i<nbval; i++)
	{
		entete->FreeKey(vals[i]);
	}
}

#if debug_BTItemIndex
debug_BTitemIndexSet BTitemIndex::sdebug_BTitemIndexSet;
#endif


void BTitemIndex::GetDebugString(IndexInfo* ind, VString& outString)
{
	VError err;
	ValPtr cv = ind->CreateVValueWithKey(this, err);
	outString.FromValue(*cv);
	delete cv;
}


									/* -----------------------------------------------  */


BTitemIndex* BTitemIndexHolder::StealKey()
{
	BTitemIndex* result;

	LoadFromTemp();

	result = fKey;
	fKey = nil;

	return result;
}


BTitemIndex* BTitemIndexHolder::RetainKey() const
{
	BTitemIndex* result;

	LoadFromTemp();

	result = fKey;
	if (result != nil)
		result->Use();

	return result;
}


class IndexKeyTempHeader
{
	public:
		//Transaction* fTrans;
		sLONG fLen;
};


VError BTitemIndexHolder::SaveToTemp(sLONG allocationBlockNumber, Transaction* trans, sLONG& totfreed)
{
	VError err = VE_OK;
	totfreed = 0;
	if (fKey != nil && OKAllocationNumber(fKey,allocationBlockNumber))
	{
		if (fAddr == -1)
		{
			sLONG len = fKey->GetInd()->CalulateFullKeyLengthInMem(fKey) + sizeof(IndexKeyTempHeader);
			fAddr = trans->FindTempSpace(len, err);
			fTransID = trans->GetID();
			if (fAddr != -1)
			{
#if debugtrans_temp
				DebugMsg(L" Index Save To Temp : "+ToString(fAddr)+L" , len = "+ToString(len)+L"\n");
#endif
				IndexKeyTempHeader header;
				//header.fTrans = trans;
				header.fLen = len;
				err = trans->PutTempData(&header, sizeof(IndexKeyTempHeader), fAddr);
				if (err == VE_OK)
				{
					err = trans->PutTempData(fKey, len - sizeof(IndexKeyTempHeader), fAddr + sizeof(IndexKeyTempHeader));
				}
			}
		}
		if (err == VE_OK)
		{
			fKey->Unuse();  // le compteur est a deux et devrait etre a un, la cle n'est pas released

			//fKey->GetInd()->FreeKey(fKey);
			fKey = nil;
		}
		else
			fAddr = -1;
	}

	if (err != VE_OK)
		err = ThrowBaseError(VE_DB4D_CANNOT_SAVE_INDEXKEY, DBaction_SavingIndexKeyFromTrans);

	return err;
}


VError BTitemIndexHolder::LoadFromTemp() const
{
	VError err = VE_OK;
	if (fKey == nil && fAddr >= 0)
	{
		Transaction* trans = Transaction::GetTransFromID(fTransID);

		IndexKeyTempHeader header;
		err = trans->GetTempData(&header, sizeof(IndexKeyTempHeader), fAddr);
		if (err == VE_OK)
		{
			if (err == VE_OK)
			{
				void* p = GetFastMem(header.fLen - sizeof(IndexKeyTempHeader), true, 'iKey');
				if (p != nil)
				{
					fKey = new (p) BTitemIndex();
					err = trans->GetTempData(p, header.fLen - sizeof(IndexKeyTempHeader), fAddr + sizeof(IndexKeyTempHeader));
					if (err != VE_OK)
					{
						delete fKey;
						fKey = nil;
					}
					else
						fKey->StartUse();
				}
				else
					err = ThrowBaseError(memfull, DBaction_LoadingIndexKeyFromTrans);
			}
		}
	}

	if (err != VE_OK)
		err = ThrowBaseError(VE_DB4D_CANNOT_LOAD_INDEXKEY, DBaction_LoadingIndexKeyFromTrans);
	return err;
}



									/* -----------------------------------------------  */




IndexHeader::IndexHeader()
{
	IHD.addrprim = -1;
	IHD.nextfreepage = kFinDesTrou;
	IHD.nbpage = 0;
	IHD.needRebuild = 0;
	IHD.cFiller1 = 0;
	IHD.cFiller2 = 0;
	IHD.cFiller3 = 0;
	IHD.filler2 = 0;
	IHD.filler3 = 0;
	IHD.filler4 = 0;
	IHD.lenNULLs = 0;
	IHD.AddrNULLs = -1;
	fNulls = nil;
	//IHD_SavedByTrans.nbpage = -1;
}


 IndexHeader::~IndexHeader()
 {
	 QuickReleaseRefCountable(fNulls);
 }



VError IndexHeader::ThrowError( VError inErrCode, ActionDB4D inAction)
{
	if (assoc != nil)
		return assoc->ThrowError(inErrCode, inAction);
	else
		return inErrCode;
}


void IndexHeader::setmodif(uBOOL xismodif, Base4D* bd, BaseTaskInfo* context)
{
	if (xismodif)
	{
		GetAssoc()->ModifyIt(context);
	}
	//IObjToFlush::setmodif(xismodif, bd, context);
}


bool IndexHeader::IsInvalidOnDisk()
{
	return (IHD.needRebuild != 0);
}

void IndexHeader::SetInvalidOnDisk()
{
	IHD.needRebuild = 1;
	setmodif(true, GetAssoc()->GetDB(), nil);
}

void IndexHeader::SetValidOnDisk()
{
	IHD.needRebuild = 0;
	setmodif(true, GetAssoc()->GetDB(), nil);
}


void IndexHeader::SetAssoc(IndexInfo* x) 
{ 
	assoc=x; 
	taddr.Init(assoc->GetDB(),this,&IHD.addrprim,&IHD.nextfreepage,&IHD.nbpage, -kIndexSegNum, false); 
}


uBOOL IndexHeader::MayBeSorted(void)
{
	return(true);
}

VError IndexHeader::PutInto(VStream& buf)
{
	VError err;
	
	err=buf.PutData(&IHD,sizeof(IHD));
	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTSAVEINDEXHEADER, DBaction_SavingIndexHeader);

	return(err);
}


void IndexHeaderDISK::SwapBytes()
{
	ByteSwap(&addrprim);
	ByteSwap(&nextfreepage);
	ByteSwap(&nbpage);
	ByteSwap(&lenNULLs);
	ByteSwap(&AddrNULLs);
}


VError IndexHeader::GetFrom(VStream& buf)
{
	VError err;
	
	err=buf.GetData(&IHD,sizeof(IHD));
	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTLOADINDEXHEADER, DBaction_LoadingIndexHeader);
	else
	{
		if (buf.NeedSwap())
		{
			IHD.SwapBytes();
		}
	}
	return(err);
}


sLONG IndexHeader::GetTyp(void) const
{ 
	if (assoc->IsAuto()) 
		return DB4D_Index_AutoType;
	else
		return(typindex); 
}


DataAddr4D IndexHeader::GetInd(OccupableStack* curstack, sLONG n, BaseTaskInfo* context, VError& err, sLONG* outLen)
{
	DataAddr4D ou;
	err = VE_OK;
	
	ou=taddr.GetxAddr(n, context, err, curstack, outLen);
	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTGETINDEXPAGEADDR, DBaction_GettingIndexPageAddr);
	return(ou);
}

sLONG IndexHeader::PutInd(OccupableStack* curstack, sLONG n, DataAddr4D r, sLONG len, BaseTaskInfo* context, VError& err)
{
	err = VE_OK;
	
	if (n==-1)
	{
		n=taddr.findtrou(context, err, curstack);
		if (n==-1) 
			n = IHD.nbpage;
	}
	if (err == VE_OK)
	{
		err=taddr.PutxAddr(n,r,len, context, curstack);
	}
	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTSETINDEXPAGEADDR, DBaction_SettingIndexPageAddr);
	return(n);
}


void IndexHeader::LiberePage(OccupableStack* curstack, sLONG numpage, BaseTaskInfo* context)
{
	taddr.liberetrou(numpage, context, curstack);
}


VError IndexHeader::LibereEspaceDisk(OccupableStack* curstack, VDB4DProgressIndicator* InProgress)
{
	VError err = taddr.LibereEspaceDisk(InProgress, curstack);
	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTDROPINDEXONDISK, DBaction_DropingIndex);
	else
	{
		IHD.addrprim=-1;
		IHD.nextfreepage=kFinDesTrou;
		IHD.nbpage=0;
		if (IHD.AddrNULLs > 0)
		{
			StErrorContextInstaller errs(false);
			assoc->GetDB()->libereplace(IHD.AddrNULLs, IHD.lenNULLs+kSizeDataBaseObjectHeader, nil);
			IHD.AddrNULLs = -1;
			IHD.lenNULLs = 0;
		}
	}
	return err;
}

void IndexHeader::LibereEspaceMem(OccupableStack* curstack)
{
	if (fNulls != nil)
		fNulls->setmodif(false, assoc->GetDB(), nil);
	ReleaseRefCountable(&fNulls);
}


/*
void IndexHeader::SaveIHDForTrans(BaseTaskInfo* context)
{
	if ( (GetCurTrans(context) != nil) && (IHD_SavedByTrans.nbpage == -1))
	{
		IHD_SavedByTrans = IHD;
	}
}
*/


VError IndexHeader::InitTablesAddr(OccupableStack* curstack)
{
	sLONG nbelem = IHD.nbpage;
	IHD.nbpage = 0;
	IHD.nextfreepage = kFinDesTrou;
	IHD.addrprim = 0;
	return taddr.InitAndSetSize(nbelem, nil, curstack);
}


VError IndexHeader::NormalizeTablesAddr(OccupableStack* curstack)
{
	return taddr.Normalize(nil, curstack);
}


VError IndexHeader::ModifyNulls(OccupableStack* curstack, BaseTaskInfo* context)
{
	VError err = VE_OK;
	BitSel* sel = fNulls;

	if (sel != nil)
	{
		Base4D* bd = assoc->GetDB();
		sLONG len = sel->CalcLenOnDisk();
		sLONG oldlen = IHD.lenNULLs;
		if (IHD.AddrNULLs <=0 || (adjuste(len+kSizeDataBaseObjectHeader) != adjuste(oldlen+kSizeDataBaseObjectHeader)))
		{
			DataAddr4D oldaddr = IHD.AddrNULLs;
			DataAddr4D ou = bd->findplace(len+kSizeDataBaseObjectHeader, context, err, -kIndexSegNum, sel);
			if (ou>0 && err == VE_OK)
			{
				if (sel->IsCluster())
					sel->SetSelAddr(ou);
				else
					sel->TransformIntoCluster(ou);
				sel->ChangeAddr(ou, bd, context);
#if debugIndexOverlap_strong
				di_IndexOverLap::AddNullsCluster(assoc, ou, len+kSizeDataBaseObjectHeader);
#endif
				IHD.AddrNULLs = ou;
				IHD.lenNULLs = len;
				setmodif(true, bd, context);
				sel->setmodif(true, bd, context);
			}
			if (err == VE_OK && oldaddr > 0)
			{
#if debugIndexOverlap_strong
				di_IndexOverLap::RemoveNullsCluster(assoc, oldaddr, oldlen+kSizeDataBaseObjectHeader);
#endif
				bd->libereplace(oldaddr, oldlen+kSizeDataBaseObjectHeader, context, sel);
			}
		}
		else
			sel->setmodif(true, bd, context);
	}

	return err;
}


VError IndexHeader::AddToNulls(OccupableStack* curstack, Bittab* b, BaseTaskInfo* context)
{
	VError err = VE_OK;

	BitSel* sel = RetainNulls(curstack, err, context);
	if (sel == nil && err == VE_OK)
	{
		Base4D* bd = assoc->GetDB();
		sel = new BitSel(assoc->GetTargetTable()->GetDF());
		if (sel != nil)
		{
			sel->Retain();
			fNulls = sel;
		}
		else
			err = ThrowError(memfull, DBaction_BuildingSelection);
	}
	if (sel != nil)
	{
		err = sel->GetTB()->Or(b, true);
		if (err == VE_OK)
		{
			err = ModifyNulls(curstack, context);
		}
		sel->Release();
	}

	if (err != VE_OK)
	{
		err = ThrowError(VE_DB4D_CANNOT_MODIFY_LIST_OF_NULL_KEYS, DBaction_ModifyingNullIndexKeys);
	}

	return err;
}



VError IndexHeader::DelFromNulls(OccupableStack* curstack, Bittab* b, BaseTaskInfo* context)
{
	VError err = VE_OK;

	BitSel* sel = RetainNulls(curstack, err, context);
	if (sel == nil && err == VE_OK)
	{
		Base4D* bd = assoc->GetDB();
		sel = new BitSel(assoc->GetTargetTable()->GetDF());
		if (sel != nil)
		{
			sel->Retain();
			fNulls = sel;
		}
		else
			err = ThrowError(memfull, DBaction_BuildingSelection);
	}
	if (sel != nil)
	{
		err = sel->GetTB()->moins(b, true);
		if (err == VE_OK)
		{
			err = ModifyNulls(curstack, context);
		}
		sel->Release();
	}

	if (err != VE_OK)
	{
		err = ThrowError(VE_DB4D_CANNOT_MODIFY_LIST_OF_NULL_KEYS, DBaction_ModifyingNullIndexKeys);
	}

	return err;
}


VError IndexHeader::AddToNulls(OccupableStack* curstack, sLONG numrec, BaseTaskInfo* context)
{
	VError err = VE_OK;

	BitSel* sel = RetainNulls(curstack, err, context);
	if (sel == nil && err == VE_OK)
	{
		Base4D* bd = assoc->GetDB();
		sel = new BitSel(assoc->GetTargetTable()->GetDF());
		if (sel != nil)
		{
			sel->Retain();
			fNulls = sel;
		}
		else
			err = ThrowError(memfull, DBaction_BuildingSelection);
	}
	if (sel != nil)
	{
		err = sel->GetTB()->Set(numrec, true);
		if (err == VE_OK)
		{
			err = ModifyNulls(curstack, context);
		}
		sel->Release();
	}

	if (err != VE_OK)
	{
		err = ThrowError(VE_DB4D_CANNOT_MODIFY_LIST_OF_NULL_KEYS, DBaction_ModifyingNullIndexKeys);
	}

	return err;
}

VError IndexHeader::DelFromNulls(OccupableStack* curstack, sLONG numrec, BaseTaskInfo* context)
{
	VError err = VE_OK;

	BitSel* sel = RetainNulls(curstack, err, context);
	if (sel != nil)
	{
		if (sel->GetTB()->isOn(numrec))
		{
			err = sel->GetTB()->Clear(numrec);
			if (err == VE_OK)
			{
				err = ModifyNulls(curstack, context);
			}
		}
		sel->Release();
	}

	if (err != VE_OK)
	{
		err = ThrowError(VE_DB4D_CANNOT_MODIFY_LIST_OF_NULL_KEYS, DBaction_ModifyingNullIndexKeys);
	}

	return err;
}


Boolean IndexHeader::HasNulls(OccupableStack* curstack)
{
	Boolean result = false;
	StErrorContextInstaller errs(false);
	VError err;
	BitSel* nulls = RetainNulls(curstack, err, nil);
	if (nulls != nil)
	{
		result = !nulls->IsEmpty();
		nulls->Release();
	}
	return result;
}


BitSel* IndexHeader::RetainNulls(OccupableStack* curstack, VError& err, BaseTaskInfo* context)
{
	err = VE_OK;

	if (fNulls == nil)
	{
		if (IHD.AddrNULLs > 0)
		{
			Base4D* bd = assoc->GetDB();

			DataBaseObjectHeader tag;
			err = tag.ReadFrom(bd, IHD.AddrNULLs, nil);
			if (err == VE_OK)
			{
				err = tag.ValidateTag(DBOH_SetDiskTable, -1, -1);
			}
			sLONG typsel;
			if (err == VE_OK)
				err = bd->readlong(&typsel, 4, IHD.AddrNULLs, kSizeDataBaseObjectHeader);
			if (err==VE_OK)
			{
				if (tag.NeedSwap())
					typsel = SwapLong(typsel);
				if (testAssert(typsel==sel_bitsel))
				{
					fNulls = new BitSel(assoc->GetTargetTable()->GetDF());
					if (fNulls == nil)
						err = ThrowError(memfull, DBaction_BuildingSelection);
					else
					{
						fNulls->SetSelAddr(IHD.AddrNULLs);
						err = fNulls->LoadSel(tag);
						if (err != VE_OK)
						{
							fNulls->Dispose();
							fNulls = nil;
						}
					}
				}
			}
		}
	}

	if (fNulls != nil)
		fNulls->Retain();

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOT_GET_LIST_OF_NULL_KEYS, DBaction_AccessingNullIndexKeys);
	return fNulls;
}



									/* -----------------------------------------------  */

IndexHeaderBTreeRoot::IndexHeaderBTreeRoot(IndexInfo *xentete)
{
	entete=xentete;
	SetAssoc(xentete);
	typindex=DB4D_Index_Btree;
	HBT.FirstPage=-1;
}


VError IndexHeaderBTreeRoot::ThrowError( VError inErrCode, ActionDB4D inAction)
{
	if (assoc != nil)
		return assoc->ThrowError(inErrCode, inAction);
	else
		return inErrCode;
}


sLONG IndexHeaderBTreeRoot::GetLen(void)
{
	return(sizeof(HeaderBtreeDisk));
}

VError IndexHeaderBTreeRoot::PutInto(VStream& buf)
{
	VError err;
	
	err=IndexHeader::PutInto(buf);
	if (err==VE_OK)
	{
		err=buf.PutData(&HBT,sizeof(HBT));
		if (err != VE_OK)
			err = ThrowError(VE_DB4D_CANNOTSAVEINDEXHEADER, DBaction_SavingIndexHeader);
	}

	return(err);
}

void HeaderBtreeDisk::SwapBytes()
{
	ByteSwap(&FirstPage);
}

VError IndexHeaderBTreeRoot::GetFrom(VStream& buf)
{
	VError err;
	
	err=IndexHeader::GetFrom(buf);
	if (err==VE_OK) 
	{
		err=buf.GetData(&HBT,sizeof(HBT));
		if (err != VE_OK)
			err = ThrowError(VE_DB4D_CANNOTLOADINDEXHEADER, DBaction_LoadingIndexHeader);
		else
		{
			if (buf.NeedSwap())
			{
				HBT.SwapBytes();
			}
		}
	}

	return(err);
}


bool IndexHeaderBTreeRoot::SaveObj(VSize& outSizeSaved)
{
	assert(false);
	outSizeSaved = 0;
	return true;
}




VError IndexHeaderBTreeRoot::LibereEspaceDisk(OccupableStack* curstack, VDB4DProgressIndicator* InProgress)
{
	sLONG i;
	sLONG len;
	VError err = VE_OK, err2;
	DataAddr4D ou;
	Base4D *db;
	Boolean AuMoinsUneErreur = false;
	Boolean PremiereErreur = true;

	Occupy(curstack, true);

	db=entete->GetDB();
	for (i=0;i<IHD.nbpage;i++)
	{
		ou=GetInd(curstack, i, nil, err2, &len);
		if (err2 != VE_OK)
		{
			if (PremiereErreur)
				PremiereErreur = false;
			else
				err2 = PullLastError();
			AuMoinsUneErreur = false;
		}
		else
		{
			if (ou>0)
			{
#if debugIndexOverlap_strong
				di_IndexOverLap::RemoveIndexPage(assoc, i, ou, len);
#endif
				err2 = db->libereplace(ou, len, nil, this);
#if debugplace
				shex.FromULong((uLONG)ou,kHexaDecimal);
				UnivStrDebug("LiberePlace Page Index = ",shex);
#endif
				if (err2 != VE_OK)
				{
					if (PremiereErreur)
						PremiereErreur = false;
					else
						err2 = PullLastError();
					AuMoinsUneErreur = false;
				}
			}
		}
	}
	err = IndexHeader::LibereEspaceDisk(curstack, InProgress);

	if (AuMoinsUneErreur)
	{
		err = ThrowError(VE_DB4D_COULDNOTCOMPLETELYDELETEINDEX, DBaction_DeletingIndex);
	}

	Free(curstack, true);

	return err;
}







									/* -----------------------------------------------  */



void IndexHeaderBTree::TryToFreeMem(sLONG allocationBlockNumber, VSize combien, VSize& tot)
{
	tot = 0;
	
	if (firstpage != nil)
	{
		if (firstpage->FreePageMem(allocationBlockNumber, combien, tot) && !IsOccupied())
		{
			tot = tot + firstpage->GetSizeToFree();
			//assert(firstpage->GetRefCount() == 1);
			firstpage->ReleaseAndCheck();
			firstpage = nil;
		}
	}

	//if (tot < combien)
	{
		VSize tot2;
		taddr.TryToFreeMem(allocationBlockNumber, combien-tot, tot2);
		tot = tot + tot2;
	}
}


VError IndexHeaderBTree::CalculateColumnFormulas(OccupableStack* curstack, ColumnFormulas* formules, BaseTaskInfo* context, Bittab* filtre, VDB4DProgressIndicator* InProgress)
{
	VError err = VE_OK;
	sLONG curpage = 0;
	Boolean stop = false;

	Occupy(curstack, true);

	Transaction* curtrans = GetCurrentTransaction(context);
	if (curtrans != nil && curtrans->ConcernIndex(entete))
	{
		XBOX::VString session_title;
		gs(1005,27,session_title);
		IndexKeyIterator iter(entete, filtre, context, InProgress, session_title, curstack);

		while (iter.NextKey(context, err))
		{
			err = formules->ExecuteOnOneBtreeKey(entete, iter.GetKey(), stop, context);
			if (stop || (err != VE_OK))
				break;
		}
	}
	else 
	{ 
		err = ChargeFirstPage(curstack, context, false);
		if (firstpage!=nil && err == VE_OK)
		{
			XBOX::VString session_title;
			gs(1005,6,session_title);	// Indexed Sort: %curValue of %maxValue Keys
			ProgressEncapsuleur progress(InProgress, IHD.nbpage, session_title, true);
			err = firstpage->CalculateColumnFormulas(curstack, formules, context, filtre, curpage, &progress, stop);
			firstpage->Free(curstack, true);
			if (InProgress != nil)
			{
				InProgress->EndSession();
			}
		}
	}

	Free(curstack, true);

	return err;
}



VError IndexHeaderBTree::CalculateMin(OccupableStack* curstack, BaseTaskInfo* context, Bittab* filtre, VDB4DProgressIndicator* InProgress, VValueSingle* &result)
{
	VError err = VE_OK;
	sLONG curpage = 0;
	Boolean stop = false;
	result = nil;

	Occupy(curstack, true);

	Transaction* curtrans = GetCurrentTransaction(context);
	if (curtrans != nil && curtrans->ConcernIndex(entete))
	{
		XBOX::VString session_title;
		gs(1005,27,session_title);
		IndexKeyIterator iter(entete, filtre, context, InProgress, session_title, curstack);

		if (iter.NextKey(context, err))
		{
			result = entete->CreateVValueWithKey(iter.GetKey(), err);
		}
	}
	else 
	{ 
		err = ChargeFirstPage(curstack, context, false);
		if (firstpage!=nil && err == VE_OK)
		{
			XBOX::VString session_title;
			gs(1005,6,session_title);	// Indexed Sort: %curValue of %maxValue Keys
			ProgressEncapsuleur progress(InProgress, IHD.nbpage, session_title, true);
			err = firstpage->CalculateMin(curstack, context, filtre, curpage, &progress, stop, result);
			firstpage->Free(curstack, true);
		}
	}

	Free(curstack, true);

	return err;
}


VError IndexHeaderBTree::CalculateMax(OccupableStack* curstack, BaseTaskInfo* context, Bittab* filtre, VDB4DProgressIndicator* InProgress, VValueSingle* &result)
{
	VError err = VE_OK;
	sLONG curpage = 0;
	Boolean stop = false;
	result = nil;

	Occupy(curstack, true);

	Transaction* curtrans = GetCurrentTransaction(context);
	if (curtrans != nil && curtrans->ConcernIndex(entete))
	{
		XBOX::VString session_title;
		gs(1005,27,session_title);
		IndexKeyIterator iter(entete, filtre, context, InProgress, session_title, curstack);

		if (iter.PreviousKey(context, err))
		{
			result = entete->CreateVValueWithKey(iter.GetKey(), err);
		}
	}
	else 
	{ // ce code est amene a disparaitre, je le garde encore un peu par securite
		err = ChargeFirstPage(curstack, context, false);
		if (firstpage!=nil && err == VE_OK)
		{
			XBOX::VString session_title;
			gs(1005,6,session_title);	// Indexed Sort: %curValue of %maxValue Keys
			ProgressEncapsuleur progress(InProgress, IHD.nbpage, session_title, true);
			err = firstpage->CalculateMax(curstack, context, filtre, curpage, &progress, stop, result);
			firstpage->Free(curstack, true);
		}
	}

	Free(curstack, true);

	return err;
}


VError IndexHeaderBTree::GetDistinctValues(OccupableStack* curstack, DB4DCollectionManager& outCollection, BaseTaskInfo* context, Bittab* filtre, 
										   VDB4DProgressIndicator* InProgress, VCompareOptions& inOptions)
{
	VError err = VE_OK;
	BTitemIndex* curkey = nil;
	sLONG curpage = 0, curvalue = 0;

	char keyprealloc[KeyPreAllocSize];
	void* prealloc = &keyprealloc;
	distinctvalue_iterator xx;

	Occupy(curstack, true);

	Transaction* curtrans = GetCurrentTransaction(context);
	if (curtrans != nil && curtrans->ConcernIndex(entete))
	{
		XBOX::VString session_title;
		gs(1005,7,session_title);	// Indexed Distinct Values: %curValue of %maxValue Keys
		IndexKeyIterator iter(entete, filtre, context, InProgress, session_title, curstack);
		BTitemIndex* curkey = nil;

		sLONG curvalue = 0;
		while (iter.NextKey(context, err))
		{
			Boolean add = false;
			if (curkey != nil)
			{
				if (entete->CompareKeys(iter.GetKey(), curkey, inOptions) != CR_EQUAL)
				{
					entete->FreeKey(curkey, prealloc);
					curkey = entete->CopyKey(iter.GetKey(), prealloc);
					add = true;
				}
			}
			else
			{
				add = true;
				curkey = entete->CopyKey(iter.GetKey(), prealloc);
			}

			if (add)
			{
				ValPtr cv = entete->CreateVValueWithKey(curkey, err);
				if (err == VE_OK)
				{
					err = EnlargeCollection(*cv, outCollection, xx);
					//err = outCollection.AddOneElement(1, *cv);
					curvalue++;
					delete cv;
				}
			}
		}
		
		if (curkey != nil)
			entete->FreeKey(curkey, prealloc);

		outCollection.SetCollectionSize(xx.nbelem, false);
	}
	else 
	{ 
		if (err == VE_OK)
			err = ChargeFirstPage(curstack, context, false);
		if (firstpage!=nil && err == VE_OK)
		{
			XBOX::VString session_title;
			gs(1005,7,session_title);	// Indexed Distinct Values: %curValue of %maxValue Keys
			ProgressEncapsuleur progress(InProgress, IHD.nbpage,session_title,true);
			err = firstpage->GetDistinctValues(curstack, outCollection, context, filtre, curpage, curvalue, curkey, &progress, inOptions, prealloc, xx);
			if (curkey != nil)
			{
				entete->FreeKey(curkey, prealloc);
			}
			firstpage->Free(curstack, true);
			outCollection.SetCollectionSize(xx.nbelem, false);
		}
	}

	Free(curstack, true);

	return err;
}



VError IndexHeaderBTree::QuickGetDistinctValues(OccupableStack* curstack, QuickDB4DArrayOfValues *outCollection, BaseTaskInfo* context, Bittab* filtre, 
										   VDB4DProgressIndicator* InProgress, VCompareOptions& inOptions)
{
	VError err = VE_OK;
	BTitemIndex* curkey = nil;
	sLONG curpage = 0, curvalue = 0;

	char keyprealloc[KeyPreAllocSize];
	void* prealloc = &keyprealloc;

	Occupy(curstack, true);

	Transaction* curtrans = GetCurrentTransaction(context);
	if (curtrans != nil && curtrans->ConcernIndex(entete))
	{
		XBOX::VString session_title;
		gs(1005,7,session_title);	// Indexed Distinct Values: %curValue of %maxValue Keys
		IndexKeyIterator iter(entete, filtre, context, InProgress, session_title, curstack);
		BTitemIndex* curkey = nil;

		sLONG curvalue = 0;
		while (iter.NextKey(context, err))
		{
			Boolean add = false;
			if (curkey != nil)
			{
				if (entete->CompareKeys(iter.GetKey(), curkey, inOptions) != CR_EQUAL)
				{
					entete->FreeKey(curkey, prealloc);
					curkey = entete->CopyKey(iter.GetKey(), prealloc);
					add = true;
				}
			}
			else
			{
				add = true;
				curkey = entete->CopyKey(iter.GetKey(), prealloc);
			}

			if (add)
			{
				if (!outCollection->AddOneElemPtr((void*)&curkey->data))
					err = memfull;
				curvalue++;
			}
		}

		if (curkey != nil)
			entete->FreeKey(curkey, prealloc);
	}
	else 
	{ 
		if (err == VE_OK)
			err = ChargeFirstPage(curstack, context, false);
		if (firstpage!=nil && err == VE_OK)
		{
			XBOX::VString session_title;
			gs(1005,7,session_title);	// Indexed Distinct Values: %curValue of %maxValue Keys
			ProgressEncapsuleur progress(InProgress, IHD.nbpage,session_title,true);
			err = firstpage->QuickGetDistinctValues(curstack, outCollection, context, filtre, curpage, curvalue, curkey, &progress, inOptions, prealloc);
			if (curkey != nil)
			{
				entete->FreeKey(curkey, prealloc);
			}
			firstpage->Free(curstack, true);
		}
	}

	Free(curstack, true);

	return err;
}


Selection* IndexHeaderBTree::SortSel(OccupableStack* curstack, Selection* sel, uBOOL ascent, BaseTaskInfo* context, VError& err, 
									 VDB4DProgressIndicator* InProgress, Boolean TestUnicite, Bittab* inWafSelbits, WafSelection* outWafSel)
{
	Bittab *bb;
	Selection* sel2;
	sLONG nb, curfic;
	err = VE_OK;

	Occupy(curstack, true);
	sel2=nil;
	
	bb=sel->GenereBittab(context, err);
	
	if (bb!=nil)
	{
		nb=sel->GetQTfic();
		if (nb>kNbElemInSel)
		{
			sel2=new LongSel(sel->GetParentFile());
			((LongSel*)sel2)->PutInCache();
		}
		else
		{
			sel2=new PetiteSel(sel->GetParentFile());
		}
		
		if (sel2!=nil)
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
		
		VCompareOptions options = entete->GetStrictOptions();
		options.SetIntlManager(GetContextIntl(context));

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
			if (trans != nil && trans->ConcernIndex(entete))
			{
				XBOX::VString session_title;
				gs(1005,8,session_title);	// Indexed Sort: %curValue of %maxValue Keys
				IndexKeyIterator iter(entete, bb, context, InProgress, session_title, curstack);
				BTitemIndex* curkey = nil;

				while (iter.NextKey(context, err, &curfic))
				{
					if (TestUnicite)
					{
						if (curkey != nil)
						{
							if (entete->CompareKeys(curkey, iter.GetKey(), options ) == CR_EQUAL)
							{
								err = VE_DB4D_DUPLICATED_KEY;
								break;
							}
							entete->FreeKey(curkey);
							curkey = nil;
						}
						curkey = entete->CopyKey(iter.GetKey());
						if (curkey == nil)
						{
							err = ThrowError(memfull, DBaction_SortingOnIndex);
							break;
						}
					}
					assert(curfic < nb);
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

				if (curkey != nil)
					entete->FreeKey(curkey);
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
					BTitemIndex* curkey = nil;
					err = firstpage->SortSel(curstack, curfic,nb,bb,sel2,ascent, context, &progress, TestUnicite, curkey, options, inWafSelbits, outWafSel);
					if (curkey != nil)
						entete->FreeKey(curkey);
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


VError IndexHeaderBTree::ChargeFirstPage(OccupableStack* curstack, BaseTaskInfo* context, uBOOL doiscreer)
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
				firstpage=BTreePageIndex::AllocatePage(curstack, entete, context, err);
				if (firstpage != nil)
				{
					HBT.FirstPage=firstpage->GetNum();
					//setmodif(true);
					taddr.setmodif(true, entete->GetDB(), context);
					// GetAssoc()->ModifyIt();
				}
			}
		}
		else
		{
			VTaskLock lock(&fLoadPageMutex);
			if (firstpage == nil)
				firstpage=BTreePageIndex::LoadPage(curstack, HBT.FirstPage, entete, context, err);
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


void IndexHeaderBTree::SetFirstPage(BTreePageIndex* first, BaseTaskInfo* context) 
{ 
	firstpage = first;
	if (first == nil)
	{
		HBT.FirstPage = -1;
	}
	else
	{
		HBT.FirstPage = firstpage->GetNum();
	}
	taddr.setmodif(true, entete->GetDB(), context);
}


VError IndexHeaderBTree::PlaceCle(OccupableStack* curstack, BTitemIndex *cle, sLONG xqui, IndexKeyArray& tempkeys, BaseTaskInfo* context)
{
	BTreePageIndex* pag;
	uBOOL h;
	BTitemIndex* u;
	VError err;
	//Transaction *curtrans = GetCurTrans(context);

	Occupy(curstack, true);

#if debuglr == 115
	CheckAllPageKeys();
#endif
#if debuglr == 116
	CheckAllPageOwner();
#endif

	CHECKFLUSH;
	u=cle;

	err=VE_OK;

	VCompareOptions options = entete->GetStrictOptions();
	options.SetIntlManager(GetContextIntl(context));

	// new mechanism , occupe();
	//taddr.occupe();
	
	//if (taddr.AccesModifOK(context))
	if (u->IsNull())
	{
		err = AddToNulls(curstack, xqui, context);
	}
	else
	{
		u->sousBT=nil;
		u->souspage=-1;
		u->SetQui(xqui);
		h=false;
		
		//SaveIHDForTrans(context);
		err = ChargeFirstPage(curstack, context, true);
		
		if (err == VE_OK)
		{
			err = firstpage->Place(curstack, &h,u, tempkeys,context, options);
			
	#if debuglr == 112
			ClearPageInfoDebug();
	#endif
			
			if (err != VE_OK)
			{
				firstpage->Free(curstack, true);
			}
			else
			{
				if (h)
				{
					pag=BTreePageIndex::AllocatePage(curstack, entete, context, err);
					if (pag != nil)
					{
						pag->SetSousPage(0,firstpage);
						pag->InsertKey(0, u, true);
						u->sousBT->SetEncours(false);
						u->sousBT->Free(curstack, true);
						firstpage->Free(curstack, true);
						
						firstpage=pag;
						HBT.FirstPage=pag->GetNum();
						taddr.setmodif(true, entete->GetDB(), context);
						err = pag->savepage(curstack, context);
						pag->Free(curstack, true);
					}
				}
				else
				{
					firstpage->Free(curstack, true);
				}
			}
		}
#if debuglr == 112
		CheckPageInfoDebug();
#endif
	}
	CHECKFLUSH;
#if debuglr == 115
	CheckAllPageKeys();
#endif
#if debuglr == 116
	CheckAllPageOwner();
#endif
	Free(curstack, true);
	
	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTINSERTKEYINTOINDEX, DBaction_InsertingKeyIntoIndex);
	return(err);
}


VError IndexHeaderBTree::DetruireCle(OccupableStack* curstack, BTitemIndex *cle, sLONG xqui, IndexKeyArray& tempkeys, BaseTaskInfo* context)
{
	uBOOL h;
	BTitemIndex* u;
	VError err;
	uBOOL locdoisdel;
	//Transaction *curtrans = GetCurTrans(context);
	Occupy(curstack, true);
	
#if debuglr == 115
	CheckAllPageKeys();
#endif
#if debuglr == 116
	CheckAllPageOwner();
#endif
#if debugplus
	CHECKMEM;
	CHECKFLUSH;
#endif
	err=VE_OK;

	VCompareOptions options = entete->GetStrictOptions();
	options.SetIntlManager(GetContextIntl(context));

#if debuglr
	debug_underflow_has_been_executed = false;
#endif

	u = cle;
	if (u->IsNull())
	{
		err = DelFromNulls(curstack, xqui, context);
	}
	else
	{
	
		u->sousBT=nil;
		u->souspage=-1;
		u->SetQui(xqui);
		h=false;
		
		err = ChargeFirstPage(curstack, context, false);
		
#if debuglr == 112
		ClearPageInfoDebug();
#endif

		if (firstpage!=nil)
		{
#if debugIndexPage
			entete->debug_ClearPageList();
#endif

			if (err == VE_OK) 
				err = firstpage->Detruire(curstack, &h,u,&locdoisdel, tempkeys, context, options);

#if debugIndexPage
			{
				debug_ListOfPages* pages = entete->debug_GetPageList();
				debug_ListOfPages::const_iterator cur = pages->begin(), end = pages->end();
				for (; cur != end; cur++)
				{
					const BTreePageIndex* p = *cur;
					if (p != firstpage)
					{
						assert(p->GetNKeys() >= HalfPage);
						//assert(p->GetCountOccupe() == 0);
					}
					/*
					else
						assert(p->GetCountOccupe() == 1);
						*/
				}
			}
#endif
			
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
						BTreePageIndex* sousbt = firstpage->GetSousPage(curstack, 0, err, context);
						firstpage->LiberePage(curstack, context);
						firstpage->Free(curstack, true);
						firstpage->ReleaseAndCheck();
						firstpage=sousbt;
						if (firstpage != nil)
						{
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
#if debuglr == 112
		CheckPageInfoDebug();
#endif
	}
	
#if debuglr == 116
	CheckAllPageOwner();
#endif
#if debuglr == 115
	CheckAllPageKeys();
#endif
	Free(curstack, true);

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTDELETEKEYFROMINDEX, DBaction_DeletingKeyFromIndex);
	return(err);
}


VError IndexHeaderBTree::LibereEspaceDisk(OccupableStack* curstack, VDB4DProgressIndicator* InProgress)
{
	if (firstpage != nil) 
	{
		firstpage->DelFromFlush(curstack);
	}
	return IndexHeaderBTreeRoot::LibereEspaceDisk(curstack, InProgress);
}


void IndexHeaderBTree::LibereEspaceMem(OccupableStack* curstack)
{
	Occupy(curstack, true);
	if (firstpage != nil) 
	{
		firstpage->LibereEspaceMem(curstack);
		//assert(firstpage->GetRefCount() == 1);
		firstpage->ReleaseAndCheck(true);
	}
	firstpage=nil;
	IndexHeaderBTreeRoot::LibereEspaceMem(curstack);
	Free(curstack, true);
}


Bittab* IndexHeaderBTree::ParseAll(OccupableStack* curstack, BTitemIndex* val1, BaseTaskInfo* context, VError& err, const VCompareOptions& inOptions, VDB4DProgressIndicator* InProgress)
{
	Occupy(curstack, true);
	Bittab* b = new Bittab;
	if (b!=nil)
	{
		err = ChargeFirstPage(curstack, context, false);
		if (firstpage!=nil && err == VE_OK)
		{
			VString session_title;
			gs(1005,29,session_title);
			ProgressEncapsuleur progress(InProgress, IHD.nbpage, session_title, true);
			err = firstpage->ParseAll(curstack, b, val1, context, inOptions, &progress);
			firstpage->Free(curstack, true);
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
	return b;
}


Bittab* IndexHeaderBTree::Fourche(OccupableStack* curstack, BTitemIndex* val1, uBOOL xstrict1, BTitemIndex* val2, uBOOL xstrict2, BaseTaskInfo* context, VError& err, 
									VDB4DProgressIndicator* InProgress, const VCompareOptions& inOptions, Bittab* dejasel, BTitemIndex** outVal)
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
				VString session_title;
				gs(1005,28,session_title);
				ProgressEncapsuleur progress(InProgress, IHD.nbpage, session_title, true);

				err = firstpage->Fourche(curstack, b, val1, xstrict1, val2, xstrict2, context, &progress, inOptions, outVal);
				firstpage->Free(curstack, true);
			}
#if 0 && VERSIONDEBUG
			// ##special test##
			if (entete->GetDB()->GetStructure() != nil && b->Compte() == 0)
			{
				b = b;
			}
#endif
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
	return(b);
}



Bittab* IndexHeaderBTree::FindKeyInArray(OccupableStack* curstack, DB4DArrayOfValues* values, BaseTaskInfo* context, VError& err, VDB4DProgressIndicator* InProgress, const VCompareOptions& inOptions, Bittab* dejasel)
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
				VString session_title;
				gs(1005,28,session_title);
				ProgressEncapsuleur progress(InProgress, IHD.nbpage, session_title, true);

				const void* curval = ((DB4DArrayOfConstValues*)values)->GetFirstPtr();
				err = firstpage->FindKeyInArray(curstack, b, (DB4DArrayOfConstValues*)values, curval, context, inOptions, &progress);
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
	return(b);
}


sLONG IndexHeaderBTree::FindKey(OccupableStack* curstack, BTitemIndex* val, BaseTaskInfo* context, const VCompareOptions& inOptions, Bittab* dejasel, BTitemIndex** outVal)
{
	Bittab *b = nil;
	sLONG n = -1;
	VError err = VE_OK;
	Bittab dejasel2;

	Occupy(curstack, true);
	Transaction* trans = GetCurrentTransaction(context);
	if (trans != nil && outVal != nil)
	{
		trans->RemoveDeleteKeysFromFourche(entete, val, false, val, false, inOptions, &dejasel2);
	}

	b=Fourche(curstack, val, false, val, false, context, err, nil, inOptions, dejasel, outVal);
	if (err == VE_OK)
	{
		if (trans != nil)
		{
			BTitemIndex* transval = nil, **xtransval = nil;
			if (outVal != nil)
				xtransval = &transval;
			
			assert(b != nil);
			err = trans->Fourche(entete, val, false, val, false, inOptions, dejasel, b, xtransval);
			
			if (outVal != nil)
			{
				if (transval != nil)
				{
					if (*outVal != nil)
						entete->FreeKey(*outVal);
					*outVal = transval;
				}
				
				err = b->moins(&dejasel2);
			}
		}
		n=b->FindNextBit(0);
	}
	ReleaseRefCountable(&b);
	Free(curstack, true);
	return(n);
}


VError IndexHeaderBTree::FindKeyAndBuildPath(OccupableStack* curstack, BTitemIndex* val, const VCompareOptions& inOptions, BaseTaskInfo* context, VDB4DIndexKey* outKey)
{
	VError err;
	Boolean trouve;

	Occupy(curstack, true);
	err = ChargeFirstPage(curstack, context, false);
	if (firstpage!=nil && err == VE_OK)
	{
		err = firstpage->FindKeyAndBuildPath(curstack, val, inOptions, context, outKey, trouve);
		firstpage->Free(curstack, true);
	}
	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTCOMPLETESCANOFINDEX, DBaction_SearchingInIndex);
	Free(curstack, true);
	return err;
}


VError IndexHeaderBTree::NextKey(OccupableStack* curstack, const VDB4DIndexKey* inKey, BaseTaskInfo* context, VDB4DIndexKey* outKey)
{
	VError err;
	Boolean outlimit;

	Occupy(curstack, true);
	err = ChargeFirstPage(curstack, context, false);
	if (firstpage!=nil && err == VE_OK)
	{
		outlimit = false;
		err = firstpage->NextKey(curstack, inKey, 0, context, outKey, outlimit);
		firstpage->Free(curstack, true);
	}
	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTCOMPLETESCANOFINDEX, DBaction_SearchingInIndex);
	Free(curstack, true);
	return err;
}

#if debuglr

void IndexHeaderBTree::CheckAllPageKeys()
{
	if (firstpage != nil)
		firstpage->CheckPageKeys();
}

void IndexHeaderBTree::CheckAllPageOwner()
{
	if (firstpage != nil)
		firstpage->CheckPageOwner();
}


#endif


VError IndexHeaderBTree::ScanIndex(OccupableStack* curstack, Selection* into, sLONG inMaxRecords, Boolean KeepSorted, Boolean Ascent, BaseTaskInfo* context, Bittab* filtre,sLONG& nbtrouves, VDB4DProgressIndicator* InProgress)
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


BTreePageIndex* IndexHeaderBTree::CrePage(void)
{
	BTreePageIndex* res = nil;
	sLONG len = BTreePageIndex::CalculateDefaultLen(GetAssoc());

	void* p = GetFastMem(len, true, 'iPag');
	if (p == nil)
	{
		// thowerr
	}
	else
	{
		res=new (p)BTreePageIndex(GetAssoc(), len, -1);
	}
	return(res);
}


BTreePageIndex* IndexHeaderBTree::LoadPage(OccupableStack* curstack, DataAddr4D addr, sLONG len, BaseTaskInfo* context, VError& err, sLONG xnum)
{
	err = VE_OK;
	BTreePageIndex* res = nil;
	
	sLONG lenInMem = BTreePageIndex::CalculateDefaultLen(GetAssoc());
	sLONG lenInMemFromDisk = len + BTreePageIndex::CalculateEmptyLen() - sizeof(BTreePageIndexDisk);

	if (lenInMemFromDisk > lenInMem)
		lenInMem = lenInMemFromDisk;

	void* p = GetFastMem(lenInMem, true, 'iPag');
	if (p == nil)
	{
		err = ThrowError(memfull, DBaction_LoadingIndexPage);
	}
	else
	{
		res=new (p)BTreePageIndex(GetAssoc(), lenInMem, xnum);
		res->Occupy(curstack, true);
		err = res->loadobj(addr, len);
		if (err != VE_OK)
		{
			res->Free(curstack, true);
			res->Release();
			res = nil;
		}
	}

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTLOADINDEXPAGE, DBaction_LoadingIndexPage);

	return(res);
}



IndexHeader* CreIndexHeaderBTree(IndexInfo *xentete)
{
	IndexHeaderBTree *head;
	
	head=new IndexHeaderBTree(xentete);
	return(head);
}


		      /* ------------------------------------------------------------------------------------------------  */
					



BTreePageIndex::BTreePageIndex(IndexInfo* xentete, sLONG datasize, sLONG xnum, sWORD DefaultAccess)/*:ObjCacheInTree(DefaultAccess)*/
{
	//btp=nil;
	entete=xentete;
	oldlen=0;
	num=xnum;
	encours=false;
	IsCluster=false;
	//original = nil;
	//origaddr = 0;
	fSizeKeyRemoved = 0;
	//fStopWrite = 0;
	btp = &fBTPdata;
	btp->TrueDataSize = BTreePageIndex::CalculateEmptyLenOnDisk();
	btp->CurrentSizeInMem = btp->TrueDataSize;
	fDataSize = datasize - BTreePageIndex::CalculateEmptyLen() + btp->TrueDataSize;
	btp->nkeys = 0;
	btp->souspage0 = -1;
	fCurrentSizeToWrite = btp->CurrentSizeInMem;
	tabmem[0] = nil;
	fDebugVerify = true;
#if debuglr_page
	debug_addpage(this);
#endif
}


#if debugCheckIndexPageOnDiskInDestructor

void BTreePageIndex::DebugCheckPageOnDisk()
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
				err = tag.ValidateTag(DBOH_BtreePage, num, -2);
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

BTreePageIndex::~BTreePageIndex()
{
#if debug_checkRelaseIndexPage
	if (gOccPool.FindObjectInStack(dynamic_cast<IOccupable*>(this)))
	{
		sLONG xdebug = 1; // put a break here
		xbox_assert(false);
	}

#endif

#if debuglr_page
	debug_suppage(this);
#endif

#if debugIndexPage
	entete->debug_DelPage(this);
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
			err = tag.ValidateTag(DBOH_BtreePage, num, -2);
		}
		assert(err == VE_OK);
		if (err != VE_OK)
		{
			addr = addr; // put a break here
		}
	}
#endif

	sLONG i,nb;
#if debuglr
	if (btp->souspage0 != -1 && fDebugVerify)
	{
		for (i = 0; i <= btp->nkeys; i++)
		{
			if (tabmem[i] != nil)
			{
				assert(tabmem[i] == nil);
			}
		}
	}
#endif

	if (btp != &fBTPdata)
	{
		FreeFastMem((void*)btp);
	}
}


VError BTreePageIndex::ThrowError( VError inErrCode, ActionDB4D inAction)
{
	if (entete != nil)
		return entete->ThrowError(inErrCode, inAction);
	else
		return inErrCode;
}


sLONG BTreePageIndex::CalculateDefaultLen(IndexInfo *xentete) 
{ 
	sLONG res = CalculateEmptyLen() + ((sLONG)BTitemIndex::CalculateEmptyLen() + xentete->CalculateDefaultSizeKey()) * kNbKeyParPageIndex;
	/*
	if (res < adjusteindex(res))
		res = adjusteindex(res);
		*/
	return res;
}


VError BTreePageIndex::CalculateColumnFormulas(OccupableStack* curstack, ColumnFormulas* formules, BaseTaskInfo* context, Bittab* filtre, sLONG &curpage, 
																							 ProgressEncapsuleur* InProgress, Boolean& stop)
{
	BTreePageIndex *sousBT;
	sLONG i,nb,l;
	VError err = VE_OK;

	if (InProgress != nil)
	{
		if (!InProgress->Progress(curpage))
			err = ThrowError(VE_DB4D_ACTIONCANCELEDBYUSER, DBaction_CalculateFomulasWithIndex);
	}

	nb = btp->nkeys;

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
				BTitemIndex* u = GetItemPtr(i);

				l = GetQui(i);
				if (IsCluster)
				{
					if (l>=0)
					{
						if (filtre == nil || filtre->isOn(l))
						{
							err = CalculateColumnFormulasOnOneKey(formules, u, stop, context);
							if (stop)
								break;
						}
					}
					else
					{
						err = entete->GetClusterSel(curstack)->CalculateColumnFormulasOnCluster(curstack, formules, u, l, filtre, context, stop, this);
						if (stop)
							break;
					}
				}
				else
				{
					if (filtre == nil || filtre->isOn(l))
					{
						err = CalculateColumnFormulasOnOneKey(formules, u, stop, context);
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



VError BTreePageIndex::CalculateMin(OccupableStack* curstack, BaseTaskInfo* context, Bittab* filtre, sLONG &curpage, 
																		ProgressEncapsuleur* InProgress, Boolean& stop, VValueSingle* &result)
{
	BTreePageIndex *sousBT;
	sLONG i,nb,l;
	VError err = VE_OK;

	if (InProgress != nil)
	{
		if (!InProgress->Progress(curpage))
			err = ThrowError(VE_DB4D_ACTIONCANCELEDBYUSER, DBaction_CalculateFomulasWithIndex);
	}

	nb = btp->nkeys;

	if (err == VE_OK)
	{
		sousBT = GetSousPage(curstack, 0, err, context);
		if (sousBT != nil && err == VE_OK)
		{
			curpage++;
			err = sousBT->CalculateMin(curstack, context, filtre, curpage, InProgress, stop, result);
			sousBT->Free(curstack, true);
		}

		if (!stop)
		{
			for (i=0;i<nb && err == VE_OK;i++)
			{
				l = GetQui(i);
				if (IsCluster)
				{
					if (filtre == nil || entete->GetClusterSel(curstack)->IntersectSel(curstack, l, filtre, context, err))
					{
						BTitemIndex* u = GetItemPtr(i);
						result = entete->CreateVValueWithKey(u, err);
						break;
					}
				}
				else
				{
					if (filtre == nil || filtre->isOn(l))
					{
						BTitemIndex* u = GetItemPtr(i);
						result = entete->CreateVValueWithKey(u, err);
						break;
					}
				}

				if (!stop && err == VE_OK)
				{
					sousBT = GetSousPage(curstack, i+1, err, context);
					if (sousBT != nil && err == VE_OK)
					{
						curpage++;
						err = sousBT->CalculateMin(curstack, context, filtre, curpage, InProgress, stop, result);
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




VError BTreePageIndex::CalculateMax(OccupableStack* curstack, BaseTaskInfo* context, Bittab* filtre, sLONG &curpage, 
																		ProgressEncapsuleur* InProgress, Boolean& stop, VValueSingle* &result)
{
	BTreePageIndex *sousBT;
	sLONG i,nb,l;
	VError err = VE_OK;

	if (InProgress != nil)
	{
		if (!InProgress->Progress(curpage))
			err = ThrowError(VE_DB4D_ACTIONCANCELEDBYUSER, DBaction_CalculateFomulasWithIndex);
	}

	nb = btp->nkeys;

	if (err == VE_OK)
	{
		for (i=nb-1; i>=0 && err == VE_OK; i--)
		{
			sousBT = GetSousPage(curstack, i+1, err, context);
			if (sousBT != nil && err == VE_OK)
			{
				curpage++;
				err = sousBT->CalculateMax(curstack, context, filtre, curpage, InProgress, stop, result);
				sousBT->Free(curstack, true);
				if (stop)
					break;
			}

			if (!stop && err == VE_OK)
			{
				l = GetQui(i);
				if (IsCluster)
				{
					if (filtre == nil || entete->GetClusterSel(curstack)->IntersectSel(curstack, l, filtre, context, err))
					{
						BTitemIndex* u = GetItemPtr(i);
						result = entete->CreateVValueWithKey(u, err);
						stop = true;
						break;
					}
				}
				else
				{
					if (filtre == nil || filtre->isOn(l))
					{
						BTitemIndex* u = GetItemPtr(i);
						result = entete->CreateVValueWithKey(u, err);
						stop = true;
						break;
					}

				}
			}

		}


		if (!stop && err == VE_OK)
		{
			sousBT = GetSousPage(curstack, 0, err, context);
			if (sousBT != nil && err == VE_OK)
			{
				curpage++;
				err = sousBT->CalculateMax(curstack, context, filtre, curpage, InProgress, stop, result);
				sousBT->Free(curstack, true);
			}
		}
	}

	return err;
}


VError BTreePageIndex::CalculateColumnFormulasOnOneKey(ColumnFormulas* formules, BTitemIndex* u, Boolean& stop, BaseTaskInfo* context)
{
	return formules->ExecuteOnOneBtreeKey(GetEntete(), u, stop, context);
}


VError BTreePageIndex::GetDistinctValues(OccupableStack* curstack, DB4DCollectionManager& outCollection, BaseTaskInfo* context, Bittab* filtre, 
										sLONG &curpage, sLONG &curvalue, BTitemIndex* &curkey, ProgressEncapsuleur* InProgress, 
										VCompareOptions& inOptions, void* prealloc, distinctvalue_iterator& xx)
{
	VTask::Yield();
	BTreePageIndex *sousBT;
	sLONG i,nb,l;
	VError err = VE_OK;

	if (InProgress != nil)
	{
		if (!InProgress->Progress(curpage))
			err = ThrowError(VE_DB4D_ACTIONCANCELEDBYUSER, DBaction_GetDistinctValuesWithIndex);
	}

	nb = btp->nkeys;

	if (err == VE_OK)
	{
		sousBT = GetSousPage(curstack, 0, err, context);
		if (sousBT != nil && err == VE_OK)
		{
			curpage++;
			err = sousBT->GetDistinctValues(curstack, outCollection, context, filtre, curpage, curvalue, curkey, InProgress, inOptions, prealloc, xx);
			sousBT->Free(curstack, true);
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
					if (curkey != nil)
					{
						if (CompareKeys(i, curkey, inOptions) != CR_EQUAL)
						{
							entete->FreeKey(curkey, prealloc);
							curkey = CopyKey(i, prealloc);
							add = true;
						}
					}
					else
					{
						add = true;
						curkey = CopyKey(i, prealloc);
					}
				}

				if (add)
				{
					ValPtr cv = entete->CreateVValueWithKey(GetItemPtr(i), err);
					if (err == VE_OK)
					{
						//err = outCollection.AddOneElement(1, *cv);
						err = EnlargeCollection(*cv, outCollection, xx);
						curvalue++;
						delete cv;
					}
				}
				
			}
			else
			{
				if (filtre == nil || filtre->isOn(l))
				{
					if (curkey != nil)
					{
						if (CompareKeys(i, curkey, inOptions) != CR_EQUAL)
						{
							entete->FreeKey(curkey, prealloc);
							curkey = CopyKey(i, prealloc);
							add = true;
						}
					}
					else
					{
						add = true;
						curkey = CopyKey(i, prealloc);
					}

				}

				if (add)
				{
					ValPtr cv = entete->CreateVValueWithKey(curkey, err);
					if (err == VE_OK)
					{
						//err = outCollection.AddOneElement(1, *cv);
						err = EnlargeCollection(*cv, outCollection, xx);
						curvalue++;
						delete cv;
					}
				}

			}

			sousBT = GetSousPage(curstack, i+1, err, context);
			if (sousBT != nil && err == VE_OK)
			{
				curpage++;
				err = sousBT->GetDistinctValues(curstack, outCollection, context, filtre, curpage, curvalue, curkey, InProgress, inOptions, prealloc, xx);
				sousBT->Free(curstack, true);
			}

		}

	}

	return err;
}



VError BTreePageIndex::QuickGetDistinctValues(OccupableStack* curstack, QuickDB4DArrayOfValues *outCollection, BaseTaskInfo* context, Bittab* filtre, 
										 sLONG &curpage, sLONG &curvalue, BTitemIndex* &curkey, ProgressEncapsuleur* InProgress, 
										 VCompareOptions& inOptions, void* prealloc)
{
	VTask::Yield();
	BTreePageIndex *sousBT;
	sLONG i,nb,l;
	VError err = VE_OK;

	if (InProgress != nil)
	{
		if (!InProgress->Progress(curpage))
			err = ThrowError(VE_DB4D_ACTIONCANCELEDBYUSER, DBaction_GetDistinctValuesWithIndex);
	}

	nb = btp->nkeys;

	if (err == VE_OK)
	{
		sousBT = GetSousPage(curstack, 0, err, context);
		if (sousBT != nil && err == VE_OK)
		{
			curpage++;
			err = sousBT->QuickGetDistinctValues(curstack, outCollection, context, filtre, curpage, curvalue, curkey, InProgress, inOptions, prealloc);
			sousBT->Free(curstack, true);
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
					if (curkey != nil)
					{
						if (CompareKeys(i, curkey, inOptions) != CR_EQUAL)
						{
							entete->FreeKey(curkey, prealloc);
							curkey = CopyKey(i, prealloc);
							add = true;
						}
					}
					else
					{
						add = true;
						curkey = CopyKey(i, prealloc);
					}
				}

				if (add)
				{			
					if (!outCollection->AddOneElemPtr((void*)&(GetItemPtr(i)->data)))
						err = memfull;
					curvalue++;		
				}

			}
			else
			{
				if (filtre == nil || filtre->isOn(l))
				{
					if (curkey != nil)
					{
						if (CompareKeys(i, curkey, inOptions) != CR_EQUAL)
						{
							entete->FreeKey(curkey, prealloc);
							curkey = CopyKey(i, prealloc);
							add = true;
						}
					}
					else
					{
						add = true;
						curkey = CopyKey(i, prealloc);
					}

				}

				if (add)
				{
					if (!outCollection->AddOneElemPtr((void*)&(curkey->data)) )
						err = memfull;
					curvalue++;		
				}

			}

			sousBT = GetSousPage(curstack, i+1, err, context);
			if (sousBT != nil && err == VE_OK)
			{
				curpage++;
				err = sousBT->QuickGetDistinctValues(curstack, outCollection, context, filtre, curpage, curvalue, curkey, InProgress, inOptions, prealloc);
				sousBT->Free(curstack, true);
			}

		}

	}

	return err;
}



VError BTreePageIndex::SortSel(OccupableStack* curstack, sLONG& curfic, sLONG maxnb, Bittab* filtre, Selection* sel, uBOOL ascent, BaseTaskInfo* context, 
									ProgressEncapsuleur* InProgress, Boolean TestUnicite, BTitemIndex* &curkey, const VCompareOptions& inOptions, Bittab* inWafSelbits, WafSelection* outWafSel)
{
	VTask::Yield();
	BTreePageIndex *sousBT;
	sLONG i,nb,l;
	VError err = VE_OK;
	
	
	if (InProgress != nil)
	{
		if (!InProgress->Progress(curfic))
			err = ThrowError(VE_DB4D_ACTIONCANCELEDBYUSER, DBaction_SortingWithIndex);
	}
	
	nb = btp->nkeys;
	
	if (err == VE_OK)
	{
		if (ascent)
		{
			sousBT = GetSousPage(curstack, 0, err, context);
			if (sousBT != nil && err == VE_OK)
			{
				err = sousBT->SortSel(curstack, curfic, maxnb, filtre, sel, ascent, context, InProgress, TestUnicite, curkey, inOptions, inWafSelbits, outWafSel);
				sousBT->Free(curstack, true);
			}
			
			for (i=0;i<nb && err == VE_OK;i++)
			{
				if (TestUnicite)
				{
					if (curkey != nil)
					{
						if (CompareKeys(i, curkey, inOptions) != CR_EQUAL)
						{
							entete->FreeKey(curkey);
							curkey = CopyKey(i);
						}
						else
						{
							// failed unicity test
							err = VE_DB4D_DUPLICATED_KEY;
							break;
						}
					}
					else
						curkey = CopyKey(i);
				}

				l = GetQui(i);
				if (IsCluster)
				{
					if (TestUnicite)
					{
						if (l < 0)
						{
							// on pointe sur un cluster donc pas unique
							err = VE_DB4D_DUPLICATED_KEY;
						}
					}
					else
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
						err = sousBT->SortSel(curstack, curfic, maxnb, filtre, sel, ascent, context, InProgress, TestUnicite, curkey, inOptions, inWafSelbits, outWafSel);
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
					err = sousBT->SortSel(curstack, curfic, maxnb, filtre, sel, ascent, context, InProgress, TestUnicite, curkey, inOptions, inWafSelbits, outWafSel);
					sousBT->Free(curstack, true);
				}

				if (err == VE_OK)
				{
					if (TestUnicite)
					{
						if (curkey != nil)
						{
							if (CompareKeys(i, curkey, inOptions) != CR_EQUAL)
							{
								entete->FreeKey(curkey);
								curkey = CopyKey(i);
							}
							else
							{
								// failed unicity test
								err = VE_DB4D_DUPLICATED_KEY;
								break;
							}
						}
						else
							curkey = CopyKey(i);
					}

					l = GetQui(i);
					if (IsCluster)
					{
						if (TestUnicite)
						{
							if (l < 0)
							{
								// on pointe sur un cluster donc pas unique
								err = VE_DB4D_DUPLICATED_KEY;
							}
						}
						else
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
					err = sousBT->SortSel(curstack, curfic, maxnb, filtre, sel, ascent, context, InProgress, TestUnicite, curkey, inOptions, inWafSelbits, outWafSel);
					sousBT->Free(curstack, true);
				}
			}
			
		}
	}
	
	/* pas la peine car fait au retour du premier niveau d'appel
	
	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTCOMPLETESORTOFINDEX, DBaction_SortingWithIndex);
	*/
	
	if (err != VE_OK)
	{
		curfic = curfic;
		// put a break here
	}
	return err;
}

sLONG BTreePageIndex::GetNKeys() const
{
	sLONG n = btp->nkeys;
	return n;
}

BTitemIndex* BTreePageIndex::GetItem(OccupableStack* curstack, sLONG n, Boolean chargesous, IndexKeyArray &tempkeys, BaseTaskInfo* context, VError& err)
{
	err = VE_OK;
	BTitemIndex* u = GetItemPtr(n);
	sLONG len = entete->CalulateFullKeyLengthInMem(u);
	BTitemIndex* v = entete->AllocateKey(len);
	if (v != nil)
	{
		tempkeys.AddKey(v);
		vBlockMove(&u->souspage, &v->souspage, len - sizeof(BTitemIndex*) /* on ne copie pas le pointer de souspage */);
		sLONG p = v->souspage;
		if (p != -1 && chargesous)
		{
			gOccPool.WaitForMemoryManager(curstack);
			BTreePageIndex* sous=tabmem[n+1];
			if (sous==nil)
			{
				gOccPool.EndWaitForMemoryManager(curstack);
				/*  pas besoin car GetItem n'est utilisee que pour les acces exclusifs
				VTaskLock lock(&fLoadPageMutex);
				sous=tabmem[n+1];
				*/
				if (sous == nil)
				{
					sous=LoadPage(curstack, p, entete, context, err);
					tabmem[n+1]=sous;
				}
			}
			else
			{
				sous->Occupy(curstack, false);
				gOccPool.EndWaitForMemoryManager(curstack);
			}
			v->sousBT = sous;
		}
		else
			v->sousBT = nil;
	}
	else
		err = ThrowError(memfull, DBaction_LoadingIndexKey);

	return v;
}


BTitemIndex* BTreePageIndex::CopyKey(sLONG n, void* prealloc)
{
	BTitemIndex* u = GetItemPtr(n);
	sLONG len = entete->CalulateFullKeyLengthInMem(u);
	BTitemIndex* v;
	if (prealloc == nil || len >= KeyPreAllocSize)
		v = entete->AllocateKey(len);
	else
		v = (BTitemIndex*)prealloc;
	if (v != nil)
	{
		vBlockMove(&u->souspage, &v->souspage, len - sizeof(BTitemIndex*) /* on ne copie pas le pointer de souspage */);
		v->sousBT = nil;
	}

	return v;
}
BTitemIndex* BTreePageIndex::GetItemPtr(sLONG n)
{
	assert(n>=0 && n<btp->nkeys);
	char* x = (char*)btp/*.suite*/;
	x = x + btp->off[n] - sizeof(void*); // on retire la taile de sousBT qui n'est pas stockee dans la cle
	return (BTitemIndex*)x;
}

sLONG BTreePageIndex::GetQui(sLONG n)
{
	BTitemIndex* item = GetItemPtr(n);
	return item->GetQui();
}


void BTreePageIndex::SetQui(sLONG n, sLONG xqui)
{
	BTitemIndex* item = GetItemPtr(n);
	item->SetQui(xqui);
}


sLONG BTreePageIndex::GetSousPageNum(sLONG n)
{
	if (n == 0)
		return btp->souspage0;
	else
	{
		BTitemIndex* item = GetItemPtr(n-1);
		return item->souspage;
	}
}


BTreePageIndex* BTreePageIndex::GetSousPage(OccupableStack* curstack, sLONG n, VError &err, BaseTaskInfo* context)
{
	err = VE_OK;
	BTreePageIndex* sous;

	if (btp->souspage0 == -1)
		sous = nil;
	else
	{
		gOccPool.WaitForMemoryManager(curstack);
		sous = tabmem[n];
		if (sous==nil)
		{
			gOccPool.EndWaitForMemoryManager(curstack);
			if (n == 0)
			{
				if (btp->souspage0 != -1)
				{
					VTaskLock lock(&fLoadPageMutex);
					sous = tabmem[n];
					if (sous == nil)
					{
						sous=LoadPage(curstack, btp->souspage0, entete, context, err);
						tabmem[n]=sous;
					}
					else
						sous->Occupy(curstack, true);
				}
			}
			else
			{
				BTitemIndex* item = GetItemPtr(n-1);
				sLONG p = item->souspage;
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
		}
		else
		{
			sous->Occupy(curstack, false);
			gOccPool.EndWaitForMemoryManager(curstack);
		}
	}
	

	return(sous);
}


void BTreePageIndex::SetSousPage(sLONG n, BTreePageIndex* sousBT) 
{ 
	//ForbidWrite();
	tabmem[n]=sousBT;
	if (n == 0)
	{
		if (sousBT!=nil)
		{
			btp->souspage0 = sousBT->GetNum();
			if (btp->souspage0 == -1)
				btp->souspage0 = -2;
		}
		else
		{
			btp->souspage0 = -1;
		}
	}
	else
	{
		BTitemIndex* item = GetItemPtr(n-1);
		if (sousBT!=nil)
		{
			item->souspage = sousBT->GetNum();
		}
		else
		{
			item->souspage = -1;
		}
	}
}


sLONG BTreePageIndex::CompareKeys(const BTitemIndex *val, sLONG inIndex, const VCompareOptions& inOptions)
{
	BTitemIndex* item = GetItemPtr(inIndex);
	return entete->CompareKeys(val, item, inOptions);
	
}


sLONG BTreePageIndex::CompareKeys(sLONG inIndex, const BTitemIndex *val, const VCompareOptions& inOptions)
{
	BTitemIndex* item = GetItemPtr(inIndex);
	return entete->CompareKeys(item, val, inOptions);

}



void BTreePageIndex::Reorganize()
{
	if (btp->CurrentSizeInMem != btp->TrueDataSize)
	{
		assert(btp->CurrentSizeInMem > btp->TrueDataSize);
		char* dest = (char*) GetFastMem(fDataSize, false, 'btp ');
		assert(dest != nil);
		if (dest != nil)
		{
			char* p2 = dest;
			sLONG newoffs[kNbKeyParPageIndex];
			sLONG i, curoff = BTreePageIndex::CalculateEmptyLenOnDisk();
			for (i = 0; i < btp->nkeys; i++)
			{
				newoffs[i] = curoff;
				BTitemIndex* p = GetItemPtr(i);
				sLONG len = entete->CalulateKeyLength(p);
				vBlockMove(&p->souspage, p2, len);
				p2 = p2 + len;
				curoff = curoff + len;
			}
			btp->CurrentSizeInMem = curoff;
			assert(btp->CurrentSizeInMem <= btp->TrueDataSize);
			btp->TrueDataSize = btp->CurrentSizeInMem;
			vBlockMove(newoffs, btp->off, sizeof(newoffs));
			vBlockMove(dest, &btp->suite, curoff - BTreePageIndex::CalculateEmptyLenOnDisk());

			FreeFastMem(dest);
		}
	}
}


Boolean BTreePageIndex::CheckFit(sLONG len)
{
	Boolean res;
	if (len + btp->CurrentSizeInMem > fDataSize)
	{
		Reorganize();
		if (len + btp->CurrentSizeInMem > fDataSize)
		{
			sLONG newlen = btp->CurrentSizeInMem * 2 + len;
			BTreePageIndexDisk* newbtp = (BTreePageIndexDisk*)GetFastMem(newlen, false, 'btp ');
			if (newbtp == nil)
				res = false;
			else
			{
				vBlockMove((void*)btp, (void*)newbtp, btp->CurrentSizeInMem);
				if (btp != &fBTPdata)
					FreeFastMem((void*)btp);
				btp = newbtp;
				fDataSize = newlen;

				res = true;
			}
		}
		else
			res = true;
	}
	else
		res = true;

	return res;
}


VError BTreePageIndex::AddKey(void* p, sLONG qui, BTreePageIndex* sousBT)
{
#if debugIndexPage
	entete->debug_AddPage(this);
#endif
	//ForbidWrite();
	VError err = VE_OK;
	sLONG len = entete->CalulateDataPtrKeyLength(p);
	Boolean fit = CheckFit(len);
	if (fit)
	{
		sLONG nb = btp->nkeys;
		sLONG n = nb;
		assert(nb < kNbKeyParPageIndex);
		btp->nkeys = nb + 1;
		btp->off[n] = btp->CurrentSizeInMem;
		btp->CurrentSizeInMem = btp->CurrentSizeInMem + len;
		btp->TrueDataSize = btp->TrueDataSize + len;
		BTitemIndex* x = GetItemPtr(n);
		x->SetQui(qui);
		if (sousBT != nil)
		{
			SetSousPage(n+1, sousBT);
		}
		else
		{
			tabmem[n+1] = nil;
			x->souspage = -1;
		}
		vBlockMove(p, &x->data, len - (sizeof(BTitemIndex)-sizeof(void*)-sizeof(BTreePageIndex*) ) );
	}
	else 
	{
		err = ThrowError(memfull, DBaction_InsertingKeyIntoPage);
	}

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTINSERTKEYINTOPAGE, DBaction_InsertingKeyIntoPage);

	return err;
}


VError BTreePageIndex::InsertKey(sLONG n, BTitemIndex* u, Boolean AvecSousPage)
{
#if debugIndexPage
	entete->debug_AddPage(this);
#endif
	//ForbidWrite();
	VError err = VE_OK;
	sLONG len = entete->CalulateKeyLength(u);
	Boolean fit = CheckFit(len);
	if (fit)
	{
		sLONG nb = btp->nkeys;
		assert(nb < kNbKeyParPageIndex);
		sLONG i, *p = &(btp->off[nb-1]), *p2 = &(btp->off[nb]);
		BTreePageIndex* *tp = &(tabmem[nb]);
		BTreePageIndex* *tp2 = &(tabmem[nb+1]);

		for (i = n; i < nb; i++)
		{
			*p2 = *p;
			p2--;
			p--;
		}

		sLONG nn = nb+1;
		for (i = n; i < nb; i++)
		{
			BTreePageIndex* sousBT = *tp;
			*tp2 = sousBT;
			tp2--;
			tp--;
			nn--;
		}

		btp->nkeys = nb + 1;
		btp->off[n] = btp->CurrentSizeInMem;
		btp->CurrentSizeInMem = btp->CurrentSizeInMem + len;
		btp->TrueDataSize = btp->TrueDataSize + len;
		BTitemIndex* x = GetItemPtr(n);
		vBlockMove(&u->souspage, &x->souspage, len);
		if (AvecSousPage)
		{
			SetSousPage(n+1, u->sousBT);
		}
		else
			tabmem[n+1] = nil;
	}
	else 
	{
		err = ThrowError(memfull, DBaction_InsertingKeyIntoPage);
	}

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTINSERTKEYINTOPAGE, DBaction_InsertingKeyIntoPage);

	return err;
}


VError BTreePageIndex::SetKey(sLONG n, BTitemIndex* u, Boolean AvecSousPage)
{
#if debugIndexPage
	entete->debug_AddPage(this);
#endif
	//ForbidWrite();
	VError err = VE_OK;
	sLONG len = entete->CalulateKeyLength(u);
	BTitemIndex* x = GetItemPtr(n);
	sLONG oldlen = entete->CalulateKeyLength(x);
	Boolean fit;

	if (len <= oldlen)
	{
		fit = true;
		vBlockMove(&u->souspage, &x->souspage, len);
		if (AvecSousPage)
		{
			SetSousPage(n+1, u->sousBT);
		}
		else
			tabmem[n+1] = nil;
	}
	else
	{
		fit = CheckFit(len);

		if (fit)
		{
			btp->off[n] = btp->CurrentSizeInMem;
			btp->CurrentSizeInMem = btp->CurrentSizeInMem + len;
			btp->TrueDataSize = btp->TrueDataSize + len;
			x = GetItemPtr(n);
			vBlockMove(&u->souspage, &x->souspage, len);
			if (AvecSousPage)
			{
				SetSousPage(n+1, u->sousBT);
			}
			else
				tabmem[n+1] = nil;
		}
		else 
		{
			err = ThrowError(memfull, DBaction_InsertingKeyIntoPage);
		}
	}

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTINSERTKEYINTOPAGE, DBaction_InsertingKeyIntoPage);

	return err;
}


void BTreePageIndex::DelKeys(sLONG from)
{
#if debugIndexPage
	entete->debug_AddPage(this);
#endif
	//ForbidWrite();
	sLONG i;
	sLONG nb = btp->nkeys;

	for (i = from; i < nb; i++)
	{
		sLONG len = entete->CalulateKeyLength(GetItemPtr(i));
		btp->TrueDataSize = btp->TrueDataSize - len;
	}

	btp->nkeys = from;
}


void BTreePageIndex::SupKey(sLONG n)
{
#if debugIndexPage
	entete->debug_AddPage(this);
#endif
	//ForbidWrite();
	sLONG len = entete->CalulateKeyLength(GetItemPtr(n));

	sLONG nb = btp->nkeys;
	assert(nb <= kNbKeyParPageIndex);
	sLONG i, *p = &(btp->off[n+1]), *p2 = &(btp->off[n]);

	BTreePageIndex* *tp = &(tabmem[n+2]);
	BTreePageIndex* *tp2 = &(tabmem[n+1]);

	for (i = n; i < nb; i++)
	{
		*p2 = *p;
		p2++;
		p++;
	}

	for (i = n; i < (nb-1); i++)
	{
		BTreePageIndex* sousBT = *tp;
		*tp2 = sousBT;
		tp2++;
		tp++;
	}

	btp->nkeys = nb - 1;
	btp->TrueDataSize = btp->TrueDataSize - len;
}



void BTreePageIndex::SupKeys(sLONG n, sLONG howmany)
{
#if debugIndexPage
	entete->debug_AddPage(this);
#endif
	//ForbidWrite();
	sLONG i, len = 0;
	for (i = n; i < n+howmany; i++)
	{
		len = len + entete->CalulateKeyLength(GetItemPtr(i));
	}

	sLONG nb = btp->nkeys;
	assert(nb <= kNbKeyParPageIndex);
	sLONG *p = &(btp->off[n+howmany]), *p2 = &(btp->off[n]);

	BTreePageIndex* *tp = &(tabmem[n+1+howmany]);
	BTreePageIndex* *tp2 = &(tabmem[n+1]);

	for (i = n; i <= (nb-howmany); i++)
	{
		*p2 = *p;
		p2++;
		p++;
	}

	for (i = n; i <= (nb-1-howmany); i++)
	{
		BTreePageIndex* sousBT = *tp;
		*tp2 = sousBT;
		tp2++;
		tp++;
	}

	btp->nkeys = nb - howmany;
	btp->TrueDataSize = btp->TrueDataSize - len;
}


VError BTreePageIndex::MoveAndAddItems(sLONG from, sLONG howmany, BTreePageIndex* into)
{
	VError err = VE_OK;
	//ForbidWrite();
	sLONG i,j,nb = from + howmany;
	assert(into->btp->nkeys+howmany <= kNbKeyParPageIndex);

	for (i = from, j = into->btp->nkeys; i < nb; i++, j++)
	{
		BTitemIndex* p = GetItemPtr(i);
		err = into->InsertKey(j, p, false);
		if (err != VE_OK)
			break;
		BTreePageIndex* sousBT = tabmem[i+1];
		into->tabmem[j+1] = sousBT;
	}

	return err;
}


VError BTreePageIndex::MoveAndInsertItemsAtBegining(sLONG from, sLONG howmany, BTreePageIndex* into)
{
	VError err = VE_OK;
	//ForbidWrite();
	sLONG i,j,nb = from + howmany;
	assert(into->btp->nkeys+howmany <= kNbKeyParPageIndex);

	for (i = from, j = 0; i < nb; i++, j++)
	{
		BTitemIndex* p = GetItemPtr(i);
		err = into->InsertKey(j, p, false);
		if (err != VE_OK)
			break;
		BTreePageIndex* sousBT = tabmem[i+1];
		into->tabmem[j+1] = sousBT;
	}

	return err;
}


VError BTreePageIndex::Place(OccupableStack* curstack, uBOOL *h, BTitemIndex* &v, IndexKeyArray& tempkeys, BaseTaskInfo* context, const VCompareOptions& inOptions)
{
	BTreePageIndex *sousBT;
	sLONG l,k,r,i,rescomp;
	BTitemIndex *u;
	uBOOL egal,comefromplace,EgalPourCluster;
	VError err = VE_OK;

#if debuglr == 112
	checktabmem();
#endif
	if (AccesModifOK(context))
	{
		
		l=1;
		r=btp->nkeys;
		
		egal=false;
		EgalPourCluster=false;
		while ( (r>=l) && !egal && err == VE_OK)
		{
			k=(r+l)>>1;

			rescomp = CompareKeys(v, k-1, inOptions);

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
					if (xqui == v->GetQui())
					{
						egal = true;
					}
					else
					{
						if (v->GetQui() > xqui)
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
				l=entete->GetClusterSel(curstack)->AddToCluster(curstack, oldqui, v->GetQui(), context, err);
				if (err == VE_OK)
				{
					if (oldqui != l)
					{
						WriteLock();
						SetQui(k-1,l);
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
							err = sousBT->Place(curstack, h, u, tempkeys, context, inOptions);
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
							if (btp->nkeys<kNbKeyParPageIndex)
							{
								*h=false;
		
								WriteLock();
								err = InsertKey(r,u,true);
		
								if (u->sousBT!=nil) 
								{
#if debuglr
									checkPosSousBT(u->sousBT);
#endif
									u->sousBT->SetEncours(false);
									u->sousBT->Free(curstack, true); // ##
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
									if (r<=HalfPage)
									{
										if (r==HalfPage)
										{
											v = u;
										}
										else
										{
											v = GetItem(curstack, HalfPage-1, true, tempkeys, context, err);
											if (err == VE_OK)
											{
												SupKey(HalfPage-1);
												InsertKey(r,u, true);
												if (u->sousBT!=nil)
												{
#if debuglr
													checkPosSousBT(u->sousBT);
#endif
													u->sousBT->SetEncours(false);
													u->sousBT->Free(curstack, true); // ##
												}
											}
										}
										
										// move HalfPage keys from this to sousBT
										if (err == VE_OK)
											err = MoveAndAddItems(HalfPage, HalfPage, sousBT);
									}
									else
									{
										r=r-HalfPage;
										v = GetItem(curstack, HalfPage, true, tempkeys, context, err);
										if (err == VE_OK)
										{
											err = MoveAndAddItems(HalfPage+1, r-1, sousBT);
											if (err == VE_OK)
											{
												err = sousBT->InsertKey(sousBT->GetNKeys(), u, true);
												if (err == VE_OK)
												{
													if (u->sousBT!=nil)
													{
#if debuglr
														sousBT->checkPosSousBT(u->sousBT);
#endif
														u->sousBT->SetEncours(false);
														u->sousBT->Free(curstack, true); // ##
													}
													err = MoveAndAddItems(HalfPage+r, HalfPage-r, sousBT);
												}
											}
										}
									}
									
									DelKeys(HalfPage);
									sousBT->SetSousPage(0,v->sousBT);
									if (v->sousBT!=nil)
										v->sousBT->Free(curstack, true);
									v->sousBT=sousBT;
									if (v->sousBT != nil)
										v->souspage = sousBT->GetNum();
									else
										v->souspage = -1;
									
#if debuglr
									//checkPosSousBT(sousBT);
#endif
									err = savepage(curstack, context);
									if (err == VE_OK)
										err = sousBT->savepage(curstack, context);
									sousBT->SetEncours(true);

									sousBT->WriteUnLock();
									WriteUnLock();

#if debugCheckIndexPageOnDiskInDestructor
									sousBT->DebugCheckPageOnDisk();
#endif
									
									// sousBT->Free(curstack, true);  // ##  il ne faut pas le liberer ici car il sera libere plus tard en tant que u.sousBT ou v->sousBT
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
	
#if debuglr
	CheckPageLength();
#endif
#if debuglr == 112
	checktabmem();
#endif

	/* pas la peine de generer une exception car c'est fait au retour du premier niveau d'appel */
	return(err);
}
			

			
void BTreePageIndex::underflow(OccupableStack* curstack, BTreePageIndex* pc, sLONG s, uBOOL *h, uBOOL *doisdel, IndexKeyArray& tempkeys, BaseTaskInfo* context)
{
	BTreePageIndex* pb;
	sLONG mc,mb,k,i;
	BTitemIndex* etamp;
	VError err;
	
#if debuglr == 115
	DisplayKeys(L"Avant UnderFlow this");
	pc->DisplayKeys(L"Avant UnderFlow pc");
#endif
#if debuglr
	debug_underflow_has_been_executed = true;
#endif

	*doisdel=false;

	mc=pc->btp->nkeys;
	if (s<mc) // { b est la page de droite de this }
	{
		++s;
		etamp = pc->GetItem(curstack, s-1, true, tempkeys, context, err);
		pb=etamp->sousBT;
#if debuglr == 115
		pb->DisplayKeys(L"Avant UnderFlow pb");
#endif
		
		mb=pb->btp->nkeys;
		k=(mb-HalfPage+1)/2;
		
		WriteLock();
		etamp->sousBT = pb->GetSousPage(curstack, 0, err, context);
		InsertKey(HalfPage-1,etamp, true);
		if (etamp->sousBT!=nil)
			etamp->sousBT->Free(curstack, true);
		if (k>0)   // deplace k items de b en this
		{
			pb->WriteLock();
			pc->WriteLock();
			pb->MoveAndAddItems(0, k-1, this);

			etamp = pb->GetItem(curstack, k-1, true, tempkeys, context, err);
			pb->SetSousPage(0,etamp->sousBT);
			if (etamp->sousBT!=nil)
				etamp->sousBT->Free(curstack, true);
			etamp->sousBT=pb;

			pc->SetKey(s-1,etamp, true);

			mb=mb-k;
#if debuglr == 112
			OKTOCHECKTABMEM1 = false;
#endif
			pb->SupKeys(0, k);
			*h=false;
			
#if debuglr == 112
			OKTOCHECKTABMEM1 = true;
#endif

#if debuglr == 115
			pb->DisplayKeys(L"Apres UnderFlow pb");
			DisplayKeys(L"Apres UnderFlow this");
			pc->DisplayKeys(L"Apres UnderFlow pc");
#endif
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

#if debuglr
			CheckPageLength();
#endif
		}
		else // fusione les pages this et b dans this
		{
			pb->WriteLock();
			pc->WriteLock();
			pb->MoveAndAddItems(0, HalfPage, this);

			pc->SupKey(s-1);
			*h=(pc->btp->nkeys < HalfPage);
			
			pb->SetValidForWrite(false);
			pb->WriteUnLock();

			pb->LiberePage(curstack, context);
			pb->Free(curstack, true);
			pb->ReleaseAndCheck();
			pb = nil;

#if debuglr == 115
			DebugMsg(L"Apres UnderFlow pb est detruite");
			DisplayKeys(L"Apres UnderFlow this");
			pc->DisplayKeys(L"Apres UnderFlow pc");
#endif
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
#if debuglr == 115
		pb->DisplayKeys(L"Avant UnderFlow pb");
#endif
		
		mb=pb->btp->nkeys+1;
		k=(mb-HalfPage+1)/2;
		
		if (/*k>0*/ pb->btp->nkeys > HalfPage) // deplace k items de b en this
		{
			WriteLock();
			pb->WriteLock();
			pc->WriteLock();

			etamp = pc->GetItem(curstack, s-1, true, tempkeys, context, err);
			if (etamp->sousBT!=nil)
				etamp->sousBT->Free(curstack, true);
			etamp->sousBT = GetSousPage(curstack, 0, err, context);

			InsertKey(/*k-1*/0, etamp, true);
			if (etamp->sousBT!=nil)
				etamp->sousBT->Free(curstack, true);
			mb=mb-k;

			//pb->MoveAndAddItems(mb, k-1, this);
			pb->MoveAndInsertItemsAtBegining(mb, k-1, this);

			etamp = pb->GetItem(curstack, mb-1, true, tempkeys, context, err);
			SetSousPage(0,etamp->sousBT);
			if (etamp->sousBT!=nil)
				etamp->sousBT->Free(curstack, true);
			etamp->sousBT=this;
			pc->SetKey(s-1,etamp, true);
			
			pb->DelKeys(mb-1);
			*h=false;
			
#if debuglr == 115
			pb->DisplayKeys(L"Apres UnderFlow pb");
			DisplayKeys(L"Apres UnderFlow this");
			pc->DisplayKeys(L"Apres UnderFlow pc");
#endif
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
#if debuglr
			pb->CheckPageLength();
#endif
		}
		else // fusionne les pages this et b dans b
		{
			WriteLock();
			pb->WriteLock();
			pc->WriteLock();

			etamp = pc->GetItem(curstack, s-1, true, tempkeys, context, err);
			if (etamp->sousBT!=nil)
				etamp->sousBT->Free(curstack, true);
			etamp->sousBT=GetSousPage(curstack, 0, err, context);

			pb->InsertKey(mb-1,etamp, true);
			if (etamp->sousBT!=nil)
				etamp->sousBT->Free(curstack, true);
			
			MoveAndAddItems(0, HalfPage-1, pb);

			// pc->DelKeys(mc-1);  // hautement suspect
			pc->SupKey(s-1); // me semble plus approprie

#if debuglr == 115
			pb->DisplayKeys(L"Apres UnderFlow pb");
			DebugMsg(L"Apres UnderFlow this est detruite");
			pc->DisplayKeys(L"Apres UnderFlow pc");
#endif
#if debuglr
			pc->checkPosSousBT(pb);
#endif
			pb->savepage(curstack, context);
			pc->savepage(curstack, context);
			*h=( pc->GetNKeys() < HalfPage );

			pc->WriteUnLock();
			pb->WriteUnLock();

			SetValidForWrite(false);
			WriteUnLock();

			*doisdel=true;
			LiberePage(curstack, context);

			pb->Free(curstack, true);
#if debuglr
			pb->CheckPageLength();
#endif
		}
	}

#if debuglr
	CheckPageLength();
	pc->CheckPageLength();
#endif
}


void BTreePageIndex::del(OccupableStack* curstack, BTreePageIndex* from, sLONG k, uBOOL *h, uBOOL *doisdel, IndexKeyArray& tempkeys, BaseTaskInfo* context)
{
	BTreePageIndex *sousBT;
	sLONG r;
	BTreePageIndex *old;
	BTitemIndex* u;
	uBOOL locdoisdel;
	VError err;

#if debuglr
	debug_underflow_has_been_executed = true;
#endif

	*doisdel=false;
	
	r = GetNKeys();
	//sousBT = GetSousPage(curstack, r-1, err, context);
	sousBT = GetSousPage(curstack, r, err, context); // j'ai remplace la ligne precedente par celle ci

	if (sousBT!=nil)
	{
		//sousBT->del(this,r,h,&locdoisdel,tempkeys, context);
		sousBT->del(curstack, from, k, h, &locdoisdel, tempkeys, context); // j'ai remplace la ligne precedente par celle ci
		if (*h) 
			sousBT->underflow(curstack, this, r, h,/*doisdel*/&locdoisdel, tempkeys, context);
		sousBT->Free(curstack, true);
		if (locdoisdel) 
		{
			sousBT->ReleaseAndCheck();
		}
	}
	else
	{
		old = from->GetSousPage(curstack, k, err, context);
		u = GetItem(curstack, r-1, true, tempkeys, context, err);

		WriteLock();
		from->WriteLock();

		if (old==nil) 
			u->souspage=-1; 
		else 
			u->souspage=old->GetNum();

		u->sousBT=old;

		if (old != nil) 
			old->Free(curstack, true);
		
		from->SetKey(k-1,u,true);
		DelKeys(r-1);
		
#if debuglr
		from->checkPosSousBT(u->sousBT);
#endif
		from->savepage(curstack, context);
		savepage(curstack, context);

		from->WriteUnLock();
		WriteUnLock();

		*h=(GetNKeys()<HalfPage);
	}

#if debuglr
	CheckPageLength();
	from->CheckPageLength();
#endif
}


VError BTreePageIndex::Detruire(OccupableStack* curstack, uBOOL *h, BTitemIndex *v, uBOOL *doisdel, IndexKeyArray& tempkeys, BaseTaskInfo* context, const VCompareOptions& inOptions)
{
	VError err = VE_OK;
	
	BTreePageIndex *sousBT;
	sLONG l,k,r,p,rescomp;
	uBOOL egal,locdoisdel,EgalPourCluster = false;

	// il faut faire deux passes lors d'une transaction
	
#if debuglr == 115
	DisplayKeys(L"Avant Detruire");
#endif

	*doisdel=false;
	
	//if (AccesModifOK())
	{		
		l=1;
		r=GetNKeys();
		//val=v->val;
		
		egal=false;
		while (r>=l)
		{
			k=(r+l)>>1;
			rescomp = CompareKeys(v, k-1, inOptions);

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
					if (xqui == v->GetQui())
					{
						egal = true;
					}
					else
					{
						if (v->GetQui() > xqui)
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
			sLONG clustval = entete->GetClusterSel(curstack)->DelFromCluster(curstack, oldqui, v->GetQui(), context, err);
			if (clustval != oldqui)
			{
				WriteLock();
				SetQui(k-1,clustval);
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
					BTitemIndex* x = GetItemPtr(k-1);
					p=x->souspage;
					SupKey(k-1);
					*h=(GetNKeys()<HalfPage);
					savepage(curstack, context);
					WriteUnLock();
				}
				else
				{
					sousBT->del(curstack, this, k, h, &locdoisdel, tempkeys, context);
					if (*h) sousBT->underflow(curstack, this, r, h, /*doisdel*/&locdoisdel, tempkeys, context);
					sousBT->Free(curstack, true);
					if (locdoisdel) 
					{
						sousBT->ReleaseAndCheck();
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
					sousBT->Detruire(curstack, h, v, &locdoisdel, tempkeys, context, inOptions);
					if (*h) sousBT->underflow(curstack, this, r, h, /*doisdel*/&locdoisdel, tempkeys, context);
					sousBT->Free(curstack, true);
					if (locdoisdel) 
					{
						sousBT->ReleaseAndCheck();
					}
				}
			}
		}
		
	}
	
#if debuglr
	CheckPageLength();
#endif
	
	return(err);
}


void BTreePageIndexDisk::SwapBytes()
{
	ByteSwap(&CurrentSizeInMem);
	ByteSwap(&TrueDataSize);
	ByteSwap(&nkeys);
	ByteSwap(&souspage0);
	ByteSwap(&off[0], kNbKeyParPageIndex);
}


VError BTreePageIndex::loadobj(DataAddr4D xaddr, sLONG len)
{
	Base4D *db;
	VError err = VE_OK;
		
	db=entete->GetDB();
	setaddr(xaddr);
		
	assert(db->IsAddrValid(getaddr()));
	
	oldlen=len-kSizeDataBaseObjectHeader;
	
	assert(oldlen>0 && oldlen<=1000000);

	DataBaseObjectHeader tag;
	err = tag.ReadFrom(db, getaddr(), nil);
	if (err == VE_OK)
	{
		err = tag.ValidateTag(DBOH_BtreePage, num, -2);
		oldlen = tag.GetLen();
	}

	if (oldlen > fDataSize)
	{
		sLONG newlen = oldlen + 1024;
		BTreePageIndexDisk* newbtp = (BTreePageIndexDisk*)GetFastMem(newlen, false, 'btp ');
		if (newbtp == nil)
			err = ThrowError(memfull, noaction);
		else
		{
			if (btp != &fBTPdata)
				FreeFastMem((void*)btp);
			btp = newbtp;
			fDataSize = newlen;
		}
	}

	if (err == VE_OK)
		err=db->readlong( btp, oldlen, getaddr(), kSizeDataBaseObjectHeader);
	if (err == VE_OK)
		err = tag.ValidateCheckSum(	btp, oldlen);

	if (tag.NeedSwap())
	{
		btp->SwapBytes();
		sLONG i;
		for (i=0;i<btp->nkeys;i++)
		{
			BTitemIndex* p = GetItemPtr(i);
			ByteSwap(&p->souspage);
			ByteSwap(&p->qui);
			entete->SwapByteKeyData(p);
		}
	}

	if (err == VE_OK)
	{
		assert(btp->nkeys>0 && btp->nkeys<=kNbKeyParPageIndex);
		assert(btp->CurrentSizeInMem <= oldlen);
		oldlen = btp->CurrentSizeInMem;
		fCurrentSizeToWrite = btp->CurrentSizeInMem;
	}

	_raz(tabmem,sizeof(BTreePageIndex*) * (btp->nkeys+1));
	
	return(err);
}



VError BTreePageIndex::savepage(OccupableStack* curstack, BaseTaskInfo* context)
{
	DataAddr4D ou;
	Base4D *db;
	IndexHeader *head;
	VError err;
#if debuglr
	Boolean TousMoinsUn = true, AuMoinsUnMoinsUn = false;
#endif
	
	err=VE_OK;
#if debuglr == 112
	AddPageInfoDebug(this, false, true);
#endif
#if debuglr == 113
	sLONG i;
	if (btp->souspage0 == -1)
		AuMoinsUnMoinsUn = true;
	else
		TousMoinsUn = false;
	for (i = 0; i<btp->nkeys; i++)
	{
		BTitemIndex* x = GetItemPtr(i);
		if (x->souspage == -1)
			AuMoinsUnMoinsUn = true;
		else
			TousMoinsUn = false;
	}
	if (AuMoinsUnMoinsUn)
		assert(TousMoinsUn);
#endif

#if debuglr

	VCompareOptions options = entete->GetStrictOptions();
	options.SetIntlManager(GetContextIntl(context));

	sLONG nb = btp->nkeys-1;
	for (sLONG i = 0; i < nb; i++)
	{
		BTitemIndex* val = GetItemPtr(i);
		sLONG rescomp = CompareKeys(i+1, val, options);
		assert(rescomp != CR_SMALLER);
	}
#endif

	if (adjusteindex(btp->TrueDataSize + kSizeDataBaseObjectHeader) != adjusteindex(btp->CurrentSizeInMem + kSizeDataBaseObjectHeader))
	{
		Reorganize();
	}

	if (oldlen == 0 || adjusteindex(oldlen+kSizeDataBaseObjectHeader)!=adjusteindex(btp->CurrentSizeInMem + kSizeDataBaseObjectHeader))
	{
		db=entete->GetDB();
		DataAddr4D oldaddr = getaddr();
		sLONG oldoldlen = oldlen;
		oldlen = btp->CurrentSizeInMem;
		ou=db->findplace(adjusteindex(oldlen+kSizeDataBaseObjectHeader), context, err, -kIndexSegNum, this);
#if debugOverWrite_strong
		if (oldoldlen!=0)
		{
			debug_ClearPageRef(oldaddr, adjusteindex(oldoldlen+kSizeDataBaseObjectHeader), entete);
		}
		debug_SetPageRef(entete, num, ou, adjusteindex(oldlen+kSizeDataBaseObjectHeader));
#endif
		ChangeAddr(ou, db, context);
		if (oldoldlen!=0)
		{
#if debugIndexOverlap_strong
			di_IndexOverLap::RemoveIndexPage(entete, num, oldaddr, adjusteindex(oldoldlen+kSizeDataBaseObjectHeader));
#endif
			db->libereplace(oldaddr, adjusteindex(oldoldlen+kSizeDataBaseObjectHeader), context, this);
		}

		if (err == VE_OK)
		{
			head=entete->GetHeader();
			num=head->PutInd(curstack, num,ou, adjusteindex(oldlen+kSizeDataBaseObjectHeader), context, err);
			fCurrentSizeToWrite = btp->CurrentSizeInMem;
			setmodif(true, entete->GetDB(), context);
#if debugIndexOverlap_strong
			di_IndexOverLap::AddIndexPage(entete, num, ou, adjusteindex(oldlen+kSizeDataBaseObjectHeader));
#endif

		}
	}
	else
	{
		oldlen=btp->CurrentSizeInMem;
		setmodif(true, entete->GetDB(), context);
	}
	
#if debuglr == 112
	checktabmem();
#endif
	
	return(err);
}


void BTreePageIndex::LiberePage(OccupableStack* curstack, BaseTaskInfo* context)
{
	Base4D *db;
	
#if debuglr == 112
	DelPageInfoDebug(this);
#endif

	db=entete->GetDB();
	setmodif(false, db, context);
#if debugOverWrite_strong
	debug_ClearPageRef(getaddr(), adjusteindex(oldlen+kSizeDataBaseObjectHeader), entete);
#endif
#if debugIndexOverlap_strong
	di_IndexOverLap::RemoveIndexPage(entete, num, getaddr(), adjusteindex(oldlen+kSizeDataBaseObjectHeader));
#endif
	db->libereplace(getaddr(), adjusteindex(oldlen+kSizeDataBaseObjectHeader), context, this);
	entete->GetHeader()->LiberePage(curstack, num, context);
	DoNotVerify();
}


bool BTreePageIndex::SaveObj(VSize& outSizeSaved)
{
#if debuglogwrite
	VString wherefrom(L"BtreePageIndex Saveobj");
	VString* whx = &wherefrom;
#else
	VString* whx = nil;
#endif
	sLONG tot;
	Base4D *db = entete->GetDB();
	VError err = VE_OK;
		
	/*
	if (VInterlocked::AtomicGet(&fStopWrite) != 0)
	{
#if debugLogEventPageIndex
		gDebugIndexEvent.AddMessage(L"cannot save", dbgIndexEvent(fCurrentSizeToWrite, num, getaddr(), entete));
#endif
		outSizeSaved = 0;
		return false;
	}
	else
	*/
	{

#if debugOverWrite_strong
		debug_SetCurrentWritingPage(getaddr());
#endif

		tot = VInterlocked::AtomicGet(&fCurrentSizeToWrite);

#if debugLogEventPageIndex
		gDebugIndexEvent.AddMessage(L"SaveObj", dbgIndexEvent(fCurrentSizeToWrite, num, getaddr(), entete));
#endif

		//tot=btp->CurrentSizeInMem;
		//assert(tot == oldlen);
		assert(db->IsAddrValid(getaddr()));
		DataBaseObjectHeader tag(btp, tot, DBOH_BtreePage, num, -2);
		err = tag.WriteInto(db, getaddr(), whx);
		if (err == VE_OK) 
			err=db->writelong( btp ,tot, getaddr(), kSizeDataBaseObjectHeader, whx, adjusteindex(tot + kSizeDataBaseObjectHeader) - kSizeDataBaseObjectHeader);
	
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




BTreePageIndex* BTreePageIndex::LoadPage(OccupableStack* curstack, sLONG n, IndexInfo* xentete, BaseTaskInfo* context, VError& err)
{
	BTreePageIndex *page = nil;
	DataAddr4D ou;
	sLONG len;
	err = VE_OK;
	
#if debug
	assert(n>=0 && n < ((IndexHeaderBTree*)(xentete->GetHeader()))->GetNbPage() );
#endif
	ou=xentete->GetHeader()->GetInd(curstack, n, context, err, &len);

	if (ou > 0 && len > 0 && err == VE_OK)
	{
		page=((IndexHeaderBTree*)(xentete->GetHeader()))->LoadPage(curstack, ou, len, context, err, n);
	}
	else
	{
		//## throw err
	}
	
	return(page);
}


BTreePageIndex* BTreePageIndex::AllocatePage(OccupableStack* curstack, IndexInfo* xentete, BaseTaskInfo* context, VError& err)
{
	BTreePageIndex *page;
	sLONG len;
	err = VE_OK;
	
	page=((IndexHeaderBTree*)(xentete->GetHeader()))->CrePage();
	if (page != nil)
	{
		page->Occupy(curstack, true);
		err = page->savepage(curstack, context);
		if (err != VE_OK)
		{
			page->Release();
			page = nil;
		}
		/*
		else
			page->ForbidWrite();  // ce n'est pas grave de ne faire le ForbidWrite que maintenant, car la page n'est pas encore dans l'abre de l'index, elle a pu etre sauvee a vide sans consequence
			*/
	}
	else
		err = ThrowBaseError(memfull, DBaction_AllocatingIndexPageInMem);
	
	return(page);
}


Boolean BTreePageIndex::FreePageMem(sLONG allocationBlockNumber, VSize combien, VSize& tot)
{
	Boolean oktodelete = true;
	if (btp->souspage0 != -1)
	{
		for (BTreePageIndex **cur = &(tabmem[0]), **end = &(tabmem[btp->nkeys+1]); cur != end; cur++)
		{
			BTreePageIndex* souspage = *cur;
			if (souspage != nil)
			{
				if (souspage->FreePageMem(allocationBlockNumber, combien, tot) && !IsOccupied())
				{
					tot = tot + souspage->GetSizeToFree();
					souspage->ReleaseAndCheck();
					//*cur = nil;
					VInterlocked::ExchangePtr(&(*cur), (BTreePageIndex*)nil);
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

//	if (oktodelete)
//		tot = tot + fDataSize + (BTreePageIndex::CalculateEmptyLen() - BTreePageIndex::CalculateEmptyLenOnDisk());

	return oktodelete;
}



void BTreePageIndex::LibereEspaceMem(OccupableStack* curstack)
{
	sLONG nb,i;
	BTreePageIndex *sous;
	
	nb = GetNKeys();
	
	assert(nb>=0 && nb<= kNbKeyParPageIndex);
			
	for (i=0;i<=nb;i++)
	{
		sous=tabmem[i];
		if (sous!=nil)
		{
			sous->LibereEspaceMem(curstack);
			sous->ReleaseAndCheck(true);
			tabmem[i] = nil;
		}
	}
}							


void BTreePageIndex::DelFromFlush(OccupableStack* curstack)
{
	sLONG nb,i;
	BTreePageIndex *sous;

	nb = GetNKeys();

	assert(nb>=0 && nb<= kNbKeyParPageIndex);

	for (i=0;i<=nb;i++)
	{
		sous=tabmem[i];
		if (sous!=nil)
		{
			sous->DelFromFlush(curstack);
		}
	}

	setmodif(false, entete->GetDB(), nil);
}							


VError BTreePageIndex::FindKeyInArray(OccupableStack* curstack, Bittab* b, DB4DArrayOfConstValues* values, const void* &CurVal, BaseTaskInfo* context, const VCompareOptions& inOptions, ProgressEncapsuleur* InProgress)
{
	BTreePageIndex *sousBT;
	sLONG i,nb,rescomp;
	uBOOL egal;
	VError err = VE_OK;

	VTask::Yield();
	if (!InProgress->Increment())
		err = ThrowError(VE_DB4D_ACTIONCANCELEDBYUSER, noaction);


	if (err == VE_OK)
	{
		nb=btp->nkeys;
		Boolean feuillefinale = btp->souspage0 == -1;

		assert(nb>=0 && nb<= kNbKeyParPageIndex);
		for (i=0;i<nb && CurVal != nil && err == VE_OK;i++)
		{
			Boolean trynextval;
			do
			{
				trynextval = false;
				BTitemIndex* key = GetItemPtr(i);
				rescomp = values->CompareKeyWithValue(&(key->data), CurVal, inOptions);
				if (rescomp == CR_BIGGER)
				{
					if (feuillefinale)
						trynextval = true;
					else
					{
						sousBT=GetSousPage(curstack, i, err, context);
						if (sousBT!=nil)
						{
							err = sousBT->FindKeyInArray(curstack, b, values, CurVal, context, inOptions, InProgress);
							sousBT->Free(curstack, true);

							if (CurVal != nil)
							{
								rescomp = values->CompareKeyWithValue(&(key->data), CurVal, inOptions);
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
								err = sousBT->FindKeyInArray(curstack, b, values, CurVal, context, inOptions, InProgress);
								sousBT->Free(curstack, true);
							}
						}
					}
				}

				if (trynextval)
				{
					CurVal = values->GetNextPtr();
					if (CurVal == nil)
						trynextval = false;
				}
			} while (trynextval && err == VE_OK);

		}

		if (CurVal != nil && (rescomp == CR_EQUAL || rescomp == CR_SMALLER) && !feuillefinale && err == VE_OK)
		{
			sousBT=GetSousPage(curstack, btp->nkeys, err, context);
			if (sousBT!=nil)
			{
				err = sousBT->FindKeyInArray(curstack, b, values, CurVal, context, inOptions, InProgress);
				sousBT->Free(curstack, true);
			}
		}
	}

	return err;
}


VError BTreePageIndex::ParseAll(OccupableStack* curstack, Bittab* b, BTitemIndex* val1, BaseTaskInfo* context, const VCompareOptions& inOptions, ProgressEncapsuleur* InProgress)
{
	VError err = VE_OK;
	sLONG nb = btp->nkeys;
	for (sLONG i = 0; i<nb && err == VE_OK; i++)
	{
		sLONG rescomp = CompareKeys(i, val1, inOptions);
		if (rescomp == CR_EQUAL)
		{
			if (IsCluster)
			{
				entete->GetClusterSel(curstack)->AddSelToBittab(curstack, GetQui(i), b, context, err);
			}
			else
			{
				err = b->Set(GetQui(i), true);
			}
		}
	}

	VTask::Yield();
	if (!InProgress->Increment())
		err = ThrowError(VE_DB4D_ACTIONCANCELEDBYUSER, noaction);

	if (btp->souspage0 != -1)
	{
		for (sLONG i = 0; i <=nb && err == VE_OK; i++)
		{
			BTreePageIndex* sousBT = GetSousPage(curstack, i, err, context);
			if (sousBT != nil)
			{
				err = sousBT->ParseAll(curstack, b, val1, context, inOptions, InProgress);
				sousBT->Free(curstack, true);
			}
		}
	}
	return err;
}


VError BTreePageIndex::Fourche(OccupableStack* curstack, Bittab* b, BTitemIndex* val1, uBOOL xstrict1, BTitemIndex* val2, uBOOL xstrict2, 
								BaseTaskInfo* context, ProgressEncapsuleur* InProgress, const VCompareOptions& inOptions, BTitemIndex** outVal)
{
	BTreePageIndex *sousBT;
	sLONG i,nb,rescomp, rescompNonDiac;
	BTitemIndex *val;
	uBOOL egal;
	VError err = VE_OK;
	Boolean canCont = true;
	VCompareOptions OptionsNonDiac(inOptions);
	
	bool diacritical = inOptions.IsDiacritical();
	OptionsNonDiac.SetDiacritical(false);
	
	VTask::Yield();
	if (!InProgress->Increment())
		err = ThrowError(VE_DB4D_ACTIONCANCELEDBYUSER, noaction);
	
	if (err == VE_OK)
	{
		nb=btp->nkeys;
		
		assert(nb>=0 && nb<= kNbKeyParPageIndex);
 		for (i=0;i<nb && canCont;i++)
		{
			if (val1==nil)
			{
				rescomp=CR_BIGGER;
				rescompNonDiac = CR_BIGGER;
			}
			else
			{
				rescomp = CompareKeys(i, val1, inOptions);
				if (diacritical)
					rescompNonDiac = CompareKeys(i, val1, OptionsNonDiac);
				else
					rescompNonDiac = rescomp;
			}
			
			egal=((rescomp==CR_EQUAL) && !xstrict1) || (rescomp==CR_BIGGER);
			if ( (rescompNonDiac==CR_BIGGER) || ( (rescompNonDiac==CR_EQUAL) && !xstrict1) )
			{
				sousBT=GetSousPage(curstack, i, err, context);
				if (sousBT!=nil)
				{
					err = sousBT->Fourche(curstack, b, val1, xstrict1, val2, xstrict2, context, InProgress, inOptions, outVal);
					sousBT->Free(curstack, true);
				}
			}
			
			if (err != VE_OK)
				break;

			if (val2==nil)
			{
				rescomp=CR_SMALLER;
				rescompNonDiac = CR_SMALLER;
			}
			else
			{
				rescomp = CompareKeys(i, val2, inOptions);
				if (diacritical)
					rescompNonDiac = CompareKeys(i, val2, OptionsNonDiac);
				else
					rescompNonDiac = rescomp;
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
					if (*outVal != nil)
						entete->FreeKey(*outVal);
					*outVal = CopyKey(i);
				}
			}
			
			if (i==(nb-1) && err == VE_OK && canCont)
			{
				if ( ((rescompNonDiac==CR_EQUAL) && !xstrict2) || (rescompNonDiac==CR_SMALLER) )
				{
					sousBT=GetSousPage(curstack, i+1, err, context);
					if (sousBT!=nil)
					{
						err = sousBT->Fourche(curstack, b, val1, xstrict1, val2, xstrict2, context, InProgress, inOptions, outVal);
						sousBT->Free(curstack, true);
					}
				}
			}
			
			if (err != VE_OK)
				break;
			if (rescompNonDiac==CR_BIGGER) 
				break; // sort de la boucle
		}
	}
	
	return err;
}
		

VError BTreePageIndex::FindKeyAndBuildPath(OccupableStack* curstack, BTitemIndex* val, const VCompareOptions& inOptions, BaseTaskInfo* context, VDB4DIndexKey* outKey, Boolean& trouve)
{
	VError err = VE_OK;
	BTreePageIndex *sousBT;
	sLONG i,nb,rescomp;
	uBOOL sup_egal;

	nb=btp->nkeys;

	assert(nb>=0 && nb<= kNbKeyParPageIndex);
	for (i=0;i<nb;i++)
	{
		rescomp = CompareKeys(i, val, inOptions);

		sup_egal=(rescomp==CR_EQUAL) || (rescomp==CR_BIGGER);
		if ( sup_egal )
		{
			sousBT=GetSousPage(curstack, i, err, context);
			if (sousBT!=nil)
			{
				outKey->AddLevelToPath(i);
				err = sousBT->FindKeyAndBuildPath(curstack, val, inOptions, context, outKey, trouve);
				sousBT->Free(curstack, true);
				if (!trouve)
					outKey->DecLevelFromPath();
			}
		}

		if (err != VE_OK)
			break;

		//rescomp = CompareKeys(i, val, false, IsBeginWith, isLike);
		rescomp = CompareKeys(i, val, inOptions); // attention avant le diacritical etait toujours false

		if ( ((rescomp==CR_EQUAL) || (rescomp==CR_SMALLER)) && sup_egal )
		{
			trouve = true;
			if (IsCluster)
			{
				outKey->SetCurPos(i, 0, entete->GetClusterSel(curstack)->GetNthRecNum(curstack, GetQui(i), 0, context, err));
			}
			else
			{
				outKey->SetCurPos(i, 0, GetQui(i));
			}
			break;
		}

		if (i==(nb-1) && err == VE_OK)
		{
			if ( (rescomp==CR_EQUAL) || (rescomp==CR_SMALLER) )
			{
				sousBT=GetSousPage(curstack, i+1, err, context);
				if (sousBT!=nil)
				{
					outKey->AddLevelToPath(i+1);
					err = sousBT->FindKeyAndBuildPath(curstack, val, inOptions, context, outKey, trouve);
					sousBT->Free(curstack, true);
					if (!trouve)
						outKey->DecLevelFromPath();
				}
			}
		}

		if (err != VE_OK)
			break;
		if (rescomp==CR_BIGGER) break; // sort de la boucle
	}

	return err;
}


VError BTreePageIndex::GetFirstKey(OccupableStack* curstack, BaseTaskInfo* context, VDB4DIndexKey* outKey)
{
	VError err = VE_OK;
	BTreePageIndex *sousBT;

	sousBT = GetSousPage(curstack, 0, err, context);
	if (err == VE_OK)
	{
		if (sousBT == nil)
		{
			if (IsCluster)
			{
				outKey->SetCurPos(0, 0, entete->GetClusterSel(curstack)->GetNthRecNum(curstack, GetQui(0), 0, context, err));
			}
			else
			{
				outKey->SetCurPos(0, 0, GetQui(0));
			}			
		}
		else
		{
			outKey->AddLevelToPath(0);
			sousBT->GetFirstKey(curstack, context, outKey);
			sousBT->Free(curstack, true);
		}
	}

	return err;
}


VError BTreePageIndex::NextKey(OccupableStack* curstack, const VDB4DIndexKey* inKey, sLONG level, BaseTaskInfo* context, VDB4DIndexKey* outKey, Boolean& outlimit)
{
	VError err = VE_OK;
	BTreePageIndex *sousBT;

	outlimit = false;

	if (level<inKey->GetNbLevel())
	{
		sLONG curpage = inKey->GetPath()[level];
		if (curpage <= btp->nkeys)
		{
			sousBT = GetSousPage(curstack, curpage, err, context);
			if (err == VE_OK)
			{
				if (sousBT != nil)
				{
					outKey->AddLevelToPath(curpage);
					err = sousBT->NextKey(curstack, inKey, level+1, context, outKey, outlimit);
					sousBT->Free(curstack, true);
					if (outlimit)
					{
						outKey->DecLevelFromPath();
						if (curpage < btp->nkeys)
						{
							outlimit = false;
							curpage++;
							sousBT = GetSousPage(curstack, curpage, err, context);
							if (err == VE_OK && sousBT != nil)
							{
								outKey->AddLevelToPath(curpage);
								err = sousBT->GetFirstKey(curstack, context, outKey);
								sousBT->Free(curstack, true);
							}
						}
						else
						{
							// rien a faire, il faut remonter d'un niveau
						}
					}
					else
					{
					}
				}
			}
		}
		else
			outlimit = true;
	}
	else
	{
		sLONG curpos = inKey->GetCurpos();
		sLONG curposinsel = inKey->GetCurposInSel();
		if (IsCluster)
		{
			sLONG recnum = entete->GetClusterSel(curstack)->GetNextRecNum(curstack, GetQui(curpos), curposinsel, inKey->GetRecNum(), context, err);
			if (err == VE_OK)
			{
				if (recnum == -1)
				{
					if (curpos<btp->nkeys)
					{
						outKey->SetCurPos(curpos+1, 0, GetQui(curpos+1));
					}
					else
						outlimit = true;
				}
				else
				{
					outKey->SetCurPos(curpos, curposinsel+1, recnum);
				}
			}
		}
		else
		{
			if (curpos<btp->nkeys)
			{
				outKey->SetCurPos(curpos+1, 0, GetQui(curpos+1));
			}
			else
				outlimit = true;
		}
	}

	return err;
}


VError BTreePageIndex::ScanIndex(OccupableStack* curstack, Selection* into, sLONG& currec, sLONG inMaxRecords, Boolean KeepSorted, Boolean Ascent, BaseTaskInfo* context, Bittab* filtre, ProgressEncapsuleur* InProgress)
{
	VError err = VE_OK;
	BTreePageIndex* sousBT;
	BTitemIndex* x;

	VTask::Yield();
	if (!InProgress->Increment())
		err = ThrowError(VE_DB4D_ACTIONCANCELEDBYUSER, noaction);

	if (err == VE_OK)
	{
		sLONG nb = btp->nkeys;

		assert(nb>=0 && nb<= kNbKeyParPageIndex);
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

#if debuglr


void BTreePageIndex::checkWrongRecordRef(sLONG limit, BTitemIndex* &curval)
{
	VCompareOptions options = entete->GetStrictOptions();
	options.SetIntlManager(GetContextIntl(nil));

	DataTable* df = entete->GetTargetTable()->GetDF();
	sLONG nb = btp->nkeys;
	for (sLONG i = 0; i < nb; i++)
	{
		BTreePageIndex* sous = tabmem[i];
		if (sous != nil)
			sous->checkWrongRecordRef(limit, curval);

		BTitemIndex* x = GetItemPtr(i);
		if (curval != nil)
		{
			sLONG result = entete->CompareKeys(curval, x, options);
			if (result == CR_EQUAL)
				assert(curval->GetQui() < x->qui);
			else
				assert(result = CR_SMALLER);
		}
		curval = x;
		//assert(! df->isRecordDeleted(x->qui));
		assert(x->GetQui() < limit);
	}

	BTreePageIndex* sous = tabmem[nb];
	if (sous != nil)
		sous->checkWrongRecordRef(limit, curval);


	/*
	for (sLONG i = 0; i <= nb; i++)
	{
		BTreePageIndex* sous = tabmem[i];
		if (sous != nil)
			sous->checkWrongRecordRef(limit, curval);
	}
	*/
}


void BTreePageIndex::CheckPageOwner()
{

	sLONG nb = btp->nkeys;
	for (sLONG i = 0; i <= nb; i++)
	{
		BTreePageIndex* sous = tabmem[i];
		if (sous != nil)
			sous->CheckPageOwner();
	}

}


void BTreePageIndex::CheckPageKeys()
{
	VCompareOptions options = entete->GetStrictOptions();
	options.SetIntlManager(GetContextIntl(nil));

	sLONG nb = btp->nkeys-1;
	for (sLONG i = 0; i < nb; i++)
	{
		BTitemIndex* val = GetItemPtr(i);
		sLONG rescomp = CompareKeys(i+1, val, options);
		assert(rescomp != CR_SMALLER);
	}
	nb = btp->nkeys;
	for (sLONG i = 0; i <= nb; i++)
	{
		BTreePageIndex* sous = tabmem[i];
		if (sous != nil)
			sous->CheckPageKeys();
	}

}


void BTreePageIndex::checkPosSousBT(BTreePageIndex* sousBT)
{
	assert(sousBT != nil);
	sLONG numsous = sousBT->GetNum();
	sLONG nb = btp->nkeys;
	sLONG found = -1;
	BTitemIndex* val = nil;


	VCompareOptions options = entete->GetStrictOptions();
	options.SetIntlManager(GetContextIntl(nil));

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
		BTitemIndex* sousBTMin = sousBT->GetItemPtr(0);
		BTitemIndex* sousBTMax = sousBT->GetItemPtr(sousBT->btp->nkeys-1);
		if (found != 0)
		{
			assert(CompareKeys(found-1, sousBTMin, options) != CR_BIGGER);
		}
		if (found != nb)
		{
			assert(CompareKeys(found, sousBTMax, options) != CR_SMALLER);
		}
	}
}


void BTreePageIndex::DisplayKeys(const VString& message)
{
	StAllocateInMainMem alloc;
	DebugMsg(message+L"  :  PageNum = "+ToString(num)+L" , NbKeys = "+ToString(btp->nkeys)+L"\n\n");
	VError err = VE_OK;

	sLONG nb = btp->nkeys;
	for (sLONG i = 0; i < nb; i++)
	{
		BTitemIndex* val = GetItemPtr(i);
		ValPtr cv = entete->CreateVValueWithKey(val, err);
		VString s;
		cv->GetString(s);
		delete cv;
		DebugMsg(s+L" : " + ToString(val->GetQui())+ L"\n");
	}

	DebugMsg(L"\n\n\n");
}

		
void BTreePageIndex::Display(void)
{
	/*
	BTreePageIndex *sousBT;
	sLONG i,nb;
	IndexValue *val;
	VError err;
	VStr255 s;

	Occupy();
	
	nb=btp->nkeys;
	
	sousBT=GetSousPage(curstack, 0, err);
	if (sousBT!=nil)
	{
		sousBT->Display();
		sousBT->Free(curstack, true);
	}
	for (i=0;i<nb;i++)
	{
		val=loadval(i);
		val->ChangeToText(&s);
//		UnivStrDebug(s);
		
		sousBT=GetSousPage(curstack, i+1, err);
		if (sousBT!=nil)
		{
			sousBT->Display();
			sousBT->Free(curstack, true);
		}
	}

	Free(curstack, true);
	*/
	
}
	
#if debuglr == 112

void BTreePageIndex::checktabmem(void)
{
	sLONG i,nb;
	BTreePageIndex* sousBT;
	
	if (OKTOCHECKTABMEM1)
	{
		if (btp!=nil)
		{
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
	}
}
#endif

#endif



									/* -----------------------------------------------  */




bool operator < (const BTitemIndexHolder& v1, const BTitemIndexHolder& v2) 
{
	bool result;
	BTitemIndex* key1 = v1.RetainKey();
	BTitemIndex* key2 = v2.RetainKey();

	if (key1 == nil || key1->IsNull())
	{
		if (key2 == nil)
			result = false;
		else
			result = true;
	}
	else
	{
		if (key2 == nil || key2->IsNull())
			result = false;
		else
		{		
			VCompareOptions options;
			options.SetIntlManager(GetContextIntl(v1.GetContext()));
			options.SetLike(key2->qui == kTransFakeKeyNumForIsLike);
			options.SetBeginsWith(key2->qui == kTransFakeKeyNumForBeginWith);
			options.SetDiacritical(true);			

			sLONG comp = key1->GetInd()->CompareKeys(key1, key2, options);
			if (comp == CR_EQUAL)
				result = key1->GetQui() < key2->GetQui();
			else
				result = comp == CR_SMALLER;
		}
	}

	if (key1 != nil)
		key1->Unuse();
	if (key2 != nil)
		key2->Unuse();

	return result;
}



									/* -----------------------------------------------  */




IndexKeyIterator::IndexKeyIterator(IndexInfo* ind, Bittab* filter, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, const VString& inMess, OccupableStack* curstack):fProgress(InProgress)
{
	fWasJustReset = true;
	fInd = ind;
	if (testAssert(ind != nil))
		fInd->Retain();

	fStrictOptions = ind->GetStrictOptions();
	fStrictOptions.SetIntlManager(GetContextIntl(context));

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
	fCurstack = curstack;

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


	libere();
}


IndexKeyIterator::~IndexKeyIterator()
{
	//if (fProgress != nil)
	{
		fProgress.EndSession();
	}

	if (fKeyInTrans != nil)
		fKeyInTrans->Unuse();
	_DisposeSel();
	_DisposePages();
	if (fInd != nil)
		fInd->Release();
	if (fContext != nil)
		fContext->Release();
}


void IndexKeyIterator::_DisposePages()
{
	for (sLONG i = 0; i<fNbLevel; i++)
		fPagePath[i].fPage->Free(fCurstack, true);
	fNbLevel = 0;
}


void IndexKeyIterator::_DisposeSel()
{
	if (fSel != nil)
		fSel->Release();
	fSel = nil;
	fSelIter.Reset(nil);
}


void IndexKeyIterator::_RetainKey()
{
	if (fKeyInTrans != nil)
		fKeyInTrans->Unuse();
	fKeyInTrans = nil;
	if (fCleAddEnTrans)
	{
		if (fCurSavedKey != fEndSavedKey)
		{
			fKeyInTrans = fCurSavedKey->RetainKey();
		}
	}
}


VError IndexKeyIterator::_SetSel(Selection* sel)
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


void IndexKeyIterator::_DisposeKey()
{
	if (fKey != nil)
		fInd->FreeKey(fKey, &keyprealloc);
	fKey = nil;
}


sLONG IndexKeyIterator::GetRecNum() const 
{ 
	sLONG result = -1;
	if (fCurKeyInTrans)
	{
		if (fKeyInTrans != nil)
			result = fKeyInTrans->GetQui();
	}
	else
		result = fRecNum; 
	return result;
}


const BTitemIndex* IndexKeyIterator::GetKey() const
{
	if (fCurKeyInTrans)
	{
		return fKeyInTrans;
	}
	else
	{
		return fKey;
	}
}


Boolean IndexKeyIterator::FirstKey(BaseTaskInfo* context, VError& err, sLONG* CurElemToProgress)
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
			assert(! fInd->IsScalar());
			fSavedKeys = (mapIndexKeys*)trans->GetSavedKeys(fInd, false, err);
			fDeletedKeys = (mapIndexKeys*)trans->GetDeletedKeys(fInd, false, err);
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

		IndexHeaderBTree* header = (IndexHeaderBTree*)fInd->GetHeader();
		if (header != nil)
		{
			err = header->ChargeFirstPage(fCurstack, fContext);
			if (err == VE_OK)
			{
				BTreePageIndex* page = header->_GetFirstPage();
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


void IndexKeyIterator::_PosToFirstLeft(BTreePageIndex* newpage, BaseTaskInfo* context, VError& err)
{
	fNbLevel++;
	fPagePath[fNbLevel-1].fPage = newpage;
	BTitemIndex* key = newpage->GetItemPtr(0);
	if (key->souspage != -1)
	{
		fPagePath[fNbLevel-1].fPos = -1;
		BTreePageIndex* sousBT = newpage->GetSousPage(fCurstack, 0, err, context);
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


Boolean IndexKeyIterator::_MatchDelWithCurKey()
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
				BTitemIndex* keyintrans = fCurDeletedKey->RetainKey();
				sLONG res = fInd->CompareKeys(fKey, keyintrans, fStrictOptions);
				if (res == CR_EQUAL)
				{
					if (fRecNum < keyintrans->GetQui())
						res = CR_SMALLER;
					else
					{
						if (fRecNum > keyintrans->GetQui())
							res = CR_BIGGER;
					}
				}

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
				if (keyintrans != nil)
					keyintrans->Unuse();
			}
			else
				fCleDelEnTrans = false;
		} while (OneMoreTime);
	}

	return match;
}


void IndexKeyIterator::_MatchAddWithCurKey()
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
				const BTitemIndex* keyintrans = fKeyInTrans;
				if (fFilter != nil && !fFilter->isOn(keyintrans->GetQui()))
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
						res = fInd->CompareKeys(fKey, keyintrans, fStrictOptions);
						if (res == CR_EQUAL)
						{
							if (fRecNum < keyintrans->GetQui())
								res = CR_SMALLER;
							else
							{
								if (fRecNum > keyintrans->GetQui())
									res = CR_BIGGER;
							}
						}
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


VError IndexKeyIterator::_IncCurPos(sLONG* CurElemToProgress)
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


Boolean IndexKeyIterator::NextKey(BaseTaskInfo* context, VError& err, sLONG* CurElemToProgress)
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
				err = _IncCurPos(CurElemToProgress);
				if (err == VE_OK)
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
						err = _IncCurPos(CurElemToProgress);
						if (err != VE_OK)
						{
							nextpos = false;
							break;
						}
						subloopcluster = false;
						fRecNum = fSelIter.NextRecord();
						if (fRecNum != -1)
						{
							if (_MatchDelWithCurKey())
								subloopcluster = true;
							else
							{
								if (testAssert(fKey != nil))
									fKey->SetQui(fRecNum);
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
					err = _IncCurPos(CurElemToProgress);
					if (err == VE_OK)
					{
						BTreePageIndex* curpage = nil;
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
								BTitemIndex* key = curpage->GetItemPtr(curpos);
								if (key->souspage != -1 && !fromSubLevel)
								{
									fromSubLevel = false;
									fPagePath[fNbLevel-1].fPos = curpos-1;
									BTreePageIndex* sousBT = curpage->GetSousPage(fCurstack, curpos, err, context);
									if (err == VE_OK)
									{
										_PosToFirstLeft(sousBT, context, err);
										OneMoreTime = true;
									}
								}
								else
								{
									fromSubLevel = false;
									_DisposeKey();
									fKey = curpage->CopyKey(curpos, &keyprealloc);
									sLONG qui = fKey->GetQui();
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
										Selection* sel = fInd->GetClusterSel(fCurstack)->GetSel(fCurstack, qui, context, err);
										if (err == VE_OK)
											err = _SetSel(sel);
										if (err == VE_OK)
										{
											OneMoreTime = true;
											/*
											fRecNum = fSelIter.NextRecord();
											fKey->qui = fRecNum;
											if (fRecNum == -1)
											{
												OneMoreTime = true;
											}
											else
												ok = true;
											*/
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
									BTreePageIndex* sousBT = curpage->GetSousPage(fCurstack, curpos+1, err, context);
									if (err == VE_OK)
									{
										if (sousBT != nil)
										{
											_PosToFirstLeft(sousBT, context, err);
											OneMoreTime = true;
											Remonte = false;
										}
									}
									else
										Remonte = false;
								}

								if (Remonte)
								{
									curpage->Free(fCurstack, true);
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
	
uLONG IndexInfo::sIndexInfoStamp = 1;
VCriticalSection IndexInfo::sIndexInfoStampMutex;

uLONG IndexInfo::GetNewIndStamp()
{
	VTaskLock lock(&sIndexInfoStampMutex);
	sIndexInfoStamp++;
	return sIndexInfoStamp;
}


uLONG IndexInfo::GetCurIndStamp()
{
	VTaskLock lock(&sIndexInfoStampMutex);
	return sIndexInfoStamp;
}


bool IndexInfo::IsInvalidOnDisk()
{
	if (header != nil)
		return header->IsInvalidOnDisk();
	else
		return false;
}

void IndexInfo::SetInvalidOnDisk()
{
	if (header != nil)
		header->SetInvalidOnDisk();
}


void IndexInfo::SetValidOnDisk()
{
	if (header != nil)
		header->SetValidOnDisk();
}


Boolean IndexInfo::AskForValid(BaseTaskInfo* context, Boolean CheckAlsoEnCreation) const
{ 
	Boolean res; 
	LockValidity(); 
	res = !Invalid && (fDelayRequestCount == 0);

	if (CheckAlsoEnCreation)
		res = res && !encreation;

	if (res && context != nil)
	{
		Transaction* trans = context->GetCurrentTransaction();
		if (trans != nil)
		{
			if (trans->GetIndexStartingStamp() < fStamp)
				res = false;
		}
	}

	if (res)
		fValidityRequestCount++;
	UnLockValidity(); 

#if debugLeakCheck_IndexValid
	if (debug_candumpleaks)
		DumpRefStackCrawls();
	if (res)
		RegisterRefStackCrawl(this);
#endif

	return res; 
}



Boolean IndexInfo::AskForValidOutsideContext(BaseTaskInfo* context, Boolean CheckAlsoEnCreation) const
{ 
	Boolean res; 
	LockValidity(); 
	res = !Invalid && (fDelayRequestCount == 0);

	if (CheckAlsoEnCreation)
		res = res && !encreation;

	// Ne semble pas facile a comprendre! En fait si l'index est valid hors transaction, 
	// mais pas pour cette transaction, alors c'est qu'il a ete cree apres le debut de la transaction
	// dans ce cas, il faut mettre a jour les cle d'index
	if (res)
	{
		if (context == nil)
			res = false;
		else
		{
			Transaction* trans = context->GetCurrentTransaction();
			if (trans != nil)
			{
				if (trans->GetIndexStartingStamp() >= fStamp)
					res = false;
			}
			else
				res = false;
		}
	}

	if (res)
		fValidityRequestCount++;
	UnLockValidity(); 

#if debugLeakCheck_IndexValid
	if (debug_candumpleaks)
		DumpRefStackCrawls();
	if (res)
		RegisterRefStackCrawl(this);
#endif

	return res; 
}


void IndexInfo::IncAskForValid()
{
	LockValidity(); 
	assert(fValidityRequestCount > 0);
	fValidityRequestCount++;
	UnLockValidity(); 
}

void IndexInfo::ReleaseValid()
{
#if debugLeakCheck_IndexValid
	UnRegisterRefStackCrawl(this);
#endif

	LockValidity(); 
	assert(fValidityRequestCount > 0);
	fValidityRequestCount--;
	if (fValidityRequestCount == 0)
	{
		if (fValidityWaitingEvent != nil)
		{
			fValidityWaitingEvent->Unlock();
			fValidityWaitingEvent->Release();
			fValidityWaitingEvent = nil;
		}
	}
	UnLockValidity(); 
}


void IndexInfo::SetInvalid()
{
	bool okset = false;

	do 
	{
		vxSyncEvent* event = nil;

		LockValidity();
		if (fValidityRequestCount > 0)
		{
			if (fValidityWaitingEvent == nil)
			{
				fValidityWaitingEvent = new vxSyncEvent;
			}
			event = fValidityWaitingEvent;
			event->Retain();
		}
		else
		{
			okset = true;
			Invalid = true;
		}
		UnLockValidity();

		if (event != nil)
		{
			event->Wait();
			event->Release();
		}

	} while(!okset);
}

void IndexInfo::SetValid()
{ 
	Invalid = false; 
}



Boolean IndexInfo::WaitEndOfQuickBuilding()
{
	fQuickBuildingWaitingEvent.Wait();
	return true;
}


bool IndexInfo::FreeMem(sLONG allocationBlockNumber, VSize combien, VSize& outSizeFreed)
{
	bool okfree = false;
	VSize tot = 0;
	if (header != nil)
	{
#if FreeMemWaitOnLockWrite
		if (fAccess.TryToLockFreeMem())
#endif
		{
			header->TryToFreeMem(allocationBlockNumber, combien, tot);
			ClearMemRequest();
#if FreeMemWaitOnLockWrite
			fAccess.Unlock(true);
#endif
			okfree = true;
		}
	}
	else
	{
		ClearMemRequest();
		okfree = true;
	}
	outSizeFreed = tot;
	return okfree;
}


VError IndexInfo::PutInto(VStream& buf, Base4D* xdb, Boolean WithHeader)
{
	VError err = fID.WriteToStream(&buf);
	if (err == VE_OK)
		err = fName.WriteToStream(&buf);
	if (err == VE_OK)
	{
		sLONG xauto = 0;
		if (fIsAuto)
			xauto = 1;
		err = buf.PutLong(xauto);
	}
	return err;
}


VError IndexInfo::GetFrom(VStream& buf, Base4D* xdb, Boolean WithHeader, Boolean oldformat)
{
	//fIsQuickBuilding = false;
	ClearQuickBuilding();
	VError err = fID.ReadFromStream(&buf);
	if (err == VE_OK)
	{
		fIDisChosen = true;
		err = fName.ReadFromStream(&buf);
	}
	if (err == VE_OK)
	{
		sLONG xauto = 0;
		//if (!xdb->MustRebuildAllIndex())
			err = buf.GetLong(xauto);
		if (xauto != 0)
			fIsAuto = true;
	}
	return err;
}


VError IndexInfo::SetName(const VString& name, CDB4DBaseContext* inContext)
{
	VError err = VE_OK;

	if (fIsRemote)
	{
		IRequest *req = GetDB()->CreateRequest( inContext, Req_SetIndexName + kRangeReqDB4D);
		if (req == NULL)
		{
			err = ThrowBaseError(memfull, DBaction_ChangingIndexName);
		}
		else
		{
			req->PutBaseParam( GetDB());
			req->PutIndexParam( this);
			req->PutStringParam( name);

			err = req->GetLastError();
			if (err == VE_OK)
			{
				err = req->Send();
				if (err == VE_OK)
				{
					fName = name;
				}
			}
			req->Release();
		}
	}
	else
	{
		if (name.GetLength() != 0)
		{
			IndexInfo* other = nil;//bd->RetainIndexByName(name);
			if (other != nil)
			{
				if (other != this)
				{
					err = ThrowError(VE_DB4D_INDEXNAMEDUPLICATE, DBactionFinale);
				}
				other->Release();
			}
		}

		if (err == VE_OK)
		{
			fName = name;
			SaveInStruct(false);
			ModifyIt(nil);
		}
	}

	return err;
}


BTitemIndex* IndexInfo::AllocateKey(sLONG len)
{
	void* p = GetFastMem(len, true, 'iKe1');
	BTitemIndex* u = nil;
	if (p != nil)
	{
		u = new (p) BTitemIndex();
		u->sousBT = nil;
	}
	return u;
}


BTitemIndex* IndexInfo::CopyKey(const BTitemIndex* key, void* prealloc)
{
	sLONG len = CalulateFullKeyLengthInMem(key);
	BTitemIndex* v;
	if (prealloc == nil || len >= KeyPreAllocSize)
		v = AllocateKey(len);
	else
		v = (BTitemIndex*)prealloc;
	if (v != nil)
	{
		vBlockMove(&key->souspage, &v->souspage, len - sizeof(BTitemIndex*) /* on ne copie pas le pointer de souspage */);
		v->sousBT = nil;
	}

	return v;
}


ValPtr IndexInfo::CreateVValueWithKey(const BTitemIndex* key, VError &err)
{
	err = VE_DB4D_NOTIMPLEMENTED;
	return nil;
}

void IndexInfo::ModifyIt(BaseTaskInfo* context)
{ 
	if (testAssert(!fIsRemote))
	{
		Save();
	}
}

IndexInfo::~IndexInfo()
{ 
	if (header != nil)
	{
		RemoveFromCache();
		Open(index_write);
		OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
		header->LibereEspaceMem(curstack);
		Close();
		delete header;
	}

/*
	if (sourcelang_isinited)
	{
		CLanguage* language = VDBMgr::GetLanguage();
		if (language != nil)
		{
			language->ResetArray(nil, &fSourceLang);
		}
	}
*/
#if 0
	bd->checkIndexInMap(this);
#endif

}


uBOOL IndexInfo::MayBeSorted(void)
{
	if (header != nil)
		return header->MayBeSorted();
	else
	{
#if 0
		// a faire quand il y aura les index hash
		switch (typindex)
		{
			/*
			case hcode:
				return false;
				*/
			default:
				return true;

		}
#endif
		return true;
	}
}


VError IndexInfo::LoadFromBagWithLoader( const VValueBag& inBag, VBagLoader *inLoader, void* inContext)
{
	VError err = VE_OK;

	if (fIsRemote)
		err = ThrowError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
	else
	{
		// le type de l'index (simple/multiple) a deja ete fixe pendant la creation de l'objet (cf CreateIndexInfo)

		// ici on relit l'algo de l'index (btree, hash, ...)
		inBag.GetLong( DB4DBagKeys::type, typindex);

		// d'autres infos
		inBag.GetBoolean( DB4DBagKeys::unique_keys, fUniqueKeys);

		// name is optionnal
		if (!inBag.GetString( DB4DBagKeys::name, fName))
			fName.Clear();

		// uuid is optionnal for an index
		if (inLoader->GetUUID( inBag, fID))
			fIDisChosen = true;

		sLONG xtypindex = typindex;

		if (typindex == DB4D_Index_AutoType)
		{
			fIsAuto = true;
			xtypindex = DB4D_Index_Btree;
			Field* crit = nil;
			if (GetTyp() != DB4D_Index_OnMultipleFields)
			{
				crit = ((IndexInfoFromField*)this)->GetField();
			}
			if (crit != nil)
			{
				if (crit->GetTyp() == DB4D_Boolean || crit->GetTyp() == DB4D_Byte)
				{
					xtypindex = DB4D_Index_BtreeWithCluster;
				}
			}
		}
		IndexHeader *h = CreateIndexHeader( xtypindex, this, GetScalarKind());
		if (h == NULL)
			err = ThrowError( VE_DB4D_INVALIDINDEX, DBaction_CreatingIndex);
		else
		{
			if (typindex == DB4D_Index_AutoType) // sc 05/06/2008 ACI0058163, sanity check
			{
				fIsAuto = true;
				typindex = h->GetRealType();
			}

			this->header = h;
		}
	}

	return err;
}



VError IndexInfo::SaveToBag( VValueBag& ioBag, VString& outKind) const
{
	// regular/keywords
	if ( (ReduceType(InfoTyp) == DB4D_Index_OnOneField) || (InfoTyp == DB4D_Index_OnMultipleFields) )
		ioBag.SetString( DB4DBagKeys::kind, CVSTR( "regular"));
	else if (testAssert( InfoTyp == DB4D_Index_OnKeyWords))
		ioBag.SetString( DB4DBagKeys::kind, CVSTR( "keywords"));

	DB4DBagKeys::unique_keys.Set( &ioBag, fUniqueKeys);
	DB4DBagKeys::name.Set( &ioBag, fName);
	DB4DBagKeys::uuid.Set( &ioBag, fID);

	// btree, hash, cluster, ...
	// sc 20/01/2009 ACI0060582, remote indexes have not header
	if (fIsRemote)
	{
		ioBag.SetLong( DB4DBagKeys::type, GetHeaderType());
	}
	else if (header != NULL)
	{
		ioBag.SetLong( DB4DBagKeys::type, header->GetTyp());
	}

	outKind = L"index";
	
	return VE_OK;
}


VError IndexInfo::ThrowError( VError inErrCode, ActionDB4D inAction)
{
	VErrorDB4D_OnIndex *err = new VErrorDB4D_OnIndex(inErrCode, inAction, bd, this);
	VTask::GetCurrent()->PushRetainedError( err);
	
	return inErrCode;
}


Boolean IndexInfo::MatchIndex(FieldsArray* fields, sLONG NbFields)
{
	return false;
}


Bittab* IndexInfo::FindKeyInArray(DB4DArrayOfValues* values, BaseTaskInfo* context, VError& err, 
								  VDB4DProgressIndicator* InProgress, const VCompareOptions& inOptions, Bittab* dejasel)
{
	Bittab* b = nil;

	if (fIsRemote)
		err = ThrowError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
	else
	{
		VCompareOptions options = inOptions;
		options.SetIntlManager(GetContextIntl(context));
		if (AskForValid(context))
		{
			Open(index_read);
			OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
			b = header->FindKeyInArray(curstack, values, context, err, InProgress, options, dejasel);
			Close();
			ReleaseValid();
		}
		else
			ThrowError(VE_DB4D_INVALIDINDEX, DBaction_SearchingInIndex);

		if (err == VE_OK && b != nil)
		{
			Transaction* trans = GetCurrentTransaction(context);
			if (trans != nil)
			{
				if (values->GetSignature() == 'cons')
				{
					mapIndexKeys* vals;
					vals = (mapIndexKeys*)trans->GetDeletedKeys(this, false, err);
					if (vals != nil)
					{
						for (mapIndexKeys::const_iterator cur = vals->begin(), end = vals->end(); cur != end; cur++)
						{
							BTitemIndex* btkey = cur->RetainKey();
							if (btkey != nil)
							{
								if (values->FindWithDataPtr(&(btkey->data), inOptions))
								{
									b->Clear(btkey->qui);
								}
								btkey->Unuse();
							}
						}
					}

					vals = (mapIndexKeys*)trans->GetSavedKeys(this, false, err);
					if (vals != nil)
					{
						for (mapIndexKeys::const_iterator cur = vals->begin(), end = vals->end(); cur != end; cur++)
						{
							BTitemIndex* btkey = cur->RetainKey();
							if (btkey != nil)
							{
								if (values->FindWithDataPtr(&(btkey->data), inOptions))
								{
									b->Set(btkey->qui);
								}
								btkey->Unuse();
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


VError IndexInfo::FindKeyAndBuildPath(BTitemIndex* val, const VCompareOptions& inOptions, BaseTaskInfo* context, VDB4DIndexKey* outKey)
{
	VError err;
	if (fIsRemote)
		err = ThrowError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
	else
	{
		VCompareOptions options = inOptions;
		options.SetIntlManager(GetContextIntl(context));
		if (AskForValid(context))
		{
			Open(index_read);
			OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
			err = header->FindKeyAndBuildPath(curstack, val, options, context, outKey);
			Close();
			ReleaseValid();
		}
		else
			err = ThrowError(VE_DB4D_INVALIDINDEX, DBaction_SearchingInIndex);

	}
	return err;
}


VError IndexInfo::NextKey(const VDB4DIndexKey* inKey, BaseTaskInfo* context, VDB4DIndexKey* outKey)
{
	VError err;
	if (fIsRemote)
		err = ThrowError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
	else
	{
		if (AskForValid(context))
		{
			Open(index_read);
			OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
			err = header->NextKey(curstack, inKey, context, outKey);
			Close();
			ReleaseValid();
		}
		else
			err = ThrowError(VE_DB4D_INVALIDINDEX, DBaction_SearchingInIndex);
	}
	return err;
}


Selection* IndexInfo::ScanIndex(sLONG inMaxRecords, Boolean KeepSorted, Boolean Ascent, BaseTaskInfo* context, VError& err, Selection* filtre, VDB4DProgressIndicator* InProgress)
{
	Selection* result = nil;
	err = VE_OK;
	if (fIsRemote)
		err = ThrowError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
	else
	{
		if (AskForValid(context))
		{
			Table* target = GetTargetTable();
			sLONG nbrecsInTable = target->GetDF()->GetNbRecords(context, false);
			Open(index_read);
			OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
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
						gs(1005,29,session_title);
						IndexKeyIterator iter(this, bfiltre, context, InProgress, session_title, curstack);

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
			Close();
			ReleaseValid();

		}
		else
			err = ThrowError(VE_DB4D_INVALIDINDEX, DBaction_SearchingInIndex);

	}

	if (err != VE_OK)
	{
		err = ThrowError(VE_DB4D_CANNOTBUILDSELECTION, DBaction_ScanningIndex);
	}

	return result;
}


Bittab* IndexInfo::Fourche(BTitemIndex* val1, uBOOL xstrict1, BTitemIndex* val2, uBOOL xstrict2, BaseTaskInfo* context, VError& err, 
													 VDB4DProgressIndicator* InProgress, const VCompareOptions& inOptions, Bittab* dejasel, BTitemIndex** outVal)
{
	Bittab* b = nil;
	
	if (fIsRemote)
		err = ThrowError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
	else
	{
		VCompareOptions options = inOptions;
		options.SetIntlManager(GetContextIntl(context));
		if (AskForValid(context))
		{
			Open(index_read);
			OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
			b = header->Fourche(curstack, val1, xstrict1, val2, xstrict2, context, err, InProgress, options, dejasel, outVal);
			Close();
			ReleaseValid();
		}
		else
			err = ThrowError(VE_DB4D_INVALIDINDEX, DBaction_SearchingInIndex);

		Transaction* trans = GetCurrentTransaction(context);
		if (trans != nil && err == VE_OK)
		{
			assert(b != nil);
			err = trans->Fourche(this, val1, xstrict1, val2, xstrict2, options, dejasel, b, outVal);
		}
	}

	return(b);
}


Bittab* IndexInfo::ParseAll(BTitemIndex* val1, BaseTaskInfo* context, VError& err, const VCompareOptions& inOptions, VDB4DProgressIndicator* InProgress)
{
	Bittab* b = nil;

	if (fIsRemote)
		err = ThrowError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
	else
	{
		VCompareOptions options = inOptions;
		options.SetIntlManager(GetContextIntl(context));
		if (AskForValid(context))
		{
			Open(index_read);
			OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
			b = header->ParseAll(curstack, val1, context, err, options, InProgress);
			Close();
			ReleaseValid();
		}
		else
			err = ThrowError(VE_DB4D_INVALIDINDEX, DBaction_SearchingInIndex);

		Transaction* trans = GetCurrentTransaction(context);
		if (trans != nil && err == VE_OK)
		{
			assert(b != nil);
			err = trans->ParseAllIndex(this, val1, options, b);
		}
	}

	return(b);
}


sLONG8 IndexInfo::ComputeMaxKeyToFit(sLONG level, sLONG8 MaxElemsInPage, sLONG8 RequiredElemsInPage, Boolean CheckAllTree)
{
	sLONG i;
	sLONG8 tot;
	if (level == 0)
		tot = 1;
	else
	{
		if (level == 1)
			tot = MaxElemsInPage;
		else
		{
			tot = RequiredElemsInPage;
			for (i = 1; i < level; i++)
			{
				sLONG8 x;
				if ((i == (level - 1)) && CheckAllTree)
					x = MaxElemsInPage; // la page primaire peut depasser le RequiredElemsInPage;
				else
					x = RequiredElemsInPage;
				tot = (x+1) * tot + x;
			}
		}
	}

	return tot;
}

sLONG IndexInfo::CalculateGenerationsLevels(sLONG8 nbrec, sLONG MaxElemsInPage, sLONG RequiredElemsInPage)
{
	assert(RequiredElemsInPage<=MaxElemsInPage);
	sLONG level = 1;
	while (ComputeMaxKeyToFit(level, MaxElemsInPage, RequiredElemsInPage, true) < nbrec)
	{
		level++;
	}
	return level;
	assert(level<20);
}


VError IndexInfo::GenereFullIndex(VDB4DProgressIndicator* InProgress, sLONG MaxElemsInPage, sLONG RequiredElemsInPage)
{
	VError err = VE_OK;
	if (fIsRemote)
		err = ThrowError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
	else
	{
		BaseTaskInfo *context = new BaseTaskInfo(bd, nil, nil, nil);

	#if trackIndex
		uLONG t = VSystem::GetCurrentTime();
		trackDebugMsg(L"start building index\n");
	#endif
		SetBuildError(VE_OK);
		err = ValidateIndexInfo();
		Boolean wasuniqkeys = fUniqueKeys;
		if (err == VE_OK)
		{
			if (InProgress != nil)
			{
				VString iname;
				IdentifyIndex(iname, true, true);
				XBOX::VString session_title;
				gs(1005,9,session_title);	// Building
				session_title += iname;
				InProgress->SetMessage(session_title);
				//InProgress->BeginSession(-1,session_title);
			}

			if (GenereQuickIndex(err, context, InProgress, MaxElemsInPage, RequiredElemsInPage))
			{
				LockValidity();
				FinCreation();
				//fIsQuickBuilding = false;
				UnLockValidity();		
				ClearQuickBuilding();
				// l'index a ete genere en memoire, plus rien a faire
			}
			else
			{
				/*
				Open();
				fIsQuickBuilding = false;
				Close();
				*/
				ClearQuickBuilding();

				if (err == VE_OK || err == memfull)
				{
					err = GenereIndex(context, InProgress);
					if (err != VE_OK)
					{
						OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
						GetHeader()->LibereEspaceDisk(curstack, InProgress);
					}
				}
				else
				{
					//assert(false);
				}
			}

			/*if (InProgress != nil)
				InProgress->EndSession();*/
		}
		
		ReleaseRefCountable( &context);
#if 0
		if (err == VE_OK)
		{
			if (wasuniqkeys != fUniqueKeys)
			{
				err = VE_DB4D_INDEXKEYS_ARE_NOT_UNIQUE;
			}
		}
#endif

	#if trackIndex
		uLONG t2 = VSystem::GetCurrentTime();
		VStr<30> s;
		VDuration x;
		x.FromLong(t2-t);
		s.FromDuration(x);
		trackDebugMsg(" Index built in "+s+" milliseconds \n");
	#endif
	}
	return err;
}


VError IndexInfo::GetDistinctValues(DB4DCollectionManager& outCollection, BaseTaskInfo* context, Bittab* filtre, VDB4DProgressIndicator* InProgress, const VCompareOptions& inOptions)
{
	return VE_DB4D_NOTIMPLEMENTED;
}


VError IndexInfo::Save()
{
	VError err = VE_OK;
	StructElemDef* e = nil;

	if (fIsRemote)
		err = ThrowError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
	else
	{
		if (fIsOKtoSave)
		{
			xbox::VPtrStream buf;
			err=buf.OpenWriting();

			buf.SetNeedSwap(false);

			sLONG IndexInfoTyp = InfoTyp;
			err=buf.PutLong(IndexInfoTyp);
			if (err==VE_OK)
			{
				err=PutInto(buf, bd, true);
			}

			if (err == VE_OK)
			{
				if (PosInMap == -1)
				{
					e = new StructElemDef(bd, -1, DBOH_IndexDefElem, ObjCache::IndexDefAccess);
					if (e == nil)
						err = ThrowError(memfull, DBaction_SavingIndexHeader);
				}
				else
				{
					e = bd->LoadIndexDef(PosInMap, err);
					if (err == VE_OK)
					{
						if (e == nil)
						{
							e = new StructElemDef(bd, -1, DBOH_IndexDefElem, ObjCache::IndexDefAccess);
							if (e == nil)
								err = ThrowError(memfull, DBaction_SavingIndexHeader);
						}
					}
				}
			}

			if (err == VE_OK)
			{
				err = e->CopyData(buf.GetDataPtr(), buf.GetSize());
				if (err == VE_OK)
				{
					err = bd->SaveIndexDef(e);
					if (err == VE_OK)
					{
						PosInMap = e->GetNum();
					}
	#if 0
					if (!GetDebugDeja())
					{
						if (IndexInfoTyp == DB4D_Index_OnOneField ||IndexInfoTyp == DB4D_Index_OnKeyWords)
						{
							IndexInfoFromField* ind = (IndexInfoFromField*)this;
							VStr<50> s;
							VStr<255> smess;
							ind->GetTable()->GetName(s);
							smess = L"Saving index on ";
							smess+=s;
							ind->GetField()->GetName(s);
							smess+=L".";
							smess+=s;
							s.FromLong(PosInMap);
							smess+=L"  : pos = ";
							smess+=s;
							smess+=L" , adresse = ";
							s.FromLong8(e->getaddr());
							smess+=s;
							smess+=L"\n";
							DebugMsg(smess);
						}
					}
	#endif
				}
			}

			if (e != nil)
				e->libere();

			buf.CloseWriting();
		}
	}

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTSAVEINDEXHEADER, DBaction_SavingIndexHeader);
	return err;
}



VError IndexInfo::SaveInStruct(Boolean ForceSave)
{
	VError err = VE_OK;
	StructElemDef* e = nil;

	if (fIsRemote)
		err = ThrowError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
	else
	{
		if (bd->StoredAsXML())
		{
			//bd->TouchXML();
		}
		else
		{
			if (bd->GetStructure() != nil && (fIsOKtoSave || ForceSave))
			{
				xbox::VPtrStream buf;
				err=buf.OpenWriting();
				buf.SetNeedSwap(false);

				sLONG IndexInfoTyp = InfoTyp;
				err=buf.PutLong(IndexInfoTyp);
				if (err==VE_OK)
				{
					err=PutInto(buf, bd, false);
				}

				if (err == VE_OK)
				{
					if (PosInMapInStruct == -1)
					{
						e = new StructElemDef(bd->GetStructure(), -1, DBOH_IndexInStructDefElem, ObjCache::IndexDefAccess);
						if (e == nil)
							err = ThrowError(memfull, DBaction_SavingIndexHeader);
					}
					else
					{
						e = bd->LoadIndexDefInStruct(PosInMapInStruct, err);
						if (err == VE_OK)
						{
							if (e == nil)
							{
								e = new StructElemDef(bd->GetStructure(), -1, DBOH_IndexInStructDefElem, ObjCache::IndexDefAccess);
								if (e == nil)
									err = ThrowError(memfull, DBaction_SavingIndexHeader);
							}
						}
					}
				}

				if (err == VE_OK)
				{
					err = e->CopyData(buf.GetDataPtr(), buf.GetSize());
					if (err == VE_OK)
					{
						err = bd->SaveIndexDefInStruct(e);
						if (err == VE_OK)
						{
							PosInMapInStruct = e->GetNum();
						}
					}
				}

				if (e != nil)
					e->libere();
				buf.CloseWriting();
			}

		}

	}

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTSAVEINDEXHEADER, DBaction_SavingIndexHeader);

	return err;
}



									
									/* -----------------------------------------------  */


IndexInfoFromField::IndexInfoFromField(Base4D* db, sLONG xnumfile, sLONG xnumfield, sLONG xtypindex, Boolean UniqueKeys, Boolean CanBeScalar ):IndexInfo(UniqueKeys)
{
	IndexHeader *indH;
	
	fDataKind = -1;
	SetDB(db);

	bd->occupe();
	fic=bd->RetainTable(xnumfile);
	if (fic != nil)
	{
		crit=fic->RetainField(xnumfield);
		if (crit != nil)
		{
			fDataKind = crit->GetTyp();
			fUniqueKeys = fUniqueKeys || crit->GetUnique();
		}
	}
	else
		crit = nil;
	bd->libere();
	if (xtypindex == DB4D_Index_AutoType)
	{
		fIsAuto = true;
		xtypindex = DB4D_Index_Btree;
		if (crit != nil)
		{
			if (crit->GetTyp() == DB4D_Boolean || crit->GetTyp() == DB4D_Byte)
			{
				xtypindex = DB4D_Index_BtreeWithCluster;
			}
		}
	}
	typindex = xtypindex;
	sLONG scalartype = -1;
	if (CanBeScalar)
		scalartype = fDataKind;
	indH=CreateIndexHeader(xtypindex, this, scalartype);
	header=indH;
	InfoTyp=DB4D_Index_OnOneField;
}


IndexInfoFromField::~IndexInfoFromField()
{
	if (crit != nil) 
		crit->Release();
	if (fic != nil) 
		fic->Release();
}


void IndexInfoFromField::GetDebugString(VString& s, sLONG numinstance)
{
	s.Clear();
	if (crit != nil)
	{
		VString s2;
		crit->GetOwner()->GetName(s);
		if (numinstance != 0)
			s+=L"("+ToString(numinstance)+L")";
		s.AppendUniChar(CHAR_FULL_STOP);
		crit->GetName(s2);
		s.AppendString(s2);
	}
}


void IndexInfoFromField::IdentifyIndex(VString& outString, Boolean WithTableName, Boolean WithIndexName) const
{
	gs(1004,28,outString);	// Index

	if (!fName.IsEmpty() && WithIndexName)
	{
		outString += L"(";
		outString += fName;
		outString += L")";
	}

	outString += L" ";
	VString liaison;
	gs(1004,24,liaison);	// on
	outString += liaison;
	outString += L" ";

	VString s2,s;
	if (WithTableName)
	{
		crit->GetOwner()->GetName(s);
		s.AppendUniChar(CHAR_FULL_STOP);
	}
	crit->GetName(s2);
	s.AppendString(s2);

	outString += s;
}


VError IndexInfoFromField::ValidateIndexInfo()
{
	VError err = VE_OK;

	if (crit == nil)
		err = VE_DB4D_WRONGFIELDREF;
	else
	{
		fDataKind = crit->GetTyp();
		if (!crit->IsIndexable())
			err = VE_DB4D_INVALIDINDEXTYPE;
	}

	return err;
}



VError IndexInfoFromField::ClearFromSystemTableIndexCols()
{
	VObjLock lock(this);
	return bd->DelIndexCol(this);
}


VError IndexInfoFromField::AddToSystemTableIndexCols()
{
	VObjLock lock(this);
	VError err = VE_OK;
	if (crit != nil)
	{
		VUUID xid;
		crit->GetUUID(xid);
		err = bd->AddIndexCol(this, xid, 1);
	}
	return err;
}


Table* IndexInfoFromField::GetTargetTable()
{
	return fic;
}


Boolean IndexInfoFromField::NeedToRebuildIndex(VIntlMgr* NewIntlMgr)
{
	Boolean result = false;
	if (crit != nil)
	{
		sLONG typ = crit->GetTyp();
		if (typ == VK_STRING || typ == VK_TEXT || typ == VK_STRING_UTF8 || typ == VK_TEXT_UTF8 || typ == VK_IMAGE)
			result = true;
	}
	return result;
}


bool IndexInfoFromField::MustBeRebuild()
{
	Boolean result = false;
	if (crit != nil)
	{
		sLONG typ = crit->GetTyp();
		if (typ != fDataKind)
			result = true;
	}
	return result;
}


void IndexInfoFromField::CreateHeader()
{
	assert(!fIsRemote);
	if (header == nil)
	{
		header=CreateIndexHeader(typindex,this, -1);
	}
	else
		header->Update(fic);
}


Boolean IndexInfoFromField::CanBeScalarConverted() const
{
	switch (fDataKind) 
	{
		case VK_WORD:
		case VK_BYTE:
		case VK_BOOLEAN:
		case VK_LONG:
		case VK_REAL:
		case VK_SUBTABLE:
		case VK_SUBTABLE_KEY:
		case VK_LONG8:
		case VK_DURATION:
		case VK_UUID:
		case VK_TIME:
			return true;
			break;

	}
	return false;

}

IndexInfo* IndexInfoFromField::ConvertToScalar() const
{
	IndexInfoFromField* ind = CreateEmptyIndexInfoFieldScalar(fDataKind);
	if (testAssert(ind != nil))
	{
		ind->fDataKind = fDataKind;
		ind->fUniqueKeys = fUniqueKeys;
		ind->crit = crit;
		if (crit != nil)
			crit->Retain();
		ind->fic = fic;
		if (fic != nil)
			fic->Retain();
		ind->fIsAuto = fIsAuto;
		ind->typindex = typindex;

		ind->fName = fName;
		ind->fID = fID;

		ind->PosInMap = PosInMap;
		ind->PosInMapInStruct = PosInMapInStruct;
		ind->bd = bd;
		ind->encreation = encreation; 
		ind->Invalid =Invalid;
//		ind->sourcelang_isinited = sourcelang_isinited;
		ind->fDelayRequestCount = fDelayRequestCount;
		ind->fIsOKtoSave = fIsOKtoSave;
		ind->fIDisChosen = fIDisChosen;
//		ind->fSourceLang = fSourceLang;
		ind->fDebugDeja = fDebugDeja;
		ind->fStamp = fStamp;
		ind->fValidityRequestCount = fValidityRequestCount;
		//ind->fIntlMgr = fIntlMgr;
		ind->fUpdateOptions = fUpdateOptions;
	}

	return ind;
}


/*
sLONG IndexInfoFromField::GetLen(void)
{
	return(IndexInfo::GetLen()+sizeof(IndexInfoFromFieldOnDisk)+4+header->GetLen());
}
*/

VError IndexInfoFromField::PutInto(VStream& buf, Base4D* xdb, Boolean WithHeader)
{
	VError err;
	
	err = IndexInfo::PutInto(buf, xdb);
	if (err == VE_OK)
	{
		if (err==VE_OK) 
			err=buf.PutLong(typindex); 

		sLONG uniq = fUniqueKeys ? 1 : 0;
		if (err == VE_OK)
			err = buf.PutLong(uniq);

		if (err==VE_OK) 
			err=buf.PutLong(fDataKind);

		VUUID xid;
		if (crit != nil)
			crit->GetUUID(xid);
		else
			xid.Clear();
		if (err == VE_OK)
			err = xid.WriteToStream(&buf);

		if (err==VE_OK && WithHeader) 
			err=header->PutInto(buf);
	}
	
	return(err);
}

/*
void IndexInfoFromFieldOnDisk::SwapBytes()
{
	ByteSwap(&typindex);
	ByteSwap(&numfile);
	ByteSwap(&numfield);
}
*/


VError IndexInfoFromField::GetFrom(VStream& buf, Base4D *xdb, Boolean WithHeader, Boolean oldformat)
{
	VError err;
	
	SetDB(xdb);

	err = IndexInfo::GetFrom(buf, xdb, WithHeader);
	if (err == VE_OK)
	{
		sLONG uniq;
		if (err==VE_OK) 
			err=buf.GetLong(typindex);
		if (err==VE_OK) 
			err=buf.GetLong(uniq);
		fUniqueKeys = (uniq == 1);

		if (err==VE_OK) 
			err=buf.GetLong(fDataKind);

		VUUID xid;
		if (err == VE_OK)
			err = xid.ReadFromStream(&buf);

		if (err==VE_OK)
		{
			crit = bd->FindAndRetainFieldRef(xid);
			if (crit != nil)
			{
				fic = crit->GetOwner();
				if (fic != nil)
					fic->Retain();
			}
			else
				fic = nil;

/*
			if( xdb->GetStructure() != nil)			//mbucatariu, pour les Tools, si on ouvre le data sans la structure, fic et crit seront nil
			{
				assert(fic != nil);
				assert(crit != nil);
				if (fic == nil || crit == nil)
					err = VE_DB4D_WRONGFIELDREF;
			}
*/
			if (WithHeader)
			{
				header=CreateIndexHeader(typindex, this, GetScalarKind());
				if (header==nil)
				{
					err = ThrowError(memfull, DBaction_LoadingIndexHeader);
				}
				else
				{
					err=header->GetFrom(buf);
					header->SetAssoc(this);
					if (typindex == DB4D_Index_AutoType) // sc 05/06/2008 ACI0058163, sanity check
					{
						fIsAuto = true;
						typindex = header->GetRealType();
					}
				}
			}
			else
			{
				if (typindex == DB4D_Index_AutoType) // sc 05/06/2008 ACI0058163, sanity check
				{
					fIsAuto = true;
					typindex = DB4D_Index_Btree;
				}

				header = nil;
			}
		}
	}

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTLOADINDEXHEADER, DBaction_LoadingIndexHeader);

	return(err);
}



void IndexInfoFromField::CalculDependence(void)
{
	if (crit)
		crit->AddIndexDep(this);
	if (fic)
		fic->AddIndexDep(this);
}
		

void IndexInfoFromField::DeleteFromDependencies(void)
{
	if (crit)
		crit->DelIndexDep(this);
	if (fic)
		fic->DelIndexDep(this);
}


void IndexInfoFromField::TouchDependencies()
{
	if (testAssert(!fIsRemote))
	{
		if (crit)
			crit->Touch();
	}

}


VError IndexInfoFromField::PlaceCle(OccupableStack* curstack, FicheInMem *rec, BaseTaskInfo* context)
{
	BTitemIndex *val;
	VError err = VE_OK;
	if (fIsRemote)
		err = ThrowError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
	else
	{
		IndexKeyArray tempkey(this);
		
		val=BuildKey(rec, err);
		if (val != nil)
		{
			Transaction* trans = GetCurrentTransaction(context);
			if (trans != nil)
			{
				err = trans->PlaceCle(this, val, rec->GetAssoc()->getnumfic());
			}
			else
			{
				tempkey.AddKey(val);
				err=header->PlaceCle(curstack, val, rec->GetAssoc()->getnumfic(), tempkey, context);
			}
		}
		else
			err = ThrowError(VE_DB4D_INVALID_INDEXKEY, DBaction_InsertingKeyIntoIndex);
	}
	return(err);
}


VError IndexInfoFromField::DetruireCle(OccupableStack* curstack, FicheInMem *rec, BaseTaskInfo* context)
{
	BTitemIndex *val;
	VError err = VE_OK;
	if (fIsRemote)
		err = ThrowError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
	else
	{
		IndexKeyArray tempkey(this);

		val=BuildKey(rec, err, true);
		if (val != nil)
		{
			Transaction* trans = GetCurrentTransaction(context);
			if (trans != nil)
			{
				err = trans->DetruireCle(this, val, rec->GetAssoc()->getnumfic());
			}
			else
			{
				tempkey.AddKey(val);
				err=header->DetruireCle(curstack, val,rec->GetAssoc()->getnumfic(), tempkey, context);
			}
		}
		else
			err = ThrowError(VE_DB4D_INVALID_INDEXKEY, DBaction_DeletingKeyFromIndex);
	}
	return(err);
}



VError IndexInfoFromField::PlaceCleForTrans(OccupableStack* curstack, BTitemIndex* val, BaseTaskInfo* context)
{
	VError err = VE_OK;
	if (fIsRemote)
		err = ThrowError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
	else
	{
		IndexKeyArray tempkey(this);

		val->souspage = -1;
		val->sousBT = nil;
		tempkey.AddKey(val);
		err=header->PlaceCle(curstack, val, val->GetQui(), tempkey, context);
	}

	return(err);
}


VError IndexInfoFromField::DetruireCleForTrans(OccupableStack* curstack, BTitemIndex* val, BaseTaskInfo* context)
{
	VError err = VE_OK;
	if (fIsRemote)
		err = ThrowError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
	else
	{
		IndexKeyArray tempkey(this);

		val->souspage = -1;
		val->sousBT = nil;
		tempkey.AddKey(val);
		err=header->DetruireCle(curstack, val, val->GetQui(), tempkey,context);
	}

	return(err);
}


uBOOL IndexInfoFromField::NeedUpdate(FicheInMem *rec, BaseTaskInfo* context)
{
	return(rec->IsModif(crit));
}


uBOOL IndexInfoFromField::Egal(IndexInfo* autre)
{
	uBOOL isegal;
	IndexInfoFromField *other;
	
	other=(IndexInfoFromField*)autre;
	isegal=false;
	
	if ( (crit==other->crit) && (fic==other->fic) )
	{
		isegal=true;
	}
	
	return(isegal);
}


VError IndexInfoFromField::GenereIndex(BaseTaskInfo* context, VDB4DProgressIndicator* InProgress)
{
	DataTable *FD;
	sLONG i,nb;
	VError err = VE_OK;
	FicheInMem *rec;
	ValPtr cv;
	BTitemIndex* val;
	
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
				IndexKeyArray tempkey(this);
				val = BuildKey(rec, err);
				if (val != nil)
				{
					tempkey.AddKey(val);
					Open(index_write);
					err=header->PlaceCle(curstack, val,i,tempkey, context);
	#if debuglr_page
					debug_checkpages();
	#endif
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


sLONG IndexInfoFromField::GetMaxNB(BaseTaskInfo* context)
{
	sLONG nb = 0;
	DataTable *FD;

	if (testAssert(!fIsRemote))
	{
		FD=fic->GetDF();
		nb=FD->GetMaxRecords(context);
	}
	return(nb);
}


Boolean IndexInfoFromField::MatchIndex(FieldsArray* fields, sLONG NbFields)
{
	Boolean res = false;
	
	if (NbFields == 1)
	{
		if ( (*fields)[1] == crit ) res = true;
	}
	
	return res;
}


VError IndexInfoFromField::LoadFromBagWithLoader( const VValueBag& inBag, VBagLoader *inLoader, void* inContext)
{

	VError err = VE_OK;
	fDataKind = -1;

	if (fIsRemote || bd->IsRemote())
		err = ThrowError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
	else
	{
		if (err == VE_OK)
		{
			const VValueBag *bag = inBag.RetainUniqueElement( DB4DBagKeys::field_ref);
			if (bag != NULL)
			{
				crit = bd->FindAndRetainFieldRef( *bag, inLoader);
				if (crit == NULL)
					err = ThrowError( VE_DB4D_WRONGFIELDREF, DBaction_CreatingIndex);
				else
				{
					fDataKind = crit->GetTyp();
					fic = crit->GetOwner();
					fic->Retain();
				}
				bag->Release();
			}
		}
		if (err == VE_OK)
			err = inherited::LoadFromBagWithLoader( inBag, inLoader, inContext);
	}

	return err;
}


VError IndexInfoFromField::SaveToBag( VValueBag& ioBag, VString& outKind) const
{
	VError err = inherited::SaveToBag( ioBag, outKind);
	if (err == VE_OK)
	{
		// reference du champ
		VValueBag *ref = this->crit->CreateReferenceBag( true);
		if (ref != NULL)
		{
			ioBag.AddElement( DB4DBagKeys::field_ref, ref);
			ref->Release();
		}
	}
		
	return err;
}


sLONG IndexInfoFromField::CalculateDefaultSizeKey()
{
	return (crit != nil) ? crit->CalcAvgSize() : 1;
}


BTitemIndex* IndexInfoFromField::BuildKeyFromVValue(const ValPtr cv, VError &err)
{
	BTitemIndex* res = nil;
	err = VE_OK;
	ValPtr cv2 = nil, cv3;

	if (cv == nil)
		err = ThrowError(VE_DB4D_WRONGKEYVALUE, DBaction_BuildingIndexKey);
	else
	{
		if (crit->GetTyp() != (sLONG)cv->GetTrueValueKind())
		{
			cv2 = cv->ConvertTo(crit->GetTyp());
			if (cv2 != nil)
			{
				cv3 = cv2;
			}
			else
				err = ThrowError(memfull, DBaction_BuildingIndexKey);
		}
		else
			cv3 = (ValPtr)cv;

		if (err == VE_OK)
		{
			VSize len = cv3->GetSpace(kMaxBytesForKeyStrings);
			VSize len2 = sizeof(BTitemIndex)- sizeof(void*) + len;
			void* p2 = GetFastMem(len2, true, 'iKe2');
			if (p2 != nil)
			{
				res=new (p2) BTitemIndex();
				cv3->WriteToPtr(&res->data, false, kMaxBytesForKeyStrings);
			}
			else
			{
				err = ThrowError(memfull, DBaction_BuildingIndexKey);
			}
		}

		if (cv2 != nil)
			delete cv2;
	}

	if (err != VE_OK)
		err = ThrowError( VE_DB4D_CANNOT_BUILD_INDEXKEY, DBaction_BuildingIndexKey);

	return res;
}


ValPtr IndexInfoFromField::CreateVValueWithKey(const BTitemIndex* key, VError &err)
{
	err = VE_OK;
	ValPtr cv = nil;
	CreVValue_Code Code = FindCV(crit->GetTyp());
	if (Code == nil)
		err = VE_DB4D_FIELDDEFCODEMISSING;
	else
	{
		cv = (*Code)(fic->GetDF(),crit->GetLimitingLen(), (void*)&key->data, false, false, NULL, creVValue_default);
	}

	if (cv == nil)
		err = ThrowError(memfull, DBaction_BuildingValue);
	return cv;
}


void IndexInfoFromField::SwapByteKeyData(BTitemIndex* key)
{
	const VValueInfo* vv = VValue::ValueInfoFromValueKind((ValueKind)crit->GetTyp());
	if (testAssert(vv != nil))
	{
		vv->ByteSwapPtr((void*)&key->data, false);
	}
}


BTitemIndex* IndexInfoFromField::BuildKey(FicheInMem *rec, VError &err, Boolean OldOne)
{
	BTitemIndex* res = nil;
	ValPtr cv;
	err = VE_OK;
	assert(rec != nil);
	if (OldOne)
		cv = rec->GetNthFieldOld(crit->GetPosInRec(), err, true);
	else
		cv = rec->GetNthField(crit->GetPosInRec(), err, false, true);

	if (cv != nil)
	{
		ValueKind typ = cv->GetValueKind();
		VSize len = cv->GetSpace(kMaxBytesForKeyStrings);
		VSize len2 = sizeof(BTitemIndex)- sizeof(void*) + len;
		void* p2 = GetFastMem(len2, true, 'iKe3');
		if (p2 != nil)
		{
			res=new (p2) BTitemIndex();
			cv->WriteToPtr(&res->data, false, kMaxBytesForKeyStrings);
			if (cv->IsNull())
				res->SetNull();
		}
		else
		{
			err = ThrowError(memfull, DBaction_BuildingIndexKey);
		}
	}

	if (err != VE_OK)
		err = ThrowError( VE_DB4D_CANNOT_BUILD_INDEXKEY, DBaction_BuildingIndexKey);

	return res;
}


BTitemIndex* IndexInfoFromField::CreateIndexKey(const void* p)
{
	BTitemIndex* val;

	sLONG len = crit->CalcDataSize(p);
	sLONG len2 = sizeof(BTitemIndex)- sizeof(void*) + len;
	void* p2 = GetFastMem(len2, true, 'iKe4');
	if (p2 != nil)
	{	
		val=new (p2) BTitemIndex();
		vBlockMove(p, &val->data, len);
	}
	else
		val = nil;

	return(val);
}

sLONG IndexInfoFromField::CalulateDataPtrKeyLength(void* data)
{
	sLONG res = sizeof(BTitemIndex)- sizeof(BTreePageIndex*) - sizeof(void*);
	res = res + crit->CalcDataSize(data);
	return res;
}


sLONG IndexInfoFromField::CalulateKeyLength(const BTitemIndex* u)
{
	sLONG res = sizeof(BTitemIndex)- sizeof(BTreePageIndex*) - sizeof(void*);
	res = res + crit->CalcDataSize(&(u->data));
	return res;
}


sLONG IndexInfoFromField::CalulateFullKeyLengthInMem(const BTitemIndex* u)
{
	return CalulateKeyLength(u)+sizeof(BTreePageIndex*);
}


sLONG IndexInfoFromField::CompareKeys(const BTitemIndex *val1, const BTitemIndex *val2, const VCompareOptions& inOptions)
{
	if (val1 == nil)
	{
		if (val2 == nil)
			return CR_EQUAL;
		else
			return CR_SMALLER;
	}
	else
	{
		if (val2 == nil)
		{
			return CR_BIGGER;
		}
		else
		{
			const void* p1 = &(val1->data);
			const void* p2 = &(val2->data);
			return crit->CompareKeys(p1, p2, inOptions);
		}
	}
}


Boolean IndexInfoFromField::GenereQuickIndex(VError &err, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, sLONG MaxElemsInPage, sLONG RequiredElemsInPage)
{
	Boolean ok = false;
	if (fIsRemote)
		err = ThrowError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
	else
	{
		context->SetTimeOutForOthersPendingLock(-1);
		sLONG typ = crit->GetTyp();
		switch(typ) {

			case VK_STRING_UTF8:
				ok = TryToBuildIndexAlphaUTF8(err, crit, MaxElemsInPage, RequiredElemsInPage, context, InProgress, typ);
				break;

			case VK_STRING:
				ok = TryToBuildIndexAlpha(err, crit, MaxElemsInPage, RequiredElemsInPage, context, InProgress, typ);
				break;

			case VK_TIME:
				ok = TryToBuildIndexTime(err, MaxElemsInPage, RequiredElemsInPage, context, InProgress, typ);
				break;

			case VK_UUID:
				ok = TryToBuildIndexUUID(err, MaxElemsInPage, RequiredElemsInPage, context, InProgress, typ);
				break;

			case VK_LONG:
				ok = TryToBuildIndexLong(err, MaxElemsInPage, RequiredElemsInPage, context, InProgress, typ);
				break;

			case VK_BOOLEAN:
				ok = TryToBuildIndexBoolean(err, MaxElemsInPage, RequiredElemsInPage, context, InProgress, typ);
				break;

			case VK_BYTE:
				ok = TryToBuildIndexByte(err, MaxElemsInPage, RequiredElemsInPage, context, InProgress, typ);
				break;

			case VK_WORD:
				ok = TryToBuildIndexShort(err, MaxElemsInPage, RequiredElemsInPage, context, InProgress, typ);
				break;

			case VK_REAL:
				ok = TryToBuildIndexReal(err, MaxElemsInPage, RequiredElemsInPage, context, InProgress, typ);
				break;

			case VK_LONG8:
			case VK_DURATION:
			case VK_SUBTABLE:
			case VK_SUBTABLE_KEY:
				ok = TryToBuildIndexLong8(err, MaxElemsInPage, RequiredElemsInPage, context, InProgress, typ);
				break;

				//default:
		}

		context->SetTimeOutForOthersPendingLock(0);
	}
	return ok;
}


VError IndexInfoFromField::GetDistinctValues(DB4DCollectionManager& outCollection, BaseTaskInfo* context, Bittab* filtre, VDB4DProgressIndicator* InProgress, const VCompareOptions& inOptions)
{
	if (fIsRemote)
		return ThrowError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
	else
	{
		Open(index_read);
		OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
		VCompareOptions Options = inOptions;
		Options.SetIntlManager(GetContextIntl(context));
		VError err = GetHeader()->GetDistinctValues(curstack, outCollection, context, filtre, InProgress, Options);
		Close();

		return err;
	}
}


VError IndexInfoFromField::QuickGetDistinctValues(QuickDB4DArrayOfValues *outCollection, BaseTaskInfo* context, Bittab* filtre, VDB4DProgressIndicator* InProgress, const VCompareOptions& inOptions)
{
	if (fIsRemote)
		return ThrowError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
	else
	{
		Open(index_read);
		OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
		VCompareOptions Options = inOptions;
		Options.SetIntlManager(GetContextIntl(context));
		VError err = GetHeader()->QuickGetDistinctValues(curstack, outCollection, context, filtre, InProgress, Options);
		Close();
		return err;
	}
}


VError IndexInfoFromField::CalculateColumnFormulas(ColumnFormulas* formules, BaseTaskInfo* context, Bittab* filtre, VDB4DProgressIndicator* InProgress)
{
	if (fIsRemote)
		return ThrowError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
	else
	{
		Open(index_read);
		OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
		VError err = GetHeader()->CalculateColumnFormulas(curstack, formules, context, filtre, InProgress);
		Close();
		return err;
	}
}


VError IndexInfoFromField::CalculateMin(BaseTaskInfo* context, Bittab* filtre, VDB4DProgressIndicator* InProgress, VValueSingle* &result)
{
	if (fIsRemote)
		return ThrowError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
	else
	{
		Open(index_read);
		OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
		VError err = GetHeader()->CalculateMin(curstack, context, filtre, InProgress, result);
		Close();
		return err;
	}
}


VError IndexInfoFromField::CalculateMax(BaseTaskInfo* context, Bittab* filtre, VDB4DProgressIndicator* InProgress, VValueSingle* &result)
{
	if (fIsRemote)
		return ThrowError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
	else
	{
		Open(index_read);
		OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
		VError err = GetHeader()->CalculateMax(curstack, context, filtre, InProgress, result);
		Close();
		return err;
	}
}


#if 0
VArrayStruct* IndexInfoFromField::GetSourceLang()
{
	Open(index_write);
	//occupe();
	if (!sourcelang_isinited)
	{
		CLanguage* language = VDBMgr::GetLanguage();
		CDB4D_Lang* lang4d = VDBMgr::GetLang4D();
		if (language != nil && lang4d != nil)
		{
			language->InitArray(&fSourceLang, lang4d->GetElemOfArrayOfFieldDef());
			CDB4DField* xcri = new VDB4DField(crit);
			language->AppendToArray(&fSourceLang, (VClassInstance)xcri, false);
			// dois je faire un release sur xcri ? , a verifier !
			sourcelang_isinited = true;
		}
		
	}
	//libere();
	Close();
	return &fSourceLang;
}
#endif


void IndexInfoFromField::CheckIfIsDelayed()
{
	fic->AddOneIndexToDelay(this);
	/*
	if (fic->AreIndexesDelayed())
		DelayIndex();
		*/
}


void IndexInfoFromField::SetDelayIndex(sLONG inAlreadyDelayRequestCount)
{
	//Open(index_write);
	VTaskLock lock(&fDelayRequestCountMutex);
	fDelayRequestCount = inAlreadyDelayRequestCount;
	SetInvalidOnDisk();
	//Close();

}


void IndexInfoFromField::DelayIndex()
{
	//Open(index_write);
	VTaskLock lock(&fDelayRequestCountMutex);
	fDelayRequestCount++;
	if (fDelayRequestCount == 1)
	{
		SetInvalidOnDisk();
		// d'autre choses a faire plus tard
	}
	//Close();
}


void IndexInfoFromField::AwakeIndex(VDB4DProgressIndicator* inProgress)
{
	//Open(index_write);
	VTaskLock lock(&fDelayRequestCountMutex);
	if (fDelayRequestCount > 0)
	{
		fDelayRequestCount--;
		if (fDelayRequestCount == 0)
		{
			SetInvalidOnDisk();
			AddRebuildIndex(this, inProgress, nil);
		}
	}
	else
	{
		sLONG xbreak = 1; // put a break here
		assert(false);
	}
	//Close();
}


Boolean IndexInfoFromField::LockTargetTables(BaseTaskInfo* context)
{
	if (fic != nil && fic->GetDF() != nil)
	{
		context->SetTimeOutForOthersPendingLock(-1);
		fic->GetDF()->LockTable(context, true, -1);
		return true;
	}
	else
		return false;
}


void IndexInfoFromField::UnLockTargetTables(BaseTaskInfo* context)
{
	if (fic != nil && fic->GetDF() != nil)
	{
		fic->GetDF()->UnLockTable(context);
	}
}



VError IndexInfoFromField::JoinWithOtherIndex(IndexInfo* other, Bittab* filtre1, Bittab* filtre2,ComplexSelection* result, BaseTaskInfo* context, 
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

		IndexKeyIterator iter1(this, filtre1, context, nil, L"", curstack);
		IndexKeyIterator iter2(other, filtre2, context, nil, L"", curstack);

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
			sLONG resultcompare = CompareKeys(iter1.GetKey(),iter2.GetKey(), inOptions);
			if (resultcompare == CR_EQUAL)
			{
				BTitemIndex* val = CopyKey(iter1.GetKey());
				RecIDType recnum1 = iter1.GetRecNum();
				selrow[0] = recnum1;

				vector<RecIDType> recordsNums;
				//try
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
							if (CompareKeys(iter2.GetKey(),val, inOptions) == CR_EQUAL)
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
							resultcompare = CompareKeys(iter1.GetKey(), val, inOptions);
							if (resultcompare == CR_EQUAL)
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
				/*
				catch (...)
				{
					err = ThrowBaseError(memfull, noaction);
				}
				*/
				FreeKey(val);
			}
			else
			{
				if (resultcompare == CR_SMALLER)
				{
					if (leftjoin)
					{
						selrow[0] = iter1.GetRecNum();
						selrow[1] = -2;
						err = result->AddRow(selrow);
					}
					cont1 = iter1.NextKey(context, err);
				}
				else
				{
					if (rightjoin)
					{
						selrow[0] = -2;
						selrow[1] = iter2.GetRecNum();
						err = result->AddRow(selrow);
					}
					cont2 = iter2.NextKey(context, err);
				}

			}

		}

		if (err == VE_OK)
		{
			if (leftjoin)
			{
				while (cont1)
				{
					selrow[0] = iter1.GetRecNum();
					selrow[1] = -2;
					err = result->AddRow(selrow);

					cont1 = iter1.NextKey(context,err);
				}
			}
			if (rightjoin)
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



VError IndexInfoFromField::JoinWithOtherIndexNotNull(IndexInfo* other, Bittab* filtre1, Bittab* filtre2, Bittab* result, BaseTaskInfo* context, VCompareOptions& inOptions, VProgressIndicator* inProgress)
{
	VError err = VE_OK;

	if (fIsRemote)
		err = ThrowError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
	else
	{
		Open(index_read);
		other->Open(index_read);
		OccupableStack* curstack = gOccPool.GetStackForCurrentThread();

		IndexKeyIterator iter1(this, filtre1, context, nil, L"", curstack);
		IndexKeyIterator iter2(other, filtre2, context, nil, L"", curstack);

		Boolean cont1 = iter1.NextKey(context, err);
		Boolean cont2 = false;
		if (err == VE_OK)
		{
			cont2 = iter2.NextKey(context, err);
		}

		while (cont1 && cont2 && err == VE_OK)
		{
			sLONG resultcompare = CompareKeys(iter1.GetKey(),iter2.GetKey(), inOptions);
			if (resultcompare == CR_EQUAL)
			{
				cont1 = iter1.NextKey(context, err);
				result->Set(iter1.GetRecNum());
			}
			else
			{
				if (resultcompare == CR_SMALLER)
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


Boolean IndexInfoFromField::SourceIsABlob() const
{
	Boolean res = false;
	if (crit != nil)
	{
		if (crit->GetTyp() == VK_TEXT || crit->GetTyp() == VK_IMAGE || crit->GetTyp() == VK_BLOB)
		{
			res = true;
		}
	}
	return res;
}



IndexInfo* CreIndexInfoFromField(void)
{
	IndexInfoFromField *ind;
	
	ind=new IndexInfoFromField;
	return(ind);
}


IndexInfo* CreIndexInfoFromFieldLexico(void)
{
	IndexInfoFromFieldLexico *ind;

	ind=new IndexInfoFromFieldLexico;
	return(ind);
}



IndexInfo* CreIndexInfoFromField_Word(void)
{
	IndexInfoScalar<sWORD, kNbKeysForScalar> *ind;
	
	ind=new IndexInfoScalar<sWORD, kNbKeysForScalar>();
	return(ind);
}


IndexInfo* CreIndexInfoFromField_Byte(void)
{
	IndexInfoScalar<sBYTE, kNbKeysForScalar> *ind;
	
	ind=new IndexInfoScalar<sBYTE, kNbKeysForScalar>();
	return(ind);
}


IndexInfo* CreIndexInfoFromField_Long(void)
{
	IndexInfoScalar<sLONG, kNbKeysForScalar> *ind;
	
	ind=new IndexInfoScalar<sLONG, kNbKeysForScalar>();
	return(ind);
}


IndexInfo* CreIndexInfoFromField_Long8(void)
{
	IndexInfoScalar<sLONG8, kNbKeysForScalar> *ind;
	
	ind=new IndexInfoScalar<sLONG8, kNbKeysForScalar>();
	return(ind);
}


IndexInfo* CreIndexInfoFromField_Real(void)
{
	IndexInfoScalar<Real, kNbKeysForScalar> *ind;
	
	ind=new IndexInfoScalar<Real, kNbKeysForScalar>();
	return(ind);
}

IndexInfo* CreIndexInfoFromField_Time(void)
{
	IndexInfoScalar<uLONG8, kNbKeysForScalar> *ind;
	
	ind=new IndexInfoScalar<uLONG8, kNbKeysForScalar>();
	return(ind);
}


IndexInfo* CreIndexInfoFromField_Bool(void)
{
	IndexInfoScalar<uBYTE, kNbKeysForScalar> *ind;
	
	ind=new IndexInfoScalar<uBYTE, kNbKeysForScalar>();
	return(ind);
}


IndexInfo* CreIndexInfoFromField_UUID(void)
{
	IndexInfoScalar<VUUIDBuffer, kNbKeysForScalar> *ind;
	
	ind=new IndexInfoScalar<VUUIDBuffer, kNbKeysForScalar>();
	return(ind);
}




									/* -----------------------------------------------  */

uBOOL IndexInfoFromFieldLexico::MayBeSorted(void)
{
	return false;
}


void IndexInfoFromFieldLexico::IdentifyIndex(VString& outString, Boolean WithTableName, Boolean WithIndexName) const
{
	gs(1004,25,outString);	// FullText Index 

	if (!fName.IsEmpty() && WithIndexName)
	{
		outString += L"(";
		outString += fName;
		outString += L")";
	}

	outString += L" ";
	VString liaison;
	gs(1004,24,liaison);	// on
	outString += liaison;
	outString += L" ";

	VString s2,s;
	if (WithTableName)
	{
		crit->GetOwner()->GetName(s);
		s.AppendUniChar(CHAR_FULL_STOP);
	}
	crit->GetName(s2);
	s.AppendString(s2);

	outString += s;
}


VError IndexInfoFromFieldLexico::ValidateIndexInfo()
{
	VError err = VE_OK;

	if (crit == nil)
		err = VE_DB4D_WRONGFIELDREF;
	else
	{
		if (!crit->IsFullTextIndexable())
			err = VE_DB4D_INVALIDINDEXTYPE;
	}

	return err;
}



void IndexInfoFromFieldLexico::GetDebugString(VString& s, sLONG numinstance)
{
	s.Clear();
	if (crit != nil)
	{
		s = L"FullText : ";
		VString s2;
		crit->GetOwner()->GetName(s2);
		s.AppendString(s2);
		if (numinstance != 0)
			s+=L"("+ToString(numinstance)+L")";
		s.AppendUniChar(CHAR_FULL_STOP);
		crit->GetName(s2);
		s.AppendString(s2);
	}
}


sLONG IndexInfoFromFieldLexico::CalculateDefaultSizeKey()
{
	return (sLONG) VDBMgr::GetStringRef()->GetValueInfo()->GetAvgSpace();
}


BTitemIndex* IndexInfoFromFieldLexico::BuildKeyFromVValue(const ValPtr cv, VError &err)
{
	BTitemIndex* res = nil;
	err = VE_OK;
	ValPtr cv2 = nil, cv3;

	if (cv == nil)
		err = ThrowError(VE_DB4D_WRONGKEYVALUE, DBaction_BuildingIndexKey);
	else
	{
		if (crit->GetTypeForLexicalIndex() != cv->GetTrueValueKind())
		{
			cv2 = cv->ConvertTo(crit->GetTypeForLexicalIndex());
			if (cv2 != nil)
			{
				cv3 = cv2;
			}
			else
				err = ThrowError(memfull, DBaction_BuildingIndexKey);
		}
		else
			cv3 = (ValPtr)cv;

		if (err == VE_OK)
		{
			VSize len = cv3->GetSpace(kMaxBytesForKeyStrings);
			VSize len2 = sizeof(BTitemIndex)- sizeof(void*) + len;
			void* p2 = GetFastMem(len2, true, 'iKe5');
			if (p2 != nil)
			{
				res=new (p2) BTitemIndex();
				cv3->WriteToPtr(&res->data, false, kMaxBytesForKeyStrings);
			}
			else
			{
				err = ThrowError(memfull, DBaction_BuildingIndexKey);
			}
		}

		if (cv2 != nil)
			delete cv2;
	}

	if (err != VE_OK)
		err = ThrowError( VE_DB4D_CANNOT_BUILD_INDEXKEY, DBaction_BuildingIndexKey);

	return res;
}


ValPtr IndexInfoFromFieldLexico::CreateVValueWithKey(const BTitemIndex* key, VError &err)
{
	err = VE_OK;
	ValPtr cv;
	
	if (crit->GetTypeForLexicalIndex() == VK_STRING)
		cv = CreVString(fic->GetDF(),-1, (void*)&key->data, false, false);
	else
		cv = CreVStringUTF8(fic->GetDF(),-1, (void*)&key->data, false, false);


	if (cv == nil)
		err = ThrowError(memfull, DBaction_BuildingValue);
	return cv;
}


void IndexInfoFromFieldLexico::SwapByteKeyData(BTitemIndex* key)
{
	const VValueInfo* vv = VValue::ValueInfoFromValueKind(crit->GetTypeForLexicalIndex());
	if (testAssert(vv != nil))
	{
		vv->ByteSwapPtr((void*)&key->data, false);
	}
}


BTitemIndex* IndexInfoFromFieldLexico::BuildKey(FicheInMem *rec, VError &err, Boolean OldOne)
{
	assert(false); // should never be called
	err = VE_DB4D_NOTIMPLEMENTED;
	return nil;
}


BTitemIndex* IndexInfoFromFieldLexico::CreateIndexKey(const void* p)
{
	BTitemIndex* val;

	VSize len;
	if (crit->GetTypeForLexicalIndex() == VK_STRING)
		len = VDBMgr::GetStringRef()->GetValueInfo()->GetSizeOfValueDataPtr(p);
	else
		len = VDBMgr::GetStringUTF8Ref()->GetValueInfo()->GetSizeOfValueDataPtr(p);
	VSize len2 = sizeof(BTitemIndex)- sizeof(void*) + len;
	void* p2 = GetFastMem(len2, true, 'iKe6');
	if (p2 != nil)
	{	
		val=new (p2) BTitemIndex();
		vBlockMove(p, &val->data, len);
	}
	else
		val = nil;

	return(val);
}

sLONG IndexInfoFromFieldLexico::CalulateDataPtrKeyLength(void* data)
{
	VSize res = sizeof(BTitemIndex)- sizeof(BTreePageIndex*) - sizeof(void*);

	// L.E. DB4DString::sInfo::GetSizeOfValueDataPtr( data); devrait suffire mais peut-etre veux-tu virtualiser l'utilisation de DB4DString?
	if (crit->GetTypeForLexicalIndex() == VK_STRING)
		res = res + VDBMgr::GetStringRef()->GetValueInfo()->GetSizeOfValueDataPtr(data);
	else
		res = res + VDBMgr::GetStringUTF8Ref()->GetValueInfo()->GetSizeOfValueDataPtr(data);

	return (sLONG) res;
}


sLONG IndexInfoFromFieldLexico::CalulateKeyLength(const BTitemIndex* u)
{
	VSize res = sizeof(BTitemIndex)- sizeof(BTreePageIndex*) - sizeof(void*);
	if (crit->GetTypeForLexicalIndex() == VK_STRING)
		res = res + VDBMgr::GetStringRef()->GetValueInfo()->GetSizeOfValueDataPtr(&(u->data));
	else
		res = res + VDBMgr::GetStringUTF8Ref()->GetValueInfo()->GetSizeOfValueDataPtr(&(u->data));

	return (sLONG) res;
}


sLONG IndexInfoFromFieldLexico::CalulateFullKeyLengthInMem(const BTitemIndex* u)
{
	return CalulateKeyLength(u)+sizeof(BTreePageIndex*);
}


sLONG IndexInfoFromFieldLexico::CompareKeys(const BTitemIndex *val1, const BTitemIndex *val2, const VCompareOptions& inOptions)
{
	if (val1 == nil)
	{
		if (val2 == nil)
			return CR_EQUAL;
		else
			return CR_SMALLER;
	}
	else
	{
		if (val2 == nil)
		{
			return CR_BIGGER;
		}
		else
		{
			const void* p1 = &(val1->data);
			const void* p2 = &(val2->data);
			if (crit->GetTypeForLexicalIndex() == VK_STRING)
				return VString::sInfo.CompareTwoPtrToDataWithOptions(p1, p2, inOptions);
			else
				return VStringUTF8::sInfo.CompareTwoPtrToDataWithOptions(p1, p2, inOptions);
		}
	}
}




VError IndexInfoFromFieldLexico::PlaceCle(OccupableStack* curstack, FicheInMem *rec, BaseTaskInfo* context)
{
	VError err = VE_OK;
	if (fIsRemote)
		err = ThrowError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
	else
	{
		ValPtr xf = rec->GetNthField(crit->GetPosInRec(), err, false, true);
		if (xf != nil)
		{
			ValueKind vkind = xf->GetValueKind();
			if (vkind == VK_STRING || vkind == VK_TEXT || vkind == VK_IMAGE)
			{
				DB4DKeyWordList keywords;
				VCompareOptions options;
				options.SetIntlManager(GetContextIntl(context));
				options.SetDiacritical(true);

				if (vkind == VK_IMAGE)
				{
#if !VERSION_LINUX   // Postponed Linux Implementation !
					VString sKeys;
					((VPicture*)xf)->GetKeywords(sKeys);
					err = BuildKeyWords(sKeys, keywords, options);
#endif
				}
				else if (crit->GetStyledText())
				{
					VString plaintext;
					VSpanTextParser::Get()->GetPlainTextFromSpanText(*(VString*)xf, plaintext);
					err = BuildKeyWords(plaintext, keywords, options);
				}
				else
				{
					VString* theText = (VString*)xf;
					err = BuildKeyWords(*theText, keywords, options);
				}

				if (err == VE_OK)
				{
					sLONG i,nb = keywords.GetCount();
					for (i=0; i<nb; i++)
					{
						BTitemIndex *val;
						IndexKeyArray tempkey(this);
						VString* s = keywords[i];

						val=BuildKeyFromVValue(s, err);
						if (val != nil)
						{
							Transaction* trans = GetCurrentTransaction(context);
							if (trans != nil)
							{
								err = trans->PlaceCle(this, val, rec->GetAssoc()->getnumfic());
							}
							else
							{
								tempkey.AddKey(val);
								err=header->PlaceCle(curstack, val, rec->GetAssoc()->getnumfic(), tempkey, context);
							}
						}

						if (err != VE_OK)
							break;
					}
				}
				
			}
			else
				err = VE_DB4D_WRONGTYPE;
		}
	}

	return(err);
}


VError IndexInfoFromFieldLexico::DetruireCle(OccupableStack* curstack, FicheInMem *rec, BaseTaskInfo* context)
{
	VError err = VE_OK;
	if (fIsRemote)
		err = ThrowError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
	else
	{
		ValPtr xf = rec->GetNthFieldOld(crit->GetPosInRec(), err);
		if (xf != nil)
		{
			ValueKind vkind = xf->GetValueKind();
			if (vkind == VK_STRING || vkind == VK_TEXT || vkind == VK_IMAGE)
			{
				DB4DKeyWordList keywords;
				VCompareOptions options;
				options.SetIntlManager(GetContextIntl(context));
				options.SetDiacritical(true);

				if (vkind == VK_IMAGE)
				{
#if !VERSION_LINUX   // Postponed Linux Implementation !
					VString sKeys;
					((VPicture*)xf)->GetKeywords(sKeys);
					err = BuildKeyWords(sKeys, keywords, options);
#endif
				}
				else if (crit->GetStyledText())
				{
					VString plaintext;
					VSpanTextParser::Get()->GetPlainTextFromSpanText(*(VString*)xf, plaintext);
					err = BuildKeyWords(plaintext, keywords, options);
				}
				else
				{
					VString* theText = (VString*)xf;
					err = BuildKeyWords(*theText, keywords, options);
				}

				if (err == VE_OK)
				{
					sLONG i,nb = keywords.GetCount();
					for (i=0; i<nb; i++)
					{
						BTitemIndex *val;
						IndexKeyArray tempkey(this);
						VString* s = keywords[i];

						val=BuildKeyFromVValue(s, err);
						if (val != nil)
						{
							Transaction* trans = GetCurrentTransaction(context);
							if (trans != nil)
							{
								err = trans->DetruireCle(this, val, rec->GetAssoc()->getnumfic());
							}
							else
							{
								tempkey.AddKey(val);
								err=header->DetruireCle(curstack, val,rec->GetAssoc()->getnumfic(), tempkey, context);
							}
						}

						if (err != VE_OK)
							break;
					}
				}

			}
			else
				err = VE_DB4D_WRONGTYPE;
		}
	}

	return(err);

}


Boolean IndexInfoFromFieldLexico::GenereQuickIndex(VError &err, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, sLONG MaxElemsInPage, sLONG RequiredElemsInPage)
{
	Boolean result = false;

	if (fIsRemote)
		err = ThrowError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
	else
	{
		context->SetTimeOutForOthersPendingLock(-1);
		Selection* into = (Selection*)-2;

		if (crit->GetTypeForLexicalIndex() == VK_STRING)
		{
			TypeSortElemArray<xString> tempsort;

			sLONG lenmax = crit->GetMaxLenForSort()+xString::NoDataSize();
			xMultiFieldDataOffsets off(crit, VK_STRING, true, lenmax);
			result = tempsort.TryToSort((Selection*)nil, into, err, off, context, InProgress, fUniqueKeys, lenmax);

			if (result)
			{
				if (header->GetRealType() == DB4D_Index_Btree)
					err = tempsort.GenereIndex(this, MaxElemsInPage, RequiredElemsInPage, context, InProgress, VK_STRING, lenmax);
				else
					err = tempsort.GenereClusterIndex(this, MaxElemsInPage, RequiredElemsInPage, context, InProgress, VK_STRING, lenmax, off);
			}
		}
		else
		{
			TypeSortElemArray<xStringUTF8> tempsort;

			sLONG lenmax = crit->GetMaxLenForSort()+xStringUTF8::NoDataSize();
			xMultiFieldDataOffsets off(crit, VK_STRING_UTF8, true, lenmax);
			result = tempsort.TryToSort((Selection*)nil, into, err, off, context, InProgress, fUniqueKeys, lenmax);

			if (result)
			{
				if (header->GetRealType() == DB4D_Index_Btree)
					err = tempsort.GenereIndex(this, MaxElemsInPage, RequiredElemsInPage, context, InProgress, VK_STRING_UTF8, lenmax);
				else
					err = tempsort.GenereClusterIndex(this, MaxElemsInPage, RequiredElemsInPage, context, InProgress, VK_STRING_UTF8, lenmax, off);
			}
		}

		context->SetTimeOutForOthersPendingLock(0);
		//FreeXString(tempsort, lenmax);

		if (err != VE_OK)
			result =false;
	}

	return result;
}


VError IndexInfoFromFieldLexico::GenereIndex(BaseTaskInfo* context, VDB4DProgressIndicator* InProgress)
{
	DataTable *FD;
	sLONG i,nb;
	VError err = VE_OK;
	FicheInMem *rec;
	ValPtr cv;
	BTitemIndex* val;

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
			gs(1005,11,session_title);	// Building FullText Index on one Field: %curValue of %maxValue Records
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
				Open(index_write);
				err = PlaceCle(curstack, rec, context);
				EnCreation(i);
				Close();
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


bool IndexInfoFromFieldLexico::MustBeRebuild()
{
	Boolean result = false;
	if (crit != nil)
	{
		sLONG typ = crit->GetTyp();
		if (typ != VK_STRING && typ != VK_TEXT && typ != VK_STRING_UTF8 && typ != VK_TEXT_UTF8 && typ != VK_IMAGE)
			result = true;
	}
	return result;
}





									/* -----------------------------------------------  */




IndexInfoFromMultipleField::IndexInfoFromMultipleField(Base4D* db, FieldNuplet* from, sLONG xtypindex, Boolean UniqueKeys):IndexInfo(UniqueKeys)
{
	IndexHeader *indH;
	
	SetDB(db);

	if (xtypindex == DB4D_Index_AutoType)
		xtypindex = DB4D_Index_Btree;
	
	typindex=xtypindex;
	fields = new FieldNuplet(from);
	frefs = fields->GetFieldRefs();
	sLONG nbfield = fields->GetNbField();
	fDataKinds.resize(nbfield, -1);
	for (sLONG i = 0; i < nbfield; i++)
	{
		Field* cri = (*frefs)[i].crit;
		if (cri != nil)
		{
			fDataKinds[i] = cri->GetTyp();
		}
	}
	
	
	indH=CreateIndexHeader(xtypindex, this, -1);
	header=indH;
		
	InfoTyp=DB4D_Index_OnMultipleFields;
}


IndexInfoFromMultipleField::~IndexInfoFromMultipleField()
{
	delete fields;
}


void IndexInfoFromMultipleField::CreateHeader()
{
	if (header == nil)
	{
		header=CreateIndexHeader(typindex, this, -1);
	}
	else
		header->Update(GetTargetTable());
}


Table* IndexInfoFromMultipleField::GetTargetTable()
{
	if (frefs != NULL)	// sc 29/08/2008 ACI0057960 (blindage)
		return (*frefs)[0].fic;
	
	return NULL;
}

VError IndexInfoFromMultipleField::LoadFromBagWithLoader( const VValueBag& inBag, VBagLoader *inLoader, void* inContext)
{
	assert( this->fields == NULL);
	assert( this->frefs == NULL);
	VError err;

	if (fIsRemote || bd->IsRemote())
		err = ThrowError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
	else
	{
		err = inherited::LoadFromBagWithLoader( inBag, inLoader, inContext);

		if (err == VE_OK)
		{
			const VBagArray *refs = inBag.RetainElements( DB4DBagKeys::field_ref);
			if (refs != NULL)
			{
				sLONG count = refs->GetCount();

				vector<sLONG> dataKinds;
				FieldNuplet *nuplet = new FieldNuplet( count, true);
				if (nuplet == NULL)
					err = ThrowError( memfull, DBaction_CreatingIndex);
				else
				{
					for( sLONG i = 1 ; i <= count && err == VE_OK ; i++)
					{
						const VValueBag *ref = refs->GetNth( i);
						Field *field = bd->FindAndRetainFieldRef( *ref, inLoader);
						if (field == NULL)
							err = ThrowError( VE_DB4D_WRONGFIELDREF, DBaction_CreatingIndex);
						else
						{
							dataKinds.push_back( field->GetTyp());
							nuplet->SetNthField( i, field);
							field->Release();
						}
					}
					if (err == VE_OK)
					{
						fDataKinds.swap( dataKinds);
						this->fields = nuplet;
						this->frefs = this->fields->GetFieldRefs();
					}
					else
					{
						delete nuplet;
					}
				}
				refs->Release();
			}
		}
	}

	return err;
}


VError IndexInfoFromMultipleField::SaveToBag( VValueBag& ioBag, VString& outKind) const
{
	VError err = inherited::SaveToBag( ioBag, outKind);

	if (err == VE_OK)
	{
		// les references de champs
		sLONG nb = frefs->GetCount();
		for( sLONG i = 1; i<=nb; i++)
		{
			VValueBag *ref = (*frefs)[i-1].crit->CreateReferenceBag( true);
			if (ref != NULL)
			{
				ioBag.AddElement( DB4DBagKeys::field_ref, ref);
				ref->Release();
			}
		}
	}
	
	return err;
}


void IndexInfoFromMultipleField::IdentifyIndex(VString& outString, Boolean WithTableName, Boolean WithIndexName) const
{
	gs(1004,29,outString);	// Composite Index

	if (!fName.IsEmpty() && WithIndexName)
	{
		outString += L"(";
		outString += fName;
		outString += L")";
	}

	outString += L" ";
	VString liaison;
	gs(1004,24,liaison);	// on
	outString += liaison;
	outString += L" ";

	VString s2;
	sLONG i,nb;
	nb = frefs->GetCount();
	for (i = 1; i<=nb; i++)
	{
		if (WithTableName)
		{
			(*frefs)[i-1].fic->GetName(s2);
			outString += s2;
			outString += L".";
		}

		(*frefs)[i-1].crit->GetName(s2);
		outString += s2;
		if (i != nb)
		{
			outString += L", ";
		}
	}
}


Boolean IndexInfoFromMultipleField::NeedToRebuildIndex(VIntlMgr* NewIntlMgr)
{
	Boolean result = false;
	sLONG i, nb = frefs->GetCount();
	for (i = 1; i<=nb; i++)
	{
		Field* crit = (*frefs)[i-1].crit;
		if (crit != nil)
		{
			sLONG typ = crit->GetTyp();
			if (typ == VK_STRING || typ == VK_TEXT || typ == VK_STRING_UTF8 || typ == VK_TEXT_UTF8 || typ == VK_IMAGE)
				result = true;
		}
	}
	return result;
}



bool IndexInfoFromMultipleField::MustBeRebuild()
{
	Boolean result = false;
	sLONG i, nb = frefs->GetCount();
	for (i = 1; i<=nb; i++)
	{
		Field* crit = (*frefs)[i-1].crit;
		if (crit != nil)
		{
			sLONG typ = crit->GetTyp();
			if (typ != fDataKinds[i-1] )
				result = true;
		}
	}
	return result;
}



VError IndexInfoFromMultipleField::ClearFromSystemTableIndexCols()
{
	VObjLock lock(this);
	return bd->DelIndexCol(this);
}


VError IndexInfoFromMultipleField::AddToSystemTableIndexCols()
{
	VObjLock lock(this);
	VError err = VE_OK;
	sLONG i, nb = frefs->GetCount();
	for (i = 1; i<=nb && err == VE_OK; i++)
	{
		Field* crit = (*frefs)[i-1].crit;
		if (crit != nil)
		{
			VUUID xid;
			crit->GetUUID(xid);
			err = bd->AddIndexCol(this, xid, i);
		}
	}
	return err;
}



VError IndexInfoFromMultipleField::ValidateIndexInfo()
{
	VError err = VE_OK;

	sLONG i, nb = frefs->GetCount();
	if (nb <= 0)
		err = VE_DB4D_WRONGFIELDREF;
	else
	{
		fDataKinds.resize(nb, -1);
		for (i = 1; i<=nb && err == VE_OK; i++)
		{
			Field* crit = (*frefs)[i-1].crit;
			if (crit == nil)
				err = VE_DB4D_WRONGFIELDREF;
			else
			{
				fDataKinds[i-1] = crit->GetTyp();
				if (!crit->IsIndexable())
					err = VE_DB4D_INVALIDINDEXTYPE;
			}
		}
	}
	return err;
}



void IndexInfoFromMultipleField::GetDebugString(VString& s, sLONG numinstance)
{
	sLONG i,nb;
	s.Clear();
	VStr<64> s2;
	
	if (frefs != NULL)	// sc 19/03/2009 ACI0061371
	{
		nb = frefs->GetCount();
		for (i = 1; i<=nb; i++)
		{
			if ( (*frefs)[i-1].crit ) /* Sergiy - 21 April 2007 - Severely damaged databases may have crit==null. */
			{
				(*frefs)[i-1].crit->GetName(s2);
				s.AppendString(s2);
			}
			else
				s.AppendUniCString(L"<NULL>");
			if (i != nb)
			{
				s.AppendUniCString(L", ");
			}
		}
	}
}


/*
sLONG IndexInfoFromMultipleField::GetLen(void)
{
	return(IndexInfo::GetLen()+fields->GetLen()+4+header->GetLen());
}
*/

VError IndexInfoFromMultipleField::PutInto(VStream& buf, Base4D* xdb, Boolean WithHeader)
{
	sLONG lll,lllu;
	VError err;
	
	err = IndexInfo::PutInto(buf, xdb);
	if (err==VE_OK) 
		err=buf.PutLong(typindex);

	if (fUniqueKeys)
		lllu = 1;
	else
		lllu = 0;
	if (err==VE_OK) err=buf.PutLong(lllu);

	lll = fields->GetNbField();
	if (err==VE_OK) 
		err=buf.PutLong(lll);
	if (err==VE_OK)
	{
		for (sLONG i = 0; i < lll && err == VE_OK; i++)
		{
			err = buf.PutLong(fDataKinds[i]);
			if (err == VE_OK)
			{
				Field* cri = (*frefs)[i].crit;
				VUUID xid;
				if (cri == nil)
					xid.Clear();
				else
					cri->GetUUID(xid);
				err = xid.WriteToStream(&buf);
			}
		}
	}
	if (err==VE_OK && WithHeader) 
		err=header->PutInto(buf);
	
	return(err);
}

VError IndexInfoFromMultipleField::GetFrom(VStream& buf, Base4D *xdb, Boolean WithHeader, Boolean oldformat)
{
	VError err;
	sLONG nbfield,lllu;
	
	fields = nil;
	frefs = nil;
	
	SetDB(xdb);
	err = IndexInfo::GetFrom(buf, xdb);
	if (err==VE_OK) 
		err=buf.GetLong(typindex);
	if (err==VE_OK) 
		err=buf.GetLong(lllu);
	fUniqueKeys = (lllu == 1);

	if (err==VE_OK) err=buf.GetLong(nbfield);
	if (err == VE_OK)
	{
		fields = new FieldNuplet(nbfield,false);
		frefs = fields->GetFieldRefs();
		/*
		err=buf.GetData(fields->GetDataPtr(),fields->GetLen()-4);
		if (err == VE_OK)
		{
			if (buf.NeedSwap())
				fields->SwapBytes();
			err = fields->UpdateCritFic(xdb);
			if (err == VE_OK)
				frefs = fields->GetFieldRefs();
			
		}
		*/
		fDataKinds.resize(nbfield, -1);
		for (sLONG i = 0; i < nbfield && err == VE_OK; i++)
		{
			sLONG datakind;
			err = buf.GetLong(datakind);
			if (err == VE_OK)
			{
				fDataKinds[i] = datakind;
				VUUID xid;
				err = xid.ReadFromStream(&buf);
				if (err == VE_OK)
				{
					sLONG numfield = 0;
					sLONG numtable = 0;
					Table* fic = nil;
					Field* cri = bd->FindAndRetainFieldRef(xid);
					fields->SetNthField(i+1, cri);
					if (cri != nil)
					{
						cri->Release();
					}
				}
			}


		}
	}
	
	if (WithHeader)
	{
		if (err==VE_OK)
		{
			header=CreateIndexHeader(typindex, this, -1);
			if (header==nil)
			{
				err=ThrowError(memfull, DBaction_LoadingIndexHeader);
			}
			else
			{
				err=header->GetFrom(buf);
				header->SetAssoc(this);
			}
		}
		else
			header = nil;
	}
	
	return(err);
}



void IndexInfoFromMultipleField::CalculDependence(void)
{
	sLONG i;
	
	Table* tt = (*frefs)[0].fic;
	for (i=0;i<frefs->GetCount();i++)
	{
		Field* cri = (*frefs)[i].crit;
		if (cri != nil)
			cri->AddIndexDep(this);
	}
	if (tt != nil)
		tt->AddIndexDep(this);
}
		

void IndexInfoFromMultipleField::DeleteFromDependencies(void)
{
	sLONG i;

	Table* tt = (*frefs)[0].fic;
	for (i=0;i<frefs->GetCount();i++)
	{
		Field* cri = (*frefs)[i].crit;
		if (cri != nil)
			cri->DelIndexDep(this);
	}
	if (tt != nil)
		tt->DelIndexDep(this);
}


void IndexInfoFromMultipleField::TouchDependencies()
{
	if (testAssert(!fIsRemote))
	{
		for (sLONG i = 0 ; i < frefs->GetCount() ; ++i)
		{
			Field* cri = (*frefs)[i].crit;
			if (cri != nil)
				cri->Touch();
		}
	}
}


VError IndexInfoFromMultipleField::PlaceCle(OccupableStack* curstack, FicheInMem *rec, BaseTaskInfo* context)
{
	BTitemIndex *val;
	VError err = VE_OK;
	if (fIsRemote)
		err = ThrowError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
	else
	{
		IndexKeyArray tempkey(this);
		
		val=BuildKey(rec, err);
		if (val != nil)
		{
			Transaction* trans = GetCurrentTransaction(context);
			if (trans != nil)
			{
				err = trans->PlaceCle(this, val, rec->GetAssoc()->getnumfic());
			}
			else
			{
				tempkey.AddKey(val);
				err=header->PlaceCle(curstack, val,rec->GetAssoc()->getnumfic(),tempkey, context);
			}
		}
		else
			err = ThrowError(VE_DB4D_INVALID_INDEXKEY, DBaction_InsertingKeyIntoIndex);
	}
	
	return(err);
}


VError IndexInfoFromMultipleField::DetruireCle(OccupableStack* curstack, FicheInMem *rec, BaseTaskInfo* context)
{
	BTitemIndex *val;
	VError err = VE_OK;
	if (fIsRemote)
		err = ThrowError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
	else
	{
		IndexKeyArray tempkey(this);
		
		val=BuildKey(rec, err, true);
		if (val != nil)
		{
			Transaction* trans = GetCurrentTransaction(context);
			if (trans != nil)
			{
				err = trans->DetruireCle(this, val, rec->GetAssoc()->getnumfic());
			}
			else
			{
				tempkey.AddKey(val);
				err=header->DetruireCle(curstack, val,rec->GetAssoc()->getnumfic(),tempkey,context);
			}
		}
		else
			err = ThrowError(VE_DB4D_INVALID_INDEXKEY, DBaction_DeletingKeyFromIndex);
	}
	
	return(err);
}


VError IndexInfoFromMultipleField::PlaceCleForTrans(OccupableStack* curstack, BTitemIndex* val, BaseTaskInfo* context)
{
	VError err = VE_OK;
	if (fIsRemote)
		err = ThrowError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
	else
	{
		IndexKeyArray tempkey(this);

		tempkey.AddKey(val);
		err=header->PlaceCle(curstack, val, val->GetQui(), tempkey, context);
	}

	return(err);
}


VError IndexInfoFromMultipleField::DetruireCleForTrans(OccupableStack* curstack, BTitemIndex* val, BaseTaskInfo* context)
{
	VError err = VE_OK;
	if (fIsRemote)
		err = ThrowError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
	else
	{
		IndexKeyArray tempkey(this);

		tempkey.AddKey(val);
		err=header->DetruireCle(curstack, val, val->GetQui(), tempkey,context);
	}

	return(err);
}


uBOOL IndexInfoFromMultipleField::NeedUpdate(FicheInMem *rec, BaseTaskInfo* context)
{
	sLONG i;
	sLONG nb;
	uBOOL ismodifx;
	
	ismodifx = false;
	nb=fields->GetNbField();
	for (i=0;i<nb;i++)
	{
		ismodifx = ismodifx | rec->IsModif(fields->GetFieldNum(i));
	}
	
	return(ismodifx);
}


uBOOL IndexInfoFromMultipleField::Egal(IndexInfo* autre)
{	
	return(MatchFields(autre));
}


VError IndexInfoFromMultipleField::GenereIndex(BaseTaskInfo* context, VDB4DProgressIndicator* InProgress)
{
	DataTable *FD;
	sLONG i,nb;
	VError err = VE_OK;
	FicheInMem *rec;
	BTitemIndex *val;
	
	if (fIsRemote)
		err = ThrowError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
	else
	{
		FD=(*frefs)[0].fic->GetDF();
		nb=FD->GetMaxRecords(context);
		SetMaxElemEnCreation(nb);
		
		if (InProgress != nil)
		{
			XBOX::VString session_title;
			gs(1005,12,session_title);	// Building Composite Index: %curValue of %maxValue Records
			InProgress->BeginSession(nb,session_title,false);
		}
		
		ReadAheadBuffer* buf = (*frefs)[0].fic->GetOwner()->NewReadAheadBuffer();

		OccupableStack* curstack = gOccPool.GetStackForCurrentThread();

		for (i=0;(i<nb) && (err == VE_OK);i++)
		{
			if (InProgress != nil)
			{
				if (!InProgress->Progress(i)) 
					 err = ThrowError(VE_DB4D_ACTIONCANCELEDBYUSER, DBaction_BuildingIndex);
			}
				
			rec=FD->LoadRecord(i, err, DB4D_Do_Not_Lock, context, false, false, buf);
			if (rec!=nil)
			{
				IndexKeyArray tempkey(this);
				if (err == VE_OK)
				{
					val=BuildKey(rec,err);
					if (val != nil)
					{
						Open(index_write);
						tempkey.AddKey(val);
						err=header->PlaceCle(curstack, val,i,tempkey,context);
						EnCreation(i);
						Close();
					}
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


Boolean IndexInfoFromMultipleField::GenereQuickIndex(VError &err, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, sLONG MaxElemsInPage, sLONG RequiredElemsInPage)
{
	Boolean result = false;
	if (fIsRemote)
		err = ThrowError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
	else
	{
		context->SetTimeOutForOthersPendingLock(-1);
		result = TryToBuildIndexMulti(err, MaxElemsInPage, RequiredElemsInPage, context, InProgress);
		context->SetTimeOutForOthersPendingLock(0);
	}
	return result;
}


sLONG IndexInfoFromMultipleField::GetMaxNB(BaseTaskInfo* context)
{
	sLONG nb = 0;
	DataTable *FD;

	if (testAssert(!fIsRemote))
	{
		FD=(*frefs)[0].fic->GetDF();
		nb=FD->GetMaxRecords(context);
	}
	return(nb);
}


uBOOL IndexInfoFromMultipleField::MatchFields(IndexInfo* OtherIndex) 
{ 
	return(fields->Match( ((IndexInfoFromMultipleField*)OtherIndex)->fields )); 
}


uBOOL IndexInfoFromMultipleField::MatchFields(FieldNuplet *OtherFields) 
{ 
	return(fields->Match(OtherFields)); 
}


uBOOL IndexInfoFromMultipleField::MatchFields(const NumFieldArray& OtherFields, sLONG numTable) 
{ 
	return(fields->Match(OtherFields, numTable)); 
}


Boolean IndexInfoFromMultipleField::MatchIndex(FieldsArray* fields, sLONG NbFields)
{
	Boolean res = true;
	Boolean found;
	sLONG i,j,nb;
	Field* cri;
	
	nb = frefs->GetCount();
	if (nb != NbFields) res = false;
	else
	{
		for (i = 1; i <= nb; i++)
		{
			found = false;
			cri = (*frefs)[i-1].crit;
			
			for (j = 1; j <= NbFields; j++)
			{
				if ( (*fields)[j] == cri)
				{
					found = true;
					break;
				}
			}
			
			if (!found)
			{
				res = false;
				break;
			}
		}
	}
	
	return res;
}



sLONG IndexInfoFromMultipleField::CalculateDefaultSizeKey()
{
	sLONG res = 0, i, nb = frefs->GetCount();

	for (i = 0; i < nb; i++)
	{
		Field* crit = (*frefs)[i].crit;
		res += crit->CalcAvgSize();
	}
	return res;
}


BTitemIndex* IndexInfoFromMultipleField::BuildKeyFromVValues(const ListOfValues& values, VError &err)
{
	Boolean good = true;
	BTitemIndex* res = nil;
	ValPtr cv, cv2;
	err = VE_OK;

	VSize len = 0;
	sLONG i, nb = frefs->GetCount();

	if (values.GetCount() != nb)
		err = ThrowError( VE_DB4D_WRONGKEYVALUE, DBaction_BuildingIndexKey);
	else
	{
		for (i = 0; i < nb; i++)
		{
			Field* crit = (*frefs)[i].crit;

			cv = values[i];

			if (cv == nil)
			{
				good = false;
				err = ThrowError( VE_DB4D_WRONGKEYVALUE, DBaction_BuildingIndexKey);
				break;
			}
			else
			{
				if (crit->GetTyp() != (sLONG)cv->GetTrueValueKind())
				{
					cv2 = cv->ConvertTo(crit->GetTyp());
					if (cv2 == nil)
					{
						err = ThrowError( memfull, DBaction_BuildingIndexKey);
						good = false;
						break;
					}
					else
					{
						len = len + cv2->GetSpace(kMaxBytesForKeyStrings);
						delete cv2;
					}
				}
				else
				{
					len = len + cv->GetSpace(kMaxBytesForKeyStrings);
				}
			}
		}


		if (good)
		{
			VSize len2 = sizeof(BTitemIndex)- sizeof(void*) + len;
			void* p2 = GetFastMem(len2, true, 'iKe7');
			if (p2 != nil)
			{
				res=new (p2) BTitemIndex();
				void* p = (void*) &res->data;
				for (i = 0; i < nb; i++)
				{
					Field* crit = (*frefs)[i].crit;
					cv = values[i];

					if (crit->GetTyp() != (sLONG)cv->GetTrueValueKind())
					{
						cv2 = cv->ConvertTo(crit->GetTyp());
						if (cv2 == nil)
						{
							err = ThrowError( memfull, DBaction_BuildingIndexKey);
							break;
						}
						else
						{
							p = cv2->WriteToPtr(p, false, kMaxBytesForKeyStrings);
							delete cv2;
						}
					}
					else
					{
						p = cv->WriteToPtr(p, false, kMaxBytesForKeyStrings);
					}
				}
			}
			else
			{
				err = ThrowError( memfull, DBaction_BuildingIndexKey);
			}
		}
	}

	if (err != VE_OK && res != nil)
		delete res;

	if (err != VE_OK)
		err = ThrowError( VE_DB4D_CANNOT_BUILD_INDEXKEY, DBaction_BuildingIndexKey);

	return res;
}


void IndexInfoFromMultipleField::SwapByteKeyData(BTitemIndex* key)
{
	sLONG i, nb = frefs->GetCount();
	void* p = (void*)&key->data;
	for (i = 0; i < nb; i++)
	{
		Field* crit = (*frefs)[i].crit;
		const VValueInfo* vv = VValue::ValueInfoFromValueKind((ValueKind)crit->GetTyp());
		if (testAssert(vv != nil))
		{
			p = vv->ByteSwapPtr(p, false);
		}
	}
}


VError IndexInfoFromMultipleField::BuildVValuesFromKey(const BTitemIndex* key, ListOfValues& outValues)
{
	VError err = VE_OK;
	sLONG i, nb = frefs->GetCount();
	Table* fic = GetTargetTable();

	if (outValues.SetCount(nb, nil))
	{
		void* p = (void*)&key->data;
		for (i = 0; i < nb && err == VE_OK; i++)
		{
			Field* crit = (*frefs)[i].crit;
			ValPtr cv = nil;
			CreVValue_Code Code = FindCV(crit->GetTyp());
			if (Code == nil)
				err = VE_DB4D_FIELDDEFCODEMISSING;
			else
			{
				cv = (*Code)(fic->GetDF(),crit->GetLimitingLen(), p, false, false, NULL, creVValue_default);
				if (cv == nil)
					err = ThrowError( memfull, DBaction_BuildingValue);
				else
				{
					outValues[i] = cv;
					p = (void*) ( ((char*)p)+cv->GetSpace() );
				}
			}
		}
	}
	else
	{
		err = ThrowError( memfull, DBaction_BuildingValue);
	}

	if (err != VE_OK)
	{
		for (ListOfValues::Iterator cur = outValues.First(), end = outValues.End(); cur != end; cur++)
		{
			if (*cur != nil)
				delete *cur;
		}
	}
	return err;
}


ValPtr IndexInfoFromMultipleField::CreateVValueWithKey(const BTitemIndex* key, VError &err)
{
	err = VE_OK;
	VString* cv = new VString;
	if (cv == nil)
		err = ThrowError( memfull, DBaction_BuildingValue);
	else
	{
		ListOfValues values;
		err = BuildVValuesFromKey(key, values);
		if (err == VE_OK)
		{
			for (ListOfValues::Iterator cur = values.First(), end = values.End(); cur != end; cur++)
			{
				VValueSingle* val = *cur;
				if (val != nil)
				{
					VStr<255> s;
					val->GetValue(s);
					if (cur != values.First())
						(*cv) += L" , ";
					(*cv) += s;
					delete val;
					*cur = nil;
				}
			}
		}
		if (err != VE_OK)
		{
			delete cv;
			cv = nil;
		}
	}
	return cv;
}


BTitemIndex* IndexInfoFromMultipleField::BuildKey(FicheInMem *rec, VError &err, Boolean OldOne)
{
	Boolean good = true;
	BTitemIndex* res = nil;
	ValPtr cv;
	assert(rec != nil);
	err = VE_OK;
	Boolean isnull = false;

	sLONG len = 0;
	sLONG i, nb = frefs->GetCount();
	for (i = 0; i < nb; i++)
	{
		Field* crit = (*frefs)[i].crit;
		if (OldOne)
			cv = rec->GetFieldOldValue(crit, err, true);
		else
			cv = rec->GetFieldValue(crit, err, false, true);

		if (cv == nil)
		{
			good = false;
			break;
		}
		else
		{
			isnull = isnull || cv->IsNull();
			len = len + (sLONG) cv->GetSpace(kMaxBytesForKeyStrings);
		}
	}


	if (good)
	{
		sLONG len2 = sizeof(BTitemIndex)- sizeof(void*) + len;
		void* p2 = GetFastMem(len2, true, 'iKe8');
		if (p2 != nil)
		{
			res=new (p2) BTitemIndex();
			void* p = (void*) &res->data;
			for (i = 0; i < nb; i++)
			{
				Field* crit = (*frefs)[i].crit;
				if (OldOne)
					cv = rec->GetFieldOldValue(crit, err, true);
				else
					cv = rec->GetFieldValue(crit, err, false, true);
				if (cv == nil)
					break;
				else
					p = cv->WriteToPtr(p, false, kMaxBytesForKeyStrings);
			}
			if (isnull)
				res->SetNull();
		}
		else
		{
			err = ThrowError( memfull, DBaction_BuildingIndexKey);
		}
	}

	if (err != VE_OK)
		err = ThrowError( VE_DB4D_CANNOT_BUILD_INDEXKEY, DBaction_BuildingIndexKey);

	return res;
}


BTitemIndex* IndexInfoFromMultipleField::CreateIndexKey(const void* p)
{
	BTitemIndex* val;

	sLONG len = 0;
	char* p3 = (char*)p;
	sLONG i, nb = frefs->GetCount();
	for (i = 0; i < nb; i++)
	{
		Field* crit = (*frefs)[i].crit;
		sLONG len3 = crit->CalcDataSize(p3);
		p3 = p3 + len3;
		len = len + len3;
	}

	sLONG len2 = sizeof(BTitemIndex)- sizeof(void*) + len;
	void* p2 = GetFastMem(len2, true, 'iKe9');
	if (p2 != nil)
	{	
		val=new (p2) BTitemIndex();
		vBlockMove(p, &val->data, len);
	}
	else
		val = nil;

	return(val);
}


sLONG IndexInfoFromMultipleField::CalulateFullKeyLengthInMem(const BTitemIndex* u)
{
	return CalulateKeyLength(u)+sizeof(BTreePageIndex*);
}


sLONG IndexInfoFromMultipleField::CalulateKeyLength(const BTitemIndex* u)
{
	sLONG res = sizeof(BTitemIndex)- sizeof(BTreePageIndex*) - sizeof(void*);
	sLONG i, nb = frefs->GetCount();
	char* p = (char*)(&(u->data));

	for (i = 0; i < nb; i++)
	{
		Field* crit = (*frefs)[i].crit;
		sLONG len = crit->CalcDataSize(p);
		p = p + len;
		res = res + len;
	}
	return res;
}


sLONG IndexInfoFromMultipleField::CalulateDataPtrKeyLength(void* data)
{
	sLONG res = sizeof(BTitemIndex)- sizeof(BTreePageIndex*) - sizeof(void*);
	sLONG i, nb = frefs->GetCount();
	char* p = (char*)data;

	for (i = 0; i < nb; i++)
	{
		Field* crit = (*frefs)[i].crit;
		sLONG len = crit->CalcDataSize(p);
		p = p + len;
		res = res + len;
	}
	return res;
}


sLONG IndexInfoFromMultipleField::CompareKeys(const BTitemIndex *val1, const BTitemIndex *val2, const VCompareOptions& inOptions)
{
	if (val1 == nil)
	{
		if (val2 == nil)
			return CR_EQUAL;
		else
			return CR_SMALLER;
	}
	else
	{
		if (val2 == nil)
		{
			return CR_BIGGER;
		}
		else
		{
			if (inOptions.IsDiacritical())
			{
				VCompareOptions nondiac = inOptions;
				nondiac.SetDiacritical(false);
				sLONG res = CompareKeys(val1, val2, nondiac);
				if (res != CR_EQUAL)
				{
					return res;
				}
				// on continue s'il y a egalite sur le non diacritique
			}

			const char* p1 = (const char*)(&(val1->data));
			const char* p2 = (const char*)(&(val2->data));
			sLONG i, nb = frefs->GetCount();
			for (i = 0; i < nb; i++)
			{
				Field* crit = (*frefs)[i].crit;
				CompareResult res = crit->CompareKeys(p1, p2, inOptions);
				if (res != CR_EQUAL)
					return res;
				p1 = p1 + crit->CalcDataSize(p1);
				p2 = p2 + crit->CalcDataSize(p2);
			}
			return CR_EQUAL;
		}
	}
}


#if 0
VArrayStruct* IndexInfoFromMultipleField::GetSourceLang()
{
	Open(index_write);
	if (!sourcelang_isinited)
	{
		CLanguage* language = VDBMgr::GetLanguage();
		CDB4D_Lang* lang4d = VDBMgr::GetLang4D();
		if (language != nil && lang4d != nil)
		{
			language->InitArray(&fSourceLang, lang4d->GetElemOfArrayOfFieldDef());
			sLONG i, nb = frefs->GetCount();
			for (i = 0; i < nb; i++)
			{
				CDB4DField* xcri = new VDB4DField((*frefs)[i].crit);
				language->AppendToArray(&fSourceLang, (VClassInstance)xcri, false);
				// dois je faire un release sur xcri ? , a verifier !
			}
			sourcelang_isinited = true;
		}

	}
	Close();
	return &fSourceLang;
}
#endif



void IndexInfoFromMultipleField::CheckIfIsDelayed()
{
	(*frefs)[0].fic->AddOneIndexToDelay(this);

	/*
	if ((*frefs)[0].fic->AreIndexesDelayed())
		DelayIndex();
		*/
}


void IndexInfoFromMultipleField::SetDelayIndex(sLONG inAlreadyDelayRequestCount)
{
	//Open(index_write);
	VTaskLock lock(&fDelayRequestCountMutex);
	fDelayRequestCount = inAlreadyDelayRequestCount;
	SetInvalidOnDisk();
	//Close();

}


void IndexInfoFromMultipleField::DelayIndex()
{
	//Open(index_write);
	VTaskLock lock(&fDelayRequestCountMutex);
	fDelayRequestCount++;
	if (fDelayRequestCount == 1)
	{
		SetInvalidOnDisk();
		// d'autre choses a faire plus tard
	}
	//Close();
}


void IndexInfoFromMultipleField::AwakeIndex(VDB4DProgressIndicator* inProgress)
{
	//Open(index_write);
	VTaskLock lock(&fDelayRequestCountMutex);
	if (fDelayRequestCount > 0)
	{
		fDelayRequestCount--;
		if (fDelayRequestCount == 0)
		{
			SetInvalidOnDisk();
			AddRebuildIndex(this, inProgress, nil);
		}
	}
	else
	{
		sLONG xbreak = 1; // put a break here
		assert(false);
	}
	//Close();
}



Boolean IndexInfoFromMultipleField::LockTargetTables(BaseTaskInfo* context)
{
	Table* fic = (*frefs)[0].fic;
	if (fic != nil && fic->GetDF() != nil)
	{
		context->SetTimeOutForOthersPendingLock(-1);
		fic->GetDF()->LockTable(context, true, -1);
		return true;
	}
	else
		return false;
}


void IndexInfoFromMultipleField::UnLockTargetTables(BaseTaskInfo* context)
{
	Table* fic = (*frefs)[0].fic;
	if (fic != nil && fic->GetDF() != nil)
	{
		fic->GetDF()->UnLockTable(context);
	}
}




VError IndexInfoFromMultipleField::JoinWithOtherIndex(IndexInfo* other, Bittab* filtre1, Bittab* filtre2,ComplexSelection* result, BaseTaskInfo* context, 
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

		IndexKeyIterator iter1(this, filtre1, context, nil, L"", curstack);
		IndexKeyIterator iter2(other, filtre2, context, nil, L"", curstack);

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
			sLONG resultcompare = CompareKeys(iter1.GetKey(),iter2.GetKey(), inOptions);
			if (resultcompare == CR_EQUAL)
			{
				BTitemIndex* val = CopyKey(iter1.GetKey());
				RecIDType recnum1 = iter1.GetRecNum();
				selrow[0] = recnum1;

				vector<RecIDType> recordsNums;
				//try
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
							if (CompareKeys(iter2.GetKey(),val, inOptions) == CR_EQUAL)
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
							resultcompare = CompareKeys(iter1.GetKey(), val, inOptions);
							if (resultcompare == CR_EQUAL)
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
				/*
				catch (...)
				{
					err = ThrowBaseError(memfull, noaction);
				}
				*/
				FreeKey(val);
			}
			else
			{
				if (resultcompare == CR_SMALLER)
				{
					if (leftjoin)
					{
						selrow[0] = iter1.GetRecNum();
						selrow[1] = -2;
						err = result->AddRow(selrow);
					}
					cont1 = iter1.NextKey(context, err);
				}
				else
				{
					if (rightjoin)
					{
						selrow[0] = -2;
						selrow[1] = iter2.GetRecNum();
						err = result->AddRow(selrow);
					}
					cont2 = iter2.NextKey(context, err);
				}

			}

		}

		if (err == VE_OK)
		{
			if (leftjoin)
			{
				while (cont1)
				{
					selrow[0] = iter1.GetRecNum();
					selrow[1] = -2;
					err = result->AddRow(selrow);

					cont1 = iter1.NextKey(context,err);
				}
			}
			if (rightjoin)
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



IndexInfo* CreIndexInfoFromMultipleField(void)
{
	IndexInfoFromMultipleField *ind;
	
	ind=new IndexInfoFromMultipleField;
	return(ind);
}

									/* -----------------------------------------------  */
									



IndexInfo* CreateIndexInfo(sLONG ll)
{
	IndexInfo* result;
	CreIndex_Code Code;

	Code=FindIndex(ll);
	if (Code == nil) result = nil;
	else
	{
		result=(*Code)();
		if (result!=nil) result->SetTyp(ll);
	}
	return(result);
		
}

IndexHeader* CreateIndexHeader(sLONG ll, IndexInfo *xentete, sLONG xDataKind)
{
	IndexHeader* result;
	CreHeader_Code Code;
	
	switch (xDataKind)
	{
		case VK_WORD:
			result = new IndexHeaderBTreeScalar<sWORD, kNbKeysForScalar>(xentete, ll == DB4D_Index_BtreeWithCluster);			
			break;
			
		case VK_BYTE:
			result = new IndexHeaderBTreeScalar<sBYTE, kNbKeysForScalar>(xentete, ll == DB4D_Index_BtreeWithCluster);			
			break;

		case VK_BOOLEAN:
			result = new IndexHeaderBTreeScalar<uBYTE, kNbKeysForScalar>(xentete, ll == DB4D_Index_BtreeWithCluster);			
			break;

		case VK_LONG:
			result = new IndexHeaderBTreeScalar<sLONG, kNbKeysForScalar>(xentete, ll == DB4D_Index_BtreeWithCluster);			
			break;

		case VK_REAL:
			result = new IndexHeaderBTreeScalar<Real, kNbKeysForScalar>(xentete, ll == DB4D_Index_BtreeWithCluster);		
			break;

		case VK_TIME:
			result = new IndexHeaderBTreeScalar<uLONG8, kNbKeysForScalar>(xentete, ll == DB4D_Index_BtreeWithCluster);
			break;

		case VK_UUID:
			result = new IndexHeaderBTreeScalar<VUUIDBuffer, kNbKeysForScalar>(xentete, ll == DB4D_Index_BtreeWithCluster);
			break;
			
		case VK_SUBTABLE:
		case VK_SUBTABLE_KEY:
		case VK_LONG8:
		case VK_DURATION:
			result = new IndexHeaderBTreeScalar<sLONG8, kNbKeysForScalar>(xentete, ll == DB4D_Index_BtreeWithCluster);		
			break;

		default:
			Code=FindHeader(ll);
			if (Code == nil) 
				result = nil;
			else 
				result=(*Code)(xentete);
			break;
	}
	
	return(result);
		
}




IndexInfoFromField* CreateIndexInfoField(Base4D* owner, sLONG nf, sLONG nc, sLONG typindex, Boolean UniqueKeys, sLONG datakind)
{
	IndexInfoFromField* ind = nil;
	switch (datakind) 
	{
		case VK_WORD:
			ind = new IndexInfoScalar<sWORD, kNbKeysForScalar>(owner, nf, nc, typindex, UniqueKeys );
			break;

		case VK_BYTE:
			ind = new IndexInfoScalar<sBYTE, kNbKeysForScalar>(owner, nf, nc, typindex, UniqueKeys );
			break;

		case VK_BOOLEAN:
			ind = new IndexInfoScalar<uBYTE, kNbKeysForScalar>(owner, nf, nc, typindex, UniqueKeys );
			break;

		case VK_LONG:
			ind = new IndexInfoScalar<sLONG, kNbKeysForScalar>(owner, nf, nc, typindex, UniqueKeys );
			break;

		case VK_REAL:
			ind = new IndexInfoScalar<Real, kNbKeysForScalar>(owner, nf, nc, typindex, UniqueKeys );
			break;

		case VK_TIME:
			ind = new IndexInfoScalar<uLONG8, kNbKeysForScalar>(owner, nf, nc, typindex, UniqueKeys );
			break;

		case VK_UUID:
			ind = new IndexInfoScalar<VUUIDBuffer, kNbKeysForScalar>(owner, nf, nc, typindex, UniqueKeys );
			break;
			
		case VK_SUBTABLE:
		case VK_SUBTABLE_KEY:
		case VK_LONG8:
		case VK_DURATION:
			ind = new IndexInfoScalar<sLONG8, kNbKeysForScalar>(owner, nf, nc, typindex, UniqueKeys );
			break;

		default:
			ind = new IndexInfoFromField(owner, nf, nc, typindex, UniqueKeys, false );
			break;
	}
	return ind;
}




IndexInfoFromField* CreateEmptyIndexInfoFieldScalar(sLONG datakind)
{
	IndexInfoFromField* ind = nil;
	switch (datakind) 
	{
		case VK_WORD:
			ind = new IndexInfoScalar<sWORD, kNbKeysForScalar>( );
			break;

		case VK_BYTE:
			ind = new IndexInfoScalar<sBYTE, kNbKeysForScalar>( );
			break;

		case VK_BOOLEAN:
			ind = new IndexInfoScalar<uBYTE, kNbKeysForScalar>( );
			break;

		case VK_LONG:
			ind = new IndexInfoScalar<sLONG, kNbKeysForScalar>( );
			break;

		case VK_REAL:
			ind = new IndexInfoScalar<Real, kNbKeysForScalar>( );
			break;

		case VK_TIME:
			ind = new IndexInfoScalar<uLONG8, kNbKeysForScalar>( );
			break;

		case VK_UUID:
			ind = new IndexInfoScalar<VUUIDBuffer, kNbKeysForScalar>( );
			break;
			
		case VK_SUBTABLE:
		case VK_SUBTABLE_KEY:
		case VK_LONG8:
		case VK_DURATION:
			ind = new IndexInfoScalar<sLONG8, kNbKeysForScalar>( );
			break;

	}
	return ind;
}



									/* -----------------------------------------------  */



void IndexAction::SetAction(sLONG xindexcommande, Base4D *xdb, sLONG n, VDB4DProgressIndicator* InProgress)
{
	indexcommande=xindexcommande;
	db=xdb;
	num=n;
	fProgress = InProgress;
	Ind = nil;
}


void IndexAction::SetAction(sLONG xindexcommande, Base4D *xdb, const VUUID& id, VDB4DProgressIndicator* InProgress)
{
	indexcommande=xindexcommande;
	db=xdb;
	num = -2;
	IndexID = id;
	fProgress = InProgress;
	Ind = nil;
}


void IndexAction::SetAction(sLONG xindexcommande, IndexInfo* ind, VDB4DProgressIndicator* InProgress)
{
	indexcommande=xindexcommande;
	ind->Retain();
	Ind=ind;
	num = -2;
	db = ind->GetDB();
	fProgress = InProgress;
}

/*
IndexAction::~IndexAction()
{
	if (Ind)
		Ind->Release();
}
*/

/*
IndexAction* IndexActionRoot = nil;
IndexAction* IndexActionLast = nil;

Obj4D GlobListIndex;
*/

IndexActionContainer GlobListIndex;
VCriticalSection GlobListIndexMutex;
IndexInfo* GlobIndexEncours = nil;
uLONG8 GlobIndexActionStamp = 0;
uLONG8 GlobIndexActionStampForCount = 0;

void SetIndexEncours(IndexInfo* ind)
{
	VTaskLock lock(&GlobListIndexMutex);
	GlobIndexEncours = ind;
}

inline Boolean CheckIfDBDataisOpen(IndexAction& aa)
{
	Boolean res = false;
	if (aa.db->IsOkToOccupe())
	{
		res = aa.db->IsDataOpened() && !aa.db->CheckIfIndexesAreDelayed();
		aa.db->libere();
	}
	return res;
}


inline Boolean CountIfDBDataisOpen(IndexAction& aa)
{
	Boolean res = false;
	if (aa.db->IsOkToOccupe())
	{
		res = aa.db->IsDataOpened() && !aa.db->CheckIfIndexesAreDelayed();
		aa.db->libere();
	}
	if (res)
	{
		if (aa.fStamp == 0)
			aa.fStamp = GlobIndexActionStampForCount+1;

		if (aa.fStamp <= GlobIndexActionStampForCount)
			res = false;
	}
	else
	{
		aa.fStamp = 0;
	}
	return res;
}

static void AddIndexAction(IndexAction& aa, VSyncEvent* event)
{
	VTaskLock lock(&GlobListIndexMutex);

	if (aa.fProgress == nil)
		aa.fProgress = VDBMgr::GetManager()->RetainDefaultProgressIndicator_For_Indexes();
	else
		aa.fProgress->Retain();

	if (event != nil)
	{
		event->Retain();
	}
	aa.fEvent = event;
	GlobIndexActionStamp++;
	aa.fStamp = GlobIndexActionStamp;

	if (/*aa.indexcommande == indexsupprime || aa.indexcommande == indexajoute || */aa.indexcommande == indexrebuild)
	{
		for (IndexActionContainer::iterator cur = GlobListIndex.begin(); cur != GlobListIndex.end(); )
		{
			bool canerase = false;
			IndexAction* xcur = &(*cur);
			if (xcur->indexcommande == aa.indexcommande)
			{
				if (aa.indexcommande == indexsupprime)
				{
					if (xcur->IndexID == aa.IndexID)
					{
						canerase = true;
					}
				}
				else
				{
					if (xcur->Ind == aa.Ind)
					{
						canerase = true;
					}
				}

			}
			if (canerase)
			{
				IndexActionContainer::iterator todel = cur;
				cur++;
				xcur->Dispose(true);
				GlobListIndex.erase(todel);
			}
			else
				cur++;
		}
	}

	GlobListIndex.push_back(aa);
	if (AllowWakeUpAfterIndexAction)
		gIndexBuilder->WakeUp();
}


Boolean SomeIndexesPending(const Base4D* onBase)
{
	VTaskLock lock(&GlobListIndexMutex);
	Boolean result = false;
	for (IndexActionContainer::iterator cur = GlobListIndex.begin(), end = GlobListIndex.end(); cur != end; cur++)
	{
		if (cur->db == onBase)
		{
			result = true;
			break;
		}
	}
	return result;
}


Boolean GetNextIndexAction(IndexAction& outAction, uLONG8& startingstamp, sLONG& outRemain)
{
	VTaskLock lock(&GlobListIndexMutex);

	if (GlobListIndex.empty())
		return false;
	else
	{
		GlobIndexActionStampForCount = startingstamp;
		outRemain = (sLONG)std::count_if(GlobListIndex.begin(), GlobListIndex.end(), CountIfDBDataisOpen);
		IndexActionRef found = std::find_if(GlobListIndex.begin(), GlobListIndex.end(), CheckIfDBDataisOpen);
		startingstamp = GlobIndexActionStamp;
		if (found != GlobListIndex.end())
		{
			outAction = *found;
			if (outAction.fProgress == nil)
				outAction.fProgress = new SimpleTextProgressIndicator();
			SetIndexEncours(outAction.Ind);
			GlobListIndex.erase(found);
		}
		else
			return false;
	}

	return true;
}


void IndexAction::Dispose(Boolean ProgressAlso)
{
	if (Ind != nil)
		Ind->Release();
	if (ProgressAlso && fProgress != nil)
		fProgress->Release();
	if (fEvent != nil)
	{
		fEvent->Unlock();
		fEvent->Release();
	}
}

class IndexActionMatchDB
{
public:
	inline IndexActionMatchDB(Base4D* db) { fDB = db; };
	inline Boolean operator()(IndexAction& valtocompare) { return fDB == valtocompare.db; };

	Base4D *fDB;
};


class IndexActionDisposeMatchingDB
{
public:
	inline IndexActionDisposeMatchingDB(Base4D* db) { fDB = db; };
	inline void operator()(IndexAction& valtocompare) { if (valtocompare.db == fDB) valtocompare.Dispose(true); };

	Base4D *fDB;
};


void RemoveIndexActionsForDB(Base4D* db)
{
	Boolean mustwait = false;
	{
		VTaskLock lock(&GlobListIndexMutex);
		std::for_each(GlobListIndex.begin(), GlobListIndex.end(), IndexActionDisposeMatchingDB(db));
		GlobListIndex.remove_if(IndexActionMatchDB(db));
		if (GlobIndexEncours != nil && GlobIndexEncours->GetDB() == db)
			mustwait = true;
	}

	while (mustwait)
	{
		VTaskMgr::Get()->GetCurrentTask()->Sleep(500);
		{
			VTaskLock lock(&GlobListIndexMutex);
			if (GlobIndexEncours != nil && GlobIndexEncours->GetDB() == db)
				mustwait = true;
			else
				mustwait = false;
		}
	}
}


Boolean StillIndexingOn(Base4D* db)
{
	VTaskLock lock(&GlobListIndexMutex);

	IndexActionRef found = std::find_if(GlobListIndex.begin(), GlobListIndex.end(), IndexActionMatchDB(db));
	if (found != GlobListIndex.end())
	{
		return true;
	}
	else
	{
		if (GlobIndexEncours != nil && GlobIndexEncours->GetDB() == db)
			return true;
		else
			return false;
	}
}


void AddDeleteIndex(Base4D *xdb, const VUUID& id, VDB4DProgressIndicator* InProgress, VSyncEvent* event)
{
	IndexAction aa;
	
	DebugMsg("AddDeleteIndex \n");
	xdb->RemoveDependenciesFromIndex(id);
	
	aa.SetAction(indexsupprime, xdb, id, InProgress);
	AddIndexAction(aa, event);
	
}


void AddRebuildIndex(IndexInfo* Ind, VDB4DProgressIndicator* InProgress, VSyncEvent* event)
{
	IndexAction aa;

	aa.SetAction(indexrebuild, Ind, InProgress);
	AddIndexAction(aa, event);

}

					
void AddCreateIndex(IndexInfo* Ind, VDB4DProgressIndicator* InProgress, VSyncEvent* event)
{
	IndexAction aa;
	
	if (Ind->GetDB()->AreIndexesDelayed() || !Ind->GetDB()->IsDataOpened())
	{
		Ind->SaveInStruct(true);
	}
	aa.SetAction(indexajoute, Ind, InProgress);
	AddIndexAction(aa, event);

}


void AddDeleteDataFile(Base4D *xdb, sLONG n, VDB4DProgressIndicator* InProgress, VSyncEvent* event) // obsolete
{
	IndexAction aa;
	
	aa.SetAction(datafilesupprime, xdb, n, InProgress);
	AddIndexAction(aa, event);
	
}
					

									/* -----------------------------------------------  */


CodeReg *Index_CodeReg = nil;
CodeReg *Header_CodeReg = nil;
IndexBuilderTask* gIndexBuilder = nil;
bool AllowWakeUpAfterIndexAction = true;



void InitIndex(VDBMgr* db4d)
{
	VTaskLock lock(&GlobListIndexMutex);

	if (Index_CodeReg == nil)
		Index_CodeReg = new CodeReg;

	RegisterIndex(DB4D_Index_OnOneField,&CreIndexInfoFromField);
	RegisterIndex(DB4D_Index_OnMultipleFields,&CreIndexInfoFromMultipleField);
	RegisterIndex(DB4D_Index_OnKeyWords,&CreIndexInfoFromFieldLexico);
	
	RegisterIndex(DB4D_Index_OnOneField_Scalar_Byte,&CreIndexInfoFromField_Byte);
	RegisterIndex(DB4D_Index_OnOneField_Scalar_Word,&CreIndexInfoFromField_Word);
	RegisterIndex(DB4D_Index_OnOneField_Scalar_Long,&CreIndexInfoFromField_Long);
	RegisterIndex(DB4D_Index_OnOneField_Scalar_Long8,&CreIndexInfoFromField_Long8);
	RegisterIndex(DB4D_Index_OnOneField_Scalar_Real,&CreIndexInfoFromField_Real);
	RegisterIndex(DB4D_Index_OnOneField_Scalar_Time,&CreIndexInfoFromField_Time);
	RegisterIndex(DB4D_Index_OnOneField_Scalar_Bool,&CreIndexInfoFromField_Bool);
	RegisterIndex(DB4D_Index_OnOneField_Scalar_UUID,&CreIndexInfoFromField_UUID);

	if (Header_CodeReg == nil)
		Header_CodeReg = new CodeReg;
	RegisterHeader(DB4D_Index_Btree,&CreIndexHeaderBTree);
	RegisterHeader(DB4D_Index_BtreeWithCluster,&CreIndexHeaderBTreeCluster);
	RegisterHeader(DB4D_Index_AutoType,&CreIndexHeaderBTree);
#if 0
	RegisterHeader(DB4D_Index_BtreeWithFixedSizePages,&CreIndexHeaderBTreeFixSize);
	RegisterHeader(DB4D_Index_BtreeWithClusterAndFixedSizePages,&CreIndexHeaderBTreeFixSizeCluster);
#endif
	
	if (gIndexBuilder == nil)
	{
		gIndexBuilder = new IndexBuilderTask(db4d);
		gIndexBuilder->SetName( CVSTR( "DB4D Index builder"));
		gIndexBuilder->Run();
	}
	
}

void DeInitIndex()
{
	VTaskLock lock(&GlobListIndexMutex);
	
	if (Index_CodeReg != nil) {
		delete Index_CodeReg;
		Index_CodeReg = nil;
	}
	
	if (Header_CodeReg != nil) {
		delete Header_CodeReg;
		Header_CodeReg = nil;
	}
	
	if (gIndexBuilder != nil)
	{
		gIndexBuilder->Kill();
		gIndexBuilder->WakeUp();
		gIndexBuilder->WaitForDeath( 10000);
		ReleaseRefCountable( &gIndexBuilder);		
	}

}


									/* -----------------------------------------------  */

void IndexBuilderTask::DoInit()
{
	// on attend que notre createur soit completement initialise
	Sleep( 500L);
}

Boolean IndexBuilderTask::DoRun()
{
	IndexInfo* Ind;
	Boolean cont = true;
	
	if ( GetState() < TS_DYING ) 
	{
		IndexAction aa;

		sLONG nbnewactions = 0;
		if (GetNextIndexAction(aa, startingstamp, nbnewactions))
		{
			// sc 09/01/2008 on utilise en priorite le progress indicator de l'action
			if (aa.fProgress != nil && progress != nil)
			{
				if (progress->GetUUID() != aa.fProgress->GetUUID())
				{
					progress->EndSession();
					progress->Release();
					progress = nil;
				}
			}

			if (progress == nil)
			{
				nbaction = nbnewactions;
				curaction = 0;

				if (aa.fProgress != nil)
				{
					progress = aa.fProgress;
					progress->Retain();
				}
				else
				{
					progress = VDBMgr::GetManager()->RetainDefaultProgressIndicator_For_Indexes();
				}

				if (progress != nil)
				{
					progress->BeginSession(nbaction, L"Indexing",false);
				}
			}
			else
			{
				if (nbnewactions > 0)
				{
					nbaction = nbaction + nbnewactions;
					progress->SetMaxValue(nbaction);
				}
			}

			if (progress != nil)
			{
				curaction++;
				progress->Progress(curaction);
			}

			if (aa.indexcommande == indexsupprime)
			{
				aa.db->KillIndex(aa.IndexID, progress);
			}
			else if (aa.indexcommande == indexajoute)
			{
				aa.Ind->GetDB()->BuildIndex(aa.Ind, progress);
			}
			else if (aa.indexcommande == indexrebuild)
			{
				aa.Ind->GetDB()->ReBuildIndex(aa.Ind, progress);
			}
			else if (aa.indexcommande == datafilesupprime)
			{
				sLONG oldcount = 0;
				if (aa.fEvent != nil)
					oldcount = aa.fEvent->GetRefCount();
				aa.db->KillDataFile(aa.num,  progress, aa.fEvent, false, -1);
				// si le killdata ne s'est pas fait parce que la DataTable etait busy alors on a redemande une action
				if (aa.fEvent != nil)
				{
					if (aa.fEvent->GetRefCount() != oldcount)
					{
						aa.fEvent->Release();
						aa.fEvent = nil;
					}
				}
			}

			SetIndexEncours(nil);
			aa.Dispose(true);
		}
		else if (GetState() < TS_DYING)
		{
			if (progress != nil)
			{
				progress->EndSession();
				progress->Release();
				progress = nil;
			}
			SleepFor(1000);
		}
	}
	else
		cont = false;

	return cont;
}



							/* -------------------------------------------- */


Index_NonOpened::Index_NonOpened(Base4D_NotOpened* owner, sLONG typ, VUUID& id, VString& name, sLONG numindex, Boolean isInStruct)
{
	fOwner = owner;
	fType = typ;
	fID = id;
	fName = name;
	fDefIsValid = true;
	fSourceValid = false;
	fHeaderIsValid = false;
	fTabTrouIsValid = false;
	fTabTrouHasBeenChecked = false;
	fTabTrouClustIsValid = false;
	fTabTrouClustHasBeenChecked = false;
	fNum = numindex;
	fTable = 0;
	fIndexHeaderType = 0;
	fUniqueKeys = false;

	fTempCachePageAddr = nil;
	fTempCacheClusterAddr = nil;

	fPagesMapIsValid = false;
	fPagesMapHasBeenChecked = false;

	fClustMapIsValid = false;
	fClustMapHasBeenChecked = false;

	fSomePagesAreInvalid = false;

	fPagesInMap = nil;
	fClustInMap = nil;
	fRecordsInKeys = nil;
	fDejaPages = nil;

	fCurrentKey = nil;
	fCurrentKeyLen = 0;
	fCurrentKeyQui = -1;
	fIsIndexInStruct = isInStruct;


	libere();
}


Index_NonOpened::~Index_NonOpened()
{
	CleanTempData();
	/*
	if (fTempCachePageAddr != nil)
		delete fTempCachePageAddr;

	if (fTempCacheClusterAddr != nil)
		delete fTempCacheClusterAddr;

	if (fPagesInMap != nil)
		delete fPagesInMap;

	if (fClustInMap != nil)
		delete fClustInMap;
	*/
}


VError Index_NonOpened::SetHeaderType(sLONG IndexHeaderType, ToolLog* log)
{
	VError errexec = VE_OK;
	fIndexHeaderType = IndexHeaderType;
	if (IndexHeaderType == DB4D_Index_Btree || IndexHeaderType == DB4D_Index_BtreeWithCluster || IndexHeaderType == DB4D_Index_AutoType)
	{
		fHeaderIsValid = true;
	}
	else
	{
		IndexDefProblem pb(TOP_IndexHeaderTypeIsInvalid, fNum);
		errexec = log->Add(pb);
	}

	return errexec;
}


VError Index_NonOpened::AddField(const VUUID& inFieldID, sLONG datakind, ToolLog* log)
{
	VError errexec = VE_OK;
	Boolean oktoadd = true;
	sLONG numtable = 0;
	sLONG numfield = 0;
	VUUIDBuffer xid;
	inFieldID.ToBuffer(xid);

	if (fOwner->ExistUUID(xid, tuuid_Field, nil, nil, &numtable, &numfield))
	{
		if (numtable <=0 || numtable > kMaxTables)
		{
			IndexDefProblem pb(TOP_WrongTableNumberInIndex, fNum);
			errexec = log->Add(pb);
			fSourceValid = false;
			oktoadd = false;
		}

		if (errexec == VE_OK)
		{
			if (numfield <=0 || numfield > kMaxFields)
			{
				IndexDefProblem pb(TOP_WrongFieldNumberInIndex, fNum);
				errexec = log->Add(pb);
				fSourceValid = false;
				oktoadd = false;
			}
		}
	}
	else
	{
		IndexDefProblem pb(TOP_WrongFieldNumberInIndex, fNum);
		errexec = log->Add(pb);
		oktoadd = false;
		fSourceValid = false;
	}

	if (errexec == VE_OK && oktoadd)
	{
		sLONG fieldtype;
		if (fOwner->ExistField(numtable, numfield, nil, &fieldtype))
		{
			if (fieldtype == VK_TEXT)
				fieldtype = VK_STRING;

			if (fieldtype == VK_IMAGE)
				fieldtype = VK_STRING;

			if (fieldtype == VK_TEXT_UTF8)
				fieldtype = VK_STRING_UTF8;

			if (fieldtype != datakind)
			{
				IndexDefProblem pb(TOP_WrongDataTypeNumberInIndex, fNum);
				errexec = log->Add(pb);
				oktoadd = false;
				fSourceValid = false;
			}

			if (fFields.size() != 0)
			{
				if (fTable != numtable)
				{
					IndexDefProblem pb(TOP_WrongTableNumberInIndex, fNum);
					errexec = log->Add(pb);
					oktoadd = false;
					fSourceValid = false;
				}
			}
			else
				fSourceValid = true;

			if (oktoadd)
			{
				try
				{
					fTable = numtable;
					fFields.push_back(numfield);
					fFieldTypes.push_back(datakind);
					fFieldInfos.push_back(VValue::ValueInfoFromValueKind((ValueKind)datakind));
				}
				catch (...)
				{
					errexec = memfull;
					fSourceValid = false;
				}
			}
		}
		else
		{
			IndexDefProblem pb(TOP_WrongFieldNumberInIndex, fNum);
			errexec = log->Add(pb);
			fSourceValid = false;
		}
	}

	return errexec;
}


VError Index_NonOpened::AddField(sLONG numtable, sLONG numfield, ToolLog* log)
{
	VError errexec = VE_OK;
	Boolean oktoadd = true;

	if (numtable <=0 || numtable > kMaxTables)
	{
		IndexDefProblem pb(TOP_WrongTableNumberInIndex, fNum);
		errexec = log->Add(pb);
		fSourceValid = false;
		oktoadd = false;
	}

	if (errexec == VE_OK)
	{
		if (numfield <=0 || numfield > kMaxFields)
		{
			IndexDefProblem pb(TOP_WrongFieldNumberInIndex, fNum);
			errexec = log->Add(pb);
			fSourceValid = false;
			oktoadd = false;
		}
	}

	if (errexec == VE_OK && oktoadd)
	{
		sLONG fieldtype;
		if (fOwner->ExistField(numtable, numfield, nil, &fieldtype))
		{
			if (fFields.size() != 0)
			{
				if (fTable != numtable)
				{
					IndexDefProblem pb(TOP_WrongTableNumberInIndex, fNum);
					errexec = log->Add(pb);
					oktoadd = false;
				}
			}
			else
				fSourceValid = true;

			if (oktoadd)
			{
				try
				{
					fTable = numtable;
					fFields.push_back(numfield);
					fFieldTypes.push_back(fieldtype);
					fFieldInfos.push_back(VValue::ValueInfoFromValueKind((ValueKind)fieldtype));
				}
				catch (...)
				{
					errexec = memfull;
					fSourceValid = false;
				}
			}
		}
		else
		{
			IndexDefProblem pb(TOP_WrongFieldNumberInIndex, fNum);
			errexec = log->Add(pb);
			fSourceValid = false;
		}
	}

	return errexec;

}


VError Index_NonOpened::ReadHeader(VStream& buf, ToolLog* log)
{
	VError errexec = VE_OK;
	VError err = VE_OK;

	if (fHeaderIsValid)
	{
		err = buf.GetData(&IHD,sizeof(IHD));
		if (err != VE_OK)
		{
			IndexDefProblem pb(TOP_IndexDefIsDamaged, fNum);
			errexec = log->Add(pb);
			fHeaderIsValid = false;
		}
		else
		{
			if (buf.NeedSwap())
			{
				IHD.SwapBytes();
			}

			if (IHD.addrprim != 0 && IHD.addrprim != -1)
			{
				if (IHD.nbpage == 0)
				{
					IndexDefProblem pb(TOP_Index_NbPagesIsInvalid, fNum);
					errexec = log->Add(pb);
					fHeaderIsValid = false;
				}

				if (errexec == VE_OK)
				{
					if (!fOwner->IsAddrValid(IHD.addrprim, false))
					{
						IndexDefProblem pb(TOP_Index_AddrOfPrimaryTabAddrOfPagesIsInvalid, fNum);
						errexec = log->Add(pb);
						fHeaderIsValid = false;
					}
				}
			}

			if (errexec == VE_OK)
			{
				if (IHD.nbpage >= 0)
				{
					if (IHD.nbpage > 0 && (IHD.addrprim == 0 || IHD.addrprim == -1))
					{
						IndexDefProblem pb(TOP_Index_NbPagesIsInvalid, fNum);
						errexec = log->Add(pb);
						fHeaderIsValid = false;
					}
					else
					{
						if (IHD.nbpage > kMaxObjsInTable)
						{
							IndexDefProblem pb(TOP_Index_NbPagesIsInvalid, fNum);
							errexec = log->Add(pb);
							fHeaderIsValid = false;
						}
					}
				}
				else
				{
					IndexDefProblem pb(TOP_Index_NbPagesIsInvalid, fNum);
					errexec = log->Add(pb);
					fHeaderIsValid = false;
				}
			}

			if (errexec == VE_OK && fHeaderIsValid)
			{
				if ( (IHD.nextfreepage <= 0 && (-IHD.nextfreepage) <= kMaxObjsInTable) || IHD.nextfreepage == kFinDesTrou )
					fTabTrouIsValid = true;
				else
				{
					IndexDefProblem pb(TOP_Index_TabTrouIsInvalid, fNum);
					errexec = log->Add(pb);
					fHeaderIsValid = false;
				}
			}

		}
	}

	if (fHeaderIsValid && errexec == VE_OK)
	{
		switch(fIndexHeaderType)
		{
			case DB4D_Index_Btree:
			case DB4D_Index_BtreeWithCluster:
			case DB4D_Index_AutoType:
				{
					err = buf.GetData(&HBT,sizeof(HBT));
					if (err != VE_OK)
					{
						IndexDefProblem pb(TOP_IndexDefIsDamaged, fNum);
						errexec = log->Add(pb);
						fHeaderIsValid = false;
					}
					else
					{
						if (buf.NeedSwap())
						{
							HBT.SwapBytes();
						}
						if ((HBT.FirstPage<-1 || HBT.FirstPage>=IHD.nbpage) && IHD.nbpage != 0)
						{
							IndexDefProblem pb(TOP_Index_FirstPageIsInvalid, fNum);
							errexec = log->Add(pb);
							fHeaderIsValid = false;
						}
					}

					if (errexec == VE_OK && fHeaderIsValid)
					{
						if (fIndexHeaderType == DB4D_Index_BtreeWithCluster)
						{
							err = buf.GetData(&IHCLUST,sizeof(IHCLUST));
							if (err != VE_OK)
							{
								IndexDefProblem pb(TOP_IndexDefIsDamaged, fNum);
								errexec = log->Add(pb);
								fHeaderIsValid = false;
							}
							else
							{
								if (buf.NeedSwap())
								{
									IHCLUST.SwapBytes();
								}

								if (IHCLUST.addrTabClust != 0)
								{
									if (IHCLUST.nbclust == 0)
									{
										IndexDefProblem pb(TOP_Index_NbClustersIsInvalid, fNum);
										errexec = log->Add(pb);
										fHeaderIsValid = false;
									}

									if (errexec == VE_OK)
									{
										if (!fOwner->IsAddrValid(IHCLUST.addrTabClust, false))
										{
											IndexDefProblem pb(TOP_Index_AddrOfPrimaryTabAddrOfClustersIsInvalid, fNum);
											errexec = log->Add(pb);
											fHeaderIsValid = false;
										}
									}
								}

								if (errexec == VE_OK)
								{
									if (IHCLUST.nbclust >= 0)
									{
										if (IHCLUST.nbclust > 0 && IHCLUST.addrTabClust == 0)
										{
											IndexDefProblem pb(TOP_Index_NbClustersIsInvalid, fNum);
											errexec = log->Add(pb);
											fHeaderIsValid = false;
										}
										else
										{
											if (IHCLUST.nbclust > kMaxObjsInTable)
											{
												IndexDefProblem pb(TOP_Index_NbClustersIsInvalid, fNum);
												errexec = log->Add(pb);
												fHeaderIsValid = false;
											}
										}
									}
									else
									{
										IndexDefProblem pb(TOP_Index_NbClustersIsInvalid, fNum);
										errexec = log->Add(pb);
										fHeaderIsValid = false;
									}
								}

								if (errexec == VE_OK && fHeaderIsValid)
								{
									if ( (IHCLUST.debuttrou <= 0 && (-IHCLUST.debuttrou) <= kMaxObjsInTable) || IHCLUST.debuttrou == kFinDesTrou )
										fTabTrouClustIsValid = true;
									else
									{
										IndexDefProblem pb(TOP_Index_TabTrouClusterIsInvalid, fNum);
										errexec = log->Add(pb);
										fHeaderIsValid = false;
									}
								}


							}
						}
					}
				}
				break;

			default:
				fHeaderIsValid = false;
				break;

		}
	}

	if (errexec != VE_OK)
		fHeaderIsValid = false;

	return errexec;
}


bool Index_NonOpened::OpenCheckKeysProgressSession(ToolLog* log, VError *outError)
{
	bool progressOpened = false;
	VError errexec = VE_OK;

	if (fHeaderIsValid && (fPagesMapIsValid || !fPagesMapHasBeenChecked))
	{
		VString s;
		log->GetVerifyOrCompactString(19,s);	// Checking Index Pages on {p1}
		VString s2;
		GetName(s2);
		FormatStringWithParamsStrings(s, &s2);
		errexec = log->OpenProgressSession(s, IHD.nbpage);
		progressOpened = (errexec == VE_OK);
	}
	*outError = errexec;
	return progressOpened;
}


VError Index_NonOpened::CheckKeys(ToolLog* log)
{
	VError errexec = VE_OK;

	if (fHeaderIsValid && (fPagesMapIsValid || !fPagesMapHasBeenChecked))
	{
		fSomePagesAreInvalid = false;

		if (HBT.FirstPage != -1)
		{
			fCurrentKey = GetFastMem(2048, false, 'IKEY');
			fCurrentKeyLen = 2048;
			fFirstKeyToCheck = true;
			sLONG curpage = 0;
			fDejaPages = new Bittab;

			errexec = CheckPage(HBT.FirstPage, log, curpage);
			if (fCurrentKey != nil)
				FreeFastMem(fCurrentKey);
			fCurrentKey = nil;
		}
	}

	return errexec;
}


VError Index_NonOpened::GetFields(std::vector<sLONG>& outFields)
{
	VError err = VE_OK;

	try
	{
		outFields.resize(fFields.size(),0);
		std::copy (fFields.begin(), fFields.end(), outFields.begin());
	}
	catch (...)
	{
		err = memfull;
	}

	return err;
}

Boolean Index_NonOpened::IsOKPageNum(sLONG pagenum, sLONG numpageparent, VError& errexec, ToolLog* log)
{
	if (pagenum < 0 || pagenum > IHD.nbpage)
	{
		IndexPageProblem pb(TOP_IndexPage_SubPageRefIsInvalid, fNum, numpageparent);
		errexec = log->Add(pb);
		return false;
	}
	else
	{
		if (fPagesInMap->isOn(pagenum))
		{
			return true;
		}
		else
		{
			IndexPageProblem pb(TOP_IndexPage_SubPageRefIsDeleted, fNum, numpageparent);
			errexec = log->Add(pb);
			return false;
		}
	}
}


VError Index_NonOpened::SwapByteKeyData(Boolean needswap, BTitemIndex* item, sLONG maxlen, Boolean& ok, sLONG numpage, ToolLog* log, CompareResult& comp)
{
	ok = true;
	VError errexec = VE_OK;
	sLONG nb = (sLONG)fFields.size();
	char* p = (char*) &(item->data);
	char* p2 = (char*) fCurrentKey;
	sLONG keylen = 0;
	comp = CR_EQUAL;
		
	Boolean strictInitial = nb == 1;

	{
		for (sLONG i = 0; i < nb && ok && errexec == VE_OK; i++)
		{
			switch(fFieldTypes[i])
			{
				case VK_STRING_UTF8:
					{
						sLONG lens = *((sLONG*)p);
						if (needswap)
							ByteSwapLong(&lens);
						if (lens > maxlen)
						{
							ok = false;
						}
					}
					break;
				case VK_STRING:
					{
						sLONG lens = *((sLONG*)p);
						if (needswap)
							ByteSwapLong(&lens);
						if (lens > maxlen)
						{
							ok = false;
						}
					}
					break;
				case VK_FLOAT:
					{
						sLONG lenf = *((sLONG*)(p+5));
						if (needswap)
							ByteSwapLong(&lenf);
						if (lenf > maxlen || lenf < 0) /* Sergiy - 5 May 2007 - Bug fix for ACI0051371 */
						{
							ok = false;
						}
					}
					break;
			}
			if (ok)
			{
				const VValueInfo* vi = fFieldInfos[i];
				if (vi != nil)
				{
					if (needswap)
						vi->ByteSwapPtr(p, false);
					if (comp == CR_EQUAL && !fFirstKeyToCheck)
					{
						comp = vi->CompareTwoPtrToData(p2, p, strictInitial);
						p2 = p2 + vi->GetSizeOfValueDataPtr(p2);
					}
					sLONG sublen = (sLONG)vi->GetSizeOfValueDataPtr(p);
					p = p + sublen;
					keylen = keylen + sublen;
				}
				else
					ok = false;
			}
		}

	}

	if (fFirstKeyToCheck)
		comp = CR_SMALLER;

	if (comp == CR_EQUAL && ok && !strictInitial && !fFirstKeyToCheck) // on refait un passage en plus strict pour la comparaison
	{
		p = (char*) &(item->data);
		p2 = (char*) fCurrentKey;

		for (sLONG i = 0; i < nb && ok && errexec == VE_OK; i++)
		{
			const VValueInfo* vi = fFieldInfos[i];
			if (vi != nil)
			{
				if (comp == CR_EQUAL)
				{
					comp = vi->CompareTwoPtrToData(p2, p, true);
					p2 = p2 + vi->GetSizeOfValueDataPtr(p2);
					if (comp != CR_EQUAL)
						break;
				}
				sLONG sublen = (sLONG)vi->GetSizeOfValueDataPtr(p);
				p = p + sublen;
			}
			else
				ok = false;
		}

	}

	if (comp == CR_EQUAL && !fFirstKeyToCheck)
	{
		if (fCurrentKeyQui > item->GetQui())
		{
			comp = CR_BIGGER;
		}
		else
		{
			if (fCurrentKeyQui < item->GetQui())
				comp = CR_SMALLER;
		}
	}

	if (ok)
	{
		fFirstKeyToCheck = false;
		if (keylen > fCurrentKeyLen)
		{
			if (fCurrentKey != nil)
				FreeFastMem(fCurrentKey);
			fCurrentKey = GetFastMem(keylen + 100, false, 'IKEY');
			fCurrentKeyLen = keylen + 100;
		}

		if (fCurrentKey == nil)
		{
			errexec = memfull;
			fCurrentKeyLen = 0;
		}
		else
		{
			fCurrentKeyQui = item->GetQui();
			vBlockMove(&(item->data), fCurrentKey, keylen);
		}
	}
	else
	{
		IndexPageProblem pb(TOP_IndexPage_KeyDataIsInvalid, fNum, numpage);
		errexec = log->Add(pb);
	}

	return errexec;
}


VError Index_NonOpened::CheckPage(sLONG numpage, ToolLog* log, sLONG &curpage)
{
	VError errexec = VE_OK;
	VError err = VE_OK;

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
							err = tag.ValidateTag(DBOH_BtreePage, numpage, -2);
							if (err != VE_OK)
							{
								IndexPageProblem pb(TOP_TagIsInvalid, fNum, numpage);
								errexec = log->Add(pb);
							}
						}

						if (err == VE_OK && errexec == VE_OK)
						{
							taglen = tag.GetLen();
							if (adjusteindex(taglen+kSizeDataBaseObjectHeader) == adjusteindex(len))
							{
								BTreePageIndexDisk* btp = (BTreePageIndexDisk*)GetFastMem(taglen, false, 'BTP ');
								if (btp != nil)
								{
									err=fOwner->readlong( btp, taglen, addr, kSizeDataBaseObjectHeader);
									if (err != VE_OK)
									{
										IndexPageProblem pb(TOP_PhysicalDataIsInvalid, fNum, numpage);
										errexec = log->Add(pb);									
									}

									if (err == VE_OK && errexec == VE_OK)
									{
										err = tag.ValidateCheckSum(	btp, taglen);
										if (err != VE_OK)
										{
											IndexPageProblem pb(TOP_CheckSumIsInvalid, fNum, numpage);
											errexec = log->Add(pb);									
										}
									}

									if (tag.NeedSwap())
									{
										btp->SwapBytes();
									}

									if (err == VE_OK && errexec == VE_OK)
									{
										if (btp->nkeys<=0 || btp->nkeys>kNbKeyParPageIndex)
										{
											IndexPageProblem pb(TOP_IndexPage_NbKeysIsInvalid, fNum, numpage);
											errexec = log->Add(pb);									
										}

										sLONG curlen = taglen;

										if (btp->CurrentSizeInMem > taglen)
										{
											IndexPageProblem pb(TOP_LengthIsInvalid, fNum, numpage);
											errexec = log->Add(pb);
										}
										else
										{
											curlen = btp->CurrentSizeInMem;
										}

										if (errexec == VE_OK)
										{
											if (btp->souspage0 != -1)
											{
												if (IsOKPageNum(btp->souspage0, numpage, errexec, log))
													errexec = CheckPage(btp->souspage0, log, curpage);
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
											for (sLONG i=0;i<btp->nkeys && errexec == VE_OK;i++)
											{
												sLONG off =btp->off[i];
												if (off < curlen && off >= BTreePageIndex::CalculateEmptyLenOnDisk())
												{
													char* x = (char*)btp/*.suite*/;
													x = x + off - sizeof(void*); // on retire la taile de sousBT qui n'est pas stockee dans la cle
													BTitemIndex* curp = (BTitemIndex*)x;

													if (tag.NeedSwap())
													{
														ByteSwap(&curp->souspage);
														ByteSwap(&curp->qui);
													}

													Boolean ok = true;
													CompareResult comp;
													errexec = SwapByteKeyData(tag.NeedSwap(), curp, curlen - off, ok, numpage, log, comp);

													if (errexec == VE_OK)
													{
														if (comp == CR_BIGGER || comp == CR_EQUAL)
														{
															IndexPageProblem pb(TOP_IndexPage_KeyOrderIsInvalid, fNum, numpage);
															errexec = log->Add(pb);;
														}
													}

													if (errexec == VE_OK)
													{
														if (curp->GetQui() > kMaxRecordsInTable)
														{
															IndexPageProblem pb(TOP_IndexPage_InvalidRecordNumber, fNum, numpage);
															errexec = log->Add(pb);;
														}
														else
														{
															if (curp->GetQui() < 0)
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
																		if (fRecordsInKeys->isOn(curp->GetQui()))
																		{
																			IndexPageProblem pb(TOP_IndexPage_DuplicatedRecordNumber, fNum, numpage);
																			errexec = log->Add(pb);;
																		}
																	}
																	if (errexec == VE_OK)
																		errexec = fRecordsInKeys->Set(curp->GetQui(), true);
																}
																if (errexec == VE_OK)
																{
																	if (DeletedRecords != nil)
																	{
																		if (DeletedRecords->isOn(curp->GetQui()))
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
																errexec = CheckPage(curp->souspage, log, curpage);
														}
													}
												}
												else
												{
													IndexPageProblem pb(TOP_IndexPage_KeyDirectoryIsInvalid, fNum, numpage);
													errexec = log->Add(pb);
													fSomePagesAreInvalid = true;
													break;
												}
											}
										}
										
									}

									if (errexec == VE_OK && log->IsCompacting())
									{
										Base4D* target = log->GetTargetCompact();
										if (fOwner->GetStruct() == nil)
											target = target->GetStructure();

										DataAddr4D pageaddr = target->findplace(adjusteindex(taglen + kSizeDataBaseObjectHeader), nil, err, -kIndexSegNum, nil);
										DataBaseObjectHeader tag2(btp, taglen, DBOH_BtreePage, numpage, -2);
										err = tag2.WriteInto(target, pageaddr, nil, -1);
										if (err == VE_OK) 
											err=target->writelong( btp ,taglen, pageaddr, kSizeDataBaseObjectHeader, nil, -1);
										OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
										log->GetTargetIndexCompact()->GetHeader()->SetPageAddr(curstack, numpage, pageaddr, adjusteindex(taglen + kSizeDataBaseObjectHeader));
										
									}

									FreeFastMem(btp);
								}
								else
									errexec = memfull;
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



VError Index_NonOpened::ReportInvalidTabAddrAddr(ToolLog* log, sLONG selector)
{
	if (selector == 0)
	{
		IndexProblem pb(TOP_Index_AddrOfPageAddressTableIsInvalid, fNum);
		return log->Add(pb);
	}
	else
	{
		IndexProblem pb(TOP_Index_AddrOfClusterAddressTableIsInvalid, fNum);
		return log->Add(pb);
	}
}


VError Index_NonOpened::ReportInvalidTabAddrTag(ToolLog* log, sLONG selector)
{
	if (selector == 0)
	{
		IndexProblem pb(TOP_Index_TagOfPageAddressTableIsInvalid, fNum);
		return log->Add(pb);
	}
	else
	{
		IndexProblem pb(TOP_Index_TagOfClusterAddressTableIsInvalid, fNum);
		return log->Add(pb);
	}
}


VError Index_NonOpened::ReportInvalidTabAddrRead(ToolLog* log, sLONG selector)
{
	if (selector == 0)
	{
		IndexProblem pb(TOP_Index_PhysicalDataOfPageAddressTableIsInvalid, fNum);
		return log->Add(pb);
	}
	else
	{
		IndexProblem pb(TOP_Index_PhysicalDataOfClusterAddressTableIsInvalid, fNum);
		return log->Add(pb);
	}
}


VError Index_NonOpened::ReportInvalidTabAddrChecksum(ToolLog* log, sLONG selector)
{
	if (selector == 0)
	{
		IndexProblem pb(TOP_Index_CheckSumOfPageAddressTableIsInvalid, fNum);
		return log->Add(pb);
	}
	else
	{
		IndexProblem pb(TOP_Index_CheckSumOfClusterAddressTableIsInvalid, fNum);
		return log->Add(pb);
	}
}



VError Index_NonOpened::CheckPagesTrous(ToolLog* log)
{
	VError errexec = VE_OK;

	fTabTrouHasBeenChecked = true;

	if (fTabTrouIsValid)
	{
		if (fPagesMapIsValid && fHeaderIsValid)
		{
			sLONG nbSets = 0;
			Bittab TempDeleted;
			{
				DataAddr4D n = IHD.nextfreepage;
				Boolean ChaineIsValid = true;
				Boolean ReferenceCirculaire = false;
				Boolean WrongReference = false;

				while (n != kFinDesTrou && ChaineIsValid && errexec == VE_OK)
				{
					DataAddr4D n2 = -n;
					if (n2>=0 && n2<IHD.nbpage)
					{
						if (TempDeleted.isOn(n2))
						{
							ChaineIsValid = false;
							ReferenceCirculaire = true;
						}
						else
						{
							errexec = TempDeleted.Set(n2, true);
							nbSets++;
							if (nbSets>1000)
							{
								TempDeleted.Epure();
								nbSets = 0;
							}
							if (errexec == VE_OK)
							{
								VError err = VE_OK;
								n = fOwner->GetAddrFromTable(fTempCachePageAddr, (sLONG)n2, IHD.addrprim, IHD.nbpage, err);
								if (err != VE_OK)
								{
									ChaineIsValid = false;
									WrongReference = true;
								}
							}
						}
					}
					else
					{
						WrongReference = true;
						ChaineIsValid = false;
					}
				}

				if (errexec == VE_OK)
				{
					if (!ChaineIsValid)
					{
						fTabTrouIsValid = false;
						if (WrongReference)
						{
							IndexProblem pb(TOP_Index_TabTrouIsInvalid, fNum);
							errexec = log->Add(pb);
						}
						else
						{
							IndexProblem pb(TOP_Index_TabTrouIsInvalid, fNum);
							errexec = log->Add(pb);
						}
					}
				}

			}
		}
		else
			fTabTrouIsValid = false;
	}


	if (errexec != VE_OK)
		fTabTrouHasBeenChecked = false;

	return errexec;
}


VError Index_NonOpened::CheckPagesAddrs(DataAddr4D ou, sLONG nbpagesmax, sLONG nbpagestocheck, sLONG pos1, sLONG pos2, ToolLog* log, sLONG mastermultiplicateur)
{
	VError errexec = VE_OK;
	VError err = VE_OK;

	TabAddrDisk taddr;
	log->MarkAddr_TabAddr_Index(ou, kSizeTabAddr+kSizeDataBaseObjectHeader, fNum, -1, -1);
	errexec = fOwner->ReadAddressTable(ou, taddr, pos1, pos2, log, this, err, 0, false);
	if (errexec == VE_OK)
	{
		if (err == VE_OK)
		{
			if (nbpagesmax > kNbElemTabAddr)
			{
				sLONG diviseur = kNbElemTabAddr;
				if (nbpagesmax > kNbElemTabAddr2)
					diviseur = kNbElemTabAddr2;

				sLONG nbparents = (nbpagestocheck+diviseur-1) / diviseur;
				for (sLONG i=0; i<nbparents && errexec == VE_OK; i++)
				{
					sLONG nbremains = diviseur;
					if (i == (nbparents-1))
						nbremains = nbpagestocheck & (diviseur-1);
					if (nbremains == 0)
						nbremains = diviseur;
					if (taddr[i].len != kSizeTabAddr)
					{
						Index_PageAddrProblem pb(TOP_LenghOfAddressTableIsInvalid, fNum, mastermultiplicateur + i);
						errexec = log->Add(pb);
						fPagesMapIsValid = false;
					}
					if (errexec == VE_OK)
						errexec = CheckPagesAddrs(taddr[i].addr, diviseur - 1, nbremains, i, -1, log, mastermultiplicateur + i * diviseur);
				}
			}
			else
			{
				for (sLONG i=0; i<nbpagestocheck && errexec == VE_OK; i++)
				{
					errexec = log->Progress(mastermultiplicateur + i);					
					if (errexec == VE_OK)
					{
						DataAddr4D addr = taddr[i].addr;
						if (fTempCachePageAddr != nil)
							fTempCachePageAddr->AddAddr(mastermultiplicateur + i, addr, taddr[i].len);
						if (addr <= 0)
						{
							DataAddr4D addr2 = -addr;
							if (addr2 < IHD.nbpage || addr == kFinDesTrou)
							{
								// on sait que la page est detruite
							}
							else
							{
								IndexPageProblem pb(TOP_AddrIsInvalid, fNum, mastermultiplicateur + i);
								errexec = log->Add(pb);
								fPagesMapIsValid = false;
							}
						}
						else
						{
							// on fait la verif des pages ailleurs
							fPagesInMap->Set(mastermultiplicateur + i, true);
						}
					}
				}
			}
		}
		else
		{
			fPagesMapIsValid = false;
		}
	}
	else
	{
		fPagesMapIsValid = false;
	}

	if (errexec != VE_OK)
		fPagesMapHasBeenChecked = false;

	return errexec;
}


VError Index_NonOpened::CheckTabAddr(ToolLog* log)
{
	VError errexec = VE_OK;

	if (fHeaderIsValid)
	{
		if (fPagesInMap == nil)
			fPagesInMap = new Bittab();

		fTempCachePageAddr = new TabAddrCache(fOwner, IHD.addrprim, IHD.nbpage);
		VString s;
		log->GetVerifyOrCompactString(20,s);	// Checking Index Pages Addresses on {p1}
		VString s2;
		GetName(s2);
		FormatStringWithParamsStrings(s, &s2);
		errexec = log->OpenProgressSession(s, IHD.nbpage);
		if (errexec == VE_OK)
		{
			fPagesMapIsValid = true;
			fPagesMapHasBeenChecked = true;
			if (IHD.addrprim != 0 && IHD.addrprim != -1)
			{
				sLONG nbmax = IHD.nbpage;
				/*
				if (nbmax == kNbElemTabAddr || nbmax == kNbElemTabAddr2)
					nbmax++;
					*/
				errexec = CheckPagesAddrs(IHD.addrprim, nbmax, IHD.nbpage, 0, -1, log, 0);
			}
			log->CloseProgressSession();
		}

		if (errexec == VE_OK)
		{
			if (fTabTrouIsValid)
			{
				if (IHD.addrprim != 0 && IHD.nextfreepage != kFinDesTrou)
				{
					VString s;
					log->GetVerifyOrCompactString(21,s);	// Checking List of deleted Index Pages on {p1}
					VString s2;
					GetName(s2);
					FormatStringWithParamsStrings(s, &s2);
					errexec = log->OpenProgressSession(s, IHD.nbpage);
					if (errexec == VE_OK)
					{
						errexec = CheckPagesTrous(log);
						log->CloseProgressSession();
					}
				}
			}
		}

		/*
		if (fTempCachePageAddr != nil)
			delete fTempCachePageAddr;
		fTempCachePageAddr = nil;
		*/
	}

	return errexec;
}


VError Index_NonOpened::CheckOneCluster(DataAddr4D ou, sLONG len, sLONG numclust, ToolLog* log)
{
	VError errexec = VE_OK;

	OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
	Base4D* target = log->GetTargetCompact();
	if (fOwner->GetStruct() == nil && target != nil)
		target = target->GetStructure();

	if (fOwner->IsAddrValid(ou, false))
	{
		log->MarkAddr_IndexCluster(ou, len + kSizeDataBaseObjectHeader, fNum, numclust);
		sLONG typsel;
		DataBaseObjectHeader tag;
		VError err = tag.ReadFrom(fOwner, ou);
		if (err != VE_OK)
		{
			IndexClusterProblem pb(TOP_PhysicalDataIsInvalid, fNum, numclust);
			errexec = log->Add(pb);
		}
		if (err == VE_OK && errexec == VE_OK)
		{
			err = tag.ValidateTag(DBOH_SetDiskTable, -1, -1);
			if (err != VE_OK)
				err = tag.ValidateTag(DBOH_PetiteSel, -1, -1);
			if (err != VE_OK)
			{
				IndexClusterProblem pb(TOP_TagIsInvalid, fNum, numclust);
				errexec = log->Add(pb);
			}
		}

		if (err == VE_OK && errexec == VE_OK)
		{
			err = fOwner->readlong(&typsel, 4, ou, kSizeDataBaseObjectHeader);
			if (err != VE_OK)
			{
				IndexClusterProblem pb(TOP_PhysicalDataIsInvalid, fNum, numclust);
				errexec = log->Add(pb);
			}
		}

		if (err == VE_OK && errexec == VE_OK)
		{
			if (tag.NeedSwap())
				typsel = SwapLong(typsel);

			switch (typsel)
			{
				case sel_bitsel:
					{
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
						
						sLONG nb, decompte, nbpage;
						err = fOwner->readlong(&nb, 4, ou, kSizeDataBaseObjectHeader + 4);
						if (err != VE_OK)
						{
							IndexClusterProblem pb(TOP_PhysicalDataIsInvalid, fNum, numclust);
							errexec = log->Add(pb);
						}
						if (err == VE_OK && errexec == VE_OK)
						{
							if (tag.NeedSwap())
								nb = SwapLong(nb);
							err = fOwner->readlong(&decompte, 4, ou, kSizeDataBaseObjectHeader + 8);
							if (err != VE_OK)
							{
								IndexClusterProblem pb(TOP_PhysicalDataIsInvalid, fNum, numclust);
								errexec = log->Add(pb);
							}
							else
							{
								if (tag.NeedSwap())
									decompte = SwapLong(decompte);
							}
						}
						if (err == VE_OK && errexec == VE_OK)
						{
							if (nb >=0 && nb<=(maxrecnum+2048))
							{
								nbpage = (nb+kNbBitParSet-1)>>kRatioSet;
								DataAddr4D* TabPage = (DataAddr4D*) GetFastMem(nbpage*sizeof(DataAddr4D), false, 'BSEL');
								if (TabPage == nil)
									errexec = memfull;
								else
								{
									err = fOwner->readlong( TabPage, nbpage<<3, ou, kSizeDataBaseObjectHeader + 12);
									if (err != VE_OK)
									{
										IndexClusterProblem pb(TOP_PhysicalDataIsInvalid, fNum, numclust);
										errexec = log->Add(pb);
									}
									else
									{
										err = tag.ValidateCheckSum(TabPage, nbpage<<3);
										if (err != VE_OK)
										{
											IndexClusterProblem pb(TOP_CheckSumIsInvalid, fNum, numclust);
											errexec = log->Add(pb);
										}
									}

									if (err == VE_OK && errexec == VE_OK)
									{
										if (tag.NeedSwap())
											ByteSwap(TabPage, nbpage);
										Boolean dejadeleteproblem = false;
										Boolean dejaendoubleproblem = false;
										if (fType == DB4D_Index_OnKeyWords)
											dejaendoubleproblem = true;

										DataAddr4D* cur = TabPage;

										for (sLONG i=0; i<nbpage && errexec == VE_OK; i++, cur++)
										{
											DataAddr4D addr = *cur;
											if (addr == 0)
											{
												// rien a faire, tous les bit sont a zero
											}
											else
											{
												if (addr == 1)
												{
													// tous les bits sont a 1
													if (!log->IsCompacting())
													{
														VError err2;
														if (DeletedRecords != nil)
														{
															if (DeletedRecords->Intersect(true, i, err2) && !dejadeleteproblem)
															{
																IndexClusterPageProblem pb(TOP_Cluster_DeletedRecordNumber, fNum, numclust, i);
																errexec = log->Add(pb);
																dejadeleteproblem = true;
															}
														}
														if (fRecordsInKeys != nil && errexec == VE_OK)
														{
															if (fRecordsInKeys->Intersect(true, i, err2) && !dejaendoubleproblem)
															{
																IndexClusterPageProblem pb(TOP_Cluster_DuplicatedRecordNumber, fNum, numclust, i);
																errexec = log->Add(pb);
																dejaendoubleproblem = true;
															}
															if (errexec == VE_OK)
																errexec = fRecordsInKeys->SetRange(true, i);
														}
													}
												}
												else
												{
													if (addr > 0)
													{
														*cur = 0;

														if (fOwner->IsAddrValid(addr, false))
														{
															log->MarkAddr_IndexCluster_part(addr, kSizeSetDisk + kSizeDataBaseObjectHeader, fNum, numclust, i);
															uLONG bits[kNbLongParSet+1];
															bitsetptr p = &bits[0];

															DataBaseObjectHeader tag;
															err = tag.ReadFrom(fOwner, addr);
															if (err != VE_OK)
															{
																IndexClusterPageProblem pb(TOP_PhysicalDataIsInvalid, fNum, numclust, i);
																errexec = log->Add(pb);
															}
															if (err == VE_OK && errexec == VE_OK)
															{
																err = tag.ValidateTag(DBOH_SetDisk, i, -1);
																if (err != VE_OK)
																{
																	IndexClusterPageProblem pb(TOP_TagIsInvalid, fNum, numclust, i);
																	errexec = log->Add(pb);
																}
															}
															if (err == VE_OK && errexec == VE_OK)
															{
																err = fOwner->readlong(p, kSizeSetDisk, addr, kSizeDataBaseObjectHeader);
																if (err != VE_OK)
																{
																	IndexClusterPageProblem pb(TOP_PhysicalDataIsInvalid, fNum, numclust, i);
																	errexec = log->Add(pb);
																}
															}
															if (err == VE_OK && errexec == VE_OK)
															{
																err = tag.ValidateCheckSum(p, kSizeSetDisk);
																if (err == VE_OK)
																{
																	if (tag.NeedSwap())
																		ByteSwap(p, kNbLongParSet);
																	if (log->IsCompacting())
																	{
																		DataAddr4D ou = target->findplace(kSizeSetDisk+kSizeDataBaseObjectHeader, nil, err, -kIndexSegNum, nil);
																		if (ou != -1)
																		{
																			*cur = ou;
																			tag.SetSwapWhileWriting(tag.NeedSwap());
																			tag.WriteInto(target, ou, nil, -1);
																			target->writelong(p, kSizeSetDisk, ou, kSizeDataBaseObjectHeader, nil, -1);
																		}
																	}
																	else
																	{
																		VError err2;
																		if (DeletedRecords != nil)
																		{
																			vector<sLONG> collisions;
																			sLONG maxcollisions = 100;
																			if (DeletedRecords->Intersect(p, i, err2, collisions, maxcollisions) && !dejadeleteproblem)
																			{
																				IndexClusterPageProblem pb(TOP_Cluster_DeletedRecordNumber, fNum, numclust, i);
																				errexec = log->Add(pb);
																				dejadeleteproblem = true;
#if debuglr
																				if (!collisions.empty())
																				{
																					for (vector<sLONG>::iterator cur = collisions.begin(), end = collisions.end(); cur != end; cur++)
																					{
																						sLONG recnum = (i*kNbBitParSet) + *cur;
																						DebugMsg("referenced deleted record number : "+ToString(recnum)+"\n");
																					}
																				}
#endif
																			}
																		}
																		if (fRecordsInKeys != nil && errexec == VE_OK)
																		{
																			vector<sLONG> collisions;
																			sLONG maxcollisions = 100;
																			if (fRecordsInKeys->Intersect(p, i, err2, collisions, maxcollisions) && !dejaendoubleproblem)
																			{
																				IndexClusterPageProblem pb(TOP_Cluster_DuplicatedRecordNumber, fNum, numclust, i);
																				errexec = log->Add(pb);
																				dejaendoubleproblem = true;
#if debuglr
																				if (!collisions.empty())
																				{
																					for (vector<sLONG>::iterator cur = collisions.begin(), end = collisions.end(); cur != end; cur++)
																					{
																						sLONG recnum = (i*kNbBitParSet) + *cur;
																						DebugMsg("duplicate referenced record number : "+ToString(recnum)+"\n");
																					}
																				}
#endif
																			}
																			if (errexec == VE_OK)
																				errexec = fRecordsInKeys->SetRange(p, i);
																		}
																	}
																}
																else
																{
																	IndexClusterPageProblem pb(TOP_CheckSumIsInvalid, fNum, numclust, i);
																	errexec = log->Add(pb);
																}
															}

														}
														else
														{
															IndexClusterPageProblem pb(TOP_AddrIsInvalid, fNum, numclust, i);
															errexec = log->Add(pb);
														}
													}
													else
													{
														IndexClusterPageProblem pb(TOP_AddrIsInvalid, fNum, numclust, i);
														errexec = log->Add(pb);
													}
												}
											}

										}
									}

									if (errexec == VE_OK && log->IsCompacting())
									{
										sLONG lensel = (nbpage<<3)+12;

										DataAddr4D SelAddr = target->findplace(lensel + kSizeDataBaseObjectHeader, nil, err, -kIndexSegNum, nil);
										if (SelAddr != -1)
										{
											sLONG lll=sel_bitsel;
											DataBaseObjectHeader tag2(TabPage, nbpage<<3, DBOH_SetDiskTable, -1, -1);
											err = tag2.WriteInto(target, SelAddr, nil, -1);
											if (err == VE_OK) 
												err = target->writelong(&lll,4,SelAddr,kSizeDataBaseObjectHeader, nil, -1);
											if (err == VE_OK) 
												err = target->writelong(&nb,4,SelAddr,kSizeDataBaseObjectHeader+4, nil, -1);
											if (err == VE_OK) 
												err = target->writelong(&decompte,4,SelAddr,kSizeDataBaseObjectHeader+8, nil, -1);
											if (err == VE_OK) 
												err = target->writelong( TabPage, nbpage<<3, SelAddr, kSizeDataBaseObjectHeader+12, nil, -1);
											log->GetTargetIndexCompact()->GetHeader()->SetClusterAddr(curstack, numclust, SelAddr, len);
										}
									}
									
									FreeFastMem(TabPage);
								}
							}
							else
							{
								IndexClusterProblem pb(TOP_Cluster_BitCountIsInvalid, fNum, numclust);
								errexec = log->Add(pb);
							}
						}

					}
					break;

				case sel_petitesel:
					{
						sLONG nbrecs;
						err = fOwner->readlong(&nbrecs, 4, ou, kSizeDataBaseObjectHeader + 4);
						if (err != VE_OK)
						{
							IndexClusterProblem pb(TOP_PhysicalDataIsInvalid, fNum, numclust);
							errexec = log->Add(pb);
						}
						if (err == VE_OK && errexec == VE_OK)
						{
							if (tag.NeedSwap())
								nbrecs = SwapLong(nbrecs);

							if (nbrecs <= 1)
							{
								IndexClusterProblem pb(TOP_PhysicalDataIsInvalid, fNum, numclust);
								errexec = log->Add(pb);
							}
							else
							{
								const sLONG MaxSizeClusterLong = 4096;
								sLONG xtabmem[MaxSizeClusterLong+1];
								sLONG* ptabmem = &xtabmem[0];
								if (nbrecs > MaxSizeClusterLong)
									ptabmem = (sLONG*)VObject::GetMainMemMgr()->Malloc((nbrecs+1)*4, false, 'pclu');
								if (ptabmem == nil)
									errexec = memfull;
								else
								{
									err=fOwner->readlong( ptabmem, nbrecs*4, ou, 8+kSizeDataBaseObjectHeader);
									if (err != VE_OK)
									{
										IndexClusterProblem pb(TOP_PhysicalDataIsInvalid, fNum, numclust);
										errexec = log->Add(pb);
									}

									if (err == VE_OK && errexec == VE_OK)
									{
										err = tag.ValidateCheckSum(ptabmem, nbrecs * 4);
										if (err != VE_OK)
										{
											IndexClusterProblem pb(TOP_CheckSumIsInvalid, fNum, numclust);
											errexec = log->Add(pb);
										}
									}

									if (err == VE_OK && errexec == VE_OK)
									{
										if (tag.NeedSwap())
											ByteSwap(ptabmem, nbrecs);

										if (log->IsCompacting())
										{
											sLONG lensel = nbrecs<<2;
											DataAddr4D SelAddr = target->findplace(lensel + 8 + kSizeDataBaseObjectHeader, nil, err, -kIndexSegNum, nil);
											if (SelAddr != -1)
											{
												DataBaseObjectHeader tag2(ptabmem, lensel, DBOH_PetiteSel, -1, -1 /*GetParentFile()->GetNum()*/);
												tag2.WriteInto(target, SelAddr, nil, -1);
												target->writelong(&typsel,4,SelAddr,kSizeDataBaseObjectHeader, nil, -1);
												target->writelong(&nbrecs,4,SelAddr,kSizeDataBaseObjectHeader+4, nil, -1);
												target->writelong(ptabmem,lensel,SelAddr,kSizeDataBaseObjectHeader+8, nil, -1);
												log->GetTargetIndexCompact()->GetHeader()->SetClusterAddr(curstack, numclust, SelAddr, len);
											}

										}
										else
										{
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

											Boolean dejadeleteproblem =false;
											Boolean dejainvalidrecordproblem =false;
											for (sLONG i=0; i<nbrecs && errexec == VE_OK; i++)
											{
												sLONG n = ptabmem[i];
												if (n<0 || n>maxrecnum)
												{
													if (!dejainvalidrecordproblem)
													{
														IndexClusterProblem pb(TOP_Cluster_InvalidRecordNumber, fNum, numclust);
														errexec = log->Add(pb);
														dejainvalidrecordproblem = true;
													}
												}
												else
												{
													if (DeletedRecords != nil)
													{
														if (DeletedRecords->isOn(n) && !dejadeleteproblem)
														{
															IndexClusterProblem pb(TOP_Cluster_DeletedRecordNumber, fNum, numclust);
															errexec = log->Add(pb);
															dejadeleteproblem = true;
														}
													}
													if (fRecordsInKeys != nil && errexec == VE_OK)
													{
														errexec = fRecordsInKeys->Set(n, true);
													}
												}
											}
										}
									}
									if (ptabmem != &xtabmem[0])
										VObject::GetMainMemMgr()->Free(ptabmem);
								}
							}

						}
					}
					break;

				default:
					{
						IndexClusterProblem pb(TOP_Cluster_TypeOfSelectionIsInvalid, fNum, numclust);
						errexec = log->Add(pb);
					}
					break;

			}
		}
	}
	else
	{
		IndexClusterProblem pb(TOP_AddrIsInvalid, fNum, numclust);
		errexec = log->Add(pb);
	}
	return errexec;
}


VError Index_NonOpened::CheckClusters(DataAddr4D ou, sLONG nbclustmax, sLONG nbclusttocheck, sLONG pos1, sLONG pos2, ToolLog* log, sLONG mastermultiplicateur)
{
	VError errexec = VE_OK;
	VError err = VE_OK;

	TabAddrDisk taddr;
	log->MarkAddr_TabAddr_IndexCluster(ou, kSizeTabAddr+kSizeDataBaseObjectHeader, fNum, -1, -1);
	errexec = fOwner->ReadAddressTable(ou, taddr, pos1, pos2, log, this, err, 1, false);
	if (errexec == VE_OK)
	{
		if (err == VE_OK)
		{
			if (nbclustmax > kNbElemTabAddr)
			{
				sLONG diviseur = kNbElemTabAddr;
				if (nbclustmax > kNbElemTabAddr2)
					diviseur = kNbElemTabAddr2;

				sLONG nbparents = (nbclusttocheck+diviseur-1) / diviseur;
				for (sLONG i=0; i<nbparents && errexec == VE_OK; i++)
				{
					sLONG nbremains = diviseur;
					if (i == (nbparents-1))
						nbremains = nbclusttocheck & (diviseur-1);
					if (nbremains == 0)
						nbremains = diviseur;
					if (taddr[i].len != kSizeTabAddr)
					{
						Index_ClusterAddrProblem pb(TOP_LenghOfAddressTableIsInvalid, fNum, mastermultiplicateur + i);
						errexec = log->Add(pb);
						fClustMapIsValid = false;
					}
					if (errexec == VE_OK)
						errexec = CheckClusters(taddr[i].addr, diviseur - 1, nbremains, i, -1, log, mastermultiplicateur + i * diviseur);
				}
			}
			else
			{
				for (sLONG i=0; i<nbclusttocheck && errexec == VE_OK; i++)
				{
					errexec = log->Progress(mastermultiplicateur + i);
					if (errexec == VE_OK)
					{
						DataAddr4D addr = taddr[i].addr;
						if (fTempCacheClusterAddr != nil)
							fTempCacheClusterAddr->AddAddr(mastermultiplicateur + i, addr, taddr[i].len);
						if (addr <= 0)
						{
							DataAddr4D addr2 = -addr;
							if (addr2 < IHCLUST.nbclust || addr == kFinDesTrou)
							{
								// on sait que le cluster est detruit
							}
							else
							{
								IndexClusterProblem pb(TOP_AddrIsInvalid, fNum, mastermultiplicateur + i);
								errexec = log->Add(pb);
								fClustMapIsValid = false;
							}
						}
						else
						{
							errexec = CheckOneCluster(addr, taddr[i].len, mastermultiplicateur + i, log);
							fClustInMap->Set(mastermultiplicateur + i, true);
						}
					}
				}
			}
		}
		else
		{
			fClustMapIsValid = false;
		}
	}
	else
	{
		fClustMapIsValid = false;
	}

	if (errexec != VE_OK)
		fClustMapHasBeenChecked = false;

	return errexec;
}


VError Index_NonOpened::CheckClusterTrous(ToolLog* log)
{
	VError errexec = VE_OK;

	fTabTrouClustHasBeenChecked = true;

	if (fTabTrouClustIsValid)
	{
		if (fClustMapIsValid && fHeaderIsValid)
		{
			sLONG nbSets = 0;
			Bittab TempDeleted;
			{
				DataAddr4D n = IHCLUST.debuttrou;
				Boolean ChaineIsValid = true;
				Boolean ReferenceCirculaire = false;
				Boolean WrongReference = false;

				while (n != kFinDesTrou && ChaineIsValid && errexec == VE_OK)
				{
					DataAddr4D n2 = -n;
					if (n2>=0 && n2<IHCLUST.nbclust)
					{
						if (TempDeleted.isOn(n2))
						{
							ChaineIsValid = false;
							ReferenceCirculaire = true;
						}
						else
						{
							errexec = TempDeleted.Set(n2, true);
							nbSets++;
							if (nbSets>1000)
							{
								TempDeleted.Epure();
								nbSets = 0;
							}
							if (errexec == VE_OK)
							{
								VError err = VE_OK;
								n = fOwner->GetAddrFromTable(fTempCacheClusterAddr, (sLONG)n2, IHCLUST.addrTabClust, IHCLUST.nbclust, err);
								if (err != VE_OK)
								{
									ChaineIsValid = false;
									WrongReference = true;
								}
							}
						}
					}
					else
					{
						WrongReference = true;
						ChaineIsValid = false;
					}
				}

				if (errexec == VE_OK)
				{
					if (!ChaineIsValid)
					{
						fTabTrouClustIsValid = false;
						if (WrongReference)
						{
							IndexProblem pb(TOP_Index_TabTrouClusterIsInvalid, fNum);
							errexec = log->Add(pb);
						}
						else
						{
							IndexProblem pb(TOP_Index_TabTrouClusterIsInvalid, fNum);
							errexec = log->Add(pb);
						}
					}
				}

			}
		}
		else
			fTabTrouClustIsValid = false;
	}


	if (errexec != VE_OK)
		fTabTrouClustHasBeenChecked = false;

	return errexec;
}


VError Index_NonOpened::CheckAllClusters(ToolLog* log)
{
	VError errexec = VE_OK;

	if (fHeaderIsValid)
	{
		if (fClustInMap == nil)
			fClustInMap = new Bittab();

		fTempCacheClusterAddr = new TabAddrCache(fOwner, IHCLUST.addrTabClust, IHCLUST.nbclust);
		VString s;
		log->GetVerifyOrCompactString(22,s);	// Checking Clusters on {p1}
		VString s2;
		GetName(s2);
		FormatStringWithParamsStrings(s, &s2);
		errexec = log->OpenProgressSession(s, IHCLUST.nbclust);
		if (errexec == VE_OK)
		{
			fClustMapIsValid = true;
			fClustMapHasBeenChecked = true;
			if (IHCLUST.addrTabClust != 0 && IHCLUST.addrTabClust != -1)
			{
				sLONG nbmax = IHCLUST.nbclust;
				/*
				if (nbmax == kNbElemTabAddr || nbmax == kNbElemTabAddr2)
					nbmax++;
					*/
				errexec = CheckClusters(IHCLUST.addrTabClust, nbmax, IHCLUST.nbclust, 0, -1, log, 0);
			}
			log->CloseProgressSession();
		}

		if (errexec == VE_OK && !log->IsCompacting())
		{
			if (fTabTrouClustIsValid)
			{
				VString s;
				log->GetVerifyOrCompactString(23,s);	// Checking List of deleted Clusters on {p1}
				VString s2;
				GetName(s2);
				FormatStringWithParamsStrings(s, &s2);
				errexec = log->OpenProgressSession(s, IHCLUST.nbclust);
				if (IHCLUST.addrTabClust != 0 && IHCLUST.debuttrou != kFinDesTrou && errexec == VE_OK)
					errexec = CheckClusterTrous(log);
				log->CloseProgressSession();
			}
		}

		if (fTempCacheClusterAddr != nil)
			delete fTempCacheClusterAddr;
		fTempCacheClusterAddr = nil;
	}

	return errexec;
}


void Index_NonOpened::CleanTempData()
{
	if (fTempCachePageAddr != nil)
		delete fTempCachePageAddr;
	fTempCachePageAddr = nil;

	if (fTempCacheClusterAddr != nil)
		delete fTempCacheClusterAddr;
	fTempCacheClusterAddr = nil;

	ReleaseRefCountable(&fPagesInMap);
	ReleaseRefCountable(&fClustInMap);
	ReleaseRefCountable(&fRecordsInKeys);
	ReleaseRefCountable(&fDejaPages);

	fClustInMap = nil;
}


VError Index_NonOpened::CheckAll(ToolLog* log)
{
	VString s;
	log->GetVerifyOrCompactString(34,s);	// Checking Index on {p1}
	VString s2;
	GetName(s2);
	FormatStringWithParamsStrings(s, &s2);
	VError errexec = log->OpenProgressSession(s, 0);

	if (errexec == VE_OK)
	{
		if (IHD.needRebuild == 1)
		{
			IndexProblem pb(TOP_Index_flagged_for_rebuilding, fNum);
			pb.SetErrorLevel(TO_ErrorLevel_Warning);
			errexec = log->Add(pb);
		}
		else
		{
			errexec = CheckTabAddr(log);

			DataFile_NotOpened* df = fOwner->GetDataTableWithTableDefNum(fTable);
			if (df != nil && !log->IsCompacting())
			{
				if (df->RecordsHaveBeenChecked() && df->NbFicIsValid() && df->TabRecAddrIsValid())
				{
					fRecordsInKeys = new Bittab;
					if (fRecordsInKeys != nil)
					{
						errexec = fRecordsInKeys->aggrandit(df->GetMaxRecords());
						if (errexec != VE_OK)
						{
							ReleaseRefCountable(&fRecordsInKeys);
						}
					}
				}
			}

			if (fIndexHeaderType == DB4D_Index_BtreeWithCluster)
			{
				if (errexec == VE_OK)
					errexec = CheckAllClusters(log);
			}

			if (errexec == VE_OK)
			{
				if (IHD.AddrNULLs != -1 && IHD.AddrNULLs != 0)
					errexec = CheckOneCluster(IHD.AddrNULLs, IHD.lenNULLs, kSpecialClustNulls, log);
			}

			if (df != nil)
			{
				if (errexec == VE_OK)
				{
					if (OpenCheckKeysProgressSession( log, &errexec))
					{
						switch (fType)
						{
							case DB4D_Index_OnOneField_Scalar_Word:
								errexec = CheckScalarKeys<sWORD, kNbKeysForScalar>(log);
								break;

							case DB4D_Index_OnOneField_Scalar_Byte:
								errexec = CheckScalarKeys<sBYTE, kNbKeysForScalar>(log);
								break;

							case DB4D_Index_OnOneField_Scalar_Long:
								errexec = CheckScalarKeys<sLONG, kNbKeysForScalar>(log);
								break;

							case DB4D_Index_OnOneField_Scalar_Long8:
								errexec = CheckScalarKeys<sLONG8, kNbKeysForScalar>(log);
								break;

							case DB4D_Index_OnOneField_Scalar_Real:
								errexec = CheckScalarKeys<Real, kNbKeysForScalar>(log);
								break;
							
							case DB4D_Index_OnOneField_Scalar_Bool:
								errexec = CheckScalarKeys<uBYTE, kNbKeysForScalar>(log);
								break;

							case DB4D_Index_OnOneField_Scalar_Time:
								errexec = CheckScalarKeys<uLONG8, kNbKeysForScalar>(log);
								break;

							case DB4D_Index_OnOneField_Scalar_UUID:
								errexec = CheckScalarKeys<VUUIDBuffer, kNbKeysForScalar>(log);
								break;
								
							default:
								errexec = CheckKeys(log);
								break;
						}

						if (errexec == VE_OK && !log->IsCompacting())
						{
							if (fRecordsInKeys != nil && fType != DB4D_Index_OnKeyWords)
							{
								if (df->NbFicIsValid())
								{
									Bittab allrecs;
									errexec = allrecs.aggrandit(df->GetMaxRecords());
									if (errexec == VE_OK)
									{
										allrecs.ClearOrSetAll(true);
										Bittab* delrecs = df->RecordsDeleted();
										if (delrecs != nil)
											errexec = allrecs.moins(delrecs);
										if (errexec == VE_OK)
										{
											errexec = allrecs.moins(fRecordsInKeys);
											if (errexec == VE_OK)
											{
												if (allrecs.Compte() != 0)
												{
													IndexProblem pb(TOP_Index_SomeRecordsAreNotReferenced, fNum);
													errexec = log->Add(pb);
													sLONG numrec = 0;
													numrec = allrecs.FindNextBit(numrec);
													VString s;
													s.FromLong(numrec);
													s += L"\n";
													DebugMsg(s);
												}
											}
										}
									}
								}
							}
						}
						log->CloseProgressSession();
					}
				}
			}
		}

		CleanTempData();
		log->CloseProgressSession();
	}

	return errexec;
}


void Index_NonOpened::GetName(VString& outName) const
{
	if (fType == DB4D_Index_OnKeyWords)
	{
		gs(1004,25,outName);	// Full Text Index#
	}
	else
	{
		switch(fIndexHeaderType)
		{
			case DB4D_Index_Btree:
				gs(1004,26,outName);	// Btree Index#
				break;

			case DB4D_Index_BtreeWithCluster:
				gs(1004,27,outName);	// Cluster Index#
				break;

			default:
				gs(1004,28,outName);	// Index#
				break;
		}
	}

	VString snum;
	snum.FromLong(fNum);
	outName += snum;
	outName += L" ";
	VString liaison;
	gs(1004,24,liaison);	// on
	outName += liaison;	
	outName += L" ";

	if (fSourceValid)
	{
		for (vector<sLONG>::const_iterator cur = fFields.begin(), end = fFields.end(); cur != end; cur ++)
		{
			if (cur != fFields.begin())
			{
				outName += L" , ";
			}
			VString s;
			fOwner->ExistField(fTable, *cur, &s);
			outName += s;
		}
	}
	else
	{
		outName+= "[n/a]";
	}
	if (!fName.IsEmpty())
	{
		outName += L" (";
		outName += fName;
		outName += L")";
	}
}


Boolean Index_NonOpened::MatchField(sLONG fieldnum)
{
	for (vector<sLONG>::iterator cur = fFields.begin(), end= fFields.end(); cur != end; cur++)
	{
		if (*cur == fieldnum)
			return true;
	}
	return false;
}


				// ================================================================ //

#if debugIndexOverlap_strong

di_IndexOverLap::di_mapOverLap di_IndexOverLap::sMapOverLaps;
VCriticalSection di_IndexOverLap::sdi_mutex;



void di_IndexOverLap::stopInMap()
{
	assert(false); // break here
}



void di_IndexOverLap::add(di_IndexOverLap& di)
{
	VTaskLock lock(&sdi_mutex);
	di_mapOverLap::iterator closestnext = sMapOverLaps.lower_bound(di.fAddr);
	if (closestnext != sMapOverLaps.end())
	{
		if (di.fAddr <= closestnext->first && (di.fAddr+di.fLen) > closestnext->first)
		{
			stopInMap(); // nous sommes en train d'ecrire sur une autre page que la bonne
		}
		if (closestnext != sMapOverLaps.begin())
		{
			di_mapOverLap::iterator closestprev = closestnext;
			--closestprev;
			di_IndexOverLap& prev = closestprev->second;
			if (di.fAddr < (prev.fAddr + prev.fLen))
			{
				stopInMap(); // nous sommes en train d'ecrire sur une autre page que la bonne
			}
		}
	}
	
	di_IndexOverLap* dip = &(sMapOverLaps[di.fAddr]);
	if (dip->fType != di_none)
	{
		stopInMap();
	}

	*dip = di;

}


void di_IndexOverLap::remove(di_IndexOverLap& di)
{
	VTaskLock lock(&sdi_mutex);
	di_mapOverLap::iterator found = sMapOverLaps.find(di.fAddr);
	if (found != sMapOverLaps.end())
	{
		bool ok = true;
		di_IndexOverLap* dip = &(found->second);
		if (di.fType != dip->fType)
		{
			stopInMap();
			ok = false;
		}

		if (di.fInd != nil && dip->fInd!= nil && di.fInd != dip->fInd)
		{
			stopInMap();
			ok = false;
		}

		if (di.fCluster != nil && dip->fCluster!= nil && di.fCluster != dip->fCluster)
		{
			stopInMap();
			ok = false;
		}

		if (di.fPageNum != -1 && dip->fPageNum!= -1 && di.fPageNum != dip->fPageNum)
		{
			stopInMap();
			ok = false;
		}

		if (di.fClusterNum != -1 && dip->fClusterNum!= -1 && di.fClusterNum != dip->fClusterNum)
		{
			stopInMap();
			ok = false;
		}

		if (di.fClusterPartNum != -1 && dip->fClusterPartNum!= -1 && di.fClusterPartNum != dip->fClusterPartNum)
		{
			stopInMap();
			ok = false;
		}

		di_mapOverLap::iterator next = found;
		++next;
		if (next != sMapOverLaps.end())
		{
			if (di.fAddr+di.fLen > next->first)
			{
				stopInMap();
			}
		}
		
		if (ok)
			sMapOverLaps.erase(found);
	}

}


void di_IndexOverLap::AddIndexPage(IndexInfo* ind, sLONG pagenum, DataAddr4D ou, sLONG len)
{
	di_IndexOverLap di;
	di.fType = di_pageIndex;
	di.fInd = ind;
	di.fPageNum = pagenum;
	di.fAddr = ou;
	di.fLen = len;
	add(di);
}


void di_IndexOverLap::AddCluster(ClusterSel* cluster, sLONG clusternum, DataAddr4D ou, sLONG len)
{
	di_IndexOverLap di;
	di.fType = di_cluster;
	di.fCluster = cluster;
	di.fClusterNum = clusternum;
	di.fAddr = ou;
	di.fLen = len;
	add(di);
}


void di_IndexOverLap::AddClusterPart(ClusterSel* cluster, sLONG clusternum, sLONG ClusterPartNum, DataAddr4D ou, sLONG len)
{
	di_IndexOverLap di;
	di.fType = di_clusterpart;
	di.fCluster = cluster;
	di.fClusterPartNum = ClusterPartNum;
	di.fClusterNum = clusternum;
	di.fAddr = ou;
	di.fLen = len;
	add(di);
}


void di_IndexOverLap::AddNullsCluster(IndexInfo* ind, DataAddr4D ou, sLONG len)
{
	di_IndexOverLap di;
	di.fType = di_nulls;
	di.fInd = ind;
	di.fAddr = ou;
	di.fLen = len;
	add(di);
}



void di_IndexOverLap::RemoveIndexPage(IndexInfo* ind, sLONG pagenum, DataAddr4D ou, sLONG len)
{
	di_IndexOverLap di;
	di.fType = di_pageIndex;
	di.fInd = ind;
	di.fPageNum = pagenum;
	di.fAddr = ou;
	di.fLen = len;
	remove(di);
}


void di_IndexOverLap::RemoveCluster(ClusterSel* cluster, sLONG clusternum, DataAddr4D ou, sLONG len)
{
	di_IndexOverLap di;
	di.fType = di_cluster;
	di.fCluster = cluster;
	di.fClusterNum = clusternum;
	di.fAddr = ou;
	di.fLen = len;
	remove(di);
}


void di_IndexOverLap::RemoveClusterPart(ClusterSel* cluster, sLONG clusternum, sLONG ClusterPartNum, DataAddr4D ou, sLONG len)
{
	di_IndexOverLap di;
	di.fType = di_clusterpart;
	di.fCluster = cluster;
	di.fClusterPartNum = ClusterPartNum;
	di.fClusterNum = clusternum;
	di.fAddr = ou;
	di.fLen = len;
	remove(di);
}


void di_IndexOverLap::RemoveNullsCluster(IndexInfo* ind, DataAddr4D ou, sLONG len)
{
	di_IndexOverLap di;
	di.fType = di_nulls;
	di.fInd = ind;
	di.fAddr = ou;
	di.fLen = len;
	remove(di);
}

#endif














