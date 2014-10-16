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



Blob4D::Blob4D( DataTable *inDataFile)
{
	fDataFile = inDataFile;
	fBlobID = -1;
	fOldBlobID = -1;
	fOKdel = false;
	//fInTrans = false;
	//fIsCached = false;
	fReservedBlobID = -1;
	SetNewInTrans(false);
	SetInTrans(false);
	fInMainMem = false;
	fInOutsidePath = false;
	fOldPathIsValid = false;
	fPathIsRelative = false;
	fOldWasInData = false;
	fHintRecNum = -1;
	fHintFieldNum = -1;
	fHintTableNum = -1;
	fPrimKey = nil;
	fFieldNum = -1;
}


Blob4D::~Blob4D()
{
#if debuglr
	if (modifie())
	{
		xbox_assert(!modifie());
	}
#endif
	DataTable* df = fDataFile;
#if debugblob_strong
	((DataTableRegular*)df)->CheckBlobRef(this);
#endif
	QuickReleaseRefCountable(fPrimKey);
}



VError Blob4D::xThrowError( VError inErrCode, ActionDB4D inAction, ValueKind inBlobType) const
{
	VErrorDB4D_OnBlob *err;
	if (fDataFile == nil)
		err = new VErrorDB4D_OnBlob(inErrCode, inAction, NULL , 0, fBlobID, inBlobType);
	else
	{
		Base4D* owner = fDataFile->GetDB();
		err = new VErrorDB4D_OnBlob(inErrCode, inAction, owner , fDataFile->GetNum(), fBlobID, inBlobType);
	}
	VTask::GetCurrent()->PushRetainedError( err);
	
	return inErrCode;
}


VError Blob4D::ThrowError( VError inErrCode, ActionDB4D inAction) const
{
	return xThrowError(inErrCode, inAction, VK_BLOB);
}


void Blob4D::Kill()
{
	Release();
}


void Blob4D::RestoreFromPop(void* context)
{
	if (fBlobID >= 0 && fDataFile != nil)
	{
		StErrorContextInstaller errs(false);
		CreBlob_Code code;
		if (GetBlobType() == kBlobTypeWithPtr)
			code = &CreBlobWithPtr;
		else
			code = &CreBlobText;

		VError err = VE_OK;
		Blob4D* dejasauve = fDataFile->LoadBlob( fBlobID, code, false, err, (BaseTaskInfo*)context);
		if (dejasauve != nil)
		{
			dejasauve->CopyBaseInto(this);
			dejasauve->Release();
		}
		else
		{
			Detach();
		}

	}
}


void Blob4D::CopyBaseInto(Blob4D* other) const
{
	other->fAnteAddress = fAnteAddress;
	other->fAnteLength = fAnteLength;
	other->fDataFile = fDataFile;
	other->fBlobID = fBlobID;
	other->setaddr(getaddr(), false);
	other->fInOutsidePath = fInOutsidePath;
	other->fOutsidePath = fOutsidePath;
	other->fComputedPath = fComputedPath;
	other->fOutsideSuffixe = fOutsideSuffixe;
	other->fPathIsRelative = fPathIsRelative;
	other->fOldPathIsValid = fOldPathIsValid;
	other->fOldOutsidePath = fOldOutsidePath;
	other->fOldBlobID = fOldBlobID;
	other->fOldWasInData = fOldWasInData;
	CopyRefCountable(&(other->fPrimKey), fPrimKey);
	other->fFieldNum = fFieldNum;

	/*other->SetNbAcces(GetNbAcces());*/
}


void Blob4D::Detach()
{
	fAnteAddress = 0;
	fAnteLength = 0;
	fBlobID = -1;
	ResetAddr();
	if (fPathIsRelative && fInOutsidePath)
	{
		fComputedPath.Clear();
		fOutsidePath.Clear();
	}
	fOldOutsidePath.Clear();
	fOldPathIsValid = false;
	fOldBlobID = -1;
	fOldWasInData = false;
//	setaddr(0, false);
//	SetNbAcces(0);
}


VError Blob4D::WriteSaveToLog(BaseTaskInfo* context, Boolean newone)
{
	VStream* log;
	Base4D* db = fDataFile->GetDB();
	Table* tt = fDataFile->GetTable();
	VError err = VE_OK;
	if (tt != nil)
	{
		if (tt->CanBeLogged())
		{
			if (fPrimKey != nil)
			{
				sLONG lenkey = fPrimKey->GetLength();
				sLONG len = lenkey + 4 /*numfield*/ + 4/*numblob*/  + sizeof(VUUIDBuffer)/*numtable*/;
				sLONG blobid = fBlobID;

				VString pathToSave;
				if (fInOutsidePath)
				{
					if (fPathIsRelative)
						blobid = -2;
					else
						blobid = -3;
					pathToSave = fOutsidePath+fOutsideSuffixe;
					len	= len + 4 + pathToSave.GetLength() * sizeof(UniChar);
				}

				if (blobid != -3)
					len += calclen();

				err = db->StartWriteLog(newone ? DB4D_Log_CreateBlobWithPrimKey : DB4D_Log_ModifyBlobWithPrimKey, len, context, log, true, true);
				if (err == VE_OK)
				{
					if (log != nil)
					{
						err = fPrimKey->PutInto(log);
						if (err == VE_OK)
							err = log->PutLong(fFieldNum);
						if (err == VE_OK)
							err = log->PutLong(blobid);
						if (err == VE_OK)
						{
							if (blobid == -2 || blobid == -3)
							{
								sLONG lenPath = pathToSave.GetLength();
								err = log->PutLong(lenPath);
								err = log->PutWords(pathToSave.GetCPointer(), lenPath);
							}
							if (err == VE_OK)
							{
								VUUID xid;
								tt->GetUUID(xid);
								//err = log->PutLong(fDataFile->GetTrueNum());
								err = xid.WriteToStream(log);
							}
						}
						if (err == VE_OK && blobid != -3) // don't save blob data when we don't own it (absolute path)
							err = PutInto(*log);
					}

					VError err2 = db->EndWriteLog(len, true);
					if (err == VE_OK)
						err = err2;
				}
			}
			else
				err = tt->ThrowError(VE_DB4D_LOG_NEEDS_A_VALID_PRIMARYKEY, noaction);
		}
		db->ReleaseLogMutex();
	}

	return err;
}

