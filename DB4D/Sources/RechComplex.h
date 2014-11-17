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
#ifndef __RECHCOMPLEX4D__
#define __RECHCOMPLEX4D__





class CQTableDef
{
	public:
		CQTableDef()
		{
			fTable = nil;
			fNumInstance = 0;
		};

		CQTableDef(const CQTableDef& other)
		{
			fTable = other.fTable;
			if (fTable != nil)
				fTable->Retain();
			fNumInstance = other.fNumInstance;
		}

		CQTableDef(const QueryTarget& other);

		CQTableDef(Table* inTable, sLONG inNumInstance)
		{
			fTable = inTable;
			if (inTable != nil)
				fTable->Retain();
			fNumInstance = inNumInstance;
		};

		~CQTableDef()
		{
			if (fTable != nil)
				fTable->Release();
		};

		CQTableDef& operator = ( const CQTableDef& inOther)
		{
			CopyRefCountable( &fTable, inOther.fTable);
			fNumInstance = inOther.fNumInstance;
			return *this;
		}

		CQTableDef& operator = ( const QueryTarget& inOther);		

		inline bool operator == (const CQTableDef& other) const
		{
			if (other.fTable == fTable)
				return other.fNumInstance == fNumInstance;
			else
				return false;
		};

		inline bool operator < (const CQTableDef& other) const
		{
			if (other.fTable == fTable)
				return fNumInstance < other.fNumInstance;
			else
				return fTable < other.fTable;
		};

		inline sLONG GetInstance() const { return fNumInstance; };
		inline Table* GetTable() const { return fTable; };

	protected:
		Table* fTable;
		sLONG fNumInstance;
};


typedef vector<CQTableDef/*, cache_allocator<CQTableDef>*/ > CQTableDefVector;

class CQTargetsDef : public IRefCountable
{
public:
	Boolean Add(const CQTableDef& target);
	bool MatchAtLeastOne(const CQTargetsDef& other);
	void AddTargets(const CQTargetsDef& other);
	bool AlreadyHas(const CQTableDef& target) const;
	void ReOrder();

	CQTableDefVector fTargets;
};

bool CompareLessCQTableDefVector(CQTargetsDef* x1, CQTargetsDef* x2);


			/* ---------------------------------------------- */

const sLONG kNbElemInComplexSel = 13860; // 4 * 5 * 9 * 7 * 11
const sLONG kSizeComplexSelOnDisk = sizeof(RecIDType) * kNbElemInComplexSel;
const sLONG kSizeComplexSelInMem = sizeof(RecIDType) * (kNbElemInComplexSel+1);

typedef RecIDType ComplexSel[kNbElemInComplexSel+1];

typedef vector<ComplexSel* /*, cache_allocator<ComplexSel*>*/ > ComplexSelVector;

class ComplexSelection : public ObjInCacheMemory, public IRefCountable, public IObjToFree
{
	public:
		ComplexSelection(const CQTableDefVector& inTargets);
		ComplexSelection(const CQTableDefVector& inTargets, bool leftJoin, bool rightJoin);
		ComplexSelection(const CQTableDef& inTarget);
		virtual ~ComplexSelection();

		virtual bool FreeMem(sLONG allocationBlockNumber, VSize combien, VSize& outSizeFreed);

		RecIDType GetRecID(sLONG Column, RecIDType Row, VError& err) const;
		VError GetFullRow(RecIDType Row, ComplexSelRow& outRow) const;

		Bittab* GenereBittab(sLONG column, BaseTaskInfo* context, VError &outErr) const;
		VError FromBittab(sLONG column, BaseTaskInfo* context, Bittab* from, Boolean canStealBittab);

		inline sLONG CalcNbPage() const { return ((fNbRows*fNbCols)+kNbElemInComplexSel-1)/kNbElemInComplexSel; };

		ComplexSelection* MergeSel(ComplexSelection& WithOther, VError& err);
		VError SortByRecIDs(const vector<sLONG>& whichcolums);

		sLONG GetNbRows() const { return fNbRows; };
		sLONG GetNbColumns() const { return fNbCols; };

		RecIDType* calcIterator(RecIDType row, RecIDType* storage) const;
		RecIDType* calcIteratorLock(RecIDType row, ComplexSel* &xLock, void* debugIterator = nil) const;

		VError AddRow(const ComplexSelRow& inRow);
		VError MergeAndAddRow(RecIDType* px1, RecIDType* px2, vector<sLONG>& cfinal, ComplexSelRow& FinalRow);

		sLONG FindTarget(const CQTableDef& inTarget);

		VError GetColumn(sLONG column, CQTableDef& outTarget);

		VError RetainColumns(QueryTargetVector& outColumns);

		VErrorDB4D ToDB4D_ComplexSel(DB4D_ComplexSel& outSel);

		inline const CQTableDefVector* GetTargets() const { return &fTargets; };

		VError ReOrderColumns(const CQTableDefVector& neworder);

		ComplexSelection* CartesianProduct(const ComplexSelection& other, VError& err) const;

		void templock(ComplexSel* p) const
		{
			(*p)[kNbElemInComplexSel] |= 0x20000;
		}

		void tempunlock(ComplexSel* p) const
		{
			(*p)[kNbElemInComplexSel] &= 0xFFFDFFFF;
		}

		void tempmodify(ComplexSel* p) const
		{
			(*p)[kNbElemInComplexSel] |= 0x10000;
		}

		void tempunmodify(ComplexSel* p) const
		{
			(*p)[kNbElemInComplexSel] |= 0xFFFEFFFF;
		}

		void tempretain(ComplexSel* p, void* debugIterator = nil) const
		{
			(*p)[kNbElemInComplexSel]++;
		}

		void temprelease(ComplexSel* p, void* debugIterator = nil) const
		{
			xbox_assert((((*p)[kNbElemInComplexSel]) & 0xFFFF) > 0);
			(*p)[kNbElemInComplexSel]--;
		}


		inline void SetCurLockPage(sLONG n) const
		{
			fCurLockPage = n;
		}

		inline void ClearCurLockPage() const
		{
			fCurLockPage = -1;
			fThreadLockAll = 0;
		}

		void WaitForFreeMemAndLockAll(bool withThreadID) const;


	protected:
		void xinit();
		void dispose();

		inline DataAddr4D CalcPagePos(sLONG numpage) const
		{
			DataAddr4D result = kSizeComplexSelOnDisk;
			result = result * numpage;
			return result;
		}

		bool OpenSelFile();

		ComplexSel* loadmem(sLONG numpage, VError& err, bool retain = false, void* debugIterator = nil) const;


		CQTableDefVector fTargets;
		ComplexSelVector fTabSel;
		vector<bool> fKeepNulls;

		sLONG fNbRows;
		sLONG fNbCols;
		sLONG fNbRowsPerPage;
		mutable sLONG fCurLockPage;
		volatile sLONG fCurPageFree;
		volatile sLONG fFreeMemInAction;
		VFile* fTempFile;
		VFileDesc *fFileDesc;
		DataAddr4D fFileSize;
		mutable VTaskID fThreadLockAll;
};




template <sLONG nbcol>
class ComplexSelectionIterator
{
public:

	class util_class
	{
	public:
		RecIDType storage[nbcol];
	};

	typedef random_access_iterator_tag	iterator_category;
	typedef util_class					value_type;
	typedef ptrdiff_t					difference_type;
	typedef value_type*			pointer;
	typedef value_type&			reference;

	inline ComplexSelectionIterator(ComplexSelection* inSel, RecIDType row)
	{
		fLock = nil;
		sel = inSel;
		ptr = (value_type*)inSel->calcIteratorLock(row, fLock, this);
		currow = row;
	};

	/*

	inline ComplexSelectionIterator(const ComplexSelectionIterator<nbcol>& other)
	{
		fLock = nil;
		sel = other.sel;
		currow = other.currow;
		ptr = (value_type*)sel->calcIteratorLock(currow, fLock);
	}

	inline ComplexSelectionIterator<nbcol>& operator = (const ComplexSelectionIterator<nbcol>& other)
	{
		fLock = nil;
		sel = other.sel;
		currow = other.currow;
		ptr = (value_type*)sel->calcIteratorLock(currow, fLock);
		return *this;
	}
	*/


