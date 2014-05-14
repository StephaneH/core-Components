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
#include "DB4D/Headers/DB4DBagKeys.h"
#include "JavaScript/Sources/VJSJSON.h"




VJSArray buildArrFromUsers(VJSParms_callStaticFunction& ioParms, CUAGUserVector& users, const VString* filter)
{
	VJSArray userArr(ioParms.GetContext());
	for (CUAGUserVector::iterator cur = users.begin(), end = users.end(); cur != end; ++cur)
	{
		if (filter == nil)
		{
			userArr.PushValue((*cur)->CreateJSUserObject(ioParms.GetContext()));
		}
		else
		{
			VString s;
			(*cur)->GetName(s);
			if (s.CompareToSameKind_Like(filter) == CR_EQUAL)
				userArr.PushValue((*cur)->CreateJSUserObject(ioParms.GetContext()));
		}
	}

	return userArr;
}


VJSArray buildArrFromGroups(VJSParms_callStaticFunction& ioParms, CUAGGroupVector groups, const VString* filter)
{
	VJSArray groupArr(ioParms.GetContext());
	for (CUAGGroupVector::iterator cur = groups.begin(), end = groups.end(); cur != end; ++cur)
	{
		if (filter == nil)
		{
			groupArr.PushValue((*cur)->CreateJSGroupObject(ioParms.GetContext()));
		}
		else
		{
			VString s;
			(*cur)->GetName(s);
			if (s.CompareToSameKind_Like(filter) == CR_EQUAL)
				groupArr.PushValue((*cur)->CreateJSGroupObject(ioParms.GetContext()));
		}
	}

	return groupArr;
}


												//------------------------




void VJSDirectory::Initialize( const VJSParms_initialize& inParms, CUAGDirectory* inDirectory)
{
	inDirectory->Retain();
}


void VJSDirectory::Finalize( const VJSParms_finalize& inParms, CUAGDirectory* inDirectory)
{
	inDirectory->Release();
}



void VJSDirectory::_getUser(VJSParms_callStaticFunction& ioParms, CUAGDirectory* inDirectory)
{
	StErrorContextInstaller errs(false, true);
	VString s;
	ioParms.GetStringParam(1, s);
	CUAGUser* user = inDirectory->RetainUser(s);
	if (user == nil)
	{
		VUUID id;
		id.FromString(s);
		user = inDirectory->RetainUser(id);
	}
	if (user == nil)
		ioParms.ReturnNullValue();
	else
	{
		ioParms.ReturnValue(VJSUser::CreateInstance(ioParms.GetContext(), user));
		user->Release();
	}
}


void VJSDirectory::_getGroup(VJSParms_callStaticFunction& ioParms, CUAGDirectory* inDirectory)
{
	StErrorContextInstaller errs(false, true);
	VString s;
	ioParms.GetStringParam(1, s);
	CUAGGroup* group = inDirectory->RetainGroup(s);
	if (group == nil)
	{
		VUUID id;
		id.FromString(s);
		group = inDirectory->RetainGroup(id);
	}
	if (group == nil)
		ioParms.ReturnNullValue();
	else
	{
		ioParms.ReturnValue(VJSGroup::CreateInstance(ioParms.GetContext(), group));
		group->Release();
	}
}


void VJSDirectory::_filterUsers(VJSParms_callStaticFunction& ioParms, CUAGDirectory* inDirectory)
{
	VString s;
	bool isquery = false;
	ioParms.GetStringParam(1, s);
	isquery = ioParms.GetBoolParam(2, "query", "not query");
	CUAGUserVector users;
	VError err = inDirectory->FilterUsers(s, isquery, users);
	
	ioParms.ReturnValue(buildArrFromUsers(ioParms, users, nil));
}


void VJSDirectory::_filterGroups(VJSParms_callStaticFunction& ioParms, CUAGDirectory* inDirectory)
{
	VString s;
	bool isquery = false;
	ioParms.GetStringParam(1, s);
	isquery = ioParms.GetBoolParam(2, "query", "not query");
	CUAGGroupVector groups;
	VError err = inDirectory->FilterGroups(s, isquery, groups);

	ioParms.ReturnValue(buildArrFromGroups(ioParms, groups, nil));
}


