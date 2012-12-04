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

#include "SecurityManager.h"
#include "UsersAndGroups/Sources/UsersAndGroups.h"
#include "JavaScript/VJavaScript.h"

#if WITH_KERBEROS
	#include "gssapi.h"
	#include "gssapi_krb5.h"
#endif

//#include "VAssert.h"
#include <stdio.h>



#if defined WIN32 || defined WIN64 
#	define snprintf sprintf_s
#	define TRACE(msg) OutputDebugString(msg);
#else
#	define TRACE(msg) //No debug on mac right now !
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Init stuff
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// 4) Finally define a xDllMain and instantiate a VComponentLibrary* object (with the new operator).
// You'll pass your kCOMPONENT_TYPE_LIST array to the constructor. If you need custom library
// initialisations, you can write your own class and override contructor, DoRegister, DoUnregister...
//
// On Mac remember to customize library (PEF) entry-points with __EnterLibrary and __LeaveLibrary


VComponentLibrary *gVSecurityManagerLibrary = NULL;

void XBOX::xDllMain()
{
    ::DebugMsg("\n*** Hello from SecurityManager ! ***\n" );
	gVSecurityManagerLibrary = new VComponentLibrary(kCOMPONENT_TYPE_LIST, kCOMPONENT_TYPE_COUNT);
}


static
void CalcDigestChallengeForUser(const XBOX::VString& inUserName,
							   const XBOX::VString& inUserPassword,
							   const XBOX::VString& inAlgorithm,
							   const XBOX::VString& inRealm,
							   const XBOX::VString& inNonce,
							   const XBOX::VString& inCNonce,
							   const XBOX::VString& inQOP,
							   const XBOX::VString& inNonceCount,
							   const XBOX::VString& inURi,
							   const XBOX::VString& inMethod,
							   XBOX::VString& outResponse);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// VSecurityManagerLibrary
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//VSecurityManager *VSecurityManager::sCurrentInstance = NULL;



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// VSecurityManager
//
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


VSecurityManager::VSecurityManager() : fUAGManager(NULL), fUAGDirectory(NULL), fKrbReady(false)
#if WITH_KERBEROS
, fCredentials(GSS_C_NO_CREDENTIAL)
#endif
{
}


VSecurityManager::~VSecurityManager()
{
    //Release components we get with VComponentManager::RetainComponent

    if(fUAGManager)
        fUAGManager->Release();	//jmo - todo : Verifier ca !

    if(fUAGDirectory)
        fUAGDirectory->Release();//jmo - todo : voir ou sont fait les retain...


    //Release Sessions ptr
    /*
	TokenCollection::iterator it;

    for(it=fValidTokens.begin() ; it!=fValidTokens.end() ; ++it)
    {
        assert(it->second);

        if(it->second)
            ReleaseRefCountable(&it->second);
    }
	*/
}


void VSecurityManager::SetUserDirectory(CUAGDirectory* inDirectory)
{ 
	fUAGDirectory = RetainRefCountable(inDirectory);
}


CUAGDirectory* VSecurityManager::GetUserDirectory()
{
	return fUAGDirectory;
}


VComponentLibrary* VSecurityManager::GetComponentLibrary() const
{
	return gVSecurityManagerLibrary;
}

/*
XBOX::VError VSecurityManager::ValidateUserGroup (const XBOX::VString& inUserName, const XBOX::VString& inUserGroup)
{
	XBOX::VError errorCode = VE_SECMAN_USER_OR_GROUP_DOES_NOT_MATCH;

	if (!fUAGDirectory)
	{   
		errorCode = VE_SECMAN_NO_UAG_DIRECTORY;
		VErrorBase *error = new VErrorBase (errorCode, 0);
		VTask::GetCurrent()->PushRetainedError (error);
	}
	else
	{
		XBOX::VUUID groupID;

		if (fUAGDirectory->GetGroupID (inUserGroup, groupID))
		{
			CUAGSession *session = NULL;

			//XBOX::VTask::GetCurrent()->GetDebugContext().DisableUI();
			session = fUAGDirectory->OpenSession (inUserName, &errorCode);
			//XBOX::VTask::GetCurrent()->GetDebugContext().EnableUI();

			if (session != NULL && errorCode == XBOX::VE_OK)
			{
				if (session->BelongsTo (groupID))
					errorCode = XBOX::VE_OK;
			}

			XBOX::QuickReleaseRefCountable(session);
		}
	}

	return errorCode;
}
*/

