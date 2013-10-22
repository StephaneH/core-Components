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
#ifndef __DB4D__ 
#define __DB4D__

#define DB4D_AS_DLL 1

#ifdef DB4D_AS_DLL
	#define DB4DasDLL 1
	#define DB4DasComponent 0
#else
	#define DB4DasDLL 0
	#define DB4DasComponent 1
#endif


#include "KernelIPC/VKernelIPC.h"
#include "ServerNet/VServerNet.h"
#include "HTTPServer/Interfaces/CHTTPServer.h"
#include "UsersAndGroups/Sources/UsersAndGroups.h"
#include "KernelIPC/Sources/VComponentManager.h"

#pragma pack( push, 8 )


#ifndef DB4D_API
// any class except components should use DB4D_API
// this is necessary so that RTTI can work with gcc.
	#if COMPIL_GCC
		#define DB4D_API IMPORT_API
	#else
		#define DB4D_API
	#endif
#endif


#ifdef DB4D_EXPORTS
	#define ENTITY_API EXPORT_API
#else
	#define ENTITY_API IMPORT_API
#endif


typedef struct OpaqueRIApplicationRef* RIApplicationRef;

namespace xbox
{
	class VJSContext;
	class VJSObject;
	class VJSGlobalContext;
	class VJSArray;
	class VWorkerPool;
	class VTCPSelectIOPool;
};


//const XBOX::VJSGlobalContext* kJSContextCreator = (XBOX::VJSContext*)(-1);
#define kJSContextCreator (XBOX::VJSGlobalContext*)(-1)

typedef sLONG RecIDType;

const OsType DB4D_StructFileType = (OsType)'.4xb';
const OsType DB4D_DataFileType = (OsType)'.4xd';
const OsType DB4D_TempFileType = (OsType)'.tmp';
const OsType DB4D_PrefFileType = (OsType)'.prf';
const OsType DB4D_Creator = (OsType)'4D07';

class CDB4DIndex;
class CDB4DBase;

typedef XBOX::VArrayOf<sLONG> xArrayOfLong;
typedef XBOX::VArrayOf<XBOX::VErrorBase*> ListOfErrors;
typedef XBOX::VError VErrorDB4D;
typedef XBOX::VArrayOf<XBOX::VValueSingle*> ListOfValues;
typedef XBOX::VArrayRetainedOwnedPtrOf<CDB4DIndex*> ArrayOf_CDB4DIndex;

typedef struct Opaque*	FicheInMem4DPtr;
typedef struct Opaque2*	LockPtr;
//typedef struct Opaque3* CDB4DRemoteRecordCachePtr;

class CDB4DFieldCacheCollection;
class VDBLanguageContext;	// defined in 4D

#ifndef nil
#define nil 0
#endif


typedef sLONG ContextID;
 
const ContextID NullContextID = 0;


class CNameSpace;
class DataToolsSaxLog;

class DB4D_API IDB4D_DataToolsIntf
{
public :

	enum { Component_Type = 'dtif'};

	virtual VErrorDB4D AddProblem(const XBOX::VValueBag& inProblemBag) = 0;
	virtual VErrorDB4D OpenProgression(const XBOX::VString inProgressTitle, sLONG8 inMaxElems) = 0;  // inMaxElems == -1 means non linear progression
	virtual VErrorDB4D CloseProgression() = 0;  // OpenProgression can be called in nested levels
	virtual VErrorDB4D Progress(sLONG8 inCurrentValue, sLONG8 inMaxElems) = 0;

	virtual VErrorDB4D SetProgressTitle(const XBOX::VString inProgressTitle) = 0;
};

/*
	interface to encapsulate all graphics related needs for DB4D to avoid the need to link with xbox::Graphics.
	not available for now on Linux platform.
*/
class DB4D_API IDB4D_GraphicsIntf
{
public:
	virtual	XBOX::VError	ConvertImageToPreferredFormat( XBOX::VValueSingle& ioValPicture, const std::vector<XBOX::VString*>& inPreferredImageFormats) = 0;
};

const sLONG Apply4DMethodTo_None = 0;
const sLONG Apply4DMethodTo_DataClass = 1;
const sLONG Apply4DMethodTo_Entity = 2;
const sLONG Apply4DMethodTo_Collection = 3;

class CDB4DTable;
class CDB4DContext;
class CDB4DRecord;
class CDB4DSelection;
class CDB4DBaseContext;

typedef struct  
{
	XBOX::VString name;
	XBOX::VString tablename;
	sLONG applyTo;
} Public4DMethodInfo;

class DB4D_API IDB4D_ApplicationIntf
{
public:
	virtual	XBOX::VWorkerPool*		GetSharedWorkerPool() = 0;
	virtual	XBOX::VTCPSelectIOPool*	GetSharedSelectIOPool() = 0;
	virtual	XBOX::VError			DescribePermissionsForSchema (
										sLONG inSchemaID,
										sLONG& outReadGroupID, XBOX::VString& outReadGroupName,
										sLONG& outReadWriteGroupID, XBOX::VString& outReadWriteGroupName,
										sLONG& outAllGroupID, XBOX::VString& outAllGroupName ) = 0;
	virtual	void					RegisterVTaskForRemoteUID(XBOX::VTask *inTask, const XBOX::VUUID& inRemoteUID, const XBOX::VUUID& inDB4DBaseContextUID) = 0;
	virtual void					UnregisterVTaskForRemoteUID(XBOX::VTask *inTask, const XBOX::VUUID& inRemoteUID) = 0;

		/** @brief	Call garbage collect for all the JavaScript contexts of the current solution */
	virtual	XBOX::VError			JS4DGarbageCollect() = 0;

	virtual XBOX::VFolder*			RetainWaLibFolder() = 0;

		/** @brief	If inRequest contains some information about the HTTP session, the session storage object is updated according to this session.
					In all other case, an empty session storage object is set. */
	virtual	XBOX::VJSGlobalContext*	RetainJSContext( RIApplicationRef inApplicationRef, XBOX::VError& outError, bool inReusable, const IHTTPRequest* inRequest) = 0;

		/** @brief	If inResponse is not NULL, we check for session storage object changes, serialize it and update the cookie if needed. */
	virtual	XBOX::VError			ReleaseJSContext( RIApplicationRef inApplicationRef, XBOX::VJSGlobalContext* inContext, IHTTPResponse* inResponse) = 0;
	
	virtual	CUAGDirectory*			RetainUAGDirectory( RIApplicationRef inApplicationRef, XBOX::VError& outError) = 0;

	virtual	CUAGSession*			RetainUAGSession(const XBOX::VJSGlobalContext* inContext) = 0;
			/** @brief	The UAG session is retained. Pass NULL to remove the uag session */
	virtual	XBOX::VError			SetUAGSession(const XBOX::VJSGlobalContext* inContext, CUAGSession *inUAGSession) = 0;
			/** @brief	set the UAGSession for the context */
	virtual	XBOX::VError			ReleaseExpiredUAGSessions() = 0;
			/** @brief	Release all the sessions of the current running solution which have expired */
	virtual	XBOX::VError			RetainSessions(const XBOX::VUUID& inUserID, std::vector<XBOX::VRefPtr<CUAGSession> >& outSessions) = 0;
			/** @brief	retainq all the sessions of the current running solution which belongs to inUserID or all of them if inUserID.IsNull() */


	virtual bool					AcceptRestConnection(const XBOX::VString& inUserName, const XBOX::VString& inPassWord) = 0;
			/** @brief	the application (usually 4D) should check is a REST connection
						is accepted for this user/password
			*/

	virtual bool					AcceptNewRestConnection() = 0;
			/** @brief	the application (usually 4D) should increment the number of connection and  check if the limit is reached
						it should return false if the limit is reached
			*/

	virtual void					CloseSomeRestConnection(sLONG HowMany) = 0;
			/** @brief	the application (usually 4D) should decrement the number of connection by HowMany
			*/

	virtual XBOX::VError			Get4DMethods(std::vector<Public4DMethodInfo>& outMethods) = 0;
			/** @brief	the application (usually 4D) should return the list of all exposed methods
			*/

	virtual XBOX::VError			 Call4DMethod(const XBOX::VString& MethodName, XBOX::VJSONArray* params, CDB4DTable* inTable, CDB4DSelection* inSel, CDB4DRecord* inRec, XBOX::VJSONValue& outResult, CDB4DBaseContext* context) = 0;
			/** @brief	the application should call a 4D method whose name is MethodName
				params can be nil
				inTable, inSel and inRec can be nil

				the result of the execution will be in outResult
			*/
};

// RetainView() and RetainViewField() return NULL if the base UUID and/or index are not incorrect.

class DB4D_API IDB4D_SQLIntf
{
public:

	// OpenBase() is of no use, except to check (in debugging mode) that no view for the base to open already exists.

	virtual void			OpenBase (const XBOX::VUUID &inBaseUUID) = 0;

	// CloseBase() will remove all view(s) of the base to close from view set.

	virtual	void			CloseBase (const XBOX::VUUID &inBaseUUID) = 0;

	// Method for information retrieval for _USER_VIEWS table.

	virtual	sLONG			NumberViews (const XBOX::VUUID &inBaseUUID) = 0;
	virtual XBOX::VValueBag	*RetainView (const XBOX::VUUID &inBaseUUID, sLONG inN) = 0;

	// Method for information retrieval for _USER_VIEW_COLUMNS table.

	virtual sLONG			NumberViewFields (const XBOX::VUUID &inBaseUUID) = 0;
	virtual	XBOX::VValueBag	*RetainViewField (const XBOX::VUUID &inBaseUUID, sLONG inN) = 0;
};

/**
 * \brief This interface describes a way to set and and retrieve backup related settings.
 * \detail Backup settings feature an overriding/inheritance mechanism.
 *  Instance A can be set as instance B 'parent'. The rationale is that A (the parent) will be looked
 *  up whenever B does not have a setting defined.
 *  Backups are stored in a specific folder which can be specified.
 * By default successive backups will overwrite one another. If that behaviour is
 * not desirable one can specify to use unique names for backups by storing them
 * under timestamped sub-folders inside the destination folder:
 * <code>
 * |-MyBackupFolder
 * |
 * |-Backup_06-06-2012_12:06:23
 * |    |
 * |    | backup content
 * |    
 * |-Backup_06-06-2012_18:06:23
 * |    |
 * |    | backup content
 * |    
 * |-Backup_06-06-2012_18:06:23_2
 * |    |
 * |    | backup content
 * |    
 * </code>
 */
class IBackupSettings : public XBOX::IRefCountable
{
public:
	IBackupSettings(){}
	virtual ~IBackupSettings(){}

	/**
	 * \brief Initialies the instance using a value bag
	 * \param inBaseFolder specifies the base folder to use if a relative backup folder is specified
	 */	
	virtual bool Initialize(const XBOX::VValueBag& inBag,const  XBOX::VFilePath& inBaseFolder) = 0;

	/**
	 * \brief Initializes/resets the instance using a JSON object
	 * \details this method can be called using only a subset of initializations keys enabling to partially reset the instance
	 * and preserve some values.
	 * The JS syntax is as follows:
	 * <code>
	 * {
	 *    destination: Folder(...),
	 *    backupRegistryFolder: Folder(...),
	 *    useUniqueNames: true|false
	 * }
	 * </code>
	 * \param inObject the JS object carrying the init values.
	 * \return false if the final object is invalid state after the new values have been taken into account
	 * \return true if the object is valid after the new values have been taken into account
	 * \see CheckValidity()
	 */
	virtual bool FromJSObject(const XBOX::VJSObject& inObject) = 0;

	/**
	 * \brief Sets this instance's parent and retain it
	 * \param inParent the address of the new parent. NULL means that the current parent instance must be released.
	 */
	virtual bool SetRetainedParent(const IBackupSettings* inParent ) = 0;

	/**
	 * \brief Returns a retained reference to the parent parent instance
	 * \return a retained reference or NULL if no parent instance is defined
	 */
	virtual const IBackupSettings*  RetainParent() = 0;
	
	/**
	 * \brief Sets the folder to use as a backup registry
	 * \details The backup registry folder is a special folder where the backup tools
	 * store and retrieve some informations such as the backup manifest registry.
	 * \param inBackupRegistryFolder the folder where the backup registry is located
	 */
	virtual bool SetBackupRegistryFolder(const XBOX::VFilePath& inBackupRegistryFolder) = 0;

	/**
	 * \brief Gets the folder to use as a backup registry
	 * \details The backup registry folder is a special folder where the backup tools
	 * store and retrieve some informations such as the backup manifest registry
	 * \param outBackupRegistryFolder the folder currently used to store the backup registry
	 * \return true if @c outBackupRegistryFolder is a valid
	 * \return false if @c outBackupRegistryFolder is not specified
	 */
	virtual bool GetBackupRegistryFolder(XBOX::VFilePath& outBackupRegistryFolder)const  = 0;

	/**
	 * \brief Sets the backup destination folder for this instance
	 * \details backups will be directly stored under this folder. 
	 * \see SetUseUniqueNames()
	 */
	virtual void SetDestination(const XBOX::VFilePath& inBackupFolder) = 0;
	
	/**
	 * \brief Returns the destination folder where backups are stored.
	 * \details If the instance does not have a proper value set the parent chain is walked up until
	 * no ancestor remain or a valid value is found
	 * \return true if the setting is defined and valid
	 * \return false if no setting was found
	 */
	virtual bool GetDestination(XBOX::VFilePath& inBackupFolder)const = 0;	

	/**
	 * \brief Returns if each backup should have a unique name
	 * \returns true if each backup should have a unique name
	 */
	virtual bool GetUseUniqueNames()const = 0;

	/**
	 * \brief Sets whether each backup should have a unique name
	 * \param inUseUniqueNames if true then backups should be stored in individual sub-directories ensuring that they can be distinguished e.g. timestamp)
	 * 
	 */
	virtual void SetUseUniqueNames(bool inUseUniqueNames) = 0;


	/**
	 * \brief Checks the setting validity for this instance
	 */
	virtual  XBOX::VError CheckValidity()const = 0;

	virtual void ToJSONString(XBOX::VString& outJSON)const = 0; 

	/**
	 * \brief Converts this instance into a JS object
	 * \param outObject the resulting object, must be passed empty on entry
	 * \see FromJSObject() for reslting syntax details
	 */
	virtual void ToJSObject(XBOX::VJSObject& outObject)const  = 0;
};




class CDB4DBaseContext;

/**
 * \brief Interface for journal-related services
 */
class DB4D_API IJournalTool
{
public:
	IJournalTool(){}
	virtual ~IJournalTool(){}

	/**
	 * \brief Opens the specified journal file and converts it into a JS object.
	 * \param inJournalFile the journal to convert
	 * \param outOperations the resulting array where each operation is stored as an entry
	 * \return true on success
	 */
	virtual bool ParseJournal(XBOX::VFile& inJournalFile,XBOX::VJSArray& outOperations,IDB4D_DataToolsIntf* inProgressLog) = 0;

	/**
	 * \brief Opens a journal file and converts it into a text file
	 * \param inJournalFile the journal to convert
	 * \param inDestinationFile the resulting text file
	 * \return true on success
	 */
	virtual bool ParseJournalToText(XBOX::VFile& inJournalFile,XBOX::VFile& inDestinationFile,IDB4D_DataToolsIntf* inProgressLog) = 0;
};

/**
 * \brief Interface for core backup related services
 * \details This class abstracts the backup processing for a database, in a variety of scenarii.
 * A backup is configured using backup settings (IBackupSettings) that control where and how backups are
 * performed.
 * Each successfull backup produces a backup manifest file which can be used later when restoring a datastore.
 * Backup manifests are stored along side the backup they describe. All backup manifest file paths are further stored
 * in a registry file of arbitrary name which parent folder is given by @c IBackupSettings::GetBackupRegistryFolder().
 */
class DB4D_API IBackupTool
{
public:

	/**
	 * Enumeration deteermining which item to restore
	 */
	typedef enum 
	{
		eDataFolder = 0x0001,
		eDataFile = 0x0002,
		eJournal = 0x0004
	} BackupItem;

	IBackupTool(){}
	virtual ~IBackupTool(){}

	/**
	 * \brief Converts the designated backup registry into a JS of manifest objects
	 * \param inBackupRegistryFilePath path to the backup registry file
	 * \param outManifests the JS array where the registry is stored
	 * \return true if the operation succeeded.
	 */
	virtual bool ConvertBackupRegistryToJsObject(const XBOX::VFilePath& inBackupRegistryFilePath,XBOX::VJSArray& outManifests) = 0;


	/**
	 * \brief Backs up a database and maintains the current journal configuration
	 * \details This method will backup the current journal if applicable, then it will archive the data.
	 * If ajournal was used, then after backing up it will be reset and re-activated.
	 * \param inBase the database to backup
	 * \param inContext the database context or NULL if not available
	 * \param inBackupSettings the backup configuration to use for this backup
	 * \param outManifestFilePath the resulting backup manifest fullpath
	 * \return true if the backup went fine
	 * \return false if an error occured in the process
	 */
	virtual bool BackupDatabase(CDB4DBase* inBase,CDB4DBaseContext* inContext,const IBackupSettings& inBackupSettings,XBOX::VFilePath* outManifestFilePath,IDB4D_DataToolsIntf* inBackupEventLog) = 0;

	/**
	 * \brief Backs up a database and change the journal according to settings
	 * \param inBase the database to backup
	 * \param inContext the database context or NULL if not available
	 * \param inBackupSettings the backup configuration to use for this backup
	 * \param inNextJournalPath the name of the journal to use after the backup has completed. If NULL then journaling *WILL BE DISABLED*.
	 * \param outManifestFilePath the resulting backup manifest fullpath
	 * \return true if the backup went fine
	 * \return false if an error occured in the process
	 * \see BackupDatabase()
	 */
	virtual bool BackupDatabaseAndChangeJournal(CDB4DBase* inBase,CDB4DBaseContext* inContext,const IBackupSettings& inBackupSettings,const XBOX::VFilePath* inNextJournalPath,XBOX::VFilePath* outManifestFilePath,IDB4D_DataToolsIntf* inBackupEventLog) = 0;
	
	/**
	 * \brief Backs up a database without applying journaling-policy changes.
	 * \param inBase the database to backup
	 * \param inJournalToArchive an optional path to the journal to archive in this backup
	 * \param inBackupSettings the backup configuration to use for this backup
	 * \return true if the backup went fine
	 * \return false if an error occured in the process
	 */
	virtual bool BackupDatabaseAndDontResetJournal(CDB4DBase* inBase,const XBOX::VFilePath& inJournalToArchive,const IBackupSettings& inBackupSettings,XBOX::VFilePath* outManifestFilePath,IDB4D_DataToolsIntf* inBackupEventLog) = 0;


	/**
	 * \brief Backs up a closed database 
	 * \details This method will backup the database journal (if applicable) and data folder.
	 * After backup if the database was using a journal, the latter will be reset
	 * \param inModelFile the database model/structure file 
	 * \param inDataFile the database data file
	 * \param inContext the database context or NULL if not available
	 * \param inBackupSettings the backup configuration to use for this backup
	 * \param outManifestFilePath the resulting backup manifest fullpath
	 * \return true if the backup went fine
	 * \return false if an error occured in the process
	 */
	virtual bool BackupClosedDatabase(XBOX::VFile& inModelFile,XBOX::VFile& inDataFile,CDB4DBaseContext* inContext,const IBackupSettings& inBackupSettings,XBOX::VFilePath* outManifestFilePath,IDB4D_DataToolsIntf* inBackupEventLog) = 0;

	/**
	 * \returns the full path of the folder containing the last backup location	
	 * \param inBackupSettings settings used for the last backup
	 * \param outLastBackupLocation on success contains the full path to the folder storing the last backup
	 * \return VE_OK if everything went fine
	 * \return VE_FOLDER_NOT_FOUND if the information cannot be retrieved
	 */
	virtual XBOX::VError GetLastBackupPath(const IBackupSettings& inBackupSettings,XBOX::VFilePath& outLastBackupLocation) = 0;

	/**
	 * \brief Restores the data file from a backup archive
	 * \param inBackupManifestPath the path the backup manifest
	 * \param inRestoredFilePath the full filepath of the restored file. 
	 * \param inOverwrite if true then if when any file named @c inRestoredFilePath will be overwritten otherwise the extract will fail
	 * \param inProgress the progress object to give feedback about the extraction
	 * \return true if the restoration succeeded return false otherwise
 	 */
	virtual bool RestoreDataFile(const XBOX::VFilePath& inBackupManifestPath,const XBOX::VFilePath& inRestoredFilePath,IDB4D_DataToolsIntf* inProgress,bool inOverwrite) = 0;

	/**
	 * \brief Restores the data file from a backup archive
	 * \param inBackupManifestPath the the backup manifest path  
	 * \param inRestoredDataFolderParentPath Path of the folder where the data folder will be restored 
	 * \param outRenamedDataFolderPath stores the name of the existing data folder in @inRestoredDataFolderPath, if any.
	 * \param inProgress the progress object to give feedback about the extraction
	 * \return true if the restoration succeeded return false otherwise
	 * Example
	 * <pre>
	 * VFilePath folder;
	 * folder.FromFullPath(CVSTR("c:/MyProject/"));
	 * backupTool->RestoreDataFolder(myManifest,folder,renamed,NULL);
	 * 
	 * Will create the following:
	 * c:/
	 * |-MyProject
	 * |     |-DataFolder                                <--- this is the restored data folder
	 * |     |    |-data.waData
	 * |     |    |-index.waIndex
	 * |     |
	 * |     |-DataFolder_REPLACED_2012-12-25_09-36-23   <--- that was the pre-existing data folder, renamed and returned in outRenamedDataFolderPath
	 * |     |    |-data.waData
	 * |     |    |-index.waIndex
	 * </pre>
 	 */
	virtual bool RestoreDataFolder(const XBOX::VFilePath& inBackupManifestPath,const XBOX::VFilePath& inRestoredDataFolderParentPath,XBOX::VFilePath& outRenamedDataFolderPath,IDB4D_DataToolsIntf* inProgress) = 0;

	/**
	 * \brief Restores the journal file from a backup archive
	 * \param inBackupManifestPath the the backup manifest path  
	 * \param inRestoreFolderPath Path of the folder where the journal file  
	 * \param outRenamedJournalPath stores the name of any existing journal file in inRestoreFolderPath, if applicable.
	 * \param inProgress the progress object to give feedback about the extraction
	 * \return true if the restoration succeeded return false otherwise
	 * Example
	 * <pre>
	 * VFilePath folder;
	 * folder.FromFullPath(CVSTR("c:/MyProject/"));
	 * backupTool->RestoreDataFolder(myManifest,folder,renamed,NULL);
	 * 
	 * Will create the following:
	 * c:/
	 * |-MyProject
	 * |     |-journal.waJournal                                <--- this is the restored journal
	 * |     |
	 * |     |-journal_REPLACED_2012-12-25_09-36-23.waJournal   <--- that was the pre-existing journal file after being renamed and returned in outRenamedJournalPath
	 * </pre>
 	 */
	virtual bool RestoreJournal(const XBOX::VFilePath& inBackupManifestPath,const XBOX::VFilePath& inRestoreFolder,XBOX::VFilePath& outRenamedJournalPath,IDB4D_DataToolsIntf* inProgress) = 0;
};

// ----------  [12/4/2008 LR]
// ce code est amene a disparaitre : L.R le 4 dec 2008

typedef std::multimap<XBOX::VString, XBOX::VString> tempHTTpHeaderMap;

class DB4D_API tempDB4DWebRequest
{
	public:

		virtual XBOX::VString* GetURL() = 0;
		virtual XBOX::VStream* GetRequestStream() = 0;
		virtual XBOX::VStream* GetAnswerStream() = 0;
		virtual tempHTTpHeaderMap* GetRequestHeader() = 0;
		virtual tempHTTpHeaderMap* GetAnswerHeader() = 0;
		virtual XBOX::VString* GetHostName() = 0;
		virtual XBOX::VString* GetUserName() = 0;
		virtual XBOX::VString* GetPassword() = 0;
};

// fin du code amene a disparaitre
// ----------------------------------------------




// for now it is the same
typedef XBOX::VProgressIndicator VDB4DProgressIndicator; 

typedef enum {
	DB4D_NoType = XBOX::VK_EMPTY,	DB4D_StrFix = XBOX::VK_STRING,
	DB4D_Text = XBOX::VK_TEXT,
	DB4D_Date = -1, // obsolete
	DB4D_Time = XBOX::VK_TIME,
	DB4D_Duration = XBOX::VK_DURATION,
	DB4D_Boolean = XBOX::VK_BOOLEAN,
	DB4D_Byte = XBOX::VK_BYTE,
	DB4D_Integer16 = XBOX::VK_WORD,
	DB4D_Integer32 = XBOX::VK_LONG,
	DB4D_Integer64 = XBOX::VK_LONG8,
	DB4D_Real = XBOX::VK_REAL,
	DB4D_Float = XBOX::VK_FLOAT,
	DB4D_Money = -2, // pas encore implemente
	DB4D_UUID = XBOX::VK_UUID,

	DB4D_Blob = XBOX::VK_BLOB_DB4D,
	DB4D_Picture = XBOX::VK_IMAGE,
	DB4D_Sound = -3, // pas encore implemente
	DB4D_SubTable = XBOX::VK_SUBTABLE,
	DB4D_SubTableKey = XBOX::VK_SUBTABLE_KEY,

	DB4D_LastType = 100,
	DB4D_NullType = -1
} __DB4DFieldType;

typedef uCHAR	DB4DFieldType;

typedef enum {
	DB4D_NOTCONJ = 0,
	DB4D_And = 1,
	DB4D_OR,
	DB4D_Except,
	DB4D_Not,
	DB4D_Intersect
} DB4DConjunction;

typedef enum {
	DB4D_NOCOMP = 0,
	DB4D_Equal = 1,
	DB4D_NotEqual,
	DB4D_Greater,
	DB4D_GreaterOrEqual,
	DB4D_Lower,
	DB4D_LowerOrEqual,
	DB4D_Contains_KeyWord,
	DB4D_DoesntContain_KeyWord,
	DB4D_BeginsWith,
	DB4D_Contains_KeyWord_BeginingWith,
	DB4D_EndsWith,
	DB4D_IN,
	DB4D_Like,
	DB4D_NotLike,
	DB4D_Greater_Like,
	DB4D_GreaterOrEqual_Like,
	DB4D_Lower_Like,
	DB4D_LowerOrEqual_Like,
	DB4D_Contains_KeyWord_Like,
	DB4D_Doesnt_Contain_KeyWord_Like,
	DB4D_Regex_Match,
	DB4D_Regex_Not_Match,
	DB4D_IsNull,
	DB4D_IsNotNull


} DB4DComparator;


typedef enum {
//	DB4D_Unused			= 1,	// previously DB4D_Invisible
	DB4D_Not_Null		= 2,
//	DB4D_Unused			= 4,	// previously DB4D_Modifiable
//	DB4D_Unused			= 8,	// previously DB4D_Enterable
	DB4D_Unique			= 16,
	DB4D_AutoSeq		= 32,
	DB4D_AutoGenerate	= 64,
	DB4D_StoreAsUTF8	= 128,
	DB4D_StoreAsUUID	= 256,
	DB4D_StoreOutsideBlob	= 512
} __DB4DFieldAttributes;
typedef uLONG DB4DFieldAttributes;

/*
typedef enum { Relation_1To1 = 1, 
							 Relation_1ToN = 2, 
							 Relation_NTo1 = 3,
							 Relation_LastType = 4
} DB4D_RelationType;
*/

typedef enum { RelationState_FromSource = 1, 
				 RelationState_ReadOnly = 2, 
				 RelationState_ReadWrite = 3
} DB4D_RelationState;


typedef enum { DB4D_Sel_Bittab = 1, DB4D_Sel_LongSel = 2, DB4D_Sel_SmallSel = 3,  DB4D_Sel_OneRecSel = 4} DB4D_SelectionType;

typedef enum { DB4D_AutoSeq_Simple = 1, DB4D_AutoSeq_NoHole = 2 } DB4D_AutoSeq_Type;



typedef enum {  DB4D_Log_CreateRecord = 1,
				DB4D_Log_ModifyRecord = 2,
				DB4D_Log_DeleteRecord = 3,
				DB4D_Log_StartTrans = 4,
				DB4D_Log_Commit = 5,
				DB4D_Log_RollBack = 6,
				DB4D_Log_OpenData = 7,
				DB4D_Log_CloseData = 8,
				DB4D_Log_AddTable = 9,
				DB4D_Log_ModifTable = 10,
				DB4D_Log_DelTable = 11,
				DB4D_Log_AddField = 12,
				DB4D_Log_ModifField = 13,
				DB4D_Log_DelField = 14,
				DB4D_Log_CreateContextWithUserUUID = 15,	// not used anymore
				DB4D_Log_CloseContext = 16,
				DB4D_Log_CreateBlob = 17,
				DB4D_Log_ModifyBlob = 18,
				DB4D_Log_DeleteBlob = 19,
				DB4D_Log_StartBackup = 20,
				DB4D_Log_TruncateTable = 21,
				DB4D_Log_CreateContextWithExtra = 22,
				DB4D_Log_SaveSeqNum = 23,

				DB4D_Log_None = 100

} DB4D_LogAction;


/*	<CacheLog> See VCacheLog.h/.cpp for usage of those constants.
	"CL" in "eCL..." goes for "CacheLog"
	
	********** WARNING - WARNING - WARNING - WARNING - WARNING **********
	Do not change this enum, the values are used by code that import the log
*/
typedef enum
{
	eCLEntryKind_Unknown			= 0,
	eCLEntryKind_Start,
	eCLEntryKind_Stop,
	eCLEntryKind_Comment,

	eCLEntryKind_NeedsBytes			= 100,		// Cache manager computes space
	eCLEntryKind_CallNeedsBytes,				// Memory manager requests space

	eCLEntryKind_FlushFromLanguage		= 200,
	eCLEntryKind_FlushFromMenuCommand,
	eCLEntryKind_FlushFromScheduler,
	eCLEntryKind_FlushFromBackup,
	eCLEntryKind_FlushFromNeedsBytes,
	eCLEntryKind_FlushFromRemote,
	eCLEntryKind_FlushFromUnknown,
	
	eCLEntryKind_Flush				= 300,

	eCLEntryKind_MemStats			= 400

} ECacheLogEntryKind;

enum {
	eCLDumpStats_Mini			= 1,
	eCLDumpStats_Objects		= 2,
	eCLDumpStats_Blocks			= 4,
	eCLDumpStats_SmallBlocks	= 8,
	eCLDumpStats_BigBlocks		= 16,
	eCLDumpStats_OtherBlocks	= 32,

	eCLDumpStats_All			= 0xFFFFFFFF,
};
typedef uLONG		ECacheLogMemStatsLevel;
// </CacheLog>

typedef enum {  DB4D_LockedByNone = 0,
				DB4D_LockedByRecord = 1,
				DB4D_LockedBySelection = 2,
				DB4D_LockedByTable = 3,
				DB4D_LockedByDataBase = 4,
				DB4D_LockedByDeletedRecord = 5,
				DB4D_LockedNoCurrentRecord = 6
} DB4D_KindOfLock;

const uLONG kTagLogDB4D = 'db4d';
const uLONG kTagLogDB4DEnd = 'DB4D';


const sLONG DB4D_Index_None = 0;
const sLONG DB4D_Index_OnOneField = 1;
const sLONG DB4D_Index_OnMultipleFields = 2;
const sLONG DB4D_Index_OnAnExpression = 3;
const sLONG DB4D_Index_OnKeyWords = 4; // fulltext index

const sLONG DB4D_Index_NoStorage = 0;
const	sLONG DB4D_Index_Btree = 1;
const	sLONG DB4D_Index_Htable = 2;
const	sLONG DB4D_Index_BtreeWithCluster = 3; // default
const	sLONG DB4D_Index_BtreeWithFixedSizePages = 5;
const	sLONG DB4D_Index_BtreeWithClusterAndFixedSizePages = 6;
const	sLONG DB4D_Index_AutoType = 7;
const	sLONG DB4D_Index_DefaultType = DB4D_Index_AutoType;	// this is what 4D uses when converting, from structure editor and from the SET INDEX command


typedef enum
{
	DB4D_DataFolder = 0,
	DB4D_StructureFolder = 2,
	DB4D_SystemTemporaryFolder = 3,
	DB4D_CustomFolder = 4,
	DB4D_DefaultTemporaryFolder = DB4D_DataFolder
} DB4DFolderSelector;


typedef enum
{
	DB4D_Do_Not_Lock = 0,
	DB4D_Keep_Lock_With_Record,
	DB4D_Keep_Lock_With_Selection,
	DB4D_Keep_Lock_With_Transaction
} DB4D_Way_of_Locking;

// If you change this enum, think about updating 4D's code (VValueController_ProcessProxy and eventually VObjectController_Process)
enum
{
	kDB4DTaskKind_IndexBuilder		= 'DB4I',
	kDB4DTaskKind_FlushCache		= 'DB4F',
	kDB4DTaskKind_GarbageCollector	= 'DB4G',
	kDB4DTaskKind_WorkerPool_User	= 'DB4U',
	kDB4DTaskKind_Cron				= 'DB4C',
};

enum
{
	DB4D_SyncAction_Update = 1,
	DB4D_SyncAction_Delete = 2
};

/*
typedef sLONG DB4D_BaseID;
const DB4D_BaseID kDB4D_NullBaseID = DB4D_BaseID(-1);
*/

typedef sLONG DB4D_TableID;
const DB4D_TableID kDB4D_NullTableID = DB4D_TableID(0);

typedef sLONG DB4D_FieldID;
const DB4D_FieldID kDB4D_NullFieldID = DB4D_FieldID(0);

const RecIDType kDB4D_NullRecordID = -1;

class CDB4DBase;
class CDB4DBaseContext;
class CDB4DQuery;
class CDB4DSqlQuery;
class CDB4DRecord;
class CDB4DImpExp;
class CDB4DCheckAndRepairAgent;
class CDB4DAutoSeqNumber;

typedef CDB4DBaseContext *CDB4DBaseContextPtr;

class CDB4DSortingCriterias;
typedef CDB4DSortingCriterias *CDB4DSortingCriteriasPtr;

class CDB4DSelection;
typedef CDB4DSelection *CDB4DSelectionPtr;

class CDB4DRelation;
class CDB4DContext;

class CDB4DTable;
class CDB4DField;

