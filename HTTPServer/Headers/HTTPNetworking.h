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
#ifndef __HTTP_NETWORKING_INCLUDED__
#define __HTTP_NETWORKING_INCLUDED__

#include "ServerNet/VServerNet.h"

class HTTPNetworkOutputStream : public XBOX::VStream
{
public:
									HTTPNetworkOutputStream (XBOX::VTCPEndPoint& vtcpEndPoint);
	virtual							~HTTPNetworkOutputStream();

protected:
	XBOX::VTCPEndPoint&				fEndPoint;
	char *							fBuffer;
	XBOX::VSize						fBufferMaxSize;
	uLONG							fBufferSize;

	virtual XBOX::VError			DoPutData (const void *inBuffer, XBOX::VSize inNbBytes);
	virtual XBOX::VError			DoFlush();

	virtual XBOX::VError			DoGetData (void *inBuffer, XBOX::VSize *ioCount) { /* A dummy stub to make the class non-abstract. */ return XBOX::VE_OK; }
	virtual XBOX::VError			DoUngetData (const void * inBuffer, XBOX::VSize inNbBytes) { /* A dummy stub to make the class non-abstract. */ return XBOX::VE_OK; }
	virtual sLONG8					DoGetSize() { /* A dummy stub to make the class non-abstract. */ return XBOX::VE_OK; }
	virtual XBOX::VError			DoSetSize (sLONG8 inNewSize) { /* A dummy stub to make the class non-abstract. */ return XBOX::VE_OK; }

private :
	XBOX::VError					_WriteToSocket (char *szchBuffer, uLONG *nioBytes);
};


class HTTPNetworkInputStream : public XBOX::VStream
{
public:
									HTTPNetworkInputStream (XBOX::VTCPEndPoint& vtcpEndPoint);
	virtual							~HTTPNetworkInputStream();

	virtual uLONG					GetBufferSize() { return fBufferSize; }

	/*
		By default, the streaming is non-buffered: it is not possible to rewind or fast-forward the stream.
		By enabling full buffering, the caller gets the ability to rewind the stream, but only to a point where
		full buffering was enabled. Fast forwarding is not implemented, but is very easy to do.
		The full buffering is implemented to support VPicture streaming, which needs to get some data, look
		at it (for versioning) and then rewind back a bit to read again into a structure of correct version.
	*/
	XBOX::VError					EnableFullBuffering();
	XBOX::VError					DisableFullBuffering();

protected:
	XBOX::VTCPEndPoint&				fEndPoint;

	char *							fBuffer;
	XBOX::VSize						fBufferMaxSize;
	uLONG							fBufferSize;

	virtual XBOX::VError			DoGetData (void *inBuffer, XBOX::VSize *ioCount);

	virtual XBOX::VError			DoFlush() { /* A dummy stub to make the class non-abstract. */ return XBOX::VE_OK; }
	virtual XBOX::VError			DoPutData (const void *inBuffer, XBOX::VSize inNbBytes) { /* A dummy stub to make the class non-abstract. */ return XBOX::VE_OK; }
	virtual XBOX::VError			DoUngetData (const void *inBuffer, XBOX::VSize inNbBytes) { /* A dummy stub to make the class non-abstract. */ return XBOX::VE_OK; }
	virtual sLONG8					DoGetSize() { return LLONG_MAX;/*XBOX::VE_OK;*/ }
	virtual XBOX::VError			DoSetSize (sLONG8 inNewSize) { /* A dummy stub to make the class non-abstract. */ return XBOX::VE_OK; }

private :
	bool							fUseFullBuffering;
	sLONG8							fFullBufferOffset;
	XBOX::VMemoryBuffer<>			fMemoryBuffer;
};


#endif // __HTTP_NETWORKING_INCLUDED__
