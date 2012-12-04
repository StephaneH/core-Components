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
// CMyComponent.h
//

#ifndef __CZipComponent__
#define __CZipComponent__


#include "Kernel/Sources/VBlob.h"
#include "Kernel/Sources/VStream.h"
#include "Kernel/Sources/VTypes.h"
#include "KernelIPC/Sources/VComponentManager.h"



//******** STEP 1/4 of creating a component*****
// 1) To write a component library, define your component class deriving it
// from CComponent. You will provide this class with your library as a header file.
//
// class CComponent1 : public CComponent
// {
//	public:
//	enum { Component_Type = 'cmp1' };
//
//	Blah, blah - Public Interface
// };


/* 
 The compresssion level can be choosen to compress data. It can be either: 
 - eCompressionLevel_Default : data are compressed with a compromise between speed and compression 
 - eCompressionLevel_Store : no compression.
 - eCompressionLevel_BestSpeed : data are  compressed as quickly as possible
 - eCompressionLevel_BestCompression : data are as compressed as possible 
*/
enum EZipCompressionLevel {
	eCompressionLevel_Default = -1,
	eCompressionLevel_Store = 0,
	eCompressionLevel_BestSpeed = 1,
	eCompressionLevel_BestCompression = 9,

	eCompressionLevel_GZip_BestSpeed = 1+16,
	eCompressionLevel_GZip_BestCompression = 9+16 
};




class CZipComponent : public XBOX::CComponent
{
public:
	enum { Component_Type = 'Zip ' };
	
// Note that a component has no constructor, no data and only pure virtuals (= 0)	

	// ***********************  CompressMemoryBlock   ***************************
	/** @brief This function will compress any block in memory. 
		@param inBlockToCompress The pointer on the block you want to compress
		@param blockSize the size of data you want to compress
		@param outCompressedStream The out compressed Stream. NB: This stream must be open for writing before passing it to this function   
		@param inCompressionLevel The level of compression you want. It can be:	
		- eCompressionLevel_Default : data are compressed with a compromise between speed and compression 
		- eCompressionLevel_Store : no compression.
		- eCompressionLevel_BestSpeed : data are  compressed as quickly as possible
		- eCompressionLevel_BestCompression : data are as compressed as possible  
		@result it can be:
		- VE_OK if evrything is OK
		- VE_INVALID_PARAMETER if the pointer in parameter are NULL or in the blockSize is 0. 
		- VE_MEMORY_FULL
		- VE_ZIP_COMPRESSON_FAILED if something wrong occured within compression
		- anonther VError related with streams manipulation: VE_STREAM_NOT_OPENED, VE_STREAM_CANNOT_WRITE.
	*/
	virtual XBOX::VError CompressMemoryBlock(void* inBlockToCompress, sLONG8 blockSize, EZipCompressionLevel inCompressionLevel, XBOX::VStream* outCompressedStream ) = 0; 
	
	
	
	// ***********************  ExpandMemoryBlock   ***************************
	/** @brief This function will expand any compressed block in memory. 
		@param inCompressedBlock The pointer on the block you want to expand
		@param blockSize the size of data you want to expand
		@param outCompressedStream The out compressed Stream. NB: This stream must be open for writing before passing it to this function    
		@result it can be:
		- VE_OK if evrything is OK
		- VE_INVALID_PARAMETER if the pointer in parameter are NULL or in the blockSize is 0. 
		- VE_MEMORY_FULL
		- VE_ZIP_DECOMPRESSON_FAILED if something wrong occured within decompression
		- anonther VError related with streams manipulation: VE_STREAM_NOT_OPENED, VE_STREAM_CANNOT_WRITE.
		*/
	
