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



UAGUser::UAGUser(UAGDirectory* inDirectory, CDB4DEntityRecord* inUserRec)
{
	fDirectory = RetainRefCountable(inDirectory);
	fUserRec = RetainRefCountable(inUserRec);
}


UAGUser::~UAGUser()
{
	QuickReleaseRefCountable(fUserRec);
	QuickReleaseRefCountable(fDirectory);
}


CUAGDirectory* UAGUser::GetDirectory()
{
	return fDirectory;
}


VError UAGUser::GetID(VUUID& outID)
{
	VError err;
	VTaskLock lock(&fMutex);
	CDB4DEntityAttributeValue* xval = fUserRec->GetAttributeValue(L"ID", err);
	if (xval != nil)
	{
		VValueSingle* val = xval->GetVValue();
		if (val != nil)
			val->GetVUUID(outID);
	}
	return err;
}


VError UAGUser::GetName(VString& outName)
{
	VError err;
	VTaskLock lock(&fMutex);
	CDB4DEntityAttributeValue* xval = fUserRec->GetAttributeValue(L"name", err);
	if (xval != nil)
	{
		VValueSingle* val = xval->GetVValue();
		if (val != nil)
			val->GetString(outName);
	}
	return err;
}


VError UAGUser::GetFullName(VString& outName)
{
	VError err;
	VTaskLock lock(&fMutex);
	CDB4DEntityAttributeValue* xval = fUserRec->GetAttributeValue(L"fullName", err);
	if (xval != nil)
	{
		VValueSingle* val = xval->GetVValue();
		if (val != nil)
			val->GetString(outName);
	}
	return err;
}


VError UAGUser::GetHA1(VString& outValue)
{
	VError err;
	VTaskLock lock(&fMutex);
	CDB4DEntityAttributeValue* xval = fUserRec->GetAttributeValue (L"password", err);
	if (xval != nil)
	{
		VValueSingle* val = xval->GetVValue();
		if (val != nil)
			val->GetString (outValue);
	}
	return err;
}


VError UAGUser::SetPassword(VString& inPassword)
{
	VError err;
	CSecurityManager* securityManager = (CSecurityManager*)VComponentManager::RetainUniqueComponent(CSecurityManager::Component_Type);
	if (securityManager != nil)
	{
		VTaskLock lock(&fMutex);
		VString username;
		VString realm("Wakanda");
		GetName(username);
		VString ha1 = securityManager->ComputeDigestHA1(username, inPassword, realm);
		securityManager->Release();

		err = fUserRec->SetAttributeValue(L"password", &ha1);
		if (err == VE_OK)
			err = fUserRec->Save(fUserRec->GetModificationStamp());
	}
	else
		err = -1;
	return err;
}



VError UAGUser::RetainOwners(CUAGGroupVector& outgroups, bool oneLevelDeep)
{
	VTaskLock lock(&fMutex);
	outgroups.clear();
	VError err = VE_OK;
	if (oneLevelDeep)
	{
		fUserRec->ResetAttributeValue("groups");
		fUserRec->ResetAttributeValue("usergroups");
		CDB4DEntityAttributeValue* xval = fUserRec->GetAttributeValue(L"groups", err);
		if (xval != nil)
		{
			CDB4DBaseContext* context = fUserRec->GetContext();
			CDB4DEntityCollection* sel = xval->GetRelatedSelection();
			if (sel != nil)
			{
				sLONG nb = sel->GetLength(context);
				outgroups.resize(nb, nil);
				for (sLONG i = 0; i < nb && err == VE_OK; i++)
				{
					CDB4DEntityRecord* gowner = sel->LoadEntityRecord(i, context, err, DB4D_Do_Not_Lock);
					if (gowner != nil)
					{
						CUAGGroup* group = new UAGGroup(fDirectory, gowner);
						outgroups[i].Adopt(group);
						gowner->Release();
					}
				}
			}
		}
	}
	else
	{
		VString root = "parents";
		set<VUUIDBuffer> alreadyGroup;
		CUAGGroupVector firstLevelGroups;
		//err = RetainSubGroups(firstLevelGroups, true);
		err = RetainOwners(firstLevelGroups, true);
		if (err == VE_OK)
		{
			for (CUAGGroupVector::iterator curg = firstLevelGroups.begin(), endg = firstLevelGroups.end(); curg != endg && err == VE_OK; ++curg)
			{
				VUUID xid;
				CUAGGroup* fgroup = *curg;
				UAGGroup* xfgroup = VImpCreator<UAGGroup>::GetImpObject(fgroup);
				fgroup->GetID(xid);
				if (alreadyGroup.find(xid.GetBuffer()) == alreadyGroup.end())
				{
					outgroups.push_back(fgroup);
					alreadyGroup.insert(xid.GetBuffer());
					CUAGGroupVector otherGroups;
					//err = fgroup->RetainSubGroups(otherGroups, false);
					err = xfgroup->RetainOtherGroups(otherGroups, false, root);
					if (err == VE_OK)
					{
						for (CUAGGroupVector::iterator cur = otherGroups.begin(), end = otherGroups.end(); cur != end; ++cur)
						{
							CUAGGroup* otherGroup = *cur;
							VUUID xid2;
							otherGroup->GetID(xid2);
							if (alreadyGroup.find(xid2.GetBuffer()) == alreadyGroup.end())
							{
								outgroups.push_back(otherGroup);
								alreadyGroup.insert(xid2.GetBuffer());						
							}
						}
					}

				}
			}		
		}
	}

	return err;
}


