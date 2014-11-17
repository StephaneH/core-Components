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
#ifndef __DB_BACKUP__ 
#define __DB_BACKUP__

#include "Kernel/VKernel.h"

BEGIN_TOOLBOX_NAMESPACE
const OsType kBACKUP_RESTORE_SIGNATURE = 'bkrs';

//Compatibility errors
//the data store has a waExtra file which is no supported
DECLARE_VERROR( kBACKUP_RESTORE_SIGNATURE, 100, VE_INCOMPATIBLE_DATASTORE_FORMAT_WAEXTRA) 
DECLARE_VERROR( kBACKUP_RESTORE_SIGNATURE, 200, VE_FAILED_TO_RETRIEVE_JOURNAL_INFO)
DECLARE_VERROR( kBACKUP_RESTORE_SIGNATURE, 201, VE_JOURNAL_NOT_FOUND)
DECLARE_VERROR( kBACKUP_RESTORE_SIGNATURE, 202, VE_FAILED_TO_PERFORM_BACKUP)
DECLARE_VERROR( kBACKUP_RESTORE_SIGNATURE, 203, VE_FAILED_TO_CREATE_BACKUP_FOLDER)
DECLARE_VERROR( kBACKUP_RESTORE_SIGNATURE, 204, VE_CANNOT_CREATE_JOURNAL_BACKUP)
DECLARE_VERROR( kBACKUP_RESTORE_SIGNATURE, 205, VE_INVALID_JOURNAL_PATH)
DECLARE_VERROR( kBACKUP_RESTORE_SIGNATURE, 206, VE_INVALID_BACKUP_FOLDER)
DECLARE_VERROR( kBACKUP_RESTORE_SIGNATURE, 207, VE_CANNOT_CREATE_DATAFOLDER_BACKUP)
DECLARE_VERROR( kBACKUP_RESTORE_SIGNATURE, 208, VE_INVALID_NEXT_JOURNAL_PATH)
DECLARE_VERROR( kBACKUP_RESTORE_SIGNATURE, 209, VE_INVALID_BACKUP_SETTINGS)
DECLARE_VERROR( kBACKUP_RESTORE_SIGNATURE, 210, VE_FAILED_TO_STORE_MANIFEST)
DECLARE_VERROR(kBACKUP_RESTORE_SIGNATURE, 211, VE_BACKUP_CONFIGURATION_PROPERTY_NOT_FOUND)
DECLARE_VERROR(kBACKUP_RESTORE_SIGNATURE, 212, VE_BACKUP_WRONG_PROPERTY_TYPE_EXPECTED_FOLDER)
DECLARE_VERROR(kBACKUP_RESTORE_SIGNATURE, 213, VE_BACKUP_WRONG_PROPERTY_TYPE_EXPECTED_STRING)
DECLARE_VERROR(kBACKUP_RESTORE_SIGNATURE, 214, VE_BACKUP_WRONG_PROPERTY_TYPE_EXPECTED_BOOLEAN)
DECLARE_VERROR(kBACKUP_RESTORE_SIGNATURE, 215, VE_BACKUP_WRONG_PROPERTY_TYPE_EXPECTED_STRING_OR_NUMBER)
DECLARE_VERROR(kBACKUP_RESTORE_SIGNATURE, 216, VE_BACKUP_INVALID_PROPERTY_VALUE)
DECLARE_VERROR(kBACKUP_RESTORE_SIGNATURE, 217, VE_BACKUP_FOLDER_NOT_EMPTY)
DECLARE_VERROR(kBACKUP_RESTORE_SIGNATURE, 218, VE_CANNOT_OPEN_BACKUP_REGISTRY)


//Restore errors
DECLARE_VERROR( kBACKUP_RESTORE_SIGNATURE, 400, VE_CANNOT_RESTORE_JOURNAL)//journal could not be restored after a failed backup
DECLARE_VERROR( kBACKUP_RESTORE_SIGNATURE, 401, VE_CANNOT_RESTORE_DATA_FOLDER)//data folder cannot be restored
DECLARE_VERROR( kBACKUP_RESTORE_SIGNATURE, 402, VE_CANNOT_CREATE_DATA_FOLDER_SAFETY_COPY)//data folder safety copy cannot be created
DECLARE_VERROR(kBACKUP_RESTORE_SIGNATURE, 403, VE_CANNOT_CREATE_JOURNAL_SAFETY_COPY)//journal safety copy cannot be created
DECLARE_VERROR(kBACKUP_RESTORE_SIGNATURE, 404, VE_CANNOT_CREATE_DATA_FOLDER)//impossible to create data folder directory


