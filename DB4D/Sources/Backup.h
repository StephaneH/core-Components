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

#include "DB4D.h"

/**
 * \brief Describes how a backup must performed
 */

const OsType kBACKUP_SETTINGS = 'bkps';

DECLARE_VERROR( kBACKUP_SETTINGS, 1, VE_INVALID_BACKUP_FOLDER) //the folder specification passed for backup is invalid
DECLARE_VERROR( kBACKUP_SETTINGS, 2, VE_CANNOT_CREATE_BACKUP_FOLDER) //the backup folder cannot be created
DECLARE_VERROR( kBACKUP_SETTINGS, 3, VE_CANNOT_CREATE_JOURNAL_BACKUP)// the database journal cannot be backed up
DECLARE_VERROR( kBACKUP_SETTINGS, 4, VE_CANNOT_CREATE_ITEM_BACKUP)//one of the items to back up could not be backed up
DECLARE_VERROR( kBACKUP_SETTINGS, 5, VE_CANNOT_RESTORE_JOURNAL)//journal could not be restored after a failed backup


namespace JournalUtils
{
	VError DisableJournal(Base4D* inBase,CDB4DBaseContext* inContext);
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

	void ConvertToJsObject(VJSObject& outObject)const;

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
	virtual bool Initialize(const VValueBag& inBag,const VFilePath& inBaseFolder);
	
	/**
	 * \brief Initializes/resets the instance using a JSON object
	 * \details this method can be called using only a subset of initializations keys enabling to partially reset the instance
	 * and preserve some values.
	 * \param inObject the JS object carrying the init values.
	 * \return false if the final object is invalid state after the new values have been taken into account
	 * \return true if the object is valid after the new values have been taken into account
	 * \see CheckValidity()
	 */
	virtual bool FromJSObject(const XBOX::VJSObject& inObject);

	///Sets the parent for this instance
	virtual bool SetRetainedParent(const IBackupSettings* inParent );
	
	virtual const IBackupSettings* RetainParent();

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

	///Checks this instance validity
	virtual VError CheckValidity()const;

	///Converts this object to its JSON representation string
	virtual void ToJSONString(XBOX::VString& outJSON)const;

	///Converts this instance into a JS object
	void ToJSObject(XBOX::VJSObject& outObject)const;

private:
	const IBackupSettings*	fParent;
	XBOX::VFilePath		fBackupRegistryFolder;
	XBOX::VFilePath		fDestinationFolderPath;
	bool				fUniqueNames;
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
	 * \param inContext not sure what this stuff is for since the database is suppoed to be closed
	 * \param inBackupSettings settings to use to perform the backup
	 * \param outManifestFilePath optional, if non NULL will contain the backup manifest file path
	 * \param inBackupEventLog optional, if non NULL will receive progress indications
	 * \return true if the backup went fine
	 * \return false if an error occured in the process
	 */
	virtual bool BackupClosedDatabase(XBOX::VFile& inModelFile,XBOX::VFile& inDataFile,CDB4DBaseContext* inContext,const IBackupSettings& inBackupSettings,XBOX::VFilePath* outManifestFilePath,IDB4D_DataToolsIntf* inBackupEventLog);
	
	/**
	 * \brief Backs up a database without applying journaling-policy changes.
	 * \param inBase the database to backup
	 * \param inJournalToArchive an optional path to the journal to archive in this backup
	 * \param inBackupSettings the backup configuration to use for this backup
	 * \param outManifestFilePath the resulting backup manifest fullpath
	 * \return true if the backup went fine
	 * \return false if an error occured in the process
	 */
	virtual bool BackupDatabaseAndDontResetJournal(CDB4DBase* inBase,const XBOX::VFilePath& inJournalToArchive,const IBackupSettings& inBackupSettings,XBOX::VFilePath* outManifestFilePath,IDB4D_DataToolsIntf* inBackupEventLog);

	virtual XBOX::VError GetLastBackupPath(const IBackupSettings& inBackupSettings,XBOX::VFilePath& outLastBackupLocation);
	
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
	virtual bool RestoreDataFolder(const XBOX::VFilePath& inBackupManifestPath,const XBOX::VFilePath& inRestoredDataFolderParentPath,XBOX::VFilePath& outRenamedDataFolderPath,IDB4D_DataToolsIntf* inProgress);

protected:
	/**
	 * \brief Backs up the database and sets the specified new journal
	 * \param inNextJournalFile if NULL then journaling is disabled after backup is performed
	 */
	bool _DoBackup(Base4D* inBase,CDB4DBaseContext* inContext,const IBackupSettings& inBackupConfig,const XBOX::VFilePath* inNextJournalFile,XBOX::VFilePath* outManifestFile,IDB4D_DataToolsIntf* inBackupEventLog);

	bool _DoBackup(Base4D* inBase,CDB4DBaseContext* inContext,const VFilePath& inJournaltoBackup,const IBackupSettings& inBackupConfig,XBOX::VFilePath* outManifestFile,IDB4D_DataToolsIntf* inBackupEventLog);

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
	XBOX::VError _WriteManifest(const VFilePath& inManifestFolder,const VBackupManifest& inManifest,XBOX::VFilePath* outManifestFile);

	/**
	 * \Brief Writes a manifest into the backup manifest registry.
	 * \details The registry centralizes the last 20 backup manifests performed.
	 * Most recent manifests are stored last.
	 * \param inBackupSettings settings where the registry folder is contained
	 * \param inManifestPath the name of the manifest to be added to the registry
	 */
	XBOX::VError _WriteManifestToRegistry(const IBackupSettings& inBackupSettings,const VFilePath& inManifestPath);

	/**
	 * \brief Reads the manifest registry
	 * \param inBackupSettings settings to retrieve the registry folder from
	 * \param outRegistry contains the registry on success
	 * \return VE_OK if no error occured
	 * \return VE_FILE_NOT_FOUND if no regisry was found
	 */
	XBOX::VError _WriteManifestRegistry(const IBackupSettings& inBackupSettings,const XBOX::VectorOfVString& inRegistry);
	
	/**
	 * \brief Reads the manifest registry
	 * \param inBackupSettings settings to retrieve the registry folder from
	 * \param outRegistry contains the registry on success
	 * \return VE_OK if no error occured
	 * \return VE_FILE_NOT_FOUND if no regisry was found
	 */
	 XBOX::VError _ReadManifestRegistry(const IBackupSettings& inBackupSettings,XBOX::VectorOfVString& outRegistry);

};
#endif //__DB_BACKUP__

