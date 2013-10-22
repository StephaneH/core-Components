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



const XBOX::VString K_UPGRADE("Upgrade");
const XBOX::VString K_WEBSOCKET("websocket");
const XBOX::VString K_WBSCK_KEY("Sec-WebSocket-Key");
const XBOX::VString K_WBSCK_VERS("Sec-WebSocket-Version");
const XBOX::VString K_WBSCK_ACCEPT("Sec-WebSocket-Accept");
const XBOX::VString K_WBSCK_EXT("Sec-WebSocket-Extensions");

const XBOX::VString K_CONNECTION("Connection:");


// buffer should be char*, u16 should be unsigned 16
#define htons16(buffer,u16)	{	\
	*(buffer) = u16 / 256;		\
	*((buffer)+1) = u16 % 256;}

#define htons64(buffer,u64)	{				\
		VSize high = (u64 / (1 << 32));		\
		VSize highH = high / (1<<16);		\
		htons16(buffer,highH);				\
		VSize highL = high % (1<<16);		\
		htons16(buffer+2,highL);			\
		VSize low = (u64 % (1 << 32));		\
		VSize lowH = low / (1<<16);			\
		htons16(buffer+4,lowH);				\
		VSize lowL = low % (1<<16);			\
		htons16(buffer+5,lowL);	}


XBOX::VError IHTTPWebsocketHandler::ReadBytes(void* inData, XBOX::VSize& ioLength, bool inExactly)
{
	XBOX::VError	err;
	if (!fEndpt)
	{
		xbox_assert((fEndpt != NULL));
		return XBOX::VE_INVALID_PARAMETER;
	}

	if (ioLength < 0)
	{
		err = XBOX::VE_INVALID_PARAMETER;
	}
	else
	{
		uLONG					length;
		XBOX::StErrorContextInstaller errorContext( true /*we keep errors*/, true /*from now on, popups are forbidden*/);

		length = ioLength;
		err = fEndpt->Read(inData,&length);
		if (err == XBOX::VE_SRVR_RESOURCE_TEMPORARILY_UNAVAILABLE)
		{
			if (length)
			{
				XBOX::DebugMsg("NON-NULL LENGTH SHOULD NOT HAPPEN!!!!!\n");
			}
			else
			{
				ioLength = 0;
				err = XBOX::VE_OK;//'cos non blocking
			}
		}
		else
		{
			if (!err)
			{
				// when exactly, the TIMEOUT_ERR is returned if the exact length is not get
				if (inExactly && (length < ioLength))
				{
					uLONG	exactLength;
					exactLength = ioLength - length;
					err = fEndpt->ReadExactly((char*)inData+length,exactLength,2000);
				}
				else
				{
					ioLength = length;
				}
			}
		}
	}

	if (err)
	{
		XBOX::DebugMsg("ReadBytes ERR=%d\n",err);
		XBOX::DebugMsg("ERRCODE_FROM_VERROR=%d\n",ERRCODE_FROM_VERROR(err));
		XBOX::DebugMsg("NATIVE_ERRCODE_FROM_VERROR=%d\n",( (XBOX::VNativeError)(err & 0xFFFFFFFF) ));
		//xbox_assert(!l_err);
	}

	return err;
}

