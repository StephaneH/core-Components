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




void ClusterSelTreeMemHeader::KillElem(void* inObject)
{
	((Selection*)inObject)->setmodif(false, /*((Selection*)inObject)->GetParentFile()->GetDB()*/((Selection*)inObject)->GetDB(), nil);
	((Selection*)inObject)->Release();
}


bool ClusterSelTreeMemHeader::TryToFreeElem(sLONG allocationBlockNumber, void* inObject, VSize& outFreed)
{
	Selection* sel = (Selection*)inObject;
	if (!sel->IsOccupied() && !sel->modifie() && sel->MatchAllocationNumber(allocationBlockNumber))
	{
		outFreed = sel->CalcLenOnDisk();
#if debuglr
		if (sel->GetRefCount() != 1)
		{
			xbox_assert(sel->GetRefCount() == 1);
		}
#endif
		sel->Release();
		return true;
	}
	else
		return false;
}


void ClusterSelTreeMemHeader::RetainElem(void* inObject)
{
	((Selection*)inObject)->Retain();
}



void ClusterSelTreeMemHeader::OccupyElem(void* inObject, OccupableStack* curstack)
{
	((Selection*)inObject)->Occupy(curstack, true);
}



						// --------------------------------------------------------------------




ClusterSel::ClusterSel(void)
{
	IHCLUST.addrTabClust=0;
	IHCLUST.debuttrou=kFinDesTrou;
	IHCLUST.nbclust=0;
	//IHCLUST_SavedByTrans.nbclust = -1;
}


VError ClusterSel::ThrowError( VError inErrCode, ActionDB4D inAction)
{
	if (bd != nil)
		return bd->ThrowError(inErrCode, inAction);
	else
		return inErrCode;
}

/*
VError ClusterSel::Reloader(AddrTableHeader* obj, ObjAlmostInCache* Owner)
{
	VError err = VE_OK;
	ClusterSel* CS = (ClusterSel*)Owner;
	
	if (obj == nil)
	{
		CS->IHCLUST_SavedByTrans.nbclust = -1;
	}
	else
	{
		xbox_assert(CS->IHCLUST_SavedByTrans.nbclust != -1);
		obj->SetValues(CS->IHCLUST_SavedByTrans.addrTabClust, CS->IHCLUST_SavedByTrans.debuttrou, CS->IHCLUST_SavedByTrans.nbclust);
	}
	
	return(err);
}


VError ClusterSel::ReloaderForKill(ObjAlmostInCache* Owner)
{
	VError err = VE_OK;
	ClusterSel* CS = (ClusterSel*)Owner;
	
	xbox_assert(CS->IHCLUST_SavedByTrans.nbclust != -1); // cet xbox_assert doit disparaitre car la condition peut etre valide
	
	if (CS->IHCLUST_SavedByTrans.nbclust != -1)
	{
		CS->IHCLUST = CS->IHCLUST_SavedByTrans;
	}
	
	if (CS->IHCLUST_SavedByTrans.nbclust != -1)
	{
		CS->IHCLUST = CS->IHCLUST_SavedByTrans;
	}
	
	return(err);
}
*/

void ClusterSel::Init(Base4D *xdb, IObjToFlush *TheOwner, Table* WhatTable)
{
	bd=xdb;
	SelClusterInMem.Init(IHCLUST.nbclust);
	//SelClusterInMem.SetContientQuoi(t_bitsel);
	if (WhatTable != nil)
		fWhatTable = WhatTable->GetDF();
	else
		fWhatTable = nil;

	SelClusterAddr.Init(xdb, TheOwner, &IHCLUST.addrTabClust, &IHCLUST.debuttrou, &IHCLUST.nbclust, -kIndexSegNum, false); 
}



#if autocheckobj
uBOOL ClusterSel::CheckObjInMem(void)
{
	return(true);
}
#endif

