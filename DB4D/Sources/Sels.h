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
#ifndef __SELS__
#define __SELS__

class DataTable;

/*
enum { wrien=0,DB4D_Equal,DB4D_NotEqual,DB4D_Greater,DB4D_GreaterOrEqual,DB4D_Lower,DB4D_LowerOrEqual,DB4D_Contains_KeyWord,DB4D_DoesntContain_KeyWord,wfourche,DB4D_BeginsWith,wendwith };
enum { DB4D_NOTCONJ=0,DB4D_And,DB4D_OR,DB4D_Except };
*/
/*
struct SearchLine
{
	sLONG numfield;
	sLONG numfile;
	sWORD comparaison;
	sWORD BoolLogic;
	uBOOL leftpar;
	uBOOL rightpar;
	ChampVar *ch;
};

struct SearchTab
{
	sLONG nbline;
	SearchLine li[1];
};
*/

enum { sel_nosel=0, sel_bitsel, sel_longsel, sel_petitesel, sel_bitsel_fullfromclient_keepOnServer, sel_longsel_fullfromclient, sel_constvector, sel_bitsel_fullfromclient };

//void KillSearchTab(SearchTab *s);

class Selection;
class SortTab;
class SortContext;
class SortElem;
class BaseTaskInfo;
class DataBaseObjectHeader;


class SelPosInfo
{
public:
	inline bool operator < (const SelPosInfo& other) const
	{
		return fRecNum < other.fRecNum;
	}

	sLONG fRecNum;
	sLONG fSelPos;
};

#if COMPIL_GCC && 0

#define _GCCAlign __attribute__((__aligned(1)))

#else

#define _GCCAlign

#endif


#pragma pack(push, 1)

template <class Type>
class _GCCAlign TypeSortElem
{
public:

	inline TypeSortElem<Type>& operator =( const TypeSortElem<Type>& other) { recnum = other.recnum; value = other.value; return *this;};
	inline bool operator <( const TypeSortElem<Type>& other) const { return value < other.value;};

	inline void GetString(VString& s)
	{
		s.Clear();
	}

	sLONG recnum;
	Type value;
};


template <>
inline void TypeSortElem<uLONG8>::GetString(VString& s)
{
	s = "U8  "+ToString(value);
}

template <>
inline void TypeSortElem<sLONG8>::GetString(VString& s)
{
	s = "S8  "+ToString(value);
}


template <>
inline void TypeSortElem<sLONG>::GetString(VString& s)
{
	s = "S  "+ToString(value);
}


#if COMPIL_GCC
inline void swap(xMultiFieldData& _Left, xMultiFieldData& _Right)
#else
template<> 
inline void std::swap(xMultiFieldData& _Left, xMultiFieldData& _Right)
#endif
{
	xMultiFieldData* _Tmp = (xMultiFieldData*)_Left.GetAuxBuffer();
	*_Tmp = _Left;
	_Left = _Right, _Right = *_Tmp;
}
/*
template<> 
inline void std::swap(TypeSortElem<xString>& _Left, TypeSortElem<xString>& _Right)
{
	TypeSortElem<xStr<kMaxXStringLen+2> > _AllocTmp;
	TypeSortElem<xString>* _Tmp = (TypeSortElem<xString>*)&_AllocTmp;
	*_Tmp = _Left;
	_Left = _Right;
	_Right = *_Tmp;
}
*/

#if COMPIL_GCC
inline void swap(TypeSortElem<xMultiFieldData>& _Left, TypeSortElem<xMultiFieldData>& _Right)
#else
template<> 
inline void std::swap(TypeSortElem<xMultiFieldData>& _Left, TypeSortElem<xMultiFieldData>& _Right)
#endif
{
	TypeSortElem<xMultiFieldData>* _Tmp = (TypeSortElem<xMultiFieldData>*)_Left.value.GetAuxBuffer();
	*_Tmp = _Left;
	_Left = _Right;
	_Right = *_Tmp;
}


class IndexInfo;
class BTreePageIndex;


template <class Type>
class _GCCAlign TypeSortElemOffset
{
	public:

		inline void SetNumRec(sLONG numrec) { fNumRec = numrec; };
		inline sLONG GetNumRec() const { return fNumRec; };

		//inline void SetDataNull() { fOffset = -Abs(fOffset); };
		//inline void SetDataPtr(char* base, char* data) { fOffset = data - base; };
		inline void SetOffSet(sLONG offset, sLONG len) { fOffset = offset; fLen = len; };
		inline sLONG GetOffSet() const { return fOffset; };
		inline sLONG GetLen() const { return fLen; };
		inline char* GetDataPtr(char* base) const 
		{ 
			if (fLen == 0)
				return nil;
			else
				return base + fOffset; 
		};

		inline char* GetAlawaysValidDataPtr(char* base) const 
		{ 
				return base + fOffset; 
		};


	protected:
		sLONG fNumRec;
		sLONG fOffset;
		sLONG fLen;
};


template <class Type>
class _GCCAlign TypeSortElemOffsetArray
{
	public:
		inline TypeSortElemOffsetArray(sLONG size) { fNbElem = 0; fSize = size;};

		inline sLONG GetLastOffset() const
		{
			if (fNbElem == 0)
				return fSize;
			else
				return fOffsets[fNbElem-1].GetOffSet();
		};

		inline sLONG CalcFreeSpace() const { return GetLastOffset() - 8 - (sizeof(TypeSortElemOffset<Type>)*fNbElem); };

		inline void SetNbElem(sLONG nb) { fNbElem = nb; };
		inline sLONG GetNbElem() const { return fNbElem; };

		void* AddOffSet(sLONG numrec, sLONG lendata);
		//inline void SetLastElemNull() { assert(fNbElem > 0); fOffsets[fNbElem-1]->SetDataNull(); };

		VError WriteToStream(VStream* out);

		VError Sort(const xMultiFieldDataOffsets& criterias, Boolean TestUniq, Boolean BreakOnNonUnique, 
					Boolean& outUniq, Boolean PourIndex, const VCompareOptions& inOptions);

	protected:
		sLONG fNbElem;
		sLONG fSize;
		TypeSortElemOffset<Type> fOffsets[1];
};

template <class Type, sLONG MaxCles> class BtreePage;


class QuickDB4DArrayOfValues;
class WafSelection;

template <class Type>
class _GCCAlign TypeSortElemArray
{
	public:
		typedef vector<TypeSortElem<Type>*> ptrvect;

		TypeSortElemArray();
		virtual ~TypeSortElemArray();

		Boolean TryToSort(Selection* From, Selection* &into, VError& err, const xMultiFieldDataOffsets& criterias, BaseTaskInfo* context, 
											VDB4DProgressIndicator* InProgress, Boolean TestUnicite, sLONG TypeSize, IndexInfo* ind = nil);
		
		void SubSort( sLONG l, sLONG r, VDB4DProgressIndicator* InProgress, sLONG TypeSize);
		VError SubSortWithUnicite( sLONG l, sLONG r, VDB4DProgressIndicator* InProgress, sLONG TypeSize);
		
		VError GenereIndex(IndexInfo* ind, sLONG MaxElemsInPage, sLONG RequiredElemsInPage, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, sLONG typ, sLONG TypeSize);
		
		void GenerePageIndex(OccupableStack* curstack, BTreePageIndex* page, VError &err, sLONG &curelem, sLONG maxlevel, sLONG level, IndexInfo* ind, 
																		sLONG MaxElemsInPage, sLONG RequiredElemsInPage, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, 
																		sLONG typ, sLONG TypeSize, sLONG maxElemsToCompute, Boolean CheckRemain, Boolean islast);

		VError GenereClusterIndex(IndexInfo* ind, sLONG MaxElemsInPage, sLONG RequiredElemsInPage, BaseTaskInfo* context, 
															VDB4DProgressIndicator* InProgress, sLONG typ, sLONG TypeSize, xMultiFieldDataOffsets& criterias);

		void GenerePageClusterIndex(OccupableStack* curstack, BTreePageIndex* page, VFileStream* keysdata, VError &err, sLONG &curelem, sLONG &curcluster, sLONG maxlevel, sLONG level, IndexInfo* ind, 
																					sLONG MaxElemsInPage, sLONG RequiredElemsInPage, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, 
																					sLONG typ, sLONG TypeSize, sLONG maxElemsToCompute, Boolean CheckRemain, Boolean islast);

