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


#if debuglr == 11
class debug_fic_holder : public VObject, public ITreeable<debug_fic_holder>
{
public:
	debug_fic_holder(FicheInMem* x) { holder = x; };
	FicheInMem* holder;
	inline Boolean IsEqualTo(debug_fic_holder* other) const { return holder == other->holder; };
	inline Boolean IsLessThan(debug_fic_holder* other) const { return (sLONG)holder < (sLONG)other->holder; };
};

static VBinTreeOf<debug_fic_holder> debug_treerec;

void debug_checktreerec(FicheOnDisk* fd)
{
	debug_fic_holder* h = debug_treerec.First();
	while (h != nil)
	{
		xbox_assert(h->holder->GetAssoc() != fd);
		h = h->Next();
	}
}

#endif



#if debugFicheOnDisk


void FicheRetainerDebug::CheckAssocData(FicheOnDisk* ficD)
{
	VTaskLock lock(&fMutex);
	for (FicheSet::iterator cur = fRecords.begin(), end = fRecords.end(); cur != end; cur++)
	{
		FicheInMem* rec = *cur;
		if (rec->GetAssoc() == ficD)
		{
			xbox_assert(false);
		}
	}

}

void FicheRetainerDebug::AddFiche(FicheInMem* rec)
{
	VTaskLock lock(&fMutex);
	fRecords.insert(rec);
}

void FicheRetainerDebug::RemoveFiche(FicheInMem* rec)
{
	VTaskLock lock(&fMutex);
	fRecords.erase(rec);
}


FicheRetainerDebug gDebugFiches;

#endif



				// -----------------------------------------------------------------------------------------------------------------


PrimKey::~PrimKey()
{
	for (parts::iterator cur = fParts.begin(), end = fParts.end(); cur != end; ++cur)
	{
		VValueSingle* cv = const_cast<VValueSingle*>(*cur);
		if (cv != nil)
			delete (cv);
	}
}


VError PrimKey::PutInto(VStream* outStream)
{
	VError err = VE_OK;
	sLONG nb = (sLONG)fParts.size();
	err = outStream->PutLong(nb);
	if (err == VE_OK)
	{
		for (parts::iterator cur = fParts.begin(), end = fParts.end(); cur != end && err == VE_OK; ++cur)
		{
			const VValueSingle* cv = *cur;
			if (cv == nil)
			{
				err = outStream->PutWord(VK_EMPTY);
			}
			else
			{
				ValueKind kind = cv->GetValueKind();
				err = outStream->PutWord(kind);
				if (err == VE_OK)
				{
					err = cv->WriteToStream(outStream);
				}
			}

		}
	}
	return err;
}


sLONG PrimKey::GetLength() const
{
	sLONG len = 4; // sizeof long
	for (parts::const_iterator cur = fParts.begin(), end = fParts.end(); cur != end; ++cur)
	{
		len += 2; // sizeof word (kind)
		const VValueSingle* cv = *cur;
		if (cv != nil)
		{
			///ACI0087130 Mar 25th 2014, PrimKey::GetLength() used to write the PK size to a stream (e.g. log) so get the size of each VValue for streaming (e.g VBoolean uses 2 bytes vs 1 in memory)
			len += (sLONG)cv->GetStreamSpace();
		}
	}

	return len;
}



VError PrimKey::GetFrom(VStream* inStream)
{
	VError err = VE_OK;
	sLONG nb;
	err = inStream->GetLong(nb);
	if (err == VE_OK)
	{
		for (sLONG i = 0; i < nb && err == VE_OK; ++i)
		{
			ValueKind kind;
			err = inStream->GetWord(kind);
			VValueSingle* cv = nil;
			if (kind != VK_EMPTY && err == VE_OK)
			{
				cv = dynamic_cast<VValueSingle*>(VValue::NewValueFromValueKind(kind));
				if (cv == nil)
					err = ThrowBaseError(VE_STREAM_CANNOT_GET_DATA);
				else
				{
					err = cv->ReadFromStream(inStream);
					if (err != VE_OK)
					{
						delete cv;
						cv = nil;
					}
				}
			}
			if (err == VE_OK)
			{
				fParts.push_back(cv);
			}
		}
	}

	return err;
}


VError PrimKey::GetFrom(void* rec, bool oldOne)
{
	FicheInMem* fic = (FicheInMem*)rec;
	VError err = VE_OK;
	FieldArray primkeyfields;
	fic->GetOwner()->RetainPrimaryKey(primkeyfields);
	for (FieldArray::Iterator cur = primkeyfields.First(), end = primkeyfields.End(); cur != end && err == VE_OK; cur++)
	{
		VValueSingle* cv;
		if (oldOne && !fic->IsNew())
			cv = fic->GetFieldOldValue(*cur, err);
		else
			cv = fic->GetFieldValue(*cur, err);
		if (cv != nil)
		{
			cv = cv->Clone();
		}
		if (cv == nil)
		{
			err = fic->GetOwner()->ThrowError(VE_DB4D_PRIMKEY_MALFORMED, noaction);
		}
		else
		{
			fParts.push_back(cv);
		}
	}
	for (FieldArray::Iterator cur = primkeyfields.First(), end = primkeyfields.End(); cur != end; cur++)
		(*cur)->Release();
	return err;
}


bool PrimKey::operator < (const PrimKey& other) const
{
	sLONG len1 = (sLONG)fParts.size();
	sLONG len2 = (sLONG)other.fParts.size();
	sLONG len = len1;
	if (len1 > len2)
		len = len2;

	for (sLONG i = 0; i < len; ++i)
	{
		const VValueSingle* cv1 = fParts[i];
		const VValueSingle* cv2 = other.fParts[i];
		if (cv1 == nil)
		{
			if (cv2 != nil)
				return true;
		}
		else
		{
			if (cv2 == nil)
				return false;
			else
			{
				CompareResult result = cv1->CompareToSameKind(cv2);
				if (result == CR_SMALLER)
					return true;
				else if (result == CR_BIGGER)
					return false;
				
			}
		}
	}

	if (len1 < len2)
		return true;
	else
		return false;
}

bool PrimKey::IsValid() const
{
	bool res = !(fParts.empty());
	for (parts::const_iterator cur = fParts.begin(), end = fParts.end(); cur != end; ++cur)
	{
		const VValueSingle* cv = *cur;
		if (cv == nil)
		{
			res = false;
		}
		else if (cv->IsNull())
		{
			res = false;
		}
	}
	return res;
}


void PrimKey::AddPart(XBOX::VValueSingle* part)
{
	VValueSingle* cv = part->Clone();
	fParts.push_back(cv);
}


void PrimKey::GetAsString(VString& outKey, bool autoQuote) const
{
	/*
	VectorOfVValue vals;
	for (parts::const_iterator cur = fParts.begin(), end = fParts.end(); cur != end; ++cur)
	{
		vals.push_back(*cur);
	}
	EntityRecord::GetPrimKeyAsString(vals, outKey, autoQuote);
	*/

	EntityRecord::GetPrimKeyAsString(fParts, outKey, autoQuote);
}





				// -----------------------------------------------------------------------------------------------------------------


RelationDep::~RelationDep()
{
	if (fRec != nil)
		fRec->Release();

	if (fSel != nil)
		fSel->Release();

	if (fRel != nil)
		fRel->Release();
}


Selection* RelationDep::GetSelection(uLONG stamp)
{
	if (stamp != fStamp && !fOldOne)
	{
		if (fSel != nil)
		{
			fSel->Release();
			fSel = nil;
		}
	}
	return fSel;
}


FicheInMem* RelationDep::GetRecord(uLONG stamp)
{
	if (stamp != fStamp && !fOldOne)
	{
		if (fRec != nil)
		{
			fRec->Release();
			fRec = nil;
		}
	}
	return fRec;
}



								/*  --------------------------------------------  */


FicheInMem::FicheInMem(BaseTaskInfo* Context, Base4D* xdb, Table* crit, VError& err)  // equivalent du initmem
{
	sLONG nb,i;
	ValPtr chp;
	StAllocateInCache	alloc;
	err = VE_OK;

	fIsUsedAsCacheInQuery = false;
	fAddressAllreadyTaken = false;
	fAllAlwaysNull = false;
	fIsASubRecordOnClient = false;
	fUsedAsSubRecordOnServer = false;
	fReadOnly = false;
	fIsRemote = false;
	fIsIDGenerated = false;
	fLockingContextExtraData = nil;
	fKindOfLock = DB4D_LockedByNone;
	fModificationStamp = 0;

	fDejaRequested = false;
	fRequestedFields = nil;
	fRequestedFieldsMaster = nil;
	fDisplayCacheMaster = nil;

	//LeDouble=nil;
	fWasClonedForPush = 0;
	fNumFieldParent = 0;
	fRecNumReserved = -1;
	fRemoteRecNum = -3;
	fRecNumPreAllocated = -1;
	RemoteAskedForReservedRecordNumber = false;
	fRecParent = nil;
	fToken = 0;
	fAlreadyAskedSeqNum = -2;
	fTokenIsValidated = false;
	fWasJustDeleted = false;

	isnew = true;
	isSaving = false;
	isModified = false;
	fCheckAllIntegrity = true;
	fMustClearIfReloadFromContext = true;

	asscrit = crit;
	/*
	if (crit != nil)
		crit->Retain();
		*/
	context = Context;
	nb=crit->GetNbCrit();
	maxnbcrit = nb;
	if (xdb->IsRemote())
	{
		fIsRemote = true;
		fIsIDGenerated = false;
		fRemoteSeqNum = -1;
		assocData = nil;

		if (crit->AtLeastOneSubTable())
		{
			NumFieldArray subfields;
			crit->CopySubTableDep(subfields);
			sLONG i,nb = subfields.GetCount();
			for (i=1; i<=nb && err == VE_OK; i++)
			{
				sLONG nc = subfields[i];
				Field *cri = crit->RetainField(nc);
				xbox_assert(cri != nil);
				if (cri != nil)
				{
					if (testAssert(cri->GetTyp() == DB4D_SubTable))
					{
						VSubTableDB4D* sub = new VSubTableDB4D;
						sub->InitForNewRecord(this, cri, Context);
						SetNthField(nc, sub, false);
					}
					cri->Release();
				}
			}
		}

	}
	else
	{
#if debugLeakCheck_NbLoadedRecords
		if (debug_candumpleaks)
			DumpStackCrawls();
		if (debug_canRegisterLeaks)
			RegisterStackCrawl(this);
#endif

		asscrit->GetDF()->IncNbLoadedRecords();
		if (crit->GetDF()->AcceptNotFullRecord())
		{
			assocData=new FicheOnDisk(xdb, crit->GetDF(), crit);
			if (assocData==nil)
			{
				err = ThrowError(memfull, DBaction_AllocatingRecordInMem);
			}
			else
			{
			}
		}
		else
			assocData = nil;
	}
#if debuglr == 11
	debug_fic_holder* dfic = new debug_fic_holder(this);
	debug_treerec.Add(dfic);
#endif

#if debugFicheOnDisk
	gDebugFiches.AddFiche(this);
#endif
}


FicheInMem::FicheInMem(BaseTaskInfo* Context, Table* crit, FicheOnDisk* ff)  // equivalent du chargehte
{
	StAllocateInCache	alloc;
	
	fIsUsedAsCacheInQuery = false;
	fAddressAllreadyTaken = false;
	fReadOnly = true;
	fIsRemote = false;
	fIsIDGenerated = false;

	fAllAlwaysNull = false;
	fIsASubRecordOnClient = false;
	fUsedAsSubRecordOnServer = false;

	fDejaRequested = false;
	fRequestedFields = nil;
	fRequestedFieldsMaster = nil;
	fDisplayCacheMaster = nil;
	fModificationStamp = 0;

	fLockingContextExtraData = nil;
	fKindOfLock = DB4D_LockedByNone;
	fRecNumReserved = -1;
	fNumFieldParent = 0;
	RemoteAskedForReservedRecordNumber = false;
	fRemoteRecNum = -3;
	fRecNumPreAllocated = -1;
	fWasClonedForPush = 0;
	fRecParent = nil;
	context = Context;
	fToken = 0;
	fAlreadyAskedSeqNum = -2;
	fTokenIsValidated = false;
	isnew = false;
	isSaving = false;
	isModified = false;
	fWasJustDeleted = false;
	//LeDouble=nil;
	asscrit = crit;
	fCheckAllIntegrity = true;
	fMustClearIfReloadFromContext = true;
	/*
	if (crit != nil)
		crit->Retain();
		*/
#if debugLeakCheck_NbLoadedRecords
	if (debug_candumpleaks)
		DumpStackCrawls();
	if (debug_canRegisterLeaks)
		RegisterStackCrawl(this);
#endif

	if (asscrit->GetDF() != nil)
		asscrit->GetDF()->IncNbLoadedRecords();
	assocData = ff;
	if (assocData != nil)
	{
		assocData->Retain();
		fModificationStamp = assocData->GetModificationStamp();
	}
	maxnbcrit=crit->GetNbCrit();
	
#if debuglr == 11
	debug_fic_holder* dfic = new debug_fic_holder(this);
	debug_treerec.Add(dfic);
#endif

#if debugFicheOnDisk
	gDebugFiches.AddFiche(this);
#endif
}


VError FicheInMem::CopyAllBlobs(FicheInMem* into, BaseTaskInfo* Context, bool ForPush)
{
	VError err = VE_OK;
	NumFieldArray blobs;
	asscrit->CopyBlobDep(blobs);

	if (blobs.GetCount() > 0)
	{
		for (NumFieldArray::Iterator cur = blobs.First(), end = blobs.End(); cur != end; cur++)
		{
			sLONG nc = *cur;
			ValPtr cv = GetNthField(nc, err);
			if (cv != nil)
			{
				ValPtr cv2 = (ValPtr)cv->FullyClone(ForPush);
				if (cv2 == nil)
					err = ThrowError(memfull, DBaction_BuildingValue);
				else
				{
					err = into->SetNthField(nc, cv2, true);
					if (err != VE_OK)
						delete cv2;
				}
			}
			if (err != VE_OK)
				break;
		}
	}

	return err;
}


FicheInMem* FicheInMem::CloneForPush(BaseTaskInfo* Context, VError& err) const
{
	FicheInMem* that = (FicheInMem*)this;

	FicheInMem* lacopy = Clone(Context, err, true);
	if (lacopy != nil)
	{
		if (!isnew)
			fWasClonedForPush++;
		if (!fReadOnly)
		{
			sLONG numrec = that->GetNum();
			if (numrec != -3)
			{
				asscrit->MarkRecordAsPushed(numrec, Context);
				if (Context != nil)
					Context->MarkRecordAsPushed(asscrit->GetDF(), numrec);
				if (!fIsRemote)
				{
					if (Context != nil)
						Context->RemoveRecFrom(asscrit->GetDF(), numrec);
				}
				that->fReadOnly = true;
			}
		}
	}
	return lacopy;
}


void FicheInMem::RestoreFromPop()
{
	isModified = true;
	fFieldsStamps.resize(maxnbcrit, 0);
	for (StampVector::iterator cur = fFieldsStamps.begin(), end = fFieldsStamps.end(); cur != end; cur++)
		(*cur)++;
	if (!fIsRemote)
	{
		if (assocData->getnumfic() != -3 && !fReadOnly)
		{
			xbox_assert((!isnew) && fWasClonedForPush > 0);
			VError err;
			FicheOnDisk* deja = GetDF()->LoadNotFullRecord(assocData->getnumfic(), err, DB4D_Keep_Lock_With_Record, context, false, nil, nil);
			if (deja != nil)
			{
#if 0
				assocData->SetNewInTrans(deja->IsNewInTrans());
				assocData->SetInTrans(deja->InTrans());
				assocData->SetAntelen(deja->GetAntelen());
				assocData->setaddr(deja->getaddr(), false);
				deja->Release();
#endif
				FicheOnDisk* oldassocData = assocData;
				assocData = deja;
				oldassocData->Release();

			}
			if (context != nil)
			{
				context->RemoveRecFrom(GetDF(), assocData->getnumfic(), false, true);
				context->PutRecIn(this);
			}

		}

		NumFieldArray blobs;
		asscrit->CopyBlobDep(blobs);

		if (blobs.GetCount() > 0)
		{
			VError err = VE_OK;
			for (NumFieldArray::Iterator cur = blobs.First(), end = blobs.End(); cur != end; cur++)
			{
				sLONG nc = *cur;
				ValPtr cv = GetNthField(-nc, err);
				if (cv != nil)
				{
					cv->RestoreFromPop((void*)context);
				}
			}
		}

		sLONG nbc = asscrit->GetNbCrit();
		for (sLONG i = 1; i <= nbc; i++)
		{
			VError err2;
			VValueSingle* cv = GetNthField(-i, err2);
			if (cv != nil)
			{
				if (cv->GetTrueValueKind() == VK_SUBTABLE)
				{
					cv->RestoreFromPop((void*)context);
				}
			}
		}


	}
	else
	{
		sLONG nbc = asscrit->GetNbCrit();
		for (sLONG i = 1; i <= nbc; i++)
		{
			VError err2;
			VValueSingle* cv = GetNthField(-i, err2);
			if (cv != nil)
			{
				if (cv->GetTrueValueKind() == VK_SUBTABLE)
				{
					cv->RestoreFromPop((void*)context);					
				}
			}
		}
	}

	if (!fReadOnly)
	{
		sLONG numrec = GetNum();
		if (numrec != -3)
		{
			asscrit->UnMarkRecordAsPushed(numrec, context);
			if (context != nil)
				context->UnMarkRecordAsPushed(asscrit->GetDF(), numrec);

		}
	}

	if (!isnew)
	{
		if (testAssert(fWasClonedForPush > 0))
			fWasClonedForPush--;
	}
	else
		xbox_assert(fWasClonedForPush == 0);
}


