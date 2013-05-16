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
#ifndef __USERSANDGROUPS__ 
#define __USERSANDGROUPS__

#include "KernelIPC/VKernelIPC.h"

#pragma pack( push, 8 )


namespace xbox
{
	class VJSContext;
	class VJSObject;
	class VJSGlobalContext;
	class VJSArray;
	class VWorkerPool;
	class VTCPSelectIOPool;
};


#ifndef UAG_API
// any class except components should use DB4D_API
// this is necessary so that RTTI can work with gcc.
#if COMPIL_GCC
#define UAG_API IMPORT_API
#else
#define UAG_API
#endif

#endif

#ifndef nil
#define nil 0
#endif

#include "DB4D/Headers/DB4D.h"

class CUAGGroup;
class CUAGUser;
class CUAGDirectory;
class CUAGSession;

class CDB4DBase;

class CUAGManager : public XBOX::CComponent
{
	public:
		enum {Component_Type = 'ugmg'};

		virtual CUAGDirectory* RetainDirectory(const XBOX::VFile& inXMLFile, XBOX::FileAccess inAccess, const XBOX::VFolder* parent = nil, const XBOX::VString* inDBName = nil, 
											XBOX::VError* outErr = nil, bool* outWasJustCreated = nil, bool buildEmpyDir = false) = 0;

};

typedef std::vector<XBOX::VRefPtr<CUAGGroup> > CUAGGroupVector;
typedef std::vector<XBOX::VRefPtr<CUAGUser> > CUAGUserVector;

namespace xbox
{
	class VJSContext;
}

class CUAGDirectory : public XBOX::CComponent
{
	public:
		typedef enum { AdminGroup = 1, DebuggerGroup = 2 } SpecialGroupEnum;

		enum {Component_Type = 'ugca'};

		virtual CDB4DBase* RetainAndExposeDB() = 0;

		virtual XBOX::VError LoadUsersAndGroups(const XBOX::VFile& inFile) = 0;
		virtual XBOX::VError LoadUsersAndGroups(const XBOX::VValueBag& inBag) = 0;

		virtual CUAGGroup* RetainSpecialGroup(SpecialGroupEnum inGroupRef, XBOX::VError* outerr = nil) = 0;
		virtual bool GetSpecialGroupID(CUAGDirectory::SpecialGroupEnum inGroupRef, XBOX::VUUID& outGroupID) = 0;

		virtual CUAGGroup* RetainGroup(const XBOX::VString& inGroupName, XBOX::VError* outerr = nil) = 0;
		virtual CUAGGroup* RetainGroup(const XBOX::VUUID& inGroupID, XBOX::VError* outerr = nil) = 0;

		virtual CUAGGroup* AddOneGroup(const XBOX::VString& inGroupName, const XBOX::VString& inFullName, XBOX::VError& outError) = 0;

		virtual bool GetGroupID(const XBOX::VString& inGroupName, XBOX::VUUID& outGroupID) = 0;

		virtual CUAGUser* RetainUser(const XBOX::VString& inUserName, XBOX::VError* outerr = nil) = 0;
		virtual CUAGUser* RetainUser(const XBOX::VUUID& inUserID, XBOX::VError* outerr = nil) = 0;

		virtual CUAGUser* AddOneUser(const XBOX::VString& inUserName, const XBOX::VString& inPassword, const XBOX::VString& inFullName, XBOX::VError& outError) = 0;

		virtual bool GetUserID(const XBOX::VString& inUserName, XBOX::VUUID& outUserID) = 0;

		/**	@brief	a JavaScript context is required to support custom authentication by login listener */
		virtual CUAGSession* OpenSession(const XBOX::VString& inUserName, const XBOX::VString& inPassword, XBOX::VError* outErr = nil, XBOX::VJSContext* inJSContext = nil) = 0;
		virtual CUAGSession* OpenSession(const XBOX::VString& inUserAtRealm, XBOX::VError* outErr = nil, XBOX::VJSContext* inJSContext = nil) = 0;
		virtual CUAGSession* OpenSession(CUAGUser* inUser, XBOX::VError* outErr = nil, XBOX::VJSContext* inJSContext = nil) = 0;
		virtual CUAGSession* MakeDefaultSession(XBOX::VError* outErr = nil, XBOX::VJSContext* inJSContext = nil, bool fromLogout = false) = 0;
		

		virtual	XBOX::VJSObject	CreateJSDirectoryObject( const XBOX::VJSContext& inContext) = 0;

		virtual XBOX::VError Save(XBOX::VFile* inFile = nil) = 0;