Selection* ClusterSel::LoadSel(OccupableStack* curstack, sLONG vrainclust, BaseTaskInfo* context, VError& err)
{
	Selection *sel;
	err = VE_OK;
	DataAddr4D ou;
	sLONG typsel;
	
	//gOccPool.WaitForMemoryManager(curstack);
	sel=(Selection*) SelClusterInMem.GetFromTreeMemAndOccupy(vrainclust, curstack);
	if (sel == nil)
	{
		//gOccPool.EndWaitForMemoryManager(curstack);
		{
			VTaskLock lock(&fLoadMutex);
	
			//gOccPool.WaitForMemoryManager(curstack);
			sel=(Selection*) SelClusterInMem.GetFromTreeMemAndOccupy(vrainclust, curstack);
			if (sel == nil)
			{
				//gOccPool.EndWaitForMemoryManager(curstack);
				ou = SelClusterAddr.GetxAddr(vrainclust, context, err, curstack);
				if (ou>0)
				{
					DataBaseObjectHeader tag;
					err = tag.ReadFrom(bd, ou, nil);
					if (err == VE_OK)
					{
						err = tag.ValidateTag(DBOH_SetDiskTable, -1, -1);
						if (err != VE_OK)
							err = tag.ValidateTag(DBOH_PetiteSel, -1, -1);
						if (err != VE_OK)
							err = tag.ValidateTag(DBOH_LongSel, -1, -1);
					}
					if (err == VE_OK)
						err = bd->readlong(&typsel,4,ou,kSizeDataBaseObjectHeader);
					if (err==VE_OK)
					{
						if (tag.NeedSwap())
							typsel = SwapLong(typsel);
						if (typsel==sel_bitsel)
						{
							if (fWhatTable == nil)
								sel = new BitSel(bd, (CDB4DBaseContext*)-1);
							else
								sel = new BitSel(fWhatTable);
						}
						else
						{
							if (typsel==sel_petitesel)
							{
								if (fWhatTable == nil)
									sel = new PetiteSel(bd, (CDB4DBaseContext*)-1);
								else
									sel = new PetiteSel(fWhatTable);
							}
						}
					}
					
					xbox_assert(sel != nil);
					if (sel != nil)
					{
						sel->Occupy(curstack, true);
						// sel->occupe();
						sel->SetSelAddr(ou);
						err=sel->LoadSel(tag);
						if (err!=VE_OK)
						{
							sel->Free(curstack, true);
							sel->Release();
							sel = nil;
						}
						else
						{
							err=SelClusterInMem.PutIntoTreeMem(IHCLUST.nbclust,vrainclust,sel, curstack);
							if (err != VE_OK)
							{
								sel->Free(curstack, true);
								sel->Release();
								sel = nil;
							}
						}
					}
				}
			}
			else
			{
				//sel->Occupy(curstack, true);
				//gOccPool.EndWaitForMemoryManager(curstack);
			}
		}
	}
	else
	{
		//sel->Occupy(curstack, true);
		//gOccPool.EndWaitForMemoryManager(curstack);
	}

	/*
	else
	{
		sel->occupe();
	}
	*/
	
	if (err!=VE_OK)
	{
		err = ThrowError(VE_DB4D_CANNOTLOADCLUSTER, DBaction_LoadingCluster);
	}
	return(sel);
}


sLONG ClusterSel::GetNthRecNum(OccupableStack* curstack, sLONG nclust, sLONG Nth, BaseTaskInfo* context, VError& err)
{
	sLONG vrainclust;
	Selection *sel;
	sLONG result = -1;

	err = VE_OK;

	if (nclust < 0)
	{
		vrainclust= -nclust - 2;
		sel = LoadSel(curstack, vrainclust, context, err);
		if (sel != nil)
		{
			result = sel->GetFic(Nth);
			sel->Free(curstack, true);
		}
	}
	else
	{
		if (Nth == 0)
			result = nclust;
	}
	return result;
}


Selection* ClusterSel::GetSel(OccupableStack* curstack, sLONG nclust, BaseTaskInfo* context, VError& err)
{
	sLONG vrainclust;
	Selection *sel = nil;
	err = VE_OK;

	if (nclust < 0)
	{
		vrainclust= -nclust - 2;
		sel = LoadSel(curstack, vrainclust, context, err);
		if (sel != nil)
		{
			sel->Retain();
			sel->Free(curstack, true);
		}
	}
	return sel;
}


sLONG ClusterSel::GetNextRecNum(OccupableStack* curstack, sLONG nclust, sLONG Nth, sLONG recnum, BaseTaskInfo* context, VError& err)
{
	sLONG vrainclust;
	Selection *sel;
	sLONG result = -1;

	err = VE_OK;

	if (nclust < 0)
	{
		vrainclust= -nclust - 2;
		sel = LoadSel(curstack, vrainclust, context, err);
		if (sel != nil)
		{
			if (sel->GetTypSel() == sel_bitsel)
			{
				Bittab *bb = ((BitSel*)sel)->GetBittab();
				result = bb->FindNextBit(recnum+1);
			}
			else
			{
				if (Nth<sel->GetQTfic())
					result = sel->GetFic(Nth+1);
			}
			sel->Free(curstack, true);
		}
	}
	else
	{
		result = -1;
	}
	return result;
}