VValueBag* UAGUser::RetainExtraProperties()
{
	return nil;
}


VError UAGUser::FromBag(const VValueBag* inBag)
{
	VError err = VE_OK;
	/*
	fName.Clear();
	if (!inBag->GetString(uag::name, fName))
	{
		err = ThrowError(VE_UAG_USERNAME_IS_MISSING);
	}
	else
	{
		VString ids;
		if (inBag->GetString(uag::id, ids))
			fID.FromString(ids);
		else
			fID.Regenerate();
		inBag->GetString(uag::fullName, fFullName);
		inBag->GetString(uag::password, fPassword);
	}
	*/

	return err;

}


VError UAGUser::ValidatePassword(const VString& inPassword)
{
	VTaskLock lock(&fMutex);
	StErrorContextInstaller errs(false);
	VError err = VE_UAG_PASSWORD_DOES_NOT_MATCH;
	VError err2;
	CDB4DEntityAttributeValue* val = fUserRec->GetAttributeValue(L"password", err2);
	if (val != nil)
	{
		VValueSingle* cv = val->GetVValue();
		if (cv != nil)
		{
			VString s;
			cv->GetString(s);
			if (s.IsEmpty() && inPassword.IsEmpty())
				err = VE_OK;
			else
			{
				CSecurityManager* securityManager = (CSecurityManager*)VComponentManager::RetainUniqueComponent(CSecurityManager::Component_Type);
				if (securityManager != nil)
				{
					VString username;
					VString realm("Wakanda");
					GetName(username);
					VString ha1 = securityManager->ComputeDigestHA1(username, inPassword, realm);
					securityManager->Release();

					if (s.EqualToString(ha1, true))
						err = VE_OK;
				}
			}
		}
	}
	return err;
}


VJSObject UAGUser::CreateJSUserObject(const VJSContext& inContext)
{
	return VJSUser::CreateInstance(inContext, this);
}


VError UAGUser::PutIntoGroup( CUAGGroup* group)
{
	VError err = VE_OK;
	if (group != nil)
	{
		VTaskLock lock(&fMutex);
		CDB4DEntityModel* ug = fDirectory->GetCoupleUGModel();

		VString userID, groupID;

		UAGGroup* xgroup = VImpCreator<UAGGroup>::GetImpObject(group);
		CDB4DEntityRecord* grouprec = xgroup->GetEntity();
		grouprec->GetAttributeValue("ID", err)->GetVValue()->GetString(groupID);
		
		fUserRec->GetAttributeValue("ID", err)->GetVValue()->GetString(userID);

		CDB4DEntityRecord* ugCouple = ug->Find("user.ID = :p1 and group.ID = :p2", fDirectory->GetDBContext(), err, &userID, &groupID);
		if (ugCouple == nil)
		{
			ugCouple = ug->NewEntity(fDirectory->GetDBContext());
			ugCouple->SetAttributeValue("user", fUserRec);
			ugCouple->SetAttributeValue("group", grouprec);
			ugCouple->Save(0);
		}
		QuickReleaseRefCountable(ugCouple);
	}
	return err;
}



