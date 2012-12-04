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
#include "Backup.h"
#include "InfoTask.h"

USING_TOOLBOX_NAMESPACE

//////////////////////////////////////////////////////////////////////////
///////////////////  H E L P E R S   F U N C T I O N S  //////////////////
//////////////////////////////////////////////////////////////////////////

namespace Helpers
{
	XBOX::VError SetDatabaseJournalEnabled(Base4D* inBase,CDB4DBaseContext* inContext,bool inEnableJournal);
	bool ComputeUniqueFolderName(VFilePath& inoutFolder,const VString& suffix);
	VError ComputeTotalSizeToBackup(const VFolder& inBackedUpFolder,sLONG8& outTotalBytes);
	VError CopyFolderWithProgress(const VFolder& inSourceFolder,VFolder& inDestinationFolder,bool inOverwriteExisting,IDB4D_DataToolsIntf* inProgressIndicator,sLONG8 inTotalBytesToCopy,sLONG8& inTotalBytesCopied,
		const VectorOfVString* inExceptFilesByExt);
	void ComputeTimeStamp(const XBOX::VTime* inTime,XBOX::VString& outTimeStamp,bool inUseMs = false);
	
	/**
	 * \brief Extra cautious method to swap the database journal
	 * \details will handle parent folder path creation etc.
	 */
	VError SwapDatabaseJournal(Base4D& inBase,const VFile* inPreviousJournal,VFile& inNextJournal,XBOX::VUUID& ioDataLink);

	/**
	 * \brief Extra cautious method to set a new file as database
	 * \details will handle parent folder path creation etc.
	 */
	VError InstallNewDatabaseJournal(Base4D& inBase,VFile& inNextJournal,XBOX::VUUID& ioDataLink);

}


//////////////////////////////////////////////////////////////////////////
//////////////////////  J O U R N A L    U T I L S  //////////////////////
//////////////////////////////////////////////////////////////////////////

namespace JournalUtils
{
	VError DisableJournal(Base4D* inBase,CDB4DBaseContext* inContext)
	{
		VError error = VE_OK;
		/*error =*/ Helpers::SetDatabaseJournalEnabled(inBase,inContext,false);
		inBase->SetJournalFile(NULL);
		return error;
	}
}



//////////////////////////////////////////////////////////////////////////
//////////////////////  V B A C K U P  M A N I F E S T  //////////////////
//////////////////////////////////////////////////////////////////////////

VBackupManifest::VBackupManifest():XBOX::VObject()
{
	XBOX::VTime::Now(fDate);
}
VBackupManifest::~VBackupManifest()
{
}

XBOX::VError VBackupManifest::ToStream(XBOX::VStream& inStream)const
{
	XBOX::VString json;
	ToJSONString(json);
	return inStream.PutText(json);
}

XBOX::VError VBackupManifest::FromStream(XBOX::VStream& inStream)
{
	XBOX::VError error = VE_OK;
	XBOX::VString json;
	inStream.GetText(json);
	error = FromJSONString(json);
	return error;
}

XBOX::VError VBackupManifest::FromJSONString(const XBOX::VString& inJsonString)
{
	XBOX::VJSONImporter parser(inJsonString);
	XBOX::VError error = VE_OK;
	VJSONValue value;

	error = parser.Parse(value);
	if(!value.IsObject() || error != VE_OK)
	{
		error = VE_INVALID_PARAMETER;
	}
	else
	{
		VJSONObject* obj = value.GetObject();
		if(obj == NULL)
		{
			error = VE_INVALID_PARAMETER;
		}
		else
		{
			VString stringValue;
			
			fDataFolderPath.Clear();
			VJSONValue value = obj->GetProperty(CVSTR("dataFolder"));
			if(value.IsString())
			{
				value.GetString(stringValue);
				fDataFolderPath.FromFullPath(stringValue,FPS_POSIX);
			}
			
			fJournalPath.Clear();
			value = obj->GetProperty(CVSTR("journal"));
			if(value.IsString())
			{
				value.GetString(stringValue);
				fJournalPath.FromFullPath(stringValue, FPS_POSIX);
			}
			
			value = obj->GetProperty(CVSTR("date"));
			if(value.IsString())
			{
				value.GetString(stringValue);
				fDate.FromXMLString(stringValue);
			}
		}
	}
	return error;
}

void VBackupManifest::ConvertToJsObject(VJSObject& outObject)const
{
	XBOX::VString string;

	if (fDataFolderPath.IsFolder() && fDataFolderPath.IsValid())
	{
		fDataFolderPath.GetPosixPath(string);
	}
	else
	{
		string.Clear();
	}
	outObject.SetProperty(CVSTR("dataFolder"),string);

	if (fJournalPath.IsValid() && fJournalPath.IsFile())
	{
		fJournalPath.GetPosixPath(string);
	}
	else
	{
		string.Clear();
		
	}
	outObject.SetProperty(CVSTR("journal"),string);
	
	fDate.GetXMLString(string,XSO_Time_NoTimezone);
	outObject.SetProperty(CVSTR("date"),string);
	
	string.Clear();
	VProcess* currentProcess = XBOX::VProcess::Get();
	if (currentProcess)
	{
		currentProcess->GetBuildNumber(string);
	}
	if(string.IsEmpty())
		string.FromCString("none");
	outObject.SetProperty(CVSTR("version"),string);
}


void VBackupManifest::ToJSONString(XBOX::VString& outJsonString)const
{
	XBOX::VJSONSingleObjectWriter jsonWriter;
	XBOX::VString string;

	uLONG jsonOpts = JSON_WithQuotesIfNecessary|JSON_PrettyFormatting;

	if (fDataFolderPath.IsValid() && fDataFolderPath.IsFolder())
	{
		fDataFolderPath.GetPosixPath(string);
	}
	else
	{
		string.Clear();
	}
	jsonWriter.AddMember(CVSTR("dataFolder"),string,jsonOpts);
	
	if (fJournalPath.IsValid() && fJournalPath.IsFile())
	{
		fJournalPath.GetPosixPath(string);
	}
	else
	{
		string.Clear();
	}
	jsonWriter.AddMember(CVSTR("journal"),string,jsonOpts);

	string.Clear();
	fDate.GetXMLString(string,XSO_Time_NoTimezone);
	jsonWriter.AddMember(CVSTR("date"),string,jsonOpts);

	string.Clear();
	VProcess* currentProcess = XBOX::VProcess::Get();
	if (currentProcess)
	{
		currentProcess->GetBuildNumber(string);
	}
	if(string.IsEmpty())
		string.FromCString("none");
	jsonWriter.AddMember(CVSTR("version"),string),jsonOpts;

	outJsonString.Clear();
	jsonWriter.GetObject(outJsonString);

}


//////////////////////////////////////////////////////////////////////////
//////////////////////  V B A C K U P  S E T T I N G S  //////////////////
//////////////////////////////////////////////////////////////////////////

VBackupSettings::VBackupSettings():
	VObject(),
	IBackupSettings(),
	fParent(NULL),
	fUniqueNames(false)
{

}

namespace BackupSettingsKeys
{
	CREATE_BAGKEY(destination);
	CREATE_BAGKEY_WITH_DEFAULT(uniqueNames,bool,false);
	
};


VBackupSettings::~VBackupSettings()
{
	XBOX::ReleaseRefCountable(&fParent);
}

bool VBackupSettings::Initialize(const VValueBag& inBag,const VFilePath& inBaseFolder)
{
	XBOX::VString temp;
	bool valid = false;
	if (inBag.GetString(BackupSettingsKeys::destination,temp))
	{
		if (temp.BeginsWith( CVSTR( "./")) || temp.BeginsWith( CVSTR( "../")))
		{
			fDestinationFolderPath.FromRelativePath( inBaseFolder, temp, FPS_POSIX);
			valid = true;
		}
		else
		{
			VFilePath path;
			VURL url(temp,false);
			if (url.GetFilePath( path))
			{
				if (path.IsFolder())
				{
					fDestinationFolderPath = path;
					valid = true;
				}
			}
		}
	}
	return valid;
}

