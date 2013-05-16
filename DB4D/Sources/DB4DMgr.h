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
#ifndef __DB4DMGR__
#define __DB4DMGR__

class VDBFlushMgr;
class VDBCacheMgr;
class ServerManager;
class Bittab;
class ClientProgressTask;
class ServerProgressTask;
class VDB4DSignaler;
class VDB4DContext;
class DataSet;

class base4dinfo
{
public:
	Base4D* base;
	Boolean isClosing;

	Boolean operator == (const base4dinfo& other) const { return base == other.base; };
};


class KeptFicheInMem
{
public:
	/*
	inline KeptFicheInMem()
	{
		fCount = 0;
	}
	*/

	FicheInMem* fRec;
	sLONG fCount;
};


class KeptSelection
{
public:
	inline KeptSelection(Selection* inSel, uBOOL inRetainedByClient, uBOOL inRetainedByServer)
	{
		fRetainedByClient = inRetainedByClient;
		fRetainedByServer = inRetainedByServer;
		fSel = inSel;
		fCount = 1;
	}

	inline KeptSelection()
	{
		fRetainedByClient = false;
		fRetainedByServer = false;
		fSel = nil;
		fCount = 1;
	}

	Selection* fSel;
	uWORD fCount;
	uBOOL fRetainedByClient, fRetainedByServer;
};


typedef	map<sLONG, DB4DArrayOfValuesCreator> mapAVCreator;
typedef	map<sLONG, DB4DCollectionManagerCreator> mapCollectionCreator;
typedef map<VUUIDBuffer, KeptSelection> mapOfSelections;
typedef map<VUUIDBuffer, KeptFicheInMem> mapOfRecords;

typedef map<VUUIDBuffer, CDB4DContext*> mapOfContext;
typedef map<VString, CDB4DContext*> mapOfContextBySessionID;

typedef map<VUUIDBuffer, VRefPtr<VProgressIndicator> > mapOfProgress;

typedef map<VUUIDBuffer, DataSet*> mapOfDataSet;

class VStringUTF8;


class PushedRecID
{
	public:
		inline PushedRecID(bool markforpush, sLONG numtable, sLONG numrec)
		{
			fNumTable = numtable;
			fNumRec = numrec;
			if (markforpush)
				fMarkForPush = 1;
			else
				fMarkForPush = 0;
		}

		sLONG fNumTable;
		sLONG fNumRec;
		uBYTE fMarkForPush;
};

typedef vector<PushedRecID> vectorPushedRecs;
typedef set<VString> DataPathCollection;


class DataSetPurger : public ObjInCacheMem, public IObjToFree
{
	public:
		virtual bool FreeMem(sLONG allocationBlockNumber, VSize combien, VSize& outSizeFreed);

};

class VGarbageTask;

class ENTITY_API VDBMgr : public VObject, public IRefCountable
{
public:
#if DB4DasDLL
	static VDBMgr *RetainManager(VLocalizationManager* inLocalizationManager); // retain it, initialize it if not allready inited
#else
	static VDBMgr *RetainManager(VComponentLibrary* DB4DCompLib); // retain it, initialize it if not allready inited
#endif
	static VDBMgr *GetManager(); // no retain, no init
	
	//static CLanguage *GetLanguage();
	//static CDB4D_Lang *GetLang4D();

	static VDBFlushMgr *GetFlushManager();
	static VDBCacheMgr *GetCacheManager();

	static inline CUAGManager* GetUAGManager()
	{
		return GetManager()->fUAGManager;
	}

	static Boolean NeedsBytesInCache( sLONG inNeededBytes);
	
	static void PutObjectInCache( ObjCache *inObject, Boolean inCheckAtomic = true);
	static void RemoveObjectFromCache( ObjCache *obj);
	
	static void PutObjectInFlush( ObjAlmostInCache *obj, Boolean inSetCacheDirty = false, Boolean inForDelete = false, Boolean NoWait = false);
	static void RemoveObjectFromFlush( ObjAlmostInCache *obj);
	

	static void PutObjectInCache( IObjToFree *inObject, Boolean inCheckAtomic = true);
	static void RemoveObjectFromCache( IObjToFree *inObject);

