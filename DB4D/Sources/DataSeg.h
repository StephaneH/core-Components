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
#ifndef __DATASEG__
#define __DATASEG__

const uLONG tagRec4D = 0x44014403;

const sLONG ktaillebloc = 128;
const sLONG kratio = 7;

const sLONG NBbitparpage = 16384;
const sLONG ratiopagebit = 14;

const sLONG NBpagedebit = 8192;		// chaque page secondaire contiendra 8192 addr de page de bit
const sLONG kratiosec = 13;

const sLONG NBbitparpagesec = NBbitparpage * NBpagedebit;

const sLONG NBPagedePage = 65536; // la page primaire (si elle existe) contiendra au maximum 65536 addr de page secondaire 
// par ex, pour des block de 128 octets, il n'y aura de table primaire qu'au dela de 17 179 869 184 octets (17G)
const sLONG sizetfb = NBpagedebit * sizeof(DataAddr4D);										// taille d'une page secondaire (64K)
const sLONG sizepagebit = NBbitparpage / 8;																// taille d'un freebit (2048 bytes)
const sLONG NBlongbit = sizepagebit / 4;
const sLONG sizetfbprim = NBPagedePage * sizeof(DataAddr4D);										// taille de la page primaire (512K)

const sLONG sizefbmaxlibre = NBpagedebit * sizeof(sWORD);		// 16K
const sLONG sizefbmaxlibreprim = NBPagedePage * sizeof(sWORD);


const sLONG ratiogroupebit = kratio + ratiopagebit;												// ratio pour le shift right qui sert de division pour la valeur ci-dessous
const sLONG taillegroupebit = NBbitparpage*ktaillebloc;										// nombre de bytes par groupe de 16384  blocs de 128

const sLONG ratiogroupebitsec = kratio + ratiopagebit + kratiosec;							// ratio pour le shift right qui sert de division pour la valeur ci-dessous
const DataAddr4D taillegroupesec = (uLONG8)taillegroupebit * (uLONG8)NBpagedebit; // 17G = taille d'un bloc secondaire

const DataAddr4D taillemaxsegdata = (uLONG8)NBPagedePage * (uLONG8)NBpagedebit * (uLONG8)NBbitparpage * (uLONG8)ktaillebloc - 128;  // taille maximum d'un segment de donnees (en gros 1 millon de millars d'octets)

const DataAddr4D blockEOF = 0x00010000;



//typedef char DataBaseObjectType[4];

const DataBaseObjectType DBOH_Record = 'rec1';
const DataBaseObjectType DBOH_Record2 = 'rec2';
const DataBaseObjectType DBOH_MultiSegHeader = 'segs';
const DataBaseObjectType DBOH_IndexTable = 'inds';
const DataBaseObjectType DBOH_TableTable = 'tbls';
const DataBaseObjectType DBOH_SetDisk = 'set ';
const DataBaseObjectType DBOH_SetDiskTable = 'setT';
const DataBaseObjectType DBOH_Blob = 'blob';
const DataBaseObjectType DBOH_BlobText = 'blbT';
const DataBaseObjectType DBOH_BlobPict = 'blbP';
const DataBaseObjectType DBOH_BtreePage = 'treP';
const DataBaseObjectType DBOH_BtreePageFixSize = 'treF';
const DataBaseObjectType DBOH_PetiteSel = 'SelP';
const DataBaseObjectType DBOH_LongSel = 'SelL';
const DataBaseObjectType DBOH_BitSel = 'SelB';
const DataBaseObjectType DBOH_TableAddress = 'TabA';
const DataBaseObjectType DBOH_TableAddressWithStamps = 'Taba';
const DataBaseObjectType DBOH_ArrayLongFix = 'ArrL';
const DataBaseObjectType DBOH_Bittab = 'BitT';
const DataBaseObjectType DBOH_StructDefElem = '_DEF';
const DataBaseObjectType DBOH_TableDefElem = 'TDEF';
const DataBaseObjectType DBOH_IndexDefElem = 'iDEF';
const DataBaseObjectType DBOH_IndexInStructDefElem = 'IDEF';
const DataBaseObjectType DBOH_RelationDefElem = 'RDEF';
const DataBaseObjectType DBOH_ExtraTableProperties = 'xTAB';
const DataBaseObjectType DBOH_ExtraFieldProperties = 'xFLD';
const DataBaseObjectType DBOH_ExtraRelationProperties = 'xREL';
const DataBaseObjectType DBOH_ExtraDataBaseProperties = 'xDAT';
const DataBaseObjectType DBOH_AutoSeqNumberSimple = 'Seq1';
const DataBaseObjectType DBOH_AutoSeqNumberNoHole = 'Seq2';
const DataBaseObjectType DBOH_DataTable = 'DTab';
const DataBaseObjectType DBOH_FieldsIDInRec = 'fIDs';
const DataBaseObjectType DBOH_TableExtraElements = 'TExE';
const DataBaseObjectType DBOH_ExtraElement = 'ExEl';