	virtual XBOX::VError ExpandMemoryBlock(void* inCompressedBlock, sLONG8 blockSize, XBOX::VStream* outExpandedStream ) = 0; 	
	
	
 	// ***********************  CompressStream   ***************************
	/** @brief This function will compress a Stream. 
		@param inStreamToCompress The stream you want to compress. NB: This stream must be open for reading before passing it to this function			
		@param outCompressedStream The out compressed Stream. NB: This stream must be open for writing before passing it to this function   
		@param inCompressionLevel The level of compression you want. It can be:	
		- eCompressionLevel_Default : data are compressed with a compromise between speed and compression 
		- eCompressionLevel_Store : no compression.
		- eCompressionLevel_BestSpeed : data are  compressed as quickly as possible
		- eCompressionLevel_BestCompression : data are as compressed as possible   
		@result it can be:
				- VE_OK if evrything is OK
				- VE_INVALID_PARAMETER if the pointer in parameter are NULL 
				- VE_ZIP_COMPRESSON_FAILED if something wrong occured within compression
				- anonther VError related with streams manipulation: VE_STREAM_NOT_OPENED, VE_STREAM_CANNOT_WRITE, VE_STREAM_EOF.
	*/	
	
	virtual XBOX::VError CompressStream(XBOX::VStream* inStreamToCompress, EZipCompressionLevel inCompressionLevel, XBOX::VStream* outCompressedStream ) = 0; 

	
 	// ***********************  ExpandStream   ***************************	
	/** @brief This function will expand a Stream.
		@param inCompressedStream The compressed stream you will send.  NB: This stream must be open for reading before passing it to this function.
		@param outExpandedStream The out expanded Stream. NB: This stream must be open for writing before passing it to this function.
		@result it can be:
				- VE_OK if evrything is OK.
				- VE_INVALID_PARAMETER if the pointer in parameter are NULL. 
				- VE_ZIP_DECOMPRESSON_FAILED if something wrong occured within decompression.
				- VE_ZIP_NOT_COMPRESSED_INPUT_STREAM if the stream to expand does not contain compressed data or if these data are corrupted. 
				- anonther VError related with streams manipulation: VE_STREAM_NOT_OPENED, VE_STREAM_CANNOT_WRITE, VE_STREAM_EOF.
		*/

	virtual XBOX::VError ExpandStream(XBOX::VStream *inCompressedStream, XBOX::VStream *outExpandedStream ) = 0;

	
	
	// **********************  CompressBlob  ******************************
	/** @brief This function compresses a blob 
		@param inBlobToCompress The in blob you want to compress
		@param outCompressedBlob The compressed out blob 
		@param inCompressionLevel The level of compression you want. It can be:	
		- eCompressionLevel_Default : data are compressed with a compromise between speed and compression 
		- eCompressionLevel_Store : no compression.
		- eCompressionLevel_BestSpeed : data are  compressed as quickly as possible
		- eCompressionLevel_BestCompression : data are as compressed as possible   
		@result it can be:
				- VE_OK if evrything is OK.
				- VE_INVALID_PARAMETER if the pointer in parameter are NULL. 
				- VE_ZIP_COMPRESSON_FAILED if something wrong occured during the compression.
				- VE_ZIP_CANNOT_ACCESS_BLOB_CONTENT if something wrong occured while opening, closing, reading or writing in the blob.
			
		*/
	virtual XBOX::VError CompressBlob( XBOX::VBlob *inBlobToCompress , const  EZipCompressionLevel inCompressionLevel, XBOX::VBlob *outCompressedBlob ) = 0;

	
		
 	// ***********************  ExpandBlob   ***************************	
	/** @brief This function will expand a Blob.
		@param inCompressedBlob The compressed blob you will send.
		@param outExpandedBlob The out expanded blob
		@result it can be:
				- VE_OK if evrything is OK
				- VE_INVALID_PARAMETER if the pointer in parameter are NULL. 
				- VE_ZIP_DECOMPRESSON_FAILED if the memory is full, or if their not enough room in the output buffer.
				- VE_ZIP_NOT_COMPRESSED_INPUT_STREAM if the stream to expand does not contain compressed data or if these data are corrupted. 
				- anonther VError related with streams manipulation: VE_STREAM_CANNOT_READ, VE_STREAM_USER_ABORTED, VE_STREAM_NO_TEXT_CONVERTER, VE_STREAM_CANNOT_WRITE.
		*/
	virtual XBOX::VError ExpandBlob( XBOX::VBlob *inCompressedBlob , XBOX::VBlob *outExpandedBlob) = 0 ;  


};

#endif

