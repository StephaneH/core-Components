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
#ifndef __UTIL4D__
#define __UTIL4D__


#ifndef ASYNC
	#define ASYNC 0
#endif


typedef enum 
{
	PK_UniquenessChecking=0
} DB4D_ProgressIndicatorKind;


class VDB4DProgressIndicatorWrapper:public XBOX::VObject
{
public:
    VDB4DProgressIndicatorWrapper(VDB4DProgressIndicator* inWrappedProgressIndicator = nil);
    virtual ~VDB4DProgressIndicatorWrapper();
    
    VDB4DProgressIndicator* GetProgressIndicator()const{return fProgressIndicator;}
    bool SetTitleAfterKind(DB4D_ProgressIndicatorKind inKind);
private:
    VDB4DProgressIndicatorWrapper(const VDB4DProgressIndicatorWrapper&);
    
    VDB4DProgressIndicator* fProgressIndicator;
};

bool SetProgressIndicatorTitleForKind(VDB4DProgressIndicator& inPi, DB4D_ProgressIndicatorKind inKind);

void trackDebugMsg(const VString& mess);

extern const VString kDataFileExt;
extern const VString kDataFileBlobExt;
extern const VString kWaRemoteModelExt;
extern const VString kWaRemoteModel;

extern const VString kDataIndexExt;
extern const VString kStructIndexExt;

extern const VString kSyncDataExt;
extern const VString kSyncHeaderExt;

extern const VString kDataMatchExt;
extern const VString kDataTableDefExt;

extern const VString kDataExtraExt;	// no dot

void InitFileExtensions(DB4D_Running_Mode inRunningMode);


ENTITY_API extern const VString kLocalJSPath;
ENTITY_API extern const VString khostname;
ENTITY_API extern const VString kuser;
ENTITY_API extern const VString kpassword;
ENTITY_API extern const VString kSQL;
ENTITY_API extern const VString kssl;
ENTITY_API extern const VString kdatabase;
ENTITY_API extern const VString kport;
ENTITY_API extern const VString kremoteAccess;
ENTITY_API extern const VString krestPrefix;
ENTITY_API extern const VString ktimeout;



#define noaction DBactionFinale

EXTERN_BAGKEY (____objectunic);

typedef std::map< const std::type_info* , sLONG > ObjCountMap;

class IObjCounter
{
	public:
#if debuglr
		IObjCounter()
		{
			/*
			VObject* thethis = dynamic_cast<VObject*>(this);
			const type_info* typinf = &(typeid(thethis));
			sObjCount[typinf]++;
			*/
		}

		virtual ~IObjCounter()
		{
			/*
			VObject* thethis = dynamic_cast<VObject*>(this);
			const type_info* typinf = &(typeid(thethis));
			sObjCount[typinf]--;
			*/
		}

#endif

	protected:
		static ObjCountMap sObjCount;

};

class VDBMgr;

const sLONG kMaxXStringLen = 30;

const sLONG kMaxBytesForKeyStrings = 2048;
const sLONG kMaxXStringPtrLen = (kMaxBytesForKeyStrings - 4) / 2;

const sLONG kMaxFieldNameLength = 31;
const sLONG kMaxFieldNameNbWords = kMaxFieldNameLength + 1;

const sLONG kMaxTableNameLength = 31;
const sLONG kMaxTableNameNbWords = kMaxTableNameLength + 1;

const sLONG DB4D_Index_OnOneField_Scalar_Range_Begin = 10;
const sLONG DB4D_Index_OnOneField_Scalar_Word = DB4D_Index_OnOneField_Scalar_Range_Begin;
const sLONG DB4D_Index_OnOneField_Scalar_Byte = 11;
const sLONG DB4D_Index_OnOneField_Scalar_Long = 12;
const sLONG DB4D_Index_OnOneField_Scalar_Long8 = 13;
const sLONG DB4D_Index_OnOneField_Scalar_Real = 14;
const sLONG DB4D_Index_OnOneField_Scalar_Bool = 15;
const sLONG DB4D_Index_OnOneField_Scalar_Time = 16;
const sLONG DB4D_Index_OnOneField_Scalar_UUID = 17;
const sLONG DB4D_Index_OnOneField_Scalar_Range_End = DB4D_Index_OnOneField_Scalar_UUID;


const VErrorDB4D	VE_DB4D_INVALID_TYPESEL	= MAKE_VERROR(CDB4DManager::Component_Type, 1300); 
const VErrorDB4D	VE_DB4D_ARRAY_TOO_BIG	= MAKE_VERROR(CDB4DManager::Component_Type, 1301); 
const VErrorDB4D	VE_DB4D_ARRAY_SIZE_DOES_NOT_MATCH	= MAKE_VERROR(CDB4DManager::Component_Type, 1302); 
const VErrorDB4D	VE_DB4D_INVALID_SELECTION_ID	= MAKE_VERROR(CDB4DManager::Component_Type, 1303); 
const VErrorDB4D	VE_DB4D_INVALID_SELECTION_PART	= MAKE_VERROR(CDB4DManager::Component_Type, 1304); 

#if !WCHAR_IS_UNICHAR
typedef const wchar_t* ConstUniCharPtr;
#else
typedef const UniChar* ConstUniCharPtr;
#endif

typedef ConstUniCharPtr ArrayOfConstUniCharPtr[];


class CompareLessVStringStrict
{
public:
	inline bool operator () (const VString& s1, const VString& s2) const
	{
		return s1.CompareToString(s2, true) == CR_SMALLER;
	};
};

struct CoupleCharLong
{
	const char* pChar;
	sLONG value;
};

typedef CoupleCharLong CoupleCharLongArray[];

class StrHashTable
{
	public:
		StrHashTable(bool diac = true)
		{
			fIsDiacritical = diac;
		}

		StrHashTable(const CoupleCharLong* pData, bool diac = true);

		sLONG GetValue(const VString& valuename) const;
		inline sLONG operator [] (const VString& valuename) const
		{
			return GetValue(valuename);
		}

	protected:
		VValueBag fHashTable;
		bool fIsDiacritical;
};

class CompareLessVStringNoIntl
{
public:
	bool operator () (const VString& s1, const VString& s2) const;
};


class ENTITY_API Enumeration
{
	public:
		Enumeration(const ConstUniCharPtr* from);
		Enumeration(const CoupleCharLong* from);

		Enumeration()
		{
		}

		Enumeration& operator = (const ConstUniCharPtr* from);

		void From(const ConstUniCharPtr* from);
		void From(const CoupleCharLong* from);

		sLONG operator [] (const VString& EnumName) const;

		const VString& operator [] (sLONG EnumID) const;

	protected:
		typedef map<VString, sLONG, CompareLessVStringNoIntl> EnumMap;
		typedef map<sLONG, VString> EnumIDMap;

		EnumMap fEnums;
		EnumIDMap fEnumIDs;

		static VString sEmptyString;


};

#if debug_nonmemmgr
inline void *GetTempMem(VSize inNbBytes, Boolean ForAnObject, sLONG inTag) { return malloc(inNbBytes); };
inline void FreeTempMem(void *inBlock) { free(inBlock); };
inline VSize GetTempMemSize( const void *inBlock) { return 200; };
#else
#if oldtempmem
inline void *GetTempMem(VSize inNbBytes, Boolean ForAnObject, sLONG inTag) { return VDBMgr::GetManager()->GetTempMemMgr()->NewPtr((inNbBytes+31) & -32, ForAnObject, inTag); };
inline void FreeTempMem(void *inBlock) { return VDBMgr::GetManager()->GetTempMemMgr()->DisposePtr( inBlock); };
inline VSize GetTempMemSize( const void *inBlock) { return VDBMgr::GetManager()->GetTempMemMgr()->GetPtrSize( (const VPtr)inBlock); };
#else
inline void *GetTempMem(VSize inNbBytes, Boolean ForAnObject, sLONG inTag) { return VObject::GetMainMemMgr()->NewPtr((inNbBytes+31) & -32, ForAnObject, inTag); };
inline void FreeTempMem(void *inBlock) { return VObject::GetMainMemMgr()->DisposePtr( inBlock); };
inline VSize GetTempMemSize( const void *inBlock) { return VObject::GetMainMemMgr()->GetPtrSize( (const VPtr)inBlock); };
#endif
#endif


inline uLONG DiffTime(uLONG t1, uLONG t2)
{
	if (t1 > t2)
		return t1-t2;
	else
		return t2-t1;
}


// -------------------------------------------------------------------




class VStringUTF8_info : public VString_info
{
public:
	VStringUTF8_info():VString_info( VK_STRING_UTF8)	{;}

	virtual	VValue*					Generate() const;
	virtual	VValue*					Generate(VBlob* inBlob) const;
	virtual	VValue*					LoadFromPtr( const void *inBackStore, bool inRefOnly) const;
	virtual void*					ByteSwapPtr( void *inBackStore, bool inFromNative) const;

	virtual CompareResult			CompareTwoPtrToData( const void* inPtrToValueData1, const void* inPtrToValueData2, Boolean inDiacritical = false) const;
	virtual CompareResult			CompareTwoPtrToDataBegining( const void* inPtrToValueData1, const void* inPtrToValueData2, Boolean inDiacritical = false) const;

	virtual CompareResult			CompareTwoPtrToData_Like( const void* inPtrToValueData1, const void* inPtrToValueData2, Boolean inDiacritical = false) const;
	virtual CompareResult			CompareTwoPtrToDataBegining_Like( const void* inPtrToValueData1, const void* inPtrToValueData2, Boolean inDiacritical = false) const;
	virtual	CompareResult			CompareTwoPtrToDataWithOptions( const void* inPtrToValueData1, const void* inPtrToValueData2, const VCompareOptions& inOptions) const;

	virtual VSize					GetSizeOfValueDataPtr( const void* inPtrToValueData) const;
	virtual	VSize					GetAvgSpace() const;
};


class VStringUTF8 : public VString
{
public:
	typedef VStringUTF8_info	InfoType;
	static const InfoType	sInfo;

	inline VStringUTF8()
	{
		fUTF8NbBytes = -1;
	}

	inline VStringUTF8( const VStringUTF8& inString):VString(inString)
	{
		fUTF8NbBytes = -1;
	}

	/*
	virtual ~VStringUTF8()
	{
	}
	*/

	virtual	const VValueInfo*	GetValueInfo() const;

	virtual	void*				LoadFromPtr( const void* inData, Boolean inRefOnly = false);
	virtual	void*				WriteToPtr( void* ioData, Boolean inRefOnly = false, VSize inMax = 0) const;
	virtual	VSize				GetSpace(VSize inMax = 0) const;

	virtual	CompareResult		CompareToSameKindPtr( const void* inPtrToValueData, Boolean inDiacritical = false) const;
	virtual	CompareResult		Swap_CompareToSameKindPtr( const void* inPtrToValueData, Boolean inDiacritical = false) const;
	virtual	CompareResult		CompareBeginingToSameKindPtr( const void* inPtrToValueData, Boolean inDiacritical = false) const;
	virtual	CompareResult		IsTheBeginingToSameKindPtr( const void* inPtrToValueData, Boolean inDiacritical = false) const;
	virtual	Boolean				EqualToSameKindPtr( const void* inPtrToValueData, Boolean inDiacritical = false) const;

	virtual	CompareResult			CompareToSameKindPtr_Like( const void* inPtrToValueData, Boolean inDiacritical = false) const;
	virtual	CompareResult			Swap_CompareToSameKindPtr_Like( const void* inPtrToValueData, Boolean inDiacritical = false) const;
	virtual CompareResult			CompareBeginingToSameKindPtr_Like( const void* inPtrToValueData, Boolean inDiacritical = false) const;
	virtual CompareResult			IsTheBeginingToSameKindPtr_Like( const void* inPtrToValueData, Boolean inDiacritical = false) const;
	virtual Boolean					EqualToSameKindPtr_Like( const void* inPtrToValueData, Boolean inDiacritical = false) const;
	virtual Boolean					Swap_EqualToSameKindPtr_Like( const void* inPtrToValueData, Boolean inDiacritical = false) const;

	virtual	CompareResult		CompareToSameKindPtrWithOptions( const void* inPtrToValueData, const VCompareOptions& inOptions) const;
	virtual bool				EqualToSameKindPtrWithOptions( const void* inPtrToValueData, const VCompareOptions& inOptions) const;
	virtual	CompareResult		Swap_CompareToSameKindPtrWithOptions( const void* inPtrToValueData, const VCompareOptions& inOptions) const;

	virtual	VStringUTF8*				Clone() const;

protected:
	//void _AdjusteUTF8() const;

	mutable sLONG fUTF8NbBytes;
};




// -------------------------------------------------------------------

// definition ici de toutes les structures stockees sur disque

#if _WIN32 || __GNUC__
	#pragma pack( push, 4 )
#else
	#pragma options align = 4
#endif

struct CritereDISK
{
	VUUIDBuffer ID;
	UniChar nom[kMaxFieldNameNbWords];
	DataAddr4D ExtraAddr;
	sLONG ReUsedNumber;
	sLONG UnusedLong1;
	DataAddr4D UnusedAddr2;
	sLONG ExtraLen;
	sLONG LimitingLength;
	uCHAR fNeverNull;
	uCHAR fDefaultValue;
	uCHAR fAutoGenerate;
	uCHAR fStoreAsUTF8;
	sLONG typ;
	sLONG fStamp;
	uCHAR not_null;
	uCHAR unique;
	uCHAR fOuterData;
	uCHAR fTextIsWithStyle;
	uBOOL fHideInRest;		// previously fInvisible, now fInvisible is stored in the field extra-properties
	uBOOL fAutoSeq;

	void SwapBytes();
};



struct SegDataDisk2
{
	DataAddr4D finfic;
	DataAddr4D limitseg;
	DataAddr4D AddPagePrim;
	sLONG sizeblockseg;
	sLONG ratio;

	void SwapBytes();
};


struct BaseHeader
{
	uLONG tag;
	sLONG lastoper;
	sLONG lastparm;
	sWORD nbsegdata;
	uBOOL IDisNotAnID;
	uBOOL filler1;
	uLONG8 VersionNumber;
	//VUUIDBuffer ID; // was an UUID now is an addr of an addrtable for many extra elements
	DataAddr4D ExtraElements_addrtabaddr;
	sLONG nbExtraElements;
	sLONG filler2;

	SegDataDisk2 seginfo;
	DataAddr4D DataTables_addrtabaddr;
	DataAddr4D addrmultisegheader;
	sLONG lenmultisegheader;
	sLONG filfil;
	sLONG fStamp;
	sLONG8 lastaction;
	sLONG countlog;
	sLONG LastFlushRandomStamp;

	sLONG nbDataTable;

	uBOOL doischangerfilfil;
	uBOOL backupmatchlog;
	uBOOL IsAStruct;
	uBOOL WithSeparateIndexSegment;

	DataAddr4D TableDef_debuttrou;
	DataAddr4D TableDef_addrtabaddr;
	sLONG nbTableDef;

	sLONG nbRelations;
	DataAddr4D Relations_debuttrou;
	DataAddr4D Relations_addrtabaddr;

	sLONG nbSeqNum;
	DataAddr4D SeqNum_debuttrou;
	DataAddr4D SeqNum_addrtabaddr;

	sLONG nbIndexDef;
	DataAddr4D IndexDef_debuttrou;
	DataAddr4D IndexDef_addrtabaddr;

	sLONG nbIndexDefInStruct;
	DataAddr4D IndexDefInStruct_debuttrou;
	DataAddr4D IndexDefInStruct_addrtabaddr;

	DataAddr4D ExtraAddr;
	sLONG ExtraLen;

	DataAddr4D DataTables_debuttrou;

	sLONG countFlush;
	uLONG dialect;
	CollatorOptions collatorOptions;	// uBYTE bit 1: withICU, bit 2: ignoreWildCharInMiddle

	void SwapBytes();
	VUUIDBuffer& GetID() 
	{
		return *((VUUIDBuffer*)&ExtraElements_addrtabaddr);
	}
};


typedef uLONG DataBaseObjectType;

typedef struct DataBaseObjectHeaderOnDisk
{
	DataBaseObjectType type;
	sLONG len;
	uLONG8 timestamp;
	uLONG checksum;
	sLONG pos;
	sLONG parent;
} DataBaseObjectHeaderOnDisk;


typedef struct RecordHeaderOnDisk
{
	DataBaseObjectType type;
	sLONG len;
	uLONG8 timestamp;
	uLONG checksum;
	sLONG pos;
	sLONG parent;
	sLONG nbfields;
} RecordHeaderOnDisk;



class ChampHeader
{
public:

	Boolean operator == (const ChampHeader& other) const { return false; };

	sLONG offset;
	sLONG typ;

	void SwapBytes();
};


#define maxTrigFile 2
#define maxPermFile 3

struct FichierDISK
{
	VUUIDBuffer ID;
	UniChar nom[kMaxTableNameNbWords];
	UniChar RecordName[kMaxTableNameNbWords];
	DataAddr4D ExtraAddr;
	DataAddr4D UnusedAddr1;
	DataAddr4D UnusedAddr2;
	sLONG ExtraLen;
	uBOOL fHideInRest;
	uBOOL fPreventLog;
	uBOOL fUnusedChar2;
	uBOOL fUnusedChar3;
	sLONG UnusedLen2;
	sLONG typ;
	sLONG nbcrit;
	sLONG fStamp;
	sLONG fSchemaID;
	sLONG Perms[maxPermFile];
	uBOOL fHasPrimaryKey; // previously : fInvisible;
	uBOOL fNotFullyDeleteRecords;
	uBYTE fKeepStamps;
	uBYTE fKeepSyncInfo;
	uBOOL AllowTrigger[maxTrigFile];

	void SwapBytes();
};


struct oldDataFileDISK
{
	sLONG nbfic;
	sLONG newnum;
	DataAddr4D debuttrou;
	DataAddr4D addrtabaddr;
	sLONG segref;
	DataAddr4D debutBlobTrou;
	DataAddr4D addrBlobtabaddr;
	sLONG nbBlob;
	VUUIDBuffer SeqNum_ID;

	void SwapBytes();
};


struct DataTableDISK
{
	sLONG nbfic;
	sLONG newnum;
	DataAddr4D debuttrou;
	DataAddr4D addrtabaddr;
	sLONG segref;
	DataAddr4D debutBlobTrou;
	DataAddr4D addrBlobtabaddr;
	sLONG nbBlob;
	VUUIDBuffer SeqNum_ID;
	VUUIDBuffer TableDefID;
	uLONG8 filler8;
	uLONG8 fLastRecordSync;
	sLONG debutTransRecTrou;
	sLONG debutTransBlobTrou;
	uBYTE fKeepStamps;
	uBYTE fPrimKeyState;
	uBYTE fillerByte2;
	uBYTE fillerByte3;
	uLONG fLastRecordStamp;

