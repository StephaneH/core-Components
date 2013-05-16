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
#include "4DDBHeaders.h"
#include <map>

ObjCountMap IObjCounter::sObjCount;

// extensions pour wakanda
const VString kWaDataFileExt = L".waData";
const VString kWaDataFileBlobExt = L".waBlobs";

const VString kWaDataIndexExt = L".waIndx";
const VString kWaStructIndexExt = L".waIndy";

const VString kWaSyncDataExt = L".4DSyncData";
const VString kWaSyncHeaderExt = L".4DSyncHeader";

const VString kWaDataExtraExt = L"waExtra";	// no dot, used with VFilePath

// extensions pour 4D
const VString k4DDataFileExt = L".4DD";
const VString k4DDataFileBlobExt = L".4DBlobs";

const VString k4DDataIndexExt = L".4DIndx";
const VString k4DStructIndexExt = L".4DIndy";

const VString k4DSyncDataExt = L".4DSyncData";
const VString k4DSyncHeaderExt = L".4DSyncHeader";

const VString k4DDataExtraExt = L"4DExtra";	// no dot, used with VFilePath

// les extensions a utiliser selon le mode de chargement de db4d
const VString kDataFileExt;
const VString kDataFileBlobExt;

const VString kDataIndexExt;
const VString kStructIndexExt;

const VString kDataMatchExt = L".Match"; // le meme pour 4D et Wakanda

const VString kSyncDataExt;
const VString kSyncHeaderExt;

const VString kDataExtraExt;


VStr4 ext_Struct("4XB");
VStr4 ext_Data("4XD");
VStr4 ext_Temp("tmp");
VStr4 ext_Pref("prf");

VCppMemMgr *gAlternateCppMem = nil;
VCppMemMgr *gCppMem = nil;

void* TaskIsFlushTask = (void*) 0x44444441;
void* TaskIsFaisPlaceTask = (void*) 0x44444442;

sLONG IOccupable::sNBActions = 0;


const Feature feature_SendSubTableID = {1,0xF0021170,0xF0021200,0xA0021300};	// 11.7 HF1, 12.0 HF1, 13.0 alpha 2


VDB4DProgressIndicatorWrapper::VDB4DProgressIndicatorWrapper(VDB4DProgressIndicator* inWrappedProgressIndicator):
    XBOX::VObject(),
    fProgressIndicator(nil)
{
    if (inWrappedProgressIndicator == nil)
    {
        fProgressIndicator = new VDB4DProgressIndicator(); 
    }
    else
    {
        fProgressIndicator = XBOX::RetainRefCountable(inWrappedProgressIndicator);
    }
}

bool VDB4DProgressIndicatorWrapper::SetTitleAfterKind(DB4D_ProgressIndicatorKind inKind)
{
    return SetProgressIndicatorTitleForKind(*fProgressIndicator,inKind);
}

VDB4DProgressIndicatorWrapper::~VDB4DProgressIndicatorWrapper()
{
    XBOX::ReleaseRefCountable(&fProgressIndicator);
}

bool SetProgressIndicatorTitleForKind(VDB4DProgressIndicator& inPi, DB4D_ProgressIndicatorKind inKind)
{
    XBOX::VString title;
    bool modified = true;
    switch(inKind)
    {
        case PK_UniquenessChecking:
            gs(1005,34,title);
			break;
		default:
            modified = false;
			break;
    }
    if(modified)
    {
        inPi.SetTitle(title);
    }
    return modified;
    
}

void trackDebugMsg(const VString& mess)
{
	sLONG ntask = VTask::GetCurrentID();
	DebugMsg("task id : "+ToString(ntask)+" , "+mess);
}

namespace Db4DError
{
	CREATE_BAGKEY( BaseName);
	CREATE_BAGKEY( TableName);
	CREATE_BAGKEY( FieldName);
	CREATE_BAGKEY( BlobNum);
	CREATE_BAGKEY( RecNum);
	CREATE_BAGKEY( IndexName);

	CREATE_BAGKEY( EntityModelName);
	CREATE_BAGKEY( EntityAttributeName);
	CREATE_BAGKEY( TypeName);
	CREATE_BAGKEY( EnumerationName);
	CREATE_BAGKEY( RelationName);
	CREATE_BAGKEY (EntityNum);
	CREATE_BAGKEY( MethodName);

	CREATE_BAGKEY( Param1);
	CREATE_BAGKEY( Param2);
	CREATE_BAGKEY( Param3);
/*
	CREATE_BAGKEY( p1);
	CREATE_BAGKEY( p2);
	CREATE_BAGKEY( p3);
*/

	CREATE_BAGKEY( jstext);
	CREATE_BAGKEY( jsline);
}


namespace DB4DRemoteBagKeys
{
	CREATE_BAGKEY_NO_DEFAULT( rm_name_Nto1, XBOX::VString);
	CREATE_BAGKEY_NO_DEFAULT( rm_name_1toN, XBOX::VString);
	CREATE_BAGKEY_NO_DEFAULT_SCALAR( rm_auto_load_Nto1, XBOX::VBoolean, bool);
	CREATE_BAGKEY_NO_DEFAULT_SCALAR( rm_auto_load_1toN, XBOX::VBoolean, bool);
	CREATE_BAGKEY_NO_DEFAULT( rm_integrity, XBOX::VString);
	CREATE_BAGKEY( rm_state);
	CREATE_BAGKEY_NO_DEFAULT_SCALAR( rm_foreign_key, XBOX::VBoolean, bool);

	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( lock_timeout, XBOX::VLong, sLONG, 0);
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( lock_for_readonly, XBOX::VBoolean, bool, false);
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( lock_with_fields, XBOX::VBoolean, bool, false);
	CREATE_BAGKEY_WITH_DEFAULT_SCALAR( lock_with_relatedfields, XBOX::VBoolean, bool, false);
};


// ----------------------------------------------------------------------------------------

void InitFileExtensions( DB4D_Running_Mode inRunningMode)
{
	if (inRunningMode == DB4D_RunningWakanda)
	{
		const_cast<VString&>( kDataFileExt) = kWaDataFileExt;
		const_cast<VString&>( kDataFileBlobExt) = kWaDataFileBlobExt;
		const_cast<VString&>( kDataIndexExt) = kWaDataIndexExt;
		const_cast<VString&>( kStructIndexExt) = kWaStructIndexExt;
		const_cast<VString&>( kSyncDataExt) = kWaSyncDataExt;
		const_cast<VString&>( kSyncHeaderExt) = kWaSyncHeaderExt;
		const_cast<VString&>( kDataExtraExt) = kWaDataExtraExt;
	}
	else if (testAssert( inRunningMode == DB4D_Running4D))
	{
		const_cast<VString&>( kDataFileExt) = k4DDataFileExt;
		const_cast<VString&>( kDataFileBlobExt) = k4DDataFileBlobExt;
		const_cast<VString&>( kDataIndexExt) = k4DDataIndexExt;
		const_cast<VString&>( kStructIndexExt) = k4DStructIndexExt;
		const_cast<VString&>( kSyncDataExt) = k4DSyncDataExt;
		const_cast<VString&>( kSyncHeaderExt) = k4DSyncHeaderExt;
		const_cast<VString&>( kDataExtraExt) = k4DDataExtraExt;
	}
}

// ----------------------------------------------------------------------------------------

StrHashTable::StrHashTable(const CoupleCharLong* pData, bool diac)
{
	char buffer[256];
	fIsDiacritical = diac;
	const CoupleCharLong* p = pData;
	bool cont = true;
	do 
	{
		if (*(p->pChar) != 0)
		{
			if (diac)
			{
				VValueBag::StKey key(p->pChar);
				fHashTable.SetLong(key, p->value);
			}
			else
			{
				char* p2 = &buffer[0];
				const char* p1 = p->pChar;
				while (*p1 != 0)
				{
					char c = *p1;
					if (c >=65 && c <= 90)
						c = c +32;
					*p2 = c;
					p2++;
					p1++;
				}
				*p2 = 0;
				VValueBag::StKey key(p->pChar);
				fHashTable.SetLong(key, p->value);
			}
		}
		else
			cont = false;
		p++;
	} while(cont);
}


sLONG StrHashTable::GetValue(const VString& valuename) const
{
	sLONG result;
	if (fIsDiacritical)
	{
		VValueBag::StKey key(valuename);
		if (!fHashTable.GetLong(key, result))
			result = -1;
	}
	else
	{
		VStr<256> localstring;
		sLONG len = valuename.GetLength();
		UniChar* p2 = localstring.GetCPointerForWrite(len+2);
		const UniChar* p1 = valuename.GetCPointer();
		for (sLONG i = 0; i < len; i++)
		{
			UniChar c = *p1;
			if (c >=65 && c <= 90)
				c = c +32;
			*p2 = c;
			p2++;
			p1++;
		}
		localstring.Validate(len);
		VValueBag::StKey key(localstring);
		if (!fHashTable.GetLong(key, result))
			result = -1;
	}

	return result;
}



// ----------------------------------------------------------------------------------------


Enumeration::Enumeration(const ConstUniCharPtr* from)
{
	From(from);
}


Enumeration::Enumeration(const CoupleCharLong* from)
{
	From(from);
}

Enumeration& Enumeration::operator = (const ConstUniCharPtr* from)
{
	From(from);
	return *this;
}

void Enumeration::From(const ConstUniCharPtr* from)
{
	const ConstUniCharPtr* p = from;
	bool cont = true;
	sLONG i = 1;
	do 
	{
		VString s(*p);
		if (s.IsEmpty())
			cont = false;
		else
		{
			fEnums[s] = i;
			fEnumIDs[i] = s;
		}
		i++;
		p++;
	} while(cont);
}



void Enumeration::From(const CoupleCharLong* from)
{
	const CoupleCharLong* p = from;
	bool cont = true;
	do 
	{
		VString s(p->pChar);
		if (s.IsEmpty())
			cont = false;
		else
		{
			fEnums[s] = p->value;
			fEnumIDs[p->value] = s;
		}
		p++;
	} while(cont);
}


sLONG Enumeration::operator[] (const VString& EnumName) const
{
	EnumMap::const_iterator found = fEnums.find(EnumName);
	if (found == fEnums.end())
		return 0;
	else
		return found->second;
}


const VString& Enumeration::operator [] (sLONG EnumID) const
{
	EnumIDMap::const_iterator found = fEnumIDs.find(EnumID);
	if (found == fEnumIDs.end())
		return sEmptyString;
	else
		return found->second;
}



VString Enumeration::sEmptyString;


// ----------------------------------------------------------------------------------------



void SimpleTextProgressIndicator::DoBeginSession(sLONG inSessionNumber)
{
	VString s2(L"Start of : ");
	VString s;
	GetMessage(s);
	s2.AppendString(s);
	s2.AppendUniCString(L"\n");
	DebugMsg(s2);
	fLastTime = VSystem::GetCurrentTime();
}


void SimpleTextProgressIndicator::DoEndSession(sLONG inSessionNumber)
{
	VString s2(L"End of : ");
	VString s;
	GetMessage(s);
	s2.AppendString(s);
	s2.AppendUniCString(L"\n");
	DebugMsg(s2);
}


bool SimpleTextProgressIndicator::DoProgress()
{
	uLONG t = VSystem::GetCurrentTime();
	sLONG8 n = GetCurrentValue();

	if ( (t-fLastTime) >= 5000 )
	{
		VString mess;
		GetMessage(mess);
		DebugMsg(mess);
		DebugMsg(L" : ");
		VString s;
		s.FromLong8(n);
		DebugMsg(s);
		DebugMsg(L"\n");
		fLastTime = t;
	}

	return true;
}

SimpleTextProgressIndicator::~SimpleTextProgressIndicator()
{
	// mettre un break ici
	fLastTime = 0;
}


						// --------------------------------------------------


void AddrDBObj::SwapBytes()
{
	ByteSwap(&addr);
	ByteSwap(&len);
}

static UniChar	sTableNameForbiddenRanges[] = 
{
	0x0028, // (
	0x0029, // )
	0x002B, // +
	0x002D, // -
	0x002F, // /
	0x002A, // *
	0x005C, // backslash
	0x0022, // "
	0x003B, // ;
	0x003D, // =
	0x0026, // &
	0x007C, // |
	0x0023, // #
	0x003E, // >
	0x003C, // <
	0x005E, // ^
	0x0060, // `
	0x007B, // {
	0x007D, // }
	0x0025, // %
	0x00D7, // x
	0x00B3, // cube
	0x00B2, // square
	0x00B1, // plus-minus
};


static std::set<UniChar> sUniCharSet;
static Boolean sInit = false;

Boolean IsValid4DTableName(const VString& name)
{
	if (!sInit)
	{
		// init an stl set with all the forbidden characters
		// it will later make it easy and fast to look for these characters
		sLONG nb_elem = sizeof(sTableNameForbiddenRanges) / sizeof(UniChar);
		while (nb_elem--)
			sUniCharSet.insert(sTableNameForbiddenRanges[nb_elem]);
		sInit = true;
	}

	Boolean isGood = true;

	if (name.IsNull()) return false;
	if (name.IsEmpty()) return false;
	
	VIndex len = name.GetLength();

	for (VIndex i = 1; i <= len; ++i)
	{
		UniChar c = name.GetUniChar(i);
		
		if ((i == 1) && VIntlMgr::GetDefaultMgr()->IsDigit(c))
		{
			// first character should not be a digit
			isGood = false;
			break;
		}
		
		if (sUniCharSet.count(c) > 0)
		{
			// this is a forbidden character
			isGood = false;
			break;
		}
	}
	
	return isGood;
}

VErrorDB4D BuildValid4DTableName( const VString& inName, VString& outValidName)
{
	if (!sInit)
	{
		// init an stl set with all the forbidden characters
		// it will later make it easy and fast to look for these characters
		sLONG nb_elem = sizeof(sTableNameForbiddenRanges) / sizeof(UniChar);
		while (nb_elem--)
			sUniCharSet.insert(sTableNameForbiddenRanges[nb_elem]);
		sInit = true;
	}
	
	outValidName.Clear();
	if (!inName.IsEmpty())
	{
		VIndex len = inName.GetLength();
		for (VIndex i = 1; i <= len; ++i)
		{
			UniChar c = inName.GetUniChar(i);

			if (sUniCharSet.count(c) > 0)
				// skip forbidden characters
				continue;

			if (outValidName.IsEmpty() && VIntlMgr::GetDefaultMgr()->IsDigit(c))
				// first character should not be a digit, then append an underscore
				outValidName.AppendUniChar( 0x005F);
		
			outValidName.AppendUniChar( c);
		}
	}
	return (::IsValid4DTableName( outValidName)) ? VE_OK : VE_DB4D_INVALIDTABLENAME;
}

Boolean IsValid4DFieldName( const VString& inName)
{
	if (inName.IsNull())
		return false;
	if (inName.IsEmpty())
		return false;
	
	Boolean isValid = true;
	
	UniChar c = inName.GetUniChar(1);
	if (VIntlMgr::GetDefaultMgr()->IsDigit(c))	// first character should not be a digit
		isValid = false;

	return isValid;
}

VErrorDB4D BuildValid4DFieldName( const VString& inName, VString& outValidName)
{
	outValidName.Clear();
	if (!inName.IsEmpty())
	{
		UniChar c = inName.GetUniChar(1);
		if (VIntlMgr::GetDefaultMgr()->IsDigit(c))	// first character should not be a digit, then append an underscore
			outValidName.AppendUniChar( 0x005F);

		outValidName.AppendString( inName);
	}
	return (::IsValid4DFieldName( outValidName)) ? VE_OK : VE_DB4D_INVALIDFIELDNAME;
}

Boolean IsValid4DName(const VString& name)
{
	if (name.IsNull()) return false;
	if (name.IsEmpty()) return false;

	return true;
}


inline VTaskDataKey GetFlushingTaskDataKey()
{
	static VTaskDataKey sKey = 0;
	if (sKey == 0)
		sKey = VTask::CreateDataKey( NULL);
	return sKey;
}


inline const void *GetTaskFlushingState( VTask *inTask)
{
	return VTask::GetCurrentData(GetFlushingTaskDataKey());
}


inline void SetTaskFlushingState( VTask *inTask, void *inData)
{
	VTask::SetCurrentData(GetFlushingTaskDataKey(), inData);
}


Boolean IsCurTaskFlushing(void)
{
	return (GetTaskFlushingState( VTask::GetCurrent()) == TaskIsFlushTask);
}

Boolean IsCurTaskFaisPlace(void)
{
	return (GetTaskFlushingState( VTask::GetCurrent()) == TaskIsFaisPlaceTask);
}


Boolean IsCurTaskFaisPlaceOrFlushing(void)
{
	const void *state = GetTaskFlushingState( VTask::GetCurrent());
	return (state == TaskIsFaisPlaceTask || state == TaskIsFlushTask);
}

void SetCurTaskFlushing(Boolean x)
{
	SetTaskFlushingState( VTask::GetCurrent(), x ? TaskIsFlushTask : nil);
}

void SetCurTaskFaisPlace(Boolean x)
{
	SetTaskFlushingState( VTask::GetCurrent(), x ? TaskIsFaisPlaceTask : nil);
}

#if debug

typedef struct __xtBlockInfo
{
	struct __xtBlockInfo **masterPtr;
	struct __xtBlockInfo *previousBlock;
	sLONG blockSize;
	uBYTE sizeAdjust;
	uBYTE flags;				// 0 si block libre
	sWORD reserved;
	sLONG blockInfo;
} __xtBlockInfo;

typedef struct __xtBlockInfo2
{
	struct __xtBlockInfo **masterPtr;		// pointeur sur prochaine heap dans la sentinelle, NULL for pointer(!=handle)
	struct __xtBlockInfo *previousBlock;	// pointe sur le __xtBlockInfo du bloc precedent
	sLONG blockSize;						// size of data, add (long)sizeof(__xtBlockInfo2) and sizeAdjust for memory size
	uBYTE sizeAdjust;
	uBYTE flags;								// 0 si block libre
	sWORD reserved;
	sLONG blockInfo;
} __xtBlockInfo2;

uBOOL IsValidHandle(void* hh)
{
#if 0
	Handle pHandle = (Handle)hh;
	tPtr tempVarBloc;
	sLONG tempWhere;
	__xtBlockInfo *curBlock;

	if (!pHandle) {
		return true;		//	Handle is nil
	}
	if (!(*pHandle)) {
		DebugMsg (" IsValidHandle FAILED *Handle=NULL");
		return false;		//	Handle is nil
	}
	curBlock = (__xtBlockInfo *)vAddToPtr(*pHandle,-(sLONG)sizeof(__xtBlockInfo2));
	if ((tHandle)curBlock->masterPtr!=pHandle)
	{
		DebugMsg (" IsValidHandle FAILED master!=Handle");
		return false;
	}
#endif
	return(true);
}


uBOOL IsValidPtr(void* p)
{
	return(true);
}

uBOOL IsValidFastPtr(void* p)
{
	if (p == nil) return(true);
	return(gAlternateCppMem->IsAPtr(p));
}

uBOOL IsFieldValid(sLONG nf, sLONG nc, Base4D* bd)
{
	uBOOL res = true;
	
	if (bd != nil)
	{
		Table* fic = bd->RetainTable(nf);
		if (fic == nil) res = false;
		else
		{
			if (nc<=0 || nc>fic->GetNbCrit()) res = false;
			fic->Release();
		}
	}
	return(res);
}


uBOOL IsFileValid(sLONG nf, Base4D* bd)
{
	uBOOL res = true;

	if (bd != nil)
	{
		Table* fic = bd->RetainTable(nf);
		if (fic == nil) res = false;
		else
			fic->Release();
	}
	return(res);
}


uBOOL IsValidDiskAddr(DataAddr4D addr, Base4D* bd)
{
	if (addr ==0) return(true);
	if (bd != nil)
	{
		return(bd->IsAddrValid(addr));
	}
	
	return(true);
}


#endif


#if debugoccupe
class debugInfoOccuppe
{
public:
	sLONG nboccupe;
	uLONG fmillisec;
};

typedef std::map<Obj4D*, debugInfoOccuppe> debugInfoOccupeMap;

VCriticalSection debugInfoOccupeMutex;
debugInfoOccupeMap diom;

typedef Obj4D* obj4dref;

void add_debugInfoOcuppe(const obj4dref obj)
{
	VTaskLock lock(&debugInfoOccupeMutex);
	debugInfoOccupeMap::iterator deja = diom.find(obj);
	if (deja == diom.end())
	{
		debugInfoOccuppe dd;
		dd.fmillisec = VSystem::GetCurrentTime();
		dd.nboccupe = 1;
		diom.insert(std::pair<Obj4D*, debugInfoOccuppe>(obj, dd));
	}
	else
	{
		deja->second.nboccupe++;
	}
}

void remove_debugInfoOcuppe(const obj4dref obj)
{
	VTaskLock lock(&debugInfoOccupeMutex);
	debugInfoOccupeMap::iterator deja = diom.find(obj);
	if (deja == diom.end())
	{
		assert(false);
	}
	else
	{
		deja->second.nboccupe--;
		if (deja->second.nboccupe == 0)
		{
			diom.erase(deja);
		}
	}
}


void forceremove_debugInfoOcuppe(const obj4dref obj)
{
	VTaskLock lock(&debugInfoOccupeMutex);
	debugInfoOccupeMap::iterator deja = diom.find(obj);
	if (deja != diom.end())
	{
		assert(false);
		diom.erase(deja);
	}
}


#endif

#if debugoccupe

Obj4D::~Obj4D()
{
	forceremove_debugInfoOcuppe(this);
}

#endif

uBOOL Obj4D::IsOkToOccupe(sLONG signature) const
{
	uBOOL result = fOccupe.TryToLock();
#if debugoccupe_with_signature
	if (result)
	{
		MarkOccupe(signature);
	}
#endif

	
	return result;
}


uBOOL Obj4D::occupe(Boolean ForceWaitEvenDringFlush, sLONG signature) const
{
	uBOOL result;
#if debuglr
	VTask::Yield();
#endif
	if (fOccupe.TryToLock())
	{
#if debugoccupe
		add_debugInfoOcuppe((Obj4D*)this);
#endif
		result = false;
	}
	else
	{
		if (!ForceWaitEvenDringFlush && IsCurTaskFaisPlaceOrFlushing())
		{
			result = true;
		}
		else
		{
			result = ! fOccupe.Lock();
#if debugoccupe
			if (!result)
			{
				add_debugInfoOcuppe((Obj4D*)this);
			}
#endif
		}
	}

#if debugoccupe_with_signature
	if (!result)
	{
		MarkOccupe(signature);
	}
#endif

	return result;
}


void Obj4D::libere(sLONG signature) const
{ 
#if debuglr
	VTask::Yield();
#endif
#if debugoccupe
	remove_debugInfoOcuppe((Obj4D*)this);
#endif
#if debugoccupe_with_signature
	if (fOccupe.GetUseCount() == 1)
	{
		VTaskLock lock(&sDebugMapOfOccupeMutext);
		sDebugMapOfOccupe.erase(this);
	}
#endif
	fOccupe.Unlock();
}


#if debugoccupe_with_signature

debugMapOfOccupe Obj4D::sDebugMapOfOccupe;
VCriticalSection Obj4D::sDebugMapOfOccupeMutext;

void Obj4D::MarkOccupe(sLONG signature) const
{ 
	if (signature != 0)
	{
		VTaskLock lock(&sDebugMapOfOccupeMutext);
		debugOccupeSignature occupesignature;
		occupesignature.fSignature = signature;
		occupesignature.fTime = VSystem::GetCurrentTime();
		occupesignature.fTaskID = VTask::GetCurrentID();
		sDebugMapOfOccupe[this].push_back(occupesignature);
	}
}

#endif


																				/****************/



																	
void* ObjInCacheMemory::operator new(size_t size)
{
#if debug_nonmemmgr
	return malloc(size);
#else
	void* res=NULL;

	if testAssert(VDBMgr::GetManager()->GetCacheManager() != NULL)
		res = VDBMgr::GetManager()->GetCacheManager()->NewPtr( (VSize)size, true, 'objC');
	return res;
#endif
}


void ObjInCacheMemory::operator delete (void *p )
{
#if debug_nonmemmgr
	free(p);
#else
	assert( VDBMgr::GetManager()->GetCacheManager() != nil);
	VDBMgr::GetManager()->GetCacheManager()->DisposePtr(p);
#endif
}



																				/****************/



uWORD ObjAlmostInCache::FlagMaskAnd[16];
uWORD ObjAlmostInCache::FlagMaskSet[16];
uWORD ObjAlmostInCache::FlagMaskClear[16];

void ObjAlmostInCache::InitMasks()
{
	sLONG i;
	uWORD x = 1;
	uWORD y = 0xFFFE;
	for (i = 0; i< 16; i++)
	{
		FlagMaskSet[i] = x;
		FlagMaskAnd[i] = x;
		x = x << 1;
		FlagMaskClear[i] = y;
		y = (y << 1) | 1;
	}
}


ObjAlmostInCache::ObjAlmostInCache(Boolean occupeOnInit)
{
#if debuglrWithTypObj
	typobj=t_ObjCache;
#endif
	//addrobj=0;
	flags = 0;
	//fPrisPar = 0;
	//fFlushInfo = 0;
	if (occupeOnInit)
		occupe();
}