class CDB4DColumnFormula;
class CDB4DQueryResult;
class CDB4DQueryOptions;

class CachedRelatedRecord;

class CDB4DEntityModel;
class CDB4DEntityAttribute;
class CDB4DEntityMethod;
class CDB4DEntityAttributeValue;
class CDB4DEntityRecord;


class EntityAttributeSortedSelection;
class EntityAttributeSelection;

enum {
	TUB_NameChanged						= 1,
	TUB_ExtraPropertiesChanged			= 2,
	TUB_end
};
typedef uLONG	ETellUpdateBaseOptions;


enum {
	TUT_NameChanged						= 1,
	TUT_ExtraPropertiesChanged			= 2,
	TUT_end
};
typedef uLONG	ETellUpdateTableOptions;


enum {
	TUF_NameChanged						= 2,
	TUF_TypeChanged						= 4,
	TUF_ExtraPropertiesChanged			= 8,
	TUF_end
};
typedef uLONG	ETellUpdateFieldOptions;


enum {
	TUI_NameChanged						= 1,
	TUI_ExtraPropertiesChanged			= 2,
	TUI_end
};
typedef uLONG	ETellUpdateIndexOptions;


enum {
	TUR_NameChanged						= 1,
	TUR_ExtraPropertiesChanged			= 2,
	TUR_end
};
typedef uLONG	ETellUpdateRelationOptions;


enum
{
	VN_LegacyLanguageCompatible			= 1,
	VN_ObjectLanguageCompatible			= 2,
	VN_SqlCompatible					= 3,
	VN_end
};
typedef uLONG EValidateNameOptions;


typedef XBOX::VArrayRetainedOwnedPtrOf<CDB4DRelation*> RelationArray;
typedef XBOX::VArrayOf<CDB4DField*> CDB4DFieldArray;


typedef enum
{
	DB4D_Pseudo_Field_RecordNum = 0
} DB4D_Pseudo_Field_Kind;

typedef enum
{
	DB4D_EM_None_Perm = 0,
	DB4D_EM_Read_Perm,
	DB4D_EM_Describe_Perm,
	DB4D_EM_Create_Perm,
	DB4D_EM_Update_Perm,
	DB4D_EM_Delete_Perm,
	DB4D_EM_Execute_Perm,
	DB4D_EM_Promote_Perm

} DB4D_EM_Perm;


typedef enum
{
	DB4D_Running4D = 0,
	DB4D_RunningWakanda,
} DB4D_Running_Mode;


class DB4D_API DB4DCommandManager
{
public:
	virtual void Tell_AddBase( const XBOX::VUUID& inBaseUUID) = 0;
	virtual void Tell_CloseBase( const XBOX::VUUID& inBaseUUID) = 0;
	virtual void Tell_UpdateBase( const XBOX::VUUID& inBaseUUID, ETellUpdateBaseOptions inWhat) = 0;
	
	virtual void Tell_AddTable( const XBOX::VUUID& inBaseUUID, const XBOX::VUUID& inTableUUID, sLONG inTableID) = 0;
	virtual void Tell_DelTable( const XBOX::VUUID& inBaseUUID, const XBOX::VUUID& inTableUUID, sLONG inTableID) = 0;
	virtual void Tell_UpdateTable( const XBOX::VUUID& inBaseUUID, const XBOX::VUUID& inTableUUID, sLONG inTableID, ETellUpdateTableOptions inWhat) = 0;
	
	virtual void Tell_AddField( const XBOX::VUUID& inBaseUUID, const XBOX::VUUID& inTableUUID, const XBOX::VUUID& inFieldUUID, sLONG inTableID, sLONG inFieldID) = 0;
	virtual void Tell_DelField( const XBOX::VUUID& inBaseUUID, const XBOX::VUUID& inTableUUID, const XBOX::VUUID& inFieldUUID, sLONG inTableID, sLONG inFieldID) = 0;
	virtual void Tell_UpdateField( const XBOX::VUUID& inBaseUUID, const XBOX::VUUID& inTableUUID, const XBOX::VUUID& inFieldUUID, sLONG inTableID, sLONG inFieldID, ETellUpdateFieldOptions inWhat) = 0;
	
	virtual void Tell_AddIndex( const XBOX::VUUID& inBaseUUID, const XBOX::VUUID& inIndexUUID) = 0;
	virtual void Tell_DelIndex( const XBOX::VUUID& inBaseUUID, const XBOX::VUUID& inIndexUUID) = 0;
	virtual void Tell_UpdateIndex( const XBOX::VUUID& inBaseUUID, const XBOX::VUUID& inIndexUUID, ETellUpdateIndexOptions inWhat) = 0;

	virtual void Tell_AddRelation( const XBOX::VUUID& inBaseUUID, const XBOX::VUUID& inRelationID) = 0;
	virtual void Tell_DelRelation( const XBOX::VUUID& inBaseUUID, const XBOX::VUUID& inRelationID) = 0;
	virtual void Tell_UpdateRelation( const XBOX::VUUID& inBaseUUID, const XBOX::VUUID& inRelationID, ETellUpdateRelationOptions inWhat) = 0;
};

const sWORD DB4D_Default_Server_Port = 4449;

enum
{
	Legacy_NetWorkManager				= 1,
	DB4D_NetworkManager					= 2,
	End_NetworkManager
};
typedef uLONG NetWorkManager_Type;


class DB4D_API DB4DNetworkManager
{
public:
	virtual XBOX::IStreamRequest*	CreateRequest( CDB4DBaseContext *inContext, sWORD inRequestID) = 0;
	virtual	void					GetServerAddressAndPort( XBOX::VString& outServerAddress, sLONG *outApplicationServerPortNumber, sLONG *outDB4DServerPortNumber, bool* outUseSSL) = 0;
	virtual	sLONG					GetServerVersion() = 0;
	virtual NetWorkManager_Type		GetType() const = 0;
	virtual void					Dispose() = 0;
};


class DB4D_API DB4DSignaler
{
public:
	typedef XBOX::VSignalT_1<const XBOX::VUUID*>																VSignal_AddBase;
	typedef XBOX::VSignalT_1<const XBOX::VUUID*>																VSignal_CloseBase;
	typedef XBOX::VSignalT_2<const XBOX::VUUID*,ETellUpdateBaseOptions>											VSignal_UpdateBase;

	typedef XBOX::VSignalT_2<const XBOX::VUUID*,const XBOX::VUUID*>												VSignal_AddTable;
	typedef XBOX::VSignalT_2<const XBOX::VUUID*,const XBOX::VUUID*>												VSignal_DelTable;
	typedef XBOX::VSignalT_3<const XBOX::VUUID*,const XBOX::VUUID*,ETellUpdateTableOptions>						VSignal_UpdateTable;

	typedef XBOX::VSignalT_3<const XBOX::VUUID*,const XBOX::VUUID*,const XBOX::VUUID*>							VSignal_AddField;
	typedef XBOX::VSignalT_3<const XBOX::VUUID*,const XBOX::VUUID*,const XBOX::VUUID*>							VSignal_DelField;
	typedef XBOX::VSignalT_4<const XBOX::VUUID*,const XBOX::VUUID*,const XBOX::VUUID*,ETellUpdateFieldOptions>	VSignal_UpdateField;

	typedef XBOX::VSignalT_2<const XBOX::VUUID*,const XBOX::VUUID*>												VSignal_AddIndex;
	typedef XBOX::VSignalT_2<const XBOX::VUUID*,const XBOX::VUUID*>												VSignal_DelIndex;
	typedef XBOX::VSignalT_3<const XBOX::VUUID*,const XBOX::VUUID*,ETellUpdateIndexOptions>						VSignal_UpdateIndex;

	typedef XBOX::VSignalT_2<const XBOX::VUUID*,const XBOX::VUUID*>												VSignal_AddRelation;
 	typedef XBOX::VSignalT_2<const XBOX::VUUID*,const XBOX::VUUID*>												VSignal_DelRelation;
 	typedef XBOX::VSignalT_3<const XBOX::VUUID*,const XBOX::VUUID*,ETellUpdateRelationOptions>					VSignal_UpdateRelation;

	
	virtual VSignal_AddBase*		GetSignal_AddBase() = 0;
	virtual VSignal_CloseBase*		GetSignal_CloseBase() = 0;
	virtual VSignal_UpdateBase*		GetSignal_UpdateBase() = 0;

	virtual VSignal_AddTable*		GetSignal_AddTable() = 0;
	virtual VSignal_DelTable*		GetSignal_DelTable() = 0;
	virtual VSignal_UpdateTable*	GetSignal_UpdateTable() = 0;

	virtual VSignal_AddField*		GetSignal_AddField() = 0;
	virtual VSignal_DelField*		GetSignal_DelField() = 0;
	virtual VSignal_UpdateField*	GetSignal_UpdateField() = 0;

	virtual VSignal_AddIndex*		GetSignal_AddIndex() = 0;
	virtual VSignal_DelIndex*		GetSignal_DelIndex() = 0;
	virtual VSignal_UpdateIndex*	GetSignal_UpdateIndex() = 0;

	virtual VSignal_AddRelation*	GetSignal_AddRelation() = 0;
 	virtual VSignal_DelRelation*	GetSignal_DelRelation() = 0;
 	virtual VSignal_UpdateRelation*	GetSignal_UpdateRelation() = 0;
};

typedef XBOX::VReadWriteActivityIndicator DB4DActivityManager;

// ATTENTION : Collections Element numbering starts from 1 ( from 1 to size elements)
//           : Column numbering also starts from 1

class DB4D_API DB4DCollectionManager : public XBOX::IRefCountable
{
	public:
		virtual VErrorDB4D SetCollectionSize(RecIDType size, Boolean ClearData = true) = 0;
		virtual RecIDType GetCollectionSize() = 0;
		virtual sLONG GetNumberOfColumns() = 0;
		virtual	bool AcceptRawData() = 0;
		virtual CDB4DField* GetColumnRef(sLONG ColumnNumber) = 0;
		virtual VErrorDB4D SetNthElement(RecIDType ElemNumber, sLONG ColumnNumber, const XBOX::VValueSingle& inValue) = 0;
		
		/*
			very similar to VValue::WriteToPtr.
			Writes the value to outRawValue and returns the adress pointing after what has been written.
			*outRejected is set to true if unsupported kind.
		*/
		virtual	XBOX::VSize GetNthElementSpace( RecIDType inIndex, sLONG inColumnNumber, XBOX::ValueKind inRawValueKind, bool *outRejected, XBOX::VError& outError) = 0;
		virtual	void* WriteNthElementToPtr( RecIDType inIndex, sLONG inColumnNumber, void *outRawValue, XBOX::ValueKind inRawValueKind, bool *outRejected, XBOX::VError& outError) = 0;

		// set *outRejected to true if you are not pleased with given raw data and want to get called with SetNthElement instead for this row (already initialized to false)
		virtual VErrorDB4D SetNthElementRawData(RecIDType ElemNumber, sLONG ColumnNumber, const void *inRawValue, XBOX::ValueKind inRawValueKind, bool *outRejected) = 0;

		virtual VErrorDB4D GetNthElement(RecIDType ElemNumber, sLONG ColumnNumber, const XBOX::VValueSingle*& outValue, bool *outDisposeIt) = 0;
		virtual VErrorDB4D AddOneElement(sLONG ColumnNumber, const XBOX::VValueSingle& inValue) = 0;
		virtual sLONG GetSignature() = 0;
		virtual XBOX::VError PutInto(XBOX::VStream& outStream) = 0;
//		virtual XBOX::VError GetFrom(XBOX::VStream& inStream) = 0;
};

// ATTENTION : Collections Element numbering starts from 1 ( from 1 to size elements)
//           : Column numbering also starts from 1
// inStream may be NULL.

typedef DB4DCollectionManager* (*DB4DCollectionManagerCreator)(sLONG signature, XBOX::VStream *inStream, XBOX::VError& outError); // Ptr to Func used to build a DB4DCollectionManager based on signature


typedef enum { DB4D_SaveNewRecord_Trigger = 1,
							 DB4D_SaveExistingRecord_Trigger = 2,
							 DB4D_DeleteRecord_Trigger = 3} DB4D_TriggerType;

class DB4D_API DB4DTriggerManager
{
	public:
		virtual VErrorDB4D CallTrigger(DB4D_TriggerType WhatTrigger, CDB4DRecord* OnWhichRecord, CDB4DBaseContext* inContext) = 0;
		virtual Boolean IsTriggerExisting(DB4D_TriggerType WhatTrigger, sLONG onWhatTableNumber) = 0;
};

// DataTools J.A.
	
typedef struct BaseStatistics
{
	uLONG8 datafreebits;
	uLONG8 datausedbits;
	uLONG8 structfreebits;
	uLONG8 structusedbits;
	uLONG8 indexdatafreebits;
	uLONG8 indexdatausedbits;
	uLONG8 indexstructfreebits;
	uLONG8 indexstructusedbits;
} BaseStatistics;


class DB4D_API DB4DArrayOfValues : public XBOX::IRefCountable
{
public:
	virtual Boolean Find(XBOX::VValueSingle& inValue, const XBOX::VCompareOptions& inOptions) const = 0;
	virtual Boolean FindWithDataPtr(void* inDataPtr, const XBOX::VCompareOptions& inOptions) const = 0;
	virtual const void* GetFirstPtr() const = 0; 
	virtual sLONG Count() const = 0;
	virtual XBOX::VError PutInto(XBOX::VStream& outStream) const = 0;
//	virtual XBOX::VError GetFrom(XBOX::VStream& inStream) = 0;
	virtual sLONG GetSignature() const = 0;
	virtual Boolean IsAllNull() const = 0;
	virtual Boolean AtLeastOneNull() const = 0;
	virtual Boolean CanBeUsedWithIndex(XBOX::VIntlMgr* inIntlMgr) const = 0;
};


typedef DB4DArrayOfValues* (*DB4DArrayOfValuesCreator)(sLONG signature, XBOX::VStream *inStream, XBOX::VError& outError); // Ptr to Func used to build a DB4DArrayOfValues based on signature, inStream may be NULL

typedef std::vector<sLONG> StampsVector;

typedef std::vector<sLONG> SubRecordIDVector;

class CDB4DQueryPathModifiers;


typedef enum
{
	DB4D_QPE_none = 0,
	DB4D_QPE_scalar,
	DB4D_QPE_array,
	DB4D_QPE_object
} DB4D_QueryParamElementType;


class ENTITY_API QueryParamElement
{
	public:
		QueryParamElement(XBOX::VValueSingle* inCV, bool ownsIt = true)
		{
			fScalar = inCV;
			fSimpleDateScalar = nil;
			fType = DB4D_QPE_scalar;
			fArray = nil;
		}

		QueryParamElement(XBOX::VValueSingle* inCV, XBOX::VValueSingle* inSimpleDate, bool ownsIt = true)
		{
			fScalar = inCV;
			fSimpleDateScalar = inSimpleDate;
			fType = DB4D_QPE_scalar;
			fArray = nil;
		}


		QueryParamElement(XBOX::VJSArray& inArray);
		

		QueryParamElement()
		{
			fScalar = nil;
			fSimpleDateScalar = nil;
			fType = DB4D_QPE_none;
			fArray = nil;
		}
		

		void Dispose();
		void Duplicate();
		

		DB4D_QueryParamElementType fType;
		XBOX::VJSArray* fArray;
		XBOX::VValueSingle* fScalar;
		XBOX::VValueSingle* fSimpleDateScalar;
};

class DB4D_API QueryParamElementVector : public std::vector<QueryParamElement>
{
	public:
		QueryParamElementVector()
		{
			fOwned = true;
		}

		void MustNotDisposeElements()
		{
			fOwned = false;
		}

		bool fOwned;

		~QueryParamElementVector()
		{
			if (fOwned)
			{
				for (QueryParamElementVector::iterator cur = begin(), last = end(); cur < last; ++cur)
				{
					cur->Dispose();
				}
			}
		}
};


class DB4D_API DB4DLanguageExpression
{
public:
	virtual void Retain() = 0;
	virtual void Release() = 0;
	virtual const XBOX::VValueSingle* Execute(CDB4DBaseContext* inBaseContext, void* &ioLanguageContext, CDB4DRecord* onWhichRecord, VErrorDB4D& outErr, std::vector<CachedRelatedRecord> *RelatedRecords = NULL) = 0;
	// ioLanguageContext is NULL the first time, it is computed from inBaseContext and returned by Execute, then it is cached by DB4D and reused each time

	virtual uBOOL ExecuteForQuery(CDB4DBaseContext* inBaseContext, void* &ioLanguageContext, CDB4DRecord* onWhichRecord, VErrorDB4D& outErr) = 0;
	// ioLanguageContext is NULL the first time, it is computed from inBaseContext and returned by Execute, then it is cached by DB4D and reused each time

	virtual VErrorDB4D GetDescription(XBOX::VString& outText) = 0;

	virtual VErrorDB4D PutInto(XBOX::VStream& into) = 0;

	virtual void ResetSelectionsAndRecords(CDB4DBaseContext* inBaseContext, void* &ioLanguageContext, DB4D_TableID inMaintable, VErrorDB4D& outErr, std::vector<CachedRelatedRecord> *inRelatedRecords) = 0;
};

typedef DB4DLanguageExpression* (*BuildLanguageExpressionMethod) (XBOX::VStream& from, VErrorDB4D& outError);


/*
class DB4D_API DB4D_RecordRef
{
	RecIDType fRecNum;
	sLONG fTableNum;
};
*/

typedef XBOX::VUUIDBuffer DB4D_RecordRef;

typedef sLONG8 DB4D_BaseContextRef;


/*!
	@class	CDB4DManager
	@abstract		The root for all sub components of DB4D.
	@discussion
*/

// parameters to Open or Create a Database
const sLONG DB4D_Open_AllDefaultParamaters = 0;
const sLONG DB4D_Open_DefaultData = 1;
const sLONG DB4D_Open_WithSeparateIndexSegment = 2;
const sLONG DB4D_Open_DO_NOT_Build_Index = 4;
const sLONG DB4D_Open_DO_NOT_Match_Data_And_Struct = 8;
const sLONG DB4D_Open_WITHOUT_JournalFile = 16;
const sLONG DB4D_Open_Convert_To_Higher_Version = 32;
const sLONG DB4D_Open_Allow_Temporary_Convert_For_ReadOnly = 64;
const sLONG DB4D_Open_As_XML_Definition = 128;
const sLONG DB4D_Open_Build_Missing_IDs = 256;
const sLONG DB4D_Open_With_Own_UsersAndGroups = 512;
const sLONG DB4D_Open_No_Respart = 1024;
const sLONG DB4D_Open_AllowOneVersionMoreRecent = 2048;	// v11 opens v12 data. v12 opens v13 data.
const sLONG DB4D_Open_StructOnly = 4096;
const sLONG DB4D_Open_BuildAutoEm = 8192;
const sLONG DB4D_Open_DelayLoadIndex = 16384;

const sLONG DB4D_Create_AllDefaultParamaters = 0;
const sLONG DB4D_Create_DefaultData = 1;
const sLONG DB4D_Create_WithSeparateIndexSegment = 2;
const sLONG DB4D_Create_DO_NOT_Build_Index = 4;
const sLONG DB4D_Create_DO_NOT_Match_Data_And_Struct = 8;
const sLONG DB4D_Create_As_XML_Definition = 128;
const sLONG DB4D_Create_With_Own_UsersAndGroups = 512;
const sLONG DB4D_Create_No_Respart = 1024;
const sLONG DB4D_Create_Empty_Catalog = 2048;
//const sLONG DB4D_Create_WITHOUT_JurnalFile = 16;		// pas necessaire	


class CDB4DRawDataBase;
class CDB4DSelection;
class CDB4DRecord;
class CDB4DComplexQuery;
class CDB4DJournalParser;
class IBackupSettings;

typedef void* (*GetCurrentSelectionFromContextMethod) (void* inContext, sLONG tablenum);
typedef CDB4DRecord* (*GetCurrentRecordFromContextMethod) (void* inContext, sLONG tablenum);

#if DB4DasComponent
class CDB4DManager : public XBOX::CComponent
#else
class ENTITY_API CDB4DManager : public XBOX::IRefCountable
#endif
{
public:
	enum {Component_Type = 'dbmg'};

	virtual void __test(const XBOX::VString& command) = 0;

	virtual IBackupSettings* CreateBackupSettings() = 0;

	virtual IBackupTool* CreateBackupTool() = 0;

	virtual IJournalTool* CreateJournalTool() = 0;

	virtual XBOX::VError GetJournalInfos(const XBOX::VFilePath& inDataFilePath,XBOX::VFilePath& journalPath,XBOX::VUUID& journalDataLink) = 0;

	virtual DB4DNetworkManager* CreateRemoteTCPSession(const XBOX::VString& inServerName, sWORD inPortNum, VErrorDB4D& outerr, bool inSSL = false) = 0;

	virtual VErrorDB4D OpenRemoteCatalogs(void* projectRef, const XBOX::VFolder* folderToScan, CDB4DBase* associate, bool stopOnFirstError) = 0;

	virtual CDB4DBase* OpenRemoteBase( CDB4DBase* inLocalDB, const XBOX::VUUID& inBaseID, VErrorDB4D& outErr, DB4DNetworkManager *inLegacyNetworkManager, CUAGDirectory* inUAGDirectory = nil) = 0;

	virtual CDB4DBase* OpenRemoteBase( CDB4DBase* inLocalDB, const XBOX::VString& inBaseName, const XBOX::VString& inFullBasePath, 
										VErrorDB4D& outErr, Boolean inCreateIfNotExist, DB4DNetworkManager* inNetworkManager = nil, CUAGDirectory* inUAGDirectory = nil) = 0;

	virtual CDB4DBase* OpenRemoteBase( CDB4DBase* inLocalDB, const XBOX::VString& inBaseName, VErrorDB4D& outErr, DB4DNetworkManager* inNetworkManager = nil, CUAGDirectory* inUAGDirectory = nil) = 0;



/*
	virtual void* GetComponentLibrary() const = 0;
*/

	

	/*!
		@function	CreateBase
		@abstract	Creates a new database (structure and data are separated files).
		@discussion
			It first creates the structure file at the location given as first parameter.
			Then, if asked to do so, it creates a new datafile with a default name (usually xxx.data).
			All data related commands throw an error until a datafile is properly opened with CDB4DBaseContext::OpenData.
		@param	inStructureFile location of the structure file (must not already exist).
	*/
	virtual CDB4DBase* CreateBase( const XBOX::VFile& inStructureFile, sLONG inParameters, XBOX::VIntlMgr* inIntlMgr, VErrorDB4D* outErr = nil, 
									XBOX::FileAccess inAccess = XBOX::FA_READ_WRITE, const XBOX::VUUID* inChosenID = nil, XBOX::VString* EntityFileExt = nil,
									CUAGDirectory* inUAGDirectory = nil, const XBOX::VFile* inPermissionsFile = nil) = 0;


	/*!
		@function	OpenBase
		@abstract	Open an existing database (structure and data are separated files).
		@discussion
			It first opens the structure file at the location given as first parameter.
			Then, if asked to do so, it opens the default datafile whose location has been
			stored in the structure file.
			All data related commands throw an error until a datafile is properly opened with CDB4DBaseContext::OpenData.
		@param	inStructureFile location of the structure file (must exist).
	*/
	virtual CDB4DBase* OpenBase( const XBOX::VFile& inStructureFile, sLONG inParameters, VErrorDB4D* outErr = nil, 
		XBOX::FileAccess inAccess = XBOX::FA_READ_WRITE, XBOX::VString* EntityFileExt = nil, CUAGDirectory* inUAGDirectory = nil, const XBOX::VFile* inPermissionsFile = nil,
		void* ProjectRef = nil) = 0;


	virtual CDB4DBase* OpenBase( const XBOX::VString& inXMLContent, sLONG inParameters, XBOX::VError* outErr, const XBOX::VFilePath& inXMLPath, 
									XBOX::unordered_map_VString<XBOX::VRefPtr<XBOX::VFile> >* outIncludedFiles = nil, const XBOX::VFile* inPermissionsFile = nil) = 0;



	/*!
		@function	CloseBase
		@abstract	Close both data and structure part of a previously open database.
		@discussion
			Throws an error if the base is not open or if a base context is still referencing this database.
		@param	inBaseID the base id of the database to close.
	*/
	// virtual VErrorDB4D CloseBase( DB4D_BaseID inBaseID) = 0;


	/*!
		@function	NewBaseContext
		@abstract	Creates a new base context that allows you to use a database.
		@discussion
			See CDB4DBaseContext documentation.
		@param	inBaseID the base id of the database to get a context from.
	*/
	// virtual CDB4DBaseContext *NewBaseContext( VUUID& inBaseID) = 0;



	/*!
		@function	CountBases
		@abstract	Returns the number of currently open bases.
		@param	inBaseID the base id of the database to close.
	*/
	virtual sLONG CountBases() = 0;

	virtual CDB4DBase* RetainBaseByUUID(const XBOX::VUUID& inBaseID) = 0;
	

	/*!
		@function	RetainNthBase
		@abstract	Returns the base id of the nth open base.
	*/
	virtual CDB4DBase* RetainNthBase( sLONG inBaseIndex) = 0;
	
	/*!
		@function	FlushCache
		@abstract	Flushes all data currently stored in the memory cache to disk
		@discussion
			starts in a separate process, will ensure logic physical integrity and logical integrity of data
			(for example, will not write a record to disk without writing the table of adresses containing the record address)
		@param	WaitUntilDone if true the task calling will be suspended until cache fully written
		@result	A unique base ID on success.
	*/
	virtual void FlushCache(Boolean WaitUntilDone = false, bool inEmptyCacheMem = false, ECacheLogEntryKind inOrigin = eCLEntryKind_FlushFromUnknown) = 0;
	virtual	void SetFlushPeriod( sLONG inMillisecondsPeriod) = 0;
	
	/*!
		@function	GetCommandList
		@abstract	Returns the list of available commands the database engine can send.
		@discussion
			The list of available command is as follow:

				DB4D_AddBase
					UUID	BaseID 
										
				DB4D_CloseBase
					UUID	BaseID 

				DB4D_UpdateBaseName
					UUID	BaseID 
	
				DB4D_AddTable
					UUID	BaseID
					Long  TableID

				DB4D_DelTable
					UUID	BaseID 
					Long  TableID

				DB4D_UpdateTableName
					UUID	BaseID 
					Long  TableID

				DB4D_AddField
					UUID	BaseID 
					Long  TableID
					Long  FieldID

				DB4D_DelField
					UUID	BaseID 
					Long  TableID
					Long  FieldID
					
				DB4D_UpdateField
					UUID	BaseID 
					Long  TableID
					Long  FieldID
					Bool  NameChanged    // true if FieldName has changed
					Bool  TypeChanged		 // true if FieldType has changed

				DB4D_AddIndex
					UUID	BaseID 

				DB4D_DelIndex
					UUID	BaseID 

				DB4D_AddRelation
					UUID	BaseID 
					UUID  RelationID
					
				DB4D_DelRelation
					UUID	BaseID 
					UUID  RelationID

		@result	a VCommandList
	*/
	//virtual VCommandList* GetCommandList() = 0;

	virtual DB4DSignaler* GetSignaler() = 0;

	virtual DB4DCommandManager* SetCommandManager(DB4DCommandManager* inNewCommandManager) = 0;
	
	virtual DB4DActivityManager* SetActivityManager(DB4DActivityManager* inNewActivityManager) = 0;
	
	virtual DB4DNetworkManager* SetNetworkManager( DB4DNetworkManager* inNewNetworkManager) = 0;

	virtual BuildLanguageExpressionMethod SetMethodToBuildLanguageExpressions(BuildLanguageExpressionMethod inNewMethodToBuildLanguageExpressions) = 0;

	virtual XBOX::VSize GetMinimumCacheSize() const = 0;
	virtual VErrorDB4D SetCacheSize( XBOX::VSize inSize, bool inPhysicalMemOnly, XBOX::VSize *outActualSize) = 0;
	virtual VErrorDB4D SetMinSizeToFlush( XBOX::VSize inMinSizeToFlush) = 0;

	virtual XBOX::VSize GetCacheSize() = 0;

	virtual XBOX::VSize GetMinSizeToFlush() = 0;
	
	virtual XBOX::VCppMemMgr *GetCacheMemoryManager() = 0;

	virtual VErrorDB4D SetTempMemSize(XBOX::VSize inSize, XBOX::VSize *outActualSize) = 0;
	virtual XBOX::VSize GetTempMemSize() = 0;
	virtual XBOX::VCppMemMgr* GetTempMemManager() = 0;

	virtual CDB4DContext* NewContext(CUAGSession* inUserSession, XBOX::VJSGlobalContext* inJSContext, bool isLocalOnly = false) = 0; // a appeler sur client
	virtual CDB4DContext* RetainOrCreateContext(const XBOX::VUUID& inID, CUAGSession* inUserSession, XBOX::VJSGlobalContext* inJSContext, bool isLocalOnly = false) = 0; // a appeler sur server

	virtual void StartDebugLog() = 0;

	virtual uLONG8 GetDB4D_VersionNumber() = 0;

	//virtual UniChar GetWildChar() = 0;
	//virtual void SetWildChar(UniChar c) = 0;

	virtual	bool	ContainsKeyword( const XBOX::VString& inString, const XBOX::VString& inKeyword, bool inDiacritical, XBOX::VIntlMgr *inIntlMgr, VErrorDB4D& outErr) = 0;
	virtual	bool	ContainsKeyword_Like( const XBOX::VString& inString, const XBOX::VString& inKeyword, bool inDiacritical, XBOX::VIntlMgr *inIntlMgr, VErrorDB4D& outErr) = 0;

	virtual	bool	ContainsKeyword( const XBOX::VString& inString, const XBOX::VString& inKeyword, const XBOX::VCompareOptions& inOptions, VErrorDB4D& outErr) = 0;

	virtual VErrorDB4D Register_DB4DArrayOfValuesCreator(sLONG signature, DB4DArrayOfValuesCreator Creator) = 0;
	virtual VErrorDB4D UnRegister_DB4DArrayOfValuesCreator(sLONG signature) = 0;

	virtual VErrorDB4D Register_DB4DCollectionManagerCreator(sLONG signature, DB4DCollectionManagerCreator Creator) = 0;
	virtual VErrorDB4D UnRegister_DB4DCollectionManagerCreator(sLONG signature) = 0;

	virtual void SetDefaultProgressIndicator_For_Indexes(VDB4DProgressIndicator* inProgress) = 0;
	virtual VDB4DProgressIndicator* RetainDefaultProgressIndicator_For_Indexes() = 0;
	virtual void SetDefaultProgressIndicator_For_DataCacheFlushing(VDB4DProgressIndicator* inProgress) = 0;
	virtual VDB4DProgressIndicator* RetainDefaultProgressIndicator_For_DataCacheFlushing() = 0;

	virtual bool CheckCacheMem() = 0;

	virtual const XBOX::VValueBag* RetainExtraProperties( XBOX::VFile* inFile, bool *outWasOpened, XBOX::VUUID *outUUID, VErrorDB4D &err, CDB4DBaseContextPtr inContext = NULL, uLONG8* outVersionNumber = NULL) = 0;

	virtual CDB4DRawDataBase* OpenRawDataBase(XBOX::VFile* inStructFile, XBOX::VFile* inDataFile, IDB4D_DataToolsIntf* inDataToolLog, VErrorDB4D& outError, XBOX::FileAccess access = XBOX::FA_MAX) = 0;
	virtual CDB4DRawDataBase* OpenRawDataBaseWithEm(CDB4DBase* inStructureDB, XBOX::VFile* inDataFile, IDB4D_DataToolsIntf* inDataToolLog, VErrorDB4D& outError, XBOX::FileAccess access = XBOX::FA_MAX) = 0;

	virtual VErrorDB4D ReleaseRawRecord(void* inRawRecord) = 0;
	virtual void* GetRawRecordNthField(void* inRawRecord, sLONG inFieldIndex, sLONG& outType, VErrorDB4D* outError = nil, bool forQueryOrSort = true) = 0; // result can be nil if field value is NULL
	virtual RecIDType GetRawRecordID(void* inRawRecord, VErrorDB4D* outError = nil) = 0;

	virtual void Register_GetCurrentSelectionFromContextMethod(GetCurrentSelectionFromContextMethod inMethod) = 0;
	virtual void Register_GetCurrentRecordFromContextMethod(GetCurrentRecordFromContextMethod inMethod) = 0;

	virtual	void	ExecuteRequest( sWORD inRequestID, XBOX::IStreamRequestReply *inRequest, CDB4DBaseContext *inContext) = 0;

	virtual VErrorDB4D StartServer(const XBOX::VString& inServerName, sWORD inPortNum, bool inSSL = false) = 0;
	virtual VErrorDB4D StopServer(sLONG inTimeOut) = 0;
	virtual void CloseAllClientConnections(sLONG inTimeOut) = 0;

	virtual void CloseConnectionWithClient(CDB4DContext* inContext) = 0; // a appeler sur le server et maintenant fonctionne aussi sur le client

	virtual CDB4DJournalParser* NewJournalParser() = 0;

	virtual	VErrorDB4D	BuildValid4DTableName( const XBOX::VString& inName, XBOX::VString& outValidName) const = 0;
	virtual	VErrorDB4D	BuildValid4DFieldName( const XBOX::VString& inName, XBOX::VString& outValidName) const = 0;
		
	// if inBase is NULL, none check on table name unicity is done
	virtual	VErrorDB4D	IsValidTableName( const XBOX::VString& inName, EValidateNameOptions inOptions, CDB4DBase *inBase = NULL, CDB4DTable *inTable = NULL) const = 0;
	// if inBase is NULL, none check on link name unicity is done
	virtual	VErrorDB4D	IsValidLinkName( const XBOX::VString& inName, EValidateNameOptions inOptions, bool inIsNameNto1 = true, CDB4DBase *inBase = NULL, CDB4DRelation *inRelation = NULL) const = 0;
	// if inTable is NULL, none check on field name unicity is done
	virtual	VErrorDB4D	IsValidFieldName( const XBOX::VString& inName, EValidateNameOptions inOptions, CDB4DTable *inTable = NULL, CDB4DField *inField = NULL) const = 0;


	virtual void StealListOfReleasedSelIDs(std::vector<XBOX::VUUIDBuffer>& outList) = 0; // to be called on client
	virtual void ForgetServerKeptSelections(const std::vector<XBOX::VUUIDBuffer>& inList) = 0; // to be called on server

	virtual void StealListOfReleasedRecIDs(std::vector<DB4D_RecordRef>& outList) = 0; // to be called on client
	virtual void ForgetServerKeptRecords(const std::vector<DB4D_RecordRef>& inList) = 0; // to be called on server

	virtual void SetRequestLogger(IRequestLogger* inLogger) = 0;

	// Attach the base context owner (the db4d context) to the JavaScript context and append the "db" and "ds" properties to the global object (4D language context need)
	virtual	void				InitializeJSGlobalObject( XBOX::VJSGlobalContext *inJSContext, CDB4DBaseContext *inBaseContext) = 0;
	
	// Attach the DB4D context to the JavaScript context
	virtual	void				InitializeJSContext( XBOX::VJSGlobalContext *inContext, CDB4DContext *inDB4DContext) = 0;
	// Remove private attachments according to the attached DB4D context and detach the DB4D context
	virtual	void				UninitializeJSContext( XBOX::VJSGlobalContext *inContext) = 0;
#if 0
	// Remove private attachments for a specific base context
	virtual	void				UninitializeJSContext( XBOX::VJSGlobalContext *inContext, CDB4DBaseContext *inBaseContext) = 0;
#endif

	virtual	CDB4DContext*		GetDB4DContextFromJSContext( const XBOX::VJSContext& inContext) = 0;
	virtual	CDB4DBaseContext*	GetDB4DBaseContextFromJSContext( const XBOX::VJSContext& inContext, CDB4DBase *inBase) = 0;

	virtual	XBOX::VJSObject		CreateJSDatabaseObject( const XBOX::VJSContext& inContext, CDB4DBaseContext *inBaseContext) = 0;
	virtual	XBOX::VJSObject		CreateJSEMObject( const XBOX::VString& emName, const XBOX::VJSContext& inContext, CDB4DBaseContext *inBaseContext) = 0;
	virtual XBOX::VJSObject		CreateJSBackupSettings(const XBOX::VJSContext& inContext,IBackupSettings* retainedBackupSettings) = 0;
	//virtual	void				PutAllEmsInGlobalObject(XBOX::VJSObject& globalObject, CDB4DBaseContext *inBaseContext) = 0;
	virtual	void				SetAllDB4DGlobalProperties(XBOX::VJSObject& globalObject, CDB4DBaseContext *inBaseContext, void* ProjectRef) = 0;
	virtual	void				ReleaseProjectInfo(void* ProjectRef) = 0;

	virtual void SetLimitPerSort(XBOX::VSize inLimit) = 0;
	virtual XBOX::VSize GetLimitPerSort() const = 0;

	virtual void StartRecordingMemoryLeaks() = 0;

	virtual void StopRecordingMemoryLeaks() = 0;

	virtual void DumpMemoryLeaks(XBOX::VString& outText) = 0;

	virtual void PutThingsToForget( XBOX::VStream* into, CDB4DBaseContext* context) = 0;

	virtual VErrorDB4D GetThingsToForget( XBOX::VStream* clientreq, CDB4DBaseContext* context) = 0;

	virtual VErrorDB4D RebuildMissingStructure(const XBOX::VFile& inStructureToRebuild, const XBOX::VFile& inExistingDataFile) = 0;

	/**	@name Cache-flush log */
	//@{
	/**	@discussion
			See VCacheLog for details about how this log works (thread safe, atomic operations, etc.),
			and for details about the parameters, ie the inParams parameter of StartCacheLog(), the
			returned value of IsCacheLogStarted(), ...
	*/
	virtual bool StartCacheLog(const XBOX::VFolder& inRoot, const XBOX::VString& inFileName, const XBOX::VValueBag *inParams = NULL) = 0;
	virtual void StopCacheLog() = 0;
	virtual bool IsCacheLogStarted() const = 0;
	virtual void AppendCommentToCacheLog(const XBOX::VString& inWhat) = 0;
	//@}

	virtual void SetRunningMode( DB4D_Running_Mode inRunningMode) = 0;

	virtual	void					SetApplicationInterface( IDB4D_ApplicationIntf *inApplication) = 0;
	virtual	IDB4D_ApplicationIntf*	GetApplicationInterface() const = 0;
	
	virtual	void					SetGraphicsInterface( IDB4D_GraphicsIntf *inGraphics) = 0;
	virtual	IDB4D_GraphicsIntf*		GetGraphicsInterface() const = 0;

	virtual	void					SetSQLInterface (IDB4D_SQLIntf *inSQLIntf) = 0;
	virtual	IDB4D_SQLIntf			*GetSQLInterface () const = 0;

	virtual void GetStaticRequiredJSFiles(std::vector<XBOX::VFilePath>& outFiles) = 0;

	virtual IDB4D_DataToolsIntf* CreateJSDataToolsIntf(XBOX::VJSContext& jscontext, XBOX::VJSObject& paramObj) = 0;

	virtual XBOX::VError EraseDataStore(XBOX::VFile* dataDS) = 0;
	
	/*		
		Call AddRestRequestHandler() to initialize the REST requests handler for the HTTP Server project. inBase can be NULL.
	*/
	virtual IHTTPRequestHandler*	AddRestRequestHandler( VErrorDB4D& outError, CDB4DBase *inBase, IHTTPServerProject *inHTTPServerProject, RIApplicationRef inApplicationRef, const XBOX::VString& inPattern, bool inEnabled) = 0;

#if DB4DasComponent
	static CDB4DManager* RetainManager(XBOX::VLocalizationManager* inLocalizationManager = NULL)
	{
		return XBOX::VComponentManager::RetainComponentOfType<CDB4DManager>();
	}
#endif

#if DB4DasDLL
	static CDB4DManager* RetainManager(XBOX::VLocalizationManager* inLocalizationManager = NULL);

private:

	static CDB4DManager* sManager;
#endif
};