	void SwapBytes();
};



struct ClusterDISK
{
	DataAddr4D addrTabClust;
	DataAddr4D debuttrou;
	sLONG nbclust;

	void SwapBytes();
};



struct IndexHeaderDISK
{
	DataAddr4D addrprim;
	DataAddr4D nextfreepage;
	sLONG nbpage;
	sLONG lenNULLs;
	DataAddr4D AddrNULLs;
	uBYTE needRebuild;
	uBYTE cFiller1, cFiller2, cFiller3;
	sLONG filler2, filler3, filler4;

	void SwapBytes();
};


const sLONG kNbKeyParPageIndex=128;
const sLONG kNbKeysForScalar = 256;

struct BTreePageIndexDisk
{
	sLONG CurrentSizeInMem; // taille utilisee en memoire et sur disque pour le block
	sLONG TrueDataSize; // vrai taille des cles sans les trous
	sLONG nkeys;
	sLONG souspage0;
	sLONG off[kNbKeyParPageIndex];
	sLONG suite;

	void SwapBytes();
};


template <class Type>
class CleIndex
{
	public:
	
		void SwapBytes()
		{
			ByteSwap(&souspage);
			ByteSwap(&qui);
			ByteSwap(&cle);
		}
		
	
		sLONG souspage;
		sLONG qui;
		Type cle;
	
};

template<class Type>
void ByteSwap(CleIndex<Type>* ioValue)
{
	ioValue->SwapBytes();
}

template <class Type, sLONG MaxCles>
struct BtreePageData
{
	
	inline void SwapBytes()
	{
		ByteSwap(&nkeys);
		ByteSwap(&souspage0);
		ByteSwap(&cles[0], &cles[MaxCles]);
	}

	sLONG nkeys;
	sLONG souspage0;
	CleIndex<Type> cles[MaxCles];
	
};


struct HeaderBtreeDisk
{
	sLONG FirstPage;

	void SwapBytes();
};


/*
struct IndexInfoFromFieldOnDisk
{
	sLONG typindex;
	sLONG numfile;
	sLONG numfield;
	Boolean uniquekeys;

	void SwapBytes();
};
*/


struct FieldDef
{
	sLONG numfield;
	sLONG numfile;
	void SwapBytes();
};



struct AddrDBObj
{
	DataAddr4D addr;
	sLONG len;

	void SwapBytes();
};



struct AutoSeqSimpleOnDisk
{
	sLONG typ;
	sLONG filler;
	VUUIDBuffer ID;
	sLONG8 InitialValue;
	sLONG8 CurValue;

	void SwapBytes();
};



struct AutoSeqNoHoleOnDisk
{
	sLONG typ;
	sLONG filler;
	VUUIDBuffer ID;
	sLONG8 InitialValue;
	sLONG8 CurValue;
	sLONG nbHoles;

	void SwapBytes();
};


typedef struct oldRelOnDisk
{
	sLONG	type;
	VUUIDBuffer ID;
	VUUIDBuffer SourceField;
	VUUIDBuffer DestField;
	DataAddr4D ExtraAddr;
	sLONG ExtraLen;	
	uCHAR RelIsAuto;
	uCHAR RelState;
	uCHAR WithRefInt;
	uCHAR AutoDel;
	uCHAR ForeignKey;
	uCHAR MultipleFields;
	uCHAR CheckRefIntOnModify;
	uCHAR fil3;

	void SwapBytes();
} oldRelOnDisk;


typedef struct RelOnDisk
{
	VUUIDBuffer ID;
	VUUIDBuffer SourceField;
	VUUIDBuffer DestField;
	DataAddr4D ExtraAddr;
	sLONG ExtraLen;	
	uCHAR RelNto1IsAuto;
	uCHAR Rel1toNIsAuto;
	uCHAR RelState;
	uCHAR WithRefInt;
	uCHAR AutoDel;
	uCHAR ForeignKey;
	uCHAR MultipleFields;
	uCHAR CheckRefIntOnModify;

	void SwapBytes();
} RelOnDisk;


typedef struct RequestHeader
{
	sLONG tag;
	sLONG lenData;
	ContextID context;
	sLONG TypeReq;

	void SwapBytes();
} RequestHeader;



typedef struct ReturnRequestHeader
{
	sLONG tag;
	sLONG lenData;
	VError err;
	ContextID context;

	void SwapBytes();
} ReturnRequestHeader;


#if _WIN32 || __GNUC__
	#pragma pack( pop )
#else
	#pragma options align = power
#endif



// fin des definitions des structures stockees sur disque


class Base4D_NotOpened;

typedef vector<AddrDBObj> AddrDBObjCollection;

class TabAddrCache
{
	public:
		TabAddrCache(Base4D_NotOpened *owner, DataAddr4D primaddr, sLONG nbelem);

		void AddAddr(sLONG position, DataAddr4D addr, sLONG len);

		Boolean GetAddr(sLONG position, DataAddr4D& outAddr, sLONG& outLen);

	protected:
		void _InitIfNecessary();

		Base4D_NotOpened* fOwner;
		AddrDBObjCollection fAddrs;
		DataAddr4D fPrimAddr;
		sLONG fNbelems;
		Boolean fAllocationIsValid, fHasAlreadyTriedAllocate;
};

// ----------------------------------------------------------------------

template <VSize maxbytes>
class tempBuffer
{
	public:
		tempBuffer()
		{
			fPtr = &fBuffer[0];
		}


		tempBuffer(VSize nbbytes, sLONG inTag = 0)
		{
			if (nbbytes > maxbytes)
			{
				fPtr = (char*)VObject::GetMainMemMgr()->Malloc(nbbytes, false, inTag);
			}
			else
				fPtr = &fBuffer[0];
		}

		~tempBuffer()
		{
			if (fPtr != nil && fPtr != &fBuffer[0])
				VObject::GetMainMemMgr()->Free(fPtr);
		}

		bool ReSize(VSize nbbytes, sLONG inTag = 0)
		{
			if (fPtr != nil && fPtr != &fBuffer[0])
				VObject::GetMainMemMgr()->Free(fPtr);
			if (nbbytes > maxbytes)
			{
				fPtr = (char*)VObject::GetMainMemMgr()->Malloc(nbbytes, false, inTag);
			}
			else
				fPtr = &fBuffer[0];

			return fPtr != nil;
		}

		const char* GetPtr() const
		{
			return fPtr;
		}

		char* GetPtr()
		{
			return fPtr;
		}

	protected:
		char fBuffer[maxbytes];
		char* fPtr;
};


class Utf8ToUtf16
{
	public:
		Utf8ToUtf16(const void* utf8Ptr)
		{
			sLONG nbbytes = *((sLONG*)utf8Ptr);
			fBuffer.FromBlock(((char*)utf8Ptr)+4, nbbytes, VTC_UTF_8);
		}

		Utf8ToUtf16(const void* utf8Ptr, sLONG inNbBytes)
		{
			sLONG nbbytes = inNbBytes;
			fBuffer.FromBlock(utf8Ptr, nbbytes, VTC_UTF_8);
		}

		const UniChar* GetPtr() const
		{
			return fBuffer.GetCPointer();
		}

		sLONG GetLength() const
		{
			return fBuffer.GetLength();
		}

	protected:
		VStr<128> fBuffer;

};


class Utf16toUtf8
{
	public:
		Utf16toUtf8(const UniChar* utf16Ptr, sLONG len)
		{
			VIndex nbbytesconsumed;
			VSize nbbytesproduced = 0;
			VTextConverters::Get()->GetUTF8FromUnicodeConverter()->Convert(utf16Ptr, len, &nbbytesconsumed, (void*)fBuffer.GetPtr(), 256, &nbbytesproduced);
			if (nbbytesproduced >= 256)
			{
				if (fBuffer.ReSize(len*4))
				{
					VTextConverters::Get()->GetUTF8FromUnicodeConverter()->Convert(utf16Ptr, len, &nbbytesconsumed, (void*)fBuffer.GetPtr(), len*4, &nbbytesproduced);
				}
			}
			fNbBytes = (sLONG)nbbytesproduced;
		}

		sLONG GetNbBytes() const
		{
			return fNbBytes;
		}

		const char* GetPtr() const
		{
			return (const char*)fBuffer.GetPtr();
		}

	protected:
		tempBuffer<256> fBuffer;
		sLONG fNbBytes;
};


// ----------------------------------------------------------------------




class vxSyncEvent : public VSyncEvent
{
	public:
		inline bool Wait() { return Lock(); };

		inline bool	Wait( sLONG inTimeoutMilliseconds) { return Lock(inTimeoutMilliseconds); };

		// bool Unlock();  // pas besoin de changer le nom, il est explicite

		inline bool ResetAndLock() { return Reset(); };	
};

// ----------------------------------------------------------------------


class SimpleTextProgressIndicator : public VDB4DProgressIndicator
{
public :
	virtual bool	DoProgress();
	virtual void	DoBeginSession(sLONG inSessionNumber);
	virtual void	DoEndSession(sLONG inSessionNumber);
	virtual ~SimpleTextProgressIndicator();

protected:
	uLONG fLastTime;
};



// ----------------------------------------------------------------------





typedef sWORD TransactionIndex;
typedef sWORD BaseFlushInfoIndex;

template<class T> void ByteSwapCollectionElem( T *ioValue)	{ioValue->SwapBytes();}

template<class ForwardIterator>
void ByteSwapCollection( ForwardIterator inBegin, ForwardIterator inEnd)
{
	for( ; inBegin != inEnd ; ++inBegin)
		ByteSwapCollectionElem( &*inBegin); // syntax qui semble etrange mais * peut etre overloaded 
}

template<class T>
void ByteSwapCollection( T *inFirstValue, VIndex inNbElems)
{
	ByteSwapCollection( inFirstValue, inFirstValue + inNbElems);
}

template<> inline void ByteSwapCollectionElem( AddrDBObj *ioValue)	{ioValue->SwapBytes();}

typedef VValueSingle* ValPtr;
typedef const VValueSingle* ConstValPtr;

extern VStr4 ext_Struct;
extern VStr4 ext_Data;
extern VStr4 ext_Temp;
extern VStr4 ext_Pref;

typedef enum { t_Obj4D=0L, t_ObjCache, t_Base4D, t_Datafile, t_bittable, t_addrtab, t_addrdefictable,
			 				 t_ficheondisk, t_treeinmem, t_treedeficinmem, t_segdata, t_addrindtable, t_btreepageindex,
							 t_IndexHeaderBTree, t_addrdeblobtable, t_blob4d, t_treedeblobinmem,
							 t_addrdeselclustertable, t_treedeselclusterinmem, t_segdatapagesec, t_btreepagefixsizeindex,
							 t_IndexHeaderBTreeFixSize, t_locker, t_tabledef, t_relationdef, t_petitesel, t_longsel, t_bitsel, t_indexdef,
							 t_fullydeletedrecinfo, t_constvectorsel} typobjcache;

							 
							 
const uLONG ALLFF = 0xFFFFFFFF;

const sLONG MaxErrSeq=1;

const sLONG kFinDesTrou=-2000000000;
const sLONG kDebutDesTrou=-2000000003;

const uBYTE kDefaultKeepStamp = 1;

class Base4D;

class BaseTaskInfo;



#if 0
extern uBOOL cachemodif;

const uLONG xxmaxnum=0xD0010600;
const uLONG xxminnum=0xD0010600;

typedef double freal;

#endif

sLONG compteptr(const void *p,sLONG nb);

Boolean IsCurTaskFlushing(void); 
Boolean IsCurTaskFaisPlace(void); 
Boolean IsCurTaskFaisPlaceOrFlushing(void);

void SetCurTaskFlushing(Boolean x); 
void SetCurTaskFaisPlace(Boolean x); 


typedef VStr63 VName;

typedef Vx1ArrayOf<sLONG,10> NumFieldArray;

#if 0
class VName : public VString
{
	public:
		inline VName(void) { INITSTRMAX(63);Clear(); };
		inline VName(const char laconst[]){INITSTRMAX(63);FromAsciiCString((uCHAR *)laconst);};
		inline VName(const uCHAR laconst[]){INITSTRMAX(63);FromAsciiCString((uCHAR *)laconst);};
		inline VName(const VString& pStr){INITSTRMAX(63);VString::operator =(pStr);};
		inline VName(void *pStrPtr,sLONG pLen){INITSTRMAX(63);FromBytes(pStrPtr,pLen);};
		inline void operator = (const VName& pStr){VString::operator =(pStr);};
	protected:
		UniChar fLocStr63[63];
};
#endif

// specific for tables
Boolean IsValid4DTableName( const VString& name);
Boolean IsValid4DFieldName( const VString& inName);
VErrorDB4D BuildValid4DTableName( const VString& inName, VString& outValidName);
VErrorDB4D BuildValid4DFieldName( const VString& inName, VString& outValidName);
void GetUniqueName(const std::vector<VString>& inAlreadyUsedNames, const VString &inName, VString &outUniqueName, bool inWithEllipse, VIndex inMaxLength);

// for everything apart from tables
Boolean IsValid4DName(const VString& name);

/*
inline tHandle NouvHandle(sLONG size) { return((tHandle) vNewHandle(size)); };
inline tHandle nouvhandle(sLONG size) { return((tHandle) vNewHandle(size)); };
inline void EffHandle(void* h) { vDisposeHandle((VHandle)h); };
inline void effhandle(void* h) { vDisposeHandle((VHandle)h); };
inline void wlock(void* h) { vHLock((VHandle)h); };
inline void wunlock(void* h) { vHUnlock((VHandle)h); };
inline void wLock(void* h) { vHLock((VHandle)h); };
inline void wUnLock(void* h) { vHUnlock((VHandle)h); };
inline short SetHSize(void* h, sLONG size) { return vSetHandleSize((VHandle)h,size) ? 0 : kErr4DMemFull; };
inline sLONG GetHSize(void* h) { return(vGetHandleSize((VHandle)h)); };

inline void EffPtr(void* h) { vDisposePtr((tPtr)h); }
*/

#define vBlockMove CopyBlock

										/* -------------------------------------------------------- */

class IOccupable;

typedef set<const IOccupable*> OccupableSet;

class OccupableStack
{
	public:
		OccupableStack(VTaskID inTaskID);
		~OccupableStack();
		void Add(const IOccupable* inObject);
		void Remove(const IOccupable* inObject);

		inline void MarkForWaitMem()
		{
			fMarkedForWaitMem++;
#if debuglr
			if (fMarkedForWaitMem > 20)
			{
				xbox_assert(fMarkedForWaitMem <= 20);
			}
#endif
		}

		inline void UnMarkForWaitMem()
		{
#if debuglr
			if (fMarkedForWaitMem <= 0)
			{
				xbox_assert(fMarkedForWaitMem > 0);
			}
#endif
			fMarkedForWaitMem--;
		}

		inline sLONG GetCurMark()
		{
			return fMarkedForWaitMem;
		}

		inline void SetBackCurMark(sLONG oldcurmark)
		{
			fMarkedForWaitMem = oldcurmark;
		}

		inline void ResetCurMark()
		{
			fMarkedForWaitMem = 0;
		}

		inline bool IsMarkedForWaitMem() const
		{
			return (fMarkedForWaitMem > 0);
		}

		inline bool IsMarkedForWaitMemAtomic()
		{
			return (VInterlocked::AtomicGet(&fMarkedForWaitMem) > 0);
		}

		inline void UsedByGarbageCollector()
		{
			VInterlocked::Increment(&fMarkedByMemMgr);
		}

		inline void ReleasedByGarbageCollector()
		{
			VInterlocked::Decrement(&fMarkedByMemMgr);
		}

		inline bool IsUsedByGarbageCollector() const
		{
			return (fMarkedByMemMgr > 0);
		}

		inline void SaveStamp()
		{
			fSavedStamp = VInterlocked::AtomicGet(&fStamp);
		}

		inline bool StampHasChanged()
		{
			return fSavedStamp != VInterlocked::AtomicGet(&fStamp);
		}

		inline VTaskID GetTaskId() const
		{
			return fTaskID;
		}

		void PutObjectsInto(OccupableSet& OccupiedObjects);

		bool FindObjectInStack(const IOccupable* inObject); // used for debugging purpose

		sLONG fLocker;

	protected:
		typedef vector<const IOccupable*> occupevector;
		enum { kMaxElemInStack = 100 };

		occupevector fStack;
		sLONG fStamp;
		sLONG fSavedStamp;
		sLONG fMarkedForWaitMem;
		sLONG fMarkedByMemMgr;
		VString fName;
		VTaskID fTaskID;
};





class OccupablePool
{
public:
	OccupablePool();
	~OccupablePool();

	OccupableStack* GetStackForCurrentThread(); // doit etre appelee une fois par LockRead ou LockWrite

	void RemoveStack(VTaskID threadID);

	void WaitForMemoryManager(OccupableStack* curstack); // dois etre appelee avant chaque occupy
	void EndWaitForMemoryManager(OccupableStack* curstack); // dois etre appelee apres chaque occupy

	void StartGarbageCollection(); // dois etre appele par la routine de liberation de memoire (NeedsBytes)
	void EndGarbageCollection();

	void MemoryBarrier()
	{
		VInterlocked::Increment(&memorybarrier);
	}

	bool IsOccupied(const IOccupable* inObject);

	void SubWaitMemMgr(OccupableStack* curstack);

	/*
	bool TryToLockNonAtomic(VTaskID inID)
	{
		if (fLocker == 0 || fLocker == inID)
		{
			fLocker = inID;
			return true;
		}
		else
			return false;
	}

	void UnlockNonAtomic()
	{
		fLocker = 0;
	}
*/
	/*
	void IncStamp()
	{
		fGlobalStamp++;
	}
	 */

	bool FindObjectInStack(const IOccupable* inObject); // used for debugging purpose

	sLONG fWaitForMemMgr;

	void init();

protected: 
	//typedef vector<OccupableStack*> OccupableStackCollection;
	typedef map<VTaskID, OccupableStack*> OccupableStackCollection;
	OccupableStackCollection fAllStacks;
	OccupableSet fOccupiedObjects;
	VCriticalSection fMutex;
	vxSyncEvent* fWaitMemMgrSync;
	sLONG memorybarrier;
	VTaskDataKey fTaskDataKey;
	//sLONG fGlobalStamp;


};

extern OccupablePool gOccPool;

#if debug_more_granularity
#define Yield_Debug VTask::Yield()
#else
#define Yield_Debug
#endif




class IOccupable
{
	public:
		inline IOccupable()
		{
			//fNBOccupied = 0;
			fOccupyStamp = 0;
		};

		void Occupy(OccupableStack* curstack, bool withWaitMem) const;

		void Free(OccupableStack* curstack, bool withWaitMem) const;