VError ClusterSel::AddSel(OccupableStack* curstack, Selection* sel, BaseTaskInfo* context)
{
	VError err = VE_OK;
	
	sel->Retain();

	sLONG nclust = SelClusterAddr.findtrou(context, err, curstack);
	if (err == VE_OK)
	{
		if (nclust<0)
		{
			err=SelClusterInMem.PutIntoTreeMem(IHCLUST.nbclust+1,IHCLUST.nbclust,sel, curstack);
			if (err==VE_OK)
			{
				nclust=IHCLUST.nbclust;
			}
		}
		else
		{
			err=SelClusterInMem.PutIntoTreeMem(IHCLUST.nbclust,nclust,sel, curstack);
		}
	}

	if (err == VE_OK)
	{
		sLONG len = sel->CalcLenOnDisk();
		DataAddr4D ou = bd->findplace(len+kSizeDataBaseObjectHeader, context, err, -kIndexSegNum, sel);
		if (ou>0)
		{
			//sel->ForbidWrite();
			err = sel->TransformIntoCluster(ou);
			if (err == VE_OK)
			{
#if debugIndexOverlap_strong
				di_IndexOverLap::AddCluster(this, nclust, ou, len+kSizeDataBaseObjectHeader);
#endif
				//sel->SetSelAddr(ou);
				err=SelClusterAddr.PutxAddr(nclust,ou,len, context, curstack);
				sel->setmodif(true, bd, context);
			}
			else
			{
				sel->setmodif(false, bd, context);
				bd->libereplace(ou, len+kSizeDataBaseObjectHeader, context, sel);
			}
		}
	}
	else
		sel->Release();

	return err;
}


