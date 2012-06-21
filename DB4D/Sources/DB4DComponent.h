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
#ifndef __DB4DComponent__
#define __DB4DComponent__

#include "../Headers/DB4D.h"
//#include "VComponentLibrary.h"

class VDB4DQuery;
class VDB4DRowSet;



class VDB4DMgr : public VComponentImp<CDB4DManager>
{
public:
	VDB4DMgr();
	virtual ~VDB4DMgr();


	virtual void __test(const VString& command);

	virtual DB4DNetworkManager* CreateRemoteTCPSession(const VString& inServerName, sWORD inPortNum, VError& outerr, bool inSSL = false);

	virtual CDB4DBase* OpenRemoteBase( CDB4DBase* inLocalDB, const XBOX::VUUID& inBaseID, VErrorDB4D& outErr, DB4DNetworkManager *inLegacyNetworkManager,CUAGDirectory* inUAGDirectory = nil);
	virtual CDB4DBase* OpenRemoteBase( CDB4DBase* inLocalDB, const VString& inBaseName, const VString& inFullBasePath, VErrorDB4D& outErr, 
										Boolean inCreateIfNotExist, DB4DNetworkManager* inNetworkManager = nil, CUAGDirectory* inUAGDirectory = nil);
	virtual CDB4DBase* OpenRemoteBase( CDB4DBase* inLocalDB, const VString& inBaseName, VErrorDB4D& outErr, DB4DNetworkManager* inNetworkManager = nil, CUAGDirectory* inUAGDirectory = nil);

	virtual void* GetComponentLibrary() const;

	virtual CDB4DBase* OpenBase( const VString& inXMLContent, sLONG inParameters, VError* outErr, const VFilePath& inXMLPath, unordered_map_VString<VRefPtr<VFile> >* outIncludedFiles  = nil);
	// one structure file and one datafile whose name is structurename.data
	virtual CDB4DBase* OpenBase( const VFile& inStructureFile, sLONG inParameters, VError* outErr = nil, FileAccess inAccess = FA_READ_WRITE, VString* EntityFileExt = nil, CUAGDirectory* inUAGDirectory = nil);
	virtual CDB4DBase* CreateBase( const VFile& inStructureFile, sLONG inParameters, VIntlMgr* inIntlMgr, 
									VError* outErr = nil, FileAccess inAccess = FA_READ_WRITE, const VUUID* inChosenID = nil, VString* EntityFileExt = nil, CUAGDirectory* inUAGDirectory = nil);


	// virtual VError CloseBase( DB4D_BaseID inBaseID);

	// virtual CDB4DBaseContextPtr NewBaseContext( VUUID& inBaseID);

	virtual sLONG CountBases();
	virtual CDB4DBase* RetainBaseByUUID(const VUUID& inBaseID);
	virtual CDB4DBase* RetainNthBase( sLONG inBaseIndex);

	virtual void FlushCache(Boolean WaitUntilDone = false, bool inEmptyCacheMem = false, ECacheLogEntryKind inOrigin = eCLEntryKind_FlushFromUnknown);
	virtual	void SetFlushPeriod( sLONG inMillisecondsPeriod);

	//virtual VCommandList* GetCommandList();

	virtual DB4DSignaler* GetSignaler();
	virtual DB4DCommandManager* SetCommandManager(DB4DCommandManager* inNewCommandManager);

	virtual DB4DActivityManager* SetActivityManager(DB4DActivityManager* inNewActivityManager);
	virtual	DB4DNetworkManager*	SetNetworkManager( DB4DNetworkManager* inNewNetworkManager);

	virtual BuildLanguageExpressionMethod SetMethodToBuildLanguageExpressions(BuildLanguageExpressionMethod inNewMethodToBuildLanguageExpressions);

	virtual VSize GetMinimumCacheSize() const;
	virtual VErrorDB4D SetCacheSize(VSize inSize, bool inPhysicalMemOnly, VSize *outActualSize);
	virtual VErrorDB4D SetMinSizeToFlush( VSize inMinSizeToFlush);
	virtual VSize GetCacheSize();
	virtual VSize GetMinSizeToFlush();

	virtual VCppMemMgr *GetCacheMemoryManager();

	virtual VErrorDB4D SetTempMemSize(VSize inSize, VSize *outActualSize);
	virtual VSize GetTempMemSize();
	virtual VCppMemMgr* GetTempMemManager();

	virtual CDB4DContext* NewContext(CUAGSession* inUserSession, VJSGlobalContext* inJSContext, bool isLocalOnly = false);
	virtual CDB4DContext* RetainOrCreateContext(const VUUID& inID, CUAGSession* inUserSession, VJSGlobalContext* inJSContext, bool isLocalOnly = false);

	virtual void StartDebugLog();

	//virtual UniChar GetWildChar();
	//virtual void SetWildChar(UniChar c);

	virtual	bool	ContainsKeyword( const VString& inString, const VString& inKeyword, bool inDiacritical, VIntlMgr *inIntlMgr, VErrorDB4D& outErr);
	virtual	bool	ContainsKeyword_Like( const VString& inString, const VString& inKeyword, bool inDiacritical, VIntlMgr *inIntlMgr, VErrorDB4D& outErr);

	virtual	bool	ContainsKeyword( const VString& inString, const VString& inKeyword, const VCompareOptions& inOptions, VErrorDB4D& outErr);

	virtual uLONG8 GetDB4D_VersionNumber();

	virtual VErrorDB4D Register_DB4DArrayOfValuesCreator(sLONG signature, DB4DArrayOfValuesCreator Creator);
	virtual VErrorDB4D UnRegister_DB4DArrayOfValuesCreator(sLONG signature);

	virtual VErrorDB4D Register_DB4DCollectionManagerCreator(sLONG signature, DB4DCollectionManagerCreator Creator);
	virtual VErrorDB4D UnRegister_DB4DCollectionManagerCreator(sLONG signature);

	virtual void SetDefaultProgressIndicator_For_Indexes(VDB4DProgressIndicator* inProgress);
	virtual VDB4DProgressIndicator* RetainDefaultProgressIndicator_For_Indexes();
	virtual void SetDefaultProgressIndicator_For_DataCacheFlushing(VDB4DProgressIndicator* inProgress);
	virtual VDB4DProgressIndicator* RetainDefaultProgressIndicator_For_DataCacheFlushing();

	virtual bool CheckCacheMem();

	// the file can be opened by the db4d manager or not
	virtual const VValueBag* RetainExtraProperties( VFile* inFile, bool *outWasOpened, VUUID *outUUID, VError &err, CDB4DBaseContextPtr inContext = NULL, uLONG8* outVersionNumber = NULL);

	virtual CDB4DRawDataBase* OpenRawDataBase(XBOX::VFile* inStructFile, XBOX::VFile* inDataFile, IDB4D_DataToolsIntf* inDataToolLog, VErrorDB4D& outError, FileAccess access = FA_MAX);
	virtual CDB4DRawDataBase* OpenRawDataBaseWithEm(CDB4DBase* inStructureDB, XBOX::VFile* inDataFile, IDB4D_DataToolsIntf* inDataToolLog, VErrorDB4D& outError, FileAccess access = FA_MAX);

	virtual VErrorDB4D ReleaseRawRecord(void* inRawRecord);
	virtual void* GetRawRecordNthField(void* inRawRecord, sLONG inFieldIndex, sLONG& outType, VErrorDB4D* outError = nil, bool forQueryOrSort = true);
	virtual RecIDType GetRawRecordID(void* inRawRecord, VErrorDB4D* outError = nil);

	virtual void Register_GetCurrentSelectionFromContextMethod(GetCurrentSelectionFromContextMethod inMethod);
	virtual void Register_GetCurrentRecordFromContextMethod(GetCurrentRecordFromContextMethod inMethod);

	virtual	void	ExecuteRequest( sWORD inRequestID, IStreamRequestReply *inRequest, CDB4DBaseContext *inContext);

	virtual VError StartServer(const VString& inServerName, sWORD inPortNum, bool inSSL = false);
	virtual VError StopServer(sLONG inTimeOut);
	virtual void CloseAllClientConnections(sLONG inTimeOut);

	virtual void CloseConnectionWithClient(CDB4DContext* inContext);

	virtual CDB4DJournalParser* NewJournalParser();

	virtual	VErrorDB4D	BuildValid4DTableName( const VString& inName, VString& outValidName) const;
	virtual	VErrorDB4D	BuildValid4DFieldName( const VString& inName, VString& outValidName) const;

	// if inBase is NULL, none check on table name unicity is done
	virtual	VErrorDB4D	IsValidTableName( const VString& inName, EValidateNameOptions inOptions, CDB4DBase *inBase = NULL, CDB4DTable *inTable = NULL) const;
	// if inBase is NULL, none check on link name unicity is done
	virtual	VErrorDB4D	IsValidLinkName( const VString& inName, EValidateNameOptions inOptions, bool inIsNameNto1 = true, CDB4DBase *inBase = NULL, CDB4DRelation *inRelation = NULL) const;
	// if inTable is NULL, one check on field name unicity is done
	virtual	VErrorDB4D	IsValidFieldName( const VString& inName, EValidateNameOptions inOptions, CDB4DTable *inTable = NULL, CDB4DField *inField = NULL) const;

	virtual void StealListOfReleasedSelIDs(vector<VUUIDBuffer>& outList); // to be called on client
	virtual void ForgetServerKeptSelections(const vector<VUUIDBuffer>& inList); // to be called on server

	virtual void StealListOfReleasedRecIDs(vector<DB4D_RecordRef>& outList); // to be called on client
	virtual void ForgetServerKeptRecords(const vector<DB4D_RecordRef>& inList); // to be called on server

	virtual void SetRequestLogger(IRequestLogger* inLogger);

	// Attach the base context owner (the db4d context) to the JavaScript context and append the "db" and "ds" properties to the global object (4D language context need)
	virtual	void				InitializeJSGlobalObject( XBOX::VJSGlobalContext *inJSContext, CDB4DBaseContext *inBaseContext);
	// Attach the DB4D context to the JavaScript context
	virtual	void				InitializeJSContext( XBOX::VJSGlobalContext *inContext, CDB4DContext *inDB4DContext) ;
	// Remove private attachments according to the attached DB4D context and detach the DB4D context
	virtual	void				UninitializeJSContext( XBOX::VJSGlobalContext *inContext);
#if 0
	// Remove private attachments for a specific base context
	virtual	void				UninitializeJSContext( XBOX::VJSGlobalContext *inContext, CDB4DBaseContext *inBaseContext);
#endif
	
	virtual	CDB4DContext*		GetDB4DContextFromJSContext( const XBOX::VJSContext& inContext);
	virtual	CDB4DBaseContext*	GetDB4DBaseContextFromJSContext( const VJSContext& inContext, CDB4DBase *inBase);

	virtual	VJSObject			CreateJSDatabaseObject( const VJSContext& inContext, CDB4DBaseContext *inBaseContext);
	virtual	VJSObject			CreateJSEMObject( const VString& emName, const VJSContext& inContext, CDB4DBaseContext *inBaseContext);
	virtual	void				PutAllEmsInGlobalObject(VJSObject& globalObject, CDB4DBaseContext *inBaseContext);

	virtual void SetLimitPerSort(VSize inLimit);

	virtual VSize GetLimitPerSort() const;

	virtual void StartRecordingMemoryLeaks()
	{
#if debugLeakCheck_Strong
		xStartRecordingMemoryLeaks();
#endif
	}

	virtual void StopRecordingMemoryLeaks()
	{
#if debugLeakCheck_Strong
		xStopRecordingMemoryLeaks();
#endif
	}

	virtual void DumpMemoryLeaks(XBOX::VString& outText)
	{
#if debugLeakCheck_Strong
		xDumpMemoryLeaks(outText);
#endif
	}

	virtual void PutThingsToForget( VStream* into, CDB4DBaseContext* context)
	{
		_PutThingsToForget(into, fManager, ConvertContext(context));
	}

	virtual VErrorDB4D GetThingsToForget( VStream* clientreq, CDB4DBaseContext* context)
	{
		return _GetThingsToForget(clientreq, fManager, ConvertContext(context));
	}

	virtual VErrorDB4D RebuildMissingStructure(const XBOX::VFile& inStructureToRebuild, const XBOX::VFile& inExistingDataFile)
	{
		return fManager->RebuildStructure(inStructureToRebuild, inExistingDataFile);
	}

	virtual bool StartCacheLog(const XBOX::VFolder& inRoot, const XBOX::VString& inFileName, const XBOX::VValueBag *inParams = NULL);
	virtual void StopCacheLog();
	virtual bool IsCacheLogStarted() const;
	virtual void AppendCommentToCacheLog(const XBOX::VString& inWhat);

	virtual void SetRunningMode( DB4D_Running_Mode inRunningMode);

	virtual	void					SetApplicationInterface( IDB4D_ApplicationIntf *inApplication);
	virtual	IDB4D_ApplicationIntf*	GetApplicationInterface() const;

	virtual	void					SetGraphicsInterface( IDB4D_GraphicsIntf *inGraphics);
	virtual	IDB4D_GraphicsIntf*		GetGraphicsInterface() const;

	virtual void GetStaticRequiredJSFiles(std::vector<XBOX::VFilePath>& outFiles);

	virtual IDB4D_DataToolsIntf* CreateJSDataToolsIntf(VJSContext& jscontext, VJSObject& paramObj);

	virtual VError EraseDataStore(VFile* dataDS);

protected:
	VDBMgr *fManager;
};



class VDB4DBase : public VComponentImp<CDB4DBase>
{
public:
	VDB4DBase( VDBMgr *inManager, Base4D *inBase, Boolean mustRetain = true);

	virtual ~VDB4DBase();

	// virtual void DoOnRefCountZero();

	virtual Boolean MatchBase(CDB4DBase* InBaseToCompare) const; 
	virtual CDB4DBaseContextPtr NewContext(CUAGSession* inUserSession, VJSGlobalContext* inJSContext, bool isLocalOnly = false); 
	virtual void GetName( VString& outName, CDB4DBaseContextPtr inContext = NULL) const;
	virtual void GetUUID(VUUID& outID, CDB4DBaseContextPtr inContext = NULL) const;
	virtual const VUUID& GetUUID(CDB4DBaseContextPtr inContext = NULL) const;
	virtual void GetBasePath( VFilePath& outPath, CDB4DBaseContextPtr inContext = NULL) const; // this may be the folder for temp files in c/s or the first data segment path
	virtual VValueBag *CreateDefinition( bool inWithTables, bool inWithIndices, bool inWithRelations, CDB4DBaseContextPtr inContext = NULL) const;

	virtual void TouchXML();

	virtual Boolean IsDataOpened(CDB4DBaseContextPtr inContext = NULL) const;
	virtual Boolean OpenDefaultData( Boolean inCreateItIfNotFound, sLONG inParameters, CDB4DBaseContextPtr inContext = NULL, VErrorDB4D* outErr = nil, FileAccess inAccess = FA_READ_WRITE) const;
	virtual Boolean CreateData( const VFile& inDataFile, sLONG inParameters, VFile *inJournalingFile = NULL, CDB4DBaseContextPtr inContext = NULL, VErrorDB4D* outErr = nil, FileAccess inAccess = FA_READ_WRITE) const;
	virtual Boolean OpenData( const VFile& inDataFile, sLONG inParameters, CDB4DBaseContextPtr inContext = NULL, VErrorDB4D* outErr = nil, FileAccess inAccess = FA_READ_WRITE) const;
	virtual VError LoadIndexesAfterCompacting(sLONG inParameters);

	virtual XBOX::VFile* RetainIndexFile() const; 

