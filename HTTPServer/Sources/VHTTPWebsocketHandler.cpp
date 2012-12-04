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
const XBOX::VString K_WBSCK_AGT("Sec-WebSocket-Accept");


VHTTPWebsocketHandler::VHTTPWebsocketHandler()
{
	//_IsClosed = true;
	fEndpt = NULL;
	//_ReadInProgress = false;
	fBytesToRead = XBOX_LONG8(0);
	fCurt = XBOX_LONG8(0);
	fOutputIsTerminated = true;
	fState = CLOSED_STATE;
}


VHTTPWebsocketHandler::~VHTTPWebsocketHandler()
{
}
VHTTPWebsocketHandler::WsState_t VHTTPWebsocketHandler::GetState() const
{
	return fState;
}


XBOX::VError VHTTPWebsocketHandler::ValidateHeader(const XBOX::VHTTPHeader& hdr,XBOX::VString & outKey) 
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
XBOX::VError VHTTPWebsocketHandler::SendHandshake(IHTTPResponse* resp,const XBOX::VString & key) 
{
	XBOX::VError	l_err;
	XBOX::SHA1		l_sha1sum;
	char			l_tmp[K_ACCEPT_MAX_LEN];
	XBOX::VString	l_accept(key); //ex (from rfc6455)"dGhlIHNhbXBsZSBub25jZQ==");
	
	l_err = VE_OK;
	l_accept.AppendCString("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
	l_accept.ToCString(l_tmp,K_ACCEPT_MAX_LEN);

	VChecksumSHA1::GetChecksumFromBytes(l_tmp,l_accept.GetLength(),l_sha1sum);
	VChecksumSHA1::EncodeChecksumBase64(l_sha1sum,l_accept);
	resp->SetResponseStatusCode(HTTP_SWITCHING_PROTOCOLS);
	if (!resp->AddResponseHeader(HEADER_CONNECTION,K_UPGRADE))
	{
		l_err = VE_STREAM_CANNOT_WRITE;
	}
	if (!l_err && !resp->AddResponseHeader(K_UPGRADE,K_WEBSOCKET))
	{
		l_err = VE_STREAM_CANNOT_WRITE;
	}
	if (!l_err && !resp->AddResponseHeader(K_WBSCK_AGT,l_accept))
	{
		l_err = VE_STREAM_CANNOT_WRITE;
	}
	resp->AllowCompression(false);
	if (!l_err)
	{
		l_err = resp->SendResponseHeader();
	}
	return l_err;
}




XBOX::VError VHTTPWebsocketHandler::ReadHeader(bool* found)
{
	VError			l_err;

	memset(&fFrame,0,sizeof(ws_frame_t));
	*found = false;
	fFrame.buf_len = 2;
//printf("ReadHeader called\n");

	l_err = ReadBytes((void *)fFrame.header, &fFrame.buf_len);
	if (!l_err && fFrame.buf_len)
	{
		// extensions not handled
		if (fFrame.header[0] & 0x70)
		{
			DebugMsg("VHTTPWebsocketHandler::ReadHeader RFC6455 EXTENSIONS NOT HANDLED!!!!\n");
			l_err = VE_INVALID_PARAMETER;
		}
		if (!l_err)
		{
			fFrame.opcode = (ws_opcode_t)(fFrame.header[0] & 0x0F);
		
			fFrame.masked = (fFrame.header[1] & 0x80);
			fFrame.len = (sLONG8)(fFrame.header[1] & 0x7F);
			if (fFrame.len == 127)
			{
				fFrame.buf_len = 8;
				l_err = ReadBytes((void *)(fFrame.header+2), &fFrame.buf_len);
				if ( !l_err && (fFrame.buf_len != 8) )
				{
					l_err = VE_STREAM_CANNOT_READ;
				}
				if (!l_err)
				{
					fFrame.len = 0;
					for(int l_i=0;l_i<8;l_i+=2)
					{
						fFrame.len = 65536*fFrame.len + (256*fFrame.header[2+l_i]+fFrame.header[3+l_i]);
					}
					DebugMsg("ReadHeader frame.len=%lld\n",fFrame.len);
				}
			}
			else
			{
				if (fFrame.len == 126)
				{
					fFrame.buf_len = 2;
					l_err = ReadBytes((void *)(fFrame.header+2), &fFrame.buf_len);
					if ( !l_err && (fFrame.buf_len != 2) )
					{
						l_err = VE_STREAM_CANNOT_READ;
					}
					if (!l_err)
					{
						fFrame.len = 256*fFrame.header[2]+fFrame.header[3];
						DebugMsg("ReadHeader frame.len=%d\n",fFrame.len);
					}
				}
				else
				{
					DebugMsg("ReadHeader frame.len=%d\n",fFrame.len);
				}
			}
			if (!l_err && fFrame.masked)
			{
				fFrame.buf_len = 4;
				l_err = ReadBytes((void *)fFrame.msk, &fFrame.buf_len);
				if ( !l_err && (fFrame.buf_len != 4) )
				{
					l_err = VE_STREAM_CANNOT_READ;
				}
			}
			*found = true;
		}
	}
	return l_err;
}


XBOX::VError VHTTPWebsocketHandler::ReadBytes(void* inData, uLONG* ioLength)
{
	XBOX::VError	l_err;

	l_err = fEndpt->Read(inData,ioLength);

	if (l_err == VE_SRVR_RESOURCE_TEMPORARILY_UNAVAILABLE)
	{
		if (*ioLength)
		{
			DebugMsg("NON-NULL LENGTH SHOULD NOT HAPPEN!!!!!\n");
		}
		l_err = VE_OK;//'cos non blocking
	}

	if (l_err)
	{
		DebugMsg("ReadBytes ERR=%d\n",l_err);
		DebugMsg("ERRCODE_FROM_VERROR=%d\n",ERRCODE_FROM_VERROR(l_err));
		DebugMsg("NATIVE_ERRCODE_FROM_VERROR=%d\n",NATIVE_ERRCODE_FROM_VERROR(l_err));
		//xbox_assert(!l_err);
	}

	return l_err;
}

XBOX::VError VHTTPWebsocketHandler::ReadMessage( void* inData, uLONG* ioLength, bool* outIsTerminated )
{

	bool			l_header_found;
	VError			l_err;

	if (!fEndpt)
	{
		xbox_assert((fEndpt != NULL));
		return VE_INVALID_PARAMETER;
	}

	l_err = VE_OK;

	if (!fBytesToRead)
	{
		l_err = ReadHeader(&l_header_found);
		if (l_err)
		{
			DebugMsg("ERR2\n");
		}
		if (!l_err && !l_header_found)
		{
			*ioLength = 0;
		}

		// not fragmented ?
		if (!l_err && (*ioLength) && (fFrame.header[0] & 0x80))
		{
			fBytesToRead = fFrame.len;
			fCurt = XBOX_LONG8(0);
			switch(fFrame.opcode)
			{
				case CONTINUATION_FRAME:
					DebugMsg("ERR3\n");
					l_err = VE_UNIMPLEMENTED;
					break;
				case CONNEXION_CLOSE:
					if (fFrame.len >= 126)
					{
						DebugMsg("ERR4\n");
						l_err = VE_INVALID_PARAMETER;
					}
					else
					{
						l_err = VE_STREAM_EOF;
					/*// close the socket
					goto NOK;
					while (!l_err && (fBytesToRead > 0))
					{
						length = (uLONG)( *ioLength >=fBytesToRead ? fBytesToRead : *ioLength);
						l_err = ReadBytes(inData,&length);
						if (l_err)
						{
							DebugMsg("ERR 5\n");
						}
						else
						{
							fBytesToRead -= length;
							*ioLength = length;
						}
					}*/
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
			if (!l_err && *(ioLength) )
			{
				DebugMsg("Fragmentation not handled ERR6\n");
			}
		}
	}
	//DebugMsg("ReadMessage req_len=%d ToRead=%lld\n",*ioLength,fBytesToRead);
	if (!l_err && fBytesToRead)
	{
//printf("...... bef ReadMessage req_len=%d ToRead=%lld\n",*ioLength,fBytesToRead);
		*ioLength = ( *ioLength >=fBytesToRead ? ((uLONG)fBytesToRead) : *ioLength);
//printf("...... aft ReadMessage req_len=%d ToRead=%lld\n",*ioLength,fBytesToRead);
		l_err = ReadBytes(inData,ioLength);
		if (l_err)
		{
			DebugMsg("ERR1\n");
		}
		else
		{
			fBytesToRead -= *ioLength;
//printf("...... ....... aft ReadMessage read_len=%d ToRead=%lld\n",*ioLength,fBytesToRead);
			if (fFrame.masked)
			{
				for(uLONG l_i=0; l_i<*ioLength; l_i++)
				{
					((unsigned char *)inData)[l_i] ^= fFrame.msk[fCurt % 4];
					fCurt++;
				}
			}

			*outIsTerminated = !(fBytesToRead);
		}
	}
	if (l_err)
	{
		/*fEndpt = NULL;*/
		Close();
	}
	return l_err;

}


//VCriticalSection TBD
#define K_NB_MAX_TRIES		(10)
XBOX::VError VHTTPWebsocketHandler::WriteBytes(const void* inData, VSize inLength)
{
	int				l_tries;
	uLONG			l_len;
	XBOX::VError	l_err;
	char*			l_tmp = (char*)inData;

	l_err = VE_OK;
	l_tries = K_NB_MAX_TRIES;
	if (!fEndpt)
	{
		xbox_assert((fEndpt != NULL));
		return VE_INVALID_PARAMETER;
	}

	while(!l_err && inLength)
	{
		l_len = ( inLength > 4096 ? 4096 : (uLONG)inLength );
		//DebugMsg("WriteBytes trying to write %d bytes at %x\n",l_len,l_tmp);
		l_err = fEndpt->Write(l_tmp,&l_len);

		if (l_err == VE_SRVR_RESOURCE_TEMPORARILY_UNAVAILABLE)
		{
			l_tries--;
			if (!l_tries)
			{
				l_err = VE_SRVR_WRITE_FAILED;
			}
			else
			{
				l_err = VE_OK;
				VTask::Sleep(100);
				continue;
			}
		}

		if (!l_err)
		{
			l_tries = K_NB_MAX_TRIES;
			inLength -= l_len;
			l_tmp += l_len;
		}
		else
		{
			DebugMsg("WriteBytes ERR=%d tries left%d\n",l_err,l_tries);
			DebugMsg("ERRCODE_FROM_VERROR=%d\n",ERRCODE_FROM_VERROR(l_err));
		}
	}
	return l_err;

}

XBOX::VError VHTTPWebsocketHandler::WriteMessage( const void* inData, VSize inLength, bool inIsTerminated  )
{
	XBOX::VError	l_err;
	unsigned char	l_header[10];

	l_err = VE_OK;
	if (fOutputIsTerminated)
	{
		fOutputIsTerminated = inIsTerminated;
		if (inIsTerminated)
		{
			l_header[0] = 0x81;
		}
		else
		{
			l_header[0] = 0x01;
		}
	}
	else
	{
		fOutputIsTerminated = inIsTerminated;
		l_header[0] = (inIsTerminated ? 0x80 : 0x00 );
	}

//SEND_COMPLETE_FRAME:
	if (!l_err)
	{
		if (inLength <= 125)
		{
			l_header[1] = (unsigned char)inLength;
			l_err = WriteBytes(l_header,2);
			if (l_err)
			{
				DebugMsg("WriteBytes ERR\n");
			}
		}
		else
		{
			if (inLength < 65536)
			{
				l_header[1] = 126;
				l_header[2] = (uLONG)inLength / 256;
				l_header[3] = (uLONG)inLength % 256;
				l_err = WriteBytes(l_header,4);
				if (l_err)
				{
					DebugMsg("Write3 ERR\n");
				}
			}
			else
			{
				DebugMsg("WriteMessage ERR\n");
				l_err = VE_INVALID_PARAMETER;
			}
		}
//SEND_ALL_DATA:
		if (!l_err)
		{
			l_err = WriteBytes(inData,inLength);
			if (l_err)
			{
				DebugMsg("WriteBytes ERR\n");
			}	
		}
	}
	if (l_err)
	{
		/*fEndpt = NULL;*/
		Close();
	}

	return l_err;

}


XBOX::VError VHTTPWebsocketHandler::Close()
{
	if (!fEndpt)
	{
		return VE_INVALID_PARAMETER;
	}
	fEndpt->Close();
	fEndpt = NULL;
	return VE_OK;
}

XBOX::VError VHTTPWebsocketHandler::TreatNewConnection( IHTTPResponse* inResponse)
{
	XBOX::VString	l_key;
	VError			l_err;

	if (fEndpt)
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
		fEndpt = inResponse->GetEndPoint();
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


