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
#ifndef __javascript_db4d__
#define __javascript_db4d__


#include "JavaScript/VJavaScript.h"

class Base4D;
class Table;
class EntityModel;
class CDB4DEntityRecord;
class EntityAttribute;
class CDB4DEntityAttributeValue;


CDB4DContext *GetDB4DContextFromJSContext( const XBOX::VJSContext& inContext);

void SetDB4DContextInJSContext( const XBOX::VJSContext& inContext, CDB4DContext* inDB4DContext);

CDB4DBaseContext *GetDB4DBaseContextFromJSContext( const XBOX::VJSContext& inContext, CDB4DBase* inBase);

BaseTaskInfo *GetBaseTaskInfoFromJSContext( const XBOX::VJSContext& inContext, Base4D* inBase);



//======================================================



class ENTITY_API EntitySelectionIterator
{
	public:
		EntitySelectionIterator(EntityCollection* inSel, bool inReadOnly, bool inAutoSave, BaseTaskInfo* inContext, EntityModel* inModel);
		EntitySelectionIterator(EntityRecord* inRec, BaseTaskInfo* inContext);
		EntitySelectionIterator(const EntitySelectionIterator& from);
		~EntitySelectionIterator();
		
		void First(BaseTaskInfo* inContext);
		void Next(BaseTaskInfo* inContext);
		void NextNotNull(BaseTaskInfo* inContext);
		
		void ReleaseCurCurec(bool canautosave);
		
		EntityRecord* GetCurRec(BaseTaskInfo* inContext);
		
		EntityModel* GetModel() const
		{
			return fModel;
		}
		
		sLONG GetCurPos() const
		{
			return fCurPos;
		}
		
		//sLONG GetCurRecID();
		
		XBOX::VError ReLoadCurRec(BaseTaskInfo* inContext, bool readonly, bool canautosave);
		
	protected:
		EntityModel* fModel;
		EntityCollection* fSel;
		EntityRecord* fCurRec;
		sLONG fCurPos, fSelSize;
		bool fReadOnly, fAutoSave;
};


//======================================================

 
class JSCollectionManager : public DB4DCollectionManager
{
	public:
		JSCollectionManager(JS4D::ContextRef inContext, bool simpleDate);
		virtual ~JSCollectionManager();

		virtual VErrorDB4D SetCollectionSize(RecIDType size, Boolean ClearData = true);
		virtual RecIDType GetCollectionSize();
		virtual sLONG GetNumberOfColumns();
		virtual	bool AcceptRawData();
		virtual CDB4DField* GetColumnRef(sLONG ColumnNumber);
		virtual VErrorDB4D SetNthElement(RecIDType ElemNumber, sLONG ColumnNumber, const XBOX::VValueSingle& inValue);
		
		virtual	XBOX::VSize GetNthElementSpace( RecIDType inIndex, sLONG inColumnNumber, XBOX::ValueKind inRawValueKind, bool *outRejected, XBOX::VError& outError);
		virtual	void* WriteNthElementToPtr( RecIDType inIndex, sLONG inColumnNumber, void *outRawValue, XBOX::ValueKind inRawValueKind, bool *outRejected, XBOX::VError& outError);

		// set *outRejected to true if you are not pleased with given raw data and want to get called with SetNthElement instead for this row (already initialized to false)
		virtual VErrorDB4D SetNthElementRawData(RecIDType ElemNumber, sLONG ColumnNumber, const void *inRawValue, XBOX::ValueKind inRawValueKind, bool *outRejected);

		virtual VErrorDB4D GetNthElement(RecIDType ElemNumber, sLONG ColumnNumber, const XBOX::VValueSingle*& outValue, bool *outDisposeIt);
		virtual VErrorDB4D AddOneElement(sLONG ColumnNumber, const XBOX::VValueSingle& inValue);
		virtual sLONG GetSignature();
		virtual XBOX::VError PutInto(XBOX::VStream& outStream);


		void SetNumberOfColumn(sLONG nb);
		inline VJSArray& getArrayRef(sLONG n)
		{
			return fValues[n-1];
		}

	protected:
		vector<VJSArray> fValues;
		JS4D::ContextRef fContextRef;
		sLONG fSize;
		bool fSimpleDate;
};



//======================================================



class VJSTable : public XBOX::VJSClass<VJSTable, Table>
{
public:
	typedef VJSClass<VJSTable, Table>	inherited;
	
	static	void			Initialize( const XBOX::VJSParms_initialize& inParms, Table* inTable);
	static	void			Finalize( const XBOX::VJSParms_finalize& inParms, Table* inTable);
	static	void			GetDefinition( ClassDefinition& outDefinition);
	static	void			GetProperty( XBOX::VJSParms_getProperty& ioParms, Table* inTable);
	static	void			GetPropertyNames( XBOX::VJSParms_getPropertyNames& ioParms, Table* inTable);