	virtual sLONG CountTables(CDB4DBaseContextPtr inContext = NULL) const;
	virtual VError CreateTable( const VValueBag& inTableDefinition, CDB4DTable **outTable, CDB4DBaseContextPtr inContext = NULL, Boolean inGenerateName = false);
	virtual VValueBag *CreateTableDefinition( DB4D_TableID inTableID, CDB4DBaseContextPtr inContext = NULL) const;
	virtual CDB4DTable* RetainNthTable(sLONG inIndex, CDB4DBaseContextPtr inContext = NULL) const;
	virtual CDB4DTable* FindAndRetainTable(const VString& inName, CDB4DBaseContextPtr inContext = NULL) const;
	virtual CDB4DTable* FindAndRetainTable(const VUUID& inID, CDB4DBaseContextPtr inContext = NULL) const;
	virtual CDB4DTable* FindAndRetainTableInStruct(const VUUID& inID, CDB4DBaseContextPtr inContext = NULL) const;
	virtual Boolean DropTable(DB4D_TableID inTableID, VDB4DProgressIndicator* InProgress = nil, CDB4DBaseContextPtr inContext = NULL);

	virtual CDB4DTable* CreateOutsideTable( const XBOX::VString& inTableName, CDB4DBaseContextPtr inContext = NULL, VErrorDB4D* outErr = NULL) const;
	virtual VError AddTable( CDB4DTable *inTable, CDB4DBaseContextPtr inContext = NULL);
	virtual VError GetNameForNewTable( XBOX::VString& outName) const;

	virtual CDB4DField* FindAndRetainField(const VUUID& inID, CDB4DBaseContextPtr inContext = NULL) const;
	virtual CDB4DField* FindAndRetainField(const VString& inTableName, const VString& inFieldName, CDB4DBaseContextPtr inContext = NULL) const;
	virtual CDB4DField* FindAndRetainField(const VString& inFieldName, CDB4DBaseContextPtr inContext = NULL) const;

/*
	virtual sLONG CountIndices() const;
	virtual CDB4DIndex* RetainNthIndex(sLONG inIndexNum) const;
	virtual VValueBag* CreateIndexDefinition(sLONG inIndexNum) const;
*/

	virtual VError CreateIndexOnOneField(CDB4DField* target, sLONG IndexTyp, Boolean UniqueKeys = false, 
										VDB4DProgressIndicator* InProgress = nil, const VString *inIndexName = nil,
										CDB4DIndex** outResult = NULL, Boolean ForceCreate = true, VSyncEvent* event = NULL,
										CDB4DBaseContextPtr inContext = NULL);

	virtual VError CreateIndexOnMultipleField(const VValueBag& IndexDefinition, Boolean UniqueKeys = false, 
											VDB4DProgressIndicator* InProgress = nil, CDB4DIndex** outResult = NULL, 
											Boolean ForceCreate = true, VSyncEvent* event = NULL,
											CDB4DBaseContextPtr inContext = NULL);

	virtual VError CreateIndexOnMultipleField(const CDB4DFieldArray& inTargets, sLONG IndexTyp, Boolean UniqueKeys = false, 
											VDB4DProgressIndicator* InProgress = nil, const VString *inIndexName = nil,
											CDB4DIndex** outResult = NULL, Boolean ForceCreate = true, VSyncEvent* event = NULL,
											CDB4DBaseContextPtr inContext = NULL);

	virtual VError DropIndexOnOneField(CDB4DField* target, sLONG IndexTyp = 0, VDB4DProgressIndicator* InProgress = nil, VSyncEvent* event = NULL,
										CDB4DBaseContextPtr inContext = NULL);

	virtual VError DropIndexOnMultipleField(const VValueBag& IndexDefinition, VDB4DProgressIndicator* InProgress = nil, VSyncEvent* event = NULL,
											CDB4DBaseContextPtr inContext = NULL);

	virtual VError DropIndexOnMultipleField(const CDB4DFieldArray& inTargets, sLONG IndexTyp = 0, 
											VDB4DProgressIndicator* InProgress = nil, VSyncEvent* event = NULL, CDB4DBaseContextPtr inContext = NULL);

	virtual VError DropIndexByName(const XBOX::VString& inIndexName, VDB4DProgressIndicator* InProgress = nil, VSyncEvent* event = NULL,
									CDB4DBaseContextPtr inContext = NULL);

	virtual VError CreateFullTextIndexOnOneField(CDB4DField* target, VDB4DProgressIndicator* InProgress = NULL, 
												const VString *inIndexName = nil, CDB4DIndex** outResult = NULL, 
												Boolean ForceCreate = true, VSyncEvent* event = NULL, CDB4DBaseContextPtr inContext = NULL);

	virtual VError DropFullTextIndexOnOneField(CDB4DField* target, VDB4DProgressIndicator* InProgress = NULL, VSyncEvent* event = NULL,
												CDB4DBaseContextPtr inContext = NULL);

	virtual CDB4DIndex* FindAndRetainIndexByName(const VString& inIndexName, CDB4DBaseContextPtr inContext = NULL, Boolean MustBeValid = true, Boolean Occupy = false) const;
	virtual CDB4DIndex* FindAndRetainIndexByUUID( const VUUID& inUUID, CDB4DBaseContextPtr inContext = NULL, Boolean MustBeValid = true, Boolean Occupy = false) const;

	virtual CDB4DIndex* RetainIndex(sLONG inNumTable, sLONG inNumField, Boolean MustBeSortable = false, CDB4DBaseContextPtr inContext = NULL, Boolean MustBeValid = true, Boolean Occupy = false) const;
	virtual CDB4DIndex* RetainFullTextIndex(sLONG inNumTable, sLONG inNumField, CDB4DBaseContextPtr inContext = NULL, Boolean MustBeValid = true, Boolean Occupy = false) const;
	virtual CDB4DIndex* RetainCompositeIndex(const CDB4DFieldArray& inTargets, Boolean MustBeSortable = false, CDB4DBaseContextPtr inContext = NULL, Boolean MustBeValid = true, Boolean Occupy = false) const;


	/* IndexDefinition content : 'fields' array of 
	'FieldName' : string
	'TableName' : string
	ou bien
	'FieldID' 	: long
	'TableID' 	: long
	end of array

	'IndexType' : long ( default = BTree )
	*/


	virtual VError CreateRelation( const VValueBag& inBag, CDB4DRelation **outRelation, CDB4DBaseContextPtr inContext = NULL);

	virtual CDB4DRelation *CreateRelation(const VString &inRelationName, const VString &inCounterRelationName,
											CDB4DField* inSourceField, CDB4DField* inDestinationField, CDB4DBaseContextPtr inContext = NULL);

	virtual CDB4DRelation *CreateRelation(const VString &inRelationName, const VString &inCounterRelationName,
											const CDB4DFieldArray& inSourceFields, const CDB4DFieldArray& inDestinationFields, CDB4DBaseContextPtr inContext = NULL);

	virtual CDB4DRelation* FindAndRetainRelationByName(const VString& Name, CDB4DBaseContextPtr inContext = NULL) const;

	virtual CDB4DRelation* FindAndRetainRelation(CDB4DField* inSourceField, CDB4DField* inDestinationField, 
												CDB4DBaseContextPtr inContext = NULL) const;

	virtual CDB4DRelation* FindAndRetainRelation(const CDB4DFieldArray& inSourceFields, const CDB4DFieldArray& inDestinationFields, 
												CDB4DBaseContextPtr inContext = NULL) const;

	virtual CDB4DRelation* FindAndRetainRelationByUUID(const VUUID& inID, CDB4DBaseContextPtr inContext = NULL) const;

	virtual VError GetAndRetainRelations(RelationArray &outRelations, CDB4DBaseContextPtr inContext = NULL) const;

	virtual VErrorDB4D IsFieldsKindValidForRelation( CDB4DField* inSourceField, CDB4DField* inDestinationField) const;

	virtual Boolean CreateElements( const VValueBag& inBag, VBagLoader *inLoader, CDB4DBaseContextPtr inContext = NULL);

	virtual CDB4DCheckAndRepairAgent *NewCheckAndRepairAgent();

	virtual	uBOOL	Lock( CDB4DBaseContextPtr inContext, Boolean WaitForEndOfRecordLocks = false, sLONG TimeToWaitForEndOfRecordLocks = 0 );
	virtual	void	UnLock( CDB4DBaseContextPtr inContext );
	virtual uBOOL	FlushAndLock( CDB4DBaseContextPtr inContext, Boolean WaitForEndOfRecordLocks = false, sLONG TimeToWaitForEndOfRecordLocks = 0 );


	virtual void RegisterForLang(void);
	virtual void UnRegisterForLang(void);
//	virtual CNameSpace* GetNameSpace() const;


	virtual void Close(VSyncEvent* waitForClose = NULL);
	virtual void CloseAndRelease(Boolean WaitUntilFullyClosed = true);

	virtual CDB4DBase* RetainStructDatabase(const char* DebugInfo) const;

	virtual CDB4DAutoSeqNumber* CreateAutoSeqNumber(DB4D_AutoSeq_Type inType, VErrorDB4D& err, CDB4DBaseContextPtr inContext = NULL);
	virtual VErrorDB4D DropAutoSeqNumber(const XBOX::VUUID& inIDtoDelete, CDB4DBaseContextPtr inContext = NULL);
	virtual CDB4DAutoSeqNumber* RetainAutoSeqNumber(const XBOX::VUUID& inIDtoFind, CDB4DBaseContextPtr inContext = NULL);

	virtual VError SetJournalFile(VFile* inNewLog, const VUUID* inDataLink = NULL, CDB4DBaseContextPtr inContext = NULL, bool inResetJournalSequenceNumber = false);
	virtual VError SetJournalFileInfos( VString *inFilePath, VUUID *inUUID );
	virtual VError ResetJournalFileContent();
	virtual VFile* RetainJournalFile(CDB4DBaseContextPtr inContext = NULL);
	virtual bool GetJournalUUIDLink( VUUID &outLink );
	virtual void GetJournalInfos( const VFilePath &inDataFilePath, VFilePath &outFilePath, VUUID &outDataLink);
	virtual VError OpenJournal( VFile *inFile, VUUID &inDataLink, bool inWriteOpenDataOperation = true);

	virtual const VValueBag* RetainExtraProperties(VError &err, CDB4DBaseContextPtr inContext = NULL);
	virtual VError SetExtraProperties(VValueBag* inExtraProperties, CDB4DBaseContextPtr inContext = NULL);

	virtual const VValueBag* RetainStructureExtraProperties(VError &err, CDB4DBaseContextPtr inContext = NULL);
	virtual VError SetStructureExtraProperties(VValueBag* inExtraProperties, CDB4DBaseContextPtr inContext = NULL);

	virtual VErrorDB4D DelayAllIndexes();
	virtual VErrorDB4D AwakeAllIndexes(vector<CDB4DIndex*>& outIndexWithProblems, VDB4DProgressIndicator* inProgress = NULL, Boolean WaitForCompletion = true);

	virtual DB4DTriggerManager* SetTriggerManager(DB4DTriggerManager* inNewTriggerManager);
	virtual DB4DTriggerManager* GetTriggerManager();

	virtual VErrorDB4D CountBaseFreeBits( BaseStatistics &outStatistics ) const;

	virtual void StartDataConversion();
	virtual void StopDataConversion();

	virtual void SetReferentialIntegrityEnabled(Boolean state);
	virtual void SetCheckUniquenessEnabled(Boolean state);
	virtual void SetCheckNot_NullEnabled(Boolean state);

	virtual CDB4DRawDataBase* OpenRawDataBase(IDB4D_DataToolsIntf* inDataToolLog, VErrorDB4D& outError);

	virtual sLONG GetStamp(CDB4DBaseContextPtr inContext = NULL) const;
	virtual sLONG GetExtraPropertiesStamp(CDB4DBaseContextPtr inContext = NULL) const;

	virtual void GetTablesStamps(StampsVector& outStamps, CDB4DBaseContextPtr inContext = NULL) const;
	virtual void GetRelationsStamps(StampsVector& outStamps, CDB4DBaseContextPtr inContext = NULL) const;

	virtual VError RetainIndexes(ArrayOf_CDB4DIndex& outIndexes, CDB4DBaseContextPtr inContext = NULL);

	virtual CDB4DComplexQuery* NewComplexQuery();

	virtual CDB4DComplexSelection* ExecuteQuery( CDB4DComplexQuery *inQuery, CDB4DBaseContextPtr inContext, CDB4DComplexSelection* Filter = NULL, 
													VDB4DProgressIndicator* InProgress = NULL, DB4D_Way_of_Locking HowToLock = DB4D_Do_Not_Lock, 
													sLONG limit = 0, VErrorDB4D *outErr = NULL);

#if debuglr
	virtual CComponent*	Retain (const char* DebugInfo = 0);
	virtual void		Release (const char* DebugInfo = 0);
#endif

	virtual Boolean IsRemote() const { return fIsRemote; };

	virtual VError CheckForUpdate();

	virtual VError IntegrateJournal(CDB4DJournalParser* inJournal, uLONG8 from = 0, uLONG8 upto = 0, uLONG8 *outCountIntegratedOperations = NULL, VDB4DProgressIndicator* InProgress = NULL);

	virtual VError GetErrorOnJournalFile() const;

	virtual sLONG8 GetCurrentLogOperation() const;
	virtual sLONG8 GetCurrentLogOperationOnDisk() const;

	virtual Boolean IsWriteProtected() const;

	virtual VFileDesc* GetDataIndexSeg() const;
	virtual VFileDesc* GetStructIndexSeg() const;

	virtual VError GetDataSegs(vector<VFileDesc*>& outSegs, Boolean inWithSpecialSegs = true) const;
	virtual VError GetStructSegs(vector<VFileDesc*>& outSegs, Boolean inWithSpecialSegs = true) const;
	virtual	XBOX::VFolder* RetainBlobsFolder() const;

	virtual XBOX::VFolder* RetainTemporaryFolder( bool inMustExists) const;
	virtual VErrorDB4D SetTemporaryFolderSelector( DB4DFolderSelector inSelector, const XBOX::VString *inCustomFolder = nil);
	virtual VErrorDB4D GetTemporaryFolderSelector( DB4DFolderSelector *outSelector, XBOX::VString& outCustomFolderPath) const;

	virtual VErrorDB4D PrepareForBackup(CDB4DBaseContextPtr inContext);
	virtual VErrorDB4D EndBackup(CDB4DBaseContextPtr inContext, VFile* oldJournal);

	virtual VIntlMgr* GetIntlMgr() const;

	virtual VErrorDB4D SetIntlMgr(VIntlMgr* inIntlMgr, VDB4DProgressIndicator* InProgress = NULL);

	virtual CDB4DQueryResult* RelateOneSelection(sLONG TargetOneTable, VErrorDB4D& err, CDB4DBaseContextPtr inContext, CDB4DQueryOptions* inOptions,
													VDB4DProgressIndicator* InProgress = nil, std::vector<CDB4DRelation*> *inPath = nil);

	virtual CDB4DQueryResult* RelateManySelection(CDB4DField* inRelationStart, VErrorDB4D& err, CDB4DBaseContextPtr inContext, 
													CDB4DQueryOptions* inOptions, VDB4DProgressIndicator* InProgress = nil);


	// client server

	virtual CDB4DRecord* BuildRecordFromServer(VStream* from, CDB4DBaseContext* inContext, CDB4DTable* inTable, VErrorDB4D& outError);
	virtual CDB4DRecord* BuildRecordFromClient(VStream* from, CDB4DBaseContext* inContext, CDB4DTable* inTable, VErrorDB4D& outError);

	virtual CDB4DSelection* BuildSelectionFromServer(VStream* from, CDB4DBaseContext* inContext, CDB4DTable* inTable, VErrorDB4D& outError);
	virtual CDB4DSelection* BuildSelectionFromClient(VStream* from, CDB4DBaseContext* inContext, CDB4DTable* inTable, VErrorDB4D& outError);

	virtual CDB4DSet* BuildSetFromServer(VStream* from, CDB4DBaseContext* inContext, CDB4DTable* inTable, VErrorDB4D& outError);
	virtual CDB4DSet* BuildSetFromClient(VStream* from, CDB4DBaseContext* inContext, CDB4DTable* inTable, VErrorDB4D& outError);