VError Blob4D::WriteDeleteToLog(BaseTaskInfo* context)
{
	VStream* log;
	Base4D* db = fDataFile->GetDB();
	Table* tt = fDataFile->GetTable();
	VError err = VE_OK;
	if (tt != nil)
	{
		if (tt->CanBeLogged())
		{
			if (fPrimKey != nil)
			{
				sLONG lenkey = fPrimKey->GetLength();
				sLONG len =  lenkey + 4 /*numfield*/ + 4/*numblob*/  + sizeof(VUUIDBuffer)/*numtable*/;
				sLONG blobid = fBlobID;
				VString pathToSave;
				if (fInOutsidePath && fPathIsRelative)
				{
					blobid = -2;
					pathToSave = fOutsidePath+fOutsideSuffixe;
					len	= len + 4 + pathToSave.GetLength() * sizeof(UniChar);
				}

				err = db->StartWriteLog(DB4D_Log_DeleteBlobWithPrimKey, len, context, log, true, true);
				if (err == VE_OK)
				{
					if (log != nil)
					{
						err = fPrimKey->PutInto(log);
						if (err == VE_OK)
							err = log->PutLong(fFieldNum);
						if (err == VE_OK)
							err = log->PutLong(fBlobID);
						if (err == VE_OK)
						{
							if (blobid == -2)
							{
								sLONG lenPath = pathToSave.GetLength();
								err = log->PutLong(lenPath);
								err = log->PutWords(pathToSave.GetCPointer(), lenPath);
							}
							if (err == VE_OK)
							{
								VUUID xid;
								tt->GetUUID(xid);
								//err = log->PutLong(fDataFile->GetTrueNum());
								err = xid.WriteToStream(log);
							}
						}
					}

					VError err2 = db->EndWriteLog(len, true);
					if (err == VE_OK)
						err = err2;
				}
			}
			else
				err = tt->ThrowError(VE_DB4D_LOG_NEEDS_A_VALID_PRIMARYKEY, noaction);

		}
		db->ReleaseLogMutex();
	}

	return err;
}

void Blob4D::PutHeaderInto(BlobTempHeader& into)
{
	into.fAntelen = fAnteLength;
	into.fDataLen = calclen();
	into.fNewInTrans = IsNewInTrans();
	into.fNumblob = fBlobID;
	into.fAnteAddr = fAnteAddress;
	into.fType = GetBlobType();
	into.fAddr = getaddr();
#if debugOverlaps_strong
	fDataFile->Debug_CheckBlobAddrMatching(into.fAddr, fBlobID);
#endif

}


VError Blob4D::CheckIfMustDeleteOldBlobID(BaseTaskInfo* context, Transaction* trans)
{
	VError err = VE_OK;
	if (trans == nil)
	{
		if (fOldBlobID != -1 && fOldWasInData)
		{
			err = fDataFile->DelBlobForTrans(fOldBlobID, context, true, fPrimKey, fFieldNum);
			fOldBlobID = -1;
			fOldWasInData = false;
		}
	}
	else
	{
		if (fOldBlobID != -1 && fOldWasInData)
		{
			err = trans->DelBlob(fDataFile->GetNum(), fOldBlobID, fPrimKey, fFieldNum);
			fOldBlobID = -1;
			fOldWasInData = false;
		}
	}
	return err;
}


VError Blob4D::CheckIfMustDeleteOldPath(BaseTaskInfo* context, Transaction* trans)
{
	VError err = VE_OK;
	if (trans == nil)
	{
		if (fOldPathIsValid)
		{
			if (IsPathRelative())
			{
				GetDF()->MarkOutsideBlobToDelete(fOldOutsidePath, fOldOutsidePath, nil);
			}
		}
	}
	else
	{
		trans->DelOldBlobPath(this);
	}
	return err;
}



VError Blob4D::LoadPathFrom(void* from)
{
	VError err = VE_OK;
	sLONG* p = (sLONG*)from;
	if (*p == -2 || *p == -3)
	{
		fComputedPath.Clear();
		if (*p == -2)
			fPathIsRelative = true;
		else
			fPathIsRelative = false;
		fInOutsidePath = true;
		p++;
		sLONG len = *p;
		p++;
		fOutsidePath.FromBlock(p, len * sizeof(UniChar), VTC_UTF_16);
		ExtractSuffixe(fOutsidePath, fOutsideSuffixe);
	}
	else
		err = ThrowBaseError(-1);
	return err;
}


VError Blob4D::SavePathTo(void* into)
{
	_CreatePathIfEmpty();
	sLONG* p = (sLONG*)into;
	if (fPathIsRelative)
		*p = -2;
	else
		*p = -3;
	p++;
	VString s = fOutsidePath+fOutsideSuffixe;
	sLONG len = s.GetLength();
	*p = len;
	p++;
	::memcpy( p, s.GetCPointer(), len*sizeof(UniChar));
	return VE_OK;
}


VSize Blob4D::GetPathLength()
{
	_CreatePathIfEmpty();
	VString s = fOutsidePath+fOutsideSuffixe;
	VSize len = 4 + 4 + s.GetLength()*sizeof(UniChar);
	return len;
}


VError Blob4D::SetOutsidePath(const VString& path, bool isRelative)
{
	if (path.IsEmpty())
	{
		fInOutsidePath = false;
	}
	else
	{
		if (!fInOutsidePath && fBlobID != -1)
		{
			fOldWasInData = true;
			fOldBlobID = fBlobID;
		}
		if (fInOutsidePath && !fComputedPath.IsEmpty() && !fOldPathIsValid)
		{
			fOldPathIsValid = true;
			fOldOutsidePath = fComputedPath;
		}
		VString path2 = path;

		if ( (path2.GetLength() == 1 && path2[0] == '*') || (path2.GetLength() == 2 && path2[0] == '/' && path2[1] == '*') )
			path2.Clear();
		if (path2.IsEmpty())
			isRelative = true;
		else
		{
			if (path2.GetLength() > 2)
			{
				VString s, s2;
				path2.GetSubString(1,2, s);
				path2.GetSubString(1,3, s2);
				if (s == L"./")
				{
					path.GetSubString(3, path.GetLength()-2, path2);
					isRelative = true;
				}
				else if (s2 == L"/./")
				{
					path.GetSubString(4, path.GetLength()-3, path2);
					isRelative = true;
				}
				if (!isRelative)
				{
					VFilePath filepath(path2, FPS_POSIX);
					VFolder* datafolder = GetDF()->GetDB()->RetainDataFolder();
					if (datafolder != nil)
					{
						const VFilePath& datapath = datafolder->GetPath();
						VString outPosixRelative;
						if (filepath.GetRelativePosixPath(datapath, outPosixRelative))
							path2 = "../"+outPosixRelative;
						datafolder->Release();
					}
				}
			}
			ExtractSuffixe(path2, fOutsideSuffixe);
		}
		fInOutsidePath = true;
		fOutsidePath = path2;
		fPathIsRelative = isRelative;
		fComputedPath.Clear();
		_ComputePath();
	}
	return VE_OK;
}


