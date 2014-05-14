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
#include "ZipComponent.h"
#include "VZipComponent.h"
#include "ZipErrors.h"
#include <zconf.h>
#include <zlib.h>
#include "Kernel/VKernel.h"


USING_TOOLBOX_NAMESPACE

VComponentLibrary *gVZipComponentLibrary = NULL;
VZipComponent *VZipComponent::sCurrentInstance = NULL;


void XBOX::xDllMain()
{
	gVZipComponentLibrary = new VZipComponentLibrary(kCOMPONENT_TYPE_LIST, kCOMPONENT_TYPE_COUNT);
}

VZipComponentLibrary::VZipComponentLibrary(const CImpDescriptor* inTypeList, VIndex inTypeCount) : VComponentLibrary(inTypeList, inTypeCount)
{
}

VZipComponentLibrary::~VZipComponentLibrary(){}
	
void VZipComponentLibrary::DoRegister (){}
void VZipComponentLibrary::DoUnregister (){}


VZipComponent::VZipComponent (){}
VZipComponent::~VZipComponent (){}
VError VZipComponent::Register() {return VE_OK;}
VError VZipComponent::UnRegister() {return VE_OK;}


VComponentLibrary* VZipComponent::GetComponentLibrary()
{
	return (gVZipComponentLibrary);
}



/************************************************************************************************* 
							CompressMemoryBlock 
************************************************************************************************** */