	inline ComplexSelectionIterator(const ComplexSelectionIterator<nbcol>& other)
	{
		fLock = other.fLock;
		sel = other.sel;
		currow = other.currow;
		ptr = other.ptr;
		if (fLock != nil)
			sel->tempretain(fLock, this);
	}

	inline ComplexSelectionIterator<nbcol>& operator = (const ComplexSelectionIterator<nbcol>& other)
	{
		if (fLock != nil)
			sel->temprelease(fLock, this);
		fLock = other.fLock;
		sel = other.sel;
		currow = other.currow;
		ptr = other.ptr;
		if (fLock != nil)
			sel->tempretain(fLock, this);
		return *this;
	}

	~ComplexSelectionIterator()
	{
		if (fLock != nil)
			sel->temprelease(fLock, this);
	}

	inline value_type& operator *() const
	{
		return (value_type&)*ptr;
	};

	inline value_type* operator ->() const
	{	
		return (value_type*)ptr;
	};

	inline bool operator < (const ComplexSelectionIterator<nbcol>& other) const
	{
		return currow < other.currow;
	};

	inline bool operator <= (const ComplexSelectionIterator<nbcol>& other) const
	{
		return currow <= other.currow;
	};

	inline bool operator > (const ComplexSelectionIterator<nbcol>& other) const
	{
		return currow > other.currow;
	};

	inline bool operator >= (const ComplexSelectionIterator<nbcol>& other) const
	{
		return currow >= other.currow;
	};

	inline bool operator == (const ComplexSelectionIterator<nbcol>& other) const
	{
		return currow == other.currow;
	};

	inline bool operator != (const ComplexSelectionIterator<nbcol>& other) const
	{
		return currow != other.currow;
	};

	inline ComplexSelectionIterator<nbcol>& operator ++()
	{
		currow++;
		ptr = (value_type*)sel->calcIteratorLock(currow, fLock, this);
		return *this;
	};

	inline ComplexSelectionIterator<nbcol> operator ++(int) // post increment
	{
		ComplexSelectionIterator<nbcol> temp = *this;
		currow++;
		ptr = (value_type*)sel->calcIteratorLock(currow, temp.fLock, &temp);
		return temp;
	};

	inline ComplexSelectionIterator<nbcol>& operator --()
	{
		currow--;
		ptr = (value_type*)sel->calcIteratorLock(currow, fLock, this);
		return *this;
	};

	inline ComplexSelectionIterator<nbcol> operator --(int) // post decrement
	{
		ComplexSelectionIterator<nbcol> temp = *this;
		currow--;
		ptr = (value_type*)sel->calcIteratorLock(currow, temp.fLock, &temp);
		return temp;
	};

	inline ComplexSelectionIterator<nbcol>& operator +=(size_t inc)
	{
		currow += (RecIDType)inc;
		ptr = (value_type*)sel->calcIteratorLock(currow, fLock, this);
		return *this;
	};

	inline ComplexSelectionIterator<nbcol> operator +(size_t inc) const
	{
		ComplexSelectionIterator<nbcol> temp = *this;
		temp.currow = currow + (RecIDType)inc;
		temp.ptr = (value_type*)sel->calcIteratorLock(temp.currow, temp.fLock, &temp);
		return temp;
	};

	inline ComplexSelectionIterator<nbcol>& operator -=(size_t inc)
	{
		currow -= (RecIDType)inc;
		ptr = (value_type*)sel->calcIteratorLock(currow, fLock, this);
		return *this;
	};

	inline ComplexSelectionIterator<nbcol> operator -(size_t inc) const
	{
		ComplexSelectionIterator<nbcol> temp = *this;
		temp.currow = currow - (RecIDType)inc;
		temp.ptr = (value_type*)sel->calcIteratorLock(temp.currow, temp.fLock, &temp);
		return temp;
	};

	inline ptrdiff_t operator -(const ComplexSelectionIterator<nbcol>& other) const
	{
		return currow - other.currow;
	};

	inline value_type& operator [](size_t off) const
	{
		return *(value_type*)sel->calcIteratorLock(currow+(RecIDType)off, fLock, this);
	};

	value_type* ptr;
	RecIDType currow;
	ComplexSelection* sel;
	mutable ComplexSel* fLock;
};




template <sLONG nbcol>
class SortComplexSelPredicate
{
public:

	inline SortComplexSelPredicate(const vector<sLONG>& whichColumns) { fColsToComp = whichColumns; };

	bool operator ()(const typename ComplexSelectionIterator<nbcol>::value_type& val1, const typename ComplexSelectionIterator<nbcol>::value_type& val2);

	vector<sLONG> fColsToComp;
};


			// ------------------------------------------------------------



enum { aucuncomplextokenrech=0L, rc_ParentG, rc_ParentD, rc_ComplexSQLScript, rc_Line, rc_Operator, rc_Script, rc_Not, rc_LineArray, rc_LineJoin, rc_Sel, rc_SQLScript, rc_BeginWhere, rc_BeginOn, rc_EndOn };

class ComplexOptimizedQuery;



class ComplexRechToken : public Obj4D, public IObjCounter
{
	public :
		sWORD TypToken;
		inline ComplexRechToken(sWORD inType = aucuncomplextokenrech) { TypToken = inType; };

		sWORD GetTyp(void) const { return(TypToken); };
};



class ComplexRechTokenOper : public ComplexRechToken
{
	public:
		sWORD BoolLogic;

		inline ComplexRechTokenOper(sWORD inBoolOper) { TypToken = rc_Operator; BoolLogic = inBoolOper;};
		inline sWORD GetOper() const { return BoolLogic; };

};



class ComplexRechTokenNot : public ComplexRechToken
{
public:

	inline ComplexRechTokenNot() { TypToken = rc_Not;};

};



class ComplexRechTokenSimpleComp : public ComplexRechToken
{
	public:
		Field* cri;
		sLONG fNumInstance;
		sLONG comparaison;
		ValPtr	ch;
		Boolean fIsDataDirect;
		//Boolean fIsDiacritic;
		VCompareOptions fOptions;
		uBOOL fIsNeverNull;
		uBOOL fCheckForNull;

		ComplexRechTokenSimpleComp(Field* xcri, sLONG instance, sLONG comp, ValPtr val, const VCompareOptions& inOptions);
		virtual ~ComplexRechTokenSimpleComp();
		void CheckIfDataIsDirect();
};



class ComplexRechTokenScriptComp : public ComplexRechToken
{
public:
	DB4DLanguageExpression* expression;
	Table* fTable;
	sLONG fNumInstance;

	ComplexRechTokenScriptComp(DB4DLanguageExpression* inExpression, Table* inTable, sLONG numinstance);
	virtual ~ComplexRechTokenScriptComp();
};



class ComplexRechTokenSQLScriptComp : public ComplexRechToken
{
public:
	DB4DSQLExpression* expression;
	Table* fTable;
	sLONG fNumInstance;
	void* fSQLContext;

	ComplexRechTokenSQLScriptComp(DB4DSQLExpression* inExpression, Table* inTable, sLONG numinstance);
	virtual ~ComplexRechTokenSQLScriptComp();
};




class ComplexRechTokenArrayComp : public ComplexRechToken
{
public:
	Field* cri;
	sLONG fNumInstance;
	sWORD comparaison;
	DB4DArrayOfValues* values;
	Boolean fIsDataDirect;
	//Boolean fIsDiacritic;
	VCompareOptions fOptions;
	uBOOL fIsNeverNull;

	ComplexRechTokenArrayComp(Field* xcri, sLONG instance, sWORD comp, DB4DArrayOfValues* val, const VCompareOptions& inOptions);
	virtual ~ComplexRechTokenArrayComp();

	void CheckIfDataIsDirect();
};



class ComplexRechTokenJoin : public ComplexRechToken
{
public:
	Field* cri1;
	sLONG fNumInstance1;
	sWORD comparaison;
	Field* cri2;
	sLONG fNumInstance2;
	bool fLeftJoin, fRightJoin;
//	Boolean fIsDiacritic;
	VCompareOptions fOptions;

	ComplexRechTokenJoin(Field* xcri1, sLONG numinstance1, sWORD comp, Field* xcri2, sLONG numinstance2, const VCompareOptions& inOptions, bool leftjoin, bool rightjoin);
	virtual ~ComplexRechTokenJoin();
};



				/* ----------------------------------------------------------- */