/*
BaseFlushInfo* ObjAlmostInCache::GetBaseFlushInfo(void) 
{ 
	if (fFlushInfo == 0) 
		return nil; 
	else 
return VDBFlushMgr::GetFlushInfo(fFlushInfo); 
}


VError ObjAlmostInCache::LibereEspaceDisk(VDB4DProgressIndicator* InProgress)
{
	return VE_OK;
}

sLONG ObjAlmostInCache::saveobj(void)
{
	return(0);
}
*/

sLONG ObjAlmostInCache::liberemem(sLONG allocationBlockNumber, sLONG combien, uBOOL tout)
{
	return(0);
}

uBOOL ObjAlmostInCache::okdel(void)
{
	return(!modifie());
}

/*
CompareResult ObjAlmostInCache::CompAddr(ObjAlmostInCache* obj2) const
{
	uLONG numseg1 = ((uLONG)addrobj) & (kMaxSegData-1), 
		numseg2 = ((uLONG)obj2->addrobj) & (kMaxSegData-1);

	if (numseg1 == numseg2)
	{
		if (addrobj > obj2->addrobj)
			return CR_BIGGER;
		else
		{
			if (addrobj == obj2->addrobj)
				return CR_EQUAL;
			else
				return CR_SMALLER;
		}
	}
	else
	{
		if (numseg1 > numseg2)
			return CR_BIGGER;
		else
			return CR_SMALLER;
	}
}


CompareResult ObjAlmostInCache::CompAddr(DataAddr4D xaddr) const
{
	uLONG numseg1 = ((uLONG)addrobj) & (kMaxSegData-1), 
		numseg2 = ((uLONG)xaddr) & (kMaxSegData-1);

	if (numseg1 == numseg2)
	{
		if (addrobj > xaddr)
			return CR_BIGGER;
		else
		{
			if (addrobj == xaddr)
				return CR_EQUAL;
			else
				return CR_SMALLER;
		}
	}
	else
	{
		if (numseg1 > numseg2)
			return CR_BIGGER;
		else
			return CR_SMALLER;
	}
}



uBOOL ObjAlmostInCache::SupAddr(ObjAlmostInCache* obj2) const 
{ 
	uLONG numseg1 = ((uLONG)addrobj) & (kMaxSegData-1), 
				numseg2 = ((uLONG)obj2->addrobj) & (kMaxSegData-1);

	if (numseg1 == numseg2)
		return(addrobj > obj2->addrobj);
	else
		return numseg1 > numseg2;
}


uBOOL ObjAlmostInCache::SupAddr(DataAddr4D xaddr) const 
{ 
	uLONG numseg1 = ((uLONG)addrobj) & (kMaxSegData-1), 
		numseg2 = ((uLONG)xaddr) & (kMaxSegData-1);

	if (numseg1 == numseg2)
		return(addrobj > xaddr);
	else
		return numseg1 > numseg2;
}


void ObjAlmostInCache::ChangeAddr(DataAddr4D addr, Base4D* bd, BaseTaskInfo* context)
{
	if (fFlushInfo == 0)
	{
		BaseFlushInfo* flushinfo = bd->GetDBBaseFlushInfo();
		flushinfo->Retain();
		fFlushInfo = flushinfo->GetID();
	}
	if (modifie())
		VDBMgr::RemoveObjectFromFlush( this);
	SetFlag(isModifFlag);
	addrobj=addr;
	VDBMgr::PutObjectInFlush( this);
}
*/

/*
void ObjAlmostInCache::setmodif(uBOOL xismodif, Base4D* bd, BaseTaskInfo* context)
{ 
	assert(!xismodif || !InTrans()); 

	if (xismodif)
	{
		if (!bd->IsWriteProtected())	// no objects to be written on disk if the db is in read-only (m.c)
		{
			if (fFlushInfo == 0)
			{
				BaseFlushInfo* flushinfo = bd->GetDBBaseFlushInfo();
				flushinfo->Retain();
				fFlushInfo = flushinfo->GetID();
			}
			if (!modifie())
			{
	#if 0 && VERSIONDEBUG
				assert(!VDBMgr::GetFlushManager()->FindObjInFlush(this));
	#endif
				SetFlag(isModifFlag);
				//assert(fPrisPar == 0 || fPrisPar == DefaultFlushTransac);
				VDBMgr::PutObjectInFlush( this, true, false);
				
			}
		}
	}
	else
	{
		if (fFlushInfo != 0)
		{
			//assert(addrobj != 0);
			assert(modifie());
			if (bd != nil)
				VDBMgr::RemoveObjectFromFlush( this);
			if (fFlushInfo != 0)
			{
				BaseFlushInfo* flushinfo = VDBFlushMgr::GetFlushInfo(fFlushInfo);
				assert(flushinfo != nil);
				flushinfo->Release();
				fFlushInfo = 0;
			}
		}
		ClearFlag(isModifFlag);
	}
}
*/

/*
VError ObjAlmostInCache::SaveTrans(void)
{
	VError err = VE_OK;

	return(err);
}
*/

/*
uBOOL ObjAlmostInCache::AccesModifOK(BaseTaskInfo* context)
{
	TransactionIndex trans = GetCurTransID(context);
	if (trans == 0)
	{
		trans = DefaultFlushTransac;
	}
	
	return(trans == fPrisPar || fPrisPar == 0);
}
*/

ObjAlmostInCache::~ObjAlmostInCache()
{
	if (!IsFlag(PourDeleteFlushFlag))
	{
		if (fModified)
		{
			fModified = false;
			VDBMgr::RemoveObjectFromFlush( this);
		}
	}
#if 0 && VERSIONDEBUG
	assert(!VDBMgr::GetFlushManager()->FindObjInFlush(this));
#endif
}

									/* ------------------------- */
									
						
ObjCache::ObjCache(sWORD DefaultAccess, Boolean occupeOnInit):ObjAlmostInCache(occupeOnInit)
{
	nbacces=DefaultAccess;
	if (nbacces == DoNotPutInCacheAcces)
		SetFlag(DoNotKeepInCacheFlag);
	else
		VDBMgr::PutObjectInCache(this);
}

void ObjCache::AddToCache(sWORD Access)
{
	nbacces=Access;
	if (IsFlag(DoNotKeepInCacheFlag))
	{
		ClearFlag(DoNotKeepInCacheFlag);
		VDBMgr::PutObjectInCache(this);
	}
}

ObjCache::~ObjCache()
{
	if (!IsFlag(PourDeleteCacheFlag) && !IsFlag(DoNotKeepInCacheFlag))
	{
		assert(!IsCurTaskFaisPlace());
		VDBMgr::RemoveObjectFromCache( this);
	}
}


uBOOL ObjCache::okdel(void)
{
	return(!modifie() || ForceDeleted());
}


void ObjCache::IncNbAcces()
{ 
	if (nbacces < 0x7ffe)
	{
		if ((nbacces & 31)==31)
		{
			PourDeleteCache(true);
			VDBMgr::PutObjectInCache(this);
			nbacces++;
			PourDeleteCache(false);
			VDBMgr::PutObjectInCache(this);
		}
		else
		{
			nbacces++;
		}
	}
}



// -----------------------------------------------------------------------------------


void* ObjInCacheMem::operator new(size_t size)
{
#if debug_nonmemmgr
	return malloc(size);
#else
	void* res=NULL;

	if testAssert(VDBMgr::GetManager()->GetCacheManager() != NULL)
		res = VDBMgr::GetManager()->GetCacheManager()->NewPtr( (VSize)size, true, 'objC');
	return res;
#endif
}


void ObjInCacheMem::operator delete (void *p )
{
#if debug_nonmemmgr
	free(p);
#else
	assert( VDBMgr::GetManager()->GetCacheManager() != nil);
	VDBMgr::GetManager()->GetCacheManager()->DisposePtr(p);
#endif
}


								/* ------------------------- */




void IObjToFree::RemoveFromCache()
{
	VDBMgr::RemoveObjectFromCache(this);
}


void IObjToFree::PutInCache()
{
	if (this != nil) // oui, oui, je sais, mais c'est bien pratique !! (LR)
		VDBMgr::PutObjectInCache(this, true);
}

bool IObjToFree::CheckForMemRequest(VSize minBlockSize)
{
	bool enoughMem = true;
#if 0
	VDBCacheMgr* cache =  VDBMgr::GetCacheManager();
	if (fNeededMem != 0 && fFreeMemRequestCount > 20)
	{
		cache->CallFreeMemObj(fNeededAllocationBlockNumber, fNeededMem, this);
	}

	VSize usedmem, totmem;
	cache->GetMemUsageInfo(totmem, usedmem);
	Real xtotmem = totmem, xusedmem = usedmem;
	if ( xusedmem / xtotmem >= 0.8 )
	{
		cache->NeedsBytes(	(VSize)(0.1*xtotmem));
	}

	if (minBlockSize != 0)
	{
		void* p = GetFastMem(minBlockSize, false, -2);
		if (p != nil)
			FreeFastMem(p);
		else
			enoughMem = false;
	}
#endif
	return enoughMem;
}


								/* ------------------------- */


void IObjToFlush::RemoveFromFlush()
{
	VDBMgr::RemoveObjectFromFlush( this);
}


BaseFlushInfo* IObjToFlush::GetBaseFlushInfo(void) 
{ 
	if (fFlushInfo == 0) 
		return nil; 
	else 
		return VDBFlushMgr::GetFlushInfo(fFlushInfo); 
}


VError IObjToFlush::LibereEspaceDisk(VDB4DProgressIndicator* InProgress)
{
	return VE_OK;
}


CompareResult IObjToFlush::CompAddr(IObjToFlush* obj2) const
{
	uLONG numseg1 = ((uLONG)addrobj) & (kMaxSegData-1), 
		numseg2 = ((uLONG)obj2->addrobj) & (kMaxSegData-1);

	if (numseg1 == numseg2)
	{
		if (addrobj > obj2->addrobj)
			return CR_BIGGER;
		else
		{
			if (addrobj == obj2->addrobj)
				return CR_EQUAL;
			else
				return CR_SMALLER;
		}
	}
	else
	{
		if (numseg1 > numseg2)
			return CR_BIGGER;
		else
			return CR_SMALLER;
	}
}


CompareResult IObjToFlush::CompAddr(DataAddr4D xaddr) const
{
	uLONG numseg1 = ((uLONG)addrobj) & (kMaxSegData-1), 
		numseg2 = ((uLONG)xaddr) & (kMaxSegData-1);

	if (numseg1 == numseg2)
	{
		if (addrobj > xaddr)
			return CR_BIGGER;
		else
		{
			if (addrobj == xaddr)
				return CR_EQUAL;
			else
				return CR_SMALLER;
		}
	}
	else
	{
		if (numseg1 > numseg2)
			return CR_BIGGER;
		else
			return CR_SMALLER;
	}
}



uBOOL IObjToFlush::SupAddr(IObjToFlush* obj2) const 
{ 
	uLONG numseg1 = ((uLONG)addrobj) & (kMaxSegData-1), 
		numseg2 = ((uLONG)obj2->addrobj) & (kMaxSegData-1);

	if (numseg1 == numseg2)
		return(addrobj > obj2->addrobj);
	else
		return numseg1 > numseg2;
}


uBOOL IObjToFlush::SupAddr(DataAddr4D xaddr) const 
{ 
	uLONG numseg1 = ((uLONG)addrobj) & (kMaxSegData-1), 
		numseg2 = ((uLONG)xaddr) & (kMaxSegData-1);

	if (numseg1 == numseg2)
		return(addrobj > xaddr);
	else
		return numseg1 > numseg2;
}


void IObjToFlush::ChangeAddr(DataAddr4D addr, Base4D* bd, BaseTaskInfo* context)
{
	fChangingAddr = true;
	if (fFlushInfo == 0)
	{
		BaseFlushInfo* flushinfo = bd->GetDBBaseFlushInfo();
		flushinfo->Retain();
		fFlushInfo = flushinfo->GetID();
	}
	if (fModified)
	{
		fModified = false;
		VDBMgr::RemoveObjectFromFlush( this);
	}
	fModified = true;
	addrobj=addr;
	if (!IsAddrAllowed())
	{
		assert(IsAddrAllowed()); // break here
		ThrowBaseError(VE_DB4D_DATABASEISWRITEPROTECTED, noaction);
	}
	VDBMgr::PutObjectInFlush( this);
	fChangingAddr = false;
}


void IObjToFlush::FailedSaving(BaseFlushInfoIndex xflushinfo)
{
	fModified = true;
	fFlushInfo = xflushinfo;
	if (xflushinfo != 0)
	{
		BaseFlushInfo* flushinfo = VDBFlushMgr::GetFlushInfo(fFlushInfo);
		flushinfo->Retain();
	}
	fSaving = false;
}


void IObjToFlush::setmodif(uBOOL xismodif, Base4D* bd, BaseTaskInfo* context)
{ 
	if (xismodif)
	{
		if (!IsAddrAllowed())
		{
			assert(IsAddrAllowed()); // break here
			ThrowBaseError(VE_DB4D_DATABASEISWRITEPROTECTED, noaction);
		}
		if (!bd->IsWriteProtected())	// no objects to be written on disk if the db is in read-only (m.c)
		{
			if (fFlushInfo == 0)
			{
				BaseFlushInfo* flushinfo = bd->GetDBBaseFlushInfo();
				flushinfo->Retain();
				fFlushInfo = flushinfo->GetID();
			}
			if (!fModified)
			{
#if 0 && VERSIONDEBUG
				assert(!VDBMgr::GetFlushManager()->FindObjInFlush(this));
#endif
				fModified = true;
				VDBMgr::PutObjectInFlush( this, true, false);
			}
		}
	}
	else
	{
		if (fFlushInfo != 0)
		{
#if 0 && debuglr
			if (!fChangingAddr && !fModified)
				assert(fModified);
#endif
			fModified = false;
			if (bd != nil)
				VDBMgr::RemoveObjectFromFlush( this);
			if (fFlushInfo != 0)
			{
				BaseFlushInfo* flushinfo = VDBFlushMgr::GetFlushInfo(fFlushInfo);
				assert(flushinfo != nil);
				flushinfo->Release();
				fFlushInfo = 0;
			}
		}
		fModified = false;
	}
}


							// -------------------------------------------------------



ObjCacheArrayLongFix::ObjCacheArrayLongFix(Base4D *xdb, sLONG xhowmany)
{
	db=xdb;
	nbelem=xhowmany;
	fbb=(sLONG*)GetFastMem((xhowmany+1)<<2, false, 'fbb ');
	if (fbb!=nil) fbb[xhowmany]=0;
}


ObjCacheArrayLongFix::~ObjCacheArrayLongFix()
{
	if (fbb!=nil) 
		FreeFastMem(fbb);
	fbb=nil;
}

sLONG ObjCacheArrayLongFix::getsize()
{
	return (sLONG) GetFastMemSize(fbb);
}



bool ObjCacheArrayLongFix::SaveObj(VSize& outSizeSaved)
{
#if debuglogwrite
	VString wherefrom(L"ObjCacheArrayLongFix SaveObj");
	VString* whx = &wherefrom;
#else
	VString* whx = nil;
#endif

	if (fbb!=nil)
	{
		DataBaseObjectHeader tag(fbb,nbelem<<2, DBOH_Bittab, -1, -1);
		tag.WriteInto(db, getaddr(), whx);
		db->writelong(fbb,nbelem<<2,getaddr(),kSizeDataBaseObjectHeader, whx);
	}
	outSizeSaved = (nbelem<<2);

	return true;
}


									/* ------------------------- */


#if autocheckobj
uBOOL ObjCacheInTree::CheckObjInMem(void)
{
	if (parent != nil)
	{
		CheckAssert(posdansparent >= 0);
		if (parent != nil)
		{
			CheckAssert(parent->GetParent() == nil || !parent->okdel());
		}
	}
	return(true);
}
#endif


void ObjCacheInTree::SetParent(ObjCacheInTree *par, sLONG pos) 
{ 
	parent=par; 
	posdansparent=pos; 
	if (par != nil)
	{
		par->InvalidateNbElemTab();
		//par->SetChild(this, pos);
	}

#if debugObjInTree_Strong
	//Debug_CheckParentEnfantsRelations();
#endif
}


sLONG ObjCacheInTree::GetNbElemTab(void) 
{ 
	if (nbelemtab == -1)
		RecalcNbElemTab();
	return(nbelemtab); 
}


sLONG ObjCacheInTree::FindPosOfChild(ObjCacheInTree* obj)
{
	return -1;
}



#if debugObjInTree_Strong

debug_setof_ObjCacheInTree ObjCacheInTree::debug_ObjCacheInTrees;
VCriticalSection ObjCacheInTree::debug_Mutex;
sLONG ObjCacheInTree::sCheckDisabled = 0;


void ObjCacheInTree::RegisterForDebug(ObjCacheInTree* obj)
{
	VTaskLock lock(&debug_Mutex);

	debug_ObjCacheInTrees.insert(obj);
}


void ObjCacheInTree::UnRegisterForDebug(ObjCacheInTree* obj)
{
	VTaskLock lock(&debug_Mutex);

	debug_ObjCacheInTrees.erase(obj);
}


void ObjCacheInTree::Debug_CheckForDelete(ObjCacheInTree* obj)
{
	VTaskLock lock(&debug_Mutex);

	for (debug_setof_ObjCacheInTree::const_iterator cur = debug_ObjCacheInTrees.begin(), end = debug_ObjCacheInTrees.end(); cur != end; cur++)
	{
		const ObjCacheInTree* curobj = *cur;

		if (curobj->parent == obj)
		{
			assert(false);
			obj = obj; // put a break here
		}
	}
}


void ObjCacheInTree::Debug_CheckParentEnfantsRelations()
{
	VTaskLock lock(&debug_Mutex);
	debug_setof_ObjCacheInTree uniques;
	for (debug_setof_ObjCacheInTree::const_iterator cur = debug_ObjCacheInTrees.begin(), end = debug_ObjCacheInTrees.end(); cur != end; cur++)
	{
		const ObjCacheInTree* curobj = *cur;

		if (curobj->IsOkToOccupe())
		{
			if (curobj->parent != nil)
			{
				if (curobj->parent->OnlyHasOneChild())
				{
					if (uniques.find(curobj->parent) != uniques.end())
					{
						assert(false);
						curobj = curobj; // put a break;
					}
					uniques.insert(curobj->parent);
				}

				if (curobj->parent->IsOkToOccupe())
				{
					curobj->parent->CheckDansParent(curobj->posdansparent,(ObjCacheInTree*)curobj);
					curobj->parent->libere();
				}
			}
			curobj->libere();
		}
	}
}

#endif






									/* ------------------------- */

#if debugoccupe

Boolean DebugOccupeTask::DoRun()
{
	Boolean cont = true;

	if( GetState() < TS_DYING) 
	{
		VTaskLock lock(&debugInfoOccupeMutex);

		debugInfoOccupeMap::iterator cur = diom.begin(), end = diom.end();

		uLONG curtime = VSystem::GetCurrentTime();
		for (;cur != end; cur++)
		{
			assert((curtime - cur->second.fmillisec) < timetocheck);
		}

		if (GetState() < TS_DYING)
			Sleep(checkinterval);
	}
	else cont = false;

	return cont;
}

#endif

								/* ------------------------- */



void _raz(void *p, sLONG len)
{
	sLONG *p2;
	sWORD *p3;
	
	if ((len & 3)==0)
	{
		p2=(sLONG*)p;
		len=len>>2; // on divise par 4
		while (len>0)
		{
			*p2++=0;
			len--;
		}
	}
	else
	{
		p3=(sWORD*)p;
		len=len>>1; // on divise par 2
		while (len>0)
		{
			*p3++=0;
			len--;
		}
	}
}

void _rau(void *p, sLONG len)
{
	sLONG *p2;
	sWORD *p3;
	
	if ((len & 3)==0)
	{
		p2=(sLONG*)p;
		len=len>>2; // on divise par 4
		while (len>0)
		{
			*p2++=0xFFFFFFFF;
			len--;
		}
	}
	else
	{
		p3=(sWORD*)p;
		len=len>>1; // on divise par 2
		while (len>0)
		{
			*p3++=(sWORD)0xFFFF;
			len--;
		}
	}
}

void copyunichar(UniChar *p, UniChar *p2)
{
	sLONG len;
	
	len=(*p)+2;
	while (len>0)
	{
		*p2++=*p++;
		len--;
	}
}

uBOOL allzero(void *p, sLONG len)
{
	uBOOL toutzero;
	sLONG *p2;
	
	toutzero=true;
	p2=(sLONG*)p;
	len=len>>2; // on divise par 4
	while (len>0)
	{
		if ((*p2++)!=0)
		{	
			toutzero=false;
			len=0;
		}
		else
		{
			len--;
		}
	}
	
	return(toutzero);
}


sLONG comptelong(void *p,sLONG nb)
{
	sLONG *p2;
	sLONG tot;
	
	tot=0;
	p2=(sLONG*)p;
	while (nb>0)
	{
		if ((*p2++)!=0)
		{	
			tot++;
		}
		nb--;
	}
	return(tot);
}


sLONG compteptr(const void *p,sLONG nb)
{
	sLONG tot = 0;
	const VPtr *p2 =(const VPtr *) p;
	while (nb>0)
	{
		if ((*p2++)!=nil)
		{	
			tot++;
		}
		nb--;
	}
	return(tot);
}


#if 0
sLONG BestTempVol(void)
{
	return(kAppliDir);
}
#endif


void moveblock(void *p1, void *p2, sLONG len)
{
	uCHAR *xp1,*xp2;
	
	xp1=(uCHAR*)p1;
	xp2=(uCHAR*)p2;
	
	while (len>0)
	{
		(*xp2++)=(*xp1++);
		len--;
	}
}

void move2block(void *p1, void *p2, sLONG len)
{
	uWORD *xp1,*xp2;
	
	xp1=(uWORD*)p1;
	xp2=(uWORD*)p2;
	len=len>>1;
	
	while (len>0)
	{
		(*xp2++)=(*xp1++);
		len--;
	}
}

void move4block(void *p1, void *p2, sLONG len)
{
	uLONG *xp1,*xp2;
	
	xp1=(uLONG*)p1;
	xp2=(uLONG*)p2;
	len=len>>2;
	
	while (len>0)
	{
		(*xp2++)=(*xp1++);
		len--;
	}
}

void move2insert(void *p1, void *p2, sLONG len)
{
	uWORD *xp1,*xp2;
	
	len=len>>1;
	xp1=((uWORD*)p1)+len;
	xp2=((uWORD*)p2)+len;
	
	while (len>0)
	{
		*(--xp2)=(*(--xp1));
		len--;
	}
}


uBOOL AllFF(void *p, sLONG len)
{
	uBOOL result;
	sLONG *p2;
	
	p2=(sLONG*)p;
	result=true;
	
	len=len>>2;
	while (len>0)
	{
		if (*p2++!=-1)
		{
			result=false;
			break;
		}
		len--;
	}
	
	return(result);
}


uBOOL All00(void *p, sLONG len)
{
	uBOOL result;
	sLONG *p2;
	
	p2=(sLONG*)p;
	result=true;
	
	len=len>>2;
	while (len>0)
	{
		if (*p2++!=0)
		{
			result=false;
			break;
		}
		len--;
	}
	
	return(result);
}


void* ResizePtr(void* p, sLONG oldlen, sLONG newlen)
{
	/*
	void* xx;
	
	xx = GetFastMem(newlen);
	if (xx != nil)
	{
		vBlockMove(p,xx,oldlen);
		FreeFastMem(p);
		return xx;
	}
	else
	{
		vThrowError(memfull);
		return p;
	}
	*/
	return SetFastMemSize(p, newlen);
}



																			/* -------------------------------- */


/*

sLONG ArrayPtrAvecTrou::FindNextFree(uBOOL from1)
{
	genericptr *p;
	sLONG i,n,nb;
	
	n=-1;
	nb=oGetNbElem();
	if (nb>0)
	{
		oLockArray();
		p=(genericptr*) oGetPtrOnFirstElem();
		for (i=0;(i<nb) && (n==-1);i++)
		{
			if (*p==nil) n=i;
			p++;
		}
		oUnlockArray();
	}
	
	if (n==-1)
	{
		if (oAddSpace()!=-1)
		{
			n=oGetNbElem()-1;
		}
	}
	
	if (from1) n++;
	return(n);
}


sLONG ArrayLongAvecTrou::FindNextFree(uBOOL from1)
{
	sLONG *p;
	sLONG i,n,nb;
	
	n=-1;
	nb=oGetNbElem();
	if (nb>0)
	{
		oLockArray();
		p=(sLONG*) oGetPtrOnFirstElem();
		for (i=0;(i<nb) && (n==-1);i++)
		{
			if (*p==0) n=i;
			p++;
		}
		oUnlockArray();
	}
	
	if (n==-1)
	{
		if (oAddSpace()!=-1)
		{
			n=oGetNbElem()-1;
		}
	}
	
	if (from1) n++;
	return(n);
}

*/


											/* ---------------------------------------------------------------- */
											

#if 0

ObjectInList::ObjectInList(ObjectInListHeader *rootHeader)
{ 
	if (rootHeader->root == nil)
	{
		rootHeader->root = this;
		next = nil; 
		prev = nil;
	}
	else
	{
		rootHeader->root->prev = this;
		next = rootHeader->root;
		prev = nil;
	}
	
	header = rootHeader;
}


ObjectInList::~ObjectInList()
{ 
	if (header->root == this)
	{
		if (next != nil) next->prev = nil;
		header->root = next;
	}
	else
	{
		if (prev != nil)
		{
			prev->next = next;
		}
		
		if (next != nil)
		{
			next->prev = prev;
		}
	}
	
}