		VError GenereIndexScalar(IndexInfo* ind, sLONG MaxElemsInPage, sLONG RequiredElemsInPage, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, sLONG typ, sLONG TypeSize);
		
		void GenerePageIndexScalar(OccupableStack* curstack, BtreePage<Type, kNbKeysForScalar>* page, VError &err, sLONG &curelem, sLONG maxlevel, sLONG level, IndexInfo* ind, 
																		sLONG MaxElemsInPage, sLONG RequiredElemsInPage, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, 
																		sLONG typ, sLONG TypeSize, sLONG maxElemsToCompute, Boolean CheckRemain, Boolean islast);

		VError GenereClusterIndexScalar(IndexInfo* ind, sLONG MaxElemsInPage, sLONG RequiredElemsInPage, BaseTaskInfo* context, 
															VDB4DProgressIndicator* InProgress, sLONG typ, sLONG TypeSize, xMultiFieldDataOffsets& criterias);

		void GenerePageClusterIndexScalar(OccupableStack* curstack, BtreePage<Type, kNbKeysForScalar>* page, VFileStream* keysdata, VError &err, sLONG &curelem, sLONG &curcluster, sLONG maxlevel, sLONG level, IndexInfo* ind, 
																					sLONG MaxElemsInPage, sLONG RequiredElemsInPage, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, 
																					sLONG typ, sLONG TypeSize, sLONG maxElemsToCompute, Boolean CheckRemain, Boolean islast);

		VError BuildDistinctKeys(DB4DCollectionManager& outCollection, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, sLONG typ, sLONG TypeSize, const VCompareOptions& inOptions);

		VError QuickBuildDistinctKeys(QuickDB4DArrayOfValues *outCollection, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, sLONG typ, sLONG TypeSize, const VCompareOptions& inOptions);

		inline void* GetDataPtr() { return data; };
		inline sLONG GetCount() const { return count; };

		inline TypeSortElem<Type>* GetDataElem(sLONG n, sLONG TypeSize) { return (TypeSortElem<Type>*)(((char*)data) + n*(TypeSize+4)); };
		inline void* GetSubDataPtr(sLONG n, sLONG TypeSize, sLONG offset) { return (void*)(((char*)data) + n*(TypeSize+4) + offset + 4); };
		static inline TypeSortElem<Type>* NextDataElem(TypeSortElem<Type>* p, sLONG TypeSize) { return (TypeSortElem<Type>*)(((char*)p) + (TypeSize+4)); };

		void SetBreakOnNonUnique(Boolean x) { fBreakOnNonUnique = x; };
		Boolean OKUnique() const { return fAllUnique; };

		Bittab* inWafSelbits;
		WafSelection* outWafSel;

	protected:
		void *data;
		sLONG count;
		void* auxdata;
		void* auxdata2;
		void* auxbuff;
		char smallbuff[128];
		VFileStream* filedata;
		Boolean fBreakOnNonUnique;
		Boolean fAllUnique;
		VIntlMgr* fIntlMgr;
		VCompareOptions fOptions;
		Bittab* fNulls;
};

/*
Constructor and destructor moved here because of a gcc linking error with optimisation
*/

template <class Type>
TypeSortElemArray<Type>::TypeSortElemArray()
{
	data = nil;
	auxdata = nil;
	auxdata2 = nil;
	auxbuff = (void*)&smallbuff;
	filedata = nil;
	fAllUnique = true;
	fBreakOnNonUnique = true;
	fIntlMgr = nil;
	fNulls = nil;
	inWafSelbits = nil;
	outWafSel = nil;
}

template <class Type>
TypeSortElemArray<Type>::~TypeSortElemArray()
{
	if (data != nil)
	{
		FreeTempMem((void*)data);
	}
	if (auxdata != nil)
	{
		FreeTempMem((void*)auxdata);
	}
	if (auxdata2 != nil)
	{
		FreeTempMem((void*)auxdata2);
	}
	if (filedata != nil)
	{
		if (filedata->IsWriting())
			filedata->CloseWriting();
		if (filedata->IsReading())
			filedata->CloseReading();
		const VFile* ff = filedata->GetVFile();
		if (ff != nil)
			ff->Delete();
		delete filedata;
	}
	QuickReleaseRefCountable(fNulls);
}

template<> inline TypeSortElem<sLONG>* TypeSortElemArray<sLONG>::GetDataElem(sLONG n, sLONG TypeSize) { return &((TypeSortElem<sLONG>*)data)[n]; };
template<> inline TypeSortElem<sLONG8>* TypeSortElemArray<sLONG8>::GetDataElem(sLONG n, sLONG TypeSize) { return &((TypeSortElem<sLONG8>*)data)[n]; };
template<> inline TypeSortElem<xTime>* TypeSortElemArray<xTime>::GetDataElem(sLONG n, sLONG TypeSize) { return &((TypeSortElem<xTime>*)data)[n]; };
template<> inline TypeSortElem<sWORD>* TypeSortElemArray<sWORD>::GetDataElem(sLONG n, sLONG TypeSize) { return &((TypeSortElem<sWORD>*)data)[n]; };
template<> inline TypeSortElem<Real>* TypeSortElemArray<Real>::GetDataElem(sLONG n, sLONG TypeSize) { return &((TypeSortElem<Real>*)data)[n]; };
template<> inline TypeSortElem<sBYTE>* TypeSortElemArray<sBYTE>::GetDataElem(sLONG n, sLONG TypeSize) { return &((TypeSortElem<sBYTE>*)data)[n]; };
template<> inline TypeSortElem<VUUIDBuffer>* TypeSortElemArray<VUUIDBuffer>::GetDataElem(sLONG n, sLONG TypeSize) { return &((TypeSortElem<VUUIDBuffer>*)data)[n]; };

template<> inline void* TypeSortElemArray<xMultiFieldData>::GetSubDataPtr(sLONG n, sLONG TypeSize, sLONG offset) 
	{ return GetDataElem(n, TypeSize)->value.GetDataPtr(offset); };


template <class X>
class _GCCAlign TypeSortElemIterator
{
public:
	typedef random_access_iterator_tag	iterator_category;
	typedef TypeSortElem<X>				value_type;
	typedef ptrdiff_t					difference_type;
	typedef TypeSortElem<X>*			pointer;
	typedef TypeSortElem<X>&			reference;

	inline TypeSortElemIterator(TypeSortElem<X>* p, size_t sizeelem)
	{
		typesize = sizeelem;
		ptr = p;
	};

	inline TypeSortElem<X>& operator *() const
	{
		return (TypeSortElem<X>&)*ptr;
	};

	inline TypeSortElem<X>* operator ->() const
	{	
		return (TypeSortElem<X>*)ptr;
	};

	inline bool operator < (const TypeSortElemIterator<X>& other) const
	{
		return ptr < other.ptr;
	};

	inline bool operator <= (const TypeSortElemIterator<X>& other) const
	{
		return ptr <= other.ptr;
	};

	inline bool operator > (const TypeSortElemIterator<X>& other) const
	{
		return ptr > other.ptr;
	};

	inline bool operator >= (const TypeSortElemIterator<X>& other) const
	{
		return ptr >= other.ptr;
	};

	inline bool operator == (const TypeSortElemIterator<X>& other) const
	{
		return ptr == other.ptr;
	};

	inline bool operator != (const TypeSortElemIterator<X>& other) const
	{
		return ptr != other.ptr;
	};

	inline TypeSortElemIterator<X>& operator ++()
	{
		ptr = (TypeSortElem<X>*) ( ((char*)ptr) + typesize );
		return *this;
	};

	inline TypeSortElemIterator<X> operator ++(int) // post increment
	{
		TypeSortElemIterator<X> temp = *this;
		ptr = (TypeSortElem<X>*) ( ((char*)ptr) + typesize );
		return temp;
	};

	inline TypeSortElemIterator<X>& operator --()
	{
		ptr = (TypeSortElem<X>*) ( ((char*)ptr) - typesize );
		return *this;
	};

	inline TypeSortElemIterator<X> operator --(int) // post decrement
	{
		TypeSortElemIterator<X> temp = *this;
		ptr = (TypeSortElem<X>*) ( ((char*)ptr) - typesize );
		return temp;
	};

