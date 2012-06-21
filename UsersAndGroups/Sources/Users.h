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
#ifndef __USERS__
#define __USERS__

class VUAGManager;
class UAGDirectory;

class UAGUser : public VComponentImp<CUAGUser>
{
	public:

		UAGUser(UAGDirectory* inDirectory, CDB4DEntityRecord* inUserRec);
		virtual ~UAGUser();

		virtual VError GetID(VUUID& outID);
		virtual VError GetName(VString& outName);
		virtual VError GetFullName(VString& outFullName);
		//virtual VError GetPassword (XBOX::VString& outValue);
		virtual VError GetHA1(VString& outValue);

		virtual VError SetPassword(VString& inPassword);

		virtual VError RetainOwners(CUAGGroupVector& outgroups, bool oneLevelDeep = true);

		virtual VValueBag* RetainExtraProperties();

		VError FromBag(const VValueBag* inBag);

		virtual VError ValidatePassword(const VString& inPassword);

		virtual	VJSObject CreateJSUserObject(const VJSContext& inContext);

		virtual	VError PutIntoGroup( CUAGGroup* group);
		virtual	VError RemoveFromGroup( CUAGGroup* group);

		inline CDB4DEntityRecord* GetEntity()
		{
			return fUserRec;
		}

		inline UAGDirectory* getDirectory()
		{
			return fDirectory;
		}
		
		virtual CUAGDirectory* GetDirectory();

		virtual	VError Drop();

		virtual XBOX::VJSSessionStorageObject* RetainStorageObject();


	protected:
		UAGDirectory* fDirectory;
		CDB4DEntityRecord* fUserRec;

};


typedef set<VUUID> IDSet;
typedef map<sLONG, IDSet> IDSetMap;

class UAGSession : public VComponentImp<CUAGSession>
{
	public:
		UAGSession(UAGDirectory* inDirectory, const VUUID& inUserID, CUAGUser* inUser);
		virtual ~UAGSession();

		VError BuildDependences();
		VError BuildDependences(CUAGGroupVector& groups);

		virtual bool BelongsTo(const XBOX::VUUID& inGroupID);

		virtual bool BelongsTo(CUAGGroup* inGroup);

		virtual bool Matches(const XBOX::VUUID& inUserID);

		virtual bool Matches(CUAGUser* inUser);

		virtual CUAGUser* RetainUser();

		virtual CUAGSession* Clone();

		virtual sLONG PromoteIntoGroup(CUAGGroup* inGroup);

		virtual void UnPromoteFromToken(sLONG promotionToken);

		virtual	VJSObject CreateJSSessionObject(const VJSContext& inContext);

		virtual	CUAGDirectory* GetDirectory();

		virtual XBOX::VJSSessionStorageObject* GetStorageObject();
		virtual XBOX::VJSSessionStorageObject* RetainStorageObject();

		virtual void SetStorageObject(XBOX::VJSSessionStorageObject* inStorage);

		virtual void GetID( XBOX::VUUID& outID) const;

		virtual bool hasExpired() const;

		virtual bool IsEmpty() const;

		virtual void SetLifeTime( sLONG inLifeTime);

		virtual void SetLastUsedJSContext( XBOX::VJSGlobalContext* inContext);
		virtual XBOX::VJSGlobalContext* GetLastUsedJSContext() const;

		virtual bool SetCookie( IHTTPResponse& inResponse, const XBOX::VString& inCookieName);

		virtual bool IsDefault() const
		{
			return fIsDefault;
		}

		void SetDefault(bool value = true)
		{
			fIsDefault = value;
		}

	protected:

		sLONG GetNextToken()
		{
			++fNextToken;
			if (fNextToken < 0)
				fNextToken = 1;
			return fNextToken;
		}

		void BuildSubPromotions(CUAGGroupVector& groups, IDSet* outGroupsAdded);

		UAGDirectory* fDirectory;
		CUAGUser* fUser;
		IDSet fBelongsTo;
		IDSet fPromotedTo;
		IDSetMap fPromotions;
		VUUID fUserID;
		VUUID fSessionID;
		sLONG fNextToken;
		VJSSessionStorageObject* fStorage;
		VJSGlobalContext *fJSContext;
		VTime fExpirationTime;
		sLONG fLifeTime;	// in seconds
		bool fIsDefault;

};


#endif