ObjectInList* ObjectInList::xFindObject(ObjectInList* root, VString& objtofind)
{
	while (root != nil)
	{
		if (root->name == objtofind)
		{
			return(root);
		}
		
		root = root->next;
	}
	
	return(nil);
}





ObjectInListHeader::~ObjectInListHeader()
{
	while (root != nil)
	{
		delete root;
		// il ne manque pas le next car le delete change le root de la liste (L.R)
	}
}

#endif


											/* ---------------------------------------------------------------- */


#if 0
void UniToAscii(void* source, void* target, sLONG nbchar)
{
	UniPtr p1;
	uCHARPTR p2,p2x;
	
	p1=(UniPtr)source;
	p2=(uCHARPTR)target;
	p2x=p2+((nbchar<<1)+4);
	vUni2Mac(p1,p2,p2x,nbchar); // EV je suppose que c'est l'effet recherche
}

void AsciiToUni(void* source, void* target, sLONG nbchar)
{
	uCHARPTR p1;
	UniPtr p2,p2x;
	
	p1=(uCHARPTR)source;
	p2=(UniPtr)target;
	p2x=p2+((nbchar<<1)+4);
	vMac2Uni(p1,p2,p2x,nbchar); // EV je suppose que c'est l'effet recherche
}
#endif


uBOOL EgalEndString(const VString& s1, const VString& s2)
{
	uBOOL egal;
	sLONG len1,len2,i;
	
	egal=false;
	len1=s1.GetLength();
	len2=s2.GetLength();
	if (len2<=len1)
	{
		egal=true;
		for (i=0;(i<len2) && egal;i++)
		{
			if ((s1.GetUniChar(len1-i-1))!=(s2.GetUniChar(len2-i-1))) egal=false;
		}
	}
	
	return(egal);
}

void TruncateExtension(VString& s1, const VString& s2)
{
	if (EgalEndString(s1,s2))
	{
		s1.Truncate(s1.GetLength()-s2.GetLength());
	}
}


sLONG Pos(const VString& s, UniChar c)
{
	sLONG i,p,len;
	const UniChar *pu;
	
	p=-1;
	len=s.GetLength();
	pu=s.GetCPointer();
	for (i=0;(i<len) && (p==-1);i++,pu++)
	{
		if (*pu==c)
		{
			p=i;
			break;
		}
	}
	
	return(p);
}


void StripExtension(VString& s)
{
	sLONG p;
	
	p=Pos(s,kExtSep);
	if (p!=-1)
	{
		s.Truncate(p);
	}
	
}



#if journalcache

uCHAR FiltreJournal[128];

void InitJournal(void)
{
	_raz(FiltreJournal, sizeof(FiltreJournal) );  // tous inactives
	// FiltreJournal[Obj_Fiche]=0;
}

uBOOL OKJournal(sLONG classID)
{
	return( FiltreJournal[classID]!=0 );
}


void Jmess(uCHAR* mess, VName& name, sLONG classID)
{
	VStr255 s;
	
	if (OKJournal(classID))
	{
		s.FromCString(mess);
		s.Append(name);
		DebugMsg(s);
	}
}


void Jmess(uCHAR* mess, ObjCache* obj)
{
	VName sname;
	sLONG classID;
	VStr255 s;
	uLONG nn;
	
	classID=obj->GetObjName(sname);
	if (OKJournal(classID))
	{
		s.FromCString(mess);
		s.Append(sname);
		sname.FromCString((uCHAR*)" , mem = ");
		s.Append(sname);
		nn=(uLONG)obj;
		sname.FromULong(nn,kHexaDecimal);
		s.Append(sname);
		sname.FromCString((uCHAR*)" , adr = ");
		s.Append(sname);
		sname.FromULong(obj->getaddr(),kHexaDecimal);
		s.Append(sname);
		DebugMsg(s);
	}
	
}

#endif

#if 0
void vHtoHandle(void* hh, void* hh2)
{
	sLONG len;
	// hh est un tHandle de VMemMgr
	// hh2 est un Handle Mac (ou Altura)
	
	len = vGetHandleSize((tHandle)hh);
	*(Handle *)hh2 = nouvhandle(len);
	if (*(Handle *)hh2 != nil)
	{
		vBlockMove(*((void**)hh), **((void***)hh2), len);
	}
	
}
#endif



						/* ---------------------------------------------------------------- */


sLONG8 SwapLong8(sLONG8 x)
{
	sLONG8 result;
	uCHAR *p = (uCHAR*)&x, *p2 = ((uCHAR*)&result)+7;

	*p2++ = *p--;
	*p2++ = *p--;
	*p2++ = *p--;
	*p2++ = *p--;
	*p2++ = *p--;
	*p2++ = *p--;
	*p2++ = *p--;
	*p2 = *p;

	return result;
}


uLONG8 SwapLong8(uLONG8 x)
{
	uLONG8 result;
	uCHAR *p = (uCHAR*)&x, *p2 = ((uCHAR*)&result)+7;

	*p2++ = *p--;
	*p2++ = *p--;
	*p2++ = *p--;
	*p2++ = *p--;
	*p2++ = *p--;
	*p2++ = *p--;
	*p2++ = *p--;
	*p2 = *p;

	return result;
}



						/* ---------------------------------------------------------------- */



VError ThrowJSExceptionAsError(VJSContext& context, JS4D::ExceptionRef excep)
{
	VJSValue val(context, excep);
	VJSObject obj(val.GetObject());
	VJSValue message(obj.GetProperty("message"));
	VJSValue line(obj.GetProperty("line"));

	sLONG linenumber = 0;
	line.GetLong(&linenumber);
	VString errmess;
	message.GetString(errmess);

	VErrorDB4DBase *err = new VErrorDB4DBase(VE_DB4D_JSTEXT_ERR, noaction);
	err->GetBag()->SetString(Db4DError::jstext, errmess);
	err->GetBag()->SetLong(Db4DError::jsline, linenumber);
	VTask::GetCurrent()->PushRetainedError( err);
	/*
	VString outEx;
	VJSJSON json(context);
	json.Stringify(val, outEx);
	*/

	return VE_DB4D_JSTEXT_ERR;
}

VError ThrowBaseError(VError inErrCode, ActionDB4D inAction)
{
	VErrorDB4DBase *err = new VErrorDB4DBase(inErrCode, inAction);
	VTask::GetCurrent()->PushRetainedError( err);
	return inErrCode;
}


VError ThrowBaseError(VError inErrCode, const VString& param1)
{
	VErrorDB4DBase *err = new VErrorDB4DBase(inErrCode, noaction);
	err->GetBag()->SetString(Db4DError::Param1, param1);
	VTask::GetCurrent()->PushRetainedError( err);
	return inErrCode;
}


VError ThrowBaseError(VError inErrCode, const VString& param1, const VString& param2)
{
	VErrorDB4DBase *err = new VErrorDB4DBase(inErrCode, noaction);
	err->GetBag()->SetString(Db4DError::Param1, param1);
	err->GetBag()->SetString(Db4DError::Param2, param2);
	VTask::GetCurrent()->PushRetainedError( err);
	return inErrCode;
}




void VErrorDB4DBase::DumpToString( VString& outString) const
{
		VError err = GetError();
		if (err == VE_DB4D_JSTEXT_ERR)
		{
			GetBag()->GetString(Db4DError::jstext, outString);
		}
		else
			GetErrorDescription(outString);
#if 0
		sLONG compnum = (sLONG)(err>>32);
		//if (compnum == (sLONG)'dbmg')
		{
			err = err & (uLONG8)0XFFFFFFFF;
			sLONG n = (sLONG)err;


			VString s,s2,s3(L" , ");
			/*
			VDBMgr::GetManager()->GetErrorString(n-1000,s);
			VDBMgr::GetManager()->GetErrorActionString((sLONG)fAction,s2);
			*/
			
#if SMALLENDIAN
			ByteSwapLong(&compnum);
#endif
			s.FromBlock(&compnum,4,CS_ANSI);
			s2.FromLong(n);

			outString.Clear();
			outString.AppendString(s);
			outString.AppendString(s3);
			outString.AppendString(s2);
		}
#endif
}


#if 0

void VErrorDB4DBase::GetErrorDescription( VString& outError) const
{
	sLONG errCode = ERRCODE_FROM_VERROR(fError);
	OsType component = COMPONENT_FROM_VERROR(fError);

	if (CDB4DManager::Component_Type == component)
	{
		VDBMgr* manager = VDBMgr::GetManager();
		VLocalizationManager* local = manager->GetDefaultLocalization();
		if (local != nil)
		{
			STRSharpCodes sc(1000,errCode-1000);
			local->LocalizeStringWithSTRSharpCodes(sc, outError);
		}
	}
	else
	{
		if (fNativeError != VNE_OK)
		{
			VString errorString;
			GetNativeErrorString( fNativeError, outError);
		}
		else
		{
			outError.Clear();
		}
	}
}


void VErrorDB4DBase::GetActionDescription( VString& outAction) const
{
	outAction.Clear();
}

#endif


void VErrorDB4DBase::GetLocation( VString& outLocation) const
{
	outLocation = L"Database Engine";
}

void gs(sLONG inID, uLONG inStringID, XBOX::VString &sout)
{
	VDBMgr* manager = VDBMgr::GetManager();
	VLocalizationManager* local = manager->GetDefaultLocalization();

	sout.Clear();

	if (local != nil)
	{
		STRSharpCodes sc(inID,inStringID);
		local->LocalizeStringWithSTRSharpCodes(sc,sout);
	}
}


VString GetString(sLONG inID, uLONG inStringID)
{
	VDBMgr* manager = VDBMgr::GetManager();
	VLocalizationManager* local = manager->GetDefaultLocalization();
	VString result;
	if (local != nil)
	{
		STRSharpCodes sc(inID,inStringID);
		local->LocalizeStringWithSTRSharpCodes(sc,result);
	}
	return result;
}


VErrorDB4D_OnBase::VErrorDB4D_OnBase( VError inErrCode, ActionDB4D inAction, const Base4D *inBase)
											:VErrorDB4DBase(inErrCode, inAction)
{
	if (inBase != nil)
		inBase->GetName(fBaseName);
	GetBag()->SetString(Db4DError::BaseName, fBaseName);
}

void VErrorDB4D_OnBase::DumpToString( VString& outString) const
{
	VErrorDB4DBase::DumpToString(outString);

	/*
	VString s3(L" , ");
	VString s(L"DataBase = ");

	outString.AppendString(s3);
	outString.AppendString(s);
	outString.AppendString(fBaseName);
	*/
}



VErrorDB4D_OnCheckAndRepair::VErrorDB4D_OnCheckAndRepair( VError inErrCode, ActionDB4D inAction, const Base4D *inBase)
											:VErrorDB4D_OnBase(inErrCode, inAction, inBase)
{
	
}


void VErrorDB4D_OnCheckAndRepair::DumpToString( VString& outString) const
{
	VErrorDB4D_OnBase::DumpToString(outString);
}



VErrorDB4D_OnRelation::VErrorDB4D_OnRelation( VError inErrCode, ActionDB4D inAction, const Base4D *inBase)
											:VErrorDB4D_OnBase(inErrCode, inAction, inBase)
{
	
}


void VErrorDB4D_OnRelation::DumpToString( VString& outString) const
{
	VErrorDB4D_OnBase::DumpToString(outString);
}



VErrorDB4D_OnTable::VErrorDB4D_OnTable( VError inErrCode, ActionDB4D inAction, const Base4D *inBase, sLONG inTableID)
											:VErrorDB4D_OnBase(inErrCode, inAction, inBase)
{
	fTableID = inTableID;
	Table* tt = RetainOwner(inBase);
	if (tt != nil)
	{
		tt->GetName(fTableName);
		GetBag()->SetString(Db4DError::TableName, fTableName);
		tt->Release();
	}
}


Table* VErrorDB4D_OnTable::RetainOwner( const Base4D *inBase) const
{
	return (inBase != nil)  ? inBase->RetainTable(fTableID) : nil;
}



void VErrorDB4D_OnTable::DumpToString( VString& outString) const
{
	VErrorDB4D_OnBase::DumpToString(outString);
	/*
	VString s3(L" , ");
	VString s(L"Table = ");
	outString.AppendString(s3);
	outString.AppendString(s);
	outString.AppendString(fTableName);
	*/
}




VErrorDB4D_OnField::VErrorDB4D_OnField( VError inErrCode, ActionDB4D inAction, const Base4D *inBase, sLONG inTableID, sLONG inFieldID)
											:VErrorDB4D_OnTable(inErrCode, inAction, inBase, inTableID)
{
	fFieldID = inFieldID;

	Field* ff = RetainOwner(inBase);
	if (ff != nil)
	{
		ff->GetName(fFieldName);
		GetBag()->SetString(Db4DError::FieldName, fFieldName);
		ff->Release();
	}
}


Field* VErrorDB4D_OnField::RetainOwner( const Base4D *inBase) const
{
	Field* res; 
	Table* t = VErrorDB4D_OnTable::RetainOwner(inBase);
	if (t != nil)
	{
		res = t->RetainField(fFieldID);
		t->Release();
	}
	return res;
}


void VErrorDB4D_OnField::DumpToString( VString& outString) const
{
	VErrorDB4D_OnTable::DumpToString(outString);
	/*
	VString s3(L" , ");
	VString s(L"Field = ");
	outString.AppendString(s3);
	outString.AppendString(s);
	outString.AppendString(fFieldName);
	*/
}




VErrorDB4D_OnBlob::VErrorDB4D_OnBlob( VError inErrCode, ActionDB4D inAction, const Base4D *inBase, sLONG inTableID, sLONG inBlobNum, ValueKind inBlobType)
											:VErrorDB4D_OnTable(inErrCode, inAction, inBase, inTableID)
{
	fBlobNum = inBlobNum;
	fBlobType = inBlobType;

	VStr<32> s;
	s.FromLong(fBlobNum);
	GetBag()->SetString(Db4DError::BlobNum, s);

}


void VErrorDB4D_OnBlob::DumpToString( VString& outString) const
{
	VErrorDB4D_OnTable::DumpToString(outString);
}




VErrorDB4D_OnRecord::VErrorDB4D_OnRecord( VError inErrCode, ActionDB4D inAction, const Base4D *inBase, sLONG inTableID, sLONG inRecNum)
											:VErrorDB4D_OnTable(inErrCode, inAction, inBase, inTableID)
{
	fRecNum = inRecNum;
	VStr<32> s;
	s.FromLong(fRecNum);
	GetBag()->SetString(Db4DError::RecNum, s);
}


void VErrorDB4D_OnRecord::DumpToString( VString& outString) const
{
	VErrorDB4D_OnTable::DumpToString(outString);
		//## a faire
}



VErrorDB4D_OnIndex::~VErrorDB4D_OnIndex()
{
}


VErrorDB4D_OnIndex::VErrorDB4D_OnIndex( VError inErrCode, ActionDB4D inAction, const Base4D *inBase, IndexInfo* ind)
											:VErrorDB4D_OnBase(inErrCode, inAction, inBase)
{
	
	if (ind != nil)
	{
		VString s;
		ind->GetDebugString(s, 0);
		GetBag()->SetString(Db4DError::IndexName, s);
	}
}


void VErrorDB4D_OnIndex::DumpToString( VString& outString) const
{
	VErrorDB4D_OnBase::DumpToString(outString);
	//## a faire
}




// ------------------



VErrorDB4D_OnEM::VErrorDB4D_OnEM( VError inErrCode, ActionDB4D inAction, const Base4D *inBase, const EntityModel* em)
:VErrorDB4D_OnBase(inErrCode, inAction, inBase)
{
	fModel = em;
	if (em != nil)
	{
		GetBag()->SetString(Db4DError::EntityModelName, em->GetName());
	}
}


void VErrorDB4D_OnEM::DumpToString( VString& outString) const
{
	VErrorDB4D_OnBase::DumpToString(outString);
	/*
	outString += L"  ,  Entity Model = ";
	if (fModel != nil)
		outString += fModel->GetName();
		*/
}




VErrorDB4D_OnEMRec::VErrorDB4D_OnEMRec( VError inErrCode, ActionDB4D inAction, const Base4D *inBase, const EntityModel* em, const VString& key)
:VErrorDB4D_OnEM(inErrCode, inAction, inBase, em)
{
	fEntityKey = key;
	GetBag()->SetString(Db4DError::EntityNum, key);
}


void VErrorDB4D_OnEMRec::DumpToString( VString& outString) const
{
	VErrorDB4D_OnEM::DumpToString(outString);
}




VErrorDB4D_OnEMAttribute::VErrorDB4D_OnEMAttribute( VError inErrCode, ActionDB4D inAction, const Base4D *inBase, const EntityModel* em, const EntityAttribute* att)
:VErrorDB4D_OnEM(inErrCode, inAction, inBase, em)
{
	fAtt = att;
	if (att != nil)
	{
		GetBag()->SetString(Db4DError::EntityAttributeName, att->GetName());
	}
}


void VErrorDB4D_OnEMAttribute::DumpToString( VString& outString) const
{
	VErrorDB4D_OnEM::DumpToString(outString);
	/*
	outString += L"  ,  Attribute = ";
	if (fAtt != nil)
		outString += fAtt->GetName();
		*/
}


VErrorDB4D_OnEMMethod::VErrorDB4D_OnEMMethod( VError inErrCode, ActionDB4D inAction, const Base4D *inBase, const EntityModel* em, const EntityMethod* meth)
:VErrorDB4D_OnEM(inErrCode, inAction, inBase, em)
{
	fMethod = meth;
	if (meth != nil)
	{
		GetBag()->SetString(Db4DError::MethodName, meth->GetName());
	}
}


void VErrorDB4D_OnEMMethod::DumpToString( VString& outString) const
{
	VErrorDB4D_OnEM::DumpToString(outString);
	
}


VErrorDB4D_OnEMType::VErrorDB4D_OnEMType( VError inErrCode, ActionDB4D inAction, const Base4D *inBase, const AttributeType* attType)
:VErrorDB4D_OnBase(inErrCode, inAction, inBase)
{
	fAttType = attType;
	if (attType != nil)
	{
		GetBag()->SetString(Db4DError::TypeName, attType->GetName());
	}
}


void VErrorDB4D_OnEMType::DumpToString( VString& outString) const
{
	VErrorDB4D_OnBase::DumpToString(outString);
	/*
	outString += L"  ,  Attribute Type = ";
	if (fAttType != nil)
		outString += fAttType->GetName();
		*/
}



VErrorDB4D_OnEMEnum::VErrorDB4D_OnEMEnum( VError inErrCode, ActionDB4D inAction, const Base4D *inBase, const EmEnum* xenum)
:VErrorDB4D_OnBase(inErrCode, inAction, inBase)
{
	fEnum = xenum;
	if (xenum != nil)
	{
		GetBag()->SetString(Db4DError::EnumerationName, xenum->GetName());
	}
}


void VErrorDB4D_OnEMEnum::DumpToString( VString& outString) const
{
	VErrorDB4D_OnBase::DumpToString(outString);
	/*
	outString += L"  ,  Enumeration = ";
	if (fEnum != nil)
		outString += fEnum->GetName();
		*/
}





VErrorDB4D_OnEMRelation::VErrorDB4D_OnEMRelation( VError inErrCode, ActionDB4D inAction, const Base4D *inBase, const EntityRelation* rel)
:VErrorDB4D_OnBase(inErrCode, inAction, inBase)
{
	fRel = rel;
	if (rel != nil)
	{
		GetBag()->SetString(Db4DError::RelationName, rel->GetName());
	}
}


void VErrorDB4D_OnEMRelation::DumpToString( VString& outString) const
{
	VErrorDB4D_OnBase::DumpToString(outString);
	/*
	outString += L"  ,  Entity Relationship = ";
	if (fRel != nil)
		outString += fRel->GetName();
		*/
}


// ------------------






				/* ----------------------- */

CompareResult xTime::CompareTo( const xTime& inTime ) const
{
	/*
	if ( fYear > inTime.fYear )
		return CR_BIGGER;
	else if ( fYear < inTime.fYear )
		return CR_SMALLER;
	else
	{
		if ( fMonth > inTime.fMonth )
			return CR_BIGGER;
		else if( fMonth < inTime.fMonth )
			return CR_SMALLER;
		else
		{
			if ( fDay > inTime.fDay )
				return CR_BIGGER;
			else if ( fDay < inTime.fDay )
				return CR_SMALLER;
			else
			{
				if ( fMilliseconds > inTime.fMilliseconds )
					return CR_BIGGER;
				else if ( fMilliseconds < inTime.fMilliseconds )
					return CR_SMALLER;
				else
					return CR_EQUAL;
			}
		}
	}
	*/
	if (fValue > inTime.fValue)
		return CR_BIGGER;
	else if (fValue < inTime.fValue)
		return CR_SMALLER;
	else
		return CR_EQUAL;

}


Boolean xTime::operator == (const xTime &inTime) const
{
	return CompareTo(inTime)==CR_EQUAL;
}


Boolean xTime::operator > (const xTime &inTime) const
{
	return CompareTo(inTime)==CR_BIGGER;
}


Boolean xTime::operator < (const xTime &inTime) const
{
	return CompareTo(inTime)==CR_SMALLER;
}


Boolean xTime::operator >= (const xTime &inTime) const
{
	return CompareTo(inTime)>=CR_EQUAL;
}


Boolean xTime::operator <= (const xTime &inTime) const
{
	return CompareTo(inTime)<=CR_EQUAL;
}


Boolean xTime::operator != (const xTime &inTime) const
{
	return CompareTo(inTime)!=CR_EQUAL;
}



					/* ----------------------- */


void* xTempObject::operator new(size_t inSize)
{
#if USE_NATIVE_MEM_MGR
	return malloc(inSize);
#else
	return GetTempMem((VSize) inSize, true, 'obj ');
#endif
}


void xTempObject::operator delete(void* inAddr)
{
#if USE_NATIVE_MEM_MGR
	free(inAddr);
#else
	return FreeTempMem((VPtr)inAddr);
#endif
}


															/* ----------------------- */


sLONG xMultiFieldDataOffset::Set(sLONG xoffset, db4dEntityAttribute* inAtt, Boolean xascent) 
{ 
	offset = xoffset; 
	typ = inAtt->ComputeScalarType();
	numcrit = -1; 
	size = 0; 
	switch (typ)
	{
		case VK_TEXT:
		case VK_STRING:
			size = 30 + xString::NoDataSize();
			break;

		case VK_BOOLEAN:
		case VK_BYTE:
			size = 1;
			break;

		case VK_WORD:
			size = 2;
			break;

		case VK_LONG:
			size = 4;
			break;

		case VK_LONG8:
		case VK_DURATION:
			size = 8;
			break;

		case VK_TIME:
			size = sizeof(xTime);
			break;

		case VK_UUID:
			size = sizeof(VUUIDBuffer);
			break;

		case VK_REAL:
			size = sizeof(Real);
			break;
	}
	ascent = xascent; 
	att = inAtt;
	return size;
};



sLONG xMultiFieldDataOffset::Set(sLONG xoffset, AttributePath* inAttPath, Boolean xascent) 
{ 
	const db4dEntityAttribute* att = (db4dEntityAttribute*)(inAttPath->LastPart()->fAtt);

	offset = xoffset; 
	typ = att->ComputeScalarType();
	numcrit = -1; 
	size = 0; 
	switch (typ)
	{
	case VK_TEXT:
	case VK_STRING:
		size = 30 + xString::NoDataSize();
		break;

	case VK_BOOLEAN:
	case VK_BYTE:
		size = 1;
		break;

	case VK_WORD:
		size = 2;
		break;

	case VK_LONG:
		size = 4;
		break;

	case VK_LONG8:
	case VK_DURATION:
		size = 8;
		break;

	case VK_TIME:
		size = sizeof(xTime);
		break;

	case VK_UUID:
		size = sizeof(VUUIDBuffer);
		break;

	case VK_REAL:
		size = sizeof(Real);
		break;
	}
	ascent = xascent; 
	fAttPath = RetainRefCountable(inAttPath);
	return size;
};

void xMultiFieldDataOffset::Release()
{
	QuickReleaseRefCountable(fNulls);
	QuickReleaseRefCountable(fAttPath);
}



															/* ----------------------- */



bool xMultiFieldDataOffset::IsNull(sLONG recnum) const
{
	if (fNulls == nil)
		return false;
	else
		return fNulls->isOn(recnum);
}

void xMultiFieldDataOffset::SetNull(sLONG recnum) const
{
	if (fNulls == nil)
		((xMultiFieldDataOffset*)this)->fNulls = new Bittab();
	fNulls->Set(recnum);
}




xMultiFieldDataOffsets::xMultiFieldDataOffsets(Field* cri, sLONG typ, Boolean ascent, sLONG size)
{
	count = 0;
	fWithExpression = false;
	fWithAttributes = false;
	ismulti = false;
	if (cri == nil)
	{
		fOwner = nil;
		AddOffset(0, typ, -1, size, ascent);
	}
	else
	{
		fOwner = cri->GetOwner();
		AddOffset(0, typ, cri->GetPosInRec(), size, ascent);
	}
}


xMultiFieldDataOffsets::xMultiFieldDataOffsets(db4dEntityAttribute* att, Boolean ascent)
{
	count = 0;
	fWithExpression = false;
	fWithAttributes = true;
	ismulti = false;
	fOwner = att->GetTable();
	fLen = AddOffset(0, att, ascent);
}


