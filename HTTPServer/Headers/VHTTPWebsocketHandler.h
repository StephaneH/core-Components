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
#ifndef __VHTTP_WEBSOCKET_HANDLER__
#define __VHTTP_WEBSOCKET_HANDLER__

#if CHTTP_SERVER_USE_NEW_WEB_SOCKET_CODE

class VHTTPWebsocketClientHandler : public IHTTPWebsocketClient
{
public:

							VHTTPWebsocketClientHandler ();
	virtual					~VHTTPWebsocketClientHandler ();

	virtual	XBOX::VError	ConnectToServer (const XBOX::VURL &inUrl, bool isBlocking = true);
	virtual	XBOX::VString	GetPeerIP () const;
};

class VHTTPWebsocketServerHandler : public IHTTPWebsocketServer
{
public:

							VHTTPWebsocketServerHandler ();
	virtual					~VHTTPWebsocketServerHandler ();

	virtual XBOX::VError	TreatNewConnection (IHTTPResponse *inResponse, bool inDetachEndPoint);
};

#else

class VHTTPWebsocketClientHandler: public XBOX::VObject, public IHTTPWebsocketClient
{
public:
									VHTTPWebsocketClientHandler();
									~VHTTPWebsocketClientHandler();

	XBOX::VError					ConnectToServer(const XBOX::VURL& inUrl, bool isBlocking=true);

	// closes the WebSocket: no more transmissions will be possible
	XBOX::VError					Close();

	
	// reads ioLength bytes from current frame and puts them into Data, ioLength is updated regarding
	// the number of bytes actually read. IsTerminated is set when all the bytes of the curt msg have been read
	// When IoLength is returned NULL, no data are available
	XBOX::VError					ReadMessage( void* inData, VSize& ioLength, bool& outIsTerminated );


	// writes Length bytes to the Frame. IsTerminated specifies that the whole message has been compltely send
	// to the Websocket object: in this case all the data have been physically sent when WriteMessage returns
	XBOX::VError					WriteMessage( const void* inData, VSize inLength, bool inIsTerminated  );

	WsState							GetState() const;

	// Return the IP of the peer (WebSocket protocol connected server).

	XBOX::VString					GetPeerIP () const;

private:

#if CHTTP_SERVER_USE_OLD_CLIENT_WS

	sLONG8							fBytesToRead;
	sLONG8							fCurt;
	bool							fOutputIsTerminated;
	XBOX::VString					fKey;
	sBYTE							fMask[4];
#define K_WEBSOCKET_HANDLER_MAX_SIZE	(1024)
	sBYTE							fOutBuffer[K_WEBSOCKET_HANDLER_MAX_SIZE];
	sBYTE							fInBuffer[K_WEBSOCKET_HANDLER_MAX_SIZE];

	XBOX::VError					CreateKey();
	XBOX::VError					SendHandshake(const VString& inHost, const VString& inPath);
	XBOX::VError					TreatHandshakeResponse(sLONG& ioLen);
	XBOX::VError					WriteHeader(XBOX::VSize inLength);
	XBOX::VError					ReadHeader(bool& outFound);
	
	XBOX::VError					ConnectToServer(XBOX::XTCPSock* inTcpSck, const XBOX::VString& inHostPort, const VString& inData);


typedef enum ws_opcode_enum
{
	CONTINUATION_FRAME=0,
	TEXT_FRAME=1,
	BIN_FRAME=2,
	CONNEXION_CLOSE=8,
	PING_FRAME=9,
	PONG_FRAME=0xA
} ws_opcode_t;

typedef struct ws_frame_st 
{
	ws_opcode_t		opcode;
	unsigned char	header[14];
	XBOX::VSize		buf_len;
	int				masked;
	sLONG8			len;
} ws_frame_t;

	ws_frame_t						fFrame;
	ws_frame_t						fReadFrame;

#else 


#endif

};

//
// this class is in charge of handling the WEBSOCKET protocol (RFC6455)
//
// the implementation is NOT THREAD-SAFE !!!! the user is supposed to call each
// method (sequentially) from the same calling thread
//


class VHTTPWebsocketServerHandler: public XBOX::VObject, public IHTTPWebsocketServer
{
public:
									VHTTPWebsocketServerHandler();

	virtual							~VHTTPWebsocketServerHandler();

	XBOX::VError					TreatNewConnection(IHTTPResponse* inResponse, bool inDetachEndPoint);

	// closes the WebSocket: no more transmissions will be possible
	XBOX::VError					Close();

	
	// reads ioLength bytes from current frame and puts them into Data, ioLength is updated regarding
	// the number of bytes actually read. IsTerminated is set when all the bytes of the curt msg have been read
	// When IoLength is returned NULL, no data are available
	XBOX::VError					ReadMessage( void* inData, VSize& ioLength, bool& outIsTerminated );


	// writes Length bytes to the Frame. IsTerminated specifies that the whole message has been compltely send
	// to the Websocket object: in this case all the data have been physically sent when WriteMessage returns
	XBOX::VError					WriteMessage( const void* inData, VSize inLength, bool inIsTerminated  );

	WsState							GetState() const;

private:
	sLONG8							fBytesToRead;
	sLONG8							fCurt;
	bool							fOutputIsTerminated;
	bool							fDetachEndPoint;

typedef enum ws_opcode_enum
{
	CONTINUATION_FRAME=0,
	TEXT_FRAME=1,
	BIN_FRAME=2,
	CONNEXION_CLOSE=8,
	PING_FRAME=9,
	PONG_FRAME=0xA
} ws_opcode_t;

typedef struct ws_frame_st 
{
	ws_opcode_t		opcode;
	unsigned char	header[14];
	XBOX::VSize		buf_len;
	int				masked;
	unsigned char	msk[4];
	sLONG8			len;
} ws_frame_t;

	ws_frame_t						fFrame;

	XBOX::VError					ValidateHeader(const XBOX::VHTTPHeader& hdr,XBOX::VString & outKey);
	XBOX::VError					SendHandshake(IHTTPResponse* resp, const XBOX::VString & key);

	XBOX::VError					ReadHeader(bool& found);


};

#endif


#endif