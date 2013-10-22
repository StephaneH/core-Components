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
#ifndef __UAG__
#define __UAG__

/*
typedef map<VUUIDBuffer, UAGUser*> UsersByIDMap;
typedef map<VString, UAGUser*> UsersByNameMap;

typedef map<VUUIDBuffer, UAGGroup*> GroupsByIDMap;
typedef map<VString, UAGGroup*> GroupsByNameMap;
*/

class VUAGManager : public VComponentImp<CUAGManager>
{
	public:

		VUAGManager();

		virtual ~VUAGManager();

		virtual CUAGDirectory* RetainDirectory(const VFile& inXMLFile, FileAccess inAccess, const VFolder* parent = nil, const VString* inDBName = nil, 
												VError* outErr = nil, bool* outWasJustCreated = nil, bool buildEmpyDir = false);

		inline CDB4DManager* GetDB4D()
		{
			return fDB4D;
		}

		VFolder* RetainFolder(BundleFolderKind inKind = kBF_BUNDLE_FOLDER);

	protected:
		CDB4DManager* fDB4D;
		VComponentLibrary* fCompLibrary;
		VFolder* fComponentFolder;
		VLocalizationManager* fDefaultLocalization;
		/*
		UsersByIDMap fUsersByID;
		UsersByNameMap fUsersByName;
		GroupsByIDMap fGroupsByID;
		GroupsByNameMap fGroupsByName;
		*/

};


class UAGDirectory : public VComponentImp<CUAGDirectory>
{
	public:
		UAGDirectory(VUAGManager* inManager);

		virtual ~UAGDirectory();

		VError OpenDB(const VFile* xmlFile, const VFolder& parent, const VString& inDBName, FileAccess inAccess, bool* outWasJustCreated = nil, bool buildEmpyDir = false);
		VError CloseDB();
		VError ClearDB(CDB4DBaseContext* context);

		VError ImportXMLData(const VFile& inFile, CDB4DBaseContext* context);

		virtual CDB4DBase* RetainAndExposeDB();
		virtual VError LoadUsersAndGroups(const VFile& inFile);

		virtual VError LoadUsersAndGroups(const VValueBag& inBag);

		virtual CUAGGroup* RetainSpecialGroup(CUAGDirectory::SpecialGroupEnum inGroupRef, XBOX::VError* outerr = nil);
		virtual bool GetSpecialGroupID(CUAGDirectory::SpecialGroupEnum inGroupRef, VUUID& outGroupID);

		virtual CUAGGroup* RetainGroup(const VString& inGroupName, VError* outerr = nil);
		virtual CUAGGroup* RetainGroup(const VUUID& inGroupID, VError* outerr = nil);

		virtual bool GetGroupID(const VString& inGroupName, VUUID& outGroupID);

		virtual CUAGUser* RetainUser(const VString& inUserName, VError* outerr = nil);
		virtual CUAGUser* RetainUser(const VUUID& inUserID, VError* outerr = nil);

		virtual bool GetUserID(const VString& inUserName, VUUID& outUserID);

		virtual VError Save(VFile* inFile = nil);

		UAGGroup* retainGroup(const VString& inGroupName, VError* outerr = nil);
		UAGGroup* retainGroup(const VUUID& inGroupID, VError* outerr = nil);

		UAGUser* retainUser(const VString& inUserName, VError* outerr = nil);
		UAGUser* retainUser(const VUUID& inUserID, VError* outerr = nil);

		UAGUser* BuildUser(const VValueBag* inBag, VError& err);
		UAGUser* addOneUser(const VString& inUserName, const VString& inPassword, const VString& inFullName, VError& err);
		virtual CUAGUser* AddOneUser(const VString& inUserName, const VString& inPassword, const VString& inFullName, VError& err);

		UAGGroup* BuildGroup(const VValueBag* inBag, VError& err);
		UAGGroup* addOneGroup(const VString& inGroupName, const VString& inFullName, VError& err, VUUID* inPredefinedID = nil);
		virtual CUAGGroup* AddOneGroup(const VString& inGroupName, const VString& inFullName, VError& err);

		inline CDB4DEntityModel* GetUserModel()
		{
			return fUserModel;
		}

		inline CDB4DEntityModel* GetGroupModel()
		{
			return fGroupModel;
		}

		inline CDB4DEntityModel* GetCoupleUGModel()
		{
			return fCoupleUGModel;
		}

		inline CDB4DEntityModel* GetCoupleGGModel()
		{
			return fCoupleGGModel;
		}

		inline CDB4DBaseContext* GetDBContext()
		{
			return fContext;
		}

		inline const VUUIDBuffer& GetAdminID() const
		{
			return fAdminGroupID;
		}

		inline const VUUIDBuffer& GetDebugID() const
		{
			return fDebugGroupID;
		}

		bool TakenByListener(const VString& inUserName, const VString& inPassword, VError& err, UAGSession* &session, VJSContext* inJSContext, bool passIsKey);
		virtual CUAGSession* OpenSession(const VString& inUserName, const VString& inPassword, VError* outErr = nil, VJSContext* inJSContext = nil, bool passIsKey = false);
		virtual CUAGSession* OpenSession(const VString& inUserAtRealm, VError* outErr = nil, VJSContext* inJSContext = nil);
		virtual CUAGSession* OpenSession(CUAGUser* inUser, VError* outErr = nil, VJSContext* inJSContext = nil);
		virtual CUAGSession* MakeDefaultSession(VError* outErr = nil, VJSContext* inJSContext = nil,bool fromLogout = false);

		virtual	XBOX::VJSObject	CreateJSDirectoryObject( const XBOX::VJSContext& inContext);
		virtual CUAGUser* RetainParamUser(XBOX::VJSParms_callStaticFunction& ioParms, sLONG paramNum);
		virtual CUAGGroup* RetainParamGroup(XBOX::VJSParms_callStaticFunction& ioParms, sLONG paramNum);

		virtual VError FilterUsers(const VString& filterString, bool isAQuery, CUAGUserVector& outUsers);
		virtual VError FilterGroups(const VString& filterString, bool isAQuery, CUAGGroupVector& outGroups);

		virtual VError SetLoginListener(const VString& listenerRef, const VString& promoteRef);

		virtual VError GetLoginListener(VString& outListenerRef)
		{
			outListenerRef = fLoginListener;
			return VE_OK;
		}

		virtual bool HasLoginListener()
		{
			return !fLoginListener.IsEmpty();
		}

		virtual bool NoAdmin();
		virtual void ComputeNoAdmin();

		VJSSessionStorageObject* RetainUserStorage(const VUUID& inID);
		void DropUserStorage(const VUUID& inID);

	protected:
		typedef map<const VUUIDBuffer, VJSSessionStorageObject*> StorageMap;

		VFile* fXmlFile;
		VUAGManager* fManager;
		CDB4DBase* fBase;
		CDB4DBaseContext* fContext;
		VCriticalSection fContextMutex;

		CDB4DTable* fUserTable;
		CDB4DTable* fGroupTable;
		CDB4DTable* fCoupleUGTable;
		CDB4DTable* fCoupleGGTable;

		CDB4DEntityModel* fUserModel;
		CDB4DEntityModel* fGroupModel;
		CDB4DEntityModel* fCoupleUGModel;
		CDB4DEntityModel* fCoupleGGModel;

		VString fLoginListener;
		CUAGGroup* fLoginPromote;

		StorageMap fUserStorages;
		VCriticalSection fUserStoragesMutex;

		VUUIDBuffer fAdminGroupID;
		VUUIDBuffer fDebugGroupID;


		bool fNoAdmin;
};





#endif