void Blob4D::_ComputePath()
{
	if (fComputedPath.IsEmpty() && !fOutsidePath.IsEmpty())
	{
		if (fPathIsRelative)
		{
			VString basePath;
			GetDF()->GetDB()->GetBlobsFolderPath(basePath);
			fComputedPath = basePath;
			fComputedPath += fOutsidePath;
		}
		else
		{
			bool subrel = false;
			sLONG toRemove = 0;
			if (fOutsidePath.GetLength() >= 3 && fOutsidePath[0] == '.' && fOutsidePath[1] == '.' && fOutsidePath[2] == '/')
			{
				subrel = true;
				toRemove = 3;
			}
			if (fOutsidePath.GetLength() >= 4 && fOutsidePath[0] == '/' && fOutsidePath[1] == '.' && fOutsidePath[2] == '.' && fOutsidePath[3] == '/')
			{
				subrel = true;
				toRemove = 4;
			}
			if (subrel)
			{
				VString ss;
				fOutsidePath.GetSubString(toRemove+1, fOutsidePath.GetLength()-toRemove, ss);
				VFolder* datafolder = GetDF()->GetDB()->RetainDataFolder();
				if (datafolder != nil)
				{
					VString datapath;
					datafolder->GetPath(datapath, FPS_POSIX);
					fComputedPath = datapath + ss;
					datafolder->Release();
				}
				else
					fComputedPath = fOutsidePath;
			}
			else
			{
				fComputedPath = fOutsidePath;
			}
		}

		fComputedPath += fOutsideSuffixe;
	}
}


void Blob4D::ExtractSuffixe(VString& ioPath, VString& outSuffixe)
{
	sLONG pos = ioPath.FindUniChar(CHAR_FULL_STOP, ioPath.GetLength(), true);
	sLONG posMin = ioPath.FindUniChar('/', ioPath.GetLength(), true);
	if (pos > 0 && pos > posMin)
	{
		ioPath.GetSubString(pos, (ioPath.GetLength()-pos) + 1, outSuffixe);
		ioPath.Truncate(pos-1);
	}
	else
		outSuffixe.Clear();
}


bool Blob4D::SomethingToDelete()
{
	bool result = false;
	if (GetNum() >=0 )
		result = true;
	else
	{
		if (fInOutsidePath && !fOutsidePath.IsEmpty() && IsPathRelative())
		{
			result = true;
		}
	}
	return result;
}


void Blob4D::MarkDeleteOutsidePath()
{
	if (fInOutsidePath && !fOutsidePath.IsEmpty())
	{
		VString s,path;
		GetOutsideID(s);
		if (IsPathRelative())
			path = fComputedPath;
		GetDF()->MarkOutsideBlobToDelete(s, path, nil );
	}
}


void Blob4D::UnMarkDeleteOutsidePath()
{
	if (fInOutsidePath && !fOutsidePath.IsEmpty())
	{
		if (IsPathRelative())
			GetDF()->UnMarkOutsideBlobToDelete(fComputedPath );
	}
}



VError Blob4D::_CreatePathIfEmpty()
{
	if (fInOutsidePath)
	{
		if (fOutsidePath.IsEmpty())
		{
			VUUID id(true);
			VString name;
			id.GetString(name);
			name = L"Data_"+name;
			if (fDataFile != nil)
			{
				VFolder* parent = fDataFile->GetDB()->RetainBlobsFolder();
				if (parent != nil)
				{
					/*
					if (!parent->Exists())
						parent->Create();
						*/
					VString s;

					s = "Table "+ToString(fHintTableNum);
					fOutsidePath = s+"/";
					/*
					VFolder tFolder(*parent, s);
					if (!tFolder.Exists())
						tFolder.Create();
						*/
					{
						s = "Field "+ToString(fHintFieldNum);
						fOutsidePath += s+"/";
						
						/*
						VFolder fFolder(fFolder, s);
						if (!fFolder.Exists())
							fFolder.Create();
							*/

						sLONG n = fHintRecNum;
						if (n < 0)
						{
							fOutsidePath += "data/";
							//VFolder xFolder(fFolder, "data");
						}
						else
						{
							n = n / 100;
							while (n > 0)
							{
								sLONG n2 = n / 100;
								sLONG n3 = (n % 100) + 1;
								s = ToString(n3);
								fOutsidePath += s+"/";
								n = n2;
							}
						}

						fOutsidePath += name;
					}

					fPathIsRelative = true;
					_ComputePath();
					parent->Release();
				}
			}
		}
	}
	return VE_OK;
}


void Blob4D::AssociateRecord(PrimKey* primkey, sLONG FieldNum)
{
	CopyRefCountable(&fPrimKey, primkey);
	fFieldNum = FieldNum;
}





							/* *********************************************** */
							


sLONG BlobText::calclen(void) const
{
	return 4 + fNbBytes;
}


VError BlobText::ThrowError( VError inErrCode, ActionDB4D inAction) const
{
	return xThrowError(inErrCode, inAction, VK_TEXT);
}