class CDB4DComplexSelection;
class CDB4DSet;
class CDB4DSchema;
class CDB4DEntityModel;

typedef sLONG DB4D_SchemaID;

class DB4D_API ExportOption
{
	public:
	
		// Binary data (blob, text, or picture) smaller than or equal to BlobThresholdSize bytes are exported
		// in SQL scripts (instead of small files). If zero, binary data are always exported in separated files, 
		// except if they have zero size.
	
		static const sLONG kDefaultBlobThresholdSize = 0;

		inline ExportOption()
		{
			NbBlobsPerLevel = 200;
			BlobThresholdSize = kDefaultBlobThresholdSize;
			MaxSQLTextSize = 0;
			BinaryExport = false;
			CreateFolder = true;
			JSONExport = false;
			Import = false;
			ChangeIntegrityRules = true;
			DelayIndexes = true;
		}

		sLONG NbBlobsPerLevel;
	
		sLONG BlobThresholdSize;
	
		sLONG8 MaxSQLTextSize;
		bool BinaryExport, CreateFolder, JSONExport, Import, ChangeIntegrityRules, DelayIndexes;
};

class DB4DSQLExpression;

class CDB4DBase/* : public XBOX::CComponent*/ : public XBOX::IRefCountable
{
public:
	enum {Component_Type = 'dbba'};

	virtual Boolean MatchBase(CDB4DBase* InBaseToCompare) const = 0; 
	virtual CDB4DBaseContextPtr NewContext(CUAGSession* inUserSession, XBOX::VJSGlobalContext* inJSContext, bool isLocalOnly = false) = 0; 
	virtual void GetName( XBOX::VString& outName, CDB4DBaseContextPtr inContext = NULL) const = 0;
	virtual void GetUUID(XBOX::VUUID& outID, CDB4DBaseContextPtr inContext = NULL) const = 0;
	virtual const XBOX::VUUID& GetUUID(CDB4DBaseContextPtr inContext = NULL) const = 0;
	virtual void GetBasePath( XBOX::VFilePath& outPath, CDB4DBaseContextPtr inContext = NULL) const = 0; // this may be the folder for temp files in c/s or the first data segment path
	virtual XBOX::VValueBag *CreateDefinition( bool inWithTables, bool inWithIndices, bool inWithRelations, CDB4DBaseContextPtr inContext = NULL) const = 0;

	virtual const IBackupSettings* RetainBackupSettings()= 0;
	virtual void  SetRetainedBackupSettings(IBackupSettings* inSettings) = 0;

	virtual Boolean IsDataOpened(CDB4DBaseContextPtr inContext = NULL) const = 0;
	virtual Boolean OpenDefaultData( Boolean inCreateItIfNotFound, sLONG inParameters, CDB4DBaseContextPtr inContext = NULL, 
										VErrorDB4D* outErr = nil, XBOX::FileAccess inAccess = XBOX::FA_READ_WRITE) const = 0;

	virtual Boolean CreateData( const XBOX::VFile& inDataFile, sLONG inParameters, XBOX::VFile *inJournalingFile = NULL, 
								CDB4DBaseContextPtr inContext = NULL, VErrorDB4D* outErr = nil, XBOX::FileAccess inAccess = XBOX::FA_READ_WRITE ) const = 0;

	virtual Boolean OpenData( const XBOX::VFile& inDataFile, sLONG inParameters, CDB4DBaseContextPtr inContext = NULL, 
								VErrorDB4D* outErr = nil, XBOX::FileAccess inAccess = XBOX::FA_READ_WRITE) const = 0;

	virtual XBOX::VError LoadIndexesAfterCompacting(sLONG inParameters) = 0;
	virtual XBOX::VFile* RetainIndexFile() const = 0;

	virtual sLONG CountTables(CDB4DBaseContextPtr inContext = NULL) const = 0;
	virtual VErrorDB4D CreateTable( const XBOX::VValueBag& inTableDefinition, CDB4DTable **outTable, CDB4DBaseContextPtr inContext = NULL, Boolean inGenerateName = false) = 0;
	virtual CDB4DTable* RetainNthTable(sLONG inIndex, CDB4DBaseContextPtr inContext = NULL) const = 0;
	virtual CDB4DTable* FindAndRetainTable(const XBOX::VString& inName, CDB4DBaseContextPtr inContext = NULL) const = 0;
	virtual CDB4DTable* FindAndRetainTable(const XBOX::VUUID& inID, CDB4DBaseContextPtr inContext = NULL) const = 0;
	virtual CDB4DTable* FindAndRetainTableInStruct(const XBOX::VUUID& inID, CDB4DBaseContextPtr inContext = NULL) const = 0;
	virtual Boolean DropTable(DB4D_TableID inTableID, VDB4DProgressIndicator* InProgress = NULL, CDB4DBaseContextPtr inContext = NULL) = 0;

	virtual CDB4DTable* CreateOutsideTable( const XBOX::VString& inTableName, CDB4DBaseContextPtr inContext = NULL, VErrorDB4D* outErr = NULL) const = 0;
	virtual VErrorDB4D AddTable( CDB4DTable *inTable, CDB4DBaseContextPtr inContext = NULL) = 0;
	virtual VErrorDB4D GetNameForNewTable( XBOX::VString& outName) const = 0;

	virtual CDB4DField* FindAndRetainField(const XBOX::VUUID& inID, CDB4DBaseContextPtr inContext = NULL) const = 0;
	virtual CDB4DField* FindAndRetainField(const XBOX::VString& inTableName, const XBOX::VString& inFieldName, CDB4DBaseContextPtr inContext = NULL) const = 0;
	virtual CDB4DField* FindAndRetainField(const XBOX::VString& inFieldName, CDB4DBaseContextPtr inContext = NULL) const = 0; // form : "Table.Field"

/*
	virtual sLONG CountIndices() const = 0;
	virtual CDB4DIndex* RetainNthIndex(sLONG inIndexNum) const = 0;
	virtual XBOX::VValueBag* CreateIndexDefinition(sLONG inIndexNum) const = 0;
*/

	virtual VErrorDB4D CreateIndexOnOneField(CDB4DField* target, sLONG IndexTyp, Boolean UniqueKeys = false, 
											VDB4DProgressIndicator* InProgress = NULL, const XBOX::VString *inIndexName = NULL,
											CDB4DIndex** outResult = NULL, Boolean ForceCreate = true, XBOX::VSyncEvent* event = NULL,
											CDB4DBaseContextPtr inContext = NULL) = 0;

	virtual VErrorDB4D CreateIndexOnMultipleField(const XBOX::VValueBag& IndexDefinition, Boolean UniqueKeys = false, 
												VDB4DProgressIndicator* InProgress = NULL, CDB4DIndex** outResult = NULL, 
												Boolean ForceCreate = true, XBOX::VSyncEvent* event = NULL,
												CDB4DBaseContextPtr inContext = NULL) =0 ;

	virtual VErrorDB4D CreateIndexOnMultipleField(const CDB4DFieldArray& inTargets, sLONG IndexTyp, Boolean UniqueKeys = false, 
												VDB4DProgressIndicator* InProgress = NULL, const XBOX::VString *inIndexName = NULL,
												CDB4DIndex** outResult = NULL, Boolean ForceCreate = true, XBOX::VSyncEvent* event = NULL,
												CDB4DBaseContextPtr inContext = NULL) =0 ;

	virtual VErrorDB4D DropIndexOnOneField(CDB4DField* target, sLONG IndexTyp = 0, VDB4DProgressIndicator* InProgress = NULL, 
											XBOX::VSyncEvent* event = NULL, CDB4DBaseContextPtr inContext = NULL) = 0;

	virtual VErrorDB4D DropIndexOnMultipleField(const XBOX::VValueBag& IndexDefinition, VDB4DProgressIndicator* InProgress = NULL, 
												XBOX::VSyncEvent* event = NULL, CDB4DBaseContextPtr inContext = NULL) = 0;

	virtual VErrorDB4D DropIndexOnMultipleField(const CDB4DFieldArray& inTargets, sLONG IndexTyp = 0, VDB4DProgressIndicator* InProgress = NULL, 
												XBOX::VSyncEvent* event = NULL, CDB4DBaseContextPtr inContext = NULL) = 0;

	virtual VErrorDB4D DropIndexByName(const XBOX::VString& inIndexName, VDB4DProgressIndicator* InProgress = NULL, XBOX::VSyncEvent* event = NULL,
										CDB4DBaseContextPtr inContext = NULL) = 0;

	virtual VErrorDB4D CreateFullTextIndexOnOneField(CDB4DField* target, VDB4DProgressIndicator* InProgress = NULL, 
													const XBOX::VString *inIndexName = nil, CDB4DIndex** outResult = NULL, 
													Boolean ForceCreate = true, XBOX::VSyncEvent* event = NULL,
													CDB4DBaseContextPtr inContext = NULL) = 0;

	virtual VErrorDB4D DropFullTextIndexOnOneField(CDB4DField* target, VDB4DProgressIndicator* InProgress = NULL, XBOX::VSyncEvent* event = NULL,
													CDB4DBaseContextPtr inContext = NULL) = 0;

	virtual CDB4DIndex* FindAndRetainIndexByName(const XBOX::VString& inIndexName, CDB4DBaseContextPtr inContext = NULL, Boolean MustBeValid = true, Boolean Occupy = false) const = 0;
	virtual CDB4DIndex* FindAndRetainIndexByUUID( const XBOX::VUUID& inUUID, CDB4DBaseContextPtr inContext = NULL, Boolean MustBeValid = true, Boolean Occupy = false) const = 0;

	virtual CDB4DIndex* RetainIndex(sLONG inNumTable, sLONG inNumField, Boolean MustBeSortable = false, CDB4DBaseContextPtr inContext = NULL, Boolean MustBeValid = true, Boolean Occupy = false) const = 0;
	virtual CDB4DIndex* RetainFullTextIndex(sLONG inNumTable, sLONG inNumField, CDB4DBaseContextPtr inContext = NULL, Boolean MustBeValid = true, Boolean Occupy = false) const = 0;
	virtual CDB4DIndex* RetainCompositeIndex(const CDB4DFieldArray& inTargets, Boolean MustBeSortable = false, CDB4DBaseContextPtr inContext = NULL, Boolean MustBeValid = true, Boolean Occupy = false) const = 0;

	/* IndexDefinition content : 'fields' array of 
	'FieldName' : string
	'TableName' : string
	ou bien
	'FieldID' 	: long
	'TableID' 	: long
	end of array

	'IndexType' : long ( default = BTree )
	*/


	virtual VErrorDB4D CreateRelation( const XBOX::VValueBag& inBag, CDB4DRelation **outRelation, CDB4DBaseContextPtr inContext = NULL) = 0;

	virtual CDB4DRelation *CreateRelation(const XBOX::VString &inRelationName, const XBOX::VString &inCounterRelationName,
										CDB4DField* inSourceField, CDB4DField* inDestinationField, CDB4DBaseContextPtr inContext = NULL) = 0;

	virtual CDB4DRelation *CreateRelation(const XBOX::VString &inRelationName, const XBOX::VString &inCounterRelationName,
										const CDB4DFieldArray& inSourceFields, const CDB4DFieldArray& inDestinationFields, 
										CDB4DBaseContextPtr inContext = NULL) = 0;

	virtual CDB4DRelation* FindAndRetainRelationByName(const XBOX::VString& Name, CDB4DBaseContextPtr inContext = NULL) const = 0;

	virtual CDB4DRelation* FindAndRetainRelation(CDB4DField* inSourceField, CDB4DField* inDestinationField,
												CDB4DBaseContextPtr inContext = NULL) const = 0;

	virtual CDB4DRelation* FindAndRetainRelation(const CDB4DFieldArray& inSourceFields, const CDB4DFieldArray& inDestinationFields, 
												CDB4DBaseContextPtr inContext = NULL) const = 0;

	virtual CDB4DRelation* FindAndRetainRelationByUUID(const XBOX::VUUID& inID, CDB4DBaseContextPtr inContext = NULL) const = 0;

	virtual VErrorDB4D GetAndRetainRelations(RelationArray &outRelations, CDB4DBaseContextPtr inContext = NULL) const = 0;

	virtual VErrorDB4D IsFieldsKindValidForRelation( CDB4DField* inSourceField, CDB4DField* inDestinationField) const = 0;

	virtual	uBOOL	Lock( CDB4DBaseContextPtr inContext, Boolean WaitForEndOfRecordLocks = false, sLONG TimeToWaitForEndOfRecordLocks = 0 ) = 0;
	virtual	void	UnLock( CDB4DBaseContextPtr inContext ) = 0;
	virtual uBOOL	FlushAndLock( CDB4DBaseContextPtr inContext, Boolean WaitForEndOfRecordLocks = false, sLONG TimeToWaitForEndOfRecordLocks = 0 ) = 0;


	// import known elements from bag (tables, relations, indexes)
	virtual Boolean CreateElements( const XBOX::VValueBag& inBag, XBOX::VBagLoader *inLoader, CDB4DBaseContextPtr inContext = NULL) = 0;

	virtual CDB4DCheckAndRepairAgent *NewCheckAndRepairAgent() = 0;


	virtual void RegisterForLang(void) = 0;
	virtual void UnRegisterForLang(void) = 0;
//	virtual CNameSpace* GetNameSpace() const = 0;


	virtual void TouchXML() = 0;
	virtual void Close(XBOX::VSyncEvent* waitForClose = NULL) = 0;
	virtual void CloseAndRelease(Boolean WaitUntilFullyClosed = true) = 0;

	virtual CDB4DBase* RetainStructDatabase(const char* DebugInfo) const = 0; 

	virtual CDB4DAutoSeqNumber* CreateAutoSeqNumber(DB4D_AutoSeq_Type inType, VErrorDB4D& err, CDB4DBaseContextPtr inContext = NULL) = 0;
	virtual VErrorDB4D DropAutoSeqNumber(const XBOX::VUUID& inIDtoDelete, CDB4DBaseContextPtr inContext = NULL) = 0;
	virtual CDB4DAutoSeqNumber* RetainAutoSeqNumber(const XBOX::VUUID& inIDtoFind, CDB4DBaseContextPtr inContext = NULL) = 0;

	virtual VErrorDB4D SetJournalFile( XBOX::VFile* inNewLog, const XBOX::VUUID* inDataLink = NULL, CDB4DBaseContextPtr inContext = NULL, bool inResetJournalSequenceNumber = false) = 0;
	virtual VErrorDB4D SetJournalFileInfos( XBOX::VString *inFilePath, XBOX::VUUID *inUUID ) = 0;
	virtual	VErrorDB4D ResetJournalFileContent() = 0;
	virtual XBOX::VFile* RetainJournalFile(CDB4DBaseContextPtr inContext = NULL) = 0;
	virtual bool GetJournalUUIDLink( XBOX::VUUID &outLink ) = 0;
	virtual void GetJournalInfos( const XBOX::VFilePath &inDataFilePath, XBOX::VFilePath &outFilePath, XBOX::VUUID &outDataLink) = 0;
	virtual VErrorDB4D CreateJournal( XBOX::VFile *inFile, XBOX::VUUID *inDataLink, bool inWriteOpenDataOperation = true ) = 0;
	virtual VErrorDB4D OpenJournal( XBOX::VFile *inFile, XBOX::VUUID &inDataLink, bool inWriteOpenDataOperation = true ) = 0;
	

	virtual const XBOX::VValueBag* RetainExtraProperties(VErrorDB4D &err, CDB4DBaseContextPtr inContext = NULL) = 0;
	virtual VErrorDB4D SetExtraProperties(XBOX::VValueBag* inExtraProperties, CDB4DBaseContextPtr inContext = NULL) = 0;

	virtual const XBOX::VValueBag* RetainStructureExtraProperties(VErrorDB4D &err, CDB4DBaseContextPtr inContext = NULL) = 0;
	virtual VErrorDB4D SetStructureExtraProperties(XBOX::VValueBag* inExtraProperties, CDB4DBaseContextPtr inContext = NULL) = 0;

	virtual VErrorDB4D DelayAllIndexes() = 0;
	virtual VErrorDB4D AwakeAllIndexes(std::vector<CDB4DIndex*>& outIndexWithProblems, VDB4DProgressIndicator* inProgress = NULL, Boolean WaitForCompletion = true) = 0;

	virtual DB4DTriggerManager* SetTriggerManager(DB4DTriggerManager* inNewTriggerManager) = 0;
	virtual DB4DTriggerManager* GetTriggerManager() = 0;
	
	virtual VErrorDB4D CountBaseFreeBits( BaseStatistics &outStatistics ) const = 0;

	virtual void StartDataConversion() = 0;
	virtual void StopDataConversion() = 0;

	virtual void SetReferentialIntegrityEnabled(Boolean state) = 0;
	virtual void SetCheckUniquenessEnabled(Boolean state) = 0;
	virtual void SetCheckNot_NullEnabled(Boolean state) = 0;

	virtual CDB4DRawDataBase* OpenRawDataBase(IDB4D_DataToolsIntf* inDataToolLog, VErrorDB4D& outError) = 0;

	virtual sLONG GetStamp(CDB4DBaseContextPtr inContext = NULL) const = 0;
	virtual sLONG GetExtraPropertiesStamp(CDB4DBaseContextPtr inContext = NULL) const = 0;

	virtual void GetTablesStamps(StampsVector& outStamps, CDB4DBaseContextPtr inContext = NULL) const = 0;
	virtual void GetRelationsStamps(StampsVector& outStamps, CDB4DBaseContextPtr inContext = NULL) const = 0;

	virtual VErrorDB4D RetainIndexes(ArrayOf_CDB4DIndex& outIndexes, CDB4DBaseContextPtr inContext = NULL) = 0;

	virtual CDB4DComplexQuery* NewComplexQuery() = 0;

	virtual CDB4DComplexSelection* ExecuteQuery( CDB4DComplexQuery *inQuery, CDB4DBaseContextPtr inContext, CDB4DComplexSelection* Filter = NULL, 
												VDB4DProgressIndicator* InProgress = NULL, DB4D_Way_of_Locking HowToLock = DB4D_Do_Not_Lock, 
												sLONG limit = 0, VErrorDB4D *outErr = NULL) = 0;
	
	virtual Boolean IsRemote() const = 0;
	virtual VErrorDB4D CheckForUpdate() = 0;

	virtual VErrorDB4D IntegrateJournal(CDB4DJournalParser* inJournal, uLONG8 from = 0, uLONG8 upto = 0, uLONG8 *outCountIntegratedOperations = NULL, VDB4DProgressIndicator* InProgress = NULL) = 0;

	virtual VErrorDB4D GetErrorOnJournalFile() const = 0;

	virtual sLONG8 GetCurrentLogOperation() const = 0;
	virtual sLONG8 GetCurrentLogOperationOnDisk() const = 0;

	virtual Boolean IsWriteProtected() const = 0;

	virtual XBOX::VFileDesc* GetDataIndexSeg() const = 0;
	virtual XBOX::VFileDesc* GetStructIndexSeg() const = 0;

	virtual VErrorDB4D GetDataSegs(std::vector<XBOX::VFileDesc*>& outSegs, Boolean inWithSpecialSegs = true) const = 0;
	virtual VErrorDB4D GetStructSegs(std::vector<XBOX::VFileDesc*>& outSegs, Boolean inWithSpecialSegs = true) const = 0;

	// returns .ExternalData folder. May not exist.
	virtual	XBOX::VFolder* RetainBlobsFolder() const = 0;
	
	/*
		Returns the temporary folder currently in use.
	*/
	virtual XBOX::VFolder* RetainTemporaryFolder( bool inMustExists) const = 0;

	/*
		Set the temporary folder policy.
		inCustomFolder is used only in the case inSelector == DB4D_CustomFolder.
		Changes take effect at next database relaunch and are stored in structure extra properties.
	*/
	virtual VErrorDB4D SetTemporaryFolderSelector( DB4DFolderSelector inSelector, const XBOX::VString *inCustomFolder = NULL) = 0;
	
	/*
		Reads temporary folder policy from structure extra properties.
		outCustomFolderPath is what has been read in the bag and can be invalid.
	*/
	virtual VErrorDB4D GetTemporaryFolderSelector( DB4DFolderSelector *outSelector, XBOX::VString& outCustomFolderPath) const = 0;

	virtual VErrorDB4D PrepareForBackup(CDB4DBaseContextPtr inContext) = 0;
	virtual VErrorDB4D EndBackup(CDB4DBaseContextPtr inContext, XBOX::VFile* oldJournal) = 0;

	virtual XBOX::VIntlMgr* GetIntlMgr() const = 0;

	virtual VErrorDB4D SetIntlMgr(XBOX::VIntlMgr* inIntlMgr, VDB4DProgressIndicator* InProgress = NULL) = 0;


	virtual CDB4DQueryResult* RelateOneSelection(sLONG TargetOneTable, VErrorDB4D& err, CDB4DBaseContextPtr inContext, CDB4DQueryOptions* inOptions,
													VDB4DProgressIndicator* InProgress = nil, std::vector<CDB4DRelation*> *inPath = nil) = 0;

	virtual CDB4DQueryResult* RelateManySelection(CDB4DField* inRelationStart, VErrorDB4D& err, CDB4DBaseContextPtr inContext, 
														CDB4DQueryOptions* inOptions, VDB4DProgressIndicator* InProgress = nil) = 0;

	// client server

	virtual CDB4DRecord* BuildRecordFromServer(XBOX::VStream* from, CDB4DBaseContext* inContext, CDB4DTable* inTable, VErrorDB4D& outError) = 0;
	virtual CDB4DRecord* BuildRecordFromClient(XBOX::VStream* from, CDB4DBaseContext* inContext, CDB4DTable* inTable, VErrorDB4D& outError) = 0;

	virtual CDB4DSelection* BuildSelectionFromServer(XBOX::VStream* from, CDB4DBaseContext* inContext, CDB4DTable* inTable, VErrorDB4D& outError) = 0;
	virtual CDB4DSelection* BuildSelectionFromClient(XBOX::VStream* from, CDB4DBaseContext* inContext, CDB4DTable* inTable, VErrorDB4D& outError) = 0;

	virtual CDB4DSet* BuildSetFromServer(XBOX::VStream* from, CDB4DBaseContext* inContext, CDB4DTable* inTable, VErrorDB4D& outError) = 0;
	virtual CDB4DSet* BuildSetFromClient(XBOX::VStream* from, CDB4DBaseContext* inContext, CDB4DTable* inTable, VErrorDB4D& outError) = 0;

	virtual CDB4DQueryOptions* NewQueryOptions() const = 0;

	virtual void Flush(Boolean WaitUntilDone = false, bool inEmptyCacheMem = false, ECacheLogEntryKind inOrigin = eCLEntryKind_FlushFromUnknown) = 0;
	// pour flusher la structure faire le flush sur la structure que l'on peut obtenir avec RetainStructDatabase

	virtual void SyncThingsToForget(CDB4DBaseContext* inContext) = 0;

	virtual Boolean StillIndexing() const = 0;

	virtual DB4D_SchemaID CreateSchema(const XBOX::VString& inSchemaName, CDB4DBaseContext* inContext, VErrorDB4D& outError) = 0;

	virtual sLONG CountSchemas() const = 0;

	virtual VErrorDB4D RetainAllSchemas(std::vector<XBOX::VRefPtr<CDB4DSchema> >& outSchemas, CDB4DBaseContext* inContext) = 0;

	virtual CDB4DSchema* RetainSchema(const XBOX::VString& inSchemaName, CDB4DBaseContext* inContext) = 0;

	virtual CDB4DSchema* RetainSchema(DB4D_SchemaID inSchemaID, CDB4DBaseContext* inContext) = 0;

	virtual void SetClientID(const XBOX::VUUID& inID) = 0;

	virtual VErrorDB4D ExportToSQL(CDB4DBaseContext* inContext, XBOX::VFolder* inFolder, VDB4DProgressIndicator* inProgress, ExportOption& options) = 0;

	virtual VErrorDB4D ImportRecords(CDB4DBaseContext* inContext, XBOX::VFolder* inFolder, VDB4DProgressIndicator* inProgress, ExportOption& options) = 0;

	virtual VErrorDB4D ExecuteRestRequest(tempDB4DWebRequest& inRequest) = 0; // temporaire L.R le 4 dec 2008
	
	//virtual sLONG CountEntityModels(CDB4DBaseContext* context, bool onlyRealOnes = true) const = 0;
	
	virtual VErrorDB4D RetainAllEntityModels(std::vector<XBOX::VRefPtr<CDB4DEntityModel> >& outList, CDB4DBaseContext* context, bool onlyRealOnes = true) const = 0;
	
	virtual CDB4DEntityModel* RetainEntityModel(const XBOX::VString& entityName, bool onlyRealOnes = true) const = 0;

	virtual CDB4DEntityModel* RetainEntityModelByCollectionName(const XBOX::VString& entityName) const = 0;

	virtual VErrorDB4D SetIdleTimeOut(uLONG inMilliseconds) = 0;

	virtual void DisposeRecoverInfo(void* inRecoverInfo) = 0;

	virtual VErrorDB4D ScanToRecover(XBOX::VFile* inOldDataFile, XBOX::VValueBag& outBag, void* &outRecoverInfo, IDB4D_DataToolsIntf* inDataToolLog = NULL) = 0;

	virtual VErrorDB4D RecoverByTags(XBOX::VValueBag& inBag, void* inRecoverInfo, IDB4D_DataToolsIntf* inDataToolLog = NULL) = 0;

	virtual CDB4DBase* RetainSyncDataBase() = 0;

	virtual XBOX::VError SaveToBag(XBOX::VValueBag& outBag) = 0;

	/*		
		Call AddRestRequestHandler() to initialize the REST data service. If inPattern string is empty, the default pattern is used.
	*/
	virtual IHTTPRequestHandler *	AddRestRequestHandler( VErrorDB4D& outError, IHTTPServerProject *inHTTPServerProject, RIApplicationRef inApplicationRef, const XBOX::VString& inPattern, bool inEnabled) = 0;
	virtual VErrorDB4D				SetRestRequestHandlerPattern( IHTTPRequestHandler* inHandler, const XBOX::VString& inPattern) = 0;

	virtual VErrorDB4D ReLoadEntityModels(XBOX::VFile* inFile) = 0;

	virtual VErrorDB4D GetListOfDeadTables(std::vector<XBOX::VString>& outTableNames, std::vector<XBOX::VUUID>& outTableIDs, CDB4DBaseContext* inContext) = 0;
	virtual VErrorDB4D ResurectDataTable(const XBOX::VString& inTableName, CDB4DBaseContext* inContext) = 0;

	virtual XBOX::VFolder* RetainDataFolder() = 0;
	virtual XBOX::VFolder* RetainStructFolder() = 0;
	virtual VErrorDB4D CompactInto(XBOX::VFile* destData, IDB4D_DataToolsIntf* inDataToolLog, bool KeepRecordNumbers) = 0;

	virtual	XBOX::VJSObject CreateJSDatabaseObject( const XBOX::VJSContext& inContext) = 0;

	virtual bool CatalogJSParsingError(XBOX::VFile* &outRetainedFile, XBOX::VString& outMessage, sLONG& outLineNumber) = 0;

	virtual void InvalidateAutoCatalog() = 0;
	/** @brief	whenever a 4D Method is added or one of its properties is modified, InvalidateAutoCatalog should be called by 4D 
	*/
};


class CDB4DSchema : public XBOX::IRefCountable
{
	public:
		enum {Component_Type = 'dbsh'};

		virtual const XBOX::VString& GetName() const = 0;
		virtual DB4D_SchemaID GetID() const = 0;

		virtual XBOX::VValueBag* RetainAndLockProperties(VErrorDB4D& outError, CDB4DBaseContext* inContext) = 0;
		virtual void SaveProperties(CDB4DBaseContext* inContext) = 0;
		virtual void UnlockProperties(CDB4DBaseContext* inContext) = 0;

		virtual VErrorDB4D Drop(CDB4DBaseContext* inContext) = 0;
		virtual VErrorDB4D Rename(const XBOX::VString& inNewName, CDB4DBaseContext* inContext) = 0;
};


typedef enum { DB4D_Rel_AutoLoad = 1,
							 DB4D_Rel_Not_AutoLoad = 2,
							 DB4D_Rel_AutoLoad_SameAsStructure = 3} DB4D_Rel_AutoLoadState;

typedef enum { DB4D_QueryDescription_Text = 0,
				DB4D_QueryDescription_XML = 1,
			 } DB4D_QueryDescriptionFormat;


class CDB4DRemoteRecordCache;


class CDB4DBaseContext : public XBOX::IRefCountable
{
public:
	enum {Component_Type = 'dbcx'};
	