VError UAGUser::RemoveFromGroup( CUAGGroup* group)
{
	VError err = VE_OK;
	if (group != nil)
	{
		VTaskLock lock(&fMutex);
		CDB4DEntityModel* ug = fDirectory->GetCoupleUGModel();

		VString userID, groupID;

		UAGGroup* xgroup = VImpCreator<UAGGroup>::GetImpObject(group);
		CDB4DEntityRecord* grouprec = xgroup->GetEntity();
		grouprec->GetAttributeValue("ID", err)->GetVValue()->GetString(groupID);

		fUserRec->GetAttributeValue("ID", err)->GetVValue()->GetString(userID);

		CDB4DEntityCollection* ugCouples = ug->Query("user.ID = :p1 and group.ID = :p2", fDirectory->GetDBContext(), err, &userID, &groupID);
		if (ugCouples != nil)
		{
			err = ugCouples->DeleteEntities(fDirectory->GetDBContext());
			ugCouples->Release();
		}
	}
	return err;
}


VError UAGUser::Drop()
{
	VUUID id;
	GetID(id);

	VError err = VE_OK;
	CDB4DEntityModel* ug = fDirectory->GetCoupleUGModel();

	{
		VTaskLock lock(&fMutex);
		VString userID;
		fUserRec->GetAttributeValue("ID", err)->GetVValue()->GetString(userID);

		CDB4DEntityCollection* ugCouples = ug->Query("user.ID = :p1", fDirectory->GetDBContext(), err, &userID);
		if (ugCouples != nil)
		{
			err = ugCouples->DeleteEntities(fDirectory->GetDBContext());
			ugCouples->Release();
		}

		fUserRec->Drop();
	}

	fDirectory->DropUserStorage(id);
	return err;
}


VJSSessionStorageObject* UAGUser::RetainStorageObject()
{
	VUUID id;
	GetID(id);
	return fDirectory->RetainUserStorage(id);
}



// --------------------------------------------------------------------------------


UAGSession::UAGSession(UAGDirectory* inDirectory, const VUUID& inUserID, CUAGUser* inUser, bool fromLogout,	VJSONObject* requestInfo)
{
	fUserID = inUserID;
	fDirectory = RetainRefCountable(inDirectory);
	fNextToken = 0;
	fUser = RetainRefCountable(inUser);
	fStorage = new VJSSessionStorageObject;
	fSessionID.Regenerate();
	fJSContext = nil;
	fLifeTime = 60 * 60; // 1 hour
	VTime::Now( fExpirationTime);
	fExpirationTime.AddSeconds( fLifeTime);
	fIsDefault = false;
	fIsFromLogout = fromLogout;
	fForcedExpired = false;
	fRequestInfo = RetainRefCountable(requestInfo);
	if (fUser != nil && fRequestInfo != nil)
	{
		VString username;
		fUser->GetName(username);
		fRequestInfo->SetPropertyAsString("wakanda-username", username);
	}
}


UAGSession::~UAGSession()
{
	for (SessionCleanerMap::iterator cur = fCleaners.begin(), end = fCleaners.end(); cur != end; ++cur)
	{
		ISessionCleaner* cleaner = cur->second;
		cleaner->CleanSession();
		cleaner->Release();
	}

	ClearAllExtraData();
	QuickReleaseRefCountable(fStorage);
	QuickReleaseRefCountable(fUser);
	QuickReleaseRefCountable(fDirectory);
	QuickReleaseRefCountable(fRequestInfo);
}


VError UAGSession::BuildDependences()
{
	VError err = VE_OK;
	CUAGUser* user = fDirectory->RetainUser(fUserID, &err);
	if (user != nil)
	{
		CUAGGroupVector owners;
		err = user->RetainOwners(owners);
		if (err == VE_OK)
			err = BuildDependences(owners);
		user->Release();
	}

	return err;
}



