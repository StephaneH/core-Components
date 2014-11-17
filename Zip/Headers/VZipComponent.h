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
#ifndef __VZipComponent__
#define __VZipComponent__

#include "KernelIPC/Sources/VComponentLibrary.h"
#include "CZipComponent.h"


class VZipComponentLibrary : public VComponentLibrary
{
public:
typedef	VComponentLibrary	inherited;

			VZipComponentLibrary (const CImpDescriptor* inTypeList, sLONG inTypeCount);
	virtual	~VZipComponentLibrary();
	
protected:
	virtual	void	DoRegister ();
	virtual	void	DoUnregister ();
};




//******** STEP 2/4 of creating a component*****
// 2) Then define a custom class deriving from VComponentImp<YourComponent>
// and implement your public and private functions. If you want to receive messages,
// override ListenToMessage(). You may define several components using the same pattern.
//
// class VComponentImp1 : public VComponentImp<CComponent1>
// {
//	public:
//	Blah, blah - Public Interface
//
//	protected:
//	Blah, blah - Private Stuff
// };




// You must declare here all the public stuff you declared in the CMyComponent class,
// plus any stuff you want (if won't be visible to users of your component).

class VZipComponent : public VComponentImp< CZipComponent >
{
public:
typedef	VComponentImp< CZipComponent >	inherited;

	VZipComponent ( );
	virtual ~VZipComponent ( );
	
	
	
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
	virtual XBOX::VError CompressMemoryBlock(void* inBlockToCompress, sLONG8 blockSize, EZipCompressionLevel inCompressionLevel, XBOX::VStream* outCompressedStream ); 
	
	
	
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
	
	virtual XBOX::VError ExpandMemoryBlock(void* inCompressedBlock, sLONG8 blockSize, XBOX::VStream* outExpandedStream ); 	
	
	
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
	
	virtual XBOX::VError CompressStream(XBOX::VStream* inStreamToCompress, EZipCompressionLevel inCompressionLevel, XBOX::VStream* outCompressedStream ); 
	
	
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
	
	virtual XBOX::VError ExpandStream(XBOX::VStream *inCompressedStream, XBOX::VStream *outExpandedStream );
	
	
	
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
	virtual XBOX::VError CompressBlob( XBOX::VBlob *inBlobToCompress , const  EZipCompressionLevel inCompressionLevel, XBOX::VBlob *outCompressedBlob );
	
	
	
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
	virtual XBOX::VError ExpandBlob( XBOX::VBlob *inCompressedBlob , XBOX::VBlob *outExpandedBlob);  
	
	
	
	virtual VError Register() ;
	virtual VError UnRegister() ;

	// Utilities
	static CComponent*			GetCurrentInstance () { return (CComponent*) sCurrentInstance; };
	static VResourceFile*		RetainResFile () { return (sCurrentInstance != NULL) ? sCurrentInstance->RetainResourceFile() : NULL; };
	static VComponentLibrary*	GetComponentLibrary();
	
protected:
		
	static VZipComponent*	sCurrentInstance;

};




//******** STEP 3/4 of creating a component*****
// 3) Declare a kCOMPONENT_TYPE_LIST constant.
// This constant will automate the CreateComponent() in the dynamic lib:
//
// const sLONG			kCOMPONENT_TYPE_COUNT	= 1;
// const CImpDescriptor	kCOMPONENT_TYPE_LIST[]	= { { CComponent1::Component_Type,
//													  VImpCreator<VComponentImp1>::CreateImp } };



// All registration/creation mecanism is handled automatically by declaring a const array
// linking your component types with its creator. The only thing you have to fo is to include
// VComponentLibrary.cpp in your project making sure that it is compiled after this file.
//

const sLONG				kCOMPONENT_TYPE_COUNT	= 1;
const CImpDescriptor	kCOMPONENT_TYPE_LIST[]	=
							{ { CZipComponent::Component_Type, VImpCreator<VZipComponent>::CreateImp } };


#endif