	inline TypeSortElemIterator<X>& operator +=(size_t inc)
	{
		ptr = (TypeSortElem<X>*) ( ((char*)ptr) + (inc*typesize) );
		return *this;
	};

	inline TypeSortElemIterator<X> operator +(size_t inc) const
	{
		TypeSortElemIterator<X> temp = *this;
		temp.ptr = (TypeSortElem<X>*) ( ((char*)ptr) + (inc*typesize) );
		return temp;
	};

	inline TypeSortElemIterator<X>& operator -=(size_t inc)
	{
		ptr = (TypeSortElem<X>*) ( ((char*)ptr) - (inc*typesize) );
		return *this;
	};

	inline TypeSortElemIterator<X> operator -(size_t inc) const
	{
		TypeSortElemIterator<X> temp = *this;
		temp.ptr = (TypeSortElem<X>*) ( ((char*)ptr) - (inc*typesize) );
		return temp;
	};

	inline ptrdiff_t operator -(const TypeSortElemIterator<X>& other) const
	{
		return (((char*)ptr) - ((char*)other.ptr)) / typesize;
	};

	inline TypeSortElem<X>& operator [](size_t off) const
	{
		return *((TypeSortElem<X>*) (((char*)ptr)+ (typesize*off)));
	};

	TypeSortElem<X>* ptr;
	size_t typesize;
};


typedef struct gcc_bug_bug
{
TypeSortElem<sBYTE> xx;
TypeSortElem<uBYTE> yy;
TypeSortElemIterator<sBYTE> aa;
TypeSortElemIterator<uBYTE> bb;
} gcc_bug_bug;

#pragma pack(pop)

class Relation;
class QuickDB4DArrayOfValues;

typedef Selection *SelectionPtr;
class IRequestReply;

class FieldsForCache;

class EntityAttributeSortedSelection;
class EntityModel;
class WafSelection;

class Selection : public ObjInCacheMem, public IObjToFlush, public IChainable<Selection>
{
	public:
		Selection(DataTable *xDF, Base4D* remoteDB = nil);
		Selection(Base4D* remoteDB, CDB4DBaseContext* inContext);
		
		void use(void) { nbuse++; };
		void unuse(void) { nbuse--; };
		uBOOL isused(void) { return(nbuse>0); };
		void Dispose(void);
		virtual void Kill();
		
		virtual bool MustCheckOccupiedStamp() const
		{
			return false; // use the new algorithm to check if the object is valid while flushing
		}

		virtual bool SaveObj(VSize& outSizeSaved);
		virtual void setmodif(uBOOL xismodif, Base4D* bd, BaseTaskInfo* context);

		virtual sLONG GetNextRecord(sLONG fromRecord, sLONG FromIndice);
		virtual sLONG GetPreviousRecord(sLONG fromRecord, sLONG FromIndice);
		virtual sLONG GetLastRecord();

		virtual Boolean IsEmpty();
		virtual sLONG GetQTfic(void);
		virtual sLONG GetFic(sLONG n, Boolean canlock = true) = 0;
		virtual VError PutFic(sLONG n, sLONG r, Boolean canlock = true) = 0;
		VError PutFicWithWafSelection(sLONG n, sLONG r, Bittab* inWafSelbits, WafSelection* outWafSel);
		virtual uBOOL FixFic(sLONG nbb, Boolean ForARemoteReceive = false) = 0;
		virtual VError UpdateFromBittab(Bittab *b) = 0;
		inline DataTable* GetParentFile(void) { return(parentfile); };
		
		inline sLONG GetTypSel(void) { return(typsel); };
		
		virtual sLONG CalcLenOnDisk(void) = 0;
		virtual void SetSelAddr(DataAddr4D addr);
		inline DataAddr4D GetSelAddr(void) { return(getaddr()); };
		virtual VError LoadSel(DataBaseObjectHeader& tag) = 0;
		
		virtual VError AddToBittab(Bittab *b, BaseTaskInfo* context);
		virtual VError AddToSel(sLONG n);
		virtual VError DelFromSel(sLONG n);
		virtual VError DelFromSel(Bittab* b);
		//virtual void OktoSave(BaseTaskInfo* context);

		VError DelFromSelRemote(sLONG n, CDB4DBaseContext* inContext);
		VError DelFromSelRemote(Bittab* b, CDB4DBaseContext* inContext);
		
		virtual Bittab* GenereBittab(BaseTaskInfo* context, VError& err) = 0;
		virtual Bittab* StealBittab(BaseTaskInfo* context, VError& err)
		{
			return GenereBittab(context, err);
		}

		virtual VError FillPosInfoArray(SelPosInfo* tab, sLONG fromindice, sLONG toindice)
		{
			return ThrowBaseError(VE_DB4D_NOTIMPLEMENTED); // fait dans les classe derivees
		}

		virtual VError FillArray(sLONG* tab, sLONG maxelem);
		virtual VError AddSelToSel(sLONG& curfic, sLONG maxnb, Bittab* filtre, Selection* sel);
		virtual VError AddSelToSelWithWafSelection(sLONG& curfic, sLONG maxnb, Bittab* filtre, Selection* sel, Bittab* inWafSelbits, WafSelection* outWafSel);

		virtual void ReduceSel(sLONG nb) { if (nb<qtfic) FixFic(nb); };
		virtual void DupliqueInto(SelectionPtr& newsel) = 0;
		void BaseCopyInto(Selection* newsel); 

		Selection* SortSel(VError& err, SortTab* tabs, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress = nil, Boolean TestUnicite = false, 
								Table* remoteTable = nil, WafSelection* inWafSel = nil, WafSelection* outWafSel = nil);

		Selection* SortSel(VError& err, EntityModel* em, EntityAttributeSortedSelection* sortingAtt, BaseTaskInfo* context, 
								VDB4DProgressIndicator* InProgress = nil, WafSelection* inWafSel = nil, WafSelection* outWafSel = nil);

		virtual Boolean LockRecords(BaseTaskInfo* context = nil) = 0;
		virtual Boolean UnLockRecords(BaseTaskInfo* context = nil, sLONG upto = -2, Boolean RemoveFromContext = true) = 0;

		virtual Selection* AddToSelection(sLONG recID, VError& err) = 0;

		virtual VError FillWith(const void* data, sLONG sizeElem, sLONG nbElem, Boolean ascent, sLONG maxrecords, sLONG startFrom, bool useIndirection, Bittab* inWafSel = nil, WafSelection* outWafSel = nil) = 0;

		inline	uLONG GetModificationCounter() const { return fModificationCounter; };
		virtual void Touch() { fModificationCounter++; };
		inline void SetModificationCounter(uLONG x) { fModificationCounter = x; };

		virtual VError DeleteRecords(BaseTaskInfo* context, Bittab* NotDeletedOnes = nil, Relation* RefIntRel = nil, VDB4DProgressIndicator* InProgress = nil, Table* inRemoteTable = nil, EntityModel* inModel = nil);

		virtual VError GetDistinctValues(Field* cri, DB4DCollectionManager& outCollection, BaseTaskInfo* context, 
											VDB4DProgressIndicator* InProgress, const VCompareOptions& inOptions);

		virtual VError GetDistinctValues(EntityAttribute* att, DB4DCollectionManager& outCollection, BaseTaskInfo* context, 
											VDB4DProgressIndicator* InProgress, const VCompareOptions& inOptions);

		virtual VError QuickGetDistinctValues(Field* cri, QuickDB4DArrayOfValues *outCollection, BaseTaskInfo* context, 
											VDB4DProgressIndicator* InProgress, const VCompareOptions& inOptions);

		virtual Boolean IntersectBittab(Bittab* b, BaseTaskInfo* context, VError& err);

		virtual VError FillWithArray(DB4DCollectionManager& inCollection);
		virtual VError FillArray(DB4DCollectionManager& outCollection);

		virtual void ClearSel() { FixFic(0); };

		virtual sLONG GetRecordPos(sLONG inRecordID, BaseTaskInfo* context) = 0;
		virtual Bittab* GenerateSetFromRange(sLONG inRecordIndex1, sLONG inRecordIndex2, VError& err, BaseTaskInfo* context) = 0;
		virtual Bittab* GenerateSetFromRecordID(sLONG inRecordID, sLONG inRecordIndex2, VError& err, BaseTaskInfo* context) = 0;