	virtual void DescribeQueryExecution(Boolean on) = 0;
	virtual Boolean ShouldDescribeQueryExecution() = 0;
	virtual void GetLastQueryDescription(XBOX::VString& outResult, DB4D_QueryDescriptionFormat inFormat) = 0;  // inFormat : 0 = plain text, 1 = XML
	virtual void GetLastQueryExecution(XBOX::VString& outResult, DB4D_QueryDescriptionFormat inFormat) = 0;
	virtual void SetLastQueryExecution (XBOX::VString& inExecution, DB4D_QueryDescriptionFormat inFormat) = 0;

	virtual XBOX::VValueBag* GetLastQueryPlan() const = 0;
	virtual XBOX::VValueBag* GetLastQueryPath() const = 0;

	virtual void SetTimerOnRecordLocking(sLONG WaitForLockTimer) = 0;
	virtual sLONG GetTimerOnRecordLocking() const = 0;
	// WaitForLockTimer is in milliseconds
	// 0 means no waiting time, -1 means wait indefinitly;

	virtual CDB4DBase* GetOwner(void) const = 0;
	virtual Boolean MatchBaseInContext(CDB4DBaseContextPtr InBaseContext) const = 0;
	
	virtual XBOX::VIntlMgr* GetIntlMgr() const = 0;

	virtual VErrorDB4D StartTransaction(sLONG WaitForLockTimer = 0) = 0; 
		// WaitForLockTimer is in milliseconds
		// 0 means no waiting time, -1 means wait indefinitly, -2 means use the value of parent transaction or context

	virtual VErrorDB4D CommitTransaction() = 0;
	virtual VErrorDB4D RollBackTransaction() = 0;
	virtual sLONG CurrentTransactionLevel() const = 0;

	virtual VErrorDB4D ReleaseFromConsistencySet(CDB4DSelection* InSel) = 0;
	virtual VErrorDB4D ReleaseFromConsistencySet(CDB4DSet* InSet) = 0;
	virtual VErrorDB4D ReleaseFromConsistencySet(RecIDType inRecordID) = 0;
	virtual VErrorDB4D ReleaseFromConsistencySet(CDB4DRecord* inRec) = 0;
	virtual VErrorDB4D SetConsistency(Boolean isOn) = 0;

	virtual CDB4DSqlQuery *NewSqlQuery(XBOX::VString& request, VErrorDB4D& err) = 0;

	virtual VErrorDB4D SetRelationAutoLoadNto1(const CDB4DRelation* inRel, DB4D_Rel_AutoLoadState inAutoLoadState) = 0;
	virtual VErrorDB4D SetRelationAutoLoad1toN(const CDB4DRelation* inRel, DB4D_Rel_AutoLoadState inAutoLoadState) = 0;
//	virtual VErrorDB4D SetRelationAutoLoad(sLONG inTableSourceID, sLONG inFieldSourceID, sLONG inTableDestID, sLONG inFieldDestID , DB4D_Rel_AutoLoadState inAutoLoadState) = 0;
//	virtual VErrorDB4D SetRelationAutoLoad(const CDB4DField* inSource, const CDB4DField* inDest, DB4D_Rel_AutoLoadState inAutoLoadState) = 0;

	virtual DB4D_Rel_AutoLoadState GetRelationAutoLoadNto1State(const CDB4DRelation* inRel) const = 0;
	virtual DB4D_Rel_AutoLoadState GetRelationAutoLoad1toNState(const CDB4DRelation* inRel) const = 0;
//	virtual DB4D_Rel_AutoLoadState GetRelationAutoLoadState(sLONG inTableSourceID, sLONG inFieldSourceID, sLONG inTableDestID, sLONG inFieldDestID) const = 0;
//	virtual DB4D_Rel_AutoLoadState GetRelationAutoLoadState(const CDB4DField* inSource, const CDB4DField* inDest) const = 0;

	virtual Boolean IsRelationAutoLoadNto1(const CDB4DRelation* inRel) const = 0;
	virtual Boolean IsRelationAutoLoad1toN(const CDB4DRelation* inRel) const = 0;

	virtual void ExcludeTableFromAutoRelationDestination(CDB4DTable* inTableToExclude) = 0;
	virtual void IncludeBackTableToAutoRelationDestination(CDB4DTable* inTableToInclude) = 0;
	virtual Boolean IsTableExcludedFromAutoRelationDestination(CDB4DTable* inTableToCheck) const = 0;

	virtual void SetAllRelationsToAutomatic(Boolean RelationsNto1, Boolean ForceAuto) = 0;
	virtual Boolean IsAllRelationsToAutomatic(Boolean RelationsNto1) = 0;

	// inTimeOut == -1 means "wait undefinitly", = 0 means "return immediatly", > 0 is the waiting time in milliseconds
	virtual LockPtr LockDataBaseDef(const CDB4DBase* inBase, sLONG inTimeOut = 0, Boolean inForReadOnly = false, const XBOX::VValueBag **outLockerExtraData = NULL) = 0;
	virtual LockPtr LockTableDef(const CDB4DTable* inTable, Boolean inWithFields, sLONG inTimeOut = 0, Boolean inForReadOnly = false, const XBOX::VValueBag **outLockerExtraData = NULL) = 0;
	virtual LockPtr LockFieldDef(const CDB4DField* inField, sLONG inTimeOut = 0, Boolean inForReadOnly = false, const XBOX::VValueBag **outLockerExtraData = NULL) = 0;
	virtual LockPtr LockRelationDef(const CDB4DRelation* inRelation, Boolean inWithRelatedFields, sLONG inTimeOut = 0, Boolean inForReadOnly = false, const XBOX::VValueBag **outLockerExtraData = NULL) = 0;
	virtual LockPtr LockIndexDef(const CDB4DIndex* inIndex, sLONG inTimeOut = 0, Boolean inForReadOnly = false, const XBOX::VValueBag **outLockerExtraData = NULL) = 0;

	virtual Boolean LockMultipleTables(const std::vector<const CDB4DTable*>& inTables, std::vector<LockPtr>& outLocks, sLONG inTimeOut = 0, Boolean inForReadOnly = false) = 0;
	virtual void UnLockMultipleTables(const std::vector<LockPtr>& inLocks) = 0;

	virtual VErrorDB4D UnLockStructObject(LockPtr inLocker) = 0;

	virtual void SetClientRequestStreams(XBOX::VStream* OutputStream, XBOX::VStream* InputStream) = 0;
	virtual void SetServerRequestStreams(XBOX::VStream* InputStream, XBOX::VStream* OutputStream) = 0;

	virtual Boolean IsRemote() const = 0;

	virtual CDB4DContext* GetContextOwner() = 0;

	virtual void SendlastRemoteInfo() = 0;

	virtual CDB4DRemoteRecordCache* StartCachingRemoteRecords(CDB4DSelection* inSel, RecIDType FromRecIndex, RecIDType ToRecIndex,  CDB4DFieldCacheCollection* inWhichFields, const std::vector<uBYTE>& inWayOfLocking) = 0;
	virtual void StopCachingRemoteRecords(CDB4DRemoteRecordCache* inCacheInfo) = 0;

	virtual VErrorDB4D SetIdleTimeOut(uLONG inMilliseconds) = 0; // should be called SetIdleTimeOut

	virtual void FreeAllJSFuncs() = 0;

	virtual void SetJSContext(XBOX::VJSGlobalContext* inJSContext, bool propagateToParent = true) = 0;
	virtual XBOX::VJSGlobalContext*	GetJSContext() const = 0;

	virtual void SetCurrentUser(const XBOX::VUUID& inUserID, CUAGSession* inSession) = 0;

	virtual CUAGSession* GetCurrentUserSession() = 0;

	virtual void CleanUpForReuse() = 0;

};


class CDB4DFieldCacheCollection : public XBOX::IRefCountable
{
public:
	enum {Component_Type = 'dbfc'};

	virtual VErrorDB4D AddField(CDB4DField* inField) = 0;

	virtual VErrorDB4D WriteToStream(XBOX::VStream* ToStream) = 0;
	virtual VErrorDB4D ReadFromStream(XBOX::VStream* FromStream) = 0;

};


class CDB4DField : public XBOX::IRefCountable
{
public:
	enum {Component_Type = 'dbFI'};
	
	virtual CDB4DTable* GetOwner(CDB4DBaseContextPtr inContext = NULL) = 0;
	virtual DB4D_FieldID GetID(CDB4DBaseContextPtr inContext = NULL) const = 0;
	virtual void GetUUID(XBOX::VUUID& outID, CDB4DBaseContextPtr inContext = NULL) const = 0;

	virtual void GetName( XBOX::VString& outName, CDB4DBaseContextPtr inContext = NULL) const = 0;
	virtual VErrorDB4D SetName( const XBOX::VString& inName, CDB4DBaseContextPtr inContext = NULL) = 0;

	virtual DB4DFieldType GetType(CDB4DBaseContextPtr inContext = NULL) const = 0;
	virtual DB4DFieldType GetRealType(CDB4DBaseContextPtr inContext = NULL) const = 0;
	virtual VErrorDB4D SetType(DB4DFieldType inType, VDB4DProgressIndicator* InProgress = NULL, CDB4DBaseContextPtr inContext = NULL) = 0;

	virtual sLONG GetTextSwitchSize(CDB4DBaseContextPtr inContext = NULL) const = 0;
	virtual VErrorDB4D SetTextSwitchSize(sLONG inLength, CDB4DBaseContextPtr inContext = NULL) = 0;

	virtual sLONG GetBlobSwitchSize(CDB4DBaseContextPtr inContext = NULL) const = 0;
	virtual VErrorDB4D SetBlobSwitchSize(sLONG inLength, CDB4DBaseContextPtr inContext = NULL) = 0;

	virtual sLONG GetLimitingLength(CDB4DBaseContextPtr inContext = NULL) const = 0;
	virtual VErrorDB4D SetLimitingLength(sLONG inLength, CDB4DBaseContextPtr inContext = NULL) = 0;

	virtual Boolean IsNot_Null(CDB4DBaseContextPtr inContext = NULL) const = 0;
	virtual VErrorDB4D SetNot_Null(Boolean Not_Null, VDB4DProgressIndicator* InProgress = nil, CDB4DBaseContextPtr inContext = NULL) = 0;

	virtual Boolean IsUnique(CDB4DBaseContextPtr inContext = NULL) const = 0;
	virtual VErrorDB4D SetUnique(Boolean unique, VDB4DProgressIndicator* InProgress = nil, CDB4DBaseContextPtr inContext = NULL) = 0;

	virtual Boolean IsAutoSequence(CDB4DBaseContextPtr inContext = NULL) const = 0;
	virtual VErrorDB4D SetAutoSequence(Boolean autoseq, CDB4DBaseContextPtr inContext = NULL) = 0;
	
	virtual Boolean IsAutoGenerate(CDB4DBaseContextPtr inContext = NULL) const = 0;
	virtual VErrorDB4D SetAutoGenerate(Boolean autogenerate, CDB4DBaseContextPtr inContext = NULL) = 0;

	virtual Boolean IsStoreAsUTF8(CDB4DBaseContextPtr inContext = NULL) const = 0;
	virtual VErrorDB4D SetStoreAsUTF8(Boolean storeUTF8, CDB4DBaseContextPtr inContext = NULL) = 0;

	virtual Boolean IsStoreAsUUID(CDB4DBaseContextPtr inContext = NULL) const = 0;
	virtual VErrorDB4D SetStoreAsUUID(Boolean storeUUID, CDB4DBaseContextPtr inContext = NULL) = 0;

	virtual Boolean IsStoreOutside(CDB4DBaseContextPtr inContext = NULL) const = 0;
	virtual VErrorDB4D SetStoreOutside(Boolean storeOutside, VDB4DProgressIndicator* InProgress = nil, CDB4DBaseContextPtr inContext = NULL) = 0;

	virtual Boolean IsStyledText(CDB4DBaseContextPtr inContext = NULL) const = 0;
	virtual VErrorDB4D SetStyledText(Boolean styledText, VDB4DProgressIndicator* InProgress = nil, CDB4DBaseContextPtr inContext = NULL) = 0;

	virtual Boolean IsHiddenInRest(CDB4DBaseContextPtr inContext = NULL) const = 0;
	virtual VErrorDB4D SetHideInRest(Boolean hideInRest, VDB4DProgressIndicator* InProgress = nil, CDB4DBaseContextPtr inContext = NULL) = 0;

	virtual VErrorDB4D SetDefinition(const XBOX::VValueBag& inFieldDefinition, VDB4DProgressIndicator* InProgress = NULL, CDB4DBaseContextPtr inContext = NULL) = 0;
	virtual XBOX::VValueBag* CreateDefinition(CDB4DBaseContextPtr inContext = NULL) const = 0;

	virtual Boolean IsFullTextIndexable(CDB4DBaseContextPtr inContext = NULL) const = 0;
	virtual Boolean IsFullTextIndexed(CDB4DBaseContextPtr inContext = NULL) const = 0;

	virtual Boolean IsIndexable(CDB4DBaseContextPtr inContext = NULL) const = 0;
	virtual Boolean IsIndexed(CDB4DBaseContextPtr inContext = NULL) const = 0;

	virtual Boolean IsPrimIndexed(CDB4DBaseContextPtr inContext = NULL) const = 0;

	virtual Boolean	IsPrimaryKeyCompatible( CDB4DBaseContextPtr inContext = NULL) const = 0;

	virtual Boolean CanBePartOfRelation(CDB4DBaseContextPtr inContext = NULL) const = 0;

	virtual Boolean Drop(VDB4DProgressIndicator* InProgress = NULL, CDB4DBaseContextPtr inContext = NULL) = 0;

	virtual const XBOX::VValueBag* RetainExtraProperties(VErrorDB4D &err, CDB4DBaseContextPtr inContext = NULL) = 0;
	virtual VErrorDB4D SetExtraProperties(XBOX::VValueBag* inExtraProperties, CDB4DBaseContextPtr inContext = NULL) = 0;

	virtual CDB4DRelation* RetainFirstNto1Relation(CDB4DBaseContextPtr inContext = NULL) = 0;

	// for a subtable field, returns the relation whose destination is the subtable.
	// returns NULL if this is not a subtable field.
	virtual CDB4DRelation* RetainRelation_SubTable(CDB4DBaseContextPtr inContext = NULL) = 0;
	
	//virtual CDB4DRelation* RetainRelation_SourceLienAller_V6(Boolean AutoRelOnly, CDB4DBaseContextPtr inContext) = 0;
	//virtual CDB4DRelation* RetainRelation_CibleLienRetour_V6(Boolean AutoRelOnly, CDB4DBaseContextPtr inContext) = 0;
	virtual VErrorDB4D GetListOfRelations1_To_N(RelationArray& outRelations, Boolean AutoRelOnly, CDB4DBaseContextPtr inContext) = 0;
	virtual VErrorDB4D GetListOfRelationsN_To_1(RelationArray& outRelations, Boolean AutoRelOnly, CDB4DBaseContextPtr inContext) = 0;

	virtual CDB4DIndex* RetainIndex(Boolean MustBeSortable = false, CDB4DBaseContextPtr inContext = NULL, Boolean MustBeValid = true, Boolean Occupy = false) const = 0;
	virtual CDB4DIndex* RetainFullTextIndex(CDB4DBaseContextPtr inContext = NULL, Boolean MustBeValid = true, Boolean Occupy = false) const = 0;

	virtual Boolean IsNeverNull(CDB4DBaseContextPtr inContext = NULL) const = 0;
	virtual VErrorDB4D SetNeverNullState(Boolean inState, CDB4DBaseContextPtr inContext = NULL) = 0;

	virtual sLONG GetStamp(CDB4DBaseContextPtr inContext = NULL) const = 0;

	virtual VErrorDB4D RetainIndexes(ArrayOf_CDB4DIndex& outIndexes, CDB4DBaseContextPtr inContext = NULL) = 0;

};


class CDB4DTable : public XBOX::IRefCountable
{
public:
	enum {Component_Type = 'dbTA'};
	
	virtual CDB4DBase* GetOwner(CDB4DBaseContextPtr inContext = NULL) = 0;

	virtual void GetName( XBOX::VString& outName, CDB4DBaseContextPtr inContext = NULL) const = 0;
	virtual VErrorDB4D SetName( const XBOX::VString& inName, CDB4DBaseContextPtr inContext = NULL) = 0;
	virtual	VErrorDB4D SetNameNoNameCheck( const XBOX::VString& inName, CDB4DBaseContextPtr inContext = NULL) = 0;		// sc 11/04/2007
	virtual VErrorDB4D SetRecordName(const XBOX::VString& inName, CDB4DBaseContextPtr inContext = NULL) = 0;
	virtual void GetRecordName( XBOX::VString& outName, CDB4DBaseContextPtr inContext = NULL) const = 0;

	virtual VErrorDB4D SetSchemaID(DB4D_SchemaID inSchemaID, CDB4DBaseContext* inContext = NULL) = 0;
	virtual DB4D_SchemaID GetSchemaID(CDB4DBaseContext* inContext = NULL) const = 0;

	virtual VErrorDB4D SetFullyDeleteRecords(Boolean FullyDeleteState, CDB4DBaseContextPtr inContext = NULL) = 0;
	virtual Boolean GetFullyDeleteRecordsState() const = 0;

	virtual DB4D_TableID GetID(CDB4DBaseContextPtr inContext = NULL) const = 0;
	virtual void GetUUID(XBOX::VUUID& outID, CDB4DBaseContextPtr inContext = NULL) const = 0;
	virtual XBOX::VValueBag *CreateDefinition(CDB4DBaseContextPtr inContext = NULL) const = 0;

	virtual sLONG CountFields(CDB4DBaseContextPtr inContext = NULL) const = 0;

	virtual CDB4DField* RetainNthField(sLONG inIndex, CDB4DBaseContextPtr inContext = NULL) const = 0;
	virtual CDB4DField* FindAndRetainField(const XBOX::VString& inName, CDB4DBaseContextPtr inContext = NULL) const = 0;
	virtual CDB4DField* FindAndRetainField(const XBOX::VUUID& inUUID, CDB4DBaseContextPtr inContext = NULL) const = 0;
	virtual DB4D_FieldID GetFieldByName( const XBOX::VString& inName, CDB4DBaseContextPtr inContext = NULL) const = 0;

	virtual VErrorDB4D AddField(const XBOX::VString& inName, DB4DFieldType inType, sLONG inSize, DB4DFieldAttributes inAttributes, 
								VDB4DProgressIndicator* InProgress = NULL, CDB4DBaseContextPtr inContext = NULL) = 0;

	virtual sLONG AddFields(const XBOX::VValueBag& inFieldsDefinition, VErrorDB4D &err, 
							VDB4DProgressIndicator* InProgress = NULL, CDB4DBaseContextPtr inContext = NULL) = 0;

	virtual VErrorDB4D SetFieldDefinition(DB4D_FieldID inFieldID, const XBOX::VValueBag& inFieldDefinition, 
											VDB4DProgressIndicator* InProgress = NULL, CDB4DBaseContextPtr inContext = NULL) = 0;

	virtual VErrorDB4D SetFieldDefinition( DB4D_FieldID inFieldID, XBOX::VString& inName, DB4DFieldType inType, sLONG inSize, 
							DB4DFieldAttributes inAttributes, VDB4DProgressIndicator* InProgress = NULL, CDB4DBaseContextPtr inContext = NULL) = 0;

	virtual void GetFieldDefinition( DB4D_FieldID inFieldID, XBOX::VString& outName, DB4DFieldType *outType, sLONG *outSize, 
									DB4DFieldAttributes *outAttributes, CDB4DBaseContextPtr inContext = NULL) const = 0;

	virtual Boolean DropField(DB4D_FieldID inFieldID, VDB4DProgressIndicator* InProgress = NULL, CDB4DBaseContextPtr inContext = NULL) = 0;

	virtual VErrorDB4D SetIdentifyingFields(const CDB4DFieldArray& inFields, VDB4DProgressIndicator* InProgress = NULL, CDB4DBaseContextPtr inContext = NULL) = 0;

	virtual VErrorDB4D SetPrimaryKey(const CDB4DFieldArray& inFields, VDB4DProgressIndicator* inProgress = NULL, 
									Boolean CanReplaceExistingOne = false, CDB4DBaseContextPtr inContext = NULL, XBOX::VString* inPrimaryKeyName = NULL) = 0;

	virtual VErrorDB4D RetainPrimaryKey(CDB4DFieldArray& outFields, CDB4DBaseContextPtr inContext = NULL) const = 0;

	virtual	bool HasPrimaryKey( CDB4DBaseContextPtr inContext = NULL) const = 0;

	virtual Boolean IsIndexable(DB4D_FieldID inFieldID, CDB4DBaseContextPtr inContext = NULL) = 0;
	virtual Boolean IsIndexed(DB4D_FieldID inFieldID, CDB4DBaseContextPtr inContext = NULL) = 0;
	virtual Boolean IsPrimIndexed(DB4D_FieldID inFieldID, CDB4DBaseContextPtr inContext = NULL) = 0;


	virtual Boolean Drop(VDB4DProgressIndicator* InProgress = NULL, CDB4DBaseContextPtr inContext = NULL) = 0;


	virtual CDB4DQuery *NewQuery() = 0;
	virtual CDB4DQueryResult *NewQueryResult( CDB4DSelection *inSelection, CDB4DSet *inSet, CDB4DSet *inLockedSet, CDB4DRecord *inFirstRecord) = 0;

	virtual CDB4DSelectionPtr ExecuteQuery( CDB4DQuery *inQuery, CDB4DBaseContextPtr inContext, CDB4DSelectionPtr Filter = NULL, 
											VDB4DProgressIndicator* InProgress = NULL, DB4D_Way_of_Locking HowToLock = DB4D_Do_Not_Lock, 
											sLONG limit = 0, CDB4DSet* outLockSet = NULL, VErrorDB4D *outErr = NULL) = 0;

	virtual CDB4DQueryResult* ExecuteQuery(CDB4DQuery* inQuery, CDB4DBaseContext* inContext, CDB4DQueryOptions* inOptions, VDB4DProgressIndicator* inProgress, VErrorDB4D& outError) = 0;

	virtual CDB4DSelectionPtr SelectAllRecords(CDB4DBaseContextPtr inContext, VErrorDB4D* outErr = NULL, 
												DB4D_Way_of_Locking HowToLock = DB4D_Do_Not_Lock, CDB4DSet* outLockSet = NULL) = 0;

	virtual	CDB4DSelectionPtr	SelectAllRecordsAndLoadFirstOne( CDB4DBaseContextPtr inContext, VErrorDB4D* outErr, bool inLoadRecordReadOnly, CDB4DRecord **outFirstRecord) = 0;

	virtual RecIDType CountRecordsInTable(CDB4DBaseContextPtr inContext) const = 0;
	virtual RecIDType GetMaxRecordsInTable(CDB4DBaseContextPtr inContext) const = 0;
	//virtual sLONG8 GetSequenceNumber(CDB4DBaseContextPtr inContext) = 0;

	
	virtual CDB4DRecord *NewRecord(CDB4DBaseContextPtr inContext) = 0;
	virtual CDB4DRecord *LoadRecord( RecIDType inRecordID, DB4D_Way_of_Locking HowToLock, CDB4DBaseContextPtr inContext, Boolean WithSubTable = true, Boolean* outLockWasKeptInTrans = NULL) = 0;
	virtual VErrorDB4D DeleteRecord( RecIDType inRecordID, CDB4DBaseContextPtr inContext) = 0;

	virtual void UnlockRecordAfterRelease_IfNotModified(RecIDType inRecordID, CDB4DBaseContextPtr inContext) = 0;
	virtual void UnlockRecordsAfterRelease_IfNotModified(const std::vector<RecIDType> &inRecordIDs, CDB4DBaseContextPtr inContext) = 0;

	virtual void* LoadRawRecord(RecIDType inRecordID, DB4D_Way_of_Locking HowToLock, CDB4DBaseContextPtr inContext, 
								VErrorDB4D& outErr, Boolean* outCouldLock = NULL, Boolean* outLockWasKeptInTrans = NULL ) = 0;

	virtual CDB4DImpExp* NewImpExp() = 0;

	virtual CDB4DSortingCriterias* NewSortingCriterias() = 0;

	
	virtual Boolean LockTable(CDB4DBaseContextPtr inContext, Boolean WaitForEndOfRecordLocks = false, sLONG TimeToWaitForEndOfRecordLocks = 0) = 0;  // timeout in milliseconds, -1 = forever, 0 = no timeout
																					// returns true if you could lock the table
	virtual void UnLockTable(CDB4DBaseContextPtr inContext) = 0;


	virtual CDB4DSet* NewSet() const = 0;

	virtual CDB4DSelectionPtr NewSelection(DB4D_SelectionType inSelectionType) const = 0;

	virtual CDB4DColumnFormula* NewColumnFormula() const = 0;

	virtual const XBOX::VValueBag* RetainExtraProperties(VErrorDB4D &err, CDB4DBaseContextPtr inContext = NULL) = 0;
	virtual VErrorDB4D SetExtraProperties(XBOX::VValueBag* inExtraProperties, CDB4DBaseContextPtr inContext = NULL) = 0;

	virtual CDB4DAutoSeqNumber* RetainAutoSeqNumber(CDB4DBaseContextPtr inContext = NULL) = 0;

	virtual VErrorDB4D GetListOfRelations1_To_N(RelationArray& outRelations, Boolean AutoRelOnly, CDB4DBaseContextPtr inContext) = 0;
	virtual VErrorDB4D GetListOfRelationsN_To_1(RelationArray& outRelations, Boolean AutoRelOnly, CDB4DBaseContextPtr inContext) = 0;

	virtual VErrorDB4D DelayIndexes() = 0;
	virtual VErrorDB4D AwakeIndexes(std::vector<CDB4DIndex*>& outIndexWithProblems, VDB4DProgressIndicator* inProgress = NULL, Boolean WaitForCompletion = true) = 0;

	virtual sLONG GetStamp(CDB4DBaseContextPtr inContext = NULL) const = 0;
	virtual void GetFieldsStamps(StampsVector& outStamps, CDB4DBaseContextPtr inContext = NULL) const = 0;

	virtual VErrorDB4D RetainIndexes(ArrayOf_CDB4DIndex& outIndexes, CDB4DBaseContextPtr inContext = NULL) = 0;

	virtual VErrorDB4D RetainExistingFields(std::vector<CDB4DField*>& outFields) = 0;
	virtual sLONG CountExistingFields() = 0;

	virtual CDB4DFieldCacheCollection* NewFieldCacheCollection(const XBOX::VString& Signature) const = 0;

	virtual VErrorDB4D Truncate(CDB4DBaseContextPtr inContext = NULL, VDB4DProgressIndicator* inProgress = NULL) = 0;

	virtual VErrorDB4D ActivateAutomaticRelations_N_To_1(CDB4DRecord* inRecord, std::vector<CachedRelatedRecord>& outResult, 
															CDB4DBaseContext* InContext, const std::vector<uBYTE>& inWayOfLocking) = 0;
	// inWayOfLocking starts with Table# 1 beeing element 0 of the vector

	virtual VErrorDB4D ActivateAutomaticRelations_1_To_N(CDB4DRecord* inRecord, std::vector<CachedRelatedRecord>& outResult, 
															CDB4DBaseContext* InContext, const std::vector<uBYTE>& inWayOfLocking,
															CDB4DField* onOneFieldOnly = nil, Boolean onOldvalue = false, 
															Boolean AutomaticOnly = false, Boolean OneLevelOnly = true) = 0;
	// inWayOfLocking starts with Table# 1 beeing element 0 of the vector

	virtual VErrorDB4D ActivateAllAutomaticRelations(CDB4DRecord* inRecord, std::vector<CachedRelatedRecord>& outResult, 
													CDB4DBaseContext* InContext, const std::vector<uBYTE>& inWayOfLocking) = 0;
	// inWayOfLocking starts with Table# 1 beeing element 0 of the vector

	virtual VErrorDB4D DataToCollection(const std::vector<RecIDType>& inRecIDs, DB4DCollectionManager& Collection, CDB4DBaseContextPtr inContext, sLONG FromRecInSel, sLONG ToRecInSel,
											VDB4DProgressIndicator* InProgress = nil) = 0;
	// Selection starts from 0,  (-1 in ToRecInSel means up to the end)

	virtual VErrorDB4D SetKeepRecordStamps(bool inKeepRecordStamps, CDB4DBaseContextPtr inContext, VDB4DProgressIndicator* InProgress = nil) = 0;

	virtual bool GetKeepRecordStamps() const = 0;

#if 0
	virtual VErrorDB4D GetModificationsSinceStamp(uLONG stamp, XBOX::VStream& outStream, uLONG& outLastStamp, sLONG& outNbRows, CDB4DBaseContextPtr inContext, std::vector<sLONG>& cols) = 0;

	virtual VErrorDB4D IntegrateModifications(XBOX::VStream& inStream, CDB4DBaseContextPtr inContext, std::vector<sLONG>& cols) = 0;

	virtual VErrorDB4D GetOneRow(XBOX::VStream& inStream, CDB4DBaseContextPtr inContext, sBYTE& outAction, uLONG& outStamp, RecIDType& outRecID, std::vector<XBOX::VValueSingle*>& outValues) = 0;
#endif

	virtual CDB4DEntityModel* BuildEntityModel() = 0;

	virtual VErrorDB4D SetKeepRecordSyncInfo(bool inKeepRecordSyncInfo, CDB4DBaseContextPtr inContext, VDB4DProgressIndicator* InProgress = nil) = 0;

	virtual bool GetKeepRecordSyncInfo() const = 0;

	virtual VErrorDB4D SetHideInRest(bool hideInRest, CDB4DBaseContextPtr inContext, VDB4DProgressIndicator* InProgress = nil) = 0;

	virtual bool IsHiddenInRest(CDB4DBaseContextPtr inContext) const = 0;

	virtual VErrorDB4D GetModificationsSinceStampWithPrimKey(uLONG8 stamp, XBOX::VStream& outStream, uLONG8& outLastStamp, sLONG& outNbRows, 
																CDB4DBaseContextPtr inContext, std::vector<sLONG>& cols,
																CDB4DSelection* filter, sLONG8 skip, sLONG8 top,
																std::vector<XBOX::VString*>* inImageFormats = 0) = 0;
	//  filter can be nil
	//  skip and top can be -1, it means all modifications
	//  cols can be empty, it means all fields
	//  stamp can be -1 it means : do not care for modifications and return all records
	//  stamp can be -2 it means : return all deleted records keys


	virtual VErrorDB4D IntegrateModificationsWithPrimKey(XBOX::VStream& inStream, CDB4DBaseContextPtr inContext, std::vector<sLONG>& cols, bool sourceOverDest,
															uLONG8& ioFirstDestStampToCheck, uLONG8& outErrorStamp, bool inBinary = true) = 0;

	virtual VErrorDB4D GetOneRowWithPrimKey(XBOX::VStream& inStream, CDB4DBaseContextPtr inContext, sBYTE& outAction, uLONG8& outStamp, XBOX::VTime& outTimeStamp, 
												std::vector<XBOX::VValueSingle*>& outPrimKey, std::vector<XBOX::VValueSingle*>& outValues) = 0;

	virtual void WhoLockedRecord( RecIDType inRecordID, CDB4DBaseContextPtr inContext, DB4D_KindOfLock& outLockType, const XBOX::VValueBag **outLockingContextRetainedExtraData) const = 0;

	virtual VErrorDB4D GetFragmentation(sLONG8& outTotalRec, sLONG8& outFrags, CDB4DBaseContext* inContext) = 0;

	virtual VErrorDB4D ImportRecords(CDB4DBaseContext* inContext, XBOX::VFolder* inFolder, VDB4DProgressIndicator* inProgress, ExportOption& options) = 0;

	virtual CDB4DField* RetainPseudoField(DB4D_Pseudo_Field_Kind kind) = 0;

	virtual VErrorDB4D GetListOfTablesForCascadingDelete(std::set<sLONG>& outSet) = 0;

	virtual sLONG GetSeqRatioCorrector() const = 0; 

	virtual void SetSeqRatioCorrector(sLONG newValue) = 0;

};



class CDB4DSelection : public XBOX::IRefCountable
{
public:
	enum {Component_Type = 'dbsl'};
	
	virtual DB4D_TableID GetTableRef() const = 0;
	virtual CDB4DBase* GetBaseRef() const = 0;

	virtual void* LoadRawRecord(RecIDType inRecordIndex, DB4D_Way_of_Locking HowToLock, CDB4DBaseContextPtr inContext, 
								VErrorDB4D& outErr, Boolean* outCouldLock = NULL, Boolean* outLockWasKeptInTrans = NULL) = 0;

	virtual RecIDType CountRecordsInSelection(CDB4DBaseContextPtr inContext) const = 0;
	virtual Boolean IsEmpty(CDB4DBaseContextPtr inContext) const = 0;

	virtual CDB4DRecord *LoadSelectedRecord( RecIDType inRecordIndex, DB4D_Way_of_Locking HowToLock, CDB4DBaseContextPtr inContext, Boolean WithSubTable = true, Boolean* outLockWasKeptInTrans = NULL) = 0;

	virtual RecIDType GetSelectedRecordID( RecIDType inRecordIndex, CDB4DBaseContextPtr inContext) = 0;

	// SortSelection runs on Server if it is a Remote Database
	virtual Boolean SortSelection(DB4D_FieldID inFieldID, Boolean inAscending, VDB4DProgressIndicator* InProgress = NULL, CDB4DBaseContextPtr inContext = NULL) = 0;
	virtual Boolean SortSelection(CDB4DSortingCriteriasPtr inCriterias, VDB4DProgressIndicator* InProgress = NULL, CDB4DBaseContextPtr inContext = NULL) = 0;

	// SortSelectionOnClient runs on Server if it is a Remote Database and there are no language formulas to execute
	virtual Boolean SortSelectionOnClient(DB4D_FieldID inFieldID, Boolean inAscending, VDB4DProgressIndicator* InProgress = NULL, CDB4DBaseContextPtr inContext = NULL) = 0;
	virtual Boolean SortSelectionOnClient(CDB4DSortingCriteriasPtr inCriterias, VDB4DProgressIndicator* InProgress = NULL, CDB4DBaseContextPtr inContext = NULL) = 0;

	virtual CDB4DSelection* Clone(VErrorDB4D* outErr) const = 0;
	virtual CDB4DSet* ConvertToSet(VErrorDB4D& err) = 0;

	virtual VErrorDB4D AddRecord(const CDB4DRecord* inRecToAdd, Boolean AtTheEnd /*= true*/, CDB4DBaseContextPtr inContext) = 0;
	virtual VErrorDB4D AddRecordID(RecIDType inRecToAdd, Boolean AtTheEnd /*= true*/, CDB4DBaseContextPtr inContext) = 0;