bool VBackupSettings::FromJSObject(const XBOX::VJSObject& inObject)
{
	VError error = VE_OK;
	if (inObject.HasProperty("destination"))
	{
		fDestinationFolderPath.Clear();
		VFolder *folder = NULL;
		VJSValue value = inObject.GetProperty( "destination");
		if (value.IsObject())
		{
			folder = value.GetFolder();
			if (folder)
			{
				folder->GetPath(fDestinationFolderPath);
			}
		}
		if (folder == NULL)
		{
			error = VE_INVALID_PARAMETER;
			vThrowError(error,CVSTR("'destination' must be a folder"));
		}
		folder = NULL;//obtained from GetFolder() so no need to release
	}
	if (inObject.HasProperty("backupRegistryFolder"))
	{
		VFolder *folder = NULL;
		VJSValue value = inObject.GetProperty( "backupRegistryFolder");
		fBackupRegistryFolder.Clear();
		if (value.IsObject())
		{
			folder = value.GetFolder();
			if (folder)
			{
				folder->GetPath(fBackupRegistryFolder);
			}
		}
		if (folder == NULL)
		{
			error = VE_INVALID_PARAMETER;
			vThrowError(error,CVSTR("'backupRegistryFolder' must be a folder"));
		}
		folder = NULL;//obtained from GetFolder() so no need to release

	}
	if (inObject.HasProperty("useUniqueNames"))
	{
		VJSValue value = inObject.GetProperty( "useUniqueNames");
		if (value.IsBoolean())
		{
			value.GetBool(&fUniqueNames);
		}
		else
		{
			error = VE_INVALID_PARAMETER;
			vThrowError(error,CVSTR("'useUniqueNames' must be a bool"));
		}
	}
	if(error== VE_OK)
	{
		error = CheckValidity();
	}
	return (error == VE_OK);
}

bool VBackupSettings::SetRetainedParent(const IBackupSettings* inParent )
{
	if (inParent == NULL)
	{
		ReleaseRefCountable(&fParent);
	}
	else
	{
		CopyRefCountable(&fParent,inParent);
	}
	return true;
}

const IBackupSettings* VBackupSettings::RetainParent()
{
	return RetainRefCountable(fParent);
}

bool VBackupSettings::SetBackupRegistryFolder(const XBOX::VFilePath& inBackupRegistryFolder)
{
	XBOX::VError error = VE_FOLDER_CANNOT_CREATE;
	if (inBackupRegistryFolder.IsValid() && !inBackupRegistryFolder.IsEmpty() && inBackupRegistryFolder.IsFolder())
	{
		XBOX::VFolder fld(inBackupRegistryFolder);
		if (!fld.Exists())
		{
			error = fld.CreateRecursive();
		}
		else
		{
			error = VE_OK;
		}
		if (error == VE_OK)
		{
			fBackupRegistryFolder = inBackupRegistryFolder;
		}
	}
	return (error == VE_OK);
}

bool VBackupSettings::GetBackupRegistryFolder(XBOX::VFilePath& outBackupRegistryFolder)const
{
	bool ok = true;
	if (!fBackupRegistryFolder.IsEmpty())
		outBackupRegistryFolder = fBackupRegistryFolder;
	else if (fParent)
		ok = fParent->GetBackupRegistryFolder(outBackupRegistryFolder);
	else
		ok = false;
	ok = (!outBackupRegistryFolder.IsEmpty() && outBackupRegistryFolder.IsValid() && outBackupRegistryFolder.IsFolder());
	return ok;
}

void VBackupSettings::SetDestination(const XBOX::VFilePath& inBackupFolder)
{
	xbox_assert(inBackupFolder.IsValid() && inBackupFolder.IsFolder());
	if (inBackupFolder.IsValid() && inBackupFolder.IsFolder())
	{
		fDestinationFolderPath.FromFilePath(inBackupFolder);
	}
}
	
bool VBackupSettings::GetDestination(XBOX::VFilePath& outBackupFolder)const
{
	bool ok = true;
	if(!fDestinationFolderPath.IsEmpty())
		outBackupFolder = fDestinationFolderPath;
	else if (fParent)
		ok = fParent->GetDestination(outBackupFolder);
	else 
		ok = false;
	return ok;
}

bool VBackupSettings::GetUseUniqueNames()const
{
	return  fUniqueNames;
}

void VBackupSettings::SetUseUniqueNames(bool inUseUniqueNames)
{
	fUniqueNames = inUseUniqueNames;
}

VError VBackupSettings::CheckValidity()const
{
	bool valid = true;
	XBOX::VError error = VE_OK;
	VFilePath path;

	GetBackupRegistryFolder(path);
	if (path.IsEmpty() || !path.IsFolder() || !path.IsValid())
	{
		error = VE_INVALID_BACKUP_FOLDER;
	}

	if(error == VE_OK)
	{
		GetDestination(path);
		if (path.IsEmpty() || !path.IsFolder() || !path.IsValid())
		{
			error = VE_INVALID_BACKUP_FOLDER;
		}
	}
	return error;
}

///Converts this instance into a JS object
void VBackupSettings::ToJSObject(XBOX::VJSObject& outObject)const 
{
	XBOX::VString string;
	VFilePath filePath;

	GetDestination(filePath);
	filePath.GetPosixPath(string);
	outObject.SetProperty(CVSTR("destination"),string);

	if (GetUseUniqueNames())
	{
		outObject.SetProperty(CVSTR("useUniqueNames"),true);
	}
	else
	{
		outObject.SetProperty(CVSTR("useUniqueNames"),false);
	}

	GetBackupRegistryFolder(filePath);
	filePath.GetPosixPath(string);
	outObject.SetProperty(CVSTR("backupRegistryFolder"),string);
}


void VBackupSettings::ToJSONString(XBOX::VString& outJSONString)const
{
	XBOX::VJSONSingleObjectWriter jsonWriter;
	XBOX::VString string;
	VFilePath filePath;

	uLONG jsonOpts = JSON_WithQuotesIfNecessary|JSON_PrettyFormatting;

	GetDestination(filePath);
	filePath.GetPosixPath(string);

	jsonWriter.AddMember(CVSTR("destination"),string,jsonOpts);

	VBoolean useUniqueNames(false);
	if (GetUseUniqueNames())
	{
		useUniqueNames = true;
	}

	jsonWriter.AddMember(CVSTR("useUniqueNames"),useUniqueNames,jsonOpts);

	GetBackupRegistryFolder(filePath);
	filePath.GetPosixPath(string);
	jsonWriter.AddMember(CVSTR("backupRegistryFolder"),string,jsonOpts);

	outJSONString.Clear();
	jsonWriter.GetObject(outJSONString);
}

//using namespace Helpers;

//////////////////////////////////////////////////////////////////////////
//////////////////////      V B A C K U P T O O L       //////////////////
//////////////////////////////////////////////////////////////////////////


VBackupTool::VBackupTool():
	VObject(), 
	IBackupTool()
{
}

VBackupTool::~VBackupTool()
{
}