	virtual CDB4DQueryOptions* NewQueryOptions() const;

	virtual void Flush(Boolean WaitUntilDone = false, bool inEmptyCacheMem = false, ECacheLogEntryKind inOrigin = eCLEntryKind_FlushFromUnknown);

	virtual void SyncThingsToForget(CDB4DBaseContext* inContext);

	virtual Boolean StillIndexing() const;

	virtual DB4D_SchemaID CreateSchema(const VString& inSchemaName, CDB4DBaseContext* inContext, VErrorDB4D& outError);

	virtual sLONG CountSchemas() const;

	virtual VErrorDB4D RetainAllSchemas(std::vector<VRefPtr<CDB4DSchema> >& outSchemas, CDB4DBaseContext* inContext);

	virtual CDB4DSchema* RetainSchema(const VString& inSchemaName, CDB4DBaseContext* inContext);

	virtual CDB4DSchema* RetainSchema(DB4D_SchemaID inSchemaID, CDB4DBaseContext* inContext);

	virtual void SetClientID(const XBOX::VUUID& inID);

	virtual VErrorDB4D ExportToSQL(CDB4DBaseContext* inContext, VFolder* inFolder, VDB4DProgressIndicator* inProgress, ExportOption& options);

	virtual VErrorDB4D ImportRecords(CDB4DBaseContext* inContext, VFolder* inFolder, VDB4DProgressIndicator* inProgress, ExportOption& options);

	virtual VErrorDB4D ExecuteRestRequest(tempDB4DWebRequest& inRequest); // temporaire L.R le 4 dec 2008
	
	virtual sLONG CountEntityModels(CDB4DBaseContext* context = NULL, bool onlyRealOnes = true) const;
	
	virtual VErrorDB4D RetainAllEntityModels(vector<XBOX::VRefPtr<CDB4DEntityModel> >& outList, CDB4DBaseContext* context = NULL, bool onlyRealOnes = true) const;
	
	virtual CDB4DEntityModel* RetainEntityModel(const VString& entityName, bool onlyRealOnes = true) const;

	virtual CDB4DEntityModel* RetainEntityModelByCollectionName(const VString& entityName) const;

	virtual VErrorDB4D SetIdleTimeOut(uLONG inMilliseconds);

	virtual void DisposeRecoverInfo(void* inRecoverInfo);

	virtual VError ScanToRecover(VFile* inOldDataFile, VValueBag& outBag, void* &outRecoverInfo, IDB4D_DataToolsIntf* inDataToolLog = NULL);

	virtual VError RecoverByTags(VValueBag& inBag, void* inRecoverInfo, IDB4D_DataToolsIntf* inDataToolLog = NULL);

	virtual CDB4DBase* RetainSyncDataBase();

	virtual VError SaveToBag(VValueBag& outBag);

	IHTTPRequestHandler *			AddRestRequestHandler( VErrorDB4D& outError, IHTTPServerProject* inHTTPServerProject, RIApplicationRef inApplicationRef, const VString& inPattern, bool inEnabled);
	virtual VErrorDB4D				SetRestRequestHandlerPattern( IHTTPRequestHandler* inHandler, const VString& inPattern);

	virtual VError ReLoadEntityModels(VFile* inFile);

	virtual VError GetListOfDeadTables(vector<VString>& outTableNames, vector<VUUID>& outTableIDs, CDB4DBaseContext* inContext);
	virtual VError ResurectDataTable(const VString& inTableName, CDB4DBaseContext* inContext);

	VFolder* RetainDataFolder();
	VFolder* RetainStructFolder();

	virtual VErrorDB4D CompactInto(VFile* destData, IDB4D_DataToolsIntf* inDataToolLog, bool KeepRecordNumbers);

	virtual	VJSObject CreateJSDatabaseObject( const VJSContext& inContext);

	virtual bool CatalogJSParsingError(VFile* &outRetainedFile, VString& outMessage, sLONG& outLineNumber);


	//========== end of component methods ===============

	VDBMgr *GetManager() const {return fManager;}
	Base4D* GetBase() const {return fBase;}
	//Base4DRemote* GetBaseRemote() const {return fBaseRemote;}


private:
	VDBMgr	*fManager;
	Base4D*	fBase;
	VCriticalSection fSyncMutex;
	//Base4DRemote* fBaseRemote;
	Boolean fMustRetain;
	Boolean fIsRemote;
};



class VDB4DSchema : public VComponentImp<CDB4DSchema>, public IBaggable
{
	public:
		VDB4DSchema(Base4D* owner, const VString& inName, sLONG inID)
		{
			fOwner = owner;
			fID = inID;
			fName = inName;
			fStorage = nil;
		}

		virtual const VString& GetName() const;
		virtual DB4D_SchemaID GetID() const;

		virtual VValueBag* RetainAndLockProperties(VErrorDB4D& outError, CDB4DBaseContext* inContext);
		virtual void SaveProperties(CDB4DBaseContext* inContext);
		virtual void UnlockProperties(CDB4DBaseContext* inContext);

		virtual VErrorDB4D Drop(CDB4DBaseContext* inContext);
		virtual VErrorDB4D Rename(const VString& inNewName, CDB4DBaseContext* inContext);

		// from IBaggable
		virtual VError	LoadFromBagWithLoader( const VValueBag& inBag, VBagLoader *inLoader, void* inContext = nil);
		virtual VError	SaveToBag( VValueBag& ioBag, VString& outKind) const;

		inline void SetName(const VString& inNewName)
		{
			fName = inNewName;
		}

	private:
		Base4D* fOwner;
		VString fName;
		DB4D_SchemaID fID;
		FicheInMem* fStorage;
};


/*
class VDB4DBaseContext : public VComponentImp<CDB4DBaseContext>
{
public:

	// the base must have already been retained
	VDB4DBaseContext( VDBMgr *inManager, CDB4DBase *inBase, bool islocal = false);
	//VDB4DBaseContext( VDBMgr *inManager, CDB4DBase *inBase, Boolean RemoteParam);

	virtual ~VDB4DBaseContext();

	virtual void DescribeQueryExecution(Boolean on);
	virtual void GetLastQueryDescription(XBOX::VString& outResult, DB4D_QueryDescriptionFormat inFormat);
	virtual void GetLastQueryExecution(XBOX::VString& outResult, DB4D_QueryDescriptionFormat inFormat);

	virtual void SetTimerOnRecordLocking(sLONG WaitForLockTimer);
	virtual sLONG GetTimerOnRecordLocking() const;

	virtual CDB4DBase* GetOwner(void) const;
	virtual Boolean MatchBaseInContext(CDB4DBaseContextPtr InBaseContext) const; 
	virtual CDB4DBaseContextPtr Spawn(Boolean isCopied = false) const; 

	virtual VError StartTransaction(sLONG WaitForLockTimer = 0);

	virtual VError CommitTransaction();
	virtual VError RollBackTransaction();
	virtual sLONG CurrentTransactionLevel() const;

	virtual VError ReleaseFromConsistencySet(CDB4DSelection* InSel);
	virtual VError ReleaseFromConsistencySet(CDB4DSet* InSet);
	virtual VError ReleaseFromConsistencySet(RecIDType inRecordID);
	virtual VError ReleaseFromConsistencySet(CDB4DRecord* inRec);
	virtual VError SetConsistency(Boolean isOn);

	//virtual CDB4DTableContext *NewTableContext( DB4D_TableID inTableID);
	virtual CDB4DSqlQuery *NewSqlQuery(VString& request, VError& err);

	virtual VError SetRelationAutoLoadNto1(const CDB4DRelation* inRel, DB4D_Rel_AutoLoadState inAutoLoadState);
	virtual VError SetRelationAutoLoad1toN(const CDB4DRelation* inRel, DB4D_Rel_AutoLoadState inAutoLoadState);
//	virtual VError SetRelationAutoLoad(sLONG inTableSourceID, sLONG inFieldSourceID, sLONG inTableDestID, sLONG inFieldDestID , DB4D_Rel_AutoLoadState inAutoLoadState);
//	virtual VError SetRelationAutoLoad(const CDB4DField* inSource, const CDB4DField* inDest, DB4D_Rel_AutoLoadState inAutoLoadState);

	virtual DB4D_Rel_AutoLoadState GetRelationAutoLoadNto1State(const CDB4DRelation* inRel) const;
	virtual DB4D_Rel_AutoLoadState GetRelationAutoLoad1toNState(const CDB4DRelation* inRel) const;
//	virtual DB4D_Rel_AutoLoadState GetRelationAutoLoadState(sLONG inTableSourceID, sLONG inFieldSourceID, sLONG inTableDestID, sLONG inFieldDestID) const;
//	virtual DB4D_Rel_AutoLoadState GetRelationAutoLoadState(const CDB4DField* inSource, const CDB4DField* inDest) const;

	virtual Boolean IsRelationAutoLoadNto1(const CDB4DRelation* inRel) const;
	virtual Boolean IsRelationAutoLoad1toN(const CDB4DRelation* inRel) const;

	virtual void ExcludeTableFromAutoRelationDestination(CDB4DTable* inTableToExclude);
	virtual void IncludeBackTableToAutoRelationDestination(CDB4DTable* inTableToInclude);
	virtual Boolean IsTableExcludedFromAutoRelationDestination(CDB4DTable* inTableToCheck) const;

	virtual void SetAllRelationsToAutomatic(Boolean RelationsNto1, Boolean ForceAuto);
	virtual Boolean IsAllRelationsToAutomatic(Boolean RelationsNto1);

	virtual void SetExtraData(const VValueBag* inExtra);
	virtual const VValueBag* RetainExtraData() const;

	virtual LockPtr LockDataBaseDef(const CDB4DBase* inBase, sLONG inTimeOut = 0, Boolean inForReadOnly = false, const VValueBag **outLockerExtraData = NULL);
	virtual LockPtr LockTableDef(const CDB4DTable* inTable, Boolean inWithFields, sLONG inTimeOut = 0, Boolean inForReadOnly = false, const VValueBag **outLockerExtraData = NULL);
	virtual LockPtr LockFieldDef(const CDB4DField* inField, sLONG inTimeOut = 0, Boolean inForReadOnly = false, const VValueBag **outLockerExtraData = NULL);
	virtual LockPtr LockRelationDef(const CDB4DRelation* inRelation, Boolean inWithRelatedFields, sLONG inTimeOut = 0, Boolean inForReadOnly = false, const VValueBag **outLockerExtraData = NULL);
	virtual LockPtr LockIndexDef(const CDB4DIndex* inIndex, sLONG inTimeOut = 0, Boolean inForReadOnly = false, const VValueBag **outLockerExtraData = NULL);

	virtual Boolean LockMultipleTables(const vector<const CDB4DTable*>& inTables, vector<LockPtr>& outLocks, sLONG inTimeOut = 0, Boolean inForReadOnly = false);
	virtual void UnLockMultipleTables(const vector<LockPtr>& inLocks);

	virtual VErrorDB4D UnLockStructObject(LockPtr inLocker);

	virtual void SetClientRequestStreams(VStream* OutputStream, VStream* InputStream);
	virtual void SetServerRequestStreams(VStream* InputStream, VStream* OutputStream);

	virtual Boolean IsRemote() const { return fIsRemote; };

	virtual CDB4DContext* GetContextOwner() const;

	virtual void SendlastRemoteInfo();

	virtual CDB4DRemoteRecordCache* StartCachingRemoteRecords(CDB4DSelection* inSel, RecIDType FromRecIndex, RecIDType ToRecIndex,  CDB4DFieldCacheCollection* inWhichFields, const vector<uBYTE>& inWayOfLocking);
	virtual void StopCachingRemoteRecords(CDB4DRemoteRecordCache* inCacheInfo);

	virtual VErrorDB4D SetIdleTimeOut(uLONG inMilliseconds);

#if debuglr
	virtual CComponent*	Retain (const char* DebugInfo = 0);
	virtual void		Release (const char* DebugInfo = 0);
#endif

	//========== end of component methods ===============

	VDBMgr *GetManager() const {return fManager;}
	BaseTaskInfo *GetBaseTaskInfo() {return &fBase;}
	

private:
	VDBMgr	*fManager;
	BaseTaskInfo	fBase;
	CDB4DBase* fOwner;
	Boolean fIsRemote;
};

*/

class VDB4DFieldCacheCollection : public VComponentImp<CDB4DFieldCacheCollection>
{
	public:
		inline VDB4DFieldCacheCollection(Table* inTable):fFieldsCache(inTable)
		{
		}

		virtual VErrorDB4D AddField(CDB4DField* inField);

		virtual VErrorDB4D WriteToStream(VStream* ToStream);
		virtual VErrorDB4D ReadFromStream(VStream* FromStream);

		inline FieldsForCache* GetFieldsCache()
		{
			return &fFieldsCache;
		}

	private:
		FieldsForCache fFieldsCache;
};



class VDB4DRemoteRecordCache : public VComponentImp<CDB4DRemoteRecordCache>
{
	public:
		
		inline VDB4DRemoteRecordCache(CDB4DFieldCacheCollection* inFields, CDB4DBaseContext* inContext):fRecordsCache(VImpCreator<VDB4DFieldCacheCollection>::GetImpObject(inFields)->GetFieldsCache(), ConvertContext(inContext))
		{
			fFields = RetainRefCountable(inFields);
		}

		virtual ~VDB4DRemoteRecordCache()
		{
			QuickReleaseRefCountable(fFields);
		}

		virtual VErrorDB4D RetainCacheRecord(RecIDType inRecIndex, CDB4DRecord* &outRecord, vector<CachedRelatedRecord>& outRelatedRecords);

		inline RemoteRecordCache* GetRemoteRecordCache()
		{
			return &fRecordsCache;
		}

	private:
		RemoteRecordCache fRecordsCache;
		CDB4DFieldCacheCollection* fFields;
};





class VDB4DField : public VComponentImp<CDB4DField>
{
public:
	VDB4DField(Field* inField);
	virtual ~VDB4DField();

	virtual CDB4DTable* GetOwner(CDB4DBaseContextPtr inContext = NULL);
	virtual DB4D_FieldID GetID(CDB4DBaseContextPtr inContext = NULL) const;
	virtual void GetUUID(VUUID& outID, CDB4DBaseContextPtr inContext = NULL) const;

	virtual void GetName( VString& outName, CDB4DBaseContextPtr inContext = NULL) const;
	virtual VError SetName( const VString& inName, CDB4DBaseContextPtr inContext = NULL);

	virtual DB4DFieldType GetType(CDB4DBaseContextPtr inContext = NULL) const;
	virtual DB4DFieldType GetRealType(CDB4DBaseContextPtr inContext = NULL) const;
	virtual VError SetType(DB4DFieldType inType, VDB4DProgressIndicator* InProgress = nil, CDB4DBaseContextPtr inContext = NULL);

	virtual sLONG GetTextSwitchSize(CDB4DBaseContextPtr inContext = NULL) const;
	virtual VErrorDB4D SetTextSwitchSize(sLONG inLength, CDB4DBaseContextPtr inContext = NULL);

	virtual sLONG GetBlobSwitchSize(CDB4DBaseContextPtr inContext = NULL) const;
	virtual VErrorDB4D SetBlobSwitchSize(sLONG inLength, CDB4DBaseContextPtr inContext = NULL);

	virtual sLONG GetLimitingLength(CDB4DBaseContextPtr inContext = NULL) const;
	virtual VErrorDB4D SetLimitingLength(sLONG inLength, CDB4DBaseContextPtr inContext = NULL);

	virtual Boolean IsUnique(CDB4DBaseContextPtr inContext = NULL) const;
	virtual VError SetUnique(Boolean unique, VDB4DProgressIndicator* InProgress = nil, CDB4DBaseContextPtr inContext = NULL);

