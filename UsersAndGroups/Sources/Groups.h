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
#ifndef __GROUPS__
#define __GROUPS__

class UAGDirectory;

class UAGGroup : public VComponentImp<CUAGGroup>
{
	public:

		UAGGroup(UAGDirectory* inDirectory, CDB4DEntityRecord* inGroupRec);
		virtual ~UAGGroup();

		virtual VError GetID(VUUID& outID);
		virtual VError GetName(VString& outName);
		virtual VError GetFullName(VString& outFullName);

		virtual VValueBag* RetainExtraProperties();

		VError RetainOtherGroups(CUAGGroupVector& outgroups, bool oneLevelDeep, const VString root);

		virtual VError RetainOwners(CUAGGroupVector& outgroups, bool oneLevelDeep = true);
		virtual VError RetainSubGroups(CUAGGroupVector& outgroups, bool oneLevelDeep = true);

		virtual VError RetainUsers(CUAGUserVector& outUsers, bool oneLevelDeep = true);

		VError FromBag(const VValueBag* inBag);

		virtual	VJSObject CreateJSGroupObject(const VJSContext& inContext);

		inline CDB4DEntityRecord* GetEntity()
		{
			return fGroupRec;
		}

		inline UAGDirectory* getDirectory()
		{
			return fDirectory;
		}

		virtual CUAGDirectory* GetDirectory();

		virtual	VError PutIntoGroup( CUAGGroup* group);
		virtual	VError RemoveFromGroup( CUAGGroup* group);

		virtual	VError Drop();

	protected:
		UAGDirectory* fDirectory;
		CDB4DEntityRecord* fGroupRec;
};
#endif