void VJSDirectory::_save(VJSParms_callStaticFunction& ioParms, CUAGDirectory* inDirectory)
{
	VFile* dest = ioParms.RetainFileParam(1);

	if (dest != nil || ioParms.CountParams() == 0)

		ioParms.ReturnBool(inDirectory->Save(dest) == VE_OK);

	else {
		
		XBOX::vThrowError(XBOX::VE_JVSC_WRONG_PARAMETER_TYPE_FILE, "1");
		ioParms.ReturnBool(false);

	}

	QuickReleaseRefCountable(dest);
}


void VJSDirectory::_addUser(VJSParms_callStaticFunction& ioParms, CUAGDirectory* inDirectory)
{
	VError err;
	VString username, password, fullname;
	ioParms.GetStringParam(1, username);
	ioParms.GetStringParam(2, password);
	ioParms.GetStringParam(3, fullname);
	CUAGUser* user = inDirectory->AddOneUser(username, password, fullname, err);
	if (user == nil)
		ioParms.ReturnNullValue();
	else
	{
		ioParms.ReturnValue(VJSUser::CreateInstance(ioParms.GetContext(), user));
		user->Release();
	}
	
}


void VJSDirectory::_addGroup(VJSParms_callStaticFunction& ioParms, CUAGDirectory* inDirectory)
{
	VError err;
	VString groupname, fullname;
	ioParms.GetStringParam(1, groupname);
	ioParms.GetStringParam(2, fullname);
	CUAGGroup* group = inDirectory->AddOneGroup(groupname, fullname, err);
	if (group == nil)
		ioParms.ReturnNullValue();
	else
	{
		ioParms.ReturnValue(VJSGroup::CreateInstance(ioParms.GetContext(), group));
		group->Release();
	}
}


void VJSDirectory::_setLoginListener(VJSParms_callStaticFunction& ioParms, CUAGDirectory* inDirectory)
{
	VError err;
	VString listenerRef;
	if (ioParms.IsStringParam(1))
	{
		ioParms.GetStringParam(1, listenerRef);
		VString groupRef;
		CUAGGroup* group = ioParms.GetParamObjectPrivateData<VJSGroup>(2);
		if (group == NULL)
			ioParms.GetStringParam(2, groupRef);
		else
		{
			VUUID xid;
			group->GetID(xid);
			xid.GetString(groupRef);
		}
		inDirectory->SetLoginListener(listenerRef, groupRef);
	}
	else
		XBOX::vThrowError(XBOX::VE_JVSC_WRONG_PARAMETER_TYPE_STRING, "1");
}


void VJSDirectory::_getLoginListener(VJSParms_callStaticFunction& ioParms, CUAGDirectory* inDirectory)
{
	VError err;
	VString listenerRef;
	inDirectory->GetLoginListener(listenerRef);
	ioParms.ReturnString(listenerRef);
}


void VJSDirectory::_computeHA1(VJSParms_callStaticFunction& ioParms, CUAGDirectory* inDirectory)
{
	VString userName, password, realm;
	ioParms.GetStringParam(1, userName);
	ioParms.GetStringParam(2, password);
	ioParms.GetStringParam(3, realm);
	if (realm.IsEmpty())
		realm = "Wakanda";

	CSecurityManager* securityManager = (CSecurityManager*)VComponentManager::RetainUniqueComponent(CSecurityManager::Component_Type);
	if (securityManager != nil)
	{
		VString ha1 = securityManager->ComputeDigestHA1(userName, password, realm);
		securityManager->Release();
		ioParms.ReturnString(ha1);
	}
}


void VJSDirectory::_hasAdministrator(VJSParms_callStaticFunction& ioParms, CUAGDirectory* inDirectory)
{
	ioParms.ReturnBool(! inDirectory->NoAdmin());
}


void VJSDirectory::_getInternalStore( XBOX::VJSParms_getProperty& ioParms, CUAGDirectory* inDirectory)
{
	CDB4DBase* base = inDirectory->RetainAndExposeDB();
	if (base != NULL)
	{
		ioParms.ReturnValue(base->CreateJSDatabaseObject(ioParms.GetContext()));
		ReleaseRefCountable( &base);	// sc 20/06/2012
	}
	else
		ioParms.ReturnNullValue();
}