xMultiFieldDataOffsets::~xMultiFieldDataOffsets()
{
	for (sLONG i = 0; i < kMaxMultiFieldData; ++i)
	{
		offsets[i].Release();
	}
}

bool xMultiFieldDataOffsets::IsAllDescent() const
{
	bool alldesc = true;
	for (sLONG i = 0; i < count; ++i)
	{
		if (offsets[i].IsAscent())
		{
			alldesc = false;
			break;
		}
	}
	return alldesc;
}




Boolean xMultiFieldDataOffsets::AddOffset(sLONG offset, sLONG typ, sLONG numcrit, sLONG size, Boolean ascent)
{
	if (count<kMaxMultiFieldData)
	{
		if (size == 0)
			fWithExpression = true;
		offsets[count].Set(offset, typ, numcrit, size, ascent);
		count++;
		return true;
	}
	else
		return false;
}


Boolean xMultiFieldDataOffsets::AddOffset(DB4DLanguageExpression* inExpression, Boolean ascent)
{
	if (count<kMaxMultiFieldData)
	{
		fWithExpression = true;
		offsets[count].Set(inExpression, ascent);
		count++;
		return true;
	}
	else
		return false;
}


sLONG xMultiFieldDataOffsets::AddOffset(sLONG offset, db4dEntityAttribute* inAtt, Boolean ascent)
{
	if (count<kMaxMultiFieldData)
	{
		sLONG result = offsets[count].Set(offset, inAtt, ascent);
		count++;
		fWithAttributes = true;
		return result;
	}
	else
		return -1;
}


sLONG xMultiFieldDataOffsets::AddOffset(sLONG offset, AttributePath* inAttPath, Boolean ascent)
{
	if (count<kMaxMultiFieldData)
	{
		sLONG result = offsets[count].Set(offset, inAttPath, ascent);
		count++;
		fWithAttributes = true;
		return result;
	}
	else
		return -1;
}





															/* ----------------------- */


xMultiFieldData::xMultiFieldData(const xMultiFieldData& other) // copy constructor
{
	data = nil;
	offsets = nil;
	CopyFrom(other);
}


xMultiFieldData& xMultiFieldData::operator =( const xMultiFieldData& other)
{
	CopyFrom(other);
	return *this;
}


xMultiFieldData::~xMultiFieldData() // non virtual on purpose
{
	if (offsets == nil && data != nil)
		FreeTempMem(data);
};


void xMultiFieldData::Free()
{
	xMultiFieldData* td = GetTrueData();
	xMultiFieldDataOffset_constiterator cur = td->offsets->Begin(), end = td->offsets->End();
	for (;cur != end; cur++)
	{
		char* pos = GetDataPtr(cur->GetOffset());
		if (cur->GetDataType() == VK_STRING) 
		{
			((xString*)pos)->Free();
		}
		if (cur->GetDataType() == VK_STRING_UTF8) 
		{
			((xStringUTF8*)pos)->Free();
		}
	}
}


void xMultiFieldData::CopyFrom(const xMultiFieldData& other)
{
	Boolean mustmakenew = false;

	if (offsets == nil)
	{
		if (other.GetTrueData()->offsets->GetLength() > sizeof(xMultiFieldData)-8)
			mustmakenew = true;
	}

	if (mustmakenew)
	{
		if (data == nil)
		{
			data = GetTempMem(other.GetTrueData()->offsets->GetLength(), false, 'xmul');
			assert(data != nil);
		}
		if (data != nil)
		{
			GetTrueData()->offsets = other.GetTrueData()->offsets;
			GetTrueData()->CopyFrom(other);
		}
	}
	else
	{
		offsets = other.GetTrueData()->offsets;
		xMultiFieldDataOffset_constiterator cur = offsets->Begin(), end = offsets->End();

		for (;cur != end; cur++)
		{
			char* pos = GetDataPtr(cur->GetOffset());
			const char* posother = other.GetDataPtr(cur->GetOffset());
			switch(cur->GetDataType()) 
			{
				case VK_LONG:
					*(sLONG*)pos = *(sLONG*)posother;
					break;

				case VK_LONG8:
				case VK_DURATION:
				case VK_SUBTABLE:
				case VK_SUBTABLE_KEY:
					*(sLONG8*)pos = *(sLONG8*)posother;
					break;

				case VK_REAL:
					*(Real*)pos = *(Real*)posother;
					break;

				case VK_WORD:
					*(sWORD*)pos = *(sWORD*)posother;
					break;

				case VK_BOOLEAN:
				case VK_BYTE:
					*(uCHAR*)pos = *(uCHAR*)posother;
					break;

				case VK_TIME:
					*(xTime*)pos = *(xTime*)posother;
					break;

				case VK_UUID:
					*(VUUIDBuffer*)pos = *(VUUIDBuffer*)posother;
					break;
					
				case VK_STRING:
					*(xString*)pos = *(xString*)posother;
					break;

				case VK_STRING_UTF8:
					*(xStringUTF8*)pos = *(xStringUTF8*)posother;
					break;

				default:
					assert(false);
					break;
			}
		}
	}
	
}

/*
Boolean xMultiFieldData::operator == ( const xMultiFieldData &other ) const
{
	return CompareTo(other)==CR_EQUAL;
}

Boolean xMultiFieldData::operator  > ( const xMultiFieldData &other ) const
{
	return CompareTo(other)==CR_BIGGER;
}

Boolean xMultiFieldData::operator  < ( const xMultiFieldData &other ) const
{
	return CompareTo(other)==CR_SMALLER;
}

Boolean xMultiFieldData::operator >= ( const xMultiFieldData &other ) const
{
	return CompareTo(other)>=CR_EQUAL;
}

Boolean xMultiFieldData::operator <= ( const xMultiFieldData &other ) const
{
	return CompareTo(other)<=CR_EQUAL;
}

Boolean xMultiFieldData::operator != ( const xMultiFieldData &other ) const
{
	return CompareTo(other)!=CR_EQUAL;
}
*/

CompareResult xMultiFieldData::CompareTo ( const xMultiFieldData& other, const VCompareOptions& inOptions, sLONG recnum1, sLONG recnum2 ) const
{
	CompareResult result = CR_EQUAL;

	if (inOptions.IsDiacritical())
	{
		VCompareOptions nondiac = inOptions;
		nondiac.SetDiacritical(false);
		result = CompareTo(other, nondiac, recnum1, recnum2);
		if (result != CR_EQUAL)
			return result;
		// on continue s'il y a egalite sur le non diacritique
	}

	xMultiFieldDataOffset_constiterator cur = GetTrueData()->offsets->Begin(), end = GetTrueData()->offsets->End();

	Boolean LastAscent;

	for (;cur != end && result == CR_EQUAL; cur++)
	{
		bool isnull = cur->IsNull(recnum1);
		bool isnullother = cur->IsNull(recnum2);
		LastAscent = cur->IsAscent();
		const char* pos = GetDataPtr(cur->GetOffset());
		const char* posother = other.GetDataPtr(cur->GetOffset());

		if (isnull)
		{
			if (isnullother)
				result = CR_EQUAL;
			else
				result = CR_SMALLER;
		}
		else if (isnullother)
		{
			result = CR_BIGGER;
		}
		else
		{
			switch(cur->GetDataType()) 
			{
			case VK_LONG:
				if (*(sLONG*)pos == *(sLONG*)posother)
					result = CR_EQUAL;
				else
				{
					if (*(sLONG*)pos < *(sLONG*)posother)
						result = CR_SMALLER;
					else
						result = CR_BIGGER;
				}
				break;

			case VK_LONG8:
			case VK_DURATION:
			case VK_SUBTABLE:
			case VK_SUBTABLE_KEY:
				if (*(sLONG8*)pos == *(sLONG8*)posother)
					result = CR_EQUAL;
				else
				{
					if (*(sLONG8*)pos < *(sLONG8*)posother)
						result = CR_SMALLER;
					else
						result = CR_BIGGER;
				}
				break;

			case VK_REAL:
				result = VReal::sInfo.CompareTwoPtrToDataWithOptions(pos, posother, inOptions);
				/*
				if (*(Real*)pos == *(Real*)posother)
					result = CR_EQUAL;
				else
				{
					if (*(Real*)pos < *(Real*)posother)
						result = CR_SMALLER;
					else
						result = CR_BIGGER;
				}
				*/
				break;

			case VK_BOOLEAN:
				if (*(uCHAR*)pos == *(uCHAR*)posother)
					result = CR_EQUAL;
				else
				{
					if (*(uCHAR*)pos < *(uCHAR*)posother)
						result = CR_SMALLER;
					else
						result = CR_BIGGER;
				}
				break;

			case VK_BYTE:
				if (*(sBYTE*)pos == *(sBYTE*)posother)
					result = CR_EQUAL;
				else
				{
					if (*(sBYTE*)pos < *(sBYTE*)posother)
						result = CR_SMALLER;
					else
						result = CR_BIGGER;
				}
				break;

			case VK_WORD:
				if (*(sWORD*)pos == *(sWORD*)posother)
					result = CR_EQUAL;
				else
				{
					if (*(sWORD*)pos < *(sWORD*)posother)
						result = CR_SMALLER;
					else
						result = CR_BIGGER;
				}
				break;

			case VK_TIME:
				if (*(xTime*)pos == *(xTime*)posother)
					result = CR_EQUAL;
				else
				{
					if (*(xTime*)pos < *(xTime*)posother)
						result = CR_SMALLER;
					else
						result = CR_BIGGER;
				}
				break;

			case VK_UUID:
				if (*(VUUIDBuffer*)pos == *(VUUIDBuffer*)posother)
					result = CR_EQUAL;
				else
				{
					if (*(VUUIDBuffer*)pos < *(VUUIDBuffer*)posother)
						result = CR_SMALLER;
					else
						result = CR_BIGGER;
				}
				break;
					
			case VK_STRING:
				result = ((xString*)pos)->CompareTo(*(xString*)posother, inOptions);
				break;

			case VK_STRING_UTF8:
				result = ((xStringUTF8*)pos)->CompareTo(*(xStringUTF8*)posother, inOptions);
				break;

			default:
				assert(false);
				break;
			}
		}
	}

	if (result != CR_EQUAL)
	{
		if (!LastAscent)
		{
			if (result == CR_BIGGER)
				result = CR_SMALLER;
			else
				result = CR_BIGGER;
		}
	}

	return result;
}


void* xMultiFieldData::CopyTo(void* pData) const
{
	xMultiFieldDataOffset_constiterator cur = GetTrueData()->offsets->Begin(), end = GetTrueData()->offsets->End();
	for (;cur != end; cur++)
	{
		const char* pos = GetDataPtr(cur->GetOffset());
		if (cur->GetDataType() == VK_STRING)
		{
			pData = ((xString*)pos)->copyto(pData);
		}
		else
		{
			if (cur->GetDataType() == VK_STRING)
			{
				pData = ((xString*)pos)->copyto(pData);
			}
			else
			{
				sLONG len = cur->GetSize();
				vBlockMove(pos, pData, len);
				pData = (void*) (((char*)pData) + len);
			}
		}
	}
	return pData;
}



															/* ----------------------- */


ValPtr NewValPtr(sLONG typ, void* from, sLONG fromType, DataTable* df, BaseTaskInfo* context)
{
	ValPtr cv2 = nil;
	CreVValue_Code Code;

	Code=FindCV(fromType);
	if (Code != nil)
	{
		ValPtr cv = (*Code)(df, -1, from, false, false, context, creVValue_default);
		if (cv != nil)
		{
			cv2 = cv->ConvertTo(typ);
			delete cv;
		}

	}
	return cv2;
}



														/* -------------------------------- */



ExtraPropertiesHeader::~ExtraPropertiesHeader()
{
	if (fExtra != nil)
		fExtra->Release();
	if (fExtraPropertiesObjRef)
		delete fExtraPropertiesObjRef;
	if (fExtraStream != nil)
		delete fExtraStream;
}


VValueBag* ExtraPropertiesHeader::RetainExtraProperties(VError &err)
{
	err = VE_OK;
	VValueBag* result = NULL;

	if (fIsRemote)
	{
		CopyRefCountable( &result, fExtra);
	}
	else
	{
		occupe();
		if (fExtra == nil)
		{
			if (*fExtraAddr != 0 && *fExtraLen != 0 && !db->StoredAsXML())
			{
				DataBaseObjectHeader tag;

				err = tag.ReadFrom(db, *fExtraAddr);
				if (err==VE_OK)
				{
					if (tag.ValidateTag(fTagInfo, fPos1, fPos2) == VE_OK)
					{
						sLONG	lenx=tag.GetLen();
						void* p=GetFastMem(lenx, false, 'extr');
						assert(lenx == (*fExtraLen - kSizeDataBaseObjectHeader));

						if (p!=nil)
						{
							err=db->readlong(p,lenx,*fExtraAddr,kSizeDataBaseObjectHeader);
							if (err==VE_OK)
							{
								err = tag.ValidateCheckSum(p,lenx);
								if (err == VE_OK)
								{
									VConstPtrStream buf(p, lenx);
									err = buf.OpenReading();
									if (err == VE_OK)
									{
										buf.SetNeedSwap(tag.NeedSwap());
										fExtra = new VValueBag();
										err = fExtra->ReadFromStream(&buf);
										if (err != VE_OK)
										{
											fExtra->Release();
											fExtra = nil;
										}
									}
								}
							}

							FreeFastMem(p);
						}
						else
							err = ThrowBaseError(memfull, DBaction_AccessingExtraProperty);
					}
					else
					{
						err = ThrowBaseError(VE_DB4D_CANNOT_GET_EXTRAPROPERTY, DBaction_AccessingExtraProperty);
					}
				}
			}
			else if (db->StoredAsXML())
			{
				VFilePath path; 
				db->GetDataSegPath(1,path);//First seg is 1 not 0 :)
				path.SetExtension( kDataExtraExt);

				if (!path.IsFile() || !path.IsValid())
				{
					err = VE_DB4D_CANNOT_GET_EXTRAPROPERTY;
				}
				else
				{
					VFile *extraFile = new VFile( path);
					if (extraFile && extraFile->Exists())
					{
						VFileStream stream(extraFile,FO_Default);
						err = stream.OpenReading();
						if (err == VE_OK)
						{
							VString json;
							stream.SetCharSet(VTC_UTF_8);
							stream.GetText( json);
							if(err == VE_OK)
							{
								fExtra = new VValueBag();
								err =  fExtra->FromJSONString(json);
								if(err != VE_OK)
								{
									delete fExtra;
									fExtra = nil;
								}
							}
						}
						stream.CloseReading();
					}
					XBOX::ReleaseRefCountable(&extraFile);
				}
			}
		}
		
		if (fExtra != nil)
		{
			fExtra->Retain();
		}

		result = fExtra;
		libere();
	}

	if (err != VE_OK)
		err = ThrowBaseError(VE_DB4D_CANNOT_GET_EXTRAPROPERTY, DBaction_AccessingExtraProperty);

	return result;
}


VError ExtraPropertiesHeader::SetExtraProperties(VValueBag* inExtraProperties, bool loadonly)
{
	VError err = VE_OK;
	Boolean AddrHasChanged = false;

	if (fIsRemote || loadonly)
	{
		CopyRefCountable( &fExtra, inExtraProperties);
	}
	else
	{
		if (db->OkToUpdate(err))
		{
			occupe();

			if (inExtraProperties != nil)
				inExtraProperties->Retain();
			if (fExtra != nil)
				fExtra->Release();
			fExtra = inExtraProperties;

			if (!db->StoredAsXML())
			{
				if (fExtraStream != nil)
				{
					delete fExtraStream;
					fExtraStream = nil;
					fExtraIsModified = false;
				}

				if (fExtra == nil)
				{
					if (*fExtraAddr != 0 && *fExtraLen != 0)
					{
						db->libereplace(*fExtraAddr, *fExtraLen, nil, nil);
						*fExtraAddr = 0;
						*fExtraLen = 0;
					}
				}
				else
				{
					fExtraStream = new xbox::VPtrStream();
					if (fExtraStream == nil)
						err = ThrowBaseError(memfull, DBaction_ModifyingExtraProperty);
					else
					{
						err = fExtraStream->OpenWriting();
						fExtraStream->SetNeedSwap(false);
						if (err == VE_OK)
						{
							err = fExtra->WriteToStream(fExtraStream);
							if (err == VE_OK)
							{
								sLONG newlen = fExtraStream->GetSize() + kSizeDataBaseObjectHeader;
								if (*fExtraAddr != 0 && *fExtraLen != 0)
								{
									if (adjuste(newlen) != adjuste(*fExtraLen))
									{
										AddrHasChanged = true;
										DataAddr4D ou = db->findplace(newlen, nil, err, 0, nil);
										if (ou > 0 && err == VE_OK)
										{
											db->libereplace(*fExtraAddr, *fExtraLen, nil, nil);
											*fExtraAddr = ou;
											*fExtraLen = newlen;
											fExtraIsModified = true;
										}
										else
										{
											err = ThrowBaseError(VE_DB4D_CANNOT_WRITE_EXTRAPROPERTIES, DBaction_ModifyingExtraProperty);
										}
									}
									else
									{
										*fExtraLen = newlen;
										fExtraIsModified = true;
									}
								}
								else
								{
									AddrHasChanged = true;
									*fExtraAddr = db->findplace(newlen, nil, err, 0, nil);
									if (*fExtraAddr > 0 && err == VE_OK)
									{
										*fExtraLen = newlen;
										fExtraIsModified = true;
									}
									else
									{
										err = ThrowBaseError(VE_DB4D_CANNOT_WRITE_EXTRAPROPERTIES, DBaction_ModifyingExtraProperty);
										*fExtraAddr = 0;
										*fExtraLen = 0;
									}
								}

								if (err == VE_OK)
								{
									if (fExtraPropertiesObjRef == nil)
									{
										fExtraPropertiesObjRef = new ExtraPropertiesObj(this);
										if (fExtraPropertiesObjRef == nil)
										{
											err = ThrowBaseError(memfull, DBaction_ModifyingExtraProperty);
											*fExtraAddr = 0;
											*fExtraLen = 0;
											fExtraIsModified = false;
										}
										else
										{
											fExtraPropertiesObjRef->setaddr(*fExtraAddr);
											fExtraPropertiesObjRef->setmodif(true, db, nil);
											fExtraPropertiesObjRef->libere();
										}
									}
									else
									{
										fExtraPropertiesObjRef->occupe();
										if (AddrHasChanged)
											fExtraPropertiesObjRef->ChangeAddr(*fExtraAddr, db, nil);
										else
											fExtraPropertiesObjRef->setmodif(true, db, nil);
										fExtraPropertiesObjRef->libere();
									}
								}

							}
						}

						if (err != VE_OK)
						{
							delete fExtraStream;
							fExtraStream = nil;
						}
					}
				}
			}
			else
			{
				// write bag as json file next the first data segment
				VFilePath path; 
				db->GetDataSegPath(1,path);//First seg is 1 not 0 :)
				path.SetExtension( kDataExtraExt);
				if (!path.IsFile() || !path.IsValid())
				{
					err = VE_DB4D_CANNOT_SET_EXTRAPROPERTY;
				}
				else
				{
					VFile *extraFile = new VFile( path);
					if (fExtra == nil)
					{
						err = extraFile->Delete();
					}
					else
					{
						VString json;
						fExtra->GetJSONString( json,JSON_PrettyFormatting);
						
						VFileStream stream(extraFile,FO_CreateIfNotFound|FO_Overwrite);
						err = stream.OpenWriting();
						if (err == VE_OK)
						{
							err = stream.SetCharSet(VTC_UTF_8);
							if(err == VE_OK)
							{
								err = stream.PutText(json);
							}
							stream.CloseWriting();
						}
					}
					extraFile->Release();
				}
			}
			libere();
			db->ClearUpdating();
		}
	}

	if (err != VE_OK)
		err = ThrowBaseError(VE_DB4D_CANNOT_SET_EXTRAPROPERTY, DBaction_ModifyingExtraProperty);
	return err;
}



uBOOL ExtraPropertiesObj::okdel(void)
{
	return !modifie();
}


sLONG ExtraPropertiesObj::saveobj(void)
{
#if debuglogwrite
	VString wherefrom(L"ExtraPropertiesObj SaveObj");
	VString* whx = &wherefrom;
#else
	VString* whx = nil;
#endif

	if (fOwner->occupe())
	{
		return 0;
	}
	else
	{
		sLONG result = 10;
		xbox::VPtrStream* stream = fOwner->GetData();
		if (testAssert(stream != nil))
		{
			result = stream->GetSize();
			void* p = stream->GetDataPtr();
			assert(p != nil);
			assert(result>0);
			DataBaseObjectHeader tag(p,result, fOwner->GetTagInfo(), fOwner->GetPos1(), fOwner->GetPos2());
			VError err;
			Base4D* db = fOwner->GetDB();
			err = tag.WriteInto(db, getaddr(), whx);
			if (err == VE_OK)
				err=db->writelong(p,result,getaddr(),kSizeDataBaseObjectHeader, whx);
			assert(err == VE_OK);
		}
		fOwner->ClearData();
		fOwner->libere();
		return result;
	}
}


sLONG ExtraPropertiesObj::liberemem(sLONG allocationBlockNumber, sLONG combien, uBOOL tout)
{
	if (fOwner->occupe())
		return 0;
	else
	{
		sLONG result = 0;
		if (okdel())
		{
			fOwner->ClearExtraPropertiesObjRef();
			result = sizeof(ExtraPropertiesObj)+16;
		}
		fOwner->libere();
		return result;
	}
}




											/* ----------------------------------------------------------- */


VFileStream* CreateTempFile(Base4D* db, VFolder* inFolder, uLONG8 size)
{
	StErrorContextInstaller temperrs;

	VFileStream* result = nil;
	VFolder *folderToRelease = nil;

	if (inFolder == nil)
	{
		folderToRelease = db->RetainTemporaryFolder();
		inFolder = folderToRelease;
	}

	if (inFolder != nil)
	{
		VUUID xid;
		xid.Regenerate();
		VString filename = L"TempSort_";
		VString s;
		s.FromValue(xid);
		filename += s;

		VFile* file = new VFile(*inFolder, filename);
		file->Delete();

		VError err = file->Create(true);
		if (err == VE_OK)
		{
			result = new VFileStream(file);
		}
		file->Release();
	}
	
	ReleaseRefCountable( &folderToRelease);

	return result;
}


								/* ----------------------------------------------------------- */



TabAddrCache::TabAddrCache(Base4D_NotOpened *owner, DataAddr4D primaddr, sLONG nbelem)
{
	fOwner = owner;
	fPrimAddr = primaddr;
	fNbelems = nbelem;
	fAllocationIsValid = false;
	fHasAlreadyTriedAllocate = false;
}


void TabAddrCache::_InitIfNecessary()
{
	if (!fHasAlreadyTriedAllocate)
	{
		fHasAlreadyTriedAllocate = true;
		try
		{
			AddrDBObj empty;
			empty.addr = 0;
			empty.len = 0;
			fAddrs.resize(fNbelems, empty);
			fAllocationIsValid = true;
		}
		catch (...)
		{
			fAllocationIsValid = false;
			fAddrs.clear();
		}
	}
}


void TabAddrCache::AddAddr(sLONG position, DataAddr4D addr, sLONG len)
{
	_InitIfNecessary();
	if (fAllocationIsValid)
	{
		AddrDBObj x;
		x.addr = addr;
		x.len = len;
		if (position >= 0 && position < fNbelems)
			fAddrs[position] = x;
		else
			assert(false);
	}
}


Boolean TabAddrCache::GetAddr(sLONG position, DataAddr4D& outAddr, sLONG& outLen)
{
	if (fAllocationIsValid)
	{
		if (position >= 0 && position < fNbelems)
		{
			AddrDBObj x;
			x = fAddrs[position];
			outAddr = x.addr;
			outLen = x.len;
			return true;
		}
		else
		{
			assert(false);
			return false;
		}
	}
	else
		return false;
}




void RemoveExtension(VString& s)
{
	sLONG p = s.FindUniChar('.', s.GetLength(), true);
	if (p != 0)
		s.Remove(p, s.GetLength()-p + 1);
}


void SetCompOptionWithOperator(VCompareOptions& inOptions, sLONG compOperator)
{
	inOptions.SetLike(false);
	inOptions.SetBeginsWith(false);
	switch(compOperator)
	{
		case DB4D_Like:
		case DB4D_NotLike:
		case DB4D_Greater_Like:
		case DB4D_GreaterOrEqual_Like:
		case DB4D_Lower_Like:
		case DB4D_LowerOrEqual_Like:
		case DB4D_Contains_KeyWord_Like:
		case DB4D_Doesnt_Contain_KeyWord_Like:
			inOptions.SetLike(true);
			break;

		case DB4D_BeginsWith:
		case DB4D_Contains_KeyWord_BeginingWith:
			inOptions.SetBeginsWith(true);
			break;
	}
}



			// -------------------------------------------------------------------------------------------- //



class UtilVValuesCollection : public DB4DCollectionManager
{
	public:
		typedef vector<VRefPtr<CDB4DField> > FieldsVect;
		typedef vector<VValueSingle*> ValuesVect;
		typedef vector<ValuesVect> ValuesCollection;