bool VBackupTool::ConvertBackupRegistryToJsObject(const XBOX::VFilePath& inBackupRegistryFilePath,XBOX::VJSArray& outManifests)
{
	XBOX::VError error = VE_OK;

	VFile registryFile(inBackupRegistryFilePath);
	if (!registryFile.Exists())
	{
		error = VE_FILE_NOT_FOUND;
	}
	else
	{
		VFileStream manifestRegistryStream(&registryFile);
		error = manifestRegistryStream.OpenReading();
		if (error == VE_OK)
		{
			VString registryContent;
			if (manifestRegistryStream.GetText(registryContent) == VE_OK)
			{
				VJSONArrayReader reader(registryContent);
				const VectorOfVString& items = reader.GetStringsValues();
				VectorOfVString::const_iterator it = items.begin();

				for(;it != items.end();++it)
				{
					VFilePath itemPath;
					VString itemPathString;
					itemPath.FromFullPath(*it,FPS_POSIX);

					itemPath.GetPosixPath(itemPathString);
					{
						VFile manifestFile(itemPath);
						VFileStream manifestStream(&manifestFile);

						if(manifestStream.OpenReading() == VE_OK)
						{
							VJSObject manifObject(outManifests.GetContextRef());
							VBackupManifest manifest;

							manifest.FromStream(manifestStream);
							manifObject.MakeEmpty();
							
							manifObject.SetProperty(CVSTR("path"),itemPathString);
							manifest.ConvertToJsObject(manifObject);
							
							outManifests.PushValue(manifObject);
						}
						manifestStream.CloseReading();
					}
				}
			}
		}
	}
	return (error == VE_OK);
}

bool VBackupTool::BackupDatabaseAndDontResetJournal(CDB4DBase* inBase,const XBOX::VFilePath& inJournalToArchive,const IBackupSettings& inBackupSettings,XBOX::VFilePath* outManifestFilePath,IDB4D_DataToolsIntf* inBackupEventLog)
{
	Base4D* base = NULL;
	CDB4DBaseContext* context = NULL;
	bool ok = false;
	base = VImpCreator<VDB4DBase>::GetImpObject(inBase)->GetBase();

	context = inBase->NewContext(nil, kJSContextCreator);
	ok = _DoBackup(base,context,inJournalToArchive,inBackupSettings,outManifestFilePath,inBackupEventLog);
	context->Release();
	context = NULL;
	return ok;
}

bool VBackupTool::BackupDatabase(CDB4DBase* inBase,CDB4DBaseContext* inContext,const IBackupSettings& inBackupSettings,XBOX::VFilePath* outManifestFilePath,IDB4D_DataToolsIntf* inBackupEventLog)
{
	XBOX::VFilePath nextJournalFilePath;
	bool success = false;
	VFile* currentJournalFile = NULL;
	CDB4DBaseContext* context = NULL;

	context = inContext; 
	if (inContext == NULL)
	{
		context = inBase->NewContext(nil, kJSContextCreator);	
	}

	
	currentJournalFile = inBase->RetainJournalFile();
	
	if (currentJournalFile)
	{
		currentJournalFile->GetPath(nextJournalFilePath);
		XBOX::ReleaseRefCountable(&currentJournalFile);

		success = BackupDatabaseAndChangeJournal(inBase,inContext,inBackupSettings,&nextJournalFilePath,
			outManifestFilePath,
			inBackupEventLog);
	}
	else
	{
		success = BackupDatabaseAndChangeJournal(inBase,inContext,inBackupSettings,NULL,
			outManifestFilePath,
			inBackupEventLog);
	}
	if (inContext == NULL)
	{
		context->Release();
		context = NULL;
	}
	
	return success;
}

bool VBackupTool::BackupDatabaseAndChangeJournal(CDB4DBase* inBase,CDB4DBaseContext* inContext,const IBackupSettings& inBackupSettings,const XBOX::VFilePath* inNextJournalPath,XBOX::VFilePath* outManifestFilePath,IDB4D_DataToolsIntf* inBackupEventLog)
{
	Base4D* theBase4D = NULL;
	XBOX::VError error = VE_OK;
	bool ok = false;
	CDB4DBaseContext* context = NULL;
	theBase4D = VImpCreator<VDB4DBase>::GetImpObject(inBase)->GetBase();
	XBOX::VFilePath nextJournalPath;

	context = inContext; 
	if (inContext == NULL)
	{
		context = inBase->NewContext(nil, kJSContextCreator);	
	}

	//If a journal file path is specified
	if (inNextJournalPath)
	{
		if (!inNextJournalPath->IsValid() || !inNextJournalPath->IsFile())
		{
			error = VE_INVALID_PARAMETER;
			vThrowError(error);
		}
	}
	XBOX::VFilePath manifestFilePath;
	if(error == VE_OK)
	{
		ok = _DoBackup(theBase4D,context,inBackupSettings,inNextJournalPath,&manifestFilePath,inBackupEventLog);
	}

	//Write manifest to the manifest registry
	if(error == VE_OK && ok)
	{
		/*error =*/ _WriteManifestToRegistry( inBackupSettings,manifestFilePath);
	}

	if (outManifestFilePath)
	{
		*outManifestFilePath = manifestFilePath;
	}
	
	if (inContext == NULL)
	{
		context->Release();
		context = NULL;
	}
	return (ok && error == VE_OK);
}

