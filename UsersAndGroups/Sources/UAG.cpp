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
#include "UAGHeaders.h"
#include "JavaScript/Sources/VJSJSON.h"

const sLONG				kCOMPONENT_TYPE_COUNT	= 1;
const CImpDescriptor	kCOMPONENT_TYPE_LIST[]	= {
	{ CUAGManager::Component_Type, VImpCreator<VUAGManager>::CreateImp }
};

VComponentLibrary* gUAGCompLib = nil;

void XBOX::xDllMain (void)
{
	gUAGCompLib = new VComponentLibrary(kCOMPONENT_TYPE_LIST, kCOMPONENT_TYPE_COUNT);
}


  

// ------------------------------------------------------------


VUAGManager::VUAGManager()
{
	fDB4D = VComponentManager::RetainComponentOfType<CDB4DManager>();
	fCompLibrary = gUAGCompLib;


	fComponentFolder = gUAGCompLib->GetLibrary()->RetainFolder(kBF_BUNDLE_FOLDER);
	fDefaultLocalization = new VLocalizationManager(VComponentManager::GetLocalizationLanguage(fComponentFolder,true));
	if (fComponentFolder != nil)
		fDefaultLocalization->LoadDefaultLocalizationFoldersForComponentOrPlugin(fComponentFolder);

	VErrorBase::RegisterLocalizer(CUAGManager::Component_Type, fDefaultLocalization);

}


VUAGManager::~VUAGManager()
{
	QuickReleaseRefCountable(fDB4D);
	QuickReleaseRefCountable(fComponentFolder);
	QuickReleaseRefCountable(fDefaultLocalization);
	/*
	for (UsersByIDMap::iterator cur = fUsersByID.begin(), end = fUsersByID.end(); cur != end; cur++)
	{
		cur->second->Release();
	}
	for (UsersByNameMap::iterator cur = fUsersByName.begin(), end = fUsersByName.end(); cur != end; cur++)
	{
		cur->second->Release();
	}

	for (GroupsByIDMap::iterator cur = fGroupsByID.begin(), end = fGroupsByID.end(); cur != end; cur++)
	{
		cur->second->Release();
	}
	for (GroupsByNameMap::iterator cur = fGroupsByName.begin(), end = fGroupsByName.end(); cur != end; cur++)
	{
		cur->second->Release();
	}
	*/
}

VFolder* VUAGManager::RetainFolder(BundleFolderKind inKind)
{
	return fCompLibrary->GetLibrary()->RetainFolder(inKind);
}


CUAGDirectory* VUAGManager::RetainDirectory(const VFile& inXMLFile, FileAccess inAccess, const VFolder* parent, const VString* inDBName, 
										VError* outErr, bool* outWasJustCreated, bool buildEmpyDir)
{
	UAGDirectory* Directory = new UAGDirectory(this);
	VError err = VE_OK;

	VString dbname;
	VFolder* xparent = nil;
	if (parent == nil)
	{
		xparent = inXMLFile.RetainParentFolder();
		parent = xparent;
	}

	if (inDBName == nil)
	{
		inDBName = &dbname;
		inXMLFile.GetNameWithoutExtension(dbname);
		dbname += ".cacheUAG";
	}

	err = Directory->OpenDB(&inXMLFile, *parent, *inDBName, inAccess, outWasJustCreated, buildEmpyDir);
	xbox_assert ( err == VE_OK );

	QuickReleaseRefCountable(xparent);

	if (err == VE_OK)
	{
		for (sLONG i = (sLONG)CUAGDirectory::AdminGroup; i <= (sLONG)CUAGDirectory::DebuggerGroup; ++i)
		{
			CUAGDirectory::SpecialGroupEnum groupref = (CUAGDirectory::SpecialGroupEnum)i;
			VError err2;
			StErrorContextInstaller errs(false);
			VUUID xid;
			Directory->GetSpecialGroupID(groupref, xid);
			UAGGroup* group = Directory->retainGroup(xid, &err2);
			if (group == nil)
			{
				VString name;
				switch (groupref)
				{
					case CUAGDirectory::AdminGroup:
						name = "Admin";
						break;
					case CUAGDirectory::DebuggerGroup:
						name = "Debugger";
						break;
				}
				group = Directory->addOneGroup(name, name, err2, &xid);
			}
			QuickReleaseRefCountable(group);
		}
		
	}

	if (err == VE_OK)
	{
		Directory->ComputeNoAdmin();
	}

	if (err != VE_OK)
	{
		Directory->Release();
		Directory = nil;
	}

	if (outErr != nil)
		*outErr = err;

	return Directory;
}



UAGDirectory::UAGDirectory(VUAGManager* inManager)
{
	fBase = nil;
	fManager = inManager;
	fUserTable = nil;
	fGroupTable = nil;
	fCoupleUGTable = nil;
	fCoupleGGTable = nil;

	fUserModel = nil;
	fGroupModel = nil;
	fCoupleUGModel = nil;
	fCoupleGGModel = nil;
	fContext = nil;

	fXmlFile = nil;
	fLoginPromote = nil;

	fNoAdmin = true;

	fAdminGroupID.FromLong((sLONG)AdminGroup);
	fDebugGroupID.FromLong((sLONG)DebuggerGroup);

}


UAGDirectory::~UAGDirectory()
{
	for (StorageMap::iterator cur = fUserStorages.begin(), end = fUserStorages.end(); cur != end; ++cur)
	{
		cur->second->Release();
	}
	QuickReleaseRefCountable(fLoginPromote);
	QuickReleaseRefCountable(fXmlFile);
	CloseDB();
}


VError UAGDirectory::CloseDB()
{
	QuickReleaseRefCountable(fUserModel);
	QuickReleaseRefCountable(fGroupModel);
	QuickReleaseRefCountable(fCoupleUGModel);
	QuickReleaseRefCountable(fCoupleGGModel);

	QuickReleaseRefCountable(fContext);

	if (fBase != nil)
	{
		//fBase->Close();
		fBase->CloseAndRelease(false);
		fBase = nil;
	}
	return VE_OK;
}