//Returns True/False and a valid/unchanged token on authentication success/failure.
XBOX::VError VSecurityManager::ValidateBasicAuthentication(const XBOX::VString& inName, const XBOX::VString& inPassword, bool* outAuthOk, CUAGSession* &outUAGSession, XBOX::VJSGlobalContext* inContext)
{
    *outAuthOk=false;
	outUAGSession = NULL;
    XBOX::VError rv=VE_OK;

	if(!fUAGDirectory)
	{   
		rv=VE_SECMAN_NO_UAG_DIRECTORY;
		VErrorBase*	eb=new VErrorBase(rv, 0);
		VTask::GetCurrent()->PushRetainedError(eb);
    }
	else
	{
		CUAGSession* session=NULL;

		//jmo : Le fait de ne pas reussir a ouvrir une session n'est pas une erreur
		//      mais un simple echec d'authentification. Par consequent, on masque
		//      le popup et on ne repercute pas /l'erreur/.

		XBOX::VTask::GetCurrent()->GetDebugContext().DisableUI();
		if (inContext != NULL)
		{
			// sc 22/06/2012, custom JavaScript authentication support
			XBOX::VJSContext jsContext( inContext);
			session=fUAGDirectory->OpenSession( inName, inPassword, &rv, &jsContext);
		}
		else
		{
			session=fUAGDirectory->OpenSession(inName, inPassword, &rv, NULL);
		}
		XBOX::VTask::GetCurrent()->GetDebugContext().EnableUI();

		if(session)
		{

			/*
			XBOX::VUUID id(true);
			id.GetString(*outToken);
			{
				VTaskLock lock(&fValidTokensMutex);
				fValidTokens[*outToken]=session;
			}
			*/
			outUAGSession = session;

			*outAuthOk=true;
		}
	}

    return rv;
}


