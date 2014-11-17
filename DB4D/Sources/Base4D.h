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
#ifndef __BASE4D__
#define __BASE4D__

#include "DataSeg.h"
#include <map>

#if FORMAC
#pragma segment Base4D
#endif

#define Max4DFiles 32000
#define MaxFields 32000

const sLONG kDefaultBufferSize = 0x10000;

const uLONG tagHeader4D = 0x44014402;
const RecIDType kMaxRecordsInTable = 0x3FFFFFFF;
const RecIDType kMaxBlobsInTable = 0x3FFFFFFF;
const RecIDType kMaxObjsInTable = 0x3FFFFFFF;
const sLONG kMaxTables = 32767;
const sLONG kMaxFields = 32767;
const sLONG kMaxFieldsInCompositeIndex = 200;

enum { startinstruct, startinuser, startinmenu };

enum { number_cadran, thermo_cadran  };

enum { pasdedefault_proc, listing_proc, organigrame_proc };

enum { GroupStruct = 0, GroupCompilo, GroupApi };

#define NbSpecialGroup 3

/*
struct StructHeader
{  
	sLONG numversion;
	sLONG nbfich;
};

typedef StructHeader *StructHeaderPtr, **StructHeaderHandle;
*/

class Base4D;
class SynchroBaseHelper;
class ModelErrorReporter;



typedef map<VString, VRefPtr<CDB4DSchema> > SchemaCatalogByName;
typedef map<DB4D_SchemaID, VRefPtr<CDB4DSchema> > SchemaCatalogByID;



typedef Boolean SmallArrayOfBoolean[2048];


class JoinRefCouple
{
	public:
		Boolean operator == (const JoinRefCouple& other) const 
		{ 
			return		(	(fNumTable1 == other.fNumTable1 && fNumField1 == other.fNumField1 && fNumInstance1 == other.fNumInstance1)
						&&	(fNumTable2 == other.fNumTable2 && fNumField2 == other.fNumField2 && fNumInstance2 == other.fNumInstance2) )
					||
						(	(fNumTable2 == other.fNumTable1 && fNumField2 == other.fNumField1 && fNumInstance2 == other.fNumInstance1)
						&&	(fNumTable1 == other.fNumTable2 && fNumField1 == other.fNumField2 && fNumInstance1 == other.fNumInstance2) );

		};

		sLONG fNumTable1;
		sLONG fNumField1;
		sLONG fNumInstance1;
		sLONG fNbOtherFields1;
		sLONG fOtherFields1[10];
		sLONG fNumTable2;
		sLONG fNumField2;
		sLONG fNumInstance2;
		sLONG fNbOtherFields2;
		sLONG fOtherFields2[10];
};


class JoinPath: public VObject
{
	public:
		//inline Boolean IsEqualTo(JoinPath* other) const { return fPath[0].fNumTable2 == other->fPath[0].fNumTable2; };
		//inline Boolean IsLessThan(JoinPath* other) const { return fPath[0].fNumTable2 < other->fPath[0].fNumTable2; };

		Boolean AllReadyIn(sLONG xnumtable1, sLONG xnumfield1, sLONG xnumtable2, sLONG xnumfield2, sLONG xnuminstance1, sLONG xnuminstance2);
		inline void Clear() { fPath.clear(); };
		inline sLONG GetCount() const { return (sLONG)fPath.size(); };
		inline JoinRefCouple* GetCouple(sLONG n) { return &fPath[n]; };
		inline JoinRefCouple& GetCoupleRef(sLONG n) { return fPath[n]; };
		VError AddJoin(sLONG xnumtable1, sLONG xnumfield1, sLONG xnumtable2, sLONG xnumfield2, sLONG xnuminstance1, sLONG xnuminstance2);
		VError AddJoin(const FieldArray& sources, const FieldArray& dests, sLONG xnuminstance1, sLONG xnuminstance2);
		VError AddJoin(JoinRefCouple& joinref);
		sLONG FindTarget(sLONG from, sLONG numtable, SmallArrayOfBoolean &Deja, sLONG numinstance);
		inline void RemoveLast() { fPath.pop_back(); };
		inline void RemoveFirst()
		{
			fPath.erase(fPath.begin());
		}

		inline void CopyFrom(const JoinPath& inPath) { fPath = inPath.fPath; };

	protected:
		vector<JoinRefCouple> fPath;

};


class ReadAheadBuffer : public VObject, public IRefCountable, public IChainable<ReadAheadBuffer>
{
	public:
		ReadAheadBuffer(Base4D* owner);
		virtual ~ReadAheadBuffer();
		Boolean BuildDataPtr(sLONG size);
		inline DataAddr4D GetPos() { return fPos; };
		inline sLONG GetLen() { return fDataLen; };
		inline sLONG GetMaxLen() { return fDataMaxLen; };
		inline sLONG GetSegment() { return fSegment; };
		VError GetData(void* p, sLONG len, DataAddr4D ou, sLONG offset);
		VError ReadFrom(DataAddr4D ou, sLONG len);
		void Lock();
		void Unlock();
		inline Boolean TryToLock() { return fmutex.TryToLock(); };
		void Invalide() { fDataLen = 0; fSegment = -1; };

	protected:
		Base4D* fOwner;
		void* fData;
		sLONG fDataLen, fDataMaxLen;
		sLONG fSegment;
		DataAddr4D fPos;
		VCriticalSection fmutex;
};


class BaseTaskInfo;

typedef uCHAR FillBackStamp[512];

class BaseFlushInfo;
class Table;
class Field;
class VDBFlushMgr;
class Relation;


class StructElemDef : public ObjCacheInTree
{
	public:
		StructElemDef(Base4D* owner, sLONG num, DataBaseObjectType signature, sWORD DefaultAccess = StructElemDefAccess);
		virtual ~StructElemDef();
		inline sLONG GetNum() const { return fNum; };
		inline void SetNum(sLONG num) { fNum = num; };

		inline void* GetDataPtr() const { return fData; };
		inline sLONG GetDataLen() const { return fDataLen; };
		inline sLONG GetAnteLen() const { return fAnteLen; };
		inline Boolean NeedSwap() const { return fNeedSwap; };
		inline void DoneSwap() { fNeedSwap = false; };

		inline void SetAnteLen() { fAnteLen = fDataLen; };
		void SetData(void* data, sLONG datalen);
		VError CopyData(void* data, sLONG datalen);

		virtual void RecalcNbElemTab() { xbox_assert(false); nbelemtab = 0; /* should not be called*/ };
		virtual sLONG liberemem(sLONG allocationBlockNumber, sLONG combien=-1, uBOOL tout=false);
		virtual sLONG saveobj();
		VError loadobj(DataAddr4D xaddr, sLONG len);

		inline void SetSwap(Boolean b) { fNeedSwap = b; };
		inline void SetSwapWhileSaving(Boolean b) { fNeedSwapWhileSaving = b; };

	protected:
		Base4D* fOwner;
		sLONG fNum;
		void* fData;
		sLONG fDataLen;
		sLONG fAnteLen;
		Boolean fNeedSwap;
		Boolean fNeedSwapWhileSaving;
		DataBaseObjectType fSignature;
};


class LocalEntityModelCatalog;
class EntityModel;
class EntityModelCatalog;



class StructElemDefTreeInMem : public TreeInMem
{
public:
	inline StructElemDefTreeInMem(sWORD DefaultAccess, typobjcache typ /* = t_Obj4D*/):TreeInMem(DefaultAccess,typ, true) {;};
protected:
	virtual TreeInMem* CreTreeInMem();
	virtual void DeleteElem( ObjCacheInTree *inObject);
};

typedef vector<VRefPtr<IndexInfo> > IndexRefCollection;

typedef map<pair<sLONG, sLONG>, JoinPath> CachedRelationPathCollection;

typedef vector<EntityModelCatalog*> EntityModelCatalogCollection;
typedef map<VString, EntityModelCatalog*, CompareLessVStringStrict> EntityModelCatalogsByName;

class StructElemDefTreeInMemHeader : public TreeInMemHeader
{
protected:
	virtual TreeInMem* CreTreeInMem();
};

class DB4DJournalParser;

typedef map<VUUID, AutoSeqNumber* /*, less<VUUID>, cache_allocator<pair<const VUUID, AutoSeqNumber*> > */ > MapOfSeqNum;
typedef map<VUUID, IndexInfo* /*, less<VUUID>, cache_allocator<pair<const VUUID, IndexInfo*> > */> IndexMap;
typedef map<VTaskID, sLONG /*, less<VTaskID>, cache_allocator<pair<const VTaskID, sLONG> > */> CountMap;

typedef map<VUUID, Obj4DContainer /*, less<VUUID>, cache_allocator<pair<const VUUID, Obj4DContainer> > */> MapOfObj4D;

typedef vector<DataTableRegular* /*, cache_allocator<DataTableRegular*> */> DataTableVector;

typedef map<VUUID, sLONG /*, less<VUUID>, cache_allocator<pair<const VUUID, sLONG> > */> TableRefMap;
typedef map<VString, sLONG/*, less<VString>, cache_allocator<pair<const VString, sLONG> > */> TableNameMap;

typedef map<uLONG8, BaseTaskInfo*> ContextMap;



class logEntryRef
{
public:
	DB4D_LogAction logAction;
	sLONG tablenum;
	sLONG recnum;
};

typedef vector<logEntryRef> logEntryRefVector;

class IBackupSettings;

class mapFieldsIDInRecByName : public IDebugRefCountable
{
	public:
		mapFieldsIDInRecByName(Base4D* inOwner);

		VError save();
		VError load();

		sLONG GetSmallID(const VString& inRef);


	protected:
		typedef unordered_map_VString<sLONG> mapFieldsID;

		Base4D* fOwner;
		mapFieldsID fFieldsID;
		sLONG fStartingID;
		VCriticalSection fMutex;

};

class Base4D : public ObjCache
{
friend class DataTools;
friend class ScanTask;
friend class Base4D_NotOpened;

public:
	Base4D(VDBFlushMgr* Flusher, bool inNeedFlushInfo);
	Base4D(Base4D* localdb, const VUUID& BaseID, DB4DNetworkManager* netacces, DB4DNetworkManager* legacy_netacces);
	virtual ~Base4D();


#if debugObjInTree_Strong
	virtual	void DoOnRefCountZero () // from IRefCountable
	{
		ObjCacheInTree::DisableCheck();
		IRefCountable::DoOnRefCountZero();
		ObjCacheInTree::EnsableCheck();
	}
#endif

#if debugLeaksAll
	virtual bool OKToTrackDebug() const
	{
		return true;
	}