void VJSDirectory::GetDefinition( ClassDefinition& outDefinition)
{
	static inherited::StaticFunction functions[] = 
	{
		{ "user", js_callStaticFunction<_getUser>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "group", js_callStaticFunction<_getGroup>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "filterUsers", js_callStaticFunction<_filterUsers>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "filterGroups", js_callStaticFunction<_filterGroups>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "save", js_callStaticFunction<_save>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "addUser", js_callStaticFunction<_addUser>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "addGroup", js_callStaticFunction<_addGroup>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "setLoginListener", js_callStaticFunction<_setLoginListener>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "getLoginListener", js_callStaticFunction<_getLoginListener>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "computeHA1", js_callStaticFunction<_computeHA1>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "hasAdministrator", js_callStaticFunction<_hasAdministrator>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ 0, 0, 0}
	};

	static inherited::StaticValue values[] = 
	{
		{ "internalStore", js_getProperty<_getInternalStore>, nil, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontDelete },
		{ 0, 0, 0, 0}
	};

	outDefinition.className = "Directory";
	outDefinition.initialize = js_initialize<Initialize>;
	outDefinition.finalize = js_finalize<Finalize>;
	outDefinition.staticValues = values;
	outDefinition.staticFunctions = functions;
}




// -------------------------------------- Users -----------------------------------------


CUAGUser* RetainParamUser(CUAGDirectory* directory, VJSParms_callStaticFunction& ioParms, sLONG paramNum)
{
	CUAGUser* result = nil;
	if (paramNum <= ioParms.CountParams())
	{
		if (ioParms.IsStringParam(paramNum))
		{
			StErrorContextInstaller errs(false);
			VString s;
			ioParms.GetStringParam(paramNum, s);
			result = directory->RetainUser(s);
			if (result == nil)
			{
				VUUID xid;
				xid.FromString(s);
				result = directory->RetainUser(xid);
			}
		}
		else
		{
			result = ioParms.GetParamObjectPrivateData<VJSUser>(paramNum);
			if (result != nil)
				result->Retain();
		}
	}
	return result;
}


CUAGGroup* RetainParamGroup(CUAGDirectory* directory, VJSParms_callStaticFunction& ioParms, sLONG paramNum)
{
	CUAGGroup* result = nil;
	if (paramNum <= ioParms.CountParams())
	{
		if (ioParms.IsStringParam(paramNum))
		{
			StErrorContextInstaller errs(false);
			VString s;
			ioParms.GetStringParam(paramNum, s);
			result = directory->RetainGroup(s);
			if (result == nil)
			{
				VUUID xid;
				xid.FromString(s);
				result = directory->RetainGroup(xid);
			}
		}
		else
		{
			result = ioParms.GetParamObjectPrivateData<VJSGroup>(paramNum);
			if (result != nil)
				result->Retain();
		}
	}
	return result;
}

CUAGUser* UAGDirectory::RetainParamUser(XBOX::VJSParms_callStaticFunction& ioParms, sLONG paramNum)
{
	return ::RetainParamUser(this, ioParms, paramNum);
}


CUAGGroup* UAGDirectory::RetainParamGroup(XBOX::VJSParms_callStaticFunction& ioParms, sLONG paramNum)
{
	return ::RetainParamGroup(this, ioParms, paramNum);
}


void VJSUser::Initialize( const VJSParms_initialize& inParms, CUAGUser* inUser)
{
	inUser->Retain();
}


void VJSUser::Finalize( const VJSParms_finalize& inParms, CUAGUser* inUser)
{
	inUser->Release();
}



void VJSUser::_setPassword(VJSParms_callStaticFunction& ioParms, CUAGUser* inUser)
{
	VString password;
	ioParms.GetStringParam(1, password);
	inUser->SetPassword(password);
}


VError putUserIntoGroup(VJSParms_callStaticFunction& ioParms, CUAGUser* inUser, const VString& s)
{
	VError err = VE_OK;
	CUAGDirectory* dir = inUser->GetDirectory();
	CUAGGroup* group;
	{
		StErrorContextInstaller errs(false);
		group = dir->RetainGroup(s);
		if (group == nil)
		{
			VUUID id;
			id.FromString(s);
			group = dir->RetainGroup(id);
		}
	}
	if (group != nil)
	{
		err = inUser->PutIntoGroup(group);
	}
	else
	{
		err = ThrowError(VE_UAG_GROUPNAME_DOES_NOT_EXIST, s);
	}
	QuickReleaseRefCountable(group);
	return err;
}