		inline sLONG GetOccupiedStamp() const
		{
			return fOccupyStamp;
		}

		inline sLONG GetOccupiedStampBarrier() const
		{
			MemoryBarrier();
			return fOccupyStamp;
		}

		
		bool IsOccupied() const; // ne doit etre appelee que dans la phase de liberation de memoire quand toutes les OccupableStack sont figées
		/*
		inline Boolean IsOccupiedBarrier() const
		{
			MemoryBarrier();
			return fNBOccupied > 0;
		}
		*/


		inline void MemoryBarrier() const
		{
			VInterlocked::AtomicAdd(&fOccupyStamp, 0);
		}

	protected:
		//mutable sLONG fNBOccupied;
		mutable sLONG fOccupyStamp;
		static sLONG sNBActions;
};






class StOccupy
{ 
public:
	StOccupy( IOccupable* inSyncObject, OccupableStack* curstack, bool withWaitMem) : fSyncObject( inSyncObject), fStack(curstack)
	{ 
		fWithWaitMem = withWaitMem;
		fSyncObject->Occupy(curstack, withWaitMem); 
	}

	~StOccupy()												
	{ 
		fSyncObject->Free(fStack, fWithWaitMem); 
	}

private:
	OccupableStack* fStack;
	IOccupable*		fSyncObject;
	bool fWithWaitMem;
};




										/* -------------------------------------------------------- */


class ITreePurgeable
{
	public:
		inline ITreePurgeable()
		{
			fAllSubsFree = false;
		}

		inline bool AtLeastOneSub() const
		{
			return !fAllSubsFree;
		}

		inline void AllSubsAreFree()
		{
			fAllSubsFree = true;
		}

		inline void OneSubIsUsed()
		{
			fAllSubsFree = false;
		}

	protected:
		bool fAllSubsFree;

};


										/* -------------------------------------------------------- */




#define ATOMIC
#define FINATOMIC

#if debug

uBOOL IsValidHandle(void* hh);
uBOOL IsValidPtr(void* p);
uBOOL IsValidFastPtr(void* p);

uBOOL IsFieldValid(sLONG nf, sLONG nc, Base4D* bd = nil);
uBOOL IsFileValid(sLONG nf, Base4D* bd = nil);

uBOOL IsValidDiskAddr(DataAddr4D addr, Base4D* bd = nil);

#endif

#if debugoccupe_with_signature

class debugOccupeSignature
{
	public:
		sLONG fSignature;
		uLONG fTime;
		sLONG fTaskID;
};

class Obj4D;

typedef vector<debugOccupeSignature> debugOccupeSignatureCollection;
typedef map<const Obj4D*, debugOccupeSignatureCollection> debugMapOfOccupe;

#endif




#if debugLeaksAll
extern uBOOL debug_candumpleaksAll;
extern ENTITY_API uBOOL debug_canRegisterLeaksAll;

class ENTITY_API IDebugRefCountable : public IRefCountable
{
public:
	IDebugRefCountable();
	virtual ~IDebugRefCountable();
	virtual	sLONG Retain(const char* DebugInfo = 0) const;
	virtual	sLONG Release(const char* DebugInfo = 0) const;

	virtual bool OKToTrackDebug() const
	{
		return false;
	}

	virtual void GetDebugInfo(VString& outText) const
	{
		outText = "object : "+ToString((VSize)this);
	}

	typedef set<VStackCrawl> StackCrawlCollection;
	typedef map<const IDebugRefCountable*, StackCrawlCollection> mapOfRefCountables;

	static void DumpStackCrawls();
	static void DumpStackCrawls(VString& outText);
	static void DumpStackCrawls(VStream& outStream);

protected:

	static mapOfRefCountables objs ;
	static VCriticalSection objsMutex;
	static void RegisterStackCrawl(const IDebugRefCountable* obj);
	static void UnRegisterStackCrawl(const IDebugRefCountable* obj);

};





static void xStartRecordingMemoryLeaksAll()
{
	debug_canRegisterLeaksAll = true;
}

static void xStopRecordingMemoryLeaksAll()
{
	debug_canRegisterLeaksAll = false;
}

#else

typedef IRefCountable IDebugRefCountable;

#endif




class BaseFlushInfo;

// ------------------------------------------------------------------


class IObjToFree
{
	public:
		inline IObjToFree()
		{
			fFreeMemRequestCount = 0;
			fNeededMem = 0;
			fNeededAllocationBlockNumber = -2;
		}

		virtual bool FreeMem(sLONG allocationBlockNumber, VSize combien, VSize& outSizeFreed) = 0;

		void RemoveFromCache();
		void PutInCache();

		inline void RequestMem(sLONG allocationBlockNumber, VSize inSize)
		{
			if (inSize > fNeededMem)
				fNeededMem = inSize;
			if (fNeededAllocationBlockNumber == -2 || fNeededAllocationBlockNumber == allocationBlockNumber)
				fNeededAllocationBlockNumber = allocationBlockNumber;
			else
				fNeededAllocationBlockNumber = -1;
			fFreeMemRequestCount++;
		}

		inline void ClearMemRequest()
		{
			fFreeMemRequestCount = 0;
			fNeededMem = 0;
			fNeededAllocationBlockNumber = -2;
		}

		bool CheckForMemRequest(VSize minBlockSize = 0);


	protected:
		VSize fNeededMem;
		sLONG fFreeMemRequestCount;
		sLONG fNeededAllocationBlockNumber;


};

VError ThrowJSExceptionAsError(VJSContext& context, XBOX::VJSException& excep);

ENTITY_API VError ThrowBaseError(VError inErrCode, ActionDB4D inAction = noaction );
ENTITY_API VError ThrowBaseError(VError inErrCode, const VString& param1 );
ENTITY_API VError ThrowBaseError(VError inErrCode, const VString& param1, const VString& param2);

const DataAddr4D kEndProtectedArea = 256 + (8192 * sizeof(DataAddr4D)) + (8192 * sizeof(sWORD));

inline bool AllowedAddress(DataAddr4D ou)
{
	if (ou < kEndProtectedArea && ou != -1)
		return false;
	else
		return true;
}

class IObjToFlush : public IDebugRefCountable, public IOccupable
{
public:
	IObjToFlush()
	{
		addrobj=0;
		fModified = false;
		fSaving = false;
		// fPourDeleteFromFlush = false;
		fFlushInfo = 0;
		fChangingAddr = false;
		fInProtectedArea = false;
		fValidForWrite = true;
	}

	//virtual ~IObjToFlush();

#if debugTabAddr

	virtual void setaddr(DataAddr4D addr, bool checknew = true) 
	{ 
		if (checknew)
			xbox_assert(addrobj == 0); 
		addrobj=addr; 
	};
#else

	inline void ResetAddr()
	{
		addrobj = 0;
	}

	inline void setaddr(DataAddr4D addr, bool checknew = true) 
	{ 
#if debuglr
		if (checknew)
			xbox_assert(addrobj == 0); 
#endif
		if (addrobj == 0 && addr == 0)
			addrobj = addr;
		else
		{
			addrobj=addr;
			if (!IsAddrAllowed())
			{
				xbox_assert(IsAddrAllowed()); // break here
				ThrowBaseError(VE_DB4D_DATABASEISWRITEPROTECTED, noaction);
			}
		}
	};

#endif

	void RemoveFromFlush();

	inline DataAddr4D getaddr(void) const { return(addrobj); };

	inline uBOOL modifie(void) const { return fModified || fSaving; };
	inline uBOOL modifieonly(void) const { return fModified; };

	virtual void setmodif(uBOOL xismodif, Base4D* bd, BaseTaskInfo* context);
	virtual void ChangeAddr(DataAddr4D addr, Base4D* bd, BaseTaskInfo* context);
	virtual bool SaveObj(VSize& outSizeSaved) = 0;

	virtual VError LibereEspaceDisk(OccupableStack* curstack, VDB4DProgressIndicator* InProgress = nil);

	virtual bool MustCheckOccupiedStamp() const
	{
		return true; // default for object that use the old algorithm to check if the object is valid while flushing
	}

	inline uBOOL AccesModifOK(BaseTaskInfo* context) { return true; };


	uBOOL SupAddr(IObjToFlush* obj2) const;
	uBOOL SupAddr(DataAddr4D xaddr) const;

	CompareResult CompAddr(IObjToFlush* obj2) const;
	CompareResult CompAddr(DataAddr4D xaddr) const;

	BaseFlushInfo* GetBaseFlushInfo(void) ;

	/*
	inline void PrepareToRemoveFromFlush()
	{
	fPourDeleteFromFlush = true;
	}
	*/

	inline void ClearModifFlag()
	{
		fModified = false;
		GetOccupiedStampBarrier();
	}

	inline BaseFlushInfoIndex SwitchToSaving()
	{
		fSaving = true;
		BaseFlushInfoIndex result = fFlushInfo;
		IObjToFlush::setmodif(false, nil, nil);
		return result;
	}

	inline void FinishSaving()
	{
		fSaving = false;
	}

	void FailedSaving(BaseFlushInfoIndex xflushinfo);

	virtual void clearfromflush() // code temporaire
	{
		Release();
	}

	inline void SetInProtectedArea()
	{
		fInProtectedArea = true;
	}

	inline uBOOL IsInProtectedArea() const
	{
		return fInProtectedArea;
	}

	inline bool IsAddrAllowed() const
	{
		if (fInProtectedArea)
			return true;
		else
			return AllowedAddress(addrobj);
	}

	inline bool TryWriteLock()
	{
		return fSaveMutex.TryToLock();
	}

	inline void WriteLock()
	{
		fSaveMutex.Lock();
	}

	inline void WriteUnLock()
	{
		fSaveMutex.Unlock();
	}

	inline void SetValidForWrite(bool value)
	{
		fValidForWrite = value;
	}

	inline bool IsValidForWrite() const
	{
		return fValidForWrite;
	}

#if debuglr == 3
	virtual	sLONG	Release(const char* DebugInfo = 0) const
	{
		if (GetRefCount() == 2 && (fModified && !fChangingAddr))
		{
			xbox_assert(!(GetRefCount() == 2 && fModified));
		}
		return IDebugRefCountable::Release(DebugInfo);
	}
#endif


#if autocheckobj
	virtual uBOOL CheckObjOnDisk(void) { return true; };
#endif

protected:
	DataAddr4D addrobj;
	BaseFlushInfoIndex fFlushInfo;
	uBOOL fModified, fSaving, fChangingAddr;
	uBOOL fInProtectedArea, fValidForWrite;
	mutable VCriticalSection fSaveMutex;
	//uBOOL fPourDeleteFromFlush;


};


						// ----------------------------------------------------------


enum { ObjBasic=0, Obj_Cache, Obj_tabaddr, Obj_treeinmem, Obj_Fiche, Obj_pageindex, Obj_headerindex, Obj_Blob, Obj_cluster, Obj_Base4D,
				Obj_bittab, Obj_fichier, Obj_BtreeFlush, Obj_BtreeCache, Obj_FieldNuplet, Obj_ObjCacheArrayLongFix, Obj_tabaddrheader
				};

class ENTITY_API Obj4D : public VObject
{
	public:
#if debuglrWithTypObj
		Obj4D() {  typobj=t_Obj4D; };
#endif
#if debugoccupe
		virtual ~Obj4D();
#endif

		uBOOL occupe(Boolean ForceWaitEvenDringFlush = false, sLONG signature = 0) const;
		uBOOL IsOkToOccupe(sLONG signature = 0) const;
		void libere(sLONG signature = 0) const;
		sLONG GetCountOccupe(void) const { return fOccupe.GetUseCount(); };

		virtual bool IsOkToFreeMem() const
		{
			return IsOkToOccupe();
		}

		virtual void ReleaseFreeMem() const
		{
			libere();
		}

#if debugoccupe_with_signature
		void MarkOccupe(sLONG signature) const;
#endif
		
		virtual void Kill() { delete this; };

		#if debuglrWithTypObj
		typobjcache GetType() { return typobj;};
		#endif
		
		#if VERSIONDEBUG
		virtual uBOOL CheckObjInMem() { return true; };
		inline void CheckUseCount() { xbox_assert(fOccupe.GetUseCount() < 20); };
		#else
		inline void CheckUseCount() {; };

		#endif
		
	protected:
		mutable VNonVirtualCriticalSection fOccupe;
		
#if debuglrWithTypObj
		typobjcache typobj;
#endif
	
#if debugoccupe_with_signature
		static VCriticalSection sDebugMapOfOccupeMutext;
		static debugMapOfOccupe sDebugMapOfOccupe;
#endif

};


class VObjLock
{ 
public:
	VObjLock (const Obj4D* inSyncObject) { fSyncObject = inSyncObject; fSyncObject->occupe(); };
	~VObjLock () { fSyncObject->libere(); }; // non virtual on purpose

protected:
	const Obj4D*	fSyncObject;
};


class VObjLockPotentiel
{ 
public:
	VObjLockPotentiel (const Obj4D* inSyncObject, Boolean aveclock) 
	{ 
		fSyncObject = inSyncObject;
		if (aveclock)
		{
			fSyncObject->occupe(); 
		}
		fAvecLock = aveclock;
	};
	
	~VObjLockPotentiel () 
	{ 
		if (fAvecLock)
		{
			fSyncObject->libere(); 
		}
	}; // non virtual on purpose

protected:
	const Obj4D*	fSyncObject;
	Boolean fAvecLock;
};


typedef enum
{
	objInBase_Table = 1,
	objInBase_Field,
	objInBase_Relation
} TypeObj4DInBase;


class Obj4DContainer
{
public:
	Obj4D* fObject;
	TypeObj4DInBase fType;
};

class Transaction;
class AddrTableHeader;
class BaseFlushInfo;

class ObjInCacheMemory : public Obj4D
{
	public:
	
		void	*operator new(size_t size);
		void	*operator new(size_t size, void *inAddr) { UNUSED(size); return inAddr;}
		void	operator delete (void *p );
		void	operator delete (void *p, void* p2 ) { UNUSED(p); UNUSED(p2); };
};


class ObjCache;

class ObjAlmostInCache : public ObjInCacheMemory, public IObjToFlush
{
public:
	enum { isModifFlag = 0, PourDeleteCacheFlag = 1, ForceDeleteFlag = 2, DansCacheFlag = 3, DansTransFlag = 4, NewInTransFlag = 5, 
			PourDeleteFlushFlag = 6, DoNotKeepInCacheFlag = 7};

	ObjAlmostInCache(Boolean occupeOnInit = true );

	virtual ~ObjAlmostInCache();
	virtual ObjCache* GetObjCache() { return nil; };

	inline Boolean IsFlag(sWORD flagnum) const { return (FlagMaskAnd[flagnum] & flags) != 0; };
	inline void SetFlag(sWORD flagnum) { flags = flags | FlagMaskSet[flagnum]; };
	inline void ClearFlag(sWORD flagnum) { flags = flags & FlagMaskClear[flagnum]; };

	virtual uBOOL okdel(void);
	
	virtual sLONG saveobj(void)
	{
		return 0;
	}

	virtual bool SaveObj(VSize& outSizeSaved)
	{
		sLONG tot = saveobj();
		outSizeSaved = tot;
		if (tot >= 2)
			return true;
		else
			return false;
	}

	virtual void clearfromflush() // code temporaire
	{
		// rien
	}

	//virtual VError LibereEspaceDisk(VDB4DProgressIndicator* InProgress);
	
	virtual sLONG liberemem(sLONG allocationBlockNumber, sLONG combien=-1, uBOOL tout=false);

	//inline uBOOL AccesModifOK(BaseTaskInfo* context) { return true; };

	inline void FreeOldOne(void) {  };

	/*
	uBOOL SupAddr(ObjAlmostInCache* obj2) const;
	uBOOL SupAddr(DataAddr4D xaddr) const;

	CompareResult CompAddr(ObjAlmostInCache* obj2) const;
	CompareResult CompAddr(DataAddr4D xaddr) const;

	BaseFlushInfo* GetBaseFlushInfo(void) ;
	*/
	inline void SetDansCache(uBOOL b) { if (b) SetFlag(DansCacheFlag); else ClearFlag(DansCacheFlag); };
	inline Boolean IsDansCache() const { return IsFlag(DansCacheFlag); };

	inline void SetInTrans(uBOOL b = true) { xbox_assert(!b || (!modifie() && !IsDansCache())); if (b) SetFlag(DansTransFlag); else ClearFlag(DansTransFlag); };
	inline Boolean InTrans() const { return IsFlag(DansTransFlag); };

	inline void SetNewInTrans(uBOOL b = true) {/* xbox_assert(!modifie() && !IsDansCache()); */ if (b) SetFlag(NewInTransFlag); else ClearFlag(NewInTransFlag); };
	inline Boolean IsNewInTrans() const { return IsFlag(NewInTransFlag); };

#if autocheckobj
	virtual uBOOL CheckObjOnDisk(void) { return true; };
#endif

	static void InitMasks();

protected:
	/*
	DataAddr4D addrobj;
	BaseFlushInfoIndex fFlushInfo;
	*/

	uWORD flags;
	static uWORD FlagMaskAnd[16];
	static uWORD FlagMaskSet[16];
	static uWORD FlagMaskClear[16];

};


class ObjCache : public ObjAlmostInCache
{
	public:
		enum { BaseAccess = 2048, RecordAccess = 8, PageIndexAccess = 64, TableAddressAccess = 256, 
					 BittableAccess = 256, HeaderAccess = 1024, SelectionAccess = 128, TreeInMemAccess = 65, TransactionAccess = 2048+256,
					 StructElemDefAccess = 1, TableDefAccess = 1, RelationDefAccess = 1, IndexDefAccess = 1, LockEntityTreeAcces = 1, DoNotPutInCacheAcces = -1};

		ObjCache(sWORD DefaultAccess, Boolean occupeOnInit = true);
		virtual ~ObjCache();

		virtual ObjCache* GetObjCache() { return this; };
		
		void AddToCache(sWORD Access);

		inline void SetNbAcces(uWORD nb) { nbacces = nb; };
		inline uWORD GetNbAcces(void)const { return(nbacces); };
		virtual void IncNbAcces(void);
		inline uBOOL EgalAcces(uWORD nbac) { return( (nbacces>>5) == (nbac>>5) ); };
		inline uBOOL EgalAccesPtr(uWORD nbac, void* p) { return( ((nbacces>>5) == (nbac>>5)) && ((void*)this == p) ); };
		inline uBOOL SupAcces(ObjCache* obj2) { 
												if ((nbacces>>5) == (obj2->nbacces>>5))
												{
													return( (void*)this > (void*)obj2 );
												}
												else
												{
													return( (nbacces>>5) > (obj2->nbacces>>5) );
												}
											};
		inline uBOOL SupAccesPtr(uWORD nbac, void* p) { 
														if ((nbacces>>5) == (nbac>>5))
														{
															return( (void*)this > p );
														}
														else
														{
															return( (nbacces>>5) > (nbac>>5) );
														}
													};
		inline uBOOL SupAcces(uWORD nbac) { return( (nbacces>>5) > (nbac>>5) ); };
		/* inline uBOOL InfAcces(sLONG nbac) { return( (nbacces>>5) <= (nbac>>5) ); }; */