	virtual void Touch() = 0;
	virtual uLONG GetModificationCounter() const = 0;

	virtual VErrorDB4D FillArray(sLONG* outArray, sLONG inMaxElements, CDB4DBaseContextPtr inContext) = 0;
	virtual VErrorDB4D FillFromArray(const sLONG* inArray, sLONG inNbElements, CDB4DBaseContextPtr inContext) = 0;

	virtual VErrorDB4D FillArray(xArrayOfLong &outArray, CDB4DBaseContextPtr inContext) = 0;
	virtual VErrorDB4D FillFromArray(const xArrayOfLong &inArray, CDB4DBaseContextPtr inContext) = 0;

	virtual VErrorDB4D FillArray(DB4DCollectionManager& outCollection, CDB4DBaseContextPtr inContext) = 0;
	virtual VErrorDB4D FillFromArray(DB4DCollectionManager& inCollection, CDB4DBaseContextPtr inContext) = 0;

	virtual VErrorDB4D FillArrayOfBits(void* outArray, sLONG inMaxElements) = 0;
	virtual VErrorDB4D FillFromArrayOfBits(const void* inArray, sLONG inNbElements) = 0;

	virtual VErrorDB4D DataToCollection(DB4DCollectionManager& Collection, CDB4DBaseContextPtr inContext, sLONG FromRecInSel, sLONG ToRecInSel,
																			VDB4DProgressIndicator* InProgress = nil) = 0;
	// Selection starts from 0,  (-1 in ToRecInSel means up to the end)

	virtual VErrorDB4D CollectionToData(DB4DCollectionManager& Collection, CDB4DBaseContextPtr inContext, Boolean AddToSel, Boolean CreateAlways,
																			CDB4DSet* &outLockedRecords, VDB4DProgressIndicator* InProgress = nil) = 0;
	
	virtual VErrorDB4D GetDistinctValues(CDB4DField* inField, DB4DCollectionManager& outCollection, CDB4DBaseContextPtr inContext, 
											VDB4DProgressIndicator* InProgress = nil, Boolean inCaseSensitive = false) = 0;

	virtual VErrorDB4D GetDistinctValues(CDB4DField* inField, DB4DCollectionManager& outCollection, CDB4DBaseContextPtr inContext, 
										VDB4DProgressIndicator* InProgress ,const XBOX::VCompareOptions& inOptions) = 0;

	virtual VErrorDB4D GetDistinctValues(CDB4DEntityAttribute* inAttribute, DB4DCollectionManager& outCollection, CDB4DBaseContextPtr inContext, 
		VDB4DProgressIndicator* InProgress = nil, Boolean inCaseSensitive = false) = 0;

	virtual VErrorDB4D GetDistinctValues(CDB4DEntityAttribute* inAttribute, DB4DCollectionManager& outCollection, CDB4DBaseContextPtr inContext, 
		VDB4DProgressIndicator* InProgress ,const XBOX::VCompareOptions& inOptions) = 0;

	virtual VErrorDB4D DeleteRecords(CDB4DBaseContextPtr inContext, CDB4DSet* outNotDeletedOnes = nil, VDB4DProgressIndicator* InProgress = nil) = 0;

	virtual VErrorDB4D RemoveRecordNumber(CDB4DBaseContextPtr inContext, RecIDType inRecToRemove) = 0;
	virtual VErrorDB4D RemoveSet(CDB4DBaseContextPtr inContext, CDB4DSet* inRecsToRemove) = 0;
	virtual VErrorDB4D RemoveSelectedRecord(CDB4DBaseContextPtr inContext, RecIDType inRecordIndexToRemove) = 0;
	virtual VErrorDB4D RemoveSelectedRange(CDB4DBaseContextPtr inContext, RecIDType inFromRecordIndexToRemove, RecIDType inToRecordIndexToRemove) = 0;

	virtual VErrorDB4D BuildOneRecSelection(RecIDType inRecNum) = 0;

	virtual VErrorDB4D ReduceSelection(RecIDType inNbRec, CDB4DBaseContextPtr Context) = 0;

	virtual RecIDType GetRecordPos(RecIDType inRecordID, CDB4DBaseContextPtr inContext) = 0;

	virtual CDB4DSet* GenerateSetFromRange(RecIDType inRecordIndex1, RecIDType inRecordIndex2, VErrorDB4D& err, CDB4DBaseContextPtr inContext) = 0;

	virtual CDB4DSet* GenerateSetFromRecordID(RecIDType inRecordID, RecIDType inRecordIndex2, VErrorDB4D& err, CDB4DBaseContextPtr inContext) = 0;

	virtual CDB4DSelection* RelateOneSelection(sLONG TargetOneTable, VErrorDB4D& err, CDB4DBaseContextPtr inContext, 
												VDB4DProgressIndicator* InProgress = nil, std::vector<CDB4DRelation*> *inPath = nil, 
												DB4D_Way_of_Locking HowToLock = DB4D_Do_Not_Lock, CDB4DSet* outLockSet = NULL) = 0;

	virtual CDB4DSelection* RelateManySelection(CDB4DField* inRelationStart, VErrorDB4D& err, CDB4DBaseContextPtr inContext, 
												VDB4DProgressIndicator* InProgress = nil, DB4D_Way_of_Locking HowToLock = DB4D_Do_Not_Lock, CDB4DSet* outLockSet = NULL) = 0;

	virtual VErrorDB4D ToClient(XBOX::VStream* into, CDB4DBaseContext* inContext) = 0;
	virtual VErrorDB4D ToServer(XBOX::VStream* into, CDB4DBaseContext* inContext) = 0;

	virtual void MarkOnServerAsPermanent() = 0; // to be called on server
	virtual void UnMarkOnServerAsPermanent(bool willResend) = 0; // to be called on server

	virtual VErrorDB4D ExportToSQL(CDB4DBaseContext* inContext, XBOX::VFolder* inFolder, VDB4DProgressIndicator* inProgress, ExportOption& options) = 0;
	
	virtual void SetAssociatedModel(CDB4DEntityModel* em) = 0;

	virtual void SetQueryPlan(XBOX::VValueBag* queryplan) = 0;
	virtual void SetQueryPath(XBOX::VValueBag* queryplan) = 0;

	virtual XBOX::VValueBag* GetQueryPlan() = 0;
	virtual XBOX::VValueBag* GetQueryPath() = 0;

	//virtual VErrorDB4D ConvertToJSObject(CDB4DBaseContext* inContext, XBOX::VJSArray& outArr, const XBOX::VString& inAttributeList, bool withKey, bool allowEmptyAttList, sLONG from, sLONG count) = 0;


};

/*!
	@class	CDB4DSet
	@abstract	Interface for manipulating a DB4D Set.
	@discussion
*/

class CDB4DSet : public XBOX::IRefCountable
{
public:
	enum {Component_Type = 'dbbt'};
	
	virtual DB4D_TableID GetTableRef() const = 0;
	virtual CDB4DBase* GetBaseRef() const = 0;

	virtual RecIDType CountRecordsInSet() const = 0;
	virtual RecIDType GetMaxRecordsInSet() const = 0;
	virtual VErrorDB4D SetMaxRecordsInSet(RecIDType inMax) const = 0;

	// available on client
	virtual Boolean IsOn(RecIDType inPos) const = 0;
	virtual RecIDType FindNextOne(RecIDType inFirstToLook) const = 0;
	virtual	RecIDType FindPreviousOne(RecIDType inFirstToLook) const = 0;

	// NOT available on client
	virtual VErrorDB4D ClearOrSet(RecIDType inPos, Boolean inValue) = 0;
	virtual VErrorDB4D ClearOrSetAll(Boolean inValue) = 0;
	virtual VErrorDB4D And(const CDB4DSet& other) = 0;
	virtual	VErrorDB4D Or(const CDB4DSet& other) = 0;
	virtual	VErrorDB4D Minus(const CDB4DSet& other) = 0;
	virtual	VErrorDB4D Invert() = 0;
	
	virtual VErrorDB4D CloneFrom(const CDB4DSet& other) = 0;
	virtual CDB4DSet* Clone(VErrorDB4D* outErr = nil) const = 0;

	 
	virtual VErrorDB4D Compact() = 0;
	virtual CDB4DSelection* ConvertToSelection(VErrorDB4D& err, CDB4DBaseContext* inContext) const = 0;

	// available on client
	virtual VErrorDB4D FillArrayOfLongs(sLONG* outArray, sLONG inMaxElements) = 0;
	virtual VErrorDB4D FillFromArrayOfLongs(const sLONG* inArray, sLONG inNbElements) = 0;

	// available on client
	virtual VErrorDB4D FillArrayOfBits(void* outArray, sLONG inMaxElements) = 0;
	virtual VErrorDB4D FillFromArrayOfBits(const void* inArray, sLONG inNbElements) = 0;

	virtual VErrorDB4D WriteToStream(XBOX::VStream& outStream) = 0;
	virtual VErrorDB4D ReadFromStream(XBOX::VStream& inStream) = 0;

	virtual VErrorDB4D ToClient(XBOX::VStream* into, CDB4DBaseContext* inContext) = 0;
	virtual VErrorDB4D ToServer(XBOX::VStream* into, CDB4DBaseContext* inContext, bool inKeepOnServer) = 0;

	virtual void MarkOnServerAsPermanent() = 0; // to be called on server
	virtual void UnMarkOnServerAsPermanent(bool willResend) = 0; // to be called on server

};



typedef std::vector<void*> RawRecordsVector;

class DB4D_API DB4DSQLExpression
{
public:
	virtual void Retain() = 0;
	virtual void Release() = 0;

	virtual uBOOL Execute(CDB4DBaseContext* inBaseContext, void* &ioLanguageContext, void* onWhichRecord, VErrorDB4D& outErr) = 0;
	// ioLanguageContext is NULL the first time, it is computed from inBaseContext and returned by Execute, then it is cached by DB4D and reused each time
	// returns 2 for NULL, else it is a boolean
	
	virtual uBOOL Execute(CDB4DBaseContext* inBaseContext, void* &ioLanguageContext, RawRecordsVector& onWhichRecords, VErrorDB4D& outErr) = 0;
	// ioLanguageContext is NULL the first time, it is computed from inBaseContext and returned by Execute, then it is cached by DB4D and reused each time
	// returns 2 for NULL, else it is a boolean

	virtual VErrorDB4D GetDescription(XBOX::VString& outText) = 0;
};



/*!
	@class	CDB4DQuery
	@abstract	Interface for manipulating a DB4D non sql query.
	@discussion
*/

class CDB4DQueryPathNode;

typedef std::vector<RecIDType> ComplexSelRow;

typedef std::pair<CDB4DTable*, sLONG> QueryTarget;

typedef std::vector<QueryTarget> QueryTargetVector;

typedef std::vector<RecIDType> ComplexSelColumn;
typedef std::vector<ComplexSelColumn*> DB4D_ComplexSel;


const sWORD DB4D_QueryDestination_Selection = 1;
const sWORD DB4D_QueryDestination_Sets = 2;
const sWORD DB4D_QueryDestination_Count = 4;

typedef sWORD DB4D_QueryDestination;



class CDB4DQueryOptions : public XBOX::IRefCountable
{
	public:
		enum {Component_Type = 'dbqo'};

		virtual void SetFilter( CDB4DSelection* inFilter) = 0;
		virtual void SetLimit(sLONG8 inNewLimit) = 0;

		virtual void SetWayOfLocking(DB4D_Way_of_Locking HowToLock) = 0;
		virtual void SetWayOfLockingForFirstRecord(DB4D_Way_of_Locking HowToLock) = 0;

		virtual void SetWantsLockedSet(Boolean WantsLockedSet) = 0;

		virtual void SetWantsFirstRecord(Boolean WantsFirstRecord) = 0;

		virtual void SetDestination(DB4D_QueryDestination inDestination) = 0;  // example : DB4D_QueryDestination_Count | DB4D_QueryDestination_Selection

		virtual void DescribeQueryExecution(Boolean on) = 0;
};


class CDB4DQueryResult : public XBOX::IRefCountable
{
	public:
		enum {Component_Type = 'dbqr'};

		virtual CDB4DSelection* GetSelection() = 0;
		virtual CDB4DSet* GetSet() = 0;
		virtual sLONG8 GetCount() = 0;
		virtual CDB4DSet* GetLockedSet() = 0;
		virtual CDB4DRecord* GetFirstRecord() = 0;

		virtual void GetQueryDescription(XBOX::VString& outResult, DB4D_QueryDescriptionFormat inFormat) = 0;
		virtual void GetQueryExecution(XBOX::VString& outResult, DB4D_QueryDescriptionFormat inFormat) = 0;

};



class CDB4DQuery : public XBOX::IRefCountable
{
public:
	enum {Component_Type = 'dbqu'};

	virtual Boolean AddCriteria( DB4D_TableID inTableID, DB4D_FieldID inFieldID, DB4DComparator inComparator, 
																const XBOX::VValueSingle& inValue, Boolean inDiacritic = false) = 0;
	virtual Boolean AddCriteria( const XBOX::VString& inTableName, const XBOX::VString& inFieldName, DB4DComparator inComparator, 
																const XBOX::VValueSingle& inValue, Boolean inDiacritic = false) = 0;
	
	virtual Boolean AddCriteria( CDB4DField* inField, DB4DComparator inComparator, const XBOX::VValueSingle& inValue, Boolean inDiacritic = false) = 0;

	virtual Boolean AddCriteria( CDB4DField* inField, DB4DComparator inComparator, DB4DArrayOfValues *inValue, Boolean inDiacritic = false) = 0;
	// DB4D will call release on the DB4DArrayOfValues when the query destructor is called

	virtual Boolean AddCriteria( CDB4DField* inField, DB4DComparator inComparator, CDB4DField* inFieldToCompareTo, Boolean inDiacritic = false) = 0;

	virtual Boolean AddEmCriteria(const XBOX::VString& inAttributePath, DB4DComparator inComparator, const XBOX::VValueSingle& inValue, Boolean inDiacritic = false) = 0;

	virtual Boolean AddExpression(DB4DLanguageExpression* inExpression, DB4D_TableID inTableID = 0) = 0;

	virtual Boolean AddLogicalOperator( DB4DConjunction inConjunction) = 0;

	virtual Boolean AddNotOperator() = 0;
	virtual Boolean OpenParenthesis() = 0;
	virtual Boolean CloseParenthesis() = 0;

	virtual void SetDisplayProperty(Boolean inDisplay) = 0;
	
	virtual CDB4DTable* GetTargetTable() = 0;

	virtual VErrorDB4D BuildFromString(const XBOX::VString& inQueryText, XBOX::VString& outOrderby, CDB4DBaseContext* context, CDB4DEntityModel* inModel = NULL, const QueryParamElementVector* params = NULL) = 0;

	virtual CDB4DQueryPathNode* BuildQueryPath(Boolean inSubSelection, const CDB4DQueryPathModifiers* inModifiers, CDB4DBaseContextPtr inContext) = 0;
	
	virtual VErrorDB4D GetParams(std::vector<XBOX::VString>& outParamNames, QueryParamElementVector& outParamValues) = 0;
	
	virtual VErrorDB4D SetParams(const std::vector<XBOX::VString>& inParamNames, const QueryParamElementVector& inParamValues) = 0;

};


class CDB4DComplexSelection : public XBOX::IRefCountable
{
	public:
		enum {Component_Type = 'dbcs'};

		virtual RecIDType GetRecID(sLONG Column, RecIDType Row, VErrorDB4D& outErr) = 0;
		virtual VErrorDB4D GetFullRow(RecIDType Row, ComplexSelRow& outRow) = 0;

		virtual CDB4DComplexSelection* And(CDB4DComplexSelection* inOther, VErrorDB4D& outErr) = 0;
		virtual CDB4DComplexSelection* Or(CDB4DComplexSelection* inOther, VErrorDB4D& outErr) = 0;
		virtual CDB4DComplexSelection* Minus(CDB4DComplexSelection* inOther, VErrorDB4D& outErr) = 0;
		virtual VErrorDB4D SortByRecIDs(const std::vector<sLONG>& onWhichColums) = 0;

		virtual sLONG CountRows() = 0;

		virtual sLONG CountColumns() = 0;

		virtual VErrorDB4D AddRow(const ComplexSelRow& inRow) = 0;

		virtual sLONG PosOfTarget(const QueryTarget& inTarget) = 0;

		virtual VErrorDB4D RetainColumns(QueryTargetVector& outColumns) = 0;

		virtual VErrorDB4D RetainColumn(sLONG Column, QueryTarget& outTarget) = 0;

		virtual VErrorDB4D ToDB4D_ComplexSel(DB4D_ComplexSel& outSel) = 0;
};



class CDB4DComplexQuery : public XBOX::IRefCountable
{
	public:
		enum {Component_Type = 'dbcq'};

		virtual void SetSimpleTarget(CDB4DTable* inTarget) = 0;

		virtual VErrorDB4D SetComplexTarget(const QueryTargetVector& inTargets) = 0;


		virtual VErrorDB4D AddCriteria( CDB4DField* inField, sLONG Instance, DB4DComparator inComparator, const XBOX::VValueSingle& inValue, Boolean inDiacritic = false) = 0;

		virtual VErrorDB4D AddCriteria( CDB4DField* inField, sLONG Instance, DB4DComparator inComparator, DB4DArrayOfValues *inValue, Boolean inDiacritic = false) = 0;
		// DB4D will call release on the DB4DArrayOfValues when the query destructor is called

		virtual VErrorDB4D AddJoin( CDB4DField* inField1, sLONG Instance1, DB4DComparator inComparator, CDB4DField* inField2, sLONG Instance2, 
										Boolean inDiacritic = false, bool inLeftJoin = false, bool inRightJoin = false) = 0;

		virtual VErrorDB4D AddExpression(CDB4DTable* inTable, sLONG Instance, DB4DLanguageExpression* inExpression) = 0;

		virtual VErrorDB4D AddSQLExpression(CDB4DTable* inTable, sLONG Instance, DB4DSQLExpression* inExpression) = 0;

		virtual VErrorDB4D AddSQLExpression(const QueryTargetVector& inTargets, DB4DSQLExpression* inExpression) = 0;
		
		virtual VErrorDB4D AddLogicalOperator( DB4DConjunction inConjunction) = 0;

		virtual VErrorDB4D AddNotOperator() = 0;
		virtual VErrorDB4D OpenParenthesis() = 0;
		virtual VErrorDB4D CloseParenthesis() = 0;

		virtual VErrorDB4D BuildFromString(const XBOX::VString& inQueryText) = 0;
		virtual VErrorDB4D BuildTargetsFromString(const XBOX::VString& inTargetsText) = 0;

		virtual VErrorDB4D BuildQueryDescriptor(XBOX::VString& outDescription) = 0;

};



/*!
	@class	CDB4DSortingCriterias
	@abstract	Interface for manipulating a DB4D Sorting Order.
	@discussion
*/

class CDB4DSortingCriterias : public XBOX::IRefCountable
{
public:
	enum {Component_Type = 'dbst'};

	virtual Boolean AddCriteria(DB4D_FieldID inFieldID, Boolean inAscending = true) = 0;
	virtual Boolean AddCriteria(CDB4DField* inField, Boolean inAscending = true) = 0;
	virtual Boolean AddExpression(DB4DLanguageExpression* inExpression, Boolean inAscending = true) = 0;
};



/*!
	@class	CDB4DSqlQuery
	@abstract	Interface for manipulating a DB4D sql query.
	@discussion
*/

class CDB4DSqlQuery : public XBOX::IRefCountable
{
public:
	enum {Component_Type = 'dbsq'};

	virtual VErrorDB4D ExecSql(CDB4DSelection* &outSelection, VDB4DProgressIndicator* InProgress = NULL) = 0;

};


/*!
	@class	CDB4DRecord
	@abstract	Interface for manipulating a DB4D record.
	@discussion
*/

class CDB4DRecord : public XBOX::IRefCountable
{
public:
	enum {Component_Type = 'dbrc'};
	
	virtual RecIDType GetID() const = 0;
	
	virtual Boolean IsNew() const = 0;
	virtual Boolean IsProtected() const = 0;
	virtual Boolean IsModified() const = 0;
	virtual Boolean IsFieldModified( DB4D_FieldID inFieldID) const = 0;
	virtual Boolean IsFieldModified( const CDB4DField* inField) const = 0;

	virtual uLONG GetFieldModificationStamp( DB4D_FieldID inFieldID) const = 0;
	virtual uLONG GetFieldModificationStamp( const CDB4DField* inField) const = 0;

	virtual void Touch(DB4D_FieldID inFieldID) = 0;
	virtual void Touch(const CDB4DField* inField) = 0;

	// tries to coerce. returns false if failed (property not found or coercion not available)
	virtual Boolean GetString( DB4D_FieldID inFieldID, XBOX::VString& outValue, Boolean OldOne = false, VErrorDB4D *outErr = nil) const = 0;
	virtual Boolean GetBoolean( DB4D_FieldID inFieldID, Boolean *outValue, Boolean OldOne = false, VErrorDB4D *outErr = nil) const = 0;
	virtual Boolean GetLong( DB4D_FieldID inFieldID, sLONG *outValue, Boolean OldOne = false, VErrorDB4D *outErr = nil) const = 0;
	virtual Boolean GetLong8( DB4D_FieldID inFieldID, sLONG8 *outValue, Boolean OldOne = false, VErrorDB4D *outErr = nil) const = 0;
	virtual Boolean GetReal( DB4D_FieldID inFieldID, Real *outValue, Boolean OldOne = false, VErrorDB4D *outErr = nil) const = 0;
	virtual Boolean GetBlob( DB4D_FieldID inFieldID, XBOX::VBlob **outBlob, Boolean OldOne = false, VErrorDB4D *outErr = nil) const = 0;
	virtual Boolean GetVUUID( DB4D_FieldID inFieldID, XBOX::VUUID& outValue, Boolean OldOne = false, VErrorDB4D *outErr = nil) const = 0;
	virtual Boolean GetDuration( DB4D_FieldID inFieldID, XBOX::VDuration& outValue, Boolean OldOne = false, VErrorDB4D *outErr = nil) const = 0;
	virtual Boolean GetTime( DB4D_FieldID inFieldID, XBOX::VTime& outValue, Boolean OldOne = false, VErrorDB4D *outErr = nil) const = 0;
	virtual Boolean GetPicture( DB4D_FieldID inFieldID, XBOX::VValueSingle **outPicture, Boolean OldOne = false, VErrorDB4D *outErr = nil) const = 0;

	virtual Boolean GetString( const CDB4DField* inField, XBOX::VString& outValue, Boolean OldOne = false, VErrorDB4D *outErr = nil) const = 0;
	virtual Boolean GetBoolean( const CDB4DField* inField, Boolean *outValue, Boolean OldOne = false, VErrorDB4D *outErr = nil) const = 0;
	virtual Boolean GetLong( const CDB4DField* inField, sLONG *outValue, Boolean OldOne = false, VErrorDB4D *outErr = nil) const = 0;
	virtual Boolean GetLong8( const CDB4DField* inField, sLONG8 *outValue, Boolean OldOne = false, VErrorDB4D *outErr = nil) const = 0;
	virtual Boolean GetReal( const CDB4DField* inField, Real *outValue, Boolean OldOne = false, VErrorDB4D *outErr = nil) const = 0;
	virtual Boolean GetBlob( const CDB4DField* inField, XBOX::VBlob **outBlob, Boolean OldOne = false, VErrorDB4D *outErr = nil) const = 0;
	virtual Boolean GetVUUID( const CDB4DField* inField, XBOX::VUUID& outValue, Boolean OldOne = false, VErrorDB4D *outErr = nil) const = 0;
	virtual Boolean GetDuration( const CDB4DField* inField, XBOX::VDuration& outValue, Boolean OldOne = false, VErrorDB4D *outErr = nil) const = 0;
	virtual Boolean GetTime( const CDB4DField* inField, XBOX::VTime& outValue, Boolean OldOne = false, VErrorDB4D *outErr = nil) const = 0;
	virtual Boolean GetPicture( const CDB4DField* inField, XBOX::VValueSingle **outPicture, Boolean OldOne = false, VErrorDB4D *outErr = nil) const = 0;

	virtual Boolean SetString( DB4D_FieldID inFieldID, const XBOX::VString& inValue, VErrorDB4D *outErr = nil) = 0;
	virtual Boolean SetBoolean( DB4D_FieldID inFieldID, Boolean inValue, VErrorDB4D *outErr = nil) = 0;
	virtual Boolean SetLong( DB4D_FieldID inFieldID, sLONG inValue, VErrorDB4D *outErr = nil) = 0;
	virtual Boolean SetLong8( DB4D_FieldID inFieldID, sLONG8 inValue, VErrorDB4D *outErr = nil) = 0;
	virtual Boolean SetReal( DB4D_FieldID inFieldID, Real inValue, VErrorDB4D *outErr = nil) = 0;
	virtual Boolean SetBlob( DB4D_FieldID inFieldID, const void *inData, sLONG inDataSize, VErrorDB4D *outErr = nil) = 0;
	virtual Boolean SetVUUID( DB4D_FieldID inFieldID, const XBOX::VUUID& inValue, VErrorDB4D *outErr = nil) = 0;
	virtual Boolean SetDuration( DB4D_FieldID inFieldID, const XBOX::VDuration& inValue, VErrorDB4D *outErr = nil) = 0;
	virtual Boolean SetTime( DB4D_FieldID inFieldID, const XBOX::VTime& inValue, VErrorDB4D *outErr = nil) = 0;
	virtual Boolean SetPicture( DB4D_FieldID inFieldID, const XBOX::VValueSingle& inPict, VErrorDB4D *outErr = nil) = 0;

	virtual Boolean SetString( const CDB4DField* inField, const XBOX::VString& inValue, VErrorDB4D *outErr = nil) = 0;
	virtual Boolean SetBoolean( const CDB4DField* inField, Boolean inValue, VErrorDB4D *outErr = nil) = 0;
	virtual Boolean SetLong( const CDB4DField* inField, sLONG inValue, VErrorDB4D *outErr = nil) = 0;
	virtual Boolean SetLong8( const CDB4DField* inField, sLONG8 inValue, VErrorDB4D *outErr = nil) = 0;
	virtual Boolean SetReal( const CDB4DField* inField, Real inValue, VErrorDB4D *outErr = nil) = 0;
	virtual Boolean SetBlob( const CDB4DField* inField, const void *inData, sLONG inDataSize, VErrorDB4D *outErr = nil) = 0;
	virtual Boolean SetVUUID( const CDB4DField* inField, const XBOX::VUUID& inValue, VErrorDB4D *outErr = nil) = 0;
	virtual Boolean SetDuration( const CDB4DField* inField, const XBOX::VDuration& inValue, VErrorDB4D *outErr = nil) = 0;
	virtual Boolean SetTime( const CDB4DField* inField, const XBOX::VTime& inValue, VErrorDB4D *outErr = nil) = 0;
	virtual Boolean SetPicture( const CDB4DField* inField,  const XBOX::VValueSingle& inPict, VErrorDB4D *outErr = nil) = 0;

	virtual void SetFieldToNull(const CDB4DField* inField, VErrorDB4D *outErr = nil) = 0;
	virtual void SetFieldToNull(DB4D_FieldID inFieldID, VErrorDB4D *outErr = nil) = 0;

	/*!
		@function	GetFieldValue
		@abstract	Returns the VValue of a field.
		@discussion
			Returns a VValueSingle owned by DB4D. Don't dispose it.
			May return NULL
		@param	inFieldID The ID of the field you want.
		@param	inForModif Pass true if you intend to modify the returned value.
	*/
	virtual XBOX::VValueSingle *GetFieldValue( DB4D_FieldID inFieldID, Boolean OldOne = false, VErrorDB4D *outErr = nil, bool forQueryOrSort = false) = 0;
	virtual XBOX::VValueSingle *GetFieldValue( const CDB4DField* inField, Boolean OldOne = false, VErrorDB4D *outErr = nil, bool forQueryOrSort = false) = 0;

	virtual Boolean GetFieldIntoBlob( const CDB4DField* inField,  XBOX::VBlob& outBlob, VErrorDB4D *outErr = nil, Boolean CanCacheData = true) = 0;
	virtual Boolean GetFieldIntoBlob( DB4D_FieldID inFieldID,  XBOX::VBlob& outBlob, VErrorDB4D *outErr = nil, Boolean CanCacheData = true) = 0;

	virtual Boolean Save(VErrorDB4D *outErr = nil) = 0;

	virtual void GetTimeStamp(XBOX::VTime& outValue) = 0;
	
	virtual FicheInMem4DPtr GetFicheInMem(void) = 0;

	virtual Boolean Drop(VErrorDB4D *outErr = nil) = 0;

	virtual CDB4DRecord* Clone(VErrorDB4D* outErr = nil) const = 0;

	virtual VErrorDB4D DetachRecord(Boolean BlobFieldsCanBeEmptied) = 0;  // transforms it internally as a new record but with its current data

	virtual void WhoLockedIt(DB4D_KindOfLock& outLockType, const XBOX::VValueBag **outLockingContextRetainedExtraData) const = 0;

	virtual sLONG8 GetSequenceNumber() = 0;
	
	// move the sequence number from this record to destination record.
	// if this record previously had a sequence number, it is unvalidated first.
	virtual	void	TransferSequenceNumber( CDB4DRecord *inDestination) = 0;

	virtual VErrorDB4D FillAllFieldsEmpty() = 0;

	virtual DB4D_TableID GetTableRef() const = 0;

	virtual CDB4DTable* RetainTable() const = 0;

	virtual VErrorDB4D ReserveRecordNumber() = 0;

	virtual CDB4DRecord* CloneOnlyModifiedValues(VErrorDB4D& err) const = 0;

	virtual VErrorDB4D RevertModifiedValues(CDB4DRecord* From) = 0;

	virtual CDB4DRecord* CloneForPush(VErrorDB4D* outErr = nil) = 0;
	virtual void RestoreFromPop() = 0;

	virtual VErrorDB4D ToClient(XBOX::VStream* into, CDB4DBaseContext* inContext) = 0;
	virtual VErrorDB4D ToServer(XBOX::VStream* into, CDB4DBaseContext* inContext) = 0;

	virtual uLONG GetModificationStamp() const = 0;

	virtual uLONG8 GetSyncInfoStamp() const = 0;

}; 



class CDB4DImpExp : public XBOX::IRefCountable
{
	public:
		enum {Component_Type = 'dbio'};
	
		virtual CDB4DTable* GetTarget(void) = 0;
		
		virtual Boolean AddCol(DB4D_FieldID inField) = 0;
		virtual sLONG CountCol(void) const = 0;
		virtual DB4D_FieldID GetCol(sLONG n) const = 0;
		
		virtual void SetPath(const XBOX::VFilePath& newpath) = 0;
		virtual void SetPath(const XBOX::VString& newpath) = 0;
		
		virtual void GetPath(XBOX::VFilePath& curpath) const = 0;
		virtual void GetPath(XBOX::VString& curpath) const = 0;
		
		virtual void SetColDelimit(const XBOX::VString& newColDelimit) = 0;
		virtual void GetColDelimit(XBOX::VString& curColDelimit) const = 0;
		
		virtual void SetRowDelimit(const XBOX::VString& newRowDelimit) = 0;
		virtual void GetRowDelimit(XBOX::VString& curRowDelimit) const = 0;
		
		virtual VErrorDB4D RunImport(CDB4DSelection* &outSel, VDB4DProgressIndicator* InProgress = NULL, CDB4DBaseContextPtr InContext = NULL) = 0;
		virtual VErrorDB4D RunExport(CDB4DSelection* inSel, VDB4DProgressIndicator* InProgress = NULL, CDB4DBaseContextPtr InContext = NULL) = 0;
		
		virtual void SetCharSet(XBOX::CharSet newset) = 0;
		virtual XBOX::CharSet GetCharSet() = 0;

};



class CDB4DCheckAndRepairAgent : public XBOX::IRefCountable
{
	public:
		enum {Component_Type = 'dbck'};
	
		virtual VErrorDB4D Run(XBOX::VStream* outMsg, ListOfErrors& OutList, CDB4DBaseContext* inContext, VDB4DProgressIndicator* InProgress = NULL) = 0;

		virtual void SetCheckTableState(Boolean OnOff) = 0;
		virtual void SetCheckAllTablesState(Boolean OnOff) = 0;

		virtual void SetCheckIndexState(Boolean OnOff) = 0;
		virtual void SetCheckAllIndexesState(Boolean OnOff) = 0;

		virtual void SetCheckBlobsState(Boolean OnOff) = 0;

		virtual CDB4DBase* GetOwner() const = 0;
};




class CDB4DRelation : public XBOX::IRefCountable
{
	public:
		enum {Component_Type = 'dbrl'};
	

		virtual VErrorDB4D ActivateManyToOneS(CDB4DRecord *InRec, CDB4DSelection* &OutResult, CDB4DBaseContextPtr inContext, 
											Boolean OldOne = false, Boolean LockedSel = false) = 0;  // special pour utiliser le wildchar en saisie dans l'activation d'un lien

		virtual VErrorDB4D ActivateManyToOne(CDB4DRecord *InRec, CDB4DRecord* &OutResult, CDB4DBaseContextPtr inContext, 
											Boolean OldOne = false, Boolean NoCache = false, Boolean ReadOnly = false) = 0;

		virtual VErrorDB4D ActivateOneToMany(CDB4DRecord *InRec, CDB4DSelection* &OutResult, CDB4DBaseContextPtr inContext, 
											 Boolean OldOne = false, Boolean NoCache = false, Boolean LockedSel = false) = 0;



		virtual VErrorDB4D ActivateManyToOneS(CDB4DRecord *InRec, CDB4DQueryOptions& inOptions, CDB4DQueryResult* &OutResult, CDB4DBaseContextPtr inContext, 
												Boolean OldOne = false) = 0; // special pour utiliser le wildchar en saisie dans l'activation d'un lien

		virtual VErrorDB4D ActivateManyToOne(CDB4DRecord *InRec, CDB4DQueryOptions& inOptions, CDB4DQueryResult* &OutResult, CDB4DBaseContextPtr inContext, 
												Boolean OldOne = false, Boolean NoCache = false) = 0;