FicheInMem* FicheInMem::Clone(BaseTaskInfo* Context, VError& err, Boolean ForPush) const
{
	FicheInMem* lacopy = nil;
	err = VE_OK;

	FicheOnDisk* copydata;
	if (fIsRemote)
		copydata = nil;
	else
		copydata = assocData->Clone(err, ForPush);
	if (err == VE_OK)
	{
		if (fIsRemote)
		{
			lacopy = new FicheInMem();
			if (lacopy != nil)
			{
				lacopy->maxnbcrit = maxnbcrit;
				lacopy->asscrit = asscrit;
				lacopy->assocData = nil;
				lacopy->fRemoteRecNum = fRemoteRecNum;
				lacopy->context = Context;
				lacopy->fKindOfLock = fKindOfLock;
				lacopy->fIsASubRecordOnClient = fIsASubRecordOnClient;
				lacopy->fRemoteSeqNum = fRemoteSeqNum;

				lacopy->fIsIDGenerated = fIsIDGenerated;
				lacopy->fID = fID;

				fIsIDGenerated = false;
				fID.FromLong(0);

			}
		}
		else
			lacopy = new FicheInMem(Context, asscrit, copydata);

		QuickReleaseRefCountable(copydata);

		if (lacopy == nil)
		{
			err = ThrowError(memfull, DBaction_AllocatingRecordInMem);
		}
		else
		{
			if (ForPush)
			{
				lacopy->fFieldsStamps = fFieldsStamps;
				lacopy->fReadOnly = fReadOnly;
				lacopy->isnew = isnew;
				lacopy->isModified = isModified;
				if (isnew)
					lacopy->fWasClonedForPush = 0;
				else
					lacopy->fWasClonedForPush = fWasClonedForPush + 1;
			}
			else
			{
				lacopy->fReadOnly = false;
				lacopy->isnew = true;
			}

			if (fIsRemote)
				lacopy->fIsRemote = true;
			else
				err = ((FicheInMem*)this)->CopyAllBlobs(lacopy, Context, ForPush);
			if (err == VE_OK)
			{
				ChampVector::const_iterator cur, end;
				sLONG i;

				if (fIsRemote)
				{
					for (cur = fch.begin(), end = fch.end(), i = 1; cur != end; cur++, i++)
					{
						VValueSingle* cv = *cur;
						if (cv != nil)
						{
							VValueSingle* cv2 = lacopy->GetNthField(-i, err);
							if (cv2 == nil && err == VE_OK)
							{
								cv2 = (VValueSingle*)cv->FullyClone(ForPush);
								if (cv2 != nil)
								{
									if (cv2->GetTrueValueKind() == VK_SUBTABLE)
									{
										((VSubTableDB4D*)cv2)->SetParent(lacopy);
									}

									err = lacopy->SetNthField(i, cv2, true);
									if (err != VE_OK)
										delete cv2;
								}
								else
									err = ThrowError(memfull, DBaction_BuildingValue);
							}
						}
						if (err != VE_OK)
							break;
					}
				}
				else
				{
					for (i = 1; i<=maxnbcrit && err == VE_OK; i++)
					{
						VValueSingle* cv2 = lacopy->GetNthField(-i, err);
						if (cv2 == nil)
						{
							VValueSingle* cv = (const_cast<FicheInMem*>(this))->GetNthField(-i, err);
							if (cv == nil)
							{
								lacopy->GetNthField(i, err);
							}
							else
							{
								cv2 = (VValueSingle*)cv->FullyClone(ForPush);
								if (cv2 != nil)
								{
									if (cv2->GetTrueValueKind() == VK_SUBTABLE)
									{
										((VSubTableDB4D*)cv2)->SetParent(lacopy);
									}
									err = lacopy->SetNthField(i, cv2, true);
									if (err != VE_OK)
										delete cv2;
								}
							}
						}
					}
				}

				if (fIsRemote)
				{
					for (cur = fchold.begin(), end = fchold.end(), i = 1; cur != end; cur++, i++)
					{
						VValueSingle* cv = *cur;
						if (cv != nil)
						{
							VValueSingle* cv2;
							cv2 = (VValueSingle*)cv->FullyClone();
							if (cv2 != nil)
							{
								if (cv2->GetTrueValueKind() == VK_SUBTABLE)
								{
									((VSubTableDB4D*)cv2)->SetParent(lacopy);
								}
								err = lacopy->SetNthFieldOld(i, cv2, true);
								if (err != VE_OK)
									delete cv2;
							}
							else
								err = ThrowError(memfull, DBaction_BuildingValue);
						}
						if (err != VE_OK)
							break;
					}
				}
			}

			if (err != VE_OK)
			{
				lacopy->Release();
				lacopy = nil;
			}
		}
	}

	return lacopy;
}


FicheInMem* FicheInMem::CloneOnlyModifiedValues(BaseTaskInfo* Context, VError& err) const
{
	FicheInMem* lacopy = nil;
	err = VE_OK;

	lacopy = new FicheInMem(Context, asscrit, nil);
	if (lacopy == nil)
	{
		err = ThrowError(memfull, DBaction_AllocatingRecordInMem);
	}
	else
	{
		lacopy->isnew = true;

		ChampVector::const_iterator cur, end;
		sLONG i;
		for (cur = fch.begin(), end = fch.end(), i = 1; cur != end; cur++, i++)
		{
			VValueSingle* cv = *cur;
			if (cv != nil)
			{
				VValueSingle* cv2 = (VValueSingle*)cv->FullyClone();
				if (cv2 != nil)
				{
					err = lacopy->SetNthField(i, cv2, true);
					if (err != VE_OK)
						delete cv2;
				}
				else
					err = ThrowError(memfull, DBaction_BuildingValue);
			}
			if (err != VE_OK)
				break;
		}

		if (err != VE_OK)
		{
			lacopy->Release();
			lacopy = nil;
		}
	}

	return lacopy;
}


VError FicheInMem::RevertModifiedValues(BaseTaskInfo* Context, FicheInMem* From)
{
	VError err = VE_OK;
	sLONG i, nb = (sLONG)fch.size();

	for (i = 1; i<=nb; i++)
	{
		VValueSingle* cv = GetNthField(-i, err);
		if (cv != nil)
		{
			VValueSingle* cv2 = From->GetNthField(-i, err);
			if (cv2 == nil)
			{
				delete cv;
				SetNthField(i, nil);
			}
			else
			{
				if (cv->FromValueSameKind(cv2))
				{
					// rien a faire
				}
				else
					err = ThrowError(memfull, DBaction_BuildingValue);
			}
		}

	}
	return err;
}


void FicheInMem::ClearSeqNumToken(BaseTaskInfo* context)
{
	if (fToken != 0)
	{
		AutoSeqNumber* seq = asscrit->GetSeqNum(context == nil ? nil :context->GetEncapsuleur());
		if (seq != nil)
		{
			if (fTokenIsValidated)
				seq->ValidateValue(fToken, asscrit, context);
			else
				seq->InvalidateValue(fToken);
		}
		fToken = 0;
		fTokenIsValidated = false;
		fAlreadyAskedSeqNum = -2;
	}
}


void FicheInMem::TransferSeqNumToken( BaseTaskInfo* context, FicheInMem *inDestination)
{
	if (testAssert( inDestination != nil && inDestination != this))
	{
		inDestination->ClearSeqNumToken( context);

		inDestination->fToken = fToken;
		inDestination->fAlreadyAskedSeqNum = fAlreadyAskedSeqNum;
		inDestination->fTokenIsValidated = fTokenIsValidated;
		inDestination->fRemoteSeqNum = fRemoteSeqNum;

		fToken = 0;
		fAlreadyAskedSeqNum = -2;
		fTokenIsValidated = false;
		fRemoteSeqNum = -1;
	}
}


VError FicheInMem::Detach(BaseTaskInfo* Context, Boolean BlobFieldsCanBeEmptied)
{
	VError err;
		
	if (fIsRemote)
	{
		err = VE_OK;

		// L.E. 06/08/08 ACI0058505 load all fields before detach
		if (fchold.size() > fch.size())
		{
			fch.resize(fchold.size(),nil);
		}

		if (err == VE_OK)
		{
			for( sLONG i = (sLONG)fchold.size()-1; i >= 0 ; --i)
			{
				ValPtr chp_old = fchold[i];
				if (chp_old != nil)
				{
					if (fch[i] == nil)
					{
						//fch[i] = chp_old;
						fch[i] = chp_old->Clone();
					}
					/*
					else
						delete chp_old;
					fchold[i] = nil;
					*/
				}
			}
		}
		

		if (fIsIDGenerated && !fReadOnly)
			VDBMgr::GetManager()->AddReleasedRecID(fID, context);

		isnew = true;
		isModified = false;
		fRemoteRecNum = -3;
		fRecNumReserved = -1;
		fReadOnly = false;
		fIsIDGenerated = false;
		VError errsub;
		if (asscrit->AtLeastOneSubTable())
		{
			NumFieldArray subfields;
			asscrit->CopySubTableDep(subfields);
			sLONG i,nb = subfields.GetCount();
			for (i=1; i<=nb; i++)
			{
				sLONG nc = subfields[i];
				ValPtr cv = GetNthField(nc, errsub);
				if (testAssert(cv != nil))
				{
					xbox_assert(cv->GetTrueValueKind() == VK_SUBTABLE);
					VSubTableDB4D* sub = (VSubTableDB4D*)cv;
					sub->Detach();
				}
			}
		}

		fRemoteSeqNum = -1;
		fIsIDGenerated = false;

		if (asscrit->AtLeastOneAutoSeqField())
		{
			NumFieldArray deps;
			asscrit->CopyAutoSeqDep(deps);
			sLONG nb = deps.GetCount();
			for (sLONG i=1; i <= nb && err == VE_OK; i++)
			{
				sLONG x = deps[i];

				ValPtr cv = GetNthField(x, err);
				if (cv != nil)
				{
					cv->FromLong8(GetAutoSeqValue());
					Touch(x);
				}
			}
		}

		if (asscrit->AtLeastOneAutoGenerateField())
		{
			NumFieldArray deps;
			asscrit->CopyAutoGenerateDep(deps);
			sLONG nb = deps.GetCount();
			for (sLONG i = 1; i <= nb && err == VE_OK; i++)
			{
				sLONG x = deps[i];
				ValPtr cv = GetNthField(x, err);
				if (cv != nil && cv->GetValueKind() == VK_UUID)
				{
					VUUID* vuid = dynamic_cast<VUUID*>(cv);
					if (testAssert(vuid))
					{
						vuid->Regenerate();
						Touch(x);
					}
				}
			}
		}
	}
	else
	{
		FicheOnDisk* newdata = assocData->Clone(err);

		if (err == VE_OK)
		{
			VError errsub;
			if (asscrit->AtLeastOneSubTable())
			{
				NumFieldArray subfields;
				asscrit->CopySubTableDep(subfields);
				sLONG i,nb = subfields.GetCount();
				for (i=1; i<=nb; i++)
				{
					sLONG nc = subfields[i];
					{
						{
							ValPtr cv = GetNthField(nc, errsub);
							if (testAssert(cv != nil))
							{
								xbox_assert(cv->GetTrueValueKind() == VK_SUBTABLE);
								VSubTableDB4D* sub = (VSubTableDB4D*)cv;
								sub->Detach();
							}
						}
					}
				}
			}

			NumFieldArray blobs;
			asscrit->CopyBlobDep(blobs);

			if (blobs.GetCount() > 0)
			{
				for (NumFieldArray::Iterator cur = blobs.First(), end = blobs.End(); cur != end; cur++)
				{
					sLONG nc = *cur;
					ValPtr cv = GetNthField(nc, err);
					if (cv != nil)
					{
						cv->Detach(BlobFieldsCanBeEmptied);
						Touch(nc);
					}
				}
			}
			
			
			if (fToken != 0)
			{
				AutoSeqNumber* seq = asscrit->GetSeqNum(Context->GetEncapsuleur());
				if (seq != nil)
				{
					if (fTokenIsValidated)
						seq->ValidateValue(fToken, asscrit, Context);
					/*
					else
						seq->InvalidateValue(fToken);
						*/
				}
				fToken = 0;
				fTokenIsValidated = false;
				fAlreadyAskedSeqNum = -2;
			}
			

			UnLockRecord();
			FicheOnDisk* oldassocData = assocData;
			assocData = newdata;
			oldassocData->Release();
			isnew = true;
			fReadOnly = false;

			if (asscrit->AtLeastOneAutoSeqField())
			{
				NumFieldArray deps;
				asscrit->CopyAutoSeqDep(deps);
				sLONG nb = deps.GetCount();
				for (sLONG i=1; i <= nb && err == VE_OK; i++)
				{
					ValPtr cv = GetNthField(deps[i], err);
					if (cv != nil)
					{
						AutoSeqNumber* seq = asscrit->GetSeqNum(nil);
						if (seq != nil)
						{
							sLONG8 val = seq->GetNewValue(fToken);
							if (val != -1)
								cv->FromLong8(val);
						}
					}
				}
			}

			if (asscrit->AtLeastOneAutoGenerateField())
			{
				NumFieldArray deps;
				asscrit->CopyAutoGenerateDep(deps);
				sLONG nb = deps.GetCount();
				for (sLONG i = 1; i <= nb && err == VE_OK; i++)
				{
					sLONG x = deps[i];
					ValPtr cv = GetNthField(x, err);
					if (cv != nil && cv->GetValueKind() == VK_UUID)
					{
						VUUID* vuid = dynamic_cast<VUUID*>(cv);
						if (testAssert(vuid) != nil)
						{
							vuid->Regenerate();
							Touch(x);
						}
					}
				}
			}

		}
	}


	return err;
}


VError FicheInMem::FillAllFieldsEmpty()
{
	sLONG i;
	VError err = VE_OK;

	for (i=1; i<=maxnbcrit && err == VE_OK; i++)
	{
		ValPtr cv = GetNthField(i, err);
	}

	return err;
}


void FicheInMem::WhoLockedIt(DB4D_KindOfLock& outLockType, const VValueBag **outLockingContextRetainedExtraData) const
{
	if (IsNew())
	{
		outLockType = DB4D_LockedByNone;
		if (outLockingContextRetainedExtraData != nil)
			*outLockingContextRetainedExtraData = nil;
	}
	else
	{
		if (fReadOnly)
		{
			outLockType = fKindOfLock;
			if (outLockingContextRetainedExtraData != nil)
				CopyRefCountable(outLockingContextRetainedExtraData, fLockingContextExtraData);
		}
		else
		{
			outLockType = DB4D_LockedByRecord;
			if (outLockingContextRetainedExtraData != nil && context != nil)
				*outLockingContextRetainedExtraData = context->RetainExtraData();
		}

	}
}



void FicheInMem::UnLockRecord()
{
	if (testAssert(!fIsRemote))
	{
		if (!fWasJustDeleted)
		{
			if (assocData!=nil)
			{
				if (!fReadOnly && !isnew)
				{
					if (context != nil)
					{
						context->RemoveRecFrom(GetDF(), assocData->getnumfic());
					}
					if (fWasClonedForPush == 0 && !assocData->InTrans())
					{
						GetDF()->UnlockRecord(assocData->getnumfic(), context, DB4D_Keep_Lock_With_Record);
					}
				}
		#if debuglr == 12
				if (context != nil)
					context->Debug_LookForRec(this);
		#endif
			}
		}
	}
}


FicheInMem::~FicheInMem()  // delmem
{
	sLONG i,nb;
	ValPtr chp;
	StAllocateInCache	alloc;
	/*
	if (LeDouble!=nil)
	{
		delete LeDouble;
		LeDouble=nil;
	}
	*/
	
#if debugFicheOnDisk
	gDebugFiches.RemoveFiche(this);
#endif

#if debuglr == 11
	debug_fic_holder dfic(this);
	debug_treerec.Remove(&dfic);
#endif

	if (fIsRemote && fIsIDGenerated && !fReadOnly)
		VDBMgr::GetManager()->AddReleasedRecID(fID, context);

	if (fRecNumReserved != -1 && !fIsRemote)
	{
		DataTable* df = asscrit->GetDF();
		if (df != nil)
		{
			df->LockWrite();
			OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
			df->LibereAddrRec(curstack, fRecNumReserved, nil);
			df->Unlock();
		}
	}

	if (fRequestedFields != nil)
		fRequestedFields->Release();

	if (fToken != 0)
	{
		xbox_assert(!fIsRemote);
		AutoSeqNumber* seq = asscrit->GetSeqNum(nil);
		if (seq != nil)
		{
			if (fTokenIsValidated)
				seq->ValidateValue(fToken, asscrit, context);
			else
				seq->InvalidateValue(fToken);
		}
	}

	nb = RelDeps.GetCount();
	for (i = 0; i < nb; i++)
	{
		delete RelDeps[i];
	}

	nb=(sLONG)fch.size();
	for (i=0;i<nb;i++)
	{
		chp=fch[i];
		if (chp!=nil)
		{
			delete chp;
		}
	}
	//ch.SetAllocatedSize(0);

	nb=(sLONG)fchold.size();
	for (i=0;i<nb;i++)
	{
		chp=fchold[i];
		if (chp!=nil)
		{
			delete chp;
		}
	}

	nb=(sLONG)fchForQuery.size();
	for (i=0;i<nb;i++)
	{
		chp=fchForQuery[i];
		if (chp!=nil)
		{
			delete chp;
		}
	}

	nb=(sLONG)fcholdForQuery.size();
	for (i=0;i<nb;i++)
	{
		chp=fcholdForQuery[i];
		if (chp!=nil)
		{
			delete chp;
		}
	}

	//chold.SetAllocatedSize(0);
	if (assocData != nil)
	{
		xbox_assert(!fIsRemote);
		UnLockRecord();
		FicheOnDisk* oldassocData = assocData;
		assocData = nil;
		oldassocData->Release();
	}

	if (asscrit != nil && !fIsRemote)
	{
#if debugLeakCheck_NbLoadedRecords
		UnRegisterStackCrawl(this);
#endif
		if (asscrit->GetDF() != nil)
			asscrit->GetDF()->DecNbLoadedRecords();
		//asscrit->Release();
	}

	ReleaseRefCountable(&fLockingContextExtraData);
	//QuickReleaseRefCountable(fDisplayCacheMaster);

}


void FicheInMem::SetAssocData(FicheOnDisk* newdata)
{
	xbox_assert(!fIsRemote);
	if (newdata != assocData)
	{
		sLONG nb, i;
		ValPtr chp;

		nb = RelDeps.GetCount();
		for (i = 0; i < nb; i++)
		{
			delete RelDeps[i];
		}
		RelDeps.SetCount(0);

		nb=(sLONG)fch.size();
		for (i=0;i<nb;i++)
		{
			chp=fch[i];
			if (chp!=nil)
			{
				delete chp;
			}
		}
		fch.resize(0);
		//ch.SetAllocatedSize(0);

		nb=(sLONG)fchold.size();
		for (i=0;i<nb;i++)
		{
			chp=fchold[i];
			if (chp!=nil)
			{
				delete chp;
			}
		}
		fchold.resize(0);

		FicheOnDisk* oldassocData = assocData;
		assocData = newdata;

		if (assocData != nil)
			assocData->Retain();

		if (oldassocData != nil)
		{
			oldassocData->Release();
		}
	}
}

Boolean FicheInMem::SetSaving()
{
	//occupe();
	Boolean res = !isSaving;
	isSaving = true;
	//libere();
	return res;
}

void FicheInMem::ClearSaving()
{
	//occupe();
	isSaving = false;
	isModified = false;
	//libere();
}


#if debuglr == 111
sLONG		FicheInMem::Retain(const char* DebugInfo) const
{
	return IDebugRefCountable::Retain(DebugInfo);
}


sLONG		FicheInMem::Release(const char* DebugInfo) const
{
	return IDebugRefCountable::Release(DebugInfo);
}
#endif


