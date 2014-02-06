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


ImpExp::ImpExp(Table* inTable)
{
	target = inTable;
	ColDelimit.AppendChar(9);
	RowDelimit.FromCString("\n");
	curset = VTC_UTF_16;
}


Boolean ImpExp::AddCol(Field* inField)
{
	Boolean ok = false;
	
	if (inField != nil)
	{
		if (inField->GetOwner() == target)
		{
			inField->Retain();
			ok = cols.Add(inField);
		}
		else
			ThrowError(VE_DB4D_WRONGTABLEREF, DBaction_AddingColToImpExp);
	}
	else
		ThrowError(VE_DB4D_WRONGFIELDREF, DBaction_AddingColToImpExp);
	return ok;
}


Field* ImpExp::GetCol(sLONG n) const
{
	if (n>=0 && n<cols.GetCount()) return cols[n];
	else return nil;
	
}


void ImpExp::SetPath(const VFilePath& newpath)
{
	path.FromFilePath(newpath);
}


void ImpExp::SetPath(const VString& newpath)
{
	path.FromFullPath(newpath);
}


void ImpExp::GetPath(VFilePath& curpath) const
{
	curpath.FromFilePath(path);
}


void ImpExp::GetPath(VString& curpath) const
{
	path.GetPath(curpath);
}


void ImpExp::SetColDelimit(const VString& newColDelimit)
{
	ColDelimit.FromString(newColDelimit);
}


void ImpExp::GetColDelimit(VString& curColDelimit) const
{
	curColDelimit.FromString(ColDelimit);
}


void ImpExp::SetRowDelimit(const VString& newRowDelimit)
{
	RowDelimit.FromString(newRowDelimit);
}


void ImpExp::GetRowDelimit(VString& curRowDelimit) const
{
	curRowDelimit.FromString(ColDelimit);
}


void ImpExp::SetCharSet(CharSet newset)
{
// L.E. 31/05/02 il faudra utiliser les VStream
//	if (newset>=CS_AUTO && newset<=VTC_UTF_16)
	{
		curset = newset;
	}
}


VError ImpExp::RunExport(Selection* inSel, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress)
{
	VFile f(path);
	Boolean ok = false;
	VError err = VE_OK;
	
	if (inSel->GetParentFile()->GetTable() == target)
	{
		if (f.Exists())
		{
			err=f.Delete();
		}
		
		if (err == VE_OK)
		{
			VFileDesc *file = NULL;
			err = f.Open(FA_MAX, &file, FO_CreateIfNotFound);
			//if (err == VE_OK) err = f.Open(FA_READ_WRITE);
			if (err == VE_OK)
			{
				sLONG nb = inSel->GetQTfic();
				sLONG i,nb2;
				VString s;

				if (InProgress != nil)
				{
					XBOX::VString session_title;
					gs(1005,5,session_title);	// Exporting a selection of records: %curValue of %maxValue Records
					InProgress->BeginSession(nb,session_title);
				}
				
				nb2 = cols.GetCount();
				
				if (nb2>0)
				{
					VStringConvertBuffer xsrow(RowDelimit, curset, eCRM_NATIVE);
					VStringConvertBuffer xscol(ColDelimit, curset, eCRM_NATIVE);
					
					for (i = 0; (i<nb) && (err == VE_OK); i++)
					{
						if (InProgress != nil)
						{
							if (!InProgress->Progress(i))
								err = ThrowError(VE_DB4D_ACTIONCANCELEDBYUSER, DBaction_ExportingData);
						}
						
						if (err == VE_OK)
						{
							sLONG nfic = inSel->GetFic(i);
							FicheInMem* fic = target->GetDF()->LoadRecord(nfic, err, DB4D_Do_Not_Lock, context);
							if (fic != nil)
							{
								sLONG j;
								for (j=0; (j<nb2) && (err == VE_OK); j++)
								{
									Field* cri = cols[j];
									if (cri != nil)
									{
										ValPtr cv = fic->GetFieldValue(cri, err);
										if (cv != nil)
										{
											cv->GetString(s);
											VStringConvertBuffer xs(s, curset, eCRM_NATIVE);
											err = file->PutDataAtPos(xs.GetCPointer(), xs.GetSize());
										}
									}
									if (err == VE_OK)
									{
										if (j == (nb2-1))
										{
											err = file->PutDataAtPos(xsrow.GetCPointer(), xsrow.GetSize());
										}
										else
										{
											err = file->PutDataAtPos(xscol.GetCPointer(), xscol.GetSize());
										}
									}
								}
							}
						}
					}
				}
				else
					err = ThrowError(VE_DB4D_EXPORT_NOCOL, DBaction_ExportingData);
				
				delete file;
				file = NULL;
				
				if (InProgress != nil)
				{
					InProgress->EndSession();
				}
			
			}
		}
	}
	else
		err = ThrowError(VE_DB4D_EXPORT_WRONGTARGET, DBaction_ExportingData);
	
	return err;
}