typedef enum { CQResult_Selection = 1, CQResult_ComplexSel } ComplexQueryResult;

typedef vector<ComplexRechToken* /*, cache_allocator<ComplexRechToken*> */> ComplexRechTokenVector;


class ComplexRech : public Obj4D, public IObjCounter
{
	public:
		ComplexRech(Table* inSimpleTarget); // result is a simple selection
		ComplexRech(Base4D* owner); // if result is a complex selection
		virtual ~ComplexRech();

		void SetSimpleTarget(Table* inSimpleTarget);

		VError AddComplexTarget(Table* inTable, sLONG inNumInstance);
		VError SetComplexTarget(const QueryTargetVector& inTargets);

		VError AddSearchExpression(DB4DLanguageExpression* inExpression, Table* inTable, sLONG inNumInstance);
		VError AddSearchSQLExpression(DB4DSQLExpression* inExpression, Table* inTable, sLONG inNumInstance);

		VError AddSearchSimple(Field* cri, sLONG numinstance, sLONG comp, const VValueSingle* ValueToCompare, Boolean isDiacritic = false);
		VError AddSearchSimple(Field* cri, sLONG numinstance, sLONG comp, const VValueSingle* ValueToCompare, const VCompareOptions& inOptions);

		VError AddSearchArray(Field* cri, sLONG numinstance, sLONG comp, DB4DArrayOfValues *Values, Boolean isDiacritic = false);
		VError AddSearchArray(Field* cri, sLONG numinstance, sLONG comp, DB4DArrayOfValues *Values, const VCompareOptions& inOptions);

		VError AddSearchJoin(Field* cri, sLONG numinstance, sLONG comp, Field* OtherCri, sLONG numOtherInstance, Boolean isdiacritic = false, bool leftjoin = false, bool rightjoin = false);
		VError AddSearchJoin(Field* cri, sLONG numinstance, sLONG comp, Field* OtherCri, sLONG numOtherInstance, const VCompareOptions& inOptions, bool leftjoin = false, bool rightjoin = false);

		VError AddSearchBoolOper(sLONG BoolOper);

		VError AddSearchNotOperator();
		VError AddSearchOpenParenthesis();
		VError AddSearchCloseParenthesis();
		VError AddBeginWhere();
		VError AddBeginOn();
		VError AddEndOn();

		VError AddSearchComplexSQLExpression(DB4DSQLExpression* inExpression, const QueryTargetVector& inTargets);

		ComplexRechToken* GetNextToken();
		void PosToFirstToken() { curToken = fTokens.begin(); };

		inline Boolean isSimpleTarget() const { return fSimpleTarget != nil; };
		inline Table* GetSimpleTarget() const { return fSimpleTarget; };
		inline const CQTargetsDef* GetComplexTarget() const { return &fComplexTarget; };
		inline CQTargetsDef* GetComplexTarget() { return &fComplexTarget; };

		VError BuildFromString(const VString& input);
		VError BuildTargetsFromString(const VString& inTargetsText);

	protected:
		VError AddGenericToken(sWORD whatToken);

		ComplexRechTokenVector fTokens;
		ComplexQueryResult fResultType;
		CQTargetsDef fComplexTarget;
		Table* fSimpleTarget;

		Base4D* fOwner;

		ComplexRechTokenVector::iterator curToken;
};



			/* ----------------------------------------------------------- */



class ComplexRechTokenComplexSQLScriptComp : public ComplexRechToken
{
public:
	DB4DSQLExpression* expression;
	CQTargetsDef fTargets;

	ComplexRechTokenComplexSQLScriptComp(DB4DSQLExpression* inExpression, const QueryTargetVector& inTargets);
	virtual ~ComplexRechTokenComplexSQLScriptComp();
};


			/* ----------------------------------------------------------- */

/*
class QueryDescriptor;

typedef vector<VRefPtr<QueryDescriptor> > QueryDescriptorVector;

class QueryDescriptor : public IRefCountable
{
	public:
		inline void SetText(const VString& inText) { fText = inText; };
		VError AddSubLine();

	protected:
		VString fText;
		QueryDescriptorVector fSubLines;

}
*/


			/* ----------------------------------------------------------- */


typedef enum {  QueryNode_None = 0, 
		QuerySimpleNode_Seq,
		QuerySimpleNode_Index,
		QuerySimpleNode_ArraySeq,
		QuerySimpleNode_ArrayIndex,
		QuerySimpleNode_BoolOper,
		QuerySimpleNode_LangExpression,
		QuerySimpleNode_SQLExpression,
		QuerySimpleNode_Not,
		QuerySimpleNode_Join,
		QuerySimpleNode_BaseJoin,
		QuerySimpleNode_CacheSel,
		QuerySimpleNode_Const,

		QueryNode_Join,
		QueryNode_BaseJoin,
		QueryNode_BoolOper,
		QueryNode_Not,
		QueryNode_CompositeJoin,

		QueryNode_End
} QueryNodeType;


class ComplexOptimizedQuery;
class SimpleQueryNode;

class CQTargetsDef;
typedef set<CQTableDef> SetOfTargets;

class QueryNode : public Obj4D, public IObjCounter, public IRefCountable
{
	public:
		QueryNode(QueryNodeType inType, ComplexOptimizedQuery* inOwner)
		{
			fType = inType;
			fOwner = inOwner;
			fParent = nil;
			fIsInWhere = true;
		};

		inline QueryNodeType GetType() const { return fType; };
		inline QueryNode* GetParent() const { return fParent; };
		inline void SetParent(QueryNode* inParent) { fParent = inParent; };
		inline ComplexOptimizedQuery* GetOwner() const { return fOwner; };

		virtual uBOOL IsIndexe(BaseTaskInfo* context) { return(false); };
		virtual uBOOL IsAllIndexe(BaseTaskInfo* context) { return(false); };

		virtual CQTargetsDef* GenerateTargets(VError& err) { err = VE_DB4D_NOTIMPLEMENTED; return nil; };
		virtual VError AddToTargets(SetOfTargets& fullTarget) { return VE_DB4D_NOTIMPLEMENTED; };

		virtual Boolean isSimple() const { return false; };
		virtual Boolean isAllJoin() { return false; };

		virtual CQTableDef* GetSimpleTarget() { return nil; };

		virtual SimpleQueryNode* ConvertToSimple(const CQTableDef& target, VError& err) { err = VE_DB4D_NOTIMPLEMENTED; return nil; };

		virtual void ResetCacheFlags() { ; };

		virtual VError AddToQueryDescriptor(VString& descriptor, sLONG level);

		virtual VError Describe(VString& result) = 0;
		virtual VError FullyDescribe(VString& result)
		{
			return Describe(result);
		}

		virtual Boolean IsComposed() const { return false; };

		virtual sLONG GetPoids(BaseTaskInfo* context) { return 0; };

		virtual sLONG GetSubPoids(BaseTaskInfo* context) { return GetPoids(context); };

		virtual sLONG GetJoinPoids(BaseTaskInfo* context) { return 0; };

		virtual void PermuteAnds(BaseTaskInfo* context) { ; };

		virtual VError PerformComplex(ComplexSelection* filtre, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, ComplexSelection* &outSel, DB4D_Way_of_Locking HowToLock, sLONG limit, VValueBag* curdesc)
		{
			return VE_DB4D_NOTIMPLEMENTED;
		};

		virtual ComplexSelection* PerformComplexJoin(Bittab* filtre1, Bittab* filtre2, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, VError& err, DB4D_Way_of_Locking HowToLock, VValueBag* curdesc) { err = VE_DB4D_NOTIMPLEMENTED; return nil; };

		virtual sLONG8 CountDiskAccess()
		{
			return 0;
		}

		virtual sLONG CountBlobSources()
		{
			return 0;
		}

		virtual void AdjusteIntl(VIntlMgr* inIntl)
		{
			;
		}

		bool IsInWhere() const
		{
			return fIsInWhere;
		}

		void SetInWhere(bool x)
		{
			fIsInWhere = x;
		}

	protected:
		QueryNodeType fType;
		QueryNode* fParent;
		ComplexOptimizedQuery* fOwner;
		bool fIsInWhere;
};


class SimpleQueryNode : public QueryNode
{
	public:

		inline SimpleQueryNode(QueryNodeType inType, ComplexOptimizedQuery* inOwner, const CQTableDef& inTarget):QueryNode(inType, inOwner)
		{
			fTarget = inTarget;
		};