DataTable* FicheInMem::GetDF(void)
{ 
	return(asscrit->GetDF()); 
}


sLONG FicheInMem::GetNum(void) 
{ 
	if (RemoteAskedForReservedRecordNumber)
	{
		xbox_assert(fIsRemote);
		if (fRecNumReserved == -1 && fRemoteRecNum < 0)
		{
			StErrorContextInstaller errors(false);
			VError err;
			IRequest *req = asscrit->GetOwner()->CreateRequest( context == nil ? nil : context->GetEncapsuleur(), Req_ReserveRecordNumber + kRangeReqDB4D);
			if (req == nil)
			{
				err = ThrowBaseError(memfull, noaction);
			}
			else
			{
				req->PutBaseParam(asscrit->GetOwner());
				req->PutThingsToForget( VDBMgr::GetManager(), context);
				req->PutTableParam(asscrit);
				req->PutFicheInMemMinimalParam(this, context == nil ? nil : context->GetEncapsuleur());
				err = req->GetLastError();
				if (err == VE_OK)
				{
					err = req->Send();
					if (err== VE_OK)
						err = req->GetUpdatedInfo(asscrit->GetOwner(), context);

					if (err == VE_OK)
					{
						fRecNumReserved = req->GetLongReply(err);
						if (err != VE_OK)
							fRecNumReserved = -1;
					}
				}
				req->Release();
			}
			RemoteAskedForReservedRecordNumber = false;
		}
		
	}
	if (fRecNumReserved == -1)
	{
		if (fIsRemote)
		{
			return fRemoteRecNum;
		}
		else
		{
			return assocData->getnumfic();
		}
	}
	else
		return fRecNumReserved;
};


Boolean FicheInMem::IsFieldValid(const Field* cri, sLONG* ncrit)
{
	sLONG n = -1;
	Boolean ok = false;

	if (cri != nil && cri->GetOwner() == asscrit)
	{
		n = cri->GetPosInRec();
		ok = true;
	}

	if (ncrit != nil)
		*ncrit = n;

	return ok;
}


uBOOL FicheInMem::IsModif(const Field* cri)
{
	sLONG n;
	if (isnew)
	{
		return(true);
	}
	else
	{
		return GetFieldModificationStamp(cri) != 0;
	}
}


uBOOL FicheInMem::IsModif(sLONG n)
{
	if (isnew)
	{
		return(true);
	}
	else
	{
		return GetFieldModificationStamp(n) != 0;
	}
}


ValPtr FicheInMem::GetFieldValue(const EntityAttribute* att, VError& err, bool touch)
{
	ValPtr chp = nil;
	sLONG n;
	Field* cri = nil;

	if (att != nil)
	{
		LocalEntityModel* localModel = dynamic_cast<LocalEntityModel*>(att->GetModel());
		if (localModel != nil)
		{
			cri = localModel->RetainField(att, true);
		}
	}

	if (IsFieldValid(cri, &n))
	{
		chp = GetNthField(n, err);
		if (touch)
			Touch(n);
	}
	else
		err = ThrowError(VE_DB4D_WRONGFIELDREF_IN_RECORD, DBactionFinale);

	return chp;
}



ValPtr FicheInMem::GetFieldValue(const Field* cri, VError& err, bool pictAsRaw, bool forQuery)
{
	ValPtr chp = nil;
	sLONG n;
	
	if (IsFieldValid(cri, &n))
	{
		chp = GetNthField(n, err, pictAsRaw, forQuery);
	}
	else
		err = ThrowError(VE_DB4D_WRONGFIELDREF_IN_RECORD, DBactionFinale);
	
	return chp;
}


ValPtr FicheInMem::GetFieldOldValue(const Field* cri, VError& err, bool forQuery)
{
	ValPtr chp = nil;
	sLONG n;

	if (IsFieldValid(cri, &n))
	{
		chp = GetNthFieldOld(n, err, forQuery);
	}
	else
		err = ThrowError(VE_DB4D_WRONGFIELDREF_IN_RECORD, DBactionFinale);

	return chp;
}



uLONG FicheInMem::GetFieldModificationStamp( sLONG inFieldID)
{
	if (inFieldID > 0 && inFieldID <= fFieldsStamps.size())
		return fFieldsStamps[inFieldID-1];
	else
		return 0;
}


uLONG FicheInMem::GetFieldModificationStamp( const Field* inField)
{
	if (inField == nil)
		return 0;
	else
		return GetFieldModificationStamp(inField->GetPosInRec());
}


VError FicheInMem::GetFieldModificationStamps(StampVector& outStamps)
{
	outStamps = fFieldsStamps;
	return VE_OK;
}



//#include <float.h>
/*
_isnan()

#include <cmath>
std::isnan()
*/


#if COMPIL_VISUAL
static inline int IsNaN(double a) { return _isnan(a);}
#else
#if __cplusplus < 201103L // not a C++11 compiler
static inline int IsNaN(double a) { return isnan(a);}
#else
static inline int IsNaN(double a) { return ::isnan(a);} // force use of the C99 function and not the macros
#endif
#endif

void FicheInMem::Touch(sLONG inFieldID)
{
	VError err;
	isModified = true;
	if (inFieldID > 0 && inFieldID <= maxnbcrit)
	{
		ValPtr cv = GetNthField(-inFieldID, err);
		if (cv != nil)
		{
			ValueKind cvkind = cv->GetValueKind();
			if (cvkind == VK_REAL)
			{
				if (!cv->IsNull())
				{
					VReal* vr = (VReal*)cv;
					if (IsNaN(vr->Get()))
						cv->SetNull(true);
				}
			}
			if (cvkind == VK_STRING || cvkind == VK_STRING_UTF8)
			{
				sLONG limit = asscrit->GetLimitingLen(inFieldID);
				if (limit>0)
					((VString*)cv)->Truncate(limit);
			}

			if (cvkind == VK_BLOB_OBJ)
			{
#if debuglr
				VBlobObj* cvobj = dynamic_cast<VBlobObj*>(cv);
				xbox_assert(cvobj != nil);
#else
				VBlobObj* cvobj = (VBlobObj*)cv;
#endif
				cvobj->SetObjDirty();

			}

#if debuglr == 111
			if (cv->GetValueKind() == VK_STRING || cv->GetValueKind() == VK_TEXT)
			{
				xbox_assert( ((VString*)cv)->IsStringValid() );
			}
#endif

		}
		Boolean ok = true;
		if (inFieldID > fFieldsStamps.size())
		{
			fFieldsStamps.resize(maxnbcrit, 0);
		}
		if (ok)
			fFieldsStamps[inFieldID-1]++;
	}
	if (fRecParent != nil)
	{
		fRecParent->Touch((sLONG)0);
		if (fIsRemote)
			fRecParent->Touch(fNumFieldParent);
	}
}


void FicheInMem::Touch(const Field* inField)
{
	isModified = true;
	if (inField != nil)
		Touch(inField->GetPosInRec());
}


VError FicheInMem::SetNthField(sLONG n, ValPtr cv, Boolean AutoMaxField)
{
	VError err = VE_OK;

	xbox_assert(n <= maxnbcrit);

	if (n>fch.size())
	{
		sLONG n2 = 0;
		if (AutoMaxField)
			n2 = maxnbcrit;
		if (n>n2)
			n2 = n;

		fch.resize(n2,nil);
	}

	if (err == VE_OK)
		fch[n-1] = cv;

	return err;
}



VError FicheInMem::SetNthFieldOld(sLONG n, ValPtr cv, Boolean AutoMaxField)
{
	VError err = VE_OK;

	xbox_assert(n <= maxnbcrit);

	if (n>fchold.size())
	{
		sLONG n2 = 0;
		if (AutoMaxField)
			n2 = maxnbcrit;
		if (n>n2)
			n2 = n;

		fchold.resize(n2,nil);
	}

	if (err == VE_OK)
		fchold[n-1] = cv;

	return err;
}


VError FicheInMem::GetNthFieldBlob(sLONG n, VBlob& outBlob, Boolean CanCacheData)
{
	VError err = VE_OK;
	Field* cri=asscrit->RetainField(n);
	sLONG* curp;

	if (cri != nil)
	{
		if (cri->GetTyp() == VK_BLOB_DB4D)
		{
			ValPtr chp = nil;
			if (n <= fch.size())
			{
				chp = fch[n-1];
			}

			if (chp == nil && fIsRemote)
			{
				if (n > fchold.size())
					chp = nil;
				else
					chp = fchold[n-1];
				if (chp != nil)
				{
					chp = (VValueSingle*)chp->FullyClone();
					if (chp == nil)
					{
						err = ThrowError(memfull, noaction);
					}
					else
					{
						fch[n-1] = chp;
					}

				}
				else
				{
					if (cri != nil)
					{
						chp = (VValueSingle*)VValue::NewValueFromValueKind(cri->GetTyp());
						if (chp == nil)
							err = ThrowError(memfull, noaction);
						else
						{
							chp->SetNull(!cri->IsNeverNull());
							fch[n-1] = chp;
						}
					}
				}
			}

			if (chp == nil && !fIsRemote)
			{
				if (assocData == nil)
					curp = nil;
				else
					curp=(sLONG*)assocData->GetDataPtr(n);
				if (curp == nil)
				{
					if (cri->IsNeverNull())
						err = outBlob.SetSize(0);
					else
						outBlob.SetNull( true);
				}
				else
				{
					sLONG numblob = *curp;
					if (assocData->IsFieldNull( n))
					{
						if (cri->IsNeverNull())
							err = outBlob.SetSize(0);
						else
							outBlob.SetNull( true);
					}
					else if (numblob == -1)
					{
						err = outBlob.SetSize(0);
					}
					else
					{
						BlobWithPtr* blob = (BlobWithPtr*)(asscrit->GetDF()->LoadBlob( numblob, &CreBlobWithPtr, false, err, nil));
						if (err == VE_OK)
						{
							if (blob != nil)
							{
								// L.E. PutData may need a SetSize() to shrink the blob. I just added VBlob::FromData.
								err = outBlob.FromData(blob->GetDataPtr(), blob->GetDataLen());
								blob->Release();
							}
							else
							{
								err = outBlob.SetSize(0);
							}
						}
					}
				}

			}
			
			if (chp != nil)
			{
				if (chp->IsNull())
				{
					if (cri->IsNeverNull())
						err = outBlob.SetSize(0);
					else
						outBlob.SetNull( true);
				}
				else
				{
					BlobWithPtr* blob =((VBlob4DWithPtr*)chp)->GetBlob4D();
					if (blob != nil)
					{
						err = outBlob.PutData(blob->GetDataPtr(), blob->GetDataLen(), 0);
					}
					else
					{
						err = outBlob.SetSize(0);
					}
				}
			}
		}
		else
			err = ThrowError(VE_DB4D_WRONGFIELDREF_IN_RECORD, DBactionFinale);
		cri->Release();
	}
	else
		err = ThrowError(VE_DB4D_WRONGFIELDREF_IN_RECORD, DBactionFinale);

	return err;
}


VSize FicheInMem::PossibleUsedDiskSize()
{
	VSize tot = 0;
	for (ChampVector::iterator cur = fch.begin(), end = fch.end(); cur != end; cur++)
	{
		ValPtr cv = *cur;
		if (cv != nil)
		{
			ValueKind typ = cv->GetValueKind();
			if (typ == VK_TEXT || typ == VK_BLOB || typ == VK_IMAGE)
				tot = tot + cv->GetFullSpaceOnDisk();
		}
	}
	tot = tot + ((VSize)maxnbcrit * 128);
	tot = tot + ((VSize)asscrit->GetIndexDep().GetCount() * 2048);
	return tot;
}


void FicheInMem::RevertToReadOnly(BaseTaskInfo* context)
{
	xbox_assert(!fIsRemote);
	asscrit->GetDF()->UnlockRecord(GetNum(), context, DB4D_Do_Not_Lock);
	fReadOnly = true;
}


void FicheInMem::ClearModifiedFields()
{
	for (ChampVector::iterator cur = fch.begin(), end = fch.end(); cur != end; cur++)
	{
		ValPtr cv = *cur;
		if (cv != nil)
		{
			delete cv;
			*cur = nil;
		}
	}
	isModified = false;
	fFieldsStamps.resize(0);
}


void FicheInMem::GetOldPrimKeyValue(VectorOfVValue& outKeyval)
{
	VError err = VE_OK;
	FieldArray primkeyfields;
	asscrit->RetainPrimaryKey(primkeyfields);
	if (primkeyfields.GetCount() > 0)
	{
		for (FieldArray::Iterator cur = primkeyfields.First(), end = primkeyfields.End(); cur != end; cur++)
		{
			VValueSingle* cv = GetFieldOldValue(*cur, err);
			if (cv != nil)
				cv = cv->Clone();
			outKeyval.push_back(cv);
		}
	}
	outKeyval.SetAutoDispose(true);
	for (FieldArray::Iterator cur = primkeyfields.First(), end = primkeyfields.End(); cur != end; cur++)
		(*cur)->Release();
}


VError FicheInMem::SetSyncInfo(Sync_Action action, BaseTaskInfo* context, VectorOfVValue* oldkey, uBOOL wasnew)
{
	VError err = VE_OK;
	if (asscrit->HasSyncInfo())
	{
		FieldArray primkeyfields;
		asscrit->RetainPrimaryKey(primkeyfields);
		if (primkeyfields.GetCount() > 0)
		{
			VectorOfVValue values(false);

			bool diff = false;

			VectorOfVValue::iterator curoldval;
			if (oldkey != nil)
				curoldval = oldkey->begin();

			for (FieldArray::Iterator cur = primkeyfields.First(), end = primkeyfields.End(); cur != end && err == VE_OK; cur++)
			{
				VValueSingle* cv;
				if (action == Sync_Update)
				{
					cv =  GetFieldValue(*cur, err);
					if (oldkey == nil)
						diff = true;
					else
					{
						const VValueSingle* cvold = *curoldval;
						if (!cv->EqualToSameKind(cvold, true))
							diff = true;
					}
				}
				else
					cv = GetFieldOldValue(*cur, err);
				values.push_back(cv);
				if (oldkey != nil)
					curoldval++;
			}

			if (err == VE_OK)
			{
				uLONG8 newSyncStamp = asscrit->GetDF()->GetNewRecordSync();
				SynchroBaseHelper* syncHelp = asscrit->GetOwner()->GetSyncHelper();
				uLONG8 timestamp;
				VTime aTime;
				aTime.FromSystemTime();
				timestamp = aTime.GetStamp();

				if (diff && !wasnew)
				{
					err = syncHelp->SetSynchro(asscrit, *oldkey, newSyncStamp, timestamp, Sync_Delete, context);
					if (err == VE_OK)
						newSyncStamp = asscrit->GetDF()->GetNewRecordSync();
				}
				if (err == VE_OK)
					err = syncHelp->SetSynchro(asscrit, values, newSyncStamp, timestamp, action, context);
			}
			
			for (FieldArray::Iterator cur = primkeyfields.First(), end = primkeyfields.End(); cur != end; cur++)
				(*cur)->Release();
		}
	}
	return err;
}


uLONG8 FicheInMem::GetSyncInfo() const
{
	uLONG8 SyncStamp = 0;
	VError err = VE_OK;
	if (asscrit->HasSyncInfo())
	{
		FieldArray primkeyfields;
		asscrit->RetainPrimaryKey(primkeyfields);
		if (primkeyfields.GetCount() > 0)
		{
			vector<VValueSingle*> values(false);

			for (FieldArray::Iterator cur = primkeyfields.First(), end = primkeyfields.End(); cur != end && err == VE_OK; cur++)
			{
				VValueSingle* cv = ((FicheInMem*)(this))->GetFieldValue(*cur, err);
				values.push_back(cv);
			}
			SynchroBaseHelper* syncHelp = asscrit->GetOwner()->GetSyncHelper();

			VTime timestamp;
			Sync_Action action;

			err = syncHelp->GetSynchroStamp(asscrit, values, SyncStamp, timestamp, action, context);
		}

		for (FieldArray::Iterator cur = primkeyfields.First(), end = primkeyfields.End(); cur != end; cur++)
			(*cur)->Release();
	}
	return SyncStamp;
}


VError FicheInMem::GetFullSyncInfo(uLONG8& outSyncstamp, VTime& outTimestamp, Sync_Action& outAction) const
{
	VError err = VE_OK;
	if (asscrit->HasSyncInfo())
	{
		FieldArray primkeyfields;
		asscrit->RetainPrimaryKey(primkeyfields);
		if (primkeyfields.GetCount() > 0)
		{
			vector<VValueSingle*> values(false);

			for (FieldArray::Iterator cur = primkeyfields.First(), end = primkeyfields.End(); cur != end && err == VE_OK; cur++)
			{
				VValueSingle* cv = ((FicheInMem*)(this))->GetFieldValue(*cur, err);
				values.push_back(cv);
			}
			SynchroBaseHelper* syncHelp = asscrit->GetOwner()->GetSyncHelper();

			VTime timestamp;
			Sync_Action action;

			err = syncHelp->GetSynchroStamp(asscrit, values, outSyncstamp, outTimestamp, outAction, context);
		}

		for (FieldArray::Iterator cur = primkeyfields.First(), end = primkeyfields.End(); cur != end; cur++)
			(*cur)->Release();
	}
	return err;
}


void FicheInMem::PrepareForCacheDisplay(FieldsForCache* RequestedFieldsMaster, sLONG recnum, BaseTaskInfo* xcontext, Table* inTable, RemoteRecordCache* DisplayCacheMaster)
{
	fIsRemote = true;
	fRequestedFieldsMaster = RequestedFieldsMaster;
	fRemoteRecNum = recnum;
	fReadOnly = true;
	context = xcontext;
	asscrit = inTable;
	maxnbcrit = asscrit->GetNbCrit();
	assocData = nil;
	//fDisplayCacheMaster = RetainRefCountable(DisplayCacheMaster);
	fDisplayCacheMaster = DisplayCacheMaster;
}


