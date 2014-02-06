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
#include "DB4D/Headers/DB4D.h"
#include "Scan.h"

using namespace std;


Boolean ObjectDiskBitInfo::IsOn(sLONG8 n)
{
	Boolean res = false;
	sLONG8 n2 = n / 0x40000000; // 2^30 = 1073741824 = 1 Gb
	if ((sLONG)n2 >= (sLONG)fEspace.size())
		return false;
	else
	{
		sLONG n3 = (sLONG)n & 0x3FFFFFFF;
		PartObjectDiskBitInfo* part = &fEspace[(sLONG)n2];
		if (part->fBits == nil)
			return false;
		else
			return part->fBits->isOn(n3);
	}
}

void ObjectDiskBitInfo::Set(sLONG8 n)
{
	try
	{
		sLONG8 n2 = n / 0x40000000; // 2^30 = 1073741824 = 1 Gb
		if ((sLONG)n2 >= (sLONG)fEspace.size())
			fEspace.resize((sLONG)n2+1);
		sLONG n3 = (sLONG)n & 0x3FFFFFFF;
		PartObjectDiskBitInfo* part = &fEspace[(sLONG)n2];
		if (part->fBits == nil)
			part->fBits.Adopt(new Bittab);
		if (part->fBits->Set(n3) != VE_OK)
			fErr = memfull;
		part->nbpassages++;
		if (part->nbpassages > 1000)
		{
			part->nbpassages = 0;
			part->fBits->Epure();
		}
	}
	catch (...)
	{
		fErr = memfull;
	}
}


ObjectDiskPosInfoCollection::~ObjectDiskPosInfoCollection()
{
	if (fAddrs != nil)
		fAddrs->Release();
	if (fContext != nil)
		fContext->Release();
	VDBMgr::GetManager()->FlushCache( true);
	if (fDatasEncapsule != nil)
	{
		fDatasEncapsule->CloseAndRelease();
	}

	if (fDeleteTempBaseOnClose)
	{
		if (f1 != nil)
			f1->Delete();

		if (f2 != nil)
			f2->Delete();

		if (f3 != nil)
			f3->Delete();

		if (f4 != nil)
			f4->Delete();
	}

	QuickReleaseRefCountable(f1);
	QuickReleaseRefCountable(f2);
	QuickReleaseRefCountable(f3);
	QuickReleaseRefCountable(f4);
	/*
	if (fDatas != nil)
	{
		VSyncEvent* waitclose = new VSyncEvent();
		fDatas->BeginClose(waitclose);
		VDBMgr::GetManager()->UnRegisterBase(fDatas);
		fDatas->Release();

		if (waitclose != nil)
		{
			waitclose->Lock();
			waitclose->Release();
		}
	}
	*/
}


VError ObjectDiskPosInfoCollection::Init(Base4D_NotOpened* source)
{
	VError err = VE_OK;
	if (!fWithBitsOnly)
	{
		VString s,s2,s3,s4;
		VFolder* where = source->GetName(s);
		xbox_assert(where != nil);
		s += L"_infos.4db";
		f1 = new VFile(*where, s);
		f1->GetNameWithoutExtension(s2);
		f2 = new VFile(*where, s2+kDataFileExt);
		f3 = new VFile(*where, s2+kDataIndexExt);
		f4 = new VFile(*where, s2+kStructIndexExt);

		f1->Delete();
		f2->Delete();
		f3->Delete();
		f4->Delete();

		fDatasEncapsule = VDBMgr::GetManager()->CreateBase(*f1, DB4D_Create_DefaultData + DB4D_Create_WithSeparateIndexSegment, XBOX::VIntlMgr::GetDefaultMgr(), &err);
		if (fDatasEncapsule != nil)
		{
			fDatas = dynamic_cast<VDB4DBase*>(fDatasEncapsule)->GetBase();
			fContext = new BaseTaskInfo(fDatas, nil,nil, nil);

			Table* t;
			Field* f[11];

			t = new TableRegular(fDatas,0,false);
			t->SetName(L"ObjectPositions", false);

			Field* *p = &f[0];

			*p = new Field(DB4D_Integer16, SegNum_FieldID, t);
			(*p)->SetName(L"SegNumber", nil);
			(*p)->SetUUID(VUUID(true));
			p++;

			*p = new Field(DB4D_Integer64, Offset_FieldID, t);
			(*p)->SetName(L"DiskAddr", nil);
			(*p)->SetUUID(VUUID(true));
			p++;

			*p = new Field(DB4D_Integer32, Len_FieldID, t);
			(*p)->SetName(L"Len", nil);
			(*p)->SetUUID(VUUID(true));
			p++;

			*p = new Field(DB4D_Integer32, Type_FieldID, t);
			(*p)->SetName(L"ObjectType", nil);
			(*p)->SetUUID(VUUID(true));
			p++;

			*p = new Field(DB4D_Integer32, Param1_FieldID, t);
			(*p)->SetName(L"Param1", nil);
			(*p)->SetUUID(VUUID(true));
			p++;

			*p = new Field(DB4D_Integer32, Param2_FieldID, t);
			(*p)->SetName(L"Param2", nil);
			(*p)->SetUUID(VUUID(true));
			p++;

			*p = new Field(DB4D_Integer32, Param3_FieldID, t);
			(*p)->SetName(L"Param3", nil);
			(*p)->SetUUID(VUUID(true));
			p++;

			*p = new Field(DB4D_Integer32, Param4_FieldID, t);
			(*p)->SetName(L"Param4", nil);
			(*p)->SetUUID(VUUID(true));
			p++;

			*p = new Field(DB4D_Integer32, Numtable_FieldID, t);
			(*p)->SetName(L"TableNum", nil);
			(*p)->SetUUID(VUUID(true));
			p++;

			*p = new Field(DB4D_Integer32, Numindex_FieldID, t);
			(*p)->SetName(L"IndexNum", nil);
			(*p)->SetUUID(VUUID(true));
			p++;

			t->AddFields(&f[0], 10, nil, true, nil);

			t->SetUUID(VUUID(true).GetBuffer());
			t->CanNowBeSaved();
			err = fDatas->AddTable(t, true, nil);

			t->Release();

			p = &f[0];
			for (sLONG i=0;i < 10;i++)
			{
				(*p)->Release();
				p++;
			}

			fAddrs = fDatas->RetainTable(1);
		}

		where->Release();
	}

	return err;
}