bool VBackupTool::BackupClosedDatabase(XBOX::VFile& inModelFile,XBOX::VFile& inDataFile,CDB4DBaseContext* inContext,const IBackupSettings& inBackupConfig,XBOX::VFilePath* outManifestFilePath,IDB4D_DataToolsIntf* inBackupEventLog)
{
	VDBMgr* dbMgr = VDBMgr::GetManager();
	const VValueBag* extraProperties = NULL;
	bool wasOpened = false;
	VUUID dataLink;
	VError backupErr = VE_OK;
	uLONG8 versionNumber = 0;
	XBOX::VFile* journalFile = NULL;
	VBackupManifest backupManifest;
	XBOX::VFilePath currentJournalPath,manifestFilePath;

	
	//Retrieve journal infos from the extra properties
	backupErr = dbMgr->GetJournalInfo(inDataFile.GetPath(),currentJournalPath,dataLink);
	
	if(backupErr == VE_OK)
	{
		if(!currentJournalPath.IsEmpty() && currentJournalPath.IsValid() && currentJournalPath.IsFile())
		{
			journalFile = new VFile(currentJournalPath);
			xbox_assert(journalFile && journalFile->Exists());
		}
	}
	//Determine where everything will be backed up
	VFilePath dataFolderPath,backupFolderPath,backupFolderPathForDataFolder;
	inDataFile.GetPath(dataFolderPath);
	dataFolderPath.ToParent();
	VFolder dataFolder(dataFolderPath);

	//Check if backup folder in settings
	inBackupConfig.GetDestination(backupFolderPath);
	if (inBackupConfig.GetUseUniqueNames())
	{
		Helpers::ComputeUniqueFolderName(backupFolderPath,CVSTR("backup"));
	}

	backupFolderPathForDataFolder = backupFolderPath;
	XBOX::VString temp;
	dataFolderPath.GetName(temp);
	backupFolderPathForDataFolder.ToSubFolder(temp);
	VFolder backupFolder(backupFolderPath);
	VFolder dataFolderBackupFolder(backupFolderPathForDataFolder);


	XBOX::VFolder* foldersToCreate[] = {&backupFolder,&dataFolderBackupFolder,NULL};
	for(int i = 0;foldersToCreate[i] != NULL && backupErr == VE_OK;i++)
	{
		if(!foldersToCreate[i]->Exists())
		{
			backupErr = foldersToCreate[i]->CreateRecursive();
		}
	}
	if (backupErr != VE_OK)
	{
		vThrowError(backupErr,"Failed to create backup folder(s)");
	}

	//Step one: archive backup journal
	if( backupErr == VE_OK && journalFile != NULL)
	{
		XBOX::VFile* backupedLogFile = NULL;
		if (inBackupEventLog)
		{
			inBackupEventLog->OpenProgression(CVSTR("Backing up log file"),-1);
		}
		//backup journal to backup folder first
		backupErr = journalFile->CopyTo(backupFolder,&backupedLogFile,XBOX::FCP_Overwrite);
		if (backupErr == VE_OK)
		{
			XBOX::VFilePath path;
			backupedLogFile->GetPath(path);
			backupManifest.SetJournal(path);
		}
		else
		{
			//Define error describing that journal backup failed
			vThrowError(VE_CANNOT_CREATE_JOURNAL_BACKUP);
		}

		if(inBackupEventLog)
		{
			inBackupEventLog->CloseProgression();
		}
	}

	//Step 2: copy files from the data folder
	if( backupErr == VE_OK)
	{
		sLONG8 totalBytesToBackUp = 0, totalBytesCopied = 0;
		//Compute the actual size to be copied
		if(inBackupEventLog)
		{
			inBackupEventLog->OpenProgression(CVSTR("Estimating backup size"),-1);
		}
		if (Helpers::ComputeTotalSizeToBackup(dataFolder,totalBytesToBackUp) != VE_OK)
		{
			totalBytesToBackUp = -1;
		}
		if(inBackupEventLog)
		{
			inBackupEventLog->CloseProgression();
			inBackupEventLog->OpenProgression(CVSTR("Backing up database"),totalBytesToBackUp);
		}
		//copy the data folder, recursively
		backupManifest.SetDataFolder(backupFolderPathForDataFolder);

		VectorOfVString filter;
		filter.push_back(CVSTR("waJournal"));
		backupErr = Helpers::CopyFolderWithProgress(dataFolder,dataFolderBackupFolder,true,inBackupEventLog,totalBytesToBackUp,totalBytesCopied, &filter);
		//backupErr = dataFolder->CopyTo(backupFolder,&destSubFolder);
		if (backupErr != VE_OK)
		{
			vThrowError(VE_CANNOT_CREATE_ITEM_BACKUP);
		}
		else 
		{
			/*backupErr =*/ _WriteManifest(backupFolderPath,backupManifest,&manifestFilePath);
		}

		if(inBackupEventLog)
		{
			inBackupEventLog->CloseProgression();
		}
	}

	//Step 3: reset journal file if applicable
	if(backupErr == VE_OK  )
	{
		if (journalFile)
		{
			//We create a temporary journal file that we initialize using the
			//datalink from the extra props.
			//If all ends well then we swap the previous journal file and this one
			//If an error occured for some reason the previous journal is preserved and unmodified
			XBOX::VFilePath tempJournalPath = currentJournalPath;
			tempJournalPath.SetFileName(CVSTR("tempJournal"),false);
			XBOX::VString journalExtension;
			XBOX::VError error = VE_MEMORY_FULL;
			currentJournalPath.GetExtension(journalExtension);
			XBOX::VFile* tempJournalFile = new VFile(tempJournalPath);

			//Create a temp file where we dump a journal header
			if (tempJournalFile)
			{
				error = tempJournalFile->Create(FCR_Overwrite);
				if (error == VE_OK)
				{
					VFileStream *journalFileStream = new VFileStream(tempJournalFile);
					if (journalFileStream)
					{
						error = journalFileStream->OpenWriting();
						if (error == VE_OK )
						{
							DB4DJournalHeader journalHeader;
							journalFileStream->SetBufferSize(0);
							journalHeader.Init( dataLink );
							error = journalHeader.WriteToStream(journalFileStream);
						}
						journalFileStream->CloseWriting();
					}
					else
					{
						error = VE_MEMORY_FULL;
					}
					delete journalFileStream; journalFileStream = NULL;
				}	
			}

			if(error == VE_OK)
			{
				//The temporary journal was successfully created and initialized, now swap it with the current journal
				XBOX::VFile* renamedOldJournalFile = NULL;
				XBOX::VString newName(CVSTR("oldJournal_")),timestamp;
				Helpers::ComputeTimeStamp(NULL,timestamp,true);

				newName.AppendString(timestamp);
				if (!journalExtension.IsEmpty())
				{
					newName.AppendChar('.');
					newName.AppendString(journalExtension);
				}
				error = journalFile->Rename(newName,&renamedOldJournalFile);
				if (error == VE_OK && renamedOldJournalFile)
				{
					currentJournalPath.GetFileName(newName,true);
					error = tempJournalFile->Rename(newName,NULL);
					if (error != VE_OK)
					{
						//The swap did not work then rollback on old journal file renaming
						error = renamedOldJournalFile->Rename(newName,NULL);
					}
					else
					{
						renamedOldJournalFile->Delete();
					}
				}
				XBOX::ReleaseRefCountable(&renamedOldJournalFile);
			}
			XBOX::ReleaseRefCountable(&tempJournalFile);

			backupErr = error;
		}
		/*backupErr =*/_WriteManifestToRegistry( inBackupConfig,manifestFilePath);
		if (outManifestFilePath)
		{
			*outManifestFilePath = manifestFilePath;
		}
	}
	XBOX::ReleaseRefCountable(&journalFile);

	return (backupErr == VE_OK);
}