VError UAGDirectory::ClearDB(CDB4DBaseContext* context)
{
	VError err = VE_OK;
	if (fBase != nil)
	{
		/*
		err = fUserTable->Truncate(context);
		err = fGroupTable->Truncate(context);
		err = fCoupleUGTable->Truncate(context);
		err = fCoupleGGTable->Truncate(context);
		*/
	}

	return err;
}




class UAGJSRuntimeDelegate : public IJSRuntimeDelegate
{
public:

	virtual	VFolder* RetainScriptsFolder()
	{
		return nil;
	}

	virtual VProgressIndicator* CreateProgressIndicator( const VString& inTitle)
	{
		return nil;
	}

};


VError UAGDirectory::Save(VFile* inFile )
{
	VError err = VE_OK;

	if (inFile == nil)
		inFile = fXmlFile;
	if (inFile != nil)
	{
		/*fBase refCount++ (3)*/		
		CDB4DManager* db4d = fManager->GetDB4D();

		UAGJSRuntimeDelegate* xJSDelegate;
		VJSGlobalContext* xJSContext;

		xJSDelegate = new UAGJSRuntimeDelegate();
		xJSContext = VJSGlobalContext::Create(xJSDelegate);
		VJSContext jscontext(xJSContext);

		VUUID uuid( true);
		CDB4DContext *db4dContext = db4d->RetainOrCreateContext( uuid, NULL, xJSContext);
		CDB4DBaseContext* xcontext = db4dContext->RetainDataBaseContext( fBase, true, true);

		db4d->InitializeJSContext( xJSContext, db4dContext);

		VJSObject globalObject( jscontext.GetGlobalObject());

		VFolder* managerResourceFolder = fManager->RetainFolder(kBF_RESOURCES_FOLDER);
		if (managerResourceFolder != nil)
		{
			VFile fileJs(*managerResourceFolder, "importUAG.js");
			if (fileJs.Exists())
			{
				StErrorContextInstaller errs(false);
				VJSValue result(jscontext);
				jscontext.EvaluateScript(&fileJs, &result, nil, nil);
				/*fBase refCount++ (4)*/				
				VJSValue jsDB(db4d->CreateJSDatabaseObject(xJSContext, xcontext));
				VString spath;
				VJSValue jsPath(jscontext);
				inFile->GetPath(spath, FPS_POSIX);
				jsPath.SetString(spath);
				vector<VJSValue> paramsValues;
				paramsValues.push_back(jsDB);
				paramsValues.push_back(jsPath);

				jscontext.EvaluateScript("exportUAG", nil, &result, nil, nil);
				if (!result.IsUndefined() && result.IsObject())
				{
					VJSObject objfunc(jscontext);
					result.GetObject(objfunc);
					if (objfunc.IsFunction())
					{
						JS4D::ExceptionRef except = nil;
						globalObject.CallFunction(objfunc, &paramsValues, &result, &except);
						if (except != nil)
						{
							VJSValue eVal(jscontext, except);
							VJSJSON json(jscontext);
							VString s;
							json.Stringify(eVal, s);
							sLONG xdebug = 1;
						}
					}
				}

			}  
			managerResourceFolder->Release();
		}

		db4d->UninitializeJSContext( xJSContext);

		//QuickReleaseRefCountable(xJSContext); 
		delete xJSDelegate;
		//jscontext. GarbageCollect ( );
		xcontext-> Release ( );

		db4dContext->Release();
	}

	return err;
}


VError UAGDirectory::OpenDB(const VFile* xmlFile, const VFolder& parent, const VString& inDBName, FileAccess inAccess, bool* outWasJustCreated, bool buildEmpyDir)
{
	VError err = VE_OK;
	bool ok = false;
	bool justcreated = true;

	CDB4DManager* db4d = fManager->GetDB4D();
	if (db4d == nil)
		err = ThrowError(VE_UAG_DB4D_NOT_LOADED);
	else
	{
		inAccess = FA_READ_WRITE;
		VFolder* managerResourceFolder = fManager->RetainFolder(kBF_RESOURCES_FOLDER);
		if (managerResourceFolder != nil)
		{
			VFile UAGModelFile(*managerResourceFolder, L"uag.waModel");
			VFile UAGDataFile(parent, inDBName);
			if (UAGDataFile.Exists())
			{
				err = UAGDataFile.Delete();
				xbox_assert ( err == VE_OK );
			}
			if (err == VE_OK)
			{
				fBase = db4d->OpenBase(UAGModelFile, DB4D_Open_As_XML_Definition | DB4D_Open_No_Respart, &err, FA_READ_WRITE);
				if (fBase != nil)
				{
					if (fBase->CreateData(UAGDataFile, DB4D_Create_AllDefaultParamaters, nil, nil, &err, inAccess))
					{
						fUserModel = fBase->RetainEntityModel(L"User", true);
						fGroupModel = fBase->RetainEntityModel(L"Group", true);
						fCoupleUGModel = fBase->RetainEntityModel(L"Usergroup", true);
						fCoupleGGModel = fBase->RetainEntityModel(L"Groupgroup", true);

						if (fUserModel == nil || fGroupModel == nil || fCoupleUGModel == nil || fCoupleGGModel == nil)
						{
							err = ThrowError(VE_UAG_UAGDB_MALFORMED);
							ok = false;
						}
						else
						{
/*fBase refCount++ (2)*/	fContext = fBase->NewContext(nil, nil);
							ok = true;
							if (xmlFile != nil)
							{
								QuickReleaseRefCountable(fXmlFile);
								fXmlFile = new VFile(*xmlFile);
								if (xmlFile->Exists())
								{
									err = ImportXMLData(*xmlFile, fContext);
									if (err != VE_OK)
										ok = false;
								}
								else
								{
									if (!buildEmpyDir)
										err = ThrowError(VE_UAG_FILE_IS_MISSING);
								}
							}
						}

					}					

					if (!ok)
					{
						CloseDB();
					}
				}
			}
		}
		else
			err = ThrowError(VE_UAG_MODEL_IS_MISSING);
	}


	if (err != VE_OK)
		err = ThrowError(VE_UAG_UAGDB_NOT_LOADED);


	if (outWasJustCreated != nil)
		*outWasJustCreated = true;

	return err;
}


