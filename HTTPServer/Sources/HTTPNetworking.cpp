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
#include "HTTPNetworking.h"
#include "HTTPServerCache.h"
#include "VVirtualHost.h"

#include "VTCPServer.h"
#include "ServerNet/VServerNet.h"

#include "VHTTPConnectionListener.h"

USING_TOOLBOX_NAMESPACE


//--------------------------------------------------------------------------------------------------


#if VERSIONWIN
	#define		HTTP_NC_WRITE_WITH_TAIL		true
#else
	#define		HTTP_NC_WRITE_WITH_TAIL		false
#endif

#define HTTP_MAX_BUFFER_LENGTH				1024	 /* NOTE: Figure out the best page size. */
#define HTTP_NETWORK_STREAM_BUFFER_LENGTH	8192	 /* NOTE: Figure out the best page size. */


//--------------------------------------------------------------------------------------------------


HTTPNetworkOutputStream::HTTPNetworkOutputStream (XBOX::VTCPEndPoint& inEndPoint)
: fEndPoint (inEndPoint)
, fBufferMaxSize (HTTP_NETWORK_STREAM_BUFFER_LENGTH)	// Default Value
, fBuffer (NULL)
, fBufferSize (0)
{
	fBuffer = new char[fBufferMaxSize];

	fIsReading = false;
	fIsWriting = true;
	fIsReadOnly = false;
	fIsWriteOnly = true;
	fPosition = 0;
}


HTTPNetworkOutputStream::~HTTPNetworkOutputStream()
{
	if (NULL != fBuffer)
	{
		delete [] fBuffer;
		fBuffer = NULL;
	}
}


XBOX::VError HTTPNetworkOutputStream::DoPutData (const void *inBuffer, XBOX::VSize inNbBytes)
{
	char *			buffer = (char *)inBuffer;
	XBOX::VSize		nbBytes = inNbBytes;
	XBOX::VSize		bytesSent = 0;
	XBOX::VSize		bytesToWriteToBuffer;
	XBOX::VError	error = VE_OK;

	while (nbBytes > 0)
	{
		bytesToWriteToBuffer = fBufferMaxSize - fBufferSize;
		if (nbBytes < bytesToWriteToBuffer)
			bytesToWriteToBuffer = nbBytes;

		::memcpy (fBuffer + fBufferSize, buffer, bytesToWriteToBuffer);
		fBufferSize += (uLONG)bytesToWriteToBuffer;
		bytesSent += bytesToWriteToBuffer;
		buffer = (buffer) + bytesToWriteToBuffer;
		nbBytes -= bytesToWriteToBuffer;
		if (fBufferSize == fBufferMaxSize)
		{
			error = _WriteToSocket (fBuffer, &fBufferSize);
			if (XBOX::VE_OK != error)
				return error;

			fBufferSize = 0;
		}
	}

	return XBOX::VE_OK;
}


XBOX::VError HTTPNetworkOutputStream::DoFlush()
{
	if (0 == fBufferSize)
		return XBOX::VE_OK;

	XBOX::VError error = _WriteToSocket (fBuffer, &fBufferSize);
	if (XBOX::VE_OK != error)
		return error;

	fBufferSize = 0;

	return XBOX::VE_OK;
}


XBOX::VError HTTPNetworkOutputStream::_WriteToSocket (char *inBuffer, uLONG *ioBytes)
{
	XBOX::VError error = XBOX::VE_OK;

	do
	{
		error = fEndPoint.Write (inBuffer, ioBytes, HTTP_NC_WRITE_WITH_TAIL && !fEndPoint.IsSSL());
		if (error == VE_SRVR_RESOURCE_TEMPORARILY_UNAVAILABLE)
			VTask::GetCurrent()->Sleep (1);
	}
	while (error == VE_SRVR_RESOURCE_TEMPORARILY_UNAVAILABLE);

	return error;
}


//--------------------------------------------------------------------------------------------------


HTTPNetworkInputStream::HTTPNetworkInputStream (XBOX::VTCPEndPoint& inEndPoint)
: fEndPoint (inEndPoint)
, fBufferMaxSize (HTTP_MAX_BUFFER_LENGTH)
, fBufferSize (0)
, fUseFullBuffering (false)
, fFullBufferOffset (-1)
{
	fBuffer = new char [fBufferMaxSize];

	fIsReading = true;
	fIsWriting = false;
	fIsReadOnly = true;
	fIsWriteOnly = false;
}