VError VZipComponent::CompressMemoryBlock(void* inBlockToCompress, sLONG8 inBlockSize, EZipCompressionLevel inCompressionLevel, XBOX::VStream* outCompressedStream )
{
	if(!testAssert(inBlockToCompress != NULL))
		return VE_INVALID_PARAMETER;
	if(!testAssert(outCompressedStream != NULL))
		return VE_INVALID_PARAMETER;
	if(!testAssert(inBlockSize > 0))
		return VE_INVALID_PARAMETER;	
	
	VError errorToReturn = VE_OK;
	
	if(!outCompressedStream->IsWriting())
		errorToReturn = vThrowError(VE_STREAM_NOT_OPENED);
		
	
	if (errorToReturn == VE_OK)
	{
		// The buffer maximum size is defined depending on the compression level required. 
		// If best speed is required, the buffer size is 5Mo, but more memory ressources are needed.  
		// If best shrunk is required, the buffer size is 500Ko, but it will compress more slowly. 
		// Otherwise, the buffer size is 1Mo.
		sLONG8 bufferMaxSize = 500000;
		sLONG8 remainingDataSize = inBlockSize;
		
		bool useGzip = false;
		switch(inCompressionLevel)
		{
		case  eCompressionLevel_BestCompression:
		case  eCompressionLevel_Store:
			bufferMaxSize = 500000;
			break;

		case  eCompressionLevel_Default:
			bufferMaxSize = 1000000;
			break;

		case  eCompressionLevel_BestSpeed:
			bufferMaxSize = 5000000;
			break;

		case  eCompressionLevel_GZip_BestCompression:
			{
				useGzip = true;
				bufferMaxSize = 500000;
			}
			break;

		case  eCompressionLevel_GZip_BestSpeed:
			{
				useGzip = true;
				bufferMaxSize = 5000000;
			}
			break;

		default:
			bufferMaxSize = ((sLONG)inCompressionLevel <= 3) ? 5000000 : 1000000; // see deflate.c (zlib)
			break;
		}
		
		sLONG8 sizeToCompressOnSingleStep = remainingDataSize;
		
		// if the size of the block to compress is bigger than the bufferSizeMax, the compression will be done in several times, compressing  bufferMaxSize of data each time. 
		// Otherwise, it will be done on a single step.
		if (sizeToCompressOnSingleStep > bufferMaxSize)
			sizeToCompressOnSingleStep = bufferMaxSize;
		
		
		// The destination buffer must be bigger than the source buffer (see the libzip doc)
		sLONG8  compressedBufferLength = 1.02 * sizeToCompressOnSingleStep + 12;	
		
		
		Bytef * compressedBuffer = (Bytef *) malloc(compressedBufferLength * sizeof(Bytef));					
		if(compressedBuffer == NULL)
			errorToReturn = VE_MEMORY_FULL;
		
		
		if  (errorToReturn == VE_OK)
		{
			
			/* initialization of the zlib structure. For info:
			- next_in/out =  adress of inBlockToCompress/compressedBuffer
			- avail_in = number of bytes available in the inBlockToCompress
			- avail_out = remaining free space in the compressedBuffer 
			- total_in = total size of data already compressed
			- total_out = total size of compressed data
			*/
			z_stream zlib_streamToCompress; 
			::memset(&zlib_streamToCompress,0,sizeof(z_stream));
			
			zlib_streamToCompress.next_in = (Bytef*)inBlockToCompress;
			zlib_streamToCompress.avail_in = zlib_streamToCompress.total_in = 0;
			
			zlib_streamToCompress.next_out = compressedBuffer;
			zlib_streamToCompress.avail_out = compressedBufferLength;
			
			zlib_streamToCompress.zalloc = Z_NULL;
			zlib_streamToCompress.zfree = Z_NULL;
			zlib_streamToCompress.opaque = NULL;		
				
			// initializing compression
			int compressionError = Z_OK;
			if (useGzip) //JQ 05/02/2009: added Gzip compression
				compressionError = deflateInit2(&zlib_streamToCompress, ((int)inCompressionLevel) & (~16), Z_DEFLATED, 15+16, 8, Z_DEFAULT_STRATEGY);
			else
				compressionError = deflateInit(&zlib_streamToCompress, inCompressionLevel);

			int flushOption = Z_NO_FLUSH;
			while ((compressionError == Z_OK) && (remainingDataSize > 0))
			{
				// the remaining size of data to compress must always be ranging between 0 and inBlockSize.
				assert(zlib_streamToCompress.total_in >= 0);
				assert(zlib_streamToCompress.total_in <= inBlockSize);

				// For large streams, after several steps for exemple, we want to compress only the remaining data to compress in the inBlockToCompress 
				if (inBlockSize - zlib_streamToCompress.total_in < sizeToCompressOnSingleStep)
					sizeToCompressOnSingleStep = inBlockSize - zlib_streamToCompress.total_in;

				// if there are still data to compress, we update avail_in.
				if (inBlockSize - zlib_streamToCompress.total_in > 0)
					zlib_streamToCompress.avail_in = (uInt)sizeToCompressOnSingleStep;

				flushOption = ((remainingDataSize - sizeToCompressOnSingleStep) > 0) ? Z_NO_FLUSH : Z_FINISH; // YT 10-Dec-2012 - ACI0079552
				compressionError = deflate(&zlib_streamToCompress, flushOption);


				if (compressionError != Z_OK && compressionError != Z_STREAM_END)
				{
					if (compressionError == Z_VERSION_ERROR)
						errorToReturn = vThrowError(VE_ZIP_BAD_VERSION);
					else
						errorToReturn = vThrowError(VE_ZIP_COMPRESSION_FAILED);
				}


				// the remaining free space in the compressedBuffer must always be ranging between 0 and compressedBufferLength 
				assert(zlib_streamToCompress.avail_out >= 0);
				assert(zlib_streamToCompress.avail_out <= compressedBufferLength);


				// if there is some compressed data in the compressed buffer, they must be written in the outCompressedStream.
				if (zlib_streamToCompress.avail_out < compressedBufferLength)
				{
					errorToReturn = outCompressedStream -> PutData(compressedBuffer, compressedBufferLength - zlib_streamToCompress.avail_out);
					if (errorToReturn != VE_OK)
					{
						errorToReturn = vThrowError(VE_STREAM_CANNOT_WRITE);
						break;
					}

					zlib_streamToCompress.next_out = (Bytef *)compressedBuffer;
					zlib_streamToCompress.avail_out = compressedBufferLength;
				}
				remainingDataSize -= sizeToCompressOnSingleStep;
			} // while ((compressionError == Z_OK) && (remainingDataSize > 0))

			deflateEnd(&zlib_streamToCompress);


			if ((compressionError == Z_OK || compressionError == Z_STREAM_END) && remainingDataSize<=0 )
				errorToReturn = VE_OK;
			else
				errorToReturn = VE_UNKNOWN_ERROR;
		}
		
		if (compressedBuffer != NULL)
			free(compressedBuffer);	
	}
	
	return errorToReturn;
}