typedef map<VUUID, VValueBag*> unresolvedIDs;
typedef map<VString, VValueBag*> unresolvedNames;

/*
class unresolvedUser
{
public:
	VString name;
	VUUID id;
	unresolvedIDs fGroupIDs;
	unresolvedNames fGroupNames;

};

class unresolvedGroup
{
public:
	VString name;
	VUUID id;
	unresolvedIDs fGroupIDs;
	unresolvedNames fGroupNames;
	unresolvedIDs fUserIDs;
	unresolvedNames fUserNames;

};

typedef map<VUUID, unresolvedUser> UsersByID;
typedef map<VString, unresolvedUser> UsersByName;

typedef map<VUUID, unresolvedGroup> GroupsByID;
typedef map<VString, unresolvedGroup> GroupsByName;
*/

CDB4DBase* UAGDirectory::RetainAndExposeDB()
{
	return RetainRefCountable(fBase);
}

#if 0
// old code 
VError UAGDirectory::ImportXMLData(const VFile& inFile, CDB4DBaseContext* context)
{
	VError err = VE_OK;

	if (fBase != nil && inFile.Exists())
	{
		VValueBag bag;
		err = LoadBagFromXML(inFile, L"UAG", bag);
		if (err == VE_OK)
		{
			unresolvedIDs usersByID;
			unresolvedNames usersByName;
			unresolvedIDs groupsByID;
			unresolvedNames groupsByName;

			VBagArray* users = bag.GetElements(uag::user);
			if (users != nil)
			{
				for (sLONG i = 1, nb = users->GetCount(); i <= nb && err == VE_OK; i++)
				{
					VUUID userid;
					VString username;

					VValueBag* userbag = users->GetNth(i);
					if (!userbag->GetVUUID(uag::id, userid))
					{
						userid.Regenerate();
						userbag->SetVUUID(uag::id, userid);
					}
					if (!userbag->GetString(uag::name, username))
					{
						err = ThrowError(VE_UAG_USERNAME_IS_MISSING);
					}

					if (err == VE_OK)
					{
						if (usersByID.find(userid) != usersByID.end())
						{
							VString ids;
							userid.GetString(ids);
							err = ThrowError(VE_UAG_USERID_ALREADY_EXISTS, ids);
						}
						if (usersByName.find(username) != usersByName.end())
							err = ThrowError(VE_UAG_USERNAME_ALREADY_EXISTS, username);
					}

					if (err == VE_OK)
					{
						usersByID[userid] = userbag;
						usersByName[username] = userbag;
					}
				}
			}

			VBagArray* groups = bag.GetElements(uag::group);
			if (groups != nil && err == VE_OK)
			{
				for (sLONG i = 1, nb = groups->GetCount(); i <= nb && err == VE_OK; i++)
				{
					VUUID groupid;
					VString groupname;

					VValueBag* groupbag = groups->GetNth(i);
					if (!groupbag->GetVUUID(uag::id, groupid))
					{
						groupid.Regenerate();
						groupbag->SetVUUID(uag::id, groupid);

					}
					if (!groupbag->GetString(uag::name, groupname))
					{
						err = ThrowError(VE_UAG_GROUPNAME_IS_MISSING);
					}

					if (err == VE_OK)
					{
						if (groupsByID.find(groupid) != groupsByID.end())
						{
							VString ids;
							groupid.GetString(ids);
							err = ThrowError(VE_UAG_GROUPID_ALREADY_EXISTS, ids);
						}
						if (groupsByName.find(groupname) != groupsByName.end())
							err = ThrowError(VE_UAG_GROUPNAME_ALREADY_EXISTS, groupname);
					}

					if (err == VE_OK)
					{
						groupsByID[groupid] = groupbag;
						groupsByName[groupname] = groupbag;
					}
				}
			}

			if (err == VE_OK)
			{
				err = ClearDB(context);
			}

			if (err == VE_OK)
			{
				for (unresolvedIDs::iterator cur = usersByID.begin(), end = usersByID.end(); cur != end && err == VE_OK; cur++)
				{
					const VUUID* userid = &(cur->first);
					CDB4DEntityRecord* userrec = fUserModel->NewEntity(context, DB4D_Do_Not_Lock);
					userrec->SetAttributeValue(L"ID", userid);
					VValueBag* userbag = cur->second;
					userrec->SetAttributeValue(L"name", userbag->GetAttribute(uag::name));
					userrec->SetAttributeValue(L"fullName", userbag->GetAttribute(uag::fullName));
					userrec->SetAttributeValue(L"password", userbag->GetAttribute(uag::password));
					userrec->Save(0);

					VBagArray* ownergroups = userbag->GetElements(uag::group);
					if (ownergroups != nil)
					{
						for (sLONG j = 1, nb2 = ownergroups->GetCount(); j <= nb2 && err == VE_OK; j++)
						{
							VValueBag* groupbag = nil;
							VValueBag* ownergroupbag = ownergroups->GetNth(j);
							VUUID gid;
							VString gname;
							if (ownergroupbag->GetVUUID(uag::id, gid))
							{
								unresolvedIDs::iterator found = groupsByID.find(gid);
								if (found == groupsByID.end())
								{
									VString ids;
									gid.GetString(ids);
									err = ThrowError(VE_UAG_GROUPID_DOES_NOT_EXIST, ids);
								}
								else
									groupbag = found->second;
							}
							else if (ownergroupbag->GetString(uag::name, gname))
							{
								unresolvedNames::iterator found = groupsByName.find(gname);
								if (found == groupsByName.end())
								{
									err = ThrowError(VE_UAG_GROUPID_DOES_NOT_EXIST, gname);
								}
								else
								{
									groupbag = found->second;
									groupbag->GetVUUID(uag::id, gid);
								}
							}
							else
							{
								err = ThrowError(VE_UAG_GROUPNAME_IS_MISSING);
							}

							if (err == VE_OK)
							{
								CDB4DEntityRecord* rec = fCoupleUGModel->NewEntity(context, DB4D_Do_Not_Lock);
								rec->SetAttributeValue(L"userID", userid);
								rec->SetAttributeValue(L"groupOwnerID", &gid);
								rec->Save(0);
							}
						}
					}

					if (err != VE_OK)
					{
						VString username;
						userbag->GetString(uag::name, username);
						err = ThrowError(VE_UAG_USER_IS_MALFORMED, username);
					}
				}
			}

			if (err == VE_OK)
			{
				for (unresolvedIDs::iterator cur = groupsByID.begin(), end = groupsByID.end(); cur != end && err == VE_OK; cur++)
				{
					const VUUID* groupid = &(cur->first);
					CDB4DEntityRecord* grouprec = fGroupModel->NewEntity(context, DB4D_Do_Not_Lock);
					grouprec->SetAttributeValue(L"ID", groupid);
					VValueBag* groupbag = cur->second;
					grouprec->SetAttributeValue(L"name", groupbag->GetAttribute(uag::name));
					grouprec->SetAttributeValue(L"fullName", groupbag->GetAttribute(uag::fullName));
					grouprec->Save(0);

					VBagArray* ownedgroups = groupbag->GetElements(uag::group);
					if (ownedgroups != nil)
					{
						for (sLONG j = 1, nb2 = ownedgroups->GetCount(); j <= nb2 && err == VE_OK; j++)
						{
							VValueBag* groupbag = nil;
							VValueBag* ownedgroupbag = ownedgroups->GetNth(j);
							VUUID gid;
							VString gname;
							if (ownedgroupbag->GetVUUID(uag::id, gid))
							{
								unresolvedIDs::iterator found = groupsByID.find(gid);
								if (found == groupsByID.end())
								{
									VString ids;
									gid.GetString(ids);
									err = ThrowError(VE_UAG_GROUPID_DOES_NOT_EXIST, ids);
								}
								else
								{
									groupbag = found->second;
									groupbag->GetVUUID(uag::id, gid);
								}
							}
							else if (ownedgroupbag->GetString(uag::name, gname))
							{
								unresolvedNames::iterator found = groupsByName.find(gname);
								if (found == groupsByName.end())
								{
									err = ThrowError(VE_UAG_GROUPNAME_DOES_NOT_EXIST, gname);
								}
								else
									groupbag = found->second;
							}
							else
							{
								err = ThrowError(VE_UAG_GROUPNAME_IS_MISSING);
							}

							if (err == VE_OK)
							{
								CDB4DEntityRecord* rec = fCoupleGGModel->NewEntity(context, DB4D_Do_Not_Lock);
								rec->SetAttributeValue(L"groupID", &gid);
								rec->SetAttributeValue(L"groupOwnerID", groupid);
								rec->Save(0);
							}
						}
					}

					VBagArray* ownedusers = groupbag->GetElements(uag::user);
					if (ownedusers != nil)
					{
						for (sLONG j = 1, nb2 = ownedusers->GetCount(); j <= nb2 && err == VE_OK; j++)
						{
							VValueBag* userbag = nil;
							VValueBag* owneduserbag = ownedusers->GetNth(j);
							VUUID uid;
							VString uname;
							if (owneduserbag->GetVUUID(uag::id, uid))
							{
								unresolvedIDs::iterator found = usersByID.find(uid);
								if (found == usersByID.end())
								{
									VString ids;
									uid.GetString(ids);
									err = ThrowError(VE_UAG_USERID_DOES_NOT_EXIST, ids);
								}
								else
									userbag = found->second;
							}
							else if (owneduserbag->GetString(uag::name, uname))
							{
								unresolvedNames::iterator found = usersByName.find(uname);
								if (found == usersByName.end())
								{
									err = ThrowError(VE_UAG_USERNAME_DOES_NOT_EXIST, uname);
								}
								else
								{
									userbag = found->second;
									userbag->GetVUUID(uag::id, uid);
								}
							}
							else
							{
								err = ThrowError(VE_UAG_USERNAME_IS_MISSING);
							}

							if (err == VE_OK)
							{
								CDB4DEntityRecord* rec = fCoupleUGModel->NewEntity(context, DB4D_Do_Not_Lock);
								rec->SetAttributeValue(L"userID", &uid);
								rec->SetAttributeValue(L"groupOwnerID", groupid);
								rec->Save(0);
							}
						}
					}

				}
			}

		}

		
	}

	return err;
}