	virtual Boolean IsNot_Null(CDB4DBaseContextPtr inContext = NULL) const;
	virtual VError SetNot_Null(Boolean mandatory, VDB4DProgressIndicator* InProgress = nil, CDB4DBaseContextPtr inContext = NULL);

	virtual Boolean IsAutoSequence(CDB4DBaseContextPtr inContext = NULL) const;
	virtual VErrorDB4D SetAutoSequence(Boolean autoseq, CDB4DBaseContextPtr inContext = NULL);

	virtual Boolean IsAutoGenerate(CDB4DBaseContextPtr inContext = NULL) const;
	virtual VErrorDB4D SetAutoGenerate(Boolean autogenerate, CDB4DBaseContextPtr inContext = NULL);

	virtual Boolean IsStoreAsUTF8(CDB4DBaseContextPtr inContext = NULL) const;
	virtual VErrorDB4D SetStoreAsUTF8(Boolean storeUTF8, CDB4DBaseContextPtr inContext = NULL);
	
	virtual Boolean IsStoreAsUUID(CDB4DBaseContextPtr inContext = NULL) const;
	virtual VErrorDB4D SetStoreAsUUID(Boolean storeUUID, CDB4DBaseContextPtr inContext = NULL);

	virtual Boolean IsStoreOutside(CDB4DBaseContextPtr inContext = NULL) const;
	virtual VErrorDB4D SetStoreOutside(Boolean storeOutside, VDB4DProgressIndicator* InProgress = nil, CDB4DBaseContextPtr inContext = NULL);

	virtual Boolean IsStyledText(CDB4DBaseContextPtr inContext = NULL) const;
	virtual VErrorDB4D SetStyledText(Boolean styledText, VDB4DProgressIndicator* InProgress = nil, CDB4DBaseContextPtr inContext = NULL);

	virtual VError SetDefinition(const VValueBag& inFieldDefinition, VDB4DProgressIndicator* InProgress = nil, CDB4DBaseContextPtr inContext = NULL);
	virtual VValueBag* CreateDefinition(CDB4DBaseContextPtr inContext = NULL) const;

	virtual Boolean IsFullTextIndexable(CDB4DBaseContextPtr inContext = NULL) const;
	virtual Boolean IsFullTextIndexed(CDB4DBaseContextPtr inContext = NULL) const;
	virtual Boolean IsIndexable(CDB4DBaseContextPtr inContext = NULL) const;
	virtual Boolean IsIndexed(CDB4DBaseContextPtr inContext = NULL) const;
	virtual Boolean IsPrimIndexed(CDB4DBaseContextPtr inContext = NULL) const;
	virtual Boolean	IsPrimaryKeyCompatible( CDB4DBaseContextPtr inContext = NULL) const;

	virtual Boolean CanBePartOfRelation(CDB4DBaseContextPtr inContext = NULL) const;

	virtual Boolean Drop(VDB4DProgressIndicator* InProgress = nil, CDB4DBaseContextPtr inContext = NULL);

	virtual const VValueBag* RetainExtraProperties(VError &err, CDB4DBaseContextPtr inContext = NULL);
	virtual VError SetExtraProperties(VValueBag* inExtraProperties, CDB4DBaseContextPtr inContext = NULL);

	virtual CDB4DRelation* RetainFirstNto1Relation(CDB4DBaseContextPtr inContext = NULL);

	virtual CDB4DRelation* RetainRelation_SubTable(CDB4DBaseContextPtr inContext = NULL);
	//virtual CDB4DRelation* RetainRelation_SourceLienAller_V6(Boolean AutoRelOnly, CDB4DBaseContextPtr inContext);
	//virtual CDB4DRelation* RetainRelation_CibleLienRetour_V6(Boolean AutoRelOnly, CDB4DBaseContextPtr inContext);
	virtual VErrorDB4D GetListOfRelations1_To_N(RelationArray& outRelations, Boolean AutoRelOnly, CDB4DBaseContextPtr inContext);
	virtual VErrorDB4D GetListOfRelationsN_To_1(RelationArray& outRelations, Boolean AutoRelOnly, CDB4DBaseContextPtr inContext);

	virtual CDB4DIndex* RetainIndex(Boolean MustBeSortable = false, CDB4DBaseContextPtr inContext = NULL, Boolean MustBeValid = true, Boolean Occupy = false) const;
	virtual CDB4DIndex* RetainFullTextIndex(CDB4DBaseContextPtr inContext = NULL, Boolean MustBeValid = true, Boolean Occupy = false) const;

	virtual Boolean IsNeverNull(CDB4DBaseContextPtr inContext = NULL) const;
	virtual VError SetNeverNullState(Boolean inState, CDB4DBaseContextPtr inContext = NULL);

	virtual sLONG GetStamp(CDB4DBaseContextPtr inContext = NULL) const;

	virtual VError RetainIndexes(ArrayOf_CDB4DIndex& outIndexes, CDB4DBaseContextPtr inContext = NULL);

	//========== end of component methods ===============

	Field* GetField() const { return fCrit; };
	VErrorDB4D GetListOfRelations(RelationArray& outRelations, Boolean AutoRelOnly, Boolean x_Nto1, CDB4DBaseContextPtr inContext);

private:
	Field* fCrit;
	CDB4DTable* fOwner;

};


class VDB4DTable : public VComponentImp<CDB4DTable>
{
public:

	// the base must have already been retained
	VDB4DTable( VDBMgr *inManager, VDB4DBase *inBase, Table *inTable);

	virtual ~VDB4DTable();

	virtual CDB4DBase* GetOwner(CDB4DBaseContextPtr inContext = NULL);

	virtual void GetName( VString& outName, CDB4DBaseContextPtr inContext = NULL) const;
	virtual VError SetName( const VString& inName, CDB4DBaseContextPtr inContext = NULL);
	virtual	VErrorDB4D SetNameNoNameCheck( const XBOX::VString& inName, CDB4DBaseContextPtr inContext = NULL);
	virtual VError SetRecordName(const VString& inName, CDB4DBaseContextPtr inContext = NULL);
	virtual void GetRecordName( VString& outName, CDB4DBaseContextPtr inContext = NULL) const;

	virtual VErrorDB4D SetSchemaID(DB4D_SchemaID inSchemaID, CDB4DBaseContext* inContext = NULL);
	virtual DB4D_SchemaID GetSchemaID(CDB4DBaseContext* inContext = NULL) const;

	virtual VError SetFullyDeleteRecords(Boolean FullyDeleteState, CDB4DBaseContextPtr inContext = NULL);
	virtual Boolean GetFullyDeleteRecordsState() const;

	virtual DB4D_TableID GetID(CDB4DBaseContextPtr inContext = NULL) const;
	virtual void GetUUID(VUUID& outID, CDB4DBaseContextPtr inContext = NULL) const;
	virtual VValueBag *CreateDefinition(CDB4DBaseContextPtr inContext = NULL) const;

	virtual sLONG CountFields(CDB4DBaseContextPtr inContext = NULL) const;

	virtual CDB4DField* RetainNthField(sLONG inIndex, CDB4DBaseContextPtr inContext = NULL) const;
	virtual CDB4DField* FindAndRetainField(const VString& inName, CDB4DBaseContextPtr inContext = NULL) const;
	virtual CDB4DField* FindAndRetainField(const VUUID& inUUID, CDB4DBaseContextPtr inContext = NULL) const;
	virtual DB4D_FieldID GetFieldByName( const VString& inName, CDB4DBaseContextPtr inContext = NULL) const;

	virtual VError AddField(const XBOX::VString& inName, DB4DFieldType inType, sLONG inSize, DB4DFieldAttributes inAttributes, 
							VDB4DProgressIndicator* InProgress = NULL, CDB4DBaseContextPtr inContext = NULL);

	virtual sLONG AddFields(const VValueBag& inFieldsDefinition, VError &err, VDB4DProgressIndicator* InProgress = NULL, CDB4DBaseContextPtr inContext = NULL);

	virtual VError SetFieldDefinition(DB4D_FieldID inFieldID, const VValueBag& inFieldDefinition, 
										VDB4DProgressIndicator* InProgress = nil, CDB4DBaseContextPtr inContext = NULL);

	virtual VError SetFieldDefinition( DB4D_FieldID inFieldID, VString& inName, DB4DFieldType inType, sLONG inSize, 
						DB4DFieldAttributes inAttributes, VDB4DProgressIndicator* InProgress = nil, CDB4DBaseContextPtr inContext = NULL);

	virtual void GetFieldDefinition( DB4D_FieldID inFieldID, VString& outName, DB4DFieldType *outType, sLONG *outSize, 
						DB4DFieldAttributes *outAttributes, CDB4DBaseContextPtr inContext = NULL) const;

	virtual Boolean DropField(DB4D_FieldID inFieldID, VDB4DProgressIndicator* InProgress = nil, CDB4DBaseContextPtr inContext = NULL);

	virtual VError SetIdentifyingFields(const CDB4DFieldArray& inFields, VDB4DProgressIndicator* InProgress = NULL, CDB4DBaseContextPtr inContext = NULL);

	virtual VError SetPrimaryKey(const CDB4DFieldArray& inFields, VDB4DProgressIndicator* InProgress = NULL, 
									Boolean CanReplaceExistingOne = false, CDB4DBaseContextPtr inContext = NULL, VString* inPrimaryKeyName = NULL);

	virtual VError RetainPrimaryKey(CDB4DFieldArray& outFields, CDB4DBaseContextPtr inContext = NULL) const;

	virtual	bool HasPrimaryKey( CDB4DBaseContextPtr inContext = NULL) const;

	virtual Boolean IsIndexable(DB4D_FieldID inFieldID, CDB4DBaseContextPtr inContext = NULL);
	virtual Boolean IsIndexed(DB4D_FieldID inFieldID, CDB4DBaseContextPtr inContext = NULL);
	virtual Boolean IsPrimIndexed(DB4D_FieldID inFieldID, CDB4DBaseContextPtr inContext = NULL);


	virtual Boolean Drop(VDB4DProgressIndicator* InProgress = nil, CDB4DBaseContextPtr inContext = NULL);


	virtual CDB4DQuery *NewQuery();
	virtual CDB4DQueryResult *NewQueryResult( CDB4DSelection *inSelection, CDB4DSet *inSet, CDB4DSet *inLockedSet, CDB4DRecord *inFirstRecord);
	virtual CDB4DSelectionPtr ExecuteQuery( CDB4DQuery *inQuery, CDB4DBaseContextPtr inContext, CDB4DSelectionPtr Filter = nil, 
											VDB4DProgressIndicator* InProgress = nil, DB4D_Way_of_Locking HowToLock = DB4D_Do_Not_Lock, 
											sLONG limit = 0, CDB4DSet* outLockSet = NULL, VErrorDB4D *outErr = NULL);

	virtual CDB4DQueryResult* ExecuteQuery(CDB4DQuery* inQuery, CDB4DBaseContext* inContext, CDB4DQueryOptions* inOptions, VDB4DProgressIndicator* inProgress, VError& outError);

	virtual CDB4DSelectionPtr SelectAllRecords(CDB4DBaseContextPtr inContext, VErrorDB4D* outErr = NULL, 
												DB4D_Way_of_Locking HowToLock = DB4D_Do_Not_Lock, CDB4DSet* outLockSet = NULL);

	virtual	CDB4DSelectionPtr	SelectAllRecordsAndLoadFirstOne( CDB4DBaseContextPtr inContext, VErrorDB4D* outErr, bool inLoadRecordReadOnly, CDB4DRecord **outFirstRecord);

	virtual RecIDType CountRecordsInTable(CDB4DBaseContextPtr inContext) const;
	virtual RecIDType GetMaxRecordsInTable(CDB4DBaseContextPtr inContext) const;
	//virtual sLONG8 GetSequenceNumber(CDB4DBaseContextPtr inContext);


	virtual CDB4DRecord *NewRecord(CDB4DBaseContextPtr inContext);
	virtual CDB4DRecord *LoadRecord( RecIDType inRecordID, DB4D_Way_of_Locking HowToLock, CDB4DBaseContextPtr inContext, Boolean WithSubTable = true, Boolean* outLockWasKeptInTrans = NULL);
	virtual VErrorDB4D DeleteRecord( RecIDType inRecordID, CDB4DBaseContextPtr inContext);

	virtual void UnlockRecordAfterRelease_IfNotModified(RecIDType inRecordID, CDB4DBaseContextPtr inContext);
	virtual void UnlockRecordsAfterRelease_IfNotModified(const vector<RecIDType> &inRecordIDs, CDB4DBaseContextPtr inContext);

	virtual void* LoadRawRecord(RecIDType inRecordID, DB4D_Way_of_Locking HowToLock, CDB4DBaseContextPtr inContext, 
								VErrorDB4D& outErr, Boolean* outCouldLock = NULL, Boolean* outLockWasKeptInTrans = NULL);

	virtual CDB4DImpExp* NewImpExp();

	virtual CDB4DSortingCriterias* NewSortingCriterias();


	virtual Boolean LockTable(CDB4DBaseContextPtr inContext, Boolean WaitForEndOfRecordLocks = false, sLONG TimeToWaitForEndOfRecordLocks = 0);
	virtual void UnLockTable(CDB4DBaseContextPtr inContext);


	virtual CDB4DSet* NewSet() const;
	virtual CDB4DSelectionPtr NewSelection(DB4D_SelectionType inSelectionType) const;

	virtual CDB4DColumnFormula* NewColumnFormula() const;

	virtual const VValueBag* RetainExtraProperties(VError &err, CDB4DBaseContextPtr inContext = NULL);
	virtual VError SetExtraProperties(VValueBag* inExtraProperties, CDB4DBaseContextPtr inContext = NULL);

	virtual CDB4DAutoSeqNumber* RetainAutoSeqNumber(CDB4DBaseContextPtr inContext = NULL);

	virtual VErrorDB4D GetListOfRelations1_To_N(RelationArray& outRelations, Boolean AutoRelOnly, CDB4DBaseContextPtr inContext);
	virtual VErrorDB4D GetListOfRelationsN_To_1(RelationArray& outRelations, Boolean AutoRelOnly, CDB4DBaseContextPtr inContext);

	virtual VErrorDB4D DelayIndexes();
	virtual VErrorDB4D AwakeIndexes(vector<CDB4DIndex*>& outIndexWithProblems, VDB4DProgressIndicator* inProgress = NULL, Boolean WaitForCompletion = true);

	virtual sLONG GetStamp(CDB4DBaseContextPtr inContext = NULL) const;
	virtual void GetFieldsStamps(StampsVector& outStamps, CDB4DBaseContextPtr inContext = NULL) const;

	virtual VError RetainIndexes(ArrayOf_CDB4DIndex& outIndexes, CDB4DBaseContextPtr inContext = NULL);

	virtual VError RetainExistingFields(vector<CDB4DField*>& outFields);
	virtual sLONG CountExistingFields();

	virtual CDB4DFieldCacheCollection* NewFieldCacheCollection(const VString& Signature) const;

	virtual VError Truncate(CDB4DBaseContextPtr inContext = NULL, VDB4DProgressIndicator* inProgress = NULL);

	virtual VError ActivateAutomaticRelations_N_To_1(CDB4DRecord* inRecord, vector<CachedRelatedRecord>& outResult, CDB4DBaseContext* InContext, const vector<uBYTE>& inWayOfLocking);

	virtual VError ActivateAutomaticRelations_1_To_N(CDB4DRecord* inRecord, vector<CachedRelatedRecord>& outResult, CDB4DBaseContext* InContext, 
														const vector<uBYTE>& inWayOfLocking, CDB4DField* onOneFieldOnly = nil, Boolean onOldvalue = false, 
														Boolean AutomaticOnly = false, Boolean OneLevelOnly = true);

	virtual VErrorDB4D ActivateAllAutomaticRelations(CDB4DRecord* inRecord, std::vector<CachedRelatedRecord>& outResult, 
														CDB4DBaseContext* InContext, const std::vector<uBYTE>& inWayOfLocking);
	// inWayOfLocking starts with Table# 1 beeing element 0 of the vector