VError UAGSession::BuildDependences(CUAGGroupVector& groups)
{
	VError err = VE_OK;
	for (CUAGGroupVector::iterator cur = groups.begin(), end = groups.end(); cur != end && err == VE_OK; cur++)
	{
		CUAGGroup* group = *cur;
		if (group != nil)
		{
			VUUID groupid;
			group->GetID(groupid);
			if (fBelongsTo.find(groupid) == fBelongsTo.end())
			{
				fBelongsTo.insert(groupid);
				CUAGGroupVector owners;
				err = group->RetainOwners(owners);
				if (err == VE_OK)
					err = BuildDependences(owners);
			}
		}
	}
	return err;
}



bool UAGSession::BelongsTo(const XBOX::VUUID& inGroupID, bool checkNoAdmin)
{
	bool result = false;

	if (fBelongsTo.find(inGroupID) != fBelongsTo.end())
		result = true;
	else if (fPromotedTo.find(inGroupID) != fPromotedTo.end())
		result = true;
	else if (fDirectory->NoAdmin() && checkNoAdmin)
	{
		if (inGroupID.GetBuffer() == fDirectory->GetAdminID())
			result = true;
		else if (inGroupID.GetBuffer() == fDirectory->GetDebugID())
			result = true;
	}

	return result;
}

bool UAGSession::BelongsTo(CUAGGroup* inGroup, bool checkNoAdmin)
{
	VUUID id;
	inGroup->GetID(id);
	return BelongsTo(id, checkNoAdmin);
}

bool UAGSession::Matches(const XBOX::VUUID& inUserID)
{
	return inUserID == fUserID;
}


bool UAGSession::Matches(CUAGUser* inUser)
{
	VUUID id;
	inUser->GetID(id);
	return Matches(id);
}


CUAGUser* UAGSession::RetainUser()
{
	VError err = VE_OK;
	//CUAGUser* user = fDirectory->retainUser(fUserID, &err);
	CUAGUser* user = RetainRefCountable(fUser);
	return user;
}


CUAGSession* UAGSession::Clone()
{
	UAGSession* result = new UAGSession(fDirectory, fUserID, fUser);
	result->fBelongsTo = fBelongsTo;
	return result;
}


void UAGSession::BuildSubPromotions(CUAGGroupVector& groups, IDSet* outGroupsAdded)
{
	for (CUAGGroupVector::iterator cur = groups.begin(), end = groups.end(); cur != end; cur++)
	{
		CUAGGroup* group = *cur;
		if (group != nil)
		{
			VUUID groupid;
			group->GetID(groupid);
			if (!BelongsTo(groupid, false))
			{
				outGroupsAdded->insert(groupid);
				fPromotedTo.insert(groupid);
				CUAGGroupVector owners;
				group->RetainOwners(owners);
				BuildSubPromotions(owners, outGroupsAdded);
			}
		}
	}
}


sLONG UAGSession::PromoteIntoGroup(CUAGGroup* inGroup)
{
	sLONG resultToken = 0;
	IDSet* groupsAdded;
	VUUID groupid;
	inGroup->GetID(groupid);

	if (!BelongsTo(groupid, false))
	{
		resultToken = GetNextToken();
		groupsAdded = &(fPromotions[resultToken]);
		groupsAdded->insert(groupid);
		fPromotedTo.insert(groupid);
		CUAGGroupVector owners;
		inGroup->RetainOwners(owners);
		BuildSubPromotions(owners, groupsAdded);
	}

	return resultToken;
	
}


void UAGSession::UnPromoteFromToken(sLONG promotionToken)
{
	if (promotionToken != 0)
	{
		IDSetMap::iterator found = fPromotions.find(promotionToken);
		if (found != fPromotions.end())
		{
			IDSet* groups = &(found->second);
			for (IDSet::iterator cur = groups->begin(), end = groups->end(); cur != end; ++cur)
			{
				fPromotedTo.erase(*cur);
			}
			fPromotions.erase(found);
		}
	}
}



VJSObject UAGSession::CreateJSSessionObject(const VJSContext& inContext)
{
	return VJSSession::CreateInstance(inContext, this);
}


CUAGDirectory* UAGSession::GetDirectory()
{
	return fDirectory;
}


XBOX::VJSSessionStorageObject* UAGSession::GetStorageObject()
{
	return fStorage;
}