VError FicheInMem::CheckNthFieldRemote(sLONG n)
{
	VError err = VE_OK;
	xbox_assert(fIsRemote);
	if (n != 0)
	{
		if (fRequestedFieldsMaster != nil && fRequestedFields == nil)
		{
			fRequestedFields = fRequestedFieldsMaster->RetainFieldsForTable(asscrit);
		}

		if (fRequestedFields != nil)
		{
			if (n < 0)
				n = -n;
			if (!fRequestedFields->IsFieldIn(n))
			{
				fRequestedFields->AddField(n);
				fRequestedFieldsMaster->Touch();
				Boolean mustreload = true;
				if (n<=fch.size() && fch[n-1] != nil)
					mustreload = false;
				if (mustreload)
				{
					if (n<=fchold.size() && fchold[n-1] != nil)
						mustreload = false;
				}
				if (mustreload)
				{
					if (fDejaRequested)
						mustreload = false;
				}

				if (mustreload)
				{
					if (fRemoteRecNum < 0)
					{
						RemoteRecordCache* rrc = fDisplayCacheMaster;
						FicheInMem* fiche = rrc->RetainTrueRelatedRecord(asscrit, err);
						if (fiche != nil)
						{
							if (fiche->fchold.size() > fchold.size())
								fchold.resize(fiche->fchold.size(), nil);
							sLONG nb = (sLONG)fchold.size();
							for (sLONG i = 0; i < nb; i++)
							{
								if (fchold[i] == nil)
								{
									fchold[i] = (VValueSingle*)(fiche->fchold[i]);
									fiche->fchold[i] = nil;
								}
							}
							fiche->Release();
						}
					}
					else
					{
						IRequest* req = asscrit->GetOwner()->CreateRequest(context->GetEncapsuleur(), Req_LoadRecord + kRangeReqDB4D);
						if (req == nil)
							err = ThrowBaseError(memfull, noaction);
						else
						{
							fDejaRequested = true;
							req->PutBaseParam(asscrit->GetOwner());
							req->PutThingsToForget( VDBMgr::GetManager(), context);
							req->PutTableParam(asscrit);
							req->PutLongParam(fRemoteRecNum);

							uLONG loadOptions = 2;	// means we want read-only
								loadOptions |= 4;
							req->GetOutputStream()->Put( loadOptions);

							err = req->GetLastError();

							if (err == VE_OK)
							{
								err = req->Send();
								if (err== VE_OK)
									err = req->GetUpdatedInfo(asscrit->GetOwner(), context);
								if (err == VE_OK)
								{
									FicheInMem* fiche = req->RetainFicheInMemReply(asscrit, err, context->GetEncapsuleur());
									if (fiche != nil)
									{
										if (fiche->fchold.size() > fchold.size())
											fchold.resize(fiche->fchold.size(), nil);
										sLONG nb = (sLONG)fchold.size();
										for (sLONG i = 0; i < nb; i++)
										{
											if (fchold[i] == nil)
											{
												fchold[i] = (VValueSingle*)(fiche->fchold[i]);
												fiche->fchold[i] = nil;
											}
										}
										fiche->Release();
									}
								}
							}
							req->Release();
						}
					}

				}
			}
		}
	}
	return err;
}



ValPtr FicheInMem::GetNthField(sLONG n, VError& err, bool pictAsRaw, bool forQuery)
{
	ValPtr chp = nil;
	Field* cri;
	UniChar* curp;
	CreVValue_Code Code;
	err = VE_OK;
	StAllocateInCache	alloc;
	creVValue_ParamType CreVValue_Params = creVValue_default;
	if (forQuery)
		CreVValue_Params = CreVValue_Params | creVValue_forquery;
	if (fIsRemote)
		err = CheckNthFieldRemote(n);

	if (n == 0 || err != VE_OK)
	{
		chp=&fValueRecNum;
		fValueRecNum.FromLong(GetNum());
	}
	else
	{
		if (n<0)
		{
			n=-n;
			if (n>fch.size())
			{
				chp=nil;
			}
			else
			{
				chp=fch[n-1];
			}
		}
		else
		{
			xbox_assert(n <= maxnbcrit);

			if (n>fch.size())
			{
				sLONG n2 = maxnbcrit;
				if (n>n2)
					n2 = n;

				fch.resize(n2,nil);
			}
			
			if (err == VE_OK)
			{
				chp=fch[n-1];

				if (chp == nil && fIsRemote)
				{
					if (n > fchold.size())
						chp = nil;
					else
						chp = fchold[n-1];
					if (chp != nil)
					{
						bool bIsNull = chp-> IsNull ( ); /* Sergiy - March 31, 2008 - For ACI0056748 */
						chp = (VValueSingle*)chp->FullyClone();
						if (chp == nil)
						{
							err = ThrowError(memfull, noaction);
						}
						else
						{
							chp-> SetNull ( bIsNull ); /* Sergiy - March 31, 2008 - For ACI0056748 */
							fch[n-1] = chp;
						}

					}
					else
					{
						cri=asscrit->RetainField(n);
						if (cri != nil)
						{
							chp = (VValueSingle*)VValue::NewValueFromValueKind(cri->GetTyp());
							if (chp == nil)
								err = ThrowError(memfull, noaction);
							else
							{
								if (cri->GetTyp() == VK_UUID && cri->GetAutoGenerate())
								{
									((VUUID*)chp)->Regenerate();
									Touch(n);
								}
								else
								{
									if (cri->GetAutoSeq())
									{
										chp->FromLong8(GetAutoSeqValue());
										//Touch(n);
									}
									else
										chp->SetNull(!cri->IsNeverNull());
								}
								fch[n-1] = chp;
							}
							cri->Release();
						}
					}
				}

				if (chp==nil && !fIsRemote)
				{
					cri=asscrit->RetainField(n);
					if (cri != nil) {
						if (cri->GetOutsideData())
							CreVValue_Params = CreVValue_Params | creVValue_outsidepath;
						if (assocData == nil || fAllAlwaysNull)
							curp = nil;
						else
							curp=(UniChar*)assocData->GetDataPtr(n);
						if (curp==nil) // on depasse le nombre de champs dans la fiche sur disque ou la fiche etait nouvelle
						{
							Code=FindCV(cri->GetTyp());
							if (Code != nil)
							{
								chp=(*Code)(asscrit->GetDF(),cri->GetSwitchSize(),nil,false,true,context, CreVValue_Params);
								if (chp == nil)
									err = ThrowError(memfull, DBaction_AccessingField);
								else
								{
									if (cri->GetTyp() == VK_UUID && cri->GetAutoGenerate() && !fAllAlwaysNull)
									{
										((VUUID*)chp)->Regenerate();
										Touch(n);
									}
									else
									{
										if (cri->GetAutoSeq() || !cri->IsNeverNull() || fAllAlwaysNull)
											chp->SetNull(true);
									}
								}
							}
							else
							{
								err = ThrowError(VE_DB4D_FIELDDEFCODEMISSING, DBaction_AccessingField);
							}
						}
						else
						{
							if ( cri->GetTyp() != assocData->GetTyp(n) ) // si pas le meme type, on convertit
							{
								if (assocData->IsFieldNull(n))
								{
									Code=FindCV(cri->GetTyp());
									if (Code != nil)
									{
										chp=(*Code)(asscrit->GetDF(),cri->GetSwitchSize(),nil,false,true,context, CreVValue_Params);
										if (chp == nil)
											err = ThrowError(memfull, DBaction_AccessingField);
										else
										{
											if (cri->GetTyp() == VK_UUID && cri->GetAutoGenerate())
											{
												((VUUID*)chp)->Regenerate();
												Touch(n);
											}
											else
											{
												if (cri->GetAutoSeq() || !cri->IsNeverNull())
													chp->SetNull(true);
											}
										}
									}
								}
								else
								{
									Code=FindCV(assocData->GetTyp(n));
									if (Code != nil)
									{			
										ValPtr chp2=(*Code)(asscrit->GetDF(),cri->GetSwitchSize(),curp, false, true, context, CreVValue_Params);
										if (chp2 == nil)
											err = ThrowError(memfull, DBaction_AccessingField);
										else
										{
											Code=FindCV(cri->GetTyp());
											if (Code != nil)
											{
												chp=(*Code)(asscrit->GetDF(),cri->GetSwitchSize(),nil,false,true,context, CreVValue_Params);
												if (chp == nil)
													err = ThrowError(memfull, DBaction_AccessingField);
												else
												{
													chp->FromValue(*chp2);
													if (cri->GetLimitingLen() > 0 && (cri->GetTyp() == (sLONG)VK_STRING || cri->GetTyp() == (sLONG)VK_STRING_UTF8))
													{
														((VString*)chp)->Truncate(cri->GetLimitingLen());
													}
												}
												delete chp2;
											}
											else
											{
												err = ThrowError(VE_DB4D_FIELDDEFCODEMISSING, DBaction_AccessingField);
											}
										}
									}
									else
									{
										err = ThrowError(VE_DB4D_FIELDDEFCODEMISSING, DBaction_AccessingField);
									}
								}
							}
							else
							{
								Code=FindCV(cri->GetTyp());
								if (Code != nil)
								{
									if (cri->GetTyp() == VK_IMAGE && pictAsRaw)
										chp = CrePictAsRaw(asscrit->GetDF(),cri->GetSwitchSize(),curp, false, true, context);
									else
										chp=(*Code)(asscrit->GetDF(),cri->GetSwitchSize(),curp, false, true, context, CreVValue_Params);
									if (chp == nil)
										err = ThrowError(memfull, DBaction_AccessingField);
									else
									{
										if (cri->GetTyp() == VK_IMAGE && !pictAsRaw)
										{
											if (*((sLONG*)curp) != -1 && IsPictureEmpty(chp))
												Touch(n);
										}
										if (cri->GetLimitingLen() > 0 && (cri->GetTyp() == (sLONG)VK_STRING || cri->GetTyp() == (sLONG)VK_STRING_UTF8))
										{
											((VString*)chp)->Truncate(cri->GetLimitingLen());
										}
									}
								}
								else
									err = ThrowError(VE_DB4D_FIELDDEFCODEMISSING, DBaction_AccessingField);
							}
						}
						
						fch[n-1]=chp;
						if (cri->GetAutoSeq() &&  err == VE_OK && chp != nil && chp->IsNull() && IsNew() && !fAllAlwaysNull)
						{
							StErrorContextInstaller errs(true);
							AutoSeqNumber* seq = asscrit->GetSeqNum(nil);
							if (seq != nil)
							{
								sLONG8 val = seq->GetNewValue(fToken);
								if (val != -1)
								{
									chp->FromLong8(val);
									Touch(n);
								}
							}
							else
								errs.Flush();
						}
						cri->Release();
					}
					{
						// faut il remvoyer une vvalue empty si le champ est detruit ou bien nil comme maintenant ?
					}
				}
				else
				{
					if (chp != nil)
					{
						if (chp->IsNull())
						{
							cri=asscrit->RetainField(n);
							if (cri != nil)
							{
								if (cri->GetAutoSeq() && IsNew() && !fAllAlwaysNull)
								{
									StErrorContextInstaller errs(true);
									AutoSeqNumber* seq = asscrit->GetSeqNum(nil);
									if (seq != nil)
									{
										sLONG8 val = seq->GetNewValue(fToken);
										if (val != -1)
										{
											chp->FromLong8(val);
											Touch(n);
										}
									}
									else
										errs.Flush();
								}
								if (cri->GetTyp() == VK_UUID && cri->GetAutoGenerate() && !fAllAlwaysNull)
								{
									((VUUID*)chp)->Regenerate();
									Touch(n);
								}
								cri->Release();
							}

						}
					}
				}

				if (chp != nil)
				{
					if (chp->IsNull())
					{
						cri=asscrit->RetainField(n);
						if (cri != nil) 
						{
							if (cri->IsNeverNull())
							{
								Code=FindCV(cri->GetTyp());
								if (Code != nil)
								{
									VValueSingle* xval = (*Code)(asscrit->GetDF(),cri->GetSwitchSize(),nil, false, true, context, CreVValue_Params);
									chp->FromValue(*xval);
									chp->SetNull(false);
									delete xval;
								}
								else
								{
									chp->FromString("");
									chp->SetNull(false);
								}
							}
							cri->Release();
						}
						else
						{
							chp->FromString("");
							chp->SetNull(false);
						}
					}
				}
			}
		}
	}
	/*
	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTACCESSFIELD, DBaction_AccessingField);
	*/
#if debugblobs
	if (chp != nil && chp->GetValueKind() == VK_BLOB)
	{
		BlobWithPtr* blob = ((VBlob4DWithPtr*)chp)->GetBlob4D();
		if (blob != nil)
		{
			sLONG blobnum = blob->GetNum();
			if (blobnum >=0)
			{
				if (asscrit != nil)
				{
					DataTableRegular* df = (DataTableRegular*)(asscrit->GetDF());
					df->debug_putblobnum(blobnum, GetNum());
				}
			}
		}
	}

#endif

	if (forQuery && chp != nil)
	{
		if (asscrit->IsFieldStyledText(n) && chp->GetValueKind() == VK_STRING)
		{
			if (n > fchForQuery.size())
				fchForQuery.resize(n+1, nil);
			ValPtr chpQuery = fchForQuery[n-1];
			if (chpQuery == nil)
			{
				chpQuery = new VString();
				VSpanTextParser::Get()->GetPlainTextFromSpanText(*(VString*)chp, *(VString*)chpQuery);
				fchForQuery[n-1] = chpQuery;
			}
			chp = chpQuery;
		}
		else if (chp->GetValueKind() == VK_IMAGE)
		{
#if !VERSION_LINUX   // Postponed Linux Implementation !
			if (n > fchForQuery.size())
				fchForQuery.resize(n+1, nil);
			ValPtr chpQuery = fchForQuery[n-1];
			if (chpQuery == nil)
			{
				chpQuery = new VString();
				if (chp->IsNull())
					chpQuery->SetNull(true);
				else
					((VPicture*)chp)->GetKeywords( *(VString*)chpQuery);
				fchForQuery[n-1] = chpQuery;
			}
			chp = chpQuery;
#endif  //(Postponed Linux Implementation)
		}
	}

	return(chp);
}


ValPtr FicheInMem::GetNthFieldOld(sLONG n, VError& err, bool forQuery)
{
	StAllocateInCache	alloc;

	ValPtr chp = nil;
	Field* cri;
	UniChar* curp;
	CreVValue_Code Code;
	creVValue_ParamType CreVValue_Params = creVValue_default;
	if (forQuery)
		CreVValue_Params = CreVValue_Params | creVValue_forquery;
	err = VE_OK;

	if (fIsRemote)
		err = CheckNthFieldRemote(n);
	if (n == 0 || err != VE_OK)
	{
		chp=nil;
	}
	else
	{
		if (n<0)
		{
			n=-n;
			if (n>fchold.size())
			{
				chp=nil;
			}
			else
			{
				chp=fchold[n-1];
			}
		}
		else
		{
			if (n>fchold.size())
			{
				fchold.resize(n,nil);
			}
			
			chp=fchold[n-1];
			if (chp==nil)
			{
				if (fIsRemote)
				{
					cri=asscrit->RetainField(n);
					if (cri != nil)
					{
						chp = (VValueSingle*)VValue::NewValueFromValueKind(cri->GetTyp());
						if (chp == nil)
							err = ThrowError(memfull, noaction);
						else
						{
							chp->SetNull(!cri->IsNeverNull());
							fchold[n-1] = chp;
						}
						cri->Release();
					}
				}
				else
				{
					cri=asscrit->RetainField(n);
					if (cri != nil) {
						if (cri->GetOutsideData())
							CreVValue_Params = CreVValue_Params | creVValue_outsidepath;
						if (assocData == nil)
							curp = nil;
						else
							curp=(UniChar*)assocData->GetDataPtr(n);
						if (curp==nil) // on depasse le nombre de champs dans la fiche sur disque ou la fiche etait nouvelle
						{
							Code=FindCV(cri->GetTyp());
							if (Code != nil)
							{
								chp=(*Code)(asscrit->GetDF(),cri->GetSwitchSize(),nil,false,false,context, CreVValue_Params);
								if (chp == nil)
									err = ThrowError(memfull, DBaction_AccessingField);
								else		
									if (!cri->IsNeverNull())
										chp->SetNull(true);
							}
							else
								err = ThrowError(VE_DB4D_FIELDDEFCODEMISSING, DBaction_AccessingField);
						}
						else
						{
							if ( cri->GetTyp() != assocData->GetTyp(n) )
							{
								if (assocData->IsFieldNull(n))
								{
									Code=FindCV(cri->GetTyp());
									if (Code != nil)
									{
										chp=(*Code)(asscrit->GetDF(),cri->GetSwitchSize(),nil,false,true,context, CreVValue_Params);
										if (chp == nil)
											err = ThrowError(memfull, DBaction_AccessingField);
										else
											if (!cri->IsNeverNull())
												chp->SetNull(true);
									}
								}
								else
								{
									Code=FindCV(assocData->GetTyp(n));
									if (Code != nil)
									{			
										ValPtr chp2=(*Code)(asscrit->GetDF(),cri->GetSwitchSize(),curp, false, true, context, CreVValue_Params);
										if (chp2 == nil)
											err = ThrowError(memfull, DBaction_AccessingField);
										else
										{
											Code=FindCV(cri->GetTyp());
											chp=(*Code)(asscrit->GetDF(),cri->GetSwitchSize(),nil,false,true,context, CreVValue_Params);
											if (chp == nil)
												err = ThrowError(memfull, DBaction_AccessingField);
											else
											{
												chp->FromValue(*chp2);
												/*
												if (cri->GetLimitingLen() > 0 && ( cri->GetTyp() == (sLONG)VK_STRING || cri->GetTyp() == (sLONG)VK_STRING_UTF8))
												{
													((VString*)chp)->Truncate(cri->GetLimitingLen());
												}
												*/
											}
											delete chp2;
										}
									}
									else
									{
										err = ThrowError(VE_DB4D_FIELDDEFCODEMISSING, DBaction_AccessingField);
									}
								}
							}
							else
							{
								Code=FindCV(cri->GetTyp());
								if (Code != nil)
								{
									chp=(*Code)(asscrit->GetDF(),cri->GetSwitchSize(),curp,false,false,context, CreVValue_Params);
									if (chp == nil)
										err = ThrowError(memfull, DBaction_AccessingField);
									else
									{
										/*
										if (cri->GetLimitingLen() > 0 && (cri->GetTyp() == (sLONG)VK_STRING || cri->GetTyp() == (sLONG)VK_STRING_UTF8))
										{
											((VString*)chp)->Truncate(cri->GetLimitingLen());
										}
										*/
									}
								}
								else
									err = ThrowError(VE_DB4D_FIELDDEFCODEMISSING, DBaction_AccessingField);
							}
						}
						
						fchold[n-1]=chp;
						cri->Release();
					}
				}
			}
		}
	}

	if (forQuery && chp != nil)
	{
		if (asscrit->IsFieldStyledText(n) && chp->GetValueKind() == VK_STRING)
		{
			if (n > fcholdForQuery.size())
				fcholdForQuery.resize(n+1, nil);
			ValPtr chpQuery = fcholdForQuery[n-1];
			if (chpQuery == nil)
			{
				chpQuery = new VString();
				VSpanTextParser::Get()->GetPlainTextFromSpanText(*(VString*)chp, *(VString*)chpQuery);
				fcholdForQuery[n-1] = chpQuery;
			}
			chp = chpQuery;
		}
		else if (chp->GetValueKind() == VK_IMAGE)
		{
#if !VERSION_LINUX   // Postponed Linux Implementation !

			if (n > fcholdForQuery.size())
				fcholdForQuery.resize(n+1, nil);
			ValPtr chpQuery = fcholdForQuery[n-1];
			if (chpQuery == nil)
			{
				chpQuery = new VString();
				((VPicture*)chp)->GetKeywords( *(VString*)chpQuery);
				fcholdForQuery[n-1] = chpQuery;
			}
			chp = chpQuery;
#endif  // (Postponed Linux Implementation)
		}
	}

	/*
	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTACCESSFIELD, DBaction_AccessingField);
	*/
	return(chp);
}

