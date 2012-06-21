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
#ifndef __JSUAG__
#define __JSUAG__



class VJSDirectory : public XBOX::VJSClass<VJSDirectory, CUAGDirectory>
{
public:
	typedef VJSClass<VJSDirectory, CUAGDirectory> inherited;

	static	void			Initialize( const XBOX::VJSParms_initialize& inParms, CUAGDirectory* inDirectory);
	static	void			Finalize( const XBOX::VJSParms_finalize& inParms, CUAGDirectory* inDirectory);
	static	void			GetDefinition( ClassDefinition& outDefinition);

	static void _getUser(XBOX::VJSParms_callStaticFunction& ioParms, CUAGDirectory* inDirectory); // user : User(userName | userID)
	static void _getGroup(XBOX::VJSParms_callStaticFunction& ioParms, CUAGDirectory* inDirectory); // group : Group(groupName | groupID)
	static void _filterUsers(XBOX::VJSParms_callStaticFunction& ioParms, CUAGDirectory* inDirectory); // array of users : filterUsers(beginingOfUserName | queryString, bool: isQuery)
	static void _filterGroups(XBOX::VJSParms_callStaticFunction& ioParms, CUAGDirectory* inDirectory); // array of groups : filterGroups(beginingOfGroupName | queryString, bool: isQuery )
	static void _save(XBOX::VJSParms_callStaticFunction& ioParms, CUAGDirectory* inDirectory); // bool : save(file | null)  if null save in the current directory file
	static void _addUser(XBOX::VJSParms_callStaticFunction& ioParms, CUAGDirectory* inDirectory); // user : addUser(userName, password, fullName) 
	static void _addGroup(XBOX::VJSParms_callStaticFunction& ioParms, CUAGDirectory* inDirectory); // group : addGroup(groupName, fullName) 
	static void _setLoginListener(XBOX::VJSParms_callStaticFunction& ioParms, CUAGDirectory* inDirectory); // bool : setLoginListener( string listenerRef)
	static void _getLoginListener(XBOX::VJSParms_callStaticFunction& ioParms, CUAGDirectory* inDirectory); // string : getLoginListener()
	static void _computeHA1(XBOX::VJSParms_callStaticFunction& ioParms, CUAGDirectory* inDirectory); // string : computeHA1(userName, password, domainName || "Wakanda")
	static void _hasAdministrator(XBOX::VJSParms_callStaticFunction& ioParms, CUAGDirectory* inDirectory); // bool : hasAdministrator()
	
	static void _getInternalStore( XBOX::VJSParms_getProperty& ioParms, CUAGDirectory* inDirectory);
};


class VJSUser : public XBOX::VJSClass<VJSUser, CUAGUser>
{
public:
	typedef VJSClass<VJSUser, CUAGUser> inherited;

	static	void			Initialize( const XBOX::VJSParms_initialize& inParms, CUAGUser* inUser);
	static	void			Finalize( const XBOX::VJSParms_finalize& inParms, CUAGUser* inUser);
	static	void			GetDefinition( ClassDefinition& outDefinition);

	static void _setPassword(XBOX::VJSParms_callStaticFunction& ioParms, CUAGUser* inUser); // bool : setPassword(oldpass, newpass)
	static void _putInto(XBOX::VJSParms_callStaticFunction& ioParms, CUAGUser* inUser); // putInto(group | string)
	static void _removeFrom(XBOX::VJSParms_callStaticFunction& ioParms, CUAGUser* inUser); // removeFrom(group | string)
	static void _remove(XBOX::VJSParms_callStaticFunction& ioParms, CUAGUser* inUser); // remove()
	static void _getParents( XBOX::VJSParms_callStaticFunction& ioParms, CUAGUser* inUser); // array of groups : getParents(bool : "firstLevel")
	static void _filterParents(XBOX::VJSParms_callStaticFunction& ioParms, CUAGUser* inUser); // array of groups : filterParents(beginingOfGroupName | queryString, bool: isQuery)

