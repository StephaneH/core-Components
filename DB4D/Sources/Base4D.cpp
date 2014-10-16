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

// SRV*c:\localstore*http://msdl.microsoft.com/download/symbols
// -memDebugLeaks 0 -memDebugFill 0 -memDebugInfo 0

const sLONG kAddToLogLen = 4 /*Tag*/ + 8 /*Operation#*/+ 4 /*len*/ + 4 /*action*/ + 8 /*ContextID*/ + 8 /*TimeStamp*/ + 4 /*len at the end*/ + 4 /*tag at the end*/;

ReadAheadBuffer::ReadAheadBuffer(Base4D* owner)
{
	fOwner = owner;
	fData = nil;
	fDataLen = 0; 
	fSegment = 1;
	fDataMaxLen = 0;
}

  
ReadAheadBuffer::~ReadAheadBuffer()
{
	fOwner->RemoveReadAheadBuffer(this);
	if (fData != nil)
		FreeFastMem(fData);
}

Boolean ReadAheadBuffer::BuildDataPtr(sLONG size)
{
	Boolean ok = false;
	xbox_assert(fData == nil);
	fData = GetFastMem(size, false, 'rbuf');
	if (fData != nil)
	{
		ok = true;
		fDataMaxLen = size;
	}
	return ok;
}


void ReadAheadBuffer::Lock()
{
	fmutex.Lock();
}

void ReadAheadBuffer::Unlock()
{
	fmutex.Unlock();
}


VError ReadAheadBuffer::ReadFrom(DataAddr4D ou, sLONG len)
{
	VError err;
	fSegment = ou & 63;
	fPos=ou & (-64);

	xbox_assert(len<=fDataMaxLen);
	if (len>fDataMaxLen)
		len = fDataMaxLen;
	err = fOwner->readlong(fData, len, ou, 0, nil);
	if (err == VE_OK)
	{
		fDataLen = len;
	}
	else
	{
		fDataLen = 0;
		fSegment = -1;
	}

	return err;
}


VError ReadAheadBuffer::GetData(void* p, sLONG len, DataAddr4D ou, sLONG offset)
{
	VError err = VE_OK;
	sLONG off = (sLONG) (ou - fPos);
	if (testAssert(off >=0 && off+offset+len<= fDataLen))
	{
		vBlockMove(((uCHAR*)fData) + off + offset, (uCHAR*)p, len);
	}
	else
		err = VE_DB4D_END_OF_BUFFER;
	return err;
}





/* -------------------------------------------------------------------------- */



TreeInMem *StructElemDefTreeInMem::CreTreeInMem()
{
	return new StructElemDefTreeInMem(StructElemDefAccess, FeuilleFinaleContientQuoi);
}


void StructElemDefTreeInMem::DeleteElem( ObjCacheInTree *inObject)
{
	StructElemDef *theElem = (StructElemDef*)inObject;
	
	if (theElem != nil)
		delete theElem;
	
}


TreeInMem *StructElemDefTreeInMemHeader::CreTreeInMem()
{
	return new StructElemDefTreeInMem(ObjCache::StructElemDefAccess, FeuilleFinaleContientQuoi);
}



StructElemDef::StructElemDef(Base4D* owner, sLONG num, DataBaseObjectType signature, sWORD DefaultAccess):ObjCacheInTree(DefaultAccess)
{
	fOwner = owner;
	fNum = num;
	fData = nil;
	fDataLen = 0;
	fAnteLen = 0;
	fNeedSwap = false;
	fNeedSwapWhileSaving = false;
	fSignature = signature;
}

StructElemDef::~StructElemDef()
{
	if (fData != nil)
		FreeFastMem(fData);
}


sLONG StructElemDef::saveobj()
{
	VError err;
	if (!occupe())
	{
#if debuglogwrite
		VString wherefrom(L"Struct Elem Save");
		VString* whx = &wherefrom;
#else
		VString* whx = nil;
#endif

		if (getaddr() != 0)
		{
			DataBaseObjectHeader tag(fData, fDataLen, /*DBOH_StructDefElem*/fSignature, fNum, -1);
			if (fNeedSwapWhileSaving)
				tag.SetSwapWhileWriting(true);
			err = tag.WriteInto(fOwner, getaddr(), whx);
			if (fDataLen>0 && err == VE_OK)
			{
				err = fOwner->writelong(fData, fDataLen,getaddr(),kSizeDataBaseObjectHeader, whx);
			}
		}
		libere();
	}
	return fDataLen+2;
}


VError StructElemDef::loadobj(DataAddr4D xaddr, sLONG len)
{
	VError err = VE_OK;

	if (xaddr != 0)
		setaddr(xaddr);

	if (fData != nil)
		FreeFastMem(fData);
	fData = nil;
	fDataLen = len - kSizeDataBaseObjectHeader;
	fAnteLen = fDataLen;

	if (fDataLen > 0)
	{
		DataBaseObjectHeader tag;
		err = tag.ReadFrom(fOwner, getaddr(), nil);
		if (err == VE_OK)
		{
			err = tag.ValidateTag(fSignature, fNum, -1);
			if (err != VE_OK)
				err = tag.ValidateTag(DBOH_StructDefElem, fNum, -1);
		}
		fNeedSwap = tag.NeedSwap();
		if (err == VE_OK)
		{
			fData = GetFastMem(fDataLen, false, 'sted');
			if (fData == nil)
			{
				err = fOwner->ThrowError(memfull, DBaction_LoadingStructElemDef);
				fDataLen = 0;
			}
			else
			{
				err = fOwner->readlong( fData, fDataLen, getaddr(), kSizeDataBaseObjectHeader);
				if (err == VE_OK)
				{
					err =tag.ValidateCheckSum(fData, fDataLen);
				}
			}
		}
	}

	return err;
}


sLONG StructElemDef::liberemem(sLONG allocationBlockNumber, sLONG combien, uBOOL tout)
{
	sLONG tot = 0;
	if ((GetParent()==nil) || (!GetParent()->occupe()))
	{
		if (fData != nil)
		{
			FreeFastMem(fData);
			fData = nil;
			fDataLen = 0;
		}
		tot = 2+fDataLen;
		if (GetParent()!=nil) 
		{
			GetParent()->DelFromParent(posdansparent, this);
			GetParent()->libere();
		}
	}
	return(tot);
}


void StructElemDef::SetData(void* data, sLONG datalen)
{
	if (fData != nil && fData != data)
		FreeFastMem(fData);
	fData = data;
	fDataLen = datalen;
	//setmodif(true, fOwner, nil);
}


VError StructElemDef::CopyData(void* data, sLONG datalen)
{
	VError err = VE_OK;

	if (datalen == fDataLen)
	{
		if (datalen != 0)
		{
			vBlockMove(data, fData, datalen);
		}
	}
	else
	{
		if (fData != nil && fData != data)
			FreeFastMem(fData);
		if (datalen == 0)
		{
			fData = nil;
			fDataLen = 0;
		}
		else
		{
			fData = GetFastMem(datalen, false, 'sted');
			if (fData == nil)
			{
				err = fOwner->ThrowError(memfull, DBaction_LoadingStructElemDef);
				fDataLen = 0;
			}
			else
			{
				fDataLen = datalen;
				vBlockMove(data, fData, datalen);
			}
		}
	}

	/*
	if (err == VE_OK)
		setmodif(true, fOwner, nil);
	*/

	return err;
}

/* -------------------------------------------------------------------------- */


mapFieldsIDInRecByName::mapFieldsIDInRecByName(Base4D* inOwner):fFieldsID(50)
{
	fOwner = inOwner;
}


VError mapFieldsIDInRecByName::save()
{
	VError err = VE_OK;

	VJSONObject* data = new VJSONObject();
	VJSONArray* fids = new VJSONArray();
	fMutex.Lock();
	data->SetPropertyAsNumber("startingFieldID", fStartingID);
	for (mapFieldsID::iterator cur = fFieldsID.begin(), end = fFieldsID.end(); cur != end; ++cur)
	{
		VJSONObject* item = new VJSONObject();
		item->SetPropertyAsString("fieldRef", cur->first);
		item->SetPropertyAsNumber("fieldID", cur->second);
		fids->Push(VJSONValue(item));
		QuickReleaseRefCountable(item);
	}
	data->SetProperty("FieldIDs", VJSONValue(fids));

	VString s;
	err = VJSONValue(data).Stringify(s);
	fMutex.Unlock();

	OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
	bool needswap;
	ExtraElement* elem = fOwner->RetainExtraElement(kExtraElement_FieldsIDInRec, curstack, err, &needswap);
	if (elem != nil)
	{
		elem->Occupy(curstack, true);
		VMemoryBuffer<>& data = elem->GetData();
		data.Clear();
		data.PutData(0, (const void*)s.GetCPointer(), s.GetLength() * 2);
		elem->save(curstack);
		elem->Free(curstack, true);
		elem->Release();
	}

	QuickReleaseRefCountable(fids);
	QuickReleaseRefCountable(data);

	return err;
}


VError mapFieldsIDInRecByName::load()
{
	VError err = VE_OK;

	OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
	bool needswap;
	ExtraElement* elem = fOwner->RetainExtraElement(kExtraElement_FieldsIDInRec, curstack, err, &needswap);
	fStartingID = 0;

	if (elem != nil)
	{
		VMemoryBuffer<>& data = elem->GetData();
		if (data.GetDataSize() != 0)
		{
			VJSONValue val;
			VString s;
			s.FromBlock(data.GetDataPtr(), data.GetDataSize(), VTC_UTF_16);
			VJSONImporter json(s);
			err = json.Parse(val);
			if (err == VE_OK)
			{
				if (val.IsObject())
				{
					VJSONObject* data = val.GetObject();
					data->GetPropertyAsNumber("startingFieldID", &fStartingID);
					VJSONValue valfids = data->GetProperty("FieldIDs");
					if (valfids.IsArray())
					{
						VJSONArray* fids = valfids.GetArray();
						for (sLONG i = 0, nb = (sLONG)fids->GetCount(); i < nb; ++i)
						{
							VJSONValue valitem = (*fids)[i];
							if (valitem.IsObject())
							{
								VString s;
								sLONG id;
								valitem.GetObject()->GetPropertyAsString("fieldRef", s);
								valitem.GetObject()->GetPropertyAsNumber("fieldID", &id);
								fFieldsID[s] = id;
							}
						}
					}
					
				}
			}
		}
	}

	return err;
}


sLONG mapFieldsIDInRecByName::GetSmallID(const VString& inRef)
{
	VTaskLock lock(&fMutex);
	mapFieldsID::iterator found = fFieldsID.find(inRef);
	if (found == fFieldsID.end())
	{
		++fStartingID;
		fFieldsID[inRef] = fStartingID;
		return fStartingID;
	}
	else
		return found->second;
}




/* -------------------------------------------------------------------------- */

Base4D::Base4D(Base4D* localdb, const VUUID& BaseID, DB4DNetworkManager* netacces, DB4DNetworkManager* legacy_netacces):ObjCache(BaseAccess),fID(true),fTempFolder(NULL),
	fExtra(&hbbloc.ExtraAddr, &hbbloc.ExtraLen, this, -1, -1, DBOH_ExtraDataBaseProperties, true)
{
#if debuglrWithTypObj
	typobj = t_Base4D;
#endif

#if debugLeaksAll
	if (debug_canRegisterLeaksAll)
		RegisterStackCrawl(this);
#endif

	fIsRemote = true;
	fFlushInfo = nil;
	fNetAccess = netacces;
	fLegacy_NetAccess = legacy_netacces;
	fLocalDB = localdb;
	if (fLocalDB != nil)
		fLocalDB->Retain();

	fRemoteMaxRecordsStamp = 0;

	xInit(true);
	fID = BaseID;

}


Base4D::Base4D(VDBFlushMgr* Flusher, bool inNeedFlushInfo):ObjCache(BaseAccess),fID(true),fTempFolder(NULL),
	fExtra(&hbbloc.ExtraAddr, &hbbloc.ExtraLen, this, -1, -1, DBOH_ExtraDataBaseProperties, false)
{
#if debuglrWithTypObj
	typobj = t_Base4D;
#endif

#if debugLeaksAll
	if (debug_canRegisterLeaksAll)
		RegisterStackCrawl(this);
#endif
	
	fFlushInfo = inNeedFlushInfo ? Flusher->NewBaseFlushInfo(this) : nil;
	fFlusher = Flusher;
	fIsRemote = false;
	fNetAccess = nil;
	fLegacy_NetAccess = nil;
	fLocalDB = nil;
	fRemoteMaxRecordsStamp = 1;


	xInit(true);

}

#if VERSIONDEBUG

Boolean debug_OKToWriteIn0 = false;
#define Set_debug_OKToWriteIn0(xset) debug_OKToWriteIn0 = xset

#else

#define Set_debug_OKToWriteIn0(xset)

#endif

Base4D::~Base4D()
{
#if debuglog
	if (fDebugLog != nil)
	{
		fDebugLog->CloseWriting();
		delete fDebugLog;
	}
#endif
//	xbox_assert(fUpdateCounts.size() == 0);

	if (fSyncHelper != nil)
		delete fSyncHelper;

	VDBMgr::GetManager()->FinishUnRegisterBase(this);

#if trackClose
	trackDebugMsg("call CloseBase in Release\n");
#endif						
	CloseBase();

	QuickReleaseRefCountable(fRemoteStructureExtraProp);

	if (fLocalDB != nil)
	{
		fLocalDB->Release();
	}

	ReleaseOutsideCatalogs();

	fTableOfFields->ReleaseAllFields();
	fTableOfTables->ReleaseAllFields();

	fTableOfIndexes->ReleaseAllFields();

	fTableOfIndexCols->ReleaseAllFields();

	fTableOfSchemas->ReleaseAllFields();

	fTableOfConstraints->ReleaseAllFields();
	fTableOfConstraintCols->ReleaseAllFields();

	fTableOfViews->ReleaseAllFields();
	fTableOfViewFields->ReleaseAllFields();

	fTableOfFields->Release();
	fTableOfTables->Release();
	fDataTableOfTables->Release();
	fDataTableOfFields->Release();

	fTableOfIndexes->Release();
	fDataTableOfIndexes->Release();

	fTableOfIndexCols->Release();
	fDataTableOfIndexCols->Release();

	fTableOfConstraints->Release();
	fDataFileOfConstraints->Release();

	fTableOfConstraintCols->Release();
	fDataFileOfConstraintCols->Release();

	fTableOfSchemas->Release();
	fDataTableOfSchemas->Release();

	fTableOfViews->Release();
	fDataTableOfViews->Release();
	fTableOfViewFields->Release();
	fDataTableOfViewFields->Release();

	DisposeSeqNums();

	xbox_assert(fBuffers.GetFirst() == nil);

	// UnRegisterForLang();
	// CloseData(false);
	
	delete IndexSemaphore;

	if (fCloseSyncEvent != nil)
	{
		fCloseSyncEvent->Unlock();
		fCloseSyncEvent->Release();
	}
	ReleaseRefCountable( &fTempFolder);

	ReleaseRefCountable( &fIntlMgr);

	QuickReleaseRefCountable(fStructFolder);

	QuickReleaseRefCountable(fEntityCatalog);
	QuickReleaseRefCountable(fAutoEntityCatalog);

	QuickReleaseRefCountable(fStructXMLFile);

	QuickReleaseRefCountable(fIndexesBag);

	QuickReleaseRefCountable(fUAGDirectory);

	QuickReleaseRefCountable(fCatalogJSFile);

	ReleaseRefCountable(&fLogErrorStack);

	if (fFieldsIDInRec != nil)
		delete fFieldsIDInRec;

	QuickReleaseRefCountable(fExtraElements);

}



#if debuglr
sLONG Base4D::Retain(const char* DebugInfo) const
{
	return IDebugRefCountable::Retain(DebugInfo);
}

sLONG Base4D::Release(const char* DebugInfo) const
{
	return IDebugRefCountable::Release(DebugInfo);
}
#endif


VError Base4D::ThrowError( VError inErrCode, ActionDB4D inAction, const VString* p1) const
{
	if (this != nil)
	{
		VUUID nullid;
		VErrorDB4D_OnBase *err = new VErrorDB4D_OnBase(inErrCode, inAction, this);
		if (p1 != nil)
			err->GetBag()->SetString(Db4DError::Param1, *p1);
		VTask::GetCurrent()->PushRetainedError( err);
	}
	
	return inErrCode;
}


XBOX::VFolder* Base4D::RetainTemporaryFolder( bool inMustExists, VError *outError) const
{
	VError err = VE_OK;
	VFolder *folder = nil;
	if (fIsRemote)
	{
		if (fLocalDB != nil)
		{
			folder = fLocalDB->RetainTemporaryFolder(inMustExists, outError);
		}
		else
		{
			folder = VFolder::RetainSystemFolder( eFK_Temporary, inMustExists);
		}
	}
	else
	{
		VTaskLock lock(&fTempFolderMutex);
		folder = fTempFolder;

		if (folder == nil)
		{
			// that may happen very early while the db is being initialized
			VFilePath x;
			fDefaultLocation.GetFolder(x);
			folder = new VFolder( x);
		}

		if (testAssert( folder != nil))
		{
			folder->Retain();
			// create it at first use.
			// the creation has already been tested in TryTempFolder.
			if (inMustExists && !folder->Exists())
			{
				err = folder->Create();
				if (err != VE_OK)
					ReleaseRefCountable( &folder);
			}
		}
	}

	if (outError != nil)
		*outError = err;

	return folder;
}


void Base4D::Touch() 
{ 
	if (testAssert(!fIsRemote))
		fStamp++;
	if (!fIsClosing && !fIsParsingStruct)
	{
		InvalidateAutoCatalog();
		if (fStoreAsXML)
			VDBMgr::GetManager()->AddStructureToStoreAsXML(this);
	}
}


void Base4D::TouchXML() 
{ 
	if (!fIsClosing && !fIsParsingStruct)
	{
		if (fStoreAsXML)
			VDBMgr::GetManager()->AddStructureToStoreAsXML(this);
	}
}



void Base4D::SetTemporaryFolder( VFolder *inFolder)
{
	if (testAssert( inFolder != nil))
	{
		VTaskLock lock(&fTempFolderMutex);
		CopyRefCountable( &fTempFolder, inFolder);
	}
}


VError Base4D::SetTemporaryFolderSelector( DB4DFolderSelector inSelector, const XBOX::VString *inCustomFolder)
{
	VError err;
	if ( (fStructure == nil) && !fIsRemote )
	{
		// la structure a toujours son dossier temp dans le system
		if (inSelector != DB4D_SystemTemporaryFolder)
			err = vThrowError( VE_DB4D_INVALID_PARAMETER);
		else
			err = VE_OK;
	}
	else
	{
		const VValueBag* old_bag = RetainStructureExtraProperties( err, nil );
		VValueBag *newBag = (old_bag != nil) ? old_bag->Clone() : new VValueBag;
		
		if (newBag != nil)
		{
			VValueBag *tempfolder_bag = newBag->RetainUniqueElement( DB4DBagKeys::temp_folder );
			if (tempfolder_bag == nil)
				tempfolder_bag = new VValueBag;
			if (tempfolder_bag != nil)
			{
				switch( inSelector)
				{
					case DB4D_DataFolder:
						{
							tempfolder_bag->SetString( DB4DBagKeys::folder_selector, CVSTR( "data"));
							tempfolder_bag->RemoveAttribute( DB4DBagKeys::path);
							break;
						}
					
					case DB4D_StructureFolder:
						{
							tempfolder_bag->SetString( DB4DBagKeys::folder_selector, CVSTR( "structure"));
							tempfolder_bag->RemoveAttribute( DB4DBagKeys::path);
							break;
						}
					
					case DB4D_SystemTemporaryFolder:
						{
							tempfolder_bag->SetString( DB4DBagKeys::folder_selector, CVSTR( "system"));
							tempfolder_bag->RemoveAttribute( DB4DBagKeys::path);
							break;
						}
					
					case DB4D_CustomFolder:
						{
							tempfolder_bag->SetString( DB4DBagKeys::folder_selector, CVSTR( "custom"));
							if (testAssert( inCustomFolder != nil))
								tempfolder_bag->SetString( DB4DBagKeys::path, *inCustomFolder);
							break;
						}
				}
				newBag->ReplaceElement( DB4DBagKeys::temp_folder, tempfolder_bag);
				err = SetStructureExtraProperties( newBag, true, nil);
			}
			else
			{
				err = vThrowError( VE_DB4D_MEMFULL);
			}
			ReleaseRefCountable( &tempfolder_bag);
		}
		else
		{
			err = vThrowError( VE_DB4D_MEMFULL);
		}
		ReleaseRefCountable( &old_bag);
		ReleaseRefCountable( &newBag);
	}
	return err;
}


VError Base4D::GetTemporaryFolderSelector( DB4DFolderSelector *outSelector, XBOX::VString& outCustomFolderPath) const
{
	VError err;
	
	outCustomFolderPath.Clear();
	if ( (fStructure == nil) && !fIsRemote )
	{
		// la structure a toujours son dossier temp dans le system
		*outSelector = DB4D_SystemTemporaryFolder;
		err = VE_OK;
	}
	else
	{
		*outSelector = DB4D_DefaultTemporaryFolder;
		const VValueBag* bag = RetainStructureExtraProperties( err, nil );
		if ( bag != NULL )
		{
			const VValueBag *tempfolder_bag = bag->GetUniqueElement( DB4DBagKeys::temp_folder );
			if ( tempfolder_bag != NULL )
			{
				VString selector, path;
				tempfolder_bag->GetString( DB4DBagKeys::folder_selector, selector);

				if (selector.EqualToUSASCIICString( "data"))
				{
					*outSelector = DB4D_DataFolder;
				}
				else if (selector.EqualToUSASCIICString( "structure"))
				{
					*outSelector = DB4D_StructureFolder;
				}
				else if (selector.EqualToUSASCIICString( "system"))
				{
					*outSelector = DB4D_SystemTemporaryFolder;
				}
				else if (testAssert( selector.EqualToUSASCIICString( "custom")))
				{
					*outSelector = DB4D_CustomFolder;
					tempfolder_bag->GetString( DB4DBagKeys::path, outCustomFolderPath);
				}
			}
			ReleaseRefCountable( &bag);
		}
	}
	return err;
}


VError Base4D::TryTempFolder( DB4DFolderSelector inSelector, const VString& inCustomFolderPath, VFolder **outRetainedFolder) const
{
	VError err;
	VFolder *folder = nil;
	VFilePath folderPath;
	bool canDeleteFolder = false;
	switch( inSelector)
	{
		case DB4D_DataFolder:
			{
				if (testAssert( fSegments.GetCount() > 0))
				{
					folderPath = fSegments[0]->GetFile()->GetPath();
					folderPath.ToFolder().ToSubFolder( CVSTR( "temporary files"));
					folder = new VFolder( folderPath);
					canDeleteFolder = true;
					if (folder != nil)
					{
						StErrorContextInstaller errs(false);
						folder->DeleteContents(true);
					}
				}
				break;
			}
		
		case DB4D_StructureFolder:
			{
				if (testAssert( fStructure != nil && fStructure->fSegments.GetCount() > 0))
				{
					folder = fStructure->fSegments[0]->GetFile()->RetainParentFolder();
					folderPath.ToFolder().ToSubFolder( CVSTR( "temporary files"));
					folder = new VFolder( folderPath);
					canDeleteFolder = true;
					if (folder != nil)
					{
						StErrorContextInstaller errs(false);
						folder->DeleteContents(true);
					}
				}
				break;
			}
		
		case DB4D_SystemTemporaryFolder:
			{
				VFolder *systemFolder = VFolder::RetainSystemFolder( eFK_Temporary, true);
				if (systemFolder != nil)
				{
					folderPath = systemFolder->GetPath();
					folderPath.ToSubFolder( CVSTR( "4D"));

					// build a unique name from db name.
					VString name;
					GetName( name);
					
					VUUID uuid( true);
					VString uuid_string;
					uuid.GetString( uuid_string);

					name += "_";
					name += uuid_string;

					folderPath.ToSubFolder( name);
					folder = new VFolder( folderPath);
					canDeleteFolder = true;

					ReleaseRefCountable( &systemFolder);
					if (folder != nil)
					{
						StErrorContextInstaller errs(false);
						folder->DeleteContents(true);
					}
				}
				break;
			}
		
		case DB4D_CustomFolder:
			{
				folderPath.FromFullPath( inCustomFolderPath);
				if (testAssert( folderPath.IsFolder()))
				{
					folder = new VFolder( folderPath);
					canDeleteFolder = false;
				}
				break;
			}
	}

	// on verifie d'une part que le dossier existe et d'autre part qu'on a les droits en ecriture dedans
	if (folder != nil)
	{
		// try to create it if it does not exist
		if (folder->Exists())
		{
			err = VE_OK;
			canDeleteFolder = false;
		}
		else
		{
			err = folder->CreateRecursive(true);	// make writable by all to avoid permission issues on mac (m.c)
		}

		// try to write a file inside
		if (err == VE_OK)
		{
			VUUID uuid( true);
			VString uuid_string;
			uuid.GetString( uuid_string);
			
			VFile tempFile( *folder, uuid_string);
			VFileDesc *fileDesc = nil;
			err = tempFile.Open( FA_READ_WRITE, &fileDesc, FO_Overwrite | FO_CreateIfNotFound);
			if (err == VE_OK)
			{
				const char *test = "this is a test";
				err = fileDesc->PutDataAtPos( test, strlen( test));
				delete fileDesc;
				
				{
					StErrorContextInstaller errs(false);

					VError err2 = tempFile.Delete();
					if (err == VE_OK)
						err = err2;

					if ( (err == VE_OK) && canDeleteFolder)
					{
						folder->Delete( false);
					}
				}
			}
		}
		else
		{
			err = vThrowError( VE_FOLDER_NOT_FOUND);
		}
		if (err != VE_OK)
		{
			ReleaseRefCountable( &folder);
		}
	}
	else
	{
		err = vThrowError( VE_FOLDER_NOT_FOUND);
	}
	
	*outRetainedFolder = folder;

	return err;
}


VError Base4D::SelectTemporaryFolder( bool inForCombinedDatabase)
{
/*
	look for user preferences into extra properties
*/
	VError err;
	VString customFolderPath;
	VFolder *folder = nil;
	if (inForCombinedDatabase)
	{
		// le temp folder pour la database des ressources sera le temp folder system (l'utilisateur n'a pas le choix)
		err = TryTempFolder( DB4D_SystemTemporaryFolder, customFolderPath, &folder);
	}
	else
	{
		StErrorContextInstaller errorContext( true /*inKeepingErrors*/, true /*inSilentContext*/);
		
		xbox_assert( fStructure != nil);		

		DB4DFolderSelector selector;
		err = GetTemporaryFolderSelector( &selector, customFolderPath);
		if (err == VE_OK)
			err = TryTempFolder( selector, customFolderPath, &folder);

		// on cherche successivement un dossier valide
		if (err != VE_OK)
			err = TryTempFolder( DB4D_DataFolder, customFolderPath, &folder);

		if (err != VE_OK)
			err = TryTempFolder( DB4D_SystemTemporaryFolder, customFolderPath, &folder);

		if (err != VE_OK)
			err = TryTempFolder( DB4D_StructureFolder, customFolderPath, &folder);
		
		// if we finally found something, let's forget previous errors.
		if (err == VE_OK)
			errorContext.Flush();
	}
	
	if (err == VE_OK)
		SetTemporaryFolder( folder);
	
	ReleaseRefCountable( &folder);
	
	return err;
}


Table* Base4D::RetainTable(sLONG numfile) const
{ 
	Table* res = nil;
	occupe();
	if (numfile < 0)
	{
		numfile = -numfile;
		if (numfile<=fSystemTables.GetCount())
		{
			res = fSystemTables[numfile];
			if (res != nil)
				res->Retain();
		}
	}
	else
	{
		if ((numfile>0) && (numfile<=fich.GetCount()))
		{
			res = fich[numfile];
			if (res != nil)
				res->Retain();
		}
	}
	
	libere();
	return res;
	
}

/*
DataTable* Base4D::GetNthDataFile(sLONG numfile) const
{ 
	DataTable* res = nil;
	occupe();
	{
		if ((numfile>0) && (numfile<=fichdata.GetCount()))
		{
			res = fichdata[numfile];	
		}
	}
	libere();
	return res;
	
}
*/

Field* Base4D::RetainField(sLONG numfile, sLONG numcrit) const
{
	Field* res = nil;
	Table* tabl = RetainTable(numfile);
	if (tabl != nil)
	{
		res = tabl->RetainField(numcrit);
		tabl->Release();
	}
	return res;
}


VError Base4D::CloseBase()
{
	VError err = VE_OK;

	// L.E. 27/08/02 need to call flushInfo->Dispose outside occupe/libere else there's a dead lock in the flush
	// il faut un lock specifique pour garantir que la base n'est plus utilisee pendant sa fermeture

	Boolean isUpdatePending;


	occupe();
	fIsClosing = true;
	isUpdatePending = fUpdateCounts.size() > 0;
	libere();

	VDBMgr::GetManager()->RemoveKeptDatasetsOnBase(this);

	if (fStoreAsXML)
		VDBMgr::GetManager()->RemoveStructureToStoreAsXML(this);

	BaseTaskInfo::InvalidateAllTransactions(this);
	RemoveIndexActionsForDB(this);

	while (isUpdatePending)
	{
		VTaskMgr::Get()->GetCurrentTask()->Sleep(100);
		occupe();
		isUpdatePending = fUpdateCounts.size() > 0;
		libere();
	}

#if trackClose
	trackDebugMsg("CloseBase no update pending\n");
#endif						

	occupe();

	if (fNetAccess != nil)
		fNetAccess->Dispose();
	fNetAccess = nil;

	BaseFlushInfo *flushInfo = fFlushInfo;
	fFlushInfo = nil;

	SetNotifyState(true);

	libere();

#if trackClose
	trackDebugMsg("wait for flushInfo\n");
#endif						
	if (flushInfo != nil)
		flushInfo->Dispose(); // c'est ici que l'on attend la fin des flush sur une base

#if trackClose
	trackDebugMsg("end of wait for flushInfo\n");
#endif						

	err = CloseData(false);
	
	err = CloseStructure();
	
	occupe();
	SetNotifyState(false);
	libere();

	XBOX::ReleaseRefCountable(&fBackupSettings);


	if (err != VE_OK)
		err = ThrowError(err, DBaction_ClosingBase);

	return err;
}



VError Base4D::CloseStructure()
{
	// ne sauve pas la structure car le fichier est peut etre deja ferme (si c'est dans le data)
	
	VError err = VE_OK;
	occupe();
	
	fEntityCatalog->DisposeEntityModels();
	fAutoEntityCatalog->DisposeEntityModels();
	fAutoEntityCatalogBuilt = false;
	DisposeRelations();
	ClearAllCachedRelPath();
	ReleaseAllTables();

	fich.SetAllocatedSize(0);
	
	fBaseName.Truncate( 0);

	if (fStructure != nil)
	{
		fStructure->Release();
		fStructure = nil;
	}
	
	libere();
	return err;
}


VError Base4D::CloseData(Boolean KeepFlushInfo)
{
	VError err = VE_OK;

#if debuglr == 2
	DebugMsg(L"Close Data : ");
	DebugMsg(fBaseName);
	DebugMsg(L"\n");
#endif

	occupe();

	ValidateHeader();

	if (err == VE_OK)
	{
		VTaskLock lock(&fLogMutex);
		if ( fLogStream )
		{
			WriteLog(DB4D_Log_CloseData, nil, false);
			fLogStream->CloseWriting();
			delete fLogStream;
			fLogStream = nil;
		}
		if ( fLogFile )
		{
			fLogFile->Release();
			fLogFile = nil;
		}
	}

	ClearAllTableDependencies(false);

//	DisposeRelations();
	DisposeIndexes();
	DisposeDataTables();
	DisposeDataSegments( );

	BaseFlushInfo *flushInfo = fFlushInfo;
	if (!KeepFlushInfo)
	{
		fFlushInfo = nil;
	}

	//xInit(false);

	libere();
	
	if (!KeepFlushInfo)
	{
		if (flushInfo != nil)
		{
			flushInfo->Dispose();
		}
	}

	return err;
}

#if 0
VError Base4D::DatabasePushMember (CLanguageContext* inContext, VClassInstance& ioClassInstance, CVariable* inVariable, StackPtr* ioStack)
{
	CDB4D_Lang* lang4D = VDBMgr::GetLang4D();
	
	sLONG nf = inVariable->GetID();
	CDB4DBase* base = (CDB4DBase*) ioClassInstance;
		
	CDB4DTable* table = base->RetainNthTable((DB4D_TableID) nf);
	
	// FM, il faut mettre quelque chose dans la pile, ou retourner une erreur pour arreter l'execution
	inContext->PushClassInstance( ioStack, (VClassInstance) table, inVariable );
	return(VE_OK);
}
#endif


#if 0
VError Base4D::DatabasePopMember (CLanguageContext* inContext, VClassInstance& ioClassInstance, VVariableStamp* ptStamp, CVariable* inVariable, StackPtr* ioStack)
{
	// FM: il faut sortir le membre de la pile ou retourner une erreur pour arreter l'execution 
	Boolean isNull;
	CVariable* var;
	CDB4DTable* dummy = (CDB4DTable*)inContext->PopClassInstance( ioStack, isNull, var );
	if (dummy != nil)
		dummy->Release();
	return(VE_OK);
}
#endif


void Base4D::RegisterForLang(void)
{
	sLONG i,nb;
	VString name;
//	CLanguage* sLanguage = VDBMgr::GetLanguage();
//	CDB4D_Lang* lang4D = VDBMgr::GetLang4D();
	
	occupe();
	
	if (fStructure != nil)
	{
#if 0
		if ( (lang4D != nil) && (sLanguage != nil) )
		{
			VUUID xUID;
			GetName(name);
			
			sLONG pos = name.FindUniChar(CHAR_FULL_STOP);
			if (pos != 0)
				name.Truncate(pos-1);
			
			VString NameSpaceName = name;

			/*
			VString	s(L"Base ", -1);
			s.AppendString(name);
			*/
			VString s = name;
			
			if (BaseRef == nil)
			{
				VCreatorMethod creatorMethod;
				VDestructorMethod destructorMethod;
				VPushMemberMethod pushMemberMethod;
				VPopMemberMethod popMemberMethod;
				VPushArrayElementMethod PushArrayElementMethod;
				VPopArrayElementMethod PopArrayElementMethod;

				lang4D->GetDataBaseClassRef()->GetMethods(creatorMethod, destructorMethod, pushMemberMethod, popMemberMethod);

				//xUID.Regenerate();
				xUID = fID;
				BaseRef = sLanguage->GetRootNameSpace()->CreateClass(s, xUID, lang4D->GetDataBaseClassRef(), creatorMethod, destructorMethod,
																										DatabasePushMember, DatabasePopMember );
				BaseRef->SetCustomData(dynamic_cast<CDB4DBase*>( fBaseX ) );
				BaseRef->SetCanEditDefaultValue(false, false);
				BaseRef->AllowDynamicCast(true);
				
				//xUID.Regenerate();
				VUUIDBuffer buf;

				xUID.ToBuffer(buf);
				buf.IncFirst8();
				xUID.FromBuffer(buf);

				BaseNameSpace = sLanguage->CreateNameSpace( NameSpaceName, xUID, BaseRef);
				
				if (VDBMgr::GetCommandManager() != nil)
					VDBMgr::GetCommandManager()->Tell_AddBase(xUID);

				// retirer la suite si le registerlang est a nouveau fait dans le open ou create
				SetNotifyState(true);
				TableArray::Iterator cur = fich.First(), end = fich.End();
				for (; cur!=end; cur++)
				{
					Table* fic = *cur;
					if (fic != nil)
						fic->RegisterForLang(true);
				}

				cur = fSystemTables.First();
				end = fSystemTables.End();
				for (; cur!=end; cur++)
				{
					Table* fic = *cur;
					if (fic != nil)
						fic->RegisterForLang(true);
				}

				V0ArrayOf<Relation*>::Iterator currel = fRelations.First(), endrel = fRelations.End();
				for (; currel!=endrel; currel++)
				{
					Relation* rel = *currel;
					if (rel != nil)
						rel->RegisterForLang();
				}
				
				SetNotifyState(false);

				fRegistered = true;
				
			}
			else
			{
				VUUID xUID;
				BaseRef->SetName(s);
				BaseNameSpace->GetNameSpaceClass()->SetName(NameSpaceName);

				xUID = fID;
				if (VDBMgr::GetCommandManager() != nil)
					VDBMgr::GetCommandManager()->Tell_UpdateBase( xUID, TUB_NameChanged);
			}
			
			
		}
		else
#endif
		{
			if (fRegistered)
			{
				VUUID xUID;
				xUID = fID;
				if (VDBMgr::GetCommandManager() != nil)
					VDBMgr::GetCommandManager()->Tell_UpdateBase( xUID, TUB_NameChanged);
			}
			else
			{
				VUUID xUID;
				xUID = fID;
				if (VDBMgr::GetCommandManager() != nil)
					VDBMgr::GetCommandManager()->Tell_AddBase(xUID);
				fRegistered = true;
			}
		}
	}
		
	libere();
}


void Base4D::UnRegisterForLang(void)
{
	sLONG i,nb;
//	CLanguage* sLanguage = VDBMgr::GetLanguage();
//	CDB4D_Lang* lang4D = VDBMgr::GetLang4D();
	VUUID xUID;
	
	occupe();
	
	if (fRegistered)
	{
		xUID = fID;
		if (VDBMgr::GetCommandManager() != nil)
			VDBMgr::GetCommandManager()->Tell_CloseBase(xUID);
		fRegistered = false;

		Boolean oldstate = IsNotifying();
		SetNotifyState(true);

		/*
		nb = fich.GetCount();
		for (i=1; i<=nb; i++)
		{
			fic = fich[i];
			if (fic != nil)
			{
				fic->UnRegisterForLang();
			}
		}
		*/

		TableArray::Iterator cur = fich.First(), end = fich.End();
		for (; cur!=end; cur++)
		{
			Table* fic = *cur;
			if (fic != nil)
				fic->UnRegisterForLang();
		}

		cur = fSystemTables.First();
		end = fSystemTables.End();
		for (; cur!=end; cur++)
		{
			Table* fic = *cur;
			if (fic != nil)
				fic->UnRegisterForLang();
		}

		V0ArrayOf<Relation*>::Iterator currel = fRelations.First(), endrel = fRelations.End();
		for (; currel!=endrel; currel++)
		{
			Relation* rel = *currel;
			if (rel != nil)
				rel->UnRegisterForLang();
		}

		SetNotifyState(oldstate);
	}
#if 0
	if (BaseRef != nil)
	{
		if ( (lang4D != nil) && (sLanguage != nil) )
		{

			BaseNameSpace->Drop();
			BaseNameSpace->Release();
			BaseRef->Drop();
			BaseRef->Release();
			
			BaseRef = nil;
			BaseNameSpace = nil;
		}
	}
#endif
	
	
	libere();
}


VError Base4D::CreateResMap()
{
	VError err = VE_OK;

	Table* t;
	Field* f[10];

	t = new TableRegular(this,1,false);
	t->SetName(L"Resources", nil);

	Field* *p = &f[0];

	*p = new Field(DB4D_Integer32, 1, t);
	(*p)->SetName(L"kind", nil);
	(*p)->SetUUID(VUUID("Res_DB4D_field1"));
	p++;

	*p = new Field(DB4D_Integer32, 2, t);
	(*p)->SetName(L"id", nil);
	(*p)->SetUUID(VUUID("Res_DB4D_field2"));
	p++;

	*p = new Field(DB4D_Blob, 3, t);
	(*p)->SetName(L"data", nil);
	(*p)->SetUUID(VUUID("Res_DB4D_field3"));
	p++;

	*p = new Field(DB4D_Boolean, 4, t);
	(*p)->SetName(L"little_endian", nil);
	(*p)->SetUUID(VUUID("Res_DB4D_field4"));
	p++;

	*p = new Field(DB4D_Integer32, 5, t);
	(*p)->SetName(L"stamp", nil);
	(*p)->SetUUID(VUUID("Res_DB4D_field5"));
	p++;

	*p = new Field(DB4D_Integer32, 6, t);
	(*p)->SetName(L"options", nil);
	(*p)->SetUUID(VUUID("Res_DB4D_field6"));
	p++;

	*p = new Field(DB4D_StrFix, 7, t);
	(*p)->SetName(L"name", nil);
	(*p)->SetUUID(VUUID("Res_DB4D_field7"));
	p++;

	*p = new Field(DB4D_StrFix, 8, t);
	(*p)->SetName(L"attributes", nil);
	(*p)->SetUUID(VUUID("Res_DB4D_field8"));
	p++;

	*p = new Field(DB4D_Time, 9, t);
	(*p)->SetName(L"modification_time", nil);
	(*p)->SetUUID(VUUID("Res_DB4D_field9"));
	p++;

	t->AddFields(&f[0], 9, nil, true, nil);

	t->SetUUID((VUUIDBuffer&)"Res_DB4D_000000");
	err = AddTable(t, true, nil, false, false, true, 0, false);

	t->Release();

	p = &f[0];
	for (sLONG i=0;i < 9;i++)
	{
		(*p)->Release();
		p++;
	}




	t = new TableRegular(this,2,false);
	t->SetName(L"Schemas", nil);

	p = &f[0];

	*p = new Field(DB4D_StrFix, 1, t);
	(*p)->SetName(L"Name", nil);
	(*p)->SetUUID(VUUID("sch_DB4D_field1"));
	p++;

	*p = new Field(DB4D_Integer32, 2, t);
	(*p)->SetName(L"id", nil);
	(*p)->SetUUID(VUUID("sch_DB4D_field2"));
	(*p)->SetAutoSeq(true, nil);
	p++;

	*p = new Field(DB4D_Blob, 3, t);
	(*p)->SetName(L"properties_data", nil);
	(*p)->SetUUID(VUUID("sch_DB4D_field3"));
	p++;

	t->AddFields(&f[0], 3, nil, true, nil);

	t->SetUUID((VUUIDBuffer&)"sch_DB4D_000000");
	err = AddTable(t, true, nil, false, false, true, 0, false);

	t->Release();

	p = &f[0];
	for (sLONG i=0;i < 3;i++)
	{
		(*p)->Release();
		p++;
	}



#if WITH_OBJECTS_TABLE
	t = new TableRegular(this,2,false);
	t->SetName(L"Objects", false);

	p = &f[0];

	*p = new Field(DB4D_UUID, 1, t);
	(*p)->SetName(L"uuid");
	p++;

	*p = new Field(DB4D_StrFix, 2, t);
	(*p)->SetName(L"type");
	p++;

	*p = new Field(DB4D_StrFix, 3, t);
	(*p)->SetName(L"name");
	p++;

	*p = new Field(DB4D_Blob, 4, t);
	(*p)->SetName(L"data");
	p++;

	*p = new Field(DB4D_UUID, 5, t);
	(*p)->SetName(L"owner");
	p++;

	*p = new Field(DB4D_Integer32, 6, t);
	(*p)->SetName(L"access");
	p++;

	t->AddFields(&f[0], 6, nil, true);

	err = AddTable(t, true, nil, false, false, true, 0, false);
	t->SetUUID((VUUIDBuffer&)"Obj_DB4D_000000");

	t->Release();

	p = &f[0];
	for (sLONG i=0;i<6;i++)
	{
		(*p)->Release();
		p++;
	}
#endif

	return err;
}


VError Base4D::LoadIndexesAfterCompacting(sLONG inParameters)
{
	VError err = VE_OK;
	occupe();

	if (err == VE_OK)
	{
		err = LoadIndexes(false);
		setmodif(true, this, nil);
	}

	libere();

	if (err == VE_OK && !fWriteProtected)
	{
		err = CheckIndexesOnSubTables();
	}

	if (err == VE_OK)
	{
		VTask *currentTask = VTaskMgr::Get()->GetCurrentTask();
		while (StillIndexingOn(this))
		{
			sLONG delay = 100;
			currentTask->ExecuteMessages();	// for progress signaling
			currentTask->Sleep(delay);
		}
	}

	return err;
}


VError Base4D::CopyTableDefsFrom(Base4D_NotOpened* from, TabAddrDisk& taddr, sLONG nbelem)
{
	VError err = VE_OK;
	for (sLONG i = 0; i < nbelem; ++i)
	{
		DataAddr4D ou = taddr[i].addr;
		sLONG len = taddr[i].len;
		if (ou > 0)
		{
			StructElemDef* e = new StructElemDef(this, -1, DBOH_TableDefElem, TableDefAccess);
			sLONG xDataLen = len - kSizeDataBaseObjectHeader;
			void* xdata = GetFastMem(xDataLen, false, 'sted');
			from->readlong(xdata, xDataLen, ou, kSizeDataBaseObjectHeader);
			e->SetData(xdata, xDataLen);
			e->libere();
			SaveTableRefInDatas(e);
		}
	}
	return err;
}


VError Base4D::CreateData( const VFile& inFile, sLONG inParameters, VIntlMgr* inIntlMgr, VFile *inJournalingFile, Boolean BuildResMap, FileAccess inAccess, const VUUID* inChosenID, CDB4DRawDataBase* fromData )
{
#if debuglr == 2
	DebugMsg(L"Create Data : ");
	VString sdebug;
	inFile.GetPath(sdebug);
		DebugMsg(sdebug);
	DebugMsg(L"\n");
#endif

	VString stringCompHash, keywordBuildingHash;

	VError err = VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE;

	if (testAssert(!fIsRemote))
	{
		err = VE_OK;

		occupe();

		if (fFlushInfo == nil)
			fFlushInfo = fFlusher->NewBaseFlushInfo(this);

		if (inChosenID != nil)
		{
			inChosenID->ToBuffer(hbbloc.GetID());
		}
		else
		{
			if (fStructure != nil)
			{
				fStructure->GetUUID().ToBuffer(hbbloc.GetID());
			}
		}
		hbbloc.VersionNumber = kVersionDB4D;

		if (IsWakandaRunningMode()) // l'emplacement de l'ancien UUID est maintenant utilise pour l'addr de la addrtable des ExtraElements
		{
			hbbloc.IDisNotAnID = 1;
			hbbloc.filler2 = 0;
			hbbloc.ExtraElements_addrtabaddr = 0;
			hbbloc.nbExtraElements = 0;
		}

		fTableDataTableTabAddr.Init(this, this, &hbbloc.DataTables_addrtabaddr, &hbbloc.DataTables_debuttrou, &hbbloc.nbDataTable, nil, nil, 0);

		if (fStructure != nil)
		{
			fTableDefTabAddr.Init(this,this,&hbbloc.TableDef_addrtabaddr,&hbbloc.TableDef_debuttrou,&hbbloc.nbTableDef, 
				nil, nil, 0);

			fTableDefInMem.Init(hbbloc.nbTableDef, false);
			fTableDefInMem.SetContientQuoi(t_tabledef);
		}

		fIndexesTabAddr.Init(this,this,&hbbloc.IndexDef_addrtabaddr,&hbbloc.IndexDef_debuttrou,&hbbloc.nbIndexDef, 
			nil, nil, 0);

		fIndexesInMem.Init(hbbloc.nbIndexDef, false);
		fIndexesInMem.SetContientQuoi(t_indexdef);

		fTableSeqNumTabAddr.Init(this, this, &hbbloc.SeqNum_addrtabaddr, &hbbloc.SeqNum_debuttrou, &hbbloc.nbSeqNum, nil, nil, 0);
		hbbloc.WithSeparateIndexSegment = (inParameters & DB4D_Open_WithSeparateIndexSegment) != 0;

		SegData *seg = new SegData( this, 1, new VFile( inFile), &hbbloc.seginfo);
		// seg->occupe();
		fSegments.Add( seg);
		const VUUID* structID = nil;
		if (inChosenID != nil)
			structID = inChosenID;
		else
		{
			if (fStructure != nil)
				structID = &(fStructure->GetUUID());
		}
		//err = seg->CreateSeg(fStructure == nil ? nil : &(fStructure->GetUUID()), inAccess );
		err = seg->CreateSeg(structID, inAccess );
		
		if (err == VE_OK)
		{
			VIntlMgr* oldmgr = fIntlMgr;

			if (fStructure == nil)
			{
				if (inIntlMgr == nil)
					inIntlMgr = VIntlMgr::GetDefaultMgr();

				fIntlMgr = inIntlMgr;
			}
			else
			{
				fIntlMgr = fStructure->fIntlMgr;
				if (fIntlMgr == nil)
					fIntlMgr = VIntlMgr::GetDefaultMgr();
			}

			if (fIntlMgr == nil)
				err = ThrowError(memfull, DBaction_CreatingDataFile);
			else
				fIntlMgr->Retain();

			if (err == VE_OK)
			{
				hbbloc.collatorOptions = fIntlMgr->GetCollator()->GetOptions();
				hbbloc.dialect = fIntlMgr->GetDialectCode();
				fIntlMgr->GetStringComparisonAlgorithmSignature(stringCompHash);
				fIntlMgr->GetKewordAlgorithmSignature(keywordBuildingHash);
			}

			if (oldmgr != nil)
				oldmgr->Release();
		}

		if (err == VE_OK) 
		{
			hbbloc.nbsegdata=1;
			Set_debug_OKToWriteIn0(true);
			
			err = writelong(&hbbloc,sizeof(hbbloc),0,0);
			if (err == VE_OK)
			{
				fIsDataOpened=true;
			}
			Set_debug_OKToWriteIn0(false);
		}
		
		if (err != VE_OK) 
		{
			fSegments.DeleteNth( fSegments.GetCount());
			hbbloc.nbsegdata=fSegments.GetCount();
			seg->DeleteSeg();
			seg->Release();
			seg = nil;
		} 
		else 
		{
			seg->PutInCache();
			err = SaveDataSegments();
			if (err == VE_OK)
			{
				Set_debug_OKToWriteIn0(true);
				err = writelong(&hbbloc,sizeof(hbbloc),0,0);
				Boolean ok = (err == VE_OK);
				Set_debug_OKToWriteIn0(false);
				
				if (ok)
				{
					if (BuildResMap) // c'est ici que l'on ajoute les deux tables qui definissent les ressources
					{
						err = CreateResMap();
					}

					if (err == VE_OK)
					{
						for( sLONG i=1;(i<=fNbTable) && (err==VE_OK);i++)
						{
							Table *fic=fich[i];
							if (fic != nil)
							{
								DataTable* df = CreateDataTable(fic, err, 0, nil, -1);
								if (df != nil)
								{
									//df->RemoveFromCache();
									df->Release();
								}
							}
						}
					}
				}
			}
		}

		if (err == VE_OK)
		{
			err = SetIntlInfo(stringCompHash, keywordBuildingHash);
		}

		if ( err == VE_OK )
		{
			fExtraElements = new ExtraElementTable(this, &hbbloc.ExtraElements_addrtabaddr, &hbbloc.nbExtraElements);
			err = fExtraElements->load();
		}


		if ( err == VE_OK && inJournalingFile )
		{
			StErrorContextInstaller errs(false);
			fLogErr = SetJournalFile(inJournalingFile);
			if (fLogErr == VE_OK)
			{
				fCurrentLogOperation = 0;
				WriteLog(DB4D_Log_OpenData, nil, false);
			}
		}

		if (fromData != nil) // used for a compact or repair data in Wakanda
		{
			/* code is now moved to CheckTableDefs
			VDB4DRawDataBase* fromDatax = dynamic_cast<VDB4DRawDataBase*>(fromData);
			Base4D_NotOpened* from = fromDatax->GetBase();
			BaseHeader* headerfrom = from->GetHbbloc();
			if (headerfrom->nbTableDef > 0)
			{
				TabAddrDisk taddr;
				from->ReadAddressTable(headerfrom->TableDef_addrtabaddr, taddr, 0, -1, nil, nil, err, 0, true, false);


				if (headerfrom->nbTableDef > kNbElemTabAddr)
				{
					sLONG nb2 = (headerfrom->nbTableDef+kNbElemTabAddr-1)/kNbElemTabAddr;
					for (sLONG i = 0; i < nb2; ++i)
					{
						TabAddrDisk taddr2;
						from->ReadAddressTable(taddr[i].addr, taddr2, i, -1, nil, nil, err, 0, true, false);
						sLONG nb;
						if (i == nb2-1)
						{
							nb = headerfrom->nbTableDef % kNbElemTabAddr;
							if (nb == 0)
								nb = kNbElemTabAddr;
						}
						else
							nb = kNbElemTabAddr;

						CopyTableDefsFrom(from, taddr2, kNbElemTabAddr);
					}
				}
				else
				{
					CopyTableDefsFrom(from, taddr, headerfrom->nbTableDef);
				}
			}

			if (err == VE_OK)
				LoadAllTableDefsInData(true);
				*/
		}
		else
		{
			if (err == VE_OK && IsWakandaRunningMode())
			{
				//fIsParsingStruct = true;
				err = SecondPassLoadEntityModels(nil, false);
				//fIsParsingStruct = false;
			}

			if (err == VE_OK && (inParameters & DB4D_Open_DelayLoadIndex) == 0)
			{
				err = LoadIndexes((inParameters & DB4D_Open_DO_NOT_Build_Index) != 0);
				setmodif(true, this, nil);
			}
		}
		libere();
	}

	if (err == VE_OK && !fWriteProtected && (inParameters & DB4D_Open_DelayLoadIndex) == 0)
	{
		err = CheckIndexesOnSubTables();
	}

	if (err == VE_OK)
		SaveDataTablesToStructTablesMatching();

	if (err == VE_OK)
	{
		StErrorContextInstaller errs(false);
#if debug_Addr_Overlap
		FillDebug_DBObjRef();
#endif
		SelectTemporaryFolder( fStructure == nil);
	}
	else
		err = ThrowError(VE_DB4D_CANNOTCREATEDATAFILE,DBaction_CreatingDataFile);

	return err;
}



void Base4D::GetDataSegPath(sLONG inDataSegNum, VFilePath& outPath)
{
	if ( (inDataSegNum > 0) && (inDataSegNum <= fSegments.GetCount())) {
		fSegments[inDataSegNum-1]->GetFullPath( outPath);
	} else {
		VFilePath empty;
		outPath = empty;
	}
}


VFolder* Base4D::RetainDataFolder()
{
	VFolder* result = nil;

	if (!fSegments.IsEmpty())
	{
		VFile* ff = fSegments[0]->GetFile();
		if (ff != nil)
			result = ff->RetainParentFolder();
	}

	return result;
}



VFolder* Base4D::RetainBlobsFolder()
{
	VFolder* result = nil;
	if (fBlobsFolder == nil)
	{
		if (fSegments.GetCount() > 0)
		{
			SegData* seg = fSegments[0];
			VFile* ff = seg->GetFile();
			if (ff != nil)
			{
				VString ss;
				ff->GetNameWithoutExtension(ss);
				VFolder* parent = ff->RetainParentFolder();
				fBlobsFolder = new VFolder(*parent,ss+".ExternalData");
				parent->Release();
			}
		}
	}
	result = RetainRefCountable(fBlobsFolder);
	return result;
}


void Base4D::GetBlobsFolderPath(VString& outPath)
{
	if (fBlobsFolder == nil)
	{
		QuickReleaseRefCountable(RetainBlobsFolder());
	}
	if (fBlobsFolder == nil)
		outPath.Clear();
	else
		fBlobsFolder->GetPath(outPath, FPS_POSIX);
}


VError Base4D::SetSeqNumAddr(sLONG SeqNumNum, DataAddr4D addr, sLONG len, sLONG &outSeqNumNum)
{
	VError err = VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE;

	if (testAssert(!fIsRemote))
	{
		occupe();
		if (SeqNumNum == -2)
			SeqNumNum = hbbloc.nbSeqNum;
		err = fTableSeqNumTabAddr.PutxAddr(SeqNumNum, addr, len, nil);
		outSeqNumNum = SeqNumNum;
		libere();
	}
	return err;
}

VError Base4D::LoadSeqNums()
{
	VError err = VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE;

	if (testAssert(!fIsRemote))
	{
		err = VE_OK;
		fTableSeqNumTabAddr.Init(this, this, &hbbloc.SeqNum_addrtabaddr, &hbbloc.SeqNum_debuttrou, &hbbloc.nbSeqNum, nil, nil, 0);
		sLONG i;

		for (i=0; i<hbbloc.nbSeqNum && err == VE_OK; i++)
		{
			sLONG len;
			DataAddr4D ou = fTableSeqNumTabAddr.GetxAddr(i, nil, err, &len);
			if (ou>0 && err == VE_OK)
			{
				DataBaseObjectHeader tag;
				AutoSeqNumber* seqres = nil;
				AutoSeqNumberSimple* seq;
				AutoSeqNumberNoHole* seq2;
				sLONG typ;

				err = tag.ReadFrom(this, ou, nil);
				if (err == VE_OK)
				{
					err = tag.ValidateTag(DBOH_AutoSeqNumberSimple, i, -1);
					if (err != VE_OK)
						err = tag.ValidateTag(DBOH_AutoSeqNumberNoHole, i, -1);
				}
				if (err == VE_OK)
					err = readlong(&typ, sizeof(typ), ou, kSizeDataBaseObjectHeader);

				if (err == VE_OK)
				{
					if (tag.NeedSwap())
						typ = SwapLong(typ);
					switch ((DB4D_AutoSeq_Type)typ)
					{
						case DB4D_AutoSeq_Simple:
							seq = new AutoSeqNumberSimple(this, i);
							if (seq != nil)
							{
								seqres = seq;
							}
							else
								err = ThrowError(memfull, DBaction_LoadingAutoSeqHeader);
							break;

						case DB4D_AutoSeq_NoHole:
							seq2 = new AutoSeqNumberNoHole(this, i);
							if (seq2 != nil)
							{
								seqres = seq2;
							}
							else
								err = ThrowError(memfull, DBaction_LoadingAutoSeqHeader);
							break;

						default:
							xbox_assert(false);
							break;

					}

					if (seqres != nil)
					{
						err = seqres->loadobj(ou, tag);
						if (err == VE_OK)
						{
							try
							{
								fSeqNums[seqres->GetID()] = seqres;
							}
							catch (...)
							{
								err = ThrowError(memfull, DBaction_LoadingAutoSeqHeader);
							}
						}
						seqres->libere();
					}

				}
			}
		}
	}

	return err;
}


AutoSeqNumber* Base4D::AddSeqNum(DB4D_AutoSeq_Type typ, VError& err, CDB4DBaseContext* inContext)
{
	AutoSeqNumber* seqres = nil;
	AutoSeqNumberSimple* seq;
	AutoSeqNumberNoHole* seq2;

	err = VE_OK;

	if (fIsRemote)
	{
	}
	else
	{

		occupe();

		sLONG n = fTableSeqNumTabAddr.findtrou(nil, err);
		if (err == VE_OK)
		{
			if (n==-1)
			{
				n = hbbloc.nbSeqNum;
			}

			switch (typ)
			{
				case DB4D_AutoSeq_Simple:
					seq = new AutoSeqNumberSimple(this, n);
					if (seq != nil)
					{
						seqres = seq;
					}
					else
						err = ThrowError(memfull, DBaction_CreatingAutoSeqHeader);
					break;

				case DB4D_AutoSeq_NoHole:
					seq2 = new AutoSeqNumberNoHole(this, n);
					if (seq2 != nil)
					{
						seqres = seq2;
					}
					else
						err = ThrowError(memfull, DBaction_CreatingAutoSeqHeader);
					break;

				default:
					err = ThrowError(VE_DB4D_NOTIMPLEMENTED, DBaction_CreatingAutoSeqHeader);
					xbox_assert(false);
					break;
			}
		}

		if (err == VE_OK)
		{
			err = seqres->InitForNew();
			if (err == VE_OK)
			{
				try
				{
					fSeqNums[seqres->GetID()] = seqres;
				}
				catch (...)
				{
					err = ThrowError(memfull, DBaction_CreatingAutoSeqHeader);
				}
				seqres->libere();
			}
		}


		libere();
	}

	return seqres;
}


void Base4D::DisposeSeqNums()
{
	MapOfSeqNum::iterator p;
	for (p = fSeqNums.begin(); p != fSeqNums.end(); p++)
	{
		AutoSeqNumber* x = p->second;
		x->SetInvalid();
		x->Release();
	}
	fSeqNums.clear();
}


AutoSeqNumber* Base4D::RetainSeqNum(const VUUID& IDtoFind)
{
	AutoSeqNumber* result = nil;
	occupe();

	MapOfSeqNum::iterator p = fSeqNums.find(IDtoFind);
	if (p != fSeqNums.end())
	{
		result = p->second;
		result->Retain();
	}

	libere();

	return result;
}


VError Base4D::DeleteSeqNum(const VUUID& IDtoDelete, CDB4DBaseContext* inContext)
{
	VError err = VE_OK;

	if (fIsRemote)
	{
	}
	else
	{
		AutoSeqNumber* seq = nil;
		occupe();

		MapOfSeqNum::iterator p = fSeqNums.find(IDtoDelete);
		if (p != fSeqNums.end())
		{
			seq = p->second;
			fSeqNums.erase(p);
		}

		libere();

		if (seq != nil)
		{
			sLONG num = seq->GetNum();
			seq->occupe();
			err = seq->LibereEspaceDisk(nil, nil);
			if (seq->modifieonly())
				VDBMgr::GetFlushManager()->RemoveObject(seq);
			seq->SetInvalid();
			seq->libere();
			seq->Release();

			fTableSeqNumTabAddr.liberetrou(num, nil);
		}
	}

	return err;
}



VError Base4D::LoadFieldIdsInRec()
{
	VError err = VE_OK;
	if (fFieldsIDInRec == nil)
	{
		fFieldsIDInRec = new mapFieldsIDInRecByName(this);
		err = fFieldsIDInRec->load();
	}
	return err;
}


sLONG Base4D::GetSmallID(Field* field, VError& err)
{
	err = VE_OK;
	sLONG id = field->GetSmallIDInRec();
	if (id == 0)
	{
		VString sfield, stable;
		field->GetName(sfield);
		field->GetOwner()->GetName(stable);
		VString s = stable+"."+sfield;
		id = GetSmallID(s, err);
		if (id != 0)
		{
			field->SetSmallIDInRec(id);
		}
	}
	return id;
}


sLONG Base4D::GetSmallID(const VString& inRef, VError& err)
{
	sLONG id = 0;
	err = LoadFieldIdsInRec();
	if (err == VE_OK)
	{
		id = fFieldsIDInRec->GetSmallID(inRef);
	}
	return id;
}


VError Base4D::SaveSmallIDsInRec()
{
	VError err = VE_OK;
	if (fFieldsIDInRec != nil)
		err = fFieldsIDInRec->save();
	return err;
}




VError Base4D::LoadDataTables()
{
	VError err = VE_OK;
	bool mustRegenerateMatchList = false;

	if (testAssert(!fIsRemote))
	{
		occupe();
		
		fTableDataTableTabAddr.Init(this, this, &hbbloc.DataTables_addrtabaddr, &hbbloc.DataTables_debuttrou, &hbbloc.nbDataTable, nil, nil, 0);

		fDataTables.resize(hbbloc.nbDataTable, nil);
		for (sLONG i = 0; i < hbbloc.nbDataTable && err == VE_OK; i++)
		{
			sLONG len;
			DataAddr4D ou = fTableDataTableTabAddr.GetxAddr(i, nil, err, &len);
			if (err == VE_OK)
			{
				if (ou > 0)
				{
					VUUID xid;

					DataTableRegular* df = new DataTableRegular(this, nil, i+1, ou, false, nil);
					if (df == nil)
						err=ThrowError(memfull, DBaction_ReadingStruct);
					else
					{
						if (df->IsInvalid())
						{
							df->Release();
							df = nil;
							err=ThrowError(VE_DB4D_CANNOTLOADTABLEDEF, DBaction_ReadingStruct);
						}
						else
						{
							if (df->GetTable() != nil)
								df->Retain();
							fDataTables[i] = df;
							df->PutInCache();
							if (!fWriteProtected)
							{
								err = df->CheckForNonEmptyTransHolesList();
							}
						}
					}
				}
			}
		}

		if (err == VE_OK)
		{
			for( sLONG i=1;(i<=fNbTable) && (err==VE_OK);i++)
			{
				Table *fic=fich[i];
				if (fic != nil)
				{
					if (fic->GetDF() == nil)
					{
						DataTable* df = CreateDataTable(fic, err, 0, nil, -1);
						mustRegenerateMatchList = true;
						if (df != nil)
							df->Release();
						/*
						if (err != VE_OK)
							err = ThrowError(VE_DB4D_CANNOTCREATEDATAFILE,DBaction_CreatingDataFile);
						*/
					}
				}
			}
		}

		if (err == VE_OK /*&& mustRegenerateMatchList*/)
			SaveDataTablesToStructTablesMatching();

		if (err != VE_OK) 
		{
			StErrorContextInstaller errs(false);
			DisposeDataTables();
		}
		
		libere();
	}
	else
		err = VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE;

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOT_LOAD_DATATABLES, DBactionFinale);
	
	return err;
}


SegData* Base4D::GetSpecialSegment(sLONG SegNum) const
{
	if (testAssert(!fIsRemote))
	{
		if (SegNum == kIndexSegNum - 1)
			return fIndexSegment;
		else
			return nil;
	}
	else
		return nil;
}


VError Base4D::CreateIndexSegment(const VFile& inFile)
{
	VError err2 = VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE;
	if (testAssert(!fIsRemote))
	{
		occupe();
		VString s;
		inFile.GetNameWithoutExtension(s);
		if (fStructure == nil)
			s.AppendString(kStructIndexExt);
		else
			s.AppendString(kDataIndexExt);
		VFolder* vf = inFile.RetainParentFolder();
		if (vf != nil)
		{
			VFile* ixf = new VFile(*vf, s);
			vf->Release();
			SegData *seg = new SegData( this, kIndexSegNum, ixf, nil);
			ixf = nil;
			err2 = seg->CreateSeg(fStructure == nil ? &(GetUUID()) : &(fStructure->GetUUID()), FA_READ_WRITE);
			if (err2 == VE_OK)
			{
				fIndexSegment = seg;
				seg->PutInCache();
			}
			else
			{
				seg->DeleteSeg();
				seg->Release();
			}
		}
		else err2 = -1;

		libere();
	}
	return err2;
}

VError Base4D::OpenIndexSegment(const VFile& inFile, Boolean isAStruct, FileAccess inAccess)
{
	VError err2 = VE_OK;
	if (testAssert(!fIsRemote))
	{
		VString s;
		inFile.GetNameWithoutExtension(s);
		if (isAStruct)
			s.AppendString(kStructIndexExt);
		else
			s.AppendString(kDataIndexExt);
		VFolder* vf = inFile.RetainParentFolder();
		if (vf != nil)
		{
			VFile* ixf = new VFile(*vf, s);
			vf->Release();
			VFileDesc *f = nil;
			if (fMustRebuildAllIndex)
			{
				if (ixf->Exists())
					err2 = ixf->Delete();
			}

			if (err2 == VE_OK)
			{
				if (ixf->Exists())
				{
					StErrorContextInstaller errs(false); 
					err2 = ixf->Open(inAccess, &f);
					if (err2 == VE_OK)
					{
						BaseHeader hbb;
						VSize len = sizeof(hbb);
						sLONG retour = 0;
						Boolean wasswapped;
						err2 = ReadBaseHeader(*f, &hbb, false, false, isAStruct || IsWakandaRunningMode() ? nil : &(GetUUID()), &retour, wasswapped, true, true, true);
						if (err2 == VE_OK)
						{
							if (hbb.LastFlushRandomStamp != hbbloc.LastFlushRandomStamp)
							{
								// LastFlushRandomStamp used to be the byte order HighLow or LowHigh.
								// Accept only pairs of these values
								if ( ((hbb.LastFlushRandomStamp != HighLow) && (hbb.LastFlushRandomStamp != LowHigh))
									|| ((hbbloc.LastFlushRandomStamp != HighLow) && (hbbloc.LastFlushRandomStamp != LowHigh)) )
									err2 = VE_DB4D_DATAFILE_DOES_NOT_MATCH_STRUCT;
							}
							if (hbb.countFlush != hbbloc.countFlush)
							{
								err2 = VE_DB4D_DATAFILE_DOES_NOT_MATCH_STRUCT;
							}
							if (hbb.lastoper != 0)
							{
								err2 = VE_DB4D_DATAFILE_DOES_NOT_MATCH_STRUCT;
							}
						}

						if (err2 == VE_OK && (retour != 0 || wasswapped) && !fWriteProtected)
						{
							f->PutData(&hbb, sizeof(hbb), 0);
						}

						if (err2 == VE_OK)
						{
							if (hbb.seginfo.finfic == 0 || hbb.seginfo.sizeblockseg != ktaillebloc || hbb.seginfo.ratio != kratio)
								err2 = 1;
						}

						if (err2 == VE_OK)
						{
							SegData* seg = new SegData(this, hbb, kIndexSegNum, ixf, f, nil);
							ixf = nil;
							f = nil;
							fIndexSegment = seg;
							seg->PutInCache();
						}
					}
				}
				else
					err2 = -2;
				if (f != nil)
					delete f;
				if (ixf != nil)
					ixf->Release();
			}
			else
				err2 = -1;
		}
	}
	else
		err2 = VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE;

	return err2;
}

void Base4D::GetJournalInfos( const VFilePath &inDataFilePath, VFilePath &outFilePath, VUUID &outDataLink)
{
	VError bagError;
	outFilePath.Clear();
	outDataLink.Clear();
	
	const VValueBag* bag = RetainExtraProperties( bagError, nil );
	if ( bag != NULL )
	{
		const VValueBag *journal_bag = bag->RetainUniqueElement( LogFileBagKey::journal_file );
		if ( journal_bag != NULL )
		{
			VString logFilePath;
			journal_bag->GetString( LogFileBagKey::filepath, logFilePath );
			journal_bag->GetVUUID( LogFileBagKey::datalink, outDataLink );
			journal_bag->Release();

			if( !logFilePath.IsEmpty() )
			{
				// test if this is a relative path
				if ( logFilePath.GetUniChar(1) == '.' )
				{
					VString folderSeparator;
					folderSeparator = XBOX::FOLDER_SEPARATOR;

					// remove the 2 first char of the relative path : ./
					logFilePath.Remove(1,2);

					// change all the / to the current folder separator
					logFilePath.Exchange(CVSTR("/"),folderSeparator,1,logFilePath.GetLength());

					// insert the data parent folder path
					logFilePath.Insert( inDataFilePath.GetPath(), 1);
				}
				outFilePath.FromFullPath(logFilePath);
			}
		}
		bag->Release();
	}
}

bool Base4D::GetJournalUUIDLink( VUUID &outLink )
{
	bool uuidFound = false;
	outLink.Clear();

	VError bagError;
	const VValueBag* bag = RetainExtraProperties( bagError, nil );
	if ( bag != NULL )
	{
		const VValueBag *journal_bag = bag->RetainUniqueElement( LogFileBagKey::journal_file );
		if ( journal_bag != NULL )
		{
			uuidFound = journal_bag->GetVUUID( LogFileBagKey::datalink, outLink );
			journal_bag->Release();
		}
	}

	return uuidFound;
}

VError Base4D::CreateJournal( VFile *inFile, VUUID *inDataLink, bool inWriteOpenDataOperation)
{
	VError err32 = VE_OK;
	
	err32 = SetJournalFile(inFile,inDataLink,true);
	if (err32  == VE_OK)
	{
		if(inWriteOpenDataOperation)
		{
			WriteLog(DB4D_Log_OpenData, nil, false);
		}
	}
	else
	{
		fLogErr = ThrowError( err32, DBaction_OpeningJournal);
	}
	return fLogErr;
}


VError Base4D::OpenJournal( VFile *inFile, VUUID &inDataLink, bool inWriteOpenDataOperation)
{
	if ( inFile != nil && inFile->Exists() )
	{
		VError err2;
		fLogStream = NULL;
		fLogIsValid = false;
		ReleaseRefCountable(&fLogErrorStack);
		VFileStream* logStream = new VFileStream( inFile );
		err2 = logStream->OpenReading();
		if (err2  == VE_OK )
		{
			sLONG8 logeof;
			bool mustchangeeof = false;
			err2 = inFile->GetSize(&logeof);
			if (err2 == VE_OK)
			{
				DB4DJournalHeader journalHeader;
				err2 = journalHeader.ReadFromStream(logStream);
				if (err2 == VE_OK)
				{
					if (logeof > logStream->GetPos()+8)
					{
						logStream->SetPos(logeof-8);
						sLONG len;
						uLONG tag;
						err2 = logStream->GetLong(len);
						if (err2 == VE_OK)
							err2 = logStream->GetLong(tag);
						if (err2 == VE_OK && tag != kTagLogDB4DEnd)
						{
							bool found = false;
							while (logeof>0 && !found)
							{
								logeof--;
								logStream->SetPos(logeof-8);
								err2 = logStream->GetLong(len);
								if (err2 == VE_OK)
									err2 = logStream->GetLong(tag);
								if (err2 == VE_OK)
								{
									if (tag == kTagLogDB4DEnd)
									{
										logStream->SetPos(logeof-len);
										uLONG tagbegin;
										err2 = logStream->GetLong(tagbegin);
										if (err2 == VE_OK && tagbegin == kTagLogDB4D)
										{
											found = true;
											mustchangeeof = true;
										}
									}
								}

							}
							if (!found)
								err2 = ThrowError( VE_DB4D_LOGFILE_IS_INVALID, DBaction_OpeningJournal);
						}
						if (err2 == VE_OK)
						{
							logStream->SetPos(logeof-len);
							uLONG tagbegin;
							err2 = logStream->GetLong(tagbegin);
							if (err2 == VE_OK && tagbegin != kTagLogDB4D)
								err2 = ThrowError( VE_DB4D_LOGFILE_IS_INVALID, DBaction_OpeningJournal);
							if (err2 == VE_OK)
							{
								err2 = logStream->GetLong8(fCurrentLogOperation);
								if (err2 == VE_OK)
								{
									if (fCurrentLogOperation != hbbloc.lastaction)
									{
										if (fCurrentLogOperation == (hbbloc.lastaction + 1))
										{
											sLONG lenx;
											err2 = logStream->GetLong(lenx);
											if (err2 == VE_OK)
											{
												sLONG action;
												err2 = logStream->GetLong(action);
												if (err2 == VE_OK)
												{
													if ((DB4D_LogAction)action != DB4D_Log_CloseData)
														err2 = ThrowError( VE_DB4D_LOGFILE_LAST_OPERATION_DOES_NOT_MATCH, DBaction_OpeningJournal);
													else
														hbbloc.lastaction = fCurrentLogOperation;
												}
											}
										}
										else
											err2 = ThrowError( VE_DB4D_LOGFILE_LAST_OPERATION_DOES_NOT_MATCH, DBaction_OpeningJournal);
									}
								}
							}
						}
					}
					else
					{
						// this is an empty log that may have been created just after a fullbackup
						fCurrentLogOperation = hbbloc.lastaction;
						if (logStream->GetPos() != logeof)
						{
							logeof = logStream->GetPos();
							mustchangeeof = true;
						}
					}

					logStream->CloseReading();

					if (mustchangeeof)
					{
						VFileDesc* ff = nil;
						VError err3 = inFile->Open(FA_READ_WRITE, &ff);
						if (err3 == VE_OK)
						{
							ff->SetSize(logeof);
						}
						if (ff != nil)
							delete ff;
					}

					if (err2 == VE_OK)
					{
						if (journalHeader.IsBeforeV14())
						{
							err2 = ThrowError( VE_DB4D_LOGFILE_ISBEFOREV14, DBaction_OpeningJournal);
						}
						else if (journalHeader.IsValid(inDataLink) )
						{
							err2 = logStream->OpenWriting();
							if (err2  == VE_OK )
							{
								logStream->SetBufferSize(0);
								logStream->SetPos(logStream->GetSize());
								fLogFile = inFile; 
								fLogFile->Retain();
								fLogStream = logStream;
								fLogIsValid = true;
								if (inWriteOpenDataOperation)
									WriteLog(DB4D_Log_OpenData, nil, false);
								/*
								if (mustchangeeof)
									err2 = ThrowError( VE_STREAM_EOF, DBaction_OpeningJournal);
									*/
							}
						}
						else
						{
							err2 = ThrowError( VE_DB4D_LOGFILE_DOES_NOT_MATCH_DATABASE, DBaction_OpeningJournal);
						}
					}
				}
			}
		}

		if ( !fLogStream )
			delete logStream;

		fLogErr = err2;
	}
	else
		fLogErr = ThrowError( VE_DB4D_LOGFILE_NOT_FOUND, DBaction_OpeningJournal);

	return fLogErr;
}


VError Base4D::CheckIndexesOnSubTables()
{
	VError err = VE_OK;

	for (TableArray::Iterator cur = fich.First(), end = fich.End(); cur != end && err == VE_OK; ++cur)
	{
		Table* tt = *cur;
		if (tt != nil)
		{
			err = tt->CheckIndexesOnPrimKeys();
		}
	}

	for (V0ArrayOf<Relation*>::Iterator cur = fRelations.First(), end = fRelations.End(); cur != end && err == VE_OK; cur++)
	{
		Relation* rel = *cur;
		if (rel != nil)
		{
			if (rel->IsForSubtable())
			{
				Field* source = rel->GetSource();
				Field* dest = rel->GetDest();

				if (source != nil)
				{
					if (!source->IsIndexed() && (source->GetOwner()->GetDF() != nil))
						err = CreIndexOnField(source, DB4D_Index_BtreeWithCluster, false, nil);

				}

				if (dest != nil && err == VE_OK)
				{
					if (!dest->IsIndexed() && (dest->GetOwner()->GetDF() != nil))
						err = CreIndexOnField(dest, DB4D_Index_Btree, false, nil);

				}
			}
		}
	}

	return err;
}


VError Base4D::OpenData( const VFile& inFile, sLONG inParameters, Boolean BuildResMap, Boolean KeepFlushInfoIfClose, FileAccess inAccess)
{
	if (fIsDataOpened)
		return kErr4DBaseAlreadyOpen;

#if debuglr == 2
	DebugMsg(L"Open Data : ");
	VString sdebug;
	inFile.GetPath(sdebug);
	DebugMsg(sdebug);
	DebugMsg(L"\n");
#endif

	fFlushWasNotComplete = false;

	if (inAccess == FA_READ)
	{
		if (fFlushInfo != nil)
		{
			BaseFlushInfo *flushInfo = fFlushInfo;
			fFlushInfo = nil;

			flushInfo->Dispose(); // c'est ici que l'on attend la fin des flush sur une base
		}
		fWriteProtected = true;
	}
	else
	{
		fWriteProtected = false;
		if (fFlushInfo == nil)
			fFlushInfo = fFlusher->NewBaseFlushInfo(this);
	}

	VError err = LoadDataSegments( inFile, (inParameters & DB4D_Open_DO_NOT_Match_Data_And_Struct) != 0, inAccess, 
							(inParameters & DB4D_Open_Convert_To_Higher_Version) != 0, (inParameters & DB4D_Open_Allow_Temporary_Convert_For_ReadOnly) != 0,
							(inParameters & DB4D_Open_AllowOneVersionMoreRecent) != 0);

	if (err == VE_OK && BuildResMap)
	{
		err = CreateResMap();
	}

	if (err == VE_OK)
	{
		if (!hbbloc.IDisNotAnID && IsWakandaRunningMode())
		{
			hbbloc.IDisNotAnID = 1;
			hbbloc.filler2 = 0;
			hbbloc.ExtraElements_addrtabaddr = 0;
			hbbloc.nbExtraElements = 0;
		}
		VIntlMgr* oldmgr = fIntlMgr;

		if (BuildResMap)
		{
			if (hbbloc.dialect == 0 || hbbloc.dialect == (uLONG)-4444)
			{
				fIntlMgr = VIntlMgr::GetDefaultMgr();
				if (fIntlMgr == nil)
					err = VE_DB4D_NOTIMPLEMENTED;
				else
				{
					fMustRebuildAllIndex = true;
					fIntlMgr->Retain();
					hbbloc.dialect = fIntlMgr->GetDialectCode();
					hbbloc.collatorOptions = fIntlMgr->GetCollator()->GetOptions();
				}
			}
			else
			{
				fIntlMgr = VIntlMgr::Create(hbbloc.dialect, hbbloc.collatorOptions);
				if (fIntlMgr == nil)
					err = memfull; // pas de throw volontairement, celui ci doit etre fait par VIntlMgr
			}
		}
		else
		{
			if (hbbloc.dialect == 0 || hbbloc.dialect == (uLONG)-4444)
			{
				fIntlMgr = fStructure->GetIntlMgr();
				if (fIntlMgr == nil)
					fIntlMgr = VIntlMgr::GetDefaultMgr();

				fIntlMgr->Retain();
				fMustRebuildAllIndex = true;
				hbbloc.dialect = fIntlMgr->GetDialectCode();
				hbbloc.collatorOptions = fIntlMgr->GetCollator()->GetOptions();
			}
			else
			{
				fIntlMgr = VIntlMgr::Create(hbbloc.dialect, hbbloc.collatorOptions);
				if (fIntlMgr == nil)
					err = memfull; // pas de throw volontairement, celui ci doit etre fait par VIntlMgr
				else
				{
					if (StoredAsXML())
						CopyRefCountable(&(fStructure->fIntlMgr), fIntlMgr);
					else
						fStructure->SetIntlMgr(fIntlMgr, nil);
				}
			}
		}

		if (oldmgr != nil)
			oldmgr->Release();
	}

	if (err == VE_OK)
		LoadAllTableDefsInData(false);

	if (fNeedRebuildDataTableHeader && err == VE_OK)
	{
		if (fWriteProtected)
		{
			err = ThrowError(VE_DB4D_CANNOT_UPGRADE_DATABASE_FORMAT, DBaction_OpeningDataFile);
		}
		else
		{
			DataAddr4D oldtableaddr = hbbloc.DataTables_addrtabaddr;
			sLONG oldnbDataTable = hbbloc.nbDataTable;
			hbbloc.nbDataTable = 0;
			hbbloc.DataTables_addrtabaddr = 0;
			hbbloc.DataTables_debuttrou = kFinDesTrou;

			fTableDataTableTabAddr.Init(this, this, &hbbloc.DataTables_addrtabaddr, &hbbloc.DataTables_debuttrou, &hbbloc.nbDataTable, nil, nil, 0);

			for (sLONG i = 1; i <= oldnbDataTable; i++)
			{
				Table* assoc = nil;
				if (i<=fNbTable)
				{
					assoc = fich[i];
				}
				if (assoc != nil)
				{
					
					oldDataFileDISK oldDFD;
					VError err = readlong(&oldDFD, sizeof(oldDFD), oldtableaddr, (i-1)*sizeof(oldDataFileDISK)+kSizeDataBaseObjectHeader);
					xbox_assert(err == VE_OK);

	#if BIGENDIAN
					oldDFD.SwapBytes();
	#endif

					DataTableDISK DFD;
					*((oldDataFileDISK*)&DFD) = oldDFD;
					DFD.debutTransRecTrou = kFinDesTrou;
					DFD.debutTransBlobTrou = kFinDesTrou;
					DFD.fKeepStamps = 0;
					DFD.fPrimKeyState = PrimKeyState_notVerified;
					DFD.fillerByte2 = 0;
					DFD.fillerByte3 = 0;
					DFD.fLastRecordStamp = 0;
					DFD.filler8 = 0;
					DFD.fLastRecordSync = 0;
					VUUID xid;
					assoc->GetUUID(xid);
					xid.ToBuffer(DFD.TableDefID);
					DataTable* df = CreateDataTable(assoc, err, 0, &DFD, -1);
					if (df != nil)
					{
						//df->RemoveFromCache();
						df->Release();
					}
				}
			}

			if (err == VE_OK)
				SaveDataTablesToStructTablesMatching();
		}
	}

	if (err == VE_OK)
		err = LoadSeqNums();

	if (err == VE_OK && !fNeedRebuildDataTableHeader)
		err = LoadDataTables();

	if (err == VE_OK)
	{
		fExtraElements = new ExtraElementTable(this, &hbbloc.ExtraElements_addrtabaddr, &hbbloc.nbExtraElements);
		err = fExtraElements->load();
	}

	if (err == VE_OK)
	{
		if (hbbloc.WithSeparateIndexSegment)
		{
			VError err2 = OpenIndexSegment(inFile, BuildResMap, inAccess);
			if (err2 != VE_OK)
			{
				hbbloc.IndexDef_debuttrou = kFinDesTrou;
				hbbloc.nbIndexDef = 0;
				hbbloc.IndexDef_addrtabaddr = 0;			
			}
			else
			{
				if (fIndexSegment != nil)
					if (fIndexSegment->GetFileDesc()->GetMode() == FA_READ)
						fWriteProtected = true;
			}
			
		}

		if (err == VE_OK)
		{
			fIsDataOpened = true;
			//fIsParsingStruct = true;
			if (IsWakandaRunningMode())
				err = SecondPassLoadEntityModels(nil, false);
			if (err != VE_OK)
				fIsDataOpened = false;
			//fIsParsingStruct = false;
		}
		if (err == VE_OK)
			err = LoadIndexesDef();

		if (err == VE_OK && (inParameters & DB4D_Open_DelayLoadIndex) == 0)
			err = LoadIndexes((inParameters & DB4D_Open_DO_NOT_Build_Index) != 0);
	}

	if (err == VE_OK)
		fIsDataOpened = true;
	else
	{
		err = ThrowError(VE_DB4D_CANNOTOPENDATAFILE, DBaction_OpeningDataFile);
		setmodif(false, this, nil);
		CloseData(KeepFlushInfoIfClose);
	}

	fLogErr = VE_OK;

	SaveDataTablesToStructTablesMatching();

	if (err == VE_OK && fIsDataOpened && !fWriteProtected)
	{
		if (fStructure != nil || !IsWakandaRunningMode())
		{
			if (fIntlMgr != nil)
			{
				VString BaseStringCompHash, BaseKeywordBuildingHash;
				VString IntlStringCompHash, IntlKeywordBuildingHash;

				GetIntlInfo(BaseStringCompHash, BaseKeywordBuildingHash);
				fIntlMgr->GetStringComparisonAlgorithmSignature(IntlStringCompHash);
				fIntlMgr->GetKewordAlgorithmSignature(IntlKeywordBuildingHash);

				if (BaseStringCompHash != IntlStringCompHash || BaseKeywordBuildingHash != IntlKeywordBuildingHash)
				{
					SetIntlInfo(IntlStringCompHash, IntlKeywordBuildingHash);
				}
			}
		}
	}

	if (err == VE_OK && fIsDataOpened && fStructure != nil && !fWriteProtected)
	{
		for (sLONG i = 1; i <= fNbTable && err == VE_OK; ++i)
		{
			Table* tt = fich[i];
			if (tt != nil)
			{
				DataTable* df = tt->GetDF();
				if (df != nil)
				{
					err = df->SetKeepStamp(true, nil);
				}
				tt->CanNowKeepStamp(true);
			}
		}
	}

	if (err == VE_OK && !fWriteProtected && (inParameters & DB4D_Open_DelayLoadIndex) == 0)
	{
		err = CheckIndexesOnSubTables();
	}

	//LoadFieldIdsInRec();

	if ( err == VE_OK && !BuildResMap && (inParameters & DB4D_Open_WITHOUT_JournalFile) == 0 )
	{
		StErrorContextInstaller errs(false);
		VUUID dataLink;
		VFile *logFile = NULL;
		VFolder *parentDataFolder = inFile.RetainParentFolder();
		if ( parentDataFolder )
		{
			VFilePath logFilePath;
			GetJournalInfos( parentDataFolder->GetPath(), logFilePath, dataLink );
			if ( logFilePath.IsEmpty() )
			{
				fLogErr = VE_DB4D_LOGFILE_NOT_SET;
			}
			else
			{
				if ( logFilePath.IsFile() )
				{
					logFile = new VFile( logFilePath );
				}
			}
			parentDataFolder->Release();
		}

		if ( fLogErr == VE_OK )
		{
			if ( logFile != nil )
			{
				err = OpenJournal( logFile, dataLink );
				logFile->Release();
			}
			else
			{
				fLogErr = VE_DB4D_LOGFILE_NOT_FOUND;
			}
		}
	}
	
		
	if (err == VE_OK)
	{
		err = SelectTemporaryFolder( fStructure == nil);
	}

	if (!BuildResMap)
	{
		VFolder* parent = inFile.RetainParentFolder();
		if (parent != nil)
		{
			VFile ff(*parent, L"StoreBlobsInRecords.txt");
			if (ff.Exists())
				fAlwaysStoreBlobsInRecs = true;
			parent->Release();
		}
	}

#if debug_Addr_Overlap
	if (err == VE_OK)
	{
		FillDebug_DBObjRef();
	}
#endif
	
	return err;
}


#if debuglog

void Base4D::WriteDebugLogData(void* p, sLONG len)
{
	if (fDebugLog == nil && fStructure != nil)
	{
		VFile* debuglogfile =  nil;
		VFolder* datafolder = RetainDataFolder();
		if (datafolder != nil)
		{
			if (fStructure == nil)
			{
				debuglogfile = new VFile(*datafolder, L"debugLogStruct.txt");
			}
			else
			{
				debuglogfile = new VFile(*datafolder, L"debugLogData.txt");
			}

			if (debuglogfile->Exists())
				debuglogfile->Delete();
			VError err = debuglogfile->Create(true);
			if (err != VE_OK)
			{
				ReleaseRefCountable(&debuglogfile);
			}

			if (debuglogfile != nil)
			{
				fDebugLog = new VFileStream(debuglogfile);
				if (fDebugLog->OpenWriting() != VE_OK)
				{
					delete fDebugLog;
					fDebugLog = nil;
				}
				debuglogfile->Release();
			}

			QuickReleaseRefCountable(datafolder);
		}
	}

	if (fDebugLog != nil)
	{
		fDebugLog->PutData(p, len);
		//fDebugLog->Flush();
	}
}

void Base4D::WriteDebugLogStr(const VString& s)
{
	WriteDebugLogData((void*)s.GetCPointer(), s.GetLength()*2);
}

void Base4D::WriteDebugLogLong(sLONG n)
{
	VStr<32> s;
	s.FromLong(n);
	WriteDebugLogStr(s);
}

void Base4D::WriteDebugLogAddr(DataAddr4D addr)
{
	VStr<32> s;
	s.FromLong8(addr);
	WriteDebugLogStr(s);
}

void Base4D::WriteDebugLogLn()
{
	VStr<32> s(L"\n");
	WriteDebugLogStr(s);
}

void Base4D::WriteDebugLogLn(const VString& mess)
{
	VStr<32> s(L"\n");
	WriteDebugLogStr(mess);
	WriteDebugLogStr(s);
}

void Base4D::WriteDebugHex(void* p, sLONG len)
{
	UniChar hex[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
	const sLONG sizebuf = 8192;
	UniChar buffer[sizebuf];
	sLONG curp = 0;
	uCHAR *px = (uCHAR*)p;

	while (len>0)
	{
		if (curp>=sizebuf)
		{
			WriteDebugLogData(&buffer[0], sizebuf*2);
			curp = 0;
		}
		uCHAR c = *px;
		buffer[curp] = hex[(c>>4)];
		buffer[curp+1] = hex[c&0xF];
		curp = curp + 2;
		len--;
		px++;
	}
	if (curp>0)
	{
		WriteDebugLogData(&buffer[0], curp*2);
	}
}

void Base4D::WriteDebugFlush()
{
	if (fDebugLog != nil)
	{
		fDebugLog->Flush();
	}
}


#endif


VError Base4D::writelong(void* p, sLONG len, DataAddr4D ou, sLONG offset, VString* WhereFrom,sLONG TrueLen)
{
	xbox_assert( this != nil);
	xbox_assert(p != nil || len == 0);

#if debuglogwrite
	VStr<256> s;
	VStr<32> s2;
	s = L"Addr = ";
	s2.FromLong8(ou);
	s += s2;
	s += L" , offset = ";
	s2.FromLong(offset);
	s += s2;
	s += L" , len = ";
	s2.FromLong(len);
	s += s2;
	if (WhereFrom != nil)
	{
		s += "  , from : ";
		s += *WhereFrom;
	}
	s += "\n";
	WriteDebugLogStr(s);
	WriteDebugHex(p, len);
	s = L"\n\n";
	WriteDebugLogStr(s);
#endif

#if debug
	if ( ou == 0 && !debug_OKToWriteIn0)
	{
		sLONG toto = 1;
	}
	xbox_assert( ou != 0 || debug_OKToWriteIn0 );
#endif

#if debugOverWrite_strong
	DataAddr4D orig_ou = ou;
	bool okcheck = false;
#endif

	VError err = VE_OK;
	SegData *sg;
	if (testAssert(!fIsRemote))
	{	
		VIndex i=ou & (kMaxSegData-1);
		ou=ou & (DataAddr4D)(-kMaxSegData);
		if (i>=kMaxSegDataNormaux)
		{
#if debugOverWrite_strong
			okcheck = true;
#endif

			sg = GetSpecialSegment(i);
			if (sg == nil)
			{
				err = ThrowError(VE_DB4D_WRONGDATASEG, DBaction_WritingData);
			}
		}
		else
		{
			if (i>=fSegments.GetCount())
			{
				err = ThrowError(VE_DB4D_WRONGDATASEG, DBaction_WritingData);
			}
			else
			{
				sg = fSegments[i];
			}
		}

		if (err == VE_OK)
		{
#if debugOverWrite_strong
			if (okcheck)
			{
				debug_CheckWriting(orig_ou+(DataAddr4D)offset, len);
			}
#endif
			fBuffersMutex.Lock();

			err=sg->writeat(p,len,ou+(DataAddr4D)offset, TrueLen);

			ReadAheadBuffer* buf = fBuffers.GetFirst();
			while (buf != nil)
			{
				buf->Lock();
				if (i == buf->GetSegment() && ou >= buf->GetPos() && (ou+(DataAddr4D)len+(DataAddr4D)offset) <= (buf->GetPos() + (DataAddr4D)buf->GetLen()) )
				{
					buf->Invalide();
				}
				buf->Unlock();
				buf = buf->GetNext();
			}

			fBuffersMutex.Unlock();
		}
	}
	else
		err = VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE;

	return(err);
}

uLONG gLastReadIconTime = 0;


VError Base4D::readlong(void* p, sLONG len, DataAddr4D ou, sLONG offset, ReadAheadBuffer* buffer)
{
	xbox_assert( this != nil);
	
	sLONG i;
	DataAddr4D oldou;
	SegData *sg;
	VError err = VE_OK;
	
	if (testAssert(!fIsRemote))
	{
		oldou = ou;
		xbox_assert(ou != 0 || p == (void*)&hbbloc);
		i = ou & (kMaxSegData-1);
		ou = ou & (DataAddr4D)(-kMaxSegData);

		if (i>=kMaxSegDataNormaux)
		{
			sg = GetSpecialSegment(i);
			if (sg == nil)
			{
				err = ThrowError(VE_DB4D_WRONGDATASEG, DBaction_WritingData);
			}
		}
		else
		{
			if (i>=fSegments.GetCount())
			{
				err = ThrowError(VE_DB4D_WRONGDATASEG, DBaction_WritingData);
			}
			else
			{
				sg = fSegments[i];
			}
		}


		if (err == VE_OK)
		{
			Boolean mustread = false;

			if (buffer != nil)
			{
				if (buffer->TryToLock())
				{
					if (i == buffer->GetSegment() && ou >= buffer->GetPos() && (ou+(DataAddr4D)len+(DataAddr4D)offset) <= (buffer->GetPos() + (DataAddr4D)buffer->GetLen()) )
					{
						// on est dans le buffer
						err = buffer->GetData(p, len, ou, offset);
					}
					else
					{
						if (sg->GetSemaphore()->TryToLockRead())
						{
							if (len+offset > buffer->GetMaxLen())
							{
								uLONG t = VSystem::GetCurrentTime();
								if (t - gLastReadIconTime > 250)
								{
									gLastReadIconTime = t;
									if (VDBMgr::GetActivityManager() != nil)
										VDBMgr::GetActivityManager()->SetActivityRead(true, 300);
								}
							// la taille a lire est plus grosse que le buffer, alors on lit en dehors
								err=sg->readat(p,len,ou+(DataAddr4D)offset);
							}
							else
							{
								sLONG lentoread = buffer->GetMaxLen();
								if (ou + (DataAddr4D)lentoread > sg->Getfinfic())
									lentoread = (sLONG)(sg->Getfinfic() - ou);
								if (lentoread > 0)
								{
									err = buffer->ReadFrom(oldou, lentoread);
									if (err == VE_OK)
										err = buffer->GetData(p, len, ou, offset);
								}
								else
									err=sg->readat(p,len,ou+(DataAddr4D)offset);
								
							}
							sg->GetSemaphore()->Unlock();
						}
						else
							mustread = true;
					}
					buffer->Unlock();
				}
				else
					mustread = true;
			}
			else
				mustread = true;

			if (mustread)
			{
				uLONG t = VSystem::GetCurrentTime();
				if (t - gLastReadIconTime > 250)
				{
					gLastReadIconTime = t;
					if (VDBMgr::GetActivityManager() != nil)
						VDBMgr::GetActivityManager()->SetActivityRead(true, 300);
				}
				err=sg->readat(p,len,ou+(DataAddr4D)offset);
			}
		}
	}
	else
		err = VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE;

	return(err);
}


uBOOL Base4D::IsAddrValid(DataAddr4D ou)
{
	VIndex i=ou & (kMaxSegData-1);
	ou=ou & (DataAddr4D)(-kMaxSegData);

	if (i>=kMaxSegDataNormaux)
	{
		SegData *sg = GetSpecialSegment(i);
		if (sg == nil)
			return false;
		return sg->IsAddrValid(ou);
	}
	else
	{
		if (i >= fSegments.GetCount())
			return false;
		SegData *sg = fSegments[i];
		return sg->IsAddrValid(ou);
	}

}


const UniChar* objnames[] = 
{ 
(const UniChar *) L"t_Obj4D", (const UniChar *) L"t_ObjCache", (const UniChar *) L"t_Base4D", (const UniChar *) L"t_Datafile", (const UniChar *) L"t_bittable", 
(const UniChar *) L"t_addrtab",(const UniChar *)  L"t_addrdefictable",
(const UniChar *) L"t_ficheondisk",(const UniChar *)  L"t_treeinmem",(const UniChar *)  L"t_treedeficinmem",(const UniChar *)  L"t_segdata",(const UniChar *)  L"t_addrindtable",(const UniChar *)  L"t_btreepageindex",
(const UniChar *) L"t_IndexHeaderBTree",(const UniChar *)  L"t_addrdeblobtable",(const UniChar *)  L"t_blob4d",(const UniChar *)  L"t_treedeblobinmem",
(const UniChar *) L"t_addrdeselclustertable",(const UniChar *)  L"t_treedeselclusterinmem",(const UniChar *)  L"t_segdatapagesec",(const UniChar *)  L"t_btreepagefixsizeindex",
(const UniChar *) L"t_IndexHeaderBTreeFixSize",(const UniChar *)  L"t_locker",(const UniChar *)  L"t_tabledef",(const UniChar *)  L"t_relationdef",(const UniChar *)  L"t_petitesel",(const UniChar *)  L"t_longsel", (const UniChar *) L"t_bitsel"
};


bool Base4D::CanFindDiskSpace(VSize nbBytesToFind)
{
	bool result = false;
	if (fSegments.GetCount() > 0)
	{
		sLONG8 nbfreebytes = 0;
		fSegments[0]->GetFile()->GetVolumeFreeSpace( &nbfreebytes, NULL, true, 5000); // on interroge toutes les 5 sec
		result = ((sLONG8)nbBytesToFind + (sLONG8)(100*1024*1024)) < nbfreebytes;
	}
	return result;
}


DataAddr4D Base4D::findplace(sLONG len, BaseTaskInfo* context, VError& err, sLONG prefseg, void *obj)
{
	sLONG i,nb;
	DataAddr4D ou;
	uBOOL uniq;
	err = VE_OK;
	
	ou=-1;
	uniq=false;
	
	if (testAssert(!fIsRemote))
	{
		// prefseg = 0 est une valeur valide
		if (prefseg!=0)
		{
			if (prefseg<0)
			{
				uniq=true;
				prefseg=(-prefseg);
			}

			if (prefseg == kIndexSegNum && !hbbloc.WithSeparateIndexSegment)
			{
				uniq = false;
			}
			else
			{
				i=prefseg-1;

				if (i>=kMaxSegDataNormaux)
				{
					SegData* seg = GetSpecialSegment(i);
					if (seg == nil)
					{
						if (i == kIndexSegNum - 1)
						{
							VError err2 = CreateIndexSegment(*(fSegments[0]->GetFile()));
							if (err2 == VE_OK)
							{
								seg = GetSpecialSegment(i);
							}
						}
					}
					
					if (seg != nil)
						ou = seg->FindFree(len, obj)+i;
					else
						err = ThrowError(VE_DB4D_WRONGDATASEG, DBaction_LookingForFreeDiskSpace);
				}
				else
				{
					if (i<fSegments.GetCount())
						ou = fSegments[i]->FindFree(len, obj)+i;
					else
						err = ThrowError(VE_DB4D_WRONGDATASEG, DBaction_LookingForFreeDiskSpace);
				}
			}
		}
		
		if ((ou==-1) && (!uniq))
		{
			nb=fSegments.GetCount();
			for (i=0; (i<nb) && (ou==-1); i++)
			{
				SegData* seg = fSegments[i];
				ou = seg->FindFree(len, obj) + i;
			}
		}
		/*
		if (ou != -1)
		{
			FlushErrors();
			err = VE_OK;
		}
		*/

		if ((ou==-1) && (err==VE_OK))
		 err = ThrowError(VE_DB4D_SEGFULL, DBaction_LookingForFreeDiskSpace);
		 
		xbox_assert(ou != -1);
		
		/*
		Transaction* curtrans = GetCurTrans(context);
		
		if (curtrans != nil)
		{
			err = curtrans->AddFindPlace(this, ou, len);
		}
		*/

	#if debuglogplace
		if (fStructure != nil)
		{
			VStr<128> s(L"Findplace : ");
			VStr<32> s2;
			/*
			if (false && obj != nil)
			{
				typobjcache typ = obj->GetType();
				VStr<32> s3(objnames[typ]);
				s += s3;
				s += " : ";
			}
			*/
			s2.FromLong(len);
			s += s2;
			s += L"  --> ";
			s2.FromLong8(ou);
			s += s2;
			s += "\n";
			DebugMsg(s);
		}
	#endif

#if debugFindPlace_log
		DebugMsg(L"Findplace : "+ToString(ou)+L"  ,  obj = "+ToString((sLONG8)obj)+"\n\n");
#endif
	}

	return(ou);
}


VError Base4D::libereplace(DataAddr4D ou, sLONG len, BaseTaskInfo* context, void *obj)
{
	sLONG i;
	VError err = VE_OK;
	

#if debugFindPlace_log
	DebugMsg(L"LiberePlace : "+ToString(ou)+L"  ,  obj = "+ToString((sLONG8)obj)+"\n\n");
#endif

#if debuglogplace
	if (fStructure != nil)
	{
		VStr<128> s(L"LiberePlace : ");
		VStr<32> s2;
		/*
		if (false && obj != nil)
		{
			typobjcache typ = obj->GetType();
			VStr<32> s3(objnames[typ]);
			s += s3;
			s += " : ";
		}
		*/
		s2.FromLong(len);
		s += s2;
		s += L"  ==> ";
		s2.FromLong8(ou);
		s += s2;
		s += "\n";
		DebugMsg(s);
	}
#endif

	/*
	Transaction* curtrans = GetCurTrans(context);
	
	if (curtrans != nil)
	{
		err = curtrans->AddLiberePlace(this, ou, len);
	}
	else
	*/
	if (testAssert(!fIsRemote))
	{	
		i=ou & (kMaxSegData-1);
		ou=ou & (DataAddr4D)(-kMaxSegData);
		
		if (i>=kMaxSegDataNormaux)
		{
			SegData* seg = GetSpecialSegment(i);
			if (seg == nil)
			{
				err = ThrowError(VE_DB4D_WRONGDATASEG, DBaction_FreeingDiskSpace);
			}
			else
			{
				err = seg->LibereBits(ou, len, obj);
			}
		}
		else
		{
			if (i>=fSegments.GetCount())
			{
				err = ThrowError(VE_DB4D_WRONGDATASEG, DBaction_FreeingDiskSpace);
			}
			else
			{
				err = fSegments[i]->LibereBits(ou, len, obj);
			}
		}
	}
	else
		err = VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE;
	
	return err;
}


#if debugFindPlace_strong

void Base4D::Debug_CheckAddrOverWrite(IObjToFlush* obj)
{
	DataAddr4D ou = obj->getaddr();
	sLONG i=ou & (kMaxSegData-1);
	ou=ou & (DataAddr4D)(-kMaxSegData);

	if (i>=kMaxSegDataNormaux)
	{
		SegData* seg = GetSpecialSegment(i);
		if (seg != nil)
		{
			seg->Debug_CheckAddrOverWrite(obj, ou);
		}
	}
	else
	{
		if (i<fSegments.GetCount())
		{
			fSegments[i]->Debug_CheckAddrOverWrite(obj, ou);
		}
	}

}

#endif


void Base4D::MarkBlockToFullyDeleted(DataAddr4D ou, sLONG len)
{
	sLONG i=ou & (kMaxSegData-1);
	ou=ou & (DataAddr4D)(-kMaxSegData);

	if (i>=kMaxSegDataNormaux)
	{
		SegData* seg = GetSpecialSegment(i);
		if (seg != nil)
		{
			seg->MarkBlockToFullyDeleted(ou, len);
		}
	}
	else
	{
		if (i<fSegments.GetCount())
		{
			fSegments[i]->MarkBlockToFullyDeleted(ou, len);
		}
	}
}

void Base4D::SwapAllSegsFullyDeletedObjects(DataAddrSetVector& AllSegsFullyDeletedObjects)
{
	AllSegsFullyDeletedObjects.resize(kIndexSegNum);
	SegData* seg = GetSpecialSegment(kIndexSegNum - 1);
	if (seg != nil)
	{
		DataAddrSetForFlush* px = &(AllSegsFullyDeletedObjects[kIndexSegNum-1]);
		seg->SwapFullyDeletedAddr(px->fAddrs);
		px->fCurrent = px->fAddrs.begin();
		px->fLast = px->fAddrs.end();
	}
	for (sLONG i = 0; i < fSegments.GetCount(); i++)
	{
		DataAddrSetForFlush* px = &(AllSegsFullyDeletedObjects[i]);
		fSegments[i]->SwapFullyDeletedAddr(px->fAddrs);
		px->fCurrent = px->fAddrs.begin();
		px->fLast = px->fAddrs.end();
	}
}



const uLONG kTagSegTable = 0x44074402;

VError Base4D::SaveDataSegments()
{
	sLONG i;
	VError err = VE_OK;
	DataAddr4D ou;
	xbox::VPtrStream bb;
	VString s;
	
	if (fSegments.GetCount() > 1)
	{
		err=bb.OpenWriting();
		bb.SetNeedSwap(false);
		err = bb.PutLong(kTagSegTable);

		for (i=0;(i<fSegments.GetCount()) && (err==VE_OK);i++) {
			VFilePath path;
			fSegments[i]->GetFullPath( path);
			s = path.GetPath();
			err=s.WriteToStream( &bb);
		}
		
		if (err==VE_OK)
		{
			if (hbbloc.addrmultisegheader>0)
			{
				if ( ((bb.GetSize()+kSizeDataBaseObjectHeader+ktaillebloc-1) & (-ktaillebloc)) == ((hbbloc.lenmultisegheader+kSizeDataBaseObjectHeader+ktaillebloc-1) & (-ktaillebloc)) )
				{
					hbbloc.lenmultisegheader=bb.GetSize();
				}
				else
				{
					ou=findplace(bb.GetSize()+kSizeDataBaseObjectHeader, nil ,err, -1, this);
					if (ou>0)
					{
						// L.E. 10/12/1999 libereplace moved before !
						libereplace (hbbloc.addrmultisegheader, hbbloc.lenmultisegheader+kSizeDataBaseObjectHeader, nil, this);
						hbbloc.lenmultisegheader=bb.GetSize();
						hbbloc.addrmultisegheader=ou;
					}
				}
			}
			else
			{
				hbbloc.lenmultisegheader=bb.GetSize();
				hbbloc.addrmultisegheader=findplace(hbbloc.lenmultisegheader+kSizeDataBaseObjectHeader, nil, err, -1, this);
			}
		}
		
		if (err==VE_OK)
		{
	#if debuglogwrite
			VString wherefrom(L"Save Data Segments");
			VString* whx = &wherefrom;
	#else
			VString* whx = nil;
	#endif
			DataBaseObjectHeader tag(bb.GetDataPtr(), hbbloc.lenmultisegheader, DBOH_MultiSegHeader, -1, -1);
			err = tag.WriteInto(this, hbbloc.addrmultisegheader, whx);
			if (err == VE_OK)
				err = writelong (bb.GetDataPtr(), hbbloc.lenmultisegheader, hbbloc.addrmultisegheader, kSizeDataBaseObjectHeader, whx);
		}
		
		if (err != VE_OK)
			err = ThrowError(VE_DB4D_CANNOTSAVEDATASEG, DBaction_SavingDataSegs);
			
		bb.CloseWriting(false);
	}
	else
	{
		hbbloc.lenmultisegheader = 0;
		hbbloc.addrmultisegheader = -1;
	}

	return(err);
}


VError Base4D::LoadDataSegments( const VFile& inFile, Boolean DO_NOT_Map_Data_And_Struct, FileAccess InAccess, bool ConvertToHigherVersion, bool allowConvertForReadOnly, bool allowOneVersionMoreRecent)
{
	VFile *firstSegFile = new VFile( inFile);
	sLONG specialretour = 0;

	VFileDesc *firstSegFileDesc = NULL;
	VError err = firstSegFile->Open(InAccess, &firstSegFileDesc);
	if (err == VE_OK) {
		if (firstSegFileDesc->GetMode() == FA_READ)
			fWriteProtected = true;
		Boolean wasswapped;
		err = ReadBaseHeader( *firstSegFileDesc, &hbbloc, true, DO_NOT_Map_Data_And_Struct, (fStructure == nil) || IsWakandaRunningMode() ? nil : &(fStructure->GetUUID()), &specialretour, wasswapped, ConvertToHigherVersion, allowConvertForReadOnly, allowOneVersionMoreRecent);
		if ( err != VE_OK )
			ThrowError(err, DBaction_ReadingBaseHeader); /* Sergiy - 19 May 2007 - For bug #ACI0051855. */
		fStamp = hbbloc.fStamp;

		if (err == VE_OK)
		{
			if (hbbloc.lastoper != 0)
			{
				fFlushWasNotComplete = true;
				err = ThrowError(VE_DB4D_FLUSH_DID_NOT_COMPLETE, DBaction_ReadingDataSegs);
			}
		}

		if (err == VE_OK) {
			if ((specialretour & kRetourInvalidClusterIndex) != 0)
				fIndexClusterInvalid = true;
			if (((specialretour & kRetourMustRebuildAllIndex) != 0) && !fWriteProtected)
				fMustRebuildAllIndex = true;
			if ((specialretour & kRetourMustRebuildDataTableHeader) != 0)
				fNeedRebuildDataTableHeader = true;
			if ((specialretour & kRetourMustLoadOldRelations) != 0)
				fNeedToLoadOldRelations = true;
			if ((specialretour & kRetourMustConvertIndex) != 0)
				fNeedToConvertIndexes = true;
			if ((specialretour != 0 || wasswapped) && !fWriteProtected)
				firstSegFileDesc->PutData(&hbbloc, sizeof(hbbloc), 0);

			if (fStructure == nil) // quand la base est une structure alors on charge le fID, sinon pour un data le fID est recopi plus loin sur celui de la structure
				fID.FromBuffer(hbbloc.GetID());

			SegData *seg1 = new SegData( this, hbbloc, 1, firstSegFile, firstSegFileDesc, &hbbloc.seginfo );
			firstSegFileDesc = nil;
			firstSegFile = nil;
			fSegments.Add( seg1);
			seg1->PutInCache();


			if (hbbloc.lenmultisegheader > 0)
			{
				void* bbp = GetFastMem(hbbloc.lenmultisegheader, false, 'bbp ');
				if (bbp == nil)
					err = ThrowError(memfull, DBaction_ReadingDataSegs);
				else
				{
					VSize len = hbbloc.lenmultisegheader;
					err = seg1->GetFileDesc()->GetData(bbp, len, hbbloc.addrmultisegheader);
					VConstPtrStream bb( bbp, hbbloc.lenmultisegheader);
					bb.OpenReading();
					uLONG tag = bb.GetLong();
					if (tag != kTagSegTable)
					{
						tag = SwapLong(tag);
						if (tag == kTagSegTable)
							bb.SetNeedSwap(true);
						else
							err = ThrowError(VE_DB4D_WRONGRECORDHEADER, DBaction_ReadingDataSegs);
					}

					if (err == VE_OK) {
						VFilePath parent = inFile.GetPath();
						parent.ToFolder();
						sLONG i;
						for( i = 1 ; (i < hbbloc.nbsegdata) && (err == VE_OK) ; ++i) 
						{
							VStr255	s;
							err = s.ReadFromStream( &bb);
							if (err == VE_OK || err == XBOX_LONG8(8679670649385386184)) 
							{
								err = VE_OK;
								VFilePath path( parent, s);
								VFile *segFile = new VFile( path);
								VFileDesc* fdesc = nil;
								err = segFile->Open(InAccess, &fdesc);
								if (err == VE_OK)
								{
									if (fdesc->GetMode() == FA_READ)
										fWriteProtected = true;
									BaseHeader hbb;
									len = sizeof(BaseHeader);
									Boolean wasswapped2;
									err = ReadBaseHeader(*fdesc, &hbb, false, DO_NOT_Map_Data_And_Struct, (fStructure == nil) || IsWakandaRunningMode() ? nil : &(fStructure->GetUUID()), nil, wasswapped2, ConvertToHigherVersion, allowConvertForReadOnly, allowOneVersionMoreRecent);
									//err = fdesc->GetData(&bbp, len, 0, true);
									if (err == VE_OK)
									{
										seg1 = new SegData( this, hbb, i+1, segFile, fdesc, nil );
										fdesc = nil;
										segFile = nil;
										fSegments.Add( seg1);
										seg1->PutInCache();

										err = seg1->OpenSeg(InAccess);
									}
								}
								if (fdesc != nil)
								{
									delete fdesc;
								}
								if (segFile != nil)
								{
									segFile->Release();
								}
							}
						}
					}
					bb.CloseReading();
				}
			}
		}
	}

	if (firstSegFile != nil) {
		firstSegFile->Release();
	}
	if (firstSegFileDesc != nil)
		delete firstSegFileDesc;

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTREADDATASEG, DBaction_ReadingDataSegs);
	return err;
}


sLONG Base4D::GetBaseHeaderSize() const
{
	return fBaseHeaderSize;
}


DataAddr4D Base4D::GetMultiSegHeaderAddress() const
{
	return hbbloc.addrmultisegheader;
}


void Base4D::xInit(Boolean FromConstructor)
{
	SetInProtectedArea();
	fFieldsIDInRec = nil;
	fFlushWasNotComplete = false;
	fDontUseEntityCatalog = false;
	fStoreAsXML = false;
	fStructXMLFile = nil;
	fUAGDirectory = nil;
	fIndexesBag = nil;
	fAlwaysStoreBlobsInRecs = false;
	fBlobsFolder = nil;
	fCatalogJSFile = nil;
	fExtraElements = nil;
	fWakandaMode = false;
	fRiaServerProjectRef = nil;
	fBaseFileSystemNamespace = nil;

	if (FromConstructor)
	{
		fStructFolder = nil;
		fStructure = nil;
		fIntlMgr = nil;
		fStructure = nil;
		fTriggerManager = nil;
//		BaseRef = nil;
		fRegistered = false;
//		BaseNameSpace = nil;
		//special_pour_index.SetDB(this);
		IndexSemaphore = new Obj4D;
//		special_pour_index.libere();
		fIsNotifying = false;
		fIsClosing = false;
		fIsParsingStruct = false;
		fIsStructureOpened = false;

		fTableDataTableTabAddr.libere();
		fTableDefTabAddr.libere();
		fTableDefInMem.libere();
		fRelationDefTabAddr.libere();
		fRelationDefInMem.libere();
		fTableSeqNumTabAddr.libere();
		fIndexesTabAddr.libere();
		fIndexesInMem.libere();
		fIndexesInStructTabAddr.libere();
		fIndexesInStructInMem.libere();
		fBaseX = nil;
		fNbTable = 0;
		fStampExtraProp = 1;	// 0 has a special meaning for c/s synchro
		fRemoteStructureStampExtraProp = 1;
		fRemoteStructureExtraProp = nil;
		fClientID.Clear();
		fBackupSettings = nil;
	}

	fSyncHelper = nil;
	fIndexSegment = nil;
	fWriteProtected = false;
	fCountLock = 0;
	fCountDataModif = 0;
	fBaseisLockedBy = nil;
	fNeedsToSaveRelation = false;
	//	fNeedsToSaveIndexes = false;

	fIsDataOpened = false;
	fIndexDelayed = false;

	fBaseHeaderSize = (sizeof(BaseHeader)+127L) & -128L;
	
	hbbloc.tag = tagHeader4D;
	hbbloc.VersionNumber = kVersionDB4D;
	hbbloc.lastoper = 0;
	hbbloc.lastparm = -1;
	hbbloc.IDisNotAnID = 0;
	hbbloc.filler1 = 0;
	hbbloc.ExtraElements_addrtabaddr = 0;
	hbbloc.nbExtraElements = 0;
	hbbloc.filler2 = 0;
	//hbbloc.addrbblock = 0;
	hbbloc.nbsegdata = 0;
	hbbloc.addrmultisegheader = 0;
	hbbloc.lenmultisegheader = 0;
	hbbloc.filfil = VSystem::Random();
	hbbloc.fStamp = 1;
	hbbloc.lastaction = 0;
	hbbloc.countlog = 0;
	hbbloc.nbDataTable = 0;
	hbbloc.DataTables_addrtabaddr = 0;
	hbbloc.DataTables_debuttrou = kFinDesTrou;

	//hbbloc.addrBackStamp = 0;
	hbbloc.LastFlushRandomStamp = 0;
	//hbbloc.embeddedStructureSize = 0;
	//hbbloc.embeddedStructureSpace = 0;
	hbbloc.doischangerfilfil = false;
	hbbloc.backupmatchlog = true;
	hbbloc.WithSeparateIndexSegment = true;
	hbbloc.IsAStruct = false;

	hbbloc.nbRelations = 0;
	hbbloc.Relations_debuttrou = kFinDesTrou;
	hbbloc.Relations_addrtabaddr = 0;
	hbbloc.TableDef_debuttrou = kFinDesTrou;
	hbbloc.TableDef_addrtabaddr = 0;
	hbbloc.nbTableDef = 0;

	hbbloc.nbSeqNum = 0;
	hbbloc.SeqNum_debuttrou = kFinDesTrou;
	hbbloc.SeqNum_addrtabaddr = 0;

	hbbloc.nbIndexDef = 0;
	hbbloc.IndexDef_debuttrou = kFinDesTrou;
	hbbloc.IndexDef_addrtabaddr = 0;

	hbbloc.nbIndexDefInStruct = 0;
	hbbloc.IndexDefInStruct_debuttrou = kFinDesTrou;
	hbbloc.IndexDefInStruct_addrtabaddr = 0;

	hbbloc.ExtraAddr = 0;
	hbbloc.ExtraLen = 0;

	hbbloc.countFlush = 0;
	hbbloc.dialect = 0;
	hbbloc.collatorOptions = COL_ICU;

	fLogStream = nil;
	fLogFile = nil;
	fLogIsValid = false;
	fLogErrorStack = nil;
	
	///WAK0084655, O.R. Oct 24th 2013: journal integration will fail if fCurrentLogOperation is initialized to -1
	fCurrentLogOperation = 0;

	if (FromConstructor)
	{
		fTableOfTables = new TableOfTable(this);
		fDataTableOfTables = new DataTableOfTables(this, fTableOfTables);
		fDataTableOfTables->Retain();
		fSystemTables.Add(fTableOfTables);

		fTableOfFields = new TableOfField(this);
		fDataTableOfFields = new DataTableOfFields(this, fTableOfFields);
		fDataTableOfFields->Retain();
		fSystemTables.Add(fTableOfFields);

		fTableOfIndexes = new TableOfIndexes(this);
		fDataTableOfIndexes = new DataTableOfIndexes(this, fTableOfIndexes);
		fDataTableOfIndexes->Retain();
		fSystemTables.Add(fTableOfIndexes);

		fTableOfIndexCols = new TableOfIndexCols(this);
		fDataTableOfIndexCols = new DataTableOfIndexCols(this, fTableOfIndexCols);
		fDataTableOfIndexCols->Retain();
		fSystemTables.Add(fTableOfIndexCols);

		fTableOfConstraints = new TableOfConstraints(this);
		fDataFileOfConstraints = new DataTableOfConstraints(this, fTableOfConstraints);
		fDataFileOfConstraints->Retain();
		fSystemTables.Add(fTableOfConstraints);

		fTableOfConstraintCols = new TableOfConstraintCols(this);
		fDataFileOfConstraintCols = new DataTableOfConstraintCols(this, fTableOfConstraintCols);
		fDataFileOfConstraintCols->Retain();
		fSystemTables.Add(fTableOfConstraintCols);

		fTableOfSchemas = new TableOfSchemas(this);
		fDataTableOfSchemas = new DataTableOfSchemas(this, fTableOfSchemas);
		fDataTableOfSchemas->Retain();
		fSystemTables.Add(fTableOfSchemas);

		fTableOfViews = new TableOfViews(this);
		fDataTableOfViews = new DataTableOfViews(this, fTableOfViews);
		fDataTableOfViews->Retain();
		fSystemTables.Add(fTableOfViews);

		fTableOfViewFields = new TableOfViewFields(this);
		fDataTableOfViewFields = new DataTableOfViewFields(this, fTableOfViewFields);
		fDataTableOfViewFields->Retain();
		fSystemTables.Add(fTableOfViewFields);

		fCloseSyncEvent = nil;

		fEntityCatalog = new LocalEntityModelCatalog(this);
		fAutoEntityCatalog = new LocalEntityModelCatalog(this);
		fAutoEntityCatalogBuilt = false;
#if debuglog
		fDebugLog = nil;
#endif
	}

	fIndexClusterInvalid = false;
	fMustRebuildAllIndex = false;
	fNeedRebuildDataTableHeader = false;
	fNeedToLoadOldRelations = false;
	fNeedToConvertIndexes = false;

	fEnableReferentialIntegrity = true;
	fEnableCheckUniqueness = true;
	fEnableCheckNot_Null = true;

	fStamp = 1;
	fTotalLocks = 0;
	fSchemasCatalogStamp = 1;
	fIndexesStamp = 0;
	fNoIntegrityRequestCount = 0;
}


void BaseHeader::SwapBytes()
{
	ByteSwap(&tag);
	ByteSwap(&lastoper);
	ByteSwap(&lastparm);
	ByteSwap(&nbsegdata);
	ByteSwap(&VersionNumber);
	if (IDisNotAnID)
	{
		ByteSwap(&ExtraElements_addrtabaddr);
		ByteSwap(&nbExtraElements);
		ByteSwap(&filler2);
	}
	else
	{
		((VUUIDBuffer*)&ExtraElements_addrtabaddr)->SwapBytes(); // the space is occupied by an UUID
	}
	seginfo.SwapBytes();
	//ByteSwap(&addrbblock);
	ByteSwap(&addrmultisegheader);
	ByteSwap(&lenmultisegheader);
	/*
	ByteSwap(&adrlbi);
	ByteSwap(&nbindex);
	ByteSwap(&lenindex);
	*/
	ByteSwap(&filfil);
	ByteSwap(&fStamp);
	ByteSwap(&lastaction);
	ByteSwap(&countlog);
	ByteSwap(&LastFlushRandomStamp);

	ByteSwap(&nbDataTable);
	ByteSwap(&DataTables_debuttrou);
	ByteSwap(&DataTables_addrtabaddr);

	ByteSwap(&TableDef_debuttrou);
	ByteSwap(&TableDef_addrtabaddr);
	ByteSwap(&nbTableDef);

	ByteSwap(&nbRelations);
	ByteSwap(&Relations_debuttrou);
	ByteSwap(&Relations_addrtabaddr);

	ByteSwap(&nbSeqNum);
	ByteSwap(&SeqNum_debuttrou);
	ByteSwap(&SeqNum_addrtabaddr);

	ByteSwap(&nbIndexDef);
	ByteSwap(&IndexDef_debuttrou);
	ByteSwap(&IndexDef_addrtabaddr);

	ByteSwap(&nbIndexDefInStruct);
	ByteSwap(&IndexDefInStruct_debuttrou);
	ByteSwap(&IndexDefInStruct_addrtabaddr);

	ByteSwap(&ExtraAddr);
	ByteSwap(&ExtraLen);

	ByteSwap(&countFlush);
	ByteSwap(&dialect);
}


VError Base4D::ReadBaseHeader( VFileDesc& inFile, BaseHeader *outHeader, Boolean CheckVersionNumber, Boolean DO_NOT_Map_Data_And_Struct, const VUUID* StructID, 
							  sLONG *retour, Boolean &WasSwapped, bool ConvertToHigherVersion, bool allowConvertForReadOnly, bool allowOneVersionMoreRecent)
{
	VError err = inFile.GetData( outHeader, sizeof(BaseHeader), 0);
	sLONG xretour = 0;
	WasSwapped = false;

	
	if (err == VE_OK) 
	{
		if (outHeader->tag != tagHeader4D)
		{
			if (outHeader->tag == SwapLong(tagHeader4D))
			{
				WasSwapped = true;
				outHeader->SwapBytes();
			}
		}
		if (outHeader->tag != tagHeader4D || outHeader->seginfo.sizeblockseg != ktaillebloc || outHeader->seginfo.ratio != kratio) {
			err = VE_DB4D_WRONGBASEHEADER;
		}
		else
		{
			if (outHeader->VersionNumber == XBOX_LONG8(0x0000000100000005) || outHeader->VersionNumber == 0)
			{
				DO_NOT_Map_Data_And_Struct = true;
				outHeader->VersionNumber = XBOX_LONG8(0x0000000100000006);
				if (StructID != nil)
				{
					StErrorContextInstaller errs(false); 
					StructID->ToBuffer(outHeader->GetID());
					inFile.PutData( outHeader, sizeof(BaseHeader), 0);
				}
			}

			if (outHeader->VersionNumber == XBOX_LONG8(0x0000000100000006) || outHeader->VersionNumber == XBOX_LONG8(0x0000000100000007))
			{
				outHeader->VersionNumber = XBOX_LONG8(0x0000000100000008);
				xretour = xretour | kRetourMustRebuildAllIndex;
			}

			if (outHeader->VersionNumber == XBOX_LONG8(0x0000000100000008))
			{
				xretour = xretour | kRetourMustRebuildDataTableHeader;

				outHeader->VersionNumber = XBOX_LONG8(0x0000000100000009);
				outHeader->DataTables_debuttrou = kFinDesTrou;
			}

			if (outHeader->VersionNumber == XBOX_LONG8(0x0000000100000009))
			{
				xretour = xretour | kRetourMustLoadOldRelations;
				outHeader->VersionNumber = XBOX_LONG8(0x000000010000000A);
			}

			if (outHeader->VersionNumber == XBOX_LONG8(0x000000010000000A))
			{
				xretour = xretour | kRetourMustRebuildAllIndex;
				xretour = xretour | kRetourMustConvertIndex;
				outHeader->VersionNumber = XBOX_LONG8(0x000000010000000B);
			}

			if (outHeader->VersionNumber == XBOX_LONG8(0x000000010000000B))
			{
				outHeader->VersionNumber = XBOX_LONG8(0x000000010000000C);
				xretour = xretour | kRetourMustRebuildAllIndex;
			}

			if (outHeader->VersionNumber == XBOX_LONG8(0x000000010000000C))
			{
				outHeader->VersionNumber = XBOX_LONG8(0x000000010000000D);
				outHeader->countFlush = 0;
				outHeader->dialect = 0;
				outHeader->collatorOptions = COL_ICU;
				xretour = xretour | kRetourMustSaveInfo;
			}

			uLONG8 majorVersionNumber = outHeader->VersionNumber >> 32;
			uLONG8 currentMajorVersionNumber = kVersionDB4D >> 32;
			if (majorVersionNumber <= 2)
				xretour = xretour | kRetourMustRebuildAllIndex;

			if ((outHeader->VersionNumber >> 32) == ((kVersionDB4D >> 32) + 1) && allowOneVersionMoreRecent)
			{
			}
			else if (CheckVersionNumber && (outHeader->VersionNumber != kVersionDB4D) )
			{
				/*
				if (majorVersionNumber == 3 && currentMajorVersionNumber == 4)
				{
					// will be done another way
				}
				else
				*/
				{
					if (ConvertToHigherVersion && (outHeader->VersionNumber < kVersionDB4D))
					{
						outHeader->VersionNumber = kVersionDB4D;
						if (!allowConvertForReadOnly)
							xretour = xretour | kRetourMustSaveInfo;
					}
					else
						err = VE_DB4D_WRONG_VERSION_NUMBER;
				}
			}

			// err = VE_DB4D_WRONG_VERSION_NUMBER; // for test

			if (err == VE_OK)
			{
				if (!DO_NOT_Map_Data_And_Struct && StructID != nil && !VDBMgr::GetManager()->IsRunningWakanda())
				{
					VUUID xid(outHeader->GetID());
					if (xid != *StructID)
						err = VE_DB4D_DATAFILE_DOES_NOT_MATCH_STRUCT;
				}
			}
		}
	}
	
	if (retour != nil)
		*retour = xretour;

	return err;
}




sLONG Base4D::saveobj()
{
	sLONG tot = 0;
	VError err = VE_OK;
	if (testAssert(!fIsRemote))
	{
#if debuglogwrite
		VString wherefrom(L"Base4D Saveobj");
		VString* whx = &wherefrom;
#else
		VString* whx = nil;
#endif
		
		if (fIsDataOpened) {

			/*
			if (fNeedsToSaveIndexes)
				err = SaveIndexes();
			*/
			
			if (err==0)
			{
				tot=sizeof(BaseHeader);
				Set_debug_OKToWriteIn0(true);
				hbbloc.fStamp = fStamp;
				err=writelong(&hbbloc,sizeof(BaseHeader),0,0, whx);
				Set_debug_OKToWriteIn0(false);
			}
			if (err!=0)
			{
				//SetError(err);
				tot=2;
			}
		}
		else
			tot = 2;
	}
	return(tot);
}


/*
sLONG Base4D::FindNextFileFree()
{
	sLONG result = -1;
	// pas besoin de mutex car fait par l'appelant
	for (sLONG i=1; i<= fNbTable; i++)
	{
		if (fich[i] == nil)
		{
			if (fichdata[i] == nil)
			{
				result = i;
				break;
			}
			else
			{
				if (! fichdata[i]->IsBusyDeleting())
				{
					result = i;
					break;
				}
			}
		}
	}
	
	return result;
}
*/

sLONG Base4D::CreTable( const VString& name, const CritereDISK *from, sLONG nbcrit, VError& err, CDB4DBaseContext* inContext, sLONG xnum, VUUID* xID)
{
	sLONG result = -1;
	err = VE_OK;
	TablePtr fic;
	Boolean increasesize = false;
	
	result = -1;
	if (fIsRemote)
	{
		IRequest* req = CreateRequest(inContext, Req_AddTable + kRangeReqDB4D);
		if (req == nil)
			err = ThrowError(memfull, DBaction_CreatingTable);
		else
		{
			VStream* reqsend = req->GetOutputStream();
			fID.WriteToStream(reqsend);
			name.WriteToStream(reqsend);
			const CritereDISK* p = from;
			const CritereDISK* end = from + nbcrit;
			reqsend->PutLong(nbcrit);
			for (; p != end; p++)
			{
				if (reqsend->NeedSwap())
				{
					CritereDISK qq = *p;
					qq.SwapBytes();
					reqsend->PutData(&qq, sizeof(CritereDISK));
				}
				else
					reqsend->PutData(p, sizeof(CritereDISK));
			}
			err = reqsend->GetLastError();
			if (err == VE_OK)
			{
				err = req->Send();
				if (err == VE_OK)
				{
					VStream* reqget = req->GetInputStream();
					sLONG numtable;
					err = reqget->GetLong(numtable);
					if (err == VE_OK && numtable > 0)
					{
						err = UpdateTable(numtable, reqget);
						if (err == VE_OK)
						{
							result = numtable;
						}
					}
				}
			}
			req->Release();
		}
		/*
		Table* fic = new TableRegular(this, 0, false);
		fic->SetName(name, inContext);
		fic->change(from, nbcrit, 0, false, inContext, false, nil);
		if (xID != nil)
			fic->SetUUID(*xID);
		err = AddTable(fic, true, inContext);
		if (err == VE_OK)
			result = fic->GetNum();
			*/
	}
	else
	{
		occupe();
		
		fic=new TableRegular( this, xnum, true);
		if (fic==nil)
		{
			err=ThrowError(memfull, DBaction_CreatingTable);
		}
		else
		{	
			fic->SetNotifyState(true);
			err = fic->SetName(name, inContext, false);
			if (err == VE_OK)
			{
				if (from != nil) 
					err=fic->change(from,nbcrit, 0, true, inContext);
			}
			if (err==VE_OK)
			{
				VUUID x;
				if (xID != nil)
					x = *xID;
				else
					x.Regenerate();
				fic->SetUUID(x);

				if (xnum == 0)
				{
					if (fStoreAsXML)
					{
						result = FindNextTableNum();
						if (result > fNbTable)
						{
							increasesize = true;
						}
						fic->SetNum(result);
					}
					else
					{
						err=fic->save(); // va ajouter fic dans la table des tabledefs
						if (err == VE_OK)
						{
							result = fic->getresnum();
							if (result > fNbTable)
							{
								increasesize = true;
							}
							fic->SetNum(result);
						}
					}
				}
				else
					result = xnum;

				if (err==VE_OK)
				{
					if (increasesize)
					{
						if (fich.Add(fic))
						{
							fNbTable++;
						}
						else
						{
							// il faut supprimer fic de la table des tabledefs
							if (!fStoreAsXML)
								fStructure->DeleteTableDef(fic->getresnum() - 1);
							err = ThrowError(memfull, DBaction_CreatingTable);
						}
					}
					else
					{
						fich[result] = fic;
					}

				}
			}
			else
				err = ThrowError(VE_DB4D_CANNOTCREATETABLE, DBaction_CreatingTable);
			
			
			if (err == VE_OK)
			{
				VUUID xid;
				fic->GetUUID(xid);
				AddObjectID(objInBase_Table, fic, xid);
				fic->RegisterForLang(true);
			}
			fic->SetNotifyState(false);
		}
		
		if (err != VE_OK)
		{
			result = -1;
			if (fic!=nil)
			{
				delete fic;
			}
		}
		else
		{
			if (fic!=nil)
			{
				fic->CalcDependencesField();
			}
		}
		
		Touch();

		libere();
	}
	return(result);
}

/*
sLONG Base4D::AddTable( const VString& name, const CritereDISK* from, sLONG nbcrit, CDB4DBaseContext* inContext, bool cantouch)
{
	Table *crit;
	sLONG n;
	VError  err = VE_OK;
	
	if (fIsRemote)
	{
		n = CreTable(name,from,nbcrit, err, inContext);		
	}
	else
	{
		occupe();
		
	//	ClearError();
		
		n = CreTable(name,from,nbcrit, err, inContext);
			
		if (err == VE_OK && fIsDataOpened)
		{
			xbox_assert(n>0);
			crit=fich[n];
			DataTable* df = CreateDataTable(crit, err, 0, nil, -1);
			if (df != nil)
			{
				//df->RemoveFromCache();
				df->Release();
			}

			if (err != VE_OK)
			{
				//## il faut en fait appeler le delete struct file ( a faire )
				crit->Release();
				if (n == fNbTable)
				{
					fich.DeleteNth(fNbTable-1);
					fNbTable--;
				}
			}
		}
		
		
		if (err != VE_OK)
		{ 
			n = 0;
		}
		else
		{
			if (cantouch)
				SaveDataTablesToStructTablesMatching();
		}

		if (cantouch)
			Touch();

		libere();
	}
	
	return(n);
}
*/


Table* Base4D::FindOrCreateTableRef(const VString& tablename, VError& err, const VUUID& xid)
{
	occupe();
	Table* result = FindAndRetainTableRef(tablename);
	if (result == nil)
	{
		result = new TableRegular(this);
		result->SetName(tablename, nil, false, false);
		if (!xid.IsNull())
		{
			result->SetUUID(xid);
		}
		err = AddTable(result, false, nil, !xid.IsNull(), false, true, 0, true, &xid);
	}
	QuickReleaseRefCountable(result);
	libere();
	return result;
}


VError Base4D::AddTable( Table *inTable, bool inWithNamesCheck, CDB4DBaseContext* inContext, bool inKeepUUID, bool inGenerateName, bool BuildDataTable, sLONG inPos, bool cantouch, const VUUID *inUUID)
{
	VError err = VE_OK;
	Boolean increasesize = false;
	sLONG result;
	
	if (fIsRemote)
	{
		xbox_assert(!inGenerateName);	// not supported
		
		VString tname;
		inTable->GetName(tname);
		sLONG nbcrit = inTable->GetNbCrit();
		void* block = GetFastMem(sizeof(CritereDISK)*nbcrit + 2, false, 'tmp1');
		if (block == nil)
			err = ThrowError(memfull, DBaction_CreatingTable);
		else
		{
			CritereDISK* p = (CritereDISK*)block;
			for (sLONG i = 1; i <= nbcrit; i++, p++)
			{
				Field* cri = inTable->RetainField(i);
				if (cri != nil)
				{
					*p = *(cri->getCRD());
					cri->Release();
				}
				else
				{
					p->typ = DB4D_NoType;
				}
			}
			sLONG numtable = CreTable(tname, (CritereDISK*)block, nbcrit, err, inContext);
			FreeFastMem(block);
		}
	}
	else
	{
		occupe();

		VStr31 name;
		inTable->GetName( name);

		if (inGenerateName)			// sc 26/02/2008 generate a name for the newly created table
		{
			GetNameForNewTable( name);
			inTable->SetNameSilently( name);
		}
		else if(inWithNamesCheck)	// verif name duplicate
		{
			if (FindTable( name) > 0)
				err = ThrowError( VE_DB4D_TABLENAMEDUPLICATE, DBaction_CreatingTable);	// sc 28/03/2007
		}

		if (err == VE_OK)
		{
			VUUID uuid;
			uuid.Regenerate();

			if (inTable->CanBeSaved() && !inKeepUUID)
			{
				inTable->SetUUID(uuid);
			}

			err=inTable->save(false, cantouch); // va ajouter inTable dans la table des tabledefs
			if (err == VE_OK)
			{
				if (fStoreAsXML)
				{
					if (inPos != 0)
					{
						if (inPos > fNbTable)
						{
							fich.SetCount(inPos, nil);
							fNbTable = inPos;
						}
						result = inPos;
					}
					else
					{
						result = FindNextTableNum();
						if (result > fNbTable)
						{
							increasesize = true;
						}
					}
					inTable->SetNum(result);
				}
				else
				{
					result = inTable->getresnum();
					if (result > fNbTable)
					{
						increasesize = true;
					}
					inTable->SetNum(result);
				}
				inTable->InitExtraPropHeaderLater(inTable->GetNum(), -1);
				inTable->InitExtraPropHeaderLaterForFields();
			}

			if (err==VE_OK)
			{
				if (increasesize)
				{
					if (fich.Add(inTable))
					{
						fNbTable++;
					}
					else
					{
						// il faut supprimer inTable de la table des tabledefs
						if (!fStoreAsXML)
							fStructure->DeleteTableDef(inTable->getresnum() - 1);
						err = ThrowError(memfull, DBaction_CreatingTable);
					}
				}
				else
				{
					xbox_assert( fich[result] == NULL);
					fich[result] = inTable;
				}

			}
			
			// keep this table instance
			if (err == VE_OK)
			{
				VUUID xid;
				inTable->GetUUID(xid);
				AddObjectID(objInBase_Table, inTable, xid);
				inTable->Retain();
			}
			
			if (err == VE_OK)
				inTable->RegisterForLang(true);
			
			inTable->SetNotifyState(false);
			
			if (err == VE_OK)
				inTable->ReCalc();
				//inTable->CalcDependencesField();

			// open data file
			// a-t-on le droit d'echouer ?
			if ( (err == VE_OK) && fIsDataOpened && fStructure != nil && BuildDataTable)
			{
				DataTable* df = nil;
				if (inUUID != nil)
				{
					df = FindDataTableWithTableUUID(*inUUID);
					if (df != nil)
					{
						df->SetAssociatedTable(inTable, df->GetTableDefNumInData());
					}
				}
				if (df == nil)
				{
					df = CreateDataTable(inTable, err, 0, nil, -1);
					if (df != nil)
					{
						//df->RemoveFromCache();
						df->Release();
					}
				}

				if (err != VE_OK)
				{
					//## il faut en fait appeler le delete struct file ( a faire )
					/*
					if (n == nbfich)
					{
						fich.DeleteNth(nbfich-1);
						nbfich--;
						hparam.nbfich--;
					}
					*/
				}
				else if (!VDBMgr::GetManager()->IsRunningWakanda())
					SaveDataTablesToStructTablesMatching();
			}
		}
		
		if (cantouch)
			Touch();
		libere();
	}
	
	return err;
}


VError Base4D::LoadTable( const VValueBag& inBag, VBagLoader *inLoader, CDB4DBaseContext* inContext)
{
	VError err = VE_OK;
	Table *table = nil;

	table = new TableRegular( this, 0, true);
	if (table == NULL)
	{
		err = ThrowError( VE_MEMORY_FULL, DBaction_CreatingTable);
	}
	else
	{
		sLONG numtable = 0;
		if (!inBag.GetLong(DB4DBagKeys::id, numtable))
		{
			numtable = fich.GetCount()+1;
		}
		if (numtable > fich.GetCount())
			fich.SetCount(numtable, nil);
		err = table->LoadFromBagWithLoader( inBag, inLoader, inContext);
		table->SetNum(numtable);
		fich[numtable] = table;

		VUUID xid;
		table->GetUUID(xid);
		AddObjectID(objInBase_Table, table, xid);

		// on rajoute les champs
		if (err == VE_OK)
		{
			table->InitExtraPropHeaderLater(table->GetNum(), -1);
			const VBagArray *fields = inBag.RetainElements( L"field");
			if (fields != NULL)
			{
				err = table->LoadFields( fields, inLoader, inContext);
				fields->Release();
			}
		}

		if (err == VE_OK)
		{
			err = table->LoadPrimKeyFromBagWithLoader( inBag, inLoader, inContext);
		}
	}
	fNbTable = fich.GetCount();

	return err;
}

VError Base4D::CreateTable( const VValueBag& inBag, VBagLoader *inLoader, Table **outTable, CDB4DBaseContext* inContext, Boolean inGenerateName)
{
	VError err = VE_OK;
	Table *table = nil;
	if (fIsRemote)
	{
		IRequest* req = CreateRequest(inContext, Req_AddTable_With_Bag + kRangeReqDB4D);
		if (req == nil)
			err = ThrowError( VE_MEMORY_FULL, DBaction_CreatingTable);
		else
		{
			VStream* reqsend = req->GetOutputStream();
			fID.WriteToStream(reqsend);
			inBag.WriteToStream(reqsend);
			req->PutBooleanParam( inGenerateName);

			err = reqsend->GetLastError();
			if (err == VE_OK)
			{
				err = req->Send();
				if (err == VE_OK)
				{
					VStream* reqget = req->GetInputStream();
					sLONG numtable;
					err = reqget->GetLong(numtable);
					if (err == VE_OK && numtable > 0)
					{
						err = UpdateTable(numtable, reqget);
						if (err == VE_OK)
						{
							table = RetainTable(numtable);
						}
					}
				}
			}
			req->Release();
		}
	}
	else
	{
		if (OkToUpdate(err))
		{
			table = new TableRegular( this, 0, true);
			if (table == NULL)
			{
				err = ThrowError( VE_MEMORY_FULL, DBaction_CreatingTable);
			}
			else
			{
				bool withNamesCheck = inLoader->WithNamesCheck();

				if (inGenerateName)
					inLoader->SetWithNamesCheck( false);

				err = table->LoadFromBagWithLoader( inBag, inLoader, inContext);

				if (err == VE_OK)
					err = AddTable( table, inLoader->WithNamesCheck(), inContext, true, inGenerateName);

				inLoader->SetWithNamesCheck( withNamesCheck);
					
				// on rajoute les champs
				if (err == VE_OK)
				{
					table->InitExtraPropHeaderLater(table->GetNum(), -1);
					const VBagArray *fields = inBag.RetainElements( L"field");
					if (fields != NULL)
					{
						err = table->CreateFields( fields, nil, inLoader, inContext);
						fields->Release();
					}
				}
			
				if (err == VE_OK)
				{
					NumFieldArray primaryKey;
					err = table->ExtractPrimKeyFromBagWithLoader( primaryKey, inBag, inLoader, inContext);	// sc 08/03/2010 ACI0064936
					if (err == VE_OK)
						err = table->SetPrimaryKey( primaryKey, NULL, true, inContext, NULL);	// sc 14/12/2010 ACI0069053
				}

				// en fait on pourrait la retourner si la creation de la table a reussi mais pas les champs
				if (err != VE_OK)
				{
					table->Release();
					table = NULL;
				}
			}
			Touch();
			ClearUpdating();
		}
		else
			table = nil;
	}
	
	if (outTable != NULL)
		*outTable = table;
	else if (table != NULL)
		table->Release();
	
	return err;
}


VError Base4D::GetNameForNewTable( VString& outName) const
{
	VError error = VE_OK;
	VString defaultName, oldDefaultName("Table {id}");
	VString newNameOldVers, name;
	VValueBag format;
	bool found = true;
	sLONG cpt = 0;

	VLocalizationManager *localizer = VDBMgr::GetManager()->GetDefaultLocalization();
	if (localizer != NULL)
		localizer->LocalizeStringWithKey( CVSTR("default_table_name"), defaultName);
	else
		defaultName = "Table_{id}";
	
	occupe();

	while (found)
	{
		++cpt;
		outName = defaultName;
		newNameOldVers = oldDefaultName;
		format.SetLong( CVSTR("id"), cpt);
		outName.Format( &format);
		newNameOldVers.Format( &format);
		
		found = false;
		sLONG nb = fich.GetCount();
		for( sLONG i = 1 ; (i <= nb) && !found ; ++i )
		{
			Table *table = fich[i];
			if (table != NULL)
			{
				table->GetName( name);
				found = (name == outName) || (name == newNameOldVers);
			}
		}
	}
	
	libere();

	return error;
}


sLONG Base4D::FindNextTableNum()
{
	sLONG result = fich.GetCount()+1;
	for (sLONG i = 1; i <= fich.GetCount(); i++)
	{
		if (fich[i] == nil)
		{
			result = i;
			break;
		}
	}
	return result;
}


sLONG Base4D::FindNextRelationNum()
{
	sLONG result = fRelations.GetCount()+1;
	for (sLONG i = 1; i <= fRelations.GetCount(); i++)
	{
		if (fRelations[i] == nil)
		{
			result = i;
			break;
		}
	}
	return result;
}


VError Base4D::SaveStructAsXML()
{
	VError err = VE_OK;

	if (!fWriteProtected)
	{
		if (fStoreAsXML && fStructXMLFile != nil)
		{
			err = fEntityCatalog->SaveEntityModels(*fStructXMLFile, true, true);
	#if 0
			VValueBag bag;
			VString kindstr;
			err = SaveToBagDeep(bag, kindstr, true, true, true);
			if (err == VE_OK)
			{
				err = vWriteBagToFileInXML(bag, kindstr, fStructXMLFile);
			}
	#endif
		}
		else
			err = VE_DB4D_CANNOTCREATESTRUCT;
	}

	return err;
}



uBOOL Base4D::okdel(void)
{
	return(false);
}


DataTable* Base4D::FindDataTableWithTableUUID(const VUUID& inTableID)
{
	DataTable* result = nil;
	for (DataTableVector::iterator cur = fDataTables.begin(), end = fDataTables.end(); cur != end; cur++)
	{
		DataTableRegular* df = *cur;
		if (df != nil)
		{
			VUUID xid;
			df->GetAssociatedTableUUID(xid);
			if (xid == inTableID)
			{
				result = df;
				break;
			}
		}
	}
	return result;
}


DataTable* Base4D::CreateDataTable(Table* Associate, VError& err, sLONG prefseg, DataTableDISK* dejaDFD, sLONG dejapos)
{
	VObjLock lock(this);
	err = VE_OK;
	DataTableRegular* result = nil;
	sLONG len = sizeof(DataTableDISK)+kSizeDataBaseObjectHeader;
//	xbox_assert(Associate != nil);
	DataAddr4D ou = findplace(len, nil, err, prefseg, nil);
	if (ou != -1)
	{
		sLONG pos = dejapos;
		if (pos == -1)
			pos = fTableDataTableTabAddr.findtrou(nil, err);
		if (err == VE_OK)
		{
			if (pos == -1)
				pos = hbbloc.nbDataTable;

			err = fTableDataTableTabAddr.PutxAddr(pos, ou, len, nil);

			if (err == VE_OK)
			{
				result = new DataTableRegular(this, Associate, pos+1, ou, true, dejaDFD);
				if (result == nil)
					err = ThrowError(VE_DB4D_CANNOTCREATETABLE, DBaction_CreatingTable);
				else
				{
					if (result->IsInvalid())
					{
						err = VE_DB4D_CANNOTCREATETABLE;
						result->Release();
						result = nil;
					}
					else
					{
						if (result->GetTable() != nil)
							result->Retain();
					}
				}
			}
		}

		if (err != VE_OK)
			libereplace(ou, len, nil, nil);
	}

	if (err == VE_OK && result != nil)
	{
		sLONG pos = result->GetTrueNum()-1;
		if (pos >= (sLONG)fDataTables.size())
			fDataTables.resize(pos+1, nil);
		fDataTables[pos] = result;
		result->Retain();
		result->PutInCache();
		result->setmodif(true, this, nil);
	}

	return result;
}

void Base4D::FlushBase()
{
	for( VIndex i=0 ; i < fSegments.GetCount() ; ++i)
		fSegments[i]->FlushSeg();

	if (fStructure != nil)
		fStructure->FlushBase();
}


void Base4D::setmodif(uBOOL xismodif, Base4D* bd, BaseTaskInfo* context)
{
	IObjToFlush::setmodif(xismodif, bd, context);
}


void Base4D::RequestForInvalideHeader()
{
	if (fFlushInfo != nil)
		fFlushInfo->RequestForInvalideHeader();
}


void Base4D::EndOfRequestForInvalideHeader()
{
	if (fFlushInfo != nil)
		fFlushInfo->EndOfRequestForInvalideHeader();
}


void Base4D::ValidateHeader()
{
	if ( (fSegments.GetCount() > 0) && !fWriteProtected)	// L.E. 08/12/06 added && !fWriteProtected
	{
		SegData* seg0 = fSegments[0];
		sLONG toutbon = 0;
		hbbloc.lastoper = 0;
		//seg0->writeat(&toutbon, 4, 4);
		if (fLogStream != nil && fLogIsValid)
		{
			sLONG offset = ((char*)&hbbloc.lastaction)- ((char*)&hbbloc);
			seg0->writeat(&fCurrentLogOperation, 8, offset);
			hbbloc.lastaction = fCurrentLogOperation;
		}
		hbbloc.countFlush++;

		for (VArrayOf<SegData*>::Iterator cur = fSegments.First(), end = fSegments.End(); cur != end; cur++)
		{
			SegData* seg = *cur;
			seg->writeat(&toutbon, 4, 4);
			sLONG offset = ((char*)&hbbloc.countFlush)- ((char*)&hbbloc);
			seg->writeat(&hbbloc.countFlush, 4, offset);
		}

		if (fIndexSegment != nil)
		{
			fIndexSegment->writeat(&toutbon, 4, 4);
			sLONG offset = ((char*)&hbbloc.countFlush)- ((char*)&hbbloc);
			fIndexSegment->writeat(&hbbloc.countFlush, 4, offset);
		}

		seg0->FlushSeg();
	}
}


void Base4D::InvalidateHeader()
{
	if ( (fSegments.GetCount() > 0) && !fWriteProtected)
	{
		SegData* seg0 = fSegments[0];
		sLONG toutmauvais = -1;
		hbbloc.lastoper = -1;
		hbbloc.LastFlushRandomStamp = VSystem::Random(false) * (sLONG)VSystem::GetCurrentTime();
		sLONG randstamp = hbbloc.LastFlushRandomStamp;
		sLONG offsetstamp = ((char*)&hbbloc.LastFlushRandomStamp)- ((char*)&hbbloc);
		//seg0->writeat(&toutmauvais, 4, 4);

		for (VArrayOf<SegData*>::Iterator cur = fSegments.First(), end = fSegments.End(); cur != end; cur++)
		{
			SegData* seg = *cur;
			seg->writeat(&toutmauvais, 4, 4);
			seg->writeat(&randstamp, 4, offsetstamp);
		}

		if (fIndexSegment != nil)
		{
			fIndexSegment->writeat(&toutmauvais, 4, 4);
			fIndexSegment->writeat(&randstamp, 4, offsetstamp);
		}

		seg0->FlushSeg();

	}
}


VError Base4D::CompletelyChangeUUID(const VUUIDBuffer& id)
{
	VError err = VE_OK;
	if (fStructure != nil)
		err = fStructure->CompletelyChangeUUID(id);
	if (err == VE_OK)
	{
		if (OkToUpdate(err))
		{
			if (fIndexSegment != nil)
			{
				BaseHeader bb;
				fIndexSegment->readat(&bb, sizeof(bb), 0);
				bb.GetID() = id;
				fIndexSegment->writeat(&bb, sizeof(bb), 0);
			}

			hbbloc.GetID() = id;
			setmodif(true, this, nil);
			ClearUpdating();
		}
	}
	return err;
}

#if 0
 // old code that will be deleted

class logEntryBaseRef
{
	public:
		logEntryBaseRef()
		{
			curTransLevel = 1;

		}

		sLONG curTransLevel;
		logEntryRefVector logEntries;
};

typedef pair<sLONG, sLONG> pairlong;


bool Base4D::ParseAndPurgeLogEntries(logEntryRefVector& logEntries, logEntryRefVector::iterator startFrom, bool& transWasCanceled)
{
	map<pairlong, logEntryRefVector::iterator> creations;
	transWasCanceled = false;
	bool returncont = true;
	logEntryRefVector::iterator cur = startFrom, end = logEntries.end();
	bool cont = true;
	while (cont)
	{
		if (cur == end)
		{
			returncont = false;
			cont = false;
		}
		else
		{
			logEntryRef* entry = &(*cur);
			if (entry->logAction == DB4D_Log_StartTrans)
			{
				bool subtransWasCanceled = false;
				cont = ParseAndPurgeLogEntries(logEntries, cur+1, subtransWasCanceled);
				if (subtransWasCanceled)
					entry->logAction = DB4D_Log_None;
				else
					entry->logAction = (DB4D_LogAction)0;
			}
			else if (entry->logAction == DB4D_Log_Commit)
			{
				cont = false;
				entry->logAction = (DB4D_LogAction)0;
			}
			else if (entry->logAction == DB4D_Log_RollBack)
			{
				cont = false;
				transWasCanceled = true;
				for (logEntryRefVector::iterator innercur = startFrom; innercur != cur; innercur++)
				{
					innercur->logAction = DB4D_Log_None;
				}
				entry->logAction = DB4D_Log_None;
			}
			else if (entry->logAction == DB4D_Log_CreateRecord)
			{
				creations[make_pair(entry->tablenum, entry->recnum)] = cur;
			}
			else if (entry->logAction == DB4D_Log_DeleteRecord)
			{
				map<pairlong, logEntryRefVector::iterator>::iterator found = creations.find(make_pair(entry->tablenum, entry->recnum));
				if (found != creations.end())
				{
					found->second->logAction = DB4D_Log_None;
					creations.erase(found);
					entry->logAction = DB4D_Log_None;
				}
			}

			cur++;
		}
	}
	return returncont;
}

#endif


VErrorDB4D Base4D::EndBackup(BaseTaskInfo* context, VFile* oldJournal)
{
	VError err = VE_OK;

#if 0
// old code that will be deleted

	if (oldJournal != nil && fLogStream != nil && fLogIsValid) 
	{
		VFileStream oldLog(oldJournal); 
		err = oldLog.OpenReading();
		if (err == VE_OK)
		{
			oldLog.SetBufferSize(512000);
			ContextCollection& contexts = BaseTaskInfo::GetAndLockListContexts();
			for (ContextCollection::const_iterator cur = contexts.begin(), end = contexts.end(); cur != end && err == VE_OK; cur++)
			{
				BaseTaskInfo* curcontext = *cur;
				if (curcontext->GetBase() == this)
				{
					Transaction* trans = curcontext->GetCurrentTransaction();
					if (trans != nil)
						trans = trans->GetBaseTrans();
					if (trans != nil)
					{
						LogEntriesCollection logentries;
						err = trans->StealLogEntries(logentries);
						if ( err == VE_OK )
						{
							trans->DoNotKeepEntries();
							trans->GetOwner()->ClearIsCreationWrittenToLog();
							err = trans->GetOwner()->WriteLog(DB4D_Log_StartTrans, true);
							trans->KeepEntries();
						}

						logEntryBaseRef leb;
						leb.logEntries.resize(logentries.size());
						logEntryRefVector::iterator curRef = leb.logEntries.begin();

						for (LogEntriesCollection::iterator curf = logentries.begin(), endf = logentries.end(); curf != endf && err == VE_OK; curf++, curRef++)
						{
							sLONG8 pos = *curf;
							err = oldLog.SetPos(pos);
							if (err == VE_OK)
							{
								uLONG operationTag;
								err = oldLog.GetLong(operationTag);
								if (err == VE_OK)
								{
									if ( operationTag == kTagLogDB4D )
									{
										sLONG8 contextID;
										DB4D_LogAction logAction;
										sLONG len;
										sLONG8 curpos;
										sLONG8 globaloperation;
										err = oldLog.GetLong8(globaloperation);

										if ( err == VE_OK )
											err = oldLog.GetLong(len);

										if ( err == VE_OK )
											err = oldLog.GetLong((uLONG&)logAction);

										if ( err == VE_OK )
											err = oldLog.GetLong8(contextID);

										uLONG8 timeStamp;
										if (err == VE_OK)
											err = oldLog.GetLong8(timeStamp);

										sLONG recnum = -1;
										sLONG tablenum = 0;

										if (logAction == DB4D_Log_CreateRecord)
										{
											RecordHeader tag;
											err = tag.ReadFromStream(&oldLog);
											recnum = tag.GetPos();
											VUUID tID;
											if (err == VE_OK)
											{
												err = tID.ReadFromStream(&oldLog);
												Table* tt = FindAndRetainTableRef(tID);
												if (tt != nil)
												{
													tablenum = tt->GetNum();
													tt->Release();
												}
											}

										}
										else if (logAction == DB4D_Log_DeleteRecord)
										{
											VUUID tID;
											if ( err == VE_OK )
												err = oldLog.GetLong(recnum);
											if (err == VE_OK)
											{
												err = tID.ReadFromStream(&oldLog);
												Table* tt = FindAndRetainTableRef(tID);
												if (tt != nil)
												{
													tablenum = tt->GetNum();
													tt->Release();
												}
											}

										}

										curRef->logAction = logAction;
										curRef->tablenum = tablenum;
										curRef->recnum = recnum;
									}
									else
										err = VE_UNIMPLEMENTED;
								}
							}
						}

						if (err == VE_OK)
						{
							bool transWasCanceled = false;
							ParseAndPurgeLogEntries(leb.logEntries, leb.logEntries.begin(), transWasCanceled);
						}
						
						curRef = leb.logEntries.begin();

						for (LogEntriesCollection::iterator curf = logentries.begin(), endf = logentries.end(); curf != endf && err == VE_OK; curf++, curRef++)
						{
							if (curRef->logAction != DB4D_Log_None)
							{
								sLONG8 pos = *curf;
								err = oldLog.SetPos(pos);
								if (err == VE_OK)
								{
									uLONG operationTag;
									err = oldLog.GetLong(operationTag);
									if (err == VE_OK)
									{
										if ( operationTag == kTagLogDB4D )
										{
											sLONG8 contextID;
											DB4D_LogAction logAction;
											sLONG len;
											sLONG8 curpos;
											sLONG8 globaloperation;
											err = oldLog.GetLong8(globaloperation);

											if ( err == VE_OK )
												err = oldLog.GetLong(len);

											if ( err == VE_OK )
												err = oldLog.GetLong((uLONG&)logAction);

											if ( err == VE_OK )
												err = oldLog.GetLong8(contextID);

											xbox_assert(contextID < 0);
											contextID = -contextID;
											xbox_assert(contextID == curcontext->GetID());

											uLONG8 timeStamp;
											if (err == VE_OK)
												err = oldLog.GetLong8(timeStamp);

											if (len >= kAddToLogLen)
											{
												void* p = GetFastMem(len-kAddToLogLen+1, false, 'tlog');
												if (p == nil)
													err = memfull;
												else
												{
													if (len > kAddToLogLen)
														err = oldLog.GetData(p, len-kAddToLogLen);
													if (err == VE_OK)
													{
														sLONG lenEnd;
														err = oldLog.GetLong(lenEnd);
														if (len != lenEnd)
															err = VE_UNIMPLEMENTED;
														
														uLONG operationTagEnd;
														if (err == VE_OK)
														{
															err = oldLog.GetLong((uLONG&)operationTagEnd);
															if (operationTagEnd != kTagLogDB4DEnd)
																err = VE_UNIMPLEMENTED;
														}

														if (err == VE_OK)
														{
															fLogMutex.Lock();
															fCurrentLogOperation++;
															err = curcontext->AddLogEntryToTrans(fLogStream->GetPos());
															if (err == VE_OK)
																err = fLogStream->PutLong(kTagLogDB4D);
															if (err == VE_OK)
																err = fLogStream->PutLong8(fCurrentLogOperation);
															if (err == VE_OK)
																err = fLogStream->PutLong(len);
															if (err == VE_OK)
																err = fLogStream->PutLong((uLONG&)logAction);
															if (err == VE_OK)
																err = fLogStream->PutLong8((sLONG8)-contextID);
															if (err == VE_OK)
																err = fLogStream->PutLong8(timeStamp);
															if (err == VE_OK && len > kAddToLogLen)
																err = fLogStream->PutData(p, len - kAddToLogLen);
															if (err == VE_OK)
																err = fLogStream->PutLong(lenEnd);
															if (err == VE_OK)
																err = fLogStream->PutLong(kTagLogDB4DEnd);

															fLogMutex.Unlock();
														}
													}

													FreeFastMem(p);
												}
											}
											else
												err = VE_UNIMPLEMENTED;

										}
										else
											err = VE_UNIMPLEMENTED;
									}
								}
							}
						}
					}
				}
			}

			BaseTaskInfo::UnlockListContexts();

			oldLog.CloseReading();
		}
	}
#endif

	return err;
}


void Base4D::ClearAllCachedRelPath()
{
	VObjLock lock(this);
	fCachedRelPath.clear();
}


void Base4D::AddCachedRelPath(sLONG source, sLONG dest, const JoinPath& inPath)
{
	VObjLock lock(this);
	
	try
	{
		fCachedRelPath[make_pair(source, dest)].CopyFrom(inPath);
	}
	catch (...)
	{
	}
}


Boolean Base4D::GetCachedRelPath(sLONG source, sLONG dest, JoinPath& outPath) const
{
	VObjLock lock(this);
	Boolean res = false;

	CachedRelationPathCollection::const_iterator found = fCachedRelPath.find(make_pair(source, dest));
	if (found != fCachedRelPath.end())
	{
		outPath.CopyFrom(found->second);
		res = true;
	}
	
	return res;
}


void Base4D::DelayForFlush()
{
	BaseFlushInfo *info = GetDBBaseFlushInfo();
	if (info != nil)
	{
		info->DelayForFlush();
		/*
		uLONG delay = info->GetDelayToInsert();
		if (delay != 0)
		{
			VTaskMgr::Get()->GetCurrentTask()->Sleep(delay);
		}
		*/
	}
}


VError Base4D::SetIntlMgr(VIntlMgr* inIntlMgr, VDB4DProgressIndicator* InProgress)
{
	VError err = VE_OK;
	if (inIntlMgr != nil)
	{
		bool stringAlgoHasChanged = false;
		bool keywordAlgoHasChanged = false;
		if (fStructure != nil)
		{
			if (StoredAsXML())
			{
				CopyRefCountable(&(fStructure->fIntlMgr), inIntlMgr);
			}
			else
			{
				err = fStructure->SetIntlMgr(inIntlMgr, InProgress);
			}
		}
		if (err == VE_OK)
		{
			occupe();
			VString BaseStringCompHash, BaseKeywordBuildingHash;
			VString IntlStringCompHash, IntlKeywordBuildingHash;

			GetIntlInfo(BaseStringCompHash, BaseKeywordBuildingHash);
			inIntlMgr->GetStringComparisonAlgorithmSignature(IntlStringCompHash);
			inIntlMgr->GetKewordAlgorithmSignature(IntlKeywordBuildingHash);

			if (fIntlMgr == nil)
			{
				stringAlgoHasChanged = true;
				keywordAlgoHasChanged = true;
			}
			else if (inIntlMgr->GetDialectCode() != fIntlMgr->GetDialectCode() || inIntlMgr->GetCollator()->GetOptions() != fIntlMgr->GetCollator()->GetOptions())
			{
				stringAlgoHasChanged = true;
				keywordAlgoHasChanged = true;
			}
			else if (BaseStringCompHash != IntlStringCompHash)
			{
				stringAlgoHasChanged = true;
				keywordAlgoHasChanged = true;
			}
			else if (BaseKeywordBuildingHash != IntlKeywordBuildingHash)
			{
				keywordAlgoHasChanged = true;
			}

			if (stringAlgoHasChanged || keywordAlgoHasChanged)
			{
				CopyRefCountable(&fIntlMgr, inIntlMgr);
				hbbloc.dialect = inIntlMgr->GetDialectCode();
				hbbloc.collatorOptions = inIntlMgr->GetCollator()->GetOptions();
				if (!fIsRemote)
					SetIntlInfo(IntlStringCompHash, IntlKeywordBuildingHash);
				if (!fIsRemote)
					setmodif(true, this, nil);
			}
			libere();

			if (!fIsRemote && (stringAlgoHasChanged || keywordAlgoHasChanged))
			{
				LockIndexes();
				for (IndexMap::iterator cur = fAllIndexes.begin(), end = fAllIndexes.end(); cur != end; cur++)
				{
					IndexInfo* ind = cur->second;
					if (ind->NeedToRebuildIndex(inIntlMgr, stringAlgoHasChanged, keywordAlgoHasChanged))
					{
						RebuildIndexByRef(ind, nil, InProgress);
					}
				}

				UnLockIndexes();
			}
		}
	}
	return err;
}


VError Base4D::LoadIndexesDef()
{
	fIndexesTabAddr.Init(this,this,&hbbloc.IndexDef_addrtabaddr,&hbbloc.IndexDef_debuttrou,&hbbloc.nbIndexDef, 
													nil, nil, 0);

	fIndexesInMem.Init(hbbloc.nbIndexDef, false);
	fIndexesInMem.SetContientQuoi(t_indexdef);

	return VE_OK;
}


IndexInfo* Base4D::LoadAndInitIndex(StructElemDef* e, VError& err)
{
	StErrorContextInstaller errorContext;

	err = VE_OK;
	IndexInfo* ind = nil;
	sLONG IndexInfoTyp;
	if (e != nil)
	{
		VConstPtrStream buf(e->GetDataPtr(), e->GetDataLen());
		err = buf.OpenReading();
		if (err == VE_OK)
		{
			buf.SetNeedSwap(e->NeedSwap());
			err = buf.GetLong(IndexInfoTyp);
			if (err != VE_OK)
				err = ThrowError(VE_DB4D_CANNOTREADINDEXTABLE, DBaction_ReadingIndexTable);
			else
			{
				if (IndexInfoTyp == DB4D_Index_None)
				{
					ind=nil;
				}
				else
				{
					ind = CreateIndexInfo(IndexInfoTyp);
					if (ind == nil) 
						err=ThrowError(memfull, DBaction_ReadingIndexTable);
					else
					{
						err=ind->GetFrom(buf,this);
						if (err !=VE_OK)
							err=ThrowError(VE_DB4D_CANNOTREADINDEXTABLE, DBaction_ReadingIndexTable);
						else
						{
							ind->SetPlace(e->GetNum());
						}
					}
				}
			}
		}
		buf.CloseReading();
	}

	if (err != VE_OK)
	{
		if (ind != nil)
			ind->Release();
		ind = nil;
	}
	else
	{
		if (ind != nil)
		{
			OccupableStack* curstack = gOccPool.GetStackForCurrentThread();

			err = ind->InitTablesAddr(curstack);
			try
			{
				//ind->AddToCache(ObjCache::HeaderAccess);
				ind->PutInCache();
				fAllIndexes.insert(std::make_pair(ind->GetID(), ind));
				if (!fStoreAsXML)
					TouchIndexes();
				ind->SetStructureStamp( GetIndexesStamp());
				ind->SetOKtoSave();
				ind->Retain();
			}
			catch (...)
			{
				ind->Release();
				err = ThrowError(memfull, DBaction_ReadingIndexTable);
			}
		}
	}
	errorContext.Flush();

	return ind;
}


bool Base4D::IsDuplicateIndex(IndexInfo* ind)
{
	bool result = false;
	Field* cri = ind->GetTargetField();
	Table* table = ind->GetTargetTable();

	if (cri != nil)
	{
		for (IndexMap::iterator cur = fAllIndexes.begin(), end = fAllIndexes.end(); cur != end; cur++)
		{
			IndexInfo* xind = cur->second;
			if (xind->GetTargetField() == cri && xind->GetTargetTable() == table && xind->MatchType(ind))
			{
				result = true;
				break;
			}
		}
	}

	return result;
}


void Base4D::GetIndices(VJSArray& outIndices)
{
	LockIndexes();
	vector<IndexInfo*> indices;
	indices.reserve(fAllIndexes.size());
	for (IndexMap::iterator cur = fAllIndexes.begin(), end = fAllIndexes.end(); cur != end; cur++)
	{
		IndexInfo* ind = cur->second;
		ind->Retain();
		indices.push_back(ind);
	}
	UnLockIndexes();

	for (vector<IndexInfo*>::iterator cur = indices.begin(), end = indices.end(); cur != end; cur++)
	{
		VJSObject objrec(outIndices.GetContext());
		objrec.MakeEmpty();
		IndexInfo* ind = *cur;

		VString stype;
		if (ind->GetTyp() == DB4D_Index_OnKeyWords)
			stype = L"keywords";
		else
		{
			if (ind->GetHeaderType() == DB4D_Index_BtreeWithCluster)
				stype = L"cluster";
			else
				stype = L"btree";
		}
		objrec.SetProperty("type", stype, JS4D::PropertyAttributeNone);

		if (ind->GetTyp() == DB4D_Index_OnMultipleFields)
		{
			VJSArray arr(outIndices.GetContext());
			IndexInfoFromMultipleField* indm = (IndexInfoFromMultipleField*)ind;
			FieldRefArray* frefs = indm->GetFields();

			Table* tt = indm->GetTargetTable();
			if (tt != nil)
			{
				VString ss;
				tt->GetName(ss);
				objrec.SetProperty("table", ss, JS4D::PropertyAttributeNone);
			}

			for (FieldRefArray::Iterator curx = frefs->First(), endx = frefs->End(); curx != endx; curx++)
			{
				Field* cri = curx->crit;
				if (cri != nil)
				{
					VString ss;
					cri->GetName(ss);
					VJSValue jsval(outIndices.GetContext());
					jsval.SetString(ss);
					arr.PushValue(jsval);
				}

			}

			objrec.SetProperty("fields", arr, JS4D::PropertyAttributeNone);
		}
		else
		{
			IndexInfoFromField* indx = (IndexInfoFromField*)ind;
			Field* cri = indx->GetField();
			Table* tt = indx->GetTable();
			if (tt != nil)
			{
				VString ss;
				tt->GetName(ss);
				objrec.SetProperty("table", ss, JS4D::PropertyAttributeNone);
			}
			if (cri != nil)
			{
				VString ss;
				cri->GetName(ss);
				objrec.SetProperty("field", ss, JS4D::PropertyAttributeNone);
			}
		}
		outIndices.PushValue(objrec);
	}
	
}


VError Base4D::LoadIndexes(Boolean DO_NOT_map_IndexesInStruct)
{
	VError err = VE_OK;
	sLONG i;
	VIntlMgr* intl = fIntlMgr;
	xbox_assert(fIntlMgr != nil);
	if (intl == nil)
		intl = VIntlMgr::GetDefaultMgr();
	xbox_assert(intl != nil);

	VString BaseStringCompHash, BaseKeywordBuildingHash;
	VString IntlStringCompHash, IntlKeywordBuildingHash;

	GetIntlInfo(BaseStringCompHash, BaseKeywordBuildingHash);
	intl->GetStringComparisonAlgorithmSignature(IntlStringCompHash);
	intl->GetKewordAlgorithmSignature(IntlKeywordBuildingHash);

	bool rebuildTextIndex = false;
	bool rebuildKeyWordIndex = false;
	if (BaseStringCompHash != IntlStringCompHash)
	{
		rebuildTextIndex = true;
		rebuildKeyWordIndex = true;
	}
	else
	{
		if (BaseKeywordBuildingHash != IntlKeywordBuildingHash)
		{
			rebuildKeyWordIndex = true;
		}
	}

	AllowWakeUpAfterIndexAction = false;

	if (fNeedToConvertIndexes && fStructure != nil)
	{
		for (i = 0; i<fStructure->hbbloc.nbIndexDefInStruct; i++)
		{
			DeleteIndexDefInStruct(i);
		}

	}
	else
	{
		for (i = 0; i<hbbloc.nbIndexDef; i++)
		{
			StErrorContextInstaller errorContext;
			
			err = VE_OK;
			IndexInfo* ind = nil;
			sLONG IndexInfoTyp;
			StructElemDef* e = LoadIndexDef(i, err);
			if (e != nil)
			{
				VConstPtrStream buf(e->GetDataPtr(), e->GetDataLen());
				err = buf.OpenReading();
				if (err == VE_OK)
				{
					buf.SetNeedSwap(e->NeedSwap());
					err = buf.GetLong(IndexInfoTyp);
					if (err != VE_OK)
						err = ThrowError(VE_DB4D_CANNOTREADINDEXTABLE, DBaction_ReadingIndexTable);
					else
					{
						if (IndexInfoTyp == DB4D_Index_None)
						{
							ind=nil;
						}
						else
						{
							ind = CreateIndexInfo(IndexInfoTyp);
							if (ind == nil) 
								err=ThrowError(memfull, DBaction_ReadingIndexTable);
							else
							{
								err=ind->GetFrom(buf,this);
								if (err !=VE_OK)
									err=ThrowError(VE_DB4D_CANNOTREADINDEXTABLE, DBaction_ReadingIndexTable);
								else
								{
									ind->SetPlace(e->GetNum());
								}
							}
						}
					}
				}
				buf.CloseReading();
				e->libere();
			}

			if (err != VE_OK)
			{
				if (ind != nil)
					ind->Release();
				ind = nil;
			}
			else
			{
				if (ind != nil)
				{
					if (ind->ValidateIndexInfo() == VE_OK)
					{
						try
						{
							ind->PutInCache();
							//ind->AddToCache(ObjCache::HeaderAccess);
							xbox_assert(!IsDuplicateIndex(ind));						
							fAllIndexes.insert(std::make_pair(ind->GetID(), ind));
							if (!fStoreAsXML)
								TouchIndexes();
							ind->SetStructureStamp( GetIndexesStamp());
							ind->SetOKtoSave();
						}
						catch (...)
						{
							ind->Release();
							err = ThrowError(memfull, DBaction_ReadingIndexTable);
							break;
						}
					}
					else
						ind->Release();
				}
			}
			errorContext.Flush();
			err = VE_OK;
		}

		if (fStructure != nil)
		{
			if (fStoreAsXML)
			{
				i = 1;
				for (IndexRefCollection::iterator cur = fXMLIndexDef.begin(), end = fXMLIndexDef.end(); cur != end; cur++, i++)
				{
					IndexInfo* ind = *cur;
					IndexInfo* ind2 = FindAndRetainIndex(ind, true);
					if (ind2 != nil)
					{
						// on a trouve l'index en data qui est le meme que l'index en struct
						// rien a faire dans ce cas
						ind2->SetPlaceInStruct(i);
						ind2->Release();
					}
					else
					{
						if ( ind->GetTargetTable() != nil )
						{
							if (ind->GetTargetTable()->GetDF() != nil)
							{
								// sinon il faut creer l'index manquant
								if (ind->CanBeScalarConverted())
								{
									ind2 = ind->ConvertToScalar();
									ind = ind2;
								}
								else
									ind->Retain();

								ind->CreateHeader();
								if (!DO_NOT_map_IndexesInStruct && !fWriteProtected)
								{
									SubCreIndex(ind, nil, nil, nil, nil, true, nil);
								}
								else
								{
									ind->PutInCache();
									//ind->AddToCache(ObjCache::HeaderAccess);
									xbox_assert(!IsDuplicateIndex(ind));						
									fAllIndexes.insert(make_pair(ind->GetID(), ind));
									TouchIndexes();
									ind->SetStructureStamp( GetIndexesStamp());
								}
							}
							else
							{
								ind = ind; // break here
							}
						}
						else
						{
							ind = ind; // break here
						}
					}
				}

			}
			else
			{
				sLONG nbindexinstruct = fStructure->hbbloc.nbIndexDefInStruct;

				for (i = 0; i < nbindexinstruct; i++)
				{
					err = VE_OK;
					IndexInfo* ind = nil;
					sLONG IndexInfoTyp;
					StructElemDef* e = LoadIndexDefInStruct(i, err);
					if (e != nil)
					{
						VConstPtrStream buf(e->GetDataPtr(), e->GetDataLen());
						err = buf.OpenReading();
						if (err == VE_OK)
						{
							buf.SetNeedSwap(e->NeedSwap());
							err = buf.GetLong(IndexInfoTyp);
							if (err != VE_OK)
								err = ThrowError(VE_DB4D_CANNOTREADINDEXTABLE, DBaction_ReadingIndexTable);
							else
							{
								if (IndexInfoTyp == DB4D_Index_None)
								{
									ind=nil;
								}
								else
								{
									ind = CreateIndexInfo(IndexInfoTyp);
									if (ind == nil) 
										err=ThrowError(memfull, DBaction_ReadingIndexTable);
									else
									{
										err=ind->GetFrom(buf,this, false);
										if (err == VE_OK)
										{
											ind->SetPlaceInStruct(i);
											IndexInfo *ind2 = nil;
											IndexMap::iterator found = fAllIndexes.find(ind->GetID());
											if (found == fAllIndexes.end())
											{
												ind2 = FindAndRetainIndex(ind, true);
											}
											else
											{
												ind2 = found->second;
												ind2->Retain();
											}
											if (ind2 != nil)
											{
												// on a trouve l'index en data qui est le meme que l'index en struct
												// rien a faire dans ce cas
												ind2->SetPlaceInStruct(i);
												ind2->Release();
												ind->Release();
											}
											else
											{
												if ( ind-> GetTargetTable ( ) ) // Sergiy - 8 Oct 2007 - ACI0054197 - If a table has been deleted, then no index loading because it leads to a crash later.
												{
													if (ind->CanBeScalarConverted())
													{
														ind2 = ind->ConvertToScalar();
														ind->Release();
														ind = ind2;
													}

													// sinon il faut creer l'index manquant
													ind->CreateHeader();
													if (!DO_NOT_map_IndexesInStruct && !fWriteProtected)
													{
														SubCreIndex(ind, nil, nil, nil, nil, true, nil);
													}
													else
													{
														ind->PutInCache();
														//ind->AddToCache(ObjCache::HeaderAccess);
														xbox_assert(!IsDuplicateIndex(ind));						
														fAllIndexes.insert(make_pair(ind->GetID(), ind));
														TouchIndexes();
														ind->SetStructureStamp( GetIndexesStamp());
													}
												}
												else
												{
													ind = ind;
												}
											}
										}
									}
								}
							}
						}
						e->libere();
					}
				}
			}

			vector<IndexInfo*> indexesToDelete;
			LockIndexes();
			transform(fAllIndexes.begin(), fAllIndexes.end(), back_inserter(indexesToDelete), select2nd<pair<VUUID, IndexInfo*> >() );
			UnLockIndexes();

			for (vector<IndexInfo*>::iterator cur = indexesToDelete.begin(), end = indexesToDelete.end(); cur != end; cur++)
			{
				IndexInfo* inddel = *cur;
				if (inddel == nil)
					break; 

				if (inddel->GetPlaceInStruct() == -1)
				{
					AddDeleteIndex(this,inddel->GetID(),nil, nil);
				}
				else
				{
					bool isdup = false;
					for (vector<IndexInfo*>::iterator cur2 = cur+1; cur2 != end; cur2++)
					{
						IndexInfo* inddel2 = *cur2;
						if (inddel2 != nil)
						{
							sLONG xindtyp;
							if (inddel2->GetHeader() == nil) 
								xindtyp = 0;
							else
								xindtyp = inddel2->GetHeader()->GetTyp();
							if (inddel->MatchType(inddel2))
							{
								Boolean oktyp = true;
								if  ( (inddel->GetHeader()->GetTyp() == xindtyp)
									|| (inddel->IsAuto() && (xindtyp == DB4D_Index_BtreeWithCluster || xindtyp == DB4D_Index_Btree) )
									|| (inddel->GetHeader()->GetTyp() == DB4D_Index_BtreeWithCluster && (xindtyp == DB4D_Index_Btree || inddel2->IsAuto()))
									|| (inddel->GetHeader()->GetTyp() == DB4D_Index_Btree && (xindtyp == DB4D_Index_BtreeWithCluster || inddel2->IsAuto())) )
								{
									oktyp = true;
								}
								else
									oktyp = false;

								if (oktyp || (inddel2->IsAuto() && inddel->IsAuto()) )
								{
									if (inddel->Egal(inddel2))
									{
										if (inddel2->IsAuto())
										{
											AddDeleteIndex(this,inddel2->GetID(),nil, nil);
											*cur2 = nil;
										}
										else
											isdup = true;
									}
								}
							}
						}
					}
					if (isdup)
					{
						AddDeleteIndex(this,inddel->GetID(),nil, nil);
					}
					else if (inddel->MustBeRebuild())
					{
						if (inddel->MatchType(DB4D_Index_OnOneField))
						{
							sLONG headerType = inddel->GetHeaderType();
							VString name = inddel->GetName();
							Boolean uniqkeys = inddel->IsUniqueKey();
							Field* crit = ((IndexInfoFromField*)inddel)->GetField();
							DeleteIndexByRef(inddel, nil, nil);
							CreIndexOnField(crit, headerType, uniqkeys, nil, nil, &name);
						}
						else
							AddRebuildIndex(inddel, nil, nil);
					}
					else
					{
						if (fMustRebuildAllIndex)
						{
							inddel->SetInvalidOnDisk();
							AddRebuildIndex(inddel, nil, nil);
						}
						else
						{
							if (inddel->NeedToRebuildIndex(intl, rebuildTextIndex, rebuildKeyWordIndex))
							{
								inddel->SetInvalidOnDisk();
								AddRebuildIndex(inddel, nil, nil);
							}
							else
							{
								if (fIndexClusterInvalid && inddel->GetHeader() != nil)
								{
									if (inddel->GetHeader()->GetRealType() == DB4D_Index_BtreeWithCluster)
									{
										AddRebuildIndex(inddel, nil, nil);
									}
								}
							}
						}
					}
				}
			}

		}

	}

	LockIndexes();
	for (IndexMap::iterator cur = fAllIndexes.begin(), end = fAllIndexes.end(); cur != end; cur++)
	{
		IndexInfo* ind = cur->second;
		if (ind != nil)
		{
			if (ind->IsInvalidOnDisk())
			{
				ind->SetInvalid();
				AddRebuildIndex(ind, nil, nil);
			}
		}
			
	}

	UnLockIndexes();


	AllowWakeUpAfterIndexAction = true;
	gIndexBuilder->WakeUp();

	if (fNeedToConvertIndexes)
	{
		fNeedToConvertIndexes = false;
		setmodif(true, this, nil);
	}

	if (fIndexClusterInvalid)
	{
		fIndexClusterInvalid = false;
		setmodif(true, this, nil);
	}

	if (fMustRebuildAllIndex)
	{
		fMustRebuildAllIndex = false;
		setmodif(true, this, nil);
	}

	if (err == VE_OK) 
		RecalcDependencesIndex(false);

	return err;
}


void Base4D::RecalcDependencesIndex(Boolean ClearFirst)
{
	sLONG i,nb,nb2,j;
	Table *fic;
	IndexInfo *ind;
	
	if (ClearFirst)
	{
		occupe();
		
		nb=fich.GetCount();
		for (i=1;i<=nb;i++)
		{
			fic=fich[i];
			if (fic != nil)
			{
				nb2=fic->GetNbCrit();
				fic->ClearIndexDep();
				for (j=1;j<=nb2;j++)
				{
					Field *fld = fic->RetainField(j);
					if (fld != nil)
					{
						fld->ClearIndexDep();
						fld->Release();
					}
				}
			}
		}
		
		libere();
	}

	LockIndexes();
	IndexMap::iterator cur = fAllIndexes.begin(), end = fAllIndexes.end();
	for (; cur != end; cur++)
	{
		ind = cur->second;
		if (ind!=nil) 
			ind->CalculDependence();
	}

	UnLockIndexes();
	
	
}


void Base4D::RecalcDependencesField(void)
{
	sLONG i,nb;
	Table *fic;
	
	occupe();
	
	nb=fich.GetCount();
	for (i=1;i<=nb;i++)
	{
		fic=fich[i];
		if (fic != nil)
			fic->CalcDependencesField();
	}
	
	libere();
}


Boolean Base4D::DeleteTable(Table* inFileToDelete, CDB4DBaseContext* inContext, VDB4DProgressIndicator* InProgress, VSyncEvent* event, Boolean onlylocal)
{
	Boolean ok = false;
	VError err = VE_OK;

	if (fIsRemote && !onlylocal)
	{
		if (inFileToDelete->GetOwner() == this)
		{
			IRequest* req = CreateRequest(inContext, Req_DropTable + kRangeReqDB4D);
			if (req == nil)
				err = ThrowError(memfull, DBaction_DeletingTable);
			else
			{
				VStream* reqsend = req->GetOutputStream();
				fID.WriteToStream(reqsend);
				reqsend->PutLong(inFileToDelete->GetNum());
				req->PutProgressParam(InProgress);
				err = reqsend->GetLastError();
				if (err == VE_OK)
				{
					err = req->Send();
					if (err == VE_OK)
					{
						sBYTE result = req->GetInputStream()->GetByte();
						if (result == 1)
							ok = DeleteTable( inFileToDelete, inContext, NULL, NULL, true);
					}
				}
				req->Release();
			}
		}
	}
	else
	{
		if (fIsRemote || OkToUpdate(err))
		{
			if (inFileToDelete->GetOwner() == this)
			{
				bool aEteLibere = false;
				occupe();
				
				sLONG n = inFileToDelete->GetNum();
				if (n > 0 && n <= fich.GetCount())
				{			
					if (fich[n] == inFileToDelete)
					{

						DataTableRegular* df = (DataTableRegular*)inFileToDelete->GetDF();
						if (!fIsRemote)
							df->Retain();

						aEteLibere = true;
						libere();
						if (fIsRemote || df->StartDeleting())
						{
							aEteLibere = false;
							occupe();
							if (fich[n] == inFileToDelete)
							{
								fich[n] = nil;
								VUUID xid;
								inFileToDelete->GetUUID(xid);
								DelObjectID(objInBase_Table, inFileToDelete, xid);
								aEteLibere = true;
								libere();

								bool mustFullyDelete = inFileToDelete->GetFullyDeleteRecordsState();
								inFileToDelete->DelConstraint();
								inFileToDelete->DelConstraintCols();

								if (!fIsRemote)
									inFileToDelete->SetExtraProperties(nil, false, inContext);
								inFileToDelete->Drop(inContext, InProgress);
								inFileToDelete->UnRegisterForLang();
								inFileToDelete->Release();

		#if debugLeakCheck_NbLoadedRecords
								if (debug_candumpleaks)
									DumpStackCrawls();
		#endif

								if (!fIsRemote)
								{
									KillDataFile(df->GetTrueNum(), InProgress, event, mustFullyDelete, n);

									df->StopDeleting();
								}
							}
							else
							{
								if (!fIsRemote)
									df->StopDeleting();
							}
							ok = true;
						}
						else err = df->ThrowError(VE_DB4D_SOME_RECORDS_ARE_STILL_LOCKED, noaction);


						if (!fIsRemote)
							df->Release();

						if (event != nil)
							event->Unlock();

						/*
						if (!fIsRemote)
						{
							if (df != nil)
							{
								df->StartDeleting(InProgress, event);
								df->Release();
							}
						}
						*/
					}
					
					if (!fIsRemote)
						Touch();
				}
				if (!aEteLibere)
					libere();
			}
			if (!fIsRemote)
				ClearUpdating();
		}
	}

	if (err != VE_OK)
	{
		err = ThrowError(VE_DB4D_CANNOT_DELETE_TABLE, DBaction_DeletingTable);
	}
	
	return ok;
}

void Base4D::SetRetainedBackupSettings(IBackupSettings* inSettings)
{
	if (inSettings == NULL)
	{
		XBOX::ReleaseRefCountable(&fBackupSettings);
	}
	else
	{
		XBOX::CopyRefCountable(&fBackupSettings,inSettings);
	}
}

const IBackupSettings* Base4D::RetainBackupSettings()const
{	
	if (fBackupSettings)
		fBackupSettings->Retain();
	return fBackupSettings;
}

VError Base4D::LoadFromBag( const VValueBag& inBag, VBagLoader *inLoader)
{
	VError err = VE_OK;
	
	// get uuid
	VUUID uuid;
	inLoader->GetUUID( inBag, uuid);
	
	// comment je fixe le uuid de mon owner ?
	
	// get name
	inBag.GetString( DB4DBagKeys::name, fBaseName);
	if (!IsValid4DName( fBaseName))
		err = ThrowError(VE_DB4D_INVALIDBASENAME, DBaction_CreatingBase);
	
	return err;
}


VError Base4D::FromBag(const VValueBag& inBag, EntityModelCatalog* catalog)
{
	VString name;
	VUUID xid;
	if (inBag.GetString(DB4DBagKeys::name, name))
		SetName(name, nil, false);
	if (inBag.GetVUUID(DB4DBagKeys::uuid, xid))
	{
		fID = xid;
		if (fStructure != nil)
			fStructure->fID = xid;
	}
	else
	{
		if (catalog != nil)
			catalog->TouchXML();
	}

	/*
	hbbloc.dialect = 0;
	hbbloc.collatorOptions = COL_ICU;

	VString locale_id;
	if (inBag.GetString(DB4DBagKeys::collation_locale, locale_id))
	{
		if (!VIntlMgr::GetDialectCodeWithRFC3066BisLanguageCode(locale_id, hbbloc.dialect, false))
		{
			hbbloc.dialect = 0;
		}
	}
	bool opt1 = false;
	if (inBag.GetBool(DB4DBagKeys::collator_ignores_middle_wildchar, opt1))
	{
		if (opt1)
			hbbloc.collatorOptions = hbbloc.collatorOptions | COL_IgnoreWildCharInMiddle;
	}

	if (inBag.GetBool(DB4DBagKeys::consider_only_dead_chars_for_keywords, opt1))
	{
		if (opt1)
			hbbloc.collatorOptions = hbbloc.collatorOptions | COL_ConsiderOnlyDeadCharsForKeywords;
	}

	ReleaseRefCountable(&fIntlMgr);
	if (hbbloc.dialect != 0)
	{
		fIntlMgr = VIntlMgr::Create(hbbloc.dialect, hbbloc.collatorOptions);
		if (fIntlMgr == 0)
			hbbloc.dialect = 0;
	}
	if (hbbloc.dialect == 0)
	{
		fIntlMgr = VIntlMgr::GetDefaultMgr();
		if (fIntlMgr != nil)
		{
			fIntlMgr->Retain();
			hbbloc.dialect = fIntlMgr->GetDialectCode();
			hbbloc.collatorOptions = fIntlMgr->GetCollator()->GetOptions();
		}
	}
	*/
	return VE_OK;
}

VError Base4D::SaveToBag( VValueBag& ioBag, VString& outKind) const
{
	VError err = VE_OK;
	
	occupe();
	
	// set properties
	VString name;
	GetName( name);
	ioBag.SetString( DB4DBagKeys::name, name);

	ioBag.SetVUUID( DB4DBagKeys::uuid, fID);

	if (fIntlMgr != nil)
	{
		VString locale_id;
		bool ok = VIntlMgr::GetRFC3066BisLanguageCodeWithDialectCode( fIntlMgr->GetDialectCode(), locale_id);
		if (testAssert(ok))
		{
			ioBag.SetString( DB4DBagKeys::collation_locale, locale_id);
		}

		DB4DBagKeys::collator_ignores_middle_wildchar.Set( &ioBag, (fIntlMgr->GetCollator()->GetOptions() & COL_IgnoreWildCharInMiddle) != 0);
		DB4DBagKeys::consider_only_dead_chars_for_keywords.Set( &ioBag, (fIntlMgr->GetCollator()->GetOptions() & COL_ConsiderOnlyDeadCharsForKeywords) != 0);
	}
	
	libere();
	
	outKind = L"base";
		
	return err;
}

typedef vector<VRefPtr<CDB4DSchema> > vectorOFschemas;

VError Base4D::SaveToBagDeep( VValueBag& ioBag, VString& outKind, bool inWithTables, bool inWithIndices, bool inWithRelations) const
{
	// save base meta-data
	VError err = SaveToBag( ioBag, outKind);
	
	//occupe();
	
	// put schema definitions
	// if (inWithSchemas)
	{
		vectorOFschemas schemas;
		err = ((Base4D*)this)->RetainAllSchemas(schemas);
		for (vectorOFschemas::iterator cur = schemas.begin(), end = schemas.end(); cur != end && err == VE_OK; cur++)
		{
			VDB4DSchema* vSchema = dynamic_cast<VDB4DSchema*>(cur->Get());
			err = ioBag.AddElement( vSchema);
		}

		/*
		VTaskLock lock(&fSchemaMutex);

		SchemaCatalogByID::const_iterator		iter = fSchemasByID.begin();
		while ( iter != fSchemasByID.end() )
		{
			if ( iter->second != NULL )
			{
				CDB4DSchema*			cSchema = iter->second;
				VDB4DSchema*			vSchema = dynamic_cast<VDB4DSchema*>(cSchema);
				if ( vSchema != NULL )
					err = ioBag.AddElement( vSchema);
			}

			iter++;
		}
		*/
	}

	// put tables definitions
	if (inWithTables)
	{
		sLONG count = GetNBTable();
		for( sLONG i = 1 ; (i <= count) && (err == VE_OK) ; ++i)
		{
			Table *table = RetainTable( i);
			if (table != NULL)
			{
				err = ioBag.AddElement( table);
				table->Release();
			}
		}
	}
	
	// put indexes definitions
	if (inWithIndices)
	{
		vector<IndexInfo*> lesindex;
		LockIndexes();
		for (IndexMap::const_iterator cur = fAllIndexes.begin(), end = fAllIndexes.end(); cur != end && err == VE_OK ; cur++)
		{
			IndexInfo* ind = cur->second;
			ind->Retain();
			lesindex.push_back(ind);
		}
		UnLockIndexes();

		for (vector<IndexInfo*>::iterator cur = lesindex.begin(), end = lesindex.end(); cur != end && err == VE_OK ; cur++)
		{
			IndexInfo* ind = *cur;
			err = ioBag.AddElement(ind);
		}

		for (vector<IndexInfo*>::iterator cur = lesindex.begin(), end = lesindex.end(); cur != end; cur++)
		{
			IndexInfo* ind = *cur;
			ind->Release();
		}

	}
	
	// put relations
	if (inWithRelations)
	{
		sLONG count = fRelations.GetCount();
		for( sLONG i = 0 ; (i < count) && (err == VE_OK) ; ++i)
		{
			Relation *rel = RetainRelation( i);
			if (rel != NULL)
			{
				err = ioBag.AddElement( rel);
				rel->Release();
			}
		}
	}

	// put structure extra properties
	if (fStructure != nil)
		err = PutExtraPropertiesInBag( DB4DBagKeys::base_extra, fStructure, ioBag, nil);
	else
		err = PutExtraPropertiesInBag( DB4DBagKeys::base_extra, const_cast<Base4D*>( this), ioBag, nil);

	//libere();
		
	return err;
}


VError Base4D::PutIndexDefIntoBag(VValueBag& ioBag) const
{
	VError err = VE_OK;
	vector<IndexInfo*> lesindex;
	LockIndexes();
	for (IndexMap::const_iterator cur = fAllIndexes.begin(), end = fAllIndexes.end(); cur != end && err == VE_OK ; cur++)
	{
		IndexInfo* ind = cur->second;
		ind->Retain();
		lesindex.push_back(ind);
	}
	UnLockIndexes();

	for (vector<IndexInfo*>::iterator cur = lesindex.begin(), end = lesindex.end(); cur != end && err == VE_OK ; cur++)
	{
		IndexInfo* ind = *cur;
		err = ioBag.AddElement(ind);
	}

	for (vector<IndexInfo*>::iterator cur = lesindex.begin(), end = lesindex.end(); cur != end && err == VE_OK ; cur++)
	{
		IndexInfo* ind = *cur;
		ind->Release();
	}
	return err;
}

/*
VError Base4D::LoadIndicesFromBag(const VValueBag* inBag)
{
	fIndexesBag = inBag->RetainElements( DB4DBagKeys::index);  // l'analyse se fera plus tard
	return VE_OK;
}
*/

/*
IndexInfo* Base4D::RetainIndex( sLONG inIndexNum) const
{
	LockIndexes();
	
	IndexInfo* ind = NULL;
	if (inIndexNum>=1 && inIndexNum<=lbi.GetCount())
	{
		ind = lbi[inIndexNum];
		if (ind != NULL)
		{
			ind->Retain();
		}
	}
	
	UnLockIndexes();

	return ind;
}
*/


IndexInfo* Base4D::FindAndRetainIndex(IndexInfo *xind, Boolean MatchTyp) const
{
	IndexInfo *ind, *res = nil;

	sLONG xindtyp;
	if (xind->GetHeader() == nil) 
		xindtyp = 0;
	else
		xindtyp = xind->GetHeader()->GetTyp();

	LockIndexes();

	for (IndexMap::const_iterator cur = fAllIndexes.begin(), end = fAllIndexes.end(); cur != end && res == nil; cur++)
	{
		ind=cur->second;
		{
			if (ind->MatchType(xind))
			{
				Boolean oktyp = true;
				if (MatchTyp)
				{
					if  ( (ind->GetHeader()->GetTyp() == xindtyp)
						|| (ind->IsAuto() && (xindtyp == DB4D_Index_BtreeWithCluster || xindtyp == DB4D_Index_Btree) )
						|| (ind->GetHeader()->GetTyp() == DB4D_Index_BtreeWithCluster && (xindtyp == DB4D_Index_Btree || xind->IsAuto()))
						|| (ind->GetHeader()->GetTyp() == DB4D_Index_Btree && (xindtyp == DB4D_Index_BtreeWithCluster || xind->IsAuto())) )
					{
						oktyp = true;
					}
					else
						oktyp = false;
				}

				if (oktyp || (xind->IsAuto() && ind->IsAuto()) )
				{
					if (ind->Egal(xind))
					{
						res = ind;
						res->Retain();
					}
				}
			}
		}
	}

	UnLockIndexes();

	return(res);
}

/*
IndexInfo* Base4D::CheckIndexOccupeAndvalid(IndexInfo *xind, Boolean MustBeValid, BaseTaskInfo* context, Boolean Occupy) const
{
	IndexInfo* ind = xind;
	if (ind != nil)
	{
		if (Occupy)
		{
			ind->Open();
			if (MustBeValid)
			{
				if (!ind->isValid(context))
				{
					ind->Close();
					ind->Release();
					ind = nil;
				}
			}
		}
	}
	return ind;
}
*/

IndexInfo* Base4D::FindAndRetainAnyIndex(IndexInfo *xind, Boolean MustBeSortable, Boolean MustBeValid, BaseTaskInfo* context, Boolean Occupy) const
{
	IndexInfo *ind, *res = nil;

	LockIndexes();

	for (IndexMap::const_iterator cur = fAllIndexes.begin(), end = fAllIndexes.end(); cur != end && res == nil; cur++)
	{
		ind=cur->second;
		{
			if (ind->MatchType(xind))
			{
				if ((MustBeSortable && ind->MayBeSorted()) || !MustBeSortable)
				{
					if (ind->Egal(xind))
					{
						if ((MustBeValid && ind->AskForValid(context)) || !MustBeValid)
						{
							res = ind;
							res->Retain();
						}
					}
				}
			}
		}
	}

	UnLockIndexes();

	//res = CheckIndexOccupeAndvalid(res, MustBeValid, context, Occupy);

	return(res);
}

/*
IndexInfo* Base4D::FindAndRetainIndex(FieldsArray* fields, sLONG NbFields, Boolean MustBeValid, BaseTaskInfo* context, Boolean Occupy) const
{
	IndexInfo *res = nil, *ind;
	
	LockIndexes();
	
	for (IndexMap::const_iterator cur = fAllIndexes.begin(), end = fAllIndexes.end(); cur != end && res == nil; cur++)
	{
		ind=cur->second;
		if (ind!=nil)
		{
			if (ind->MatchIndex(fields, NbFields))
			{
				if ((MustBeValid && ind->isValid(context)) || !MustBeValid)
				{
					ind->UseForQuery();
					res = ind;
					res->Retain();
				}
			}
		}
	}
	
	UnLockIndexes();

	res = CheckIndexOccupeAndvalid(res, MustBeValid, context, Occupy);

	return res;
}
*/


IndexInfo* Base4D::FindAndRetainIndexLexico(sLONG numtable, sLONG nc, Boolean MustBeValid, BaseTaskInfo* context, Boolean Occupy) const
{
	IndexInfo *ind,*ind2;

	LockIndexes();

	ind2=nil;
	for (IndexMap::const_iterator cur = fAllIndexes.begin(), end = fAllIndexes.end(); cur != end && ind2 == nil; cur++)
	{
		ind=cur->second;
		if (ind!=nil)
		{
			if (ind->MatchType(DB4D_Index_OnKeyWords))
			{
				if (((IndexInfoFromFieldLexico*)ind)->MatchField(numtable,nc))
				{
					if ((MustBeValid && ind->AskForValid(context)) || !MustBeValid)
					{
						ind2=ind;
						ind2->Retain();
						break;
					}
				}
			}
		}
	}

	UnLockIndexes();

	//ind2 = CheckIndexOccupeAndvalid(ind2, MustBeValid, context, Occupy);

	return(ind2);
}


IndexInfo* Base4D::FindAndRetainIndexSimple(sLONG numtable, sLONG nc, uBOOL sortable, Boolean MustBeValid, BaseTaskInfo* context, Boolean Occupy) const
{
	IndexInfo *ind,*ind2;

	LockIndexes();

	ind2=nil;
	for (IndexMap::const_iterator cur = fAllIndexes.begin(), end = fAllIndexes.end(); cur != end && ind2 == nil; cur++)
	{
		ind=cur->second;
		if (ind!=nil)
		{
			if (ind->MatchType(DB4D_Index_OnOneField))
			{
				if (((IndexInfoFromField*)ind)->MatchField(numtable,nc))
				{
					if (sortable)
					{
						if (ind->MayBeSorted())
						{
							if ((MustBeValid && ind->AskForValid(context)) || !MustBeValid)
							{
								ind2=ind;
								ind2->Retain();
								break;
							}
						}
					}
					else
					{
						if ((MustBeValid && ind->AskForValid(context)) || !MustBeValid)
						{
							ind2=ind;
							ind2->Retain();
							break;
						}
					}
				}
			}
		}
	}

	UnLockIndexes();

	//ind2 = CheckIndexOccupeAndvalid(ind2, MustBeValid, context, Occupy);

	return(ind2);
}


IndexInfo* Base4D::FindAndRetainIndexByName(const VString& inIndexName, Boolean MustBeValid, BaseTaskInfo* context, Boolean Occupy) const
{
	IndexInfo* result = nil;
	LockIndexes();

	for (IndexMap::const_iterator cur = fAllIndexes.begin(), end = fAllIndexes.end(); cur != end && result == nil; cur++)
	{
		IndexInfo* ind=cur->second;
		if (ind != nil)
		{
			if (ind->GetName() == inIndexName)
			{
				if ((MustBeValid && ind->AskForValid(context)) || !MustBeValid)
				{
					ind->Retain();
					result = (IndexInfo*)ind;
				}
			}
		}
	}

	UnLockIndexes();

	//result = CheckIndexOccupeAndvalid(result, MustBeValid, context, Occupy);
	return result;
}


IndexInfo* Base4D::FindAndRetainIndexByUUID( const VUUID& inUUID, Boolean MustBeValid, BaseTaskInfo* context, Boolean Occupy) const
{
	IndexInfo *result = NULL;
	LockIndexes();
	
	IndexMap::const_iterator found = fAllIndexes.find( inUUID);
	if (found != fAllIndexes.end())
	{
		IndexInfo *ind = found->second;
		if (ind != NULL)
		{
			if ((MustBeValid && ind->AskForValid(context)) || !MustBeValid)
			{
				ind->Retain();
				result = (IndexInfo*)ind;
			}
		}
	}

	UnLockIndexes();

	//result = CheckIndexOccupeAndvalid(result, MustBeValid, context, Occupy);
	return result;
}



void Base4D::RemoveDependenciesFromIndex(const VUUID& indexid)
{
	LockIndexes();
	IndexInfo* ind;
	IndexMap::iterator found = fAllIndexes.find(indexid);
	if (found == fAllIndexes.end())
		ind = nil;
	else
		ind = found->second;
	UnLockIndexes();
	if (ind != nil)
	{
		ind->DeleteFromDependencies();
	}
}


VError Base4D::CreIndexOnField(Field* target, sLONG typindex, Boolean UniqueKeys, CDB4DBaseContext* inContext, VDB4DProgressIndicator* InProgress,
															 const VString* inName,	IndexInfo **outIndex, Boolean ForceCreate, VSyncEvent* event)
{
	VError err;
	
	if (target != nil)
	{
		err = CreIndexOnField(target->GetOwner()->GetNum(), target->GetPosInRec(), typindex, UniqueKeys, inContext, InProgress, inName, outIndex, ForceCreate, event);
	}
	else 
		err = ThrowError(VE_DB4D_WRONGFIELDREF, DBaction_CreatingIndex);
	
	return err;
}


VError Base4D::SubCreIndex(IndexInfo *ind, CDB4DBaseContext* inContext, VDB4DProgressIndicator* InProgress,
							const VString* inName,	IndexInfo **outIndex, Boolean ForceCreate, VSyncEvent* event)
{
	sLONG n;
	VError err = VE_OK;

	if (fIsRemote)
	{
	}
	else
	{
		ind->GenID();
		if (inName != nil)
		{
			err = ind->SetName(*inName, inContext);
		}

		if (err == VE_OK)
		{
			if (ind->GetHeader() == nil)
			{
				err = ThrowError(VE_DB4D_INVALIDINDEXTYPE, DBaction_CreatingIndex);
			}
			else
			{
				Boolean deja = false;
				IndexInfo* ind2 = FindAndRetainIndex(ind, true);
				VUUID id;
				if (!ForceCreate)
				{
					if (ind2 == nil)
						ForceCreate = true;
					else
					{
						ind->Release();
						if (outIndex != nil)
						{
							ind = ind2;
							*outIndex = ind;
						}
						else
						{
							ind2->Release();
						}
					}
				}
				else
				{
					if (ind2 != nil)
					{
						deja = true;
						id = ind2->GetID();
						ind2->Release();
					}
				}
				if (ForceCreate)
				{
					if (OkToUpdate(err))
					{
						ind->CheckIfIsDelayed();
						
						
						VJSONObject* indexRequestOriginatorInfo = RetainRequestOriginatorInfo(inContext);

						if (deja)
						{
							//if (InProgress != nil) InProgress->Retain();
							AddDeleteIndex(this, id, InProgress, nil);
						}

						AddCreateIndex(ind, InProgress, event, indexRequestOriginatorInfo);
						event = nil;

						QuickReleaseRefCountable(indexRequestOriginatorInfo);

						// on peut se permettre de releaser l'index car un retain a ete fait par le AddCreateIndex
						if (outIndex != nil)
						{
							*outIndex = ind;
						}
						else
							ind->Release();
						ClearUpdating();
					}
					else
						err = ThrowError(err, DBaction_CreatingIndex);
				}
			}
		}
	}

	if (err != VE_OK)
		if (testAssert(ind != nil))
			ind->Release();

	if (event != nil)
	{
		event->Unlock();
		//event->Release();
	}
	return err;
}


VError Base4D::CreIndexOnField(sLONG nf, sLONG nc, sLONG typindex, Boolean UniqueKeys, CDB4DBaseContext* inContext, VDB4DProgressIndicator* InProgress,
															 const VString* inName,	IndexInfo **outIndex, Boolean ForceCreate, VSyncEvent* event)
{
	VError err = VE_OK;

	Field* cri = RetainField(nf,nc);
	if (cri != nil)
	{
		if (cri->IsIndexable())
		{
			if (fIsRemote)
			{
				IRequest *req = CreateRequest( inContext, Req_CreateIndexOnOneField + kRangeReqDB4D);
				if (req == NULL)
				{
					err = ThrowError( memfull, DBaction_CreatingIndex);
				}
				else
				{
					req->PutBaseParam( this);
					req->PutLongParam( nf);
					req->PutLongParam( nc);
					req->PutLongParam( typindex);
					req->PutBooleanParam( UniqueKeys);
					req->PutStringParam( inName);
					req->PutBooleanParam( ForceCreate);
					req->PutBooleanParam( event != nil);
					req->PutProgressParam(InProgress);

					err = req->GetLastError();
					if (err == VE_OK)
					{
						err = req->Send();
					}
					req->Release();
				}
			}
			else
			{
				IndexInfo *ind = CreateIndexInfoField(this, nf, nc, typindex, UniqueKeys, cri->GetTyp());
				if (ind!=nil)
				{
					err = SubCreIndex(ind, inContext, InProgress, inName, outIndex, ForceCreate, event);
					event = nil;
				}
				else 
					err = ThrowError(memfull, DBaction_CreatingIndex);
			}
		}
		else
			err = ThrowError(VE_DB4D_WRONGTYPE, DBaction_CreatingIndex);

		cri->Release();
	}
	else
		err = ThrowError(VE_DB4D_WRONGFIELDREF, DBaction_CreatingIndex);

	if (event != nil)
	{
		event->Unlock();
		//event->Release();
	}
	return(err);
}

VError Base4D::CreIndexOnFields(FieldNuplet *fields, sLONG typindex, Boolean UniqueKeys, CDB4DBaseContext* inContext, VDB4DProgressIndicator* InProgress,
																const VString* inName,	IndexInfo **outIndex, Boolean ForceCreate, VSyncEvent* event)
{
	VError err = VE_OK;
	
	if (fIsRemote)
	{
		if (testAssert(fields != NULL))
		{
			IRequest *req = CreateRequest( inContext, Req_CreateIndexOnMultipleField + kRangeReqDB4D);
			if (req == NULL)
			{
				err = ThrowError( memfull, DBaction_CreatingIndex);
			}
			else
			{
				req->PutBaseParam( this);
				err = fields->PutInto( *req->GetOutputStream());
				if (err == VE_OK)
				{
					req->PutLongParam( typindex);
					req->PutBooleanParam( UniqueKeys);
					req->PutStringParam( inName);
					req->PutBooleanParam( ForceCreate);
					req->PutBooleanParam( event != nil);
					req->PutProgressParam(InProgress);

					err = req->GetLastError();
					if (err == VE_OK)
					{
						err = req->Send();
					}
				}
				req->Release();
			}
		}
	}
	else
	{
		IndexInfo *ind = new IndexInfoFromMultipleField(this, fields, typindex, UniqueKeys);
		if (ind!=nil)
		{
			err = SubCreIndex(ind, inContext, InProgress, inName, outIndex, ForceCreate, event);
			event = nil;
		}
		else 
			err = ThrowError(memfull, DBaction_CreatingIndex);
	}

	if (event != nil)
	{
		event->Unlock();
		//event->Release();
	}
	return(err);
}


VError Base4D::CreateIndex( const VValueBag& inBag, VBagLoader *inLoader, IndexInfo **outIndex, CDB4DBaseContext* inContext, bool loadonly)
{
	VError err = VE_OK;

	if (outIndex)
		*outIndex = nil;

	// count fields
	VIndex count_fields = inBag.GetElementsCount( DB4DBagKeys::field_ref);

	// get index kind
	VString kind;
	sLONG IndexInfoTyp;

	if (!inBag.GetString( DB4DBagKeys::kind, kind) || kind.EqualToString( CVSTR( "regular"), false))
	{
		IndexInfoTyp = (count_fields > 1) ? DB4D_Index_OnMultipleFields : DB4D_Index_OnOneField;
	}
	else if (kind.EqualToString( CVSTR( "keywords"), false))
	{
		IndexInfoTyp = DB4D_Index_OnKeyWords;
	}
	else
	{
		IndexInfoTyp = DB4D_Index_None;
	}
	
	IndexInfo *ind = CreateIndexInfo( IndexInfoTyp);
	if (ind == nil)
		err = ThrowError( VE_DB4D_INVALIDINDEX, DBaction_CreatingIndex);
	else
	{
		ind->SetDB( this);
		
		err = ind->LoadFromBagWithLoader( inBag, inLoader, inContext);

		if (err == VE_OK)
		{
			if (!loadonly)
				err = SubCreIndex( ind, inContext, nil, nil, outIndex, false, nil);	// releases ind
		}
		else
			ind->Release();
	}
	
	if (outIndex)
		*outIndex = ind;

	return err;	
}


void Base4D::DeleteIndexByRef(IndexInfo* ind, CDB4DBaseContext* inContext, VDB4DProgressIndicator* InProgress, VSyncEvent* event)
{
	VError err = VE_OK;
	if (fIsRemote)
	{
		IRequest *req = CreateRequest( inContext, Req_DropIndexByRef + kRangeReqDB4D);
		if (req == NULL)
		{
			err = ThrowError( memfull, DBaction_DeletingIndex);
		}
		else
		{
			req->PutBaseParam( this);
			req->PutBooleanParam( event != nil);
			req->PutIndexParam( ind);
			req->PutProgressParam(InProgress);
			
			err = req->GetLastError();
			if (err == VE_OK)
			{
				err = req->Send();
			}
			req->Release();
		}
	}
	else
	{
		if (OkToUpdate(err))
		{
			//if (InProgress != nil) InProgress->Retain();
			AddDeleteIndex(this,ind->GetID(),InProgress, event);
			event = nil;
			ClearUpdating();
		}
	}
	if (event != nil)
	{
		event->Unlock();
		//event->Release();
	}
}


void Base4D::RebuildIndexByRef(IndexInfo* ind, CDB4DBaseContext* inContext, VDB4DProgressIndicator* InProgress, VSyncEvent* event)
{
	VError err = VE_OK;
	if (fIsRemote)
	{
		IRequest *req = CreateRequest( inContext, Req_RebuildIndexByRef + kRangeReqDB4D);
		if (req == NULL)
		{
			err = ThrowError( memfull, DBaction_RebuildingIndex);
		}
		else
		{
			req->PutBaseParam( this);
			req->PutIndexParam( ind);
			req->PutProgressParam(InProgress);
			
			err = req->GetLastError();
			if (err == VE_OK)
			{
				err = req->Send();
			}
			req->Release();
		}
	}
	else
	{
		if (OkToUpdate(err))
		{
			ind->SetInvalidOnDisk();
			ind->SetInvalid();

			VJSONObject* indexRequestOriginatorInfo = RetainRequestOriginatorInfo(inContext);

			//if (InProgress != nil) InProgress->Retain();
			AddRebuildIndex(ind, InProgress, event, indexRequestOriginatorInfo);
			event = nil;

			QuickReleaseRefCountable(indexRequestOriginatorInfo);
			ClearUpdating();
		}
	}
	if (event != nil)
	{
		event->Unlock();
		//event->Release();
	}
}


VError Base4D::DeleteIndexOnFields(FieldNuplet *fields, CDB4DBaseContext* inContext, sLONG typindex, VDB4DProgressIndicator* InProgress, VSyncEvent* event)
{
	IndexInfo *ind;
	sLONG n;
	VError err;
	
	err=VE_OK;
	if (fIsRemote)
	{
		if (testAssert(fields != NULL))
		{
			IRequest *req = CreateRequest( inContext, Req_DropIndexOnMultipleField + kRangeReqDB4D);
			if (req == NULL)
			{
				err = ThrowError( memfull, DBaction_DeletingIndex);
			}
			else
			{
				req->PutBaseParam( this);
				err = fields->PutInto( *req->GetOutputStream());
				if (err == VE_OK)
				{
					req->PutLongParam( typindex);
					req->PutBooleanParam( event != nil);
					req->PutProgressParam(InProgress);

					err = req->GetLastError();
					if (err == VE_OK)
					{
						err = req->Send();
					}
				}
				req->Release();
			}
		}
	}
	else
	{
		if (OkToUpdate(err))
		{
			ind=new IndexInfoFromMultipleField(this,fields,typindex,false);
			if (ind!=nil)
			{
				IndexInfo* ind2 = FindAndRetainIndex(ind, typindex != 0);
				if (ind2 != nil)
				{
					//if (InProgress != nil) InProgress->Retain();
					AddDeleteIndex(this,ind2->GetID(),InProgress, event);
					event = nil;
					ind2->Release();
				}
				ind->Release();
			}
			else 
				err = ThrowError(memfull, DBaction_DeletingIndex);
			ClearUpdating();
		}
	}

	if (event != nil)
	{
		event->Unlock();
		//event->Release();
	}
	return(err);
}


VError Base4D::DeleteIndexOnField(sLONG nf, sLONG nc, CDB4DBaseContext* inContext, sLONG typindex, VDB4DProgressIndicator* InProgress, VSyncEvent* event)
{
	IndexInfo *ind;
	sLONG n;
	VError err;
	
	err=VE_OK;
	if (fIsRemote)
	{
		IRequest *req = CreateRequest( inContext, Req_DropIndexOnOneField + kRangeReqDB4D);
		if (req == NULL)
		{
			err = ThrowError( memfull, DBaction_DeletingIndex);
		}
		else
		{
			req->PutBaseParam( this);
			req->PutLongParam( nf);
			req->PutLongParam( nc);
			req->PutLongParam( typindex);
			req->PutBooleanParam( event != nil);
			req->PutProgressParam(InProgress);

			err = req->GetLastError();
			if (err == VE_OK)
			{
				err = req->Send();
			}
			req->Release();
		}
	}
	else
	{
		if (OkToUpdate(err))
		{
			ind=new IndexInfoFromField(this,nf,nc,typindex,false, false);
			if (ind!=nil)
			{
				IndexInfo* ind2 = FindAndRetainIndex(ind, typindex != 0);
				if (ind2 != nil)
				{
					//if (InProgress != nil) InProgress->Retain();
					AddDeleteIndex(this,ind2->GetID(),InProgress, event);
					event = nil;
					ind2->Release();
				}
				ind->Release();
			}
			else
				err = ThrowError(memfull, DBaction_DeletingIndex);
			ClearUpdating();
		}
	}
	if (event != nil)
	{
		event->Unlock();
		//event->Release();
	}
	return(err);
}


VError Base4D::DeleteIndexOnField(Field* target, CDB4DBaseContext* inContext, sLONG typindex, VDB4DProgressIndicator* InProgress, VSyncEvent* event)
{
	VError err;
	
	if (target != nil)
	{
		err = DeleteIndexOnField(target->GetOwner()->GetNum(), target->GetPosInRec(), inContext, typindex, InProgress, event);
		event = nil;
	}
	else
		err = ThrowError(VE_DB4D_WRONGFIELDREF, DBaction_DeletingIndex);
	
	if (event != nil)
	{
		event->Unlock();
		//event->Release();
	}
	return err;
}



VError Base4D::CreFullTextIndexOnField(Field* target, CDB4DBaseContext* inContext, VDB4DProgressIndicator* InProgress,
										const VString* inName,	IndexInfo **outIndex, Boolean ForceCreate, VSyncEvent* event)
{
	VError err;

	if (target != nil)
	{
		err = CreFullTextIndexOnField(target->GetOwner()->GetNum(), target->GetPosInRec(), inContext, InProgress, inName, outIndex, ForceCreate, event);
		event = nil;
	}
	else 
		err = ThrowError(VE_DB4D_WRONGFIELDREF, DBaction_CreatingIndex);

	if (event != nil)
	{
		event->Unlock();
		//event->Release();
	}
	return err;
}


VError Base4D::CreFullTextIndexOnField(sLONG nf, sLONG nc, CDB4DBaseContext* inContext, VDB4DProgressIndicator* InProgress,
										const VString* inName,	IndexInfo **outIndex, Boolean ForceCreate, VSyncEvent* event)
{
	VError err = VE_OK;

	Field* cri = RetainField(nf,nc);
	if (cri != nil)
	{
		if (cri->IsFullTextIndexable())
		{
			if (fIsRemote)
			{
				IRequest *req = CreateRequest( inContext, Req_CreateFullTextIndexOnOneField + kRangeReqDB4D);
				if (req == NULL)
				{
					err = ThrowError( memfull, DBaction_CreatingIndex);
				}
				else
				{
					req->PutBaseParam( this);
					req->PutLongParam( nf);
					req->PutLongParam( nc);
					req->PutStringParam( inName);
					req->PutBooleanParam( ForceCreate);
					req->PutBooleanParam( event != nil);
					req->PutProgressParam(InProgress);

					err = req->GetLastError();
					if (err == VE_OK)
					{
						err = req->Send();
					}
					req->Release();
				}
			}
			else
			{
				IndexInfo *ind = new IndexInfoFromFieldLexico(this, nf, nc, DB4D_Index_BtreeWithCluster );
				if (ind!=nil)
				{
					err = SubCreIndex(ind, inContext, InProgress, inName, outIndex, ForceCreate, event);
					event = nil;
				}
				else 
					err = ThrowError(memfull, DBaction_CreatingIndex);
			}
		}
		else 
			err = ThrowError(VE_DB4D_WRONGTYPE, DBaction_CreatingIndex);

		cri->Release();
	}
	else 
		err = ThrowError(VE_DB4D_WRONGFIELDREF, DBaction_CreatingIndex);

	if (event != nil)
	{
		event->Unlock();
		//event->Release();
	}
	return(err);
}


VError Base4D::DeleteFullTextIndexOnField(sLONG nf, sLONG nc, CDB4DBaseContext* inContext, VDB4DProgressIndicator* InProgress, VSyncEvent* event)
{
	IndexInfo *ind;
	sLONG n;
	VError err;

	err=VE_OK;
	if (fIsRemote)
	{
		IRequest *req = CreateRequest( inContext, Req_DropFullTextIndexOnOneField + kRangeReqDB4D);
		if (req == NULL)
		{
			err = ThrowError( memfull, DBaction_DeletingIndex);
		}
		else
		{
			req->PutBaseParam( this);
			req->PutLongParam( nf);
			req->PutLongParam( nc);
			req->PutBooleanParam( event != nil);
			req->PutProgressParam(InProgress);

			err = req->GetLastError();
			if (err == VE_OK)
			{
				err = req->Send();
			}
			req->Release();
		}
	}
	else
	{
		if (OkToUpdate(err))
		{
			ind=new IndexInfoFromFieldLexico(this,nf,nc,DB4D_Index_BtreeWithCluster);
			if (ind!=nil)
			{
				IndexInfo* ind2 = FindAndRetainIndex(ind, false);
				if (ind2 != nil)
				{
					//if (InProgress != nil) InProgress->Retain();
					AddDeleteIndex(this,ind2->GetID(),InProgress, event);
					event = nil;
					ind2->Release();
				}
				ind->Release();
			}
			else
				err = ThrowError(memfull, DBaction_DeletingIndex);
			ClearUpdating();
		}
	}
	if (event != nil)
	{
		event->Unlock();
		//event->Release();
	}
	return(err);
}


VError Base4D::DeleteFullTextIndexOnField(Field* target, CDB4DBaseContext* inContext, VDB4DProgressIndicator* InProgress, VSyncEvent* event)
{
	VError err;

	if (target != nil)
	{
		err = DeleteFullTextIndexOnField(target->GetOwner()->GetNum(), target->GetPosInRec(), inContext, InProgress, event);
		event = nil;
	}
	else
		err = ThrowError(VE_DB4D_WRONGFIELDREF, DBaction_DeletingIndex);

	if (event != nil)
	{
		event->Unlock();
		//event->Release();
	}
	return err;
}



void Base4D::LockIndexes(void) const
{ 
	IndexSemaphore->occupe(); 
}

void Base4D::UnLockIndexes(void) const
{ 
	IndexSemaphore->libere(); 
}


void Base4D::ClearAllTableDependencies(bool includingRelations)
{
	TableArray::Iterator cur = fich.First(), end = fich.End();
	for (; cur != end; cur++)
	{
		Table* tt = *cur;
		if (tt != nil)
			tt->ClearAllDependencies(includingRelations);
	}

}


void Base4D::ReleaseAllTables()
{
	sLONG i,nb;
	
	occupe();
	
	nb = fich.GetCount();
	for (i=1;i<=nb;i++)
	{
		Table* fic = fich[i];
		if (fic !=nil)
		{ 
			/*
			VUUID xid;
			fic->GetUUID(xid);
			DelObjectID(objInBase_Table, fic, xid);
			*/
			// pas necessaire car appelle uniquement a la fermeture de la base
			fic->ReleaseAllFields();
			fic->Release();
		}
		fich[i] = nil;
	}
	
	libere();
}


void Base4D::FindField( const VString& pFileName, const VString& pFieldName, sLONG *pFileNumber, sLONG *pFieldNumber) const
{
	sLONG i,nb;
	Table* fic;
	VStr31 ss;
	
	occupe();
	
	*pFieldNumber = 0;
	*pFileNumber = 0,
	nb=fich.GetCount();
	
	for (i=1;i<=nb;i++)
	{
		fic=fich[i];
		if (fic != nil)
		{
			fic->GetName(ss);
			if (ss == pFileName)
			{
				*pFileNumber = i;
				*pFieldNumber = fic->FindField(pFieldName);
				break;
			}
		}
	}

	if (*pFileNumber == 0)
	{
		nb=fSystemTables.GetCount();

		for (i=1;i<=nb;i++)
		{
			fic=fSystemTables[i];
			if (fic != nil)
			{
				fic->GetName(ss);
				if (ss == pFileName)
				{
					*pFileNumber = -i;
					*pFieldNumber = fic->FindField(pFieldName);
					break;
				}
			}
		}
	}
	libere();
}


Field* Base4D::FindAndRetainFieldRef(const VString& pFileName, const VString& pFieldName) const
{
	sLONG i,nb;
	Table* fic;
	VStr31 ss;
	Field* result = nil;

	occupe();
	
	nb=fich.GetCount();

	for (i=1;i<=nb;i++)
	{

		fic=fich[i];
		if (fic != nil)
		{
			fic->GetName(ss);
			if (ss == pFileName)
			{
				result = fic->FindAndRetainFieldRef(pFieldName);
				break;
			}
		}
	}

	if (result == nil)
	{
		nb=fSystemTables.GetCount();

		for (i=1;i<=nb;i++)
		{
			fic=fSystemTables[i];
			if (fic != nil)
			{
				fic->GetName(ss);
				if (ss == pFileName)
				{
					result = fic->FindAndRetainFieldRef(pFieldName);
					break;
				}
			}
		}
	}

	libere();
	return result;
}



Field* Base4D::FindAndRetainFieldRef(const VString& pFieldName) const
{
	sLONG i,nb;
	Table* fic;
	VStr31 ss;
	Field* result = nil;
	VString tableName = pFieldName, fieldName = pFieldName;
	sLONG p = pFieldName.FindUniChar(CHAR_FULL_STOP);

	if (p != 0)
	{
		tableName.Remove(p,tableName.GetLength()-p+1);
		fieldName.Remove(1,p);

		occupe();

		nb=fich.GetCount();

		for (i=1;i<=nb;i++)
		{
			fic=fich[i];
			if (fic != nil)
			{
				fic->GetName(ss);
				if (ss == tableName)
				{
					result = fic->FindAndRetainFieldRef(fieldName);
					break;
				}
			}
		}

		if (result == nil)
		{
			nb=fSystemTables.GetCount();

			for (i=1;i<=nb;i++)
			{
				fic=fSystemTables[i];
				if (fic != nil)
				{
					fic->GetName(ss);
					if (ss == tableName)
					{
						result = fic->FindAndRetainFieldRef(fieldName);
						break;
					}
				}
			}
		}

		libere();
	}
	return result;
}



Field* Base4D::FindAndRetainFieldRef( const VValueBag& inRef, VBagLoader *inLoader) const
{
	Field *field = NULL;

	bool mayFindByName = true;
	if (!inLoader->FindReferencesByName())
	{
		VUUID uuid;
		if (inRef.GetVUUID( DB4DBagKeys::uuid, uuid))
		{
			mayFindByName = false;
			if (inLoader->ResolveUUID( uuid))
				field = FindAndRetainFieldRef( uuid);
		}
	}
	
	if ( (field == NULL) && mayFindByName)
	{
		const VValueBag *table_ref = inRef.RetainUniqueElement( DB4DBagKeys::table_ref);
		if (table_ref)
		{
			Table *table = FindAndRetainTableRef( *table_ref, inLoader);

			if (table != NULL)
			{
				VStr31 field_name;
				if (inRef.GetString( DB4DBagKeys::name, field_name))
				{
					field = table->FindAndRetainFieldRef( field_name);
				}
				table->Release();
			}

			table_ref->Release();
		}
	}

	return field;
}


Field* Base4D::FindAndRetainFieldRef(const VUUID& pRefID) const
{
	VObjLock lock(this);
	Field* result = nil;
	/*
	sLONG i,nb;
	Table* fic;
	VStr31 ss;

	occupe();
	
	nb=fich.GetCount();

	for (i=1;i<=nb;i++)
	{

		fic=fich[i];
		if (fic != nil)
		{
			result = fic->FindAndRetainFieldRef(pRefID);
			if (result != nil)
				break;
		}
	}
	
	libere();
	*/
	result = (Field*)GetObjectByID(objInBase_Field, pRefID);
	if (result != nil)
		result->Retain();
	return result;
}


sLONG Base4D::FindTable( const VString& pFileName) const
{
	sLONG result, i, nb;
	Table* fic;
	VStr31 ss;
	
	occupe();
	result = 0;
	
	nb=fich.GetCount();
	
	for (i=1;i<=nb;i++)
	{
		fic=fich[i];
		if (fic != nil)
		{
			fic->GetName(ss);
			if (ss == pFileName)
			{
				result = i;
				break;
			}
		}
	}

	if (result == 0)
	{
		nb=fSystemTables.GetCount();

		for (i=1;i<=nb;i++)
		{
			fic=fSystemTables[i];
			if (fic != nil)
			{
				fic->GetName(ss);
				if (ss == pFileName)
				{
					result = -i;
					break;
				}
			}
		}
	}

	libere();
	return(result);
}


Table* Base4D::FindAndRetainTableRef( const VString& pFileName) const
{
	sLONG i, nb;
	Table* result;
	Table* fic;
	VStr31 ss;

	occupe();
	result = nil;
	nb=fich.GetCount();

	for (i=1;i<=nb;i++)
	{
		fic=fich[i];
		if (fic != nil)
		{
			fic->GetName(ss);
			if (ss == pFileName)
			{
				result = fic;
				fic->Retain();
				break;
			}
		}
	}

	if (result == nil)
	{
		nb=fSystemTables.GetCount();

		for (i=1;i<=nb;i++)
		{
			fic=fSystemTables[i];
			if (fic != nil)
			{
				fic->GetName(ss);
				if (ss == pFileName)
				{
					result = fic;
					fic->Retain();
					break;
				}
			}
		}
	}

	libere();
	return(result);
}



Table* Base4D::FindAndRetainTableRefByRecordName( const VString& pRecName) const
{
	sLONG i, nb;
	Table* result;
	Table* fic;
	VStr31 ss;

	occupe();
	result = nil;
	nb=fich.GetCount();

	for (i=1;i<=nb;i++)
	{
		fic=fich[i];
		if (fic != nil)
		{
			fic->GetRecordName(ss);
			if (ss == pRecName)
			{
				result = fic;
				fic->Retain();
				break;
			}
		}
	}

	if (result == nil)
	{
		nb=fSystemTables.GetCount();

		for (i=1;i<=nb;i++)
		{
			fic=fSystemTables[i];
			if (fic != nil)
			{
				fic->GetRecordName(ss);
				if (ss == pRecName)
				{
					result = fic;
					fic->Retain();
					break;
				}
			}
		}
	}

	libere();
	return(result);
}

/*
Table* Base4D::FindAndRetainTableByRecordCreator(const CMethod* method) const
{
	sLONG i, nb;
	Table* result;
	Table* fic;

	occupe();
	result = nil;
	nb=fich.GetCount();

	for (i=1;i<=nb;i++)
	{
		fic=fich[i];
		if (fic != nil)
		{
			if (fic->GetFakeRecordCreator() == method)
			{
				result = fic;
				fic->Retain();
				break;
			}
		}
	}

	libere();
	return(result);
}
*/

Table* Base4D::FindAndRetainTableRef( const VUUID& pRefID) const
{
	VObjLock lock(this);
	Table* result;
	/*
	sLONG i, nb;
	Table* fic;
	VUUID refID;

	occupe();
	result = nil;
	nb=fich.GetCount();

	for (i=1;i<=nb;i++)
	{
		fic=fich[i];
		if (fic != nil)
		{
			fic->GetUUID(refID);
			if (refID == pRefID)
			{
				result = fic;
				fic->Retain();
				break;
			}
		}
	}

	libere();
	*/
	result = (Table*)GetObjectByID(objInBase_Table, pRefID);
	if (result != nil)
		result->Retain();
	return(result);
}


Table *Base4D::FindAndRetainTableRef( const VValueBag& inRef, VBagLoader *inLoader) const
{
	Table *table = NULL;

	bool mayFindByName = true;
	if (!inLoader->FindReferencesByName())
	{
		VUUID uuid;
		if (inRef.GetVUUID( DB4DBagKeys::uuid, uuid))
		{
			mayFindByName = false;
			if (inLoader->ResolveUUID( uuid))
				table = FindAndRetainTableRef( uuid);
		}
	}
	
	if ( (table == NULL) && mayFindByName)
	{
		VString name;
		if (inRef.GetString( DB4DBagKeys::name, name))
			table = FindAndRetainTableRef( name);
	}

	return table;
}



void Base4D::GetName( VString& outName) const
{
	//occupe();
	outName = fBaseName;

	if ((fStructure == nil) && (outName.GetLength()==0))
	{
		outName.FromCString("InternalDatabase");
	}

	//libere();
}


VError Base4D::SetName( VString& inName, CDB4DBaseContext* inContext, bool cantouch)
{
	VError err = VE_OK;
	VString name = inName;
	RemoveExtension(name);

	if (fIsRemote)
	{
	}
	else
	{
		occupe();
		
		if (IsValid4DName(name))
		{
			fBaseName = name;
			if (cantouch)
				Touch();
			RegisterForLang();
		}
		else
			err = ThrowError(VE_DB4D_INVALIDBASENAME, DBaction_ChangingBaseName);
			
		libere();
	}

	return err;
}


Boolean Base4D::OkToUpdate(VError& err) // attention : peut ne pas modifier err
{
	VObjLock lock(this);

	if (fWriteProtected)
	{
		err = ThrowError( VE_DB4D_DATABASEISWRITEPROTECTED, DBaction_TryingToUpdateDB);
		return false;
	}

	if (!IsLogValid())
	{
		if (fLogErrorStack != nil)
			VTask::GetErrorContext(true)->PushErrorsFromBag(*fLogErrorStack);
		err = ThrowError( VE_DB4D_CURRENT_JOURNAL_IS_INVALID, DBaction_TryingToUpdateDB);
		return false;
	}

	sLONG curtaskid = VTaskMgr::Get()->GetCurrentTaskID();

	CountMap::iterator found = fUpdateCounts.find(curtaskid);
	if (found == fUpdateCounts.end())
	{
		if (fIsClosing)
		{
			err = ThrowError( VE_DB4D_DATABASEISCLOSING, DBaction_TryingToUpdateDB);
			return false;
		}
		else
		{
			fUpdateCounts.insert(make_pair(curtaskid, 1));
			return true;
		}
	}
	else
	{
		found->second++;
		return true;
	}

}


void Base4D::ClearUpdating()
{
	VObjLock lock(this);
	if (!fWriteProtected)
	{
		sLONG curtaskid = VTaskMgr::Get()->GetCurrentTaskID();

		CountMap::iterator found = fUpdateCounts.find(curtaskid);
		if (testAssert(found != fUpdateCounts.end()))
		{
			if (testAssert(found->second > 0))
			{
				found->second--;
				if (found->second == 0)
				{
					fUpdateCounts.erase(found);
				}
			}
		}
	}

}


void Base4D::DisposeDataTables()
{
	for (DataTableVector::iterator cur = fDataTables.begin(), end = fDataTables.end(); cur != end; cur++)
	{
		DataTable* df = *cur;
		if (df != nil)
		{
			df->RemoveFromCache();
			df->Release();
		}
	}

	fDataTables.clear();
}


void Base4D::DisposeDataSegments()
{
	if (fIndexSegment != nil)
	{
		fIndexSegment->CloseSeg();
		fIndexSegment->Release();
		fIndexSegment = nil;
	}
	for( sLONG i=0 ; i < fSegments.GetCount(); i++) {
		fSegments[i]->CloseSeg();
		fSegments[i]->Release();
	}
	fSegments.Destroy();
}


void Base4D::DisposeIndexes()
{
	/*
	for( sLONG i=1;i<=lbi.GetCount();i++) {
		if (lbi[i]!=nil) lbi[i]->Release();
	}
	*/
	for (IndexMap::iterator cur = fAllIndexes.begin(), end = fAllIndexes.end(); cur != end; cur++)
	{
		IndexInfo* ind = cur->second;
		ind->SetInvalid();
		ind->Release();
	}
	fAllIndexes.clear();
}


VFile *Base4D::RetainDefaultDataFile() const
{
	// should look into fResourceFile for another custom path
	VFilePath path( fDefaultLocation);
	path.SetExtension( CVSTR( "4DD"));
	return new VFile( path); // should specify virtual kind
}

VFile *Base4D::RetainIndexFile() const
{
	VFile* indexFile = NULL;
	if ( fIndexSegment )
		indexFile = fIndexSegment->GetFile();
	if ( indexFile )
		indexFile->Retain();
	return indexFile;
}


VError Base4D::OpenRemote(Boolean CheckForUpdateOnly)
{
	VError err = VE_OK;

	if (testAssert(fIsRemote))
	{
		occupe();
		if (fLocalDB != nil && !CheckForUpdateOnly)
		{
			xbox_assert(fIntlMgr==nil);
			fIntlMgr = VIntlMgr::Create(fLocalDB->fStructure->hbbloc.dialect, fLocalDB->fStructure->hbbloc.collatorOptions);
			if (fIntlMgr == nil)
				err = memfull; // pas de throw volontairement, celui ci doit etre fait par VIntlMgr

#if 0 // a faire dans la prochaine
			sLONG nbtablecached = fLocalDB->fStructure->hbbloc.nbTableDef;

			for (sLONG i = 0; i < nbtablecached; i++)
			{
				StructElemDef* e = fLocalDB->LoadTableDef(i, err);
				if (err == VE_OK)
				{
					if (e != nil)
					{
						VConstPtrStream buf(e->GetDataPtr(), (VSize)e->GetDataLen());
						err = buf.OpenReading();
						if (err == VE_OK)
						{
							buf.SetNeedSwap(e->NeedSwap());
							err = UpdateTable(i, &buf);
						}
						buf.CloseReading();
						e->libere();
					}
				}
			}
#endif
		}

		sWORD reqid = Req_OpenBase + kRangeReqDB4D;
		if (CheckForUpdateOnly)
			reqid = Req_CheckBaseForUpdate + kRangeReqDB4D;

		IRequest* req = CreateRequest(nil, reqid);
		if (req != nil)
		{
			VStream *reqsend = req->GetOutputStream();
			
			fID.WriteToStream(reqsend);
			reqsend->PutLong( CheckForUpdateOnly ? fStamp : 0);		// sc 11/06/2008 ACI0058176, pass 0 as stamp when opening the remote base
			reqsend->PutLong( CheckForUpdateOnly ? fStampExtraProp : 0);
			reqsend->PutLong( CheckForUpdateOnly ? fRemoteStructureStampExtraProp : 0);

			sLONG nbtable = fich.GetCount();
			reqsend->PutLong(nbtable);
			for (sLONG i = 1; i <= nbtable; i++)
			{
				Table* tt = fich[i];
				if (tt == nil)
					reqsend->PutLong(0);
				else
					reqsend->PutLong(tt->GetStamp());
			}
			
			sLONG relCount = fRelations.GetCount();
			reqsend->PutLong( relCount);
			for( sLONG i = 0 ; i < relCount ; ++i )
			{
				Relation *rel = fRelations[i];
				if (rel == NULL)
					reqsend->PutLong(0);
				else
					reqsend->PutLong( rel->GetStamp());
			}

			reqsend->PutLong( CheckForUpdateOnly ? fIndexesStamp : -1);
			reqsend->PutLong( CheckForUpdateOnly ? fSchemasCatalogStamp : -1);

			err = reqsend->GetLastError();
			if (err == VE_OK)
			{
				err = req->Send();
				if (err == VE_OK)
				{
					VStream *reqget = req->GetInputStream();
					sLONG baseStamp = 0;
					err = reqget->GetLong( baseStamp);
					if (err == VE_OK)
					{
						if (baseStamp != 0)
						{
							uLONG olddialect = hbbloc.dialect;
							CollatorOptions oldoptions = hbbloc.collatorOptions;
							err = reqget->GetLong(hbbloc.dialect);
							if (err == VE_OK)
								err = reqget->GetByte(hbbloc.collatorOptions);

							if (fIntlMgr == nil)
							{
								fIntlMgr = VIntlMgr::Create(hbbloc.dialect, hbbloc.collatorOptions);
							}
							else
							{
								if (oldoptions != hbbloc.collatorOptions || olddialect != hbbloc.dialect)
								{
									fIntlMgr->Release();
									fIntlMgr = VIntlMgr::Create(hbbloc.dialect, hbbloc.collatorOptions);
								}
							}

							// update the tables
							sLONG numtable;
							if (err == VE_OK)
							{
								do 
								{
									err = reqget->GetLong(numtable);
									if (err == VE_OK && numtable != 0)
									{
										if (numtable < 0)
											err = DeleteTable(-numtable, reqget);
										else
											err = UpdateTable(numtable, reqget);
									}
								} while (numtable != 0 && err == VE_OK);
							}

							if (err == VE_OK)
							{
								// update the relations
								sLONG relStatus = 0;
								do 
								{
									err = reqget->GetLong( relStatus);		// read the relation status
									if (err == VE_OK && relStatus != 0)
									{
										sLONG relIndex = 0;
										err = reqget->GetLong( relIndex);
										if (err == VE_OK && relIndex >= 0)
										{
											if (relStatus == 1)
											{
												err = UpdateRelation( relIndex, reqget);
											}
											else if (relStatus == -1)
											{
												err = DeleteRelation( relIndex, reqget);
											}
										}
									}
								} while (relStatus != 0 && err == VE_OK);
							}

							if (err == VE_OK)
							{
								// update indexes
								// sc 08/01/2009 ACI0060249 and ACI0056353
								uLONG newIndexesStamp;
								err = reqget->GetLong( newIndexesStamp);

								if (err == VE_OK)
								{
									if (newIndexesStamp != 0)
									{
										uBYTE c;
										do 
										{
											err = reqget->GetByte(c);
											if (c == '+')
											{
												sLONG IndexInfoTyp;
												err = reqget->GetLong(IndexInfoTyp);
												if (err == VE_OK)
												{
													IndexInfo* ind = CreateIndexInfo(IndexInfoTyp);
													if (ind != nil)
													{
														err = ind->GetFrom(*reqget, this, false);
														ind->FinalizeForRemote();
														if (err == VE_OK)
														{
															err = ind->ValidateIndexInfo();
															if (err == VE_OK)
															{
																ind->SetDB(this);

																LockIndexes();
																try
																{
																	KillIndex(ind->GetID(), nil);
																	fAllIndexes.insert(make_pair(ind->GetID(), ind));
																	ind->Retain();
																}
																catch (...)
																{
																	err = ThrowError(memfull, noaction);
																}
																UnLockIndexes();

																if (err == VE_OK)
																{
																	ind->CalculDependence();
																	fDataTableOfIndexes->AddIndexRef(ind);
																	ind->AddToSystemTableIndexCols();
																}
															}
														}
														ind->Release();
													}
												}
											}
											else
											{
												if (c == '-')
												{
													VUUID xid;
													err = xid.ReadFromStream(reqget);
													if (err == VE_OK)
													{
														KillIndex(xid, nil);
													}
												}
											}
										} while(c != '.' && err == VE_OK);

										if (err == VE_OK)
											fIndexesStamp = newIndexesStamp;
									}
								}
							}

							if (err == VE_OK)
								fStamp = baseStamp;
						}
					}
					if (err == VE_OK)
					{
						sLONG baseExtraStamp = 0;
						err = reqget->GetLong( baseExtraStamp);
						if (err == VE_OK)
						{
							if (baseExtraStamp != 0)
							{
								VValueBag *extraBag = req->RetainValueBagReply( err);
								if (err == VE_OK)
									err = fExtra.SetExtraProperties( extraBag);
								ReleaseRefCountable( &extraBag);

								if (err == VE_OK)
									fStampExtraProp = baseExtraStamp;
							}
						}
					}
					if (err == VE_OK)
					{
						sLONG structExtraStamp = 0;
						err = reqget->GetLong( structExtraStamp);
						if (err == VE_OK)
						{
							if (structExtraStamp != 0)
							{
								VValueBag *extraBag = req->RetainValueBagReply( err);
								if (err == VE_OK)
									CopyRefCountable(&fRemoteStructureExtraProp, extraBag);
								ReleaseRefCountable( &extraBag);

								if (err == VE_OK)
									fRemoteStructureStampExtraProp = structExtraStamp;
							}
						}
					}
					if (err == VE_OK)
					{
						sLONG schemasCatalogStamp = req->GetLongReply(err);
						if (err == VE_OK)
						{
							if (schemasCatalogStamp != 0)
							{
								fSchemasByName.clear();
								fSchemasByID.clear();

								sLONG count = req->GetLongReply(err);
								if (err == VE_OK)
								{
									DB4D_SchemaID id = 0;
									VString name;

									while (count > 0)
									{
										id = req->GetLongReply(err);
										if (err == VE_OK)
											err = req->GetStringReply( name);
										if (err == VE_OK)
										{
											VDB4DSchema *schema = new VDB4DSchema( this, name, id);
											fSchemasByName[name] = schema;
											fSchemasByID[id] = schema;
											ReleaseRefCountable( &schema);
										}
										count--;
									}
								}
								fSchemasCatalogStamp = schemasCatalogStamp;
							}
						}
					}
				}
			}
			req->Release();
		}
		else
			err = ThrowError(memfull, DBaction_OpeningRemoteBase);

		libere();
	}

	if (err != VE_OK)
	{
		err = ThrowError(VE_DB4D_CANNOT_OPEN_REMOTE_BASE, DBaction_OpeningRemoteBase);
	}

	return err;
}


VError Base4D::UpdateTable(sLONG numtable, VStream* dataget)
{
	VError err = VE_OK;
	occupe();
	if (numtable > 0 && numtable <= kMaxTables)
	{
		if (numtable > fich.GetCount())
		{
			if (fich.SetCount(numtable, nil))
			{
				fNbTable = fich.GetCount();
			}
			else
			{
				err =  ThrowError(memfull, DBaction_UpdatingRemoteTable);
			}
		}
		if (err == VE_OK)
		{
			Table* tt = fich[numtable];
			if (tt == nil)
			{
				tt = new TableRegular(this, numtable, true, true);
				if (tt == nil)
					err = ThrowError(memfull, DBaction_UpdatingRemoteTable);
			}
			if (tt != nil)
			{
				err = tt->Update(dataget);
				if (err == VE_OK)
				{
					fich[numtable] = tt;
					tt->RegisterForLang(false);
				}
				else
					tt->Release();
			}

		}
	}
	else
		err = ThrowError(VE_DB4D_INVALID_TABLENUM, DBaction_UpdatingRemoteTable);
	libere();
	return err;
}


VError Base4D::DeleteTable(sLONG numtable, VStream* dataget)
{
	VError err = VE_OK;
	occupe();
	if (numtable > 0 && numtable <=fich.GetCount())
	{
		Table* tt = fich[numtable];
		if (tt != nil)
			DeleteTable(tt, nil, nil, nil, true);
	}

	libere();
	return err;
}


VError Base4D::UpdateRelation( sLONG inRelationIndex, VStream *inDataGet)
{
	VError err = VE_OK;

	if (inRelationIndex >= 0)
	{
		occupe();
		if (inRelationIndex >= fRelations.GetCount())
		{
			if (!fRelations.SetCount(inRelationIndex + 1, NULL))
				err = ThrowError( memfull, noaction);
		}
		if (err == VE_OK)
		{
			Relation *rel = fRelations[inRelationIndex];
			bool isNewRelation = (rel == NULL);
			if (rel == NULL)
			{
				rel = new Relation( this, inRelationIndex, true);
				if (rel == NULL)
					err = ThrowError( memfull, noaction);
			}
			if (rel != NULL)
			{
				err = rel->Update( inDataGet);
				if (err == VE_OK)
				{
					if (isNewRelation)
						rel->CalculDependence();	// sc 14/12/2010 ACI006854
					fRelations[inRelationIndex] = rel;
				}
				else
				{
					rel->Release();
				}
			}
		}
		libere();
	}
	return err;
}


VError Base4D::DeleteRelation( sLONG inRelationIndex, VStream *inDataGet)
{
	VError err = VE_OK;

	occupe();
	if (inRelationIndex >=0 && inRelationIndex < fRelations.GetCount())
	{
		Relation *rel = fRelations[inRelationIndex];
		if (rel != NULL)
			DeleteRelation( rel, NULL, true);
	}
	libere();

	return err;
}


IRequest* Base4D::CreateRequest( CDB4DBaseContext *inContext, sWORD inRequestID, Boolean legacy) const
{
	IStreamRequest* req = nil;
	if (legacy)
	{
		if (fLegacy_NetAccess != nil)
		{
			req = fLegacy_NetAccess->CreateRequest(inContext, inRequestID);
		}
	}
	else
	{
		if (fNetAccess != nil)
		{
			req = fNetAccess->CreateRequest(inContext, inRequestID);
		}
	}
	return (IRequest*)req;
}


VError Base4D::SendToClient(IRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;
	vector<sLONG> tableRemoteStamps, relRemoteStamps;
	sLONG baseRemoteStamp=0, nbRemoteTable=0, nbRemoteRel=0, baseRemoteExtraStamp=0, structRemoteExtraStamp = 0, schemasCatalogStamp = 0;

	VStream *reqget = inRequest->GetInputStream();
	// Get the remote base stamps
	err = reqget->GetLong(baseRemoteStamp);
	if (err == VE_OK)
		err = reqget->GetLong(baseRemoteExtraStamp);
	if (err == VE_OK)
		err = reqget->GetLong(structRemoteExtraStamp);
	// Get the remote tables stamps
	if (err == VE_OK)
		err = reqget->GetLong(nbRemoteTable);
	if (err == VE_OK)
	{
		if (nbRemoteTable >= 0 && nbRemoteTable <= kMaxTables)
		{
			try
			{
				tableRemoteStamps.resize(nbRemoteTable,0);
			}
			catch (...)
			{
				err = ThrowError(memfull, DBaction_SendingRemoteBase);
			}
			if (nbRemoteTable > 0)
				err = reqget->GetLongs(&tableRemoteStamps[0], &nbRemoteTable);
		}
		else
		{
			err = ThrowError(VE_DB4D_INDICE_OUT_OF_RANGE, DBaction_SendingRemoteBase);
		}
	}
	// Get the remote relations stamps
	if (err == VE_OK)
		err = reqget->GetLong(nbRemoteRel);
	if (err == VE_OK)
	{
		if (nbRemoteRel >= 0)
		{
			try
			{
				relRemoteStamps.resize(nbRemoteRel,0);
			}
			catch (...)
			{
				err = ThrowError(memfull, DBaction_SendingRemoteBase);
			}
			if (nbRemoteRel > 0)
				err = reqget->GetLongs(&relRemoteStamps[0], &nbRemoteRel);
		}
		else
		{
			err = ThrowError(VE_DB4D_INDICE_OUT_OF_RANGE, DBaction_SendingRemoteBase);
		}
	}

	sLONG xindexstamp;
	uLONG indexstamp;

	if (err == VE_OK)
	{
		err = reqget->GetLong(xindexstamp);
		indexstamp = xindexstamp;
	}

	if (err == VE_OK)
	{
		err = reqget->GetLong( schemasCatalogStamp);
	}

	if (err == VE_OK)
	{
		inRequest->InitReply(0);
		VStream *reqsend = inRequest->GetOutputStream();
		if (baseRemoteStamp == fStamp)
		{
			err = reqsend->PutLong(0);
		}
		else
		{
			err = reqsend->PutLong(fStamp);

			if (err == VE_OK)
				err = reqsend->PutLong(hbbloc.dialect);

			if (err == VE_OK)
				err = reqsend->PutByte(hbbloc.collatorOptions);

			
			// Send only tables which need to be updated in the remote database
			sLONG nbTable = max(  fich.GetCount(), nbRemoteTable);
			for (sLONG i = 1; (i <= nbTable) && (err == VE_OK); i++)
			{
				sLONG stamp = 0;
				Table *tt = RetainTable( i);
				if (tt != NULL)
					stamp = tt->GetStamp();
				
				bool sendIt = false;
				if (i > nbRemoteTable)
					sendIt = (tt != NULL);
				else
					sendIt = (tableRemoteStamps[i-1] != stamp);
					
				if (sendIt)
				{
					if (tt == NULL)
					{
						err = reqsend->PutLong(-i);
					}
					else
					{
						err = tt->SendToClient(reqsend);
					}
				}
				ReleaseRefCountable( &tt);
			}
			if (err == VE_OK)
				err = reqsend->PutLong(0);
			if (err == VE_OK)
			{
				// Send only relations which need to be updated in the remote database
				sLONG nbRel = max( fRelations.GetCount(), nbRemoteRel);
				for( sLONG relIndex = 0 ; (relIndex < nbRel) && (err == VE_OK) ; ++relIndex )
				{
					sLONG stamp = 0;
					Relation *rel = RetainRelation( relIndex);
					if (rel != NULL)
						stamp = rel->GetStamp();

					bool sendIt = false;
					if (relIndex > nbRemoteRel-1)
						sendIt = (rel != NULL);
					else
						sendIt = (relRemoteStamps[relIndex] != stamp);
					if (sendIt)
					{
						if (rel == NULL)
						{
							err = reqsend->PutLong( -1);	// put relation status
							if (err == VE_OK)
								err = reqsend->PutLong( relIndex);
						}
						else
						{
							err = reqsend->PutLong( 1);		// put relation status
							if (err == VE_OK)
								err = reqsend->PutLong( relIndex);
							if (err == VE_OK)
								err = rel->SendToClient( reqsend);
						}
					}
					ReleaseRefCountable( &rel);
				}
				if (err == VE_OK)
					err = reqsend->PutLong(0);
			}

			if (err == VE_OK)
			{
				// sc 08/01/2009 ACI0060249, ACI0056353
				if (indexstamp == fIndexesStamp || fIndexesStamp == 0)
				{
					err = reqsend->PutLong(0);
				}
				else
				{
					err = reqsend->PutLong(fIndexesStamp);

					if (err == VE_OK)
					{
						BaseTaskInfo* context = ConvertContext(inContext);

						if (context != nil)
						{
							vector<VUUIDBuffer> deletedIndexesID;
							context->StealDeletedIndexesID(deletedIndexesID);
							for (vector<VUUIDBuffer>::iterator cur = deletedIndexesID.begin(), end = deletedIndexesID.end(); cur != end && err == VE_OK; cur++)
							{
								VUUID xid(*cur);
								err = reqsend->PutByte('-');
								if (err == VE_OK)
									err = xid.WriteToStream(reqsend);
							}
						}

						if (err == VE_OK)
						{
							LockIndexes();
							for (IndexMap::iterator cur = fAllIndexes.begin(), end = fAllIndexes.end(); cur != end && err == VE_OK; cur++)
							{
								IndexInfo* ind = cur->second;
								if (ind != nil)
								{
									uLONG istamp = ind->GetStructureStamp();
									xbox_assert(istamp > 0 && istamp <= fIndexesStamp);
									if (xindexstamp == -1 || istamp > indexstamp)
									{
										err = reqsend->PutByte('+');
										if (err == VE_OK)
											err = reqsend->PutLong(ind->GetTyp());
										if (err == VE_OK)
											err = ind->PutInto(*reqsend, this, false);
									}
								}
							}
							UnLockIndexes();
						}
					}
					
					if (err == VE_OK)
						err = reqsend->PutByte('.');
				}
			}
		}

		if (err == VE_OK)
		{
			if (baseRemoteExtraStamp == fStampExtraProp)
			{
				err = reqsend->PutLong(0);
			}
			else
			{
				err = reqsend->PutLong( fStampExtraProp);
				if (err == VE_OK)
				{
					VValueBag *extraBag = fExtra.RetainExtraProperties( err);
					if (err != VE_OK)
						ReleaseRefCountable( &extraBag);
					if (extraBag == NULL)
						extraBag = new VValueBag();
					inRequest->PutValueBagReply( *extraBag);
					ReleaseRefCountable( &extraBag);
				}
			}
		}

		if (err == VE_OK)
		{
			if (structRemoteExtraStamp == fStructure->fStampExtraProp)
			{
				err = reqsend->PutLong(0);
			}
			else
			{
				err = reqsend->PutLong( fStructure->fStampExtraProp);
				if (err == VE_OK)
				{
					VValueBag *extraBag = fStructure->fExtra.RetainExtraProperties( err);
					if (err != VE_OK)
						ReleaseRefCountable( &extraBag);
					if (extraBag == NULL)
						extraBag = new VValueBag();
					inRequest->PutValueBagReply( *extraBag);
					ReleaseRefCountable( &extraBag);
				}
			}
		}

		if (err == VE_OK)
		{
			BuildFirstSchemaIfNeeded();		// sc 23/09/2008
			if (schemasCatalogStamp == fSchemasCatalogStamp)
			{
				inRequest->PutLongReply(0);
			}
			else
			{
				inRequest->PutLongReply( fSchemasCatalogStamp);
				inRequest->PutLongReply( (sLONG)fSchemasByID.size());
				for (SchemaCatalogByID::iterator cur = fSchemasByID.begin(), end = fSchemasByID.end() ; cur != end ; ++cur)
				{
					CDB4DSchema *schema = cur->second;
					if (schema != NULL)
					{
						inRequest->PutLongReply( schema->GetID());
						inRequest->PutStringReply( schema->GetName());
					}
				}
			}
			err = inRequest->GetLastErrorReply();
		}
	}
	else
	{
		inRequest->InitReply(-1);
		VStream *reqsend = inRequest->GetOutputStream();
		reqsend->PutLong8(err);
	}
	return err;
}


VError Base4D::BuildAndLoadFirstStageDBStruct( const VFile& inStructureFile, sLONG inParameters, FileAccess inAccess)
{
	VError err = VE_OK;

	if ((inParameters & DB4D_Open_As_XML_Definition) != 0)
	{
		VFolder* parent = inStructureFile.RetainParentFolder();
		VString structname;
		inStructureFile.GetNameWithoutExtension(structname);
		if ((inParameters & DB4D_Open_No_Respart) == 0)
		{
			VFile respart(*parent, structname+L".respart");
			if (respart.Exists())
				err = OpenData(respart, inParameters, true, false, inAccess);
			else
			{
				if (inAccess == FA_READ)
				{
					err = ThrowError(VE_DB4D_DATABASEISWRITEPROTECTED, noaction);
				}
				else
				{
					err = CreateData(respart, inParameters, VIntlMgr::GetDefaultMgr(), nil, true, inAccess);
				}
			}
		}
		else
			fWakandaMode =true;

	}
	else
		err = OpenData(inStructureFile, inParameters, true, false, inAccess);

	fTableDefTabAddr.Init(this,this,&hbbloc.TableDef_addrtabaddr,&hbbloc.TableDef_debuttrou,&hbbloc.nbTableDef, 
		nil, nil, 0);

	fRelationDefTabAddr.Init(this,this,&hbbloc.Relations_addrtabaddr,&hbbloc.Relations_debuttrou,&hbbloc.nbRelations, 
		nil, nil, 0);

	fTableDefInMem.Init(hbbloc.nbTableDef, false);
	fTableDefInMem.SetContientQuoi(t_tabledef);

	fRelationDefInMem.Init(hbbloc.nbRelations, false);
	fRelationDefInMem.SetContientQuoi(t_relationdef);

	fIndexesInStructTabAddr.Init(this,this,&hbbloc.IndexDefInStruct_addrtabaddr,&hbbloc.IndexDefInStruct_debuttrou,&hbbloc.nbIndexDefInStruct, 
		nil, nil, 0);
	fIndexesInStructInMem.Init(hbbloc.nbIndexDefInStruct, false);
	fIndexesInStructInMem.SetContientQuoi(t_indexdef);

	return err;
}


VError Base4D::LoadAllTableDefs()
{
	VError err = VE_OK;
	sLONG i;

	fNbTable = fStructure->hbbloc.nbTableDef;
	if (fich.SetAllocatedSize(fNbTable))
	{
		fich.SetCount(fNbTable, nil);
		for (i=1; i<=fNbTable; i++)
		{
			StructElemDef* e = LoadTableDef(i-1, err);
			if (err == VE_OK)
			{
				if (e != nil)
				{
					Table* fic = new TableRegular(this, i, true);
					fich[i] = fic;
					err = fic->load();
					e->libere();
					if (err != VE_OK)
						break;
					VUUID xid;
					fic->GetUUID(xid);
					AddObjectID(objInBase_Table, fic, xid);
				}
			}
			else
				break;
		}
	}
	else
	{
		fNbTable = 0;
		err = ThrowError(memfull, DBaction_OpeningStruct);	
	}

	return err;
}


VError Base4D::SetEntityCatalog(LocalEntityModelCatalog* newcat)
{
	VTaskLock lock(&fEntityCatalogMutex);
	QuickReleaseRefCountable(fEntityCatalog);
	fEntityCatalog = RetainRefCountable(newcat);
	return VE_OK;
}


VError Base4D::ReLoadEntityModels(const VFile* inFile)
{
	VTaskLock lock(&fEntityCatalogMutex);
	QuickReleaseRefCountable(fAutoEntityCatalog);
	QuickReleaseRefCountable(fEntityCatalog);
	fEntityCatalog = new LocalEntityModelCatalog(this);
	fAutoEntityCatalog = new LocalEntityModelCatalog(this);
	fAutoEntityCatalogBuilt = false;
	VError err = LoadEntityModels(inFile);
	if (err != VE_OK)
	{
		QuickReleaseRefCountable(fEntityCatalog);
		fEntityCatalog = new LocalEntityModelCatalog(this);
	}
	return err;
}


VError Base4D::SecondPassLoadEntityModels(ModelErrorReporter* errorReporter, bool dataWasAlreadyThere)
{
	VTaskLock lock(&fEntityCatalogMutex);
	bool oldparse = fIsParsingStruct;
	fIsParsingStruct = true;
	VError err = fEntityCatalog->SecondPassLoadEntityModels(errorReporter, dataWasAlreadyThere);
	fIsParsingStruct = oldparse;
	return err;
}


VError Base4D::LoadEntityModels(const VFile* inFile, ModelErrorReporter* errorReporter, const VString* inXmlContent, unordered_map_VString<VRefPtr<VFile> >* outIncludedFiles, CUAGDirectory* inDirectory, const VFile* inPermissions)
{
	VTaskLock lock(&fEntityCatalogMutex);
	VError err = VE_OK;

	if (fStructure != nil  && !fDontUseEntityCatalog)
	{
		if (inFile != nil)
		{
			if (inFile->Exists())
			{
				QuickReleaseRefCountable(fCatalogJSFile);
				VFilePath path;
				inFile->GetPath(path);
				path.SetExtension( "js" );
				fCatalogJSFile = new VFile(path);
				if ( ! fCatalogJSFile->Exists() )
				{
					VString alternateName;
					path.GetFileName( alternateName, false );
					alternateName += ".waModel.js";
					path.SetFileName( alternateName, true );
					VFile file( path);
					if ( file.Exists() )
					{
						QuickReleaseRefCountable(fCatalogJSFile);
						fCatalogJSFile = new VFile(path);
					}
				}


				err = fEntityCatalog->LoadEntityModels(*inFile, true, errorReporter, inXmlContent, outIncludedFiles);
				if (err == VE_OK && errorReporter == nil)
				{
					if ( inPermissions )
					{
						err = fEntityCatalog->LoadEntityPermissions(*inPermissions, inDirectory, true, errorReporter);
					}
					else
					{
						VFolder* parent = inFile->RetainParentFolder();
						VString name;
						inFile->GetNameWithoutExtension(name);
						name += ".waPerm";
						{
							VFile permfile(*parent, name);
							err = fEntityCatalog->LoadEntityPermissions(permfile, inDirectory, true, errorReporter);
						}

						QuickReleaseRefCountable(parent);
					}
				}
			}
		}
		else
		{
			VString entityfilename;
			if (fEntityFileExt.IsEmpty())
				entityfilename = L"EntityModels.xml";
			else
				entityfilename = fBaseName+fEntityFileExt;
			VFile entityFile2(*fStructFolder, entityfilename);
			if (entityFile2.Exists())
			{
				err = fEntityCatalog->LoadEntityModels(entityFile2, true, errorReporter);
			}
			if (err == VE_OK)
			{
				VFile permfile(*fStructFolder, L"EntitiesPermissions.xml");
				if (permfile.Exists())
				{
					err = fEntityCatalog->LoadEntityPermissions(permfile, inDirectory, true, errorReporter);
				}
			}
		}

	}

	return err;
}


VError Base4D::SaveEntityModels(bool alternateName)
{
	VTaskLock lock(&fEntityCatalogMutex);
	VError err = VE_OK;

	VString entityfilename;
	if (fEntityFileExt.IsEmpty())
		entityfilename = L"EntityModels.xml";
	else
		entityfilename = fBaseName+fEntityFileExt;

	if (alternateName)
		entityfilename = L"EntityModels2.xml";
	VFile entityFile(*fStructFolder, entityfilename);
	if (entityFile.Exists())
	{
		err = entityFile.Delete();
	}
	if (err == VE_OK)
	{
		err = fEntityCatalog->SaveEntityModels(entityFile, true);
	}

#if 0 && VERSIONDEBUG
	{
		VFile entityFile2(*fStructFolder, L"CopyEntityModels.json");
		if (entityFile2.Exists())
		{
			err = entityFile2.Delete();
		}
		if (err == VE_OK)
		{
			err = fEntityCatalog->SaveEntityModels(entityFile2, false);
		}
	}
#endif

	return err;
}


EntityModelCatalog* Base4D::GetGoodEntityCatalog(bool onlyRealOnes) const
{
	if (fEntityCatalog->CountEntityModels() == 0 && !onlyRealOnes)
		return fAutoEntityCatalog;
	else
		return fEntityCatalog;
}


EntityModel* Base4D::FindEntity(const VString& entityName, bool onlyRealOnes) const
{
	VTaskLock lock(&fEntityCatalogMutex);
	EntityModelCatalog* catalog = GetGoodEntityCatalog(onlyRealOnes);

	EntityModel* em = catalog->FindEntity(entityName);
	if (em == nil)
	{
		for (EntityModelCatalogCollection::const_iterator cur = fOutsideCatalogs.begin(), end = fOutsideCatalogs.end(); cur != end; ++cur)
		{
			em = (*cur)->FindEntity(entityName);
			if (em != nil)
				break;
		}
	}

	return em;
}



EntityModel* Base4D::RetainEntityModelByCollectionName(const VString& entityName) const
{
	VTaskLock lock(&fEntityCatalogMutex);
	EntityModelCatalog* catalog = GetGoodEntityCatalog(true);

	EntityModel* em = catalog->FindEntityByCollectionName(entityName);
	if (em == nil)
	{
		for (EntityModelCatalogCollection::const_iterator cur = fOutsideCatalogs.begin(), end = fOutsideCatalogs.end(); cur != end; ++cur)
		{
			em = (*cur)->FindEntityByCollectionName(entityName);
			if (em != nil)
				break;
		}
	}

	if (em != nil)
		em->Retain();
	return em;
}


EntityModel* Base4D::RetainEntity(const VString& entityName, bool onlyRealOnes) const
{
	VTaskLock lock(&fEntityCatalogMutex);

	EntityModel* em = FindEntity(entityName, onlyRealOnes);
	if (em == nil)
	{
#if AllowDefaultEMBasedOnTables
		if (entityName.GetLength() > 0 && entityName.GetUniChar(1) == kEntityTablePrefixChar)
		{
			VString tablename;
			entityName.GetSubString(2, entityName.GetLength()-1, tablename);
			Table* tt = FindAndRetainTableRef(tablename);
			if (tt != nil)
			{
				if (!tt->GetHideInRest())
					em = EntityModel::BuildLocalEntityModel(tt);
				tt->Release();
			}
		}
#endif
	}
	else
		em->Retain();
	return em;
}


VError Base4D::ReleaseOutsideCatalogs()
{
	VTaskLock lock(&fEntityCatalogMutex);
	VError err = VE_OK;
	for (EntityModelCatalogCollection::iterator cur = fOutsideCatalogs.begin(), end = fOutsideCatalogs.end(); cur != end; ++cur)
	{
		(*cur)->Release();
	}
	fOutsideCatalogs.clear();
	return err;
}


void Base4D::GetOutsideCatalogs(EntityModelCatalogCollection& outOutsideCats)
{
	VTaskLock lock(&fEntityCatalogMutex);
	outOutsideCats = fOutsideCatalogs;
	for (EntityModelCatalogCollection::iterator cur = outOutsideCats.begin(), end = outOutsideCats.end(); cur != end; ++cur)
	{
		(*cur)->Retain();
	}

}

VError Base4D::EvaluateOutsideCatalogModelScript(VJSObject& globalObject, BaseTaskInfo* context, void* ProjectRef, std::vector<XBOX::VFile*> &outModelJSFiles)
{
	VError err = VE_OK;
	EntityModelCatalogCollection cats;
	GetOutsideCatalogs(cats);

	for (EntityModelCatalogCollection::iterator cur = cats.begin(), end = cats.end(); cur != end; ++cur)
	{
		err = (*cur)->EvaluateModelScript(globalObject, context, ProjectRef, outModelJSFiles);
		(*cur)->Release();
	}

	return err;
}






VError Base4D::ReleaseWhatNeedsToBeReleasedOnCatalogs()
{
	VError err = VE_OK;
	EntityModelCatalogCollection catalogs;
	{
		VTaskLock lock(&fEntityCatalogMutex);
		for (EntityModelCatalogCollection::iterator cur = fOutsideCatalogs.begin(), end = fOutsideCatalogs.end(); cur != end; ++cur)
		{
			catalogs.push_back(RetainRefCountable(*cur));
		}
	}
	for (EntityModelCatalogCollection::iterator cur = catalogs.begin(), end = catalogs.end(); cur != end; ++cur)
	{
		EntityModelCatalog* cat = *cur;
		cat->ReleaseWhatNeedsToBeReleased();
		cat->Release();
	}
	return err;
}


VError Base4D::AddOutsideCatalog(const VValueBag* catref)
{
	VError err = VE_OK;
	VString path;
	VString username, password;
	VString ss;
	VJSONObject* params = new VJSONObject();
	VString catname;
	VString jsfilepath;

	catref->GetString("catalogName", catname);

	catref->GetString(khostname, path);
	if (path.IsEmpty())
		catref->GetString(khostname, path);
	params->SetPropertyAsString(khostname, path);

	if (catref->GetString(kLocalJSPath, jsfilepath))
		params->SetPropertyAsString(kLocalJSPath, jsfilepath);

	if (catref->GetString(d4::user, ss))
		params->SetPropertyAsString(kuser, ss);

	if (catref->GetString(d4::password, ss))
		params->SetPropertyAsString(kpassword, ss);

	if (catref->GetString(krestPrefix, ss))
		params->SetPropertyAsString(krestPrefix, ss);

	sLONG ll;
	if (catref->GetLong(ktimeout, ll))
		params->SetPropertyAsNumber(ktimeout, ll);

	bool ssl;
	if (catref->GetBool(kssl, ssl))
		params->SetPropertyAsBool(kssl, ssl);

	//EntityModelCatalog* cat = EntityModelCatalog::NewCatalog(kRemoteCatalogFactory, this, params, err);

	VFile* cacheFile = nil;
	if (!catname.IsEmpty())
	{
		VFolder* parent = GetStructFolder();
		cacheFile = new VFile(*parent, catname+kWaRemoteModelExt);
	}
	EntityModelCatalog* cat = VDBMgr::GetManager()->OpenRemoteCatalog(catname, params, this, cacheFile, true, err);
	if (cat != nil)
	{
		AddOutsideCatalog(catname, cat);
		cat->Release();
	}
	QuickReleaseRefCountable(cacheFile);
	QuickReleaseRefCountable(params);

	return err;
}


VError Base4D::AddOutsideSQLCatalog(const VValueBag* catref)
{
	VError err = VE_OK;
	VString hostname,user,database,password,jsfilepath;
	bool withssl = false;
	sLONG port = 3306;
	VString catname;

	catref->GetString("catalogName", catname);
	catref->GetString(khostname, hostname);
	catref->GetString(kLocalJSPath, jsfilepath);
	catref->GetString(kuser, user);
	catref->GetString(kdatabase, database);
	catref->GetString(kpassword, password);
	catref->GetLong(kport, port);
	catref->GetBool(kssl, withssl);

	VJSONObject* params = new VJSONObject();

	params->SetPropertyAsString(khostname, hostname);
	params->SetPropertyAsString(kLocalJSPath, jsfilepath);
	params->SetPropertyAsString(kuser, user);
	params->SetPropertyAsString(kdatabase, database);
	params->SetPropertyAsString(kpassword, password);
	params->SetPropertyAsBool(kssl, withssl);
	params->SetPropertyAsNumber(kport, port);
	params->SetPropertyAsBool(kSQL, true);

	VFile* cacheFile = nil;
	if (!catname.IsEmpty())
	{
		VFolder* parent = GetStructFolder();
		cacheFile = new VFile(*parent, catname+kWaRemoteModelExt);
	}
	EntityModelCatalog* cat = VDBMgr::GetManager()->OpenRemoteCatalog(catname, params, this, cacheFile, true, err);
	//EntityModelCatalog* cat = EntityModelCatalog::NewCatalog(kSQLCatalogFactory, this, params, err);
	if (cat != nil)
	{
		AddOutsideCatalog(catname, cat);
		cat->Release();
	}

	QuickReleaseRefCountable(cacheFile);
	QuickReleaseRefCountable(params);

	return err;
}


void Base4D::AddCatalogRef(const VString& catname, EntityModelCatalog* catalog)
{
	VTaskLock lock(&fEntityCatalogMutex);
	fCatalogs[catname] = catalog;
}


EntityModelCatalog* Base4D::GetCatalog(const VString& catname)
{
	EntityModelCatalog* result = nil;
	VTaskLock lock(&fEntityCatalogMutex);
	EntityModelCatalogsByName::iterator found = fCatalogs.find(catname);
	if (found != fCatalogs.end())
		result = found->second;
	return result;
}


VError Base4D::AddOutsideCatalog(const VString& catname, EntityModelCatalog* catalog)
{
	VTaskLock lock(&fEntityCatalogMutex);
	VError err = VE_OK;
	bool found = false;
	for (EntityModelCatalogCollection::iterator cur = fOutsideCatalogs.begin(), end = fOutsideCatalogs.end(); cur != end; ++cur)
	{
		if (*cur == catalog)
		{
			found = true;
			break;
		}
	}
	if (!found)
	{
		fOutsideCatalogs.push_back(RetainRefCountable(catalog));
		fCatalogs[catname] = catalog;
	}
	return err;
}

VError Base4D::RemoveOutsideCatalog(EntityModelCatalog* catalog)
{
	VTaskLock lock(&fEntityCatalogMutex);
	VError err = VE_OK;
	bool found = false;
	EntityModelCatalogCollection::iterator from;
	for (EntityModelCatalogCollection::iterator cur = fOutsideCatalogs.begin(), end = fOutsideCatalogs.end(); cur != end; ++cur)
	{
		if (*cur == catalog)
		{
			found = true;
			from = cur;
			break;
		}
	}
	if (found)
	{
		(*from)->Release();
		fOutsideCatalogs.erase(from);
	}
	return err;
}


VError Base4D::RollBackTransactionOnRemoteDataStores(BaseTaskInfo* context)
{
	VError err = VE_OK;
	fEntityCatalogMutex.Lock();
	EntityModelCatalogCollection catalogs = fOutsideCatalogs;
	fEntityCatalogMutex.Unlock();
	for (EntityModelCatalogCollection::iterator cur = catalogs.begin(), end = catalogs.end(); cur != end; ++cur)
	{
		(*cur)->RollBackTransaction(context);
	}

	return err;
}


VError Base4D::CommitTransactionOnRemoteDataStores(BaseTaskInfo* context)
{
	VError err = VE_OK;
	fEntityCatalogMutex.Lock();
	EntityModelCatalogCollection catalogs = fOutsideCatalogs;
	fEntityCatalogMutex.Unlock();
	for (EntityModelCatalogCollection::iterator cur = catalogs.begin(), end = catalogs.end(); cur != end; ++cur)
	{
		(*cur)->CommitTransaction(context);
	}
	return err;
}


VError Base4D::StartTransactionOnRemoteDataStores(BaseTaskInfo* context)
{
	VError err = VE_OK;
	fEntityCatalogMutex.Lock();
	EntityModelCatalogCollection catalogs = fOutsideCatalogs;
	fEntityCatalogMutex.Unlock();
	for (EntityModelCatalogCollection::iterator cur = catalogs.begin(), end = catalogs.end(); cur != end; ++cur)
	{
		(*cur)->StartTransaction(context);
	}
	return err;
}





/*
sLONG Base4D::CountEntityModels(bool onlyRealOnes) const
{
	VTaskLock lock(&fEntityCatalogMutex);
	EntityModelCatalog* catalog = GetGoodEntityCatalog(onlyRealOnes);
	return catalog->CountEntityModels();
}
*/


VError Base4D::GetAllEntityModels(vector<CDB4DEntityModel*>& outList, CDB4DBaseContext* context, bool onlyRealOnes) const
{
	VTaskLock lock(&fEntityCatalogMutex);
	EntityModelCatalog* catalog = GetGoodEntityCatalog(onlyRealOnes);
	VError err = catalog->GetAllEntityModels(outList);
	if (err == VE_OK)
	{
		for (EntityModelCatalogCollection::const_iterator cur = fOutsideCatalogs.begin(), end = fOutsideCatalogs.end(); cur != end && err == VE_OK; ++cur)
		{
			vector<CDB4DEntityModel*> templist;
			err = (*cur)->GetAllEntityModels(templist);
			outList.insert(outList.end(), templist.begin(), templist.end());
		}
	}
	return err;
}

VError Base4D::RetainAllEntityModels(vector<CDB4DEntityModel*>& outList, CDB4DBaseContext* context, bool withTables, bool onlyRealOnes) const
{
	VTaskLock lock(&fEntityCatalogMutex);
	VError err = VE_OK;
	outList.clear();

#if AllowDefaultEMBasedOnTables
	if (withTables)
	{
		for (TableArray::ConstIterator cur = fich.First(), end = fich.End(); cur != end; cur++)
		{
			Table* tt = (Table*)*cur;
			if (tt != nil)
			{
				if (!tt->GetHideInRest())
					EntityModel* em = EntityModel::BuildLocalEntityModel(tt);
				outList.push_back(em);
			}
		}
	}
#endif

	vector<CDB4DEntityModel*> templist;
	err = GetAllEntityModels(templist, context, onlyRealOnes);
	if (err == VE_OK)
	{
		for (vector<CDB4DEntityModel*>::iterator cur = templist.begin(), end = templist.end(); cur != end; cur++)
		{
			CDB4DEntityModel* em = *cur;
			em->Retain();
			outList.push_back(em);
		}
	}

	return err;
}


VError Base4D::RetainAllEntityModels(vector<VRefPtr<EntityModel> >& outList, bool onlyRealOnes) const
{
	VTaskLock lock(&fEntityCatalogMutex);
	VError err = VE_OK;
	outList.clear();
	EntityModelCatalog* catalog = GetGoodEntityCatalog(onlyRealOnes);

	err = catalog->RetainAllEntityModels(outList);
	if (err == VE_OK)
	{
		for (EntityModelCatalogCollection::const_iterator cur = fOutsideCatalogs.begin(), end = fOutsideCatalogs.end(); cur != end && err == VE_OK; ++cur)
		{
			vector<EntityModel*> templist;
			err = (*cur)->GetAllEntityModels(templist);
			outList.reserve(outList.size()+templist.size());
			for (vector<EntityModel*>::iterator curm = templist.begin(), endm = templist.end(); curm != endm; ++curm)
			{
				outList.push_back(*curm);
			}
		}
	}

	return err;
}


SynchroBaseHelper* Base4D::GetSyncHelper(bool BuildIfMissing)
{
	VObjLock lock(this);
	if ((fSyncHelper == nil) && BuildIfMissing)
	{
		fSyncHelper = new SynchroBaseHelper(this);
	}

	return fSyncHelper;
}



VError Base4D::OpenStructure( const VFile& inStructureFile, sLONG inParameters, FileAccess inAccess, VString* EntityFileExt,CUAGDirectory* inUAGDirectory, 
							 const VString* inXmlContent, unordered_map_VString<VRefPtr<VFile> >* outIncludedFiles, const VFile* inPermissionsFile)
{
	// the structure file is actually a database in a single file
	
	/*
	UniChar a[2], b[2];
	a[0] = 0xd6f;
	a[1] = 0;
	b[0] = 0xd6f;
	b[1] = 0x202e;

	CompareResult ResultWithDiac = VIntlMgr::GetDefaultMgr()->CompareString(&a[0], 1, &b[0], 2, true);
	CompareResult ResultWithoutDiac = VIntlMgr::GetDefaultMgr()->CompareString(&a[0], 1, &b[0], 2, false);
*/

	VError err = VE_OK;

	SetNotifyState(true);

	inStructureFile.GetName( fBaseName);
	RemoveExtension(fBaseName);
	if (EntityFileExt == nil)
		fEntityFileExt.Clear();
	else
		fEntityFileExt = *EntityFileExt;

	Base4D* firststage = new Base4D(fFlusher, inAccess != FA_READ);

	if (firststage == nil)
	{
		err = ThrowError(memfull, DBaction_OpeningStruct);
	}
	else
	{
		firststage->SetRiaServerProjectRef(fRiaServerProjectRef);
		firststage->SetName(fBaseName,NULL, false);
		err = firststage->BuildAndLoadFirstStageDBStruct(inStructureFile, inParameters, inAccess);
		fWakandaMode = firststage->fWakandaMode;

		if (err == VE_OK)
		{
			QuickReleaseRefCountable(fStructFolder);
			fStructFolder = inStructureFile.RetainParentFolder();
			fStructure = firststage;
			fID = fStructure->fID;
			sLONG i;

			fBaseX = new VDB4DBase(VDBMgr::GetManager(), this, true);

			{
				if (inUAGDirectory != nil)
				{
					fUAGDirectory = RetainRefCountable(inUAGDirectory);
				}
				else if ((inParameters & DB4D_Open_With_Own_UsersAndGroups) != 0)
				{
					VFile xmldata(*fStructFolder, fBaseName+L".RIADirectory");
					CUAGManager* uag = (CUAGManager*) VComponentManager::RetainComponent((CType)CUAGManager::Component_Type);
					if (uag != nil)
					{
						fUAGDirectory = uag->RetainDirectory(xmldata, inAccess, fStructFolder, &fBaseName, &err);
						QuickReleaseRefCountable(uag);
					}
				}
			}

			if ((inParameters & DB4D_Open_As_XML_Definition) != 0)
			{
				VFileDesc* ff = nil;
				//if (inStructureFile.Exists())
				err = inStructureFile.Open(inAccess == FA_READ ?  FA_READ : FA_MAX, &ff);
				if (ff != nil)
				{
					if (ff->GetMode() == FA_READ)
						fWriteProtected = true;
					delete ff;
				}
				if (inAccess == FA_READ)
					fWriteProtected = true;
				fStoreAsXML = true;
				fStructXMLFile = new VFile(inStructureFile);

				fEntityCatalog->ClearTouchXML();
				fEntityCatalog->ClearErrors();
				if (err == VE_OK)
				{
					fIsParsingStruct = true;
					ModelErrorReporter* errReporter = nil;
					if ((inParameters & DB4D_Open_StructOnly) != 0)
					{
						errReporter = fEntityCatalog->CreateErrorReporter();
					}
					err = LoadEntityModels(fStructXMLFile, errReporter, inXmlContent, outIncludedFiles, inUAGDirectory, inPermissionsFile);
					fIsParsingStruct = false;

#if debuglr
					if (errReporter != nil)
					{
						const VErrorStack& errs = errReporter->GetErrors();
						for (VErrorStack::const_iterator cur = errs.begin(), end = errs.end(); cur != end; ++cur)
						{
							VErrorBase* error = *cur;
							VString s;
							error->GetErrorDescription(s);
							DebugMsg(s);
							DebugMsg("\n");
						}
					}
#endif

					/*
					if (fEntityCatalog->IsXMLTouched() && !fEntityCatalog->someErrors())
						TouchXML();
						*/
					fEntityCatalog->ClearTouchXML();
				}

				if (err != VE_OK)
					err = ThrowError(VE_DB4D_CANNOTOPENSTRUCT, DBaction_OpeningStruct);	

			}
			else
			{
				err = LoadAllTableDefs();

				if (err == VE_OK)
					err = LoadSchemas();

				if (err == VE_OK)
				{

					if (fStructure->fNeedToLoadOldRelations)
						err = oldLoadRelations();
					else
						err = LoadRelations();

				}
			}

			if (err == VE_OK && !fStoreAsXML)
			{
				err = LoadEntityModels(nil, nil, nil, nil, inUAGDirectory, inPermissionsFile);
				if (err == VE_OK && ((inParameters & DB4D_Open_BuildAutoEm) != 0))
				{
					BuildAutoModelCatalog(nil);
					if (fAutoEntityCatalog != nil && ((fEntityCatalog != nil && fEntityCatalog->CountEntityModels() == 0) || fEntityCatalog == nil))
					{
						LocalEntityModelCatalog* x = fEntityCatalog;
						fEntityCatalog = fAutoEntityCatalog;
						fAutoEntityCatalog = x;
					}
				}

			}

			if (err == VE_OK)
			{
				fIsStructureOpened = true;
				inStructureFile.GetName( fBaseName);
				RemoveExtension(fBaseName);
				fDefaultLocation = inStructureFile.GetPath();

				VDBMgr::GetManager()->RegisterBase(this);
			}
			


			if ((fBaseX != nil) && (err != VE_OK))
			{
				fBaseX->Release();
				fBaseX = nil;
			}

		}
		else
			err = ThrowError(VE_DB4D_CANNOTOPENSTRUCT, DBaction_OpeningStruct);	

		firststage->libere();
	}

	SetNotifyState(false);

	return err;
}


VError Base4D::BuildAutoModelCatalog(BaseTaskInfo* context)
{
	VError err = VE_OK;
	VTaskLock lock(&fEntityCatalogMutex);
	if (!fAutoEntityCatalogBuilt)
	{
		for (TableArray::Iterator cur = fich.First(), end = fich.End(); cur != end && err == VE_OK; cur++)
		{
			Table* tt = *cur;
			if (tt != nil)
			{
				if (!tt->GetHideInRest())
				{
					EntityModel* em = EntityModel::BuildLocalEntityModel(tt, fAutoEntityCatalog);
					QuickReleaseRefCountable(em);
				}
			}
		}
		if (err == VE_OK)
			err = fAutoEntityCatalog->BuildLocalEntityModelRelations();
		if (err == VE_OK)
			err = fAutoEntityCatalog->ResolveRelatedEntities(context);
		if (err == VE_OK)
		{
			IDB4D_ApplicationIntf *applicationIntf = VDBMgr::GetManager()->GetApplicationInterface();
			if (applicationIntf != nil)
			{
				std::vector<Public4DMethodInfo> methods;
				err = applicationIntf->Get4DMethods(methods);
				if (err == VE_OK)
				{
					for (std::vector<Public4DMethodInfo>::iterator cur = methods.begin(), end = methods.end(); cur != end; ++cur)
					{
						sLONG applyTo = cur->applyTo;
						VString name = cur->name;
						VString tablename = cur->tablename;
						if (applyTo != Apply4DMethodTo_None)
						{
							EntityModel* model = fAutoEntityCatalog->FindEntity(tablename);
							if (model != nil)
							{
								EntityMethodKind kind;
								switch (applyTo)
								{
									case Apply4DMethodTo_Entity:
										kind = emeth_rec;
										break;
									case Apply4DMethodTo_Collection:
										kind = emeth_sel;
										break;
									default:
										kind = emeth_static;
										break;
								}
								EntityMethod* meth = new EntityMethod(model, name, kind);
								model->AddMethod(meth);
								meth->Release();

							}
						}
					}
				}
			}
		}
		//if (err == VE_OK)
			fAutoEntityCatalogBuilt = true;
	}
	return err;
}

void Base4D::InvalidateAutoCatalog()
{
	VTaskLock lock(&fEntityCatalogMutex);
	QuickReleaseRefCountable(fAutoEntityCatalog);
	fAutoEntityCatalog = new LocalEntityModelCatalog(this);
	fAutoEntityCatalogBuilt = false;
}


VError Base4D::BuildAndCreateFirstStageDBStruct( const VFile& inStructureFile, sLONG inParameters, VIntlMgr* inIntlMgr, FileAccess inAccess, const VUUID* inChosenID)
{
	VError err = VE_OK;

	if ((inParameters & DB4D_Create_As_XML_Definition) != 0)
	{
		VFolder* parent = inStructureFile.RetainParentFolder();
		VString structname;
		inStructureFile.GetNameWithoutExtension(structname);
		CopyRefCountable(&fIntlMgr, inIntlMgr);
		if ((inParameters & DB4D_Open_No_Respart) == 0)
		{
			VFile respart(*parent, structname+L".respart");
			err = CreateData(respart, inParameters, inIntlMgr, nil, true, inAccess);
		}
		else
			fWakandaMode =true;
	}
	else
		err = CreateData(inStructureFile, inParameters, inIntlMgr, NULL, true, inAccess, inChosenID);

	fTableDefTabAddr.Init(this,this,&hbbloc.TableDef_addrtabaddr,&hbbloc.TableDef_debuttrou,&hbbloc.nbTableDef, 
		nil, nil, 0);

	fRelationDefTabAddr.Init(this,this,&hbbloc.Relations_addrtabaddr,&hbbloc.Relations_debuttrou,&hbbloc.nbRelations, 
		nil, nil, 0);

	fTableDefInMem.Init(hbbloc.nbTableDef, false);
	fTableDefInMem.SetContientQuoi(t_tabledef);

	fRelationDefInMem.Init(hbbloc.nbRelations, false);
	fRelationDefInMem.SetContientQuoi(t_relationdef);

	fIndexesInStructTabAddr.Init(this,this,&hbbloc.IndexDefInStruct_addrtabaddr,&hbbloc.IndexDefInStruct_debuttrou,&hbbloc.nbIndexDefInStruct, 
		nil, nil, 0);
	fIndexesInStructInMem.Init(hbbloc.nbIndexDefInStruct, false);
	fIndexesInStructInMem.SetContientQuoi(t_indexdef);

	return err;
}



VError Base4D::CreateStructure( const VFile& inStructureFile, sLONG inParameters, VIntlMgr* inIntlMgr, FileAccess InAccess, const VUUID* inChosenID, VString* EntityFileExt, CUAGDirectory* inUAGDirectory, const VFile* inPermissionsFile)
{
	if (EntityFileExt == nil)
		fEntityFileExt.Clear();
	else
		fEntityFileExt = *EntityFileExt;

	Base4D* firststage = new Base4D(fFlusher, InAccess != FA_READ);
	VError err = VE_OK;

	if (firststage == nil )
	{
		err = ThrowError(memfull, DBaction_OpeningStruct);
	}
	else
	{
		err = firststage->BuildAndCreateFirstStageDBStruct(inStructureFile, inParameters, inIntlMgr, InAccess, inChosenID);
		fWakandaMode = firststage->fWakandaMode;
		if (err == VE_OK)
		{
			fStructure = firststage;
			CopyRefCountable(&fIntlMgr, fStructure->fIntlMgr);
			fIsStructureOpened = true;

			if ((inParameters & DB4D_Create_As_XML_Definition) != 0)
			{
				fStoreAsXML = true;
				fStructXMLFile = new VFile(inStructureFile);

				if ((inParameters & DB4D_Create_Empty_Catalog) != 0)
				{
					// don't build a catalog based on XML file
				}
				else
				{
					VFileDesc* ff = nil;
					//if (inStructureFile.Exists())
					err = inStructureFile.Open(FA_MAX, &ff);
					if (ff != nil)
					{
						if (ff->GetMode() == FA_READ)
							fWriteProtected = true;
						delete ff;
					}
					
					fEntityCatalog->ClearTouchXML();
					fEntityCatalog->ClearErrors();
					if (err == VE_OK)
					{
						fIsParsingStruct = true;
						err = LoadEntityModels(fStructXMLFile, nil, nil, nil, nil, inPermissionsFile);
						fIsParsingStruct = false;

						if (fEntityCatalog->IsXMLTouched() && !fEntityCatalog->someErrors())
							TouchXML();
						fEntityCatalog->ClearTouchXML();
					}
				}

				if (err != VE_OK)
					err = ThrowError(VE_DB4D_CANNOTOPENSTRUCT, DBaction_OpeningStruct);	
			}

			QuickReleaseRefCountable(fStructFolder);
			fStructFolder = inStructureFile.RetainParentFolder();
			// preque tout est fait dans le constructeur et plus specialement dans xinit
			inStructureFile.GetName( fBaseName);
			RemoveExtension(fBaseName);
			fDefaultLocation = inStructureFile.GetPath();

			if (inChosenID != nil)
				fStructure->fID = *inChosenID;
			else
				fStructure->fID.Regenerate();
			fID = fStructure->fID;
			fID.ToBuffer(fStructure->hbbloc.GetID());
			fStructure->setmodif(true, fStructure, nil);

			fBaseX = new VDB4DBase(VDBMgr::GetManager(), this, true);
			VDBMgr::GetManager()->RegisterBase(this);
			//RegisterForLang();  // doit etre maintenant appele par celui qui ouvre ou cree la base
			if (err == VE_OK)
			{
				if (inUAGDirectory != nil)
				{
					fUAGDirectory = RetainRefCountable(inUAGDirectory);
				}
				else if ((inParameters & DB4D_Open_With_Own_UsersAndGroups) != 0)
				{
					StErrorContextInstaller errs(false);
					VError err2;
					VFile xmldata(*fStructFolder, fBaseName+L".RIADirectory");
					CUAGManager* uag = (CUAGManager*) VComponentManager::RetainComponent((CType)CUAGManager::Component_Type);
					if (uag != nil)
					{
						fUAGDirectory = uag->RetainDirectory(xmldata, InAccess, fStructFolder, &fBaseName, &err2);
						QuickReleaseRefCountable(uag);
					}
				}
			}
		}
		firststage->libere();
	}

	return err;
}


Boolean Base4D::IsDataOpened() const
{
	VObjLock(this);
	return fIsDataOpened;
}


Boolean Base4D::IsStructureOpened() const
{
	VObjLock(this);
	return fIsStructureOpened;
}


Boolean Base4D::AreIndexesDelayed() const
{
	VTaskLock lock(&fDelayIndexMutex);
	return fIndexDelayed;
}

Boolean Base4D::CheckIfIndexesAreDelayed() const
{
	Boolean result = true;
	if (fDelayIndexMutex.TryToLock())
	{
		result = fIndexDelayed;
		fDelayIndexMutex.Unlock();
	}

	return result;
}


void Base4D::DelayAllIndexes()
{
	VTaskLock lock(&fDelayIndexMutex);
	if (!fIndexDelayed)
	{
		for (TableArray::Iterator cur = fich.First(), end = fich.End(); cur != end;  cur++)
		{
			if (*cur != nil)
				(*cur)->DelayIndexes(true);
		}
		fIndexDelayed = true;
	}
}


void Base4D::AwakeAllIndexes(VDB4DProgressIndicator* inProgress, Boolean WaitForCompletion, vector<IndexInfo*>& outIndexList)
{
	outIndexList.clear();
	vector<IndexInfo*> FullList;
	{
		VTaskLock lock(&fDelayIndexMutex);
		for (TableArray::Iterator cur = fich.First(), end = fich.End(); cur != end; cur++)
		{
			if (*cur != nil)
			{
				vector<IndexInfo*> sublist;
				(*cur)->AwakeIndexes(inProgress, &sublist);
				copy(sublist.begin(), sublist.end(), back_insert_iterator<vector<IndexInfo*> >(FullList));
			}
		}
		fIndexDelayed = false;
	}

	if (WaitForCompletion)
	{
		VTask *currentTask = VTaskMgr::Get()->GetCurrentTask();
		while (StillIndexingOn(this))
		{
			sLONG delay = 100;
			currentTask->ExecuteMessages();	// for progress signaling
			currentTask->Sleep(delay);
//			delay+= 100;
//			if (delay>2000)
//				delay = 2000;
		}
		for (vector<IndexInfo*>::iterator cur = FullList.begin(), end = FullList.end(); cur != end; cur++)
		{
			IndexInfo* ind = *cur;
			if (ind->GetBuildError() == VE_OK)
			{
				ind->Release();
			}
			else
			{
				outIndexList.push_back(ind);
			}
		}
	}
	else
	{
		for (vector<IndexInfo*>::iterator cur = FullList.begin(), end = FullList.end(); cur != end; cur++)
			(*cur)->Release();
	}
}


/*
void Base4D::SetSaveIndexes()
{
	fNeedsToSaveIndexes = true;
	setmodif( true, this, nil);
}
*/


VError Base4D::KillDataFile(sLONG TableNumber, VDB4DProgressIndicator* InProgress, VSyncEvent* event, bool mustFullyDelete, sLONG StructTableNumber) 
{
	VError err = VE_OK;
	if (testAssert(!fIsRemote))
	{
		if (OkToUpdate(err))
		{
			DataTableRegular* df = nil;
			{
				VObjLock lock(this);
				df = fDataTables[TableNumber-1];
				df->RemoveFromCache();
				fDataTables[TableNumber-1] = nil;
			}

			if (df != nil)
			{
				DropTableDefInData(df->GetTableDefNumInData());
				err = df->DeleteAll(InProgress, event, mustFullyDelete, StructTableNumber);
				if (err == VE_OK)
				{
					DataAddr4D ou = df->getaddr();
					df->setmodif(false, this, nil);
					df->Release();
					if (ou > 0)
					{
						libereplace(ou, sizeof(DataTableDISK)+kSizeDataBaseObjectHeader, nil, df);
					}
					fTableDataTableTabAddr.liberetrou(TableNumber-1, nil);
				}
				else
				{
					VObjLock lock(this);
					fDataTables[TableNumber-1] = df;
					df->PutInCache();
				}
			}

			SaveDataTablesToStructTablesMatching();
			ClearUpdating();
		}
	}
	return err;
}


VFile* Base4D::RetainDataTablesToStructTablesMatchingFile()
{
	VFile* result = nil;

	if (fStructure != nil && fSegments.GetCount() > 0)
	{
		StErrorContextInstaller errs(false);
		VFile* inFile = fSegments[0]->GetFile();
		VString s;
		inFile->GetNameWithoutExtension(s);
		if (VDBMgr::GetManager()->IsRunningWakanda())
			s += kDataTableDefExt;
		else
			s += kDataMatchExt;
		VFolder* vf = inFile->RetainParentFolder();
		if (vf != nil)
		{
			result = new VFile(*vf, s);
			vf->Release();
		}
	}


	return result;
}

bool Base4D::ExistDataTablesToStructTablesMatching()
{
	bool result = false;
	VFile* matchFile = RetainDataTablesToStructTablesMatchingFile();
	if (matchFile != nil)
	{
		result = matchFile->Exists();
		matchFile->Release();
	}

	return result;;
}

VError Base4D::SaveDataTablesToStructTablesMatching()
{
	VError err = VE_OK;
	VFile* matchFile = RetainDataTablesToStructTablesMatchingFile();
	if (matchFile != nil)
	{
		if (VDBMgr::GetManager()->IsRunningWakanda())
		{
			if (fIsDataOpened)
			{
				VJSONArray* arr = new VJSONArray();
				StErrorContextInstaller errs(false);
				occupe();

				sLONG i = 1;
				for (DataTableVector::iterator cur = fDataTables.begin(), end = fDataTables.end(); cur != end; cur++, i++)
				{
					DataTableRegular* df = *cur;
					if (df != nil)
					{
						Table* crit = df->GetTable();
						if (crit != nil)
						{
							VJSONObject* obj = new VJSONObject();
							obj->SetPropertyAsNumber("dataTableNum", i);
							VString tname;
							crit->GetName(tname);
							obj->SetPropertyAsString("tableName", tname);
							VUUID xid = df->GetUUID();
							VString sid;
							xid.GetString(sid);
							obj->SetPropertyAsString("dataTableID", sid);
							VJSONArray* fieldarr = new VJSONArray();
							sLONG nbfields = crit->GetNbCrit();
							for (sLONG j = 1; j <= nbfields; ++j)
							{
								Field* cri = crit->RetainField(j);
								if (cri != nil)
								{
									VJSONObject* fieldobj = new VJSONObject();
									VString fieldname;
									cri->GetName(fieldname);
									fieldobj->SetPropertyAsString("fieldName", fieldname);
									fieldobj->SetPropertyAsNumber("posInRecord", cri->GetPosInRec());
									cri->Release();
									fieldarr->Push(VJSONValue(fieldobj));
									QuickReleaseRefCountable(fieldobj);
								}
							}
							obj->SetProperty("fields", VJSONValue(fieldarr));
							QuickReleaseRefCountable(fieldarr);
							arr->Push(VJSONValue(obj));
							QuickReleaseRefCountable(obj);
						}
					}
				}

				libere();
				err = matchFile->Delete();
				VJSONValue jsonval(arr);
				QuickReleaseRefCountable(arr);
				jsonval.SaveToFile(matchFile);
			}
		}
		else
		{
			StErrorContextInstaller errs(false);
			VValueBag bag;
			occupe();
			
			sLONG i = 1;
			for (DataTableVector::iterator cur = fDataTables.begin(), end = fDataTables.end(); cur != end; cur++, i++)
			{
				DataTableRegular* df = *cur;
				if (df != nil)
				{
					BagElement elem(bag, L"DataTable");
					elem->SetLong(L"num", i);
					VUUID xid;
					df->GetAssociatedTableUUID(xid);
					elem->SetVUUID(L"TableDefID", xid);
				}
			}

			libere();

			err = matchFile->Delete();
			err = SaveBagToXML(bag, L"DataTableMatching",*matchFile, true);
		}
		matchFile->Release();
	}
	return err;
}



void Base4D::KillIndex(const VUUID& indexid, VDB4DProgressIndicator* InProgress)  // appelee en asynchrone par la tache IndexBuilder
{
	IndexInfo *Ind;
	
#if trackIndex
	trackDebugMsg("before killIndex \n");
#endif
	VError err = VE_OK;
	//if (testAssert(!fIsRemote))
	{
		if (OkToUpdate(err))
		{
			LockIndexes();
			IndexMap::iterator found = fAllIndexes.find(indexid);
			if (found == fAllIndexes.end())
				Ind = nil;
			else
			{
				Ind = found->second;
				Ind->ClearFromSystemTableIndexCols();
				fDataTableOfIndexes->DelIndexRef(Ind);
				fAllIndexes.erase(found);
			}
			UnLockIndexes();

			if (Ind != nil)
			{
				BaseTaskInfo::AddDeletedIndexIDInAllContexts(indexid.GetBuffer());
				if (!Ind->IsRemote())
				{
					xbox_assert(!fIsRemote);
					DeleteIndexDef(Ind->GetPlace());

					if (Ind->GetPlaceInStruct() != -1 && !fStoreAsXML)
						DeleteIndexDefInStruct(Ind->GetPlaceInStruct());
					Ind->TouchDependencies();
					TouchIndexes();
				}
				Ind->Open(index_write);
				//Ind->SetInvalidOnDisk();
				Ind->DeleteFromDependencies();
				Ind->Close();
				Ind->SetInvalid();
				
				OccupableStack* curstack = gOccPool.GetStackForCurrentThread();

				if (Ind->GetHeader() != nil)
					Ind->GetHeader()->LibereEspaceDisk(curstack, InProgress);
				
				//Ind->GetHeader()->LibereEspaceMem();
				// plus la peine car appele dans le delete de l'indexinfo
				
				Ind->Release();
			}
			ClearUpdating();
		}
	}
#if trackIndex
	trackDebugMsg("after killIndex \n");
#endif
}

void Base4D::BuildIndex(IndexInfo *Ind, VDB4DProgressIndicator* InProgress) // appelee en asynchrone par la tache IndexBuilder
{
	sLONG i,nb, place = 0;
	VError err = VE_OK;

	if (testAssert(!fIsRemote))
	{
		if (OkToUpdate(err))
		{
			err = Ind->ValidateIndexInfo();
			if (err == VE_OK)
			{
				Ind->GetTargetTable()->WaitToBuildIndex();

				Ind->LockValidity();
				Ind->SetDB(this);
				Ind->EnCreation(-1);
		//		Ind->SetMayInsertOrDelete(false);
				Ind->UnLockValidity();

				//BaseTaskInfo* context = new BaseTaskInfo(this, nil);
				//Ind->LockTargetTables(context);
				
				LockIndexes();

				IndexInfo* ind2 = Ind->GetDB()->FindAndRetainIndex(Ind, true);
				if (ind2 != nil && ind2 != Ind)
				{
					err = 1;
					sLONG n = Ind->GetPlaceInStruct();
					if (n != -1 && Ind->GetDB()->GetStructure() != nil && !fStoreAsXML)
					{
						Ind->GetDB()->DeleteIndexDefInStruct(n);
					}
				}
				else
				{
					try
					{
						//Ind->AddToCache(ObjCache::HeaderAccess);
						Ind->PutInCache();
						xbox_assert(!IsDuplicateIndex(Ind));						
						fAllIndexes.insert(make_pair(Ind->GetID(), Ind));
						TouchIndexes();
						Ind->SetStructureStamp( GetIndexesStamp());
						Ind->SetOKtoSave();
						Ind->Retain();
					}
					catch (...)
					{
						err = ThrowError(memfull, DBaction_BuildingIndex);
					}
				}

				if (ind2 != nil)
					ind2->Release();

				Ind->CheckIfIsDelayed();

				if (err == VE_OK)
					Ind->CalculDependence();
				
				UnLockIndexes();

				if (err == VE_OK)
				{
					Ind->SetInvalidOnDisk();
					Ind->Save();
					Ind->SaveInStruct();
					
					fDataTableOfIndexes->AddIndexRef(Ind);
					Ind->AddToSystemTableIndexCols();

					err = Ind->GenereFullIndex(InProgress, kNbKeyParPageIndex, kNbKeyParPageIndex);
					Ind->SetValidOnDisk();

					Ind->CheckIfIsDelayed();
				}

				//Ind->UnLockTargetTables(context);
				//context->Release();

				Ind->GetTargetTable()->FinishedBuildingIndex();

				if (err != VE_OK)
				{
					Ind->SetBuildError(err);
					Ind->Retain();
					KillIndex(Ind->GetID(), nil);
					SetIndexEncours(nil);
					Ind->Release();
					if (err == 1)
						err = VE_OK;
				}
				else
					SetIndexEncours(nil);
			}
			else 
				SetIndexEncours(nil);
			ClearUpdating();
		}
		else
			SetIndexEncours(nil);
	}
}


void Base4D::ReBuildIndex(IndexInfo *Ind, VDB4DProgressIndicator* InProgress) // appelee en asynchrone par la tache IndexBuilder
{
	VError err = VE_OK;

	if (testAssert(!fIsRemote))
	{
		StartDataModif(nil);
		if (OkToUpdate(err))
		{

#if trackIndex
			trackDebugMsg("start rebuild index, before removing \n");
#endif
			Ind->SetDB(this);
			Ind->SetInvalid();
			//Ind->DeleteFromDependencies();

			OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
			err = Ind->GetHeader()->LibereEspaceDisk(curstack, InProgress);
			Ind->GetHeader()->LibereEspaceMem(curstack);

#if trackIndex
			trackDebugMsg("start rebuild index, after removing \n");
#endif
			//BaseTaskInfo* context = new BaseTaskInfo(this, nil);
			//Ind->LockTargetTables(context);

			if (err == VE_OK)
			{
				Ind->GetTargetTable()->WaitToBuildIndex();

				Ind->SetQuickBuilding();
				Ind->LockValidity();
				Ind->EnCreation(-1);
				Ind->UnLockValidity();

				err = Ind->GenereFullIndex(InProgress, kNbKeyParPageIndex, kNbKeyParPageIndex);

				if (err != VE_OK)
				{
					Ind->GetTargetTable()->FinishedBuildingIndex();

					Ind->Retain();
					Ind->SetBuildError(err);
					Ind->DeleteFromDependencies();
					KillIndex(Ind->GetID(), nil);
					SetIndexEncours(nil);
					Ind->Release();
				}
				else
				{
					Ind->Open(index_write);
					Ind->SetValidOnDisk();
					Ind->CheckIfIsDelayed();
					//Ind->CalculDependence();
					Ind->Save();
					Ind->SaveInStruct();
					Ind->Close();

					Ind->SetValid();

					Ind->GetTargetTable()->FinishedBuildingIndex();

				}
			}
			else
				SetIndexEncours(nil);

#if trackIndex
			trackDebugMsg("end rebuild index\n");
#endif
			//Ind->UnLockTargetTables(context);
			//context->Release();
			EndDataModif(nil);
			ClearUpdating();
		}
		else
			SetIndexEncours(nil);
	}
}


Boolean Base4D::TryToPoseVerrouDataLocker()
{
	return fDataLockerMutex.TryToLock();
}


Boolean Base4D::PoseVerrouDataLocker()
{
	return fDataLockerMutex.Lock();
}


Boolean Base4D::RetireVerrouDataLocker()
{
	return fDataLockerMutex.Unlock();
}


void Base4D::ClearLockCount(BaseTaskInfo* context)
{
	PoseVerrouDataLocker();

	fTotalLocks = fTotalLocks - fAllLocks.RemoveContext(context);
	for (DataTableVector::iterator cur = fDataTables.begin(), end = fDataTables.end(); cur != end; cur++)
	{
		if (*cur != nil)
			(*cur)->ClearLockCount(context);
	}

	RetireVerrouDataLocker();
}


Boolean Base4D::IncLockCount(BaseTaskInfo* context)
{
	if (fAllLocks.IncCount(context))
	{
		fTotalLocks++;
		return true;
	}
	else
		return false;
}


Boolean Base4D::DecLockCount(BaseTaskInfo* context)
{
	if (fAllLocks.DecCount(context))
	{
		fTotalLocks--;
		return true;
	}
	else
		return false;
}



void Base4D::StartDataModif(BaseTaskInfo* context)
{
	LockEntity* newlock;
	if (context == nil)
	{
		newlock = vGetLockEntity();
	}
	else newlock = context->GetLockEntity();

	Boolean stop = false;
	sLONG sleeptime = 5;
	while (!stop)
	{
		PoseVerrouDataLocker();
		LockEntity* dblocker = GetDBLocker();
		if (dblocker == nil || dblocker == newlock || dblocker->GetOwner() == nil || dblocker->GetLockOthersTimeOut() != -1)
		{
			stop = true;
		}
		else
		{
			if (GetDataModifCount(context) > 0)
			{
				stop = true;
			}
		}

		if (stop)
		{
			IncDataModif(context);
		}

		RetireVerrouDataLocker();

		if (!stop)
		{
			VTaskMgr::Get()->GetCurrentTask()->Sleep(sleeptime);
			sleeptime = sleeptime * 2;
			if (sleeptime > 100)
				sleeptime = 100;			
		}
	}
}


Boolean Base4D::TryToStartDataModif(BaseTaskInfo* context)
{
	LockEntity* newlock;
	if (context == nil)
	{
		newlock = vGetLockEntity();
	}
	else newlock = context->GetLockEntity();

	Boolean stop = false;
	sLONG sleeptime = 5;
	
	
	PoseVerrouDataLocker();
	LockEntity* dblocker = GetDBLocker();
	if (dblocker == nil || dblocker == newlock || dblocker->GetOwner() == nil || dblocker->GetLockOthersTimeOut() != -1)
	{
		stop = true;
	}
	else
	{
		if (GetDataModifCount(context) > 0)
		{
			stop = true;
		}
	}

	if (stop)
	{
		IncDataModif(context);
	}

	RetireVerrouDataLocker();
	

	return stop;
}


void Base4D::EndDataModif(BaseTaskInfo* context)
{
	PoseVerrouDataLocker();
	DecDataModif(context);
	RetireVerrouDataLocker();
}


void Base4D::IncDataModif(BaseTaskInfo* context)
{
	if (testAssert(fDataModifCounts.IncCount(context)))
		fCountDataModif++;
	xbox_assert(fCountDataModif>0);
}


void Base4D::DecDataModif(BaseTaskInfo* context)
{
	if (testAssert(fDataModifCounts.DecCount(context)))
		fCountDataModif--;
	xbox_assert(fCountDataModif>=0);
}


sLONG Base4D::GetDataModifCount(BaseTaskInfo* context)
{
	return fDataModifCounts.GetCountFor(context);
}


uBOOL Base4D::Lock(BaseTaskInfo* Context, bool specialFlushAndLock, Boolean WaitForEndOfRecordLocks, sLONG TimeToWaitForEndOfRecordLocks)
{
	uBOOL canlock = true;
	LockEntity *newlock;
	Boolean stop = false;
	
	if (fIsRemote)
	{
	}
	else
	{
		if (Context == nil)
		{
			newlock = vGetLockEntity();
		}
		else newlock = Context->GetLockEntity();
		uLONG startmillisec = VSystem::GetCurrentTime();
		sLONG sleeptime = 5;
		
		while (!stop)
		{
			canlock = true;
			sLONG8 counttotlock;
			Boolean SomeOtherLock = false, SomeDataModif = false;

			PoseVerrouDataLocker();
			if (fBaseisLockedBy == nil || fBaseisLockedBy == newlock || fBaseisLockedBy->GetOwner() == nil)
			{
				if (newlock->GetLockOthersTimeOut() == -1)
				{
					if ((fCountDataModif-GetDataModifCount(Context)) > 0)
						SomeDataModif = true;
				}
				if (WaitForEndOfRecordLocks)
				{
					counttotlock = fTotalLocks - fAllLocks.GetCountFor(Context);
					SomeOtherLock = (counttotlock != 0);
				}
				if (SomeOtherLock && TimeToWaitForEndOfRecordLocks == 0)
					canlock = false;
				else
				{
					fCountLock++;
					fBaseisLockedBy = newlock;
					if (newlock != nil)
						newlock->SetSpecialFlushAndLock(specialFlushAndLock);
				}
			}
			else
				canlock = false;
			RetireVerrouDataLocker();

			if (canlock)
			{
				if (SomeDataModif)
				{
					sLONG sleeptime = 5;
					Boolean stop2 = false;
					while (!stop2)
					{
						VTaskMgr::Get()->GetCurrentTask()->Sleep(sleeptime);
						sleeptime = sleeptime * 2;
						if (sleeptime > 100)
							sleeptime = 100;

						PoseVerrouDataLocker();
						if (WaitForEndOfRecordLocks)
						{
							counttotlock = fTotalLocks - fAllLocks.GetCountFor(Context);
							SomeOtherLock = (counttotlock != 0);
						}
						if ((fCountDataModif-GetDataModifCount(Context)) <= 0)
							stop2 = true;
						RetireVerrouDataLocker();
					}
				}

				if (SomeOtherLock)
				{
					sLONG sleeptime = 5;
					Boolean stop2 = false;
					while (!stop2)
					{
						if (TimeToWaitForEndOfRecordLocks == -1)
						{
							VTaskMgr::Get()->GetCurrentTask()->Sleep(sleeptime);
							sleeptime = sleeptime * 2;
							if (sleeptime > 100)
								sleeptime = 100;
						}
						else
						{
							uLONG currentime = VSystem::GetCurrentTime();
							sLONG passedtime = currentime - startmillisec;
							if ( passedtime < TimeToWaitForEndOfRecordLocks)
							{
								sLONG remaintime = TimeToWaitForEndOfRecordLocks - passedtime;
								if (sleeptime > remaintime)
									sleeptime = remaintime;
								VTaskMgr::Get()->GetCurrentTask()->Sleep(sleeptime);
								sleeptime = sleeptime * 2;
								if (sleeptime > 100)
									sleeptime = 100;
							}
							else
								stop2 = true;
						}

						if (!stop2)
						{
							PoseVerrouDataLocker();
							counttotlock = fTotalLocks - fAllLocks.GetCountFor(Context);
							SomeOtherLock = (counttotlock != 0);
							RetireVerrouDataLocker();
							if (!SomeOtherLock)
								stop2 = true;
						}
					}

					if (SomeOtherLock)
					{
						PoseVerrouDataLocker();
						fCountLock--;
						if (fCountLock == 0)
							fBaseisLockedBy = nil;			
						RetireVerrouDataLocker();
						canlock = false;
					}
				}
			}

			if (canlock)
				stop = true;
			else
			{
				if (SomeOtherLock)
					stop = true;
				else
				{
					sLONG timeout = fBaseisLockedBy->GetLockOthersTimeOut();
					sLONG timeout2 = Context->GetLockTimer();
					if (timeout == -1 || timeout2 == -1)
					{
						VTaskMgr::Get()->GetCurrentTask()->Sleep(sleeptime);
						sleeptime = sleeptime * 2;
						if (sleeptime > 100)
							sleeptime = 100;
						stop = false;
					}
					else
					{
						if (timeout == 0 && timeout2 == 0)
						{
							stop = true;
						}
						else
						{
							if (timeout < timeout2)
								timeout = timeout2;
							uLONG currentime = VSystem::GetCurrentTime();
							sLONG passedtime = currentime - startmillisec;
							if ( passedtime < timeout)
							{
								sLONG remaintime = timeout - passedtime;
								if (sleeptime > remaintime)
									sleeptime = remaintime;
								VTaskMgr::Get()->GetCurrentTask()->Sleep(sleeptime);
								sleeptime = sleeptime * 2;
								if (sleeptime > 100)
									sleeptime = 100;
								stop = false;
							}
							else
								stop = true;
						}
					}
				}
			}
		}
	}

	return canlock;
}


void Base4D::UnLock(BaseTaskInfo* Context)
{
	LockEntity *newlock;

	if (fIsRemote)
	{
	}
	else
	{
		if (Context == nil)
		{
			newlock = vGetLockEntity();
		}
		else newlock = Context->GetLockEntity();
		
		PoseVerrouDataLocker();
		if (fBaseisLockedBy == newlock)
		{
			fBaseisLockedBy->SetSpecialFlushAndLock(false);
			fCountLock--;
			if (fCountLock == 0)
				fBaseisLockedBy = nil;
		}

		RetireVerrouDataLocker();
	}
}


uBOOL Base4D::FlushAndLock(BaseTaskInfo* Context, Boolean WaitForEndOfRecordLocks, sLONG TimeToWaitForEndOfRecordLocks)
{
	if (fIsRemote)
	{
		return true;
	}
	else
	{
#if debuglr == 117
		DebugMsg(L"Flush and Lock : avant lock, pID = "+ToString(VTaskMgr::Get()->GetCurrentTaskID())+L" \n\n");
#endif
		if (Lock(Context, true, WaitForEndOfRecordLocks, TimeToWaitForEndOfRecordLocks))
		{
#if debuglr == 117
			DebugMsg(L"Flush and Lock : avant flush, pID = "+ToString(VTaskMgr::Get()->GetCurrentTaskID())+L" \n\n");
#endif
			VDBMgr::GetManager()->FlushCache(true);
#if debuglr == 117
			DebugMsg(L"Flush and Lock : apres flush, pID = "+ToString(VTaskMgr::Get()->GetCurrentTaskID())+L" \n\n");
##endif
if debuglr
			debug_tools_started = true;
#endif
			return true;
		}
		else
			return false;
	}
}





Boolean Base4D::ValidAddress(DataAddr4D adr, sLONG len)
{
	Boolean res = true;

	if (adr <= 0)
		res = false;
	else
	{
		sLONG numseg = adr & (kMaxSegData-1);
		adr=adr & (DataAddr4D)(-kMaxSegData);
		if (numseg>=kMaxSegDataNormaux)
		{
			SegData* seg = GetSpecialSegment(numseg);
			if (seg == nil)
				res = false;
			else
			{
				if (adr + (DataAddr4D)len > seg->Getfinfic())
					res = false;
			}
		}
		else
		{
			if (numseg>fSegments.GetCount())
				res = false;
			else
			{
				if (adr + (DataAddr4D)len > fSegments[numseg]->Getfinfic())
					res = false;
			}
		}
	}

	return res;
}

/*
bool comparelessrel(const Relation* r1, const Relation* r2)
{
	if (r1->fSources.GetCount() != r2->fSources.GetCount())
		return r1->fSources.GetCount() < r2->fSources.GetCount();
	else
	{
		for (FieldArray::ConstIterator cur1 = r1->fSources.First(), cur2 = r2->fSources.First(), end1 = r1->fSources.End(); cur1 != end1; cur1++; cur2++)
		{
			if ((*cur1)->GetPosInRec() != (*cur2)->GetPosInRec())
				return (*cur1)->GetPosInRec() < (*cur2)->GetPosInRec();
		}
		for (FieldArray::ConstIterator cur1 = r1->fDestinations.First(), cur2 = r2->fDestinations.First(), end1 = r1->fDestinations.End(); cur1 != end1; cur1++; cur2++)
		{
			if ((*cur1)->GetPosInRec() != (*cur2)->GetPosInRec())
				return (*cur1)->GetPosInRec() < (*cur2)->GetPosInRec();
		}
		return r1->GetOldType() < r2->GetOldType();
	}
}
*/

VError Base4D::oldLoadRelations()
{
	VError err = VE_OK;
	Relation* rel;
	vector<Relation*> allrels;

	StructElemDef* e = LoadRelationDef(0, err);
	if (e != nil)
	{
		VConstPtrStream buf(e->GetDataPtr(), (VSize)e->GetDataLen());
		err = buf.OpenReading();
		if (err == VE_OK)
		{
			buf.SetNeedSwap(e->NeedSwap());
			sLONG nb,i;
			err = buf.GetLong(nb);
			if (nb >= 0)
			{

				for (i = 0; i < nb && err == VE_OK; i++)
				{
					sLONG xtyp = buf.GetLong();
					if (xtyp != 0)
					{
						rel = new Relation( this, -1);
						if (rel != nil)
						{
							err = rel->oldGetFrom(buf, this);
							if (err != VE_OK)
							{
								rel->Release();
							}
							else
							{
								allrels.push_back(rel);
								
							}
						}
					}
				}
			}


			buf.CloseReading();
		}

		e->libere();

		DeleteRelationDef(0);
	}


	if (err == VE_OK)
	{
		//sort(allrels.begin(), allrels.end(), comparelessrel);
		sLONG nb2 = (sLONG)allrels.size();
		for (sLONG i = 0; i < nb2; i++)
		{
			Relation* rel = allrels[i];
			if (rel != nil)
			{
				rel->NotOkToSave();
				for (sLONG j = i+1; j < nb2; j++)
				{
					Relation* rel2 = allrels[j];
					if (rel2 != nil)
					{
						if (rel2->Match(rel->GetSources(), rel->GetDestinations()))
						{
							rel2->Release();
							allrels[j] = nil;
						}
						else
						{
							if (rel2->Match(rel->GetDestinations(), rel->GetSources()))
							{
								if (rel2->GetOldType() == 2 && rel->GetOldType() == 3)
								{
									rel->SetOppositeName(rel2->GetName(), NULL);
									rel->SetAuto1toNLoad(rel2->IsAutoLoadNto1(), NULL);
									rel->SetForeignKey(rel2->IsForeignKey(), NULL);
									rel->SetReferentialIntegrity(rel2->WithReferentialIntegrity(), rel2->AutoDeleteRelatedRecords(), NULL);
									if (rel2->IsForSubtable())
										rel->SetForSubTable(true);
									rel2->Release();
									rel2 = nil;
									allrels[j] = nil;
									rel->OkToSave();
									AddRelation(rel, false, nil);
									rel->Release();
									rel = nil;
									allrels[i] = nil;
									break;
								}
								else
								{
									if (rel2->GetOldType() == 3 && rel->GetOldType() == 2)
									{
										rel2->NotOkToSave();
										rel2->SetOppositeName(rel->GetName(), NULL);
										rel2->SetAuto1toNLoad(rel->IsAutoLoadNto1(), NULL);
										rel2->SetForeignKey(rel->IsForeignKey(), NULL);
										rel2->SetReferentialIntegrity(rel->WithReferentialIntegrity(), rel->AutoDeleteRelatedRecords(), NULL);
										if (rel->IsForSubtable())
											rel2->SetForSubTable(true);
										rel->Release();
										rel = nil;
										allrels[i] = nil;
										rel2->OkToSave();
										AddRelation(rel2, false, nil);
										rel2->Release();
										rel2 = nil;
										allrels[j] = nil;
										break;
									}
									else
									{
										xbox_assert(false);
									}
								}
							}
						}
					}
				}
			}

			if (rel != nil)
			{
				if (rel->GetOldType() == 2)
					rel->SwapSourceDest();
				rel->OkToSave();
				AddRelation(rel, false, nil);
				rel->Release();
				rel = nil;
				allrels[i] = nil;

			}
		}
	}
	

	return err;
}


VError Base4D::LoadRelations()
{
	VError err = VE_OK;
	Relation* rel;
	sLONG nb = fStructure->hbbloc.nbRelations;

	fRelations.SetCount(0);

	if (fRelations.SetAllocatedSize(nb))
	{
		fRelations.SetCount(nb, nil);

		for (sLONG i = 0; i < nb && err == VE_OK; i++)
		{
			StructElemDef* e = LoadRelationDef(i, err);
			if (e != nil)
			{
				rel = new Relation( this, i);
				if (rel == nil)
					err = ThrowError(memfull, DBaction_LoadingRelations);
				else
				{
					err = rel->Load(e);
					if (err != VE_OK)
					{
						rel->Release();
					}
					else
					{
						fRelations[i] = rel;
						AddObjectID(objInBase_Relation, rel, rel->GetUUID());
						//rel->RegisterForLang();  // fait dans le Base4D:RegisterLang
						rel->CalculDependence();
					}
				}
				e->libere();
			}
		}
	}
	else
		err = ThrowError(memfull, DBaction_LoadingRelations);

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOTLOADRELATIONS, DBaction_LoadingRelations);
	else
		DeleteBadRelations();

	return err;
}


VFileDesc* Base4D::GetDataIndexSeg() const
{
	if (fIndexSegment != nil)
		return fIndexSegment->GetFileDesc();
	else
		return nil;
}

VFileDesc* Base4D::GetStructIndexSeg() const
{
	if (fStructure != nil)
		return fStructure->GetDataIndexSeg();
	else
		return nil;
}

VError Base4D::GetDataSegs(vector<VFileDesc*>& outSegs, Boolean inWithSpecialSegs) const
{
	outSegs.clear();
	try
	{
		for (VArrayOf<SegData*>::ConstIterator cur = fSegments.First(), end = fSegments.End(); cur != end; cur++)
		{
			if (*cur != nil)
				outSegs.push_back((*cur)->GetFileDesc());
		}
		if (inWithSpecialSegs)
		{
			if (fIndexSegment != nil)
				outSegs.push_back(fIndexSegment->GetFileDesc());
		}

		return VE_OK;
	}
	catch (...)
	{
		return ThrowError(memfull, DBaction_GettingDataSegs);
	}
}


VError Base4D::GetStructSegs(vector<VFileDesc*>& outSegs, Boolean inWithSpecialSegs) const
{
	outSegs.clear();
	if (fStructure == nil)
		return VE_OK;
	else
		return fStructure->GetDataSegs(outSegs, inWithSpecialSegs);
}


sLONG Base4D::GetNumOfTableRefInData(const VString& name) const
{
	sLONG result = 0;
	occupe();

	TableNameMap::const_iterator found = fTableNamesInData.find(name);
	if (found != fTableNamesInData.end())
		result = found->second;
	libere();

	return result;
}


sLONG Base4D::GetNumOfTableRefInData(const VUUID& id) const
{
	sLONG result = 0;
	occupe();

	TableRefMap::const_iterator found = fTableDefsInData.find(id);
	if (found != fTableDefsInData.end())
		result = found->second;
	libere();

	return result;
}


void Base4D::DropTableDefInData(sLONG num)
{
	if (num >= 0)
	{
		DeleteTableDefInDatas(num-1);
		for (TableRefMap::iterator cur = fTableDefsInData.begin(), end = fTableDefsInData.end(); cur != end; cur++)
		{
			if (cur->second == num)
			{
				fTableDefsInData.erase(cur);
				break;
			}
		}
		for (TableNameMap::iterator cur = fTableNamesInData.begin(), end = fTableNamesInData.end(); cur != end; cur++)
		{
			if (cur->second == num)
			{
				fTableNamesInData.erase(cur);
				break;
			}
		}
	}

}


DataTable* Base4D::RetainDataTableByUUID(const VUUIDBuffer& id) const
{
	VObjLock lock(this);
	for (DataTableVector::const_iterator cur = fDataTables.begin(), end = fDataTables.end(); cur != end; cur++)
	{
		DataTableRegular* df = *cur;
		if (df != nil)
		{
			if (df->GetUUID() == id)
			{
				df->Retain();
				return df;
			}
		}

	}
	return nil;
}



void Base4D::LoadAllTableDefsInData(bool buildTableInStruct)
{
	VError err = VE_OK;
	if (fStructure != nil)
	{
		StErrorContextInstaller errs(false);

		Boolean enoughmem = true;

		if (!buildTableInStruct)
		{
			fTableDefTabAddr.Init(this,this,&hbbloc.TableDef_addrtabaddr,&hbbloc.TableDef_debuttrou,&hbbloc.nbTableDef, 
				nil, nil, 0);

			fTableDefInMem.Init(hbbloc.nbTableDef, false);
			fTableDefInMem.SetContientQuoi(t_tabledef);
		}
		
		fRealDataTableNums.resize(hbbloc.nbTableDef, -1);
		fTableNamesInDataByNum.resize(hbbloc.nbTableDef);
		fTableDefsInDataByNum.resize(hbbloc.nbTableDef);

		for (sLONG i = 0; i < hbbloc.nbTableDef && enoughmem; i++)
		{
			err = VE_OK;
			StructElemDef* e = LoadTableDefInDatas(i, err);
			if (e != nil)
			{
				e->libere();
				TableRegular* tt = new TableRegular(this, 0, buildTableInStruct, false);
				err = tt->load(true, i+1);
				if (err == VE_OK)
				{
					VString s;
					VUUID id;
					tt->GetName(s);
					tt->GetUUID(id);
					try
					{
						fTableNamesInData.insert(make_pair(s, i+1));
						fTableDefsInData.insert(make_pair(id, i+1));
						fTableNamesInDataByNum[i] = s;
						fTableDefsInDataByNum[i] = id;
					}
					catch (...)
					{
						enoughmem = false;
					}
				}
				if (buildTableInStruct)
				{
					err = AddTable(tt, false, nil, true, false, false);
				}
				else
					tt->ReleaseAllFields();
				tt->Release();
			}
		}

	}
}


VError Base4D::ResurectDataTable(sLONG TableDefNumInData, CDB4DBaseContext* inContext)
{
	VError err = VE_OK;

	if (fIsRemote)
		err = ThrowError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
	else
	{
		sLONG dfnum = fRealDataTableNums[TableDefNumInData-1];
		if (dfnum != -1)
		{
			StructElemDef* e = LoadTableDefInDatas(TableDefNumInData-1, err);
			if (e != nil)
			{
				e->libere();
				TableRegular* tt = new TableRegular(this, 0, true, false);
				err = tt->load(true, TableDefNumInData, true);
				if (err == VE_OK)
				{
					DataTableRegular* df = fDataTables[dfnum-1];
					err = AddTable(tt, false, inContext, true, false, false);
					if (err == VE_OK)
					{
						df->SetAssociatedTable(tt, TableDefNumInData);
					}
				}
				tt->Release();
			}
		}
	}

	if (err != VE_OK)
	{
		err = ThrowError(VE_DB4D_DATATABLE_CANNOT_BE_RESURECTED, noaction);
	}
	return err;
}


VError Base4D::ResurectDataTable(const VUUID& inTableID, CDB4DBaseContext* inContext)
{
	VError err = VE_OK;
	occupe();
	Table* tt = FindAndRetainTableRef(inTableID);
	if (tt != nil)
	{
		tt->Release();
	}
	else
	{
		sLONG TableDefNumInData = GetNumOfTableRefInData(inTableID);
		if (TableDefNumInData != 0)
			err = ResurectDataTable(TableDefNumInData, inContext);
		else
			err = ThrowError(VE_DB4D_DATATABLE_CANNOT_BE_RESURECTED, noaction);
	}
	libere();
	return err;
}

VError Base4D::ResurectDataTable(const VString& inTableName, CDB4DBaseContext* inContext)
{
	VError err = VE_OK;
	occupe();
	Table* tt = FindAndRetainTableRef(inTableName);
	if (tt != nil)
	{
		tt->Release();
	}
	else
	{
		if (fIsRemote)
		{
			IRequest *req = CreateRequest( inContext, Req_ResurectTable + kRangeReqDB4D);
			if (req == NULL)
			{
				err = ThrowError( memfull, noaction);
			}
			else
			{
				req->PutBaseParam( this);
				req->PutStringParam( inTableName);

				err = req->GetLastError();
				if (err == VE_OK)
				{
					err = req->Send();
					if (err == VE_OK)
					{
						sLONG result = req->GetLongReply( err);
					}
				}
				req->Release();
			}
			
		}
		else
		{
			sLONG TableDefNumInData = GetNumOfTableRefInData(inTableName);
			if (TableDefNumInData != 0)
				err = ResurectDataTable(TableDefNumInData, inContext);
			else
			{
				ThrowBaseError(VE_DB4D_DATATABLE_CANNOT_BE_FOUND, inTableName);
				err = ThrowError(VE_DB4D_DATATABLE_CANNOT_BE_RESURECTED, noaction);
			}
		}
	}
	libere();
	return err;
}


void Base4D::AssociateTableRefInDataWithDataTable(sLONG TableDefInDataNum, sLONG DataTableRealNum)
{
	if (TableDefInDataNum > fRealDataTableNums.size())
		fRealDataTableNums.resize(TableDefInDataNum, -1);
	fRealDataTableNums[TableDefInDataNum-1] = DataTableRealNum;
}


VError Base4D::ResurectGhostTables()
{
	VError err = VE_OK;

	occupe();
	for (TableArray::Iterator cur = fich.First(), end = fich.End(); cur != end; ++cur)
	{
		Table* tt = *cur;
		if (tt != nil)
		{
			VString tname;
			tt->GetName(tname);
			EntityModel* em = fEntityCatalog->RetainEntity(tname);
			if (em == nil)
				em = EntityModel::BuildLocalEntityModel(tt, fEntityCatalog);
			QuickReleaseRefCountable(em);

		}
	}
	libere();
	

	return err;
}


VError Base4D::BuildTablesFromDataTables()
{
	VError err = VE_OK;
	for (DataTableVector::iterator cur = fDataTables.begin(), end = fDataTables.end(); cur != end && err == VE_OK; cur++)
	{
		DataTableRegular* df = *cur;
		if (df != nil)
		{
			if (df->CanBeResurected())
			{
				sLONG TableDefNumInData = df->GetTableDefNumInData();
				err = ResurectDataTable(TableDefNumInData, nil);
			}
		}
	}
	return err;
}


VError Base4D::_fixForWakandaV4()
{
	for (sLONG i = 0; i < fNbTable; ++i)
	{
		Table* tt = fich[i+1];
		if (tt != nil)
		{
			tt->save(true);
		}
	}
	return VE_OK;
}


VError Base4D::GetListOfDeadTables(vector<VString>& outTableNames, vector<VUUID>& outTableIDs, CDB4DBaseContext* inContext)
{
	VError err = VE_OK;
	if (fIsRemote)
	{
		IRequest *req = CreateRequest( inContext, Req_GetListOfDeadTables + kRangeReqDB4D);
		if (req == NULL)
		{
			err = ThrowError( memfull, noaction);
		}
		else
		{
			req->PutBaseParam( this);

			err = req->GetLastError();
			if (err == VE_OK)
			{
				err = req->Send();
				if (err == VE_OK)
				{
					sLONG nb = req->GetLongReply( err);
					VStream* instream = req->GetInputStream();
					outTableNames.clear();
					outTableIDs.clear();
					try
					{
						for (sLONG i = 0; i < nb && err == VE_OK; i++)
						{
							VString s;
							VUUID id;
							err = s.ReadFromStream(instream);
							if (err == VE_OK)
								err = id.ReadFromStream(instream);
							if (err == VE_OK)
							{
								outTableNames.push_back(s);
								outTableIDs.push_back(id);
							}
						}
					}
					catch (...)
					{
						err = ThrowBaseError(memfull);
					}
				}
			}
			req->Release();
		}

	}
	else
	{
		try
		{
			outTableNames.clear();
			outTableIDs.clear();
			for (DataTableVector::iterator cur = fDataTables.begin(), end = fDataTables.end(); cur != end; cur++)
			{
				DataTableRegular* df = *cur;
				if (df != nil)
				{
					if (df->CanBeResurected())
					{
						sLONG TableDefNumInData = df->GetTableDefNumInData();
						outTableNames.push_back(fTableNamesInDataByNum[TableDefNumInData-1]);
						outTableIDs.push_back(fTableDefsInDataByNum[TableDefNumInData-1]);
					}
				}
			}
		}
		catch (...)
		{
			err = ThrowBaseError(memfull);
		}
	}
	return err;
}


/*
VError Base4D::SaveRelations()
{
	VError err = VE_OK;
	sLONG i,len,nb,reltype;
	xbox::VPtrStream buf;
	Relation* rel;

	occupe();

	err = buf.OpenWriting();
	buf.SetNeedSwap(false);
	if (err == VE_OK)
	{
		nb = fRelations.GetCount();
		err = buf.PutLong(nb);
		if (err == VE_OK)
		{
			for (i = 0; i < nb && err == VE_OK; i++)
			{
				rel = fRelations[i];
				if (rel == nil)
				{
					err = buf.PutLong(0);
				}
				else
				{
					if (rel->occupe())
						err = VE_DB4D_CANNOTSAVESTRUCT;
					else
					{
						reltype = rel->GetType();
						err = buf.PutLong(reltype);
						if (err == VE_OK)
							err = rel->PutInto(buf, this);

						rel->libere();
					}
					
				}
					

			}

		}

		if (err == VE_OK)
		{
			Boolean isnew = false;
			StructElemDef* e = LoadRelationDef(0, err);
			if (err == VE_OK)
			{
				if (e == nil)
				{
					e = new StructElemDef(fStructure, -1);
					isnew = true;
				}
				err = e->CopyData(buf.GetDataPtr(), buf.GetSize());
				if (err == VE_OK)
				{
					err = SaveRelationDef(e);
					e->libere();
				}
				else
				{
					if (isnew)
						delete e;
				}
			}
		}

		buf.CloseWriting(false);

	}

	if (err == VE_OK)
		fNeedsToSaveRelation = false;

	libere();

	return err;
}
*/

VError Base4D::DisposeRelations()
{
	V0ArrayOf<Relation*>::Iterator cur, end = fRelations.End();
	for (cur = fRelations.First(); cur != end; cur++)
	{
		Relation* rel = *cur;
		if (rel != nil)
			rel->Release();
	}
	fRelations.SetCount(0);
	return VE_OK;
}

#if 0
sLONG Base4D::FindNextFreeRelation(VError& err) const
{
	err = VE_OK;
	sLONG res = fRelationDefTabAddr.findtrou(NULL, err);
	if (res != -1)
		xbox_assert(fRelations[res] == nil);
	/*
	sLONG nb = fRelations.GetCount();
	sLONG i, res = -1;

	for (i = 0; i < nb; i++)
	{
		if (fRelations[i] == nil)
		{
			res = i;
			break;
		}
	}
	*/

	return res;
}
#endif

Relation* Base4D::RetainRelation( sLONG inRelationIndex) const
{
	occupe();
	
	Relation *rel = NULL;
	if ( (inRelationIndex >= 0) && (inRelationIndex < fRelations.GetCount()) )
	{
		rel = fRelations[inRelationIndex];
		if (rel != NULL)
			rel->Retain();
	}
	
	libere();
	
	return rel;
}


Relation* Base4D::FindAndRetainRelationByName(const VString& Name) const
{
	Relation* res = nil;
	Relation* rel;

	occupe();
	sLONG nb = fRelations.GetCount();
	sLONG i;

	for (i = 0; i < nb; i++)
	{
		rel = fRelations[i];
		if (rel != nil)
		{
			if (rel->GetName() == Name || rel->GetOppositeName() == Name)
			{
				rel->Retain();
				res = rel;
				break;
			}

		}
	}

	libere();
	return res;

}


Relation* Base4D::FindAndRetainRelationByRelVar(const CVariable* RefToSearch) const
{
	Relation* res = nil;
	Relation* rel;

	occupe();
	sLONG nb = fRelations.GetCount();
	sLONG i;

	for (i = 0; i < nb; i++)
	{
		Field* xsource;
		Field* xdest;
		rel = fRelations[i];
		if (rel != nil)
		{
			if (rel->GetRelVarNto1() == RefToSearch || rel->GetRelVar1toN() == RefToSearch)
			{
				rel->Retain();
				res = rel;
				break;
			}
		}
	}

	libere();
	return res;
}


Relation* Base4D::FindAndRetainRelationByUUID(const VUUID &ID) const
{
	VObjLock lock(this);
	Relation* res = nil;
	/*
	Relation* rel;

	occupe();
	sLONG nb = fRelations.GetCount();
	sLONG i;

	for (i = 0; i < nb; i++)
	{
		Field* xsource;
		Field* xdest;
		rel = fRelations[i];
		if (rel != nil)
		{
			if (rel->GetUUID() == ID)
			{
				rel->Retain();
				res = rel;
				break;
			}
		}
	}

	libere();
	*/
	res = (Relation*)GetObjectByID(objInBase_Relation, ID);
	if (res != nil)
		res->Retain();
	return res;
}


Relation* Base4D::FindAndRetainRelation(const FieldArray& sources, const FieldArray& dests) const
{
	VObjLock lock(this);

	Relation* res = nil;

	for (V0ArrayOf<Relation*>::ConstIterator cur = fRelations.First(), end = fRelations.End(); cur != end; cur++)
	{
		Relation* rel = *cur;
		if (rel != nil)
		{
			if (rel->Match(sources, dests))
			{
				rel->Retain();
				res = rel;
				break;
			}
		}
	}

	return res;

}


Relation* Base4D::FindAndRetainRelation(const Field* source, const Field* dest) const
{
	Relation* res = nil;
	Relation* rel;

	occupe();
	sLONG nb = fRelations.GetCount();
	sLONG i;

	for (i = 0; i < nb; i++)
	{
		Field* xsource;
		Field* xdest;
		rel = fRelations[i];
		if (rel != nil)
		{
			rel->GetDef(xsource,xdest);
			// L.E. 27/05/05, in 4D there can be only one relation from a field,
			// so I need to find it without knowing its destination,
			// so I pass NULL as dest.


			// L.E. 02/01/06, en 4D on peut avoir deux liens croises entre deux champs donc 4 CDB4DRelation.
			// d'ou la necessite de passer le type de relation en plus du couple (source, destination).
			// pour le nouveau language (DB4DLang::FindRelationMethod), en passant 0, la fonction retourne la premiere relation trouve (hack temporaire ?).
			if ( (source == xsource) &&  (dest == xdest) )
			{
				rel->Retain();
				res = rel;
				break;
			}
		}
	}

	libere();
	return res;
}


VError Base4D::AddRelation( Relation *inRelation, bool inWithNamesCheck, CDB4DBaseContext* inContext)
{
	VError err = VE_OK;
	
	ClearAllCachedRelPath();
	if (fIsRemote)
	{
	}
	else
	{
		if (OkToUpdate(err))
		{
			const FieldArray& sources = inRelation->GetSources();
			const FieldArray& dests = inRelation->GetDestinations();

			occupe();

			Relation* other = nil;
			
			if (inWithNamesCheck)
			{
				VString name;
				inRelation->GetName( name);
				other = FindAndRetainRelationByName( name);
			}
			
			Relation* other2 = FindAndRetainRelation( sources, dests);

			if ( (other == NULL) && (other2 == NULL))
			{
				if (fStoreAsXML && inRelation->GetPosInList() == -1)
				{
					inRelation->SetPosInList(FindNextRelationNum());
				}

				err = inRelation->Save();
				if (err == VE_OK)
				{
					sLONG n = inRelation->GetPosInList();
					if (n >= 0)
					{
						if (fRelations.GetCount() <= n)
							fRelations.SetCount(n+1, nil);
						inRelation->Retain();
						fRelations[n] = inRelation;
						{
							StErrorContextInstaller errs(false);
							inRelation->AddConstraint();
							inRelation->AddConstraintCols();
						}
					}
				}

				/*
				sLONG n = FindNextFreeRelation(err);
				if (err == VE_OK)
				{
					if (n == -1)
					{
						
						if (fRelations.Add( inRelation))
						{
							inRelation->Retain();
							inRelation->SetPosInList(fRelations.GetCount() - 1);
						}
						else
							err = ThrowError( VE_MEMORY_FULL, DBaction_CreatingRelation);
					}
					else
					{
						fRelations[n] = inRelation;
						inRelation->Retain();
						inRelation->SetPosInList(n);
						inRelation->Touch();
					}
				}
				*/

				if (err == VE_OK)
				{
					AddObjectID(objInBase_Relation, inRelation, inRelation->GetUUID());
					NeedToSaveRelations();
				}
			}
			else
			{
				err = ThrowError(VE_DB4D_RELATION_ALREADY_EXISTS, DBaction_CreatingRelation);
			}

			if (other != nil)
				other->Release();

			if (other2 != nil)
				other2->Release();

			libere();

			// after releasing Base4D
			if (err == VE_OK)
			{
				Touch();
				inRelation->RegisterForLang();
				inRelation->CalculDependence();
				VUUID xUID2, xUID = fID;
				inRelation->GetUUID(xUID2);
				if (VDBMgr::GetCommandManager() != nil)
					VDBMgr::GetCommandManager()->Tell_AddRelation(xUID, xUID2);
			}
			ClearUpdating();
		}
	}
	
	return err;
}



VError Base4D::LoadRelation( const VValueBag& inBag, VBagLoader *inLoader, CDB4DBaseContext* inContext)
{
	VError err = VE_OK;
	Relation *rel = new Relation( this, -1);
	if (rel == NULL)
	{
		err = ThrowError( VE_MEMORY_FULL, DBaction_CreatingRelation);
	}
	else
	{
		err = rel->LoadFromBagWithLoader( inBag, inLoader, inContext);
		fRelations.Add(rel);
		rel->SetPosInList(fRelations.GetCount()-1);
		AddObjectID( objInBase_Relation, rel, rel->GetUUID());
	}

	return err;
}


VError Base4D::CreateRelation( const VValueBag& inBag, VBagLoader *inLoader, Relation **outRelation, CDB4DBaseContext* inContext)
{
	VError err = VE_OK;
	Relation *rel = new Relation( this, -1);
	if (rel == NULL)
	{
		err = ThrowError( VE_MEMORY_FULL, DBaction_CreatingRelation);
	}
	else
	{
		err = rel->LoadFromBagWithLoader( inBag, inLoader, inContext);

		if (err == VE_OK)
			err = AddRelation( rel, inLoader->WithNamesCheck(), inContext);

		if (err != VE_OK)
		{
			rel->Release();
			rel = NULL;
		}
	}
	
	if (outRelation != NULL)
		*outRelation = rel;
	else if (rel != NULL)
		rel->Release();
	
	return err;
}


Relation* Base4D::CreateRelation(const VString &name, const VString &oppositename, Field* source, Field* dest, VError &err, CDB4DBaseContext* inContext)
{
	err = VE_OK;

	Relation *rel = NULL;

	if (source == nil)
	{
		err = ThrowError(VE_DB4D_WRONGSOURCEFIELD, DBaction_CreatingRelation);
	}
	else if (dest == nil)
	{
		err = ThrowError(VE_DB4D_WRONGDESTINATIONFIELD, DBaction_CreatingRelation);
	}
	else if (!IsValid4DName(name))
	{
		err = ThrowError(VE_DB4D_INVALIDRELATIONNAME, DBaction_CreatingRelation);
	}
	else if (source->GetTyp() != dest->GetTyp())
	{
		err = ThrowError(VE_DB4D_FIELDTYPENOTMATCHING, DBaction_CreatingRelation);
	}
	else
	{
		if (fIsRemote)
		{
			IRequest *req = CreateRequest( inContext, Req_AddRelation + kRangeReqDB4D);
			if (req == NULL)
			{
				err = ThrowError( memfull, DBaction_CreatingRelation);
			}
			else
			{
				req->PutBaseParam( this);
				req->PutStringParam( name);
				req->PutStringParam( oppositename);
				req->PutTableParam( source->GetOwner());
				req->PutFieldParam( source);
				req->PutTableParam( dest->GetOwner());
				req->PutFieldParam( dest);

				err = req->GetLastError();
				if (err == VE_OK)
				{
					err = req->Send();
					if (err == VE_OK)
					{
						sLONG result = req->GetByteReply( err);
						if (err == VE_OK && result == 1)
						{
							sLONG relIndex = req->GetLongReply( err);
							if (err == VE_OK && relIndex >= 0)
								err = UpdateRelation( relIndex, req->GetInputStream());
							if (err == VE_OK)
								rel = RetainRelation( relIndex);
						}
					}
				}
				req->Release();
			}
		}
		else
		{
			rel = new Relation(&name, &oppositename, source, dest, -1);
			if (rel == NULL)
			{
				err = ThrowError(memfull, DBaction_CreatingRelation);
			}
			else
			{
				err = AddRelation( rel, true, inContext);

				if (err != VE_OK)
				{
					rel->Release();
					rel = NULL;
				}
			}
		}
	}
		
	return rel;
}



Relation* Base4D::CreateRelation(const VString &name, const VString &oppositename, const CDB4DFieldArray& inSourceFields, 
								 const CDB4DFieldArray& inDestinationFields, VError &err, CDB4DBaseContext* inContext)
{
	err = VE_OK;

	Relation *rel = NULL;

	if (inSourceFields.GetCount() > 0 && inSourceFields.GetCount() == inDestinationFields.GetCount())
	{
		Field* source = dynamic_cast<const VDB4DField*>(inSourceFields[0])->GetField();
		Field* dest = dynamic_cast<const VDB4DField*>(inDestinationFields[0])->GetField();

		if (source == nil)
		{
			err = ThrowError(VE_DB4D_WRONGSOURCEFIELD, DBaction_CreatingRelation);
		}
		else if (dest == nil)
		{
			err = ThrowError(VE_DB4D_WRONGDESTINATIONFIELD, DBaction_CreatingRelation);
		}
		else if (!IsValid4DName(name))
		{
			err = ThrowError(VE_DB4D_INVALIDRELATIONNAME, DBaction_CreatingRelation);
		}
		else if (source->GetTyp() != dest->GetTyp())
		{
			err = ThrowError(VE_DB4D_FIELDTYPENOTMATCHING, DBaction_CreatingRelation);
		}
		else
		{
			rel = new Relation(&name, &oppositename, source, dest, -1);
			if (rel == NULL)
			{
				err = ThrowError(memfull, DBaction_CreatingRelation);
			}
			else
			{
				err = rel->ExtendRelationFields(inSourceFields, inDestinationFields);
				if (err != VE_OK)
				{
					rel->Release();
					rel = nil;
				}
				else
				{
					err = AddRelation( rel, true, inContext);

					if (err != VE_OK)
					{
						rel->Release();
						rel = NULL;
					}
				}
			}
		}
	}
	else
		err = VE_DB4D_FIELDTYPENOTMATCHING;


	return rel;
}


void Base4D::DeleteBadRelations()
{
	StErrorContextInstaller errs(false);
	for (sLONG i = 0, nb = fRelations.GetCount(); i < nb; i++)
	{
		Relation* rel = fRelations[i];
		if (rel != nil)
		{
			bool bad = false;
			Field* source = rel->GetSource();
			if (source == nil)
				bad = true;
			else if (!source->CanBePartOfRelation())
				bad = true;
			Field* dest = rel->GetDest();
			if (dest == nil)
				bad = true;
			else if (!dest->CanBePartOfRelation())
				bad = true;

			if (bad)
				DeleteRelation(rel, nil, false);
		}
	}

}


VError Base4D::DeleteRelation(Relation* RelationToDelete, CDB4DBaseContext* inContext, Boolean inOnlyLocal)
{
	ClearAllCachedRelPath();
	VError err = VE_OK;
	if (fIsRemote && !inOnlyLocal)
	{
		if (RelationToDelete->GetOwner() == this)
		{
			IRequest *req = CreateRequest(inContext, Req_DropRelation + kRangeReqDB4D);
			if (req == NULL)
			{
				err = ThrowError(memfull, DBaction_DeletingRelation);
			}
			else
			{
				req->PutBaseParam( this);
				req->PutRelationParam( RelationToDelete);
				err = req->GetLastError();
				if (err == VE_OK)
				{
					err = req->Send();
					if (err == VE_OK)
					{
						err = DeleteRelation( RelationToDelete, inContext, true);
					}
				}
				req->Release();
			}
		}
	}
	else
	{
		if (fIsRemote || OkToUpdate(err))	// sc 14/05/2008 ACI0056313
		{
			occupe();
			sLONG nb = fRelations.GetCount();
			sLONG i;

			if testAssert(RelationToDelete != nil)
			{
				//for (i = 0; i < nb; i++)
				i = RelationToDelete->GetPosInList();
				{
					if (fRelations[i] == RelationToDelete)
					{
						VUUID xUID2, xUID = fID;
						RelationToDelete->GetUUID(xUID2);
						if (VDBMgr::GetCommandManager() != nil)
							VDBMgr::GetCommandManager()->Tell_DelRelation(xUID, xUID2);

						fRelations[i] = nil;
						//NeedToSaveRelations();

						DelObjectID(objInBase_Relation, RelationToDelete, RelationToDelete->GetUUID());
						RelationToDelete->DelConstraint();
						RelationToDelete->DelConstraintCols();
						RelationToDelete->RemoveFromDependence();
						RelationToDelete->UnRegisterForLang();
						if (!fIsRemote)
							RelationToDelete->SetExtraProperties(nil, false, inContext);
						RelationToDelete->InvalidRelation();
						RelationToDelete->Release();
						if (!fIsRemote && !fStoreAsXML)
							DeleteRelationDef(i);

					}
				}
			}

			if (!fIsRemote)
				Touch();

			libere();
			
			if (!fIsRemote)
				ClearUpdating();
		}
	}

	return err;
}


VError Base4D::GetAndRetainRelations(VArrayRetainedOwnedPtrOf<Relation*> &outRelations) const
{
	VError err = VE_OK;
	Relation* empty = nil;

	occupe();

	sLONG nb = fRelations.GetCount();
	if (outRelations.SetAllocatedSize(nb))
	{
		outRelations.SetCount(nb);
		sLONG i, nb2 = 0;
		for (i = 0; i < nb; i++)
		{
			Relation* rel = fRelations[i];
			if (rel != nil)
			{
				rel->Retain();
				outRelations[nb2] = rel;
				nb2++;
			}
		}
		outRelations.SetCount(nb2, empty, NO_DESTROY);
	}
	else
	{
		ThrowError(memfull, DBaction_BuildingListOfRelations);
	}
	libere();

	return err;
}


CDB4DBase* Base4D::RetainStructDatabase(const char* DebugInfo) const
{
	if (fStructure == nil)
	{
		return nil;
	}
	else
	{
		return fStructure->RetainBaseX(DebugInfo);
	}

}

CDB4DBase* Base4D::RetainBaseX(const char* DebugInfo) const
{
	CDB4DBase* result;
	occupe();
	if (fBaseX == nil)
	{
		result = new VDB4DBase(VDBMgr::GetManager(), (Base4D*)this, true);
		/*
		fBaseX = result;
		fBaseX->Retain(DebugInfo);
		*/
	}
	else
	{
		fBaseX->Retain(DebugInfo);
		result = fBaseX;
	}
	libere();
	return result;
}


void Base4D::BeginClose(VSyncEvent* waitclose)
{
	VDBMgr::GetManager()->RemoveProjectInfo(fRiaServerProjectRef);
	occupe();

	if (waitclose != nil)
	{
		fCloseSyncEvent = waitclose;
		fCloseSyncEvent->Retain();
	}

	if (fBaseX != nil)
	{
		xbox_assert(dynamic_cast<VDB4DBase*>(fBaseX)->GetRefCount() == 1);
		fBaseX->Release();
		fBaseX = nil;
	}

	libere();
}



Boolean Base4D::LoadElements( const VValueBag& inBag, VBagLoader *inLoader, CDB4DBaseContext* inContext)
{
	sLONG i, count;
	const VBagArray *elements;
	const VValueBag *bag;

	Boolean success = true;	
	VError err = VE_OK;

	{
		//schemas
		BuildFirstSchemaIfNeeded();

		elements = inBag.RetainElements( DB4DBagKeys::schema);
		count = elements->GetCount();
		for( i = 1 ; i <= count ; ++i)
		{
			StErrorContextInstaller errorContext;

			bag = elements->GetNth( i);
			if (bag != NULL)
			{
				VString name;
				bag->GetString(DB4DBagKeys::name, name);
				sLONG id;
				if (!bag->GetLong(DB4DBagKeys::id, id))
					id = i;

				CDB4DSchema* schema = new VDB4DSchema(this, name, id);
				if (schema != nil)
				{
					fSchemasByName[name] = schema;
					fSchemasByID[id] = schema;
				}

				success = success && (err == VE_OK);
				if ( (err != VE_OK) && inLoader->StopOnError())
					break;
				errorContext.MergeAndFlush();
			}
		}
		elements->Release();

		// tables
		elements = inBag.RetainElements( DB4DBagKeys::table);
		count = elements->GetCount();
		for( i = 1 ; i <= count ; ++i)
		{
			StErrorContextInstaller errorContext;

			bag = elements->GetNth( i);
			if (bag != NULL)
			{
				err = LoadTable( *bag, inLoader, inContext);
				success = success && (err == VE_OK);
				if ( (err != VE_OK) && inLoader->StopOnError())
					break;
				errorContext.MergeAndFlush();
			}
		}
		elements->Release();


		// indices
		fIndexesBag = inBag.RetainElements( DB4DBagKeys::index);  // l'analyse se fera plus tard


		// relations
		elements = inBag.RetainElements( DB4DBagKeys::relation);
		count = elements->GetCount();
		for( i = 1 ; i <= count ; ++i)
		{
			StErrorContextInstaller errorContext;

			bag = elements->GetNth( i);
			if (bag != NULL)
			{
				err = LoadRelation( *bag, inLoader, inContext);
				success = success && (err == VE_OK);
				if ( (err != VE_OK) && inLoader->StopOnError())
					break;
				errorContext.MergeAndFlush();
			}
		}
		elements->Release();

		//SaveRelations();
	}

	return success;
}


Boolean Base4D::CreateElements( const VValueBag& inBag, VBagLoader *inLoader, CDB4DBaseContext* inContext)
{
	sLONG i, count;
	const VBagArray *elements;
	const VValueBag *bag;

	Boolean success = true;	
	VError err = VE_OK;
	
	if (OkToUpdate(err))
	{
		//schemas
		BuildFirstSchemaIfNeeded();

		elements = inBag.RetainElements( DB4DBagKeys::schema);
		count = elements->GetCount();
		for( i = 1 ; i <= count ; ++i)
		{
			StErrorContextInstaller errorContext;

			bag = elements->GetNth( i);
			if (bag != NULL)
			{
				err = CreateSchema( *bag, inLoader, NULL, inContext);
				success = success && (err == VE_OK);
				if ( (err != VE_OK) && inLoader->StopOnError())
					break;
				errorContext.MergeAndFlush();
			}
		}
		elements->Release();

		// tables
		elements = inBag.RetainElements( DB4DBagKeys::table);
		count = elements->GetCount();
		for( i = 1 ; i <= count ; ++i)
		{
			StErrorContextInstaller errorContext;

			bag = elements->GetNth( i);
			if (bag != NULL)
			{
				err = CreateTable( *bag, inLoader, NULL, inContext, false);
				success = success && (err == VE_OK);
				if ( (err != VE_OK) && inLoader->StopOnError())
					break;
				errorContext.MergeAndFlush();
			}
		}
		elements->Release();

		
		// indices
		elements = inBag.RetainElements( DB4DBagKeys::index);
		count = elements->GetCount();
		for( i = 1 ; i <= count ; ++i)
		{
			StErrorContextInstaller errorContext;

			bag = elements->GetNth( i);
			if (bag != NULL)
			{
				err = CreateIndex( *bag, inLoader, NULL, inContext);
				success = success && (err == VE_OK);
				if ( (err != VE_OK) && inLoader->StopOnError())
					break;
				errorContext.MergeAndFlush();
			}
		}
		elements->Release();


		// relations
		elements = inBag.RetainElements( DB4DBagKeys::relation);
		count = elements->GetCount();
		for( i = 1 ; i <= count ; ++i)
		{
			StErrorContextInstaller errorContext;

			bag = elements->GetNth( i);
			if (bag != NULL)
			{
				err = CreateRelation( *bag, inLoader, NULL, inContext);
				success = success && (err == VE_OK);
				if ( (err != VE_OK) && inLoader->StopOnError())
					break;
				errorContext.MergeAndFlush();
			}
		}
		elements->Release();

		//SaveRelations();
		ClearUpdating();
	}
	else
		success = false;
	
	return success;
}


ReadAheadBuffer* Base4D::NewReadAheadBuffer(sLONG size)
{
	ReadAheadBuffer* buf = new ReadAheadBuffer(this);
	if (buf != nil)
	{
		if (buf->BuildDataPtr(size))
		{
			fBuffersMutex.Lock();
			fBuffers.AddTail(buf);
			fBuffersMutex.Unlock();
		}
		else
		{
			buf->Release();
			buf = nil;
		}
	}

	return buf;
}


void Base4D::RemoveReadAheadBuffer(ReadAheadBuffer* buffer)
{
	fBuffersMutex.Lock();
	fBuffers.Remove(buffer);
	fBuffersMutex.Unlock();
}



StructElemDef* Base4D::LoadStructElemDef(sLONG num, VError &err, AddrTableHeader& StructElemDefTabAddr, 
										StructElemDefTreeInMemHeader& StructElemDefInMem, sLONG& maxelem, DataBaseObjectType signature)
{
	StructElemDef* result = nil;
	//occupe();
	err = VE_OK;
	if (testAssert(!fIsRemote))
	{
		result = (StructElemDef*) StructElemDefInMem.GetFromTreeMemAndOccupe(num);
		if (result == nil)
		{
			sLONG len;
			DataAddr4D ou = StructElemDefTabAddr.GetxOldOne(nil)->GetxAddr(num, nil, err, &len);
			if (ou > 0)
			{
				result = new StructElemDef(this, num, signature, TableDefAccess);
				if (result == nil)
				{
					err = ThrowError(memfull, DBaction_LoadingTableDef);
				}
				else
				{
					err = result->loadobj(ou, len);
					if (err == VE_OK) 
					{
						VError err2 = StructElemDefInMem.PutIntoTreeMem(maxelem, num, result);
						if (err2 == VE_OK)
						{
							result->SetDansCache(true);
						}
					}
				}
			}

			if (err != VE_OK && result != nil)
			{
				result->libere();
				delete result;
				result = nil;
			}
		}
		/*
		else
		{
			result->occupe();
		}
		*/

		//libere();
	}
	else
		err = VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE;

	return result;
}


VError Base4D::SaveStructElemRef(StructElemDef* elem, AddrTableHeader& StructElemDefTabAddr, 
								 StructElemDefTreeInMemHeader& StructElemDefInMem, sLONG& maxelem, DataBaseObjectType signature)
{
	VError err = VE_OK;
	sLONG n;
	Boolean newone = false;

	if (testAssert(!fIsRemote))
	{
		occupe();

		elem->occupe();
		DataAddr4D ou = elem->getaddr();

		if (ou == 0) // new one
		{
			newone = true;
			n = elem->GetNum();
			if (n == -1)
			{
				n = StructElemDefTabAddr.findtrou(nil, err);
				if (err == VE_OK)
				{
					if (n == -1)
					{
						err = StructElemDefInMem.PutIntoTreeMem(maxelem+1, maxelem, elem);
						n = maxelem;
					}
					else
					{
		#if VERSIONDEBUG
						StructElemDef* old = (StructElemDef*)StructElemDefInMem.GetFromTreeMem(n);
						xbox_assert(old == nil || old == elem);
		#endif
						err = StructElemDefInMem.PutIntoTreeMem(maxelem, n, elem);
					}
				}

				if (err == VE_OK)
				{
					elem->SetNum(n);
				}
			}
			else
			{
				err = StructElemDefInMem.PutIntoTreeMem(maxelem, n, elem);
			}
		}
		else
		{
			n = elem->GetNum();
	#if VERSIONDEBUG
			StructElemDef* old = (StructElemDef*)StructElemDefInMem.GetFromTreeMem(n);
			xbox_assert(old == nil || old == elem);
	#endif
			err = StructElemDefInMem.PutIntoTreeMem(maxelem, n, elem);
		}

		if (err == VE_OK)
		{
			if (newone)
			{
				ou = findplace(elem->GetDataLen()+kSizeDataBaseObjectHeader, nil, err, 0, elem);
				if (ou>0)
				{
					elem->setaddr(ou);
					elem->SetAnteLen();
					err = StructElemDefTabAddr.PutxAddr(n ,ou, elem->GetDataLen()+kSizeDataBaseObjectHeader,nil);
					if (err != VE_OK)
					{
						elem->setmodif(false, this, nil);
						elem->ResetAddr();
						libereplace(ou, elem->GetDataLen()+kSizeDataBaseObjectHeader, nil, elem);
					}
				}
			}
			else
			{
				if (adjuste(elem->GetAnteLen()+kSizeDataBaseObjectHeader) != adjuste(elem->GetDataLen()+kSizeDataBaseObjectHeader))
				{
					DataAddr4D ou2 = findplace(elem->GetDataLen()+kSizeDataBaseObjectHeader, nil, err, 0, elem);
					if (ou2 > 0)
					{
						elem->ChangeAddr(ou2, this, nil);
						libereplace(ou, elem->GetAnteLen()+kSizeDataBaseObjectHeader, nil, elem);
						elem->SetAnteLen();
						//elem->setaddr(ou2);
						err = StructElemDefTabAddr.PutxAddr(n ,ou2, elem->GetDataLen()+kSizeDataBaseObjectHeader, nil);
						if (err != VE_OK)
						{
							elem->setmodif(false, this, nil);
							libereplace(ou, elem->GetDataLen()+kSizeDataBaseObjectHeader, nil, elem);
							elem->ResetAddr();
						}
					}

					if (err != VE_OK)
					{
						elem->ResetAddr();
					}
				}
				else
				{
					if (elem->GetAnteLen() != elem->GetDataLen())
					{
						err = StructElemDefTabAddr.PutxAddr(n ,ou, elem->GetDataLen()+kSizeDataBaseObjectHeader,nil);
						elem->SetAnteLen();
					}
				}
			}

			if (err == VE_OK)
			{
				elem->SetDansCache(true);
				elem->setmodif(true, this, nil);
			}
		}

		elem->libere();
		libere();
	}
	else
		err = VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE;

	return err;
}


VError Base4D::DeleteStructElemDef(sLONG num, AddrTableHeader& StructElemDefTabAddr, StructElemDefTreeInMemHeader& StructElemDefInMem, sLONG& maxelem)
{
	VError err = VE_OK;
	StructElemDef* elem;
	DataAddr4D ou;
	sLONG len;

	if (testAssert(!fIsRemote))
	{
		//occupe();
		elem = (StructElemDef*) StructElemDefInMem.GetFromTreeMemAndOccupe(num);

		ou = StructElemDefTabAddr.GetxAddr(num, nil, err, &len);
		if (ou>0)
		{
			err = StructElemDefTabAddr.liberetrou(num, nil);
		}

		if (elem != nil)
		{
			StructElemDefInMem.DelFromTreeMem(num);
			elem->SetParent(nil, 0);
			//VDBMgr::RemoveObjectFromFlush( elem);
			elem->setmodif(false, this, nil);
			elem->SetDansCache(false);
			elem->libere();
			delete elem;
		}

		if (err == VE_OK && ou > 0)
			err = libereplace(ou, len, nil, elem);

		//libere();
	}
	else
		err = VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE;

	return err;
}


VError Base4D::ResizeTableOfStructElemDef(sLONG nbEntries, AddrTableHeader& StructElemDefTabAddr, StructElemDefTreeInMemHeader& StructElemDefInMem, sLONG& maxelem)
{
	return StructElemDefTabAddr.InitAndSetSize(nbEntries, nil);
}


VError Base4D::NormalizeTableOfStructElemDef(AddrTableHeader& StructElemDefTabAddr, StructElemDefTreeInMemHeader& StructElemDefInMem, sLONG& maxelem)
{
	return StructElemDefTabAddr.Normalize(nil);
}





StructElemDef* Base4D::LoadIndexDef(sLONG num, VError &err)
{
	return LoadStructElemDef(num, err, fIndexesTabAddr, fIndexesInMem, hbbloc.nbIndexDef, DBOH_IndexDefElem);
}


VError Base4D::SaveIndexDef(StructElemDef* elem)
{
	return SaveStructElemRef(elem, fIndexesTabAddr, fIndexesInMem, hbbloc.nbIndexDef, DBOH_IndexDefElem);
}


VError Base4D::DeleteIndexDef(sLONG num)
{
	return DeleteStructElemDef(num, fIndexesTabAddr, fIndexesInMem, hbbloc.nbIndexDef);
}





StructElemDef* Base4D::LoadIndexDefInStruct(sLONG num, VError &err)
{
	xbox_assert(!fStoreAsXML);
	return fStructure->LoadStructElemDef(num, err, fStructure->fIndexesInStructTabAddr, fStructure->fIndexesInStructInMem, fStructure->hbbloc.nbIndexDefInStruct, DBOH_IndexInStructDefElem);
}


VError Base4D::SaveIndexDefInStruct(StructElemDef* elem)
{
	xbox_assert(!fStoreAsXML);
	return fStructure->SaveStructElemRef(elem, fStructure->fIndexesInStructTabAddr, fStructure->fIndexesInStructInMem, fStructure->hbbloc.nbIndexDefInStruct, DBOH_IndexInStructDefElem);
}


VError Base4D::DeleteIndexDefInStruct(sLONG num)
{
	xbox_assert(!fStoreAsXML);
	return fStructure->DeleteStructElemDef(num, fStructure->fIndexesInStructTabAddr, fStructure->fIndexesInStructInMem, fStructure->hbbloc.nbIndexDefInStruct);
}






StructElemDef* Base4D::LoadTableDef(sLONG num, VError &err)
{
	xbox_assert(!fStoreAsXML);
	return fStructure->LoadStructElemDef(num, err, fStructure->fTableDefTabAddr, fStructure->fTableDefInMem, fStructure->hbbloc.nbTableDef, DBOH_TableDefElem);
}


VError Base4D::SaveTableRef(StructElemDef* elem)
{
	xbox_assert(!fStoreAsXML);
	return fStructure->SaveStructElemRef(elem, fStructure->fTableDefTabAddr, fStructure->fTableDefInMem, fStructure->hbbloc.nbTableDef, DBOH_TableDefElem);
}


VError Base4D::DeleteTableDef(sLONG num)
{
	xbox_assert(!fStoreAsXML);
	return fStructure->DeleteStructElemDef(num, fStructure->fTableDefTabAddr, fStructure->fTableDefInMem, fStructure->hbbloc.nbTableDef);
}


VError Base4D::ResizeTableOfTableDef(sLONG nbEntries)
{
	if (fStructure == nil)
		return VE_OK;
	else
		return fStructure->ResizeTableOfStructElemDef(nbEntries, fStructure->fTableDefTabAddr, fStructure->fTableDefInMem, fStructure->hbbloc.nbTableDef);
}


VError Base4D::NormalizeTableOfTableDef()
{
	if (fStructure == nil)
		return VE_OK;
	else
		return fStructure->NormalizeTableOfStructElemDef(fStructure->fTableDefTabAddr, fStructure->fTableDefInMem, fStructure->hbbloc.nbTableDef);
}


StructElemDef* Base4D::LoadTableDefInDatas(sLONG num, VError &err)
{
	return LoadStructElemDef(num, err, fTableDefTabAddr, fTableDefInMem, hbbloc.nbTableDef, DBOH_TableDefElem);
}


VError Base4D::SaveTableRefInDatas(StructElemDef* elem)
{
	return SaveStructElemRef(elem, fTableDefTabAddr, fTableDefInMem, hbbloc.nbTableDef, DBOH_TableDefElem);
}


VError Base4D::DeleteTableDefInDatas(sLONG num)
{
	return DeleteStructElemDef(num, fTableDefTabAddr, fTableDefInMem, hbbloc.nbTableDef);
}





StructElemDef* Base4D::LoadRelationDef(sLONG num, VError &err)
{ 
	xbox_assert(!fStoreAsXML);
	if (fStructure->hbbloc.nbRelations == 0)
		return nil;
	else
		return fStructure->LoadStructElemDef(num, err, fStructure->fRelationDefTabAddr, fStructure->fRelationDefInMem, fStructure->hbbloc.nbRelations, DBOH_RelationDefElem);
}


VError Base4D::SaveRelationDef(StructElemDef* elem)
{
	xbox_assert(!fStoreAsXML);
	return fStructure->SaveStructElemRef(elem, fStructure->fRelationDefTabAddr, fStructure->fRelationDefInMem, fStructure->hbbloc.nbRelations, DBOH_RelationDefElem);
}


VError Base4D::DeleteRelationDef(sLONG num)
{
	xbox_assert(!fStoreAsXML);
	return fStructure->DeleteStructElemDef(num, fStructure->fRelationDefTabAddr, fStructure->fRelationDefInMem, fStructure->hbbloc.nbRelations);
}


VStream* Base4D::GetLogStream() const 
{ 
	VStream* result;
	VTaskLock lock(&fLogMutex);
	result = fLogStream; 
	return result;
}


VFile* Base4D::RetainJournalFile() const 
{ 
	VFile* result;
	VTaskLock lock(&fLogMutex);
	if ( fLogFile )
		fLogFile->Retain();
	result = fLogFile; 
	return result;
}

/* create a new fresh journal file using the current log informations */
VError Base4D::ResetJournalFileContent()
{
	VError err = VE_DB4D_LOGFILE_NOT_FOUND;

	/* closing current log stream */
	if ( fLogStream )
	{
		err = fLogStream->CloseWriting();
		delete fLogStream;
		fLogStream = NULL;
		fLogIsValid = false;
		ReleaseRefCountable(&fLogErrorStack);
	}
	
	if ( err == VE_OK )
	{
		if ( fLogFile )
		{
			err = fLogFile->Create( FCR_Overwrite );
			if ( err != VE_OK )
				err = VE_DB4D_CANNOT_CREATE_JOURNAL_FILE;
		}
		else
		{
			err = VE_DB4D_LOGFILE_NOT_FOUND;
		}
	}

	if ( err == VE_OK )
	{
		VFileStream *fileStream = new VFileStream( fLogFile );
		fileStream->SetBufferSize(1024*1024);
		err = fileStream->OpenWriting();
		if ( err != VE_OK )
			err = VE_DB4D_CANNOT_WRITE_JOURNAL_FILE;
		else
		{
			VUUID dataLink;
			GetJournalUUIDLink(dataLink);

			DB4DJournalHeader journalHeader;
			journalHeader.Init(dataLink);
			
			fileStream->SetBufferSize(0);
			err = journalHeader.WriteToStream(fileStream);

			if ( err == VE_OK )
			{
				fLogStream = fileStream;
				fLogIsValid = true;
			}
		}
	}
	return err;
}

VError Base4D::SetJournalFileInfos( VString *inFilePath, VUUID *inUUID, bool inResetJournalSequenceNumber)
{
	VError err = VE_MEMORY_FULL;

	VError bagError;
	VValueBag *myExtraPropertiesBag = nil;
	const VValueBag *bag = RetainExtraProperties( bagError, nil );
	if ( bag != NULL )
	{
		myExtraPropertiesBag = bag->Clone();
		bag->Release();
	}
	else
	{
		myExtraPropertiesBag = new VValueBag();
	}

	if ( myExtraPropertiesBag )
	{
		VValueBag *journal_bag = myExtraPropertiesBag->RetainUniqueElement(LogFileBagKey::journal_file);

		if ( !journal_bag && ((inFilePath != NULL) || (inUUID != NULL)) )
			journal_bag = new VValueBag();

		if (journal_bag != NULL)
		{
			if ( inFilePath )
			{
				VFile *dataFile;
				if (fSegments.GetCount() > 0)
					dataFile = RetainRefCountable( fSegments[0]->GetFile());
				else
					dataFile = RetainDefaultDataFile();

				if ( dataFile )
				{
					VFolder *parentFolder = dataFile->RetainParentFolder();
					if ( parentFolder )
					{
						VString relativePath;
						VFilePath logFilePath;
						logFilePath.FromFullPath( *inFilePath );
						if (logFilePath.GetRelativePosixPath( parentFolder->GetPath(), relativePath))
						{
							// stocke le path relatif en posix
							relativePath.Insert( "./", 1);
							journal_bag->SetString(LogFileBagKey::filepath,relativePath);
						}
						else
						{
							// stocke le path absolu en format natif
							journal_bag->SetString(LogFileBagKey::filepath,logFilePath.GetPath());
						}
			
						// increment log file sequence number
						sLONG sequence_number = 0;
						if (!inResetJournalSequenceNumber)
							journal_bag->GetLong( LogFileBagKey::sequence_number, sequence_number);
						sequence_number += 1;
						journal_bag->SetLong( LogFileBagKey::sequence_number, sequence_number);
					}
					ReleaseRefCountable( &parentFolder);
				}
				ReleaseRefCountable( &dataFile);
			}

			if ( inUUID )
				journal_bag->SetVUUID(LogFileBagKey::datalink,*inUUID);

			if ( inUUID || inFilePath )
			{
				myExtraPropertiesBag->ReplaceElement( LogFileBagKey::journal_file, journal_bag);
				err = SetExtraProperties(myExtraPropertiesBag, true, nil);
				if ( err == VE_OK )
					VDBMgr::GetManager()->FlushCache(false);
			}
			journal_bag->Release();
		}
		myExtraPropertiesBag->Release();
	}

	return err;
}

/* if the VFile is Null then when we remove the journal */
VError Base4D::SetJournalFile(VFile* inNewLog, const VUUID *inDataLink, bool inResetJournalSequenceNumber)
{
	VError err;
	if ( inNewLog )
	{
		err = inNewLog->Create(FCR_Overwrite);
		if (err == VE_OK)
		{
			VFileStream *fileStream = new VFileStream(inNewLog);
			err = fileStream->OpenWriting();
			fileStream->SetBufferSize(0);

			if (err == VE_OK )
			{
				VUUID dataLink;
				DB4DJournalHeader journalHeader;

				if ( inDataLink )
					dataLink = *inDataLink;
				else
					dataLink.Regenerate();

				journalHeader.Init( dataLink );
				err = journalHeader.WriteToStream(fileStream);
				if (err == VE_OK)
				{
					VString fullPath(inNewLog->GetPath().GetPath());
					err = SetJournalFileInfos( &fullPath, &dataLink, inResetJournalSequenceNumber );
				}

				if (err == VE_OK)
				{
					VTaskLock lock(&fLogMutex);
					if (fLogStream != nil)
					{
						fLogStream->CloseWriting();
						delete fLogStream;
						if (fLogFile != nil)
							fLogFile->Release();
					}
					fLogFile = inNewLog; 
					fLogFile->Retain();
					fLogStream = fileStream;
					fLogIsValid = true;
				}
			}
		}
	}
	else
	{
		err = VE_UNIMPLEMENTED;
		VError bagError;
		const VValueBag *bag = RetainExtraProperties( bagError, nil );
		if ( bag != NULL && bagError == VE_OK )
		{
			VTaskLock lock(&fLogMutex);
	
			if ( fLogStream )
			{
				fLogStream->CloseWriting();
				delete fLogStream;
				fLogStream = NULL;
				fLogIsValid = false;
				ReleaseRefCountable(&fLogErrorStack);
			}

			if ( fLogFile )
			{
				fLogFile->Release();
				fLogFile = NULL;
			}

			/* remove all journal information but sequence_number */
			VValueBag *myExtraPropertiesBag = bag->Clone();
			if (myExtraPropertiesBag != NULL)
			{
				VValueBag *journal_bag = myExtraPropertiesBag->GetUniqueElement( LogFileBagKey::journal_file);
				if ( (journal_bag != NULL) && !inResetJournalSequenceNumber)
				{
					journal_bag->RemoveAttribute( LogFileBagKey::datalink);
					journal_bag->RemoveAttribute( LogFileBagKey::filepath);
					journal_bag->RemoveAttribute( LogFileBagKey::next_filepath);
				}
				else
				{
					myExtraPropertiesBag->RemoveElements( LogFileBagKey::journal_file );
				}
			}
			err = SetExtraProperties(myExtraPropertiesBag, true, nil);
			ReleaseRefCountable( &myExtraPropertiesBag);

			if ( err == VE_OK )
				VDBMgr::GetManager()->FlushCache(false);

			bag->Release();
		}
	}
	return err;
}



VError Base4D::StartWriteLog(DB4D_LogAction action, sLONG len, BaseTaskInfo* context, VStream* &outLogStream, bool SignificantAction, bool mutextAlreadyLocked)
{
	if (context != nil)
		context->WriteCreationToLog();

	if (!mutextAlreadyLocked)
		fLogMutex.Lock();

	VError err = VE_OK;
	outLogStream = fLogStream;
	if (fLogStream != nil)
	{
		StErrorContextInstaller errors;
		if (fLogIsValid)
		{
			if (SignificantAction)
				fCurrentLogOperation++;
			if (context != nil)
				context->AddLogEntryToTrans(fLogStream->GetPos());
			fLogStream->ResetFlushedStatus();
			err = fLogStream->PutLong(kTagLogDB4D);
			if (err == VE_OK)
			{
				err = fLogStream->PutLong8(fCurrentLogOperation);
				if (err == VE_OK)
				{			
					err = fLogStream->PutLong(len + kAddToLogLen);
					if (err == VE_OK)
					{
						err = fLogStream->PutLong(action);
						if (err == VE_OK)
						{
							sLONG8 id = 0;
							if (context != nil)
							{
								id = context->GetID();
								if (context->GetCurrentTransaction() != nil)
									id = -id;
							}
							err = fLogStream->PutLong8(id);
							if (err == VE_OK)
							{
								VTime tt;
								tt.FromSystemTime();
								err = fLogStream->PutLong8(tt.GetStamp());
							}
						}
					}
				}
			}
		}
		else
			err = VE_DB4D_CURRENT_JOURNAL_IS_INVALID;

		if (err != VE_OK)
		{
			if (fLogIsValid)
			{
				QuickReleaseRefCountable(fLogErrorStack);
				fLogErrorStack = new VValueBag();
				errors.GetContext()->SaveToBag(*fLogErrorStack);
			}
			fLogIsValid = false;
			fLogMutex.Unlock();
			err = ThrowError(VE_DB4D_CURRENT_JOURNAL_IS_INVALID, noaction);
		}
	}
	return err;

}


VError Base4D::EndWriteLog(sLONG len, bool keepMutexLocked)
{
	VError err = VE_OK;
	if (fLogStream != nil)
	{
		StErrorContextInstaller errors;
		if (fLogIsValid)
		{
			err = fLogStream->PutLong(len + kAddToLogLen);
			if (err == VE_OK)
			{
				err = fLogStream->PutLong(kTagLogDB4DEnd);
				if (err == VE_OK)
				{
					if (fLogStream->GetFlushedStatus())
					{
						//fLogStream->Flush();
						fLogStream->ResetFlushedStatus();
					}
				}
			}
		}
		else
			err = VE_DB4D_CURRENT_JOURNAL_IS_INVALID;

		if (err != VE_OK)
		{
			if (fLogIsValid)
			{
				QuickReleaseRefCountable(fLogErrorStack);
				fLogErrorStack = new VValueBag();
				errors.GetContext()->SaveToBag(*fLogErrorStack);
			}
			fLogIsValid = false;
			err = ThrowError(VE_DB4D_CURRENT_JOURNAL_IS_INVALID, noaction);
		}
	}

	if (!keepMutexLocked)
		fLogMutex.Unlock();
	return err;
}



VError Base4D::WriteLog(DB4D_LogAction action, BaseTaskInfo* context, bool SignificantAction)
{
	VStream *log;
	VError err = StartWriteLog(action, 0, context, log, SignificantAction);
	if (err == VE_OK)
		err = EndWriteLog(0);
	return err;
}


const VValueBag* Base4D::RetainExtraProperties(VError &err, CDB4DBaseContext* inContext)
{
	const VValueBag *extraBag = fExtra.RetainExtraProperties(err);
	return extraBag;
}

const VValueBag* Base4D::RetainStructureExtraProperties(VError &err, CDB4DBaseContext* inContext) const
{
	if (fIsRemote)
	{
		const VValueBag *extraBag = RetainRefCountable(fRemoteStructureExtraProp);
		return extraBag;
	}
	else
	{
		if (fStructure != nil)
		{
			return fStructure->RetainExtraProperties(err, inContext);
		}
		else
			return const_cast<Base4D*>(this)->RetainExtraProperties(err, inContext);
	}
}



VError Base4D::SetStructureExtraProperties(VValueBag* inExtraProperties, bool inNotify, CDB4DBaseContext* inContext)
{
	VError err = VE_UNKNOWN_ERROR;
	if (fIsRemote)
	{
		if (inExtraProperties != NULL)
		{
			IRequest *req = CreateRequest( inContext, Req_SetStructExtraProperties + kRangeReqDB4D);
			if (req == NULL)
			{
				err = ThrowBaseError(memfull, DBaction_ModifyingExtraProperty);
			}
			else
			{
				req->PutBaseParam( this);
				req->PutValueBagParam( *inExtraProperties);

				err = req->GetLastError();
				if (err == VE_OK)
				{
					err = req->Send();
					if (err == VE_OK)
					{
						occupe();
						CopyRefCountable(&fRemoteStructureExtraProp, inExtraProperties);
						libere();
					}
				}
				req->Release();
			}
		}
	}
	else
	{
		if (fStructure == nil)
		{
			err = SetExtraProperties(inExtraProperties, inNotify, inContext);
		}
		else
		{
			err = fStructure->SetExtraProperties(inExtraProperties, inNotify, inContext);
		}
	}
	return err;
}


VError Base4D::SetExtraProperties(VValueBag* inExtraProperties, bool inNotify, CDB4DBaseContext* inContext, bool loadonly)
{
	VError err = VE_UNKNOWN_ERROR;
	if (fIsRemote)
	{
		if (inExtraProperties != NULL)
		{
			IRequest *req = CreateRequest( inContext, Req_SetBaseExtraProperties + kRangeReqDB4D);
			if (req == NULL)
			{
				err = ThrowBaseError(memfull, DBaction_ModifyingExtraProperty);
			}
			else
			{
				req->PutBaseParam( this);
				req->PutValueBagParam( *inExtraProperties);

				err = req->GetLastError();
				if (err == VE_OK)
				{
					err = req->Send();
					if (err == VE_OK)
					{
						occupe();
						err = fExtra.SetExtraProperties( inExtraProperties);
						libere();
					}
				}
				req->Release();
			}
		}
	}
	else
	{
		if (loadonly || OkToUpdate(err))
		{
			err = fExtra.SetExtraProperties(inExtraProperties, loadonly);
			if (!loadonly)
			{
			//	Touch();
				TouchExtraProp();

				if (!fStoreAsXML || GetStructure() != nil)
					setmodif( true, this, nil);

				ClearUpdating();

				if (inNotify && (VDBMgr::GetCommandManager() != nil))
					VDBMgr::GetCommandManager()->Tell_UpdateBase( fID, TUB_ExtraPropertiesChanged);
			}
		}
	}
	return err;
}




DB4DTriggerManager* Base4D::SetTriggerManager(DB4DTriggerManager* inNewTriggerManager)
{
	DB4DTriggerManager* old = fTriggerManager;
	fTriggerManager = inNewTriggerManager;
	return old;
}


Boolean Base4D::IsThereATrigger(DB4D_TriggerType WhatTrigger, Table* tt, bool inCheckCascadingDelete)
{
	if (fTriggerManager == nil)
		return false;

	bool hasTrigger = false;
	if (inCheckCascadingDelete && (WhatTrigger == DB4D_DeleteRecord_Trigger) )
	{
		set<sLONG> tables;
		tt->GetListOfTablesForCascadingDelete( tables);
		for( set<sLONG>::iterator i = tables.begin() ; (i != tables.end()) && !hasTrigger ; ++i)
			hasTrigger = fTriggerManager->IsTriggerExisting(WhatTrigger, *i);
	}
	else
	{
		hasTrigger = fTriggerManager->IsTriggerExisting(WhatTrigger, tt->GetNum());
	}

	return hasTrigger;
}

Boolean Base4D::IsThereATrigger(DB4D_TriggerType WhatTrigger, DataTable* df)
{
	if (fTriggerManager == nil)
		return false;
	else
		return fTriggerManager->IsTriggerExisting(WhatTrigger, df->GetNum());
}


VError Base4D::AddObjectID(TypeObj4DInBase typ, Obj4D* obj, const VUUID& id)
{
	VTaskLock lock(&fObjectIDsMutex);

	Obj4DContainer x;
	x.fType = typ;
	x.fObject = obj;
	try
	{
		fObjectIDs[id] = x;
	}
	catch (...)
	{
		return memfull;
	}
	return VE_OK;
}


VError Base4D::DelObjectID(TypeObj4DInBase typ, Obj4D* obj, const VUUID& id)
{
	VTaskLock lock(&fObjectIDsMutex);

	fObjectIDs.erase(id);
	return VE_OK;
}


Obj4D* Base4D::GetObjectByID(TypeObj4DInBase typ, const VUUID& id) const
{
	VTaskLock lock(&fObjectIDsMutex);

	MapOfObj4D::const_iterator found = fObjectIDs.find(id);
	if (found == fObjectIDs.end())
		return nil;
	else
	{
		if (testAssert(typ == found->second.fType))
			return found->second.fObject;
		else
			return nil;
	}
}


sLONG Base4D::GetNBTableWithoutDeletedTables() const
{
	VObjLock lock(this);

	sLONG result = 0;
	for (TableArray::ConstIterator cur = fich.First(), end = fich.End(); cur != end; cur++)
	{
		if (*cur != nil)
			result++;
	}
	return result;
}


VError Base4D::TakeOffDeletedTables(Bittab *tb) const
{
	VObjLock lock(this);
	VError err = VE_OK;

	sLONG i = 0;
	for (TableArray::ConstIterator cur = fich.First(), end = fich.End(); cur != end; cur++, i++)
	{
		if (*cur == nil)
		{
			err = tb->Clear(i);
			if (err != VE_OK)
				break;
		}
	}

	return err;
}


VError Base4D::InitDataFileOfFields()
{
	VObjLock lock(this);
	VError err = VE_OK;

	TableArray::Iterator cur = fich.First(), end = fich.End();
	for (; cur!=end; cur++)
	{
		Table* fic = *cur;
		if (fic != nil)
			err = fic->InitDataFileOfFields();
		if (err != VE_OK)
			break;
	}

	return err;
}



VError Base4D::InitDataFileOfIndexes()
{
	VObjLock lock(this);
	VError err = VE_OK;

	for (IndexMap::iterator cur = fAllIndexes.begin(), end = fAllIndexes.end(); cur != end && err == VE_OK; cur++)
	{
		err = fDataTableOfIndexes->AddIndexRef(cur->second);
	}

	return err;
}


VError Base4D::InitDataFileOfConstraints()
{
	VObjLock lock(this);
	VError err = VE_OK;

	for (TableArray::Iterator cur = fich.First(), end = fich.End(); cur != end && err == VE_OK; cur++)
	{
		Table* tab = *cur;
		if (tab != nil)
		{
			err = tab->AddConstraint();
		}
	}

	for (V0ArrayOf<Relation*>::Iterator cur = fRelations.First(), end = fRelations.End(); cur != end && err == VE_OK; cur++)
	{
		Relation* rel = *cur;
		if (rel != nil)
		{
			err = rel->AddConstraint();
		}
	}

	return err;

}


VError Base4D::InitDataFileOfConstraintCols()
{
	VObjLock lock(this);
	VError err = VE_OK;

	for (TableArray::Iterator cur = fich.First(), end = fich.End(); cur != end && err == VE_OK; cur++)
	{
		Table* tab = *cur;
		if (tab != nil)
		{
			err = tab->AddConstraintCols();
		}
	}

	for (V0ArrayOf<Relation*>::Iterator cur = fRelations.First(), end = fRelations.End(); cur != end && err == VE_OK; cur++)
	{
		Relation* rel = *cur;
		if (rel != nil)
		{
			err = rel->AddConstraintCols();
		}
	}

	return err;

}


VError Base4D::InitDataFileOfIndexCols()
{
	VObjLock lock(this);
	VError err = VE_OK;

	for (IndexMap::iterator cur = fAllIndexes.begin(), end = fAllIndexes.end(); cur != end && err == VE_OK; cur++)
	{
		err = cur->second->AddToSystemTableIndexCols();
	}

	return err;
}



VError Base4D::CountSegmentFreeBits( SegData *inSegdata, uLONG8 &outFreebits, uLONG8 &outUsedbits) const
{
	XBOX::VError	err = XBOX::VE_OK;
	uLONG8			dsize = 0;
	sLONG			nbFreebitsSec = 0;

	uLONG8 tailleSeg = 0 ; 
	uLONG8 maxBits = 0; 
	uLONG sizeblockseg = 0;

	if( inSegdata != NULL)
	{
		inSegdata->GetSemaphore()->LockWrite();
		OccupableStack* curstack = gOccPool.GetStackForCurrentThread();

		tailleSeg = inSegdata->Getfinfic();
		sizeblockseg = inSegdata->GetSDD()->sizeblockseg; 
		maxBits = tailleSeg / sizeblockseg ;

		err = inSegdata->chargetfb(); 
		if ( err == VE_OK) 
		{
			nbFreebitsSec = inSegdata->CalcNbpageSec();
			uLONG8 crtBit = 0; 
			for ( sLONG j = 0 ; j < nbFreebitsSec; j++) 
			{
				FreebitsSecPtr freebitsec = NULL;
				freebitsec = inSegdata->chargefbsec(j, err, curstack);
				if (freebitsec != NULL)
				{
					sLONG nbPages = freebitsec->calcNBpages();
					for (sLONG k = 0; k < nbPages && err == VE_OK; k++ ) 
					{
						bittableptr fbt = nil;
						sLONG x,y;
						bool dernierePageSec = (k == (freebitsec->calcNBpages()-1));
						fbt = freebitsec->chargefb(k, err, curstack);
						if (fbt != nil) 
						{
							FreeBitsptr fb = fbt->getfbb();
							uLONG* bittableptr =(uLONG*)fb;							
							for ( sLONG n = 0 ; n < NBlongbit;  n++,bittableptr++) 
							{
								if ( *bittableptr == 0)	// tous 32 blocks libres
								{
									dsize=dsize+32;
								}
								else
								{
									if ( (*((sLONG*)bittableptr))==-1)	// tous 32 les blocks occupes
									{
										//rien
									}
									else
									{
										for(sLONG insidelong = 0; insidelong<32; insidelong++)
										{
											if ( (*bittableptr & (1<<insidelong)) == 0 )  // block libre
											{
												dsize=dsize+1;
											}
										}
									}
								}
								crtBit += 32;
								if( dernierePageSec)
								{
									if( crtBit > maxBits)
										break;
								}
							} 
							fbt->unlock();
							fbt->Free(curstack, true);
						}
					}
					freebitsec->Free(curstack, true);
				}	// pour chaque BitTable
			}	// pour chaque page secondaire
		}  
		inSegdata->GetSemaphore()->Unlock();	
	}
	else
	{
		err = XBOX::VE_UNKNOWN_ERROR;
	}

	outFreebits = dsize*sizeblockseg;
	outUsedbits = (maxBits-dsize)*sizeblockseg;
	
	return err;
}

  
VError Base4D::CountDataFreeBits( uLONG8 &outFreebits, uLONG8 &outUsedbits) const
{
	XBOX::VError	err = XBOX::VE_OK;
	uLONG8	freebits = 0, usedbits = 0;

	// pour chaque segment
	for ( sLONG i = 0 ; i < hbbloc.nbsegdata && err == XBOX::VE_OK; i++) 
	{
		SegData*		segData = NULL;
		sLONG			nbFreebitsSec;

		if (i < fSegments.GetCount())
		{
			segData = fSegments[i];
			if( segData)
			{
				uLONG8	segfreebits = 0, segusedbits = 0;
				err = CountSegmentFreeBits( segData, segfreebits, segusedbits);
				if( err == XBOX::VE_OK)
				{
					freebits += segfreebits;
					usedbits += segusedbits;
				}
				else
					break;
			}
			else
			{
				err = XBOX::VE_UNKNOWN_ERROR;
			}
		}		 
		else
		{
			err = XBOX::VE_UNKNOWN_ERROR;
		}
	} 
	outFreebits = freebits;
	outUsedbits = usedbits;
	
	return err;
}


XBOX::VError Base4D::CountBaseFreeBits( BaseStatistics &outStatistics) const
{
	XBOX::VError	err = XBOX::VE_OK;

	uLONG8			datafreebits = 0, datausedbits = 0,
					structfreebits = 0, structusedbits = 0,
					indexdatafreebits = 0, indexdatausedbits = 0,
					indexstructfreebits = 0, indexstructusedbits = 0;
	if( fBaseX)
	{
		err = CountDataFreeBits( datafreebits, datausedbits);
		SegData *dataIndexSegment = fIndexSegment;
		err = CountSegmentFreeBits( dataIndexSegment, indexdatafreebits, indexdatausedbits);
	}

	if (fStructure)
	{
		err = fStructure->CountDataFreeBits( structfreebits, structusedbits);
		SegData *structIndexSegment = fStructure->fIndexSegment;
		err = CountSegmentFreeBits( structIndexSegment, indexstructfreebits, indexstructusedbits);
	}

	outStatistics.datafreebits = datafreebits;
	outStatistics.datausedbits = datausedbits;
	outStatistics.structfreebits = structfreebits;
	outStatistics.structusedbits = structusedbits;
	outStatistics.indexdatafreebits = indexdatafreebits;
	outStatistics.indexdatausedbits = indexdatausedbits;
	outStatistics.indexstructfreebits = indexstructfreebits;
	outStatistics.indexstructusedbits = indexstructusedbits;
	
	return err;
}


void Base4D::GetTablesStamps(StampsVector& outStamps) const
{
	occupe();
	outStamps.clear();
	try
	{
		outStamps.reserve(fich.GetCount()+1);
		for (TableArray::ConstIterator cur = fich.First(), end = fich.End(); cur != end; cur++)
		{
			if (*cur == nil)
				outStamps.push_back(0);
			else
				outStamps.push_back((*cur)->GetStamp());
		}
	}
	catch (...)
	{
	}

	libere();
}


void Base4D::GetRelationsStamps(StampsVector& outStamps) const
{
	occupe();
	outStamps.clear();
	try
	{
		outStamps.reserve(fRelations.GetCount()+1);
		for (V0ArrayOf<Relation*>::ConstIterator cur = fRelations.First(), end = fRelations.End(); cur != end; cur++)
		{
			if (*cur == nil)
				outStamps.push_back(0);
			else
				outStamps.push_back((*cur)->GetStamp());
		}
	}
	catch (...)
	{
	}

	libere();
}



VError Base4D::RetainIndexes(ArrayOf_CDB4DIndex& outIndexes)
{
	VError err = VE_OK;
	outIndexes.ReduceCount(0);
	LockIndexes();

	if (outIndexes.SetAllocatedSize((VIndex)fAllIndexes.size()))
	{
		for (IndexMap::iterator cur = fAllIndexes.begin(), end = fAllIndexes.end(); cur != end && err == VE_OK; cur++)
		{
			IndexInfo* ind = cur->second;
			if (ind != nil)
			{
				VDB4DIndex* xind = new VDB4DIndex(ind);
				if (xind == nil)
					err = ThrowError(memfull, DBaction_GettingIndexes);
				else
				{
					outIndexes.Add(xind);
				}
			}
		}
	}
	else
		err = ThrowError(memfull, DBaction_GettingIndexes);

	UnLockIndexes();
	return err;
}


VError Base4D::DropAllRelationOnOneField(Field* inFieldToDelete, CDB4DBaseContext* inContext, Boolean inOnlyLocal)
{
	occupe();
	for (V0ArrayOf<Relation*>::Iterator cur = fRelations.First(), end = fRelations.End(); cur != end; cur++)
	{
		Relation* rel = *cur;
		if (rel != nil)
		{
			Boolean ok = false;
			FieldArray* fields = &(rel->GetSources());
			for (FieldArray::Iterator cur = fields->First(), end = fields->End(); cur != end; cur++)
			{
				if (*cur == inFieldToDelete)
				{
					ok = true;
				}
			}

			fields = &(rel->GetDestinations());
			for (FieldArray::Iterator cur = fields->First(), end = fields->End(); cur != end; cur++)
			{
				if (*cur == inFieldToDelete)
				{
					ok = true;
				}
			}

			if (ok)
			{
				DeleteRelation(rel, inContext, inOnlyLocal);
			}
		}
	}
	libere();
	return VE_OK;
}


VError Base4D::DropAllRelationOnOneTable(Table* inTableToDelete, CDB4DBaseContext* inContext)
{
	occupe();
	for (V0ArrayOf<Relation*>::Iterator cur = fRelations.First(), end = fRelations.End(); cur != end; cur++)
	{
		Relation* rel = *cur;
		if (rel != nil)
		{
			Boolean ok = false;
			if (rel->GetSource()->GetOwner() == inTableToDelete)
				ok = true;
			if (rel->GetDest()->GetOwner() == inTableToDelete)
				ok = true;

			if (ok)
			{
				DeleteRelation(rel, inContext);
			}
		}
	}
	libere();
	return VE_OK;
}


class RecRefHolder
{
	public:
		RecRefHolder(PrimKey* primkey, sLONG numTable, sLONG numField):fPrimKey(primkey)
		{
			fNumTable = numTable;
			fNumField = numField;
		}

		bool operator < (const RecRefHolder& other) const
		{
			if (fNumTable < other.fNumTable)
				return true;
			else if (fNumTable > other.fNumTable)
				return false;
			else
			{
				if (fNumField < other.fNumField)
					return true;
				else if (fNumField > other.fNumField)
					return false;
				else
				{
					const PrimKey* thisone = fPrimKey.Get();
					const PrimKey* otherone = other.fPrimKey.Get();
					if (*thisone < *otherone)
						return true;
					else
						return false;
				}
			}
		}

	protected:
		VRefPtr<PrimKey> fPrimKey;
		sLONG fNumField;
		sLONG fNumTable;
};


typedef map<pair<sLONG, sLONG>, CDB4DJournalData*> BlobActionMapByBlobID;
typedef map<VString, CDB4DJournalData*> BlobActionMapByPath;

typedef map<RecRefHolder, CDB4DJournalData*> BlobActionMapPrimKey;

//typedef map<pair<uLONG8, sLONG>, CDB4DJournalData*> BlobActionMapByTarget;

static void ClearContextMap(ContextMap& contexts)
{
	for (ContextMap::iterator cur = contexts.begin(), end = contexts.end(); cur != end; cur++)
	{
		BaseTaskInfo* context = cur->second;
		if (context != nil)
			context->Release();
	}
	contexts.clear();
}


BaseTaskInfo* Base4D::BuildContextForJournal(ContextMap& contexts, uLONG8 contextid, VError& err, bool wasInTrans)
{
	BaseTaskInfo* context = nil;
	ContextMap::iterator found = contexts.find(contextid);
	if (found == contexts.end() || found->second == nil)
	{
		if (wasInTrans)
			context = nil;
		else
		{
			context = new BaseTaskInfo(this, nil, nil, nil);
			if (context == nil)
				err = ThrowError(memfull, DBaction_IntegratingJournal);
			else
			{
				try
				{
					contexts[contextid] = context;
				}
				catch (...)
				{
					context->Release();
					err = ThrowError(memfull, DBaction_IntegratingJournal);
				}
			}
		}
	}
	else
		context = found->second;

	return context;
}


VError Base4D::IntegrateJournal(DB4DJournalParser* jparser, uLONG8 from, uLONG8 upto, uLONG8 *outCountIntegratedOperations, VDB4DProgressIndicator* InProgress, bool isMirror, sLONG8* lastMirrorAction )
{
	VError err = VE_OK;
	Boolean startoncurrentoperation = false;
	const VValueInfo* xVPictureRef = nil;

	{
		// wait for current indexes to complete
		vector<IndexInfo*> indexes;
		AwakeAllIndexes( nil, true, indexes);
		for (vector<IndexInfo*>::iterator cur = indexes.begin(), end = indexes.end(); cur != end; cur++)
			(*cur)->Release();
	}

	SetReferentialIntegrityEnabled(false);
	SetCheckUniquenessEnabled(false);
	SetCheckNot_NullEnabled(false);

	uLONG8 nboper = jparser->CountOperations();
	if (from == 0)
	{
		from = 1;
		startoncurrentoperation = true;
	}
	if (upto == 0)
		upto = nboper;

	uLONG8 countIntegratedOperations = 0;
	
	if (InProgress != nil)
	{
		XBOX::VString session_title;
		gs(1005,2,session_title);	// Integrating Journal: %curValue of %maxValue Entries
		InProgress->BeginSession(upto - from + 1,session_title,true);
	}

	if (isMirror)
	{
		if (lastMirrorAction == nil)
			startoncurrentoperation = false;
		if (lastMirrorAction != nil)
		{
			if (jparser->GetFirstOperation() > (*lastMirrorAction) + 1)
			{
				// too recent
				err = ThrowError(VE_DB4D_LOGFILE_TOO_RECENT, DBaction_IntegratingJournal);
			}
			else if (jparser->GetLastOperation() < (*lastMirrorAction))
			{
				// too old
				err = ThrowError(VE_DB4D_LOGFILE_TOO_OLD, DBaction_IntegratingJournal);
			}

		}
	}
	else
	{
		if (hbbloc.lastaction == -1)
			hbbloc.lastaction = 0;
		// check here if there's something to integrate
		if (jparser->GetFirstOperation() > hbbloc.lastaction + 1)
		{
			// too recent
			err = ThrowError(VE_DB4D_LOGFILE_TOO_RECENT, DBaction_IntegratingJournal);
		}
		else if (jparser->GetLastOperation() < hbbloc.lastaction)
		{
			// too old
			err = ThrowError(VE_DB4D_LOGFILE_TOO_OLD, DBaction_IntegratingJournal);
		}
	}

	if ( (from <= upto) && (err == VE_OK) )
	{
		ContextMap contexts;
		BlobActionMapPrimKey blobactions;
		BlobActionMapByPath blobactionsbypath;

		sLONG8 xcuroper = -1;

		CDB4DJournalData* jd;
		err = jparser->SetCurrentOperation(from, &jd);
		
		//ACI0085624: O.R. save current journal entry index and global number of operation being integrated for error reporting
		uLONG8 curoper = 0;
		sLONG8 curOperGlobalNumber = 0;

		for (curoper = from; curoper <= upto && err == VE_OK; curoper++)
		{
			if ( curoper%64 == 0 )
				VTask::Yield();
			if ( InProgress && !InProgress->Progress(curoper-from))
				err = ThrowError(VE_DB4D_ACTIONCANCELEDBYUSER, DBaction_IntegratingJournal);
			else
			{
				Boolean ok = true;
				if (startoncurrentoperation)
				{
					ok = false;
					sLONG8 logoper = jd->GetGlobalOperationNumber();
					if (isMirror)
					{
						if ((*lastMirrorAction)+1 == logoper)
						{
							ok = true;
							startoncurrentoperation = false;
						}
					}
					else
					{
						if ((hbbloc.lastaction+1) == logoper)
						{
							ok = true;
							startoncurrentoperation = false;
						}
					}
				}

				if (ok)
				{
					bool wasintrans = false;
					sLONG action = jd->GetActionType();
					sLONG8 contextid = 0;
					jd->GetContextID(contextid);
					if (contextid < 0)
					{
						contextid = - contextid;
						wasintrans = true;
					}
					curOperGlobalNumber = jd->GetGlobalOperationNumber();

					switch (action)
					{
						case DB4D_Log_OpenData:
						case DB4D_Log_CloseData:
						case DB4D_Log_StartBackup:
							{
								ClearContextMap(contexts);
								//curcontext = nil;
							}
							break;

						case DB4D_Log_CreateContextWithUserUUID:
						case DB4D_Log_CreateContextWithExtra:
							{
								BaseTaskInfo* newcontext = new BaseTaskInfo(this, nil, nil, nil);
								if (newcontext == nil)
									err = ThrowError(memfull, DBaction_IntegratingJournal);
								else
								{
									try
									{
										contexts[contextid] = newcontext;
									}
									catch (...)
									{
										newcontext->Release();
										err = ThrowError(memfull, DBaction_IntegratingJournal);
									}
								}
							}
							break;

						case DB4D_Log_CloseContext:
							{
								BaseTaskInfo* context = contexts[contextid];
								if (context != nil)
									context->Release();
								contexts.erase(contextid);
							}
							break;

						case DB4D_Log_StartCommit:
							{
								BaseTaskInfo* context = BuildContextForJournal(contexts, contextid, err, false);
								if (context != nil)
								{
									context->StartTransaction(err);
								}
							}
							break;
						case DB4D_Log_EndCommit:
							{
								BaseTaskInfo* context = BuildContextForJournal(contexts, contextid, err, wasintrans);
								if (context != nil)
								{
									err = context->CommitTransaction();
								}
							}
							break;

						case DB4D_Log_StartTrans:
							{
								BaseTaskInfo* context = BuildContextForJournal(contexts, contextid, err, wasintrans);
								if (context != nil)
								{
									context->StartTransaction(err);
								}
							}
							break;

						case DB4D_Log_Commit:
							{
								BaseTaskInfo* context = BuildContextForJournal(contexts, contextid, err, wasintrans);
								if (context != nil)
								{
									err = context->CommitTransaction();
								}
							}
							break;

						case DB4D_Log_RollBack:
							{
								BaseTaskInfo* context = BuildContextForJournal(contexts, contextid, err, wasintrans);
								if (context != nil)
								{
									context->RollBackTransaction();
								}
							}
							break;

						case DB4D_Log_CreateBlobWithPrimKey:
						case DB4D_Log_ModifyBlobWithPrimKey:
						case DB4D_Log_DeleteBlobWithPrimKey:
							{
								PrimKey* primkey = jd->GetPrimKey();
								sLONG numfield = jd->GetFieldNumber();
								VUUID tID;
								jd->GetTableID(tID);
								Table* tt = FindAndRetainTableRef(tID);
								if (tt != nil)
								{
									sLONG numblob;
									VString path;
									jd->GetBlobNumber(numblob, path);
									if (numblob == -2 || numblob == -3)
									{
										try
										{
											blobactionsbypath[path] = jd;
											jd->Retain();

											jd->GetDataPtr(); // force loading of data from stream
										}
										catch (...)
										{
											err = ThrowError(memfull, DBaction_IntegratingJournal);
										}
									}
									else
									{
										try
										{
											blobactions[RecRefHolder(primkey, tt->GetNum(), numfield)] = jd;
											jd->Retain();

											jd->GetDataPtr(); // force loading of data from stream
										}
										catch (...)
										{
											err = ThrowError(memfull, DBaction_IntegratingJournal);
										}
									}
								}
							}
							break;

						case DB4D_Log_CreateRecordWithPrimKey:
						case DB4D_Log_ModifyRecordWithPrimKey:
						case DB4D_Log_DeleteRecordWithPrimKey:
						case DB4D_Log_TruncateTable:
							{
								BaseTaskInfo* context = BuildContextForJournal(contexts, contextid, err, wasintrans);
								if (context != nil)
								{
									VUUID tID;
									jd->GetTableID(tID);
									Table* tt = FindAndRetainTableRef(tID);
									if (tt != nil)
									{
										DataTable* df = tt->GetDF();
										if (df != nil)
										{
											sLONG datalen;
											PrimKey* primkey = jd->GetPrimKey();

											if (action == DB4D_Log_TruncateTable)
											{
												bool mustunlock = false;
												err = tt->Truncate(context, nil, false, mustunlock);
											}

											if (action == DB4D_Log_DeleteRecordWithPrimKey)
											{
												
												FicheInMem* fic = df->LoadRecord(primkey, err, DB4D_Keep_Lock_With_Record, context, false, false, nil, nil);

												if (fic != nil)
												{
													FicheOnDisk* ficD = fic->GetAssoc();
													if (ficD != nil)
													{
														sLONG nbfield = ficD->GetNbCrit();
														for (sLONG i = 1; i <= nbfield; i++)
														{
															sLONG typ;
															tPtr p = ficD->GetDataPtr(i, &typ);
															if (p != nil)
															{
																if (typ == VK_TEXT || typ == VK_IMAGE || typ == VK_BLOB || typ == VK_BLOB_DB4D)
																{
																	blobactions.erase(RecRefHolder(primkey, tt->GetNum(), i));
																}
															}
														}
													}
													fic->DoNotCheckIntegrity();
													err = df->DelRecord(fic, context);
													fic->Release();
												}
											}

											if (action == DB4D_Log_CreateRecordWithPrimKey || action == DB4D_Log_ModifyRecordWithPrimKey)
											{
												FicheInMem* fic;
												sLONG nbfield;
												jd->GetCountFields(nbfield);
												if (action == DB4D_Log_CreateRecordWithPrimKey)
												{
													fic = df->NewRecord(err, context);
												}
												else
												{
													fic = df->LoadRecord(primkey, err, DB4D_Keep_Lock_With_Record, context, false, false, nil, nil);
												}
												if (err == VE_OK && fic != nil)
												{
													for (sLONG i = 1; i <= nbfield && err == VE_OK; i++)
													{
														VValueSingle* cv = fic->GetNthField(i, err);
														if (cv != nil)
														{
															sLONG typ;
															VValueSingle* logcv = jd->GetNthFieldValue(i, &typ, df);
															if (logcv == nil && (typ == VK_TEXT || typ == VK_TEXT_UTF8 || typ == VK_IMAGE || typ == VK_BLOB || typ == VK_BLOB_DB4D))
															{
																xbox_assert(logcv == nil);
																VString path;
																sLONG blobid = jd->GetNthFieldBlobRef(i, path);
																if (blobid == -4)
																{
																	cv->SetNull(true);
																	fic->Touch(i);
																}
																else
																{
																	if (blobid == -1)
																	{
																		cv->Clear();
																		fic->Touch(i);
																	}
																	else
																	{
																		if (blobid >= 0)
																		{
																			BlobActionMapPrimKey::iterator found = blobactions.find(RecRefHolder(primkey, tt->GetNum(), i));
																			if (found != blobactions.end())
																			{
																				CDB4DJournalData* jdaction = found->second;
																				switch (jdaction->GetActionType())
																				{
																					case DB4D_Log_DeleteBlobWithPrimKey:
																						cv->Clear();
																						break;

																					case DB4D_Log_CreateBlobWithPrimKey:
																					case DB4D_Log_ModifyBlobWithPrimKey:
																						{
																							sLONG datalen;
																							jdaction->GetDataLen(datalen);
																							if (typ == VK_IMAGE)
																							{
																								if (xVPictureRef == nil)
																								{
																									xVPictureRef = VValue::ValueInfoFromValueKind(VK_IMAGE);
																								}

																								if (xVPictureRef != nil) 
																								{
																									VBlobWithPtr *blob = new VBlobWithPtr();
																									blob->FromData(jdaction->GetDataPtr(), datalen);
																									VValueSingle* tempcv = (ValPtr)xVPictureRef->Generate(blob);
																									cv->FromValue(*tempcv);
																									delete tempcv;
																									//delete blob;
																								}
																							}
																							else
																							{
																								if (typ == VK_TEXT)
																								{
																									((VBlobText*)cv)->FromBlock(jdaction->GetDataPtr(), datalen, VTC_UTF_16);
																								}
																								else
																								{
																									if (typ == VK_TEXT_UTF8)
																									{
																										((VBlobTextUTF8*)cv)->FromBlock(jdaction->GetDataPtr(), datalen, VTC_UTF_8);
																									}
																									else
																									{
																										((VBlob4DWithPtr*)cv)->SetSize(datalen);
																										((VBlob4DWithPtr*)cv)->PutData(jdaction->GetDataPtr(), datalen, 0);
																									}
																								}
																							}

																						}
																						break;
																				}

																				blobactions.erase(found);
																				jdaction->Release();

																				fic->Touch(i);
																			}
																		}
																		else
																		{
																			BlobActionMapByPath::iterator found = blobactionsbypath.find(path);
																			if (found != blobactionsbypath.end())
																			{
																				CDB4DJournalData* jdaction = found->second;
																				switch (jdaction->GetActionType())
																				{
																				case DB4D_Log_DeleteBlobWithPrimKey:
																					cv->Clear();
																					break;

																				case DB4D_Log_CreateBlobWithPrimKey:
																				case DB4D_Log_ModifyBlobWithPrimKey:
																					{
																						sLONG datalen;
																						jdaction->GetDataLen(datalen);
																						if (typ == VK_IMAGE)
																						{
																							if (xVPictureRef == nil)
																							{
																								xVPictureRef = VValue::ValueInfoFromValueKind(VK_IMAGE);
																							}

																							if (xVPictureRef != nil) 
																							{
																								VBlob4DWithPtr *blob = (VBlob4DWithPtr*)CreVBlob4DWithPtr(df, 0, nil, true, false, context);
																								if (blobid != -3)
																								blob->FromData(jdaction->GetDataPtr(), datalen);
																								blob->SetOutsidePath(path, blobid == -2);
																								cv->SetOutsidePath(path, blobid == -2);
																								if (blobid == -3)
																								{
																									{
																										StErrorContextInstaller errs(false);
																										blob->ReloadFromOutsidePath();
																									}
																								}
																								
																								if (err == VE_OK)
																								{
																								VValueSingle* tempcv = (ValPtr)xVPictureRef->Generate(blob);
																								cv->FromValue(*tempcv);
																								delete tempcv;
																								}
																								//delete blob;
																							}
																						}
																						else
																						{
																							if (blobid != -3)
																							{
																							if (typ == VK_TEXT)
																							{
																								((VBlobText*)cv)->FromBlock(jdaction->GetDataPtr(), datalen, VTC_UTF_16);
																							}
																							else
																							{
																								if (typ == VK_TEXT_UTF8)
																								{
																									((VBlobTextUTF8*)cv)->FromBlock(jdaction->GetDataPtr(), datalen, VTC_UTF_8);
																								}
																								else
																								{
																									((VBlob4DWithPtr*)cv)->SetSize(datalen);
																									((VBlob4DWithPtr*)cv)->PutData(jdaction->GetDataPtr(), datalen, 0);
																								}
																							}
																							}
																							cv->SetOutsidePath(path, blobid == -2);
																							{
																								StErrorContextInstaller errs(false);
																								cv->ReloadFromOutsidePath();
																							}
																						}
																						
																					}
																					break;
																				}

																				blobactionsbypath.erase(found);
																				jdaction->Release();

																				fic->Touch(i);
																			}

																		}
																	}
																}
															}
															else
															{
																if (logcv != nil)
																{
																	cv->FromValue(*logcv);
																	delete logcv;
																}
																else
																{
																	cv->SetNull(true);
																}
																fic->Touch(i);
															}
															
														}
													}

													if (err == VE_OK)
													{
														bool oksave = true;
														if (action == DB4D_Log_CreateRecordWithPrimKey)
														{
															PrimKey* primkey = fic->RetainPrimKey(err, false);
															if (err == VE_OK)
															{
																RecIDType dejaID = df->FindRecordID(primkey, err, context);
																if (dejaID != -1)
																{
																	oksave = false;
																}
															}
															QuickReleaseRefCountable(primkey);
														}
														if (oksave && err == VE_OK)
														{
															fic->DoNotCheckIntegrity();
															err = df->SaveRecord(fic, context);
														}
													}
												}

												if (fic != nil)
													fic->Release();

											}

										}
										tt->Release();
									}
								}
							}
							break;

						case DB4D_Log_SaveSeqNum:
							{
								VUUID tID;
								jd->GetTableID(tID);
								Table* tt = FindAndRetainTableRef(tID);
								if (tt != nil)
								{
									DataTable* df = tt->GetDF();
									if (df != nil)
									{
										sLONG8 seqnum;
										if (jd->GetSequenceNumber(seqnum))
										{
											AutoSeqNumber* autoseq = df->GetSeqNum(nil);
											if (autoseq != nil)
											{
												autoseq->SetCurrentValue(seqnum);
											}
										}
									}
									tt->Release();
								}
							}
							break;


					} // du switch
					if (err == VE_OK)
						++countIntegratedOperations;

				}

			}

			if (jd != nil)
			{
				if (err == VE_OK)
					xcuroper = jd->GetGlobalOperationNumber();
				jd->Release();
			}
			uLONG8 indiceoper;
			if (curoper != upto && err == VE_OK)
				err = jparser->NextOperation(indiceoper, NULL, &jd);

			if (err != VE_OK)
			{
				//ACI0085624: O.R. show current entry index and matching operation number as well
				XBOX::VString operationIndex,operationGlobalNumber;
				operationGlobalNumber.FromLong8(curOperGlobalNumber);
				operationIndex.FromLong8((sLONG8)curoper);
				ThrowBaseError(VE_DB4D_INTEGRATE_JOURNAL_FAILED_AT, operationIndex, operationGlobalNumber);
			}
		} // du for

		ClearContextMap(contexts);

		if (xcuroper != -1)
		{
			if (isMirror)
			{
				if (lastMirrorAction != nil)
					*lastMirrorAction = xcuroper;
			}
			else
			{
				xbox_assert(xcuroper >= hbbloc.lastaction);
				hbbloc.lastaction = xcuroper;
				setmodif(true,this,NULL);
			}
		}
//		if (jd != nil)		// afabre : there is nothing to release here
//			jd->Release();

	}
	
	if (outCountIntegratedOperations != NULL)
		*outCountIntegratedOperations = countIntegratedOperations;

	if (InProgress != nil)
	{
		InProgress->EndSession();
	}

	SetReferentialIntegrityEnabled(true);
	SetCheckUniquenessEnabled(true);
	SetCheckNot_NullEnabled(true);

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOT_INTEGRATE_JOURNAL, DBaction_IntegratingJournal);

	return err;
}


VError Base4D::RelateManySelection(Table* StartingSelectionTable, Field* cri, QueryResult& outResult, BaseTaskInfo* context, 
									QueryOptions* options, VDB4DProgressIndicator* InProgress)
{
	VError err = VE_OK;

	Selection* sel = options->GetFilter();
	if (sel != nil)
	{
		Table* dest = cri->GetOwner();

		if (fIsRemote)
		{
			IRequest *req = CreateRequest( context == nil ? nil : context->GetEncapsuleur(), Req_RelateManySelection + kRangeReqDB4D);
			if (req == nil)
			{
				err = ThrowBaseError(memfull, noaction);
			}
			else
			{
				req->PutBaseParam(this);
				req->PutThingsToForget( VDBMgr::GetManager(), context);
				req->PutTableParam(StartingSelectionTable);
				req->PutQueryOptionsParam(options, context);
				req->PutTableParam(dest);
				req->PutFieldParam(cri);
				req->PutProgressParam(InProgress);
				err = req->GetLastError();
				if (err == VE_OK)
				{
					err = req->Send();
					if (err== VE_OK)
						err = req->GetUpdatedInfo(this, context);
					if (err == VE_OK)
					{
						err = req->ReadQueryResultReply(this, dest, &outResult, context);
					}
				}
				req->Release();
			}

		}
		else
		{
			Base4D* db = dest->GetOwner();
			xbox_assert(db == this);
#if WITH_ASSERT
			if (context != nil)
				xbox_assert(context->GetBase() == db);
#endif

			Bittab* lockedset = nil;
			if (options->GetWantsLockedSet())
			{
				lockedset = new Bittab;
				if (lockedset != nil)
					lockedset->SetOwner( db);
			}
			outResult.SetLockedSet(lockedset);

			SearchTab xs(dest, true);
			xs.AddSearchLineSel(sel);
			cri->occupe();
			DepRelationArrayIncluded* rels = cri->GetRelNto1Deps();
			for (DepRelationArrayIncluded::Iterator cur = rels->First(), end = rels->End(); cur != end; cur++)
			{
				Relation* rel = *cur;
				if (rel != nil)
				{
					if (rel->GetDest() != nil && rel->GetDest()->GetOwner() == sel->GetParentFile()->GetTable())
					{
						xs.AddSearchLineBoolOper(DB4D_And);
						xs.AddSearchLineJoin(rel->GetSource(), DB4D_Like, rel->GetDest());
					}
				}
			}
			cri->libere();
			OptimizedQuery query;
			// db->LockIndexes(); // will be done at another level when necessary
			err = query.AnalyseSearch(&xs, context);
			// db->UnLockIndexes();
			if (err == VE_OK)
			{
				Selection* resultsel = query.Perform((Bittab*)nil, InProgress, context, err, options->GetWayOfLocking(), options->GetLimit(), lockedset);
				if (err == VE_OK)
				{
					outResult.SetSelection(resultsel);
					if (resultsel != nil && options->GetWantsFirstRecord() && !resultsel->IsEmpty())
					{
						FicheInMem* firstrect = resultsel->GetParentFile()->LoadRecord(resultsel->GetFic(0), err, options->GetRecordWayOfLocking(), context, true);
						outResult.SetFirstRecord(firstrect);
						ReleaseRefCountable(&firstrect);
					}
				}
				QuickReleaseRefCountable(resultsel);
			}
		}
	}
	else
		err = ThrowError(VE_DB4D_SELECTION_IS_NULL, noaction);

	return err;
}



VError Base4D::RelateOneSelection(Table* StartingSelectionTable, sLONG TargetOneTable, QueryResult& outResult, BaseTaskInfo* context, 
									 QueryOptions* options, VDB4DProgressIndicator* InProgress, std::vector<Relation*> *inPath)
{
	VError err = VE_OK;

	Selection* sel = options->GetFilter();
	if (sel != nil)
	{
		Table* dest = RetainTable(TargetOneTable);
		if (dest == nil)
			err = ThrowBaseError(VE_DB4D_WRONGTABLEREF, noaction);
		else
		{
			if (fIsRemote)
			{
				IRequest *req = CreateRequest( context == nil ? nil : context->GetEncapsuleur(), Req_RelateOneSelection + kRangeReqDB4D);
				if (req == nil)
				{
					err = ThrowBaseError(memfull, noaction);
				}
				else
				{
					req->PutBaseParam(this);
					req->PutThingsToForget( VDBMgr::GetManager(), context);
					req->PutTableParam(StartingSelectionTable);
					req->PutQueryOptionsParam(options, context);
					req->PutLongParam(TargetOneTable);
					sLONG nb = 0;
					if (inPath != nil)
						nb = (sLONG)inPath->size();
					req->PutLongParam(nb);
					if (inPath != nil)
					{
						for (vector<Relation*>::iterator cur = inPath->begin(), end = inPath->end(); cur != end; cur++)
						{
							req->PutRelationParam(*cur);
						}
					}
					req->PutProgressParam(InProgress);

					err = req->GetLastError();
					if (err == VE_OK)
					{
						err = req->Send();
						if (err== VE_OK)
							err = req->GetUpdatedInfo(this, context);

						if (err == VE_OK)
						{
							err = req->ReadQueryResultReply(this, dest, &outResult, context);
						}
					}
					req->Release();
				}
			}
			else
			{
				Base4D* db = dest->GetOwner();
				xbox_assert(db == this);
#if WITH_ASSERT
				if (context != nil)
					xbox_assert(context->GetBase() == db);
#endif

				Bittab* lockedset = nil;
				if (options->GetWantsLockedSet())
				{
					lockedset = new Bittab;
					if (lockedset != nil)
						lockedset->SetOwner( db);
				}
				outResult.SetLockedSet(lockedset);

				SearchTab xs(dest, true);
				xs.AddSearchLineSel(sel);
				if (inPath != nil)
				{
					for (vector<Relation*>::iterator cur = inPath->begin(), end = inPath->end(); cur != end; cur++)
					{
						Relation* rel = *cur;
						if (testAssert(rel != nil))
						{
							xs.AddSearchLineBoolOper(DB4D_And);
							xs.AddSearchLineJoin(rel->GetSource(), DB4D_Like, rel->GetDest());
						}
					}
				}
				OptimizedQuery query;
				// db->LockIndexes(); // will be done at another level when necessary
				err = query.AnalyseSearch(&xs, context);
				// db->UnLockIndexes();
				if (err == VE_OK)
				{
					Selection* resultsel = query.Perform((Bittab*)nil, InProgress, context, err, options->GetWayOfLocking(), options->GetLimit(), lockedset);
					if (err == VE_OK)
					{
						outResult.SetSelection(resultsel);
						if (resultsel != nil && options->GetWantsFirstRecord() && !resultsel->IsEmpty())
						{
							FicheInMem* firstrect = resultsel->GetParentFile()->LoadRecord(resultsel->GetFic(0), err, options->GetRecordWayOfLocking(), context, true);
							outResult.SetFirstRecord(firstrect);
							ReleaseRefCountable(&firstrect);
						}
					}
					QuickReleaseRefCountable(resultsel);
				}
			}
			dest->Release();
		}
	}
	else
		err = ThrowError(VE_DB4D_SELECTION_IS_NULL, noaction);

	return err;
}



FicheInMem* Base4D::BuildRecordFromServer(VStream* from, BaseTaskInfo* inContext, Table* inTable, VError& outError)
{
	FicheInMem* fic = new FicheInMem();
	if (fic == nil)
		outError = ThrowBaseError(memfull, noaction);
	else
	{
		outError = fic->FromServer(from, inContext, inTable);
		if (outError != VE_OK)
		{
			fic->Release();
			fic = nil;
		}
	}

	return fic;
}



FicheInMem* Base4D::BuildRecordFromServerAsSubRecord(VStream* from, BaseTaskInfo* inContext, Table* inTable, VError& outError)
{
	FicheInMem* fic = new FicheInMem();
	if (fic == nil)
		outError = ThrowBaseError(memfull, noaction);
	else
	{
		outError = fic->FromServerAsSubRecord(from, inContext, inTable);
		if (outError != VE_OK)
		{
			fic->Release();
			fic = nil;
		}
	}

	return fic;
}


FicheInMem* Base4D::BuildRecordFromClient(VStream* from, BaseTaskInfo* inContext, Table* inTable, VError& outError)
{
	FicheInMem* fic = nil;
	sLONG xnumrec = -3;

	VUUID xid;
	outError = xid.ReadFromStream(from);
	if (outError == VE_OK)
	{
		fic = VDBMgr::GetManager()->RetainServerKeptRecord(xid.GetBuffer());
	}

	if (fic == nil && outError == VE_OK)
	{
		if (outError == VE_OK)
			outError = from->GetLong(xnumrec);
		if (xnumrec == -3)
		{
			fic = inTable->GetDF()->NewRecord(outError, inContext);	
		}
		else
		{
			//xbox_assert(false); // should never happen ?
			fic = inTable->GetDF()->LoadRecord(xnumrec, outError, DB4D_Keep_Lock_With_Record, inContext, true);
		}
		if (fic != nil)
			fic->SetID(xid.GetBuffer());
	}
	else
		outError = from->GetLong(xnumrec);

	if (outError == VE_OK && fic != nil)
	{
		outError = fic->FromClient(from, inContext);
		if (outError != VE_OK)
		{
			fic->Release();
			fic = nil;
		}
	}

	return fic;
}



FicheInMem* Base4D::BuildRecordMinimalFromClient(VStream* from, BaseTaskInfo* inContext, Table* inTable, VError& outError)
{
	FicheInMem* fic = nil;
	sLONG xnumrec;

	VUUID xid;
	outError = xid.ReadFromStream(from);
	if (outError == VE_OK)
	{
		fic = VDBMgr::GetManager()->RetainServerKeptRecord(xid.GetBuffer());
	}

	if (fic == nil)
	{
		if (outError == VE_OK)
			outError = from->GetLong(xnumrec);
		if (xnumrec == -3)
		{
			fic = inTable->GetDF()->NewRecord(outError, inContext);	
		}
		else
		{
			//xbox_assert(false); // should never happen ?
			fic = inTable->GetDF()->LoadRecord(xnumrec, outError, DB4D_Keep_Lock_With_Record, inContext, true);
		}
		if (fic != nil)
			fic->SetID(xid.GetBuffer());
	}
	else
		outError = from->GetLong(xnumrec);

	return fic;
}


Selection* Base4D::BuildSelectionFromServer(VStream* from, CDB4DBaseContext* inContext, Table* inTable, VError& outErr)
{
	Selection* sel = nil;
	sLONG typsel = 0;
	outErr = from->GetLong(typsel);
	if (typsel != sel_nosel)
	{
		if (outErr == VE_OK)
		{
			switch(typsel)
			{
				case sel_bitsel:
					sel = new BitSel(this, inContext);
					break;
				case sel_petitesel:
					sel = new PetiteSel(this, inContext);
					break;
				case sel_longsel:
					sel = new LongSel(this, inContext);
					((LongSel*)sel)->PutInCache();
					break;
				default:
					outErr = ThrowBaseError(VE_DB4D_INVALID_TYPESEL, noaction);
			}
			if (outErr == VE_OK && sel == nil)
				outErr = ThrowBaseError(memfull, noaction);
		}

		if (outErr == VE_OK)
		{
			outErr = sel->FromServer(from, inContext);
			if (outErr != VE_OK)
			{
				sel->Release();
				sel = nil;
			}
		}
	}

	return sel;
}


Selection* Base4D::BuildSelectionFromClient(VStream* from, CDB4DBaseContext* inContext, Table* inTable, VError& outError)
{
	Selection* sel = nil;
	sLONG typsel;
	VError err = from->GetLong(typsel);
	switch(typsel)
	{
		case sel_nosel:
			sel = nil;
			break;

		case sel_petitesel:
			if (inTable == nil)
				sel = new PetiteSel(this, (CDB4DBaseContext*)-1);
			else
				sel = new PetiteSel(inTable->GetDF(), this);
			if (sel == nil)
				err = ThrowBaseError(memfull, noaction);
			else
			{
				err = sel->FromClient(from, nil); 
				if (err != VE_OK)
				{
					sel->Release();
					sel = nil;
				}
			}
			break;

		case sel_longsel:
		case sel_bitsel:
			{
				VUUID xid;
				err = xid.ReadFromStream(from);
				if (err == VE_OK)
				{
					sel = VDBMgr::GetManager()->RetainServerKeptSelection(xid.GetBuffer());
					if (sel == nil)
						err = ThrowBaseError(VE_DB4D_INVALID_SELECTION_ID, noaction);
				}
			}
			break;

		case sel_longsel_fullfromclient:
			if (inTable == nil)
			{
				sel = new LongSel(this, (CDB4DBaseContext*)-1);
				((LongSel*)sel)->PutInCache();
			}
			else
			{
				sel = new LongSel(inTable->GetDF(), this);
				((LongSel*)sel)->PutInCache();
			}

			if (sel == nil)
				err = ThrowBaseError(memfull, noaction);
			else
			{
				err = sel->FromClient(from, nil); 
				if (err != VE_OK)
				{
					sel->Release();
					sel = nil;
				}
				else
					VDBMgr::GetManager()->KeepSelectionOnServer(sel);
			}
			break;

		case sel_bitsel_fullfromclient:
		case sel_bitsel_fullfromclient_keepOnServer:
			if (inTable == nil)
				sel = new BitSel(this, (CDB4DBaseContext*)-1);
			else
				sel = new BitSel(inTable->GetDF(), this);

			if (sel == nil)
				err = ThrowBaseError(memfull, noaction);
			else
			{
				err = sel->FromClient(from, nil); 
				if (err != VE_OK)
				{
					sel->Release();
					sel = nil;
				}
				else
				{
					if (typsel == sel_bitsel_fullfromclient_keepOnServer)
					{
						VDBMgr::GetManager()->KeepSelectionOnServer(sel);
					}
				}
			}
			break;

		default:
			err = ThrowBaseError(VE_DB4D_INVALID_TYPESEL, noaction);
	}

	return sel;
}


Bittab* Base4D::BuildSetFromServer(VStream* from, CDB4DBaseContext* inContext, VError& outErr)
{
	Bittab* sel = nil;
	sLONG typsel = 0;
	outErr = from->GetLong(typsel);
	if (outErr == VE_OK)
	{
		if (typsel == sel_nosel)
		{
		}
		else
		{
			if (typsel == sel_bitsel)
			{
				sel = new Bittab();
				if (sel == nil)
					outErr = ThrowBaseError(memfull, noaction);
				else
				{
					outErr = sel->FromServer(from, inContext);
					sel->SetOwner(this);
				}
			}
			else
				outErr = ThrowError(VE_DB4D_INVALID_TYPESEL, noaction);
		}
	}

	return sel;
}

Bittab* Base4D::BuildSetFromClient(VStream* from, CDB4DBaseContext* inContext, VError& outErr)
{
	Bittab* sel = nil;
	sLONG typsel = 0;
	outErr = from->GetLong(typsel);
	if (outErr == VE_OK)
	{
		if (typsel == sel_nosel)
		{
		}
		else
		{
			if (typsel == sel_bitsel)
			{
				VUUID xid;
				outErr = xid.ReadFromStream(from);
				if (outErr == VE_OK)
				{
					Selection* xsel = VDBMgr::GetManager()->RetainServerKeptSelection(xid.GetBuffer());
					if (xsel == nil)
						outErr = ThrowBaseError(VE_DB4D_INVALID_SELECTION_ID, noaction);
					else
					{
						if (xsel->GetTypSel() == sel_bitsel)
						{
							sel = ((BitSel*)xsel)->GetBittab();
							sel->Retain();
						}
						else
							outErr = ThrowBaseError(VE_DB4D_INVALID_SELECTION_ID, noaction);
						xsel->Release();
					}
				}
			}
			else
			{
				if ( (typsel == sel_bitsel_fullfromclient) || (typsel == sel_bitsel_fullfromclient_keepOnServer) )
				{
					sel = new Bittab();
					if (sel == nil)
						outErr = ThrowBaseError(memfull, noaction);
					else
					{
						outErr = sel->FromClient(from, inContext);
						if (outErr == VE_OK)
						{
							sel->SetOwner(this);
							if (typsel == sel_bitsel_fullfromclient_keepOnServer)
								VDBMgr::GetManager()->KeepSetOnServer(sel);
						}
						else
						{
							sel->Release();
							sel = nil;
						}
					}
				}
				else
					outErr = ThrowError(VE_DB4D_INVALID_TYPESEL, noaction);
			}
		}
	}

	return sel;
}


void Base4D::IncRemoteMaxRecordsStamp()
{
	VInterlocked::Increment(&fRemoteMaxRecordsStamp);
	if (fRemoteMaxRecordsStamp > 2000000000)
	{
		VInterlocked::Exchange(&fRemoteMaxRecordsStamp, 1);
		occupe();
		for (DataTableVector::iterator cur = fDataTables.begin(), end = fDataTables.end(); cur != end; cur++)
		{
			DataTableRegular* df = *cur;
			if (df != nil)
			{
				df->ResetMaxRecordRemoteInfo();
			}
		}
		libere();
	}
}


VError Base4D::PutMaxRecordRemoteInfo(sLONG curstamp, VStream* outstream, sLONG& outNewStamp)
{
	VError err = VE_OK;
	SmallBittabForTables dejaoccupe;
	Boolean aumoinsunoccupe = false;
	occupe();
	outNewStamp = fRemoteMaxRecordsStamp;

	if (curstamp > fRemoteMaxRecordsStamp)
		curstamp = 0;

	if (curstamp != fRemoteMaxRecordsStamp || curstamp == 0)
	{
		for (DataTableVector::iterator cur = fDataTables.begin(), end = fDataTables.end(); cur != end && err == VE_OK; cur++)
		{
			DataTableRegular* df = *cur;
			if (df != nil)
			{
				if (df->TryToLockWrite())
				{
					err = df->PutMaxRecordRemoteInfo(curstamp, outstream);
					df->Unlock();
				}
				else
				{
					aumoinsunoccupe = true;
					dejaoccupe.Set(cur - fDataTables.begin());
				}
			}
		}
	}
	sLONG nb = (sLONG)fDataTables.size();
	libere();

	if (aumoinsunoccupe)
	{
		for (sLONG i = 0; i < nb && err == VE_OK; i++)
		{
			if (dejaoccupe.isOn(i))
			{
				occupe();
				DataTableRegular* df = fDataTables[i];
				if (df != nil)
					df->Retain();
				libere();
				if (df != nil)
				{
					err = df->PutMaxRecordRemoteInfo(curstamp, outstream);
					df->Release();
				}
			}
		}
	}

	return err;
}


VError Base4D::BuildFirstSchemaIfNeeded()
{
	VError err = VE_OK;
	
	if (fStructure != nil)		// sc 23/09/2008 ACI0059182, fStructure may be nil for remote databases
	{
		Base4D* db = fStructure;
		Table* tt = db->RetainTable(2);
		if (tt != nil)
		{
			if (tt->GetDF()->GetNbRecords(nil) == 0)
			{
				BaseTaskInfo* context = new BaseTaskInfo(db, nil, nil, nil);
				FicheInMem* rec = new FicheInMem(context, db, tt, err);
				if (rec != nil)
				{
					VValueSingle* cv = rec->GetNthField(1, err);
					if (cv != nil)
						cv->FromString(L"DEFAULT_SCHEMA");
					rec->Touch(1);

					cv = rec->GetNthField(2, err);
					if (cv != nil)
						cv->FromLong(1);

					rec->Touch(2);

					err = tt->GetDF()->SaveRecord(rec, context);
					rec->Release();

					VString name(L"DEFAULT_SCHEMA");
					sLONG id = 1;
					CDB4DSchema* schema = new VDB4DSchema(this, name, id);
					fSchemasByName[name] = schema;
					fSchemasByID[id] = schema;
					schema->Release();
				}

				context->Release();
			}
			tt->Release();
		}
	}
	return err;
}


VError Base4D::LoadSchemas()
{
	StErrorContextInstaller errs(false);
	VError err = VE_OK;
	if (fStructure != nil)
	{
		Base4D* db = fStructure;

		Table* table_schema = db->RetainTable(2);
		if (table_schema != nil)
		{
			BaseTaskInfo* context = new BaseTaskInfo(db, nil, nil, nil);
			Selection* sel = table_schema->GetDF()->AllRecords(context, err);
			if (sel != nil)
			{
				SelectionIterator itersel(sel);
				sLONG n = itersel.FirstRecord();
				while (n != -1)
				{
					FicheInMem* rec = table_schema->GetDF()->LoadRecord(n, err, DB4D_Do_Not_Lock, context);
					if (rec != nil)
					{
						VValueSingle* cv = rec->GetNthField(1, err);
						VValueSingle* cv2 = rec->GetNthField(2, err);

						if (cv != nil && cv2 != nil)
						{
							VString name;
							cv->GetString(name);
							sLONG id = cv2->GetLong();

							CDB4DSchema* schema = new VDB4DSchema(this, name, id);
							fSchemasByName[name] = schema;
							fSchemasByID[id] = schema;
							schema->Release();
						}

						rec->Release();
					}
					n = itersel.NextRecord();
				}
				sel->Release();
			}

			context->Release();
			table_schema->Release();
		}

	}

	err = VE_OK;
	return err;
}

VError Base4D::CreateSchema( const VValueBag& inBag, VBagLoader *inLoader, CDB4DSchema **outSchema, CDB4DBaseContext* inContext)
{
	VError			err = VE_OK;
	CDB4DSchema		*schema = NULL;

	VString			name;
	inBag.GetString( DB4DBagKeys::name, name);
	if ( !name. EqualToString ( "DEFAULT_SCHEMA" ) )
		CDB4DSchema *schema = CreateSchema (name, ConvertContext(inContext), err);

	if (outSchema != NULL)
		*outSchema = schema;
	else if (schema != NULL)
		schema->Release();

	return err;
}

CDB4DSchema* Base4D::CreateSchema(const VString& inName, BaseTaskInfo* context, VErrorDB4D& outError)
{
	CDB4DSchema* schema = nil;
	Boolean ok = false;
	outError = VE_OK;
	if (fStructure != nil)
	{
		if ( inName. EqualToString ( "SYSTEM_SCHEMA" ) )
			outError = ThrowError(VE_DB4D_DUPLICATED_SCHEMA_NAME, noaction);
		else
		{
			VTaskLock lock(&fSchemaMutex);
			BuildFirstSchemaIfNeeded();

			Base4D* db = fStructure;
			BaseTaskInfo* context2 = new BaseTaskInfo(db, nil, nil, nil);
			SchemaCatalogByName::iterator deja = fSchemasByName.find(inName);
			if (deja == fSchemasByName.end())
			{
				if (fStoreAsXML)
				{
					sLONG id = 1;
					if (!fSchemasByID.empty())
						id = fSchemasByID.rbegin()->first + 1;
					schema = new VDB4DSchema(this, inName, id);
					if (schema != nil)
					{
						fSchemasByName[inName] = schema;
						fSchemasByID[id] = schema;
						Touch();
						TouchSchemasCatalogStamp();
						ok = true;
					}
				}
				else
				{
					Table* table_schema = db->RetainTable(2);
					if (table_schema != nil)
					{
						FicheInMem* rec_schema = new FicheInMem(context2, db, table_schema, outError);
						if (rec_schema == nil)
							outError = ThrowBaseError(memfull);
						else
						{
							VValueSingle* cv = rec_schema->GetNthField(1, outError);
							if (cv != nil)
							{
								cv->FromString(inName);
								rec_schema->Touch(1);

								outError = table_schema->GetDF()->SaveRecord(rec_schema, context2);

								if (outError == VE_OK)
								{
									cv = rec_schema->GetNthField(2, outError);
									if (cv != nil)
									{
										DB4D_SchemaID id = cv->GetLong();
										xbox_assert(id>1);
										schema = new VDB4DSchema(this, inName, id);
										if (schema != nil)
										{
											fSchemasByName[inName] = schema;
											fSchemasByID[id] = schema;
											Touch();
											TouchSchemasCatalogStamp();
											ok = true;
										}
									}
								}
							}
							rec_schema->Release();
						}

						table_schema->Release();
					}
				}
			}
			else
				outError = ThrowError(VE_DB4D_DUPLICATED_SCHEMA_NAME, noaction);

			context2->Release();
		}
	}

	if (outError != VE_OK || !ok)
		outError = ThrowError(VE_DB4D_CANNOT_SAVE_SCHEMA, noaction);

	return schema;
}


VErrorDB4D Base4D::DropSchema(CDB4DSchema* inSchema, BaseTaskInfo* context)
{
	VError outError = VE_OK;
	DB4D_SchemaID id = 0;
	if (fStructure != nil && inSchema != nil)
	{
		id = inSchema->GetID();
		if (id == 1 || id == 0)
			outError = VE_DB4D_CANNOT_DROP_SCHEMA; /* Should not drop default or system schemas. */
		else
		{
			inSchema->UnlockProperties(context->GetEncapsuleur());
			Base4D* db = fStructure;
			BaseTaskInfo* context2 = new BaseTaskInfo(db, nil, nil, nil);
			Table* table_schema = db->RetainTable(2);
			if (table_schema != nil)
			{
				VTaskLock lock(&fSchemaMutex);

				SearchTab rech(table_schema);
				VLong cvid(id);
				rech.AddSearchLineSimple(2, 2, DB4D_Equal, &cvid);
				OptimizedQuery query;
				query.AnalyseSearch(&rech, context2);

				Selection* sel = query.Perform((Bittab*)nil, nil, context2, outError, DB4D_Do_Not_Lock);
				if (sel != nil)
				{
					outError = sel->DeleteRecords(context2, nil);
					if (outError == VE_OK)
					{
						fSchemasByID.erase(id);
						fSchemasByName.erase(inSchema->GetName());
						Touch();
						TouchSchemasCatalogStamp();
					}
					sel->Release();
				}

				table_schema->Release();
			}


			sLONG nbtable = GetNBTable();
			for (sLONG i = 1; i <= nbtable; i++)
			{
				Table* tt = RetainTable(i);
				if (tt != nil)
				{
					if (tt->GetSchema() == id)
						tt->SetSchema( 1, context->GetEncapsuleur());
					tt->Release();
				}
			}
			context2->Release();
		}
	}

	if (outError != VE_OK)
		outError = ThrowError(VE_DB4D_CANNOT_DROP_SCHEMA, noaction);

	return outError;
}



VErrorDB4D Base4D::RenameSchema(CDB4DSchema* inSchema, const VString& inNewName, BaseTaskInfo* context)
{
	VError outError = VE_OK;
	DB4D_SchemaID id = 0;
	if (fStructure != nil && inSchema != nil)
	{
		id = inSchema->GetID();
		if (id == 1 || id == 0 || inNewName. EqualToString ( "SYSTEM_SCHEMA" ) )
			outError = VE_DB4D_CANNOT_RENAME_SCHEMA; /* Should not rename default or system schemas. */
		else
		{
			inSchema->UnlockProperties(context->GetEncapsuleur());
			Base4D* db = fStructure;
			BaseTaskInfo* context2 = new BaseTaskInfo(db, nil, nil, nil);
			Table* table_schema = db->RetainTable(2);
			if (table_schema != nil)
			{
				VTaskLock lock(&fSchemaMutex);

				SearchTab rech(table_schema);
				VLong cvid(id);
				rech.AddSearchLineSimple(2, 2, DB4D_Equal, &cvid);
				OptimizedQuery query;
				query.AnalyseSearch(&rech, context2);

				Selection* sel = query.Perform((Bittab*)nil, nil, context2, outError, DB4D_Do_Not_Lock);
				if (sel != nil)
				{
					if (sel->GetQTfic() > 0)
					{
						FicheInMem* rec = table_schema->GetDF()->LoadRecord(sel->GetFic(0), outError, DB4D_Keep_Lock_With_Record, context2);
						if (rec != nil)
						{
							VValueSingle* cv = rec->GetNthField(1, outError);
							if (cv != nil)
							{
								cv->FromString(inNewName);
								rec->Touch(1);
								outError = table_schema->GetDF()->SaveRecord(rec, context2);
							}
							rec->Release();
							if (outError == VE_OK)
							{
								fSchemasByName.erase(inSchema->GetName());
								dynamic_cast<VDB4DSchema*>(inSchema)->SetName(inNewName);
								fSchemasByName[inSchema->GetName()] = inSchema;
								Touch();
								TouchSchemasCatalogStamp();
							}
						}
					}
					sel->Release();
				}

				table_schema->Release();
			}

			context2->Release();
		}
	}

	if (outError != VE_OK)
		outError = ThrowError(VE_DB4D_CANNOT_DROP_SCHEMA, noaction);

	return outError;
}


VErrorDB4D Base4D::RetainAllSchemas(std::vector<VRefPtr<CDB4DSchema> >& outSchemas)
{
	VTaskLock lock(&fSchemaMutex);
	BuildFirstSchemaIfNeeded();
	outSchemas.reserve(fSchemasByID.size());
	for (SchemaCatalogByID::iterator cur = fSchemasByID.begin(), end = fSchemasByID.end(); cur != end; cur++)
	{
		outSchemas.push_back(cur->second);
	}

	return VE_OK;
}


CDB4DSchema* Base4D::RetainSchema(const VString& inSchemaName)
{
	VTaskLock lock(&fSchemaMutex);
	CDB4DSchema* schema = nil;
	BuildFirstSchemaIfNeeded();
	
	SchemaCatalogByName::iterator found = fSchemasByName.find(inSchemaName);
	if (found != fSchemasByName.end())
	{
		schema = found->second;
		schema->Retain();
	}

	return schema;
}


CDB4DSchema* Base4D::RetainSchema(DB4D_SchemaID inSchemaID)
{
	VTaskLock lock(&fSchemaMutex);
	CDB4DSchema* schema = nil;
	BuildFirstSchemaIfNeeded();

	SchemaCatalogByID::iterator found = fSchemasByID.find(inSchemaID);
	if (found != fSchemasByID.end())
	{
		schema = found->second;
		schema->Retain();
	}

	return schema;
}


#if debug_Addr_Overlap


void Base4D::SetDBObjRef(DataAddr4D addr, sLONG len, debug_dbobjref* dbobjref, bool checkConflict)
{
	VTaskLock lock(&fDBObjRefMutex);
	sLONG segnum = addr & (kMaxSegData-1);
	addr = addr & (DataAddr4D)(-kMaxSegData);

	Debug_dbobjrefMap* refmap = &(fDBObjRef[segnum]);
	debug_DataAddr daddr(addr, len);

	Debug_dbobjrefMap::iterator found = refmap->find(daddr);
	if (found != refmap->end())
	{
		if (!dbobjref->EqualTo(*(found->second)))
		{
			xbox_assert(false);
		}
		delete found->second;
	}
	else
	{
		if (checkConflict)
		{
			Debug_dbobjrefMap::iterator limitsup = refmap->lower_bound(daddr);
			if (limitsup != refmap->end())
			{
				if (addr+len > limitsup->first.fAddr)
				{
					xbox_assert(false);
				}
				if (limitsup != refmap->begin())
				{
					Debug_dbobjrefMap::iterator limitinf = limitsup;
					limitinf--;
					if (limitinf->first.fAddr + limitinf->first.fLen > addr)
					{
						xbox_assert(false);
					}
				}
			}
		}
	}
	(*refmap)[daddr] = dbobjref;
}


void Base4D::DelDBObjRef(DataAddr4D addr, sLONG len)
{
	VTaskLock lock(&fDBObjRefMutex);
	sLONG segnum = addr & (kMaxSegData-1);
	addr = addr & (DataAddr4D)(-kMaxSegData);
	fDBObjRef[segnum].erase(debug_DataAddr(addr, len));
}


void Base4D::ChangeDBObjRefLength(DataAddr4D addr, sLONG len, const debug_dbobjref& dbobjref)
{
	VTaskLock lock(&fDBObjRefMutex);
	sLONG segnum = addr & (kMaxSegData-1);
	addr = addr & (DataAddr4D)(-kMaxSegData);

	Debug_dbobjrefMap* refmap = &(fDBObjRef[segnum]);
	debug_DataAddr daddr(addr, len);

	Debug_dbobjrefMap::iterator found = refmap->find(daddr);
	if (found != refmap->end())
	{
		if (found->second->EqualTo(dbobjref))
		{
			Debug_dbobjrefMap::iterator next = found;
			next++;
			if (next != refmap->end())
			{
				if (addr+len > next->first.fAddr)
				{
					xbox_assert(false);
				}
			}
			debug_DataAddr* paddr = (debug_DataAddr*)(&(found->first));
			paddr->fLen = len;
		}
		else
		{
			xbox_assert(false);
		}
	}
	else
	{
		xbox_assert(false);
	}
}


void Base4D::FillDebug_DBObjRef()
{
	VTaskLock lock(&fDBObjRefMutex);
	for (DataTableVector::iterator cur = fDataTables.begin(), end = fDataTables.end(); cur != end; cur++)
	{
		DataTable* df = *cur;
		if (df != nil)
		{
			df->FillDebug_DBObjRef();
		}
	}
}


void Base4D::CheckDBObjRef(DataAddr4D addr, sLONG len, const debug_dbobjref& dbobjref)
{
	VTaskLock lock(&fDBObjRefMutex);
	sLONG segnum = addr & (kMaxSegData-1);
	addr = addr & (DataAddr4D)(-kMaxSegData);

	Debug_dbobjrefMap* refmap = &(fDBObjRef[segnum]);
	debug_DataAddr daddr(addr, len);

	Debug_dbobjrefMap::iterator found = refmap->find(daddr);
	if (found != refmap->end())
	{
		if (found->second->EqualTo(dbobjref))
		{
			Debug_dbobjrefMap::iterator next = found;
			next++;
			if (next != refmap->end())
			{
				if (addr+len > next->first.fAddr)
				{
					xbox_assert(false);
				}
			}
		}
		else
		{
			xbox_assert(false);
		}
	}
	else
	{
		//xbox_assert(false);
		addr = addr; // break here
	}
}


#endif


#if debugOverlaps_strong

Boolean Base4D::Debug_CheckAddrOverlap(DataAddr4D addrToCheck, sLONG lenToCheck, sLONG numtableToCheck, sLONG numblobToCheck, sLONG numrecToCheck)
{
	Boolean result = false;
	for (DataTableVector::iterator cur = fDataTables.begin(), end = fDataTables.end(); cur != end; cur++)
	{
		DataTable* df = *cur;
		if (df != nil)
		{
			sLONG xnumblobToCheck;
			sLONG xnumrecToCheck;
			if (numtableToCheck == df->GetTrueNum())
			{
				xnumblobToCheck = numblobToCheck;
				xnumrecToCheck = numrecToCheck;
			}
			else
			{
				xnumblobToCheck = -1;
				xnumrecToCheck = -1;
			}
			result = result || df->Debug_CheckAddrOverlap(addrToCheck, lenToCheck, xnumblobToCheck, xnumrecToCheck);
		}
	}
	return result;
}

#endif

#if debuglr

void Base4D::checkIndexInMap(IndexInfo* ind)
{
	LockIndexes();
	for (IndexMap::iterator cur = fAllIndexes.begin(), end = fAllIndexes.end(); cur != end; cur++)
	{
		if (cur->second == ind)
		{
			xbox_assert(false);
			ind = ind;
		}
	}

	UnLockIndexes();
}

#endif



VError Base4D::ImportRecords(BaseTaskInfo* context, VFolder* inBaseFolder, VProgressIndicator* inProgress, ExportOption& options)
{
	VError err = VE_OK;
	
	{
		VString foldername;
		if (options.JSONExport)
			foldername = L"JsonExport";
		else if (options.BinaryExport)
			foldername = L"BinaryExport";
		else
			foldername = L"SQLExport";
		VFolder		subfolder (*inBaseFolder, foldername);
		if (options.CreateFolder)
		{
			inBaseFolder = &subfolder;
		}
				
		if (err == VE_OK)
		{
			if (inProgress != nil)
			{
				XBOX::VString session_title;
				gs(1005,32,session_title);
				inProgress->BeginSession(GetNBTable(),session_title,true);
			}

			err = StartDataConversion();
			if (err == VE_OK)
			{
				//DelayAllIndexes();

				for (sLONG i = 1, nb = GetNBTable(); i <= nb && err == VE_OK; i++)
				{
					if (inProgress != nil)
						inProgress->Increment();
					Table* t = RetainTable(i);
					if (t != nil)
					{
						VString tablename;
						t->GetName(tablename);
						tablename.ExchangeAll(CHAR_COLON, CHAR_LOW_LINE); /* Sergiy - 25 May 2009 */
						VFolder* folder = new VFolder(*inBaseFolder, tablename);

						if (folder->Exists())
						{
							options.CreateFolder = false;
							options.ChangeIntegrityRules = false;
							options.DelayIndexes = true;
							err = t->ImportRecords(folder, context, inProgress, options);
						}
						
						folder->Release();
						t->Release();
					}
				}

				StopDataConversion(context, inProgress, true);
				/*
				vector<IndexInfo*> indexes;
				AwakeAllIndexes( nil, true, indexes);
				for (vector<IndexInfo*>::iterator cur = indexes.begin(), end = indexes.end(); cur != end; cur++)
					(*cur)->Release();
					*/
			}

			if (inProgress != nil)
			{
				inProgress->EndSession();
			}
		}
	}
	
	return err;
}


VError Base4D::StartDataConversion()
{
	VError err = VE_OK;
	if (LogIsOn())
	{
		err = ThrowError(VE_DB4D_CONSTRAINTS_CANNOT_REMOVED_WITH_LOG, noaction);
	}
	else
	{
		VTaskLock lock(&fNoIntegrityRequestCountMutex);
		if (fNoIntegrityRequestCount == 0)
		{
			sLONG nb = (sLONG)fDataTables.size();
			fModifiedPrimKeyCount.resize(nb);
			for (sLONG i = 0; i < nb; ++i)
			{
				DataTableRegular* dt;
				dt = fDataTables[i];
				if (dt != nil)
				{
					fModifiedPrimKeyCount[i] = dt->GetNbPrimKeyUpdates();
				}
			}
			SetReferentialIntegrityEnabled(false);
			SetCheckUniquenessEnabled(false);
			SetCheckNot_NullEnabled(false);
		}
		++fNoIntegrityRequestCount;
	}
	ReleaseLogMutex();
	return err;
}

void Base4D::StopDataConversion(BaseTaskInfo* context, VProgressIndicator* progress, bool MustCheckPrimKeyValidity)
{
	VTaskLock lock(&fNoIntegrityRequestCountMutex);
	if (testAssert(fNoIntegrityRequestCount > 0))
	{
		--fNoIntegrityRequestCount;
		if (fNoIntegrityRequestCount == 0)
		{
			SetReferentialIntegrityEnabled(true);
			SetCheckUniquenessEnabled(true);
			SetCheckNot_NullEnabled(true);

			if (MustCheckPrimKeyValidity)
			{
				sLONG nb = (sLONG)(min(fDataTables.size(), fModifiedPrimKeyCount.size()));
				for (sLONG i = 0; i < nb; ++i)
				{
					DataTableRegular* dt;
					dt = fDataTables[i];
					if (dt != nil)
					{
						if (fModifiedPrimKeyCount[i] != dt->GetNbPrimKeyUpdates())
						{
							StErrorContextInstaller errs(false);
							VError err = VE_OK;
							dt->CheckPrimaryKeyValid(context, progress, err, true);
						}
					}
				}
			}
		}
	}
	else
	{
		sLONG xdebug = 1; // put a break here
	}
}


VError Base4D::ExportToSQL(BaseTaskInfo* context, VFolder* inBaseFolder, VProgressIndicator* inProgress, ExportOption& options)
{
	VError err = VE_OK;
	
	{
		VString foldername;
		if (options.JSONExport)
			foldername = L"JsonExport";
		else if (options.BinaryExport)
			foldername = L"BinaryExport";
		else
			foldername = L"SQLExport";
		VFolder		subfolder (*inBaseFolder, foldername);

		if (options.CreateFolder)
		{
			if (!inBaseFolder->Exists())
			{
				err = inBaseFolder->Create();
			}
			inBaseFolder = &subfolder;
		}

		if (inBaseFolder->Exists())
		{
			XBOX::VString sessionTitle;
			gs(1005,33,sessionTitle);
			if (inProgress != nil)
				inProgress->BeginUndeterminedSession(sessionTitle);
			err = inBaseFolder->DeleteContents(true);
			if (inProgress != nil)
				inProgress->EndSession();
		}
		else
			err = inBaseFolder->Create();
		
		if (err == VE_OK)
		{
			if (inProgress != nil)
			{
				XBOX::VString session_title;
				gs(1005,32,session_title);
				inProgress->BeginSession(GetNBTable(),session_title,true);
			}

			for (sLONG i = 1, nb = GetNBTable(); i <= nb && err == VE_OK; i++)
			{
				if (inProgress != nil)
					inProgress->Increment();
				Table* t = RetainTable(i);
				if (t != nil)
				{
					VString tablename;
					t->GetName(tablename);
					tablename.ExchangeAll(CHAR_COLON, CHAR_LOW_LINE); /* Sergiy - 25 May 2009 */
					VFolder* folder = new VFolder(*inBaseFolder, tablename);

					CDB4DSelection* xsel = nil;
					Selection* sel;
					if (fIsRemote)
					{
						CDB4DBase* xbase = RetainBaseX();
						CDB4DTable* xtt = new VDB4DTable(VDBMgr::GetManager(), dynamic_cast<VDB4DBase*>(xbase), t);
						xsel = xtt->SelectAllRecords(context->GetEncapsuleur(), &err, DB4D_Do_Not_Lock, nil);
						sel = dynamic_cast<VDB4DSelection*>(xsel)->GetSel();
						sel->Retain();
						QuickReleaseRefCountable(xbase);
						QuickReleaseRefCountable(xtt);
					}
					else
						sel = t->GetDF()->AllRecords(context, err);
					if (sel != nil)
					{
						options.CreateFolder = false;
						err = sel->ExportToSQL(xsel, t, context, folder, inProgress, options);
						sel->Release();
					}
					QuickReleaseRefCountable(xsel);
					folder->Release();
					t->Release();
				}
			}

			if (inProgress != nil)
			{
				inProgress->EndSession();
			}
		}
	}
	
	return err;
}


ExtraElement* Base4D::RetainExtraElement(sLONG n, OccupableStack* curstack, VError& err, bool* outNeedSwap)
{
	ExtraElement* result = nil;
	err = VE_OK;

	// pour Jean
	if (true)
	{
		if (n == 0)
			return result;
	}

	if (fExtraElements != nil)
	{
		result = fExtraElements->RetainElement(n, curstack, err, outNeedSwap);
	}
	else
		err = VE_UNIMPLEMENTED;
	return result;
}


bool Base4D::SomeTablesCannotBeLogged(VJSONObject* ioParams)
{
	bool allOK = true;
	for (TableArray::Iterator cur = fich.First(), end = fich.End(); cur != end; ++cur)
	{
		Table* tt = *cur;
		if (tt != nil)
		{
			DataTable* df = tt->GetDF();
			if (df != nil)
			{
				uBYTE validityState = df->LogInfoValidityState();
				if (validityState != PrimKeyState_Valid)
				{
					allOK = false;
					if (ioParams != nil)
					{
						VJSONObject* ttobj = tt->AddToJSON("tables", ioParams);
						ttobj->SetPropertyAsNumber("primaryKeyValidityState", validityState);
					}
				}
			}
		}
	}
	return !allOK;
}


bool Base4D::CheckPrimaryKeysValidity(BaseTaskInfo* context, VProgressIndicator* progress, VError& outErr, VJSONObject* ioParams, bool forceCheck)
{
	outErr = VE_OK;
	bool allOK = true;

	occupe();
	TableArray alltables;
	sLONG nbtable = fich.GetCount();
	alltables.SetCount(nbtable);
	for (sLONG i = 1; i <= nbtable; ++i)
	{
		alltables[i] = RetainRefCountable(fich[i]);
	}
	libere();

	for (TableArray::Iterator cur = alltables.First(), end = alltables.End(); cur != end && outErr == VE_OK; ++cur)
	{
		Table* tt = *cur;
		if (tt != nil)
		{
			DataTable* df = tt->GetDF();
			if (df != nil)
			{
				if (!df->CheckPrimaryKeyValid(context, progress, outErr, forceCheck))
				{
					VJSONObject* ttobj = tt->AddToJSON("tables", ioParams);
					if (outErr == VE_DB4D_PRIMARYKEY_IS_NOT_UNIQUE)
					{
						if (ttobj != nil)
							ttobj->SetPropertyAsNumber("primaryKeyValidityState", (int)PrimKeyState_NonUnique);
						outErr = VE_OK;
					}
					else if (outErr == VE_DB4D_PRIMARYKEY_IS_NULL)
					{
						if (ttobj != nil)
							ttobj->SetPropertyAsNumber("primaryKeyValidityState", (int)PrimKeyState_SomeNULLs);
						outErr = VE_OK;
					}
					else if (outErr == VE_DB4D_PRIMARYKEY_IS_MISSING)
					{
						if (ttobj != nil)
							ttobj->SetPropertyAsNumber("primaryKeyValidityState", (int)PrimKeyState_MissingPrimKeyDef);
						outErr = VE_OK;
					}
				}
			}
		}
	}

	for (TableArray::ConstIterator iter = alltables.First(), end = alltables.End() ; iter != end ; ++iter)
	{
		Table* table = *iter;
		QuickReleaseRefCountable(table);
	}

	return allOK;
}


void Base4D::GetPrimaryKeysValidityState( BaseTaskInfo* inContext, VJSONObject* ioParams) const
{
	occupe();
	TableArray alltables;
	sLONG nbtable = fich.GetCount();
	alltables.SetCount(nbtable);
	for (sLONG i = 1; i <= nbtable; ++i)
	{
		alltables[i] = RetainRefCountable(fich[i]);
	}
	libere();

	for (TableArray::ConstIterator iter = alltables.First(), end = alltables.End() ; iter != end ; ++iter)
	{
		Table* table = *iter;
		if (table != nil)
		{
			sWORD state = table->GetPrimaryKeyValidityState();
			VJSONObject* tableObj = table->AddToJSON("tables", ioParams);
			if (tableObj != nil)
			{
				tableObj->SetPropertyAsNumber("number", table->GetNum());
				tableObj->SetPropertyAsNumber("primaryKeyValidityState", table->GetPrimaryKeyValidityState());
			}
		}
	}

	for (TableArray::ConstIterator iter = alltables.First(), end = alltables.End() ; iter != end ; ++iter)
	{
		Table* table = *iter;
		QuickReleaseRefCountable(table);
	}

}


VError Base4D::SetIntlInfo(const VString& stringCompHash, const VString& keywordBuildingHash)
{
	VError err = VE_OK;
	VValueBag* bag = fExtra.RetainExtraProperties(err);
	if (err == VE_OK)
	{
		if (bag == nil)
			bag = new VValueBag();
		bag->SetString("__stringCompHash", stringCompHash);
		bag->SetString("__keywordBuildingHash", keywordBuildingHash);
		err = fExtra.SetExtraProperties(bag);
		if (err == VE_OK)
			setmodif( true, this, nil);
	}
	QuickReleaseRefCountable(bag);
	return err;

}

VError Base4D::GetIntlInfo(VString& outStringCompHash, VString& outKeywordBuildingHash)
{
	outStringCompHash.Clear();
	outKeywordBuildingHash.Clear();
	VError err = VE_OK;
	VValueBag* bag = fExtra.RetainExtraProperties(err);
	if (bag != nil)
	{
		bag->GetString("__stringCompHash", outStringCompHash);
		bag->GetString("__keywordBuildingHash", outKeywordBuildingHash);
	}
	if (outStringCompHash.IsEmpty())
		outStringCompHash = "icu:4.8.0";
	if (outKeywordBuildingHash.IsEmpty())
		outKeywordBuildingHash = "icu:4.8.0";
	QuickReleaseRefCountable(bag);
	return err;
}


void Base4D::HitCache(sLONG nbHits, VSize howManyBytes)
{
	VTaskLock lock(&fMeasureMutex);
	fCacheHitBytes.AddMeasure(howManyBytes);
	fCacheHitCount.AddMeasure(nbHits);
}

void Base4D::MissCache(sLONG nbMisses, VSize howManyBytes)
{
	VTaskLock lock(&fMeasureMutex);
	fCacheMissBytes.AddMeasure(howManyBytes);
	fCacheMissCount.AddMeasure(nbMisses);
}

void Base4D::HasRead(sLONG nbReads, VSize howManyBytes)
{
	VTaskLock lock(&fMeasureMutex);
	fDiskReadBytes.AddMeasure(howManyBytes);
	fDiskReadCounts.AddMeasure(nbReads);
}


void Base4D::HasWritten(sLONG nbWrites, VSize howManyBytes)
{
	VTaskLock lock(&fMeasureMutex);
	fDiskWriteBytes.AddMeasure(howManyBytes);
	fDiskWriteCounts.AddMeasure(nbWrites);
}


VJSONObject* Base4D::RetainMeasures(VJSONObject* options, JSONPath* path)
{
	VJSONObject* resultTop = new VJSONObject();

	VString dbmarker("DB");
	JSONPathMarker pathDB(path);
	if (pathDB.AcceptJsonPathPart(dbmarker))
	{
		VJSONObject* result = nil;
		JSONPathMarker path1(path);

		if (path1.AcceptJsonPathPart(MeasureCollection::kDiskReadBytes))
			jsonProp(&result, MeasureCollection::kDiskReadBytes, fDiskReadBytes.RetainMeasures(options));

		if (path1.AcceptJsonPathPart(MeasureCollection::kDiskWriteBytes))
			jsonProp(&result, MeasureCollection::kDiskWriteBytes, fDiskWriteBytes.RetainMeasures(options));

		if (path1.AcceptJsonPathPart(MeasureCollection::kDiskReadCount))
			jsonProp(&result, MeasureCollection::kDiskReadCount, fDiskReadCounts.RetainMeasures(options));

		if (path1.AcceptJsonPathPart(MeasureCollection::kDiskWriteCount))
			jsonProp(&result, MeasureCollection::kDiskWriteCount, fDiskWriteCounts.RetainMeasures(options));

		if (path1.AcceptJsonPathPart(MeasureCollection::kCacheHitBytes))
			jsonProp(&result, MeasureCollection::kCacheHitBytes, fCacheHitBytes.RetainMeasures(options));

		if (path1.AcceptJsonPathPart(MeasureCollection::kCacheMissBytes))
			jsonProp(&result, MeasureCollection::kCacheMissBytes, fCacheMissBytes.RetainMeasures(options));

		if (path1.AcceptJsonPathPart(MeasureCollection::kCacheHits))
			jsonProp(&result, MeasureCollection::kCacheHits, fCacheHitCount.RetainMeasures(options));

		if (path1.AcceptJsonPathPart(MeasureCollection::kCacheMisses))
			jsonProp(&result, MeasureCollection::kCacheMisses, fCacheMissCount.RetainMeasures(options));

		sLONG curseg = 0;
		for (VArrayOf<SegData*>::Iterator cur = fSegments.First(), end = fSegments.End(); cur != end; ++cur)
		{
			++curseg;
			VString spart = "dataSegment" + ToString(curseg);
			if (path1.AcceptJsonPathPart(spart))
			{
				jsonProp(&result, spart, (*cur)->RetainMeasures(options, path));
				/*
				VJSONObject* segmeasure = (*cur)->RetainMeasures(options, path);
				result->SetProperty(spart, VJSONValue(segmeasure));
				segmeasure->Release();
				*/
			}
		}
		if (fIndexSegment != nil)
		{
			VString spart("indexSegment");
			if (path1.AcceptJsonPathPart(spart))
			{
				jsonProp(&result, spart, fIndexSegment->RetainMeasures(options, path));
				/*
				VJSONObject* segmeasure = fIndexSegment->RetainMeasures(options, path);
				result->SetProperty(spart, VJSONValue(segmeasure));
				segmeasure->Release();
				*/
			}
		}

		VString sparttable("tables");
		if (path1.AcceptJsonPathPart(sparttable))
		{
			JSONPathMarker pathtable(path);
			VJSONObject* tableobj = nil;

			DataTableVector alltables;
			occupe();
			alltables.reserve(fDataTables.size());
			for (DataTableVector::iterator cur = fDataTables.begin(), end = fDataTables.end(); cur != end; ++cur)
			{
				alltables.push_back(RetainRefCountable(*cur));
			}
			libere();

			for (DataTableVector::iterator cur = alltables.begin(), end = alltables.end(); cur != end; ++cur)
			{
				DataTable* df = *cur;
				if (df != nil)
				{
					VString tname;
					Table* tt = df->GetTable();
					if (tt != nil)
					{
						tt->GetName(tname);
						if (pathtable.AcceptJsonPathPart(tname))
						{
							jsonProp(&tableobj, tname, df->RetainMeasures(options, path));
						}
					}
					df->Release();
				}
			}
			jsonProp(&result, sparttable, tableobj);
		}

		VString spartindex("indexes");
		if (path1.AcceptJsonPathPart(spartindex))
		{
			JSONPathMarker pathindex(path);
			VJSONObject* indexobj = nil;

			vector<IndexInfo*> lesindex;
			LockIndexes();
			for (IndexMap::const_iterator cur = fAllIndexes.begin(), end = fAllIndexes.end(); cur != end; cur++)
			{
				IndexInfo* ind = cur->second;
				ind->Retain();
				lesindex.push_back(ind);
			}
			UnLockIndexes();

			for (vector<IndexInfo*>::iterator cur = lesindex.begin(), end = lesindex.end(); cur != end; cur++)
			{
				IndexInfo* ind = *cur;
				Table* tt = ind->GetTargetTable();
				if (tt != nil)
				{
					VString tname;
					tt->GetName(tname);
					if (pathindex.AcceptJsonPathPart(tname))
					{
						VJSONObject* tableobj = nil;
						if (indexobj != nil)
						{
							VJSONValue val = indexobj->GetProperty(tname);
							if (val.IsObject())
								tableobj = RetainRefCountable(val.GetObject());
						}
						JSONPathMarker pathtable(path);
						VString indexname;
						ind->IdentifyIndex(indexname, false, false, true);
						if (pathtable.AcceptJsonPathPart(indexname))
						{
							if (ind->GetHeader() != nil)
								jsonProp(&tableobj, indexname, ind->GetHeader()->RetainMeasures(options, path));
						}
						if (tableobj != nil)
						{
							jsonProp(&indexobj, tname, tableobj);
						}
					}
				}

				ind->Release();
			}

			jsonProp(&result, spartindex, indexobj);
		}
		jsonProp(resultTop, dbmarker, result);
	}
	return resultTop;
}


void Base4D::SetMeasureInterval(uLONG interval)
{
	fCacheHitBytes.SetInterval(interval);
	fCacheHitCount.SetInterval(interval);
	fCacheMissCount.SetInterval(interval);
	fCacheMissBytes.SetInterval(interval);

	for (VArrayOf<SegData*>::Iterator cur = fSegments.First(), end = fSegments.End(); cur != end; ++cur)
	{
		(*cur)->SetMeasureInterval(interval);
	}
	if (fIndexSegment != nil)
	{
		fIndexSegment->SetMeasureInterval(interval);
	}

}








			/* ------------------------------------------------------------- */
			






Base4D_NotOpened::Base4D_NotOpened()
{
	fIsOpen = false;
	fIsFullyOpen = false;
	FHeaderIsValid = false;
	fValidVersionNumber = false;
	FAllSegsIDMatch = false;
	fIsDataTablesHeaderValid = false;
	fIsDataTablesHeaderFullyValid = false;
	fAreDataTablesLoaded = false;
	fSeqNumsAreLoaded = false;
	fIndexes = nil;
	fIndexesInStruct = nil;
	fSeqNums = nil;
	fTableDefs = nil;
	fRelationDefs = nil;
	fDataTableChecker = nil;
	fStruct = nil;
	fDatas = nil;
	fAttachedBase = nil;
	fAttachedContext = nil;
	fID.FromLong(0);
	libere();
}


Base4D_NotOpened::~Base4D_NotOpened()
{
	if (fStruct != nil)
	{
		if (fDatas != nil)
			delete fDatas;
	}
	if (fIndexes != nil)
		delete fIndexes;
	if (fIndexesInStruct != nil)
		delete fIndexesInStruct;
	if (fSeqNums != nil)
		delete fSeqNums;
	if (fTableDefs != nil)
		delete fTableDefs;
	if (fRelationDefs != nil)
		delete fRelationDefs;
	if (fDataTableChecker != nil)
		delete fDataTableChecker;
	Close();
}

VError Base4D_NotOpened::writelong(void* p, sLONG len, DataAddr4D ou, sLONG offset, VString* WhereFrom)
{
	VError err = VE_OK;
	return err;
}

VError Base4D_NotOpened::readlong(void* p, sLONG len, DataAddr4D ou, sLONG offset)
{
	VError err = VE_OK;
	sLONG i = (ou & (kMaxSegData-1)) + 1;
	
	SegDataMap::iterator found = segs.find(i);
	if (found != segs.end())
	{
		ou = ou & (DataAddr4D)(-kMaxSegData);
		found->second->readat(p, len, ou+(DataAddr4D)offset);
	}
	else
		err = VE_DB4D_WRONGDATASEG;

	return err;
}


DataAddr4D Base4D_NotOpened::SubGetAddrFromTable(sLONG n, DataAddr4D addr, sLONG maxnbelem, sLONG nbelem, VError& err, 
												 sLONG pos1, sLONG pos2, Boolean TestRegularSeg, sLONG* outLen, bool expectingStamps)
{
	TabAddrDisk taddr;
	DataAddr4D result = 0;

	VError errexec = ReadAddressTable(addr, taddr, pos1, pos2, nil, nil, err, 0, TestRegularSeg, expectingStamps);
	if (err == VE_OK)
	{
		if (maxnbelem > kNbElemTabAddr2)
		{
			sLONG n1 = n / kNbElemTabAddr2;
			sLONG n2 = n & (kNbElemTabAddr2-1);
			result = SubGetAddrFromTable(n2, taddr[n1].addr, kNbElemTabAddr2-1, kNbElemTabAddr2-1, err, n1, -1, TestRegularSeg, outLen, expectingStamps);

		}
		else
		{
			if (maxnbelem> kNbElemTabAddr)
			{
				sLONG n1 = n / kNbElemTabAddr;
				sLONG n2 = n & (kNbElemTabAddr-1);
				result = SubGetAddrFromTable(n2, taddr[n1].addr, kNbElemTabAddr-1, kNbElemTabAddr-1, err, n1, -1, TestRegularSeg, outLen, expectingStamps);
			}
			else
			{
				result = taddr[n].addr;
				if (outLen != nil)
					*outLen = taddr[n].len;
			}
		}
	}

	return result;
}


DataAddr4D Base4D_NotOpened::GetAddrFromTable(TabAddrCache* cache, sLONG n, DataAddr4D addr, sLONG nbelem, VError& err, Boolean TestRegularSeg, sLONG* outLen, bool expectingStamps)
{
	DataAddr4D result;
	sLONG len;
	err = VE_OK;

	if (cache != nil && cache->GetAddr(n, result, len))
	{
		if (outLen != nil)
			*outLen = len;
		return result;
	}
	else
	{
		sLONG nbmax = nbelem;
		/*
		if (nbmax == kNbElemTabAddr || nbmax == kNbElemTabAddr2)
			nbmax++;
			*/

		return SubGetAddrFromTable(n, addr, nbmax, nbelem, err, 0, -1, TestRegularSeg, outLen, expectingStamps);
	}
}


VError Base4D_NotOpened::ReadAddressTable(DataAddr4D ou, TabAddrDisk& taddr, sLONG pos1, sLONG pos2, ToolLog* log, 
										  IProblemReporterIntf* reporter, VError& err, sLONG selector, Boolean TestRegularSeg, bool expectingStamps)
{
	err = VE_OK;
	VError errexec = VE_OK;

	if (IsAddrValid(ou, TestRegularSeg))
	{
		DataBaseObjectHeader tag;
		err = tag.ReadFrom(this, ou);
		if (err != VE_OK && reporter != nil)
			errexec = reporter->ReportInvalidTabAddrRead(log, selector);

		if (err == VE_OK && errexec == VE_OK)
		{
			if (expectingStamps)
				err = tag.ValidateTag(DBOH_TableAddressWithStamps, pos1, pos2);
			else
				err = tag.ValidateTag(DBOH_TableAddress, pos1, pos2);
			if (err != VE_OK && reporter != nil)
				errexec = reporter->ReportInvalidTabAddrTag(log, selector);
		}

		if (err == VE_OK && errexec == VE_OK)
		{
			err=readlong(&taddr, kSizeTabAddr, ou, kSizeDataBaseObjectHeader);
			if (err != VE_OK && reporter != nil)
				errexec = reporter->ReportInvalidTabAddrRead(log, selector);
		}

		if (err == VE_OK && errexec == VE_OK)
		{
			err = tag.ValidateCheckSum(&taddr, kSizeTabAddr);
			if (err != VE_OK && reporter != nil)
				errexec = reporter->ReportInvalidTabAddrChecksum(log, selector);
		}

		if (tag.NeedSwap())
			ByteSwapCollection(&taddr[0], kNbElemTabAddr);
		
	}
	else
	{
		err = VE_DB4D_ADDRESSISINVALID;
		if (reporter != nil)
			errexec = reporter->ReportInvalidTabAddrAddr(log, selector);
	}

	return errexec;
}


uBOOL Base4D_NotOpened::IsAddrValid(DataAddr4D ou, Boolean TestRegularSeg)
{
	sLONG i = (ou & (kMaxSegData-1)) + 1;
	if ( TestRegularSeg && (i > kMaxSegDataNormaux))
		return false;
	else
	{
		SegDataMap::iterator found = segs.find(i);
		if (found != segs.end())
		{
			return found->second->IsAddrValid(ou);
		}
		else
			return false;
	}

}


VError Base4D_NotOpened::AttachTo(Base4D* inBase, ToolLog *log, BaseTaskInfo* context)
{
	VError errexec = VE_OK;

	fAttachedBase = inBase;
	fAttachedContext = context;
	fIsOpen = true;
	if (context != nil)
		context->Retain();

	Base4D* basestruct = inBase->GetStructure();
	if (basestruct == nil)
	{
		fStruct = nil;
	}
	else
	{
		fStruct = new Base4D_NotOpened();
		if (!inBase->StoredAsXML())
			errexec = fStruct->AttachTo(basestruct, log, nil);
	}

	if (inBase->StoredAsXML())
	{
		for (sLONG i = 1, nb = inBase->GetNBTable(); i <= nb; i++)
		{
			Table* tt = inBase->RetainTable(i);
			if (tt != nil)
			{
				VString tname;
				VUUID xid;
				tt->GetName(tname);
				tt->GetUUID(xid);
				AddTableName(i, tname, log, nil);
				//AddUUID(xid.GetBuffer(), tuuid_Table, tname, i, 0, log);
				for (sLONG j = 1, nbcrit = tt->GetNbCrit(); j <= nbcrit; j++)
				{
					Field* crit = tt->RetainField(j);
					if (crit != nil)
					{
						VString fname;
						VUUID fid;
						crit->GetName(fname);
						fname = tname+"."+fname;
						crit->GetUUID(fid);
						AddFieldName(i, j, crit->GetTyp(), fname, log);
						//AddUUID(fid.GetBuffer(), tuuid_Field, fname, i, j, log);
						crit->Release();
					}
				}
				tt->Release();
			}
		}
	}

	const VArrayOf<SegData*>* xsegs = inBase->GetDataSegs();

	for (VArrayOf<SegData*>::ConstIterator cur = xsegs->First(), end = xsegs->End(); cur != end; cur++)
	{
		sLONG segnum = (*cur)->GetNum();
		SegData_NotOpened* seg1 = new SegData_NotOpened(this);

		seg1->Init((*cur)->GetFileDesc(), segnum);
		seg1->Open(segnum, log);
		segs[segnum] = seg1;
	}

	SegData* indseg = inBase->GetIndexSeg();
	if (indseg != nil)
	{
		sLONG segnum = indseg->GetNum();
		SegData_NotOpened* seg1 = new SegData_NotOpened(this);

		seg1->Init(indseg->GetFileDesc(), segnum);
		seg1->Open(segnum, log);
		segs[segnum] = seg1;
	}

	readlong(&hbbloc, sizeof(hbbloc), 0, 0);
	if (hbbloc.tag != tagHeader4D)
	{
		if (hbbloc.tag == SwapLong(tagHeader4D))
		{
			hbbloc.SwapBytes();
		}
	}

	fID = hbbloc.GetID();

	FHeaderIsValid = true;
	FAllSegsIDMatch = true;
	fValidVersionNumber = true;


	if (errexec == VE_OK && fStruct == nil)
	{
		// si la base est une structure alors on ajoute les deux tables de resource
		AddTableName(1, L"Resources", log, nil);
		AddFieldName(1, 1, DB4D_Integer32, L"kind", log);
		AddFieldName(1, 2, DB4D_Integer32, L"id", log);
		AddFieldName(1, 3, DB4D_Blob, L"data", log);
		AddFieldName(1, 4, DB4D_Boolean, L"little_endian", log);
		AddFieldName(1, 5, DB4D_Integer32, L"stamp", log);
		AddFieldName(1, 6, DB4D_Integer32, L"options", log);
		AddFieldName(1, 7, DB4D_StrFix, L"name", log);
		AddFieldName(1, 8, DB4D_StrFix, L"attributes", log);
		AddFieldName(1, 9, DB4D_Time, L"modification_time", log);

		AddUUID((VUUIDBuffer&)"Res_DB4D_000000", tuuid_Table, L"Resources", 1, 0, log);

		AddUUID((VUUIDBuffer&)"Res_DB4D_field1", tuuid_Field, L"kind", 1, 1, log);
		AddUUID((VUUIDBuffer&)"Res_DB4D_field2", tuuid_Field, L"id", 1, 2, log);
		AddUUID((VUUIDBuffer&)"Res_DB4D_field3", tuuid_Field, L"data", 1, 3, log);
		AddUUID((VUUIDBuffer&)"Res_DB4D_field4", tuuid_Field, L"little_endian", 1, 4, log);
		AddUUID((VUUIDBuffer&)"Res_DB4D_field5", tuuid_Field, L"options", 1, 5, log);
		AddUUID((VUUIDBuffer&)"Res_DB4D_field6", tuuid_Field, L"name", 1, 6, log);
		AddUUID((VUUIDBuffer&)"Res_DB4D_field7", tuuid_Field, L"attributes", 1, 7, log);
		AddUUID((VUUIDBuffer&)"Res_DB4D_field8", tuuid_Field, L"kind", 1, 8, log);
		AddUUID((VUUIDBuffer&)"Res_DB4D_field9", tuuid_Field, L"modification_time", 1, 9, log);

#if WITH_OBJECTS_TABLE
		AddTableName(2, L"Objects", log, nil);
		AddFieldName(2, 1, DB4D_UUID, L"uuid", log);
		AddFieldName(2, 2, DB4D_StrFix, L"type", log);
		AddFieldName(2, 3, DB4D_StrFix, L"name", log);
		AddFieldName(2, 4, DB4D_Blob, L"data", log);
		AddFieldName(2, 5, DB4D_UUID, L"owner", log);
		AddFieldName(2, 6, DB4D_Integer32, L"access", log);
#endif
	}

	if (errexec != VE_OK)
		fStruct = nil;
	else
	{
		if (fStruct != nil)
		{
			fStruct->SetDatas(this);
			fDatas = new Base4D_NotOpened();
		}
	}

	return errexec;
}



VError Base4D_NotOpened::Open(VFile* inFile, ToolLog *log, FileAccess access, Base4D_NotOpened* inStruct)
{
	VError errexec = VE_OK;

	fStruct = inStruct;

	SegData_NotOpened* seg1 = new SegData_NotOpened(this);

	seg1->Init(inFile, 1);
	VError err = seg1->Open(1, log, access);
	if (err == VE_OK)
	{
		fIsOpen = true;
		segs[1] = seg1;
		BaseHeader bh;
		err = seg1->readat(&hbbloc, sizeof(hbbloc), 0);
		if ( hbbloc. lastoper != 0 ) /* SGT - 17 April 2007 - Bug fix for ACI0050830 */
		{
			DataBaseProblem	pb( TOP_LastFlushDidNotComplete );
			log-> Add ( pb );
		}

		if (err == VE_OK)
		{
			fID = hbbloc.GetID();
			FHeaderIsValid = true;
			FAllSegsIDMatch = true;
			if (hbbloc.tag != tagHeader4D)
			{
				if (hbbloc.tag == SwapLong(tagHeader4D))
				{
					hbbloc.SwapBytes();
				}
			}
			if (hbbloc.tag != tagHeader4D) 
			{
				DataBaseProblem pb(TOP_WrongHeader);
				errexec = log->Add(pb);
				FHeaderIsValid = false;
			}

			if (hbbloc.VersionNumber != kVersionDB4D)
			{
				DataBaseProblem pb(TOP_WrongHeaderVersionNumber);
				errexec = log->Add(pb);
			}
			else
				fValidVersionNumber = true;

			
			if ( (hbbloc.addrmultisegheader != -1 && hbbloc.lenmultisegheader == 0)
					|| (hbbloc.addrmultisegheader == -1 && hbbloc.lenmultisegheader != 0)
					|| hbbloc.lenmultisegheader < 0 || hbbloc.lenmultisegheader > 60000
					|| hbbloc.nbsegdata <= 0 || hbbloc.nbsegdata > kMaxSegDataNormaux)
			{
				DataBaseProblem pb(TOP_MultiSegHeaderIsInvalid);
				errexec = log->Add(pb);
			}
			else
			{
				if (hbbloc.lenmultisegheader > 0)
				{
					if (seg1->IsAddrValid(hbbloc.addrmultisegheader, true))
					{
						void* p = GetFastMem(hbbloc.lenmultisegheader, false, 'tmp2');
						if (p == nil)
							errexec = memfull;
						else
						{
							err = seg1->readat(p, hbbloc.lenmultisegheader, hbbloc.addrmultisegheader);
							if (err != VE_OK)
							{
								DataBaseProblem pb(TOP_MultiSegHeaderIsInvalid);
								errexec = log->Add(pb);
							}
							else
							{
								fIsFullyOpen = true;
								VConstPtrStream bb( p, hbbloc.lenmultisegheader);
								bb.OpenReading();
								uLONG tag = bb.GetLong();
								if (tag != kTagSegTable)
								{
									tag = SwapLong(tag);
									if (tag == kTagSegTable)
										bb.SetNeedSwap(true);
									else
									{
										fIsFullyOpen = false;
										DataBaseProblem pb(TOP_MultiSegHeaderIsInvalid);
										errexec = log->Add(pb);
									}
								}

								if (fIsFullyOpen) 
								{
									sLONG i;
									VError errstream = VE_OK;
									for( i = 1 ; (i < hbbloc.nbsegdata) && (errexec == VE_OK) && (errstream == VE_OK) ; ++i) 
									{
										VStr255	s;
										errstream = s.ReadFromStream( &bb);
										if (errstream == VE_OK || errstream == XBOX_LONG8(8679670649385386184)) 
										{
											errstream = VE_OK;
											if (i != 1)
											{
												SegData_NotOpened *segx = new SegData_NotOpened(this);
												segx->Init(inFile, i);
												err = segx->Open(i, log, access);
												if (err != VE_OK)
												{
													delete segx;
													fIsFullyOpen = false;
													DataSegProblem pb(TOP_CannotOpenDataSeg, i);
													errexec = log->Add(pb);
												}
												else
												{
													segs[i] = segx;
												}
											}

										}
										else
										{
											fIsFullyOpen = false;
											DataBaseProblem pb(TOP_MultiSegHeaderIsInvalid);
											errexec = log->Add(pb);										
										}
									}
								}
								bb.CloseReading();

							}
							FreeFastMem(p);
						}
					}
					else
					{
						DataBaseProblem pb(TOP_MultiSegHeaderIsInvalid);
						errexec = log->Add(pb);
					}
				}
			}

		}
	}
	else
	{
		errexec = err;
	}

	if (errexec == VE_OK)
	{
		VFolder* parent = inFile->RetainParentFolder();
		if (parent != nil)
		{
			VString name;
			inFile->GetNameWithoutExtension(name);
			if (fStruct == nil)
				name += kStructIndexExt;
			else
				name += kDataIndexExt;

			VFile* indfile = new VFile(*parent, name);

			SegData_NotOpened* segind = new SegData_NotOpened(this);

			segind->Init(indfile, kIndexSegNum);
			VError err2 = segind->Open(kIndexSegNum, log, access);
			if (err2 != VE_OK)
			{
				delete segind;
			}
			else
			{
				segs[kIndexSegNum] = segind;
			}

			indfile->Release();
			parent->Release();
		}
	}

	if (errexec == VE_OK && fStruct == nil)
	{
		// si la base est une structure alors on ajoute les deux tables de resource
		AddTableName(1, L"Resources", log, nil);
		AddFieldName(1, 1, DB4D_Integer32, L"kind", log);
		AddFieldName(1, 2, DB4D_Integer32, L"id", log);
		AddFieldName(1, 3, DB4D_Blob, L"data", log);
		AddFieldName(1, 4, DB4D_Boolean, L"little_endian", log);
		AddFieldName(1, 5, DB4D_Integer32, L"stamp", log);
		AddFieldName(1, 6, DB4D_Integer32, L"options", log);
		AddFieldName(1, 7, DB4D_StrFix, L"name", log);
		AddFieldName(1, 8, DB4D_StrFix, L"attributes", log);
		AddFieldName(1, 9, DB4D_Time, L"modification_time", log);

		AddUUID((VUUIDBuffer&)"Res_DB4D_000000", tuuid_Table, L"Resources", 1, 0, log);

		AddUUID((VUUIDBuffer&)"Res_DB4D_field1", tuuid_Field, L"kind", 1, 1, log);
		AddUUID((VUUIDBuffer&)"Res_DB4D_field2", tuuid_Field, L"id", 1, 2, log);
		AddUUID((VUUIDBuffer&)"Res_DB4D_field3", tuuid_Field, L"data", 1, 3, log);
		AddUUID((VUUIDBuffer&)"Res_DB4D_field4", tuuid_Field, L"little_endian", 1, 4, log);
		AddUUID((VUUIDBuffer&)"Res_DB4D_field5", tuuid_Field, L"options", 1, 5, log);
		AddUUID((VUUIDBuffer&)"Res_DB4D_field6", tuuid_Field, L"name", 1, 6, log);
		AddUUID((VUUIDBuffer&)"Res_DB4D_field7", tuuid_Field, L"attributes", 1, 7, log);
		AddUUID((VUUIDBuffer&)"Res_DB4D_field8", tuuid_Field, L"kind", 1, 8, log);
		AddUUID((VUUIDBuffer&)"Res_DB4D_field9", tuuid_Field, L"modification_time", 1, 9, log);

#if WITH_OBJECTS_TABLE
		AddTableName(2, L"Objects", log, nil);
		AddFieldName(2, 1, DB4D_UUID, L"uuid", log);
		AddFieldName(2, 2, DB4D_StrFix, L"type", log);
		AddFieldName(2, 3, DB4D_StrFix, L"name", log);
		AddFieldName(2, 4, DB4D_Blob, L"data", log);
		AddFieldName(2, 5, DB4D_UUID, L"owner", log);
		AddFieldName(2, 6, DB4D_Integer32, L"access", log);
#endif
	}

	if (errexec != VE_OK)
		fStruct = nil;
	else
	{
		if (fStruct != nil)
		{
			fStruct->SetDatas(this);
			fDatas = new Base4D_NotOpened();
		}
	}

	return errexec;
}


void Base4D_NotOpened::SetStruct(Base4D_NotOpened* inStruct)
{
	fStruct = inStruct;
	fStruct->SetDatas(this);
	fDatas = new Base4D_NotOpened();
}


void Base4D_NotOpened::Close()
{
	for (IndexNonOpenedCollection::iterator cur = fInds.begin(), end = fInds.end(); cur != end; cur++)
	{
		Index_NonOpened* ind = *cur;
		if (ind != nil)
			delete ind;
		*cur = nil;
	}

	for (IndexNonOpenedCollection::iterator cur = fIndsInStruct.begin(), end = fIndsInStruct.end(); cur != end; cur++)
	{
		Index_NonOpened* ind = *cur;
		if (ind != nil)
			delete ind;
		*cur = nil;
	}

	for (SegDataMap::iterator cur = segs.begin(), end = segs.end(); cur != end; cur++)
	{
		cur->second->Close();
		delete cur->second;
	}
	segs.clear();

	for (DataTableCollection::iterator cur = fDataTables.begin(), end = fDataTables.end(); cur != end; cur++)
	{
		if (*cur != nil)
		{
			delete *cur;
		}
	}
	fDataTables.clear();
	if (fStruct != nil)
	{
		fStruct->Close();
		delete fStruct;
		fStruct = nil;
	}

	if (fAttachedContext != nil)
	{
		xbox_assert(fAttachedBase != nil);
#if debuglr
		debug_tools_started = false;
#endif
		fAttachedBase->UnLock(fAttachedContext);
		fAttachedContext->Release();
		fAttachedContext = nil;
		fAttachedBase = nil;
	}
}


VError Base4D_NotOpened::CheckAllDataSegs(ToolLog *log)
{
	VError err = VE_OK;
	for (SegDataMap::iterator cur = segs.begin(), end = segs.end(); cur != end && err == VE_OK; cur++)
	{
		err = cur->second->CheckBittables(log);
	}
	return err;
}


VError Base4D_NotOpened::LoadDataTables(ToolLog *log)
{
	VError errexec = VE_OK;

	if (!fSeqNumsAreLoaded)
	{
		errexec = CheckSeqNums(log);
	}
	if (errexec == VE_OK && !fAreDataTablesLoaded)
	{
		VString s;
		log->GetVerifyOrCompactString(1,s);	// "Load Data Tables"
		errexec = log->OpenProgressSession(s,0);
		if (errexec == VE_OK)
		{
			if (hbbloc.DataTables_addrtabaddr == 0 && hbbloc.nbDataTable != 0)
			{
				DataBaseProblem pb(TOP_TableOfDataTablesIsInvalid);
				errexec = log->Add(pb);
			}
			else
			{
				if (hbbloc.nbDataTable == 0 && hbbloc.DataTables_addrtabaddr != 0)
				{
					DataBaseProblem pb(TOP_TableOfDataTablesIsInvalid);
					errexec = log->Add(pb);
				}
				else
				{
					if (hbbloc.nbDataTable<0 || hbbloc.nbDataTable > kMaxTables)
					{
						DataBaseProblem pb(TOP_TableOfDataTablesIsInvalid);
						errexec = log->Add(pb);
					}
					else
					{
						if (hbbloc.nbDataTable>0)
						{
							if (!IsAddrValid(hbbloc.DataTables_addrtabaddr))
							{
								DataBaseProblem pb(TOP_TableOfDataTablesIsInvalid);
								errexec = log->Add(pb);
							}
							else
							{
								fIsDataTablesHeaderValid = true;
								try
								{
									fDataTables.resize(hbbloc.nbDataTable, nil);
									fIsDataTablesHeaderFullyValid = true;
									errexec = CheckDataTables(log);
									if (errexec == VE_OK)
									{
										for (DataTableCollection::iterator cur = fDataTables.begin(), end = fDataTables.end(); cur != end && errexec == VE_OK; cur++)
										{
											DataFile_NotOpened* df = *cur;
											if (df != nil)
											{
												errexec = df->Load(log);
											}
										}
										fAreDataTablesLoaded = true;
									}
									else
										fIsDataTablesHeaderFullyValid = false;

								}
								catch (...)
								{
									errexec = memfull;
								}
							}
						}

						else
						{
							// no tables... it's not an error
							fIsDataTablesHeaderValid = true;
						}
					}
				}
			}
			log->CloseProgressSession();
		}
	}

	return errexec;
}


DataFile_NotOpened* Base4D_NotOpened::GetDataTable(sLONG n) const
{
	if (n<1 || n>(sLONG)fDataTables.size())
		return nil;
	else
		return fDataTables[n-1];
}



DataFile_NotOpened* Base4D_NotOpened::GetDataTableWithTableDefNum(sLONG n) const
{
	if (n<1 || n>(sLONG)fDataTablesByTableDefNum.size())
		return nil;
	else
		return fDataTablesByTableDefNum[n-1];
}


void Base4D_NotOpened::SetDataTableByTableRefNum(DataFile_NotOpened* df, sLONG n)
{
	if (n > (sLONG)fDataTablesByTableDefNum.size())
		fDataTablesByTableDefNum.resize(n, nil);
	fDataTablesByTableDefNum[n-1] = df;
}




VError Base4D_NotOpened::CheckAndLoadStructElemDef(DataAddr4D ou, sLONG len, sLONG numobj, ObjContainer* checker, void* &p, sLONG &outlen, Boolean &needswap, VError &err, DataBaseObjectType fxSignature)
{
	err = VE_OK;
	VError errexec = VE_OK;

	p = nil;
	if (IsAddrValid(ou, checker->TestRegularSeg()))
	{
		checker->GetLog()->MarkAddr_StructElemDef(ou, len, numobj, checker->GetTypObj());
		outlen = len - kSizeDataBaseObjectHeader;

		if (outlen > 0)
		{
			DataBaseObjectHeader tag;
			err = tag.ReadFrom(this, ou);
			if (err != VE_OK)
				errexec = checker->AddProblem(TOP_PhysicalDataIsInvalid, numobj);
			if (err == VE_OK && errexec == VE_OK)
			{
				err = tag.ValidateTag(fxSignature, numobj, -1);
				if (err != VE_OK)
					err = tag.ValidateTag(DBOH_StructDefElem, numobj, -1);
				if (err != VE_OK)
					errexec = checker->AddProblem(TOP_TagIsInvalid, numobj);
			}
			needswap = tag.NeedSwap();
			if (err == VE_OK && errexec == VE_OK)
			{
				p = GetFastMem(outlen, false, 'STED');
				if (p == nil)
				{
					errexec = memfull;
					outlen = 0;
				}
				else
				{
					err = readlong( p, outlen, ou, kSizeDataBaseObjectHeader);
					if (err != VE_OK)
						errexec = checker->AddProblem(TOP_PhysicalDataIsInvalid, numobj);

					if (err == VE_OK && errexec == VE_OK)
					{
						err =tag.ValidateCheckSum(p, outlen);
						if (err != VE_OK)
							errexec = checker->AddProblem(TOP_CheckSumIsInvalid, numobj);
					}
					if (err != VE_OK || errexec != VE_OK)
					{
						FreeFastMem(p);
						p = nil;
						outlen = 0;
					}
				}
			}
		}
		else
		{
			if (outlen != 0)
			{
				errexec = checker->AddProblem(TOP_LengthIsInvalid, numobj);
				err = VE_DB4D_WRONGRECORDHEADER;
			}
		}
	}
	else
	{
		errexec = checker->AddProblem(TOP_AddrIsInvalid, numobj);
		err = VE_DB4D_WRONGRECORDHEADER;
	}

	return errexec;
}


VError Base4D_NotOpened::CheckAllDataTables(ToolLog *log)
{
	VError errexec = VE_OK;
	errexec = LoadDataTables(log);
	if (errexec == VE_OK)
	{
		for (DataTableCollection::iterator cur = fDataTables.begin(), end = fDataTables.end(); cur != end && errexec == VE_OK; cur++)
		{
			if (*cur != nil)
			{
				errexec = (*cur)->CheckAllBlobs(log);
				if (errexec == VE_OK)
					errexec = (*cur)->CheckAllRecords(log);

			}
		}
	}
	return errexec;
}


VError Base4D_NotOpened::CheckOneTable(sLONG tablenum, ToolLog* log)
{
	VError errexec = VE_OK;
	errexec = LoadDataTables(log);

	if (errexec == VE_OK)
	{
		bool found = false;
		if (tablenum > 0)
		{
			for (DataTableCollection::iterator cur = fDataTables.begin(), end = fDataTables.end(); cur != end; cur++)
			{
				DataFile_NotOpened* df = *cur;

				if ((df != nil) && (df->GetNumTableDef() == tablenum))
				{
					found = true;
					errexec = df->CheckAllBlobs(log);
					if (errexec == VE_OK)
						errexec = df->CheckAllRecords(log);
					break;
				}
			}
		}

		if (!found)
			errexec = VE_DB4D_INDICE_OUT_OF_RANGE;
	}
	return errexec;
}


VError Base4D_NotOpened::CheckIndexDefs(ToolLog *log)
{
	VError errexec = VE_OK;

	if (fIndexes == nil)
	{
		// check if the segment containing the segment was sucessfully opened
		SegDataMap::iterator found = segs.find(kIndexSegNum);
		if (found != segs.end() || !hbbloc.WithSeparateIndexSegment)
		{
			fIndexes = new IndexDefChecker(this, hbbloc.IndexDef_addrtabaddr, hbbloc.nbIndexDef ,hbbloc.IndexDef_debuttrou, log);
			if (fIndexes == nil)
				errexec = memfull;
		}
		else
		{
			// warning for missing 4DIndx ou 4DIndy file... we prefer to be silent, it's not a real problem (m.c)
			//	DataSegProblem pb(TOP_Segment_IndexFileMissing, kIndexSegNum,TO_ErrorLevel_Warning);
			//	errexec = log->Add(pb);
		}
	}

	if (fIndexes != nil)
	{
		VString progMsg;
		VString progMsg2;
		log->GetVerifyOrCompactString(4,progMsg);		// Checking Index Definitions
		log->GetVerifyOrCompactString(5,progMsg2);	// Checking chained List of deleted Index Definitions

		errexec = fIndexes->CheckAllObjs(progMsg,progMsg2);
	}

	return errexec;
}



VError Base4D_NotOpened::CheckIndexDefInStructs(ToolLog *log)
{
	VError errexec = VE_OK;

	if (fIndexesInStruct == nil)
	{
		fIndexesInStruct = new IndexDefInStructChecker(this, hbbloc.IndexDefInStruct_addrtabaddr, hbbloc.nbIndexDefInStruct ,hbbloc.IndexDefInStruct_debuttrou, log);
		if (fIndexesInStruct == nil)
			errexec = memfull;
	}

	if (fIndexesInStruct != nil)
	{
		VString progMsg;
		VString progMsg2;
		log->GetVerifyOrCompactString(6,progMsg);	// Checking Backup Index Definitions
		log->GetVerifyOrCompactString(7,progMsg2); // Checking chained List of deleted Backup Index Definitions

		errexec = fIndexesInStruct->CheckAllObjs(progMsg,progMsg2);
	}

	return errexec;
}


VError Base4D_NotOpened::CheckTableDefs(ToolLog *log)
{
	VError errexec = VE_OK;

	if (fTableDefs == nil)
	{
		fTableDefs = new TableDefChecker(this, hbbloc.TableDef_addrtabaddr, hbbloc.nbTableDef ,hbbloc.TableDef_debuttrou, log);
		if (fTableDefs == nil)
			errexec = memfull;
	}

	if (fTableDefs != nil)
	{
		VString progMsg;
		log->GetVerifyOrCompactString(2,progMsg); // Checking Data Tables Definitions
		VString progMsg2;	
		log->GetVerifyOrCompactString(3,progMsg2); // Checking chained List of deleted Tables Definitions

		errexec = fTableDefs->CheckAllObjs(progMsg,progMsg2);
	}

	if (fStruct != nil && errexec == VE_OK) // we are on a data file
	{
		if (log->IsCompacting())
		{
			Base4D* xbd = log->GetTargetCompact();
			if (xbd->IsWakandaRunningMode())
			{
				xbd->occupe();
				xbd->LoadAllTableDefsInData(true);
			
				if (errexec == VE_OK)
					errexec = xbd->SecondPassLoadEntityModels(nil, false);
				if (errexec == VE_OK )
				{
					//errexec = xbd->LoadIndexes(false);
					xbd->setmodif(true, xbd, nil);
				}
				xbd->libere();
			}

		}
	}

	return errexec;
}


VError Base4D_NotOpened::CheckRelationDefs(ToolLog *log)
{
	VError errexec = VE_OK;

	if (fRelationDefs == nil)
	{
		fRelationDefs = new RelationDefChecker(this, hbbloc.Relations_addrtabaddr, hbbloc.nbRelations ,hbbloc.Relations_debuttrou, log);
		if (fRelationDefs == nil)
			errexec = memfull;
	}

	if (fRelationDefs != nil)
	{
		VString progMsg;
		VString progMsg2;
		log->GetVerifyOrCompactString(8,progMsg);	// Checking Relations Definitions
		log->GetVerifyOrCompactString(9,progMsg2);	// Checking chained List of deleted Relations Definitions

		errexec = fRelationDefs->CheckAllObjs(progMsg,progMsg2);
	}

	return errexec;
}


VError Base4D_NotOpened::CheckSeqNums(ToolLog *log)
{
	VError errexec = VE_OK;

	if (fSeqNums == nil)
	{
		fSeqNums = new SeqNumChecker(this, hbbloc.SeqNum_addrtabaddr, hbbloc.nbSeqNum ,hbbloc.SeqNum_debuttrou, log);
		if (fSeqNums == nil)
			errexec = memfull;
	}

	if (fSeqNums != nil)
	{
		VString progMsg;
		VString progMsg2;
		log->GetVerifyOrCompactString(10,progMsg);	// Checking Auto Sequence Numbers Definitions
		log->GetVerifyOrCompactString(11,progMsg2);	// Checking chained List of deleted Auto Sequence Numbers Definitions

		errexec = fSeqNums->CheckAllObjs(progMsg,progMsg2);
		fSeqNumsAreLoaded = true;
	}

	return errexec;
}


VError Base4D_NotOpened::CheckDataTables(ToolLog *log)
{
	VError errexec = VE_OK;

	if (fDataTableChecker == nil)
	{
		fDataTableChecker = new DataTableChecker(this, hbbloc.DataTables_addrtabaddr, hbbloc.nbDataTable ,hbbloc.DataTables_debuttrou, log);
		if (fDataTableChecker == nil)
			errexec = memfull;
	}

	if (fDataTableChecker != nil)
	{
		fDataTableChecker->setDataTables(&fDataTables);

		VString progMsg;
		VString progMsg2;
		log->GetVerifyOrCompactString(12,progMsg);	// Checking Data Table Headers
		log->GetVerifyOrCompactString(13,progMsg2);	// Checking chained List of deleted Data Table Headers

		errexec = fDataTableChecker->CheckAllObjs(progMsg,progMsg2);
	}

	return errexec;
}


VError Base4D_NotOpened::CheckExtraProperties(ToolLog *log)
{
	VError err = VE_OK;
	if (log->IsCompacting())
	{
		DataBaseProblem potentialProblem(TOP_FullyWrong);
		VValueBag* dataBag = nil;
		DataAddr4D ou = hbbloc.ExtraAddr;
		sLONG len = hbbloc.ExtraLen;
		err = CheckExtraProp(ou, len, potentialProblem, log, dataBag, -1, -1, DBOH_ExtraDataBaseProperties);
		if (ou == -1)
		{
			ou = 0;
			len = 0;
		}
		
		Base4D* target = log->GetTargetCompact();
		if (GetStruct() == nil)
			target = target->GetStructure();
		target->SetExtraPropHeader(ou, len);

		//log->GetTargetCompact()->SetExtraPropHeader(ou, len);

		if (dataBag != nil)
			dataBag->Release();
	}
	else
	{
		DataBaseProblem potentialProblem(TOP_FullyWrong);
		VValueBag* dataBag = nil;
		err = CheckExtraProp(hbbloc.ExtraAddr, hbbloc.ExtraLen, potentialProblem, log, dataBag, -1, -1, DBOH_ExtraDataBaseProperties);
		if (dataBag != nil)
			dataBag->Release();
	}
	return err;
}


VError Base4D_NotOpened::AddTableName(sLONG tablenum, const VString& tablename, ToolLog* log, VString* outName)
{
	if (outName != nil)
	{
		VString outname = tablename;
		if (fAlreadyTableNames.find(outname) != fAlreadyTableNames.end())
		{
			sLONG i = 0;
			bool stop = false;
			while (!stop)
			{
				++i;
				outname = tablename+"_"+ToString(i);
				if (fAlreadyTableNames.find(outname) == fAlreadyTableNames.end())
					stop = true;
			}
		}
		*outName = outname;
		fTableNames[tablenum] = *outName;
		fAlreadyTableNames.insert(*outName);
	}
	else
	{
		try
		{
			fTableNames[tablenum] = tablename;
		}
		catch (...)
		{
			return memfull;
		}
	}
	return VE_OK;
}


Boolean Base4D_NotOpened::ExistTable(sLONG numTable, VString* outName)
{
	TableNamesCollection::iterator found = fTableNames.find(numTable);

	if (found != fTableNames.end())
	{
		if (outName != nil)
			*outName = found->second;
		return true;
	}
	else
	{
		if (outName != nil)
		{
			VString s;
			s.FromLong(numTable);
			*outName = L"[Table#";
			*outName += s;
			*outName += L"]";
		}
		return false;
	}
}


VError Base4D_NotOpened::AddFieldName(sLONG tablenum, sLONG fieldnum, sLONG fieldtype, const VString& fieldname, ToolLog* log)
{
	Field_NotOpened_Ref ref(tablenum, fieldnum);

	try
	{
		fFieldNames[ref] = FieldObj_NotOpened(fieldtype, fieldname);
	}
	catch (...)
	{
		return memfull;
	}

	return VE_OK;

}


Boolean Base4D_NotOpened::ExistField(sLONG numTable, sLONG numField, VString* outName, sLONG* outType)
{
	FieldNamesCollection::iterator found = fFieldNames.find(Field_NotOpened_Ref(numTable, numField));
	if (found != fFieldNames.end())
	{
		if (outName != nil)
			*outName = found->second.fName;
		if (outType != nil)
			*outType = found->second.fType;
		return true;
	}
	else
	{
		if (outName != nil)
		{
			VString s;
			s.FromLong(numTable);
			*outName = L"[Table#";
			*outName += s;
			*outName += L"].[Field#";
			s.FromLong(numField);
			*outName += L"]";
		}
		if (outType != nil)
			*outType = 0;
		return false;
	}
}



VError Base4D_NotOpened::AddUUID(const VUUIDBuffer& id, TypeOfUUID type, const VString& name, sLONG numobj, sLONG numobj2, ToolLog* log)
{
	VError errexec = VE_OK;

	UUIDObj_NotOpened_Collection::iterator found = fUUIDs.find(id);
	if (found != fUUIDs.end())
	{
		UUIDConflictProblem pb(TOP_TwoIdenticalUUIDs, found->second.fType, found->second.fName, type, name);
		errexec = log->Add(pb);
	}
	else
	{
		try
		{
			fUUIDs.insert(make_pair(id, UUIDObj_NotOpened(type, name, numobj, numobj2)));
		}
		catch (...)
		{
			errexec = memfull;
		}

	}

	return errexec;
}


Boolean Base4D_NotOpened::ExistUUID(const VUUIDBuffer& id, TypeOfUUID type, VString* outName, TypeOfUUID* outType, sLONG* outNumObj, sLONG* outNumObj2)
{
	UUIDObj_NotOpened_Collection::iterator found = fUUIDs.find(id);
	if (found != fUUIDs.end())
	{
		if (outName != nil)
			*outName = found->second.fName;
		if (outType != nil)
			*outType = found->second.fType;
		if (outNumObj != nil)
			*outNumObj = found->second.fNumObj;
		if (outNumObj2 != nil)
			*outNumObj2 = found->second.fNumObj2;
		return type == found->second.fType;
	}
	else
	{
		if (outName != nil)
			outName->Clear();
		if (outType != nil)
			*outType = 0;
		if (outNumObj != nil)
			*outNumObj = 0;
		if (outNumObj2 != nil)
			*outNumObj2 = 0;
		return false;
	}
}


VError Base4D_NotOpened::CheckExtraProp(DataAddr4D& ou, sLONG& len, ToolObject& problemToReport, ToolLog* log, VValueBag* &outData, sLONG pos1, sLONG pos2, DataBaseObjectType taginfo)
{
	VError errexec = VE_OK;
	outData = nil;

	VError err = VE_OK;
	log->ClearLastProblem();

	if ((ou == 0 && len != 0) || (ou != 0 && len == 0))
	{
		problemToReport.SetProblem(TOP_ExtraPropertyLengthIsInvalid);
		errexec = log->Add(problemToReport);
	}

	if (errexec == VE_OK && ou != 0 && len != 0)
	{
		log->MarkAddr_ExtraProp(ou, len);
		DataBaseObjectHeader tag;

		err = tag.ReadFrom(this, ou);
		if (err == VE_OK)
		{
			if (tag.ValidateTag(taginfo, pos1, pos2) == VE_OK)
			{
				sLONG lenx = tag.GetLen();
				if (lenx != len - kSizeDataBaseObjectHeader)
				{
					problemToReport.SetProblem(TOP_ExtraPropertyTagLengthDoesNotMatch);
					errexec = log->Add(problemToReport);
				}
				if (errexec == VE_OK)
				{
					void* p = GetFastMem(lenx, false, 'EXTR');
					if (p != nil)
					{
						err=readlong(p,lenx,ou,kSizeDataBaseObjectHeader);
						if (err==VE_OK)
						{
							err = tag.ValidateCheckSum(p,lenx);
							if (err == VE_OK)
							{
								VConstPtrStream buf(p, lenx);
								err = buf.OpenReading();
								if (err == VE_OK)
								{
									buf.SetNeedSwap(tag.NeedSwap());
									outData = new VValueBag();
									err = outData->ReadFromStream(&buf);
									if (err != VE_OK)
									{
										outData->Release();
										outData = nil;
										problemToReport.SetProblem(TOP_ExtraPropertiesStreamIsInconsistant);
										errexec = log->Add(problemToReport);
									}
								}
							}
							else
							{
								problemToReport.SetProblem(TOP_ExtraPropertyCheckSumIsInvalid);
								errexec = log->Add(problemToReport);
							}
						}
						else
						{
							problemToReport.SetProblem(TOP_ExtraPropertyPhysicalDataIsInvalid);
							errexec = log->Add(problemToReport);
						}

						if (err == VE_OK && errexec == VE_OK && log->IsCompacting() && !log->SomeProblem())
						{
							Base4D* target = log->GetTargetCompact();
							if (GetStruct() == nil)
								target = target->GetStructure();
							ou = target->findplace(len, nil, err, 0, nil);
							if (ou != -1)
							{
								tag.SetSwapWhileWriting(tag.NeedSwap());
								tag.WriteInto(target, ou, nil);
								target->writelong(p, lenx, ou, kSizeDataBaseObjectHeader, nil, -1);
							}
						}

						FreeFastMem(p);
					}
					else
						errexec = memfull;
				}
			}
			else
			{
				problemToReport.SetProblem(TOP_ExtraPropertyTagIsInvalid);
				errexec = log->Add(problemToReport);
			}
		}
		else
		{
			problemToReport.SetProblem(TOP_ExtraPropertyPhysicalDataIsInvalid);
			errexec = log->Add(problemToReport);
		}
	}

	if (err != VE_OK || log->SomeProblem())
		ou = -1;

	log->ClearLastProblem();
	return errexec;
}


VError Base4D_NotOpened::AddIndex(Index_NonOpened* ind, ToolLog* log)
{
	VError errexec = VE_OK;
	sLONG n = ind->GetNum();

	if (n >= (sLONG)fInds.size())
	{
		try
		{
			fInds.resize(n+1, nil);
		}
		catch (...)
		{
			errexec = memfull;
		}
	}

	if (errexec == VE_OK)
	{
		Index_NonOpened* previous = fInds[n];
		if (previous != nil)
			delete previous;
		fInds[n] = ind;
	}

	return errexec;
}



VError Base4D_NotOpened::AddIndexInStruct(Index_NonOpened* ind, ToolLog* log)
{
	VError errexec = VE_OK;
	sLONG n = ind->GetNum();

	if (n >= (sLONG)fIndsInStruct.size())
	{
		try
		{
			fIndsInStruct.resize(n+1, nil);
		}
		catch (...)
		{
			errexec = memfull;
		}
	}

	if (errexec == VE_OK)
	{
		Index_NonOpened* previous = fIndsInStruct[n];
		if (previous != nil)
			delete previous;
		fIndsInStruct[n] = ind;
	}

	return errexec;
}


VError Base4D_NotOpened::CheckAllIndexes(ToolLog* log)
{
	VError errexec = VE_OK;

	for (IndexNonOpenedCollection::iterator cur = fInds.begin(), end = fInds.end(); cur != end && errexec == VE_OK; cur++)
	{
		Index_NonOpened* ind = *cur;
		if (ind != nil)
			errexec = ind->CheckAll(log);
	}
	return errexec;
}


VError Base4D_NotOpened::CheckOneIndex(sLONG indexnum, ToolLog* log)
{
	VError errexec = VE_OK;

	errexec = LoadDataTables(log);
	errexec = VE_OK;	// we should be able to verify part of the index without any data tables

	if (fIndexes == nil)
		errexec = CheckIndexDefs(log);

	if (errexec == VE_OK)
	{
		Index_NonOpened* ind = nil;

		if ((indexnum >= 0 && indexnum < (sLONG)fIndsInStruct.size()) || (indexnum >= 0 && indexnum < (sLONG)fInds.size()))
		{
			Index_NonOpened* ind_struct = NULL;

			if (indexnum >= 0 && indexnum < (sLONG)fIndsInStruct.size())
				ind_struct = fIndsInStruct[indexnum];

			if (ind_struct != nil)
			{
				// find the matching index in fInds
				for (IndexNonOpenedCollection::iterator cur = fInds.begin(), end = fInds.end(); cur != end; cur++)
				{
					if (*cur != nil)
					{
						if ((*cur)->GetUUID() == ind_struct->GetUUID())
						{
							ind = *cur;
							break;
						}
					}
				}
			}
			else if (ind_struct == nil && indexnum >= 0 && indexnum < (sLONG)fInds.size())
				ind = fInds[indexnum];

			if (ind != nil)
			{
				errexec = ind->CheckAll(log);
			}

			// an index can be in fIndsInStruct, and not yet in fInds (for instance if no data)... it's not an error 
/*
			else
			{
				IndexProblem pb(TOP_IndexDefIsDamaged, indexnum);
				errexec = log->Add(pb);
			}
*/
		}
		else
		{
			errexec = VE_DB4D_INDICE_OUT_OF_RANGE;
		}
	}

	return errexec;
}



			/* ------------------------------------------------------------- */



VValueBag* _RetainExtraPropertiesOnClosedBase(VFile* base, VUUID *outUUID, VError &err, uLONG8* outVersionNumber)
{
	VValueBag* result = nil;
	VFileDesc* f;
	err = base->Open(FA_READ, &f);
	if (err == VE_OK)
	{
		if (f != nil)
		{
			BaseHeader bh;
			err = f->GetData(&bh, sizeof(bh), 0);
			if (err == VE_OK)
			{
				if (bh.tag != tagHeader4D)
				{
					if (bh.tag == SwapLong(tagHeader4D))
					{
						bh.SwapBytes();
					}
				}
				if (bh.tag != tagHeader4D) 
				{
					err = VE_DB4D_WRONGBASEHEADER;
				}
				else
				{
					if (outVersionNumber != nil)
						*outVersionNumber = bh.VersionNumber;
					if (outUUID != nil)
						outUUID->FromBuffer( bh.GetID());
					
					if (bh.ExtraAddr > 0 && bh.ExtraLen > 0)
					{
						void* p = GetFastMem(bh.ExtraLen, false, 'extr');
						if (p == nil)
							err = memfull;
						else
						{
							DataBaseObjectHeader tag;
							err = tag.ReadFrom(f, bh.ExtraAddr);
							if (err==VE_OK)
							{
								if (tag.ValidateTag(DBOH_ExtraDataBaseProperties, -1, -1) == VE_OK)
								{
									err=f->GetData(p,bh.ExtraLen,bh.ExtraAddr+kSizeDataBaseObjectHeader);
									if (err==VE_OK)
									{
										err = tag.ValidateCheckSum(p,bh.ExtraLen - kSizeDataBaseObjectHeader);
										if (err == VE_OK)
										{
											VConstPtrStream buf(p, bh.ExtraLen - kSizeDataBaseObjectHeader);
											err = buf.OpenReading();
											if (err == VE_OK)
											{
												buf.SetNeedSwap(tag.NeedSwap());
												result = new VValueBag();
												err = result->ReadFromStream(&buf);
												if (err != VE_OK)
												{
													result->Release();
													result = nil;
												}
											}
										}
									}

								}
							}

							FreeFastMem(p);
						}
					}
				}
			}
			delete f;
		}
	}

	return result;
}


						/* --------------------------------------- */


ObjContainer::ObjContainer(Base4D_NotOpened* owner, DataAddr4D firstaddr, sLONG nbelem, sLONG debuttrou, ToolLog* log, TypObjContainer typobj, DataBaseObjectType xSignature)
{
	fOwner = owner;
	fLog = log;
	fAddr = firstaddr;
	fNbelem = nbelem;
	fDebuttrou = debuttrou;
	fTypObj = typobj;

	fTabAddrHeaderHasBeenChecked = false;
	fTabAddrHeaderIsValid = false;
	fNbElemIsValid = false;
	fTabAddrIsValid = false;
	fTabTrouIsValid = false;
	fMapIsValid = false;
	fMapHasBeenChecked = false;
	fTestRegularSeg = true;
	fMustCheckWithAddr = false;

	fTempCache = nil;
	fxSignature = xSignature;
}


ObjContainer::~ObjContainer()
{
	if (fTempCache != nil)
		delete fTempCache;
}


VError ObjContainer::ReportInvalidTabAddrAddr(ToolLog* log, sLONG selector)
{
	return AddProblem(TOP_AddrOfTabAddrIsInvalid);
}


VError ObjContainer::ReportInvalidTabAddrTag(ToolLog* log, sLONG selector)
{
	return AddProblem(TOP_TagOfTabAddrIsInvalid);
}


VError ObjContainer::ReportInvalidTabAddrRead(ToolLog* log, sLONG selector)
{
	return AddProblem(TOP_PhysicalDataOfTabAddrIsInvalid);
}


VError ObjContainer::ReportInvalidTabAddrChecksum(ToolLog* log, sLONG selector)
{
	return AddProblem(TOP_ChecksumOfTabAddrIsInvalid);
}


VError ObjContainer::AddProblem(ToolObjectProblem problem, sLONG param1, sLONG param2, sLONG param3)
{
	VError errexec = VE_OK;

	if (param1 == -1)
	{
		switch (fTypObj)
		{
			case obj_TableDef:
				{
					TableDefHeaderProblem pb(problem);
					errexec = fLog->Add(pb);
				}
				break;

			case obj_RelationDef:
				{
					RelationDefHeaderProblem pb(problem);
					errexec = fLog->Add(pb);
				}
				break;

			case obj_SeqNum:
				{
					SeqNumHeaderProblem pb(problem);
					errexec = fLog->Add(pb);
				}
				break;

			case obj_IndexDef:
				{
					IndexDefHeaderProblem pb(problem);
					errexec = fLog->Add(pb);
				}
				break;

			case obj_IndexInStructDef:
				{
					IndexDefInStructHeaderProblem pb(problem);
					errexec = fLog->Add(pb);
				}
				break;

			case obj_DataTable:
				{
					DataTableHeaderProblem pb(problem);
					errexec = fLog->Add(pb);
				}

			}
	}
	else
	{
		switch (fTypObj)
		{
			case obj_TableDef:
				{
					TableDefProblem pb(problem, param1+1);
					errexec = fLog->Add(pb);
				}
				break;

			case obj_RelationDef:
				{
					RelationDefProblem pb(problem, param1);
					errexec = fLog->Add(pb);
				}
				break;

			case obj_SeqNum:
				{
					SeqNumProblem pb(problem, param1);
					errexec = fLog->Add(pb);
				}
				break;

			case obj_IndexDef:
				{
					IndexDefProblem pb(problem, param1);
					errexec = fLog->Add(pb);
				}
				break;

			case obj_IndexInStructDef:
				{
					IndexDefInStructProblem pb(problem, param1);
					errexec = fLog->Add(pb);
				}
				break;

			case obj_DataTable:
				{
					DataTableProblem pb(problem, param1);
					errexec = fLog->Add(pb);
				}
				break;

		}
	}

	return errexec;
}


VError ObjContainer::CheckObjWithAddr(DataAddr4D ou, sLONG len, sLONG numobj, VError& err)
{
	xbox_assert(false);
	return VE_OK;
}


VError ObjContainer::CheckOneEntry(DataAddr4D ou, sLONG len, sLONG numobj)
{
	void* p;
	sLONG objlen = 0;
	Boolean needswap;
	VError err = VE_OK;
	VError errexec;

	if (fMustCheckWithAddr)
	{
		errexec = CheckObjWithAddr(ou, len, numobj, err);
	}
	else
	{
		errexec = fOwner->CheckAndLoadStructElemDef(ou, len, numobj, this, p, objlen, needswap, err, fxSignature);

		if (errexec == VE_OK && err == VE_OK)
		{
			errexec = CheckObj(p, objlen, numobj, needswap, err);
		}

		if (p != nil)
			FreeFastMem(p);
	}

	return errexec;
}



VError ObjContainer::CheckObjs(DataAddr4D ou, sLONG nbobjsmax, sLONG nbobjs, sLONG pos1, sLONG pos2, sLONG mastermultiplicateur)
{
	VError errexec = VE_OK;

	VError err = VE_OK;

	TabAddrDisk taddr;
	fLog->MarkAddr_TabAddr_Other(ou, kSizeTabAddr+kSizeDataBaseObjectHeader, -1, -1);
	errexec = fOwner->ReadAddressTable(ou, taddr, pos1, pos2, fLog, this, err, fTypObj, fTestRegularSeg);
	if (errexec == VE_OK)
	{
		if (err == VE_OK)
		{
			if (nbobjsmax > kNbElemTabAddr)
			{
				sLONG diviseur = kNbElemTabAddr;
				if (nbobjsmax > kNbElemTabAddr2)
					diviseur = kNbElemTabAddr2;

				sLONG nbparents = (nbobjs+diviseur-1) / diviseur;
				for (sLONG i=0; i<nbparents && errexec == VE_OK; i++)
				{
					sLONG nbremains = diviseur;
					if (i == (nbparents-1))
						nbremains = nbobjs & (diviseur-1);
					if (nbremains == 0)
						nbremains = diviseur;
					if (taddr[i].len != kSizeTabAddr)
					{
						errexec = AddProblem(TOP_LenghOfAddressTableIsInvalid /*, mastermultiplicateur + i*/);
						fMapIsValid = false;
					}
					if (errexec == VE_OK)
						errexec = CheckObjs(taddr[i].addr, diviseur - 1, nbremains, i, -1, mastermultiplicateur + i * diviseur);
				}
			}
			else
			{
				for (sLONG i=0; i<nbobjs && errexec == VE_OK; i++)
				{
					errexec = fLog->Progress(mastermultiplicateur + i);
					
					if (errexec == VE_OK)
					{
						DataAddr4D addr = taddr[i].addr;
						if (fTempCache != nil)
							fTempCache->AddAddr(mastermultiplicateur + i, addr, taddr[i].len);
						if (addr <= 0)
						{
							DataAddr4D addr2 = -addr;
							if (addr2 < fNbelem || addr == kFinDesTrou)
							{
								// on ne fait rien du fait que l'entree a ete detruite
							}
							else
							{
								errexec = AddProblem(TOP_AddrIsInvalid , mastermultiplicateur + i);
								fMapIsValid = false;
							}
						}
						else
						{
							errexec = CheckOneEntry(addr, taddr[i].len, mastermultiplicateur + i );
						}
					}
				}
			}
		}
		else
		{
			fMapIsValid = false;
		}
	}
	else
	{
		fMapIsValid = false;
	}

	if (errexec != VE_OK)
		fMapHasBeenChecked = false;

	return errexec;
}


VError ObjContainer::CheckAllObjs(const VString& ProgressMessage, const VString& ProgressMessage2)
{
	VError errexec = VE_OK;
	fTabAddrHeaderHasBeenChecked = true;

	if (fNbelem >= 0 && fNbelem <= kMaxObjsInTable)
		fNbElemIsValid = true;
	else
	{
		errexec = AddProblem(TOP_NbEntriesIsInvalid);
	}

	if (errexec == VE_OK)
	{
		if (fAddr == 0 || fOwner->IsAddrValid(fAddr))
			fTabAddrIsValid = true;
		else
		{
			errexec = AddProblem(TOP_AddrOfPrimaryTabAddrIsInvalid);
		}
	}

	if (errexec == VE_OK)
	{
		if ( (fDebuttrou <= 0 && (-fDebuttrou) <= kMaxObjsInTable) || fDebuttrou == kFinDesTrou )
			fTabTrouIsValid = true;
		else
		{
			errexec = AddProblem(TOP_ListOfDeletedEntriesIsInvalid);
		}
	}

	if (errexec == VE_OK)
	{
		fTabAddrHeaderIsValid = fNbElemIsValid && fTabAddrIsValid && fTabTrouIsValid;

		fMapHasBeenChecked = true;
		if (fNbElemIsValid && fTabAddrIsValid)
		{
			fMapIsValid = true;

			if (fAddr != 0)
			{
				fTempCache = new TabAddrCache(fOwner, fAddr, fNbelem);

				if (fLog->IsCompacting())
				{
					PrepareAddrTable();
				}

				errexec = fLog->OpenProgressSession(ProgressMessage, fNbelem);
				if (errexec == VE_OK)
				{
					sLONG nbmax = fNbelem;
					/*
					if (nbmax == kNbElemTabAddr || nbmax == kNbElemTabAddr2)
						nbmax++;
						*/
					errexec = CheckObjs(fAddr, nbmax, fNbelem, 0, -1, 0);
					fLog->CloseProgressSession();
				}

				if (fLog->IsCompacting())
				{
					NormalizeAddrTable();
				}

				if (fTabTrouIsValid && errexec == VE_OK && !fLog->IsCompacting())
				{
					errexec = fLog->OpenProgressSession(ProgressMessage2, fNbelem);
					errexec = CheckTrous();
					fLog->CloseProgressSession();
				}

				if (fTempCache != nil)
					delete fTempCache;
				fTempCache = nil;
			}
		}
	}

	return errexec;
}



VError ObjContainer::CheckTrous()
{
	VError errexec = VE_OK;

	sLONG nbSets = 0;
	Bittab TempDeleted;

	DataAddr4D n = fDebuttrou;
	Boolean ChaineIsValid = true;
	Boolean ReferenceCirculaire = false;
	Boolean WrongReference = false;

	while (n != kFinDesTrou && ChaineIsValid && errexec == VE_OK)
	{
		DataAddr4D n2 = -n;
		if (n2>=0 && n2<fNbelem)
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
					n = fOwner->GetAddrFromTable(fTempCache, (sLONG)n2, fAddr, fNbelem, err);
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
				errexec = AddProblem(TOP_ListOfDeletedEntriesIsInvalid);
			}
			else
			{
				errexec = AddProblem(TOP_ListOfDeletedEntriesIsInvalid);
			}
		}
	}

	return errexec;
}



				/* --------------------------------------- */


VError IndexDefChecker::PrepareAddrTable()
{
	VError err = VE_OK;

	// pas besoin de preparer la table d'adresses
	// les index n'ont pas besoin de numero d'ordre

	return VE_OK;
}


VError IndexDefChecker::CheckObj(void* p, sLONG len, sLONG numobj, Boolean needswap, VError& err)
{
	VError errexec = VE_OK;
	Base4D_NotOpened* datas;
	Index_NonOpened* ind = nil;

	if (fTypObj == obj_IndexInStructDef)
		datas = fOwner->GetDatas();
	else
		datas = fOwner;


	VConstPtrStream buf(p, len);
	errexec = buf.OpenReading();
	if (errexec == VE_OK)
	{
		buf.SetNeedSwap(needswap);
		sLONG IndexInfoTyp;
		err = buf.GetLong(IndexInfoTyp);
		if (err != VE_OK)
		{
			errexec = AddProblem(TOP_IndexDefIsDamaged, numobj);
		}

		VString indexname;
		VUUID id;
		sLONG xauto;

		if (errexec == VE_OK && err == VE_OK)
		{
			err = id.ReadFromStream(&buf);
			if (err != VE_OK)
			{
				errexec = AddProblem(TOP_IndexDefIsDamaged, numobj);
			}
		}

		if (errexec == VE_OK && err == VE_OK)
		{
			err = indexname.ReadFromStream(&buf);
			if (err != VE_OK)
			{
				errexec = AddProblem(TOP_IndexDefIsDamaged, numobj);
			}
		}

		if (errexec == VE_OK && err == VE_OK)
		{
			err = buf.GetLong(xauto);
			if (err != VE_OK)
			{
				errexec = AddProblem(TOP_IndexDefIsDamaged, numobj);
			}
		}

		if (errexec == VE_OK && err == VE_OK)
		{
			ind = new Index_NonOpened(datas, IndexInfoTyp, id, indexname, numobj, fTypObj == obj_IndexInStructDef);
			if (ind == nil)
			{
				errexec = memfull;
			}
			else
			{
				if (fTypObj != obj_IndexInStructDef)
					errexec = datas->AddIndex(ind, fLog);
				else
					errexec = datas->AddIndexInStruct(ind, fLog);
			}
		}

		if (errexec == VE_OK && err == VE_OK)
		{
			switch(IndexInfo::ReduceType(IndexInfoTyp))
			{
				case DB4D_Index_OnOneField:
				/*
				case DB4D_Index_OnOneField_Scalar_Word:
				case DB4D_Index_OnOneField_Scalar_Byte:
				case DB4D_Index_OnOneField_Scalar_Long:
				case DB4D_Index_OnOneField_Scalar_Long8:
				case DB4D_Index_OnOneField_Scalar_Real:
				*/
				case DB4D_Index_OnKeyWords:
					{
						sLONG typindex = 0;
						sLONG lllu = 0;
						sLONG datakind = 0;
						
						err = buf.GetLong(typindex);
						if (err != VE_OK)
						{
							errexec = AddProblem(TOP_IndexDefIsDamaged, numobj);
						}

						if (errexec == VE_OK && err == VE_OK)
						{
							err=buf.GetLong(lllu);
							if (err != VE_OK)
							{
								errexec = AddProblem(TOP_IndexDefIsDamaged, numobj);
							}
						}
						ind->SetUniqueKeys(lllu == 1);

						VUUID fieldid;

						if (errexec == VE_OK && err == VE_OK)
						{
							err=buf.GetLong(datakind);
							if (err != VE_OK)
							{
								errexec = AddProblem(TOP_IndexDefIsDamaged, numobj);
							}
						}

						if (errexec == VE_OK && err == VE_OK)
						{
							err = fieldid.ReadFromStream(&buf);
							if (err != VE_OK)
							{
								errexec = AddProblem(TOP_IndexDefIsDamaged, numobj);
							}
						}
							if (errexec == VE_OK && err == VE_OK)
						{
							errexec = ind->AddField(fieldid, datakind, fLog);
							if (!ind->IsSourceValid())
								err = -1;
						}
						if (errexec == VE_OK && err == VE_OK)
						{
							errexec = ind->SetHeaderType(typindex, fLog);
						}

						if (errexec == VE_OK && err == VE_OK)
						{
							if (fTypObj != obj_IndexInStructDef)
							{
								errexec = ind->ReadHeader(buf, fLog);
							}
							else
								ind->SetHeaderInvalid();
						}

						
					}
					break;

				case DB4D_Index_OnMultipleFields:
					{
						sLONG typindex = 0;
						sLONG lllu = 0;
						sLONG nbfield = 0;
						err = buf.GetLong(typindex);
						if (err != VE_OK)
						{
							errexec = AddProblem(TOP_IndexDefIsDamaged, numobj);
						}

						if (errexec == VE_OK && err == VE_OK)
						{
							err=buf.GetLong(lllu);
							if (err != VE_OK)
							{
								errexec = AddProblem(TOP_IndexDefIsDamaged, numobj);
							}
						}
						ind->SetUniqueKeys(lllu == 1);

						if (errexec == VE_OK && err == VE_OK)
						{
							err=buf.GetLong(nbfield);
							if (err == VE_OK)
							{
								if (nbfield <=0 || nbfield > kMaxFieldsInCompositeIndex)
								{
									errexec = AddProblem(TOP_IndexDefIsDamaged, numobj);
								}
								else
								{
									for (sLONG i = 0; i < nbfield && errexec == VE_OK && err == VE_OK; i++)
									{
										sLONG datakind;
										VUUID fieldid;
										err = buf.GetLong(datakind);
										if (err == VE_OK)
											err = fieldid.ReadFromStream(&buf);
										if (err != VE_OK)
										{
											errexec = AddProblem(TOP_IndexDefIsDamaged, numobj);
										}
										if (err == VE_OK && errexec == VE_OK)
										{
											errexec = ind->AddField(fieldid, datakind, fLog);
											if (!ind->IsSourceValid())
												err = -1;
										}
									}
								}
							}
							else
							{
								errexec = AddProblem(TOP_IndexDefIsDamaged, numobj);
							}
						}

						if (errexec == VE_OK && err == VE_OK)
						{
							errexec = ind->SetHeaderType(typindex, fLog);
						}

						if (errexec == VE_OK && err == VE_OK)
						{
							if (fTypObj != obj_IndexInStructDef)
							{
								errexec = ind->ReadHeader(buf, fLog);
							}
							else
								ind->SetHeaderInvalid();
						}
					}
					break;

				default:
					{
						errexec = AddProblem(TOP_IndexTypeIsInvalid, numobj);
						err = VE_DB4D_CANNOTLOADINDEXHEADER;
					}
					break;

			}// end du swith
		}

		/*
		if (fTypObj == obj_IndexInStructDef)
		{
			if (ind != nil)
				delete ind;
		}
		*/

	}
	buf.CloseReading();

	if (errexec == VE_OK && err == VE_OK && fLog->IsCompacting())
	{
		Base4D* target = fLog->GetTargetCompact();
		Base4D* truetarget = target;
		if (fOwner->GetStruct() == nil && fTypObj != obj_IndexInStructDef)
		{
			truetarget = target->GetStructure();
		}

		StructElemDef* e = new StructElemDef(fTypObj == obj_IndexInStructDef ? truetarget->GetStructure() : truetarget, -1, fxSignature);
		e->CopyData(p, len);  // les addresses des pages ne sont pas valides, mais seront modifiee lors de la copie des pages elles memes
		
		//e->SetSwapWhileSaving(needswap);
		/*										Sergiy - 29 May 2007 - Bug fix for ACI0052032
		::SetSwapWhileSaving should be called only if the object needs to be copied without usage. If, however, we
		need to actully do something with the object, then we need to load it with the proper byte-swapping, as is 
		the case below. */
		e->SetSwap(needswap);
		
		e->libere();
		if (fTypObj == obj_IndexInStructDef)
		{
			e->SetSwapWhileSaving(needswap);
			err = truetarget->SaveIndexDefInStruct(e);
		}
		else
		{
			if (ind != nil)
			{
				err = truetarget->SaveIndexDef(e);
				if (err == VE_OK)
				{
					IndexInfo* targetind = truetarget->LoadAndInitIndex(e, err);
					if (targetind != nil)
					{
						fLog->SetIndexToBeCompacted(targetind);
						fLog->ClearLastProblem();
						errexec = ind->CheckAll(fLog);
						OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
						targetind->NormalizeTablesAddr(curstack);
						fLog->SetIndexToBeCompacted(nil);
						targetind->Release();
					}
				}
			}
		}
	}

	return errexec;
}


VError IndexDefInStructChecker::PrepareAddrTable()
{
	VError err = VE_OK;

	// pas besoin de preparer la table d'adresses
	// les index n'ont pas besoin de numero d'ordre

	return VE_OK;
}

/*
VError IndexDefInStructChecker::CheckObj(void* p, sLONG len, sLONG numobj, Boolean needswap, VError& err)
{
	VError errexec = VE_OK;

	return errexec;
}
*/


VError TableDefChecker::PrepareAddrTable()
{
	Base4D* target = fLog->GetTargetCompact();
	if (fOwner->GetStruct() != nil)
		return VE_OK;
	else
		return target->ResizeTableOfTableDef(fNbelem);
}


VError TableDefChecker::NormalizeAddrTable()
{
	Base4D* target = fLog->GetTargetCompact();
	if (fOwner->GetStruct() != nil)
		return VE_OK;
	else
		return target->NormalizeTableOfTableDef();
}



VError TableDefChecker::CheckObj(void* p, sLONG len, sLONG numobj, Boolean needswap, VError& err)
{
	VError errexec = VE_OK;
	Base4D_NotOpened* datas = fOwner->GetDatas();
	xbox_assert(datas != nil);

	err = VE_OK; // pour l'instant est inutile, pourra peut etre servir plus tard.
	fLog->ClearLastProblem();

	if (len < sizeof(FichierDISK))
	{
		errexec = AddProblem(TOP_LengthIsInvalid, numobj+1);
	}
	else
	{
		FichierDISK* fid = (FichierDISK*)p;
		VString tablename;
		if (needswap)
			fid->SwapBytes();
		sLONG nbc = fid->nbcrit;
		if (fid->nom[0] > kMaxTableNameLength || fid->nom[0] <= 0)
		{
			tablename = L"[Table# ";
			VString s2;
			s2.FromLong(numobj+1);
			tablename += s2;
			tablename += L"]";
			errexec = AddProblem(TOP_TableNameLengthIsIvalid, numobj+1);
			fLog->ClearLastProblem();
		}
		else
		{
			tablename.FromBlock(&fid->nom[1], fid->nom[0]*2, VTC_UTF_16);
			if (VDBMgr::GetManager()->IsRunningWakanda())
			{
				//Base4D_NotOpened* xstruct = (fOwner->GetStruct() == nil) ? fOwner : fOwner->GetStruct();
				VString newtablename;
				fOwner->AddTableName(numobj+1, tablename, fLog, &newtablename);
				if (tablename != newtablename)
				{
					newtablename.Truncate(31);
					newtablename.ToBlock(&fid->nom, sizeof(fid->nom), VTC_UTF_16, false, true);
					tablename = newtablename;
				}
			}
			else
			{
				if (fOwner->GetStruct() == nil) // dans le cas des tabledef qui sont dans la structure (les vrais)
					errexec = datas->AddTableName(numobj+1, tablename, fLog, nil);
			}
		}
		if (errexec == VE_OK)
		{
			if (VDBMgr::GetManager()->IsRunningWakanda())
			{
				//Base4D_NotOpened* xstruct = (fOwner->GetStruct() == nil) ? fOwner : fOwner->GetStruct();
				fOwner->AddUUID(fid->ID, tuuid_Table, tablename, numobj+1, 0, fLog);
			}
			else
			{
				if (fOwner->GetStruct() == nil ) // dans le cas des tabledef qui sont dans la structure (les vrais)
					errexec = datas->AddUUID(fid->ID, tuuid_Table, tablename, numobj+1, 0, fLog);
			}
		}
		
		if (errexec == VE_OK)
		{
			if (fid->typ != 0)
				errexec = AddProblem(TOP_TableTypeIsInvalid, numobj+1);
		}

		if (errexec == VE_OK)
		{
			TableDefProblem potentialProblem(TOP_FullyWrong, numobj+1);
			VValueBag* dataBag = nil;
			if (fOwner->GetStruct() != nil) // dans le cas des tabledef qui servent de backup dans le data
			{
				fid->ExtraAddr = 0;
				fid->ExtraLen = 0;
			}
			errexec = fOwner->CheckExtraProp(fid->ExtraAddr, fid->ExtraLen, potentialProblem, fLog, dataBag, numobj+1, -1, DBOH_ExtraTableProperties);
			if (fid->ExtraAddr == -1)
			{
				// problem sur extra prop
				fid->ExtraAddr = 0;
				fid->ExtraLen = 0;
			}

			if (dataBag != nil)
				dataBag->Release();
		}

		if (errexec == VE_OK)
		{
			if (nbc < 0 || nbc > kMaxFields)
				errexec = AddProblem(TOP_Table_NumberOfFieldsIsInvalid, numobj+1);
			else
			{
				sLONG nbc2 = (len - sizeof(FichierDISK)) / sizeof(CritereDISK);
				if (nbc != nbc2)
				{
					errexec = AddProblem(TOP_Table_NumberOfFieldsDoesNotMatchTableLength, numobj+1);
					if (nbc > nbc2)
						nbc = nbc2;
				}

				if (errexec == VE_OK)
				{
					CritereDISK* cid = (CritereDISK*)(((char*)p)+sizeof(FichierDISK));
					for (sLONG i = 1; i <= nbc && errexec == VE_OK; i++, cid++)
					{

						if (cid->typ == DB4D_NoType && cid->nom[0] == 0)
						{
							// rien a faire le champ est detruit
						}
						else
						{
							VString fieldname = tablename;
							fieldname += L".";
							if (needswap)
								cid->SwapBytes();
							if (cid->nom[0] > kMaxFieldNameLength || cid->nom[0] <= 0)
							{
								VString s = L"[Field# ";
								VString s2;
								s2.FromLong(i);
								s += s2;
								s += L"]";
								fieldname += s;
								FieldDefProblem pb(TOP_FieldNameLengthIsInvalid, numobj+1, i);
								errexec = fLog->Add(pb);
								fLog->ClearLastProblem();
							}
							else
							{
								VString fieldname2;
								fieldname2.FromBlock(&cid->nom[1], cid->nom[0]*2, VTC_UTF_16);
								fieldname += fieldname2;
								if (VDBMgr::GetManager()->IsRunningWakanda())
								{
									//Base4D_NotOpened* xstruct = (fOwner->GetStruct() == nil) ? fOwner : fOwner->GetStruct();
									fOwner->AddFieldName(numobj+1, i, cid->typ, fieldname, fLog);
								}
								else
								{
									if (fOwner->GetStruct() == nil) // dans le cas des tabledef qui sont dans la structure (les vrais)
										errexec = datas->AddFieldName(numobj+1, i, cid->typ, fieldname, fLog);
								}
							}

							if (errexec == VE_OK)
							{
								if (VDBMgr::GetManager()->IsRunningWakanda())
								{
									//Base4D_NotOpened* xstruct = (fOwner->GetStruct() == nil) ? fOwner : fOwner->GetStruct();
									fOwner->AddUUID(cid->ID, tuuid_Field, fieldname, numobj+1, i, fLog);
								}
								else
								{
									if (fOwner->GetStruct() == nil) // dans le cas des tabledef qui sont dans la structure (les vrais)
										errexec = datas->AddUUID(cid->ID, tuuid_Field, fieldname, numobj+1, i, fLog);
								}
							}

							if (errexec == VE_OK)
							{
								if (cid->typ != 0)
								{
									CreCritere_Code Code = FindCri(cid->typ);
									if (Code == nil)
									{
										FieldDefProblem pb(TOP_FieldTypeIsInvalid, numobj+1, i);
										errexec = fLog->Add(pb);
									}

								}
							}

							if (errexec == VE_OK)
							{
								FieldDefProblem potentialProblem(TOP_FullyWrong, numobj+1, i);
								VValueBag* dataBag = nil;
								if (fOwner->GetStruct() != nil) // dans le cas des tabledef qui servent de backup dans le data
								{
									cid->ExtraAddr = 0;
									cid->ExtraLen = 0;
								}
								errexec = fOwner->CheckExtraProp(cid->ExtraAddr, cid->ExtraLen, potentialProblem, fLog, dataBag, numobj+1, i, DBOH_ExtraFieldProperties);
								if (cid->ExtraAddr == -1)
								{
									// problem sur extra prop
									cid->ExtraAddr = 0;
									cid->ExtraLen = 0;
								}
								if (dataBag != nil)
									dataBag->Release();
							}
						}
					}
				}
			}
		}
	}

	if (errexec == VE_OK && err == VE_OK && fLog->IsCompacting() && !fLog->SomeProblem())
	{
		Base4D* target = fLog->GetTargetCompact();

		if (fOwner->GetStruct() == nil)
		{
			StructElemDef* e = new StructElemDef(target->GetStructure(), numobj, fxSignature);
			e->CopyData(p, len); 
			//e->SetSwapWhileSaving(needswap);
			e->libere();
			err = target->SaveTableRef(e);
		}
		else
		{
			//if (!VDBMgr::GetManager()->IsRunningWakanda())
			{
				StructElemDef* e = new StructElemDef(target, -1, fxSignature);
				e->CopyData(p, len); 
				e->libere();
				err = target->SaveTableRefInDatas(e);
			}
		}
	}

	return errexec;
}


VError RelationDefChecker::PrepareAddrTable()
{
	VError err = VE_OK;

	// pas besoin de preparer la table d'adresses
	// les relations n'ont pas besoin de numero d'ordre

	return VE_OK;
}

VError RelationDefChecker::CheckObj(void* p, sLONG len, sLONG numobj, Boolean needswap, VError& err)
{
	Base4D_NotOpened* datas = fOwner->GetDatas();
	xbox_assert(datas != nil);
	VError errexec = VE_OK;
	VConstPtrStream buf(p, len);
	fLog->ClearLastProblem();
	errexec = buf.OpenReading();
	if (errexec == VE_OK)
	{
		buf.SetNeedSwap(needswap);
			
				
		RelOnDisk dd;
		VString relname, relnameopposite, sourcename, destname;

		err = buf.GetData(&dd, (uLONG)sizeof(dd));
		if (err == VE_OK)
		{
			if (buf.NeedSwap())
				dd.SwapBytes();

			err = relname.ReadFromStream(&buf);
			err = relnameopposite.ReadFromStream(&buf);
			if (err == VE_OK)
			{
				errexec = datas->AddUUID(dd.ID, tuuid_Relation, relname, numobj, 0, fLog);

				if (errexec == VE_OK)
				{
					if (!datas->ExistUUID(dd.SourceField, tuuid_Field, &sourcename))
					{
						errexec = AddProblem(TOP_SourceFieldIDCannotBeFound, numobj);
					}
				}

				if (errexec == VE_OK)
				{
					if (!datas->ExistUUID(dd.DestField, tuuid_Field, &destname))
					{
						errexec = AddProblem(TOP_DestFieldIDCannotBeFound, numobj);
					}
				}

				if (errexec == VE_OK)
				{
					RelationDefProblem potentialProblem(TOP_FullyWrong, numobj);
					VValueBag* dataBag = nil;
					errexec = fOwner->CheckExtraProp(dd.ExtraAddr, dd.ExtraLen, potentialProblem, fLog, dataBag, -1, -1, DBOH_ExtraRelationProperties);
					if (dd.ExtraAddr == -1)
					{
						// problem avec extra prop
						dd.ExtraAddr = 0;
						dd.ExtraLen = 0;
					}
					*((RelOnDisk*)p) = dd;
					if (needswap)
						((RelOnDisk*)p)->SwapBytes();
					if (dataBag != nil)
						dataBag->Release();
				}

				if (dd.MultipleFields == 1 && err == VE_OK && errexec == VE_OK)
				{
					sLONG nbfields;

					err = buf.GetLong(nbfields);
					if (err == VE_OK)
					{
						if (nbfields < 2)
						{
							errexec = AddProblem(TOP_RelationListIsDamaged);
						}
						else
						{
							for (sLONG i = 1; i < nbfields && err == VE_OK && errexec == VE_OK; i++)  // on part de 1 car le premier element est deja dedans (fSource)
							{
								VUUIDBuffer bufid;
								err = buf.GetData(bufid.fBytes, sizeof(bufid.fBytes));
								if (err == VE_OK)
								{
									VString s;
									if (datas->ExistUUID(bufid, tuuid_Field, &s))
									{
										sourcename += L" , ";
										sourcename += s;
									}
									else
									{
										errexec = AddProblem(TOP_SourceFieldIDCannotBeFound, numobj);
									}
								}
								else
								{
									errexec = AddProblem(TOP_RelationListIsDamaged);
								}
							}
						}
					}
					else
					{
						errexec = AddProblem(TOP_RelationListIsDamaged);
					}

					sLONG nbfields2;

					if (err == VE_OK && errexec)
					{
						err = buf.GetLong(nbfields2);
						if (err == VE_OK)
						{
							if (nbfields2 != nbfields)
							{
								errexec = AddProblem(TOP_NumberOfSourcesDoesNotNumberOfDests, numobj);
							}
						}
						else
						{
							errexec = AddProblem(TOP_RelationListIsDamaged);
						}
					}

					if (err == VE_OK)
					{
						for (sLONG i = 1; i < nbfields2 && err == VE_OK && errexec == VE_OK; i++)  // on part de 1 car le premier element est deja dedans (fDestination)
						{
							VUUIDBuffer bufid;
							err = buf.GetData(bufid.fBytes, sizeof(bufid.fBytes));
							if (err == VE_OK)
							{
								VString s;
								if (datas->ExistUUID(bufid, tuuid_Field, &s))
								{
									destname += L" , ";
									destname += s;
								}
								else
								{
									errexec = AddProblem(TOP_DestFieldIDCannotBeFound, numobj);
								}
							}
							else
							{
								errexec = AddProblem(TOP_RelationListIsDamaged);
							}
						}
					}
				}

			}
			else
			{
				errexec = AddProblem(TOP_RelationListIsDamaged);
			}
		}
		else
		{
			errexec = AddProblem(TOP_RelationListIsDamaged);
		}
			

		buf.CloseReading();
	}

	if (errexec == VE_OK && err == VE_OK && fLog->IsCompacting() && !fLog->SomeProblem())
	{
		Base4D* target = fLog->GetTargetCompact();

		StructElemDef* e = new StructElemDef(target->GetStructure(), -1, fxSignature);
		e->CopyData(p, len); 
		e->SetSwapWhileSaving(needswap);
		e->libere();
		err = target->SaveRelationDef(e);
	}

	return errexec;
}



VError DataTableChecker::PrepareAddrTable()
{
	VError err = VE_OK;

	// pas besoin de preparer la table d'adresses
	// les datatables n'ont pas besoin de numero d'ordre

	return VE_OK;
}


VError DataTableChecker::CheckObj(void* p, sLONG len, sLONG numobj, Boolean needswap, VError& err)
{
	xbox_assert(false);
	return VE_OK;
}


VError DataTableChecker::CheckObjWithAddr(DataAddr4D ou, sLONG len, sLONG numobj, VError& err)
{
	VError errexec = VE_OK;

	if (fOwner->IsAddrValid(ou, false))
	{
		fLog->MarkAddr_DataTable(ou, len, numobj);
		if (len != (sizeof(DataTableDISK) + kSizeDataBaseObjectHeader))
		{
			errexec = AddProblem(TOP_DataTableHeader_LengthIsInvalid, numobj+1);
		}
		else
		{
			DataFile_NotOpened* df = new DataFile_NotOpened(GetOwner(), ou, numobj+1);
			if (df == nil)
			{
				errexec = memfull;
			}
			else
			{
				(*fDataTables)[numobj] = df;
				if (fLog->IsCompacting())
				{
					fLog->ClearLastProblem();
					Base4D* target = fLog->GetTargetCompact();
					if (fOwner->GetStruct() == nil)
						target = target->GetStructure();
					DataTableDISK dfd;

					DataBaseObjectHeader tag;
					VError err = tag.ReadFrom(GetOwner(), ou);
					if (err != VE_OK)
					{
						DataTableProblem pb(TOP_DataTableHeader_PhysicalDataIsInvalid, numobj+1);
						errexec = fLog->Add(pb);
					}

					if (err == VE_OK && errexec == VE_OK)
					{
						err = tag.ValidateTag(DBOH_DataTable, numobj+1, -3);
						if (err != VE_OK)
						{
							DataTableProblem pb(TOP_DataTableHeader_TagIsInvalid, numobj+1);
							errexec = fLog->Add(pb);
						}
					}

					if (err == VE_OK && errexec == VE_OK)
					{
						err = GetOwner()->readlong(&dfd, sizeof(dfd), ou, kSizeDataBaseObjectHeader);
						if (err == VE_OK)
						{
							err = tag.ValidateCheckSum(&dfd, sizeof(dfd));
							if (err == VE_OK)
							{
								if (tag.NeedSwap())
									dfd.SwapBytes();
							}
						}
					}

					if (err == VE_OK && errexec == VE_OK && !fLog->SomeProblem())
					{
						dfd.addrBlobtabaddr = 0;
						dfd.addrtabaddr = 0;
						dfd.debutBlobTrou = kFinDesTrou;
						dfd.debuttrou = kFinDesTrou;
						dfd.nbBlob = 0;
						dfd.nbfic = 0;
						DataTable* dftarget = nil;
						//if (target->GetStructure() == nil)
						{
							if (VDBMgr::GetManager()->IsRunningWakanda())
							{
								VString tname;
								if (fOwner->ExistUUID(dfd.TableDefID, tuuid_Table, &tname, nil, nil, nil))
								{
									Table* tt = target->FindAndRetainTableRef(tname);
									if (tt != nil)
									{
										dftarget = RetainRefCountable(tt->GetDF());
										tt->Release();
									}
								}
							}
							else
								dftarget = target->RetainDataTableByUUID(dfd.TableDefID);
						}
						if (dftarget == nil)
						{
							dftarget = target->CreateDataTable(nil, err, 0, &dfd, -1);
						}
						else
						{
							dftarget->SetSeqID(dfd.SeqNum_ID);
						}
						if (dftarget != nil)
							dftarget->Release();
					}
				}
			}
		}

	}
	else
	{
		errexec = AddProblem(TOP_DataTableHeader_AddrIsInvalid, numobj+1);
	}
	return errexec;
}



VError SeqNumChecker::PrepareAddrTable()
{
	VError err = VE_OK;

	// pas besoin de preparer la table d'adresses
	// les seqnums n'ont pas besoin de numero d'ordre

	return VE_OK;
}


VError SeqNumChecker::CheckObj(void* p, sLONG len, sLONG numobj, Boolean needswap, VError& err)
{
	xbox_assert(false);
	return VE_OK;
}


VError SeqNumChecker::CheckObjWithAddr(DataAddr4D ou, sLONG len, sLONG numobj, VError& err)
{
	VError errexec = VE_OK;
	DataBaseObjectHeader tag;

	fLog->ClearLastProblem();
	if (fOwner->IsAddrValid(ou, false))
	{
		fLog->MarkAddr_SeqNum(ou, len, numobj);

		sLONG typ;

		if (len < (sizeof(AutoSeqSimpleOnDisk) + kSizeDataBaseObjectHeader))
		{
			errexec = AddProblem(TOP_LengthIsInvalid, numobj);
		}

		if (errexec == VE_OK)
		{
			err = tag.ReadFrom(fOwner, ou);
			if (err != VE_OK)
			{
				errexec = AddProblem(TOP_PhysicalDataIsInvalid, numobj);
			}
		}

		if (err == VE_OK && errexec == VE_OK)
		{
			err = tag.ValidateTag(DBOH_AutoSeqNumberSimple, numobj, -1);
			if (err != VE_OK)
				err = tag.ValidateTag(DBOH_AutoSeqNumberNoHole, numobj, -1);
			if (err != VE_OK)
			{
				errexec = AddProblem(TOP_TagIsInvalid, numobj);
			}
		}

		if (err == VE_OK && errexec == VE_OK)
		{
			err = fOwner->readlong(&typ, sizeof(typ), ou, kSizeDataBaseObjectHeader);
			if (err != VE_OK)
			{
				errexec = AddProblem(TOP_PhysicalDataIsInvalid, numobj);
			}
		}

		VString seqnumName = L"Auto Sequence Number# ";
		VString s2;
		s2.FromLong(numobj);
		seqnumName += s2;

		if (err == VE_OK && errexec == VE_OK)
		{
			if (tag.NeedSwap())
				typ = SwapLong(typ);
			switch ((DB4D_AutoSeq_Type)typ)
			{
				case DB4D_AutoSeq_Simple:
					{
						AutoSeqSimpleOnDisk ASSOD;

						err=fOwner->readlong(&ASSOD, sizeof(ASSOD), ou, kSizeDataBaseObjectHeader);
						if (err != VE_OK)
						{
							errexec = AddProblem(TOP_PhysicalDataIsInvalid, numobj);
						}

						if (err == VE_OK && errexec == VE_OK)
						{
							err = tag.ValidateCheckSum(&ASSOD, sizeof(ASSOD));
							if (err != VE_OK)
							{
								errexec = AddProblem(TOP_CheckSumIsInvalid, numobj);
							}
						}

						if (err == VE_OK && errexec == VE_OK)
						{
							if (tag.NeedSwap())
								ASSOD.SwapBytes();
							errexec = fOwner->AddUUID(ASSOD.ID, tuuid_AutoSeqNum, seqnumName, numobj, 0, fLog);
						}

						if (err == VE_OK && errexec == VE_OK)
						{
							if (tag.GetLen() + kSizeDataBaseObjectHeader != len)
							{
								errexec = AddProblem(TOP_TagLengthDoesNotMatch, numobj);
							}
						}

						if (err == VE_OK && errexec == VE_OK)
						{
							if (tag.GetLen() != sizeof(AutoSeqSimpleOnDisk))
							{
								errexec = AddProblem(TOP_LengthIsInvalid, numobj);
							}
						}
						
					}
					break;

				case DB4D_AutoSeq_NoHole:
					{
						AutoSeqNoHoleOnDisk ASSOD;

						if (err != VE_OK)
						{
							errexec = AddProblem(TOP_PhysicalDataIsInvalid, numobj);
						}

						if (err == VE_OK && errexec == VE_OK)
						{
							err = tag.ValidateCheckSum(&ASSOD, sizeof(ASSOD));
							if (err != VE_OK)
							{
								errexec = AddProblem(TOP_CheckSumIsInvalid, numobj);
							}
						}

						if (err == VE_OK && errexec == VE_OK)
						{
							if (tag.NeedSwap())
								ASSOD.SwapBytes();
							errexec = fOwner->AddUUID(ASSOD.ID, tuuid_AutoSeqNum, seqnumName, numobj, 0, fLog);
						}

						if (err == VE_OK && errexec == VE_OK)
						{
							if (adjuste(tag.GetLen() + kSizeDataBaseObjectHeader) != adjuste(len))
							{
								errexec = AddProblem(TOP_TagLengthDoesNotMatch, numobj);
							}
						}

						if (err == VE_OK && errexec == VE_OK)
						{
							if (tag.GetLen() != sizeof(AutoSeqNoHoleOnDisk))
							{
								errexec = AddProblem(TOP_LengthIsInvalid, numobj);
							}
						}

						if (err == VE_OK && errexec == VE_OK)
						{
							if (ASSOD.nbHoles < 0)
							{
								errexec = AddProblem(TOP_LengthIsInvalid, numobj);
							}
						}

						if (err == VE_OK && errexec == VE_OK)
						{
							if (adjuste(len) != adjuste(sizeof(AutoSeqNoHoleOnDisk) + kSizeDataBaseObjectHeader + 8*ASSOD.nbHoles))
							{
								errexec = AddProblem(TOP_LengthIsInvalid, numobj);
							}
						}

					}
					break;

				default:
					{
						errexec = AddProblem(TOP_SeqNumTypeIsInvalid, numobj);
					}
					break;

			}
		}
	}
	else
	{
		errexec = AddProblem(TOP_AddrIsInvalid, numobj);
	}

	if (fLog->IsCompacting() && !fLog->SomeProblem() && errexec == VE_OK)
	{
		Base4D* target = fLog->GetTargetCompact();
		if (GetOwner()->GetStruct() == nil)
			target = target->GetStructure();
		void* p = GetFastMem(len, false, 'tmp1');
		if (testAssert(p != nil))
		{
			err = fOwner->readlong(p, len, ou, 0);
			if (err == VE_OK)
			{
				DataAddr4D ou2 = target->findplace(len, nil, err, 0, nil);
				if (ou2 > 0)
				{
					sLONG newnum;
					target->SetSeqNumAddr(-2, ou2, len, newnum);
					((DataBaseObjectHeaderOnDisk*)p)->pos = newnum;
					if (tag.NeedSwap())
					{
						ByteSwapLong(&((DataBaseObjectHeaderOnDisk*)p)->pos);
					}
					err = target->writelong(p, len, ou2, 0, nil, -1);
				}
			}
			FreeFastMem(p);
		}
	}

	return errexec;
}


void Base4D_NotOpened::GetIndexName(sLONG n, VString& outName) const
{
	Index_NonOpened* ind = nil;
	if (n >= 0)
	{
		if (n >= 0 && n < (sLONG)fIndsInStruct.size())
		{
			ind = fIndsInStruct[n];
		}
	}
	else
	{
		n = -n - 1;
		if (ind == nil && n >= 0 && n < (sLONG)fInds.size())
		{
			ind = fInds[n];
		}
	}
	if (ind == nil)
	{
		outName = L"[index #";
		VString s;
		s.FromLong(n);
		outName += s;
		outName += L"]";
	}
	else
	{
		ind->GetName(outName);
	}
}

sLONG Base4D_NotOpened::CountTables(Boolean& outInfoIsValid, ToolLog *log)
{
	outInfoIsValid = true;
	return (sLONG) fTableNames.size();
}


void Base4D_NotOpened::GetTableName(sLONG inTableNum, VString& outName, Boolean& outInfoIsValid, ToolLog *log)
{
	if (ExistTable(inTableNum, &outName))
		outInfoIsValid = true;
}

sLONG Base4D_NotOpened::CountIndexes(Boolean& outInfoIsValid, ToolLog *log)
{
	outInfoIsValid = true;
	sLONG result = (sLONG) fIndsInStruct.size();
	if (result == 0)
		result = (sLONG) fInds.size();
	return result;
}

sLONG Base4D_NotOpened::GetTables(std::vector<sLONG>& outTables,Boolean& outInfoIsValid,ToolLog* log)
{
	outInfoIsValid = true;

	outTables.reserve(fTableNames.size());

	for (TableNamesCollection::iterator cur = fTableNames.begin(); cur != fTableNames.end(); ++cur)
	{
		outTables.push_back(cur->first);
	}

	return (sLONG) outTables.size();
}

VError Base4D_NotOpened::GetTableFields(sLONG inTableNum, std::vector<sLONG>& outFields, Boolean& outInfoIsValid, ToolLog *log)
{
	VError err = VE_OK;

	outInfoIsValid = false;

	if (ExistTable(inTableNum))
	{
		outInfoIsValid = true;
		for (FieldNamesCollection::iterator cur = fFieldNames.begin(), end = fFieldNames.end(); cur != end; cur++)
		{
			const Field_NotOpened_Ref* x = &(cur->first);
			if (x->fTablenum == inTableNum)
				outFields.push_back(x->fFieldnum);
		}
	}
	return err;
}

VError Base4D_NotOpened::GetTableIndexes(sLONG inTableNum, std::vector<sLONG>& outIndexes, Boolean& outInfoIsValid, ToolLog *log)
{
	VError err = VE_OK;

	outInfoIsValid = true;
	IndexNonOpenedCollection* indcol = &fIndsInStruct;
	if (fIndsInStruct.empty())
		indcol = &fInds;

	for (IndexNonOpenedCollection::iterator cur = indcol->begin(), end = indcol->end(); cur != end && err == VE_OK; cur++)
	{
		Index_NonOpened* ind = *cur;
		if (ind != nil)
		{
			if (ind->IsSourceValid())
			{
				if (ind->GetTarget() == inTableNum)
				{
					try
					{
						outIndexes.push_back(ind->GetNum());
					}
					catch (...)
					{
						err = memfull;
					}
				}
			}
		}
	}

	return err;
}


sLONG Base4D_NotOpened::CountRecordsInTable(sLONG inTableNum, Boolean& outInfoIsValid, ToolLog* log)
{
	sLONG result = -1;

	outInfoIsValid = false;

	VError errexec = LoadDataTables(log);
	if (errexec == VE_OK && fIsDataTablesHeaderValid)
	{
		if (inTableNum >= 1 && inTableNum <= (sLONG)fDataTables.size())
		{
			DataFile_NotOpened* df = fDataTables[inTableNum-1];
			if (df != nil)
			{
				errexec = df->CountRecords(log, result, outInfoIsValid);
			}
		}
	}

	return result;
}


sLONG Base4D_NotOpened::CountFieldsInTable(sLONG inTableNum, Boolean& outInfoIsValid, ToolLog* log)
{
	sLONG res = 0;
	if (ExistTable(inTableNum))
	{
		outInfoIsValid = true;
		for (FieldNamesCollection::iterator cur = fFieldNames.begin(), end = fFieldNames.end(); cur != end; cur++)
		{
			const Field_NotOpened_Ref* x = &(cur->first);
			if (x->fTablenum == inTableNum)
				res++;
		}
	}
	return res;
}


VError Base4D_NotOpened::CheckBitTables(ToolLog* log)
{
	VError err = VE_OK;
	ObjectDiskPosInfoCollection* addrs = log->GetAddressKeeper();
	if (addrs != nil)
	{
		err = addrs->CheckOverlaps(log, nil);
	}

	return err;
}


VFolder* Base4D_NotOpened::GetName(VString& outName)
{
	VFolder* result = nil;
	outName.Clear();
	if (fIsOpen && !segs.empty())
	{
		const VFile* ff = segs[1]->GetVFile();
		if (ff != nil)
		{
			result = ff->RetainParentFolder();
			ff->GetNameWithoutExtension(outName);
			if (fStruct == nil)
				outName += L"_struct";
		}
	}
	return result;
}


VError Base4D_NotOpened::GetIndexInfo(sLONG inIndexNum,XBOX::VString& outName,sLONG& outTableNum,std::vector<sLONG>& outFields, Boolean& outInfoIsValid, ToolLog* log)
{
	VError err = VE_OK;

	//for (IndexNonOpenedCollection::iterator cur = fInds.begin(), end = fInds.end(); cur != end && err == VE_OK; cur++)
	{
		//Index_NonOpened* ind = *cur;
		Index_NonOpened* ind = nil;
		if (inIndexNum >= 0 && inIndexNum < (sLONG)fIndsInStruct.size())
			ind = fIndsInStruct[inIndexNum];
		if (ind == nil && inIndexNum >= 0 && inIndexNum < (sLONG)fInds.size())
			ind = fInds[inIndexNum];
		if (ind != nil)
		{
			if (ind->IsSourceValid())
			{
				if (testAssert(ind->GetNum() == inIndexNum))
				{
					try
					{
						outTableNum = ind->GetTarget();
						ind->GetFields(outFields);
						ind->GetName(outName);
						outInfoIsValid = true;
					}
					catch (...)
					{
						err = memfull;
					}

					//break; // get info on one specific index
				}
			}
		}
	}
	return err;
}

VError Base4D_NotOpened::GetFieldInfo(sLONG inTableNum, sLONG inFieldNum, VString& outName, sLONG& outType, std::vector<sLONG>& outIndexes, Boolean& outInfoIsValid, ToolLog* log)
{
	VError err = VE_OK;
	if (ExistField(inTableNum, inFieldNum, &outName, &outType))
	{
		outInfoIsValid = true;
	}

	IndexNonOpenedCollection* indcol = &fIndsInStruct;
	if (fIndsInStruct.empty())
		indcol = &fInds;

	for (IndexNonOpenedCollection::iterator cur = indcol->begin(), end = indcol->end(); cur != end && err == VE_OK; cur++)
	{
		Index_NonOpened* ind = *cur;
		if (ind != nil)
		{
			if (ind->IsSourceValid())
			{
				if (ind->GetTarget() == inTableNum && ind->MatchField(inFieldNum))
				{
					try
					{
						outIndexes.push_back(ind->GetNum());
					}
					catch (...)
					{
						err = memfull;
					}
				}
			}
		}
	}
	return err;
}

class RecInfo
{
	public:
		inline RecInfo()
		{
			fTimeStamp = 0;
			fAddr = 0;
			fLen = 0;
		};

		uLONG8 fTimeStamp;
		DataAddr4D fAddr;
		sLONG fLen;

};


class BlobNumInfo
{
	public:
		inline BlobNumInfo()
		{
			oldBlobNum = -1;
			newBlobNum = -1;
		}

		sLONG oldBlobNum;
		sLONG newBlobNum;
};


class DataTableWithSeqNumMatch : public pair<VUUIDBuffer, VUUIDBuffer>
{
public:
	DataTableWithSeqNumMatch()
	{
		first.FromLong(0);
		second.FromLong(0);
	}

	DataTableWithSeqNumMatch(VUUIDBuffer& datatableID, VUUIDBuffer& seqnumID)
	{
		first = datatableID;
		second = seqnumID;
	}
};

typedef vector<RecInfo> RecInfoVector;
typedef vector<RecInfoVector> RecInfoMatrix;
typedef map<VUUIDBuffer, sLONG> TableIDsMap;
typedef vector<DataTableWithSeqNumMatch> DataTablesVector;
typedef vector<BlobNumInfo> BlobNumVector;
typedef vector<BlobNumVector> BlobNumMatrix;
typedef map<VUUIDBuffer, AutoSeqSimpleOnDisk> SeqNumMap;


class RecoverInfo
{
	public:

		RecoverInfo()
		{
			olddatafile = nil;
		}

		~RecoverInfo()
		{
			if (olddatafile != nil)
				delete olddatafile;
		}

		VError SetRecInfo(RecInfoVector& arr, sLONG pos, DataAddr4D ou, sLONG len, uLONG8 stamp)
		{
			try
			{
				if (pos >= (sLONG)arr.size())
					arr.resize(pos+1, RecInfo());
				RecInfo* recinf = &(arr[pos]);
				if (stamp > recinf->fTimeStamp)
				{
					recinf->fAddr = ou;
					recinf->fLen = len;
					//if (stamp > recinf->fTimeStamp)
						recinf->fTimeStamp = stamp;
				}
			}
			catch (...)
			{
				return memfull;
			}
			return VE_OK;
		}


		void* AddRecInfoFromDisk(VFileDesc* f, DataAddr4D ou, RecordHeader& tag, RecInfoVector& arr, VError& errout)
		{
			errout = VE_OK;
			VError err;
			sLONG len = tag.GetLen();
			sLONG numobj = tag.GetPos();
			void* p = GetFastMem(len, false, 'elem');
			if (p != nil)
			{
				err = f->GetData(p, len, ou+kSizeDataBaseObjectHeader);
				if (err == VE_OK)
				{
					err = tag.ValidateCheckSum(p, len);
					if (err == VE_OK)
					{
						errout = SetRecInfo(arr, numobj, ou, len, tag.GetTimeStamp());
					}
				}

			}
			else
				errout = memfull;

			if (errout != VE_OK && p != nil)
			{
				FreeFastMem(p);
				p = nil;
			}
			return p;
		}


		RecInfoMatrix recs;
		RecInfoMatrix blobs;
		RecInfoVector tabledefs;
		RecInfoVector seqnumdefs;
		RecInfoVector datatableheaders;
		TableIDsMap tableIDs;
		DataTablesVector datatableinfos;
		vector<sLONG> tableMapping;
		vector<Table*> tableForData;
		SeqNumMap seqnums;
		VFileDesc* olddatafile;

};


void Base4D::DisposeRecoverInfo(void* xrecover)
{
	RecoverInfo* recover = (RecoverInfo*)xrecover;
	delete recover;
}


VError Base4D::RecoverByTags(VValueBag& inBag, void* inRecover, ToolLog *log)
{
	VError err = VE_OK;

	RecoverInfo* xrecover = (RecoverInfo*)inRecover;
	RecoverInfo& recover = *xrecover;

	VFileDesc* olddatafile = recover.olddatafile;


	BaseTaskInfo* context = new BaseTaskInfo(this, nil, nil, nil);

	VBagArray* tables = inBag.GetElements(DB4DBagKeys::matching_datatable);
	if (tables != nil)
	{
		sLONG nbmatch = tables->GetCount();
		for (sLONG i = 1; i <= nbmatch; i++)
		{
			VValueBag* tableBag = tables->GetNth(i);
			if (tableBag != nil)
			{
				sLONG dataTableNum;
				sLONG tableNum;
				if (tableBag->GetLong(DB4DBagKeys::datatable_num, dataTableNum) && tableBag->GetLong(DB4DBagKeys::table_num, tableNum))
				{
					if (dataTableNum > 0 && dataTableNum <= recover.tableMapping.size())
					{
						recover.tableMapping[dataTableNum-1] = tableNum;
					}
				}
			}
		}
	}

	bool toutesLesTablesMatch = true;

	recover.tableForData.resize(recover.tableMapping.size(), nil);

	vector<Table*>::iterator curtable = recover.tableForData.begin();
	for (vector<sLONG>::iterator cur = recover.tableMapping.begin(), end = recover.tableMapping.end(); cur != end; cur++, curtable++)
	{
		sLONG tablenum = *cur;
		if (tablenum == 0)
		{
			//toutesLesTablesMatch = false;
		}
		else
		{
			if (tablenum != -1)
			{
				Table* tt = RetainTable(tablenum);
				if (tt == nil)
					toutesLesTablesMatch = false;
				else
				{
					*curtable = tt;
					tt->Release();
				}
			}
		}
	}

	if (toutesLesTablesMatch)
	{
		OccupableStack* curstack = gOccPool.GetStackForCurrentThread();

		DelayAllIndexes();

		sLONG nbtablewithdata = (sLONG)recover.recs.size();
		if (nbtablewithdata > recover.blobs.size())
			recover.blobs.resize(nbtablewithdata);
		for (sLONG i = 1; i <= nbtablewithdata && err == VE_OK; i++)
		{
			Table* tt = recover.tableForData[i-1];
			if (tt != nil)
			{
				RecInfoVector& blobs = recover.blobs[i-1];
				RecInfoVector& recs = recover.recs[i-1];
				sLONG nbrecs = (sLONG)recs.size();
				DataTable* df = tt->GetDF();

				SeqNumMap::iterator foundseq = recover.seqnums.find(recover.datatableinfos[i-1].second);
				if (foundseq != recover.seqnums.end())
				{
					AutoSeqNumber* seqnum = df->GetSeqNum(nil);
					if (seqnum != nil)
					{
						seqnum->SetStartingValue(foundseq->second.InitialValue);
						seqnum->SetCurrentValue(foundseq->second.CurValue);
					}
				}

				Bittab usedBlobs;
				if (nbrecs > 0)
				{
					VString progressMess;
					if (log != nil)
					{
						VString secondpart;
						log->GetVerifyOrCompactString(31, progressMess);
						log->GetVerifyOrCompactString(33, secondpart);

						FormatStringWithParams(secondpart, 2 , i, nbtablewithdata);
						progressMess += secondpart;
							//progressMess.AppendPrintf(" (%d/%d)", i, nbtablewithdata);
						log->OpenProgressSession(progressMess, nbrecs);
					}

					err = df->ResizeTableRecs(curstack, nbrecs);

					for (sLONG j = 0; j < nbrecs && err == VE_OK; j++)
					{
						if (log != nil)
							err = log->Progress(j);
						if (err == VE_OK)
						{
							RecInfo& info = recs[j];
							if (info.fAddr != 0)
							{
								VError err2;
								FicheOnDisk* ficD = FicheOnDisk::BuildRecordFromLoad(err2, i, j, info.fAddr, olddatafile, tt->GetDF());
								if (ficD != nil)
								{
									for (sLONG k = 1, nbfield = ficD->GetNbCrit(); k <= nbfield; k++)
									{
										sLONG whattype;
										void* data = ficD->GetDataPtr(k, &whattype);
										if (data != nil)
										{
											if ( (whattype == VK_TEXT || whattype == VK_TEXT_UTF8 || whattype == VK_BLOB || whattype == VK_BLOB_DB4D || whattype == VK_IMAGE)
												&& (ficD->GetActualFieldLen(k) == 4))
											{
												sLONG blobnum = *((sLONG*)data);
												if (blobnum >= 0)
												{
													usedBlobs.Set(blobnum);
													if (blobnum < blobs.size())
													{
														RecInfo& blobinfo = blobs[blobnum];
														if (blobinfo.fAddr == 0)
															*((sLONG*)data) = -1;
													}
												}
											}

										}
									}
									
									err = df->SaveRawRecord(curstack, j, ficD->GetDataBegining(), ficD->GetDataLen(), ficD->GetNbCrit(), context);
									ficD->Release();

								}
							}
						}
					}

					df->NormalizeTableRecs(curstack);

					if (log != nil)
						log->CloseProgressSession();
				}

				sLONG nbblob = (sLONG)blobs.size();
				if (nbblob > 0)
				{
					VString progressMess;
					if (log != nil)
					{
						VString secondpart;
						log->GetVerifyOrCompactString(32, progressMess);
						log->GetVerifyOrCompactString(33, secondpart);
						FormatStringWithParams(secondpart, 2 , i, nbtablewithdata);
						progressMess += secondpart;
						log->OpenProgressSession(progressMess, nbblob);
					}
					err = df->ResizeTableBlobs(curstack, nbblob);
					if (err == VE_OK)
					{
						sLONG numblob = 0;
						for (RecInfoVector::iterator cur = blobs.begin(), end = blobs.end(); cur != end && err == VE_OK; cur++, numblob++)
						{
							if (log != nil)
								err = log->Progress(numblob);
							DataAddr4D ou = cur->fAddr;
							if (err == VE_OK)
							{
								if (ou != 0)
								{
									DataBaseObjectHeader tag;
									err = tag.ReadFrom(olddatafile, ou);
									if (err == VE_OK)
									{
										Boolean isBlob = tag.Match(DBOH_Blob);
										Boolean isText = tag.Match(DBOH_BlobText);
										Boolean isPict = tag.Match(DBOH_BlobPict);

										if (isBlob || isText || isPict)
										{
											if (usedBlobs.isOn(numblob))
											{
												sLONG len = tag.GetLen();

												void* p = GetFastMem(len+2, false, 'blbx');
												if (p != nil)
												{
													err = olddatafile->GetData(p, len, ou+kSizeDataBaseObjectHeader+4);
													if (err == VE_OK)
													{
														DataBaseObjectType type = isText ? DBOH_BlobText : (isPict ? DBOH_BlobPict : DBOH_Blob);													
														err = df->SaveRawBlob(curstack, type, numblob, p, len, context);
													}
													FreeFastMem(p);
												}
											}
											else
											{
												ou = ou; // break here
											}
										}
									}
								}
							}
						}
					}
					df->NormalizeTableBlobs(curstack);
					if (log != nil)
						log->CloseProgressSession();
				}

			}
		}
		FlushBase();
		vector<IndexInfo*> indexlist;
		AwakeAllIndexes(nil, true, indexlist);
		for (vector<IndexInfo*>::iterator cur = indexlist.begin(), end = indexlist.end(); cur != end; cur++)
			(*cur)->Release();
	}
	else
		err = ThrowError(VE_DB4D_SOME_DATATABLES_HAVE_NO_MATCH, noaction);

	context->Release();

	return err;
}


VError Base4D::ScanToRecover(VFile* inOldDataFile, VValueBag& outBag, void* &outRecover, ToolLog *log)
{
	VError errout = VE_OK;
	VError err = VE_OK;

	RecoverInfo* recoverout = new RecoverInfo();
	RecoverInfo& recover = *recoverout;
	outRecover = (void*)recoverout;

	VFileDesc* olddatafile;
	errout = inOldDataFile->Open(FA_READ, &olddatafile);
	if (errout == VE_OK)
	{
		recover.olddatafile = olddatafile;
		DataAddr4D finfic;
		finfic = olddatafile->GetSize();
		if (err == VE_OK)
		{
			VString progressMess;
			if (log != nil)
			{
				log->GetVerifyOrCompactString(30, progressMess);
				log->OpenProgressSession(progressMess, finfic);
			}
			DataAddr4D ou = 0;
			while (ou < finfic /*&& err == VE_OK*/)
			{
				if (log != nil)
					err = log->Progress(ou);
				if (err == VE_OK)
				{
					Boolean okblock = false;
					RecordHeader tag;
					err = tag.ReadFrom(olddatafile, ou);
					if (err == VE_OK)
					{
						if (tag.Match(DBOH_Record))
						{
							sLONG numrec = tag.GetPos();
							sLONG numtable = tag.GetParent();
							sLONG len = tag.GetLen()+(tag.GetNbFields()*sizeof(ChampHeader));
							void* p = GetFastMem(len, false, 'Rec1');
							if (p != nil)
							{
								err = olddatafile->GetData(p, len, ou+kSizeRecordHeader);
								if (err == VE_OK)
								{
									err = tag.ValidateCheckSum(p, tag.GetLen());
									if (err == VE_OK)
									{
										try
										{
											if (numtable>(sLONG)recover.recs.size())
												recover.recs.resize(numtable);
											/*
											if (numtable>(sLONG)recover.datatableheaders.size())
												recover.datatableheaders.resize(numtable);

											if (tag.GetTimeStamp() > recover.datatableheaders[numtable-1].fTimeStamp)
												recover.datatableheaders[numtable-1].fTimeStamp = tag.GetTimeStamp();
												*/

											errout = recover.SetRecInfo(recover.recs[numtable-1], numrec, ou, len, tag.GetTimeStamp());
										}
										catch (...)
										{
											errout = memfull;
										}

										okblock = true;
										ou = ou + ((len+kSizeRecordHeader+ktaillebloc-1)&(-ktaillebloc));

									}
								}
								FreeFastMem(p);
							}

						}
						else
						{
							if (tag.Match(DBOH_Blob) || tag.Match(DBOH_BlobPict) || tag.Match(DBOH_BlobText))
							{
								sLONG numblob = tag.GetPos();
								sLONG numtable = tag.GetParent();
								sLONG len = tag.GetLen();
								void* p = GetFastMem(len+4, false, 'Blb1');
								if (p != nil)
								{
									err = olddatafile->GetData(p, len+4, ou+kSizeDataBaseObjectHeader);
									if (err == VE_OK)
									{
										err = tag.ValidateCheckSum(((char*)p)+4, len);
										if (err == VE_OK)
										{
											try
											{
												if (numtable>(sLONG)recover.blobs.size())
													recover.blobs.resize(numtable);
												errout = recover.SetRecInfo(recover.blobs[numtable-1], numblob, ou, len, tag.GetTimeStamp());
											}
											catch (...)
											{
												errout = memfull;
											}
											okblock = true;
											ou = ou + ((len+4+kSizeDataBaseObjectHeader+ktaillebloc-1)&(-ktaillebloc));
										}
									}
									FreeFastMem(p);
								}
							}
							else
							{
								if (tag.Match(DBOH_TableDefElem))
								{
									void* p = recover.AddRecInfoFromDisk(olddatafile, ou, tag, recover.tabledefs, errout);
									if (p != nil)
									{
										FichierDISK* fid = (FichierDISK*)p;
										try
										{
											recover.tableIDs[fid->ID] = tag.GetPos()+1;
										}
										catch (...)
										{
											errout = memfull;
										}

										FreeFastMem(p);

										okblock = true;
										ou = ou + ((tag.GetLen()+kSizeDataBaseObjectHeader+ktaillebloc-1)&(-ktaillebloc));
									}
								}
								else
								{
									if (tag.Match(DBOH_DataTable))
									{
										void* p = recover.AddRecInfoFromDisk(olddatafile, ou, tag, recover.datatableheaders, errout);
										if (p != nil)
										{
											DataTableDISK* dtd = (DataTableDISK*)p;
											try
											{
												sLONG numdatatable = tag.GetPos();
												if ((sLONG)recover.datatableinfos.size() < numdatatable)
												{
													recover.datatableinfos.resize(numdatatable);
												}
												recover.datatableinfos[numdatatable-1] = DataTableWithSeqNumMatch(dtd->TableDefID, dtd->SeqNum_ID);	
											}
											catch (...)
											{
												errout = memfull;
											}
											FreeFastMem(p);

											okblock = true;
											ou = ou + ((tag.GetLen()+kSizeDataBaseObjectHeader+ktaillebloc-1)&(-ktaillebloc));
										}
									}
									else if (tag.Match(DBOH_AutoSeqNumberSimple))
									{
										void* p = recover.AddRecInfoFromDisk(olddatafile, ou, tag, recover.seqnumdefs, errout);
										if (p != nil)
										{
											AutoSeqSimpleOnDisk* assod = (AutoSeqSimpleOnDisk*)p;
											recover.seqnums[assod->ID] = *assod;
											FreeFastMem(p);
											okblock = true;
											ou = ou + ((tag.GetLen()+kSizeDataBaseObjectHeader+ktaillebloc-1)&(-ktaillebloc));
										}
									}
								}
							}
						}
					}

					if (!okblock)
						ou = ou + ktaillebloc;
				}

			}

			if (log != nil)
				log->CloseProgressSession();
		}

	}

	if (errout == VE_OK)
	{
		sLONG nbtablewithdata = (sLONG)recover.recs.size();
		recover.tableMapping.resize(nbtablewithdata, 0);
		vector<sLONG>::iterator curdest = recover.tableMapping.begin();
		for (RecInfoMatrix::iterator cur = recover.recs.begin(), end = recover.recs.end(); cur != end; cur++, curdest++)
		{
			if ((*cur).empty())
				*curdest = -1;
		}

		sLONG nbdatatable = (sLONG)recover.datatableheaders.size();

		{
			VError err;
			StErrorContextInstaller errs(false);
			VString s;
			inOldDataFile->GetNameWithoutExtension(s);
			s += kDataMatchExt;
			VFolder* vf = inOldDataFile->RetainParentFolder();
			if (vf != nil)
			{
				VFile matchFile(*vf, s);
				VValueBag bag;
				err = LoadBagFromXML(matchFile, L"DataTableMatching", bag);
				if (err == VE_OK)
				{
					VBagArray* elems = bag.GetElements(L"DataTable");
					if (elems)
					{
						for (VIndex i = 1, nb = elems->GetCount(); i <= nb; i++)
						{
							VValueBag* elem = elems->GetNth(i);
							sLONG num;
							VUUID xid;
							elem->GetLong(L"num", num);
							elem->GetVUUID(L"TableDefID", xid);
							if (num <= nbtablewithdata)
							{
								if (num >= nbdatatable)
								{
									recover.datatableinfos.resize(num+1);
									recover.datatableheaders.resize(num+1, RecInfo());
									nbdatatable = num + 1;
								}
								if (recover.datatableheaders[num-1].fAddr == 0 || recover.datatableheaders[num-1].fLen == 0)
								{
									VUUIDBuffer tabledefid, anyid;
									xid.ToBuffer(tabledefid);
									recover.datatableheaders[num-1].fAddr = -2;
									anyid.FromLong(0);
									VUUID xid;
									VUUID xid2(recover.datatableinfos[num-1].first);
									if (xid == xid2)
										recover.datatableinfos[num-1] = DataTableWithSeqNumMatch(tabledefid, anyid);
								}
							}

						}
					}
				}
				vf->Release();
			}
		}

		for (sLONG i = 1; i <= nbtablewithdata; i++)
		{
			VUUIDBuffer tabledefid;
			if ((i>=nbdatatable) || (recover.datatableheaders[i].fAddr == 0))
			{
				// Nous sommes dans le cas ou le header d'une DataTable est abime ou inexistant
			}
			else
			{
				tabledefid = recover.datatableinfos[i-1].first;
				uLONG8 xstamp = recover.datatableheaders[i].fTimeStamp;
				sLONG nbrec = 0;
				if (i<=recover.recs.size())
					nbrec = (sLONG)recover.recs[i-1].size();

				for (sLONG j = i+1; j <= nbtablewithdata; j++)
				{
					VUUIDBuffer tabledefid2;
					if ((j>=nbdatatable) || (recover.datatableheaders[j].fAddr == 0))
					{
						// Nous sommes dans le cas ou le header d'une DataTable est abime ou inexistant
					}
					else
					{
						tabledefid2 = recover.datatableinfos[j-1].first;
						if (tabledefid2 == tabledefid)
						{
							uLONG8 xstamp2 = recover.datatableheaders[j].fTimeStamp;
							sLONG nbrec2 = 0;
							if (j<=recover.recs.size())
								nbrec2 = (sLONG)recover.recs[j-1].size();

							bool keepfirst = true;
							if (nbrec < nbrec2)
								keepfirst = false;
							else
							{
								if (nbrec2 == nbrec)
								{
									if (xstamp2 > xstamp)
										keepfirst = false;
								}
							}
							
							if (keepfirst)
							{
								recover.datatableinfos[j-1].first.FromLong(0);
								recover.datatableheaders[j].fAddr = 0;
								//recover.recs[j-1].clear();
							}
							else
							{
								recover.datatableinfos[i-1].first.FromLong(0);
								recover.datatableheaders[i].fAddr = 0;
								//recover.recs[i-1].clear();
								j = nbtablewithdata+1;
							}
						}
					}
				}
			}
		}


		for (sLONG i = 1; i <= nbtablewithdata; i++)
		{
			VUUIDBuffer tabledefid;
			VUUIDBuffer seqnumid;
			if ((i>=nbdatatable) || (recover.datatableheaders[i].fAddr == 0))
			{
				tabledefid.FromLong(0);
				seqnumid.FromLong(0);
				// Nous sommes dans le cas ou le header d'une DataTable est abime ou inexistant
			}
			else
			{
				tabledefid = recover.datatableinfos[i-1].first;
				seqnumid = recover.datatableinfos[i-1].second;
			}

			BagElement tableinfo(outBag, DB4DBagKeys::datatable);
			tableinfo->SetLong(DB4DBagKeys::datatable_num, i);
			if (i<=recover.recs.size())
			{
				sLONG reccount = 0;
				RecInfoVector& recs = recover.recs[i-1];

				for (RecInfoVector::iterator cur = recs.begin(), end = recs.end(); cur != end; cur++)
				{
					if (cur->fAddr != 0)
						reccount++;
				}

				tableinfo->SetLong(DB4DBagKeys::records_count, /*recover.recs[i-1].size()*/reccount);
			}
			else
			{
				tableinfo->SetLong(DB4DBagKeys::records_count, 0);
			}

			VUUID tabledefID(tabledefid);
			Table* tt = FindAndRetainTableRef(tabledefID);
			if (tt != nil)
			{
				tableinfo->SetLong(DB4DBagKeys::table_num, tt->GetNum());
				VString ss;
				tt->GetName(ss);
				tableinfo->SetString(DB4DBagKeys::table_name, ss);
				recover.tableMapping[i-1] = tt->GetNum();
				tt->Release();
			}
			else
			{
				if (i <= nbtablewithdata)
				{
					if (recover.recs[i-1].empty())
					{
						recover.tableMapping[i-1] = -1;
					}
					else
					{
						VValueBag* tablebag = new VValueBag();
						tablebag->SetLong(DB4DBagKeys::datatable_num, i);

						RecInfoVector& curdatatable = recover.recs[i-1];
						sLONG nb = (sLONG)curdatatable.size();
						sLONG nbinbag = 0;
						for (sLONG j = 0; j < nb && nbinbag < 20; j++)
						{
							RecInfo& info = curdatatable[j];
							if (info.fAddr != 0)
							{
								FicheOnDisk* ficD = FicheOnDisk::BuildRecordFromLoad(err, i, j, info.fAddr, olddatafile, nil);
								if (ficD != nil)
								{
									nbinbag++;
									BagHolder recBag;
									for (sLONG k = 1, nbfield = ficD->GetNbCrit(); k <= nbfield; k++)
									{
										BagHolder fieldBag;
										sLONG whattype;
										void* data = ficD->GetDataPtr(k, &whattype);
										VValueSingle* cv = nil;
										if (data == nil)
										{
											fieldBag->SetString(DB4DBagKeys::value,L"<null>");
										}
										else
										{
											switch (whattype)
											{
											case VK_TEXT:
											case VK_TEXT_UTF8:
												fieldBag->SetString(DB4DBagKeys::value,L"<Blob TEXT>");
												break;
											case VK_BLOB:
											case VK_BLOB_DB4D:
												fieldBag->SetString(DB4DBagKeys::value,L"<Blob>");
												break;
											case VK_IMAGE:
												fieldBag->SetString(DB4DBagKeys::value,L"<Blob Picture>");
												break;
											default:
												{
													CreVValue_Code Code=FindCV(whattype);
													if (Code != nil)
													{
														cv=(*Code)(nil,0,data, false, true, nil, creVValue_default);
														fieldBag->SetAttribute(DB4DBagKeys::value, cv);
													}
												}
												break;
											}

										}
										recBag->AddElement(DB4DBagKeys::field, fieldBag);
									}
									tablebag->AddElement(DB4DBagKeys::record, recBag);
									ficD->Release();
								}
							}
						}

						outBag.AddElement(DB4DBagKeys::missing_datatable, tablebag);
						tablebag->Release();
					}
				}
			}
		}
 
	}
 

//	context->Release();

	if (err == VE_USER_ABORT)
	{
		errout = err;
	}
	return errout;
}