/************************************************************************************************* 
									ExpandMemoryBlock 
************************************************************************************************** */

VError VZipComponent::ExpandMemoryBlock(void* inCompressedBlock, sLONG8 blockSize, XBOX::VStream* outExpandedStream ) 	
{
	if(!testAssert(inCompressedBlock != NULL))
		return VE_INVALID_PARAMETER;
	if(!testAssert(outExpandedStream != NULL))
		return VE_INVALID_PARAMETER;
	if(!testAssert(blockSize > 0))
		return VE_INVALID_PARAMETER;
	
	VError errorToReturn = VE_OK;
	
	if (!outExpandedStream -> IsWriting()){
		errorToReturn = vThrowError(VE_STREAM_NOT_OPENED);
	}
	
	if (errorToReturn == VE_OK){ 
		sLONG8 bufferMaxSize = 1000000;
		
		// the decompression will be done by steps if needed, decompressing data until the expanded buffer becomes full each time. 
		sLONG8 expandedBufferLength = bufferMaxSize;
		
		Bytef * expandedBuffer = (Bytef *) malloc(expandedBufferLength  * sizeof(Bytef));
		if(expandedBuffer == NULL){
			errorToReturn = vThrowError(VE_MEMORY_FULL);
		}	
		
		
		if (errorToReturn == VE_OK){
			
			/* initialization of zlib structure. For info:
			- next_in/out =  adress of inCompressedBlock/expandedBuffer
			- avail_in = number of bytes available in the inCompressedBlock
			- avail_out = remaining free space in the expandedBuffer 
			- total_in = total size of data already expanded
			- total_out = total size of expanded data
			*/
			z_stream zlib_compressedStream;
			::memset(&zlib_compressedStream,0,sizeof(z_stream));
			
			zlib_compressedStream.next_in = (Bytef *)inCompressedBlock;
			zlib_compressedStream.avail_in = blockSize;
			zlib_compressedStream.total_in = 0;
			
			zlib_compressedStream.next_out = expandedBuffer;
			zlib_compressedStream.avail_out = (uInt)expandedBufferLength;
			
			zlib_compressedStream.zalloc = Z_NULL;
			zlib_compressedStream.zfree = Z_NULL;
			zlib_compressedStream.opaque = Z_NULL;
			
			// initializing decompression
			// JQ 05/02/2009: we need to add 32 to default windowBits
			//				  in order to enable gzip (+zlib) inflating 
			//				  (with 15 inflate can inflate only zlib stream)
			//				  (SVG component need gzip inflating in order to inflate svgz files
			//				   which are gzip compressed)
			int decompressionError = inflateInit2(&zlib_compressedStream, 15+32);
			
			if (decompressionError == Z_OK){
				
				do{
					
					decompressionError = inflate(&zlib_compressedStream, Z_SYNC_FLUSH);
					
					// the remaining size of data to expand must always be ranging between 0 and blockSize.
					assert(zlib_compressedStream.total_in >= 0);
					assert(zlib_compressedStream.total_in <= blockSize);
					// the remaining free space in the expandedBuffer must always be ranging between 0 and expandedBufferLength. 
					assert(zlib_compressedStream.avail_out >= 0);
					assert(zlib_compressedStream.avail_out <= expandedBufferLength);
					
					
					switch (decompressionError){
						
						case Z_OK:
						case Z_STREAM_END: 	
							// if there is some expanded data in the expanded buffer, they must be written in the expandedStream.
							if(expandedBufferLength - zlib_compressedStream.avail_out > 0){
								errorToReturn = outExpandedStream -> PutData(expandedBuffer, expandedBufferLength - zlib_compressedStream.avail_out);
								if (errorToReturn != VE_OK){
									errorToReturn = vThrowError(VE_STREAM_CANNOT_WRITE);
									break;
								}
								// we update next_out and avail_out with the recent emptied buffer. 
								zlib_compressedStream.next_out = expandedBuffer;
								zlib_compressedStream.avail_out = (uInt)expandedBufferLength;
							} 
							break;
							
							
							// error cases		
						case Z_DATA_ERROR:
							errorToReturn = vThrowError(VE_ZIP_NON_COMPRESSED_INPUT_DATA);
							break;
							
						case Z_VERSION_ERROR:
							errorToReturn = vThrowError(VE_ZIP_BAD_VERSION);
							break;
							
						case Z_BUF_ERROR:	
						case Z_NEED_DICT:
						case Z_STREAM_ERROR:
						case Z_MEM_ERROR:
							errorToReturn = vThrowError(VE_ZIP_DECOMPRESSION_FAILED);
							break;	
					}
					
				}
				// we stop looping if there are no more data to expand, or if an error occured in decompression 
				while(errorToReturn == VE_OK  && decompressionError != Z_STREAM_END);
				// DH 22-Feb-2013 In case the blob is corrupted we have to stop on Z_STREAM_END even if blockSize>zlib_compressedStream.total_in otherwise we loop indefinitely (see http://www.zlib.net/zlib_how.html for usage example)
				
				inflateEnd(&zlib_compressedStream);	// YT 13-Mar-2009 - ACI0060740
			}
			
			// if the initilization of decompression failed. 
			else
				errorToReturn = vThrowError(VE_ZIP_DECOMPRESSION_FAILED);
		}
		
		if (expandedBuffer != NULL)	
			free(expandedBuffer);
	}
	
	return errorToReturn;
	
}