#endif


VError UAGDirectory::ImportXMLData(const VFile& inFile, CDB4DBaseContext* context)
{
	VError err = VE_OK;

	CDB4DManager* db4d = fManager->GetDB4D();

	if (fBase != nil && inFile.Exists() && db4d != nil)
	{
/*fBase refCount++ (3)*/

		UAGJSRuntimeDelegate* xJSDelegate;
		VJSGlobalContext* xJSContext;

		xJSDelegate = new UAGJSRuntimeDelegate();
		xJSContext = VJSGlobalContext::Create(xJSDelegate);
		VJSContext jscontext(xJSContext);

		VUUID uuid( true);
		CDB4DContext *db4dContext = db4d->RetainOrCreateContext( uuid, NULL, xJSContext);
		CDB4DBaseContext* xcontext = db4dContext->RetainDataBaseContext( context->GetOwner(), true, true);

		db4d->InitializeJSContext( xJSContext, db4dContext);

		VJSObject globalObject( jscontext.GetGlobalObject());

		VFolder* managerResourceFolder = fManager->RetainFolder(kBF_RESOURCES_FOLDER);
		if (managerResourceFolder != nil)
		{
			VFile fileJs(*managerResourceFolder, "importUAG.js");
			if (fileJs.Exists())
			{
				StErrorContextInstaller errs(false);
				VJSValue result(jscontext);
				jscontext.EvaluateScript(&fileJs, &result, nil, nil);
				/*fBase refCount++ (4)*/				
				VJSValue jsDB(db4d->CreateJSDatabaseObject(xJSContext, xcontext));
				VString spath;
				inFile.GetPath(spath, FPS_POSIX);
				VJSValue jsPath(jscontext);
				jsPath.SetString(spath);
				vector<VJSValue> paramsValues;
				paramsValues.push_back(jsDB);
				paramsValues.push_back(jsPath);

				jscontext.EvaluateScript("importUAG", nil, &result, nil, nil);
				if (!result.IsUndefined() && result.IsObject())
				{
					VJSObject objfunc(jscontext);
					result.GetObject(objfunc);
					if (objfunc.IsFunction())
					{
						JS4D::ExceptionRef except = nil;
						globalObject.CallFunction(objfunc, &paramsValues, &result, &except);
						if (except != nil)
						{
							VJSValue eVal(jscontext, except);
							VJSJSON json(jscontext);
							VString s;
							json.Stringify(eVal, s);
							sLONG xdebug = 1;
						}
#if debuglr == 2
						CDB4DSelection* sel = fUserModel->SelectAllEntities(xcontext);
						for (sLONG i = 1, nb = sel->CountRecordsInSelection(xcontext); i <= nb; i++)
						{
							CDB4DEntityRecord* rec = sel->LoadEntity(i, DB4D_Do_Not_Lock, xcontext);
							if (rec != nil)
							{
								CDB4DEntityAttributeValue* xval = rec->GetAttributeValue("name", err);
								if (xval != nil)
								{
									VValueSingle* cv = xval->GetVValue();
									if (cv != nil)
									{
										VString s;
										cv->GetString(s);
										DebugMsg("user: "+s+"\n");
									}
								}
								rec->Release();
							}

						}
						QuickReleaseRefCountable(sel);
#endif
					}
				}

			}
			managerResourceFolder->Release();
		}

		db4d->UninitializeJSContext( xJSContext);

		QuickReleaseRefCountable(xJSContext); 
		delete xJSDelegate;
		//jscontext. GarbageCollect ( );

		xcontext-> Release ( );

		db4dContext->Release();
	}

	return err;
}