VError removeUserFromGroup(VJSParms_callStaticFunction& ioParms, CUAGUser* inUser, const VString& s)
{
	VError err = VE_OK;
	CUAGDirectory* dir = inUser->GetDirectory();
	CUAGGroup* group;
	{
		StErrorContextInstaller errs(false);
		group = dir->RetainGroup(s);
		if (group == nil)
		{
			VUUID id;
			id.FromString(s);
			group = dir->RetainGroup(id);
		}
	}
	if (group != nil)
	{
		err = inUser->RemoveFromGroup(group);
	}
	else
	{
		err = ThrowError(VE_UAG_GROUPNAME_DOES_NOT_EXIST, s);
	}
	QuickReleaseRefCountable(group);
	return err;
}


VError putGroupIntoGroup(VJSParms_callStaticFunction& ioParms, CUAGGroup* inGroup, const VString& s)
{
	VError err = VE_OK;
	CUAGDirectory* dir = inGroup->GetDirectory();
	CUAGGroup* group;
	{
		StErrorContextInstaller errs(false);
		group = dir->RetainGroup(s);
		if (group == nil)
		{
			VUUID id;
			id.FromString(s);
			group = dir->RetainGroup(id);
		}
	}
	if (group != nil)
	{
		err = inGroup->PutIntoGroup(group);
	}
	else
	{
		err = ThrowError(VE_UAG_GROUPNAME_DOES_NOT_EXIST, s);
	}
	QuickReleaseRefCountable(group);
	return err;
}



VError removeGroupFromGroup(VJSParms_callStaticFunction& ioParms, CUAGGroup* inGroup, const VString& s)
{
	VError err = VE_OK;
	CUAGDirectory* dir = inGroup->GetDirectory();
	CUAGGroup* group;
	{
		StErrorContextInstaller errs(false);
		group = dir->RetainGroup(s);
		if (group == nil)
		{
			VUUID id;
			id.FromString(s);
			group = dir->RetainGroup(id);
		}
	}
	if (group != nil)
	{
		err = inGroup->RemoveFromGroup(group);
	}
	else
	{
		err = ThrowError(VE_UAG_GROUPNAME_DOES_NOT_EXIST, s);
	}
	QuickReleaseRefCountable(group);
	return err;
}


void VJSUser::_putInto(VJSParms_callStaticFunction& ioParms, CUAGUser* inUser)
{
	VError err = VE_OK;
	sLONG nbparam = ioParms.CountParams();
	for (sLONG i = 1; i <= nbparam /*&& err == VE_OK*/; i++)
	{
		if (ioParms.IsArrayParam(i))
		{
			VJSArray arr(ioParms.GetContext(), nil, false);
			ioParms.GetParamArray(i, arr);
			sLONG nbelem = arr.GetLength();
			for (sLONG j = 0; j < nbelem; ++j)
			{
				VJSValue val(arr.GetValueAt(j));
				if (val.IsString())
				{
					VString s;
					val.GetString(s);
					err = putUserIntoGroup(ioParms, inUser, s);
				}
				else if (val.IsInstanceOf("Group"))
				{
					CUAGGroup* group = val.GetObjectPrivateData<VJSGroup>();
					err = inUser->PutIntoGroup(group);
				}
			}
		}
		else if (ioParms.IsStringParam(i))
		{
			VString s;
			ioParms.GetStringParam(i, s);
			err = putUserIntoGroup(ioParms, inUser, s);
		}
		else
		{
			CUAGGroup* group = ioParms.GetParamObjectPrivateData<VJSGroup>(i);
			err = inUser->PutIntoGroup(group);
		}
	}
}



void VJSUser::_removeFrom(VJSParms_callStaticFunction& ioParms, CUAGUser* inUser)
{
	VError err = VE_OK;
	sLONG nbparam = ioParms.CountParams();
	for (sLONG i = 1; i <= nbparam /*&& err == VE_OK*/; i++)
	{
		if (ioParms.IsArrayParam(i))
		{
			VJSArray arr(ioParms.GetContext(), nil, false);
			ioParms.GetParamArray(i, arr);
			sLONG nbelem = arr.GetLength();
			for (sLONG j = 0; j < nbelem; ++j)
			{
				VJSValue val(arr.GetValueAt(j));
				if (val.IsString())
				{
					VString s;
					val.GetString(s);
					err = removeUserFromGroup(ioParms, inUser, s);
				}
				else if (val.IsInstanceOf("Group"))
				{
					CUAGGroup* group = val.GetObjectPrivateData<VJSGroup>();
					err = inUser->RemoveFromGroup(group);
				}
			}
		}
		else if (ioParms.IsStringParam(i))
		{
			VString s;
			ioParms.GetStringParam(i, s);
			err = removeUserFromGroup(ioParms, inUser, s);
		}
		else
		{
			CUAGGroup* group = ioParms.GetParamObjectPrivateData<VJSGroup>(i);
			err = inUser->RemoveFromGroup(group);
		}
	}
}