		virtual VErrorDB4D ActivateOneToMany(CDB4DRecord *InRec, CDB4DQueryOptions& inOptions, CDB4DQueryResult* &OutResult, CDB4DBaseContextPtr inContext, 
												Boolean OldOne = false, Boolean NoCache = false) = 0;



		virtual VErrorDB4D Drop(CDB4DBaseContextPtr inContext = NULL) = 0;
		virtual void GetNameNto1(XBOX::VString& outName, CDB4DBaseContextPtr inContext = NULL) const = 0;
		virtual void GetName1toN(XBOX::VString& outName, CDB4DBaseContextPtr inContext = NULL) const = 0;
		virtual void RetainSource(CDB4DField* &outSourceField, CDB4DBaseContextPtr inContext = NULL) const = 0;
		virtual void RetainDestination(CDB4DField* &outDestinationField, CDB4DBaseContextPtr inContext = NULL) const = 0;
		// for relation which are linked with several source fields
		virtual VErrorDB4D RetainSources(CDB4DFieldArray& outSourceFields, CDB4DBaseContextPtr inContext = NULL) const = 0;
		// for relation which are linked with several destination fields		
		virtual VErrorDB4D RetainDestinations(CDB4DFieldArray& outDestinationFields, CDB4DBaseContextPtr inContext = NULL) const = 0;
		//virtual DB4D_RelationType GetType(CDB4DBaseContextPtr inContext = NULL) const = 0;
		virtual void GetUUID(XBOX::VUUID& outID, CDB4DBaseContextPtr inContext = NULL) const = 0;
		virtual VErrorDB4D SetNameNto1(const XBOX::VString& inName, CDB4DBaseContextPtr inContext = NULL) = 0;
		virtual VErrorDB4D SetName1toN(const XBOX::VString& inName, CDB4DBaseContextPtr inContext = NULL) = 0;
		virtual VErrorDB4D SetAutoLoadNto1(Boolean inState, CDB4DBaseContextPtr inContext = NULL) = 0;
		virtual Boolean IsAutoLoadNto1(CDB4DBaseContextPtr inContext = NULL) const = 0;
		virtual VErrorDB4D SetAutoLoad1toN(Boolean inState, CDB4DBaseContextPtr inContext = NULL) = 0;
		virtual Boolean IsAutoLoad1toN(CDB4DBaseContextPtr inContext = NULL) const = 0;
		virtual VErrorDB4D SetState(DB4D_RelationState InState, CDB4DBaseContextPtr inContext = NULL) = 0;
		virtual DB4D_RelationState GetState(CDB4DBaseContextPtr inContext = NULL) const = 0;
		virtual XBOX::VValueBag *CreateDefinition(CDB4DBaseContextPtr inContext = NULL) const = 0;
		virtual VErrorDB4D SetReferentialIntegrity(Boolean inReferentialIntegrity, Boolean inAutoDeleteRelatedRecords, CDB4DBaseContextPtr inContext = NULL) = 0;
		virtual Boolean isReferentialIntegrity(CDB4DBaseContextPtr inContext = NULL) = 0;
		virtual Boolean isAutoDeleteRelatedRecords(CDB4DBaseContextPtr inContext = NULL) = 0;

		virtual VErrorDB4D SetForeignKey(Boolean on, CDB4DBaseContextPtr inContext = NULL) = 0;
		virtual Boolean isForeignKey(CDB4DBaseContextPtr inContext = NULL) const = 0;

		virtual const XBOX::VValueBag* RetainExtraProperties(VErrorDB4D &err, CDB4DBaseContextPtr inContext = NULL) = 0;
		virtual VErrorDB4D SetExtraProperties(XBOX::VValueBag* inExtraProperties, CDB4DBaseContextPtr inContext = NULL) = 0;

		virtual sLONG GetStamp(CDB4DBaseContextPtr inContext = NULL) const = 0;
		virtual sLONG GetPosInList(CDB4DBaseContextPtr inContext = NULL) const = 0;

};


typedef enum { DB4D_ColumnFormulae_none = 0, DB4D_Sum = 1, DB4D_Average, DB4D_Min, DB4D_Max, DB4D_Count, DB4D_Count_distinct, DB4D_Average_distinct, DB4D_Sum_distinct} DB4D_ColumnFormulae;


class CDB4DColumnFormula : public XBOX::IRefCountable
{
	public:
		enum {Component_Type = 'dbfo'};

		virtual VErrorDB4D Add(CDB4DField* inColumn, DB4D_ColumnFormulae inFormula) = 0;
		virtual VErrorDB4D Execute(CDB4DSelection* inSel, CDB4DBaseContext* inContext, VDB4DProgressIndicator* InProgress = NULL, CDB4DRecord* inCurrentRecord = NULL) = 0;
		virtual sLONG8 GetResultAsLong8(sLONG inColumnNumber) const = 0;
		virtual Real GetResultAsReal(sLONG inColumnNumber) const = 0;
		virtual void GetResultAsFloat(sLONG inColumnNumber, XBOX::VFloat& outResult) const = 0;
		virtual void GetResultAsDuration(sLONG inColumnNumber, XBOX::VDuration& outResult) const = 0;

		virtual XBOX::VValueSingle* GetResult(sLONG inColumnNumber) const = 0;

};


class CDB4DIndexKey;

class CDB4DIndex : public XBOX::IRefCountable
{
	public:
		enum {Component_Type = 'dbin'};

		virtual sLONG GetSourceType(CDB4DBaseContextPtr inContext = NULL) = 0; // for example, index on 1 field, or on multiple fields, or on fulltext

		virtual sLONG GetStorageType(CDB4DBaseContextPtr inContext = NULL) = 0; // btree, hastable, etc...

		virtual Boolean IsTypeAuto(CDB4DBaseContextPtr inContext = NULL) = 0;

		virtual const	XBOX::VString& GetName(CDB4DBaseContextPtr inContext = NULL) const = 0;
		virtual VErrorDB4D SetName(const XBOX::VString& inName, CDB4DBaseContextPtr inContext = NULL) = 0;
		virtual void GetUUID(XBOX::VUUID& outID, CDB4DBaseContextPtr inContext = NULL) const = 0;

		virtual CDB4DField* RetainDataSource(CDB4DBaseContextPtr inContext = NULL) = 0; // when index is on 1 field return the CDB4DField else returns nil

		virtual VErrorDB4D RetainDataSources(CDB4DFieldArray& outSources, CDB4DBaseContextPtr inContext = NULL) = 0; // when index is on 1 or more fields returns and array of retained CDB4DField

		virtual CDB4DTable* RetainTargetReference(CDB4DBaseContextPtr inContext = NULL) = 0; // usually nil which means the same table than the field(s) data source.

		virtual Boolean MayBeSorted(CDB4DBaseContextPtr inContext = NULL) = 0;

		virtual VErrorDB4D Drop(VDB4DProgressIndicator* InProgress = NULL, XBOX::VSyncEvent* event = NULL, CDB4DBaseContextPtr inContext = NULL) = 0;

		virtual RecIDType FindKey(const XBOX::VValueSingle &inValToFind, VErrorDB4D& err, CDB4DBaseContext* inContext, XBOX::VValueSingle* *outResult, CDB4DSelectionPtr Filter, const XBOX::VCompareOptions& inOptions) = 0;
		virtual RecIDType FindKey(const ListOfValues &inValToFind, VErrorDB4D& err, CDB4DBaseContext* inContext, ListOfValues* *outResult, CDB4DSelectionPtr Filter, const XBOX::VCompareOptions& inOptions) = 0;

		virtual CDB4DIndexKey* FindIndexKey(const XBOX::VValueSingle &inValToFind, VErrorDB4D& err, CDB4DBaseContext* inContext, Boolean inBeginOfText = true) = 0;
		virtual CDB4DIndexKey* FindIndexKey(const ListOfValues &inValToFind, VErrorDB4D& err, CDB4DBaseContext* inContext, Boolean inBeginOfText = true) = 0;

		virtual CDB4DSelection* ScanIndex(RecIDType inMaxRecords, Boolean KeepSorted, Boolean Ascent, CDB4DBaseContext* inContext, VErrorDB4D& outError, CDB4DSelection* filter = nil) = 0;

		virtual CDB4DQueryResult* ScanIndex(CDB4DQueryOptions& inOptions, Boolean KeepSorted, Boolean Ascent, CDB4DBaseContext* inContext, VErrorDB4D& outError) = 0;

		virtual Boolean IsBuilding() = 0;

//		virtual void* GetSourceLang() = 0;

		virtual CDB4DBase* RetainOwner(const char* DebugInfo, CDB4DBaseContextPtr inContext = NULL) = 0;

		virtual void ReleaseFromQuery() = 0;

		virtual VErrorDB4D GetBuildError() const = 0;

		virtual Boolean IsValid(CDB4DBaseContextPtr inContext = NULL) = 0;

		virtual void FreeAndRelease() = 0;

};


class CDB4DIndexKey : public XBOX::IRefCountable
{
	public:
		enum {Component_Type = 'dbik'};

		virtual RecIDType GetRecordID() const = 0;

		virtual CDB4DRecord* GetRecord(VErrorDB4D& err) = 0;

		virtual CDB4DIndexKey* NextKey(VErrorDB4D& err) const = 0;

	
};



class CDB4DContext : public XBOX::IRefCountable
{
public:
	enum {Component_Type = 'dbcx'};

	virtual CDB4DBaseContextPtr RetainDataBaseContext(CDB4DBase* inTarget, Boolean ForceCreate = true, bool reallyRetain = true) = 0;
	//virtual void SetDataBaseContext(CDB4DBase* inTarget, CDB4DBaseContextPtr inContext) = 0;

	virtual XBOX::VUUID& GetID() = 0;
	virtual const XBOX::VUUID& GetID() const = 0;

	virtual void SendlastRemoteInfo() = 0;
	
	virtual	void				SetLanguageContext( VDBLanguageContext *inLanguageContext) = 0;
	virtual	VDBLanguageContext*	GetLanguageContext() const = 0;

	virtual void					SetJSContext(XBOX::VJSGlobalContext* inJSContext) = 0;
	virtual XBOX::VJSGlobalContext*	GetJSContext() const = 0;

	virtual void FreeAllJSFuncs() = 0;

	virtual void SetExtraData( const XBOX::VValueBag* inExtra) = 0;
	virtual const XBOX::VValueBag* RetainExtraData() const = 0;

	virtual void SetCurrentUser(const XBOX::VUUID& inUserID, CUAGSession* inSession) = 0;

	virtual void CleanUpForReuse() = 0;


};


class StDBContext
{
	public:

		inline StDBContext(CDB4DBase* inBase)
		{
			fContext = inBase->NewContext(nil, kJSContextCreator);
		}

		inline ~StDBContext()
		{
			fContext->Release();
		}

		inline CDB4DBaseContext* operator()()
		{
			return fContext;
		}

		inline CDB4DBaseContext* operator*()
		{
			return fContext;
		}

	protected:
		CDB4DBaseContext* fContext;
};


typedef uLONG8 DB4D_AutoSeqToken;

class CDB4DAutoSeqNumber : public XBOX::IRefCountable
{
public:
	enum {Component_Type = 'dbas'};

	virtual const XBOX::VUUID& GetID() const  = 0;

	virtual void SetCurrentValue(sLONG8 currentvalue) = 0;
	virtual sLONG8 GetCurrentValue() const = 0;

	virtual sLONG8 GetNewValue(DB4D_AutoSeqToken& ioToken) = 0; // ioToken must be 0 if you want a new token out
	virtual void ValidateValue(DB4D_AutoSeqToken inToken, CDB4DTable* inTable, CDB4DBaseContext* context) = 0;
	virtual void InvalidateValue(DB4D_AutoSeqToken inToken) = 0;

	virtual VErrorDB4D Drop(CDB4DBaseContext* inContext = NULL) = 0;
};



typedef std::vector<sLONG> SubRecordIDVector;

class DB4D_API VSubTable : public XBOX::VLong8
{
public:
	virtual CDB4DRecord* RetainNthSubRecord(sLONG inIndex, VErrorDB4D& err, CDB4DBaseContextPtr inContext, Boolean inSubSelection) = 0;
	virtual sLONG CountSubRecords(CDB4DBaseContextPtr inContext, Boolean inSubSelection) = 0;
	virtual CDB4DRecord* AddSubRecord(VErrorDB4D& err, CDB4DBaseContextPtr inContext, Boolean inSubSelection) = 0;
	virtual VErrorDB4D DeleteNthRecord(sLONG inIndex, CDB4DBaseContextPtr inContext, Boolean inSubSelection) = 0;
	virtual VErrorDB4D QuerySubRecords(const CDB4DQuery* inQuery, CDB4DBaseContextPtr inContext) = 0;
	virtual VErrorDB4D SortSubRecords(const CDB4DSortingCriterias* inCriterias, CDB4DBaseContextPtr inContext) = 0;
	virtual VErrorDB4D AllSubRecords(CDB4DBaseContextPtr inContext) = 0;
	virtual VErrorDB4D SetSubSel(CDB4DBaseContextPtr inContext, const SubRecordIDVector& inRecords) = 0;
	virtual sLONG GetSubRecordIDFromSubSel(CDB4DBaseContextPtr inContext, sLONG PosInSel) = 0;

};


class VSubTableKey : public XBOX::VLong8
{
public:
};



/* *************************************************************************************** */


class CDB4DQueryPathModifiers : public XBOX::IRefCountable
{
	public:
		enum {Component_Type = 'dbqm'};

		virtual void SetLineDelimiter(const XBOX::VString& inDelimiter) = 0;
		virtual void SetTabDelimiter(const XBOX::VString& inDelimiter) = 0;
		virtual void SetTabSize(sLONG inTabSize) = 0;
		virtual void SetHTMLOutPut(Boolean inState) = 0;
		virtual void SetVerbose(Boolean inState) = 0;
		virtual void SetSimpleTree(Boolean inState, Boolean inWithTempVariables) = 0;

		virtual const XBOX::VString GetLineDelimiter() const = 0;
		virtual const XBOX::VString GetTabDelimiter() const = 0;
		virtual sLONG GetTabSize() const = 0;
		virtual Boolean isHTML() const = 0;
		virtual Boolean isVerbose() const = 0;
		virtual Boolean isSimpleTree() const = 0;
		virtual Boolean isTempVarDisplay() const  = 0;

};


class CDB4DQueryPathNode : public XBOX::IRefCountable
{
	public:
		enum {Component_Type = 'dbqq'};

		virtual CDB4DQueryPathNode* GetParent() const = 0;
		virtual VErrorDB4D BuildFullString(XBOX::VString& outResult, sLONG inStartingLevel) = 0;
		virtual const XBOX::VString& GetThisString() const = 0;
		virtual CDB4DQueryPathNode* GetNthChild(sLONG Nth, Boolean inBefore) const = 0;
		virtual sLONG GetCountChildren(Boolean inBefore) const = 0;
		virtual VErrorDB4D AddChild(CDB4DQueryPathNode* inChild, Boolean inBefore) = 0;
		virtual void SetString(const XBOX::VString& inString) = 0;

};

class DB4D_API CDB4DJournalData : public XBOX::IRefCountable
{
	public:
		virtual uLONG	GetActionType() const = 0;
		virtual uLONG8	GetTimeStamp() const = 0;
		virtual sLONG8	GetGlobalOperationNumber() const = 0;

		// this functions return true only if there is a consistant value to return
		// ( ex. GetDataLen() is not usable for DB4D_Log_OpenData ... )
		virtual Boolean	GetContextID( sLONG8 &outContextID ) const = 0;
		virtual Boolean GetDataLen( sLONG &outDataLen ) const = 0;
		virtual Boolean GetRecordNumber( sLONG &outRecordNumber ) const = 0;
		virtual Boolean GetBlobNumber( sLONG &outBlobNumber, XBOX::VString& outPath ) const = 0;
		virtual Boolean GetSequenceNumber( sLONG8 &outSequenceNumber ) const = 0;
		//virtual Boolean	GetTableIndex( sLONG &outIndex ) const = 0;
		virtual Boolean GetTableID( XBOX::VUUID& outID ) const = 0;
		virtual Boolean GetUserID(XBOX::VUUID& outID) const = 0;
		virtual const XBOX::VValueBag* GetExtraData() const = 0;
		virtual Boolean GetCountFields( sLONG &outCountFields ) const = 0;
		virtual XBOX::VValueSingle* GetNthFieldValue(sLONG inFieldIndex, sLONG* outType, void* df) = 0;
		virtual void* GetDataPtr() = 0;
		virtual Boolean NeedSwap() = 0;
		virtual sLONG GetNthFieldBlobRef(sLONG inFieldIndex, XBOX::VString& outPath) = 0;

};


class CDB4DJournalParser : public XBOX::IRefCountable
{
	public:
		enum {Component_Type = 'dbjp'};

		virtual	XBOX::VError	Init(XBOX::VFile* inJournalFile,uLONG8 &outTotalOperationCount, VDB4DProgressIndicator *inProgressIndicator ) = 0;
		virtual	XBOX::VError	DeInit() = 0;

		virtual	XBOX::VError	SetCurrentOperation( uLONG8 inOperation, CDB4DJournalData **outJournalData ) = 0;
		virtual	XBOX::VError	NextOperation( uLONG8 &outOperation, CDB4DJournalData **outJournalData ) = 0;
		virtual uLONG8			CountOperations() = 0;
		virtual	XBOX::VError	SetEndOfJournal( uLONG8 inOperation ) = 0;
		virtual bool			IsValid( const XBOX::VUUID &inDataLink ) = 0;
};



class CachedRelatedRecord
{
public:
	CachedRelatedRecord( sLONG inTableNum, CDB4DRecord *inRecord, CDB4DSelection *inSelection)
		: fTableNum( inTableNum), fRecord( inRecord), fSelection( inSelection)
	{
	}

	inline CachedRelatedRecord()
	{
		fTableNum = 0;
		fRecord = nil;
		fSelection = nil;
	}

	inline void Steal(CachedRelatedRecord& other)
	{
		fTableNum = other.fTableNum;
		fRecord = other.fRecord;
		fSelection = other.fSelection;

		other.fRecord = nil;
		other.fSelection = nil;
	}

	inline void AdoptRecord(CDB4DRecord* inRec)
	{
		fRecord = inRec;
	}

	inline void AdoptSelection(CDB4DSelection* inSel)
	{
		fSelection = inSel;
	}

	inline void SetRecord(CDB4DRecord* inRec)
	{
		fRecord = inRec;
		if (fRecord != nil)
			fRecord->Retain();
	}

	inline void SetSelection(CDB4DSelection* inSel)
	{
		fSelection = inSel;
		if (fSelection != nil)
			fSelection->Retain();
	}

	inline void SetTableNum(sLONG inTableNum)
	{
		fTableNum = inTableNum;
	}

	inline ~CachedRelatedRecord()
	{
		if (fRecord != NULL)
			fRecord->Release();
		if (fSelection != NULL)
			fSelection->Release();
	}

	CDB4DSelection*	GetSelection()			{ return fSelection;}

	CDB4DRecord*	GetRecord()				{ return fRecord;}

	sLONG			GetTableNum() const		{ return fTableNum;}

private:
	sLONG fTableNum;
	CDB4DRecord* fRecord;
	CDB4DSelection* fSelection;
};


#if 0
typedef std::pair<CDB4DRecord*, CDB4DSelection*> CachedRelatedRecSel;

typedef std::pair<sLONG, CachedRelatedRecSel> CachedRelatedRecord;
#endif


class CDB4DRemoteRecordCache : public XBOX::IRefCountable
{
public:
	enum {Component_Type = 'dbRc'};

	virtual VErrorDB4D RetainCacheRecord(RecIDType inRecIndex, CDB4DRecord* &outRecord, std::vector<CachedRelatedRecord>& outRelatedRecords) = 0;  
				// inRecIndex is the position in the selection that was used to create the CDB4DRemoteRecordCache
};


struct DB4D_ToolsOptions
{
	uBOOL KeepAddrInfos;
	uBOOL CheckOverLapWithBittable;
};

class CDB4DRawDataBase : public XBOX::IRefCountable
{
	public:
		enum {Component_Type = 'dbzz'};

		virtual VErrorDB4D CheckAll(IDB4D_DataToolsIntf* inDataToolLog, DB4D_ToolsOptions* inOption = NULL) = 0;

		virtual sLONG CountTables(Boolean& outInfoIsValid,IDB4D_DataToolsIntf* inDataToolLog) = 0;  // needs a CheckStruct_Elems before use
		virtual sLONG CountIndexes(Boolean& outInfoIsValid,IDB4D_DataToolsIntf* inDataToolLog) = 0;  // needs a CheckStruct_Elems before use
		virtual sLONG GetTables(std::vector<sLONG>& outTables,Boolean& outInfoIsValid,IDB4D_DataToolsIntf* inDataToolLog) = 0;  // needs a CheckStruct_Elems before use
		virtual void GetTableName(sLONG inTableNum, XBOX::VString& outName, Boolean& outInfoIsValid, IDB4D_DataToolsIntf* inDataToolLog) = 0;  // needs a CheckStruct_Elems before use
		virtual VErrorDB4D GetTableIndexes(sLONG inTableNum, std::vector<sLONG>& outIndexes, Boolean& outInfoIsValid, IDB4D_DataToolsIntf* inDataToolLog) = 0;  // needs a CheckStruct_Elems before use
		virtual RecIDType CountRecordsInTable(sLONG inTableNum, Boolean& outInfoIsValid, IDB4D_DataToolsIntf* inDataToolLog) = 0;  // needs a CheckStruct_Elems before use
		virtual sLONG CountFieldsInTable(sLONG inTableNum, Boolean& outInfoIsValid, IDB4D_DataToolsIntf* inDataToolLog) = 0;  // needs a CheckStruct_Elems before use
		virtual VErrorDB4D GetTableFields(sLONG inTableNum, std::vector<sLONG>& outFields, Boolean& outInfoIsValid, IDB4D_DataToolsIntf* inDataToolLog) = 0;  // needs a CheckStruct_Elems before use

		virtual VErrorDB4D GetFieldInfo(sLONG inTableNum, sLONG inFieldNum, XBOX::VString& outName, sLONG& outType, std::vector<sLONG>& outIndexes, Boolean& outInfoIsValid, IDB4D_DataToolsIntf* inDataToolLog) = 0;
		virtual VErrorDB4D GetIndexInfo(sLONG inIndexNum,XBOX::VString& outName,sLONG& outTableNum,std::vector<sLONG>& outFields, Boolean& outInfoIsValid, IDB4D_DataToolsIntf* inDataToolLog) = 0;

		virtual bool CheckStructAndDataUUIDMatch(IDB4D_DataToolsIntf* inDataToolLog) = 0;

		virtual VErrorDB4D CheckStruct_DataSegs(IDB4D_DataToolsIntf* inDataToolLog) = 0;
		virtual VErrorDB4D CheckStruct_Elems(IDB4D_DataToolsIntf* inDataToolLog) = 0; // needs to be performs if you want to scan individual tables of indexes
		virtual VErrorDB4D CheckStruct_RecordsAndIndexes(IDB4D_DataToolsIntf* inDataToolLog) = 0;
		virtual VErrorDB4D CheckStruct_All(IDB4D_DataToolsIntf* inDataToolLog, DB4D_ToolsOptions* inOption = NULL) = 0; // regroups all other CheckStruct methods

		virtual VErrorDB4D CheckData_DataSegs(IDB4D_DataToolsIntf* inDataToolLog) = 0;
		virtual VErrorDB4D CheckData_Elems(IDB4D_DataToolsIntf* inDataToolLog) = 0;
		virtual VErrorDB4D CheckData_OneTable(sLONG inTableNum, IDB4D_DataToolsIntf* inDataToolLog) = 0;
		virtual VErrorDB4D CheckData_OneIndex(sLONG inIndexNum, IDB4D_DataToolsIntf* inDataToolLog) = 0;
		virtual VErrorDB4D CheckData_All(IDB4D_DataToolsIntf* inDataToolLog, DB4D_ToolsOptions* inOption = NULL) = 0; // regroups all other CheckData methods

		virtual VErrorDB4D CompactInto(CDB4DBase* outCompactedBase, IDB4D_DataToolsIntf* inDataToolLog, Boolean withIndexes = true, 
										Boolean WithStruct = true, Boolean WithData = true, Boolean EraseOrphanTables = false) = 0;

		//virtual VErrorDB4D RecoverByTags(CDB4DBase* outRecoveredBase, IDB4D_DataToolsIntf* inDataToolLog, Boolean WithStruct = true, Boolean WithData = true) = 0;

};


typedef enum
{
	eattr_none = 0,
	eattr_storage = 1,
	eattr_alias,
	eattr_computedField,
	eattr_relation_Nto1,
	eattr_relation_1toN,
	eattr_remove,
	eattr_alter,
	//eattr_field,
	eattr_composition
} EntityAttributeKind;


typedef enum
{
	emeth_none = 0,
	emeth_static = 1,
	emeth_rec,
	emeth_sel
} EntityMethodKind;


typedef enum
{
	escope_none = 0,
	escope_public,
	escope_public_server,
	escope_protected,
	escope_private
} EntityAttributeScope;


typedef enum
{
	erel_none = 0,
	erel_Nto1 = 1,
	erel_1toN
} EntityRelationKind;


typedef enum 
{ 
	eav_vvalue = 1, 
	eav_subentity, 
	eav_selOfSubentity,
	eav_composition
} EntityAttributeValueKind;

/*
class VectorOfVValue : public std::vector<const XBOX::VValueSingle*>
{
	public:
		VectorOfVValue(bool AutoDisposeValues = false)
		{
			fAutoDisposeValues = AutoDisposeValues;
		}

		virtual ~VectorOfVValue()
		{
			if (fAutoDisposeValues)
			{
				for (std::vector<const XBOX::VValueSingle*>::iterator cur = begin(), last = end(); cur != last; cur++)
				{
					delete (XBOX::VValueSingle*)*cur;
				}
			}
		}

		void SetAutoDispose(bool x)
		{
			fAutoDisposeValues = x;
		}

	protected:
		bool fAutoDisposeValues;
};
*/

class CDB4DEntityCollection;

class CDB4DEntityModel : public XBOX::IRefCountable
{
	public:
		enum {Component_Type = 'dbem'};

		//virtual CDB4DBase* RetainDataBase() const = 0;

		virtual sLONG CountAttributes() const = 0;

		//virtual CDB4DTable* RetainTable() const = 0;

		virtual void GetEntityName(XBOX::VString& outName) const = 0;

		virtual const EntityAttributeScope GetScope() const = 0;

		virtual const XBOX::VString& GetEntityName() const = 0;

		virtual CDB4DEntityAttribute* GetAttribute(sLONG pos) const = 0;
		virtual CDB4DEntityAttribute* GetAttribute(const XBOX::VString& AttributeName) const = 0;

		virtual CDB4DEntityMethod* GetMethod(const XBOX::VString& inMethodName, bool publicOnly = false) const = 0;

		//virtual CDB4DEntityRecord* LoadEntity(sLONG n, VErrorDB4D& err, DB4D_Way_of_Locking HowToLock, CDB4DBaseContext* context, bool autoexpand) = 0;

		virtual RecIDType CountEntities(CDB4DBaseContext* inContext) = 0;

		virtual CDB4DEntityCollection* NewSelection(bool ordered = false, bool safeRef = false) const = 0;

		virtual CDB4DEntityRecord* NewEntity(CDB4DBaseContextPtr inContext) const = 0;

		
		virtual CDB4DEntityCollection* SelectAllEntities(CDB4DBaseContextPtr inContext, VErrorDB4D* outErr = NULL, 
												 DB4D_Way_of_Locking HowToLock = DB4D_Do_Not_Lock, CDB4DEntityCollection* outLockSet = NULL) = 0;
				
		virtual CDB4DEntityCollection* ExecuteQuery( CDB4DQuery *inQuery, CDB4DBaseContextPtr inContext, CDB4DEntityCollection* Filter = NULL, 
											   VDB4DProgressIndicator* InProgress = NULL, DB4D_Way_of_Locking HowToLock = DB4D_Do_Not_Lock, 
											   sLONG limit = 0, CDB4DEntityCollection* outLockSet = NULL, VErrorDB4D *outErr = NULL) = 0;
	
	
		virtual bool HasPrimaryKey() const = 0;

		virtual bool HasIdentifyingAttributes() const = 0;

		virtual CDB4DEntityRecord* FindEntityWithPrimKey(const XBOX::VString& primkey, CDB4DBaseContext* inContext, VErrorDB4D& err, DB4D_Way_of_Locking HowToLock) = 0;

		virtual CDB4DEntityRecord* FindEntityWithPrimKey(const XBOX::VectorOfVString& primkey, CDB4DBaseContext* inContext, VErrorDB4D& err, DB4D_Way_of_Locking HowToLock) = 0;

		virtual CDB4DEntityRecord* FindEntityWithPrimKey(const XBOX::VValueBag& primkey, CDB4DBaseContext* inContext, VErrorDB4D& err, DB4D_Way_of_Locking HowToLock) = 0;

		virtual CDB4DEntityRecord* FindEntityWithPrimKey(const XBOX::VectorOfVValue& primkey, CDB4DBaseContext* inContext, VErrorDB4D& err, DB4D_Way_of_Locking HowToLock) = 0;

		virtual CDB4DEntityRecord* FindEntityWithIdentifyingAtts(const XBOX::VectorOfVString& idents, CDB4DBaseContext* inContext, VErrorDB4D& err, DB4D_Way_of_Locking HowToLock) = 0;

		virtual CDB4DEntityRecord* FindEntityWithIdentifyingAtts(const XBOX::VValueBag& idents, CDB4DBaseContext* inContext, VErrorDB4D& err, DB4D_Way_of_Locking HowToLock) = 0;

		virtual CDB4DEntityRecord* FindEntityWithIdentifyingAtts(const XBOX::VectorOfVValue& idents, CDB4DBaseContext* inContext, VErrorDB4D& err, DB4D_Way_of_Locking HowToLock) = 0;

		virtual CDB4DEntityCollection* Query( const XBOX::VString& inQuery, CDB4DBaseContext* inContext, VErrorDB4D& err, const XBOX::VValueSingle* param1 = nil, const XBOX::VValueSingle* param2 = nil, const XBOX::VValueSingle* param3 = nil) = 0;
		virtual CDB4DEntityRecord* Find(const XBOX::VString& inQuery, CDB4DBaseContext* inContext, VErrorDB4D& err, const XBOX::VValueSingle* param1 = nil, const XBOX::VValueSingle* param2 = nil, const XBOX::VValueSingle* param3 = nil) = 0;

		virtual VErrorDB4D SetPermission(DB4D_EM_Perm inPerm, const XBOX::VUUID& inGroupID, bool forced) = 0;
		virtual VErrorDB4D GetPermission(DB4D_EM_Perm inPerm, XBOX::VUUID& outGroupID, bool& forced) const = 0 ;

		virtual bool PermissionMatch(DB4D_EM_Perm inPerm, CUAGSession* inSession) const = 0;

		// create a bag containing description of entity model with attributes.
		// get resolved representation suitable for editors.
		virtual XBOX::VValueBag *CreateDefinition() const = 0;
};


class CDB4DEntityMethod : public XBOX::IRefCountable
{
	public:
		enum {Component_Type = 'dbef'};

		virtual CDB4DEntityModel* GetModel() const = 0;

		virtual EntityMethodKind GetMethodKind() const = 0;

		virtual XBOX::VJSObject* GetFuncObject(CDB4DBaseContext* inContext, XBOX::VJSObject& outObjFunc) const = 0;

};


class CDB4DEntityAttribute : public XBOX::IRefCountable
{
	public:
		enum {Component_Type = 'dbea'};

		//virtual CDB4DEntityModel* GetModel() const = 0;

		virtual EntityAttributeKind GetAttributeKind() const = 0;

		virtual const EntityAttributeScope GetScope() const = 0;

		//virtual sLONG GetPosInModel() const = 0;

		virtual void GetAttibuteName(XBOX::VString& outName) const = 0;

		virtual const XBOX::VString& GetAttibuteName() const = 0;

		virtual CDB4DEntityModel* GetRelatedEntityModel() const = 0;

		virtual XBOX::ValueKind GetDataKind() const = 0;


};


class CDB4DEntityAttributeValue : public XBOX::IRefCountable
{
	public:
		enum {Component_Type = 'dbev'};

		virtual EntityAttributeValueKind GetAttributeKind() const = 0;

		virtual XBOX::VValueSingle* GetVValue() const = 0;

		virtual CDB4DEntityRecord* GetRelatedEntity() const = 0;

		virtual CDB4DEntityModel* GetRelatedEntityModel() const = 0;

		virtual CDB4DEntityCollection* GetRelatedSelection() const = 0;

		//virtual void SetExtraData(void* ExtraData) = 0;

		//virtual void* GetExtraData() const = 0;

		virtual XBOX::VError GetJsonString(XBOX::VString& outJsonValue) = 0;

		virtual XBOX::VError GetJSObject(XBOX::VJSObject& outJSObject) = 0;

};





class CDB4DEntityCollection : public XBOX::IRefCountable
{
public:
	enum {Component_Type = 'dbec'};

	virtual CDB4DEntityRecord* LoadEntityRecord(RecIDType posInCol, CDB4DBaseContext* context, VErrorDB4D& outError, DB4D_Way_of_Locking HowToLock = DB4D_Do_Not_Lock) = 0;
	virtual RecIDType GetLength(CDB4DBaseContext* context) = 0;
	virtual VErrorDB4D DeleteEntities(CDB4DBaseContext* context, VDB4DProgressIndicator* InProgress = nil, CDB4DEntityCollection* *outLocked = nil) = 0;

};




class CDB4DEntityRecord : public XBOX::IRefCountable
{
	public:
		enum {Component_Type = 'dber'};

		//virtual CDB4DEntityModel* GetModel() const = 0;

		//virtual CDB4DRecord* GetRecord() const = 0;

		virtual CDB4DEntityAttributeValue* GetAttributeValue(const XBOX::VString& inAttributeName, VErrorDB4D& err) = 0;

	//	virtual CDB4DEntityAttributeValue* GetAttributeValue(sLONG pos, VErrorDB4D& err) = 0;

		virtual CDB4DEntityAttributeValue* GetAttributeValue(const CDB4DEntityAttribute* inAttribute, VErrorDB4D& err) = 0;

		virtual VErrorDB4D SetAttributeValue(const CDB4DEntityAttribute* inAttribute, const XBOX::VValueSingle* inValue) = 0;
		virtual VErrorDB4D SetAttributeValue(const XBOX::VString& inAttributeName, const XBOX::VValueSingle* inValue) = 0;