	virtual VErrorDB4D DataToCollection(const vector<RecIDType>& inRecIDs, DB4DCollectionManager& Collection, CDB4DBaseContextPtr inContext, sLONG FromRecInSel, sLONG ToRecInSel,
											VDB4DProgressIndicator* InProgress = nil);

	virtual VErrorDB4D SetKeepRecordStamps(bool inKeepRecordStamps, CDB4DBaseContextPtr inContext, VDB4DProgressIndicator* InProgress = nil);

	virtual bool GetKeepRecordStamps() const;

#if AllowSyncOnRecID
	virtual VError GetModificationsSinceStamp(uLONG stamp, VStream& outStream, uLONG& outLastStamp, sLONG& outNbRows, CDB4DBaseContextPtr inContext, vector<sLONG>& cols);

	virtual VError IntegrateModifications(VStream& inStream, CDB4DBaseContextPtr inContext, vector<sLONG>& cols);

	virtual VErrorDB4D GetOneRow(VStream& inStream, CDB4DBaseContextPtr inContext, sBYTE& outAction, uLONG& outStamp, RecIDType& outRecID, vector<VValueSingle*>& outValues);
#endif

	virtual CDB4DEntityModel* BuildEntityModel();

	virtual VErrorDB4D SetKeepRecordSyncInfo(bool inKeepRecordSyncInfo, CDB4DBaseContextPtr inContext, VDB4DProgressIndicator* InProgress = nil);

	virtual bool GetKeepRecordSyncInfo() const;

	virtual VErrorDB4D GetModificationsSinceStampWithPrimKey(uLONG8 stamp, VStream& outStream, uLONG8& outLastStamp, sLONG& outNbRows, 
																CDB4DBaseContextPtr inContext, vector<sLONG>& cols,
																CDB4DSelection* filter, sLONG8 skip, sLONG8 top,
																std::vector<XBOX::VString*>* inImageFormats = 0);

	virtual VErrorDB4D IntegrateModificationsWithPrimKey(VStream& inStream, CDB4DBaseContextPtr inContext, vector<sLONG>& cols, bool sourceOverDest,
															uLONG8& ioFirstDestStampToCheck, uLONG8& outErrorStamp, bool inBinary = true);

	virtual VErrorDB4D GetOneRowWithPrimKey(VStream& inStream, CDB4DBaseContextPtr inContext, sBYTE& outAction, uLONG8& outStamp, VTime& outTimeStamp, 
												vector<VValueSingle*>& outPrimKey, vector<VValueSingle*>& outValues);

	virtual void WhoLockedRecord( RecIDType inRecordID, CDB4DBaseContextPtr inContext, DB4D_KindOfLock& outLockType, const VValueBag **outLockingContextRetainedExtraData) const;

	virtual VError GetFragmentation(sLONG8& outTotalRec, sLONG8& outFrags, CDB4DBaseContext* inContext);

	virtual VErrorDB4D ImportRecords(CDB4DBaseContext* inContext, VFolder* inFolder, VDB4DProgressIndicator* inProgress, ExportOption& options);

	virtual CDB4DField* RetainPseudoField(DB4D_Pseudo_Field_Kind kind);

	virtual VErrorDB4D GetListOfTablesForCascadingDelete(std::set<sLONG>& outSet);

	virtual sLONG GetSeqRatioCorrector() const; 

	virtual void SetSeqRatioCorrector(sLONG newValue);


	//========== end of component methods ===============

	VDBMgr *GetManager() const {return fManager;}
	//VDB4DBaseContext *xGetBaseContext() const {return fBase;}
	Table* GetTable() const {return fTable;}
	VDB4DBase*	GetBase() const	{ return fBase;}

	FicheInMem *NewFicheInMem(BaseTaskInfoPtr context);
	FicheInMem *LoadFicheInMem( RecIDType inRecordID, DB4D_Way_of_Locking HowToLock, BaseTaskInfoPtr context, Boolean WithSubTable, Boolean* outLockWasKeptInTrans);
	
	VErrorDB4D GetListOfRelations(RelationArray& outRelations, Boolean AutoRelOnly, Boolean x_Nto1, CDB4DBaseContextPtr inContext);


private:
	VDBMgr	*fManager;
	Table *fTable;
	VDB4DBase*	fBase;
};



class VDB4DSelection : public VComponentImp<CDB4DSelection>
{
public:

	// the base must have already been retained
	VDB4DSelection( VDBMgr *inManager, VDB4DBase *inBase, Table *inTable, Selection* inSel, CDB4DEntityModel* inModel = nil);

	virtual ~VDB4DSelection();

	virtual DB4D_TableID GetTableRef() const;
	virtual CDB4DBase* GetBaseRef() const;

	virtual void* LoadRawRecord(RecIDType inRecordIndex, DB4D_Way_of_Locking HowToLock, CDB4DBaseContextPtr inContext, 
								VErrorDB4D& outErr, Boolean* outCouldLock = NULL, Boolean* outLockWasKeptInTrans = NULL);

	virtual RecIDType CountRecordsInSelection(CDB4DBaseContextPtr inContext) const;
	virtual Boolean IsEmpty(CDB4DBaseContextPtr inContext) const;
	virtual CDB4DRecord *LoadSelectedRecord( RecIDType inRecordIndex, DB4D_Way_of_Locking HowToLock, CDB4DBaseContextPtr inContext, Boolean WithSubTable = true, Boolean* outLockWasKeptInTrans = NULL);
	virtual RecIDType GetSelectedRecordID( RecIDType inRecordIndex, CDB4DBaseContextPtr inContext);

	virtual Boolean SortSelection(DB4D_FieldID inFieldID, Boolean inAscending, VDB4DProgressIndicator* InProgress = nil, CDB4DBaseContextPtr inContext = nil);
	virtual Boolean SortSelection(CDB4DSortingCriteriasPtr inCriterias, VDB4DProgressIndicator* InProgress = nil, CDB4DBaseContextPtr inContext = nil);
	virtual Boolean SortSelection(const VString orderString, VDB4DProgressIndicator* InProgress = NULL, CDB4DBaseContextPtr inContext = NULL);

	virtual Boolean SortSelectionOnClient(DB4D_FieldID inFieldID, Boolean inAscending, VDB4DProgressIndicator* InProgress = nil, CDB4DBaseContextPtr inContext = nil);
	virtual Boolean SortSelectionOnClient(CDB4DSortingCriteriasPtr inCriterias, VDB4DProgressIndicator* InProgress = nil, CDB4DBaseContextPtr inContext = nil);

	virtual CDB4DSelection* Clone(VError* outErr) const;
	virtual CDB4DSet* ConvertToSet(VError& err);

	virtual VError AddRecord(const CDB4DRecord* inRecToAdd, Boolean AtTheEnd = true, CDB4DBaseContextPtr inContext = nil);
	virtual VError AddRecordID(RecIDType inRecToAdd, Boolean AtTheEnd = true, CDB4DBaseContextPtr inContext = nil);

	virtual void Touch();
	virtual uLONG GetModificationCounter() const;

	virtual VError FillArray(sLONG* outArray, sLONG inMaxElements, CDB4DBaseContextPtr inContext);
	virtual VError FillFromArray(const sLONG* inArray, sLONG inNbElements, CDB4DBaseContextPtr inContext);

	virtual VError FillArray(xArrayOfLong &outArray, CDB4DBaseContextPtr inContext);
	virtual VError FillFromArray(const xArrayOfLong &inArray, CDB4DBaseContextPtr inContext);

	virtual VError FillArray(DB4DCollectionManager& outCollection, CDB4DBaseContextPtr inContext);
	virtual VError FillFromArray(DB4DCollectionManager& inCollection, CDB4DBaseContextPtr inContext);

	virtual VError FillArrayOfBits(void* outArray, sLONG inMaxElements);
	virtual VError FillFromArrayOfBits(const void* inArray, sLONG inNbElements);

	virtual VError DataToCollection(DB4DCollectionManager& Collection, CDB4DBaseContextPtr inContext, sLONG FromRecInSel, sLONG ToRecInSel,
																	VDB4DProgressIndicator* InProgress = nil);

	virtual VError CollectionToData(DB4DCollectionManager& Collection, CDB4DBaseContextPtr inContext, Boolean AddToSel, Boolean CreateAlways,
																	CDB4DSet* &outLockedRecords, VDB4DProgressIndicator* InProgress = nil);

	virtual VError GetDistinctValues(CDB4DField* inField, DB4DCollectionManager& outCollection, CDB4DBaseContextPtr inContext, 
										VDB4DProgressIndicator* InProgress = nil, Boolean inCaseSensitive = false);

	virtual VError GetDistinctValues(CDB4DField* inField, DB4DCollectionManager& outCollection, CDB4DBaseContextPtr inContext, 
										VDB4DProgressIndicator* InProgress, const VCompareOptions& inOptions);

	virtual VErrorDB4D GetDistinctValues(CDB4DEntityAttribute* inAttribute, DB4DCollectionManager& outCollection, CDB4DBaseContextPtr inContext, 
		VDB4DProgressIndicator* InProgress = nil, Boolean inCaseSensitive = false);

	virtual VErrorDB4D GetDistinctValues(CDB4DEntityAttribute* inAttribute, DB4DCollectionManager& outCollection, CDB4DBaseContextPtr inContext, 
		VDB4DProgressIndicator* InProgress ,const XBOX::VCompareOptions& inOptions);

	virtual VError DeleteRecords(CDB4DBaseContextPtr inContext, CDB4DSet* outNotDeletedOnes = nil, VDB4DProgressIndicator* InProgress = nil);

	virtual VError RemoveRecordNumber(CDB4DBaseContextPtr inContext, RecIDType inRecToRemove);
	virtual VError RemoveSet(CDB4DBaseContextPtr inContext, CDB4DSet* inRecsToRemove);
	virtual VError RemoveSelectedRecord(CDB4DBaseContextPtr inContext, RecIDType inRecordIndexToRemove);
	virtual VError RemoveSelectedRange(CDB4DBaseContextPtr inContext, RecIDType inFromRecordIndexToRemove, RecIDType inToRecordIndexToRemove);
	
	virtual VError BuildOneRecSelection(RecIDType inRecNum);

	virtual VError ReduceSelection(RecIDType inNbRec, CDB4DBaseContext* Context = nil);

	virtual RecIDType GetRecordPos(RecIDType inRecordID, CDB4DBaseContextPtr inContext);

	virtual CDB4DSet* GenerateSetFromRange(RecIDType inRecordIndex1, RecIDType inRecordIndex2, VErrorDB4D& err, CDB4DBaseContextPtr inContext);

	virtual CDB4DSet* GenerateSetFromRecordID(RecIDType inRecordID, RecIDType inRecordIndex2, VErrorDB4D& err, CDB4DBaseContextPtr inContext);

	virtual CDB4DSelection* RelateOneSelection(sLONG TargetOneTable, VError& err, CDB4DBaseContextPtr inContext, 
												VDB4DProgressIndicator* InProgress = nil, vector<CDB4DRelation*> *inPath = nil, 
												DB4D_Way_of_Locking HowToLock = DB4D_Do_Not_Lock, CDB4DSet* outLockSet = NULL);

	virtual CDB4DSelection* RelateManySelection(CDB4DField* inRelationStart, VError& err, CDB4DBaseContextPtr inContext, 
												VDB4DProgressIndicator* InProgress = nil, DB4D_Way_of_Locking HowToLock = DB4D_Do_Not_Lock, CDB4DSet* outLockSet = NULL);


	virtual VErrorDB4D ToClient(VStream* into, CDB4DBaseContext* inContext);
	virtual VErrorDB4D ToServer(VStream* into, CDB4DBaseContext* inContext);

	virtual void MarkOnServerAsPermanent();
	virtual void UnMarkOnServerAsPermanent(bool willResend);

	virtual VErrorDB4D ExportToSQL(CDB4DBaseContext* inContext, VFolder* inFolder, VDB4DProgressIndicator* inProgress, ExportOption& options);

	virtual void SetAssociatedModel(CDB4DEntityModel* em);
	virtual CDB4DEntityModel* GetModel() const;

	virtual CDB4DEntityRecord *LoadEntity( RecIDType inEntityIndex, DB4D_Way_of_Locking HowToLock, CDB4DBaseContextPtr inContext, Boolean* outLockWasKeptInTrans = NULL);

	virtual void SetQueryPlan(VValueBag* queryplan);
	virtual void SetQueryPath(VValueBag* queryplan);

	virtual VValueBag* GetQueryPlan();
	virtual VValueBag* GetQueryPath();

	virtual VError ConvertToJSObject(CDB4DBaseContext* inContext, VJSArray& outArr, const VString& inAttributeList, bool withKey, bool allowEmptyAttList, sLONG from, sLONG count);

	
	//========== end of component methods ===============

	VDBMgr *GetManager() const {return fManager;}
	Boolean SortSelection(SortTab* tabs, VDB4DProgressIndicator* InProgress, CDB4DBaseContextPtr inContext, VError& err, Boolean TestUnicite, Boolean forceserver);
	/*
	VError SubSort( sLONG l, sLONG r, SortContext& tri);
	uBOOL issuptri( sLONG l, sLONG r, VError *err, SortContext& tri);
	void DeChargeTri(SortElem &se);
	VError ChargeTri(SortElem &se, sLONG n);
	*/

	FicheInMem *NewFicheInMem();
	FicheInMem *LoadFicheInMem( RecIDType inRecordID, DB4D_Way_of_Locking HowToLock, BaseTaskInfo* context, Boolean WithSubTable, Boolean* outLockWasKeptInTrans);
	Selection* GetSel(void) { return fSel; };

	Table* GetTable() { return fTable; };
	
	

private:
	VDBMgr	*fManager;
	Table *fTable;
	VDB4DBase*	fBase;
	Selection* fSel;
	CDB4DEntityModel* fModel;
};


class VDB4DSet : public VComponentImp<CDB4DSet>
{
public:

	// the base must have already been retained
	VDB4DSet( VDB4DBase *inBase, Table *inTable, Bittab* inSet);
	virtual ~VDB4DSet();

	virtual DB4D_TableID GetTableRef() const;
	virtual CDB4DBase* GetBaseRef() const;

	virtual RecIDType CountRecordsInSet() const;
	virtual RecIDType GetMaxRecordsInSet() const;
	virtual VError SetMaxRecordsInSet(RecIDType inMax) const;

	virtual Boolean IsOn(RecIDType inPos) const;
	virtual VError ClearOrSet(RecIDType inPos, Boolean inValue);
	virtual VError ClearOrSetAll(Boolean inValue);
	virtual RecIDType	FindNextOne(RecIDType inFirstToLook) const;
	virtual	RecIDType	FindPreviousOne(RecIDType inFirstToLook) const;
	virtual VError And(const CDB4DSet& other);
	virtual	VError Or(const CDB4DSet& other);
	virtual	VError Minus(const CDB4DSet& other);
	virtual	VError Invert();
	
	virtual VError CloneFrom(const CDB4DSet& other);
	virtual CDB4DSet* Clone(VError* outErr = nil) const;
	virtual VError Compact();
	virtual CDB4DSelection* ConvertToSelection(VError& err, CDB4DBaseContext* inContext) const;

	virtual VError FillArrayOfLongs(sLONG* outArray, sLONG inMaxElements);
	virtual VError FillFromArrayOfLongs(const sLONG* inArray, sLONG inNbElements);

	virtual VError FillArrayOfBits(void* outArray, sLONG inMaxElements);
	virtual VError FillFromArrayOfBits(const void* inArray, sLONG inNbElements);

	virtual VError WriteToStream(VStream& outStream);
	virtual VError ReadFromStream(VStream& inStream);

	virtual VErrorDB4D ToClient(VStream* into, CDB4DBaseContext* inContext);
	virtual VErrorDB4D ToServer(VStream* into, CDB4DBaseContext* inContext, bool inKeepOnServer);