	virtual void GetDebugInfo(VString& outText) const
	{
		VString s;
		GetName(s);
		outText = "base4D : "+s;
	}

#endif

	inline void GetUUID(VUUID& outID) const { outID = fID; };
	inline const VUUID& GetUUID() const { return fID; };

	VError OpenRemote(Boolean CheckForUpdateOnly = false);

	VError BuildAndLoadFirstStageDBStruct( const VFile& inStructureFile, sLONG inParameters, FileAccess inAccess);
	VError OpenStructure( const VFile& inStructureFile, sLONG inParameters, FileAccess inAccess, VString* EntityFileExt = nil, CUAGDirectory* inUAGDirectory = nil, 
							const VString* inXmlContent = nil, unordered_map_VString<VRefPtr<VFile> >* outIncludedFiles = nil, const VFile* inPermissionsFile = nil);

	VError BuildAndCreateFirstStageDBStruct( const VFile& inStructureFile, sLONG inParameters, VIntlMgr* inIntlMgr, FileAccess inAccess, const VUUID* inChosenID = nil);
	VError CreateStructure( const VFile& inStructureFile, sLONG inParameters, VIntlMgr* inIntlMgr, FileAccess inAccess, const VUUID* inChosenID = nil, 
							VString* EntityFileExt = nil, CUAGDirectory* inUAGDirectory = nil, const VFile* inPermissionsFile = nil);

	VError CreateResMap();

	VError CloseStructure();

	VError OpenData( const VFile& inFile, sLONG inParameters, Boolean BuildResMap = false, Boolean KeepFlushInfoIfClose = false, FileAccess inAccess = FA_READ_WRITE);
	VError CreateData( const VFile& inFile, sLONG inParameters, VIntlMgr* inIntlMgr, VFile *inJournalingFile = NULL, Boolean BuildResMap = false, 
						FileAccess inAccess = FA_READ_WRITE, const VUUID* inChosenID = nil, CDB4DRawDataBase* fromData = nil );
	VError CloseData(Boolean KeepFlushInfo = false);
	void GetDataSegPath(sLONG inDataSegNum, VFilePath& outPath);
	sLONG CountDataSegs() { return fSegments.GetCount(); };
	VFolder* RetainDataFolder();

	VError LoadIndexesAfterCompacting(sLONG inParameters);
	
	VError OpenIndexSegment(const VFile& inFile, Boolean isAStruct, FileAccess inAccess);
	VError CreateIndexSegment(const VFile& inFile);
	inline SegData* GetIndexSeg() const { return fIndexSegment; };

	VFile *RetainIndexFile() const;
	VFile *RetainDefaultDataFile() const;
	
	VError CloseBase();

	VError CheckIndexesOnSubTables();

	void DisposeRecoverInfo(void* xrecover);
	VError ScanToRecover(VFile* inOldDataFile, VValueBag& outBag, void* &outRecover, ToolLog *log = nil);
	VError RecoverByTags(VValueBag& inBag, void* inRecover, ToolLog *log = nil);

	Boolean IsStructureOpened() const;
	Boolean IsDataOpened() const;
	Boolean AreIndexesDelayed() const;
	Boolean CheckIfIndexesAreDelayed() const;
	
	VError SetName( VString& inName, CDB4DBaseContext* inContext, bool cantouch);
	void GetName( VString& outName) const; // return the base name (defaults to the file name if any)

	VError FromBag(const VValueBag& inBag, EntityModelCatalog* catalog);
	VError PutIndexDefIntoBag(VValueBag& ioBag) const;
	//VError LoadIndicesFromBag(const VValueBag* inBag);

	void SetRetainedBackupSettings(IBackupSettings* inSettings);

	const IBackupSettings* RetainBackupSettings()const;
	
	VError	LoadFromBag( const VValueBag& inBag, VBagLoader *inLoader);
	VError	SaveToBag( VValueBag& ioBag, VString& outKind) const;
	VError	SaveToBagDeep( VValueBag& ioBag, VString& outKind, bool inWithTables, bool inWithIndices, bool inWithRelations) const;

	sLONG CreTable( const VString& name, const CritereDISK* from, sLONG nbcrit, VError& err, CDB4DBaseContext* inContext, sLONG xnum = 0, VUUID* xID = nil);
	Table* RetainTable(sLONG numfile) const;
//	DataTable* GetNthDataFile( sLONG numfile) const;

	Field* RetainField(sLONG numfile, sLONG numcrit) const;

	VError LoadTable( const VValueBag& inBag, VBagLoader *inLoader, CDB4DBaseContext* inContext);
	VError LoadAllTableDefs();
	VError	CreateTable( const VValueBag& inBag, VBagLoader *inLoader, Table **outTable, CDB4DBaseContext* inContext, Boolean inGenerateName);
	VError	AddTable( Table *inTable, bool inWithNamesCheck, CDB4DBaseContext* inContext, bool inKeepUUID = false, bool inGenerateName = false, bool BuildDataTable = true, sLONG inPos = 0, bool cantouch = true, const VUUID *inID = nil);
	VError	GetNameForNewTable( VString& outName) const;

	void ReleaseAllTables();
	void ClearAllTableDependencies(bool includingRelations);

	Table* FindOrCreateTableRef(const VString& tablename, VError& err, const VUUID& xid);
	
	//VError CreDataFile(Table* crit, Boolean atTheEnd = true);

	DataTable* CreateDataTable(Table* Associate, VError& err, sLONG prefseg = 0, DataTableDISK* dejaDFD = nil, sLONG dejapos = -1);
	DataTable* FindDataTableWithTableUUID(const VUUID& inTableID);

	//sLONG AddTable( const VString& name, const CritereDISK* from, sLONG nbcrit, CDB4DBaseContext* inContext, bool cantouch);

	VError writelong(void* p, sLONG len, DataAddr4D ou, sLONG offset, VString* WhereFrom = nil, sLONG TrueLen = 0);
	VError readlong(void* p, sLONG len, DataAddr4D ou, sLONG offset, ReadAheadBuffer* buffer = nil);
	
	uBOOL IsAddrValid(DataAddr4D ou);

	bool CanFindDiskSpace(VSize nbBytesToFind);

	DataAddr4D findplace(sLONG len, BaseTaskInfo* context, VError& err, sLONG prefseg=0, void *obj = nil);
	VError libereplace(DataAddr4D ou, sLONG len, BaseTaskInfo* context, void *obj = nil); 
	void MarkBlockToFullyDeleted(DataAddr4D ou, sLONG len);

	void SwapAllSegsFullyDeletedObjects(DataAddrSetVector& AllSegsFullyDeletedObjects);

	void FlushBase();
	
	virtual void setmodif(uBOOL xismodif, Base4D* bd, BaseTaskInfo* context);

	virtual sLONG saveobj();
	virtual uBOOL okdel();
	
	//void SetSaveIndexes();

	void RemoveDependenciesFromIndex(const VUUID& indexid);
	/*
	sLONG FindIndexRef(IndexInfo *xind) const;
	sLONG FindIndex(IndexInfo *xind, Boolean MatchTyp = true) const;
	*/

	//IndexInfo* CheckIndexOccupeAndvalid(IndexInfo *xind, Boolean MustBeValid, BaseTaskInfo* context, Boolean Occupy) const;

	IndexInfo* FindAndRetainIndex(IndexInfo *xind, Boolean MatchTyp = true) const;
	//IndexInfo* FindAndRetainIndex(FieldsArray* fields, sLONG NbFields, Boolean MustBeValid = true, BaseTaskInfo* context = nil, Boolean Occupy = false) const;
	IndexInfo* FindAndRetainIndexSimple(sLONG numtable, sLONG nc, uBOOL sortable, Boolean MustBeValid = true, BaseTaskInfo* context = nil, Boolean Occupy = false) const;
	IndexInfo* FindAndRetainIndexLexico(sLONG numtable, sLONG nc, Boolean MustBeValid = true, BaseTaskInfo* context = nil, Boolean Occupy = false) const;
	IndexInfo* FindAndRetainIndexByName(const VString& inIndexName, Boolean MustBeValid = true, BaseTaskInfo* context = nil, Boolean Occupy = false) const;
	IndexInfo* FindAndRetainIndexByUUID( const VUUID& inUUID, Boolean MustBeValid = true, BaseTaskInfo* context = nil, Boolean Occupy = false) const;

	IndexInfo* FindAndRetainAnyIndex(IndexInfo *xind, Boolean MustBeSortable, Boolean MustBeValid, BaseTaskInfo* context = nil, Boolean Occupy = false) const;

	//IndexInfo* RetainIndex( sLONG inIndexNum) const;
	VError	CreateIndex( const VValueBag& inBag, VBagLoader *inLoader, IndexInfo **outIndex, CDB4DBaseContext* inContext, bool loadonly = false);

	VError SubCreIndex(IndexInfo* ind, CDB4DBaseContext* inContext, VDB4DProgressIndicator* InProgress,
						const VString* inName, IndexInfo **outIndex, Boolean ForceCreate, VSyncEvent* event);

	VError CreFullTextIndexOnField(Field* target, CDB4DBaseContext* inContext, VDB4DProgressIndicator* InProgress = nil,
									const VString* inName = nil,	IndexInfo **outIndex = nil, Boolean ForceCreate = true, VSyncEvent* event = nil);
	VError CreFullTextIndexOnField(sLONG nf, sLONG nc, CDB4DBaseContext* inContext, VDB4DProgressIndicator* InProgress = nil,
									const VString* inName = nil,	IndexInfo **outIndex = nil, Boolean ForceCreate = true, VSyncEvent* event = nil);

	VError DeleteFullTextIndexOnField(sLONG nf, sLONG nc, CDB4DBaseContext* inContext, VDB4DProgressIndicator* InProgress = nil, VSyncEvent* event = nil);
	VError DeleteFullTextIndexOnField(Field* target, CDB4DBaseContext* inContext, VDB4DProgressIndicator* InProgress = nil, VSyncEvent* event = nil);

	VError CreIndexOnField(Field* target, sLONG typindex, Boolean UniqueKeys, CDB4DBaseContext* inContext, VDB4DProgressIndicator* InProgress = nil,
							const VString* inName = nil,	IndexInfo **outIndex = nil, Boolean ForceCreate = true, VSyncEvent* event = nil);
	VError CreIndexOnField(sLONG nf, sLONG nc, sLONG typindex, Boolean UniqueKeys, CDB4DBaseContext* inContext, VDB4DProgressIndicator* InProgress = nil,
							const VString* inName = nil,	IndexInfo **outIndex = nil, Boolean ForceCreate = true, VSyncEvent* event = nil);