		inline void PourDeleteCache(uBOOL b) { if (b) SetFlag(PourDeleteCacheFlag); else ClearFlag(PourDeleteCacheFlag); };
		inline uBOOL IsPourDeleteCache(void) { return(IsFlag(PourDeleteCacheFlag)); };

		inline void PourDeleteFlush(uBOOL b) { if (b) SetFlag(PourDeleteFlushFlag); else ClearFlag(PourDeleteFlushFlag); };
		inline uBOOL IsPourDeleteFlush(void) { return(IsFlag(PourDeleteFlushFlag)); };

		inline void ForceDelete(void) { SetFlag(ForceDeleteFlag); };
		inline uBOOL ForceDeleted(void) { return(IsFlag(ForceDeleteFlag)); };
	
		virtual uBOOL okdel(void);

		virtual uBOOL okToLiberemem()
		{
			return (!modifie()) && (GetCountOccupe() == 1);
		}

		virtual uBOOL isBigObject()
		{
			return false;
		};

	protected:
		sWORD nbacces;

};


//====================================================================================================================

#if debugoccupe

class DebugOccupeTask : public VTask
{
public:
	DebugOccupeTask( ):VTask(nil,0,eTaskStylePreemptive,NULL) {timetocheck = 200;checkinterval = 500;}

protected:
	virtual Boolean DoRun();

	sLONG timetocheck, checkinterval;
};

#endif


//====================================================================================================================
class ObjCacheInTree;

#if debugObjInTree_Strong
typedef set<ObjCacheInTree*> debug_setof_ObjCacheInTree;
#endif


class ObjCacheInTree : public ObjCache
{
	public:
		inline ObjCacheInTree(sWORD defaultAccess): ObjCache(defaultAccess) 
		{ 	
			parent = nil; 
			nbelemtab = -1;
			posdansparent = -1000;
#if debugObjInTree_Strong
			if (sCheckDisabled == 0) 
				Debug_CheckParentEnfantsRelations();
			RegisterForDebug(this);
#endif
		}

#if debugObjInTree_Strong
		virtual ~ObjCacheInTree()
		{
			if (sCheckDisabled == 0) 
			{
				Debug_CheckParentEnfantsRelations();
			}
			Debug_CheckForDelete(this);
			UnRegisterForDebug(this);
		}
#endif

		inline ObjCacheInTree* GetParent(void) { return(parent); };
		void SetParent(ObjCacheInTree *par, sLONG pos);
		virtual sLONG FindPosOfChild(ObjCacheInTree* obj);
		virtual void DelFromParent(sLONG n, ObjCacheInTree* enfant) { };
		sLONG GetNbElemTab(void);
		virtual void RecalcNbElemTab() = 0;
		inline void IncPosParent(sLONG QtToAdd = 1) { posdansparent = posdansparent + QtToAdd; };
		inline void DecPosParent(sLONG QtToSub = 1) { posdansparent = posdansparent - QtToSub; };
		inline sLONG GetPosInParent() { return posdansparent; };
		inline void InvalidateNbElemTab() { nbelemtab = -1; };
		inline void PrepareToDelete() { nbelemtab = -2; };
		virtual void CheckDansParent(sLONG n, ObjCacheInTree* enfant) const { };
		virtual Boolean OnlyHasOneChild() const
		{
			return false;
		}

		virtual void Kill()
		{
			Release();
		}

#if debugObjInTree_Strong
		static void RegisterForDebug(ObjCacheInTree* obj);
		static void UnRegisterForDebug(ObjCacheInTree* obj);
		static void Debug_CheckForDelete(ObjCacheInTree* obj);
		static void Debug_CheckParentEnfantsRelations();

		static void DisableCheck()
		{
			sCheckDisabled++;
		}

		static void EnsableCheck()
		{
			sCheckDisabled--;
		}
#endif

	protected:
		ObjCacheInTree* parent;
		sLONG posdansparent;
		sLONG nbelemtab;

#if debugObjInTree_Strong
		static debug_setof_ObjCacheInTree debug_ObjCacheInTrees;
		static VCriticalSection debug_Mutex;
		static sLONG sCheckDisabled;
#endif


};

class ObjAlmostInCacheInTree : public ObjCacheInTree
{
	public:
		ObjAlmostInCacheInTree():ObjCacheInTree(DoNotPutInCacheAcces) { ; }
};





class ObjInCacheMem : public VObject
{
public:

	void	*operator new(size_t size);
	void	*operator new(size_t size, void *inAddr) { UNUSED(size); return inAddr;}
	void	operator delete (void *p );
	void	operator delete (void *p, void* p2 ) { UNUSED(p); UNUSED(p2); };
};



						// ------------------------------------------------------------------


class BaseTaskInfo;

class LockEntity : /*public ObjAlmostInCacheInTree*/ public ObjInCacheMem, public IDebugRefCountable
{
	public:
		inline LockEntity(BaseTaskInfo* owner) 
		{ 
			fOwner = owner; 
			fLockOthersTimeOut = 0; 
			fSpecialFlushAndLock = false;

#if debugLeaksAll
			if (debug_canRegisterLeaksAll)
				RegisterStackCrawl(this);
#endif
		};

#if debugLeaksAll
		virtual bool OKToTrackDebug() const
		{
			return true;
		}

		virtual void GetDebugInfo(VString& outText) const
		{
			outText = "LockEntity : ";
		}

#endif

		inline BaseTaskInfo* GetOwner() const { /*VObjLock lock(this); */return fOwner; };
		inline void SetOwner(BaseTaskInfo* owner) { /*VObjLock lock(this);*/fOwner = owner; };

		inline void SetLockOthersTimeOut(sLONG inTimeOut) { /*VObjLock lock(this); */fLockOthersTimeOut = inTimeOut; };
		inline sLONG GetLockOthersTimeOut() const { /*VObjLock lock(this); */return fLockOthersTimeOut; };

		inline void SetSpecialFlushAndLock(bool x)
		{
			fSpecialFlushAndLock = x;
		}

		inline bool GetSpecialFlushAndLock() const
		{
			return fSpecialFlushAndLock;
		}

		virtual ~LockEntity()
		{
			xbox_assert(fOwner == nil);
		};
		virtual void Kill() 
		{ 
		};




	protected:
		BaseTaskInfo* fOwner;
		sLONG fLockOthersTimeOut;
		bool fSpecialFlushAndLock;
};



class Base4D;

class ObjCacheArrayLongFix : public ObjInCacheMem, public IObjToFlush
{
public:
	ObjCacheArrayLongFix(Base4D *xdb, sLONG xhowmany);
	virtual ~ObjCacheArrayLongFix();
	inline void* LockAndGetptr(void) { return((void*)fbb); };
	inline void unlock(void) { ; };
	sLONG getsize();
	inline uBOOL okcree(void) { return(fbb!=nil); };
	virtual bool SaveObj(VSize& outSizeSaved);
#if autocheckobj
	uBOOL CheckObjInMem(void);
#endif

protected:
	sLONG *fbb;
	sLONG nbelem;
	Base4D *db;
};

#if 0

class ObjectInListHeader;

class ObjectInList : public VObject
{
	public:
	friend class ObjectInListHeader;
		ObjectInList(ObjectInListHeader *rootHeader);
		virtual ~ObjectInList();
		void GetName(VString& Name) { Name.FromString(name); };
		void SetName(VString& Name) { name.FromString(Name); };
		
		static ObjectInList* xFindObject(ObjectInList* root, VString& objtofind);
		
	protected:
		ObjectInList *next;	
		ObjectInList *prev;
		VStr80 name;
		ObjectInListHeader* header;
};


class ObjectInListHeader : public VObject
{
	public:
	friend class ObjectInList;
		ObjectInListHeader(void) { root = nil; };
		virtual ~ObjectInListHeader();
		ObjectInList* FindObject(VString& objtofind) { return(ObjectInList::xFindObject(root, objtofind)); };
		
	protected:
		ObjectInList* root;
		
};

#endif


											/* -------------------------------------------------------- */

inline sWORD SwapWord(sWORD x) { return ((x & 0x00FF) << 8) | (x >> 8); };
inline uWORD SwapWord(uWORD x) { return ((x & 0x00FF) << 8) | (x >> 8); };

inline sLONG SwapLong(sLONG x) { return ( ((sLONG)(SwapWord((uWORD)(x & 0x0000FFFF))) << 16) | (sLONG)SwapWord((uWORD)(x >> 16))); };
inline uLONG SwapLong(uLONG x) { return ( ((uLONG)(SwapWord((uWORD)(x & 0x0000FFFF))) << 16) | (uLONG)SwapWord((uWORD)(x >> 16))); };

// on a 32 bit processor it is not very clever to inline SwapLong8 (L.R)

sLONG8 SwapLong8(sLONG8 x);
uLONG8 SwapLong8(uLONG8 x);

											/* -------------------------------------------------------- */


void _raz(void *p, sLONG len);

void _rau(void *p, sLONG len);

void copyunichar(UniChar *p, UniChar *p2);

uBOOL allzero(void *p, sLONG len);

sLONG comptelong(void *p,sLONG nb);

//sLONG BestTempVol(void);

void moveblock(void *p1, void *p2, sLONG len);
void move2block(void *p1, void *p2, sLONG len);
void move4block(void *p1, void *p2, sLONG len);
void move2insert(void *p1, void *p2, sLONG len);

uBOOL AllFF(void *p, sLONG len);
uBOOL All00(void *p, sLONG len);

void* ResizePtr(void* p, sLONG oldlen, sLONG newlen);


inline VError CumulErr(VError err1, VError err2)
{
	if (err1==VE_OK)
	{
		return(err2);
	}
	else return(err1);
}



																			/* -------------------------------- */

namespace Db4DError
{
	EXTERN_BAGKEY( BaseName);
	EXTERN_BAGKEY( TableName);
	EXTERN_BAGKEY( FieldName);
	EXTERN_BAGKEY( BlobNum);
	EXTERN_BAGKEY( RecNum);
	EXTERN_BAGKEY( IndexName);

	EXTERN_BAGKEY( EntityModelName);
	EXTERN_BAGKEY( EntityAttributeName);
	EXTERN_BAGKEY( TypeName);
	EXTERN_BAGKEY( EnumerationName);
	EXTERN_BAGKEY( RelationName);
	EXTERN_BAGKEY (EntityNum);
	EXTERN_BAGKEY (MethodName);

	EXTERN_BAGKEY( Param1);
	EXTERN_BAGKEY( Param2);
	EXTERN_BAGKEY( Param3);
/*
	EXTERN_BAGKEY( p1);
	EXTERN_BAGKEY( p2);
	EXTERN_BAGKEY( p3);
*/

	EXTERN_BAGKEY( jstext);
	EXTERN_BAGKEY( jsline);
};


class VErrorDB4DBase : public VErrorBase
{
public:
	
	VErrorDB4DBase( VError inErrCode, ActionDB4D inAction):VErrorBase(inErrCode,0) { fAction = inAction;};

	// give description for the error code (optionnal)
	// to be localized.
	// ex: "File 'something.4db' not found."

	//virtual	void					GetErrorDescription( VString& outError) const;

	// describe the action (optionnal)
	// to be localized.
	// ex: "While opening the database 'something'".

	//virtual	void					GetActionDescription( VString& outAction) const;

	// describe the component where the error occured.
	// not to be localized.
	// ex: "database engine component"

	virtual	void					GetLocation( VString& outLocation) const;

protected:
	virtual void DumpToString( VString& outString) const;
	ActionDB4D fAction;

};

class EntityModelCatalog;

class VErrorDB4D_OnBase : public VErrorDB4DBase
{
public:
	
	VErrorDB4D_OnBase( VError inErrCode, ActionDB4D inAction, const Base4D *inBase);
	VErrorDB4D_OnBase( VError inErrCode, ActionDB4D inAction, const EntityModelCatalog *inCatalog);

protected:
	virtual void DumpToString( VString& outString) const;
	VString fBaseName;
};



class VErrorDB4D_OnCheckAndRepair : public VErrorDB4D_OnBase
{
public:
	
	VErrorDB4D_OnCheckAndRepair( VError inErrCode, ActionDB4D inAction, const Base4D *inBase);

protected:
	virtual void DumpToString( VString& outString) const;

};


class VErrorDB4D_OnRelation : public VErrorDB4D_OnBase
{
public:
	
	VErrorDB4D_OnRelation( VError inErrCode, ActionDB4D inAction, const Base4D *inBase);

protected:
	virtual void DumpToString( VString& outString) const;

};


class VErrorDB4D_OnTable : public VErrorDB4D_OnBase
{
public:
	
	VErrorDB4D_OnTable( VError inErrCode, ActionDB4D inAction, const Base4D *inBase, sLONG inTableID);
	Table* RetainOwner( const Base4D *inBase) const;

protected:
	virtual void DumpToString( VString& outString) const;
	sLONG fTableID;
	VString fTableName;

};

class Field;

class VErrorDB4D_OnField : public VErrorDB4D_OnTable
{
public:
	
	VErrorDB4D_OnField( VError inErrCode, ActionDB4D inAction, const Base4D *inBase, sLONG inTableID, sLONG inFieldID);
	Field* RetainOwner( const Base4D *inBase) const;

protected:
	virtual void DumpToString( VString& outString) const;
	sLONG fFieldID;
	VString fFieldName;

};


class VErrorDB4D_OnBlob : public VErrorDB4D_OnTable
{
public:
	
	VErrorDB4D_OnBlob( VError inErrCode, ActionDB4D inAction, const Base4D *inBase, sLONG inTableID, sLONG inBlobNum, ValueKind inBlobType);

protected:
	virtual void DumpToString( VString& outString) const;
	sLONG fBlobNum;
	ValueKind fBlobType;

};


class VErrorDB4D_OnRecord : public VErrorDB4D_OnTable
{
public:
	
	VErrorDB4D_OnRecord( VError inErrCode, ActionDB4D inAction, const Base4D *inBase, sLONG inTableID, sLONG inRecNum);

protected:
	virtual void DumpToString( VString& outString) const;
	sLONG fRecNum;

};

class IndexInfo;

class VErrorDB4D_OnIndex : public VErrorDB4D_OnBase
{
public:
	
	VErrorDB4D_OnIndex( VError inErrCode, ActionDB4D inAction, const Base4D *inBase, IndexInfo* ind);
	virtual ~VErrorDB4D_OnIndex();

protected:
	virtual void DumpToString( VString& outString) const;

};


class EntityModel;
class EntityAttribute;
class db4dEntityAttribute;
class EmEnum;
class AttributeType;
class EntityRelation;
class EntityMethod;


class VErrorDB4D_OnEM : public VErrorDB4D_OnBase
{
	public:

		VErrorDB4D_OnEM( VError inErrCode, ActionDB4D inAction, const EntityModelCatalog *inCatalog, const EntityModel* em);

	protected:
		virtual void DumpToString( VString& outString) const;
		const EntityModel* fModel;

};



class VErrorDB4D_OnEMRec : public VErrorDB4D_OnEM
{
public:

	VErrorDB4D_OnEMRec( VError inErrCode, ActionDB4D inAction, const EntityModelCatalog *inCatalog, const EntityModel* em, const VString& key);

protected:
	virtual void DumpToString( VString& outString) const;
	VString fEntityKey;

};


class VErrorDB4D_OnEMAttribute : public VErrorDB4D_OnEM
{
	public:

		VErrorDB4D_OnEMAttribute( VError inErrCode, ActionDB4D inAction, const EntityModelCatalog *inCatalog, const EntityModel* em, const EntityAttribute* att);

	protected:
		virtual void DumpToString( VString& outString) const;
		const EntityAttribute* fAtt;

};


class VErrorDB4D_OnEMMethod : public VErrorDB4D_OnEM
{
public:

	VErrorDB4D_OnEMMethod( VError inErrCode, ActionDB4D inAction, const EntityModelCatalog *inCatalog, const EntityModel* em, const EntityMethod* meth);

protected:
	virtual void DumpToString( VString& outString) const;
	const EntityMethod* fMethod;

};



class VErrorDB4D_OnEMType : public VErrorDB4D_OnBase
{
public:

	VErrorDB4D_OnEMType( VError inErrCode, ActionDB4D inAction, const EntityModelCatalog* inCatalog, const AttributeType* attType);

protected:
	virtual void DumpToString( VString& outString) const;
	const AttributeType* fAttType;

};


class VErrorDB4D_OnEMEnum : public VErrorDB4D_OnBase
{
public:

	VErrorDB4D_OnEMEnum( VError inErrCode, ActionDB4D inAction, const EntityModelCatalog* inCatalog, const EmEnum* xenum);

protected:
	virtual void DumpToString( VString& outString) const;
	const EmEnum* fEnum;

};


class VErrorDB4D_OnEMRelation : public VErrorDB4D_OnBase
{
public:

	VErrorDB4D_OnEMRelation( VError inErrCode, ActionDB4D inAction, const EntityModelCatalog *inCatalog, const EntityRelation* rel);

protected:
	virtual void DumpToString( VString& outString) const;
	const EntityRelation* fRel;

};




#if 0
inline VError DerniereErreur() { return VTask::GetCurrent()->GetLastError(); };
#endif
inline VError PullLastError() { return VTask::GetCurrent()->GetLastError();  /* a modifier plus tard */ };
inline void FlushErrors() { VTask::GetCurrent()->FlushErrors(); };


																			/* -------------------------------- */

typedef sLONG *genericptr;
/*
class ArrayPtrAvecTrou : public VArrayVIT
{
public:
	inline ArrayPtrAvecTrou(sLONG pNbInit = 0,sLONG pNbToAdd = 4):VArrayVIT(sizeof(genericptr),pNbInit,pNbToAdd){;};
	inline genericptr& operator[](sLONG pIndex) {CALLRANGECHECK(pIndex);return (((genericptr*)(*(tPtr*)fTabHandle))[pIndex]);};
	virtual sLONG FindNextFree(uBOOL from1=false);
};

class ArrayLongAvecTrou : public VArrayVIT
{
public:
	inline ArrayLongAvecTrou(sLONG pNbInit = 0,sLONG pNbToAdd = 4):VArrayVIT(sizeof(sLONG),pNbInit,pNbToAdd){;};
	inline sLONG& operator[](sLONG pIndex) {CALLRANGECHECK(pIndex);return (((sLONG*)(*(tPtr*)fTabHandle))[pIndex]);};
	virtual sLONG FindNextFree(uBOOL from1=false);
};
*/

																			/* -------------------------------- */ 