void VJSUser::_getParents( XBOX::VJSParms_callStaticFunction& ioParms, CUAGUser* inUser)
{
	bool firstlevel = ioParms.GetBoolParam(1, "firstLevel", "allLevels");
	CUAGGroupVector groups;
	inUser->RetainOwners(groups, firstlevel);
	ioParms.ReturnValue(buildArrFromGroups(ioParms, groups, nil));
}


void VJSUser::_filterParents( XBOX::VJSParms_callStaticFunction& ioParms, CUAGUser* inUser)
{
	VString s;
	ioParms.GetStringParam(1,s);
	s.AppendUniChar(ioParms.GetWildChar());
	bool firstlevel = ioParms.GetBoolParam(2, "firstLevel", "allLevels");
	CUAGGroupVector groups;
	inUser->RetainOwners(groups, firstlevel);
	ioParms.ReturnValue(buildArrFromGroups(ioParms, groups, &s));
}


void VJSUser::_remove(VJSParms_callStaticFunction& ioParms, CUAGUser* inUser)
{
	inUser->Drop();
}


void VJSUser::_getName( XBOX::VJSParms_getProperty& ioParms, CUAGUser* inUser)
{
	VString s;
	inUser->GetName(s);
	ioParms.ReturnString(s);
}

void VJSUser::_getFullName( XBOX::VJSParms_getProperty& ioParms, CUAGUser* inUser)
{
	VString s;
	inUser->GetFullName(s);
	ioParms.ReturnString(s);
}

void VJSUser::_getID( XBOX::VJSParms_getProperty& ioParms, CUAGUser* inUser)
{
	VUUID id;
	inUser->GetID(id);
	VString s;
	s.FromVUUID(id);
	ioParms.ReturnString(s);
}

void VJSUser::_getStorage( XBOX::VJSParms_getProperty& ioParms, CUAGUser* inUser)
{
	VJSSessionStorageObject* storage = inUser->RetainStorageObject();
	if (storage != nil)
	{
		ioParms.ReturnValue(VJSStorageClass::CreateInstance(ioParms.GetContext(), storage));
		storage->Release();
	}
}


void VJSUser::GetDefinition( ClassDefinition& outDefinition)
{
	static inherited::StaticFunction functions[] = 
	{
		{ "setPassword", js_callStaticFunction<_setPassword>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "putInto", js_callStaticFunction<_putInto>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "removeFrom", js_callStaticFunction<_removeFrom>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "remove", js_callStaticFunction<_remove>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "getParents", js_callStaticFunction<_getParents>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },		
		{ "filterParents", js_callStaticFunction<_filterParents>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ 0, 0, 0}
	};

	static inherited::StaticValue values[] = 
	{
		{ "name", js_getProperty<_getName>, nil, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontDelete },
		{ "fullName", js_getProperty<_getFullName>, nil, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontDelete },
		{ "ID", js_getProperty<_getID>, nil, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontDelete },
		{ "storage", js_getProperty<_getStorage>, nil, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontDelete },
		{ 0, 0, 0, 0}
	};

	outDefinition.className = "User";
	outDefinition.initialize = js_initialize<Initialize>;
	outDefinition.finalize = js_finalize<Finalize>;
	outDefinition.staticValues = values;
	outDefinition.staticFunctions = functions;
}




// -------------------------------------- Groups -----------------------------------------




void VJSGroup::Initialize( const VJSParms_initialize& inParms, CUAGGroup* inGroup)
{
	inGroup->Retain();
}


void VJSGroup::Finalize( const VJSParms_finalize& inParms, CUAGGroup* inGroup)
{
	inGroup->Release();
}


void VJSGroup::_getName( XBOX::VJSParms_getProperty& ioParms, CUAGGroup* inGroup)
{
	VString s;
	inGroup->GetName(s);
	ioParms.ReturnString(s);
}