	VError DeleteIndexOnField(sLONG nf, sLONG nc, CDB4DBaseContext* inContext, sLONG typindex = 0, VDB4DProgressIndicator* InProgress = nil, VSyncEvent* event = nil);
	VError DeleteIndexOnField(Field* target, CDB4DBaseContext* inContext, sLONG typindex = 0, VDB4DProgressIndicator* InProgress = nil, VSyncEvent* event = nil);
	
	VError CreIndexOnFields(FieldNuplet *fields, sLONG typindex, Boolean UniqueKeys, CDB4DBaseContext* inContext, VDB4DProgressIndicator* InProgress = nil,
							const VString* inName = nil,	IndexInfo **outIndex = nil, Boolean ForceCreate = true, VSyncEvent* event = nil);
	VError DeleteIndexOnFields(FieldNuplet *fields, CDB4DBaseContext* inContext, sLONG typindex = 0, VDB4DProgressIndicator* InProgress = nil, VSyncEvent* event = nil);
	
	void DeleteIndexByRef(IndexInfo* ind, CDB4DBaseContext* inContext, VDB4DProgressIndicator* InProgress = nil, VSyncEvent* event = nil);
	void RebuildIndexByRef(IndexInfo* ind, CDB4DBaseContext* inContext, VDB4DProgressIndicator* InProgress = nil, VSyncEvent* event = nil);

///	inline DataTable* GetSpecialDataFile() { return( &special_pour_index ); };
			
	void FindField( const VString& pFileName, const VString& pFieldName, sLONG *pFileNumber, sLONG *pFieldNumber) const;
	sLONG FindTable( const VString& pFileName) const;

	Table* FindAndRetainTableRef( const VUUID& pRefID) const;
	Table* FindAndRetainTableRef( const VString& pFileName) const;
	Table *FindAndRetainTableRef( const VValueBag& inRef, VBagLoader *inLoader) const;

	Field* FindAndRetainFieldRef( const VUUID& pRefID) const;
	Field* FindAndRetainFieldRef( const VString& pFileName, const VString& pFieldName) const;
	Field* FindAndRetainFieldRef( const VString& pFieldName) const; // form : "Table.Field"
	Field* FindAndRetainFieldRef( const VValueBag& inRef, VBagLoader *inLoader) const;

	//Table* FindAndRetainTableByRecordCreator(const CMethod* method) const;
	Table* FindAndRetainTableRefByRecordName( const VString& pRecName) const;

	inline sLONG GetNBTable() const { return fNbTable; };
	sLONG GetNBTableWithoutDeletedTables() const;
	VError TakeOffDeletedTables(Bittab *tb) const;

	
	DataAddr4D GetMultiSegHeaderAddress() const;
	
	void LockIndexes() const ;
	void UnLockIndexes() const ;
	
	sLONG GetBaseHeaderSize() const;
	DataAddr4D GetBaseProtectedSize() const
	{
		return fBaseHeaderSize + sizetfb + sizefbmaxlibre;
	}
	
	BaseFlushInfo* GetDBBaseFlushInfo(void) { return fFlushInfo; };

	void RegisterForLang(void);
	void UnRegisterForLang(void);

//	static VError DatabasePushMember (CLanguageContext* inContext, VClassInstance& ioClassInstance, CVariable* inVariable, StackPtr* ioStack);
//	static VError DatabasePopMember  (CLanguageContext* inContext, VClassInstance& ioClassInstance, VVariableStamp* ptStamp, CVariable* inVariable, StackPtr* ioStack);
	
//	inline CNameSpace* GetNameSpace(void) { return BaseNameSpace; };
//	inline CClass* GetBaseRef(void) { return BaseRef; };
	
	void KillIndex(const VUUID& indexid, VDB4DProgressIndicator* InProgress); // destinee a etre appelee par la tache IndexBuilder
	void BuildIndex(IndexInfo *Ind, VDB4DProgressIndicator* InProgress); // destinee a etre appelee par la tache IndexBuilder
	void ReBuildIndex(IndexInfo *Ind, VDB4DProgressIndicator* InProgress); // destinee a etre appelee par la tache IndexBuilder
	VError KillDataFile(sLONG TableNumber, VDB4DProgressIndicator* InProgress, VSyncEvent* event, bool mustFullyDelete,sLONG StructTableNumber);
		
	inline void SetNotifyState(Boolean inNotifyState) { fIsNotifying = inNotifyState; };
	inline Boolean IsNotifying() { return fIsNotifying; };
	
	//inline sLONG CountIndices() const { return lbi.GetCount(); };
	
	Boolean DeleteTable(Table* inFileToDelete, CDB4DBaseContext* inContext, VDB4DProgressIndicator* InProgress = nil, VSyncEvent* event = nil, Boolean onlylocal = false);
	// sLONG FindNextFileFree();
	
	VFolder*	RetainTemporaryFolder( bool inMustExists = true, VError *outError = nil) const;
	void		SetTemporaryFolder( VFolder *inFolder);
	VError		SelectTemporaryFolder( bool inForCombinedDatabase);

	VError		SetTemporaryFolderSelector( DB4DFolderSelector inSelector, const XBOX::VString *inCustomFolder = nil);
	VError		GetTemporaryFolderSelector( DB4DFolderSelector *outSelector, XBOX::VString& outCustomFolderPath) const;

	VError ThrowError( VError inErrCode, ActionDB4D inAction, const VString* p1 = nil) const;

	inline Boolean CanRegister() { return fIsRemote || fStructure!=nil; };

	uBOOL Lock(BaseTaskInfo* Context, bool specialFlushAndLock, Boolean WaitForEndOfRecordLocks = false, sLONG TimeToWaitForEndOfRecordLocks = 0);
	void UnLock(BaseTaskInfo* Context);

	uBOOL FlushAndLock(BaseTaskInfo* Context, Boolean WaitForEndOfRecordLocks = false, sLONG TimeToWaitForEndOfRecordLocks = 0);
	
	Boolean ValidAddress(DataAddr4D adr, sLONG len);

	//sLONG FindNextFreeRelation(VError& err) const;
	Relation* RetainRelation( sLONG inRelationIndex) const;		// inRelationIndex is 0 based
	Relation* FindAndRetainRelationByName(const VString& Name) const;
	Relation* FindAndRetainRelation(const Field* source, const Field* dest) const;
	Relation* FindAndRetainRelation(const FieldArray& sources, const FieldArray& dests) const;
	Relation* FindAndRetainRelationByRelVar(const CVariable* RefToSearch) const;
	Relation* FindAndRetainRelationByUUID(const VUUID &ID) const;
	Relation* CreateRelation(const VString &name, const VString &oppositename, Field* source, Field* dest, VError &err, CDB4DBaseContext* inContext);
	Relation* CreateRelation(const VString &name, const VString &oppositename, const CDB4DFieldArray& inSourceFields, 
								const CDB4DFieldArray& inDestinationFields, VError &err, CDB4DBaseContext* inContext);
	VError	CreateRelation( const VValueBag& inBag, VBagLoader *inLoader, Relation **outRelation, CDB4DBaseContext* inContext);
	VError	LoadRelation( const VValueBag& inBag, VBagLoader *inLoader, CDB4DBaseContext* inContext);
	VError AddRelation( Relation *inRelation, bool inWithNamesCheck, CDB4DBaseContext* inContext);
	VError DeleteRelation(Relation* RelationToDelete, CDB4DBaseContext* inContext, Boolean inOnlyLocal = false);
	void DeleteBadRelations();
	VError GetAndRetainRelations(VArrayRetainedOwnedPtrOf<Relation*> &outRelations) const;
	inline void NeedToSaveRelations() { fNeedsToSaveRelation = true; };

	CDB4DBase* RetainStructDatabase(const char* DebugInfo) const;
	inline Base4D* GetStructure() const { return fStructure; };

	Boolean	CreateElements( const VValueBag& inBag, VBagLoader *inLoader, CDB4DBaseContext* inContext);
	Boolean	LoadElements( const VValueBag& inBag, VBagLoader *inLoader, CDB4DBaseContext* inContext);

	ReadAheadBuffer* NewReadAheadBuffer(sLONG size = kDefaultBufferSize);
	void RemoveReadAheadBuffer(ReadAheadBuffer* buffer);

	StructElemDef* LoadStructElemDef(sLONG num, VError &err, AddrTableHeader& StructElemDefTabAddr, 
									StructElemDefTreeInMemHeader& StructElemDefInMem, sLONG& maxelem, DataBaseObjectType signature);

	VError SaveStructElemRef(StructElemDef* elem, AddrTableHeader& StructElemDefTabAddr, 
							StructElemDefTreeInMemHeader& StructElemDefInMem, sLONG& maxelem, DataBaseObjectType signature);

	VError DeleteStructElemDef(sLONG num, AddrTableHeader& StructElemDefTabAddr, StructElemDefTreeInMemHeader& StructElemDefInMem, sLONG& maxelem);
	VError ResizeTableOfStructElemDef(sLONG nbEntries, AddrTableHeader& StructElemDefTabAddr, StructElemDefTreeInMemHeader& StructElemDefInMem, sLONG& maxelem);
	VError NormalizeTableOfStructElemDef(AddrTableHeader& StructElemDefTabAddr, StructElemDefTreeInMemHeader& StructElemDefInMem, sLONG& maxelem);

	StructElemDef* LoadTableDefInDatas(sLONG num, VError &err);
	VError SaveTableRefInDatas(StructElemDef* elem);
	VError DeleteTableDefInDatas(sLONG num);

	StructElemDef* LoadTableDef(sLONG num, VError &err);
	VError SaveTableRef(StructElemDef* elem);
	VError DeleteTableDef(sLONG num);
	VError ResizeTableOfTableDef(sLONG nbEntries);
	VError NormalizeTableOfTableDef();

	StructElemDef* LoadRelationDef(sLONG num, VError &err);
	VError SaveRelationDef(StructElemDef* elem);
	VError DeleteRelationDef(sLONG num);

	StructElemDef* LoadIndexDef(sLONG num, VError &err);
	VError SaveIndexDef(StructElemDef* elem);
	VError DeleteIndexDef(sLONG num);

	StructElemDef* LoadIndexDefInStruct(sLONG num, VError &err);
	VError SaveIndexDefInStruct(StructElemDef* elem);
	VError DeleteIndexDefInStruct(sLONG num);