	static void _isEntityModel(XBOX::VJSParms_callStaticFunction& ioParms, Table* inTable);  // bool : isEntityModel()
	static void _GetID(XBOX::VJSParms_callStaticFunction& ioParms, Table* inTable);  // Number : GetID()
	static void _GetName(XBOX::VJSParms_callStaticFunction& ioParms, Table* inTable);  // String : GetName()
	static void _SetName(XBOX::VJSParms_callStaticFunction& ioParms, Table* inTable);  // void : SetName(String)
	static void _CountFields(XBOX::VJSParms_callStaticFunction& ioParms, Table* inTable); // Number : CountFields()
	static void _Drop(XBOX::VJSParms_callStaticFunction& ioParms, Table* inTable);  // Drop()
	static void _CreateField(XBOX::VJSParms_callStaticFunction& ioParms, Table* inTable);  // CreateField(String : name, Number : type, {Number : Size}, [{String : attribute}])
																					  // attribute : "Not Null", "Unique", "Auto Increment"
	static void _keepSyncInfo(XBOX::VJSParms_callStaticFunction& ioParms, Table* inTable);  // keepSyncInfo(bool)
	static void _setPrimaryKey(XBOX::VJSParms_callStaticFunction& ioParms, Table* inTable);  // _setPrimaryKey([field])
	static void _dropPrimaryKey(XBOX::VJSParms_callStaticFunction& ioParms, Table* inTable);  // _dropPrimaryKey()
	
	static void _AllRecords(XBOX::VJSParms_callStaticFunction& ioParms, Table* inTable);  // Selection : AllRecords()
	static void _Query(XBOX::VJSParms_callStaticFunction& ioParms, Table* inTable);  // Selection : Query(string : query, { bool : lock })
	static void _Find(XBOX::VJSParms_callStaticFunction& ioParms, Table* inTable);  // SelectionIterator : Find(string : query, { bool : lock })
	static void _NewRecord(XBOX::VJSParms_callStaticFunction& ioParms, Table* inTable);  // Record : NewRecord()
	static void _NewSelection(XBOX::VJSParms_callStaticFunction& ioParms, Table* inTable);  // Selection : NewSelection(bool : keepsorted | string : "KeepSorted")
	static void _setAutoSeqValue(XBOX::VJSParms_callStaticFunction& ioParms, Table* inTable);  // setAutoSeqValue(number : newValue)

};


//======================================================



class VJSField : public XBOX::VJSClass<VJSField, Field>
{
public:
	typedef VJSClass<VJSField, Field>	inherited;

	static	void			Initialize( const XBOX::VJSParms_initialize& inParms, Field* inField);
	static	void			Finalize( const XBOX::VJSParms_finalize& inParms, Field* inField);
	static	void			GetDefinition( ClassDefinition& outDefinition);

	static void _GetID(XBOX::VJSParms_callStaticFunction& ioParms, Field* inField);  // Number : GetID()
	static void _GetName(XBOX::VJSParms_callStaticFunction& ioParms, Field* inField);  // String : GetName()
	static void _SetName(XBOX::VJSParms_callStaticFunction& ioParms, Field* inField);  // void : SetName(String)
	static void _Drop(XBOX::VJSParms_callStaticFunction& ioParms, Field* inField);  // Drop()
	static void _CreateIndex(XBOX::VJSParms_callStaticFunction& ioParms, Field* inField);  // Index : CreateIndex({String : IndexType, String : Name})
	static void _DropIndex(XBOX::VJSParms_callStaticFunction& ioParms, Field* inField);  // DropIndex({String : IndexType})
	static void _Create_FullText_Index(XBOX::VJSParms_callStaticFunction& ioParms, Field* inField);  // Index : Create_FullText_Index({String : Name})
	static void _Drop_FullText_Index(XBOX::VJSParms_callStaticFunction& ioParms, Field* inField);  // Drop_FullText_Index()
	static void _SetType(XBOX::VJSParms_callStaticFunction& ioParms, Field* inField);  // SetType(Number : Type, {Number : Size})
	static void _GetType(XBOX::VJSParms_callStaticFunction& ioParms, Field* inField);  // Number : GetType()
	static void _IsIndexed(XBOX::VJSParms_callStaticFunction& ioParms, Field* inField);  // bool : IsIndexed()
	static void _Is_FullText_Indexed(XBOX::VJSParms_callStaticFunction& ioParms, Field* inField);  // bool : Is_FullText_Indexed()
	static void _IsUnique(XBOX::VJSParms_callStaticFunction& ioParms, Field* inField);  // bool : IsUnique()
	static void _IsNotNull(XBOX::VJSParms_callStaticFunction& ioParms, Field* inField);  // bool : IsNotNull()
	static void _IsAutoIncrement(XBOX::VJSParms_callStaticFunction& ioParms, Field* inField);  // bool : IsAutoIncrement()
	static void _SetNotNull(XBOX::VJSParms_callStaticFunction& ioParms, Field* inField);  // SetNotNull(bool)
	static void _SetAutoIncrement(XBOX::VJSParms_callStaticFunction& ioParms, Field* inField);  // SetAutoIncrement(bool)
	static void _SetUnique(XBOX::VJSParms_callStaticFunction& ioParms, Field* inField);  // SetUnique(bool)
	static void _IsAutoGenerate(XBOX::VJSParms_callStaticFunction& ioParms, Field* inField);  // bool : IsAutogenerate()
	static void _SetAutoGenerate(XBOX::VJSParms_callStaticFunction& ioParms, Field* inField);  // SetAutoGenerate(bool)
	static void _IsStoredAsUUID(XBOX::VJSParms_callStaticFunction& ioParms, Field* inField);  // bool : IsStoredAsUUID()
	static void _SetStoredAsUUID(XBOX::VJSParms_callStaticFunction& ioParms, Field* inField);  // SetStoredAsUUID(bool)
	static void _isStoredAsUTF8(XBOX::VJSParms_callStaticFunction& ioParms, Field* inField);  // bool : IsStoredAsUTF8()
	static void _SetStoredAsUTF8(XBOX::VJSParms_callStaticFunction& ioParms, Field* inField);  // SetStoredAsUTF8(bool)
	static void _isStoredOutside(XBOX::VJSParms_callStaticFunction& ioParms, Field* inField);  // bool : isStoredOutside()
	static void _setStoreOutside(XBOX::VJSParms_callStaticFunction& ioParms, Field* inField);  // setStoreOutside(bool)

};