void VJSGroup::_getFullName( XBOX::VJSParms_getProperty& ioParms, CUAGGroup* inGroup)
{
	VString s;
	inGroup->GetFullName(s);
	ioParms.ReturnString(s);
}

void VJSGroup::_getID( XBOX::VJSParms_getProperty& ioParms, CUAGGroup* inGroup)
{
	VUUID id;
	inGroup->GetID(id);
	VString s;
	s.FromVUUID(id);
	ioParms.ReturnString(s);
}


void VJSGroup::_getUsers( XBOX::VJSParms_callStaticFunction& ioParms, CUAGGroup* inGroup)
{
	bool firstlevel = ioParms.GetBoolParam(1, "firstLevel", "allLevels");
	CUAGUserVector users;
	inGroup->RetainUsers(users, firstlevel);
	ioParms.ReturnValue(buildArrFromUsers(ioParms, users, nil));
}

void VJSGroup::_filterUsers( XBOX::VJSParms_callStaticFunction& ioParms, CUAGGroup* inGroup)
{
	VString s;
	ioParms.GetStringParam(1,s);
	s.AppendUniChar(ioParms.GetWildChar());
	CUAGUserVector users;
	bool firstlevel = ioParms.GetBoolParam(2, "firstLevel", "allLevels");
	inGroup->RetainUsers(users, firstlevel);
	ioParms.ReturnValue(buildArrFromUsers(ioParms, users, &s));
}


void VJSGroup::_getChildren( XBOX::VJSParms_callStaticFunction& ioParms, CUAGGroup* inGroup)
{
	bool firstlevel = ioParms.GetBoolParam(1, "firstLevel", "allLevels");
	CUAGGroupVector groups;
	inGroup->RetainSubGroups(groups, firstlevel);
	ioParms.ReturnValue(buildArrFromGroups(ioParms, groups, nil));
}

void VJSGroup::_filterChildren( XBOX::VJSParms_callStaticFunction& ioParms, CUAGGroup* inGroup)
{
	VString s;
	ioParms.GetStringParam(1,s);
	s.AppendUniChar(ioParms.GetWildChar());
	CUAGGroupVector groups;
	bool firstlevel = ioParms.GetBoolParam(2, "firstLevel", "allLevels");
	inGroup->RetainSubGroups(groups, firstlevel);
	ioParms.ReturnValue(buildArrFromGroups(ioParms, groups, &s));
}


void VJSGroup::_getParents( XBOX::VJSParms_callStaticFunction& ioParms, CUAGGroup* inGroup)
{
	bool firstlevel = ioParms.GetBoolParam(1, "firstLevel", "allLevels");
	CUAGGroupVector groups;
	inGroup->RetainOwners(groups, firstlevel);
	ioParms.ReturnValue(buildArrFromGroups(ioParms, groups, nil));
}


void VJSGroup::_filterParents( XBOX::VJSParms_callStaticFunction& ioParms, CUAGGroup* inGroup)
{
	VString s;
	ioParms.GetStringParam(1,s);
	s.AppendUniChar(ioParms.GetWildChar());
	CUAGGroupVector groups;
	bool firstlevel = ioParms.GetBoolParam(2, "firstLevel", "allLevels");
	inGroup->RetainOwners(groups, firstlevel);
	ioParms.ReturnValue(buildArrFromGroups(ioParms, groups, &s));
}



void VJSGroup::_putInto(VJSParms_callStaticFunction& ioParms, CUAGGroup* inGroup)
{
	VError err = VE_OK;
	sLONG nbparam = ioParms.CountParams();
	for (sLONG i = 1; i <= nbparam /*&& err == VE_OK*/; i++)
	{
		if (ioParms.IsArrayParam(i))
		{
			VJSArray arr(ioParms.GetContext(), nil, false);
			ioParms.GetParamArray(i, arr);
			sLONG nbelem = arr.GetLength();
			for (sLONG j = 0; j < nbelem; ++j)
			{
				VJSValue val(arr.GetValueAt(j));
				if (val.IsString())
				{
					VString s;
					val.GetString(s);
					err = putGroupIntoGroup(ioParms, inGroup, s);
				}
				else if (val.IsInstanceOf("Group"))
				{
					CUAGGroup* group = val.GetObjectPrivateData<VJSGroup>();
					err = inGroup->PutIntoGroup(group);
				}
			}
		}
		else if (ioParms.IsStringParam(i))
		{
			VString s;
			ioParms.GetStringParam(i, s);
			err = putGroupIntoGroup(ioParms, inGroup, s);
		}
		else
		{
			CUAGGroup* group = ioParms.GetParamObjectPrivateData<VJSGroup>(i);
			err = inGroup->PutIntoGroup(group);
		}
	}
}