UniChar ImpExp::NextChar(VFileDesc* f, Boolean& outEOF, VError& err)
{
	VSize actualread;
	UniChar cuni;
	uBYTE cascii;
	StErrorContextInstaller errs(true);
	
	if (curset == VTC_UTF_16)
	{
		actualread = 2;
		err = f->GetDataAtPos(&cuni,actualread,0,&actualread);
		if (actualread < 2 || err == VE_STREAM_EOF)
			outEOF = true;
	}
	else
	{
		actualread = 1;
		err = f->GetDataAtPos(&cascii,actualread,0,&actualread);
		cuni = cascii; // il faut en fait convertir ici le char en unicode (possibilite de UTF8 : relire un octet a la suite)
		
		if (actualread < 1 || err == VE_STREAM_EOF)
			outEOF = true;
	}
	
	if (err == VE_STREAM_EOF)
	{
		errs.Flush();
		err = VE_OK;
	}
	
	if (err != VE_OK)
		err = ThrowError(err, DBaction_ImportingData);
		
	return cuni;
}


Boolean ImpExp::NextToken(VFileDesc* f, VString& outToken, Boolean& outEOL, Boolean& outEOF, VError& err)
{
	Boolean stop = false;
	sLONG curchar1 = 0, curchar2 = 0;
	outToken.Clear();
	
	outEOL = false;
	err = VE_OK;
	
	while (!stop && err == VE_OK)
	{
		UniChar c;
		
		c = NextChar(f, outEOF, err);
		if (err == VE_OK)
		{
			if (outEOF)
			{
				stop = true;
			}
			else
			{
				outToken.AppendUniChar(c);
				if (c == ColDelimit[curchar1])
				{
					curchar1++;
				}
				
				if (c == RowDelimit[curchar2])
				{
					curchar2++;
				}
			
				if (curchar1>=ColDelimit.GetLength())
				{
					stop = true;
					outToken.Truncate(outToken.GetLength()-ColDelimit.GetLength());
				}
				
				if (curchar2>=RowDelimit.GetLength())
				{
					stop = true;
					outEOL = true;
					outToken.Truncate(outToken.GetLength()-RowDelimit.GetLength());
				}
			}
		}

	}
	
	return (err != VE_OK);
}


VError ImpExp::RunImport(Selection* &outSel, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress)
{
	VError err = VE_OK;
	VFile f(path);
	
	outSel = nil;

	if (f.Exists())
	{
		VFileDesc *file = NULL;
		err = f.Open(FA_READ,&file);
		if (err == VE_OK)
		{
			VStr<512> s;
			Boolean stop = false;
			Boolean RecordStarted = false;
			Boolean endofline = false;
			Boolean eof = false;
			sLONG curcol = 0;
			DataTable* table = target->GetDF();
			sLONG nbcol = cols.GetCount();
			FicheInMem* rec = nil;
			
			while (!stop && err == VE_OK)
			{
				stop = NextToken(file, s, endofline, eof, err);
				if (eof && s.IsEmpty())
					stop = true;
				
				if (!stop)
				{
					if (!RecordStarted)
					{
						RecordStarted = true;
						rec = table->NewRecord(err, context);
						if (rec == nil)
						{
							stop = true;
							RecordStarted = false;
						}
					}

					if (rec != nil)
					{
						if (curcol<nbcol)
						{
							Field* cri = cols[curcol];
							
							ValPtr cv = rec->GetFieldValue(cri, err);
							if (cv != nil)
							{
								cv->FromString(s);
								rec->Touch(cri);
							}
						}
							
						curcol++;
						
						if (endofline && err == VE_OK)
						{
							curcol = 0;
							err = table->SaveRecord(rec, context);
							rec->Release();
							rec= nil;
							if (err != VE_OK)
							{
								stop = true;
							}
							RecordStarted = false;
						}
					}
					
					if (eof)
						stop = true;
				}
			}
			
			if (RecordStarted && err == VE_OK)
			{
				err = table->SaveRecord(rec, context);
				rec->Release();
				rec = nil;
			}
		}
	}
	else
		err = ThrowError(VE_DB4D_IMPORT_DOCDOESNOTEXISTS, DBaction_ImportingData);
	
	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTIMPORTDATA, DBaction_ImportingData);
	
	return err;
}