	virtual void MarkOnServerAsPermanent();
	virtual void UnMarkOnServerAsPermanent(bool willResend);

	//========== end of component methods ===============

	inline Bittab* GetBittab() { return fSet; };

private:
	Table *fTable;
	VDB4DBase*	fBase;
	Bittab* fSet;
};


class VDB4DQueryOptions : public VComponentImp<CDB4DQueryOptions>
{
	public:
		virtual void SetFilter( CDB4DSelection* inFilter);
		virtual void SetLimit(sLONG8 inNewLimit);

		virtual void SetWayOfLocking(DB4D_Way_of_Locking HowToLock);
		virtual void SetWayOfLockingForFirstRecord(DB4D_Way_of_Locking HowToLock);

		virtual void SetWantsLockedSet(Boolean WantsLockedSet);
		virtual void SetWantsFirstRecord(Boolean WantsFirstRecord);

		virtual void SetDestination(DB4D_QueryDestination inDestination);

		virtual void DescribeQueryExecution(Boolean on);

		inline QueryOptions* GetOptions()
		{
			return &fOptions;
		}

	protected:

		QueryOptions fOptions;
};


class VDB4DQueryResult : public VComponentImp<CDB4DQueryResult>
{
	public:
		VDB4DQueryResult(CDB4DTable* inTable)
		{
			fSelection = nil;
			fSet = nil;
			fLockedSet = nil;
			fFirstRec = nil;
			fTable = inTable;
			assert(fTable != nil);
			fTable->Retain();
		}

		VDB4DQueryResult(Table* inTable)
		{
			VDB4DBase* xbase = VImpCreator<VDB4DBase>::GetImpObject(inTable->GetOwner()->RetainBaseX());
			fSelection = nil;
			fSet = nil;
			fLockedSet = nil;
			fFirstRec = nil;
			fTable = new VDB4DTable(VDBMgr::GetManager(), xbase, inTable);
			xbase->Release();
		}

		VDB4DQueryResult(CDB4DTable* inTable, CDB4DSelection *inSelection, CDB4DSet *inSet, CDB4DSet *inLockedSet, CDB4DRecord *inFirstRecord)
		{
			fSelection = inSelection;
			fSet = inSet;
			fLockedSet = inLockedSet;
			fFirstRec = inFirstRecord;
			fTable = inTable;

			assert(fTable != nil);
			fTable->Retain();

			if (fSelection != nil)
				fSelection->Retain();
			if (fSet != nil)
				fSet->Retain();
			if (fLockedSet != nil)
				fLockedSet->Retain();
			if (fFirstRec != nil)
				fFirstRec->Retain();
		}

		virtual ~VDB4DQueryResult()
		{
			ReleaseRefCountable(&fSelection);
			ReleaseRefCountable(&fSet);
			ReleaseRefCountable(&fLockedSet);
			ReleaseRefCountable(&fFirstRec);
			ReleaseRefCountable(&fTable);
		}

		inline QueryResult& GetResult()
		{
			return fResult;
		}

		virtual CDB4DSelection* GetSelection();
		virtual CDB4DSet* GetSet();
		virtual sLONG8 GetCount();
		virtual CDB4DSet* GetLockedSet();
		virtual CDB4DRecord* GetFirstRecord();

		virtual void GetQueryDescription(XBOX::VString& outResult, DB4D_QueryDescriptionFormat inFormat);
		virtual void GetQueryExecution(XBOX::VString& outResult, DB4D_QueryDescriptionFormat inFormat);

	protected:
		QueryResult fResult;
		CDB4DSelection* fSelection;
		CDB4DSet* fSet;
		CDB4DSet* fLockedSet;
		CDB4DRecord* fFirstRec;
		CDB4DTable* fTable;

};


class VDB4DQuery : public VComponentImp<CDB4DQuery>
{
public:
	//VDB4DQuery( VDB4DTableContext *inTableContext);
	VDB4DQuery( VDBMgr* manager, Table *inTable, CDB4DTable* owner, EntityModel* inModel = nil);
	virtual ~VDB4DQuery();

	virtual Boolean AddCriteria( DB4D_TableID inTableID, DB4D_FieldID inFieldID, DB4DComparator inComparator, const VValueSingle& inValue, Boolean inDiacritic = false);
	virtual Boolean AddCriteria( const VString& inTableName, const VString& inFieldName, DB4DComparator inComparator, const VValueSingle& inValue, Boolean inDiacritic = false);
	
	virtual Boolean AddCriteria( CDB4DField* inField, DB4DComparator inComparator, const XBOX::VValueSingle& inValue, Boolean inDiacritic = false);

	virtual Boolean AddCriteria( CDB4DField* inField, DB4DComparator inComparator, DB4DArrayOfValues *inValue, Boolean inDiacritic = false);
	// DB4D will call release on the DB4DArrayOfValues when the query destructor is called

	virtual Boolean AddCriteria( CDB4DField* inField, DB4DComparator inComparator, CDB4DField* inFieldToCompareTo, Boolean inDiacritic = false);

	virtual Boolean AddEmCriteria(const VString& inAttributePath, DB4DComparator inComparator, const VValueSingle& inValue, Boolean inDiacritic = false);

	virtual Boolean AddExpression(DB4DLanguageExpression* inExpression, DB4D_TableID inTableID = 0);

	virtual Boolean AddLogicalOperator( DB4DConjunction inConjunction);

	virtual Boolean AddNotOperator();
	virtual Boolean OpenParenthesis();
	virtual Boolean CloseParenthesis();

	virtual void SetDisplayProperty(Boolean inDisplay);

	virtual CDB4DTable* GetTargetTable();

	virtual VError BuildFromString(const VString& inQueryText, VString& outOrderby, CDB4DBaseContext* context, CDB4DEntityModel* inModel = NULL, const QueryParamElementVector* params = NULL);

	virtual CDB4DQueryPathNode* BuildQueryPath(Boolean inSubSelection, const CDB4DQueryPathModifiers* inModifiers, CDB4DBaseContextPtr inContext);

	virtual VError GetParams(vector<VString>& outParamNames, QueryParamElementVector& outParamValues);
	
	virtual VError SetParams(const vector<VString>& inParamNames, const QueryParamElementVector& inParamValues);
	
	//========== end of component methods ===============

	VDBMgr *GetManager() const {return fManager;}

	SearchTab	*GetSearchTab()	{return &fQuery;}
	const SearchTab	*GetSearchTab() const	{return &fQuery;}

	Boolean WithFormulas() const;

private:
	VDBMgr	*fManager;
	CDB4DTable* Owner;
	SearchTab	fQuery;
	EntityModel* fModel;
};



class VDB4DComplexSelection : public VComponentImp<CDB4DComplexSelection>
{
	public:
		inline VDB4DComplexSelection(VDBMgr* manager, ComplexSelection* sel)
		{ 
			fManager = manager;
			fSel = sel;
			assert(sel != nil);
			fSel->Retain();
		};

		virtual ~VDB4DComplexSelection();

		virtual RecIDType GetRecID(sLONG Column, RecIDType Row, VErrorDB4D& outErr);
		virtual VErrorDB4D GetFullRow(RecIDType Row, ComplexSelRow& outRow);

		virtual CDB4DComplexSelection* And(CDB4DComplexSelection* inOther, VErrorDB4D& outErr);
		virtual CDB4DComplexSelection* Or(CDB4DComplexSelection* inOther, VErrorDB4D& outErr);
		virtual CDB4DComplexSelection* Minus(CDB4DComplexSelection* inOther, VErrorDB4D& outErr);
		virtual VErrorDB4D SortByRecIDs(const vector<sLONG>& onWhichColums);

		virtual sLONG CountRows();

		virtual sLONG CountColumns();

		virtual VErrorDB4D AddRow(const ComplexSelRow& inRow);

		virtual sLONG PosOfTarget(const QueryTarget& inTarget);

		virtual VErrorDB4D RetainColumns(QueryTargetVector& outColumns);

		virtual VErrorDB4D RetainColumn(sLONG Column, QueryTarget& outTarget);

		virtual VErrorDB4D ToDB4D_ComplexSel(DB4D_ComplexSel& outSel);

		inline ComplexSelection* GetSel() const { return fSel; };

	protected:
		ComplexSelection* fSel;
		VDBMgr	*fManager;
};



class VDB4DComplexQuery : public VComponentImp<CDB4DComplexQuery>
{
	public:

		inline VDB4DComplexQuery(VDBMgr* manager, Base4D* owner):fQuery(owner) { fManager = manager; };

		virtual void SetSimpleTarget(CDB4DTable* inTarget);

		virtual VError SetComplexTarget(const QueryTargetVector& inTargets);


		virtual VError AddCriteria( CDB4DField* inField, sLONG Instance, DB4DComparator inComparator, const XBOX::VValueSingle& inValue, Boolean inDiacritic = false);

		virtual VError AddCriteria( CDB4DField* inField, sLONG Instance, DB4DComparator inComparator, DB4DArrayOfValues *inValue, Boolean inDiacritic = false);
		// DB4D will call release on the DB4DArrayOfValues when the query destructor is called

		virtual VError AddJoin( CDB4DField* inField1, sLONG Instance1, DB4DComparator inComparator, CDB4DField* inField2, sLONG Instance2, 
								Boolean inDiacritic = false, bool inLeftJoin = false, bool inRightJoin = false);

		virtual VError AddExpression(CDB4DTable* inTable, sLONG Instance, DB4DLanguageExpression* inExpression);

		virtual VError AddSQLExpression(CDB4DTable* inTable, sLONG Instance, DB4DSQLExpression* inExpression);

		virtual VError AddSQLExpression(const QueryTargetVector& inTargets, DB4DSQLExpression* inExpression);

		virtual VError AddLogicalOperator( DB4DConjunction inConjunction);

		virtual VError AddNotOperator();
		virtual VError OpenParenthesis();
		virtual VError CloseParenthesis();

		virtual VError BuildFromString(const VString& inQueryText);
		virtual VError BuildTargetsFromString(const VString& inTargetsText);

		virtual VError BuildQueryDescriptor(VString& outDescription);

		inline ComplexRech* GetQuery() { return &fQuery; };

	protected:
		ComplexRech fQuery;
		VDBMgr	*fManager;
};



class VDB4DSortingCriterias : public VComponentImp<CDB4DSortingCriterias>
{
public:
	//VDB4DSortingCriterias( VDB4DTableContext *inTableContext);
	VDB4DSortingCriterias( VDBMgr* manager, Table *inTable);
	virtual ~VDB4DSortingCriterias();

	virtual Boolean AddCriteria(DB4D_FieldID inFieldID, Boolean inAscending = true);
	virtual Boolean AddCriteria(CDB4DField* inField, Boolean inAscending = true);
	virtual Boolean AddExpression(DB4DLanguageExpression* inExpression, Boolean inAscending = true);

	//========== end of component methods ===============

	VDBMgr *GetManager() const {return fManager;}

	SortTab	*GetSortTab()	{return &fSortTab;}
	const SortTab	*GetSortTab() const	{return &fSortTab;}

private:
	VDBMgr	*fManager;
	Table *fTable;
	SortTab	fSortTab;
};



class VDB4DSqlQuery : public VComponentImp<CDB4DSqlQuery>
{
public:
	VDB4DSqlQuery( VDBMgr *inManager, Base4D* WhichBase, BaseTaskInfo* WhichContext, VString& request, VError& err );
	virtual ~VDB4DSqlQuery();

	VDBMgr *GetManager() const {return fManager;}
	
	virtual VError ExecSql(CDB4DSelection* &outSelection, VDB4DProgressIndicator* InProgress = nil);

private:
	VDBMgr	*fManager;
//	SqlQuery*	fQuery;
	VError lasterr;
	BaseTaskInfo* fOwner;
};


class VDB4DRecord : public VComponentImp<CDB4DRecord>
{
public:
	VDB4DRecord( VDBMgr* manager, FicheInMem *inFiche, CDB4DBaseContextPtr inContext);
	//VDB4DRecord( VDB4DTableContext *inTableContext, FicheInMem *inFiche);
	virtual ~VDB4DRecord();

	virtual RecIDType GetID() const;
	
	virtual Boolean IsNew() const;
	virtual Boolean IsProtected() const;
	virtual Boolean IsModified() const;
	virtual Boolean IsFieldModified( DB4D_FieldID inFieldID) const;

	virtual uLONG GetFieldModificationStamp( DB4D_FieldID inFieldID) const;
	virtual uLONG GetFieldModificationStamp( const CDB4DField* inField) const;

	virtual void Touch(DB4D_FieldID inFieldID);
	virtual void Touch(const CDB4DField* inField);

	// tries to coerce. returns false if failed (property not found or coercion not available)
	virtual Boolean GetString( DB4D_FieldID inFieldID, VString& outValue, Boolean OldOne = false, VError *outErr = nil) const;
	virtual Boolean GetBoolean( DB4D_FieldID inFieldID, Boolean *outValue, Boolean OldOne = false, VError *outErr = nil) const;
	virtual Boolean GetLong( DB4D_FieldID inFieldID, sLONG *outValue, Boolean OldOne = false, VError *outErr = nil) const;
	virtual Boolean GetBlob( DB4D_FieldID inFieldID, VBlob **outBlob, Boolean OldOne = false, VError *outErr = nil) const;
	virtual Boolean GetVUUID( DB4D_FieldID inFieldID, VUUID& outValue, Boolean OldOne = false, VError *outErr = nil) const;
	virtual Boolean GetPicture( DB4D_FieldID inFieldID, VValueSingle **outBlob, Boolean OldOne = false, VError *outErr = nil) const;

	virtual Boolean SetString( DB4D_FieldID inFieldID, const VString& inValue, VError *outErr = nil);
	virtual Boolean SetBoolean( DB4D_FieldID inFieldID, Boolean inValue, VError *outErr = nil);
	virtual Boolean SetLong( DB4D_FieldID inFieldID, sLONG inValue, VError *outErr = nil);
	virtual Boolean SetBlob( DB4D_FieldID inFieldID, const void *inData, sLONG inDataSize, VError *outErr = nil);
	virtual Boolean SetVUUID( DB4D_FieldID inFieldID, const VUUID& inValue, VError *outErr = nil);
	virtual Boolean SetPicture( DB4D_FieldID inFieldID, const VValueSingle& inPict, VError *outErr = nil);
	
	virtual VValueSingle *GetFieldValue( DB4D_FieldID inFieldID, Boolean OldOne = false, VError *outErr = nil, bool forQueryOrSort = false);
	
	virtual Boolean Save(VError *outErr = nil);

	virtual Boolean GetDuration( DB4D_FieldID inFieldID, VDuration& outValue, Boolean OldOne = false, VError *outErr = nil) const;
	virtual Boolean GetTime( DB4D_FieldID inFieldID, VTime& outValu, Boolean OldOne = false, VError *outErr = nil) const;
	virtual Boolean GetLong8( DB4D_FieldID inFieldID, sLONG8 *outValue, Boolean OldOne = false, VError *outErr = nil) const;

	virtual Boolean SetDuration( DB4D_FieldID inFieldID, const VDuration& inValue, VError *outErr = nil);
	virtual Boolean SetTime( DB4D_FieldID inFieldID, const VTime& inValue, VError *outErr = nil);
	virtual Boolean SetLong8( DB4D_FieldID inFieldID, sLONG8 inValue, VError *outErr = nil);

	virtual Boolean GetReal( DB4D_FieldID inFieldID, Real *outValue, Boolean OldOne = false, VError *outErr = nil) const;
	virtual Boolean SetReal( DB4D_FieldID inFieldID, Real inValue, VError *outErr = nil);

	virtual void GetTimeStamp(VTime& outValue);

	virtual FicheInMem4DPtr GetFicheInMem(void);

	virtual Boolean Drop(VError *outErr = nil);


	virtual Boolean IsFieldModified( const CDB4DField* inField) const;

