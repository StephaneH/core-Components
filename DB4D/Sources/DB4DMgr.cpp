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
#include "javascript_db4d.h"
#include "Backup.h"

VDBMgr *VDBMgr::sDB4DMgr = nil;
//UniChar VDBMgr::sWildChar = '@';

VCriticalSection	VDBMgr::fMutex;

#if oldtempmem
VDBMgr::VDBMgr():fTempMemMgr( VCppMemMgr::kAllocator_xbox)
#else
VDBMgr::VDBMgr()
#endif
{
	fLimitPerSort = 500; // 500 mega
	fFlushMgr = nil;
	fCacheMgr = nil;
//	lang4D = nil;
//	sLanguage = nil;
	fUAGManager = nil;
	//sWildChar = '@';
	fProgress_Flush = nil;
	fProgress_Indexes = nil;
	fDefaultLocalization = nil;
	fComponentFolder = nil;
	fDefaultServer = nil;
	fServerPort = 0;
	fServerProgressTask = nil;
	fClientProgressTask = nil;
	fRunningMode = DB4D_Running4D;	// will be set afterwards by calling application
	fGraphicsIntf = nil;
	fApplicationIntf = nil;
	fGarbageTask = nil;
#if oldtempmem
	fTempMemSize = 0;
#endif
}

VDBMgr::~VDBMgr()
{
	if (fGarbageTask != nil) {
		fGarbageTask->Kill();
		QuickReleaseRefCountable(fGarbageTask);
	}

	delete fSignaler;
	fSignaler = NULL;
	fDataSetPurger.RemoveFromCache();

	for (mapOfSelections::iterator cur = fServerKeptSelections.begin(), end = fServerKeptSelections.end(); cur != end; cur++)
	{
		cur->second.fSel->Release();
	}

	StopServer(0);
	/*
	if (fDefaultServer != nil)
	{
		fDefaultServer->Stop();
		fDefaultServer->Release();
	}
	*/
	
	if (fFlushMgr != nil) {
		delete fFlushMgr;
		fFlushMgr = nil;
	}

	/*
	for( VIndex i = fBases.GetCount()-1 ; i >= 0 ; --i) {
		if (fBases[i] != nil)
			CloseBase( i);
	}
	*/
	
	DeInitTransactionManager();
	DeInitIndex();
	DeInitBlob4D();
	DeInitChampVar();
	DeInitCritere();

	ReleaseRefCountable( &fDefaultLocalization);

	if (fComponentFolder != nil)
		fComponentFolder->Release();
	if (fCacheMgr != nil) {
		delete fCacheMgr;
		fCacheMgr = nil;
	}
	
//	if (lang4D != nil) lang4D->Release();
//	if (sLanguage != nil) sLanguage->Release();
	QuickReleaseRefCountable(fUAGManager);

	if (fProgress_Indexes != nil)
		fProgress_Indexes->Release();

	if (fProgress_Flush != nil)
		fProgress_Flush->Release();
}


VDBMgr *VDBMgr::GetManager()
{
	assert( sDB4DMgr != nil);
	return sDB4DMgr;
}


VError VDBMgr::Register_DB4DCollectionManagerCreator(sLONG signature, DB4DCollectionManagerCreator Creator)
{
	try
	{
		fCollectionCreators[signature] = Creator;
	}
	catch (...)
	{
		return memfull;
	}
	return VE_OK;
}


VError VDBMgr::UnRegister_DB4DCollectionManagerCreator(sLONG signature)
{
	fCollectionCreators.erase(signature);
	return VE_OK;
}


DB4DCollectionManagerCreator VDBMgr::GetDB4DCollectionManagerCreator(sLONG signature) const
{
	mapCollectionCreator::const_iterator found = fCollectionCreators.find(signature);
	if (found == fCollectionCreators.end())
		return nil;
	else
		return found->second;
}


VError VDBMgr::Register_DB4DArrayOfValuesCreator(sLONG signature, DB4DArrayOfValuesCreator Creator)
{
	try
	{
		fAVCreators[signature] = Creator;
	}
	catch (...)
	{
		return memfull;
	}
	return VE_OK;
}


VError VDBMgr::UnRegister_DB4DArrayOfValuesCreator(sLONG signature)
{
	fAVCreators.erase(signature);
	return VE_OK;
}


DB4DArrayOfValuesCreator VDBMgr::GetDB4DArrayOfValuesCreator(sLONG signature) const
{
	mapAVCreator::const_iterator found = fAVCreators.find(signature);
	if (found == fAVCreators.end())
		return nil;
	else
		return found->second;
}

/*
UniChar VDBMgr::GetWildChar() 
{ 
	return sWildChar; 
}


void VDBMgr::SetWildChar(UniChar c) 
{ 
	sWildChar = c; 
}
*/


bool VDBMgr::ContainsKeyword( const VString& inString, const VString& inKeyword, const VCompareOptions& inOptions, VError& outErr)
{
	VError err = VE_OK;
	bool ok;

	#if USE_OLD_WORD_BOUNDARIES
	
	VCollator *collator = inOptions.GetIntlManager()->GetCollator();
	
	// need expensive algo when asked for pattern matching
	if (inOptions.IsLike() && (inKeyword.FindUniChar( collator->GetWildChar()) > 0))
	{
		DB4DKeyWordList keywords;
		err = BuildKeyWords( inString, keywords, inOptions);
		if (err == VE_OK)
		{
			// on ne peut faire de recherche dichotomique avec une pattern

			//CompareLessStringWithOptions predicate( &inKeyword, inOptions);
			//CompareLessStringWithOptions& predicateRef = predicate;
			//ok = std::binary_search(keywords.First(), keywords.End(), &inKeyword, predicateRef);

			ok = false;
			for( VString **i = keywords.First() ; (i != keywords.End()) && !ok ; ++i)
				ok = collator->EqualString_Like( (*i)->GetCPointer(), (*i)->GetLength(), inKeyword.GetCPointer(), inKeyword.GetLength(), inOptions.IsDiacritical());
		}
		else
		{
			ok = false;
		}
	}
	else
	{
		ok = false;
		sLONG matchedLength;
		const UniChar* pString = inString.GetCPointer();
		sLONG lenString = inString.GetLength();
		sLONG pos;

		do 
		{
			pos = collator->FindString( pString, lenString, inKeyword.GetCPointer(), inKeyword.GetLength(), inOptions.IsDiacritical(), &matchedLength);
			if (pos > 0)
			{
				// see if the found string is enclosed by dead chars
				bool deadChar_onLeft = (pos > 1) ? VDBMgr::IsItaDeadChar( pString[pos-2], inOptions.GetIntlManager()) : true;
				bool deadChar_onRight = (pos + matchedLength < lenString) ? VDBMgr::IsItaDeadChar( pString[pos + matchedLength - 1], inOptions.GetIntlManager()) : true;
				ok = deadChar_onLeft && deadChar_onRight;
				if (!ok)
				{
					pString = pString + pos;
					lenString = lenString - pos;
				}
			}

		} while(pos > 0 && !ok);


		/*
		sLONG matchedLength;
		sLONG pos = collator->FindString( inString.GetCPointer(), inString.GetLength(), inKeyword.GetCPointer(), inKeyword.GetLength(), inOptions.IsDiacritical(), &matchedLength);
		
		if (pos > 0)
		{
			bool deadChar_onLeft = (pos > 1) ? VDBMgr::IsItaDeadChar( inString[pos-2], inOptions.GetIntlManager()) : true;
			bool deadChar_onRight = (pos + matchedLength < inString.GetLength()) ? VDBMgr::IsItaDeadChar( inString[pos + matchedLength - 1], inOptions.GetIntlManager()) : true;
			ok = deadChar_onLeft && deadChar_onRight;
		}
		else
		{
			ok = false;
		}
		*/
	}
	#else
	ok = inOptions.GetIntlManager()->FindWord( inString, inKeyword, inOptions);
	#endif
	
	outErr = err;

	return ok;
}


/*
bool VDBMgr::ContainsKeyword( const VString& inString, const VString& inKeyword, bool inDiacritical, VIntlMgr *inIntlMgr, VError& outErr)
{
	bool ok;

	DB4DKeyWordList keywords;
	VError err = BuildKeyWords( inString, keywords);
	if (err == VE_OK)
	{
		if (inDiacritical)
			ok = std::binary_search(keywords.First(), keywords.End(), &inKeyword);
		else
		{
			CompareLessString predicate( &inKeyword);
			CompareLessString& predicateRef = predicate;
			ok = std::binary_search(keywords.First(), keywords.End(), &inKeyword, predicateRef);
		}
	}
	else
	{
		ok = false;
	}
	
	outErr = err;
	
	return ok;
}


bool VDBMgr::ContainsKeyword_Like( const VString& inString, const VString& inKeyword, bool inDiacritical, VIntlMgr *inIntlMgr, VError& outErr)
{
	bool ok;

	DB4DKeyWordList keywords;
	VError err = BuildKeyWords( inString, keywords);
	if (err == VE_OK)
	{
		if (inDiacritical)
		{
			CompareLessString_Like_DiacSens predicate( &inKeyword);
			CompareLessString_Like_DiacSens& predicateRef = predicate;
			ok = std::binary_search(keywords.First(), keywords.End(), &inKeyword, predicateRef);
		}
		else
		{
			CompareLessString_Like predicate( &inKeyword);
			CompareLessString_Like& predicateRef = predicate;
			ok = std::binary_search(keywords.First(), keywords.End(), &inKeyword, predicateRef);
		}
	}
	else
	{
		ok = false;
	}
	
	outErr = err;
	
	return ok;
}
*/

Boolean VDBMgr::IsItaDeadChar(UniChar c, const VIntlMgr* inIntlMgr)
{
	if (c<32)
		return true;
	else
		return !(inIntlMgr->IsAlpha(c) || inIntlMgr->IsDigit(c));
		//return VIntlMgr::GetDefaultMgr()->IsSpace(c) || VIntlMgr::GetDefaultMgr()->IsPunctuation(c);
	// faire avec une table
	/*
	switch (c)
	{
		case CHAR_FULL_STOP:
		case CHAR_SPACE:
		case CHAR_EXCLAMATION_MARK:
		case CHAR_QUOTATION_MARK:
		case CHAR_APOSTROPHE:
		case CHAR_LEFT_PARENTHESIS:
		case CHAR_RIGHT_PARENTHESIS:
		case CHAR_COMMA:
		case CHAR_HYPHEN_MINUS:
		case CHAR_ASTERISK:
		case CHAR_SOLIDUS:
		case CHAR_COLON:
		case CHAR_SEMICOLON:
		case CHAR_QUESTION_MARK:
		case CHAR_NO_BREAK_SPACE:
		case CHAR_INVERTED_EXCLAMATION_MARK:
		case CHAR_LEFT_POINTING_DOUBLE_ANGLE_QUOTATION_MARK:
		case CHAR_RIGHT_POINTING_DOUBLE_ANGLE_QUOTATION_MARK:
		case CHAR_INVERTED_QUESTION_MARK:
		case CHAR_LEFT_SINGLE_QUOTATION_MARK:
		case CHAR_RIGHT_SINGLE_QUOTATION_MARK:
		case CHAR_SINGLE_LOW_9_QUOTATION_MARK:
		case CHAR_SINGLE_HIGH_REVERSED_9_QUOTATION_MARK:
		case CHAR_LEFT_DOUBLE_QUOTATION_MARK:
		case CHAR_RIGHT_DOUBLE_QUOTATION_MARK:
		case CHAR_CONTROL_000D:
		case CHAR_CONTROL_000A:
			return true;
	}
	return false;
	*/
}

/*

CompareResult VDBMgr::CompareString(const UniChar* inText1, sLONG inSize1, const UniChar* inText2, sLONG inSize2, Boolean inWithDiacritics)
{
	if (inSize2 > 0 && inText2[inSize2-1] == sWildChar)
	{
		inSize2 = inSize2-1;
		if (inSize1 > inSize2)
			inSize1 = inSize2;
	}
	return VIntlMgr::GetDefaultMgr()->CompareString(inText1, inSize1, inText2, inSize2, inWithDiacritics);
}


sLONG VDBMgr::FindStringInPtr(void* HostString, const VString* StrToFind, Boolean inDiacritical)
{
	VIndex pos = 0;

	VIndex findlen = StrToFind->GetLength();
	VIndex len = *((sLONG*)HostString);
	const UniChar* ptrStart = (UniChar*)(((char*)HostString)+4);
	const UniChar* ptrLast = ptrStart + len - findlen;
	for(const UniChar* ptr = ptrStart ; ptr <= ptrLast ; ++ptr)
	{
		if (VIntlMgr::GetDefaultMgr()->EqualString(StrToFind->GetCPointer(), findlen, ptr, findlen, inDiacritical))
		{
			pos = (ptr - ptrStart) + 1;
			break;
		}
	}
	return pos;
}

*/


Boolean VDBMgr::NeedsBytesInCache( sLONG inNeededBytes)
{
	return GetFlushManager()->NeedsBytes( inNeededBytes);
}

void VDBMgr::PutObjectInCache( ObjCache *inObject, Boolean inCheckAtomic)
{
	GetCacheManager()->PutObject( inObject, inCheckAtomic);
}


void VDBMgr::PutObjectInFlush( ObjAlmostInCache *inObject, Boolean inSetCacheDirty, Boolean inForDelete, Boolean NoWait)
{
	GetFlushManager()->PutObject( inObject, inSetCacheDirty, inForDelete, NoWait);
}


void VDBMgr::RemoveObjectFromFlush( ObjAlmostInCache *obj)
{
	GetFlushManager()->RemoveObject( obj);
}



void VDBMgr::PutObjectInCache( IObjToFree *inObject, Boolean inCheckAtomic)
{
	GetCacheManager()->PutObject( inObject, inCheckAtomic);
}


void VDBMgr::RemoveObjectFromCache( IObjToFree *inObject)
{
	GetCacheManager()->RemoveObject(inObject);
}


void VDBMgr::PutObjectInFlush( IObjToFlush *inObject, Boolean inSetCacheDirty, Boolean inForDelete, Boolean NoWait)
{
	GetFlushManager()->PutObject( inObject, inSetCacheDirty, inForDelete, NoWait);
}


void VDBMgr::RemoveObjectFromFlush( IObjToFlush *obj)
{
	GetFlushManager()->RemoveObject( obj);
}
VDBFlushMgr *VDBMgr::GetFlushManager()
{
	return GetManager()->fFlushMgr;
}


IBackupSettings* VDBMgr::CreateBackupSettings()
{
	return new VBackupSettings();
}

XBOX::VError VDBMgr::GetJournalInfo(const XBOX::VFilePath& inDataFilePath,XBOX::VFilePath& outJournalPath,XBOX::VUUID& outDataLink)
{
	XBOX::VError error = VE_OK;
	XBOX::VFilePath extraPropsPath(inDataFilePath);
	extraPropsPath.SetExtension(CVSTR("waExtra"));
	XBOX::VFile* extraPropsFile = new VFile(extraPropsPath);
	XBOX::VFileStream extraPropsStream(extraPropsFile);
	error  = VE_FILE_NOT_FOUND;
	
	//WAK0078901: avoid causing unnecessary throws and test that the extra props file actully exists
	if (extraPropsFile->Exists())
	{
		error = VE_OK;
		error = extraPropsStream.OpenReading();
		if ( error == VE_OK)
		{
			VString jsonString;
			error = extraPropsStream.GetText(jsonString);
			if (error == VE_OK)
			{
				VValueBag *extraBag = new VValueBag();
				extraBag->FromJSONString(jsonString);
				const VValueBag* journal_bag = extraBag->RetainUniqueElement( LogFileBagKey::journal_file );
				if ( journal_bag != NULL )
				{
					VString logFilePath;
					journal_bag->GetString( LogFileBagKey::filepath, logFilePath );
					journal_bag->GetVUUID( LogFileBagKey::datalink, outDataLink );
					if( !logFilePath.IsEmpty() )
					{
						if ( logFilePath.BeginsWith(CVSTR(".")))
						{
							outJournalPath.FromRelativePath(inDataFilePath,logFilePath);
						}
						else
						{
							outJournalPath.FromFullPath(logFilePath);
						}
					}
				}
				XBOX::ReleaseRefCountable(&journal_bag);
				XBOX::ReleaseRefCountable(&extraBag);
			}
		}
		extraPropsStream.CloseReading();
	}
	XBOX::ReleaseRefCountable(&extraPropsFile);
	return error;
}


IBackupTool* VDBMgr::CreateBackupTool()
{
	return new VBackupTool();
}

IJournalTool* VDBMgr::CreateJournalTool()
{
	return new VJournalTool();
}

CDB4DBase* VDBMgr::OpenBase( const VFile& inStructureFile, sLONG inParameters, VError* outErr, FileAccess inAccess, VString* EntityFileExt, 
							CUAGDirectory* inUAGDirectory, const VString* inHtmlContent, unordered_map_VString<VRefPtr<VFile> >* outIncludedFiles, const VFile* inPermissionsFile )
{
	return xOpenCreateBase( inStructureFile, false, inParameters, nil, outErr, inAccess, nil,  EntityFileExt, inUAGDirectory, inHtmlContent, outIncludedFiles, inPermissionsFile );
}

#if 0
CLanguage* VDBMgr::GetLanguage()
{
	return GetManager()->sLanguage;
}
#endif

#if 0
CDB4D_Lang* VDBMgr::GetLang4D()
{
	return GetManager()->lang4D;
}
#endif

/*
sLONG VDBMgr::CloseBase(DB4D_BaseID inBase)
{
	Base4D *bd = nil;

	verror err = xUnregisterBase( inBase, &bd);
	if (err == 0) {
		fFlushMgr->Flush( true);	// L.E. 26/12/00 shouldn't be necessary
		err = bd->CloseBase();
		if (err != 0) {
			// on le reenregistre. ca change son id mais c'est pas grave car personne l'utililise.
			xRegisterBase( bd, nil);
		}
		bd->Release();
	}
	
	return err;
}
*/


void VDBMgr::AddBlobPathToDelete(const VString& inBlobPath)
{
	{
		VTaskLock lock(&fBlobPathsToDeleteMutex);
		fBlobPathsToDelete.insert(inBlobPath);
	}
	VDBMgr::GetFlushManager()->SetDirty(true);
}


void VDBMgr::RemoveBlobPathToDelete(const VString& inBlobPath)
{
	{
		VTaskLock lock(&fBlobPathsToDeleteMutex);
		fBlobPathsToDelete.erase(inBlobPath);
	}
	VDBMgr::GetFlushManager()->SetDirty(true);
}



void VDBMgr::StealBlobPathsToDelete(DataPathCollection& outList)
{
	VTaskLock lock(&fBlobPathsToDeleteMutex);
	outList.swap(fBlobPathsToDelete);
}


void VDBMgr::DeleteDeadBlobPaths()
{
	StErrorContextInstaller errs(false);
	DataPathCollection blobPathsToDelete;
	StealBlobPathsToDelete(blobPathsToDelete);
	for (DataPathCollection::iterator cur = blobPathsToDelete.begin(), end = blobPathsToDelete.end(); cur != end; cur++)
	{
		VFile ff(*cur, FPS_POSIX);
		ff.Delete();
	}
}



VError VDBMgr::RebuildStructure( const VFile& inStructureFile, const VFile& inDataFile)
{
	VFileDesc *firstSegFileDesc = NULL;
	VUUID dataID;
	bool ok = false;
	VError err = inDataFile.Open(FA_READ, &firstSegFileDesc);
	if (err == VE_OK)
	{
		BaseHeader hhb;
		err = firstSegFileDesc->GetData(&hhb, sizeof(hhb), 0);
		if (err == VE_OK)
		{
			dataID.FromBuffer(hhb.ID);
			ok = true;
		}
		delete firstSegFileDesc;
	}

	if (ok)
	{
		CDB4DBase* base = CreateBase(inStructureFile, DB4D_Create_WithSeparateIndexSegment, VIntlMgr::GetDefaultMgr(), &err, FA_READ_WRITE, &dataID);
		if (base != nil)
		{
			base->OpenData(inDataFile, DB4D_Open_WithSeparateIndexSegment, nil, &err, FA_READ_WRITE);
			if (err == VE_OK)
			{
				vector<VString> tNames;
				vector<VUUID> tIDs;
				err = base->GetListOfDeadTables(tNames, tIDs, nil);
				if (err == VE_OK)
				{
					for (vector<VString>::iterator cur = tNames.begin(), end = tNames.end(); cur != end && err == VE_OK; cur++)
					{
						err = base->ResurectDataTable(*cur, nil);
					}
				}
			}
			FlushCache(true);
			base->CloseAndRelease(true);
		}
	}

	if (err != VE_OK)
		err = ThrowBaseError(VE_DB4D_CANNOT_REBUILD_MISSING_STRUCT);
	return err;
}


CDB4DBase* VDBMgr::CreateBase( const VFile& inStructureFile, sLONG inParameters, VIntlMgr* inIntlMgr, VError* outErr, FileAccess inAccess, const VUUID* inChosenID, VString* EntityFileExt, CUAGDirectory* inUAGDirectory, const VFile* inPermissionsFile)
{
	return xOpenCreateBase( inStructureFile, true, inParameters, inIntlMgr, outErr, inAccess, inChosenID, EntityFileExt, inUAGDirectory, nil, nil, inPermissionsFile);
}


CDB4DBase* VDBMgr::xOpenCreateBase( const VFile& inStructureFile, Boolean inCreate, sLONG inParameters, VIntlMgr* inIntlMgr, VError* outErr, FileAccess inAccess, 
								     const VUUID* inChosenID, VString* EntityFileExt, CUAGDirectory* inUAGDirectory, 
									 const VString* inXmlContent, unordered_map_VString<VRefPtr<VFile> >* outIncludedFiles,
									 const VFile* inPermissionsFile)
{
//	ClearError();
	VError err = VE_OK;
	
	Base4D *basd = new Base4D(fFlushMgr, inAccess != FA_READ);
	VDB4DBase *xbd = nil;

	if (basd == nil) {
		err = ThrowBaseError(memfull, DBaction_OpeningBase);
	} else {
		
		if (inCreate)
			err=basd->CreateStructure( inStructureFile, inParameters, inIntlMgr, inAccess, inChosenID, EntityFileExt, inUAGDirectory, inPermissionsFile);
		else
			err=basd->OpenStructure( inStructureFile, inParameters, inAccess, EntityFileExt, inUAGDirectory, inXmlContent, outIncludedFiles, inPermissionsFile );
		
		if (err == 0 && ((inParameters & DB4D_Open_DefaultData) != 0) )
		{
			VFile *dataFile = basd->RetainDefaultDataFile();
			if (dataFile != NULL)
			{
				if (inCreate)
					err = basd->CreateData( *dataFile, inParameters, inIntlMgr, nil, false, inAccess, inChosenID);
				else
				{
					StErrorContextInstaller errs(true);
					err = basd->OpenData( *dataFile, inParameters, false, true, inAccess);
					if (err != VE_OK && !basd->FlushNotCompleted())
					{
						errs.Flush();
						err = basd->CreateData( *dataFile, inParameters, inIntlMgr, nil, false, inAccess, inChosenID);
					}
				}
			}
			ReleaseRefCountable( &dataFile);
			
			if (err != VE_OK)
				basd->CloseBase();
		}

		Base4D* xstruct = basd->GetStructure();
		if (xstruct != nil && xstruct->FlushNotCompleted())
			err = xstruct->ThrowError(VE_DB4D_FLUSH_COMPLETE_ON_STRUCT, DBaction_OpeningBase);
		else
		{
			if (basd->FlushNotCompleted())
				err = basd->ThrowError(VE_DB4D_FLUSH_COMPLETE_ON_DATA, DBaction_OpeningBase);
		}
				
		basd->libere();

		if (err == VE_OK)
		{
			xbd = new VDB4DBase( this, basd, true);
			if (xbd == nil)
				err = ThrowBaseError(memfull, DBaction_OpeningBase);
		}

		if (err != VE_OK)
		{
			basd->Release();
			basd = nil;
		}
	}
	
	if (outErr != nil)
		*outErr = err;

	return(xbd);
}




void VDBMgr::FlushAllData()
{
}



void VDBMgr::FlushCache(Boolean WaitUntilDone, Boolean onlyForAllWritten)
{
	fFlushMgr->Flush(WaitUntilDone, onlyForAllWritten);
}


void VDBMgr::FlushCache(Base4D* target, Boolean WaitUntilDone)
{
	fFlushMgr->Flush(target, WaitUntilDone);
}


void VDBMgr::SetFlushPeriod( sLONG inMillisecondsPeriod)
{
	fFlushMgr->SetFlushPeriod( inMillisecondsPeriod);
}


VError VDBMgr::SetCacheSize( VSize inSize, bool inPhysicalMemOnly, VSize *outActualCacheSize)
{
	return fCacheMgr->SetMaxSize(inSize, inPhysicalMemOnly, outActualCacheSize);
}

VError VDBMgr::SetMinSizeToFlush( VSize inMinSizeToFlush)
{
	return fCacheMgr->SetMinSizeToFlush( inMinSizeToFlush);
}

VSize VDBMgr::GetCacheSize()
{
	return fCacheMgr->GetMaxSize();
}

VSize VDBMgr::GetMinSizeToFlush()
{
	return fCacheMgr->GetMinSizeToFlush();
}

VDBCacheMgr *VDBMgr::GetCacheManager()
{
	return GetManager()->fCacheMgr;
}