void FicheInMem::ClearAllDirty(void)
{
	sLONG i;
	ValPtr cv;
	
	for (i=(sLONG)fch.size()-1; i>=0; i--)
	{
		cv = fch[i];
		if (cv != nil) cv->SetDirty(false);
	}
}

void FicheInMem::ValidateAutoSeqToken(BaseTaskInfo* Context)
{
	fTokenIsValidated = true;
	if (fToken != 0)
	{
		AutoSeqNumber* seq = asscrit->GetSeqNum(Context->GetEncapsuleur());
		if (seq != nil)
		{
			seq->ValidateValue(fToken, asscrit, Context);
		}
	}
}


sLONG8 FicheInMem::GetAutoSeqValue()
{
	sLONG8 result = -1;
	if (fIsRemote)
	{
		if (!fIsIDGenerated)
		{
			// si la fiche a ete cree sur le client et n'a pas encore ete connecte au server
			IRequest* req = asscrit->GetOwner()->CreateRequest(context->GetEncapsuleur(), req_ConnectRecord + kRangeReqDB4D);
			if (req != nil)
			{
				VError err;
				req->PutBaseParam(asscrit->GetOwner());
				req->PutThingsToForget(VDBMgr::GetManager(), context);
				req->PutTableParam(asscrit);
				VUUID newid(true);
				fID = newid.GetBuffer();
				newid.WriteToStream(req->GetOutputStream());
				err = req->GetLastError();
				if (err == VE_OK)
				{
					err = req->Send();
					if (err== VE_OK)
						err = req->GetUpdatedInfo(asscrit->GetOwner(), context);
					if (err == VE_OK)
					{
						fIsIDGenerated = true;
						fRemoteSeqNum = req->GetInputStream()->GetLong8();
					}
				}

				req->Release();
			}
			
		}
		else
		{
			if (fRemoteSeqNum == -1)
			{
				IRequest* req = asscrit->GetOwner()->CreateRequest(context->GetEncapsuleur(), req_ConnectRecord + kRangeReqDB4D);
				if (req != nil)
				{
					VError err;
					req->PutBaseParam(asscrit->GetOwner());
					req->PutThingsToForget(VDBMgr::GetManager(), context);
					req->PutTableParam(asscrit);
					VUUID xid(fID);
					xid.WriteToStream(req->GetOutputStream());
					err = req->GetLastError();
					if (err == VE_OK)
					{
						err = req->Send();
						if (err== VE_OK)
							err = req->GetUpdatedInfo(asscrit->GetOwner(), context);
						if (err == VE_OK)
						{
							fRemoteSeqNum = req->GetInputStream()->GetLong8();
						}
					}

					req->Release();
				}
			}
		}

		result = fRemoteSeqNum;
	}
	else
	{
		if (fAlreadyAskedSeqNum == -2)
		{
			AutoSeqNumber* seq = asscrit->GetSeqNum(nil);
			if (seq != nil)
			{
				result = seq->GetNewValue(fToken);
				fAlreadyAskedSeqNum = result;
			}
		}
		else
			result = fAlreadyAskedSeqNum;
	}
	return result;
}



VError FicheInMem::stockcrit(BaseTaskInfo* context,XBOX::VProgressIndicator* inProgress)
{
	VError err;
	
	err=VE_OK;
	NumFieldArray deps, deps2;
	sLONG i,nb;

	xbox_assert(!fIsRemote);

	if (asscrit->AtLeastOneAutoSeqField())
	{
		asscrit->CopyAutoSeqDep(deps);
		nb = deps.GetCount();
		for (i=1; i <= nb && err == VE_OK; i++)
			ValPtr cv = GetNthField(deps[i], err); // force le calcul des valeurs autosequences
	}

	if (asscrit->AtLeastOneAutoGenerateField())
	{
		asscrit->CopyAutoGenerateDep(deps);
		nb = deps.GetCount();
		for (i=1; i <= nb && err == VE_OK; i++)
			ValPtr cv = GetNthField(deps[i], err); // force le calcul des valeurs autogenerate
	}
	
	if (err == VE_OK && asscrit->AtLeastOneNot_NullField() && asscrit->GetOwner()->CheckNot_NullEnabled() && MustCheckIntegrity())
	{
		asscrit->CopyNot_NullDep(deps);
		nb = deps.GetCount();
		for (i=1; i <= nb; i++)
		{
			sLONG n = deps[i];
			ValPtr cv = GetNthField(-n, err);
			if (err != VE_OK)
				break;
			if (cv == nil)
			{
				if (assocData->IsFieldNull(n))
				{
					err = ThrowError(VE_DB4D_Not_NullFIELD_IS_NULL, DBaction_UpdatingRecord);
					break;
				}
			}
			else
			{
				if (cv->IsNull())
				{
					err = ThrowError(VE_DB4D_Not_NullFIELD_IS_NULL, DBaction_UpdatingRecord);
					break;
				}
			}
		}
	}

	if (err == VE_OK)
	{
		deps.SetCount(0);
		if (asscrit->AtLeastOneUniqueField() && MustCheckIntegrity())
		{
			asscrit->CopyUniqueDep(deps);
			/*
			nb = deps.GetCount();
			for (i=nb; i >= 1; i--)
			{
				Field* cri = asscrit->RetainField(deps[i]);
				if (cri != nil)
				{
					if (cri->IsPrimIndexe())
					{
						deps.DeleteNth(i);
					}
					cri->Release();
				}
			}
			*/
		}

		asscrit->CopyPrimaryKey(deps2);
		if (deps2.GetCount() > 0 && asscrit->GetOwner()->CheckNot_NullEnabled() && MustCheckIntegrity())
		{
			nb = deps2.GetCount();
			//if (!asscrit->IsIndexed(deps2))
			{
				for (i=1; i<=nb; i++)
				{
					sLONG n = deps2[i];
					ValPtr cv = GetNthField(-n, err);
					if (err != VE_OK)
						break;
					if (cv == nil)
					{
						if (assocData->IsFieldNull(n))
						{
							err = ThrowError(VE_DB4D_Not_NullFIELD_IS_NULL, DBaction_UpdatingRecord);
							break;
						}
					}
					else
					{
						if (cv->IsNull())
						{
							err = ThrowError(VE_DB4D_Not_NullFIELD_IS_NULL, DBaction_UpdatingRecord);
							break;
						}
					}
				}
			}
		}

		vector<IndexInfo*> UniqIndexes;

		if ((deps.GetCount() > 0 || deps2.GetCount() > 0) && err == VE_OK && MustCheckIntegrity())
		{
			//Use a progress indicator to allow monitoring integrity checks when saving
			//sinec they can take a lot of time
			VProgressIndicator* actualProgress = GetCachedProgressIndicator(context, inProgress);
			
			if (!asscrit->CheckUniqueness(this, deps, context, deps2, actualProgress, UniqIndexes))
			{
				//err = ThrowError(VE_DB4D_DUPLICATED_KEY, DBaction_UpdatingRecord);
				err = VE_DB4D_DUPLICATED_KEY;
			}

			if ( (actualProgress != NULL) && (inProgress== NULL))
			{
				context->SetCachedProgressIndicator(actualProgress);
			}
		}

		FicheOnDisk* olddata = RetainRefCountable(assocData);
		if (err == VE_OK)
			err = ReajusteData(context);

		if (err == VE_OK)
		{
			FicheOnDisk* newdata = RetainRefCountable(assocData);
			CopyRefCountable(&assocData, olddata);
			err=asscrit->UpdateIndexKey(this, context);
			CopyRefCountable(&assocData, newdata);
			QuickReleaseRefCountable(newdata);

			/*
			if (err==0)
			{
				err=asscrit->UpdateBlobs(this);
				est fait maintenant plus tot dans le saverecord
			}
			*/
			
			/*
			if (err==0)
				FlushFieldsVValue();
			*/
			
			if (err==VE_OK)
			{
				sLONG nb=(sLONG)fchold.size();
				for (sLONG i=0;i<nb;i++)
				{
					ValPtr chp=fchold[i];
					if (chp!=nil)
					{
						delete chp;
					}
				}
				fchold.resize(0);
				
				isnew=false;
				//assocData->SetNotNew();
			}
		}

		OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
		for (vector<IndexInfo*>::iterator cur = UniqIndexes.begin(), end = UniqIndexes.end(); cur != end; ++cur)
		{
			IndexInfo* ind = *cur;
			ind->Open(index_write);
			ind->ReleaseGlobalUniqKey(curstack, this, context, true);
			ind->Close();
			ind->Release();
		}

		QuickReleaseRefCountable(olddata);

	}

	if (err != VE_OK)
	{
		if (isnew)
			err = ThrowError(VE_DB4D_CANNOTUPDATENEWRECORD, DBaction_UpdatingRecord);
		else
			err = ThrowError(VE_DB4D_CANNOTUPDATERECORD, DBaction_UpdatingRecord);
	}
		
	return(err);
}


VError FicheInMem::ReajusteData(BaseTaskInfo* context)
{
	FicheOnDisk* newFicD;
	VError err = VE_OK;
	
	xbox_assert(!fIsRemote);
	
	Boolean maykeeporiginal = false; // L.R le 22/8/2011 il faut absolument toujours recreer le assocData
	/*
	Transaction* trans = GetCurrentTransaction(context);
	if (trans != nil)
	{
		maykeeporiginal = assocData->IsNewInTrans();
	}
	*/

	newFicD = assocData->ReajusteData(this, err, maykeeporiginal);
	
	if (err == VE_OK)
	{
		if (assocData != newFicD)
		{
			FicheOnDisk* oldassocData = assocData;
			assocData = newFicD;

			oldassocData->Release();
		}		
	}

	if (err != VE_OK)
	{
		if (isnew)
			err = ThrowError(VE_DB4D_CANNOTUPDATENEWRECORD, DBaction_UpdatingRecord);
		else
			err = ThrowError(VE_DB4D_CANNOTUPDATERECORD, DBaction_UpdatingRecord);
	}
		
	return err;
}





VError FicheInMem::ThrowError( VError inErrCode, ActionDB4D inAction) const
{
	Base4D* owner = asscrit->GetOwner();
	sLONG numfic = -1;
	
	if (assocData != nil)
		numfic = assocData->getnumfic();

	VErrorDB4D_OnRecord *err = new VErrorDB4D_OnRecord(inErrCode, inAction, owner, asscrit->GetNum(), numfic);
	VTask::GetCurrent()->PushRetainedError( err);
	
	return inErrCode;
}


/*
void FicheInMem::FlushFieldsVValue()
{
	if (IsRecordModified() && (assocData->GetDataHandle()!=nil))  {
		for( sLONG i = ch.GetCount() ; i >= 1 ; --i) {
			ValPtr cv = ch[i];
			if (cv != nil) {
				cv->Flush( assocData->GetDataPtr( i));
			}
		}
	}
}
*/

RelationDep* FicheInMem::FindRelationDep(Relation* rel, Boolean OldOne)
{
	RelationDep *res = nil, *x;
	sLONG nb = RelDeps.GetCount();
	sLONG i;

	for (i=0; i < nb; i++)
	{
		x = RelDeps[i];
		if (x->GetRelation() == rel && x->IsOld() == OldOne)
		{
			res = x;
			break;
		}
	}

	return res;
}


FicheInMem* FicheInMem::RetainCachedRelatedRecord(Relation* rel, Boolean OldOne)
{
	FicheInMem* rec = nil;
	uLONG stamp = 0;
	if (!OldOne)
	{
		stamp = GetFieldModificationStamp(rel->GetSource());
	}

	RelationDep* reldep = FindRelationDep(rel, OldOne);
	if (reldep != nil)
	{
		rec = reldep->GetRecord(stamp);
		if (rec != nil)
			rec->Retain();
	}

	return rec;
}


Selection* FicheInMem::RetainCachedRelatedSelection(Relation* rel, Boolean OldOne)
{
	Selection* sel = nil;
	uLONG stamp = 0;
	if (!OldOne)
	{
		stamp = GetFieldModificationStamp(rel->GetSource());
	}

	RelationDep* reldep = FindRelationDep(rel, OldOne);
	if (reldep != nil)
	{
		sel = reldep->GetSelection(stamp);
		if (sel != nil)
			sel->Retain();
	}

	return sel;
}


void FicheInMem::SetCachedRelatedRecord(Relation* rel, FicheInMem* rec, Boolean OldOne)
{
	uLONG stamp = 0;
	if (!OldOne)
	{
		stamp = GetFieldModificationStamp(rel->GetSource());
	}
	RelationDep* reldep = FindRelationDep(rel, OldOne);
	if (reldep == nil)
	{
		reldep = new RelationDep(rel,rec,stamp,OldOne);
		if (reldep != nil)
		{
			if (RelDeps.Add(reldep))
			{
				rel->Retain();
				rec->Retain();
			}
		}
	}
	else
	{
		reldep->SetRecord(rec,stamp);
		rec->Retain();
	}
}


void FicheInMem::SetCachedRelatedSelection(Relation* rel, Selection* sel, Boolean OldOne)
{
	uLONG stamp = 0;
	if (!OldOne)
	{
		stamp = GetFieldModificationStamp(rel->GetSource());
	}
	RelationDep* reldep = FindRelationDep(rel, OldOne);
	if (reldep == nil)
	{
		reldep = new RelationDep(rel,sel,stamp,OldOne);
		if (reldep != nil)
		{
			if (RelDeps.Add(reldep))
			{
				rel->Retain();
				sel->Retain();
			}
		}
	}
	else
	{
		reldep->SetSelection(sel,stamp);
		sel->Retain();
	}

}


VError FicheInMem::ReservedRecordNumber(BaseTaskInfo* context)
{
	VError err = VE_OK;
	{
		if (fRecNumReserved == -1)
		{
			if (GetNum() < 0)
			{
				if (fIsRemote)
					RemoteAskedForReservedRecordNumber = true;
				else
					fRecNumReserved  = asscrit->GetDF()->ReserveNewRecAddr(context, err);
			}
		}
	}

	return err;
}



VError FicheInMem::FromClientAsSubRecord(VStream* from, BaseTaskInfo* inContext)
{
	VError err;

	fUsedAsSubRecordOnServer = true;
	VUUID xid;
	err = xid.ReadFromStream(from);
	if (err == VE_OK)
	{
		fIsIDGenerated = true;
		fID = xid.GetBuffer();
		sLONG numfield;
		do 
		{
			err = from->GetLong(numfield);
			if (numfield != 0)
			{
				if (numfield < 0)
					err = ThrowError(VE_DB4D_WRONGFIELDREF_IN_RECORD, noaction);
				else
				{
					if (numfield <= maxnbcrit)
					{
						VValueSingle* cv = nil;
						sLONG datakind;
						err = from->GetLong(datakind);
						if (err == VE_OK)
						{
							cv = GetNthField(numfield, err);
							if (cv != nil && err == VE_OK)
							{
								if (datakind == VK_EMPTY)
								{
									if (cv->GetTrueValueKind() != VK_SUBTABLE_KEY)
										cv->SetNull(true);
								}
								else
								{
									cv->SetNull(false);
									if (datakind == VK_SUBTABLE)
									{
										xbox_assert(datakind != VK_SUBTABLE);
									}
									else
									{
										if (datakind == VK_SUBTABLE_KEY || cv->GetTrueValueKind() == VK_SUBTABLE_KEY)
										{
											VValueSingle* cv2 = (VValueSingle*)VValue::NewValueFromValueKind(datakind);
											if (cv2 != nil)
											{
												err = cv2->ReadRawFromStream(from);
												delete cv2;
											}
											else
												err = ThrowBaseError(memfull, noaction);
										}
										else
										{
											if (cv->GetValueKind() == datakind)
											{
												err = cv->ReadRawFromStream(from);
											}
											else
											{
												VValueSingle* cv2 = (VValueSingle*)VValue::NewValueFromValueKind(datakind);
												if (cv2 != nil)
												{
													cv2->SetNull(false);
													err = cv2->ReadRawFromStream(from);
													if (err == VE_OK)
														cv2->GetValue(*cv);
													delete cv2;
												}
												else
													err = ThrowBaseError(memfull, noaction);
											}
										}
									}
								}

								Touch(numfield);
							}

						}
					}
				}
			}
		} while(err == VE_OK && numfield != 0);
	}

	return err;
}



VError FicheInMem::FromClient(VStream* from, BaseTaskInfo* inContext)
{
	VError err;
	sLONG numfield;
	do 
	{
		err = from->GetLong(numfield);
		if (numfield != 0)
		{
			if (numfield < 0)
				err = ThrowError(VE_DB4D_WRONGFIELDREF_IN_RECORD, noaction);
			else
			{
				if (numfield <= maxnbcrit)
				{
					VValueSingle* cv = nil;
					sLONG datakind;
					err = from->GetLong(datakind);
					if (err == VE_OK)
					{
						cv = GetNthField(numfield, err);
						if (cv != nil && err == VE_OK)
						{
							if (datakind == VK_EMPTY)
							{
								cv->SetNull(true);
							}
							else
							{
								cv->SetNull(false);
								if (datakind == VK_SUBTABLE)
								{
									Table* tsub;
									VSubTableDB4D* subtable = nil;
									if (cv->GetTrueValueKind() == VK_SUBTABLE)
									{
										subtable = (VSubTableDB4D*)cv;
										tsub = subtable->GetSubTable();
									}
									else
										tsub = asscrit;

									if (subtable != nil)
										subtable->UnMarkAllSubRecords();

									sLONG nb;
									err = from->GetLong(nb);
									if (err == VE_OK)
									{
										for (sLONG j = 0; j < nb && err == VE_OK; j++)
										{
											sLONG subrecnum = -1;
											err = from->GetLong(subrecnum);
											if (err == VE_OK)
											{
												FicheInMem* subrec = nil;
												if (subtable != nil)
												{
													if (subrecnum != -3)
													{
														subrec = subtable->FindSubRecNum(subrecnum);
														if (subrec != nil)
														{
															subrec->Retain();
														}
														else
														{
															xbox_assert(false);
														}
													}
													if (subrec == nil)
													{
														subrec = subtable->AddSubRecord(err, context);
														if (subrec != nil)
															subrec->Retain();
														else
															xbox_assert(false);
													}
												}
												else
													subrec = tsub->GetDF()->NewRecord(err, context);

												if (subrec != nil)
												{
													err = subrec->FromClientAsSubRecord(from, context);
													subrec->Release();
												}
											}

										}
									}

									if (err == VE_OK && subtable != nil)
									{
										err = subtable->DeleteSubRecordsNotMarked();
									}

								}
								else
								{
									if (cv->GetValueKind() == datakind)
									{
										err = cv->ReadRawFromStream(from);
									}
									else
									{
										VValueSingle* cv2 = (VValueSingle*)VValue::NewValueFromValueKind(datakind);
										if (cv2 != nil)
										{
											cv2->SetNull(false);
											err = cv2->ReadRawFromStream(from);
											if (err == VE_OK)
												cv2->GetValue(*cv);
											delete cv2;
										}
										else
											err = ThrowBaseError(memfull, noaction);
									}
								}
							}

							Touch(numfield);
						}

					}
				}
			}
		}
	} while(err == VE_OK && numfield != 0);

	return err;
}