		virtual VErrorDB4D SetAttributeValue(const CDB4DEntityAttribute* inAttribute, CDB4DEntityRecord* inRelatedEntity) = 0;
		virtual VErrorDB4D SetAttributeValue(const XBOX::VString& inAttributeName, CDB4DEntityRecord* inRelatedEntity) = 0;

		virtual VErrorDB4D SetAttributeValue(const CDB4DEntityAttribute* inAttribute, const XBOX::VectorOfVValue* inValues) = 0;
		virtual VErrorDB4D SetAttributeValue(const XBOX::VString& inAttributeName, const XBOX::VectorOfVValue* inValues) = 0;

		virtual VErrorDB4D SetAttributeValue(const CDB4DEntityAttribute* inAttribute, const XBOX::VString& inJsonValue) = 0;
		virtual VErrorDB4D SetAttributeValue(const XBOX::VString& inAttributeName, const XBOX::VString& inJsonValue) = 0;

		virtual VErrorDB4D SetAttributeValue(const CDB4DEntityAttribute* inAttribute, const XBOX::VJSObject& inJSObject) = 0;
		virtual VErrorDB4D SetAttributeValue(const XBOX::VString& inAttributeName, const XBOX::VJSObject& inJSObject) = 0;

		virtual VErrorDB4D TouchAttributeValue(const CDB4DEntityAttribute* inAttribute) = 0;
		virtual VErrorDB4D ResetAttributeValue(const CDB4DEntityAttribute* inAttribute) = 0;
		virtual VErrorDB4D ResetAttributeValue(const XBOX::VString& inAttributeName) = 0;

		// virtual sLONG GetNum() const = 0;

		virtual sLONG GetModificationStamp() const = 0;
	
		virtual VErrorDB4D Validate() = 0;

		virtual VErrorDB4D Save(uLONG stamp, bool allowOverrideStamp = false) = 0;

		virtual Boolean IsNew() const = 0;
		virtual Boolean IsProtected() const = 0;
		virtual Boolean IsModified() const = 0;

		virtual void GetTimeStamp(XBOX::VTime& outValue) = 0;
				
		virtual VErrorDB4D Drop() = 0;

		//virtual VErrorDB4D SetIdentifyingAtts(const XBOX::VectorOfVValue& idents) = 0;

		//virtual VErrorDB4D SetPrimKey(const XBOX::VectorOfVValue& primkey) = 0;

		virtual void GetPrimKeyValue(XBOX::VectorOfVValue& outPrimkey) = 0;

		virtual CDB4DBaseContext* GetContext() = 0;

		//virtual void ReleaseExtraDatas() = 0;

		//virtual VErrorDB4D ConvertToJSObject(XBOX::VJSObject& outObj, const XBOX::VString& inAttributeList, bool withKey, bool allowEmptyAttList) = 0;

};

/* *************************************************************************************** */

// deplace depuis VDB4DLang
const VErrorDB4D	VE_DB4D_NOT_FOUND	= MAKE_VERROR(CDB4DManager::Component_Type, 2);


const VErrorDB4D	VE_DB4D_IMPORT_NOCOL	= MAKE_VERROR(CDB4DManager::Component_Type, 1001);
const VErrorDB4D	VE_DB4D_IMPORT_WRONGTARGET	= MAKE_VERROR(CDB4DManager::Component_Type, 1002);
const VErrorDB4D	VE_DB4D_EXPORT_NOCOL	= MAKE_VERROR(CDB4DManager::Component_Type, 1003);
const VErrorDB4D	VE_DB4D_EXPORT_WRONGTARGET	= MAKE_VERROR(CDB4DManager::Component_Type, 1004);
const VErrorDB4D	VE_DB4D_IMPORT_DOCDOESNOTEXISTS	= MAKE_VERROR(CDB4DManager::Component_Type, 1005);

//const VErrorDB4D	VE_DB4D_CANNOTSAVESTRUCT	= MAKE_VERROR(CDB4DManager::Component_Type, 1006); // unused now
const VErrorDB4D	VE_DB4D_CANNOTCREATEDATAFILE	= MAKE_VERROR(CDB4DManager::Component_Type, 1007);

const VErrorDB4D	VE_DB4D_WRONGDATASEG	= MAKE_VERROR(CDB4DManager::Component_Type, 1008);

const VErrorDB4D	VE_DB4D_MEMFULL	= MAKE_VERROR(CDB4DManager::Component_Type, 1009); // sera surement remplacee par l'erreur systeme

const VErrorDB4D	VE_DB4D_CANNOTLOADTABLEDEF	= MAKE_VERROR(CDB4DManager::Component_Type, 1010); 
const VErrorDB4D	VE_DB4D_CANNOTOPENDATAFILE	= MAKE_VERROR(CDB4DManager::Component_Type, 1011); 

const VErrorDB4D	VE_DB4D_SEGFULL	= MAKE_VERROR(CDB4DManager::Component_Type, 1012); 

const VErrorDB4D	VE_DB4D_CANNOTSAVEDATASEG = MAKE_VERROR(CDB4DManager::Component_Type, 1013); 
const VErrorDB4D	VE_DB4D_CANNOTREADDATASEG = MAKE_VERROR(CDB4DManager::Component_Type, 1014); 

const VErrorDB4D	VE_DB4D_WRONGBASEHEADER = MAKE_VERROR(CDB4DManager::Component_Type, 1015); 
const VErrorDB4D	VE_DB4D_CANNOTCREATETABLE	= MAKE_VERROR(CDB4DManager::Component_Type, 1016);

const VErrorDB4D	VE_DB4D_CANNOTREADINDEXTABLE	= MAKE_VERROR(CDB4DManager::Component_Type, 1017);
const VErrorDB4D	VE_DB4D_CANNOTWRITEINDEXTABLE	= MAKE_VERROR(CDB4DManager::Component_Type, 1018);

const VErrorDB4D	VE_DB4D_WRONGTABLEREF	= MAKE_VERROR(CDB4DManager::Component_Type, 1019);
const VErrorDB4D	VE_DB4D_WRONGFIELDREF	= MAKE_VERROR(CDB4DManager::Component_Type, 1020);
const VErrorDB4D	VE_DB4D_INVALIDINDEXTYPE	= MAKE_VERROR(CDB4DManager::Component_Type, 1021);

const VErrorDB4D	VE_DB4D_INVALIDFIELDNAME	= MAKE_VERROR(CDB4DManager::Component_Type, 1022);
const VErrorDB4D	VE_DB4D_INVALIDBASENAME	= MAKE_VERROR(CDB4DManager::Component_Type, 1023);

const VErrorDB4D	VE_DB4D_CANNOTOPENSTRUCT	= MAKE_VERROR(CDB4DManager::Component_Type, 1024); 
const VErrorDB4D	VE_DB4D_CANNOTCREATESTRUCT	= MAKE_VERROR(CDB4DManager::Component_Type, 1025); 

const VErrorDB4D	VE_DB4D_CANNOTLOADBITSEL	= MAKE_VERROR(CDB4DManager::Component_Type, 1026); 
const VErrorDB4D	VE_DB4D_CANNOTLOADSET	= MAKE_VERROR(CDB4DManager::Component_Type, 1027); 
const VErrorDB4D	VE_DB4D_CANNOTMODIFYSET	= MAKE_VERROR(CDB4DManager::Component_Type, 1028); 
const VErrorDB4D	VE_DB4D_CANNOTSAVESET	= MAKE_VERROR(CDB4DManager::Component_Type, 1029); 


const VErrorDB4D	VE_DB4D_CANNOTSAVEBLOB	= MAKE_VERROR(CDB4DManager::Component_Type, 1030); 
const VErrorDB4D	VE_DB4D_CANNOTLOADBLOB	= MAKE_VERROR(CDB4DManager::Component_Type, 1031); 
const VErrorDB4D	VE_DB4D_CANNOTALLOCATEBLOB	= MAKE_VERROR(CDB4DManager::Component_Type, 1032); 

const VErrorDB4D	VE_DB4D_CANNOTLOADFREEBIT	= MAKE_VERROR(CDB4DManager::Component_Type, 1033); 
const VErrorDB4D	VE_DB4D_CANNOTSAVEFREEBIT	= MAKE_VERROR(CDB4DManager::Component_Type, 1034); 
const VErrorDB4D	VE_DB4D_CANNOTLOADFREEBITTABLE	= MAKE_VERROR(CDB4DManager::Component_Type, 1035); 
const VErrorDB4D	VE_DB4D_CANNOTSAVEFREEBITTABLE	= MAKE_VERROR(CDB4DManager::Component_Type, 1036); 

const VErrorDB4D	VE_DB4D_CANNOTCLOSESEG	= MAKE_VERROR(CDB4DManager::Component_Type, 1037); 
const VErrorDB4D	VE_DB4D_CANNOTDELETESEG	= MAKE_VERROR(CDB4DManager::Component_Type, 1038); 
const VErrorDB4D	VE_DB4D_CANNOTOPENSEG	= MAKE_VERROR(CDB4DManager::Component_Type, 1039); 
const VErrorDB4D	VE_DB4D_CANNOTCREATESEG	= MAKE_VERROR(CDB4DManager::Component_Type, 1040); 

const VErrorDB4D	VE_DB4D_CANNOTALLOCATESPACEINDATASEG	= MAKE_VERROR(CDB4DManager::Component_Type, 1041); 
const VErrorDB4D	VE_DB4D_CANNOTFREESPACEINDATASEG	= MAKE_VERROR(CDB4DManager::Component_Type, 1042);
const VErrorDB4D	VE_DB4D_FILEISWRITEPROTECTED	= MAKE_VERROR(CDB4DManager::Component_Type, 1043);

const VErrorDB4D	VE_DB4D_CANNOTACCESSFIELD	= MAKE_VERROR(CDB4DManager::Component_Type, 1044);
const VErrorDB4D	VE_DB4D_FIELDDEFCODEMISSING	= MAKE_VERROR(CDB4DManager::Component_Type, 1045);

const VErrorDB4D	VE_DB4D_CANNOTSAVERECORD	= MAKE_VERROR(CDB4DManager::Component_Type, 1046); 
const VErrorDB4D	VE_DB4D_CANNOTLOADRECORD	= MAKE_VERROR(CDB4DManager::Component_Type, 1047); 
const VErrorDB4D	VE_DB4D_CANNOTALLOCATERECORD	= MAKE_VERROR(CDB4DManager::Component_Type, 1048); 
const VErrorDB4D	VE_DB4D_CANNOTUPDATERECORD	= MAKE_VERROR(CDB4DManager::Component_Type, 1049); 
const VErrorDB4D	VE_DB4D_WRONGRECORDHEADER	= MAKE_VERROR(CDB4DManager::Component_Type, 1050); 


const VErrorDB4D	VE_DB4D_CANNOTSAVETABLEDEF	= MAKE_VERROR(CDB4DManager::Component_Type, 1051); 
const VErrorDB4D	VE_DB4D_CANNOTUPDATETABLEDEF	= MAKE_VERROR(CDB4DManager::Component_Type, 1052); 
const VErrorDB4D	VE_DB4D_FIELDNAMEDUPLICATE	= MAKE_VERROR(CDB4DManager::Component_Type, 1053);
//const VErrorDB4D	VE_DB4D_CANNOTLOADTABLEDEF	= MAKE_VERROR(CDB4DManager::Component_Type, 1054); 

const VErrorDB4D	VE_DB4D_CANNOTUPDATEINDEX	= MAKE_VERROR(CDB4DManager::Component_Type, 1055); 
const VErrorDB4D	VE_DB4D_CANNOTUPDATEBLOBS	= MAKE_VERROR(CDB4DManager::Component_Type, 1056); 
const VErrorDB4D	VE_DB4D_CANNOTDELETEBLOBS	= MAKE_VERROR(CDB4DManager::Component_Type, 1057); 


const VErrorDB4D	VE_DB4D_CANNOTADDFIELD	= MAKE_VERROR(CDB4DManager::Component_Type, 1058); 
const VErrorDB4D	VE_DB4D_CANNOTALLOCATETABLEINMEM	= MAKE_VERROR(CDB4DManager::Component_Type, 1059); 

const VErrorDB4D	VE_DB4D_CANNOTALLOCATERECORDINMEM	= MAKE_VERROR(CDB4DManager::Component_Type, 1061); 

const VErrorDB4D	VE_DB4D_COULDNOTCOMPLETELYDELETETABLE	= MAKE_VERROR(CDB4DManager::Component_Type, 1062); 
const VErrorDB4D	VE_DB4D_CANNOTLOCKRECORD	= MAKE_VERROR(CDB4DManager::Component_Type, 1063); 
const VErrorDB4D	VE_DB4D_CANNOTUNLOCKRECORD	= MAKE_VERROR(CDB4DManager::Component_Type, 1064); 
const VErrorDB4D	VE_DB4D_CANNOTDELETERECORD	= MAKE_VERROR(CDB4DManager::Component_Type, 1065); 
const VErrorDB4D	VE_DB4D_RECORDISLOCKED	= MAKE_VERROR(CDB4DManager::Component_Type, 1066); 

const VErrorDB4D	VE_DB4D_SEARCHCOULDNOTCOMPLETE	= MAKE_VERROR(CDB4DManager::Component_Type, 1067); 

const VErrorDB4D	VE_DB4D_CANNOTSAVETABLEHEADER	= MAKE_VERROR(CDB4DManager::Component_Type, 1068); 

const VErrorDB4D	VE_DB4D_CANNOTIMPORTDATA	= MAKE_VERROR(CDB4DManager::Component_Type, 1069); 

const VErrorDB4D	VE_DB4D_CANNOTLOADINDEXHEADER	= MAKE_VERROR(CDB4DManager::Component_Type, 1070); 
const VErrorDB4D	VE_DB4D_CANNOTSAVEINDEXHEADER	= MAKE_VERROR(CDB4DManager::Component_Type, 1071); 

const VErrorDB4D	VE_DB4D_CANNOTGETINDEXPAGEADDR	= MAKE_VERROR(CDB4DManager::Component_Type, 1072); 
const VErrorDB4D	VE_DB4D_CANNOTSETINDEXPAGEADDR	= MAKE_VERROR(CDB4DManager::Component_Type, 1073); 

const VErrorDB4D	VE_DB4D_CANNOTDROPINDEXONDISK	= MAKE_VERROR(CDB4DManager::Component_Type, 1074); 
const VErrorDB4D	VE_DB4D_CANNOTSORTONINDEX	= MAKE_VERROR(CDB4DManager::Component_Type, 1075); 

const VErrorDB4D	VE_DB4D_CANNOTLOADINDEXPAGE	= MAKE_VERROR(CDB4DManager::Component_Type, 1076); 
const VErrorDB4D	VE_DB4D_CANNOTSAVEINDEXPAGE	= MAKE_VERROR(CDB4DManager::Component_Type, 1077); 

const VErrorDB4D	VE_DB4D_CANNOTINSERTKEYINTOINDEX	= MAKE_VERROR(CDB4DManager::Component_Type, 1078); 
const VErrorDB4D	VE_DB4D_CANNOTDELETEKEYFROMINDEX	= MAKE_VERROR(CDB4DManager::Component_Type, 1079); 

const VErrorDB4D	VE_DB4D_COULDNOTCOMPLETELYDELETEINDEX	= MAKE_VERROR(CDB4DManager::Component_Type, 1080); 
const VErrorDB4D	VE_DB4D_CANNOTCOMPLETESCANOFINDEX	= MAKE_VERROR(CDB4DManager::Component_Type, 1081); 
const VErrorDB4D	VE_DB4D_CANNOTCOMPLETESORTOFINDEX	= MAKE_VERROR(CDB4DManager::Component_Type, 1082); 

const VErrorDB4D	VE_DB4D_CANNOTINSERTKEYINTOPAGE	= MAKE_VERROR(CDB4DManager::Component_Type, 1083); 
const VErrorDB4D	VE_DB4D_CANNOTDELETEKEYFROMPAGE	= MAKE_VERROR(CDB4DManager::Component_Type, 1084); 

const VErrorDB4D	VE_DB4D_CANNOTLOADCLUSTER	= MAKE_VERROR(CDB4DManager::Component_Type, 1085); 
const VErrorDB4D	VE_DB4D_CANNOTADDTOCLUSTER	= MAKE_VERROR(CDB4DManager::Component_Type, 1086); 
const VErrorDB4D	VE_DB4D_CANNOTDELFROMCLUSTER	= MAKE_VERROR(CDB4DManager::Component_Type, 1087); 

const VErrorDB4D	VE_DB4D_INVALIDINDEX	= MAKE_VERROR(CDB4DManager::Component_Type, 1088);

const VErrorDB4D	VE_DB4D_SQLSYNTAXERROR	= MAKE_VERROR(CDB4DManager::Component_Type, 1089);
const VErrorDB4D	VE_DB4D_SQLTOKENNOTFOUND	= MAKE_VERROR(CDB4DManager::Component_Type, 1090);

const VErrorDB4D	VE_DB4D_NOTIMPLEMENTED	= MAKE_VERROR(CDB4DManager::Component_Type, 1091); 
const VErrorDB4D	VE_DB4D_CANNOTREGISTERCODE	= MAKE_VERROR(CDB4DManager::Component_Type, 1092); 
const VErrorDB4D	VE_DB4D_ACTIONCANCELEDBYUSER	= MAKE_VERROR(CDB4DManager::Component_Type, 1093); 
const VErrorDB4D	VE_DB4D_TRANSACTIONCONFLICT	= MAKE_VERROR(CDB4DManager::Component_Type, 1094); 

const VErrorDB4D	VE_DB4D_INVALIDTABLENAME	= MAKE_VERROR(CDB4DManager::Component_Type, 1095);

const VErrorDB4D	VE_DB4D_TABLEISLOCKED	= MAKE_VERROR(CDB4DManager::Component_Type, 1096);
const VErrorDB4D	VE_DB4D_DATABASEISLOCKED	= MAKE_VERROR(CDB4DManager::Component_Type, 1097);

const VErrorDB4D	VE_DB4D_ADDRESSISINVALID	= MAKE_VERROR(CDB4DManager::Component_Type, 1098);

const VErrorDB4D	VE_DB4D_RECORDISEMPTY	= MAKE_VERROR(CDB4DManager::Component_Type, 1099);

const VErrorDB4D	VE_DB4D_WRONGSOURCEFIELD	= MAKE_VERROR(CDB4DManager::Component_Type, 1100);
const VErrorDB4D	VE_DB4D_WRONGDESTINATIONFIELD	= MAKE_VERROR(CDB4DManager::Component_Type, 1101);
const VErrorDB4D	VE_DB4D_INVALIDRELATIONNAME	= MAKE_VERROR(CDB4DManager::Component_Type, 1102);
const VErrorDB4D	VE_DB4D_FIELDTYPENOTMATCHING	= MAKE_VERROR(CDB4DManager::Component_Type, 1103);
const VErrorDB4D	VE_DB4D_RELATIONISEMPTY	= MAKE_VERROR(CDB4DManager::Component_Type, 1104);
const VErrorDB4D	VE_DB4D_CANNOTLOADRELATIONS	= MAKE_VERROR(CDB4DManager::Component_Type, 1105);
const VErrorDB4D	VE_DB4D_CANNOTSAVERELATIONS	= MAKE_VERROR(CDB4DManager::Component_Type, 1106);

const VErrorDB4D	VE_DB4D_SEARCHNOTCOMPLETE_LOCKED = MAKE_VERROR(CDB4DManager::Component_Type, 1107);
const VErrorDB4D  VE_DB4D_INVALIDRECORD = MAKE_VERROR(CDB4DManager::Component_Type, 1108);
const VErrorDB4D  VE_DB4D_WRONGRECORDID = MAKE_VERROR(CDB4DManager::Component_Type, 1109);

const VErrorDB4D	VE_DB4D_RELATION_ALREADY_EXISTS	= MAKE_VERROR(CDB4DManager::Component_Type, 1110);
const VErrorDB4D	VE_DB4D_INDEX_ALREADY_EXISTS	= MAKE_VERROR(CDB4DManager::Component_Type, 1111);
const VErrorDB4D	VE_DB4D_WRONG_COMP_OPERATOR	= MAKE_VERROR(CDB4DManager::Component_Type, 1112);
const VErrorDB4D	VE_DB4D_END_OF_BUFFER	= MAKE_VERROR(CDB4DManager::Component_Type, 1113);

const VErrorDB4D	VE_DB4D_WRONG_VERSION_NUMBER	= MAKE_VERROR(CDB4DManager::Component_Type, 1114);
const VErrorDB4D	VE_DB4D_DUPLICATED_KEY	= MAKE_VERROR(CDB4DManager::Component_Type, 1115);

const VErrorDB4D	VE_DB4D_Not_NullFIELD_IS_NULL	= MAKE_VERROR(CDB4DManager::Component_Type, 1116);
const VErrorDB4D	VE_DB4D_CANNOT_SET_FIELD_TO_Not_Null	= MAKE_VERROR(CDB4DManager::Component_Type, 1117);
const VErrorDB4D	VE_DB4D_CANNOT_GET_EXCLUSIVEACCESS_ON_TABLE	= MAKE_VERROR(CDB4DManager::Component_Type, 1118);
const VErrorDB4D	VE_DB4D_CANNOT_CHECK_REFERENCIAL_INTEGRITY	= MAKE_VERROR(CDB4DManager::Component_Type, 1119);
const VErrorDB4D	VE_DB4D_SOME_RELATED_RECORDS_STILL_EXIST	= MAKE_VERROR(CDB4DManager::Component_Type, 1120);

const VErrorDB4D	VE_DB4D_CANNOT_DELETE_ALL_RECORDS_IN_SEL	= MAKE_VERROR(CDB4DManager::Component_Type, 1121);

const VErrorDB4D	VE_DB4D_BLOB_IS_NULL	= MAKE_VERROR(CDB4DManager::Component_Type, 1122);

const VErrorDB4D	VE_DB4D_WRONG_CONTEXT	= MAKE_VERROR(CDB4DManager::Component_Type, 1123);

const VErrorDB4D	VE_DB4D_WRONG_RELATIONREF	= MAKE_VERROR(CDB4DManager::Component_Type, 1124);

const VErrorDB4D	VE_DB4D_INVALIDRECORDNAME	= MAKE_VERROR(CDB4DManager::Component_Type, 1125);

const VErrorDB4D	VE_DB4D_WRONGTYPE	= MAKE_VERROR(CDB4DManager::Component_Type, 1126);

const VErrorDB4D	VE_DB4D_CANNOT_LOAD_EXTRAPROPERTIES	= MAKE_VERROR(CDB4DManager::Component_Type, 1127);
const VErrorDB4D	VE_DB4D_CANNOT_WRITE_EXTRAPROPERTIES	= MAKE_VERROR(CDB4DManager::Component_Type, 1128);

const VErrorDB4D	VE_DB4D_SUBRECORD_ID_OUT_OF_RANGE	= MAKE_VERROR(CDB4DManager::Component_Type, 1129);

const VErrorDB4D	VE_DB4D_INDEXNAMEDUPLICATE	= MAKE_VERROR(CDB4DManager::Component_Type, 1130);

const VErrorDB4D	VE_DB4D_WRONGINDEXNAME	= MAKE_VERROR(CDB4DManager::Component_Type, 1131);

const VErrorDB4D	VE_DB4D_WRONGKEYVALUE	= MAKE_VERROR(CDB4DManager::Component_Type, 1132);

const VErrorDB4D	VE_DB4D_WRONGINDEXTYP	= MAKE_VERROR(CDB4DManager::Component_Type, 1133);

const VErrorDB4D	VE_DB4D_INVALID_ACCESSOR	= MAKE_VERROR(CDB4DManager::Component_Type, 1134);
const VErrorDB4D	VE_DB4D_ACCESSOR_IS_READONLY	= MAKE_VERROR(CDB4DManager::Component_Type, 1135);
const VErrorDB4D	VE_DB4D_NULL_NOTACCEPTED	= MAKE_VERROR(CDB4DManager::Component_Type, 1136);
const VErrorDB4D	VE_DB4D_THIS_IS_NULL	= MAKE_VERROR(CDB4DManager::Component_Type, 1137);

const VErrorDB4D	VE_DB4D_SELECTION_IS_NULL	= MAKE_VERROR(CDB4DManager::Component_Type, 1138);

const VErrorDB4D	VE_DB4D_DATABASEISWRITEPROTECTED	= MAKE_VERROR(CDB4DManager::Component_Type, 1139);
const VErrorDB4D	VE_DB4D_DATABASEISCLOSING	= MAKE_VERROR(CDB4DManager::Component_Type, 1140);

const VErrorDB4D	VE_DB4D_TRANSACTIONISINVALID	= MAKE_VERROR(CDB4DManager::Component_Type, 1141);

const VErrorDB4D	VE_DB4D_ARRAYLIMIT_IS_EXCEEDED	= MAKE_VERROR(CDB4DManager::Component_Type, 1142);

const VErrorDB4D	VE_DB4D_ARRAYOFVALUES_CREATOR_IS_MISSING	= MAKE_VERROR(CDB4DManager::Component_Type, 1143);

const VErrorDB4D	VE_DB4D_CANNOTBUILDSELECTION	= MAKE_VERROR(CDB4DManager::Component_Type, 1144);

const VErrorDB4D	VE_DB4D_BUSY_OBJECT	= MAKE_VERROR(CDB4DManager::Component_Type, 1145);

const VErrorDB4D	VE_DB4D_DATAFILE_DOES_NOT_MATCH_STRUCT	= MAKE_VERROR(CDB4DManager::Component_Type, 1146);

const VErrorDB4D	VE_DB4D_CANNOT_START_LISTENER	= MAKE_VERROR(CDB4DManager::Component_Type, 1147);
const VErrorDB4D	VE_DB4D_CANNOT_START_SERVER	= MAKE_VERROR(CDB4DManager::Component_Type, 1148);
const VErrorDB4D	VE_DB4D_NO_LISTENER	= MAKE_VERROR(CDB4DManager::Component_Type, 1149);
const VErrorDB4D	VE_DB4D_TASK_IS_DYING	= MAKE_VERROR(CDB4DManager::Component_Type, 1150);

const VErrorDB4D	VE_DB4D_WRONG_REQUEST_TAG	= MAKE_VERROR(CDB4DManager::Component_Type, 1151);
const VErrorDB4D	VE_DB4D_WRONG_CONTEXT_ID	= MAKE_VERROR(CDB4DManager::Component_Type, 1152);

const VErrorDB4D	VE_DB4D_NOT_ENOUGH_FREE_SPACE_ON_DISK	= MAKE_VERROR(CDB4DManager::Component_Type, 1153);

const VErrorDB4D	VE_DB4D_SET_IS_NULL	= MAKE_VERROR(CDB4DManager::Component_Type, 1154);

const VErrorDB4D	VE_DB4D_NO_PRIMARYKEY_MATCHING_THIS_FOREIGNKEY	= MAKE_VERROR(CDB4DManager::Component_Type, 1155);

const VErrorDB4D	VE_DB4D_FIELDTYPE_DOES_NOT_SUPPORT_UNIQUE	= MAKE_VERROR(CDB4DManager::Component_Type, 1156);
const VErrorDB4D	VE_DB4D_FIELDTYPE_DOES_NOT_SUPPORT_NEVER_NULL	= MAKE_VERROR(CDB4DManager::Component_Type, 1157);

const VErrorDB4D	VE_DB4D_CANNOT_CHANGE_PRIMARYKEY_DEFINITION	= MAKE_VERROR(CDB4DManager::Component_Type, 1158);

const VErrorDB4D	VE_DB4D_MAXRECORDS_REACHED	= MAKE_VERROR(CDB4DManager::Component_Type, 1159);

const VErrorDB4D	VE_DB4D_MAXBLOBS_REACHED	= MAKE_VERROR(CDB4DManager::Component_Type, 1160);

const VErrorDB4D	VE_DB4D_INDICE_OUT_OF_RANGE	= MAKE_VERROR(CDB4DManager::Component_Type, 1161);

const VErrorDB4D	VE_DB4D_INVALID_QUERY	= MAKE_VERROR(CDB4DManager::Component_Type, 1162);

const VErrorDB4D	VE_DB4D_RECORD_IS_NULL	= MAKE_VERROR(CDB4DManager::Component_Type, 1163);

const VErrorDB4D	VE_DB4D_OBJECT_IS_NULL	= MAKE_VERROR(CDB4DManager::Component_Type, 1164);

const VErrorDB4D	VE_DB4D_WRONG_OWNER	= MAKE_VERROR(CDB4DManager::Component_Type, 1165);

const VErrorDB4D	VE_DB4D_OBJECT_WAS_NOT_LOCKED	= MAKE_VERROR(CDB4DManager::Component_Type, 1166);

const VErrorDB4D	VE_DB4D_OBJECT_IS_LOCKED_BY_OTHER_CONTEXT	= MAKE_VERROR(CDB4DManager::Component_Type, 1167);

const VErrorDB4D	VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE	= MAKE_VERROR(CDB4DManager::Component_Type, 1168);

const VErrorDB4D	VE_DB4D_INVALID_TABLENUM	= MAKE_VERROR(CDB4DManager::Component_Type, 1169);
const VErrorDB4D	VE_DB4D_INVALID_FIELDNUM	= MAKE_VERROR(CDB4DManager::Component_Type, 1170);

const VErrorDB4D	VE_DB4D_INVALID_BASEID	= MAKE_VERROR(CDB4DManager::Component_Type, 1171);

const VErrorDB4D	VE_DB4D_INVALID_PARAMETER	= MAKE_VERROR(CDB4DManager::Component_Type, 1172);

const VErrorDB4D	VE_DB4D_FLUSH_DID_NOT_COMPLETE	= MAKE_VERROR(CDB4DManager::Component_Type, 1173);

const VErrorDB4D	VE_DB4D_FLUSH_COMPLETE_ON_STRUCT	= MAKE_VERROR(CDB4DManager::Component_Type, 1174);

const VErrorDB4D	VE_DB4D_FLUSH_COMPLETE_ON_DATA	= MAKE_VERROR(CDB4DManager::Component_Type, 1175);

const VErrorDB4D	VE_DB4D_LOGFILE_IS_INVALID	= MAKE_VERROR(CDB4DManager::Component_Type, 1176);
const VErrorDB4D	VE_DB4D_LOGFILE_NOT_FOUND	= MAKE_VERROR(CDB4DManager::Component_Type, 1177);
const VErrorDB4D	VE_DB4D_LOGFILE_LAST_OPERATION_DOES_NOT_MATCH	= MAKE_VERROR(CDB4DManager::Component_Type, 1178);
const VErrorDB4D	VE_DB4D_LOGFILE_DOES_NOT_MATCH_DATABASE	= MAKE_VERROR(CDB4DManager::Component_Type, 1179);

const VErrorDB4D	VE_DB4D_DATATABLE_HAS_BEEN_DELETED	= MAKE_VERROR(CDB4DManager::Component_Type, 1180);

const VErrorDB4D	VE_DB4D_INDEXKEYS_ARE_NOT_UNIQUE	= MAKE_VERROR(CDB4DManager::Component_Type, 1181);

const VErrorDB4D	VE_DB4D_CANNOT_CREATE_JOURNAL_FILE	= MAKE_VERROR(CDB4DManager::Component_Type, 1182);
const VErrorDB4D	VE_DB4D_CANNOT_WRITE_JOURNAL_FILE	= MAKE_VERROR(CDB4DManager::Component_Type, 1183);

const VErrorDB4D	VE_DB4D_CANNOT_DELETE_TABLE	= MAKE_VERROR(CDB4DManager::Component_Type, 1184);

const VErrorDB4D	VE_DB4D_CANNOT_OPEN_REMOTE_BASE	= MAKE_VERROR(CDB4DManager::Component_Type, 1185);

const VErrorDB4D	VE_DB4D_CANNOT_INTEGRATE_JOURNAL	= MAKE_VERROR(CDB4DManager::Component_Type, 1186);

const VErrorDB4D	VE_DB4D_CANNOT_COMPUTE_SET	= MAKE_VERROR(CDB4DManager::Component_Type, 1187);

const VErrorDB4D	VE_DB4D_CANNOT_SAVE_ARRAY	= MAKE_VERROR(CDB4DManager::Component_Type, 1188);
const VErrorDB4D	VE_DB4D_CANNOT_LOAD_ARRAY	= MAKE_VERROR(CDB4DManager::Component_Type, 1189);

const VErrorDB4D	VE_DB4D_CANNOT_LOAD_AUTOSEQ	= MAKE_VERROR(CDB4DManager::Component_Type, 1190);

const VErrorDB4D	VE_DB4D_CANNOTSELECTRECORD	= MAKE_VERROR(CDB4DManager::Component_Type, 1191);

const VErrorDB4D	VE_DB4D_CANNOT_CREATE_RECORD	= MAKE_VERROR(CDB4DManager::Component_Type, 1192);

const VErrorDB4D	VE_DB4D_CANNOT_COMPLETE_COLLECTION_TO_DATA	= MAKE_VERROR(CDB4DManager::Component_Type, 1193);
const VErrorDB4D	VE_DB4D_CANNOT_COMPLETE_DATA_TO_COLLECTION	= MAKE_VERROR(CDB4DManager::Component_Type, 1194);

const VErrorDB4D	VE_DB4D_CANNOT_COMPLETE_SEQ_SORT	= MAKE_VERROR(CDB4DManager::Component_Type, 1195);

const VErrorDB4D	VE_DB4D_CANNOT_LOCK_SEL	= MAKE_VERROR(CDB4DManager::Component_Type, 1196);

const VErrorDB4D	VE_DB4D_CANNOT_LOAD_INDEXKEY	= MAKE_VERROR(CDB4DManager::Component_Type, 1197);

const VErrorDB4D	VE_DB4D_CANNOT_SAVE_INDEXKEY	= MAKE_VERROR(CDB4DManager::Component_Type, 1198);

const VErrorDB4D	VE_DB4D_CANNOT_BUILD_INDEXKEY	= MAKE_VERROR(CDB4DManager::Component_Type, 1199);

const VErrorDB4D	VE_DB4D_CANNOT_COMPLETE_QUERY	= MAKE_VERROR(CDB4DManager::Component_Type, 1200);

const VErrorDB4D	VE_DB4D_CANNOT_ANALYZE_QUERY	= MAKE_VERROR(CDB4DManager::Component_Type, 1201);