VError BlobText::LoadDataFromPath()
{
	VError err = VE_OK;

	_ComputePath();
	if (fInOutsidePath && !fComputedPath.IsEmpty())
	{
		StErrorContextInstaller errs(false);
		VString ss;
		//VFile outfile(fComputedPath,FPS_POSIX);
		VFile outfile(fDataFile->GetDB()->GetFileSystemNamespace(), fComputedPath);
		if (outfile.Exists())
		{
			sLONG8 len8;
			outfile.GetSize(&len8);
			GetDF()->MeasureLoadedOutsideBlob(len8);
			VFileStream input(&outfile);
			err = input.OpenReading();
			if (err == VE_OK)
				err = input.GuessCharSetFromLeadingBytes(VTC_UTF_8);
			if (err == VE_OK)
				err = input.GetText(ss);
			if (err == VE_OK)
				err = GetFrom(ss.GetCPointer(), ss.GetLength()*sizeof(UniChar));
			input.CloseReading();
		}
	}

	return err;
}



VError BlobText::SetOutsidePath(const VString& path, bool isRelative)
{
	if (path.IsEmpty() && fOutsideSuffixe.IsEmpty())
		fOutsideSuffixe = L".txt";
	return Blob4D::SetOutsidePath(path, isRelative);
}

bool BlobText::SaveObj(VSize& outSizeSaved)
{
#if debuglogwrite
	VString wherefrom(L"BlobText SaveObj");
	VString* whx = &wherefrom;
#else
	VString* whx = nil;
#endif

	if (fInOutsidePath)
	{
		StErrorContextInstaller errs(false);
		if (fOldPathIsValid)
		{
			VFile oldoutfile(fDataFile->GetDB()->GetFileSystemNamespace(), fOldOutsidePath);
			//VFile oldoutfile(fOldOutsidePath,FPS_POSIX);
			oldoutfile.Delete();
			fOldPathIsValid = false;
		}
		const VString& outPath = GetComputedPath();
		//VFile outfile(outPath,FPS_POSIX);
		VFile outfile(fDataFile->GetDB()->GetFileSystemNamespace(), outPath);
		VFolder* parent = outfile.RetainParentFolder();
		if (parent != nil)
		{
			parent->CreateRecursive();
			parent->Release();
		}
		outfile.Delete();
		VError err = outfile.Create();
		if (err == VE_OK)
		{
			if (fNbBytes > 0)
			{
				Utf16toUtf8 buffer(fString, fNbBytes/2);
				VFileStream fout(&outfile);
				err = fout.OpenWriting();
				if (err == VE_OK)
				{
					fout.SetCharSet(VTC_UTF_8);
					err = fout.WriteBOM();
					if (err == VE_OK)
						err = fout.PutData(buffer.GetPtr(), buffer.GetNbBytes());
					GetDF()->MeasureSavedOutsideBlob(buffer.GetNbBytes());
				}
				fout.CloseWriting();
			}
		}
		fOldOutsidePath = outPath;
		fOldPathIsValid = true;
		outSizeSaved = fNbBytes;
	}
	else
	{
		VError err;
		Base4D *db = GetDF()->GetDB();
	#if debug_Addr_Overlap
		db->CheckDBObjRef(getaddr(), fNbBytes+4+kSizeDataBaseObjectHeader, debug_BlobRef(GetDF()->GetNum(), GetNum()));
	#endif
		DataBaseObjectHeader tag(fString, fNbBytes, DBOH_BlobText, GetNum(), GetDF()->GetTrueNum());
		err = tag.WriteInto(db, getaddr(), whx);
		if (err == VE_OK) 
			err = db->writelong(&fNbBytes,4,getaddr(),kSizeDataBaseObjectHeader, whx);
		if (fNbBytes>0 && err == VE_OK)
		{
			err = db->writelong(fString,fNbBytes,getaddr(),kSizeDataBaseObjectHeader+4, whx);
		}
		
		if (err != VE_OK)
			err = db->ThrowError(VE_DB4D_CANNOTSAVEBLOB, DBaction_SavingBlob);
		outSizeSaved = fNbBytes+4;
		GetDF()->MeasureSavedBlob(outSizeSaved+kSizeDataBaseObjectHeader);
	}

	return true;
}