END_TOOLBOX_NAMESPACE


namespace Helpers
{
	extern void LocalizeJournalOperation(uLONG inOpType, XBOX::VString& caption);
}


namespace JournalUtils
{
	VError DisableJournal(Base4D* inBase,CDB4DBaseContext* inContext);
	void GetJournalInfos(const VValueBag& inExtraProperties,const VFolder& inBaseFolder,XBOX::VFilePath& outJournalPath,XBOX::VUUID& outDataLink);
};

/**
 * \brief Describes backup content date of production, along with some other meta data
 * \details a backup manifest is produced by the backup tool each time a backup successfully completes.
 * The manifest details things such as what the backup contains and under which form, when the backup was
 * produced and the version of the producing tool.
 * The manifest is primarily used for communication between backup and restore tools
 */
class VBackupManifest : public XBOX::VObject, public XBOX::IRefCountable
{
public:
	VBackupManifest();
	virtual ~VBackupManifest();

	/**
	 * \brief Sets the data folder contained in this backup
	 */
	void SetDataFolder(const XBOX::VFilePath& inDataFolderPath){fDataFolderPath = inDataFolderPath;}

	const XBOX::VFilePath& GetDataFolder()const{return fDataFolderPath;}
	
	/**
	 * \brief Sets the path of the journal contined in this backup (if applicable)
	 */
	void SetJournal(const XBOX::VFilePath& inJournalPath){fJournalPath = inJournalPath;}

	const XBOX::VFilePath& GetJournal()const{return fJournalPath;}
	
	/**
	 * \brief returns a JSON object representation of this instance
	 * \param outJsonString the JSON representation of this instance
	 */
	void ToJSONString (XBOX::VString& outJsonString)const;

	XBOX::VError FromJSONString(const XBOX::VString& inJsonString);

	XBOX::VError ToStream(XBOX::VStream& inStream)const;

	XBOX::VError FromStream(XBOX::VStream& inStream);

private:
	XBOX::VFilePath fDataFolderPath;
	XBOX::VFilePath fJournalPath;
	XBOX::VTime		fDate;
};

class VBackupSettings : public XBOX::VObject, public IBackupSettings
{
public:
	VBackupSettings();
	virtual ~VBackupSettings();

	///Initializes the instance from a value bag
	virtual XBOX::VError Initialize(const VValueBag& inBag, const VFilePath& inBaseFolder);

	/**
	* \brief Constructs an instance from a JS object
	* \param inObject the object to init from
	*/
	virtual XBOX::VError Initialize(const XBOX::VJSObject& inObject);

	virtual IBackupSettings* Clone()const;
	
	/**
	 * \brief Initializes/resets the instance using a JSON object
	 * \details this method can be called using only a subset of initializations keys enabling to partially reset the instance
	 * and preserve some values.
	 * \param inObject the JS object carrying the init values.
	 * \return false if the final object is invalid state after the new values have been taken into account
	 * \return true if the object is valid after the new values have been taken into account
	 * \see CheckValidity()
	 */
	virtual XBOX::VError FromJSObject(const XBOX::VJSObject& inObject);

	/**
	 * \brief Sets the folder to use as a backup registry
	 * \details The backup registry folder is a special folder where the backup tools
	 * store and retrieve some informations such as the backup manifest registry.
	 * \param inBackupRegistryFolder the folder where the backup registry is located
	 * \return true if @c inBackupRegistryFolder is a valid folder and is actually reachable.
	 * \return false if @c inBackupRegistryFolder is not a valid folder or is not reachable.
	 */
	virtual bool SetBackupRegistryFolder(const XBOX::VFilePath& inBackupRegistryFolder);