bool VBackupTool::_DoBackup(Base4D* inBase,CDB4DBaseContext* inContext,const IBackupSettings& inBackupConfig,const XBOX::VFilePath* inNextJournalFilePath,XBOX::VFilePath* outManifestFilePath,IDB4D_DataToolsIntf* inBackupEventLog)
{
	VError backupErr = VE_OK;
	VFile* backedupLogFile = NULL;
	VFile* logFile = NULL;
	XBOX::VFilePath currentJournalFilePath;
	VFile* nextJournalFile = NULL;
	VFolder* dataFolder = NULL;
	VBackupManifest backupManifest;
	XBOX::VFilePath backupFolderPath;
	VString nextJournalPath;
	XBOX::VUUID backedUpJournalUUID;
	bool restoreFromOldJournalFileIfError = false;
	bool changedLogFile = false;
	dataFolder = inBase->RetainDataFolder();
	logFile = inBase->RetainJournalFile();

	if (logFile)
	{
		logFile->GetPath(currentJournalFilePath);
	}

	//Verify that the next journal param is sane
	if (inNextJournalFilePath != NULL )
	{
		if(!inNextJournalFilePath->IsValid() ||!inNextJournalFilePath->IsFile())
		{
			backupErr = VE_INVALID_PARAMETER;
		}
		else
		{
			nextJournalFile = new VFile(*inNextJournalFilePath);
		}
	}

	//Verify backup config sanity
	if (backupErr == VE_OK)
	{
		backupErr = inBackupConfig.CheckValidity();
	}

	if (backupErr == VE_OK)
	{
		//Check if backup folder in settings
		inBackupConfig.GetDestination(backupFolderPath);
		if (inBackupConfig.GetUseUniqueNames())
		{
			Helpers::ComputeUniqueFolderName(backupFolderPath,CVSTR("backup"));
		}
	
		XBOX::VFolder backupFolder(backupFolderPath);
		backupFolder.GetPath(backupFolderPath);

		XBOX::VFilePath backupFolderPathForDataFolder(backupFolderPath);
		{
			XBOX::VString dataFolderName;
			dataFolder->GetName(dataFolderName);
			backupFolderPathForDataFolder.ToSubFolder(dataFolderName);
		}
		
		XBOX::VFolder backupFolderForDataFolder(backupFolderPathForDataFolder);
		XBOX::VFolder* foldersToCreate[] = {&backupFolder,&backupFolderForDataFolder,NULL};
		for(int i = 0;foldersToCreate[i] != NULL && backupErr == VE_OK;i++)
		{
			if(!foldersToCreate[i]->Exists())
			{
				backupErr = foldersToCreate[i]->CreateRecursive();
			}
			//TODO: should we erase pre-existing content here ??
		}
		if(backupErr != VE_OK)
		{
			backupErr = VE_CANNOT_CREATE_BACKUP_FOLDER;
			vThrowError(backupErr,backupFolderPath.GetPath());
		}

		if (backupErr == VE_OK)
		{
			backupErr = _PrepareForBackup(inBase,inContext);
			if(backupErr != VE_OK)
			{
				vThrowError(backupErr);
			}

			//Step one: backup journal file , create a salvage
			if( backupErr == VE_OK && logFile )
			{
				if (inBackupEventLog)
				{
					inBackupEventLog->OpenProgression(CVSTR("Backing up log file"),-1);
				}

				//Disabe the current log file
				inBase->GetJournalUUIDLink( backedUpJournalUUID);
				inBase->SetJournalFile(NULL);
				//backup journal to backup folder first
				backupErr = logFile->Move(backupFolder,&backedupLogFile,XBOX::FCP_Overwrite);
				if (backupErr != VE_OK /*|| backedupLogFile == NULL*/)
				{
					restoreFromOldJournalFileIfError = true;
					//Define error describing that journal backup failed
					vThrowError(VE_CANNOT_CREATE_JOURNAL_BACKUP);
				}
				else
				{
					XBOX::VFilePath path;
					backedupLogFile->GetPath(path);
					backupManifest.SetJournal(path);
		
					restoreFromOldJournalFileIfError = (backedupLogFile != NULL);

					if (nextJournalFile)
					{
						//install journal file using same data link
						backupErr = Helpers::InstallNewDatabaseJournal(*inBase,*nextJournalFile,backedUpJournalUUID);
						changedLogFile = (backupErr == VE_OK);
						XBOX::ReleaseRefCountable( &nextJournalFile);
					}
				}
				if(inBackupEventLog)
				{
					inBackupEventLog->CloseProgression();
				}
			}

			//Special case where we need to swap the journal
			if ( !changedLogFile && ((nextJournalFile != NULL) && ( (logFile == NULL) //there was no journal before backup and one must be activated after backup
					|| !logFile->IsSameFile( nextJournalFile) ) ))// or the journal properties (name/location) have changed and must be applied after backup
			{

				XBOX::VUUID dataLink;

				backupErr = Helpers::SwapDatabaseJournal(*inBase,logFile,*nextJournalFile,dataLink);
				changedLogFile = ( backupErr == VE_OK);
			}

			//Step 2: activate journaling and backup the whole data folder and attachments
			if (backupErr == VE_OK)
			{
				sLONG8 totalBytesToBackUp = 0, totalBytesCopied = 0;
				
				if ( changedLogFile && (logFile == NULL) )
				{
					//Journal has been changed so make sure journaling is enabled for data & model
					Helpers::SetDatabaseJournalEnabled(inBase,inContext,true);
				}

				//Notify flush for backup in log
				VDBMgr::GetManager()->FlushCache(inBase, true);

				//Compute the actual size to be copied
				if(inBackupEventLog)
				{
					inBackupEventLog->OpenProgression(CVSTR("Estimating backup size"),-1);
				}
				if (Helpers::ComputeTotalSizeToBackup(*dataFolder,totalBytesToBackUp) != VE_OK)
				{
					totalBytesToBackUp = -1;
				}
				if(inBackupEventLog)
				{
					inBackupEventLog->CloseProgression();
				}

				if (inBackupEventLog)
				{
					inBackupEventLog->OpenProgression(CVSTR("Backing up database"),totalBytesToBackUp);
				}

				//copy the data folder, recursively
				VFolder* destSubFolder = NULL;
				backupManifest.SetDataFolder(backupFolderForDataFolder.GetPath());

				VectorOfVString filter;
				filter.push_back(CVSTR("waJournal"));
				backupErr = Helpers::CopyFolderWithProgress(*dataFolder,backupFolderForDataFolder,true,inBackupEventLog,totalBytesToBackUp,totalBytesCopied,&filter);

				if (backupErr != VE_OK)
				{
					vThrowError(VE_CANNOT_CREATE_ITEM_BACKUP);
				}
				else 
				{
					/*backupErr =*/ _WriteManifest(backupFolderPath,backupManifest,outManifestFilePath);
				}

				if(inBackupEventLog)
				{
					inBackupEventLog->CloseProgression();
				}
				XBOX::ReleaseRefCountable(&destSubFolder);
			}

			///Step 3: cleanup
			if (backupErr != VE_OK && restoreFromOldJournalFileIfError)
			{
				//In case of error restore old journal
				if (/*logFile &&*/ backedupLogFile)
				{
					//TODO: add logging here to indicate journal re-activation went fine
					testAssert(!backedUpJournalUUID.IsNull());
					inBase->SetJournalFile( NULL );

					backedupLogFile->CopyTo( currentJournalFilePath, NULL, XBOX::FCP_Overwrite );
					backupErr = inBase->OpenJournal( logFile, backedUpJournalUUID );
					if (backupErr != VE_OK)
						vThrowError(VE_CANNOT_RESTORE_JOURNAL);
					xbox_assert( backupErr == XBOX::VE_OK);
				}
			}
			_EndBackup(inBase,inContext,backedupLogFile);
		}
	}
	
	XBOX::ReleaseRefCountable(&backedupLogFile);
	XBOX::ReleaseRefCountable(&nextJournalFile);
	XBOX::ReleaseRefCountable(&logFile);
	XBOX::ReleaseRefCountable(&dataFolder);

	return (backupErr == VE_OK);
}

bool VBackupTool::_DoBackup(Base4D* inBase,CDB4DBaseContext* inContext,const VFilePath& inJournaltoBackup,const IBackupSettings& inBackupConfig,XBOX::VFilePath* outManifestFilePath,IDB4D_DataToolsIntf* inBackupEventLog)
{
	VError backupErr = VE_OK;
	VFile* logFile = NULL;
	VFolder* dataFolder = NULL;
	XBOX::VFilePath backupFolderPath;

	dataFolder = inBase->RetainDataFolder();
	logFile = inBase->RetainJournalFile();

	//Check settings are ok
	backupErr = inBackupConfig.CheckValidity();
	if (backupErr != VE_OK)
	{
		vThrowError(backupErr);
	}
	else
	{
		//Check if backup folder in settings
		inBackupConfig.GetDestination(backupFolderPath);
		Helpers::ComputeUniqueFolderName(backupFolderPath,CVSTR("backup"));
	
		XBOX::VFolder backupFolder(backupFolderPath);
		backupFolder.GetPath(backupFolderPath);

		XBOX::VFilePath backupFolderPathForDataFolder(backupFolderPath);
		backupFolderPathForDataFolder.ToSubFolder(CVSTR("Data"));
		XBOX::VFolder backupFolderForDataFolder(backupFolderPathForDataFolder);

		XBOX::VFolder* foldersToCreate[] = {&backupFolder,&backupFolderForDataFolder,NULL};
		for(int i = 0;foldersToCreate[i]!= NULL && backupErr == VE_OK;i++)
		{
			if(!foldersToCreate[i]->Exists())
			{
				backupErr = foldersToCreate[i]->CreateRecursive();
			}
		}

		if(backupErr != VE_OK)
		{
			backupErr = VE_CANNOT_CREATE_BACKUP_FOLDER;
			vThrowError(backupErr,backupFolderPath.GetPath());
		}

		if (backupErr == VE_OK)
		{
			backupErr = _PrepareForBackup(inBase,inContext);
			if(backupErr != VE_OK)
			{
				vThrowError(backupErr);
			}

			//Step one: backup journal file , create a salvage
			if (!inJournaltoBackup.IsEmpty())
			{
				xbox_assert(inJournaltoBackup.IsFile() && inJournaltoBackup.IsValid());
				if (inJournaltoBackup.IsFile()&& inJournaltoBackup.IsValid())
				{
					VFile fileToCopy(inJournaltoBackup),*copiedFile = NULL;
				
					if (inBackupEventLog)
					{
						inBackupEventLog->OpenProgression(CVSTR("Backing up log file"),-1);
					}

					//backup journal to backup folder first
					backupErr = fileToCopy.CopyTo(backupFolder,&copiedFile,XBOX::FCP_Overwrite);
					if (backupErr != VE_OK || copiedFile == NULL)
					{
						//Define error describing that journal backup failed
						vThrowError(VE_CANNOT_CREATE_JOURNAL_BACKUP);
					}
					XBOX::ReleaseRefCountable( &copiedFile);
					if(inBackupEventLog)
					{
						inBackupEventLog->CloseProgression();
					}
				}
			}

			//Step 2: copy the data folder and attachments to the backup folder
			if (backupErr == VE_OK)
			{
				sLONG8 totalBytesToBackUp = 0, totalBytesCopied = 0;
				
				//Notify flush for backup in log
				//XXXX: require VDB4DBase: Flush( true, false, eCLEntryKind_FlushFromBackup );
				VDBMgr::GetManager()->FlushCache(inBase, true);

				//Compute the actual size to be copied
				if(inBackupEventLog)
				{
					inBackupEventLog->OpenProgression(CVSTR("Estimating backup size"),-1);
				}
				if (Helpers::ComputeTotalSizeToBackup(*dataFolder,totalBytesToBackUp) != VE_OK)
				{
					totalBytesToBackUp = -1;
				}
				if(inBackupEventLog)
				{
					inBackupEventLog->CloseProgression();
				}

				if (inBackupEventLog)
				{
					inBackupEventLog->OpenProgression(CVSTR("Backing up database"),totalBytesToBackUp);
				}

				//copy the data folder, recursively, omitting the journal if it is in the data folder
				VectorOfVString filter;
				filter.push_back(CVSTR("waJournal"));
				backupErr = Helpers::CopyFolderWithProgress(*dataFolder,backupFolderForDataFolder,true,inBackupEventLog,totalBytesToBackUp,totalBytesCopied,&filter);
				if (backupErr != VE_OK)
				{
					vThrowError(VE_CANNOT_CREATE_ITEM_BACKUP);
				}
				if(inBackupEventLog)
				{
					inBackupEventLog->CloseProgression();
				}
			}
			//Backup complete
			_EndBackup(inBase,inContext,logFile);
		}
		
	}
	XBOX::ReleaseRefCountable(&logFile);
	XBOX::ReleaseRefCountable(&dataFolder);
	return (backupErr == VE_OK);
}