class SegData;
class Base4D_NotOpened;
class SegData_NotOpened;

class RootDataBaseObjectHeader : public VObject
{
public:
	RootDataBaseObjectHeader(const void* data);
	virtual VError PutInto(VStream& outStream) = 0;
	virtual VError GetFrom(const VStream& inStream) = 0;
	virtual VError WriteInto(SegData* seg, DataAddr4D offset, VString* WhereFrom = nil) = 0;

	virtual VError WriteInto(Base4D* bd, DataAddr4D offset, VString* WhereFrom = nil, sLONG truelen = 0) = 0;
	virtual VError ReadFrom(Base4D* bd, DataAddr4D offset, ReadAheadBuffer* buffer = nil) = 0;

	virtual VError WriteInto(Base4D_NotOpened* bd, DataAddr4D offset, VString* WhereFrom = nil) = 0;
	virtual VError ReadFrom(Base4D_NotOpened* bd, DataAddr4D offset) = 0;

	virtual VError WriteInto(SegData_NotOpened* seg, DataAddr4D offset, VString* WhereFrom = nil) = 0;
	virtual VError ReadFrom(SegData_NotOpened* seg, DataAddr4D offset) = 0;

	virtual VError ReadFrom(VFileDesc* f, DataAddr4D offset) = 0;

	virtual VError ValidateTag(DataBaseObjectType WhatType, sLONG PosInParent, sLONG Parent) = 0;

	virtual VError ValidateCheckSum(const void* data, sLONG len) = 0;
	uLONG CalcCheckSum(sLONG len);
	inline Boolean NeedSwap() const { return fDataNeedSwap; };

	inline void SetSwapWhileWriting(Boolean b) { fNeedSwapWhileSaving = b; };

	virtual Boolean Match(DataBaseObjectType signature) = 0;

protected:
	void* fData;
	Boolean fDataNeedSwap;
	Boolean fNeedSwapWhileSaving;

};


class DataBaseObjectHeader : public RootDataBaseObjectHeader
{
public:
	DataBaseObjectHeader():RootDataBaseObjectHeader(nil) { };
	DataBaseObjectHeader(const void* data, sLONG len, DataBaseObjectType WhatType, sLONG PosInParent, sLONG Parent);
	virtual VError PutInto(VStream& outStream);
	virtual VError GetFrom(const VStream& inStream);
	virtual VError WriteInto(SegData* seg, DataAddr4D offset, VString* WhereFrom = nil);

	virtual VError WriteInto(Base4D* bd, DataAddr4D offset, VString* WhereFrom = nil, sLONG truelen = 0);
	virtual VError ReadFrom(Base4D* bd, DataAddr4D offset, ReadAheadBuffer* buffer = nil);

	virtual VError WriteInto(Base4D_NotOpened* bd, DataAddr4D offset, VString* WhereFrom = nil);
	virtual VError ReadFrom(Base4D_NotOpened* bd, DataAddr4D offset);