class VJSBackupSettings : public XBOX::VJSClass<VJSBackupSettings, IBackupSettings>
{
public:
	typedef XBOX::VJSClass<VJSBackupSettings, IBackupSettings> inherited;

	static	VJSObject		CreateInstance( JS4D::ContextRef inContext, IBackupSettings *inSettings);


	static	void			CallAsConstructor(VJSParms_callAsConstructor& ioParms);
	static	void			Initialize( const XBOX::VJSParms_initialize& inParms, IBackupSettings* inSettings);
	static	void			Finalize( const XBOX::VJSParms_finalize& inParms, IBackupSettings* inSettings);
	static	void			GetDefinition( ClassDefinition& outDefinition);

	static void	_setDestination(XBOX::VJSParms_callStaticFunction& ioParms, IBackupSettings* inBackupSettings);  // SetDestination(String) SetDestination(Folder) or SetDestination(URL)
	static void	_isValid(XBOX::VJSParms_callStaticFunction& ioParms, IBackupSettings* inBackupSettings);  // Bool : IsValid()
	static void	_getDestination(XBOX::VJSParms_getProperty& ioParms, IBackupSettings* inBackupSettings);  // String : GetDestination()

};


//======================================================


class VJSDatabase : public XBOX::VJSClass<VJSDatabase, Base4D>
{
public:
	typedef VJSClass<VJSDatabase, Base4D>	inherited;

	static	void			PutAllModelsInGlobalObject(VJSObject& globalObject, Base4D* inDatabase, BaseTaskInfo* context);
	static	VJSObject		CreateJSEMObject( const VString& emName, const VJSContext& inContext, BaseTaskInfo *inBaseContext);
	
	static	void			Initialize( const XBOX::VJSParms_initialize& inParms, Base4D* inDatabase);
	static	void			Finalize( const XBOX::VJSParms_finalize& inParms, Base4D* inDatabase);
	static	void			GetProperty( XBOX::VJSParms_getProperty& ioParms, Base4D* inTables);
	static	void			GetPropertyNames( XBOX::VJSParms_getPropertyNames& ioParms, Base4D* inTables);
	static	void			GetDefinition( ClassDefinition& outDefinition);
	static	VJSObject		CreateInstance( JS4D::ContextRef inContext, Base4D *inDatabase);
	static IDB4D_DataToolsIntf* CreateJSDataToolsIntf(VJSContext& jscontext, VJSObject& paramObj);

	/**
	 * \brief Performs a backup using the given parameters.
	 * \return NULL if an error occured
	 * \return a File object leading to the backup manifest file for this backup
	 */
	static void	_Backup(XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase);
	
	/**
	 * \brief Returns the journal file as a File object or NULL if no journal is available
	 */
	static void	_GetJournalFile(XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase);

	/**
	 * \brief Indicates whether journal is enabled
	 * \return true if journaling is enabled
	 * \return false otherwise
	 */
	static void	_IsJournalEnabled(XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase);