void VDBMgr::RemoveObjectFromCache( ObjCache *obj)
{
	GetCacheManager()->RemoveObject( obj);
}

#if oldtempmem
VError VDBMgr::SetTempMemSize(VSize inSize, VSize *outActualSize)
{
	*outActualSize = fTempMemSize;
	VError err = VE_OK;
	if (inSize > fTempMemSize)
	{
		VSize diff = inSize - fTempMemSize;
		if (diff >= 0x80000)
		{
			if (fTempMemMgr.AddVirtualAllocation(diff, NULL, true))
			{
				*outActualSize = inSize;
				fTempMemSize = inSize;
			}
			else
				err = ThrowBaseError(memfull, DBaction_ChangeCacheSize);
		}
	}
	return err;
}
#endif


VDBMgr *VDBMgr::RetainManager(VComponentLibrary* DB4DCompLib)
{
	VTaskLock lock( &fMutex);
	
	VDBMgr *manager = sDB4DMgr;
	if (sDB4DMgr != nil) {
		sDB4DMgr->Retain();
	} else {
		manager = new VDBMgr;
		if (!manager->Init(DB4DCompLib)) {
			manager->Release();
			manager = nil;
		}
		sDB4DMgr = manager;
	}

	return manager;
}

#if 0

void VDBMgr::AddErrorString(sLONG n, VString& s)
{
	Boolean ok = true;
	if (n>=ErrorStrings.GetCount())
	{
			ok = ErrorStrings.AddNSpaces(n-ErrorStrings.GetCount()+1, true);
	}
	if (ok)
	{
		VString* x = new VString(s);
		ErrorStrings[n] = x;
	}
}


void VDBMgr::AddActionString(sLONG n, VString& s)
{
	Boolean ok = true;
	if (n>=ActionStrings.GetCount())
	{
			ok = ActionStrings.AddNSpaces(n-ActionStrings.GetCount()+1, true);
	}
	if (ok)
	{
		VString* x = new VString(s);
		ActionStrings[n] = x;
	}
}


void VDBMgr::GetErrorString(sLONG n, VString& outMess) const
{
	if (n>0 && n<ErrorStrings.GetCount())
	{
		VString* s = ErrorStrings[n];
		if (s != nil)
			outMess = *s;
	}
}

void VDBMgr::GetErrorActionString(sLONG n, VString& outMess) const
{
	if (n>0 && n<ActionStrings.GetCount())
	{
		VString* s = ActionStrings[n]; 
		if (s != nil)
			outMess = *s;
	}
}


void VDBMgr::InitErrors()
{
	VError err;
#if 0
	{
		VFile f2(L"C:\\test.xml");
		f2.Create(true);
		VFileStream data(&f2);
		data.OpenWriting();
		VValueBag b;

		VBagArray* barr = new VBagArray();
		for (sLONG k = 1; k<11; k++)
		{
			VValueBag* b2 = new VValueBag();
			b2->SetLong(L"Number", k);
			b2->SetString(L"Text", L"Toto");
			barr->AddTail(b2);
		}

		b.SetElements(L"ErrorString",barr);

		vWriteBagToStreamInXML(b, &data);
		data.CloseWriting();
		f2.Close();

	}
#endif

#if 0
	VFolder* xmlfolder = gApplication->GetPrivateFolder(eFS_XMLFilesFolder);
	if (xmlfolder != nil)
	{
		VFile f(*xmlfolder, "DB4DErrors.xml");
		
		err = f.Open(FA_READ);

		if (err == VE_OK)
		{
			VValueBag bag;
			VXMLBagHandler_UniqueElement handler( &bag, L"ErrorStringArray");
			
			VXMLParser parser;
			parser.Parse( &f, &handler);

			if (err == VE_OK)
			{
				VBagArray* strs = bag.RetainElements(L"ErrorString");
				for (sLONG i = strs->GetCount(); i>0; i--)
				{
					VValueBag* elem = strs->RetainNth(i);
					sLONG numerr;
					VString stext;
					if (elem->GetLong(L"Number", &numerr) && elem->GetString(L"Text", stext) )
					{
						AddErrorString(numerr-1000, stext);
					}
					elem->Release();
				}
				strs->Release();

				ActionStrings.AddNSpaces((sLONG)DBactionFinale, true);
				strs = bag.RetainElements(L"ActionString");
				for (sLONG i = strs->GetCount(); i>0; i--)
				{
					VValueBag* elem = strs->RetainNth(i);
					sLONG numerr;
					VString stext;
					if (elem->GetLong(L"Number", &numerr) && elem->GetString(L"Text", stext) )
					{
						AddActionString(numerr, stext);
					}
					elem->Release();
				}
				strs->Release();
			}

			f.Close();
		}
	}
#endif
	
}

#endif


void VDBMgr::SetDefaultProgressIndicator_For_Indexes(VDB4DProgressIndicator* inProgress)
{
	VTaskLock lock(&fMutex);
	CopyRefCountable(&fProgress_Indexes, inProgress);
	if (fProgress_Indexes != nil)
		fProgress_Indexes->SetUserInfo("indexProgressIndicator");
}


VDB4DProgressIndicator* VDBMgr::RetainDefaultProgressIndicator_For_Indexes()
{
	VTaskLock lock(&fMutex);
	if (fProgress_Indexes == nil)
	{
		if (IsRunningWakanda())
		{
			fProgress_Indexes = new VProgressIndicator();
			fProgress_Indexes->SetUserInfo("indexProgressIndicator");
			fProgress_Indexes->Retain();
			return fProgress_Indexes;
		}
		else
			return nil;
	}
	else
	{
		fProgress_Indexes->Retain();
		return fProgress_Indexes;
	}
}


void VDBMgr::SetDefaultProgressIndicator_For_DataCacheFlushing(VDB4DProgressIndicator* inProgress)
{
	VTaskLock lock(&fMutex);
	CopyRefCountable(&fProgress_Flush, inProgress);
	if (fProgress_Flush != nil)
		fProgress_Flush->SetUserInfo("flushProgressIndicator");
}


VDB4DProgressIndicator* VDBMgr::RetainDefaultProgressIndicator_For_DataCacheFlushing()
{
	VTaskLock lock(&fMutex);
	if (fProgress_Flush == nil)
	{
		if (IsRunningWakanda())
		{
			fProgress_Flush = new VProgressIndicator();
			fProgress_Flush->SetUserInfo("flushProgressIndicator");
			fProgress_Flush->Retain();
			return fProgress_Flush;
		}
		else
			return nil;
	}
	else
	{
		fProgress_Flush->Retain();
		return fProgress_Flush;
	}
}


BuildLanguageExpressionMethod VDBMgr::SetMethodToBuildLanguageExpressions(BuildLanguageExpressionMethod inNewMethodToBuildLanguageExpressions)
{
	VTaskLock lock(&fMutex);
	BuildLanguageExpressionMethod old = fCurrentBuildLanguageExpressionMethod;
	fCurrentBuildLanguageExpressionMethod = inNewMethodToBuildLanguageExpressions;
	return old;
}



DB4DCommandManager* VDBMgr::SetCommandManager(DB4DCommandManager* inNewCommandManager)
{
	VTaskLock lock(&fMutex);
	DB4DCommandManager* old = fCommandManager;
	fCommandManager = inNewCommandManager;
	return old;
}


DB4DActivityManager* VDBMgr::SetActivityManager(DB4DActivityManager* inNewActivityManager)
{
	VTaskLock lock(&fMutex);
	DB4DActivityManager* old = fActivityManager;
	fActivityManager = inNewActivityManager;
	return old;
}


DB4DNetworkManager* VDBMgr::SetNetworkManager(DB4DNetworkManager* inNewNetworkManager)
{
	VTaskLock lock(&fMutex);
	DB4DNetworkManager* old = fNetworkManager;
	fNetworkManager = inNewNetworkManager;
	return old;
}


Boolean VDBMgr::Init(VComponentLibrary* DB4DCompLib)
{
	fLastDataSetPurge = 0;
	fRequestLogger = nil;
	fDB4DCompLib = DB4DCompLib;
	gCppMem = VObject::GetMainMemMgr();
	if (sDB4DMgr == nil)
		sDB4DMgr = this;

	Boolean isOK = false;

	sLONG taille = sizeof(TypeSortElem<sWORD>);
	sLONG taille2 = sizeof(sWORD);
	
	fComponentFolder = DB4DCompLib->GetLibrary()->RetainFolder(kBF_BUNDLE_FOLDER);
	fDefaultLocalization = new VLocalizationManager(VComponentManager::GetLocalizationLanguage(fComponentFolder,true));
	if (fComponentFolder != nil)
		fDefaultLocalization->LoadDefaultLocalizationFoldersForComponentOrPlugin(fComponentFolder);

	VErrorBase::RegisterLocalizer(CDB4DManager::Component_Type, fDefaultLocalization);

	//InitErrors();

	VSize dejasize;
#if oldtempmem
	SetTempMemSize(50000000, &dejasize);
#endif
	fCacheMgr = VDBCacheMgr::NewCacheManager( this);
	if (testAssert( fCacheMgr != nil)) {
			fCacheMgr->Init();
		InitFileExtensions(fRunningMode);
	#if journalcache
		InitJournal();
	#endif
		InitBlob4D();
		InitChampVar();
		InitCritere();
		
		fFlushMgr = VDBFlushMgr::NewFlushManager( this);
		if (testAssert( fFlushMgr != nil)) {
			InitIndex(this);
			InitTransactionManager();
			isOK = true;
		}
	}

//	lang4D = (CDB4D_Lang*) VComponentManager::RetainComponent((CType)CDB4D_Lang::Component_Type);
//	sLanguage = (CLanguage*) VComponentManager::RetainComponent((CType)CLanguage::Component_Type);
	fUAGManager = (CUAGManager*) VComponentManager::RetainComponent((CType)CUAGManager::Component_Type);
	
	fActivityManager = nil;
	fNetworkManager = nil;
	fSignaler = new VDB4DSignaler( this);
	fCommandManager = dynamic_cast<DB4DCommandManager*>( fSignaler);

	Register_DB4DArrayOfValuesCreator(kDBAVSIG, DBAVVCreator);
//	Register_DB4DArrayOfValuesCreator('cons', DBAVConsCreator);

	/*
	fDefaultServer = new ServerManager();
	fDefaultServer->Start(DB4D_Default_Server_Port);
	*/

	// Init javascript classes definition
	CreateGlobalDB4DClasses();
	
#if 0
	VFile fff(L"c:\\toto.txt");
	{
		VString s;
		VFileStream vff(&fff);
		vff.OpenReading();
		vff.GetText(s);
		VValueBag bag;
		bag.FromJSONString(s);
		VString s2;
		bag.GetJSONString(s2, JSON_PrettyFormatting);
		DebugMsg(s2);
		vff.CloseReading();
	}
#endif

	fDataSetPurger.PutInCache();

	fGarbageTask = new VGarbageTask( this);
	fGarbageTask->SetName( CVSTR( "Garbage Handler"));
	fGarbageTask->Run();

	return isOK;
}


VError VDBMgr::RegisterBase(Base4D* base)
{
	VError err = VE_OK;
	VTaskLock lock( &fBasesMutex);
	sLONG i,n = 0, nb = fBases.GetCount();

	for (i = 0; i < nb; i++)
	{
		if (fBases[i].base == nil) 
		{
			n = i+1;
			break;
		}
	}

	base4dinfo bi;
	bi.base = base;
	bi.isClosing = false;

	if (n == 0)
	{
		if (! fBases.Add(bi))
		{
			err = ThrowBaseError(memfull, DBaction_RegisteringBase);
		}
	}
	else
		fBases[n-1] = bi;

	return err;
}


void VDBMgr::UnRegisterBase(Base4D* base) // means CloseDatabase
{
	VTaskLock lock( &fBasesMutex);
	sLONG i,nb = fBases.GetCount();

	for (i = 0; i<nb; i++)
	{
		if (!fBases[i].isClosing)
		{
			if (fBases[i].base == base)
			{
				fBases[i].isClosing = true;
				base->Release();
				// attention, je fais expres de ne pas mettre fBases[i].base a nil
				// fBases[i].base est donc une reference instable qui ne dois plus etre utilisee que pour comparer son pointeur
				break;
			}
		}
	}
}


void VDBMgr::FinishUnRegisterBase(Base4D* base)
{
	VTaskLock lock( &fBasesMutex);
	sLONG i,nb = fBases.GetCount();

	for (i = 0; i<nb; i++)
	{
		if (fBases[i].isClosing)
		{
			if (fBases[i].base == base)
			{
				fBases[i].base = nil;
			}
		}
	}
}


CDB4DBase* VDBMgr::RetainNthBase( sLONG inBaseIndex)
{
	CDB4DBase* xbase = nil;
	Base4D* base = nil;
	VTaskLock lock( &fBasesMutex);

	if (inBaseIndex>0 && inBaseIndex<=fBases.GetCount())
	{
		if (!fBases[inBaseIndex-1].isClosing)
			base = fBases[inBaseIndex-1].base;
	}

	if (base != nil)
	{
		xbase = new VDB4DBase(this, base, true);
	}

	return xbase;
}


sLONG VDBMgr::CountBases() const
{
	VTaskLock lock( &fBasesMutex);
	return fBases.GetCount();
}


CDB4DBase* VDBMgr::RetainBaseByUUID(const VUUID& inBaseID)
{
	CDB4DBase* xbase = nil;
	Base4D* base = nil;
	VTaskLock lock( &fBasesMutex);
	sLONG i,nb = fBases.GetCount();

	for (i = 0; i<nb; i++)
	{
		if (!fBases[i].isClosing)
		{
			if (fBases[i].base->GetUUID() == inBaseID)
			{
				base = fBases[i].base;
				break;
			}
		}
	}

	if (base != nil)
	{
		xbase = base->RetainBaseX();
		//xbase = new VDB4DBase(this, base, true);
	}

	return xbase;
}



Base4D* VDBMgr::RetainBase4DByUUID(const VUUID& inBaseID)
{
	Base4D* base = nil;
	VTaskLock lock( &fBasesMutex);
	sLONG i,nb = fBases.GetCount();

	for (i = 0; i<nb; i++)
	{
		if (!fBases[i].isClosing)
		{
			if (fBases[i].base->GetUUID() == inBaseID)
			{
				base = fBases[i].base;
				base->Retain();
				break;
			}
		}
	}

	return base;
}


Base4D* VDBMgr::RetainBase4DByName(const VString& inName)
{
	Base4D* base = nil;
	VTaskLock lock( &fBasesMutex);
	sLONG i,nb = fBases.GetCount();
	VStr<512> s;

	for (i = 0; i<nb; i++)
	{
		if (!fBases[i].isClosing)
		{
			fBases[i].base->GetName(s);
			if (s == inName)
			{
				base = fBases[i].base;
				base->Retain();
				break;
			}
		}
	}

	return base;
}


const VValueBag* VDBMgr::RetainExtraProperties( VFile* inFile, bool *outWasOpened, VUUID *outUUID, VError &err, CDB4DBaseContextPtr inContext, uLONG8* outVersionNumber)
{
	const VValueBag *bag = NULL;
	bool wasOpened = false;
	
	// look on opened bases first
	{
		VTaskLock lock( &fBasesMutex);
		sLONG i,nb = fBases.GetCount();

		for (i = 0; i<nb; i++)
		{
			Base4D *base_data = fBases[i].base;
			if (!fBases[i].isClosing && (base_data != NULL) )
			{
				if ( (base_data->CountDataSegs() > 0) && base_data->GetDataSegs()->GetFirst()->GetFile()->IsSameFile( inFile))
				{
					bag = fBases[i].base->RetainExtraProperties( err, inContext);
					if (outUUID != nil)
						fBases[i].base->GetUUID( *outUUID);
					if (outVersionNumber != nil)
						*outVersionNumber = base_data->GetVersionNumber();
					wasOpened = true;
					break;
				}

				Base4D *base_structure = base_data->GetStructure();
				if ( (base_structure != NULL) && (base_structure->CountDataSegs() > 0) && base_structure->GetDataSegs()->GetFirst()->GetFile()->IsSameFile( inFile))
				{
					bag = base_structure->RetainExtraProperties( err, inContext);
					if (outUUID != nil)
						base_structure->GetUUID( *outUUID);
					if (outVersionNumber != nil)
						*outVersionNumber = base_structure->GetVersionNumber();
					wasOpened = true;
					break;
				}
			}
		}
	}
	
	if ( (bag == NULL) && !wasOpened)
	{
		bag = _RetainExtraPropertiesOnClosedBase( inFile, outUUID, err, outVersionNumber);
	}
	
	if (outWasOpened != NULL)
		*outWasOpened = wasOpened;
	
	return bag;
}


void VDBMgr::DoOnRefCountZero()
{
	VTaskLock lock( &fMutex);
	
	// RetainManager may have been called inbetween Release and DoOnRefCountZero
	if (GetRefCount() == 0) {
		assert( this == sDB4DMgr);
		//sDB4DMgr = nil;
		IRefCountable::DoOnRefCountZero();
	}
}


CDB4DContext* VDBMgr::NewContext(CUAGSession* inUserSession, VJSGlobalContext* inJSContext, bool islocal)
{
	VDB4DContext* res = new VDB4DContext(inUserSession, inJSContext, islocal);
	if (res != nil)
	{
		res->GenerateID();
		{
			VTaskLock lock(&fAllContextsMutex);
			try
			{
				fAllContexts[res->GetID().GetBuffer()] = res;
			}
			catch (...)
			{
				res->Release();
				res = nil;
			}
		}
	}

	if (res == nil)
		ThrowBaseError(memfull, noaction);

	return res;
}


CDB4DContext* VDBMgr::RetainOrCreateContext(const VUUID& inID, CUAGSession* inUserSession, VJSGlobalContext* inJSContext, bool islocal)
{
	CDB4DContext* result = nil;
	VTaskLock lock(&fAllContextsMutex);
	mapOfContext::iterator found = fAllContexts.find(inID.GetBuffer());
	if (found == fAllContexts.end())
	{
		result = new VDB4DContext(inUserSession, inJSContext, islocal);
		if (result != nil)
		{
			VImpCreator<VDB4DContext>::GetImpObject(result)->SetID(inID);
			try
			{
				fAllContexts[inID.GetBuffer()] = result;
			}
			catch (...)
			{
				result->Release();
				result = nil;
			}
		}
	}
	else
	{
		result = found->second;
		result->Retain();
	}

	if (result == nil)
		ThrowBaseError(memfull, noaction);

	return result;

}


void VDBMgr::UnRegisterContext(const VUUID& inID)
{
	VTaskLock lock(&fAllContextsMutex);
	fAllContexts.erase(inID.GetBuffer());
}



void VDBMgr::KeepPostPonedContext(const VString& sessionID, CDB4DContext* inContext)
{
	VTaskLock lock(&fAllContextsMutex);
	inContext->Retain();
	fPostPonedContexts[sessionID] = inContext;
}


CDB4DContext* VDBMgr::StealPostPonedContext(const VString& sessionID)
{
	VTaskLock lock(&fAllContextsMutex);
	mapOfContextBySessionID::iterator found = fPostPonedContexts.find(sessionID);
	CDB4DContext* result = nil;
	if (found != fPostPonedContexts.end())
	{
		result = found->second;
		fPostPonedContexts.erase(found);
	}
	return result;
}


void VDBMgr::ForgetPostPonedContext(CDB4DContext* inContext)
{
	VTaskLock lock(&fAllContextsMutex);
	for (mapOfContextBySessionID::iterator cur = fPostPonedContexts.begin(), end = fPostPonedContexts.end(); cur != end; cur++)
	{
		if (inContext == cur->second)
		{
			inContext->Release();
			fPostPonedContexts.erase(cur);
			break;
		}
	}
}


void VDBMgr::ForgetAllPostPonedContext()
{
	VTaskLock lock(&fAllContextsMutex);
	for (mapOfContextBySessionID::iterator cur = fPostPonedContexts.begin(), end = fPostPonedContexts.end(); cur != end; cur++)
	{
		cur->second->Release();
	}
	fPostPonedContexts.clear();
}



VError VDBMgr::StartServer(const VString& inServerName, sWORD inPortNum, bool inSSL)
{
	if (inPortNum == 0)
		inPortNum = DB4D_Default_Server_Port;
	VError err = VE_OK;
	VTaskLock lock(&fDefaultServerMutex);
	if (fDefaultServer == nil)
	{
		fServerPort = inPortNum;
		fDefaultServer = new ServerManager();
		if (fDefaultServer == nil)
			err = ThrowBaseError(memfull, DBaction_StartingServer);
		else
			err = fDefaultServer->Start(inPortNum, inSSL);
	}
	return err;
}


VError VDBMgr::StopServer(sLONG inTimeOut)
{	
	VTaskLock lock(&fDefaultServerMutex);
	if (fDefaultServer != nil)
	{
		fDefaultServer->Stop();
		fDefaultServer->Release();
		fDefaultServer = nil;
	}
	
	return VE_OK;
}


void VDBMgr::CloseAllClientConnections(sLONG inTimeOut)
{
	DB4DConnectionHandler::CloseAllConnections();
}


void VDBMgr::CloseClientConnection(CDB4DContext* inContext)
{
	DB4DConnectionHandler::CloseConnection(inContext);
}


void VDBMgr::MarkSetOnServerAsPermanent(Bittab* set)
{
	if (set != nil)
	{
		VTaskLock lock(&fServerKeptSelectionsMutex);
		mapOfSelections::iterator found = fServerKeptSelections.find(*(set->GetID()));

		if ( found == fServerKeptSelections.end())
		{
			set->GenerateID(false);
			BitSel* sel = new BitSel(nil, set, set->GetOwner());
			assert(sel != nil);
#if debugLeakCheck_KeptSelections
			if (debug_candumpleaks)
				DumpStackCrawls();
			if (debug_canRegisterLeaks)
				RegisterStackCrawl(sel);
#endif
			fServerKeptSelections[*(set->GetID())] = KeptSelection(sel, false, true);
		}
		else
			found->second.fRetainedByServer = true;
	}
}

void VDBMgr::KeepSetOnServer(Bittab* set)
{
	if (set != nil)
	{
		VTaskLock lock(&fServerKeptSelectionsMutex);
		mapOfSelections::iterator found = fServerKeptSelections.find(*(set->GetID()));

		if ( found == fServerKeptSelections.end())
		{
			set->GenerateID(false);
			BitSel* sel = new BitSel(nil, set, set->GetOwner());
			assert(sel != nil);
#if debugLeakCheck_KeptSelections
			if (debug_candumpleaks)
				DumpStackCrawls();
			if (debug_canRegisterLeaks)
				RegisterStackCrawl(sel);
#endif
			fServerKeptSelections[*(set->GetID())] = KeptSelection(sel, true, false);
		}
		else
			found->second.fRetainedByClient = true;
	}
}


void VDBMgr::MarkSelectionOnServerAsPermanent(Selection* sel)
{
	if (sel != nil && sel->MustBeKeptOnServer())
	{
		VTaskLock lock(&fServerKeptSelectionsMutex);
		mapOfSelections::iterator found = fServerKeptSelections.find(*(sel->GetID()));

		if ( found == fServerKeptSelections.end())
		{
#if debugLeakCheck_KeptSelections
			if (debug_candumpleaks)
				DumpStackCrawls();
			if (debug_canRegisterLeaks)
				RegisterStackCrawl(sel);
#endif
			sel->GenerateID(false);
			fServerKeptSelections[*(sel->GetID())] = KeptSelection(sel, false, true);
			sel->Retain();
		}
		else
			found->second.fRetainedByServer = true;
	}
}


void VDBMgr::KeepSelectionOnServer(Selection* sel)
{
	if (sel != nil && sel->MustBeKeptOnServer())
	{
		VTaskLock lock(&fServerKeptSelectionsMutex);
		mapOfSelections::iterator found = fServerKeptSelections.find(*(sel->GetID()));

		if (found == fServerKeptSelections.end())
		{
#if debugLeakCheck_KeptSelections
			if (debug_candumpleaks)
				DumpStackCrawls();
			if (debug_canRegisterLeaks)
				RegisterStackCrawl(sel);
#endif
			sel->GenerateID(false);
			fServerKeptSelections[*(sel->GetID())] = KeptSelection(sel, true, false);
			sel->Retain();
		}
		else
		{
			found->second.fRetainedByClient = true;
		}
	}
}


Selection* VDBMgr::RetainServerKeptSelection(const VUUIDBuffer& inID)
{
	Selection* sel = nil;
	VTaskLock lock(&fServerKeptSelectionsMutex);
	mapOfSelections::iterator found = fServerKeptSelections.find(inID);
	if (found != fServerKeptSelections.end())
	{
		sel = found->second.fSel;
		sel->Retain();
	}

	return sel;
}


void VDBMgr::ForgetServerKeptSelection(const VUUIDBuffer& inID)
{
	VTaskLock lock(&fServerKeptSelectionsMutex);
	mapOfSelections::iterator found = fServerKeptSelections.find(inID);
	if (found != fServerKeptSelections.end())
	{
#if debugLeakCheck_KeptSelections
		UnRegisterStackCrawl(found->second);
		if (debug_candumpleaks)
			DumpStackCrawls();
#endif
		if (found->second.fRetainedByServer)
			found->second.fRetainedByClient = false;
		else
		{
			found->second.fSel->Release();
			fServerKeptSelections.erase(found);
		}

	}
}