	virtual VError WriteInto(SegData_NotOpened* seg, DataAddr4D offset, VString* WhereFrom = nil);
	virtual VError ReadFrom(SegData_NotOpened* seg, DataAddr4D offset);

	virtual VError ReadFrom(VFileDesc* f, DataAddr4D offset);

	virtual VError ValidateTag(DataBaseObjectType WhatType, sLONG PosInParent, sLONG Parent);
	virtual VError ValidateCheckSum(const void* data, sLONG len);

	inline uLONG8 GetTimeStamp() const { return fDBOH.timestamp; };
	inline sLONG GetLen() const { return fDBOH.len; };
	inline DataBaseObjectType GetType() const { return fDBOH.type; };
	inline DataBaseObjectType GetPos() const { return fDBOH.pos; };
	inline DataBaseObjectType GetParent() const { return fDBOH.parent; };

	virtual Boolean Match(DataBaseObjectType signature);

protected:
	DataBaseObjectHeaderOnDisk fDBOH;
};

const sLONG kSizeDataBaseObjectHeader = sizeof(DataBaseObjectHeaderOnDisk);


// --------------


class RecordHeader : public RootDataBaseObjectHeader
{
public:
	RecordHeader():RootDataBaseObjectHeader(nil) { };
	RecordHeader(const void* data, sLONG len, DataBaseObjectType WhatType, sLONG PosInParent, sLONG Parent, sLONG nbfields);
	virtual VError PutInto(VStream& outStream);
	virtual VError GetFrom(const VStream& inStream);
	virtual VError WriteInto(SegData* seg, DataAddr4D offset, VString* WhereFrom = nil);

	virtual VError WriteInto(Base4D* bd, DataAddr4D offset, VString* WhereFrom = nil, sLONG truelen = 0);
	virtual VError ReadFrom(Base4D* bd, DataAddr4D offset, ReadAheadBuffer* buffer = nil);

	virtual VError WriteInto(Base4D_NotOpened* bd, DataAddr4D offset, VString* WhereFrom = nil);
	virtual VError ReadFrom(Base4D_NotOpened* bd, DataAddr4D offset);

	virtual VError WriteInto(SegData_NotOpened* seg, DataAddr4D offset, VString* WhereFrom = nil);
	virtual VError ReadFrom(SegData_NotOpened* seg, DataAddr4D offset);

	virtual VError ReadFrom(VFileDesc* f, DataAddr4D offset);

	virtual VError ValidateTag(DataBaseObjectType WhatType, sLONG PosInParent, sLONG Parent);
	virtual VError ValidateCheckSum(const void* data, sLONG len);

	VError ReadFromStream(VStream* log);
	VError WriteToStream(VStream* log);

	virtual Boolean Match(DataBaseObjectType signature);

	inline sLONG GetNbFields() const { return fDBOH.nbfields; };
	inline uLONG8 GetTimeStamp() const { return fDBOH.timestamp; };
	inline sLONG GetLen() const { return fDBOH.len; };
	inline DataBaseObjectType GetType() const { return fDBOH.type; };
	inline sLONG GetPos() const { return fDBOH.pos; };
	inline sLONG GetParent() const { return fDBOH.parent; };
	inline uLONG GetCheckSum() const { return fDBOH.checksum; };

	inline void SetTimeStamp(uLONG8 stamp)
	{
		fDBOH.timestamp = stamp;
	}

protected:
	RecordHeaderOnDisk fDBOH;
};

const sLONG kSizeRecordHeader = sizeof(RecordHeaderOnDisk);


/* ------------------------------------------- */

#if debugFindPlace_strong

class debug_FindFreeInfo
{
	public:
		inline debug_FindFreeInfo()
		{
			obj = nil;
		}

		void *obj;
		VStackCrawl fCrawl;

};



typedef map<DataAddr4D, debug_FindFreeInfo> debug_MapOfFindFree;

#endif