		virtual XBOX::VError FilterUsers(const XBOX::VString& filterString, bool isAQuery, CUAGUserVector& outUsers) = 0;
		virtual XBOX::VError FilterGroups(const XBOX::VString& filterString, bool isAQuery, CUAGGroupVector& outGroups) = 0;

		virtual XBOX::VError SetLoginListener(const XBOX::VString& listenerRef, const XBOX::VString& promoteRef) = 0;
		virtual XBOX::VError GetLoginListener(XBOX::VString& outListenerRef) = 0;
		virtual	bool HasLoginListener() = 0;

		virtual bool NoAdmin() = 0;
		virtual void ComputeNoAdmin() = 0;

};


class CUAGGroup : public XBOX::CComponent
{
	public:
		enum {Component_Type = 'uggr'};

		virtual XBOX::VError GetID(XBOX::VUUID& outID) = 0;
		virtual XBOX::VError GetName(XBOX::VString& outName) = 0;
		virtual XBOX::VError GetFullName(XBOX::VString& outFullName) = 0;

		virtual XBOX::VError RetainOwners(CUAGGroupVector& outgroups, bool oneLevelDeep = true) = 0;

		virtual XBOX::VValueBag* RetainExtraProperties() = 0;

		virtual XBOX::VError RetainUsers(CUAGUserVector& outUsers, bool oneLevelDeep = true) = 0;

		virtual XBOX::VError RetainSubGroups(CUAGGroupVector& outgroups, bool oneLevelDeep = true) = 0;

		virtual	XBOX::VJSObject	CreateJSGroupObject( const XBOX::VJSContext& inContext) = 0;

		virtual CUAGDirectory* GetDirectory() = 0;

		virtual	XBOX::VError PutIntoGroup( CUAGGroup* group) = 0;
		virtual	XBOX::VError RemoveFromGroup( CUAGGroup* group) = 0;

		virtual	XBOX::VError Drop() = 0;


};



namespace xbox
{
	class VJSSessionStorageObject;
	class VJSGlobalContext;
};



class CUAGUser : public XBOX::CComponent
{
	public:
		enum {Component_Type = 'ugus'};

		virtual XBOX::VError GetID(XBOX::VUUID& outID) = 0;
		virtual XBOX::VError GetName(XBOX::VString& outName) = 0;
		virtual XBOX::VError GetFullName(XBOX::VString& outFullName) = 0;
		//virtual XBOX::VError GetPassword (XBOX::VString& outValue) = 0;
		virtual XBOX::VError GetHA1 (XBOX::VString& outValue) = 0;

		virtual XBOX::VError SetPassword(XBOX::VString& inPassword) = 0;

		virtual XBOX::VError RetainOwners(CUAGGroupVector& outgroups, bool oneLevelDeep = true) = 0;

		virtual XBOX::VValueBag* RetainExtraProperties() = 0;

		virtual XBOX::VError ValidatePassword(const XBOX::VString& inPassword) = 0;

		virtual	XBOX::VJSObject	CreateJSUserObject( const XBOX::VJSContext& inContext) = 0;

		virtual	XBOX::VError PutIntoGroup( CUAGGroup* group) = 0;
		virtual	XBOX::VError RemoveFromGroup( CUAGGroup* group) = 0;

		virtual	XBOX::VError Drop() = 0;

		virtual CUAGDirectory* GetDirectory() = 0;

		virtual XBOX::VJSSessionStorageObject* RetainStorageObject() = 0;

};



class IHTTPResponse;

class CUAGSession : public XBOX::CComponent
{
	public:
		enum {Component_Type = 'ugse'};

		virtual bool BelongsTo(const XBOX::VUUID& inGroupID, bool checkNoAdmin = true) = 0;

		virtual bool BelongsTo(CUAGGroup* inGroup, bool checkNoAdmin = true) = 0;

		virtual bool Matches(const XBOX::VUUID& inUserID) = 0;

		virtual bool Matches(CUAGUser* inUser) = 0;

		virtual CUAGUser* RetainUser() = 0;

		virtual CUAGSession* Clone() = 0;

		virtual sLONG PromoteIntoGroup(CUAGGroup* inGroup) = 0; // returns a promotion token or 0 if not promotion was necessary

		virtual void UnPromoteFromToken(sLONG promotionToken) = 0;

		virtual	XBOX::VJSObject	CreateJSSessionObject( const XBOX::VJSContext& inContext) = 0;

		virtual	CUAGDirectory* GetDirectory() = 0;