	/**
	 * \brief Gets the folder to use as a backup registry
	 * \details The backup registry folder is a special folder where the backup tools
	 * store and retrieve some informations such as the backup manifest registry
	 * \param outBackupRegistryFolder the folder currently used to store the backup registry
	 */
	virtual bool GetBackupRegistryFolder(XBOX::VFilePath& outBackupRegistryFolder)const;

	
	///Set the backup destination folder for this instance
	virtual void SetDestination(const XBOX::VFilePath& inBackupFolder);
	
	///Gets the backup destination folder set for this instance or the parent if applicable
	virtual bool GetDestination(XBOX::VFilePath& inBackupFolder)const;	

	///Returns whether the backups should use unique names
	virtual bool GetUseUniqueNames()const;

	///Returns whether the backups should use unique names
	virtual void SetUseUniqueNames(bool inUseUniqueNames);

	/**
	* \brief Sets whether all backups must be kept
	* \param inRetainAll if true all backups are retained. If false then the previous max retained backup count is used
	*/
	virtual void SetRetainAllBackups(bool inRetainAll);

	/**
	* \brief Sets the maximum number of backups to keep
	* \param inCount the number of backups to keep (at least one)
	*/
	virtual void SetMaxRetainedBackups(int inCount);

	/**
	* \brief Gets the number of backups to keep
	* \param[out] outKeepAll true if all backups must be kept, false otherwise
	* \returns the number of backups to keep (if @coutKeepAll is false). Value must be ignored if @c outKeepAll is true
	*/
	virtual int GetMaxRetainedBackups(bool& outKeepAll) const;


	///Checks this instance validity
	virtual VError CheckValidity()const;

	///Converts this object to its JSON representation string
	virtual void ToJSONString(XBOX::VString& outJSON)const;

	///Converts this instance into a JS object
	void ToJSObject(XBOX::VJSObject& outObject)const;

private:
	XBOX::VFilePath		fBackupRegistryFolder;
	XBOX::VFilePath		fDestinationFolderPath;
	bool				fUniqueNames;
	int					fMaxRetainedBackups;
};

class Base4D;
class CDB4DBaseContext;
class IDB4D_DataToolsIntf;

/**
 * \brief The Wakanda database backup and restore tool API
 * \details This class implements methods that are to be used to perform
 * backup or restore operation on a database data folder
 */
class VBackupTool: public XBOX::VObject, public IBackupTool
{
public:
	///Constructor
	VBackupTool();
	virtual ~VBackupTool();

	/**
	 * \brief Converts the designated backup registry into a JS array of manifests
	 * \param inBackupRegistryFilePath path to the backup registry file
	 * \param outManifests the JS object where the registry is stored
	 * \return true if the operation succeeded.
	 */
	virtual bool ConvertBackupRegistryToJsObject(const XBOX::VFilePath& inBackupRegistryFilePath,XBOX::VJSArray& outManifests);


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
	virtual bool BackupDatabase(CDB4DBase* inBase,CDB4DBaseContext* inContext,const IBackupSettings& inBackupSettings,XBOX::VFilePath* outManifestFilePath,IDB4D_DataToolsIntf* inBackupEventLog);
	
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
	virtual bool BackupDatabaseAndChangeJournal(CDB4DBase* inBase,CDB4DBaseContext* inContext,const IBackupSettings& inBackupSettings,const XBOX::VFilePath* inNextJournalPath,XBOX::VFilePath* outManifestFilePath,IDB4D_DataToolsIntf* inBackupEventLog);

	
	/**
	 * \brief Backs up a closed database by referring to its data and model file 
	 * \details This method will perform a backup of the journal file and the data folder and will reset the journal file afterwards.
	 * The next time the target database is opened it will start with a fresh empty journal
	 * \param inModelFile the structure file of the database to backup. This file must not belong to an opened database.
	 * \param inDataFile  the data  file of the database to backup. This file must not belong to an opened database.
	 * \param inBackupSettings settings to use to perform the backup
	 * \param outManifestFilePath optional, if non NULL will contain the backup manifest file path
	 * \param inBackupEventLog optional, if non NULL will receive progress indications
	 * \return true if the backup went fine
	 * \return false if an error occured in the process
	 */
	virtual bool BackupClosedDatabase(XBOX::VFile& inModelFile,XBOX::VFile& inDataFile,const IBackupSettings& inBackupSettings,XBOX::VFilePath* outManifestFilePath,IDB4D_DataToolsIntf* inBackupEventLog);