		virtual VError RemoveSelectedRange(sLONG inRecordIndex1, sLONG inRecordIndex2, BaseTaskInfo* context) = 0;

		VError RemoveSelectedRangeRemote(sLONG inRecordIndex1, sLONG inRecordIndex2, CDB4DBaseContext* inContext);

		virtual VError TransformIntoCluster(DataAddr4D addr) = 0;

		VError ThrowError( VError inErrCode, ActionDB4D inAction) const;

		inline Boolean IsCluster() const { return SelisCluster; };
		
		Boolean IsAllRecords(BaseTaskInfo* context);

		inline Boolean IsRemote() const { return fIsRemote; };

		virtual Boolean MustBeKeptOnServer() { return true; };
		virtual VUUIDBuffer* GenerateID(Boolean forcenew = true) { return nil; };
		virtual VUUIDBuffer* GetID() { return nil; };

		virtual VError FromServer(VStream* from, CDB4DBaseContext* inContext) = 0;
		virtual VError ToClient(VStream* into, CDB4DBaseContext* inContext) = 0;
		virtual VError FromClient(VStream* from, CDB4DBaseContext* inContext) = 0;
		virtual VError ToServer(VStream* into, CDB4DBaseContext* inContext) = 0;

		virtual VError GetPartReply(sLONG numpart, IRequestReply *inRequest)
		{
			assert(false);
			return VE_DB4D_NOTIMPLEMENTED;
		}

		Boolean IsOwnerRemote() const;

		inline Boolean IsRemoteLike() const { return fIsRemote || IsOwnerRemote(); };

		virtual void SetCompte(sLONG count)
		{
			qtfic = count;
		}

		virtual void ClearRemoteCache(VStream* fromServer)
		{
			;
		}

		virtual void PutClearCacheInfo(VStream* toClient)
		{
			;
		}

		/*
		virtual CDB4DBaseContext* GetRemoteContext() const
		{
			return nil;
		}
		*/

		inline Base4D* GetRemoteOwner()
		{
			return db;
		}

		Base4D* GetDB();

		inline void SetSelAsVirtualForCache(FieldsForCache* inRequestedFieldsMaster)
		{
			fRequestedFieldsMaster = inRequestedFieldsMaster;
		}

		virtual void CheckFieldsForCache();

		VError ExportToSQL(CDB4DSelection* xsel, Table* target, BaseTaskInfo* context, VFolder* inBaseFolder, VProgressIndicator* inProgress, ExportOption& options);

		virtual bool MatchAllocationNumber(sLONG allocationNumber) = 0;

		void SetQueryPlan(VValueBag* queryplan)
		{
			CopyRefCountable(&fQueryPlan, queryplan);
		}

		void SetQueryPath(VValueBag* querypath)
		{
			CopyRefCountable(&fQueryPath, querypath);
		}

		inline VValueBag* GetQueryPlan()
		{
			return fQueryPlan;
		}

		inline VValueBag* GetQueryPath()
		{
			return fQueryPath;
		}

		virtual VError FillBittabWithWafSel(WafSelection* inWafSel, Bittab& outWafSelBits);

		/*
		inline void ForbidWrite()
		{
			if (fStopWrite == 0)
				VInterlocked::Increment(&fStopWrite);
		}
		*/


	protected:

		virtual ~Selection()
		{
			QuickReleaseRefCountable(fQueryPath);
			QuickReleaseRefCountable(fQueryPlan);
		}

		virtual	void DoOnRefCountZero () // from IRefCountable
		{
			if (fIsRemote)
			{
				VDBMgr::GetManager()->AddReleasedSelID(*(GetID()));
			}			
			IRefCountable::DoOnRefCountZero();
		}

		void xInit();
		VError SubSort( sLONG l, sLONG r, SortContext& tri);
		uBOOL issuptri( sLONG l, sLONG r, VError *err, SortContext& tri);
		void DeChargeTri(SortElem &se);
		VError ChargeTri(SortElem &se, sLONG n, SortContext& tri);
		void TryToLoadIntoMem(SortContext& tri);
		void FreeSortMem(SortContext& tri);

		template <class Type> sLONG SizeOfType(Field* cri);
		template <class Type> void FreeAfter(TypeSortElemArray<Type>& tempsort, sLONG lenmax);

		Boolean TryToSortTime(VError& err, Field* cri, Boolean ascent, BaseTaskInfo* context, 
													VDB4DProgressIndicator* InProgress, Selection* &into, sLONG typ, Boolean TestUnicite, Bittab* inWafSelbits = nil, WafSelection* outWafSel = nil);
		Boolean TryToSortuLong8(VError& err, Field* cri, Boolean ascent, BaseTaskInfo* context, 
													VDB4DProgressIndicator* InProgress, Selection* &into, sLONG typ, Boolean TestUnicite, Bittab* inWafSelbits = nil, WafSelection* outWafSel = nil);
		Boolean TryToSortLong8(VError& err, Field* cri, Boolean ascent, BaseTaskInfo* context, 
													VDB4DProgressIndicator* InProgress, Selection* &into, sLONG typ, Boolean TestUnicite, Bittab* inWafSelbits = nil, WafSelection* outWafSel = nil);
		Boolean TryToSortLong(VError& err, Field* cri, Boolean ascent, BaseTaskInfo* context, 
													VDB4DProgressIndicator* InProgress, Selection* &into, sLONG typ, Boolean TestUnicite, Bittab* inWafSelbits = nil, WafSelection* outWafSel = nil);
		Boolean TryToSortShort(VError& err, Field* cri, Boolean ascent, BaseTaskInfo* context, 
													VDB4DProgressIndicator* InProgress, Selection* &into, sLONG typ, Boolean TestUnicite, Bittab* inWafSelbits = nil, WafSelection* outWafSel = nil);
		Boolean TryToSortBoolean(VError& err, Field* cri, Boolean ascent, BaseTaskInfo* context, 
													VDB4DProgressIndicator* InProgress, Selection* &into, sLONG typ, Boolean TestUnicite, Bittab* inWafSelbits = nil, WafSelection* outWafSel = nil);
		Boolean TryToSortByte(VError& err, Field* cri, Boolean ascent, BaseTaskInfo* context, 
													VDB4DProgressIndicator* InProgress, Selection* &into, sLONG typ, Boolean TestUnicite, Bittab* inWafSelbits = nil, WafSelection* outWafSel = nil);
		Boolean TryToSortReal(VError& err, Field* cri, Boolean ascent, BaseTaskInfo* context, 
													VDB4DProgressIndicator* InProgress, Selection* &into, sLONG typ, Boolean TestUnicite, Bittab* inWafSelbits = nil, WafSelection* outWafSel = nil);
		Boolean TryToSortAlpha(VError& err, Field* cri, Boolean ascent, BaseTaskInfo* context, 
													VDB4DProgressIndicator* InProgress, Selection* &into, sLONG typ, Boolean TestUnicite, Bittab* inWafSelbits = nil, WafSelection* outWafSel = nil);
		Boolean TryToSortAlphaUTF8(VError& err, Field* cri, Boolean ascent, BaseTaskInfo* context, 
													VDB4DProgressIndicator* InProgress, Selection* &into, sLONG typ, Boolean TestUnicite, Bittab* inWafSelbits = nil, WafSelection* outWafSel = nil);
		Boolean TryToSortUUID(VError& err, Field* cri, Boolean ascent, BaseTaskInfo* context, 
													VDB4DProgressIndicator* InProgress, Selection* &into, sLONG typ, Boolean TestUnicite, Bittab* inWafSelbits = nil, WafSelection* outWafSel = nil);

		Boolean TryToSortMulti(VError& err, const SortTab* tabs, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, Selection* &into, Boolean TestUnicite, Table* remoteTable = nil, Bittab* inWafSelbits = nil, WafSelection* outWafSel = nil);


		Boolean TryToFastGetDistinctValues(VError& err, Field* cri, DB4DCollectionManager& outCollection, BaseTaskInfo* context, 
												VDB4DProgressIndicator* InProgress, const VCompareOptions& inOptions);

		Boolean TryToFastGetDistinctValuesAlpha(VError& err, Field* cri, DB4DCollectionManager& outCollection, BaseTaskInfo* context, 
												VDB4DProgressIndicator* InProgress, sLONG typ, const VCompareOptions& inOptions);