sLONG ClusterSel::AddToCluster(OccupableStack* curstack, sLONG nclust, sLONG n, BaseTaskInfo* context, VError& err)
{
	sLONG vrainclust;
	Selection *sel, *sel2;
	sLONG len,oldlen;
	DataAddr4D ou,oldou;
	
	//SaveIHCLUSTForTrans(context);
	err=VE_OK;
	if (nclust == -1)
	{
		return(n);
	}
	else
	{
		if (nclust<0)
		{
			vrainclust= -nclust - 2;
			sel = LoadSel(curstack, vrainclust, context, err);
			if (sel!=nil)
			{
				//sel->ForbidWrite();
				sel->WriteLock();
				oldlen = sel->CalcLenOnDisk();
				oldou = sel->getaddr();
				err=sel->AddToSel(n);
				if (err==VE_OK)
				{
					len = sel->CalcLenOnDisk();
					if (adjuste(len+kSizeDataBaseObjectHeader) != adjuste(oldlen+kSizeDataBaseObjectHeader))
					{
						ou=bd->findplace(len+kSizeDataBaseObjectHeader, context, err, -kIndexSegNum, sel);
						if (ou>0 && err == VE_OK)
						{
#if debugIndexOverlap_strong
							di_IndexOverLap::AddCluster(this, vrainclust, ou, len+kSizeDataBaseObjectHeader);
#endif
							sel->ChangeAddr(ou, bd, context);
							err=SelClusterAddr.PutxAddr(vrainclust,ou,len, context, curstack);
							sel->setmodif(true, bd, context);
						}
						if (err == VE_OK)
						{
#if debugIndexOverlap_strong
							di_IndexOverLap::RemoveCluster(this, vrainclust, oldou, oldlen+kSizeDataBaseObjectHeader);
#endif
							// #filtrer err
							bd->libereplace(oldou, oldlen+kSizeDataBaseObjectHeader, context, sel);

						}
					}
					else
					{
						sel->setmodif(true, bd, context);
						if (len != oldlen)
							err=SelClusterAddr.PutxAddr(vrainclust,oldou,len, context, curstack);
					}
					sel->WriteUnLock();

					//if ( err == VE_OK && (sel->GetTypSel() == sel_petitesel) && (sel->GetQTfic() > MaxSizeClusterLong) )
					if ((err == VE_OK) && MustConvertPetiteSelToBitSel(sel))
					{
						sel2 = new BitSel(fWhatTable);
						if (sel2 != nil)
						{
							sel2->Occupy(curstack, true);
							//sel2->ForbidWrite();
							// sel2->occupe();
							oldlen = sel2->CalcLenOnDisk();
							ou = bd->findplace(oldlen+kSizeDataBaseObjectHeader, context, err, -kIndexSegNum, sel2);
							oldou=ou;
							if (ou>0 && err == VE_OK)
							{
#if debugIndexOverlap_strong
								di_IndexOverLap::AddCluster(this, vrainclust, ou, oldlen+kSizeDataBaseObjectHeader);
#endif

								sel2->SetSelAddr(ou);
								err = ((PetiteSel*)sel)->PutIntoBittab( ((BitSel*)sel2)->GetBittab() );
								if (err == VE_OK)
								{
									len = sel2->CalcLenOnDisk();

									if (adjuste(len+kSizeDataBaseObjectHeader) != adjuste(oldlen+kSizeDataBaseObjectHeader))
									{
										ou=bd->findplace(len+kSizeDataBaseObjectHeader, context, err, -kIndexSegNum, sel2);
										if (ou>0 && err == VE_OK)
										{
#if debugIndexOverlap_strong
											di_IndexOverLap::AddCluster(this, vrainclust, ou, len+kSizeDataBaseObjectHeader);
#endif
											sel2->setmodif(true, bd, context);
											sel2->ChangeAddr(ou, bd, context);
#if debugIndexOverlap_strong
											di_IndexOverLap::RemoveCluster(this, vrainclust, oldou, oldlen+kSizeDataBaseObjectHeader);
#endif
											bd->libereplace(oldou, oldlen+kSizeDataBaseObjectHeader, context, sel2);
										}
									}
									else
										sel2->setmodif(true, bd, context);


									if (err == VE_OK)
									{
										sel->WriteLock();
										sel->SetValidForWrite(false);
										err=SelClusterInMem.PutIntoTreeMem(IHCLUST.nbclust,vrainclust,sel2, curstack);
										sel->WriteUnLock();
										sel->setmodif(false, bd, context);
										sel->LibereEspaceDisk(nil, nil);
										//VDBMgr::RemoveObjectFromFlush( sel);
										//sel->SetParent(nil, -1);
										//sel->SetDansCache(false);
										sel->Free(curstack, true);
										sel->Release();

										err=SelClusterInMem.PutIntoTreeMem(IHCLUST.nbclust,vrainclust,sel2, curstack);
										err=SelClusterAddr.PutxAddr(vrainclust,ou,len, context, curstack);
										sel=sel2;
										sel2=nil;
									}
								}
								
							}
						}
						
						if (sel2 != nil)
						{
							sel2->Free(curstack, true);
							sel2->Release();
							sel2=nil;
						}
					}
				}
				else
				{
					err = err; // break here #index
					sel->WriteUnLock();
				}
				sel->Free(curstack, true);
			}
			
		}
		else
		{
			sel = new PetiteSel(fWhatTable);
			if (sel!=nil)
			{
				sel->Occupy(curstack, true);
				//sel->occupe();
				err=sel->AddToSel(nclust);
				if (err == VE_OK)
					err=sel->AddToSel(n);
				if (err==VE_OK)
				{
					//sel->ForbidWrite();

					vrainclust = SelClusterAddr.findtrou(context, err, curstack);
					if (err == VE_OK)
					{
						if (vrainclust<0)
						{
							err=SelClusterInMem.PutIntoTreeMem(IHCLUST.nbclust+1,IHCLUST.nbclust,sel, curstack);
							if (err==VE_OK)
							{
								vrainclust=IHCLUST.nbclust;
							}
						}
						else
						{
							err=SelClusterInMem.PutIntoTreeMem(IHCLUST.nbclust,vrainclust,sel, curstack);
						}
					}
				}
#if debugIndexOverlap_strong
				else
				{
					err = err; // break here #index
				}
#endif
				
				if (err == VE_OK)
				{
					len = sel->CalcLenOnDisk();
					ou = bd->findplace(len+kSizeDataBaseObjectHeader, context, err, -kIndexSegNum, sel);
					if (ou>0)
					{
#if debuglr == 114
						VString s(L" Sel FindPlace : ");
						VString s2;
						s2.FromLong8(ou);
						s+=s2;
						s+=L" , ";
						s2.FromLong(len+kSizeDataBaseObjectHeader);
						s+=s2;
						s2.FromLong(vrainclust);
						s+=L"   :   clust = ";
						s+=s2;
						s+=L" , type = ";
						s2.FromLong(sel->GetTypSel());
						s+=s2;
						s+=L"\n";
						DebugMsg(s);
#endif

#if debugIndexOverlap_strong
						di_IndexOverLap::AddCluster(this, vrainclust, ou, len+kSizeDataBaseObjectHeader);
#endif
						sel->SetSelAddr(ou);
						sel->setmodif(true, bd, context);
						err=SelClusterAddr.PutxAddr(vrainclust,ou,len, context, curstack);
					}
				}
				
				sel->Free(curstack, true);
			}
			else 
				err = ThrowError(memfull, DBaction_AddingToCluster);
		}
				
		if (err != VE_OK)
			err = ThrowError(VE_DB4D_CANNOTADDTOCLUSTER, DBaction_AddingToCluster);
		return(-(vrainclust+2));
	}
}
	
	
sLONG ClusterSel::DelFromCluster(OccupableStack* curstack, sLONG nclust, sLONG n, BaseTaskInfo* context, VError& err)
{
	sLONG res;
	sLONG vrainclust;
	Selection *sel, *sel2;
	sLONG len,oldlen,qt;
	err = VE_OK;
	DataAddr4D ou,oldou;
	
	//SaveIHCLUSTForTrans(context);
	
	if (nclust>=0)
	{
		if (n == nclust)
		{
			res = -1;
		}
		else
		{
			res = nclust;
		}
	}
	else
	{
		vrainclust = -nclust - 2;

		sel = LoadSel(curstack, vrainclust, context, err);
		if (sel!=nil)
		{
			//sel->ForbidWrite();
			sel->WriteLock();

			oldlen = sel->CalcLenOnDisk();
			oldou = sel->getaddr();
			err=sel->DelFromSel(n);
			if (err==VE_OK)
			{
				len = sel->CalcLenOnDisk();
				if (adjuste(len+kSizeDataBaseObjectHeader) != adjuste(oldlen+kSizeDataBaseObjectHeader))
				{
					ou=bd->findplace(len+kSizeDataBaseObjectHeader, context, err, -kIndexSegNum, sel);
					if (ou>0)
					{
#if debugIndexOverlap_strong
						di_IndexOverLap::AddCluster(this, vrainclust, ou, len+kSizeDataBaseObjectHeader);
#endif
						err=SelClusterAddr.PutxAddr(vrainclust,ou,len, context, curstack);
						sel->ChangeAddr(ou, bd, context);
						sel->setmodif(true, bd, context);
#if debugIndexOverlap_strong
						di_IndexOverLap::RemoveCluster(this, vrainclust, oldou, oldlen+kSizeDataBaseObjectHeader);
#endif
						bd->libereplace(oldou,oldlen+kSizeDataBaseObjectHeader, context, sel);
					}
				}
				else
				{
					sel->setmodif(true, bd, context);
					if (len != oldlen)
						err=SelClusterAddr.PutxAddr(vrainclust,oldou,len, context, curstack);
				}
				sel->WriteUnLock();

				if (err == VE_OK)
				{
					qt=sel->GetQTfic();
					if (qt<2)
					{
						if (qt==1)
						{
							res = sel->GetFic(0);
						}
						else
						{
							res = -1;
						}
						sel->WriteLock();
						sel->SetValidForWrite(false);
						sel->WriteUnLock();
						sel->setmodif(false, bd, context);
						sel->LibereEspaceDisk(nil, nil);
						SelClusterAddr.liberetrou(vrainclust, context, curstack);
						SelClusterInMem.DelFromTreeMem(vrainclust, curstack);
						sel->Free(curstack, true);
						sel->Release();
						sel=nil;
						vrainclust = -res - 2;
					}
					else
					{
						//if ( (qt<MaxSizeClusterLongMin) && (sel->GetTypSel() == sel_bitsel) )
						if (MustConvertBitSelToPetiteSel(sel))
						{
							sel2 = new PetiteSel(fWhatTable);
							if (sel2!=nil)
							{
								sel2->Occupy(curstack, true);
								// sel2->occupe();
								if (sel2->FixFic(qt))
								{
									err=sel->FillArray( ((PetiteSel*)sel2)->GetArrayPtr(), qt );
									((PetiteSel*)sel2)->ReleaseArrayPtr();
									if (err == VE_OK)
									{
										len = sel2->CalcLenOnDisk();
										ou = bd->findplace(len+kSizeDataBaseObjectHeader, context, err, -kIndexSegNum, sel2);
										if (ou>0)
										{
#if debugIndexOverlap_strong
											di_IndexOverLap::AddCluster(this, vrainclust, ou, len+kSizeDataBaseObjectHeader);
#endif
											sel2->SetSelAddr(ou);
											sel2->setmodif(true, bd, context);
											err=SelClusterInMem.PutIntoTreeMem(IHCLUST.nbclust,vrainclust,sel2,curstack);
											if (err == VE_OK)
											{
												sel->WriteLock();
												sel->setmodif(false, bd, context);
												sel->WriteUnLock();
												sel->LibereEspaceDisk(nil, nil);
												sel->Free(curstack, true);
												sel->Release();
												//err=SelClusterInMem.PutIntoTreeMem(IHCLUST.nbclust,vrainclust,sel2);
												err=SelClusterAddr.PutxAddr(vrainclust,ou,len, context, curstack);
												sel=sel2;
												sel2=nil;
											}
										}
									}
								}
								else
								{
									err = ThrowError(memfull, DBaction_DeletingFromCluster);
								}
							}
							
						}
					}
				}
				
			}
			else
			{
				err = err; // break here #index
				sel->WriteUnLock();
			}
				
			if (sel!=nil) 
				sel->Free(curstack, true);
		}
		
		res = -(vrainclust+2);
	}
	
	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTDELFROMCLUSTER, DBaction_DeletingFromCluster);
	return(res);
}