		virtual ~UtilVValuesCollection()
		{
			for (ValuesCollection::iterator cur = fValues.begin(), end = fValues.end(); cur != end; cur++)
			{
				ValuesVect* values = &(*cur);
				for (ValuesVect::iterator curvalue = values->begin(), endvalue = values->end(); curvalue != endvalue; curvalue++)
				{
					VValueSingle* cv = *curvalue;
					if (cv != nil)
						delete cv;
				}
			}
		}

		virtual VErrorDB4D SetCollectionSize(RecIDType size, Boolean ClearData = true)
		{
			VErrorDB4D err = VE_OK;
			try
			{
				for (ValuesCollection::iterator cur = fValues.begin(), end = fValues.end(); cur != end; cur++)
				{
					cur->resize(size, nil);
				}
			}
			catch (...)
			{
				err = ThrowBaseError(memfull, noaction);
			}

			return err;
		}

		virtual RecIDType GetCollectionSize()
		{
			if (fValues.empty())
				return 0;
			else
				return (RecIDType)fValues[0].size();
		}

		virtual sLONG GetNumberOfColumns()
		{
			return (sLONG)fCols.size();
		}

		virtual	bool AcceptRawData()
		{
			return false;
		}

		virtual CDB4DField* GetColumnRef(sLONG ColumnNumber)
		{
			if (ColumnNumber > 0 && ColumnNumber <= (sLONG)fCols.size())
			{
				return fCols[ColumnNumber-1].Get();
			}
			else
				return nil;
		}

		virtual VErrorDB4D SetNthElement(RecIDType ElemNumber, sLONG ColumnNumber, const XBOX::VValueSingle& inValue)
		{
			VErrorDB4D err = VE_DB4D_INDICE_OUT_OF_RANGE;
			if (ColumnNumber>0 && ColumnNumber <= (sLONG)fValues.size())
			{
				ValuesVect* values = &(fValues[ColumnNumber-1]);
				if (ElemNumber>0 && ElemNumber <= (sLONG)values->size())
				{
					err = VE_OK;
					VValueSingle* cv = (*values)[ElemNumber-1];
					if (cv != nil)
						delete cv;
					cv = inValue.Clone();
					if (cv == nil)
						err = ThrowBaseError(memfull, noaction);
					(*values)[ElemNumber-1] = cv;
				}
				
			}
			return err;
		}

		// set *outRejected to true if you are not pleased with given raw data and want to get called with SetNthElement instead for this row (already initialized to false)
		virtual VErrorDB4D SetNthElementRawData(RecIDType ElemNumber, sLONG ColumnNumber, const void *inRawValue, XBOX::ValueKind inRawValueKind, bool *outRejected)
		{
			*outRejected = true;
			return VE_DB4D_NOTIMPLEMENTED;
		}

		virtual VErrorDB4D GetNthElement(RecIDType ElemNumber, sLONG ColumnNumber, const XBOX::VValueSingle*& outValue, bool *outDisposeIt)
		{
			*outDisposeIt = false;
			VErrorDB4D err = VE_DB4D_INDICE_OUT_OF_RANGE;
			if (ColumnNumber>0 && ColumnNumber <= (sLONG)fValues.size())
			{
				ValuesVect* values = &(fValues[ColumnNumber-1]);
				if (ElemNumber>0 && ElemNumber <= (sLONG)values->size())
				{
					err = VE_OK;
					outValue = (*values)[ElemNumber-1];
				}
			}
			return err;
		}

		virtual VErrorDB4D AddOneElement(sLONG ColumnNumber, const XBOX::VValueSingle& inValue)
		{
			VErrorDB4D err = VE_DB4D_INDICE_OUT_OF_RANGE;
			if (ColumnNumber>0 && ColumnNumber <= (sLONG)fValues.size())
			{
				ValuesVect* values = &(fValues[ColumnNumber-1]);
				VValueSingle* cv = inValue.Clone();
				if (cv == nil)
					err = ThrowBaseError(memfull, noaction);
				else
				{
					try
					{
						values->push_back(cv);
						err = VE_OK;
					}
					catch (...)
					{
						delete cv;
						err = ThrowBaseError(memfull, noaction);
					}
				}
			}
			return err;
		}

		virtual	XBOX::VSize GetNthElementSpace( RecIDType inIndex, sLONG inColumnNumber, XBOX::ValueKind inRawValueKind, bool *outRejected, XBOX::VError& outError)
		{
			return 0;
		}

		virtual	void* WriteNthElementToPtr( RecIDType inIndex, sLONG inColumnNumber, void *outRawValue, XBOX::ValueKind inRawValueKind, bool *outRejected, XBOX::VError& outError)
		{
			return nil;
		}

		virtual sLONG GetSignature()
		{
			return 0;
		}

		virtual XBOX::VError PutInto(XBOX::VStream& outStream)
		{
			return VE_DB4D_NOTIMPLEMENTED;
		}
		//		virtual XBOX::VError GetFrom(XBOX::VStream& inStream) = 0;

		VError _ResizeColumns(sLONG nbcol)
		{
			VError err = VE_OK;
			try
			{
				fCols.resize(nbcol, nil);
				fValues.resize(nbcol);
			}
			catch (...)
			{
				err = ThrowBaseError(memfull, noaction);
			}
			return err;
		}

		void _SetOneCol(sLONG colnum, CDB4DField* ff)
		{
			fCols[colnum-1] = ff;
		}

		void _SetNthElement(RecIDType ElemNumber, sLONG ColumnNumber, VValueSingle* inValue)
		{
			fValues[ColumnNumber-1][ElemNumber-1] = inValue;
		}

	protected:

		FieldsVect fCols;
		ValuesCollection fValues;


};


			// -------------------------------------------------------------------------------------------- //



void IRequest::PutBaseParam( const Base4D* inBase)
{
	assert(inBase != nil);
	inBase->GetUUID().WriteToStream(GetOutputStream());
}


void IRequest::PutTableParam( const Table* inTable)
{
	assert(inTable != nil);
	GetOutputStream()->PutLong(inTable->GetNum());
}


void IRequest::PutFieldParam( const Field *inField)
{
	assert(inField != NULL);
	GetOutputStream()->PutLong(inField->GetPosInRec());
}


void IRequest::PutRelationParam( const Relation *inRelation)
{
	assert(inRelation != NULL);
	//GetOutputStream()->PutLong(inRelation->GetPosInList());
	inRelation->GetUUID().WriteToStream(GetOutputStream());

}


void IRequest::PutSelectionParam(Selection* inSel, CDB4DBaseContext* inContext)
{
	assert(inSel != nil);
	VStream* reqstream = GetOutputStream();
	//reqstream->PutLong(inSel->GetTypSel());
	inSel->ToServer(reqstream, inContext);
}


void IRequest::PutSetParam(Bittab* inSet, CDB4DBaseContext* inContext)
{
	assert(inSet != nil);
	VStream* reqstream = GetOutputStream();
	inSet->ToServer(reqstream, inContext, false);
}



void IRequest::PutFicheInMemParam(FicheInMem* rec, CDB4DBaseContext* inContext)
{
	assert(rec != nil);
	VStream* reqstream = GetOutputStream();
	BaseTaskInfoPtr context = ConvertContext(inContext);
	rec->ToServer(reqstream, context);
}


void IRequest::PutFicheInMemMinimalParam(FicheInMem* rec, CDB4DBaseContext* inContext)
{
	assert(rec != nil);
	VStream* reqstream = GetOutputStream();
	BaseTaskInfoPtr context = ConvertContext(inContext);
	rec->ToServerMinimal(reqstream, context);
}

/*******************************************************
T.A., 2010-07

VLogThingsToForget
	** NOT THREAD SAFE ***
	Because not supposed to be used in this context: each task logs its own infos, it's the RequestLogger that takes care of
	locking threads before writing to disk the message of one task.

	WARNING: do not put line ending (CR and or LF) in the things you log (or pass true to Log() in the destructor)

	Routines are explicitely inlined because we try to be as fast as possible, to avoid slowing down things when the
	request log is not enabled (which is 99% of the cases after all!)
********************************************************/
class VLogThingsToForget : public XBOX::VObject
{
public:
					VLogThingsToForget(VDBMgr *inMgr, BaseTaskInfo *inContext) :
						fDBMgr(inMgr),
						fContext(inContext),
						fRequestLoggerWasOn(false)
						{
							if(fDBMgr != NULL && fContext != NULL)
							{
								fRequestLoggerWasOn = fDBMgr->GetRequestLogger() != NULL && fDBMgr->GetRequestLogger()->IsEnable();
							}
						}

					~VLogThingsToForget()
					{
						if(		fRequestLoggerWasOn
					// No need to check fDBMgr and fContext, constructor has set fRequestLoggerWasOn top false if at least one of the both was NULL
							&&	!fToLog.IsEmpty()
							&&	fDBMgr->GetRequestLogger() != NULL
							)

						{
							fToLog.Insert(CVSTR("TTF/"), 1); // "Things To Forget"
							fDBMgr->GetRequestLogger()->Log(0, fContext->GetEncapsuleur(), fToLog, 0);
						}
					}


			void	Append(UniChar inToAdd)
			{
				if(fRequestLoggerWasOn && inToAdd != 0)
				{
					_InsertSlash();
					fToLog.AppendUniChar(inToAdd);
				}
			}

			// Used in *Put* Things To Forget
			void	AppendPushedRecInfos(const vectorPushedRecs &inPushedRect)
			{
				if(fRequestLoggerWasOn && !inPushedRect.empty())
				{
					_InsertSlash();
					for (vectorPushedRecs::const_iterator cur = inPushedRect.begin(), end = inPushedRect.end(); cur != end; ++cur)
					{
						_AppendPushRecInfo(cur->fNumTable, cur->fNumRec, cur->fMarkForPush);
					}
				}
			}

			// Used in *Get* Things To Forget
			void	AppendPushedRecInfos(sLONG intable, sLONG inRecNum, uBYTE inMarkForPush)
			{
				if(fRequestLoggerWasOn)
				{
					_InsertSlash();
					_AppendPushRecInfo(intable, inRecNum, inMarkForPush);
				}
			}


			void	AppendTransactionInfos(const vector<uBYTE> &inTransInfos)
			{
				if(fRequestLoggerWasOn && !inTransInfos.empty())
				{
					_InsertSlash();
					for (vector<uBYTE>::const_iterator cur = inTransInfos.begin(), end = inTransInfos.end(); cur != end; ++cur)
					{
						fToLog.AppendUniChar((UniChar) 't');
						fToLog.AppendLong(*cur);
					}
				}
			}


private:
			void	_InsertSlash()
					{
						if(!fToLog.IsEmpty())
							fToLog.AppendUniChar((UniChar) '/');
					}

			void	_AppendPushRecInfo(sLONG inTable, sLONG inRecNum, uBYTE inMarkForPush)
			{
				fToLog.AppendUniChar((UniChar) 'p');
				fToLog.AppendLong(inTable);
				fToLog.AppendUniChar((UniChar) '-');
				fToLog.AppendLong(inRecNum);
				fToLog.AppendUniChar((UniChar) '-');
				fToLog.AppendLong(inMarkForPush);
			}

	bool			fRequestLoggerWasOn;
	XBOX::VString	fToLog;

	VDBMgr			*fDBMgr;	// Not retained
	BaseTaskInfo	*fContext;	// Not retained

};


/********************************************************
********************************************************/

void _PutThingsToForget( VStream* into, VDBMgr *inMgr, BaseTaskInfo* context)
{
	VLogThingsToForget		ttfToLog(inMgr, context);// ttf == "Things To Forget"

	if (context != nil)
	{
		into->PutLong(context->GetBase()->GetRemoteMaxRecordsStamp());
	}
	else
		into->PutLong(-1);

	if (inMgr->HasSomeReleasedObjects(context))
	{
		// selections
		{
			vector<VUUIDBuffer> ids;
			inMgr->StealListOfReleasedSelIDs( ids);

			if (!ids.empty())
			{
				into->Put( 's');
				into->Put( (sLONG) ids.size());
				into->PutBytes( &ids.front(), (sLONG)ids.size() * sizeof( VUUIDBuffer));
				
				ttfToLog.Append( (UniChar) 's');
			}
		}

		// records
		{
			vector<DB4D_RecordRef> ids;
			inMgr->StealListOfReleasedRecIDs( ids, context);

			if (!ids.empty())
			{
				into->Put( 'r');
				into->Put( (sLONG) ids.size());
				into->PutBytes( &ids.front(), (sLONG)ids.size() * sizeof( DB4D_RecordRef));
				
				ttfToLog.Append( (UniChar) 'r');
			}
		}

	}
	if (context != nil)
	{
		// pushrecordsIDs
		{
			vectorPushedRecs pushedRecs;
			inMgr->StealMarkedRecordsAsPushed(pushedRecs);
			if (!pushedRecs.empty())
			{
				into->Put( 'p');
				into->Put( (sLONG) pushedRecs.size());
				for (vectorPushedRecs::iterator cur = pushedRecs.begin(), end = pushedRecs.end(); cur != end; cur++)
				{
					into->PutLong(cur->fNumTable);
					into->PutLong(cur->fNumRec);
					into->PutByte(cur->fMarkForPush);
				}

				ttfToLog.AppendPushedRecInfos(pushedRecs);
			}
		}

		vector<uBYTE> remoteTrans;
		context->StealRemoteTransactions(remoteTrans);
		sLONG nb = (sLONG)remoteTrans.size();
		if (nb != 0)
		{
			into->Put( 't');
			into->PutLong(nb);
			into->PutBytes(&remoteTrans[0], nb);
			
			ttfToLog.AppendTransactionInfos(remoteTrans);
		}
		
		// automatic relations
		if (context->NeedsToSendAutoRelInfo())
		{
			into->Put( 'a');
			VError err = context->PutAutoRelInfoToStream( into);
			if (err == VE_OK)
				context->AutoRelInfoWasSent();
			
			ttfToLog.Append( (UniChar) 'a');
		}
		
		if (context->getContextOwner()->NeedsToSendExtraData())
		{
			const VValueBag *extraData = context->getContextOwner()->RetainExtraData();
			into->Put( 'i');
			into->PutByte( (extraData != nil) ? 1 : 0);
			if (extraData != nil)
			{
				VError err = extraData->WriteToStream( into);
				if (err == VE_OK)
					context->getContextOwner()->ClearNeedsToSendExtraData();
			}
			ReleaseRefCountable( &extraData);
			
			ttfToLog.Append( (UniChar) 'i');
		}
	}
	into->Put( '.'); // end marker
}


void IRequest::PutThingsToForget( VDBMgr *inMgr, BaseTaskInfo* context)
{
	VStream* into = GetOutputStream();
	_PutThingsToForget(into, inMgr, context);
}

void IRequest::PutSortTabParams(SortTab* tabs)
{
	assert(tabs != nil);
	VStream* reqstream = GetOutputStream();
	tabs->PutInto(reqstream);
}

void IRequest::PutIndexParam( const IndexInfo *inIndex)
{
	assert(inIndex != NULL);
	inIndex->GetID().WriteToStream( GetOutputStream());
}

void IRequest::PutQueryParam( SearchTab *query)
{
	assert(query != nil);
	VStream* reqstream = GetOutputStream();
	query->PutInto(*reqstream);
}

void IRequest::PutQueryOptionsParam( QueryOptions* inQueryOptions, BaseTaskInfo* context)
{
	assert(inQueryOptions != nil);
	VStream* reqstream = GetOutputStream();
	inQueryOptions->ToServer(reqstream, context);
}


void IRequest::PutValueParam(const VValueSingle* inVal)
{
	VStream* reqstream = GetOutputStream();
	if (inVal == nil)
		reqstream->PutLong(VK_EMPTY);
	else
	{
		reqstream->PutLong((sLONG)inVal->GetValueKind());
		inVal->WriteToStream(reqstream);
	}
}


void IRequest::PutCollectionParam(DB4DCollectionManager& inCollection, bool OnlyHeader)
{
	VError err = VE_OK;
	VStream* reqstream = GetOutputStream();
	sLONG nbcol = inCollection.GetNumberOfColumns();
	sLONG nbrow = inCollection.GetCollectionSize();
	if (OnlyHeader)
		nbrow = 0;
	err = reqstream->PutLong(nbcol);
	if (err == VE_OK)
		err = reqstream->PutLong(nbrow);

	if (err == VE_OK)
	{
		for (sLONG i = 1; i <= nbcol && err == VE_OK; i++)
		{
			CDB4DField* f = inCollection.GetColumnRef(i);
			if (f == nil)
				err = reqstream->PutByte('.');
			else
			{
				if (f->GetID() == 0)
				{
					err = reqstream->PutByte('*');
					err = reqstream->PutLong(f->GetOwner()->GetID());
					err = reqstream->PutLong(0);
				}
				else
				{
					err = reqstream->PutByte('+');
					VUUID xid;
					f->GetUUID(xid);
					err = xid.WriteToStream(reqstream);
				}
			}
		}
	}

	if (err == VE_OK)
	{
		for (sLONG i = 1; i <= nbrow && err == VE_OK; i++)
		{
			for (sLONG j = 1; j <= nbcol && err == VE_OK; j++)
			{
				ConstValPtr from;
				bool DisposeIt;
				err = inCollection.GetNthElement(i, j, from, &DisposeIt);
				if (err == VE_OK)
				{
					err = reqstream->PutLong((sLONG)from->GetValueKind());
					if (err == VE_OK)
						err = from->WriteToStream(reqstream);
					if (DisposeIt)
						delete from;
				}
				else
					reqstream->SetError(err);

			}
		}
	}

}



VError IRequest::PutColumnFormulasParam(ColumnFormulas* formulas, Base4D* inBase)
{
	VStream* reqstream = GetOutputStream();
	return formulas->WriteToStream(reqstream, inBase);
}


void IRequest::PutProgressParam(VDB4DProgressIndicator* inProgress)
{
	VStream* reqstream = GetOutputStream();
	if(inProgress)
	{
		inProgress->WriteInfoForRemote(*reqstream);
	}
	else
		reqstream->PutWord(0);
	//sLONG ProgressID = VDBMgr::GetManager()->GetClientProgressID(inProgress);
	//sLONG ProgressID = 0;
	
	/*
	if (inProgress != nil)
		ProgressID = inProgress->getid();
		*/

	
	
}


VError IRequest::PutNumFieldArrayParam( const NumFieldArray& inNumFieldArray)
{
	VStream *reqstream = GetOutputStream();
	VError err = reqstream->PutLong( inNumFieldArray.GetCount());
	if (err == VE_OK)
	{
		for (NumFieldArray::ConstIterator iter = inNumFieldArray.First() , end = inNumFieldArray.End() ; iter != end && err == VE_OK ; ++iter)
			err = reqstream->PutLong( *iter);
	}
	return err;
}


Selection* IRequest::RetainSelectionReply(Base4D* inBase, VError& outErr, CDB4DBaseContextPtr inContext)
{
	/*
	Selection* sel = nil;
	assert(inBase != nil);
	VStream* serverReply = GetInputStream();
	sLONG typsel = 0;
	outErr = serverReply->GetLong(typsel);
	if (outErr == VE_OK)
	{
		switch(typsel)
		{
		case sel_bitsel:
			sel = new BitSel(inBase, inContext);
			break;
		case sel_petitesel:
			sel = new PetiteSel(inBase, inContext);
			break;
		case sel_longsel:
			sel = new LongSel(inBase, inContext);
			((LongSel*)sel)->PutInCache();
			break;
		default:
			outErr = ThrowBaseError(VE_DB4D_INVALID_TYPESEL, noaction);
		}
		if (outErr == VE_OK && sel == nil)
			outErr = ThrowBaseError(memfull, noaction);
	}

	if (outErr == VE_OK)
	{
		outErr = sel->FromServer(serverReply, inContext);
		if (outErr != VE_OK)
		{
			sel->libere();
			sel->Release();
			sel = nil;
		}
	}
	
	if (sel != nil)
		sel->libere();

	return sel;
	*/
	return inBase->BuildSelectionFromServer(GetInputStream(), inContext, nil,  outErr);
}


Bittab* IRequest::RetainSetReply(Base4D* inBase, Table* inTable, VError& outErr, CDB4DBaseContextPtr inContext)
{
	/*
	Bittab* set = nil;
	assert(inBase != nil);
	VStream* serverReply = GetInputStream();
	sLONG typsel = 0;
	outErr = serverReply->GetLong(typsel);
	if (outErr == VE_OK)
	{
		switch(typsel)
		{
		case sel_bitsel:
			set = new Bittab;
			break;
		case sel_nosel:
			set = nil;
			break;
		default:
			outErr = ThrowBaseError(VE_DB4D_INVALID_TYPESEL, noaction);
		}
	}

	if (outErr == VE_OK)
	{
		outErr = set->FromServer(serverReply, inContext);
		if (outErr != VE_OK)
		{
			set->Release();
			set = nil;
		}
	}

	return set;
	*/
	return inBase->BuildSetFromServer(GetInputStream(), inContext, outErr);
}



FicheInMem* IRequest::RetainFicheInMemReply(Table* inTable, VError& outErr, CDB4DBaseContextPtr inContext)
{
	assert(inTable != nil);
	VStream* serverReply = GetInputStream();
	FicheInMem* fic = new FicheInMem();
	if (fic == nil)
		outErr = ThrowBaseError(memfull, noaction);
	else
	{
		BaseTaskInfoPtr context = ConvertContext(inContext);
		outErr = fic->FromServer(serverReply, context, inTable);
		if (outErr != VE_OK)
		{
			fic->Release();
			fic = nil;
		}
	}

	return fic;
}


QueryResult* IRequest::RetainQueryResultReply(Base4D* inBase, Table* inTable, VError& outErr, BaseTaskInfo* inContext)
{
	assert(inTable != nil);
	VStream* serverReply = GetInputStream();
	QueryResult* result = new QueryResult();
	if (result == nil)
		outErr = ThrowBaseError(memfull, noaction);
	else
	{
		outErr = result->FromServer(serverReply, inBase, inTable, inContext);
		if (outErr != VE_OK)
		{
			result->Release();
			result = nil;
		}
	}

	return result;
}



VError IRequest::ReadQueryResultReply(Base4D* inBase, Table* inTable, QueryResult* outResult, BaseTaskInfo* inContext)
{
	VError outErr;
	assert(inTable != nil);
	assert(outResult != nil);
	VStream* serverReply = GetInputStream();
	{
		outErr = outResult->FromServer(serverReply, inBase, inTable, inContext);
	}

	return outErr;
}



VValueSingle* IRequest::BuildValueReply(VError& outErr)
{
	VStream* serverReply = GetInputStream();
	VValueSingle* result = nil;

	sLONG kind;
	outErr = serverReply->GetLong(kind);

	if ((ValueKind)kind != VK_EMPTY)
	{
		result = (VValueSingle*)VValue::NewValueFromValueKind((ValueKind)kind);
		if (result != nil)
		{
			outErr = result->ReadFromStream(serverReply);
			if (outErr != VE_OK)
			{
				delete result;
				result = nil;
			}
		}
		else
			outErr = ThrowBaseError(memfull, noaction);
	}

	return result;
}


VError IRequest::GetCollectionReply(DB4DCollectionManager& outCollection, sLONG& totalRow, sLONG startingRow, sLONG& processedRows)
{
	VStream* serverReply = GetInputStream();
	sLONG nbcol, nbrow;
	VError err = serverReply->GetLong(nbcol);
	if (err == VE_OK)
		err = serverReply->GetLong(nbrow);
	if (err == VE_OK)
		err = serverReply->GetLong(totalRow);

	processedRows = nbrow;

	if (err == VE_OK)
	{
		if (startingRow == 0)
			err = outCollection.SetCollectionSize(totalRow);
	}

	if (err == VE_OK)
	{
		for (sLONG i = 1; i <= nbrow && err == VE_OK; i++)
		{
			for (sLONG j = 1; j <= nbcol && err == VE_OK; j++)
			{
				sLONG kind;
				err = serverReply->GetLong(kind);
				if (err == VE_OK)
				{
					VValueSingle* cv = (VValueSingle*)VValue::NewValueFromValueKind((ValueKind)kind);
					if (cv == nil)
						err = ThrowBaseError(memfull, noaction);
					else
					{
						err = cv->ReadFromStream(serverReply);
						if (err == VE_OK)
							err = outCollection.SetNthElement(i+startingRow, j, *cv);
						delete cv;
					}
				}

			}
		}
	}
	return err;
}



VError IRequest::GetArrayLongReply(sLONG* outArray, sLONG maxlongs)
{
	VStream* serverReply = GetInputStream();
	VError err = VE_OK;

	sLONG nblongs;
	err = serverReply->GetLong(nblongs);
	if (err == VE_OK)
	{
		if (nblongs > maxlongs)
			err = ThrowBaseError(VE_DB4D_ARRAY_TOO_BIG, noaction);
		else
		{
			sLONG nb = nblongs;
			err = serverReply->GetLongs(outArray, &nb);
			if (nb != nblongs)
				err = ThrowBaseError(VE_DB4D_ARRAY_SIZE_DOES_NOT_MATCH, noaction);
		}
	}
	return err;
}


VError IRequest::GetNumFieldArrayReply( NumFieldArray& outNumFieldArray)
{
	VStream *serverReply = GetInputStream();
	sLONG count = 0;
	
	outNumFieldArray.Destroy();
	
	VError err = serverReply->GetLong( count);
	if (err == VE_OK)
	{
		for (sLONG i = 0 ; i < count && err == VE_OK ; ++i)
		{
			sLONG value = 0;
			err = serverReply->GetLong( value);
			if (err == VE_OK)
				outNumFieldArray.Add( value);
		}
	}
	return err;
}