		Boolean TryToFastGetDistinctValuesAlphaUTF8(VError& err, Field* cri, DB4DCollectionManager& outCollection, BaseTaskInfo* context, 
												VDB4DProgressIndicator* InProgress, sLONG typ, const VCompareOptions& inOptions);

		Boolean TryToFastGetDistinctValuesTime(VError& err, Field* cri, DB4DCollectionManager& outCollection, BaseTaskInfo* context, 
												VDB4DProgressIndicator* InProgress, sLONG typ, const VCompareOptions& inOptions);

		Boolean TryToFastGetDistinctValuesLong8(VError& err, Field* cri, DB4DCollectionManager& outCollection, BaseTaskInfo* context, 
												VDB4DProgressIndicator* InProgress, sLONG typ, const VCompareOptions& inOptions);

		Boolean TryToFastGetDistinctValuesLong(VError& err, Field* cri, DB4DCollectionManager& outCollection, BaseTaskInfo* context, 
												VDB4DProgressIndicator* InProgress, sLONG typ, const VCompareOptions& inOptions);

		Boolean TryToFastGetDistinctValuesShort(VError& err, Field* cri, DB4DCollectionManager& outCollection, BaseTaskInfo* context, 
												VDB4DProgressIndicator* InProgress, sLONG typ, const VCompareOptions& inOptions);

		Boolean TryToFastGetDistinctValuesBoolean(VError& err, Field* cri, DB4DCollectionManager& outCollection, BaseTaskInfo* context, 
												VDB4DProgressIndicator* InProgress, sLONG typ, const VCompareOptions& inOptions);

		Boolean TryToFastGetDistinctValuesByte(VError& err, Field* cri, DB4DCollectionManager& outCollection, BaseTaskInfo* context, 
												VDB4DProgressIndicator* InProgress, sLONG typ, const VCompareOptions& inOptions);

		Boolean TryToFastGetDistinctValuesReal(VError& err, Field* cri, DB4DCollectionManager& outCollection, BaseTaskInfo* context, 
												VDB4DProgressIndicator* InProgress, sLONG typ, const VCompareOptions& inOptions);

		Boolean TryToFastGetDistinctValuesUUID(VError& err, Field* cri, DB4DCollectionManager& outCollection, BaseTaskInfo* context, 
											   VDB4DProgressIndicator* InProgress, sLONG typ, const VCompareOptions& inOptions);


		Boolean TryToFastGetDistinctValues(VError& err, EntityAttribute* att, DB4DCollectionManager& outCollection, BaseTaskInfo* context, 
			VDB4DProgressIndicator* InProgress, const VCompareOptions& inOptions);

		Boolean TryToFastGetDistinctValuesAlpha(VError& err, EntityAttribute* att, DB4DCollectionManager& outCollection, BaseTaskInfo* context, 
			VDB4DProgressIndicator* InProgress, sLONG typ, const VCompareOptions& inOptions);

		Boolean TryToFastGetDistinctValuesAlphaUTF8(VError& err, EntityAttribute* att, DB4DCollectionManager& outCollection, BaseTaskInfo* context, 
			VDB4DProgressIndicator* InProgress, sLONG typ, const VCompareOptions& inOptions);

		Boolean TryToFastGetDistinctValuesTime(VError& err, EntityAttribute* att, DB4DCollectionManager& outCollection, BaseTaskInfo* context, 
			VDB4DProgressIndicator* InProgress, sLONG typ, const VCompareOptions& inOptions);

		Boolean TryToFastGetDistinctValuesLong8(VError& err, EntityAttribute* att, DB4DCollectionManager& outCollection, BaseTaskInfo* context, 
			VDB4DProgressIndicator* InProgress, sLONG typ, const VCompareOptions& inOptions);

		Boolean TryToFastGetDistinctValuesLong(VError& err, EntityAttribute* att, DB4DCollectionManager& outCollection, BaseTaskInfo* context, 
			VDB4DProgressIndicator* InProgress, sLONG typ, const VCompareOptions& inOptions);

		Boolean TryToFastGetDistinctValuesShort(VError& err, EntityAttribute* att, DB4DCollectionManager& outCollection, BaseTaskInfo* context, 
			VDB4DProgressIndicator* InProgress, sLONG typ, const VCompareOptions& inOptions);

		Boolean TryToFastGetDistinctValuesBoolean(VError& err, EntityAttribute* att, DB4DCollectionManager& outCollection, BaseTaskInfo* context, 
			VDB4DProgressIndicator* InProgress, sLONG typ, const VCompareOptions& inOptions);

		Boolean TryToFastGetDistinctValuesByte(VError& err, EntityAttribute* att, DB4DCollectionManager& outCollection, BaseTaskInfo* context, 
			VDB4DProgressIndicator* InProgress, sLONG typ, const VCompareOptions& inOptions);

		Boolean TryToFastGetDistinctValuesReal(VError& err, EntityAttribute* att, DB4DCollectionManager& outCollection, BaseTaskInfo* context, 
			VDB4DProgressIndicator* InProgress, sLONG typ, const VCompareOptions& inOptions);

		Boolean TryToFastGetDistinctValuesUUID(VError& err, EntityAttribute* att, DB4DCollectionManager& outCollection, BaseTaskInfo* context, 
			VDB4DProgressIndicator* InProgress, sLONG typ, const VCompareOptions& inOptions);




		template <class Type>
		Boolean TryToFastGetDistinctValuesScalar(VError& err, Field* cri, QuickDB4DArrayOfValues *outCollection, BaseTaskInfo* context, 
												VDB4DProgressIndicator* InProgress, sLONG typ, const VCompareOptions& inOptions);

		Boolean TryToFastQuickGetDistinctValues(VError& err, Field* cri, QuickDB4DArrayOfValues *outCollection, BaseTaskInfo* context, 
												VDB4DProgressIndicator* InProgress, const VCompareOptions& inOptions);


		DataTable *parentfile;
		Base4D *db;
		sLONG qtfic;
		sLONG typsel;
		sLONG antelen;
		sLONG nbuse;
		//sLONG fStopWrite;
		uLONG fModificationCounter;
		Boolean SelisCluster;
		Boolean fKeepRecordLocked;
		Boolean fIsRemote;
		BaseTaskInfo* fLockingContext;
		FieldsForCache* fRequestedFieldsMaster;
		VValueBag* fQueryPlan;
		VValueBag* fQueryPath;


};


class PetiteSel : public Selection
{
	public:
		PetiteSel(DataTable *xDF, Base4D* remoteDB = nil, sLONG numtableRemote = 0);
		PetiteSel(Base4D* remoteDB, CDB4DBaseContext* inContext, sLONG numtableRemote = 0);
		virtual ~PetiteSel();
		virtual bool SaveObj(VSize& outSizeSaved);
		virtual void setmodif(uBOOL xismodif, Base4D* bd, BaseTaskInfo* context);
		virtual sLONG GetFic(sLONG n, Boolean canlock = true);
		virtual VError PutFic(sLONG n, sLONG r, Boolean canlock = true);
		virtual uBOOL FixFic(sLONG nbb, Boolean ForARemoteReceive = false);
		virtual VError UpdateFromBittab(Bittab *b);
		virtual sLONG CalcLenOnDisk(void) ;
		virtual VError LoadSel(DataBaseObjectHeader& tag);
		sLONG FindInSel(sLONG n);
		virtual VError AddToBittab(Bittab *b, BaseTaskInfo* context);
		virtual VError AddToSel(sLONG n);
		virtual VError DelFromSel(sLONG n);
		virtual VError DelFromSel(Bittab* b);
		//virtual void OktoSave(BaseTaskInfo* context);
		virtual Bittab* GenereBittab(BaseTaskInfo* context, VError& err);
		virtual VError LibereEspaceDisk(VDB4DProgressIndicator* InProgress);
		VError PutIntoBittab(Bittab *bb);
		inline sLONG* GetArrayPtr(void) { return fTabMem; };
		inline void ReleaseArrayPtr(void) { ; };
		virtual void DupliqueInto(SelectionPtr& newsel);

		virtual Boolean LockRecords(BaseTaskInfo* context = nil);
		virtual Boolean UnLockRecords(BaseTaskInfo* context = nil, sLONG upto = -2, Boolean RemoveFromContext = true);
		virtual Selection* AddToSelection(sLONG recID, VError& err);