FicheInMem* ObjectDiskPosInfoCollection::PrepareNewObject(DataAddr4D ou, sLONG len, sLONG type)
{
	xbox_assert(!fWithBitsOnly);

	VError err = VE_OK;
	FicheInMem* rec = fAddrs->GetDF()->NewRecord(err, fContext);
	if (rec != nil)
	{
		VValueSingle* cv = rec->GetNthField(Offset_FieldID, err);
		if (err == VE_OK && cv != nil)
			cv->FromLong8(StripDataAddr(ou));

		if (err == VE_OK)
		{
			cv = rec->GetNthField(SegNum_FieldID, err);
			if (err == VE_OK && cv != nil)
				cv->FromLong(ou & (kMaxSegData-1));
		}

		if (err == VE_OK)
		{
			cv = rec->GetNthField(Len_FieldID, err);
			if (err == VE_OK && cv != nil)
				cv->FromLong((len+kMaxSegData-1)&(-kMaxSegData));
		}

		if (err == VE_OK)
		{
			cv = rec->GetNthField(Type_FieldID, err);
			if (err == VE_OK && cv != nil)
				cv->FromLong(type);
		}
	}

	SetLastError(err);
	return rec;
}


FicheInMem* ObjectDiskPosInfoCollection::PrepareNewObject(DataAddr4D ou, sLONG len, sLONG type, sLONG param1)
{
	VError err = VE_OK;
	FicheInMem* rec = PrepareNewObject(ou, len, type);

	if (rec != nil && GetLastError() == VE_OK)
	{
		VValueSingle* cv = rec->GetNthField(Param1_FieldID, err);
		if (err == VE_OK && cv != nil)
			cv->FromLong(param1);
	}

	SetLastError(err);
	return rec;
}


FicheInMem* ObjectDiskPosInfoCollection::PrepareNewObject(DataAddr4D ou, sLONG len, sLONG type, sLONG param1, sLONG param2)
{
	VError err = VE_OK;
	FicheInMem* rec = PrepareNewObject(ou, len, type, param1);

	if (rec != nil && GetLastError() == VE_OK)
	{
		VValueSingle* cv = rec->GetNthField(Param2_FieldID, err);
		if (err == VE_OK && cv != nil)
			cv->FromLong(param2);
	}

	SetLastError(err);
	return rec;
}


FicheInMem* ObjectDiskPosInfoCollection::PrepareNewObject(DataAddr4D ou, sLONG len, sLONG type, sLONG param1, sLONG param2, sLONG param3)
{
	VError err = VE_OK;
	FicheInMem* rec = PrepareNewObject(ou, len, type, param1, param2);

	if (rec != nil && GetLastError() == VE_OK)
	{
		VValueSingle* cv = rec->GetNthField(Param3_FieldID, err);
		if (err == VE_OK && cv != nil)
			cv->FromLong(param3);
	}

	SetLastError(err);
	return rec;
}


FicheInMem* ObjectDiskPosInfoCollection::PrepareNewObject(DataAddr4D ou, sLONG len, sLONG type, sLONG param1, sLONG param2, sLONG param3, sLONG param4)
{
	VError err = VE_OK;
	FicheInMem* rec = PrepareNewObject(ou, len, type, param1, param2, param3);

	if (rec != nil && GetLastError() == VE_OK)
	{
		VValueSingle* cv = rec->GetNthField(Param3_FieldID, err);
		if (err == VE_OK && cv != nil)
			cv->FromLong(param4);
	}

	SetLastError(err);
	return rec;
}


void ObjectDiskPosInfoCollection::TakeBits(DataAddr4D ou, sLONG len)
{
	sLONG segnum = ou & (kMaxSegData-1);
	ou = ou & (DataAddr4D)(-kMaxSegData);
	if (segnum >= (sLONG)fSegsBitInfo.size())
		fSegsBitInfo.resize(segnum+1);
	ObjectDiskBitInfo* odbi = &fSegsBitInfo[segnum];
	for (DataAddr4D i = ou >> kratio, end = (ou+(DataAddr4D)(len+kMaxSegData-1)) >> kratio; i < end; i++)
	{
		if (odbi->IsOn(i))
			odbi->SetOverLap();
		else
			odbi->Set(i);
	}
}


void ObjectDiskPosInfoCollection::MarkAddr_SegHeader(DataAddr4D ou, sLONG len, sLONG segnum)
{
	if (fWithBitsOnly)
		TakeBits(ou, len);
	else
	{
		VError err = VE_OK;
		FicheInMem* rec = PrepareNewObject(ou, len, DataObject_DataSegHeader, segnum);
		if (rec != nil)
		{
			SetLastError(fAddrs->GetDF()->SaveRecord(rec, fContext));
			rec->Release();
		}
	}
}


void ObjectDiskPosInfoCollection::MarkAddr_PrimBittab(DataAddr4D ou, sLONG len, sLONG segnum)
{
	if (fWithBitsOnly)
		TakeBits(ou, len);
	else
	{
		VError err = VE_OK;
		FicheInMem* rec = PrepareNewObject(ou, len, DataObject_BittabPrim, segnum);
		if (rec != nil)
		{
			SetLastError(fAddrs->GetDF()->SaveRecord(rec, fContext));
			rec->Release();
		}
	}
}