VError UAGDirectory::LoadUsersAndGroups(const VFile& inFile)
{
	VError err = VE_UAG_NOT_IMPLEMENTED;
	if (fBase != nil)
	{
		StDBContext context(fBase);
		err = ImportXMLData(inFile, *context);
	}
	return err;
}

VError UAGDirectory::LoadUsersAndGroups(const VValueBag& inBag)
{
	return VE_UAG_NOT_IMPLEMENTED;
}


CUAGGroup* UAGDirectory::RetainGroup(const VString& inGroupName, VError* outerr)
{
	return retainGroup(inGroupName, outerr);
}


CUAGGroup* UAGDirectory::RetainGroup(const VUUID& inGroupID, VError* outerr)
{
	return retainGroup(inGroupID, outerr);
}

CUAGGroup* UAGDirectory::RetainSpecialGroup(CUAGDirectory::SpecialGroupEnum inGroupRef, VError* outerr)
{
	VUUIDBuffer buf;
	buf.FromLong((sLONG)inGroupRef);
	VUUID xid(buf);
	return retainGroup(xid, outerr);
}


bool UAGDirectory::GetSpecialGroupID(CUAGDirectory::SpecialGroupEnum inGroupRef, VUUID& outGroupID)
{
	VUUIDBuffer buf;
	buf.FromLong((sLONG)inGroupRef);
	outGroupID.FromBuffer(buf);
	return true;
}


bool UAGDirectory::GetGroupID(const VString& inGroupName, VUUID& outGroupID)
{
	StErrorContextInstaller errs(false);
	CUAGGroup* group = RetainGroup(inGroupName);
	if (group == nil)
		return false;
	else
	{
		group->GetID(outGroupID);
		group->Release();
		return true;
	}
}


CUAGUser* UAGDirectory::RetainUser(const VString& inUserName, VError* outerr )
{
	return retainUser(inUserName, outerr);
}


CUAGUser* UAGDirectory::RetainUser(const VUUID& inUserID, VError* outerr)
{
	return retainUser(inUserID, outerr);
}


bool UAGDirectory::GetUserID(const VString& inUserName, VUUID& outUserID)
{
	StErrorContextInstaller errs(false);
	CUAGUser* user = RetainUser(inUserName);
	if (user == nil)
		return false;
	else
	{
		user->GetID(outUserID);
		user->Release();
		return true;
	}
}


UAGGroup* UAGDirectory::retainGroup(const VString& inGroupName, VError* outerr)
{
	UAGGroup* group = nil;
	VError err = VE_OK;

	CDB4DEntityRecord* grouprec = fGroupModel->Find(L"name = :p1", fContext, err, &inGroupName);
	if (grouprec != nil)
	{
		group = new UAGGroup(this, grouprec);
		grouprec->Release();
	}
	else
	{
		if (err == VE_OK)
		{
			err = ThrowError(VE_UAG_GROUPNAME_DOES_NOT_EXIST, inGroupName);
		}
	}

	if (outerr != nil)
		*outerr = err;

	return group;
}


