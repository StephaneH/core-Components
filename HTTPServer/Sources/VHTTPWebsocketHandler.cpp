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
#include "VHTTPWebsocketHandler.h"

XBOX::VWebSocket::STATES IHTTPWebsocketHandler::GetState() const
{
	xbox_assert(fWebSocket != NULL);

	return fWebSocket->GetState();
}

XBOX::VError IHTTPWebsocketHandler::Close (sLONG inReasonCode)
{
	if (fWebSocket == NULL)

		return XBOX::VE_OK;

	xbox_assert(fWebSocket->GetState() != XBOX::VWebSocket::STATE_CONNECTING);

	XBOX::StErrorContextInstaller	context(false, true);
	XBOX::VError					error;
	
	if (fWebSocket->GetState() != XBOX::VWebSocket::STATE_OPEN)

		error = XBOX::VE_OK;

	else {

		uBYTE	reason[2];

		reason[0] = (inReasonCode << 8) & 0xff;
		reason[1] = inReasonCode & 0xff;

		error = fWebSocket->SendClose(0, reason, 2);

	}

	return error;
}

XBOX::VError IHTTPWebsocketHandler::ReadMessage (void *ioData, XBOX::VSize &ioLength, bool &outIsTerminated, sLONG inReadTimeOut)
{
	xbox_assert(ioData != NULL && ioLength && inReadTimeOut >= 0);
	xbox_assert(fWebSocket != NULL);	

	XBOX::StErrorContextInstaller	context(false, true);
	sLONG							sleepDuration;
	XBOX::VError					error;
	VSize							size;

	// Read message.

	sleepDuration = 0;
	for ( ; ; ) {

		bool	hasReadFrame;

		if (fWebSocket->GetState() == XBOX::VWebSocket::STATE_CLOSED) {
					
			error = XBOX::VE_SOCK_PEER_OVER;
			ioLength = 0;
			outIsTerminated = true;
			break;

		} else if ((error = fWebSocketMessage->Receive(&hasReadFrame, &outIsTerminated)) == XBOX::VE_OK) {

			// Read until a complete message has been read.

			if (outIsTerminated) {

				size = fWebSocketMessage->GetSize();
				break;

			}

		} else if (error == XBOX::VE_SRVR_RESOURCE_TEMPORARILY_UNAVAILABLE) {
					
			// Polling, but nothing to read.

			hasReadFrame = false;
			error = XBOX::VE_OK;
					
		} else {

			ioLength = 0;
			outIsTerminated = true;	// An error happened.
			break;

		}

		// If nothing was read, then wait.

		if (!hasReadFrame) {

			if (sleepDuration < inReadTimeOut) {

				sleepDuration += 100;
				XBOX::VTask::Sleep(100);

			} else {

				// Time-out return.

//				DebugMsg("WebSocket %p Time-out\n", this);

				ioLength = size = 0;
				outIsTerminated = true;
				break;

			}			 

		} else

			sleepDuration = 0;

	}

	// Copy message to output buffer.

	if (error == XBOX::VE_OK && size) {

		if (ioLength < size) {

			size = ioLength;		// Truncate message!
			xbox_assert(false);

		} else

			ioLength = size;

		::memcpy(ioData, fWebSocketMessage->GetMessage(), size);

		char	*p;

		p = (char *) ioData;
		p += size;
		*p = 0;

//		DebugMsg("%p message size = %d => %s\n", this, (int) size, (char *) ioData);

		fWebSocketMessage->ResetBuffer();

	}

	return error;
}

XBOX::VError IHTTPWebsocketHandler::WriteMessage (const void *inData, XBOX::VSize inLength, bool inIsTerminated)
{	
	xbox_assert(!(inData == NULL && inLength));
	xbox_assert(fWebSocket != NULL);
	
	XBOX::StErrorContextInstaller	context(false, true);
	XBOX::VError					error;

/*
	char	buffer[10000];

	::memcpy(buffer, inData, inLength);
	buffer[inLength] = 0;
	DebugMsg("%p Write %s\n", this, buffer);
*/
	if (fMessageStarted) {

		// Continue an already started message.

		if (inIsTerminated)

			fMessageStarted = false;

		error = fWebSocket->ContinueMessage(0, (const uBYTE *) inData, inLength, inIsTerminated);

	} else if (inIsTerminated) {

		// Send a complete message, may be fragmented.

		error = fWebSocket->SendMessage((const uBYTE *) inData, inLength, true);

	} else {

		// Start a new message, further data will follow.

		fMessageStarted = true;
		error = fWebSocket->StartMessage(true, 0, (const uBYTE *) inData, inLength);

	}

	return error;
}

