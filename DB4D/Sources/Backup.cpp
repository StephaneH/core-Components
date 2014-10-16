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

const XBOX::VString& BackupRegistryName("lastBackups.json");
const XBOX::VString& FormerBackupRegistryName("lastBackup.json");
const XBOX::VString& BackupManifestName("backupManifest.json");

namespace Helpers
{
	XBOX::VError ThrowPathError(const XBOX::VError& inError,const XBOX::VFilePath& inPath);

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

	enum
	{
		JournalOperationsGroupID = 1011,
		JournalToolStringsGroupID = 1012
		
	};

	void LocalizeString(sLONG inID, uLONG inStringID, XBOX::VString &sout);

	void LocalizeJournalOperation(uLONG inOpType,XBOX::VString& caption);
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

	void GetJournalInfos(const VValueBag& inExtraProperties,const VFolder& inBaseFolder,XBOX::VFilePath& outJournalPath,XBOX::VUUID& outDataLink)
	{
		const VValueBag* journal_bag = inExtraProperties.RetainUniqueElement( LogFileBagKey::journal_file );
		if ( journal_bag != NULL )
		{
			VString logFilePath;
			journal_bag->GetString( LogFileBagKey::filepath, logFilePath );
			journal_bag->GetVUUID( LogFileBagKey::datalink, outDataLink );
			if( !logFilePath.IsEmpty() )
			{
				if ( logFilePath.BeginsWith(CVSTR(".")))
				{
					outJournalPath.FromRelativePath(inBaseFolder.GetPath(),logFilePath, FPS_POSIX);
				}
				else
				{
					outJournalPath.FromFullPath(logFilePath);
				}
			}
		}
		XBOX::ReleaseRefCountable(&journal_bag);
	}


	class VProgressWrapperToIDB4DToolInterface: public XBOX::VProgressIndicator
	{
	public:
		VProgressWrapperToIDB4DToolInterface(IDB4D_DataToolsIntf* inToolInterface):
		XBOX::VProgressIndicator(),
		fToolInterface(inToolInterface)
		{

		}
		virtual ~VProgressWrapperToIDB4DToolInterface(){}
		virtual void	DoBeginSession(sLONG inSessionNumber)
		{
			if (fToolInterface)
			{
				VString msg;
				fCurrent.GetMessage(msg);
				fToolInterface->OpenProgression(msg,fCurrent.GetMax());
			}
		}

		virtual void	DoEndSession(sLONG inSessionNumber)
		{
			 if(fToolInterface)
			 {
				 fToolInterface->CloseProgression();
			 }
		}
		virtual bool	DoProgress ()
		{
				 if(fToolInterface)
			 {
				fToolInterface->Progress(fCurrent.GetValue(),fCurrent.GetMax());
			 }
			 return true;
		}


	private:
		VProgressWrapperToIDB4DToolInterface();
		VProgressWrapperToIDB4DToolInterface(const VProgressWrapperToIDB4DToolInterface&);

	private:
		IDB4D_DataToolsIntf* fToolInterface;
	};
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
	XBOX::VJSONImporter parser(inJsonString, XBOX::VJSONImporter::EJSI_Strict);
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

	//WAK0084451 Apr 22nd 2014, O.R. add time zone info
	fDate.GetXMLString(string, XSO_Time_Local);
	jsonWriter.AddMember(CVSTR("date"),string,jsonOpts);

	string.Clear();
	VProcess* currentProcess = XBOX::VProcess::Get();
	if (currentProcess)
	{
		currentProcess->GetBuildNumber(string);
	}
	if(string.IsEmpty())
		string.FromCString("none");
	jsonWriter.AddMember(CVSTR("version"),string,jsonOpts);

	outJsonString.Clear();
	jsonWriter.GetObject(outJsonString);

}


//////////////////////////////////////////////////////////////////////////
//////////////////////  V B A C K U P  S E T T I N G S  //////////////////
//////////////////////////////////////////////////////////////////////////

VBackupSettings::VBackupSettings():
	VObject(),
	IBackupSettings(),
	fUniqueNames(false),
	fMaxRetainedBackups(20)
{

}

namespace BackupSettingsKeys
{
	CREATE_BAGKEY(maxRetainedBackups);
};


VBackupSettings::~VBackupSettings()
{
}

XBOX::VError VBackupSettings::Initialize(const XBOX::VJSObject& inObject)
{
	XBOX::VError error = VE_OK;

	const char* properties[] = { "destination", "backupRegistryFolder", "useUniqueNames", "maxRetainedBackups", NULL };
	for (int i = 0; properties[i] != NULL && (error == VE_OK); i++)
	{
		if (!inObject.HasProperty(properties[i]))
		{
			error = XBOX::vThrowError(VE_BACKUP_CONFIGURATION_PROPERTY_NOT_FOUND, properties[i]);
			}
		}
	if (error == VE_OK)
		{
		error = FromJSObject(inObject);
		}
	return error;
	}

XBOX::VError VBackupSettings::Initialize(const VValueBag& inBag, const VFilePath& inBaseFolder)
	{
	XBOX::VError error = VE_OK;
	bool valid = false;
	XBOX::VString temp;
	if (inBag.GetString(BackupSettingsKeys::maxRetainedBackups,temp))
	{
		temp.ToLowerCase();
		if (temp == CVSTR("all"))
		{
			SetRetainAllBackups(true);
				}
		else if (temp.GetLong() > 1)
				{
			SetMaxRetainedBackups(temp.GetLong());
				}
			else
			{
			error = vThrowError(VE_INVALID_BACKUP_SETTINGS);
			}
		}
		else
		{
		SetMaxRetainedBackups(20);
			}
	return error;
}

IBackupSettings* VBackupSettings::Clone()const
{
	VBackupSettings* copy = new VBackupSettings();
	copy->fBackupRegistryFolder = fBackupRegistryFolder;
	copy->fDestinationFolderPath = fDestinationFolderPath;
	copy->fMaxRetainedBackups = fMaxRetainedBackups;
	copy->fUniqueNames = fUniqueNames;
	return copy;
		}