	CDB4DBase* RetainBaseX(const char* DebugInfo = 0) const;
	void BeginClose(VSyncEvent* waitclose = nil);

	inline Boolean IsClosing() const { return fIsClosing; };

	//VError SaveRelations();
	VError DisposeRelations();

	VError SetSeqNumAddr(sLONG SeqNumNum, DataAddr4D addr, sLONG len, sLONG &outSeqNumNum);
	AutoSeqNumber* AddSeqNum(DB4D_AutoSeq_Type typ, VError& err, CDB4DBaseContext* inContext);
	AutoSeqNumber* RetainSeqNum(const VUUID& IDtoFind);
	VError DeleteSeqNum(const VUUID& IDtoDelete, CDB4DBaseContext* inContext);

	VFile* RetainJournalFile() const;
	VError SetJournalFile( VFile* inNewLog, const VUUID *inDataLink = NULL, bool inResetJournalSequenceNumber = false);
	VError SetJournalFileInfos( VString *inFilePath, VUUID *inUUID, bool inResetJournalSequenceNumber);
	//VError ResetJournalFileContent();
	VStream* GetLogStream() const;
	VError WriteLog(DB4D_LogAction action, BaseTaskInfo* context, bool SignificantAction);
	VError StartWriteLog(DB4D_LogAction action, sLONG len, BaseTaskInfo* context, VStream* &outLogStream, bool SignificantAction, bool mutextAlreadyLocked = false);
	VError EndWriteLog(sLONG len, bool keepMutexLocked = false);

	JournalPool* GetJournalManager()
	{
#if NewJournal
		return &fLogPool;
#else
		return nil;
#endif
	}

	bool LogIsOn() const
	{
#if NewJournal
		return fLogPool.LogIsOn();
#else
		fLogMutex.Lock();
		return fLogStream != nil; // The mutex remains locked on purpose
#endif
	}

	void ReleaseLogMutex()
	{
#if NewJournal
		fLogPool.ReleaseLogMutex();
#else
		fLogMutex.Unlock();
#endif
	}

	const VValueBag* RetainStructureExtraProperties(VError &err, CDB4DBaseContext* inContext) const;
	VError SetStructureExtraProperties(VValueBag* inExtraProperties, bool inNotify, CDB4DBaseContext* inContext);

	const VValueBag* RetainExtraProperties(VError &err, CDB4DBaseContext* inContext);
	VError SetExtraProperties(VValueBag* inExtraProperties, bool inNotify, CDB4DBaseContext* inContext, bool loadonly = false);

	inline sLONG getSDDoffset() { return ((char*)&hbbloc.seginfo) - (char*)&hbbloc; };
	
#if debuglog
	void WriteDebugLogData(void* p, sLONG len);
	void WriteDebugLogStr(const VString& s);
	void WriteDebugLogLong(sLONG n);
	void WriteDebugLogAddr(DataAddr4D addr);
	void WriteDebugLogLn();
	void WriteDebugLogLn(const VString& mess);
	void WriteDebugHex(void* p, sLONG len);
	void WriteDebugFlush();
#endif

	SegData* GetSpecialSegment(sLONG SegNum) const;

	void DelayAllIndexes();
	void AwakeAllIndexes(VDB4DProgressIndicator* inProgress, Boolean WaitForCompletion, vector<IndexInfo*>& outIndexList);

	Boolean OkToUpdate(VError& err);
	void ClearUpdating();

	DB4DTriggerManager* SetTriggerManager(DB4DTriggerManager* inNewTriggerManager);
	inline DB4DTriggerManager* GetTriggerManager() const { return fTriggerManager; };
	Boolean IsThereATrigger(DB4D_TriggerType WhatTrigger, DataTable* df);
	Boolean IsThereATrigger(DB4D_TriggerType WhatTrigger, Table* tt, bool inCheckCascadingDelete = false);

	VError AddObjectID(TypeObj4DInBase typ, Obj4D* obj, const VUUID& id);
	VError DelObjectID(TypeObj4DInBase typ, Obj4D* obj, const VUUID& id);
	Obj4D* GetObjectByID(TypeObj4DInBase typ, const VUUID& id) const;


	VError InitDataFileOfFields();
	inline VError AddFieldRec(sLONG tablenum, sLONG fieldnum)
	{
		return fDataTableOfFields->AddFieldRec(tablenum, fieldnum);
	};

	inline VError DelFieldRec(sLONG tablenum, sLONG fieldnum)
	{
		return fDataTableOfFields->DelFieldRec(tablenum, fieldnum);
	};

	VError InitDataFileOfIndexes();

	VError InitDataFileOfIndexCols();
	inline VError AddIndexCol(IndexInfo* ind, const VUUID& fieldid, sLONG pos)
	{
		return fDataTableOfIndexCols->AddIndexCol(ind, fieldid, pos);
	};

	inline VError DelIndexCol(IndexInfo* ind)
	{
		return fDataTableOfIndexCols->DelIndexCol(ind);
	};

	VError InitDataFileOfConstraints();
	inline VError AddConstraint(Table* tab)
	{
		return fDataFileOfConstraints->AddConstraint(tab);
	};

	VError DelConstraint(Table* tab)
	{
		return fDataFileOfConstraints->DelConstraint(tab);
	};

	VError AddConstraint(Relation* rel)
	{
		return fDataFileOfConstraints->AddConstraint(rel);
	};

	VError DelConstraint(Relation* rel)
	{
		return fDataFileOfConstraints->DelConstraint(rel);
	};

	VError InitDataFileOfConstraintCols();
	inline VError AddConstraintCol(Table* tab, Field* cri, sLONG pos)
	{
		return fDataFileOfConstraintCols->AddConstraintCol(tab, cri, pos);
	};

	VError DelConstraintCol(Table* tab)
	{
		return fDataFileOfConstraintCols->DelConstraintCol(tab);
	};

	VError AddConstraintCol(Relation* rel, Field* cri, sLONG pos, Field* relatedcri)
	{
		return fDataFileOfConstraintCols->AddConstraintCol(rel, cri, pos, relatedcri);
	};

	VError DelConstraintCol(Relation* rel)
	{
		return fDataFileOfConstraintCols->DelConstraintCol(rel);
	};



	VError CountBaseFreeBits( BaseStatistics &outStatistics ) const;

	Boolean MustRebuildAllIndex() const { return fMustRebuildAllIndex; };

	inline Boolean ReferentialIntegrityEnabled() const { return fEnableReferentialIntegrity; };
	inline Boolean CheckUniquenessEnabled() const { return fEnableCheckUniqueness; };
	inline Boolean CheckNot_NullEnabled() const { return fEnableCheckNot_Null; };

	inline void SetReferentialIntegrityEnabled(Boolean state) { fEnableReferentialIntegrity = state; };
	inline void SetCheckUniquenessEnabled(Boolean state) { fEnableCheckUniqueness = state; };
	inline void SetCheckNot_NullEnabled(Boolean state) { fEnableCheckNot_Null = state; };

	VError StartDataConversion();
	void StopDataConversion(BaseTaskInfo* context, VProgressIndicator* progress, bool MustCheckPrimKeyValidity);

	inline const VArrayOf<SegData*>* GetDataSegs() const { return &fSegments; };
	inline VArrayOf<SegData*>* GetDataSegs() { return &fSegments; };

	void Touch();

	inline sLONG GetStamp() const { return fStamp; };

	inline void TouchExtraProp() { fStampExtraProp++; };
	inline sLONG GetStampExtraProp() const { return fStampExtraProp; };

	inline void TouchSchemasCatalogStamp()
	{
		if (testAssert(!fIsRemote))
			fSchemasCatalogStamp++;
	};

	inline sLONG GetSchemasCatalogStamp() const { return fSchemasCatalogStamp; }

	inline void	TouchIndexes()
	{
		if (testAssert(!fIsRemote))
		{
			++fIndexesStamp;
			if (fStoreAsXML)
				fStamp++;
			else
				Touch();
		}
	}

	inline uLONG GetIndexesStamp() const { return fIndexesStamp; }

	void GetTablesStamps(StampsVector& outStamps) const;
	void GetRelationsStamps(StampsVector& outStamps) const;

	VError RetainIndexes(ArrayOf_CDB4DIndex& outIndexes);

	VError DropAllRelationOnOneField(Field* inFieldToDelete, CDB4DBaseContext* inContext, Boolean inOnlyLocal = false);
	VError DropAllRelationOnOneTable(Table* inTableToDelete, CDB4DBaseContext* inContext);

	inline Boolean IsRemote() const { return fIsRemote; };

	VError UpdateTable(sLONG numtable, VStream* dataget);
	VError DeleteTable(sLONG numtable, VStream* dataget);
	VError UpdateRelation( sLONG inRelationIndex, VStream *inDataGet);		// inRelationIndex is 0 based
	VError DeleteRelation( sLONG inRelationIndex, VStream *inDataGet);		// inRelationIndex is 0 based

	VError SendToClient(IRequestReply *inRequest, CDB4DBaseContext *inContext);
	inline DB4DNetworkManager* GetNetAccess() const { return fNetAccess; };
	inline DB4DNetworkManager* GetLegacyNetAccess() const { return fLegacy_NetAccess; };
	IRequest* CreateRequest( CDB4DBaseContext *inContext, sWORD inRequestID, Boolean legacy = false) const;

	Boolean TryToPoseVerrouDataLocker();
	Boolean PoseVerrouDataLocker();
	Boolean RetireVerrouDataLocker();

	LockEntity* GetDBLocker() const { return fBaseisLockedBy; };

	Boolean IncLockCount(BaseTaskInfo* context);
	Boolean DecLockCount(BaseTaskInfo* context);

	void ClearLockCount(BaseTaskInfo* context);

	Boolean TryToStartDataModif(BaseTaskInfo* context);
	void StartDataModif(BaseTaskInfo* context);
	void EndDataModif(BaseTaskInfo* context);

	void IncDataModif(BaseTaskInfo* context);
	void DecDataModif(BaseTaskInfo* context);
	sLONG GetDataModifCount(BaseTaskInfo* context);

	inline Boolean FlushNotCompleted() const { return fFlushWasNotComplete; };
	void ValidateHeader();
	void InvalidateHeader();
	void RequestForInvalideHeader();
	void EndOfRequestForInvalideHeader();

	BaseTaskInfo* BuildContextForJournal(ContextMap& contexts, uLONG8 contextid, VError& err, bool wasInTrans);

