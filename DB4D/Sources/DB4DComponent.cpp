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

//#include "kernelIPC/VComponentLibrary.h"
#include "DB4DComponent.h"
#include "javascript_db4d.h"
#include <set>


const void* kNullRawRecord = (void*)-2;

const sLONG				kCOMPONENT_TYPE_COUNT	= 2;
const CImpDescriptor	kCOMPONENT_TYPE_LIST[]	= {
	{ CDB4DManager::Component_Type, VImpCreator<VDB4DMgr>::CreateImp },
	{ CDB4DJournalParser::Component_Type, VImpCreator<VDB4DJournalParser>::CreateImp }
	};

VComponentLibrary* gDB4DCompLib = nil;

void XBOX::xDllMain (void)
{
	gDB4DCompLib = new VComponentLibrary(kCOMPONENT_TYPE_LIST, kCOMPONENT_TYPE_COUNT);
}



//=================================================================================

/*
QueryParamElement::QueryParamElement(XBOX::VValueSingle* inCV, bool ownsIt)
{
	fScalar = inCV;
	fType = DB4D_QPE_scalar;
	fArray = nil;
}
*/

QueryParamElement::QueryParamElement(XBOX::VJSArray& inArray)
{
	fArray = new VJSArray(inArray);
	fScalar = nil;
	fSimpleDateScalar = nil;
	fType = DB4D_QPE_array;
	XBOX::JS4D::ProtectValue(fArray->GetContextRef(), *fArray);
}

/*
QueryParamElement::QueryParamElement()
{
	fScalar = nil;
	fType = DB4D_QPE_none;
	fArray = nil;
}
*/


void QueryParamElement::Duplicate()
{
	if (fType == DB4D_QPE_scalar)
	{
		if (fScalar != nil)
			fScalar = fScalar->Clone();
		if (fSimpleDateScalar != nil)
			fSimpleDateScalar = fSimpleDateScalar->Clone();
	}
	else if (fType == DB4D_QPE_array)
	{
		if (fArray != nil)
		{
			fArray = new VJSArray(*fArray);
			XBOX::JS4D::ProtectValue(fArray->GetContextRef(), *fArray);
		}
	}
}


void QueryParamElement::Dispose()
{
	if (fType == DB4D_QPE_scalar)
	{
		if (fScalar != nil)
			delete fScalar;
		if (fSimpleDateScalar != nil)
			delete fSimpleDateScalar;
	}
	else if (fType == DB4D_QPE_array)
	{
		if (fArray != nil)
		{
			XBOX::JS4D::UnprotectValue(fArray->GetContextRef(), *fArray);
			delete fArray;
		}
	}
}


//=================================================================================


DB4DKeyWordList::~DB4DKeyWordList()
{
	if (mustdispose)
	{
		sLONG i,nb = GetCount();

		for (i=0; i<nb; i++)
		{
			VString* s = (*this)[i];
			if (s != nil)
				delete s;
		}
	}
}



//=================================================================================



VDB4DMgr::VDB4DMgr()
{
	fManager = VDBMgr::RetainManager(gDB4DCompLib);
	assert( fManager != nil);
}


VDB4DMgr::~VDB4DMgr()
{
	::CopyRefCountable( &fManager, (VDBMgr*) nil);
}


DB4DNetworkManager* VDB4DMgr::CreateRemoteTCPSession(const VString& inServerName, sWORD inPortNum, VError& outerr, bool inSSL)
{
	return DB4DNetManager::NewConnection(inServerName, inPortNum, outerr, inSSL);
}

IBackupSettings* VDB4DMgr::CreateBackupSettings()
{
	return VDBMgr::CreateBackupSettings();
}

IBackupTool* VDB4DMgr::CreateBackupTool()
{
	return VDBMgr::CreateBackupTool();
}

IJournalTool* VDB4DMgr::CreateJournalTool()
{
	return VDBMgr::CreateJournalTool();
}

XBOX::VError VDB4DMgr::GetJournalInfos(const XBOX::VFilePath& inDataFilePath,XBOX::VFilePath& outJournalPath,XBOX::VUUID& outJournalDataLink)
{
	return VDBMgr::GetJournalInfo(inDataFilePath,outJournalPath,outJournalDataLink);
}


CDB4DBase* VDB4DMgr::OpenRemoteBase( CDB4DBase* inLocalDB, const VUUID& inBaseID, VErrorDB4D& outErr, DB4DNetworkManager *inLegacyNetworkManager, CUAGDirectory* inUAGDirectory)
{
	CDB4DBase* result = nil;

	Base4D* localdb = nil;
	if (inLocalDB != nil)
	{
		localdb = VImpCreator<VDB4DBase>::GetImpObject(inLocalDB)->GetBase();
	}

	VString serverAddress;
	sLONG db4dServerPortNumber;
	bool useSSL;
	inLegacyNetworkManager->GetServerAddressAndPort( serverAddress, NULL, &db4dServerPortNumber, &useSSL);

	DB4DNetworkManager *networkManager = CreateRemoteTCPSession( serverAddress, db4dServerPortNumber, outErr, useSSL);

	if (networkManager != nil)
	{
		Base4D* db = new Base4D(localdb, inBaseID, networkManager, inLegacyNetworkManager);
		if (db == nil)
			outErr = ThrowBaseError(memfull, DBaction_OpeningRemoteBase);
		else
		{
			db->libere();
			outErr = db->OpenRemote(false);
			if (outErr == VE_OK)
			{
				db->Retain("Registerbase");
				VDBMgr::GetManager()->RegisterBase(db);
				result = new VDB4DBase(VDBMgr::GetManager(), db, true);
			}
			db->Release();
		}
	}

	if (outErr != VE_OK)
		outErr = ThrowBaseError(VE_DB4D_CANNOTOPENSTRUCT, DBaction_OpeningRemoteBase);

	return result;
}


CDB4DBase* VDB4DMgr::OpenRemoteBase( CDB4DBase* inLocalDB, const VString& inBaseName, const VString& inFullBasePath, VErrorDB4D& outErr, Boolean inCreateIfNotExist, DB4DNetworkManager* inNetworkManager, CUAGDirectory* inUAGDirectory)
{
	CDB4DBase* result = nil;
	outErr = VE_OK;

	if (inNetworkManager == nil)
		inNetworkManager = VDBMgr::GetNetworkManager();

	if (inNetworkManager != nil)
	{
		IStreamRequest* req = inNetworkManager->CreateRequest(nil, kRangeReqDB4D + req_OpenOrCreateBase);
		if (req != nil)
		{
			VStream* reqsend = req->GetOutputStream();
			inBaseName.WriteToStream(reqsend);
			inFullBasePath.WriteToStream(reqsend);
			sBYTE b = 0;
			if (inCreateIfNotExist)
				b = 1;
			reqsend->PutByte(b);
			outErr = reqsend->GetLastError();
			if (outErr == VE_OK)
			{
				outErr = req->Send();
				if (outErr == VE_OK)
				{
					VStream* reqget = req->GetInputStream();
					VUUID xid;
					outErr = xid.ReadFromStream(reqget);
					if (outErr == VE_OK)
					{
						result = OpenRemoteBase(inLocalDB, xid, outErr, inNetworkManager);
					}
				}
			}
			req->Release();
		}
		else outErr = ThrowBaseError(memfull, DBaction_OpeningRemoteBase);
	}

	if (outErr != VE_OK)
		outErr = ThrowBaseError(VE_DB4D_CANNOTOPENSTRUCT, DBaction_OpeningRemoteBase);

	return result;
}


CDB4DBase* VDB4DMgr::OpenRemoteBase( CDB4DBase* inLocalDB, const VString& inBaseName, VErrorDB4D& outErr, DB4DNetworkManager* inNetworkManager, CUAGDirectory* inUAGDirectory)
{
	return nil;
}



void* VDB4DMgr::GetComponentLibrary() const
{
	return (void*)gDB4DCompLib;
}


void VDB4DMgr::__test(const VString& command)
{

	if (command == L"empty mem")
	{
		VMemStats stats;
		VDBMgr::GetCacheManager()->GetMemoryManager()->GetStatistics(stats);
		stats.Dump();

		
		void* p =GetFastMem(1000000000,false,'none');
		if (p != nil)
			FreeFastMem(p);
			
		VMemStats stats2;
		VDBMgr::GetCacheManager()->GetMemoryManager()->GetStatistics(stats2);
		stats2.Dump();
	}
	else
	{
		if (command == L"plus rien du tout")
		{
			VError err = VE_OK;
			CDB4DManager* comp = (CDB4DManager*) VComponentManager::RetainComponent((CType)CDB4DManager::Component_Type);
			if (comp != nil)
			{
				//DB4DNetworkManager* netacces = comp->CreateRemoteTCPSession(L"192.168.92.224", 0, err, false);
				DB4DNetworkManager* netacces = comp->CreateRemoteTCPSession(L"127.0.0.1", 0, err, false);
				if (netacces != nil)
				{
					CDB4DBase* base = comp->OpenRemoteBase(nil, L"test", L"c:\\", err, true, netacces);
					if (base != nil)
					{
						CDB4DBaseContext* context = base->NewContext(nil, kJSContextCreator);

						CDB4DTable* tt = base->FindAndRetainTable(L"t1", context);
						if (tt != nil)
						{
							Base4D* bd = VImpCreator<VDB4DBase>::GetImpObject(base)->GetBase();

							IRequest* req = bd->CreateRequest(context, Req_TestServer + kRangeReqDB4D);
							if (req == nil)
								err = ThrowBaseError(memfull, noaction);
							else
							{
								req->PutBaseParam(bd);

								err = req->GetLastError();

								if (err == VE_OK)
								{
									err = req->Send();
									if (err == VE_OK)
									{
										VStream* fromserver = req->GetInputStream();
										CDB4DSelection* sel = base->BuildSelectionFromServer(fromserver, context, tt, err);
										if (sel != nil)
										{
											DebugMsg(L"All records : "+ToString(sel->CountRecordsInSelection(context))+L" records\n");
											CDB4DRecord* rec = base->BuildRecordFromServer(fromserver, context, tt, err);
											if (rec != nil)
											{
												sLONG nb = tt->CountFields();
												for (sLONG i = 1; i <= nb; i++)
												{
													VString s;
													if (i != 1)
														DebugMsg(L" , ");
													rec->GetString(i, s);
													DebugMsg(s);
												}
												DebugMsg(L"\n");
												rec->Release();
											}
											sel->Release();
										}
									}
								}
								req->Release();
							}

							tt->Release();
						}

						context->Release();
						base->CloseAndRelease();
					}
					netacces->Dispose();
				}

				comp->Release();
			}
		}
		else
		{
			if (command == L"test server")
			{
				Base4D* bd = VDBMgr::GetManager()->RetainBase4DByName(L"InternalDatabase");
				if (bd != nil)
				{
					CDB4DBase* xdb = bd->RetainBaseX();
					CDB4DBaseContext*  context = xdb->NewContext(nil, kJSContextCreator);
					IRequest* req = bd->CreateRequest(context, Req_TestServer + kRangeReqDB4D);
					if (req != nil)
					{
						req->PutBaseParam(bd);
						req->Send();

						req->Release();
					}
					context->Release();
					xdb->Release();

					bd->Release();
				}
			}
			else if (command == "test ReadWriteSemaphore")
			{
				Test_ReadWriteSemaphore( 100);
			}
			else
			{
				Base4D* bd = VDBMgr::GetManager()->RetainBase4DByName(command);
				if (bd != nil)
				{
					CDB4DBase* xdb = bd->RetainBaseX();
					CDB4DBaseContext*  context = xdb->NewContext(nil, kJSContextCreator);
					IRequest* req = bd->CreateRequest(context, Req_TestServer + kRangeReqDB4D);
					if (req != nil)
					{
						req->PutBaseParam(bd);
						req->Send();

						req->Release();
					}
					context->Release();
					xdb->Release();
					
					bd->Release();
				}
			}
		}
	}

#if 0

	CDB4DManager* db4dman = (CDB4DManager*) VComponentManager::RetainComponent( (CType) CDB4DManager::Component_Type );
	VError err = VE_OK;

	VFile path(L"C:\\testdbFP.4db");
	if (path.Exists())
	{
		CDB4DBase* db = db4dman->OpenBase(path, true);
		if (db != nil)
		{
			if (true)
			{
				CDB4DTable* table = db->RetainNthTable(1);
				if (table != nil)
				{
					CDB4DBaseContextPtr context = db->NewContext();
					CDB4DQuery* query = table->NewQuery();
					query->AddCriteria(1, 1, DB4D_Greater, (VString)L"7");
					
					CDB4DImpExp* io = table->NewImpExp();
					io->AddCol(1);
					io->AddCol(2);
					io->AddCol(3);

					CDB4DSelection* sel = table->ExecuteQuery(query, context, nil, nil);
					if (sel != nil)
					{
						sel->SortSelection(1, true, nil, context);

						io->SetPath(L"c:\\testIO2.txt");
						err = io->RunExport(sel, nil, context);

						sel->Release();
					}

					io->Release();

					context->Release();
					table->Release();
				}
			}

			db->Close();
			db->Release();
		}
	}
	else
	{
		path.Delete();
		VFile path2(L"C:\\testdbFP.data");
		path2.Delete();

		CDB4DBase* db = db4dman->CreateBase(path, true);
		if (db != nil)
		{
			VValueBag def;
			def.SetString( L"name", L"Emps");

			VBagArray *fields = new VBagArray;
			VValueBag *fieldBag = new VValueBag;

			fieldBag = new VValueBag;
			fieldBag->SetString( L"name", L"Name");
			fieldBag->SetLong( L"type", DB4D_StrFix);
			fields->AddTail( fieldBag);
			fieldBag->Release();

			fieldBag = new VValueBag;
			fieldBag->SetString( L"name", L"FirstName");
			fieldBag->SetLong( L"type", DB4D_StrFix);
			fields->AddTail( fieldBag);
			fieldBag->Release();

			fieldBag = new VValueBag;
			fieldBag->SetString( L"name", L"Salary");
			fieldBag->SetLong( L"type", DB4D_Real);
			fields->AddTail( fieldBag);
			fieldBag->Release();

			fieldBag = new VValueBag;
			fieldBag->SetString( L"name", L"DeptNo");
			fieldBag->SetLong( L"type", DB4D_Integer32);
			fields->AddTail( fieldBag);
			fieldBag->Release();

			def.SetElements( L"field", fields);
			fields->Release();

			CDB4DTable* table = nil;
			err = db->CreateTable(def, &table);

			if (err == VE_OK)
			{
				CDB4DRecord* rec = nil;
				sLONG i;

				CDB4DBaseContextPtr context = db->NewContext();

				for (i = 0; i<200; i++)
				{
					rec = table->NewRecord(context);
					rec->SetLong(1, VSystem::Random(1,1000000000));
					rec->SetLong(2, i);
					rec->SetReal(3, (Real)i * (Real)i * (Real)i);
					rec->Save();
					rec->Release();
				}

				CDB4DImpExp* io = table->NewImpExp();
				io->AddCol(1);
				io->AddCol(2);
				io->AddCol(3);

				CDB4DSelection* sel = table->SelectAllRecords(context);
				if (sel != nil)
				{
					sel->SortSelection(1, true, nil, context);

					io->SetPath(L"c:\\testIO.txt");
					err = io->RunExport(sel, nil, context);

					sel->Release();
				}

				/*
				context->StartTransaction();

				for (i = 400; i<600; i++)
				{
					rec = table->NewRecord(context);
					rec->SetLong(1, VSystem::Random(1,1000000000));
					rec->SetLong(2, i);
					rec->SetReal(3, (Real)i * (Real)i * (Real)i);
					rec->Save();
					rec->Release();
				}

				context->RollBackTransaction();

				context->StartTransaction();

				for (i = 800; i<1000; i++)
				{
					rec = table->NewRecord(context);
					rec->SetLong(1, VSystem::Random(1,1000000000));
					rec->SetLong(2, i);
					rec->SetReal(3, (Real)i * (Real)i * (Real)i);
					rec->Save();
					rec->Release();
				}

				context->CommitTransaction();


				sel = table->SelectAllRecords(context);
				if (sel != nil)
				{
					sel->SortSelection(1, true, nil, context);

					io->SetPath(L"c:\\testIO3.txt");
					err = io->RunExport(sel, nil, context);

					sel->Release();
				}
				*/

				io->Release();

				context->Release();
			}


			db4dman->FlushCache(true);

			db->Close();

			db->Release();
		}
	}


	db4dman->Release();

#endif

}


CDB4DBase* VDB4DMgr::OpenBase( const VString& inXMLContent, sLONG inParameters, VError* outErr, const VFilePath& inXMLPath, unordered_map_VString<VRefPtr<VFile> >* outIncludedFiles, const VFile* inPermissionsFile)
{
	CDB4DBase* id = nil;
	if (testAssert( fManager != nil)) {
		VFile structureFile( inXMLPath );
		id = fManager->OpenBase( structureFile, inParameters, outErr, XBOX::FA_READ, nil, nil, &inXMLContent, outIncludedFiles, inPermissionsFile);
	}
	return id;
}


CDB4DBase* VDB4DMgr::OpenBase( const VFile& inStructureFile, sLONG inParameters, VError* outErr, FileAccess inAccess, VString* EntityFileExt, CUAGDirectory* inUAGDirectory, const VFile* inPermissionsFile)
{
	CDB4DBase* id = nil;
	if (testAssert( fManager != nil)) {
		id = fManager->OpenBase( inStructureFile, inParameters, outErr, inAccess, EntityFileExt, inUAGDirectory, nil, nil, inPermissionsFile);
	}
	return id;
}


CDB4DBase* VDB4DMgr::CreateBase( const VFile& inStructureFile, sLONG inParameters, VIntlMgr* inIntlMgr, VError* outErr, FileAccess inAccess, const VUUID* inChosenID, VString* EntityFileExt, CUAGDirectory* inUAGDirectory, const VFile* inPermissionsFile)
{
	CDB4DBase* id = nil;
	if (testAssert( fManager != nil)) {
		id = fManager->CreateBase( inStructureFile, inParameters, inIntlMgr, outErr, inAccess, inChosenID, EntityFileExt, inUAGDirectory, inPermissionsFile);
	}
	return id;
}


sLONG VDB4DMgr::CountBases()
{
	sLONG count = 0;
	if (fManager != nil) {
		count = fManager->CountBases();
	}
	return count;
}


CDB4DBase* VDB4DMgr::RetainNthBase( sLONG inBaseIndex)
{
	CDB4DBase* id = nil;
	if (testAssert( fManager != nil)) {
		id = fManager->RetainNthBase( inBaseIndex);
	}
	return id;
}


CDB4DBase* VDB4DMgr::RetainBaseByUUID(const VUUID& inBaseID)
{
	CDB4DBase* id = nil;
	if (testAssert( fManager != nil)) {
		id = fManager->RetainBaseByUUID( inBaseID);
	}
	return id;
}


void VDB4DMgr::FlushCache(Boolean WaitUntilDone, bool inEmptyCacheMem, ECacheLogEntryKind inOrigin)
{
	VCacheLogEntryFlushFromAction		logFlush( inOrigin, WaitUntilDone, inEmptyCacheMem, true);

	if (testAssert( fManager != nil))
	{
		if (inEmptyCacheMem)
		{
			fManager->GetCacheManager()->GetMemoryManager()->PurgeMem();
		}
		else
		{
			fManager->FlushCache( WaitUntilDone);
		}
	}
}


void VDB4DMgr::SetFlushPeriod( sLONG inMillisecondsPeriod)
{
	if (testAssert( fManager != nil))
	{
		fManager->SetFlushPeriod( inMillisecondsPeriod);
	}
}


VSize VDB4DMgr::GetMinimumCacheSize() const
{
	return VDBCacheMgr::kMinimumCache;
}

VErrorDB4D VDB4DMgr::SetCacheSize(VSize inSize, bool inPhysicalMemOnly, VSize *outActualSize)
{
	if (testAssert( fManager != nil)) {
		return fManager->SetCacheSize( inSize, inPhysicalMemOnly, outActualSize);
	}
	else
		return ThrowBaseError(memfull, DBaction_ChangeCacheSize);
	
}

VErrorDB4D VDB4DMgr::SetMinSizeToFlush( VSize inMinSizeToFlush)
{
	if (testAssert( fManager != nil)) {
		return fManager->SetMinSizeToFlush( inMinSizeToFlush);
	}
	else
		return ThrowBaseError(VE_UNKNOWN_ERROR, DBaction_ChangeCacheSize);
}


VSize VDB4DMgr::GetCacheSize()
{
	if (testAssert( fManager != nil)) {
		return fManager->GetCacheSize();
	}

	return 0;
}

VSize VDB4DMgr::GetMinSizeToFlush()
{
	if (testAssert( fManager != nil)) {
		return fManager->GetMinSizeToFlush();
	}

	return 0;
}

VErrorDB4D VDB4DMgr::SetTempMemSize(VSize inSize, VSize *outActualSize)
{
#if oldtempmem
	if (testAssert( fManager != nil)) 
	{
		return fManager->SetTempMemSize(inSize, outActualSize);
	}
	else
	{
		*outActualSize = 0;
		return ThrowBaseError(memfull, DBaction_ChangeCacheSize);
	}
#else
	return ThrowBaseError(VE_DB4D_NOTIMPLEMENTED);
#endif
}


VSize VDB4DMgr::GetTempMemSize()
{
#if oldtempmem
	if (testAssert( fManager != nil))
	{
		return fManager->GetTempMemSize();
	}
	else
		return 0;
#else
	return 0;
#endif
}


VCppMemMgr* VDB4DMgr::GetTempMemManager()
{
#if oldtempmem
	if (testAssert( fManager != nil))
	{
		return fManager->GetTempMemMgr();
	}
	else
		return nil;
#else
	return VObject::GetMainMemMgr();
#endif
}

/*
VCommandList* VDB4DMgr::GetCommandList()
{
	return fManager->GetCommandList();
}
*/

DB4DCommandManager* VDB4DMgr::SetCommandManager(DB4DCommandManager* inNewCommandManager)
{
	return fManager->SetCommandManager(inNewCommandManager);
}


DB4DSignaler *VDB4DMgr::GetSignaler()
{
	return fManager->GetSignaler();
}


DB4DActivityManager* VDB4DMgr::SetActivityManager(DB4DActivityManager* inNewActivityManager)
{
	return fManager->SetActivityManager(inNewActivityManager);
}


DB4DNetworkManager* VDB4DMgr::SetNetworkManager( DB4DNetworkManager* inNewNetworkManager)
{
	return fManager->SetNetworkManager( inNewNetworkManager);
}


BuildLanguageExpressionMethod VDB4DMgr::SetMethodToBuildLanguageExpressions(BuildLanguageExpressionMethod inNewMethodToBuildLanguageExpressions)
{
	return fManager->SetMethodToBuildLanguageExpressions(inNewMethodToBuildLanguageExpressions);
}

 
VCppMemMgr *VDB4DMgr::GetCacheMemoryManager()
{
	return VDBMgr::GetCacheManager()->GetMemoryManager();
}


CDB4DContext* VDB4DMgr::NewContext(CUAGSession* inUserSession, VJSGlobalContext* inJSContext, bool isLocalOnly)
{
	return fManager->NewContext(inUserSession, inJSContext, isLocalOnly);
}


CDB4DContext* VDB4DMgr::RetainOrCreateContext(const VUUID& inID, CUAGSession* inUserSession, VJSGlobalContext* inJSContext, bool isLocalOnly)
{
	return fManager->RetainOrCreateContext(inID, inUserSession, inJSContext, isLocalOnly);
}


void VDB4DMgr::StartDebugLog()
{
	ACCEPTJOURNAL;
}


uLONG8 VDB4DMgr::GetDB4D_VersionNumber()
{
	return kVersionDB4D;
}

/*
UniChar VDB4DMgr::GetWildChar()
{
	return VDBMgr::GetWildChar();
}


void VDB4DMgr::SetWildChar(UniChar c)
{
	VDBMgr::SetWildChar(c);
}
*/


bool VDB4DMgr::ContainsKeyword( const VString& inString, const VString& inKeyword, bool inDiacritical, VIntlMgr *inIntlMgr, VErrorDB4D& outErr)
{
	VCompareOptions options;
	options.SetDiacritical(inDiacritical);
	options.SetIntlManager(inIntlMgr);
	return VDBMgr::ContainsKeyword( inString, inKeyword, options, outErr);
}


bool VDB4DMgr::ContainsKeyword_Like( const VString& inString, const VString& inKeyword, bool inDiacritical, VIntlMgr *inIntlMgr, VErrorDB4D& outErr)
{
	VCompareOptions options;
	options.SetDiacritical(inDiacritical);
	options.SetIntlManager(inIntlMgr);
	options.SetLike(true);
	return VDBMgr::ContainsKeyword( inString, inKeyword, options, outErr);
}


bool VDB4DMgr::ContainsKeyword( const VString& inString, const VString& inKeyword, const VCompareOptions& inOptions, VErrorDB4D& outErr)
{
	return VDBMgr::ContainsKeyword( inString, inKeyword, inOptions, outErr);
}


VError VDB4DMgr::Register_DB4DArrayOfValuesCreator(sLONG signature, DB4DArrayOfValuesCreator Creator)
{
	return fManager->Register_DB4DArrayOfValuesCreator(signature, Creator);;
}


VError VDB4DMgr::UnRegister_DB4DArrayOfValuesCreator(sLONG signature)
{
	return fManager->UnRegister_DB4DArrayOfValuesCreator(signature);
}


VError VDB4DMgr::Register_DB4DCollectionManagerCreator(sLONG signature, DB4DCollectionManagerCreator Creator)
{
	return fManager->Register_DB4DCollectionManagerCreator(signature, Creator);;
}


VError VDB4DMgr::UnRegister_DB4DCollectionManagerCreator(sLONG signature)
{
	return fManager->UnRegister_DB4DCollectionManagerCreator(signature);
}


void VDB4DMgr::SetDefaultProgressIndicator_For_Indexes(VDB4DProgressIndicator* inProgress)
{
	fManager->SetDefaultProgressIndicator_For_Indexes(inProgress);
}


VDB4DProgressIndicator* VDB4DMgr::RetainDefaultProgressIndicator_For_Indexes()
{
	return fManager->RetainDefaultProgressIndicator_For_Indexes();
}


void VDB4DMgr::SetDefaultProgressIndicator_For_DataCacheFlushing(VDB4DProgressIndicator* inProgress)
{
	fManager->SetDefaultProgressIndicator_For_DataCacheFlushing(inProgress);
}


VDB4DProgressIndicator* VDB4DMgr::RetainDefaultProgressIndicator_For_DataCacheFlushing()
{
	return fManager->RetainDefaultProgressIndicator_For_DataCacheFlushing();
}


bool VDB4DMgr::CheckCacheMem()
{
	return CheckFastMem();
}


const VValueBag* VDB4DMgr::RetainExtraProperties(VFile* inFile, bool *outWasOpened, VUUID *outUUID, VError &err, CDB4DBaseContextPtr inContext, uLONG8* outVersionNumber)
{
	return fManager->RetainExtraProperties(inFile, outWasOpened, outUUID, err, inContext, outVersionNumber);
}


CDB4DRawDataBase* VDB4DMgr::OpenRawDataBaseWithEm(CDB4DBase* inStructureDB, XBOX::VFile* inDataFile, IDB4D_DataToolsIntf* inDataToolLog, VErrorDB4D& outError, FileAccess access)
{
	StErrorContextInstaller errs(false);
	ToolLog mylog(inDataToolLog);
	CDB4DRawDataBase* result = nil;

	outError = VE_OK;
	Base4D_NotOpened* xstruct = new Base4D_NotOpened();
	Base4D_NotOpened* xdata = new Base4D_NotOpened();
	mylog.SetCurrentBase(xdata);
	outError = xdata->Open(inDataFile, &mylog, access, xstruct);
	if (outError == VE_OK)
	{
		if (xstruct != nil)
			xdata->SetStruct(xstruct);
		Base4D* structdb = VImpCreator<VDB4DBase>::GetImpObject(inStructureDB)->GetBase();
		for (sLONG i = 1, nb = inStructureDB->CountTables(nil); i <= nb; i++)
		{
			Table* tt = structdb->RetainTable(i);
			if (tt != nil)
			{
				VString tname;
				VUUID xid;
				tt->GetName(tname);
				tt->GetUUID(xid);
				xdata->AddTableName(i, tname, &mylog);
				xdata->AddUUID(xid.GetBuffer(), tuuid_Table, tname, i, 0, &mylog);
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
						xdata->AddFieldName(i, j, crit->GetTyp(), fname, &mylog);
						xdata->AddUUID(fid.GetBuffer(), tuuid_Field, fname, i, j, &mylog);
						crit->Release();
					}
				}
				tt->Release();
			}
		}

		result = new VDB4DRawDataBase(xdata);
	}

	return result;
}


CDB4DRawDataBase* VDB4DMgr::OpenRawDataBase(XBOX::VFile* inStructFile, XBOX::VFile* inDataFile, IDB4D_DataToolsIntf* inDataToolLog, VErrorDB4D& outError, FileAccess access)
{
	StErrorContextInstaller errs(false);
	ToolLog mylog(inDataToolLog);

	outError = VE_OK;
	Base4D_NotOpened* xstruct = new Base4D_NotOpened();
	Base4D_NotOpened* xdata = new Base4D_NotOpened();
	mylog.SetCurrentBase(xstruct);
	if (inStructFile != nil)
		outError = xstruct->Open(inStructFile, &mylog, access, nil);

	if (outError == VE_OK)
	{
		mylog.SetCurrentBase(xdata);
		if (inDataFile != nil)
			outError = xdata->Open(inDataFile, &mylog, access, xstruct);
		else
		{
			if (xstruct != nil)
				xdata->SetStruct(xstruct);
		}
	}
	
	if (outError == VE_OK)
	{
		return new VDB4DRawDataBase(xdata);
	}
	else
	{
		delete xstruct;
		delete xdata;
		return nil;
	}
}


VErrorDB4D VDB4DMgr::ReleaseRawRecord(void* inRawRecord)
{
	if (inRawRecord == kNullRawRecord)
	{
		return VE_OK;
	}
	else
	{
		if (testAssert(inRawRecord != nil))
		{
			((FicheOnDisk*)inRawRecord)->Release();
			/*
			((FicheOnDisk*)inRawRecord)->occupe();
			((FicheOnDisk*)inRawRecord)->FreeAfterUse();
			*/
			return VE_OK;
		}
		else
			return VE_DB4D_RECORD_IS_NULL;
	}
}


RecIDType VDB4DMgr::GetRawRecordID(void* inRawRecord, VErrorDB4D* outError)
{
	if (outError != nil)
		*outError = VE_OK;
	if (inRawRecord == kNullRawRecord)
	{
		return -1;
	}
	else
	{
		if (testAssert(inRawRecord != nil))
			return 	((FicheOnDisk*)inRawRecord)->getnumfic();
		else
		{
			if (outError != nil)
				*outError = VE_DB4D_RECORD_IS_NULL;
			return -1;
		}
	}

}


void* VDB4DMgr::GetRawRecordNthField(void* inRawRecord, sLONG inFieldIndex, sLONG& outType, VErrorDB4D* outError, bool forQueryOrSort)
{
	if (outError != nil)
		*outError = VE_OK;
	if (inRawRecord == kNullRawRecord)
	{
		outType = -1;
		return nil;
	}
	else
	{
		if (testAssert(inRawRecord != nil))
		{
			tPtr result;
			if (forQueryOrSort)
				result = ((FicheOnDisk*)inRawRecord)->GetDataPtrForQuery(inFieldIndex, &outType);
			else
				result = ((FicheOnDisk*)inRawRecord)->GetDataPtr(inFieldIndex, &outType);

			return (void*)result;
		}
		else
		{
			if (outError != nil)
				*outError = VE_DB4D_RECORD_IS_NULL;
			return nil;
		}
	}
}


void VDB4DMgr::Register_GetCurrentSelectionFromContextMethod(GetCurrentSelectionFromContextMethod inMethod)
{
	fManager->Register_GetCurrentSelectionFromContextMethod(inMethod);
}


void VDB4DMgr::Register_GetCurrentRecordFromContextMethod(GetCurrentRecordFromContextMethod inMethod)
{
	fManager->Register_GetCurrentRecordFromContextMethod(inMethod);
}


void VDB4DMgr::ExecuteRequest( sWORD inRequestID, IStreamRequestReply *inRequest, CDB4DBaseContext *inContext)
{
	fManager->ExecuteRequest(inRequestID, (IRequestReply*)inRequest, inContext);
}


VError VDB4DMgr::StartServer(const VString& inServerName, sWORD inPortNum, bool inSSL)
{
	return fManager->StartServer(inServerName, inPortNum, inSSL);
}


VError VDB4DMgr::StopServer(sLONG inTimeOut)
{
	return fManager->StopServer(inTimeOut);
}

void VDB4DMgr::CloseAllClientConnections(sLONG inTimeOut)
{
	fManager->CloseAllClientConnections(inTimeOut); // pour les connections sur le server
	DB4DNetManager::CloseAllConnections(); // pour les connections sur le client

}

void VDB4DMgr::CloseConnectionWithClient(CDB4DContext* inContext)
{
	fManager->CloseClientConnection(inContext); // pour les connections sur le server

	VImpCreator<VDB4DContext>::GetImpObject(inContext)->CloseAllConnections(); // pour les connections sur le client
	
	/*
	if (inContext != nil)
	{
		VArrayRetainedOwnedPtrOf<CDB4DBaseContextPtr>* contexts = VImpCreator<VDB4DContext>::GetImpObject(inContext)->GetContexts();
		for (VArrayRetainedOwnedPtrOf<CDB4DBaseContextPtr>::Iterator cur = contexts->First(), end = contexts->End(); cur != end; cur++)
		{
			DB4DNetManager::CloseConnection(*cur); // pour les connections sur le client
		}
	}
	*/
}


CDB4DJournalParser* VDB4DMgr::NewJournalParser()
{
	return new VDB4DJournalParser;
}


VErrorDB4D VDB4DMgr::BuildValid4DTableName( const VString& inName, VString& outValidName) const
{
	return ::BuildValid4DTableName( inName, outValidName);
}


VErrorDB4D VDB4DMgr::BuildValid4DFieldName( const VString& inName, VString& outValidName) const
{
	return ::BuildValid4DFieldName( inName, outValidName);
}


VErrorDB4D VDB4DMgr::IsValidTableName( const VString& inName, EValidateNameOptions inOptions, CDB4DBase *inBase, CDB4DTable *inTable) const
{
	VError error = VE_OK;

	if ( (inName.GetLength() > kMaxTableNameLength) || (!::IsValid4DTableName( inName)) )
	{
		error = VE_DB4D_INVALIDTABLENAME;
	}
	else if (inBase != NULL)
	{
		Base4D *base = VImpCreator<VDB4DBase>::GetImpObject(inBase)->GetBase();
		if (testAssert(base != NULL))
		{
			Table *table = (inTable != NULL) ? VImpCreator<VDB4DTable>::GetImpObject(inTable)->GetTable() : NULL;
			Table *other = base->FindAndRetainTableRef( inName);

			if ( (other != nil) && (other != table) )
				error = VE_DB4D_TABLENAMEDUPLICATE;

			ReleaseRefCountable( &other);
		}
	}
	return error;
}


VErrorDB4D VDB4DMgr::IsValidLinkName( const VString& inName, EValidateNameOptions inOptions, bool inIsNameNto1, CDB4DBase *inBase, CDB4DRelation *inRelation) const
{
	VError error = VE_OK;

	if ( !::IsValid4DName( inName) )
	{
		error = VE_DB4D_INVALIDRELATIONNAME;
	}
	else if (inBase != NULL)
	{
		Base4D *base = VImpCreator<VDB4DBase>::GetImpObject(inBase)->GetBase();
		if (testAssert(base != NULL))
		{
			Relation *relation = (inRelation != NULL) ? VImpCreator<VDB4DRelation>::GetImpObject(inRelation)->GetRel() : NULL;
			Relation *other = base->FindAndRetainRelationByName( inName);

			if (other != nil)
			{
				VString oppositeName = (inIsNameNto1) ? other->GetOppositeName() : other->GetName();
				if ( (other != relation) || (inName == oppositeName) )
					error = VE_DB4D_RELATIONNAMEDUPLICATE;
			}

			ReleaseRefCountable( &other);
		}
	}
	return error;
}


VErrorDB4D VDB4DMgr::IsValidFieldName( const VString& inName, EValidateNameOptions inOptions, CDB4DTable *inTable, CDB4DField *inField) const
{
	VError error = VE_OK;

	if ( (inName.GetLength() > kMaxFieldNameLength) || (!::IsValid4DFieldName( inName)) )
	{
		error = VE_DB4D_INVALIDFIELDNAME;
	}
	else if (inTable != NULL)
	{
		Table *table = VImpCreator<VDB4DTable>::GetImpObject(inTable)->GetTable();
		if (testAssert(table != NULL))
		{
			Field *field = (inField != NULL) ? VImpCreator<VDB4DField>::GetImpObject(inField)->GetField() : NULL;
			Field *other = table->FindAndRetainFieldRef( inName);

			if ( (other != nil) && (other != field) )
				error = VE_DB4D_FIELDNAMEDUPLICATE;

			ReleaseRefCountable( &other);
		}
	}
	return error;
}


void VDB4DMgr::StealListOfReleasedSelIDs(vector<VUUIDBuffer>& outList) // to be called on client
{
	fManager->StealListOfReleasedSelIDs(outList);
}


void VDB4DMgr::ForgetServerKeptSelections(const vector<VUUIDBuffer>& inList) // to be called on server
{
	fManager->ForgetServerKeptSelections(inList);
}


void VDB4DMgr::StealListOfReleasedRecIDs(vector<DB4D_RecordRef>& outList) // to be called on client
{
	fManager->StealListOfReleasedRecIDs(outList, nil);
}


void VDB4DMgr::ForgetServerKeptRecords(const vector<DB4D_RecordRef>& inList) // to be called on server
{
	fManager->ForgetServerKeptRecords(inList);
}


void VDB4DMgr::SetRequestLogger(IRequestLogger* inLogger)
{
	fManager->SetRequestLogger(inLogger);
}

bool VDB4DMgr::StartCacheLog(const XBOX::VFolder& inRoot, const XBOX::VString& inFileName,  const XBOX::VValueBag *inParams)
{
	return fManager->StartCacheLog(inRoot, inFileName, inParams);
}

bool VDB4DMgr::IsCacheLogStarted() const
{
	return fManager->IsCacheLogStarted();
}

void VDB4DMgr::StopCacheLog()
{
	fManager->StopCacheLog();
}

void VDB4DMgr::AppendCommentToCacheLog(const XBOX::VString& inWhat)
{
	fManager->AppendCommentToCacheLog(inWhat);
}


void VDB4DMgr::SetRunningMode( DB4D_Running_Mode inRunningMode)
{
	fManager->SetRunningMode( inRunningMode);
}

void VDB4DMgr::SetApplicationInterface( IDB4D_ApplicationIntf *inApplication)
{
	fManager->SetApplicationInterface( inApplication);
}


IDB4D_ApplicationIntf *VDB4DMgr::GetApplicationInterface() const
{
	return fManager->GetApplicationInterface();
}


void VDB4DMgr::SetGraphicsInterface( IDB4D_GraphicsIntf *inGraphics)
{
	fManager->SetGraphicsInterface( inGraphics);
}

IDB4D_GraphicsIntf *VDB4DMgr::GetGraphicsInterface() const
{
	return fManager->GetGraphicsInterface();
}


void VDB4DMgr::GetStaticRequiredJSFiles(std::vector<XBOX::VFilePath>& outFiles)
{
	fManager->GetStaticRequiredJSFiles(outFiles);
}


CDB4DContext* VDB4DMgr::GetDB4DContextFromJSContext( const XBOX::VJSContext& inContext)
{
	return ::GetDB4DContextFromJSContext( inContext);
}

IDB4D_DataToolsIntf* VDB4DMgr::CreateJSDataToolsIntf(VJSContext& jscontext, VJSObject& paramObj)
{
	return VJSDatabase::CreateJSDataToolsIntf(jscontext, paramObj);
}


CDB4DBaseContext* VDB4DMgr::GetDB4DBaseContextFromJSContext( const VJSContext& inContext, CDB4DBase *inBase)
{
	return ::GetDB4DBaseContextFromJSContext( inContext, inBase);
}


void VDB4DMgr::InitializeJSGlobalObject( VJSGlobalContext* inContext, CDB4DBaseContext *inBaseContext)
{
	VJSContext jsContext( inContext);

	if (testAssert(::GetDB4DContextFromJSContext( jsContext) == NULL))
	{
		inBaseContext->GetContextOwner()->SetJSContext( inContext);
		::SetDB4DContextInJSContext( jsContext, inBaseContext->GetContextOwner());

		CDB4DBase* xbase = inBaseContext->GetOwner();
		Base4D* base = VImpCreator<VDB4DBase>::GetImpObject(xbase)->GetBase();
		XBOX::VJSObject globalObject( jsContext.GetGlobalObject());
		XBOX::VJSObject database( VJSDatabase::CreateInstance( jsContext, base));
		globalObject.SetProperty( CVSTR( "db"), database, XBOX::JS4D::PropertyAttributeReadOnly | XBOX::JS4D::PropertyAttributeDontDelete, NULL);
		globalObject.SetProperty( CVSTR( "ds"), database, XBOX::JS4D::PropertyAttributeReadOnly | XBOX::JS4D::PropertyAttributeDontDelete, NULL);
		globalObject.SetProperty("BackupSettings", VJSBackupSettings::CreateInstance(jsContext,NULL), JS4D::PropertyAttributeDontDelete | JS4D::PropertyAttributeReadOnly); 

		BaseTaskInfo* context = ConvertContext(inBaseContext);
		VJSDatabase::PutAllModelsInGlobalObject(globalObject, context->GetBase(), context);
	}

}


void VDB4DMgr::PutAllEmsInGlobalObject(VJSObject& globalObject, CDB4DBaseContext *inBaseContext)
{
	BaseTaskInfo* context = ConvertContext(inBaseContext);
	VJSDatabase::PutAllModelsInGlobalObject(globalObject, context->GetBase(), context);
}


void VDB4DMgr::InitializeJSContext( VJSGlobalContext *inContext, CDB4DContext *inDB4DContext)
{
	VJSContext jsContext( inContext);

	if (testAssert(::GetDB4DContextFromJSContext( jsContext) == NULL))
	{
		xbox_assert(inDB4DContext->GetJSContext() == inContext);
		::SetDB4DContextInJSContext( jsContext, inDB4DContext);
	}
}


void VDB4DMgr::UninitializeJSContext( XBOX::VJSGlobalContext *inContext)
{
	VJSContext jsContext( inContext);

	CDB4DContext *db4dContext = ::GetDB4DContextFromJSContext( jsContext);
	if (testAssert(db4dContext != NULL))
	{
		xbox_assert(db4dContext->GetJSContext() == inContext);
		db4dContext->FreeAllJSFuncs();
		db4dContext->SetJSContext( NULL);
		::SetDB4DContextInJSContext( jsContext, NULL);
	}
}


#if 0
void VDB4DMgr::UninitializeJSContext( XBOX::VJSGlobalContext *inContext, CDB4DBaseContext *inBaseContext)
{
	VJSContext jsContext( inContext);

	CDB4DContext *db4dContext = ::GetDB4DContextFromJSContext( jsContext);
	if (db4dContext != NULL)
	{
		if (testAssert(inBaseContext->GetContextOwner() == db4dContext))
		{
			BaseTaskInfo *taskInfo = ConvertContext( inBaseContext);
			if (taskInfo != NULL)
				taskInfo->FreeAllJSMethods();
		}
	}
}
#endif



VJSObject VDB4DMgr::CreateJSDatabaseObject( const VJSContext& inContext, CDB4DBaseContext *inBaseContext)
{
	return VJSDatabase::CreateInstance( inContext,ConvertContext(inBaseContext)->GetBase());
}


VJSObject VDB4DMgr::CreateJSEMObject( const VString& emName, const VJSContext& inContext, CDB4DBaseContext *inBaseContext)
{
	return VJSDatabase::CreateJSEMObject( emName, inContext, ConvertContext(inBaseContext));
}

VJSObject VDB4DMgr::CreateJSBackupSettings(const VJSContext& inContext,IBackupSettings* retainedBackupSettings)
{
	return VJSBackupSettings::CreateInstance(inContext,retainedBackupSettings);
}


void VDB4DMgr::SetLimitPerSort(VSize inLimit)
{
	fManager->SetLimitPerSort(inLimit);
}

VSize VDB4DMgr::GetLimitPerSort() const
{
	return fManager->GetLimitPerSort();
}


typedef std::vector<XBOX::VRefPtr<VFile> > fileVector;

void buildFullDSDataRef(VFile* dataDS, fileVector& outFiles)
{
	outFiles.clear();
	outFiles.reserve(3);
	outFiles.push_back(dataDS);
	VString name;
	dataDS->GetNameWithoutExtension(name);
	VFolder* parent = dataDS->RetainParentFolder();

	VFile* indexFile = new VFile(*parent, name+kDataIndexExt);
	outFiles.push_back(dataDS);
	QuickReleaseRefCountable(indexFile);

	VFile* matchFile = new VFile(*parent, name+kDataMatchExt);
	outFiles.push_back(dataDS);
	QuickReleaseRefCountable(matchFile);

	QuickReleaseRefCountable(parent);
}


VError _EraseDataStore(VFile* dataDS)
{
	VError err = VE_OK;
	fileVector files;
	buildFullDSDataRef(dataDS, files);

	for (fileVector::iterator cur = files.begin(), end = files.end(); cur != end && err == VE_OK; cur++)
	{
		VFile* ff = *cur;
		if (ff->Exists())
		{
			err = ff->Delete();
		}
	}

	return err;
}


VError VDB4DMgr::EraseDataStore(VFile* dataDS)
{
	return _EraseDataStore(dataDS);
}


IHTTPRequestHandler* VDB4DMgr::AddRestRequestHandler( VErrorDB4D& outError, CDB4DBase *inBase, IHTTPServerProject *inHTTPServerProject, RIApplicationRef inApplicationRef, const XBOX::VString& inPattern, bool inEnabled)
{
	outError = VE_OK;

	RestRequestHandler *resthandler = NULL;
	if (inHTTPServerProject != NULL)
	{
		VString pattern = (!inPattern.IsEmpty()) ? inPattern : L"(?i)/rest/.*";
		resthandler = new RestRequestHandler( inBase, pattern, inEnabled, inApplicationRef);
		if (resthandler != NULL)
		{
			outError = inHTTPServerProject->AddHTTPRequestHandler (resthandler);
			if (outError != VE_OK)
				ReleaseRefCountable( &resthandler);
		}
		else
		{
			outError = VE_MEMORY_FULL;
		}

	}
	return resthandler;
}



//=================================================================================




VDB4DBase::VDB4DBase( VDBMgr *inManager, Base4D *inBase, Boolean mustRetain)
{
	fManager = inManager;
	inManager->Retain();

	fMustRetain = mustRetain;
	fBase = inBase;

	if (mustRetain)
	{
		inBase->Retain();
	}

	fIsRemote = inBase->IsRemote();
	fBackupSettings = NULL;
}


VDB4DBase::~VDB4DBase()
{
	XBOX::ReleaseRefCountable(&fBackupSettings);
	
	if (fMustRetain) 
		fBase->Release(); // a la derniere instance de fBase, le destructeur de celle ci appelera le CloseBase
	fManager->Release();
}


const IBackupSettings* VDB4DBase::RetainBackupSettings()
{
	if(fBase)
	{
		return fBase->RetainBackupSettings();
	}
	return NULL;
}

void  VDB4DBase::SetRetainedBackupSettings(IBackupSettings* inSettings)
{
	if (fBase)
	{
		fBase->SetRetainedBackupSettings(inSettings);
	}
}

Boolean VDB4DBase::MatchBase(CDB4DBase* InBaseToCompare) const
{
	if (fBase == ((VDB4DBase*)InBaseToCompare)->fBase) return true;
	else return false;
}


CDB4DBaseContextPtr VDB4DBase::NewContext(CUAGSession* inUserSession, VJSGlobalContext* inJSContext, bool isLocalOnly)
{
	CDB4DBaseContextPtr xbd;
	xbd = new BaseTaskInfo(fBase, inUserSession, inJSContext, this, isLocalOnly);
	return xbd;
}


static BaseTaskInfo* GetBaseTaskInfo(CDB4DBaseContextPtr inContext, Base4D* base)
{
	BaseTaskInfoPtr context = nil;
	if (inContext != nil)
	{
		context = (VImpCreator<BaseTaskInfo>::GetImpObject(inContext));
		assert(context->GetBase() == base);
	}
	return context;
}


class ContextMaker
{
	public:
		inline ContextMaker(CDB4DBaseContextPtr inContext, Base4D* base)
		{
			fContext = GetBaseTaskInfo(inContext, base);
			if (fContext == nil)
			{
				fContext = new BaseTaskInfo(base, nil, nil, nil);
			}
			else
			{
				fContext->Retain("ContextMaker");
			}
		};

		inline ~ContextMaker()
		{
			assert(fContext != nil);
			fContext->Release("ContextMaker");
		}

		operator BaseTaskInfo*() const	{ return fContext; }

		BaseTaskInfo* operator -> () const					{ return fContext; }

	protected:
		BaseTaskInfo* fContext;

};


void VDB4DBase::GetName( VString& outName, CDB4DBaseContextPtr inContext) const
{
	fBase->GetName( outName);
}


void VDB4DBase::GetUUID(VUUID& outID, CDB4DBaseContextPtr inContext) const
{
	BaseTaskInfo* context = GetBaseTaskInfo(inContext, fBase);
	fBase->GetUUID(outID);
}

const VUUID& VDB4DBase::GetUUID(CDB4DBaseContextPtr inContext) const
{
	BaseTaskInfo* context = GetBaseTaskInfo(inContext, fBase);
	return fBase->GetUUID();
}


CDB4DTable* VDB4DBase::CreateOutsideTable( const XBOX::VString& inTableName, CDB4DBaseContextPtr inContext, VErrorDB4D* outErr) const
{
	ObjLocker locker(inContext, fBase);

	VDB4DTable* result = nil;
	VErrorDB4D  vErr = VE_OK;
	if (locker.CanWork())
	{
		Table *table = new TableRegular( fBase, 0, false);

		if (table != nil)
		{
			vErr = table->SetName(inTableName, inContext, false);		// sc 21/12/2006, ACI0047257, pass the base context parameter
			if ( vErr == VE_OK )
				result = new VDB4DTable(fManager, const_cast<VDB4DBase*>(this), table);
			table-> Release ( );
		}
	}
	if ( outErr != 0 )
		*outErr = vErr;

	return result;
}

VError VDB4DBase::AddTable( CDB4DTable *inTable, CDB4DBaseContextPtr inContext)
{
	VError err = VE_OK;
	ObjLocker locker(inContext, fBase);
	if (locker.CanWork())
	{
		if (inTable != nil)
		{
			Table* tt = VImpCreator<VDB4DTable>::GetImpObject(inTable)->GetTable();
			tt->CanNowBeSaved();
			tt->CanNowKeepStamp(kDefaultKeepStamp);
			err = fBase->AddTable(tt, true, inContext);
		}
	}
	else
		err = VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT;

	return err;
}


VError VDB4DBase::CreateTable( const VValueBag& inTableDefinition, CDB4DTable **outTable, CDB4DBaseContextPtr inContext, Boolean inGenerateName)
{
	VError err = VE_OK;
	
	ObjLocker locker(inContext, fBase);
	if (locker.CanWork())
	{
		VBagLoader loader( true, true);

		if (outTable == NULL)
			err = fBase->CreateTable( inTableDefinition, &loader, NULL, inContext, inGenerateName);
		else
		{
			Table *table;
			err = fBase->CreateTable( inTableDefinition, &loader, &table, inContext, inGenerateName);

			if (table != NULL)
			{
				if (!fIsRemote)
					err = table->save();

				*outTable = new VDB4DTable( fManager, const_cast<VDB4DBase*>(this), table);
				table->Release();
			}
			else
			{
				*outTable = NULL;
			}
		}
	}
	else
		err = VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT;
	
	return err;
}


VError VDB4DBase::GetNameForNewTable( VString& outName) const
{
	return fBase->GetNameForNewTable( outName);
}


VValueBag *VDB4DBase::CreateDefinition( bool inWithTables, bool inWithIndices, bool inWithRelations, CDB4DBaseContextPtr inContext) const
{
	VValueBag *bag = new VValueBag;
	if (bag != NULL)
	{
		VString kind;
		VError err = fBase->SaveToBagDeep( *bag, kind, inWithTables, inWithIndices, inWithRelations);
		vThrowError( err);
	}
	return bag;
}


Boolean VDB4DBase::CreateElements( const VValueBag& inBag, VBagLoader *inLoader, CDB4DBaseContextPtr inContext)
{
	ObjLocker locker(inContext, fBase);
	if (locker.CanWork())
	{
		return fBase->CreateElements( inBag, inLoader, inContext);
	}
	else
		return false;
}


VValueBag *VDB4DBase::CreateTableDefinition( DB4D_TableID inTableID, CDB4DBaseContextPtr inContext) const
{
	VValueBag *bag = NULL;
	Table *file = fBase->RetainTable( (sLONG) inTableID);
	if (file != NULL)
	{
		bag = new VValueBag;
		if (bag != NULL)
		{
			VString kind;
			VError err = file->SaveToBag( *bag, kind);
			vThrowError( err);
		}

		file->Release();
	}
	
	return bag;
}

/*
DB4D_TableID VDB4DBase::GetTableByName( const VString& inName) const
{
	DB4D_TableID id = kDB4D_NullTableID;
	sLONG index = fBase->FindTable( inName);
	if (index > 0)
		id = (DB4D_TableID) index;
		
	return id;
}
*/


VError VDB4DBase::CreateFullTextIndexOnOneField(CDB4DField* target, VDB4DProgressIndicator* InProgress, 
												const VString *inIndexName , CDB4DIndex** outResult, Boolean ForceCreate, 
												VSyncEvent* event, CDB4DBaseContextPtr inContext)
{
	VError err;

	// ObjLocker locker(inContext, fBase);
	// if (locker.CanWork())
	{
		IndexInfo **outindex = nil, *ind = NULL;
		if (outResult != nil)
			outindex = &ind;

		err = fBase->CreFullTextIndexOnField(VImpCreator<VDB4DField>::GetImpObject(target)->GetField(), inContext, InProgress,
											inIndexName, outindex, ForceCreate, event);
		if (outResult != nil && *outindex != NULL)
		{
			*outResult = new VDB4DIndex(*outindex);
			(*outindex)->Release();
		}
	}
	// else
		// err = VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT;

	return err;
}


VError VDB4DBase::DropFullTextIndexOnOneField(CDB4DField* target, VDB4DProgressIndicator* InProgress, VSyncEvent* event, CDB4DBaseContextPtr inContext)
{
	VError err;

	// ObjLocker locker(inContext, fBase);
	// if (locker.CanWork())
	{
		err = fBase->DeleteFullTextIndexOnField(VImpCreator<VDB4DField>::GetImpObject(target)->GetField(), inContext, InProgress, event);
	}
	// else
		// err = VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT;
	return err;
}


VError VDB4DBase::CreateIndexOnOneField(CDB4DField* target, sLONG IndexTyp, Boolean UniqueKeys, VDB4DProgressIndicator* InProgress, 
										const VString *inIndexName, CDB4DIndex** outResult, Boolean ForceCreate, VSyncEvent* event,
										CDB4DBaseContextPtr inContext)
{
	VError err;
	
	// ObjLocker locker(inContext, fBase);
	// if (locker.CanWork())
	{
		IndexInfo **outindex = nil, *ind = nil;
		if (outResult != nil)
			outindex = &ind;
		
		err = fBase->CreIndexOnField(VImpCreator<VDB4DField>::GetImpObject(target)->GetField(), IndexTyp, UniqueKeys, inContext, InProgress,
										inIndexName, outindex, ForceCreate, event);
		
		if (outResult != nil && *outindex != nil)	// sc 02/07/2007 ACI0052845, outindex is NULL if the index has not been created.
		{
			*outResult = new VDB4DIndex(*outindex);
			(*outindex)->Release();
		}
	}
	// else
		// err = VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT;

	return err;
}


VError VDB4DBase::CreateIndexOnMultipleField(const CDB4DFieldArray& inTargets, sLONG IndexTyp, Boolean UniqueKeys, 
											VDB4DProgressIndicator* InProgress, const VString *inIndexName, CDB4DIndex** outResult, 
											Boolean ForceCreate, VSyncEvent* event, CDB4DBaseContextPtr inContext)
{
	VError err = VE_OK;

	// ObjLocker locker(inContext, fBase);
	// if (locker.CanWork())
	{
		sLONG nbfield = inTargets.GetCount();
		FieldNuplet fields(nbfield, true);
		CDB4DField* const *cur; 
		CDB4DField* const *end = inTargets.AfterLast();
		sLONG i;

		for (i = 1, cur = inTargets.First(); cur != end; cur++, i++)
		{
			CDB4DField* xcri = *cur;
			Field* cri = VImpCreator<VDB4DField>::GetImpObject(xcri)->GetField();
			assert(cri != nil);
			fields.SetNthField(i, cri);
		}

		IndexInfo **outindex = nil, *ind = NULL;
		if (outResult != nil)
			outindex = &ind;

		err = fBase->CreIndexOnFields(&fields, IndexTyp, UniqueKeys, inContext, InProgress, inIndexName, outindex, ForceCreate, event);

		if (outResult != nil && *outindex != NULL)
		{
			*outResult = new VDB4DIndex(*outindex);
			(*outindex)->Release();
		}
	}
	// else
		// err = VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT;

	return err;
}


VError VDB4DBase::DropIndexByName(const VString& inIndexName, VDB4DProgressIndicator* InProgress, VSyncEvent* event, CDB4DBaseContextPtr inContext)
{
	VError err = VE_OK;

	// ObjLocker locker(inContext, fBase);
	// if (locker.CanWork())
	{
		IndexInfo* ind = fBase->FindAndRetainIndexByName(inIndexName, false);
		if (ind != nil)
		{
			ObjLocker locker2(inContext, ind, /*&locker*/nil);
			if (locker2.CanWork())
			{
				fBase->DeleteIndexByRef(ind, inContext, InProgress, event);
			}
			else
				err = VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT;
			ind->Release();
		}
		else
		{
			if (event != nil)
				event->Unlock();
			err = VE_DB4D_WRONGINDEXNAME;
		}
	}
	// else
		// err = VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT;

	return err;
}


VError VDB4DBase::CreateIndexOnMultipleField(const VValueBag& IndexDefinition, Boolean UniqueKeys, 
											VDB4DProgressIndicator* InProgress, CDB4DIndex** outResult, Boolean ForceCreate, 
											VSyncEvent* event, CDB4DBaseContextPtr inContext)
{
	sLONG i;
	VError err = VE_OK;
	sLONG IndexTyp;
	FieldNuplet *fields;
	VString indexname;
	Field *cri;
	
	// ObjLocker locker(inContext, fBase);
	// if (locker.CanWork())
	{
		VBagLoader loader( true, true);

		IndexDefinition.GetString( DB4DBagKeys::name, indexname);

		const VBagArray *fieldsDef = IndexDefinition.RetainElements( DB4DBagKeys::field_ref);
		sLONG nbfield = fieldsDef->GetCount();
		fields = new FieldNuplet(nbfield, true);
		
		for(i = 1 ; i <= nbfield && err == VE_OK ; i++) 
		{
			const VValueBag *oneFieldBag = fieldsDef->GetNth( i);
			cri = fBase->FindAndRetainFieldRef( *oneFieldBag, &loader);
			if (cri != nil)
			{
				fields->SetNthField(i, cri);
				cri->Release();
			}
			else
			{
				err = fBase->ThrowError(VE_DB4D_WRONGFIELDREF, DBactionFinale);
			}
		}
		
		if (!IndexDefinition.GetLong( DB4DBagKeys::type, IndexTyp))
			IndexTyp = DB4D_Index_Btree;
		
		IndexInfo **outindex = nil, *ind = NULL;
		if (outResult != nil)
			outindex = &ind;

		err = fBase->CreIndexOnFields(fields, IndexTyp, UniqueKeys, inContext, InProgress, indexname.GetLength() == 0 ? nil : &indexname, outindex, ForceCreate, event);

		if (outResult != nil && *outindex != NULL)
		{
			*outResult = new VDB4DIndex(*outindex);
			(*outindex)->Release();
		}

		delete fields;
		fieldsDef->Release();
	}
	// else
		// err = VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT;

	return err;
}




VError VDB4DBase::DropIndexOnOneField(CDB4DField* target, sLONG IndexTyp, VDB4DProgressIndicator* InProgress, VSyncEvent* event, CDB4DBaseContextPtr inContext)
{
	VError err;
	
	// ObjLocker locker(inContext, fBase);
	// if (locker.CanWork())
	{
		err = fBase->DeleteIndexOnField(VImpCreator<VDB4DField>::GetImpObject(target)->GetField(), inContext, IndexTyp, InProgress, event);
	}
	// else
		// err = VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT;

	return err;
}


VError VDB4DBase::DropIndexOnMultipleField(const CDB4DFieldArray& inTargets, sLONG IndexTyp, VDB4DProgressIndicator* InProgress, 
										   VSyncEvent* event, CDB4DBaseContextPtr inContext)
{
	VError err;

	// ObjLocker locker(inContext, fBase);
	// if (locker.CanWork())
	{
		sLONG nbfield = inTargets.GetCount();
		FieldNuplet fields(nbfield, true);
		CDB4DField* const *cur; 
		CDB4DField* const *end = inTargets.AfterLast();
		sLONG i;

		for (i = 1, cur = inTargets.First(); cur != end; cur++, i++)
		{
			CDB4DField* xcri = *cur;
			Field* cri = VImpCreator<VDB4DField>::GetImpObject(xcri)->GetField();
			assert(cri != nil);
			fields.SetNthField(i, cri);
		}

		err = fBase->DeleteIndexOnFields(&fields, inContext, IndexTyp, InProgress, event);
	}
	// else
		// err = VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT;

	return err;
}


VError VDB4DBase::DropIndexOnMultipleField(const VValueBag& IndexDefinition, VDB4DProgressIndicator* InProgress, VSyncEvent* event, CDB4DBaseContextPtr inContext)
{
	sLONG i;
	VError err = VE_OK;
	sLONG IndexTyp;
	FieldNuplet *fields;
	VStr<64> fieldname;
	VStr<64> tablename;
	Field *cri;
	
	// ObjLocker locker(inContext, fBase);
	// if (locker.CanWork())
	{
		const VBagArray *fieldsDef = IndexDefinition.RetainElements( L"fields");
		sLONG nbfield = fieldsDef->GetCount();
		fields = new FieldNuplet(nbfield, true);
		
		for(i = 1 ; i <= nbfield && err == VE_OK ; i++) 
		{
			const VValueBag *oneFieldBag = fieldsDef->RetainNth( i);

			if (oneFieldBag->GetString( L"fieldname", fieldname) && oneFieldBag->GetString( L"tablename", tablename))
			{
				cri = fBase->FindAndRetainFieldRef(tablename,fieldname);
				if (cri == nil) err = fBase->ThrowError(VE_DB4D_WRONGFIELDREF, DBactionFinale);
				else
				{
					fields->SetNthField(i, cri);
					cri->Release();
				}
			}
			else 
				err = fBase->ThrowError(VE_DB4D_WRONGFIELDREF, DBactionFinale);
			oneFieldBag->Release();
		}
		
		if (!IndexDefinition.GetLong(L"indextype", IndexTyp)) IndexTyp = 0;
		
		err = fBase->DeleteIndexOnFields(fields, inContext, IndexTyp, InProgress, event);
		
		delete fields;
		fieldsDef->Release();
	}
	// else
		// err = VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT;

	return err;
}


#if debuglr
CComponent* VDB4DBase::Retain(const char* DebugInfo)
{
	return VComponentImp<CDB4DBase>::Retain(DebugInfo);
}

void VDB4DBase::Release(const char* DebugInfo)
{
	VComponentImp<CDB4DBase>::Release(DebugInfo);
}
#endif


void VDB4DBase::Close(VSyncEvent* waitForClose)
{
#if UseDB4DJSContext
	releaseAllTempStores();
#endif

	UnRegisterForLang();
	fBase->BeginClose(waitForClose);
	VDBMgr::GetManager()->UnRegisterBase(fBase);
}


void VDB4DBase::CloseAndRelease(Boolean WaitUntilFullyClosed)
{
#if trackClose
	trackDebugMsg("Begining of Close and Release\n");
#endif
	if (!fBase->IsWriteProtected())
		Flush(true);
#if UseDB4DJSContext
	releaseAllTempStores();
#endif
	VSyncEvent* waitclose = nil;
	UnRegisterForLang();
	if (WaitUntilFullyClosed)
		waitclose = new VSyncEvent();
	fBase->BeginClose(waitclose);
	VDBMgr::GetManager()->UnRegisterBase(fBase);

	assert(fBase->GetRefCount() == 1);
	assert(GetRefCount() == 1);
	Release();  // attention this n'est plus valide

#if trackClose
	trackDebugMsg("End of Close and Release\n");
#endif

	if (waitclose != nil)
	{
		waitclose->Lock();
		waitclose->Release();
	}
}


CDB4DBase* VDB4DBase::RetainStructDatabase(const char* DebugInfo) const
{
	return fBase->RetainStructDatabase(DebugInfo);
}


sLONG VDB4DBase::CountTables(CDB4DBaseContextPtr inContext) const
{
	return fBase->GetNBTable();
}



void VDB4DBase::GetBasePath( VFilePath& outPath, CDB4DBaseContextPtr inContext) const
{
	fBase->GetDataSegPath( 1, outPath);
}

Boolean VDB4DBase::OpenData( const VFile& inDataFile, sLONG inParameters, CDB4DBaseContextPtr inContext, VError* outErr, FileAccess inAccess) const
{
	VError err = fBase->OpenData( inDataFile, inParameters, false, true, inAccess);
	if (fBase->FlushNotCompleted())
		err = VE_DB4D_FLUSH_COMPLETE_ON_DATA;
	if (outErr != nil)
		*outErr = err;
	return err == VE_OK;
}

XBOX::VFile* VDB4DBase::RetainIndexFile() const
{
	return fBase->RetainIndexFile();
}


Boolean VDB4DBase::IsDataOpened(CDB4DBaseContextPtr inContext) const
{
	return fBase->IsDataOpened();
}


void VDB4DBase::TouchXML()
{
	fBase->TouchXML();
}


VError VDB4DBase::LoadIndexesAfterCompacting(sLONG inParameters)
{
	return fBase->LoadIndexesAfterCompacting(inParameters);
}


Boolean VDB4DBase::CreateData( const VFile& inDataFile, sLONG inParameters, VFile *inJournalingFile, CDB4DBaseContextPtr inContext, VError* outErr, FileAccess inAccess) const
{
	VError err = fBase->CreateData( inDataFile, inParameters, fBase->GetIntlMgr(), inJournalingFile, false, inAccess);
	if (outErr != nil)
		*outErr = err;
	return err == VE_OK;
}


Boolean VDB4DBase::OpenDefaultData( Boolean inCreateItIfNotFound, sLONG inParameters, CDB4DBaseContextPtr inContext, VError* outErr, FileAccess inAccess) const
{
	if (fBase->IsDataOpened())
		return false;
	
	Boolean isOK = false;
	VError err = VE_OK;
	VFile *theFile = fBase->RetainDefaultDataFile();
	if ( (theFile != NULL) && theFile->Exists()) 
	{
		err = fBase->OpenData( *theFile, inParameters, false, true, inAccess);
		if (fBase->FlushNotCompleted())
			err = VE_DB4D_FLUSH_COMPLETE_ON_DATA;
	} 
	else if (inCreateItIfNotFound) 
	{
		err = fBase->CreateData( *theFile, inParameters, fBase->GetIntlMgr(), NULL, false, inAccess);
	} 
	else
		err = VE_UNKNOWN_ERROR;	// should find a better err
	ReleaseRefCountable( &theFile);

	if (outErr != nil)
		*outErr = err;
	return err == VE_OK;
}


void VDB4DBase::RegisterForLang(void)
{
	fBase->RegisterForLang();
}


void VDB4DBase::UnRegisterForLang(void)
{
	fBase->UnRegisterForLang();
}


Boolean VDB4DBase::DropTable(DB4D_TableID inTableID, VDB4DProgressIndicator* InProgress, CDB4DBaseContextPtr inContext)
{
	Boolean ok = false;
	ObjLocker locker(inContext, fBase);
	if (locker.CanWork())
	{
		Table* fic = fBase->RetainTable(inTableID);
		if (fic != nil) 
		{
			ObjLocker locker2(inContext, fic, false, &locker);
			if (locker2.CanWork())
			{
				ok = fBase->DeleteTable(fic, inContext);
			}
			fic->Release();
		}	
	}
	return ok;
}

/*
CNameSpace* VDB4DBase::GetNameSpace() const
{
	return fBase->GetNameSpace();
}
*/

/*
VValueBag* VDB4DBase::CreateIndexDefinition(sLONG inIndexNum) const
{
	VValueBag *bag = NULL;
	IndexInfo *ind = fBase->RetainIndex( inIndexNum);
	if (ind != NULL)
	{
		bag = new VValueBag;
		if (bag != NULL)
		{
			VString kind;
			VError err = ind->SaveToBag( *bag, kind);
			vThrowError( err);
		}
		ind->Release();
	}

	return bag;
}

	
sLONG VDB4DBase::CountIndices() const
{
	return fBase->CountIndices();
}
*/

CDB4DIndex* VDB4DBase::FindAndRetainIndexByName(const VString& inIndexName, CDB4DBaseContextPtr inContext, Boolean MustBeValid, Boolean Occupy) const
{
	CDB4DIndex* result = nil;

	BaseTaskInfoPtr context = nil;
	if (inContext != nil)
	{
		context = ConvertContext(inContext);
		assert(context->GetBase() == fBase);
	}

	IndexInfo* ind = fBase->FindAndRetainIndexByName(inIndexName, MustBeValid, context, Occupy);
	if (ind != nil)
	{
		result = new VDB4DIndex(ind);
		if (MustBeValid && !Occupy)
			ind->ReleaseValid();
		ind->Release();
}

	return result;
}


CDB4DIndex* VDB4DBase::FindAndRetainIndexByUUID( const VUUID& inUUID, CDB4DBaseContextPtr inContext, Boolean MustBeValid, Boolean Occupy) const
{
	CDB4DIndex *result = NULL;

	BaseTaskInfoPtr context = nil;
	if (inContext != nil)
	{
		context = ConvertContext(inContext);
		assert(context->GetBase() == fBase);
	}

	IndexInfo *ind = fBase->FindAndRetainIndexByUUID( inUUID, MustBeValid, context, Occupy);
	if (ind != NULL)
	{
		result = new VDB4DIndex( ind);
		if (MustBeValid && !Occupy)
			ind->ReleaseValid();
		ind->Release();
	}

	return result;
}


CDB4DIndex* VDB4DBase::RetainIndex(sLONG inNumTable, sLONG inNumField, Boolean MustBeSortable, CDB4DBaseContextPtr inContext, Boolean MustBeValid, Boolean Occupy) const
{
	CDB4DIndex* result = nil;

	BaseTaskInfoPtr context = nil;
	if (inContext != nil)
	{
		context = ConvertContext(inContext);
		assert(context->GetBase() == fBase);
	}

	IndexInfo* ind = fBase->FindAndRetainIndexSimple(inNumTable, inNumField, MustBeSortable, MustBeValid, context, Occupy);
	if (ind != nil)
	{
		result = new VDB4DIndex(ind);
		if (MustBeValid && !Occupy)
			ind->ReleaseValid();
		ind->Release();
	}
	return result;
}


CDB4DIndex* VDB4DBase::RetainFullTextIndex(sLONG inNumTable, sLONG inNumField, CDB4DBaseContextPtr inContext, Boolean MustBeValid, Boolean Occupy) const
{
	CDB4DIndex* result = nil;

	BaseTaskInfoPtr context = nil;
	if (inContext != nil)
	{
		context = ConvertContext(inContext);
		assert(context->GetBase() == fBase);
	}

	IndexInfo* ind = fBase->FindAndRetainIndexLexico(inNumTable, inNumField, MustBeValid, context, Occupy);
	if (ind != nil)
	{
		result = new VDB4DIndex(ind);
		if (MustBeValid && !Occupy)
			ind->ReleaseValid();
		ind->Release();
	}
	return result;
}


CDB4DIndex* VDB4DBase::RetainCompositeIndex(const CDB4DFieldArray& inTargets, Boolean MustBeSortable, CDB4DBaseContextPtr inContext, Boolean MustBeValid, Boolean Occupy) const
{
	CDB4DIndex* result = nil;
	sLONG nbfield = inTargets.GetCount();
	FieldNuplet fields(nbfield, true);
	CDB4DField* const *cur; 
	CDB4DField* const *end = inTargets.AfterLast();
	sLONG i;

	BaseTaskInfoPtr context = nil;
	if (inContext != nil)
	{
		context = ConvertContext(inContext);
		assert(context->GetBase() == fBase);
	}

	for (i = 1, cur = inTargets.First(); cur != end; cur++, i++)
	{
		CDB4DField* xcri = *cur;
		Field* cri = VImpCreator<VDB4DField>::GetImpObject(xcri)->GetField();
		assert(cri != nil);
		fields.SetNthField(i, cri);
	}

	IndexInfo* ind2 = new IndexInfoFromMultipleField(fBase,&fields,1,false);
	if (ind2!=nil)
	{
		IndexInfo* ind = fBase->FindAndRetainAnyIndex(ind2, MustBeSortable, MustBeValid, context, Occupy);
		if (ind != nil)
		{
			result = new VDB4DIndex(ind);
			if (MustBeValid && !Occupy)
				ind->ReleaseValid();
			ind->Release();
		}
	}

	return result;
}


/*
CDB4DIndex* VDB4DBase::RetainNthIndex(sLONG inIndexNum) const
{
	CDB4DIndex* result = nil;
	IndexInfo* ind = fBase->RetainIndex(inIndexNum);
	if (ind != nil)
	{
		result = new VDB4DIndex(ind);
		ind->Release();
	}

	return result;
}
*/


CDB4DCheckAndRepairAgent* VDB4DBase::NewCheckAndRepairAgent()
{
	CDB4DCheckAndRepairAgent* ckinf = new VDB4DCheckAndRepairAgent(this);
	return ckinf;

}


VError VDB4DBase::CreateRelation( const VValueBag& inBag, CDB4DRelation **outRelation, CDB4DBaseContextPtr inContext)
{
	VError err = VE_OK;

	ObjLocker locker(inContext, fBase);
	if (locker.CanWork())
	{
		VBagLoader loader( true, true);

		if (outRelation == NULL)
			err = fBase->CreateRelation( inBag, &loader, NULL, inContext);
		else
		{
			Relation *rel;
			err = fBase->CreateRelation( inBag, &loader, &rel, inContext);

			if (rel != NULL)
			{
				*outRelation = new VDB4DRelation( this, rel);
				rel->Release();
			}
			else
			{
				*outRelation = NULL;
			}
		}

		/*
		if (err == VE_OK)
		{
			err = fBase->SaveRelations();
		}
		*/
	}
	else
		err = VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT;
	return err;
}


CDB4DRelation *VDB4DBase::CreateRelation(const VString &inRelationName, const VString &inCounterRelationName, CDB4DField* inSourceField, 
										 CDB4DField* inDestinationField, CDB4DBaseContextPtr inContext)
{
	VDB4DRelation* crel = NULL;
	VError err = VE_OK;
	
	ObjLocker locker(inContext, fBase);
	if (locker.CanWork())
	{
		if (inSourceField != NULL &&& inDestinationField != NULL)
		{
			Relation *rel = fBase->CreateRelation( inRelationName, inCounterRelationName
						, VImpCreator<VDB4DField>::GetImpObject(inSourceField)->GetField()
						, VImpCreator<VDB4DField>::GetImpObject(inDestinationField)->GetField()
						, err, inContext);

			if (rel != NULL)
			{
				crel = new VDB4DRelation( this, rel);
				rel->Release(); // sc 18/10/2006
				//err = fBase->SaveRelations();
			}
		}
		else
		{
			err = VE_DB4D_WRONGSOURCEFIELD;
		}
	}
	else
		err = VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT;

	return crel;
}


static void CopyAndRetainFieldArray(const CDB4DFieldArray& infields, FieldArray& outfields)
{
	for (CDB4DFieldArray::ConstIterator cur = infields.First(), end = infields.End(); cur != end; cur++)
	{
		Field* cri =  VImpCreator<VDB4DField>::GetImpObject(*cur)->GetField();
		if (testAssert(cri != nil))
			cri->Retain();
		outfields.Add(cri);
	}
}


static void CopyFieldArray(const CDB4DFieldArray& infields, FieldArray& outfields)
{
	for (CDB4DFieldArray::ConstIterator cur = infields.First(), end = infields.End(); cur != end; cur++)
	{
		Field* cri =  VImpCreator<VDB4DField>::GetImpObject(*cur)->GetField();
		outfields.Add(cri);
	}
}


CDB4DRelation *VDB4DBase::CreateRelation(const VString &inRelationName, const VString &inCounterRelationName,
										const CDB4DFieldArray& inSourceFields, const CDB4DFieldArray& inDestinationFields,
										CDB4DBaseContextPtr inContext)
{
	VDB4DRelation* crel = NULL;
	VError err = VE_OK;

	ObjLocker locker(inContext, fBase);
	if (locker.CanWork())
	{
		Relation *rel = fBase->CreateRelation( inRelationName, inCounterRelationName, inSourceFields , inDestinationFields , err, inContext);
		if (rel != NULL)
		{
			crel = new VDB4DRelation( this, rel);
			//err = fBase->SaveRelations();
		}
	}
	else
		err = VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT;
	return crel;
}



CDB4DRelation* VDB4DBase::FindAndRetainRelationByUUID(const VUUID& inID, CDB4DBaseContextPtr inContext) const
{
	VDB4DBase* x = const_cast<VDB4DBase*>(this);
	CDB4DBase* xthis = x;

	VDB4DRelation* rel = nil;

	Relation* xrel = fBase->FindAndRetainRelationByUUID(inID);
	if (xrel != nil)
	{
		rel = new VDB4DRelation(xthis, xrel);
		xrel->Release();
	}
		
	return rel;
}


CDB4DRelation* VDB4DBase::FindAndRetainRelationByName(const VString& Name, CDB4DBaseContextPtr inContext) const
{
	VDB4DBase* x = const_cast<VDB4DBase*>(this);
	CDB4DBase* xthis = x;

	VDB4DRelation* rel = nil;

	Relation* xrel = fBase->FindAndRetainRelationByName(Name);
	if (xrel != nil)
	{
		rel = new VDB4DRelation(xthis, xrel);
		xrel->Release();
	}
		
	return rel;
}


CDB4DRelation* VDB4DBase::FindAndRetainRelation(const CDB4DFieldArray& inSourceFields, const CDB4DFieldArray& inDestinationFields, 
												CDB4DBaseContextPtr inContext) const
{
	FieldArray sources, dests;
	CopyFieldArray(inSourceFields, sources);
	CopyFieldArray(inDestinationFields, dests);
	VDB4DBase* x = const_cast<VDB4DBase*>(this);
	CDB4DBase* xthis = x;

	VDB4DRelation* rel = nil;

	Relation* xrel = fBase->FindAndRetainRelation(sources, dests);

	if (xrel != NULL)
	{
		rel = new VDB4DRelation(xthis, xrel);
		xrel->Release();
	}

	return rel;
}

CDB4DRelation* VDB4DBase::FindAndRetainRelation(CDB4DField* inSourceField, CDB4DField* inDestinationField, 
												CDB4DBaseContextPtr inContext) const
{
	VDB4DBase* x = const_cast<VDB4DBase*>(this);
	CDB4DBase* xthis = x;

	VDB4DRelation* rel = nil;

	if (inSourceField != nil && inDestinationField != nil)
	{
		Relation* xrel = fBase->FindAndRetainRelation(
				VImpCreator<VDB4DField>::GetImpObject(inSourceField)->GetField()
				, VImpCreator<VDB4DField>::GetImpObject(inDestinationField)->GetField());
		
		if (xrel != NULL)
		{
			rel = new VDB4DRelation(xthis, xrel);
			xrel->Release();
		}
	}
		
	return rel;
}


VError VDB4DBase::GetAndRetainRelations(RelationArray &outRelations, CDB4DBaseContextPtr inContext) const
{
	VDB4DBase* x = const_cast<VDB4DBase*>(this);
	CDB4DBase* xthis = x;

	VError err = VE_OK;
	VArrayRetainedOwnedPtrOf<Relation*> xrels;

	fBase->GetAndRetainRelations(xrels);
	sLONG i, nb = xrels.GetCount();
	if (outRelations.SetAllocatedSize(nb))
	{
		outRelations.SetCount(nb);
		for (i = 0; i < nb; i++)
		{
			outRelations[i] = new VDB4DRelation(xthis, xrels[i]);
		}
	}
	else
		err = ThrowBaseError(memfull, DBaction_BuildingListOfRelations);

	return err;
}


VErrorDB4D VDB4DBase::IsFieldsKindValidForRelation( CDB4DField* inSourceField, CDB4DField* inDestinationField) const
{
	VErrorDB4D err = VE_OK;
	Field *source = (inSourceField != NULL) ? VImpCreator<VDB4DField>::GetImpObject(inSourceField)->GetField() : NULL;
	Field *dest = (inDestinationField != NULL) ? VImpCreator<VDB4DField>::GetImpObject(inDestinationField)->GetField() : NULL;
	
	if (source == nil)
	{
		err = VE_DB4D_WRONGSOURCEFIELD;
	}
	else if (!source->CanBePartOfRelation())
	{
		err = VE_DB4D_WRONGSOURCEFIELD;
	}
	else if (dest == nil)
	{
		err = VE_DB4D_WRONGDESTINATIONFIELD;
	}
	else if (!dest->CanBePartOfRelation())
	{
		err = VE_DB4D_WRONGDESTINATIONFIELD;
	}
	else if (source->GetTyp() != dest->GetTyp())
	{
		err = VE_DB4D_FIELDTYPENOTMATCHING;
	}

	return err;
}


uBOOL VDB4DBase::Lock( CDB4DBaseContextPtr inContext, Boolean WaitForEndOfRecordLocks, sLONG TimeToWaitForEndOfRecordLocks )
{
	uBOOL result = false;
	BaseTaskInfo* context = GetBaseTaskInfo(inContext, fBase);
	if ( context )
		result = fBase->Lock(context, false, WaitForEndOfRecordLocks, TimeToWaitForEndOfRecordLocks);
	return result;
}

void VDB4DBase::UnLock( CDB4DBaseContextPtr inContext )
{
	BaseTaskInfo* context = GetBaseTaskInfo(inContext, fBase);
	if ( context )
		fBase->UnLock(context);
}

uBOOL VDB4DBase::FlushAndLock( CDB4DBaseContextPtr inContext, Boolean WaitForEndOfRecordLocks, sLONG TimeToWaitForEndOfRecordLocks )
{
	uBOOL result = false;
	BaseTaskInfo* context = GetBaseTaskInfo(inContext, fBase);
	if ( context )
		result = fBase->FlushAndLock(context, WaitForEndOfRecordLocks, TimeToWaitForEndOfRecordLocks);
	return result;
}

CDB4DTable* VDB4DBase::RetainNthTable(sLONG inIndex, CDB4DBaseContextPtr inContext) const
{
	CDB4DTable *theTable = nil;
	Table *file = fBase->RetainTable( inIndex);
	if (file != nil) {
		theTable = new VDB4DTable( fManager, const_cast<VDB4DBase*>(this), file);
		file->Release();
	}
	return theTable;
}


CDB4DTable* VDB4DBase::FindAndRetainTable(const VString& inName, CDB4DBaseContextPtr inContext) const
{
	CDB4DTable *theTable = nil;
	Table *file = fBase->FindAndRetainTableRef(inName);
	if (file != nil) {
		theTable = new VDB4DTable( fManager, const_cast<VDB4DBase*>(this), file);
		file->Release();
	}
	return theTable;
}


CDB4DTable* VDB4DBase::FindAndRetainTable(const VUUID& inID, CDB4DBaseContextPtr inContext) const
{
	CDB4DTable *theTable = nil;
	Table *file = fBase->FindAndRetainTableRef(inID);
	if (file != nil) {
		theTable = new VDB4DTable( fManager, const_cast<VDB4DBase*>(this), file);
		file->Release();
	}
	return theTable;
}

CDB4DTable* VDB4DBase::FindAndRetainTableInStruct(const VUUID& inID, CDB4DBaseContextPtr inContext) const
{
	CDB4DTable *theTable = nil;
	Base4D		*structure;
	if (testAssert(fBase && NULL != (structure = fBase->GetStructure ()))) {
		Table *file = structure->FindAndRetainTableRef(inID);
		if (file != nil) {
			theTable = new VDB4DTable( fManager, const_cast<VDB4DBase*>(this), file);
			file->Release();
		}
	}
	return theTable;
}

CDB4DField* VDB4DBase::FindAndRetainField(const VUUID& inID, CDB4DBaseContextPtr inContext) const
{
	CDB4DField *theField = nil;
	Field *cri = fBase->FindAndRetainFieldRef(inID);
	if (cri != nil) {
		theField = new VDB4DField( cri);
		cri->Release();
	}
	return theField;
}


CDB4DField* VDB4DBase::FindAndRetainField(const VString& inTableName, const VString& inFieldName, CDB4DBaseContextPtr inContext) const
{
	CDB4DField *theField = nil;
	Field *cri = fBase->FindAndRetainFieldRef(inTableName, inFieldName);
	if (cri != nil) {
		theField = new VDB4DField( cri);
		cri->Release();
	}
	return theField;
}


CDB4DField* VDB4DBase::FindAndRetainField(const XBOX::VString& inFieldName, CDB4DBaseContextPtr inContext) const
{
	CDB4DField *theField = nil;
	Field *cri = fBase->FindAndRetainFieldRef(inFieldName);
	if (cri != nil) {
		theField = new VDB4DField( cri);
		cri->Release();
	}
	return theField;
}


CDB4DAutoSeqNumber* VDB4DBase::CreateAutoSeqNumber(DB4D_AutoSeq_Type inType, VErrorDB4D& err, CDB4DBaseContextPtr inContext)
{
	ObjLocker locker(inContext, fBase);
	if (locker.CanWork())
	{
		AutoSeqNumber* seq = fBase->AddSeqNum(inType, err, inContext);
		if (seq == nil)
			return nil;
		else
		{
			return new VDB4DAutoSeqNumber(seq);
		}
	}
	else
		return nil;
}


VErrorDB4D VDB4DBase::DropAutoSeqNumber(const XBOX::VUUID& inIDtoDelete, CDB4DBaseContextPtr inContext)
{
	ObjLocker locker(inContext, fBase);
	if (locker.CanWork())
	{
		return fBase->DeleteSeqNum(inIDtoDelete, inContext);
	}
	else
		return VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT;
}


CDB4DAutoSeqNumber* VDB4DBase::RetainAutoSeqNumber(const XBOX::VUUID& inIDtoFind, CDB4DBaseContextPtr inContext)
{
	AutoSeqNumber* seq = fBase->RetainSeqNum(inIDtoFind);
	if (seq == nil)
		return nil;
	else
	{
		CDB4DAutoSeqNumber* result = new VDB4DAutoSeqNumber(seq);
		seq->Release();
		return result;
	}
}


VError VDB4DBase::SetJournalFile(VFile* inNewLog, const VUUID* inDataLink, CDB4DBaseContextPtr inContext, bool inResetJournalSequenceNumber)
{
	return fBase->SetJournalFile(inNewLog,inDataLink,inResetJournalSequenceNumber);
}

VError VDB4DBase::SetJournalFileInfos( VString *inFilePath, VUUID *inUUID )
{
	return fBase->SetJournalFileInfos( inFilePath, inUUID, false);
}

VError VDB4DBase::ResetJournalFileContent()
{
	return fBase->ResetJournalFileContent();
}

VFile* VDB4DBase::RetainJournalFile(CDB4DBaseContextPtr inContext)
{
	return fBase->RetainJournalFile();
}

bool VDB4DBase::GetJournalUUIDLink( VUUID &outLink )
{
	return fBase->GetJournalUUIDLink( outLink );
}

void VDB4DBase::GetJournalInfos( const VFilePath &inDataFilePath, VFilePath &outFilePath, VUUID &outDataLink)
{
	fBase->GetJournalInfos( inDataFilePath, outFilePath, outDataLink);
}

VError VDB4DBase::OpenJournal( VFile *inFile, VUUID &inDataLink, bool inWriteOpenDataOperation )
{
	return fBase->OpenJournal( inFile, inDataLink, inWriteOpenDataOperation);
}

VError VDB4DBase::CreateJournal( VFile *inFile, VUUID *inDataLink, bool inWriteOpenDataOperation )
{
	return fBase->CreateJournal( inFile, inDataLink, inWriteOpenDataOperation);
}

const VValueBag* VDB4DBase::RetainExtraProperties(VError &err, CDB4DBaseContextPtr inContext)
{
	return fBase->RetainExtraProperties(err, inContext);
}


VError VDB4DBase::SetExtraProperties(VValueBag* inExtraProperties, CDB4DBaseContextPtr inContext)
{
	ObjLocker locker(inContext, fBase);
	if (locker.CanWork())
	{
		return fBase->SetExtraProperties(inExtraProperties, true, inContext);
	}
	else
		return VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT;
}


const VValueBag* VDB4DBase::RetainStructureExtraProperties(VError &err, CDB4DBaseContextPtr inContext)
{
	return fBase->RetainStructureExtraProperties(err, inContext);
}


VError VDB4DBase::SetStructureExtraProperties(VValueBag* inExtraProperties, CDB4DBaseContextPtr inContext)
{
	ObjLocker locker(inContext, fBase);
	if (locker.CanWork())
	{
		return fBase->SetStructureExtraProperties(inExtraProperties, true, inContext);
	}
	else
		return VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT;
}


VErrorDB4D VDB4DBase::DelayAllIndexes()
{
	fBase->DelayAllIndexes();
	return VE_OK;
}


VErrorDB4D VDB4DBase::AwakeAllIndexes(vector<CDB4DIndex*>& outIndexWithProblems, VDB4DProgressIndicator* inProgress, Boolean WaitForCompletion)
{
	vector<IndexInfo*> indexes;
	fBase->AwakeAllIndexes(inProgress, WaitForCompletion, indexes);
	outIndexWithProblems.clear();
	for (vector<IndexInfo*>::iterator cur = indexes.begin(), end = indexes.end(); cur != end; cur++)
	{
		IndexInfo* ind = *cur;
		CDB4DIndex* xind = new VDB4DIndex(ind);
		ind->Release();
		outIndexWithProblems.push_back(xind);
	}
	return VE_OK;
}


DB4DTriggerManager* VDB4DBase::SetTriggerManager(DB4DTriggerManager* inNewTriggerManager)
{
	return fBase->SetTriggerManager(inNewTriggerManager);
}

DB4DTriggerManager* VDB4DBase::GetTriggerManager()
{
	return fBase->GetTriggerManager();
}


VErrorDB4D VDB4DBase::CountBaseFreeBits( BaseStatistics &outStatistics ) const
{
	return fBase->CountBaseFreeBits( outStatistics );
}


void VDB4DBase::StartDataConversion()
{
	fBase->StartDataConversion();
}


void VDB4DBase::StopDataConversion()
{
	fBase->StopDataConversion();
}


void VDB4DBase::SetReferentialIntegrityEnabled(Boolean state)
{
	fBase->SetReferentialIntegrityEnabled(state);
}


void VDB4DBase::SetCheckUniquenessEnabled(Boolean state)
{
	fBase->SetCheckUniquenessEnabled(state);
}


void VDB4DBase::SetCheckNot_NullEnabled(Boolean state)
{
	fBase->SetCheckNot_NullEnabled(state);
}


CDB4DRawDataBase* VDB4DBase::OpenRawDataBase(IDB4D_DataToolsIntf* inDataToolLog, VErrorDB4D& outError)
{
	StErrorContextInstaller errs(false);
	ToolLog mylog(inDataToolLog);
	CDB4DRawDataBase* result = nil;
	outError = -1;
	if (fBase != nil)
	{
		BaseTaskInfo* context = new BaseTaskInfo(fBase, nil, nil, nil);
		context->SetTimeOutForOthersPendingLock(-1);
		if (fBase->FlushAndLock(context))
		{
			Base4D_NotOpened* xdata = new Base4D_NotOpened();
			mylog.SetCurrentBase(xdata);
			outError = xdata->AttachTo(fBase, &mylog, context);
			if (outError == VE_OK)
			{
				result = new VDB4DRawDataBase(xdata);
			}
			else
				delete xdata;
		}
		context->Release();
		
	}

	return result;
}


sLONG VDB4DBase::GetStamp(CDB4DBaseContextPtr inContext) const
{
	return fBase->GetStamp();
}


sLONG VDB4DBase::GetExtraPropertiesStamp(CDB4DBaseContextPtr inContext) const
{
	return fBase->GetStampExtraProp();
}


void VDB4DBase::GetTablesStamps(StampsVector& outStamps, CDB4DBaseContextPtr inContext) const
{
	fBase->GetTablesStamps(outStamps);
}


void VDB4DBase::GetRelationsStamps(StampsVector& outStamps, CDB4DBaseContextPtr inContext) const
{
	fBase->GetRelationsStamps(outStamps);
}


VError VDB4DBase::RetainIndexes(ArrayOf_CDB4DIndex& outIndexes, CDB4DBaseContextPtr inContext)
{
	return fBase->RetainIndexes(outIndexes);
}


CDB4DComplexQuery* VDB4DBase::NewComplexQuery()
{
	return new VDB4DComplexQuery(fManager, fBase);
}


CDB4DComplexSelection* VDB4DBase::ExecuteQuery( CDB4DComplexQuery *inQuery, CDB4DBaseContextPtr inContext, CDB4DComplexSelection* Filter, 
											VDB4DProgressIndicator* InProgress, DB4D_Way_of_Locking HowToLock, sLONG limit, VErrorDB4D *outErr)
{
	CDB4DComplexSelection* xresult = nil;
	ComplexSelection* result = nil;
	BaseTaskInfoPtr context = nil;
	if (inContext != nil)
	{
		context = ConvertContext(inContext);
		assert(context->GetBase() == fBase);
	}

	ComplexRech* query = VImpCreator<VDB4DComplexQuery>::GetImpObject(inQuery)->GetQuery();
	ComplexOptimizedQuery optimized;
	fBase->LockIndexes();
	VError err = optimized.AnalyseSearch(query, context);
	fBase->UnLockIndexes();
	if (err == VE_OK)
	{
		ComplexSelection* filtre = nil;
		if (Filter != nil)
			filtre = VImpCreator<VDB4DComplexSelection>::GetImpObject(Filter)->GetSel();
		err = optimized.PerformComplex(filtre, InProgress, context, result, HowToLock, limit);
		if (err == VE_OK && result != nil)
		{
			xresult = new VDB4DComplexSelection(fManager, result);
		}
		if (result != nil)
			result->Release();

	}

	if (outErr != nil)
		*outErr = err;
	return xresult;
}


VError VDB4DBase::CheckForUpdate()
{
	return fBase->OpenRemote(true);
}


VError VDB4DBase::IntegrateJournal(CDB4DJournalParser* inJournal, uLONG8 from, uLONG8 upto, uLONG8 *outCountIntegratedOperations, VDB4DProgressIndicator* InProgress)
{
	VError err = VE_OK;

	if (inJournal != nil)
	{
		DB4DJournalParser* jparser = VImpCreator<VDB4DJournalParser>::GetImpObject(inJournal)->GetJournalParser();
		if (jparser != nil)
		{
			err = fBase->IntegrateJournal(jparser, from, upto, outCountIntegratedOperations, InProgress);
		}
		else
			err = VE_DB4D_NULL_NOTACCEPTED;

	}
	else
		err = VE_DB4D_NULL_NOTACCEPTED;

	return err;
}


VError VDB4DBase::GetErrorOnJournalFile() const
{
	return fBase->GetErrorOnJournalFile();
}


sLONG8 VDB4DBase::GetCurrentLogOperation() const
{
	return fBase->GetCurrentLogOperation();
}

sLONG8 VDB4DBase::GetCurrentLogOperationOnDisk() const
{
	return fBase->GetCurrentLogOperationOnDisk();
}


Boolean VDB4DBase::IsWriteProtected() const
{
	return fBase->IsWriteProtected();
}


VFileDesc* VDB4DBase::GetDataIndexSeg() const
{
	return fBase->GetDataIndexSeg();
}


VFileDesc* VDB4DBase::GetStructIndexSeg() const
{
	return fBase->GetStructIndexSeg();
}


VError VDB4DBase::GetDataSegs(vector<VFileDesc*>& outSegs, Boolean inWithSpecialSegs) const
{
	return fBase->GetDataSegs(outSegs, inWithSpecialSegs);
}


VError VDB4DBase::GetStructSegs(vector<VFileDesc*>& outSegs, Boolean inWithSpecialSegs) const
{
	return fBase->GetStructSegs(outSegs, inWithSpecialSegs);
}

XBOX::VFolder* VDB4DBase::RetainBlobsFolder() const
{
	return fBase->RetainBlobsFolder();
}


VErrorDB4D VDB4DBase::SetTemporaryFolderSelector( DB4DFolderSelector inSelector, const XBOX::VString *inCustomFolder)
{
	return fBase->SetTemporaryFolderSelector( inSelector, inCustomFolder);
}


VErrorDB4D VDB4DBase::GetTemporaryFolderSelector( DB4DFolderSelector *outSelector, XBOX::VString& outCustomFolderPath) const
{
	return fBase->GetTemporaryFolderSelector( outSelector, outCustomFolderPath);
}

XBOX::VFolder* VDB4DBase::RetainTemporaryFolder( bool inMustExists) const
{
	return fBase->RetainTemporaryFolder( inMustExists);
}

VErrorDB4D VDB4DBase::PrepareForBackup(CDB4DBaseContextPtr inContext)
{
	VError err = VE_OK;
	BaseTaskInfoPtr context = nil;
	if (inContext != nil)
	{
		context = ConvertContext(inContext);
		assert(context->GetBase() == fBase);
	}
	if (context == nil)
	{
		err = ThrowBaseError(VE_DB4D_CONTEXT_IS_NULL, DBaction_StartingBackup);
	}
	else
	{
		context->SetTimeOutForOthersPendingLock(-1);
		if (fBase->FlushAndLock(context))
		{
			fBase->WriteLog(DB4D_Log_StartBackup, nil);
		}
		else
			err = fBase->ThrowError(VE_DB4D_CANNOT_LOCK_BASE, DBaction_StartingBackup);
	}
	return err;
}


VErrorDB4D VDB4DBase::EndBackup(CDB4DBaseContextPtr inContext, VFile* oldJournal)
{
	VError err = VE_OK;

	BaseTaskInfoPtr context = nil;
	if (inContext != nil)
	{
		context = ConvertContext(inContext);
		assert(context->GetBase() == fBase);
	}
	if (context == nil)
	{
		err = ThrowBaseError(VE_DB4D_CONTEXT_IS_NULL, DBaction_FinishingBackup);
	}
	else
	{
		err = fBase->EndBackup(context, oldJournal);
		fBase->UnLock(context);
	}

	return err;
}


VIntlMgr* VDB4DBase::GetIntlMgr() const
{
	return fBase->GetIntlMgr();
}


VErrorDB4D VDB4DBase::SetIntlMgr(VIntlMgr* inIntlMgr, VDB4DProgressIndicator* InProgress)
{
	return fBase->SetIntlMgr(inIntlMgr, InProgress);
}


CDB4DQueryResult* VDB4DBase::RelateOneSelection(sLONG TargetOneTable, VErrorDB4D& err, CDB4DBaseContextPtr inContext, CDB4DQueryOptions* inOptions,
											 VDB4DProgressIndicator* InProgress, std::vector<CDB4DRelation*> *inPath)
{
	CDB4DQueryResult* xresult = nil;
	if (inOptions != nil)
	{
		QueryOptions* options = VImpCreator<VDB4DQueryOptions>::GetImpObject(inOptions)->GetOptions();
		/* 
		if (fBase->IsRemote()) // deplace dans Base4D::RelateOneSelection
		{
		}
		else
		*/
		{
			CDB4DTable* tt = RetainNthTable(TargetOneTable, inContext);
			if (tt != nil)
			{
				vector<Relation*> path;
				if (inPath != nil)
				{
					for (vector<CDB4DRelation*>::iterator cur = inPath->begin(), end = inPath->end(); cur != end; cur++)
					{
						CDB4DRelation* xrel = *cur;
						path.push_back((VImpCreator<VDB4DRelation>::GetImpObject(xrel))->GetRel());
					}
				}
				xresult = new VDB4DQueryResult(tt);
				err = fBase->RelateOneSelection(options->GetFiltertable(), TargetOneTable, VImpCreator<VDB4DQueryResult>::GetImpObject(xresult)->GetResult(), ConvertContext(inContext), options, InProgress, inPath == nil ? nil : &path);
				if (err != VE_OK)
				{
					xresult->Release();
					xresult = nil;
				}
				tt->Release();
			}
		}
	}
	else
		err = ThrowBaseError(VE_DB4D_THIS_IS_NULL, noaction);

	return xresult;
}


CDB4DQueryResult* VDB4DBase::RelateManySelection(CDB4DField* inRelationStart, VErrorDB4D& err, CDB4DBaseContextPtr inContext, 
											  CDB4DQueryOptions* inOptions, VDB4DProgressIndicator* InProgress)
{
	CDB4DQueryResult* xresult = nil;
	if (inRelationStart != nil)
	{
		if (inOptions != nil)
		{
			QueryOptions* options = VImpCreator<VDB4DQueryOptions>::GetImpObject(inOptions)->GetOptions();
			/*
			if (fBase->IsRemote())  // deplace dans Base4D::RelateManySelection
			{
			}
			else
			*/
			{
				Field* cri = (VImpCreator<VDB4DField>::GetImpObject(inRelationStart))->GetField();
				CDB4DTable* tt = inRelationStart->GetOwner();
				xresult = new VDB4DQueryResult(tt);
				err = fBase->RelateManySelection(options->GetFiltertable(), cri, VImpCreator<VDB4DQueryResult>::GetImpObject(xresult)->GetResult(), ConvertContext(inContext), options, InProgress);
				if (err != VE_OK)
				{
					xresult->Release();
					xresult = nil;
				}
			}
		}
		else
			err = ThrowBaseError(VE_DB4D_THIS_IS_NULL, noaction);
	}
	else
		err = ThrowBaseError(VE_DB4D_WRONGFIELDREF, noaction);

	return xresult;
}



CDB4DRecord* VDB4DBase::BuildRecordFromServer(VStream* from, CDB4DBaseContext* inContext, CDB4DTable* inTable, VErrorDB4D& outError)
{
	CDB4DRecord* result = nil;
	BaseTaskInfo* context = nil;
	if (inContext != nil)
		context = ConvertContext(inContext);

	FicheInMem* fic = fBase->BuildRecordFromServer(from, context, (VImpCreator<VDB4DTable>::GetImpObject(inTable))->GetTable(), outError);

	if (fic != nil)
	{
		result = new VDB4DRecord(fManager, fic, inContext);
		if (result == nil)
		{
			fic->Release();
			outError = ThrowBaseError(memfull, noaction);
		}
	}

	return result;
}


CDB4DRecord* VDB4DBase::BuildRecordFromClient(VStream* from, CDB4DBaseContext* inContext, CDB4DTable* inTable, VErrorDB4D& outError)
{
	Table* owner = (VImpCreator<VDB4DTable>::GetImpObject(inTable))->GetTable();
	CDB4DRecord* result = nil;
	FicheInMem* fic = nil;
	BaseTaskInfo* context = nil;
	if (inContext != nil)
		context = ConvertContext(inContext);


	fic = fBase->BuildRecordFromClient(from, context, owner, outError);

	if (fic != nil)
	{
		result = new VDB4DRecord(fManager, fic, inContext);
		if (result == nil)
		{
			fic->Release();
			outError = ThrowBaseError(memfull, noaction);
		}
	}

	return result;
}


CDB4DSelection* VDB4DBase::BuildSelectionFromServer(VStream* from, CDB4DBaseContext* inContext, CDB4DTable* inTable, VErrorDB4D& outErr)
{
	CDB4DSelection* result = nil;
	Selection* sel = fBase->BuildSelectionFromServer(from, inContext, (VImpCreator<VDB4DTable>::GetImpObject(inTable))->GetTable(), outErr);

	if (sel != nil)
	{
		result = new VDB4DSelection(fManager, this, (VImpCreator<VDB4DTable>::GetImpObject(inTable))->GetTable(), sel);
	}

	return result;
}


CDB4DSelection* VDB4DBase::BuildSelectionFromClient(VStream* from, CDB4DBaseContext* inContext, CDB4DTable* inTable, VErrorDB4D& outErr)
{
	CDB4DSelection* result = nil;
	Selection* sel = fBase->BuildSelectionFromClient(from, inContext, (VImpCreator<VDB4DTable>::GetImpObject(inTable))->GetTable(), outErr);

	if (sel != nil)
	{
		result = new VDB4DSelection(fManager, this, (VImpCreator<VDB4DTable>::GetImpObject(inTable))->GetTable(), sel);
	}

	return result;
}


CDB4DSet* VDB4DBase::BuildSetFromServer(VStream* from, CDB4DBaseContext* inContext, CDB4DTable* inTable, VErrorDB4D& outError)
{
	CDB4DSet* result = nil;
	Bittab* sel = fBase->BuildSetFromServer(from, inContext, outError);

	if (sel != nil)
	{
		result = new VDB4DSet(this, (VImpCreator<VDB4DTable>::GetImpObject(inTable))->GetTable(), sel);
		sel->Release();
	}

	return result;
}


CDB4DSet* VDB4DBase::BuildSetFromClient(VStream* from, CDB4DBaseContext* inContext, CDB4DTable* inTable, VErrorDB4D& outError)
{
	CDB4DSet* result = nil;
	Bittab* sel = fBase->BuildSetFromClient(from, inContext, outError);

	if (sel != nil)
	{
		result = new VDB4DSet(this, (VImpCreator<VDB4DTable>::GetImpObject(inTable))->GetTable(), sel);
		sel->Release();
	}

	return result;
}


CDB4DQueryOptions* VDB4DBase::NewQueryOptions() const
{
	return new VDB4DQueryOptions;
}


void VDB4DBase::Flush(Boolean WaitUntilDone, bool inEmptyCacheMem, ECacheLogEntryKind inOrigin)
{
	VCacheLogEntryFlushFromAction		logFlush( inOrigin, WaitUntilDone, inEmptyCacheMem, true);
	
	if (inEmptyCacheMem)
	{
#if trackClose
		trackDebugMsg("call PurgeMem before Flush\n");
#endif
		VDBMgr::GetManager()->GetCacheManager()->GetMemoryManager()->PurgeMem();
	}
	else
	{
		VDBMgr::GetManager()->FlushCache(fBase, WaitUntilDone);
	}
}

void VDB4DBase::SyncThingsToForget(CDB4DBaseContext* inContext)
{
	if (fBase->IsRemote())
	{
		VTaskLock lock(&fSyncMutex);
		BaseTaskInfo* context = ConvertContext(inContext);

		if (fManager->HasSomeReleasedObjects(context))
		{
			StErrorContextInstaller errs(false);
			IRequest *req = fBase->CreateRequest( inContext, Req_SyncThingsToForget + kRangeReqDB4D);
			if (req != nil)
			{
				req->PutBaseParam(fBase);
				req->PutThingsToForget( fManager, context);
				VError err = req->GetLastError();
				if (err == VE_OK)
				{
					err = req->Send();
					if (err== VE_OK)
						err = req->GetUpdatedInfo(fBase, context);
				}
				req->Release();
			}
		}
	}
}


Boolean VDB4DBase::StillIndexing() const
{
	if (fBase->IsRemote())
		return false;
	else
	{
		return SomeIndexesPending(fBase);
	}
}


DB4D_SchemaID VDB4DBase::CreateSchema(const VString& inSchemaName, CDB4DBaseContext* inContext, VErrorDB4D& outError)
{
	DB4D_SchemaID result = -1;

	CDB4DSchema* schema = fBase->CreateSchema(inSchemaName, ConvertContext(inContext), outError);
	if (schema != nil)
	{
		result = schema->GetID();
		schema->Release();
	}

	return result;
}


sLONG VDB4DBase::CountSchemas() const
{
	return fBase->CountSchemas();
}


VErrorDB4D VDB4DBase::RetainAllSchemas(std::vector<VRefPtr<CDB4DSchema> >& outSchemas, CDB4DBaseContext* inContext)
{
	return fBase->RetainAllSchemas(outSchemas);
}


CDB4DSchema* VDB4DBase::RetainSchema(const VString& inSchemaName, CDB4DBaseContext* inContext)
{
	return fBase->RetainSchema(inSchemaName);
}


CDB4DSchema* VDB4DBase::RetainSchema(DB4D_SchemaID inSchemaID, CDB4DBaseContext* inContext)
{
	return fBase->RetainSchema(inSchemaID);
}


void VDB4DBase::SetClientID(const XBOX::VUUID& inID)
{
	fBase->SetClientID(inID);
}


VErrorDB4D VDB4DBase::ExportToSQL(CDB4DBaseContext* inContext, VFolder* inFolder, VDB4DProgressIndicator* inProgress, ExportOption& options)
{
	VError err = VE_OK;
	err = fBase->ExportToSQL(ConvertContext(inContext), inFolder, inProgress, options);
	return err;
}


VErrorDB4D VDB4DBase::ImportRecords(CDB4DBaseContext* inContext, VFolder* inFolder, VDB4DProgressIndicator* inProgress, ExportOption& options)
{
	VError err = VE_OK;
	err = fBase->ImportRecords(ConvertContext(inContext), inFolder, inProgress, options);
	return err;
}

/*
sLONG VDB4DBase::CountEntityModels(CDB4DBaseContext* context, bool onlyRealOnes) const
{
	return fBase->CountEntityModels(onlyRealOnes);
}
*/


VErrorDB4D VDB4DBase::RetainAllEntityModels(vector<XBOX::VRefPtr<CDB4DEntityModel> >& outList, CDB4DBaseContext* context, bool onlyRealOnes) const
{
	vector<CDB4DEntityModel*> ems;
	VError err = fBase->RetainAllEntityModels(ems, context, false, onlyRealOnes);
	if (err == VE_OK)
	{
		for (vector<CDB4DEntityModel*>::iterator cur = ems.begin(), end = ems.end(); cur != end; cur++)
		{
			CDB4DEntityModel* em = *cur;
			XBOX::VRefPtr<CDB4DEntityModel> p;
			p.Adopt(em);
			outList.push_back(p);
		}
	}
	return err;
}


CDB4DEntityModel* VDB4DBase::RetainEntityModel(const XBOX::VString& entityName, bool onlyRealOnes) const
{
	return fBase->RetainEntity(entityName, onlyRealOnes);
}


CDB4DEntityModel* VDB4DBase::RetainEntityModelByCollectionName(const XBOX::VString& entityName) const
{
	return fBase->RetainEntityModelByCollectionName(entityName);
}


VErrorDB4D VDB4DBase::SetIdleTimeOut(uLONG inMilliseconds)
{
	VError err = VE_OK;
	DB4DNetManager* net = dynamic_cast<DB4DNetManager*>(fBase->GetNetAccess());
	if (net != nil)
	{
		err = net->SetIdleTimeOut(inMilliseconds);
	}

	return err;
}


void VDB4DBase::DisposeRecoverInfo(void* inRecoverInfo)
{
	fBase->DisposeRecoverInfo(inRecoverInfo);
}

VError VDB4DBase::ScanToRecover(VFile* inOldDataFile, VValueBag& outBag, void* &outRecoverInfo, IDB4D_DataToolsIntf* inDataToolLog)
{
	ToolLog mylog(inDataToolLog);
	return fBase->ScanToRecover(inOldDataFile, outBag, outRecoverInfo, &mylog);
}


VError VDB4DBase::RecoverByTags(VValueBag& inBag, void* inRecoverInfo, IDB4D_DataToolsIntf* inDataToolLog)
{
	ToolLog mylog(inDataToolLog);
	return fBase->RecoverByTags(inBag, inRecoverInfo, &mylog);
}


CDB4DBase* VDB4DBase::RetainSyncDataBase()
{
	VError err = VE_OK;
	Base4D* syncdb = nil;
	SynchroBaseHelper* synchelp = fBase->GetSyncHelper(false);
	if (synchelp != nil)
		syncdb = synchelp->GetBase(err, false);
	if (syncdb == nil)
		return nil;
	else
	{
		return new VDB4DBase(fManager, syncdb, true);
	}
}


VError VDB4DBase::SaveToBag(VValueBag& outBag)
{
	VError err = VE_OK;
	EntityModelCatalog* catalog = fBase->GetGoodEntityCatalog(true);
	if (catalog != nil)
	{
		err = catalog->SaveEntityModels(outBag, true, true);
	}
	return err;
}



IHTTPRequestHandler *VDB4DBase::AddRestRequestHandler( VErrorDB4D& outError, IHTTPServerProject* inHTTPServerProject, RIApplicationRef inApplicationRef, const VString& inPattern, bool inEnabled)
{
	outError = VE_OK;

	RestRequestHandler *resthandler = NULL;
	if (inHTTPServerProject != NULL)
	{
		VString pattern = (!inPattern.IsEmpty()) ? inPattern : L"(?i)/rest/.*";
		resthandler = new RestRequestHandler( this, pattern, inEnabled, inApplicationRef);
		if (resthandler != NULL)
		{
			outError = inHTTPServerProject->AddHTTPRequestHandler (resthandler);
			if (outError != VE_OK)
				ReleaseRefCountable( &resthandler);
		}
		else
		{
			outError = VE_MEMORY_FULL;
		}

	}
	return resthandler;
}


VErrorDB4D VDB4DBase::SetRestRequestHandlerPattern( IHTTPRequestHandler* inHandler, const VString& inPattern)
{
	VError err = VE_OK;
	if (!inPattern.IsEmpty())
	{
		RestRequestHandler *resthandler = dynamic_cast<RestRequestHandler*>(inHandler);
		if (resthandler != NULL)
		{
			err = resthandler->SetPattern( inPattern);
		}
		else
		{
			err = VE_DB4D_INVALID_REST_REQUEST_HANDLER;
		}
	}
	return err;
}


VError VDB4DBase::ReLoadEntityModels(VFile* inFile)
{
	return fBase->ReLoadEntityModels(inFile);
}


VError VDB4DBase::GetListOfDeadTables(vector<VString>& outTableNames, vector<VUUID>& outTableIDs, CDB4DBaseContext* inContext)
{
	return fBase->GetListOfDeadTables(outTableNames, outTableIDs, inContext);
}


VError VDB4DBase::ResurectDataTable(const VString& inTableName, CDB4DBaseContext* inContext)
{
	return fBase->ResurectDataTable(inTableName, inContext);
}


VFolder* VDB4DBase::RetainDataFolder()
{
	return fBase->RetainDataFolder();
}


VFolder* VDB4DBase::RetainStructFolder()
{
	return RetainRefCountable(fBase->GetStructFolder());
}


VErrorDB4D VDB4DBase::CompactInto(VFile* destData, IDB4D_DataToolsIntf* inDataToolLog, bool KeepRecordNumbers)
{
	return fBase->CompactInto(destData, inDataToolLog, KeepRecordNumbers);
}


VJSObject VDB4DBase::CreateJSDatabaseObject( const VJSContext& inContext)
{
	return VJSDatabase::CreateInstance(inContext, fBase);
}


bool VDB4DBase::CatalogJSParsingError(VFile* &outRetainedFile, VString& outMessage, sLONG& outLineNumber)
{
	EntityModelCatalog* cat = fBase->GetEntityCatalog(true);
	if (cat != nil)
	{
		return cat->ParsingJSError(outRetainedFile, outMessage, outLineNumber);
	}
	else
		return false;
}



//=================================================================================



const VString& VDB4DSchema::GetName() const
{
	return fName;
}


DB4D_SchemaID VDB4DSchema::GetID() const
{
	return fID;
}


VValueBag* VDB4DSchema::RetainAndLockProperties(VErrorDB4D& outError, CDB4DBaseContext* inContext)
{
	return nil;
}


void VDB4DSchema::SaveProperties(CDB4DBaseContext* inContext)
{
}


void VDB4DSchema::UnlockProperties(CDB4DBaseContext* inContext)
{
}


VErrorDB4D VDB4DSchema::Drop(CDB4DBaseContext* inContext)
{
	return fOwner->DropSchema(this, ConvertContext(inContext));
}


VErrorDB4D VDB4DSchema::Rename(const VString& inNewName, CDB4DBaseContext* inContext)
{
	return fOwner->RenameSchema(this, inNewName, ConvertContext(inContext));
}


VError VDB4DSchema::LoadFromBagWithLoader( const VValueBag& inBag, VBagLoader *inLoader, void* inContext)
{
	VError err = VE_OK;

	VString s;
	inBag.GetString( DB4DBagKeys::name, s);
	if ( fOwner )
	{
		CDB4DSchema *schema = fOwner->RetainSchema( s);
		if (schema != NULL)
			err = VE_DB4D_DUPLICATED_SCHEMA_NAME;
		ReleaseRefCountable( &schema);
	}

	if ( err == VE_OK )
	{
		fName = s;
		fID = 1;
	}

	return err;
}

VError VDB4DSchema::SaveToBag( VValueBag& ioBag, VString& outKind) const
{
	VError err = VE_OK;

	VString name = GetName ( );
	DB4DBagKeys::name.Set( &ioBag, name);

	outKind = L"schema";

	return err;
}




//=================================================================================



/*

VDB4DBaseContext::VDB4DBaseContext( VDBMgr *inManager, CDB4DBase *inBase, bool islocal):fBase( VImpCreator<VDB4DBase>::GetImpObject(inBase)->GetBase(), this, islocal)
{
	fManager = inManager;
	fOwner = inBase;
	fOwner->Retain("VDB4DBaseContext");
	fIsRemote = inBase->IsRemote();
}




VDB4DBaseContext::~VDB4DBaseContext()
{
	fOwner->Release("VDB4DBaseContext");
}


CDB4DBase* VDB4DBaseContext::GetOwner(void) const
{
	return (fOwner);
}


CDB4DBaseContextPtr VDB4DBaseContext::Spawn(Boolean isCopied) const
{
	CDB4DBaseContextPtr xbd;
	if (isCopied)
	{
		// L.R : a faire, il faut dupliquer les selections et autres variables du context
	}
	xbd = new VDB4DBaseContext(fManager, fOwner);
	return xbd;
}


Boolean VDB4DBaseContext::MatchBaseInContext(CDB4DBaseContextPtr InBaseContext) const
{
	if (fBase.GetBase() == ((VDB4DBaseContext*)InBaseContext)->fBase.GetBase()) return true;
	else return false;
}


void VDB4DBaseContext::SetTimerOnRecordLocking(sLONG WaitForLockTimer)
{
	fBase.SetLockTimer(WaitForLockTimer);
}


sLONG VDB4DBaseContext::GetTimerOnRecordLocking() const
{
	return fBase.GetLockTimer();
}


VError VDB4DBaseContext::StartTransaction(sLONG WaitForLockTimer)
{
	VError err;
	fBase.StartTransaction(err, WaitForLockTimer);
	return err;
}


VError VDB4DBaseContext::CommitTransaction()
{		
	VError err = fBase.CommitTransaction();
	fOwner->SyncThingsToForget(this);
	return err;
}


VError VDB4DBaseContext::RollBackTransaction()
{
	fBase.RollBackTransaction();
	fOwner->SyncThingsToForget(this);
	return VE_OK;
}


sLONG VDB4DBaseContext::CurrentTransactionLevel() const
{
	return fBase.CurrentTransactionLevel();
}


VError VDB4DBaseContext::ReleaseFromConsistencySet(CDB4DSelection* InSel)
{

	return VE_OK;
}


VError VDB4DBaseContext::ReleaseFromConsistencySet(CDB4DSet* InSet)
{

	return VE_OK;
}


VError VDB4DBaseContext::ReleaseFromConsistencySet(RecIDType inRecordID)
{

	return VE_OK;
}


VError VDB4DBaseContext::ReleaseFromConsistencySet(CDB4DRecord* inRec)
{

	return VE_OK;
}


VError VDB4DBaseContext::SetConsistency(Boolean isOn)
{

	return VE_OK;
}


CDB4DSqlQuery* VDB4DBaseContext::NewSqlQuery(VString& request, VError& err)
{
	CDB4DSqlQuery* sq = new VDB4DSqlQuery(fManager, fBase.GetBase(), this, request, err);
	return sq;
}


VError VDB4DBaseContext::SetRelationAutoLoadNto1(const CDB4DRelation* inRel, DB4D_Rel_AutoLoadState inAutoLoadState)
{
	if (inRel == nil)
		return VE_DB4D_WRONG_RELATIONREF;
	else
	{
		const Relation* rel = VImpCreator<VDB4DRelation>::GetImpObject(inRel)->GetRel();
		return fBase.SetRelationAutoLoadNto1(rel, inAutoLoadState);
	}
}


VError VDB4DBaseContext::SetRelationAutoLoad1toN(const CDB4DRelation* inRel, DB4D_Rel_AutoLoadState inAutoLoadState)
{
	if (inRel == nil)
		return VE_DB4D_WRONG_RELATIONREF;
	else
	{
		const Relation* rel = VImpCreator<VDB4DRelation>::GetImpObject(inRel)->GetRel();
		return fBase.SetRelationAutoLoad1toN(rel, inAutoLoadState);
	}
}

Boolean VDB4DBaseContext::IsRelationAutoLoadNto1(const CDB4DRelation* inRel) const
{
	Boolean res = false;
	if (inRel != nil)
	{
		Boolean unknown;
		const Relation* rel = VImpCreator<VDB4DRelation>::GetImpObject(inRel)->GetRel();
		res = fBase.IsRelationAutoLoadNto1(rel, unknown);
	}

	return res;
}


Boolean VDB4DBaseContext::IsRelationAutoLoad1toN(const CDB4DRelation* inRel) const
{
	Boolean res = false;
	if (inRel != nil)
	{
		Boolean unknown;
		const Relation* rel = VImpCreator<VDB4DRelation>::GetImpObject(inRel)->GetRel();
		res = fBase.IsRelationAutoLoad1toN(rel, unknown);
	}

	return res;
}


DB4D_Rel_AutoLoadState VDB4DBaseContext::GetRelationAutoLoadNto1State(const CDB4DRelation* inRel) const
{
	DB4D_Rel_AutoLoadState res = DB4D_Rel_AutoLoad_SameAsStructure;

	if (inRel != nil)
	{
		Boolean unknown;
		const Relation* rel = VImpCreator<VDB4DRelation>::GetImpObject(inRel)->GetRel();
		Boolean res2 = fBase.IsRelationAutoLoadNto1(rel, unknown);
		if (!unknown)
			if (res2)
				res = DB4D_Rel_AutoLoad;
			else
				res = DB4D_Rel_Not_AutoLoad;
	}

	return res;
}


DB4D_Rel_AutoLoadState VDB4DBaseContext::GetRelationAutoLoad1toNState(const CDB4DRelation* inRel) const
{
	DB4D_Rel_AutoLoadState res = DB4D_Rel_AutoLoad_SameAsStructure;

	if (inRel != nil)
	{
		Boolean unknown;
		const Relation* rel = VImpCreator<VDB4DRelation>::GetImpObject(inRel)->GetRel();
		Boolean res2 = fBase.IsRelationAutoLoad1toN(rel, unknown);
		if (!unknown)
			if (res2)
				res = DB4D_Rel_AutoLoad;
			else
				res = DB4D_Rel_Not_AutoLoad;
	}

	return res;
}

void VDB4DBaseContext::ExcludeTableFromAutoRelationDestination(CDB4DTable* inTableToExclude)
{
	fBase.ExcludeTableFromAutoRelationDestination(VImpCreator<VDB4DTable>::GetImpObject(inTableToExclude)->GetTable());
}


void VDB4DBaseContext::IncludeBackTableToAutoRelationDestination(CDB4DTable* inTableToInclude)
{
	fBase.IncludeBackTableToAutoRelationDestination(VImpCreator<VDB4DTable>::GetImpObject(inTableToInclude)->GetTable());
}


Boolean VDB4DBaseContext::IsTableExcludedFromAutoRelationDestination(CDB4DTable* inTableToCheck) const
{
	return fBase.IsTableExcludedFromAutoRelationDestination(VImpCreator<VDB4DTable>::GetImpObject(inTableToCheck)->GetTable());
}


void VDB4DBaseContext::SetAllRelationsToAutomatic(Boolean RelationsNto1, Boolean ForceAuto)
{
	fBase.SetAllRelationsToAutomatic(RelationsNto1, ForceAuto);
}


Boolean VDB4DBaseContext::IsAllRelationsToAutomatic(Boolean RelationsNto1)
{
	return fBase.IsAllRelationsToAutomatic(RelationsNto1);
}


void VDB4DBaseContext::SetExtraData( const VValueBag* inExtra)
{
	fBase.SetExtraData(inExtra);
}


const VValueBag* VDB4DBaseContext::RetainExtraData() const
{
	return fBase.RetainExtraData();
}


LockPtr VDB4DBaseContext::LockDataBaseDef(const CDB4DBase* inBase, sLONG inTimeOut, Boolean inForReadOnly, const VValueBag **outLockerExtraData)
{
	if (testAssert(inBase != nil))
		return (LockPtr)fBase.LockDataBaseDef(VImpCreator<VDB4DBase>::GetImpObject(inBase)->GetBase(), inTimeOut, inForReadOnly, outLockerExtraData);
	else
		return nil;
}


LockPtr VDB4DBaseContext::LockTableDef(const CDB4DTable* inTable, Boolean inWithFields, sLONG inTimeOut, Boolean inForReadOnly, const VValueBag **outLockerExtraData)
{
	if (testAssert(inTable != nil))
		return (LockPtr)fBase.LockTableDef(VImpCreator<VDB4DTable>::GetImpObject(inTable)->GetTable(), inWithFields, inTimeOut, inForReadOnly, outLockerExtraData);
	else
		return nil;
}


LockPtr VDB4DBaseContext::LockFieldDef(const CDB4DField* inField, sLONG inTimeOut, Boolean inForReadOnly, const VValueBag **outLockerExtraData)
{
	if (testAssert(inField != nil))
		return (LockPtr)fBase.LockFieldDef(VImpCreator<VDB4DField>::GetImpObject(inField)->GetField(), inTimeOut, inForReadOnly, outLockerExtraData);
	else
		return nil;
}


LockPtr VDB4DBaseContext::LockRelationDef(const CDB4DRelation* inRelation, Boolean inWithRelatedFields, sLONG inTimeOut, Boolean inForReadOnly, const VValueBag **outLockerExtraData)
{
	if (testAssert(inRelation != nil))
		return (LockPtr)fBase.LockRelationDef(VImpCreator<VDB4DRelation>::GetImpObject(inRelation)->GetRel(), inWithRelatedFields, inTimeOut, inForReadOnly, outLockerExtraData);
	else
		return nil;
}


LockPtr VDB4DBaseContext::LockIndexDef(const CDB4DIndex* inIndex, sLONG inTimeOut, Boolean inForReadOnly, const VValueBag **outLockerExtraData)
{
	if (testAssert(inIndex != nil))
		return (LockPtr)fBase.LockIndexDef(VImpCreator<VDB4DIndex>::GetImpObject(inIndex)->GetInd(), inTimeOut, inForReadOnly, outLockerExtraData);
	else
		return nil;
}


Boolean VDB4DBaseContext::LockMultipleTables(const vector<const CDB4DTable*>& inTables, vector<LockPtr>& outLocks, sLONG inTimeOut, Boolean inForReadOnly)
{
	Boolean result = true;
	uLONG startmillisec = VSystem::GetCurrentTime();

	try
	{
		outLocks.resize(inTables.size(), nil);
	}
	catch (...)
	{
		result = false;
	}
	vector<LockPtr>::iterator curlock = outLocks.begin();
	for (vector<const CDB4DTable*>::const_iterator cur = inTables.begin(), end = inTables.end(); cur != end && result; cur++, curlock++)
	{
		LockPtr xlock = LockTableDef(*cur, true, inTimeOut, inForReadOnly);
		if (xlock != nil)
		{
			*curlock = xlock;
			if (inTimeOut != -1 && inTimeOut != 0)
			{
				uLONG currentime = VSystem::GetCurrentTime();
				sLONG passedtime = currentime - startmillisec;
				if (passedtime < inTimeOut)
					inTimeOut = inTimeOut - passedtime;
				else
					inTimeOut = 0;

			}
		}
		else
			result = false;
	}

	if (!result)
		UnLockMultipleTables(outLocks);

	return result;
}


void VDB4DBaseContext::UnLockMultipleTables(const vector<LockPtr>& inLocks)
{
	for (vector<LockPtr>::const_iterator cur = inLocks.begin(), end = inLocks.end(); cur != end; cur++)
	{
		if (*cur != nil)
			UnLockStructObject(*cur);
	}
}


VErrorDB4D VDB4DBaseContext::UnLockStructObject(LockPtr inLocker)
{
	return fBase.UnLockStructObject((StructObjLockPtr)inLocker);
}


void VDB4DBaseContext::SetClientRequestStreams(VStream* OutputStream, VStream* InputStream)
{
	//## a faire
}

void VDB4DBaseContext::SetServerRequestStreams(VStream* InputStream, VStream* OutputStream)
{
	//## a faire
}


void VDB4DBaseContext::SendlastRemoteInfo()
{
	fBase.SendlastRemoteInfo();
}


CDB4DContext* VDB4DBaseContext::GetContextOwner() const
{
	return fBase.GetContextOwner();
}


void VDB4DBaseContext::DescribeQueryExecution(Boolean on)
{
	fBase.MustDescribeQuery(on);
}


void VDB4DBaseContext::GetLastQueryDescription(XBOX::VString& outResult, DB4D_QueryDescriptionFormat inFormat)
{
	fBase.GetLastQueryDescription(outResult);
}


void VDB4DBaseContext::GetLastQueryExecution(XBOX::VString& outResult, DB4D_QueryDescriptionFormat inFormat)
{
	if (inFormat == DB4D_QueryDescription_XML)
	{
		fBase.GetLastQueryExecutionXML(outResult);
	}
	else
	{
		fBase.GetLastQueryExecution(outResult);
	}
}



CDB4DRemoteRecordCache* VDB4DBaseContext::StartCachingRemoteRecords(CDB4DSelection* inSel, RecIDType FromRecIndex, RecIDType ToRecIndex,  CDB4DFieldCacheCollection* inWhichFields, const vector<uBYTE>& inWayOfLocking)
{
	VDB4DRemoteRecordCache* result = nil;
	StErrorContextInstaller errors(false);
	VError err = VE_OK;

	if (testAssert(inSel != nil))
	{
		result = new VDB4DRemoteRecordCache(inWhichFields, this);

		if (result != nil)
		{
			RemoteRecordCache* remoterecords = result->GetRemoteRecordCache();

			err = remoterecords->StartCachingRemoteRecords(VImpCreator<VDB4DSelection>::GetImpObject(inSel)->GetSel(), FromRecIndex, ToRecIndex, inWayOfLocking);
		}

		if (err != VE_OK)
			ReleaseRefCountable(&result);
	}

	return result;
}


void VDB4DBaseContext::StopCachingRemoteRecords(CDB4DRemoteRecordCache* inCacheInfo)
{
	// pourra etre utile, un jour
}


VErrorDB4D VDB4DBaseContext::SetIdleTimeOut(uLONG inMilliseconds)
{
	VError err = VE_OK;
	DB4DNetManager* net = fBase.GetRemoteConnection();
	if (net != nil)
	{
		err = net->SetContextIdleTimeOut(this, inMilliseconds);
	}

	return err;
}



#if debuglr
CComponent* VDB4DBaseContext::Retain(const char* DebugInfo)
{
	return VComponentImp<CDB4DBaseContext>::Retain(DebugInfo);
}

void VDB4DBaseContext::Release(const char* DebugInfo)
{
	VComponentImp<CDB4DBaseContext>::Release(DebugInfo);
}
#endif


*/

//=================================================================================



VDB4DSet::~VDB4DSet()
{
	ReleaseRefCountable(&fSet);
	if (fTable != nil)
		fTable->Release();
	fBase->Release("VDB4DSet");
}


VDB4DSet::VDB4DSet( VDB4DBase *inBase, Table *inTable, Bittab* inSet)
{
	fBase = inBase;
	fTable = inTable;
	fSet = inSet;
	if (fSet != nil)
		fSet->Retain();
	
	fBase->Retain("VDB4DSet");
	if (fTable != nil)
	{
		fTable->Retain();
		if (fSet != nil)
			fSet->SetOwner(fTable->GetOwner());
	}

}


DB4D_TableID VDB4DSet::GetTableRef() const
{
	if (fTable == nil)
		return (DB4D_TableID)0;
	else
		return (DB4D_TableID)(fTable->GetNum());
}


CDB4DBase* VDB4DSet::GetBaseRef() const
{
	return fBase;
}


RecIDType VDB4DSet::CountRecordsInSet() const
{
	if (fSet == nil) return 0;
	else return fSet->Compte();
}


RecIDType VDB4DSet::GetMaxRecordsInSet() const
{
	if (fSet == nil) return 0;
	else return fSet->NbTotalBits();
}


VError VDB4DSet::SetMaxRecordsInSet(RecIDType inMax) const
{
	VError err = VE_OK;
	if (fSet != nil)
	{
		err = fSet->aggrandit(inMax);
	}
	return err;
}


Boolean VDB4DSet::IsOn(RecIDType inPos) const
{
	if (fSet == nil) return false;
	else
	{
		if (inPos>=0)
			return fSet->isOn(inPos);
		else
			return false;
	}
}


VError VDB4DSet::ClearOrSet(RecIDType inPos, Boolean inValue)
{
	VError err = VE_OK;

	if (fSet != nil)
	{
		sLONG n = (sLONG)inPos;
		//if (n>=0)
		{
			/*
			if (n>=fSet->NbTotalBits())
				err = fSet->aggrandit(n+1);
			if (err == VE_OK)
			*/
			{
				err = fSet->ClearOrSet(inPos, (uBOOL)inValue, true);
			}
		}
	}

	return err;
}


VError VDB4DSet::ClearOrSetAll(Boolean inValue)
{
	VError err = VE_OK;

	if (fSet != nil)
	{
		fSet->ClearOrSetAll((uBOOL)inValue);
	}

	return err;
}


RecIDType VDB4DSet::FindNextOne(RecIDType inFirstToLook) const
{
	sLONG n = -1;

	if (fSet != nil)
	{
		n = fSet->FindNextBit(inFirstToLook);
	}

	return n;
}


RecIDType VDB4DSet::FindPreviousOne(RecIDType inFirstToLook) const
{
	sLONG n = -1;

	if (fSet != nil)
	{
		n = fSet->FindPreviousBit(inFirstToLook);
	}

	return n;
}


VError VDB4DSet::And(const CDB4DSet& other)
{
	VError err = VE_OK;
	VDB4DSet *xother = const_cast<VDB4DSet*> (VImpCreator<VDB4DSet>::GetImpObject(&other));

	if (fSet != nil && xother->fSet != nil)
	{
		err = fSet->And(xother->fSet);
	}

	return err;
}


VError VDB4DSet::Or(const CDB4DSet& other)
{
	VError err = VE_OK;
	VDB4DSet *xother = const_cast<VDB4DSet*> (VImpCreator<VDB4DSet>::GetImpObject(&other));

	if (fSet != nil && xother->fSet != nil)
	{
		err = fSet->Or(xother->fSet);
	}

	return err;
}


VError VDB4DSet::Minus(const CDB4DSet& other)
{
	VError err = VE_OK;
	VDB4DSet *xother = const_cast<VDB4DSet*> (VImpCreator<VDB4DSet>::GetImpObject(&other));

	if (fSet != nil && xother->fSet != nil)
	{
		err = fSet->moins(xother->fSet);
	}

	return err;
}


VError VDB4DSet::Invert()
{
	VError err = VE_OK;

	if (fSet != nil)
	{
		err = fSet->Invert();
	}

	return err;
}

	
VError VDB4DSet::CloneFrom(const CDB4DSet& other)
{
	VError err = VE_OK;

	VDB4DSet *xother = const_cast<VDB4DSet*> (VImpCreator<VDB4DSet>::GetImpObject(&other));

	if (fSet != nil)
	{
		fSet->raz();
		if (xother->fSet != nil)
		{
			err = fSet->Or(xother->fSet);
		}

	}

	return err;
}


CDB4DSet* VDB4DSet::Clone(VError* outErr) const
{
	VError err = VE_OK;
	Bittab* newset;
	VDB4DSet* xnewset = nil;

	if (fSet == nil)
		newset = nil;
	else
	{
		newset = fSet->Clone(err);
	}
	if (err == VE_OK)
	{
		xnewset = new VDB4DSet(fBase, fTable, newset);
		if (xnewset == nil)
		{
			err = ThrowBaseError(memfull, DBaction_BuildingSet);
		}
	}

	ReleaseRefCountable(&newset);

	if (outErr != nil)
		*outErr = err;
	return xnewset;
}



VError VDB4DSet::Compact()
{
	VError err = VE_OK;

	if (fSet != nil)
	{
		fSet->Epure();
	}

	return err;
}


CDB4DSelection* VDB4DSet::ConvertToSelection(VError& err, CDB4DBaseContext* inContext) const
{
	VDB4DSelection* res = nil;
	err = VE_OK;

	if (fSet!=nil)
	{
		if (fTable->GetDF() != nil)
			fTable->GetDF()->TakeOffBits(fSet, ConvertContext(inContext));
		Selection* sel = CreFromBittab(fTable->GetDF(), fSet, fTable->GetOwner());
		if (sel != nil)
		{
			res = new VDB4DSelection(fBase->GetManager(), fBase, fTable, sel);
			if (res == nil) 
			{
				sel->Release();
				err = ThrowBaseError(memfull, DBaction_BuildingSelection);
			}
		}
	}

	return res;
}


VError VDB4DSet::FillArrayOfLongs(sLONG* outArray, sLONG inMaxElements)
{
	VError err = VE_OK;

	if (fSet != nil)
	{
		if (fSet->IsRemote())
		{
			IRequest *req = fTable->GetOwner()->CreateRequest( nil, Req_FillArrayOfLongsFromSet + kRangeReqDB4D);
			if (req == nil)
			{
				err = ThrowBaseError(memfull, noaction);
			}
			else
			{
				req->PutBaseParam(fTable->GetOwner());
				req->PutThingsToForget( VDBMgr::GetManager(), nil);
				req->PutTableParam(fTable);
				req->PutSetParam(fSet, nil);
				req->PutLongParam(inMaxElements);

				err = req->GetLastError();
				if (err == VE_OK)
				{
					err = req->Send();
					if (err== VE_OK)
						err = req->GetUpdatedInfo(fTable->GetOwner(), nil);
					if (err == VE_OK)
					{
						sLONG nb = req->GetLongReply(err);
						if (err == VE_OK)
						{
							sLONG nblong = nb;
							err = req->GetInputStream()->GetLongs(outArray, &nblong);
						}
					}
				}
				req->Release();
			}

		}
		else
			err = fSet->FillArray(outArray, inMaxElements);
	}

	return err;
}


VError VDB4DSet::FillFromArrayOfLongs(const sLONG* inArray, sLONG inNbElements)
{
	VError err = VE_OK;

	if (fSet != nil)
	{
		if (fSet->IsRemote())
		{
			IRequest *req = fTable->GetOwner()->CreateRequest( nil, Req_FillSetFromArrayOfLongs + kRangeReqDB4D);
			if (req == nil)
			{
				err = ThrowBaseError(memfull, noaction);
			}
			else
			{
				req->PutBaseParam(fTable->GetOwner());
				req->PutThingsToForget( VDBMgr::GetManager(), nil);
				req->PutTableParam(fTable);
				req->PutSetParam(fSet, nil);
				req->PutLongParam(inNbElements);
				if (inNbElements > 0)
					req->GetOutputStream()->PutLongs(inArray, inNbElements);

				err = req->GetLastError();
				if (err == VE_OK)
				{
					err = req->Send();
					if (err == VE_OK)
					{
						sLONG nb = req->GetLongReply(err);
						if (err == VE_OK)
						{
							fSet->ClearRemoteCache(req->GetInputStream());
							fSet->SetCompte(nb);
						}
					}
				}
				req->Release();
			}

		}
		else
		{
			sLONG nbrec = fTable->IsRemote() ? /*fTable->GetRemoteMaxRecordsInTable()*/ 1073741825 : fTable->GetDF()->GetMaxRecords(nil);
			err = fSet->FillFromArray((const char*)inArray, sizeof(sLONG), inNbElements, nbrec, false);
			if (err == VE_OK && !fTable->IsRemote())
			{
				err = fSet->Reduit(nbrec);
			}
		}
	}

	return err;
}


VError VDB4DSet::FillArrayOfBits(void* outArray, sLONG inMaxElements)
{
	VError err = VE_OK;

	if (fSet != nil)
	{
		if (fSet->IsRemote())
		{
			IRequest *req = fTable->GetOwner()->CreateRequest( nil, Req_FillArrayOfBitsFromSet + kRangeReqDB4D);
			if (req == nil)
			{
				err = ThrowBaseError(memfull, noaction);
			}
			else
			{
				req->PutBaseParam(fTable->GetOwner());
				req->PutThingsToForget( VDBMgr::GetManager(), nil);
				req->PutTableParam(fTable);
				req->PutSetParam(fSet, nil);
				req->PutLongParam(inMaxElements);

				err = req->GetLastError();
				if (err == VE_OK)
				{
					err = req->Send();
					if (err== VE_OK)
						err = req->GetUpdatedInfo(fTable->GetOwner(), nil);
					if (err == VE_OK)
					{
						sLONG nb = req->GetLongReply(err);
						if (err == VE_OK)
						{
							sLONG nblong = (nb+31)/32;
							err = req->GetInputStream()->GetLongs((sLONG*)outArray, &nblong);
						}
					}
				}
				req->Release();
			}

		}
		else
			err = fSet->FillArrayOfBits(outArray, inMaxElements);
	}

	return err;
}


VError VDB4DSet::FillFromArrayOfBits(const void* inArray, sLONG inNbElements)
{
	VError err = VE_OK;

	if (fSet != nil)
	{
		if (fSet->IsRemote())
		{
			IRequest *req = fTable->GetOwner()->CreateRequest( nil, Req_FillSetFromArrayOfBits + kRangeReqDB4D);
			if (req == nil)
			{
				err = ThrowBaseError(memfull, noaction);
			}
			else
			{
				req->PutBaseParam(fTable->GetOwner());
				req->PutThingsToForget( VDBMgr::GetManager(), nil);
				req->PutTableParam(fTable);
				req->PutSetParam(fSet, nil);
				req->PutLongParam(inNbElements);
				if (inNbElements > 0)
					req->GetOutputStream()->PutLongs((const sLONG*)inArray, (inNbElements+31)/32);

				err = req->GetLastError();
				if (err == VE_OK)
				{
					err = req->Send();
					if (err == VE_OK)
					{
						sLONG nb = req->GetLongReply(err);
						if (err == VE_OK)
						{
							fSet->ClearRemoteCache(req->GetInputStream());
							fSet->SetCompte(nb);
						}
					}
				}
				req->Release();
			}

		}
		else
		{
			err = fSet->FillFromArrayOfBits(inArray, inNbElements);
			if (err == VE_OK && !fTable->IsRemote())
			{
				sLONG nbrec = fTable->IsRemote() ? fTable->GetRemoteMaxRecordsInTable() : fTable->GetDF()->GetMaxRecords(nil);
				err = fSet->Reduit(nbrec);
			}
		}
	}

	return err;
}


VError VDB4DSet::WriteToStream(VStream& outStream)
{
	if (fSet == nil)
		return VE_DB4D_SET_IS_NULL;
	else
		return fSet->PutInto(outStream);
}


VError VDB4DSet::ReadFromStream(VStream& inStream)
{
	if (fSet == nil)
		return VE_DB4D_SET_IS_NULL;
	else
		return fSet->GetFrom(inStream);
}


VErrorDB4D VDB4DSet::ToClient(VStream* into, CDB4DBaseContext* inContext)
{
	VError err;

	if (fSet == nil)
		err = VE_DB4D_SELECTION_IS_NULL;
	else
	{
		err = into->PutLong(sel_bitsel);
		err = fSet->ToClient(into, inContext);
	}

	return err;
}


VErrorDB4D VDB4DSet::ToServer(VStream* into, CDB4DBaseContext* inContext, bool inKeepOnServer)
{
	VError err;

	if (fSet == nil)
		err = VE_DB4D_SELECTION_IS_NULL;
	else
	{
		err = fSet->ToServer(into, inContext, inKeepOnServer);
	}

	return err;
}


void VDB4DSet::MarkOnServerAsPermanent()
{
	if (fSet != nil)
		VDBMgr::GetManager()->MarkSetOnServerAsPermanent(fSet);
}


void VDB4DSet::UnMarkOnServerAsPermanent(bool willResend)
{
	if (fSet != nil)
		VDBMgr::GetManager()->UnMarkSetOnServerAsPermanent(fSet, willResend);
}




//=================================================================================





VDB4DSelection::~VDB4DSelection()
{
	if (fSel != nil) 
		fSel->Release();
	fTable->Release();
	if (fBase != nil)
		fBase->Release("VDB4DSelection");
}


VDB4DSelection::VDB4DSelection( VDBMgr *inManager, VDB4DBase *inBase, Table *inTable, Selection* inSel, CDB4DEntityModel* inModel)
{
	fManager = inManager;
	fBase = inBase;
	fTable = inTable;
	fSel = inSel;
	
	if (fBase != nil)
		fBase->Retain("VDB4DSelection");
	fTable->Retain();
	fModel = inModel;
}


DB4D_TableID VDB4DSelection::GetTableRef() const
{
	return (DB4D_TableID)(fTable->GetNum());
}


CDB4DBase* VDB4DSelection::GetBaseRef() const
{
	return fBase;
}


RecIDType VDB4DSelection::CountRecordsInSelection(CDB4DBaseContextPtr inContext) const
{
	if (fSel == nil) return 0;
	else return fSel->GetQTfic();
}


Boolean VDB4DSelection::IsEmpty(CDB4DBaseContextPtr inContext) const
{
	if (fSel == nil) 
		return true;
	else 
		return fSel->IsEmpty();
}


CDB4DRecord *VDB4DSelection::LoadSelectedRecord( RecIDType inRecordIndex, DB4D_Way_of_Locking HowToLock, CDB4DBaseContextPtr inContext, Boolean WithSubTable, Boolean* outLockWasKeptInTrans)
{
	if (fSel->IsRemoteLike())
	{
		ThrowBaseError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
		return nil;
	}
	else
	{
		BaseTaskInfoPtr context = nil;
		if (inContext != nil)
		{
			context = ConvertContext(inContext);
			assert(context->GetBase() == fSel->GetParentFile()->GetDB());
		}
		RecIDType recordID = GetSelectedRecordID( inRecordIndex, inContext);
		FicheInMem *fiche = LoadFicheInMem( recordID, HowToLock, context, WithSubTable, outLockWasKeptInTrans);
		if (fiche == nil)
			return nil;
		CDB4DRecord* result = new VDB4DRecord( fManager, fiche, inContext);
		if (result == nil)
			fiche->Release();
		return result;
	}
}


void* VDB4DSelection::LoadRawRecord(RecIDType inRecordIndex, DB4D_Way_of_Locking HowToLock, CDB4DBaseContextPtr inContext, 
									VErrorDB4D& outErr, Boolean* outCouldLock, Boolean* outLockWasKeptInTrans)
{
	if (fSel->IsRemoteLike())
	{
		outErr = ThrowBaseError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
		return nil;
	}
	else
	{
		outErr = VE_OK;

		BaseTaskInfoPtr context = nil;
		if (inContext != nil)
		{
			context = ConvertContext(inContext);
			assert(context->GetBase() == fTable->GetOwner());
		}
		RecIDType recordID = GetSelectedRecordID( inRecordIndex, inContext);
		if (recordID == -2)
			return (void*)kNullRawRecord;
		else
		{
			if (outLockWasKeptInTrans != nil)
			{
				*outLockWasKeptInTrans = false;
				Transaction* trans = GetCurrentTransaction(context);
				if (trans != nil)
					*outLockWasKeptInTrans = trans->WasLockKeptInThatTransaction(fTable->GetDF(), recordID);
			}
			FicheOnDisk* result = fTable->GetDF()->LoadNotFullRecord(recordID, outErr, HowToLock, context, false, nil, outCouldLock, nil);
			return result;
		}
	}
}


FicheInMem *VDB4DSelection::LoadFicheInMem( RecIDType inRecordID, DB4D_Way_of_Locking HowToLock, BaseTaskInfo* context, Boolean WithSubTable, Boolean* outLockWasKeptInTrans)
{
	assert(!fSel->IsRemoteLike());
	VError err = VE_OK;
	if (outLockWasKeptInTrans != nil)
	{
		*outLockWasKeptInTrans = false;
		Transaction* trans = GetCurrentTransaction(context);
		if (trans != nil)
			*outLockWasKeptInTrans = trans->WasLockKeptInThatTransaction(fTable->GetDF(), inRecordID);
	}
	return fTable->GetDF()->LoadRecord( (sLONG) inRecordID, err, HowToLock, context, WithSubTable);
}

Boolean VDB4DSelection::SortSelection(DB4D_FieldID inFieldID, Boolean inAscending, VDB4DProgressIndicator* InProgress, CDB4DBaseContextPtr inContext)
{
	Boolean result = false;
	
	{
		SortTab sortTab(fTable->GetOwner());
		VError err = sortTab.AddTriLineField( fTable->GetNum(), (sLONG) inFieldID, (uBOOL) inAscending);
		
		if (err == VE_OK)
			result = SortSelection( &sortTab, InProgress, inContext, err, false, true );
		else
			result = false;
	}

	return result;
}


Boolean VDB4DSelection::SortSelection(CDB4DSortingCriteriasPtr inCriterias, VDB4DProgressIndicator* InProgress, CDB4DBaseContextPtr inContext)
{
	{
		VDB4DSortingCriterias *SC = VImpCreator<VDB4DSortingCriterias>::GetImpObject(inCriterias);
		VError err;
		return SortSelection(SC->GetSortTab(), InProgress, inContext, err, false, true);
	}
}


Boolean VDB4DSelection::SortSelectionOnClient(DB4D_FieldID inFieldID, Boolean inAscending, VDB4DProgressIndicator* InProgress, CDB4DBaseContextPtr inContext)
{
	Boolean result = false;

	{
		SortTab sortTab(fTable->GetOwner());
		VError err = sortTab.AddTriLineField( fTable->GetNum(), (sLONG) inFieldID, (uBOOL) inAscending);

		if (err == VE_OK)
			result = SortSelection( &sortTab, InProgress, inContext, err, false, false );
		else
			result = false;
	}

	return result;
}


Boolean VDB4DSelection::SortSelectionOnClient(CDB4DSortingCriteriasPtr inCriterias, VDB4DProgressIndicator* InProgress, CDB4DBaseContextPtr inContext)
{
	{
		VDB4DSortingCriterias *SC = VImpCreator<VDB4DSortingCriterias>::GetImpObject(inCriterias);
		VError err;
		return SortSelection(SC->GetSortTab(), InProgress, inContext, err, false, false);
	}
}


Boolean VDB4DSelection::SortSelection(SortTab* tabs, VDB4DProgressIndicator* InProgress, CDB4DBaseContextPtr inContext, VError& err, Boolean TestUnicite, Boolean forceserver)
{
	Boolean result = true;
	sLONG i,nb;
	FieldNuplet *fn;
	SortLine p1;
	FieldDef *p2;
	IndexInfo *ind;
	Base4D *bd;
	Selection *newsel = nil;
	Boolean ascent = true;
	SortContext tri;
	
	err=VE_OK;

	if (fSel != nil)
	{
		if (fSel->IsRemoteLike())
		{
			if (fSel->GetQTfic() > 1)
			{
				Boolean withformula = false;
				for (SortLineArray::iterator cur = tabs->GetLI()->begin(), end = tabs->GetLI()->end(); cur != end; cur++)
				{
					if (!cur->isfield)
					{
						withformula = true;
						break;
					}
				}
				if (withformula && !forceserver)
				{
					BaseTaskInfo* context = nil;
					if (inContext != nil)
						context = ConvertContext(inContext);
					newsel = fSel->SortSel(err, tabs, context, InProgress, TestUnicite, fTable);
					if (newsel == nil)
					{
						result = false;
						if (!TestUnicite && err == VE_OK)
							err = ThrowBaseError(memfull, DBaction_BuildingSelection);
					}
					else
					{
						if (newsel != fSel)
						{
							newsel->SetModificationCounter(fSel->GetModificationCounter() + 1);
							fSel->Release();
							fSel=newsel;
						}
						else
						{
							newsel->Release();	// L.E. 07/08/07 ACI0053430 SortSel a fait un Retain
						}
					}
				}
				else
				{
					IRequest *req = fTable->GetOwner()->CreateRequest( inContext, Req_SortSelection + kRangeReqDB4D, withformula);
					if (req == nil)
					{
						err = ThrowBaseError(memfull, noaction);
					}
					else
					{
						req->PutBaseParam(fTable->GetOwner());
						req->PutThingsToForget( VDBMgr::GetManager(), ConvertContext(inContext));
						req->PutTableParam(fTable);
						req->PutSelectionParam(fSel, inContext);
						req->PutSortTabParams(tabs);
						req->PutBooleanParam(TestUnicite);
						req->PutProgressParam(InProgress);
						err = req->GetLastError();
						if (err == VE_OK)
						{
							err = req->Send();
							if (err== VE_OK)
								err = req->GetUpdatedInfo(fTable->GetOwner(), ConvertContext(inContext));

							if (err == VE_OK)
							{
								Boolean selIsTheSame = req->GetBooleanReply(err);
								if (selIsTheSame)
								{
									sLONG nb = req->GetLongReply(err);
									if (err == VE_OK)
									{
										fSel->ClearRemoteCache(req->GetInputStream());
										fSel->SetCompte(nb);
										fSel->SetModificationCounter( fSel->GetModificationCounter() + 1);		// sc 08/02/2008 ACI0056050
									}
								}
								else
								{
									Selection* newsel = req->RetainSelectionReply(fTable->GetOwner(), err, inContext);
									if (newsel != NULL)
										newsel->SetModificationCounter( fSel->GetModificationCounter() + 1);		// sc 08/02/2008 ACI0056050
									fSel->Release();
									fSel = newsel;
								}
							}
						}
						req->Release();
					}
				}
			}
		}
		else
		{
			nb=tabs->GetNbLine();
			fn=new FieldNuplet(nb,false);
			if (fn==nil)
			{
				err = ThrowBaseError(memfull, DBaction_BuildingSelection);
			}
			else
			{
				p2=fn->GetDataPtr();
				bool okToLookForIndex = true;
				for (i=1;i<=nb;i++)
				{
					tabs->GetTriLine(i, &p1);
					if (i == 1) 
						ascent = p1.ascendant;
					else
					{
						if (p1.ascendant != ascent)
							okToLookForIndex = false;
					}
					if (p1.isfield)
					{
						p2->numfield=p1.numfield;
						p2->numfile=p1.numfile;
					}
					else
					{
						p2->numfield=0;
						p2->numfile=0;
					}
					p2++;
				}
				
				bd=fTable->GetDF()->GetDB();
				
				fn->UpdateCritFic(bd);
				
				// bd->LockIndexes();
				ind = nil;
				if (okToLookForIndex)
					ind=fTable->FindAndRetainIndex(fn, true);
				if (ind != nil)
				{
					if (((sLONG8)fSel->GetQTfic() * fTable->GetSeqRatioCorrector()) < ((sLONG8)ind->GetNBDiskAccessForAScan(true) / 3))
					{
						ind->ReleaseValid();
						ind->Release();
						ind = nil;
					}
				}

				if (ind!=nil)
				{
					ind->Open(index_read);
					// bd->UnLockIndexes();
					OccupableStack* curstack = gOccPool.GetStackForCurrentThread();

					BaseTaskInfo* context = nil;
					if (inContext != nil)
					{
						context = ConvertContext(inContext);
						assert(context->GetBase() == fSel->GetParentFile()->GetDB());
					}
					newsel=ind->GetHeader()->SortSel(curstack, fSel, ascent, context, err, InProgress, TestUnicite);
					if (newsel!=nil)
					{
						newsel->SetModificationCounter(fSel->GetModificationCounter() + 1);
						fSel->Release();
						fSel=newsel;
					}
					ind->Close();
					ind->ReleaseValid();
					ind->Release();
				}
				else
				{
					// bd->UnLockIndexes();
					if (fSel!=nil)
					{
						BaseTaskInfo* context = nil;
						if (inContext != nil)
							context = ConvertContext(inContext);
						newsel = fSel->SortSel(err, tabs, context, InProgress, TestUnicite);
						if (newsel == nil)
						{
							result = false;
							if (!TestUnicite && err == VE_OK)
								err = ThrowBaseError(memfull, DBaction_BuildingSelection);
						}
						else
						{
							if (newsel != fSel)
							{
								newsel->SetModificationCounter(fSel->GetModificationCounter() + 1);
								fSel->Release();
								fSel=newsel;
							}
							else
							{
								newsel->Release();	// L.E. 07/08/07 ACI0053430 SortSel a fait un Retain
							}
						}
					}
				}
				
				delete fn;
			}
		}
	}
				
	if (err != VE_OK)
	{
		result = false;
	}
	return result;
}


RecIDType VDB4DSelection::GetSelectedRecordID( RecIDType inRecordIndex, CDB4DBaseContextPtr inContext)
{
	RecIDType recordID = kDB4D_NullRecordID;
	if (testAssert( inRecordIndex > 0 && inRecordIndex <= fSel->GetQTfic()))
		recordID = fSel->GetFic( (sLONG) inRecordIndex -1);
	return recordID;
}


CDB4DSet* VDB4DSelection::ConvertToSet(VError& err)
{
	VDB4DSet* res = nil;
	err = VE_OK;

	if (fSel!=nil)
	{
		Bittab* b = new Bittab;
		if (b != nil)
		{
			err = b->FillWithSel(fSel, nil);
			if (err == VE_OK)
			{
				res = new VDB4DSet(fBase, fTable, b);
				if (res == nil) 
				{
					err = ThrowBaseError(memfull, DBaction_BuildingSet);
				}
			}
			b->Release();
		}
	}

	return res;
}


VError VDB4DSelection::AddRecord(const CDB4DRecord* inRecToAdd, Boolean AtTheEnd, CDB4DBaseContext* Context)
{
	VError err = VE_OK;

	if (inRecToAdd != nil)
	{
		RecIDType nrec = inRecToAdd->GetID();
		err = AddRecordID(nrec, AtTheEnd, Context);
	}
	else
		err = VE_DB4D_INVALIDRECORD;

	return err;
}


VError VDB4DSelection::AddRecordID(RecIDType inRecToAdd, Boolean AtTheEnd, CDB4DBaseContext* Context)
{
	VError err = VE_OK;

	if (inRecToAdd >= 0 && inRecToAdd <= kMaxRecordsInTable)
	{
		sLONG nrec = (sLONG)inRecToAdd;
		if (fSel != nil && fSel->IsRemote())
		{
			//CDB4DBaseContext* Context = fSel->GetRemoteContext();
			IRequest *req = fBase->GetBase()->CreateRequest(Context , Req_AddRecIDToSel + kRangeReqDB4D);
			if (req == nil)
			{
				err = ThrowBaseError(memfull, noaction);
			}
			else
			{
				req->PutBaseParam( fBase->GetBase());
				req->PutThingsToForget( VDBMgr::GetManager(), ConvertContext(Context));
				req->PutSelectionParam( fSel, Context);
				req->PutLongParam((sLONG)inRecToAdd);

				err = req->GetLastError();
				if (err == VE_OK)
				{
					err = req->Send();
					if (err == VE_OK)
					{
						Boolean selIsTheSame = req->GetBooleanReply(err);
						if (selIsTheSame)
						{
							sLONG nb = req->GetLongReply(err);
							if (err == VE_OK)
							{
								fSel->ClearRemoteCache(req->GetInputStream());
								fSel->SetCompte(nb);
							}
						}
						else
						{
							Selection* newsel = req->RetainSelectionReply(fBase->GetBase(), err, Context);
							fSel->Release();
							fSel = newsel;
						}
					}
				}
				req->Release();
			}

		}
		else
		{
			if (!AtTheEnd && fSel->GetTypSel() == sel_bitsel)
				fSel->AddToSel(nrec);
			else
			{
				Selection* newsel = fSel->AddToSelection(nrec, err);
				if (newsel != fSel)
				{
					if (fSel != nil)
						fSel->Release();
					fSel = newsel;
				}
			}
		}
	}
	else
		err = VE_DB4D_WRONGRECORDID;

	return err;
}



void VDB4DSelection::Touch()
{
	if (fSel != nil)
		fSel->Touch();
}


uLONG VDB4DSelection::GetModificationCounter() const
{
	if (fSel != nil)
		return fSel->GetModificationCounter();
	else
		return 0;
}



VError VDB4DSelection::FillArray(DB4DCollectionManager& outCollection, CDB4DBaseContextPtr inContext)
{
	VError err = VE_OK;
	if (fSel == nil)
		outCollection.SetCollectionSize(0);
	else
	{
		/*
		BaseTaskInfo* context = nil;
		if (inContext != nil)
		{
			context = ConvertContext(inContext);
			//assert(context->GetBase() == fSel->GetParentFile()->GetDB());
		}
		*/
		err = fSel->FillArray(outCollection);
	}
	return err;
}


VError VDB4DSelection::FillFromArray(DB4DCollectionManager& inCollection, CDB4DBaseContextPtr inContext)
{
	VError err = VE_OK;
	uLONG laststamp = 0;
	sLONG nb = inCollection.GetCollectionSize();
	uBOOL petitesel = nb<kNbElemInSel;
	BaseTaskInfo* context = nil;
	if (inContext != nil)
	{
		context = ConvertContext(inContext);
		//assert(context->GetBase() == fSel->GetParentFile()->GetDB());
	}

	if (fSel != nil)
	{
		laststamp = fSel->GetModificationCounter();
		sLONG typ = fSel->GetTypSel();
		if (petitesel)
		{
			if (typ != sel_petitesel || fSel->IsRemote())
			{
				fSel->Release();
				fSel = nil;
			}
		}
		else
		{
			if (typ != sel_longsel || fSel->IsRemote())
			{
				fSel->Release();
				fSel = nil;
			}
		}
	}

	if (fSel == nil)
	{
		if (petitesel)
		{
			fSel = new PetiteSel(fTable->GetDF(), fTable->GetOwner(), fTable->GetNum());
			if (fSel == nil)
			{
				err = ThrowBaseError(memfull, DBaction_BuildingSelection);
			}
		}
		else
		{
			fSel = new LongSel(fTable->GetDF(), fTable->GetOwner());
			if (fSel == nil)
			{
				err = ThrowBaseError(memfull, DBaction_BuildingSelection);
			}
			else
				((LongSel*)fSel)->PutInCache();

		}
	}

	if (err == VE_OK)
	{
		fSel->SetModificationCounter(laststamp+1);
		err = fSel->FillWithArray(inCollection);
	}

	return err;
}


VError VDB4DSelection::FillArray(sLONG* outArray, sLONG inMaxElements, CDB4DBaseContextPtr inContext)
{
	VError err = VE_OK;
	if (fSel != nil)
	{
		BaseTaskInfo* context = nil;
		if (inContext != nil)
		{
			context = ConvertContext(inContext);
			//assert(context->GetBase() == fSel->GetParentFile()->GetDB());
		}
		err = fSel->FillArray(outArray, inMaxElements);
	}
	return err;
}


VError VDB4DSelection::FillFromArray(const sLONG* inArray, sLONG inNbElements, CDB4DBaseContextPtr inContext)
{
	VError err = VE_OK;
	uLONG laststamp = 0;
	sLONG nb = inNbElements;
	uBOOL petitesel = nb<kNbElemInSel;
	BaseTaskInfo* context = nil;
	if (inContext != nil)
	{
		context = ConvertContext(inContext);
		//assert(context->GetBase() == fSel->GetParentFile()->GetDB());
	}

	if (fSel != nil)
	{
		laststamp = fSel->GetModificationCounter();
		sLONG typ = fSel->GetTypSel();
		if (petitesel)
		{
			if (typ != sel_petitesel || fSel->IsRemote())
			{
				fSel->Release();
				fSel = nil;
			}
		}
		else
		{
			if (typ != sel_longsel || fSel->IsRemote())
			{
				fSel->Release();
				fSel = nil;
			}
		}
	}

	if (fSel == nil)
	{
		if (petitesel)
		{
			fSel = new PetiteSel(fTable->GetDF(), fTable->GetOwner(), fTable->GetNum());
			if (fSel == nil)
			{
				err = ThrowBaseError(memfull, DBaction_BuildingSelection);
			}
		}
		else
		{
			fSel = new LongSel(fTable->GetDF(), fTable->GetOwner());
			if (fSel == nil)
			{
				err = ThrowBaseError(memfull, DBaction_BuildingSelection);
			}
			else
				((LongSel*)fSel)->PutInCache();

		}
	}

	if (err == VE_OK)
	{
		fSel->SetModificationCounter(laststamp+1);
		err = fSel->FillWith(inArray, sizeof(sLONG), inNbElements, true, fTable->GetMaxRecords(context), 0, false);
	}

	return err;
}


VError VDB4DSelection::FillArray(xArrayOfLong &outArray, CDB4DBaseContextPtr inContext)
{
	VError err = VE_OK;
	if (fSel == nil)
		outArray.SetCount(0);
	else
	{
		BaseTaskInfo* context = nil;
		if (inContext != nil)
		{
			context = ConvertContext(inContext);
			//assert(context->GetBase() == fSel->GetParentFile()->GetDB());
		}
		sLONG nb = fSel->GetQTfic();
		if (outArray.SetCount(nb))
		{
			err = fSel->FillArray(outArray.First(), nb);
		}
		else
			err = ThrowBaseError(memfull, DBaction_BuildingSelection);
	}
	return err;
}


VError VDB4DSelection::FillFromArray(const xArrayOfLong &inArray, CDB4DBaseContextPtr inContext)
{
	VError err = VE_OK;
	uLONG laststamp = 0;
	sLONG nb = inArray.GetCount();
	uBOOL petitesel = nb<kNbElemInSel;
	BaseTaskInfo* context = nil;
	if (inContext != nil)
	{
		context = ConvertContext(inContext);
		//assert(context->GetBase() == fSel->GetParentFile()->GetDB());
	}

	if (fSel != nil)
	{
		laststamp = fSel->GetModificationCounter();
		sLONG typ = fSel->GetTypSel();
		if (petitesel)
		{
			if (typ != sel_petitesel || fSel->IsRemote())
			{
				fSel->Release();
				fSel = nil;
			}
		}
		else
		{
			if (typ != sel_longsel || fSel->IsRemote())
			{
				fSel->Release();
				fSel = nil;
			}
		}
	}

	if (fSel == nil)
	{
		if (petitesel)
		{
			fSel = new PetiteSel(fTable->GetDF(), fTable->GetOwner(), fTable->GetNum());
			if (fSel == nil)
			{
				err = ThrowBaseError(memfull, DBaction_BuildingSelection);
			}
		}
		else
		{
			fSel = new LongSel(fTable->GetDF(), fTable->GetOwner());
			if (fSel == nil)
			{
				err = ThrowBaseError(memfull, DBaction_BuildingSelection);
			}
			else
				((LongSel*)fSel)->PutInCache();

		}
	}

	if (err == VE_OK)
	{
		fSel->SetModificationCounter(laststamp+1);
		err = fSel->FillWith((void*) inArray.First(), sizeof(sLONG), nb, true, fTable->GetMaxRecords(context), 0, false);
	}

	return err;
}


VError VDB4DSelection::FillArrayOfBits(void* outArray, sLONG inMaxElements)
{
	VError err = VE_OK;

	if (fSel != nil)
	{
		Bittab* b = fSel->GenereBittab(nil, err);
		if (err == VE_OK)
		{
			if (b != nil)
			{
				err = b->FillArrayOfBits(outArray, inMaxElements);
			}
		}

		ReleaseRefCountable(&b);
	}
	return err;
}


VError VDB4DSelection::FillFromArrayOfBits(const void* inArray, sLONG inNbElements)
{
	sLONG laststamp = 0;
	VError err = VE_OK;
	if (fSel != nil)
	{
		laststamp = fSel->GetModificationCounter();
		fSel->Release();
		fSel = nil;
	}
	fSel = new BitSel(fTable->GetDF(), fTable->GetOwner());
	if (fSel == nil)
		err = ThrowBaseError(memfull, DBaction_BuildingSelection);
	if (fSel != nil)
	{
		Bittab* b = fSel->GenereBittab(nil, err);
		if (b != nil)
		{
			err = b->FillFromArrayOfBits(inArray, inNbElements);
			b->Release();
		}
		fSel->SetModificationCounter(laststamp);
		fSel->Touch();
	}

	return err;
}


VError VDB4DSelection::DataToCollection(DB4DCollectionManager& Collection, CDB4DBaseContextPtr inContext, sLONG FromRecInSel, sLONG ToRecInSel,
																				VDB4DProgressIndicator* InProgress)
{
	VError err = VE_OK;
	if (fSel->IsRemoteLike())
	{
		if (fTable != nil)
		{
			Collection.SetCollectionSize(0);
			sLONG totalRows;
			sLONG startingRow = 0;
			sLONG totalProcessedRows = 0;
			bool cont = true;
			do
			{
				IRequest* req = fTable->GetOwner()->CreateRequest(inContext, Req_DataToCollection + kRangeReqDB4D);
				if (req == nil)
					err = ThrowBaseError(memfull, noaction);
				else
				{
					BaseTaskInfo* context = ConvertContext(inContext);
					req->PutBaseParam(fTable->GetOwner());
					req->PutThingsToForget(fManager, context);
					req->PutTableParam(fTable);
					req->PutSelectionParam(fSel, inContext);
					req->PutLongParam(FromRecInSel+startingRow);
					req->PutLongParam(ToRecInSel);
					req->PutCollectionParam(Collection, true);
					req->PutProgressParam(InProgress);
					err = req->GetLastError();
					if (err == VE_OK)
					{
						err = req->Send();
						if (err== VE_OK)
							err = req->GetUpdatedInfo(fTable->GetOwner(), context);

						if (err == VE_OK)
						{
							sLONG totalRows2, processedRows;
							err = req->GetCollectionReply(Collection, totalRows2, startingRow, processedRows);
							if (startingRow == 0)
							{
								totalRows = totalRows2;
							}
							if (processedRows == 0)
								cont = false;
							else
							{
								totalProcessedRows += processedRows;
								if (totalProcessedRows >= totalRows)
									cont = false;
								else
								{
									startingRow += processedRows;
								}
							}
						}
					}
					req->Release();
				} 
			} while (err == VE_OK && cont);
		}
	}
	else
	{
		BaseTaskInfoPtr context = nil;
		if (inContext != nil)
		{
			context = ConvertContext(inContext);
			assert(context->GetBase() == fSel->GetParentFile()->GetDB());
		}

		if (fTable != nil)
		{
			if (fSel != nil && fTable->GetDF() != nil)
			{
				bool fullycompleted = true;
				sLONG maxelems;
				err = fTable->GetDF()->DataToCollection(fSel, Collection, FromRecInSel, ToRecInSel, context, InProgress, false, fullycompleted, maxelems, false); 
			}
		}
		else
			err = VE_DB4D_WRONGTABLEREF;
	}

	return err;
}


VError VDB4DSelection::CollectionToData(DB4DCollectionManager& Collection, CDB4DBaseContextPtr inContext, Boolean AddToSel, Boolean CreateAlways,
																				CDB4DSet* &outLockedRecords, VDB4DProgressIndicator* InProgress)
{
	VError err = VE_OK;
	BaseTaskInfoPtr context = nil;
	outLockedRecords = nil;

	if (fSel->IsRemoteLike())
	{
		if (fTable != nil)
		{
			Boolean legacycall = fTable->GetOwner()->IsThereATrigger(DB4D_SaveNewRecord_Trigger, fTable) || fTable->GetOwner()->IsThereATrigger(DB4D_SaveExistingRecord_Trigger, fTable);
			IRequest* req = fTable->GetOwner()->CreateRequest(inContext, Req_CollectionToData + kRangeReqDB4D, legacycall);
			if (req == nil)
				err = ThrowBaseError(memfull, noaction);
			else
			{
				BaseTaskInfo* context = ConvertContext(inContext);
				req->PutBaseParam(fTable->GetOwner());
				req->PutThingsToForget(fManager, context);
				req->PutTableParam(fTable);
				req->PutSelectionParam(fSel, inContext);
				req->PutBooleanParam(AddToSel);
				req->PutBooleanParam(CreateAlways);
				req->PutCollectionParam(Collection, false);
				req->PutProgressParam(InProgress);
				err = req->GetLastError();
				if (err == VE_OK)
				{
					err = req->Send();
					if (err== VE_OK)
						err = req->GetUpdatedInfo(fTable->GetOwner(), context);

					if (err == VE_OK)
					{
						Boolean selIsTheSame = req->GetBooleanReply(err);
						if (selIsTheSame)
						{
							sLONG nb = req->GetLongReply(err);
							if (err == VE_OK)
							{
								fSel->ClearRemoteCache(req->GetInputStream());
								fSel->SetCompte(nb);
							}
						}
						else
						{
							Selection* newsel = req->RetainSelectionReply(fTable->GetOwner(), err, inContext);
							fSel->Release();
							fSel = newsel;
						}

						if (err == VE_OK)
						{
							Bittab* b = req->RetainSetReply(fTable->GetOwner(), fTable, err, inContext);
							if (b != nil)
							{
								CDB4DBase* xbase = fTable->GetOwner()->RetainBaseX();
								outLockedRecords = new VDB4DSet(VImpCreator<VDB4DBase>::GetImpObject(xbase), fTable, b);
								xbase->Release();
								b->Release();
							}
						}
					}
				}
				req->Release();
			}
		}
	}
	else
	{
		if (inContext != nil)
		{
			context = ConvertContext(inContext);
			assert(context->GetBase() == fSel->GetParentFile()->GetDB());
		}

		if (fTable != nil)
		{
			if (fSel != nil && fTable->GetDF() != nil)
			{
				Selection* newsel = nil;
				Bittab* lockedset = nil;
				err = fTable->GetDF()->CollectionToData(fSel, Collection, context,  AddToSel, CreateAlways, newsel, lockedset, InProgress);
				if (newsel	!= nil)
				{
					if (newsel != fSel)
					{
						fSel->Release();
						fSel = newsel;
					}
				}
				if (lockedset != nil)
				{
					outLockedRecords = new VDB4DSet(fBase, fTable, lockedset);
					lockedset->Release();
				}
			}
		}
		else
			err = VE_DB4D_WRONGTABLEREF;
	}

	return err;
}


VErrorDB4D VDB4DSelection::GetDistinctValues(CDB4DEntityAttribute* inAttribute, DB4DCollectionManager& outCollection, CDB4DBaseContextPtr inContext, 
									 VDB4DProgressIndicator* InProgress, Boolean inCaseSensitive)
{
	VCompareOptions options;
	options.SetDiacritical(inCaseSensitive);
	return GetDistinctValues(inAttribute, outCollection, inContext, InProgress, options);
}

VErrorDB4D VDB4DSelection::GetDistinctValues(CDB4DEntityAttribute* inAttribute, DB4DCollectionManager& outCollection, CDB4DBaseContextPtr inContext, 
									 VDB4DProgressIndicator* InProgress ,const XBOX::VCompareOptions& inOptions)
{
	VError err = VE_OK;
	BaseTaskInfo* context = ConvertContext(inContext);
	EntityAttribute* att = nil;
	if (inAttribute != nil)
		att = VImpCreator<EntityAttribute>::GetImpObject(inAttribute);
	if (att != nil)
	{
		outCollection.SetCollectionSize(0);
		if (fSel != nil)
		{
			err = fSel->GetDistinctValues((db4dEntityAttribute*)att, outCollection, context, InProgress, inOptions);
		}
	}
	else
		err = fBase->GetBase()->ThrowError(VE_DB4D_WRONGFIELDREF, DBactionFinale);

	return err;
}


VError VDB4DSelection::GetDistinctValues(CDB4DField* inField, DB4DCollectionManager& outCollection, CDB4DBaseContextPtr inContext, 
										 VDB4DProgressIndicator* InProgress, Boolean inCaseSensitive)
{
	VCompareOptions options;
	options.SetDiacritical(inCaseSensitive);
	return GetDistinctValues(inField, outCollection, inContext, InProgress, options);
}


VError VDB4DSelection::GetDistinctValues(CDB4DField* inField, DB4DCollectionManager& outCollection, CDB4DBaseContextPtr inContext, 
										 VDB4DProgressIndicator* InProgress, const VCompareOptions& inOptions)
{
	VError err = VE_OK;
	Field* cri = nil;
	if (inField != nil)
		cri = (VImpCreator<VDB4DField>::GetImpObject(inField))->GetField();
	BaseTaskInfo* context = ConvertContext(inContext);

	if (cri != nil && cri->GetOwner() == fTable)
	{
		outCollection.SetCollectionSize(0);
		if (fSel != nil)
		{
			if (fSel->IsRemoteLike())
			{
				if (fTable != nil)
				{
					IRequest* req = fTable->GetOwner()->CreateRequest(inContext, Req_GetDistinctValues + kRangeReqDB4D);
					if (req == nil)
						err = ThrowBaseError(memfull, noaction);
					else
					{
						BaseTaskInfo* context = ConvertContext(inContext);
						req->PutBaseParam(fTable->GetOwner());
						req->PutThingsToForget(fManager, context);
						req->PutTableParam(fTable);
						req->PutFieldParam(cri);
						req->PutSelectionParam(fSel, inContext);
						req->PutCollectionParam(outCollection, true);
						PutVCompareOptionsIntoStream(inOptions, *(req->GetOutputStream()));
						req->PutProgressParam(InProgress);
						err = req->GetLastError();
						if (err == VE_OK)
						{
							err = req->Send();
							if (err== VE_OK)
								err = req->GetUpdatedInfo(fTable->GetOwner(), context);

							if (err == VE_OK)
							{
								sLONG totalRow, processedRows;
								err = req->GetCollectionReply(outCollection, totalRow, 0, processedRows);
							}
						}
						req->Release();
					}
				}
			}
			else
			{
				err = fSel->GetDistinctValues(cri, outCollection, context, InProgress, inOptions);
			}
		}
	}
	else
		err = fBase->GetBase()->ThrowError(VE_DB4D_WRONGFIELDREF, DBactionFinale);

	return err;
}


VError VDB4DSelection::DeleteRecords(CDB4DBaseContextPtr inContext, CDB4DSet* outNotDeletedOnes, VDB4DProgressIndicator* InProgress)
{
	VError err = VE_OK;

	if (fSel != nil)
	{
		/*
		if (fSel->IsRemoteLike())
		{
			err = ThrowBaseError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
		}
		else
		*/
		{
			BaseTaskInfoPtr context = nil;
			if (inContext != nil)
			{
				context = ConvertContext(inContext);
				//assert(context->GetBase() == fSel->GetParentFile()->GetDB());
			}
			Bittab* b = nil;
			if (outNotDeletedOnes != nil)
			{
				b = (VImpCreator<VDB4DSet>::GetImpObject(outNotDeletedOnes))->GetBittab();
			}
			EntityModel* model = nil;
			if (fModel != nil)
				model = VImpCreator<EntityModel>::GetImpObject(fModel);
			err = fSel->DeleteRecords(context, b, nil, InProgress, fTable);
		}
	}

	return err;
}


VError VDB4DSelection::RemoveRecordNumber(CDB4DBaseContextPtr inContext, RecIDType inRecToRemove)
{
	VError err = VE_OK;

	if (fSel != nil)
	{
		if (fSel->IsRemote())
		{
			err = fSel->DelFromSelRemote((sLONG)inRecToRemove, inContext);
		}
		else
			err = fSel->DelFromSel((sLONG)inRecToRemove);
	}

	return err;
}


VError VDB4DSelection::RemoveSet(CDB4DBaseContextPtr inContext, CDB4DSet* inRecsToRemove)
{
	VError err = VE_OK;

	Bittab* b = nil;
	if (inRecsToRemove != nil)
	{
		b = (VImpCreator<VDB4DSet>::GetImpObject(inRecsToRemove))->GetBittab();
		if (fSel != nil)
		{
			if (fSel->IsRemote())
				err = fSel->DelFromSelRemote(b, inContext);
			else
				err = fSel->DelFromSel(b);
		}
	}

	return err;
}


VError VDB4DSelection::RemoveSelectedRecord(CDB4DBaseContextPtr inContext, RecIDType inRecordIndexToRemove)
{
	return RemoveSelectedRange(inContext, inRecordIndexToRemove, inRecordIndexToRemove);
}


VError VDB4DSelection::RemoveSelectedRange(CDB4DBaseContextPtr inContext, RecIDType inFromRecordIndexToRemove, RecIDType inToRecordIndexToRemove)
{
	VError err = VE_OK;

	if (fSel != nil)
	{
		if (fSel->IsRemote())
		{
			err = fSel->RemoveSelectedRangeRemote(inFromRecordIndexToRemove, inToRecordIndexToRemove, inContext);
		}
		else
		{
			BaseTaskInfoPtr context = nil;
			if (inContext != nil)
			{
				context = ConvertContext(inContext);
				//assert(context->GetBase() == fSel->GetParentFile()->GetDB());
			}
			err = fSel->RemoveSelectedRange(inFromRecordIndexToRemove, inToRecordIndexToRemove, context);
		}
	}

	return err;
}


VError VDB4DSelection::BuildOneRecSelection(RecIDType inRecNum)
{
	VError err = VE_OK;
	uLONG laststamp = 0;

	if (fSel != nil)
	{
		laststamp = fSel->GetModificationCounter();
		if (fSel->GetTypSel() != sel_petitesel)
		{
			fSel->Release();
			fSel = nil;
		}
	}

	if (fSel == nil)
	{
		fSel = new PetiteSel(fTable->GetDF(), fTable->GetOwner(), fTable->GetNum());
		if (fSel == nil)
			err = ThrowBaseError(memfull, DBaction_BuildingSelection);
		else
		{
			err = ((PetiteSel*)fSel)->BuildWithOneRec((sLONG)inRecNum);
			fSel->SetModificationCounter(laststamp+1);
		}
	}
	else
	{
		err = ((PetiteSel*)fSel)->BuildWithOneRec((sLONG)inRecNum);
	}


	return err;
}


VError VDB4DSelection::ReduceSelection(RecIDType inNbRec, CDB4DBaseContext* Context)
{
	VError err = VE_OK;
	if (fSel != nil)
	{
		if (fSel->IsRemote())
		{
			//CDB4DBaseContext* Context = fSel->GetRemoteContext();
			IRequest *req = fBase->GetBase()->CreateRequest(Context , Req_ReduceSel + kRangeReqDB4D);
			if (req == nil)
			{
				err = ThrowBaseError(memfull, noaction);
			}
			else
			{
				req->PutBaseParam( fBase->GetBase());
				req->PutThingsToForget( VDBMgr::GetManager(), ConvertContext(Context));
				req->PutSelectionParam( fSel, Context);
				req->PutLongParam((sLONG)inNbRec);

				err = req->GetLastError();
				if (err == VE_OK)
				{
					err = req->Send();
					sLONG nb = req->GetLongReply(err);
					if (err == VE_OK)
					{
						fSel->ClearRemoteCache(req->GetInputStream());
						fSel->SetCompte(nb);
					}

				}
				req->Release();
			}
		}
		else
		{
			fSel->ReduceSel((sLONG)inNbRec);
		}

	}
	return err;
}



CDB4DSelection* VDB4DSelection::Clone(VError* outErr) const
{
	// L.E. 28/11/05 added
	
	VError err = VE_OK;
	Selection *newsel = nil;
	VDB4DSelection *xnewsel = nil;

	if (fSel == nil)
		newsel = nil;
	else
	{
		fSel->DupliqueInto( newsel);
		if (newsel == nil)
		{
			err = ThrowBaseError(memfull, DBaction_BuildingSelection);
		}
	}
	if (err == VE_OK)
	{
		xnewsel = new VDB4DSelection(fManager, fBase, fTable, newsel);
		if (xnewsel == nil)
		{
			err = ThrowBaseError(memfull, DBaction_BuildingSelection);
			if (newsel != nil)
				newsel->Release();
		}
	}

	if (outErr != nil)
		*outErr = err;
	return xnewsel;
}


RecIDType VDB4DSelection::GetRecordPos(RecIDType inRecordID, CDB4DBaseContextPtr inContext)
{
	BaseTaskInfoPtr context = nil;
	if (inContext != nil)
	{
		context = ConvertContext(inContext);
		assert( (fSel->GetParentFile() == nil)  || (context->GetBase() == fSel->GetParentFile()->GetDB()) );
	}

	if (fSel == nil)
		return -1;
	else
		return fSel->GetRecordPos((sLONG)inRecordID, context);
}


CDB4DSet* VDB4DSelection::GenerateSetFromRange(RecIDType inRecordIndex1, RecIDType inRecordIndex2, VErrorDB4D& err, CDB4DBaseContextPtr inContext)
{
	BaseTaskInfoPtr context = nil;
	if (inContext != nil)
	{
		context = ConvertContext(inContext);
		assert( (fSel->GetParentFile() == nil) || (context->GetBase() == fSel->GetParentFile()->GetDB()) );
	}
	if (fSel == nil)
	{
		err = ThrowBaseError(VE_DB4D_SELECTION_IS_NULL, DBaction_BuildingSelection);
		return nil;
	}
	else
	{
		CDB4DSet* res = nil;
		Bittab* b = fSel->GenerateSetFromRange((sLONG)inRecordIndex1, (sLONG)inRecordIndex2, err, context);
		if (b!= nil)
		{
			res = new VDB4DSet(fBase, fTable, b);
			if (res == nil) 
			{
				err = ThrowBaseError(memfull, DBaction_BuildingSelection);
			}
			b->Release();
		}
		return res;
	}

}


CDB4DSet* VDB4DSelection::GenerateSetFromRecordID(RecIDType inRecordID, RecIDType inRecordIndex2, VErrorDB4D& err, CDB4DBaseContextPtr inContext)
{
	BaseTaskInfoPtr context = nil;
	if (inContext != nil)
	{
		context = ConvertContext(inContext);
		assert( (fSel->GetParentFile() == nil) || (context->GetBase() == fSel->GetParentFile()->GetDB()) );
	}
	if (fSel == nil)
	{
		err = ThrowBaseError(VE_DB4D_SELECTION_IS_NULL, DBaction_BuildingSelection);
		return nil;
	}
	else
	{
		CDB4DSet* res = nil;
		Bittab* b = fSel->GenerateSetFromRecordID((sLONG)inRecordID, (sLONG)inRecordIndex2, err, context);
		if (b!= nil)
		{
			res = new VDB4DSet(fBase, fTable, b);
			if (res == nil) 
			{
				err = ThrowBaseError(memfull, DBaction_BuildingSelection);
			}
			b->Release();
		}
		return res;
	}
}


CDB4DSelection* VDB4DSelection::RelateOneSelection(sLONG TargetOneTable, VError& err, CDB4DBaseContextPtr inContext, 
												   VDB4DProgressIndicator* InProgress, vector<CDB4DRelation*> *inPath, DB4D_Way_of_Locking HowToLock, CDB4DSet* outLockSet)
{
	CDB4DSelection* result = nil;
	if (fSel != nil)
	{
		if (fSel->IsRemoteLike())
		{
			CDB4DQueryOptions* options = fBase->NewQueryOptions();
			//VImpCreator<VDB4DQueryOptions>::GetImpObject(options)->GetOptions()->SetFilterTable(fTable);
			options->SetFilter(this);
			options->SetWayOfLocking(HowToLock);
			options->SetWantsLockedSet(outLockSet != nil);
			CDB4DQueryResult* xqresult = fBase->RelateOneSelection(TargetOneTable, err, inContext, options, InProgress, inPath);
			if (xqresult != nil)
			{
				result = xqresult->GetSelection();
				if (result != nil)
					result->Retain();
				if (outLockSet != nil)
				{
					outLockSet->ClearOrSetAll(false);
					CDB4DSet* xset = xqresult->GetLockedSet();
					if (xset != nil)
						outLockSet->Or(*xset);
				}
				xqresult->Release();
			}
		}
		else
		{
			Table* dest = fSel->GetParentFile()->GetDB()->RetainTable(TargetOneTable);
			if (dest == nil)
				err = VE_DB4D_WRONGTABLEREF;
			else
			{
				Base4D* db = dest->GetOwner();
				BaseTaskInfoPtr context = nil;
				if (inContext != nil)
				{
					context = ConvertContext(inContext);
					assert(context->GetBase() == db);
				}

				Bittab* lockedset = nil;
				if (outLockSet != nil)
					lockedset = (VImpCreator<VDB4DSet>::GetImpObject(outLockSet))->GetBittab();

				assert(fBase->GetBase() == db);

				SearchTab xs(dest, true);
				xs.AddSearchLineSel(fSel);
				if (inPath != nil)
				{
					for (vector<CDB4DRelation*>::iterator cur = inPath->begin(), end = inPath->end(); cur != end; cur++)
					{
						CDB4DRelation* xrel = *cur;
						if (testAssert(xrel != nil))
						{
							xs.AddSearchLineBoolOper(DB4D_And);
							Relation* rel = (VImpCreator<VDB4DRelation>::GetImpObject(xrel))->GetRel();
							xs.AddSearchLineJoin(rel->GetSource(), DB4D_Like, rel->GetDest());
						}

					}
				}
				OptimizedQuery query;
				db->LockIndexes();
				err = query.AnalyseSearch(&xs, context);
				db->UnLockIndexes();
				if (err == VE_OK)
				{
					Selection* sel = query.Perform((Bittab*)nil, InProgress, context, err, HowToLock, 0, lockedset);

					if (sel != nil)
					{
						result = new VDB4DSelection(fManager, fBase, dest, sel);
					}
				}
			}
		}
	}
	else
		err = VE_DB4D_SELECTION_IS_NULL;

	return result;
}


CDB4DSelection* VDB4DSelection::RelateManySelection(CDB4DField* inRelationStart, VError& err, CDB4DBaseContextPtr inContext, 
													VDB4DProgressIndicator* InProgress, DB4D_Way_of_Locking HowToLock, CDB4DSet* outLockSet)
{
	CDB4DSelection* result = nil;
	if (fSel != nil)
	{
		err = VE_OK;

		if (inRelationStart != nil)
		{
			if (fSel->IsRemoteLike())
			{
				CDB4DQueryOptions* options = fBase->NewQueryOptions();
				//VImpCreator<VDB4DQueryOptions>::GetImpObject(options)->GetOptions()->SetFilterTable(fTable);
				options->SetFilter(this);
				options->SetWayOfLocking(HowToLock);
				options->SetWantsLockedSet(outLockSet != nil);
				CDB4DQueryResult* xqresult = fBase->RelateManySelection(inRelationStart, err, inContext, options, InProgress);
				if (xqresult != nil)
				{
					result = xqresult->GetSelection();
					if (result != nil)
						result->Retain();
					if (outLockSet != nil)
					{
						outLockSet->ClearOrSetAll(false);
						CDB4DSet* xset = xqresult->GetLockedSet();
						if (xset != nil)
							outLockSet->Or(*xset);
					}
					xqresult->Release();
				}
			}
			else
			{
				Field* cri = (VImpCreator<VDB4DField>::GetImpObject(inRelationStart))->GetField();
				if (testAssert(cri != nil))
				{
					Table* dest = cri->GetOwner();
					Base4D* db = dest->GetOwner();
					BaseTaskInfoPtr context = nil;
					if (inContext != nil)
					{
						context = ConvertContext(inContext);
						assert(context->GetBase() == db);
					}

					Bittab* lockedset = nil;
					if (outLockSet != nil)
						lockedset = (VImpCreator<VDB4DSet>::GetImpObject(outLockSet))->GetBittab();

					assert(fBase->GetBase() == db);

					SearchTab xs(dest, true);
					xs.AddSearchLineSel(fSel);
					cri->occupe();
					DepRelationArrayIncluded* rels = cri->GetRelNto1Deps();
					for (DepRelationArrayIncluded::Iterator cur = rels->First(), end = rels->End(); cur != end; cur++)
					{
						Relation* rel = *cur;
						if (rel != nil)
						{
							if (rel->GetDest() != nil && rel->GetDest()->GetOwner() == fSel->GetParentFile()->GetTable())
							{
								xs.AddSearchLineBoolOper(DB4D_And);
								xs.AddSearchLineJoin(rel->GetSource(), DB4D_Like, rel->GetDest());
							}
						}
					}
					cri->libere();
					OptimizedQuery query;
					db->LockIndexes();
					err = query.AnalyseSearch(&xs, context);
					db->UnLockIndexes();
					if (err == VE_OK)
					{
						Selection* sel = query.Perform((Bittab*)nil, InProgress, context, err, HowToLock, 0, lockedset);

						if (sel != nil)
						{
							result = new VDB4DSelection(fManager, fBase, dest, sel);
						}
					}

				}
			}
		}
		else
			err = fBase->GetBase()->ThrowError(VE_DB4D_WRONGFIELDREF, DBactionFinale);
	}
	else
		err = ThrowBaseError(VE_DB4D_SELECTION_IS_NULL, noaction);

	return result;
}



VErrorDB4D VDB4DSelection::ToClient(VStream* into, CDB4DBaseContext* inContext)
{
	VError err;

	if (fSel == nil)
		err = VE_DB4D_SELECTION_IS_NULL;
	else
	{
		err = fSel->ToClient(into, inContext);
	}

	return err;
}


VErrorDB4D VDB4DSelection::ToServer(VStream* into, CDB4DBaseContext* inContext)
{
	VError err;

	if (fSel == nil)
		err = VE_DB4D_SELECTION_IS_NULL;
	else
	{
		err = fSel->ToServer(into, inContext);
	}

	return err;
}


void VDB4DSelection::MarkOnServerAsPermanent()
{
	if (fSel != nil)
		VDBMgr::GetManager()->MarkSelectionOnServerAsPermanent(fSel);
}


void VDB4DSelection::UnMarkOnServerAsPermanent(bool willResend)
{
	if (fSel != nil)
		VDBMgr::GetManager()->UnMarkSelectionOnServerAsPermanent(fSel, willResend);
}


VErrorDB4D VDB4DSelection::ExportToSQL(CDB4DBaseContext* inContext, VFolder* inFolder, VDB4DProgressIndicator* inProgress, ExportOption& options)
{
	VError err = VE_OK;
	if (fSel != nil)
		err = fSel->ExportToSQL(this, fTable, ConvertContext(inContext), inFolder, inProgress, options);
	return err;
}


void VDB4DSelection::SetAssociatedModel(CDB4DEntityModel* em)
{
	fModel = em;
}


CDB4DEntityModel* VDB4DSelection::GetModel() const
{
	return fModel;
}

/*
CDB4DEntityRecord* VDB4DSelection::LoadEntity( RecIDType inEntityIndex, DB4D_Way_of_Locking HowToLock, CDB4DBaseContextPtr inContext, Boolean* outLockWasKeptInTrans)
{
	if (fSel->IsRemoteLike())
	{
		ThrowBaseError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
		return nil;
	}
	else
	{
		BaseTaskInfoPtr context = nil;
		if (inContext != nil)
		{
			context = ConvertContext(inContext);
			assert(context->GetBase() == fSel->GetParentFile()->GetDB());
		}
		CDB4DEntityRecord* result = nil;
		RecIDType recordID = GetSelectedRecordID( inEntityIndex, inContext);
		if (fModel != nil)
		{
			EntityModel* xmodel = VImpCreator<EntityModel>::GetImpObject(fModel);
			if (okperm(context, xmodel, DB4D_EM_Read_Perm) || okperm(context, xmodel, DB4D_EM_Update_Perm) || okperm(context, xmodel, DB4D_EM_Delete_Perm))
			{
				FicheInMem *fiche = LoadFicheInMem( recordID, HowToLock, context, false, outLockWasKeptInTrans);
				if (fiche != nil)
				{
					EntityRecord* xrec = new EntityRecord(xmodel , fiche, inContext, HowToLock);
					result = xrec;
					fiche->Release();
					xrec->CallDBEvent(dbev_load, context);
				}
			}
			else
			{
				xmodel->ThrowError(VE_DB4D_NO_PERM_TO_READ);
				context->SetPermError();
			}
		}
		return result;
	}
}
*/

void VDB4DSelection::SetQueryPlan(VValueBag* queryplan)
{
	fSel->SetQueryPlan(queryplan);
}

void VDB4DSelection::SetQueryPath(VValueBag* queryplan)
{
	fSel->SetQueryPath(queryplan);
}

VValueBag* VDB4DSelection::GetQueryPlan()
{
	return fSel->GetQueryPlan();
}

VValueBag* VDB4DSelection::GetQueryPath()
{
	return fSel->GetQueryPath();
}

/*
VError VDB4DSelection::ConvertToJSObject(CDB4DBaseContext* inContext, VJSArray& outArr, const VString& inAttributeList, bool withKey, bool allowEmptyAttList, sLONG from, sLONG count)
{
	if (fSel != nil && fModel != nil)
	{
		return SelToJSObject(ConvertContext(inContext), outArr, VImpCreator<EntityModel>::GetImpObject(fModel), fSel, inAttributeList, withKey, allowEmptyAttList, from, count);
	}
	else
		return -1;
}
*/






//=================================================================================



VErrorDB4D VDB4DFieldCacheCollection::AddField(CDB4DField* inField)
{
	assert(inField != nil);
	return fFieldsCache.AddField(VImpCreator<VDB4DField>::GetImpObject(inField)->GetField());
}


VErrorDB4D VDB4DFieldCacheCollection::WriteToStream(VStream* ToStream)
{
	return fFieldsCache.WriteToStream(ToStream);
}


VErrorDB4D VDB4DFieldCacheCollection::ReadFromStream(VStream* FromStream)
{
	return fFieldsCache.ReadFromStream(FromStream);
}



//=================================================================================



VErrorDB4D VDB4DRemoteRecordCache::RetainCacheRecord(RecIDType inRecIndex, CDB4DRecord* &outRecord, vector<CachedRelatedRecord>& outRelatedRecords)
{
	return fRecordsCache.RetainCacheRecord(inRecIndex, outRecord, outRelatedRecords);
}



//=================================================================================


VDB4DField::VDB4DField(Field* inField)
{
	fCrit = inField;
	fCrit->Retain();
	fOwner = nil;
}


VDB4DField::~VDB4DField()
{
	fCrit->Release();
	if (fOwner != nil)
		fOwner->Release();
}


void VDB4DField::GetName( VString& outName, CDB4DBaseContextPtr inContext) const
{
	fCrit->GetName(outName);
}


VError VDB4DField::SetName( const VString& inName, CDB4DBaseContextPtr inContext)
{
	ObjLocker locker(inContext, fCrit);
	if (locker.CanWork())
	{
		VString s = inName;
		sLONG len = s.GetLength();
		if (len > 3)
		{
			VString s2;
			s.GetSubString(len-2, 3, s2);
			if (s2 == L"+++")
			{
				s.Truncate(len-3);
				SetAutoSequence(true, inContext);
			}
			else
			{
				if (s2 == L"---")
				{
					s.Truncate(len-3);
					SetAutoSequence(false, inContext);
				}
			}
		}

		return fCrit->SetName(s,inContext);
	}
	else
		return VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT;
}


DB4D_FieldID VDB4DField::GetID(CDB4DBaseContextPtr inContext) const
{
	return (DB4D_FieldID)fCrit->GetPosInRec();
}


void VDB4DField::GetUUID(VUUID& outID, CDB4DBaseContextPtr inContext) const
{
	fCrit->GetUUID(outID);
}


VError VDB4DField::SetDefinition(const VValueBag& inFieldDefinition, VDB4DProgressIndicator* InProgress, CDB4DBaseContextPtr inContext)
{
	ObjLocker locker(inContext, fCrit);
	if (locker.CanWork())
	{
		return fCrit->GetOwner()->SetCrit(fCrit->GetPosInRec(), inFieldDefinition, inContext, InProgress);
	}
	else
		return VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT;
}


VValueBag* VDB4DField::CreateDefinition(CDB4DBaseContextPtr inContext) const
{
	VValueBag *bag = new VValueBag;
	if (bag != NULL)
	{
		VString kind;
		VError err = fCrit->SaveToBag( *bag, kind);
		vThrowError( err);
	}
	return bag;
}


VError VDB4DField::SetType(DB4DFieldType inType, VDB4DProgressIndicator* InProgress, CDB4DBaseContextPtr inContext)
{
	VError err = VE_OK;
	ObjLocker locker(inContext, fCrit);
	if (locker.CanWork())
	{
		CritereDISK CRD = *(fCrit->getCRD());
		CRD.typ = inType;
		fCrit->setCRD(&CRD, inContext, true, InProgress);
	}
	else
		err = VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT;
	return err;
}


DB4DFieldType VDB4DField::GetType(CDB4DBaseContextPtr inContext) const
{
	sLONG typ = fCrit->GetTyp();
	if (typ == VK_UUID)
		return VK_STRING;
	if (typ == VK_STRING_UTF8)
		return VK_STRING;
	if (typ == VK_TEXT_UTF8)
		return VK_TEXT;
	return (DB4DFieldType) typ;
}


DB4DFieldType VDB4DField::GetRealType(CDB4DBaseContextPtr inContext) const
{
	return (DB4DFieldType)fCrit->GetTyp();
}



sLONG VDB4DField::GetTextSwitchSize(CDB4DBaseContextPtr inContext) const
{
	return fCrit->GetTextSwitchSize();
}


VErrorDB4D VDB4DField::SetTextSwitchSize(sLONG inLength, CDB4DBaseContextPtr inContext)
{
	ObjLocker locker(inContext, fCrit);
	if (locker.CanWork())
	{
		fCrit->SetTextSwitchSize(inLength, inContext);
		return VE_OK;
	}
	else
		return VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT;
}


sLONG VDB4DField::GetBlobSwitchSize(CDB4DBaseContextPtr inContext) const
{
	return fCrit->GetBlobSwitchSize();
}


VErrorDB4D VDB4DField::SetBlobSwitchSize(sLONG inLength, CDB4DBaseContextPtr inContext)
{
	ObjLocker locker(inContext, fCrit);
	if (locker.CanWork())
	{
		fCrit->SetBlobSwitchSize(inLength, inContext);
		return VE_OK;
	}
	else
		return VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT;
}


sLONG VDB4DField::GetLimitingLength(CDB4DBaseContextPtr inContext) const
{
	return fCrit->GetLimitingLen();
}


VErrorDB4D VDB4DField::SetLimitingLength(sLONG inLength, CDB4DBaseContextPtr inContext)
{
	ObjLocker locker(inContext, fCrit);
	if (locker.CanWork())
	{
		fCrit->SetLimitingLen(inLength, inContext);
		return VE_OK;
	}
	else
		return VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT;
}


Boolean VDB4DField::IsUnique(CDB4DBaseContextPtr inContext) const
{
	return fCrit->GetUnique();
}


VError VDB4DField::SetUnique(Boolean unique, VDB4DProgressIndicator* InProgress, CDB4DBaseContextPtr inContext)
{
	VError err;
	ObjLocker locker(inContext, fCrit);
	if (locker.CanWork())
	{
		err = fCrit->SetUnique(unique, InProgress, inContext);
	}
	else
		err = VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT;
	return err;
}


Boolean VDB4DField::IsNot_Null(CDB4DBaseContextPtr inContext) const
{
	return fCrit->GetNot_Null();
}


VError VDB4DField::SetNot_Null(Boolean Not_Null, VDB4DProgressIndicator* InProgress, CDB4DBaseContextPtr inContext)
{
	VError err;
	ObjLocker locker(inContext, fCrit);
	if (locker.CanWork())
	{
		err = fCrit->SetNot_Null(Not_Null, InProgress, inContext);
	}
	else
		err = VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT;
	return err;
}


Boolean VDB4DField::IsStoreOutside(CDB4DBaseContextPtr inContext) const
{
	return fCrit->GetOutsideData();
}


VError VDB4DField::SetStoreOutside(Boolean storeOutside, VDB4DProgressIndicator* InProgress, CDB4DBaseContextPtr inContext)
{
	VError err = VE_OK;
	ObjLocker locker(inContext, fCrit);
	if (locker.CanWork())
	{
		fCrit->SetOutsideData(storeOutside, inContext);
	}
	else
		err = VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT;
	return err;
}


Boolean VDB4DField::IsStyledText(CDB4DBaseContextPtr inContext) const
{
	return fCrit->GetStyledText();
}


VError VDB4DField::SetStyledText(Boolean storeOutside, VDB4DProgressIndicator* InProgress, CDB4DBaseContextPtr inContext)
{
	VError err = VE_OK;
	ObjLocker locker(inContext, fCrit);
	if (locker.CanWork())
	{
		fCrit->SetStyledText(storeOutside, inContext, InProgress);
	}
	else
		err = VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT;
	return err;
}


Boolean VDB4DField::IsHiddenInRest(CDB4DBaseContextPtr inContext) const
{
	return fCrit->GetHideInRest();
}


VError VDB4DField::SetHideInRest(Boolean x, VDB4DProgressIndicator* InProgress, CDB4DBaseContextPtr inContext)
{
	VError err = VE_OK;
	ObjLocker locker(inContext, fCrit);
	if (locker.CanWork())
	{
		fCrit->SetHideInRest(x, inContext, InProgress);
	}
	else
		err = VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT;
	return err;
}


Boolean VDB4DField::IsAutoSequence(CDB4DBaseContextPtr inContext) const
{
	return fCrit->GetAutoSeq();
}


VErrorDB4D VDB4DField::SetAutoSequence(Boolean autoseq, CDB4DBaseContextPtr inContext)
{
	ObjLocker locker(inContext, fCrit);
	if (locker.CanWork())
	{
		fCrit->SetAutoSeq(autoseq, inContext);
		return VE_OK;
	}
	else
		return VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT;
}

Boolean VDB4DField::IsAutoGenerate(CDB4DBaseContextPtr inContext) const
{
	return fCrit->GetAutoGenerate();
}


VErrorDB4D VDB4DField::SetAutoGenerate(Boolean autogenerate, CDB4DBaseContextPtr inContext)
{
	ObjLocker locker(inContext, fCrit);
	if (locker.CanWork())
	{
		fCrit->SetAutoGenerate(autogenerate, inContext);
		return VE_OK;
	}
	else
		return VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT;
}


Boolean VDB4DField::IsStoreAsUTF8(CDB4DBaseContextPtr inContext) const
{
	if (fCrit->GetTyp() == VK_STRING_UTF8 || fCrit->GetTyp() == VK_TEXT_UTF8)
		return true;
	else
		return false;
	//return fCrit->GetStoreUTF8();
}


VErrorDB4D VDB4DField::SetStoreAsUTF8(Boolean storeUTF8, CDB4DBaseContextPtr inContext)
{
	ObjLocker locker(inContext, fCrit);
	if (locker.CanWork())
	{
		fCrit->SetStoreUTF8(storeUTF8, inContext);
		if (fCrit->GetTyp() == VK_STRING_UTF8 || fCrit->GetTyp() == VK_STRING || fCrit->GetTyp() == VK_UUID)
		{
			CritereDISK CRD = *(fCrit->getCRD());
			CRD.typ = VK_STRING_UTF8;
			fCrit->setCRD(&CRD, inContext, true, nil);
		}
		else if (fCrit->GetTyp() == VK_TEXT_UTF8 || fCrit->GetTyp() == VK_TEXT)
		{
			CritereDISK CRD = *(fCrit->getCRD());
			CRD.typ = VK_TEXT_UTF8;
			fCrit->setCRD(&CRD, inContext, true, nil);
		}
		return VE_OK;
	}
	else
		return VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT;
}


Boolean VDB4DField::IsStoreAsUUID(CDB4DBaseContextPtr inContext) const
{
	if (fCrit->GetTyp() == VK_UUID)
		return true;
	else
		return false;
}


VErrorDB4D VDB4DField::SetStoreAsUUID(Boolean storeUUID, CDB4DBaseContextPtr inContext)
{
	ObjLocker locker(inContext, fCrit);
	if (fCrit->GetTyp() == VK_UUID || fCrit->GetTyp() == VK_STRING || fCrit->GetTyp() == VK_TEXT || fCrit->GetTyp() == VK_STRING_UTF8 || fCrit->GetTyp() == VK_TEXT_UTF8)
	{
		CritereDISK CRD = *(fCrit->getCRD());
		CRD.typ = VK_UUID;
		fCrit->setCRD(&CRD, inContext, true, nil);
		return VE_OK;
	}
	else
		return VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT;
}


Boolean VDB4DField::IsIndexable(CDB4DBaseContextPtr inContext) const
{
	return fCrit->IsIndexable();
}


Boolean VDB4DField::IsFullTextIndexable(CDB4DBaseContextPtr inContext) const
{
	return fCrit->IsFullTextIndexable();
}


Boolean VDB4DField::IsFullTextIndexed(CDB4DBaseContextPtr inContext) const
{
	return fCrit->IsFullTextIndexed();
}


Boolean VDB4DField::IsIndexed(CDB4DBaseContextPtr inContext) const
{
	return fCrit->IsIndexed();
}


Boolean VDB4DField::IsPrimIndexed(CDB4DBaseContextPtr inContext) const
{
	return fCrit->IsPrimIndexe();
}


Boolean	VDB4DField::IsPrimaryKeyCompatible( CDB4DBaseContextPtr inContext) const
{
	return fCrit->IsSortable() && fCrit->CanBeUnique() && fCrit->CanBeNot_Null();
}


Boolean	VDB4DField::CanBePartOfRelation( CDB4DBaseContextPtr inContext) const
{
	return fCrit->CanBePartOfRelation();
}



CDB4DTable* VDB4DField::GetOwner(CDB4DBaseContextPtr inContext)
{
	if (fOwner == nil)
	{
		CDB4DBase* base;
		VDB4DBase* xbase;
		xbase = new VDB4DBase(VDBMgr::GetManager(), fCrit->GetOwner()->GetOwner(), true);
		fOwner = new VDB4DTable(xbase->GetManager(), xbase, fCrit->GetOwner());
		xbase->Release();

	}
	return fOwner;
}


Boolean VDB4DField::Drop(VDB4DProgressIndicator* InProgress, CDB4DBaseContextPtr inContext)
{
	Boolean ok = false;
	ObjLocker locker(inContext, fCrit);
	if (locker.CanWork())
	{
		ok = fCrit->GetOwner()->DeleteField(fCrit, inContext, InProgress);
	}
	return ok;
}


const VValueBag* VDB4DField::RetainExtraProperties(VError &err, CDB4DBaseContextPtr inContext)
{
	assert(fCrit != nil);
	return fCrit->RetainExtraProperties(err, inContext);
}


VError VDB4DField::SetExtraProperties(VValueBag* inExtraProperties, CDB4DBaseContextPtr inContext)
{
	assert(fCrit != nil);
	ObjLocker locker(inContext, fCrit);
	if (locker.CanWork())
	{
		return fCrit->SetExtraProperties(inExtraProperties, true, inContext);
	}
	else
		return VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT;
}


CDB4DRelation* VDB4DField::RetainFirstNto1Relation(CDB4DBaseContextPtr inContext)
{
	assert(fCrit != nil);
	CDB4DRelation* result = nil;

	fCrit->occupe();

	DepRelationArrayIncluded* rels = fCrit->GetRelNto1Deps();
	if (rels->GetCount() > 0)
	{
		Relationptr rel = *(rels->First());
		if (rel != nil)
		{
			CDB4DBase* xbase = fCrit->GetOwner()->GetOwner()->RetainBaseX("RetainFirstNto1Relation");
			result = new VDB4DRelation(xbase, rel);
			xbase->Release("RetainFirstNto1Relation");
		}
	}

	fCrit->libere();
	return result;
}


#if 0
CDB4DRelation* VDB4DField::RetainRelation_SourceLienAller_V6(Boolean AutoRelOnly, CDB4DBaseContextPtr inContext)
{
	assert(fCrit != nil);
	CDB4DRelation* result = nil;

	fCrit->occupe();
	DepRelationArrayIncluded* rels = fCrit->GetRelNto1Deps();
	Relationptr *end = rels->End(), *p;

	BaseTaskInfoPtr context = nil;
	if (inContext != nil)
	{
		context = ConvertContext(inContext);
	}

	for (p = rels->First(); p != end; p++)
	{
		Relationptr rel = *p;
		if (rel != nil)
		{
			if (rel->IsAutoLoad(context) || !AutoRelOnly)
			{
				CDB4DBase* xbase = fCrit->GetOwner()->GetOwner()->RetainBaseX("RetainRelation_SourceLienAller_V6");
				result = new VDB4DRelation(xbase, rel);
				xbase->Release("RetainRelation_SourceLienAller_V6");
				if (result != nil)
				{
					break;
				}
				/*
				else
					err = memfull;
				*/
			}
		}
	}

	fCrit->libere();
	return result;
}
#endif

CDB4DRelation* VDB4DField::RetainRelation_SubTable(CDB4DBaseContextPtr inContext)
{
	fCrit->occupe();
	Relation *relation = fCrit->GetSubTableRel();
	CDB4DBase* xbase = fCrit->GetOwner()->GetOwner()->RetainBaseX("RetainRelation_SubTable");
	VDB4DRelation *result = (relation != NULL) ? new VDB4DRelation( xbase, relation) : NULL;
	xbase->Release("RetainRelation_SubTable");
	fCrit->libere();
	
	return result;
}

#if 0
CDB4DRelation* VDB4DField::RetainRelation_CibleLienRetour_V6(Boolean AutoRelOnly, CDB4DBaseContextPtr inContext)
{
	assert(fCrit != nil);
	CDB4DRelation* result = nil;

	fCrit->occupe();
	DepRelationArrayIncluded* rels = fCrit->GetRel1toNDeps();
	Relationptr *end = rels->End(), *p;

	BaseTaskInfoPtr context = nil;
	if (inContext != nil)
	{
		context = ConvertContext(inContext);
	}

	for (p = rels->First(); p != end; p++)
	{
		Relationptr rel = *p;
		if (rel != nil)
		{
			if (rel->IsAutoLoad(context) || !AutoRelOnly)
			{
				CDB4DBase* xbase = fCrit->GetOwner()->GetOwner()->RetainBaseX("RetainRelation_CibleLienRetour_V6");
				result = new VDB4DRelation(xbase, rel);
				xbase->Release("RetainRelation_CibleLienRetour_V6");
				break;
			}

			if (result != nil)
			{
				break;
			}
		
		}
	}

	fCrit->libere();
	return result;
}
#endif

VErrorDB4D VDB4DField::GetListOfRelations(RelationArray& outRelations, Boolean AutoRelOnly, Boolean x_Nto1, CDB4DBaseContextPtr inContext)
{
	VError err = VE_OK;
	assert(fCrit != nil);
	outRelations.SetCount(0);
	fCrit->occupe();
	DepRelationArrayIncluded* rels = x_Nto1 ? fCrit->GetRelNto1Deps() : fCrit->GetRel1toNDeps();
	Relationptr *end = rels->End(), *p;

	BaseTaskInfoPtr context = nil;
	if (inContext != nil)
	{
		context = ConvertContext(inContext);
	}

	for (p = rels->First(); p != end; p++)
	{
		Relationptr rel = *p;
		if (rel != nil)
		{
			Boolean ok;
			if (x_Nto1)
				ok = !AutoRelOnly || rel->IsAutoLoadNto1(context);
			else
				ok = !AutoRelOnly || rel->IsAutoLoad1toN(context);
			if (ok)
			{
				CDB4DBase* xbase = fCrit->GetOwner()->GetOwner()->RetainBaseX("GetListOfRelations");
				CDB4DRelation* xrel = new VDB4DRelation(xbase, rel);
				xbase->Release("GetListOfRelations");
				if (xrel != nil)
				{
					if (!outRelations.Add(xrel))
						err = ThrowBaseError(memfull, DBaction_BuildingListOfRelations);
				}
				else
					err = ThrowBaseError(memfull, DBaction_BuildingListOfRelations);
			}
		}
	}

	fCrit->libere();

	return err;
}


VErrorDB4D VDB4DField::GetListOfRelations1_To_N(RelationArray& outRelations, Boolean AutoRelOnly, CDB4DBaseContextPtr inContext)
{
	return GetListOfRelations(outRelations, AutoRelOnly, false, inContext);
}


VErrorDB4D VDB4DField::GetListOfRelationsN_To_1(RelationArray& outRelations, Boolean AutoRelOnly, CDB4DBaseContextPtr inContext)
{
	return GetListOfRelations(outRelations, AutoRelOnly, true, inContext);
}


CDB4DIndex* VDB4DField::RetainIndex(Boolean MustBeSortable, CDB4DBaseContextPtr inContext, Boolean MustBeValid, Boolean Occupy) const
{
	CDB4DIndex* result = nil;

	BaseTaskInfoPtr context = nil;
	context = ConvertContext(inContext);

	if (fCrit != nil)
	{
		IndexInfo* ind = fCrit->GetOwner()->GetOwner()->FindAndRetainIndexSimple(fCrit->GetOwner()->GetNum(), fCrit->GetPosInRec(), MustBeSortable, MustBeValid, context, Occupy);
		if (ind != nil)
		{
			result = new VDB4DIndex(ind);
			if (MustBeValid && !Occupy)
				ind->ReleaseValid();
			ind->Release();
		}
	}
	return result;
}


CDB4DIndex* VDB4DField::RetainFullTextIndex(CDB4DBaseContextPtr inContext, Boolean MustBeValid, Boolean Occupy) const
{
	CDB4DIndex* result = nil;

	BaseTaskInfoPtr context = nil;
	context = ConvertContext(inContext);

	if (fCrit != nil)
	{
		IndexInfo* ind = fCrit->GetOwner()->GetOwner()->FindAndRetainIndexLexico(fCrit->GetOwner()->GetNum(), fCrit->GetPosInRec(), MustBeValid, context, Occupy);
		if (ind != nil)
		{
			result = new VDB4DIndex(ind);
			if (MustBeValid && !Occupy)
				ind->ReleaseValid();
			ind->Release();
		}
	}
	return result;
}


Boolean VDB4DField::IsNeverNull(CDB4DBaseContextPtr inContext) const
{
	if (fCrit != nil)
		return fCrit->IsNeverNull();
	else
		return false;
}


VError VDB4DField::SetNeverNullState(Boolean inState, CDB4DBaseContextPtr inContext)
{
	VError err = VE_OK;

	ObjLocker locker(inContext, fCrit);
	if (locker.CanWork())
	{
		if (fCrit != nil)
			fCrit->SetNeverNull(inState, inContext);
	}
	else
		err = VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT;
	return err;
}


sLONG VDB4DField::GetStamp(CDB4DBaseContextPtr inContext) const
{
	if (fCrit == nil)
		return 0;
	else
		return fCrit->GetStamp();
}


VError VDB4DField::RetainIndexes(ArrayOf_CDB4DIndex& outIndexes, CDB4DBaseContextPtr inContext)
{
	if (fCrit == nil)
		return VE_OK;
	else
		return fCrit->RetainIndexes(outIndexes);
}


//=================================================================================



VDB4DTable::~VDB4DTable()
{
	fTable->Release();
	fBase->Release("VDB4DTable");
	
}


VDB4DTable::VDB4DTable( VDBMgr *inManager, VDB4DBase *inBase, Table *inTable)
{
	fManager = inManager;
	fTable = inTable;
	fBase = inBase;
	fBase->Retain("VDB4DTable");
	fTable->Retain();
}


void VDB4DTable::GetName( VString& outName, CDB4DBaseContextPtr inContext) const
{
	fTable->GetName( outName);
}

VError VDB4DTable::SetName( const VString& inName, CDB4DBaseContextPtr inContext)
{
	ObjLocker locker(inContext, fTable);
	if (locker.CanWork())
	{
		return fTable->SetName( inName, inContext);
	}
	else
		return VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT;
}

VErrorDB4D VDB4DTable::SetNameNoNameCheck( const XBOX::VString& inName, CDB4DBaseContextPtr inContext)
{
	ObjLocker locker(inContext, fTable);
	if (locker.CanWork())
	{
		return fTable->SetName( inName, inContext, true, false);
	}
	else
		return VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT;
}

VError VDB4DTable::SetRecordName(const VString& inName, CDB4DBaseContextPtr inContext)
{
	ObjLocker locker(inContext, fTable);
	if (locker.CanWork())
	{
		return fTable->SetRecordName( inName, inContext);
	}
	else
		return VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT;
}

void VDB4DTable::GetRecordName( VString& outName, CDB4DBaseContextPtr inContext) const
{
	fTable->GetRecordName( outName);
}


VErrorDB4D VDB4DTable::SetSchemaID(DB4D_SchemaID inSchemaID, CDB4DBaseContext* inContext)
{
	return fTable->SetSchema( inSchemaID, inContext);
}


DB4D_SchemaID VDB4DTable::GetSchemaID(CDB4DBaseContext* inContext) const
{
	return fTable->GetSchema();
}


VError VDB4DTable::SetFullyDeleteRecords(Boolean FullyDeleteState, CDB4DBaseContextPtr inContext)
{
	ObjLocker locker(inContext, fTable);
	if (locker.CanWork())
	{
		return fTable->SetFullyDeleteRecords(FullyDeleteState, inContext);
	}
	else
		return VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT;
}


Boolean VDB4DTable::GetFullyDeleteRecordsState() const
{
	return fTable->GetFullyDeleteRecordsState();
}


DB4D_TableID VDB4DTable::GetID(CDB4DBaseContextPtr inContext) const
{
	return (DB4D_TableID) fTable->GetNum();
}


VValueBag *VDB4DTable::CreateDefinition(CDB4DBaseContextPtr inContext) const
{
	VValueBag *bag = new VValueBag;
	if (bag != NULL)
	{
		VString kind;
		VError err = fTable->SaveToBag( *bag, kind);
		vThrowError( err);
	}

	return bag;
}


CDB4DQuery *VDB4DTable::NewQuery()
{
	return new VDB4DQuery( fManager, fTable, this);
}


CDB4DQueryResult *VDB4DTable::NewQueryResult( CDB4DSelection *inSelection, CDB4DSet *inSet, CDB4DSet *inLockedSet, CDB4DRecord *inFirstRecord)
{
	return new VDB4DQueryResult( this, inSelection, inSet, inLockedSet, inFirstRecord);
}


sLONG VDB4DTable::CountFields(CDB4DBaseContextPtr inContext) const
{
	return fTable->GetNbCrit();
}


CDB4DSelectionPtr VDB4DTable::ExecuteQuery( CDB4DQuery *inQuery, CDB4DBaseContextPtr inContext, CDB4DSelectionPtr Filter, 
											VDB4DProgressIndicator* InProgress, DB4D_Way_of_Locking HowToLock, sLONG limit, CDB4DSet* outLockSet, VErrorDB4D *outErr)
{
	Selection *sel;
	CDB4DSelection* result = nil;
	Base4D *db;
	DataTable *DF;
	OptimizedQuery rech;
	VError err = VE_OK;

	if (fTable->IsRemote())
	{
		CDB4DQueryOptions* options = fBase->NewQueryOptions();
		options->SetDestination(DB4D_QueryDestination_Selection);
		options->SetFilter(Filter);
		options->SetWayOfLocking(HowToLock);
		options->SetWantsLockedSet(outLockSet != nil);
		options->SetLimit(limit);
		CDB4DQueryResult* xresult = ExecuteQuery(inQuery, inContext, options, InProgress, err);
		if (xresult != nil)
		{
			result = xresult->GetSelection();
			if (result != nil)
				result->Retain();
			xresult->Release();
		}
		options->Release();
	}
	else
	{
		assert(inQuery != nil);
		VDB4DQuery *query = VImpCreator<VDB4DQuery>::GetImpObject(inQuery);
		VDB4DSelection *xfilter;
		if (Filter == nil) xfilter = nil;
		else xfilter = VImpCreator<VDB4DSelection>::GetImpObject(Filter);
		Selection* oldsel = nil;

		BaseTaskInfoPtr context = nil;
		if (inContext != nil)
		{
			context = ConvertContext(inContext);
			assert(context->GetBase() == fTable->GetOwner());
		}
		
		Bittab* lockedset = nil;
		if (outLockSet != nil)
			lockedset = (VImpCreator<VDB4DSet>::GetImpObject(outLockSet))->GetBittab();

		if (testAssert( query != nil)) {
			
			DF=fTable->GetDF();
			db=DF->GetDB();
			if (xfilter != nil) oldsel = xfilter->GetSel();
			
			db->LockIndexes();
			err = rech.AnalyseSearch(query->GetSearchTab(), context);
			db->UnLockIndexes();
			if (err == VE_OK)
			{
				sel = rech.Perform(oldsel, InProgress, context, err, HowToLock, 0, lockedset);
				
				if (sel != nil)
				{
					result = new VDB4DSelection(fManager, fBase, fTable, sel);
					if (result == nil)
					{
						err = memfull;
						sel->Release();
					}
				}
			}
		
		
		}
	}

	if (outErr != nil)
		*outErr = err;
	return result;
}


CDB4DQueryResult* VDB4DTable::ExecuteQuery(CDB4DQuery* inQuery, CDB4DBaseContext* inContext, CDB4DQueryOptions* inOptions, VDB4DProgressIndicator* inProgress, VError& outError)
{

	VDB4DQueryResult* xresult = nil;
	assert(inQuery != nil);
	VDB4DQuery *query = VImpCreator<VDB4DQuery>::GetImpObject(inQuery);
	
	QueryOptions* options = VImpCreator<VDB4DQueryOptions>::GetImpObject(inOptions)->GetOptions();
	BaseTaskInfo* context = ConvertContext(inContext);
	if (context != nil && options->IsDescribeUndef())
		options->SetDescribeQuery(context->ShouldDescribeQuery());

	if (fTable->IsRemote())
	{
		SearchTab* xsearch = query->GetSearchTab();
		Boolean legacycall = query->WithFormulas() || xsearch->WithArrays();
		IRequest *req = fTable->GetOwner()->CreateRequest( inContext, Req_ExecuteQuery + kRangeReqDB4D, legacycall);
		if (req == nil)
		{
			outError = ThrowBaseError(memfull, noaction);
		}
		else
		{
			req->PutBaseParam( fTable->GetOwner());
			req->PutThingsToForget( VDBMgr::GetManager(), context);
			req->PutTableParam( fTable);
			req->PutQueryParam(query->GetSearchTab());
			req->PutQueryOptionsParam(options, context);
			req->PutProgressParam(inProgress);

			outError = req->GetLastError();
			if (outError == VE_OK)
			{
				outError = req->Send();
				if (outError== VE_OK)
					outError = req->GetUpdatedInfo(fTable->GetOwner(), context);

				if (outError == VE_OK)
				{
					xresult = new VDB4DQueryResult(this);
					if (xresult == nil)
						outError = ThrowBaseError(memfull, noaction);
					else
					{
						QueryResult& result = xresult->GetResult();
						outError = req->ReadQueryResultReply( fTable->GetOwner(), fTable, &(xresult->GetResult()), context);
						if (context != nil && outError == VE_OK)
						{
							context->SetQueryDescription(xresult->GetResult().GetQueryDescription());
							context->SetQueryExecution(xresult->GetResult().GetQueryExecution());
							context->SetQueryExecutionXML(xresult->GetResult().GetQueryExecutionXML());
						}
						if (outError == VE_OK && result.GetLockedSet() != nil && result.GetLockedSet()->Compte() != 0)
							outError = VE_DB4D_RECORDISLOCKED;
					}
				}
			}
			req->Release();
		}
	}
	else
	{
		if (testAssert( query != nil)) 
		{
			xresult = new VDB4DQueryResult(this);
			if (xresult == nil)
				outError = ThrowBaseError(memfull, noaction);
			else
			{
				outError = fTable->GetDF()->ExecuteQuery(query->GetSearchTab(), options, xresult->GetResult(), context, inProgress);
				/*
				if (outError != VE_OK)
				{
					xresult->Release();
					xresult = nil;
				}
				*/
			}

		}
	}

	return xresult;
}


CDB4DSelectionPtr VDB4DTable::SelectAllRecordsAndLoadFirstOne( CDB4DBaseContextPtr inContext, VErrorDB4D* outErr, bool inLoadRecordReadOnly, CDB4DRecord **outFirstRecord)
{
	VError err;
	CDB4DSelection *selection = nil;
	CDB4DRecord *record = nil;
	
	if (fTable->IsRemote())
	{
		Selection *sel = nil;
		FicheInMem* fic = nil;

		IRequest *req = fTable->GetOwner()->CreateRequest( inContext, Req_SelectAllRecords + kRangeReqDB4D);
		if (req == nil)
		{
			err = ThrowBaseError(memfull, noaction);
		}
		else
		{
			req->PutBaseParam( fTable->GetOwner());
			req->PutThingsToForget( VDBMgr::GetManager(), ConvertContext(inContext));
			req->PutTableParam( fTable);
			
			uLONG loadOptions = 1;	// means we want the first record
			if (inLoadRecordReadOnly)
				loadOptions |= 2;
			req->GetOutputStream()->Put( loadOptions);
		
			err = req->GetLastError();
			if (err == VE_OK)
			{
				err = req->Send();
				if (err== VE_OK)
					err = req->GetUpdatedInfo(fTable->GetOwner(), ConvertContext(inContext));
				if (err == VE_OK)
				{
					sel = req->RetainSelectionReply( fTable->GetOwner(), err, inContext);

					if ( (err == VE_OK) && !sel->IsEmpty())
						fic = req->RetainFicheInMemReply( fTable, err, inContext);
				}
			}
			req->Release();
		}

		if (err == VE_OK && sel != nil)
		{
			selection = new VDB4DSelection(fManager, fBase, fTable, sel);
		}
		else
		{
			if (sel != nil) 
				sel->Release();
		}

		if (err == VE_OK && fic != nil)
		{
			record = new VDB4DRecord(fManager, fic, inContext);
		}
		else
		{
			if (fic != nil) 
				fic->Release();
		}
	}
	else
	{
		selection = SelectAllRecords( inContext, &err, DB4D_Do_Not_Lock, NULL);
		if ( (selection != nil) && (outFirstRecord != nil) && !selection->IsEmpty( inContext) )
		{
			record = selection->LoadSelectedRecord( 1, inLoadRecordReadOnly ? DB4D_Do_Not_Lock : DB4D_Keep_Lock_With_Record, inContext);
		}
	}
	
	if (outErr != nil)
		*outErr = err;

	if (outFirstRecord != nil)
		*outFirstRecord = record;
	else
		ReleaseRefCountable( &record);
		
	return selection;
}


CDB4DSelectionPtr VDB4DTable::SelectAllRecords(CDB4DBaseContextPtr inContext, VErrorDB4D* outErr, DB4D_Way_of_Locking HowToLock, CDB4DSet* outLockSet)
{
	Selection *sel;
	VDB4DSelection* result = nil;
	VError err;
	DataTable *DF;
	Bittab *tb;
	
	if (fTable->IsRemote())
	{
		xbox_assert( HowToLock == DB4D_Do_Not_Lock);	// other options are not implemented here
		
		sel = nil;

		IRequest* req = fTable->GetOwner()->CreateRequest(inContext, Req_SelectAllRecords + kRangeReqDB4D);
		if (req == nil)
			err = ThrowBaseError(memfull, noaction);
		else
		{
			req->PutBaseParam(fTable->GetOwner());
			req->PutThingsToForget( VDBMgr::GetManager(), ConvertContext(inContext));
			req->PutTableParam(fTable);

			uLONG loadOptions = 0;	// means we don't want the first record
			req->GetOutputStream()->Put( loadOptions);

			err = req->GetLastError();

			if (err == VE_OK)
			{
				err = req->Send();
				if (err== VE_OK)
					err = req->GetUpdatedInfo(fTable->GetOwner(), ConvertContext(inContext));
				if (err == VE_OK)
				{
					sel = req->RetainSelectionReply(fTable->GetOwner(), err, inContext);
				}
			}
			req->Release();
		}
	}
	else
	{
		BaseTaskInfoPtr context = nil;
		if (inContext != nil)
		{
			context = ConvertContext(inContext);
			assert(context->GetBase() == fTable->GetOwner());
		}

		Bittab* lockedset = nil;
		if (outLockSet != nil)
			lockedset = (VImpCreator<VDB4DSet>::GetImpObject(outLockSet))->GetBittab();

		DF=fTable->GetDF();
		sel=new BitSel(DF);
		if (sel==nil)
		{
			err = ThrowBaseError(memfull, DBaction_BuildingSelection);
		}
		else
		{
			if (sel->FixFic(DF->GetMaxRecords(context)))
			{
				tb=((BitSel*)sel)->GetTB();
				tb->ClearOrSetAll(true);
				err = DF->TakeOffBits(tb,context);
				if (err == VE_OK && HowToLock == DB4D_Keep_Lock_With_Transaction)
				{
					Transaction* trans = GetCurrentTransaction(context);
					if (trans != nil)
						err = trans->TryToLockSel(DF, tb, lockedset);
				}

			}
			else
			{
				err = ThrowBaseError(memfull, DBaction_BuildingSelection);
			}
		}
	}
	
	if (outErr != nil)
		*outErr = err;

	if (err == VE_OK && sel != nil)
	{
		result = new VDB4DSelection(fManager, fBase, fTable, sel);
	}
	else
	{
		if (sel != nil) 
			sel->Release();
	}

	return result;
}


CDB4DRecord *VDB4DTable::LoadRecord( RecIDType inRecordID, DB4D_Way_of_Locking HowToLock, CDB4DBaseContextPtr inContext, Boolean WithSubTable, Boolean* outLockWasKeptInTrans)
{
	FicheInMem *fiche = nil;
	BaseTaskInfoPtr context = nil;
	if (inContext != nil)
	{
		context = ConvertContext(inContext);
		assert(context->GetBase() == fTable->GetOwner());
	}

	if (testAssert( inRecordID != kDB4D_NullRecordID))
	{
		if (inRecordID == -2)
		{
			VError err;
			fiche = fTable->GetDF()->NewRecord(err, context);
			fiche->SetAllAlwaysNull();
		}
		else
		{
			if (fTable->IsRemote())
			{
				VError err;
				xbox_assert( outLockWasKeptInTrans == nil); // not implemented
				xbox_assert( (HowToLock == DB4D_Do_Not_Lock) || (HowToLock == DB4D_Keep_Lock_With_Record)); // other options not implemented

				IRequest* req = fTable->GetOwner()->CreateRequest(inContext, Req_LoadRecord + kRangeReqDB4D);
				if (req == nil)
					err = ThrowBaseError(memfull, noaction);
				else
				{
					req->PutBaseParam(fTable->GetOwner());
					req->PutThingsToForget( VDBMgr::GetManager(), context);
					req->PutTableParam(fTable);
					req->PutLongParam(inRecordID);

					uLONG loadOptions = (HowToLock == DB4D_Do_Not_Lock) ? 2 : 0;	// means we want read-only
					if (WithSubTable)
						loadOptions |= 4;
					req->GetOutputStream()->Put( loadOptions);

					err = req->GetLastError();
					if (err == VE_OK)
					{
						err = req->Send();

						if (err== VE_OK)
							err = req->GetUpdatedInfo(fTable->GetOwner(), context);

						if (err == VE_OK)
						{
							fiche = req->RetainFicheInMemReply(fTable, err, inContext);
						}
					}
					req->Release();
				}
			}
			else
			{
				fiche = LoadFicheInMem( inRecordID, HowToLock, context, WithSubTable, outLockWasKeptInTrans);
			}
		}
	}

	if (fiche == nil)
		return nil;
	CDB4DRecord* result = new VDB4DRecord( fManager, fiche, inContext);
	if (result == nil)
		fiche->Release();
	return result;
}


void VDB4DTable::WhoLockedRecord( RecIDType inRecordID, CDB4DBaseContextPtr inContext, DB4D_KindOfLock& outLockType, const VValueBag **outLockingContextRetainedExtraData) const
{
	BaseTaskInfoPtr context = nil;
	if (inContext != nil)
	{
		context = ConvertContext(inContext);
		assert(context->GetBase() == fTable->GetOwner());
	}

	if (testAssert( inRecordID != kDB4D_NullRecordID))
	{
		if (fTable->IsRemote())
		{
			DB4D_KindOfLock kindOfLock = DB4D_LockedByNone;
			const VValueBag *lockingContextRetainedExtraData = nil;

			VError err;

			IRequest* req = fTable->GetOwner()->CreateRequest(inContext, Req_WhoLockedRecord + kRangeReqDB4D);
			if (req == nil)
				err = ThrowBaseError(memfull, noaction);
			else
			{
				req->PutBaseParam(fTable->GetOwner());
				req->PutThingsToForget( VDBMgr::GetManager(), context);
				req->PutTableParam(fTable);
				req->PutLongParam(inRecordID);

				err = req->GetLastError();
				if (err == VE_OK)
				{
					err = req->Send();

					if (err== VE_OK)
						err = req->GetUpdatedInfo(fTable->GetOwner(), context);

					if (err == VE_OK)
					{
						kindOfLock = (DB4D_KindOfLock) req->GetLongReply(err);
						if (err == VE_OK)
							lockingContextRetainedExtraData = req->RetainValueBagReply( err);
					}
				}
				req->Release();
			}

			outLockType = kindOfLock;
			if (outLockingContextRetainedExtraData)
				*outLockingContextRetainedExtraData = lockingContextRetainedExtraData;
			else
				QuickReleaseRefCountable( lockingContextRetainedExtraData);
		}
		else
		{
			VError err = VE_OK;
			fTable->GetDF()->WhoLockedRecord( (sLONG) inRecordID, err, context, &outLockType, outLockingContextRetainedExtraData);
		}
	}
}


FicheInMem *VDB4DTable::LoadFicheInMem( RecIDType inRecordID, DB4D_Way_of_Locking HowToLock, BaseTaskInfoPtr context, Boolean WithSubTable, Boolean* outLockWasKeptInTrans)
{
	assert(!fTable->IsRemote());
	VError err = VE_OK;
	if (outLockWasKeptInTrans != nil)
	{
		*outLockWasKeptInTrans = false;
		Transaction* trans = GetCurrentTransaction(context);
		if (trans != nil)
			*outLockWasKeptInTrans = trans->WasLockKeptInThatTransaction(fTable->GetDF(), inRecordID);
	}
	return fTable->GetDF()->LoadRecord( (sLONG) inRecordID, err, HowToLock, context, WithSubTable);
}


void* VDB4DTable::LoadRawRecord(RecIDType inRecordID, DB4D_Way_of_Locking HowToLock, CDB4DBaseContextPtr inContext, 
								VErrorDB4D& outErr, Boolean* outCouldLock, Boolean* outLockWasKeptInTrans)
{
	if (fTable->IsRemote())
	{
		outErr = ThrowBaseError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
		return nil;
	}
	else
	{
		outErr = VE_OK;

		if (inRecordID == -2)
			return (void*)kNullRawRecord;
		else
		{
			BaseTaskInfoPtr context = nil;
			if (inContext != nil)
			{
				context = ConvertContext(inContext);
				assert(context->GetBase() == fTable->GetOwner());
			}

			if (outLockWasKeptInTrans != nil)
			{
				*outLockWasKeptInTrans = false;
				Transaction* trans = GetCurrentTransaction(context);
				if (trans != nil)
					*outLockWasKeptInTrans = trans->WasLockKeptInThatTransaction(fTable->GetDF(), inRecordID);
			}
			FicheOnDisk* result = fTable->GetDF()->LoadNotFullRecord(inRecordID, outErr, HowToLock, context, false, nil, outCouldLock, nil);
			return result;
		}
	}
}


VErrorDB4D VDB4DTable::DeleteRecord( RecIDType inRecordID, CDB4DBaseContextPtr inContext)
{
	VError err = VE_OK;

	if (fTable->IsRemote())
	{
		VError err;
		Boolean ok = false;
		Boolean legacycall = fTable->GetOwner()->IsThereATrigger(DB4D_DeleteRecord_Trigger, fTable, true);
		IRequest *req = fTable->GetOwner()->CreateRequest( inContext, Req_DeleteRecordByID + kRangeReqDB4D, legacycall);
		if (req == nil)
		{
			err = ThrowBaseError(memfull, noaction);
		}
		else
		{
			req->PutBaseParam( fTable->GetOwner());
			req->PutThingsToForget( VDBMgr::GetManager(), ConvertContext(inContext));
			req->PutTableParam( fTable);
			req->PutLongParam( inRecordID);

			err = req->GetLastError();
			if (err == VE_OK)
			{
				err = req->Send();
				if (err== VE_OK)
					err = req->GetUpdatedInfo(fTable->GetOwner(), ConvertContext(inContext));

			}
			req->Release();
		}
	}
	else
	{
		BaseTaskInfoPtr context = nil;
		if (inContext != nil)
		{
			context = ConvertContext(inContext);
			assert(context->GetBase() == fTable->GetOwner());
		}

		if (testAssert( inRecordID != kDB4D_NullRecordID)) {
			DataTable *df = fTable->GetDF();
			FicheInMem *f = df->LoadRecord( (sLONG) inRecordID, err, DB4D_Keep_Lock_With_Record, context, true);
			if (f != nil)
			{
				err = df->DelRecord( f, context);
				f->Release();
			}
			else
				err = VE_UNKNOWN_ERROR;
		}
	}
	return err;
}



FicheInMem *VDB4DTable::NewFicheInMem(BaseTaskInfoPtr context)
{
	VError err;
	FicheInMem *fic;
	if (fTable->IsRemote())
	{
		fic = new FicheInMem(context, fTable->GetOwner(), fTable, err);
/*
		if (fTable->AtLeastOneSubTable())
		{
			xbox_assert( false);
		}
*/
	}
	else
	{
		fic = fTable->GetDF()->NewRecord(err, context);
	}
	return fic;
}



CDB4DRecord *VDB4DTable::NewRecord(CDB4DBaseContextPtr inContext)
{
	BaseTaskInfoPtr context = nil;
	if (inContext != nil)
	{
		context = ConvertContext(inContext);
		assert(context->GetBase() == fTable->GetOwner());
	}

	FicheInMem *fiche = NewFicheInMem(context);
	if (fiche == nil)
		return nil;
	CDB4DRecord* result = new VDB4DRecord( fManager, fiche, inContext);
	if (result == nil)
		fiche->Release();
	return result;
}


RecIDType VDB4DTable::CountRecordsInTable(CDB4DBaseContextPtr inContext) const
{
	RecIDType count = 0;
	if (fTable->IsRemote())
	{
		VError err;
		IRequest* req = fTable->GetOwner()->CreateRequest(inContext, Req_CountRecordsInTable + kRangeReqDB4D);
		if (req == nil)
			err = ThrowBaseError(memfull, noaction);
		else
		{
			req->PutBaseParam(fTable->GetOwner());
			req->PutThingsToForget( VDBMgr::GetManager(), ConvertContext(inContext));
			req->PutTableParam(fTable);

			err = req->GetLastError();

			if (err == VE_OK)
			{
				err = req->Send();
				if (err== VE_OK)
					err = req->GetUpdatedInfo(fTable->GetOwner(), ConvertContext(inContext));
				if (err == VE_OK)
				{
					count = req->GetLongReply( err);
				}
			}
			req->Release();
		}
	}
	else
	{
		BaseTaskInfoPtr context = nil;
		if (inContext != nil)
		{
			context = ConvertContext(inContext);
			assert(context->GetBase() == fTable->GetOwner());
		}

		count = fTable->GetDF()->GetNbRecords(context);
	}

	return count;
}

#if 0
sLONG8 VDB4DTable::GetSequenceNumber(CDB4DBaseContextPtr inContext)
{
	if (fTable->IsRemote())
	{
		ThrowBaseError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
		return -1;
	}
	else
		return fTable->GetDF()->GetAutoSequence();
}
#endif


void VDB4DTable::GetFieldDefinition( DB4D_FieldID inFieldID, VString& outName, DB4DFieldType *outType, sLONG *outSize, 
									DB4DFieldAttributes *outAttributes, CDB4DBaseContextPtr inContext) const
{
	outName.SetNull();
	sLONG countFields = fTable->GetNbCrit();
	sLONG index = (sLONG) inFieldID;
	if (index >= 1 && index <= countFields) {
		FieldPtr thefield = fTable->RetainField( index);
		if (thefield == nil)
		{
			outName.Clear();
			if (outType != nil)
				*outType = DB4D_NoType;
		}
		else
		{
			CritereDISK *oneField = thefield->getCRD();
			
			outName.FromBlock( &oneField->nom[1], oneField->nom[0] * sizeof( UniChar), VTC_UTF_16);
	
			if (outType != nil)
			{
				if (oneField->typ == VK_UUID)
					*outType = VK_STRING;
				else
				{
					if (oneField->typ == VK_STRING_UTF8)
						*outType = VK_STRING;
					else
					{
						if (oneField->typ == VK_TEXT_UTF8)
							*outType = VK_TEXT;
						else
							*outType = oneField->typ;
					}
				}
			}
			
			if (outSize != nil)
				*outSize = oneField->LimitingLength;
	
			if (outAttributes != nil) {
				*outAttributes = 0;
				if (oneField->not_null)
					*outAttributes |= DB4D_Not_Null;
				if (oneField->unique)
					*outAttributes |= DB4D_Unique;
				if (oneField->fAutoSeq)
					*outAttributes |= DB4D_AutoSeq;
				if (oneField->fAutoGenerate)
					*outAttributes |= DB4D_AutoGenerate;
				if (oneField->typ == VK_STRING_UTF8 || oneField->typ == VK_TEXT_UTF8)
					*outAttributes |= DB4D_StoreAsUTF8;
				if (oneField->typ == VK_UUID)
					*outAttributes |= DB4D_StoreAsUUID;
			}
			
			thefield->Release();
		}
	}
}



CDB4DBase* VDB4DTable::GetOwner(CDB4DBaseContextPtr inContext)
{
	return fBase;
}


DB4D_FieldID VDB4DTable::GetFieldByName( const VString& inName, CDB4DBaseContextPtr inContext) const
{
	DB4D_FieldID id = kDB4D_NullFieldID;
	sLONG index = fTable->FindField( inName);
	if (index > 0)
		id = (DB4D_FieldID) index;
	return id;
}


VError VDB4DTable::AddField(const XBOX::VString& inName, DB4DFieldType inType, sLONG inSize, DB4DFieldAttributes inAttributes, 
							VDB4DProgressIndicator* InProgress, CDB4DBaseContextPtr inContext)
{
	VError err = VE_OK;
	
	ObjLocker locker(inContext, fTable);
	if (locker.CanWork())
	{
		fTable->AddField(inName, inType, inSize, inAttributes, err, inContext, InProgress);
		fTable->save();
	}
	else
		err = VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT;
	return err;
}


sLONG VDB4DTable::AddFields(const VValueBag& inFieldsDefinition, VError &err, VDB4DProgressIndicator* InProgress, CDB4DBaseContextPtr inContext)
{
	// retourne -1 si echec sinon l'index du premier champ rajoute
	
	err = VE_OK;
	sLONG firstAddedFieldNo = -1;
	
	ObjLocker locker(inContext, fTable);
	if (locker.CanWork())
	{
		const VBagArray *fields = inFieldsDefinition.RetainElements( L"field");
		if (fields != NULL)
		{
			VBagLoader loader( true, true);
		
			err = fTable->CreateFields( fields, &firstAddedFieldNo, &loader, inContext, InProgress);

			if (!fTable->IsRemote())
				fTable->save();

			fields->Release();
		}
	}
	else
		err = VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT;

	return (err == VE_OK) ? firstAddedFieldNo : -1;
}

	
VError VDB4DTable::SetFieldDefinition(DB4D_FieldID inFieldID, const VValueBag& inFieldDefinition, VDB4DProgressIndicator* InProgress, CDB4DBaseContextPtr inContext)
{
	ObjLocker locker(inContext, fTable);
	if (locker.CanWork())
	{
		return fTable->SetCrit((sLONG)inFieldID, inFieldDefinition, inContext, InProgress);
	}
	else
		return VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT;
}


VError VDB4DTable::SetFieldDefinition( DB4D_FieldID inFieldID, VString& inName, DB4DFieldType inType, sLONG inSize, DB4DFieldAttributes inAttributes, 
									  VDB4DProgressIndicator* InProgress, CDB4DBaseContextPtr inContext)
{
	VError err;
	
	// on ne fait que rappeler VDB4DTable::SetFieldDefinition avec unautre prototype donc pas de lock ici.

	Field* cri = fTable->RetainField(inFieldID);
	if (cri != nil)
	{
		VValueBag *fieldBag = new VValueBag;
		if (fieldBag != NULL)
		{
			fieldBag->SetString( DB4DBagKeys::name, inName);
			fieldBag->SetLong( DB4DBagKeys::type, (sLONG)inType);
			fieldBag->SetLong( DB4DBagKeys::limiting_length, inSize);
			fieldBag->SetBoolean( DB4DBagKeys::not_null, (inAttributes & DB4D_Not_Null) != 0);
			fieldBag->SetBoolean( DB4DBagKeys::unique, (inAttributes & DB4D_Unique) != 0);
			fieldBag->SetBoolean( DB4DBagKeys::autosequence, (inAttributes & DB4D_AutoSeq) != 0);
			fieldBag->SetBoolean( DB4DBagKeys::autogenerate, (inAttributes & DB4D_AutoGenerate) != 0);
			fieldBag->SetBoolean( DB4DBagKeys::store_as_utf8, (inAttributes & DB4D_StoreAsUTF8) != 0);
			fieldBag->SetBoolean( DB4DBagKeys::store_as_UUID, (inAttributes & DB4D_StoreAsUUID) != 0);
			fieldBag->SetBoolean( DB4DBagKeys::outside_blob, (inAttributes & DB4D_StoreOutsideBlob) != 0);
			
			err = SetFieldDefinition(inFieldID, *fieldBag, InProgress,inContext);
			fieldBag->Release();
		}
		else
			err = ThrowBaseError(memfull, DBaction_ChangingFieldProperties);

		cri->Release();
	}
	else
		err = ThrowBaseError(VE_DB4D_WRONGFIELDREF, DBaction_ChangingFieldProperties);

	return err;
}


CDB4DSortingCriterias* VDB4DTable::NewSortingCriterias()
{
	return new VDB4DSortingCriterias(GetManager(), fTable);
}


CDB4DImpExp* VDB4DTable::NewImpExp()
{
	return new VDB4DImpExp(this, fTable);
}


VError VDB4DTable::SetIdentifyingFields(const CDB4DFieldArray& inFields, VDB4DProgressIndicator* InProgress, CDB4DBaseContextPtr inContext)
{
	VError err = SetPrimaryKey(inFields, InProgress, true, inContext);

	return err;
}


VError VDB4DTable::SetPrimaryKey(const CDB4DFieldArray& inFields, VDB4DProgressIndicator* InProgress, Boolean CanReplaceExistingOne, CDB4DBaseContextPtr inContext, VString* inPrimaryKeyName)
{
	VError err = VE_OK;

	ObjLocker locker(inContext, fTable);
	if (locker.CanWork())
	{
		NumFieldArray fields;
		CDB4DFieldArray::ConstIterator cur = inFields.First(), end = inFields.End();
		for (;cur != end && err == VE_OK;cur++)
		{
			CDB4DField* xcri = *cur;
			if (xcri != nil)
			{
				Field* cri = (VImpCreator<VDB4DField>::GetImpObject(xcri))->GetField();
				if (cri != nil)
				{
					if (cri->GetOwner() == fTable)
					{
						if (!fields.Add(cri->GetPosInRec()))
							err = ThrowBaseError(memfull, DBaction_UpdatingTableDef);
					}
					else
						err = ThrowBaseError(VE_DB4D_WRONGTABLEREF, DBaction_UpdatingTableDef);
				}
				else
					err = ThrowBaseError(VE_DB4D_WRONGFIELDREF, DBaction_UpdatingTableDef);
			}
			else
				err = ThrowBaseError(VE_DB4D_WRONGFIELDREF, DBaction_UpdatingTableDef);
		}

		if (err == VE_OK)
		{
			err = fTable->SetPrimaryKey(fields, InProgress, CanReplaceExistingOne, inContext, inPrimaryKeyName);
		}
	}
	else
		err = ThrowBaseError(VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT, DBaction_UpdatingTableDef);
	return err;
}


VError VDB4DTable::RetainPrimaryKey(CDB4DFieldArray& outFields, CDB4DBaseContextPtr inContext) const
{
	FieldArray fields;

	VError err = fTable->RetainPrimaryKey(fields);
	outFields.SetCount(0);

	FieldArray::Iterator cur = fields.First(), end = fields.End();
	for (; cur != end; cur++)
	{
		CDB4DField* xcri = new VDB4DField(*cur);
		(*cur)->Release();
		if (xcri != nil)
		{
			if (!outFields.Add(xcri))
				err = ThrowBaseError(memfull, DBaction_AccessingFieldDef);
		}
		else
			err = ThrowBaseError(memfull, DBaction_AccessingFieldDef);
	}

	return err;
}


bool VDB4DTable::HasPrimaryKey( CDB4DBaseContextPtr inContext) const
{
	return fTable->HasPrimKey();
}


Boolean VDB4DTable::DropField(DB4D_FieldID inFieldID, VDB4DProgressIndicator* InProgress, CDB4DBaseContextPtr inContext)
{
	Boolean ok = false;
	ObjLocker locker(inContext, fTable);
	if (locker.CanWork())
	{
		Field* fld = fTable->RetainField(inFieldID);
		if (fld != nil) 
		{
			ObjLocker locker2(inContext, fld, &locker);
			if (locker2.CanWork())
			{
				ok = fTable->DeleteField(fld, inContext, InProgress);
			}
			fld->Release();
		}
	}
	return ok;
}


Boolean VDB4DTable::IsIndexable(DB4D_FieldID inFieldID, CDB4DBaseContextPtr inContext)
{
	Boolean res = false;
	Field* fld = fTable->RetainField(inFieldID);
	if (fld != nil) 
	{
		res = fld->IsIndexable();
		fld->Release();
	}
	
	return res;
}


Boolean VDB4DTable::IsIndexed(DB4D_FieldID inFieldID, CDB4DBaseContextPtr inContext)
{
	Boolean res = false;
	Field* fld = fTable->RetainField(inFieldID);
	if (fld != nil) 
	{
		res = fld->IsIndexe();
		fld->Release();
	}
	
	return res;
}


Boolean VDB4DTable::IsPrimIndexed(DB4D_FieldID inFieldID, CDB4DBaseContextPtr inContext)
{
	Boolean res = false;
	Field* fld = fTable->RetainField(inFieldID);
	if (fld != nil) 
	{
		res = fld->IsPrimIndexe();
		fld->Release();
	}
	
	return res;
}


RecIDType VDB4DTable::GetMaxRecordsInTable(CDB4DBaseContextPtr inContext) const
{
	if (fTable->IsRemote())
	{
		return fTable->GetRemoteMaxRecordsInTable();
	}
	else
	{
		BaseTaskInfoPtr context = nil;
		if (inContext != nil)
		{
			context = ConvertContext(inContext);
			assert(context->GetBase() == fTable->GetOwner());
		}
		return fTable->GetDF()->GetMaxRecords(context);
	}
}
	
	
Boolean VDB4DTable::LockTable(CDB4DBaseContextPtr inContext, Boolean WaitForEndOfRecordLocks, sLONG TimeToWaitForEndOfRecordLocks)
{
	if (fTable->IsRemote())
	{
		ThrowBaseError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
		return false;
	}
	else
	{
		BaseTaskInfoPtr context = nil;
		if (inContext != nil)
		{
			context = ConvertContext(inContext);
			assert(context->GetBase() == fTable->GetOwner());
		}
		return fTable->GetDF()->LockTable(context, WaitForEndOfRecordLocks, TimeToWaitForEndOfRecordLocks);
	}
}


void VDB4DTable::UnLockTable(CDB4DBaseContextPtr inContext)
{
	if (fTable->IsRemote())
	{
		ThrowBaseError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
	}
	else
	{
		BaseTaskInfoPtr context = nil;
		if (inContext != nil)
		{
			context = ConvertContext(inContext);
			assert(context->GetBase() == fTable->GetOwner());
		}
		fTable->GetDF()->UnLockTable(context);
	}
}


CDB4DSet* VDB4DTable::NewSet() const
{
	Bittab* b = new Bittab;
	if (b == nil)
		return nil;
	else
	{
		b->SetOwner(fTable->GetOwner());
		CDB4DSet* res = new VDB4DSet(fBase, fTable, b);
		b->Release();
		return res;
	}
}


CDB4DColumnFormula* VDB4DTable::NewColumnFormula() const
{
	return new VDB4DColumnFormula(fTable); 
}


CDB4DSelectionPtr VDB4DTable::NewSelection(DB4D_SelectionType inSelectionType) const
{
	Selection* sel = nil;
	CDB4DSelectionPtr result = nil;

	if (inSelectionType == DB4D_Sel_OneRecSel)
		inSelectionType = DB4D_Sel_SmallSel;

	switch (inSelectionType)
	{
		case DB4D_Sel_SmallSel:
			sel = new PetiteSel(fTable->GetDF(), fTable->GetOwner(), fTable->GetNum());
			break;

		case DB4D_Sel_LongSel:
			sel = new LongSel(fTable->GetDF(), fTable->GetOwner());
			((LongSel*)sel)->PutInCache();
			break;

		case DB4D_Sel_Bittab:
			sel = new BitSel(fTable->GetDF(), fTable->GetOwner());
			break;
	}

	if (sel != nil)
	{
		result = new VDB4DSelection(fManager, fBase, fTable, sel);
	}

	return result;
}


void VDB4DTable::GetUUID(VUUID& outID, CDB4DBaseContextPtr inContext) const
{
	fTable->GetUUID(outID);
}


CDB4DField* VDB4DTable::RetainNthField(sLONG inIndex, CDB4DBaseContextPtr inContext) const
{
	VDB4DField* result= nil;
	Field* cri = fTable->RetainField(inIndex);
	
	if (cri != nil)
	{
		result = new VDB4DField(cri);
		cri->Release();
	}

	return result;
}


CDB4DField* VDB4DTable::FindAndRetainField(const VString& inName, CDB4DBaseContextPtr inContext) const
{
	VDB4DField* result= nil;
	Field* cri = fTable->FindAndRetainFieldRef(inName);
	
	if (cri != nil)
	{
		result = new VDB4DField(cri);
		cri->Release();
	}

	return result;
}


CDB4DField* VDB4DTable::FindAndRetainField(const VUUID& inUUID, CDB4DBaseContextPtr inContext) const
{
	VDB4DField* result= nil;
	Field* cri = fTable->FindAndRetainFieldRef(inUUID);
	
	if (cri != nil)
	{
		result = new VDB4DField(cri);
		cri->Release();
	}

	return result;
}


Boolean VDB4DTable::Drop(VDB4DProgressIndicator* InProgress, CDB4DBaseContextPtr inContext)
{
	Boolean ok = false;
	ObjLocker locker(inContext, fTable);
	if (locker.CanWork())
	{
		ok = fTable->GetOwner()->DeleteTable(fTable, inContext);
	}
	return ok;
}


const VValueBag* VDB4DTable::RetainExtraProperties(VError &err, CDB4DBaseContextPtr inContext)
{
	assert(fTable != nil);
	return fTable->RetainExtraProperties(err, inContext);
}


VError VDB4DTable::SetExtraProperties(VValueBag* inExtraProperties, CDB4DBaseContextPtr inContext)
{
	assert(fTable != nil);
	ObjLocker locker(inContext, fTable);
	if (locker.CanWork())
	{
		return fTable->SetExtraProperties(inExtraProperties, true, inContext);
	}
	else
		return VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT;
}


CDB4DAutoSeqNumber* VDB4DTable::RetainAutoSeqNumber(CDB4DBaseContextPtr inContext)
{
	AutoSeqNumber* seq = fTable->GetSeqNum(inContext);
	if (seq == nil)
		return nil;
	else
		return new VDB4DAutoSeqNumber(seq);
}


VErrorDB4D VDB4DTable::GetListOfRelations(RelationArray& outRelations, Boolean AutoRelOnly, Boolean x_Nto1, CDB4DBaseContextPtr inContext)
{
	VError err = VE_OK;
	outRelations.SetCount(0);
	fTable->occupe();
	DepRelationArrayIncluded* rels = x_Nto1 ? fTable->GetRelNto1Deps() : fTable->GetRel1toNDeps();
	Relationptr *end = rels->End(), *p;

	BaseTaskInfoPtr context = nil;
	if (inContext != nil)
	{
		context = ConvertContext(inContext);
		assert(context->GetBase() == fTable->GetOwner());
	}

	for (p = rels->First(); p != end; p++)
	{
		Relationptr rel = *p;
		if (rel != nil)
		{
			Boolean ok;
			if (x_Nto1)
				ok = !AutoRelOnly || rel->IsAutoLoadNto1(context);
			else
				ok = !AutoRelOnly || rel->IsAutoLoad1toN(context);
			if (ok)
			{
				CDB4DRelation* xrel = new VDB4DRelation(fBase, rel);
				if (xrel != nil)
				{
					if (!outRelations.Add(xrel))
						err = ThrowBaseError(memfull, DBaction_BuildingListOfRelations);
				}
				else
					err = ThrowBaseError(memfull, DBaction_BuildingListOfRelations);
			}
		}
	}

	fTable->libere();

	return err;
}


VErrorDB4D VDB4DTable::GetListOfRelations1_To_N(RelationArray& outRelations, Boolean AutoRelOnly, CDB4DBaseContextPtr inContext)
{
	return GetListOfRelations(outRelations, AutoRelOnly, false, inContext);
}


VErrorDB4D VDB4DTable::GetListOfRelationsN_To_1(RelationArray& outRelations, Boolean AutoRelOnly, CDB4DBaseContextPtr inContext)
{
	return GetListOfRelations(outRelations, AutoRelOnly, true, inContext);
}



VErrorDB4D VDB4DTable::DelayIndexes()
{
	if (fTable->IsRemote())
	{
		//return ThrowBaseError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
		return VE_OK;
	}
	else
	{
		fTable->DelayIndexes();
		return VE_OK;
	}
}


VErrorDB4D VDB4DTable::AwakeIndexes(vector<CDB4DIndex*>& outIndexWithProblems, VDB4DProgressIndicator* inProgress, Boolean WaitForCompletion)
{
	if (fTable->IsRemote())
	{
	//	return ThrowBaseError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
		return VE_OK;
	}
	else
	{
		outIndexWithProblems.clear();
		vector<IndexInfo*> FullList;
		fTable->AwakeIndexes(inProgress, &FullList);
		if (WaitForCompletion)
		{
			VTask *currentTask = VTaskMgr::Get()->GetCurrentTask();
			while (StillIndexingOn(fTable->GetOwner()))
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
					CDB4DIndex* xind = new VDB4DIndex(ind);
					ind->Release();
					outIndexWithProblems.push_back(xind);
				}
			}
		}
		else
		{
			for (vector<IndexInfo*>::iterator cur = FullList.begin(), end = FullList.end(); cur != end; cur++)
				(*cur)->Release();
		}

		return VE_OK;
	}
}


sLONG VDB4DTable::GetStamp(CDB4DBaseContextPtr inContext) const
{
	return fTable->GetStamp();
}


void VDB4DTable::GetFieldsStamps(StampsVector& outStamps, CDB4DBaseContextPtr inContext) const
{
	fTable->GetFieldsStamps(outStamps);
}


VError VDB4DTable::RetainIndexes(ArrayOf_CDB4DIndex& outIndexes, CDB4DBaseContextPtr inContext)
{
	return fTable->RetainIndexes(outIndexes);
}


VErrorDB4D VDB4DTable::RetainExistingFields(vector<CDB4DField*>& outFields)
{
	return fTable->RetainExistingFields(outFields);
}

sLONG VDB4DTable::CountExistingFields()
{
	return fTable->CountExistingFields();
}


void VDB4DTable::UnlockRecordAfterRelease_IfNotModified(RecIDType inRecordID, CDB4DBaseContextPtr inContext)
{
	if (fTable->IsRemote())
	{
		ThrowBaseError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
	}
	else
	{
		if (fTable->GetDF() != nil)
		{
			BaseTaskInfoPtr context = nil;
			if (inContext != nil)
			{
				context = ConvertContext(inContext);
				assert(context->GetBase() == fTable->GetOwner());
			}
			fTable->GetDF()->UnlockRecord(inRecordID, context, DB4D_Keep_Lock_With_Transaction);
		}
	}
}


void VDB4DTable::UnlockRecordsAfterRelease_IfNotModified(const vector<RecIDType> &inRecordIDs, CDB4DBaseContextPtr inContext)
{
	if (fTable->IsRemote())
	{
		ThrowBaseError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE, noaction);
	}
	else
	{
		if (fTable->GetDF() != nil)
		{
			StErrorContextInstaller errs(false);

			BaseTaskInfoPtr context = nil;
			if (inContext != nil)
			{
				context = ConvertContext(inContext);
				assert(context->GetBase() == fTable->GetOwner());
			}

			Transaction* trans = GetCurrentTransaction(context);
			
			if (trans != nil)
			{
				Bittab b;
				if (inRecordIDs.size() != 0)
				{
					VError err = b.FillFromArray((const char*)&inRecordIDs[0], sizeof(RecIDType), (sLONG)inRecordIDs.size(), 0x7FFFFFFF, false);
					if (err == VE_OK)
					{
						trans->UnlockSel(fTable->GetDF(), &b, false);
					}
				}
			}
		}
	}
}


CDB4DFieldCacheCollection* VDB4DTable::NewFieldCacheCollection(const VString& Signature) const
{
	return new VDB4DFieldCacheCollection(fTable);
}


VError VDB4DTable::Truncate(CDB4DBaseContextPtr inContext, VDB4DProgressIndicator* inProgress)
{
	VError err = VE_OK;

	if (fTable->IsRemote())
	{
		IRequest *req = fTable->GetOwner()->CreateRequest( inContext, Req_TruncateTable + kRangeReqDB4D);
		if (req == nil)
		{
			err = ThrowBaseError(memfull, noaction);
		}
		else
		{
			req->PutBaseParam( fTable->GetOwner());
			req->PutThingsToForget( VDBMgr::GetManager(), ConvertContext(inContext));
			req->PutTableParam( fTable);
			req->PutProgressParam(inProgress);

			err = req->GetLastError();
			if (err == VE_OK)
			{
				err = req->Send();
				if (err== VE_OK)
					err = req->GetUpdatedInfo(fTable->GetOwner(), ConvertContext(inContext));

			}
			req->Release();
		}
	}
	else
	{
		bool mustunlock = false;
		err =  fTable->Truncate(ConvertContext(inContext), inProgress, false, mustunlock);
	}

	return err;
}


VError VDB4DTable::ActivateAutomaticRelations_N_To_1(CDB4DRecord* inRecord, vector<CachedRelatedRecord>& outResult, CDB4DBaseContext* InContext, const vector<uBYTE>& inWayOfLocking)
{
	if (fTable->IsRemote() && inRecord != nil)
	{
		VError err = VE_OK;
		IRequest *req = fTable->GetOwner()->CreateRequest( InContext, Req_ActivateAutomaticRelations_N_To_1 + kRangeReqDB4D);
		if (req == nil)
		{
			err = ThrowBaseError(memfull, noaction);
		}
		else
		{
			req->PutBaseParam( fTable->GetOwner());
			req->PutThingsToForget( VDBMgr::GetManager(), ConvertContext(InContext));
			req->PutTableParam( fTable);
			req->PutFicheInMemParam( VImpCreator<VDB4DRecord>::GetImpObject(inRecord)->GetRec(), InContext);
			req->GetOutputStream()->PutLong((sLONG)inWayOfLocking.size());
			if (!inWayOfLocking.empty())
				req->GetOutputStream()->PutData(&inWayOfLocking[0], inWayOfLocking.size());

			err = req->GetLastError();
			if (err == VE_OK)
			{
				err = req->Send();
				if (err== VE_OK)
					err = req->GetUpdatedInfo(fTable->GetOwner(), ConvertContext(InContext));

				if (err == VE_OK)
				{
					sLONG nb = req->GetLongReply(err);
					if (err == VE_OK)
					{
						outResult.reserve(nb);
						for (sLONG i = 0; i < nb && err == VE_OK; i++)
						{
							sLONG tablenum = req->GetLongReply(err);
							if (err == VE_OK)
							{
								CDB4DRecord* xrec = nil;
								CDB4DSelection* xsel = nil;
								Table* tt = fTable->GetOwner()->RetainTable(tablenum);
								uBYTE b;
								if (tt == nil)
									err = ThrowBaseError(VE_DB4D_WRONGTABLEREF);
								else
									b = req->GetByteReply(err);
								if (err == VE_OK)
								{
									if (b == '+')
									{
										FicheInMem* rec = req->RetainFicheInMemReply(tt, err, InContext);
										if (rec != nil)
										{
											xrec = new VDB4DRecord(VDBMgr::GetManager(), rec, InContext);
										}
									}
								}

								if (err == VE_OK)
								{
									Selection* sel = req->RetainSelectionReply(fTable->GetOwner(), err, InContext);
									if (sel != nil)
									{
										xsel = new VDB4DSelection(VDBMgr::GetManager(), fBase, tt, sel);				
									}
								}

								if (err == VE_OK)
								{
									outResult.resize( outResult.size()+1);
									outResult.back().SetTableNum( tablenum);
									outResult.back().AdoptRecord( xrec);
									outResult.back().AdoptSelection( xsel);
								}
								else
								{
									QuickReleaseRefCountable(xrec);
									QuickReleaseRefCountable(xsel);
								}

								QuickReleaseRefCountable(tt);
							}
						}
					}
				}
			}
			req->Release();
		}
		return err;
	}
	else
		return  fTable->ActivateAutomaticRelations_N_To_1((inRecord == nil) ? nil : VImpCreator<VDB4DRecord>::GetImpObject(inRecord)->GetRec(), outResult, ConvertContext(InContext), inWayOfLocking);
}


VError VDB4DTable::ActivateAutomaticRelations_1_To_N(CDB4DRecord* inRecord, vector<CachedRelatedRecord>& outResult, CDB4DBaseContext* InContext, 
													 const vector<uBYTE>& inWayOfLocking, CDB4DField* onOneFieldOnly, Boolean onOldvalue, Boolean AutomaticOnly, Boolean OneLevelOnly)
{
	if (fTable->IsRemote() && inRecord != nil)
	{
		VError err = VE_OK;
		IRequest *req = fTable->GetOwner()->CreateRequest( InContext, Req_ActivateAutomaticRelations_1_To_N + kRangeReqDB4D);
		if (req == nil)
		{
			err = ThrowBaseError(memfull, noaction);
		}
		else
		{
			req->PutBaseParam( fTable->GetOwner());
			req->PutThingsToForget( VDBMgr::GetManager(), ConvertContext(InContext));
			req->PutTableParam( fTable);
			req->PutFicheInMemParam( VImpCreator<VDB4DRecord>::GetImpObject(inRecord)->GetRec(), InContext);
			req->GetOutputStream()->PutLong((sLONG)inWayOfLocking.size());
			if (!inWayOfLocking.empty())
				req->GetOutputStream()->PutData(&inWayOfLocking[0], inWayOfLocking.size());
			if (onOneFieldOnly == nil)
				req->PutBooleanParam(false);
			else
			{
				req->PutBooleanParam(true);
				req->PutFieldParam(VImpCreator<VDB4DField>::GetImpObject(onOneFieldOnly)->GetField());
			}
			req->PutBooleanParam(onOldvalue);
			req->PutBooleanParam(AutomaticOnly);
			req->PutBooleanParam(OneLevelOnly);

			err = req->GetLastError();
			if (err == VE_OK)
			{
				err = req->Send();
				if (err== VE_OK)
					err = req->GetUpdatedInfo(fTable->GetOwner(), ConvertContext(InContext));

				if (err == VE_OK)
				{
					sLONG nb = req->GetLongReply(err);
					if (err == VE_OK)
					{
						outResult.reserve(nb);
						for (sLONG i = 0; i < nb && err == VE_OK; i++)
						{
							sLONG tablenum = req->GetLongReply(err);
							if (err == VE_OK)
							{
								CDB4DRecord* xrec = nil;
								CDB4DSelection* xsel = nil;
								Table* tt = fTable->GetOwner()->RetainTable(tablenum);
								uBYTE b;
								if (tt == nil)
									err = ThrowBaseError(VE_DB4D_WRONGTABLEREF);
								else
									b = req->GetByteReply(err);
								if (err == VE_OK)
								{
									if (b == '+')
									{
										FicheInMem* rec = req->RetainFicheInMemReply(tt, err, InContext);
										if (rec != nil)
										{
											xrec = new VDB4DRecord(VDBMgr::GetManager(), rec, InContext);
										}
									}
								}

								if (err == VE_OK)
								{
									Selection* sel = req->RetainSelectionReply(fTable->GetOwner(), err, InContext);
									if (sel != nil)
									{
										xsel = new VDB4DSelection(VDBMgr::GetManager(), fBase, tt, sel);				
									}
								}

								if (err == VE_OK)
								{
									outResult.resize( outResult.size()+1);
									outResult.back().SetTableNum( tablenum);
									outResult.back().AdoptRecord( xrec);
									outResult.back().AdoptSelection( xsel);
								}
								else
								{
									QuickReleaseRefCountable(xrec);
									QuickReleaseRefCountable(xsel);
								}
								QuickReleaseRefCountable(tt);
							}
						}
					}
				}
			}
			req->Release();
		}
		return err;
	}
	else
		return  fTable->ActivateAutomaticRelations_1_To_N((inRecord == nil) ? nil : VImpCreator<VDB4DRecord>::GetImpObject(inRecord)->GetRec(), outResult, 
															ConvertContext(InContext), inWayOfLocking, 
															onOneFieldOnly == nil ? nil : VImpCreator<VDB4DField>::GetImpObject(onOneFieldOnly)->GetField(), onOldvalue,
															AutomaticOnly, OneLevelOnly);
}


VErrorDB4D VDB4DTable::ActivateAllAutomaticRelations(CDB4DRecord* inRecord, std::vector<CachedRelatedRecord>& outResult, 
												 CDB4DBaseContext* InContext, const std::vector<uBYTE>& inWayOfLocking)
{
	VError err = VE_OK;

	if (fTable->IsRemote() && inRecord != nil)
	{
		IRequest *req = fTable->GetOwner()->CreateRequest( InContext, Req_ActivateAllAutomaticRelations + kRangeReqDB4D);
		if (req == nil)
		{
			err = ThrowBaseError(memfull, noaction);
		}
		else
		{
			req->PutBaseParam( fTable->GetOwner());
			req->PutThingsToForget( VDBMgr::GetManager(), ConvertContext(InContext));
			req->PutTableParam( fTable);
			req->PutFicheInMemParam( VImpCreator<VDB4DRecord>::GetImpObject(inRecord)->GetRec(), InContext);
			req->GetOutputStream()->PutLong((sLONG)inWayOfLocking.size());
			if (!inWayOfLocking.empty())
				req->GetOutputStream()->PutData(&inWayOfLocking[0], inWayOfLocking.size());

			err = req->GetLastError();
			if (err == VE_OK)
			{
				err = req->Send();
				if (err== VE_OK)
					err = req->GetUpdatedInfo(fTable->GetOwner(), ConvertContext(InContext));

				if (err == VE_OK)
				{
					sLONG nb = req->GetLongReply(err);
					if (err == VE_OK)
					{
						outResult.resize(nb);
						for (sLONG i = 0; i < nb && err == VE_OK; i++)
						{
							CachedRelatedRecord* p = &outResult[i];
							CDB4DRecord* xrec = nil;
							CDB4DSelection* xsel = nil;
							sLONG tablenum = req->GetLongReply(err);
							if (err == VE_OK)
							{
								Table* tt = fTable->GetOwner()->RetainTable(tablenum);
								uBYTE b;
								if (tt == nil)
									err = ThrowBaseError(VE_DB4D_WRONGTABLEREF);
								else
									b = req->GetByteReply(err);
								if (err == VE_OK)
								{
									if (b == '+')
									{
										{
											FicheInMem* rec = req->RetainFicheInMemReply(tt, err, InContext);
											if (rec != nil)
											{
												xrec = new VDB4DRecord(VDBMgr::GetManager(), rec, InContext);
											}
										}
									}
								}

								if (err == VE_OK)
								{
									Selection* sel = req->RetainSelectionReply(fTable->GetOwner(), err, InContext);
									if (sel != nil)
									{
										xsel = new VDB4DSelection(VDBMgr::GetManager(), fBase, tt, sel);				
									}
								}
								QuickReleaseRefCountable(tt);
							}

							if (err == VE_OK)
							{
								p->SetTableNum( tablenum);
								p->AdoptRecord( xrec);
								p->AdoptSelection( xsel);
							}
						}
					}
				}
			}
			req->Release();
		}
	}
	else
	{
		vector<CachedRelatedRecord> result1, result2;
		err = ActivateAutomaticRelations_N_To_1(inRecord, result1, InContext, inWayOfLocking);
		if (err == VE_OK)
		{
			err = ActivateAutomaticRelations_1_To_N(inRecord, result2, InContext, inWayOfLocking, nil, false, true, false);
			if (err == VE_OK)
			{
				outResult.resize(result1.size()+result2.size());
				vector<CachedRelatedRecord>::iterator target = outResult.begin();

				for (vector<CachedRelatedRecord>::iterator cur = result1.begin(), end = result1.end(); cur != end; cur++, target++)
					(*target).Steal(*cur);

				for (vector<CachedRelatedRecord>::iterator cur = result2.begin(), end = result2.end(); cur != end; cur++, target++)
					(*target).Steal(*cur);
			}
		}
	}
	return err;
}



VErrorDB4D VDB4DTable::DataToCollection(const vector<RecIDType>& inRecIDs, DB4DCollectionManager& Collection, CDB4DBaseContextPtr inContext, sLONG FromRecInSel, sLONG ToRecInSel,
									VDB4DProgressIndicator* InProgress)
{
	VErrorDB4D err = VE_OK;
	if (fTable->IsRemote())
	{
		err = ThrowBaseError(VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE);
	}
	else
	{
		if (fTable->GetDF() != nil)
		{
			ConstVectorSel SEL(fTable->GetDF(), inRecIDs);
			bool fullycompleted = true;
			sLONG maxelems;
			err = fTable->GetDF()->DataToCollection(&SEL, Collection, FromRecInSel, ToRecInSel, ConvertContext(inContext), InProgress, false, fullycompleted, maxelems, false);
		}
		else
			err = ThrowBaseError(VE_DB4D_WRONGTABLEREF);
	}

	return err;
}


VErrorDB4D VDB4DTable::SetKeepRecordStamps(bool inKeepRecordStamps, CDB4DBaseContextPtr inContext, VDB4DProgressIndicator* InProgress)
{
	return fTable->SetKeepStamp( inContext, inKeepRecordStamps, InProgress);
}

bool VDB4DTable::GetKeepRecordStamps() const
{
	return fTable->GetKeepStamp();
}


#if AllowSyncOnRecID
VError VDB4DTable::GetModificationsSinceStamp(uLONG stamp, VStream& outStream, uLONG& outLastStamp, sLONG& outNbRows, CDB4DBaseContextPtr inContext, vector<sLONG>& cols)
{
	if (fTable->IsRemote())
	{
		return ThrowBaseError(VE_DB4D_NOTIMPLEMENTED);
	}
	else
		return fTable->GetDF()->GetModificationsSinceStamp(stamp, outStream, outLastStamp, outNbRows, ConvertContext(inContext), cols);
}


VError VDB4DTable::IntegrateModifications(VStream& inStream, CDB4DBaseContextPtr inContext, vector<sLONG>& cols)
{
	if (fTable->IsRemote())
	{
		return ThrowBaseError(VE_DB4D_NOTIMPLEMENTED);
	}
	else
		return fTable->GetDF()->IntegrateModifications(inStream, ConvertContext(inContext), cols);
}


VError VDB4DTable::GetOneRow(VStream& inStream, CDB4DBaseContextPtr inContext, sBYTE& outAction, uLONG& outStamp, RecIDType& outRecID, vector<VValueSingle*>& outValues)
{
	return fTable->GetOneRow(inStream, ConvertContext(inContext), outAction, outStamp, outRecID, outValues);
}
#endif


CDB4DEntityModel* VDB4DTable::BuildEntityModel()
{
#if BuildEmFromTable
	return EntityModel::BuildLocalEntityModel(fTable, (LocalEntityModelCatalog*)(fTable->GetOwner()->GetEntityCatalog(false)));
#else
	return nil;
#endif
}


VErrorDB4D VDB4DTable::SetKeepRecordSyncInfo(bool inKeepRecordSyncInfo, CDB4DBaseContextPtr inContext, VDB4DProgressIndicator* InProgress)
{
	return fTable->SetKeepRecordSyncInfo( inContext, inKeepRecordSyncInfo, InProgress);
}

bool VDB4DTable::GetKeepRecordSyncInfo() const
{
	return fTable->GetKeepRecordSyncInfo();
}



VErrorDB4D VDB4DTable::SetHideInRest(bool x, CDB4DBaseContextPtr inContext, VDB4DProgressIndicator* InProgress)
{
	return fTable->SetHideInRest( inContext, x, InProgress);
}

bool VDB4DTable::IsHiddenInRest(CDB4DBaseContextPtr inContext) const
{
	return fTable->GetHideInRest();
}


VErrorDB4D VDB4DTable::GetModificationsSinceStampWithPrimKey(uLONG8 stamp, VStream& outStream, uLONG8& outLastStamp, sLONG& outNbRows, 
														 CDB4DBaseContextPtr inContext, vector<sLONG>& cols,
														 CDB4DSelection* filter, sLONG8 skip,sLONG8 top,
														 std::vector<XBOX::VString*>* inImageFormats)
{
	if (fTable->IsRemote())
	{
		return ThrowBaseError(VE_DB4D_NOTIMPLEMENTED);
	}
	else
	{
		Selection* selfilter = nil;
		if (filter != nil)
			selfilter = VImpCreator<VDB4DSelection>::GetImpObject(filter)->GetSel();
		return fTable->GetDF()->GetModificationsSinceStamp(stamp, outStream, outLastStamp, outNbRows, ConvertContext(inContext), cols, selfilter, skip, top, inImageFormats);
	}
}


VErrorDB4D VDB4DTable::IntegrateModificationsWithPrimKey(VStream& inStream, CDB4DBaseContextPtr inContext, vector<sLONG>& cols, bool sourceOverDest,
															uLONG8& ioFirstDestStampToCheck, uLONG8& outErrorStamp, bool inBinary)
{
	if (fTable->IsRemote())
	{
		return ThrowBaseError(VE_DB4D_NOTIMPLEMENTED);
	}
	else
		return fTable->GetDF()->IntegrateModificationsWithPrimKey(inStream, ConvertContext(inContext), cols, sourceOverDest, ioFirstDestStampToCheck, outErrorStamp, inBinary);
}


VErrorDB4D VDB4DTable::GetOneRowWithPrimKey(VStream& inStream, CDB4DBaseContextPtr inContext, sBYTE& outAction, uLONG8& outStamp, VTime& outTimeStamp, 
										vector<VValueSingle*>& outPrimKey, vector<VValueSingle*>& outValues)
{
	ReplicationInputBinaryFormatter		irFormatter ( inStream );

	return fTable->GetOneRow(irFormatter, ConvertContext(inContext), outAction, outStamp, outTimeStamp, outPrimKey, outValues);
}


VError VDB4DTable::GetFragmentation(sLONG8& outTotalRec, sLONG8& outFrags, CDB4DBaseContext* inContext)
{
	return fTable->GetFragmentation(outTotalRec, outFrags, inContext);
}


VError VDB4DTable::ImportRecords(CDB4DBaseContext* inContext, VFolder* inFolder, VDB4DProgressIndicator* inProgress, ExportOption& options)
{
	return fTable->ImportRecords(inFolder, ConvertContext(inContext), inProgress, options);
}

CDB4DField* VDB4DTable::RetainPseudoField(DB4D_Pseudo_Field_Kind kind)
{
	VDB4DField* result= nil;
	Field* cri = fTable->RetainPseudoField(kind);

	if (cri != nil)
	{
		result = new VDB4DField(cri);
		cri->Release();
	}

	return result;
}


VErrorDB4D VDB4DTable::GetListOfTablesForCascadingDelete(std::set<sLONG>& outSet)
{
	return fTable->GetListOfTablesForCascadingDelete(outSet);
}


sLONG VDB4DTable::GetSeqRatioCorrector() const
{
	return (sLONG)fTable->GetSeqRatioCorrector();
}

void VDB4DTable::SetSeqRatioCorrector(sLONG newValue)
{
	DataTable* df = fTable->GetDF();
	if (df != nil)
		df->SetSeqRatioCorrector(newValue);
}




//=================================================================================




VDB4DQuery::VDB4DQuery( VDBMgr* manager, Table *inTable, CDB4DTable* owner, EntityModel* inModel):fQuery( inTable, true)
{
	fManager = manager;
	Owner = owner;
	fQuery.GetTargetFile()->Retain();
	fQuery.SetModel(inModel);
	if (Owner != nil)
		Owner->Retain();
	fModel = inModel;
}


VDB4DQuery::~VDB4DQuery()
{
	if (Owner != nil) 
		Owner->Release();
	fQuery.GetTargetFile()->Release();
}


Boolean VDB4DQuery::AddCriteria( DB4D_TableID inTableID, DB4D_FieldID inFieldID, DB4DComparator inComparator, const VValueSingle& inValue, Boolean inDiacritic)
{
	// il faudrait mieux tester la validite des id passes en parametres
	
	Boolean isOK = false;
	if (inTableID == kDB4D_NullTableID)
		inTableID = (DB4D_TableID) fQuery.GetTargetFile()->GetNum();
	if (testAssert( inFieldID != kDB4D_NullFieldID)) {
		/*
		ValPtr cv = (ValPtr)(inValue.Clone());
		if (cv != nil)
		*/
		{
			fQuery.AddSearchLineSimple( (sLONG) inTableID, (sLONG) inFieldID, (sLONG) inComparator, &inValue, inDiacritic);
			isOK = true;
		}
	}
	return isOK;
}


Boolean VDB4DQuery::AddLogicalOperator( DB4DConjunction inConjunction)
{
	fQuery.AddSearchLineBoolOper( (sLONG) inConjunction);
	return true;
}

Boolean VDB4DQuery::AddCriteria( const VString& inTableName, const VString& inFieldName, DB4DComparator inComparator, const VValueSingle& inValue, Boolean inDiacritic)
{
	DB4D_TableID idTable = kDB4D_NullTableID;
	DB4D_FieldID idField = kDB4D_NullFieldID;
	
	if (inTableName.GetLength() == 0) {
		sLONG indexField = fQuery.GetTargetFile()->FindField( inFieldName);
		if (indexField > 0)
			idField = (DB4D_FieldID) indexField;
	} else {
		Base4D *base = fQuery.GetTargetFile()->GetDF()->GetDB();
		sLONG indexTable, indexField;
		base->FindField( inTableName, inFieldName, &indexTable, &indexField);
		if (indexTable != 0) 
		{
			idTable = (DB4D_TableID) indexTable;
			if (indexField > 0)
				idField = (DB4D_FieldID) indexField;
		}
	}
	if (idTable == kDB4D_NullTableID || idField == kDB4D_NullFieldID)
		return false;
	
	return AddCriteria( idTable, idField, inComparator, inValue, inDiacritic);
}


Boolean VDB4DQuery::AddCriteria( CDB4DField* inField, DB4DComparator inComparator, const XBOX::VValueSingle& inValue, Boolean inDiacritic)
{
	Field* f = VImpCreator<VDB4DField>::GetImpObject(inField)->GetField();
	if (f != nil)
	{
		fQuery.AddSearchLineSimple(f, inComparator, &inValue, inDiacritic);
		return true;
	}
	else return false;
}


Boolean VDB4DQuery::AddEmCriteria(const VString& inAttributePath, DB4DComparator inComparator, const VValueSingle& inValue, Boolean inDiacritic)
{
	fQuery.AddSearchLineEm(inAttributePath, inComparator, &inValue, inDiacritic);
	return true;
}



Boolean VDB4DQuery::AddCriteria( CDB4DField* inField, DB4DComparator inComparator, DB4DArrayOfValues *inValue, Boolean inDiacritic)
{
	Field* f = VImpCreator<VDB4DField>::GetImpObject(inField)->GetField();
	if (f != nil)
	{
		fQuery.AddSearchLineArray(f, inComparator, inValue, inDiacritic);
		return true;
	}
	else return false;	
}


Boolean VDB4DQuery::AddCriteria( CDB4DField* inField, DB4DComparator inComparator, CDB4DField* inFieldToCompareTo, Boolean inDiacritic)
{
	Field* f = VImpCreator<VDB4DField>::GetImpObject(inField)->GetField();
	Field* f2 = VImpCreator<VDB4DField>::GetImpObject(inFieldToCompareTo)->GetField();
	if (f != nil && f2 != nil)
	{
		fQuery.AddSearchLineJoin(f, inComparator, f2, inDiacritic);
		return true;
	}
	else return false;	
}


Boolean VDB4DQuery::AddExpression(DB4DLanguageExpression* inExpression, DB4D_TableID inTableID)
{
	fQuery.AddSearchLineExpression(inExpression, inTableID);
	return true;
}


void VDB4DQuery::SetDisplayProperty(Boolean inDisplay)
{
	fQuery.SetDisplayTree(inDisplay);
}


Boolean VDB4DQuery::AddNotOperator()
{
	fQuery.AddSearchLineNotOperator();
	return true;
}


Boolean VDB4DQuery::OpenParenthesis()
{
	fQuery.AddSearchLineOpenParenthesis();
	return true;
}


Boolean VDB4DQuery::CloseParenthesis()
{
	fQuery.AddSearchLineCloseParenthesis();
	return true;
}


CDB4DTable* VDB4DQuery::GetTargetTable()
{
	if (Owner == nil)
	{
		/*assert(fModel != nil);
		Owner = fModel->RetainTable();
		*/
		return nil;
	}
	return Owner;
}


VError VDB4DQuery::BuildFromString(const VString& inQueryText, VString& outOrderby, CDB4DBaseContext* context, CDB4DEntityModel* inModel, const QueryParamElementVector* params)
{
	return fQuery.BuildFromString(inQueryText, outOrderby, ConvertContext(context), (inModel == nil) ? nil : VImpCreator<EntityModel>::GetImpObject(inModel), false, (QueryParamElementVector*)params);
}


CDB4DQueryPathNode* VDB4DQuery::BuildQueryPath(Boolean inSubSelection, const CDB4DQueryPathModifiers* inModifiers, CDB4DBaseContextPtr inContext)
{
	OptimizedQuery rech;

	BaseTaskInfoPtr context = nil;
	if (inContext != nil)
	{
		context = ConvertContext(inContext);
	}

	VError err = rech.AnalyseSearch(GetSearchTab(), context);

	CDB4DQueryPathNode* queryplan = rech.BuildQueryPath(inSubSelection, inModifiers);

	return queryplan;
}


Boolean VDB4DQuery::WithFormulas() const
{
	return fQuery.WithFormulas();
}


VError VDB4DQuery::GetParams(vector<VString>& outParamNames, QueryParamElementVector& outParamValues)
{
	return fQuery.GetParams(outParamNames, outParamValues);
}

VError VDB4DQuery::SetParams(const vector<VString>& inParamNames, const QueryParamElementVector& inParamValues)
{
	return fQuery.SetParams(inParamNames, inParamValues);
}



//=================================================================================


VDB4DComplexSelection::~VDB4DComplexSelection()
{
	if (fSel != nil)
		fSel->Release();
}


RecIDType VDB4DComplexSelection::GetRecID(sLONG Column, RecIDType Row, VErrorDB4D& outErr)
{
	if (fSel == nil)
	{
		return -1;
		outErr = VE_DB4D_SELECTION_IS_NULL;
	}
	else
		return fSel->GetRecID(Column, Row, outErr);
}


VErrorDB4D VDB4DComplexSelection::GetFullRow(RecIDType Row, ComplexSelRow& outRow)
{
	if (fSel == nil)
		return VE_DB4D_SELECTION_IS_NULL;
	else
		return fSel->GetFullRow(Row, outRow);
}


CDB4DComplexSelection* VDB4DComplexSelection::And(CDB4DComplexSelection* inOther, VErrorDB4D& outErr)
{
	VDB4DComplexSelection* other = VImpCreator<VDB4DComplexSelection>::GetImpObject(inOther);

	if (fSel == nil || other->fSel == nil)
	{
		outErr = VE_DB4D_SELECTION_IS_NULL;
		return nil;
	}
	else
	{
		VRefPtr<ComplexSelection> result(fSel->MergeSel(*(other->fSel), outErr), false);
		if (result == nil)
			return nil;
		else
		{
			CDB4DComplexSelection* xresult = new VDB4DComplexSelection(fManager, result);
			if (xresult == nil)
				outErr = ThrowBaseError(memfull, DBaction_BuildingSelection);
			return xresult;
		}
	}
}


CDB4DComplexSelection* VDB4DComplexSelection::Or(CDB4DComplexSelection* inOther, VErrorDB4D& outErr)
{
	outErr = VE_DB4D_NOTIMPLEMENTED;
	return nil;
}


CDB4DComplexSelection* VDB4DComplexSelection::Minus(CDB4DComplexSelection* inOther, VErrorDB4D& outErr)
{
	outErr = VE_DB4D_NOTIMPLEMENTED;
	return nil;
}


VErrorDB4D VDB4DComplexSelection::SortByRecIDs(const vector<sLONG>& onWhichColums)
{
	if (fSel == nil)
	{
		return VE_DB4D_SELECTION_IS_NULL;
	}
	else
		return fSel->SortByRecIDs(onWhichColums);
}


sLONG VDB4DComplexSelection::CountRows()
{
	if (fSel == nil)
		return 0;
	else
		return fSel->GetNbRows();
}


VErrorDB4D VDB4DComplexSelection::AddRow(const ComplexSelRow& inRow)
{
	if (fSel == nil)
		return VE_DB4D_SELECTION_IS_NULL;
	else
		return fSel->AddRow(inRow);
}


sLONG VDB4DComplexSelection::PosOfTarget(const QueryTarget& inTarget)
{
	if (fSel == nil || inTarget.first == nil)
		return -1;
	else
	{
		CQTableDef t1(VImpCreator<VDB4DTable>::GetImpObject(inTarget.first)->GetTable(), inTarget.second);
		return fSel->FindTarget(t1);
	}
}


sLONG VDB4DComplexSelection::CountColumns()
{
	if (fSel == nil)
		return 0;
	else
		return fSel->GetNbColumns();
}


VErrorDB4D VDB4DComplexSelection::RetainColumns(QueryTargetVector& outColumns)
{
	if (fSel == nil)
		return VE_DB4D_SELECTION_IS_NULL;
	else
	{
		return fSel->RetainColumns(outColumns);
	}
}


VErrorDB4D VDB4DComplexSelection::RetainColumn(sLONG Column, QueryTarget& outTarget)
{
	if (fSel == nil)
		return VE_DB4D_SELECTION_IS_NULL;
	else
	{
		CQTableDef xx;
		VError err = fSel->GetColumn(Column, xx);
		if (err == VE_OK)
		{
			CDB4DBase* base = xx.GetTable()->GetOwner()->RetainBaseX();
			CDB4DTable* theTable = new VDB4DTable( fManager, VImpCreator<VDB4DBase>::GetImpObject(base), xx.GetTable());
			base->Release();
			outTarget.first = theTable;
			outTarget.second = xx.GetInstance();
		}
		return err;
	}
}


VErrorDB4D VDB4DComplexSelection::ToDB4D_ComplexSel(DB4D_ComplexSel& outSel)
{
	if (fSel == nil)
		return VE_DB4D_SELECTION_IS_NULL;
	else
	{
		return fSel->ToDB4D_ComplexSel(outSel);
	}
}






//=================================================================================



void VDB4DComplexQuery::SetSimpleTarget(CDB4DTable* inTarget)
{
	fQuery.SetSimpleTarget(VImpCreator<VDB4DTable>::GetImpObject(inTarget)->GetTable());
}


VError VDB4DComplexQuery::SetComplexTarget(const QueryTargetVector& inTargets)
{
	return fQuery.SetComplexTarget(inTargets);
}


VError VDB4DComplexQuery::AddCriteria( CDB4DField* inField, sLONG Instance, DB4DComparator inComparator, const XBOX::VValueSingle& inValue, Boolean inDiacritic)
{
	return fQuery.AddSearchSimple(VImpCreator<VDB4DField>::GetImpObject(inField)->GetField(), Instance, inComparator, &inValue, inDiacritic);
}


VError VDB4DComplexQuery::AddCriteria( CDB4DField* inField, sLONG Instance, DB4DComparator inComparator, DB4DArrayOfValues *inValue, Boolean inDiacritic)
{
	return fQuery.AddSearchArray(VImpCreator<VDB4DField>::GetImpObject(inField)->GetField(), Instance, inComparator, inValue, inDiacritic);
}


VError VDB4DComplexQuery::AddJoin( CDB4DField* inField1, sLONG Instance1, DB4DComparator inComparator, CDB4DField* inField2, sLONG Instance2, 
								  Boolean inDiacritic, bool inLeftJoin, bool inRightJoin)
{
	return fQuery.AddSearchJoin(VImpCreator<VDB4DField>::GetImpObject(inField1)->GetField(), Instance1, inComparator,
								 VImpCreator<VDB4DField>::GetImpObject(inField2)->GetField(), Instance2, inDiacritic, inLeftJoin, inRightJoin);
}


VError VDB4DComplexQuery::AddExpression(CDB4DTable* inTable, sLONG Instance, DB4DLanguageExpression* inExpression)
{
	return fQuery.AddSearchExpression(inExpression, VImpCreator<VDB4DTable>::GetImpObject(inTable)->GetTable(), Instance);
}


VError VDB4DComplexQuery::AddSQLExpression(CDB4DTable* inTable, sLONG Instance, DB4DSQLExpression* inExpression)
{
	return fQuery.AddSearchSQLExpression(inExpression, VImpCreator<VDB4DTable>::GetImpObject(inTable)->GetTable(), Instance);
}


VError VDB4DComplexQuery::AddSQLExpression(const QueryTargetVector& inTargets, DB4DSQLExpression* inExpression)
{
	return fQuery.AddSearchComplexSQLExpression(inExpression, inTargets);
}


VError VDB4DComplexQuery::AddLogicalOperator( DB4DConjunction inConjunction)
{
	return fQuery.AddSearchBoolOper((sLONG)inConjunction);
}


VError VDB4DComplexQuery::AddNotOperator()
{
	return fQuery.AddSearchNotOperator();
}


VError VDB4DComplexQuery::OpenParenthesis()
{
	return fQuery.AddSearchOpenParenthesis();
}


VError VDB4DComplexQuery::CloseParenthesis()
{
	return fQuery.AddSearchCloseParenthesis();
}


VError VDB4DComplexQuery::BuildFromString(const VString& inQueryText)
{
	return fQuery.BuildFromString(inQueryText);
}


VError VDB4DComplexQuery::BuildTargetsFromString(const VString& inTargetsText)
{
	return fQuery.BuildTargetsFromString(inTargetsText);
}



VError VDB4DComplexQuery::BuildQueryDescriptor(VString& outDescription)
{
	ComplexOptimizedQuery oq;
	VError err = oq.AnalyseSearch(&fQuery, nil);
	if (err == VE_OK)
		err = oq.BuildQueryDescriptor(outDescription);

	return err;
}


//=================================================================================



VDB4DSortingCriterias::VDB4DSortingCriterias( VDBMgr* manager, Table *inTable):fSortTab(inTable->GetOwner())
{
	fManager = manager;
	fTable = inTable;
	fTable->Retain();
}


VDB4DSortingCriterias::~VDB4DSortingCriterias()
{
	fTable->Release();
}


Boolean VDB4DSortingCriterias::AddCriteria(CDB4DField* inField, Boolean inAscending)
{
	VError err;
	assert(inField != nil);
	if (inField != nil)
	{
		sLONG inTableID = VImpCreator<VDB4DField>::GetImpObject(inField)->GetField()->GetOwner()->GetNum();
		err = fSortTab.AddTriLineField(inTableID, inField->GetID(), inAscending);
	}
	else
		err = VE_DB4D_WRONGFIELDREF;

	return (err == VE_OK);
}


Boolean VDB4DSortingCriterias::AddCriteria(DB4D_FieldID inFieldID, Boolean inAscending)
{
	sLONG inTableID = fTable->GetNum();
	VError err = fSortTab.AddTriLineField(inTableID, (sLONG)inFieldID, inAscending);
	
	return (err == VE_OK);
}


Boolean VDB4DSortingCriterias::AddExpression(DB4DLanguageExpression* inExpression, Boolean inAscending)
{
	return fSortTab.AddExpression(inExpression, inAscending);
}



//=================================================================================


VDB4DSqlQuery::VDB4DSqlQuery( VDBMgr *inManager, Base4D* WhichBase, BaseTaskInfo* WhichContext, VString& request, VError& err)
{
	fManager = inManager;
//	fQuery = new SqlQuery(WhichBase);
	fOwner = WhichContext;
	fOwner->Retain();
	/*
	if (fQuery != nil)
	{
		lasterr = fQuery->BuildFromText(&request);
	}
	else lasterr = memfull;
	*/
	err = lasterr;
}


VDB4DSqlQuery::~VDB4DSqlQuery()
{
//	if (fQuery != nil) delete fQuery;
	fOwner->Release();
}


VError VDB4DSqlQuery::ExecSql(CDB4DSelection* &outSelection, VDB4DProgressIndicator* InProgress)
{
	Selection *sel = nil;
	/*
	if (fQuery != nil)
	{
		lasterr = fQuery->ExecSql(sel);
	}
	*/
	if (sel == nil)
		outSelection = nil;
	else
		outSelection = new VDB4DSelection(fManager, VImpCreator<VDB4DBase>::GetImpObject(fOwner->GetOwner()), sel->GetParentFile()->GetTable(), sel);
	
	return lasterr;
}



//=================================================================================



Boolean VDB4DRecord::GetBoolean( DB4D_FieldID inFieldID, Boolean *outValue, Boolean OldOne, VError *outErr) const
{
	VError err;
	Boolean isOK = false;
	ValPtr cv;
	if (OldOne)
		cv = fRecord->GetNthFieldOld( (sLONG) inFieldID, err);
	else
		cv = fRecord->GetNthField( (sLONG) inFieldID, err);
	if (cv != nil) {
		VValueSingle *cvs = cv->IsVValueSingle();
		if (cvs != nil) {
			*outValue = cvs->GetBoolean();
			isOK = true;
		}
	}
	if (outErr != nil)
		*outErr = err;
	return isOK;
}


Boolean VDB4DRecord::GetBoolean( const CDB4DField* inField, Boolean *outValue, Boolean OldOne, VError *outErr) const
{
	VError err;
	Boolean isOK = false;
	ValPtr cvs;
	if (OldOne)
		cvs = fRecord->GetFieldOldValue( VImpCreator<VDB4DField>::GetImpObject(inField)->GetField(), err);
	else
		cvs = fRecord->GetFieldValue( VImpCreator<VDB4DField>::GetImpObject(inField)->GetField(), err);

	if (cvs != nil) {
		*outValue = cvs->GetBoolean();
		isOK = true;
	}
	if (outErr != nil)
		*outErr = err;
	return isOK;
}


Boolean VDB4DRecord::GetLong( DB4D_FieldID inFieldID, sLONG *outValue, Boolean OldOne, VError *outErr) const
{
	VError err;
	Boolean isOK = false;
	ValPtr cv;
	if (OldOne)
		cv = fRecord->GetNthFieldOld( (sLONG) inFieldID, err);
	else
		cv = fRecord->GetNthField( (sLONG) inFieldID, err);
	if (cv != nil) {
		VValueSingle *cvs = cv->IsVValueSingle();
		if (cvs != nil) {
			*outValue = cvs->GetLong();
			isOK = true;
		}
	}
	if (outErr != nil)
		*outErr = err;
	return isOK;
}


Boolean VDB4DRecord::GetLong( const CDB4DField* inField, sLONG *outValue, Boolean OldOne, VError *outErr) const
{
	VError err;
	Boolean isOK = false;
	ValPtr cvs;
	if (OldOne)
		cvs = fRecord->GetFieldOldValue( VImpCreator<VDB4DField>::GetImpObject(inField)->GetField(), err);
	else
		cvs = fRecord->GetFieldValue( VImpCreator<VDB4DField>::GetImpObject(inField)->GetField(), err);

	if (cvs != nil) {
		*outValue = cvs->GetLong();
		isOK = true;
	}
	if (outErr != nil)
		*outErr = err;
	return isOK;
}


Boolean VDB4DRecord::GetLong8( DB4D_FieldID inFieldID, sLONG8 *outValue, Boolean OldOne, VError *outErr) const
{
	VError err;
	Boolean isOK = false;
	ValPtr cv;
	if (OldOne)
		cv = fRecord->GetNthFieldOld( (sLONG) inFieldID, err);
	else
		cv = fRecord->GetNthField( (sLONG) inFieldID, err);
	if (cv != nil) {
		VValueSingle *cvs = cv->IsVValueSingle();
		if (cvs != nil) {
			*outValue = cvs->GetLong8();
			isOK = true;
		}
	}
	if (outErr != nil)
		*outErr = err;
	return isOK;
}


Boolean VDB4DRecord::GetLong8( const CDB4DField* inField, sLONG8 *outValue, Boolean OldOne, VError *outErr) const
{
	VError err;
	Boolean isOK = false;
	ValPtr cvs;
	if (OldOne)
		cvs = fRecord->GetFieldOldValue( VImpCreator<VDB4DField>::GetImpObject(inField)->GetField(), err);
	else
		cvs = fRecord->GetFieldValue( VImpCreator<VDB4DField>::GetImpObject(inField)->GetField(), err);

	if (cvs != nil) {
		*outValue = cvs->GetLong8();
		isOK = true;
	}
	if (outErr != nil)
		*outErr = err;
	return isOK;
}


Boolean VDB4DRecord::GetReal( DB4D_FieldID inFieldID, Real *outValue, Boolean OldOne, VError *outErr) const
{
	VError err;
	Boolean isOK = false;
	ValPtr cv;
	if (OldOne)
		cv = fRecord->GetNthFieldOld( (sLONG) inFieldID, err);
	else
		cv = fRecord->GetNthField( (sLONG) inFieldID, err);
	if (cv != nil) {
		VValueSingle *cvs = cv->IsVValueSingle();
		if (cvs != nil) {
			*outValue = cvs->GetReal();
			isOK = true;
		}
	}
	if (outErr != nil)
		*outErr = err;
	return isOK;
}


Boolean VDB4DRecord::GetReal( const CDB4DField* inField, Real *outValue, Boolean OldOne, VError *outErr) const
{
	VError err;
	Boolean isOK = false;
	ValPtr cvs;
	if (OldOne)
		cvs = fRecord->GetFieldOldValue( VImpCreator<VDB4DField>::GetImpObject(inField)->GetField(), err);
	else
		cvs = fRecord->GetFieldValue( VImpCreator<VDB4DField>::GetImpObject(inField)->GetField(), err);

	if (cvs != nil) {
		*outValue = cvs->GetReal();
		isOK = true;
	}
	if (outErr != nil)
		*outErr = err;
	return isOK;
}


Boolean VDB4DRecord::Save(VError *outErr)
{
	VError err;
	if (fRecord->IsRemote())
	{
		Table *owner = fRecord->GetOwner();

		Boolean legacycall = owner->GetOwner()->IsThereATrigger(DB4D_SaveNewRecord_Trigger, owner) || owner->GetOwner()->IsThereATrigger(DB4D_SaveExistingRecord_Trigger, owner);
		IRequest *req = owner->GetOwner()->CreateRequest( fContext, Req_SaveRecord + kRangeReqDB4D, legacycall);
		if (req == nil)
		{
			err = ThrowBaseError(memfull, noaction);
		}
		else
		{
			req->PutBaseParam( owner->GetOwner());
			req->PutThingsToForget( VDBMgr::GetManager(), ConvertContext(fContext));
			req->PutTableParam( owner);
			req->PutFicheInMemParam( fRecord, fContext);

			err = req->GetLastError();
			if (err == VE_OK)
			{
				err = req->Send();
				if (err== VE_OK)
					err = req->GetUpdatedInfo(owner->GetOwner(), ConvertContext(fContext));
				if (err == VE_OK)
				{
					sLONG recnum = req->GetLongReply(err);
					if (err == VE_OK)
						fRecord->SetRemoteRecordNumber(recnum);
				}
				if (err == VE_OK)
				{
					fRecord->SetRemoteSeqNum(req->GetLong8Reply(err));
					fRecord->ClearNew();
					fRecord->ReceiveModifiedFields(req->GetInputStream(), ConvertContext(fContext), owner);
					fRecord->ClearModified();
				}
			}
			req->Release();
		}
	}
	else
	{
		BaseTaskInfoPtr context = fContext;
		err = fRecord->GetDF()->SaveRecord( fRecord, context);
	}
	if (outErr != nil)
		*outErr = err;
	return err == VE_OK;
}


VDB4DRecord::~VDB4DRecord()
{
	if (fOwner != nil)
		fOwner->Release();
	if (fRecord != nil)
		fRecord->Release();
	fRecord = nil;
	if (fContext != nil)
		fContext->Release();
}

Boolean VDB4DRecord::GetBlob( DB4D_FieldID inFieldID, VBlob **outBlob, Boolean OldOne, VError *outErr) const
{
	VError err;
	Boolean isOK = false;
	ValPtr cv;
	if (OldOne)
		cv = fRecord->GetNthFieldOld( (sLONG) inFieldID, err);
	else
		cv = fRecord->GetNthField( (sLONG) inFieldID, err);
	if (cv != nil) {
		if (cv->GetValueKind() == VK_BLOB) {
			VBlob *blob = dynamic_cast<VBlob *>( cv);
			if (blob != nil) {
				*outBlob = (VBlob *) cv->Clone();
				if (*outBlob == nil)
					err = fRecord->ThrowError(memfull, DBaction_BuildingValue);
				else
					isOK = true;			
			}
		}
	}
	if (outErr != nil)
		*outErr = err;
	return isOK;
}


Boolean VDB4DRecord::GetBlob( const CDB4DField* inField, VBlob **outBlob, Boolean OldOne, VError *outErr) const
{
	VError err;
	Boolean isOK = false;
	ValPtr cv;
	if (OldOne)
		cv = fRecord->GetFieldOldValue( VImpCreator<VDB4DField>::GetImpObject(inField)->GetField(), err);
	else
		cv = fRecord->GetFieldValue( VImpCreator<VDB4DField>::GetImpObject(inField)->GetField(), err);

	if (cv != nil) {
		if (cv->GetValueKind() == VK_BLOB) {
			VBlob *blob = dynamic_cast<VBlob *>( cv);
			if (blob != nil) {
				*outBlob = (VBlob *) cv->Clone();
				if (*outBlob == nil)
					err = fRecord->ThrowError(memfull, DBaction_BuildingValue);
				else
					isOK = true;			
			}
		}
	}
	if (outErr != nil)
		*outErr = err;
	return isOK;
}



Boolean VDB4DRecord::SetBlob( DB4D_FieldID inFieldID, const void *inData, sLONG inDataSize, VError *outErr)
{
	VError err;
	Boolean isOK = false;
	ValPtr cv = fRecord->GetNthField( (sLONG) inFieldID, err);
	if (cv != nil) {
		if (cv->GetValueKind() == VK_BLOB) {
			VBlob *blob = dynamic_cast<VBlob *>( cv);
			VError err = blob->FromData( inData, inDataSize);
			fRecord->Touch(inFieldID);
			isOK = (err == VE_OK);
		}
	}
	if (outErr != nil)
		*outErr = err;
	return isOK;
}


Boolean VDB4DRecord::SetBlob( const CDB4DField* inField, const void *inData, sLONG inDataSize, VError *outErr)
{
	VError err;
	Boolean isOK = false;
	ValPtr cv = fRecord->GetFieldValue( VImpCreator<VDB4DField>::GetImpObject(inField)->GetField(), err);
	if (cv != nil) {
		if (cv->GetValueKind() == VK_BLOB) {
			VBlob *blob = dynamic_cast<VBlob *>( cv);
			VError err = blob->FromData( inData, inDataSize);
			fRecord->Touch(VImpCreator<VDB4DField>::GetImpObject(inField)->GetField());
			isOK = (err == VE_OK);
		}
	}
	if (outErr != nil)
		*outErr = err;
	return isOK;
}

Boolean VDB4DRecord::SetPicture( DB4D_FieldID inFieldID,  const XBOX::VValueSingle& inPict, VError *outErr)
{
	VError err;
	Boolean isOK = false;
	ValPtr cv = fRecord->GetNthField( (sLONG) inFieldID, err);
	if (cv != nil) {
		if (cv->GetValueKind() == VK_IMAGE && inPict.GetValueKind()==VK_IMAGE) {
			VValueSingle *pict = dynamic_cast<VValueSingle*>( cv);
			pict->FromValue(inPict);
			fRecord->Touch(inFieldID);
			isOK = (err == VE_OK);
		}
	}
	if (outErr != nil)
		*outErr = err;
	return isOK;
}


Boolean VDB4DRecord::SetPicture( const CDB4DField* inField,  const XBOX::VValueSingle& inPict, VError *outErr)
{
	VError err;
	Boolean isOK = false;
	ValPtr cv = fRecord->GetFieldValue( VImpCreator<VDB4DField>::GetImpObject(inField)->GetField(), err);
	if (cv != nil) {
		if (cv->GetValueKind() == VK_IMAGE && inPict.GetValueKind()==VK_IMAGE) {
			VValueSingle *pict = dynamic_cast<VValueSingle *>( cv);
			pict->FromValue(inPict);
			fRecord->Touch(VImpCreator<VDB4DField>::GetImpObject(inField)->GetField());
			isOK = (err == VE_OK);
		}
	}
	if (outErr != nil)
		*outErr = err;
	return isOK;
}

Boolean VDB4DRecord::GetPicture( DB4D_FieldID inFieldID, VValueSingle **outPicture, Boolean OldOne, VError *outErr) const
{
	VError err;
	Boolean isOK = false;
	ValPtr cv;
	if (OldOne)
		cv = fRecord->GetNthFieldOld( (sLONG) inFieldID, err);
	else
		cv = fRecord->GetNthField( (sLONG) inFieldID, err);
	if (cv != nil) {
		if (cv->GetValueKind() == VK_IMAGE) {
			VValueSingle *blob = dynamic_cast<VValueSingle *>( cv);
			if (blob != nil) {
				*outPicture = (VValueSingle *) cv->Clone();
				if (*outPicture == nil)
					err = fRecord->ThrowError(memfull, DBaction_BuildingValue);
				else
					isOK = true;			
			}
		}
	}
	if (outErr != nil)
		*outErr = err;
	return isOK;
}


Boolean VDB4DRecord::GetPicture( const CDB4DField* inField, VValueSingle **outPicture, Boolean OldOne, VError *outErr) const
{
	VError err;
	Boolean isOK = false;
	ValPtr cv;
	if (OldOne)
		cv = fRecord->GetFieldOldValue( VImpCreator<VDB4DField>::GetImpObject(inField)->GetField(), err);
	else
		cv = fRecord->GetFieldValue( VImpCreator<VDB4DField>::GetImpObject(inField)->GetField(), err);

	if (cv != nil) {
		if (cv->GetValueKind() == VK_IMAGE) {
			VValueSingle *pict = dynamic_cast<VValueSingle *>( cv);
			if (pict != nil) {
				*outPicture = (VValueSingle *) cv->Clone();
				if (*outPicture == nil)
					err = fRecord->ThrowError(memfull, DBaction_BuildingValue);
				else
					isOK = true;			
			}
		}
	}
	if (outErr != nil)
		*outErr = err;
	return isOK;
}

VDB4DRecord::VDB4DRecord( VDBMgr* manager, FicheInMem *inFiche, CDB4DBaseContextPtr inContext)
{
	fManager = manager;
	fRecord = inFiche;
	fOwner = nil;
	fIsLoaded = true;
	/*
	if (fRecord != nil)
		fRecord->Retain();
	*/
	if (inContext != nil)
	{
		fContext = ConvertContext(inContext);
		fContext->Retain();
		assert(fRecord == nil || fContext->GetBase() == fRecord->GetOwner()->GetOwner());
	}
	else fContext = nil;
}


Boolean VDB4DRecord::SetString( DB4D_FieldID inFieldID, const VString& inValue, VError *outErr)
{
	VError err;
	Boolean isOK = false;
	ValPtr cv = fRecord->GetNthField( (sLONG) inFieldID, err);
	if (cv != nil) {
		VValueSingle *cvs = cv->IsVValueSingle();
		if (cvs != nil) {
			cvs->FromString( inValue);
			fRecord->Touch(inFieldID);
			isOK = true;
		}
	}
	if (outErr != nil)
		*outErr = err;
	return isOK;
}


Boolean VDB4DRecord::SetString( const CDB4DField* inField, const VString& inValue, VError *outErr)
{
	VError err;
	Boolean isOK = false;
	ValPtr cv = fRecord->GetFieldValue( VImpCreator<VDB4DField>::GetImpObject(inField)->GetField(), err);
	if (cv != nil) {
		cv->FromString( inValue);
		fRecord->Touch(VImpCreator<VDB4DField>::GetImpObject(inField)->GetField());
		isOK = true;
	}
	if (outErr != nil)
		*outErr = err;
	return isOK;
}


Boolean VDB4DRecord::SetDuration( DB4D_FieldID inFieldID, const VDuration& inValue, VError *outErr)
{
	VError err;
	Boolean isOK = false;
	ValPtr cv = fRecord->GetNthField( (sLONG) inFieldID, err);
	if (cv != nil) {
		VValueSingle *cvs = cv->IsVValueSingle();
		if (cvs != nil) {
			cvs->FromDuration( inValue);
			fRecord->Touch(inFieldID);
			isOK = true;
		}
	}
	if (outErr != nil)
		*outErr = err;
	return isOK;
}


Boolean VDB4DRecord::SetDuration( const CDB4DField* inField, const VDuration& inValue, VError *outErr)
{
	VError err;
	Boolean isOK = false;
	ValPtr cv = fRecord->GetFieldValue( VImpCreator<VDB4DField>::GetImpObject(inField)->GetField(), err);
	if (cv != nil) {
		cv->FromDuration( inValue);
		fRecord->Touch(VImpCreator<VDB4DField>::GetImpObject(inField)->GetField());
		isOK = true;
	}
	if (outErr != nil)
		*outErr = err;
	return isOK;
}


Boolean VDB4DRecord::SetTime( DB4D_FieldID inFieldID, const VTime& inValue, VError *outErr)
{
	VError err;
	Boolean isOK = false;
	ValPtr cv = fRecord->GetNthField( (sLONG) inFieldID, err);
	if (cv != nil) {
		VValueSingle *cvs = cv->IsVValueSingle();
		if (cvs != nil) {
			cvs->FromTime( inValue);
			fRecord->Touch(inFieldID);
			isOK = true;
		}
	}
	if (outErr != nil)
		*outErr = err;
	return isOK;
}

Boolean VDB4DRecord::SetTime( const CDB4DField* inField, const VTime& inValue, VError *outErr)
{
	VError err;
	Boolean isOK = false;
	ValPtr cv = fRecord->GetFieldValue( VImpCreator<VDB4DField>::GetImpObject(inField)->GetField(), err);
	if (cv != nil) {
		cv->FromTime( inValue);
		fRecord->Touch(VImpCreator<VDB4DField>::GetImpObject(inField)->GetField());
		isOK = true;
	}
	if (outErr != nil)
		*outErr = err;
	return isOK;
}



Boolean VDB4DRecord::SetLong( DB4D_FieldID inFieldID, sLONG inValue, VError *outErr)
{
	VError err;
	Boolean isOK = false;
	ValPtr cv = fRecord->GetNthField( (sLONG) inFieldID, err);
	if (cv != nil) {
		VValueSingle *cvs = cv->IsVValueSingle();
		if (cvs != nil) {
			cvs->FromLong( inValue);
			fRecord->Touch(inFieldID);
			isOK = true;
		}
	}
	if (outErr != nil)
		*outErr = err;
	return isOK;
}


Boolean VDB4DRecord::SetLong( const CDB4DField* inField, sLONG inValue, VError *outErr)
{
	VError err;
	Boolean isOK = false;
	ValPtr cv = fRecord->GetFieldValue( VImpCreator<VDB4DField>::GetImpObject(inField)->GetField(), err);
	if (cv != nil) {
		cv->FromLong( inValue);
		fRecord->Touch(VImpCreator<VDB4DField>::GetImpObject(inField)->GetField());
		isOK = true;
	}
	if (outErr != nil)
		*outErr = err;
	return isOK;
}


Boolean VDB4DRecord::SetLong8( DB4D_FieldID inFieldID, sLONG8 inValue, VError *outErr)
{
	VError err;
	Boolean isOK = false;
	ValPtr cv = fRecord->GetNthField( (sLONG) inFieldID, err);
	if (cv != nil) {
		VValueSingle *cvs = cv->IsVValueSingle();
		if (cvs != nil) {
			cvs->FromLong8( inValue);
			fRecord->Touch(inFieldID);
			isOK = true;
		}
	}
	if (outErr != nil)
		*outErr = err;
	return isOK;
}


Boolean VDB4DRecord::SetLong8( const CDB4DField* inField, sLONG8 inValue, VError *outErr)
{
	VError err;
	Boolean isOK = false;
	ValPtr cv = fRecord->GetFieldValue( VImpCreator<VDB4DField>::GetImpObject(inField)->GetField(), err);
	if (cv != nil) {
		cv->FromLong8( inValue);
		fRecord->Touch(VImpCreator<VDB4DField>::GetImpObject(inField)->GetField());
		isOK = true;
	}
	if (outErr != nil)
		*outErr = err;
	return isOK;
}


Boolean VDB4DRecord::SetReal( DB4D_FieldID inFieldID, Real inValue, VError *outErr)
{
	VError err;
	Boolean isOK = false;
	ValPtr cv = fRecord->GetNthField( (sLONG) inFieldID, err);
	if (cv != nil) {
		VValueSingle *cvs = cv->IsVValueSingle();
		if (cvs != nil) {
			cvs->FromReal( inValue);
			fRecord->Touch(inFieldID);
			isOK = true;
		}
	}
	if (outErr != nil)
		*outErr = err;
	return isOK;
}


Boolean VDB4DRecord::SetReal( const CDB4DField* inField, Real inValue, VError *outErr)
{
	VError err;
	Boolean isOK = false;
	ValPtr cv = fRecord->GetFieldValue( VImpCreator<VDB4DField>::GetImpObject(inField)->GetField(), err);
	if (cv != nil) {
		cv->FromReal( inValue);
		fRecord->Touch(VImpCreator<VDB4DField>::GetImpObject(inField)->GetField());
		isOK = true;
	}
	if (outErr != nil)
		*outErr = err;
	return isOK;
}


Boolean VDB4DRecord::SetBoolean( DB4D_FieldID inFieldID, Boolean inValue, VError *outErr)
{
	VError err;
	Boolean isOK = false;
	ValPtr cv = fRecord->GetNthField( (sLONG) inFieldID, err);
	if (cv != nil) {
		VValueSingle *cvs = cv->IsVValueSingle();
		if (cvs != nil) {
			cvs->FromBoolean( inValue);
			fRecord->Touch(inFieldID);
			isOK = true;
		}
	}
	if (outErr != nil)
		*outErr = err;
	return isOK;
}


Boolean VDB4DRecord::SetBoolean( const CDB4DField* inField, Boolean inValue, VError *outErr)
{
	VError err;
	Boolean isOK = false;
	ValPtr cv = fRecord->GetFieldValue( VImpCreator<VDB4DField>::GetImpObject(inField)->GetField(), err);
	if (cv != nil) {
		cv->FromBoolean( inValue);
		fRecord->Touch(VImpCreator<VDB4DField>::GetImpObject(inField)->GetField());
		isOK = true;
	}
	if (outErr != nil)
		*outErr = err;
	return isOK;
}



Boolean VDB4DRecord::IsProtected() const
{
	return fRecord->ReadOnlyState();
}


Boolean VDB4DRecord::IsNew() const
{
	return fRecord->IsNew();
}


Boolean VDB4DRecord::IsModified() const
{
	return fRecord->IsRecordModified();
}


Boolean VDB4DRecord::IsFieldModified( DB4D_FieldID inFieldID) const
{
	return fRecord->IsModif( inFieldID);
}


Boolean VDB4DRecord::IsFieldModified( const CDB4DField* inField) const
{
	return fRecord->IsModif( VImpCreator<VDB4DField>::GetImpObject(inField)->GetField());
}


Boolean VDB4DRecord::GetString( DB4D_FieldID inFieldID, VString& outValue, Boolean OldOne, VError *outErr) const
{
	VError err;
	Boolean isOK = false;
	ValPtr cv;
	if (OldOne)
		cv = fRecord->GetNthFieldOld( (sLONG) inFieldID, err);
	else
		cv = fRecord->GetNthField( (sLONG) inFieldID, err);
	if (cv != nil) {
		VValueSingle *cvs = cv->IsVValueSingle();
		if (cvs != nil) {
			cvs->GetString( outValue);
			isOK = true;
		}
	}
	if (outErr != nil)
		*outErr = err;
	return isOK;
}


Boolean VDB4DRecord::GetString( const CDB4DField* inField, VString& outValue, Boolean OldOne, VError *outErr) const
{
	VError err;
	Boolean isOK = false;
	ValPtr cvs;
	if (OldOne)
		cvs = fRecord->GetFieldOldValue( VImpCreator<VDB4DField>::GetImpObject(inField)->GetField(), err);
	else
		cvs = fRecord->GetFieldValue( VImpCreator<VDB4DField>::GetImpObject(inField)->GetField(), err);

	if (cvs != nil) {
		cvs->GetString(outValue);
		isOK = true;
	}
	if (outErr != nil)
		*outErr = err;
	return isOK;
}


Boolean VDB4DRecord::GetDuration( DB4D_FieldID inFieldID, VDuration& outValue, Boolean OldOne, VError *outErr) const
{
	VError err;
	Boolean isOK = false;
	ValPtr cv;
	if (OldOne)
		cv = fRecord->GetNthFieldOld( (sLONG) inFieldID, err);
	else
		cv = fRecord->GetNthField( (sLONG) inFieldID, err);
	if (cv != nil) {
		VValueSingle *cvs = cv->IsVValueSingle();
		if (cvs != nil) {
			cvs->GetDuration( outValue);
			isOK = true;
		}
	}
	if (outErr != nil)
		*outErr = err;
	return isOK;
}


Boolean VDB4DRecord::GetDuration( const CDB4DField* inField, VDuration& outValue, Boolean OldOne, VError *outErr) const
{
	VError err;
	Boolean isOK = false;
	ValPtr cvs;
	if (OldOne)
		cvs = fRecord->GetFieldOldValue( VImpCreator<VDB4DField>::GetImpObject(inField)->GetField(), err);
	else
		cvs = fRecord->GetFieldValue( VImpCreator<VDB4DField>::GetImpObject(inField)->GetField(), err);

	if (cvs != nil) {
		cvs->GetDuration(outValue);
		isOK = true;
	}
	if (outErr != nil)
		*outErr = err;
	return isOK;
}


Boolean VDB4DRecord::GetTime( DB4D_FieldID inFieldID, VTime& outValue, Boolean OldOne, VError *outErr) const
{
	VError err;
	Boolean isOK = false;
	ValPtr cv;
	if (OldOne)
		cv = fRecord->GetNthFieldOld( (sLONG) inFieldID, err);
	else
		cv = fRecord->GetNthField( (sLONG) inFieldID, err);
	if (cv != nil) {
		VValueSingle *cvs = cv->IsVValueSingle();
		if (cvs != nil) {
			cvs->GetTime( outValue);
			isOK = true;
		}
	}
	if (outErr != nil)
		*outErr = err;
	return isOK;
}


Boolean VDB4DRecord::GetTime( const CDB4DField* inField, VTime& outValue, Boolean OldOne, VError *outErr) const
{
	VError err;
	Boolean isOK = false;
	ValPtr cvs;
	if (OldOne)
		cvs = fRecord->GetFieldOldValue( VImpCreator<VDB4DField>::GetImpObject(inField)->GetField(), err);
	else
		cvs = fRecord->GetFieldValue( VImpCreator<VDB4DField>::GetImpObject(inField)->GetField(), err);

	if (cvs != nil) {
		cvs->GetTime(outValue);
		isOK = true;
	}
	if (outErr != nil)
		*outErr = err;
	return isOK;
}


RecIDType VDB4DRecord::GetID() const
{
	/*
	if (fRecord->IsNew())
		return kDB4D_NullRecordID;
	*/
	return fRecord->GetNum();
}

Boolean VDB4DRecord::GetVUUID( DB4D_FieldID inFieldID, VUUID& outValue, Boolean OldOne, VError *outErr) const
{
	VError err;
	Boolean isOK = false;
	ValPtr cv;
	if (OldOne)
		cv = fRecord->GetNthFieldOld( (sLONG) inFieldID, err);
	else
		cv = fRecord->GetNthField( (sLONG) inFieldID, err);
	if (cv != nil) {
		VValueSingle *cvs = cv->IsVValueSingle();
		if (cvs != nil) {
			cvs->GetVUUID( outValue);
			isOK = true;
		}
	}
	if (outErr != nil)
		*outErr = err;
	return isOK;
}


Boolean VDB4DRecord::GetVUUID( const CDB4DField* inField, VUUID& outValue, Boolean OldOne, VError *outErr) const
{
	VError err;
	Boolean isOK = false;
	ValPtr cvs;
	if (OldOne)
		cvs = fRecord->GetFieldOldValue( VImpCreator<VDB4DField>::GetImpObject(inField)->GetField(), err);
	else
		cvs = fRecord->GetFieldValue( VImpCreator<VDB4DField>::GetImpObject(inField)->GetField(), err);

	if (cvs != nil) {
		cvs->GetVUUID(outValue);
		isOK = true;
	}
	if (outErr != nil)
		*outErr = err;
	return isOK;
}


Boolean VDB4DRecord::SetVUUID( DB4D_FieldID inFieldID, const VUUID& inValue, VError *outErr)
{
	VError err;
	Boolean isOK = false;
	ValPtr cv = fRecord->GetNthField( (sLONG) inFieldID, err);
	if (cv != nil) {
		VValueSingle *cvs = cv->IsVValueSingle();
		if (cvs != nil) {
			cvs->FromVUUID( inValue);
			fRecord->Touch((sLONG)inFieldID);
			isOK = true;
		}
	}
	if (outErr != nil)
		*outErr = err;
	return isOK;
}


Boolean VDB4DRecord::SetVUUID( const CDB4DField* inField, const VUUID& inValue, VError *outErr)
{
	VError err;
	Boolean isOK = false;
	ValPtr cv = fRecord->GetFieldValue( VImpCreator<VDB4DField>::GetImpObject(inField)->GetField(), err);
	if (cv != nil) {
		cv->FromVUUID( inValue);
		fRecord->Touch(VImpCreator<VDB4DField>::GetImpObject(inField)->GetField());
		isOK = true;
	}
	if (outErr != nil)
		*outErr = err;
	return isOK;
}


VValueSingle *VDB4DRecord::GetFieldValue( DB4D_FieldID inFieldID, Boolean OldOne, VError *outErr, bool forQueryOrSort)
{
	VError err;
	VValueSingle *val = nil;
	
	if (OldOne)
		val = fRecord->GetNthFieldOld( (sLONG) inFieldID, err, forQueryOrSort);
	else
		val = fRecord->GetNthField( (sLONG) inFieldID, err, false, forQueryOrSort);
	
	if (val != nil && val->GetValueKind() == VK_UUID)
	{
		/*
		if (val->IsNull())
		{
			Field* cri = fRecord->GetOwner()->RetainField(inFieldID);
			if (cri != nil && cri->GetAutoGenerate())
				((VUUID*)val)->Regenerate();
			QuickReleaseRefCountable(cri);
		}
		*/
		VString* val2 = ((VUUID*)val)->GetCachedConvertedString(true);
		val = val2;
	}
	
	if (outErr != nil)
		*outErr = err;
	return val;
}


VValueSingle *VDB4DRecord::GetFieldValue( const CDB4DField* inField, Boolean OldOne, VError *outErr, bool forQueryOrSort)
{
	VError err;
	VValueSingle *val = nil;

	Field* cri = nil;
	if (testAssert(inField != nil))
	{
		cri = VImpCreator<VDB4DField>::GetImpObject(inField)->GetField();
		val = fRecord->GetFieldValue(cri, err, false, forQueryOrSort);
	}
	else
		err = fRecord->ThrowError(VE_DB4D_WRONGFIELDREF_IN_RECORD, DBactionFinale);

	if (val != nil && val->GetValueKind() == VK_UUID)
	{
		/*
		if (val->IsNull() && cri->GetAutoGenerate())
			((VUUID*)val)->Regenerate();
			*/
		VString* val2 = ((VUUID*)val)->GetCachedConvertedString(true);
		val = val2;
	}

	if (outErr != nil)
		*outErr = err;
	return val;
}


Boolean VDB4DRecord::GetFieldIntoBlob( const CDB4DField* inField,  VBlob& outBlob, VErrorDB4D *outErr, Boolean CanCacheData)
{
	if (testAssert(inField != nil))
		return GetFieldIntoBlob(VImpCreator<VDB4DField>::GetImpObject(inField)->GetField()->GetPosInRec(), outBlob, outErr, CanCacheData);
	else
	{
		if (outErr != nil)
			*outErr = fRecord->ThrowError(VE_DB4D_WRONGFIELDREF_IN_RECORD, DBactionFinale);
		return false;
	}
}


Boolean VDB4DRecord::GetFieldIntoBlob( DB4D_FieldID inFieldID,  VBlob& outBlob, VErrorDB4D *outErr, Boolean CanCacheData)
{
	VError err = fRecord->GetNthFieldBlob(inFieldID, outBlob, CanCacheData);
	if (outErr != nil)
		*outErr = err;
	return err == VE_OK;
}



void VDB4DRecord::SetFieldToNull(const CDB4DField* inField, VError *outErr)
{
	VError err = VE_OK;
	VValueSingle *val = GetFieldValue(inField, err);
	if (val != nil)
	{
		val->SetNull(true);
		fRecord->Touch(VImpCreator<VDB4DField>::GetImpObject(inField)->GetField());
	}
	if (outErr != nil)
		*outErr = err;
}


void VDB4DRecord::SetFieldToNull(DB4D_FieldID inFieldID, VError *outErr)
{
	VError err = VE_OK;
	VValueSingle *val = GetFieldValue(inFieldID, err);
	if (val != nil)
	{
		val->SetNull(true);
		fRecord->Touch(inFieldID);
	}
	if (outErr != nil)
		*outErr = err;
}


void VDB4DRecord::GetTimeStamp(VTime& outValue)
{
	uLONG8 quand = fRecord->GetAssoc()->GetTimeStamp();
	outValue.FromStamp(quand);
}


FicheInMem4DPtr VDB4DRecord::GetFicheInMem(void)
{
	return (FicheInMem4DPtr)fRecord;
}



Boolean VDB4DRecord::Drop(VError *outErr)
{
	if (fRecord->IsRemote())
	{
		Table* owner = VImpCreator<VDB4DTable>::GetImpObject(fOwner)->GetTable();

		VError err;
		Boolean ok = false;
		Boolean legacycall = owner->GetOwner()->IsThereATrigger(DB4D_DeleteRecord_Trigger, owner, true);
		IRequest *req = owner->GetOwner()->CreateRequest( fContext, req_DeleteRecord + kRangeReqDB4D, legacycall);
		if (req == nil)
		{
			err = ThrowBaseError(memfull, noaction);
		}
		else
		{
			req->PutBaseParam( owner->GetOwner());
			req->PutThingsToForget( VDBMgr::GetManager(), ConvertContext(fContext));
			req->PutTableParam( owner);
			req->PutFicheInMemParam( fRecord, fContext);

			err = req->GetLastError();
			if (err == VE_OK)
			{
				err = req->Send();
				if (err== VE_OK)
					err = req->GetUpdatedInfo(owner->GetOwner(), ConvertContext(fContext));
				if (err == VE_OK)
					ok = req->GetBooleanReply(err);
			}
			req->Release();
		}
		if (outErr != nil)
			*outErr = err;
		return ok;
	}
	else
	{
		BaseTaskInfoPtr context = fContext;
		VError err = fRecord->GetDF()->DelRecord(fRecord, context);
		if (outErr != nil)
			*outErr = err;
		return (err == VE_OK);
	}
}


uLONG VDB4DRecord::GetFieldModificationStamp( DB4D_FieldID inFieldID) const
{
	return fRecord->GetFieldModificationStamp((sLONG)inFieldID);
}


uLONG VDB4DRecord::GetFieldModificationStamp( const CDB4DField* inField) const
{
	return fRecord->GetFieldModificationStamp(VImpCreator<VDB4DField>::GetImpObject(inField)->GetField());
}


void VDB4DRecord::Touch(DB4D_FieldID inFieldID)
{
	//CheckFastMem();
	fRecord->Touch((sLONG)inFieldID);
	if (fRecord->GetOwner()->GetFieldType(inFieldID) == VK_UUID)
	{
		VError err;
		VValueSingle* val = fRecord->GetNthField((sLONG)inFieldID, err);
		if (val != nil && val->GetValueKind() == VK_UUID)
		{
			VString* val2 = ((VUUID*)val)->GetCachedConvertedString(false);
			if (val2 != nil)
				val->FromString(*val2);
		}
	}
	//CheckFastMem();
}


void VDB4DRecord::Touch(const CDB4DField* inField)
{
	Field* cri = VImpCreator<VDB4DField>::GetImpObject(inField)->GetField();
	fRecord->Touch(cri);
	if (cri->GetTyp() == VK_UUID)
	{
		VError err;
		VValueSingle* val = fRecord->GetFieldValue(cri, err);
		if (val != nil && val->GetValueKind() == VK_UUID)
		{
			VString* val2 = ((VUUID*)val)->GetCachedConvertedString(false);
			if (val2 != nil)
				val->FromString(*val2);
		}
	}
	//CheckFastMem();
}


CDB4DRecord* VDB4DRecord::Clone(VError* outErr) const
{
	VError err = VE_OK;
	*outErr = VE_OK;
	CDB4DRecord* result = nil;
	FicheInMem* fic = fRecord->Clone(fContext, err);
	if (fic != nil)
	{
		result = new VDB4DRecord(fManager, fic, fContext);
		if (result == nil)
		{
			err = fRecord->ThrowError(memfull, DBaction_BuildingRecord);
			fic->Release();
		}
	}

	if (outErr != nil)
		*outErr = err;
	return result;
}


CDB4DRecord* VDB4DRecord::CloneForPush(VError* outErr)
{
	VError err = VE_OK;
	*outErr = VE_OK;
	CDB4DRecord* result = nil;
	FicheInMem* fic = fRecord->CloneForPush(fContext, err);
	if (fic != nil)
	{
		result = new VDB4DRecord(fManager, fic, fContext);
		if (result == nil)
		{
			err = fRecord->ThrowError(memfull, DBaction_BuildingRecord);
			fic->Release();
		}
	}

	if (outErr != nil)
		*outErr = err;
	return result;
}


void VDB4DRecord::RestoreFromPop()
{
	if (fRecord != nil)
		fRecord->RestoreFromPop();
}


uLONG VDB4DRecord::GetModificationStamp() const
{
	if (fRecord != nil)
		return fRecord->GetModificationStamp();
	else
		return 0;
}


uLONG8 VDB4DRecord::GetSyncInfoStamp() const
{
	if (fRecord != nil)
		return fRecord->GetSyncInfo();
	else
		return 0;
}




VError VDB4DRecord::DetachRecord(Boolean BlobFieldsCanBeEmptied)
{
	return fRecord->Detach(fContext, BlobFieldsCanBeEmptied);
}


void VDB4DRecord::WhoLockedIt(DB4D_KindOfLock& outLockType, const VValueBag **outLockingContextRetainedExtraData) const
{
	// on ne retourne plus le basetaskinfo car sitot retourne il peut avoir detruit,
	// et meme l'appel a GetEncapsuleur peut crasher.
	fRecord->WhoLockedIt(outLockType, outLockingContextRetainedExtraData);
}


sLONG8 VDB4DRecord::GetSequenceNumber()
{
	return fRecord->GetAutoSeqValue();
}
	

void VDB4DRecord::TransferSequenceNumber( CDB4DRecord *inDestination)
{
	if (fRecord != nil)
	{
		FicheInMem* destination = (inDestination == nil) ? nil : VImpCreator<VDB4DRecord>::GetImpObject(inDestination)->fRecord;
		fRecord->TransferSeqNumToken( fContext, destination);
	}
}


VError VDB4DRecord::FillAllFieldsEmpty()
{
	return fRecord->FillAllFieldsEmpty();
}


DB4D_TableID VDB4DRecord::GetTableRef() const
{
	return fRecord->GetOwner()->GetNum();
}


CDB4DTable* VDB4DRecord::RetainTable() const
{
	if (fOwner == nil)
	{
		VDB4DBase* xbase;
		xbase = new VDB4DBase(VDBMgr::GetManager(), fRecord->GetOwner()->GetOwner(), true);
		fOwner = new VDB4DTable(xbase->GetManager(), xbase, fRecord->GetOwner());
		xbase->Release();

	}
	if (fOwner != nil)
		fOwner->Retain();
	return fOwner;
}


VErrorDB4D VDB4DRecord::ReserveRecordNumber()
{
	return fRecord->ReservedRecordNumber(fContext);
}


CDB4DRecord* VDB4DRecord::CloneOnlyModifiedValues(VErrorDB4D& err) const
{
	err = VE_OK;
	CDB4DRecord* result = nil;
	FicheInMem* fic = fRecord->CloneOnlyModifiedValues(fContext, err);
	if (fic != nil)
	{
		result = new VDB4DRecord(fManager, fic, fContext);
		if (result == nil)
		{
			err = fRecord->ThrowError(memfull, DBaction_BuildingRecord);
			fic->Release();
		}
	}

	return result;
}


VErrorDB4D VDB4DRecord::RevertModifiedValues(CDB4DRecord* From)
{
	FicheInMem* fic = VImpCreator<VDB4DRecord>::GetImpObject(From)->fRecord;
	return fRecord->RevertModifiedValues(fContext, fic);
}


VErrorDB4D VDB4DRecord::ToClient(VStream* into, CDB4DBaseContext* inContext)
{
	BaseTaskInfo* context = nil;
	if (inContext != nil)
		context = ConvertContext(inContext);

	return fRecord->ToClient(into, context);
}

VErrorDB4D VDB4DRecord::ToServer(VStream* into, CDB4DBaseContext* inContext)
{
	BaseTaskInfo* context = nil;
	if (inContext != nil)
		context = ConvertContext(inContext);

	return fRecord->ToServer(into, context);
}




//=================================================================================


VDB4DImpExp::VDB4DImpExp(CDB4DTable* inTarget, Table* intarget):IO(intarget)
{
	target = inTarget;
}


VDB4DImpExp::~VDB4DImpExp()
{
}


CDB4DTable* VDB4DImpExp::GetTarget(void)
{
	return target;
}

		
Boolean VDB4DImpExp::AddCol(DB4D_FieldID inField)
{
	Field* cri = IO.GetTarget()->RetainField((sLONG)inField);
	Boolean ok = IO.AddCol(cri);
	cri->Release();
	return ok;
}


sLONG VDB4DImpExp::CountCol(void) const
{
	return IO.CountCol();
}


DB4D_FieldID VDB4DImpExp::GetCol(sLONG n) const
{
	Field* cri = IO.GetCol(n);
	if (cri == nil) return kDB4D_NullFieldID;
	else return (DB4D_FieldID)cri->GetPosInRec();
}

		
void VDB4DImpExp::SetPath(const VFilePath& newpath)
{
	IO.SetPath(newpath);
}


void VDB4DImpExp::SetPath(const VString& newpath)
{
	IO.SetPath(newpath);
}

		
void VDB4DImpExp::GetPath(VFilePath& curpath) const
{
	IO.GetPath(curpath);
}


void VDB4DImpExp::GetPath(VString& curpath) const
{
	IO.GetPath(curpath);
}

		
void VDB4DImpExp::SetColDelimit(const VString& newColDelimit)
{
	IO.SetColDelimit(newColDelimit);
}


void VDB4DImpExp::GetColDelimit(VString& curColDelimit) const
{
	IO.GetColDelimit(curColDelimit);
}

		
void VDB4DImpExp::SetRowDelimit(const VString& newRowDelimit)
{
	IO.SetRowDelimit(newRowDelimit);
}


void VDB4DImpExp::GetRowDelimit(VString& curRowDelimit) const
{
	IO.GetRowDelimit(curRowDelimit);
}

		
VError VDB4DImpExp::RunImport(CDB4DSelection* &outSel, VDB4DProgressIndicator* InProgress, CDB4DBaseContextPtr InContext)
{
	BaseTaskInfoPtr context = nil;
	if (InContext != nil)
	{
		context = ConvertContext(InContext);
	}
	VDB4DTable* xtarget = VImpCreator<VDB4DTable>::GetImpObject( target);
	Selection* sel;
	VError err = IO.RunImport(sel, context, InProgress);
	if (sel == nil) outSel = nil;
	else
	{
		outSel = new VDB4DSelection( xtarget->GetManager(), VImpCreator<VDB4DBase>::GetImpObject(xtarget->GetOwner()), IO.GetTarget(), sel);
	}
	
	return err;
}


VError VDB4DImpExp::RunExport(CDB4DSelection* inSel, VDB4DProgressIndicator* InProgress, CDB4DBaseContextPtr InContext)
{
	Selection* sel;
	
	BaseTaskInfoPtr context = nil;
	if (InContext != nil)
	{
		context = ConvertContext(InContext);
	}

	if (inSel != nil)
	{
		VDB4DSelection* xinSel = VImpCreator<VDB4DSelection>::GetImpObject( inSel);
		sel = xinSel->GetSel();
	}
	else sel = nil;
	
	VDB4DTable* xtarget = VImpCreator<VDB4DTable>::GetImpObject( target);
	
	return IO.RunExport(sel, context, InProgress);
	
}


void VDB4DImpExp::SetCharSet(CharSet newset)
{
	IO.SetCharSet(newset);
}


CharSet VDB4DImpExp::GetCharSet()
{
	return IO.GetCharSet();
}


//=================================================================================


VDB4DCheckAndRepairAgent::VDB4DCheckAndRepairAgent(CDB4DBase* inTarget):
	fAgent(VImpCreator<VDB4DBase>::GetImpObject( inTarget)->GetBase())
{
	fTarget = VImpCreator<VDB4DBase>::GetImpObject( inTarget);
	fTarget->Retain();
}


VDB4DCheckAndRepairAgent::~VDB4DCheckAndRepairAgent()
{
	fTarget->Release();
}


VError VDB4DCheckAndRepairAgent::Run(VStream* outMsg, ListOfErrors& OutList, CDB4DBaseContext* inContext, VDB4DProgressIndicator* InProgress)
{
	VError err;
	err = fAgent.Run(outMsg, OutList, ConvertContext(inContext), InProgress);

	return err;
}

CDB4DBase* VDB4DCheckAndRepairAgent::GetOwner() const
{
	return fTarget;
}


void VDB4DCheckAndRepairAgent::SetCheckTableState(Boolean OnOff)
{
		fAgent.SetCheckTableState(OnOff);
}

void VDB4DCheckAndRepairAgent::SetCheckAllTablesState(Boolean OnOff)
{
		fAgent.SetCheckAllTablesState(OnOff);
}

void VDB4DCheckAndRepairAgent::SetCheckIndexState(Boolean OnOff)
{
		fAgent.SetCheckIndexState(OnOff);
}

void VDB4DCheckAndRepairAgent::SetCheckAllIndexesState(Boolean OnOff)
{
		fAgent.SetCheckAllIndexesState(OnOff);
}

void VDB4DCheckAndRepairAgent::SetCheckBlobsState(Boolean OnOff)
{
		fAgent.SetCheckBlobsState(OnOff);
}



//=================================================================================


VDB4DRelation::VDB4DRelation(CDB4DBase* inTarget, Relation* xrel)
{
	fTarget = VImpCreator<VDB4DBase>::GetImpObject(inTarget);
	fTarget->Retain("VDB4DRelation");

	if (xrel != NULL)
		xrel->Retain();	
	fRel = xrel;
}

VDB4DRelation::~VDB4DRelation()
{
	fTarget->Release("VDB4DRelation");

	if (fRel != NULL)
		fRel->Release();
}



VErrorDB4D VDB4DRelation::ActivateManyToOneS(CDB4DRecord *InRec, CDB4DQueryOptions& inOptions, CDB4DQueryResult* &OutResult, CDB4DBaseContextPtr inContext, 
									  Boolean OldOne)
{
	VError err = VE_OK;

	VDB4DQueryResult* xresult = nil;
	QueryOptions* options = VImpCreator<VDB4DQueryOptions>::GetImpObject(&inOptions)->GetOptions();
	BaseTaskInfo* context = ConvertContext(inContext);
	if (fRel != nil)
	{
		if (fRel->IsRemoteLike())
		{
			if (InRec == nil)
			{ 
				CDB4DTable* target = RetainDestinationTable();
				if (target != nil)
				{
					xresult = new VDB4DQueryResult(target);
					target->Release();
				}
			}
			else
			{
				IRequest *req = fRel->GetOwner()->CreateRequest( inContext, Req_ActivateManyToOneS + kRangeReqDB4D);
				if (req == nil)
				{
					err = ThrowBaseError(memfull, noaction);
				}
				else
				{
					req->PutBaseParam( fRel->GetOwner());
					req->PutThingsToForget( VDBMgr::GetManager(), context);
					req->PutRelationParam(fRel);
					req->PutFicheInMemParam( (FicheInMem*)InRec->GetFicheInMem(), inContext);
					req->PutQueryOptionsParam(options, context);
					req->PutBooleanParam(OldOne);

					err = req->GetLastError();
					if (err == VE_OK)
					{
						err = req->Send();
						if (err== VE_OK)
							err = req->GetUpdatedInfo(fRel->GetOwner(), context);

						if (err == VE_OK)
						{
							CDB4DTable* target = RetainDestinationTable();
							if (target != nil)
							{
								xresult = new VDB4DQueryResult(target);
								if (xresult == nil)
									err = ThrowBaseError(memfull, noaction);
								else
								{
									QueryResult& result = xresult->GetResult();
									err = req->ReadQueryResultReply( fRel->GetOwner(), fRel->GetDestTable(), &(xresult->GetResult()), context);
								}
								target->Release();
							}
						}
					}
					req->Release();
				}
			}
		}
		else
		{
			CDB4DTable* target = RetainDestinationTable();
			if (target != nil)
			{
				xresult = new VDB4DQueryResult(target);
				QueryResult& result = xresult->GetResult();
				if (InRec != nil)
				{
					FicheInMem* xrec = (FicheInMem*)InRec->GetFicheInMem();
					Selection* selresult;
					err = fRel->ActivateManyToOneS(xrec, selresult, context, OldOne, options->GetWayOfLocking());
					result.SetSelection(selresult);
					if (selresult != nil && options->GetWantsFirstRecord() && !selresult->IsEmpty())
					{
						FicheInMem* firstrect = selresult->GetParentFile()->LoadRecord(selresult->GetFic(0), err, options->GetRecordWayOfLocking(), context, true);
						result.SetFirstRecord(firstrect);
						ReleaseRefCountable(&firstrect);
					}
					ReleaseRefCountable(&selresult);
				}
				target->Release();
			}
		}
	}
	else
		err = ThrowBaseError(VE_DB4D_RELATIONISEMPTY, noaction);

	OutResult = xresult;
	return err;
}


VErrorDB4D VDB4DRelation::ActivateManyToOne(CDB4DRecord *InRec, CDB4DQueryOptions& inOptions, CDB4DQueryResult* &OutResult, CDB4DBaseContextPtr inContext, 
									 Boolean OldOne, Boolean NoCache)
{
	VError err = VE_OK;

	VDB4DQueryResult* xresult = nil;
	QueryOptions* options = VImpCreator<VDB4DQueryOptions>::GetImpObject(&inOptions)->GetOptions();
	BaseTaskInfo* context = ConvertContext(inContext);
	if (fRel != nil)
	{
		if (fRel->IsRemoteLike())
		{
			if (InRec == nil)
			{ 
				CDB4DTable* target = RetainDestinationTable();
				if (target != nil)
				{
					xresult = new VDB4DQueryResult(target);
					target->Release();
				}
			}
			else
			{
				IRequest *req = fRel->GetOwner()->CreateRequest( inContext, Req_ActivateManyToOne + kRangeReqDB4D);
				if (req == nil)
				{
					err = ThrowBaseError(memfull, noaction);
				}
				else
				{
					req->PutBaseParam( fRel->GetOwner());
					req->PutThingsToForget( VDBMgr::GetManager(), context);
					req->PutRelationParam(fRel);
					req->PutFicheInMemParam( (FicheInMem*)InRec->GetFicheInMem(), inContext);
					req->PutQueryOptionsParam(options, context);
					req->PutBooleanParam(OldOne);
					req->PutBooleanParam(/*NoCache*/true);

					err = req->GetLastError();
					if (err == VE_OK)
					{
						err = req->Send();
						if (err== VE_OK)
							err = req->GetUpdatedInfo(fRel->GetOwner(), context);

						if (err == VE_OK)
						{
							CDB4DTable* target = RetainDestinationTable();
							if (target != nil)
							{
								xresult = new VDB4DQueryResult(target);
								if (xresult == nil)
									err = ThrowBaseError(memfull, noaction);
								else
								{
									QueryResult& result = xresult->GetResult();
									err = req->ReadQueryResultReply( fRel->GetOwner(), fRel->GetDestTable(), &(xresult->GetResult()), context);
									Selection* sel = new PetiteSel(fRel->GetOwner(), inContext, target->GetID());
									if (sel != nil)
									{
										if (result.GetFirstRecord() != nil)
										{
											sel->FixFic(1);
											sel->PutFic(0, result.GetFirstRecord()->GetNum());
										}
										result.SetSelection(sel);
									}
									QuickReleaseRefCountable(sel);
								}
								target->Release();
							}
						}
					}
					req->Release();
				}
			}
		}
		else
		{
			CDB4DTable* target = RetainDestinationTable();
			if (target != nil)
			{
				xresult = new VDB4DQueryResult(target);
				QueryResult& result = xresult->GetResult();
				if (InRec != nil)
				{
					FicheInMem* xrec = (FicheInMem*)InRec->GetFicheInMem();
					FicheInMem* recresult;
					err = fRel->ActivateManyToOne(xrec, recresult, context, OldOne, NoCache, options->GetRecordWayOfLocking());
					result.SetFirstRecord(recresult);
					Selection* sel = new PetiteSel(fRel->GetDestTable()->GetDF());
					if (sel != nil)
					{
						if (recresult != nil)
						{
							sel->FixFic(1);
							sel->PutFic(0, recresult->GetNum());
						}
						result.SetSelection(sel);
					}
					QuickReleaseRefCountable(sel);
					ReleaseRefCountable(&recresult);
				}
				target->Release();
			}
		}
	}
	else
		err = ThrowBaseError(VE_DB4D_RELATIONISEMPTY, noaction);

	OutResult = xresult;
	return err;
}


VErrorDB4D VDB4DRelation::ActivateOneToMany(CDB4DRecord *InRec, CDB4DQueryOptions& inOptions, CDB4DQueryResult* &OutResult, CDB4DBaseContextPtr inContext, 
									 Boolean OldOne, Boolean NoCache)
{
	VError err = VE_OK;

	VDB4DQueryResult* xresult = nil;
	QueryOptions* options = VImpCreator<VDB4DQueryOptions>::GetImpObject(&inOptions)->GetOptions();
	BaseTaskInfo* context = ConvertContext(inContext);
	if (fRel != nil)
	{
		if (fRel->IsRemoteLike())
		{
			if (InRec == nil)
			{ 
				CDB4DTable* target = RetainSourceTable();
				if (target != nil)
				{
					xresult = new VDB4DQueryResult(target);
					target->Release();
				}
			}
			else
			{
				IRequest *req = fRel->GetOwner()->CreateRequest( inContext, Req_ActivateOneToMany + kRangeReqDB4D);
				if (req == nil)
				{
					err = ThrowBaseError(memfull, noaction);
				}
				else
				{
					req->PutBaseParam( fRel->GetOwner());
					req->PutThingsToForget( VDBMgr::GetManager(), context);
					req->PutRelationParam(fRel);
					req->PutFicheInMemParam( (FicheInMem*)InRec->GetFicheInMem(), inContext);
					req->PutQueryOptionsParam(options, context);
					req->PutBooleanParam(OldOne);
					req->PutBooleanParam(/*NoCache*/true);

					err = req->GetLastError();
					if (err == VE_OK)
					{
						err = req->Send();
						if (err== VE_OK)
							err = req->GetUpdatedInfo(fRel->GetOwner(), context);
						if (err == VE_OK)
						{
							CDB4DTable* target = RetainDestinationTable();
							if (target != nil)
							{
								xresult = new VDB4DQueryResult(target);
								if (xresult == nil)
									err = ThrowBaseError(memfull, noaction);
								else
								{
									QueryResult& result = xresult->GetResult();
									err = req->ReadQueryResultReply( fRel->GetOwner(), fRel->GetDestTable(), &(xresult->GetResult()), context);
								}
								target->Release();
							}
						}
					}
					req->Release();
				}
			}
		}
		else
		{
			CDB4DTable* target = RetainSourceTable();
			if (target != nil)
			{
				xresult = new VDB4DQueryResult(target);
				QueryResult& result = xresult->GetResult();
				if (InRec != nil)
				{
					FicheInMem* xrec = (FicheInMem*)InRec->GetFicheInMem();
					Selection* selresult;
					err = fRel->ActivateOneToMany(xrec, selresult, context, OldOne, NoCache, options->GetWayOfLocking());
					result.SetSelection(selresult);
					if (selresult != nil && options->GetWantsFirstRecord() && !selresult->IsEmpty())
					{
						FicheInMem* firstrect = selresult->GetParentFile()->LoadRecord(selresult->GetFic(0), err, options->GetRecordWayOfLocking(), context, true);
						result.SetFirstRecord(firstrect);
						ReleaseRefCountable(&firstrect);
					}
					ReleaseRefCountable(&selresult);
				}
				target->Release();
			}
		}
	}
	else
		err = ThrowBaseError(VE_DB4D_RELATIONISEMPTY, noaction);

	OutResult = xresult;
	return err;
}



	
VError VDB4DRelation::ActivateManyToOne( CDB4DRecord *InRec, CDB4DRecord* &OutResult, CDB4DBaseContextPtr inContext, Boolean OldOne, Boolean NoCache, Boolean ReadOnly)
{
	VError err = VE_OK;
	OutResult = nil;

	BaseTaskInfoPtr context = nil;
	if (inContext != nil)
	{
		context = ConvertContext(inContext);
	}

	if (fRel != nil)
	{
		if (InRec != nil)
		{
			if (fRel->IsRemoteLike())
			{
				CDB4DQueryOptions* options = fTarget->NewQueryOptions();
				options->SetWantsFirstRecord(true);
				options->SetLimit(1);
				options->SetWayOfLockingForFirstRecord(ReadOnly ? DB4D_Do_Not_Lock : DB4D_Keep_Lock_With_Record);
				CDB4DQueryResult* xresult = nil;
				err = ActivateManyToOne(InRec, *options, xresult, inContext, OldOne, NoCache);
				if (err == VE_OK)
				{
					OutResult = xresult->GetFirstRecord();
					if (OutResult != nil)
						OutResult->Retain();
				}
				ReleaseRefCountable(&xresult);
				options->Release();
				
			}
			else
			{
				VDB4DRecord* rec = VImpCreator<VDB4DRecord>::GetImpObject(InRec);
				FicheInMem* xrec = (FicheInMem*)rec->GetFicheInMem();
				FicheInMem* result;

				err = fRel->ActivateManyToOne(xrec, result, context, OldOne, NoCache, ReadOnly ? DB4D_Do_Not_Lock : DB4D_Keep_Lock_With_Record);
				if (result == nil)
					OutResult = nil;
				else
				{
					OutResult = new VDB4DRecord(VDBMgr::GetManager(), result, inContext);
					if (OutResult == nil)
					{
						err = fRel->ThrowError(memfull, DBaction_BuildingRecord);
						result->Release();
					}
				}
			}
		}
		else
			OutResult = nil;
	}
	else
		err = ThrowBaseError(VE_DB4D_RELATIONISEMPTY, noaction);

	return err;
}


VError VDB4DRelation::ActivateOneToMany( CDB4DRecord *InRec, CDB4DSelection* &OutResult, CDB4DBaseContextPtr inContext, Boolean OldOne, Boolean NoCache, Boolean LockedSel)
{
	VError err = VE_OK;

	BaseTaskInfoPtr context = nil;
	if (inContext != nil)
	{
		context = ConvertContext(inContext);
	}

	if (fRel != nil)
	{
		if (InRec != nil)
		{
			if (fRel->IsRemoteLike())
			{
				CDB4DQueryOptions* options = fTarget->NewQueryOptions();
				options->SetWayOfLocking(LockedSel ? DB4D_Keep_Lock_With_Transaction : DB4D_Do_Not_Lock);
				CDB4DQueryResult* xresult = nil;
				err = ActivateOneToMany(InRec, *options, xresult, inContext, OldOne, NoCache);
				if (err == VE_OK)
				{
					OutResult = xresult->GetSelection();
					if (OutResult != nil)
						OutResult->Retain();
				}
				ReleaseRefCountable(&xresult);
				options->Release();
			}
			else
			{
				VDB4DRecord* rec = VImpCreator<VDB4DRecord>::GetImpObject(InRec);
				FicheInMem* xrec = (FicheInMem*)rec->GetFicheInMem();
				Selection* result;

				err = fRel->ActivateOneToMany(xrec, result, context, OldOne, NoCache, LockedSel ? DB4D_Keep_Lock_With_Transaction : DB4D_Do_Not_Lock);
				if (result == nil)
					OutResult = nil;
				else
				{
					Table* assoctable = result->GetParentFile()->RetainTable();
					if (assoctable != nil)
					{
						OutResult = new VDB4DSelection(VDBMgr::GetManager(), fTarget, assoctable, result);
						assoctable->Release();
					}
					else
					{
						result->Release();
						OutResult = nil;
					}
				}
			}
		}
		else
			OutResult = nil;
	}
	else
		err = ThrowBaseError(VE_DB4D_RELATIONISEMPTY, noaction);

	return err;
}


VError VDB4DRelation::ActivateManyToOneS( CDB4DRecord *InRec, CDB4DSelection* &OutResult, CDB4DBaseContextPtr inContext, Boolean OldOne, Boolean LockedSel)
{
	VError err = VE_OK;

	BaseTaskInfoPtr context = nil;
	if (inContext != nil)
	{
		context = ConvertContext(inContext);
	}

	if (fRel != nil)
	{
		if (InRec != nil)
		{
			if (fRel->IsRemoteLike())
			{
				CDB4DQueryOptions* options = fTarget->NewQueryOptions();
				options->SetWayOfLocking(LockedSel ? DB4D_Keep_Lock_With_Transaction : DB4D_Do_Not_Lock);
				CDB4DQueryResult* xresult = nil;
				err = ActivateManyToOneS(InRec, *options, xresult, inContext, OldOne);
				if (err == VE_OK)
				{
					OutResult = xresult->GetSelection();
					if (OutResult != nil)
						OutResult->Retain();
				}
				ReleaseRefCountable(&xresult);
				options->Release();
			}
			else
			{
				VDB4DRecord* rec = VImpCreator<VDB4DRecord>::GetImpObject(InRec);
				FicheInMem* xrec = (FicheInMem*)rec->GetFicheInMem();
				Selection* result;

				err = fRel->ActivateManyToOneS(xrec, result, context, OldOne, LockedSel ? DB4D_Keep_Lock_With_Transaction : DB4D_Do_Not_Lock);
				if (result == nil)
					OutResult = nil;
				else
				{
					Table* assoctable = result->GetParentFile()->RetainTable();
					if (assoctable != nil)
					{
						OutResult = new VDB4DSelection(VDBMgr::GetManager(), fTarget, assoctable, result);
						assoctable->Release();
					}
					else
					{
						result->Release();
						OutResult = nil;
					}
				}
			}
		}
		else
			OutResult = nil;
	}
	else
		err = ThrowBaseError(VE_DB4D_RELATIONISEMPTY, noaction);

	return err;
}



#if 0
VError VDB4DRelation::SetFields(DB4D_RelationType inWhatType, const VString& inName, CDB4DField* inSourceField, CDB4DField* inDestinationField)
{
	VError err = VE_OK;

	if (inSourceField != nil && inDestinationField != nil)
	{
		Base4D* bd = fTarget->GetBase();
		fRel = bd->CreateRelation(inWhatType, inName
			, VImpCreator<VDB4DField>::GetImpObject(inSourceField)->GetField()
			, VImpCreator<VDB4DField>::GetImpObject(inDestinationField)->GetField(), err);

	}

	return err;
}
#endif


VError VDB4DRelation::Drop(CDB4DBaseContextPtr inContext)
{
	VError err = VE_OK;

	if (fRel != nil && fRel->GetOwner() != nil)
	{
		ObjLocker locker(inContext, fRel);
		if (locker.CanWork())
		{
			err = fRel->GetOwner()->DeleteRelation(fRel, inContext);
			//fRel->GetOwner()->SaveRelations();
		}
		else
			err = VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT;
		fRel->Release();
		fRel = nil;
	}

	return err;
}


void VDB4DRelation::GetNameNto1(VString& outName, CDB4DBaseContextPtr inContext) const
{
	if (fRel != nil)
		fRel->GetName(outName);
	else
		outName.Clear();
}


void VDB4DRelation::GetName1toN(VString& outName, CDB4DBaseContextPtr inContext) const
{
	if (fRel != nil)
		fRel->GetOppositeName(outName);
	else
		outName.Clear();
}


CDB4DTable* VDB4DRelation::RetainSourceTable()
{
	CDB4DTable* result = nil;
	if (fRel != nil)
	{
		Field* cri = fRel->GetSource();
		if (cri != nil)
		{
			Table* tt = cri->GetOwner();
			if (tt != nil)
			{
				result = new VDB4DTable(VDBMgr::GetManager(), fTarget, tt);
			}
		}
	}

	return result;
}


CDB4DTable* VDB4DRelation::RetainDestinationTable()
{
	CDB4DTable* result = nil;
	if (fRel != nil)
	{
		Field* cri = fRel->GetDest();
		if (cri != nil)
		{
			Table* tt = cri->GetOwner();
			if (tt != nil)
			{
				result = new VDB4DTable(VDBMgr::GetManager(), fTarget, tt);
			}
		}
	}

	return result;
}



void VDB4DRelation::RetainSource(CDB4DField* &outSourceField, CDB4DBaseContextPtr inContext) const
{
	outSourceField = nil;
	if (fRel != nil)
	{
		Field *cri,*rien;
		fRel->GetDef(cri,rien);
		if (cri != nil)
		{
			outSourceField = new VDB4DField(cri);
		}
	}
}


void VDB4DRelation::RetainDestination(CDB4DField* &outDestinationField, CDB4DBaseContextPtr inContext) const
{
	outDestinationField = nil;
	if (fRel != nil)
	{
		Field *cri,*rien;
		fRel->GetDef(rien,cri);
		if (cri != nil)
		{
			outDestinationField = new VDB4DField(cri);
		}
	}
}


VError VDB4DRelation::RetainSources(CDB4DFieldArray& outSourceFields, CDB4DBaseContextPtr inContext) const
{
	VErrorDB4D err = VE_OK;
	outSourceFields.SetCount(0);

	if (fRel != nil)
	{
		for ( FieldArray::ConstIterator cur = fRel->GetSources().First(), end = fRel->GetSources().End() ; cur != end && err == VE_OK ; ++cur )
		{
			Field *cri = *cur;
			if (cri != NULL)
			{
				CDB4DField *xcri = new VDB4DField(cri);
				if (!outSourceFields.Add(xcri))
					err = fRel->ThrowError(memfull, DBaction_BuildingFieldList);
			}
		}
	}
	return err;
}


VError VDB4DRelation::RetainDestinations(CDB4DFieldArray& outDestinationFields, CDB4DBaseContextPtr inContext) const
{
	VErrorDB4D err = VE_OK;
	outDestinationFields.SetCount(0);

	if (fRel != nil)
	{
		for ( FieldArray::ConstIterator cur = fRel->GetDestinations().First(), end = fRel->GetDestinations().End() ; cur != end && err == VE_OK ; ++cur )
		{
			Field *cri = *cur;
			if (cri != NULL)
			{
				CDB4DField *xcri = new VDB4DField(cri);
				if (!outDestinationFields.Add(xcri))
					err = fRel->ThrowError(memfull, DBaction_BuildingFieldList);
			}
		}
	}
	return err;
}


/*
DB4D_RelationType VDB4DRelation::GetType(CDB4DBaseContextPtr inContext) const
{
	if (fRel != nil)
		return fRel->GetType();
	else
		return (DB4D_RelationType)0;
}
*/


void VDB4DRelation::GetUUID(VUUID& outID, CDB4DBaseContextPtr inContext) const
{
	if (fRel != nil)
	{
		outID = fRel->GetUUID();
	}
}


VError VDB4DRelation::SetNameNto1(const VString& inName, CDB4DBaseContextPtr inContext)
{
	VError err = VE_OK;

	if (fRel == nil)
		err = VE_DB4D_RELATIONISEMPTY;
	else
	{
		ObjLocker locker(inContext, fRel);
		if (locker.CanWork())
		{
			err = fRel->SetName(inName, inContext);
			//fRel->GetOwner()->SaveRelations();
		}
		else
			err = VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT;
	}

	return err;
}


VError VDB4DRelation::SetName1toN(const VString& inName, CDB4DBaseContextPtr inContext)
{
	VError err = VE_OK;

	if (fRel == nil)
		err = VE_DB4D_RELATIONISEMPTY;
	else
	{
		ObjLocker locker(inContext, fRel);
		if (locker.CanWork())
		{
			err = fRel->SetOppositeName(inName, inContext);
			//fRel->GetOwner()->SaveRelations();
		}
		else
			err = VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT;
	}

	return err;
}


VError VDB4DRelation::SetAutoLoadNto1(Boolean inState, CDB4DBaseContextPtr inContext)
{
	VError err = VE_OK;

	if (fRel == nil)
		err = VE_DB4D_RELATIONISEMPTY;
	else
	{
		ObjLocker locker(inContext, fRel);
		if (locker.CanWork())
		{
			err = fRel->SetAutoNto1Load(inState, inContext);
			//fRel->GetOwner()->SaveRelations();
		}
		else
			err = VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT;
	}

	return err;
}


Boolean VDB4DRelation::IsAutoLoadNto1(CDB4DBaseContextPtr inContext) const
{
	if (fRel == nil)
		return false;
	else
		return fRel->IsAutoLoadNto1();
}


VError VDB4DRelation::SetAutoLoad1toN(Boolean inState, CDB4DBaseContextPtr inContext)
{
	VError err = VE_OK;

	if (fRel == nil)
		err = VE_DB4D_RELATIONISEMPTY;
	else
	{
		ObjLocker locker(inContext, fRel);
		if (locker.CanWork())
		{
			err = fRel->SetAuto1toNLoad(inState, inContext);
			//fRel->GetOwner()->SaveRelations();
		}
		else
			err = VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT;
	}

	return err;
}


Boolean VDB4DRelation::IsAutoLoad1toN(CDB4DBaseContextPtr inContext) const
{
	if (fRel == nil)
		return false;
	else
		return fRel->IsAutoLoad1toN();
}




VError VDB4DRelation::SetState(DB4D_RelationState InState, CDB4DBaseContextPtr inContext)
{
	VError err = VE_OK;

	if (fRel == nil)
		err = VE_DB4D_RELATIONISEMPTY;
	else
	{
		ObjLocker locker(inContext, fRel);
		if (locker.CanWork())
		{
			err = fRel->SetState(InState);
			//fRel->GetOwner()->SaveRelations();
		}
		else
			err = VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT;
	}

	return err;
}


DB4D_RelationState VDB4DRelation::GetState(CDB4DBaseContextPtr inContext) const
{
	if (fRel == nil)
		return (DB4D_RelationState)0;
	else
	{
		return fRel->GetState();
	}
}


VValueBag *VDB4DRelation::CreateDefinition(CDB4DBaseContextPtr inContext) const
{
	if (fRel == NULL)
		return NULL;
	
	VValueBag *bag = new VValueBag;
	if (bag != NULL)
	{
		VString kind;
		VError err = fRel->SaveToBag( *bag, kind);
		vThrowError( err);
	}

	return bag;
}


VErrorDB4D VDB4DRelation::SetReferentialIntegrity(Boolean inReferentialIntegrity, Boolean inAutoDeleteRelatedRecords, CDB4DBaseContextPtr inContext)
{
	VError err;
	if (fRel == nil)
		err = VE_DB4D_RELATIONISEMPTY;
	else
	{
		ObjLocker locker(inContext, fRel);
		if (locker.CanWork())
		{
			err = fRel->SetReferentialIntegrity(inReferentialIntegrity, inAutoDeleteRelatedRecords, inContext);
			/*
			if (err == VE_OK)
			{
				err = fRel->GetOwner()->SaveRelations();
			}
			*/
		}
		else
			err = VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT;
	}
	return err;
}


Boolean VDB4DRelation::isReferentialIntegrity(CDB4DBaseContextPtr inContext)
{
	return fRel->WithReferentialIntegrity();
}


Boolean VDB4DRelation::isAutoDeleteRelatedRecords(CDB4DBaseContextPtr inContext)
{
	return fRel->AutoDeleteRelatedRecords();
}


VError VDB4DRelation::SetForeignKey(Boolean on, CDB4DBaseContextPtr inContext)
{
	VError err = VE_OK;

	if (fRel == nil)
		err = VE_DB4D_RELATIONISEMPTY;
	else
	{
		ObjLocker locker(inContext, fRel);
		if (locker.CanWork())
		{
			err = fRel->SetForeignKey(on, inContext);
		}
		else
			err = VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT;
	}

	return err;
}


Boolean VDB4DRelation::isForeignKey(CDB4DBaseContextPtr inContext) const
{
	if (fRel == nil)
		return false;
	else
		return fRel->IsForeignKey();
}


const VValueBag* VDB4DRelation::RetainExtraProperties(VError &err, CDB4DBaseContextPtr inContext)
{
	if (fRel == nil)
	{
		err = VE_DB4D_RELATIONISEMPTY;
		return nil;
	}
	else
		return fRel->RetainExtraProperties(err, inContext);
}


VError VDB4DRelation::SetExtraProperties(VValueBag* inExtraProperties, CDB4DBaseContextPtr inContext)
{
	if (fRel == nil)
		return VE_DB4D_RELATIONISEMPTY;
	else
	{
		ObjLocker locker(inContext, fRel);
		if (locker.CanWork())
		{
			return fRel->SetExtraProperties(inExtraProperties, true, inContext);
		}
		else
			return VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT;
	}
}


sLONG VDB4DRelation::GetStamp(CDB4DBaseContextPtr inContext) const
{
	if (fRel == nil)
		return 0;
	else
		return fRel->GetStamp();
}


sLONG VDB4DRelation::GetPosInList(CDB4DBaseContextPtr inContext) const
{
	if (fRel == nil)
		return -1;
	else
		return fRel->GetPosInList();
}


//=================================================================================


VDB4DIndex::VDB4DIndex(IndexInfo* xInd)
{
	assert(xInd != nil);
	fInd = xInd;
	fInd->Retain();
}


VDB4DIndex::~VDB4DIndex()
{
	fInd->Release();
}


sLONG VDB4DIndex::GetSourceType(CDB4DBaseContextPtr inContext)
{
	return fInd->GetReducedType();		// sc 04/07/2007 return the reduced type
}


sLONG VDB4DIndex::GetStorageType(CDB4DBaseContextPtr inContext)
{
	sLONG res = 0;

	if (fInd->IsRemote())
	{
		// sc 13/12/2007, remote indexes have not header
		res = fInd->GetHeaderType();
	}
	else
	{
		IndexHeader* head = fInd->GetHeader();
		if (head != nil)
		{
			res = head->GetRealType();
		}
	}
	return res;
}


Boolean VDB4DIndex::IsTypeAuto(CDB4DBaseContextPtr inContext)
{
	return fInd->IsAuto();
}


const	XBOX::VString& VDB4DIndex::GetName(CDB4DBaseContextPtr inContext) const
{
	return fInd->GetName();
}


void VDB4DIndex::GetUUID(XBOX::VUUID& outID, CDB4DBaseContextPtr inContext) const
{
	outID = fInd->GetID();
}


VErrorDB4D VDB4DIndex::SetName(const XBOX::VString& inName, CDB4DBaseContextPtr inContext)
{
	ObjLocker locker(inContext, fInd);
	if (locker.CanWork())
	{
		return fInd->SetName( inName, inContext);
	}
	else
		return VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT;
}


CDB4DField* VDB4DIndex::RetainDataSource(CDB4DBaseContextPtr inContext)
{
	CDB4DField* xcri = nil;

	if (fInd->MatchType(DB4D_Index_OnOneField) || fInd->MatchType(DB4D_Index_OnKeyWords))
	{
		IndexInfoFromField* xind = (IndexInfoFromField*) fInd;
		Field* cri = xind->GetField();
		if (cri != nil)
		{
			xcri = new VDB4DField(cri);
		}
	}
	return xcri;
}


VErrorDB4D VDB4DIndex::RetainDataSources(CDB4DFieldArray& outSources, CDB4DBaseContextPtr inContext)
{
	VErrorDB4D err = VE_OK;
	outSources.SetCount(0);

	switch (fInd->GetReducedType())
	{
		case DB4D_Index_OnOneField:
		case DB4D_Index_OnKeyWords:
			{
				IndexInfoFromField* xind = (IndexInfoFromField*) fInd;
				Field* cri = xind->GetField();
				if (cri != nil)
				{
					CDB4DField* xcri = new VDB4DField(cri);
					if (!outSources.Add(xcri))
						err = fInd->ThrowError(memfull, DBaction_BuildingFieldList);
				}
			}
			break;

		case DB4D_Index_OnMultipleFields:
			{
				IndexInfoFromMultipleField* xindm = (IndexInfoFromMultipleField*) fInd;
				FieldRefArray* fields = xindm->GetFields();

				FieldRef *cur, *end = fields->AfterLast();

				for (cur = fields->First(); cur != end && err == VE_OK; cur++)
				{
					Field* cri = cur->crit;
					if (testAssert(cri != nil))
					{
						CDB4DField* xcri = new VDB4DField(cri);
						if (!outSources.Add(xcri))
							err = fInd->ThrowError(memfull, DBaction_BuildingFieldList);
					}
				}
			}
			break;
	}

	return err;
}


CDB4DTable* VDB4DIndex::RetainTargetReference(CDB4DBaseContextPtr inContext)
{
	return nil;
}


Boolean VDB4DIndex::MayBeSorted(CDB4DBaseContextPtr inContext)
{
	return fInd->MayBeSorted();
}


VErrorDB4D VDB4DIndex::Drop(VDB4DProgressIndicator* InProgress, VSyncEvent* event, CDB4DBaseContextPtr inContext)
{
	// ObjLocker locker(inContext, fInd);
	// if (locker.CanWork())
	{
		fInd->GetDB()->DeleteIndexByRef(fInd, inContext, InProgress, event);
		return VE_OK;
	}
	// else
		// return VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT;
}

static Bittab* GetBittabFromCDB4DSelectionPtr(CDB4DSelectionPtr xsel, BaseTaskInfoPtr context, VError& err)
{
	Selection* sel = nil;
	Bittab* result = nil;

	if (xsel != nil)
	{
		sel = VImpCreator<VDB4DSelection>::GetImpObject(xsel)->GetSel();
	}

	if (sel != nil)
	{
		result = sel->GenereBittab(context, err);
	}

	return result;
}


RecIDType VDB4DIndex::FindKey(const VValueSingle &inValToFind, VErrorDB4D& err, CDB4DBaseContext* inContext, VValueSingle* *outResult, CDB4DSelectionPtr Filter, const VCompareOptions& inOptions)
{
	RecIDType res = -1;
	if (outResult != nil)
		*outResult = nil;
	err = VE_OK;

	BaseTaskInfoPtr context = nil;
	if (inContext != nil)
	{
		context = ConvertContext(inContext);
		assert(context->GetBase() == fInd->GetDB());
	}

	if (fInd->IsRemote())
	{
		IRequest *req = fInd->GetDB()->CreateRequest( inContext, Req_FindKey + kRangeReqDB4D);
		if (req == nil)
		{
			err = ThrowBaseError(memfull, noaction);
		}
		else
		{
			req->PutBaseParam(fInd->GetDB());
			req->PutThingsToForget( VDBMgr::GetManager(), context);
			req->PutIndexParam(fInd);
			if (Filter == nil)
				req->PutLongParam(sel_nosel);
			else
				req->PutSelectionParam(VImpCreator<VDB4DSelection>::GetImpObject(Filter)->GetSel(), inContext);
			PutVCompareOptionsIntoStream(inOptions, *(req->GetOutputStream()));
			req->PutValueParam(&inValToFind);
			err = req->GetLastError();
			if (err == VE_OK)
			{
				err = req->Send();
				if (err == VE_OK)
				{
					res = req->GetLongReply( err);
					if (err == VE_OK)
					{
						if (outResult != nil)
						 *outResult = req->BuildValueReply(err);
					}
				}
			}
			req->Release();
		}

	}
	else
	{
		Bittab* dejabitsel = GetBittabFromCDB4DSelectionPtr(Filter, context, err);

		if (err == VE_OK)
		{
			if (fInd->MatchType(DB4D_Index_OnOneField) || fInd->MatchType(DB4D_Index_OnKeyWords))
			{
				BTitemIndex* key = ((IndexInfoFromField*)fInd)->BuildKeyFromVValue((const ValPtr)&inValToFind, err);
				if (key != nil)
				{
					BTitemIndex* val = nil;
					BTitemIndex** pval = nil;
					if (outResult != nil)
						pval = &val;
					
					VCompareOptions options( inOptions);

					res = fInd->FindKey(key, context, options, dejabitsel, pval);
					if (val != nil)
					{
						*outResult = ((IndexInfoFromField*)fInd)->CreateVValueWithKey(val, err);
						fInd->FreeKey(val);
					}
					fInd->FreeKey(key);
				}
			}
			else
			{
				err = VE_DB4D_WRONGINDEXTYP;
			}
		}
		ReleaseRefCountable(&dejabitsel);
	}

	return res;
}


RecIDType VDB4DIndex::FindKey(const ListOfValues &inValToFind, VErrorDB4D& err, CDB4DBaseContext* inContext, ListOfValues* *outResult, CDB4DSelectionPtr Filter, const VCompareOptions& inOptions)
{
	RecIDType res = -1;
	if (outResult != nil)
		*outResult = nil;
	err = VE_OK;

	BaseTaskInfoPtr context = nil;
	if (inContext != nil)
	{
		context = ConvertContext(inContext);
		assert(context->GetBase() == fInd->GetDB());
	}

	Bittab* dejabitsel = GetBittabFromCDB4DSelectionPtr(Filter, context, err);

	if (fInd->MatchType(DB4D_Index_OnMultipleFields))
	{
		BTitemIndex* key = ((IndexInfoFromMultipleField*)fInd)->BuildKeyFromVValues(inValToFind, err);
		if (key != nil)
		{
			BTitemIndex* val = nil;
			BTitemIndex** pval = nil;
			if (outResult != nil)
				pval = &val;

			res = fInd->FindKey(key, context, inOptions, dejabitsel, pval);
			if (val != nil)
			{
				ListOfValues* lv = new ListOfValues;
				if (lv == nil)
					err = fInd->ThrowError(memfull, DBaction_BuildingArrayOfValues);
				else
				{
					err = ((IndexInfoFromMultipleField*)fInd)->BuildVValuesFromKey(val, *lv);
					if (err != VE_OK)
					{
						delete lv;
						lv = nil;
					}
				}
				*outResult = lv;
				fInd->FreeKey(val);
			}
			fInd->FreeKey(key);
		}
	}
	else
	{
		err = fInd->ThrowError(VE_DB4D_WRONGINDEXTYP, DBaction_BuildingArrayOfValues);
	}

	ReleaseRefCountable(&dejabitsel);

	return res;
}


CDB4DIndexKey* VDB4DIndex::FindIndexKey(const VValueSingle &inValToFind, VErrorDB4D& err, CDB4DBaseContext* inContext, Boolean inBeginOfText)
{
	VDB4DIndexKey* res = nil;
	BaseTaskInfoPtr context = nil;
	if (inContext != nil)
	{
		context = ConvertContext(inContext);
		assert(context->GetBase() == fInd->GetDB());
	}

	if (fInd->MatchType(DB4D_Index_OnOneField) || fInd->MatchType(DB4D_Index_OnKeyWords))
	{
		BTitemIndex* key = ((IndexInfoFromField*)fInd)->BuildKeyFromVValue((const ValPtr)&inValToFind, err);
		if (key != nil)
		{
			res = new VDB4DIndexKey(fInd, context);
			VCompareOptions options;
			options.SetLike(false);
			options.SetBeginsWith(inBeginOfText);
			options.SetDiacritical(false);
			err = fInd->FindKeyAndBuildPath(key, options, context, res);

			//err = fInd->FindKeyAndBuildPath(key, false, inBeginOfText, true, context, res);
			fInd->FreeKey(key);
		}
	}
	else
	{
		err = VE_DB4D_WRONGINDEXTYP;
	}
	return res;
}


CDB4DIndexKey* VDB4DIndex::FindIndexKey(const ListOfValues &inValToFind, VErrorDB4D& err, CDB4DBaseContext* inContext, Boolean inBeginOfText)
{
	VDB4DIndexKey* res = nil;
	BaseTaskInfoPtr context = nil;
	if (inContext != nil)
	{
		context = ConvertContext(inContext);
		assert(context->GetBase() == fInd->GetDB());
	}

	if (fInd->MatchType(DB4D_Index_OnMultipleFields))
	{
		BTitemIndex* key = ((IndexInfoFromMultipleField*)fInd)->BuildKeyFromVValues(inValToFind, err);
		if (key != nil)
		{
			res = new VDB4DIndexKey(fInd, context);

			VCompareOptions options;
			options.SetLike(false);
			options.SetBeginsWith(inBeginOfText);
			options.SetDiacritical(false);
			err = fInd->FindKeyAndBuildPath(key, options, context, res);
			//err = fInd->FindKeyAndBuildPath(key, false, inBeginOfText, true, context, res);

			fInd->FreeKey(key);
		}
	}
	else
	{
		err = VE_DB4D_WRONGINDEXTYP;
	}
	return res;
}



CDB4DQueryResult* VDB4DIndex::ScanIndex(CDB4DQueryOptions& inOptions, Boolean KeepSorted, Boolean Ascent, CDB4DBaseContext* inContext, VErrorDB4D& outError)
{
	CDB4DQueryResult* xresult = nil;

	BaseTaskInfo* context = ConvertContext(inContext);
	QueryOptions* options = VImpCreator<VDB4DQueryOptions>::GetImpObject(&inOptions)->GetOptions();

	if (fInd->IsRemote())
	{
		IRequest *req = fInd->GetDB()->CreateRequest( inContext, Req_ScanIndex + kRangeReqDB4D);
		if (req == nil)
		{
			outError = ThrowBaseError(memfull, noaction);
		}
		else
		{
			req->PutBaseParam(fInd->GetDB());
			req->PutThingsToForget( VDBMgr::GetManager(), context);
			req->PutIndexParam(fInd);
			req->PutQueryOptionsParam(options, context);
			req->PutBooleanParam(KeepSorted);
			req->PutBooleanParam(Ascent);

			outError = req->GetLastError();
			if (outError == VE_OK)
			{
				outError = req->Send();
				if (outError== VE_OK)
					outError = req->GetUpdatedInfo(fInd->GetDB(), context);

				if (outError == VE_OK)
				{
					xresult = new VDB4DQueryResult(fInd->GetTargetTable());
					if (xresult == nil)
						outError = ThrowBaseError(memfull, noaction);
					else
					{
						QueryResult* result = &(VImpCreator<VDB4DQueryResult>::GetImpObject(xresult)->GetResult());
						outError = req->ReadQueryResultReply(fInd->GetDB(), fInd->GetTargetTable(), result, context);
					}
				}
			}
			req->Release();
		}
	}
	else
	{
		Selection* sel = fInd->ScanIndex(options->GetLimit(), KeepSorted, Ascent, context, outError, options->GetFilter(), nil);
		if (sel != nil)
		{
			xresult = new VDB4DQueryResult(fInd->GetTargetTable());
			QueryResult* result = &(VImpCreator<VDB4DQueryResult>::GetImpObject(xresult)->GetResult());
			result->SetSelection(sel);
			if (options->GetWantsFirstRecord() && !sel->IsEmpty())
			{
				VError err2;
				FicheInMem* rec = fInd->GetTargetTable()->GetDF()->LoadRecord(sel->GetFic(0), err2, options->GetRecordWayOfLocking(), context, true);
				if (rec != nil)
				{
					result->SetFirstRecord(rec);
					rec->Release();
				}
			}
			sel->Release();
		}
	}

	return xresult;
}



CDB4DSelection* VDB4DIndex::ScanIndex(RecIDType inMaxRecords, Boolean KeepSorted, Boolean Ascent, CDB4DBaseContext* inContext, VError& outError, CDB4DSelection* filter)
{
	CDB4DSelection* res = nil;
	Selection* filtre = nil;
	BaseTaskInfoPtr context = nil;
	VDB4DBase *base = nil;
	
	if (fInd->IsRemote())
	{
		CDB4DQueryOptions* options = new VDB4DQueryOptions;

		if (options != nil)
		{
			options->SetFilter(filter);
			options->SetLimit(inMaxRecords);
			CDB4DQueryResult* xresult = ScanIndex(*options, KeepSorted, Ascent, inContext, outError);
			if (xresult != nil)
			{
				res = xresult->GetSelection();
				if (res != nil)
					res->Retain();
				xresult->Release();
			}
			options->Release();
		}
		else
			outError = ThrowBaseError(memfull, noaction);
	}
	else
	{
		if (filter != nil)
			filtre = VImpCreator<VDB4DSelection>::GetImpObject(filter)->GetSel();

		if (inContext != nil)
		{
			BaseTaskInfo* context = ConvertContext(inContext);
			base = VImpCreator<VDB4DBase>::GetImpObject(context->GetOwner());
			assert(context->GetBase() == fInd->GetDB());
		}

		if (base != nil)
		{
			Selection* sel = fInd->ScanIndex(inMaxRecords, KeepSorted, Ascent, context, outError, filtre, nil);
			if (sel != nil)
			{
				Table *table = fInd->GetTargetTable();
				res = new VDB4DSelection( base->GetManager(), base, table, sel);
				if (res == nil)
				{
					sel->Release();
					outError = fInd->ThrowError(memfull, DBaction_BuildingSelection);
				}
			}
		}
	}

	return res;
}


Boolean VDB4DIndex::IsBuilding()
{
	return fInd->IsBuilding();
}

/*
void* VDB4DIndex::GetSourceLang()
{
	return (void*)fInd->GetSourceLang();
}
*/

CDB4DBase* VDB4DIndex::RetainOwner(const char* DebugInfo, CDB4DBaseContextPtr inContext)
{
	return fInd->GetDB()->RetainBaseX(DebugInfo);
}


void VDB4DIndex::ReleaseFromQuery()
{
	fInd->ReleaseValid();
}


VError VDB4DIndex::GetBuildError() const
{
	return fInd->GetBuildError();
}


Boolean VDB4DIndex::IsValid(CDB4DBaseContextPtr inContext)
{
	Boolean isvalid = false;
	BaseTaskInfoPtr context = nil;
	if (inContext != nil)
	{
		context = ConvertContext(inContext);
		assert(context->GetBase() == fInd->GetDB());
	}
	isvalid = fInd->AskForValid(context, true);
	if (isvalid)
		fInd->ReleaseValid();
	return isvalid;
}


void VDB4DIndex::FreeAndRelease()
{
	fInd->ReleaseValid();
	Release();
}


//=================================================================================


RecIDType VDB4DIndexKey::GetRecordID() const
{
	return fRecNum;
}


CDB4DRecord* VDB4DIndexKey::GetRecord(VError& err)
{
	err = VE_OK;
	if (fCurRec == nil)
	{
		if (fRecNum>=0)
		{
			Table* t = fInd->GetTargetTable();
			FicheInMem* fic = t->GetDF()->LoadRecord(fRecNum, err, fReadOnly ? DB4D_Do_Not_Lock : DB4D_Keep_Lock_With_Record, fContext, true, false, nil);
			if (fic != nil)
			{
				fCurRec = new VDB4DRecord(VDBMgr::GetManager(), fic, fContext->GetEncapsuleur());
				if (fCurRec == nil)
					fic->Release();
			}
		}
	}
	return fCurRec;
}


CDB4DIndexKey* VDB4DIndexKey::NextKey(VError& err) const
{
	VDB4DIndexKey* res = new VDB4DIndexKey(fInd, fContext);
	if (res == nil)
		err = fInd->ThrowError(memfull, DBaction_BuildingIndexKey);
	else
	{
		err = VE_OK;

		if (fRecNum>=0)
		{		
			fInd->NextKey(this, fContext, res);
		}
	}

	return res;
}





VDB4DIndexKey::VDB4DIndexKey(IndexInfo* xind, BaseTaskInfo* inContext)
{
	fContext = inContext;
	if (fContext != nil)
		fContext->Retain();
	assert(xind != nil);
	fInd = xind;
	fInd->Retain();
	fNbLevel = 0;
	fCurPos = -1;
	fCurPosInSel = -1;
	fRecNum = -1;
	fCurRec = nil;
	fReadOnly = false;
}


VDB4DIndexKey::~VDB4DIndexKey()
{
	if (fContext != nil)
		fContext->Release();
	if (fCurRec != nil)
		fCurRec->Release();
	fInd->Release();
}


void VDB4DIndexKey::AddLevelToPath(sLONG numpage)
{
	assert(fNbLevel<kMaxPageLevelInPath);
	fPagePath[fNbLevel] = numpage;
	fNbLevel++;

}


void VDB4DIndexKey::DecLevelFromPath()
{
	assert(fNbLevel>0);
	fNbLevel--;
}


void VDB4DIndexKey::SetCurPos(sLONG curpos, sLONG curposinsel, sLONG recnum)
{
	if (fCurRec != nil)
	{
		fCurRec->Release();
		fCurRec = nil;
	}
	fCurPos = curpos;
	fCurPosInSel = curposinsel;
	fRecNum = recnum;
}



//=================================================================================


VDB4DContext::~VDB4DContext()
{
	ContextByBaseMap::iterator cur = fContexts.begin(), end = fContexts.end();
	for (; cur != end; cur++)
	{
		contextElem* elem = &(cur->second);
		if (elem->fMustRelease)
			elem->fContext->Release();
	}

	if (fJSContext != nil)
	{
		VJSContext jscontext(fJSContext);
		for (map<sLONG, VJSObject>::iterator cur = fJSFuncs.begin(), end = fJSFuncs.end(); cur != end; cur++)
		{
			JS4D::UnprotectValue(jscontext, cur->second);
		}
	}

	QuickReleaseRefCountable(fUserSession);
	VDBMgr::GetManager()->UnRegisterContext(fID);
	QuickReleaseRefCountable( fExtra);
}


void VDB4DContext::FreeAllJSFuncs()
{
	if (fJSContext != nil)
	{
		VJSContext jscontext(fJSContext);
		for (map<sLONG, VJSObject>::iterator cur = fJSFuncs.begin(), end = fJSFuncs.end(); cur != end; cur++)
		{
			if ( cur->second != 0 )
				JS4D::UnprotectValue(jscontext, cur->second);
		}
		fJSFuncs.clear();

		for (ContextByBaseMap::iterator cur = fContexts.begin(), end = fContexts.end(); cur != end; cur++)
		{
			BaseTaskInfo* context = VImpCreator<BaseTaskInfo>::GetImpObject(cur->second.fContext);
			context->FreeAllJSMethods();
		}

	}
}

void VDB4DContext::CleanUpForReuse()
{
	for (ContextByBaseMap::iterator cur = fContexts.begin(), end = fContexts.end(); cur != end; cur++)
	{
		cur->second.fContext->CleanUpForReuse();
	}
}



void VDB4DContext::SetCurrentUser(const VUUID& inUserID, CUAGSession* inSession)
{
	for (ContextByBaseMap::iterator cur = fContexts.begin(), end = fContexts.end(); cur != end; cur++)
	{
		BaseTaskInfo* context = VImpCreator<BaseTaskInfo>::GetImpObject(cur->second.fContext);
		context->SetCurrentUser(inUserID, inSession);
	}

}


void VDB4DContext::SetExtraData( const XBOX::VValueBag* inExtra)
{
	CopyRefCountable( &fExtra, inExtra);
	fNeedsToSendExtraData = true;
}


const VValueBag* VDB4DContext::RetainExtraData() const
{
	return RetainRefCountable( fExtra);
}


CDB4DBaseContextPtr VDB4DContext::RetainDataBaseContext(CDB4DBase* inTarget, Boolean ForceCreate, bool reallyRetain)
{
	if (inTarget == nil)
		return nil;
	
	VTaskLock lock(&fMutex);

	CDB4DBaseContextPtr res = nil;

	Base4D* base = VImpCreator<VDB4DBase>::GetImpObject(inTarget)->GetBase();

	ContextByBaseMap::iterator found = fContexts.find(base);
	if (found != fContexts.end())
	{
		res = found->second.fContext;
	}

	if (ForceCreate && (res == nil))
	{
		res = inTarget->NewContext(fUserSession, fJSContext, fIsLocal);
		VImpCreator<BaseTaskInfo>::GetImpObject(res)->SetContextOwner(this);

		contextElem elem;
		elem.fContext = res;
		elem.fMustRelease = true;

		fContexts[base] = elem;
	}
	
	if (res != nil && reallyRetain)
	{
		res->Retain();
	}
	
	return res;
}


BaseTaskInfo* VDB4DContext::RetainDataBaseContext(Base4D* inTarget, Boolean ForceCreate, bool reallyRetain)
{
	if (inTarget == nil)
		return nil;
	CDB4DBase *basex = inTarget->RetainBaseX();
	CDB4DBaseContext* context = RetainDataBaseContext(basex, ForceCreate, reallyRetain);
	QuickReleaseRefCountable(basex);
	return ConvertContext(context);
}


void VDB4DContext::NowOwns(CDB4DBaseContextPtr context)
{
	Base4D* base = VImpCreator<BaseTaskInfo>::GetImpObject(context)->GetBase();
	contextElem* elem = &(fContexts[base]);
	elem->fContext = RetainRefCountable(context);
	elem->fMustRelease = true;
}


void VDB4DContext::ContainButDoesNotOwn(CDB4DBaseContextPtr context)
{
	Base4D* base = VImpCreator<BaseTaskInfo>::GetImpObject(context)->GetBase();
	contextElem* elem = &(fContexts[base]);
	elem->fContext = context;
	elem->fMustRelease = false;
}


void VDB4DContext::CloseAllConnections()
{
	for (ContextByBaseMap::iterator cur = fContexts.begin(), end = fContexts.end(); cur != end; cur++)
	{
		DB4DNetManager::CloseConnection(cur->second.fContext);
	}
}



/*
void VDB4DContext::SetDataBaseContext(CDB4DBase* inTarget, CDB4DBaseContextPtr inContext)
{
	VTaskLock lock(&fMutex);
	CDB4DBaseContextPtr res = nil;
	sLONG nb = fContexts.GetCount();
	CDB4DBaseContextPtr *p = fContexts.First();
	while (nb>0)
	{
		if ((*p)->GetOwner()->MatchBase(inTarget))
		{
			res = *p;
			break;
		}
		p++;
		--nb;
	}

	if (res == nil)
	{
		res = inContext;
		res->Retain();
		VImpCreator<VDB4DBaseContext>::GetImpObject(res)->GetBaseTaskInfo()->SetContextOwner(this);
		fContexts.Add(res);
	}

}
*/

VUUID& VDB4DContext::GetID()
{
	return fID;
}


const VUUID& VDB4DContext::GetID() const
{
	return fID;
}

void VDB4DContext::SendlastRemoteInfo()
{
	ContextByBaseMap::iterator cur = fContexts.begin(), end = fContexts.end();
	for (; cur != end; cur++)
	{
		cur->second.fContext->SendlastRemoteInfo();
	}
}


void VDB4DContext::SetLanguageContext( VDBLanguageContext *inLanguageContext)
{
	fLanguageContext = inLanguageContext;
}

VDBLanguageContext *VDB4DContext::GetLanguageContext() const
{
	return fLanguageContext;
}


sLONG VDB4DContext::sCurJSFuncNum = 0;


void VDB4DContext::SetJSContext(VJSGlobalContext* inJSContext)
{
	ContextByBaseMap::iterator cur = fContexts.begin(), end = fContexts.end();
	for (; cur != end; cur++)
	{
		BaseTaskInfo* context = VImpCreator<BaseTaskInfo>::GetImpObject(cur->second.fContext);
		context->SetJSContext(inJSContext, false);
	}
	fJSContext = inJSContext;	// sc 24/08/2009 no more retain on JavaScript context
}


VJSGlobalContext* VDB4DContext::GetJSContext() const
{
	return fJSContext;
}


VJSObject* VDB4DContext::GetJSFunction(sLONG& ioFuncNum, const VString& inScript, const VectorOfVString* inParams)
{
	if (inScript.IsEmpty())
		return nil;
	else
	{
		if (ioFuncNum == 0)
		{
			ioFuncNum = VInterlocked::Increment(&sCurJSFuncNum);
		}

		map<sLONG, VJSObject>::iterator found = fJSFuncs.find(ioFuncNum);
		if (found == fJSFuncs.end())
		{
			if (fJSContext != nil)
			{
				VJSContext jscontext(fJSContext);
				VJSObject result = jscontext.MakeFunction(L"$$emfunc"+ToString(ioFuncNum), inParams, inScript, nil, 1, nil);
				fJSFuncs.insert(make_pair(ioFuncNum, result));
				//*pobj = fJSContext->MakeFunction(L"$$emfunc"+ToString(ioFuncNum), inParams, inScript, nil, 1, nil);
				found = fJSFuncs.find(ioFuncNum);
				VJSObject* pobj = &(found->second);
				if ( *pobj != 0 )
					JS4D::ProtectValue(jscontext, *pobj);

				return pobj;
			}
			else
			{
				return nil;
			}
		}
		else
		{
			return &(found->second);
		}
	}
}




//=================================================================================


VDB4DColumnFormula::VDB4DColumnFormula(Table* target):fFormules(target)
{
}


VErrorDB4D VDB4DColumnFormula::Add(CDB4DField* inColumn, DB4D_ColumnFormulae inFormula)
{
	if (inColumn == nil)
		return VE_DB4D_WRONGFIELDREF;
	else
	return fFormules.AddAction(inFormula, VImpCreator<VDB4DField>::GetImpObject(inColumn)->GetField());
}


VErrorDB4D VDB4DColumnFormula::Execute(CDB4DSelection* inSel, CDB4DBaseContext* inContext, VDB4DProgressIndicator* InProgress, CDB4DRecord* inCurrentRecord)
{
	BaseTaskInfoPtr context = nil;
	if (inContext != nil)
	{
		context = ConvertContext(inContext);
	}
	if (context != nil)
	{
		Selection* sel = nil;
		if (inSel != nil)
		{
			sel = VImpCreator<VDB4DSelection>::GetImpObject(inSel)->GetSel();
		}
		FicheInMem* currec = nil;
		if (inCurrentRecord != nil)
			currec = (FicheInMem*)inCurrentRecord->GetFicheInMem();
		return fFormules.Execute(sel, context, InProgress, currec);
	}
	else
	{
		return VE_DB4D_WRONG_CONTEXT;
	}
}


sLONG8 VDB4DColumnFormula::GetResultAsLong8(sLONG inColumnNumber) const
{
	return fFormules.GetResultAsLong8(inColumnNumber);
}


Real VDB4DColumnFormula::GetResultAsReal(sLONG inColumnNumber) const
{
	return fFormules.GetResultAsReal(inColumnNumber);
}


void VDB4DColumnFormula::GetResultAsFloat(sLONG inColumnNumber, VFloat& outResult) const
{
	fFormules.GetResultAsFloat(inColumnNumber, outResult);
}


void VDB4DColumnFormula::GetResultAsDuration(sLONG inColumnNumber, VDuration& outResult) const
{
	fFormules.GetResultAsDuration(inColumnNumber, outResult);
}


VValueSingle* VDB4DColumnFormula::GetResult(sLONG inColumnNumber) const
{
	return fFormules.GetResult(inColumnNumber);
}




//=================================================================================



VDB4DAutoSeqNumber::VDB4DAutoSeqNumber(AutoSeqNumber* seq)
{
	assert(seq != nil);
	fSeq = seq;
	seq->Retain();
}


VDB4DAutoSeqNumber::~VDB4DAutoSeqNumber()
{
	assert(fSeq != nil);
	fSeq->Release();
}


const VUUID& VDB4DAutoSeqNumber::GetID() const
{
	return fSeq->GetID();
}

void VDB4DAutoSeqNumber::SetCurrentValue(sLONG8 currentvalue)
{
	fSeq->SetCurrentValue(currentvalue);
}

sLONG8 VDB4DAutoSeqNumber::GetCurrentValue() const
{
	return fSeq->GetCurrentValue();
}

void VDB4DAutoSeqNumber::SetStartingValue(sLONG8 initialvalue)
{
	fSeq->SetStartingValue(initialvalue);
}


sLONG8 VDB4DAutoSeqNumber::GetStartingValue() const
{
	return fSeq->GetStartingValue();
}


sLONG8 VDB4DAutoSeqNumber::GetNewValue(DB4D_AutoSeqToken& ioToken)
{
	return fSeq->GetNewValue(ioToken);
}


void VDB4DAutoSeqNumber::ValidateValue(DB4D_AutoSeqToken inToken, CDB4DTable* inTable, CDB4DBaseContext* context)
{
	fSeq->ValidateValue(inToken, VImpCreator<VDB4DTable>::GetImpObject(inTable)->GetTable(), ConvertContext(context));
}


void VDB4DAutoSeqNumber::InvalidateValue(DB4D_AutoSeqToken inToken)
{
	fSeq->InvalidateValue(inToken);
}


VErrorDB4D VDB4DAutoSeqNumber::Drop(CDB4DBaseContext* inContext)
{
	return fSeq->GetOwner()->DeleteSeqNum(fSeq->GetID(), inContext);
}

//=================================================================================

void VDB4DJournalData::xinit(sLONG8 globaloperation, uLONG inActionType, sLONG8 inContextID, uLONG8 inTimeStamp)
{
	fActionType = inActionType;
	fContextID = inContextID;
	fTimeStamp = inTimeStamp;
	fGlobalOperation = globaloperation;

	fDataPtr = nil;
	fRecordNumber = -1;
	//fTableIndex = -1;
	fUserID.Clear();
	fDataPosInStream = -1;
	fDataStream = nil;
	fBlobLen = 0;
}


VDB4DJournalData::VDB4DJournalData(sLONG8 globaloperation, uLONG inActionType, sLONG8 inContextID, const VValueBag *inContextExtraData, RecordHeader *inRecHeader, uLONG8 inTimeStamp, sLONG8 inPos, VStream* dataStream, const VUUID& inTableID)
: fExtraData( RetainRefCountable( inContextExtraData))
{
	xinit(globaloperation, inActionType, inContextID, inTimeStamp);
	fRecHeader = *inRecHeader;
	fDataPosInStream = inPos;
	fDataStream = dataStream;
	fTableID = inTableID;	
}


VDB4DJournalData::VDB4DJournalData(sLONG8 globaloperation, uLONG inActionType, sLONG8 inContextID, const VValueBag *inContextExtraData, uLONG8 inTimeStamp)
: fExtraData( RetainRefCountable( inContextExtraData))
{
	xinit(globaloperation, inActionType, inContextID, inTimeStamp);
}


VDB4DJournalData::VDB4DJournalData(sLONG8 globaloperation, uLONG inActionType, sLONG8 inContextID, const VValueBag *inContextExtraData, uLONG8 inTimeStamp, sLONG inRecord, const VUUID& inTableID)
: fExtraData( RetainRefCountable( inContextExtraData))
{
	xinit(globaloperation, inActionType, inContextID, inTimeStamp);
	fRecordNumber = inRecord;
	//fTableIndex = inTable;
	fTableID = inTableID;	
}


VDB4DJournalData::VDB4DJournalData(sLONG8 globaloperation, uLONG inActionType, sLONG8 inContextID, const VValueBag *inContextExtraData, uLONG8 inTimeStamp, sLONG8 inSeqNum, const VUUID& inTableID, bool forSeqNum)
: fExtraData( RetainRefCountable( inContextExtraData))
{
	xinit(globaloperation, inActionType, inContextID, inTimeStamp);
	fLastSeqNum = inSeqNum;
	//fTableIndex = inTable;
	fTableID = inTableID;	
}



VDB4DJournalData::VDB4DJournalData(sLONG8 globaloperation, uLONG inActionType, sLONG8 inContextID, const VValueBag *inContextExtraData, uLONG8 inTimeStamp, const VUUID& inUserID)
: fExtraData( RetainRefCountable( inContextExtraData))
{
	xinit(globaloperation, inActionType, inContextID, inTimeStamp);
	fUserID = inUserID;
}


VDB4DJournalData::VDB4DJournalData(sLONG8 globaloperation, uLONG inActionType, sLONG8 inContextID, const VValueBag *inContextExtraData, uLONG8 inTimeStamp, sLONG inBlob, sLONG inBlobLen, const VUUID& inTableID, sLONG8 inPos, VStream* dataStream)
: fExtraData( RetainRefCountable( inContextExtraData))
{
	xinit(globaloperation, inActionType, inContextID, inTimeStamp);
	fDataPosInStream = inPos;
	fDataStream = dataStream;
	fRecordNumber = inBlob;
	//fTableIndex = inTable;
	fBlobLen = inBlobLen;
	fTableID = inTableID;
}


VDB4DJournalData::~VDB4DJournalData()
{
	if (fDataPtr != nil)
		FreeFastMem(fDataPtr);

	ReleaseRefCountable( &fExtraData);
}

uLONG VDB4DJournalData::GetActionType() const
{
	return fActionType;
}


sLONG8 VDB4DJournalData::GetGlobalOperationNumber() const
{
	return fGlobalOperation;
}


Boolean VDB4DJournalData::GetContextID( sLONG8 &outContextID ) const
{
	Boolean result = false;
	if ( fActionType != DB4D_Log_OpenData && fActionType != DB4D_Log_CloseData && fActionType != DB4D_Log_StartBackup )
	{
		outContextID = fContextID;
		result = true;
	}
	return result;
}

uLONG8 VDB4DJournalData::GetTimeStamp() const
{
	return fTimeStamp;
}

Boolean VDB4DJournalData::GetDataLen( sLONG &outDataLen ) const
{
	Boolean result = false;
	if ( fActionType == DB4D_Log_CreateRecord || fActionType == DB4D_Log_ModifyRecord )
	{
		outDataLen = fRecHeader.GetLen();
		result = true;
	}
	else if (fActionType == DB4D_Log_CreateBlob || fActionType == DB4D_Log_ModifyBlob)
	{
		outDataLen = fBlobLen;
		result = true;
	}
	return result;
}

Boolean VDB4DJournalData::GetRecordNumber( sLONG &outRecordNumber ) const
{
	Boolean result = false;
	if ( fActionType == DB4D_Log_CreateRecord || fActionType == DB4D_Log_ModifyRecord)
	{
		outRecordNumber = fRecHeader.GetPos();
		result = true;
	} else if (fActionType == DB4D_Log_DeleteRecord)
	{
		outRecordNumber = fRecordNumber;
		result = true;
	}
	return result;
}


Boolean VDB4DJournalData::GetBlobNumber( sLONG &outBlobNumber, VString& outPath ) const
{
	Boolean result = false;
	if (fActionType == DB4D_Log_CreateBlob || fActionType == DB4D_Log_ModifyBlob || fActionType == DB4D_Log_DeleteBlob)
	{
		outBlobNumber = fRecordNumber;
		result = true;
	}
	outPath = fPath;
	return result;
}


Boolean VDB4DJournalData::GetSequenceNumber( sLONG8 &outSequenceNumber ) const
{
	Boolean result = false;
	if ( fActionType == DB4D_Log_SaveSeqNum)
	{
		outSequenceNumber = fLastSeqNum;
		result = true;
	} 
	return result;
}

/*

Boolean VDB4DJournalData::GetTableIndex( sLONG &outIndex ) const
{
	Boolean result = false;
	if ( fActionType == DB4D_Log_CreateRecord || fActionType == DB4D_Log_ModifyRecord || fActionType == DB4D_Log_DeleteRecord 
		|| fActionType == DB4D_Log_CreateBlob || fActionType == DB4D_Log_ModifyBlob || fActionType == DB4D_Log_DeleteBlob )
	{
		outIndex = fTableIndex;
		result = true;
	}
	return result;
}

*/


Boolean VDB4DJournalData::GetTableID( VUUID& outID ) const
{
	Boolean result = false;
	if ( fActionType == DB4D_Log_CreateRecord || fActionType == DB4D_Log_ModifyRecord || fActionType == DB4D_Log_DeleteRecord 
		|| fActionType == DB4D_Log_CreateBlob || fActionType == DB4D_Log_ModifyBlob || fActionType == DB4D_Log_DeleteBlob 
		|| fActionType == DB4D_Log_TruncateTable  || fActionType == DB4D_Log_SaveSeqNum )
	{
		outID = fTableID;
		result = true;
	}

	return result;
}


Boolean VDB4DJournalData::GetUserID(VUUID& outID) const
{
	Boolean result = false;
	if (fActionType == DB4D_Log_CreateContextWithUserUUID)
	{
		outID = fUserID;
		result = true;
	}
	return result;
}


const VValueBag* VDB4DJournalData::GetExtraData() const
{
	return fExtraData;
}


Boolean VDB4DJournalData::GetCountFields( sLONG &outCountFields ) const
{
	Boolean result = false;
	if ( fActionType == DB4D_Log_CreateRecord || fActionType == DB4D_Log_ModifyRecord )
	{
		outCountFields = fRecHeader.GetNbFields();
		result = true;
	}
	return result;
}


Boolean VDB4DJournalData::ReadDataFromPos()
{
	if (fDataPtr == nil)
	{
		if (fDataStream == nil)
			return false;
		else
		{
			VError err = VE_OK;
			sLONG dataSize;
			sLONG offset = 0;
			if (fActionType == DB4D_Log_CreateBlob || fActionType == DB4D_Log_ModifyBlob)
			{
				dataSize = fBlobLen/*+4*/;
				offset = 4;

			}
			else
			{
				dataSize = fRecHeader.GetLen() + sizeof(ChampHeader)*(fRecHeader.GetNbFields());
			}

			fDataPtr = GetFastMem(dataSize+2, false, 'log4');
			if (fDataPtr == nil)
				return false;
			else
			{
				sLONG8 curpos = fDataStream->GetPos();
				err = fDataStream->SetPos(fDataPosInStream + offset);
				if (err == VE_OK)
					err = fDataStream->GetData(fDataPtr, dataSize);
				fDataStream->SetPos(curpos);
				if (err == VE_OK)
					return true;
				else
				{
					FreeFastMem(fDataPtr);
					fDataPtr = nil;
					return false;
				}
			}
		}
	}
	else
		return true;
}


VValueSingle* VDB4DJournalData::GetNthFieldValue(sLONG inFieldIndex, sLONG* outType, void* df)
{
	VValueSingle* result = nil;
	sLONG type = -1;
	if ( fActionType == DB4D_Log_CreateRecord || fActionType == DB4D_Log_ModifyRecord )
	{
		if (inFieldIndex > 0 && inFieldIndex <= fRecHeader.GetNbFields())
		{
			if (ReadDataFromPos())
			{
				if (fDataPtr != nil)
				{
					sLONG dataSize = fRecHeader.GetLen();
					ChampHeader* ch = ((ChampHeader*) (((char*)fDataPtr)+dataSize) ) + (inFieldIndex - 1);
					type = ch->typ;
					sLONG valuelen;
					if (inFieldIndex == fRecHeader.GetNbFields())
						valuelen = dataSize - ch->offset;
					else
					{
						ChampHeader* ch2 = ((ChampHeader*) (((char*)fDataPtr)+dataSize) ) + inFieldIndex;
						valuelen = ch2->offset - ch->offset;
					}

					if (valuelen <= 0)
						result = nil;
					else
					{
						if (ch->typ == VK_TEXT || ch->typ == VK_TEXT_UTF8 || ch->typ == VK_BLOB || ch->typ == VK_IMAGE || ch->typ == VK_BLOB_DB4D)
						{
							sLONG* p = (sLONG*)(((char*)fDataPtr)+ch->offset);
							if (valuelen == 4 || df == nil || *p == -2)
							{
								result = nil;
							}
							else
							{
								CreVValue_Code Code = FindCV(ch->typ);
								if (Code != nil)
								{
									result = (*Code)((DataTable*)df, 0, ((char*)fDataPtr)+ch->offset, false, true, nil, creVValue_default);
								}
								
							}
						}
						else
						{
							result = (VValueSingle*)VValue::NewValueFromValueKind(ch->typ);
							if (result != nil)
							{
								result->LoadFromPtr(((char*)fDataPtr)+ch->offset);
							}
						}
					}
				}
			}
		}
	}

	if (outType != nil)
		*outType = type;
	return result;
}



sLONG VDB4DJournalData::GetNthFieldBlobRef(sLONG inFieldIndex, VString& outPath)
{
	sLONG result = -4;
	if ( fActionType == DB4D_Log_CreateRecord || fActionType == DB4D_Log_ModifyRecord )
	{
		if (inFieldIndex > 0 && inFieldIndex <= fRecHeader.GetNbFields())
		{
			if (ReadDataFromPos())
			{
				if (fDataPtr != nil)
				{
					sLONG dataSize = fRecHeader.GetLen();
					ChampHeader* ch = ((ChampHeader*) (((char*)fDataPtr)+dataSize) ) + (inFieldIndex - 1);
					sLONG valuelen;
					if (inFieldIndex == fRecHeader.GetNbFields())
						valuelen = dataSize - ch->offset;
					else
					{
						ChampHeader* ch2 = ((ChampHeader*) (((char*)fDataPtr)+dataSize) ) + inFieldIndex;
						valuelen = ch2->offset - ch->offset;
					}

					if (valuelen <= 0)
						result = -4;
					else
					{
						if (ch->typ == VK_TEXT || ch->typ == VK_BLOB || ch->typ == VK_IMAGE || ch->typ == VK_BLOB_DB4D)
						{
							sLONG* p = (sLONG*)(((char*)fDataPtr)+ch->offset);
							result = *p;
							if (result == -2 /*|| result == -3*/)
							{
								p++;
								sLONG len = *p;
								p++;
								outPath.FromBlock(p, len * sizeof(UniChar), VTC_UTF_16);
							}
						}
						else
						{
							result = -1;
						}
					}
				}
			}
		}
	}
	return result;
}


void* VDB4DJournalData::GetDataPtr()
{
	if (ReadDataFromPos())
		return fDataPtr;
	else
		return nil;
}


Boolean VDB4DJournalData::NeedSwap()
{
	return fRecHeader.NeedSwap();
}


VError VDB4DJournalData::ReadBlob(Blob4D* blob)
{
	VError err = VE_OK;
	if ( fActionType == DB4D_Log_CreateBlob || fActionType == DB4D_Log_ModifyBlob )
	{
		if (fDataStream == nil)
			err = VE_UNIMPLEMENTED;
		else
		{
			sLONG8 curpos = fDataStream->GetPos();
			err = fDataStream->SetPos(fDataPosInStream);
			if (err == VE_OK)
				err = blob->GetFrom(*fDataStream);
			fDataStream->SetPos(curpos);
		}
	}
	else
		err = VE_UNIMPLEMENTED;

	return err;
}


//=================================================================================


VDB4DJournalParser::VDB4DJournalParser()
{
	fJournalParser = NULL;
}

VDB4DJournalParser::~VDB4DJournalParser()
{
	if (fJournalParser != nil)
		fJournalParser->DeInit();
	delete fJournalParser;
}

VError VDB4DJournalParser::Init(VFile* inJournalFile,uLONG8 &outTotalOperationCount, VDB4DProgressIndicator *inProgressIndicator )
{
	VError result;
	if ( !fJournalParser )
	{
		fJournalParser = new DB4DJournalParser(inJournalFile);
		result = fJournalParser->Init(outTotalOperationCount,inProgressIndicator);
	}
	else
		result = VE_UNIMPLEMENTED; // in used, you have to de init first
	
	return result;
}

VError VDB4DJournalParser::DeInit()
{
	VError result = VE_OK;
	if ( fJournalParser )
	{
		result = fJournalParser->DeInit();
		delete fJournalParser;
		fJournalParser = NULL;
	}
	else
		result = VE_UNIMPLEMENTED; // not yet initialised
	return result;
}

VError VDB4DJournalParser::SetCurrentOperation( uLONG8 inOperation, CDB4DJournalData **outJournalData )
{
	VError result = VE_OK;
	if ( fJournalParser )
		result = fJournalParser->SetCurrentOperation( inOperation, outJournalData );
	else
		result = VE_UNIMPLEMENTED; // not yet initialised
	return result; 
}

VError VDB4DJournalParser::NextOperation( uLONG8 &outOperation, CDB4DJournalData **outJournalData )
{
	VError result = VE_OK;
	if ( fJournalParser )
		result = fJournalParser->NextOperation( outOperation, NULL, outJournalData );
	else
		result = VE_UNIMPLEMENTED; // not yet initialised
	return result;
}

VError VDB4DJournalParser::SetEndOfJournal( uLONG8 inOperation )
{
	VError result = VE_OK;
	if ( fJournalParser )
		result = fJournalParser->SetEndOfJournal( inOperation );
	else
		result = VE_UNIMPLEMENTED; // not yet initialised
	return result;
}

uLONG8 VDB4DJournalParser::CountOperations()
{
	if (fJournalParser != nil)
		return fJournalParser->CountOperations();
	else
		return 0;
}

bool VDB4DJournalParser::IsValid( const VUUID &inDataLink )
{
	return fJournalParser->IsValid( inDataLink );
}

//=================================================================================

VDB4DRawDataBase::VDB4DRawDataBase(Base4D_NotOpened* inBase)
{
	fBase = inBase;
}

VDB4DRawDataBase::~VDB4DRawDataBase()
{
	if (fBase != nil)
		delete fBase;
}


VIntlMgr *VDB4DRawDataBase::SwitchIntlMgr()
{
	BaseHeader* header = NULL;	
	Base4D_NotOpened* xstruct = fBase->GetStruct();
	Base4D_NotOpened* xdata = fBase->GetDatas();	
	
	if (fBase->IsOpen())
		header = fBase->GetHbbloc();
	else if (xdata != NULL && xdata->IsOpen())
		header = xdata->GetHbbloc();
	else if (xstruct != NULL && xstruct->IsOpen())
		header = xstruct->GetHbbloc();
	
	VIntlMgr *curIntl = VTask::GetCurrentIntlManager();
	curIntl->Retain();

	if (header != NULL)
	{
		VTask::SetCurrentDialectCode(header->dialect, header->collatorOptions);
	}

	return curIntl;
}


VErrorDB4D VDB4DRawDataBase::CheckAll(IDB4D_DataToolsIntf* inDataToolLog, DB4D_ToolsOptions* inOption)
{
	StErrorContextInstaller errs(false);
	ToolLog mylog(inDataToolLog);


#if debuglr == 112
	if (VDBMgr::GetManager()->GetFlushManager()->GetDirty())
	{
		*((sLONG*)0) = 0xffff; // crash on purpose
	}
#endif

	VError err = VE_OK;

	if (fBase != nil)
	{
		VIntlMgr *curIntl = SwitchIntlMgr();

		Base4D_NotOpened* xstruct = fBase->GetStruct();
		if (testAssert(xstruct != nil))
		{
			if (xstruct->IsOpen())
			{
				mylog.SetCurrentBase(xstruct);
				mylog.InitAddressKeeper(xstruct, inOption != nil && inOption->KeepAddrInfos, inOption == nil || inOption->CheckOverLapWithBittable);
				err = xstruct->CheckAllDataSegs(&mylog);
				if (err == VE_OK)
					err = xstruct->CheckSeqNums(&mylog);
				if (err == VE_OK)
					err = xstruct->CheckTableDefs(&mylog);
				if (err == VE_OK)
					err = xstruct->CheckRelationDefs(&mylog);
				if (err == VE_OK)
					err = xstruct->CheckIndexDefs(&mylog);
				if (err == VE_OK)
					err = xstruct->CheckIndexDefInStructs(&mylog);
				if (err == VE_OK)
					err = xstruct->CheckExtraProperties(&mylog);
				if (err == VE_OK)
					err = xstruct->CheckAllDataTables(&mylog);
				if (err == VE_OK)
					err = xstruct->CheckAllIndexes(&mylog);

				if (err == VE_OK)
				{
					err = xstruct->CheckBitTables(&mylog);
				}
				mylog.ReleaseAddressKeeper();
			}
		}

		mylog.SetCurrentBase(fBase);
		if (fBase->IsOpen())
		{
			mylog.InitAddressKeeper(fBase, inOption != nil && inOption->KeepAddrInfos, inOption == nil || inOption->CheckOverLapWithBittable);
			err = fBase->CheckAllDataSegs(&mylog);
			if (err == VE_OK)
				err = fBase->CheckSeqNums(&mylog);
			if (err == VE_OK)
				err = fBase->CheckTableDefs(&mylog); // should be empty
			if (err == VE_OK)
				err = fBase->CheckRelationDefs(&mylog); // should be empty
			if (err == VE_OK)
				err = fBase->CheckIndexDefs(&mylog);
			if (err == VE_OK)
				err = fBase->CheckIndexDefInStructs(&mylog); // should be empty
			if (err == VE_OK)
				err = fBase->CheckExtraProperties(&mylog);
			if (err == VE_OK)
				err = fBase->CheckAllDataTables(&mylog);
			if (err == VE_OK)
				err = fBase->CheckAllIndexes(&mylog);

			if (err == VE_OK)
			{
				err = fBase->CheckBitTables(&mylog);
			}
			mylog.ReleaseAddressKeeper();

		}
		VTask::SetCurrentIntlManager( curIntl);
		ReleaseRefCountable( &curIntl);
	}

	return err;
}


VErrorDB4D VDB4DRawDataBase::CheckStruct_All(IDB4D_DataToolsIntf* inDataToolLog, DB4D_ToolsOptions* inOption)
{
	StErrorContextInstaller errs(false);
	ToolLog mylog(inDataToolLog);
#if debuglr == 112
	if (VDBMgr::GetManager()->GetFlushManager()->GetDirty())
	{
		*((sLONG*)0) = 0xffff; // crash on purpose
	}
#endif

	VError err = VE_OK;

	if (fBase != nil)
	{
		VIntlMgr *curIntl = SwitchIntlMgr();

		Base4D_NotOpened* xstruct = fBase->GetStruct();
		if (testAssert(xstruct != nil))
		{
			if (xstruct->IsOpen())
			{
				mylog.SetCurrentBase(xstruct);
				mylog.InitAddressKeeper(xstruct, inOption != nil && inOption->KeepAddrInfos, inOption == nil || inOption->CheckOverLapWithBittable);
				err = xstruct->CheckAllDataSegs(&mylog);
				if (err == VE_OK)
					err = xstruct->CheckSeqNums(&mylog);
				if (err == VE_OK)
					err = xstruct->CheckTableDefs(&mylog);
				if (err == VE_OK)
					err = xstruct->CheckRelationDefs(&mylog);
				if (err == VE_OK)
					err = xstruct->CheckIndexDefs(&mylog);
				if (err == VE_OK)
					err = xstruct->CheckIndexDefInStructs(&mylog);
				if (err == VE_OK)
					err = xstruct->CheckExtraProperties(&mylog);
				if (err == VE_OK)
					err = xstruct->CheckAllDataTables(&mylog);
				if (err == VE_OK)
					err = xstruct->CheckAllIndexes(&mylog);
				if (err == VE_OK)
				{
					err = xstruct->CheckBitTables(&mylog);
				}
				mylog.ReleaseAddressKeeper();
			}
		}
		VTask::SetCurrentIntlManager( curIntl);
		ReleaseRefCountable( &curIntl);
	}
	return err;
}


VErrorDB4D VDB4DRawDataBase::CheckData_All(IDB4D_DataToolsIntf* inDataToolLog, DB4D_ToolsOptions* inOption)
{
	StErrorContextInstaller errs(false);
	ToolLog mylog(inDataToolLog);
#if debuglr == 112
	if (VDBMgr::GetManager()->GetFlushManager()->GetDirty())
	{
		*((sLONG*)0) = 0xffff; // crash on purpose
	}
#endif

	VError err = VE_OK;

	if (fBase != nil)
	{
		VIntlMgr *curIntl = SwitchIntlMgr();

		if (fBase->IsOpen())
		{
			mylog.SetCurrentBase(fBase);
			mylog.InitAddressKeeper(fBase, inOption != nil && inOption->KeepAddrInfos, inOption == nil || inOption->CheckOverLapWithBittable);
			err = fBase->CheckAllDataSegs(&mylog);
			if (err == VE_OK)
				err = fBase->CheckSeqNums(&mylog);
			if (err == VE_OK)
				err = fBase->CheckTableDefs(&mylog); // should be empty
			if (err == VE_OK)
				err = fBase->CheckRelationDefs(&mylog); // should be empty
			if (err == VE_OK)
				err = fBase->CheckIndexDefs(&mylog);
			if (err == VE_OK)
				err = fBase->CheckIndexDefInStructs(&mylog); // should be empty
			if (err == VE_OK)
				err = fBase->CheckExtraProperties(&mylog);
			if (err == VE_OK)
				err = fBase->CheckAllDataTables(&mylog);
			if (err == VE_OK)
				err = fBase->CheckAllIndexes(&mylog);
			if (err == VE_OK)
			{
				err = fBase->CheckBitTables(&mylog);
			}
			mylog.ReleaseAddressKeeper();
		}
		VTask::SetCurrentIntlManager( curIntl);
		ReleaseRefCountable( &curIntl);
	}
	return err;
}


bool VDB4DRawDataBase::CheckStructAndDataUUIDMatch(IDB4D_DataToolsIntf* inDataToolLog)
{
	StErrorContextInstaller errs(false);
	ToolLog mylog(inDataToolLog);
	bool match = true;
	VError err = VE_OK;

	// both data and struct are opened, we can check their UUID match
	if ((fBase != nil) && fBase->IsOpen())
	{
		Base4D_NotOpened* xstruct = fBase->GetStruct();

		mylog.SetCurrentBase(xstruct);

		if ((xstruct != nil) && xstruct->IsOpen() && (fBase->GetUUID() != xstruct->GetUUID()))
		{
			DataBaseProblem pb(TOP_Datafile_Does_Not_Match_Struct);
			err = mylog.Add(pb);
			match = false;
		}
	}

	return match;
}

VErrorDB4D VDB4DRawDataBase::CheckStruct_DataSegs(IDB4D_DataToolsIntf* inDataToolLog)
{
	StErrorContextInstaller errs(false);
	ToolLog mylog(inDataToolLog);

	VError err = VE_OK;

	if (fBase != nil)
	{
		Base4D_NotOpened* xstruct = fBase->GetStruct();
		if (testAssert(xstruct != nil) && xstruct->IsOpen())
		{
			mylog.SetCurrentBase(xstruct);
			err = xstruct->CheckAllDataSegs(&mylog);
		}
	}
	return err;
}


VErrorDB4D VDB4DRawDataBase::CheckStruct_Elems(IDB4D_DataToolsIntf* inDataToolLog)
{
	StErrorContextInstaller errs(false);
	ToolLog mylog(inDataToolLog);

	VError err = VE_OK;

	if (fBase != nil)
	{
		Base4D_NotOpened* xstruct = fBase->GetStruct();
		if (testAssert(xstruct != nil) && xstruct->IsOpen())
		{
			mylog.SetCurrentBase(xstruct);

			if ((fBase != nil) && fBase->IsOpen())
			{
				// both data and struct are opened, we can check their UUID match
				if (fBase->GetUUID() != xstruct->GetUUID())
				{
					DataBaseProblem pb(TOP_Datafile_Does_Not_Match_Struct);
					err = mylog.Add(pb);
					//err = VE_DB4D_DATAFILE_DOES_NOT_MATCH_STRUCT;
				}
			}

			if (err == VE_OK)
				err = xstruct->CheckSeqNums(&mylog);
			if (err == VE_OK)
				err = xstruct->CheckTableDefs(&mylog);
			if (err == VE_OK)
				err = xstruct->CheckRelationDefs(&mylog);
			if (err == VE_OK)
				err = xstruct->CheckIndexDefs(&mylog);
			if (err == VE_OK)
				err = xstruct->CheckIndexDefInStructs(&mylog);
			if (err == VE_OK)
				err = xstruct->CheckExtraProperties(&mylog);
		}
	}

	return err;
}


VErrorDB4D VDB4DRawDataBase::CheckStruct_RecordsAndIndexes(IDB4D_DataToolsIntf* inDataToolLog)
{
	StErrorContextInstaller errs(false);
	ToolLog mylog(inDataToolLog);

	VError err = VE_OK;

	if (fBase != nil)
	{
		VIntlMgr *curIntl = SwitchIntlMgr();
		
		Base4D_NotOpened* xstruct = fBase->GetStruct();
		if (testAssert(xstruct != nil) && xstruct->IsOpen())
		{
			mylog.SetCurrentBase(xstruct);
			err = xstruct->CheckAllDataTables(&mylog);
			if (err == VE_OK)
				err = xstruct->CheckAllIndexes(&mylog);
		}
		VTask::SetCurrentIntlManager( curIntl);
		ReleaseRefCountable( &curIntl);
	}
	return err;
}


VErrorDB4D VDB4DRawDataBase::CheckData_DataSegs(IDB4D_DataToolsIntf* inDataToolLog)
{
	StErrorContextInstaller errs(false);
	ToolLog mylog(inDataToolLog);

	VError err = VE_OK;

	if (fBase != nil && fBase->IsOpen())
	{
		mylog.SetCurrentBase(fBase);
		err = fBase->CheckAllDataSegs(&mylog);
	}
	return err;
}


VErrorDB4D VDB4DRawDataBase::CheckData_Elems(IDB4D_DataToolsIntf* inDataToolLog)
{
	StErrorContextInstaller errs(false);
	ToolLog mylog(inDataToolLog);

	VError err = VE_OK;

	if (fBase != nil && fBase->IsOpen())
	{
		mylog.SetCurrentBase(fBase);
		err = fBase->CheckSeqNums(&mylog);
		if (err == VE_OK)
			err = fBase->CheckTableDefs(&mylog); // should be empty
		if (err == VE_OK)
			err = fBase->CheckRelationDefs(&mylog); // should be empty
		if (err == VE_OK)
			err = fBase->CheckIndexDefs(&mylog);
		if (err == VE_OK)
			err = fBase->CheckIndexDefInStructs(&mylog); // should be empty
		if (err == VE_OK)
			err = fBase->CheckExtraProperties(&mylog);
	}
	return err;
}


VErrorDB4D VDB4DRawDataBase::CheckData_OneTable(sLONG inTableNum, IDB4D_DataToolsIntf* inDataToolLog)
{
	StErrorContextInstaller errs(false);
	ToolLog mylog(inDataToolLog);

	VError err = VE_OK;

	if (fBase != nil && fBase->IsOpen())
	{
		VIntlMgr *curIntl = SwitchIntlMgr();

		mylog.SetCurrentBase(fBase);
		err = fBase->CheckOneTable(inTableNum, &mylog);

		VTask::SetCurrentIntlManager( curIntl);
		ReleaseRefCountable( &curIntl);
	}
	return err;
}


VErrorDB4D VDB4DRawDataBase::CheckData_OneIndex(sLONG inIndexNum, IDB4D_DataToolsIntf* inDataToolLog)
{
	StErrorContextInstaller errs(false);
	ToolLog mylog(inDataToolLog);

	VError err = VE_OK;

	if (fBase != nil && fBase->IsOpen())
	{
		VIntlMgr *curIntl = SwitchIntlMgr();

		mylog.SetCurrentBase(fBase);
		err = fBase->CheckOneIndex(inIndexNum, &mylog);

		VTask::SetCurrentIntlManager( curIntl);
		ReleaseRefCountable( &curIntl);
	}
	return err;
}


sLONG VDB4DRawDataBase::CountTables(Boolean& outInfoIsValid, IDB4D_DataToolsIntf* inDataToolLog)
{
	ToolLog mylog(inDataToolLog);

	outInfoIsValid = false;
	if (fBase != nil && fBase->GetStruct() != nil && fBase->GetStruct()->IsOpen())
	{
		mylog.SetCurrentBase(fBase);
		return fBase->CountTables(outInfoIsValid,&mylog);
	}
	else
		return 0;
}

sLONG VDB4DRawDataBase::CountIndexes(Boolean& outInfoIsValid, IDB4D_DataToolsIntf* inDataToolLog)
{
	ToolLog mylog(inDataToolLog);

	outInfoIsValid = false;
	if (fBase != nil && fBase->GetStruct() != nil && fBase->GetStruct()->IsOpen())
	{
		mylog.SetCurrentBase(fBase);
		return fBase->CountIndexes(outInfoIsValid, &mylog);
	}
	else
		return 0;
}

void VDB4DRawDataBase::GetTableName(sLONG inTableNum, VString& outName, Boolean& outInfoIsValid, IDB4D_DataToolsIntf* inDataToolLog)
{
	ToolLog mylog(inDataToolLog);

	outInfoIsValid = false;
	if (fBase != nil && fBase->GetStruct() != nil && fBase->GetStruct()->IsOpen())
	{
		mylog.SetCurrentBase(fBase);
		return fBase->GetTableName(inTableNum, outName, outInfoIsValid, &mylog);
	}
}

sLONG VDB4DRawDataBase::GetTables(std::vector<sLONG>& outTables,Boolean& outInfoIsValid,IDB4D_DataToolsIntf* inDataToolLog)
{
	ToolLog mylog(inDataToolLog);

	outInfoIsValid = false;
	if (fBase != nil && fBase->GetStruct() != nil && fBase->GetStruct()->IsOpen())
	{
		mylog.SetCurrentBase(fBase);
		return fBase->GetTables(outTables, outInfoIsValid, &mylog);
	}
	else
		return VE_OK;
}

VErrorDB4D VDB4DRawDataBase::GetTableFields(sLONG inTableNum, std::vector<sLONG>& outFields, Boolean& outInfoIsValid, IDB4D_DataToolsIntf* inDataToolLog)
{
	ToolLog mylog(inDataToolLog);

	outInfoIsValid = false;
	if (fBase != nil && fBase->GetStruct() != nil && fBase->GetStruct()->IsOpen())
	{
		mylog.SetCurrentBase(fBase);
		return fBase->GetTableFields(inTableNum, outFields, outInfoIsValid, &mylog);
	}
	else
		return VE_OK;
}

VErrorDB4D VDB4DRawDataBase::GetTableIndexes(sLONG inTableNum, std::vector<sLONG>& outIndexes, Boolean& outInfoIsValid, IDB4D_DataToolsIntf* inDataToolLog)
{
	ToolLog mylog(inDataToolLog);

	outInfoIsValid = false;
	if (fBase != nil && fBase->GetStruct() != nil && fBase->GetStruct()->IsOpen())
	{
		mylog.SetCurrentBase(fBase);
		return fBase->GetTableIndexes(inTableNum, outIndexes, outInfoIsValid, &mylog);
	}
	else
		return VE_OK;
}


RecIDType VDB4DRawDataBase::CountRecordsInTable(sLONG inTableNum, Boolean& outInfoIsValid, IDB4D_DataToolsIntf* inDataToolLog)
{
	ToolLog mylog(inDataToolLog);

	outInfoIsValid = false;
	if (fBase != nil && fBase->IsOpen())
	{
		mylog.SetCurrentBase(fBase);
		return fBase->CountRecordsInTable(inTableNum, outInfoIsValid, &mylog);
	}
	else
		return 0;
}

sLONG VDB4DRawDataBase::CountFieldsInTable(sLONG inTableNum, Boolean& outInfoIsValid, IDB4D_DataToolsIntf* inDataToolLog)
{
	ToolLog mylog(inDataToolLog);

	outInfoIsValid = false;
	if (fBase != nil && fBase->GetStruct() != nil && fBase->GetStruct()->IsOpen())
	{
		mylog.SetCurrentBase(fBase);
		return fBase->CountFieldsInTable(inTableNum, outInfoIsValid, &mylog);
	}
	else
		return 0;
}

VErrorDB4D VDB4DRawDataBase::GetIndexInfo(sLONG inIndexNum,XBOX::VString& outName,sLONG& outTableNum,std::vector<sLONG>& outFields, Boolean& outInfoIsValid, IDB4D_DataToolsIntf* inDataToolLog)
{
	ToolLog mylog(inDataToolLog);

	outInfoIsValid = false;
	
	if (fBase != nil && fBase->GetStruct() != nil && fBase->GetStruct()->IsOpen())
	{
		mylog.SetCurrentBase(fBase);
		return fBase->GetIndexInfo(inIndexNum, outName, outTableNum, outFields,outInfoIsValid, &mylog);
	}
	else
		return VE_OK;
}

VErrorDB4D VDB4DRawDataBase::GetFieldInfo(sLONG inTableNum, sLONG inFieldNum, VString& outName, sLONG& outType, std::vector<sLONG>& outIndexes, Boolean& outInfoIsValid, IDB4D_DataToolsIntf* inDataToolLog)
{
	ToolLog mylog(inDataToolLog);

	outInfoIsValid = false;
	
	if (fBase != nil && fBase->GetStruct() != nil && fBase->GetStruct()->IsOpen())
	{
		mylog.SetCurrentBase(fBase);
		return fBase->GetFieldInfo(inTableNum, inFieldNum, outName, outType, outIndexes, outInfoIsValid, &mylog);
	}
	else
		return VE_OK;
}



VErrorDB4D VDB4DRawDataBase::CompactInto(CDB4DBase* outCompactedBase, IDB4D_DataToolsIntf* inDataToolLog, Boolean withIndexes, 
										 Boolean WithStruct, Boolean WithData, Boolean EraseOrphanTables)
{
	StErrorContextInstaller errs(false);
	ToolLog mylog(inDataToolLog);
	Base4D* target = nil;

	if (outCompactedBase != nil)
	{
		target = VImpCreator<VDB4DBase>::GetImpObject(outCompactedBase)->GetBase();
	}

	VError err = VE_OK;

	if (fBase != nil && target != nil)
	{
		VIntlMgr *curIntl = SwitchIntlMgr();

		mylog.SetBaseToBeCompacted(target);
		mylog.SkipOrphanTables(EraseOrphanTables);
		if (WithStruct)
		{
			Base4D_NotOpened* xstruct = fBase->GetStruct();
			if (testAssert(xstruct != nil))
			{
				mylog.SetCurrentBase(xstruct);
				// err = xstruct->CheckAllDataSegs(&mylog); // pas necessaire pour compactage
				if (err == VE_OK)
					err = xstruct->CheckSeqNums(&mylog);
				/*
				if (err == VE_OK)
					err = xstruct->CheckTableDefs(&mylog);  // la defintion de la table 'Resources' est creee a l'ouverture de la base et n'est pas stockee
				*/
				/*
				if (err == VE_OK)
					err = xstruct->CheckRelationDefs(&mylog); // pas de relations sur la table 'Resources' dans la structure
				*/
				/*
				if (err == VE_OK)
					err = xstruct->CheckIndexDefInStructs(&mylog);  // les definitions des index sur la table 'Resources' sont creees a louverture de la base et ne sont pas stockees
				*/
				if (err == VE_OK)
					err = xstruct->CheckExtraProperties(&mylog);

				if (err == VE_OK)
					err = xstruct->CheckAllDataTables(&mylog);
				if (withIndexes)
				{
					if (err == VE_OK)
						err = xstruct->CheckIndexDefs(&mylog);
				}
			}
		}

		if (WithData && fBase->IsOpen())
		{
			Base4D_NotOpened* xstruct = fBase->GetStruct();
			mylog.SetCurrentBase(fBase);

			// err = fBase->CheckAllDataSegs(&mylog); // pas necessaire pour compactage
			if (err == VE_OK)
				err = fBase->CheckSeqNums(&mylog);
			if (err == VE_OK && xstruct->IsOpen())
				err = xstruct->CheckTableDefs(&mylog); 
			if (err == VE_OK)
				err = fBase->CheckTableDefs(&mylog); 
			if (err == VE_OK && xstruct->IsOpen())
				err = xstruct->CheckRelationDefs(&mylog);
			if (err == VE_OK && xstruct->IsOpen())
				err = xstruct->CheckIndexDefInStructs(&mylog);
			if (err == VE_OK)
				err = fBase->CheckExtraProperties(&mylog);
			if (err == VE_OK)
				err = fBase->CheckAllDataTables(&mylog);
			if (withIndexes && err == VE_OK)
			{
				err = target->LoadAllTableDefs();
				if (err == VE_OK)
					err = fBase->CheckIndexDefs(&mylog);
			}
		}
		else
		{
			Base4D_NotOpened* xstruct = fBase->GetStruct();
			mylog.SetCurrentBase(fBase);
			if (err == VE_OK)
				err = xstruct->CheckTableDefs(&mylog); 
			if (err == VE_OK)
				err = xstruct->CheckRelationDefs(&mylog);
			if (err == VE_OK)
				err = xstruct->CheckIndexDefInStructs(&mylog);
		}

		if (err == VE_OK)
		{
			if (fBase->IsOpen())
			{
				err = target->CompletelyChangeUUID(fBase->GetUUID());
				target->GetHbbloc()->fStamp = fBase->GetHbbloc()->fStamp;
				target->GetHbbloc()->lastaction = fBase->GetHbbloc()->lastaction;
				target->GetHbbloc()->countlog = fBase->GetHbbloc()->countlog;
				target->GetHbbloc()->dialect = fBase->GetHbbloc()->dialect;
				target->GetHbbloc()->collatorOptions = fBase->GetHbbloc()->collatorOptions;
				target->setmodif(true, target, nil);
			}
			else
			{
				err = target->CompletelyChangeUUID(fBase->GetStruct()->GetUUID());
			}
		}
		VTask::SetCurrentIntlManager( curIntl);
		ReleaseRefCountable( &curIntl);
	}

	return err;
}

#if 0

VErrorDB4D VDB4DRawDataBase::RecoverByTags(CDB4DBase* outRecoveredBase, IDB4D_DataToolsIntf* inDataToolLog, Boolean WithStruct, Boolean WithData)
{
	VError err = VE_DB4D_NOTIMPLEMENTED;
	Base4D* target = nil;
	ToolLog mylog(inDataToolLog);

	if (outRecoveredBase != nil)
	{
		target = VImpCreator<VDB4DBase>::GetImpObject(outRecoveredBase)->GetBase();
	}

	if (fBase != nil && target != nil)
	{
		VIntlMgr *curIntl = SwitchIntlMgr();
		if (WithStruct)
		{
			Base4D_NotOpened* xstruct = fBase->GetStruct();
			if (xstruct != nil)
			{
				Base4D* newstruct = target->GetStructure();
				if (newstruct != nil)
					err = xstruct->RecoverByTags(*newstruct, &mylog);
			}

		}

		if (WithData && err == VE_OK)
		{
			err = fBase->RecoverByTags(*target, &mylog);
			
			if (WithData && (err == VE_OK))
			{
				VUUID newJournalLink(true);
				err = target->SetJournalFileInfos( NULL, &newJournalLink, false );
			}
		}

		VTask::SetCurrentIntlManager( curIntl);
		ReleaseRefCountable( &curIntl);
	}
	return err;
}

#endif



//=================================================================================




void VDB4DQueryOptions::SetFilter( CDB4DSelection* inFilter)
{
	fOptions.SetFilter(inFilter == nil ? nil : VImpCreator<VDB4DSelection>::GetImpObject(inFilter)->GetSel());
	if (inFilter != nil)
	{
		fOptions.SetFilterTable(VImpCreator<VDB4DSelection>::GetImpObject(inFilter)->GetTable());
	}
}


void VDB4DQueryOptions::SetLimit(sLONG8 inNewLimit)
{
	fOptions.SetLimit(inNewLimit);
}


void VDB4DQueryOptions::SetWayOfLocking(DB4D_Way_of_Locking HowToLock)
{
	fOptions.SetWayOfLocking(HowToLock);
}


void VDB4DQueryOptions::SetWayOfLockingForFirstRecord(DB4D_Way_of_Locking HowToLock)
{
	fOptions.SetRecordWayOfLocking(HowToLock);
}


void VDB4DQueryOptions::SetWantsLockedSet(Boolean WantsLockedSet)
{
	fOptions.SetWantsLockedSet(WantsLockedSet);
}


void VDB4DQueryOptions::SetWantsFirstRecord(Boolean WantsFirstRecord)
{
	fOptions.SetWantsFirstRecord(WantsFirstRecord);
}


void VDB4DQueryOptions::SetDestination(DB4D_QueryDestination SetDestination)
{
	fOptions.SetDestination(SetDestination);
}


void VDB4DQueryOptions::DescribeQueryExecution(Boolean on)
{
	fOptions.SetDescribeQuery(on);
}






//=================================================================================



CDB4DSelection* VDB4DQueryResult::GetSelection()
{
	if (fSelection == nil)
	{
		Selection* sel = fResult.GetSelection();
		if (sel != nil)
		{
			fSelection = new VDB4DSelection(VDBMgr::GetManager(), VImpCreator<VDB4DBase>::GetImpObject(fTable->GetOwner()), VImpCreator<VDB4DTable>::GetImpObject(fTable)->GetTable(), sel);
			if (fSelection != nil)
				sel->Retain();
			else
				ThrowBaseError(memfull, noaction);
		}
	}
	return fSelection;
}


CDB4DSet* VDB4DQueryResult::GetSet()
{
	if (fSet == nil)
	{
		Bittab* set = fResult.GetSet();
		if (set != nil)
		{
			fSet = new VDB4DSet(VImpCreator<VDB4DBase>::GetImpObject(fTable->GetOwner()), VImpCreator<VDB4DTable>::GetImpObject(fTable)->GetTable(), set);
			if (fSet == nil)
				ThrowBaseError(memfull, noaction);
		}
	}
	return fSet;
}


sLONG8 VDB4DQueryResult::GetCount()
{
	/*
		VDB4DQueryResult can be created with empty fResult but with non-nil fSelection or fSet
	*/
	sLONG8 count = fResult.GetCount();
	if (count >= 0)
		return count;

	if (fSelection != nil)
		return fSelection->CountRecordsInSelection( nil);

	if (fSet != nil)
		return fSet->CountRecordsInSet();
	
	return 0;
}


CDB4DSet* VDB4DQueryResult::GetLockedSet()
{
	if (fLockedSet == nil)
	{
		Bittab* set = fResult.GetLockedSet();
		if (set != nil)
		{
			fLockedSet = new VDB4DSet(VImpCreator<VDB4DBase>::GetImpObject(fTable->GetOwner()), VImpCreator<VDB4DTable>::GetImpObject(fTable)->GetTable(), set);
			if (fLockedSet == nil)
				ThrowBaseError(memfull, noaction);
		}
	}
	return fLockedSet;
}


CDB4DRecord* VDB4DQueryResult::GetFirstRecord()
{
	if (fFirstRec == nil)
	{
		FicheInMem* rec = fResult.GetFirstRecord();
		if (rec != nil)
		{
			fFirstRec = new VDB4DRecord(VDBMgr::GetManager(), rec, rec->GetContext()->GetEncapsuleur());
			if (fFirstRec != nil)
				rec->Retain();
			else
				ThrowBaseError(memfull, noaction);
		}
	}
	return fFirstRec;
}


void VDB4DQueryResult::GetQueryDescription(VString& outResult, DB4D_QueryDescriptionFormat inFormat)
{
	outResult = fResult.GetQueryDescription();
}


void VDB4DQueryResult::GetQueryExecution(VString& outResult, DB4D_QueryDescriptionFormat inFormat)
{
	if (inFormat == DB4D_QueryDescription_XML)
	{
		outResult = fResult.GetQueryExecutionXML();
	}
	else
	{
		outResult = fResult.GetQueryExecution();
	}
}