Blob4D* BlobText::loadobj(DataAddr4D xaddr, VError& err)
{
	err=VE_OK;
	Base4D *db=GetDF()->GetDB();
	BlobText* result = nil;
	if (xaddr!=0)
	{
		ReleaseMem((void*)fString);
		fString = nil;
		fNbBytes = 0;

		setaddr(xaddr);
		
		DataBaseObjectHeader tag;
		err = tag.ReadFrom(db, getaddr(), nil);
		if (err == VE_OK)
			err = tag.ValidateTag(DBOH_BlobText, GetNum(), GetDF()->GetTrueNum());
		if (err == VE_OK)
			err=db->readlong(&fNbBytes,4,getaddr(),kSizeDataBaseObjectHeader);
		if (err == VE_OK)
		{
			if (tag.NeedSwap())
				fNbBytes = SwapLong(fNbBytes);
		}
		if ((fNbBytes>0) && (err==VE_OK))
		{
			fString = (UniChar *) AllocateMem( fNbBytes, 'text');
			if (fString != nil) {
				err=db->readlong( fString, fNbBytes, getaddr(), kSizeDataBaseObjectHeader+4);

				if (err == VE_OK)
					err = tag.ValidateCheckSum(fString, fNbBytes);
				if (err == VE_OK)
				{
					if (fCharsCanBeSwapped && tag.NeedSwap())
						ByteSwap(fString, fNbBytes/2);
				}
			}
			else
			{
				err=ThrowError(memfull, DBaction_LoadingBlob);
			}
		}
		GetDF()->MeasureLoadedBlob(fNbBytes + 4 + kSizeDataBaseObjectHeader);
		
		
	}
	SetAntelen( calclen());

	if (err != VE_OK)
	{
		ReleaseMem((void*)fString);
		fString = nil;
		fNbBytes = 0;
		err = ThrowError(VE_DB4D_CANNOTLOADBLOB, DBaction_LoadingBlob);
	}
	else
	{
		sLONG alloc = -2;
		if (fString != nil && !fInMainMem)
			alloc = GetAllocationNumber(fString);
		if (fInMainMem || fString == nil || GetAllocationNumber(this) == alloc)
		{
			result = this;
			result->Retain(); // retenu une fois de plus pour que le LoadBlob puisse le mettre dans le treemem du cache
		}
		else
		{
			void* p = GetFastMem(sizeof(BlobText), true, 'blxb', alloc);
			if (GetAllocationNumber(p) == alloc)
			{
#if 0
				vBlockMove(this, p, sizeof(BlobText)); // la copie byte a byte copie aussi le refcount qui est a 1 sur l'original
				result = (BlobText*)p;
				result->Retain(); // retenu une fois de plus pour que le LoadBlob puisse le mettre dans le treemem du cache
				fString = nil;
				fNbBytes = 0;
				//Release(); 
				// ATTENTION, cette methode n'est appellée que sur des blobtext qui sont alloue dans la memoire du cache
				// on ne peut pas faire un release qui appelerait delete et ferait un delete de tout les objets inclus une deuxieme fois.
				FreeFastMem((void*)this);
#endif
				result = new (p)BlobText(fDataFile);
				result->addrobj = addrobj;
				result->fFlushInfo = fFlushInfo;
				result->fModified = fModified;
				result->fSaving = fSaving;
				result->fChangingAddr = fChangingAddr;
				result->fInProtectedArea = fInProtectedArea;
				result->fOccupyStamp = fOccupyStamp;
				result->fBlobID = fBlobID;
				result->fReservedBlobID = fReservedBlobID;
				result->fAnteAddress = fAnteAddress;
				result->fAnteLength = fAnteLength;
				result->fOKdel = fOKdel;
				result->fNewInTrans = fNewInTrans;
				result->fInTrans = fInTrans;
				result->fInMainMem = fInMainMem;
				result->fInOutsidePath = fInOutsidePath;
				result->fOldPathIsValid = fOldPathIsValid;
				result->fPathIsRelative = fPathIsRelative;
				result->fOutsidePath = fOutsidePath;
				result->fOutsideSuffixe = fOutsideSuffixe;
				result->fOldOutsidePath = fOldOutsidePath;
				result->fComputedPath = fComputedPath;
				result->fString = fString;
				result->fNbBytes = fNbBytes;
				result->fCharsCanBeSwapped = fCharsCanBeSwapped;
				result->fHintRecNum = fHintRecNum;
				result->Retain();
				fString = nil;
				fNbBytes = 0;
				Release(); 
			}
			else
			{
				FreeFastMem(p);
				result = this;
				result->Retain(); // retenu une fois de plus pour que le LoadBlob puisse le mettre dans le treemem du cache
			}
		}
	}
	return(result);
}


VError BlobText::CopyDataFrom(const Blob4D* from)
{
	VError err = VE_OK;
	XBOX_ASSERT_KINDOFNIL( BlobText, from);
	ReleaseMem((void*)fString);
	if ( ((BlobText *)from)->fNbBytes > 0)
	{
		fString = (UniChar *) AllocateMem( ((BlobText *)from)->fNbBytes, 'text');
		if (fString != nil) {
			fNbBytes = ((BlobText *)from)->fNbBytes;
			::CopyBlock( ((BlobText *)from)->fString, fString, fNbBytes);
		} else
		{
			fNbBytes = 0;
			err = ThrowError(memfull, DBaction_CopyingBlob);
		}
	}
	else
	{
		fString = nil;
		fNbBytes = 0;
	}
		
	return err;
}


uBOOL BlobText::IsEmpty(void)
{
	return fNbBytes == 0;
}



Blob4D* CreBlobText(DataTable *df)
{
	return(new BlobText(df));
}


Blob4D* CreBlobTextUTF8(DataTable *df)
{
	return(new BlobTextUTF8(df));
}


VError BlobText::GetFrom(const void* p, sLONG len, sLONG offset)
{
	ReleaseMem((void*)fString);

	if (len >0)
	{
		fString = (UniChar *) AllocateMem( len, 'text');
		if (fString != nil) {
			::CopyBlock( p, fString, len);
			fNbBytes = len;
			return VE_OK;
		} else
		{
			fNbBytes = 0;
			return ThrowError(memfull, DBaction_CopyingBlob);
		}
	}
	else
	{
		fString = nil;
		fNbBytes = 0;
		return VE_OK;
	}

}


VError BlobText::PutInto(void* p, sLONG len, sLONG offset, sLONG* ActualLen) const
{
	sLONG nbbyte = len;
	if (len == -1)
		nbbyte = fNbBytes;
	if ((offset+nbbyte) > fNbBytes)
		nbbyte = fNbBytes - offset;
	if (nbbyte<0)
		nbbyte = 0;

	if (fString != nil)
		::CopyBlock( ((char*)fString + offset), p, nbbyte);
	else
		nbbyte = 0;

	if (ActualLen != nil)
		*ActualLen = nbbyte;

	return VE_OK;
}


VError BlobText::GetFrom(VStream& buf)
{
	VError err;
	
	ReleaseMem((void*)fString);
	fString = nil;

	sLONG len;
	err = buf.GetLong(len);
	if (err == VE_OK) {
		if (len > 0)
		{
			fString = (UniChar *) AllocateMem( len, 'text');
			if (fString != nil) {
				err = buf.GetData( fString, len);
				if (err != VE_OK) {
					ReleaseMem((void*)fString);
					fString = nil;
					fNbBytes = 0;
				}
			} else {
				fNbBytes = len;
				err = ThrowError(memfull, DBaction_CopyingBlob);
			}
		}
		else
		{
			fString = nil;
			fNbBytes = 0;
		}
	}
	
	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTALLOCATEBLOB, DBaction_CopyingBlob);
	
	return(err);
}

VError BlobText::PutInto(VStream& buf) const
{
	VError err = buf.PutLong( fNbBytes);
	if (err == VE_OK && fString != nil)
		err = buf.PutData( fString, fNbBytes);
	
	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTSAVEBLOB, DBaction_CopyingBlob);
	
	return(err);
}

void BlobText::GetCString( UniPtr *outCString, sLONG *outLength)
{
	*outCString = fString;
	*outLength = (fNbBytes / sizeof( UniChar));
}


BlobText::~BlobText()
{
	ReleaseMem((void*)fString);
}

/*
void	BlobText::SetCString( UniPtr inCString, sLONG inLength)
{
	if ( (inCString != fString) && (fString != nil) ) 
	{
		ReleaseMem((void*)fString);
	}
	fString = inCString;
	fNbBytes = inLength* sizeof( UniChar);
	fInMainMem = false;
}
*/
	