	VError IntegrateJournal(DB4DJournalParser* jparser, uLONG8 from, uLONG8 upto, uLONG8 *outCountIntegratedOperations, VDB4DProgressIndicator* InProgress, bool isMirror = false, sLONG8* lastMirrorAction = nil);

#if NewJournal

	VError GetErrorOnJournalFile() const
	{ 
		return fLogPool.GetLastError(); 
	}

	sLONG8 GetCurrentLogOperation() const 
	{ 
		return fLogPool.GetCurrentOperation();
	}

#else
	VError GetErrorOnJournalFile() const { return fLogErr; }
	sLONG8 GetCurrentLogOperation() const { return fCurrentLogOperation; }
#endif

	sLONG8 GetCurrentLogOperationOnDisk() const { return hbbloc.lastaction; }

	void SetCurrentLogOperationOnDisk(sLONG8 currentOper, bool modif = false)
	{
		hbbloc.lastaction = currentOper;
		if (modif)
			setmodif(true, this, nil);
	}

	bool GetJournalUUIDLink( VUUID &outLink );
	void GetJournalInfos( const VFilePath &inDataFilePath, VFilePath &outFilePath, VUUID &outDataLink);

#if NewJournal
	JournalPool* GetLogPool()
	{
		return &fLogPool;
	}
#else
	VError CreateJournal( VFile *inFile, VUUID *inDataLink, bool inWriteOpenDataOperation = true);
	VError OpenJournal( VFile *inFile, VUUID &inDataLink, bool inWriteOpenDataOperation = true);
#endif

	inline Boolean IsWriteProtected() const { return fWriteProtected; }

	VFileDesc* GetDataIndexSeg() const;
	VFileDesc* GetStructIndexSeg() const;

	VError GetDataSegs(vector<VFileDesc*>& outSegs, Boolean inWithSpecialSegs) const;
	VError GetStructSegs(vector<VFileDesc*>& outSegs, Boolean inWithSpecialSegs) const;

	sLONG GetNumOfTableRefInData(const VString& name) const;
	sLONG GetNumOfTableRefInData(const VUUID& id) const;
	void DropTableDefInData(sLONG num);

	void LoadAllTableDefsInData(bool buildTableInStruct);

	DataTable* RetainDataTableByUUID(const VUUIDBuffer& id) const;

	inline void SetExtraPropHeader(DataAddr4D addr, sLONG len)
	{
		hbbloc.ExtraAddr = addr;
		hbbloc.ExtraLen = len;
		setmodif(true, this, nil);
	}

	IndexInfo* LoadAndInitIndex(StructElemDef* e, VError& err);

	VError CompletelyChangeUUID(const VUUIDBuffer& id);

	bool ParseAndPurgeLogEntries(logEntryRefVector& logEntries, logEntryRefVector::iterator startFrom, bool& transWasCanceled);
	VErrorDB4D EndBackup(BaseTaskInfo* context, VFile* oldJournal);

	inline VIntlMgr* GetIntlMgr() const { return fIntlMgr; };

	VError SetIntlMgr(VIntlMgr* inIntlMgr, VDB4DProgressIndicator* InProgress = nil);
	
	void ClearAllCachedRelPath();
	void AddCachedRelPath(sLONG source, sLONG dest, const JoinPath& inPath);
	Boolean GetCachedRelPath(sLONG source, sLONG dest, JoinPath& outPath) const;

	void DelayForFlush();

	VError RelateManySelection(Table* StartingSelectionTable, Field* cri, QueryResult& outResult, BaseTaskInfo* context, 
								QueryOptions* options, VDB4DProgressIndicator* InProgress);

	VError RelateOneSelection(Table* StartingSelectionTable, sLONG TargetOneTable, QueryResult& outResult, BaseTaskInfo* context, 
								QueryOptions* options, VDB4DProgressIndicator* InProgress, std::vector<Relation*> *inPath);


	FicheInMem* BuildRecordFromServerAsSubRecord(VStream* from, BaseTaskInfo* inContext, Table* inTable, VError& outError);

	FicheInMem* BuildRecordFromServer(VStream* from, BaseTaskInfo* inContext, Table* inTable, VError& outError);
	FicheInMem* BuildRecordFromClient(VStream* from, BaseTaskInfo* inContext, Table* inTable, VError& outError);
	FicheInMem* BuildRecordMinimalFromClient(VStream* from, BaseTaskInfo* inContext, Table* inTable, VError& outError);

	Selection* BuildSelectionFromServer(VStream* from, CDB4DBaseContext* inContext, Table* inTable, VError& outError);
	Selection* BuildSelectionFromClient(VStream* from, CDB4DBaseContext* inContext, Table* inTable, VError& outError);

	Bittab* BuildSetFromServer(VStream* from, CDB4DBaseContext* inContext, VError& outError);
	Bittab* BuildSetFromClient(VStream* from, CDB4DBaseContext* inContext, VError& outError);

	VError PutMaxRecordRemoteInfo(sLONG curstamp, VStream* outstream, sLONG& outNewStamp);

	void IncRemoteMaxRecordsStamp();

	inline sLONG GetRemoteMaxRecordsStamp() const // used on client
	{
		return fRemoteMaxRecordsStamp;
	}

	inline void SetRemoteMaxRecordsStamp(sLONG newstamp) // used on client
	{
		fRemoteMaxRecordsStamp = newstamp;
	}

	inline BaseHeader* GetHbbloc()
	{
		return &hbbloc;
	}

	bool IsDuplicateIndex(IndexInfo* ind);

#if debugOverlaps_strong
	Boolean Debug_CheckAddrOverlap(DataAddr4D addrToCheck, sLONG lenToCheck, sLONG numtableToCheck = -1, sLONG numblobToCheck = -1, sLONG numrecToCheck = -1);
#endif

#if debuglr
	virtual	sLONG		Retain(const char* DebugInfo = 0) const;
	virtual	sLONG		Release(const char* DebugInfo = 0) const;

	void checkIndexInMap(IndexInfo* ind);
#endif

	VError BuildFirstSchemaIfNeeded();

	VError LoadSchemas();

	VError	CreateSchema( const VValueBag& inBag, VBagLoader *inLoader, CDB4DSchema **outSchema, CDB4DBaseContext* inContext);
	CDB4DSchema* CreateSchema(const VString& inName, BaseTaskInfo* context, VErrorDB4D& outError);

	VErrorDB4D DropSchema(CDB4DSchema* inSchema, BaseTaskInfo* context);

	VErrorDB4D RenameSchema(CDB4DSchema* inSchema, const VString& inNewName, BaseTaskInfo* context);

	VErrorDB4D RetainAllSchemas(std::vector<VRefPtr<CDB4DSchema> >& outSchemas);

	CDB4DSchema* RetainSchema(const VString& inSchemaName);

	CDB4DSchema* RetainSchema(DB4D_SchemaID inSchemaID);

	sLONG CountSchemas() const
	{
		return (sLONG)fSchemasByID.size();
	}

	void SetClientID(const VUUID& inID)
	{
		fClientID = inID;
	}

	const VUUID& GetClientID() const
	{
		return fClientID;
	}

	inline VFolder* GetStructFolder() const
	{
		return fStructFolder;
	}

	VError ExportToSQL(BaseTaskInfo* context, VFolder* inBaseFolder, VProgressIndicator* inProgress, ExportOption& options);
	VError ImportRecords(BaseTaskInfo* context, VFolder* inBaseFolder, VProgressIndicator* inProgress, ExportOption& options);

	VError BuildAutoModelCatalog(BaseTaskInfo* context);
	void InvalidateAutoCatalog();

	VError LoadEntityModels(const VFile* inFile = nil, ModelErrorReporter* errorReporter = nil, const VString* inXmlContent = nil, unordered_map_VString<VRefPtr<VFile> >* outIncludedFiles = nil, CUAGDirectory* inDirectory = nil, const VFile* inPermissions = nil);
	VError ReLoadEntityModels(const VFile* inFile = nil);
	VError SecondPassLoadEntityModels(ModelErrorReporter* errorReporter, bool dataWasAlreadyThere);

	VError SetEntityCatalog(LocalEntityModelCatalog* newcat);

	VError SaveEntityModels(bool alternateName = false);

	EntityModel* FindEntity(const VString& entityName, bool onlyRealOnes) const;

	EntityModel* RetainEntity(const VString& entityName, bool onlyRealOnes) const;

	EntityModel* RetainEntityModelByCollectionName(const VString& entityName) const;

	//sLONG CountEntityModels(bool onlyRealOnes) const;

	VError GetAllEntityModels(vector<CDB4DEntityModel*>& outList, CDB4DBaseContext* context, bool onlyRealOnes) const;

	VError RetainAllEntityModels(vector<CDB4DEntityModel*>& outList, CDB4DBaseContext* context, bool withTables, bool onlyRealOnes) const;
	VError RetainAllEntityModels(vector<VRefPtr<EntityModel> >& outList, bool onlyRealOnes) const;

	EntityModelCatalog* GetGoodEntityCatalog(bool onlyRealOnes) const;

	inline EntityModelCatalog* GetEntityCatalog(bool onlyRealOnes) const
	{
		VTaskLock lock(&fEntityCatalogMutex);
		return GetGoodEntityCatalog(onlyRealOnes);
	}

	VFile* RetainCatalogJSFile()
	{
		return RetainRefCountable(fCatalogJSFile);
	}

#if debugFindPlace_strong
	void Debug_CheckAddrOverWrite(IObjToFlush* obj);
#endif

	inline uLONG8 GetVersionNumber() const
	{
		return hbbloc.VersionNumber;
	}

	SynchroBaseHelper* GetSyncHelper(bool BuildIfMissing = true);

	void DontUseEntityCatalog()
	{
		fDontUseEntityCatalog = true;
	}

	inline bool StoredAsXML() const
	{
		return fStoreAsXML;
	}

	sLONG FindNextTableNum();

	sLONG FindNextRelationNum();

	VError SaveStructAsXML();

	void TouchXML();

	inline CUAGDirectory* GetUAGDirectory() const
	{
		return fUAGDirectory;
	}

	inline bool AlwaysStoreBlobsInRecs() const
	{
		return fAlwaysStoreBlobsInRecs;
	}

	VError ResurectGhostTables();

	VError ResurectDataTable(sLONG TableDefNumInData, CDB4DBaseContext* inContext);
	VError ResurectDataTable(const VUUID& inTableID, CDB4DBaseContext* inContext);
	VError ResurectDataTable(const VString& inTableName, CDB4DBaseContext* inContext);
	void AssociateTableRefInDataWithDataTable(sLONG TableDefInDataNum, sLONG DataTableRealNum);