void ObjectDiskPosInfoCollection::MarkAddr_SecondaryBittab(DataAddr4D ou, sLONG len, sLONG segnum, sLONG SecBitNum)
{
	if (fWithBitsOnly)
		TakeBits(ou, len);
	else
	{
		VError err = VE_OK;
		FicheInMem* rec = PrepareNewObject(ou, len, DataObject_BittabSec, segnum, SecBitNum);
		if (rec != nil)
		{
			SetLastError(fAddrs->GetDF()->SaveRecord(rec, fContext));
			rec->Release();
		}
	}
}


void ObjectDiskPosInfoCollection::MarkAddr_Bittab(DataAddr4D ou, sLONG len, sLONG segnum, sLONG SecBitNum, sLONG BittabNum)
{
	if (fWithBitsOnly)
		TakeBits(ou, len);
	else
	{
		VError err = VE_OK;
		FicheInMem* rec = PrepareNewObject(ou, len, DataObject_Bittab, segnum, SecBitNum, BittabNum);
		if (rec != nil)
		{
			SetLastError(fAddrs->GetDF()->SaveRecord(rec, fContext));
			rec->Release();
		}
	}
}


void ObjectDiskPosInfoCollection::MarkAddr_SeqNum(DataAddr4D ou, sLONG len, sLONG seqnum_num)
{
	if (fWithBitsOnly)
		TakeBits(ou, len);
	else
	{
		VError err = VE_OK;
		FicheInMem* rec = PrepareNewObject(ou, len, DataObject_SeqNum, seqnum_num);
		if (rec != nil)
		{
			SetLastError(fAddrs->GetDF()->SaveRecord(rec, fContext));
			rec->Release();
		}
	}
}


void ObjectDiskPosInfoCollection::MarkAddr_DataTable(DataAddr4D ou, sLONG len, sLONG DataTable_num)
{
	if (fWithBitsOnly)
		TakeBits(ou, len);
	else
	{
		VError err = VE_OK;
		FicheInMem* rec = PrepareNewObject(ou, len, DataObject_DataTable, DataTable_num);
		if (rec != nil)
		{
			SetLastError(fAddrs->GetDF()->SaveRecord(rec, fContext));
			rec->Release();
		}
	}
}


void ObjectDiskPosInfoCollection::MarkAddr_StructElemDef(DataAddr4D ou, sLONG len, sLONG numobj, TypObjContainer xtypeobj)
{
	if (fWithBitsOnly)
		TakeBits(ou, len);
	else
	{
		sLONG typ;

		switch(xtypeobj)
		{
			case obj_IndexInStructDef:
			case obj_IndexDef:
				typ = DataObject_IndexDef ;
				break;
			case obj_TableDef:
				typ = DataObject_TableDef ;
				break;
			case obj_RelationDef:
				typ = DataObject_RelationDef ;
				break;
			default:
				xbox_assert(false); // en aurais je oublie un ?
				break;
		}

		VError err = VE_OK;
		FicheInMem* rec = PrepareNewObject(ou, len, typ, numobj);
		if (rec != nil)
		{
			SetLastError(fAddrs->GetDF()->SaveRecord(rec, fContext));
			rec->Release();
		}
	}
}


void ObjectDiskPosInfoCollection::MarkAddr_Blob(DataAddr4D ou, sLONG len, sLONG numtable, sLONG numblob)
{
	if (fWithBitsOnly)
		TakeBits(ou, len);
	else
	{
		VError err = VE_OK;
		FicheInMem* rec = PrepareNewObject(ou, len, DataObject_Blob, numblob);
		if (rec != nil)
		{
			if (rec != nil && GetLastError() == VE_OK)
			{
				VValueSingle* cv = rec->GetNthField(Numtable_FieldID, err);
				if (err == VE_OK && cv != nil)
					cv->FromLong(numtable);
			}
			SetLastError(fAddrs->GetDF()->SaveRecord(rec, fContext));
			rec->Release();
		}
	}
}


void ObjectDiskPosInfoCollection::MarkAddr_Record(DataAddr4D ou, sLONG len, sLONG numtable, sLONG numrec)
{
	if (fWithBitsOnly)
		TakeBits(ou, len);
	else
	{
		VError err = VE_OK;
		FicheInMem* rec = PrepareNewObject(ou, len, DataObject_Record, numrec);
		if (rec != nil && GetLastError() == VE_OK)
		{
			VValueSingle* cv = rec->GetNthField(Numtable_FieldID, err);
			if (err == VE_OK && cv != nil)
				cv->FromLong(numtable);
			SetLastError(fAddrs->GetDF()->SaveRecord(rec, fContext));
			rec->Release();
		}
	}
}


void ObjectDiskPosInfoCollection::MarkAddr_Index(DataAddr4D ou, sLONG len, sLONG numindex, sLONG numpage)
{
	if (fWithBitsOnly)
		TakeBits(ou, len);
	else
	{
		VError err = VE_OK;
		FicheInMem* rec = PrepareNewObject(ou, len, DataObject_BtreePage, numpage);
		if (rec != nil && GetLastError() == VE_OK)
		{
			VValueSingle* cv = rec->GetNthField(Numindex_FieldID, err);
			if (err == VE_OK && cv != nil)
				cv->FromLong(numindex);
			SetLastError(fAddrs->GetDF()->SaveRecord(rec, fContext));
			rec->Release();
		}
	}
}