/*
void ClusterSel::SaveIHCLUSTForTrans(BaseTaskInfo* context)
{
	if ((GetCurTrans(context) != nil) && (IHCLUST_SavedByTrans.nbclust == -1))
	{
		IHCLUST_SavedByTrans = IHCLUST;
	}
}
*/


void ClusterSel::AddSelToBittab(OccupableStack* curstack, sLONG nclust, Bittab *b, BaseTaskInfo* context, VError& err)
{
	err = VE_OK;
	Selection *sel;
	if (nclust>=0)
	{
		err = b->Set(nclust);
	}
	else
	{
		nclust = -nclust - 2;
		if (nclust != -1)
		{
			sel = LoadSel(curstack, nclust, context, err);
			if (sel!=nil)
			{
				err = sel->AddToBittab(b, context);
				sel->Free(curstack, true);
			}
		}
	}
}


Boolean ClusterSel::IntersectSel(OccupableStack* curstack, sLONG nclust, Bittab *b, BaseTaskInfo* context, VError& err)
{
	Selection *sel;
	err = VE_OK;
	Boolean res = false;

	if (nclust>=0)
	{
		res = b->isOn(nclust);
	}
	else
	{
		nclust = -nclust - 2;
		if (nclust != -1)
		{
			sel = LoadSel(curstack, nclust, context, err);
			if (sel!=nil)
			{
				res = sel->IntersectBittab(b, context, err);
				sel->Free(curstack, true);
			}
		}
	}

	return res;
}