	VError GetListOfDeadTables(vector<VString>& outTableNames, vector<VUUID>& outTableIDs, CDB4DBaseContext* inContext);

	VError BuildTablesFromDataTables();

	VError SaveDataTablesToStructTablesMatching();
	bool ExistDataTablesToStructTablesMatching();

	VFile* RetainDataTablesToStructTablesMatchingFile();

	VFolder* RetainBlobsFolder();
	void GetBlobsFolderPath(VString& outPath);

	void AddXMLIndex(IndexInfo* ind)
	{
		fXMLIndexDef.push_back(ind);
	}

	void GetIndices(VJSArray& outIndices);

#if !NewJournal
	bool IsLogValid() const
	{
		return (fLogStream == nil) || fLogIsValid;
	}
#endif

	VError CompactInto(VFile* destData, IDB4D_DataToolsIntf* inDataToolLog, bool KeepRecordNumbers);

	VError ReleaseOutsideCatalogs();
	VError AddOutsideCatalog(const VString& catname, EntityModelCatalog* catalog);
	VError RemoveOutsideCatalog(EntityModelCatalog* catalog);

	EntityModelCatalog* GetCatalog(const VString& catname);
	void AddCatalogRef(const VString& catname, EntityModelCatalog* catalog);

	VError AddOutsideCatalog(const VValueBag* catref);
	VError AddOutsideSQLCatalog(const VValueBag* catref);

	VError EvaluateOutsideCatalogModelScript(VJSObject& globalObject, BaseTaskInfo* context, void* ProjectRef, std::vector<XBOX::VFile*> &outModelJSFiles);

	VError _fixForWakandaV4();

	ExtraElement* RetainExtraElement(sLONG n, OccupableStack* curstack, VError& err, bool* outNeedSwap);

	sLONG GetSmallID(Field* field, VError& err);
	sLONG GetSmallID(const VString& inRef, VError& err);
	VError SaveSmallIDsInRec();

	VError RollBackTransactionOnRemoteDataStores(BaseTaskInfo* context);
	VError CommitTransactionOnRemoteDataStores(BaseTaskInfo* context);
	VError StartTransactionOnRemoteDataStores(BaseTaskInfo* context);

	bool IsWakandaRunningMode() const
	{
		return fWakandaMode;
	}

	bool Is4DRunningMode() const
	{
		return !fWakandaMode;
	}

	void SetRiaServerProjectRef(void* ProjectRef)
	{
		fRiaServerProjectRef = ProjectRef;
	}

	RIApplicationRef GetRiaServerProjectRef() const
	{
		return (RIApplicationRef)fRiaServerProjectRef;
	}

	VError ReleaseWhatNeedsToBeReleasedOnCatalogs();

	bool SomeTablesCannotBeLogged(VJSONObject* ioParams);

	bool CheckPrimaryKeysValidity(BaseTaskInfo* context, VProgressIndicator* progress, VError& outErr, VJSONObject* ioParams, bool forceCheck = false);

	void GetPrimaryKeysValidityState( BaseTaskInfo* inContext, VJSONObject* ioParams) const;

	void SetFileSystemNamespace(VFileSystemNamespace* inFsNameSpace)
	{
		CopyRefCountable(&fBaseFileSystemNamespace, inFsNameSpace);
	}

	VFileSystemNamespace* GetFileSystemNamespace()
	{
		return fBaseFileSystemNamespace;
	}

	VError SetIntlInfo(const VString& stringCompHash, const VString& keywordBuildingHash);
	VError GetIntlInfo(VString& outStringCompHash, VString& outKeywordBuildingHash);

	VFile* GetStructFile() {
		return fStructXMLFile;
	}

	void GetOutsideCatalogs(EntityModelCatalogCollection& outOutsideCats);

	VJSONObject* RetainMeasures(VJSONObject* options = nil, JSONPath* path = nil);
	void SetMeasureInterval(uLONG interval);

	void HitCache(sLONG nbHits, VSize howManyBytes);
	void MissCache(sLONG nbMisses, VSize howManyBytes);
	void HasRead(sLONG nbReads, VSize howManyBytes);
	void HasWritten(sLONG nbWrites, VSize howManyBytes);

	

#if debug_Addr_Overlap
	void SetDBObjRef(DataAddr4D addr, sLONG len, debug_dbobjref* dbobjref, bool checkConflict = false);
	void DelDBObjRef(DataAddr4D addr, sLONG len);
	void ChangeDBObjRefLength(DataAddr4D addr, sLONG len, const debug_dbobjref& dbobjref);
	void CheckDBObjRef(DataAddr4D addr, sLONG len, const debug_dbobjref& dbobjref);
	void FillDebug_DBObjRef();
#endif

protected:
	void RecalcDependencesIndex(Boolean ClearFirst = true);
	void RecalcDependencesField();

	VError LoadDataTables();
	void DisposeDataTables();

	VError LoadFieldIdsInRec();

	VError LoadSeqNums();
	void DisposeSeqNums();

	VError LoadIndexesDef();

	VError SaveDataSegments();
	VError LoadDataSegments( const VFile& inFile, Boolean DO_NOT_Map_Data_And_Struct = false, FileAccess InAccess = FA_READ_WRITE, bool ConvertToHigherVersion = false, bool allowConvertForReadOnly = false, bool allowV11toOpenV12 = false);
	void DisposeDataSegments();

	VError LoadIndexes(Boolean DO_NOT_map_IndexesInStruct);
	//VError SaveIndexes();
	void DisposeIndexes();

	void xInit(Boolean FromConstructor);

	static VError ReadBaseHeader( VFileDesc& inFile, BaseHeader *outHeader, Boolean CheckVersionNumber, Boolean DO_NOT_Map_Data_And_Struct, const VUUID* StructID, 
								sLONG *retour, Boolean &WasSwapped, bool ConvertToHigherVersion, bool allowConvertForReadOnly, bool allowV11toOpenV12);

	VError LoadRelations();
	VError oldLoadRelations();
	
	VError CountSegmentFreeBits( SegData *inSegdata, uLONG8 &outFreebits, uLONG8 &outUsedbits) const;
	VError CountDataFreeBits( uLONG8 &outFreebits, uLONG8 &outUsedbits) const;

	VError	TryTempFolder( DB4DFolderSelector inSelector, const VString& inCustomFolderPath, VFolder **outRetainedFolder) const;

	VError CopyTableDefsFrom(Base4D_NotOpened* from, TabAddrDisk& taddr, sLONG nbelem);


	CDB4DBase* fBaseX;
	Base4D *fStructure;
	sLONG fBaseHeaderSize;
	VArrayOf<SegData*> fSegments;
	SegData* fIndexSegment;
	BaseHeader hbbloc;
	VUUID fID;
	VUUID fClientID;
	
	VFilePath		fDefaultLocation;	// starting point for data files/segments
	VFolder*		fTempFolder;
	VFolder*		fStructFolder;
	mutable VCriticalSection fTempFolderMutex;

	Boolean			fIndexDelayed;
	Boolean			fIsDataOpened;
	Boolean			fIsStructureOpened;
	//Boolean			fNeedsToSaveIndexes;
	Boolean     	fNeedsToSaveRelation;
	Boolean			fIsNotifying;
	Boolean			fIsClosing;
	Boolean			fIsParsingStruct;
	Boolean fWriteProtected;
	sLONG fNbTable;
	//sLONG fNbDataTable;
	TableArray fich;
	TableArray fSystemTables;
	//DataTableRegularArray fichdata;
	DataTableVector fDataTables;
	
	AddrTableHeader fTableDataTableTabAddr;
	
	//StructHeader hparam;

	//IndexArray lbi;
	IndexMap fAllIndexes;
	sLONG fIndexesStamp;	// used for C/S synchro.
	MapOfObj4D fObjectIDs;
	mutable VCriticalSection fObjectIDsMutex;
	mutable VCriticalSection fDelayIndexMutex;
		
	//DataTableRegular special_pour_index;
	Obj4D *IndexSemaphore;
	VString	fBaseName;
	sWORD fresnum;
	
	BaseFlushInfo* fFlushInfo;
	VDBFlushMgr* fFlusher;
//	CClass* BaseRef;
	// CVariable* BaseVar;
//	CNameSpace* BaseNameSpace;

	Boolean fRegistered;
	LockEntity *fBaseisLockedBy;
	VCriticalSection fDataLockerMutex;
	sLONG fCountLock;
	sLONG8 fTotalLocks;
	LockerCount fAllLocks;
	LockerCount fDataModifCounts;
	sLONG fCountDataModif;

	V0ArrayOf<Relation*> fRelations;

	VChainOf<ReadAheadBuffer> fBuffers;
	mutable VCriticalSection fBuffersMutex;

	AddrTableHeader fIndexesInStructTabAddr;
	StructElemDefTreeInMemHeader fIndexesInStructInMem;

	AddrTableHeader fTableDefTabAddr;
	StructElemDefTreeInMemHeader fTableDefInMem;

	AddrTableHeader fRelationDefTabAddr;
	StructElemDefTreeInMemHeader fRelationDefInMem;

	AddrTableHeader fIndexesTabAddr;
	StructElemDefTreeInMemHeader fIndexesInMem;

	AddrTableHeader fTableSeqNumTabAddr;
	MapOfSeqNum fSeqNums;

	TableRefMap fTableDefsInData;
	TableNameMap fTableNamesInData;
	vector<sLONG> fRealDataTableNums;
	vector<VString> fTableNamesInDataByNum;
	vector<VUUID> fTableDefsInDataByNum;
#if NewJournal
	JournalPool fLogPool;
#else
	mutable VCriticalSection fLogMutex;
	VStream* fLogStream;
	VFile* fLogFile;
	VValueBag* fLogErrorStack;
	sLONG8 fCurrentLogOperation;
	VError fLogErr;
	bool fLogIsValid;
#endif

	ExtraPropertiesHeader fExtra;
	CountMap fUpdateCounts;

	DB4DTriggerManager *fTriggerManager;

	TableOfTable* fTableOfTables;
	DataTableOfTables* fDataTableOfTables;

	TableOfField* fTableOfFields;
	DataTableOfFields* fDataTableOfFields;

	TableOfIndexes* fTableOfIndexes;
	DataTableOfIndexes* fDataTableOfIndexes;

	TableOfIndexCols* fTableOfIndexCols;
	DataTableOfIndexCols* fDataTableOfIndexCols;