VError ImpExp::ThrowError( VError inErrCode, ActionDB4D inAction)
{
	VErrorDB4D_OnTable *err;
	
	if (target == nil)
		err = new VErrorDB4D_OnTable(inErrCode, inAction, NULL, -1);
	else
	{
		Base4D* owner = target->GetOwner();
		err = new VErrorDB4D_OnTable(inErrCode, inAction, owner, target->GetNum());
	}
	VTask::GetCurrent()->PushRetainedError( err);
	
	return inErrCode;
}




// -------------------------------------------------------------------------------------------------------



SynchroBaseHelper::SynchroBaseHelper(Base4D* owner)
{
	fBase = nil;
	fOwner = owner;
	fTableCat = nil;
	//fContext = nil;
}


SynchroBaseHelper::~SynchroBaseHelper()
{
	QuickReleaseRefCountable(fTableCat);
	//QuickReleaseRefCountable(fContext);
	if (fBase != nil)
	{
		VDBMgr::GetManager()->UnRegisterBase(fBase);
		fBase->BeginClose(nil);
		xbox_assert(fBase->GetRefCount() == 1);
		fBase->Release();
	}
}


Base4D* SynchroBaseHelper::GetBase(VError& err, bool BuildIfMissing)
{
	err = VE_OK;
	if (fBase == nil && BuildIfMissing)
	{
		VFilePath path;
		VFilePath parent;
		fOwner->GetDataSegPath(1, path);
		path.GetParent(parent);
		VString name;
		VFile datafile(path);
		datafile.GetNameWithoutExtension(name);
		VFolder parentfolder(parent);
		VFile helperStruct(parentfolder, name + kSyncHeaderExt);
		VFile helperData(parentfolder, name + kSyncDataExt);
		fBase = new Base4D(VDBMgr::GetFlushManager(), true /* need flush info */);
		fBase->libere();
		if (helperStruct.Exists())
		{
			fBase->DontUseEntityCatalog();
			err = fBase->OpenStructure(helperStruct, DB4D_Open_WITHOUT_JournalFile | DB4D_Open_Convert_To_Higher_Version, fOwner->IsWriteProtected()? FA_READ : FA_READ_WRITE);
			if (err == VE_OK)
			{
				err = fBase->OpenData(helperData, DB4D_Open_WITHOUT_JournalFile | DB4D_Open_Convert_To_Higher_Version, false, false, fOwner->IsWriteProtected()? FA_READ : FA_READ_WRITE);
				fTableCat = fBase->FindAndRetainTableRef(L"__tables");
				if (fTableCat == nil)
				{
					err = ThrowBaseError(VE_DB4D_SYNCHELPER_MISSING_TABLECAT);
				}
				else
				{
					BaseTaskInfo* xContext = new BaseTaskInfo(fBase, nil, nil,nil);
					Selection* sel = fTableCat->GetDF()->AllRecords(xContext, err);
					if (sel != nil)
					{
						SelectionIterator itersel(sel);
						sLONG i = itersel.FirstRecord();
						while (i != -1)
						{
							FicheInMem* rec = fTableCat->GetDF()->LoadRecord(i, err, DB4D_Do_Not_Lock, xContext);
							if (rec != nil)
							{
								VValueSingle* sourceid = rec->GetNthField(1, err);
								VValueSingle* syncid = rec->GetNthField(2, err);
								if (sourceid->GetValueKind() == VK_UUID && syncid->GetValueKind() == VK_UUID)
								{
									Table* tsource = fOwner->FindAndRetainTableRef(*((VUUID*)sourceid));
									Table* tsync = fBase->FindAndRetainTableRef(*((VUUID*)syncid));
									if (tsource != nil && tsync != nil)
									{
										fTableMap[tsource] = tsync;
									}
									QuickReleaseRefCountable(tsource);
									QuickReleaseRefCountable(tsync);
								}
								QuickReleaseRefCountable(rec);
							}
							i = itersel.NextRecord();
						}
						sel->Release();
					}
					QuickReleaseRefCountable(xContext);
				}
			}
		}
		else
		{
			err = fBase->CreateStructure(helperStruct, DB4D_Create_AllDefaultParamaters, fOwner->GetIntlMgr(), FA_READ_WRITE);
			if (err == VE_OK)
			{
				err = fBase->CreateData(helperData, DB4D_Create_AllDefaultParamaters, fOwner->GetIntlMgr(), nil, false, FA_READ_WRITE);
				if (err == VE_OK)
				{
					VBagLoader loader( true, true);
					VValueBag def;
					def.SetString(L"name", L"__tables");
					err = fBase->CreateTable(def, &loader, &fTableCat, nil, false );
					if (err == VE_OK)
					{
						fTableCat->AddField(L"IDSource", VK_UUID, 0, 0, err, nil, nil);
						fTableCat->AddField(L"IDSync", VK_UUID, 0, 0, err, nil, nil);
						fTableCat->AddField(L"NameSource", VK_STRING, 0, 0, err, nil, nil);
						fTableCat->save();
						//fContext = new BaseTaskInfo(fBase, nil, nil, nil);
					}
				}
			}
		}
		if (err != VE_OK)
		{
			//QuickReleaseRefCountable(fContext);
			//fContext = nil;

			fBase->BeginClose(nil);
			fBase->Release();
			fBase = nil;
		}
	}

	if (err != VE_OK)
	{
		err = ThrowBaseError(VE_DB4D_SYNCHELPER_NOT_VALID);
	}

	return fBase;
}


