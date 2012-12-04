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
#ifndef __DataService__
#define __DataService__

// fgr 24.05.2000 Service de donnees en client/serveur
// Est vu comme un client toute partie de l'application de demande a acceder aux donnees
// Est vu comme un serveur toute partie du code qui repond aux demandes d'acces aux donnees

#include "VKernel.h"



typedef enum ReplyMode // mode de reponse
{
	kNoReply,					// pas de reponse requise
	kWaitForReply,				// attendre la reponse (synchrone)
	kDontWaitForReply			// ne pas attendre la reponse (asynchrone)
};

typedef struct RequestParamBlock
{
	uLONG8			fClientID;
	IMessageAble	*fClient;		// a nil pour les appel en reseau
	uLONG8			fSessionID;
	sLONG			fSelector;
	VStream			*fRequest;
	VStream			*fReply;
	ReplyMode		fMode;
	sLONG			fServerError;
} RequestParamBlock;

class VRequestMessage : public VMessage
{
public:	
	VRequestMessage();
	virtual ~VRequestMessage();

	RequestParamBlock	fMsgData;
};

class VClientInfo;

class VDBServer : public VObject, public IMessageAble
{
public:
	VDBServer();
	virtual ~VDBServer();

	void			Start(Boolean inAllowNetService,Boolean inAllowDirectService);
	void			StopAndWaitForDisconnections();
	void			StopAndWaitForTimer(sLONG inTimeBeforeStop); // inTimeBeforeStop en secondes (0 = arret immediat)
	void			Pause();
	void			Resume();



	virtual void	DoMessage(VMessage& inMessage);

private:
	void			ProcessRequest(RequestParamBlock &inPB);

	uLONG8			DoConnect(uLONG8 inClientID); // inClientID a 0 pour une nouvelle connexion
	void			DoDisconnect(uLONG8 inClientID,Boolean inDestroy);

	VClientInfo*	FindClientFromID(uLONG8 inClientID,sLONG *outIndex = nil);

private:
	VArrayListOf<VClientInfo*>			fClients;		// clients connectes

	Boolean								fRunning;		// en fonction
	Boolean								fPaused;		// en pause
	Boolean								fStopping;		// en cours d'arret
	sWORD								fStopMode;		// mode d'arret
	sLONG								fCountDown;		// decrement (secondes ou clients)
	uLONG8								fNextClientID;	// prochain numero de client
};


class VSessionInfo;

// Informations sur le client 
class VClientInfo : public VObject
{
private:
	VClientInfo();
	virtual ~VClientInfo();

	VSessionInfo*	OpenSession();
	void			CloseSession(uLONG8 inSessionID);

	VSessionInfo*	FindSessionFromID(uLONG8 inSessionID,sLONG *outIndex = nil);

private:
	uLONG8								fUniqueID;				// identifiant unique
	uLONG8								fNextSessionID;			// prochain ID de session
	Boolean								fConnected;				// connecte
	VDate								fLastConnectDate;		// date de debut de connexion
	VHour								fLastConnectTime;		// heure de debut de connexion
	VDate								fLastDisconnectDate;	// date de derniere deconnexion
	VHour								fLastDisconnectTime;	// heure de derniere deconnexion
	VArrayListOf<VSessionInfo*>			fSessions;				// sessions ouvertes

friend class VDBServer;
};

// Session ouverte par un client
class VSession : public VObject
{
public:
	// Requetes
	virtual sLONG SelectAllRecords(sLONG inTableNum) = 0;

private:
	VSession();
	virtual ~VSession() = 0;

private:
	uLONG8								fUniqueID;		// identifiant unique
	uLONG8								fClientID;		// identifiant client
};

class VSessionInfo : public VSession
{
public:
	// Requetes
	virtual sLONG SelectAllRecords(sLONG inTableNum);


private:
	VSessionInfo();
	virtual ~VSessionInfo();

private:
	// ensembles, selections, ...


friend class VClientInfo;
};

/*---------------------------------------------------------------------------------*/
enum // Requete
{
	// Les requetes de services sont en negatif
	kRQNoOperation = 0,
	kRQConnect = -1,
	kRQDisconnect = -2,
	kRQOpenSession = -3,
	kRQCloseSession = -4,

	// Les requetes utilisateurs sont en positif
	kRQAllrecords = 1,




	kRQLastRequest
};

enum	// Codes d'erreurs
{
	// erreurs renvoyees par le serveur
	kSEBadPacketHeader			= -80000,	// En-tete corrumpu
	kSEUnsupportedVersion		= -80001,	// Version incompatible entre le client et le serveur
	kSEBadRequestData			= -80002,
	kSETransmissionError		= -80003,
	kSEConnectionDropped		= -80004,
	kSENotModified				= -80005,	// Non modifie depuis la derniere requete
	kSEUnknownRequest			= -80006,
	kSEUnknownClient			= -80007,
	kSEUnkknownSession			= -80008,

	kSEOtherError				= -70000,
	
	kSENoError					= 0
};

#endif /* __DataService__ */