	/**
	 * \brief disables journaling for the current database
	 * \return true if the journal was disabled
	 * \return false if an error occured
	 */
	static void	_DisableJournal(XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase);

	/**
	 * \brief Queries the datastore backup settings
	 * \return an object containing the backup settings
	 * \return a NULL value if the backup settings are not available
	 */
	static void	_GetBackupSettings(XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase);

	
	
	/**
	 * \brief Performs a backup and then configures the journal.
	 * \return NULL if an error occured
	 * \return a File object leading to the backup manifest file for this backup
	 */
	static void	_BackupAndChangeJournal(XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase);
	
	
	static void _GetName(XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase);  // String : GetName()
	static void _SetName(XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase);  // SetName(String)
	static void _CountTables(XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase);  // Number : CountTables()
	static void _GetPath(XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase);  // String : GetPath()
	static void _CreateTable(XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase);  // Table : CreateTable(String:name)
	static void _CreateIndex(XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase);  // Index : CreateIndex(array of fields : fields, String : IndexType, String : Name) // IndexType : "Btree", "Cluster", "Hash"
	static void _DropIndex(XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase);  // DropIndex(array of fields)
	static void _StartTransaction(XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase);  // StartTransaction()
	static void _Commit(XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase);  // Commit()
	static void _RollBack(XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase);  // RollBack()
	static void _TransactionLevel(XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase);  // Number : TransactionLevel()
	static void _GetStructure(XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase);  		
	static void _FlushCache(XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase);  // FlushCache(bool : waitUntilDone)
	static void _ExportAsSQL(XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase);  // _ExportAsSQL(Folder | StringURL, number : nbBlobsPerLevel, number : maxExportFileSize)
	static void _clearErrs(XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase);  // clearErrs() // temporaire
	static void _GetSyncInfo(XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase);  		
	static void _loadModelsDefinition(XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase);  // loadModelsDefinition(File : XMLDefinitionFile)
	static void _setCacheSize(XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase);  // setCacheSize(number : newCacheSize, {bool inPhysicalMemOnly})
	static void _getCacheSize(XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase);  // number : getCacheSize()
	static void _getDataFolder(XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase);  // Folder : getDataFolder()
	static void _getCatalogFolder(XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase);  // Folder : getCatalogFolder()
	static void _getTempFolder(XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase);  // Folder : getTempFolder()
	static void _getCacheInfo(XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase);
	static void _freeCacheMem(XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase);
	static void _getDBList(XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase);
	static void _getIndices(XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase); // Array : getIndices()
	static void _close(XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase); // close(SyncEventName) : returns null or a SyncEvent
	static void _fixForV4(XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase); // fixForV4()

	static void _verify(XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase); // bool : verify(Object: paramObj)

	static void _tempSetIndexNewFourche(XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase); 


	/*
	paramObj = {
		addProblem : function(problem),
		openProgress: function(progressTitle, maxElems),
		closeProgress: function(),
		progress: function(curElem, maxElems),
		setProgressTitle: function(progressTitle)
	}
	*/

	static void _queryOptions(XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase); // bool : queryOptions(object options ) options: { queryPlan: bool, queryPath: bool }
	static void _setSortMaxMem(XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase);  // setSortMaxMem(number : memSize)

	static void _getModelDefinition(XBOX::VJSParms_callStaticFunction& ioParms, Base4D* inDatabase);  // Object : getModelDefinition()


	static void _getTables( XBOX::VJSParms_getProperty& ioParms, Base4D* inDatabase);
	static void _getEntityModels( XBOX::VJSParms_getProperty& ioParms, Base4D* inDatabase);

};



class VJSDatabaseTableEnumerator : public XBOX::VJSClass<VJSDatabaseTableEnumerator, Base4D>
{
public:
	typedef VJSClass<VJSDatabaseTableEnumerator, Base4D>	inherited;

	static	void			Initialize( const XBOX::VJSParms_initialize& inParms, Base4D* inDatabase);
	static	void			Finalize( const XBOX::VJSParms_finalize& inParms, Base4D* inDatabase);
	static	void			GetProperty( XBOX::VJSParms_getProperty& ioParms, Base4D* inTables);
	static	void			GetPropertyNames( XBOX::VJSParms_getPropertyNames& ioParms, Base4D* inTables);
	static	void			GetDefinition( ClassDefinition& outDefinition);
	static	VJSObject		CreateInstance( JS4D::ContextRef inContext, Base4D *inDatabase);

};