	virtual XBOX::VError GetLastBackupPath(const IBackupSettings& inBackupSettings,XBOX::VFilePath& outLastBackupLocation);
	
	/**
	* \brief Restores a damaged data folder and its journal using another data folder and possibly another journal.
	* \details Restores the data folder pointed to by @inDamagedDataFolder by copying data related files from @inDataFolderToRestoreFrom.
	* If @c inDamagedDataFolder uses a journal file, it must be specified in @c inCurrentJournal. After restoration, the journal is properly
	* preserved, unless it requires to be replaced by another journal which has to be specified by @c outReplacedJournalCopy.
	* \param inDamagedDataFolder location of the data folder requiring restoration
	* \param inDataFolderToRestoreFrom location of the data folder to use for restoration
	* \param inCurrentJournal if not NULL, the path to @c inDamagedDataFolder's current journal
	* \param inJournalToRestoreFrom if not NULL and @c inCurrentJournal is not NULL, the journal at @c inJournalToRestoreFrom will replace @c inDamagedDataFolder's journal after restoration.
	* \param outDamagedDFCopy path to the saftey copy of @c inDamagedDataFolder
	* \param outReplacedJournalCopy, the path to @c inCurrentJournal's safety copy if it had to be replaced.
	*/
	virtual bool RestoreDataStore(XBOX::VFolder& inDamagedDataFolder, const XBOX::VFolder& inDataFolderToRestoreFrom, const XBOX::VFile* inDamagedDFCurrentJournal, const XBOX::VFile* inJournalToRestoreFrom, 
		IDB4D_DataToolsIntf* inBackupEventLog, VFilePath* outDamagedDFCopy, VFilePath* outReplacedJournalCopy);

	/**
	 * \brief Restores the data file from a backup archive
	 * \param inBackupManifestPath the path the backup manifest
	 * \param inRestoredFilePath the full filepath of the restored file. 
	 * \param inOverwrite if true then if when any file named @c inRestoredFilePath will be overwritten otherwise the extract will fail
	 * \param inProgress the progress object to give feedback about the extraction
	 * \return true if the restoration succeeded return false otherwise
 	 */
	virtual bool RestoreDataFile(const XBOX::VFilePath& inBackupManifestPath,const XBOX::VFilePath& inRestoredFilePath,IDB4D_DataToolsIntf* inProgress,bool inOverwrite);
	
	/**
	 * \brief Restores the data file from a backup archive
	 * \param inBackupManifestPath the the backup manifest path  
	 * \param inRestoreFolder Path of the folder where the data folder will be restored 
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
	virtual bool RestoreDataFolder(const XBOX::VFilePath& inBackupManifestPath,const XBOX::VFilePath& inRestoreFolderPath,XBOX::VFilePath& outRenamedDataFolderPath,IDB4D_DataToolsIntf* inProgress);
	
	/**
	 * \brief Copies the content of a backed up data folder into the specified folder, overwritting conflicting content
	 * \param inBackupManifestPath the the backup manifest path  
	 * \param inRestoreFolderPath Path to the folder where the data will be copied
	 * \param inProgress the progress object to give feedback about the extraction
	 * \return true if the restoration succeeded return false otherwise
	 * Example
	 * <pre>
	 * Assuming the following hierarchy:
	 * c:/
	 * |-MyProject
	 * |     |-DataFolder   
	 * |     |    |-data.waData
	 * |     |    |-index.waIndex
	 *
	 * VFilePath folder;
	 * folder.FromFullPath(CVSTR("c:/MyProject/DataFolder_Restored/"));
	 * backupTool->RestoreDataFolder(myManifest,folder,renamed,NULL);
	 *
	 * Will create the following:
	 * c:/
	 * |-MyProject
	 * |     |-DataFolder        <--- this was already there and unmodified
	 * |     |    |-data.waData
	 * |     |    |-index.waIndex
	 * |     |
	 * |     |-data.waData       <--- this is the restored content
	 * |     |-index.waIndex
	 * </pre>
 	 */
	virtual bool RestoreDataFolderContent(const XBOX::VFilePath& inBackupManifestPath,const XBOX::VFilePath& inRestoreFolderPath,IDB4D_DataToolsIntf* inProgress);



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
	virtual bool RestoreJournal(const XBOX::VFilePath& inBackupManifestPath,const XBOX::VFilePath& inRestoreFolder,XBOX::VFilePath& outRenamedJournalPath,IDB4D_DataToolsIntf* inProgress);

	
protected:
	/**
	 * \brief Backs up the database and sets the specified new journal
	 * \param inNextJournalFile if NULL then journaling is disabled after backup is performed
	 */
	bool _DoBackup(Base4D* inBase,CDB4DBaseContext* inContext,const IBackupSettings& inBackupConfig,const XBOX::VFilePath* inNextJournalFile,XBOX::VFilePath* outManifestFile,IDB4D_DataToolsIntf* inBackupEventLog);