	virtual Boolean GetString( const CDB4DField* inField, VString& outValue, Boolean OldOne = false, VError *outErr = nil) const;
	virtual Boolean GetBoolean( const CDB4DField* inField, Boolean *outValue, Boolean OldOne = false, VError *outErr = nil) const;
	virtual Boolean GetLong( const CDB4DField* inField, sLONG *outValue, Boolean OldOne = false, VError *outErr = nil) const;
	virtual Boolean GetLong8( const CDB4DField* inField, sLONG8 *outValue, Boolean OldOne = false, VError *outErr = nil) const;
	virtual Boolean GetReal( const CDB4DField* inField, Real *outValue, Boolean OldOne = false, VError *outErr = nil) const;
	virtual Boolean GetBlob( const CDB4DField* inField, VBlob **outBlob, Boolean OldOne = false, VError *outErr = nil) const;
	virtual Boolean GetVUUID( const CDB4DField* inField, VUUID& outValue, Boolean OldOne = false, VError *outErr = nil) const;
	virtual Boolean GetDuration( const CDB4DField* inField, VDuration& outValue, Boolean OldOne = false, VError *outErr = nil) const;
	virtual Boolean GetTime( const CDB4DField* inField, VTime& outValue, Boolean OldOne = false, VError *outErr = nil) const;
	virtual Boolean GetPicture( const CDB4DField* inField, VValueSingle **outPict, Boolean OldOne = false, VError *outErr = nil) const;
	
	virtual Boolean SetString( const CDB4DField* inField, const VString& inValue, VError *outErr = nil);
	virtual Boolean SetBoolean( const CDB4DField* inField, Boolean inValue, VError *outErr = nil);
	virtual Boolean SetLong( const CDB4DField* inField, sLONG inValue, VError *outErr = nil);
	virtual Boolean SetLong8( const CDB4DField* inField, sLONG8 inValue, VError *outErr = nil);
	virtual Boolean SetReal( const CDB4DField* inField, Real inValue, VError *outErr = nil);
	virtual Boolean SetBlob( const CDB4DField* inField, const void *inData, sLONG inDataSize, VError *outErr = nil);
	virtual Boolean SetVUUID( const CDB4DField* inField, const VUUID& inValue, VError *outErr = nil);
	virtual Boolean SetDuration( const CDB4DField* inField, const VDuration& inValue, VError *outErr = nil);
	virtual Boolean SetTime( const CDB4DField* inField, const VTime& inValue, VError *outErr = nil);
	virtual Boolean SetPicture( const CDB4DField* inField, const VValueSingle& inPict, VError *outErr = nil);
	
	virtual void SetFieldToNull(const CDB4DField* inField, VError *outErr = nil);
	virtual void SetFieldToNull(DB4D_FieldID inFieldID, VError *outErr = nil);

	virtual VValueSingle *GetFieldValue( const CDB4DField* inField, Boolean OldOne = false, VError *outErr = nil, bool forQueryOrSort = false);

	virtual Boolean GetFieldIntoBlob( const CDB4DField* inField,  VBlob& outBlob, VErrorDB4D *outErr = nil, Boolean CanCacheData = true);
	virtual Boolean GetFieldIntoBlob( DB4D_FieldID inFieldID,  VBlob& outBlob, VErrorDB4D *outErr = nil, Boolean CanCacheData = true);

	virtual CDB4DRecord* Clone(VError* outErr = nil) const;

	virtual VError DetachRecord(Boolean BlobFieldsCanBeEmptied);  // transforms it internally as a new record but with its current data

	virtual void WhoLockedIt(DB4D_KindOfLock& outLockType, const VValueBag **outLockingContextRetainedExtraData) const;

	virtual sLONG8 GetSequenceNumber();

	virtual VError FillAllFieldsEmpty();

	virtual DB4D_TableID GetTableRef() const;

	virtual CDB4DTable* RetainTable() const;

	virtual VErrorDB4D ReserveRecordNumber();

	virtual CDB4DRecord* CloneOnlyModifiedValues(VErrorDB4D& err) const;

	virtual VErrorDB4D RevertModifiedValues(CDB4DRecord* From);

	virtual CDB4DRecord* CloneForPush(VErrorDB4D* outErr = nil);

	virtual void RestoreFromPop();

	virtual uLONG GetModificationStamp() const;
	virtual uLONG8 GetSyncInfoStamp() const;

	virtual VErrorDB4D ToClient(VStream* into, CDB4DBaseContext* inContext);
	virtual VErrorDB4D ToServer(VStream* into, CDB4DBaseContext* inContext);

	inline FicheInMem* GetRec() const { return fRecord; };
	inline void SetRec(FicheInMem* rec) { fRecord = rec; };

private:
	VDBMgr	*fManager;
	mutable FicheInMem	*fRecord;
	BaseTaskInfo* fContext;
	mutable CDB4DTable* fOwner;
	mutable Boolean fIsLoaded;
};


class VDB4DImpExp : public VComponentImp<CDB4DImpExp>
{
	public:
		VDB4DImpExp(CDB4DTable* inTarget, Table* intarget);
		virtual ~VDB4DImpExp();
		
		virtual CDB4DTable* GetTarget(void);
		
		virtual Boolean AddCol(DB4D_FieldID inField);
		virtual sLONG CountCol(void) const;
		virtual DB4D_FieldID GetCol(sLONG n) const;
		
		virtual void SetPath(const VFilePath& newpath);
		virtual void SetPath(const VString& newpath);
		
		virtual void GetPath(VFilePath& curpath) const;
		virtual void GetPath(VString& curpath) const;
		
		virtual void SetColDelimit(const VString& newColDelimit);
		virtual void GetColDelimit(VString& curColDelimit) const;
		
		virtual void SetRowDelimit(const VString& newRowDelimit);
		virtual void GetRowDelimit(VString& curRowDelimit) const;
		
		virtual VError RunImport(CDB4DSelection* &outSel, VDB4DProgressIndicator* InProgress = nil, CDB4DBaseContextPtr InContext = nil);
		virtual VError RunExport(CDB4DSelection* inSel, VDB4DProgressIndicator* InProgress = nil, CDB4DBaseContextPtr InContext = nil);

		virtual void SetCharSet(CharSet newset);
		virtual CharSet GetCharSet();

	private:
		ImpExp IO;
		CDB4DTable* target;
		
};


class VDB4DCheckAndRepairAgent : public VComponentImp<CDB4DCheckAndRepairAgent>
{
	public:
		VDB4DCheckAndRepairAgent(CDB4DBase* inTarget);
		virtual ~VDB4DCheckAndRepairAgent();
		
		virtual VError Run(VStream* outMsg, ListOfErrors& OutList, CDB4DBaseContext* inContext, VDB4DProgressIndicator* InProgress = nil);

		virtual void SetCheckTableState(Boolean OnOff);
		virtual void SetCheckAllTablesState(Boolean OnOff);

		virtual void SetCheckIndexState(Boolean OnOff);
		virtual void SetCheckAllIndexesState(Boolean OnOff);

		virtual void SetCheckBlobsState(Boolean OnOff);

		virtual CDB4DBase* GetOwner() const;

	private:
		VDB4DBase* fTarget;
		CheckAndRepairAgent fAgent;
		
};


class VDB4DRelation : public VComponentImp<CDB4DRelation>
{
	public:
		VDB4DRelation(CDB4DBase* inTarget, Relation* xrel);
		virtual ~VDB4DRelation();
	

		virtual VError ActivateManyToOneS(CDB4DRecord *InRec, CDB4DSelection* &OutResult, CDB4DBaseContextPtr inContext, 
																			Boolean OldOne = false, Boolean LockedSel = false);

		virtual VError ActivateManyToOne(CDB4DRecord *InRec, CDB4DRecord* &OutResult, CDB4DBaseContextPtr inContext, 
																		 Boolean OldOne = false, Boolean NoCache = false, Boolean ReadOnly = false);
		virtual VError ActivateOneToMany(CDB4DRecord *InRec, CDB4DSelection* &OutResult, CDB4DBaseContextPtr inContext, 
																		 Boolean OldOne = false, Boolean NoCache = false, Boolean LockedSel = false);


		virtual VErrorDB4D ActivateManyToOneS(CDB4DRecord *InRec, CDB4DQueryOptions& inOptions, CDB4DQueryResult* &OutResult, CDB4DBaseContextPtr inContext, 
												Boolean OldOne = false); 

		virtual VErrorDB4D ActivateManyToOne(CDB4DRecord *InRec, CDB4DQueryOptions& inOptions, CDB4DQueryResult* &OutResult, CDB4DBaseContextPtr inContext, 
												Boolean OldOne = false, Boolean NoCache = false);

		virtual VErrorDB4D ActivateOneToMany(CDB4DRecord *InRec, CDB4DQueryOptions& inOptions, CDB4DQueryResult* &OutResult, CDB4DBaseContextPtr inContext, 
												Boolean OldOne = false, Boolean NoCache = false);


		virtual VError Drop(CDB4DBaseContextPtr inContext = NULL);
		virtual void GetNameNto1(VString& outName, CDB4DBaseContextPtr inContext = NULL) const;
		virtual void GetName1toN(VString& outName, CDB4DBaseContextPtr inContext = NULL) const;
		virtual void RetainSource(CDB4DField* &outSourceField, CDB4DBaseContextPtr inContext = NULL) const;
		virtual void RetainDestination(CDB4DField* &outDestinationField, CDB4DBaseContextPtr inContext = NULL) const;
		virtual VError RetainSources(CDB4DFieldArray& outSourceFields, CDB4DBaseContextPtr inContext = NULL) const;
		virtual VError RetainDestinations(CDB4DFieldArray& outDestinationFields, CDB4DBaseContextPtr inContext = NULL) const;
		//virtual DB4D_RelationType GetType(CDB4DBaseContextPtr inContext = NULL) const;
		virtual void GetUUID(VUUID& outID, CDB4DBaseContextPtr inContext = NULL) const;
		virtual VError SetNameNto1(const VString& inName, CDB4DBaseContextPtr inContext = NULL);
		virtual VError SetName1toN(const VString& inName, CDB4DBaseContextPtr inContext = NULL);
		virtual VError SetAutoLoadNto1(Boolean inState, CDB4DBaseContextPtr inContext = NULL);
		virtual Boolean IsAutoLoadNto1(CDB4DBaseContextPtr inContext = NULL) const;
		virtual VError SetAutoLoad1toN(Boolean inState, CDB4DBaseContextPtr inContext = NULL);
		virtual Boolean IsAutoLoad1toN(CDB4DBaseContextPtr inContext = NULL) const;
		virtual VError SetState(DB4D_RelationState InState, CDB4DBaseContextPtr inContext = NULL);
		virtual DB4D_RelationState GetState(CDB4DBaseContextPtr inContext = NULL) const;
		virtual VValueBag *CreateDefinition(CDB4DBaseContextPtr inContext = NULL) const;
		virtual VErrorDB4D SetReferentialIntegrity(Boolean inReferentialIntegrity, Boolean inAutoDeleteRelatedRecords, CDB4DBaseContextPtr inContext = NULL);
		virtual Boolean isReferentialIntegrity(CDB4DBaseContextPtr inContext = NULL);
		virtual Boolean isAutoDeleteRelatedRecords(CDB4DBaseContextPtr inContext = NULL);

		virtual VErrorDB4D SetForeignKey(Boolean on, CDB4DBaseContextPtr inContext = NULL);
		virtual Boolean isForeignKey(CDB4DBaseContextPtr inContext = NULL) const;

		virtual const VValueBag* RetainExtraProperties(VError &err, CDB4DBaseContextPtr inContext = NULL);
		virtual VError SetExtraProperties(VValueBag* inExtraProperties, CDB4DBaseContextPtr inContext = NULL);

		virtual sLONG GetStamp(CDB4DBaseContextPtr inContext = NULL) const;

		virtual sLONG GetPosInList(CDB4DBaseContextPtr inContext = NULL) const;

		CDB4DTable* RetainSourceTable();
		CDB4DTable* RetainDestinationTable();

		//========== end of component methods ===============

		inline Relation* GetRel() const {return fRel; };

		// plus utilise
		//VError SetFields(DB4D_RelationType inWhatType, const VString& inName, CDB4DField* inSourceField, CDB4DField* inDestinationField);

		//il faudrait utiliser 	IRefCountable::CopyRefCountable
		//inline void SetRel(Relation* xrel) { if (fRel!= nil) fRel->Release(); fRel = xrel; };

	private:
		Relation* fRel;
		VDB4DBase* fTarget;
};


class VDB4DColumnFormula : public VComponentImp<CDB4DColumnFormula>
{
	public:
		VDB4DColumnFormula(Table* target);

		virtual VErrorDB4D Add(CDB4DField* inColumn, DB4D_ColumnFormulae inFormula);
		virtual VErrorDB4D Execute(CDB4DSelection* inSel, CDB4DBaseContext* inContext, VDB4DProgressIndicator* InProgress = NULL, CDB4DRecord* inCurrentRecord = NULL);
		virtual sLONG8 GetResultAsLong8(sLONG inColumnNumber) const;
		virtual Real GetResultAsReal(sLONG inColumnNumber) const;
		virtual void GetResultAsFloat(sLONG inColumnNumber, VFloat& outResult) const;
		virtual void GetResultAsDuration(sLONG inColumnNumber, VDuration& outResult) const;
		virtual VValueSingle* GetResult(sLONG inColumnNumber) const;

	private:
		ColumnFormulas fFormules;

};


class VDB4DIndex : public VComponentImp<CDB4DIndex>
{
	public:

		VDB4DIndex(IndexInfo* xInd);
		virtual ~VDB4DIndex();

		virtual sLONG GetSourceType(CDB4DBaseContextPtr inContext = NULL); // for example, index on 1 field, or on multiple fields, or on fulltext

		virtual sLONG GetStorageType(CDB4DBaseContextPtr inContext = NULL); // btree, hastable, etc...

		virtual Boolean IsTypeAuto(CDB4DBaseContextPtr inContext = NULL);

		virtual const	XBOX::VString& GetName(CDB4DBaseContextPtr inContext = NULL) const;
		virtual VErrorDB4D SetName(const XBOX::VString& inName, CDB4DBaseContextPtr inContext = NULL);
		virtual void GetUUID(XBOX::VUUID& outID, CDB4DBaseContextPtr inContext = NULL) const;

		virtual CDB4DField* RetainDataSource(CDB4DBaseContextPtr inContext = NULL); // when index is on 1 field return the CDB4DField else returns nil

		virtual VErrorDB4D RetainDataSources(CDB4DFieldArray& outSources, CDB4DBaseContextPtr inContext = NULL); // when index is on 1 or more fields returns and array of retained CDB4DField

		virtual CDB4DTable* RetainTargetReference(CDB4DBaseContextPtr inContext = NULL); // usually the same table than the field(s) data source

		virtual Boolean MayBeSorted(CDB4DBaseContextPtr inContext = NULL);

		virtual VErrorDB4D Drop(VDB4DProgressIndicator* InProgress = NULL, VSyncEvent* event = NULL, CDB4DBaseContextPtr inContext = NULL);

		virtual RecIDType FindKey(const VValueSingle &inValToFind, VErrorDB4D& err, CDB4DBaseContext* inContext, VValueSingle* *outResult, CDB4DSelectionPtr Filter, const VCompareOptions& inOptions);
		virtual RecIDType FindKey(const ListOfValues &inValToFind, VErrorDB4D& err, CDB4DBaseContext* inContext, ListOfValues* *outResult, CDB4DSelectionPtr Filter, const VCompareOptions& inOptions);

		virtual CDB4DIndexKey* FindIndexKey(const VValueSingle &inValToFind, VErrorDB4D& err, CDB4DBaseContext* inContext, Boolean inBeginOfText = true);
		virtual CDB4DIndexKey* FindIndexKey(const ListOfValues &inValToFind, VErrorDB4D& err, CDB4DBaseContext* inContext, Boolean inBeginOfText = true);