VError IRequest::GetColumnFormulasResultReply(ColumnFormulas* formulas)
{
	return formulas->ReadResultsFromStream(GetInputStream());
}



VError IRequest::GetUpdatedInfo(Base4D* base, BaseTaskInfo *context)
{
	VStream* serverReply = GetInputStream();
	VError err = VE_OK;

	sLONG numtable;
	do 
	{
		err = serverReply->GetLong(numtable);
		if (err == VE_OK && numtable != 0)
		{
			sLONG nbfic;
			err = serverReply->GetLong(nbfic);
			if (err == VE_OK)
			{
				Table* tt = base->RetainTable(numtable);
				if (tt != nil)
				{
					tt->SetRemoteMaxRecordsInTable(nbfic);
					tt->Release();
				}
			}
		}

	} while(numtable != 0 && err == VE_OK);

	if (err == VE_OK)
	{
		sLONG newstamp;
		err = serverReply->GetLong(newstamp);
		if (newstamp != 0)
			base->SetRemoteMaxRecordsStamp(newstamp);
	}

	return err;
}



			// ------------------------------------------------ //




Base4D* IRequestReply::RetainBaseParam( VError &outErr)
{
	VUUID xid;
	Base4D* base = nil;
	outErr = xid.ReadFromStream(GetInputStream());
	if (outErr == VE_OK)
	{
		base = VDBMgr::GetManager()->RetainBase4DByUUID(xid);
		if (base == nil)
			outErr = ThrowBaseError(VE_DB4D_INVALID_BASEID, noaction);
	}
	return base;

}


Table* IRequestReply::RetainTableParam( Base4D* inBase, VError &outErr)
{
	assert(inBase != nil);
	sLONG numtable = 0;
	Table* tt = nil;
	outErr = GetInputStream()->GetLong(numtable);
	if (outErr == VE_OK)
	{
		tt = inBase->RetainTable(numtable);
		if (tt == nil)
			outErr = inBase->ThrowError(VE_DB4D_INVALID_TABLENUM, noaction);
	}
	return tt;
}


Field* IRequestReply::RetainFieldParam( Table *inTable, VError &outErr)
{
	assert(inTable != NULL);

	sLONG numField = 0;
	Field *field = NULL;

	outErr = GetInputStream()->GetLong( numField);
	if (outErr == VE_OK)
	{
		if (numField == 0)
			field = inTable->RetainPseudoField(DB4D_Pseudo_Field_RecordNum);
		else
			field = inTable->RetainField( numField);
		if (field == NULL)
			outErr = inTable->ThrowError( VE_DB4D_INVALID_FIELDNUM, noaction);
	}
	return field;
}


Relation* IRequestReply::RetainRelationParam( Base4D *inBase, VError &outErr)
{
	assert(inBase != NULL);

	sLONG relationIndex = 0;
	Relation *rel = NULL;
/*
	outErr = GetInputStream()->GetLong( relationIndex);
	if (outErr == VE_OK)
	{
		rel = inBase->RetainRelation( relationIndex);
		if (rel == NULL)
			outErr = inBase->ThrowError( VE_DB4D_INVALID_RELATIONNUM, noaction);
		
	}
*/

	VUUID xid;
	outErr = xid.ReadFromStream(GetInputStream());
	if (outErr == VE_OK)
	{
		rel = inBase->FindAndRetainRelationByUUID(xid);
		if (rel == nil)
			outErr = inBase->ThrowError( VE_DB4D_INVALID_RELATIONNUM, noaction);
	}

	return rel;
}


VError IRequestReply::ReadSortTabParam(SortTab& tabs)
{
	VStream* clientreq = GetInputStream();
	return tabs.GetFrom(clientreq);
}


DB4DCollectionManager* IRequestReply::RetainCollectionParam(Base4D* inBase, VError &outErr)
{
	UtilVValuesCollection* collection = new UtilVValuesCollection;
	VStream* clientreq = GetInputStream();
	VError err;
	sLONG nbcol, nbrow;

	err = clientreq->GetLong(nbcol);
	if (err == VE_OK)
		err = clientreq->GetLong(nbrow);

	if (err == VE_OK)
		err = collection->_ResizeColumns(nbcol);

	if (err == VE_OK)
	{
		for (sLONG i = 0; i < nbcol && err == VE_OK; i++)
		{
			CDB4DField* ff = nil;
			uBYTE cc;
			err = clientreq->GetByte(cc);
			if (cc == '.')
			{
				ff = nil;
			}
			else
			{
				if (cc == '*')
				{
					sLONG tablenum;
					sLONG fieldnum;
					err = clientreq->GetLong(tablenum);
					err = clientreq->GetLong(fieldnum);
					Table* tt = inBase->RetainTable(tablenum);
					if (tt != nil)
					{
						Field* cri = tt->RetainPseudoField(DB4D_Pseudo_Field_RecordNum);
						ff = new VDB4DField(cri);
						cri->Release();
						tt->Release();
					}

				}
				else if (cc == '+')
				{
					VUUID xid;
					err = xid.ReadFromStream(clientreq);
					Field* cri = inBase->FindAndRetainFieldRef(xid);
					if (cri != nil)
					{
						ff = new VDB4DField(cri);
						cri->Release();
					}
				}
				else
					err = ThrowBaseError(VE_DB4D_INVALID_FIELDNUM, noaction);
			}
			collection->_SetOneCol(i+1, ff);
			if (ff != nil)
				ff->Release();
		}
	}

	if (err == VE_OK)
	{
		err = collection->SetCollectionSize(nbrow);
	}

	if (err == VE_OK)
	{
		for (sLONG i = 1; i <= nbrow && err == VE_OK; i++)
		{
			for (sLONG j = 1; j <= nbcol && err == VE_OK; j++)
			{
				sLONG kind;
				err = clientreq->GetLong(kind);
				if (err == VE_OK)
				{
					VValueSingle* cv = (VValueSingle*)VValue::NewValueFromValueKind((ValueKind)kind);
					if (cv == nil)
						err = ThrowBaseError(memfull, noaction);
					else
					{
						err = cv->ReadFromStream(clientreq);
						collection->_SetNthElement(i, j, cv);
					}
				}

			}
		}
	}


	outErr = err;
	return collection;
}


FicheInMem* IRequestReply::RetainFicheInMemParam( Base4D* inBase, Table* inTable, BaseTaskInfo* inContext, VError &outErr)
{
	VStream* clientreq = GetInputStream();
	return inBase->BuildRecordFromClient(clientreq, inContext, inTable, outErr);
}


FicheInMem* IRequestReply::RetainFicheInMemMinimalParam( Base4D* inBase, Table* inTable, BaseTaskInfo* inContext, VError &outErr)
{
	VStream* clientreq = GetInputStream();
	return inBase->BuildRecordMinimalFromClient(clientreq, inContext, inTable, outErr);
}


IndexInfo* IRequestReply::RetainIndexParam( Base4D *inBase, BaseTaskInfo *inContext, VError& outErr, Boolean mustBeValid)
{
	assert(inBase != NULL);

	IndexInfo *index = NULL;
	VUUID id;
	outErr = id.ReadFromStream( GetInputStream());
	if (outErr == VE_OK)
	{
		index = inBase->FindAndRetainIndexByUUID( id, mustBeValid, inContext);
		if (index == nil)
			outErr = inBase->ThrowError( VE_DB4D_INVALIDINDEX, noaction);
	}
	return index;
}


VValueSingle* IRequestReply::BuildValueParam(VError& outErr)
{
	VStream* clientreq = GetInputStream();
	VValueSingle* result = nil;

	sLONG kind;
	outErr = clientreq->GetLong(kind);

	if ((ValueKind)kind != VK_EMPTY)
	{
		result = (VValueSingle*)VValue::NewValueFromValueKind((ValueKind)kind);
		if (result != nil)
		{
			outErr = result->ReadFromStream(clientreq);
			if (outErr != VE_OK)
			{
				delete result;
				result = nil;
			}
		}
		else
			outErr = ThrowBaseError(memfull, noaction);
	}

	return result;
}


Selection* IRequestReply::RetainSelectionParam( Base4D* inBase, Table* inTable)
{
	/*
	Selection* sel = nil;
	VStream* clientreq = GetInputStream();
	sLONG typsel;
	VError err = clientreq->GetLong(typsel);
	switch(typsel)
	{
		case sel_petitesel:
			sel = new PetiteSel(inBase, (CDB4DBaseContext*)-1);
			if (sel == nil)
				err = ThrowBaseError(memfull, noaction);
			else
			{
				err = sel->FromClient(clientreq, nil); 
				if (err != VE_OK)
				{
					sel->Release();
					sel = nil;
				}
			}
			break;

		case sel_longsel:
		case sel_bitsel:
			{
				VUUID xid;
				err = xid.ReadFromStream(clientreq);
				if (err == VE_OK)
				{
					sel = VDBMgr::GetManager()->RetainServerKeptSelection(xid.GetBuffer());
					if (sel == nil)
						err = ThrowBaseError(VE_DB4D_INVALID_SELECTION_ID, noaction);
				}
			}
			break;

		default:
			err = ThrowBaseError(VE_DB4D_INVALID_TYPESEL, noaction);


	}

	return sel;
	*/

	VError err = VE_OK;
	return inBase->BuildSelectionFromClient(GetInputStream(), nil, inTable, err);
}


Bittab* IRequestReply::RetainSetParam(Base4D* inBase, Table *inTable)
{
	VError err = VE_OK;
	return inBase->BuildSetFromClient(GetInputStream(), nil, err);
}


QueryOptions* IRequestReply::RetainQueryOptionsParam( Base4D* inBase, Table* inTable, BaseTaskInfo* inContext, VError &outErr)
{
	VStream* clientreq = GetInputStream();
	QueryOptions* result = new QueryOptions();
	if (result == nil)
		outErr = ThrowBaseError(memfull, noaction);
	else
	{
		outErr = result->FromClient(clientreq, inBase, inTable, inContext);
		if (outErr != VE_OK)
		{
			result->Release();
			result = nil;
		}
	}

	return result;
}


SearchTab* IRequestReply::RetainQueryParam( Table* inTable, VError &outErr)
{
	VStream* clientreq = GetInputStream();
	SearchTab* query = new SearchTab(inTable);
	if (query == nil)
		outErr = ThrowBaseError(memfull, noaction);
	else
	{
		outErr = query->GetFrom(*clientreq);
		if (outErr != VE_OK)
		{
			delete query;
			query = nil;
		}
	}
	return query;
}


ColumnFormulas* IRequestReply::GetColumnFormulasParam(Base4D* inBase, VError &outErr)
{
	ColumnFormulas* cols = new ColumnFormulas();

	if (cols == nil)
		outErr = ThrowBaseError(memfull);
	else
		outErr = cols->ReadFromStream(GetInputStream(), inBase);

	if (outErr != VE_OK)
	{
		if (cols != nil)
			delete cols;
		cols = nil;
	}
	return cols;
}


VDB4DProgressIndicator* IRequestReply::RetainProgressParam(CDB4DBaseContext* inContext)
{
	VDB4DProgressIndicator* result = nil;
	VTask* curtask = VTask::GetCurrent();
	const VValueBag *extradata = VImpCreator<BaseTaskInfo>::GetImpObject(inContext)->RetainExtraData();
	if (extradata != nil)
	{
		VUUID clientUID;
		extradata->GetVUUID(DB4DBagKeys::client_uid, clientUID);
		extradata->Release();
		if(clientUID!=VUUID::sNullUUID)
		{
			result = VProgressManager::CreateRemoteProgress(*GetInputStream(),clientUID);
		}
	}
	
	/* 
	const XBOX::VValueBag* extra=curtask->RetainProperties();
	if(extra)
	{
		VUUID clientUID;
		extra->GetVUUID(DB4DBagKeys::client_uid, clientUID);
		extra->Release();
		if(clientUID!=VUUID::sNullUUID)
		{
			result = VProgressManager::CreateRemoteProgress(*GetInputStream(),clientUID);
		}
		
	}
	*/
	return result;
}


VError IRequestReply::GetNumFieldArrayParam( NumFieldArray& outNumFieldArray)
{
	VStream *clientreq = GetInputStream();
	sLONG count = 0;
	
	outNumFieldArray.Destroy();
	
	VError err = clientreq->GetLong( count);
	if (err == VE_OK)
	{
		for (sLONG i = 0 ; i < count && err == VE_OK ; ++i)
		{
			sLONG value = 0;
			err = clientreq->GetLong( value);
			if (err == VE_OK)
				outNumFieldArray.Add( value);
		}
	}
	return err;
}


VError IRequestReply::PutSelectionReply(Selection* inSel, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;

	VStream* reply = GetOutputStream();

	if (inSel == nil)
		err = reply->PutLong(sel_nosel);
	else
	{
		err = inSel->ToClient(reply, inContext);

		if (err == VE_OK)
		{
			VDBMgr::GetManager()->KeepSelectionOnServer(inSel);
		}
	}

	return err;
}


VError IRequestReply::PutSetReply(Bittab* inSet, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;

	VStream* reply = GetOutputStream();

	if (inSet == nil)
		err = reply->PutLong(sel_nosel);
	else
	{
		err = reply->PutLong(sel_bitsel);
		err = inSet->ToClient(reply, inContext);
	}

	return err;
}



VError IRequestReply::PutFicheInMemReply(FicheInMem* inFiche, CDB4DBaseContext *inContext)
{
	VError err = VE_OK;

	VStream* reply = GetOutputStream();
	assert(inFiche != nil);
	BaseTaskInfoPtr context = ConvertContext(inContext);
	err = inFiche->ToClient(reply, context);

#if 0
	// deja appele dans inFiche->ToClient
	if (err == VE_OK)
	{
		VDBMgr::GetManager()->KeepRecordOnServer( inFiche);
	}
#endif

	return err;
}


VError IRequestReply::PutQueryResultReply(QueryResult* inResult, BaseTaskInfo *inContext)
{
	VStream* reply = GetOutputStream();
	assert(inResult != nil);

	return inResult->ToClient(reply, inContext);
}


VError IRequestReply::PutValueReply(const VValueSingle* inVal)
{
	VError err;
	VStream* reply = GetOutputStream();
	if (inVal == nil)
		err = reply->PutLong(VK_EMPTY);
	else
	{
		err = reply->PutLong((sLONG)inVal->GetValueKind());
		if (err == VE_OK)
			err = inVal->WriteToStream(reply);
	}
	return err;
}


VError IRequestReply::PutNumFieldArrayReply( const NumFieldArray& inNumFieldArray)
{
	VStream *reply = GetOutputStream();
	VError err = reply->PutLong( inNumFieldArray.GetCount());
	if (err == VE_OK)
	{
		for (NumFieldArray::ConstIterator iter = inNumFieldArray.First() , end = inNumFieldArray.End() ; iter != end && err == VE_OK ; ++iter)
			err = reply->PutLong( *iter);
	}
	return err;
}


VError IRequestReply::PutCollectionReply(DB4DCollectionManager& inCollection, bool fullyCompleted, sLONG maxelems)
{
	VStream* reply = GetOutputStream();
	sLONG nbcol = inCollection.GetNumberOfColumns();
	sLONG nbrow = inCollection.GetCollectionSize();
	sLONG totalrow = nbrow;
	if (!fullyCompleted)
		nbrow = maxelems;
	VError err = reply->PutLong(nbcol);
	if (err == VE_OK)
		err = reply->PutLong(nbrow);
	if (err == VE_OK)
		err = reply->PutLong(totalrow);

	if (err == VE_OK)
	{
		for (sLONG i = 1; i <= nbrow && err == VE_OK; i++)
		{
			for (sLONG j = 1; j <= nbcol && err == VE_OK; j++)
			{
				ConstValPtr from;
				bool DisposeIt;
				err = inCollection.GetNthElement(i, j, from, &DisposeIt);
				if (err == VE_OK)
				{
					if (from == nil)
					{
						sLONG typ;
						CDB4DField* xcri = inCollection.GetColumnRef(j);
						if (xcri == nil)
							typ = VK_LONG;
						else
							typ = xcri->GetType();
						from = (VValueSingle*)VValue::NewValueFromValueKind(typ);
						DisposeIt = true;
					}
					err = reply->PutLong((sLONG)from->GetValueKind());
					if (err == VE_OK)
						err = from->WriteToStream(reply);
					if (DisposeIt)
						delete from;
				}
				else
					reply->SetError(err);

			}
		}
	}

	return err;
}


VError IRequestReply::PutArrayLongReply(sLONG* inArray, sLONG nblongs)
{
	VError err = VE_OK;

	VStream* reply = GetOutputStream();
	err = reply->PutLong(nblongs);
	if (err == VE_OK)
		err = reply->PutLongs(inArray, nblongs);

	return err;
}


VError IRequestReply::PutColumnFormulasResultReply(ColumnFormulas* formulas)
{
	return formulas->WriteResultsToStream(GetOutputStream());
}


VError IRequestReply::PutUpdatedInfo(Base4D* base, BaseTaskInfo *context)
{
	VError err = VE_OK;
	sLONG newstamp = 0;
	VStream* outstream = GetOutputStream();
	if (context != nil)
	{
		sLONG stamp = context->GetRemoteMaxrecordStamp();
		err = base->PutMaxRecordRemoteInfo(stamp, outstream, newstamp);
	}
	if (err == VE_OK)
		err = outstream->PutLong(0);
	if (err == VE_OK)
		err = outstream->PutLong(newstamp);
	return err;
}


VError IRequestReply::PutUpdatedInfo(Table* inTable, BaseTaskInfo *context)
{
	VError err = VE_OK;
	VStream* outstream = GetOutputStream();
	err = inTable->GetDF()->PutMaxRecordRemoteInfo(0, outstream);
	if (err == VE_OK)
		err = outstream->PutLong(0);
	if (err == VE_OK)
		err = outstream->PutLong(0);
	return err;
}


VError IRequestReply::PutUpdatedInfo(Table* inTable1, Table* inTable2, BaseTaskInfo *context)
{
	VError err = VE_OK;
	VStream* outstream = GetOutputStream();
	err = inTable1->GetDF()->PutMaxRecordRemoteInfo(0, outstream);
	if (err == VE_OK)
		err = inTable2->GetDF()->PutMaxRecordRemoteInfo(0, outstream);
	if (err == VE_OK)
		err = outstream->PutLong(0);
	if (err == VE_OK)
		err = outstream->PutLong(0);
	return err;
}



VError _GetThingsToForget( VStream* clientreq, VDBMgr *inMgr, BaseTaskInfo* context)
{
	char code;
	Boolean cont;
	VError err;
	sLONG stampmaxrecord;
	err = clientreq->GetLong(stampmaxrecord);
	if (context != nil)
		context->SetRemoteMaxrecordStamp(stampmaxrecord);

	VLogThingsToForget		ttfToLog(inMgr, context);// ttf == "Things To Forget"

	do
	{
		cont = false;
		err = clientreq->Get( &code);
		if (err == VE_OK)
		{
			if (code == 's')
			{
				cont = true;
				vector<VUUIDBuffer> ids;
				sLONG count;
				err = clientreq->Get( &count);
				if (err == VE_OK)
				{
					try
					{
						ids.resize( count);
						count *= sizeof( VUUIDBuffer);
						err = clientreq->GetBytes( &ids.front(), &count);
					}
					catch(...)
					{
						err = ThrowBaseError(memfull, noaction);
					}
				}

				if (err == VE_OK)
				{
					inMgr->ForgetServerKeptSelections( ids);
				}

				ttfToLog.Append( (UniChar) 's');

			}

			if (code == 'r')
			{
				cont = true;
				vector<DB4D_RecordRef> ids;
				sLONG count;
				err = clientreq->Get( &count);
				if (err == VE_OK)
				{
					try
					{
						ids.resize( count);
						count *= sizeof( DB4D_RecordRef);
						err = clientreq->GetBytes( &ids.front(), &count);
					}
					catch(...)
					{
						err = ThrowBaseError(memfull, noaction);
					}
				}

				if (err == VE_OK)
				{
					inMgr->ForgetServerKeptRecords( ids);
				}

				ttfToLog.Append( (UniChar) 'r');
			}

			if (code == 't')
			{
				cont = true;
				vector<uBYTE> remoteTrans;
				sLONG nb;
				err = clientreq->GetLong(nb);
				if (err== VE_OK && nb > 0)
				{
					remoteTrans.resize(nb, 0);
					err = clientreq->GetBytes(&remoteTrans[0], &nb);
					if (err == VE_OK && context != nil)
					{
						for (vector<uBYTE>::iterator cur = remoteTrans.begin(), end = remoteTrans.end(); cur != end; cur++)
						{
							VError err2;
							switch(*cur)
							{
							case remote_starttrans:
								context->StartTransaction(err2);
								break;
							case remote_rollbacktrans:
								context->RollBackTransaction();
								break;
							case remote_committrans:
								context->CommitTransaction();
								break;
							default:
								break;
							}
						}
					}

					ttfToLog.AppendTransactionInfos(remoteTrans);
				}
			}

			if (code == 'a')
			{
				cont = true;
				if (testAssert( context != nil))
					err = context->GetAutoRelInfoFromStream( clientreq);

				ttfToLog.Append( (UniChar) 'a');
			}
			
			if (code == 'i')
			{
				// updating context extra data
				cont = true;
				if (testAssert( context != nil))
				{
					VValueBag *extraData = nil;
					uBYTE hasExtra = 0;
					err = clientreq->GetByte( hasExtra);
					if (hasExtra)
					{
						extraData = new VValueBag;
						if (extraData != nil)
							err = extraData->ReadFromStream( clientreq);
						else
							err = memfull;
					}
					if (err == VE_OK)
						context->getContextOwner()->SetExtraData( extraData);
					ReleaseRefCountable( &extraData);
				}
			
				ttfToLog.Append( (UniChar) 'i');
			}

			if (code == 'p')
			{
				if (testAssert(context != nil))
				{
					Base4D* base = context->GetBase();
					cont = true;
					sLONG count;
					err = clientreq->Get( &count);
					for (sLONG i = 0; i< count && err == VE_OK; i++)
					{
						sLONG numtable,numrec;
						uBYTE markforpush;
						err = clientreq->GetLong(numtable);
						if (err == VE_OK)
							err = clientreq->GetLong(numrec);
						if (err == VE_OK)
							err = clientreq->GetByte(markforpush);

						ttfToLog.AppendPushedRecInfos(numtable, numrec, markforpush);

						Table* tt = base->RetainTable(numtable);
						if (tt != nil)
						{
							if (markforpush)
							{
								tt->MarkRecordAsPushed(numrec);
								context->MarkRecordAsPushed(tt->GetDF(), numrec);
							}
							else
							{
								tt->UnMarkRecordAsPushed(numrec);
								context->UnMarkRecordAsPushed(tt->GetDF(), numrec);
							}
							tt->Release();
						}
					}
				}
			}
		}
	} while (cont && err == VE_OK);
	return err;
}

VError IRequestReply::GetThingsToForget( VDBMgr *inMgr, BaseTaskInfo* context)
{
	VStream* clientreq = GetInputStream();
	return _GetThingsToForget(clientreq, inMgr, context);
}


			// ------------------------------------------------ //


VError PutVCompareOptionsIntoStream(const VCompareOptions& options, VStream& into)
{
	VError err = into.PutByte(options.IsDiacritical() ? 1 : 0);
	if (err == VE_OK)
		err = into.PutByte(options.IsLike() ? 1 : 0);
	if (err == VE_OK)
		err = into.PutByte(options.IsBeginsWith() ? 1 : 0);
	if (err == VE_OK)
		err = into.PutReal(options.GetEpsilon());
	return err;
}


VError GetVCompareOptionsFromStream(VCompareOptions& options, VStream& from)
{
	uBYTE cc;
	VError err;

	err = from.GetByte(cc);
	options.SetDiacritical(cc == 1);

	if (err == VE_OK)
		err = from.GetByte(cc);
	options.SetLike(cc == 1);

	if (err == VE_OK)
		err = from.GetByte(cc);
	options.SetBeginsWith(cc == 1);

	if (err == VE_OK)
	{
		Real rr;
		err = from.GetReal(rr);
		if (err == VE_OK)
			options.SetEpsilon(rr);
	}

	return err;
}



// ------------------------------------------------------------------------------

#if debugOverlaps_strong

map<RecordIDInTrans, DataAddr4D> debug_MapBlobAddrInTrans;


Boolean debug_CheckBlobAddrInTrans(const RecordIDInTrans& blobID, DataAddr4D addr)
{
	map<RecordIDInTrans, DataAddr4D>::iterator found = debug_MapBlobAddrInTrans.find(blobID);
	if (found != debug_MapBlobAddrInTrans.end())
	{
		DataAddr4D addrDansTans = found->second;
		if (addrDansTans != addr && addrDansTans != 0)
		{
			addr = addr; // break here
			assert(addrDansTans == addr);
			return true;
		}
	}
	return false;
}


void debug_AddBlobAddrInTrans(const RecordIDInTrans& blobID, DataAddr4D addr)
{
	debug_MapBlobAddrInTrans[blobID] = addr;
}


void debug_DelBlobAddrInTrans(const RecordIDInTrans& blobID)
{
	debug_MapBlobAddrInTrans.erase(blobID);
}

#endif

#if debugLeaksAll

uBOOL debug_candumpleaksAll = 0;
uBOOL debug_canRegisterLeaksAll = 0;