VError ClusterSel::AddSelToSel(OccupableStack* curstack, sLONG& curfic, sLONG maxnb, Bittab* filtre, Selection* sel, sLONG n, BaseTaskInfo* context, Bittab* inWafSelbits, WafSelection* outWafSel)
{
	Selection *sel2;
	VError err = VE_OK;
	
	if (n>=0)
	{
		if (filtre == nil || filtre->isOn(n))
		{
			if (curfic<maxnb)
			{
				err = sel->PutFicWithWafSelection(curfic,n, inWafSelbits, outWafSel);
				++curfic;
			}
		}
	}
	else
	{
		n = -n - 2;
		if (n != -1)
		{
			sel2 = LoadSel(curstack, n, context, err);
			if (sel2!=nil)
			{
				if (inWafSelbits != nil && outWafSel != nil)
					err = sel2->AddSelToSelWithWafSelection(curfic, maxnb, filtre, sel, inWafSelbits, outWafSel);
				else
					err = sel2->AddSelToSel(curfic, maxnb, filtre, sel);
				sel2->Free(curstack, true);
			}
		}
	}
	return err;
}


VError ClusterSel::CalculateColumnFormulasOnCluster(OccupableStack* curstack, ColumnFormulas* formules, BTitemIndex* u, sLONG nclust, 
													Bittab* filtre, BaseTaskInfo* context, Boolean& stop, BTreePageIndex* page)
{
	VError err = VE_OK;
	if (nclust>=0)
	{
		if (filtre == nil || filtre->isOn(nclust))
			err = page->CalculateColumnFormulasOnOneKey(formules, u, stop, context);
	}
	else
	{
		nclust = -nclust - 2;
		if (nclust != -1)
		{
			Selection *sel = LoadSel(curstack, nclust, context, err);
			if (sel != nil)
			{
				if (sel->GetTypSel() == sel_petitesel)
				{
					sLONG nb = sel->GetQTfic();
					sLONG i;

					for (i = 0; i<nb && err == VE_OK; i++)
					{
						sLONG n = sel->GetFic(i);
						if (n>=0)
						{
							if (filtre == nil || filtre->isOn(n))
							{
								err = page->CalculateColumnFormulasOnOneKey(formules, u, stop, context);
							}
						}
					}

				}
				else
				{
					Bittab* b = sel->GenereBittab(context, err);
					Bittab inter;
					if (err == VE_OK)
						err = inter.Or(filtre);
					if (err == VE_OK)
					{
						err = inter.And(b);
						if (err == VE_OK)
						{
							sLONG i = inter.FindNextBit(0);
							while (i != -1 && err == VE_OK)
							{
								err = page->CalculateColumnFormulasOnOneKey(formules, u, stop, context);
								i = inter.FindNextBit(i+1);
							}
						}
					}
					ReleaseRefCountable(&b);
				}
				sel->Free(curstack, true);
			}
		}
	}

	return err;
}


void ClusterSel::LibereEspaceMem(OccupableStack* curstack)
{
	SelClusterInMem.LibereEspaceMem(curstack);
}


void ClusterSel::TryToFreeMem(sLONG allocationBlockNumber, VSize combien, VSize& tot)
{
	SelClusterInMem.TryToFreeMem(allocationBlockNumber, combien, tot);

	// if (tot < combien)
	{
		VSize tot2 = 0;
		SelClusterAddr.TryToFreeMem(allocationBlockNumber, combien-tot, tot2);
		tot = tot + tot2;
	}
}


VError ClusterSel::LibereEspaceDisk(OccupableStack* curstack, VDB4DProgressIndicator* InProgress)
{
	sLONG i;
	VError err = VE_OK;

	for (i=0; i<IHCLUST.nbclust; i++)
	{
		VError err2 = VE_OK;
		Selection* sel = LoadSel(curstack, i, nil, err2);
		if (sel != nil)
		{
			sel->LibereEspaceDisk(curstack, InProgress);
			sel->Free(curstack, true);
		}
	}

	SelClusterAddr.LibereEspaceDisk(curstack, InProgress);
	return err;
}


VError ClusterSel::InitTableAddr(OccupableStack* curstack)
{
	sLONG nbelem = IHCLUST.nbclust;
	IHCLUST.nbclust = 0;
	IHCLUST.addrTabClust = 0;
	IHCLUST.debuttrou = kFinDesTrou;
	return SelClusterAddr.InitAndSetSize(nbelem, nil, curstack);
}