void VDBMgr::UnMarkSetOnServerAsPermanent(Bittab* set, bool willResend)
{
	if (set != nil)
	{
		VTaskLock lock(&fServerKeptSelectionsMutex);
		mapOfSelections::iterator found = fServerKeptSelections.find(*(set->GetID()));

		if ( found != fServerKeptSelections.end())
		{
			if (willResend)
			{
				found->second.fSel->Release();
				fServerKeptSelections.erase(found);
			}
			else
			{
				found->second.fRetainedByServer = false;
				if (!found->second.fRetainedByClient)
				{
					found->second.fSel->Release();
					fServerKeptSelections.erase(found);
				}
			}
			set->GenerateID(true);
		}
	}
}


void VDBMgr::UnMarkSelectionOnServerAsPermanent(Selection* sel, bool willResend)
{
	if (sel != nil && sel->MustBeKeptOnServer())
	{
		VTaskLock lock(&fServerKeptSelectionsMutex);
		mapOfSelections::iterator found = fServerKeptSelections.find(*(sel->GetID()));

		if ( found != fServerKeptSelections.end())
		{
			if (willResend)
			{
				found->second.fSel->Release();
				fServerKeptSelections.erase(found);
			}
			else
			{
				found->second.fRetainedByServer = false;
				if (!found->second.fRetainedByClient)
				{
					found->second.fSel->Release();
					fServerKeptSelections.erase(found);
				}
			}
			sel->GenerateID(true);
		}
	}
}


void VDBMgr::AddReleasedSelID(const VUUIDBuffer& inID)
{
	VTaskLock lock(&fServerKeptSelectionsMutex);
	fClientReleasedSelIDs.push_back(inID);

}


void VDBMgr::StealListOfReleasedSelIDs(vector<VUUIDBuffer>& outList) // to be called on client
{
	VTaskLock lock(&fServerKeptSelectionsMutex);
	outList = fClientReleasedSelIDs;
	fClientReleasedSelIDs.clear();
}


void VDBMgr::ForgetServerKeptSelections(const vector<VUUIDBuffer>& inList) // to be called on server
{
	for (vector<VUUIDBuffer>::const_iterator cur = inList.begin(), end = inList.end(); cur != end; cur++)
	{
		ForgetServerKeptSelection(*cur);
	}
}


void VDBMgr::AddReleasedRecID(const DB4D_RecordRef& inID, BaseTaskInfo* context)
{
	if (context == nil)
	{
		VTaskLock lock(&fServerKeptSelectionsMutex);
		fClientReleasedRecIDs.push_back(inID);
	}
	else
	{
		context->AddReleasedRecID(inID);
	}
}


void VDBMgr::StealListOfReleasedRecIDs(vector<DB4D_RecordRef>& outList, BaseTaskInfo* context) // to be called on client
{
	if (context == nil)
	{
		VTaskLock lock(&fServerKeptSelectionsMutex);
		outList = fClientReleasedRecIDs;
		fClientReleasedRecIDs.clear();
	}
	else
		context->StealListOfReleasedRecIDs(outList);
}


void VDBMgr::ForgetServerKeptRecords(const vector<DB4D_RecordRef>& inList) // to be called on server
{
	VTaskLock lock(&fServerKeptSelectionsMutex);
	for (vector<DB4D_RecordRef>::const_iterator cur = inList.begin(), end = inList.end(); cur != end; cur++)
	{
		ForgetServerKeptRecord(*cur);
	}
}

void VDBMgr::ForgetServerKeptRecords(BaseTaskInfo* context)
{
	VTaskLock lock(&fServerKeptSelectionsMutex);
	vector<DB4D_RecordRef> records;
	for (mapOfRecords::iterator cur = fServerKeptRecords.begin(), end = fServerKeptRecords.end(); cur != end; cur++)
	{
		if (cur->second.fRec->GetContext() == context)
		{
			cur->second.fCount = 1;
			records.push_back(cur->first);
		}
	}
	ForgetServerKeptRecords(records);
}



void VDBMgr::ForgetServerKeptRecord(const DB4D_RecordRef& inID)
{
	VTaskLock lock(&fServerKeptSelectionsMutex);
	mapOfRecords::iterator found = fServerKeptRecords.find(inID);
	if (found != fServerKeptRecords.end())
	{
		KeptFicheInMem* xrec = &(found->second);
		xrec->fCount--;
		if (xrec->fCount == 0)
		{
			xrec->fRec->Release();
			fServerKeptRecords.erase(found);
		}
	}
}


void VDBMgr::KeepRecordOnServer(FicheInMem* rec, bool onlyonce)
{
	if (rec != nil && !rec->ReadOnlyState())
	{
		rec->DoNotClearIfReloadFromContext();
		VTaskLock lock(&fServerKeptSelectionsMutex);
		mapOfRecords::iterator found = fServerKeptRecords.find(rec->GetID());
		if (found == fServerKeptRecords.end())
		{
			KeptFicheInMem* xrec = &(fServerKeptRecords[rec->GetID()]);
			xrec->fRec = rec;
			xrec->fCount = 1;
			rec->Retain();
		}
		else
		{
			if (!onlyonce)
				found->second.fCount++;
		}
	}
}

FicheInMem* VDBMgr::RetainServerKeptRecord(const VUUIDBuffer& inID)
{
	FicheInMem* rec = nil;
	VTaskLock lock(&fServerKeptSelectionsMutex);
	mapOfRecords::iterator found = fServerKeptRecords.find(inID);
	if (found != fServerKeptRecords.end())
	{
		rec = found->second.fRec;
		rec->Retain();
	}

	return rec;
}


bool VDBMgr::HasSomeReleasedObjects(BaseTaskInfo* context)
{
	VTaskLock lock(&fServerKeptSelectionsMutex);
	if (context == nil)
	{
		return !fClientReleasedRecIDs.empty() || !fClientReleasedSelIDs.empty();
	}
	else
	{
		return context->HasSomeReleasedObjects() || !fClientReleasedSelIDs.empty() || !fMarkedRecordsAsPushed.empty();
	}

}


void VDBMgr::AddMarkRecordAsPushed(sLONG numtable, sLONG numrec)
{
	VTaskLock lock(&fServerKeptSelectionsMutex);
	fMarkedRecordsAsPushed.push_back(PushedRecID(true, numtable, numrec));
}


void VDBMgr::AddUnMarkRecordAsPushed(sLONG numtable, sLONG numrec)
{
	VTaskLock lock(&fServerKeptSelectionsMutex);
	fMarkedRecordsAsPushed.push_back(PushedRecID(false, numtable, numrec));
}

void VDBMgr::StealMarkedRecordsAsPushed(vectorPushedRecs& outRecs)
{
	VTaskLock lock(&fServerKeptSelectionsMutex);
	outRecs.clear();
	outRecs.swap(fMarkedRecordsAsPushed);
}


void VDBMgr::ExecuteRequest( sWORD inRequestID, IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	StErrorContextInstaller errs(false);
	assert(inRequestID >= kRangeReqDB4DWithBaseID && inRequestID < kMaxRangeReqDB4DWithBaseID);

	switch(inRequestID-kRangeReqDB4D)
	{
		case Req_OpenBase:
		case Req_CheckBaseForUpdate:
			ExecOpenBase(inRequest, inContext);
			break;

		case Req_SetBaseExtraProperties:
			ExecSetBaseExtraProperties( inRequest, inContext);
			break;

		case Req_SetStructExtraProperties:
			ExecSetStructureExtraProperties( inRequest, inContext);
			break;

		case Req_AddTable_With_Bag:
			ExecAddTableWithBag(inRequest, inContext);
			break;

		case Req_AddTable:
			ExecAddTable(inRequest, inContext);
			break;

		case Req_DropTable:
			ExecDropTable(inRequest, inContext);
			break;

		case Req_SetTableName:
			ExecSetTableName(inRequest, inContext);
			break;

		case Req_SetTableExtraProperties:
			ExecSetTableExtraProperties( inRequest, inContext);
			break;

		case Req_SetTableKeepStamp:
			ExecSetTableKeepStamp( inRequest, inContext);
			break;

		case Req_SetTableKeepRecordSyncInfo:
			ExecSetTableKeepRecordSyncInfo( inRequest, inContext);
			break;

		case Req_SetTableHideInRest:
			ExecSetTableHideInRest( inRequest, inContext);
			break;

		case Req_SetTablePrimaryKey:
			ExecSetTablePrimaryKey( inRequest, inContext);
			break;

		case Req_SelectAllRecords:
			ExecSelectAllRecords(inRequest, inContext);
			break;
		
		case Req_CountRecordsInTable:
			ExecCountRecordsInTable(inRequest, inContext);
			break;

		case Req_DeleteRecordsInSelection:
			ExecDeleteRecordsInSelection(inRequest, inContext);
			break;

		case Req_AskForASelectionPart:
			ExecAskForASelectionPart(inRequest, inContext);
			break;
		
		case Req_LoadRecord:
			ExecLoadRecord(inRequest, inContext);
			break;
			
		case Req_SetFieldName:
			ExecSetFieldName( inRequest, inContext);
			break;

		case Req_SetFieldAttributes:
			ExecSetFieldAttributes( inRequest, inContext);
			break;

		case Req_SetFieldTextSwitchSize:
			ExecSetFieldTextSwitchSize( inRequest, inContext);
			break;

		case Req_SetFieldNeverNull:
			ExecSetFieldNeverNull( inRequest, inContext);
			break;

		case Req_SetFieldStyledText:
			ExecSetFieldStyledText( inRequest, inContext);
			break;

		case Req_SetFieldHideInRest:
			ExecSetFieldHideInRest( inRequest, inContext);
			break;

		case Req_SetOutsideData:
			ExecSetFieldOutsideData( inRequest, inContext);
			break;

		case Req_SetFieldExtraProperties:
			ExecSetFieldExtraProperties( inRequest, inContext);
			break;

		case Req_AddFields_With_BagArray:
			ExecAddFieldsWithBagArray( inRequest, inContext);
			break;

		case Req_DropField:
			ExecDropField( inRequest, inContext);
			break;

		case Req_AddRelation:
			ExecAddRelation( inRequest, inContext);
			break;

		case Req_DropRelation:
			ExecDropRelation( inRequest, inContext);
			break;

		case Req_SetRelationAttributes:
			ExecSetRelationAttributes( inRequest, inContext);
			break;

		case Req_SetRelationExtraProperties:
			ExecSetRelationExtraProperties( inRequest, inContext);
			break;

		case Req_CreateIndexOnOneField:
			ExecCreateIndexOnOneField( inRequest, inContext);
			break;

		case Req_CreateIndexOnMultipleField:
			ExecCreateIndexOnMultipleField( inRequest, inContext);
			break;

		case Req_CreateFullTextIndexOnOneField:
			ExecCreateFullTextIndexOnOneField( inRequest, inContext);
			break;

		case Req_DropIndexOnOneField:
			ExecDropIndexOnOneField( inRequest, inContext);
			break;

		case Req_DropIndexOnMultipleField:
			ExecDropIndexOnMultipleField( inRequest, inContext);
			break;

		case Req_DropFullTextIndexOnOneField:
			ExecDropFullTextOnOneField( inRequest, inContext);
			break;

		case Req_SetIndexName:
			ExecSetIndexName( inRequest, inContext);
			break;

		case Req_DropIndexByRef:
			ExecDropIndexByRef( inRequest, inContext);
			break;
	
		case Req_RebuildIndexByRef:
			ExecRebuildIndexByRef( inRequest, inContext);
			break;

		case Req_LockDataBaseDef:
			ExecLockDataBaseDef( inRequest, inContext);
			break;

		case Req_LockTableDef:
			ExecLockTableDef( inRequest, inContext);
			break;

		case Req_LockFieldDef:
			ExecLockFieldDef( inRequest, inContext);
			break;

		case Req_LockRelationDef:
			ExecLockRelationDef( inRequest, inContext);
			break;
			
		case Req_LockIndexDef:
			ExecLockIndexDef( inRequest, inContext);
			break;

		case Req_UnlockObjectDef:
			ExecUnlockObjectDef( inRequest, inContext);
			break;

		case Req_SetContextExtraData:
			ExecSetContextExtraData( inRequest, inContext);
			break;

		case Req_TestServer:
			ExecTestServer(inRequest, inContext);
			break;
			
		case Req_SaveRecord:
			ExecSaveRecord(inRequest, inContext);
			break;

		case req_ConnectRecord:
			ExecConnectRecord(inRequest, inContext);
			break;

		case req_DeleteRecord:
			ExecDeleteRecord(inRequest, inContext);
			break;

		case Req_ExecuteQuery:
			ExecExecuteQuery(inRequest, inContext);
			break;

		case Req_SortSelection:
			ExecSortSelection(inRequest, inContext);
			break;

		case Req_SendLastRemoteInfo:
			ExecSendLastRemoteInfo(inRequest, inContext);
			break;

		case Req_DataToCollection:
			ExecDataToCollection(inRequest, inContext);
			break;

		case Req_CollectionToData:
			ExecCollectionToData(inRequest, inContext);
			break;

		case Req_GetDistinctValues:
			ExecGetDistinctValues(inRequest, inContext);
			break;

		case Req_ActivateManyToOne:
			ExecActivateManyToOne(inRequest, inContext);
			break;

		case Req_ActivateManyToOneS:
			ExecActivateManyToOneS(inRequest, inContext);
			break;

		case Req_ActivateOneToMany:
			ExecActivateOneToMany(inRequest, inContext);
			break;

		case Req_RelateManySelection:
			ExecRelateManySelection(inRequest, inContext);
			break;

		case Req_RelateOneSelection:
			ExecRelateOneSelection(inRequest, inContext);
			break;

		case Req_FindKey:
			ExecFindKey(inRequest, inContext);
			break;

		case Req_ReserveRecordNumber:
			ExecReserveRecordNumber(inRequest, inContext);
			break;

		case Req_ScanIndex:
			ExecScanIndex(inRequest, inContext);
			break;

		case Req_ActivateRelsOnAPath:
			ExecActivateRelsOnAPath(inRequest, inContext);
			break;

		case Req_CacheDisplaySelection:
			ExecCacheDisplaySelection(inRequest, inContext);
			break;

		case Req_SetFullyDeleteRecords:
			ExecSetFullyDeleteRecords(inRequest, inContext);
			break;

		case Req_SetTableSchema:
			ExecSetTableSchema( inRequest, inContext);
			break;

		case Req_TruncateTable:
			ExecTruncateTable(inRequest, inContext);
			break;

		case Req_ExecuteColumnFormulas:
			ExecExecuteColumnFormulas(inRequest, inContext);
			break;

		case Req_DelRecFromSel:
			ExecDelRecFromSel(inRequest, inContext);
			break;

		case Req_DelSetFromSel:
			ExecDelSetFromSel(inRequest, inContext);
			break;

		case Req_DelRangeFromSel:
			ExecDelRangeFromSel(inRequest, inContext);
			break;

		case Req_AddRecIDToSel:
			ExecAddRecIDToSel(inRequest, inContext);
			break;

		case Req_ReduceSel:
			ExecReduceSel(inRequest, inContext);
			break;

		case Req_ActivateAutomaticRelations_N_To_1:
			ExecActivateAutomaticRelations_N_To_1(inRequest, inContext);
			break;

		case Req_ActivateAutomaticRelations_1_To_N:
			ExecActivateAutomaticRelations_1_To_N(inRequest, inContext);
			break;

		case Req_ActivateAllAutomaticRelations:
			ExecActivateAllAutomaticRelations(inRequest, inContext);
			break;

		case Req_DeleteRecordByID:
			ExecDeleteRecordByID(inRequest, inContext);
			break;

		case Req_FillSetFromArrayOfBits:
			ExecFillSetFromArrayOfBits(inRequest, inContext);
			break;

		case Req_FillSetFromArrayOfLongs:
			ExecFillSetFromArrayOfLongs(inRequest, inContext);
			break;

		case Req_FillArrayOfBitsFromSet:
			ExecFillArrayOfBitsFromSet(inRequest, inContext);
			break;

		case Req_FillArrayOfLongsFromSet:
			ExecFillArrayOfLongsFromSet(inRequest, inContext);
			break;

		case Req_SyncThingsToForget:
			ExecSyncThingsToForget(inRequest, inContext);
			break;

		case Req_WhoLockedRecord:
			ExecWhoLockedRecord(inRequest, inContext);
			break;

		case Req_GetListOfDeadTables:
			ExecGetListOfDeadTables(inRequest, inContext);
			break;

		case Req_ResurectTable:
			ExecResurectTable(inRequest, inContext);
			break;

		case Req_GetTableFragmentation:
			ExecGetTableFragmentation(inRequest, inContext);
			break;


		default:
			assert(false);
	}
}


void VDBMgr::ExecuteRequest( sWORD inRequestID, IStreamRequestReply *inStreamRequest, CDB4DContext *inContext)
{
	assert(inRequestID >= kRangeReqDB4D && inRequestID < kMaxRangeReqDB4D);

	CDB4DBaseContext* context = nil;
	uLONG startTime;
	Boolean oklog = false;
	sLONG RequestNbBytes;
	if (fRequestLogger != nil && fRequestLogger->IsEnable())
	{
		oklog = true;
		startTime = VSystem::GetCurrentTime();
		RequestNbBytes = inStreamRequest->GetInputStream()->GetSize();
	}

	IRequestReply* inRequest = (IRequestReply*)inStreamRequest;

	if (inRequestID >= kRangeReqDB4DWithBaseID && inRequestID < kMaxRangeReqDB4DWithBaseID)
	{
		VUUID xid;
		VError err = xid.ReadFromStream(inRequest->GetInputStream());
		inRequest->GetInputStream()->SetPos(0);
		CDB4DBase* base = VDBMgr::RetainBaseByUUID(xid);
		if (base == nil)
		{
			base = base; // put a break here
		}
		else
			context = inContext->RetainDataBaseContext(base);

#if VERSIONDEBUG
		Base4D *thebase = (base == nil) ? nil : VImpCreator<VDB4DBase>::GetImpObject(base)->GetBase();
		sLONG old_refcount1 = (thebase == nil) ? 0 : thebase->GetRefCount();
		sLONG old_refcount2 = (context == nil) ? 0 : context->GetRefCount();
#endif

		ExecuteRequest(inRequestID, inRequest, context);

#if VERSIONDEBUG
		sLONG new_refcount1 = (thebase == nil) ? 0 : thebase->GetRefCount();
		sLONG new_refcount2 = (context == nil) ? 0 : context->GetRefCount();
		
		if ( old_refcount1 != new_refcount1)
		{
			int a = 1;
		}

		if ( old_refcount2 != new_refcount2)
		{
			int a = 1;
		}
#endif

		if (context != nil)
			context->Release();
		if (base != nil)
			base->Release();
	}
	else
	{
		StErrorContextInstaller errs(false);
		switch(inRequestID-kRangeReqDB4D)
		{
			case req_OpenOrCreateBase:
				ExecOpenOrCreateBase(inRequest, inContext);
				break;

			default:
				assert(false);
		}
	}

	if (fRequestLogger!= nil && oklog)
	{
		fRequestLogger->Log(CDB4DManager::Component_Type, context, inRequestID, RequestNbBytes, inStreamRequest->GetOutputStream()->GetSize(), VSystem::GetCurrentTime()-startTime);
	}
}


void VDBMgr::ExecAddTableWithBag(IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	VStream* reqget = inRequest->GetInputStream();
	VUUID xid;
	err = xid.ReadFromStream(reqget);
	Boolean alreadysend = false;
	if (err == VE_OK)
	{
		Base4D* base = RetainBase4DByUUID(xid);
		if (base != nil)
		{
			VValueBag bag;
			Boolean generateName = false;

			err = bag.ReadFromStream(reqget);
			if (err == VE_OK)
				generateName = inRequest->GetBooleanParam( err);
			if (err == VE_OK)
			{
				ObjLocker locker( inContext, base);
				if (locker.CanWork())
				{
					Table *tt = nil;
					VBagLoader loader( true, true);
					err = base->CreateTable(bag, &loader, &tt, inContext, generateName);
					if (err == VE_OK)
					{
						inRequest->InitReply(0);
						VStream* reqsend = inRequest->GetOutputStream();
						err = tt->SendToClient(reqsend);
						tt->Release();
						alreadysend = true;
					}
				}
				else
					err = base->ThrowError( VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT, DBaction_CreatingTable);
			}
			base->Release();
		}
		else
			err = ThrowBaseError( VE_DB4D_INVALID_BASEID, DBaction_CreatingTable);
	}

	if (!alreadysend)
		SendError(inRequest, inContext, err);

}


void VDBMgr::ExecAddTable(IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	VStream* reqget = inRequest->GetInputStream();
	VUUID xid;
	err = xid.ReadFromStream(reqget);
	Boolean alreadysend = false;
	if (err == VE_OK)
	{
		Base4D* base = RetainBase4DByUUID(xid);
		if (base != nil)
		{
			VString tablename;
			err = tablename.ReadFromStream(reqget);
			if (err == VE_OK)
			{
				sLONG nbcrit;
				err = reqget->GetLong(nbcrit);
				if (nbcrit >= 0 && nbcrit < kMaxFields)
				{
					void* block = GetFastMem(sizeof(CritereDISK)*nbcrit+2, false, 'tmp3');
					if (block == nil)
						err = base->ThrowError(memfull, DBaction_CreatingTable);
					else
					{
						err = reqget->GetData(block, sizeof(CritereDISK)*nbcrit);
						if (err == VE_OK)
						{
							ObjLocker locker( inContext, base);
							if (locker.CanWork())
							{
								if (reqget->NeedSwap())
								{
									CritereDISK* p = (CritereDISK*)block;
									CritereDISK* end = p + nbcrit;
									for (; p != end; p++)
									{
										p->SwapBytes();
									}
								}

								sLONG numtable = base->CreTable(tablename, (CritereDISK*)block, nbcrit, err, inContext);
								if (err == VE_OK)
								{
									Table* tt = base->RetainTable(numtable);
									if (tt != nil)
									{
										inRequest->InitReply(0);
										VStream* reqsend = inRequest->GetOutputStream();
										err = tt->SendToClient(reqsend);
										tt->Release();
										alreadysend = true;
									}
									else
										err = -1;
								}
							}
							else
								err = base->ThrowError( VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT, DBaction_CreatingTable);
						}
						FreeFastMem(block);
					}
				}
				else
					err = base->ThrowError(VE_DB4D_INVALID_FIELDNUM, DBaction_CreatingTable);
			}
			base->Release();
		}
		else
			err = ThrowBaseError(VE_DB4D_INVALID_BASEID, DBaction_CreatingTable);
	}

	if (!alreadysend)
		SendError(inRequest, inContext, err);
}



void VDBMgr::ExecOpenBase(IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	VStream* reqget = inRequest->GetInputStream();
	VUUID xid;
	Boolean alreadysend = false;
	err = xid.ReadFromStream(reqget);
	if (err == VE_OK)
	{
		Base4D* base = RetainBase4DByUUID(xid);
		if (base != nil)
		{
			err = base->SendToClient(inRequest, inContext);
			alreadysend = true;
			base->Release();
		}
		else
			err = ThrowBaseError(VE_DB4D_INVALID_BASEID, DBaction_OpeningBase);
	}

	if (!alreadysend)
		SendError(inRequest, inContext, err);
}


void VDBMgr::ExecOpenOrCreateBase(IRequestReply *inRequest, CDB4DContext *inContext)
{
	VError err = VE_OK;
	VStream* reqget = inRequest->GetInputStream();
	VString basename;
	VString fullpath;
	sBYTE b;
	Boolean createifnotexist;
	err = basename.ReadFromStream(reqget);
	if (err == VE_OK)
		err = fullpath.ReadFromStream(reqget);
	if (err == VE_OK)
		b = reqget->GetByte();
	createifnotexist = b != 0;
	if (err == VE_OK)
	{
		VString name = basename;
		RemoveExtension(name);
		Base4D* bd = RetainBase4DByName(name);
		if (bd == nil)
		{
			CDB4DBase* cdb;
			VFolder* folder = nil;
			if (!fullpath.IsEmpty())
				folder = new VFolder(fullpath);

			VFile* path;
			if (folder == nil)
				path = new VFile(basename);
			else
				path = new VFile(*folder, basename);

			if (path->Exists())
			{
				cdb = OpenBase(*path, DB4D_Open_DefaultData + DB4D_Open_WithSeparateIndexSegment);
			}
			else
			{
				cdb = CreateBase(*path, DB4D_Open_DefaultData + DB4D_Open_WithSeparateIndexSegment, VIntlMgr::GetDefaultMgr());
			}
			if (cdb != nil)
			{
				bd = VImpCreator<VDB4DBase>::GetImpObject(cdb)->GetBase();
				cdb->Release();
			}

			path->Release();
			if (folder != nil)
				folder->Release();
		}
		if (bd != nil)
		{
			inRequest->InitReply(0);
			VStream* reqsend = inRequest->GetOutputStream();
			bd->GetUUID().WriteToStream(reqsend);
			bd->Release();
		}
	}

	if (err != VE_OK)
	{
		inRequest->InitReply(-2);
		VStream* reqsend = inRequest->GetOutputStream();
		reqsend->PutLong8(err);
	}
}


void VDBMgr::ExecSetBaseExtraProperties(IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	
	Base4D *base = inRequest->RetainBaseParam( err);
	if (err == VE_OK && base != NULL)
	{
		VValueBag *extraBag = inRequest->RetainValueBagParam( err);
		if (err == VE_OK && extraBag != NULL)
		{
			ObjLocker locker( inContext, base);
			if (locker.CanWork())
			{
				err = base->SetExtraProperties( extraBag, false, inContext);
				if (err == VE_OK)
				{
					inRequest->InitReply(0);
					alreadysend = true;
				}
			}
			else
				err = base->ThrowError( VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT, DBaction_ModifyingExtraProperty);
		}
		ReleaseRefCountable( &extraBag);
	}
	ReleaseRefCountable( &base);

	if (!alreadysend)
		SendError( inRequest, inContext, err);
}