void ObjectDiskPosInfoCollection::MarkAddr_IndexCluster(DataAddr4D ou, sLONG len, sLONG numindex, sLONG numcluster)
{
	if (fWithBitsOnly)
		TakeBits(ou, len);
	else
	{
		VError err = VE_OK;
		FicheInMem* rec = PrepareNewObject(ou, len, DataObject_Cluster, numcluster);
		if (rec != nil && GetLastError() == VE_OK)
		{
			VValueSingle* cv = rec->GetNthField(Numindex_FieldID, err);
			if (err == VE_OK && cv != nil)
				cv->FromLong(numindex);
			SetLastError(fAddrs->GetDF()->SaveRecord(rec, fContext));
			rec->Release();
		}
	}
}


void ObjectDiskPosInfoCollection::MarkAddr_IndexCluster_part(DataAddr4D ou, sLONG len, sLONG numindex, sLONG numcluster, sLONG numpart)
{
	if (fWithBitsOnly)
		TakeBits(ou, len);
	else
	{
		VError err = VE_OK;
		FicheInMem* rec = PrepareNewObject(ou, len, DataObject_Cluster_Part, numcluster, numpart);
		if (rec != nil && GetLastError() == VE_OK)
		{
			VValueSingle* cv = rec->GetNthField(Numindex_FieldID, err);
			if (err == VE_OK && cv != nil)
				cv->FromLong(numindex);
			SetLastError(fAddrs->GetDF()->SaveRecord(rec, fContext));
			rec->Release();
		}
	}
}


void ObjectDiskPosInfoCollection::MarkAddr_TabAddr_Other(DataAddr4D ou, sLONG len, sLONG pos, sLONG posparent)
{
	if (fWithBitsOnly)
		TakeBits(ou, len);
	else
	{
		VError err = VE_OK;
		FicheInMem* rec = PrepareNewObject(ou, len, DataObject_TabAddr, pos, posparent);
		if (rec != nil)
		{
			SetLastError(fAddrs->GetDF()->SaveRecord(rec, fContext));
			rec->Release();
		}
	}

}


void ObjectDiskPosInfoCollection::MarkAddr_TabAddr_Index(DataAddr4D ou, sLONG len, sLONG numindex, sLONG pos, sLONG posparent)
{
	if (fWithBitsOnly)
		TakeBits(ou, len);
	else
	{
		VError err = VE_OK;
		FicheInMem* rec = PrepareNewObject(ou, len, DataObject_TabAddr_Index, pos, posparent);
		if (rec != nil && GetLastError() == VE_OK)
		{
			VValueSingle* cv = rec->GetNthField(Numindex_FieldID, err);
			if (err == VE_OK && cv != nil)
				cv->FromLong(numindex);
			SetLastError(fAddrs->GetDF()->SaveRecord(rec, fContext));
			rec->Release();
		}
	}
}


void ObjectDiskPosInfoCollection::MarkAddr_TabAddr_IndexCluster(DataAddr4D ou, sLONG len, sLONG numindex, sLONG pos, sLONG posparent)
{
	if (fWithBitsOnly)
		TakeBits(ou, len);
	else
	{
		VError err = VE_OK;
		FicheInMem* rec = PrepareNewObject(ou, len, DataObject_TabAddr_IndexCluster, pos, posparent);
		if (rec != nil && GetLastError() == VE_OK)
		{
			VValueSingle* cv = rec->GetNthField(Numindex_FieldID, err);
			if (err == VE_OK && cv != nil)
				cv->FromLong(numindex);
			SetLastError(fAddrs->GetDF()->SaveRecord(rec, fContext));
			rec->Release();
		}
	}
}


void ObjectDiskPosInfoCollection::MarkAddr_TabAddr_Record(DataAddr4D ou, sLONG len, sLONG numtable, sLONG pos, sLONG posparent)
{
	if (fWithBitsOnly)
		TakeBits(ou, len);
	else
	{
		VError err = VE_OK;
		FicheInMem* rec = PrepareNewObject(ou, len, DataObject_TabAddr_Record, pos, posparent);
		if (rec != nil && GetLastError() == VE_OK)
		{
			VValueSingle* cv = rec->GetNthField(Numtable_FieldID, err);
			if (err == VE_OK && cv != nil)
				cv->FromLong(numtable);
			SetLastError(fAddrs->GetDF()->SaveRecord(rec, fContext));
			rec->Release();
		}
	}
}

void ObjectDiskPosInfoCollection::MarkAddr_TabAddr_Blob(DataAddr4D ou, sLONG len, sLONG numtable, sLONG pos, sLONG posparent)
{
	if (fWithBitsOnly)
		TakeBits(ou, len);
	else
	{
		VError err = VE_OK;
		FicheInMem* rec = PrepareNewObject(ou, len, DataObject_TabAddr_Blob, pos, posparent);
		if (rec != nil && GetLastError() == VE_OK)
		{
			VValueSingle* cv = rec->GetNthField(Numtable_FieldID, err);
			if (err == VE_OK && cv != nil)
				cv->FromLong(numtable);
			SetLastError(fAddrs->GetDF()->SaveRecord(rec, fContext));
			rec->Release();
		}
	}
}


void ObjectDiskPosInfoCollection::MarkAddr_ExtraProp(DataAddr4D ou, sLONG len)
{
	if (fWithBitsOnly)
		TakeBits(ou, len);
	else
	{
		VError err = VE_OK;
		FicheInMem* rec = PrepareNewObject(ou, len, DataObject_DataTable);
		if (rec != nil)
		{
			SetLastError(fAddrs->GetDF()->SaveRecord(rec, fContext));
			rec->Release();
		}
	}
}