UAGGroup* UAGDirectory::retainGroup(const VUUID& inGroupID, VError* outerr)
{
	UAGGroup* group = nil;
	VError err = VE_OK;

	CDB4DEntityRecord* grouprec = fGroupModel->Find(L"ID = :p1", fContext, err, &inGroupID);
	if (grouprec != nil)
	{
		group = new UAGGroup(this, grouprec);
		grouprec->Release();
	}
	else
	{
		if (err == VE_OK)
		{
			VString ids;
			inGroupID.GetString(ids);
			err = ThrowError(VE_UAG_GROUPID_DOES_NOT_EXIST, ids);
		}
	}

	if (outerr != nil)
		*outerr = err;

	return group;
}


UAGUser* UAGDirectory::retainUser(const VString& inUserName, VError* outerr)
{
	UAGUser* user = nil;
	VError err = VE_OK;

	CDB4DEntityRecord* userrec = fUserModel->Find(L"name = :p1", fContext, err, &inUserName);
	if (userrec != nil)
	{
		user = new UAGUser(this, userrec);
		userrec->Release();
	}
	else
	{
		if (err == VE_OK)
		{
			err = ThrowError(VE_UAG_USERNAME_DOES_NOT_EXIST, inUserName);
		}
	}

	if (outerr != nil)
		*outerr = err;

	return user;
}

UAGUser* UAGDirectory::retainUser(const VUUID& inUserID, VError* outerr)
{
	UAGUser* user = nil;
	VError err = VE_OK;

	CDB4DEntityRecord* userrec = fUserModel->Find(L"ID = :p1", fContext, err, &inUserID);
	if (userrec != nil)
	{
		user = new UAGUser(this, userrec);
		userrec->Release();
	}
	else
	{
		if (err == VE_OK)
		{
			VString ids;
			inUserID.GetString(ids);
			err = ThrowError(VE_UAG_USERID_DOES_NOT_EXIST, ids);
		}
	}

	if (outerr != nil)
		*outerr = err;

	return user;
}


UAGUser* UAGDirectory::BuildUser(const VValueBag* inBag, VError& err)
{
	return nil;
}

CUAGUser* UAGDirectory::AddOneUser(const VString& inUserName, const VString& inPassword, const VString& inFullName, VError& err)
{
	return addOneUser(inUserName, inPassword, inFullName, err);
}

UAGUser* UAGDirectory::addOneUser(const VString& inUserName, const VString& inPassword, const VString& inFullName, VError& err)
{
	UAGUser* user = nil;
	err = VE_OK;
	CDB4DEntityRecord* userrec = fUserModel->Find(L"name = :p1", fContext, err, &inUserName);
	if (userrec != nil)
	{
		err = ThrowError(VE_UAG_USERNAME_ALREADY_EXISTS, inUserName);
		QuickReleaseRefCountable(userrec);
	}
	else
	{
		userrec = fUserModel->NewEntity(fContext, DB4D_Do_Not_Lock);
		userrec->SetAttributeValue("name", &inUserName);
		if (!inPassword.IsEmpty())
		{
			CSecurityManager* securityManager = (CSecurityManager*)VComponentManager::RetainUniqueComponent(CSecurityManager::Component_Type);
			if (securityManager != nil)
			{
				VString realm("Wakanda");
				VString ha1 = securityManager->ComputeDigestHA1(inUserName, inPassword, realm);
				userrec->SetAttributeValue("password", &ha1);
				securityManager->Release();
			}
		}
		userrec->SetAttributeValue("fullName", &inFullName);
		userrec->Save(0);
		user = new UAGUser(this, userrec);
		userrec->Release();
	}

	return user;
}


UAGGroup* UAGDirectory::BuildGroup(const VValueBag* inBag, VError& err)
{
	return nil;
}


UAGGroup* UAGDirectory::addOneGroup(const VString& inGroupName, const VString& inFullName, VError& err, VUUID* inPredefinedID)
{
	UAGGroup* group = nil;
	err = VE_OK;
	CDB4DEntityRecord* grouprec = fGroupModel->Find(L"name = :p1", fContext, err, &inGroupName);
	if (grouprec != nil)
	{
		err = ThrowError(VE_UAG_GROUPNAME_ALREADY_EXISTS, inGroupName);
		QuickReleaseRefCountable(grouprec);
	}
	else
	{
		grouprec = fGroupModel->NewEntity(fContext, DB4D_Do_Not_Lock);
		grouprec->SetAttributeValue("name", &inGroupName);
		grouprec->SetAttributeValue("fullName", &inFullName);
		if (inPredefinedID != nil)
		{
			grouprec->SetAttributeValue("ID", inPredefinedID);
		}
		grouprec->Save(0);
		group = new UAGGroup(this, grouprec);
		grouprec->Release();
	}

	return group;
}

CUAGGroup* UAGDirectory::AddOneGroup(const VString& inGroupName, const VString& inFullName, VError& err)
{
	return addOneGroup(inGroupName, inFullName, err);
}