void VDBMgr::ExecSetStructureExtraProperties(IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;

	Base4D *base = inRequest->RetainBaseParam( err);
	if (err == VE_OK && base != NULL)
	{
		VValueBag *extraBag = inRequest->RetainValueBagParam( err);
		if (err == VE_OK && extraBag != NULL)
		{
			ObjLocker locker( inContext, base);
			if (locker.CanWork())
			{
				err = base->SetStructureExtraProperties( extraBag, false, inContext);
				if (err == VE_OK)
				{
					inRequest->InitReply(0);
					alreadysend = true;
				}
			}
			else
				err = base->ThrowError( VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT, DBaction_ModifyingExtraProperty);
		}
		ReleaseRefCountable( &extraBag);
	}
	ReleaseRefCountable( &base);

	if (!alreadysend)
		SendError( inRequest, inContext, err);
}


void VDBMgr::ExecDropTable(IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	VStream* reqget = inRequest->GetInputStream();
	VUUID xid;
	err = xid.ReadFromStream(reqget);
	Boolean alreadysend = false;
	if (err == VE_OK)
	{
		Base4D* base = RetainBase4DByUUID(xid);
		if (base != nil)
		{
			sLONG numtable;
			err = reqget->GetLong(numtable);
			if (err == VE_OK)
			{
				ObjLocker locker( inContext, base);
				if (locker.CanWork())
				{
					Table* tt = base->RetainTable(numtable);
					if (tt != nil)
					{
						VDB4DProgressIndicator* progress = inRequest->RetainProgressParam(inContext);
						ObjLocker locker2( inContext, tt, false, &locker);
						if (locker2.CanWork())
						{
							sBYTE retour = 0;
							Boolean ok = base->DeleteTable(tt, inContext, progress);
							if (ok)
								retour = 1;
							inRequest->InitReply(0);
							inRequest->GetOutputStream()->PutByte(retour);
							alreadysend = true;
						}
						else
							err = base->ThrowError( VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT, DBaction_DeletingTable);
						QuickReleaseRefCountable(progress);
							
						tt->Release();
					}
					else
						err = base->ThrowError( VE_DB4D_INVALID_TABLENUM, DBaction_DeletingTable);
				}
				else
					err = base->ThrowError( VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT, DBaction_DeletingTable);

			}
			base->Release();
		}
		else
			err = ThrowBaseError(VE_DB4D_INVALID_BASEID, DBaction_DeletingTable);
	}

	if (!alreadysend)
		SendError(inRequest, inContext, err);
}


void VDBMgr::ExecSetTableName(IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	VStream* reqget = inRequest->GetInputStream();
	VUUID xid;
	err = xid.ReadFromStream(reqget);
	Boolean alreadysend = false;
	if (err == VE_OK)
	{
		Base4D* base = RetainBase4DByUUID(xid);
		if (base != nil)
		{
			sLONG numtable;
			err = reqget->GetLong(numtable);
			Table* tt = base->RetainTable(numtable);
			Boolean ok = false;
			if (tt != nil)
			{
				VString tablename;
				err = tablename.ReadFromStream(reqget);
				if (err == VE_OK)
				{
					sWORD withNameCheck = 0;
					err = reqget->GetWord( withNameCheck);		// sc 11/04/2007 read check name mode
					if (err == VE_OK)
					{
						ObjLocker locker( inContext, tt);
						if (locker.CanWork())
							err = tt->SetName( tablename, inContext, true, withNameCheck!=0);		// sc 24/01/2008 ACI0055908
						else
							err = base->ThrowError( VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT, DBaction_ChangingTableName);
					}
				}
				tt->Release();
			}
			else
				err = base->ThrowError(VE_DB4D_INVALID_TABLENUM, DBaction_ChangingTableName);

			if (err == VE_OK)
			{
				sBYTE retour = 1;
				inRequest->InitReply(0);
				inRequest->GetOutputStream()->PutByte(retour);
				alreadysend = true;
			}

			base->Release();
		}
		else
			err = ThrowBaseError(VE_DB4D_INVALID_BASEID, DBaction_ChangingTableName);
	}

	if (!alreadysend)
		SendError(inRequest, inContext, err);
}


void VDBMgr::ExecSetTableExtraProperties( IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	
	Base4D *base = inRequest->RetainBaseParam( err);
	if (err == VE_OK && base != NULL)
	{
		Table *table = inRequest->RetainTableParam( base, err);
		if (err == VE_OK && table != NULL)
		{
			VValueBag *extraBag = inRequest->RetainValueBagParam( err);
			if (err == VE_OK && extraBag != NULL)
			{
				ObjLocker locker( inContext, table);
				if (locker.CanWork())
				{
					err = table->SetExtraProperties( extraBag, false, inContext);
					if (err == VE_OK)
					{
						inRequest->InitReply(0);
						alreadysend = true;
					}					
				}
				else
					err = base->ThrowError( VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT, DBaction_ModifyingExtraProperty);
			}
			ReleaseRefCountable( &extraBag);
		}
		ReleaseRefCountable( &table);
	}
	ReleaseRefCountable( &base);

	if (!alreadysend)
		SendError( inRequest, inContext, err);
}


void VDBMgr::ExecSetTableKeepStamp( IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	Base4D *base = inRequest->RetainBaseParam( err);
	if (err == VE_OK && base != nil)
	{
		Table *tt = inRequest->RetainTableParam( base, err);
		if (err == VE_OK && tt != nil)
		{
			Boolean keepStamp = inRequest->GetBooleanParam( err);
			if (err == VE_OK)
			{
				VDB4DProgressIndicator *progress = inRequest->RetainProgressParam( inContext);
				ObjLocker locker( inContext, tt);
				if (locker.CanWork())
				{
					err = tt->SetKeepStamp( inContext, keepStamp, progress);
				}
				else
				{
					err = ThrowBaseError( VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT, DBaction_ChangingTableProperties);
				}

				if (err == VE_OK)
				{
					alreadysend = true;
					inRequest->InitReply(0);
				}
				QuickReleaseRefCountable( progress);
			}
		}
		QuickReleaseRefCountable( tt);
	}
	QuickReleaseRefCountable( base);

	if (!alreadysend)
		SendError( inRequest, inContext, err);
}


void VDBMgr::ExecSetTableKeepRecordSyncInfo( IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	Base4D *base = inRequest->RetainBaseParam( err);
	if (err == VE_OK && base != nil)
	{
		Table *tt = inRequest->RetainTableParam( base, err);
		if (err == VE_OK && tt != nil)
		{
			Boolean keepStamp = inRequest->GetBooleanParam( err);
			if (err == VE_OK)
			{
				VDB4DProgressIndicator *progress = inRequest->RetainProgressParam( inContext);
				ObjLocker locker( inContext, tt);
				if (locker.CanWork())
				{
					err = tt->SetKeepRecordSyncInfo( inContext, keepStamp, progress);
				}
				else
				{
					err = ThrowBaseError( VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT, DBaction_ChangingTableProperties);
				}

				if (err == VE_OK)
				{
					alreadysend = true;
					inRequest->InitReply(0);
				}
				QuickReleaseRefCountable( progress);
			}
		}
		QuickReleaseRefCountable( tt);
	}
	QuickReleaseRefCountable( base);

	if (!alreadysend)
		SendError( inRequest, inContext, err);
}



void VDBMgr::ExecSetTableHideInRest( IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	Base4D *base = inRequest->RetainBaseParam( err);
	if (err == VE_OK && base != nil)
	{
		Table *tt = inRequest->RetainTableParam( base, err);
		if (err == VE_OK && tt != nil)
		{
			Boolean keepStamp = inRequest->GetBooleanParam( err);
			if (err == VE_OK)
			{
				VDB4DProgressIndicator *progress = inRequest->RetainProgressParam( inContext);
				ObjLocker locker( inContext, tt);
				if (locker.CanWork())
				{
					err = tt->SetHideInRest( inContext, keepStamp, progress);
				}
				else
				{
					err = ThrowBaseError( VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT, DBaction_ChangingTableProperties);
				}

				if (err == VE_OK)
				{
					alreadysend = true;
					inRequest->InitReply(0);
				}
				QuickReleaseRefCountable( progress);
			}
		}
		QuickReleaseRefCountable( tt);
	}
	QuickReleaseRefCountable( base);

	if (!alreadysend)
		SendError( inRequest, inContext, err);
}


void VDBMgr::ExecSetTablePrimaryKey( IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	Base4D *base = inRequest->RetainBaseParam( err);
	if (err == VE_OK && base != nil)
	{
		Table *tt = inRequest->RetainTableParam( base, err);
		if (err == VE_OK && tt != nil)
		{
			NumFieldArray numField;
			VString name;
			Boolean canReplaceExistingOne = false;
			VDB4DProgressIndicator *progress = NULL;
			
			err = inRequest->GetNumFieldArrayParam( numField);
			if (err == VE_OK)
				err = inRequest->GetStringParam( name);
			if (err == VE_OK)
				canReplaceExistingOne = inRequest->GetBooleanParam( err);
			if (err == VE_OK)
				progress = inRequest->RetainProgressParam( inContext);
			if (err == VE_OK)
			{
				ObjLocker locker( inContext, tt);
				if (locker.CanWork())
				{
					err = tt->SetPrimaryKey(numField, progress, canReplaceExistingOne, inContext, &name);
					if (err == VE_OK)
					{
						alreadysend = true;
						inRequest->InitReply(0);
						tt->CopyPrimaryKey( numField);
						inRequest->PutNumFieldArrayReply( numField);
						inRequest->PutStringReply( tt->GetPrimaryKeyName());
					}
				}
				else
				{
					err = ThrowBaseError( VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT, DBaction_ChangingTableProperties);
				}
				QuickReleaseRefCountable( progress);
			}
		}
		QuickReleaseRefCountable( tt);
	}
	QuickReleaseRefCountable( base);
	
	if (!alreadysend)
		SendError( inRequest, inContext, err);
}


void VDBMgr::ExecSetFieldName( IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	
	Base4D *base = inRequest->RetainBaseParam( err);
	if (err == VE_OK && base != NULL)
	{
		Table *table = inRequest->RetainTableParam( base, err);
		if (err == VE_OK && table != NULL)
		{
			Field *field = inRequest->RetainFieldParam( table, err);
			if (err == VE_OK && field != NULL)
			{
				VString name;
				err = inRequest->GetStringParam( name);
				if (err == VE_OK)
				{
					ObjLocker locker( inContext, field);
					if (locker.CanWork())
					{
						err = field->SetName( name, inContext);
						if (err == VE_OK)
						{
							inRequest->InitReply(0);
							alreadysend = true;
						}
					}
					else
						err = base->ThrowError( VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT, DBaction_ChangingFieldName);
				}
			}
			ReleaseRefCountable( &field);
		}
		ReleaseRefCountable( &table);
	}
	ReleaseRefCountable( &base);

	if (!alreadysend)
		SendError( inRequest, inContext, err);
}


void VDBMgr::ExecSetFieldAttributes( IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	
	Base4D *base = inRequest->RetainBaseParam( err);
	if (err == VE_OK && base != NULL)
	{
		Table *table = inRequest->RetainTableParam( base, err);
		if (err == VE_OK && table != NULL)
		{
			Field *field = inRequest->RetainFieldParam( table, err);
			if (err == VE_OK && field != NULL)
			{
				VValueBag *attributesBag = inRequest->RetainValueBagParam( err);
				if (err == VE_OK && attributesBag != NULL)
				{
					VDB4DProgressIndicator* progress = inRequest->RetainProgressParam(inContext);
					ObjLocker locker( inContext, field);
					if (locker.CanWork())
					{
						err = table->SetCrit( field->GetPosInRec(), *attributesBag, inContext, progress);
						if (err == VE_OK)
						{
							field->occupe();
							CritereDISK crd = *(field->getCRD());
							const VValueBag *extraBag = field->RetainExtraProperties( err, inContext);
							if (err != VE_OK)
								ReleaseRefCountable( &extraBag);
							if (extraBag == NULL)
								extraBag = new VValueBag();
							field->libere();

							inRequest->InitReply(0);
							inRequest->GetOutputStream()->PutData( &crd, sizeof(crd));
							inRequest->PutValueBagReply( *extraBag);
							ReleaseRefCountable( &extraBag);
							alreadysend = true;
						}					
					}
					else
						err = base->ThrowError( VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT, DBaction_ChangingFieldProperties);
					QuickReleaseRefCountable(progress);

				}
				ReleaseRefCountable( &attributesBag);
			}
			ReleaseRefCountable( &field);
		}
		ReleaseRefCountable( &table);
	}
	ReleaseRefCountable( &base);

	if (!alreadysend)
		SendError( inRequest, inContext, err);
}


void VDBMgr::ExecSetFieldTextSwitchSize( IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	
	Base4D *base = inRequest->RetainBaseParam( err);
	if (err == VE_OK && base != NULL)
	{
		Table *table = inRequest->RetainTableParam( base, err);
		if (err == VE_OK && table != NULL)
		{
			Field *field = inRequest->RetainFieldParam( table, err);
			if (err == VE_OK && field != NULL)
			{
				sLONG size = inRequest->GetLongParam( err);
				if (err == VE_OK)
				{
					ObjLocker locker( inContext, field);
					if (locker.CanWork())
					{
						err = field->SetTextSwitchSize( size, inContext);
						if (err == VE_OK)
						{
							inRequest->InitReply(0);
							alreadysend = true;
						}					
					}
					else
						err = base->ThrowError( VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT, DBaction_ChangingFieldProperties);
				}
			}
			ReleaseRefCountable( &field);
		}
		ReleaseRefCountable( &table);
	}
	ReleaseRefCountable( &base);

	if (!alreadysend)
		SendError( inRequest, inContext, err);
}


void VDBMgr::ExecSetFieldNeverNull( IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	
	Base4D *base = inRequest->RetainBaseParam( err);
	if (err == VE_OK && base != NULL)
	{
		Table *table = inRequest->RetainTableParam( base, err);
		if (err == VE_OK && table != NULL)
		{
			Field *field = inRequest->RetainFieldParam( table, err);
			if (err == VE_OK && field != NULL)
			{
				Boolean neverNull = inRequest->GetBooleanParam( err);
				if (err == VE_OK)
				{
					ObjLocker locker( inContext, field);
					if (locker.CanWork())
					{
						err = field->SetNeverNull( neverNull, inContext);
						if (err == VE_OK)
						{
							inRequest->InitReply(0);
							alreadysend = true;
						}					
					}
					else
						err = base->ThrowError( VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT, DBaction_ChangingFieldProperties);
				}
			}
			ReleaseRefCountable( &field);
		}
		ReleaseRefCountable( &table);
	}
	ReleaseRefCountable( &base);

	if (!alreadysend)
		SendError( inRequest, inContext, err);
}


void VDBMgr::ExecSetFieldStyledText( IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	
	Base4D *base = inRequest->RetainBaseParam( err);
	if (err == VE_OK && base != NULL)
	{
		Table *table = inRequest->RetainTableParam( base, err);
		if (err == VE_OK && table != NULL)
		{
			Field *field = inRequest->RetainFieldParam( table, err);
			if (err == VE_OK && field != NULL)
			{
				Boolean styledText = inRequest->GetBooleanParam( err);
				if (err == VE_OK)
				{
					VDB4DProgressIndicator *progress = inRequest->RetainProgressParam(inContext);

					ObjLocker locker( inContext, field);
					if (locker.CanWork())
					{
						field->SetStyledText( styledText, inContext, progress);
						inRequest->InitReply(0);
						alreadysend = true;
					}
					else
					{
						err = base->ThrowError( VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT, DBaction_ChangingFieldProperties);
					}

					ReleaseRefCountable( &progress);
				}
			}
			ReleaseRefCountable( &field);
		}
		ReleaseRefCountable( &table);
	}
	ReleaseRefCountable( &base);

	if (!alreadysend)
		SendError( inRequest, inContext, err);
}


void VDBMgr::ExecSetFieldHideInRest( IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;

	Base4D *base = inRequest->RetainBaseParam( err);
	if (err == VE_OK && base != NULL)
	{
		Table *table = inRequest->RetainTableParam( base, err);
		if (err == VE_OK && table != NULL)
		{
			Field *field = inRequest->RetainFieldParam( table, err);
			if (err == VE_OK && field != NULL)
			{
				Boolean hidden = inRequest->GetBooleanParam( err);
				if (err == VE_OK)
				{
					VDB4DProgressIndicator *progress = inRequest->RetainProgressParam(inContext);

					ObjLocker locker( inContext, field);
					if (locker.CanWork())
					{
						field->SetHideInRest( hidden, inContext, progress);
						inRequest->InitReply(0);
						alreadysend = true;
					}
					else
					{
						err = base->ThrowError( VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT, DBaction_ChangingFieldProperties);
					}

					ReleaseRefCountable( &progress);
				}
			}
			ReleaseRefCountable( &field);
		}
		ReleaseRefCountable( &table);
	}
	ReleaseRefCountable( &base);

	if (!alreadysend)
		SendError( inRequest, inContext, err);
}


void VDBMgr::ExecSetFieldOutsideData( IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	
	Base4D *base = inRequest->RetainBaseParam( err);
	if (err == VE_OK && base != NULL)
	{
		Table *table = inRequest->RetainTableParam( base, err);
		if (err == VE_OK && table != NULL)
		{
			Field *field = inRequest->RetainFieldParam( table, err);
			if (err == VE_OK && field != NULL)
			{
				Boolean outsideData = inRequest->GetBooleanParam( err);
				if (err == VE_OK)
				{
					ObjLocker locker( inContext, field);
					if (locker.CanWork())
					{
						field->SetOutsideData( outsideData, inContext);
						inRequest->InitReply(0);
						alreadysend = true;
					}
					else
					{
						err = base->ThrowError( VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT, DBaction_ChangingFieldProperties);
					}
				}
			}
			ReleaseRefCountable( &field);
		}
		ReleaseRefCountable( &table);
	}
	ReleaseRefCountable( &base);

	if (!alreadysend)
		SendError( inRequest, inContext, err);
}


void VDBMgr::ExecSetFieldExtraProperties( IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	
	Base4D *base = inRequest->RetainBaseParam( err);
	if (err == VE_OK && base != NULL)
	{
		Table *table = inRequest->RetainTableParam( base, err);
		if (err == VE_OK && table != NULL)
		{
			Field *field = inRequest->RetainFieldParam( table, err);
			if (err == VE_OK && field != NULL)
			{
				VValueBag *extraBag = inRequest->RetainValueBagParam( err);
				if (err == VE_OK && extraBag != NULL)
				{
					ObjLocker locker( inContext, field);
					if (locker.CanWork())
					{
						err = field->SetExtraProperties( extraBag, false, inContext);
						if (err == VE_OK)
						{
							inRequest->InitReply(0);
							alreadysend = true;
						}					
					}
					else
						err = base->ThrowError( VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT, DBaction_ModifyingExtraProperty);
				}
				ReleaseRefCountable( &extraBag);
			}
			ReleaseRefCountable( &field);
		}
		ReleaseRefCountable( &table);
	}
	ReleaseRefCountable( &base);

	if (!alreadysend)
		SendError( inRequest, inContext, err);
}


void VDBMgr::ExecAddFieldsWithBagArray( IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	
	Base4D *base = inRequest->RetainBaseParam( err);
	if (err == VE_OK && base != NULL)
	{
		Table *table = inRequest->RetainTableParam( base, err);
		if (err == VE_OK && table != NULL)
		{
			VBagArray *fieldsBag = inRequest->RetainValueBagArrayParam( err);
			if (err == VE_OK && fieldsBag != NULL)
			{
				VDB4DProgressIndicator* progress = inRequest->RetainProgressParam(inContext);
				ObjLocker locker( inContext, table);
				if (locker.CanWork())
				{
					VBagLoader loader( true, true);
					sLONG firstAddedFieldNo = -1;
					err = table->CreateFields( fieldsBag, &firstAddedFieldNo, &loader, inContext, progress);
					if (err == VE_OK)
					{
						inRequest->InitReply(0);		// sc 13/12/2007
						inRequest->PutLongReply( firstAddedFieldNo);
						table->SendToClient( inRequest->GetOutputStream());
						alreadysend = true;
					}
				}
				else
					err = base->ThrowError( VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT, DBaction_AddingField);
				QuickReleaseRefCountable(progress);

			}
			ReleaseRefCountable( &fieldsBag);
		}
		ReleaseRefCountable( &table);
	}
	ReleaseRefCountable( &base);

	if (!alreadysend)
		SendError( inRequest, inContext, err);
}


void VDBMgr::ExecDropField( IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;

	Base4D *base = inRequest->RetainBaseParam( err);
	if (err == VE_OK && base != NULL)
	{
		Table *table = inRequest->RetainTableParam( base, err);
		if (err == VE_OK && table != NULL)
		{
			ObjLocker locker( inContext, table);
			if (locker.CanWork())
			{
				Field *field = inRequest->RetainFieldParam( table, err);
				if (err == VE_OK && field != NULL)
				{
					VDB4DProgressIndicator* progress = inRequest->RetainProgressParam(inContext);
					ObjLocker locker2( inContext, field, &locker);
					if (locker2.CanWork())
					{
						inRequest->InitReply(0);
						Boolean ok = table->DeleteField( field, inContext, progress);
						if (ok)
						{
							table->SendToClient( inRequest->GetOutputStream());
						}
						else
						{
							inRequest->PutLongReply( 0);
						}
						alreadysend = true;
					}
					else
						err = base->ThrowError( VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT, DBaction_DeletingField);
					QuickReleaseRefCountable(progress);
				}
				ReleaseRefCountable( &field);
			}
			else
				err = base->ThrowError( VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT, DBaction_DeletingField);
		}
		ReleaseRefCountable( &table);
	}
	ReleaseRefCountable( &base);

	if (!alreadysend)
		SendError( inRequest, inContext, err);
}


void VDBMgr::ExecAddRelation( IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;

	Base4D *base = inRequest->RetainBaseParam( err);
	if (err == VE_OK && base != NULL)
	{
		VString name, oppositeName;
		Table *sourceTable = NULL, *destTable = NULL;
		Field *sourceField = NULL, *destField = NULL;
		
		err = inRequest->GetStringParam( name);
		if (err == VE_OK)
			err = inRequest->GetStringParam( oppositeName);
		if (err == VE_OK)
			sourceTable = inRequest->RetainTableParam( base, err);
		if (err == VE_OK && sourceTable != NULL)
			sourceField = inRequest->RetainFieldParam( sourceTable, err);
		if (err == VE_OK)
			destTable = inRequest->RetainTableParam( base, err);
		if (err == VE_OK && destTable != NULL)
			destField = inRequest->RetainFieldParam( destTable, err);
		if (err == VE_OK)
		{
			ObjLocker locker( inContext, base);
			if (locker.CanWork())
			{
				Relation *rel = base->CreateRelation( name, oppositeName, sourceField, destField, err, inContext);
				if (err == VE_OK)
				{
					inRequest->InitReply(0);
					if (rel != NULL)
					{
						inRequest->PutByteReply( 1);
						inRequest->PutLongReply( rel->GetPosInList());
						rel->SendToClient( inRequest->GetOutputStream());
						rel->Release();
					}
					else
					{
						inRequest->PutByteReply( -1);
					}
					alreadysend = true;
				}
			}
			else
				err = base->ThrowError( VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT, DBaction_CreatingRelation);

		}
		ReleaseRefCountable( &sourceTable);
		ReleaseRefCountable( &sourceField);
		ReleaseRefCountable( &destTable);
		ReleaseRefCountable( &destField);
	}
	ReleaseRefCountable( &base);

	if (!alreadysend)
		SendError( inRequest, inContext, err);
}


void VDBMgr::ExecDropRelation( IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;

	Base4D *base = inRequest->RetainBaseParam( err);
	if (err == VE_OK && base != NULL)
	{
		Relation *rel = inRequest->RetainRelationParam( base, err);
		if (err == VE_OK && rel != NULL)
		{
			ObjLocker locker( inContext, base);
			if (locker.CanWork())
			{
				ObjLocker locker2( inContext, rel, false, &locker);
				if (locker2.CanWork())
				{
					err = base->DeleteRelation( rel, inContext);
					if (err == VE_OK)
					{
						inRequest->InitReply(0);
						alreadysend = true;
					}
				}
				else
					err = base->ThrowError( VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT, DBaction_DeletingRelation);
			}
			else
				err = base->ThrowError( VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT, DBaction_DeletingRelation);
		}
		ReleaseRefCountable( &rel);
	}
	ReleaseRefCountable( &base);

	if (!alreadysend)
		SendError( inRequest, inContext, err);
}


void VDBMgr::ExecSetRelationAttributes( IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	Relation::ExecReqSetAttributes( inRequest, inContext);
}


void VDBMgr::ExecSetRelationExtraProperties( IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	Relation::ExecReqSetExtraProperties( inRequest, inContext);
}