	static void PutObjectInFlush( IObjToFlush *obj, Boolean inSetCacheDirty = false, Boolean inForDelete = false, Boolean NoWait = false);
	static void RemoveObjectFromFlush( IObjToFlush *obj);

	static IBackupSettings* CreateBackupSettings();
	static IBackupTool*		CreateBackupTool();
	static IJournalTool*	CreateJournalTool();

	static XBOX::VError GetJournalInfo(const XBOX::VFilePath& inDataFilePath,XBOX::VFilePath& journalPath,XBOX::VUUID& journalDataLink);

	CDB4DBase* OpenBase( const VFile& inStructureFile, sLONG inParameters, VError* outErr = nil, FileAccess inAccess = FA_READ_WRITE, VString* EntityFileExt = nil, 
							CUAGDirectory* inUAGDirectory = nil, const VString* inHtmlContent = nil, unordered_map_VString<VRefPtr<VFile> >* outIncludedFiles = nil, const VFile* inPermissions = nil);

	CDB4DBase* CreateBase( const VFile& inStructureFile, sLONG inParameters, VIntlMgr* inIntlMgr, VError* outErr = nil, FileAccess inAccess = FA_READ_WRITE, 
							const VUUID* inChosenID = nil, VString* EntityFileExt = nil, CUAGDirectory* inUAGDirectory = nil, const VFile* inPermissions = nil);

	CDB4DBase* RetainNthBase( sLONG inBaseIndex);
	CDB4DBase* RetainBaseByUUID(const VUUID& inBaseID);
	Base4D* RetainBase4DByUUID(const VUUID& inBaseID);
	Base4D* RetainBase4DByName(const VString& inName);

	const VValueBag* RetainExtraProperties( VFile* inFile, bool *outWasOpened, VUUID *outUUID, VError &err, CDB4DBaseContextPtr inContext, uLONG8* outVersionNumber = nil);
	
	VError RegisterBase(Base4D* base);
	void UnRegisterBase(Base4D* base); // means CloseDatabase
	void FinishUnRegisterBase(Base4D* base);

	sLONG CountBases() const;

	void FlushAllData();

	void FlushCache(Base4D* target, Boolean WaitUntilDone);
	void FlushCache(Boolean WaitUntilDone, Boolean onlyForAllWritten = false);
	void SetFlushPeriod( sLONG inMillisecondsPeriod);
	VError SetCacheSize( VSize inSize, bool inPhysicalMemOnly, VSize *outActualCacheSize);
	VError SetMinSizeToFlush( VSize inMinSizeToFlush);
	VSize GetCacheSize();
	VSize GetMinSizeToFlush();
	
	DB4DActivityManager* SetActivityManager(DB4DActivityManager* inNewActivityManager);
	static inline DB4DActivityManager* GetActivityManager() { return sDB4DMgr->fActivityManager; };

	DB4DNetworkManager* SetNetworkManager(DB4DNetworkManager* inNewNetworkManager);
	static inline DB4DNetworkManager* GetNetworkManager() { return sDB4DMgr->fNetworkManager; };

	DB4DCommandManager* SetCommandManager(DB4DCommandManager* inNewCommandManager);
	static inline DB4DCommandManager* GetCommandManager() { return sDB4DMgr->fCommandManager; };

	BuildLanguageExpressionMethod SetMethodToBuildLanguageExpressions(BuildLanguageExpressionMethod inNewMethodToBuildLanguageExpressions);
	static inline BuildLanguageExpressionMethod GetBuildLanguageExpressionMethod() { return sDB4DMgr->fCurrentBuildLanguageExpressionMethod; };

	static inline VDB4DSignaler* GetSignaler() { return sDB4DMgr->fSignaler; };

#if 0
	void GetErrorString(sLONG n, VString& outMess) const ;
	void GetErrorActionString(sLONG n, VString& outMess) const;
	void InitErrors();
	void AddErrorString(sLONG n, VString& s);
	void AddActionString(sLONG n, VString& s);
#endif

	/*
	static UniChar GetWildChar();
	static void SetWildChar(UniChar c);
	static CompareResult CompareString(const UniChar* inText1, sLONG inSize1, const UniChar* inText2, sLONG inSize2, Boolean inWithDiacritics);
	static sLONG FindStringInPtr(void* HostString, const VString* StrToFind, Boolean inDiacritical);
	*/