bool UAGDirectory::TakenByListener(const VString& inUserName, const VString& inPassword, VError& err, UAGSession* &session, VJSContext* inJSContext)
{
	bool taken = false;
	err = VE_OK;
	session = nil;

	if (!fLoginListener.IsEmpty())
	{
		if (inJSContext != nil)
		{
			VJSObject funcobj(*inJSContext);
			if (inJSContext->GetFunction(fLoginListener, funcobj))
			{
				vector<VJSValue> paramsValues;
				VJSValue vUser(*inJSContext);
				VJSValue vPass(*inJSContext);
				vUser.SetString(inUserName);
				vPass.SetString(inPassword);
				paramsValues.push_back(vUser);
				paramsValues.push_back(vPass);
				VJSValue result2(*inJSContext);
				JS4D::ExceptionRef excep2 = nil;

				sLONG promoteToken = 0;
				CUAGSession* currentSession = nil;
				if (fLoginPromote != nil)
				{
					VJSValue resSession(*inJSContext);
					inJSContext->EvaluateScript(L"currentSession()", nil, &resSession, nil, nil);
					currentSession = resSession.GetObjectPrivateData<VJSSession>();
					if (currentSession != nil)
						promoteToken = currentSession->PromoteIntoGroup(fLoginPromote);
				}

				inJSContext->GetGlobalObject().CallFunction(funcobj, &paramsValues, &result2, &excep2); // appel du listener

				if (promoteToken != 0)
				{
					currentSession->UnPromoteFromToken(promoteToken);
				}
				//QuickReleaseRefCountable(currentSession);
				
				taken = true;
				if (result2.IsObject())
				{
					VJSObject resultObj = result2.GetObject();

					VJSValue val(resultObj.GetProperty("error"));
					if (!val.IsNull() && !val.IsUndefined())
					{
						sLONG errnum = 0;
						val.GetLong(&errnum);
						if (errnum != 0)
						{
							VString errorMessage;
							VJSValue valMess(resultObj.GetProperty("errorMessage"));
							if (!valMess.IsNull() && !valMess.IsUndefined())
							{
								valMess.GetString(errorMessage);
							}
							err = vThrowUserGeneratedError(MAKE_VERROR('uaug', errnum), errorMessage);
						}
					}

					if (err == VE_OK)
					{
						bool regularuser = false;
						VString sid, sname, sfullname;
						VJSValue val(resultObj.GetProperty("ID"));
						val.GetString(sid);
						VUUID xid;
						xid.FromString(sid);
						VJSValue valName(resultObj.GetProperty("name"));
						valName.GetString(sname);
						VJSValue valFullName(resultObj.GetProperty("fullName"));
						valFullName.GetString(sfullname);
						CUAGUser* user = nil;
						if (!sid.IsEmpty())
						{
							StErrorContextInstaller errs(false);
							user = retainUser(xid);
						}
						else
						{
							StErrorContextInstaller errs(false);
							user = retainUser(sname);
						}

						if (user != nil)
						{
							/*
							err = user->ValidatePassword(inPassword);
							if (err != VE_OK)
								ReleaseRefCountable(&user);
							else
							*/
								regularuser = true;
						}

						if (user == nil && !sid.IsEmpty() && !sname.IsEmpty() && err == VE_OK)
						{
							CDB4DEntityRecord* userrec = fUserModel->NewEntity(fContext, DB4D_Do_Not_Lock);
							userrec->SetAttributeValue("ID", &xid);
							userrec->SetAttributeValue("name", &sname);
							userrec->SetAttributeValue("fullName", &sfullname);
							UAGUser* newuser = new UAGUser(this, userrec);
							QuickReleaseRefCountable(userrec);
							user = newuser;
						}

						if (user != nil)
						{
							session = new UAGSession(this, xid, user);
							if (regularuser)
								session->BuildDependences();

							VJSValue arrval(resultObj.GetProperty("belongsTo"));
							if (arrval.IsArray())
							{
								CUAGGroupVector groups;
								StErrorContextInstaller errs(false);
								VJSArray arr(arrval, false);
								for (sLONG i = 0, nb = arr.GetLength(); i < nb; ++i)
								{
									VString sgroup;
									VJSValue varitem(arr.GetValueAt(i));
									varitem.GetString(sgroup);
									VUUID gid;
									gid.FromString(sgroup);
									CUAGGroup* group = RetainGroup(gid);
									if (group == nil)
										group = RetainGroup(sgroup);
									if (group != nil)
									{
										groups.push_back(group);
										group->Release();
									}
								}
								session->BuildDependences(groups);
							}

							VJSValue storageVal(resultObj.GetProperty("storage"));
							if (storageVal.IsObject())
							{
								VJSSessionStorageObject* store = session->GetStorageObject();
								if (store != nil)
								{
									VJSObject storageObj(storageVal.GetObject());
									vector<VString> propNames;
									storageObj.GetPropertyNames(propNames);
									for (vector<VString>::iterator cur = propNames.begin(), end = propNames.end(); cur != end; ++cur)
									{
										VString propName = *cur;
										VJSValue val(storageObj.GetProperty(propName));
										store->SetKeyValue(propName, val);
									}
								}
							}

							user->Release();
						}
					}
				}
				else
				{
					if (result2.IsBoolean())
					{
						result2.GetBool(&taken);
					}
				}
			}
			else
				err = ThrowError(VE_UAG_LOGINLISTENER_NOT_FOUND);
		}
		/*
		else
			err = ThrowError(VE_UAG_LOGINLISTENER_NOT_FOUND);*/
	}
	return taken;
}


CUAGSession* UAGDirectory::MakeDefaultSession(VError* outErr, VJSContext* inJSContext)
{
	UAGSession* session = nil;
	CDB4DEntityRecord* userrec = fUserModel->NewEntity(fContext, DB4D_Do_Not_Lock);
	VUUIDBuffer zeroBuf;
	zeroBuf.FromLong(0);
	VUUID xid(zeroBuf);
	userrec->SetAttributeValue("ID", &xid);
	VString sname = "default guest";
	userrec->SetAttributeValue("name", &sname);
	VString sfullname = "default guest";
	userrec->SetAttributeValue("fullName", &sfullname);
	UAGUser* newuser = new UAGUser(this, userrec);
	QuickReleaseRefCountable(userrec);

	if (newuser != nil)
	{
		session = new UAGSession(this, xid, newuser);
		session->SetDefault(true);
		newuser->Release();
	}

	return session;
}