void VDBMgr::ExecCreateIndexOnOneField( IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;

	Base4D *base = inRequest->RetainBaseParam( err);
	if (err == VE_OK && base != NULL)
	{
		sLONG numTable=0, numField=0, typIndex=DB4D_Index_NoStorage;
		Boolean uniqueKeys=false, forceCreate=false, synchronous=false;
		VString *name = NULL;

		numTable = inRequest->GetLongParam( err);
		if (err == VE_OK)
			numField = inRequest->GetLongParam( err);
		if (err == VE_OK)
			typIndex = inRequest->GetLongParam( err);
		if (err == VE_OK)
			uniqueKeys = inRequest->GetBooleanParam( err);
		if (err == VE_OK)
			name = inRequest->GetStringParam( err);
		if (err == VE_OK)
			forceCreate = inRequest->GetBooleanParam( err);
		if (err == VE_OK)
			synchronous = inRequest->GetBooleanParam( err);

		if (err == VE_OK)
		{
			VDB4DProgressIndicator* progress = inRequest->RetainProgressParam(inContext);
			// ObjLocker locker( inContext, base);
			// if (locker.CanWork())
			{
				VSyncEvent *event = synchronous ? new VSyncEvent : nil;
				err = base->CreIndexOnField( numTable, numField, typIndex, uniqueKeys, inContext, progress, name, NULL, forceCreate, event);
				if (event != nil)
				{
					event->Lock();
					event->Release();
				}
				if (err == VE_OK)
				{
					inRequest->InitReply(0);
					alreadysend	= true;
				}
			}
			// else
			// 	err = base->ThrowError( VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT, DBaction_CreatingIndex);
			QuickReleaseRefCountable(progress);
		}
		delete name;
	}
	ReleaseRefCountable( &base);

	if (!alreadysend)
		SendError( inRequest, inContext, err);
}


void VDBMgr::ExecCreateIndexOnMultipleField( IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;

	Base4D *base = inRequest->RetainBaseParam( err);
	if (err == VE_OK && base != NULL)
	{
		FieldNuplet *fields = FieldNuplet::CreateFrom( *inRequest->GetInputStream(), base, err);
		if (fields != NULL && err == VE_OK)
		{
			sLONG typIndex=DB4D_Index_NoStorage;
			Boolean uniqueKeys=false, forceCreate=false, synchronous=false;
			VString *name = NULL;

			if (err == VE_OK)
				typIndex = inRequest->GetLongParam( err);
			if (err == VE_OK)
				uniqueKeys = inRequest->GetBooleanParam( err);
			if (err == VE_OK)
				name = inRequest->GetStringParam( err);
			if (err == VE_OK)
				forceCreate = inRequest->GetBooleanParam( err);
			if (err == VE_OK)
				synchronous = inRequest->GetBooleanParam( err);

			if (err == VE_OK)
			{
				VDB4DProgressIndicator* progress = inRequest->RetainProgressParam(inContext);
				// ObjLocker locker( inContext, base);
				// if (locker.CanWork())
				{
					VSyncEvent *event = synchronous ? new VSyncEvent : nil;
					err = base->CreIndexOnFields( fields, typIndex, uniqueKeys, inContext, progress, name,	NULL, forceCreate, event);
					if (event != nil)
					{
						event->Lock();
						event->Release();
					}
					if (err == VE_OK)
					{
						inRequest->InitReply(0);
						alreadysend	= true;
					}
				}
				// else
				// 	err = base->ThrowError( VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT, DBaction_CreatingIndex);
				QuickReleaseRefCountable(progress);
			}
			delete name;
		}
		delete fields;
	}
	ReleaseRefCountable( &base);

	if (!alreadysend)
		SendError( inRequest, inContext, err);
}


void VDBMgr::ExecCreateFullTextIndexOnOneField( IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;

	Base4D *base = inRequest->RetainBaseParam( err);
	if (err == VE_OK && base != NULL)
	{
		sLONG numTable=0, numField=0;
		Boolean forceCreate=false;
		Boolean synchronous=false;
		VString *name = NULL;

		numTable = inRequest->GetLongParam( err);
		if (err == VE_OK)
			numField = inRequest->GetLongParam( err);
		if (err == VE_OK)
			name = inRequest->GetStringParam( err);
		if (err == VE_OK)
			forceCreate = inRequest->GetBooleanParam( err);
		if (err == VE_OK)
			synchronous = inRequest->GetBooleanParam( err);

		if (err == VE_OK)
		{
			VDB4DProgressIndicator* progress = inRequest->RetainProgressParam(inContext);
			// ObjLocker locker( inContext, base);
			// if (locker.CanWork())
			{
				VSyncEvent *event = synchronous ? new VSyncEvent : nil;
				err = base->CreFullTextIndexOnField( numTable, numField, inContext, progress, name, NULL, forceCreate, event);
				if (event != nil)
				{
					event->Lock();
					event->Release();
				}
				if (err == VE_OK)
				{
					inRequest->InitReply(0);
					alreadysend	= true;
				}
			}
			// else
			// 	err = base->ThrowError( VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT, DBaction_CreatingIndex);
			QuickReleaseRefCountable(progress);
		}
		delete name;
	}
	ReleaseRefCountable( &base);

	if (!alreadysend)
		SendError( inRequest, inContext, err);
}


void VDBMgr::ExecDropIndexOnOneField( IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;

	Base4D *base = inRequest->RetainBaseParam( err);
	if (err == VE_OK && base != NULL)
	{
		sLONG numTable=0, numField=0, typIndex=DB4D_Index_NoStorage;
		Boolean synchronous = false;

		numTable = inRequest->GetLongParam( err);
		if (err == VE_OK)
			numField = inRequest->GetLongParam( err);
		if (err == VE_OK)
			typIndex = inRequest->GetLongParam( err);
		if (err == VE_OK)
			synchronous = inRequest->GetBooleanParam( err);

		if (err == VE_OK)
		{
			VDB4DProgressIndicator* progress = inRequest->RetainProgressParam(inContext);
			// ObjLocker locker( inContext, base);
			// if (locker.CanWork())
			{
				VSyncEvent *event = synchronous ? new VSyncEvent : nil;
				err = base->DeleteIndexOnField( numTable, numField, inContext, typIndex, progress, event);
				if (event != nil)
				{
					event->Lock();
					event->Release();
				}
				if (err == VE_OK)
				{
					inRequest->InitReply(0);
					alreadysend	= true;
				}
			}
			// else
			// 	err = base->ThrowError( VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT, DBaction_DeletingIndex);
			QuickReleaseRefCountable(progress);
		}
	}
	ReleaseRefCountable( &base);

	if (!alreadysend)
		SendError( inRequest, inContext, err);
}


void VDBMgr::ExecDropIndexOnMultipleField( IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;

	Base4D *base = inRequest->RetainBaseParam( err);
	if (err == VE_OK && base != NULL)
	{
		FieldNuplet *fields = FieldNuplet::CreateFrom( *inRequest->GetInputStream(), base, err);
		if (fields != NULL && err == VE_OK)
		{
			Boolean synchronous = false;
			sLONG typIndex = inRequest->GetLongParam( err);
			if (err == VE_OK)
				synchronous = inRequest->GetBooleanParam( err);

			if (err == VE_OK)
			{
				VDB4DProgressIndicator* progress = inRequest->RetainProgressParam(inContext);
				// ObjLocker locker( inContext, base);
				// if (locker.CanWork())
				{
					VSyncEvent *event = synchronous ? new VSyncEvent : nil;
					err = base->DeleteIndexOnFields( fields, inContext, typIndex, progress, event);
					if (event != nil)
					{
						event->Lock();
						event->Release();
					}
					if (err == VE_OK)
					{
						inRequest->InitReply(0);
						alreadysend	= true;
					}
				}
				// else
				// 	err = base->ThrowError( VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT, DBaction_DeletingIndex);
				QuickReleaseRefCountable(progress);
			}
		}
		delete fields;
	}
	ReleaseRefCountable( &base);

	if (!alreadysend)
		SendError( inRequest, inContext, err);
}


void VDBMgr::ExecDropFullTextOnOneField( IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;

	Base4D *base = inRequest->RetainBaseParam( err);
	if (err == VE_OK && base != NULL)
	{
		sLONG numTable=0, numField=0;
		Boolean synchronous = false;

		numTable = inRequest->GetLongParam( err);
		if (err == VE_OK)
			numField = inRequest->GetLongParam( err);
		if (err == VE_OK)
			synchronous = inRequest->GetBooleanParam( err);

		if (err == VE_OK)
		{
			VDB4DProgressIndicator* progress = inRequest->RetainProgressParam(inContext);
			// ObjLocker locker( inContext, base);
			// if (locker.CanWork())
			{
				VSyncEvent *event = synchronous ? new VSyncEvent : nil;
				err = base->DeleteFullTextIndexOnField( numTable, numField, inContext, progress, event);
				if (event != nil)
				{
					event->Lock();
					event->Release();
				}
				if (err == VE_OK)
				{
					inRequest->InitReply(0);
					alreadysend	= true;
				}
			}
			// else
			// 	err = base->ThrowError( VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT, DBaction_DeletingIndex);
			QuickReleaseRefCountable(progress);
		}
	}
	ReleaseRefCountable( &base);

	if (!alreadysend)
		SendError( inRequest, inContext, err);
}


void VDBMgr::ExecSetIndexName( IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;

	BaseTaskInfoPtr context = ConvertContext( inContext);
	Base4D *base = inRequest->RetainBaseParam( err);
	if (err == VE_OK && base != NULL && context != NULL)
	{
		IndexInfo *index = inRequest->RetainIndexParam( base, context, err, false);
		if (err == VE_OK && index != NULL)
		{
			VString name;
			err = inRequest->GetStringParam( name);
			if (err == VE_OK)
			{
				ObjLocker locker( inContext, index);
				if (locker.CanWork())
				{
					err = index->SetName( name, inContext);
					if (err == VE_OK)
					{
						inRequest->InitReply(0);
						alreadysend	= true;
					}
				}
				else
				{
					err = base->ThrowError( VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT, DBaction_ChangingIndexName);
				}
			}
		}
		ReleaseRefCountable( &index);
	}
	ReleaseRefCountable( &base);

	if (!alreadysend)
		SendError( inRequest, inContext, err);
}


void VDBMgr::ExecDropIndexByRef( IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;

	BaseTaskInfoPtr context = ConvertContext( inContext);
	Base4D *base = inRequest->RetainBaseParam( err);
	if (err == VE_OK && base != NULL && context != NULL)
	{
		Boolean synchronous = inRequest->GetBooleanParam( err);
		IndexInfo *index = inRequest->RetainIndexParam( base, context, err, false);
		if (err == VE_OK && index != NULL)
		{
			VDB4DProgressIndicator* progress = inRequest->RetainProgressParam(inContext);
			// ObjLocker locker( inContext, base);
			// if (locker.CanWork())
			{
				ObjLocker locker2( inContext, index);
				if (locker2.CanWork())
				{
					VSyncEvent *event = synchronous ? new VSyncEvent : nil;
					base->DeleteIndexByRef( index, inContext, progress, event);
					if (event != nil)
					{
						event->Lock();
						event->Release();
					}
					inRequest->InitReply(0);
					alreadysend	= true;
				}
				else
				{
					err = base->ThrowError( VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT, DBaction_DeletingIndex);
				}
			}
			// else
			// {
				// err = base->ThrowError( VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT, DBaction_DeletingIndex);
			// }
			QuickReleaseRefCountable(progress);
		}
		ReleaseRefCountable( &index);
	}
	ReleaseRefCountable( &base);

	if (!alreadysend)
		SendError( inRequest, inContext, err);
}


void VDBMgr::ExecRebuildIndexByRef( IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;

	BaseTaskInfoPtr context = ConvertContext( inContext);
	Base4D *base = inRequest->RetainBaseParam( err);
	if (err == VE_OK && base != NULL && context != NULL)
	{
		IndexInfo *index = inRequest->RetainIndexParam( base, context, err, false);
		if (err == VE_OK && index != NULL)
		{
			VDB4DProgressIndicator* progress = inRequest->RetainProgressParam(inContext);
			// ObjLocker locker( inContext, base);
			// if (locker.CanWork())
			{
				ObjLocker locker2( inContext, index);
				if (locker2.CanWork())
				{
					base->RebuildIndexByRef( index, inContext, progress, NULL);
					inRequest->InitReply(0);
					alreadysend	= true;
				}
				else
				{
					err = base->ThrowError( VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT, DBaction_RebuildingIndex);
				}
			}
			/*
			else
			{
				err = base->ThrowError( VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT, DBaction_RebuildingIndex);
			}
			*/
			QuickReleaseRefCountable(progress);
		}
		ReleaseRefCountable( &index);
	}
	ReleaseRefCountable( &base);

	if (!alreadysend)
		SendError( inRequest, inContext, err);
}


void VDBMgr::ExecLockDataBaseDef( IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;

	Base4D *base = inRequest->RetainBaseParam( err);
	if (err == VE_OK && base != NULL)
	{
		const VValueBag *lockDef = inRequest->RetainValueBagParam( err);
		if (err == VE_OK && lockDef != NULL)
		{
			BaseTaskInfoPtr context = ConvertContext(inContext);
			if (context != NULL)
			{
				sLONG lockRef;
				const VValueBag *lockerExtra = NULL;
				err = context->LockDataBaseDefWithBag( base, *lockDef, lockRef, &lockerExtra);
				
				inRequest->InitReply(0);
				if (err == VE_OK)
					inRequest->PutLongReply( lockRef);
				else
					inRequest->PutLongReply( 0);

				inRequest->PutBooleanReply( (lockerExtra != NULL) ? true : false);
				if (lockerExtra != NULL)
					inRequest->PutValueBagReply( *lockerExtra);

				ReleaseRefCountable( &lockerExtra);
				alreadysend = true;
			}
		}
		ReleaseRefCountable( &lockDef);
	}
	ReleaseRefCountable( &base);

	if (!alreadysend)
		SendError( inRequest, inContext, err);

}


void VDBMgr::ExecLockTableDef( IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	
	Base4D *base = inRequest->RetainBaseParam( err);
	if (err == VE_OK && base != NULL)
	{
		Table *table = inRequest->RetainTableParam( base, err);
		if (err == VE_OK && table != NULL)
		{
			const VValueBag *lockDef = inRequest->RetainValueBagParam( err);
			if (err == VE_OK && lockDef != NULL)
			{
				BaseTaskInfoPtr context = ConvertContext(inContext);
				if (context != NULL)
				{
					sLONG lockRef;
					const VValueBag *lockerExtra = NULL;
					err = context->LockTableDefWithBag( table, *lockDef, lockRef, &lockerExtra);
					
					inRequest->InitReply(0);
					if (err == VE_OK)
						inRequest->PutLongReply( lockRef);
					else
						inRequest->PutLongReply( 0);

					inRequest->PutBooleanReply( (lockerExtra != NULL) ? true : false);
					if (lockerExtra != NULL)
						inRequest->PutValueBagReply( *lockerExtra);

					ReleaseRefCountable( &lockerExtra);
					alreadysend = true;
				}
			}
			ReleaseRefCountable( &lockDef);
		}
		ReleaseRefCountable( &table);
	}
	ReleaseRefCountable( &base);

	if (!alreadysend)
		SendError( inRequest, inContext, err);
}


void VDBMgr::ExecLockFieldDef( IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	
	Base4D *base = inRequest->RetainBaseParam( err);
	if (err == VE_OK && base != NULL)
	{
		Table *table = inRequest->RetainTableParam( base, err);
		if (err == VE_OK && table != NULL)
		{
			Field *field = inRequest->RetainFieldParam( table, err);
			if (err == VE_OK && field != NULL)
			{
				const VValueBag *lockDef = inRequest->RetainValueBagParam( err);
				if (err == VE_OK && lockDef != NULL)
				{
					BaseTaskInfoPtr context = ConvertContext(inContext);
					if (context != NULL)
					{
						sLONG lockRef;
						const VValueBag *lockerExtra = NULL;
						err = context->LockFieldDefWithBag( field, *lockDef, lockRef, &lockerExtra);
						
						inRequest->InitReply(0);
						if (err == VE_OK)
							inRequest->PutLongReply( lockRef);
						else
							inRequest->PutLongReply( 0);

						inRequest->PutBooleanReply( (lockerExtra != NULL) ? true : false);
						if (lockerExtra != NULL)
							inRequest->PutValueBagReply( *lockerExtra);

						ReleaseRefCountable( &lockerExtra);
						alreadysend = true;
					}
				}
				ReleaseRefCountable( &lockDef);
			}
			ReleaseRefCountable( &field);
		}
		ReleaseRefCountable( &table);
	}
	ReleaseRefCountable( &base);

	if (!alreadysend)
		SendError( inRequest, inContext, err);
}


void VDBMgr::ExecLockRelationDef( IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;

	Base4D *base = inRequest->RetainBaseParam( err);
	if (err == VE_OK && base != NULL)
	{
		Relation *relation = inRequest->RetainRelationParam( base, err);
		if (err == VE_OK && relation != NULL)
		{
			const VValueBag *lockDef = inRequest->RetainValueBagParam( err);
			if (err == VE_OK && lockDef != NULL)
			{
				BaseTaskInfoPtr context = ConvertContext(inContext);
				if (context != NULL)
				{
					sLONG lockRef;
					const VValueBag *lockerExtra = NULL;
					err = context->LockRelationDefWithBag( relation, *lockDef, lockRef, &lockerExtra);
					
					inRequest->InitReply(0);
					if (err == VE_OK)
						inRequest->PutLongReply( lockRef);
					else
						inRequest->PutLongReply( 0);

					inRequest->PutBooleanReply( (lockerExtra != NULL) ? true : false);
					if (lockerExtra != NULL)
						inRequest->PutValueBagReply( *lockerExtra);

					ReleaseRefCountable( &lockerExtra);
					alreadysend = true;
				}
			}
			ReleaseRefCountable( &lockDef);
		}
		ReleaseRefCountable( &relation);
	}
	ReleaseRefCountable( &base);

	if (!alreadysend)
		SendError( inRequest, inContext, err);
}


void VDBMgr::ExecLockIndexDef( IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;

	BaseTaskInfoPtr context = ConvertContext( inContext);
	Base4D *base = inRequest->RetainBaseParam( err);
	if (err == VE_OK && base != NULL && context != NULL)
	{
		IndexInfo *index = inRequest->RetainIndexParam( base, context, err, false);
		if (err == VE_OK && index != NULL)
		{
			const VValueBag *lockDef = inRequest->RetainValueBagParam( err);
			if (err == VE_OK && lockDef != NULL)
			{
				sLONG lockRef;
				const VValueBag *lockerExtra = NULL;
				err = context->LockIndexDefWithBag( index, *lockDef, lockRef, &lockerExtra);
					
				inRequest->InitReply(0);
				if (err == VE_OK)
					inRequest->PutLongReply( lockRef);
				else
					inRequest->PutLongReply( 0);

				inRequest->PutValueBagReply( lockerExtra);
				ReleaseRefCountable( &lockerExtra);
				alreadysend = true;
			}
			ReleaseRefCountable( &lockDef);
		}
		ReleaseRefCountable( &index);
	}
	ReleaseRefCountable( &base);

	if (!alreadysend)
		SendError( inRequest, inContext, err);
}


void VDBMgr::ExecUnlockObjectDef( IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	
	Base4D *base = inRequest->RetainBaseParam( err);
	if (err == VE_OK && base != NULL)
	{
		sLONG lockRef = inRequest->GetLongParam( err);
		if (err == VE_OK)
		{
			BaseTaskInfoPtr context = ConvertContext(inContext);
			if (context != NULL)
			{
				err = context->UnLockStructObject( lockRef);
				
				inRequest->InitReply(0);
				if (err == VE_OK)
					inRequest->PutByteReply( 1);
				else
					inRequest->PutByteReply( 0);

				alreadysend = true;
			}
		}
	}
	ReleaseRefCountable( &base);

	if (!alreadysend)
		SendError( inRequest, inContext, err);
}


void VDBMgr::ExecSetContextExtraData( IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;

	Base4D *base = inRequest->RetainBaseParam( err);
	if (err == VE_OK && base != NULL)
	{
		Boolean haveBag = inRequest->GetBooleanParam( err);
		if (err == VE_OK)
		{
			const VValueBag *extraData = NULL;
			
			if (haveBag)
				extraData = inRequest->RetainValueBagParam( err);

			if (err == VE_OK)
			{
				BaseTaskInfoPtr context = ConvertContext(inContext);
				if (context != NULL)
				{
					context->getContextOwner()->SetExtraData( extraData);
					if (err == VE_OK)
					{
						inRequest->InitReply(0);
						alreadysend = true;
					}
				}
			}
			ReleaseRefCountable( &extraData);
		}
	}
	ReleaseRefCountable( &base);

	if (!alreadysend)
		SendError( inRequest, inContext, err);
}


void VDBMgr::ExecSelectAllRecords(IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	BaseTaskInfoPtr context = ConvertContext(inContext);
	Base4D* base = inRequest->RetainBaseParam( err);
	if (base != nil)
	{
		err = inRequest->GetThingsToForget( this, context);
		if (err == VE_OK)
		{
			Table* tt = inRequest->RetainTableParam(base, err);
			if (tt != nil)
			{
				uLONG loadOptions = 0;
				inRequest->GetInputStream()->Get( &loadOptions);
				bool wantsLoadRecord = (loadOptions & 1) != 0;
				bool wantsReadOnly = (loadOptions & 2) != 0;
				bool wantsSubTable =  true; // always true for 4D old langage (loadOptions & 4) != 0;

				Selection* sel = tt->GetDF()->AllRecords( context, err);
				FicheInMem *fiche = nil;

				if ( (err == VE_OK) && wantsLoadRecord && !sel->IsEmpty() )
				{
					// client expects the first record
					RecIDType recordID = sel->GetFic( 0);
					fiche = tt->GetDF()->LoadRecord( recordID, err, wantsReadOnly ? DB4D_Do_Not_Lock : DB4D_Keep_Lock_With_Record, context, wantsSubTable);
				}

				if (err == VE_OK)
				{
					inRequest->InitReply(0);

					err = inRequest->PutUpdatedInfo(tt, context);

					if (err == VE_OK)
						err = inRequest->PutSelectionReply(sel, inContext);
					if ( (err == VE_OK) && (fiche != nil) )
					{
						err = inRequest->PutFicheInMemReply( fiche, inContext);
					}

					if (err != VE_OK)
					{
						inRequest->ReplyFailed();
					}
					alreadysend = true;
					sel->Release();
					if (fiche != nil)
						fiche->Release();
				}
				tt->Release();
			}
			base->Release();
		}
	}

	if (!alreadysend)
		SendError(inRequest, inContext, err);
}



void VDBMgr::ExecSaveRecord(IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	BaseTaskInfoPtr context = ConvertContext(inContext);
	Base4D* base = inRequest->RetainBaseParam( err);
	if (base != nil)
	{
		err = inRequest->GetThingsToForget( this, context);
		if (err == VE_OK)
		{
			Table* tt = inRequest->RetainTableParam(base, err);
			if (tt != nil)
			{
				FicheInMem* rec = inRequest->RetainFicheInMemParam(base, tt, context, err);
				if (rec != nil)
				{
					StampVector oldStamps;
					rec->GetFieldModificationStamps(oldStamps);
					err = tt->GetDF()->SaveRecord(rec, context);
					if (err == VE_OK)
					{
						KeepRecordOnServer(rec, true);
						inRequest->InitReply(0);
						inRequest->PutUpdatedInfo(tt, context);
						inRequest->PutLongReply(rec->GetNum());
						inRequest->PutLong8Reply(rec->GetAutoSeqValue());
						rec->SendModifiedFields(inRequest->GetOutputStream(), context, &oldStamps);

						if (inRequest->GetLastErrorReply() != VE_OK)
						{
							inRequest->ReplyFailed();
						}
						alreadysend = true;
					}
					rec->Release();
				}
				tt->Release();
			}
		}
		base->Release();
	}

	if (!alreadysend)
		SendError(inRequest, inContext, err);
}


void VDBMgr::ExecDeleteRecordByID(IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	BaseTaskInfoPtr context = ConvertContext(inContext);
	Base4D* base = inRequest->RetainBaseParam( err);
	if (base != nil)
	{
		err = inRequest->GetThingsToForget( this, context);
		if (err == VE_OK)
		{
			Table* tt = inRequest->RetainTableParam(base, err);
			if (tt != nil)
			{
				sLONG recid = inRequest->GetLongParam(err);
				if (err == VE_OK)
				{
					if (testAssert( recid != kDB4D_NullRecordID)) 
					{
						DataTable *df = tt->GetDF();
						FicheInMem *f = df->LoadRecord( recid, err, DB4D_Keep_Lock_With_Record, context, true);
						if (f != nil)
						{
							err = df->DelRecord( f, context);
							f->Release();
						}
					}
					if (err == VE_OK)
					{
						inRequest->InitReply(0);
						inRequest->PutUpdatedInfo(tt, context);

						alreadysend = true;
					}
				}
			}
		}
		base->Release();
	}

	if (!alreadysend)
		SendError(inRequest, inContext, err);
}


void VDBMgr::ExecDeleteRecord(IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	BaseTaskInfoPtr context = ConvertContext(inContext);
	Base4D* base = inRequest->RetainBaseParam( err);
	if (base != nil)
	{
		err = inRequest->GetThingsToForget( this, context);
		if (err == VE_OK)
		{
			Table* tt = inRequest->RetainTableParam(base, err);
			if (tt != nil)
			{
				FicheInMem* rec = inRequest->RetainFicheInMemParam(base, tt, context, err);
				if (rec != nil)
				{
					err = tt->GetDF()->DelRecord(rec, context);
					rec->Release();
					if (err == VE_OK)
					{
						inRequest->InitReply(0);
						inRequest->PutUpdatedInfo(tt, context);

						inRequest->PutBooleanReply(true);
						if (inRequest->GetLastErrorReply() != VE_OK)
						{
							inRequest->ReplyFailed();
						}
						alreadysend = true;
					}
				}
				tt->Release();
			}
		}
		base->Release();
	}

	if (!alreadysend)
		SendError(inRequest, inContext, err);
}