VError ClusterSel::NormalizeTableAddr(OccupableStack* curstack)
{
	return SelClusterAddr.Normalize(nil, curstack);
}




									/* -----------------------------------------------  */



IndexHeaderBTreeCluster::IndexHeaderBTreeCluster(IndexInfo *xentete) : IndexHeaderBTree(xentete)  
{ 
	typindex = DB4D_Index_BtreeWithCluster;
	cls.Init(xentete->GetDB(), this, xentete->GetTargetTable());
};


void IndexHeaderBTreeCluster::SetAssoc(IndexInfo* x) 
{
	IndexHeaderBTree::SetAssoc(x);
	cls.Init(x->GetDB(), this, x->GetTargetTable());
}


void IndexHeaderBTreeCluster::Update(Table* inTable)
{
	cls.SetDataTable(inTable->GetDF());
}


BTreePageIndex* IndexHeaderBTreeCluster::CrePage(void)
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
		res=new (p)BTreePageIndexCluster(GetAssoc(), len, -1);
	}

	return(res);
}


BTreePageIndex* IndexHeaderBTreeCluster::LoadPage(OccupableStack* curstack, DataAddr4D addr, sLONG len, BaseTaskInfo* context, VError& err, sLONG xnum)
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
		res=new (p)BTreePageIndexCluster(GetAssoc(), lenInMem, xnum);
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


IndexHeader* CreIndexHeaderBTreeCluster(IndexInfo *xentete)
{
	IndexHeaderBTreeCluster *head;
	
	head=new IndexHeaderBTreeCluster(xentete);
	return(head);
}


sLONG IndexHeaderBTreeCluster::GetLen(void)
{
	return(IndexHeaderBTree::GetLen()+sizeof(ClusterDISK));
}

VError IndexHeaderBTreeCluster::PutInto(VStream& buf)
{
	VError err;
	
	err=IndexHeaderBTree::PutInto(buf);
	if (err==VE_OK) err=buf.PutData(cls.getIHCLUST(),sizeof(ClusterDISK));

	return(err);
}


void ClusterDISK::SwapBytes()
{
	ByteSwap(&addrTabClust);
	ByteSwap(&debuttrou);
	ByteSwap(&nbclust);
}


VError IndexHeaderBTreeCluster::GetFrom(VStream& buf)
{
	VError err;
	
	err=IndexHeaderBTree::GetFrom(buf);
	if (err==VE_OK) err=buf.GetData(cls.getIHCLUST(),sizeof(ClusterDISK));
	if (err == VE_OK && buf.NeedSwap())
		cls.getIHCLUST()->SwapBytes();

	return(err);
}


void IndexHeaderBTreeCluster::TryToFreeMem(sLONG allocationBlockNumber, VSize combien, VSize& tot)
{
	tot = 0;

	cls.TryToFreeMem(allocationBlockNumber, combien, tot);

	if (/*tot < combien && */firstpage != nil)
	{
		VSize tot2 = 0;
		if (firstpage->FreePageMem(allocationBlockNumber, combien-tot, tot2) && !IsOccupied())
		{
			tot2 = tot2 + firstpage->GetSizeToFree();
			xbox_assert(firstpage->GetRefCount() == 1);
			firstpage->Release();
			firstpage = nil;
		}
		tot = tot + tot2;
	}

	//if (tot < combien)
	{
		VSize tot2 = 0;
		taddr.TryToFreeMem(allocationBlockNumber, combien-tot, tot2);
		tot = tot + tot2;
	}
}


/*
sLONG IndexHeaderBTreeCluster::GetNthRecNum(sLONG nclust, sLONG Nth, BaseTaskInfo* context, VError& err)
{
	return cls.GetNthRecNum(nclust, Nth, context, err);
}


sLONG IndexHeaderBTreeCluster::GetNextRecNum(sLONG nclust, sLONG Nth, sLONG recnum, BaseTaskInfo* context, VError& err)
{
	return cls.GetNextRecNum(nclust, Nth, recnum, context, err);
}


sLONG IndexHeaderBTreeCluster::AddToClusterSel(sLONG nclust, sLONG n, BaseTaskInfo* context, VError& err)
{
	return(cls.AddToCluster(nclust,n, context, err));
}


sLONG IndexHeaderBTreeCluster::DelFromClusterSel(sLONG nclust, sLONG n, BaseTaskInfo* context, VError& err)
{
	return(cls.DelFromCluster(nclust,n, context, err));
}


void IndexHeaderBTreeCluster::AddSelToBittab(sLONG nclust, Bittab *b, BaseTaskInfo* context, VError& err)
{
	cls.AddSelToBittab(nclust,b, context, err);
}


VError IndexHeaderBTreeCluster::AddSelToSel(sLONG& curfic, sLONG maxnb, Bittab* filtre, Selection* sel, sLONG n, BaseTaskInfo* context)
{
	return cls.AddSelToSel(curfic, maxnb, filtre, sel, n, context);
}


Boolean IndexHeaderBTreeCluster::IntersectSel(sLONG nclust, Bittab *b, BaseTaskInfo* context, VError& err)
{
	return cls.IntersectSel(nclust, b, context, err);
}


VError IndexHeaderBTreeCluster::CalculateColumnFormulasOnCluster(ColumnFormulas* formules, BTitemIndex* u, sLONG nclust, 
																Bittab* filtre, BaseTaskInfo* context, Boolean& stop, BTreePageIndex* page)
{
	return cls.CalculateColumnFormulasOnCluster(formules, u, nclust, filtre, context, stop, page);
}


VError IndexHeaderBTreeCluster::AddSel(Selection* sel, BaseTaskInfo* context)
{
	return cls.AddSel(sel, context);
}
*/