	/**
	 * \brief Prepares the database for backup
	 * \return VE_OK if no error occured
	 */
	XBOX::VError _PrepareForBackup(Base4D* inBase,CDB4DBaseContext* inContext);
	
	/**
	 * \brief Performs processing after a backup transaction
	 * \return VE_OK if no error occured
	 */
	XBOX::VError _EndBackup(Base4D* inBase,CDB4DBaseContext* inContext, VFile* inOldJournal);


	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////

	/**
	 * \brief Restores a file item  from a backup
	 * \param manifest the target backup manifest
	 * \param inItem the file item to be extracted from teh backup 
	 * \param inRestoredFilePath the full path where the item should be restored
	 * \param inProgress the operation progress object where extraction progress is notified
	 * \param inOverwrite if @c inRestoredFilePath leads to an existing object, whether to remove it or fail to extract
	 * \return true for success or false otherwise
	 * \throws VE_INVALID_PARAMETER if @c inItem is actually a folder and not a file
	 * \throws VE_FILE_NOT_FOUND if @c inItem cannot be retrieved from the backup archive
	 * \throws any file-related error code in case of error
	 */
	bool _RestoreFile(const VBackupManifest& manifest,const IBackupTool::BackupItem& inItem,const XBOX::VFilePath& inRestoredFilePath,IDB4D_DataToolsIntf* inProgress,bool inOverwrite);

	
	/**
	 * \brief Restores a file item  from a backup
	 * \param inPathOfFileToRestore path of the file to be restored
	 * \param inRestoreFolder path of the folder where the file will be restored  
	 * \param outFormerFilePath path if @c inPathOfFileToRestore exists in @ inRestoreFolder before restoration, then it will be renamed and the resuting name
	 * is stored in that parameter
	 * \param inProgress the operation progress object where extraction progress is notified
	 * \return true for success or false otherwise
	 * \throws VE_INVALID_PARAMETER if @c inItem is actually a folder and not a file
	 * \throws VE_FILE_NOT_FOUND if @c inItem cannot be retrieved from the backup archive
	 * \throws any file-related error code in case of error
	 */
	bool _RestoreFile(const XBOX::VFilePath& inPathOfFileToRestore,const XBOX::VFilePath& inRestoreFolder,XBOX::VFilePath& outFormerFilePath,IDB4D_DataToolsIntf* inProgress);

	
	/**
	 * \brief Restores a folder item  from a backup archive
	 * \param manifest the target backup manifest
	 * \param inPathOfFolderToRestore path of folder to restore in the backup "archive"
	 * \param inRestoredFolderParent the full path where the folder item will be restored
	 * \param outFormerFolderPath path if @c inPathOfFolderToRestore exists in @ inRestoredFolderParent before extraction then it will be renamed and the resuting path
	 * is stored in that parameter
	 * \param inProgress the operation progress object where extraction progress is notified
	 * \return true for success or false otherwise
	 * \throws VE_INVALID_PARAMETER if @c inItem is actually a file and not a folder
	 * \throws VE_FILE_NOT_FOUND if @c inItem cannot be retrieved from the backup archive
	 * \throws any file-related error code in case of error
	 */
	bool _RestoreFolder(const XBOX::VFilePath& inPathOfFolderToRestore,const XBOX::VFilePath& inRestoredFolderParent,XBOX::VFilePath& outFormerFolderPath,IDB4D_DataToolsIntf* inProgress);


	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////