void VDBMgr::ExecConnectRecord(IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	BaseTaskInfoPtr context = ConvertContext(inContext);
	Base4D* base = inRequest->RetainBaseParam( err);
	if (base != nil)
	{
		err = inRequest->GetThingsToForget( this, context);
		if (err == VE_OK)
		{
			Table* tt = inRequest->RetainTableParam(base, err);
			if (tt != nil)
			{
				VUUID xid;
				err = xid.ReadFromStream(inRequest->GetInputStream());
				if (err == VE_OK)
				{
					bool alreadyKept = false;;
					FicheInMem* rec = VDBMgr::GetManager()->RetainServerKeptRecord(xid.GetBuffer());
					if (rec == nil)
					{
						rec = tt->GetDF()->NewRecord(err, context);
						if (rec != nil)
						{
							rec->SetID(xid.GetBuffer());
						}
					}
					else
						alreadyKept = true;

					if (rec != nil)
					{
						inRequest->InitReply(0);
						inRequest->PutUpdatedInfo(tt, context);
						inRequest->PutLong8Reply(rec->GetAutoSeqValue());
						if (inRequest->GetLastErrorReply() != VE_OK)
						{
							inRequest->ReplyFailed();
						}
						alreadysend = true;
						if (!alreadyKept)
							KeepRecordOnServer(rec, true);
						rec->Release();
					}
				}

				tt->Release();
			}
		}
		base->Release();
	}

	if (!alreadysend)
		SendError(inRequest, inContext, err);
}


void VDBMgr::ExecCountRecordsInTable(IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	BaseTaskInfoPtr context = ConvertContext(inContext);
	Base4D* base = inRequest->RetainBaseParam( err);
	if (base != nil)
	{
		err = inRequest->GetThingsToForget( this, context);
		if (err == VE_OK)
		{
			Table* tt = inRequest->RetainTableParam(base, err);
			if (tt != nil)
			{
				inRequest->InitReply(0);
				inRequest->PutUpdatedInfo(tt, context);
				inRequest->PutLongReply( tt->GetDF()->GetNbRecords(context));
				alreadysend = true;

				tt->Release();
			}
		}
		base->Release();
	}
	if (!alreadysend)
		SendError(inRequest, inContext, err);
}


void VDBMgr::ExecLoadRecord(IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	BaseTaskInfoPtr context = ConvertContext(inContext);
	Base4D* base = inRequest->RetainBaseParam( err);
	if (base != nil)
	{
		err = inRequest->GetThingsToForget( this, context);
		if (err == VE_OK)
		{
			Table* tt = inRequest->RetainTableParam(base, err);
			if (tt != nil)
			{
				RecIDType recordID = inRequest->GetLongParam(err);

				uLONG loadOptions = 0;
				inRequest->GetInputStream()->Get( &loadOptions);
				bool wantsReadOnly = (loadOptions & 2) != 0;
				bool wantsSubTable = (loadOptions & 4) != 0;

				FicheInMem *fiche = tt->GetDF()->LoadRecord( recordID, err, wantsReadOnly ? DB4D_Do_Not_Lock : DB4D_Keep_Lock_With_Record, context, wantsSubTable);

				if ( (err == VE_OK) && (fiche != nil) )
				{
					inRequest->InitReply(0);
					inRequest->PutUpdatedInfo(tt, context);
					err = inRequest->PutFicheInMemReply( fiche, inContext);
					if (err != VE_OK)
					{
						inRequest->ReplyFailed();
					}
					alreadysend = true;
				}

				if (fiche != nil)
					fiche->Release();

				tt->Release();
			}
		}
		base->Release();
	}
	if (!alreadysend)
		SendError(inRequest, inContext, err);
}


void VDBMgr::ExecWhoLockedRecord(IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	BaseTaskInfoPtr context = ConvertContext(inContext);
	Base4D* base = inRequest->RetainBaseParam( err);
	if (base != nil)
	{
		err = inRequest->GetThingsToForget( this, context);
		if (err == VE_OK)
		{
			Table* tt = inRequest->RetainTableParam(base, err);
			if (tt != nil)
			{
				RecIDType recordID = inRequest->GetLongParam(err);

				DB4D_KindOfLock kindOfLock = DB4D_LockedByNone;
				const VValueBag *lockingContextRetainedExtraData = nil;
				tt->GetDF()->WhoLockedRecord( recordID, err, context, &kindOfLock, &lockingContextRetainedExtraData);

				if (err == VE_OK)
				{
					inRequest->InitReply(0);
					inRequest->PutUpdatedInfo(tt, context);
					inRequest->PutLongReply( (sLONG) kindOfLock);
					inRequest->PutValueBagReply( lockingContextRetainedExtraData);
					alreadysend = true;
				}
				QuickReleaseRefCountable( lockingContextRetainedExtraData);

				tt->Release();
			}
		}
		base->Release();
	}
	if (!alreadysend)
		SendError(inRequest, inContext, err);
}


void VDBMgr::ExecDeleteRecordsInSelection(IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	BaseTaskInfo* context = ConvertContext(inContext);
	Base4D* base = inRequest->RetainBaseParam( err);
	if (base != nil)
	{
		err = inRequest->GetThingsToForget( this, context);
		if (err == VE_OK)
		{
			Table* tt = inRequest->RetainTableParam(base, err);
			if (tt != nil)
			{
				Selection* sel = inRequest->RetainSelectionParam(base, tt);
				if (sel != nil)
				{
					Boolean buildlockedset = inRequest->GetBooleanParam(err);
					if (err == VE_OK)
					{
						VDB4DProgressIndicator* progress = inRequest->RetainProgressParam(inContext);
						Bittab* lockedset = new Bittab();
						lockedset->SetOwner(base);
						err = sel->DeleteRecords(context, lockedset, nil, progress);
						/*
						sLONG firstnotdeleted = lockedset.FindNextBit(0);
						if (firstnotdeleted == -1 && err != VE_OK)
							firstnotdeleted = -2;
							*/
						inRequest->InitReply(0);
						inRequest->PutUpdatedInfo(tt, context);
						if (lockedset->IsEmpty())
							inRequest->PutSetReply(nil, inContext);
						else
							inRequest->PutSetReply(lockedset, inContext);
						//inRequest->PutLongReply(firstnotdeleted);
						alreadysend = true;
						QuickReleaseRefCountable(progress);
						QuickReleaseRefCountable(lockedset);
					}

					sel->Release();
				}
				tt->Release();
			}
		}
		base->Release();
	}
	if (!alreadysend)
		SendError(inRequest, inContext, err);
}


void VDBMgr::ExecExecuteQuery(IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	BaseTaskInfo* context = ConvertContext(inContext);
	Base4D* base = inRequest->RetainBaseParam( err);
	if (base != nil)
	{
		err = inRequest->GetThingsToForget( this, context);
		if (err == VE_OK)
		{
			Table* tt = inRequest->RetainTableParam(base, err);
			if (tt != nil)
			{
				SearchTab* query = inRequest->RetainQueryParam(tt, err);
				if (query != nil)
				{
					QueryOptions* options = inRequest->RetainQueryOptionsParam(base, tt, context, err);
					if (options != nil)
					{
						VDB4DProgressIndicator* progress = inRequest->RetainProgressParam(inContext);
						QueryResult result;
						err = tt->GetDF()->ExecuteQuery(query, options, result, context, progress);
						if (result.GetLockedSet() != nil && result.GetLockedSet()->Compte() != 0)
							err = VE_OK;
						if (err == VE_OK)
						{
							inRequest->InitReply(0);
							inRequest->PutUpdatedInfo(base, context);
							inRequest->PutQueryResultReply(&result, context);
							alreadysend = true;
							
						}
						QuickReleaseRefCountable(progress);
						options->Release();
					}
					delete query;
				}
				tt->Release();
			}
		}
		base->Release();
	}
	if (!alreadysend)
		SendError(inRequest, inContext, err);
}


void VDBMgr::ExecSortSelection(IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	BaseTaskInfo* context = ConvertContext(inContext);
	Base4D* base = inRequest->RetainBaseParam( err);
	if (base != nil)
	{
		err = inRequest->GetThingsToForget( this, context);
		if (err == VE_OK)
		{
			Table* tt = inRequest->RetainTableParam(base, err);
			if (tt != nil)
			{
				Selection* sel = inRequest->RetainSelectionParam(base, tt);
				if (sel != nil)
				{
					SortTab tabs(base);
					err = inRequest->ReadSortTabParam(tabs);
					if (err == VE_OK)
					{
						Boolean uniq = inRequest->GetBooleanParam(err);
						if (err == VE_OK)
						{
							VDB4DProgressIndicator* progress = inRequest->RetainProgressParam(inContext);
							DataTable* df = tt->GetDF();
							if (df != nil)
							{
								Selection* newsel = df->SortSel(tabs, sel, context, progress, err);
								if (err == VE_OK)
								{
									alreadysend = true;
									inRequest->InitReply(0);
									inRequest->PutUpdatedInfo(tt, context);

									if (newsel == sel && sel->MustBeKeptOnServer())
									{
										inRequest->PutBooleanReply(true);
										inRequest->PutLongReply(sel->GetQTfic());
										sel->PutClearCacheInfo(inRequest->GetOutputStream());
									}
									else
									{
										inRequest->PutBooleanReply(false);
										inRequest->PutSelectionReply(newsel, inContext);
									}
								}
								QuickReleaseRefCountable(newsel);
							}
							else
								err = ThrowBaseError(VE_DB4D_WRONGTABLEREF, noaction);
							QuickReleaseRefCountable(progress);
						}
					}
					sel->Release();
				}
				tt->Release();
			}
		}
		base->Release();
	}

	if (!alreadysend)
		SendError(inRequest, inContext, err);
}


void VDBMgr::ExecSendLastRemoteInfo(IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	BaseTaskInfo* context = ConvertContext(inContext);
	Base4D* base = inRequest->RetainBaseParam( err);
	if (base != nil)
	{
		err = inRequest->GetThingsToForget( this, context);
		if (err == VE_OK)
		{
			alreadysend = true;
			inRequest->InitReply(0);
		}
		base->Release();
	}
	if (!alreadysend)
		SendError(inRequest, inContext, err);
}


void VDBMgr::ExecDataToCollection(IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	BaseTaskInfo* context = ConvertContext(inContext);
	Base4D* base = inRequest->RetainBaseParam( err);
	if (base != nil)
	{
		err = inRequest->GetThingsToForget( this, context);
		if (err == VE_OK)
		{
			Table* tt = inRequest->RetainTableParam(base, err);
			if (tt != nil)
			{
				if (err == VE_OK)
				{
					Selection* sel = inRequest->RetainSelectionParam(base, tt);
					if (sel != nil)
					{
						sLONG FromRecInSel, ToRecInSel;
						FromRecInSel = inRequest->GetLongParam(err);
						if (err == VE_OK)
							ToRecInSel = inRequest->GetLongParam(err);

						DB4DCollectionManager* collection = inRequest->RetainCollectionParam(base, err);
						if (collection != nil)
						{
							if (err == VE_OK)
							{
								VDB4DProgressIndicator* progress = inRequest->RetainProgressParam(inContext);
								bool fullycompleted = true;
								sLONG maxelems;
								err = tt->GetDF()->DataToCollection(sel, *collection, FromRecInSel, ToRecInSel, context, progress, true, fullycompleted, maxelems, true); 
								if (err == VE_OK)
								{
									alreadysend = true;
									inRequest->InitReply(0);
									inRequest->PutUpdatedInfo(tt, context);
									inRequest->PutCollectionReply(*collection, fullycompleted, maxelems);
								}
								QuickReleaseRefCountable(progress);
							}

							collection->Release();
						}
						sel->Release();
					}
				}
				tt->Release();
			}
		}
		base->Release();
	}

	if (!alreadysend)
	SendError(inRequest, inContext, err);
}


void VDBMgr::ExecCollectionToData(IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	BaseTaskInfo* context = ConvertContext(inContext);
	Base4D* base = inRequest->RetainBaseParam( err);
	if (base != nil)
	{
		err = inRequest->GetThingsToForget( this, context);
		if (err == VE_OK)
		{
			Table* tt = inRequest->RetainTableParam(base, err);
			if (tt != nil)
			{
				Selection* sel = inRequest->RetainSelectionParam(base, tt);
				if (sel != nil)
				{
					Boolean AddToSel, CreateAlways;
					AddToSel = inRequest->GetBooleanParam(err);
					if (err == VE_OK)
						CreateAlways = inRequest->GetBooleanParam(err);
					if (err == VE_OK)
					{
						DB4DCollectionManager* collection = inRequest->RetainCollectionParam(base, err);
						if (collection != nil)
						{
							VDB4DProgressIndicator* progress = inRequest->RetainProgressParam(inContext);
							Selection* newsel = nil;
							Bittab* lockedset = nil;
							
							err = tt->GetDF()->CollectionToData(sel, *collection, context,  AddToSel, CreateAlways, newsel, lockedset, progress);

							if (err == VE_OK)
							{
								alreadysend = true;
								inRequest->InitReply(0);
								inRequest->PutUpdatedInfo(tt, context);

								if (newsel == sel && sel->MustBeKeptOnServer())
								{
									inRequest->PutBooleanReply(true);
									inRequest->PutLongReply(sel->GetQTfic());
									sel->PutClearCacheInfo(inRequest->GetOutputStream());
								}
								else
								{
									inRequest->PutBooleanReply(false);
									inRequest->PutSelectionReply(newsel, inContext);
								}
								if (lockedset != nil)
									lockedset->SetOwner(base);
								inRequest->PutSetReply(lockedset, inContext);
							}

							collection->Release();
							QuickReleaseRefCountable(lockedset);
							QuickReleaseRefCountable(progress);
						}
					}
					sel->Release();
				}
				tt->Release();
			}
		}
		base->Release();
	}

	if (!alreadysend)
		SendError(inRequest, inContext, err);
}


void VDBMgr::ExecGetDistinctValues(IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	BaseTaskInfo* context = ConvertContext(inContext);
	Base4D* base = inRequest->RetainBaseParam( err);
	if (base != nil)
	{
		err = inRequest->GetThingsToForget( this, context);
		if (err == VE_OK)
		{
			Table* tt = inRequest->RetainTableParam(base, err);
			if (tt != nil)
			{
				Field* cri = inRequest->RetainFieldParam(tt, err);
				{
					Selection* sel = inRequest->RetainSelectionParam(base, tt);
					if (sel != nil)
					{
						DB4DCollectionManager* collection = inRequest->RetainCollectionParam(base, err);
						if (collection != nil)
						{
							VCompareOptions options;
							err = GetVCompareOptionsFromStream(options, *(inRequest->GetInputStream()));
							if (err == VE_OK)
							{
								VDB4DProgressIndicator* progress = inRequest->RetainProgressParam(inContext);
								err = sel->GetDistinctValues(cri, *collection, context, progress, options);
								if (err == VE_OK)
								{
									alreadysend = true;
									inRequest->InitReply(0);
									inRequest->PutUpdatedInfo(tt, context);

									inRequest->PutCollectionReply(*collection, true, -1);
								}
								QuickReleaseRefCountable(progress);
							}

							collection->Release();
						}
						sel->Release();
					}
					cri->Release();
				}
				tt->Release();
			}
		}
		base->Release();
	}

	if (!alreadysend)
		SendError(inRequest, inContext, err);
}


void VDBMgr::ExecActivateManyToOne(IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	BaseTaskInfo* context = ConvertContext(inContext);
	Base4D* base = inRequest->RetainBaseParam( err);
	if (base != nil)
	{
		err = inRequest->GetThingsToForget( this, context);
		if (err == VE_OK)
		{
			Relation* rel = inRequest->RetainRelationParam(base, err);
			if (rel != nil)
			{
				FicheInMem* rec = inRequest->RetainFicheInMemParam(base, rel->GetSourceTable(), context, err);
				if (rec != nil)
				{
					QueryOptions* options = inRequest->RetainQueryOptionsParam(base, rel->GetDestTable(), context, err);
					if (options != nil)
					{
						Boolean OldOne, NoCache;
						OldOne = inRequest->GetBooleanParam(err);
						if (err == VE_OK)
							NoCache = inRequest->GetBooleanParam(err);
						if (err == VE_OK)
						{
							FicheInMem* recresult = nil;
							err = rel->ActivateManyToOne(rec, recresult, context, OldOne, NoCache, options->GetRecordWayOfLocking());
							if (err == VE_OK)
							{
								alreadysend = true;
								QueryResult qresult;
								qresult.SetFirstRecord(recresult);
								inRequest->InitReply(0);
								inRequest->PutUpdatedInfo(rel->GetSourceTable(), rel->GetDestTable(), context);

								inRequest->PutQueryResultReply(&qresult, context);
							}
							QuickReleaseRefCountable(recresult);
							options->Release();
						}
					}
					rec->Release();
				}
				rel->Release();
			}
		}
		base->Release();
	}

	if (!alreadysend)
		SendError(inRequest, inContext, err);
}


void VDBMgr::ExecActivateManyToOneS(IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	BaseTaskInfo* context = ConvertContext(inContext);
	Base4D* base = inRequest->RetainBaseParam( err);
	if (base != nil)
	{
		err = inRequest->GetThingsToForget( this, context);
		if (err == VE_OK)
		{
			Relation* rel = inRequest->RetainRelationParam(base, err);
			if (rel != nil)
			{
				FicheInMem* rec = inRequest->RetainFicheInMemParam(base, rel->GetSourceTable(), context, err);
				if (rec != nil)
				{
					QueryOptions* options = inRequest->RetainQueryOptionsParam(base, rel->GetDestTable(), context, err);
					if (options != nil)
					{
						Boolean OldOne;
						OldOne = inRequest->GetBooleanParam(err);
						if (err == VE_OK)
						{
							Selection* selresult = nil;
							err = rel->ActivateManyToOneS(rec, selresult, context, OldOne, options->GetWayOfLocking());
							if (err == VE_OK)
							{
								alreadysend = true;
								QueryResult qresult;
								qresult.SetSelection(selresult);
								if (selresult != nil && options->GetWantsFirstRecord() && !selresult->IsEmpty())
								{
									FicheInMem* firstrect = selresult->GetParentFile()->LoadRecord(selresult->GetFic(0), err, options->GetRecordWayOfLocking(), context, true);
									qresult.SetFirstRecord(firstrect);
									ReleaseRefCountable(&firstrect);
								}
								inRequest->InitReply(0);
								inRequest->PutUpdatedInfo(rel->GetSourceTable(), rel->GetDestTable(), context);
								inRequest->PutQueryResultReply(&qresult, context);
							}
							options->Release();
							QuickReleaseRefCountable(selresult);
						}
					}
					rec->Release();
				}
				rel->Release();
			}
		}
		base->Release();
	}

	if (!alreadysend)
		SendError(inRequest, inContext, err);
}


void VDBMgr::ExecRelateManySelection(IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	BaseTaskInfo* context = ConvertContext(inContext);
	Base4D* base = inRequest->RetainBaseParam( err);
	if (base != nil)
	{
		err = inRequest->GetThingsToForget( this, context);
		if (err == VE_OK)
		{
			Table* StartingSelectionTable = inRequest->RetainTableParam(base, err);
			if (StartingSelectionTable != nil)
			{
				QueryOptions* options = inRequest->RetainQueryOptionsParam(base, StartingSelectionTable, context, err);
				if (options != nil)
				{
					Table* dest = inRequest->RetainTableParam(base, err);
					if (dest != nil)
					{
						Field* RelationStart = inRequest->RetainFieldParam(dest, err);
						if (RelationStart != nil)
						{
							VDB4DProgressIndicator* progress = inRequest->RetainProgressParam(inContext);
							QueryResult qresult;
							err = base->RelateManySelection(StartingSelectionTable, RelationStart, qresult, context, options, progress);
							if (err == VE_OK)
							{
								alreadysend = true;
								inRequest->InitReply(0);
								inRequest->PutUpdatedInfo(StartingSelectionTable, dest, context);
								inRequest->PutQueryResultReply(&qresult, context);
							}
							RelationStart->Release();
							QuickReleaseRefCountable(progress);
						}
						dest->Release();
					}
					options->Release();
				}

				StartingSelectionTable->Release();
			}
		}
		base->Release();
	}

	if (!alreadysend)
		SendError(inRequest, inContext, err);
}


void VDBMgr::ExecRelateOneSelection(IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	BaseTaskInfo* context = ConvertContext(inContext);
	Base4D* base = inRequest->RetainBaseParam( err);
	if (base != nil)
	{
		err = inRequest->GetThingsToForget( this, context);
		if (err == VE_OK)
		{
			Table* StartingSelectionTable = inRequest->RetainTableParam(base, err);
			if (StartingSelectionTable != nil)
			{
				QueryOptions* options = inRequest->RetainQueryOptionsParam(base, StartingSelectionTable, context, err);
				if (options != nil)
				{
					sLONG TargetOneTable = inRequest->GetLongParam(err);
					if (err == VE_OK)
					{
						Table* dest = base->RetainTable(TargetOneTable);
						if (dest == nil)
							err = ThrowBaseError(VE_DB4D_WRONGTABLEREF, noaction);
						else
						{
							sLONG nb = inRequest->GetLongParam(err);
							if (err == VE_OK)
							{
								vector<Relation*> path;
								if (nb != 0)
								{
									for (sLONG i = 0; i < nb && err == VE_OK; i++)
									{
										Relation* rel = inRequest->RetainRelationParam(base, err);
										if (rel != nil)
										{
											path.push_back(rel);
										}
									}
								}
								VDB4DProgressIndicator* progress = inRequest->RetainProgressParam(inContext);
								QueryResult qresult;
								err = base->RelateOneSelection(StartingSelectionTable, TargetOneTable, qresult, context, options, progress, nb == 0 ? nil : &path);
								if (err == VE_OK)
								{
									alreadysend = true;
									inRequest->InitReply(0);
									inRequest->PutUpdatedInfo(StartingSelectionTable, dest, context);
									inRequest->PutQueryResultReply(&qresult, context);
								}
								for (vector<Relation*>::iterator cur = path.begin(), end = path.end(); cur != end; cur++)
								{
									(*cur)->Release();
								}
								QuickReleaseRefCountable(progress);
							}
							dest->Release();
						}
					}
					options->Release();
				}

				StartingSelectionTable->Release();
			}
		}
		base->Release();
	}

	if (!alreadysend)
		SendError(inRequest, inContext, err);
}


void VDBMgr::ExecActivateOneToMany(IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	BaseTaskInfo* context = ConvertContext(inContext);
	Base4D* base = inRequest->RetainBaseParam( err);
	if (base != nil)
	{
		err = inRequest->GetThingsToForget( this, context);
		if (err == VE_OK)
		{
			Relation* rel = inRequest->RetainRelationParam(base, err);
			if (rel != nil)
			{
				FicheInMem* rec = inRequest->RetainFicheInMemParam(base, rel->GetDestTable(), context, err);
				if (rec != nil)
				{
					QueryOptions* options = inRequest->RetainQueryOptionsParam(base, rel->GetSourceTable(), context, err);
					if (options != nil)
					{
						Boolean OldOne, NoCache;
						OldOne = inRequest->GetBooleanParam(err);
						if (err == VE_OK)
							NoCache = inRequest->GetBooleanParam(err);
						if (err == VE_OK)
						{
							Selection* selresult = nil;
							err = rel->ActivateOneToMany(rec, selresult, context, OldOne, true, options->GetWayOfLocking());
							if (err == VE_OK)
							{
								alreadysend = true;
								QueryResult qresult;
								qresult.SetSelection(selresult);
								if (selresult != nil && options->GetWantsFirstRecord() && !selresult->IsEmpty())
								{
									FicheInMem* firstrect = selresult->GetParentFile()->LoadRecord(selresult->GetFic(0), err, options->GetRecordWayOfLocking(), context, true);
									qresult.SetFirstRecord(firstrect);
									ReleaseRefCountable(&firstrect);
								}
								inRequest->InitReply(0);
								inRequest->PutUpdatedInfo(rel->GetDestTable(), rel->GetSourceTable(), context);
								inRequest->PutQueryResultReply(&qresult, context);
							}
							QuickReleaseRefCountable(selresult);
							options->Release();
						}
					}
					rec->Release();
				}
				rel->Release();
			}
		}
		base->Release();
	}

	if (!alreadysend)
		SendError(inRequest, inContext, err);
}


void VDBMgr::ExecReserveRecordNumber(IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	BaseTaskInfo* context = ConvertContext(inContext);
	Base4D* base = inRequest->RetainBaseParam( err);
	if (base != nil)
	{
		err = inRequest->GetThingsToForget( this, context);
		if (err == VE_OK)
		{
			Table* tt = inRequest->RetainTableParam(base, err);
			if (tt != nil)
			{
				FicheInMem* rec = inRequest->RetainFicheInMemMinimalParam(base, tt, context, err);
				if (rec != nil)
				{
					err = rec->ReservedRecordNumber(context);
					if (err == VE_OK)
					{
						alreadysend = true;
						inRequest->InitReply(0);
						inRequest->PutUpdatedInfo(tt, context);
						inRequest->PutLongReply(rec->GetRecordNumberReserved());
					}
					rec->Release();
				}
				tt->Release();
			}
		}
		base->Release();
	}

	if (!alreadysend)
		SendError(inRequest, inContext, err);
}