VError ObjectDiskPosInfoCollection::CheckOverlaps(ToolLog* log, VProgressIndicator* progress)
{
	VError errexec = VE_OK;
	if (fWithBitsOnly)
	{
		for (ObjectDiskBitInfoCollection::iterator cur = fSegsBitInfo.begin(), end = fSegsBitInfo.end(); cur != end; cur++)
		{
			VError err = cur->GetLastError();
			if (err != VE_OK)
			{
				errexec = err;
			}
		}
		if (errexec == VE_OK)
		{
			VString progressMess;
			if (log != nil)
			{
				log->GetVerifyOrCompactString(30, progressMess);
				log->OpenProgressSession(progressMess, 65);
			}
			sLONG bidon = 0;

			for (ObjectDiskBitInfoCollection::iterator cur = fSegsBitInfo.begin(), end = fSegsBitInfo.end(); cur != end; cur++, bidon++)
			{
				if (log != nil)
					log->Progress(bidon);
				if (cur->OverlapDetected())
				{
					GlobalOverlapProblem pb(TOP_ObjAddrOverlapping, (cur - fSegsBitInfo.begin())+1);
					errexec = log->Add(pb);
				}
			}
			if (log != nil)
				log->CloseProgressSession();
		}
		else
			errexec = ThrowBaseError(errexec, noaction);
	}
	else
	{
		if (fAddrs != nil)
		{
			DataTable* df = fAddrs->GetDF();
			Selection* sel = df->AllRecords(fContext, errexec);
			if (sel != nil)
			{
				SortTab howtosort(fDatas);
				howtosort.AddTriLineField(fAddrs->GetNum(), SegNum_FieldID, true);
				howtosort.AddTriLineField(fAddrs->GetNum(), Offset_FieldID, true);
				Selection* sel2 = sel->SortSel(errexec, &howtosort, fContext, progress, false);
				sel->Release();
				Boolean deja = false, dejareport = false;
				sWORD curseg;
				DataAddr4D curaddr;
				sLONG curlen;
				sLONG curobjtype;
				FicheOnDisk* prevrec = nil;

				if (sel2 != nil)
				{
					SelectionIterator itersel(sel2);
					sLONG recnum = itersel.FirstRecord();
					while (recnum != -1 && errexec == VE_OK)
					{
						FicheOnDisk* ficD = df->LoadNotFullRecord(recnum, errexec, DB4D_Do_Not_Lock, fContext);
						if (ficD != nil)
						{
							sWORD numseg = *((sWORD*)ficD->GetDataPtr(SegNum_FieldID));
							DataAddr4D addr = *((DataAddr4D*)ficD->GetDataPtr(Offset_FieldID));
							sLONG len = *((sLONG*)ficD->GetDataPtr(Len_FieldID));
							sLONG objtype = *((sLONG*)ficD->GetDataPtr(Type_FieldID));

							if (deja)
							{
								if (numseg == curseg)
								{
									if (curaddr+curlen > addr)
									{
										if (!dejareport)
										{
											ObjectAddrProblem pb2(TOP_ObjAddrOverlapping, prevrec);
											errexec = log->Add(pb2);
										}

										ObjectAddrProblem pb(TOP_ObjAddrOverlapping, ficD);
										errexec = log->Add(pb);
										dejareport = true;
									}
									else
										dejareport = false;
								}
								else
									dejareport = false;
							}
							else
								deja = true;

							curseg = numseg;
							curaddr = addr;
							curlen = len;
							curobjtype = objtype;

							if (prevrec != nil)
								prevrec->Release();
								//prevrec->FreeAfterUse();
							prevrec = ficD;
						}
						recnum = itersel.NextRecord();
					}
					sel2->Release();
				}

				if (prevrec != nil)
					prevrec->Release();
					//prevrec->FreeAfterUse();
			}
		}
	}

	return errexec;
}







				// ---------------------------------------------------------------------- //



ToolLog::ToolLog(IDB4D_DataToolsIntf* inDataToolLog)
{
	VDBMgr* manager = VDBMgr::GetManager();
	fLocal = manager->GetDefaultLocalization();
	fLastDisplayTime = 0;
	fDataTools = inDataToolLog;
	fCurrentSession = 0;
	fCurrentBase = nil;
	fToBeCompacted = nil;
	fSkipOrphanDataTables = false;
	fAtLeastOneProblemSinceReset = false;
	fCurrentDataFileToCompact = nil;
	fIndexToBeCompacted = nil;
	fDiskOffsets = nil;
}


VError ToolLog::InitAddressKeeper(Base4D_NotOpened* source, Boolean KeepTempBase, Boolean withbitsonly)
{
	VError err = VE_OK;
	ReleaseAddressKeeper();
	fDiskOffsets = new ObjectDiskPosInfoCollection(KeepTempBase, withbitsonly);
	err = fDiskOffsets->Init(source);
	if (err != VE_OK)
	{
		delete fDiskOffsets;
		fDiskOffsets = nil;
	}
	return err;
}


VError ToolLog::Add(const ToolObject& TOx)
{
	VError errexec = VE_OK;
	fAtLeastOneProblemSinceReset = true;
	try
	{
		VValueBag* bag = new VValueBag();
		if (bag == nil)
			return memfull;

		VString errortext;
		STRSharpCodes sc(1001,TOx.GetProblem());
		fLocal->LocalizeStringWithSTRSharpCodes(sc, errortext);
		bag->SetString("ErrorText", errortext);
		bag->SetLong("ErrorNumber", TOx.GetProblem());
		bag->SetLong("ProblemType", TOx.GetType());
		bag->SetLong("ErrorLevel", TOx.GetErrorLevel());
		TOx.GetText(*bag, fCurrentBase);

		if (fDataTools != nil)
			errexec = fDataTools->AddProblem(*bag);
		
#if debuglr
		VString s, s2;
		bag->GetString("ProblemTypeText", s);
		s.Format(bag);
		bag->GetString("ErrorText", s2);
		s += L" : ";
		s += s2;
		s += L"\n";
		DebugMsg(s);
#endif
	}
	catch (...)
	{
		return memfull;
	}

	if (TOx.GetProblem() != TOP_Index_flagged_for_rebuilding)
	{
		sLONG xbreak = 1; // put a break here
	}

	return errexec;
}