		virtual VError FillWith(const void* data, sLONG sizeElem, sLONG nbElem, Boolean ascent, sLONG maxrecords, sLONG startFrom, bool useIndirection, Bittab* inWafSel = nil, WafSelection* outWafSel = nil);
		virtual VError FillArray(sLONG* tab, sLONG maxelem);

		virtual VError FillPosInfoArray(SelPosInfo* tab, sLONG fromindice, sLONG toindice);

		virtual Boolean IntersectBittab(Bittab* b, BaseTaskInfo* context, VError& err);

		VError BuildWithOneRec(sLONG inRecNum);

		virtual sLONG GetRecordPos(sLONG inRecordID, BaseTaskInfo* context);
		virtual Bittab* GenerateSetFromRange(sLONG inRecordIndex1, sLONG inRecordIndex2, VError& err, BaseTaskInfo* context);
		virtual Bittab* GenerateSetFromRecordID(sLONG inRecordID, sLONG inRecordIndex2, VError& err, BaseTaskInfo* context);

		virtual VError RemoveSelectedRange(sLONG inRecordIndex1, sLONG inRecordIndex2, BaseTaskInfo* context);

		inline sLONG* GetTabMem() { return fTabMem; };
		inline const sLONG* GetTabMem() const { return fTabMem; };

		virtual VError TransformIntoCluster(DataAddr4D addr)
		{
			SetSelAddr(addr);
			return VE_OK;
		};

		virtual Boolean MustBeKeptOnServer() { return false; };
		virtual VError FromServer(VStream* from, CDB4DBaseContext* inContext);
		virtual VError ToClient(VStream* into, CDB4DBaseContext* inContext);
		virtual VError FromClient(VStream* from, CDB4DBaseContext* inContext);
		virtual VError ToServer(VStream* into, CDB4DBaseContext* inContext);

		virtual void CheckFieldsForCache();

		virtual bool MatchAllocationNumber(sLONG allocationNumber);

	protected:
		sLONG* fTabMem;
		sLONG fNumTableRemote;
		sLONG fQTFicToWrite;
};




class ConstVectorSel : public Selection
{
	public:
		ConstVectorSel(DataTable *xDF, const vector<RecIDType>& inVector):Selection(xDF, nil),fVector(inVector)
		{
			qtfic = (sLONG)inVector.size();
			typsel=sel_constvector;
			fIsRemote = false;
		}

#if debuglr
		virtual ~ConstVectorSel()
		{
		}
#endif

		virtual sLONG liberemem(sLONG allocationBlockNumber, sLONG combien=-1, uBOOL tout=false)
		{
			return 0;
		}

		virtual bool SaveObj(VSize& outSizeSaved)
		{
			assert(false);
			outSizeSaved = 0;
			return true;
		}

		virtual sLONG GetFic(sLONG n, Boolean canlock = true)
		{
			return fVector[n];
		}

		virtual VError PutFic(sLONG n, sLONG r, Boolean canlock = true)
		{
			assert(false);
			return VE_DB4D_INDICE_OUT_OF_RANGE;
		}

		virtual uBOOL FixFic(sLONG nbb, Boolean ForARemoteReceive = false)
		{
			assert(false);
			return false;
		}


		virtual VError UpdateFromBittab(Bittab *b)
		{
			assert(false);
			return VE_DB4D_INDICE_OUT_OF_RANGE;
		}

		virtual sLONG CalcLenOnDisk(void)
		{
			assert(false);
			return 0;
		}

		virtual VError LoadSel(DataBaseObjectHeader& tag)
		{
			assert(false);
			return VE_DB4D_INDICE_OUT_OF_RANGE;
		}


		virtual VError AddToBittab(Bittab *b, BaseTaskInfo* context);

		virtual VError AddToSel(sLONG n)
		{
			assert(false);
			return VE_DB4D_INDICE_OUT_OF_RANGE;
		}

		virtual VError DelFromSel(sLONG n)
		{
			assert(false);
			return VE_DB4D_INDICE_OUT_OF_RANGE;
		}

		virtual VError DelFromSel(Bittab* b)
		{
			assert(false);
			return VE_DB4D_INDICE_OUT_OF_RANGE;
		}

		/*
		virtual void OktoSave(BaseTaskInfo* context)
		{
			;
		}
		*/

		sLONG FindInSel(sLONG n);
		VError PutIntoBittab(Bittab *bb);

		virtual Bittab* GenereBittab(BaseTaskInfo* context, VError& err);

		virtual VError LibereEspaceDisk(VDB4DProgressIndicator* InProgress)
		{
			assert(false);
			return VE_DB4D_INDICE_OUT_OF_RANGE;
		}

		virtual void DupliqueInto(SelectionPtr& newsel)
		{
			assert(false);
			newsel = nil;
		}

		virtual Boolean LockRecords(BaseTaskInfo* context = nil);
		virtual Boolean UnLockRecords(BaseTaskInfo* context = nil, sLONG upto = -2, Boolean RemoveFromContext = true);

		virtual Selection* AddToSelection(sLONG recID, VError& err)
		{
			assert(false);
			err = VE_DB4D_INDICE_OUT_OF_RANGE;
			return nil;
		}


		virtual VError FillWith(const void* data, sLONG sizeElem, sLONG nbElem, Boolean ascent, sLONG maxrecords, sLONG startFrom, bool useIndirection, Bittab* inWafSel = nil, WafSelection* outWafSel = nil)
		{
			assert(false);
			return VE_DB4D_NOTIMPLEMENTED;
		}


		virtual VError FillArray(sLONG* tab, sLONG maxelem);

		virtual VError FillPosInfoArray(SelPosInfo* tab, sLONG fromindice, sLONG toindice);

		virtual Boolean IntersectBittab(Bittab* b, BaseTaskInfo* context, VError& err);

		virtual sLONG GetRecordPos(sLONG inRecordID, BaseTaskInfo* context);
		virtual Bittab* GenerateSetFromRange(sLONG inRecordIndex1, sLONG inRecordIndex2, VError& err, BaseTaskInfo* context);
		virtual Bittab* GenerateSetFromRecordID(sLONG inRecordID, sLONG inRecordIndex2, VError& err, BaseTaskInfo* context);

		virtual VError RemoveSelectedRange(sLONG inRecordIndex1, sLONG inRecordIndex2, BaseTaskInfo* context)
		{
			assert(false);
			return VE_DB4D_INDICE_OUT_OF_RANGE;
		}

		virtual VError TransformIntoCluster(DataAddr4D addr)
		{
			assert(false);
			return VE_DB4D_INDICE_OUT_OF_RANGE;
		}

		virtual Boolean MustBeKeptOnServer() 
		{ 
			return false; 
		}

		virtual VError FromServer(VStream* from, CDB4DBaseContext* inContext)
		{
			assert(false);
			return VE_DB4D_INDICE_OUT_OF_RANGE;
		}

		virtual VError ToClient(VStream* into, CDB4DBaseContext* inContext)
		{
			assert(false);
			return VE_DB4D_INDICE_OUT_OF_RANGE;
		}

		virtual VError FromClient(VStream* from, CDB4DBaseContext* inContext)
		{
			assert(false);
			return VE_DB4D_INDICE_OUT_OF_RANGE;
		}

		virtual VError ToServer(VStream* into, CDB4DBaseContext* inContext)
		{
			assert(false);
			return VE_DB4D_INDICE_OUT_OF_RANGE;
		}

		virtual bool MatchAllocationNumber(sLONG allocationNumber)
		{
			return false;
		}



	protected:
		const vector<RecIDType>& fVector;
};



const sLONG kNbElemInSel = 8*1024;
const sLONG kRatioSel = 13;
const sLONG kSizePageSel = (kNbElemInSel+1)*4;
const sLONG kSizePageSelDisk = kSizePageSel-4;

typedef sLONG **TabSelHandle;


class LongSel : public Selection, public IObjToFree
{
	friend class Bittab;
	
	public:
		LongSel(DataTable *xDF, Base4D* remoteDB = nil);
		LongSel(Base4D* remoteDB, CDB4DBaseContext* inContext);
		virtual ~LongSel();
		virtual bool SaveObj(VSize& outSizeSaved);
		void DeleteSel(void);
		sLONG* loadMem(sLONG n1, Boolean canCreate = false, Boolean canlock = true);