void VDBMgr::ExecFindKey(IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	BaseTaskInfo* context = ConvertContext(inContext);
	Base4D* base = inRequest->RetainBaseParam( err);
	if (base != nil)
	{
		err = inRequest->GetThingsToForget( this, context);
		if (err == VE_OK)
		{
			IndexInfo* ind = inRequest->RetainIndexParam(base, context, err, true);
			if (ind != nil)
			{
				Selection* sel = inRequest->RetainSelectionParam(base, ind->GetTargetTable());
				VCompareOptions options;
				err = GetVCompareOptionsFromStream(options, *(inRequest->GetInputStream()));
				if (err == VE_OK)
				{
					VValueSingle* valtofind = inRequest->BuildValueParam(err);
					if (valtofind == nil && err == VE_OK)
					{
						base->ThrowError(VE_DB4D_THIS_IS_NULL, noaction);
					}
					if (err == VE_OK)
					{
						Bittab* filter = nil;
						if (sel != nil)
							filter = sel->GenereBittab(context, err);

						if (err == VE_OK)
						{
							BTitemIndex* key = ((IndexInfoFromField*)ind)->BuildKeyFromVValue((const ValPtr)valtofind, err);
							if (key != nil)
							{
								BTitemIndex* val = nil;
								RecIDType res = ind->FindKey(key, context, options, filter, &val);
								VValueSingle* valresult = nil;
								if (val != nil)
								{
									valresult = ((IndexInfoFromField*)ind)->CreateVValueWithKey(val, err);
									ind->FreeKey(val);
								}
								ind->FreeKey(key);

								alreadysend = true;
								inRequest->InitReply(0);
								inRequest->PutLongReply(res);
								inRequest->PutValueReply(valresult);
								if (valresult != nil)
									delete valresult;
							}
						}
					}

					if (valtofind != nil)
						delete valtofind;

				}
				
				ReleaseRefCountable(&sel);
				ind->ReleaseValid();
				ind->Release();
			}
		}
		base->Release();
	}

	if (!alreadysend)
		SendError(inRequest, inContext, err);
}


void VDBMgr::ExecScanIndex(IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	BaseTaskInfo* context = ConvertContext(inContext);
	Base4D* base = inRequest->RetainBaseParam( err);
	if (base != nil)
	{
		err = inRequest->GetThingsToForget( this, context);
		if (err == VE_OK)
		{
			IndexInfo* ind = inRequest->RetainIndexParam(base, context, err, true);
			if (ind != nil)
			{
				QueryOptions* options = inRequest->RetainQueryOptionsParam(base, ind->GetTargetTable(), context, err);
				if (options != nil)
				{
					Boolean Ascent, KeepSorted = inRequest->GetBooleanParam(err);
					if (err == VE_OK)
						Ascent = inRequest->GetBooleanParam(err);
					if (err == VE_OK)
					{
						Selection* sel = ind->ScanIndex(options->GetLimit(), KeepSorted, Ascent, context, err, options->GetFilter(), nil);
						if (sel != nil)
						{
							QueryResult result;
							result.SetSelection(sel);
							if (options->GetWantsFirstRecord() && !sel->IsEmpty())
							{
								VError err2;
								FicheInMem* rec = ind->GetTargetTable()->GetDF()->LoadRecord(sel->GetFic(0), err2, options->GetRecordWayOfLocking(), context, true);
								if (rec != nil)
								{
									result.SetFirstRecord(rec);
									rec->Release();
								}
							}
							alreadysend = true;
							inRequest->InitReply(0);
							inRequest->PutUpdatedInfo(ind->GetTargetTable(), context);
							inRequest->PutQueryResultReply(&result, context);

							sel->Release();
						}

					}
					options->Release();
				}
				ind->ReleaseValid();
				ind->Release();
			}
		}
		base->Release();
	}

	if (!alreadysend)
		SendError(inRequest, inContext, err);
}


void VDBMgr::ExecActivateRelsOnAPath(IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	BaseTaskInfo* context = ConvertContext(inContext);
	Base4D* base = inRequest->RetainBaseParam( err);
	if (base != nil)
	{
		err = inRequest->GetThingsToForget( this, context);
		if (err == VE_OK)
		{
			Table* maintable = inRequest->RetainTableParam(base, err);
			if (maintable != nil)
			{
				FicheInMem* mainrec = inRequest->RetainFicheInMemParam(base, maintable, context, err);
				if (mainrec != nil)
				{
					vector<Relation*> path;
					sLONG nbrels = inRequest->GetLongParam(err);
					for (sLONG i = 0; i < nbrels && err == VE_OK; i++)
					{
						Relation* rel = inRequest->RetainRelationParam(base, err);
						if (rel != nil)
						{
							path.push_back(rel);
						}
					}
					if (err == VE_OK)
					{
						FicheInMem* dest = mainrec;
						FicheInMem* source = nil;
						mainrec = nil;

						for (vector<Relation*>::iterator cur = path.begin(), end = path.end(); cur != end && err == VE_OK && dest != nil; cur++)
						{
							source = dest;
							Relation* rel = *cur;
							err = rel->ActivateManyToOne(source, dest, context, false, true, DB4D_Do_Not_Lock);
							source->Release();
						}

						if (err == VE_OK)
						{
							alreadysend = true;
							inRequest->InitReply(0);
							if (dest == nil)
								inRequest->PutBooleanReply(false);
							else
							{
								inRequest->PutBooleanReply(true);
								inRequest->PutFicheInMemReply(dest, inContext);
								dest->Release();
							}
						}

					}
					for (vector<Relation*>::iterator cur = path.begin(), end = path.end(); cur != end; cur++)
					{
						(*cur)->Release();
					}
					if (mainrec != nil)
						mainrec->Release();
				}
				maintable->Release();
			}
		}
		base->Release();
	}

	if (!alreadysend)
		SendError(inRequest, inContext, err);
}



void VDBMgr::ExecCacheDisplaySelection(IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	BaseTaskInfo* context = ConvertContext(inContext);
	Base4D* base = inRequest->RetainBaseParam( err);
	if (base != nil)
	{
		err = inRequest->GetThingsToForget( this, context);
		if (err == VE_OK)
		{
			Table* maintable = inRequest->RetainTableParam(base, err);
			if (maintable != nil)
			{
				Selection* sel = inRequest->RetainSelectionParam(base, maintable);
				if (sel != nil)
				{
					RecIDType FromRecIndex, ToRecIndex;
					FromRecIndex = inRequest->GetLongParam(err);
					if (err == VE_OK)
						ToRecIndex = inRequest->GetLongParam(err);
					sLONG nb = inRequest->GetLongParam(err);
					vector<uBYTE> wayoflocking;
					if (err == VE_OK)
					{
						wayoflocking.resize(nb);
						if (nb > 0)
							err = inRequest->GetInputStream()->GetData(&wayoflocking[0], nb);
					}

					if (err == VE_OK)
					{
						FieldsForCache ffc(maintable);
						err = ffc.ReadFromStream(inRequest->GetInputStream());
						if (err == VE_OK)
							err = ffc.BuildRelationsOnServer(context, wayoflocking);
						if (err == VE_OK)
						{
							VPtrStream tempstream;
							tempstream.SetNeedSwap(inRequest->GetInputStream()->NeedSwap());
							tempstream.OpenWriting();
							for (sLONG i = FromRecIndex; i <= ToRecIndex && err == VE_OK; i++)
							{
								FicheInMem* mainrec = nil;
								sLONG recnum = sel->GetFic(i);
								if (recnum >=0 )
								{
									mainrec = maintable->GetDF()->LoadRecord(recnum, err, DB4D_Do_Not_Lock, context, false);
								}
								if (err == VE_OK)
								{
									if (mainrec != nil)
									{
										err = tempstream.PutByte('+');
										err = ffc.FillStreamWithData(mainrec, &tempstream, context);

										mainrec->Release();
									}
									else
										err = tempstream.PutByte('-');
								}
							}
							if (err == VE_OK)
								err = tempstream.PutByte('.');
							tempstream.CloseWriting();
							if (err == VE_OK)
							{
								alreadysend = true;
								inRequest->InitReply(0);
								inRequest->PutUpdatedInfo(base, context);
								inRequest->GetOutputStream()->PutData(tempstream.GetDataPtr(), tempstream.GetDataSize());
							}
						}
					}
					sel->Release();
				}
				maintable->Release();
			}
			alreadysend = true;
			inRequest->InitReply(0);

		}
		base->Release();
	}

	if (!alreadysend)
		SendError(inRequest, inContext, err);
}



void VDBMgr::ExecSetFullyDeleteRecords(IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	BaseTaskInfo* context = ConvertContext(inContext);
	Base4D* base = inRequest->RetainBaseParam( err);
	if (base != nil)
	{
		Table* tt = inRequest->RetainTableParam(base, err);
		if (tt != nil)
		{
			Boolean fullydeletestate = inRequest->GetBooleanParam(err);
			if (err == VE_OK)
			{
				ObjLocker locker(inContext, tt);
				if (locker.CanWork())
				{
					err = tt->SetFullyDeleteRecords(fullydeletestate);
				}
				else
					err = ThrowBaseError(VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT, noaction);
				if (err == VE_OK)
				{
					alreadysend = true;
					inRequest->InitReply(0);
				}
			}
			tt->Release();
		}
		base->Release();
	}

	if (!alreadysend)
		SendError(inRequest, inContext, err);
}


void VDBMgr::ExecSetTableSchema( IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	
	Base4D *base = inRequest->RetainBaseParam( err);
	if (err == VE_OK && base != NULL)
	{
		Table *table = inRequest->RetainTableParam( base, err);
		if (err == VE_OK && table != NULL)
		{
			DB4D_SchemaID schemaID = inRequest->GetLongParam( err);
			if (err == VE_OK)
			{
				ObjLocker locker( inContext, table);
				if (locker.CanWork())
				{
					err = table->SetSchema( schemaID, inContext);
					if (err == VE_OK)
					{
						inRequest->InitReply(0);
						alreadysend = true;
					}					
				}
				else
					err = base->ThrowError( VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT, noaction);
			}
		}
		ReleaseRefCountable( &table);
	}
	ReleaseRefCountable( &base);

	if (!alreadysend)
		SendError( inRequest, inContext, err);
}


void VDBMgr::ExecTruncateTable(IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	BaseTaskInfo* context = ConvertContext(inContext);
	Base4D* base = inRequest->RetainBaseParam( err);
	if (base != nil)
	{
		err = inRequest->GetThingsToForget( this, context);
		if (err == VE_OK)
		{
			Table* tt = inRequest->RetainTableParam(base, err);
			if (tt != nil)
			{
				VDB4DProgressIndicator* progress = inRequest->RetainProgressParam(inContext);
				bool mustunlock = false;
				err = tt->Truncate(context, progress, false, mustunlock);
				if (err == VE_OK)
				{
					alreadysend = true;
					inRequest->InitReply(0);
					inRequest->PutUpdatedInfo(tt, context);
				}
				tt->Release();
				QuickReleaseRefCountable(progress);
			}
		}
		base->Release();
	}

	if (!alreadysend)
		SendError(inRequest, inContext, err);
}


void VDBMgr::ExecExecuteColumnFormulas(IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	BaseTaskInfo* context = ConvertContext(inContext);
	Base4D* base = inRequest->RetainBaseParam( err);
	if (base != nil)
	{
		err = inRequest->GetThingsToForget( this, context);
		if (err == VE_OK)
		{
			ColumnFormulas* formulas = inRequest->GetColumnFormulasParam(base, err);
			if (formulas != nil)
			{
				Selection* sel = inRequest->RetainSelectionParam(base, formulas->GetTarget());
				if (sel != nil)
				{
					VDB4DProgressIndicator* progress = inRequest->RetainProgressParam(inContext);
					err = formulas->Execute(sel, context, progress, (FicheInMem*)-1);
					if (err == VE_OK)
					{
						alreadysend = true;
						inRequest->InitReply(0);
						inRequest->PutUpdatedInfo(formulas->GetTarget(), context);
						inRequest->PutColumnFormulasResultReply(formulas);
					}
					sel->Release();
					QuickReleaseRefCountable(progress);
				}
				delete formulas;
			}
		}
		base->Release();
	}

	if (!alreadysend)
		SendError(inRequest, inContext, err);
}


void VDBMgr::ExecDelRecFromSel(IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	BaseTaskInfo* context = ConvertContext(inContext);
	Base4D* base = inRequest->RetainBaseParam( err);
	if (base != nil)
	{
		err = inRequest->GetThingsToForget( this, context);
		if (err == VE_OK)
		{
			Selection* sel = inRequest->RetainSelectionParam(base, nil);
			if (sel != nil)
			{
				sLONG n = inRequest->GetLongParam(err);
				if (err == VE_OK)
				{
					err = sel->DelFromSel(n);
					if (err == VE_OK)
					{
						alreadysend = true;
						inRequest->InitReply(0);
						inRequest->PutLongReply(sel->GetQTfic());
						sel->PutClearCacheInfo(inRequest->GetOutputStream());
					}
				}
				sel->Release();
			}
		}
		base->Release();
	}

	if (!alreadysend)
		SendError(inRequest, inContext, err);
}


void VDBMgr::ExecDelSetFromSel(IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	BaseTaskInfo* context = ConvertContext(inContext);
	Base4D* base = inRequest->RetainBaseParam( err);
	if (base != nil)
	{
		err = inRequest->GetThingsToForget( this, context);
		if (err == VE_OK)
		{
			Selection* sel = inRequest->RetainSelectionParam(base, nil);
			if (sel != nil)
			{
				Bittab* b = inRequest->RetainSetParam(base, nil);
				if (b != nil)
				{
					err = sel->DelFromSel(b);
					if (err == VE_OK)
					{
						alreadysend = true;
						inRequest->InitReply(0);
						inRequest->PutLongReply(sel->GetQTfic());
						sel->PutClearCacheInfo(inRequest->GetOutputStream());
					}
					b->Release();
				}
				sel->Release();
			}
		}
		base->Release();
	}

	if (!alreadysend)
		SendError(inRequest, inContext, err);
}


void VDBMgr::ExecDelRangeFromSel(IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK, err2;
	Boolean alreadysend = false;
	BaseTaskInfo* context = ConvertContext(inContext);
	Base4D* base = inRequest->RetainBaseParam( err);
	if (base != nil)
	{
		err = inRequest->GetThingsToForget( this, context);
		if (err == VE_OK)
		{
			Selection* sel = inRequest->RetainSelectionParam(base, nil);
			if (sel != nil)
			{
				sLONG n1 = inRequest->GetLongParam(err);
				sLONG n2 = inRequest->GetLongParam(err2);
				if (err == VE_OK && err2 == VE_OK)
				{
					err = sel->RemoveSelectedRange(n1, n2, context);
					if (err == VE_OK)
					{
						alreadysend = true;
						inRequest->InitReply(0);
						inRequest->PutLongReply(sel->GetQTfic());
						sel->PutClearCacheInfo(inRequest->GetOutputStream());
					}
				}
				sel->Release();
			}
		}
		base->Release();
	}

	if (!alreadysend)
		SendError(inRequest, inContext, err);
}


void VDBMgr::ExecAddRecIDToSel(IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	BaseTaskInfo* context = ConvertContext(inContext);
	Base4D* base = inRequest->RetainBaseParam( err);
	if (base != nil)
	{
		err = inRequest->GetThingsToForget( this, context);
		if (err == VE_OK)
		{
			Selection* sel = inRequest->RetainSelectionParam(base, nil);
			if (sel != nil)
			{
				sLONG n = inRequest->GetLongParam(err);
				if (err == VE_OK)
				{
					Selection* newsel = sel->AddToSelection(n, err);
					if (err == VE_OK)
					{
						alreadysend = true;
						inRequest->InitReply(0);
						if (newsel == sel && sel->MustBeKeptOnServer())
						{
							inRequest->PutBooleanReply(true);
							inRequest->PutLongReply(sel->GetQTfic());
							sel->PutClearCacheInfo(inRequest->GetOutputStream());
						}
						else
						{
							inRequest->PutBooleanReply(false);
							inRequest->PutSelectionReply(newsel, inContext);
						}
					}
				}
				sel->Release();
			}
		}
		base->Release();
	}

	if (!alreadysend)
		SendError(inRequest, inContext, err);
}

void VDBMgr::ExecReduceSel(IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	BaseTaskInfo* context = ConvertContext(inContext);
	Base4D* base = inRequest->RetainBaseParam( err);
	if (base != nil)
	{
		err = inRequest->GetThingsToForget( this, context);
		if (err == VE_OK)
		{
			Selection* sel = inRequest->RetainSelectionParam(base, nil);
			if (sel != nil)
			{
				sLONG n = inRequest->GetLongParam(err);
				if (err == VE_OK)
				{
					sel->ReduceSel(n);
					alreadysend = true;
					inRequest->InitReply(0);
					inRequest->PutLongReply(sel->GetQTfic());
					sel->PutClearCacheInfo(inRequest->GetOutputStream());
				}
				sel->Release();
			}
		}
		base->Release();
	}

	if (!alreadysend)
		SendError(inRequest, inContext, err);
}



void VDBMgr::ExecActivateAllAutomaticRelations(IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	BaseTaskInfo* context = ConvertContext(inContext);
	Base4D* base = inRequest->RetainBaseParam( err);
	if (base != nil)
	{
		err = inRequest->GetThingsToForget( this, context);
		if (err == VE_OK)
		{
			Table* tt = inRequest->RetainTableParam(base, err);
			if (tt != nil)
			{
				FicheInMem* rec = inRequest->RetainFicheInMemParam(base, tt, context, err);
				if (rec != nil)
				{
					sLONG nb = inRequest->GetLongParam(err);
					if (err == VE_OK)
					{
						vector<uBYTE> wayoflocking;
						wayoflocking.resize(nb);
						if (nb > 0)
							err = inRequest->GetInputStream()->GetData(&wayoflocking[0], nb);
						if (err == VE_OK)
						{
							vector<CachedRelatedRecord> result, result2;
							err = tt->ActivateAutomaticRelations_N_To_1(rec, result, context, wayoflocking);
							if (err == VE_OK)
								err = tt->ActivateAutomaticRelations_1_To_N(rec, result2, context, wayoflocking, nil, false, true, false);


							if (err == VE_OK)
							{
								alreadysend = true;
								inRequest->InitReply(0);
								inRequest->PutUpdatedInfo(tt, context);
								inRequest->PutLongReply((sLONG)(result.size()+result2.size()));
								for (vector<CachedRelatedRecord>::iterator cur = result.begin(), end = result.end(); cur != end; cur++)
								{
									inRequest->PutLongReply(cur->GetTableNum());
									if (cur->GetRecord() == nil)
										inRequest->PutByteReply('-');
									else
									{
										inRequest->PutByteReply('+');
										inRequest->PutFicheInMemReply(VImpCreator<VDB4DRecord>::GetImpObject(cur->GetRecord())->GetRec(), inContext);
									}
									inRequest->PutSelectionReply(VImpCreator<VDB4DSelection>::GetImpObject(cur->GetSelection())->GetSel(), inContext);
								}
								for (vector<CachedRelatedRecord>::iterator cur = result2.begin(), end = result2.end(); cur != end; cur++)
								{
									inRequest->PutLongReply(cur->GetTableNum());
									if (cur->GetRecord() == nil)
										inRequest->PutByteReply('-');
									else
									{
										inRequest->PutByteReply('+');
										inRequest->PutFicheInMemReply(VImpCreator<VDB4DRecord>::GetImpObject(cur->GetRecord())->GetRec(), inContext);
									}
									inRequest->PutSelectionReply(VImpCreator<VDB4DSelection>::GetImpObject(cur->GetSelection())->GetSel(), inContext);
								}
							}
						}
					}
					rec->Release();
				}
				tt->Release();
			}
		}
		base->Release();
	}

	if (!alreadysend)
		SendError(inRequest, inContext, err);
}


void VDBMgr::ExecActivateAutomaticRelations_N_To_1(IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	BaseTaskInfo* context = ConvertContext(inContext);
	Base4D* base = inRequest->RetainBaseParam( err);
	if (base != nil)
	{
		err = inRequest->GetThingsToForget( this, context);
		if (err == VE_OK)
		{
			Table* tt = inRequest->RetainTableParam(base, err);
			if (tt != nil)
			{
				FicheInMem* rec = inRequest->RetainFicheInMemParam(base, tt, context, err);
				if (rec != nil)
				{
					sLONG nb = inRequest->GetLongParam(err);
					if (err == VE_OK)
					{
						vector<uBYTE> wayoflocking;
						wayoflocking.resize(nb);
						if (nb > 0)
							err = inRequest->GetInputStream()->GetData(&wayoflocking[0], nb);
						if (err == VE_OK)
						{
							vector<CachedRelatedRecord> result;
							err = tt->ActivateAutomaticRelations_N_To_1(rec, result, context, wayoflocking);

							if (err == VE_OK)
							{
								alreadysend = true;
								inRequest->InitReply(0);
								inRequest->PutUpdatedInfo(tt, context);
								inRequest->PutLongReply((sLONG)result.size());
								for (vector<CachedRelatedRecord>::iterator cur = result.begin(), end = result.end(); cur != end; cur++)
								{
									inRequest->PutLongReply(cur->GetTableNum());
									if (cur->GetRecord() == nil)
										inRequest->PutByteReply('-');
									else
									{
										inRequest->PutByteReply('+');
										inRequest->PutFicheInMemReply(VImpCreator<VDB4DRecord>::GetImpObject(cur->GetRecord())->GetRec(), inContext);
									}
									inRequest->PutSelectionReply(VImpCreator<VDB4DSelection>::GetImpObject(cur->GetSelection())->GetSel(), inContext);
								}
							}
						}
					}
					rec->Release();
				}
				tt->Release();
			}
		}
		base->Release();
	}

	if (!alreadysend)
		SendError(inRequest, inContext, err);
}


void VDBMgr::ExecActivateAutomaticRelations_1_To_N(IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	BaseTaskInfo* context = ConvertContext(inContext);
	Base4D* base = inRequest->RetainBaseParam( err);
	if (base != nil)
	{
		err = inRequest->GetThingsToForget( this, context);
		if (err == VE_OK)
		{
			Table* tt = inRequest->RetainTableParam(base, err);
			if (tt != nil)
			{
				FicheInMem* rec = inRequest->RetainFicheInMemParam(base, tt, context, err);
				if (rec != nil)
				{
					sLONG nb = inRequest->GetLongParam(err);
					if (err == VE_OK)
					{
						vector<uBYTE> wayoflocking;
						wayoflocking.resize(nb);
						if (nb > 0)
							err = inRequest->GetInputStream()->GetData(&wayoflocking[0], nb);
						if (err == VE_OK)
						{
							Field* onField = nil;
							Boolean ononefieldonly = inRequest->GetBooleanParam(err);
							if (err == VE_OK && ononefieldonly)
								onField = inRequest->RetainFieldParam(tt, err);
							if (err == VE_OK)
							{
								Boolean oldvalues = inRequest->GetBooleanParam(err);
								if (err == VE_OK)
								{
									Boolean automaticonly = inRequest->GetBooleanParam(err);
									if (err == VE_OK)
									{		
										Boolean onelevelonly = inRequest->GetBooleanParam(err);
										if (err == VE_OK)
										{		
											vector<CachedRelatedRecord> result;
											err = tt->ActivateAutomaticRelations_1_To_N(rec, result, context, wayoflocking, onField, oldvalues, automaticonly, onelevelonly);

											if (err == VE_OK)
											{
												alreadysend = true;
												inRequest->InitReply(0);
												inRequest->PutUpdatedInfo(tt, context);
												inRequest->PutLongReply((sLONG)result.size());
												for (vector<CachedRelatedRecord>::iterator cur = result.begin(), end = result.end(); cur != end; cur++)
												{
													inRequest->PutLongReply(cur->GetTableNum());
													if (cur->GetRecord() == nil)
														inRequest->PutByteReply('-');
													else
													{
														inRequest->PutByteReply('+');
														inRequest->PutFicheInMemReply(VImpCreator<VDB4DRecord>::GetImpObject(cur->GetRecord())->GetRec(), inContext);
													}
													inRequest->PutSelectionReply(VImpCreator<VDB4DSelection>::GetImpObject(cur->GetSelection())->GetSel(), inContext);
												}
											}
										}
									}
								}
							}
						}
					}
					rec->Release();
				}
				tt->Release();
			}
		}
		base->Release();
	}

	if (!alreadysend)
		SendError(inRequest, inContext, err);
}