	/*
	static	bool	ContainsKeyword( const VString& inString, const VString& inKeyword, bool inDiacritical, VIntlMgr *inIntlMgr, VError& outErr);
	static	bool	ContainsKeyword_Like( const VString& inString, const VString& inKeyword, bool inDiacritical, VIntlMgr *inIntlMgr, VError& outErr);
	*/

	static	bool	ContainsKeyword( const VString& inString, const VString& inKeyword, const VCompareOptions& inOptions, VError& outErr);

	inline VString* GetUtil() { return &fUtil; };

	static inline VString* GetStringRef() { return &(sDB4DMgr->fUtil); };
	static inline VStringUTF8* GetStringUTF8Ref() { return &(sDB4DMgr->fUtilUTF8); };

	static Boolean IsItaDeadChar(UniChar c, const VIntlMgr* inIntlMgr);

	VError Register_DB4DArrayOfValuesCreator(sLONG signature, DB4DArrayOfValuesCreator Creator);
	VError UnRegister_DB4DArrayOfValuesCreator(sLONG signature);
	DB4DArrayOfValuesCreator GetDB4DArrayOfValuesCreator(sLONG signature) const;

	VError Register_DB4DCollectionManagerCreator(sLONG signature, DB4DCollectionManagerCreator Creator);
	VError UnRegister_DB4DCollectionManagerCreator(sLONG signature);
	DB4DCollectionManagerCreator GetDB4DCollectionManagerCreator(sLONG signature) const;

	void SetDefaultProgressIndicator_For_Indexes(VDB4DProgressIndicator* inProgress);
	VDB4DProgressIndicator* RetainDefaultProgressIndicator_For_Indexes();
	void SetDefaultProgressIndicator_For_DataCacheFlushing(VDB4DProgressIndicator* inProgress);
	VDB4DProgressIndicator* RetainDefaultProgressIndicator_For_DataCacheFlushing();

	inline VLocalizationManager* GetDefaultLocalization() const { return fDefaultLocalization; };
	inline void SetDefaultLocalization(VLocalizationManager* inLocalizationManager) 
	{ 
		 CopyRefCountable(&fDefaultLocalization, inLocalizationManager);
	};

	inline void Register_GetCurrentSelectionFromContextMethod(GetCurrentSelectionFromContextMethod inMethod) { fGetCurrentSelectionFromContextMethod = inMethod; };
	inline void Register_GetCurrentRecordFromContextMethod(GetCurrentRecordFromContextMethod inMethod) { fGetCurrentRecordFromContextMethod = inMethod; };

	inline GetCurrentSelectionFromContextMethod Get_GetCurrentSelectionFromContextMethod() const { return fGetCurrentSelectionFromContextMethod; };
	inline GetCurrentRecordFromContextMethod Get_GetCurrentRecordFromContextMethod() const { return fGetCurrentRecordFromContextMethod; };

	void ExecuteRequest( sWORD inRequestID, IRequestReply *inStreamRequest, CDB4DBaseContext *inContext);
	void ExecuteRequest( sWORD inRequestID, IStreamRequestReply *inRequest, CDB4DContext *inContext);