XBOX::VError VBackupSettings::FromJSObject(const XBOX::VJSObject& inObject)
{
	VError error = VE_OK;

	if (inObject.HasProperty("destination"))
	{
		VFolder *folder = NULL;
		VJSValue value = inObject.GetProperty( "destination");
		if (value.IsObject())
		{
			folder = value.GetFolder();
			if (folder)
			{
				folder->GetPath(fDestinationFolderPath);
			}
			else
			{
				error = XBOX::vThrowError(VE_BACKUP_WRONG_PROPERTY_TYPE_EXPECTED_FOLDER, CVSTR("destination"));
			}
		}
		else
		{
			error = XBOX::vThrowError(VE_BACKUP_WRONG_PROPERTY_TYPE_EXPECTED_FOLDER, CVSTR("destination"));
		}
		folder = NULL;//obtained from GetFolder() so no need to release
	}
	

	if (inObject.HasProperty("backupRegistryFolder"))
	{
		VFolder *folder = NULL;
		VJSValue value = inObject.GetProperty( "backupRegistryFolder");
		if (value.IsObject())
		{
			folder = value.GetFolder();
			if (folder)
			{
				folder->GetPath(fBackupRegistryFolder);
			}
			else
			{
				error = XBOX::vThrowError(VE_BACKUP_WRONG_PROPERTY_TYPE_EXPECTED_FOLDER, CVSTR("backupRegistryFolder"));
			}
		}
		else
		{
			error = XBOX::vThrowError(VE_BACKUP_WRONG_PROPERTY_TYPE_EXPECTED_FOLDER, CVSTR("backupRegistryFolder"));
		}
		if (folder == NULL)
		{
			
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
			error = XBOX::vThrowError(VE_BACKUP_WRONG_PROPERTY_TYPE_EXPECTED_BOOLEAN, CVSTR("useUniqueNames"));
		}
	}

	if (inObject.HasProperty("maxRetainedBackups"))
	{
		VJSValue value = inObject.GetProperty("maxRetainedBackups");
		if (value.IsString())
		{
			XBOX::VString temp;
			value.GetString(temp);
			temp.ToLowerCase();
			if (temp == CVSTR("all"))
			{
				if (fMaxRetainedBackups > 0)
					fMaxRetainedBackups = 0 - fMaxRetainedBackups;
			}
			else
			{
						error = XBOX::vThrowError(VE_BACKUP_INVALID_PROPERTY_VALUE, CVSTR("maxRetainedBackups"));
			}
		}
		else if (value.IsNumber())
		{
			sLONG temp = 0;
			value.GetLong(&temp);
			if (temp >= 1)
			{
				fMaxRetainedBackups = temp;
			}
			else
			{
				error = XBOX::vThrowError(VE_BACKUP_INVALID_PROPERTY_VALUE, CVSTR("maxRetainedBackups"));
			}
		}
		else
		{
				error = XBOX::vThrowError(VE_BACKUP_WRONG_PROPERTY_TYPE_EXPECTED_STRING_OR_NUMBER, CVSTR("maxRetainedBackups"));
		}
	}

	if(error== VE_OK)
	{
		error = CheckValidity();
		error = vThrowError(error);
	}
	return error;
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

void VBackupSettings::SetRetainAllBackups(bool inRetainAll)
{
	if (testAssert((inRetainAll && fMaxRetainedBackups >0) || (!inRetainAll && (fMaxRetainedBackups < 0))))
	{
		fMaxRetainedBackups = 0 - fMaxRetainedBackups;
	}
}

void VBackupSettings::SetMaxRetainedBackups(int inCount)
{
	if (inCount > 0)
		fMaxRetainedBackups = inCount;
	else
		vThrowError(VE_INVALID_PARAMETER);
}

int VBackupSettings::GetMaxRetainedBackups(bool& outKeepAll)const
{
	if (fMaxRetainedBackups >0)
		outKeepAll = false;
	else
		outKeepAll = true;
	return fMaxRetainedBackups;
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
	{
		VFolder* f = new VFolder(filePath);
		if (f)
		{
			VJSValue val(outObject.GetContext());
			val.SetFolder(f);
			outObject.SetProperty(CVSTR("destination"),val);
		}
		XBOX::ReleaseRefCountable(&f);
	}
	if (GetUseUniqueNames())
	{
		outObject.SetProperty(CVSTR("useUniqueNames"),true);
	}
	else
	{
		outObject.SetProperty(CVSTR("useUniqueNames"),false);
	}

	filePath.Clear();
	GetBackupRegistryFolder(filePath);
	{
		VFolder* f = new VFolder(filePath);
		if(f)
		{
			VJSValue val(outObject.GetContext());
			val.SetFolder(f);
			outObject.SetProperty(CVSTR("backupRegistryFolder"),val);
		}
		XBOX::ReleaseRefCountable(&f);
	}
	bool all = 0; 
	sLONG n  = GetMaxRetainedBackups(all);
	if (testAssert(all || (n>1)))
	{
		if (all)
		{
			outObject.SetProperty(CVSTR("maxRetainedBackups"), CVSTR("all"));
		}
		else
		{
			outObject.SetProperty(CVSTR("maxRetainedBackups"), n);
}
	}
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

bool VBackupTool::ConvertBackupRegistryToJsObject(const XBOX::VFilePath& inBackupRegistryFilePath, XBOX::VJSArray& outManifests)
{
	XBOX::VError error = VE_OK;
	StErrorContextInstaller errContext;
	 
	//WAK0089203 Sept 1st 2014, O.R.: don't test file existence by trying to open and checking the error code, it is not portable. Us ethe exists() ethod instead
	VFile registryFile(inBackupRegistryFilePath);
	if (registryFile.Exists())
	{
		VFileStream manifestRegistryStream(&registryFile);
		error = manifestRegistryStream.OpenReading();
		if (error != VE_OK)
		{
			error = Helpers::ThrowPathError(VE_CANNOT_OPEN_BACKUP_REGISTRY, inBackupRegistryFilePath);
		}
		else
		{
			VString registryContent;
			if (manifestRegistryStream.GetText(registryContent) == VE_OK)
			{
				VJSONArrayReader reader(registryContent);
				const VectorOfVString& items = reader.GetStringsValues();
				VectorOfVString::const_iterator it = items.begin();

				for (; it != items.end() && (error == VE_OK); ++it)
				{
					VFilePath itemPath;
					VString itemPathString;
					itemPath.FromFullPath(*it, FPS_POSIX);
					itemPath.GetPosixPath(itemPathString);
					//WAK0087923 May 13th 2014, O.R.: previously we convertig the JSON manifest data to JSObject which
					//caused reinterpretation of the date property with respect to current server timezone, which is confusing if the 
					//backup manifest was generated in another timezone.
					VFile manifestFile(itemPath);
					VJSONValue parsedManifestData;

					//WAK0089203 Sept 1st 2014, O.R.: don't test file existence by trying to open and checking the error code, it is not portable. Us ethe exists() ethod instead
					//WAK0089332 Sept 2nd 2014, O.R.: if the manifest cannot be found, skip the entry
					if (manifestFile.Exists())
					{
						error = VJSONImporter::ParseFile(&manifestFile, parsedManifestData);
						if (error == VE_OK)
						{
							if (parsedManifestData.IsObject())
							{
								VJSObject manifObject(outManifests.GetContext());
								manifObject.MakeEmpty();

								manifObject.SetProperty(CVSTR("path"), itemPathString);

								//Copy properties from parsed JSONObject to JSObject and add the latter to array
								VJSONPropertyConstIterator propsIterator(parsedManifestData.GetObject());
								for (; propsIterator.IsValid(); ++propsIterator)
								{
									const VString& propertyName = propsIterator.GetName();
									const VJSONValue& valueFromJSONObject = propsIterator.GetValue();
									VJSValue propertyValue(outManifests.GetContext());
									propertyValue.SetJSONValue(valueFromJSONObject);
									manifObject.SetProperty(propertyName, propertyValue);
								}
								outManifests.PushValue(manifObject); 
							}
						}
					}
				}
			}
		}
	}
	return (errContext.GetLastError() == VE_OK);
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
	theBase4D = dynamic_cast<VDB4DBase*>(inBase)->GetBase();
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
			error = Helpers::ThrowPathError(VE_INVALID_JOURNAL_PATH,*inNextJournalPath);
		}
	}
	XBOX::VFilePath manifestFilePath;
	if(error == VE_OK)
	{
		ok = _DoBackup(theBase4D,context,inBackupSettings,inNextJournalPath,&manifestFilePath,inBackupEventLog);
		if(!ok)
		{
			error = VE_FAILED_TO_PERFORM_BACKUP;
		}
	}

	//Write manifest to the manifest registry
	if(error == VE_OK && ok)
	{
		error = _WriteManifestToRegistry(inBackupSettings, manifestFilePath, inBackupEventLog);
		if(error != VE_OK)
		{
			error = XBOX::vThrowError(VE_FAILED_TO_STORE_MANIFEST);
		}
	}

	if (error == VE_OK && (outManifestFilePath!= NULL))
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

bool VBackupTool::BackupClosedDatabase(XBOX::VFile& inModelFile,XBOX::VFile& inDataFile,const IBackupSettings& inBackupConfig,XBOX::VFilePath* outManifestFilePath,IDB4D_DataToolsIntf* inBackupEventLog)
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

	StErrorContextInstaller errorContext(true);
	
	//Since WAK7, project's journal info is stored inside the data file extra properties header.
	//Former WAK versions use a waExtra file side-by-side with the data file
	//These versions can be converted started with a WAK7 server
	//From now on we refuse to backup unconverted closed projects, they must have been opened once
	{
		VFilePath waExtraPath = inDataFile.GetPath();
		waExtraPath.SetExtension(CVSTR("waExtra"));
		VFile waExtra(waExtraPath);
		if(waExtra.Exists())
		{
			backupErr = vThrowError(VE_INCOMPATIBLE_DATASTORE_FORMAT_WAEXTRA);
		}
	}

	//Retrieve journal infos from the extra properties
	if(backupErr == VE_OK)
	{
		const VValueBag* extraProps = dbMgr->RetainExtraProperties( &inDataFile, NULL, NULL,backupErr,NULL,NULL);
		if(backupErr != VE_OK)
		{
			backupErr = vThrowError(VE_FAILED_TO_RETRIEVE_JOURNAL_INFO);
		}
		else if(extraProps != NULL)
		{
			XBOX::VFolder* parent = inDataFile.RetainParentFolder();
			JournalUtils::GetJournalInfos(*extraProps,*parent,currentJournalPath,dataLink);
			if(!currentJournalPath.IsEmpty())
			{
				journalFile = new VFile(currentJournalPath);
				if(!journalFile->Exists())
				{
					backupErr = Helpers::ThrowPathError(VE_JOURNAL_NOT_FOUND,journalFile->GetPath());
				}
			}
			XBOX::ReleaseRefCountable(&parent);
		}
		XBOX::ReleaseRefCountable(&extraProps);
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

	VFolder backupFolder(backupFolderPath);

	//For backup retention, user can use non-unique names however we impose
	//that the specified backup folder does not exist. If it does exist it's likely been used for
	//backup before and we don't want to overwrite content
	//If useUniqueNames is used the test will always succeed but is there for defensive purposes
	if (backupFolder.Exists())
	{
		backupErr = Helpers::ThrowPathError(VE_FOLDER_ALREADY_EXISTS, backupFolder.GetPath());
	}
	else
	{
		backupFolderPathForDataFolder = backupFolderPath;
		XBOX::VString temp;
		dataFolderPath.GetName(temp);
		backupFolderPathForDataFolder.ToSubFolder(temp);
		VFolder dataFolderBackupFolder(backupFolderPathForDataFolder);

		if (backupErr == VE_OK)
		{
			XBOX::VFolder* foldersToCreate[] = { &backupFolder, &dataFolderBackupFolder, NULL };
			for (int i = 0; foldersToCreate[i] != NULL && backupErr == VE_OK; i++)
			{
				if (!foldersToCreate[i]->Exists())
				{
					backupErr = foldersToCreate[i]->CreateRecursive();
					if (backupErr != VE_OK)
					{
						backupErr = Helpers::ThrowPathError(VE_FAILED_TO_CREATE_BACKUP_FOLDER, foldersToCreate[i]->GetPath());
					}
				}
			}
		}

		//Step one: archive backup journal
		if (backupErr == VE_OK && journalFile != NULL)
		{
			XBOX::VFile* backupedLogFile = NULL;
			if (inBackupEventLog)
			{
				inBackupEventLog->OpenProgression(CVSTR("Backing up log file"), -1);
			}
			//backup journal to backup folder first
			backupErr = journalFile->CopyTo(backupFolder, &backupedLogFile, XBOX::FCP_Overwrite);
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

			if (inBackupEventLog)
			{
				inBackupEventLog->CloseProgression();
			}
		}

		//Step 2: copy files from the data folder
		if (backupErr == VE_OK)
		{
			sLONG8 totalBytesToBackUp = 0, totalBytesCopied = 0;
			//Compute the actual size to be copied
			if (inBackupEventLog)
			{
				inBackupEventLog->OpenProgression(CVSTR("Estimating backup size"), -1);
			}
			if (Helpers::ComputeTotalSizeToBackup(dataFolder, totalBytesToBackUp) != VE_OK)
			{
				totalBytesToBackUp = -1;
			}
			if (inBackupEventLog)
			{
				inBackupEventLog->CloseProgression();
				inBackupEventLog->OpenProgression(CVSTR("Backing up database"), totalBytesToBackUp);
			}
			//copy the data folder, recursively
			backupManifest.SetDataFolder(backupFolderPathForDataFolder);

			VectorOfVString filter;
			filter.push_back(CVSTR("waJournal"));
			backupErr = Helpers::CopyFolderWithProgress(dataFolder, dataFolderBackupFolder, true, inBackupEventLog, totalBytesToBackUp, totalBytesCopied, &filter);
			//backupErr = dataFolder->CopyTo(backupFolder,&destSubFolder);
			if (backupErr != VE_OK)
			{
				vThrowError(VE_CANNOT_CREATE_DATAFOLDER_BACKUP);
			}
			else
			{
				/*backupErr =*/ _WriteManifest(backupFolderPath, backupManifest, &manifestFilePath);
			}

			if (inBackupEventLog)
			{
				inBackupEventLog->CloseProgression();
			}
		}

		//Step 3: reset journal file if applicable
		if (backupErr == VE_OK)
		{
			if (journalFile)
			{
				//We create a temporary journal file that we initialize using the
				//datalink from the extra props.
				//If all ends well then we swap the previous journal file and this one
				//If an error occured for some reason the previous journal is preserved and unmodified
				XBOX::VFilePath tempJournalPath = currentJournalPath;
				tempJournalPath.SetFileName(CVSTR("tempJournal"), false);
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
							if (error == VE_OK)
							{
								DB4DJournalHeader journalHeader;
								journalFileStream->SetBufferSize(0);
								journalHeader.Init(dataLink);
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

				if (error == VE_OK)
				{
					//The temporary journal was successfully created and initialized, now swap it with the current journal
					XBOX::VFile* renamedOldJournalFile = NULL;
					XBOX::VString newName(CVSTR("oldJournal_")), timestamp;
					Helpers::ComputeTimeStamp(NULL, timestamp, true);

					newName.AppendString(timestamp);
					if (!journalExtension.IsEmpty())
					{
						newName.AppendChar('.');
						newName.AppendString(journalExtension);
					}
					error = journalFile->Rename(newName, &renamedOldJournalFile);
					if (error == VE_OK && renamedOldJournalFile)
					{
						currentJournalPath.GetFileName(newName, true);
						error = tempJournalFile->Rename(newName, NULL);
						if (error != VE_OK)
						{
							//The swap did not work then rollback on old journal file renaming
							error = renamedOldJournalFile->Rename(newName, NULL);
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
			/*backupErr =*/_WriteManifestToRegistry(inBackupConfig, manifestFilePath, inBackupEventLog);
			if (outManifestFilePath)
			{
				*outManifestFilePath = manifestFilePath;
			}
		}
		XBOX::ReleaseRefCountable(&journalFile);
	}

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
			backupErr = Helpers::ThrowPathError(VE_INVALID_NEXT_JOURNAL_PATH,*inNextJournalFilePath);
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
		if(backupErr != VE_OK)
		{
			backupErr = vThrowError(VE_INVALID_BACKUP_SETTINGS);
		}
	}

	if (backupErr == VE_OK)
	{
		//Check if backup folder in settings
		inBackupConfig.GetDestination(backupFolderPath);
		if (inBackupConfig.GetUseUniqueNames())
		{
			Helpers::ComputeUniqueFolderName(backupFolderPath, CVSTR("backup"));
		}

		XBOX::VFolder backupFolder(backupFolderPath);
		backupFolder.GetPath(backupFolderPath);


		//For backup retention, user can use non-unique names however we impose
		//that the specified backup folder does not exist. If it is does exist it's likely been used for
		//backup before and we don't want to overwrite content
		//If useUniqueNAmes is defined the test should always pass but it there for defensive purposes
		if (backupFolder.Exists())
		{
			backupErr = Helpers::ThrowPathError(VE_FOLDER_ALREADY_EXISTS, backupFolder.GetPath());
		}
		else
		{
			XBOX::VFilePath backupFolderPathForDataFolder(backupFolderPath);
			{
				XBOX::VString dataFolderName;
				dataFolder->GetName(dataFolderName);
				backupFolderPathForDataFolder.ToSubFolder(dataFolderName);
			}

			XBOX::VFolder backupFolderForDataFolder(backupFolderPathForDataFolder);
			XBOX::VFolder* foldersToCreate[] = { &backupFolder, &backupFolderForDataFolder, NULL };
			for (int i = 0; foldersToCreate[i] != NULL && backupErr == VE_OK; i++)
			{
				if (!foldersToCreate[i]->Exists())
				{
					backupErr = foldersToCreate[i]->CreateRecursive();
					if (backupErr != VE_OK)
					{
						backupErr = Helpers::ThrowPathError(VE_FAILED_TO_CREATE_BACKUP_FOLDER, foldersToCreate[i]->GetPath());
					}
				}
			}

			if (backupErr == VE_OK)
			{
				backupErr = _PrepareForBackup(inBase, inContext);
				if (backupErr != VE_OK)
				{
					vThrowError(backupErr);
				}

				//Step one: backup journal file , create a salvage
				if (backupErr == VE_OK && logFile)
				{
					if (inBackupEventLog)
					{
						inBackupEventLog->OpenProgression(CVSTR("Backing up log file"), -1);
					}

					//Disabe the current log file
					inBase->GetJournalUUIDLink(backedUpJournalUUID);
					inBase->SetJournalFile(NULL);
					//backup journal to backup folder first
					backupErr = logFile->Move(backupFolder, &backedupLogFile, XBOX::FCP_Overwrite);
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
							backupErr = Helpers::InstallNewDatabaseJournal(*inBase, *nextJournalFile, backedUpJournalUUID);
							changedLogFile = (backupErr == VE_OK);
							XBOX::ReleaseRefCountable(&nextJournalFile);
						}
					}
					if (inBackupEventLog)
					{
						inBackupEventLog->CloseProgression();
					}
				}

				//Special case where we need to swap the journal
				if (!changedLogFile && ((nextJournalFile != NULL) && ((logFile == NULL) //there was no journal before backup and one must be activated after backup
					|| !logFile->IsSameFile(nextJournalFile))))// or the journal properties (name/location) have changed and must be applied after backup
				{

					XBOX::VUUID dataLink;

					backupErr = Helpers::SwapDatabaseJournal(*inBase, logFile, *nextJournalFile, dataLink);
					changedLogFile = (backupErr == VE_OK);
				}

				//Step 2: activate journaling and backup the whole data folder and attachments
				if (backupErr == VE_OK)
				{
					sLONG8 totalBytesToBackUp = 0, totalBytesCopied = 0;

					if (changedLogFile && (logFile == NULL))
					{
						//Journal has been changed so make sure journaling is enabled for data & model
						Helpers::SetDatabaseJournalEnabled(inBase, inContext, true);
					}

					//Notify flush for backup in log
					VDBMgr::GetManager()->FlushCache(inBase, true);

					//Compute the actual size to be copied
					if (inBackupEventLog)
					{
						inBackupEventLog->OpenProgression(CVSTR("Estimating backup size"), -1);
					}
					if (Helpers::ComputeTotalSizeToBackup(*dataFolder, totalBytesToBackUp) != VE_OK)
					{
						totalBytesToBackUp = -1;
					}
					if (inBackupEventLog)
					{
						inBackupEventLog->CloseProgression();
					}

					if (inBackupEventLog)
					{
						inBackupEventLog->OpenProgression(CVSTR("Backing up database"), totalBytesToBackUp);
					}

					//copy the data folder, recursively
					VFolder* destSubFolder = NULL;
					backupManifest.SetDataFolder(backupFolderForDataFolder.GetPath());

					VectorOfVString filter;
					filter.push_back(CVSTR("waJournal"));
					backupErr = Helpers::CopyFolderWithProgress(*dataFolder, backupFolderForDataFolder, true, inBackupEventLog, totalBytesToBackUp, totalBytesCopied, &filter);

					if (backupErr != VE_OK)
					{
						vThrowError(VE_CANNOT_CREATE_DATAFOLDER_BACKUP);
					}
					else
					{
						/*backupErr =*/ _WriteManifest(backupFolderPath, backupManifest, outManifestFilePath);
					}

					if (inBackupEventLog)
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
						xbox_assert(!backedUpJournalUUID.IsNull());
						inBase->SetJournalFile(NULL);

						backedupLogFile->CopyTo(currentJournalFilePath, NULL, XBOX::FCP_Overwrite);
						backupErr = inBase->OpenJournal(logFile, backedUpJournalUUID);
						if (backupErr != VE_OK)
							vThrowError(VE_CANNOT_RESTORE_JOURNAL);
						xbox_assert(backupErr == XBOX::VE_OK);
					}
				}
				_EndBackup(inBase, inContext, backedupLogFile);
			}
		}
	}
	
	XBOX::ReleaseRefCountable(&backedupLogFile);
	XBOX::ReleaseRefCountable(&nextJournalFile);
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
		xbox_assert(context->GetBase() == inBase);
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
		xbox_assert(context->GetBase() == inBase);
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

XBOX::VError VBackupTool::_WriteManifestToRegistry(const IBackupSettings& inBackupSettings, const VFilePath& inManifestPath, IDB4D_DataToolsIntf* inProgress)
{
	StErrorContextInstaller errContext;

	XBOX::VError error = VE_OK;
	XBOX::VFilePath registryPath;

	VectorOfVString registryItems;
	error = _ReadManifestRegistry(inBackupSettings,registryItems);
	if(error == VE_OK ||error == VE_FILE_NOT_FOUND )
	{
		//registry not found? it is ok, we can discard that error
		errContext.Flush();
		
		//Manifest are kept as most recent last
		VString value;
		inManifestPath.GetPosixPath(value);

		//WAK0084452: Jan 27th 2014, O.R.: ensure no duplicate entries are kept, some registries may have more than 1 duplicate so we need to iterate
		VectorOfVString::iterator dup = find(registryItems.begin(), registryItems.end(), value);
		while (dup != registryItems.end()){
			registryItems.erase(dup);
		}

		//Then add new entry
		registryItems.push_back(value);
		
		//Keep only the last 20 entries
		bool keepAll = false;
		sLONG maxRegistryCapacity = inBackupSettings.GetMaxRetainedBackups(keepAll);
		sLONG listSize = registryItems.size();
		if (!keepAll && listSize > maxRegistryCapacity)
		{
			//Erase items by popping them from the beginning. 
			//less efficient than doing a erase(begin,end), it keeps a coherence between
			//manifest and backup folders deleted from the file system and the content of registryItems
			for (sLONG i = 0; i < listSize - maxRegistryCapacity; i++)
			{
				VectorOfVString::iterator it = registryItems.begin();
				VFile manifFile(*it, FPS_POSIX);

				//Remove from registry
				registryItems.erase(it);

				//Remove from file system
				VFolder* backupFolder = manifFile.RetainParentFolder();
				error = backupFolder->Delete(true);

				//Silently discard error, the backup folder is no longer referenced anyway
				if (error != VE_OK)
				{
					if (inProgress != NULL)
					{
						VErrorBase* errorBase = errContext.GetContext()->GetLast();
						if (errorBase)
						{
							VValueBag* errorBag = new VValueBag();
							if (errorBag != NULL)
							{
								errorBag->SetLong(L"ErrorLevel", 3);//warning
								VString temp;
								errorBase->GetErrorDescription(temp);
								errorBag->SetString(L"ErrorText", temp);
								inProgress->AddProblem(*errorBag);
								errorBag->Release();
							}
						}
					}
					errContext.Flush();
				}
				backupFolder->Release();
			}
			xbox_assert(registryItems.size() == maxRegistryCapacity);
		}
		error = _WriteManifestRegistry(inBackupSettings,registryItems);
	}
	return errContext.GetLastError();
}


bool VBackupTool::_RestoreFile(const XBOX::VFilePath& inPathOfFileToRestore,const XBOX::VFilePath& inRestoreFolderPath,XBOX::VFilePath& outFormerFilePath,IDB4D_DataToolsIntf* inProgress)
{
	VError error = VE_OK;
	//const VFilePath& sourcePath =  inPathOfFileToRestore;
	
	if ( (error == VE_OK) && testAssert(inPathOfFileToRestore.IsValid() && inPathOfFileToRestore.IsFile()))
	{
		VFile* destFile = NULL;
		VFilePath destFilePath;
		VString suffix,name;
		
		destFilePath = inRestoreFolderPath;
		
		inPathOfFileToRestore.GetFileName(name);
		destFilePath.ToSubFile(name);
		
		//Check if file exists in destination folder
		destFile = new VFile(destFilePath);
		if (destFile->Exists())
		{
			//It does, rename it
			VFile* renamedFile = NULL;
			VString ext;
			
			//Generate rename prefix.
			destFilePath.GetFileName(name,false);
			destFilePath.GetExtension(ext);
			Helpers::ComputeTimeStamp(NULL,suffix,false);
			name.AppendPrintf("_REPLACED_%S",&suffix);
			if (!ext.IsEmpty())
			{
				name.AppendPrintf(".%S",&ext);
			}
			error = destFile->Rename(name,&renamedFile);
			if (error == VE_OK)
			{
				renamedFile->GetPath(outFormerFilePath);
			}
			XBOX::ReleaseRefCountable(&renamedFile);
		}
		XBOX::ReleaseRefCountable(&destFile);
				
		if(error == VE_OK)
		{
			VFile* sourceFile = new VFile(inPathOfFileToRestore);
			VFolder* destFolder = new VFolder(inRestoreFolderPath);
			
			if (!destFolder->Exists())
			{
				error = destFolder->CreateRecursive();
			}
			
			if( error == VE_OK)
			{
				sLONG8 totalBytesToCopy = 0;
				if(inProgress)
				{
					sourceFile->GetSize(&totalBytesToCopy);
					inProgress->OpenProgression(CVSTR("Copying file"),totalBytesToCopy);
				}
				error = sourceFile->CopyTo(*destFolder,NULL,FCP_Default);
				if(inProgress)
				{
					inProgress->Progress(totalBytesToCopy,totalBytesToCopy);
					inProgress->CloseProgression();
				}
			}
			XBOX::ReleaseRefCountable(&destFolder);
			XBOX::ReleaseRefCountable(&sourceFile);
			error = vThrowError(error);
		}
	}
	return (error == VE_OK);
	
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


XBOX::VError VBackupTool::_WriteManifestRegistry(const IBackupSettings& inBackupSettings, const VectorOfVString& inRegistry)
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
		registryPath.SetFileName(BackupRegistryName,true);
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
	StErrorContextInstaller errContext;

	//WAK0088867July 23rd 2014, O.R.: we were previously relying on filtering VE_FILE_NOT_FOUND that
	//was thrown when opening a VFileStream that does not exists. On Linux it does not work this way.
    outRegistry.clear();
	inBackupSettings.GetBackupRegistryFolder(registryPath);
	registryPath.SetFileName(BackupRegistryName,true);

	//WAK0088009: pre-WAK9 projects used the wrong backup registry name. Now check if the registry with the correct filename exists. If not
	//use the former registry filename. The registry with the correct name will be created when a backup is next performed.
	{
		XBOX::VFile test(registryPath);
		if (!test.Exists())
		{
			registryPath.SetFileName(FormerBackupRegistryName, true);
		}
	}

	//WAK0088867July 23rd 2014, O.R.: so now we just check if the file exists before attempting to open it
	XBOX::VFile registryFile(registryPath);
    if(registryFile.Exists())
    {
        XBOX::VFileStream stream(&registryFile,XBOX::FO_Default);
        error = stream.OpenReading();
        if(error == VE_OK)
        {
            VString jsonString;
            VJSONArrayReader registryReader;
            stream.GetText(jsonString);
            registryReader.FromJSONString(jsonString);
            registryReader.ToArrayString(outRegistry);
            stream.CloseReading();
        }
        else
        {
            Helpers::ThrowPathError(VE_CANNOT_OPEN_BACKUP_REGISTRY, registryPath);
        }
    }
	return errContext.GetLastError();
}
	
	
XBOX::VError VBackupTool::_WriteManifest(const VFilePath& inManifestFolder,const VBackupManifest& inManifest,XBOX::VFilePath* outManifestFile)
{
	VError error(VE_OK);
	XBOX::VFilePath manifestPath(inManifestFolder);

	manifestPath.SetFileName(BackupManifestName,true);


	/* WAK0080369 O.R. Mar 1st 2013, let VFileStream handle creation and trunctation so 
	   everything works smoothly on all platforms*/
	XBOX::VFile file(manifestPath);
	XBOX::VFileStream stream(&file,XBOX::FO_Overwrite| XBOX::FO_CreateIfNotFound);
	error = stream.OpenWriting();
	if(error == VE_OK)
	{
		stream.SetCharSet(XBOX::VTC_UTF_8);
		error = inManifest.ToStream(stream);
		stream.Flush();
		stream.CloseWriting();
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

bool VBackupTool::RestoreJournal(const XBOX::VFilePath& inBackupManifestPath,const XBOX::VFilePath& inRestoreFolder,XBOX::VFilePath& outRenamedJournalPath,IDB4D_DataToolsIntf* inProgress)
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
		const VFilePath& journalPath = manifest.GetJournal();
		if(!journalPath.IsEmpty())
		{
			success = _RestoreFile(journalPath,inRestoreFolder,outRenamedJournalPath,inProgress);
		}
		else 
		{
			success = true;
		}
	}
	return (success && error == VE_OK);
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

bool VBackupTool::RestoreDataFolderContent(const XBOX::VFilePath& inBackupManifestPath,const XBOX::VFilePath& inRestoreFolderPath,IDB4D_DataToolsIntf* inProgress)
{
	VBackupManifest manifest;
	VFile manifFile(inBackupManifestPath);
	VFileStream manifStream(&manifFile);
	VError error = VE_OK;
	
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
		VFolder restoreFolder(inRestoreFolderPath);
		if(!restoreFolder.Exists())
			error = restoreFolder.CreateRecursive();
		
		if(error == XBOX::VE_OK)
		{
			sLONG8 totalCopied = 0, totalToCopy = 0;
			XBOX::VFolder backedUpDataFolder(manifest.GetDataFolder());
			Helpers::ComputeTotalSizeToBackup(backedUpDataFolder,totalToCopy);
			error = Helpers::CopyFolderWithProgress(backedUpDataFolder,restoreFolder,true,inProgress,totalToCopy,totalCopied, NULL);
		}
	}
	return (error == VE_OK);
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


////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////

/////
///// Wrapper 

class VProgressIndicatorBrigdeToIDB4DToolInterface: public XBOX::VProgressIndicator
{
private:
	IDB4D_DataToolsIntf* fToolInterface;
public:
	 VProgressIndicatorBrigdeToIDB4DToolInterface(IDB4D_DataToolsIntf* inToolInterface):
	  XBOX::VProgressIndicator(),
		  fToolInterface(inToolInterface)
	  {
	  }
	  virtual ~VProgressIndicatorBrigdeToIDB4DToolInterface(){}
	  virtual void	DoBeginSession(sLONG inSessionNumber)
	  {
		  if (fToolInterface)
		  {
			  VString msg;
			  fCurrent.GetMessage(msg);
			  fToolInterface->OpenProgression(msg,fCurrent.GetMax());
		  }
	  }

	  virtual void	DoEndSession(sLONG inSessionNumber)
	  {
		 if(fToolInterface)
		 {
			 fToolInterface->CloseProgression();
		 }
	  }
	 virtual bool	DoProgress ()
	 {
		 if(fToolInterface)
		 {
			fToolInterface->Progress(fCurrent.GetValue(),fCurrent.GetMax());
		 }
		 return true;
	 }
	private:
		VProgressIndicatorBrigdeToIDB4DToolInterface();
		VProgressIndicatorBrigdeToIDB4DToolInterface(const VProgressIndicatorBrigdeToIDB4DToolInterface&);
};




VJournalTool::VJournalTool():XBOX::VObject(),
	IJournalTool()
{
}

VJournalTool::~VJournalTool()
{
}

XBOX::VError VJournalTool::Initialize(const XBOX::VFile& inTableDefFile)
{
	XBOX::VError error = VE_OK;
	if (inTableDefFile.Exists())
	{
		VJSONValue value;
		error = vThrowError(VJSONImporter::ParseFile(&inTableDefFile, value));
		if (error == VE_OK)
		{
			if (value.IsArray())
			{
				fMapTableUUidToName.clear();
				VJSONArray* array = value.GetArray();
				for (size_t i = array->GetCount(); i > 0; i--)
				{
					const VJSONValue& item = (*array)[i-1];
					if (item.IsObject())
					{
						XBOX::VString tableName, uuidStr;
						if (item.GetObject()->GetPropertyAsString("dataTableID", uuidStr) && item.GetObject()->GetPropertyAsString("tableName", tableName))
						{
							XBOX::VUUID tableUUid;
							tableUUid.FromString(uuidStr);

							std::pair<XBOX::VUUID, XBOX::VString> item(tableUUid, tableName);
							fMapTableUUidToName.insert(item);
						}
					}
				}
			}
		}
		else
		{
			error = Helpers::ThrowPathError(VE_FILE_NOT_FOUND, inTableDefFile.GetPath());
		}
	}
	return error;
}

bool VJournalTool::ParseJournal(XBOX::VFile& inJournalFile, XBOX::VJSONArray& outOperations, IDB4D_DataToolsIntf* inProgressLog)
{
	CDB4DManager* dbMgr = NULL;
	CDB4DJournalParser* journalParser = NULL;
	JournalUtils::VProgressWrapperToIDB4DToolInterface* progress = NULL;
	XBOX::VError err = VE_OK;

	XBOX::StErrorContextInstaller errContext(true,true);

	dbMgr = CDB4DManager::RetainManager();
	journalParser = dbMgr->NewJournalParser();
	if(inProgressLog)
	{
		progress = new JournalUtils::VProgressWrapperToIDB4DToolInterface(inProgressLog);
	}

	if( journalParser)
	{
		uLONG8 totalOperations = 0;
		if (inJournalFile.Exists())
		{
			err = journalParser->Init(&inJournalFile, totalOperations, progress);
			if (err == VE_OK)
			{
				uLONG8 total = journalParser->CountOperations();
				CDB4DJournalData* journalOp = NULL;

				for (uLONG8 current = 0; (current < total) && (err == VE_OK); current++)
				{
					//WAK0080167 O.R. 28/03/2013 journal operations range from 1 to totalOperations
					err = journalParser->SetCurrentOperation((current + 1), &journalOp);
					if (journalOp)
					{
						VJSONObject* obj = _ConvertToObject(*journalOp);
						XBOX::VJSONValue entry(obj);
						outOperations.Push(entry);
						XBOX::ReleaseRefCountable(&obj);
					}
					XBOX::ReleaseRefCountable(&journalOp);
				}
			}
			else
			{
				err = Helpers::ThrowPathError(VE_INVALID_JOURNAL_PATH, inJournalFile.GetPath());
			}
		}
		else
		{
			err = Helpers::ThrowPathError(VE_FILE_NOT_FOUND, inJournalFile.GetPath());
		}

	}
	XBOX::ReleaseRefCountable(&progress);
	XBOX::ReleaseRefCountable(&journalParser);
	XBOX::ReleaseRefCountable(&dbMgr);
	
	return (errContext.GetLastError() == VE_OK);
}

bool VJournalTool::ParseJournalToText(XBOX::VFile& inJournalFile,XBOX::VFile& inDestinationFile,IDB4D_DataToolsIntf* inProgressLog)
{
	XBOX::StErrorContextInstaller errContext(true, true);
	XBOX::VJSONArray* arrayOps = new XBOX::VJSONArray();
	
	if (ParseJournal(inJournalFile, *arrayOps, inProgressLog))
	{
		size_t total = arrayOps->GetCount();
		VJSONWriter writer(JSON_PrettyFormatting);

		XBOX::VFileStream str(&inDestinationFile, FO_Overwrite | FO_CreateIfNotFound);
		str.SetCharSet(VTC_UTF_8);
		str.OpenWriting();
		XBOX::VString line, temp;
		inJournalFile.GetPath().GetPosixPath(temp);
		line.Clear();
		line.AppendPrintf("{\r\n\"journal\": \"%S\",\r\n\"operations\": [\r\n", &temp);
		str.PutText(line);
		str.Flush();

		if (inProgressLog)
		{
			XBOX::VString title;
			LocalizeString(Helpers::JournalToolStringsGroupID, 1, title);
			inProgressLog->OpenProgression(title, total);
		}
		
		for (size_t current = 0; (current < total); current++)
		{
			const VJSONValue& item = (*arrayOps)[current];
			if (testAssert(item.IsObject()))
			{
				VString dump;
				writer.StringifyObject(item.GetObject(), dump);
				line.Clear();
				if (current < (total - 2))
				{
					line.AppendPrintf("%S,\r\n", &dump);
				}
				else
				{
					line.AppendPrintf("%S\r\n", &dump);
				}
				str.PutText(line);			
				if (inProgressLog)
				{
					inProgressLog->Progress(current, total);
				}
			}
		}
		if (inProgressLog)
		{
			inProgressLog->CloseProgression();
		}
		line.Clear();
		line.AppendPrintf("]\r\n}\r\n");
		str.PutText(line);
		str.Flush();
		str.CloseWriting();
	}
	return (errContext.GetLastError() == VE_OK);
}



XBOX::VJSONObject* VJournalTool::_ConvertToObject(CDB4DJournalData& injournalOperation)const
{
	XBOX::VError err = VE_OK;
	XBOX::VJSONObject* obj = injournalOperation.RetainDescription(err, true, NULL);
	VUUID tableUuid;
	
	obj->SetPropertyAsNumber(CVSTR("globalIndex"), injournalOperation.GetGlobalOperationNumber());

	if (injournalOperation.GetTableID(tableUuid))
	{
		std::map<XBOX::VUUID, XBOX::VString>::const_iterator it = fMapTableUUidToName.find(tableUuid);
		if (it != fMapTableUUidToName.end())
		{
			obj->SetPropertyAsString("tableName", it->second);
		}
	}
	
	return obj;
}

namespace Helpers{
	
	XBOX::VError ThrowPathError(const XBOX::VError& inError,const XBOX::VFilePath& inPath)
	{
		StThrowError<> err(inError);
		XBOX::VString posixPath;
		inPath.GetPosixPath(posixPath);
		switch (inError)
		{
		case VE_FILE_NOT_FOUND:
		{
			XBOX::VString name;
			inPath.GetName(name);
			err->SetString("name", name);
		}
			//fallthrough
		default:
			err->SetString("path", posixPath);
			break;
		}
		return inError;
	}

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

	///WAK0084654, O.R. Oct 24th 2013: if no journal installed yet uuid will be null. Make sure it is generated
	//so that other code in db4D does not set it, causing journal mismatch problem at integration
	if(ioDataLink.IsNull())
	{
		ioDataLink.Regenerate();
	}

	/* connect new one */
	error = InstallNewDatabaseJournal(inBase,inNextJournal,ioDataLink);
	return error;
}

void LocalizeString(sLONG inID, uLONG inStringID, XBOX::VString &sout)
{
	VDBMgr* manager = VDBMgr::GetManager();
	VLocalizationManager* local = manager->GetDefaultLocalization();

	sout.Clear();

	if (local != NULL)
	{
		STRSharpCodes sc(inID,inStringID);
		local->LocalizeStringWithSTRSharpCodes(sc,sout);
	}
}

void LocalizeJournalOperation(uLONG inOpType,XBOX::VString& caption)
{
	uLONG strId = 0;
	switch ( inOpType )
	{
		case DB4D_Log_CreateRecord: 
		case DB4D_Log_CreateRecordWithPrimKey: 
			strId = 1;
			
			break;
		case DB4D_Log_ModifyRecord: 
		case DB4D_Log_ModifyRecordWithPrimKey: 
			strId = 2;
			break;
		case DB4D_Log_DeleteRecord: 
		case DB4D_Log_DeleteRecordWithPrimKey: 
			strId = 3;
			break;
		case DB4D_Log_StartCommit: 
		case DB4D_Log_StartTrans: 
			strId = 4;
			break;
		case DB4D_Log_EndCommit: 
		case DB4D_Log_Commit: 
			strId = 5;
			break;
		case DB4D_Log_RollBack: 
			strId = 6;
			break;
		case DB4D_Log_OpenData: 
			strId = 7;
			break;
		case DB4D_Log_CloseData: 
			strId = 8;
			break;
		case DB4D_Log_CreateContextWithUserUUID:
			strId = 9;
			break;
		case DB4D_Log_CreateContextWithExtra: 
			strId = 10;
			break;
		case DB4D_Log_CloseContext: 
			strId = 11;
			break;
		case DB4D_Log_CreateBlob: 
		case DB4D_Log_CreateBlobWithPrimKey: 
			strId = 12;
			break;
		case DB4D_Log_ModifyBlob: 
		case DB4D_Log_ModifyBlobWithPrimKey: 
			strId = 13;
			break;
		case DB4D_Log_DeleteBlob: 
		case DB4D_Log_DeleteBlobWithPrimKey: 
			strId = 14;
			break;
		case DB4D_Log_StartBackup:
			strId = 15;
			break;
		case DB4D_Log_TruncateTable: 
			strId = 16;
			break;
		case DB4D_Log_SaveSeqNum: 
			strId = 17;
			break;
		default: 
			strId = 0;
			break;
	}
	caption.Clear();
	LocalizeString(JournalOperationsGroupID,strId,caption);
	if(caption.IsEmpty())
	{
		caption.AppendPrintf("??? 0x%04x",inOpType);
	}
}
	
}//namespace Helpers