IDebugRefCountable::mapOfRefCountables IDebugRefCountable::objs;
VCriticalSection IDebugRefCountable::objsMutex;


IDebugRefCountable::IDebugRefCountable()
{
	if (debug_candumpleaksAll)
		DumpStackCrawls();
	if (debug_canRegisterLeaksAll && OKToTrackDebug())
		RegisterStackCrawl(this);
}

IDebugRefCountable::~IDebugRefCountable()
{
	if (debug_candumpleaksAll)
		DumpStackCrawls();
	UnRegisterStackCrawl(this);
}

sLONG IDebugRefCountable::Retain(const char* DebugInfo) const
{
	if (debug_candumpleaksAll)
		DumpStackCrawls();
	if (debug_canRegisterLeaksAll && OKToTrackDebug())
		RegisterStackCrawl(this);
	return IRefCountable::Retain(DebugInfo);
}

sLONG IDebugRefCountable::Release(const char* DebugInfo) const
{
	if (debug_candumpleaksAll)
		DumpStackCrawls();
	if (debug_canRegisterLeaksAll && OKToTrackDebug())
		RegisterStackCrawl(this);
	return IRefCountable::Release(DebugInfo);
}



void IDebugRefCountable::DumpStackCrawls()
{
	VFile ff("f:\\dump.txt");
	if (ff.Exists())
		ff.Delete();

	{
		VFileStream stream(&ff, FO_CreateIfNotFound);
		stream.OpenWriting();
		DumpStackCrawls(stream);
		stream.CloseWriting();
	}
	sLONG xdebug = 1;

	/*
	VString s;
	DumpStackCrawls(s);
	DebugMsg(s);
	*/
	/*
	VTaskLock lock(&objsMutex);
	for (mapOfRefCountables::iterator cur = objs.begin(),end = objs.end(); cur != end; ++cur)
	{
		VString title;
		cur->first->GetDebugInfo(title);
		DebugMsg(title+"\n\n");
		for (StackCrawlCollection::iterator curs = cur->second.begin(), ends = cur->second.end(); curs != ends; ++curs)
		{
			VStr<2048> ss;
			curs->Dump(ss);
			DebugMsg(ss);
			DebugMsg(L"\n\n\n\n");
		}
		DebugMsg(L"\n\n\n\n -------------------------------------------------------- \n\n\n\n");
	}
	*/
}


void IDebugRefCountable::DumpStackCrawls(VStream& outStream)
{
	VTaskLock lock(&objsMutex);
	for (mapOfRefCountables::iterator cur = objs.begin(),end = objs.end(); cur != end; ++cur)
	{
		VString title;
		cur->first->GetDebugInfo(title);
		outStream.PutText(title+"\n\n");
		for (StackCrawlCollection::iterator curs = cur->second.begin(), ends = cur->second.end(); curs != ends; ++curs)
		{
			VStr<2048> ss;
			curs->Dump(ss);
			outStream.PutText(ss + "\n\n\n\n");
		}
		outStream.PutText("\n\n\n\n -------------------------------------------------------- \n\n\n\n");
	}
	
}


void IDebugRefCountable::DumpStackCrawls(VString& outText)
{
	VTaskLock lock(&objsMutex);
	for (mapOfRefCountables::iterator cur = objs.begin(),end = objs.end(); cur != end; ++cur)
	{
		VString title;
		cur->first->GetDebugInfo(title);
		outText += title+"\n\n";
		for (StackCrawlCollection::iterator curs = cur->second.begin(), ends = cur->second.end(); curs != ends; ++curs)
		{
			VStr<2048> ss;
			curs->Dump(ss);
			outText += ss;
			outText += "\n\n\n\n";
		}
		outText += "\n\n\n\n -------------------------------------------------------- \n\n\n\n";
	}
}


void IDebugRefCountable::RegisterStackCrawl(const IDebugRefCountable* obj)
{
	VTaskLock lock(&objsMutex);
	VStackCrawl crawl;
	crawl.LoadFrames(0, kMaxScrawlFrames);
	objs[obj].insert(crawl);
	/*
	StackCrawlCollection* stacks = &(objs[obj]);
	stacks->insert(crawl);
	*/

}


void IDebugRefCountable::UnRegisterStackCrawl(const IDebugRefCountable* obj)
{
	VTaskLock lock(&objsMutex);
	objs.erase(obj);
}





#else

//typedef IRefCountable IDebugRefCountable;

#endif



#if debugLeakCheck_Strong

debug_mapOfStacks debug_Stacks;
VCriticalSection debug_StacksMutex;


uBOOL debug_candumpleaks = 0;
uBOOL debug_canRegisterLeaks = 0;


void RegisterStackCrawl(const VObject* obj)
{
	VTaskLock lock(&debug_StacksMutex);
	VStackCrawl crawl;
	crawl.LoadFrames(0, kMaxScrawlFrames);
	debug_Stacks[obj] = crawl;
	
}


void UnRegisterStackCrawl(const VObject* obj)
{
	VTaskLock lock(&debug_StacksMutex);
	debug_Stacks.erase(obj);
}


void DumpStackCrawls()
{
	VStr<2048> ss;
	VTaskLock lock(&debug_StacksMutex);
	for (debug_mapOfStacks::iterator cur = debug_Stacks.begin(), end = debug_Stacks.end(); cur != end; cur++)
	{
		ss.Clear();
		cur->second.Dump(ss);
		DebugMsg(ss);
		DebugMsg(L"\n\n\n\n");
	}
}



void xDumpMemoryLeaks(VString& outText)
{
	VTaskLock lock(&debug_StacksMutex);
	for (debug_mapOfStacks::iterator cur = debug_Stacks.begin(), end = debug_Stacks.end(); cur != end; cur++)
	{
		VString ss;
		cur->second.Dump(ss);
		outText += ss;
		outText += L"\n\n\n\n";
	}
}


debug_RefmapOfStack debug_RefStacks;
debug_mapOfThread debug_Threads;


void RegisterRefStackCrawl(const VObject* obj)
{
	VTaskLock lock(&debug_StacksMutex);
	VTaskID threadid  = VTask::GetCurrentID();
	sLONG refid;
	debug_mapOfThread::iterator found = debug_Threads.find(threadid);
	if (found == debug_Threads.end())
	{
		refid = 1;
		debug_Threads.insert(make_pair(threadid,1));
	}
	else
	{
		found->second++;
		refid = found->second;
	}
	sLONG fullrefid = (threadid * 65536) + refid;

	debug_VObjRef ID = make_pair(obj, fullrefid);

	VStackCrawl crawl;
	crawl.LoadFrames(0, kMaxScrawlFrames);
	debug_RefStacks[ID] = crawl;

}


void UnRegisterRefStackCrawl(const VObject* obj)
{
	VTaskLock lock(&debug_StacksMutex);

	VTaskID threadid  = VTask::GetCurrentID();
	sLONG refid = 0;
	debug_mapOfThread::iterator found = debug_Threads.find(threadid);

	if (found == debug_Threads.end())
	{
		// attention
		refid = refid; // put a break;
	}
	else
	{
		refid = found->second;
		found->second--;
		if (found->second == 0)
			debug_Threads.erase(found);

		sLONG fullrefid = (threadid * 65536) + refid;
		debug_VObjRef ID = make_pair(obj, fullrefid);
		debug_RefStacks.erase(ID);
	}

}


void DumpRefStackCrawls()
{
	VStr<2048> ss;
	VTaskLock lock(&debug_StacksMutex);
	for (debug_RefmapOfStack::iterator cur = debug_RefStacks.begin(), end = debug_RefStacks.end(); cur != end; cur++)
	{
		ss.Clear();
		cur->second.Dump(ss);
		DebugMsg(ss);
		DebugMsg(L"\n\n\n\n");
	}
}




#endif


ReadWriteSemaphore::ReadWriteSemaphore()
{
	fWriteCount = 0;
	fReadCount = 0;
	fWriteOwner = 0;
	fNextToken = 0;
	fCurrentToken = 0;
	fFreeMemEnCours = 0;
//	fCumulReadEnWrite = 0;
}


#if VERSIONDEBUG
ReadWriteSemaphore::~ReadWriteSemaphore()
{
	assert(fPendingActions.empty());
	assert(fWriteOwner == NULL_TASK_ID);
	assert(fReadCount == 0);
	assert(fWriteCount == 0);
}
#endif


sLONG ReadWriteSemaphore::GetCurrentTaskReadOwnerCount()
{
	VTaskLock lock(&fMutex);
	
	return fReadOwnerCount[VTask::GetCurrentID()];
}

#define quickfixmem 0

bool ReadWriteSemaphore::TryToLock(ReadWriteSemaphore_Access inAccessRights)
{
#if quickfixmem
	if (inAccessRights == RWS_ReadOnly)
		inAccessRights = RWS_WriteExclusive;
#endif

#if debugrws2
	fDebugRWS.AddMessage(L"Avant TryToLock", dbgRWS(inAccessRights, fReadCount, fWriteCount, fWriteOwner, fFreeMemEnCours));
#endif

	bool result;
	VTaskID currentTaskID = VTask::GetCurrentID();

	{
		VTaskLock lock(&fMutex);

		if (inAccessRights == RWS_ReadOnly && fWriteOwner == currentTaskID)
			inAccessRights = RWS_WriteExclusive;

		if (inAccessRights == RWS_ReadOnly)
		{
			sLONG* pReadOwnerCount = &(fReadOwnerCount[currentTaskID]);
			assert((*pReadOwnerCount) >= 0 && (*pReadOwnerCount) <= 32000);

			if ((fPendingActions.empty() || ((*pReadOwnerCount) > 0)) && (fWriteOwner == NULL_TASK_ID || fFreeMemEnCours > 0))
			{
				(*pReadOwnerCount)++;
				fReadCount++;
#if debugrws2
				fDebugRWS.AddMessage(L"TryToLock inc Read", dbgRWS(inAccessRights, fReadCount, fWriteCount, fWriteOwner, fFreeMemEnCours));
#endif
				result = true;
			}
			else
			{
				result = false;
			}
		}
		else
		{
			sLONG* pReadOwnerCount = &(fReadOwnerCount[currentTaskID]);
			assert((*pReadOwnerCount) >= 0 && (*pReadOwnerCount) <= 32000);

			if ((fReadCount - (*pReadOwnerCount) == 0 || inAccessRights == RWS_FreeMemAccess) && (fWriteOwner == NULL_TASK_ID || fWriteOwner == currentTaskID))
			{
				if (inAccessRights == RWS_FreeMemAccess)
					fFreeMemEnCours++;
				fWriteOwner = currentTaskID;
				fWriteCount++;
#if debugrws2
				fDebugRWS.AddMessage(L"TryToLock inc Write", dbgRWS(inAccessRights, fReadCount, fWriteCount, fWriteOwner, fFreeMemEnCours));
#endif
				result	 = true;
			}
			else
			{
				result = false;
			}
		}

#if debugrws2
		if (result)
		{
			fDebugRWS.AddMessage(L"TryToLock OK", dbgRWS(inAccessRights, fReadCount, fWriteCount, fWriteOwner, fFreeMemEnCours));
		}
#endif
	}

	return result;
}


void ReadWriteSemaphore::Lock(ReadWriteSemaphore_Access inAccessRights)
{
#if quickfixmem
	if (inAccessRights == RWS_ReadOnly)
		inAccessRights = RWS_WriteExclusive;
#endif
#if debugrws2
	//fDebugRWS.AddMessage(L"Avant Lock", dbgRWS(inAccessRights, fReadCount, fWriteCount, fWriteOwner, fFreeMemEnCours));
#endif
	VTaskID currentTaskID = VTask::GetCurrentID();
	uLONG token = -1;
	dVSyncEvent* event;
	bool OneMoreTime;
	bool myTurnNow;
	bool mustyield;
	bool mustgettoken = true;
	bool dejaIncRead = false;
	do
	{
		event = nil;
		OneMoreTime = false;
		myTurnNow = false;
		mustyield = false;

		sLONG reportReadWaiting = 0;

		{
			fMutex.Lock();

			if (!mustgettoken && !fPendingActions.empty())
			{
				ReadWriteSemaphore_PendingAction* xaction = &(fPendingActions[0]);
#if debugrws2
				fDebugRWS.AddMessage(L"Got Pending Action", dbgRWS(inAccessRights, fReadCount, fWriteCount, fWriteOwner, fFreeMemEnCours, nil, token, xaction));
#endif
				if (xaction->fToken == token)
				{
#if debugrws2
					if (inAccessRights != xaction->fWhatAccess)
					{
						sLONG i = 0; // break here
						assert(false);
					}
#endif
					myTurnNow = true; // comme on attendait son tour et qu'il vient d'arriver
					if (xaction->fWhatAccess == RWS_ReadOnly)
					{
						xaction->fReadWaiting--;
					}
					else if (xaction->fWhatAccess == RWS_WriteExclusive)
					{
						if (xaction->fReadWaiting != 0)
						{
							sLONG* pReadOwnerCount = &(fReadOwnerCount[currentTaskID]);
#if debugrws2
							if (fFreeMemEnCours == 0)
							{
								if (fWriteOwner != 0 && fWriteOwner != currentTaskID)
								{
									sLONG i = 0; // break here
									assert(false);
								}
								if (*pReadOwnerCount != 0)
								{
									sLONG i = 0; // break here;
									assert(false);
								}
							}
#endif
							assert((*pReadOwnerCount) >= 0 && (*pReadOwnerCount) <= 32000);
							reportReadWaiting = xaction->fReadWaiting;
							*pReadOwnerCount = xaction->fReadWaiting;
#if debugrws2
							if (fReadCount != 0)
							{
								sLONG i = 0; // break here;
								assert(false);
							}
#endif
							fReadCount = xaction->fReadWaiting;
							xaction->fReadWaiting = 0;
							
						}
					}
					
					if (xaction->fReadWaiting == 0)
					{
#if debugrws2
						fDebugRWS.AddMessage(L"Relache Pending Action", dbgRWS(inAccessRights, fReadCount, fWriteCount, fWriteOwner, fFreeMemEnCours, xaction->fWaitingEvent, token, xaction));
#endif
						xaction->fWaitingEvent->Release();
						fPendingActions.erase(fPendingActions.begin());
					}
					mustgettoken = true;
					token = -2;
				}
				else
				{
					token = token; // break here
#if debugrws2
					for (RWS_PendingActionVector::iterator cur = fPendingActions.begin(), end = fPendingActions.end(); cur != end; cur++)
					{
						if (cur->fToken == token)
						{
							token = token; // break here
							assert(false);
						}

					}
#endif
					//assert(false);
				}
			}

			if (inAccessRights == RWS_ReadOnly && fWriteOwner == currentTaskID)
				inAccessRights = RWS_WriteExclusive;

			if (inAccessRights == RWS_ReadOnly)
			{
				sLONG* pReadOwnerCount = &(fReadOwnerCount[currentTaskID]);
				assert((*pReadOwnerCount) >= 0 && (*pReadOwnerCount) <= 32000);
				if ((*pReadOwnerCount) > 0)
				{
					myTurnNow = true;
				}
				if ((fPendingActions.empty() || myTurnNow) && (fWriteOwner == NULL_TASK_ID || fFreeMemEnCours > 0))
				{
					fReadCount++;
					(*pReadOwnerCount)++;
#if debugrws2
					fDebugRWS.AddMessage(L"Lock inc Read", dbgRWS(inAccessRights, fReadCount, fWriteCount, fWriteOwner, fFreeMemEnCours, nil, token, fPendingActions.empty() ? nil : &(fPendingActions[0])));
#endif
				}
				else
				{
					if (fPendingActions.empty() || fPendingActions.rbegin()->fWhatAccess != RWS_ReadOnly)
					{
						//if (mustgettoken)
						{
							mustgettoken = false;
							fNextToken++;
							token = fNextToken;
						}
						ReadWriteSemaphore_PendingAction newaction;
						newaction.fWaitingEvent = new dVSyncEvent();
						newaction.fWhatAccess = RWS_ReadOnly;
						newaction.fToken = token;
						newaction.fReadWaiting = 1;
						newaction.fUnlocked = false;
						fPendingActions.push_back(newaction);
						event = newaction.fWaitingEvent;
						event->Retain();
#if debugrws2
						fDebugRWS.AddMessage(L"Push Pending Action (Read)", dbgRWS(inAccessRights, fReadCount, fWriteCount, fWriteOwner, fFreeMemEnCours, event, token, &newaction));
#endif
						OneMoreTime = true;
					}
					else
					{
						{
							mustyield = true;
							mustgettoken = false;
							token = fPendingActions.rbegin()->fToken;
							if (!dejaIncRead)
							{
								fPendingActions.rbegin()->fReadWaiting++;
								dejaIncRead = true;
							}
							event = fPendingActions.rbegin()->fWaitingEvent;
							event->Retain();
							OneMoreTime = true;
						}
					}
				}
			}
			else
			{
				sLONG* pReadOwnerCount = &(fReadOwnerCount[currentTaskID]);
				assert((*pReadOwnerCount) >= 0 && (*pReadOwnerCount) <= 32000);
				if ((*pReadOwnerCount) > 0 && inAccessRights != RWS_FreeMemAccess) // si la meme tache avait deja locke le sempaphore en read
				{
					bool mustwait = false;
					if (fWriteOwner != 0 && fWriteOwner != currentTaskID)
					{
						assert(fFreeMemEnCours > 0);
						mustwait = true;
					}
					if ( mustwait || (fReadCount - (*pReadOwnerCount) > 0))
					{
						mustgettoken = false;
						fNextToken++;
						token = fNextToken;
						ReadWriteSemaphore_PendingAction newaction;
						newaction.fWaitingEvent = new dVSyncEvent();
						newaction.fWhatAccess = RWS_WriteExclusive;
						newaction.fToken = token;
						newaction.fReadWaiting = *pReadOwnerCount;
						newaction.fUnlocked = false;
						event = newaction.fWaitingEvent;
						event->Retain();
						RWS_PendingActionVector::iterator firstnext = fPendingActions.begin();
						if ((!fPendingActions.empty()) && firstnext->fUnlocked)
						{
							firstnext++;
						}
						fPendingActions.insert(firstnext, newaction);
						fReadCount = fReadCount - (*pReadOwnerCount);
						assert(fReadCount >= 0);
						*pReadOwnerCount = 0;
#if debugrws2
						fDebugRWS.AddMessage(L"Insert Pending Action (Write after Read)", dbgRWS(inAccessRights, fReadCount, fWriteCount, fWriteOwner, fFreeMemEnCours, event, token, &newaction));
#endif
						OneMoreTime = true;
					}
					else
					{
#if debugrws2
						if (fWriteOwner != 0 && fWriteOwner != currentTaskID)
						{
							sLONG i =0; // break here;
							assert(false);
						}
						if (fWriteCount != 0)
						{
							sLONG i =0; // break here;
						}
#endif
						fWriteOwner = currentTaskID;
						fWriteCount++;
					}
				}
				else
				{
					if ((fReadCount == 0 || inAccessRights == RWS_FreeMemAccess) && ((fWriteOwner == NULL_TASK_ID && (fPendingActions.empty() || myTurnNow)) || fWriteOwner == currentTaskID))
					{
						if (inAccessRights == RWS_FreeMemAccess)
							fFreeMemEnCours++;
						fWriteOwner = currentTaskID;
						fWriteCount++;
					}
					else
					{
						if (mustgettoken)
						{
							mustgettoken = false;
							fNextToken++;
							token = fNextToken;
						}
						ReadWriteSemaphore_PendingAction newaction;
						newaction.fWaitingEvent = new dVSyncEvent();
						newaction.fWhatAccess = RWS_WriteExclusive;
						newaction.fToken = token;
						newaction.fReadWaiting = 0;
						newaction.fUnlocked = false;
						fPendingActions.push_back(newaction);
						event = newaction.fWaitingEvent;
						event->Retain();
#if debugrws2
						fDebugRWS.AddMessage(L"Push Pending Action (Write)", dbgRWS(inAccessRights, fReadCount, fWriteCount, fWriteOwner, fFreeMemEnCours, event, token, &newaction));
#endif
						OneMoreTime = true;
					}
				}
			}

			fMutex.Unlock();
		}

		// maintenant fMutex est libere
		if (event != nil)
		{
			if (mustyield)
				VTask::YieldNow();

#if debugsyncevent
			{
				VTaskLock lock(&fMutex);
				bool trouve = false;
				for (RWS_PendingActionVector::iterator cur = fPendingActions.begin(), end = fPendingActions.end(); cur != end; cur++)
				{
					if (cur->fWaitingEvent == event)
						trouve = true;
				}
				if (!trouve && event->IsStillLocked())
				{
					trouve = trouve; // break here
					assert(false);
				}
			}
#endif
#if debugrws2
			//fDebugRWS.AddMessage(L"Lock en Attente", dbgRWS(inAccessRights, fReadCount, fWriteCount, fWriteOwner, fFreeMemEnCours, event, token));
#endif
			event->Lock(); // on attend qu'une autre tache le libere
			event->Release();
		}

	} while (OneMoreTime);

#if debugrws2
	//fDebugRWS.AddMessage(L"Apres Lock", dbgRWS(inAccessRights, fReadCount, fWriteCount, fWriteOwner, fFreeMemEnCours));
#endif

}

void ReadWriteSemaphore::Unlock(bool WasFreeMem)
{
#if debugrws2
	//fDebugRWS.AddMessage(L"Avant UnLock", dbgRWS(RWS_NoAccess, fReadCount, fWriteCount, fWriteOwner, fFreeMemEnCours));
#endif
	VTaskLock lock(&fMutex);

#if debugrws2
	ReadWriteSemaphore_PendingAction* xaction = nil;
	if (!fPendingActions.empty())
		xaction = &(fPendingActions[0]);
	fDebugRWS.AddMessage(L"Debut Unlock", dbgRWS(RWS_NoAccess, fReadCount, fWriteCount, fWriteOwner, fFreeMemEnCours, nil, 0, xaction));
#endif

	VTaskID currentTaskID = VTask::GetCurrentID();
	if (fWriteOwner == currentTaskID)
	{
		if (WasFreeMem)
		{
			assert(fFreeMemEnCours > 0);
			fFreeMemEnCours--;
		}
		fWriteCount--;
		assert(fWriteCount >= 0);
		//assert(fReadCount == 0);
		if (fWriteCount == 0)
		{
			fWriteOwner = NULL_TASK_ID;
			if (fReadCount > 0)
			{
				fReadCount = fReadCount; // break here
				// on est ici dans le cas ou on unlock un write alors que la meme tache avait demande un lock read auparavant
				// et il ne faut pas unlocker le fWaitingEvent suivant car la thread en cours garde la main en lecture
			}
			else
			{
				if (!fPendingActions.empty())
				{
#if debugrws2
					fDebugRWS.AddMessage(L"Unlock Event (Write)", dbgRWS(RWS_NoAccess, fReadCount, fWriteCount, fWriteOwner, fFreeMemEnCours, fPendingActions.begin()->fWaitingEvent, 0, xaction));
#endif
					fPendingActions.begin()->fUnlocked = true;
					fPendingActions.begin()->fWaitingEvent->Unlock(); // on previent le suivant que c'est son tour;
				}
			}
		}
	}
	else
	{
		assert(!WasFreeMem);
#if debugrws2
		if ((fWriteOwner != 0 || fWriteCount!=0) && fFreeMemEnCours == 0)
		{
			sLONG i = 0; //break here
			assert(false);
		}
#endif
		sLONG* pReadOwnerCount = &(fReadOwnerCount[currentTaskID]);
		assert((*pReadOwnerCount) > 0);
		if (*pReadOwnerCount > 0)
			(*pReadOwnerCount)--;

		assert(fReadCount > 0);
		if (fReadCount > 0)
			fReadCount--;
		// assert(fWriteCount == 0); may happen because of RWS_FreeMemAccess
		assert(fReadCount >= 0);
		{
			if (fReadCount == 0)
			{
				if (!fPendingActions.empty())
				{
#if debugrws2
					fDebugRWS.AddMessage(L"Unlock Event (Read)", dbgRWS(RWS_NoAccess, fReadCount, fWriteCount, fWriteOwner, fFreeMemEnCours, fPendingActions.begin()->fWaitingEvent, 0, xaction));
#endif
					fPendingActions.begin()->fUnlocked = true;
					fPendingActions.begin()->fWaitingEvent->Unlock(); // on previent le suivant que c'est son tour;
				}
			}
		}
	}
#if debugrws2
	fDebugRWS.AddMessage(L"Apres UnLock", dbgRWS(RWS_NoAccess, fReadCount, fWriteCount, fWriteOwner, fFreeMemEnCours));
#endif

}

#if debugrws2

void ReadWriteSemaphore::checkPendingActions()
{
	for (RWS_PendingActionVector::iterator cur = fPendingActions.begin(), end = fPendingActions.end(); cur != end; cur++)
	{
		for (RWS_PendingActionVector::iterator cur2 = cur+1; cur2 != end; cur2++)
		{
			if (cur->fToken == cur2->fToken)
			{
				sLONG i = 0; // break here
				assert(false);
			}
		}
	}
}

#endif


			// ---------------------------------------------------------------------------------

#if debugrws2
sLONG ReadWriteSemaphore::sTotalCount = 0;
#endif


inline void ConsumeCPU()
{
	for( sLONG j = VSystem::Random() % 10 ; j >= 0 ; --j)
	{
		for( sLONG i = VSystem::Random() % 100000 ; i >= 0 ; --i)
			;
		VTask::YieldNow();
	}
}