		inline CQTableDef& GetTarget() { return fTarget; };
		inline const CQTableDef& GetTarget() const { return fTarget; };

		virtual VError PerformComplex(ComplexSelection* filtre, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, ComplexSelection* &outSel, DB4D_Way_of_Locking HowToLock, sLONG limit, VValueBag* curdesc);

		virtual VError Perform(Bittab **super, Bittab* filtre, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, Bittab &Nulls, VValueBag* curdesc);
		virtual uBOOL PerformSeq(FicheInMem* curfic, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit) { return(false); };
		virtual uBOOL PerformSeq(FicheOnDisk* ficD, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit) { return(false); };

		virtual Bittab* PerformJoin(Bittab* sel, Bittab* selJoin, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, VError& err, VValueBag* curdesc) { err = VE_DB4D_NOTIMPLEMENTED; return nil; };

		virtual CQTargetsDef* GenerateTargets(VError& err);
		virtual VError AddToTargets(SetOfTargets& fullTarget);

		virtual Boolean IsConst() { return false; };
		virtual uBOOL GetConstResult() { return false; };

		virtual Boolean isSimple() const { return true; };

		virtual CQTableDef* GetSimpleTarget() { return &fTarget; };

		virtual SimpleQueryNode* ConvertToSimple(const CQTableDef& target, VError& err) 
		{ 
			err = VE_OK;
			Retain();
			return this; 
		};

		inline sLONG8 CalculateNBDiskAccessForSeq(Bittab* filtre, BaseTaskInfo* context, sLONG CountBlobSources = 0)
		{
			if (filtre == nil)
			{
				return ((sLONG8)(1+CountBlobSources*3)) * (sLONG8)(fTarget.GetTable()->GetDF()->GetNbRecords(context, false));
			}
			else
			{
				return ((sLONG8)(1+CountBlobSources*3)) * (sLONG8)(filtre->Compte());
			}
		}

	protected:
		CQTableDef fTarget;


};



class SimpleQueryNodeConst : public SimpleQueryNode
{
public:
	inline SimpleQueryNodeConst(ComplexOptimizedQuery* inOwner, const CQTableDef& inTarget, uBOOL constval)
		:SimpleQueryNode(QuerySimpleNode_Const, inOwner, inTarget)
	{
		fResult = constval;
	};

	virtual Boolean IsConst() { return true; };
	virtual uBOOL GetConstResult() { return fResult; };

	virtual uBOOL PerformSeq(FicheOnDisk* ficD, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit)
	{
		return fResult;
	}

	virtual uBOOL PerformSeq(FicheInMem* curfic, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit)
	{
		return fResult;
	}

	virtual uBOOL IsIndexe(BaseTaskInfo* context) { return(true); };
	virtual uBOOL IsAllIndexe(BaseTaskInfo* context) { return(true); };

	virtual VError Describe(VString& result)
	{
		VString s(L"Constant Value: "),s2;
		switch(fResult)
		{
			case 0:
				s2 = L"<Always False>";
				break;

			case 1:
				s2 = L"<Always True>";
				break;

			case 2:
				s2 = L"<Always NULL>";
				break;

			default:
				s2 = L"error";
				break;
		}
		s += s2;
		result = s;
		return VE_OK;
	}


	virtual VError AddToQueryDescriptor(VString& descriptor, sLONG level)
	{
		QueryNode::AddToQueryDescriptor(descriptor, level);
		VString s;
		Describe(s);
		s += L"\n";
		descriptor += s;
		return VE_OK;
	}

	virtual sLONG GetPoids(BaseTaskInfo* context) { return 0; };

	uBOOL fResult;
};

class SimpleQueryNodeSeq : public SimpleQueryNode
{
	public:
		inline SimpleQueryNodeSeq(ComplexOptimizedQuery* inOwner, const CQTableDef& inTarget, ComplexRechTokenSimpleComp* inRt)
								:SimpleQueryNode(QuerySimpleNode_Seq, inOwner, inTarget)
		{
			rt = inRt;
		};

		virtual uBOOL PerformSeq(FicheOnDisk* ficD, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit);
		virtual uBOOL PerformSeq(FicheInMem* curfic, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit);

		virtual VError AddToQueryDescriptor(VString& descriptor, sLONG level);

		virtual sLONG GetPoids(BaseTaskInfo* context) { return 100; };

		virtual VError Describe(VString& result);

		virtual void AdjusteIntl(VIntlMgr* inIntl)
		{
			rt->fOptions.SetIntlManager(inIntl);
		}


		ComplexRechTokenSimpleComp *rt;
};



class SimpleQueryNodeArraySeq : public SimpleQueryNode
{
	public:
		inline SimpleQueryNodeArraySeq(ComplexOptimizedQuery* inOwner, const CQTableDef& inTarget, ComplexRechTokenArrayComp* inRt)
			:SimpleQueryNode(QuerySimpleNode_ArraySeq, inOwner, inTarget)
		{
			rt = inRt;
			rechtyp = -1;
		};

		virtual uBOOL PerformSeq(FicheOnDisk* ficD, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit);
		virtual uBOOL PerformSeq(FicheInMem* curfic, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit);

		virtual VError AddToQueryDescriptor(VString& descriptor, sLONG level);
		virtual sLONG GetPoids(BaseTaskInfo* context) { return 110; };

		virtual VError Describe(VString& result);

		virtual void AdjusteIntl(VIntlMgr* inIntl)
		{
			rt->fOptions.SetIntlManager(inIntl);
		}

		ComplexRechTokenArrayComp *rt;

	protected:
		sLONG rechtyp;
};



class SimpleQueryNodeLangExpression : public SimpleQueryNode
{
	public:
		inline SimpleQueryNodeLangExpression(ComplexOptimizedQuery* inOwner, const CQTableDef& inTarget, ComplexRechTokenScriptComp* inRt)
			:SimpleQueryNode(QuerySimpleNode_LangExpression, inOwner, inTarget)
		{
			rt = inRt;
		};

		virtual uBOOL PerformSeq(FicheOnDisk* ficD, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit);
		virtual uBOOL PerformSeq(FicheInMem* curfic, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit);

		virtual VError AddToQueryDescriptor(VString& descriptor, sLONG level);
		virtual sLONG GetPoids(BaseTaskInfo* context) { return 120; };

		virtual VError Describe(VString& result);

		ComplexRechTokenScriptComp *rt;
};



class SimpleQueryNodeSQLExpression : public SimpleQueryNode
{
public:
	inline SimpleQueryNodeSQLExpression(ComplexOptimizedQuery* inOwner, const CQTableDef& inTarget, ComplexRechTokenSQLScriptComp* inRt)
		:SimpleQueryNode(QuerySimpleNode_SQLExpression, inOwner, inTarget)
	{
		rt = inRt;
	};

	virtual uBOOL PerformSeq(FicheOnDisk* ficD, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit);
	virtual uBOOL PerformSeq(FicheInMem* curfic, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit);

	virtual VError AddToQueryDescriptor(VString& descriptor, sLONG level);
	virtual sLONG GetPoids(BaseTaskInfo* context) { return 120; };

	virtual VError Describe(VString& result);

	ComplexRechTokenSQLScriptComp *rt;
};


typedef vector<VRefPtr<SimpleQueryNodeSeq>/*, cache_allocator<VRefPtr<SimpleQueryNodeSeq> > */> SimpleQueryNodeSeqVector;

class SimpleQueryNodeIndex : public SimpleQueryNode
{
	public:
		inline SimpleQueryNodeIndex(ComplexOptimizedQuery* inOwner, const CQTableDef& inTarget, ComplexRechTokenSimpleComp* inRt, IndexInfo* ind, SimpleQueryNodeSeq* nodeseq, BaseTaskInfo* context)
			:SimpleQueryNode(QuerySimpleNode_Index, inOwner, inTarget)
		{
			rt = inRt;
			fInd = ind;
			fInd->Retain();
			fIncludingFork = nil;
			rt2 = nil;
			fNodeSeq = nodeseq;
			fOptions.SetIntlManager(GetContextIntl(context));
			fParseAllIndex = false;
		};

		virtual ~SimpleQueryNodeIndex();

		virtual uBOOL IsIndexe(BaseTaskInfo* context) { return(true); };
		virtual uBOOL IsAllIndexe(BaseTaskInfo* context) { return(true); };