VError VBackupTool::_PrepareForBackup(Base4D* inBase,CDB4DBaseContext* inContext)
{
	//Slightly borrowed from VDB4DBase::_PrepareForBackup
	VError err = VE_OK;
	BaseTaskInfoPtr context = nil;
	if (inContext != nil)
	{
		context = ConvertContext(inContext);
		assert(context->GetBase() == inBase);
	}
	if (context == nil)
	{
		//err = ThrowBaseError(VE_DB4D_CONTEXT_IS_NULL, DBaction_StartingBackup);
		err = VE_DB4D_CONTEXT_IS_NULL;
	}
	else
	{
		context->SetTimeOutForOthersPendingLock(-1);
		if (inBase->FlushAndLock(context))
		{
			//XXX: On WAK DB4D_Log_StartBackup will be written to log but not in the datafile's last action field
			//consequently it will be impossible to integrate a log into this backup data file
			//so don't write DB4D_Log_StartBackup in the journal
			//This is not compatible with 4D
			//inBase->WriteLog(DB4D_Log_StartBackup, nil);
		}
		else
		{
			err = VE_DB4D_CANNOT_LOCK_BASE;
		}
	}
	return err;
}

VError VBackupTool::_EndBackup(Base4D* inBase,CDB4DBaseContext* inContext, VFile* inOldJournal)
{
	VError err = VE_OK;

	BaseTaskInfoPtr context = nil;
	if (inContext != nil)
	{
		context = ConvertContext(inContext);
		assert(context->GetBase() == inBase);
	}
	if (context == nil)
	{
		//err = ThrowBaseError(VE_DB4D_CONTEXT_IS_NULL, DBaction_FinishingBackup);
		err = VE_DB4D_CONTEXT_IS_NULL;
	}
	else
	{
		err = inBase->EndBackup(context, inOldJournal);
		inBase->UnLock(context);
	}
	return err;
}

XBOX::VError VBackupTool::_WriteManifestToRegistry(const IBackupSettings& inBackupSettings,const VFilePath& inManifestPath)
{
	XBOX::VError error = VE_OK;
	XBOX::VFilePath registryPath;

	VectorOfVString registryItems;
	error = _ReadManifestRegistry(inBackupSettings,registryItems);

	if(error == VE_OK ||error == VE_FILE_NOT_FOUND )
	{
		//Manifest are kept as most recent last
		VString value;
		inManifestPath.GetPosixPath(value);
		registryItems.push_back(value);
		
		//Keep only the last 20 entries
		const sLONG maxRegistryCapacity = 20;
		sLONG listSize = registryItems.size();
		if(listSize > maxRegistryCapacity)
		{
			VectorOfVString::iterator it = registryItems.begin();
			it += (listSize - maxRegistryCapacity);
			registryItems.erase(registryItems.begin(),it);
			xbox_assert(registryItems.size() == 20);
		}
		xbox_assert(registryItems.size() <= 20);
		error = _WriteManifestRegistry(inBackupSettings,registryItems);
	}
	return error;
}

bool  VBackupTool::_RestoreFolder(const VFilePath& inPathOfFolderToRestore,const XBOX::VFilePath& inRestoredFolderPath,XBOX::VFilePath& outFormerFolderPath,IDB4D_DataToolsIntf* inProgress)
{
	VError error = VE_OK;
	const VFilePath& sourcePath =  inPathOfFolderToRestore;

	if ( (error == VE_OK) && testAssert(sourcePath.IsValid() && sourcePath.IsFolder()))
	{
		VFolder* destFolder = NULL,*sourceFolder = NULL;
		VFilePath destFolderPath;
		VString suffix,name;

		destFolderPath = inRestoredFolderPath;

		sourcePath.GetFolderName(name);
		destFolderPath.ToSubFolder(name);

		//Check if destination exists

		destFolder = new VFolder(destFolderPath);
		if (destFolder->Exists())
		{
			//It does, rename it
			
			VFolder* renamedFolder = NULL;

			//Generate rename prefix.
			destFolderPath.GetFolderName(name);
			Helpers::ComputeTimeStamp(NULL,suffix,false);
			name.AppendString(CVSTR("_REPLACED_"));
			name.AppendString(suffix);
			error = destFolder->Rename(name,&renamedFolder);
			if (error == VE_OK)
			{
				renamedFolder->GetPath(outFormerFolderPath);
			}
			XBOX::ReleaseRefCountable(&renamedFolder);
		}
		else
		{
			//Create
			error = destFolder->CreateRecursive();
		}

		if(error == VE_OK)
		{
			sourceFolder = new VFolder(sourcePath);
			sLONG8 totalBytesToCopy = 0, totalBytesCopied = 0;
			if(inProgress)
			{
				inProgress->OpenProgression(CVSTR("Estimating size"),-1);
			}
			if (Helpers::ComputeTotalSizeToBackup(*sourceFolder,totalBytesToCopy) != VE_OK)
			{
				totalBytesToCopy = -1;
			}

			if(inProgress)
			{
				inProgress->CloseProgression();
				inProgress->OpenProgression(CVSTR("Copying folder"),totalBytesToCopy);
			}
			VectorOfVString filter;
			filter.push_back(CVSTR("waJournal"));
			error = Helpers::CopyFolderWithProgress(*sourceFolder,*destFolder,false,inProgress,totalBytesToCopy,totalBytesCopied,&filter);
			if(inProgress)
			{
				inProgress->CloseProgression();
			}
			error = vThrowError(error);
		}
		XBOX::ReleaseRefCountable(&destFolder);
		XBOX::ReleaseRefCountable(&sourceFolder);
	}
	return (error == VE_OK);
}