static sLONG TaskRunProc( VTask* inTask)
{
	std::pair<ReadWriteSemaphore*,sLONG> *info = (std::pair<ReadWriteSemaphore*,sLONG>*) inTask->GetKindData();
	
	ReadWriteSemaphore *sema = info->first;
	
	xbox_assert( sema->GetCurrentTaskReadOwnerCount() == 0);
	static uLONG t0 = 0;
	
	while( !inTask->IsDying())
	{
		sLONG sel = VSystem::Random() % 20;
		switch( sel)
		{
			default:
				inTask->SetName( CVSTR( "reading  ") + ToString(VTask::GetCurrentID()));
				sema->LockRead();
				ConsumeCPU();
				if ((VSystem::Random() % 40) == 0)
				{
					if (sema->TryToLockFreeMem())
					{
						ConsumeCPU();
						sema->Unlock(true);
					}
				}
				sema->Unlock();
				break;

				
			case 3:
				inTask->SetName( CVSTR( "double reading  ") + ToString(VTask::GetCurrentID()));
				sema->LockRead();
				ConsumeCPU();
				sema->LockRead();
				ConsumeCPU();
				if ((VSystem::Random() % 40) == 0)
				{
					if (sema->TryToLockFreeMem())
					{
						ConsumeCPU();
						sema->Unlock(true);
					}
				}
				sema->Unlock();
				sema->Unlock();
				break;
			
			case 4:
				inTask->SetName( CVSTR( "writing  ") + ToString(VTask::GetCurrentID()));
				sema->LockWrite();
				ConsumeCPU();
				if ((VSystem::Random() % 40) == 0)
				{
					if (sema->TryToLockFreeMem())
					{
						ConsumeCPU();
						sema->Unlock(true);
					}
				}
				sema->Unlock();
				break;
				
			case 5:
				inTask->SetName( CVSTR( "reading then writing  ") + ToString(VTask::GetCurrentID()));
				sema->LockRead();
				ConsumeCPU();
				sema->LockWrite();
				ConsumeCPU();
				if ((VSystem::Random() % 40) == 0)
				{
					if (sema->TryToLockFreeMem())
					{
						ConsumeCPU();
						sema->Unlock(true);
					}
				}
				sema->Unlock();
				sema->Unlock();
				break;
				
			
			case 6:
				// first thread only calls freemem each 1/60 second
				if ((info->second == 1) && VSystem::GetCurrentDeltaTicks( t0) > 1)
				{
					fprintf( stdout, "freemem\n");
					inTask->SetName( CVSTR( "freemem") + ToString(VTask::GetCurrentID()));
					if (sema->TryToLockFreeMem())
					{
						ConsumeCPU();
						sema->Unlock(true);
					}
					t0 = VSystem::GetCurrentTicks();
				}
				break;

		}
		xbox_assert( sema->GetCurrentTaskReadOwnerCount() == 0);
		inTask->SetName( CVSTR( "sleeping  ") + ToString(VTask::GetCurrentID()));
		VTask::Sleep( VSystem::Random() % 20);
	}
	
	delete info;
	return 0;
}


void Test_ReadWriteSemaphore( sLONG inCountTasks)
{
	ReadWriteSemaphore *sema = new ReadWriteSemaphore;
	
	for( sLONG i = 1 ; i <= inCountTasks ; ++i)
	{
		VTask *task = new VTask( NULL, 0, eTaskStylePreemptive, TaskRunProc);
		task->SetKindData( (sLONG_PTR) new std::pair<ReadWriteSemaphore*,sLONG>( sema, i) );
		task->Run();
		task->Release();
	}
	
//	delete sema;
}


			// ---------------------------------------------------------------------------------



bool IOccupable::IsOccupied() const
{
	return gOccPool.IsOccupied(this);
}


void IOccupable::Occupy(OccupableStack* curstack, bool withWaitMem) const
{
	if (withWaitMem)
		gOccPool.WaitForMemoryManager(curstack);
	Yield_Debug;
	curstack->Add(this);
	fOccupyStamp++;
	if (withWaitMem)
		gOccPool.EndWaitForMemoryManager(curstack);
}

void IOccupable::Free(OccupableStack* curstack, bool withWaitMem) const
{
	if (withWaitMem)
		gOccPool.WaitForMemoryManager(curstack);
	Yield_Debug;
	curstack->Remove(this);
	fOccupyStamp++;
	if (withWaitMem)
		gOccPool.EndWaitForMemoryManager(curstack);
}



			// -------------------------



OccupableStack::OccupableStack(VTaskID inTaskID)
{
	fStack.reserve(kMaxElemInStack);
	fStamp = 0;
	fSavedStamp = 0;
	fMarkedForWaitMem = 0;
	fMarkedByMemMgr = 0;
	fTaskID = inTaskID;
	fLocker = 0;
	VTask::GetCurrent()->GetName(fName);
}



OccupableStack::~OccupableStack()
{
	assert(fStack.empty());
}



void OccupableStack::Add(const IOccupable* inObject)
{
	bool stop;
	do
	{
		fStamp++;
		if (fLocker == 0 || fLocker == fTaskID)
		{
			fLocker = fTaskID;
			stop = true;
			fStack.push_back(inObject);
			assert(fStack.size() < kMaxElemInStack);
			fStamp++;
			fLocker = 0;
		}
		else
		{
			VTaskID owner;
			do 
			{
				VTask::YieldNow();
				owner = VInterlocked::CompareExchange(&fLocker, 0, fTaskID);
			} while(owner != fTaskID);
			//gOccPool.SubWaitMemMgr(this);
			stop = false;
		}
	} while (!stop);
}


void OccupableStack::Remove(const IOccupable* inObject)
{
	bool stop;
	do
	{
		fStamp++;
		if (fLocker == 0 || fLocker == fTaskID)
		{
			fLocker = fTaskID;
			stop = true;
			if (testAssert(!fStack.empty()))
			{
				occupevector::iterator cur = fStack.end() - 1;
				if (*cur == inObject)
				{
					fStack.pop_back();
				}
				else
				{
					cur = find(fStack.begin(), fStack.end(), inObject);
					if (testAssert(cur != fStack.end()))
					{
						fStack.erase(cur);
					}
				}
			}
			fStamp++;
			fLocker = 0;
		}
		else
		{
			VTaskID owner;
			do 
			{
				VTask::YieldNow();
				owner = VInterlocked::CompareExchange(&fLocker, 0, fTaskID);
			} while (owner != fTaskID);
			//gOccPool.SubWaitMemMgr(this);
			stop = false;
		}
	} while (!stop);
}


void OccupableStack::PutObjectsInto(OccupableSet& OccupiedObjects)
{
	for (occupevector::iterator cur = fStack.begin(), end = fStack.end(); cur != end; cur++)
	{
		OccupiedObjects.insert(*cur);
	}
}


bool OccupableStack::FindObjectInStack(const IOccupable* inObject) // used for debugging purpose
{
	bool found = false;
	VTaskID owner;
	do 
	{
		owner = VInterlocked::CompareExchange(&fLocker, 0, fTaskID);
		if (owner != fTaskID)
			VTask::YieldNow();
	} while(owner != fTaskID);
	
	for (occupevector::iterator cur = fStack.begin(), end = fStack.end(); cur != end && !found; cur++)
	{
		if (inObject == *cur)
			found = true;
	}
	
	fLocker = 0;

	return found;
}




			// -------------------------

void RemoveOccupableStackFromThread ( void *inData)
{
	OccupableStack* stack = (OccupableStack*)inData;
	if (stack != nil)
	{
		gOccPool.RemoveStack(stack->GetTaskId());
	}
}


OccupablePool::OccupablePool()
{
	fWaitForMemMgr = 0;
	fWaitMemMgrSync = nil;

	fTaskDataKey = VTask::CreateDataKey( RemoveOccupableStackFromThread);
	//fGlobalStamp = 0;
}


OccupablePool::~OccupablePool()
{
	for (OccupableStackCollection::iterator cur = fAllStacks.begin(), end = fAllStacks.end(); cur != end; cur++)
	{
		OccupableStack* stack = cur->second;
		if (stack != nil)
			delete stack;
	}
	VTask::DeleteDataKey(fTaskDataKey);
}


void OccupablePool::RemoveStack(sLONG threadID)
{
	VTaskLock lock(&fMutex);
	fAllStacks.erase(threadID);
}


OccupableStack* OccupablePool::GetStackForCurrentThread()
{
	VTaskID curthreadid = VTask::GetCurrentID();
	VTaskLock lock(&fMutex);
	/*
	if (curthreadid+1 > fAllStacks.size())
		fAllStacks.resize(curthreadid+1, nil);
		*/
	OccupableStack** cur = &fAllStacks[curthreadid];
	if (*cur == nil)
	{
		*cur = new OccupableStack(curthreadid);
		VTask::SetCurrentData(fTaskDataKey, *cur);
	}
	return *cur;
}


void OccupablePool::WaitForMemoryManager(OccupableStack* curstack)
{
	Yield_Debug;
	if (fWaitForMemMgr != 0 || curstack->IsUsedByGarbageCollector())
	{
		if (!curstack->IsMarkedForWaitMem())
		{
			vxSyncEvent* event = nil;
			{
				VTaskLock lock(&fMutex);
				if (fWaitForMemMgr != 0) // peut avoir change car le premier n'est pas protege par mutex
				{
					event = fWaitMemMgrSync;
					event->Retain();
				}
			}
			if (event != nil)
			{
				event->Wait();
				event->Release();
			}
		}
		else
		{
			sLONG a = 1;
		}
	}
	curstack->MarkForWaitMem();
}


void OccupablePool::SubWaitMemMgr(OccupableStack* curstack)
{
	/*
	sLONG oldmark = curstack->GetCurMark();
	curstack->ResetCurMark();
	vxSyncEvent* event = nil;
	{
		VTaskLock lock(&fMutex);
		if (fWaitForMemMgr != 0) 
		{
			event = fWaitMemMgrSync;
			event->Retain();
		}
	}
	if (event != nil)
	{
		event->Wait();
		event->Release();
	}
	curstack->SetBackCurMark(oldmark);
	*/
	VTaskLock lock(&fMutex);
}



void OccupablePool::EndWaitForMemoryManager(OccupableStack* curstack)
{
	Yield_Debug;
	curstack->UnMarkForWaitMem();
}


bool OccupablePool::FindObjectInStack(const IOccupable* inObject) // used for debugging purpose
{
	bool found = false;
	VTaskLock lock(&fMutex);
	for (OccupableStackCollection::iterator cur = fAllStacks.begin(), end = fAllStacks.end(); cur != end && !found; cur++)
	{
		OccupableStack* stack = cur->second;
		if (stack != nil)
		{
			if (stack->FindObjectInStack(inObject))
			{
				found = true;
			}
		}
	}

	return found;
}

void OccupablePool::StartGarbageCollection()
{
	VTaskID currentTaskID = VTask::GetCurrentID();
	Yield_Debug;
	{
		VTaskLock lock(&fMutex);
		VInterlocked::Exchange(&fWaitForMemMgr,1);
		fWaitMemMgrSync = new vxSyncEvent;
	}

	VTask::Sleep(5);

	{
		bool alLeastOneNotFree;
		do
		{
			//sLONG oldGlobalStamp = fGlobalStamp;
			{
				alLeastOneNotFree = false;
				VTaskLock lock(&fMutex);
				fOccupiedObjects.clear();
				for (OccupableStackCollection::iterator cur = fAllStacks.begin(), end = fAllStacks.end(); cur != end; cur++)
				{
					OccupableStack* stack = cur->second;
					if (stack != nil)
					{
						stack->SaveStamp();
					}
				}

				for (OccupableStackCollection::iterator cur = fAllStacks.begin(), end = fAllStacks.end(); cur != end; cur++)
				{
					OccupableStack* stack = cur->second;
					if (stack != nil)
					{
						sLONG newlock = VInterlocked::CompareExchange(&(stack->fLocker), 0, currentTaskID);
						if (newlock != currentTaskID && newlock != 0)
						{
							alLeastOneNotFree = true;
						}
						stack->UsedByGarbageCollector();
					}
				}
				//VTask::Sleep(5);
				VTask::YieldNow();

				
				if (!alLeastOneNotFree)
				{
					//VTask::YieldNow();

					for (OccupableStackCollection::iterator cur = fAllStacks.begin(), end = fAllStacks.end(); cur != end; cur++)
					{
						OccupableStack* stack = cur->second;
						if (stack != nil)
						{
#if debuglr
							if (stack->fLocker != currentTaskID)
							{
								sLONG a = 1; // put a big break here
								assert(stack->fLocker == currentTaskID);
							}
#endif

							if (stack->IsMarkedForWaitMemAtomic() || stack->StampHasChanged())
							{
								alLeastOneNotFree = true;
							}
							else
								stack->PutObjectsInto(fOccupiedObjects);

							VInterlocked::Exchange(&(stack->fLocker), 0);
							stack->ReleasedByGarbageCollector();
						}
					}
				}
				
				for (OccupableStackCollection::iterator cur = fAllStacks.begin(), end = fAllStacks.end(); cur != end; cur++)
				{
					OccupableStack* stack = cur->second;
					if (stack != nil)
					{
						VInterlocked::CompareExchange(&(stack->fLocker), currentTaskID, 0);
					}
				}

				
			}

			/*
			if (oldGlobalStamp != VInterlocked::AtomicGet(&fGlobalStamp))
				alLeastOneNotFree = true;
			 */
			
			if (alLeastOneNotFree)
			{
				VTask::Sleep(5);
			}
			
		} while (alLeastOneNotFree);

	}
}


void OccupablePool::EndGarbageCollection()
{
	Yield_Debug;
	VTaskLock lock(&fMutex);
	fWaitForMemMgr = 0;
	fWaitMemMgrSync->Unlock();
	fWaitMemMgrSync->Release();
	fWaitMemMgrSync = nil;
	fOccupiedObjects.clear();
	MemoryBarrier();
}


bool OccupablePool::IsOccupied(const IOccupable* inObject)
{
	Yield_Debug;
	if (fOccupiedObjects.find(inObject) == fOccupiedObjects.end())
		return false;
	else
		return true;
}



OccupablePool gOccPool;



											// ----------------------------------------------------------------------------  




ProgressEncapsuleur::ProgressEncapsuleur(VProgressIndicator* inProgress)
{
	fStartingTime = 0;
	fProgress = inProgress;
	fIsStarted = false;
}


ProgressEncapsuleur::ProgressEncapsuleur(VProgressIndicator* inProgress, sLONG8 inMaxValue, const VString& inMessage, bool inCanInterrupt)
{
	fProgress = inProgress;
	fIsStarted = false;
	fMaxValue = inMaxValue;
	fMessage = inMessage;
	fCanInterrupt = inCanInterrupt;
	if (fProgress != nil)
		fStartingTime = VSystem::GetCurrentTime();
}



bool ProgressEncapsuleur::Progress(sLONG8 inCurValue)
{
	if (fIsStarted)
	{
		if (fProgress != nil)
			return fProgress->Progress(inCurValue);
		else
			return !MustStopTask();
	}
	else
	{
		if (fProgress != nil)
		{
			if ((VSystem::GetCurrentTime() - fStartingTime) > 1000)
			{
				fIsStarted = true;
				if (fCanInterrupt)
					fProgress->BeginSession(fMaxValue, fMessage, fCanInterrupt);
				else
					fProgress->BeginSession(fMaxValue, fMessage);
				return fProgress->Progress(inCurValue);
			}
		}
		return !MustStopTask();
	}
}


bool ProgressEncapsuleur::Increment(sLONG8 inInc)
{
	if (fIsStarted)
	{
		if (fProgress != nil)
			return fProgress->Increment(inInc);
		else
			return !MustStopTask();
	}
	else
	{
		if (fProgress != nil)
		{
			if ((VSystem::GetCurrentTime() - fStartingTime) > 1000)
			{
				fIsStarted = true;
				if (fCanInterrupt)
					fProgress->BeginSession(fMaxValue, fMessage, fCanInterrupt);
				else
					fProgress->BeginSession(fMaxValue, fMessage);
				return fProgress->Increment(inInc);
			}
		}
		return !MustStopTask();
	}
}



void ProgressEncapsuleur::BeginSession(sLONG8 inMaxValue, const VString& inMessage, bool inCanInterrupt)
{
	fMaxValue = inMaxValue;
	fMessage = inMessage;
	fCanInterrupt = inCanInterrupt;
	fStartingTime = VSystem::GetCurrentTime();
}


void ProgressEncapsuleur::EndSession()
{
	if (fIsStarted && fProgress != nil)
	{
		fProgress->EndSession();
		fIsStarted = false;
	}
}


ProgressEncapsuleur::~ProgressEncapsuleur()
{
	if (fIsStarted)
		EndSession();
}



// ----------------------------------------------------

#if debugOverWrite_strong

class debug_indexpageref
{
public:
	inline debug_indexpageref()
	{
		fInd = nil;
		fPageNum = 0;
		fLen = 0;
	}

	IndexInfo* fInd;
	sLONG fPageNum;
	sLONG fLen;
};

typedef map<DataAddr4D, debug_indexpageref> debug_mapofindexpageref;

debug_mapofindexpageref debug_IndexPageRefs;
VCriticalSection debug_IndexPageRefMutex;

DataAddr4D debug_curWritingPage;

void debug_SetCurrentWritingPage(DataAddr4D ou)
{
	debug_curWritingPage = ou;
}


void debug_ClearCurrentWritingPage()
{
	debug_curWritingPage = 0;
}


void debug_SetPageRef(IndexInfo* ind, sLONG pagenum, DataAddr4D ou, sLONG len)
{
	if (ind != nil && ind->GetDB()->GetStructure() != nil)
	{
		VTaskLock lock(&debug_IndexPageRefMutex);
		debug_indexpageref* xref = &debug_IndexPageRefs[ou];
		if (xref->fInd != nil)
		{
			assert(false);
		}
		xref->fInd = ind;
		xref->fLen = len;
		xref->fPageNum = pagenum;
	}
}


void debug_ClearPageRef(DataAddr4D ou, sLONG len, IndexInfo* ind)
{
	if (ind != nil && ind->GetDB()->GetStructure() != nil)
	{
		VTaskLock lock(&debug_IndexPageRefMutex);
		debug_IndexPageRefs.erase(ou);
	}
}



void debug_CheckWriting(DataAddr4D ou, sLONG len)
{
	if (ou <= debug_curWritingPage && (ou+len) > debug_curWritingPage)
	{
		// rien a faire, nous somme sur la page courante qui dois etre ecrite
	}
	else
	{
		VTaskLock lock(&debug_IndexPageRefMutex);
		debug_mapofindexpageref::iterator closest = debug_IndexPageRefs.lower_bound(ou);
		if (closest != debug_IndexPageRefs.end())
		{
			if (ou <= closest->first && (ou+len) > closest->first)
			{
				assert(false); // nous sommes en train d'ecrire sur une autre page que la bonne
			}
		}
	}

}


#endif


// -------------------------------------------------------------


void Correspondance::From(const ConstUniCharPtr* from)
{
	const ConstUniCharPtr* p = from;
	bool cont = true;
	do 
	{
		VString singulier(*p);
		if (singulier.IsEmpty())
			cont = false;
		else
		{
			p++;
			VString pluriel(*p);
			fFrom[singulier] = pluriel;
			fTo[pluriel] = singulier;
		}
		p++;
	} while(cont);
}


const VString& Correspondance::MatchFrom(const VString& fromString) const
{
	mapOfVString::const_iterator found = fFrom.find(fromString);
	if (found == fFrom.end())
		return fromString;
	else
		return found->second;
}


const VString& Correspondance::MatchTo(const VString& toString) const
{
	mapOfVString::const_iterator found = fTo.find(toString);
	if (found == fTo.end())
		return toString;
	else
		return found->second;
}





VError SaveBagToXML(const VValueBag& inBag, const VString& inRootElementKind, VFile& outFile, bool overwrite, const Correspondance* correspond )
{
	VError err = VE_OK;
	if (overwrite && outFile.Exists())
		err = outFile.Delete();

	if (err == VE_OK)
	{
		err = WriteBagToFileInXML(inBag, inRootElementKind, &outFile);
	}

	return err;
}


VError SaveBagToXML(const VValueBag& inBag, const VString& inRootElementKind, VString& outXML, bool prettyformat, const Correspondance* correspond, bool withheader )
{
	inBag.DumpXML( outXML, inRootElementKind, prettyformat);
	if (withheader)
	{
		if (prettyformat)
			outXML = L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" + outXML;
		else
			outXML = L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>" + outXML;
	}

	return VE_OK;
}


VError SaveBagToJson(const VValueBag& inBag, VString& outJSON, bool prettyformat)
{
	VError err = inBag.GetJSONString(outJSON, prettyformat ? JSON_PrettyFormatting : JSON_Default);
	return err;
}


VError SaveBagToJson(const VValueBag& inBag, VFile& outFile, bool overwrite)
{
	VString jsonString;
	VError err = inBag.GetJSONString(jsonString, JSON_PrettyFormatting);
	if (err == VE_OK)
	{
		VStringConvertBuffer buffer( jsonString, VTC_UTF_8);

		VFileDesc *fd;
		err = outFile.Open( FA_READ_WRITE, &fd, FO_CreateIfNotFound | FO_Overwrite);
		if (err == VE_OK)
		{
			err = fd->PutDataAtPos( buffer.GetCPointer(), buffer.GetSize());
			delete fd;
			if (err != VE_OK)
			{
				if (outFile.Exists())
					outFile.Delete();
			}
		}
	}

	return err;
}


        //---------------------------------------------------------------------------



bool Wordizer::GetNextWord(VString& result, UniChar separator, bool skipleadingspaces, bool keepquotes)
{
	UniChar c;
	result.Clear();
	Boolean first = true;
	Boolean insidequotes = false, insidedoublequotes = false;
	bool WasInQuotes = false;

	do {
		if (curpos<input.GetLength())
		{
			c = input[curpos];
			curpos++;
			if (insidequotes)
			{
				if (c == 39)
				{
					insidequotes = false;
					if (keepquotes)
						result.AppendUniChar(c);
				}
				else
					result.AppendUniChar(c);
			}
			else
			{
				if (insidedoublequotes)
				{
					if (c == 34)
					{
						insidedoublequotes = false;
						if (keepquotes)
							result.AppendUniChar(c);
					}
					else
						result.AppendUniChar(c);
				}
				else
				{
					if (first && c == 32 && skipleadingspaces)
					{
					}
					else
					{
						first = false;
						if (c == separator)
						{
							c = 0;
						}
						else
						{
							if (c == 39)
							{
								insidequotes = true;
								if (keepquotes)
									result.AppendUniChar(c);
							}
							else
							{
								if (c == 34)
								{
									insidedoublequotes = true;
									if (keepquotes)
										result.AppendUniChar(c);
								}
								else
									result.AppendUniChar(c);
							}
						}
					}
				}
			}
		}
		else
			c = 0;

	} while(c != 0);
	return !result.IsEmpty();
}


void Wordizer::RemoveExtraSpaces(VString& s)
{
	Boolean cont = true;

	sLONG len = s.GetLength();
	while (len>0 && cont)
	{
		if (s[len-1] == 32)
			len--;
		else
			cont = false;
	}
	s.Truncate(len);
}


void Wordizer::ExctractStrings(VectorOfVString& outStrings, bool removeExtraSpaces, UniChar separator, bool keepquotes)
{
	outStrings.clear();
	while (curpos < input.GetLength())
	{
		VString s;
		if (GetNextWord(s, separator, removeExtraSpaces, keepquotes))
		{
			if (removeExtraSpaces)
			{
				RemoveExtraSpaces(s);
			}
			if (!s.IsEmpty())
			{
				outStrings.push_back(s);
			}
		}
	}
}




bool okperm(BaseTaskInfo* context, const VUUID& inGroupID)
{
	if (inGroupID.IsNull())
		return true;
	else
	{
		if (context == nil || context->GetCurrentUserSession() == nil)
		{
			return false;
		}
		else
		{
			return context->GetCurrentUserSession()->BelongsTo(inGroupID);
		}
	}
}


bool okperm(CDB4DBaseContext* context, const VUUID& inGroupID)
{
	return okperm(ConvertContext(context), inGroupID);
}


bool okperm(BaseTaskInfo* context, const EntityModel* model, DB4D_EM_Perm perm)
{
	VUUID groupID;
	if (model != nil)
	{
		bool forced;
		model->GetPermission(perm, groupID, forced);
		return okperm(context, groupID);
	}
	else
		return false;
}


bool promoteperm(BaseTaskInfo* context, const VUUID& inGroupID)
{
	return true;
}

bool promoteperm(CDB4DBaseContext* context, const VUUID& inGroupID)
{
	return promoteperm(ConvertContext(context), inGroupID);
}


void endpromote(BaseTaskInfo* context, const VUUID& inGroupID)
{
	
}

bool endpromote(CDB4DBaseContext* context, const VUUID& inGroupID)
{
	return promoteperm(ConvertContext(context), inGroupID);
}


// code sale en attendant quelque chose de Stephane
//RIApplicationRef global_ApplicationRef = nil;


// ---------------------------------------------------------------------------------------------------


#if debugsyncevent
SyncEventMapByTask dVSyncEvent::sEventsByTask;
VCriticalSection dVSyncEvent::sMutex;
#endif