#define K_WEBSOCKET_NB_MAX_TRIES		(10)
XBOX::VError IHTTPWebsocketHandler::WriteBytes(const void* inData, XBOX::VSize inLength)
{
	int				nbTries;
	sLONG			len;
	XBOX::VError	err;
	char*			tmpData = (char*)inData;

	err = XBOX::VE_OK;
	nbTries = K_WEBSOCKET_NB_MAX_TRIES;
	if (!fEndpt)
	{
		xbox_assert((fEndpt != NULL));
		return XBOX::VE_INVALID_PARAMETER;
	}

	while(!err && inLength)
	{
		XBOX::StErrorContextInstaller errorContext( true /*we keep errors*/, true /*from now on, popups are forbidden*/);

		len = ( inLength > 4096 ? 4096 : (sLONG)inLength );
		//DebugMsg("WriteBytes trying to write %d bytes at %x\n",l_len,l_tmp);
		err = fEndpt->Write(tmpData,(uLONG*)&len);

		if (err == XBOX::VE_SRVR_RESOURCE_TEMPORARILY_UNAVAILABLE)
		{
			nbTries--;
			if (!nbTries)
			{
				err = XBOX::VE_SRVR_WRITE_FAILED;
			}
			else
			{
				err = XBOX::VE_OK;
				XBOX::VTask::Sleep(100);
				continue;
			}
		}

		if (!err)
		{
			nbTries = K_WEBSOCKET_NB_MAX_TRIES;
			inLength -= len;
			tmpData += len;
		}
		else
		{
			XBOX::DebugMsg("WriteBytes ERR=%d tries left%d\n",err,nbTries);
			XBOX::DebugMsg("ERRCODE_FROM_VERROR=%d\n",ERRCODE_FROM_VERROR(err));
		}
	}
	return err;

}
void IHTTPWebsocketHandler::CreateAcceptString(const XBOX::VString& key, XBOX::VString& outAcceptString)
{
	XBOX::SHA1		sha1sum;
	outAcceptString = key;
	outAcceptString.AppendCString("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
	XBOX::VStringConvertBuffer	buffer( outAcceptString, XBOX::VTC_UTF_8);

	XBOX::VChecksumSHA1::GetChecksumFromBytes(buffer.GetCPointer(),buffer.GetLength(),sha1sum);
	XBOX::VChecksumSHA1::EncodeChecksumBase64(sha1sum,outAcceptString);
}



VHTTPWebsocketClientHandler::WsState VHTTPWebsocketClientHandler::GetState() const
{
	return fState;
}
VHTTPWebsocketClientHandler::VHTTPWebsocketClientHandler()
{
	fState = CLOSED_STATE;
	fEndpt = NULL;
	fBytesToRead = XBOX_LONG8(0);
	fCurt = XBOX_LONG8(0);
	fOutputIsTerminated = true;
}

VHTTPWebsocketClientHandler::~VHTTPWebsocketClientHandler()
{
	testAssert(fEndpt == NULL);
	fState = (WsState)-1;
}

XBOX::VError VHTTPWebsocketClientHandler::SendHandshake(const VString& inHost, const VString& inPath)
{
	XBOX::VError	err = VE_OK;
	
	VString			message("GET ");
	message += inPath;
	message += CVSTR(" HTTP/1.1\r\n");
	message += K_UPGRADE;
	message += CVSTR(": ");
	message += K_WEBSOCKET;
	message += "\r\n";
	message += "Connection: Upgrade\r\n";
	message += "Host: ";
	message += inHost;
	message += "\r\n";
	message += "Origin: null\r\n";	message += "Pragma: no-cache\r\n";
	message += "Cache-Control: no-cache\r\n";
	message += K_WBSCK_KEY;
	message += ": ";
	message += fKey;
	message += "\r\n";
	message += K_WBSCK_VERS;
	message += ": 13\r\n\r\n";

	VStringConvertBuffer	buffer( message, VTC_UTF_8);
	uLONG					len = (sLONG)buffer.GetLength();
	err = fEndpt->WriteWithTimeout((void*)buffer.GetCPointer(),&len,1000);
	return err;
}

XBOX::VError VHTTPWebsocketClientHandler::TreatHandshakeResponse(sLONG& ioLen)
{
	XBOX::VError	err = VE_INVALID_PARAMETER;
	bool			upgradeToWS = false;
	bool			connectionUpgrade = false;
	bool			acceptOK = false;
	sBYTE			*inData = fInBuffer;

	const sBYTE		K_HTTP_EOL[2] = { '\r','\n' };
	for( int idx=0; idx<ioLen; idx++ )
	{
		if (!memcmp(inData+idx,K_HTTP_EOL,2))
		{
			inData[idx] = 0;
			if (strstr(inData,"HTTP") && strstr(inData,"101"))
			{
				err = VE_OK;
			}
			ioLen -= (idx + 2);
			inData += idx + 2;
			break;
		}
	}
	while( (ioLen > 0) && !err )
	{
		if (!memcmp(inData,K_HTTP_EOL,2))
		{
			ioLen -= 2;
			inData += 2;
			break;
		}
		for( int idx=0; idx<ioLen; idx++ )
		{
			if (!memcmp(inData+idx,K_HTTP_EOL,2))
			{
				inData[idx] = 0;
				VString		curtLine((const char*)inData);
				ioLen -= (idx + 2);
				VIndex upgradeIndex = curtLine.Find(K_UPGRADE);
				if ( upgradeIndex == 1 )
				{
					upgradeToWS = curtLine.Find(K_WEBSOCKET,upgradeIndex+1) > 0;
				}
				VIndex connectionIndex = curtLine.Find(K_CONNECTION);
				if ( connectionIndex == 1 )
				{
					connectionUpgrade = curtLine.Find(K_UPGRADE,connectionIndex+1) > 0;
				}
				VIndex acceptIndex = curtLine.Find(K_WBSCK_ACCEPT);
				if ( acceptIndex == 1 )
				{
					VIndex acceptKeyLen = curtLine.GetLength() - K_WBSCK_ACCEPT.GetLength() - 2;
					VString acceptKey;
					curtLine.GetSubString( K_WBSCK_ACCEPT.GetLength() + 3, acceptKeyLen, acceptKey);
					VString	awaitedAcceptKey;
					CreateAcceptString(fKey,awaitedAcceptKey);
					//DebugMsg("VHTTPWebsocketClientHandler::TreatHandshakeResponse '%S' vs '%S'\n",&awaitedAcceptKey,&acceptKey);
					acceptOK = ( awaitedAcceptKey.CompareToString(acceptKey,true) == CR_EQUAL );
				}
				inData += idx + 2;
				break;
			}
		}
	}
	if (!connectionUpgrade || !upgradeToWS || !acceptOK)
	{
		err = VE_INVALID_PARAMETER;
	}
	return err;
}

const char	K_BASE64_ARRAY[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
XBOX::VError VHTTPWebsocketClientHandler::CreateKey()
{
	fKey = CVSTR("");
	for( int idx=0; idx<11; idx++ )
	{
		sLONG rnd = VSystem::Random();
		fKey += K_BASE64_ARRAY[rnd % 64];
		rnd = (rnd / 64) % 64;
		fKey += K_BASE64_ARRAY[rnd];
	}
	fKey += "==";
	//fKey = "M/1qGPeVFygnm55eBj8YZA==";
	return VE_OK;
}

XBOX::VError VHTTPWebsocketClientHandler::ConnectToServer(const XBOX::VURL& inUrl, bool isBlocking)
{
	XBOX::VError		err = VE_OK;
	VString				tmpStr;
	VString				host;
	VString				params;
	VString				query;
	sLONG				portNumber;
	
	if (!testAssert(fState == CLOSED_STATE))
	{
		return VE_INVALID_PARAMETER;	
	}

	inUrl.GetScheme(tmpStr);
	bool isSSL = tmpStr.EqualToString(CVSTR("wss"),true);
	if (!tmpStr.EqualToString(CVSTR("ws"),true) && !isSSL )
	{
		err = VE_INVALID_PARAMETER;
	}
	if (!err)
	{
		inUrl.GetPortNumber( tmpStr, false );
		portNumber = tmpStr.GetLong();
		if (!portNumber)
		{
			portNumber = 80;
		}
		inUrl.GetRelativeURL(tmpStr,false);
		inUrl.GetHostName( host, false );

		inUrl.GetPath( tmpStr );
		inUrl.GetRelativePath( tmpStr );
		inUrl.GetParameters( params, false );
		inUrl.GetQuery( query, false );
	}
	if (!err)
	{
		fState = CONNECTING_STATE;
		XBOX::XTCPSock*		tcpSck = XTCPSock::NewClientConnectedSock(host,portNumber,1000);
		if (tcpSck)
		{
			if (isSSL)
			{
				err = tcpSck->PromoteToSSL();
			}
			if (!err)
			{
				if (query.GetLength() > 0)
				{
					params += "?";
					params += query;
				}
				tmpStr += "/";
				tmpStr += params;
				VString		urlData = "/";
				urlData += tmpStr;
				VString hostPort(host);
				hostPort += ":";
				hostPort.AppendULong8(portNumber);
				err = ConnectToServer(tcpSck,hostPort,urlData);
			}
		}
		else
		{
			err = VE_INVALID_PARAMETER;
		}
	}
	if (err)
	{
		if (fEndpt)
		{
			Close();
		}
		fState = CLOSED_STATE;
	}
	else
	{
		fState = OPENED_STATE;
		fEndpt->SetIsBlocking(isBlocking);
	}
	return err;
}

XBOX::VError VHTTPWebsocketClientHandler::ConnectToServer(XBOX::XTCPSock* inTcpSck, const VString& inHostPort, const VString& inData)
{
	XBOX::VError		err = VE_INVALID_PARAMETER;

	if (inTcpSck)
	{
		fEndpt = new VTCPEndPoint(inTcpSck);
		err = CreateKey();
		if (!err)
		{
			err = SendHandshake(inHostPort,inData);
		}
		if (testAssert(err == VE_OK))
		{
			sLONG	len = K_WEBSOCKET_HANDLER_MAX_SIZE;
			err = fEndpt->Read((void*)fInBuffer,(uLONG*)&len);
			if (testAssert(err == VE_OK))
			{
				err = TreatHandshakeResponse(len);
				if (testAssert(err == VE_OK))
				{
					if (len > 0)
					{
						xbox_assert(false);
					}
				}
			}
		}
	}
	return err;
}


XBOX::VError VHTTPWebsocketClientHandler::ReadHeader(bool& outFound)
{
	VError			err;

	memset(&fReadFrame,0,sizeof(ws_frame_t));
	outFound = false;
	fReadFrame.buf_len = 2;

	// see if there's frame start
	err = ReadBytes((void *)fReadFrame.header, fReadFrame.buf_len, true);
	if (!err && fReadFrame.buf_len)
	{
		// extensions not handled
		if (fReadFrame.header[0] & 0x70)
		{
			DebugMsg("VHTTPWebsocketClientHandler::ReadHeader RFC6455 EXTENSIONS NOT HANDLED!!!!\n");
			err = VE_INVALID_PARAMETER;
		}
		if (!err)
		{
			fReadFrame.opcode = (ws_opcode_t)(fReadFrame.header[0] & 0x0F);
		
			fReadFrame.masked = (fReadFrame.header[1] & 0x80);
			if (testAssert(fReadFrame.masked == 0))
			{
				fReadFrame.len = (sLONG8)(fReadFrame.header[1] & 0x7F);
				if (fReadFrame.len == 127)
				{
					fReadFrame.buf_len = 8;
					err = ReadBytes((void *)(fReadFrame.header+2), fReadFrame.buf_len, true);
					if ( err || !fReadFrame.buf_len)
					{
						err = VE_STREAM_CANNOT_READ;
					}
					if (!err)
					{
						fReadFrame.len = 0;
						for(int l_i=0;l_i<8;l_i+=2)
						{
							fReadFrame.len = 65536*fReadFrame.len + (256*fReadFrame.header[2+l_i]+fReadFrame.header[3+l_i]);
						}
						DebugMsg("ReadHeader frame.len1=%lld, msk=%d\n",fReadFrame.len,fReadFrame.masked);
					}
				}
				else
				{
					if (fReadFrame.len == 126)
					{
						fReadFrame.buf_len = 2;
						err = ReadBytes((void *)(fReadFrame.header+2), fReadFrame.buf_len, true);
						if ( err || !fReadFrame.buf_len)
						{
							err = VE_STREAM_CANNOT_READ;
						}
						if (!err)
						{
							fReadFrame.len = 256*fReadFrame.header[2]+fReadFrame.header[3];
							DebugMsg("ReadHeader frame.len2=%d, msk=%d\n",fReadFrame.len,fReadFrame.masked);
						}
					}
					else
					{
						DebugMsg("ReadHeader frame.len3=%d, msk=%d\n",fReadFrame.len,fReadFrame.masked);
					}
				}
			}
			else
			{
				DebugMsg("VHTTPWebsocketClientHandler::ReadHeader header shoud not be masked!!!!\n");
				err = VE_INVALID_PARAMETER;
			}
			if (!err)
			{
				outFound = true;
			}
		}
	}

	return err;
}

XBOX::VError VHTTPWebsocketClientHandler::ReadMessage( void* inData, VSize& ioLength, bool& outIsTerminated )
{
	bool			headerFound;
	VError			err;

	if (!testAssert(fState == OPENED_STATE))
	{
		return  VE_INVALID_PARAMETER;
	}

	if (ioLength < 0)
	{
		return  VE_INVALID_PARAMETER;
	}

	if (fEndpt == NULL)
	{
		return VE_INVALID_PARAMETER;
	}

	err = VE_OK;

	if (!fBytesToRead)
	{
		err = ReadHeader(headerFound);
		if (err)
		{
			DebugMsg("ERR2\n");
		}
		if (!err && !headerFound)
		{
			ioLength = 0;
		}

		// not fragmented ?
		if (!err && ioLength && (fReadFrame.header[0] & 0x80))
		{
			fBytesToRead = fReadFrame.len;
			fCurt = XBOX_LONG8(0);
			switch(fReadFrame.opcode)
			{
				case CONTINUATION_FRAME:
					DebugMsg("ERR3\n");
					err = VE_UNIMPLEMENTED;
					break;
				case CONNEXION_CLOSE:
					if (fReadFrame.len >= 126)
					{
						DebugMsg("ERR4\n");
						err = VE_INVALID_PARAMETER;
					}
					else
					{
						err = VE_STREAM_EOF;
					}
					break;
				case TEXT_FRAME:
				case BIN_FRAME:
					break;
				default:
					break;
			}
		}
		else
		{
			if (!err && ioLength )
			{
				DebugMsg("Fragmentation not handled ERR6\n");
				err = VE_INVALID_PARAMETER;
			}
		}
	}
	//DebugMsg("ReadMessage req_len=%d ToRead=%lld\n",*ioLength,fBytesToRead);
	if (!err && fBytesToRead)
	{
//printf("...... bef ReadMessage req_len=%d ToRead=%lld\n",*ioLength,fBytesToRead);
		ioLength = ( ioLength >= fBytesToRead ? ((VSize)fBytesToRead) : ioLength);
//printf("...... aft ReadMessage req_len=%d ToRead=%lld\n",*ioLength,fBytesToRead);
		err = ReadBytes(inData,ioLength,true);
		if (err)
		{
			DebugMsg("ERR1\n");
		}
		else
		{
			fBytesToRead -= ioLength;
//printf("...... ....... aft ReadMessage read_len=%d ToRead=%lld\n",*ioLength,fBytesToRead);
			/*if (fFrame.masked)
			{
				for(VSize l_i=0; l_i<*ioLength; l_i++)
				{
					((unsigned char *)inData)[l_i] ^= fFrame.msk[fCurt % 4];
					fCurt++;
				}
			}*/

			outIsTerminated = !(fBytesToRead);
		}
	}
	if (err)
	{
		/*fEndpt = NULL;*/
		Close();
	}
	return err;
}

XBOX::VError VHTTPWebsocketClientHandler::WriteHeader(VSize inLength)
{
	VError			err;

	memset(&fFrame,0,sizeof(ws_frame_t));
	fFrame.buf_len = 2;

	if (fOutputIsTerminated)
	{
		fFrame.header[0] = 0x81;
	}
	else
	{
		fFrame.header[0] = 0x01;
	}
	if (inLength <= 125)
	{
		fFrame.header[1] = inLength;
		fFrame.buf_len = 2;
	}
	else
	{
		if (inLength < 65536)
		{
			fFrame.header[1] = 126;
			htons16(fFrame.header+2,inLength);
			fFrame.buf_len = 4;
		}
		else
		{
			fFrame.header[1] = 127;
			fFrame.buf_len = 10;
			xbox_assert(false);
			// TBC GH
		}
	}
	//set mask indicator
	fFrame.header[1] |= 0x80;

	memcpy( fFrame.header+fFrame.buf_len, fMask, 4);
	fFrame.buf_len += 4;

	err = WriteBytes((void *)fFrame.header, fFrame.buf_len);

	return err;
}

XBOX::VError VHTTPWebsocketClientHandler::WriteMessage( const void* inData, VSize inLength, bool inIsTerminated  )
{
	XBOX::VError	err;
	
	if (!testAssert(fState == OPENED_STATE))
	{
		return  VE_INVALID_PARAMETER;
	}

	if (inLength < 0)
	{
		return  VE_INVALID_PARAMETER;
	}

	if (fEndpt == NULL)
	{
		return VE_INVALID_PARAMETER;
	}

	err = VE_OK;

	sLONG	randomMask = VSystem::Random();
	memcpy( fMask, &randomMask, 4 );
	fOutputIsTerminated = inIsTerminated;

	{

		err = WriteHeader(inLength);
		if (err)
		{
			DebugMsg("VHTTPWebsocketClientHandler::WriteHeader ERR2\n");
		}
		else
		{	
			// mask bytes before sending
			sLONG	nbLoops;
			VIndex	curtIdx;
			curtIdx = 0;
			nbLoops = inLength / K_WEBSOCKET_HANDLER_MAX_SIZE;
			while( !err && nbLoops )
			{
				for(VSize idx=0; idx<K_WEBSOCKET_HANDLER_MAX_SIZE; idx++)
				{
					fOutBuffer[idx] = ((unsigned char *)inData)[curtIdx] ^ fMask[curtIdx % 4];
					curtIdx++;
				}
				err = WriteBytes( fOutBuffer, K_WEBSOCKET_HANDLER_MAX_SIZE);
				nbLoops--;
			}
			if (!err)
			{
				VIndex	remainder = inLength % K_WEBSOCKET_HANDLER_MAX_SIZE;
				for(VSize idx=0; idx<remainder; idx++)
				{
					fOutBuffer[idx] = ((unsigned char *)inData)[curtIdx] ^ fMask[curtIdx % 4];
					curtIdx++;
				}
				err = WriteBytes( fOutBuffer, remainder);
			}
		}
	}

	if (err)
	{
		Close();
	}
	return err;
}



XBOX::VError VHTTPWebsocketClientHandler::Close()
{
	if (!fEndpt)
	{
		return VE_INVALID_PARAMETER;
	}
	fEndpt->Close();
	fState = CLOSED_STATE;
	ReleaseRefCountable(&fEndpt);
	return VE_OK;
}

VHTTPWebsocketServerHandler::VHTTPWebsocketServerHandler()
{
	fDetachEndPoint = false;
	fEndpt = NULL;
	fBytesToRead = XBOX_LONG8(0);
	fCurt = XBOX_LONG8(0);
	fOutputIsTerminated = true;
	fState = CLOSED_STATE;
}


VHTTPWebsocketServerHandler::~VHTTPWebsocketServerHandler()
{
	testAssert(fEndpt == NULL);
	fState = (WsState)-1;
}

VHTTPWebsocketServerHandler::WsState VHTTPWebsocketServerHandler::GetState() const
{
	return fState;
}


XBOX::VError VHTTPWebsocketServerHandler::ValidateHeader(const XBOX::VHTTPHeader& hdr,XBOX::VString & outKey) 
{
	XBOX::VError		l_err;
	XBOX::VString		l_hdrstr;
	XBOX::VString		l_tmp;

	l_err = VE_OK;
	hdr.ToString(l_hdrstr);

	if ( !hdr.GetHeaderValue(K_UPGRADE,l_tmp) || !l_tmp.EqualToString(K_WEBSOCKET,true) )
	{
		l_err = VE_INVALID_PARAMETER;
	}
	if ( !l_err && ( !hdr.GetHeaderValue(HEADER_CONNECTION,l_tmp) || (l_tmp.Find(K_UPGRADE) < 1 ) ) )
	{
		l_err = VE_INVALID_PARAMETER;
	}
	if ( !l_err && ( !hdr.GetHeaderValue(K_WBSCK_VERS,l_tmp) || (l_tmp.GetWord() < 13) ) )
	{
		l_err = VE_INVALID_PARAMETER;
	}
	if ( !l_err && !hdr.GetHeaderValue(K_WBSCK_KEY,outKey) )
	{
		l_err = VE_INVALID_PARAMETER;
	}	

	return l_err;
}




#define K_ACCEPT_MAX_LEN	(256)
XBOX::VError VHTTPWebsocketServerHandler::SendHandshake(IHTTPResponse* resp,const XBOX::VString & key) 
{
	XBOX::VError	err;
	//XBOX::SHA1		sha1sum;
	XBOX::VString	acceptStr; //ex (from rfc6455)"dGhlIHNhbXBsZSBub25jZQ==");
	
	err = VE_OK;
	/*acceptStr.AppendCString("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
	VStringConvertBuffer	buffer( acceptStr, VTC_UTF_8);

	VChecksumSHA1::GetChecksumFromBytes(buffer.GetCPointer(),buffer.GetLength(),sha1sum);
	VChecksumSHA1::EncodeChecksumBase64(sha1sum,acceptStr);*/
	CreateAcceptString(key,acceptStr);
	resp->SetResponseStatusCode(HTTP_SWITCHING_PROTOCOLS);
	if (!resp->AddResponseHeader(HEADER_CONNECTION,K_UPGRADE))
	{
		err = VE_STREAM_CANNOT_WRITE;
	}
	if (!err && !resp->AddResponseHeader(K_UPGRADE,K_WEBSOCKET))
	{
		err = VE_STREAM_CANNOT_WRITE;
	}
	if (!err && !resp->AddResponseHeader(K_WBSCK_ACCEPT,acceptStr))
	{
		err = VE_STREAM_CANNOT_WRITE;
	}
	resp->AllowCompression(false);
	if (!err)
	{
		err = resp->SendResponseHeader();
	}
	return err;
}




XBOX::VError VHTTPWebsocketServerHandler::ReadHeader(bool& found)
{
	VError			err;

	memset(&fFrame,0,sizeof(ws_frame_t));
	found = false;
	fFrame.buf_len = 2;

	// see if there's frame start
	err = ReadBytes((void *)fFrame.header, fFrame.buf_len, true);

	if (!err && fFrame.buf_len)
	{
		// extensions not handled
		if (fFrame.header[0] & 0x70)
		{
			DebugMsg("VHTTPWebsocketServerHandler::ReadHeader RFC6455 EXTENSIONS NOT HANDLED!!!!\n");
			err = VE_INVALID_PARAMETER;
		}
		if (!err)
		{
			fFrame.opcode = (ws_opcode_t)(fFrame.header[0] & 0x0F);
		
			fFrame.masked = (fFrame.header[1] & 0x80);
			fFrame.len = (sLONG8)(fFrame.header[1] & 0x7F);
			if (fFrame.len == 127)
			{
				fFrame.buf_len = 8;
				err = ReadBytes((void *)(fFrame.header+2), fFrame.buf_len, true);
				if ( err || !fFrame.buf_len)
				{
					err = VE_STREAM_CANNOT_READ;
				}
				if (!err)
				{
					fFrame.len = 0;
					for(int l_i=0;l_i<8;l_i+=2)
					{
						fFrame.len = 65536*fFrame.len + (256*fFrame.header[2+l_i]+fFrame.header[3+l_i]);
					}
					DebugMsg("ReadHeader frame.len1=%lld, msk=%d\n",fFrame.len,fFrame.masked);
				}
			}
			else
			{
				if (fFrame.len == 126)
				{
					fFrame.buf_len = 2;
					err = ReadBytes((void *)(fFrame.header+2), fFrame.buf_len, true);
					if ( err || !fFrame.buf_len)
					{
						err = VE_STREAM_CANNOT_READ;
					}
					if (!err)
					{
						fFrame.len = 256*fFrame.header[2]+fFrame.header[3];
						DebugMsg("ReadHeader frame.len2=%d, msk=%d\n",fFrame.len,fFrame.masked);
					}
				}
				else
				{
					DebugMsg("ReadHeader frame.len3=%d, msk=%d\n",fFrame.len,fFrame.masked);
				}
			}
			if (!err && fFrame.masked)
			{
				fFrame.buf_len = 4;
				err = ReadBytes((void *)fFrame.msk, fFrame.buf_len, true);
				if ( err || !fFrame.buf_len)
				{
					err = VE_STREAM_CANNOT_READ;
				}
			}
			found = true;
		}
	}

	return err;
}

XBOX::VError VHTTPWebsocketServerHandler::ReadMessage( void* inData, VSize& ioLength, bool& outIsTerminated )
{
	bool			headerFound;
	VError			err;

	outIsTerminated = true;

	if (!testAssert(ioLength > 0))
	{
		return  VE_INVALID_PARAMETER;
	}

	if (fEndpt == NULL)
	{
		DebugMsg("VHTTPWebsocketServerHandler::ReadMessage fEndpt==NULL\n");
		return VE_INVALID_PARAMETER;
	}

	err = VE_OK;

	if (!fBytesToRead)
	{
		err = ReadHeader(headerFound);
		if (err)
		{
			DebugMsg("VHTTPWebsocketServerHandler::ReadMessage  ReadHeader pb\n");
		}
		else
		{
			if (!headerFound)
			{
				ioLength = 0;
			}
		}

		// not fragmented ?
		if (!err && (ioLength) && (fFrame.header[0] & 0x80))
		{
			fBytesToRead = fFrame.len;
			fCurt = XBOX_LONG8(0);
			switch(fFrame.opcode)
			{
				case CONTINUATION_FRAME:
					DebugMsg("VHTTPWebsocketServerHandler::ReadMessage ERR3\n");
					err = VE_UNIMPLEMENTED;
					break;
				case CONNEXION_CLOSE:
					if (fFrame.len >= 126)
					{
						DebugMsg("VHTTPWebsocketServerHandler::ReadMessage ERR4\n");
						err = VE_INVALID_PARAMETER;
					}
					else
					{
						DebugMsg("VHTTPWebsocketServerHandler::ReadMessage CONNEXION_CLOSE\n");
						err = VE_STREAM_EOF;
					}
					break;
				case TEXT_FRAME:
				case BIN_FRAME:
					break;
				default:
					break;
			}
		}
		else
		{
			if (!err && ioLength )
			{
				DebugMsg("VHTTPWebsocketServerHandler::ReadMessage Fragmentation not handled ERR6\n");
				err = VE_INVALID_PARAMETER;
			}
		}
	}
	//DebugMsg("ReadMessage req_len=%d ToRead=%lld\n",*ioLength,fBytesToRead);
	if (!err && fBytesToRead)
	{
//printf("...... bef ReadMessage req_len=%d ToRead=%lld\n",*ioLength,fBytesToRead);
		ioLength = ( ioLength >= fBytesToRead ? ((VSize)fBytesToRead) : ioLength);
//printf("...... aft ReadMessage req_len=%d ToRead=%lld\n",*ioLength,fBytesToRead);
		err = ReadBytes(inData,ioLength);
		if (err)
		{
			DebugMsg("VHTTPWebsocketServerHandler::ReadMessage ReadBytes pb\n");
		}
		else
		{
			fBytesToRead -= ioLength;
//printf("...... ....... aft ReadMessage read_len=%d ToRead=%lld\n",*ioLength,fBytesToRead);
			if (fFrame.masked)
			{
				for(VSize l_i=0; l_i<ioLength; l_i++)
				{
					((unsigned char *)inData)[l_i] ^= fFrame.msk[fCurt % 4];
					fCurt++;
				}
			}

			outIsTerminated = !(fBytesToRead);
		}
	}
	if (err)
	{
		Close();
	}
	return err;

}
				

XBOX::VError VHTTPWebsocketServerHandler::WriteMessage( const void* inData, VSize inLength, bool inIsTerminated  )
{
	XBOX::VError	err;
	unsigned char	header[10];

	if (fEndpt == NULL)
	{
		return VE_INVALID_PARAMETER;
	}

	err = VE_OK;
	if (fOutputIsTerminated)
	{
		fOutputIsTerminated = inIsTerminated;
		if (inIsTerminated)
		{
			header[0] = 0x81;
		}
		else
		{
			header[0] = 0x01;
		}
	}
	else
	{
		fOutputIsTerminated = inIsTerminated;
		header[0] = (inIsTerminated ? 0x80 : 0x00 );
	}
	err = ( inLength >= 0 ? VE_OK : VE_INVALID_PARAMETER);
	testAssert( err == VE_OK );
//SEND_COMPLETE_FRAME:
	if (!err)
	{
		if (inLength <= 125)
		{
			header[1] = (unsigned char)inLength;
			err = WriteBytes(header,2);
			if (err)
			{
				DebugMsg("WriteBytes ERR\n");
			}
		}
		else
		{
			if (inLength < 65536)
			{
				header[1] = 126;
				htons16(header+2,inLength);
				err = WriteBytes(header,4);
				if (err)
				{
					DebugMsg("Write3 ERR\n");
				}
			}
			else
			{
				header[1] = 127;
				htons64(header+2,inLength);
				err = WriteBytes(header,10);

			}
		}
//SEND_ALL_DATA:
		if (!err)
		{
			err = WriteBytes(inData,inLength);
			if (err)
			{
				DebugMsg("WriteBytes ERR\n");
			}	
		}
	}
	if (err)
	{
		/*fEndpt = NULL;*/
		Close();
	}

	return err;

}


XBOX::VError VHTTPWebsocketServerHandler::Close()
{
	if (!fEndpt)
	{
		return VE_INVALID_PARAMETER;
	}
	fEndpt->Close();
	if (fDetachEndPoint)
	{
		ReleaseRefCountable(&fEndpt);
	}
	else
	{
		fEndpt = NULL;
	}
	return VE_OK;
}

XBOX::VError VHTTPWebsocketServerHandler::TreatNewConnection( IHTTPResponse* inResponse, bool inDetachEndPoint)
{
	XBOX::VString	l_key;
	VError			l_err;

	if (!testAssert(fEndpt == NULL))
	{
		DebugMsg("TreatNewConnection previous connection still active\n");
		return VE_INVALID_PARAMETER;
	}

	l_err = VE_OK;

	if (inResponse->GetRequestHTTPVersion() != VERSION_1_1)
	{
		DebugMsg("Version HTTP 1.1 required for websockets\n");
		l_err = VE_INVALID_PARAMETER;
	}
	if (!l_err && (inResponse->GetRequestMethod() != HTTP_GET))
	{
		l_err = VE_INVALID_PARAMETER;
	}
	const XBOX::VHTTPHeader &l_rqst_hdr = inResponse->GetRequestHeader();
	if (!l_err)
	{
		l_err = ValidateHeader(l_rqst_hdr,l_key);
	}
	if (!l_err)
	{
		l_err = SendHandshake(inResponse,l_key);
	}

	if (!l_err)
	{
		fDetachEndPoint = inDetachEndPoint;
		if (fDetachEndPoint)
		{
			fEndpt = inResponse->DetachEndPoint();
		}
		else
		{
			fEndpt = inResponse->GetEndPoint();
		}
		if (!fEndpt)
		{
			DebugMsg("TreatNewConnection invalid ENDPOINT\n");
			l_err = VE_INVALID_PARAMETER;
		}
		else
		{
			// make socket non-blocking for the moment (the user should POLL by using ReadMessage)
			fEndpt->SetIsBlocking(false);

			fCurt = XBOX_LONG8(0);
			fOutputIsTerminated = true;
		}
	}
	if (l_err)
	{	
		inResponse->ReplyWithStatusCode( HTTP_BAD_REQUEST);
	}
	return l_err;

}