	void ExecOpenOrCreateBase(IRequestReply *inRequest, CDB4DContext *inContext);
	void ExecOpenBase(IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecSetBaseExtraProperties(IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecSetStructureExtraProperties(IRequestReply *inRequest, CDB4DBaseContext *inContext);

	void ExecAddTableWithBag(IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecAddTable(IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecDropTable(IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecSetTableName(IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecSetTableExtraProperties( IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecSetTableKeepStamp( IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecSetTableKeepRecordSyncInfo( IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecSetTablePrimaryKey( IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecSetTableHideInRest( IRequestReply *inRequest, CDB4DBaseContext *inContext);

	void ExecSetFieldName( IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecSetFieldAttributes( IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecSetFieldTextSwitchSize( IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecSetFieldNeverNull( IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecSetFieldStyledText( IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecSetFieldHideInRest( IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecSetFieldOutsideData( IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecSetFieldExtraProperties( IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecAddFieldsWithBagArray( IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecDropField( IRequestReply *inRequest, CDB4DBaseContext *inContext);

	void ExecAddRelation( IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecDropRelation( IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecSetRelationAttributes( IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecSetRelationExtraProperties( IRequestReply *inRequest, CDB4DBaseContext *inContext);

	void ExecCreateIndexOnOneField( IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecCreateIndexOnMultipleField( IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecCreateFullTextIndexOnOneField( IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecDropIndexOnOneField( IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecDropIndexOnMultipleField( IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecDropFullTextOnOneField( IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecSetIndexName( IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecDropIndexByRef( IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecRebuildIndexByRef( IRequestReply *inRequest, CDB4DBaseContext *inContext);

	void ExecLockDataBaseDef( IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecLockTableDef( IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecLockFieldDef( IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecLockRelationDef( IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecLockIndexDef( IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecUnlockObjectDef( IRequestReply *inRequest, CDB4DBaseContext *inContext);

	void ExecSetContextExtraData( IRequestReply *inRequest, CDB4DBaseContext *inContext);

	void ExecSelectAllRecords(IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecDeleteRecordsInSelection(IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecLoadRecord(IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecCountRecordsInTable(IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecWhoLockedRecord(IRequestReply *inRequest, CDB4DBaseContext *inContext);

	void ExecSaveRecord(IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecDeleteRecord(IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecConnectRecord(IRequestReply *inRequest, CDB4DBaseContext *inContext);

	void ExecSortSelection(IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecExecuteQuery(IRequestReply *inRequest, CDB4DBaseContext *inContext);

	void ExecSendLastRemoteInfo(IRequestReply *inRequest, CDB4DBaseContext *inContext);

	void ExecDataToCollection(IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecCollectionToData(IRequestReply *inRequest, CDB4DBaseContext *inContext);

	void ExecGetDistinctValues(IRequestReply *inRequest, CDB4DBaseContext *inContext);

	void ExecActivateManyToOne(IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecActivateManyToOneS(IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecActivateOneToMany(IRequestReply *inRequest, CDB4DBaseContext *inContext);

	void ExecRelateManySelection(IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecRelateOneSelection(IRequestReply *inRequest, CDB4DBaseContext *inContext);

	void ExecFindKey(IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecReserveRecordNumber(IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecScanIndex(IRequestReply *inRequest, CDB4DBaseContext *inContext);

	void ExecActivateRelsOnAPath(IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecCacheDisplaySelection(IRequestReply *inRequest, CDB4DBaseContext *inContext);

	void ExecSetFullyDeleteRecords(IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecSetTableSchema( IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecTruncateTable(IRequestReply *inRequest, CDB4DBaseContext *inContext);

	void ExecExecuteColumnFormulas(IRequestReply *inRequest, CDB4DBaseContext *inContext);

	void ExecDelRecFromSel(IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecDelSetFromSel(IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecDelRangeFromSel(IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecAddRecIDToSel(IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecReduceSel(IRequestReply *inRequest, CDB4DBaseContext *inContext);

	void ExecActivateAutomaticRelations_N_To_1(IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecActivateAutomaticRelations_1_To_N(IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecActivateAllAutomaticRelations(IRequestReply *inRequest, CDB4DBaseContext *inContext);

	void ExecDeleteRecordByID(IRequestReply *inRequest, CDB4DBaseContext *inContext);
	
	void ExecFillSetFromArrayOfBits(IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecFillSetFromArrayOfLongs(IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecFillArrayOfBitsFromSet(IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecFillArrayOfLongsFromSet(IRequestReply *inRequest, CDB4DBaseContext *inContext);

	void ExecSyncThingsToForget(IRequestReply *inRequest, CDB4DBaseContext *inContext);

	void ExecTestServer( IRequestReply *inRequest, CDB4DBaseContext *inContext);

	void ExecAskForASelectionPart(IRequestReply *inRequest, CDB4DBaseContext *inContext);

	void ExecGetListOfDeadTables(IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecResurectTable(IRequestReply *inRequest, CDB4DBaseContext *inContext);
	void ExecGetTableFragmentation(IRequestReply *inRequest, CDB4DBaseContext *inContext);

	VError StartServer(const VString& inServerName, sWORD inPortNum, bool inSSL = false);
	VError StopServer(sLONG inTimeOut);
	void CloseAllClientConnections(sLONG inTimeOut);
	void CloseClientConnection(CDB4DContext* inContext);

#if oldtempmem
	VError SetTempMemSize(VSize inSize, VSize *outActualSize);
	VSize GetTempMemSize() const { return fTempMemSize; };
	VCppMemMgr* GetTempMemMgr() { return &fTempMemMgr; };
#endif

	void MarkSetOnServerAsPermanent(Bittab* set);
	void UnMarkSetOnServerAsPermanent(Bittab* set, bool willResend);

	void MarkSelectionOnServerAsPermanent(Selection* sel);
	void UnMarkSelectionOnServerAsPermanent(Selection* sel, bool willResend);

	void KeepSetOnServer(Bittab* set);
	void KeepSelectionOnServer(Selection* sel);
	Selection* RetainServerKeptSelection(const VUUIDBuffer& inID);

	void ForgetServerKeptSelection(const VUUIDBuffer& inID);
	void AddReleasedSelID(const VUUIDBuffer& inID);
	void StealListOfReleasedSelIDs(vector<VUUIDBuffer>& outList); // to be called on client
	void ForgetServerKeptSelections(const vector<VUUIDBuffer>& inList); // to be called on server

	void KeepRecordOnServer(FicheInMem* rec, bool onlyonce = false);
	FicheInMem* RetainServerKeptRecord(const VUUIDBuffer& inID);

	void ForgetServerKeptRecord(const DB4D_RecordRef& inID);
	void AddReleasedRecID(const DB4D_RecordRef& inID, BaseTaskInfo* context);
	void StealListOfReleasedRecIDs(vector<DB4D_RecordRef>& outList, BaseTaskInfo* context); // to be called on client
	void ForgetServerKeptRecords(const vector<DB4D_RecordRef>& inList); // to be called on server

	void ForgetServerKeptRecords(BaseTaskInfo* context);

	void AddMarkRecordAsPushed(sLONG numtable, sLONG numrec);
	void AddUnMarkRecordAsPushed(sLONG numtable, sLONG numrec);
	void StealMarkedRecordsAsPushed(vectorPushedRecs& outRecs); // to be called on client

	bool HasSomeReleasedObjects(BaseTaskInfo* context);

	CDB4DContext* NewContext(CUAGSession* inUserSession, VJSGlobalContext* inJSContext, bool islocal);
	CDB4DContext* RetainOrCreateContext(const VUUID& inID, CUAGSession* inUserSession, VJSGlobalContext* inJSContext, bool islocal);

	void RegisterContext( VDB4DContext *inContext);
	void UnRegisterContext(const VUUID& inID);

	inline void SetRequestLogger(IRequestLogger* inLogger)
	{
		fRequestLogger = inLogger;
	}

	inline IRequestLogger* GetRequestLogger() const
	{
		return fRequestLogger;
	}

	VDB4DProgressIndicator* BuildProgressIndicator(sLONG ID, CDB4DBaseContext* inContext)
	{
		return nil;
	}

	void KeepPostPonedContext(const VString& sessionID, CDB4DContext* inContext);
	CDB4DContext* StealPostPonedContext(const VString& sessionID);
	void ForgetPostPonedContext(CDB4DContext* inContext);
	void ForgetAllPostPonedContext();


	bool AddKeptDataSet(DataSet* inDataSet);
	void ReleaseKeptDataSet(const VUUID& inID);
	DataSet* RetainKeptDataSet(const VUUID& inID);

	void GetKeptDataSetsInfo(bool minimum, VValueBag& outBag);
	void PurgeExpiredDataSets();
	void PurgeDataSetsForMem(sLONG allocationBlockNumber, VSize combien, VSize& outSizeFreed);

	void AddStructureToStoreAsXML(Base4D* inBase)
	{
		VTaskLock lock(&fStoreAsXMLMutex);
		fBasesToStoreAsXML.insert(inBase);
	}

	void RemoveStructureToStoreAsXML(Base4D* inBase)
	{
		VTaskLock lock(&fStoreAsXMLMutex);
		fBasesToStoreAsXML.erase(inBase);
	}

	VError StoreStructuresAsXML();

	inline void SetLimitPerSort(VSize inLimit)
	{
		fLimitPerSort = inLimit;
	}

	inline VSize GetLimitPerSort() const
	{
		return fLimitPerSort;
	}
/*
	inline VFolder* RetainComponentFolder()
	{
		return RetainRefCountable(fComponentFolder);
	}
*/

	inline VFolder* RetainResourceFolder()
	{
#if DB4DasComponent
		return fDB4DCompLib->GetLibrary()->RetainFolder(kBF_RESOURCES_FOLDER);
#else
		return RetainRefCountable(fResourceFolder);
#endif
	}

	VFolder* RetainInsideResourceFolder();
	VFolder* RetainJSCodeResourceFolder();

	VError RebuildStructure( const VFile& inStructureFile, const VFile& inDataFile);

	void AddBlobPathToDelete(const VString& inBlobPath);
	void RemoveBlobPathToDelete(const VString& inBlobPath);
	void StealBlobPathsToDelete(DataPathCollection& outList);
	void DeleteDeadBlobPaths();

	bool StartCacheLog(const XBOX::VFolder& inRoot, const XBOX::VString& inFileName, const XBOX::VValueBag *inParams = NULL);
	void StopCacheLog();
	bool IsCacheLogStarted() const;
	void AppendCommentToCacheLog(const XBOX::VString& inWhat);

	void	SetRunningMode( DB4D_Running_Mode inRunningMode);
	bool	IsRunning4D() const				{ return fRunningMode == DB4D_Running4D;}
	bool	IsRunningWakanda() const		{ return fRunningMode == DB4D_RunningWakanda;}
	
	void					SetApplicationInterface( IDB4D_ApplicationIntf *inApplication)	{ fApplicationIntf = inApplication;}
	IDB4D_ApplicationIntf*	GetApplicationInterface() const									{ return fApplicationIntf;}

	void					SetGraphicsInterface( IDB4D_GraphicsIntf *inGraphics)			{ fGraphicsIntf = inGraphics;}
	IDB4D_GraphicsIntf*		GetGraphicsInterface() const									{ return fGraphicsIntf;}

	void					SetSQLInterface (IDB4D_SQLIntf *inSQLIntf)						{ fSQLIntf = inSQLIntf; }
	IDB4D_SQLIntf			*GetSQLInterface () const										{ return fSQLIntf; }

	void GetStaticRequiredJSFiles(vector<VFilePath>& outFiles);

	VGarbageTask* GetGarbageTask()
	{
		return fGarbageTask;
	}

	inline void SetRestPrefix(const VString& inPrefix)
	{
		fRestPrefix = inPrefix;
	}

	inline const VString& GetRestPrefix() const
	{
		return fRestPrefix;
	}

protected:
	virtual void DoOnRefCountZero();

private:
	VDBMgr(VLocalizationManager* inLocalizationManager);
	virtual ~VDBMgr();
#if DB4DasDLL
	Boolean Init();
#else
	Boolean Init(VComponentLibrary* DB4DCompLib);
#endif
	
	CDB4DBase* xOpenCreateBase( const VFile& inStructureFile, Boolean inCreate, sLONG inParameters, VIntlMgr* inIntlMgr, VError* outErr = nil, 
								FileAccess inAccess = FA_READ_WRITE, const VUUID* inChosenID = nil, VString* EntityFileExt = nil, 
								CUAGDirectory* inUAGDirectory = nil, const VString* inXmlContent = nil, unordered_map_VString<VRefPtr<VFile> >* outIncludedFiles = nil, const VFile* inPermissionsFile = nil);
	
//	CDB4D_Lang* lang4D;
//	CLanguage* sLanguage;
	
	static VDBMgr	*sDB4DMgr; // there's only one manager
	static VCriticalSection	fMutex;	// protects sDB4DMgr
	
	VDBFlushMgr		*fFlushMgr;
	VDBCacheMgr		*fCacheMgr;
	DB4DCommandManager *fCommandManager;
	VDB4DSignaler		*fSignaler;
	DB4DActivityManager *fActivityManager;
	DB4DNetworkManager	*fNetworkManager;
	CUAGManager			*fUAGManager;

	VArrayPtrOf<VString*> ErrorStrings;
	VArrayPtrOf<VString*> ActionStrings;

	VArrayOf<base4dinfo>			fBases;
	mutable VCriticalSection	fBasesMutex; // protege fBases
	VString fUtil;
	VStringUTF8 fUtilUTF8;
	mapAVCreator fAVCreators;
	mapCollectionCreator fCollectionCreators;

	VDB4DProgressIndicator* fProgress_Indexes;
	VDB4DProgressIndicator* fProgress_Flush;
#if DB4DasComponent
	VComponentLibrary* fDB4DCompLib;
#endif
	VLocalizationManager* fDefaultLocalization;

#if DB4DasComponent
	VFolder* fComponentFolder;
#else
	VFolder* fResourceFolder;
#endif

	VFolder* fInsideResourceFolder;
	VFolder* fJSCodeResourceFolder;
	//static UniChar sWildChar;

	GetCurrentSelectionFromContextMethod fGetCurrentSelectionFromContextMethod;
	GetCurrentRecordFromContextMethod fGetCurrentRecordFromContextMethod;

	ServerManager* fDefaultServer;
	VCriticalSection fDefaultServerMutex;

#if oldtempmem
	VCppMemMgr	fTempMemMgr;
	VSize fTempMemSize;
#endif
	vectorPushedRecs fMarkedRecordsAsPushed;
	mapOfRecords fServerKeptRecords;
	mapOfSelections fServerKeptSelections;
	VCriticalSection fServerKeptSelectionsMutex;

	mapOfDataSet fKeptDataSets;
	VCriticalSection fKeptDataSetsMutex;
	uLONG fLastDataSetPurge;

	vector<VUUIDBuffer> fClientReleasedSelIDs;
	vector<DB4D_RecordRef> fClientReleasedRecIDs;

	mapOfContextBySessionID fPostPonedContexts;
	mapOfContext fAllContexts;
	VCriticalSection fAllContextsMutex;

	BuildLanguageExpressionMethod fCurrentBuildLanguageExpressionMethod;
	IRequestLogger* fRequestLogger;

	VCriticalSection fProgressTaskMutex;
	ServerProgressTask* fServerProgressTask;
	ClientProgressTask* fClientProgressTask;

	mapOfProgress fClientProgresses;
	mapOfProgress fServerProgresses;

	set<Base4D*> fBasesToStoreAsXML;
	VCriticalSection fStoreAsXMLMutex;

	sWORD fServerPort;
	VSize fLimitPerSort;

	DataPathCollection fBlobPathsToDelete;
	VCriticalSection fBlobPathsToDeleteMutex;

	DataSetPurger	fDataSetPurger;

	DB4D_Running_Mode	fRunningMode;

	IDB4D_ApplicationIntf	*fApplicationIntf;
	IDB4D_GraphicsIntf		*fGraphicsIntf;
	IDB4D_SQLIntf			*fSQLIntf;

	VGarbageTask* fGarbageTask;

	VString fRestPrefix;
};



class VDB4DSignaler : public VObject, public DB4DSignaler, public DB4DCommandManager
{
public:
	VDB4DSignaler( VDBMgr *inManager);
	
	// from DB4DSignaler
	virtual VSignal_AddBase*		GetSignal_AddBase();
	virtual VSignal_CloseBase*		GetSignal_CloseBase();
	virtual VSignal_UpdateBase*		GetSignal_UpdateBase();

	virtual VSignal_AddTable*		GetSignal_AddTable();
	virtual VSignal_DelTable*		GetSignal_DelTable();
	virtual VSignal_UpdateTable*	GetSignal_UpdateTable();

	virtual VSignal_AddField*		GetSignal_AddField();
	virtual VSignal_DelField*		GetSignal_DelField();
	virtual VSignal_UpdateField*	GetSignal_UpdateField();

	virtual VSignal_AddIndex*		GetSignal_AddIndex();
	virtual VSignal_DelIndex*		GetSignal_DelIndex();
	virtual VSignal_UpdateIndex*	GetSignal_UpdateIndex();

	virtual VSignal_AddRelation*	GetSignal_AddRelation();
 	virtual VSignal_DelRelation*	GetSignal_DelRelation();
 	virtual VSignal_UpdateRelation*	GetSignal_UpdateRelation();

	// from DB4DCommandManager
	virtual void					Tell_AddBase( const XBOX::VUUID& inBaseUUID);
	virtual void					Tell_CloseBase( const XBOX::VUUID& inBaseUUID);
	virtual void					Tell_UpdateBase( const XBOX::VUUID& inBaseUUID, ETellUpdateBaseOptions inWhat);
	
	virtual void					Tell_AddTable( const XBOX::VUUID& inBaseUUID, const XBOX::VUUID& inTableUUID, sLONG inTableID);
	virtual void					Tell_DelTable( const XBOX::VUUID& inBaseUUID, const XBOX::VUUID& inTableUUID, sLONG inTableID);
	virtual void					Tell_UpdateTable( const XBOX::VUUID& inBaseUUID, const XBOX::VUUID& inTableUUID, sLONG inTableID, ETellUpdateTableOptions inWhat);
	
	virtual void					Tell_AddField( const XBOX::VUUID& inBaseUUID, const XBOX::VUUID& inTableUUID, const XBOX::VUUID& inFieldUUID, sLONG inTableID, sLONG inFieldID);
	virtual void					Tell_DelField( const XBOX::VUUID& inBaseUUID, const XBOX::VUUID& inTableUUID, const XBOX::VUUID& inFieldUUID, sLONG inTableID, sLONG inFieldID);
	virtual void					Tell_UpdateField( const XBOX::VUUID& inBaseUUID, const XBOX::VUUID& inTableUUID, const XBOX::VUUID& inFieldUUID, sLONG inTableID, sLONG inFieldID, ETellUpdateFieldOptions inWhat);
	
	virtual void					Tell_AddIndex( const XBOX::VUUID& inBaseUUID, const XBOX::VUUID& inIndexUUID);
	virtual void					Tell_DelIndex( const XBOX::VUUID& inBaseUUID, const XBOX::VUUID& inIndexUUID);
	virtual void					Tell_UpdateIndex( const XBOX::VUUID& inBaseUUID, const XBOX::VUUID& inIndexUUID, ETellUpdateIndexOptions inWhat);

	virtual void					Tell_AddRelation( const XBOX::VUUID& inBaseUUID, const XBOX::VUUID& inRelationID);
	virtual void					Tell_DelRelation( const XBOX::VUUID& inBaseUUID, const XBOX::VUUID& inRelationID);
	virtual void					Tell_UpdateRelation( const XBOX::VUUID& inBaseUUID, const XBOX::VUUID& inRelationID, ETellUpdateRelationOptions inWhat);

private:
			bool					GetTableUUID( const XBOX::VUUID& inBaseUUID, sLONG inTableID, VUUID& outUUID);
			bool					GetTableAndFieldUUID( const XBOX::VUUID& inBaseUUID, sLONG inTableID, sLONG inFieldID, VUUID& outTableUUID, VUUID& outFieldUUID);

	VDBMgr*						fManager;
	
	VSignal_AddBase				fSignal_AddBase;
	VSignal_CloseBase			fSignal_CloseBase;
	VSignal_UpdateBase			fSignal_UpdateBase;

	VSignal_AddTable			fSignal_AddTable;
	VSignal_DelTable			fSignal_DelTable;
	VSignal_UpdateTable			fSignal_UpdateTable;

	VSignal_AddField			fSignal_AddField;
	VSignal_DelField			fSignal_DelField;
	VSignal_UpdateField			fSignal_UpdateField;

	VSignal_AddIndex			fSignal_AddIndex;
	VSignal_DelIndex			fSignal_DelIndex;
	VSignal_UpdateIndex			fSignal_UpdateIndex;

	VSignal_AddRelation			fSignal_AddRelation;
 	VSignal_DelRelation			fSignal_DelRelation;
 	VSignal_UpdateRelation		fSignal_UpdateRelation;
};


class VGarbageTask : public VTask
{
	public:
		VGarbageTask(VDBMgr* inMgr);

		void SetJSGarbageInterval(sLONG value)
		{
			fJSGarbageCollectInterval = value;
		}

		sLONG GetJSGarbageInterval() const
		{
			return fJSGarbageCollectInterval;
		}

	protected:
		virtual void DoInit();
		virtual void DoDeInit();
		virtual Boolean DoRun();

		VDBMgr* fDBMgr;
		sLONG fJSGarbageCollectInterval; // in milliseconds;
		uLONG fLastJSGarbageCollect;
};

#endif