VError FicheInMem::ToClientAsSubRecord(VStream* into, BaseTaskInfo* inContext)
{
	VError err = VE_OK;

	if (err == VE_OK)
	{
		if (!fIsIDGenerated)
		{
			VUUID newid(true);
			fID = newid.GetBuffer();
			fIsIDGenerated = true;
			err = newid.WriteToStream(into);
		}
		else
		{
			VUUID xid(fID);
			err = xid.WriteToStream(into);
		}
	}

	sLONG nbvals = asscrit->GetNbCrit();

	if (err == VE_OK)
		err = into->PutLong(GetNum());
	if (err == VE_OK)
		err = into->PutLong(nbvals);
	if (err == VE_OK)
	{
		for (sLONG i = 1; i <= nbvals && err == VE_OK; i++)
		{
			ValPtr cv = GetNthField(i, err);
			if (err == VE_OK)
			{
				if (cv == nil)
				{
					err = into->PutLong(VK_EMPTY);
				}
				else
				{
					if (cv->GetTrueValueKind() == VK_SUBTABLE)
					{
						xbox_assert(cv->GetTrueValueKind() != VK_SUBTABLE);
						// pas plus d'un niveau
					}
					else
					{
						if (cv->IsNull())
						{
							err = into->PutLong(VK_EMPTY);
						}
						else
						{
							err = into->PutLong(cv->GetValueKind());
							if (err == VE_OK)
								err = cv->WriteToStream(into);
						}
					}
				}
			}

		}
	}

	return err;
}

VError FicheInMem::ToClient(VStream* into, BaseTaskInfo* inContext)
{
	VError err;
	sLONG nbvals = asscrit->GetNbCrit();

	xbox_assert(!fIsASubRecordOnClient);

	if (!fIsIDGenerated)
	{
		VUUID newid(true);
		fID = newid.GetBuffer();
		fIsIDGenerated = true;
		err = newid.WriteToStream(into);
	}
	else
	{
		VUUID xid(fID);
		err = xid.WriteToStream(into);
	}

	if (err == VE_OK)
	{
		if (fToken != 0 || isnew)
			err = into->PutLong8(GetAutoSeqValue());
		else
			err = into->PutLong8(-1);
	}
	if (err == VE_OK)
		err = into->PutLong(GetNum());
	if (err == VE_OK)
		err = into->PutLong(nbvals);
	if (err == VE_OK)
		err = into->PutByte(fReadOnly ? 1 : 0);
	if (err == VE_OK)
	{
		for (sLONG i = 1; i <= nbvals && err == VE_OK; i++)
		{
			ValPtr cv = GetNthField(i, err);
			if (err == VE_OK)
			{
				if (cv == nil)
				{
					err = into->PutLong(VK_EMPTY);
				}
				else
				{
					if (cv->GetTrueValueKind() == VK_SUBTABLE)
					{
						VSubTableDB4D* subtable = (VSubTableDB4D*)cv;
						sLONG nb = subtable->CountRecord(inContext, false);
						err = into->PutLong((sLONG)VK_SUBTABLE);
						if ((err == VE_OK) && inContext->CheckClientFeature( feature_SendSubTableID))
							err = into->PutLong8(cv->GetLong8());
						if (err == VE_OK)
							err = into->PutLong(nb);
						for (sLONG i = 1; i <= nb && err == VE_OK; i++)
						{
							FicheInMem* subrec = subtable->GetNthSubRecord(i, err, inContext, false);
							if (subrec != nil)
								err = subrec->ToClientAsSubRecord(into, inContext);
						}
					}
					else
					{
						if (cv->IsNull())
						{
							err = into->PutLong(VK_EMPTY);
						}
						else
						{
							err = into->PutLong(cv->GetValueKind());
							if (err == VE_OK)
								err = cv->WriteToStream(into);
						}
					}
				}
			}

		}
	}

	if (err == VE_OK)
	{
		if (fReadOnly)
		{
			err = into->PutWord((sWORD)fKindOfLock);
			if (err == VE_OK)
			{
				if (fLockingContextExtraData == nil)
					err = into->PutByte('.');
				else
				{
					err = into->PutByte('+');
					if (err == VE_OK)
						err = fLockingContextExtraData->WriteToStream(into);
				}
			}
		}
	}

	if (err == VE_OK)
		VDBMgr::GetManager()->KeepRecordOnServer(this);

	return err;
}



VError FicheInMem::FromServerAsSubRecord(VStream* from, BaseTaskInfo* inContext, Table* inTable)
{
	VError err = VE_OK;

	sLONG nbvals;

	fIsASubRecordOnClient = true;

	VUUID xid;
	err = xid.ReadFromStream(from);
	if (err == VE_OK)
	{
		fIsIDGenerated = true;
		fID = xid.GetBuffer();
	}

	asscrit = inTable;
	maxnbcrit = asscrit->GetNbCrit();
	assocData = nil;

	context = inContext;

	fIsRemote = true;
	if (err == VE_OK)
		err = from->GetLong(fRemoteRecNum);
	if (fRemoteRecNum>=0)
		RemoteAskedForReservedRecordNumber = false;
	if (err == VE_OK)
		err = from->GetLong(nbvals);
	if (err == VE_OK)
	{
		fchold.resize(nbvals, nil);
	}
	fReadOnly = false;
	isnew = fRemoteRecNum == -3;
	if (err == VE_OK)
	{
		for (sLONG i = 1; i <= nbvals && err == VE_OK; i++)
		{
			VValueSingle* cv = nil;
			sLONG datakind;
			err = from->GetLong(datakind);
			if (err == VE_OK)
			{
				sLONG fieldtype = asscrit->GetFieldType(i);
				if (datakind == VK_EMPTY)
				{
					cv = (VValueSingle*)VValue::NewValueFromValueKind(fieldtype);
					cv->SetNull(true);
					fchold[i-1] = cv;
				}
				else
				{
					if (datakind == VK_SUBTABLE)
					{
						xbox_assert(datakind != VK_SUBTABLE);
						// pas plus d'un niveau
					}
					else
					{
						if (fieldtype == datakind)
						{
							cv = (VValueSingle*)VValue::NewValueFromValueKind(fieldtype);
							err = cv->ReadFromStream(from);
							cv->SetNull(false);
							if (err == VE_OK)
							{
								fchold[i-1] = cv;
							}
							else
								delete cv;
						}
						else
						{
							VValueSingle* cv2 = (VValueSingle*)VValue::NewValueFromValueKind(datakind);
							if (cv2 != nil)
							{
								cv2->SetNull(false);
								err = cv2->ReadFromStream(from);
								if (err == VE_OK)
									cv = cv2->ConvertTo(fieldtype);
								if (cv == nil)
									err = ThrowBaseError(memfull, noaction);
								else
								{
									fchold[i-1] = cv;
								}
								delete cv2;
							}
							else
								err = ThrowBaseError(memfull, noaction);
						}
					}
				}
			}
		}
	}

	fKindOfLock = DB4D_LockedByNone;

	return err;
}



VError FicheInMem::FromServer(VStream* from, BaseTaskInfo* inContext, Table* inTable)
{
	VError err;

	sLONG nbvals;

	xbox_assert(!fIsASubRecordOnClient);

	asscrit = inTable;
	maxnbcrit = asscrit->GetNbCrit();
	assocData = nil;
	
	context = inContext;

	fIsRemote = true;
	VUUID xid;
	err = xid.ReadFromStream(from);
	fID = xid.GetBuffer();
	fIsIDGenerated = true;
	if (err == VE_OK)
		err = from->GetLong8(fRemoteSeqNum);
	if (err == VE_OK)
		err = from->GetLong(fRemoteRecNum);
	if (fRemoteRecNum>=0)
		RemoteAskedForReservedRecordNumber = false;
	if (err == VE_OK)
		err = from->GetLong(nbvals);
	if (err == VE_OK)
	{
		fchold.resize(nbvals, nil);
	}
	uBYTE readonly = 1;
	if (err == VE_OK)
		err = from->GetByte(readonly);
	fReadOnly = readonly == 0 ? false : true;
	isnew = fRemoteRecNum == -3;
	if (err == VE_OK)
	{
		for (sLONG i = 1; i <= nbvals && err == VE_OK; i++)
		{
			VValueSingle* cv = nil;
			sLONG datakind;
			err = from->GetLong(datakind);
			if (err == VE_OK)
			{
				sLONG fieldtype = asscrit->GetFieldType(i);
				if (datakind == VK_EMPTY)
				{
					cv = (VValueSingle*)VValue::NewValueFromValueKind(fieldtype);
					cv->SetNull(true);
					fchold[i-1] = cv;
				}
				else
				{
					if (datakind == VK_SUBTABLE)
					{
						sLONG8 idsub = 0;
						if (inContext->CheckServerFeature( feature_SendSubTableID))
							err = from->GetLong8(idsub);
						sLONG nb = 0;
						if (err == VE_OK)
							err = from->GetLong(nb);
						if (err == VE_OK)
						{
							Table* tsub;
							VSubTableDB4D* subtable = nil;
							if (fieldtype == VK_SUBTABLE)
							{
								subtable = new VSubTableDB4D;
								Field* cri = asscrit->RetainField(i);
								err = subtable->InitForNewRecord(this, cri, inContext);
								cri->Release();
								tsub = subtable->GetSubTable();
								fch.resize(i, nil);
								fch[i-1] = subtable;
								subtable->FromLong8(idsub);
							}
							else
								tsub = inTable;

							for (sLONG j = 0; j < nb && err == VE_OK; j++)
							{
								FicheInMem* subrec = asscrit->GetOwner()->BuildRecordFromServerAsSubRecord(from, inContext, tsub, err);
								if (subrec != nil)
								{
									if (subtable != nil)
										subtable->AddOrUpdateSubRecord(subrec);
									subrec->Release();
								}
							}

							if (err == VE_OK && subtable != nil)
							{
								subtable->AllSubRecords(inContext);
							}
						}
					}
					else
					{
						if (fieldtype == datakind)
						{
							cv = (VValueSingle*)VValue::NewValueFromValueKind(fieldtype);
							err = cv->ReadFromStream(from);
							cv->SetNull(false);
							if (err == VE_OK)
							{
								fchold[i-1] = cv;
							}
							else
								delete cv;
						}
						else
						{
							VValueSingle* cv2 = (VValueSingle*)VValue::NewValueFromValueKind(datakind);
							if (cv2 != nil)
							{
								cv2->SetNull(false);
								err = cv2->ReadFromStream(from);
								if (err == VE_OK)
									cv = cv2->ConvertTo(fieldtype);
								if (cv == nil)
									err = ThrowBaseError(memfull, noaction);
								else
								{
									fchold[i-1] = cv;
								}
								delete cv2;
							}
							else
								err = ThrowBaseError(memfull, noaction);
						}
					}
				}
			}
		}
	}

	if (err == VE_OK)
	{
		ReleaseRefCountable(&fLockingContextExtraData);
		if (fReadOnly)
		{
			sWORD xlock;
			uBYTE cc;
			err = from->GetWord(xlock);
			fKindOfLock = (DB4D_KindOfLock)xlock;
			if (err == VE_OK)
				err = from->GetByte(cc);
			if (err == VE_OK)
			{
				if (cc == '+')
				{
					VValueBag* bag = new VValueBag();
					err = bag->ReadFromStream(from);
					fLockingContextExtraData = bag;
				}
			}
		}
		else
			fKindOfLock = DB4D_LockedByNone;
	}

	return err;
}


VError FicheInMem::ToServerMinimal(VStream* into, BaseTaskInfo* inContext)
{
	VError err = VE_OK;
	xbox_assert(!fIsASubRecordOnClient);

	if (!fIsIDGenerated)
	{
		VUUID newid(true);
		fID = newid.GetBuffer();
		fIsIDGenerated = true;
		err = newid.WriteToStream(into);
	}
	else
	{
		VUUID xid(fID);
		err = xid.WriteToStream(into);
	}

	err = into->PutLong(fRemoteRecNum);
	return err;
}



VError FicheInMem::ToServerAsSubRecord(VStream* into, BaseTaskInfo* inContext)
{
	VError err = VE_OK;

	xbox_assert(fIsASubRecordOnClient);

	err = into->PutLong(fRemoteRecNum);
	if (err == VE_OK)
	{
		if (!fIsIDGenerated)
		{
			VUUID newid(true);
			fID = newid.GetBuffer();
			fIsIDGenerated = true;
			err = newid.WriteToStream(into);
		}
		else
		{
			VUUID xid(fID);
			err = xid.WriteToStream(into);
		}
	}

	if (err == VE_OK)
	{
		sLONG i, nb = (sLONG)fch.size();
		for (i = 1; i <= nb && err == VE_OK; i++)
		{
			VValueSingle* cv = fch[i-1];
			if (cv != nil && IsModif(i))
			{
				err = into->PutLong(i);
				if (err == VE_OK)
				{
					if (cv->GetTrueValueKind() == VK_SUBTABLE)
					{
						xbox_assert(cv->GetTrueValueKind() != VK_SUBTABLE);
					}
					else
					{
						if (cv->IsNull())
						{
							err = into->PutLong(VK_EMPTY);
						}
						else
						{
							err = into->PutLong(cv->GetValueKind());
							if (err == VE_OK)
								err = cv->WriteToStream(into);
						}
					}
				}
			}
		}
		if (err == VE_OK)
		{
			err = into->PutLong(0);
		}
	}


	return err;
}


VError FicheInMem::ToServer(VStream* into, BaseTaskInfo* inContext)
{
	VError err = VE_OK;

	xbox_assert(!fIsASubRecordOnClient);

	if (!fIsIDGenerated)
	{
		VUUID newid(true);
		fID = newid.GetBuffer();
		fIsIDGenerated = true;
		err = newid.WriteToStream(into);
	}
	else
	{
		VUUID xid(fID);
		err = xid.WriteToStream(into);
	}

	err = into->PutLong(fRemoteRecNum);
	if (err == VE_OK)
	{
		sLONG i, nb = (sLONG)fch.size();
		for (i = 1; i <= nb && err == VE_OK; i++)
		{
			VValueSingle* cv = fch[i-1];
			if (cv != nil && IsModif(i))
			{
				err = into->PutLong(i);
				if (err == VE_OK)
				{
					if (cv->GetTrueValueKind() == VK_SUBTABLE)
					{
						VSubTableDB4D* subtable = (VSubTableDB4D*)cv;
						sLONG nb = subtable->CountRecord(inContext, false);
						err = into->PutLong((sLONG)VK_SUBTABLE);
						if (err == VE_OK)
							err = into->PutLong(nb);
						for (sLONG i = 1; i <= nb && err == VE_OK; i++)
						{
							FicheInMem* subrec = subtable->GetNthSubRecord(i, err, inContext, false);
							if (subrec != nil)
								err = subrec->ToServerAsSubRecord(into, inContext);
						}
						/*
						if (err == VE_OK)
						{
							nb = subtable->CountDeletedRecord();
							err = into->PutLong(nb);
							for (sLONG i = 1; i <= nb && err == VE_OK; i++)
							{
								FicheInMem* subrec = subtable->GetNthDeletedSubRecord(i);
								if (subrec != nil)
									err = subrec->ToServer(into, inContext);
							}
						}
						*/
					}
					else
					{
						if (cv->IsNull())
						{
							err = into->PutLong(VK_EMPTY);
						}
						else
						{
							err = into->PutLong(cv->GetValueKind());
							if (err == VE_OK)
								err = cv->WriteToStream(into);
						}
					}
				}
			}
		}
		if (err == VE_OK)
		{
			err = into->PutLong(0);
		}
	}


	return err;
}



VError FicheInMem::SendModifiedFields(VStream* into, BaseTaskInfo* inContext, StampVector* oldstamps)
{
	xbox_assert(!fIsASubRecordOnClient);

	VError err = VE_OK;
	sLONG nbvals = asscrit->GetNbCrit();

	if (nbvals > fFieldsStamps.size())
		nbvals = (sLONG)fFieldsStamps.size();

	for (sLONG i = 1; i <= nbvals && err == VE_OK; i++)
	{
		ValPtr cv = GetNthField(i, err);
		sLONG oldstamp = 0;
		if (oldstamps != nil && oldstamps->size() >= i)
			oldstamp = (*oldstamps)[i-1];
		if ((err == VE_OK) && (oldstamps == nil || fFieldsStamps[i-1] != oldstamp || (cv != nil && cv->GetTrueValueKind() == VK_SUBTABLE)))
		{
			err = into->PutLong(i);
			if (err == VE_OK)
			{
				if (cv == nil)
				{
					err = into->PutLong(VK_EMPTY);
				}
				else
				{
					if (cv->GetTrueValueKind() == VK_SUBTABLE)
					{		
						VSubTableDB4D* subtable = (VSubTableDB4D*)cv;
						sLONG nb = subtable->CountRecord(inContext, false);
						//err = into->PutLong((sLONG)VK_SUBTABLE);
						err = into->PutLong((sLONG)VK_BAG);
						
						if (err == VE_OK)
							err = into->PutLong(nb);
						for (sLONG i = 1; i <= nb && err == VE_OK; i++)
						{
							FicheInMem* subrec = subtable->GetNthSubRecord(i, err, inContext, false);
							if (subrec != nil)
							{
								VUUID xid(subrec->GetID());
								err = xid.WriteToStream(into);
								err = into->PutLong(subrec->GetNum());
								//err = subrec->ToClient(into, inContext);
							}
						}
						
					}
					else
					{
						if (cv->IsNull())
						{
							err = into->PutLong(VK_EMPTY);
						}
						else
						{
							err = into->PutLong(cv->GetValueKind());
							if (err == VE_OK)
								err = cv->WriteToStream(into);
						}
					}
				}
			}
		}
	}
	if (err == VE_OK)
		err = into->PutLong(0);

	return err;
}