	/**
	 * \brief Writes the backup manifest into a file
	 * \param inManifestFolder the folder where the manifest will be stored
	 * \param inManifest the manifest to be serialized
	 * \param outManifestFilePath if non-null will contain the full path of the manifest file
	 */
	XBOX::VError _WriteManifest(const VFilePath& inManifestFolder, const VBackupManifest& inManifest, XBOX::VFilePath* outManifestFile);

	/**
	 * \Brief Writes a manifest into the backup manifest registry.
	 * \details The registry centralizes the last 20 backup manifests performed.
	 * Most recent manifests are stored last.
	 * \param inBackupSettings settings where the registry folder is contained
	 * \param inManifestPath the name of the manifest to be added to the registry
	 */
	XBOX::VError _WriteManifestToRegistry(const IBackupSettings& inBackupSettings, const VFilePath& inManifestPath, IDB4D_DataToolsIntf* inProgress);

	/**
	 * \brief Reads the manifest registry
	 * \param inBackupSettings settings to retrieve the registry folder from
	 * \param outRegistry contains the registry on success
	 * \return VE_OK if no error occured
	 * \return VE_FILE_NOT_FOUND if no regisry was found
	 */
	XBOX::VError _WriteManifestRegistry(const IBackupSettings& inBackupSettings, const XBOX::VectorOfVString& inRegistry);
	
	/**
	 * \brief Reads the manifest registry
	 * \param inBackupSettings settings to retrieve the registry folder from
	 * \param outRegistry contains the registry on success
	 * \return VE_OK if no error occured
	 * \return VE_FILE_NOT_FOUND if no regisry was found
	 */
	 XBOX::VError _ReadManifestRegistry(const IBackupSettings& inBackupSettings,XBOX::VectorOfVString& outRegistry);
};

/**
 * \brief Journal utility class
 * \details This class gathers several utility methods to parse and convert DB4D journal file to a couple of formats
 */
class VJournalTool: public XBOX::VObject, public IJournalTool
{
public:
	VJournalTool();
	virtual ~VJournalTool();

	virtual XBOX::VError Initialize(const XBOX::VFile& inTableDefFile);
	/**
	 * \brief Opens the specified journal file and converts it into a JS object.
	 * \param inJournalFile the journal to convert
	 * \param outOperations the resulting array where each operation is stored as an entry
	 * \return true on success
	 */
	virtual bool ParseJournal(XBOX::VFile& inJournalFile, XBOX::VJSONArray& outOperations, IDB4D_DataToolsIntf* inProgressLog);

	/**
	 * \brief Opens a journal file and converts it into a text file
	 * \param inJournalFile the journal to convert
	 * \param inDestinationFile the resulting text file
	 * \return true on success
	 */
	virtual bool ParseJournalToText(XBOX::VFile& inJournalFile,XBOX::VFile& inDestinationFile,IDB4D_DataToolsIntf* inProgressLog);

	/**
	 * \brief Retrieves journal information from a data file
	 * \param inDataFile the data file to read from
	 * \param outJournalPath the resulting journal path or empty if no journal was found
	 * \param outDataLink the matching journal UUID
	 */
	virtual XBOX::VError ReadJournalInfoFromDataFile(XBOX::VFile& inDataFile,XBOX::VFilePath& outJournalPath,XBOX::VUUID& outDataLink);
	
	/**
	 * \brief Resets a data file's journal by truncating all operations and writing a new header
	 */
	virtual XBOX::VError ResetDataFileJournal(XBOX::VFile& inDataFile, XBOX::VFilePath& outFormerJournalPath, XBOX::VFilePath& outCurrentJournalPath);

protected:

	bool _TableNameFromUUID(const XBOX::VUUID& inTableId, XBOX::VString& outName);
	
	///Converts a journal operation into a VJSONObject
	XBOX::VJSONObject* _ConvertToObject(CDB4DJournalData& injournalOperation)const;

private:
	std::map<XBOX::VUUID, XBOX::VString> fMapTableUUidToName;

};

#endif //__DB_BACKUP__