class VJSDatabaseEMEnumerator : public XBOX::VJSClass<VJSDatabaseEMEnumerator, Base4D >
 {
 public:
	 typedef VJSClass<VJSDatabaseEMEnumerator, Base4D >	inherited;

	 static	void			Initialize( const XBOX::VJSParms_initialize& inParms, Base4D* inDatabase);
	 static	void			Finalize( const XBOX::VJSParms_finalize& inParms, Base4D* inDatabase);
	 static	void			GetProperty( XBOX::VJSParms_getProperty& ioParms, Base4D* inTables);
	 static	void			GetPropertyNames( XBOX::VJSParms_getPropertyNames& ioParms, Base4D* inTables);
	 static	void			GetDefinition( ClassDefinition& outDefinition);
	 static	VJSObject		CreateInstance( JS4D::ContextRef inContext, Base4D *inDatabase);

 };




//======================================================


class ENTITY_API VJSEntityModel : public XBOX::VJSClass<VJSEntityModel, EntityModel>
{
public:
	typedef VJSClass<VJSEntityModel, EntityModel> inherited;
	
	static	void			Initialize( const XBOX::VJSParms_initialize& inParms, EntityModel* inModel);
	static	void			Finalize( const XBOX::VJSParms_finalize& inParms, EntityModel* inModel);
	static	void			GetDefinition( ClassDefinition& outDefinition);
	static	void			GetProperty( XBOX::VJSParms_getProperty& ioParms, EntityModel* inModel);
	static	void			GetPropertyNames( XBOX::VJSParms_getPropertyNames& ioParms, EntityModel* inModel);
	static	void			CallAsFunction(VJSParms_callAsFunction& ioParms);
	static	void			CallAsConstructor(VJSParms_callAsConstructor& ioParms);
	
	static void _getDataStore(XBOX::VJSParms_callStaticFunction& ioParms, EntityModel* inModel); // dataStore : getDataStore()
	static void _getName(XBOX::VJSParms_callStaticFunction& ioParms, EntityModel* inModel); // string : getName()
	static void _isEntityModel(XBOX::VJSParms_callStaticFunction& ioParms, EntityModel* inModel); // bool : isEntityModel()
	static void _getScope(XBOX::VJSParms_callStaticFunction& ioParms, EntityModel* inModel); // string : getScope()
	
	static void _AllEntities(XBOX::VJSParms_callStaticFunction& ioParms, EntityModel* inModel);  // EntitySelection : AllEntities()  // alias all()
	static void _Query(XBOX::VJSParms_callStaticFunction& ioParms, EntityModel* inModel);  // EntitySelection : Query(string : query, { bool : lock })
	static void _Find(XBOX::VJSParms_callStaticFunction& ioParms, EntityModel* inModel);  // EntitySelectionIterator : Find(string : query, { bool : lock })
	static void _NewEntity(XBOX::VJSParms_callStaticFunction& ioParms, EntityModel* inModel);  // EntitySelectionIterator : _NewEntity()
	static void _NewSelection(XBOX::VJSParms_callStaticFunction& ioParms, EntityModel* inModel);  // EntitySelection : NewSelection(bool : keepsorted | string : "KeepSorted")

	static void _First(XBOX::VJSParms_callStaticFunction& ioParms, EntityModel* inModel);  // EntityRecord : First({bool ReadOnly})
	static void _Count(XBOX::VJSParms_callStaticFunction& ioParms, EntityModel* inModel);  // Number : _Count()
	static void _OrderBy(XBOX::VJSParms_callStaticFunction& ioParms, EntityModel* inModel);  // selection : OrderBy(string | attribute, {"asc|desc"} , attribute, attribute) 
	static void _Each(XBOX::VJSParms_callStaticFunction& ioParms, EntityModel* inModel);  // each(function)
	static void _dropEntities(XBOX::VJSParms_callStaticFunction& ioParms, EntityModel* inModel);  // selection : dropEntities()  // result is the locked set
	static void _distinctValues(XBOX::VJSParms_callStaticFunction& ioParms, EntityModel* inModel); // array : distinctValues(EntityAttribute | string)
	static void _toArray(XBOX::VJSParms_callStaticFunction& ioParms, EntityModel* inModel); // array : toArray(string : attributeList | ( EntityAttribute, EntityAttribute, EntityAttribute...))
	static void _fromArray(XBOX::VJSParms_callStaticFunction& ioParms, EntityModel* inModel); // selection : fromArray(array)

	static void _setAutoSequenceNumber(XBOX::VJSParms_callStaticFunction& ioParms, EntityModel* inModel); // setAutoSequenceNumber(number)
	static void _getFragmentation(XBOX::VJSParms_callStaticFunction& ioParms, EntityModel* inModel); // getFragmentation()