		virtual VError Perform(Bittab **super, Bittab* filtre, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, Bittab &Nulls, VValueBag* curdesc);

		uBOOL PeutFourche(void);
		uBOOL PeutFourche(SimpleQueryNodeIndex* autre);
		VError SubBuildCompFrom(sLONG comp, BTitemIndex *val);
		VError BuildFrom(ComplexRechTokenSimpleComp *xrt);
		VError BuildFromMultiple(SimpleQueryNodeSeq *IncludingFork, BaseTaskInfo* context);
		VError Add(SimpleQueryNodeSeq* nodeseq);

		virtual VError AddToQueryDescriptor(VString& descriptor, sLONG level);
		virtual sLONG GetPoids(BaseTaskInfo* context);

		virtual sLONG8 CountDiskAccess();
		virtual sLONG CountBlobSources();

		virtual VError Describe(VString& result);

		inline void SetParseAllIndex(Boolean x)
		{
			fParseAllIndex = x;
		}

		virtual void AdjusteIntl(VIntlMgr* inIntl)
		{
			if (rt != nil)
				rt->fOptions.SetIntlManager(inIntl);
			if (rt2 != nil)
				rt2->fOptions.SetIntlManager(inIntl);
			fOptions.SetIntlManager(inIntl);
		}

		SimpleQueryNodeSeqVector fNodes;
		ComplexRechTokenSimpleComp *rt, *rt2;
		VRefPtr<SimpleQueryNodeSeq> fNodeSeq;
		VRefPtr<SimpleQueryNodeSeq> fIncludingFork;
		uBOOL isfourche;
		IndexInfo *fInd;
		BTitemIndex *cle1;
		BTitemIndex *cle2;
		VCompareOptions fOptions;
		uBOOL xstrict1,xstrict2;
		uBOOL inverse, fParseAllIndex/*,fIsBeginWith, fIsLike, fIsDiacritic*/;
};



class SimpleQueryNodeArrayIndex : public SimpleQueryNode
{
	public:
		inline SimpleQueryNodeArrayIndex(ComplexOptimizedQuery* inOwner, const CQTableDef& inTarget, ComplexRechTokenArrayComp* inRt, IndexInfo* ind, SimpleQueryNodeArraySeq* nodeseq)
			:SimpleQueryNode(QuerySimpleNode_ArrayIndex, inOwner, inTarget)
		{
			rt = inRt;
			fInd = ind;
			fInd->Retain();
			fNodeSeq = nodeseq;
		};

		virtual ~SimpleQueryNodeArrayIndex()
		{
			fInd->ReleaseValid();
			fInd->Release();
		};

		virtual uBOOL IsIndexe(BaseTaskInfo* context) { return(true); };
		virtual uBOOL IsAllIndexe(BaseTaskInfo* context) { return(true); };

		virtual VError Perform(Bittab **super, Bittab* filtre, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, Bittab &Nulls, VValueBag* curdesc);

		VError BuildFrom(ComplexRechTokenArrayComp *xrt);

		virtual VError AddToQueryDescriptor(VString& descriptor, sLONG level);
		virtual sLONG GetPoids(BaseTaskInfo* context) { return 8; };

		virtual sLONG CountBlobSources();
		virtual sLONG8 CountDiskAccess();
		virtual VError Describe(VString& result);

		virtual void AdjusteIntl(VIntlMgr* inIntl)
		{
			if (rt != nil)
				rt->fOptions.SetIntlManager(inIntl);
			fOptions.SetIntlManager(inIntl);
		}

		VRefPtr<SimpleQueryNodeArraySeq> fNodeSeq;
		ComplexRechTokenArrayComp *rt;
		IndexInfo *fInd;
		VCompareOptions fOptions;
		uBOOL inverse/*,fIsBeginWith,fIsLike, fIsDiacritic*/;
};



typedef vector<VRefPtr<SimpleQueryNode>/*, cache_allocator<VRefPtr<SimpleQueryNode> > */> SimpleQueryNodeVector;

class SimpleQueryNodeBoolOper : public SimpleQueryNode
{
	public:
		inline SimpleQueryNodeBoolOper(ComplexOptimizedQuery* inOwner, const CQTableDef& inTarget, sWORD operbool)
			:SimpleQueryNode(QuerySimpleNode_BoolOper, inOwner, inTarget)
		{
			fOperBool = operbool;
			recalcIndex = true; 
			recalcAllIndex = true; 
			recalcAllJoin = true;
			recalcPoids = true;
			recalcConst = true;
		};

		virtual ~SimpleQueryNodeBoolOper()
		{
			
		};

		virtual Boolean IsConst();
		virtual uBOOL GetConstResult();
		virtual uBOOL IsIndexe(BaseTaskInfo* context);
		virtual uBOOL IsAllIndexe(BaseTaskInfo* context);
		virtual Boolean isAllJoin();

		virtual VError Perform(Bittab **super, Bittab* filtre, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, Bittab &Nulls, VValueBag* curdesc);
		virtual uBOOL PerformSeq(FicheOnDisk* ficD, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit);
		virtual uBOOL PerformSeq(FicheInMem* curfic, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit);

		virtual Bittab* PerformJoin(Bittab* sel, Bittab* selJoin, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, VError& err, VValueBag* curdesc) { err = VE_DB4D_NOTIMPLEMENTED; return nil; };
		// a faire

		inline sWORD GetOper() const { return fOperBool; };

		Boolean AddNode(SimpleQueryNode* rn);
		void DeleteNode(SimpleQueryNodeVector::iterator NodePos);
		void DeleteNode(sLONG pos);
		SimpleQueryNodeVector& GetArrayNodes() { return fNodes; };

		virtual void ResetCacheFlags() { recalcIndex = true; recalcAllIndex = true; recalcAllJoin = true; };

		virtual VError AddToQueryDescriptor(VString& descriptor, sLONG level);

		virtual Boolean IsComposed() const { return true; };

		virtual sLONG GetPoids(BaseTaskInfo* context);

		virtual void PermuteAnds(BaseTaskInfo* context);

		virtual sLONG8 CountDiskAccess();
		virtual sLONG CountBlobSources();

		virtual VError Describe(VString& result);
		virtual VError FullyDescribe(VString& result);

		virtual void AdjusteIntl(VIntlMgr* inIntl);

	protected:
		sWORD fOperBool;
		SimpleQueryNodeVector fNodes;
		uBOOL recalcIndex, recalcAllIndex, recalcAllJoin, recalcConst;
		uBOOL isindexe, isallindexe, isconst, resultconst;
		uBOOL isalljoin;
		uBOOL recalcPoids;
		sLONG fPoids;
};



class SimpleQueryNodeNot : public SimpleQueryNode
{
	public:
		inline SimpleQueryNodeNot(ComplexOptimizedQuery* inOwner, const CQTableDef& inTarget)
			:SimpleQueryNode(QuerySimpleNode_Not, inOwner, inTarget)
		{
			fRoot = nil;
		};

		virtual uBOOL IsIndexe(BaseTaskInfo* context);
		virtual uBOOL IsAllIndexe(BaseTaskInfo* context);

		virtual Boolean IsConst();
		virtual uBOOL GetConstResult();

		virtual VError Perform(Bittab **super, Bittab* filtre, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, Bittab &Nulls, VValueBag* curdesc);
		virtual uBOOL PerformSeq(FicheOnDisk* ficD, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit);
		virtual uBOOL PerformSeq(FicheInMem* curfic, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit);

		inline void SetRoot(SimpleQueryNode* inRoot) { fRoot = inRoot; };
		inline void AdoptRoot(SimpleQueryNode* inRoot) { fRoot.Adopt(inRoot); };
		inline SimpleQueryNode* GetRoot() const { return fRoot; };

		virtual VError AddToQueryDescriptor(VString& descriptor, sLONG level);

		virtual void PermuteAnds(BaseTaskInfo* context) 
		{ 
			if (fRoot != nil)
				fRoot->PermuteAnds(context);
		};

		virtual void AdjusteIntl(VIntlMgr* inIntl)
		{
			if (fRoot != nil)
				fRoot->AdjusteIntl(inIntl);
		}

		virtual sLONG8 CountDiskAccess();
		virtual sLONG CountBlobSources();