VError ToolLog::Progress(sLONG8 currentValue)
{
	VTask::Yield();
#if debuglr
	sLONG curt = VSystem::GetCurrentTime();
	if ((abs(curt-fLastDisplayTime)) > 300)
	{
		fLastDisplayTime = curt;
		VString s;
		DebugMsg(L"Progress --> ");

		s.FromLong8(currentValue);
		s += L" out of ";
		DebugMsg(s);

		s.FromLong8(fSessions[fCurrentSession-1]);
		s += L"\n";
		DebugMsg(s);
	}
#endif
	if (fDataTools != nil)
		return fDataTools->Progress(currentValue, fSessions[fCurrentSession-1]);
	return VE_OK;
}


VError ToolLog::OpenProgressSession(const VString& sessionName, sLONG8 maxValue)
{
#if debuglr
	DebugMsg(L"Open Session : ");
	DebugMsg(sessionName);
	DebugMsg(L"\n");
#endif

	fCurrentSession++;
	fSessions.push_back(maxValue);
	if (fDataTools != nil)
		return fDataTools->OpenProgression(sessionName, maxValue);
	return VE_OK;
}


VError ToolLog::CloseProgressSession()
{
	if (fCurrentSession > 0)
	{
		fSessions.pop_back();
		fCurrentSession--;
	}

#if debuglr
	DebugMsg(L"Close Session \n");
#endif
	if (fDataTools != nil)
		return fDataTools->CloseProgression();
	return VE_OK;
}


void FormatStringWithParams(VString& sout, sLONG nbparams, sLONG param1, sLONG param2, sLONG param3, sLONG param4, sLONG param5)
{
	VValueBag bag;
	VString s;

	if (nbparams>=1)
	{
		s.FromLong(param1);
		bag.SetString("p1", s);
	}

	if (nbparams>=2)
	{
		s.FromLong(param2);
		bag.SetString("p2", s);
	}

	if (nbparams>=3)
	{
		s.FromLong(param3);
		bag.SetString("p3", s);
	}

	if (nbparams>=4)
	{
		s.FromLong(param4);
		bag.SetString("p4", s);
	}

	if (nbparams>=5)
	{
		s.FromLong(param5);
		bag.SetString("p5", s);
	}

	sout.Format(&bag);
}


void FormatStringWithParamsStrings(VString& sout, VString* param1, VString* param2, VString* param3, VString* param4, VString* param5)
{
	VValueBag bag;
	VString s;

	if (param1 != nil)
	{
		bag.SetString("p1", *param1);
	}

	if (param2 != nil)
	{
		bag.SetString("p2", *param2);
	}

	if (param3 != nil)
	{
		bag.SetString("p3", *param3);
	}

	if (param4 != nil)
	{
		bag.SetString("p4", *param4);
	}

	if (param5 != nil)
	{
		bag.SetString("p5", *param5);
	}

	sout.Format(&bag);
}


static void GetObjectTypeText(sLONG nummess, VString& outText)
{
	STRSharpCodes sc(1006,nummess);
	VDBMgr* manager = VDBMgr::GetManager();
	VLocalizationManager* local = manager->GetDefaultLocalization();
	local->LocalizeStringWithSTRSharpCodes(sc, outText);
}


static void GetProblemTypeText(sLONG nummess, VString& outText)
{
	STRSharpCodes sc(1002,nummess);
	VDBMgr* manager = VDBMgr::GetManager();
	VLocalizationManager* local = manager->GetDefaultLocalization();
	local->LocalizeStringWithSTRSharpCodes(sc, outText);
}


static void GetProblemType(sLONG nummess, VValueBag& outBag)
{
	VString sprob;
	GetProblemTypeText(nummess, sprob);
	outBag.SetString("ProblemTypeText", sprob);
}

static void AddTableParam(sLONG numtable, VValueBag& outBag, Base4D_NotOpened* bd)
{
	VString s;
	bd->ExistTable(numtable, &s);
	outBag.SetLong("TableNum", numtable);
	outBag.SetString("TableName", s);
}


static void AddDataTableParam(sLONG numDatatable, sLONG numtabledef, VValueBag& outBag, Base4D_NotOpened* bd)
{
	VString s = L"# ", s2;
	s2.FromLong(numDatatable);
	s += s2;
	outBag.SetLong("TableNum", numtabledef);
	outBag.SetString("TableName", s);
}


static void AddFieldParam(sLONG numtable, sLONG numfield, VValueBag& outBag, Base4D_NotOpened* bd)
{
	VString s;
	bd->ExistField(numtable, numfield, &s);
	AddTableParam(numtable, outBag, bd);
	outBag.SetLong("FieldNum", numfield);
	outBag.SetString("FieldName", s);
}


static void AddDataFieldParam(sLONG numDatatable, sLONG numtabledef, sLONG numfield, VValueBag& outBag, Base4D_NotOpened* bd)
{
	VString s = L"{Data Table# "+ToString(numDatatable)+L".Field# "+ToString(numfield)+L"}";
	AddDataTableParam(numDatatable, numtabledef, outBag, bd);
	outBag.SetLong("FieldNum", numfield);
	outBag.SetString("FieldName", s);
}


static void AddIndexParam(sLONG numindex, VValueBag& outBag, Base4D_NotOpened* bd)
{
	VString s;
	bd->GetIndexName(-numindex - 1, s);
	outBag.SetLong("IndexNum", numindex);
	outBag.SetString("IndexName", s);
}



void ToolObject::GetText(VValueBag& outBag, Base4D_NotOpened* bd) const
{
	
}