bool  VBackupTool::_RestoreFile(const VBackupManifest& inManifest,const IBackupTool::BackupItem& inItem,const XBOX::VFilePath& inRestoredFilePath,IDB4D_DataToolsIntf* inProgress,bool inOverwrite)
{
	VError error = VE_OK;
	VFilePath sourcePath; 

	switch (inItem)
	{
		case IBackupTool::eDataFile:
			sourcePath = inManifest.GetDataFolder();
			sourcePath.ToSubFile(CVSTR("data.waData"));
			break;
		case IBackupTool::eJournal:
			sourcePath = inManifest.GetJournal();
			break;
		default:
			error = vThrowError(VE_INVALID_PARAMETER);
			break;
	}

	if ( (error == VE_OK) && testAssert(sourcePath.IsValid() && sourcePath.IsFile()))
	{
		VFilePath parentPath;
		if(inRestoredFilePath.GetParent(parentPath))
		{
			VFolder parentFolder(parentPath);
			if(!parentFolder.Exists())
			{
				error = parentFolder.CreateRecursive();
			}
		}
		if(error == VE_OK)
		{
			VFile fileItem(sourcePath);
			if(testAssert(fileItem.Exists()))
			{
				error = VE_OK;
				XBOX::FileCopyOptions opts = XBOX::FCP_Default;
				if (inOverwrite)
					opts |=FCP_Overwrite;

				error = fileItem.CopyTo( inRestoredFilePath,NULL,opts);
			}
			else
			{
				error = vThrowError(VE_FILE_NOT_FOUND);
			}
		}
	}
	else if(error == VE_OK)
	{
		error = vThrowError(VE_FILE_NOT_FOUND);
	}
	return (error == VE_OK);
}


XBOX::VError VBackupTool::_WriteManifestRegistry(const IBackupSettings& inBackupSettings,const VectorOfVString& inRegistry)
{
	XBOX::VError error = VE_OK;
	XBOX::VFilePath registryPath;

	inBackupSettings.GetBackupRegistryFolder(registryPath);
	VFolder folder(registryPath);
	if (!folder.Exists())
	{
		error = folder.CreateRecursive();
	}
	if(error == VE_OK)
	{
		registryPath.SetFileName(CVSTR("lastBackup.json"),true);
		if(registryPath.IsEmpty() || !registryPath.IsValid() || !registryPath.IsFile())
		{
			error = VE_INVALID_PARAMETER;
		}
		else
		{
			XBOX::VFile registryFile(registryPath);
			XBOX::VFileStream stream(&registryFile,XBOX::FO_CreateIfNotFound|XBOX::FO_Overwrite);
			error = stream.OpenWriting();
			if(error == VE_OK)
			{
				VJSONArrayWriter writer;
				writer.AddStrings(inRegistry, JSON_WithQuotesIfNecessary | JSON_PrettyFormatting);
				writer.Close();
				const VString& jsonString = writer.GetArray();
				stream.SetCharSet(XBOX::VTC_UTF_8);
				error = stream.PutText(jsonString);
			}
			stream.CloseWriting();
		}
	}
	return error;
}

XBOX::VError VBackupTool::_ReadManifestRegistry(const IBackupSettings& inBackupSettings,VectorOfVString& outRegistry)
{
	XBOX::VError error = VE_OK;
	XBOX::VFilePath registryPath;

	outRegistry.clear();
	inBackupSettings.GetBackupRegistryFolder(registryPath);
	registryPath.SetFileName(CVSTR("lastBackup.json"),true);
	if(registryPath.IsEmpty() || !registryPath.IsValid() || !registryPath.IsFile())
	{
		error = VE_FILE_NOT_FOUND;
	}
	else
	{
		XBOX::VFile registryFile(registryPath);
		if (!registryFile.Exists())
		{
			error = VE_FILE_NOT_FOUND;
		}
		else
		{
			XBOX::VFileStream stream(&registryFile,XBOX::FO_Default);
			error = stream.OpenReading();
			if(error == VE_OK)
			{
				//Read 
				VString jsonString;
				VJSONArrayReader registryReader;
				stream.GetText(jsonString);
				registryReader.FromJSONString(jsonString);
				registryReader.ToArrayString(outRegistry);
			}
			stream.CloseReading();
		}
	}
	return error;
}
	
	
XBOX::VError VBackupTool::_WriteManifest(const VFilePath& inManifestFolder,const VBackupManifest& inManifest,XBOX::VFilePath* outManifestFile)
{
	VError error(VE_OK);
	XBOX::VFilePath manifestPath(inManifestFolder);

	manifestPath.SetFileName(CVSTR("backupManifest.json"),true);


	XBOX::VFile file(manifestPath);
	if (!file.Exists())
	{
		error = file.Create();
	}
	if(error == VE_OK)
	{
		XBOX::VFileStream stream(&file,XBOX::FO_Overwrite);
		error = stream.OpenWriting();
		if(error == VE_OK)
		{	
			stream.SetCharSet(XBOX::VTC_UTF_8);
			error = inManifest.ToStream(stream);
			stream.Flush();
			stream.CloseWriting();
		}
	}
	if(error == VE_OK && outManifestFile)
	{
		*outManifestFile = manifestPath;
	}
	return error;
}



XBOX::VError VBackupTool::GetLastBackupPath(const IBackupSettings& inBackupSettings,XBOX::VFilePath& outLastBackupLocation)
{
	VError error = VE_OK;
	VectorOfVString registry;
	outLastBackupLocation.Clear();
	error = _ReadManifestRegistry(inBackupSettings,registry);
	if(error == VE_OK)
	{
		error = VE_FILE_NOT_FOUND;
		if (!registry.empty())
		{
			outLastBackupLocation.FromFullPath(registry.back(),FPS_POSIX);
			if (testAssert(outLastBackupLocation.IsValid() && outLastBackupLocation.IsFile()))
			{
				error = VE_OK;
			}
		}
	}
	return error;
}

bool VBackupTool::RestoreDataFolder(const XBOX::VFilePath& inBackupManifestPath,const XBOX::VFilePath& inRestoredDataFolderPath,XBOX::VFilePath& outRenamedDataFolderPath,IDB4D_DataToolsIntf* inProgress)
{
	VBackupManifest manifest;
	VFile manifFile(inBackupManifestPath);
	VFileStream manifStream(&manifFile);
	VError error = VE_OK;
	StErrorContextInstaller errorContext; 
	bool success = false;

	error = manifStream.OpenReading();
	if( error != VE_OK) 
	{
		error = vThrowError(error);
	}
	else
	{
		error = manifest.FromStream(manifStream);
	}
	manifStream.CloseReading();
	if(error == VE_OK)
	{
		success = _RestoreFolder(manifest.GetDataFolder(),inRestoredDataFolderPath,outRenamedDataFolderPath,inProgress);
	}
	return (success && error == VE_OK);
}

bool VBackupTool::RestoreDataFile(const XBOX::VFilePath& inBackupManifestPath,const XBOX::VFilePath& inRestoredFilePath,IDB4D_DataToolsIntf* inProgress,bool inOverwrite)
{
	VBackupManifest manifest;
	VFile manifFile(inBackupManifestPath);
	VFileStream manifStream(&manifFile);
	VError error = VE_OK;
	StErrorContextInstaller errorContext; 
	bool success = false;

	error = manifStream.OpenReading();
	if( error != VE_OK) 
	{
		error = vThrowError(error);
	}
	else
	{
		error = manifest.FromStream(manifStream);
	}
	manifStream.CloseReading();
	if(error == VE_OK)
	{
		success = _RestoreFile(manifest,IBackupTool::eDataFile,inRestoredFilePath,inProgress,inOverwrite);
	}
	return (success && error == VE_OK);
}