VError IndexHeaderBTreeCluster::LibereEspaceDisk(OccupableStack* curstack, VDB4DProgressIndicator* InProgress)
{
	VError err = VE_OK;


	err = cls.LibereEspaceDisk(curstack, InProgress);
	if (err == VE_OK)
	{
		err = IndexHeaderBTree::LibereEspaceDisk(curstack, InProgress);
	}

	HBT.FirstPage = -1;
	return err;
}


void IndexHeaderBTreeCluster::LibereEspaceMem(OccupableStack* curstack)
{
	cls.LibereEspaceMem(curstack);
	IndexHeaderBTree::LibereEspaceMem(curstack);
}


VError IndexHeaderBTreeCluster::InitTablesAddr(OccupableStack* curstack)
{
	VError err = IndexHeaderBTree::InitTablesAddr(curstack);
	if (err == VE_OK)
	{
		err = cls.InitTableAddr(curstack);
	}
	return err;
}


VError IndexHeaderBTreeCluster::NormalizeTablesAddr(OccupableStack* curstack)
{
	VError err = IndexHeaderBTree::NormalizeTablesAddr(curstack);
	if (err == VE_OK)
	{
		err = cls.NormalizeTableAddr(curstack);
	}
	return err;
}




#if 0

									/* -----------------------------------------------  */




IndexHeaderBTreeFixSizeCluster::IndexHeaderBTreeFixSizeCluster(IndexInfo *xentete) : IndexHeaderBTreeFixSize(xentete)  
{ 
	cls.Init(xentete->GetDB(), this);
};


BTreePageFixSizeIndex* IndexHeaderBTreeFixSizeCluster::CrePage(void)
{
	BTreePageFixSizeIndex* res;
	
	res=new BTreePageIndexFixSizeCluster(GetAssoc(), fMaxSize);
	return(res);
}


IndexHeader* CreIndexHeaderBTreeFixSizeCluster(IndexInfo *xentete)
{
	IndexHeaderBTreeFixSizeCluster *head;
	
	head=new IndexHeaderBTreeFixSizeCluster(xentete);
	return(head);
}


sLONG IndexHeaderBTreeFixSizeCluster::GetLen(void)
{
	return(IndexHeaderBTreeFixSize::GetLen()+sizeof(ClusterDISK));
}

VError IndexHeaderBTreeFixSizeCluster::PutInto(VStream& buf)
{
	VError err;
	
	err=IndexHeaderBTreeFixSize::PutInto(buf);
	if (err==VE_OK) err=buf.PutData(cls.getIHCLUST(),sizeof(ClusterDISK));

	return(err);
}

VError IndexHeaderBTreeFixSizeCluster::GetFrom(VStream& buf)
{
	VError err;
	
	err=IndexHeaderBTreeFixSize::GetFrom(buf);
	if (err==VE_OK) err=buf.GetData(cls.getIHCLUST(),sizeof(ClusterDISK));
	if (err == VE_OK)
		cls.getIHCLUST()->SwapBytes();

	return(err);
}


sLONG IndexHeaderBTreeFixSizeCluster::AddToClusterSel(sLONG nclust, sLONG n, BaseTaskInfo* context, VError& err)
{
	return(cls.AddToCluster(nclust,n, context, err));
}


sLONG IndexHeaderBTreeFixSizeCluster::DelFromClusterSel(sLONG nclust, sLONG n, BaseTaskInfo* context, VError& err)
{
	return(cls.DelFromCluster(nclust,n, context, err));
}


void IndexHeaderBTreeFixSizeCluster::AddSelToBittab(sLONG nclust, Bittab *b, BaseTaskInfo* context, VError& err)
{
	cls.AddSelToBittab(nclust,b, context, err);
}


VError IndexHeaderBTreeFixSizeCluster::AddSelToSel(sLONG& curfic, sLONG maxnb, Bittab* filtre, Selection* sel, sLONG n, BaseTaskInfo* context)
{
	return cls.AddSelToSel(curfic, maxnb, filtre, sel, n, context);
}




#endif // du if 0

									/* -----------------------------------------------  */