		inline Boolean isMemModified(sLONG val) const { return (val & 1) == 1; };
		inline Boolean isMemRetained(sLONG val) const { return (val & 2) == 2; };

		inline Boolean isMemModified(sLONG* p) const { return (p[kNbElemInSel] & 1) == 1; };
		inline Boolean isMemRetained(sLONG* p) const { return (p[kNbElemInSel] & 2) == 2; };
		inline void ModifMem(sLONG* p) { p[kNbElemInSel] = p[kNbElemInSel] | 1; };
		inline void RetainMem(sLONG* p) { p[kNbElemInSel] = p[kNbElemInSel] | 2; };
		inline void ReleaseMem(sLONG* p) {  p[kNbElemInSel] = p[kNbElemInSel] & (-3); };

		virtual bool FreeMem(sLONG allocationBlockNumber, VSize combien, VSize& outSizeFreed);

		uBOOL OpenSelFile(uBOOL doiscreate=true);
		virtual sLONG GetFic(sLONG n, Boolean canlock = true);
		virtual VError PutFic(sLONG n, sLONG r, Boolean canlock = true);
		virtual uBOOL FixFic(sLONG nbb, Boolean ForARemoteReceive = false);
		virtual VError UpdateFromBittab(Bittab *b);
		virtual sLONG CalcLenOnDisk(void);
		virtual VError LoadSel(DataBaseObjectHeader& tag);
		virtual Bittab* GenereBittab(BaseTaskInfo* context, VError& err);
		virtual VError AddToBittab(Bittab *b, BaseTaskInfo* context);
		virtual void DupliqueInto(SelectionPtr& newsel);

		virtual Boolean LockRecords(BaseTaskInfo* context = nil);
		virtual Boolean UnLockRecords(BaseTaskInfo* context = nil, sLONG upto = -2, Boolean RemoveFromContext = true);
		virtual Selection* AddToSelection(sLONG recID, VError& err);
		virtual VError DelFromSel(sLONG n);
		virtual VError DelFromSel(Bittab* b);

		virtual VError FillWith(const void* data, sLONG sizeElem, sLONG nbElem, Boolean ascent, sLONG maxrecords, sLONG startFrom, bool useIndirection, Bittab* inWafSel = nil, WafSelection* outWafSel = nil);
		virtual VError FillArray(sLONG* tab, sLONG maxelem);

		virtual VError FillPosInfoArray(SelPosInfo* tab, sLONG fromindice, sLONG toindice);

		virtual Boolean IntersectBittab(Bittab* b, BaseTaskInfo* context, VError& err);

		virtual sLONG GetRecordPos(sLONG inRecordID, BaseTaskInfo* context);
		virtual Bittab* GenerateSetFromRange(sLONG inRecordIndex1, sLONG inRecordIndex2, VError& err, BaseTaskInfo* context);
		virtual Bittab* GenerateSetFromRecordID(sLONG inRecordID, sLONG inRecordIndex2, VError& err, BaseTaskInfo* context);

		virtual VError RemoveSelectedRange(sLONG inRecordIndex1, sLONG inRecordIndex2, BaseTaskInfo* context);

		virtual VError TransformIntoCluster(DataAddr4D addr)
		{
			return VE_DB4D_NOTIMPLEMENTED;
		};

		virtual VUUIDBuffer* GenerateID(Boolean forcenew = true) 
		{ 
			if (forcenew || !fIDHasBeenGenerated)
			{
				VUUID xid(true);
				xid.ToBuffer(fID);
				fIDHasBeenGenerated = true;
			}
			return &fID; 
		};

		virtual VUUIDBuffer* GetID()
		{
			return &fID;
		}

		virtual VError FromServer(VStream* from, CDB4DBaseContext* inContext);
		virtual VError ToClient(VStream* into, CDB4DBaseContext* inContext);
		virtual VError FromClient(VStream* from, CDB4DBaseContext* inContext);
		virtual VError ToServer(VStream* into, CDB4DBaseContext* inContext);
		virtual VError GetPartReply(sLONG numpart, IRequestReply *inRequest);

		inline VUUIDBuffer* xxGetId()
		{
			return &fID;
		}

		virtual void ClearRemoteCache(VStream* fromServer);

		/*
		virtual CDB4DBaseContext* GetRemoteContext() const
		{
			return fRemoteContext;
		}
		*/

		virtual void SetCompte(sLONG count)
		{
			FixFic(count, true);
		}

		inline void SetCurLockPage(sLONG n)
		{
			fCurLockPage = n;
		}

		inline void ClearCurLockPage()
		{
			fCurLockPage = -1;
		}

		inline void LockAllPages()
		{
			fCurLockPage = -2;
		}

		virtual bool MatchAllocationNumber(sLONG allocationNumber);


	protected:

		VError xDelFromSel(Bittab* b, sLONG n);

		VUUIDBuffer fID;
		sLONG fCurLockPage;
		//CDB4DBaseContext* fRemoteContext;
		sLONG* *fTabMem;
		VFile *fFile;
		VFileDesc *fFileDesc;
		Boolean fIDHasBeenGenerated;
};

#if debugLeakCheck_Strong
extern uBOOL debug_candumpleaks;
extern uBOOL debug_canRegisterLeaks;
#endif


class BitSel : public Selection
{
	public:
		inline BitSel(DataTable *xDF, Base4D* remoteDB = nil) : Selection(xDF, remoteDB) 
		{ 
			recalcnbfic=true; typsel=sel_bitsel; pTB = &Owned_TB;
			pTB->SetAsSel();
			pTB->SetOwner(remoteDB);
			pTB->Retain();

#if debugLeakCheck_BitSel
			if (debug_candumpleaks)
				DumpStackCrawls();
			if (debug_canRegisterLeaks)
				RegisterStackCrawl(this);
#endif
		};

		inline BitSel(Base4D* remoteDB, CDB4DBaseContext* inContext) : Selection(remoteDB, inContext) 
		{ 
			recalcnbfic=true; typsel=sel_bitsel; pTB = &Owned_TB;
			pTB->SetAsSel();
			pTB->Retain();
			pTB->SetOwner(remoteDB);
			/*
			if ((inContext != (CDB4DBaseContext*)-1))
				pTB->SetRemoteContext(inContext);
				*/
#if debugLeakCheck_BitSel
			if (debug_candumpleaks)
				DumpStackCrawls();
			if (debug_canRegisterLeaks)
				RegisterStackCrawl(this);
#endif
		};

		inline BitSel(DataTable *xDF, Bittab* inBT, Base4D* remoteDB = nil) : Selection(xDF, remoteDB) 
		{ 
			assert(inBT != nil);
			recalcnbfic=true; 
			typsel=sel_bitsel; 
			pTB = inBT;
			pTB->Retain();
			pTB->SetAsSel();
			pTB->SetOwner(remoteDB);

#if debugLeakCheck_BitSel
			if (debug_candumpleaks)
				DumpStackCrawls();
			if (debug_canRegisterLeaks)
				RegisterStackCrawl(this);
#endif
		};

		virtual ~BitSel()
		{
#if debugLeakCheck_BitSel
			UnRegisterStackCrawl(this);
#endif
			if (pTB != nil)
				pTB->Release();
		};

		virtual void setmodif(uBOOL xismodif, Base4D* bd, BaseTaskInfo* context);
		virtual void ChangeAddr(DataAddr4D addr, Base4D* bd, BaseTaskInfo* context);
		virtual bool SaveObj(VSize& outSizeSaved);
		virtual sLONG GetNextRecord(sLONG fromRecord, sLONG FromIndice);
		virtual sLONG GetPreviousRecord(sLONG fromRecord, sLONG FromIndice);
		virtual sLONG GetLastRecord();
		virtual uBOOL FixFic(sLONG nbb, Boolean ForARemoteReceive = false);
		virtual sLONG GetFic(sLONG n, Boolean canlock = true);
		virtual VError PutFic(sLONG n, sLONG r, Boolean canlock = true);
		virtual sLONG GetQTfic(void);
		virtual Boolean IsEmpty();
		inline Bittab* GetTB(void) { return(pTB); };
		virtual VError UpdateFromBittab(Bittab *b);
		virtual sLONG CalcLenOnDisk(void);
		virtual void SetSelAddr(DataAddr4D addr);
		virtual VError LoadSel(DataBaseObjectHeader& tag);
		virtual VError AddToBittab(Bittab *b, BaseTaskInfo* context);
		virtual VError AddToSel(sLONG n);
		virtual VError DelFromSel(sLONG n);
		virtual VError DelFromSel(Bittab* b);
		//virtual void OktoSave(BaseTaskInfo* context);
		virtual Bittab* GenereBittab(BaseTaskInfo* context, VError& err);
		virtual Bittab* StealBittab(BaseTaskInfo* context, VError& err);
		virtual VError LibereEspaceDisk(VDB4DProgressIndicator* InProgress);
		inline Bittab* GetBittab(void) { return(pTB); };
		virtual VError FillArray(sLONG* tab, sLONG maxelem);
		virtual VError AddSelToSel(sLONG& curfic, sLONG maxnb, Bittab* filtre, Selection* sel);
		virtual void DupliqueInto(SelectionPtr& newsel);

