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
#ifndef __HTTP_CONNECTION_HANDLER_INCLUDED__
#define __HTTP_CONNECTION_HANDLER_INCLUDED__


#include "VHTTPConnectionListener.h"


class VHTTPServerSession;


class VHTTPConnectionHandler : public VConnectionHandler
{
public :

	enum	{ Handler_Type = 'HTTP' };

									VHTTPConnectionHandler (VHTTPServer * inServer);
	virtual							~VHTTPConnectionHandler();

	virtual XBOX::VError			SetEndPoint (XBOX::VEndPoint *inEndPoint);
	virtual bool					CanShareWorker() { return false; } /* Exclusive. */

	virtual enum VConnectionHandler::E_WORK_STATUS	Handle (XBOX::VError& outError);

	virtual int						GetType() { return Handler_Type; }
	virtual XBOX::VError			Stop();

private :
	VHTTPServer *					fHTTPServer;
	VHTTPServerSession *			fHTTPServerSession;
	bool							fShouldStop;

	static XBOX::VString			fTaskName;
};


#endif	// __HTTP_CONNECTION_HANDLER_INCLUDED__