CUAGSession* UAGDirectory::OpenSession(const VString& inUserName, const VString& inPassword, VError* outErr, VJSContext* inJSContext)
{
	VError err = VE_OK;
	UAGSession* session = nil;

	if (!TakenByListener(inUserName, inPassword, err, session, inJSContext))
	{
		CUAGUser* user = RetainUser(inUserName, &err);
		if (user != nil)
		{
			VUUID userid;
			user->GetID(userid);

			err = user->ValidatePassword(inPassword);
			if (err == VE_OK)
			{
				session = new UAGSession(this, userid, user);
				err = session->BuildDependences();
				if (err != VE_OK)
				{
					ReleaseRefCountable(&session);
				}
			}

			user->Release();
		}
	}

	if (outErr != nil)
		*outErr = err;
	return session;
}


CUAGSession* UAGDirectory::OpenSession(const XBOX::VString& inUserAtRealm, XBOX::VError* outErr, VJSContext* inJSContext)
{
	//jmo - todo : Faire qqchose de toto@SOME_REALM !
	return OpenSession(inUserAtRealm, "kerberized", outErr);
}


CUAGSession* UAGDirectory::OpenSession(CUAGUser* inUser, VError* outErr, VJSContext* inJSContext)
{
	VError err = VE_OK;
	UAGSession* session = nil;

	xbox_assert(inUser != nil);

	VUUID userid;
	inUser->GetID(userid);
	session = new UAGSession(this, userid, inUser);
	err = session->BuildDependences();
	if (err != VE_OK)
	{
		ReleaseRefCountable(&session);
	}

	if (outErr != nil)
		*outErr = err;
	return session;
}


VJSObject UAGDirectory::CreateJSDirectoryObject( const VJSContext& inContext)
{
	return VJSDirectory::CreateInstance(inContext, this);
}


VError UAGDirectory::FilterUsers(const VString& filterString, bool isAQuery, CUAGUserVector& outUsers)
{
	VError err = VE_OK;
	CDB4DSelection* sel;
	if (isAQuery)
	{
		sel = fUserModel->Query(filterString, fContext, err);
	}
	else
	{
		VString param = filterString;
		param.AppendUniChar(fContext->GetIntlMgr()->GetWildChar());
		sel = fUserModel->Query(L"name = :p1", fContext, err, &param);
	}
	if (sel != nil)
	{
		sLONG nb = sel->CountRecordsInSelection(fContext);
		outUsers.reserve(nb);
		for (sLONG i = 1; i <= nb; ++i)
		{
			CDB4DEntityRecord* userrec = sel->LoadEntity(i, DB4D_Do_Not_Lock, fContext);
			if (userrec != nil)
			{
				CUAGUser* user = new UAGUser(this, userrec);
				userrec->Release();
				outUsers.push_back(user);
				user->Release();
			}
		}
		sel->Release();
	}
	return err;
}


VError UAGDirectory::FilterGroups(const VString& filterString, bool isAQuery, CUAGGroupVector& outGroups)
{
	VError err = VE_OK;
	CDB4DSelection* sel;
	if (isAQuery)
	{
		sel = fGroupModel->Query(filterString, fContext, err);
	}
	else
	{
		VString param = filterString;
		param.AppendUniChar(fContext->GetIntlMgr()->GetWildChar());
		sel = fGroupModel->Query(L"name = :p1", fContext, err, &param);
	}
	if (sel != nil)
	{
		sLONG nb = sel->CountRecordsInSelection(fContext);
		outGroups.reserve(nb);
		for (sLONG i = 1; i <= nb; ++i)
		{
			CDB4DEntityRecord* grouprec = sel->LoadEntity(i, DB4D_Do_Not_Lock, fContext);
			if (grouprec != nil)
			{
				CUAGGroup* group = new UAGGroup(this, grouprec);
				grouprec->Release();
				outGroups.push_back(group);
				group->Release();
			}
		}
		sel->Release();
	}
	return err;
}


VError UAGDirectory::SetLoginListener(const VString& listenerRef, const VString& promoteRef)
{
	ReleaseRefCountable(&fLoginPromote);
	if (!promoteRef.IsEmpty())
	{
		StErrorContextInstaller errs(false);
		CUAGGroup* group = RetainGroup(promoteRef, nil);
		if (group == nil)
		{
			VUUID xid;
			xid.FromString(promoteRef);
			group = RetainGroup(xid, nil);
		}
		if (group != nil)
		{
			fLoginPromote = group;
		}
	}
	fLoginListener = listenerRef;
	return VE_OK;
}


VJSSessionStorageObject* UAGDirectory::RetainUserStorage(const VUUID& inID)
{
	VJSSessionStorageObject* result = nil;
	VTaskLock lock(&fUserStoragesMutex);
	StorageMap::iterator found = fUserStorages.find(inID.GetBuffer());
	if (found != fUserStorages.end())
	{
		result = RetainRefCountable(found->second);
	}
	else
	{
		result = new VJSSessionStorageObject();
		result->Retain();
		fUserStorages.insert(make_pair(inID.GetBuffer(), result));
	}

	return result;
}


void UAGDirectory::DropUserStorage(const VUUID& inID)
{
	VTaskLock lock(&fUserStoragesMutex);
	StorageMap::iterator found = fUserStorages.find(inID.GetBuffer());
	if (found != fUserStorages.end())
	{
		found->second->Release();
		fUserStorages.erase(found);
	}
}



bool UAGDirectory::NoAdmin()
{
	return fNoAdmin;
}


void UAGDirectory::ComputeNoAdmin()
{
	fNoAdmin = true;
	CUAGGroup* admins = RetainSpecialGroup(AdminGroup);
	if (admins != nil)
	{
		CUAGUserVector users;
		admins->RetainUsers(users, false);
		if (users.size() > 0)
		{
			if (users.size() == 1)
			{
				CUAGUser* user = users[0];
				if (user->ValidatePassword("") != VE_OK)
				{
					fNoAdmin = false;
				}
			}
			else
			{
				fNoAdmin = false;
			}
		}
		admins->Release();
	}
}








