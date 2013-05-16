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
	testAssert(fEndpt == NULL);
	fState = (WsState_t)-1;
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
	VError			err;

	memset(&fFrame,0,sizeof(ws_frame_t));
	*found = false;
	fFrame.buf_len = 2;

	// see if there's frame start
	err = ReadBytes((void *)fFrame.header, &fFrame.buf_len, true);
	if (!err && fFrame.buf_len)
	{
		// extensions not handled
		if (fFrame.header[0] & 0x70)
		{
			DebugMsg("VHTTPWebsocketHandler::ReadHeader RFC6455 EXTENSIONS NOT HANDLED!!!!\n");
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
				err = ReadBytes((void *)(fFrame.header+2), &fFrame.buf_len, true);
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
					err = ReadBytes((void *)(fFrame.header+2), &fFrame.buf_len, true);
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
				err = ReadBytes((void *)fFrame.msk, &fFrame.buf_len, true);
				if ( err || !fFrame.buf_len)
				{
					err = VE_STREAM_CANNOT_READ;
				}
			}
			*found = true;
		}
	}

	return err;
}


// when inExactly set to true, the nb of read bytes should be either 0 (nothing found) or exactly what is requested,
//   otherwise an error is returned
XBOX::VError VHTTPWebsocketHandler::ReadBytes(void* inData, VSize* ioLength, bool inExactly)
{
	XBOX::VError	err;
	if (!fEndpt)
	{
		xbox_assert((fEndpt != NULL));
		return VE_INVALID_PARAMETER;
	}

	if (*ioLength < 0)
	{
		err = VE_INVALID_PARAMETER;
	}
	else
	{
		uLONG					length;
		StErrorContextInstaller errorContext( true /*we keep errors*/, true /*from now on, popups are forbidden*/);

		length = *ioLength;
		err = fEndpt->Read(inData,&length);
		if (err == VE_SRVR_RESOURCE_TEMPORARILY_UNAVAILABLE)
		{
			if (length)
			{
				DebugMsg("NON-NULL LENGTH SHOULD NOT HAPPEN!!!!!\n");
			}
			else
			{
				*ioLength = 0;
				err = VE_OK;//'cos non blocking
			}
		}
		else
		{
			if (!err)
			{
				// when exactly, the TIMEOUT_ERR is returned if the exact length is not get
				if (inExactly && (length < *ioLength))
				{
					uLONG	exactLength;
					exactLength = *ioLength - length;
					err = fEndpt->ReadExactly((char*)inData+length,exactLength,2000);
				}
				else
				{
					*ioLength = length;
				}
			}
		}
	}

	if (err)
	{
		DebugMsg("ReadBytes ERR=%d\n",err);
		DebugMsg("ERRCODE_FROM_VERROR=%d\n",ERRCODE_FROM_VERROR(err));
		DebugMsg("NATIVE_ERRCODE_FROM_VERROR=%d\n",NATIVE_ERRCODE_FROM_VERROR(err));
		//xbox_assert(!l_err);
	}

	return err;
}

XBOX::VError VHTTPWebsocketHandler::ReadMessage( void* inData, VSize* ioLength, bool* outIsTerminated )
{

	bool			l_header_found;
	VError			l_err;

	if (*ioLength < 0)
	{
		return  VE_INVALID_PARAMETER;
	}

	if (fEndpt == NULL)
	{
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
				l_err = VE_INVALID_PARAMETER;
			}
		}
	}
	//DebugMsg("ReadMessage req_len=%d ToRead=%lld\n",*ioLength,fBytesToRead);
	if (!l_err && fBytesToRead)
	{
//printf("...... bef ReadMessage req_len=%d ToRead=%lld\n",*ioLength,fBytesToRead);
		*ioLength = ( *ioLength >= fBytesToRead ? ((VSize)fBytesToRead) : *ioLength);
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
				for(VSize l_i=0; l_i<*ioLength; l_i++)
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
	int				nbTries;
	sLONG			len;
	XBOX::VError	err;
	char*			tmpData = (char*)inData;

	err = VE_OK;
	nbTries = K_NB_MAX_TRIES;
	if (!fEndpt)
	{
		xbox_assert((fEndpt != NULL));
		return VE_INVALID_PARAMETER;
	}

	while(!err && inLength)
	{
		StErrorContextInstaller errorContext( true /*we keep errors*/, true /*from now on, popups are forbidden*/);

		len = ( inLength > 4096 ? 4096 : (sLONG)inLength );
		//DebugMsg("WriteBytes trying to write %d bytes at %x\n",l_len,l_tmp);
		err = fEndpt->Write(tmpData,(uLONG*)&len);

		if (err == VE_SRVR_RESOURCE_TEMPORARILY_UNAVAILABLE)
		{
			nbTries--;
			if (!nbTries)
			{
				err = VE_SRVR_WRITE_FAILED;
			}
			else
			{
				err = VE_OK;
				VTask::Sleep(100);
				continue;
			}
		}

		if (!err)
		{
			nbTries = K_NB_MAX_TRIES;
			inLength -= len;
			tmpData += len;
		}
		else
		{
			DebugMsg("WriteBytes ERR=%d tries left%d\n",err,nbTries);
			DebugMsg("ERRCODE_FROM_VERROR=%d\n",ERRCODE_FROM_VERROR(err));
		}
	}
	return err;

}

XBOX::VError VHTTPWebsocketHandler::WriteMessage( const void* inData, VSize inLength, bool inIsTerminated  )
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
				header[2] = inLength / 256;
				header[3] = inLength % 256;
				err = WriteBytes(header,4);
				if (err)
				{
					DebugMsg("Write3 ERR\n");
				}
			}
			else
			{
				DebugMsg("WriteMessage ERR\n");
				err = VE_INVALID_PARAMETER;
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


