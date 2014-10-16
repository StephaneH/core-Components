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
#ifndef __SECURITY_MANAGER__
#define __SECURITY_MANAGER__



////////////////////////////////////////////////////////////////////////////////
// Toolbox headers
////////////////////////////////////////////////////////////////////////////////

#include "Kernel/VKernel.h"
#include "KernelIPC/VKernelIPC.h"
#include "Kernel/Sources/VString.h"

USING_TOOLBOX_NAMESPACE

//jmo - Disable kerberos which is broken right now
//Additional Libraries Directories : $(ProjectDir)\..\..\..\..\..\kerberos\kfw-3-2-3\Win64\lib\amd64\
//Additional Dependencies : gssapi64.lib
#define WITH_KERBEROS 0

////////////////////////////////////////////////////////////////////////////////
// Needed Interfaces
////////////////////////////////////////////////////////////////////////////////

class CUAGDirectory;
class CUAGSession;
class CUAGManager;
class CUAGDirectory;

#include "CSecurityManager.h"

#if WITH_KERBEROS
	#include "gssapi.h"
#endif


////////////////////////////////////////////////////////////////////////////////
// Component Errors
////////////////////////////////////////////////////////////////////////////////

const XBOX::VError	VE_SECMAN_NO_UAG_DIRECTORY						= MAKE_VERROR(CSecurityManager::Component_Type, 1000);
const XBOX::VError	VE_SECMAN_GSS_IMPORT_NAME_FAILED				= MAKE_VERROR(CSecurityManager::Component_Type, 1001);
const XBOX::VError	VE_SECMAN_KERBEROS_NOT_INITIALIZED			   	= MAKE_VERROR(CSecurityManager::Component_Type, 1002);
const XBOX::VError	VE_SECMAN_GSS_ACQUIRE_CRED_FAILED				= MAKE_VERROR(CSecurityManager::Component_Type, 1003);
const XBOX::VError	VE_SECMAN_GSS_REGISTER_ACCEPTOR_IDENTITY_FAILED	= MAKE_VERROR(CSecurityManager::Component_Type, 1004);
const XBOX::VError	VE_SECMAN_COULD_NOT_DECODE_AUTH_DATA			= MAKE_VERROR(CSecurityManager::Component_Type, 1005);
const XBOX::VError	VE_SECMAN_GSS_ACCEPT_SEC_CONTEXT_FAILED			= MAKE_VERROR(CSecurityManager::Component_Type, 1006);
const XBOX::VError	VE_SECMAN_GSS_DISPLAY_NAME_FAILED				= MAKE_VERROR(CSecurityManager::Component_Type, 1007);
const XBOX::VError	VE_SECMAN_USER_OR_GROUP_DOES_NOT_MATCH			= MAKE_VERROR(CSecurityManager::Component_Type, 1008);



// 2) Then define a custom class deriving from VComponentImp<YourComponent>
// and implement your public and private functions. If you want to receive messages,
// override ListenToMessage(). You may define several components using the same pattern.
//
// class VComponentImp1 : public VComponentImp<CComponent1>
// {
//	public:
//	Blah, blah - Public Interface
//
//	protected:
//	Blah, blah - Private Stuff
// };

class VSecurityManager : public VComponentImp<CSecurityManager>
{
public:
    //typedef	VComponentImp<CSecurityManager>	inherited;

	VSecurityManager();
	virtual ~VSecurityManager();

    VComponentLibrary* GetComponentLibrary() const;

	//XBOX::VError ValidateUserGroup (const XBOX::VString& inUserName, const XBOX::VString& inUserGroup);

    //Returns True/False and a valid/unchanged token on authentication success/failure.
    XBOX::VError ValidateBasicAuthentication(const XBOX::VString& inName, const XBOX::VString& inPassword, bool* outAuthOk, CUAGSession* &outUAGSession, XBOX::VJSGlobalContext* inContext, XBOX::VJSONObject* requestInfo);

	XBOX::VError ValidateDigestAuthentication (	const XBOX::VString& inUserName,
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
												CUAGSession* &outUAGSession,
												XBOX::VJSGlobalContext* inContext,
												XBOX::VJSONObject* requestInfo);
	
	XBOX::VString ComputeDigestHA1(	const XBOX::VString& inUserName,
									const XBOX::VString& inPassword, 
									const XBOX::VString& inRealm );
	

	XBOX::VError SetKerberosConfig(const XBOX::VString& inHostName, const XBOX::VString& inKeytabPath);

    //Returns True/False and valid/unchanged token and outAuthData on authentication success/failure.
    XBOX::VError ValidateKerberosAuthentication(const XBOX::VString& inAuthData, bool* outAuthOk, XBOX::VString* outAuthData, CUAGSession* &outUAGSession, XBOX::VJSONObject* requestInfo);

	/*
    //Returns True/False if token refers to a valid session or not
    bool IsValideToken(const XBOX::VString& inToken);
    
    //Tells the Security Manager that inToken isn't used anymore. Do not fail (doesn't complain on /bad/ inToken)
	void InvalidateToken(const XBOX::VString& inToken);
	*/

	//Set the user directory to ask for sessions. Accept NULL value.
	void SetUserDirectory(CUAGDirectory* inDirectory);

	virtual CUAGDirectory* GetUserDirectory();

	/*
	//returns the Authentication session used to check privilileges and group owning
	virtual CUAGSession* RetainAuthenticationSession(const XBOX::VString& inToken);
	*/


private :

    CUAGManager* fUAGManager;
    CUAGDirectory* fUAGDirectory;
    
    //typedef std::map<XBOX::VString, CUAGSession*> TokenCollection;

    //TokenCollection fValidTokens;
	//VCriticalSection fValidTokensMutex;

	//jmo - todo : virer ca !
	XBOX::VFilePath fUserDirectoryPath;

	bool			fKrbReady;

#if WITH_KERBEROS
    gss_cred_id_t	fCredentials;
#endif

};



// 3) Declare a kCOMPONENT_TYPE_LIST constant.
// This constant will automate the CreateComponent() in the dynamic lib:

const sLONG				kCOMPONENT_TYPE_COUNT	= 1;
const CImpDescriptor	kCOMPONENT_TYPE_LIST[]	= {{CSecurityManager::Component_Type, VImpCreator<VSecurityManager>::CreateImp}};



#endif