		virtual Boolean LockRecords(BaseTaskInfo* context = nil);
		virtual Boolean UnLockRecords(BaseTaskInfo* context = nil, sLONG upto = -2, Boolean RemoveFromContext = true);
		virtual Selection* AddToSelection(sLONG recID, VError& err);

		virtual VError FillWith(const void* data, sLONG sizeElem, sLONG nbElem, Boolean ascent, sLONG maxrecords, sLONG startFrom, bool useIndirection, Bittab* inWafSel = nil, WafSelection* outWafSel = nil);

		virtual Boolean IntersectBittab(Bittab* b, BaseTaskInfo* context, VError& err);
		virtual void ReduceSel(sLONG nb);

		virtual sLONG GetRecordPos(sLONG inRecordID, BaseTaskInfo* context);
		virtual Bittab* GenerateSetFromRange(sLONG inRecordIndex1, sLONG inRecordIndex2, VError& err, BaseTaskInfo* context);
		virtual Bittab* GenerateSetFromRecordID(sLONG inRecordID, sLONG inRecordIndex2, VError& err, BaseTaskInfo* context);

		virtual void Touch() { Selection::Touch(); recalcnbfic = true; };
		virtual VError RemoveSelectedRange(sLONG inRecordIndex1, sLONG inRecordIndex2, BaseTaskInfo* context);

		virtual VError TransformIntoCluster(DataAddr4D addr);

		virtual VUUIDBuffer* GenerateID(Boolean forcenew = true) 
		{ 
			return pTB->GenerateID(forcenew); 
		};

		virtual VUUIDBuffer* GetID() 
		{ 
			return pTB->GetID(); 
		};

		virtual VError FromServer(VStream* from, CDB4DBaseContext* inContext)
		{
			recalcnbfic = true;
			return pTB->FromServer(from, inContext);
		}

		virtual VError ToClient(VStream* into, CDB4DBaseContext* inContext)
		{
			into->PutLong(GetTypSel());
			VError err = pTB->ToClient(into, inContext);
			if (err == VE_OK)
			{
				VDBMgr::GetManager()->KeepSelectionOnServer(this);
			}
			return err;
		}

		virtual VError FromClient(VStream* from, CDB4DBaseContext* inContext)
		{
			recalcnbfic = true;
			return pTB->FromClient(from, inContext);
		}

		virtual VError ToServer(VStream* into, CDB4DBaseContext* inContext)
		{
			if (fIsRemote)
			{
				// into->PutLong(GetTypSel()); // fait dans Bittab::ToServer
				return pTB->ToServer(into, inContext, true);
			}
			else
			{
				fIsRemote = true;
				// into->PutLong(sel_bitsel_fullfromclient); // fait dans Bittab::ToServer
				return pTB->ToServer(into, inContext, true);
			}
		}

		virtual VError GetPartReply(sLONG numpart, IRequestReply *inRequest)
		{
			return pTB->GetPartReply(numpart, inRequest);
		}

		virtual void SetCompte(sLONG count)
		{
			Selection::SetCompte(count);
			recalcnbfic = false;
			pTB->SetCompte(count);
		}

		virtual void ClearRemoteCache(VStream* fromServer)
		{
			pTB->ClearRemoteCache(fromServer);
		}

		virtual void PutClearCacheInfo(VStream* toClient)
		{
			pTB->PutClearCacheInfo(toClient);
		}

		/*
		virtual CDB4DBaseContext* GetRemoteContext() const
		{
			return pTB->GetRemoteContext();
		}
		*/

		virtual bool MatchAllocationNumber(sLONG allocationNumber)
		{
			return pTB->MatchAllocationNumber(allocationNumber, this);
		}


	protected:
		Bittab Owned_TB;
		Bittab *pTB;
		uBOOL recalcnbfic;
};



class SelectionIterator
{
	public:
		SelectionIterator(Selection* sel = nil);
		SelectionIterator(const SelectionIterator& other);

		SelectionIterator& operator =(const SelectionIterator& other);

		void Reset(Selection* sel);

		sLONG PreviousRecord();
		sLONG NextRecord();
		sLONG FirstRecord();
		sLONG LastRecord();
		sLONG SetCurrentRecord(sLONG PosInSel);

	protected:
		sLONG fCurRecord;
		sLONG fCurIndice;
		Selection* fSel;
		Boolean fWasJustReset;
};



//Selection* CreFromBittabAndModifyIt(DataTable *DF, Bittab* &b);
Selection* CreFromBittabPtr(DataTable *DF, Bittab* b, Base4D* remoteDB = nil);
Selection* CreFromBittab(DataTable *DF, Bittab* b, Base4D* remoteDB = nil);
Selection* CreFromBittab_nobitsel(DataTable *DF, Bittab* b, Base4D* remoteDB = nil);


	/* ---------------------------------------------------- */
	
class SelPosIterator
{
	public:
		SelPosIterator()
		{
			fSet = nil;
			fCurPos = 0;
			fPosInfos = nil;
			fMax = 0;
			fSomeSkipped = false;
		}

		~SelPosIterator()
		{
			QuickReleaseRefCountable(fSet);
			if (fPosInfos != nil)
				FreeTempMem(fPosInfos);
		}

		VError InitWithBittab(Bittab* b, sLONG fromindice, sLONG toindice);
		VError InitWithSel(Selection* sel, sLONG fromindice, sLONG toindice);

		void NextPos();

		sLONG GetCurRecNum() const
		{
			return fCurRecNum;
		}

		sLONG GetCurSelPos() const
		{
			return fCurSelPos;
		}

		sLONG GetCurPos() const
		{
			return fCurPos;
		}

		Boolean ReachedEnd()
		{
			return fCurRecNum == -1;
		}

		void SkippedOne()
		{
			fSomeSkipped = true;
		}

		Boolean AllTaken()
		{
			return ReachedEnd() && !fSomeSkipped;
		}

	protected:
		Bittab* fSet;
		SelPosInfo* fPosInfos;

		sLONG fCurPos;
		sLONG fMax;
		sLONG fCurRecNum;
		sLONG fCurSelPos;

		Boolean fSomeSkipped;
		
};


	/* ---------------------------------------------------- */




#if 0

class NamedSelection : public ObjectInList
{
	public:
		NamedSelection(Table *pFic, ObjectInListHeader *rootHeader) : ObjectInList(rootHeader) { sel = nil; permanent = false; };
		virtual ~NamedSelection() { if (sel != nil) sel->Release(); };
		Table* GetFile(void);
		inline void SetSel(Selection *xsel, Boolean DeleteOld = true) { if (sel != nil && DeleteOld) sel->Dispose(); sel = xsel; };
		inline Selection* GetSel(void) { return sel; };
		void SetPermanent(Boolean xperm) { permanent = xperm; };
		Boolean IsPermanent(void) { return(permanent); };
		
	protected:
		Selection *sel;
		Boolean permanent;
};


class NamedSet : public ObjectInList
{
	public:
		NamedSet(Table *pFic, ObjectInListHeader *rootHeader);
		virtual ~NamedSet();
		Table* GetFile(void) { return(fic); };
		sLONG GetCurFic(void) { return(curfic); };
		void SetCurFic(sLONG n) { curfic = n; };
		inline Bittab* GetBittab(void) { return(&b); };
		VError GetFrom(VStream& buf);
		VError PutInto(VStream& buf);
		
		
	protected:
		Table *fic;
		sLONG curfic;
		Bittab	b;
};

#endif


#endif