VError FicheInMem::ReceiveModifiedFields(VStream* from, BaseTaskInfo* inContext, Table* inTable)
{
	sLONG i;
	VError err = VE_OK;

	do 
	{
		err = from->GetLong(i);
		if (i != 0)
		{
			sLONG datakind;
			err = from->GetLong(datakind);
			VValueSingle* cv =nil ;
			if (err == VE_OK)
				cv = GetNthField(i, err);
			if (cv != nil && err == VE_OK)
			{
				Touch(i);
				if (datakind == VK_EMPTY)
				{
					cv->SetNull(true);
				}
				else
				{
					cv->SetNull(false);
					if (datakind == VK_SUBTABLE)
					{
						// pour d'ancien server
					}
					else
					{
						if (datakind == VK_BAG) // petit trick pour ne pas casser la compatibilite avec d'ancien serveur
						{
							sLONG nb;
							err = from->GetLong(nb);
							if (err == VE_OK)
							{
								Table* tsub;
								VSubTableDB4D* subtable = nil;
								if (cv->GetTrueValueKind() == VK_SUBTABLE)
								{		
									subtable = (VSubTableDB4D*)cv;
								}

								for (sLONG j = 0; j < nb && err == VE_OK; j++)
								{
									VUUID xid;
									err = xid.ReadFromStream(from);
									sLONG recnum = -1;
									err = from->GetLong(recnum);
									if (err == VE_OK && subtable != nil)
									{
										sLONG nb2 = subtable->CountRecord(inContext, false);
										for (sLONG k = 1; k <= nb2; k++)
										{
											FicheInMem* subrec = subtable->GetNthSubRecord(k, err, inContext, false);
											if (subrec != nil)
											{
												if (subrec->GetID() == xid.GetBuffer())
												{
													subrec->SetRemoteRecordNumber(recnum);
												}
											}
										}
									}

								}
							}
						}
						else
						{
							if (cv->GetValueKind() == datakind)
							{
								err = cv->ReadFromStream(from);
							}
							else
							{
								VValueSingle* cv2 = (VValueSingle*)VValue::NewValueFromValueKind(datakind);
								if (cv2 != nil)
								{
									cv2->SetNull(false);
									err = cv2->ReadFromStream(from);
									if (err == VE_OK)
										cv2->GetValue(*cv);
									delete cv2;
								}
								else
									err = ThrowBaseError(memfull, noaction);
							}
						}
					}
				}
			}
		}
	} while(i != 0 && err == VE_OK);

	sLONG nb = (sLONG)fch.size();
	if (fchold.size() < nb)
		fchold.resize(nb, nil);
	for (i = 1; i <= nb; i++)
	{
		// L.E. 29/10/08 ACI0059619 leaves field value in chold[] if nothing in ch[]
		VValueSingle* cv = fch[i-1];
		if (cv != nil)
		{
			VValueSingle* cv2 = fchold[i-1];
			delete cv2;
			fchold[i-1] = cv->Clone();
		}
	}


	return err;
}


PrimKey* FicheInMem::RetainPrimKey(VError& outErr, bool oldOne)
{
	outErr = VE_OK;
	PrimKey* result = nil;
	if (asscrit->HasPrimKey())
	{
		result = new PrimKey();
		outErr = result->GetFrom(this, oldOne);
		if (outErr != VE_OK)
			ReleaseRefCountable(&result);
	}
	else
		outErr = asscrit->ThrowError(VE_DB4D_PRIMKEY_IS_NEEDED, noaction);

	return result;
}




																				 /* ========================================== */



FicheInMemSystem::FicheInMemSystem(BaseTaskInfo* Context, Base4D* xdb, Table* crit, VError& err):FicheInMem(Context, xdb, crit, err)
{
	isnew = false;
}


FicheInMemSystem::~FicheInMemSystem()
{
}





			         /* ========================================================================================== */


VError FicheOnDisk::ThrowError( VError inErrCode, ActionDB4D inAction) const
{
	Base4D* owner = GetDB();

	VErrorDB4D_OnRecord *err = new VErrorDB4D_OnRecord(inErrCode, inAction, owner , DF->GetNum(), numfic);
	VTask::GetCurrent()->PushRetainedError( err);
	
	return inErrCode;
}


				

bool FicheOnDisk::SaveObj(VSize& outSizeSaved)
{
#if debuglogwrite
	VString wherefrom(L"FicheOnDisk Saveobj");
	VString* whx = &wherefrom;
#else
	VString* whx = nil;
#endif

	VSize tot = 0;
	sLONG l,l2,nbcrit;
	DataAddr4D ou;
	VError err;
	
#if debuglr
	if (InTrans() && !fCommited)
		xbox_assert(!InTrans());
#endif
	nbcrit = fNbCrit;
	ou=getaddr();
#if debug_Addr_Overlap
	GetDB()->CheckDBObjRef(ou, fDataLen+sizeof(ChampHeader)*nbcrit+kSizeRecordHeader, debug_FicheRef(DF->GetNum(), numfic));
#endif
	RecordHeader tag(GetDataBegining(),fDataLen, DBOH_Record, getnumfic(), DF->GetTrueNum(), nbcrit);
	tag.SetTimeStamp(TimeStamp);
	err = tag.WriteInto(GetDB(), getaddr(), whx);
	//TimeStamp = tag.GetTimeStamp();
	if (err==VE_OK)
	{
		l = fDataLen;
		l2=sizeof(ChampHeader)*nbcrit;
		err=GetDB()->writelong(GetDataBegining(),l+l2,ou,kSizeRecordHeader, whx);
		if (err==VE_OK)
		{
			tot=kSizeRecordHeader+l+l2;
			DF->MeasureSavedRecord(tot);
		}
	}

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTSAVERECORD, DBaction_SavingRecord);

	outSizeSaved = tot;
	return true;
}


FicheOnDisk* FicheOnDisk::DuplicateRecord(const FicheOnDisk* from, uBOOL pourdouble)
{
	FicheOnDisk* rec = nil;

	void* p = GetFastMem(from->ComputeSizeInMem(), true, 'ficD');
	if (p != nil)
	{
		rec = new (p) FicheOnDisk(from, pourdouble);
	}

	return rec;
}


FicheOnDisk::FicheOnDisk(const FicheOnDisk* from, uBOOL pourdouble) // pour le copymem, appelee par DuplicateRecord
{
	sLONG l;
	StAllocateInCache	alloc;
	
	//original = nil;
	TimeStamp = 0;
	fModificationStamp = 0;
	DF=from->DF;
	fQueryValuesMutex = 0;
	//nbuse=0;
	//fInTrans = false;
#if debuglr
	fCommited = false;
#endif
	SetNewInTrans(false);
	SetInTrans(false);
	
	fPrimKey = RetainRefCountable(from->fPrimKey);
	l = from->fDataLen + from->GetChampHeaderSize();
	fDataLen = from->fDataLen;
	fNbCrit = from->fNbCrit;
	vBlockMove(from->GetDataBegining(), GetDataBegining(), l);
	
	if (!pourdouble)
	{
		antelen = from->antelen;
		numfic = from->numfic;
		setaddr(from->getaddr());
	}
}


FicheOnDisk* FicheOnDisk::BuildRecordFromLog(VError& err, DataTable *xDF, sLONG xnumfic, void* dataptr, sLONG datalen, sLONG nbfield, Boolean needswap )
{
	err = VE_OK;
	StAllocateInCache	alloc;
	FicheOnDisk* rec = nil;

	void* p = GetFastMem((sLONG)FicheOnDisk::ComputeRecordSizeInMem(datalen, nbfield), true, 'ficD');
	if (p!=nil)
	{
		rec = new (p) FicheOnDisk(xDF, xnumfic, dataptr, datalen, nbfield, needswap);
	}
	else
		err = xDF->ThrowError(memfull, DBaction_AllocatingRecordInMem);

	return rec;
}


FicheOnDisk* FicheOnDisk::BuildRecordFromLoad(VError& err, DataTable *xDF, sLONG xnumfic, DataAddr4D xaddr, ReadAheadBuffer* buffer )
{
	//FicheHeader tt;
	sLONG nbc,lenx;
	err = VE_OK;
	StAllocateInCache	alloc;
	FicheOnDisk* rec = nil;

	RecordHeader tag;
	err = tag.ReadFrom(xDF->GetDB(), xaddr, buffer);
	if (err==VE_OK)
	{
		if (tag.ValidateTag(DBOH_Record,xnumfic,xDF->GetTrueNum()) == VE_OK)
		{
			nbc = tag.GetNbFields();
			lenx=tag.GetLen();
			void* p = GetFastMem((sLONG)FicheOnDisk::ComputeRecordSizeInMem(lenx, nbc), true, 'ficD');
			if (p!=nil)
			{
				rec = (FicheOnDisk*)p; // uniquement pour pouvoir appeller GetDataBegining sur l'objet, cette methode n'a pas besoin des autre champs
				err=xDF->GetDB()->readlong(rec->GetDataBegining(),lenx+(nbc*sizeof(ChampHeader)),xaddr,kSizeRecordHeader, buffer);
				xDF->MeasureLoadedRecord(lenx+(nbc*sizeof(ChampHeader))+kSizeRecordHeader);
				if (err==VE_OK)
				{
					err = tag.ValidateCheckSum(rec->GetDataBegining(),lenx);
					if (err != VE_OK)
					{
						FreeFastMem(p);
						rec = nil;
						err = xDF->ThrowError(VE_DB4D_WRONGRECORDHEADER, DBaction_LoadingRecord);
						//err = VE_DB4D_WRONGRECORDHEADER;
					}
					else
					{
						rec = new (p) FicheOnDisk(xDF, xnumfic, xaddr, tag);
					}
				}
				else
				{
					FreeFastMem(p);
					rec = nil;
				}
			}
			else
			{
				err = xDF->ThrowError(memfull, DBaction_LoadingRecord);
			}
		}
		else
		{
			err = xDF->ThrowError(VE_DB4D_WRONGRECORDHEADER, DBaction_LoadingRecord);
		}
	}
#if debuglr
	if (err != VE_OK)
		sLONG xdebug = 1; // put a break here
#endif

	/*
	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTLOADRECORD, DBaction_LoadingRecord);
	*/

	return rec;
}



FicheOnDisk* FicheOnDisk::BuildRecordFromLoad(VError& err, sLONG DataTableNum, sLONG xnumfic, DataAddr4D xaddr, VFileDesc* f, DataTable *xDF )
{
	//FicheHeader tt;
	sLONG nbc,lenx;
	err = VE_OK;
	FicheOnDisk* rec = nil;

	RecordHeader tag;
	err = tag.ReadFrom(f, xaddr);
	if (err==VE_OK)
	{
		if (tag.ValidateTag(DBOH_Record,xnumfic,DataTableNum) == VE_OK)
		{
			nbc = tag.GetNbFields();
			lenx=tag.GetLen();
			void* p = GetFastMem((sLONG)FicheOnDisk::ComputeRecordSizeInMem(lenx, nbc), true, 'ficD');
			if (p!=nil)
			{
				rec = (FicheOnDisk*)p; // uniquement pour pouvoir appeller GetDataBegining sur l'objet, cette methode n'a pas besoin des autre champs
				err=f->GetData(rec->GetDataBegining(),lenx+(nbc*sizeof(ChampHeader)),xaddr+kSizeRecordHeader);
				if (err==VE_OK)
				{
					err = tag.ValidateCheckSum(rec->GetDataBegining(),lenx);
					if (err != VE_OK)
					{
						FreeFastMem(p);
						rec = nil;
						err = VE_DB4D_WRONGRECORDHEADER;
					}
					else
					{
						rec = new (p) FicheOnDisk(xDF, xnumfic, xaddr, tag, true);
					}
				}
				else
				{
					FreeFastMem(p);
					rec = nil;
				}
			}
			else
			{
				err = memfull;
			}
		}
		else
		{
			err = VE_DB4D_WRONGRECORDHEADER;
		}
	}

	return rec;
}



FicheOnDisk::FicheOnDisk(DataTable *xDF, sLONG xnumfic, DataAddr4D xaddr, RecordHeader& tag, bool allowForbiddenAddr) // appele apres avoir charge la fiche dand BuildRecordFromLoad
{	
	//original = nil;
	//nbuse=0;
	DF=xDF;
	numfic=xnumfic;
	if (allowForbiddenAddr)
		SetInProtectedArea();
	setaddr(xaddr);
	fModificationStamp = 0;
	fQueryValuesMutex = 0;
	//fInTrans = false;
#if debuglr
	fCommited = false;
#endif
	SetNewInTrans(false);
	SetInTrans(false);

	fPrimKey = nil;
	fNbCrit = tag.GetNbFields();
	TimeStamp = tag.GetTimeStamp();
	fDataLen = tag.GetLen();
	antelen=fDataLen+GetChampHeaderSize();

	if (tag.NeedSwap())
	{
		SwapBytes();
	}
}


FicheOnDisk::FicheOnDisk(DataTable *xDF, sLONG xnumfic, void* dataptr, sLONG datalen, sLONG nbfield, Boolean needswap) // appele apres avoir charge la fiche dand BuildRecordFromLog
{
	//original = nil;
	TimeStamp = 0;
	//nbuse=0;
	DF=xDF;
	numfic=xnumfic;
	fModificationStamp = 0;
	fQueryValuesMutex = 0;
	//fInTrans = false;
#if debuglr
	fCommited = false;
#endif
	SetNewInTrans(false);
	SetInTrans(false);

	fPrimKey = nil;
	fNbCrit = nbfield;
	fDataLen = datalen;
	antelen=0;
	vBlockMove(dataptr, GetDataBegining(), datalen+(nbfield*sizeof(ChampHeader)));

	if (needswap)
	{
		SwapBytes();
	}
}


void ChampHeader::SwapBytes()
{
	ByteSwap(&offset);
	ByteSwap(&typ);
}


void FicheOnDisk::SwapBytes()
{
	ByteSwapCollection(GetChampHeader(1), fNbCrit);
	sLONG i;
	for (i = 1; i<=fNbCrit; i++)
	{
		uCHAR* p = (uCHAR*)GetDataPtr(i); // peut retourner nil si valeur est NULL
		ChampHeader* ch = GetChampHeader(i);
		if (p != nil)
		{
			if (ch->typ == (sLONG)VK_BLOB)
				ch->typ = (sLONG)VK_BLOB_DB4D;
			const VValueInfo* xinfo = VValue::ValueInfoFromValueKind((ValueKind)ch->typ);
			if (xinfo != nil)
				xinfo->ByteSwapPtr((void*)p, false);
		}

	}
}


FicheOnDisk::FicheOnDisk(Base4D *xdb, DataTable *xDF, Table* crit)  // equivalent de initmem
{
	StAllocateInCache	alloc;

	//original = nil;
	//origaddr = 0;
	TimeStamp = 0;
	fModificationStamp = 0;
	fQueryValuesMutex = 0;
	//isnew=true;
	//nbuse=0;
	//fData=nil;
	fPrimKey = nil;
	fDataLen = 0;
	antelen=0;
	DF=xDF;
	numfic=-3;
	fNbCrit = 0;
	SetNewInTrans(false);
	SetInTrans(false);
	//fInTrans = false;
#if debuglr
	fCommited = false;
#endif
	//danscache=false;
	
}

/*
VError FicheOnDisk::Detach(BaseTaskInfo* Context)
{
	VError err = VE_OK;
	occupe();

	numfic = -3;
	antelen = 0;
	setaddr(0, false);

	libere();
	return err;
}
*/


FicheOnDisk* FicheOnDisk::Clone(VError& err, Boolean ForPush) const
{
	err = VE_OK;
	FicheOnDisk* lacopy = nil;

	void* p = GetFastMem(ComputeSizeInMem(), true, 'ficD');
	if (p == nil)
		err = ThrowError(memfull, DBaction_AllocatingRecordInMem);
	else
	{
		lacopy=new (p)FicheOnDisk(this, true);
		if (ForPush)
		{
			lacopy->numfic = numfic;
			lacopy->antelen = antelen;
			lacopy->setaddr(getaddr());
		}
		else
		{
			lacopy->numfic = -3;
			lacopy->antelen = 0;
			lacopy->ResetAddr();
		}

	}
	return lacopy;
}

/*
void FicheOnDisk::FreeAfterUse()
{
	unuse();
	if (okdel2())
	{
		libere();
		delete this;
	}
	else libere();
}
*/


sLONG FicheOnDisk::GetActualFieldLen(sLONG n)
{
	sLONG res;
	
	xbox_assert( (n>0) && (n<=fNbCrit) );
	
	bool doit = true;
	// L.E. 25/10/05 pour les tools, on n'a pas de Table*
	Table* fic = DF->RetainTable();
	if (fic != NULL)
	{
		Field* cri = fic->RetainField(n);
		if (cri == nil)
		{
			res = 0;
			doit = false;
		}
		else
		{
			cri->Release();
		}
		fic->Release();
	}
	
	if (doit)
	{
		sLONG off2;
		if (n == fNbCrit) 
			off2 = fDataLen;
		else 
			off2 = GetChampHeader(n+1)->offset;
			
		res = off2 - GetChampHeader(n)->offset;
	}
	
	xbox_assert(res >=0);
	
	return res;
	
}

#if debug_blobOrphans
void FicheOnDisk::debugBlobOrphans()
{
	bool once = false;
	for (sLONG i = 1; i <= fNbCrit; ++i)
	{
		sLONG typ;
		sLONG* p = (sLONG*)GetDataPtr(i, &typ);
		if (p != nil)
		{
			if (typ == VK_BLOB_DB4D || typ == VK_BLOB || typ == VK_IMAGE || typ == VK_TEXT || typ == VK_BLOB_OBJ)
			{
				sLONG len = GetActualFieldLen(i);
				if (len == 4)
				{
					DF->GetDB()->WriteDebugLogLn("\t\tfield: "+ToString(i)+" , blobnum: "+ToString(*p));
					once = true;
				}
			}
		}
	}
	if (once)
	{
		DF->GetDB()->WriteDebugLogLn("");
	}
}
#endif