const VErrorDB4D	VE_DB4D_CANNOT_COMPLETE_FORMULA_ON_COLUMN	= MAKE_VERROR(CDB4DManager::Component_Type, 1202);

const VErrorDB4D	VE_DB4D_CANNOT_COMPLETE_COMPLEXQUERY	= MAKE_VERROR(CDB4DManager::Component_Type, 1203);

const VErrorDB4D	VE_DB4D_CANNOT_ANALYZE_COMPLEXQUERY	= MAKE_VERROR(CDB4DManager::Component_Type, 1204);

const VErrorDB4D	VE_DB4D_CANNOT_CANNOT_COMPLETE_DISTINCT_VALUES	= MAKE_VERROR(CDB4DManager::Component_Type, 1205);

const VErrorDB4D	VE_DB4D_CANNOT_BUILD_ARRAYOFVALUES	= MAKE_VERROR(CDB4DManager::Component_Type, 1206);

const VErrorDB4D	VE_DB4D_CANNOT_LOAD_SEL	= MAKE_VERROR(CDB4DManager::Component_Type, 1207);

const VErrorDB4D	VE_DB4D_CANNOT_SEND_DATA = MAKE_VERROR(CDB4DManager::Component_Type, 1208);

const VErrorDB4D	VE_DB4D_CANNOT_RECEIVE_DATA = MAKE_VERROR(CDB4DManager::Component_Type, 1209);

const VErrorDB4D	VE_DB4D_CANNOT_SEND_REQUEST = MAKE_VERROR(CDB4DManager::Component_Type, 1210);

const VErrorDB4D	VE_DB4D_CANNOT_CREATE_CONNECTION = MAKE_VERROR(CDB4DManager::Component_Type, 1211);

const VErrorDB4D	VE_DB4D_CANNOT_BUILD_QUICK_INDEX = MAKE_VERROR(CDB4DManager::Component_Type, 1212);

const VErrorDB4D	VE_DB4D_CANNOT_BUILD_DISTINCT_KEYS = MAKE_VERROR(CDB4DManager::Component_Type, 1213);

const VErrorDB4D	VE_DB4D_CANNOT_SORT_SELECTION = MAKE_VERROR(CDB4DManager::Component_Type, 1214);

const VErrorDB4D	VE_DB4D_CANNOT_LOAD_ADDRTABLE = MAKE_VERROR(CDB4DManager::Component_Type, 1215);

const VErrorDB4D	VE_DB4D_CANNOT_MODIFY_ADDRTABLE = MAKE_VERROR(CDB4DManager::Component_Type, 1216);

const VErrorDB4D	VE_DB4D_CANNOT_ALLOCATE_NEW_ENTRY = MAKE_VERROR(CDB4DManager::Component_Type, 1217);

const VErrorDB4D	VE_DB4D_CANNOT_FREE_ENTRY = MAKE_VERROR(CDB4DManager::Component_Type, 1218);

const VErrorDB4D	VE_DB4D_CANNOT_SAVE_TRANS_RECORD = MAKE_VERROR(CDB4DManager::Component_Type, 1219);

const VErrorDB4D	VE_DB4D_CANNOT_SAVE_TRANS_BLOB = MAKE_VERROR(CDB4DManager::Component_Type, 1220);

const VErrorDB4D	VE_DB4D_CANNOT_LOAD_TRANS_RECORD = MAKE_VERROR(CDB4DManager::Component_Type, 1221);

const VErrorDB4D	VE_DB4D_CANNOT_LOAD_TRANS_BLOB = MAKE_VERROR(CDB4DManager::Component_Type, 1222);

const VErrorDB4D	VE_DB4D_CANNOT_START_TRANSACTION = MAKE_VERROR(CDB4DManager::Component_Type, 1223);

const VErrorDB4D	VE_DB4D_CANNOT_COMMIT_TRANSACTION = MAKE_VERROR(CDB4DManager::Component_Type, 1224);

const VErrorDB4D	VE_DB4D_CANNOT_GET_EXTRAPROPERTY = MAKE_VERROR(CDB4DManager::Component_Type, 1225);

const VErrorDB4D	VE_DB4D_CANNOT_SET_EXTRAPROPERTY = MAKE_VERROR(CDB4DManager::Component_Type, 1226);

const VErrorDB4D	VE_DB4D_TABLENAMEDUPLICATE = MAKE_VERROR(CDB4DManager::Component_Type, 1227);

const VErrorDB4D	VE_DB4D_CANNOT_GET_LIST_OF_NULL_KEYS = MAKE_VERROR(CDB4DManager::Component_Type, 1228);

const VErrorDB4D	VE_DB4D_CANNOT_MODIFY_LIST_OF_NULL_KEYS = MAKE_VERROR(CDB4DManager::Component_Type, 1229);

const VErrorDB4D	VE_DB4D_INVALID_INDEXKEY = MAKE_VERROR(CDB4DManager::Component_Type, 1230);

const VErrorDB4D	VE_DB4D_LOGFILE_NOT_SET	= MAKE_VERROR(CDB4DManager::Component_Type, 1231);

const VErrorDB4D	VE_DB4D_CONTEXT_IS_NULL	= MAKE_VERROR(CDB4DManager::Component_Type, 1232);

const VErrorDB4D	VE_DB4D_CANNOT_LOCK_BASE	= MAKE_VERROR(CDB4DManager::Component_Type, 1233);

const VErrorDB4D	VE_DB4D_WRONGFIELDREF_IN_RECORD	= MAKE_VERROR(CDB4DManager::Component_Type, 1234);

const VErrorDB4D	VE_DB4D_WRONGFIELDREF_IN_TABLE	= MAKE_VERROR(CDB4DManager::Component_Type, 1235);

const VErrorDB4D	VE_DB4D_CANNOT_GET_TEMP_DATA_FOR_TRANS	= MAKE_VERROR(CDB4DManager::Component_Type, 1236);

const VErrorDB4D	VE_DB4D_CARTESIAN_PRODUCT_FAILED = MAKE_VERROR(CDB4DManager::Component_Type, 1237);

const VErrorDB4D	VE_DB4D_CANNOT_MERGE_SELECTIONS	= MAKE_VERROR(CDB4DManager::Component_Type, 1238);

const VErrorDB4D	VE_DB4D_CANNOT_UPGRADE_DATABASE_FORMAT	= MAKE_VERROR(CDB4DManager::Component_Type, 1239);

const VErrorDB4D	VE_DB4D_WRONG_TAG_HEADER	= MAKE_VERROR(CDB4DManager::Component_Type, 1240);

const VErrorDB4D	VE_DB4D_WRONG_CHECKSUM	= MAKE_VERROR(CDB4DManager::Component_Type, 1241);

const VErrorDB4D	VE_DB4D_RELATIONNAMEDUPLICATE = MAKE_VERROR(CDB4DManager::Component_Type, 1242);

const VErrorDB4D	VE_DB4D_CANNOT_LOAD_DATATABLES = MAKE_VERROR(CDB4DManager::Component_Type, 1243);

const VErrorDB4D	VE_DB4D_FOREIGN_KEY_CONSTRAINT_LIST_NOT_EMPTY	= MAKE_VERROR(CDB4DManager::Component_Type, 1244);

const VErrorDB4D	VE_DB4D_ADDR_ENTRY_IS_NOT_EMPTY	= MAKE_VERROR(CDB4DManager::Component_Type, 1245);

const VErrorDB4D	VE_DB4D_CANNOT_PREALLOCATE_ADDR	= MAKE_VERROR(CDB4DManager::Component_Type, 1246);

const VErrorDB4D	VE_DB4D_CANNOTUPDATENEWRECORD	= MAKE_VERROR(CDB4DManager::Component_Type, 1247); 

const VErrorDB4D	VE_DB4D_CANNOTSAVENEWRECORD	= MAKE_VERROR(CDB4DManager::Component_Type, 1248); 

const VErrorDB4D	VE_DB4D_CANNOT_SAVE_SUBRECORD	= MAKE_VERROR(CDB4DManager::Component_Type, 1249); 

const VErrorDB4D	VE_DB4D_CANNOTSAVERECORD_NO_PARAM	= MAKE_VERROR(CDB4DManager::Component_Type, 1250);

const VErrorDB4D	VE_DB4D_CANNOT_LOCK_STRUCT_OBJECTDEF	= MAKE_VERROR(CDB4DManager::Component_Type, 1251);

const VErrorDB4D	VE_DB4D_CANNOT_UNLOCK_STRUCT_OBJECTDEF	= MAKE_VERROR(CDB4DManager::Component_Type, 1252);

const VErrorDB4D	VE_DB4D_INVALID_RELATIONNUM				= MAKE_VERROR(CDB4DManager::Component_Type, 1253);
 
const VErrorDB4D	VE_DB4D_CIRCULAR_REF_IN_REC_ADDR_TABLE		= MAKE_VERROR(CDB4DManager::Component_Type, 1254);

const VErrorDB4D	VE_DB4D_CIRCULAR_REF_IN_BLOB_ADDR_TABLE		= MAKE_VERROR(CDB4DManager::Component_Type, 1255);

const VErrorDB4D	VE_DB4D_DUPLICATED_SCHEMA_NAME		= MAKE_VERROR(CDB4DManager::Component_Type, 1256);

const VErrorDB4D	VE_DB4D_CANNOT_SAVE_SCHEMA		= MAKE_VERROR(CDB4DManager::Component_Type, 1257);

const VErrorDB4D	VE_DB4D_CANNOT_DROP_SCHEMA		= MAKE_VERROR(CDB4DManager::Component_Type, 1258);

const VErrorDB4D	VE_DB4D_CANNOT_RENAME_SCHEMA		= MAKE_VERROR(CDB4DManager::Component_Type, 1259);

const VErrorDB4D	VE_DB4D_LOGFILE_TOO_RECENT	= MAKE_VERROR(CDB4DManager::Component_Type, 1260);
const VErrorDB4D	VE_DB4D_LOGFILE_TOO_OLD	= MAKE_VERROR(CDB4DManager::Component_Type, 1261);

const VErrorDB4D	VE_DB4D_SOME_DATATABLES_HAVE_NO_MATCH	= MAKE_VERROR(CDB4DManager::Component_Type, 1262);

const VErrorDB4D	VE_DB4D_RECORD_STAMP_NOT_MATCHING	= MAKE_VERROR(CDB4DManager::Component_Type, 1263);

const VErrorDB4D	VE_DB4D_PRIMKEY_IS_NEEDED	= MAKE_VERROR(CDB4DManager::Component_Type, 1264);

const VErrorDB4D	VE_DB4D_INVALIDFIELD	= MAKE_VERROR(CDB4DManager::Component_Type, 1265);

const VErrorDB4D	VE_DB4D_UNKNOWN_FIELD_TYPE	= MAKE_VERROR(CDB4DManager::Component_Type, 1266);

const VErrorDB4D	VE_DB4D_STACK_OVERFLOW	= MAKE_VERROR(CDB4DManager::Component_Type, 1267);

const VErrorDB4D	VE_DB4D_DATATABLE_CANNOT_BE_RESURECTED	= MAKE_VERROR(CDB4DManager::Component_Type, 1268);

const VErrorDB4D	VE_DB4D_DATATABLE_CANNOT_BE_FOUND	= MAKE_VERROR(CDB4DManager::Component_Type, 1269);

const VErrorDB4D	VE_DB4D_CANNOT_REBUILD_MISSING_STRUCT	= MAKE_VERROR(CDB4DManager::Component_Type, 1270);

const VErrorDB4D	VE_DB4D_INVALID_REST_REQUEST_HANDLER	= MAKE_VERROR(CDB4DManager::Component_Type, 1271);

const VErrorDB4D	VE_DB4D_SOME_RECORDS_ARE_STILL_LOCKED	= MAKE_VERROR(CDB4DManager::Component_Type, 1272);

const VErrorDB4D	VE_DB4D_CANNOT_DROP_TABLE	= MAKE_VERROR(CDB4DManager::Component_Type, 1273);

const VErrorDB4D	VE_DB4D_CURRENT_JOURNAL_IS_INVALID	= MAKE_VERROR(CDB4DManager::Component_Type, 1274);

const VErrorDB4D	VE_DB4D_PARENTHESIS_ERROR	= MAKE_VERROR(CDB4DManager::Component_Type, 1275);

const VErrorDB4D	VE_DB4D_WRONG_STREAM_HEADER	= MAKE_VERROR(CDB4DManager::Component_Type, 1276);

const VErrorDB4D	VE_DB4D_INTEGRATE_JOURNAL_FAILED_AT	= MAKE_VERROR(CDB4DManager::Component_Type, 1277);

const VErrorDB4D	VE_DB4D_JS_NOT_ALLOWED_IN_THAT_QUERY	= MAKE_VERROR(CDB4DManager::Component_Type, 1278);

const VErrorDB4D	VE_DB4D_QUERY_PLACEHOLDER_NULL_OR_MISSING	= MAKE_VERROR(CDB4DManager::Component_Type, 1279);

const VErrorDB4D	VE_DB4D_QUERY_PLACEHOLDER_WRONGTYPE	= MAKE_VERROR(CDB4DManager::Component_Type, 1280);


/* Backup related errors */
const VErrorDB4D	VE_DB4D_BACKUP_FOLDER_INVALID	  = MAKE_VERROR(CDB4DManager::Component_Type, 3000);
const VErrorDB4D	VE_DB4D_BACKUP_FOLDER_NOT_FOUND   = MAKE_VERROR(CDB4DManager::Component_Type, 3001);
const VErrorDB4D	VE_DB4D_BACKUP_CANNOT_BACKUP_JOURNAL = MAKE_VERROR(CDB4DManager::Component_Type, 3002);
const VErrorDB4D	VE_DB4D_BACKUP_CANNOT_RESTORE_JOURNAL = MAKE_VERROR(CDB4DManager::Component_Type, 3003);
const VErrorDB4D	VE_DB4D_BACKUP_CANNOT_BACKUP_ITEM = MAKE_VERROR(CDB4DManager::Component_Type, 3004);





           /* -------------------------------------- */

typedef enum { DBaction_SavingRecord, DBaction_LoadingRecord, DBaction_DeletingRecord, DBaction_UpdatingRecord, 
							 DBaction_OpeningBase, DBaction_ClosingBase, DBaction_SavingStruct, DBaction_CreatingBase, 
							 DBaction_WritingData, DBaction_ReadingData, DBaction_ReadingStruct, DBaction_OpeningStruct, DBaction_CreatingStruct,
							 DBaction_OpeningDataFile, DBaction_ClosingDataFile, DBaction_CreatingDataFile,
							 DBaction_LookingForFreeDiskSpace, DBaction_FreeingDiskSpace,
							 DBaction_SavingDataSegs, DBaction_ReadingDataSegs, DBaction_ReadingBaseHeader,
							 DBaction_CreatingTable, DBaction_ReadingIndexTable, DBaction_WritingIndexTable,
							 DBaction_CreatingIndex, DBaction_DeletingIndex, DBaction_ChangingBaseName,
							 DBaction_LoadingBitSel, DBaction_LoadingSet, DBaction_ModifyingSet, DBaction_IncreasingSetSize,
							 DBaction_DropingSet, DBaction_SavingSet, DBaction_SavingBitSel, DBaction_SavingBlob, DBaction_LoadingBlob, DBaction_CopyingBlob,
							 DBaction_LoadingFreeBit, DBaction_SavingFreeBit, DBaction_LoadingFreeBitTable, DBaction_SavingFreeBitTable,
							 DBaction_ClosingSeg, DBaction_DeletingSeg, DBaction_OpeningSeg, DBaction_CreatingSeg, DBaction_AllocatingSpaceInDataSeg,
							 DBaction_FreeingSpaceInDataSeg, DBaction_SavingDataSeg, DBaction_AllocatingRecordInMem, DBaction_AccessingField,
							 DBaction_ChangingFieldName, DBaction_ChangingTableName, DBaction_AccessingFieldDef, DBaction_AddingBlobDependancy,
							 DBaction_SavingTableDef, DBaction_UpdatingTableDef, DBaction_LoadingTableDef, DBaction_UpdatingIndexKeyForRecord, 
							 DBaction_DeletingIndexKeyForRecord, DBaction_UpdatingBlobForRecord, DBaction_DeletingBlobForRecord, 
							 DBaction_AddingField, DBaction_ConstructingTableInMem, DBaction_DeletingTable, DBaction_LockingRecord, DBaction_UnLockingRecord, 
							 DBaction_DeletingBlob, DBaction_SeqSearchingInDataTable, DBaction_SavingTableHeader,
							 DBaction_AddingColToImpExp, DBaction_ExportingData, DBaction_ImportingData,
							 DBaction_AllocatingIndexKeyInMem, DBaction_SavingIndexHeader, DBaction_LoadingIndexHeader, 
							 DBaction_GettingIndexPageAddr, DBaction_SettingIndexPageAddr, DBaction_DropingIndex, DBaction_SortingOnIndex,
							 DBaction_LoadingIndexPage, DBaction_SavingIndexPage, DBaction_InsertingKeyIntoIndex, DBaction_DeletingKeyFromIndex,
							 DBaction_SearchingInIndex, DBaction_SortingWithIndex, DBaction_InsertingKeyIntoPage, DBaction_DeletingKeyFromPage,
							 DBaction_BuildingIndex, DBaction_GetDistinctValuesWithIndex, DBaction_CalculateFomulasWithIndex,
							 DBaction_LoadingCluster, DBaction_AddingToCluster, DBaction_DeletingFromCluster,
							 DBaction_SelectingAllRecords, DBaction_PerformingSearch, DBaction_SortingSelection,
							 DBaction_CheckingTable, DBaction_ActivatingManyToOneRelation, DBaction_ActivatingOneToManyRelation, DBaction_CreatingRelation,
							 DBaction_BuildingListOfRelations, DBaction_LoadingRelations, DBaction_SavingRelations, DBaction_ModifyingRelation,
							 DBaction_BuildingQueryFromString, DBaction_CheckingReferentialIntegrity, DBaction_TryingToUpdateDB,
							 DBaction_BuildingSelection, DBaction_ScanningIndex, DBaction_ChangingFieldProperties, 
							 DBaction_UpdatingRemoteTable, DBaction_SendingRemoteBase, DBaction_OpeningRemoteBase, 
							 DBaction_LoadingStructElemDef, DBaction_LoadingAutoSeqHeader, DBaction_CreatingAutoSeqHeader, DBaction_GettingDataSegs,
							 DBaction_GettingIndexes, DBaction_IntegratingJournal, DBaction_ComputingSet, DBaction_BuildingArrayOfValues, 
							 DBaction_SavingArrayOfValues, DBaction_LoadingArrayOfValues, DBaction_BuildingKeywords, DBaction_LoadingSubRecord,
							 DBaction_SelectinSubRecords, DBaction_SavingSubRecord, DBaction_CreatingSubRecord, DBaction_DeletingSubRecord,
							 DBaction_BuildingValue, DBaction_ChangeCacheSize, DBaction_BuildingSet, DBaction_BuildingRecord, DBaction_BuildingFieldList,
							 DBaction_BuildingIndexKey, DBaction_RegisteringBase, DBaction_BuildingListOfIndexes, DBaction_BuildingListOfFields,
							 DBaction_StartingServer, DBaction_SettingPrimaryKey, DBaction_DataToCollection, DBaction_CollectionToData, 
							 DBaction_LockingSelection, DBaction_CheckingUniqueness, DBaction_LoadingIndexKeyFromTrans, DBaction_SavingIndexKeyFromTrans,
							 DBaction_LoadingIndexKey, DBaction_AllocatingIndexPageInMem, DBaction_IndexKeyIterator, DBaction_AddingALock, 
							 DBaction_LoadingQuery, DBaction_SavingQuery, DBaction_ExecutingQuery, DBaction_ExecutingColumnFormula, 
							 DBaction_AccessingComplexSelection, DBaction_BuildingComplexQuery, DBaction_ExecutingComplexQuery,
							 DBaction_GetDistinctValues, DBaction_DeletingSelectionOfRecord, DBaction_LoadingSelection, DBaction_ServerConnecting,
							 DBaction_SendingData, DBaction_ReceivingData, DBaction_SendingRequest, dbaction_BuildingSortCriterias,
							 DBaction_BuildingDistinctKeys, DBaction_StoringObjectInCache, DBaction_BuildingAddrTable, DBaction_LoadingAddrTable,
							 DBaction_ModifyingAddrTable, DBaction_LoadingRecordFromTemp, DBaction_ExecutingTransaction, DBaction_LoadingBlobFromTemp, 
							 DBaction_SavingRecordIntoTemp, DBaction_SavingBlobIntoTemp, DBaction_StartingTransaction, DBaction_CommittingTransaction,
							 DBaction_AccessingExtraProperty, DBaction_ModifyingExtraProperty, DBaction_AccessingNullIndexKeys, DBaction_ModifyingNullIndexKeys,
							 DBaction_StartingBackup, DBaction_FinishingBackup, DBaction_DeletingRelation, DBaction_DeletingField, DBaction_ChangingIndexName,
							 DBaction_RebuildingIndex, DBaction_ChangingTableProperties, DBaction_OpeningJournal,
							 DBactionFinale } ActionDB4D;


           /* -------------------------------------- */


/*
	These are keys XBOX::VValueBag::StKey accessors for DB4D definition bags.

	Because DB4D is a component, these values cannot be exported from DB4D.
	You have to include DB4DBagKeys.h in one, and only one, cpp file of your project to have these values defined.
	
*/
namespace DB4DBagKeys
{
	EXTERN_BAGKEY(missing_datatable);
	EXTERN_BAGKEY(matching_datatable);
	EXTERN_BAGKEY(datatable_num);
	EXTERN_BAGKEY(table_num);
	EXTERN_BAGKEY(value);
	EXTERN_BAGKEY(record);
	EXTERN_BAGKEY(datatable);
	EXTERN_BAGKEY(records_count);
	EXTERN_BAGKEY(table_name);

	EXTERN_BAGKEY_WITH_DEFAULT( name, XBOX::VString);
	EXTERN_BAGKEY_WITH_DEFAULT( uuid, XBOX::VUUID);
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( id, XBOX::VLong, sLONG);
	EXTERN_BAGKEY( kind);
	EXTERN_BAGKEY( type);
	EXTERN_BAGKEY( state);

	EXTERN_BAGKEY( base);
	EXTERN_BAGKEY( base_extra);
	EXTERN_BAGKEY( editor_base_info);

	EXTERN_BAGKEY( relation);
	EXTERN_BAGKEY( relation_extra);
	EXTERN_BAGKEY( editor_relation_info);

	EXTERN_BAGKEY( schema);

	EXTERN_BAGKEY( table);
	EXTERN_BAGKEY( table_extra);
	EXTERN_BAGKEY( editor_table_info);

	EXTERN_BAGKEY( field);
	EXTERN_BAGKEY( field_extra);
	EXTERN_BAGKEY( editor_field_info);

	EXTERN_BAGKEY( journal_file);
	EXTERN_BAGKEY( journal_file_enabled );

	EXTERN_BAGKEY_WITH_DEFAULT( name_Nto1, XBOX::VString);
	EXTERN_BAGKEY_WITH_DEFAULT( name_1toN, XBOX::VString);

	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( entry_autofill, XBOX::VBoolean, bool);
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( entry_wildchar, XBOX::VBoolean, bool);
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( entry_create, XBOX::VBoolean, bool);

	EXTERN_BAGKEY_WITH_DEFAULT( integrity, XBOX::VString);
	EXTERN_BAGKEY( tip);
	EXTERN_BAGKEY( comment);
	EXTERN_BAGKEY( format);
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( auto_load_Nto1, XBOX::VBoolean, bool);
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( auto_load_1toN, XBOX::VBoolean, bool);
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( foreign_key, XBOX::VBoolean, bool);
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( choice_field, XBOX::VLong, sLONG);
	EXTERN_BAGKEY( related_field);
	EXTERN_BAGKEY( index);
	EXTERN_BAGKEY( field_ref);
	EXTERN_BAGKEY( table_ref);
	EXTERN_BAGKEY( index_ref);
	EXTERN_BAGKEY( primary_key);
	EXTERN_BAGKEY( field_name);
	EXTERN_BAGKEY( field_uuid);
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( hide_in_REST, XBOX::VBoolean, bool);
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( styled_text, XBOX::VBoolean, bool);
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( outside_blob, XBOX::VBoolean, bool);
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( enterable, XBOX::VBoolean, bool);
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( modifiable, XBOX::VBoolean, bool);
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( mandatory, XBOX::VBoolean, bool);
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( enumeration_id, XBOX::VLong, sLONG);
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( visible, XBOX::VBoolean, bool);
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( trashed, XBOX::VBoolean, bool);
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( never_null, XBOX::VBoolean, bool);
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( not_null, XBOX::VBoolean, bool);
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( unique, XBOX::VBoolean, bool);	// for field
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( unique_keys, XBOX::VBoolean, bool);	// for index
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( autosequence, XBOX::VBoolean, bool);
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( autogenerate, XBOX::VBoolean, bool);
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( store_as_utf8, XBOX::VBoolean, bool);
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( store_as_UUID, XBOX::VBoolean, bool);
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( limiting_length, XBOX::VLong, sLONG);
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( text_switch_size, XBOX::VLong, sLONG);
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( blob_switch_size, XBOX::VLong, sLONG);
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( compressed, XBOX::VBoolean, bool);
	EXTERN_BAGKEY_WITH_DEFAULT( multi_line, XBOX::VString);
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( position, XBOX::VLong, sLONG);
//	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( fully_delete_records, XBOX::VBoolean, bool);		// sc 19/02/2008, doublon de "leave_tag_on_delete"
	EXTERN_BAGKEY( coordinates);
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( trigger_load, XBOX::VBoolean, bool);
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( trigger_delete, XBOX::VBoolean, bool);
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( trigger_update, XBOX::VBoolean, bool);
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( trigger_insert, XBOX::VBoolean, bool);
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( fields_ordering, XBOX::VLong, sLONG);
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( leave_tag_on_delete, XBOX::VBoolean, bool);
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( keep_record_sync_info, XBOX::VBoolean, bool);
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( keep_record_stamps, XBOX::VBoolean, bool);
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( sql_schema_id, XBOX::VLong, sLONG);
	EXTERN_BAGKEY( color);
	EXTERN_BAGKEY( qt_spatial_settings);
	EXTERN_BAGKEY( wedd);
	EXTERN_BAGKEY( data_file_path);
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( structure_opener, XBOX::VBoolean, bool);
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( resman_stamp, XBOX::VLong, sLONG);
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( resman_marker, XBOX::VLong8, sLONG8);
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( source_code_available, XBOX::VBoolean, bool);
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( source_code_stamp, XBOX::VLong, uLONG);
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( intel_code_stamp, XBOX::VLong, uLONG);
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( intel64_code_stamp, XBOX::VLong, uLONG);
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( last_opening_mode, XBOX::VLong, sLONG);
	EXTERN_BAGKEY_WITH_DEFAULT( package_name, XBOX::VString); //Used to detect the package name changes
	EXTERN_BAGKEY_WITH_DEFAULT( structure_file_name, XBOX::VString); //Used to detect the structure file name changes
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( picture_format, XBOX::VLong, sLONG);
	EXTERN_BAGKEY_WITH_DEFAULT( picture_name, XBOX::VString);
	EXTERN_BAGKEY_WITH_DEFAULT( font_name, XBOX::VString);
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( font_size, XBOX::VLong, sLONG);
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( collapsed, XBOX::VBoolean, bool);
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( displayable_fields_count, XBOX::VLong, sLONG);
	EXTERN_BAGKEY( temp_folder);
	EXTERN_BAGKEY_WITH_DEFAULT( folder_selector, XBOX::VString);	// data | structure | system | custom
	EXTERN_BAGKEY_WITH_DEFAULT( path, XBOX::VString);	// file system path
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( open_data_one_version_more_recent_mode, XBOX::VLong, sLONG);	// tells if one can open a data one major version more recent.
	EXTERN_BAGKEY( collation_locale);	// locale id for VCollator
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( collator_ignores_middle_wildchar, XBOX::VBoolean, bool);
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( consider_only_dead_chars_for_keywords, XBOX::VBoolean, bool);

	// for extra data in CDBBaseContext
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( task_id, XBOX::VLong, sLONG);
	EXTERN_BAGKEY_WITH_DEFAULT( user_name, XBOX::VString);
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( user4d_id, XBOX::VLong, sLONG);
	EXTERN_BAGKEY_WITH_DEFAULT( host_name, XBOX::VString);
	EXTERN_BAGKEY_WITH_DEFAULT( task_name, XBOX::VString);
	EXTERN_BAGKEY_WITH_DEFAULT( client_uid, XBOX::VUUID);
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( is_remote_context, XBOX::VBoolean, bool);
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( client_version, XBOX::VLong, sLONG);
	
	extern const XBOX::VValueBag::StBagElementsOrdering	ordering[]; 
};

namespace LogFileBagKey
{
	EXTERN_BAGKEY( journal_file );
	EXTERN_BAGKEY( datalink );
	EXTERN_BAGKEY( filepath );
	EXTERN_BAGKEY( next_filepath );
	EXTERN_BAGKEY( sequence_number );
};


namespace d4
{
	EXTERN_BAGKEY(name);
	EXTERN_BAGKEY(type);
	EXTERN_BAGKEY(kind);
	EXTERN_BAGKEY(dbInfo);

	EXTERN_BAGKEY(relationship);
	EXTERN_BAGKEY(relationPath);
	EXTERN_BAGKEY(sourceTable);
	EXTERN_BAGKEY(sourceColumn);
	EXTERN_BAGKEY(destinationTable);
	EXTERN_BAGKEY(destinationColumn);

	EXTERN_BAGKEY(columnName);
	EXTERN_BAGKEY(fieldPos);
	EXTERN_BAGKEY(readOnly);
	EXTERN_BAGKEY(script);
	EXTERN_BAGKEY(scriptKind);
	EXTERN_BAGKEY(path);
	EXTERN_BAGKEY(reversePath);
	EXTERN_BAGKEY(primKey);
	EXTERN_BAGKEY(indexKind);
	EXTERN_BAGKEY(indexed);
	EXTERN_BAGKEY(identifying);
	EXTERN_BAGKEY(multiLine);
	EXTERN_BAGKEY(simpleDate);

	EXTERN_BAGKEY(onGet);
	EXTERN_BAGKEY(onSet);
	EXTERN_BAGKEY(onQuery);
	EXTERN_BAGKEY(onSort);

	EXTERN_BAGKEY(dataClasses);
	EXTERN_BAGKEY(dataSource);
	EXTERN_BAGKEY(extends);
	EXTERN_BAGKEY(attributes);
	EXTERN_BAGKEY(defaultTopSize);
	EXTERN_BAGKEY(identifyingAttribute);
	EXTERN_BAGKEY(key);
	EXTERN_BAGKEY(optionnal);
	EXTERN_BAGKEY(uuid);
	EXTERN_BAGKEY(tablePos);
	EXTERN_BAGKEY(singleEntityName);
	EXTERN_BAGKEY(className);
	EXTERN_BAGKEY(collectionName);
	EXTERN_BAGKEY(noEdit);
	EXTERN_BAGKEY(noSave);
	EXTERN_BAGKEY(allow);
	EXTERN_BAGKEY(publishAsJSGlobal);
	EXTERN_BAGKEY(allowOverrideStamp);

	EXTERN_BAGKEY(queryStatement);
	EXTERN_BAGKEY(applyToModel);
	EXTERN_BAGKEY(restrictingQuery);
	EXTERN_BAGKEY(top);
	EXTERN_BAGKEY(orderBy);

	EXTERN_BAGKEY(methods);
	EXTERN_BAGKEY(source);
	EXTERN_BAGKEY(from);
	EXTERN_BAGKEY(applyTo);
	EXTERN_BAGKEY(parameter);
	EXTERN_BAGKEY(returnType);
	EXTERN_BAGKEY(scope);
	EXTERN_BAGKEY(serverOnly);
	EXTERN_BAGKEY(userDefined);

	EXTERN_BAGKEY(enumeration);
	EXTERN_BAGKEY(item);
	EXTERN_BAGKEY(value);

	EXTERN_BAGKEY(minValue);
	EXTERN_BAGKEY(maxValue);
	EXTERN_BAGKEY(defaultValue);
	EXTERN_BAGKEY(pattern);
	EXTERN_BAGKEY(fixedLength);
	EXTERN_BAGKEY(minLength);
	EXTERN_BAGKEY(maxLength);
	EXTERN_BAGKEY(autoComplete);

	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( textAsBlob, XBOX::VBoolean, bool);

	EXTERN_BAGKEY(defaultFormat);
	EXTERN_BAGKEY(locale);
	EXTERN_BAGKEY(format);
	EXTERN_BAGKEY(presentation);
	EXTERN_BAGKEY(sliderMin);
	EXTERN_BAGKEY(sliderMax);
	EXTERN_BAGKEY(sliderInc);

	EXTERN_BAGKEY(And);
	EXTERN_BAGKEY(Or);
	EXTERN_BAGKEY(Not);
	EXTERN_BAGKEY(subquery);

	EXTERN_BAGKEY(__KEY);
	EXTERN_BAGKEY(__ENTITIES);
	EXTERN_BAGKEY(__RECID);
	EXTERN_BAGKEY(__STAMP);
	EXTERN_BAGKEY(__ERROR);

	EXTERN_BAGKEY(__deferred);
	EXTERN_BAGKEY(image);
	EXTERN_BAGKEY(uri);
	EXTERN_BAGKEY(dataURI);
	EXTERN_BAGKEY(mediatype);
	EXTERN_BAGKEY(height);
	EXTERN_BAGKEY(width);

	EXTERN_BAGKEY(permissions);
	EXTERN_BAGKEY(action);
	EXTERN_BAGKEY(group);
	EXTERN_BAGKEY(groupID);
	EXTERN_BAGKEY(resource);
	EXTERN_BAGKEY(temporaryForcePermissions);
	EXTERN_BAGKEY(entityModelPerm);
	EXTERN_BAGKEY(read);
	EXTERN_BAGKEY(update);
	EXTERN_BAGKEY(xdelete);
	EXTERN_BAGKEY(create);
	EXTERN_BAGKEY(execute);
	EXTERN_BAGKEY(write);

	EXTERN_BAGKEY(extraProperties);

	EXTERN_BAGKEY(events);

	EXTERN_BAGKEY(outsideCatalogs);
	EXTERN_BAGKEY(user);
	EXTERN_BAGKEY(password);

};

#pragma pack( pop )

#endif