	TableOfConstraints* fTableOfConstraints;
	DataTableOfConstraints* fDataFileOfConstraints;

	TableOfConstraintCols* fTableOfConstraintCols;
	DataTableOfConstraintCols* fDataFileOfConstraintCols;

	TableOfSchemas* fTableOfSchemas;
	DataTableOfSchemas* fDataTableOfSchemas;

	TableOfViews			*fTableOfViews;
	DataTableOfViews		*fDataTableOfViews;

	TableOfViewFields		*fTableOfViewFields;
	DataTableOfViewFields	*fDataTableOfViewFields;

	VSyncEvent* fCloseSyncEvent;

	Boolean fIndexClusterInvalid, fMustRebuildAllIndex, fNeedRebuildDataTableHeader, fNeedToLoadOldRelations, fNeedToConvertIndexes;

	Boolean fEnableReferentialIntegrity;
	Boolean fEnableCheckUniqueness;
	Boolean fEnableCheckNot_Null;

	Boolean fIsRemote;
	Boolean fFlushWasNotComplete;

	sLONG fStamp;
	sLONG fStampExtraProp;

	DB4DNetworkManager* fNetAccess;
	DB4DNetworkManager* fLegacy_NetAccess;
	Base4D* fLocalDB;
	VIntlMgr* fIntlMgr;
	sLONG fRemoteMaxRecordsStamp;

	sLONG fRemoteStructureStampExtraProp;
	VValueBag* fRemoteStructureExtraProp;

	CachedRelationPathCollection fCachedRelPath;

	SchemaCatalogByName fSchemasByName;
	SchemaCatalogByID fSchemasByID;
	sLONG fSchemasCatalogStamp;
	mutable VCriticalSection fSchemaMutex;

	EntityModelCatalogsByName fCatalogs;
	EntityModelCatalogCollection fOutsideCatalogs;
	LocalEntityModelCatalog* fEntityCatalog;
	LocalEntityModelCatalog* fAutoEntityCatalog;
	mutable VCriticalSection fEntityCatalogMutex;
//	mutable VCriticalSection fAutoEntityCatalogMutex;
	SynchroBaseHelper* fSyncHelper;
	VFile* fCatalogJSFile;

	bool fDontUseEntityCatalog;
	bool fStoreAsXML;
	bool fAlwaysStoreBlobsInRecs;
	bool fAutoEntityCatalogBuilt;
	bool fWakandaMode;
	VFile* fStructXMLFile;
	const VBagArray* fIndexesBag;
	IndexRefCollection fXMLIndexDef;
	VString fEntityFileExt;

	CUAGDirectory* fUAGDirectory;

	VFolder* fBlobsFolder;

	IBackupSettings*	fBackupSettings;
	mapFieldsIDInRecByName* fFieldsIDInRec;

	ExtraElementTable* fExtraElements;

	void* fRiaServerProjectRef;
	VFileSystemNamespace* fBaseFileSystemNamespace;

	sLONG fNoIntegrityRequestCount;
	VCriticalSection fNoIntegrityRequestCountMutex;
	vector<sLONG> fModifiedPrimKeyCount;

	MeasureCollection fDiskReadCounts;
	MeasureCollection fDiskWriteCounts;
	MeasureCollection fDiskReadBytes;
	MeasureCollection fDiskWriteBytes;

	MeasureCollection fCacheHitBytes;
	MeasureCollection fCacheHitCount;
	MeasureCollection fCacheMissBytes;
	MeasureCollection fCacheMissCount;
	VCriticalSection fMeasureMutex;


	

#if debug_Addr_Overlap
	Debug_dbobjrefMapBySeg fDBObjRef;
	VCriticalSection fDBObjRefMutex;
#endif

#if debuglog
	VFileStream* fDebugLog;
#endif


};



class SegData_NotOpened;
class ToolLog;

typedef map<sLONG, SegData_NotOpened*> SegDataMap;
typedef vector<DataFile_NotOpened*> DataTableCollection;

class Base4D_NotOpened;

typedef enum
{
	obj_TableDef = 1,
	obj_RelationDef,
	obj_SeqNum,
	obj_IndexDef,
	obj_IndexInStructDef,
	obj_DataTable
} TypObjContainer;

class ObjContainer : public VObject, public IProblemReporterIntf
{
	public:
		ObjContainer(Base4D_NotOpened* owner, DataAddr4D firstaddr, sLONG nbelem, sLONG debuttrou, ToolLog* log, TypObjContainer typobj, DataBaseObjectType xSignature);
		virtual ~ObjContainer();

		VError CheckAllObjs(const VString& ProgressMessage, const VString& ProgressMessage2);
		VError CheckObjs(DataAddr4D ou, sLONG nbobjsmax, sLONG nbobjs, sLONG pos1, sLONG pos2, sLONG mastermultiplicateur);
		VError CheckOneEntry(DataAddr4D ou, sLONG len, sLONG numobj);

		virtual VError CheckObj(void* p, sLONG len, sLONG numobj, Boolean needswap, VError& err) = 0;
		virtual VError CheckObjWithAddr(DataAddr4D ou, sLONG len, sLONG numobj, VError& err);

		virtual VError PrepareAddrTable() = 0;
		virtual VError NormalizeAddrTable() { return VE_OK; };

		VError AddProblem(ToolObjectProblem problem, sLONG param1 = -1, sLONG param2 = -1, sLONG param3 = -1);

		virtual VError ReportInvalidTabAddrAddr(ToolLog* log, sLONG selector);
		virtual VError ReportInvalidTabAddrTag(ToolLog* log, sLONG selector);
		virtual VError ReportInvalidTabAddrRead(ToolLog* log, sLONG selector);
		virtual VError ReportInvalidTabAddrChecksum(ToolLog* log, sLONG selector);

		inline Boolean TestRegularSeg() const { return fTestRegularSeg; };

		VError CheckTrous();

		inline Base4D_NotOpened* GetOwner() const { return fOwner; };
		inline ToolLog* GetLog() const { return fLog; };
		inline TypObjContainer GetTypObj() const { return fTypObj; };


	protected:
		Base4D_NotOpened* fOwner;
		ToolLog* fLog;
		DataAddr4D fAddr;
		sLONG fNbelem;
		sLONG fDebuttrou;
		TypObjContainer fTypObj;

		Boolean fTabAddrHeaderHasBeenChecked;
		Boolean fTabAddrHeaderIsValid;
		Boolean fNbElemIsValid;
		Boolean fTabAddrIsValid;
		Boolean fTabTrouIsValid;
		Boolean fMapIsValid;
		Boolean fMapHasBeenChecked;
		Boolean fTestRegularSeg;
		Boolean fMustCheckWithAddr;

		TabAddrCache* fTempCache;
		DataBaseObjectType fxSignature;
};


class IndexDefChecker : public ObjContainer
{
	public:
		inline IndexDefChecker(Base4D_NotOpened* owner, DataAddr4D firstaddr, sLONG nbelem, sLONG debuttrou, ToolLog* log):
					ObjContainer(owner, firstaddr, nbelem, debuttrou, log, obj_IndexDef, DBOH_IndexDefElem) { fTestRegularSeg = false; };

		virtual VError CheckObj(void* p, sLONG len, sLONG numobj, Boolean needswap, VError& err);

		virtual VError PrepareAddrTable();

};


class IndexDefInStructChecker : public IndexDefChecker
{
	public:
		inline IndexDefInStructChecker(Base4D_NotOpened* owner, DataAddr4D firstaddr, sLONG nbelem, sLONG debuttrou, ToolLog* log):
					IndexDefChecker(owner, firstaddr, nbelem, debuttrou, log) { fTypObj = obj_IndexInStructDef; fxSignature = DBOH_IndexInStructDefElem;};

		//virtual VError CheckObj(void* p, sLONG len, sLONG numobj, Boolean needswap, VError& err);
		virtual VError PrepareAddrTable();

};


class TableDefChecker : public ObjContainer
{
	public:
		inline TableDefChecker(Base4D_NotOpened* owner, DataAddr4D firstaddr, sLONG nbelem, sLONG debuttrou, ToolLog* log):
					ObjContainer(owner, firstaddr, nbelem, debuttrou, log, obj_TableDef, DBOH_TableDefElem) { ; };

		virtual VError CheckObj(void* p, sLONG len, sLONG numobj, Boolean needswap, VError& err);
		virtual VError PrepareAddrTable();
		virtual VError NormalizeAddrTable();

};


class RelationDefChecker : public ObjContainer
{
	public:
		inline RelationDefChecker(Base4D_NotOpened* owner, DataAddr4D firstaddr, sLONG nbelem, sLONG debuttrou, ToolLog* log):
					ObjContainer(owner, firstaddr, nbelem, debuttrou, log, obj_RelationDef, DBOH_RelationDefElem) { ; };

		virtual VError CheckObj(void* p, sLONG len, sLONG numobj, Boolean needswap, VError& err);
		virtual VError PrepareAddrTable();

};


class SeqNumChecker : public ObjContainer
{
	public:
		inline SeqNumChecker(Base4D_NotOpened* owner, DataAddr4D firstaddr, sLONG nbelem, sLONG debuttrou, ToolLog* log):
					ObjContainer(owner, firstaddr, nbelem, debuttrou, log, obj_SeqNum, (DataBaseObjectType)0) { fMustCheckWithAddr = true; };

		virtual VError CheckObj(void* p, sLONG len, sLONG numobj, Boolean needswap, VError& err);
		virtual VError CheckObjWithAddr(DataAddr4D ou, sLONG len, sLONG numobj, VError& err);
		virtual VError PrepareAddrTable();

};


class DataTableChecker : public ObjContainer
{
	public:
		inline DataTableChecker(Base4D_NotOpened* owner, DataAddr4D firstaddr, sLONG nbelem, sLONG debuttrou, ToolLog* log):
					ObjContainer(owner, firstaddr, nbelem, debuttrou, log, obj_DataTable, (DataBaseObjectType)0) { fMustCheckWithAddr = true; };

		virtual VError CheckObj(void* p, sLONG len, sLONG numobj, Boolean needswap, VError& err);
		virtual VError CheckObjWithAddr(DataAddr4D ou, sLONG len, sLONG numobj, VError& err);
		virtual VError PrepareAddrTable();

		inline void setDataTables(DataTableCollection* inDataTables) { fDataTables = inDataTables; };

	protected:
		DataTableCollection* fDataTables;

};


class Field_NotOpened_Ref
{
	public:
		inline Field_NotOpened_Ref(sLONG tablenum, sLONG fieldnum) { fTablenum = tablenum; fFieldnum = fieldnum; };