Blob4D* BlobText::Clone(Boolean WithData) const
{
	BlobText* result = new BlobText((DataTable*)GetDF());
	if (result != nil)
	{
		CopyBaseInto(result);
		if (WithData)
		{
			VError err = result->CopyDataFrom(this);
			if (err != VE_OK)
			{
				result->Release();
				result = nil;
			}
		}
	}
	return result;
}


bool BlobText::MatchAllocationNumber(sLONG allocationNumber)
{
	if (allocationNumber == -1)
		return true;
	else
	{
		if (GetAllocationNumber(this) == allocationNumber)
			return true;
		if (fInMainMem || (fString != nil && GetAllocationNumber(fString) == allocationNumber))
			return true;
		return false;
	}
}



							// ------------------------------------------------



VError BlobTextUTF8::GetFrom(const void* p, sLONG len, sLONG offset)
{
	ReleaseMem((void*)fString);

	if (len >0)
	{
		Utf16toUtf8 buffer((UniChar*)p, len/2);

		fString = (UniChar *) AllocateMem( buffer.GetNbBytes(), 'text');
		if (fString != nil) {
			fNbBytes = buffer.GetNbBytes();
			::CopyBlock( buffer.GetPtr(), fString, fNbBytes);
			return VE_OK;
		} else
		{
			fNbBytes = 0;
			return ThrowError(memfull, DBaction_CopyingBlob);
		}
	}
	else
	{
		fString = nil;
		fNbBytes = 0;
		return VE_OK;
	}

}


VError BlobTextUTF8::PutInto(void* p, sLONG len, sLONG offset, sLONG* ActualLen) const
{
	Utf8ToUtf16 buffer(fString, fNbBytes);

	sLONG nbbyte = len;
	if (len == -1)
		nbbyte = buffer.GetLength() * 2;
	if ((offset+nbbyte) > fNbBytes)
		nbbyte = fNbBytes - offset;
	if (nbbyte<0)
		nbbyte = 0;

	if (buffer.GetPtr() != nil)
		::CopyBlock( (((char*)buffer.GetPtr()) + offset), p, nbbyte);
	else
		nbbyte = 0;

	if (ActualLen != nil)
		*ActualLen = nbbyte;

	return VE_OK;
}


Blob4D* BlobTextUTF8::Clone(Boolean WithData) const
{
	BlobTextUTF8* result = new BlobTextUTF8((DataTable*)GetDF());
	if (result != nil)
	{
		CopyBaseInto(result);
		if (WithData)
		{
			VError err = result->CopyDataFrom(this);
			if (err != VE_OK)
			{
				result->Release();
				result = nil;
			}
		}
	}
	return result;
}









							/* *********************************************** */
			




BlobWithPtr::~BlobWithPtr()
{
//	ClearFlag(ObjAlmostInCache::isModifFlag);
	if (fData != nil)
	{
		ReleaseMem( fData);
	}
}
							

sLONG BlobWithPtr::calclen(void) const
{
	if (fData==nil) return(0);
	else return(4+fDataLen);
}


VError BlobWithPtr::ThrowError( VError inErrCode, ActionDB4D inAction) const
{
	return xThrowError(inErrCode, inAction, VK_BLOB);
}


VError BlobWithPtr::LoadDataFromPath()
{
	VError err = VE_OK;

	if (fData != nil)
	{
		ReleaseMem( fData);
	}
	fData = nil;
	fDataLen = 0;

	_ComputePath();
	if (fInOutsidePath && !fComputedPath.IsEmpty())
	{
		VFile outfile(fDataFile->GetDB()->GetFileSystemNamespace(), fComputedPath);
		//VFile outfile(fComputedPath,FPS_POSIX);
		if (outfile.Exists())
		{
			VFileDesc *ff;
			sLONG8 len8;
			err = outfile.GetSize(&len8);
			if (len8 < kMAX_sLONG)
			{
				GetDF()->MeasureLoadedOutsideBlob(len8);
				VSize len = (VSize)len8;
				if (len == 0)
					fData = nil;
				else
				{
					fData = (VPtr)AllocateMem(AdjusteSize(len), 'blb4');
					if (fData != nil)
					{
						fDataLen = len;
						err = outfile.Open(FA_READ, &ff);
						if (err == VE_OK)
						{
							err = ff->GetData(fData, len, 0);
						}
						if (ff != nil)
							delete ff;
						if (err != VE_OK)
						{
							ReleaseMem( fData);
							fData = nil;
							fDataLen = 0;
						}
					}
				}
			}
		}
	}

	return err;
}


bool BlobWithPtr::SaveObj(VSize& outSizeSaved)
{
#if debuglogwrite
	VString wherefrom(L"BlobWithPtr SaveObj");
	VString* whx = &wherefrom;
#else
	VString* whx = nil;
#endif

	if (fInOutsidePath)
	{
		StErrorContextInstaller errs(false);
		if (fOldPathIsValid)
		{
			VFile oldoutfile(fDataFile->GetDB()->GetFileSystemNamespace(), fOldOutsidePath);
			//VFile oldoutfile(fOldOutsidePath,FPS_POSIX);
			oldoutfile.Delete();
			fOldPathIsValid = false;
		}
		const VString& outPath = GetComputedPath();
		VFile outfile(fDataFile->GetDB()->GetFileSystemNamespace(), outPath);
		//VFile outfile(outPath,FPS_POSIX);
		VFolder* parent = outfile.RetainParentFolder();
		if (parent != nil)
		{
			parent->CreateRecursive();
			parent->Release();
		}
		outfile.Delete();
		VError err = outfile.Create();
		if (err == VE_OK)
		{
			VFileDesc *ff;
			err = outfile.Open(FA_READ_WRITE, &ff);
			if (err == VE_OK && fData != nil)
			{
				err = ff->PutData(fData, fDataLen, 0);
				GetDF()->MeasureSavedOutsideBlob(fDataLen);
			}
			if (ff != nil)
				delete ff;

		}
		outSizeSaved = fDataLen;
	}
	else
	{
		Base4D *db;
		sLONG lll;
		VError err;
		VPtr pp = fData;

		lll = fDataLen;

		db=GetDF()->GetDB();
	#if debug_Addr_Overlap
		db->CheckDBObjRef(getaddr(), lll+4+kSizeDataBaseObjectHeader, debug_BlobRef(GetDF()->GetNum(), GetNum()));
	#endif
		DataBaseObjectHeader tag(pp, lll, DBOH_Blob, GetNum(), GetDF()->GetTrueNum());
		err = tag.WriteInto(db, getaddr(), whx);
		if (err == VE_OK)
			err = db->writelong(&lll,4,getaddr(),kSizeDataBaseObjectHeader, whx);
		if (err == VE_OK)
		{
			err = db->writelong( pp, lll,getaddr(),kSizeDataBaseObjectHeader+4, whx);
		}

		if (err != VE_OK)
			err = db->ThrowError(VE_DB4D_CANNOTSAVEBLOB, DBaction_SavingBlob);
			
		outSizeSaved = lll+4;
		GetDF()->MeasureSavedBlob(outSizeSaved + kSizeDataBaseObjectHeader);
	}
	return true;
}