/************************************************************************************************* 
										CompressStream 
************************************************************************************************** */

VError VZipComponent::CompressStream(VStream* inStreamToCompress, EZipCompressionLevel inCompressionLevel, VStream* outCompressedStream )
{
	if(!testAssert(inStreamToCompress != NULL))
		return VE_INVALID_PARAMETER;
	if(!testAssert(outCompressedStream != NULL))
		return VE_INVALID_PARAMETER;	
	if(!testAssert( ( (inStreamToCompress -> GetSize()) - (inStreamToCompress -> GetPos()) ) > 0))
		return VE_STREAM_EOF;
	
	VError errorToReturn = VE_OK;
	
	if (!inStreamToCompress->IsReading()){
		errorToReturn = vThrowError(VE_STREAM_NOT_OPENED);
	}
	
	if(errorToReturn == VE_OK){
		if(!outCompressedStream->IsWriting())
			errorToReturn = vThrowError(VE_STREAM_NOT_OPENED);
	}	
	
	if(errorToReturn == VE_OK){
		
		// The stream to compress is put in a buffer before calling CompressMemoryBlock
		sLONG8 streamToCompressSize = (inStreamToCompress -> GetSize()) - (inStreamToCompress -> GetPos());
		
		Bytef * bufferToCompress = (Bytef *) malloc(streamToCompressSize * sizeof(Bytef));
		if(bufferToCompress == NULL){
			errorToReturn = VE_MEMORY_FULL;
		}
		
		if (errorToReturn == VE_OK){
			
			errorToReturn = inStreamToCompress -> GetData(bufferToCompress, streamToCompressSize);
			if (errorToReturn != VE_OK){
				errorToReturn = vThrowError(VE_STREAM_CANNOT_READ);
			}
			
			
			if (errorToReturn == VE_OK){
				errorToReturn = CompressMemoryBlock(bufferToCompress, streamToCompressSize, inCompressionLevel, outCompressedStream);
			}
			
			free (bufferToCompress); // YT 23-Nov-2009 - Fix memory leaks
		}
	}
	
	return errorToReturn;

}



/************************************************************************************************* 
										ExpandStream 
***************************************************************************************************/