void VJSGroup::_removeFrom(VJSParms_callStaticFunction& ioParms, CUAGGroup* inGroup)
{
	VError err = VE_OK;
	sLONG nbparam = ioParms.CountParams();
	for (sLONG i = 1; i <= nbparam /*&& err == VE_OK*/; i++)
	{
		if (ioParms.IsArrayParam(i))
		{
			VJSArray arr(ioParms.GetContext(), nil, false);
			ioParms.GetParamArray(i, arr);
			sLONG nbelem = arr.GetLength();
			for (sLONG j = 0; j < nbelem; ++j)
			{
				VJSValue val(arr.GetValueAt(j));
				if (val.IsString())
				{
					VString s;
					val.GetString(s);
					err = removeGroupFromGroup(ioParms, inGroup, s);
				}
				else if (val.IsInstanceOf("Group"))
				{
					CUAGGroup* group = val.GetObjectPrivateData<VJSGroup>();
					err = inGroup->RemoveFromGroup(group);
				}
			}
		}
		else if (ioParms.IsStringParam(i))
		{
			VString s;
			ioParms.GetStringParam(i, s);
			err = removeGroupFromGroup(ioParms, inGroup, s);
		}
		else
		{
			CUAGGroup* group = ioParms.GetParamObjectPrivateData<VJSGroup>(i);
			err = inGroup->RemoveFromGroup(group);
		}
	}
}


void VJSGroup::_remove(VJSParms_callStaticFunction& ioParms, CUAGGroup* inGroup)
{
	inGroup->Drop();
}



void VJSGroup::GetDefinition( ClassDefinition& outDefinition)
{
	static inherited::StaticFunction functions[] = 
	{
		{ "filterUsers", js_callStaticFunction<_filterUsers>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "getUsers", js_callStaticFunction<_getUsers>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "getChildren", js_callStaticFunction<_getChildren>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "filterChildren", js_callStaticFunction<_filterChildren>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "getParents", js_callStaticFunction<_getParents>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "filterParents", js_callStaticFunction<_filterParents>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "putInto", js_callStaticFunction<_putInto>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "removeFrom", js_callStaticFunction<_removeFrom>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "remove", js_callStaticFunction<_remove>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ 0, 0, 0}
	};

	static inherited::StaticValue values[] = 
	{
		{ "name", js_getProperty<_getName>, nil, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontDelete },
		{ "fullName", js_getProperty<_getFullName>, nil, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontDelete },
		{ "ID", js_getProperty<_getID>, nil, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontDelete },
		/*
		{ "users", js_getProperty<_getUsers>, nil, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontDelete },
		{ "children", js_getProperty<_getChildren>, nil, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontDelete },
		{ "parents", js_getProperty<_getParents>, nil, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontDelete },
		*/
		{ 0, 0, 0, 0}
	};

	outDefinition.className = "Group";
	outDefinition.initialize = js_initialize<Initialize>;
	outDefinition.finalize = js_finalize<Finalize>;
	outDefinition.staticValues = values;
	outDefinition.staticFunctions = functions;
}




// -------------------------------------- Sessions -----------------------------------------




void VJSSession::Initialize( const VJSParms_initialize& inParms, CUAGSession* inSession)
{
	inSession->Retain();
}


void VJSSession::Finalize( const VJSParms_finalize& inParms, CUAGSession* inSession)
{
	inSession->Release();
}


void VJSSession::_forceExpire(VJSParms_callStaticFunction& ioParms, CUAGSession* inSession)
{
	inSession->forceExpire();
}


void VJSSession::_belongsTo(VJSParms_callStaticFunction& ioParms, CUAGSession* inSession)
{
	bool ok = false;

	CUAGGroup* group = RetainParamGroup(inSession->GetDirectory(), ioParms, 1);
	if (group != nil)
	{
		ok = inSession->BelongsTo(group);
	}

	QuickReleaseRefCountable(group);
	ioParms.ReturnBool(ok);
}


