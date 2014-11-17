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
#ifndef __HTTP_CONNECTION_LISTENER_INCLUDED__
#define __HTTP_CONNECTION_LISTENER_INCLUDED__

#include "ServerNet/VServerNet.h"

class VHTTPServer;


class VHTTPConnectionListener : public XBOX::IConnectionListener, public XBOX::VTask
{
public:
										VHTTPConnectionListener (VHTTPServer *inServer, IRequestLogger *inRequestLogger = NULL);
	virtual								~VHTTPConnectionListener();

	virtual XBOX::VError				StartListening();
	virtual bool						IsListening();
	virtual XBOX::VError				StopListening();

	/* TODO: Move this following funtion in VConnectionListener */
	virtual XBOX::VError				AddConnectionHandlerFactory (VConnectionHandlerFactory *inFactory);
	virtual XBOX::VError				AddWorkerPool (XBOX::VWorkerPool *inPool);
	virtual XBOX::VError				AddSelectIOPool (XBOX::VTCPSelectIOPool *inPool);

	virtual void						SetSSLCertificatesFolderPath (const XBOX::VFilePath& inCertificatesFolderPath);

	virtual XBOX::VError				GetPorts  (std::vector<PortNumber>& outPorts) { outPorts.push_back (GetPort()); return VE_OK; }
	virtual PortNumber					GetPort() { return fPort; }
	virtual void						SetPort (PortNumber inPort) { fPort = inPort; }
	virtual PortNumber					GetSSLPort() { return fSSLPort; }
	virtual void						SetSSLPort (PortNumber inSSLPort) { fSSLPort = inSSLPort; }
	virtual bool						IsSSLEnabled() { return fSSLEnabled; }
	virtual void						SetSSLEnabled (bool inSSLEnabled) { fSSLEnabled = inSSLEnabled; }
	virtual bool						IsSSLMandatory() { return fSSLMandatory; }
	virtual void						SetSSLMandatory (bool inValue);
	/* Used in SSLMandatory mode only: May accept incoming request from local network */
    virtual bool						GetAcceptHTTPOnLocal() { return fAcceptHTTPOnLocal; }
	virtual void						SetAcceptHTTPOnLocal (bool inValue) { fAcceptHTTPOnLocal = inValue; }

	/* Returns either an IP or 0 which means 'ALL'. */
	virtual const XBOX::VString&		GetListeningIP() const { return fListeningIP; }
	virtual void						SetListeningIP (const XBOX::VString& inIP) { fListeningIP.FromString (inIP); }
	virtual XBOX::VWorkerPool *			GetWorkerPool() const { return fWorkerPool; }
	virtual XBOX::VTCPSelectIOPool *	GetSelectIOPool() const { return fSelectIOPool; }

	bool								operator == (const VHTTPConnectionListener& inCpnnectionListener) const;

	void								IncrementUsageCounter();
	void								DecrementUsageCounter();
	sLONG								GetUsageCounter();
	
	void								SetSocketDescriptor (sLONG inValue) { fSocketDescriptor = inValue; }
	void								SetSSLSocketDescriptor (sLONG inValue) { fSSLSocketDescriptor = inValue; }

	void								SetReuseAddressSocketOption (bool inValue) { fReuseAddressSocketOption = inValue; }

	void								SaveToBag(XBOX::VValueBag &ioBag);

protected:
	virtual Boolean						DoRun();
	virtual void						DeInit();

	VSockListener *						fServerListener;
	XBOX::VWorkerPool *					fWorkerPool;
	unsigned short						fExclusiveWorkerCount;
	unsigned short						fExclusiveWorkerMaxCount;
	XBOX::VTCPSelectIOPool *			fSelectIOPool;
	XBOX::VFilePath						fCertificatesFolderPath;
	IRequestLogger *					fRequestLogger;
	bool								fAbortTask;

private:
	XBOX::VString						fListeningIP;
	
	PortNumber							fPort;
	PortNumber							fSSLPort;
	bool								fSSLEnabled;
	bool								fSSLMandatory;
    bool                                fAcceptHTTPOnLocal;
	VHTTPServer *						fHTTPServer;
	XBOX::VConnectionHandlerFactory *	fConnectionHandlerFactory;
	sLONG								fUsageCounter;
	
	/* for Sockets openened by HelperTools */
	sLONG								fSocketDescriptor;
	sLONG								fSSLSocketDescriptor;

	bool								fReuseAddressSocketOption;
	
	bool								_AcceptSocket (XBOX::XTCPSock *inSocket);
};


#endif	// __HTTP_CONNECTION_LISTENER_INCLUDED__