VError VZipComponent::ExpandStream(VStream* inCompressedStream, VStream* outExpandedStream )
{
	if(!testAssert(inCompressedStream != NULL))
		return VE_INVALID_PARAMETER;
	if(!testAssert(outExpandedStream != NULL))
		return VE_INVALID_PARAMETER;
	if(!testAssert( ( (inCompressedStream -> GetSize()) - (inCompressedStream -> GetPos()) ) > 0))
		return VE_STREAM_EOF;
	
	
	VError errorToReturn = VE_OK;
	
	if (!inCompressedStream->IsReading()){
		errorToReturn = vThrowError(VE_STREAM_NOT_OPENED);
	}
	
	
	if(errorToReturn == VE_OK){
		if (!outExpandedStream -> IsWriting())
			errorToReturn = vThrowError(VE_STREAM_NOT_OPENED);
	}	
	
	
	if(errorToReturn == VE_OK){
		// The stream to expand is put in a buffer before calling ExpandMemoryBlock
		sLONG8 streamToExpandSize = (inCompressedStream -> GetSize()) - (inCompressedStream -> GetPos()) ;
		
		Bytef * compressedBuffer = (Bytef *) malloc(streamToExpandSize  * sizeof(Bytef));					
		if(compressedBuffer == NULL){
			errorToReturn = vThrowError(VE_MEMORY_FULL);
		}
		
		if(errorToReturn == VE_OK){
			
			errorToReturn = inCompressedStream -> GetData(compressedBuffer, streamToExpandSize);
			if (errorToReturn != VE_OK){
				errorToReturn = vThrowError(VE_STREAM_CANNOT_READ);
			}
			
			if(errorToReturn == VE_OK)
				errorToReturn = ExpandMemoryBlock(compressedBuffer, streamToExpandSize, outExpandedStream); 
		}

		if (NULL != compressedBuffer)
			free (compressedBuffer); // YT 14-Mar-2013 - ACI0080816
	}
	
	return errorToReturn;
}



/************************************************************************************************* 
										CompressBlob 
***************************************************************************************************/

VError VZipComponent::CompressBlob( VBlob *inBlobToCompress , const EZipCompressionLevel inCompressionLevel , VBlob *outCompressedBlob )
{
	if(!testAssert(inBlobToCompress != NULL))
		return VE_INVALID_PARAMETER;
	if(!testAssert(outCompressedBlob != NULL))
		return VE_INVALID_PARAMETER;
	
	VError errorToReturn = VE_OK;
	
	
	VBlobStream* compressedBlobStream = new VBlobStream();
	
	errorToReturn = compressedBlobStream -> OpenWriting();	
	if (errorToReturn != VE_OK)
		errorToReturn = vThrowError(VE_ZIP_CANNOT_ACCESS_BLOB_CONTENT);

	
	if (errorToReturn == VE_OK){ 
	
		VBlobWithPtr* blobWithPtrToCompress = new VBlobWithPtr(*inBlobToCompress);
		
		if (errorToReturn == VE_OK){
			
			errorToReturn = CompressMemoryBlock(blobWithPtrToCompress->GetDataPtr(), inBlobToCompress->GetSize(), inCompressionLevel, compressedBlobStream);
			
			if (errorToReturn == VE_OK){
				
				// Once the compression succedeed, we have to put the content of compressedBlobStream in outCompressedBlob. 
				//To do so, we have to pass via a temporary buffer to transfer these data (since the class VBlob does not inherit IStreamable). 
				errorToReturn = compressedBlobStream -> CloseWriting(true);
				
				if (errorToReturn == VE_OK){
										
					errorToReturn = compressedBlobStream -> OpenReading();
					
					if (errorToReturn == VE_OK){
						
						sLONG8 compressedBlobStreamSize = compressedBlobStream -> GetSize();
						Bytef * compressedBuffer = (Bytef *) malloc(compressedBlobStreamSize  * sizeof(Bytef));	
						if (compressedBuffer == NULL){
							errorToReturn = vThrowError(VE_MEMORY_FULL);
						}	
						
						if (errorToReturn == VE_OK){
							
							// if everything is ok, the data of compressedBlobStream are copyied in the temporary buffer
							errorToReturn = compressedBlobStream -> GetData(compressedBuffer, compressedBlobStreamSize);
							compressedBlobStream -> CloseReading();
							
							if (errorToReturn == VE_OK){
								// if everything is still ok, the data of the temporary buffer are copyied in outCompressedBlob
								errorToReturn = outCompressedBlob -> PutData(compressedBuffer, compressedBlobStreamSize);
								
							}
						}	
						
						if (compressedBuffer != NULL)
							free(compressedBuffer);	
					} 
				}
				
				// if something wrong occured with streams either in compression (streams cannot be read, opened, closed, etc.) or in transfering data to outCompressedBlob
				// note: if it is a pure compression error, this error is thrown.
				if (errorToReturn != VE_OK && errorToReturn != VE_ZIP_COMPRESSION_FAILED){
					errorToReturn = vThrowError(VE_ZIP_CANNOT_ACCESS_BLOB_CONTENT);
				}
			}
		}
	
		delete blobWithPtrToCompress;
	}
	delete compressedBlobStream; 
	return errorToReturn; 
}