#if debugFindPlace_inMap
typedef map<DataAddr4D, sLONG> MapOfAddrs;
#endif


// pour les bittable et les FreebitsSec, il n' y jamais besoin d'encapsuler les manipulation d'object occupable par des OccupablePool::WaitForMemoryManager
// car tous les acces a la racine des datasegs se font en Write Exclusif



typedef uLONG FreeBits[512];
typedef FreeBits *FreeBitsptr;

class SegData;
class FreebitsSec;

class bittable : public ObjCacheArrayLongFix
{
public:
	inline bittable(Base4D *xdb, sLONG xhowmany) : ObjCacheArrayLongFix(xdb, xhowmany) 
	{ 
	};
	inline FreeBitsptr getfbb(void) { return((FreeBitsptr)LockAndGetptr()); };

#if debuglr
	virtual	sLONG		Retain(const char* DebugInfo = 0) const
	{
		return IDebugRefCountable::Retain(DebugInfo);
	}

	virtual	sLONG		Release(const char* DebugInfo = 0) const
	{
		if (GetRefCount() == 2 && modifie())
		{
			sLONG n = 1; // put a break here
		}
		return IDebugRefCountable::Release(DebugInfo);
	}
#endif


private:
	virtual ~bittable()
	{
	}

	FreebitsSec *parent;
	sLONG posdansparent;
};

typedef bittable *bittableptr;

typedef DataAddr4D FreeBitsPageOnDisk[NBpagedebit];
typedef bittableptr FreeBitsPageInMem[NBpagedebit];
typedef sWORD MaxLibreArray[NBpagedebit];

class FreebitsSec : public ObjInCacheMem, public IObjToFlush
{
public:
	FreebitsSec(SegData* segowner, sLONG NumInTable, DataAddr4D addr, uBOOL init = false);

	sLONG calcNBpages(void);
	VError chargetfb(void);
	bittableptr chargefb(sLONG i, VError& err, OccupableStack* curstack);
	void setfb(sLONG i, bittableptr fbt, DataAddr4D ou) { tfb[i] = ou; tfbm[i] = fbt; savetfb = true; };
	sWORD CalcMaxFree(void);
	sLONG GetMaxLibre(sLONG i) { return((sLONG)(fbmaxlibre[i])); };
	void SetMaxLibre(sLONG i, sLONG newmax) { fbmaxlibre[i] = (sWORD)newmax; savefbmaxlibre = true; };

	void setfbt(sLONG n, bittable *fbt);
	virtual bool SaveObj(VSize& outSizeSaved);

	bool TryToFree(sLONG allocationBlockNumber, VSize inNeedToFree, VSize& outFreed, bool ForceAll = false);

	sLONG GetNum() const { return No; }

	inline void SetCurrentNbPages(sLONG nbp)
	{
		fNbPagesEncours = nbp;
	}

	inline void SetAtLeastCurrentNbPages(sLONG nbp)
	{
		if (nbp > calcNBpages() && nbp > fNbPagesEncours)
			fNbPagesEncours = nbp;
	}

protected:
	virtual ~FreebitsSec();

	FreeBitsPageOnDisk tfb;
	FreeBitsPageInMem tfbm;
	MaxLibreArray fbmaxlibre;
	SegData *seg;
	sLONG No;
	sLONG fNbPagesEncours;
	uBOOL savetfb;
	uBOOL savefbmaxlibre;
};

typedef FreebitsSec *FreebitsSecPtr;

typedef V0ArrayOf<FreebitsSecPtr> FreebitsSecArray;

typedef V0ArrayOf<DataAddr4D> DiskAddrArray;

struct BaseHeader;


class debug_addrRange
{
public:
	inline debug_addrRange(IObjToFlush* obj, sLONG len)
	{
		fObj = obj;
		fLen = len;
	}

	IObjToFlush* fObj;
	sLONG fLen;
};

typedef map<DataAddr4D, debug_addrRange> debug_AddrMap;

typedef set<DataAddr4D> DataAddrSet;