//void UniToAscii(void* source, void* target, sLONG nbchar);
//void AsciiToUni(void* source, void* target, sLONG nbchar);

// void CenterWindow(VWindow *w);

uBOOL EgalEndString(const VString& s1, const VString& s2);

void TruncateExtension(VString& s1, const VString& s2);


																			/* -------------------------------- */


void StripExtension(VString& s);

sLONG Pos(const VString& s, UniChar c);

sLONG Pos(const VString& s, UniChar c);


//void vHtoHandle(void* hh, void* hh2);


#if journalcache

void InitJournal(void);

void Jmess(uCHAR* mess, VName& name, sLONG classID);

void Jmess(uCHAR* mess, ObjCache* obj);

#endif

//const	sLONG xStringMaxLen = 20;

class xTempObject
{
	public:
		void*			operator new( size_t inSize);
		void			operator delete( void* inAddr);

		void*			operator new( size_t /*inSize*/, void* inAddr)			{ return inAddr; };
		void			operator delete( void* /*inAddr*/, void* /*inAddr*/)	{};

};

template <sLONG datakind>
class xStringBase : public xTempObject
{
	public:


		inline xStringBase(sLONG inMaxChar = kMaxXStringLen - 1) { fMaxLength = (sWORD)inMaxChar; xbox_assert((inMaxChar) >= (sizeof(void*)/2)); };  // xStringBase data must a least contain a pointer
		inline sLONG GetSize() const { return fMaxLength*2 + 4; };
		inline void SetSize(sLONG inSize) { fMaxLength = (inSize-4) / 2; };

		xStringBase(const xStringBase<datakind>& inString); // copy constructor
		xStringBase& operator =( const xStringBase<datakind>& inString);

		//Boolean operator == ( const xString &other ) const;
		//Boolean operator  > ( const xString &other ) const;
		//Boolean operator  < ( const xString &other ) const;
		//Boolean operator >= ( const xString &other ) const;
		//Boolean operator <= ( const xString &other ) const;
		//Boolean operator != ( const xString &other ) const;
		CompareResult CompareTo ( const xStringBase<datakind>& other, const VCompareOptions& inOptions ) const;

		inline UniChar* GetPtr() const { if (fLength > fMaxLength) return *((UniChar**)(&data)); else return (UniChar*)data; };
		inline sLONG GetLength() const { return (sLONG)fLength; };

		void Free();
		Boolean CopyFrom(void* pData);
		Boolean CopyFromString(VString* s);
		void* copyto(void* pData);
		VError CopyToVString(VString& s);

		void CopyFromXString(const xStringBase<datakind>& inString);

		inline static sLONG NoDataSize() { return 4; };

	protected:
		sWORD fLength;
		sWORD fMaxLength;
		UniChar data[kMaxXStringLen];
};

/*
template <sLONG xMaxChar>
class xStr : public xString
{
	public:
		inline xStr():xString(xMaxChar) {;};
	protected:
		UniChar fExtraData[xMaxChar-2];
};
*/




template <sLONG datakind>
void xStringBase<datakind>::Free()
{
	if (fLength > fMaxLength)
	{
		FreeTempMem(*((void**)(&data)));
	}
}


template <sLONG datakind>
void* xStringBase<datakind>::copyto(void* pData)
{
	sLONG len = fLength;
	UniChar* p = GetPtr();
	UniChar* x = ((UniChar*)pData)+2;
	*((sLONG*)pData) = len;
	vBlockMove((void*)p, (void*)x, len*2);
	return (void*)(((char*)pData) + len*2 + 4);
}


template <sLONG datakind>
VError xStringBase<datakind>::CopyToVString(VString& s)
{
	s.FromBlock((void*)GetPtr(), fLength*2, VTC_UTF_16);
	if (s.GetLength() == fLength)
		return VE_OK;
	else
		return ThrowBaseError(memfull, DBaction_BuildingValue);
}


template <sLONG datakind>
Boolean xStringBase<datakind>::CopyFrom(void* pData)
{
	Boolean res = true;
	sLONG len = *((sLONG*)pData);
	UniChar* x = ((UniChar*)pData)+2;  // on ajoute 4 octets

	if (len > kMaxXStringPtrLen)
		len = kMaxXStringPtrLen;
	fLength = (sWORD)len;
	UniChar* p;
	if (len > fMaxLength)
	{
		p = (UniChar*)GetTempMem(len * sizeof(UniChar), false, 'xstr');
		if (p == nil)
		{
			res = false;
			p = data;
			fLength = fMaxLength;
		}
		else
		{
			*((UniChar**)(&data)) = p;
		}
	}
	else
	{
		p = data;
	}

	vBlockMove((void*)x, (void*)p, len * sizeof(UniChar));

	return res;
}


template <sLONG datakind>
Boolean xStringBase<datakind>::CopyFromString(VString* s)
{
	Boolean res = true;
	sLONG len = s->GetLength();
	const UniChar* x = s->GetCPointer();

	if (len > kMaxXStringPtrLen)
		len = kMaxXStringPtrLen;
	fLength = (sWORD)len;
	UniChar* p;
	if (len > fMaxLength)
	{
		p = (UniChar*)GetTempMem(len * sizeof(UniChar), false, 'xstr');
		if (p == nil)
		{
			res = false;
			p = data;
			fLength = fMaxLength;
		}
		else
		{
			*((UniChar**)(&data)) = p;
		}
	}
	else
	{
		p = data;
	}

	vBlockMove((void*)x, (void*)p, len * sizeof(UniChar));

	return res;
}



template <sLONG datakind>
CompareResult xStringBase<datakind>::CompareTo( const xStringBase<datakind>& other, const VCompareOptions& inOptions ) const
{
	UniChar* t1 = GetPtr();
	UniChar* t2 = other.GetPtr();
	//return VDBMgr::CompareString(t1, fLength, t2, other.fLength, true);
	return inOptions.GetIntlManager()->CompareString(t1, fLength, t2, other.fLength, inOptions);
}


template <sLONG datakind>
void xStringBase<datakind>::CopyFromXString(const xStringBase<datakind>& inString)
{
	fLength = inString.fLength;
	fMaxLength = inString.fMaxLength;
	sLONG len = fLength;
	if (len>fMaxLength)
		len = fMaxLength;
	if (len>0)
		vBlockMove((void*)inString.data, (void*)data, len*2);
}


template <sLONG datakind>
xStringBase<datakind>& xStringBase<datakind>::operator =( const xStringBase<datakind>& inString)
{
	CopyFromXString(inString);
	return *this;
}


template <sLONG datakind>
xStringBase<datakind>::xStringBase(const xStringBase<datakind>& inString) // copy constructor
{
	CopyFromXString(inString);
}




template <>
inline void* xStringBase<VK_STRING_UTF8>::copyto(void* pData)
{
	Utf16toUtf8 buffer(GetPtr(), fLength);
	sLONG nbbytes = buffer.GetNbBytes();
	*((sLONG*)pData) = nbbytes;
	const char* p = buffer.GetPtr();
	char* x = ((char*)pData)+4;
	std::copy(p, p+nbbytes, x);
	return (void*)(((char*)pData) + nbbytes + 4);
}


template <>
inline Boolean xStringBase<VK_STRING_UTF8>::CopyFrom(void* pData)
{
	Utf8ToUtf16 buffer(pData);
	Boolean res = true;
	sLONG len = buffer.GetLength();
	const UniChar* x = buffer.GetPtr();

	if (len > kMaxXStringPtrLen)
		len = kMaxXStringPtrLen;
	fLength = (sWORD)len;
	UniChar* p;
	if (len > fMaxLength)
	{
		p = (UniChar*)GetTempMem(len * sizeof(UniChar), false, 'xstr');
		if (p == nil)
		{
			res = false;
			p = data;
			fLength = fMaxLength;
		}
		else
		{
			*((UniChar**)(&data)) = p;
		}
	}
	else
	{
		p = data;
	}

	vBlockMove((void*)x, (void*)p, len * sizeof(UniChar));

	return res;
}



typedef xStringBase<VK_STRING> xString;
typedef xStringBase<VK_STRING_UTF8> xStringUTF8;


class xTime
{
	public:
		Boolean operator == ( const xTime &inTime ) const;
		Boolean operator  > ( const xTime &inTime ) const;
		Boolean operator  < ( const xTime &inTime ) const;
		Boolean operator >= ( const xTime &inTime ) const;
		Boolean operator <= ( const xTime &inTime ) const;
		Boolean operator != ( const xTime &inTime ) const;
		CompareResult CompareTo ( const xTime& inTime ) const;

		xTime(const VTime& inTime)
		{
			fValue = inTime.GetStamp();
		}

		xTime()
		{
			fValue = 0;
		}

		uLONG8 GetStamp() const
		{
			return fValue;
		}

	protected:
		/*
		sWORD fYear;
		sBYTE fMonth;
		sBYTE fDay;
		sLONG fMilliseconds;
		*/
		uLONG8 fValue;

};


class EntityAttribute;
class AttributePath;
class Bittab;

const sLONG kMaxMultiFieldData = 128;

class xMultiFieldDataOffset
{
	public:
		inline xMultiFieldDataOffset()
		{
			fNulls = nil;
			att = nil;
			fAttPath = nil;
			expression = nil;
		}

		/*
		~xMultiFieldDataOffset()  // non virtual on purpose
		{
			QuickReleaseRefCountable(fNulls);
		}
		*/

		inline void Set(sLONG xoffset, sLONG xtyp, sLONG xnumcrit, sLONG xsize, Boolean xascent) 
		{ 
			offset = xoffset; 
			typ = xtyp; 
			numcrit = xnumcrit; 
			size = xsize; 
			ascent = xascent; 
		};

		inline void Set(DB4DLanguageExpression* inExpression, Boolean xascent) 
		{ 
			offset = -1; 
			typ = (sLONG)VK_UNDEFINED; 
			numcrit = -1; 
			size = 0; 
			ascent = xascent; 
			expression = inExpression;
		};

		sLONG Set(sLONG xoffset, db4dEntityAttribute* inAtt, Boolean xascent);
		sLONG Set(sLONG xoffset, AttributePath* inAttPath, Boolean xascent);
		
		inline sLONG GetOffset() const { return offset; };
		inline sLONG GetDataType() const { return typ; };
		inline sLONG GetNumCrit() const { return numcrit; };
		inline sLONG GetSize() const { return size; };
		inline Boolean IsAscent() const { return ascent; };
		inline void SetDataType(sLONG inType) { typ = inType; };
		inline DB4DLanguageExpression* GetExpression() const { return expression; };
		inline db4dEntityAttribute* GetAttribute() const { return att; };
		inline AttributePath* GetAttributePath() const { return fAttPath; };

		void Release();

		bool IsNull(sLONG recnum) const;

		void SetNull(sLONG recnum) const;

	protected:
		sLONG offset;
		sLONG typ;
		sLONG numcrit;
		sLONG size;
		Boolean ascent;
		Bittab* fNulls;
		DB4DLanguageExpression* expression;
		db4dEntityAttribute* att;
		AttributePath* fAttPath;
};

typedef xMultiFieldDataOffset* xMultiFieldDataOffset_iterator;
typedef const xMultiFieldDataOffset* xMultiFieldDataOffset_constiterator;

class Table;

class xMultiFieldDataOffsets
{
	public:
		inline xMultiFieldDataOffsets(Table* target) 
		{ 
			count = 0; 
			fOwner = target; 
			ismulti = true; 
			fWithExpression = false; 
			fWithAttributes = false; 
			fWithObjects = false;
		};

		xMultiFieldDataOffsets(Field* cri, sLONG typ, Boolean ascent, sLONG size);
		xMultiFieldDataOffsets(db4dEntityAttribute* att, Boolean ascent);
		~xMultiFieldDataOffsets(); // non virtual on purpose
		inline sLONG GetCount() const { return count; };
		inline xMultiFieldDataOffset* GetOffset(sLONG n) { return &(offsets[n]); };
		inline const xMultiFieldDataOffset* GetOffset(sLONG n) const { return &(offsets[n]); };
		Boolean AddOffset(sLONG offset, sLONG typ, sLONG numcrit, sLONG size, Boolean ascent);
		Boolean AddOffset(DB4DLanguageExpression* inExpression, Boolean ascent);
		sLONG AddOffset(sLONG offset, db4dEntityAttribute* inAtt, Boolean ascent);
		sLONG AddOffset(sLONG offset, AttributePath* inAttPath, Boolean ascent);
		xMultiFieldDataOffset_iterator Begin() { return &(offsets[0]); };
		xMultiFieldDataOffset_iterator End() { return &(offsets[count]); };
		xMultiFieldDataOffset_constiterator Begin() const { return &(offsets[0]); };
		xMultiFieldDataOffset_constiterator End() const { return &(offsets[count]); };
		inline Table* GetTarget() const { return fOwner; };
		inline Boolean IsMulti() const { return ismulti; };
		inline Boolean WithExpression() const { return fWithExpression; };
		inline Boolean WithAttributes() const { return fWithAttributes; };
		inline Boolean WithObjects() const { return fWithObjects; };
		inline Boolean IsAscent() const { return IsMulti() || WithExpression() || offsets[0].IsAscent(); };
		bool IsAllDescent() const;
		inline void SetAuxBuffer(void* auxbuff) const { fAuxBuffer = auxbuff; };
		inline void* GetAuxBuffer() const { return (void*)fAuxBuffer; };
		inline sLONG GetLength() const { return fLen; };
		inline void SetLength(sLONG len) const { fLen = len; };
		inline void computeMulti()
		{
			ismulti = (count > 1);
		}

	protected:
		sLONG count;
		mutable sLONG fLen;
		xMultiFieldDataOffset offsets[kMaxMultiFieldData];
		Table* fOwner;
		mutable void* fAuxBuffer;
		Boolean ismulti;
		Boolean fWithExpression, fWithAttributes, fWithObjects;
};

class xMultiFieldData
{
	public:
		inline xMultiFieldData()
		{
			offsets = nil;
			data = nil;
		};

		~xMultiFieldData(); // non virtual on purpose

		xMultiFieldData(const xMultiFieldData& other); // copy constructor
		xMultiFieldData& operator =( const xMultiFieldData& other);
		void CopyFrom(const xMultiFieldData& other);

		inline xMultiFieldData* GetTrueData()
		{
			if (offsets == nil)
				return (xMultiFieldData*)data;
			else
				return this;
		};

		inline const xMultiFieldData* GetTrueData() const
		{
			if (offsets == nil)
				return (const xMultiFieldData*)data;
			else
				return this;
		};

		//Boolean operator == ( const xMultiFieldData &other ) const;
		//Boolean operator  > ( const xMultiFieldData &other ) const;
		//Boolean operator  < ( const xMultiFieldData &other ) const;
		//Boolean operator >= ( const xMultiFieldData &other ) const;
		//Boolean operator <= ( const xMultiFieldData &other ) const;
		//Boolean operator != ( const xMultiFieldData &other ) const;
		CompareResult CompareTo ( const xMultiFieldData& other, const VCompareOptions& inOptions, sLONG recnum1, sLONG recnum2 ) const;

		inline char* GetDataPtr(sLONG off) { return ((char*)&(GetTrueData()->data))+off; };
		inline const char* GetDataPtr(sLONG off) const { return ((char*)&(GetTrueData()->data))+off; };

		inline void SetHeader(const xMultiFieldDataOffsets* header) { offsets = header; };

		static inline sLONG GetEmptySize() { return sizeof(xMultiFieldDataOffsets*); };

		void* CopyTo(void* pData) const;

		void Free(RecIDType recnum);

		inline void* GetAuxBuffer() { return GetTrueData()->offsets->GetAuxBuffer(); };

	protected:
		const xMultiFieldDataOffsets* offsets;
		void* data;
};

class DataTable;

ValPtr NewValPtr(sLONG typ, void* from, sLONG fromType, DataTable* df, BaseTaskInfo* context);




																			/* -------------------------------- */


class ExtraPropertiesObj;


class ExtraPropertiesHeader : public Obj4D
{
	public:
		inline ExtraPropertiesHeader(DataAddr4D* inExtraAddr, sLONG* inExtraLen, Base4D* owner, sLONG pos1, sLONG pos2, DataBaseObjectType taginfo, Boolean inIsRemote) 
		{ fExtraAddr = inExtraAddr; 
			fExtraLen = inExtraLen;  
			fExtraIsModified = false;
			fExtra = nil;
			fExtraStream = nil;
			fExtraPropertiesObjRef = nil;
			db = owner;
			fPos1 = pos1;
			fPos2 = pos2;
			fTagInfo = taginfo;
			fIsRemote = inIsRemote;
		};

		inline void InitLater(sLONG pos1, sLONG pos2)
		{	
			fPos1 = pos1;
			fPos2 = pos2;
		};

		virtual ~ExtraPropertiesHeader();

		VValueBag* RetainExtraProperties(VError &err);
		VError SetExtraProperties(VValueBag* inExtraProperties, bool loadonly = false);
		inline void ClearExtraPropertiesObjRef() { fExtraPropertiesObjRef = nil; };
		inline VPtrStream* GetData() const { return fExtraStream; };
		inline void ClearData() { xbox_assert(fExtraStream != nil); delete fExtraStream; fExtraStream = nil; };

		inline sLONG GetPos1() const { return fPos1; };
		inline sLONG GetPos2() const { return fPos2; };
		inline sLONG GetTagInfo() const { return fTagInfo; };
		inline Base4D* GetDB() const { return db; };
		inline void SetDB(Base4D* owner) { db = owner; };

	protected:
		DataAddr4D* fExtraAddr;
		sLONG* fExtraLen;
		VValueBag* fExtra;
		VPtrStream* fExtraStream;
		Boolean fExtraIsModified;
		ExtraPropertiesObj* fExtraPropertiesObjRef;
		Base4D* db;
		sLONG fPos1;
		sLONG fPos2;
		DataBaseObjectType fTagInfo;
		Boolean fIsRemote;
};


class ExtraPropertiesObj : public ObjAlmostInCache
{
	public:
		inline ExtraPropertiesObj(ExtraPropertiesHeader* Owner) { fOwner = Owner; };

		virtual uBOOL okdel(void);
		virtual sLONG saveobj(void);
		virtual sLONG liberemem(sLONG allocationBlockNumber, sLONG combien=-1, uBOOL tout=false);

	protected:
		ExtraPropertiesHeader* fOwner;
};