Blob4D* BlobWithPtr::loadobj(DataAddr4D xaddr, VError& err)
{
	Base4D *db;
	sLONG lll = 0;
	BlobWithPtr* result = nil;
	
	err=VE_OK;
	db=GetDF()->GetDB();
	
	if (xaddr!=0)
	{
		setaddr(xaddr);
		DataBaseObjectHeader tag;
		err = tag.ReadFrom(db, getaddr(), nil);
		if (err == VE_OK)
			err = tag.ValidateTag(DBOH_Blob, GetNum(), GetDF()->GetTrueNum());
		if (err == VE_OK)
			err=db->readlong(&lll,4,getaddr(),kSizeDataBaseObjectHeader);
		if (err == VE_OK)
		{
			if (tag.NeedSwap())
				lll = SwapLong(lll);
		}
		if ((lll>0) && (err==VE_OK))
		{
			fData = (VPtr)AllocateMem(AdjusteSize(lll), 'blb4');
			if (fData!=nil)
			{
				fDataLen = lll;
				err=db->readlong( fData,lll,getaddr(),kSizeDataBaseObjectHeader+4);
				if (err == VE_OK)
					err = tag.ValidateCheckSum(fData, lll);
				// le swap du contenu du blob est fait a un plus haut niveau car dependant de son type
			}
			else
			{
				err = ThrowError(memfull, DBaction_LoadingBlob);
			}
		}
		GetDF()->MeasureLoadedBlob(lll + 4 + kSizeDataBaseObjectHeader);
	}
	SetAntelen( calclen());

	
	if (err != VE_OK)
	{
		if (fData != nil)
		{
			ReleaseMem(fData);
			fData = nil;
			fDataLen = 0;
		}

		err = ThrowError(VE_DB4D_CANNOTLOADBLOB, DBaction_LoadingBlob);
		Release();
	}
	else
	{
		sLONG alloc = -2;
		if (fData != nil && !fInMainMem)
			alloc = GetAllocationNumber(fData);
		if (fInMainMem || fData == nil || GetAllocationNumber(this) == alloc)
		{
			result = this;
			result->Retain(); // retenu une fois de plus pour que le LoadBlob puisse le mettre dans le treemem du cache
		}
		else
		{
			void* p = GetFastMem(sizeof(BlobWithPtr), true, 'blxb', alloc);
			if (GetAllocationNumber(p) == alloc)
			{
#if 0
				vBlockMove(this, p, sizeof(BlobWithPtr)); // la copie byte a byte copie aussi le refcount qui est a 1 sur l'original
				result = (BlobWithPtr*)p;
				result->Retain(); // retenu une fois de plus pour que le LoadBlob puisse le mettre dans le treemem du cache
				fData = nil;
				fDataLen = 0;
				//Release();
				FreeFastMem((void*)this);
#endif
				result = new (p)BlobWithPtr(fDataFile);
				result->addrobj = addrobj;
				result->fFlushInfo = fFlushInfo;
				result->fModified = fModified;
				result->fSaving = fSaving;
				result->fChangingAddr = fChangingAddr;
				result->fInProtectedArea = fInProtectedArea;
				result->fOccupyStamp = fOccupyStamp;
				result->fBlobID = fBlobID;
				result->fReservedBlobID = fReservedBlobID;
				result->fAnteAddress = fAnteAddress;
				result->fAnteLength = fAnteLength;
				result->fOKdel = fOKdel;
				result->fNewInTrans = fNewInTrans;
				result->fInTrans = fInTrans;
				result->fInMainMem = fInMainMem;
				result->fInOutsidePath = fInOutsidePath;
				result->fOldPathIsValid = fOldPathIsValid;
				result->fPathIsRelative = fPathIsRelative;
				result->fOutsidePath = fOutsidePath;
				result->fOutsideSuffixe = fOutsideSuffixe;
				result->fOldOutsidePath = fOldOutsidePath;
				result->fComputedPath = fComputedPath;
				result->fData = fData;
				result->fDataLen = fDataLen;
				result->fHintRecNum = fHintRecNum;
				result->Retain();
				fData = nil;
				fDataLen = 0;
				Release(); 

			}
			else
			{
				FreeFastMem(p);
				result = this;
				result->Retain(); // retenu une fois de plus pour que le LoadBlob puisse le mettre dans le treemem du cache
			}
		}
	}
	return(result);
}


VError BlobWithPtr::CopyDataFrom(const Blob4D* from)
{
	VError err = VE_OK;
	BlobWithPtr* other = (BlobWithPtr*)from;
	if (other != nil)
	{
		if (fData != nil)
		{
			ReleaseMem( fData);
			fData = nil;
			fDataLen = 0;
		}

		if (other->fDataLen > 0)
		{
			fData = (VPtr)AllocateMem(AdjusteSize(other->fDataLen), 'blb4');
			if (fData == nil)
			{
				err = ThrowError(memfull, DBaction_CopyingBlob);
			}
			else
			{
				fDataLen = other->fDataLen;
				vBlockMove(other->fData, fData, fDataLen);
			}
		}
	}

	return err;
}


uBOOL BlobWithPtr::IsEmpty(void)
{
	return((fData==nil) || (fDataLen==0));
}