	static void _sum(XBOX::VJSParms_callStaticFunction& ioParms, EntityModel* inModel);  // number : sum(attribute)
	static void _min(XBOX::VJSParms_callStaticFunction& ioParms, EntityModel* inModel);  // number : min(attribute)
	static void _max(XBOX::VJSParms_callStaticFunction& ioParms, EntityModel* inModel);  // number : max(attribute)
	static void _average(XBOX::VJSParms_callStaticFunction& ioParms, EntityModel* inModel);  // number : average(attribute)
	static void _compute(XBOX::VJSParms_callStaticFunction& ioParms, EntityModel* inModel);  // object : compute(string : attributeList | ( EntityAttribute, EntityAttribute, EntityAttribute...))

	static void _callMethod(XBOX::VJSParms_callStaticFunction& ioParms, EntityModel* inModel);  // jsvalue : callMethod(string : funcName, array : params, object: this)

	static void _getAttributes( XBOX::VJSParms_getProperty& ioParms, EntityModel* inModel);
	static void _getLength( XBOX::VJSParms_getProperty& ioParms, EntityModel* inModel);
};

/*
class VJSEntityConstructor : public XBOX::VJSClass<VJSEntityConstructor, EntityModel>
{
	public:
		typedef VJSClass<VJSEntityConstructor, EntityModel> inherited;
		static	void			Initialize( const XBOX::VJSParms_initialize& inParms, EntityModel* inModel);
		static	void			Finalize( const XBOX::VJSParms_finalize& inParms, EntityModel* inModel);
		static	void			GetDefinition( ClassDefinition& outDefinition);
		static	void			CallAsFunction(VJSParms_callAsFunction& ioParms);
		static	void			CallAsConstructor(VJSParms_callAsConstructor& ioParms);

};
*/


class VJSEntityAttributeEnumerator : public XBOX::VJSClass<VJSEntityAttributeEnumerator, EntityModel>
{
public:
	typedef VJSClass<VJSEntityAttributeEnumerator, EntityModel>	inherited;

	static	void			Initialize( const XBOX::VJSParms_initialize& inParms, EntityModel* inModel);
	static	void			Finalize( const XBOX::VJSParms_finalize& inParms, EntityModel* inModel);
	static	void			GetProperty( XBOX::VJSParms_getProperty& ioParms, EntityModel* inModel);
	static	void			GetPropertyNames( XBOX::VJSParms_getPropertyNames& ioParms, EntityModel* inModel);
	static	void			GetDefinition( ClassDefinition& outDefinition);

};


class VJSEntityAttribute : public XBOX::VJSClass<VJSEntityAttribute, EntityAttribute>
{
public:
	typedef VJSClass<VJSEntityAttribute, EntityAttribute> inherited;
	
	static	void			Initialize( const XBOX::VJSParms_initialize& inParms, EntityAttribute* inAttribute);
	static	void			Finalize( const XBOX::VJSParms_finalize& inParms, EntityAttribute* inAttribute);
	static	void			GetDefinition( ClassDefinition& outDefinition);
	
	static void _getName(XBOX::VJSParms_callStaticFunction& ioParms, EntityAttribute* inAttribute); // string : getName()

	static void _getPropName( XBOX::VJSParms_getProperty& ioParms, EntityAttribute* inAttribute);
	static void _getKind( XBOX::VJSParms_getProperty& ioParms, EntityAttribute* inAttribute);
	static void _getType( XBOX::VJSParms_getProperty& ioParms, EntityAttribute* inAttribute);
	static void _getScope( XBOX::VJSParms_getProperty& ioParms, EntityAttribute* inAttribute);

//	static void _getUnique( XBOX::VJSParms_getProperty& ioParms, EntityAttribute* inAttribute);
	static void _getIndexed( XBOX::VJSParms_getProperty& ioParms, EntityAttribute* inAttribute);
	static void _getFullTextIndexed( XBOX::VJSParms_getProperty& ioParms, EntityAttribute* inAttribute);
	static void _getIndexType( XBOX::VJSParms_getProperty& ioParms, EntityAttribute* inAttribute);
	static void _getDataClass( XBOX::VJSParms_getProperty& ioParms, EntityAttribute* inAttribute);
	static void _getRelatedDataClass( XBOX::VJSParms_getProperty& ioParms, EntityAttribute* inAttribute);
//	static void _getReadOnly( XBOX::VJSParms_getProperty& ioParms, EntityAttribute* inAttribute);

};




class ENTITY_API VJSEntitySelectionIterator : public XBOX::VJSClass<VJSEntitySelectionIterator, EntitySelectionIterator>
{
public:
	typedef VJSClass<VJSEntitySelectionIterator, EntitySelectionIterator>	inherited;
	
	static	void			Initialize( const XBOX::VJSParms_initialize& inParms, EntitySelectionIterator* inSelIter);
	static	void			Finalize( const XBOX::VJSParms_finalize& inParms, EntitySelectionIterator* inSelIter);
	static	void			GetProperty( XBOX::VJSParms_getProperty& ioParms, EntitySelectionIterator* inSelIter);
	static	bool			SetProperty( VJSParms_setProperty& ioParms, EntitySelectionIterator* inSelIter);
	static	void			GetPropertyNames( XBOX::VJSParms_getPropertyNames& ioParms, EntitySelectionIterator* inSelIter);
	static	void			GetDefinition( ClassDefinition& outDefinition);
	