HTTPNetworkInputStream::~HTTPNetworkInputStream()
{
	if (NULL != fBuffer)
	{
		delete [] fBuffer;
		fBuffer = NULL;
	}
}


XBOX::VError HTTPNetworkInputStream::EnableFullBuffering()
{
	fUseFullBuffering = true;
	fFullBufferOffset = fPosition;

	assert (0 == fMemoryBuffer.GetDataSize());

	/* First of all, lets move all the data (if any) from internal read buffer into the full buffer. */
	if (fBufferSize > 0)
	{
		bool bResult = fMemoryBuffer.PutDataAmortized (0, fBuffer, fBufferSize);
		if (!bResult)
			return XBOX::VE_MEMORY_FULL;

		fBufferSize = 0;
	}

	return XBOX::VE_OK;
}


XBOX::VError HTTPNetworkInputStream::DisableFullBuffering()
{
	fUseFullBuffering = false;
	fFullBufferOffset = -1;

	if (fMemoryBuffer.GetDataSize() > 0)
		fMemoryBuffer.Clear();

	return XBOX::VE_OK;
}


XBOX::VError HTTPNetworkInputStream::DoGetData (void *inBuffer, VSize *ioCount)
{
	if  ((NULL == inBuffer) || (NULL == ioCount))
		return VE_HTTP_INVALID_ARGUMENT;

	XBOX::VSize		nTotalBytesRead = 0;
	uLONG			nBytesRead = 0;
	XBOX::VSize		nBytesLeftToRead = *ioCount;

	if (fUseFullBuffering && (fPosition < fFullBufferOffset + fMemoryBuffer.GetDataSize()))
	{
		/* The stream was rewound back. Need to read from the full cache first. */
		XBOX::VSize	nByteCountFromFullBuffer = *ioCount <= fMemoryBuffer. GetDataSize () ? *ioCount : fMemoryBuffer. GetDataSize ();
		bool bResult = fMemoryBuffer.GetData ((VSize)(fPosition - fFullBufferOffset), inBuffer, nByteCountFromFullBuffer, &nByteCountFromFullBuffer);
		if (!bResult)
			return XBOX::VE_STREAM_EOF;

		nTotalBytesRead += nByteCountFromFullBuffer;
		nBytesLeftToRead -= nByteCountFromFullBuffer;

		if (0 == nBytesLeftToRead)
			return XBOX::VE_OK;
	}

	/* First, lets try to read out of internal buffer. */
	if (fBufferSize > 0)
	{
		if (*ioCount <= fBufferSize)
		{
			/* There is enough data in the internal buffer to fulfill the request. */
			memcpy (inBuffer, fBuffer, *ioCount);
			char * szchSource = fBuffer + *ioCount;
			size_t nCount = fBufferSize - *ioCount;
			memmove (fBuffer, szchSource, nCount);
			fBufferSize -= (uLONG)(*ioCount);

			return XBOX::VE_OK;
		}
		else
		{
			/* Read whatever is available in the internal buffer and then read some more from the socket. */
			memcpy (inBuffer, fBuffer, fBufferSize);
			nTotalBytesRead = fBufferSize;
			nBytesLeftToRead = *ioCount - fBufferSize;
			fBufferSize = 0;
		}
	}

	/* And now lets read out of the socket. */
	XBOX::VError error = XBOX::VE_OK;
	while (nTotalBytesRead < *ioCount)
	{
		nBytesRead = (uLONG)(nBytesLeftToRead > fBufferMaxSize ? fBufferMaxSize : nBytesLeftToRead);

		error = fEndPoint.Read (fBuffer, &nBytesRead);
		if (error == VE_SRVR_CONNECTION_BROKEN)
			return VE_SRVR_CONNECTION_BROKEN;

		nBytesLeftToRead -= nBytesRead;
		if (nBytesRead > 0)
		{
			memcpy (((char *)inBuffer) + nTotalBytesRead, fBuffer, nBytesRead);
			if (fUseFullBuffering)
			{
				bool bResult = fMemoryBuffer.PutDataAmortized (fMemoryBuffer.GetDataSize(), fBuffer, nBytesRead);
				if (!bResult)
					return XBOX::VE_MEMORY_FULL;
			}
		}

		nTotalBytesRead += nBytesRead;
	}

	return XBOX::VE_OK;
}