		virtual XBOX::VJSSessionStorageObject* GetStorageObject() = 0;
		virtual XBOX::VJSSessionStorageObject* RetainStorageObject() = 0;

		virtual void SetStorageObject(XBOX::VJSSessionStorageObject* inStorage) = 0;

		virtual void GetID( XBOX::VUUID& outID) const = 0;

		virtual bool hasExpired() const = 0;

		virtual bool IsEmpty() const = 0;

		virtual void SetLifeTime( sLONG inLifeTime) = 0;

		// For optimization: set the last JavaScript context which has been initialized for this session.
		// The JavaScript context is usually the one in which a request handler has been called.
		virtual void SetLastUsedJSContext( XBOX::VJSGlobalContext* inContext) = 0;
		virtual XBOX::VJSGlobalContext* GetLastUsedJSContext() const = 0;

		/** @brief	Set the cookie with the http session ID. */
		virtual bool SetCookie( IHTTPResponse& inResponse, const XBOX::VString& inCookieName) = 0;

		virtual bool IsDefault() const = 0;

		virtual bool IsFromLogout() const = 0;
		virtual void ClearFromLogout() = 0;
		
};





const XBOX::VError	VE_UAG_NOT_IMPLEMENTED	= MAKE_VERROR(CUAGManager::Component_Type, 1);

const XBOX::VError	VE_UAG_CANNOT_LOAD_DIRECTORY	= MAKE_VERROR(CUAGManager::Component_Type, 2);

const XBOX::VError	VE_UAG_USERNAME_IS_MISSING	= MAKE_VERROR(CUAGManager::Component_Type, 3);

const XBOX::VError	VE_UAG_USERNAME_ALREADY_EXISTS	= MAKE_VERROR(CUAGManager::Component_Type, 4);

const XBOX::VError	VE_UAG_USERID_ALREADY_EXISTS	= MAKE_VERROR(CUAGManager::Component_Type, 5);

const XBOX::VError	VE_UAG_USERNAME_DOES_NOT_EXIST	= MAKE_VERROR(CUAGManager::Component_Type, 6);

const XBOX::VError	VE_UAG_USERID_DOES_NOT_EXIST	= MAKE_VERROR(CUAGManager::Component_Type, 7);

const XBOX::VError	VE_UAG_GROUPNAME_IS_MISSING	= MAKE_VERROR(CUAGManager::Component_Type, 8);

const XBOX::VError	VE_UAG_GROUPNAME_ALREADY_EXISTS	= MAKE_VERROR(CUAGManager::Component_Type, 9);

const XBOX::VError	VE_UAG_GROUPID_ALREADY_EXISTS	= MAKE_VERROR(CUAGManager::Component_Type, 10);

const XBOX::VError	VE_UAG_GROUPNAME_DOES_NOT_EXIST	= MAKE_VERROR(CUAGManager::Component_Type, 11);

const XBOX::VError	VE_UAG_GROUPID_DOES_NOT_EXIST	= MAKE_VERROR(CUAGManager::Component_Type, 12);

const XBOX::VError	VE_UAG_DB4D_NOT_LOADED	= MAKE_VERROR(CUAGManager::Component_Type, 13);

const XBOX::VError	VE_UAG_UAGDB_MALFORMED	= MAKE_VERROR(CUAGManager::Component_Type, 14);

const XBOX::VError	VE_UAG_UAGDB_NOT_LOADED	= MAKE_VERROR(CUAGManager::Component_Type, 15);

const XBOX::VError	VE_UAG_USER_IS_MALFORMED	= MAKE_VERROR(CUAGManager::Component_Type, 16);

const XBOX::VError	VE_UAG_GROUP_IS_MALFORMED	= MAKE_VERROR(CUAGManager::Component_Type, 17);

const XBOX::VError	VE_UAG_PASSWORD_DOES_NOT_MATCH	= MAKE_VERROR(CUAGManager::Component_Type, 18);

const XBOX::VError	VE_UAG_FILE_IS_MISSING	= MAKE_VERROR(CUAGManager::Component_Type, 19);

const XBOX::VError	VE_UAG_MODEL_IS_MISSING	= MAKE_VERROR(CUAGManager::Component_Type, 20);

const XBOX::VError	VE_UAG_SESSION_FAILED_PERMISSION = MAKE_VERROR(CUAGManager::Component_Type, 21);

const XBOX::VError	VE_UAG_LOGINLISTENER_NOT_FOUND = MAKE_VERROR(CUAGManager::Component_Type, 22);

#pragma pack( pop )

#endif
