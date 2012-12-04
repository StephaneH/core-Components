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
	
	// TODO: change VString by VFilePath for all file paths.
	virtual void						SetSSLCertificatePaths (const XBOX::VFilePath& inCertificatePath, const XBOX::VFilePath& inKeyPath);

	virtual XBOX::VError				GetPorts  (std::vector<PortNumber>& outPorts) { outPorts.push_back (GetPort()); return VE_OK; }
	virtual PortNumber					GetPort() { return fPort; }
	virtual void						SetPort (PortNumber inPort) { fPort = inPort; }
	virtual PortNumber					GetSSLPort() { return fSSLPort; }
	virtual void						SetSSLPort (PortNumber inSSLPort) { fSSLPort = inSSLPort; }
	virtual bool						IsSSLEnabled() { return fSSLEnabled; }
	virtual void						SetSSLEnabled (bool inSSLEnabled) { fSSLEnabled = inSSLEnabled; }
	virtual bool						IsSSLMandatory() { return fSSLMandatory; }
	virtual void						SetSSLMandatory (bool inValue);

	/* Returns either an IP or 0 which means 'ALL'. */
#if WITH_DEPRECATED_IPV4_API
	virtual IP4	/*done*/				GetListeningIP() const { return fListeningIP; }
	virtual void						SetListeningIP (const IP4 /*done*/ inIP) { fListeningIP = inIP; }
#else
	virtual const XBOX::VString&		GetListeningIP() const { return fListeningIP; }
	virtual void						SetListeningIP (const XBOX::VString& inIP) { fListeningIP.FromString (inIP); }
#endif
	
	virtual XBOX::VWorkerPool *			GetWorkerPool() const { return fWorkerPool; }
	virtual XBOX::VTCPSelectIOPool *	GetSelectIOPool() const { return fSelectIOPool; }

	bool								operator == (const VHTTPConnectionListener& inCpnnectionListener) const;

	void								IncrementUsageCounter();
	void								DecrementUsageCounter();
	sLONG								GetUsageCounter();
	
	void								SetSocketDescriptor (sLONG inValue) { fSocketDescriptor = inValue; }
	void								SetSSLSocketDescriptor (sLONG inValue) { fSSLSocketDescriptor = inValue; }

protected:
	virtual Boolean						DoRun();
	virtual void						DeInit();

	VSockListener *						fServerListener;
	XBOX::VWorkerPool *					fWorkerPool;
	XBOX::VTCPSelectIOPool *			fSelectIOPool;
	XBOX::VFilePath						fCertificatePath;
	XBOX::VFilePath						fKeyPath;
	IRequestLogger *					fRequestLogger;
	bool								fAbortTask;

private:
	
#if WITH_DEPRECATED_IPV4_API	
	IP4	/*done*/						fListeningIP;
#else
	XBOX::VString						fListeningIP;
#endif
	
	PortNumber							fPort;
	PortNumber							fSSLPort;
	bool								fSSLEnabled;
	bool								fSSLMandatory;
	VHTTPServer *						fHTTPServer;
	XBOX::VConnectionHandlerFactory *	fConnectionHandlerFactory;
	sLONG								fUsageCounter;
	
	/* for Sockets openened by HelperTools */
	sLONG								fSocketDescriptor;
	sLONG								fSSLSocketDescriptor;
};


#endif	// __HTTP_CONNECTION_LISTENER_INCLUDED__