void VDBMgr::ExecFillSetFromArrayOfBits(IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	BaseTaskInfo* context = ConvertContext(inContext);
	Base4D* base = inRequest->RetainBaseParam( err);
	if (base != nil)
	{
		err = inRequest->GetThingsToForget( this, context);
		if (err == VE_OK)
		{
			Table* tt = inRequest->RetainTableParam(base, err);
			if (tt != nil)
			{
				Bittab* set = inRequest->RetainSetParam(base, tt);
				if (set != nil)
				{
					sLONG nbelem = inRequest->GetLongParam(err);
					if (err == VE_OK)
					{
						void* p = VObject::GetMainMemMgr()->Malloc(nbelem/8+8, false, 'bits');
						if (p == nil)
							err = ThrowBaseError(memfull);
						else
						{
							sLONG nblong = (nbelem+31)/32;
							err = inRequest->GetInputStream()->GetLongs((sLONG*)p, &nblong);
							if (err == VE_OK)
							{
								err = set->FillFromArrayOfBits(p, nbelem);
								if (err == VE_OK)
								{
									sLONG nbrec = tt->GetDF()->GetMaxRecords(context);
									err = set->Reduit(nbrec);
									alreadysend = true;
									inRequest->InitReply(0);
									inRequest->PutLongReply(set->Compte());
									set->PutClearCacheInfo(inRequest->GetOutputStream());
								}
							}
							VObject::GetMainMemMgr()->Free(p);
						}
					}
					set->Release();
				}
				tt->Release();
			}
		}
		base->Release();
	}

	if (!alreadysend)
		SendError(inRequest, inContext, err);
}


void VDBMgr::ExecFillSetFromArrayOfLongs(IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	BaseTaskInfo* context = ConvertContext(inContext);
	Base4D* base = inRequest->RetainBaseParam( err);
	if (base != nil)
	{
		err = inRequest->GetThingsToForget( this, context);
		if (err == VE_OK)
		{
			Table* tt = inRequest->RetainTableParam(base, err);
			if (tt != nil)
			{
				Bittab* set = inRequest->RetainSetParam(base, tt);
				if (set != nil)
				{
					sLONG nbelem = inRequest->GetLongParam(err);
					if (err == VE_OK)
					{
						void* p = VObject::GetMainMemMgr()->Malloc(nbelem*sizeof(sLONG) + 8, false, 'long');
						if (p == nil)
							err = ThrowBaseError(memfull);
						else
						{
							sLONG nblong = nbelem;
							err = inRequest->GetInputStream()->GetLongs((sLONG*)p, &nblong);
							if (err == VE_OK)
							{
								err = set->FillFromArray((const char*)p, sizeof(sLONG), nbelem, tt->GetDF()->GetMaxRecords(context), false);
								if (err == VE_OK)
								{
									sLONG nbrec = tt->GetDF()->GetMaxRecords(context);
									err = set->Reduit(nbrec);
									alreadysend = true;
									inRequest->InitReply(0);
									inRequest->PutLongReply(set->Compte());
									set->PutClearCacheInfo(inRequest->GetOutputStream());
								}
							}
							VObject::GetMainMemMgr()->Free(p);
						}
					}
					set->Release();
				}
				tt->Release();
			}
		}
		base->Release();
	}

	if (!alreadysend)
		SendError(inRequest, inContext, err);
}



void VDBMgr::ExecFillArrayOfBitsFromSet(IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	BaseTaskInfo* context = ConvertContext(inContext);
	Base4D* base = inRequest->RetainBaseParam( err);
	if (base != nil)
	{
		err = inRequest->GetThingsToForget( this, context);
		if (err == VE_OK)
		{
			Table* tt = inRequest->RetainTableParam(base, err);
			if (tt != nil)
			{
				Bittab* set = inRequest->RetainSetParam(base, tt);
				if (set != nil)
				{
					sLONG maxelem = inRequest->GetLongParam(err);
					if (err == VE_OK)
					{
						void* p = VObject::GetMainMemMgr()->Malloc(maxelem/8+8, false, 'bits');
						if (p == nil)
							err = ThrowBaseError(memfull);
						else
						{
							err = set->FillArrayOfBits(p, maxelem);
							if (err == VE_OK)
							{
								sLONG nblong = (maxelem+31)/32;
								alreadysend = true;
								inRequest->InitReply(0);
								inRequest->PutUpdatedInfo(tt, context);
								inRequest->PutLongReply(maxelem);
								inRequest->GetOutputStream()->PutLongs((sLONG*)p, nblong);
							}
							VObject::GetMainMemMgr()->Free(p);
						}
					}
					set->Release();
				}
				tt->Release();
			}
		}
		base->Release();
	}

	if (!alreadysend)
		SendError(inRequest, inContext, err);
}



void VDBMgr::ExecFillArrayOfLongsFromSet(IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	BaseTaskInfo* context = ConvertContext(inContext);
	Base4D* base = inRequest->RetainBaseParam( err);
	if (base != nil)
	{
		err = inRequest->GetThingsToForget( this, context);
		if (err == VE_OK)
		{
			Table* tt = inRequest->RetainTableParam(base, err);
			if (tt != nil)
			{
				Bittab* set = inRequest->RetainSetParam(base, tt);
				if (set != nil)
				{
					sLONG maxelem = inRequest->GetLongParam(err);
					if (err == VE_OK)
					{
						void* p = VObject::GetMainMemMgr()->Malloc(maxelem*sizeof(sLONG)+8, false, 'long');
						if (p == nil)
							err = ThrowBaseError(memfull);
						else
						{
							err = set->FillArray((sLONG*)p, maxelem);
							if (err == VE_OK)
							{
								sLONG nblong = maxelem;
								alreadysend = true;
								inRequest->InitReply(0);
								inRequest->PutUpdatedInfo(tt, context);
								inRequest->PutLongReply(maxelem);
								inRequest->GetOutputStream()->PutLongs((sLONG*)p, nblong);
							}
							VObject::GetMainMemMgr()->Free(p);
						}
					}
					set->Release();
				}
				tt->Release();
			}
		}
		base->Release();
	}

	if (!alreadysend)
		SendError(inRequest, inContext, err);
}



void VDBMgr::ExecSyncThingsToForget(IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	BaseTaskInfoPtr context = ConvertContext(inContext);
	Base4D* base = inRequest->RetainBaseParam( err);
	if (base != nil)
	{
		err = inRequest->GetThingsToForget( this, context);
		if (err == VE_OK)
		{
			inRequest->InitReply(0);
			inRequest->PutUpdatedInfo(base, context);

			alreadysend = true;
		}
		base->Release();
	}

	if (!alreadysend)
		SendError(inRequest, inContext, err);
}



/*
void VDBMgr::Exec ????(IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	BaseTaskInfo* context = ConvertContext(inContext);
	Base4D* base = inRequest->RetainBaseParam( err);
	if (base != nil)
	{
		err = inRequest->GetThingsToForget( this, context);
		if (err == VE_OK)
		{
			alreadysend = true;
			inRequest->InitReply(0);

		}
		base->Release();
	}

	if (!alreadysend)
		SendError(inRequest, inContext, err);
}
*/


void VDBMgr::ExecTestServer(IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	Base4D* base = inRequest->RetainBaseParam( err);
	if (base != nil)
	{
		inRequest->InitReply(0);
		Table* tt = base->FindAndRetainTableRef(L"t1");
		if (tt != nil)
		{
			BaseTaskInfo* context = ConvertContext(inContext);
			DataTable* df = tt->GetDF();
			for (sLONG i = 0; i < 1000000 && err == VE_OK; i++)
			{
				FicheInMem* rec = df->NewRecord(err, context);
				if (rec != nil)
				{
					ValPtr cv = rec->GetNthField(1, err);
					if (cv != nil)
						cv->FromLong(i*i);
					rec->Touch(1);

					cv = rec->GetNthField(2, err);
					if (cv != nil)
						cv->FromLong(i);
					rec->Touch(2);

					cv = rec->GetNthField(3, err);
					if (cv != nil)
						cv->FromLong(i);
					rec->Touch(3);

					err = df->SaveRecord(rec, context);
					rec->Release();
				}
			}
			tt->Release();
		}
		base->Release();
	}

	if (err != VE_OK)
	{
		if (!alreadysend)
			SendError(inRequest, inContext, err);
	}
}

void VDBMgr::ExecAskForASelectionPart(IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	Base4D* base = inRequest->RetainBaseParam( err);
	if (base != nil)
	{
		Selection* sel = inRequest->RetainSelectionParam(base, nil);
		if (sel != nil)
		{
			sLONG numpart = inRequest->GetLongParam(err);
			if (err == VE_OK)
			{
				inRequest->InitReply(0);
				err = sel->GetPartReply(numpart, inRequest);
				if (err == VE_OK)
					alreadysend = true;
			}
			sel->Release();
		}
		base->Release();
	}
	if (!alreadysend)
		SendError(inRequest, inContext, err);

}


void VDBMgr::ExecGetListOfDeadTables(IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	Base4D* base = inRequest->RetainBaseParam( err);
	if (base != nil)
	{
		vector<VString> tnames;
		vector<VUUID> tids;
		VStream* outstream = inRequest->GetOutputStream();
		err = base->GetListOfDeadTables(tnames, tids, inContext);
		if (err == VE_OK)
		{
			inRequest->InitReply(0);
			alreadysend = true;
			sLONG nb = (sLONG)tnames.size();
			inRequest->PutLongReply(nb);
			for (sLONG i = 0; i < nb; i++)
			{
				tnames[i].WriteToStream(outstream);
				tids[i].WriteToStream(outstream);
			}
		}
	}
	QuickReleaseRefCountable(base);
	if (!alreadysend)
		SendError(inRequest, inContext, err);
}


void VDBMgr::ExecResurectTable(IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	Base4D* base = inRequest->RetainBaseParam( err);
	if (base != nil)
	{
		VString tName;
		err = inRequest->GetStringParam(tName);
		if (err == VE_OK)
		{
			err = base->ResurectDataTable(tName, inContext);
			if (err == VE_OK)
			{
				alreadysend = true;
				inRequest->InitReply(0);
				inRequest->PutLongReply(0);
			}
		}
	}
	QuickReleaseRefCountable(base);
	if (!alreadysend)
		SendError(inRequest, inContext, err);
}


void VDBMgr::ExecGetTableFragmentation(IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	Boolean alreadysend = false;
	Base4D* base = inRequest->RetainBaseParam( err);
	if (base != nil)
	{
		Table* tt = inRequest->RetainTableParam(base, err);
		if (tt != nil)
		{
			sLONG8 totalrec, frags;
			err = tt->GetFragmentation(totalrec, frags, inContext);
			if (err == VE_OK)
			{
				alreadysend = true;
				inRequest->InitReply(0);
				inRequest->PutLong8Reply(totalrec);
				inRequest->PutLong8Reply(frags);
			}
		}
		QuickReleaseRefCountable(tt);
	}
	QuickReleaseRefCountable(base);
	if (!alreadysend)
		SendError(inRequest, inContext, err);
}



bool VDBMgr::AddKeptDataSet(DataSet* inDataSet)
{
	bool result = true;
	VTaskLock lock(&fKeptDataSetsMutex);
	try
	{
		fKeptDataSets[inDataSet->GetID().GetBuffer()] = inDataSet;
		inDataSet->Retain();
	}
	catch (...)
	{
		result = false;
	}
	return result;
}


void VDBMgr::ReleaseKeptDataSet(const VUUID& inID)
{
	VTaskLock lock(&fKeptDataSetsMutex);
	mapOfDataSet::iterator found = fKeptDataSets.find(inID.GetBuffer());
	if (found != fKeptDataSets.end())
	{
		found->second->Release();
		fKeptDataSets.erase(found);
	}
}


void VDBMgr::GetKeptDataSetsInfo(bool minimum, VValueBag& outBag)
{
	VTaskLock lock(&fKeptDataSetsMutex);
	outBag.SetLong8(L"entitySetCount", fKeptDataSets.size());
	if (!minimum)
	{
		for (mapOfDataSet::iterator cur = fKeptDataSets.begin(), end = fKeptDataSets.end(); cur != end; cur++)
		{
			BagElement subbag(outBag, L"entitySet");
			DataSet* ds = cur->second;
			ds->GetInfo(*subbag);
		}
	}

}

DataSet* VDBMgr::RetainKeptDataSet(const VUUID& inID)
{
	VTaskLock lock(&fKeptDataSetsMutex);
	mapOfDataSet::iterator found = fKeptDataSets.find(inID.GetBuffer());
	if (found != fKeptDataSets.end())
	{
		DataSet* result = found->second;
		result->ResetTimer(VSystem::GetCurrentTime());
		result->Retain();
		return result;
	}
	else
		return nil;
}


void VDBMgr::PurgeExpiredDataSets()
{
	uLONG curtime = VSystem::GetCurrentTime();
	if (DiffTime(curtime, fLastDataSetPurge) > 10000)
	{
		vector<DataSet*> toRelease;
		VTaskLock lock(&fKeptDataSetsMutex);
		fLastDataSetPurge = curtime;
		for (mapOfDataSet::iterator cur = fKeptDataSets.begin(), end = fKeptDataSets.end(); cur != end; cur++)
		{
			if (cur->second->Expired(curtime))
				toRelease.push_back(cur->second);
		}

		for (vector<DataSet*>::iterator cur = toRelease.begin(), end = toRelease.end(); cur != end; cur++)
		{
			DataSet* curset = *cur;
			fKeptDataSets.erase(curset->GetID().GetBuffer());
			curset->Release();
		}

	}
}


bool DataSetPurger::FreeMem(sLONG allocationBlockNumber, VSize combien, VSize& outSizeFreed)
{
	VDBMgr::GetManager()->PurgeDataSetsForMem(allocationBlockNumber, combien, outSizeFreed);
	return true;
}


inline Boolean DataSetLess(DataSet* d1, DataSet* d2)
{
	return d1->GetExpirationTime() < d2->GetExpirationTime();
}


void VDBMgr::PurgeDataSetsForMem(sLONG allocationBlockNumber, VSize combien, VSize& outSizeFreed)
{
	{
		outSizeFreed = 0;
#if 0
		// temporary disable dataset purge all

		VTaskLock lock(&fKeptDataSetsMutex);
		vector<DataSet*> toRelease;
		toRelease.reserve(fKeptDataSets.size());
		for (mapOfDataSet::iterator cur = fKeptDataSets.begin(), end = fKeptDataSets.end(); cur != end; cur++)
		{
			toRelease.push_back(cur->second);
		}

		//sort(toRelease.begin(), toRelease.end(), DataSetLess); // ce n'est plus necessaire

		for (vector<DataSet*>::iterator cur = toRelease.begin(), end = toRelease.end(); cur != end; cur++)
		{
			DataSet* curset = *cur;
			if (curset->GetSel()->MatchAllocationNumber(allocationBlockNumber))
			{
				VSize sizesel = curset->GetSel()->CalcLenOnDisk();
				outSizeFreed += sizesel;
				fKeptDataSets.erase(curset->GetID().GetBuffer());
				curset->Release();
			}
		}
#endif
	}
}


VError VDBMgr::StoreStructuresAsXML()
{
	vector<Base4D*> bases;
	{
		VTaskLock lock(&fStoreAsXMLMutex);
		for (set<Base4D*>::iterator cur = fBasesToStoreAsXML.begin(), end = fBasesToStoreAsXML.end(); cur != end; cur++)
		{
			(*cur)->Retain();
			bases.push_back(*cur);
		}
		fBasesToStoreAsXML.clear();
	}

	for (vector<Base4D*>::iterator cur = bases.begin(), end = bases.end(); cur != end; cur++)
	{
		StErrorContextInstaller errs(false);
		Base4D* db = *cur;
		VError err = db->SaveStructAsXML();
		db->Release();
	}

	return VE_OK;

}


static void addStaticRequiredJSFile(vector<VFilePath>& outFiles, const VFolder* inFolder, const VString& fileName)
{
	VFile scriptfile(*inFolder, fileName);
	outFiles.push_back(scriptfile.GetPath());
}

void VDBMgr::GetStaticRequiredJSFiles(vector<VFilePath>& outFiles)
{
	outFiles.clear();
	VFolder* compfolder = RetainResourceFolder();
	if (compfolder != nil)
	{
		addStaticRequiredJSFile(outFiles, compfolder, "directoryRest.js");
		addStaticRequiredJSFile(outFiles, compfolder, "ImpExpRest.js");
		addStaticRequiredJSFile(outFiles, compfolder, "reporting.js");
		addStaticRequiredJSFile(outFiles, compfolder, "ModelLoadTime.js");

		/*
		VFile scriptfile(*compfolder, "directoryRest.js");
		outFiles.push_back(scriptfile.GetPath());
		VFile impexpfile(*compfolder, "ImpExpRest.js");
		outFiles.push_back(impexpfile.GetPath());
		*/

		compfolder->Release();
	}
}





	// ------------------------------------------------------------------------------ //





bool VDBMgr::StartCacheLog(const XBOX::VFolder& inRoot, const XBOX::VString& inFileName, const XBOX::VValueBag *inParams)
{
	return VCacheLog::Get()->Start(inRoot, inFileName, inParams);
}

void VDBMgr::StopCacheLog()
{
	VCacheLog::Get()->Stop();
}

bool VDBMgr::IsCacheLogStarted() const
{
	return VCacheLog::IsStarted();
}

void VDBMgr::AppendCommentToCacheLog(const XBOX::VString& inWhat)
{
	VCacheLog::Get()->LogComment(inWhat);
}


void VDBMgr::SetRunningMode( DB4D_Running_Mode inRunningMode)
{
	fRunningMode = inRunningMode;
	InitFileExtensions( fRunningMode);
}


	// ------------------------------------------------------------------------------ //


VGarbageTask::VGarbageTask(VDBMgr* inMgr):VTask(inMgr, 512*1024L, eTaskStylePreemptive, NULL)
{
	fJSGarbageCollectInterval = 10 * 1000; // 10 seconds
	fDBMgr = inMgr;
}


void VGarbageTask::DoInit()
{
	// on attend que notre createur soit completement initialise
	Sleep( 500L);
	fLastJSGarbageCollect = VSystem::GetCurrentTime();
}

void VGarbageTask::DoDeInit()
{
}


Boolean VGarbageTask::DoRun()
{
	Boolean cont = true;

	if( GetState() < TS_DYING ) 
	{
		uLONG currentTime = VSystem::GetCurrentTime();

		if (fJSGarbageCollectInterval != 0 && currentTime-fLastJSGarbageCollect > fJSGarbageCollectInterval || currentTime < fLastJSGarbageCollect)
		{
			IDB4D_ApplicationIntf *applicationIntf = VDBMgr::GetManager()->GetApplicationInterface();
			if (applicationIntf != nil)
			{
				applicationIntf->JS4DGarbageCollect();
			}
			fLastJSGarbageCollect = VSystem::GetCurrentTime();
		}


		Sleep(1000);
	}
	else
		cont = false;

	return cont;
}


// ------------------------------------------------------------------------------ //




VDB4DSignaler::VDB4DSignaler( VDBMgr *inManager):fManager( inManager),
	fSignal_AddBase( VSignal::ESM_Asynchonous),
	fSignal_CloseBase( VSignal::ESM_Asynchonous),
	fSignal_UpdateBase( VSignal::ESM_Asynchonous),

	fSignal_AddTable( VSignal::ESM_Asynchonous),
	fSignal_DelTable( VSignal::ESM_Asynchonous),
	fSignal_UpdateTable( VSignal::ESM_Asynchonous),

	fSignal_AddField( VSignal::ESM_Asynchonous),
	fSignal_DelField( VSignal::ESM_Asynchonous),
	fSignal_UpdateField( VSignal::ESM_Asynchonous),

	fSignal_AddIndex( VSignal::ESM_Asynchonous),
	fSignal_DelIndex( VSignal::ESM_Asynchonous),
	fSignal_UpdateIndex( VSignal::ESM_Asynchonous),

	fSignal_AddRelation( VSignal::ESM_Asynchonous),
 	fSignal_DelRelation( VSignal::ESM_Asynchonous),
 	fSignal_UpdateRelation( VSignal::ESM_Asynchonous)
{
}


DB4DSignaler::VSignal_AddBase*		VDB4DSignaler::GetSignal_AddBase()			{return &fSignal_AddBase;}
DB4DSignaler::VSignal_CloseBase*	VDB4DSignaler::GetSignal_CloseBase()		{return &fSignal_CloseBase;}
DB4DSignaler::VSignal_UpdateBase*	VDB4DSignaler::GetSignal_UpdateBase()		{return &fSignal_UpdateBase;}

DB4DSignaler::VSignal_AddTable*		VDB4DSignaler::GetSignal_AddTable()			{return &fSignal_AddTable;}
DB4DSignaler::VSignal_DelTable*		VDB4DSignaler::GetSignal_DelTable()			{return &fSignal_DelTable;}
DB4DSignaler::VSignal_UpdateTable*	VDB4DSignaler::GetSignal_UpdateTable()		{return &fSignal_UpdateTable;}

DB4DSignaler::VSignal_AddField*		VDB4DSignaler::GetSignal_AddField()			{return &fSignal_AddField;}
DB4DSignaler::VSignal_DelField*		VDB4DSignaler::GetSignal_DelField()			{return &fSignal_DelField;}
DB4DSignaler::VSignal_UpdateField*	VDB4DSignaler::GetSignal_UpdateField()		{return &fSignal_UpdateField;}

DB4DSignaler::VSignal_AddIndex*		VDB4DSignaler::GetSignal_AddIndex()			{return &fSignal_AddIndex;}
DB4DSignaler::VSignal_DelIndex*		VDB4DSignaler::GetSignal_DelIndex()			{return &fSignal_DelIndex;}
DB4DSignaler::VSignal_UpdateIndex*	VDB4DSignaler::GetSignal_UpdateIndex()		{return &fSignal_UpdateIndex;}

DB4DSignaler::VSignal_AddRelation*	VDB4DSignaler::GetSignal_AddRelation()		{return &fSignal_AddRelation;}
DB4DSignaler::VSignal_DelRelation*	VDB4DSignaler::GetSignal_DelRelation()		{return &fSignal_DelRelation;}
DB4DSignaler::VSignal_UpdateRelation*	VDB4DSignaler::GetSignal_UpdateRelation()		{return &fSignal_UpdateRelation;}


void VDB4DSignaler::Tell_AddBase( const XBOX::VUUID& inBaseUUID)
{
	fSignal_AddBase( &inBaseUUID);
}


void VDB4DSignaler::Tell_CloseBase( const XBOX::VUUID& inBaseUUID)
{
	fSignal_CloseBase( &inBaseUUID);
}


void VDB4DSignaler::Tell_UpdateBase( const XBOX::VUUID& inBaseUUID, ETellUpdateBaseOptions inWhat)
{
	fSignal_UpdateBase( &inBaseUUID, inWhat);
}

	
void VDB4DSignaler::Tell_AddTable( const XBOX::VUUID& inBaseUUID, const XBOX::VUUID& inTableUUID, sLONG inTableID)
{
	fSignal_AddTable( &inBaseUUID, &inTableUUID);
}


void VDB4DSignaler::Tell_DelTable( const XBOX::VUUID& inBaseUUID, const XBOX::VUUID& inTableUUID, sLONG inTableID)
{
	fSignal_DelTable( &inBaseUUID, &inTableUUID);
}


void VDB4DSignaler::Tell_UpdateTable( const XBOX::VUUID& inBaseUUID, const XBOX::VUUID& inTableUUID, sLONG inTableID, ETellUpdateTableOptions inWhat)
{
	fSignal_UpdateTable( &inBaseUUID, &inTableUUID, inWhat);
}


void VDB4DSignaler::Tell_AddField( const XBOX::VUUID& inBaseUUID, const XBOX::VUUID& inTableUUID, const XBOX::VUUID& inFieldUUID, sLONG inTableID, sLONG inFieldID)
{
	fSignal_AddField( &inBaseUUID, &inTableUUID, &inFieldUUID);
}


void VDB4DSignaler::Tell_DelField( const XBOX::VUUID& inBaseUUID, const XBOX::VUUID& inTableUUID, const XBOX::VUUID& inFieldUUID, sLONG inTableID, sLONG inFieldID)
{
	fSignal_DelField( &inBaseUUID, &inTableUUID, &inFieldUUID);
}


void VDB4DSignaler::Tell_UpdateField( const XBOX::VUUID& inBaseUUID, const XBOX::VUUID& inTableUUID, const XBOX::VUUID& inFieldUUID, sLONG inTableID, sLONG inFieldID, ETellUpdateFieldOptions inWhat)
{
	fSignal_UpdateField( &inBaseUUID, &inTableUUID, &inFieldUUID, inWhat);
}

	
void VDB4DSignaler::Tell_AddIndex( const XBOX::VUUID& inBaseUUID, const XBOX::VUUID& inIndexUUID)
{
	fSignal_AddIndex( &inBaseUUID, &inIndexUUID);
}


void VDB4DSignaler::Tell_DelIndex( const XBOX::VUUID& inBaseUUID, const XBOX::VUUID& inIndexUUID)
{
	fSignal_DelIndex( &inBaseUUID, &inIndexUUID);
}


void VDB4DSignaler::Tell_UpdateIndex( const XBOX::VUUID& inBaseUUID, const XBOX::VUUID& inIndexUUID, ETellUpdateIndexOptions inWhat)
{
	fSignal_UpdateIndex( &inBaseUUID, &inIndexUUID, inWhat);
}


void VDB4DSignaler::Tell_AddRelation( const XBOX::VUUID& inBaseUUID, const XBOX::VUUID& inRelationID)
{
	fSignal_AddRelation( &inBaseUUID, &inRelationID);
}


void VDB4DSignaler::Tell_DelRelation( const XBOX::VUUID& inBaseUUID, const XBOX::VUUID& inRelationID)
{
	fSignal_DelRelation( &inBaseUUID, &inRelationID);
}


void VDB4DSignaler::Tell_UpdateRelation( const XBOX::VUUID& inBaseUUID, const XBOX::VUUID& inRelationID, ETellUpdateRelationOptions inWhat)
{
	fSignal_UpdateRelation( &inBaseUUID, &inRelationID, inWhat);
}