VError BlobWithPtr::SetSize(sLONG inNewSize)
{
	VError err = VE_OK;
	if (fData == NULL)
	{
		if (inNewSize > 0)
		{
			fData = (VPtr)AllocateMem(AdjusteSize(inNewSize), 'blb4');
			if (fData == NULL)
				err = VE_MEMORY_FULL;
			else
				fDataLen = inNewSize;
		}
	}
	else if (inNewSize == 0)
	{
		ReleaseMem( fData);
		fData = NULL;
		fDataLen = 0;
	}
	else if (AdjusteSize(inNewSize) != AdjusteSize(fDataLen))
	{
		VPtr newdata = (VPtr)SetPtrSize((void*)fData, AdjusteSize(inNewSize), fDataLen);
		if (newdata == NULL)
			err = VE_MEMORY_FULL;
		else
		{
			fData = newdata;
			fDataLen = inNewSize;
		}
	}
	else
	{
		fDataLen = inNewSize;
	}
	return err;
}


VError BlobWithPtr::GetFrom(const void* p, sLONG len, sLONG offset)
{
	VError err = VE_OK;
	if (fData == NULL)
	{
		if (offset + len > 0)
		{
			fData = (VPtr) AllocateMem(AdjusteSize(offset + len), 'blb4');
			if (fData != NULL) 
			{
				::CopyBlock(p, fData + offset, len);
				fDataLen = offset + len;
			}
			else
				err = VE_MEMORY_FULL;
		}
	} 
	else
	{
		if (AdjusteSize(fDataLen) < AdjusteSize(offset + len)) 
		{
			VPtr newdata = (VPtr) SetPtrSize(fData, AdjusteSize(offset + len), fDataLen);
			if (newdata == NULL)
				err = VE_MEMORY_FULL;
			else
			{
				fData = newdata;
				fDataLen = offset + len;
			}
		}
		else
		{
			if (fDataLen < offset + len)
				fDataLen = offset + len;
		}
		if (err == VE_OK) {
			::CopyBlock(p, fData + offset, len);
		}
	}
	return err;
}


VError BlobWithPtr::PutInto(void* p, sLONG len, sLONG offset, sLONG* ActualLen) const
{

	sLONG nbbyte = len;
	if (len == -1)
		nbbyte = fDataLen;
	if ((offset+nbbyte) > fDataLen)
		nbbyte = fDataLen - offset;
	if (nbbyte<0)
		nbbyte = 0;

	if (fData == nil)
		nbbyte = 0;

	if (ActualLen != nil)
		*ActualLen = nbbyte;

	if (nbbyte>0)
	{
		vBlockMove(((char*)fData)+offset, p, nbbyte);
	}
	return VE_OK;
}

VError BlobWithPtr::GetFrom(VStream& buf)
{
	sLONG len;
	VError err;
	
	err = buf.GetLong(len);
	if (err == VE_OK)
	{
		if (fData != nil)
		{
			ReleaseMem( fData);
			fData = nil;
			fDataLen = 0;
		}
		if (len>0)
		{
			fData = (VPtr)AllocateMem(AdjusteSize(len), 'blb4');
			if (fData == nil)
			{
				err = ThrowError(memfull, DBaction_LoadingBlob);
				buf.SetPosByOffset(len);
			}
			else
			{
				fDataLen = len;
				err = buf.GetData( fData, len);
			}
		}
	} 
	
	if (err != VE_OK)
			err = ThrowError(VE_DB4D_CANNOTLOADBLOB, DBaction_LoadingBlob);
	
	return(err);
}


VError BlobWithPtr::PutInto(VStream& buf) const
{
	sLONG len;
	VError err;
	
	len = 0;
	if (fData != nil)
		len = fDataLen;

	err = buf.PutLong(len);
	if (err == VE_OK)
	{
		if (len>0)
			err = buf.PutData( fData, len);
	}
	
	if (err != VE_OK)
			err = ThrowError(VE_DB4D_CANNOTSAVEBLOB, DBaction_SavingBlob);

	return(err);
}



Blob4D* BlobWithPtr::Clone(Boolean WithData) const
{
	BlobWithPtr* result = new BlobWithPtr((DataTable*)GetDF());
	if (result != nil)
	{
		CopyBaseInto(result);
		if (WithData)
		{
			VError err = result->CopyDataFrom(this);
			if (err != VE_OK)
			{
				result->Release();
				result = nil;
			}
		}
	}
	return result;
}
							
	
VError BlobWithPtr::ReadFromStream( VStream* ioStream)
{
	sLONG len = 0;
	VError err = ioStream->GetLong(len);
	if (err == VE_OK)
	{
		err = SetSize(len);
		if (err == VE_OK && len > 0)
		{
			err = ioStream->GetData(fData, len);
		}
	}
	return err;
}


VError BlobWithPtr::WriteToStream( VStream* ioStream, bool withLength) const
{
	sLONG len = 0;
	if (fData != nil)
		len = fDataLen;
	VError err = VE_OK;

	if (withLength)
		err = ioStream->PutLong(len);
	if (len>0 && err == VE_OK)
		err = ioStream->PutData( fData, len);

	return err;
}


bool BlobWithPtr::MatchAllocationNumber(sLONG allocationNumber)
{
	if (allocationNumber == -1)
		return true;
	else
	{
		if (GetAllocationNumber(this) == allocationNumber)
			return true;
		if (fInMainMem || (fData != nil && GetAllocationNumber(fData) == allocationNumber))
			return true;
		return false;
	}
}



Blob4D* CreBlobWithPtr(DataTable *df)
{
	return(new BlobWithPtr(df));
}


							/* *********************************************** */

CodeReg *Blob_CodeReg = nil;

void InitBlob4D()
{
	xbox_assert( Blob_CodeReg == nil);
	Blob_CodeReg = new CodeReg;
	RegisterBlob(blobtext,&CreBlobText);
	RegisterBlob(blobtextUTF8,&CreBlobTextUTF8);
	RegisterBlob(blobptr,&CreBlobWithPtr);
}


void DeInitBlob4D()
{
	if (Blob_CodeReg != nil) {
		delete Blob_CodeReg;
		Blob_CodeReg = nil;
	}
}