		virtual CDB4DSelection* ScanIndex(RecIDType inMaxRecords, Boolean KeepSorted, Boolean Ascent, CDB4DBaseContext* inContext, VError& outError, CDB4DSelection* filter = nil);

		virtual CDB4DQueryResult* ScanIndex(CDB4DQueryOptions& inOptions, Boolean KeepSorted, Boolean Ascent, CDB4DBaseContext* inContext, VErrorDB4D& outError);

		virtual Boolean IsBuilding();

//		virtual void* GetSourceLang();

		virtual CDB4DBase* RetainOwner(const char* DebugInfo, CDB4DBaseContextPtr inContext = NULL);

		virtual void ReleaseFromQuery();

		virtual VError GetBuildError() const;

		virtual Boolean IsValid(CDB4DBaseContextPtr inContext = NULL);

		virtual void FreeAndRelease();

		inline IndexInfo* GetInd() const { return fInd; };

	protected:
		IndexInfo* fInd;
};

const sLONG kMaxPageLevelInPath = 20;

class VDB4DIndexKey : public VComponentImp<CDB4DIndexKey>
{
	public:

		virtual RecIDType GetRecordID() const;

		virtual CDB4DRecord* GetRecord(VError& err);

		virtual CDB4DIndexKey* NextKey(VError& err) const;

		VDB4DIndexKey(IndexInfo* xind, BaseTaskInfo* inContext);
		virtual ~VDB4DIndexKey();

		void AddLevelToPath(sLONG numpage);
		void DecLevelFromPath();
		inline const sLONG* GetPath() const { return &fPagePath[0]; };
		inline sLONG GetNbLevel() const { return fNbLevel; };

		void SetCurPos(sLONG curpos, sLONG curposinsel, sLONG recnum);
		inline sLONG GetCurpos() const { return fCurPos; }; 
		inline sLONG GetCurposInSel() const { return fCurPosInSel; }; 
		inline sLONG GetRecNum() const { return fRecNum; }; 

	protected:
		BaseTaskInfo* fContext;
		sLONG fPagePath[kMaxPageLevelInPath];
		sLONG fNbLevel;
		IndexInfo* fInd;
		sLONG fCurPos;
		sLONG fCurPosInSel;
		sLONG fRecNum;
		CDB4DRecord *fCurRec;
		Boolean fReadOnly;
};


class contextElem
{
	public:

		CDB4DBaseContextPtr fContext;
		bool fMustRelease;
};

typedef map<Base4D*, contextElem> ContextByBaseMap;

class VDB4DContext : public VComponentImp<CDB4DContext>
{
	public:
		inline VDB4DContext(CUAGSession* inUserSession, VJSGlobalContext* inJSContext, bool islocal):fLanguageContext( nil),fExtra( nil), fNeedsToSendExtraData( false)
		{ 
			fIsLocal = islocal;
			fUserSession = RetainRefCountable(inUserSession);
			if (inJSContext != kJSContextCreator)
				fJSContext = inJSContext;	// sc 24/08/2009 no more retain on JavaScript context
			else
				fJSContext = nil;
			fCurJSFuncNum = 0;
		};
		virtual ~VDB4DContext();
		virtual CDB4DBaseContextPtr RetainDataBaseContext(CDB4DBase* inTarget, Boolean ForceCreate = true, bool reallyRetain = true);
		//virtual void SetDataBaseContext(CDB4DBase* inTarget, CDB4DBaseContextPtr inContext);

		virtual VUUID& GetID();
		virtual const VUUID& GetID() const;

		virtual void SendlastRemoteInfo();

		virtual	void				SetLanguageContext( VDBLanguageContext *inLanguageContext);
		virtual	VDBLanguageContext*	GetLanguageContext() const;

		inline void GenerateID()
		{
			fID.Regenerate();
		}

		inline void SetID(const VUUID& inID)
		{
			fID = inID;
		}

		inline CDB4DBaseContextPtr GetFirstBaseContext()
		{
			if (fContexts.empty())
				return nil;
			else
				return fContexts.begin()->second.fContext;
		}

		void NowOwns(CDB4DBaseContextPtr context);
		void ContainButDoesNotOwn(CDB4DBaseContextPtr context);

		void CloseAllConnections();

		/*
		inline VArrayRetainedOwnedPtrOf<CDB4DBaseContextPtr>* GetContexts()
		{
			return &fContexts;
		}
		*/

		VJSObject* GetJSFunction(sLONG& ioFuncNum, const VString& inScript, const VectorOfVString* inParams);

		virtual void				SetJSContext(VJSGlobalContext* inJSContext);
		virtual VJSGlobalContext*	GetJSContext() const;

		virtual void FreeAllJSFuncs();

		virtual void SetExtraData( const VValueBag* inExtra);	// thread safe
		virtual const VValueBag* RetainExtraData() const;	// thread safe

				const VValueBag* GetExtraData() const	{ return fExtra;}	// not thread safe

		bool	NeedsToSendExtraData() const	{ return fNeedsToSendExtraData;}
		void	ClearNeedsToSendExtraData()		{ fNeedsToSendExtraData = false;}

		virtual void SetCurrentUser(const VUUID& inUserID, CUAGSession* inSession);

		virtual void CleanUpForReuse();

	protected:
		VUUID fID;
		//VArrayRetainedOwnedPtrOf<CDB4DBaseContextPtr> fContexts;
		ContextByBaseMap fContexts;
		VCriticalSection fMutex;
		VDBLanguageContext*	fLanguageContext;
		CUAGSession* fUserSession;
		VJSGlobalContext* fJSContext;
		bool fIsLocal;
		map<sLONG, VJSObject> fJSFuncs;
		sLONG fCurJSFuncNum;
		const VValueBag* fExtra;
		bool	fNeedsToSendExtraData;

		static sLONG sCurJSFuncNum;
};



class VDB4DAutoSeqNumber : public VComponentImp<CDB4DAutoSeqNumber>
{
	public:
		VDB4DAutoSeqNumber(AutoSeqNumber* seq);
		virtual ~VDB4DAutoSeqNumber();

		virtual const VUUID& GetID() const;

		virtual void SetStartingValue(sLONG8 initialvalue);
		virtual sLONG8 GetStartingValue() const;

		virtual void SetCurrentValue(sLONG8 currentvalue);
		virtual sLONG8 GetCurrentValue() const;

		virtual sLONG8 GetNewValue(DB4D_AutoSeqToken& ioToken);
		virtual void ValidateValue(DB4D_AutoSeqToken inToken, CDB4DTable* inTable, CDB4DBaseContext* context);
		virtual void InvalidateValue(DB4D_AutoSeqToken inToken);

		virtual VErrorDB4D Drop(CDB4DBaseContext* inContext = nil);

	protected:
		AutoSeqNumber* fSeq;
};

class CDB4DJournalData;
class VDB4DJournalData : public CDB4DJournalData
{
	public:
		VDB4DJournalData(sLONG8 globaloperation, uLONG inActionType, sLONG8 inContextID, const XBOX::VValueBag *inContextExtraData, RecordHeader *inRecHeader, uLONG8 inTimeStamp, sLONG8 inPos, VStream* dataStream, const VUUID& inTableID);
		VDB4DJournalData(sLONG8 globaloperation, uLONG inActionType, sLONG8 inContextID, const XBOX::VValueBag *inContextExtraData, uLONG8 inTimeStamp);
		VDB4DJournalData(sLONG8 globaloperation, uLONG inActionType, sLONG8 inContextID, const XBOX::VValueBag *inContextExtraData, uLONG8 inTimeStamp, sLONG inRecord, const VUUID& inTableID);
		VDB4DJournalData(sLONG8 globaloperation, uLONG inActionType, sLONG8 inContextID, const XBOX::VValueBag *inContextExtraData, uLONG8 inTimeStamp, sLONG8 inSeqNum, const VUUID& inTableID, bool forSeqNum);
		VDB4DJournalData(sLONG8 globaloperation, uLONG inActionType, sLONG8 inContextID, const XBOX::VValueBag *inContextExtraData, uLONG8 inTimeStamp, const VUUID& inUserID);
		VDB4DJournalData(sLONG8 globaloperation, uLONG inActionType, sLONG8 inContextID, const XBOX::VValueBag *inContextExtraData, uLONG8 inTimeStamp, sLONG inBlob, sLONG inBlobLen, const VUUID& inTableID, sLONG8 inPos, VStream* dataStream);

		virtual ~VDB4DJournalData();

		virtual uLONG	GetActionType() const;
		virtual uLONG8	GetTimeStamp() const;
		virtual sLONG8	GetGlobalOperationNumber() const;
		virtual Boolean	GetContextID( sLONG8 &outContextID ) const;
		virtual Boolean GetDataLen( sLONG &outDataLen ) const;
		virtual Boolean GetRecordNumber( sLONG &outRecordNumber ) const;
		virtual Boolean GetBlobNumber( sLONG &outBlobNumber, VString& outPath ) const;
		virtual Boolean GetSequenceNumber( sLONG8 &outSequenceNumber ) const;
		//virtual Boolean	GetTableIndex( sLONG &outIndex ) const;
		virtual Boolean GetTableID( VUUID& outID ) const;
		virtual Boolean GetUserID(VUUID& outID) const;
		virtual const VValueBag* GetExtraData() const;
		virtual Boolean GetCountFields( sLONG &outCountFields ) const;
		virtual VValueSingle* GetNthFieldValue(sLONG inFieldIndex, sLONG* outType, void* df);
		virtual void* GetDataPtr();
		virtual Boolean NeedSwap();
		virtual sLONG GetNthFieldBlobRef(sLONG inFieldIndex, VString& outPath);

		VError ReadBlob(Blob4D* blob);
		void SetPath(const VString& path)
		{
			fPath = path;
		}

	protected :
		void xinit(sLONG8 globaloperation, uLONG inActionType, sLONG8 inContextID, uLONG8 inTimeStamp);
		Boolean ReadDataFromPos();

		uLONG				fActionType;
		sLONG				fRecordNumber;
		//sLONG				fTableIndex;
		sLONG8				fContextID;
		uLONG8				fTimeStamp;
		RecordHeader		fRecHeader;
		VUUID				fUserID;
		VUUID				fTableID;
		void*				fDataPtr;
		sLONG8				fDataPosInStream;
		VStream*			fDataStream;
		sLONG				fBlobLen;
		sLONG8				fGlobalOperation;
		sLONG8				fLastSeqNum;
		const VValueBag*	fExtraData;
		VString				fPath;
};

class DB4DJournalParser;
class VDB4DJournalParser : public VComponentImp<CDB4DJournalParser>
{
	public:
		VDB4DJournalParser();
		virtual	~VDB4DJournalParser();

		virtual VError	Init(VFile* inJournalFile,uLONG8 &outTotalOperationCount, VDB4DProgressIndicator *inProgressIndicator);
		virtual VError	DeInit();

		virtual VError	SetCurrentOperation( uLONG8 inOperation, CDB4DJournalData **outJournalData );
		virtual VError	NextOperation( uLONG8 &outOperation, CDB4DJournalData **outJournalData );

		virtual	VError	SetEndOfJournal( uLONG8 inOperation );
		virtual uLONG8	CountOperations();

		virtual bool	IsValid( const VUUID &inDataLink );

		inline DB4DJournalParser* GetJournalParser() const { return fJournalParser; };
	protected:
		DB4DJournalParser *fJournalParser;
};



class VDB4DRawDataBase : public VComponentImp<CDB4DRawDataBase>
{
	public:
		VDB4DRawDataBase(Base4D_NotOpened* inBase);
		virtual ~VDB4DRawDataBase();

		virtual VErrorDB4D CheckAll(IDB4D_DataToolsIntf* inDataToolLog, DB4D_ToolsOptions* inOption = nil);

		virtual sLONG CountTables(Boolean& outInfoIsValid, IDB4D_DataToolsIntf* inDataToolLog);  // needs a CheckStruct_Elems before use
		virtual sLONG CountIndexes(Boolean& outInfoIsValid, IDB4D_DataToolsIntf* inDataToolLog);  // needs a CheckStruct_Elems before use
		virtual void GetTableName(sLONG inTableNum, VString& outName, Boolean& outInfoIsValid, IDB4D_DataToolsIntf* inDataToolLog);  // needs a CheckStruct_Elems before use
		virtual sLONG GetTables(std::vector<sLONG>& outTables,Boolean& outInfoIsValid,IDB4D_DataToolsIntf* inDataToolLog);  // needs a CheckStruct_Elems before use
		virtual VErrorDB4D GetTableIndexes(sLONG inTableNum, std::vector<sLONG>& outIndexes, Boolean& outInfoIsValid, IDB4D_DataToolsIntf* inDataToolLog);  // needs a CheckStruct_Elems before use
		virtual RecIDType CountRecordsInTable(sLONG inTableNum, Boolean& outInfoIsValid, IDB4D_DataToolsIntf* inDataToolLog);  // needs a CheckStruct_Elems before use
		virtual sLONG CountFieldsInTable(sLONG inTableNum, Boolean& outInfoIsValid, IDB4D_DataToolsIntf* inDataToolLog);  // needs a CheckStruct_Elems before use
		virtual VErrorDB4D GetTableFields(sLONG inTableNum, std::vector<sLONG>& outFields, Boolean& outInfoIsValid, IDB4D_DataToolsIntf* inDataToolLog);  // needs a CheckStruct_Elems before use

		virtual VErrorDB4D GetFieldInfo(sLONG inTableNum, sLONG inFieldNum, VString& outName, sLONG& outType, std::vector<sLONG>& outIndexes, Boolean& outInfoIsValid, IDB4D_DataToolsIntf* inDataToolLog);
		virtual VErrorDB4D GetIndexInfo(sLONG inIndexNum,XBOX::VString& outName,sLONG& outTableNum,std::vector<sLONG>& outFields, Boolean& outInfoIsValid, IDB4D_DataToolsIntf* inDataToolLog);

		virtual bool CheckStructAndDataUUIDMatch(IDB4D_DataToolsIntf* inDataToolLog);

		virtual VErrorDB4D CheckStruct_DataSegs(IDB4D_DataToolsIntf* inDataToolLog);
		virtual VErrorDB4D CheckStruct_Elems(IDB4D_DataToolsIntf* inDataToolLog); // needs to be performs if you want to scan individual tables of indexes
		virtual VErrorDB4D CheckStruct_RecordsAndIndexes(IDB4D_DataToolsIntf* inDataToolLog);
		virtual VErrorDB4D CheckStruct_All(IDB4D_DataToolsIntf* inDataToolLog, DB4D_ToolsOptions* inOption = nil); // regroups all other CheckStruct methods

		virtual VErrorDB4D CheckData_DataSegs(IDB4D_DataToolsIntf* inDataToolLog);
		virtual VErrorDB4D CheckData_Elems(IDB4D_DataToolsIntf* inDataToolLog);
		virtual VErrorDB4D CheckData_OneTable(sLONG inTableNum, IDB4D_DataToolsIntf* inDataToolLog);
		virtual VErrorDB4D CheckData_OneIndex(sLONG inIndexNum, IDB4D_DataToolsIntf* inDataToolLog);
		virtual VErrorDB4D CheckData_All(IDB4D_DataToolsIntf* inDataToolLog, DB4D_ToolsOptions* inOption = nil); // regroups all other CheckData methods

		virtual VErrorDB4D CompactInto(CDB4DBase* outCompactedBase, IDB4D_DataToolsIntf* inDataToolLog, Boolean withIndexes = true, 
										Boolean WithStruct = true, Boolean WithData = true, Boolean EraseOrphanTables = false);

		//virtual VErrorDB4D RecoverByTags(CDB4DBase* outRecoveredBase, IDB4D_DataToolsIntf* inDataToolLog, Boolean WithStruct = true, Boolean WithData = true);

		VIntlMgr *SwitchIntlMgr();

protected:
		Base4D_NotOpened* fBase;
};


// *******************************************obsolete *********************************************************





#endif