namespace Helpers{
	
	XBOX::VError SetDatabaseJournalEnabled(Base4D* inBase,CDB4DBaseContext* inContext,bool inEnable)
	{
		XBOX::VError errExtraProperties = VE_OK;
		const XBOX::VValueBag* extraProperties = NULL;
		XBOX::VValueBag* newBag = NULL;

		extraProperties = inBase->RetainExtraProperties(errExtraProperties,inContext);
		if (extraProperties == NULL)
		{
			newBag = new XBOX::VValueBag();
		}
		else
		{
			newBag = extraProperties->Clone();
		}
		XBOX::ReleaseRefCountable(&extraProperties);

		if (errExtraProperties == VE_OK && newBag)
		{
			XBOX::VValueBag *journalBag = newBag->RetainUniqueElement( DB4DBagKeys::journal_file );
			if ( !journalBag )
				journalBag = new XBOX::VValueBag();

			if ( journalBag != NULL )
			{
				journalBag->SetBool( DB4DBagKeys::journal_file_enabled, inEnable );
				newBag->ReplaceElement( DB4DBagKeys::journal_file, journalBag );
				inBase->SetExtraProperties( newBag,true,inContext );
			}
			XBOX::ReleaseRefCountable(&journalBag);
		}
		XBOX::ReleaseRefCountable(&newBag);
		return errExtraProperties;
	}


void ComputeTimeStamp(const XBOX::VTime* inTime,XBOX::VString& outTimeStamp,bool inUseMs)
{
	sWORD year,month,day,hour,min,sec,msec,temp;
	if (inTime)
	{
		inTime->GetLocalTime(year,month,day,hour,min,sec,msec);
	}
	else
	{
		XBOX::VTime now(eInitWithCurrentTime);
		now.GetLocalTime(year,month,day,hour,min,sec,msec);
	}
	outTimeStamp.Clear();
	if (!inUseMs)
		outTimeStamp.AppendPrintf("%04d-%02d-%02d_%02d-%02d-%02d",year,month,day,hour,min,sec);
	else
		outTimeStamp.AppendPrintf("%04d-%02d-%02d_%02d-%02d-%02d-%03d",year,month,day,hour,min,sec,msec);
}

bool ComputeUniqueFolderName(VFilePath& inoutFolder,const VString& suffix)
{
	VFilePath folder(inoutFolder);
	XBOX::VString base,ts;
	bool done = false;

	ComputeTimeStamp(NULL,ts);

	xbox_assert(!inoutFolder.IsEmpty() &&inoutFolder.IsFolder());

	for(int i = 0;i <500 && !done;i++)
	{
		base.Clear();
		if (suffix.GetLength() > 0)
		{
			base.AppendString(suffix);
		}
		base.AppendChar('_');
		base.AppendString(ts);
		if(i>0)
		{
			base.AppendPrintf("_%03d",i);
		}
		folder.ToSubFolder(base);
		{
			VFolder test(folder);
			done = !(test.Exists());
			if (!done)
			{
				folder.ToParent();
			}
			else
			{
				inoutFolder = folder;
			}
		}
	}
	return done;
}

VError ComputeTotalSizeToBackup(const VFolder& inSourceFolder,sLONG8& outTotalBytes)
{
	VError err = VE_OK;
	if (!inSourceFolder.Exists())
	{
		err = VE_FOLDER_NOT_FOUND;
	}
	for( VFileIterator fileIterator( &inSourceFolder, FI_WANT_FILES | FI_WANT_INVISIBLES) ; fileIterator.IsValid() /*&& err == VE_OK*/ ; ++fileIterator)
	{
		sLONG8 fileSize = 0;
		err = fileIterator->GetSize( &fileSize);
		if (err == VE_OK)
		{
			outTotalBytes += fileSize;
		}
	}
	for( VFolderIterator folderIterator( &inSourceFolder, FI_WANT_FOLDERS | FI_WANT_INVISIBLES) ; folderIterator.IsValid() && err == VE_OK ; ++folderIterator)
	{
		err = Helpers::ComputeTotalSizeToBackup(*folderIterator,outTotalBytes);
	}
	return err;
}

VError CopyFolderWithProgress(const VFolder& inSourceFolder,VFolder& inDestinationFolder,bool inOverwriteExisting,
										  IDB4D_DataToolsIntf* inProgressIndicator,sLONG8 inTotalBytesToCopy,sLONG8& inTotalBytesCopied,
										  const VectorOfVString* inFilterByExt)
{
	VError err = VE_OK;
	FileCopyOptions fileCopyOptions = FCP_Default;

	if (inOverwriteExisting)
		fileCopyOptions |= FCP_Overwrite;

	if (inSourceFolder.IsSameFolder(&inDestinationFolder) ||inDestinationFolder.GetPath().IsChildOf( inSourceFolder.GetPath()))
	{
		err = VE_CANNOT_COPY_ON_ITSELF;
	}
	else if (!inDestinationFolder.Exists())
	{
		err = inDestinationFolder.Create();
	}

	if (err == VE_OK)
	{
		for( VFileIterator fileIterator( &inSourceFolder, FI_WANT_FILES | FI_WANT_INVISIBLES) ; fileIterator.IsValid() && err == VE_OK ; ++fileIterator)
		{
			sLONG8 fileSize = 0;
			if (inFilterByExt)
			{
				VString ext;
				fileIterator->GetPath().GetExtension(ext);
				VectorOfVString::const_iterator it = std::find(inFilterByExt->begin(),inFilterByExt->end(),ext);
				if (it == inFilterByExt->end())
				{
					//no match, copy
					err = fileIterator->CopyTo( inDestinationFolder, NULL,fileCopyOptions);
				}
			}
			else
			{
				err = fileIterator->CopyTo( inDestinationFolder, NULL,fileCopyOptions);
			}

			if (err == VE_OK && fileIterator->GetSize( &fileSize) == VE_OK)
			{
				inTotalBytesCopied += fileSize;
				if (inProgressIndicator)
				{
					inProgressIndicator->Progress(inTotalBytesCopied,inTotalBytesToCopy);
				}
			}
		}
		
		for( VFolderIterator folderIterator( &inSourceFolder, FI_WANT_FOLDERS | FI_WANT_INVISIBLES) ; folderIterator.IsValid() && err == VE_OK ; ++folderIterator)
		{
			const VFolder& source = *folderIterator;
			VString subFolderName;
			source.GetName(subFolderName);

			VFolder target(inDestinationFolder,subFolderName);
			err = CopyFolderWithProgress(source,target,inOverwriteExisting,inProgressIndicator,inTotalBytesToCopy,inTotalBytesCopied,inFilterByExt);
		}
	}
	return err;
}

VError InstallNewDatabaseJournal(Base4D& inBase,VFile& inNextJournal,XBOX::VUUID& ioDataLink)
{
	VError error = VE_OK;
	VFolder* journalParentFolder = inNextJournal.RetainParentFolder();
	if (!journalParentFolder->Exists())
	{
		error = journalParentFolder->CreateRecursive();
	}
	if (error == VE_OK)
	{
		error = inBase.SetJournalFile( &inNextJournal, &ioDataLink, true );
	}
	XBOX::ReleaseRefCountable(&journalParentFolder);
	return error;
}

VError SwapDatabaseJournal(Base4D& inBase,const VFile* inPreviousJournal,VFile& inNextJournal,XBOX::VUUID& ioDataLink)
{
	VError error = VE_OK;
	inBase.GetJournalUUIDLink( ioDataLink );
					
	/* disconnect old one */
	if (inPreviousJournal != NULL)
		inBase.SetJournalFile(NULL);

	/* connect new one */
	error = InstallNewDatabaseJournal(inBase,inNextJournal,ioDataLink);
	return error;
}
	
}//namespace Helpers

