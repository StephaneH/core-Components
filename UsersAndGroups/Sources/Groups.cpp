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






UAGGroup::UAGGroup(UAGDirectory* inDirectory, CDB4DEntityRecord* inGroupRec)
{
	fDirectory = RetainRefCountable(inDirectory);
	fGroupRec = RetainRefCountable(inGroupRec);
}


UAGGroup::~UAGGroup()
{
	QuickReleaseRefCountable(fGroupRec);
	QuickReleaseRefCountable(fDirectory);
}


CUAGDirectory* UAGGroup::GetDirectory()
{
	return fDirectory;
}


VError UAGGroup::GetID(VUUID& outID)
{
	VError err;
	CDB4DEntityAttributeValue* xval = fGroupRec->GetAttributeValue(L"ID", err);
	if (xval != nil)
	{
		VValueSingle* val = xval->GetVValue();
		if (val != nil)
			val->GetVUUID(outID);
	}
	return err;
}


VError UAGGroup::GetName(VString& outName)
{
	VError err;
	CDB4DEntityAttributeValue* xval = fGroupRec->GetAttributeValue(L"name", err);
	if (xval != nil)
	{
		VValueSingle* val = xval->GetVValue();
		if (val != nil)
			val->GetString(outName);
	}
	return err;
}


VError UAGGroup::GetFullName(VString& outName)
{
	VError err;
	CDB4DEntityAttributeValue* xval = fGroupRec->GetAttributeValue(L"fullName", err);
	if (xval != nil)
	{
		VValueSingle* val = xval->GetVValue();
		if (val != nil)
			val->GetString(outName);
	}
	return err;
}


