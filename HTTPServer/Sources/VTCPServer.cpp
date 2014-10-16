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
#include "HTTPServer.h"
#include "VTCPServer.h"
#include "VHTTPConnectionHandler.h"


//--------------------------------------------------------------------------------------------------


VTCPServer::VTCPServer()
{
}


VTCPServer::~VTCPServer()
{
}


bool VTCPServer::SameConnectionListener (IConnectionListener *inListener)
{
	bool			isOK = false;
	XBOX::VTaskLock lock (&m_vmtxListenerProtector);

	std::vector<IConnectionListener *>::iterator it = m_vctrListeners.begin();
	while (it != m_vctrListeners.end())
	{
		if (dynamic_cast<VHTTPConnectionHandler *>(*it) == dynamic_cast<VHTTPConnectionHandler *>(inListener))
		{
			isOK = true;
			break;
		}

		++it;
	}

	return isOK;
}


XBOX::VError VTCPServer::RemoveConnectionListener (IConnectionListener *inListener)
{
	XBOX::VError	error = VE_OK;
	XBOX::VTaskLock lock (&m_vmtxListenerProtector);

	std::vector<IConnectionListener *>::iterator it = m_vctrListeners.begin();
	while (it != m_vctrListeners.end())
	{
		if ((*it) == inListener)
		{
			VHTTPConnectionListener*	httpCL = dynamic_cast<VHTTPConnectionListener*>(*it);
			if ((NULL != httpCL) && (0 == httpCL->GetUsageCounter()))
			{
				if ((*it)->IsListening())
					error = (*it)->StopListening();

				m_vctrListeners.erase (it);
				break;
			}
		}

		++it;
	}

	return error;
}


XBOX::VError VTCPServer::StartConnectionListener (IConnectionListener *inListener)
{
	XBOX::VError	error = VE_OK;
	XBOX::VTaskLock lock (&m_vmtxListenerProtector);

	std::vector<IConnectionListener *>::iterator it = m_vctrListeners.begin();
	while (it != m_vctrListeners.end())
	{
		if (((*it) == inListener) && !(*it)->IsListening())
		{
			error = (*it)->StartListening();
			break;
		}

		++it;
	}

	return error;
}


XBOX::VError VTCPServer::StopConnectionListener (IConnectionListener *inListener)
{
	XBOX::VError	error = VE_OK;
	XBOX::VTaskLock lock (&m_vmtxListenerProtector);

	std::vector<IConnectionListener *>::iterator it = m_vctrListeners.begin();
	while (it != m_vctrListeners.end())
	{
		if (((*it) == inListener) && (*it)->IsListening())
		{
			error = (*it)->StopListening();
			break;
		}

		++it;
	}

	return error;
}


bool VTCPServer::IsConnectionListenerRunning (IConnectionListener *inListener)
{
	bool			isListening = false;
	XBOX::VTaskLock lock (&m_vmtxListenerProtector);

	std::vector<IConnectionListener *>::iterator it = m_vctrListeners.begin();
	while (it != m_vctrListeners.end())
	{
		if ((*it) == inListener)
		{
			isListening = (*it)->IsListening();
			break;
		}

		++it;
	}

	return isListening;
}