void VJSSession::_checkPermission(VJSParms_callStaticFunction& ioParms, CUAGSession* inSession)
{
	bool ok = false;

	CUAGGroup* group = RetainParamGroup(inSession->GetDirectory(), ioParms, 1);
	if (group != nil)
	{
		ok = inSession->BelongsTo(group);
	}

	if (!ok)
	{
		VString groupName;
		if (group != nil)
			group->GetName(groupName);
		ThrowError(VE_UAG_SESSION_FAILED_PERMISSION, groupName);
	}

	QuickReleaseRefCountable(group);
	ioParms.ReturnBool(ok);
}


void VJSSession::_promoteWith(VJSParms_callStaticFunction& ioParms, CUAGSession* inSession)
{
	sLONG promotionToken = 0;

	CUAGGroup* group = RetainParamGroup(inSession->GetDirectory(), ioParms, 1);
	if (group != nil)
	{
		promotionToken = inSession->PromoteIntoGroup(group);
	}

	QuickReleaseRefCountable(group);
	ioParms.ReturnNumber(promotionToken);
}


void VJSSession::_unPromote(VJSParms_callStaticFunction& ioParms, CUAGSession* inSession)
{
	sLONG promotionToken = 0;
	ioParms.GetLongParam(1, &promotionToken);
	inSession->UnPromoteFromToken(promotionToken);
}


void VJSSession::_getUser( XBOX::VJSParms_getProperty& ioParms, CUAGSession* inSession)
{
	VString s;
	CUAGUser* user = inSession->RetainUser();
	if (user != nil)
		ioParms.ReturnValue(user->CreateJSUserObject(ioParms.GetContext()));
	QuickReleaseRefCountable(user);
}



void VJSSession::_getID( XBOX::VJSParms_getProperty& ioParms, CUAGSession* inSession)
{
	VString s;
	VUUID xid;
	inSession->GetID(xid);
	xid.GetString(s);
	ioParms.ReturnString(s);
}


void VJSSession::_getLifeTime( XBOX::VJSParms_getProperty& ioParms, CUAGSession* inSession)
{
	ioParms.ReturnNumber(inSession->GetLifeTime());
}


void VJSSession::_getExpiration( XBOX::VJSParms_getProperty& ioParms, CUAGSession* inSession)
{
	VTime exp;
	inSession->GetExpiration(exp);
	VJSValue exptime(ioParms.GetContext());
	exptime.SetVValue(exp);
	ioParms.ReturnValue(exptime);
}


void VJSSession::_getStorage( XBOX::VJSParms_getProperty& ioParms, CUAGSession* inSession)
{
	VString s;
	VJSSessionStorageObject* storage = inSession->GetStorageObject();
	if (storage != nil)
		ioParms.ReturnValue(VJSStorageClass::CreateInstance(ioParms.GetContext(), storage));
}




void VJSSession::GetDefinition( ClassDefinition& outDefinition)
{
	static inherited::StaticFunction functions[] = 
	{
		{ "belongsTo", js_callStaticFunction<_belongsTo>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "checkPermission", js_callStaticFunction<_checkPermission>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "promoteWith", js_callStaticFunction<_promoteWith>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "unPromote", js_callStaticFunction<_unPromote>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ "forceExpire", js_callStaticFunction<_forceExpire>, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontEnum | JS4D::PropertyAttributeDontDelete },
		{ 0, 0, 0}
	};

	static inherited::StaticValue values[] = 
	{
		{ "user", js_getProperty<_getUser>, nil, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontDelete },
		{ "ID", js_getProperty<_getID>, nil, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontDelete },
		{ "expiration", js_getProperty<_getExpiration>, nil, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontDelete },
		{ "lifeTime", js_getProperty<_getLifeTime>, nil, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontDelete },
		{ "storage", js_getProperty<_getStorage>, nil, JS4D::PropertyAttributeReadOnly | JS4D::PropertyAttributeDontDelete },
		{ 0, 0, 0, 0}
	};

	outDefinition.className = "ConnectionSession";
	outDefinition.initialize = js_initialize<Initialize>;
	outDefinition.finalize = js_finalize<Finalize>;
	outDefinition.staticValues = values;
	outDefinition.staticFunctions = functions;
}