template<class T>
VError PutExtraPropertiesInBag( const VValueBag::StKey& inElementName, T *inObject, VValueBag& ioBag, CDB4DBaseContext* inContext)
{
	VError err = VE_OK;
	// insert extra properties.
	const VValueBag *bag_extra = inObject->RetainExtraProperties( err, inContext);
	if ( (bag_extra != NULL) && !bag_extra->IsEmpty())
	{
		VValueBag *bag_extra_clone = new VValueBag( *bag_extra);
		if (bag_extra_clone != NULL)
		{
			ioBag.AddElement( inElementName, bag_extra_clone);
			bag_extra_clone->Release();
		}
		else
			err = ThrowBaseError(memfull, DBaction_AccessingExtraProperty);
		bag_extra->Release();
	}
	return err;
}



template<class T>
VError GetExtraPropertiesFromBag( const VValueBag::StKey& inElementName, T *inObject, const VValueBag& ioBag, bool inNotify, CDB4DBaseContext* inContext, bool loadonly = false)
{
	VError err = VE_OK;
	const VValueBag *bag_extra = ioBag.GetUniqueElement(inElementName);
	if (bag_extra != NULL)
	{
		VValueBag *bag_extra_clone = new VValueBag( *bag_extra);
		if (bag_extra_clone != NULL)
		{
			err = inObject->SetExtraProperties(bag_extra_clone, inNotify, inContext, loadonly);
			bag_extra_clone->Release();
		}
		else
			err = ThrowBaseError(memfull, DBaction_ModifyingExtraProperty);
	}
	else
		err = inObject->SetExtraProperties(nil, inNotify, inContext, loadonly);

	return err;
}

const sLONG kTransFakeKeyNumForBeginWith = -1500000000;
const sLONG kTransFakeKeyNumForIsLike = -1500000001;




																			/* -------------------------------- */


VFileStream* CreateTempFile(Base4D* db, VFolder* inFolder = nil, uLONG8 size = 0);

void FormatStringWithParams(VString& sout, sLONG nbparams, sLONG param1, sLONG param2 = 0, sLONG param3 = 0, sLONG param4 = 0,  sLONG param5 = 0);
void FormatStringWithParamsStrings(VString& sout, VString* param1 = nil, VString* param2 = nil, VString* param3 = nil, VString* param4 = nil, VString* param5 = nil);

// get localized string from XLIFF
void gs(sLONG inID, uLONG inStringID, XBOX::VString &sout);
VString GetString(sLONG inID, uLONG inStringID);

typedef sLONG ToolObjectErrorLevel;
enum
{
	TO_ErrorLevel_Lethal = 1,
	TO_ErrorLevel_Normal,
	TO_ErrorLevel_Warning
};

typedef sLONG ToolObjectType;
enum
{
	TO_Base = 1,
	TO_Seg,
	TO_Bittable,
	TO_BittableSec,
	TO_DataTable,
	TO_DataField,
	TO_DataTable_RecTabAddr,
	TO_DataTable_BlobTabAddr,
	TO_Record,
	TO_Blob,
	TO_TableDefHeader,
	TO_TableDef,
	TO_RelationDefHeader,
	TO_RelationDef,
	TO_IndexDefHeader,
	TO_IndexDef,
	TO_IndexDefInStructHeader,
	TO_IndexDefInStruct,
	TO_SeqNumHeader,
	TO_SeqNum,
	TO_UUIDConflict,
	TO_FieldDef,
	TO_Index_PageTabAddr,
	TO_Index_Page,
	TO_Index_ClusterTabAddr,
	TO_Index_Cluster,
	TO_Index_ClusterPage,
	TO_Index,
	TO_DataTableHeader,
	TO_ObjectAddr,
	TO_GlobalOverlap,

	TO_Final
};


typedef sLONG ToolObjectProblem;
enum
{
	TOP_FullyWrong = 1,
	TOP_PartiallyWrong,
	TOP_WrongCheckSum,
	TOP_WrongHeader,
	TOP_WrongHeaderVersionNumber,
	TOP_MultiSegHeaderIsInvalid,
	TOP_EndOfFileNotMatching,
	TOP_AddrOfPrimaryTableOfBittableIsInvalid,
	TOP_PrimaryTableOfBittableIsInvalid,
	TOP_AddrOfSecondaryTableOfBittableIsInvalid,
	TOP_SecondaryTableOfBittableIsInvalid,
	TOP_AddrOfBittableIsInvalid,
	TOP_BittableIsInvalid,
	TOP_CannotOpenDataSeg,

	TOP_TableOfDataTablesIsInvalid,
	TOP_Table_NBficIsInvalid,
	TOP_Table_AddrOfPrimaryTableOfRecordsIsInvalid,
	TOP_Table_ListOfDeletedRecordsIsInvalid,
	TOP_Table_NBblobsIsInvalid,
	TOP_Table_AddrOfPrimaryTableOfBlobsIsInvalid,
	TOP_Table_ListOfDeletedBlobsIsInvalid,

	TOP_Table_AddrOfRecordAddressTableIsInvalid,
	TOP_Table_AddrOfBlobAddressTableIsInvalid,
	TOP_Table_TagOfRecordAddressTableIsInvalid,
	TOP_Table_TagOfBlobAddressTableIsInvalid,
	TOP_Table_PhysicalDataOfRecordAddressTableIsInvalid,
	TOP_Table_PhysicalDataOfBlobAddressTableIsInvalid,
	TOP_Table_CheckSumOfRecordAddressTableIsInvalid,
	TOP_Table_CheckSumOfBlobAddressTableIsInvalid,

	TOP_LenghOfAddressTableIsInvalid,
	TOP_BlobAddrIsInvalid,
	TOP_BlobPhysicalDataIsInvalid,
	TOP_BlobCheckSumIsInvalid,
	TOP_BlobLengthIsInvalid,
	TOP_BlobLengthDoesNotMatchTabAddr,
	TOP_BlobTagIsInvalid,

	TOP_RecordAddrIsInvalid,
	TOP_RecordPhysicalDataIsInvalid,
	TOP_RecordCheckSumIsInvalid,
	TOP_RecordLengthIsInvalid,
	TOP_RecordLengthDoesNotMatchTabAddr,
	TOP_RecordTagIsInvalid,
	TOP_RecordNumberOfFieldsIsInvalid,

	TOP_FieldDataTypeIsInvalid,
	TOP_FieldDataOffsetIsInvalid,
	TOP_FieldDataLengthIsInvalid,
	TOP_FieldBlobNumIsInvalid,
	TOP_FieldBlobIsDeleted,
	TOP_FieldBlobIsInvalid,
	TOP_FieldBlobRefIsAlreadyUsed,

	TOP_Table_ChainOfDeletedBlobsIsCircular,
	TOP_Table_ChainOfDeletedBlobsIsInvalid,
	TOP_Table_ChainOfDeletedRecordsIsCircular,
	TOP_Table_ChainOfDeletedRecordsIsInvalid,
	TOP_Table_SomeBlobsAreOrphan,

	TOP_AddrOfPrimaryTabAddrIsInvalid,
	TOP_NbEntriesIsInvalid,
	TOP_ListOfDeletedEntriesIsInvalid,
	TOP_AddrOfEntryIsInvalid,

	TOP_AddrOfTabAddrIsInvalid,
	TOP_TagOfTabAddrIsInvalid,
	TOP_PhysicalDataOfTabAddrIsInvalid,
	TOP_ChecksumOfTabAddrIsInvalid,

	TOP_LengthIsInvalid,
	TOP_AddrIsInvalid,
	TOP_CheckSumIsInvalid,
	TOP_PhysicalDataIsInvalid,
	TOP_TagIsInvalid,

	TOP_TableNameLengthIsIvalid,
	TOP_RecordNameLengthIsInvalid,
	TOP_FieldNameLengthIsInvalid,
	TOP_TableTypeIsInvalid,
	TOP_Table_NumberOfFieldsIsInvalid,
	TOP_Table_NumberOfFieldsDoesNotMatchTableLength,

	TOP_TwoIdenticalUUIDs,
	TOP_FieldTypeIsInvalid,
	TOP_TagLengthDoesNotMatch,
	TOP_ExtraPropertiesStreamIsInconsistant,

	TOP_SeqNumTypeIsInvalid,
	TOP_SeqNumIDIsInvalid,
	TOP_RelationTypeIsInvalid,
	TOP_NumberOfRelationsIsInvalid,
	TOP_RelationTypeNotMatching,
	TOP_SourceFieldIDCannotBeFound,
	TOP_DestFieldIDCannotBeFound,
	TOP_RelationListIsDamaged,
	TOP_NumberOfSourcesDoesNotNumberOfDests,

	TOP_IndexDefIsDamaged,
	TOP_IndexTypeIsInvalid,
	TOP_WrongFieldNumberInIndex,
	TOP_WrongTableNumberInIndex,
	TOP_IndexHeaderTypeIsInvalid,

	TOP_Index_AddrOfPrimaryTabAddrOfPagesIsInvalid,
	TOP_Index_NbPagesIsInvalid,
	TOP_Index_TabTrouIsInvalid,
	TOP_Index_FirstPageIsInvalid,
	TOP_Index_AddrOfPrimaryTabAddrOfClustersIsInvalid,
	TOP_Index_NbClustersIsInvalid,
	TOP_Index_TabTrouClusterIsInvalid,

	TOP_ExtraPropertyLengthIsInvalid,
	TOP_ExtraPropertyAddrIsInvalid,
	TOP_ExtraPropertyCheckSumIsInvalid,
	TOP_ExtraPropertyPhysicalDataIsInvalid,
	TOP_ExtraPropertyTagIsInvalid,
	TOP_ExtraPropertyTagLengthDoesNotMatch,

	TOP_Index_AddrOfPageAddressTableIsInvalid,
	TOP_Index_AddrOfClusterAddressTableIsInvalid,
	TOP_Index_TagOfPageAddressTableIsInvalid,
	TOP_Index_TagOfClusterAddressTableIsInvalid,
	TOP_Index_PhysicalDataOfPageAddressTableIsInvalid,
	TOP_Index_PhysicalDataOfClusterAddressTableIsInvalid,
	TOP_Index_CheckSumOfPageAddressTableIsInvalid,
	TOP_Index_CheckSumOfClusterAddressTableIsInvalid,

	TOP_Cluster_TypeOfSelectionIsInvalid,
	TOP_Cluster_NbRecIsPetiteSelIsInvalid,
	TOP_Cluster_InvalidRecordNumber,
	TOP_Cluster_DeletedRecordNumber,
	TOP_Cluster_BitCountIsInvalid,
	TOP_IndexPage_NbKeysIsInvalid,
	TOP_Cluster_DuplicatedRecordNumber,
	TOP_IndexPage_SubPageRefIsInvalid,
	TOP_IndexPage_SubPageRefIsDeleted,
	TOP_IndexPage_KeyDirectoryIsInvalid,
	TOP_IndexPage_InvalidRecordNumber,
	TOP_IndexPage_DeletedRecordNumber,
	TOP_IndexPage_DuplicatedRecordNumber,
	TOP_IndexPage_KeyOrderIsInvalid,
	TOP_IndexPage_KeyDataIsInvalid,

	TOP_Index_SomeRecordsAreNotReferenced,
	TOP_IndexPage_PageUsedInCircularReference,

	TOP_DataTableHeader_CheckSumIsInvalid,
	TOP_DataTableHeader_LengthIsInvalid,
	TOP_DataTableHeader_TagIsInvalid,
	TOP_DataTableHeader_PhysicalDataIsInvalid,
	TOP_DataTableHeader_AddrIsInvalid,

	TOP_WrongDataTypeNumberInIndex,
	TOP_Datafile_Does_Not_Match_Struct,
	TOP_Segment_IndexFileMissing,

	TOP_LastFlushDidNotComplete,

	TOP_ObjAddrOverlapping,
	TOP_Index_flagged_for_rebuilding,

	TOP_FieldExternalBlobIsMissing,

	TOP_Final
};


typedef sLONG TypeOfUUID;
enum
{
	tuuid_Table = 1,
	tuuid_Field,
	tuuid_Index,
	tuuid_AutoSeqNum,
	tuuid_Relation,

	tuuid_Final
};


class ToolLog;

class IProblemReporterIntf
{
public:
	virtual VError ReportInvalidTabAddrAddr(ToolLog* log, sLONG selector) = 0;
	virtual VError ReportInvalidTabAddrTag(ToolLog* log, sLONG selector) = 0;
	virtual VError ReportInvalidTabAddrRead(ToolLog* log, sLONG selector) = 0;
	virtual VError ReportInvalidTabAddrChecksum(ToolLog* log, sLONG selector) = 0;
};

template<class Collection, class Cle>
inline bool IsInCollection(const Collection& col, const Cle& cle)
{
	if (col.find(cle) == col.end())
		return false;
	else
		return true;
}




const sWORD kRangeReqDB4D = 10000;
const sWORD kMaxRangeReqDB4D = 20000;
const sWORD kRangeReqDB4DWithBaseID = 11000;
const sWORD kMaxRangeReqDB4DWithBaseID = 12000;


// --------------------------------------------------------------------------------------------------- //


typedef enum 
{
	req_OpenOrCreateBase = 1,
	req_PostPone = 2,

	Req_OpenBase = 1001,
	Req_CheckBaseForUpdate,
	Req_SetBaseExtraProperties,

	Req_AddTable_With_Bag ,
	Req_AddTable ,
	Req_SetTableName,
	Req_SetTableExtraProperties,
	Req_DropTable,

	Req_SelectAllRecords,
	Req_Query,
	Req_DeleteRecordsInSelection,
	Req_AskForASelectionPart,
	Req_LoadRecord,
	Req_CountRecordsInTable,

	Req_SetFieldName,
	Req_SetFieldAttributes,
	Req_SetFieldTextSwitchSize,
	Req_SetFieldExtraProperties,
	Req_AddFields_With_BagArray,
	Req_DropField,

	Req_AddRelation,
	Req_DropRelation,
	Req_SetRelationAttributes,
	Req_SetRelationExtraProperties,

	Req_CreateIndexOnOneField,
	Req_CreateIndexOnMultipleField,
	Req_CreateFullTextIndexOnOneField,
	Req_DropIndexOnOneField,
	Req_DropIndexOnMultipleField,
	Req_DropFullTextIndexOnOneField,
	Req_SetIndexName,
	Req_DropIndexByRef,
	Req_RebuildIndexByRef,

	Req_LockDataBaseDef,
	Req_LockTableDef,
	Req_LockFieldDef,
	Req_LockRelationDef,
	Req_LockIndexDef,
	Req_UnlockObjectDef,

	Req_SetContextExtraData,
	Req_SaveRecord,
	req_DeleteRecord,
	req_ConnectRecord,

	Req_ExecuteQuery,
	Req_SortSelection,

	Req_SendLastRemoteInfo,

	Req_DataToCollection,
	Req_CollectionToData,

	Req_GetDistinctValues,

	Req_ActivateManyToOne,
	Req_ActivateOneToMany,
	Req_ActivateManyToOneS,

	Req_ActivateRelsOnAPath,

	Req_RelateManySelection,
	Req_RelateOneSelection,

	Req_FindKey,
	Req_ReserveRecordNumber,
	Req_ScanIndex,

	Req_CacheDisplaySelection,

	Req_SetFullyDeleteRecords,
	Req_TruncateTable,
	Req_ExecuteColumnFormulas,

	Req_DelRecFromSel,
	Req_DelSetFromSel,
	Req_DelRangeFromSel,
	Req_AddRecIDToSel,
	Req_ReduceSel,

	Req_ActivateAutomaticRelations_N_To_1,
	Req_ActivateAutomaticRelations_1_To_N,
	Req_ActivateAllAutomaticRelations,

	Req_DeleteRecordByID,

	Req_FillSetFromArrayOfBits,
	Req_FillSetFromArrayOfLongs,
	Req_FillArrayOfBitsFromSet,
	Req_FillArrayOfLongsFromSet,

	Req_SetStructExtraProperties,

	Req_SyncThingsToForget,

	Req_SetFieldNeverNull,

	Req_SetTableSchema,
	Req_SetTableKeepStamp,
	Req_SetTableKeepRecordSyncInfo,

	Req_TestServer,

	Req_WhoLockedRecord,

	Req_GetListOfDeadTables,
	Req_ResurectTable,
	Req_GetTableFragmentation,
	
	Req_SetTablePrimaryKey,
	Req_SetFieldStyledText,
	Req_SetOutsideData,

	Req_SetFieldHideInRest,
	Req_SetTableHideInRest,
	Req_SetTablePreventJournaling,

	Req_Final

} Req_ID;


namespace DB4DRemoteBagKeys
{
	EXTERN_BAGKEY_NO_DEFAULT( rm_name_Nto1, XBOX::VString);
	EXTERN_BAGKEY_NO_DEFAULT( rm_name_1toN, XBOX::VString);
	EXTERN_BAGKEY_NO_DEFAULT_SCALAR( rm_auto_load_Nto1, XBOX::VBoolean, bool);
	EXTERN_BAGKEY_NO_DEFAULT_SCALAR( rm_auto_load_1toN, XBOX::VBoolean, bool);
	EXTERN_BAGKEY_NO_DEFAULT( rm_integrity, XBOX::VString);
	EXTERN_BAGKEY( rm_state);
	EXTERN_BAGKEY_NO_DEFAULT_SCALAR( rm_foreign_key, XBOX::VBoolean, bool);

	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( lock_timeout, XBOX::VLong, sLONG);
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( lock_for_readonly, XBOX::VBoolean, bool);
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( lock_with_fields, XBOX::VBoolean, bool);
	EXTERN_BAGKEY_WITH_DEFAULT_SCALAR( lock_with_relatedfields, XBOX::VBoolean, bool);

};


class Base4D;
class Table;
class Selection;
class Relation;
class QueryOptions;
class QueryResult;
class SearchTab;
class IndexInfo;
class SortTab;
class Bittab;
class ColumnFormulas;

extern void _PutThingsToForget( VStream* into, VDBMgr *inMgr, BaseTaskInfo* context);

class IRequest : public IStreamRequest // attention ne pas ajouter de fonctions virtuelles, ni de constructeur/destructeurs, ni de donnees dans cette classe 
{
	public:
		void PutBooleanParam(Boolean b)
		{
			GetOutputStream()->PutByte(b);
		}

		void PutWordParam(sWORD w)
		{
			GetOutputStream()->PutWord(w);
		}

		void PutLongParam(sLONG l)
		{
			GetOutputStream()->PutLong(l);
		}

		void PutStringParam( const VString& inString)
		{
			GetOutputStream()->PutByte(1);
			inString.WriteToStream( GetOutputStream());
		}

		void PutStringParam( const VString *inString)
		{
			GetOutputStream()->PutByte( (inString != NULL) ? 1 : 0);
			if (inString != NULL)
				inString->WriteToStream( GetOutputStream());
		}

		void PutValueBagParam( const VValueBag& inBag)
		{
			GetOutputStream()->PutByte(1);
			inBag.WriteToStream( GetOutputStream());
		}