XBOX::VJSSessionStorageObject* UAGSession::RetainStorageObject()
{
	return RetainRefCountable(fStorage);
}


void UAGSession::SetStorageObject(XBOX::VJSSessionStorageObject* inStorage)
{
	CopyRefCountable(&fStorage, inStorage);
}


void UAGSession::GetID( XBOX::VUUID& outID) const
{
	outID = fSessionID;
}


bool UAGSession::hasExpired() const
{
	VTime now;
	VTime::Now( now);
	return (fForcedExpired || fExpirationTime < now);
}


void UAGSession::forceExpire()
{
	VTime::Now(fExpirationTime);
	fForcedExpired = true;
}


void UAGSession::GetExpiration(XBOX::VTime& outExpiration) const
{
	outExpiration = fExpirationTime;
}

sLONG UAGSession::GetLifeTime() const
{
	return fLifeTime;
}


bool UAGSession::IsEmpty() const
{
	return ((fStorage == NULL) || (fStorage->NumberKeysValues() == 0));
}


void UAGSession::SetLifeTime( sLONG inLifeTime)
{
	fLifeTime = inLifeTime;
}

void UAGSession::SetLastUsedJSContext( VJSGlobalContext* inContext)
{
	fJSContext = inContext;
}


VJSGlobalContext* UAGSession::GetLastUsedJSContext() const
{
	return fJSContext;
}


bool UAGSession::SetCookie( IHTTPResponse& inResponse, const VString& inCookieName)
{
	VString value;
	fSessionID.GetString( value);

	// update the expiration time
	VTime::Now( fExpirationTime);
	fExpirationTime.AddSeconds( fLifeTime);

	inResponse.AddResponseHeader(inCookieName, value, true);
	return inResponse.AddCookie ( inCookieName, value, L"", L"", L"", false, true, fLifeTime);
}



void UAGSession::ClearAllExtraData()
{
	for (DataMap::iterator cur = fDataMap.begin(), end = fDataMap.end(); cur != end; ++cur)
	{
		IRefCountable* session = cur->second;
		if (session == nil)
		{
			//assert(false);
			sLONG xdebug = 1; // put a break here
		}
		else
			session->Release();
	}
	fDataMap.clear();
}


VJSONObject* UAGSession::RetainRequestInfo()
{
	return RetainRefCountable(fRequestInfo);
}


void UAGSession::LockExtraData()
{
	fCleanersMutex.Lock();
}

void UAGSession::UnLockExtraData()
{
	fCleanersMutex.Unlock();
}


IRefCountable* UAGSession::GetExtraData(void* selector)
{
	VTaskLock lock(&fCleanersMutex);
	return fDataMap[selector];
}


void UAGSession::SetExtraData(void* selector, IRefCountable* data)
{
	VTaskLock lock(&fCleanersMutex);
	fDataMap[selector] = data;
}


void UAGSession::RemoveExtraData(void* selector)
{
	VTaskLock lock(&fCleanersMutex);
	DataMap::iterator found = fDataMap.find(selector);
	if (found != fDataMap.end())
	{
		QuickReleaseRefCountable(found->second);
		fDataMap.erase(found);
	}
}

void UAGSession::AddSessionCleaner(void* selector, ISessionCleaner* cleaner)
{
	VTaskLock lock(&fCleanersMutex);
	CopyRefCountable(&(fCleaners[selector]), cleaner);
}

void UAGSession::RemoveSessionCleaner(void* selector)
{
	ISessionCleaner* cleaner = nil;
	{
		VTaskLock lock(&fCleanersMutex);
		SessionCleanerMap::iterator found = fCleaners.find(selector);
		if (found != fCleaners.end())
		{
			cleaner = found->second;
			fCleaners.erase(found);
		}
	}
	QuickReleaseRefCountable(cleaner);
}


ISessionCleaner* UAGSession::RetainSessionCleaner(void* selector)
{
	ISessionCleaner* cleaner = nil;
	VTaskLock lock(&fCleanersMutex);
	SessionCleanerMap::iterator found = fCleaners.find(selector);
	if (found != fCleaners.end())
		cleaner = RetainRefCountable(found->second);
	
	return cleaner;
}


 