		virtual VError Describe(VString& result);
		virtual VError FullyDescribe(VString& result);

	protected:
		VRefPtr<SimpleQueryNode> fRoot;
};


class QuerySimpleNodeCacheSel : public SimpleQueryNode
{
	public:
		inline QuerySimpleNodeCacheSel(ComplexOptimizedQuery* inOwner, const CQTableDef& inTarget, SimpleQueryNode* root)
			:SimpleQueryNode(QuerySimpleNode_CacheSel, inOwner, inTarget)
		{
			fRoot = root;
			fCachedSel = nil;
		};

		virtual ~QuerySimpleNodeCacheSel()
		{
			if (fCachedSel != nil)
				fCachedSel->Release();
		}

		virtual uBOOL IsIndexe(BaseTaskInfo* context);
		virtual uBOOL IsAllIndexe(BaseTaskInfo* context);

		virtual VError Perform(Bittab **super, Bittab* filtre, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, Bittab &Nulls, VValueBag* curdesc);
		virtual uBOOL PerformSeq(FicheOnDisk* ficD, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit);
		virtual uBOOL PerformSeq(FicheInMem* curfic, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit);

		virtual VError AddToQueryDescriptor(VString& descriptor, sLONG level);

		inline void SetRoot(SimpleQueryNode* inRoot) { fRoot = inRoot; };
		inline SimpleQueryNode* GetRoot() const { return fRoot; };

		virtual void PermuteAnds(BaseTaskInfo* context) 
		{ 
			if (fRoot != nil)
				fRoot->PermuteAnds(context);
		};

		virtual Boolean IsConst()
		{
			if (fRoot != nil)
				return fRoot->IsConst();
			else
				return false;
		}

		virtual uBOOL GetConstResult()
		{
			if (fRoot != nil)
				return fRoot->GetConstResult();
			else
				return false;
		}

		virtual sLONG8 CountDiskAccess();
		virtual sLONG CountBlobSources();

		virtual VError Describe(VString& result);
		virtual VError FullyDescribe(VString& result);

		virtual void AdjusteIntl(VIntlMgr* inIntl)
		{
			if (fRoot != nil)
				fRoot->AdjusteIntl(inIntl);
		}

	protected:
		VRefPtr<SimpleQueryNode> fRoot;
		Bittab* fCachedSel;
		Bittab fNulls;
};



class SimpleQueryNodeJoin : public SimpleQueryNode
{
	public:
		inline SimpleQueryNodeJoin(ComplexOptimizedQuery* inOwner, const CQTableDef& inTarget, ComplexRechTokenJoin* inRt)
			:SimpleQueryNode(QuerySimpleNode_Join, inOwner, inTarget)
		{
			rt = inRt;
			recalcIndex = true;
			isindex = false;
		};

		virtual uBOOL IsIndexe(BaseTaskInfo* context);
		virtual uBOOL IsAllIndexe(BaseTaskInfo* context);

		//virtual VError Perform(Bittab **super, Bittab* filtre, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, Bittab &Nulls, VValueBag* curdesc);
		virtual Bittab* PerformJoin(Bittab* sel, Bittab* selJoin, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, VError& err, VValueBag* curdesc);
		virtual Boolean isAllJoin() { return true; };

		virtual void ResetCacheFlags() { recalcIndex = true; };

		virtual VError AddToQueryDescriptor(VString& descriptor, sLONG level);

		virtual Boolean IsComposed() const { return true; };

		ComplexRechTokenJoin *rt;

		virtual sLONG GetPoids(BaseTaskInfo* context) { return IsIndexe(context) ? 10 : 20; };

		virtual VError Describe(VString& result);
		virtual VError FullyDescribe(VString& result);

	protected:

		Boolean recalcIndex, isindex;

};


class SimpleQueryNodeBaseJoin : public SimpleQueryNode
{
	public:
		inline SimpleQueryNodeBaseJoin(ComplexOptimizedQuery* inOwner, const CQTableDef& inTarget, const CQTableDef& inJoinTarget)
			:SimpleQueryNode(QuerySimpleNode_BaseJoin, inOwner, inTarget)
		{
			fRoot = nil;
			fJoinRoot = nil;
			fJoinTarget = inJoinTarget;
		};

		virtual uBOOL IsIndexe(BaseTaskInfo* context);
		virtual uBOOL IsAllIndexe(BaseTaskInfo* context);

		virtual VError Perform(Bittab **super, Bittab* filtre, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, Bittab &Nulls, VValueBag* curdesc);

		inline void SetRoot(SimpleQueryNode* inRoot) { fRoot = inRoot; };
		inline SimpleQueryNode* GetRoot() const { return fRoot; };

		inline void SetJoinRoot(SimpleQueryNode* inJoinRoot) { fJoinRoot = inJoinRoot; };
		inline SimpleQueryNode* GetJoinRoot() const { return fJoinRoot; };

		virtual VError AddToQueryDescriptor(VString& descriptor, sLONG level);

		virtual Boolean IsComposed() const { return true; };

		virtual Boolean isAllJoin() { return true; };

		virtual sLONG GetSubPoids(BaseTaskInfo* context); 
		virtual sLONG GetJoinPoids(BaseTaskInfo* context); 

		virtual sLONG GetPoids(BaseTaskInfo* context) { return GetSubPoids(context) + GetJoinPoids(context); };

		virtual void PermuteAnds(BaseTaskInfo* context) 
		{ 
			if (fRoot != nil)
				fRoot->PermuteAnds(context);
		};

		virtual VError Describe(VString& result);
		virtual VError FullyDescribe(VString& result);

		virtual void AdjusteIntl(VIntlMgr* inIntl)
		{
			if (fRoot != nil)
				fRoot->AdjusteIntl(inIntl);
			if (fJoinRoot != nil)
				fJoinRoot->AdjusteIntl(inIntl);
		}

	protected:
		VRefPtr<SimpleQueryNode> fRoot;
		VRefPtr<SimpleQueryNode> fJoinRoot;
		CQTableDef fJoinTarget;


};


			/* ----------------------------- */


typedef vector<VRefPtr<QueryNode>/*, cache_allocator<VRefPtr<QueryNode> >*/ > QueryNodeVector;


class CQNodesContent : public IRefCountable
{
	public:
		Boolean Add(QueryNode* node);

		QueryNodeVector fnodes;
};

			/* ----------------------------- */


class QueryNodeJoin : public QueryNode
{
	public:

		inline QueryNodeJoin(ComplexOptimizedQuery* inOwner, const CQTableDef& inTarget1, const CQTableDef& inTarget2, ComplexRechTokenJoin* inRt)
							:QueryNode(QueryNode_Join, inOwner)
		{
			fTarget1 = inTarget1;
			fTarget2 = inTarget2;
			rt = inRt;
			recalcIndex = true;
			isindex = false;
		};

		virtual uBOOL IsIndexe(BaseTaskInfo* context);
		virtual uBOOL IsAllIndexe(BaseTaskInfo* context);

		virtual VError AddToTargets(SetOfTargets& fullTarget);
		virtual Boolean isAllJoin() { return true; };

		virtual void AdjusteIntl(VIntlMgr* inIntl)
		{
			if (rt != nil)
				rt->fOptions.SetIntlManager(inIntl);
		}

		virtual SimpleQueryNode* ConvertToSimple(const CQTableDef& target, VError& err);

		ComplexRechTokenJoin *rt;

		inline CQTableDef* GetTarget1() { return &fTarget1; };
		inline CQTableDef* GetTarget2() { return &fTarget2; };

		virtual VError AddToQueryDescriptor(VString& descriptor, sLONG level);
		virtual CQTargetsDef* GenerateTargets(VError& err);

		virtual sLONG GetPoids(BaseTaskInfo* context) { return IsIndexe(context) ? 10 : 20; };

		virtual ComplexSelection* PerformComplexJoin(Bittab* filtre1, Bittab* filtre2, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, VError& err, DB4D_Way_of_Locking HowToLock, VValueBag* curdesc);

		virtual Boolean IsComposed() const { return true; };

		virtual VError Describe(VString& result);
		virtual VError FullyDescribe(VString& result);

	protected:
		CQTableDef fTarget1, fTarget2;
		Boolean recalcIndex, isindex;
};



class QueryNodeCompositeJoin : public QueryNode
{
	public:

		inline QueryNodeCompositeJoin(ComplexOptimizedQuery* inOwner, CQTableDef& t1, CQTableDef& t2) :QueryNode(QueryNode_CompositeJoin, inOwner)
		{
			recalcIndex = true;
			isindex = false;
			fLeftJoin = false;
			fRightJoin = false;
			fInd1 = nil;
			fInd2 = nil;
			fTarget1 = t1;
			fTarget2 = t2;
		};

		virtual ~QueryNodeCompositeJoin()
		{
			ForgetIndexes();
		}

		void ForgetIndexes()
		{
			if (fInd1 != nil)
				fInd1->ReleaseValid();
			if (fInd2 != nil)
				fInd2->ReleaseValid();
			QuickReleaseRefCountable(fInd1);
			QuickReleaseRefCountable(fInd2);
			fInd1 = nil;
			fInd2 = nil;
		}

		VError AddJoinPart(Field* cri1, Field* cri2, bool leftjoin, bool rightjoin, const VCompareOptions& inOptions);

		virtual uBOOL IsIndexe(BaseTaskInfo* context);
		virtual uBOOL IsAllIndexe(BaseTaskInfo* context);

		virtual VError AddToTargets(SetOfTargets& fullTarget);
		virtual Boolean isAllJoin() { return true; };

		virtual void AdjusteIntl(VIntlMgr* inIntl)
		{
			fOptions.SetIntlManager(inIntl);
		}

		virtual SimpleQueryNode* ConvertToSimple(const CQTableDef& target, VError& err);

		inline CQTableDef* GetTarget1() { return &fTarget1; };
		inline CQTableDef* GetTarget2() { return &fTarget2; };

		virtual VError AddToQueryDescriptor(VString& descriptor, sLONG level);
		virtual CQTargetsDef* GenerateTargets(VError& err);

		virtual sLONG GetPoids(BaseTaskInfo* context) { return IsIndexe(context) ? 10 : 20; };

		virtual ComplexSelection* PerformComplexJoin(Bittab* filtre1, Bittab* filtre2, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, VError& err, DB4D_Way_of_Locking HowToLock, VValueBag* curdesc);

		virtual Boolean IsComposed() const { return true; };

		virtual VError Describe(VString& result);
		virtual VError FullyDescribe(VString& result);

	protected:
		CQTableDef fTarget1, fTarget2;
		vector<Field*> fFields1;
		vector<Field*> fFields2;
		Boolean recalcIndex, isindex;
		bool fLeftJoin, fRightJoin;
		VCompareOptions fOptions;
		IndexInfo* fInd1;
		IndexInfo* fInd2;
};



class QueryNodeBaseJoin : public QueryNode
{
	public:
		inline QueryNodeBaseJoin(ComplexOptimizedQuery* inOwner, const CQTableDef& inTarget1, const CQTableDef& inTarget2)
			:QueryNode(QueryNode_BaseJoin, inOwner)
		{
			fTarget1 = inTarget1;
			fTarget2 = inTarget2;
			fRoot1 = nil;
			fJoinRoot = nil;
			fRoot2 = nil;
		};

		virtual uBOOL IsIndexe(BaseTaskInfo* context);
		virtual uBOOL IsAllIndexe(BaseTaskInfo* context);

		virtual Boolean isAllJoin() { return true; };

		inline void SetRoot1(QueryNode* inRoot) { fRoot1 = inRoot; };
		inline QueryNode* GetRoot1() const { return fRoot1; };

		inline void SetRoot2(QueryNode* inRoot) { fRoot2 = inRoot; };
		inline QueryNode* GetRoot2() const { return fRoot2; };

		inline void SetJoinRoot(QueryNode* inJoinRoot) { fJoinRoot = inJoinRoot; };
		inline QueryNode* GetJoinRoot() const { return fJoinRoot; };

		virtual VError AddToQueryDescriptor(VString& descriptor, sLONG level);

		virtual VError AddToTargets(SetOfTargets& fullTarget);

		virtual CQTargetsDef* GenerateTargets(VError& err);

		virtual sLONG GetSubPoids(BaseTaskInfo* context); 
		virtual sLONG GetJoinPoids(BaseTaskInfo* context); 

		virtual sLONG GetPoids(BaseTaskInfo* context) { return GetSubPoids(context) + GetJoinPoids(context); };

		virtual void PermuteAnds(BaseTaskInfo* context) 
		{ 
			if (fRoot1 != nil)
				fRoot1->PermuteAnds(context);
			if (fRoot2 != nil)
				fRoot2->PermuteAnds(context);
		};

		virtual VError PerformComplex(ComplexSelection* filtre, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, ComplexSelection* &outSel, DB4D_Way_of_Locking HowToLock, sLONG limit, VValueBag* curdesc);

		virtual Boolean IsComposed() const { return true; };

		virtual VError Describe(VString& result);
		virtual VError FullyDescribe(VString& result);

		virtual void AdjusteIntl(VIntlMgr* inIntl)
		{
			if (fRoot1 != nil)
				fRoot1->AdjusteIntl(inIntl);
			if (fRoot2 != nil)
				fRoot2->AdjusteIntl(inIntl);
			if (fJoinRoot != nil)
				fJoinRoot->AdjusteIntl(inIntl);
		}

	protected:
		VRefPtr<QueryNode> fRoot1;
		VRefPtr<QueryNode> fJoinRoot;
		VRefPtr<QueryNode> fRoot2;
		CQTableDef fTarget1, fTarget2;


};



class QueryNodeBoolOper : public QueryNode
{
	public:

		inline QueryNodeBoolOper(ComplexOptimizedQuery* inOwner, sWORD boolOper):QueryNode(QueryNode_BoolOper, inOwner), fCachedTargets(nil)
		{
			fOperBool = boolOper;
			mustRecalcTarget = true;
			fAllSameTarget = false;
			recalcAllJoin = true;
			recalcIndex = true; 
			recalcAllIndex = true;
			recalcPoids = true;
		};

		inline sWORD GetOper() const { return fOperBool; };

		Boolean AddNode(QueryNode* rn);
		void DeleteNode(QueryNodeVector::iterator NodePos);
		void DeleteNode(sLONG pos);
		CQNodesContent& GetArrayNodes() { return fNodes; };

		virtual CQTargetsDef* GenerateTargets(VError& err);
		virtual VError AddToTargets(SetOfTargets& fullTarget);

		inline Boolean AllSameTarget() const { return fAllSameTarget; };
		inline void SetAllSameTarget(Boolean x) { fAllSameTarget = x; };

		virtual Boolean isAllJoin();

		virtual SimpleQueryNode* ConvertToSimple(const CQTableDef& target, VError& err);

		virtual void ResetCacheFlags() { recalcIndex = true; recalcAllIndex = true; recalcAllJoin = true; mustRecalcTarget = true;};

		virtual VError AddToQueryDescriptor(VString& descriptor, sLONG level);

		virtual sLONG GetPoids(BaseTaskInfo* context);

		virtual void PermuteAnds(BaseTaskInfo* context);

		virtual VError PerformComplex(ComplexSelection* filtre, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, ComplexSelection* &outSel, DB4D_Way_of_Locking HowToLock, sLONG limit, VValueBag* curdesc);

		virtual ComplexSelection* PerformComplexJoin(Bittab* filtre1, Bittab* filtre2, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, VError& err, DB4D_Way_of_Locking HowToLock, VValueBag* curdesc) { err = VE_DB4D_NOTIMPLEMENTED; return nil; };
		// a faire
		virtual Boolean IsComposed() const { return true; };

		virtual VError Describe(VString& result);
		virtual VError FullyDescribe(VString& result);

		virtual void AdjusteIntl(VIntlMgr* inIntl);

protected:
		sWORD fOperBool;
		CQNodesContent fNodes;
		VRefPtr<CQTargetsDef> fCachedTargets;
		Boolean mustRecalcTarget, fAllSameTarget;
		uBOOL recalcAllJoin, isalljoin;
		uBOOL recalcIndex, recalcAllIndex;
		uBOOL recalcPoids;
		sLONG fPoids;

};


class QueryNodeNot : public QueryNode
{
	public:
		inline QueryNodeNot(ComplexOptimizedQuery* inOwner):QueryNode(QueryNode_Not, inOwner), fRoot(nil) {;};
		
		virtual uBOOL IsIndexe(BaseTaskInfo* context)
		{
			if (fRoot == nil)
				return false;
			else
				return fRoot->IsIndexe(context);
		};