class DataAddrSetForFlush
{
public:
	DataAddrSet fAddrs;
	DataAddrSet::iterator fCurrent;
	DataAddrSet::iterator fLast;
};

typedef vector<DataAddrSetForFlush> DataAddrSetVector;


class SegData : public ObjInCacheMem, public IObjToFlush, public IObjToFree
{
	friend class DataTools;

public:
	SegData( Base4D *inBase, sLONG inNumSeg, VFile *inFile, SegDataDisk2 *realSDD);
	SegData( Base4D *inBase, const BaseHeader& inSegInfo, sLONG inNumSeg, VFile *inFile, VFileDesc* inDesc, SegDataDisk2 *realSDD);

	inline DataAddr4D Getfinfic(void) { return(SDDx->finfic); };

	// clone given VFile, create it on disk
	VError CreateSeg(const VUUID* StructID, FileAccess inAccess);
	VError OpenSeg(FileAccess InAccess);

	VError CloseSeg();
	VError DeleteSeg();
	void GetFullPath( VFilePath& outPath) const;

	void PrendsBits(DataAddr4D ou, sLONG len );										// J.A. Tools 10/2004
	void PrendsBits(FreeBitsptr fb, sLONG start, sLONG end);
	void ClearBits(FreeBitsptr fb, sLONG start, sLONG end);
	Boolean TestBits(DataAddr4D ou, sLONG len, Boolean test = true);				// J.A. Tools 10/2004
	Boolean	TestBits(FreeBitsptr fb, sLONG start, sLONG end,Boolean test = true);	// J.A. Tools 10/2004
	VError chargetfb(void);

	virtual bool FreeMem(sLONG allocationBlockNumber, VSize combien, VSize& outSizeFreed);

	virtual bool SaveObj(VSize& outSizeSaved);

	DataAddr4D FindFree(sLONG len, void *obj, bool cangrow = true, DataAddr4D excludeAddr = 0, sLONG excludeLen = 0);
	VError LibereBits(DataAddr4D ou, sLONG len, void *obj);
	bittableptr chargefb(sLONG i, VError& err, OccupableStack* curstack);
	FreebitsSecPtr chargefbsec(sLONG i, VError& err, OccupableStack* curstack);
	sLONG maxbitfree(FreeBitsptr fb);
	void ResetMaxLibre(sLONG i, OccupableStack* curstack);
	//virtual sLONG saveobj(void);
	VError readat(void* p, sLONG len, DataAddr4D ou);
	VError writeat(void* p, sLONG len, DataAddr4D ou, sLONG TrueLen = -1);
	VError FlushBuffer();
	//		inline VFileVIT* getsegref(void) { return(segref); };

	inline SegDataDisk2* GetSDD(void) { return(SDDx); };
	inline VFile* GetFile(void) const { return(fFile); };
	inline VFileDesc* GetFileDesc(void) const { return(fFileDesc); };
	void FlushSeg() { FlushBuffer(); if (fFileDesc!=nil) fFileDesc->Flush(); }
	uBOOL IsAddrValid(DataAddr4D ou);

	sLONG CalcNbpageSec(void) { return ((SDDx->finfic+taillegroupesec-1)>>ratiogroupebitsec); };

	Base4D* GetDB(void) { return db; };

	VError ThrowError( VError inErrCode, ActionDB4D inAction);

	inline sLONG GetNum() const { return numseg; };

	void MarkBlockToFullyDeleted(DataAddr4D ou, sLONG len);

	void SwapFullyDeletedAddr(DataAddrSet& other)
	{
		VTaskLock lock(&fFullyDeletedMutex);
		fFullyDeletedAddr.swap(other);
	}