	static void _getName( XBOX::VJSParms_getProperty& ioParms, CUAGUser* inUser); // string
	static void _getFullName( XBOX::VJSParms_getProperty& ioParms, CUAGUser* inUser); // string 
	static void _getID( XBOX::VJSParms_getProperty& ioParms, CUAGUser* inUser); // string
	static void _getStorage( XBOX::VJSParms_getProperty& ioParms, CUAGUser* inUser); // storage
};


class VJSGroup : public XBOX::VJSClass<VJSGroup, CUAGGroup>
{
public:
	typedef VJSClass<VJSGroup, CUAGGroup> inherited;

	static	void			Initialize( const XBOX::VJSParms_initialize& inParms, CUAGGroup* inGroup);
	static	void			Finalize( const XBOX::VJSParms_finalize& inParms, CUAGGroup* inGroup);
	static	void			GetDefinition( ClassDefinition& outDefinition);

	static void _filterUsers(XBOX::VJSParms_callStaticFunction& ioParms, CUAGGroup* inGroup); // array of users : filterUsers(beginingOfUserName | queryString, bool: isQuery)
	static void _getUsers( XBOX::VJSParms_callStaticFunction& ioParms, CUAGGroup* inGroup); // array of users : getUsers(bool : "firstLevel")
	static void _getChildren( XBOX::VJSParms_callStaticFunction& ioParms, CUAGGroup* inGroup); // array of groups : getChildren(bool : "firstLevel")
	static void _filterChildren(XBOX::VJSParms_callStaticFunction& ioParms, CUAGGroup* inGroup); // array of groups : filterChildren(beginingOfGroupName | queryString, bool: isQuery)
	static void _getParents( XBOX::VJSParms_callStaticFunction& ioParms, CUAGGroup* inGroup); // array of groups : getParents(bool : "firstLevel")
	static void _filterParents(XBOX::VJSParms_callStaticFunction& ioParms, CUAGGroup* inGroup); // array of groups : filterParents(beginingOfGroupName | queryString, bool: isQuery)
	static void _putInto( XBOX::VJSParms_callStaticFunction& ioParms, CUAGGroup* inGroup); // putInto(group | string)
	static void _removeFrom( XBOX::VJSParms_callStaticFunction& ioParms, CUAGGroup* inGroup); // removeFrom(group | string)
	static void _remove( XBOX::VJSParms_callStaticFunction& ioParms, CUAGGroup* inGroup); // remove()

	static void _getName( XBOX::VJSParms_getProperty& ioParms, CUAGGroup* inGroup); // string
	static void _getFullName( XBOX::VJSParms_getProperty& ioParms, CUAGGroup* inGroup); // string 
	static void _getID( XBOX::VJSParms_getProperty& ioParms, CUAGGroup* inGroup); // string
};


class VJSSession : public XBOX::VJSClass<VJSSession, CUAGSession>
{
public:
	typedef VJSClass<VJSSession, CUAGSession> inherited;

	static	void			Initialize( const XBOX::VJSParms_initialize& inParms, CUAGSession* inSession);
	static	void			Finalize( const XBOX::VJSParms_finalize& inParms, CUAGSession* inSession);
	static	void			GetDefinition( ClassDefinition& outDefinition);

	static void _belongsTo(XBOX::VJSParms_callStaticFunction& ioParms, CUAGSession* inSession); // bool : belongsTo(group || groupID)
	static void _checkPermission(XBOX::VJSParms_callStaticFunction& ioParms, CUAGSession* inSession); // bool : checkPermission(group || groupID)
	static void _promoteWith(XBOX::VJSParms_callStaticFunction& ioParms, CUAGSession* inSession); // promotionToken : promoteWith(group || groupID)
	static void _unPromote(XBOX::VJSParms_callStaticFunction& ioParms, CUAGSession* inSession); //  unPromote(promotionToken)

	static void _getUser( XBOX::VJSParms_getProperty& ioParms, CUAGSession* inSession); // user
	static void _getStorage( XBOX::VJSParms_getProperty& ioParms, CUAGSession* inSession); // storage
};


#endif