		virtual uBOOL IsAllIndexe(BaseTaskInfo* context)
		{
			if (fRoot == nil)
				return false;
			else
				return fRoot->IsAllIndexe(context);
		};

		virtual CQTargetsDef* GenerateTargets(VError& err)
		{
			if (fRoot == nil)
			{
				err = VE_OK;
				return nil;
			}
			else
				return fRoot->GenerateTargets(err);
		}

		virtual VError AddToTargets(SetOfTargets& fullTarget)
		{
			if (fRoot != nil)
				return fRoot->AddToTargets(fullTarget);
			else
				return VE_OK;
		}

		virtual Boolean isAllJoin()
		{ 
			if (fRoot == nil) 
				return false;
			else
				return fRoot->isAllJoin();
		};

		virtual SimpleQueryNode* ConvertToSimple(const CQTableDef& target, VError& err);

		virtual VError AddToQueryDescriptor(VString& descriptor, sLONG level);

		virtual sLONG GetPoids(BaseTaskInfo* context) 
		{ 
			if (fRoot == nil) 
				return 0;
			else
				return fRoot->GetPoids(context);
		};

		virtual VError PerformComplex(ComplexSelection* filtre, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, ComplexSelection* &outSel, DB4D_Way_of_Locking HowToLock, sLONG limit, VValueBag* curdesc);

		inline void SetRoot(QueryNode* inRoot) { fRoot = inRoot; };
		inline QueryNode* GetRoot() const { return fRoot; };

		virtual VError Describe(VString& result);
		virtual VError FullyDescribe(VString& result);

		virtual void AdjusteIntl(VIntlMgr* inIntl)
		{
			if (fRoot != nil)
				fRoot->AdjusteIntl(inIntl);
		}

	protected:
		VRefPtr<QueryNode> fRoot;


};




			/* ----------------------------------------------------------- */



class CompareLessCQTableDefVectorTemplate
{
public:
	inline bool operator () (CQTargetsDef* x1, CQTargetsDef* x2) const
	{
		return CompareLessCQTableDefVector(x1,x2);
	};
};

typedef map<VRefPtr<CQTargetsDef>, VRefPtr<CQNodesContent>, CompareLessCQTableDefVectorTemplate/*, cache_allocator<pair<VRefPtr<CQTargetsDef> const,  VRefPtr<CQNodesContent> > >*/ > QueryNodeMap;

class QueryNodeAggregate
{
	public:
		QueryNodeAggregate() { fOper = DB4D_NOTCONJ; };
		~QueryNodeAggregate();

		inline sWORD GetOper() const { return fOper; };
		inline void SetOper(sWORD inOper) { fOper = inOper; };

		Boolean AddNode(CQTargetsDef* cle, QueryNode* Node);
		Boolean AddNode(const CQTableDef& cle, QueryNode* Node);

		sWORD fOper;
		QueryNodeMap fNodes;
};



class NodeUsage
{
public:
	inline NodeUsage(): fRoot(nil), fBelongsToBaseJoin(nil), fBelongsToSimpleBaseJoin(nil), fSelCacheNode(nil) { fCountUsed = 0; posinvector = -1; fRootNum = 0;};

	inline NodeUsage& operator = (const NodeUsage& other)
	{
		fCountUsed = other.fCountUsed;
		posinvector = other.posinvector;
		fBelongsToBaseJoin = other.fBelongsToBaseJoin;
		fBelongsToSimpleBaseJoin = other.fBelongsToSimpleBaseJoin;
		fSelCacheNode = other.fSelCacheNode;
		fRootNum = other.fRootNum;
		return *this;
	};

#if debuglr
	virtual ~NodeUsage()
	{
		sLONG i = 1;
	}
#endif

	inline void SetBelongsToBaseJoin(QueryNodeBaseJoin* basejoin, sLONG rootnum)
	{
		fRootNum = rootnum;
		fBelongsToBaseJoin = basejoin;
	}

	inline void Set(SimpleQueryNode* root, sLONG pos)
	{
		fRoot = root;
		posinvector = pos;
	};

	void SetRoot(SimpleQueryNode* root);

	VError CacheRoot(ComplexOptimizedQuery* query);
	SimpleQueryNode* GetCachedRoot() const;

	VRefPtr<SimpleQueryNode> fSelCacheNode;
	sLONG posinvector;
	sLONG fCountUsed;
	sLONG fRootNum;
	VRefPtr<SimpleQueryNode> fRoot;
	VRefPtr<QueryNodeBaseJoin> fBelongsToBaseJoin;
	VRefPtr<SimpleQueryNodeBaseJoin> fBelongsToSimpleBaseJoin;
};


class JoinDependence
{
public:
	CQTableDefVector others;
};


typedef map<CQTableDef, NodeUsage> NodeUsageMap;
typedef map<CQTableDef, JoinDependence> JoinDependenceMap;

class ComplexOptimizedQuery : public Obj4D, public IObjCounter
{
	public:
		ComplexOptimizedQuery()
		{
			fRoot = nil;
			fCurLevel = 0;
			fExecutionDescription = nil; 
			fDescribeExecution = false;
			fIsInWhere = false;
			fWaitForJoin = false;
			fCurJoinLeft = false;
			fCurJoinRight = false;
		};

		virtual ~ComplexOptimizedQuery()
		{
			if (fExecutionDescription != nil)
				fExecutionDescription->Release();
		}

		VError AnalyseSearch(ComplexRech *model, BaseTaskInfo* context);

		inline void SetRoot(QueryNode* inRoot) { fRoot = inRoot; };
		inline QueryNode* GetRoot() const { return fRoot; };

		void FillLevelString(VString &outString, sLONG level);
	
		VError BuildQueryDescriptor(VString& outDescription);

		VError PerformComplex(ComplexSelection* dejasel, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, ComplexSelection* &outSel, DB4D_Way_of_Locking HowToLock, sLONG limit);

		inline void SetLevel(sLONG n) { fCurLevel = n; };
		inline void IncLevel() { fCurLevel++; };
		inline void DecLevel() { fCurLevel--; };
		inline sLONG GetLevel() const { return fCurLevel; };

		QueryNode* xTransformSeq(QueryNode* root, Boolean seq, VError& err, BaseTaskInfo* context);

		inline void DescribeQuery(Boolean describe) { fDescribeExecution = describe; };
		inline Boolean IsQueryDescribed() const { return fDescribeExecution; };

		inline uLONG GetStartingTime() const
		{
			if (fDescribeExecution)
				return VSystem::GetCurrentTime();
			else
				return 0;
		};

		inline VValueBag* GetExecutionDescription() const
		{
			return fExecutionDescription;
		}

		VValueBag* AddToDescription(VValueBag* root, const VString s, uLONG startingMillisecs, Bittab* b);
		VValueBag* AddToDescription(VValueBag* root, const VString s, uLONG startingMillisecs, ComplexSelection& sel);

	protected:
		QueryNodeAggregate* xBuildNodes(ComplexRech *model, VError& err, Boolean OnlyOneToken);
		QueryNode* xMakeOperNode(QueryNodeAggregate* onemap, VError& err);
		QueryNode* xBuildJoins(QueryNode* root, VError& err, QueryNode* &LeftOver);
		QueryNode* xRegroupCompositeJoins(QueryNode* root, VError& err, BaseTaskInfo* context);
		QueryNode* xTransformIndex(QueryNode* root, VError& err, BaseTaskInfo* context);
		VError BuildTargetSet();
		Boolean MatchTarget(const CQTableDef& target);
		Boolean IsInFinalTarget(const CQTableDef& target, SetOfTargets& deja, JoinDependenceMap joindeps);
		SimpleQueryNodeBaseJoin* ConvertToSimpleBaseJoin(QueryNode* joinroot, const CQTableDef& target, const CQTableDef& targetjoin, SimpleQueryNode* root, VError& err);

		VRefPtr<QueryNode> fRoot;
		BaseTaskInfo* fAnalyseContext;
		ComplexRech* fModel;
		SetOfTargets fTargetSet;
		sLONG fCurLevel;
		Boolean fDescribeExecution;
		bool fIsInWhere, fWaitForJoin, fCurJoinLeft, fCurJoinRight;
		VValueBag* fExecutionDescription;
};


 

#endif