void DataBaseProblem::GetText(VValueBag& outBag, Base4D_NotOpened* bd) const
{
	GetProblemType(1, outBag);
}


void DataSegProblem::GetText(VValueBag& outBag, Base4D_NotOpened* bd) const
{
	GetProblemType(2, outBag);
	outBag.SetLong("SegNum", GetSegNum());
}


void BittableSecTableProblem::GetText(VValueBag& outBag, Base4D_NotOpened* bd) const
{
	GetProblemType(3, outBag);
	outBag.SetLong("SegNum", GetSegNum());
	outBag.SetLong("BittableTable", GetBittableTableNum());
}


void BittableProblem::GetText(VValueBag& outBag, Base4D_NotOpened* bd) const
{
	GetProblemType(4, outBag);
	outBag.SetLong("SegNum", GetSegNum());
	outBag.SetLong("BittableTable", GetBittableTableNum());
	outBag.SetLong("Bittable", GetBittableNum());
}


void DataTableProblem::GetText(VValueBag& outBag, Base4D_NotOpened* bd) const
{
	GetProblemType(5, outBag);
	if (GetTableDefNum() == 0)
	{
		AddDataTableParam(GetTableNum(), 0, outBag, bd);
	}
	else
	{
		AddTableParam(GetTableDefNum(), outBag, bd);
	}
}


void DataTable_RecTabAddrProblem::GetText(VValueBag& outBag, Base4D_NotOpened* bd) const
{
	GetProblemType(6, outBag);
	if (GetTableDefNum() == 0)
	{
		AddDataTableParam(GetTableNum(), 0, outBag, bd);
	}
	else
	{
		AddTableParam(GetTableDefNum(), outBag, bd);
	}
	outBag.SetLong("RecordAddressTableNum", GetTabAddrNum());
}


void DataTable_BlobTabAddrProblem::GetText(VValueBag& outBag, Base4D_NotOpened* bd) const
{
	GetProblemType(7, outBag);
	if (GetTableDefNum() == 0)
	{
		AddDataTableParam(GetTableNum(), 0, outBag, bd);
	}
	else
	{
		AddTableParam(GetTableDefNum(), outBag, bd);
	}
	outBag.SetLong("BlobAddressTableNum", GetTabAddrNum());
}


void BlobProblem::GetText(VValueBag& outBag, Base4D_NotOpened* bd) const
{
	GetProblemType(8, outBag);
	if (GetTableDefNum() == 0)
	{
		AddDataTableParam(GetTableNum(), 0, outBag, bd);
	}
	else
	{
		AddTableParam(GetTableDefNum(), outBag, bd);
	}
	outBag.SetLong("BlobNum", GetBlobNum());
}


void RecordProblem::GetText(VValueBag& outBag, Base4D_NotOpened* bd) const
{
	GetProblemType(9, outBag);
	if (GetTableDefNum() == 0)
	{
		AddDataTableParam(GetTableNum(), 0, outBag, bd);
	}
	else
	{
		AddTableParam(GetTableDefNum(), outBag, bd);
	}
	outBag.SetLong("RecordNum", GetRecordNum());
}


void FieldProblem::GetText(VValueBag& outBag, Base4D_NotOpened* bd) const
{
	GetProblemType(10, outBag);
	if (GetTableDefNum() == 0)
	{
		AddDataFieldParam(GetTableNum(), 0, GetFieldNum(), outBag, bd);
	}
	else
	{
		AddFieldParam(GetTableDefNum(), GetFieldNum(), outBag, bd);
	}
	outBag.SetLong("RecordNum", GetRecordNum());
}


void TableDefHeaderProblem::GetText(VValueBag& outBag, Base4D_NotOpened* bd) const
{
	GetProblemType(11, outBag);
}


void TableDefProblem::GetText(VValueBag& outBag, Base4D_NotOpened* bd) const
{
	GetProblemType(12, outBag);
	AddTableParam(GetTableDefNum(), outBag, bd);
}


void RelationDefHeaderProblem::GetText(VValueBag& outBag, Base4D_NotOpened* bd) const
{
	GetProblemType(13, outBag);
}


void RelationDefProblem::GetText(VValueBag& outBag, Base4D_NotOpened* bd) const
{
	GetProblemType(14, outBag);
	outBag.SetLong("RelationNum", GetRelationDefNum());
}


void SeqNumHeaderProblem::GetText(VValueBag& outBag, Base4D_NotOpened* bd) const
{
	GetProblemType(15, outBag);
}


void SeqNumProblem::GetText(VValueBag& outBag, Base4D_NotOpened* bd) const
{
	GetProblemType(16, outBag);
	outBag.SetLong("SeqNum", GetSeqNumNum());
}


void IndexDefHeaderProblem::GetText(VValueBag& outBag, Base4D_NotOpened* bd) const
{
	GetProblemType(17, outBag);
}


void IndexDefProblem::GetText(VValueBag& outBag, Base4D_NotOpened* bd) const
{
	GetProblemType(18, outBag);
	AddIndexParam(GetIndexDefNum(), outBag, bd);
}


void IndexDefInStructHeaderProblem::GetText(VValueBag& outBag, Base4D_NotOpened* bd) const
{
	GetProblemType(19, outBag);
}


void IndexDefInStructProblem::GetText(VValueBag& outBag, Base4D_NotOpened* bd) const
{
	GetProblemType(20, outBag);
	AddIndexParam(GetIndexDefNum(), outBag, bd);
}


void UUIDConflictProblem::GetText(VValueBag& outBag, Base4D_NotOpened* bd) const
{
	GetProblemType(21, outBag);
	outBag.SetString("ObjName1", fName1);
	outBag.SetString("ObjName2", fName2);
	VString styp;
	Get_TypeOfUUID_Name(fType1, styp);
	outBag.SetString("ObjType1", styp);
	Get_TypeOfUUID_Name(fType2, styp);
	outBag.SetString("ObjType2", styp);
}