/************************************************************************************************* 
										ExpandBlob 
***************************************************************************************************/

VError VZipComponent::ExpandBlob(VBlob *inCompressedBlob , VBlob *outExpandedBlob)
{
	if(!testAssert(inCompressedBlob != NULL))
		return VE_INVALID_PARAMETER;
	if(!testAssert(outExpandedBlob != NULL))
		return VE_INVALID_PARAMETER;
	
	VError errorToReturn = VE_OK;
	
	
	VBlobStream* expandedBlobStream = new VBlobStream();	
	VError openWritingExpandedBlobStream = expandedBlobStream -> OpenWriting();			
	if (openWritingExpandedBlobStream != VE_OK)
		errorToReturn = vThrowError(VE_ZIP_CANNOT_ACCESS_BLOB_CONTENT);
	
	
	if (errorToReturn == VE_OK){
		
		VBlobWithPtr* compressedBlobWithPtr = new VBlobWithPtr(*inCompressedBlob);
		
		if (errorToReturn == VE_OK){ 
			
			errorToReturn = ExpandMemoryBlock(compressedBlobWithPtr->GetDataPtr(), inCompressedBlob->GetSize(), expandedBlobStream);
			
			if (errorToReturn == VE_ZIP_NON_COMPRESSED_INPUT_DATA){
				errorToReturn = vThrowError(VE_ZIP_NON_COMPRESSED_INPUT_BLOB);	
			}
			
			if (errorToReturn == VE_OK){
				
				// Once the decompression succeded, we have to put the content of expandedBlobStream in outExpandedBlob. 
				//To do so, we have to pass via a temporary buffer to transfer these data (since the class VBlob does not inherit IStreamable). 
				errorToReturn = expandedBlobStream -> CloseWriting(true);
				
				if (errorToReturn == VE_OK){
					
					errorToReturn = expandedBlobStream -> OpenReading();
					
					sLONG8 expandedBlobStreamSize = expandedBlobStream -> GetSize();
					Bytef * expandedBuffer = (Bytef *) malloc(expandedBlobStreamSize  * sizeof(Bytef));	
					if (expandedBuffer == NULL){
						errorToReturn = vThrowError(VE_MEMORY_FULL);
					}	
					
					if (errorToReturn == VE_OK){
						// if everything is ok, the data of expandedBlobStream are copyied in the temporary buffer
						errorToReturn = expandedBlobStream -> GetData(expandedBuffer, expandedBlobStreamSize);
						expandedBlobStream -> CloseReading();
						
						
						if (errorToReturn == VE_OK){
							// if everything is still ok, the data of the temporary buffer are copyied in outExpandedBlob
							errorToReturn = outExpandedBlob -> PutData(expandedBuffer, expandedBlobStreamSize);
						}
					}	
					
					if (expandedBuffer != NULL)
						free(expandedBuffer); 
				}				
			}
		}
		
		// if something wrong occured with streams either in decompression (streams cannot be read, opened, closed, etc.) or in transfering data to outExpandedBlob
		// note: if it is a pure decompression error, this error is thrown.
		if (errorToReturn != VE_OK && errorToReturn != VE_ZIP_DECOMPRESSION_FAILED && errorToReturn != VE_ZIP_NON_COMPRESSED_INPUT_BLOB){
			errorToReturn = vThrowError(VE_ZIP_CANNOT_ACCESS_BLOB_CONTENT);
		}
	}
	delete expandedBlobStream;
	
	return errorToReturn;	
}  