	inline ReadWriteSemaphore* GetSemaphore() 
	{ 
		return &fAccess; 
	}

#if debugFindPlace_strong
	void Debug_CheckAddrOverWrite(IObjToFlush* obj, DataAddr4D ou);
#endif


protected:
	virtual ~SegData();
	SegDataDisk2 SDDInside;
	VFile *fFile;
	VFileDesc *fFileDesc;
	FreebitsSecArray *tfbprim;
	V0ArrayOf<sWORD> *fbmaxlibreprim;
	DiskAddrArray *tfbdiskprim;
	sLONG numseg;
	uBOOL isopen;
	uBOOL savetfb;
	uBOOL savefbmaxlibre;
	uBOOL ecrisfinfic;
	uBOOL WriteProtected;
	Base4D *db;
	sLONG fLastPageSec;
	sLONG fLastPage;
	sLONG fLastBloc;
	sLONG fCountLibereBit;
	bool fOnePassOnly;
	SegDataDisk2 *SDDx;
	VCriticalSection fMutex;
	void* fWriteBuffer;
	sLONG fWriteBufferLen;
	sLONG fWriteBufferOffset;
	DataAddr4D fWriteBufferStart;
	DataAddrSet fFullyDeletedAddr;
	VCriticalSection fFullyDeletedMutex;
	ReadWriteSemaphore fAccess;

#if debugFindPlace_strong
	VCriticalSection debug_FindFreeMutex;
	debug_MapOfFindFree debug_FindFrees;
#endif


#if debugFindPlace_inMap
	MapOfAddrs fDebugAddrs;
#endif

#if debuglr == 126
	debug_AddrMap fDebugAddrMap;
#endif

};


class ToolLog;

class SegData_NotOpened : public ObjInCacheMem
{
public:
	SegData_NotOpened(Base4D_NotOpened *xdb);
	virtual ~SegData_NotOpened();
	void Init(VFile* inFile, sLONG xsegnum);
	void Init(VFileDesc* inFileDesc, sLONG xsegnum);
	VError Open(sLONG numseg, ToolLog *log, FileAccess access = FA_MAX);
	void Close();

	inline Boolean IsOpen() const { return fFileDesc != nil; };

	VError readat(void* p, sLONG len, DataAddr4D ou);
	VError writeat(void* p, sLONG len, DataAddr4D ou);
	uBOOL IsAddrValid(DataAddr4D ou, Boolean TestAppartenance = false);

	VError CheckBittables(ToolLog* log);
	VError CheckBitTableSec(ToolLog* log, DataAddr4D addrfbsec, sLONG No);
	VError CheckBitTable(ToolLog* log, DataAddr4D addrfb, sLONG numsec, sLONG No);

	sLONG CalcNbpageSec(void) { return ((finfic+taillegroupesec-1)>>ratiogroupebitsec); };
	DataAddr4D GetFinfic() const { return finfic; };
	Boolean IsHeaderValid() const { return fHeaderIsValid; };
	Boolean IsFinFicValid() const { return finficIsValid; };

	VError GetEOF(DataAddr4D *outSize) const { return fFile->GetSize(outSize); };

	const VFile* GetVFile() 
	{ 
		if (fFile != nil)
			return fFile;
		else
		{
			if (fFileDesc == nil)
				return nil;
			else
				return fFileDesc->GetParentVFile();
		}
	}

protected:
	SegDataDisk2 SDDInside;
	VFile *fFile;
	VFileDesc *fFileDesc;
	Base4D_NotOpened* db;
	Boolean fHeaderIsValid;
	VUUID fID;
	sLONG fSegNum;
	DataAddr4D finfic;
	Boolean finficIsValid;
	Boolean fIsAttached;
};

typedef SegData *SegDataPtr;

//typedef V0ArrayOf<SegDataPtr> SegDataArray;


inline sLONG adjuste(sLONG n) { return((n+ktaillebloc-1) & (-ktaillebloc)); };

const sLONG kSizeIncPageIndex=2048;

inline sLONG adjusteindex(sLONG n) { return((n+kSizeIncPageIndex-1) & (-kSizeIncPageIndex)); };

inline DataAddr4D StripDataAddr(DataAddr4D addr) { return addr & (-kMaxSegData); };



#endif