VHTTPWebsocketClientHandler::VHTTPWebsocketClientHandler ()
{
	fWebSocket = NULL;
	fWebSocketMessage = NULL;
	fMessageStarted = false;
}

VHTTPWebsocketClientHandler::~VHTTPWebsocketClientHandler()
{
	if (fWebSocket != NULL) 

		delete fWebSocket;

	if (fWebSocketMessage != NULL)

		delete fWebSocketMessage;
}

XBOX::VError VHTTPWebsocketClientHandler::ConnectToServer (const XBOX::VURL &inUrl, bool inIsBlocking)
{
	XBOX::StErrorContextInstaller	context(false, true);	// Silence error throwing.
	XBOX::VWebSocketConnector		connector(inUrl);
	XBOX::VError					error;

	if ((error = connector.StartOpeningHandshake(NULL)) == XBOX::VE_OK
	&& (error = connector.ContinueOpeningHandshake()) == XBOX::VE_OK) {

		XBOX::VTCPEndPoint	*endPoint;

		endPoint = connector.StealEndPoint();
		xbox_assert(endPoint != NULL);
		
		endPoint->SetIsBlocking(inIsBlocking);
		if ((fWebSocket = new XBOX::VWebSocket(endPoint, true, connector.GetLeftOverDataPtr(), connector.GetLeftOverSize())) == NULL)

			error = XBOX::VE_MEMORY_FULL;

		else if ((fWebSocketMessage = new XBOX::VWebSocketMessage(fWebSocket)) == NULL) {
		
			delete fWebSocket;
			fWebSocket = NULL;
			error = XBOX::VE_MEMORY_FULL;
		
		} else 

			error = XBOX::VE_OK;

	}

	return error;
}

XBOX::VString VHTTPWebsocketClientHandler::GetPeerIP () const 
{
	xbox_assert(fWebSocket != NULL && fWebSocket->GetEndPoint() != NULL);

	return fWebSocket->GetEndPoint()->GetIP();
}

VHTTPWebsocketServerHandler::VHTTPWebsocketServerHandler ()
{
	fWebSocket = NULL;
	fWebSocketMessage = NULL;
	fMessageStarted = false;
}

VHTTPWebsocketServerHandler::~VHTTPWebsocketServerHandler()
{
	if (fWebSocket != NULL) 
		
		delete fWebSocket;

	if (fWebSocketMessage != NULL)

		delete fWebSocketMessage;
}

XBOX::VError VHTTPWebsocketServerHandler::TreatNewConnection (IHTTPResponse *inResponse, bool inDetachEndPoint)
{
	xbox_assert(inResponse != NULL);

	XBOX::StErrorContextInstaller	context(false, true);
	XBOX::VError					error;
    VHTTPHeader                     header      = inResponse->GetRequestHeader();
	XBOX::VTCPEndPoint				*endPoint   = inResponse->GetEndPoint();
    
	xbox_assert(endPoint != NULL);

	if ((error = XBOX::VWebSocketListener::SendOpeningHandshake(&header, endPoint)) == XBOX::VE_OK) {
	    
		if ((fWebSocket = new XBOX::VWebSocket(endPoint, true, NULL, 0, false)) == NULL)

			error = XBOX::VE_MEMORY_FULL;

		else if ((fWebSocketMessage = new XBOX::VWebSocketMessage(fWebSocket)) == NULL) {
		
			delete fWebSocket;
			fWebSocket = NULL;
			error = XBOX::VE_MEMORY_FULL;
		
		} else {

			if (inDetachEndPoint) 

				inResponse->DetachEndPoint();

			endPoint->SetIsBlocking(false);
			error = XBOX::VE_OK;

		}
		VTask::Sleep(100);

	}
	return error;
}