void SynchroBaseHelper::RemoveTable(Table* tt)
{
	VTaskLock lock(&fMutex);
	Table* result = fTableMap[tt];
	if (result != nil)
		result->Release();
	fTableMap.erase(tt);
}



Table* SynchroBaseHelper::GetTableSync(Table* tt, VError& err, bool BuildIfMissing)
{
	VTaskLock lock(&fMutex);
	err = VE_OK;
	Table* result = nil;
	Base4D* base = GetBase(err, true);
	if (base != nil)
	{
		result = fTableMap[tt];
		if (result == nil && BuildIfMissing)
		{
			VString tablename;
			tt->GetName(tablename);
			VBagLoader loader( true, true);
			VValueBag def;
			def.SetString(L"name", tablename);

			err = fBase->CreateTable(def, &loader, &result, nil, false);
			if (result != nil)
			{
				result->AddField(L"Action", VK_WORD, 0, 0, err, nil, nil);
				result->AddField(L"Stamp", VK_LONG8, 0, 0, err, nil, nil);
				result->AddField(L"TimeStamp", VK_TIME, 0, 0, err, nil, nil);
				FieldArray primkeyfields;
				tt->RetainPrimaryKey(primkeyfields);

				sLONG i = 1;
				FieldNuplet fields(primkeyfields.GetCount(), true);
				for (FieldArray::Iterator cur = primkeyfields.First(), end = primkeyfields.End(); cur != end; cur++, i++)
				{
					Field* sourceField = *cur;
					VString fieldname;
					sourceField->GetName(fieldname);
					sLONG fieldnum = result->AddField(fieldname, sourceField->GetTyp(), 0, 0, err, nil, nil);
					Field* cri = result->RetainField(fieldnum);
					fields.SetNthField(i, cri);
					QuickReleaseRefCountable(cri);
				}
				result->save();
				if (primkeyfields.GetCount() == 1)
				{
					err = fBase->CreIndexOnField(result->GetNum(), fields.GetFieldNum(0), DB4D_Index_Btree, false, nil);
				}
				else
				{
					err = fBase->CreIndexOnFields(&fields, DB4D_Index_Btree, false, nil);
				}

				err = fBase->CreIndexOnField(result->GetNum(), 2, DB4D_Index_Btree, false, nil);
				err = fBase->CreIndexOnField(result->GetNum(), 3, DB4D_Index_Btree, false, nil);

				for (FieldArray::Iterator cur = primkeyfields.First(), end = primkeyfields.End(); cur != end; cur++)
					(*cur)->Release();

				BaseTaskInfo* xContext = new BaseTaskInfo(fBase, nil, nil,nil);
				FicheInMem* rec = fTableCat->GetDF()->NewRecord(err, xContext);
				if (rec != nil)
				{
					VValueSingle* sourceid = rec->GetNthField(1, err);
					VValueSingle* syncid = rec->GetNthField(2, err);
					VValueSingle* sourcename = rec->GetNthField(3, err);
					VUUID sourceID;
					VUUID syncID;
					tt->GetUUID(sourceID);
					result->GetUUID(syncID);
					sourceid->FromVUUID(sourceID);
					syncid->FromVUUID(syncID);
					sourcename->FromString(tablename);

					rec->Touch(1);
					rec->Touch(2);
					rec->Touch(3);
					err = fTableCat->GetDF()->SaveRecord(rec, xContext);
					rec->Release();
				}
				fTableMap[tt] = result;
				QuickReleaseRefCountable(xContext);
			}
		}
	}

	return result;
}