void FieldDefProblem::GetText(VValueBag& outBag, Base4D_NotOpened* bd) const
{
	GetProblemType(22, outBag);
	AddFieldParam(GetTableDefNum(), GetFieldNum(), outBag, bd);
}


void Index_PageAddrProblem::GetText(VValueBag& outBag, Base4D_NotOpened* bd) const
{
	GetProblemType(23, outBag);
	AddIndexParam(GetIndexNum(), outBag, bd);
	outBag.SetLong("IndexPageAddressTable", GetTabAddrNum());
}


void IndexPageProblem::GetText(VValueBag& outBag, Base4D_NotOpened* bd) const
{
	GetProblemType(24, outBag);
	AddIndexParam(GetIndexNum(), outBag, bd);
	outBag.SetLong("IndexPageNum", GetPageNum());
}


void IndexProblem::GetText(VValueBag& outBag, Base4D_NotOpened* bd) const
{
	GetProblemType(25, outBag);
	AddIndexParam(GetIndexNum(), outBag, bd);
}


void IndexClusterProblem::GetText(VValueBag& outBag, Base4D_NotOpened* bd) const
{
	if (GetClusterNum() == kSpecialClustNulls)
		GetProblemType(30, outBag);
	else
		GetProblemType(26, outBag);
	AddIndexParam(GetIndexNum(), outBag, bd);
	outBag.SetLong("IndexClusterNum", GetClusterNum());
}


void Index_ClusterAddrProblem::GetText(VValueBag& outBag, Base4D_NotOpened* bd) const
{
	GetProblemType(27, outBag);
	AddIndexParam(GetIndexNum(), outBag, bd);
	outBag.SetLong("IndexClusterAddressTable", GetTabAddrNum());

}


void IndexClusterPageProblem::GetText(VValueBag& outBag, Base4D_NotOpened* bd) const
{
	if (GetClusterNum() == kSpecialClustNulls)
		GetProblemType(31, outBag);
	else
		GetProblemType(28, outBag);
	AddIndexParam(GetIndexNum(), outBag, bd);
	outBag.SetLong("IndexClusterNum", GetClusterNum());
	outBag.SetLong("BitPageNum", GetClusterPage());
}


void DataTableHeaderProblem::GetText(VValueBag& outBag, Base4D_NotOpened* bd) const
{
	GetProblemType(29, outBag);
}


void ObjectAddrProblem::GetText(VValueBag& outBag, Base4D_NotOpened* bd) const
{
	GetProblemType(32, outBag);
	FicheOnDisk* ficD = GetObjectDesc();

	sWORD numseg = *((sWORD*)ficD->GetDataPtr(ObjectDiskPosInfoCollection::SegNum_FieldID));
	DataAddr4D addr = *((DataAddr4D*)ficD->GetDataPtr(ObjectDiskPosInfoCollection::Offset_FieldID));
	sLONG len = *((sLONG*)ficD->GetDataPtr(ObjectDiskPosInfoCollection::Len_FieldID));
	sLONG objtype = *((sLONG*)ficD->GetDataPtr(ObjectDiskPosInfoCollection::Type_FieldID));


	outBag.SetLong("SegNum", numseg);
	outBag.SetLong8("ObjAddr", addr);
	outBag.SetLong("ObjLen", len);
	VString s;
	GetObjectTypeText(objtype, s);
	VValueBag* bag = new VValueBag();
	sLONG* xl = (sLONG*)ficD->GetDataPtr(ObjectDiskPosInfoCollection::Param1_FieldID);
	if (xl != nil)
		bag->SetLong("param1", *xl);
	xl = (sLONG*)ficD->GetDataPtr(ObjectDiskPosInfoCollection::Param2_FieldID);
	if (xl != nil)
		bag->SetLong("param2", *xl);
	xl = (sLONG*)ficD->GetDataPtr(ObjectDiskPosInfoCollection::Param3_FieldID);
	if (xl != nil)
		bag->SetLong("param3", *xl);
	xl = (sLONG*)ficD->GetDataPtr(ObjectDiskPosInfoCollection::Param4_FieldID);
	if (xl != nil)
		bag->SetLong("param4", *xl);

	switch (objtype)
	{
		case DataObject_Blob:
		case DataObject_Record:
		case DataObject_TabAddr_Record:
		case DataObject_TabAddr_Blob:
			AddTableParam(*((sLONG*)ficD->GetDataPtr(ObjectDiskPosInfoCollection::Numtable_FieldID)), *bag, bd);
			break;
		case DataObject_Cluster_Part:
		case DataObject_Cluster:
		case DataObject_BtreePage:
		case DataObject_TabAddr_Index:
		case DataObject_TabAddr_IndexCluster:
			AddIndexParam(*((sLONG*)ficD->GetDataPtr(ObjectDiskPosInfoCollection::Numindex_FieldID)), *bag, bd);
			break;
	}
	s.Format(bag);
	bag->Release();

	outBag.SetString("ObjType", s);
}


void GlobalOverlapProblem::GetText(VValueBag& outBag, Base4D_NotOpened* bd) const
{
	GetProblemType(33, outBag);
	outBag.SetLong("SegNum", GetSegNum());
}


void Get_TypeOfUUID_Name(TypeOfUUID inType, VString& outName)
{
	VDBMgr* manager = VDBMgr::GetManager();
	VLocalizationManager* local = manager->GetDefaultLocalization();
	STRSharpCodes sc(1003,(sLONG)inType);
	local->LocalizeStringWithSTRSharpCodes(sc, outName);
}