	static void _GetModel(XBOX::VJSParms_callStaticFunction& ioParms, EntitySelectionIterator* inSelIter);  // EntityModel : GetModel()
	static void _Next(XBOX::VJSParms_callStaticFunction& ioParms, EntitySelectionIterator* inSelIter);  // bool : Next()
	static void _Valid(XBOX::VJSParms_callStaticFunction& ioParms, EntitySelectionIterator* inSelIter);  // bool : Valid()
	static void _Loaded(XBOX::VJSParms_callStaticFunction& ioParms, EntitySelectionIterator* inSelIter);  // bool : Loaded()
	static void _Save(XBOX::VJSParms_callStaticFunction& ioParms, EntitySelectionIterator* inSelIter);  // bool ok : Save(bool : silent)
	static void _validate(XBOX::VJSParms_callStaticFunction& ioParms, EntitySelectionIterator* inSelIter);  // bool ok : validate()
	static void _Reload(XBOX::VJSParms_callStaticFunction& ioParms, EntitySelectionIterator* inSelIter);  // Reload(bool | string : "ReadOnly")
	static void _IsModified(XBOX::VJSParms_callStaticFunction& ioParms, EntitySelectionIterator* inSelIter);  // bool : IsModified()
	static void _IsNew(XBOX::VJSParms_callStaticFunction& ioParms, EntitySelectionIterator* inSelIter);  // bool : IsNew()
	static void _Drop(XBOX::VJSParms_callStaticFunction& ioParms, EntitySelectionIterator* inSelIter);  // bool ok : Drop(bool : silent)
	static void _Release(XBOX::VJSParms_callStaticFunction& ioParms, EntitySelectionIterator* inSelIter);  // Release()
	static void _getTimeStamp(XBOX::VJSParms_callStaticFunction& ioParms, EntitySelectionIterator* inSelIter);   // Date : _getTimeStamp()
	static void _toString(XBOX::VJSParms_callStaticFunction& ioParms, EntitySelectionIterator* inSelIter);   // string : toString()
	static void _toObject(XBOX::VJSParms_callStaticFunction& ioParms, EntitySelectionIterator* inSelIter);   // object : _toObject(string : attributeList | ( EntityAttribute, EntityAttribute, EntityAttribute...))
	static void _toJSON(XBOX::VJSParms_callStaticFunction& ioParms, EntitySelectionIterator* inSelIter);   // string : _toJSON
	static void _getKey(XBOX::VJSParms_callStaticFunction& ioParms, EntitySelectionIterator* inSelIter);   // value : getKey()
	static void _getStamp(XBOX::VJSParms_callStaticFunction& ioParms, EntitySelectionIterator* inSelIter);   // long : getStamp()
	static void _getModifiedAttributes(XBOX::VJSParms_callStaticFunction& ioParms, EntitySelectionIterator* inSelIter);  // Array of string : getModifiedAttributes()
	
};



class ENTITY_API VJSEntitySelection : public XBOX::VJSClass<VJSEntitySelection, EntityCollection>
{
public:
	typedef VJSClass<VJSEntitySelection, EntityCollection>	inherited;
	
	static	void			Initialize( const XBOX::VJSParms_initialize& inParms, EntityCollection* inSelection);
	static	void			Finalize( const XBOX::VJSParms_finalize& inParms, EntityCollection* inSelection);
	static	void			GetProperty( XBOX::VJSParms_getProperty& ioParms, EntityCollection* inSelection);
	static	void			GetDefinition( ClassDefinition& outDefinition);
	