VError SynchroBaseHelper::SetSynchro(Table* tt, VectorOfVValue& primkeyvalues, uLONG8 syncstamp, uLONG8 timestamp, Sync_Action action, BaseTaskInfo* context)
{
	VTaskLock lock(&fMutex);

	VError err = VE_OK;
	Table* tsync = GetTableSync(tt, err);

	if (tsync != nil)
	{
		BaseTaskInfo* syncContext = nil;
		if (context != nil)
			syncContext = context->GetOrBuildSyncHelperContext(fBase);
		bool dejarec = false;
		FicheInMem* recsync = nil;
		SearchTab query(tsync);
		bool first = true;
		sLONG i = 4;
		for (VectorOfVValue::iterator cur = primkeyvalues.begin(), end = primkeyvalues.end(); cur != end; cur++, i++)
		{
			const VValueSingle* val = *cur;
			if (first)
				first = false;
			else
				query.AddSearchLineBoolOper(DB4D_And);

			query.AddSearchLineSimple(tsync->GetNum(), i, DB4D_Equal, val, true);
		}

		OptimizedQuery xquery;
		xquery.AnalyseSearch(&query, syncContext);
		Selection* sel = xquery.Perform((Bittab*)nil, nil, syncContext, err, DB4D_Do_Not_Lock);
		if (sel != nil && sel->GetQTfic() > 0)
		{
			recsync = tsync->GetDF()->LoadRecord(sel->GetFic(0), err, DB4D_Keep_Lock_With_Record, syncContext);
		}
		QuickReleaseRefCountable(sel);

		if (recsync == nil)
		{
			recsync = tsync->GetDF()->NewRecord(err, syncContext);
		}
		else
			dejarec = true;

		if (recsync != nil)
		{
			VTime xtimestamp;
			xtimestamp.FromStamp(timestamp);

			VValueSingle* cv = recsync->GetNthField(1, err);
			if (cv != nil)
				cv->FromWord((sWORD)action);
			recsync->Touch(1);

			cv = recsync->GetNthField(2, err);
			if (cv != nil)
				cv->FromLong8(syncstamp);
			recsync->Touch(2);
	
			cv = recsync->GetNthField(3, err);
			if (cv != nil)
				cv->FromTime(xtimestamp);
			recsync->Touch(3);

			if (!dejarec)
			{
				sLONG i = 4;
				for (VectorOfVValue::iterator cur = primkeyvalues.begin(), end = primkeyvalues.end(); cur != end; cur++, i++)
				{
					const VValueSingle* val = *cur;
					cv = recsync->GetNthField(i, err);
					if (cv != nil)
						cv->FromValue(*val);
					recsync->Touch(i);
				}
			}
			
			err = tsync->GetDF()->SaveRecord(recsync, syncContext);
			recsync->Release();
		}
	}

	return err;
}


VError SynchroBaseHelper::GetSynchroStamp(Table* tt, vector<VValueSingle*>& primkeyvalues, uLONG8& outSyncstamp, VTime& outTimestamp, Sync_Action& outAction, BaseTaskInfo* context)
{
	VTaskLock lock(&fMutex);

	VError err = VE_OK;
	Table* tsync = GetTableSync(tt, err);
	outSyncstamp = 0;
	outAction = Sync_None;
	outTimestamp.FromLong(0);

	if (tsync != nil)
	{
		BaseTaskInfo* syncContext = nil;
		if (context != nil)
			syncContext = context->GetOrBuildSyncHelperContext(fBase);

		FicheInMem* recsync = nil;
		SearchTab query(tsync);
		bool first = true;
		sLONG i = 4;
		for (vector<VValueSingle*>::iterator cur = primkeyvalues.begin(), end = primkeyvalues.end(); cur != end; cur++, i++)
		{
			const VValueSingle* val = *cur;
			if (first)
				first = false;
			else
				query.AddSearchLineBoolOper(DB4D_And);

			query.AddSearchLineSimple(tsync->GetNum(), i, DB4D_Equal, val, true);
		}

		OptimizedQuery xquery;
		xquery.AnalyseSearch(&query, syncContext);
		Selection* sel = xquery.Perform((Bittab*)nil, nil, syncContext, err, DB4D_Do_Not_Lock);
		if (sel != nil && sel->GetQTfic() > 0)
		{
			recsync = tsync->GetDF()->LoadRecord(sel->GetFic(0), err, DB4D_Keep_Lock_With_Record, syncContext);
		}
		QuickReleaseRefCountable(sel);

		if (recsync != nil)
		{
			VValueSingle* cv = recsync->GetNthField(1, err);
			outAction = (Sync_Action)cv->GetByte();

			cv = recsync->GetNthField(2, err);
			outSyncstamp = cv->GetLong8();

			cv = recsync->GetNthField(3, err);
			cv->GetTime(outTimestamp);

			recsync->Release();
		}
	}

	return err;
}



//  ------------------------------------------------ compacting -------------------------------------------------------