		inline bool operator < (const Field_NotOpened_Ref& other) const
		{
			if (fTablenum == other.fTablenum)
				return fFieldnum < other.fFieldnum;
			else
				return fTablenum < other.fTablenum;
		};

		sLONG fTablenum;
		sLONG fFieldnum;
};


class UUIDObj_NotOpened
{
	public:
		inline UUIDObj_NotOpened(TypeOfUUID typ, const VString& name, sLONG numobj, sLONG numobj2)
		{
			fType = typ;
			fName = name;
			fNumObj = numobj;
			fNumObj2 = numobj2;
		};

		TypeOfUUID fType;
		VString fName;
		sLONG fNumObj;
		sLONG fNumObj2;
};


class FieldObj_NotOpened
{
public:
	inline FieldObj_NotOpened()
	{
		fType = 0;
	};

	inline FieldObj_NotOpened(sLONG typ, const VString& name)
	{
		fType = typ;
		fName = name;
	};

	sLONG fType;
	VString fName;
};


typedef map<sLONG, VString> TableNamesCollection;
typedef map<Field_NotOpened_Ref, FieldObj_NotOpened> FieldNamesCollection;
typedef map<VUUIDBuffer, UUIDObj_NotOpened> UUIDObj_NotOpened_Collection;

class ToolObject;
class Index_NonOpened;

typedef vector<Index_NonOpened*> IndexNonOpenedCollection;

class Base4D_NotOpened : public ObjAlmostInCache
{
	friend class Base4D;
	public:

		Base4D_NotOpened();
		virtual ~Base4D_NotOpened();

		void SetStruct(Base4D_NotOpened* inStruct);

		VError Open(VFile* inFile, ToolLog *log, FileAccess access = FA_MAX, Base4D_NotOpened* inStruct = nil);
		VError AttachTo(Base4D* inBase, ToolLog *log, BaseTaskInfo* context);

		void Close();

		inline void SetDatas(Base4D_NotOpened* datas) { fDatas = datas; };
		inline Base4D_NotOpened* GetDatas() const { return fDatas; };

		inline Base4D_NotOpened* GetStruct() const { return fStruct; };

		inline sLONG GetBaseHeaderSize() const { return (sizeof(BaseHeader)+127L) & -128L; };

		VError writelong(void* p, sLONG len, DataAddr4D ou, sLONG offset, VString* WhereFrom = nil);
		VError readlong(void* p, sLONG len, DataAddr4D ou, sLONG offset);

		uBOOL IsAddrValid(DataAddr4D ou, Boolean TestRegularSeg = true);

		inline Boolean IsOpen() const { return fIsOpen; };
		inline Boolean AllSegsAreOpen() const { return fIsFullyOpen; };

		inline Boolean HeaderIsValid() const { return FHeaderIsValid; };

		inline Boolean ValidHeaderVersion() const { return fValidVersionNumber; };
		inline Boolean AllSegmentsIDAreMatching() const { return FAllSegsIDMatch; };

		VError CheckAllDataSegs(ToolLog *log);

		VError CheckAllDataTables(ToolLog *log);

		VError LoadDataTables(ToolLog *log);

		inline Boolean DataTablesHeaderIsValid() const { return fIsDataTablesHeaderValid; };
		inline Boolean DataTablesHeaderIsFullyValid() const { return fIsDataTablesHeaderFullyValid; };

		VError ReadAddressTable(DataAddr4D ou, TabAddrDisk& taddr, sLONG pos1, sLONG pos2, ToolLog* log, 
								IProblemReporterIntf* reporter, VError& err, sLONG selector, Boolean TestRegularSeg = true, bool expectingStamps = false);

		DataAddr4D SubGetAddrFromTable(sLONG n, DataAddr4D addr, sLONG maxnbelem, sLONG nbelem, VError& err, 
										sLONG pos1, sLONG pos2, Boolean TestRegularSeg, sLONG* outLen, bool expectingStamps);
		DataAddr4D GetAddrFromTable(TabAddrCache* cache, sLONG n, DataAddr4D addr, sLONG nbelem, VError& err, Boolean TestRegularSeg = true, sLONG* outLen = nil, bool expectingStamps = false);

		VError CheckIndexDefs(ToolLog *log);
		VError CheckIndexDefInStructs(ToolLog *log);
		VError CheckSeqNums(ToolLog *log);
		VError CheckTableDefs(ToolLog *log);
		VError CheckRelationDefs(ToolLog *log);
		VError CheckDataTables(ToolLog *log);

		VError CheckExtraProperties(ToolLog *log);

		VError CheckAndLoadStructElemDef(DataAddr4D ou, sLONG len, sLONG numobj, ObjContainer* checker, void* &p, sLONG &outlen, Boolean &needswap, VError &err, DataBaseObjectType fxSignature);

		VError AddTableName(sLONG tablenum, const VString& tablename, ToolLog* log, VString* outName);
		VError AddFieldName(sLONG tablenum, sLONG fieldnum, sLONG fieldtype, const VString& fieldname, ToolLog* log);
		VError AddUUID(const VUUIDBuffer& id, TypeOfUUID type, const VString& name, sLONG numobj, sLONG numobj2, ToolLog* log);

		Boolean ExistUUID(const VUUIDBuffer& id, TypeOfUUID type, VString* outName = nil, TypeOfUUID* outType = nil, sLONG* outNumObj = nil, sLONG* outNumObj2 = nil);
		Boolean ExistField(sLONG numTable, sLONG numField, VString* outName = nil, sLONG* outType = nil);
		Boolean ExistTable(sLONG numTable, VString* outName = nil);

		VError CheckExtraProp(DataAddr4D& ou, sLONG& len, ToolObject& problemToReport, ToolLog* log, VValueBag* &outData, sLONG pos1, sLONG pos2, DataBaseObjectType taginfo);

		VError AddIndex(Index_NonOpened* ind, ToolLog* log);
		VError AddIndexInStruct(Index_NonOpened* ind, ToolLog* log);

		VError CheckAllIndexes(ToolLog* log);

		DataFile_NotOpened* GetDataTable(sLONG n) const;
		DataFile_NotOpened* GetDataTableWithTableDefNum(sLONG n) const;
		void SetDataTableByTableRefNum(DataFile_NotOpened* df, sLONG n);

		void GetIndexName(sLONG n, VString& outName) const;

		VError CheckOneTable(sLONG tablenum, ToolLog* log);
		VError CheckOneIndex(sLONG indexnum, ToolLog* log);

		sLONG CountTables(Boolean& outInfoIsValid, ToolLog* log);
		sLONG CountIndexes(Boolean& outInfoIsValid, ToolLog* log);
		void GetTableName(sLONG inTableNum, VString& outName, Boolean& outInfoIsValid, ToolLog* log);
		sLONG GetTables(std::vector<sLONG>& outTables,Boolean& outInfoIsValid,ToolLog* log);
		VError GetTableIndexes(sLONG inTableNum, std::vector<sLONG>& outIndexes, Boolean& outInfoIsValid, ToolLog* log);
		sLONG CountRecordsInTable(sLONG inTableNum, Boolean& outInfoIsValid, ToolLog* log);
		VError GetTableFields(sLONG inTableNum, std::vector<sLONG>& outFields, Boolean& outInfoIsValid, ToolLog* log);
		sLONG CountFieldsInTable(sLONG inTableNum, Boolean& outInfoIsValid, ToolLog* log);

		VError GetFieldInfo(sLONG inTableNum, sLONG inFieldNum, VString& outName, sLONG& outType, std::vector<sLONG>& outIndexes, Boolean& outInfoIsValid, ToolLog* log);
		VError GetIndexInfo(sLONG inIndexNum,XBOX::VString& outName,sLONG& outTableNum,std::vector<sLONG>& outFields, Boolean& outInfoIsValid, ToolLog* log);

		VUUIDBuffer& GetUUID() { return fID; };
		const VUUIDBuffer& GetUUID() const { return fID; };

		//VError RecoverByTags(Base4D& outBase, ToolLog *log);

		VError CheckBitTables(ToolLog* log);

		VFolder* GetName(VString& outName);

		inline BaseHeader* GetHbbloc()
		{
			return &hbbloc;
		}

		const VFile* GetDataFile()
		{
			if (segs.empty())
				return nil;
			else
				return segs.begin()->second->GetVFile();
		}




	protected:
		typedef set<VString> VstringSet;

		Base4D_NotOpened* fStruct;
		Base4D_NotOpened* fDatas;
		SegDataMap segs;
		BaseHeader hbbloc;

		Boolean fIsOpen, fIsFullyOpen, FHeaderIsValid, fValidVersionNumber, FAllSegsIDMatch;

		Boolean fIsDataTablesHeaderValid, fAreDataTablesLoaded, fIsDataTablesHeaderFullyValid, fSeqNumsAreLoaded;
		DataTableCollection fDataTables;
		DataTableCollection fDataTablesByTableDefNum;
		VstringSet fAlreadyTableNames;

		IndexDefChecker* fIndexes;
		IndexDefInStructChecker* fIndexesInStruct;
		SeqNumChecker* fSeqNums;
		TableDefChecker* fTableDefs;
		RelationDefChecker* fRelationDefs;
		DataTableChecker* fDataTableChecker;

		TableNamesCollection fTableNames;
		FieldNamesCollection fFieldNames;
		UUIDObj_NotOpened_Collection fUUIDs;

		IndexNonOpenedCollection fInds;
		IndexNonOpenedCollection fIndsInStruct;

		Base4D* fAttachedBase;
		BaseTaskInfo* fAttachedContext;

		VUUIDBuffer fID;

};


class ReadAheadBufferActivator
{
public:
	inline ReadAheadBufferActivator(Base4D* owner) { fBuf = owner->NewReadAheadBuffer(); };
	~ReadAheadBufferActivator() { if (fBuf != nil) fBuf->Release(); };
	inline ReadAheadBuffer* GetBuffer() const { return fBuf; };

protected:
	ReadAheadBuffer* fBuf;
};


typedef Base4D* Base4Dptr;

//sLONG FindStructResFile(VResourceFile *f);

Base4D *GetDefaultBase();


VValueBag* _RetainExtraPropertiesOnClosedBase(VFile* base, VUUID *outUUID, VError &err, uLONG8* outVersionNumber);


class Base4DRemote
{
	public:

		inline Base4D* GetLocalDB() const { return fLocalBase; };

	protected:

		Base4D* fLocalBase;

};




#endif