FicheOnDisk* FicheOnDisk::ReajusteData(FicheInMem *owner, VError& err, Boolean MayKeepOriginal)
{
	FicheOnDisk* result = nil;
	sLONG newlen = 0;
	sLONG i;
	sLONG nb;
	void* newData = nil;
	void* newrec = nil;
	Boolean samelength = true;
	VError err2;
	Table* assocTable = nil;
	//ChampHeaderArray newchd;
	
	err = VE_OK;
	// si la fiche est nouvelle alors fData sera nil et chd sera un tableau de zero elements (pas de headers)
	
	
	// trouvons le maximum de champs a scanner
	// ce doit etre au maximum le nombre de champs du fichier, mais ce peut etre moins car on s'arrete au dernier remplis
	// c'est a dire le max de CV et de ChampHeader
	
	assocTable = DF->RetainTable();
	if (assocTable == nil)
		nb = 0;
	else
		nb = assocTable->GetNbCrit();
	
	sLONG nb1 = owner->GetMaxCV();
	if (nb1>nb) nb1 = nb; // ne devrait jamais se produire car le getnthfield ne doit etre appeler que si n <= nbcrit
	sLONG nb2 = fNbCrit;
	if (nb2>nb)
	{
		nb2 = nb; // peut se produire si l'ancienne fiche sur disque avait plus de champs que nbcrit
		samelength = false;
	}
	
	for (i=nb2; i>0; i--)
	{
		if (GetActualFieldLen(i) == 0)
			nb2--; // on supprime a la fin les champs non remplis sur disque
		else
			break;
	}
	
	// maintenant on calcul l'espace requis sur disque
	
	if (nb1>nb2)
		nb = nb1;
	else
		nb = nb2;

	sLONG nbx = owner->GetMaxCV();
	if (nbx > fNbCrit)
	{
		samelength = false;
		for (i = fNbCrit+1; i<= nbx && err == VE_OK; i++)
		{
			ValPtr cv =  owner->GetNthField(i, err); // on force a creer les champs non initilises a nil
		}
	}
		
	if (err == VE_OK)
	{
		for (i = 1; i<=nb; i++)
		{
			ValPtr cv = owner->GetNthField(-i, err2); // on cherche si le champs a une valeur en memoire
			// tester ici les champs nevernull

			if (cv == nil || !owner->IsModif(i))
			{
				// sinon on cherche la taille du champs enregistree sur disque (si ce n'est pas une nouvelle fiche)
				if (i<=fNbCrit)
				{
					newlen = newlen + GetActualFieldLen(i);
				}
			}
			else
			{
#if 0
				// may never be needed, just written to remember I could need it : L.R on the 15/07/2014
				sLONG truncLen = assocTable->GetTruncateLength(i);
				if (truncLen > 0 && cv->GetValueKind() == VK_STRING)
				{
					VString* s = dynamic_cast<VString*>(cv);
					if (s != nil)
					{
						if (truncLen > s->GetLength())
						{
							s->Truncate(truncLen);
							samelength = false;
						}
					}
				}
#endif
					
				sLONG xnewlen;
				if (cv->IsNull())
				{
					xnewlen = 0;
				}
				else
				{
					xnewlen = (sLONG) cv->GetSpace();
				}
				newlen = newlen + xnewlen;
				if (i<=fNbCrit)
				{
					if (xnewlen != GetActualFieldLen(i))
						samelength = false;
				}
				else
					samelength = false;
			}
		}
		
		if (nb != fNbCrit)
			samelength = false;

		if (samelength && MayKeepOriginal) // quand chaque champ modifie ne change pas de taille, alors on peut reecrire dans la meme fiche
		{
			result = this;
			for (i = 1; i <= fNbCrit; i++)
			{
				ValPtr cv = owner->GetNthField(-i, err2); // on cherche si le champs a une valeur en memoire (c'est a dire qu'il a ete modifie)
				// tester ici les champs nevernull
				if (cv != nil)
				{
					if (cv->IsNull())
					{
						GetChampHeader(i)->typ = DB4D_NullType;
					}
					else
					{
						GetChampHeader(i)->typ = (sLONG)cv->GetTrueValueKind();
						cv->WriteToPtr(((char*)GetDataBegining())+GetChampHeader(i)->offset);
					}
				}
			}
		}
		else // sinon on a beaucoup a faire, car il faut tout recopier
		{
			//if (newlen == 0) newlen = 1;
			
			newrec = GetFastMem((sLONG)FicheOnDisk::ComputeRecordSizeInMem(newlen, nb), true, 'ficD');
			sLONG newnbcrit = nb;

			if (newrec == nil )
			{
				if (err == VE_OK)
					err = ThrowError(memfull, DBaction_UpdatingRecord);
			}
			else
			{
				newData = ((FicheOnDisk*)newrec)->GetDataBegining();
				ChampHeader* newchd = (ChampHeader*)(((char*)newData)+newlen) - 1;
				sLONG curoff = 0;
				sLONG lastcuroff = 0;
				sLONG newcuroff = 0;
				sLONG newlastcuroff = 0;
				
				for (i = 1; i<=nb && err == VE_OK; i++)
				{
					Field* cri = assocTable == nil ? nil : assocTable->RetainField(i);
					
					newchd[i].offset = newcuroff;
					
					if (cri == nil) // si le champs est detruit
					{
						newchd[i].typ = DB4D_NullType;
						// on recopie de l'ancienne fiche a partir de lastcuroff
						if ((curoff - lastcuroff) > 0)
							vBlockMove( ((char*)GetDataBegining())+lastcuroff, ((char*)newData)+newlastcuroff, curoff - lastcuroff);
						
						newlastcuroff = newcuroff;
						if (i<fNbCrit)
						{
							lastcuroff = GetChampHeader(i+1)->offset;
							curoff = lastcuroff;
						}
						else
						{
							lastcuroff = fDataLen;
							curoff = fDataLen;
						}
						
					}
					else
					{
						ValPtr cv = owner->GetNthField(-i, err2); // on cherche si le champs a une valeur en memoire
						// tester ici les champs nevernull
						if (cv == nil || !owner->IsModif(i))
						{
							if (i<=fNbCrit)
							{
								newchd[i].typ = GetChampHeader(i)->typ; // on garde le type qui est sur disque meme si il est different du champs car celui ci est non modifié
								sLONG len =GetActualFieldLen(i);
								curoff = curoff + len;
								newcuroff = newcuroff + len;
							}
							else
							{
								//newchd[i].typ = cri->GetTyp();
								newchd[i].typ = DB4D_NullType;
							}
						}
						else
						{
							newchd[i].typ = /*cri->GetTyp()*/ (sLONG)cv->GetTrueValueKind();
			
							// on recopie de l'ancienne fiche a partir de lastcuroff
							if ((curoff - lastcuroff) > 0)
								vBlockMove( ((char*)GetDataBegining())+lastcuroff, ((char*)newData)+newlastcuroff, curoff - lastcuroff);
								
							// puis on place la nouvelle valeur du champs
							VSize len;
							if (cv->IsNull())
							{
								len = 0;
								newchd[i].typ = DB4D_NullType;
							}
							else
							{
								len = cv->GetSpace();
								cv->WriteToPtr(((char*)newData)+newcuroff);
							}
							newcuroff = newcuroff + (sLONG) len;
							newlastcuroff = newcuroff;
							
							if (i<fNbCrit)
							{
								lastcuroff = GetChampHeader(i+1)->offset;
								curoff = lastcuroff;
							}
							else
							{
								lastcuroff = fDataLen;
								curoff = fDataLen;
							}
							
						}
					}
					
					if (cri != nil) cri->Release();
				}
				
				// on recopie de l'ancienne fiche a partir de lastcuroff (si il reste un reliquat)
				if ((curoff - lastcuroff) > 0)
					vBlockMove( ((char*)GetDataBegining())+lastcuroff, ((char*)newData)+newlastcuroff, curoff - lastcuroff);

				/*
				if (nbuse == 1) // si la fiche sur disque n'est pas referencee par une autre ficheinmem (en lecture seulement bien sur)
				{
					if (fData != nil)
						FreeFastMem(fData);
					fData = newData;
					fDataLen = newlen;
					fNbCrit = newchd.GetCount();
					vBlockMove(newchd.First(), GetChampHeader(1), GetChampHeaderSize());
					result = this;
				}
				else
				*/
				{
					result = new (newrec) FicheOnDisk();
					if (result == nil)
						err = ThrowError(memfull, DBaction_UpdatingRecord); // should NEVER happen
					else
					{
						result->fDataLen = newlen;
						result->fNbCrit = newnbcrit;
						result->DF = DF;
						//result->nbuse = 1; // va etre pris par le ficheinmem owner
						result->numfic = numfic;
						result->antelen = antelen;
						result->TimeStamp = TimeStamp;
						result->fPrimKey = RetainRefCountable(fPrimKey);

						if (!owner->IsSystem())
						{
							result->setaddr(getaddr());
							result->SetNewInTrans(IsNewInTrans());
							result->SetInTrans(InTrans());

							if (MayKeepOriginal)
								setmodif(false, GetOwner()->GetDB(), nil);
						}
					}
				}
				
			}
		}
	}

	if (assocTable != nil)
		assocTable->Release();

	if (result != nil)
	{
		VTime aTime;
		aTime.FromSystemTime();
		result->TimeStamp = aTime.GetStamp();
	}

	if (err != VE_OK)
	{
		if (numfic == -3)
			err = ThrowError(VE_DB4D_CANNOTUPDATENEWRECORD, DBaction_UpdatingRecord);
		else
			err = ThrowError(VE_DB4D_CANNOTUPDATERECORD, DBaction_UpdatingRecord);
	}

	return result;
}

/*
uBOOL FicheOnDisk::okdel2(void)
{
	return( (nbuse==0) && !IsDansCache() && !modifie() );
}

uBOOL FicheOnDisk::okdel(void)
{
	return(((nbuse==0) && (!modifie()) && (!DF->PasToucheFiche())) );
}
*/

sLONG FicheOnDisk::calclen(void)
{
	sLONG tot;
	Table* crit;

	tot=fDataLen+GetChampHeaderSize();
	
	return(tot);
}


FicheOnDisk::FicheOnDisk(DataTable* owner, const RecTempHeader& From)
{
	DF = owner;
	setaddr(From.fAddr, false);
	fDataLen = From.fDataLen;
	numfic = From.fNumfic;
	antelen = From.fAntelen;
	fModificationStamp = From.fModificationStamp;
	fQueryValuesMutex = 0;
	//nbuse = 0;
	fPrimKey = nil;
	fNbCrit = From.fNbCrit;
	TimeStamp = From.fTimeStamp;
	SetNewInTrans(From.fNewInTrans);
	SetInTrans(true);
}


void FicheOnDisk::PutHeaderInto(RecTempHeader& into)
{
	into.fAddr = getaddr();
	into.fDataLen = fDataLen;
	into.fNumfic = numfic;
	into.fAntelen = antelen;
	into.fModificationStamp = fModificationStamp;
	into.fNbCrit = fNbCrit;
	into.fTimeStamp = TimeStamp;
	into.fNewInTrans = IsNewInTrans();
}


FicheOnDisk::~FicheOnDisk()
{
#if debuglr
	if (modifie())
	{
		xbox_assert(!modifie());
	}
	if (GetRefCount() > 0)
	{
		xbox_assert(GetRefCount() == 0);
	}
#endif
	QuickReleaseRefCountable(fPrimKey);
	for (vector<void*>::iterator cur = fQueryValues.begin(), end = fQueryValues.end(); cur != end; cur++)
	{
		void* p = *cur;
		if (p != nil)
			FreeFastMem(p);
	}
#if debugFicheOnDisk
	gDebugFiches.CheckAssocData(this);
#endif
#if debuglr == 11
	debug_checktreerec(this);
#endif
#if debugfiche_strong
	DF->CheckRecordRef(this);
#endif
}


tPtr FicheOnDisk::GetDataPtr(sLONG n, sLONG* WhatType)
{
	tPtr result = nil;
	/*
	if (fData == nil)
	{
		return nil;
	}
	else
	*/
	{
		if (n>fNbCrit || n <= 0)
		{
			//result = nil;
		}
		else
		{
			sLONG off2;
			if (n==fNbCrit) 
				off2 = fDataLen;
			else
				off2 = GetChampHeader(n+1)->offset;
			sLONG off = GetChampHeader(n)->offset;
			if ((off2-off)>0)
			{
				if (WhatType != nil)
					*WhatType = GetChampHeader(n)->typ;
				return(((tPtr)GetDataBegining())+off);
			}
			else
			{
				//result = nil; // si le champs ne prend pas de place dans la fiche
			}
		}
	}

	if (result == nil)
	{
		if (WhatType != nil)
			*WhatType = -1;
	}

	return result;
}



tPtr FicheOnDisk::GetDataPtrForQuery(sLONG n, sLONG* WhatType, bool forceNonNull, bool checkForStyle)
{
	tPtr result = nil;
	bool isadate = false;
	{
		if (n>fNbCrit || n <= 0)
		{
			//result = nil;
		}
		else
		{
			sLONG off2;
			if (n==fNbCrit) 
				off2 = fDataLen;
			else
				off2 = GetChampHeader(n+1)->offset;
			sLONG off = GetChampHeader(n)->offset;
			if ((off2-off)>0)
			{
				isadate = (GetChampHeader(n)->typ == VK_TIME);
				if (WhatType != nil)
					*WhatType = GetChampHeader(n)->typ;
				if (GetChampHeader(n)->typ == VK_TEXT)
				{
					sLONG numblob = *((sLONG*)(((tPtr)GetDataBegining())+off));
					if (numblob == -1)
						result = (tPtr) Table::xGetEmptyPtr(VK_TEXT);
					else
					{
						//DF->LoadBlob(numblob); // a faire
						//result = (tPtr) Table::xGetEmptyPtr(VK_TEXT);
						result = ((tPtr)GetDataBegining())+off;
					}
				}
				else
				{
					result = ((tPtr)GetDataBegining())+off;
				}
			}
			else
			{
				//result = nil; // si le champs ne prend pas de place dans la fiche
			}
		}
	}

	if (result == nil)
	{
		if (forceNonNull)
		{
			if (WhatType != nil)
				*WhatType = -1;
			Table* fic = DF->GetTable();
			if (fic != nil)
			{
				if (fic->IsNeverNull(n))
				{
					Field* cri =fic->RetainField(n);
					if (cri != nil && cri->GetTyp() != VK_TIME)
					{
						result = (tPtr) cri->GetEmptyValPtr();
					}
				}
				//fic->Release();
			}
		}
	}
	else
	{
		if (checkForStyle && DF != nil && DF->GetTable()->IsFieldStyledText(n) && WhatType != nil && (*WhatType == VK_STRING || *WhatType == VK_STRING_UTF8))
		{
			SpinLockFiber(fQueryValuesMutex);
			if (n > fQueryValues.size())
				fQueryValues.resize(n, nil);
			void* p = fQueryValues[n-1];
			if (p == nil)
			{
				VString s, s2;
				s.LoadFromPtr(result);
				VSpanTextParser::Get()->GetPlainTextFromSpanText(s, s2);
				sLONG len = (sLONG)s2.GetSpace();
				p = GetFastMem(len, false, 'txt_');
				s2.WriteToPtr(p);
				fQueryValues[n-1] = p;
			}
			result = (tPtr)p;

			SpinUnlock(fQueryValuesMutex);
		}
	}

	/*
	if (result != nil && isadate)
	{
		if (*((uLONG8*)result) == 0)
			result = nil;
	}
	*/

	return result;
}


/*
void FicheOnDisk::lock(void)
{
	use();
}

void FicheOnDisk::unlock(void)
{
	unuse();
}
*/


VError FicheOnDisk::WriteSaveToLog(BaseTaskInfo* context, Boolean newone, FicheInMem* fic, PrimKey* oldPrimKey)
{
	VError err = VE_OK;
	if (DF->GetTable()->CanBeLogged())
	{
		PrimKey* primkey = RetainRefCountable(oldPrimKey);
		if (primkey == nil)
		{
			if (fic == nil)
				primkey = RetainPrimKey();
			else
				primkey = fic->RetainPrimKey(err, true);
		}
		if (primkey != nil)
		{
			sLONG lenkey = primkey->GetLength();
			VStream* log;
			Base4D* db = DF->GetDB();
			sLONG len = lenkey + fDataLen + fNbCrit*sizeof(ChampHeader) + sizeof(RecordHeaderOnDisk) + sizeof(VUUIDBuffer);
			Table* tt = DF->GetTable();

			err = db->StartWriteLog(newone ? DB4D_Log_CreateRecordWithPrimKey : DB4D_Log_ModifyRecordWithPrimKey, len, context, log, true, true);
			if (err == VE_OK)
			{
				if (log != nil)
				{
					sLONG l,l2,nbcrit;
					nbcrit = fNbCrit;
					RecordHeader tag(GetDataBegining(),fDataLen, DBOH_Record, getnumfic(), DF->GetTrueNum(), nbcrit);
					err = primkey->PutInto(log);
					if (err == VE_OK)
						err = tag.WriteToStream(log);
					if (err == VE_OK)
					{
						VUUID xid;
						tt->GetUUID(xid);
						err = xid.WriteToStream(log);
					}
					if (err==VE_OK)
					{
						l = fDataLen;
						err=log->PutData(GetDataBegining(),l);
						if (err==VE_OK)
						{
							l2=sizeof(ChampHeader)*nbcrit;
							err=log->PutData(GetChampHeader(1),l2); 
						}
					}
				}

				VError err2 = db->EndWriteLog(len, true);
				if (err == VE_OK)
					err = err2;
			}
			primkey->Release();
		}
		else
			err = ThrowError(VE_DB4D_LOG_NEEDS_A_VALID_PRIMARYKEY, noaction);

	}
	DF->GetDB()->ReleaseLogMutex();

	return err;
}


VError FicheOnDisk::WriteDeleteToLog(BaseTaskInfo* context, FicheInMem* fic)
{
	VError err = VE_OK;
	if (fic->GetOwner()->CanBeLogged())
	{
		PrimKey* primkey = fic->RetainPrimKey(err, true);
		if (primkey != nil)
		{
			sLONG lenkey = primkey->GetLength();
			VStream* log;
			Base4D* db = DF->GetDB();
			sLONG len = lenkey + 4 /*numrecord*/ + sizeof(VUUIDBuffer) /*numtable*/;
			Table* tt = DF->GetTable();

			err = db->StartWriteLog(DB4D_Log_DeleteRecordWithPrimKey, len, context, log, true, true);
			if (err == VE_OK)
			{
				if (log != nil)
				{
					err = primkey->PutInto(log);
					if (err == VE_OK)
						err = log->PutLong(getnumfic());
					if (err == VE_OK)
					{
						VUUID xid;
						tt->GetUUID(xid);
						//err = log->PutLong(DF->GetTrueNum());
						err = xid.WriteToStream(log);
					}
				}
				VError err2 = db->EndWriteLog(len, true);
				if (err == VE_OK)
					err = err2;
			}
			primkey->Release();
		}
		else
			err = ThrowError(VE_DB4D_LOG_NEEDS_A_VALID_PRIMARYKEY, noaction);

	}
	DF->GetDB()->ReleaseLogMutex();

	return err;
}







#if 0
BaseTaskInfo* FicheOnDisk::WhoLockedIt(DB4D_KindOfLock& outLockType, BaseTaskInfo* context, const VValueBag **outLockingContextRetainedExtraData) const
{
	BaseTaskInfo* result = DF->WhoLockedRecord(numfic, outLockType, context, outLockingContextRetainedExtraData);

	return result;
}
#endif