VError Base4D::CompactInto(VFile* destData, IDB4D_DataToolsIntf* inDataToolLog, bool KeepRecordNumbers)
{
	ToolLog mylog(inDataToolLog);
	ToolLog* log = &mylog;

	VFile* destStruct = nil;
	Base4D* destBase = nil;
	VError err = VE_OK;

	VDBMgr::GetManager()->FlushCache(true);

	VFolder* parent = destData->RetainParentFolder();
	destStruct = new VFile(*parent, "tempDB.tempDB");
	parent->Release();
	if (destStruct->Exists())
		err = destStruct->Delete();

	if (err == VE_OK)
	{
		if (destData->Exists())
			err = destData->Delete();
	}

	if (err == VE_OK)
	{
		VFile* structFile = nil;
		if (fStructure->fSegments.GetCount() > 0)
			structFile = fStructure->fSegments[0]->GetFile();
		if (structFile == nil)
			err = -1; // a faire
		else
			err = destStruct->CopyFrom(*structFile);
	}

	if (err == VE_OK)
	{
		destBase = new Base4D(VDBMgr::GetManager()->GetFlushManager(), true /* need flush info */);
		destBase->libere();

		err=destBase->OpenStructure( *destStruct, DB4D_Open_WithSeparateIndexSegment, FA_READ_WRITE);
	}

	if (destBase != nil)
	{

		err = destBase->CreateData(*destData, DB4D_Create_WithSeparateIndexSegment, GetIntlMgr());
	
		if (err == VE_OK)
		{
			BaseTaskInfo* contextSource  = new BaseTaskInfo(this, nil, nil, nil);
			BaseTaskInfo* contextDest = new BaseTaskInfo(destBase, nil, nil, nil);

			const VValueBag* extra = RetainExtraProperties(err, contextSource);
			if (extra != nil)
			{
				VValueBag* extracopy = extra->Clone();
				destBase->SetExtraProperties(extracopy, false, contextDest);
				extra->Release();
				extracopy->Release();
			}

			destBase->DelayAllIndexes();
			destBase->StartDataConversion();

			ReadAheadBuffer* buf = NewReadAheadBuffer();

			sLONG nbtable = GetNBTable();
			for (sLONG k = 1; k <= nbtable && err == VE_OK; k++)
			{
				Table* sourceTable = RetainTable(k);
				if (sourceTable != nil)
				{
					Table* destTable = destBase->RetainTable(k);
					if (testAssert(destTable != nil))
					{
						DataTable* sourceDF = sourceTable->GetDF();
						DataTable* destDF = destTable->GetDF();
						if (testAssert(sourceDF != nil && destDF != nil))
						{
							VString s;
							vector<VRefPtr<Field> > fields;
							sLONG nbfieldsInTable = sourceTable->GetNbCrit();
							fields.reserve(nbfieldsInTable);
							for (sLONG j = 1; j <= nbfieldsInTable; j++)
							{
								VRefPtr<Field> ff;
								ff.Adopt(sourceTable->RetainField(j));
								fields.push_back(ff);
							}
							Bittab needSecondPass;
							Bittab forSecondPass;
							sLONG nbrec = sourceDF->GetMaxRecords(contextSource);
							needSecondPass.aggrandit(nbrec);
							forSecondPass.aggrandit(nbrec);

							OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
							if (KeepRecordNumbers)
								destDF->ResizeTableRecs(curstack, nbrec);

							gs(1010, 1, s); // Compacting Records in Table {p1}
							VString tablename;
							sourceTable->GetName(tablename);
							FormatStringWithParamsStrings(s, &tablename);
							err = log->OpenProgressSession(s, nbrec);

							for (sLONG i = 0; i < nbrec && err == VE_OK; i++) // first pass
							{
								Boolean enoughMem = true;
								VError err2 =  log->Progress(i);
								FicheInMem* sourceRec = sourceDF->LoadRecord(i, err, DB4D_Do_Not_Lock, contextSource, false, false, buf, &enoughMem);
								if (sourceRec != nil)
								{
									bool oneMorePass = false;
									FicheInMem* destRec = destDF->NewRecord(err, contextDest);

									if (KeepRecordNumbers)
									{
										destRec->SetRecordNumberPreAllocated(i);
										destRec->SetAddressAllreadyTaken();
									}

									sLONG nbfields = sourceRec->NBCrit();
									if (err == VE_OK)
										err = err2;

									for (sLONG j = 1; j <= nbfields && err == VE_OK; j++)
									{
										Field* cri = fields[j-1];
										if (cri != nil)
										{
											sLONG typ = cri->GetTyp();
											bool okcopy = true;
											bool relative = true;
											VString path;
											if (typ == VK_TEXT || typ == VK_IMAGE || typ == VK_BLOB || typ == VK_BLOB_DB4D)
											{
												okcopy = false;
												
												sLONG* p = (sLONG*)sourceRec->GetAssoc()->GetDataPtr(j);
												if (p != nil && *p != -1)
												{
													sLONG bloblen;
													VValueSingle* cv = sourceRec->GetNthField(j, err);
													if (typ == VK_TEXT)
													{
														VBlobText* blobhandle = (VBlobText*)cv;
														bloblen = blobhandle->GetLength() * 2 + 4;
														if (blobhandle->IsOutsidePath())
														{
															blobhandle->GetTrueOutsidePath(path, &relative);
														}
													}
													else if (typ == VK_IMAGE)
													{
#if VERSIONDEBUG
														VBlob4DWithPtr* blobhandle = dynamic_cast<VBlob4DWithPtr*>(cv->GetDataBlob());
#else
														VBlob4DWithPtr* blobhandle = (VBlob4DWithPtr*)(cv->GetDataBlob());
#endif
														bloblen = (sLONG)blobhandle->GetSize();
														if (blobhandle->IsOutsidePath())
														{
															blobhandle->GetTrueOutsidePath(path, &relative);
														}
													}
													else
													{
														VBlob4DWithPtr* blobhandle = (VBlob4DWithPtr*)cv;
														bloblen = (sLONG)blobhandle->GetSize();
														if (blobhandle->IsOutsidePath())
														{
															blobhandle->GetTrueOutsidePath(path, &relative);
														}
													}
													
													if (cri->GetOutsideData() || !relative)
													{
														okcopy = true;
													}
													else
													{
														if (bloblen <= cri->GetSwitchSize())
														{
															okcopy = true;
														}
														else
														{
															oneMorePass = true;
															VValueSingle* cv2 = destRec->GetNthField(j, err);
															destRec->Touch(j);
															cv2->SetNull(false);
														}
													}
												}
											}
											if (okcopy)
											{
												VValueSingle* cv = sourceRec->GetNthField(j, err);
												if ( cv != nil)
												{
													VValueSingle* cv2 = destRec->GetNthField(j, err);
													if (cv2 != nil)
													{
														if (!relative)
														{
															cv2->SetOutsidePath(path, false);
															cv2->SetForcePathIfEmpty(true);
															//cv2->ReloadFromOutsidePath();
														}
														else
														{
															if (cv2->GetValueKind() == cv->GetValueKind())
																cv2->FromValueSameKind(cv);
															else
																cv2->FromValue(*cv);

														}
														
														destRec->Touch(j);
													}
												}
											}
										}
										
										if (oneMorePass)
											needSecondPass.Set(i);
									}

									if (err == VE_OK)
										err = destDF->SaveRecord(destRec, contextDest);
									if (oneMorePass && err == VE_OK)
									{
										sLONG numdest = destRec->GetNum();
										if (numdest >= 0)
										{
											forSecondPass.Set(numdest);
										}
									}

									QuickReleaseRefCountable(destRec);
									sourceRec->Release();
								}
								else
								{
									/* plus besoin
									if (err == VE_OK && KeepRecordNumbers)
									{
										destDF->SetRecordEntryAsFree(i, contextDest);
									}
									*/
								}
							}

							log->CloseProgressSession();

							///ACI0081570, Apr 16th 2013, O.R. unasking and checking possible error (e.g. from SaveRecord()). Masking error
							///would make compacting report success BUT the compacted data file would miss many records (unprocessed from source data file)
							if(err == VE_OK)
							{
								gs(1010, 2, s); // Compacting Blobs in Table {p1}
								FormatStringWithParamsStrings(s, &tablename);
								err = log->OpenProgressSession(s, nbrec);

								sLONG i = needSecondPass.FindNextBit(0);
								sLONG i2 = forSecondPass.FindNextBit(0);
								while (i != -1 && err == VE_OK)  // second pass to put the blobs that are in the data file
								{
									VError err2 =  log->Progress(i);

									FicheInMem* sourceRec = sourceDF->LoadRecord(i, err, DB4D_Do_Not_Lock, contextSource, false, false, buf);
									if (sourceRec != nil)
									{
										FicheInMem* destRec = destDF->LoadRecord(i2, err, DB4D_Keep_Lock_With_Record, contextDest, false, false, nil);
										if (destRec != nil)
										{
											err = err2;
											sLONG nbfields = sourceRec->NBCrit();
											for (sLONG j = 1; j <= nbfields && err == VE_OK; j++)
											{
												Field* cri = fields[j-1];
												if (cri != nil)
												{
													sLONG typ = cri->GetTyp();
													if (typ == VK_TEXT || typ == VK_IMAGE || typ == VK_BLOB || typ == VK_BLOB_DB4D)
													{
														sLONG* p = (sLONG*)sourceRec->GetAssoc()->GetDataPtr(j);
														if (p != nil && *p != -1)
														{
															sLONG bloblen;
															bool relative = true;
															VValueSingle* cv = sourceRec->GetNthField(j, err);
															if (typ == VK_TEXT)
															{
																VBlobText* blobhandle = (VBlobText*)cv;
																bloblen = blobhandle->GetLength() * 2 + 4;
																if (blobhandle->IsOutsidePath())
																{
																	VString path;
																	blobhandle->GetOutsidePath(path, &relative);
																}
															}
															else if (typ == VK_IMAGE)
															{
	#if VERSIONDEBUG
																VBlob4DWithPtr* blobhandle = dynamic_cast<VBlob4DWithPtr*>(cv->GetDataBlob());
	#else
																VBlob4DWithPtr* blobhandle = (VBlob4DWithPtr*)(cv->GetDataBlob());
	#endif
																bloblen = (sLONG)blobhandle->GetSize();
																if (blobhandle->IsOutsidePath())
																{
																	VString path;
																	blobhandle->GetOutsidePath(path, &relative);
																}
															}
															else
															{
																VBlob4DWithPtr* blobhandle = (VBlob4DWithPtr*)cv;
																bloblen = (sLONG)blobhandle->GetSize();
																if (blobhandle->IsOutsidePath())
																{
																	VString path;
																	blobhandle->GetOutsidePath(path, &relative);
																}
															}

															if (cri->GetOutsideData() || !relative)
															{
																
															}
															else
															{
																if (bloblen <= cri->GetSwitchSize())
																{
																	
																}
																else
																{
																	VValueSingle* cv2 = destRec->GetNthField(j, err);
																	if (cv2 != nil)
																	{
																		cv2->FromValueSameKind(cv);
																		destRec->Touch(j);
																	}
																}
															}
														}
													}
												}

											}

											destDF->SaveRecord(destRec, contextDest);
											QuickReleaseRefCountable(destRec);
										}
										sourceRec->Release();
									}
									i = needSecondPass.FindNextBit(i+1);
									i2 = forSecondPass.FindNextBit(i2+1);
								}

								log->CloseProgressSession();
								if (KeepRecordNumbers)
									destDF->NormalizeTableRecs(curstack);
							
								AutoSeqNumber* seqSource = sourceTable->GetSeqNum(contextSource);
								AutoSeqNumber* seqDest = destTable->GetSeqNum(contextDest);
								if (seqSource != nil && seqDest != nil)
								{
									seqDest->SetStartingValue(seqSource->GetStartingValue());
									seqDest->SetCurrentValue(seqSource->GetCurrentValue());
								}
							}
						}
						destTable->Release();
					}
					sourceTable->Release();
				}
			}

			QuickReleaseRefCountable(buf);

			if (!KeepRecordNumbers)
			{
				/*O.R. ACI 0071637 - Feb 09 2012
				Commenting this resetting of journal file config because
				it will completely disable journaling when the base is later
				reopened in production.
				Instead upper layers will **move** the uncompacted base's journal to a safe location.
				When the compacted base is later reopened in production it will
				prompt for a new journal creation and appropriate backup.
				*/
				//destBase->SetJournalFile(nil);
				
			}

			destBase->GetHbbloc()->fStamp = GetHbbloc()->fStamp;
			destBase->GetHbbloc()->lastaction = GetHbbloc()->lastaction + 2;
			destBase->GetHbbloc()->countlog = GetHbbloc()->countlog;
			destBase->GetHbbloc()->dialect = GetHbbloc()->dialect;
			destBase->GetHbbloc()->collatorOptions = GetHbbloc()->collatorOptions;
			destBase->setmodif(true, this, nil);


			contextSource->Release();
			contextDest->Release();

			destBase->StopDataConversion(nil, nil, false);
			vector<IndexInfo*> indexes;
			destBase->AwakeAllIndexes( nil, true, indexes);
			for (vector<IndexInfo*>::iterator cur = indexes.begin(), end = indexes.end(); cur != end; cur++)
				(*cur)->Release();		
		}

		VDBMgr::GetManager()->FlushCache(true);
		VSyncEvent* waitclose = nil;
		UnRegisterForLang();
		waitclose = new VSyncEvent();
		destBase->BeginClose(waitclose);
		VDBMgr::GetManager()->UnRegisterBase(destBase);

		if (waitclose != nil)
		{
			waitclose->Lock();
			waitclose->Release();
		}

	}

	if (destStruct!= nil && destStruct->Exists())
		destStruct->Delete();

	QuickReleaseRefCountable(destStruct);
	return err;
}