XBOX::VError VSecurityManager::ValidateDigestAuthentication (	const XBOX::VString& inUserName,
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
																XBOX::VJSGlobalContext* inContext)
{
	//jmo - FIX ME : pour l'instant le realm est codŽ en dur dans le reste de Wakanda ! 
	//				 (et le realm reu ici, "Digest Realm", ne correspond pas)
	//				 WAK0072424
	xbox_assert(inRealm==CVSTR("Wakanda"));
	
	VString WakandaRealm=CVSTR("Wakanda");
	
	XBOX::VError error = XBOX::VE_OK;

	if (outAuthOk)
		*outAuthOk = false;

	if(!fUAGDirectory)
	{   
		error = VE_SECMAN_NO_UAG_DIRECTORY;
		VErrorBase *eb = new VErrorBase (error, 0);
		VTask::GetCurrent()->PushRetainedError (eb);
    }
	else
	{
		CUAGSession *session = NULL;

		/*
		XBOX::VTask::GetCurrent()->GetDebugContext().DisableUI();
		session = fUAGDirectory->OpenSession (inUserName, &error);
		XBOX::VTask::GetCurrent()->GetDebugContext().EnableUI();
		*/

		if (/*session != NULL && */error == XBOX::VE_OK)
		{
			XBOX::VString challenge;
			XBOX::VString ha1;
			
			CUAGUser *user = fUAGDirectory->RetainUser (inUserName, &error);
			
			if (user != NULL)
			{
#if VERSIONDEBUG && 0
				XBOX::VValueBag *extras = user->RetainExtraProperties();
				if (extras != NULL)
				{
					XBOX::VString string ("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
					extras->DumpXML (string, "uagExtrasProps", true);
					XBOX::ReleaseRefCountable (&extras);
				}
#endif
				user->GetHA1(ha1);
			}
			
			CalcDigestChallengeForUser(	inUserName,
										ha1,
										inAlgorithm,
										WakandaRealm,	//jmo - FIX ME !
										inNonce,
										inCNonce,
										inQOP,
										inNonceCount,
										inURi,
										inMethod,
										challenge);

			if (challenge.EqualToString (inChallenge))
			{
				/*
				XBOX::VUUID id (true);
				id.GetString (*outToken);
				{
					VTaskLock lock(&fValidTokensMutex);
					fValidTokens[*outToken] = session;
				}
				*/

				if (inContext != NULL)
				{
					// sc 22/06/2012, custom JavaScript authentication support
					XBOX::VJSContext jsContext( inContext);
					outUAGSession = fUAGDirectory->OpenSession(user, &error, &jsContext);
				}
				else
				{
					outUAGSession = fUAGDirectory->OpenSession(user, &error, NULL);
				}

				*outAuthOk = true;
			}

			XBOX::ReleaseRefCountable (&user);
		}
	}

	return error;
}


#define HASHLEN 16
#define HASHHEXLEN 32

typedef uBYTE HASH[HASHLEN];
typedef uBYTE HASHHEX[HASHHEXLEN+1];

void CvtHex (HASH Bin, HASHHEX Hex)
{
	uWORD	i;
	uBYTE 	j;

	for (i = 0; i < HASHLEN; i++)
	{
		j = (Bin[i] >> 4) & 0xf;
		if(j <= 9)
			Hex[i*2] = (j + '0');
		else
			Hex[i*2] = (j + 'a' - 10);
		j = Bin[i] & 0xf;
		if(j <= 9)
			Hex[i*2+1] = (j + '0');
		else
			Hex[i*2+1] = (j + 'a' - 10);
	}
	Hex[HASHHEXLEN] = 0;
}


void DigestCalcHA1(	uBYTE *pszAlg, 
					uBYTE *pszUserName,
					uBYTE *pszRealm,
					uBYTE *pszPassword,
					uBYTE *pszNonce,
					uBYTE *pszCNonce,
					HASHHEX SessionKey)
{
	HASH				HA1 = {0};
	XBOX::VChecksumMD5	checksum;

	checksum.Update (pszUserName, strlen((char *)pszUserName));
	checksum.Update ((uBYTE *)":", 1);
	checksum.Update (pszRealm, strlen((char *)pszRealm));
	checksum.Update ((uBYTE *)":", 1);
	checksum.Update (pszPassword, strlen((char *)pszPassword));
	checksum.GetChecksum (HA1);

	if (pszAlg!=NULL && !strcmp ((char *)pszAlg, "md5-sess"))
	{
		checksum.Clear();
		checksum.Update (HA1, HASHLEN);
		checksum.Update ((uBYTE *)":", 1);
		checksum.Update (pszNonce, strlen((char *)pszNonce));
		checksum.Update ((uBYTE *)":", 1);
		checksum.Update (pszCNonce, strlen((char *)pszCNonce));
		checksum.GetChecksum (HA1);
	}

	CvtHex (HA1, SessionKey);
}



void DigestCalcResponse (HASHHEX HA1, 
						uBYTE *pszNonce, 
						uBYTE *pszNonceCount, 
						uBYTE *pszCNonce, 
						uBYTE *pszQop, 
						uBYTE *pszMethod, 
						uBYTE *pszDigestUri, 
						HASHHEX HEntity, 
						HASHHEX Response)
{
	HASH				HA2 = {0};
	HASH				RespHash = {0};
	HASHHEX				HA2Hex = {0};
	XBOX::VChecksumMD5	checksum;

	checksum.Update (pszMethod, strlen ((char *)pszMethod));
	checksum.Update ((uBYTE *)":", 1);
	checksum.Update (pszDigestUri, strlen ((char *)pszDigestUri));
	if (!strcmp ((char *)pszQop, "auth-int"))
	{
		checksum.Update ((uBYTE *)":", 1);
		checksum.Update (HEntity, HASHHEXLEN);
	}

	checksum.GetChecksum (HA2);
	CvtHex (HA2, HA2Hex);

	checksum.Clear();
	checksum.Update (HA1, HASHHEXLEN);
	checksum.Update ((uBYTE *)":", 1);
	checksum.Update (pszNonce, strlen ((char *)pszNonce));
	checksum.Update ((uBYTE *)":", 1);
	if (*pszQop)
	{
		checksum.Update (pszNonceCount, strlen ((char *)pszNonceCount));
		checksum.Update ((uBYTE *)":", 1);
		checksum.Update (pszCNonce, strlen ((char *)pszCNonce));
		checksum.Update ((uBYTE *)":", 1);
		checksum.Update (pszQop, strlen ((char *)pszQop));
		checksum.Update ((uBYTE *)":", 1);
	}

	checksum.Update (HA2Hex, HASHHEXLEN);
	checksum.GetChecksum (RespHash);
	CvtHex (RespHash, Response);
}


void CalcDigestChallengeForUser(	const XBOX::VString& inUserName,
							   const XBOX::VString& inUserHA1,
							   const XBOX::VString& inAlgorithm,
							   const XBOX::VString& inRealm,
							   const XBOX::VString& inNonce,
							   const XBOX::VString& inCNonce,
							   const XBOX::VString& inQOP,
							   const XBOX::VString& inNonceCount,
							   const XBOX::VString& inURi,
							   const XBOX::VString& inMethod,
							   XBOX::VString& outResponse)
{
	uBYTE uname[256], algorithm[256],realm[256], nonce[256], cnonce[256], uri[256], qop[256], nonceCount[256], methodName[256];

	HASHHEX HA1 = {0};
	
	inUserName.ToCString ((char*)uname, sizeof(uname));
	inUserHA1.ToCString ((char*)HA1, sizeof(HA1));
	inAlgorithm.ToCString ((char*)algorithm, sizeof(algorithm));
	inRealm.ToCString ((char*)realm, sizeof(realm));
	inNonce.ToCString ((char*)nonce, sizeof(nonce));
	inCNonce.ToCString ((char*)cnonce, sizeof(cnonce));
	inQOP.ToCString ((char*)qop, sizeof(qop));
	inNonceCount.ToCString ((char*)nonceCount, sizeof(nonceCount));
	inURi.ToCString ((char*)uri, sizeof(uri));
	inMethod.ToCString ((char*)methodName, sizeof(methodName));

	
	HASHHEX HA2 = {0};
	HASHHEX response = {0};

	DigestCalcResponse (HA1, nonce, nonceCount, cnonce, qop, methodName, uri, HA2, response);

	outResponse.FromCString ((char *)response);
}


XBOX::VString VSecurityManager::ComputeDigestHA1(	const XBOX::VString& inUserName,
													const XBOX::VString& inPassword, 
													const XBOX::VString& inRealm )
{
	
	uBYTE uname[256], realm[256], passwd[256];
	
	inUserName.ToCString ((char*)uname, sizeof(uname));
	inPassword.ToCString ((char*)passwd, sizeof(passwd));
	inRealm.ToCString ((char*)realm, sizeof(realm));

	
	HASHHEX ha1= {0};;
	
	DigestCalcHA1 (NULL, uname, realm, passwd, NULL, NULL, ha1);
	
	VString result;
	result.FromCString((const char*)&ha1[0]);
	return result;
}


//jmo - todo : remplacer ca par une vraie fonction de log !
void gss_log(const char* prefix, uLONG MS, uLONG ms)
{
#if WITH_KERBEROS
    char  tmp[1024];

	memset(tmp, 0, sizeof(tmp));

    char* pos=tmp;
    char* past=tmp+sizeof(tmp);

	const char* fmt="%s : ";
	const char* pref=prefix;

    OM_uint32		ctx=0;

    do
    {
		pos+=snprintf(pos, past-pos, fmt, pref);

        OM_uint32		RES=0;
		OM_uint32		res=0;

        gss_buffer_desc buf;

        //RES=gss_display_status(&res, MS, GSS_C_GSS_CODE, GSS_C_NO_OID, &ctx, &buf);
		RES=gss_display_status(&res, ms, GSS_C_MECH_CODE, GSS_C_NO_OID, &ctx, &buf);

        switch(RES)
        {
        case GSS_S_COMPLETE :
            pos+=snprintf(pos, past-pos, "%s ", (char*)buf.value);
            break;

        case GSS_S_BAD_MECH :
            pos+=printf(pos, past-pos, "gss_display_status : Bad mech. requested\n");
            ctx=0;
            break;
            
        case GSS_S_BAD_STATUS :	
            //jmo - Statut code en dur : on ne devrait jamais l'avoir !
            pos+=printf(pos, past-pos, "gss_display_status : Bad status\n");
            ctx=0;
            break;

        default :
            //jmo - On a traite tous les cas, on ne devrait jamais arriver la !
            pos+=snprintf(pos, past-pos, "gss_display_status : Unknown error\n");
            ctx=0;
        }
        
        gss_release_buffer(&res, &buf);

		fmt=" %s ";
		pref=";";
    }
    while(ctx);

   TRACE(tmp);
   TRACE("\n");
#endif
}


XBOX::VError VSecurityManager::SetKerberosConfig(const XBOX::VString& inHostName, const XBOX::VString& inKeytabPath)
{
    XBOX::VError rv=VE_OK;

#if WITH_KERBEROS

    ////////////////////////////////////////////////////////////////////////////////
    // Charger la bonne keytab
    ////////////////////////////////////////////////////////////////////////////////

    XBOX::VStringConvertBuffer	tmpPath(inKeytabPath, XBOX::VTC_StdLib_char);
   
    OM_uint32	MS, ms;
    MS=krb5_gss_register_acceptor_identity(tmpPath.GetCPointer());
    
    if(GSS_ERROR(MS))
    {
		//jmo - todo : Est-ce bien comme cela qu'on doit tester l'erreur de cette fonction ?
		//             Il n'y a pas de code mineur...
        gss_log("krb5_gss_register_acceptor_identity", MS, 0);
        rv=VE_SECMAN_GSS_REGISTER_ACCEPTOR_IDENTITY_FAILED;
		VErrorBase*	eb=new VErrorBase(rv, 0);
		VTask::GetCurrent()->PushRetainedError(eb);
    }


    ////////////////////////////////////////////////////////////////////////////////
    // Obtenir un nom au format interne dans srvName
    ////////////////////////////////////////////////////////////////////////////////

	XBOX::VString				tmpName=XBOX::VString("HTTP@")+inHostName;
    XBOX::VStringConvertBuffer	tmpBuff(tmpName, XBOX::VTC_StdLib_char);
    gss_buffer_desc				tmpTok=GSS_C_EMPTY_BUFFER;
    gss_name_t					srvName=GSS_C_NO_NAME;

    //tmpTok.value="HTTP@srvdev" or "HTTP@srvdev.vmnet.local" : it works in both cases

	//jmo : peu elegant, mais me parait peu dangereux...
    tmpTok.value=const_cast<char*>(tmpBuff.GetCPointer());
    tmpTok.length=tmpBuff.GetLength();

    if(rv==VE_OK)
    {
        MS=gss_import_name(&ms, &tmpTok, (gss_OID) GSS_C_NT_HOSTBASED_SERVICE, &srvName);
        
        if(GSS_ERROR(MS))
        {
            gss_log("gss_import_name", MS, ms);
            rv=VE_SECMAN_GSS_IMPORT_NAME_FAILED;
			VErrorBase*	eb=new VErrorBase(rv, 0);
			VTask::GetCurrent()->PushRetainedError(eb);
        }
    }


    ////////////////////////////////////////////////////////////////////////////////
    // Obtenir les accreditations associees a srvName
    ////////////////////////////////////////////////////////////////////////////////

    if(rv==VE_OK)
    {
        if(fCredentials!=GSS_C_NO_CREDENTIAL)
            gss_release_cred(&ms, &fCredentials);

        MS=gss_acquire_cred(&ms, srvName, GSS_C_INDEFINITE, GSS_C_NO_OID_SET, GSS_C_ACCEPT,
                            &fCredentials, NULL, NULL);
    
        if(GSS_ERROR(MS))
        {
            gss_log("gss_acquire_cred", MS, ms);
            rv=VE_SECMAN_GSS_ACQUIRE_CRED_FAILED;
			VErrorBase*	eb=new VErrorBase(rv, 0);
			VTask::GetCurrent()->PushRetainedError(eb);
        }
        
        fKrbReady=true;
    }


    ////////////////////////////////////////////////////////////////////////////////
    // Menage !
    ////////////////////////////////////////////////////////////////////////////////

    if(srvName!=NULL)
        MS=gss_release_name(&ms, &srvName);

#endif

    return rv;
}


//Returns True/False and valid/unchanged token and outAuthData on authentication success/failure.
XBOX::VError VSecurityManager::ValidateKerberosAuthentication(const XBOX::VString& inAuthData, bool* outAuthOk, XBOX::VString* outAuthData, CUAGSession* &outUAGSession)
{
    *outAuthOk=false;
	outUAGSession = NULL;
    XBOX::VError rv=VE_OK;

    if(!fKrbReady)
    {
        rv=VE_SECMAN_KERBEROS_NOT_INITIALIZED;
		VErrorBase*	eb=new VErrorBase(rv, 0);
		VTask::GetCurrent()->PushRetainedError(eb);
        
        return rv;
    }

#if WITH_KERBEROS

    ////////////////////////////////////////////////////////////////////////////////
    // Decoder les infos d'authentification
    ////////////////////////////////////////////////////////////////////////////////

   	XBOX::VStringConvertBuffer	inBuff(inAuthData, XBOX::VTC_StdLib_char);

	unsigned int				b64Len=0;
	const unsigned char* 		b64Ptr=reinterpret_cast<const unsigned char*>(inBuff.GetCPointer());

    gss_buffer_desc		 		inTok=GSS_C_EMPTY_BUFFER;

	inTok.value=XBOX::Base64Coder::Decode(b64Ptr, &b64Len);
	inTok.length=b64Len;

    if(b64Len==0)
    {
        rv=VE_SECMAN_COULD_NOT_DECODE_AUTH_DATA;
		VErrorBase*	eb=new VErrorBase(rv, 0);
		VTask::GetCurrent()->PushRetainedError(eb);
    }


    ////////////////////////////////////////////////////////////////////////////////
    // Valider le contexte du client et preparer une reponse dans outTok
    ////////////////////////////////////////////////////////////////////////////////

    gss_ctx_id_t	ctx=GSS_C_NO_CONTEXT;
    gss_buffer_desc	outTok=GSS_C_EMPTY_BUFFER;
    gss_name_t		cliName=GSS_C_NO_NAME;
    gss_cred_id_t	dlgCred=GSS_C_NO_CREDENTIAL;
    OM_uint32		flags=0;
    OM_uint32		MS, ms;
	bool			gssAuthorized=false;

    if(rv==VE_OK)
    {
        //jmo : pour l'instant on n'utilise pas la delegation d'accreditation
        //MS=gss_accept_sec_context(&ms, &ctx, fCredentials, &inTok, GSS_C_NO_CHANNEL_BINDINGS,
        //                          &cliName, NULL, &outTok, &flags, NULL, &dlgCred);

        MS=gss_accept_sec_context(&ms, &ctx, fCredentials, &inTok, GSS_C_NO_CHANNEL_BINDINGS,
                                  &cliName, NULL, &outTok, &flags, NULL, NULL);

        switch(MS)
        {
        case GSS_S_COMPLETE :
            gssAuthorized=true;
            break;
            
        case GSS_S_BAD_BINDINGS :
        case GSS_S_BAD_MECH :
        case GSS_S_BAD_SIG :
        case GSS_S_CREDENTIALS_EXPIRED :
        case GSS_S_DEFECTIVE_CREDENTIAL :
        case GSS_S_DEFECTIVE_TOKEN :
        case GSS_S_DUPLICATE_TOKEN :
        case GSS_S_FAILURE :
        case GSS_S_NO_CONTEXT :
        case GSS_S_NO_CRED :
        case GSS_S_OLD_TOKEN :
            gssAuthorized=false;
            break;
            
        case GSS_S_CONTINUE_NEEDED :
            //Si on ne se concentrait pas sur Kerberos, il faudrait supporter
            //plusieurs allers-retours et promener le contexte... Quoiqu'il en soit,
            //dans le cas de kerberos, les allers-retours s'arretent la.
        default :
            gssAuthorized=false;
            
            gss_log("gss_accept_sec_context", MS, ms);
            rv=VE_SECMAN_GSS_ACCEPT_SEC_CONTEXT_FAILED;
			VErrorBase*	eb=new VErrorBase(rv, 0);
			VTask::GetCurrent()->PushRetainedError(eb);
        }

        if(gssAuthorized && (outTok.length==0 ||cliName==GSS_C_NO_NAME))
            gssAuthorized=false;
    }

    ////////////////////////////////////////////////////////////////////////////////
    // Obtenir le principal du client et sauvegarder ses accreditations
    ////////////////////////////////////////////////////////////////////////////////

    gss_buffer_desc	nameTok=GSS_C_EMPTY_BUFFER;
	VString userAtRealm;

    if(rv==VE_OK)
    {
        if(gssAuthorized)
        {
            MS=gss_display_name(&ms, cliName, &nameTok, NULL);
            
            if(GSS_ERROR(MS))
            {
                gss_log("gss_display_name", MS, ms);
                rv=VE_SECMAN_GSS_DISPLAY_NAME_FAILED;
				VErrorBase*	eb=new VErrorBase(rv, 0);
				VTask::GetCurrent()->PushRetainedError(eb);
            }
            else if(nameTok.length>0)
            {
                //jmo : Pour l'instant, on n'utilise pas la delegation d'accreditation. Regarder mod. Apache...
                // int	save_cred=0;
                //
                // if(save_cred && dlgCred!=GSS_C_NO_CREDENTIAL)
                // {
                //     store_gss_creds((char *)nameTok.value, dlgCred);
                // }

                userAtRealm.FromBlock(nameTok.value, nameTok.length, XBOX::VTC_StdLib_char);	//jmo - todo : ca ou XBOX::VTC_UTF_8 ?
            }
        }
    }


    ////////////////////////////////////////////////////////////////////////////////
    // Refus ou succes et preparation de l'entete de reponse avec outTok
    ////////////////////////////////////////////////////////////////////////////////

    if(rv==VE_OK && gssAuthorized)
	{
		if(!fUAGDirectory)
		{   
			rv=VE_SECMAN_NO_UAG_DIRECTORY;
			VErrorBase*	eb=new VErrorBase(rv, 0);
			VTask::GetCurrent()->PushRetainedError(eb);
		}
		else
		{
			CUAGSession* session=NULL;

			//jmo : Le fait de ne pas reussir a ouvrir une session n'est pas une erreur
			//      mais un simple echec d'authentification. Par consequent, on masque
			//      le popup et on ne repercute pas /l'erreur/.

			XBOX::VTask::GetCurrent()->GetDebugContext().DisableUI();
			session=fUAGDirectory->OpenSession(userAtRealm, &rv);
			XBOX::VTask::GetCurrent()->GetDebugContext().EnableUI();

			if(session)
			{
				/*
				XBOX::VUUID id(true);
				id.GetString(*outToken);
				{
					VTaskLock lock(&fValidTokensMutex);
					fValidTokens[*outToken]=session;
				}
				*/

				outUAGSession = session;
				*outAuthOk=true;
			}

			if(*outAuthOk && outTok.length>0 && outAuthData)
			{      
				b64Len=0;
				b64Ptr=XBOX::Base64Coder::Encode((const unsigned char*)outTok.value, outTok.length, &b64Len);
        
				if(b64Ptr!=NULL)
				{
					outAuthData->FromBlock(b64Ptr, b64Len, XBOX::VTC_StdLib_char);
					delete b64Ptr;
					b64Ptr=NULL;
				}            
			}
		}
	}


    ////////////////////////////////////////////////////////////////////////////////
    // Menage !
    ////////////////////////////////////////////////////////////////////////////////

    if(inTok.length>0)
        delete[] inTok.value;

    if(ctx!=GSS_C_NO_CONTEXT)
        gss_delete_sec_context(&ms, &ctx, GSS_C_NO_BUFFER);
   
    if(outTok.length>0)
        gss_release_buffer(&ms, &outTok); 

    if(cliName!=GSS_C_NO_NAME)
        gss_release_name(&ms, &cliName);
     
    if (dlgCred!=GSS_C_NO_CREDENTIAL)
        gss_release_cred(&ms, &dlgCred);
    
    if(nameTok.length>0)
        gss_release_buffer(&ms, &nameTok);

#endif

    return rv;
}


/*
//Returns True/False if token refers to a valid session or not
bool VSecurityManager::IsValideToken(const XBOX::VString& inToken)
{
	VTaskLock lock(&fValidTokensMutex);
    return fValidTokens.find(inToken)!=fValidTokens.end();
}


//Tells the Security Manager that inToken isn't used anymore. Do not fail (doesn't complain on /bad/ inToken)
void VSecurityManager::InvalidateToken(const XBOX::VString& inToken)
{
	VTaskLock lock(&fValidTokensMutex);
	TokenCollection::iterator it=fValidTokens.find(inToken);
    
    if(it!=fValidTokens.end())
    {
        assert(it->second);

        if(it->second)
            ReleaseRefCountable(&it->second);
        
        fValidTokens.erase(it);
    }
}


CUAGSession* VSecurityManager::RetainAuthenticationSession(const XBOX::VString& inToken)
{
	VTaskLock lock(&fValidTokensMutex);
	CUAGSession* result;

	TokenCollection::iterator it = fValidTokens.find(inToken);
	if (it == fValidTokens.end())
		result = nil;
	else
	{
		result = it->second;
		result->Retain();
	}

	return result;
}
*/