		void PutValueBagParam( const VValueBag *inBag)
		{
			GetOutputStream()->PutByte( (inBag != NULL) ? 1 : 0);
			if (inBag != NULL)
				inBag->WriteToStream( GetOutputStream());
		}

		void PutValueBagArrayParam( const VBagArray& inBagArray)
		{
			inBagArray.WriteToStreamMinimal( GetOutputStream());
		}

		void PutBaseParam( const Base4D* inBase);
		void PutTableParam( const Table* inTable);
		void PutFieldParam( const Field *inField);
		void PutRelationParam( const Relation *inRelation);
		void PutSelectionParam(Selection* inSel, CDB4DBaseContext* inContext);
		void PutSetParam(Bittab* inSet, CDB4DBaseContext* inContext);
		void PutFicheInMemParam(FicheInMem* rec, CDB4DBaseContext* inContext);
		void PutFicheInMemMinimalParam(FicheInMem* rec, CDB4DBaseContext* inContext);
		void PutThingsToForget( VDBMgr *inMgr, BaseTaskInfo* context);
		void PutIndexParam( const IndexInfo *inIndex);

		void PutQueryParam( SearchTab *query);
		void PutSortTabParams(SortTab* tabs);

		void PutQueryOptionsParam( QueryOptions* inQueryOptions, BaseTaskInfo* context);

		void PutCollectionParam(DB4DCollectionManager& inCollection, bool OnlyHeader);

		void PutValueParam(const VValueSingle* inVal);

		VError PutColumnFormulasParam(ColumnFormulas* formulas, Base4D* inBase);

		void PutProgressParam(VDB4DProgressIndicator* inProgress);
	
		VError PutNumFieldArrayParam( const NumFieldArray& inNumFieldArray);

		VError GetLastError()
		{
			return GetOutputStream()->GetLastError();
		}


		// get reply  ///////////////////////

		Boolean GetBooleanReply(VError& outErr)
		{
			uBYTE result;
			outErr = GetInputStream()->GetByte(result);
			return result;
		}

		sBYTE GetByteReply( VError& outErr)
		{
			sBYTE result;
			outErr = GetInputStream()->GetByte( result);
			return result;
		}

		sWORD GetWordReply(VError& outErr)
		{
			sWORD result;
			outErr = GetInputStream()->GetWord(result);
			return result;
		}

		sLONG GetLongReply(VError& outErr)
		{
			sLONG result;
			outErr = GetInputStream()->GetLong(result);
			return result;
		}

		sLONG8 GetLong8Reply(VError& outErr)
		{
			sLONG8 result;
			outErr = GetInputStream()->GetLong8(result);
			return result;
		}

		VError GetStringReply( VString& outString)
		{
			sBYTE haveString = 0;
			VError err = GetInputStream()->GetByte( haveString);
			if (err==VE_OK && haveString==1)
			{
				err = outString.ReadFromStream( GetInputStream());
			}
			return err;
		}

		VString* GetStringReply( VError &outErr)
		{
			VString *result = NULL;
			sBYTE haveString = 0;
			outErr = GetInputStream()->GetByte( haveString);
			if (outErr==VE_OK && haveString==1)
			{
				result = new VString();
				outErr = result->ReadFromStream( GetInputStream());
			}
			return result;
		}

		VValueBag* RetainValueBagReply( VError &outErr)
		{
			VValueBag *result = NULL;
			sBYTE haveBag = 0;
			outErr = GetInputStream()->GetByte( haveBag);
			if (outErr==VE_OK && haveBag==1)
			{
				result = new VValueBag();
				outErr = result->ReadFromStream( GetInputStream());
			}
			return result;
		}
		
		VError GetArrayLongReply(sLONG* outArray, sLONG maxlongs);
	
		VError GetNumFieldArrayReply( NumFieldArray& outNumFieldArray);

		Selection* RetainSelectionReply(Base4D* inBase, VError& outErr, CDB4DBaseContextPtr inContext);
		Bittab* RetainSetReply(Base4D* inBase, Table* inTable, VError& outErr, CDB4DBaseContextPtr inContext);
		FicheInMem* RetainFicheInMemReply(Table* inTable, VError& outErr, CDB4DBaseContextPtr inContext);

		QueryResult* RetainQueryResultReply(Base4D* inBase, Table* inTable, VError& outErr, BaseTaskInfo* inContext);
		VError ReadQueryResultReply(Base4D* inBase, Table* inTable, QueryResult* outResult, BaseTaskInfo* inContext);

		VError GetCollectionReply(DB4DCollectionManager& outCollection, sLONG& totalRow, sLONG startingRow, sLONG& processedRows);

		VError GetColumnFormulasResultReply(ColumnFormulas* formulas);

		VValueSingle* BuildValueReply(VError& outErr);

		VError GetUpdatedInfo(Base4D* base, BaseTaskInfo *context);

};


		/////////////////////////////////////////////////////


extern VError _GetThingsToForget( VStream* clientreq, VDBMgr *inMgr, BaseTaskInfo* context);


class IRequestReply : public IStreamRequestReply  // attention ne pas ajouter de fonctions virtuelles, ni de constructeur/destructeurs, ni de donnees dans cette classe
{
public:

	Boolean GetBooleanParam(VError& outErr)
	{
		uBYTE result;
		outErr = GetInputStream()->GetByte(result);
		return result;
	}

	sWORD GetWordParam(VError& outErr)
	{
		sWORD result;
		outErr = GetInputStream()->GetWord(result);
		return result;
	}

	sLONG GetLongParam(VError& outErr)
	{
		sLONG result;
		outErr = GetInputStream()->GetLong(result);
		return result;
	}

	VError GetStringParam( VString& outString)
	{
		sBYTE haveString = 0;
		VError err = GetInputStream()->GetByte( haveString);
		if (err==VE_OK && haveString==1)
		{
			err = outString.ReadFromStream( GetInputStream());
		}
		return err;
	}

	VString* GetStringParam( VError &outErr)
	{
		VString *result = NULL;
		sBYTE haveString = 0;
		outErr = GetInputStream()->GetByte( haveString);
		if (outErr==VE_OK && haveString==1)
		{
			result = new VString();
			outErr = result->ReadFromStream( GetInputStream());
		}
		return result;
	}

	VValueBag* RetainValueBagParam( VError& outErr)
	{
		VValueBag *result = NULL;
		sBYTE haveBag = 0;
		outErr = GetInputStream()->GetByte( haveBag);
		if (outErr==VE_OK && haveBag==1)
		{
			result = new VValueBag();
			outErr = result->ReadFromStream( GetInputStream());
		}
		return result;
	}
	
	VBagArray* RetainValueBagArrayParam( VError& outErr)
	{
		VBagArray *bagArray = new VBagArray();
		if (bagArray != nil)
			outErr = bagArray->ReadFromStreamMinimal( GetInputStream());
		else
			outErr = memfull;
		return bagArray;
	}

	Base4D* RetainBaseParam( VError &outErr);
	Table* RetainTableParam( Base4D* inBase, VError &outErr);
	Field* RetainFieldParam( Table *inTable, VError &outErr);
	Relation* RetainRelationParam( Base4D *inBase, VError &outErr);
	Selection* RetainSelectionParam(Base4D* inBase, Table* inTable);
	Bittab* RetainSetParam(Base4D* inBase, Table *inTable);
	VError GetThingsToForget( VDBMgr *inMgr, BaseTaskInfo* context);
	FicheInMem* RetainFicheInMemParam( Base4D* inBase, Table* inTable, BaseTaskInfo* inContext, VError &outErr);
	FicheInMem* RetainFicheInMemMinimalParam( Base4D* inBase, Table* inTable, BaseTaskInfo* inContext, VError &outErr);
	IndexInfo* RetainIndexParam( Base4D *inBase, BaseTaskInfo *inContext, VError& outErr, Boolean mustBeValid);
	VValueSingle* BuildValueParam(VError& outErr);

	QueryOptions* RetainQueryOptionsParam( Base4D* inBase, Table* inTable, BaseTaskInfo* inContext, VError &outErr);

	SearchTab* RetainQueryParam( Table* inTable, VError &outErr);
	VError ReadSortTabParam(SortTab& tabs);

	DB4DCollectionManager* RetainCollectionParam(Base4D* inBase, VError &outErr);

	ColumnFormulas* GetColumnFormulasParam(Base4D* inBase, VError &outErr);

	VDB4DProgressIndicator* RetainProgressParam(CDB4DBaseContext* inContext);
	
	VError GetNumFieldArrayParam( NumFieldArray& outNumFieldArray);

	// put reply /////////////////////////////

	void PutBooleanReply(Boolean b)
	{
		GetOutputStream()->PutByte(b);
	}

	void PutByteReply( sBYTE inByte)
	{
		GetOutputStream()->PutByte( inByte);
	}

	void PutWordReply(sWORD w)
	{
		GetOutputStream()->PutWord(w);
	}

	void PutLong8Reply(sLONG8 l)
	{
		GetOutputStream()->PutLong8(l);
	}

	void PutLongReply(sLONG l)
	{
		GetOutputStream()->PutLong(l);
	}

	void PutStringReply( const VString& inString)
	{
		GetOutputStream()->PutByte(1);
		inString.WriteToStream( GetOutputStream());
	}

	void PutStringReply( const VString *inString)
	{
		GetOutputStream()->PutByte( (inString != NULL) ? 1 : 0);
		if (inString != NULL)
			inString->WriteToStream( GetOutputStream());
	}

	void PutValueBagReply( const VValueBag& inBag)
	{
		GetOutputStream()->PutByte(1);
		inBag.WriteToStream( GetOutputStream());
	}

	void PutValueBagReply( const VValueBag *inBag)
	{
		GetOutputStream()->PutByte( (inBag != NULL) ? 1 : 0);
		if (inBag != NULL)
			inBag->WriteToStream( GetOutputStream());
	}

	VError PutSelectionReply(Selection* inSel, CDB4DBaseContext *inContext);
	VError PutSetReply(Bittab* inSet, CDB4DBaseContext *inContext);
	VError PutFicheInMemReply(FicheInMem* inFiche, CDB4DBaseContext *inContext);

	VError PutQueryResultReply(QueryResult* inResult, BaseTaskInfo *inContext);

	VError PutArrayLongReply(sLONG* inArray, sLONG nblongs);

	VError PutCollectionReply(DB4DCollectionManager& inCollection, bool fullyCompleted, sLONG maxelems);

	VError PutValueReply(const VValueSingle* inVal);
	
	VError PutNumFieldArrayReply( const NumFieldArray& inNumFieldArray);

	VError GetLastErrorReply()
	{
		return GetOutputStream()->GetLastError();
	}

	VError PutColumnFormulasResultReply(ColumnFormulas* formulas);

	VError PutUpdatedInfo(Base4D* base, BaseTaskInfo *context);
	VError PutUpdatedInfo(Table* inTable, BaseTaskInfo *context);
	VError PutUpdatedInfo(Table* inTable1, Table* inTable2, BaseTaskInfo *context);


	void ReplyFailed() { ; }; // a faire

};



// --------------------------------------------------------------------------------------------- //



void RemoveExtension(VString& s);



class CompareLessStringWithOptions
{
public:
	inline CompareLessStringWithOptions(const VString* ref, const VCompareOptions& inOptions)
	{
		fRef = ref;
		fOptions = &inOptions;
	};

	inline bool operator()(const VString* s1, const VString* s2) const
	{
		if (s1 == fRef)
			return s2->CompareToSameKindWithOptions(s1, *fOptions) == CR_BIGGER;
		else
			return s1->CompareToSameKindWithOptions(s2, *fOptions) == CR_SMALLER;
	};

protected:
	const VString* fRef;
	const VCompareOptions* fOptions;
};

void SetCompOptionWithOperator(VCompareOptions& inOptions, sLONG compOperator);

inline sLONG RemoveTheLike(sLONG comp)
{
	return comp - ((sLONG)DB4D_Like - (sLONG)DB4D_Equal);
}

inline Boolean InLikeRange(sLONG comp)
{
	return ( comp >= (sLONG)DB4D_Like && comp <= (sLONG)DB4D_Doesnt_Contain_KeyWord_Like );
}


VError PutVCompareOptionsIntoStream(const VCompareOptions& options, VStream& into);
VError GetVCompareOptionsFromStream(VCompareOptions& options, VStream& from);

/*
utile pour client perforce

//depot/... //LR_Home_4/... 
-//depot/4eDimension/11/... //LR_Home_4/4eDimension/11/... 
-//depot/4eDimension/11.1/... //LR_Home_4/4eDimension/11.1/... 
-//depot/4eDimension/2003/... //LR_Home_4/4eDimension/2003/... 
-//depot/4eDimension/2004/... //LR_Home_4/4eDimension/2004/... 
-//depot/4eDimension/11/... //LR_Home_4/4eDimension/11/... 
-//depot/4eDimension/11.1/... //LR_Home_4/4eDimension/11.1/... 
-//depot/4eDimension/Resources/11/... //LR_Home_4/4eDimension/Resources/11/... 
-//depot/4eDimension/Resources/11.1/... //LR_Home_4/4eDimension/Resources/11.1/... 
-//depot/4eDimension/Resources/2004/... //LR_Home_4/4eDimension/Resources/2004/... 
-//depot/Components/11/... //LR_Home_4/Components/11/... 
-//depot/Components/11.1/... //LR_Home_4/Components/11.1/... 
-//depot/GoldFinger/... //LR_Home_4/GoldFinger/... 
-//depot/MacToLinux/... //LR_Home_4/MacToLinux/... 
-//depot/web2.0pack/... //LR_Home_4/web2.0pack/... 
-//depot/Flex/... //LR_Home_4/Flex/... 
-//depot/WebSTAR/... //LR_Home_4/WebSTAR/... 
-//depot/WebSTAR.5.2/... //LR_Home_4/WebSTAR.5.2/... 
-//depot/XToolbox/11/... //LR_Home_4/XToolbox/11/... 
-//depot/XToolbox/11.1/... //LR_Home_4/XToolbox/11.1/... 


*/

class FicheOnDisk;
class Blob4D;


class RecordIDInTrans
{
	public:
		RecordIDInTrans(const FicheOnDisk* ficD);
		RecordIDInTrans(const Blob4D* blob);
		inline RecordIDInTrans(sLONG TableID, sLONG RecID) { fTableID = TableID; fNumRec = RecID; };
		sLONG fTableID;
		sLONG fNumRec;
};


VIntlMgr* GetContextIntl(const BaseTaskInfo* context);

UniChar GetWildChar(const BaseTaskInfo* context);

#if debugOverlaps_strong

extern map<RecordIDInTrans, DataAddr4D> debug_MapBlobAddrInTrans;

Boolean debug_CheckBlobAddrInTrans(const RecordIDInTrans& blobID, DataAddr4D addr);
void debug_AddBlobAddrInTrans(const RecordIDInTrans& blobID, DataAddr4D addr);
void debug_DelBlobAddrInTrans(const RecordIDInTrans& blobID);

#endif


#if debugLeakCheck_Strong

extern uBOOL debug_candumpleaks;
extern uBOOL debug_canRegisterLeaks;


typedef map<const VObject*, VStackCrawl> debug_mapOfStacks;
extern debug_mapOfStacks debug_Stacks;
extern VCriticalSection debug_StacksMutex;

void RegisterStackCrawl(const VObject* obj);
void UnRegisterStackCrawl(const VObject* obj);
void DumpStackCrawls();



typedef pair<const VObject*, sLONG> debug_VObjRef;
typedef map<debug_VObjRef, VStackCrawl> debug_RefmapOfStack;
extern debug_RefmapOfStack debug_RefStacks;

typedef map<VTaskID, sLONG> debug_mapOfThread;
extern debug_mapOfThread debug_Threads;

void RegisterRefStackCrawl(const VObject* obj);
void UnRegisterRefStackCrawl(const VObject* obj);
void DumpRefStackCrawls();

static void xStartRecordingMemoryLeaks()
{
	debug_canRegisterLeaks = true;
}

static void xStopRecordingMemoryLeaks()
{
	debug_canRegisterLeaks = false;
}

void xDumpMemoryLeaks(VString& outText);


#endif



// ----------------------------------------------------


template <class T>
class EventLoggerElem
{
public:
	inline EventLoggerElem()
	{
		fTaskID = 0;
	}

	inline EventLoggerElem(const VString& inMessage, const T& inData):fData(inData), fMessage(inMessage)
	{
		fTaskID = VTask::GetCurrentID();
	}

	inline void Set(const VString& inMessage, const T& inData)
	{
		fMessage = inMessage;
		fData = inData;
		fTaskID = VTask::GetCurrentID();
	}

	VString fMessage;
	VTaskID fTaskID;
	T fData;
};

template <class T, sLONG MaxElems>
class EventLogger
{
public:
	EventLogger()
	{
		fCurElem = -1;
		fElems.resize(MaxElems);
	}

	void AddMessage(const VString& inMessage, const T& inData)
	{
		VTaskLock lock(&fMutex);
		fCurElem++;
		if (fCurElem >= MaxElems)
			fCurElem = 0;
		fElems[fCurElem].Set(inMessage, inData);
	}

protected:
	VCriticalSection fMutex;
	vector<EventLoggerElem<T> > fElems;
	sLONG fCurElem;
};




// ----------------------------------------------------


typedef enum
{
	RWS_NoAccess = 0,
	RWS_ReadOnly,
	RWS_WriteExclusive,
	RWS_FreeMemAccess
	//RWS_WriteExclusiveAfterRead
} ReadWriteSemaphore_Access;


class dVSyncEvent;


class ReadWriteSemaphore_PendingAction
{
public:
	/*
	inline ReadWriteSemaphore_PendingAction()
	{
	fWaitingEvent = nil;
	fWhatAccess = RWS_NoAccess;
	}
	*/

	inline bool operator ==(const ReadWriteSemaphore_PendingAction& other) const
	{
		return fToken == other.fToken;
	}

	dVSyncEvent* fWaitingEvent;
	ReadWriteSemaphore_Access fWhatAccess;
	sLONG fToken;
	sLONG fReadWaiting;
	bool fUnlocked;
};

#if debugrws2

class dbgRWS
{
public:
	dbgRWS()
	{
		fAccessRights = RWS_NoAccess;
		fWaitingEvent = nil;
		fToken = 0;
		fReadCount = 0;
		fWriteCount = 0;
		fFreeMemEnCours = 0;
		fWriteOwner = 0;
		fPendingAction.fWaitingEvent = nil;
		fPendingAction.fWhatAccess = RWS_NoAccess;
		fPendingAction.fToken = 0;
		fPendingAction.fReadWaiting = 0;
		fPendingAction.fUnlocked = false;
	}