	static void _First(XBOX::VJSParms_callStaticFunction& ioParms, EntityCollection* inSelection);  // EntityRecord : First({bool ReadOnly})
	static void _Count(XBOX::VJSParms_callStaticFunction& ioParms, EntityCollection* inSelection);  // Number : _Count()
	static void _OrderBy(XBOX::VJSParms_callStaticFunction& ioParms, EntityCollection* inSelection);  // selection : OrderBy(string | attribute, {"asc|desc"} , attribute, attribute) 
	static void _Add(XBOX::VJSParms_callStaticFunction& ioParms, EntityCollection* inSelection);  // _Add(Selection | Entity | Number)
	static void _GetModel(XBOX::VJSParms_callStaticFunction& ioParms, EntityCollection* inSelection);  // EntityModel : GetModel()
	static void _Each(XBOX::VJSParms_callStaticFunction& ioParms, EntityCollection* inSelection);  // each(function)
	static void _dropEntities(XBOX::VJSParms_callStaticFunction& ioParms, EntityCollection* inSelection);  // selection : dropEntities()  // result is the locked set
	//static void _orderBy(XBOX::VJSParms_callStaticFunction& ioParms, EntityCollection* inSelection);  // selection : OrderBy(string | field, {"asc|desc"} , field, field) 
	static void _toString(XBOX::VJSParms_callStaticFunction& ioParms, EntityCollection* inSelection);
	static void _distinctValues(XBOX::VJSParms_callStaticFunction& ioParms, EntityCollection* inSelection); // array : distinctValues(EntityAttribute | string)
	static void _toArray(XBOX::VJSParms_callStaticFunction& ioParms, EntityCollection* inSelection); // array : toArray(string : attributeList | ( EntityAttribute, EntityAttribute, EntityAttribute...))
	static void _Query(XBOX::VJSParms_callStaticFunction& ioParms, EntityCollection* inSelection);  // EntitySelection : Query(string : query)
	static void _Find(XBOX::VJSParms_callStaticFunction& ioParms, EntityCollection* inSelection);  // EntitySelectionIterator : Find(string : query)
	static void _sum(XBOX::VJSParms_callStaticFunction& ioParms, EntityCollection* inSelection);  // number : sum(attribute)
	static void _min(XBOX::VJSParms_callStaticFunction& ioParms, EntityCollection* inSelection);  // number : min(attribute)
	static void _max(XBOX::VJSParms_callStaticFunction& ioParms, EntityCollection* inSelection);  // number : max(attribute)
	static void _average(XBOX::VJSParms_callStaticFunction& ioParms, EntityCollection* inSelection);  // number : average(attribute)
	static void _compute(XBOX::VJSParms_callStaticFunction& ioParms, EntityCollection* inSelection);  // object : compute(string : attributeList | ( EntityAttribute, EntityAttribute, EntityAttribute...))

	static void _and(XBOX::VJSParms_callStaticFunction& ioParms, EntityCollection* inSelection);  // Collection : and(otherCollection)
	static void _or(XBOX::VJSParms_callStaticFunction& ioParms, EntityCollection* inSelection);  // Collection : or(otherCollection)
	static void _minus(XBOX::VJSParms_callStaticFunction& ioParms, EntityCollection* inSelection);  // Collection : minus(otherCollection)

	static void _getLength( XBOX::VJSParms_getProperty& ioParms, EntityCollection* inSelection);
	static void _getQueryPlan( XBOX::VJSParms_getProperty& ioParms, EntityCollection* inSelection);
	static void _getQueryPath( XBOX::VJSParms_getProperty& ioParms, EntityCollection* inSelection);

	static void _toJSON(XBOX::VJSParms_callStaticFunction& ioParms, EntityCollection* inSelection);   // string : _toJSON

	
};

/*
class VJSEntityAttributeValue : public VJSClass<VJSEntityAttributeValue, CDB4DEntityAttributeValue>
{
public:
	typedef VJSClass<VJSEntityAttributeValue, CDB4DEntityAttributeValue> inherited;
	
	static	void			Initialize( const XBOX::VJSParms_initialize& inParms, CDB4DEntityAttributeValue* inVal);
	static	void			Finalize( const XBOX::VJSParms_finalize& inParms, CDB4DEntityAttributeValue* inVal);
	static	void			GetDefinition( ClassDefinition& outDefinition);
	
	static void _getName(XBOX::VJSParms_callStaticFunction& ioParms, CDB4DEntityAttributeValue* inVal); // string : getName()
	
};
*/




class VJSCacheManager : public XBOX::VJSClass<VJSCacheManager, VDBMgr>
{
public:
	typedef VJSClass<VJSCacheManager, VDBMgr >	inherited;
	
	static	void			Initialize( const XBOX::VJSParms_initialize& inParms, VDBMgr* inMgr);
	static	void			Finalize( const XBOX::VJSParms_finalize& inParms, VDBMgr* inMgr);
	static	void			GetDefinition( ClassDefinition& outDefinition);

	static void _getObjects(XBOX::VJSParms_callStaticFunction& ioParms, VDBMgr* inMgr);  // Object : GetObjects()

	static void _getGarbageCollectInterval( XBOX::VJSParms_getProperty& ioParms, VDBMgr* inMgr);
	static bool _setGarbageCollectInterval( XBOX::VJSParms_setProperty& ioParms, VDBMgr* inMgr);

};



void CreateGlobalDB4DClasses();

void buildAttributeListFromParams(XBOX::VJSParms_callStaticFunction& ioParms, sLONG& StartParam, EntityModel* model, EntityAttributeSortedSelection& outList, 
								  EntityAttributeSelection& outExpand, EntityAttributeSortedSelection& outSortingAtts, BaseTaskInfo* context);


#endif
