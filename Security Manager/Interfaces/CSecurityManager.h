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
#ifndef __CSECURITY_MANAGER__
#define __CSECURITY_MANAGER__

class CUAGDirectory;

// Receipe:
//
// 1) To write a component library, define your component class deriving it
// from CComponent. You will provide this class with your library as a header file.
//
// class CComponent1 : public CComponent
// {
//	public:
//	enum { Component_Type = 'cmp1' };
//
//	Blah, blah - Public Interface
// };
//
// Note that a component has no constructor, no data and only pure virtuals (= 0)

class CUAGSession;

class CSecurityManager : public XBOX::CComponent
{
public:
    enum {Component_Type = 'scmg'};

	//virtual XBOX::VError ValidateUserGroup (const XBOX::VString& inUserName, const XBOX::VString& inUserGroup) = 0;

	//Returns True/False and a valid/unchanged token on authentication success/failure.
	virtual XBOX::VError ValidateBasicAuthentication(	const XBOX::VString& inName, 
														const XBOX::VString& inPassword,
														bool* outAuthOk,
														CUAGSession* &outUAGSession) = 0;

	virtual XBOX::VError ValidateDigestAuthentication (	const XBOX::VString& inUserName,
														const XBOX::VString& inAlgorithm,
														const XBOX::VString& inRealm,
														const XBOX::VString& inNonce,
														const XBOX::VString& inCNonce,
														const XBOX::VString& inQOP,
														const XBOX::VString& inNonceCount,
														const XBOX::VString& inURi,
														const XBOX::VString& inMethod,
														const XBOX::VString& inChallenge,
														bool *outAuthOk,
														CUAGSession* &outUAGSession) = 0;
	
	virtual XBOX::VString ComputeDigestHA1(	const XBOX::VString& inUserName,
											const XBOX::VString& inPassword, 
											const XBOX::VString& inRealm ) = 0;
	
	
	
	//Set (or reset) the kerberos configuration to use : Wakanda server hostname (full dns name also works) and full keytab path
	virtual XBOX::VError SetKerberosConfig(const XBOX::VString& inHostName, const XBOX::VString& inKeytabPath)=0;

    //Returns True/False and valid/unchanged token and possibly outAuthData (which may be NULL) on authentication success/failure.
    virtual XBOX::VError ValidateKerberosAuthentication(const XBOX::VString& inAuthData, bool* outAuthOk, XBOX::VString* outAuthData, CUAGSession* &outUAGSession)=0;

	/*
    //Returns True/False if token refers to a valid session or not
    virtual bool IsValideToken(const XBOX::VString& inToken)=0;
    
    //Tells the Security Manager that inToken isn't used anymore. Do not fail (doesn't complain on /bad/ inToken)
	virtual void InvalidateToken(const XBOX::VString& inToken)=0;
	*/

	//Set the user directory to ask for sessions. Accept NULL value.
	virtual void SetUserDirectory(CUAGDirectory* inDirectory)=0;

	virtual CUAGDirectory* GetUserDirectory()=0;

	/*
	//returns the Authentication session used to check privilileges and group owning
	virtual CUAGSession* RetainAuthenticationSession(const XBOX::VString& inToken) = 0;
	*/
};


#endif