	dbgRWS(ReadWriteSemaphore_Access inAccessRights, sLONG inReadCount,  sLONG inWriteCount, VTaskID inWriteOwner, sLONG inFreeMemEnCours,
			dVSyncEvent* inWaitingEvent = nil, sLONG inToken = 0, ReadWriteSemaphore_PendingAction* inPendingAction = nil)
	{
		fAccessRights = inAccessRights;
		fWaitingEvent = inWaitingEvent;
		fToken = inToken;
		fReadCount = inReadCount;
		fWriteCount = inWriteCount;
		fFreeMemEnCours = inFreeMemEnCours;
		fWriteOwner = inWriteOwner;
		if (inPendingAction == nil)
		{
			fPendingAction.fWaitingEvent = nil;
			fPendingAction.fWhatAccess = RWS_NoAccess;
			fPendingAction.fToken = 0;
			fPendingAction.fReadWaiting = 0;
			fPendingAction.fUnlocked = false;
		}
		else
			fPendingAction = *inPendingAction;
	}

	ReadWriteSemaphore_Access fAccessRights;
	ReadWriteSemaphore_PendingAction fPendingAction;
	sLONG fReadCount;
	sLONG fWriteCount;
	sLONG fFreeMemEnCours;
	VTaskID fWriteOwner;
	dVSyncEvent* fWaitingEvent;
	sLONG fToken;
};


//EventLogger<dbgRWS, 3000> gDebugRWS;

#endif

class dVSyncEvent;

typedef set<VTaskID> TaskIDSet;
typedef map<VTaskID, dVSyncEvent*> SyncEventMapByTask;

class dVSyncEvent : public VSyncEvent
{
#if debugsyncevent
	public:
		dVSyncEvent()
		{
			fLastUnlocker = 0;
		}

		~dVSyncEvent()
		{
			if (!fEnAttente.empty())
			{
				sLONG i = 0; // break here;
				xbox_assert(false);
			}
		}

		bool Lock()										
		{ 
			VTaskID curtaskid = VTask::GetCurrentID();
			{
				VTaskLock lock(&sMutex);
				sEventsByTask[curtaskid] = this;
				fEnAttente.insert(curtaskid);
			}
			bool result = VSyncEvent::Lock();
			{
				VTaskLock lock(&sMutex);
				sEventsByTask[curtaskid] = nil;
				fEnAttente.erase(curtaskid);
			}
			return result;
		}


		bool Lock( sLONG inTimeoutMilliseconds)
		{ 
			VTaskID curtaskid = VTask::GetCurrentID();
			{
				VTaskLock lock(&sMutex);
				sEventsByTask[curtaskid] = this;
				fEnAttente.insert(curtaskid);
			}
			bool result =  VSyncEvent::Lock(inTimeoutMilliseconds);
			{
				VTaskLock lock(&sMutex);
				sEventsByTask[curtaskid] = nil;
				fEnAttente.erase(curtaskid);
			}
			return result;
		}


		bool TryToLock()									
		{ 
			return VSyncEvent::TryToLock();
		}

		// Signal the event (all waiting tasks are unblocked)
		bool Unlock() 
		{ 
			VTaskLock lock(&sMutex);
			if (fLastUnlocker != 0)
			{
				fLastUnlocker = fLastUnlocker; // break here
			}
			if (fEnAttente.empty())
			{
				if (fLastUnlocker == 0)
				{
					fLastUnlocker = fLastUnlocker; // break here
				}
			}
			if (fLastUnlocker == 0)
				fEtaitEnAttente = fEnAttente;
			fLastUnlocker = VTask::GetCurrentID();
			return VSyncEvent::Unlock();
		}

		bool IsStillLocked()
		{
			VTaskLock lock(&sMutex);
			return fLastUnlocker == 0;
		}

	protected:
		TaskIDSet fEnAttente;
		TaskIDSet fEtaitEnAttente;
		VTaskID fLastUnlocker;

		static SyncEventMapByTask sEventsByTask;
		static VCriticalSection sMutex;
		
#endif

};


typedef std::vector<ReadWriteSemaphore_PendingAction> RWS_PendingActionVector;

class ReadWriteSemaphore
{
	public:
		ReadWriteSemaphore();
#if VERSIONDEBUG
		virtual ~ReadWriteSemaphore();
#endif

		bool TryToLock(ReadWriteSemaphore_Access inAccessRights);
		void Lock(ReadWriteSemaphore_Access inAccessRights);
		void Unlock(bool WasFreeMem = false);


		bool TryToLockWrite()
		{
			return TryToLock(RWS_WriteExclusive);
		}

		void LockWrite()
		{
			Lock(RWS_WriteExclusive);
		}

		bool TryToLockRead()
		{
			return TryToLock(RWS_ReadOnly);
		}

		void LockRead()
		{
			Lock(RWS_ReadOnly);
		}

		bool TryToLockFreeMem()
		{
			return TryToLock(RWS_FreeMemAccess);
		}

		void LockFreeMem()
		{
			Lock(RWS_FreeMemAccess);
		}
		
		sLONG GetCurrentTaskReadOwnerCount();

#if debugrws2
		void checkPendingActions();
		static sLONG sTotalCount;
#endif
		
protected:
		VCriticalSection fMutex;
		VTaskID fWriteOwner;
		sLONG fReadCount;
		sLONG fWriteCount;
		sLONG fFreeMemEnCours;
		//sLONG fCumulReadEnWrite;
		uLONG fCurrentToken;
		uLONG fNextToken;
		RWS_PendingActionVector fPendingActions;
		map<VTaskID, sLONG> fReadOwnerCount;

#if debugrws2
		EventLogger<dbgRWS, 2000> fDebugRWS;
#endif

#if debugrws
		uBOOL fDebugMustLog;
#endif
};


class IReadWriteSemaphorable
{
	public:
		bool TryToLockWrite()
		{
			return fSemaphore.TryToLockWrite();
		}

		void LockWrite()
		{
			fSemaphore.LockWrite();
		}

		bool TryToLockRead()
		{
			return fSemaphore.TryToLockRead();
		}

		void LockRead()
		{
			fSemaphore.LockRead();
		}

		bool TryToLockFreeMem()
		{
			return fSemaphore.TryToLockFreeMem();
		}

		void LockFreeMem()
		{
			fSemaphore.LockFreeMem();
		}

		void Unlock(bool WasFreeMem = false)
		{
			fSemaphore.Unlock(WasFreeMem);
		}

		ReadWriteSemaphore* GetSemaphore() const
		{
			return &fSemaphore;
		}

	protected:
		mutable ReadWriteSemaphore fSemaphore;

};

void Test_ReadWriteSemaphore( sLONG inCountTasks);


class StLockerRead
{ 
public:
	StLockerRead( ReadWriteSemaphore* inSyncObject) : fSyncObject( inSyncObject)	
	{ 
		fSyncObject->LockRead(); 
	}

	StLockerRead(const IReadWriteSemaphorable* inSyncObject)
	{
		fSyncObject = inSyncObject->GetSemaphore();
		fSyncObject->LockRead();
	}


	~StLockerRead()												
	{ 
		fSyncObject->Unlock(); 
	}

private:
	ReadWriteSemaphore*	fSyncObject;
};


class StLockerWrite
{ 
public:
	StLockerWrite( ReadWriteSemaphore* inSyncObject) : fSyncObject( inSyncObject)	
	{ 
		fSyncObject->LockWrite(); 
	}

	StLockerWrite(const IReadWriteSemaphorable* inSyncObject)
	{
		fSyncObject = inSyncObject->GetSemaphore();
		fSyncObject->LockWrite();
	}

	~StLockerWrite()												
	{ 
		fSyncObject->Unlock(); 
	}

private:
	ReadWriteSemaphore*	fSyncObject;
};




// ----------------------------------------------------



class ProgressEncapsuleur
{
	public:
		ProgressEncapsuleur(VProgressIndicator* inProgress);
		ProgressEncapsuleur(VProgressIndicator* inProgress, sLONG8 inMaxValue, const VString& inMessage, bool inCanInterrupt = false);
		~ProgressEncapsuleur();

		bool Progress(sLONG8 inCurValue);
		bool Increment(sLONG8 inInc=1);
		void BeginSession(sLONG8 inMaxValue, const VString& inMessage, bool inCanInterrupt = false);
		void EndSession();

	protected:
		uLONG fStartingTime;
		VProgressIndicator* fProgress;
		sLONG8 fMaxValue;
		VString fMessage;
		bool fIsStarted, fCanInterrupt;

};



template<class vect, class elem>
class IPartable
{
	public:
		inline IPartable()
		{
			fBeforeFirst = true;
		}

		inline elem GetElem(typename vect::const_iterator iter) const
		{
			return *iter;
		}

		inline size_t Count() const
		{
			return fParts.size();
		}

		elem CurPart() const
		{
			if (fBeforeFirst)
				fCurrent = fParts.begin();
			fBeforeFirst = false;
			if (fCurrent != fParts.end())
				return GetElem(fCurrent);

			else
				return nil;
		}

		elem NextPart() const
		{
			if (fBeforeFirst)
				fCurrent = fParts.begin();
			else
			{
				if (fCurrent != fParts.end())
					fCurrent++;
			}
			fBeforeFirst = false;

			if (fCurrent != fParts.end())
				return GetElem(fCurrent);

			else
				return nil;
		}


		elem FirstPart() const
		{
			fBeforeFirst = false;
			fCurrent = fParts.begin();
			if (fCurrent != fParts.end())
				return GetElem(fCurrent);

			else
				return nil;
		}

		elem LastPart() const
		{
			sLONG nb = (sLONG)fParts.size();
			if (nb == 0)
				return nil;
			else
				return GetElem(fParts.begin()+(nb-1));
		}

		const vect* GetAll() const
		{
			return &fParts;
		}

		void copyfrom(const IPartable& other)
		{
			fParts = other.fParts;
			fBeforeFirst = other.fBeforeFirst;
			fCurrent = fParts.begin();
		}

		typename vect::const_iterator GetCurrent() const
		{
			if (fBeforeFirst)
				fCurrent = fParts.begin();
			fBeforeFirst = false;
			return fCurrent;
		}

		void SetCurrent(typename vect::const_iterator current)
		{
			fCurrent = current;
			fBeforeFirst = false;
		}

	protected:
		vect fParts;
		mutable typename vect::const_iterator fCurrent;
		mutable bool fBeforeFirst;
};


// ----------------------------------------------------

void debug_SetCurrentWritingPage(DataAddr4D ou);
void debug_ClearCurrentWritingPage();
void debug_SetPageRef(IndexInfo* ind, sLONG pagenum, DataAddr4D ou, sLONG len);
void debug_ClearPageRef(DataAddr4D ou, sLONG len, IndexInfo* ind);
void debug_CheckWriting(DataAddr4D ou, sLONG len);


// -----------------------------------------------------

class ENTITY_API Correspondance
{
	public:
		Correspondance(const ConstUniCharPtr* from)
		{
			From(from);
		}

		void From(const ConstUniCharPtr* from);

		const VString& MatchFrom(const VString& fromString) const;
		const VString& MatchTo(const VString& toString) const;

	protected:
		typedef map<VString, VString, CompareLessVStringNoIntl> mapOfVString;
		mapOfVString fFrom;
		mapOfVString fTo;

};


VError SaveBagToXML(const VValueBag& inBag, const VString& inRootElementKind, VFile& outFile, bool overwrite = true, const Correspondance* correspond = nil );
VError SaveBagToXML(const VValueBag& inBag, const VString& inRootElementKind, VString& outXML, bool prettyformat = false, const Correspondance* correspond = nil, bool withheader = false );


VError SaveBagToJson(const VValueBag& inBag, VFile& outFile, bool overwrite = true);
VError SaveBagToJson(const VValueBag& inBag, VString& outJSON, bool prettyformat = false);



class ENTITY_API Wordizer
{
	public:
		Wordizer(const VString& xinput):input(xinput)
		{
			curpos = 0;
		}

		inline sLONG GetCurPos() const
		{
			return curpos;
		}

		inline void SetCurPos(sLONG pos)
		{
			curpos = pos;
		}

		void RemoveExtraSpaces(VString& s);

		bool GetNextWord(VString& result, UniChar separator, bool skipleadingspaces, bool keepquotes);

		void ExctractStrings(VectorOfVString& outStrings, bool removeExtraSpaces, UniChar separator, bool keepquotes);

	protected:
		const VString& input;
		sLONG curpos;

};


inline bool MustStopTask( VProgressIndicator *inProgressIndicator = NULL)
{
	if ( (inProgressIndicator != NULL) && inProgressIndicator->IsInterrupted())
		return true;

	/*
		stop operations in progress if task has been called more than 3" ago
	*/
	sLONG dyingSinceMilliseconds;
	bool isDying = VTask::GetCurrent()->IsDying( &dyingSinceMilliseconds);
	return isDying && (dyingSinceMilliseconds > 3000);
}


bool ENTITY_API okperm(BaseTaskInfo* context, const VUUID& inGroupID);
bool ENTITY_API okperm(CDB4DBaseContext* context, const VUUID& inGroupID);
bool ENTITY_API okperm(BaseTaskInfo* context, const EntityModel* model, DB4D_EM_Perm perm);

bool promoteperm(BaseTaskInfo* context, const VUUID& inGroupID);
bool promoteperm(CDB4DBaseContext* context, const VUUID& inGroupID);

void endpromote(BaseTaskInfo* context, const VUUID& inGroupID);
bool endpromote(CDB4DBaseContext* context, const VUUID& inGroupID);

// -----------------------------------------------------

struct Feature
{
	sLONG	fFeatureFlag;
	sLONG	fMinVersionFor11;
	sLONG	fMinVersionFor12;
	sLONG	fMinVersionFor13;
};

inline bool VersionGreaterOrEqual( sLONG inVersion1, sLONG inVersion2)
{
	return ( ((inVersion1 >> 16) & 0xffff) | (inVersion1 << 16)) >= ( ((inVersion2 >> 16) & 0xffff) | (inVersion2 << 16));
}

extern const Feature feature_SendSubTableID;


// -----------------------------------------------------

typedef enum { objrefdebug_fiche, objrefdebug_blob, objrefdebug_tabaddr, objrefdebug_bittab, objrefdebug_bittabmap } objrefdebug;

class debug_dbobjref
{
	public:

		objrefdebug fType;
		virtual bool EqualTo(const debug_dbobjref& other) const = 0;

		
};


class debug_FicheRef : public debug_dbobjref
{
	public:

		inline debug_FicheRef(sLONG inTableNum, sLONG inRecNum)
		{
			fType = objrefdebug_fiche;
			fTableNum = inTableNum;
			fRecNum = inRecNum;		
		}


		virtual bool EqualTo(const debug_dbobjref& other) const
		{
			if (fType == other.fType)
			{
				const debug_FicheRef* xother = (const debug_FicheRef*)&other;
				return (fTableNum == xother->fTableNum && fRecNum == xother->fRecNum);
			}
			else
				return false;
		}

		sLONG fTableNum;
		sLONG fRecNum;
};


class debug_BlobRef : public debug_dbobjref
{
	public:

		inline debug_BlobRef(sLONG inTableNum, sLONG inBlobNum)
		{
			fType = objrefdebug_blob;
			fTableNum = inTableNum;
			fBlobNum = inBlobNum;		
		}

		virtual bool EqualTo(const debug_dbobjref& other) const
		{
			if (fType == other.fType)
			{
				const debug_BlobRef* xother = (const debug_BlobRef*)&other;
				return (fTableNum == xother->fTableNum && fBlobNum == xother->fBlobNum);
			}
			else
				return false;
		}

		sLONG fTableNum;
		sLONG fBlobNum;
};

typedef enum { addrtable_RecRef, addrtable_BlobRef, addrtable_BtreePageRef, addrtable_ClusterPageRef} AddrTable_elemRef;

class debug_AddrTableRef : public debug_dbobjref
{
public:

	inline debug_AddrTableRef(sLONG inTableNum, bool forRecords)
	{
		fType = objrefdebug_tabaddr;
		fTableNum = inTableNum;
		if (forRecords)
			fElemKind = addrtable_RecRef;
		else
			fElemKind = addrtable_BlobRef;
	}

	virtual bool EqualTo(const debug_dbobjref& other) const
	{
		if (fType == other.fType)
		{
			const debug_AddrTableRef* xother = (const debug_AddrTableRef*)&other;
			return (/*fTableNum == xother->fTableNum &&*/ fElemKind == xother->fElemKind);
		}
		else
			return false;
	}

	sLONG fTableNum;
	AddrTable_elemRef fElemKind;
};



class debug_DataAddr
{
	public:
		inline bool operator <(const debug_DataAddr& other) const
		{
			return fAddr < other.fAddr;
		}
		
		inline debug_DataAddr(DataAddr4D inAddr, sLONG inLen)
		{
			fAddr = inAddr;
			fLen = inLen;
		}

		DataAddr4D fAddr;
		sLONG fLen;
};


typedef map<debug_DataAddr, debug_dbobjref*> Debug_dbobjrefMap;
typedef map<sLONG, Debug_dbobjrefMap> Debug_dbobjrefMapBySeg;


// code sale en attendant quelque chose de Stephane
//extern RIApplicationRef global_ApplicationRef;


template <class T>
T xAbs(T value)
{
	if (value < 0)
		return -value;
	else
		return value;
}

inline uLONG8 getTimeStamp()
{
	VTime now;
	VTime::Now(now);
	return now.GetStamp();
}


extern bool GetNextWord(const VString& input, sLONG& curpos, VString& result);



			// --------------------------------------------------

class ENTITY_API DB4DLogger : public IDebugRefCountable
{
	public:
		DB4DLogger(VFile* logfile, bool apppendAtTheEnd, VJSONObject* options);
		VError StartLogging();
		VError StopLogging();
		VError Flush();

		VJSONObject* GetOptions()
		{
			return fOptions;
		}

		VError Write(VJSONObject* obj);

	protected:
		virtual ~DB4DLogger();
		VTime fLastTimeFlushed;
		VCriticalSection fMutex;
		VFile* fLogFile;
		VFileStream* fLog;
		VJSONObject* fOptions;
};



			// --------------------------------------------------


class AttributeStringPath : public IPartable<VectorOfVString, const VString*>
{
public:
	AttributeStringPath(const VString& inPath);
	AttributeStringPath()
	{
	}

	void FromString(const VString& inPath);
	void GetString(VString& outString) const;

};


/*
class ENTITY_API JSONPath : public AttributeStringPath
{
};
*/

typedef AttributeStringPath JSONPath;

typedef VectorOfVString::const_iterator JSONPathPosition;


// --------------------------------------------------
// Monitoring related utility function
// --------------------------------------------------
VJSONObject* RetainRequestOriginatorInfo(CDB4DBaseContext* inContext);

#endif