VError UAGGroup::RetainOtherGroups(CUAGGroupVector& outgroups, bool oneLevelDeep, const VString root)
{
	outgroups.clear();
	VError err = VE_OK;
	if (oneLevelDeep)
	{
		CDB4DEntityAttributeValue* xval = fGroupRec->GetAttributeValue(root, err);
		if (xval != nil)
		{
			CDB4DBaseContext* context = fGroupRec->GetContext();
			CDB4DSelection* sel = xval->GetRelatedSelection();
			if (sel != nil)
			{
				sLONG nb = sel->CountRecordsInSelection(context);
				outgroups.reserve(outgroups.size() + nb);
				for (sLONG i = 0; i < nb && err == VE_OK; i++)
				{
					CDB4DEntityRecord* subgroup = fDirectory->GetGroupModel()->LoadEntity(sel->GetSelectedRecordID(i+1, context), err, DB4D_Do_Not_Lock, context, false);
					if (subgroup != nil)
					{
						CUAGGroup* group = new UAGGroup(fDirectory, subgroup);
						outgroups.push_back(group);
						group->Release();
						subgroup->Release();
					}
				}
			}
		}
	}
	else
	{
		set<VUUIDBuffer> alreadyGroup;
		VUUID xid;
		GetID(xid);
		alreadyGroup.insert(xid.GetBuffer());
		CUAGGroupVector firstLevelGroups;
		//err = RetainSubGroups(firstLevelGroups, true);
		err = RetainOtherGroups(firstLevelGroups, true, root);
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
					err = xfgroup->RetainOtherGroups(otherGroups, true, root);
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


VError UAGGroup::RetainOwners(CUAGGroupVector& outgroups, bool oneLevelDeep)
{
	return RetainOtherGroups(outgroups, oneLevelDeep, "parents");
}


XBOX::VError UAGGroup::RetainSubGroups(CUAGGroupVector& outgroups, bool oneLevelDeep)
{
	return RetainOtherGroups(outgroups, oneLevelDeep, "children");
}

XBOX::VValueBag* UAGGroup::RetainExtraProperties()
{
	return nil;
}



XBOX::VError UAGGroup::RetainUsers(CUAGUserVector& outUsers, bool oneLevelDeep)
{
	outUsers.clear();
	VError err = VE_OK;
	if (oneLevelDeep)
	{
		CDB4DEntityAttributeValue* xval = fGroupRec->GetAttributeValue(L"users", err);
		if (xval != nil)
		{
			CDB4DBaseContext* context = fGroupRec->GetContext();
			CDB4DSelection* sel = xval->GetRelatedSelection();
			if (sel != nil)
			{
				sLONG nb = sel->CountRecordsInSelection(context);
				outUsers.reserve(outUsers.size() + nb);
				for (sLONG i = 0; i < nb && err == VE_OK; i++)
				{
					CDB4DEntityRecord* userrec = fDirectory->GetUserModel()->LoadEntity(sel->GetSelectedRecordID(i+1, context), err, DB4D_Do_Not_Lock, context, false);
					if (userrec != nil)
					{
						UAGUser* user = new UAGUser(fDirectory, userrec);
						outUsers.push_back(user);
						user->Release();
						userrec->Release();
					}
				}
			}
		}
	}
	else
	{
		set<VUUIDBuffer> alreadyUsers;
		err = RetainUsers(outUsers, true);
		if (err == VE_OK)
		{
			for (CUAGUserVector::iterator curu = outUsers.begin(), endu = outUsers.end(); curu != endu; ++curu)
			{
				VUUID xid;
				(*curu)->GetID(xid);
				alreadyUsers.insert(xid.GetBuffer());
			}

			CUAGGroupVector children;
			err = RetainSubGroups(children, false);
			if (err == VE_OK)
			{
				for (CUAGGroupVector::iterator cur = children.begin(), end = children.end(); cur != end && err == VE_OK; ++cur)
				{
					CUAGUserVector otherUsers;
					err = (*cur)->RetainUsers(otherUsers, true);
					if (err == VE_OK)
					{
						for (CUAGUserVector::iterator curu = otherUsers.begin(), endu = otherUsers.end(); curu != endu; ++curu)
						{
							CUAGUser* otherUser = *curu;
							VUUID xid;
							otherUser->GetID(xid);
							if (alreadyUsers.find(xid.GetBuffer()) == alreadyUsers.end())
							{
								alreadyUsers.insert(xid.GetBuffer());
								outUsers.push_back(otherUser);
							}
						}
					}
				}
			}
		}
	}

	return err;
}



VError UAGGroup::FromBag(const VValueBag* inBag)
{
	VError err = VE_OK;
	/*
	fName.Clear();
	if (!inBag->GetString(uag::name, fName))
	{
		err = ThrowError(VE_UAG_GROUPNAME_IS_MISSING);
	}
	else
	{
		VString ids;
		if (inBag->GetString(uag::id, ids))
			fID.FromString(ids);
		else
			fID.Regenerate();
		inBag->GetString(uag::fullName, fFullName);
	}
	*/
	return err;

}


VJSObject UAGGroup::CreateJSGroupObject(const VJSContext& inContext)
{
	return VJSGroup::CreateInstance(inContext, this);
}


VError UAGGroup::PutIntoGroup( CUAGGroup* group)
{
	VError err = VE_OK;
	if (group != nil)
	{
		CDB4DEntityModel* gg = fDirectory->GetCoupleGGModel();

		VString thisGroupID, groupID;

		UAGGroup* xgroup = VImpCreator<UAGGroup>::GetImpObject(group);
		CDB4DEntityRecord* grouprec = xgroup->GetEntity();
		grouprec->GetAttributeValue("ID", err)->GetVValue()->GetString(groupID);

		fGroupRec->GetAttributeValue("ID", err)->GetVValue()->GetString(thisGroupID);

		CDB4DEntityRecord* ggCouple = gg->Find("child.ID = :p1 and parent.ID = :p2", fDirectory->GetDBContext(), err, &thisGroupID, &groupID);
		if (ggCouple == nil)
		{
			ggCouple = gg->NewEntity(fDirectory->GetDBContext(), DB4D_Do_Not_Lock);
			ggCouple->SetAttributeValue("child", fGroupRec);
			ggCouple->SetAttributeValue("parent", grouprec);
			ggCouple->Save(0);
		}
	}
	return err;
}



VError UAGGroup::RemoveFromGroup( CUAGGroup* group)
{
	VError err = VE_OK;
	if (group != nil)
	{
		CDB4DEntityModel* gg = fDirectory->GetCoupleGGModel();

		VString thisGroupID, groupID;

		UAGGroup* xgroup = VImpCreator<UAGGroup>::GetImpObject(group);
		CDB4DEntityRecord* grouprec = xgroup->GetEntity();
		grouprec->GetAttributeValue("ID", err)->GetVValue()->GetString(groupID);

		fGroupRec->GetAttributeValue("ID", err)->GetVValue()->GetString(thisGroupID);

		CDB4DSelection* ggCouples = gg->Query("child.ID = :p1 and parent.ID = :p2", fDirectory->GetDBContext(), err, &thisGroupID, &groupID);
		if (ggCouples != nil)
		{
			err = ggCouples->DeleteRecords(fDirectory->GetDBContext());
			ggCouples->Release();
		}
	}
	return err;
}


VError UAGGroup::Drop()
{
	VError err = VE_OK;
	CDB4DEntityModel* gg = fDirectory->GetCoupleGGModel();

	VString thisGroupID;
	fGroupRec->GetAttributeValue("ID", err)->GetVValue()->GetString(thisGroupID);

	CDB4DSelection* ggCouples = gg->Query("child.ID = :p1", fDirectory->GetDBContext(), err, &thisGroupID);
	if (ggCouples != nil)
	{
		err = ggCouples->DeleteRecords(fDirectory->GetDBContext());
		ggCouples->Release();
	}

	ggCouples = gg->Query("parent.ID = :p1", fDirectory->GetDBContext(), err, &thisGroupID);
	if (ggCouples != nil)
	{
		err = ggCouples->DeleteRecords(fDirectory->GetDBContext());
		ggCouples->Release();
	}

	fGroupRec->Drop();
	return err;
}



