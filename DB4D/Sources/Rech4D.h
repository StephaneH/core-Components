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
#ifndef __RECH4D__
#define __RECH4D__

enum { aucuntokenrech=0L, r_ParentG, r_ParentD, r_Fin, r_Line, r_LineEM, r_BoolOperator, r_Script, r_Not, r_LineArray, r_LineEMArray, r_LineJoin, r_LineEMJoin, r_Sel, r_EmSel, r_EMComp, 
		r_InterpConst, r_InterpAtt, r_InterpOper, r_RecordExist, r_RecordEntityExist, r_InterpFormula, r_JSScript };

class OptimizedQuery;


extern void CheckForNullOn(sLONG& comp, bool& checkfornull);

namespace QueryKeys
{
	CREATE_BAGKEY( description);
	CREATE_BAGKEY( time);
	CREATE_BAGKEY( recordsfounds);
	CREATE_BAGKEY( steps);
};

class EntityCollection;


class InstanceMap : public map<EntityModel*, sLONG>
{
public:
	sLONG GetInstance(EntityModel* model) const
	{
		InstanceMap::const_iterator found = find(model);
		if (found == end())
			return 0;
		else
			return found->second;
	}

	sLONG GetInstanceAndIncrement(EntityModel* model)
	{
		InstanceMap::iterator found = find(model);
		if (found == end())
		{
			(*this)[model] = 1;
			return 0;
		}
		else
		{
			sLONG inst = found->second;
			++(found->second);
			return inst;
		}
	}

};


class RechToken : public Obj4D, public IObjCounter
{
	public :
		sWORD TypToken;
		
		sWORD GetTyp(void) { return(TypToken); };
		virtual void Dispose(void) { /* rien */ };
		virtual VError PutInto(VStream& buf);
		virtual VError GetFrom(VStream& buf, Table* target);

		virtual RechToken* Clone() const;

		virtual VError BuildString(VString& outStr)
		{
			return ThrowBaseError(VE_UNIMPLEMENTED);
		}

		virtual VJSONObject* ToJSON(BaseTaskInfo* context, UniChar wildcharReplace = 0)
		{
			VJSONObject* result = new VJSONObject();

			result->SetPropertyAsString("tokenType", ERechToken[TypToken]);
			return result;
		}

		virtual VError FromJSON(BaseTaskInfo* context, VJSONObject* obj)
		{
			VString s;
			obj->GetPropertyAsString("tokenType", s);
			TypToken = ERechToken[s];
			return VE_OK;
		}

		virtual void addInstance(const EntityModel* model, sLONG howmany)
		{
		};

		virtual VError BuildRefString(VString& outRef, bool canMeasure, Base4D* bd)
		{
			switch (TypToken)
			{
				case r_ParentG:
					outRef += "(";
					break;
				case r_ParentD:
					outRef += ")";
					break;
				case r_Not:
					outRef += " not ";
					break;

				default:
					break;
			}

			return VE_OK;
		}

	
};


typedef RechToken *RechTokenPtr;
typedef vector<RechTokenPtr> RechTokenVector;



class RechTokenFormula : public RechToken
{
	public:
		RechTokenVector fLines;
		virtual ~RechTokenFormula()
		{
			for (RechTokenVector::iterator cur = fLines.begin(), end = fLines.end(); cur != end; cur++)
			{
				delete (*cur);
			}
		}

		RechTokenFormula()
		{
			TypToken = r_InterpFormula;
		}

};



class RechTokenBoolOper : public RechToken
{
	public:
		sWORD BoolLogic;
		
		RechTokenBoolOper(void) { TypToken = r_BoolOperator; };
		virtual VError PutInto(VStream& buf);
		virtual VError GetFrom(VStream& buf, Table* target);
		
		virtual VError BuildString(VString& outStr);
		virtual RechToken* Clone() const;

		virtual VJSONObject* ToJSON(BaseTaskInfo* context, UniChar wildcharReplace = 0)
		{
			VJSONObject* result = RechToken::ToJSON(context, wildcharReplace);

			result->SetPropertyAsNumber("oper", BoolLogic);
			return result;
		}

		virtual VError FromJSON(BaseTaskInfo* context, VJSONObject* obj)
		{
			RechToken::FromJSON(context, obj);
			obj->GetPropertyAsNumber("oper", &BoolLogic);
			return VE_OK;
		}

		virtual VError BuildRefString(VString& outRef, bool canMeasure, Base4D* bd)
		{
			switch (BoolLogic)
			{
				case DB4D_And:
					outRef += " and ";
					break;
				case DB4D_OR:
					outRef += " or ";
					break;
				case DB4D_Except:
					outRef += " except ";
					break;

				default:
					break;
			}

			return VE_OK;
		}

};


class RechTokenInterpOper : public RechToken
{
	public:
		sWORD oper;

		RechTokenInterpOper(void) { TypToken = r_InterpOper; };
		virtual VError PutInto(VStream& buf);
		virtual VError GetFrom(VStream& buf, Table* target);

		virtual RechToken* Clone() const;
};


class RechTokenSel : public RechToken
{
	public:
		Selection* fSel;
		sLONG fNumInstance;

		RechTokenSel(Selection* sel, sLONG numinstance) { TypToken = r_Sel; fSel = sel; sel->Retain(); fNumInstance = numinstance; };
		virtual void Dispose(void);

		virtual VError PutInto(VStream& buf);
		virtual VError GetFrom(VStream& buf, Table* target);

		virtual RechToken* Clone() const;

		virtual VError BuildRefString(VString& outRef, bool canMeasure, Base4D* bd)
		{
			outRef += "current selection";
			return VE_OK;
		}
};



class RechTokenEmSel : public RechToken
{
public:
	EntityCollection* fSel;
	sLONG fNumInstance;

	RechTokenEmSel(EntityCollection* sel, sLONG numinstance) { TypToken = r_EmSel; fSel = sel; sel->Retain(); fNumInstance = numinstance; };
	RechTokenEmSel() { TypToken = r_EmSel; fSel = nil; fNumInstance = 0; };

	virtual void Dispose(void);

	virtual VError PutInto(VStream& buf);
	virtual VError GetFrom(VStream& buf, Table* target);

	virtual RechToken* Clone() const;

	virtual VJSONObject* ToJSON(BaseTaskInfo* context, UniChar wildcharReplace = 0);
	virtual VError FromJSON(BaseTaskInfo* context, VJSONObject* obj);

	virtual VError BuildRefString(VString& outRef, bool canMeasure, Base4D* bd)
	{
		outRef += "current selection";
		return VE_OK;
	}
};


class RechTokenSimpleComp : public RechToken
{
	public:
		sLONG numfield;
		sLONG numfile;
		sLONG numinstance;
		sWORD comparaison;
		ValPtr	ch;
		VCompareOptions fOptions;
		ObjectPath fObjPath;
		Boolean fIsDataDirect;
		uBOOL fIsNeverNull;
		uBOOL fCheckForNull;
		VString fParam;
		sLONG fExpectedType;
		VRegexMatcher* fRegMatcher;

		RechTokenSimpleComp(void) { TypToken = r_Line; /*fIsDiacritic = false;*/ fIsNeverNull = 2; fCheckForNull = false; fRegMatcher = nil; };
		virtual void Dispose(void);
		virtual VError PutInto(VStream& buf);
		virtual VError GetFrom(VStream& buf, Table* target);
		void CheckIfDataIsDirect(Table* target);

		virtual RechToken* Clone() const;

		virtual void addInstance(const EntityModel* model, sLONG howmany)
		{
			/*
			if (numtable == numfile)
				numinstance += howmany;
				*/
		}
		virtual VError BuildRefString(VString& outRef, bool canMeasure, Base4D* bd);
};


class RechTokenEMSimpleComp : public RechToken
{
	public:
		const EntityAttribute* fAtt;
		sLONG numinstance;
		sWORD comparaison;
		ValPtr	ch;
		VCompareOptions fOptions;
		ObjectPath fObjPath;
		Boolean fIsDataDirect;
		uBOOL fIsNeverNull;
		uBOOL fCheckForNull;
		bool fSimpleDate;
		VString fParam;
		sLONG fExpectedType;
		VRegexMatcher* fRegMatcher;
		RechTokenSimpleComp* fCopyForLocalModel;

		RechTokenEMSimpleComp(void) { TypToken = r_LineEM; /*fIsDiacritic = false;*/ fIsNeverNull = 2; fCheckForNull = false; fRegMatcher = nil; fCopyForLocalModel = nil;};
		virtual void Dispose(void);
		virtual VError PutInto(VStream& buf);
		virtual VError GetFrom(VStream& buf, Table* target);
		void CheckIfDataIsDirect(Table* target);
		RechTokenSimpleComp* CopyIntoRechTokenSimpleComp();

		virtual VError BuildString(VString& outStr);
		virtual RechToken* Clone() const;

		virtual void addInstance(const EntityModel* model, sLONG howmany)
		{
			if (model == fAtt->GetModel()->GetRootBaseEm())
				numinstance += howmany;
		}

		virtual VJSONObject* ToJSON(BaseTaskInfo* context, UniChar wildcharReplace = 0);
		virtual VError FromJSON(BaseTaskInfo* context, VJSONObject* obj);

		virtual VError BuildRefString(VString& outRef, bool canMeasure, Base4D* bd);

};


class RechTokenRecordExist : public RechToken
{
	public:
		sLONG numfile;
		sLONG numinstance;
		uBOOL fCheckIfExists;

		RechTokenRecordExist(void) { TypToken = r_RecordExist; }
		virtual VError PutInto(VStream& buf);
		virtual VError GetFrom(VStream& buf, Table* target);
		virtual RechToken* Clone() const;

		virtual void addInstance(const EntityModel* model, sLONG howmany)
		{
			/*
			if (numtable == numfile)
				numinstance += howmany;
				*/
		}
		virtual VError BuildRefString(VString& outRef, bool canMeasure, Base4D* bd);

};



class RechTokenEntityRecordExist : public RechToken
{
	public:
		EntityModel* fModel;
		sLONG numinstance;
		uBOOL fCheckIfExists;
		RechTokenRecordExist* fCopyForLocalModel;

		RechTokenEntityRecordExist(void) { TypToken = r_RecordEntityExist; fCopyForLocalModel = nil;}
		virtual VError PutInto(VStream& buf);
		virtual VError GetFrom(VStream& buf, Table* target);
		virtual RechToken* Clone() const;
		virtual void Dispose(void);

		RechTokenRecordExist* CopyIntoRechTokenRecordExist();

		virtual void addInstance(const EntityModel* model, sLONG howmany)
		{
			if (model == fModel->GetRootBaseEm())
				numinstance += howmany;
		}

		virtual VJSONObject* ToJSON(BaseTaskInfo* context, UniChar wildcharReplace = 0);
		virtual VError FromJSON(BaseTaskInfo* context, VJSONObject* obj);

		virtual VError BuildRefString(VString& outRef, bool canMeasure, Base4D* bd);

};



class RechTokenEmComp : public RechToken
{
	public:
		AttributePath fAttPath;
		ObjectPath fObjPath;
		sWORD comparaison;
		sLONG numinstance;
		sLONG fExpectedType;
		ValPtr	ch;
		VCompareOptions fOptions;
		VString fParam;
		uBOOL fCheckForNull;
		bool fSimpleDate;
		VRegexMatcher* fRegMatcher;

		RechTokenEmComp(void) { TypToken = r_EMComp; fCheckForNull = false; fRegMatcher = nil; ch = nil;}
		virtual void Dispose(void);
		virtual VError PutInto(VStream& buf);
		virtual VError GetFrom(VStream& buf, Table* target);
		virtual RechToken* Clone() const;
		virtual VError BuildString(VString& outStr);

		virtual void addInstance(const EntityModel* model, sLONG howmany);
		virtual VError BuildRefString(VString& outRef, bool canMeasure, Base4D* bd);

};


class RechTokenScriptComp : public RechToken
{
	public:
		DB4DLanguageExpression* expression;
		sLONG numtable;
		
		RechTokenScriptComp(DB4DLanguageExpression* inExpression, sLONG inTableID) 
		{ 
			TypToken = r_Script; 
			numtable = inTableID;
			expression = inExpression;
			if (expression != nil)
				expression->Retain();
		};
		virtual void Dispose(void);
		virtual VError PutInto(VStream& buf);
		virtual VError GetFrom(VStream& buf, Table* target);

		virtual RechToken* Clone() const;

		virtual VError BuildRefString(VString& outRef, bool canMeasure, Base4D* bd)
		{
			outRef += "4D language expression";
			return VE_OK;
		}
};


class RechTokenArrayComp : public RechToken
{
public:
	sLONG numfield;
	sLONG numfile;
	sLONG numinstance;
	sWORD comparaison;
	DB4DArrayOfValues* values;
	Boolean fIsDataDirect;
	//Boolean fIsDiacritic;
	VCompareOptions fOptions;
	ObjectPath fObjPath;
	uBOOL fIsNeverNull;
	VString fParam;
	sLONG fExpectedType;

	RechTokenArrayComp(void) { TypToken = r_LineArray; values = nil; /*fIsDiacritic = false;*/ fIsNeverNull = 2;};
	virtual void Dispose(void);
	virtual VError PutInto(VStream& buf);
	virtual VError GetFrom(VStream& buf, Table* target);
	void CheckIfDataIsDirect(Table* target);

	virtual RechToken* Clone() const;

	virtual void addInstance(const EntityModel* model, sLONG howmany)
	{
		/*
		if (numtable == numfile)
			numinstance += howmany;
			*/
	}

	virtual VError BuildRefString(VString& outRef, bool canMeasure, Base4D* bd);
};


class RechTokenEmArrayComp : public RechToken
{
public:
	const EntityAttribute* fAtt;
	sLONG numinstance;
	sWORD comparaison;
	DB4DArrayOfValues* values;
	Boolean fIsDataDirect;
	//Boolean fIsDiacritic;
	VCompareOptions fOptions;
	ObjectPath fObjPath;
	uBOOL fIsNeverNull;
	bool fSimpleDate;
	VString fParam;
	sLONG fExpectedType;
	RechTokenArrayComp* fCopyForLocalModel;

	RechTokenEmArrayComp(void) { TypToken = r_LineEMArray; values = nil; /*fIsDiacritic = false;*/ fIsNeverNull = 2; fCopyForLocalModel = nil;};
	virtual void Dispose(void);
	virtual VError PutInto(VStream& buf);
	virtual VError GetFrom(VStream& buf, Table* target);
	void CheckIfDataIsDirect(Table* target);
	RechTokenArrayComp* CopyIntoRechTokenArrayComp();

	virtual RechToken* Clone() const;
//	virtual VError BuildString(VString& outStr);

	virtual void addInstance(const EntityModel* model, sLONG howmany)
	{
		if (model == fAtt->GetModel()->GetRootBaseEm())
			numinstance += howmany;
	}

	virtual VJSONObject* ToJSON(BaseTaskInfo* context, UniChar wildcharReplace = 0);
	virtual VError FromJSON(BaseTaskInfo* context, VJSONObject* obj);

	virtual VError BuildRefString(VString& outRef, bool canMeasure, Base4D* bd);

};



class RechTokenEmJoin : public RechToken
{
public:
	EntityModel* fRootBaseEm;
	const EntityAttribute* fAtt;
	sLONG numinstance;
	sWORD comparaison;
	EntityModel* fRootBaseEmOther;
	const EntityAttribute* fAttOther;
	sLONG numinstanceOther;
	VCompareOptions fOptions;

	RechTokenEmJoin(void) { TypToken = r_LineEMJoin; };
	virtual void Dispose(void);
	virtual VError PutInto(VStream& buf);
	virtual VError GetFrom(VStream& buf, Table* target);

	virtual RechToken* Clone() const;

	virtual void addInstance(const EntityModel* model, sLONG howmany)
	{
		if (model == fRootBaseEm)
			numinstance += howmany;
		if (model == fRootBaseEmOther)
			numinstanceOther += howmany;
	}

	virtual VJSONObject* ToJSON(BaseTaskInfo* context, UniChar wildcharReplace = 0);
	virtual VError FromJSON(BaseTaskInfo* context, VJSONObject* obj);

	virtual VError BuildRefString(VString& outRef, bool canMeasure, Base4D* bd);

};


class RechTokenJoin : public RechToken
{
public:
	sLONG numfield;
	sLONG numfile;
	sLONG numinstance;
	sWORD comparaison;
	sLONG numfieldOther;
	sLONG numfileOther;
	sLONG numinstanceOther;
	VCompareOptions fOptions;

	RechTokenJoin(void) { TypToken = r_LineJoin; };
	virtual void Dispose(void);
	virtual VError PutInto(VStream& buf);
	virtual VError GetFrom(VStream& buf, Table* target);

	virtual RechToken* Clone() const;

	virtual void addInstance(const EntityModel* model, sLONG howmany)
	{
		/*
		if (numtable == numfile)
			numinstance += howmany;
		if (numtable == numfileOther)
			numinstanceOther += howmany;
			*/
	}

	virtual VError BuildRefString(VString& outRef, bool canMeasure, Base4D* bd);
};



class RechTokenInterpConst : public RechToken
{
	public:
		RechTokenInterpConst()
		{
			TypToken = r_InterpConst;
		}

		VValueSingle* fVal;

		virtual RechToken* Clone() const
		{
			RechTokenInterpConst* result = new RechTokenInterpConst();
			if (fVal == nil)
				result->fVal = nil;
			else
				result->fVal = fVal->Clone();
			return result;
		}
};


class RechTokenInterpAtt : public RechToken
{
	public:
		RechTokenInterpAtt(const AttributePath& inAttPath)
		{
			TypToken = r_InterpAtt;
			fAttPath = inAttPath;
		}

		AttributePath fAttPath;

		virtual RechToken* Clone() const
		{
			RechTokenInterpAtt* result = new RechTokenInterpAtt(fAttPath);
			return result;
		}
};


class RechTokenJavaScript : public RechToken
{
	public:
		RechTokenJavaScript(const VString& inJSCode)
		{
			TypToken = r_JSScript;
			fJSCode = inJSCode;
		}

		VString fJSCode;

		virtual RechToken* Clone() const
		{
			RechTokenJavaScript* result = new RechTokenJavaScript(fJSCode);
			return result;
		}

		virtual VError BuildString(VString& outStr)
		{
			outStr = "$("+fJSCode+")";
			return VE_OK;
		}

		virtual VError BuildRefString(VString& outRef, bool canMeasure, Base4D* bd)
		{
			outRef += "JavaScript expression";
			return VE_OK;
		}
};


/*
class RechTokenArray : public VArrayVIT
{
public:
	inline RechTokenArray(sLONG pNbInit = 0,sLONG pNbToAdd = 1):VArrayVIT(sizeof(RechTokenPtr),pNbInit,pNbToAdd){;};
	inline RechTokenPtr& operator[](sLONG pIndex) {CALLRANGECHECK(pIndex);return (((RechTokenPtr*)(*fTabHandle))[pIndex]);};
};
*/

RechToken* creRechToken(sWORD typ);


									/* ================================================ */

/*
class DB4DArrayOfValuesCollection : public DB4DCollectionManager
{
public:
	inline DB4DArrayOfValuesCollection(DB4DArrayOfVValues* values) { fValues = values; };
	virtual VErrorDB4D SetCollectionSize(sLONG size);
	virtual sLONG GetCollectionSize();
	virtual sLONG GetNumberOfColumns();
	virtual	bool AcceptRawData();
	virtual CDB4DField* GetColumnRef(sLONG ColumnNumber);
	virtual VErrorDB4D SetNthElement(sLONG ElemNumber, sLONG ColumnNumber, const XBOX::VValueSingle& inValue);
	virtual VErrorDB4D SetNthElementRawData(RecIDType ElemNumber, sLONG ColumnNumber, const void *inRawValue, ValueKind inRawValueKind, bool *outRejected);
	virtual VErrorDB4D GetNthElement(sLONG ElemNumber, sLONG ColumnNumber, const XBOX::VValueSingle*& outValue, bool *outDisposeIt);
	virtual VErrorDB4D AddOneElement(sLONG ColumnNumber, const XBOX::VValueSingle& inValue);

protected:
	DB4DArrayOfVValues* fValues;
};
*/

								/* ================================================ */

class QueryValueMap : public map<VString, QueryParamElement>
{
	public:
		~QueryValueMap()
		{
			for (QueryValueMap::iterator cur = begin(), last = end(); cur != last; ++cur)
			{
				cur->second.Dispose();
			}
		}
};


class ENTITY_API SearchTab : public Obj4D, public IObjCounter
{
	public:
		SearchTab(Table* destination, Boolean CanDelete = true) 
		{ 
			fMustDisplay = false; 
			sousselect = false; 
			destFile = RetainRefCountable(destination); 
			curtoken = 0; 
			fCanDelete = CanDelete; 
			fModel = nil;
			fAllowJSCode = false;
			fWithQueryPlan = false;
			fWithQueryPath = false;
		}

		SearchTab(EntityModel* destModel, Boolean CanDelete = true) 
		{ 
			fMustDisplay = false; 
			sousselect = false; 
			destFile = nil; 
			curtoken = 0; 
			fCanDelete = CanDelete; 
			fModel = destModel;
			fAllowJSCode = false;
			fWithQueryPlan = false;
			fWithQueryPath = false;
		}


		virtual ~SearchTab();

		inline void SetTarget(Table* destination)
		{
			 CopyRefCountable(&destFile, destination); 
		}

		VJSONObject* ToJSON(BaseTaskInfo* context, UniChar wildcharReplace = 0);
		VError FromJSON(BaseTaskInfo* context, VJSONObject* jsonQuery);

		void SetSubSelect(Boolean SubSelection) { sousselect = SubSelection; };
		Boolean GetSubSelect(void) { return sousselect; };
		
		void AddSearchLineExpression(DB4DLanguageExpression* inExpression, sLONG inTableID);
		void AddSearchLineSimple(sLONG numfile, sLONG numfield, sLONG comp, const VValueSingle* ValueToCompare, Boolean isDiacritic = false, sLONG numinstance = 0, bool checkfornull = false);
		void AddSearchLineSimple(Field* cri, sLONG comp, const VValueSingle* ValueToCompare, Boolean isDiacritic = false, sLONG numinstance = 0, bool checkfornull = false);

		void AddSearchLineSimple(Field* cri, const VectorOfVString& objectPath, sLONG comp, const VValueSingle* ValueToCompare, Boolean isDiacritic = false, sLONG numinstance = 0, bool checkfornull = false);

		void AddSearchLineSimple(sLONG numfile, sLONG numfield, sLONG comp, const VValueSingle* ValueToCompare, const VCompareOptions& inOptions, sLONG numinstance = 0, bool checkfornull = false);
		void AddSearchLineSimple(Field* cri, sLONG comp, const VValueSingle* ValueToCompare, const VCompareOptions& inOptions, sLONG numinstance = 0, bool checkfornull = false);
	
		void AddSearchLineSimple(sLONG numfile, sLONG numfield, sLONG comp, const VString& inParamToCompare, const VCompareOptions& inOptions, sLONG numinstance = 0, bool checkfornull = false);
		void AddSearchLineSimple(Field* cri, sLONG comp, const VString& inParamToCompare, const VCompareOptions& inOptions, sLONG numinstance = 0, bool checkfornull = false);

		void AddSearchLineEm(AttributePath& attpath, ObjectPath& objpath, sLONG comp, const VValueSingle* ValueToCompare, const VCompareOptions& inOptions, bool checkfornull = false, sLONG numinstance = 0);
		void AddSearchLineEm(AttributePath& attpath, ObjectPath& objpath, sLONG comp, const VValueSingle* ValueToCompare, Boolean isDiacritic = false, bool checkfornull = false, sLONG numinstance = 0);
		void AddSearchLineEm(const VString& attpath, ObjectPath& objpath, sLONG comp, const VValueSingle* ValueToCompare, const VCompareOptions& inOptions, bool checkfornull = false, sLONG numinstance = 0);
		void AddSearchLineEm(const VString& attpath, ObjectPath& objpath, sLONG comp, const VValueSingle* ValueToCompare, Boolean isDiacritic = false, bool checkfornull = false, sLONG numinstance = 0);
		void AddSearchLineEm(AttributePath& attpath, ObjectPath& objpath, sLONG comp, const VString& inParamToCompare, const VCompareOptions& inOptions, sLONG numinstance = 0);

		void AddSearchLineArray(Field* cri, sLONG comp, DB4DArrayOfValues *Values, Boolean isDiacritic = false, sLONG numinstance = 0);
		void AddSearchLineArray(Field* cri, sLONG comp, DB4DArrayOfValues *Values, const VCompareOptions& inOptions, sLONG numinstance = 0);

		void AddSearchLineArray(Field* cri, sLONG comp, const VString& inParamToCompare, Boolean isDiacritic = false, sLONG numinstance = 0);
		void AddSearchLineArray(Field* cri, sLONG comp, const VString& inParamToCompare, const VCompareOptions& inOptions, sLONG numinstance = 0);

		void AddSearchLineJoin(Field* cri, sLONG comp, Field* OtherCri, Boolean isDiacritic = false, sLONG numinstance = 0, sLONG numinstanceOther = 0);
		void AddSearchLineJoinEm(const EntityAttribute* att, sLONG comp, const EntityAttribute* OtherAtt, Boolean isDiacritic, sLONG numinstance = 0, sLONG numinstanceOther = 0);

		void AddSearchLineRecordExists(sLONG numfile, uBOOL checkIfExists, sLONG numinstance);
		void AddSearchLineRecordExists(Table* inTable, uBOOL checkIfExists, sLONG numinstance);

		void AddSearchLineEntityRecordExists(EntityModel* model, uBOOL checkIfExists, sLONG numinstance);

		void AddSearchLineEmSimple(const EntityAttribute* att, ObjectPath& objpath, sLONG comp, const VValueSingle* ValueToCompare, const VCompareOptions& inOptions, sLONG numinstance = 0, bool checkfornull = false);
		void AddSearchLineEmSimple(const EntityAttribute* att, ObjectPath& objpath, sLONG comp, const VString& inParamToCompare, const VCompareOptions& inOptions, sLONG numinstance = 0, bool checkfornull = false);

		void AddSearchLineEmArray(const EntityAttribute* att, ObjectPath& objpath, sLONG comp, DB4DArrayOfValues *Values, const VCompareOptions& inOptions, sLONG numinstance = 0);
		void AddSearchLineEmArray(const EntityAttribute* att, ObjectPath& objpath, sLONG comp, const VString& inParamToCompare, const VCompareOptions& inOptions, sLONG numinstance = 0);

		void AddSearchLineBoolOper(sLONG BoolOper);
		void AddSearchLineNotOperator(void);
		void AddSearchLineOpenParenthesis(void);
		void AddSearchLineCloseParenthesis(void);

		void AddSearchLineSel(Selection* sel, sLONG numinstance = 0);
		void AddSearchLineEmSel(EntityCollection* sel, sLONG numinstance = 0);
		
		sLONG GetNbLine(void) const { return((sLONG)fLines.size()); };
		
		RechToken* GetNextToken(void) { if (curtoken<fLines.size()) return(fLines[curtoken++]); else return nil; };
		void PosToFirstToken(void) { curtoken = 0; };
		
		VError PutInto(VStream& buf);
		VError GetFrom(VStream& buf);
		
		void Dispose(void);
		Table* GetTargetFile(void) { return(destFile); };
		const Table* GetTargetFile(void) const { return(destFile); };

		inline Boolean MustDisplay() const { return fMustDisplay; };
		inline void SetDisplayTree(Boolean display) { fMustDisplay = display; };

		bool WithQueryPlan() const
		{
			return fWithQueryPlan;
		}

		bool WithQueryPath() const
		{
			return fWithQueryPath;
		}

		void SetQueryPlan(bool value)
		{
			fWithQueryPlan = true;
		}

		void SetQueryPath(bool value)
		{
			fWithQueryPath = true;
		}

		VError BuildFromString(const VString& input, VString& outOrderBy, BaseTaskInfo* context, EntityModel* model = nil, bool keepmodel = false, QueryParamElementVector* params = nil);
		VError BuildFormulaFromString(const VString& input, sLONG& curpos, EntityModel* model, RechTokenFormula* formula, sLONG curkeyword);

		Boolean WithFormulas() const;

		Boolean WithArrays() const
		{
			if (fLines.size() > 0)
			{
				RechToken* tok = fLines[0];
				if (tok->GetTyp() == r_LineArray)
					return true;
			}
			return false;
		}

		VError From(const SearchTab& other);
		VError Add(const SearchTab& other);
	
		VError ReplaceParams(BaseTaskInfo* context);
		VError GetParams(vector<VString>& outParamNames, QueryParamElementVector& outParamValues);
		VError SetParams(const vector<VString>& inParamNames, const QueryParamElementVector& inParamValues);

		inline EntityModel* GetModel() const
		{
			return fModel;
		}

		inline void SetModel(EntityModel* inModel)
		{
			fModel = inModel;
		}

		inline void AllowJSCode(bool b = true)
		{
			fAllowJSCode = b;
		}

		VError BuildQueryString(BaseTaskInfo* context, VString outQuery);

		VError ThrowError(VError err, const VString* s1 = nil);

		VError BuildRefString(VString& outRef, bool canMeasure);


	protected:
		RechTokenVector fLines;
		QueryValueMap fParamValues;
		Boolean sousselect;
		bool fAllowJSCode;
		Table* destFile;
		sLONG curtoken;
		Boolean fCanDelete, fMustDisplay, fWithQueryPlan, fWithQueryPath;
		EntityModel* fModel;
	
};



									/* ================================================ */


class VDB4DQueryPathModifiers : public CDB4DQueryPathModifiers
{
	public:
		
		virtual void SetLineDelimiter(const VString& inDelimiter);
		virtual void SetTabDelimiter(const VString& inDelimiter);
		virtual void SetTabSize(sLONG inTabSize);
		virtual void SetHTMLOutPut(Boolean inState);
		virtual void SetVerbose(Boolean inState);
		virtual void SetSimpleTree(Boolean inState, Boolean inWithTempVariables);

		virtual const VString GetLineDelimiter() const { return fLineDelimiter; };
		virtual const VString GetTabDelimiter() const { return fTabDelimiter; };
		virtual sLONG GetTabSize() const { return fTabSize; };
		virtual Boolean isHTML() const { return fOutHTML; };
		virtual Boolean isVerbose() const { return fVerbose; };
		virtual Boolean isSimpleTree() const { return fSimpleTree; };
		virtual Boolean isTempVarDisplay() const { return fTempVarDisplay; };

		VDB4DQueryPathModifiers();

	protected:
		VStr<8> fLineDelimiter;
		VStr<8> fTabDelimiter;
		sLONG fTabSize;
		Boolean fOutHTML;
		Boolean fVerbose;
		Boolean fSimpleTree;
		Boolean fTempVarDisplay;

};


									/* ================================================ */



enum { aucuntypnode=0L, NoeudSeq, NoeudIndexe, NoeudCompose, NoeudTemp, NoeudOper, NoeudScript, NoeudNot, 
			 NoeudWithArraySeq, NoeudWithArrayIndex, NoeudJoin, NoeudSubQuery, NoeudMultiOper, NoeudSel, NoeudConst, NoeudEmSeq, NoeudRecordExist,
			 NoeudJSScript};

class RechNode;
typedef RechNode *RechNodePtr;

/*
class RechNodeArray : public VArrayVIT
{
public:
	inline RechNodeArray(sLONG pNbInit = 0,sLONG pNbToAdd = 1):VArrayVIT(sizeof(RechNodePtr),pNbInit,pNbToAdd){;};
	inline RechNodePtr& operator[](sLONG pIndex) {CALLRANGECHECK(pIndex);return (((RechNodePtr*)(*fTabHandle))[pIndex]);};
};
*/

typedef Vx0ArrayOf<RechNodePtr, 10> RechNodeArray;
typedef Vx0ArrayOf<RechNode*, 10> ArrayNode;

class RechNode : public VObject
{
	
	public:
		RechNode(void) { pere = nil; fTargetTable = 0; fTargetInstance = 0; fTargetModel = nil; };
		//virtual ~RechNode() { fTargetTable = 0; };

		virtual VError Perform(OptimizedQuery *query, Bittab **super, Bittab* filtre, VDB4DProgressIndicator* InProgress, VValueBag* curdesc,
								BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, Bittab &Nulls, RechNode* &transformation);

		virtual uBOOL PerformSeq(sLONG nf, FicheInMem* curfic, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, 
									BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, LocalEntityModel* model) 
		{ 
			return(false); 
		};

		virtual uBOOL PerformFullSeq(sLONG nf, FicheInMem* curfic, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, 
									BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, LocalEntityModel* model) 
		{ 
			return PerformSeq(nf, curfic, tfb, InProgress, context,  HowToLock, exceptions, limit, model);
		};

		virtual uBOOL PerformSeq(FicheOnDisk* ficD, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, 
								BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, LocalEntityModel* model) 
		{ 
			return(false); 
		};

		virtual void ForceRecalcIndex() { ; };

		virtual Boolean IsComposed() const { return false; };

		virtual void DisposeTree(void) { };
		
		inline sLONG GetTyp(void) { return(TypNode); };
		virtual uBOOL IsConst() { return false; };
		virtual uBOOL GetResultConst() { return 0; };
		virtual uBOOL IsIndexe(void) { return(false); };
		virtual uBOOL IsAllIndexe(void) { return(false); };
		
		virtual uBOOL IsAllJoin(void) { return(false); };

		inline RechNode* GetParent(void) { return(pere); };
		inline void SetParent(RechNode* x) { pere = x; };

		virtual void DisplayTree(Base4D* bd, sLONG level, VString* result, VValueBag* bagResult = nil) = 0;
		static void FillLevelString(sLONG level, VString& s);
		virtual void Describe(Base4D* bd, VString& result) = 0;
		virtual void FullyDescribe(Base4D* bd, VString& result)
		{
			Describe(bd, result);
		}

		inline sLONG GetTarget() { return fTargetTable; };
		inline sLONG GetInstance() { return fTargetInstance; };

		inline EntityModel* GetTargetModel() {
			return fTargetModel;
		};

		inline void SetTarget(sLONG target, sLONG numinstance, EntityModel* model) { 
			fTargetTable = target; 
			fTargetInstance = numinstance;
			fTargetModel = model;
		};

		virtual CDB4DQueryPathNode* BuildQueryPath(Boolean inBefore, Table* target, Base4D* bd, sLONG DejaSel, CDB4DQueryPathNode* parent, sLONG& curvar, sLONG &outvarnum, const CDB4DQueryPathModifiers* inModifiers) = 0;
		virtual void BuildQueryPathSeq(Boolean inBefore, sLONG& curvar, CDB4DQueryPathNode* parent, Base4D* bd, VString& outString, const CDB4DQueryPathModifiers* inModifiers, Boolean first = true) { ; };

		virtual sLONG PoidsIndex() { return (IsAllIndexe() ? 1 : 0)*4 + (IsIndexe() ? 2 : 0); };

		virtual sLONG8 CountDiskAccess()
		{
			return 0;
		}

		virtual sLONG CountBlobSources()
		{
			return 0;
		}

		void AddCR(VString& s)
		{
			s += L"\n";
		}

		virtual void AdjusteIntl(VIntlMgr* inIntl)
		{ ; }

		virtual VValueSingle* Compute(LocalEntityRecord* erec, BaseTaskInfo* context, VError& err)
		{
			return nil;
		}

		virtual uBOOL PerformSeqOnEm(LocalEntityRecord* erec, BaseTaskInfo* context)
		{ 
			return(false); 
		}

		virtual bool NeedToLoadFullRec()
		{
			return false;
		}



	protected:
		RechNode* pere;
		sLONG TypNode;
		sLONG fTargetTable;
		sLONG fTargetInstance;
		EntityModel* fTargetModel;
};


class RechNodeScript : public RechNode
{
	public:
		RechNodeScript(void);
		virtual ~RechNodeScript();

		virtual uBOOL PerformSeq(sLONG nf, FicheInMem* curfic, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, 
								BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, LocalEntityModel* model);

		virtual uBOOL PerformSeq(FicheOnDisk* ficD, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, 
								BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, LocalEntityModel* model);
		
		virtual void DisplayTree(Base4D* bd, sLONG level, VString* result, VValueBag* bagResult = nil);
		virtual void Describe(Base4D* bd, VString& result);
		virtual void FullyDescribe(Base4D* bd, VString& result);
	
		virtual CDB4DQueryPathNode* BuildQueryPath(Boolean inBefore, Table* target, Base4D* bd, sLONG DejaSel, CDB4DQueryPathNode* parent, sLONG& curvar, sLONG &outvarnum, const CDB4DQueryPathModifiers* inModifiers);

		RechTokenScriptComp *rt;
		void* fLanguageContext;
		CDB4DRecord* cacherec;
		FicheInMem* cachefiche;
};


class RechNodeJSScript : public RechNode
{
	public:
		RechNodeJSScript(void) : funcobj((VJSContext*)nil)
		{
			TypNode = NoeudJSScript;
			rt = nil;
			funcInited = false;
			parseError = false;
			cachefiche = nil;

		}

		virtual ~RechNodeJSScript();

		virtual uBOOL PerformSeq(sLONG nf, FicheInMem* curfic, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, 
								BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, LocalEntityModel* model);

		virtual uBOOL PerformSeq(FicheOnDisk* ficD, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, 
								BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, LocalEntityModel* model);
		
		virtual void DisplayTree(Base4D* bd, sLONG level, VString* result, VValueBag* bagResult = nil);
		virtual void Describe(Base4D* bd, VString& result);
	
		virtual CDB4DQueryPathNode* BuildQueryPath(Boolean inBefore, Table* target, Base4D* bd, sLONG DejaSel, CDB4DQueryPathNode* parent, sLONG& curvar, sLONG &outvarnum, const CDB4DQueryPathModifiers* inModifiers);

		RechTokenJavaScript *rt;
		VJSObject funcobj;
		bool funcInited, parseError;
		FicheInMem* cachefiche;
};



class RechNodeSeq : public RechNode
{
	public:
		RechNodeSeq(void);
		// virtual ~RechNodeSeq();
		virtual uBOOL PerformSeq(sLONG nf, FicheInMem* curfic, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, 
			BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, LocalEntityModel* model);

		virtual uBOOL PerformSeq(FicheOnDisk* ficD, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, 
			BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, LocalEntityModel* model);
	
		virtual void Describe(Base4D* bd, VString& result);
		virtual void DisplayTree(Base4D* bd, sLONG level, VString* result, VValueBag* bagResult = nil);
		virtual CDB4DQueryPathNode* BuildQueryPath(Boolean inBefore, Table* target, Base4D* bd, sLONG DejaSel, CDB4DQueryPathNode* parent, sLONG& curvar, sLONG &outvarnum, const CDB4DQueryPathModifiers* inModifiers);
		virtual void BuildQueryPathSeq(Boolean inBefore, sLONG& curvar, CDB4DQueryPathNode* parent, Base4D* bd, VString& outString, const CDB4DQueryPathModifiers* inModifiers, Boolean first = true);

		virtual void AdjusteIntl(VIntlMgr* inIntl)
		{ 
			rt->fOptions.SetIntlManager(inIntl);
		}

		virtual bool NeedToLoadFullRec()
		{
			return !rt->fIsDataDirect;
		}


		RechTokenSimpleComp *rt;
};


class RechNodeEmSeq : public RechNode
{
	public:

		RechNodeEmSeq(void)
		{
			rt = nil;
			TypNode = NoeudEmSeq;
			cachefiche = nil;
		}

		virtual ~RechNodeEmSeq()
		{
			QuickReleaseRefCountable(cachefiche);
		}

		virtual uBOOL PerformSeq(sLONG nf, FicheInMem* curfic, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, 
			BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, LocalEntityModel* model);
		

		virtual uBOOL PerformSeq(FicheOnDisk* ficD, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, 
			BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, LocalEntityModel* model);
		

		virtual void Describe(Base4D* bd, VString& result);
		virtual void DisplayTree(Base4D* bd, sLONG level, VString* result, VValueBag* bagResult = nil);
		virtual CDB4DQueryPathNode* BuildQueryPath(Boolean inBefore, Table* target, Base4D* bd, sLONG DejaSel, CDB4DQueryPathNode* parent, sLONG& curvar, sLONG &outvarnum, const CDB4DQueryPathModifiers* inModifiers);
		virtual void BuildQueryPathSeq(Boolean inBefore, sLONG& curvar, CDB4DQueryPathNode* parent, Base4D* bd, VString& outString, const CDB4DQueryPathModifiers* inModifiers, Boolean first = true);

		virtual void AdjusteIntl(VIntlMgr* inIntl)
		{ 
			rt->fOptions.SetIntlManager(inIntl);
		}

		virtual VValueSingle* Compute(LocalEntityRecord* erec, BaseTaskInfo* context, VError& err);

		void CalcTarget(sLONG nf, EntityModel* model);

		RechTokenEmComp *rt;
		FicheInMem* cachefiche;

};


class RechNodeWithArraySeq : public RechNode
{
	public:
		RechNodeWithArraySeq(void);
		virtual ~RechNodeWithArraySeq();

		virtual uBOOL PerformSeq(sLONG nf, FicheInMem* curfic, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, 
			BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, LocalEntityModel* model);

		virtual uBOOL PerformSeq(FicheOnDisk* ficD, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, 
			BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, LocalEntityModel* model);

		virtual void Describe(Base4D* bd, VString& result);
		virtual void FullyDescribe(Base4D* bd, VString& result);
		virtual void DisplayTree(Base4D* bd, sLONG level, VString* result, VValueBag* bagResult = nil);
		virtual CDB4DQueryPathNode* BuildQueryPath(Boolean inBefore, Table* target, Base4D* bd, sLONG DejaSel, CDB4DQueryPathNode* parent, sLONG& curvar, sLONG &outvarnum, const CDB4DQueryPathModifiers* inModifiers);
		virtual void BuildQueryPathSeq(Boolean inBefore, sLONG& curvar, CDB4DQueryPathNode* parent, Base4D* bd, VString& outString, const CDB4DQueryPathModifiers* inModifiers, Boolean first = true);

		virtual void AdjusteIntl(VIntlMgr* inIntl)
		{ 
			rt->fOptions.SetIntlManager(inIntl);
		}

		virtual bool NeedToLoadFullRec()
		{
			return !rt->fIsDataDirect;
		}

		DB4DArrayOfValues* values;
		RechTokenArrayComp *rt;
		sLONG rechtyp;
};



class RechNodeRecordExist : public RechNode
{
public:
	RechNodeRecordExist(void)
	{
		TypNode=NoeudRecordExist;
		rt = nil;
	}

	virtual uBOOL PerformSeq(sLONG nf, FicheInMem* curfic, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, 
		BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, LocalEntityModel* model)
	{
		xbox_assert(false);
		return false;
	}

	virtual uBOOL PerformSeq(FicheOnDisk* ficD, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, 
		BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, LocalEntityModel* model)
	{
		xbox_assert(false);
		return false;
	}

	virtual sLONG PoidsIndex() { return 1000000000; };

	virtual void Describe(Base4D* bd, VString& result);
	virtual void DisplayTree(Base4D* bd, sLONG level, VString* result, VValueBag* bagResult = nil);
	virtual CDB4DQueryPathNode* BuildQueryPath(Boolean inBefore, Table* target, Base4D* bd, sLONG DejaSel, CDB4DQueryPathNode* parent, sLONG& curvar, sLONG &outvarnum, const CDB4DQueryPathModifiers* inModifiers);
	//virtual void BuildQueryPathSeq(Boolean inBefore, sLONG& curvar, CDB4DQueryPathNode* parent, Base4D* bd, VString& outString, const CDB4DQueryPathModifiers* inModifiers, Boolean first = true);

	virtual VValueSingle* Compute(LocalEntityRecord* erec, BaseTaskInfo* context, VError& err);

	RechTokenRecordExist *rt;
};




class RechNodeConst : public RechNode
{

public:
	RechNodeConst(uBOOL inResult)
	{
		fResult = inResult;
		TypNode = NoeudConst;
	}

	virtual uBOOL GetResultConst() { return fResult; };

	virtual VError Perform(OptimizedQuery *query, Bittab **super, Bittab* filtre, VDB4DProgressIndicator* InProgress,  VValueBag* curdesc,
		BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, Bittab &Nulls, RechNode* &transformation);

	virtual uBOOL IsConst() { return true; };
	virtual uBOOL IsIndexe(void) { return(true); };
	virtual uBOOL IsAllIndexe(void) { return(true); };

	virtual uBOOL PerformSeq(sLONG nf, FicheInMem* curfic, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, 
								BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, LocalEntityModel* model) 
	{ 
		return(fResult); 
	};

	virtual uBOOL PerformSeq(FicheOnDisk* ficD, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, 
							BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, LocalEntityModel* model) 
	{ 
		return(fResult); 
	};

	virtual void Describe(Base4D* bd, VString& result);
	virtual void DisplayTree(Base4D* bd, sLONG level, VString* result, VValueBag* bagResult = nil);
	virtual CDB4DQueryPathNode* BuildQueryPath(Boolean inBefore, Table* target, Base4D* bd, sLONG DejaSel, CDB4DQueryPathNode* parent, sLONG& curvar, sLONG &outvarnum, const CDB4DQueryPathModifiers* inModifiers);

	virtual VValueSingle* Compute(LocalEntityRecord* erec, BaseTaskInfo* context, VError& err);

	uBOOL fResult;
};


class RechNodeIndex : public RechNode
{

	public:
		RechNodeIndex(void);
		virtual ~RechNodeIndex();

		virtual VError Perform(OptimizedQuery *query, Bittab **super, Bittab* filtre, VDB4DProgressIndicator* InProgress,  VValueBag* curdesc,
			BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, Bittab &Nulls, RechNode* &transformation);

		void SetIndex(IndexInfo *xind) { ind=xind; };
		uBOOL PeutFourche(void);
		uBOOL PeutFourche(RechNodeIndex* autre);
		VError SubBuildCompFrom(sLONG comp, BTitemIndex *val);
		VError SubBuildCompFrom(sLONG comp, VValueSingle* val, VValueSingle* &val1, VValueSingle* &val2);
		VError BuildFrom(RechTokenSimpleComp *xrt, bool isObjInd);
		VError BuildFromMultiple(RechNode *IncludingFork, BaseTaskInfo* context);

		virtual uBOOL IsIndexe(void) { return(true); };
		virtual uBOOL IsAllIndexe(void) { return(true); };
	
		virtual void Describe(Base4D* bd, VString& result);
		virtual void DisplayTree(Base4D* bd, sLONG level, VString* result, VValueBag* bagResult = nil);
		virtual CDB4DQueryPathNode* BuildQueryPath(Boolean inBefore, Table* target, Base4D* bd, sLONG DejaSel, CDB4DQueryPathNode* parent, sLONG& curvar, sLONG &outvarnum, const CDB4DQueryPathModifiers* inModifiers);

		virtual sLONG8 CountDiskAccess();
		virtual sLONG CountBlobSources();

		inline void SetParseAllIndex(Boolean x)
		{
			fParseAllIndex = x;
		}

		virtual void AdjusteIntl(VIntlMgr* inIntl)
		{ 
			fOptions.SetIntlManager(inIntl);
			if (rt != nil)
				rt->fOptions.SetIntlManager(inIntl);
			if (rt2 != nil)
				rt2->fOptions.SetIntlManager(inIntl);
		}

		ArrayNode Nodes;
		RechTokenSimpleComp *rt, *rt2;
		RechNode* fIncludingFork;
		uBOOL isfourche;
		IndexInfo *ind;
		BTitemIndex *cle1;
		BTitemIndex *cle2;
		VCompareOptions fOptions;
		uBOOL xstrict1,xstrict2;
		uBOOL inverse, fParseAllIndex/*,fIsBeginWith, fIsLike, fIsDiacritic*/;
};



class RechNodeWithArrayIndex : public RechNode
{
	public:
		RechNodeWithArrayIndex(void);
		virtual ~RechNodeWithArrayIndex();

		virtual VError Perform(OptimizedQuery *query, Bittab **super, Bittab* filtre, VDB4DProgressIndicator* InProgress,  VValueBag* curdesc,
			BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, Bittab &Nulls, RechNode* &transformation);

		void SetIndex(IndexInfo *xind) { ind=xind; };
		VError BuildFrom(RechTokenArrayComp *xrt);

		virtual uBOOL IsIndexe(void) { return(true); };
		virtual uBOOL IsAllIndexe(void) { return(true); };

		virtual void Describe(Base4D* bd, VString& result);
		virtual void DisplayTree(Base4D* bd, sLONG level, VString* result, VValueBag* bagResult = nil);
		virtual CDB4DQueryPathNode* BuildQueryPath(Boolean inBefore, Table* target, Base4D* bd, sLONG DejaSel, CDB4DQueryPathNode* parent, sLONG& curvar, sLONG &outvarnum, const CDB4DQueryPathModifiers* inModifiers);

		virtual sLONG8 CountDiskAccess();
		virtual sLONG CountBlobSources();

		virtual void AdjusteIntl(VIntlMgr* inIntl)
		{ 
			fOptions.SetIntlManager(inIntl);
			if (rt != nil)
				rt->fOptions.SetIntlManager(inIntl);
		}

		RechTokenArrayComp *rt;
		DB4DArrayOfValues* values;
		IndexInfo *ind;
		VCompareOptions fOptions;
		uBOOL inverse/*,fIsBeginWith,fIsLike, fIsDiacritic*/;
};



class RechNodeExistingSelection : public RechNode
{
	public:
		inline RechNodeExistingSelection(Selection* sel) { TypNode = NoeudSel; fConverted = nil; fSel = sel; fSel->Retain(); };
		virtual ~RechNodeExistingSelection();

		virtual VError Perform(OptimizedQuery *query, Bittab **super, Bittab* filtre, VDB4DProgressIndicator* InProgress,  VValueBag* curdesc,
			BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, Bittab &Nulls, RechNode* &transformation);
		
		virtual uBOOL PerformSeq(sLONG nf, FicheInMem* curfic, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, 
			BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, LocalEntityModel* model);

		virtual uBOOL PerformSeq(FicheOnDisk* ficD, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, 
			BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, LocalEntityModel* model);

		uBOOL sub_PerformSeq(sLONG numfiche, BaseTaskInfo* context);

		virtual uBOOL IsIndexe(void) { return true; };
		virtual uBOOL IsAllIndexe(void) { return true; };

		virtual void Describe(Base4D* bd, VString& result);
		virtual void DisplayTree(Base4D* bd, sLONG level, VString* result, VValueBag* bagResult = nil);
		virtual CDB4DQueryPathNode* BuildQueryPath(Boolean inBefore, Table* target, Base4D* bd, sLONG DejaSel, CDB4DQueryPathNode* parent, sLONG& curvar, sLONG &outvarnum, const CDB4DQueryPathModifiers* inModifiers);
		virtual void BuildQueryPathSeq(Boolean inBefore, sLONG& curvar, CDB4DQueryPathNode* parent, Base4D* bd, VString& outString, const CDB4DQueryPathModifiers* inModifiers, Boolean first = true);


	protected:
		Selection* fSel;
		Bittab* fConverted;
};


class RechNodeOperator : public RechNode
{
	public:
		inline RechNodeOperator(void) {
			TypNode = NoeudOper; recalcIndex = true; recalcAllIndex = true; recalcAllJoin = true;
			left=nil; right=nil; // these changes are needed for valgrind
			isindexe = false; isallindexe = false; isalljoin = false; // not detected by valgrind
		};

		virtual VError Perform(OptimizedQuery *query, Bittab **super, Bittab* filtre, VDB4DProgressIndicator* InProgress,  VValueBag* curdesc,
			BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, Bittab &Nulls, RechNode* &transformation);

		virtual uBOOL PerformSeq(sLONG nf, FicheInMem* curfic, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, 
			BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, LocalEntityModel* model);

		virtual uBOOL PerformSeq(FicheOnDisk* ficD, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, 
			BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, LocalEntityModel* model);
		
		virtual uBOOL IsIndexe(void);
		virtual uBOOL IsAllIndexe(void);

		virtual uBOOL IsAllJoin(void);

		virtual void DisposeTree(void);

		virtual Boolean IsComposed() const { return true; };
		
		void SetLeft(RechNode *xleft);
		void SetRight(RechNode *xright);
		inline RechNode* GetLeft(void) { return(left); };
		inline RechNode* GetRight(void) { return(right); };
		
		virtual void Describe(Base4D* bd, VString& result);
		virtual void DisplayTree(Base4D* bd, sLONG level, VString* result, VValueBag* bagResult = nil);
		virtual CDB4DQueryPathNode* BuildQueryPath(Boolean inBefore, Table* target, Base4D* bd, sLONG DejaSel, CDB4DQueryPathNode* parent, sLONG& curvar, sLONG &outvarnum, const CDB4DQueryPathModifiers* inModifiers);
		virtual void BuildQueryPathSeq(Boolean inBefore, sLONG& curvar, CDB4DQueryPathNode* parent, Base4D* bd, VString& outString, const CDB4DQueryPathModifiers* inModifiers, Boolean first = true);

		virtual void ForceRecalcIndex() 
		{ 
			recalcIndex = true; 
			recalcAllIndex = true; 
			recalcAllJoin = true; 
		};

		virtual void AdjusteIntl(VIntlMgr* inIntl);

		virtual bool NeedToLoadFullRec();
		

		sWORD OperBool;

	protected:
		RechNode* left;
		RechNode* right;
		
		uBOOL recalcIndex, recalcAllIndex, recalcAllJoin;
		uBOOL isindexe, isallindexe;
		uBOOL isalljoin;
		
};


class RechNodeMultiOperator : public RechNode
{
	public:
		inline RechNodeMultiOperator(void) { TypNode = NoeudMultiOper; recalcIndex = true; recalcAllIndex = true; recalcAllJoin = true; recalcConst = true; };
		//virtual ~RechNodeMultiOperator() { TypNode = NoeudMultiOper; };

		virtual VError Perform(OptimizedQuery *query, Bittab **super, Bittab* filtre, VDB4DProgressIndicator* InProgress,  VValueBag* curdesc,
			BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, Bittab &Nulls, RechNode* &transformation);

		virtual uBOOL PerformSeq(sLONG nf, FicheInMem* curfic, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, 
			BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, LocalEntityModel* model);

		virtual uBOOL PerformFullSeq(sLONG nf, FicheInMem* curfic, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, 
			BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, LocalEntityModel* model);

		virtual uBOOL PerformSeq(FicheOnDisk* ficD, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, 
			BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, LocalEntityModel* model);

		virtual uBOOL IsIndexe(void);
		virtual uBOOL IsAllIndexe(void);

		virtual uBOOL IsAllJoin(void);
		virtual uBOOL IsConst() ;
		virtual uBOOL GetResultConst() { return resultconst; };

		virtual void DisposeTree(void);

		virtual void FullyDescribe(Base4D* bd, VString& result);
		virtual void Describe(Base4D* bd, VString& result);
		virtual void DisplayTree(Base4D* bd, sLONG level, VString* result, VValueBag* bagResult = nil);
		virtual CDB4DQueryPathNode* BuildQueryPath(Boolean inBefore, Table* target, Base4D* bd, sLONG DejaSel, CDB4DQueryPathNode* parent, sLONG& curvar, sLONG &outvarnum, const CDB4DQueryPathModifiers* inModifiers);
		virtual void BuildQueryPathSeq(Boolean inBefore, sLONG& curvar, CDB4DQueryPathNode* parent, Base4D* bd, VString& outString, const CDB4DQueryPathModifiers* inModifiers, Boolean first = true);

		virtual sLONG8 CountDiskAccess();
		virtual sLONG CountBlobSources();
		virtual Boolean IsComposed() const { return true; };

		virtual void ForceRecalcIndex() 
		{ 
			recalcIndex = true; 
			recalcAllIndex = true; 
			recalcAllJoin = true;
			recalcConst = true;
		};
			
		virtual void AdjusteIntl(VIntlMgr* inIntl);

		virtual bool NeedToLoadFullRec();

		sWORD OperBool;

		void AddNode(RechNode* rn);
		void DeleteNode(ArrayNode::Iterator NodePos);
		void DeleteNode(sLONG pos);
		ArrayNode& GetArrayNodes() { return Nodes; };

		virtual VValueSingle* Compute(LocalEntityRecord* erec, BaseTaskInfo* context, VError& err);

	protected:
		ArrayNode Nodes;
		uBOOL recalcIndex, recalcAllIndex, recalcAllJoin, recalcConst;
		uBOOL isindexe, isallindexe, isconst, resultconst;
		uBOOL isalljoin;
};


class RechNodeNot : public RechNodeOperator
{
	public:
		inline RechNodeNot(void) { TypNode = NoeudNot; SetRight(nil);};

		virtual VError Perform(OptimizedQuery *query, Bittab **super, Bittab* filtre, VDB4DProgressIndicator* InProgress,  VValueBag* curdesc,
			BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, Bittab &Nulls, RechNode* &transformation);


		virtual uBOOL PerformSeq(sLONG nf, FicheInMem* curfic, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, 
			BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, LocalEntityModel* model);

		virtual uBOOL PerformFullSeq(sLONG nf, FicheInMem* curfic, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, 
			BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, LocalEntityModel* model);

		virtual uBOOL PerformSeq(FicheOnDisk* ficD, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, 
			BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, LocalEntityModel* model);

		virtual void FullyDescribe(Base4D* bd, VString& result);
		virtual void Describe(Base4D* bd, VString& result);
		virtual void DisplayTree(Base4D* bd, sLONG level, VString* result, VValueBag* bagResult = nil);
		virtual CDB4DQueryPathNode* BuildQueryPath(Boolean inBefore, Table* target, Base4D* bd, sLONG DejaSel, CDB4DQueryPathNode* parent, sLONG& curvar, sLONG &outvarnum, const CDB4DQueryPathModifiers* inModifiers);
		virtual void BuildQueryPathSeq(Boolean inBefore, sLONG& curvar, CDB4DQueryPathNode* parent, Base4D* bd, VString& outString, const CDB4DQueryPathModifiers* inModifiers, Boolean first = true);
		virtual uBOOL IsConst() ;
		virtual uBOOL GetResultConst();

		virtual uBOOL IsIndexe(void);
		virtual uBOOL IsAllIndexe(void);

		virtual uBOOL IsAllJoin(void);

		virtual sLONG8 CountDiskAccess();
		virtual sLONG CountBlobSources();

		virtual void AdjusteIntl(VIntlMgr* inIntl);

		virtual VValueSingle* Compute(LocalEntityRecord* erec, BaseTaskInfo* context, VError& err);

	protected:

};


class RechNodeBaseJoin : public RechNode
{
	public:
		inline RechNodeBaseJoin(sLONG xnumtable1, sLONG xnumfield1, sLONG xnumtable2, sLONG xnumfield2, const VCompareOptions& inOptions, sLONG numinstance1, sLONG numinstance2)
		{
			TypNode = NoeudJoin; 
			fNumTable1 = xnumtable1; 
			fNumField1 = xnumfield1; 
			fNumTable2 = xnumtable2; 
			fNumField2 = xnumfield2;
			fNumInstance1 = numinstance1;
			fNumInstance2 = numinstance2;
			fOptions = inOptions;
		};
		virtual void Describe(Base4D* bd, VString& result);
		virtual void DisplayTree(Base4D* bd, sLONG level, VString* result, VValueBag* bagResult = nil);
		virtual CDB4DQueryPathNode* BuildQueryPath(Boolean inBefore, Table* target, Base4D* bd, sLONG DejaSel, CDB4DQueryPathNode* parent, sLONG& curvar, sLONG &outvarnum, const CDB4DQueryPathModifiers* inModifiers);
		virtual uBOOL IsAllJoin(void) { return(true); };

		virtual void AdjusteIntl(VIntlMgr* inIntl)
		{
			fOptions.SetIntlManager(inIntl);
		}

		sLONG fNumTable1;
		sLONG fNumField1;
		sLONG fNumTable2;
		sLONG fNumField2;
		sLONG fNumInstance1;
		sLONG fNumInstance2;
		VCompareOptions fOptions;

};



class RechNodeSubQuery : public RechNode
{
	public:
		RechNodeSubQuery(RechNode* root = nil);

		virtual ~RechNodeSubQuery();

		virtual void FullyDescribe(Base4D* bd, VString& result);
		virtual void Describe(Base4D* bd, VString& result);
		virtual void DisplayTree(Base4D* bd, sLONG level, VString* result, VValueBag* bagResult = nil);
		virtual CDB4DQueryPathNode* BuildQueryPath(Boolean inBefore, Table* target, Base4D* bd, sLONG DejaSel, CDB4DQueryPathNode* parent, sLONG& curvar, sLONG &outvarnum, const CDB4DQueryPathModifiers* inModifiers);
		virtual void BuildQueryPathSeq(Boolean inBefore, sLONG& curvar, CDB4DQueryPathNode* parent, Base4D* bd, VString& outString, const CDB4DQueryPathModifiers* inModifiers, Boolean first = true);

		inline RechNode* GetRoot() const { return fRoot; };
		inline void SetRoot(RechNode* root) { fRoot = root; };
		void SolvePath(sLONG QueryTarget, sLONG QueryInstance, BaseTaskInfo* context);

		inline void GetPathFrom(JoinPath& inPath) 
		{ 
			fPath.CopyFrom(inPath); 
		};

		void SetOneLevelPath(JoinRefCouple& couple, BaseTaskInfo* context) ;
		void ResetIndex(BaseTaskInfo* context);

		virtual uBOOL IsIndexe(void);
		virtual uBOOL IsAllIndexe(void);

		virtual VError Perform(OptimizedQuery *query, Bittab **super, Bittab* filtre, VDB4DProgressIndicator* InProgress,  VValueBag* curdesc,
			BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, Bittab &Nulls, RechNode* &transformation);

		virtual uBOOL PerformSeq(sLONG nf, FicheInMem* curfic, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, 
			BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, LocalEntityModel* model);

		virtual uBOOL PerformFullSeq(sLONG nf, FicheInMem* curfic, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, 
			BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, LocalEntityModel* model);

		virtual uBOOL PerformSeq(FicheOnDisk* ficD, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, 
			BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, LocalEntityModel* model);

		VError GenereData(VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, VValueBag* curdesc, OptimizedQuery *query);

		virtual Boolean IsComposed() const { return true; };

		virtual void AdjusteIntl(VIntlMgr* inIntl);
		virtual void DisposeTree(void);

		inline bool ContainsSubTables() const
		{
			return fContainsSubTables;
		}

		inline void MustInverse(bool b)
		{
			mustinverse = b;
		}

		RechNodeSubQuery* Clone();

		JoinPath& GetFinalPath()
		{
			return fFinalPath;
		}

		Boolean IsSolved() const
		{
			return !fUnSolved;
		}

		virtual sLONG PoidsIndex() 
		{ 
			return (IsAllIndexe() ? 1 : 0)*3 + (IsIndexe() ? 1 : 0); 
		};

		virtual bool NeedToLoadFullRec();

	protected:
		Boolean BuildFinalPath(sLONG QueryTarget, sLONG QueryInstance, SmallArrayOfBoolean &Deja, sLONG target, sLONG numinstance);

		RechNode* fRoot;
		JoinPath fPath;
		JoinPath fFinalPath;
		Boolean fUnSolved, fIsIndexed;
		bool fContainsSubTables, mustinverse;
		IndexInfo* fInd;
		IndexInfo* fOtherInd;
		DB4DArrayOfValues* data;
		sLONG rechtyp;
		VCompareOptions fOptions;
};



       /* ============================================================================== */


typedef VStackArrayRetainedPtrOf<CDB4DQueryPathNode*, 5> ArrayQueryPathNode;

class VDB4DQueryPathNode : public CDB4DQueryPathNode
{
	public:
		virtual CDB4DQueryPathNode* GetParent() const;
		virtual VError BuildFullString(VString& outResult, sLONG level);
		virtual const VString& GetThisString() const;
		virtual CDB4DQueryPathNode* GetNthChild(sLONG Nth, Boolean inBefore) const;
		virtual sLONG GetCountChildren(Boolean inBefore) const;
		virtual VError AddChild(CDB4DQueryPathNode* child, Boolean inBefore);
		virtual void SetString(const VString& data);

		VDB4DQueryPathNode(CDB4DQueryPathNode* parent, const VString& inData, const CDB4DQueryPathModifiers* inModifiers, Boolean inBefore); 
		virtual ~VDB4DQueryPathNode();

	protected:
		const CDB4DQueryPathModifiers* fModifiers; 
		VString fData;
		ArrayQueryPathNode fBeforeChildren;
		ArrayQueryPathNode fAfterChildren;
		CDB4DQueryPathNode* fParent;
		//Boolean fDisplayChildrenFirst;
};



			/* ============================================================================== */




class OptimizedQuery : public Obj4D, public IObjCounter
{
	public:
		OptimizedQuery(void) 
		{ 
			fQueryPlan = nil; 
			fExecutionDescription = nil; 
			fDescribeExecution = false; 
			/*fCurLevel = 0;*/ 
			root = nil; 
			errQuery = 0; 
			fMaxRecords = -1; 
			fic = nil; 
			fDefaultInstance = 0;
			fModel = nil;
		};

		virtual ~OptimizedQuery();
		VError AnalyseSearch(SearchTab *model, BaseTaskInfo* context, bool forcompute = false);
		Selection* Perform(Bittab* DejaSel, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, VError& err, 
							DB4D_Way_of_Locking HowToLock, sLONG limit = 0, Bittab *exceptions = nil);
		Selection* Perform(Selection* DejaSel, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, VError& err, 
							DB4D_Way_of_Locking HowToLock, sLONG limit = 0, Bittab *exceptions = nil);

		inline Table* GetTargetFile(void) { return(fic); };
		inline void SetTarget(Table* xtab, sLONG defaultInstance = 0) { fic = xtab; xbox_assert(fic!=nil); nf = fic->GetNum(); fDefaultInstance = defaultInstance;};
		inline sLONG GetDefaultInstance() { return fDefaultInstance; };

		inline RechNode* GetRoot() { return root; };
		inline void SetRoot(RechNode* xroot) { root = xroot; };
		CDB4DQueryPathNode* BuildQueryPath(Boolean DejaSel, const CDB4DQueryPathModifiers* inModifiers);
		VError BuildQueryPathString(Boolean DejaSel, VString& outResult, const CDB4DQueryPathModifiers* inModifiers);

		VError MaximizeBittab(Bittab* b);

		//inline void SetLevel(sLONG n) { fCurLevel = n; };
		//inline void IncLevel() { fCurLevel++; };
		//inline void DecLevel() { fCurLevel--; };
		//inline sLONG GetLevel() const { return fCurLevel; };

		inline void DescribeQuery(Boolean describe) { fDescribeExecution = describe; };
		inline Boolean IsQueryDescribed() const { return fDescribeExecution; };

		inline uLONG GetStartingTime() const
		{
			if (fDescribeExecution)
				return VSystem::GetCurrentTime();
			else
				return 0;
		};

		sLONG8 CalculateNBDiskAccessForSeq(Bittab* filtre, BaseTaskInfo* context, sLONG CountBlobSources = 0);

		VValueBag* AddToDescription(VValueBag* root, const VString s, uLONG startingMillisecs, Bittab* b);

		RechNode* xTransformSeq(RechNode* rn, uBOOL seq);

		inline VValueBag* GetExecutionDescription() const
		{
			return fExecutionDescription;
		}

		inline VValueBag* GetQueryPlan() const
		{
			return fQueryPlan;
		}

		VValueSingle* Compute(LocalEntityRecord* erec, BaseTaskInfo* context, VError& err);

		static void BuildQueryFulltext(VValueBag* step, VString& fulltext, sLONG level);

		inline sLONG GetMaxRecords() const
		{
			return fMaxRecords;
		}

		inline LocalEntityModel* GetModel()
		{
			return fModel;
		}

		inline void SetModel(EntityModel* model)
		{
			if (model == nil)
				fModel = nil;
			else
				fModel = dynamic_cast<LocalEntityModel*>(model);
		}

	protected:
		RechNode* xBuildNodes(SearchTab *model, RechNode* dejaleft);
		RechNode* xFlattenNodes(RechNode* rn, RechNodeMultiOperator* encours);
		RechNode* xTransformIndex(RechNode* rn, const Table* curtarget, BaseTaskInfo* context);
		//RechNode* xCreerFourche(RechNode* rn, RechNodeArray *deja);
		RechNode* xPurgeNilAndSolvePath(RechNode* rn, BaseTaskInfo* context);
		RechNode* xRemoveRecordExist(RechNode* rn, uBOOL& mustinverse);
		RechNode* xRegroupJoinPaths(RechNode* rn, bool checkForSubQueryAlone, BaseTaskInfo* context, sLONG subTarget, sLONG subInstance);
		
		/*
		uBOOL xPermuteAnds_FullIndex(RechNode* rn, RechNode** Permutant);
		uBOOL xPermuteAnds_PartialIndex(RechNode* rn, RechNode** Permutant);
		*/

		RechNode* xPermuteAnds(RechNode* rn);


		void xBuildSubQueries(RechNode* rn, RechNodeArray *deja, JoinPath& xPath);

		RechNode* xBuildSubQueryFrom(RechNodeArray *ListOfNodes, JoinPath& xPath);
		sLONG xGetSubQueryTarget(RechNode* rn, sLONG& outTargetInstance);
		RechNode* xStripSubQuery(RechNode* rn);
		void xAddToSubQuery(RechNodeArray *ListOfNodes, sLONG numtable, RechNode* rn, sLONG numinstance);

		RechNode* xSplitSubQueryOnSubTables(RechNode* rn, BaseTaskInfo* context, RechNodeSubQuery* DejaDansSousTables);

		//uBOOL xPermuteAnds_Joins(RechNode* rn, RechNode** Permutant);

		RechNode* root;
		sLONG errQuery;
		sLONG nf;
		sLONG fDefaultInstance;
		sLONG fMaxRecords;
		sLONG8 fStartingTime;
		Table *fic;
		VString fResult, fRefStr;
		//sLONG fCurLevel;
		Boolean fDescribeExecution;
		VValueBag* fExecutionDescription;
		VValueBag* fQueryPlan;
		LocalEntityModel* fModel;
};



/* ============================================================================== */


class ColumnFormulae : public VObject
{
	public:
		ColumnFormulae();
		virtual ~ColumnFormulae();

		void Dispose();

		Boolean operator == (const ColumnFormulae& other) const { return false; };

		VError SetAction(DB4D_ColumnFormulae inWhatToDo, Field* InField);
		inline	DB4D_ColumnFormulae GetAction() const { return fWhatToDo; };
		//void SetIndex(IndexInfo* ind);

		inline sLONG8* GetResultLong8Ptr() { return &fResult1; };
		inline Real* GetResultRealPtr() { return &fResult2; };
		inline VFloat* GetResultFloatPtr() { return &fResult3; };
		inline VDuration* GetResultDurationPtr() { return &fResult4; };

		void SetResult(sLONG8 result, sLONG xcount);
		void SetResult(Real result, sLONG xcount);

		inline void AddDistinctLong8(sLONG8 value)
		{
			if (fFirstTime || value != fPreviousValue1)
			{
				countDistinct++;
				fResult1 = fResult1 + value;
				fPreviousValue1 = value;
			}
		}

		inline void AddDistinctReal(Real value)
		{
			if (fFirstTime || value != fPreviousValue2)
			{
				countDistinct++;
				fResult2 = fResult2 + value;
				fPreviousValue2 = value;
			}
		}

		inline void AddDistinctFloat(const VFloat& value)
		{
			if (fFirstTime || value != fPreviousValue3)
			{
				countDistinct++;
				fResult3.Add(fResult3, value);
				fPreviousValue3 = value;
			}
		}

		inline void AddDistinctDuration(const VDuration& value)
		{
			if (fFirstTime || value != fPreviousValue4)
			{
				countDistinct++;
				fResult4.Add(value);
				fPreviousValue4 = value;
			}
		}

		void AddDistinctGeneric(const VValueSingle* value)
		{
			if (fGenericResult == nil)
			{
				countDistinct++;
				fGenericResult = value->Clone();
			}
			else
			{
				if (fFirstTime || !value->EqualToSameKind(fGenericResult, true))
				{
					countDistinct++;
					fGenericResult->FromValue(*value);
				}
			}
		}

		void SetGenericValue(const VValueSingle* value)
		{
			if (fGenericResult == nil)
			{
				fGenericResult = value->Clone();
			}
			else
			{
				fGenericResult->FromValue(*value);			
			}
		}

		inline VValueSingle* GetGenericValue()
		{
			return fGenericResult;
		}

		VValueSingle* GetResult() const;

		VValueSingle* GetCurrecValue()
		{
			return fCurrecValue;
		}

		void SetCurrecvalue(VValueSingle* val)
		{
			if (fCurrecValue != nil)
				delete fCurrecValue;
			if (val == nil)
				fCurrecValue = val;
			else
				fCurrecValue = (VValueSingle*)val->Clone();
		}

		inline ValueKind GetResultType() const { return fResultType; };
		inline Field* GetField() const { return fCri; };
		//inline IndexInfo* GetIndex() { return fInd; };
		inline void AddCount() { count++; };
		inline sLONG8 GetCount() const { return count; };

		VError CopyFrom(ColumnFormulae* other, sLONG nformule);
		VError CopyResultBackInto(ColumnFormulas* into);

		VError Finalize();

		inline Boolean FirstTime() { Boolean x = fFirstTime; fFirstTime = false; return x; };
		inline Boolean isNull() { return fFirstTime; };

		VError WriteToStream(VStream* toStream, Table* tt);
		VError ReadFromStream(VStream* fromStream, Table* tt);

		VError WriteResultsToStream(VStream* toStream);
		VError ReadResultsFromStream(VStream* fromStream);



	protected:
		DB4D_ColumnFormulae fWhatToDo;
		Field* fCri;
		sLONG8 fResult1;
		Real fResult2;
		VFloat fResult3;
		VDuration fResult4;
		VValueSingle* fGenericResult;

		sLONG8 fPreviousValue1;
		Real fPreviousValue2;
		VFloat fPreviousValue3;
		VDuration fPreviousValue4;

		VValueSingle* fCurrecValue;

		//IndexInfo* fInd;
		sLONG8 count;
		sLONG8 countDistinct;
		ValueKind fResultType;
		sLONG fFromNthFormule;
		Boolean fFirstTime, fSpecialBool;
};


class ColumnFormulas : public VObject
{
	public:
		ColumnFormulas(Table* target = nil);
		virtual ~ColumnFormulas();

		Boolean operator == (const ColumnFormulas& other) const { return false; };

		void SetTarget(Table* target);
		void SetIndex(IndexInfo* ind);
		inline IndexInfo* GetIndex() const { return fInd; };
		VError AddAction(DB4D_ColumnFormulae inWhatToDo, Field* InField);
		//VValueSingle* GetResult(sLONG WhatAction);
		VError Execute(Selection* Sel, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, FicheInMem* currec, bool keepSel = false);
		Boolean ExecuteOnOneRec(FicheOnDisk* ficD, BaseTaskInfo* context);
		Boolean ExecuteOnOneRec(FicheInMem* fic, BaseTaskInfo* context);
		VError ExecuteOnOneBtreeKey(IndexInfo* ind, const BTitemIndex* u, Boolean& stop, BaseTaskInfo* context);

		template <class Type>
		VError ExecuteOnOneKey(IndexInfo* ind, Type data, sLONG numrec, Boolean& stop, BaseTaskInfo* context);
	
		VError CopyActionFrom(ColumnFormulae *from, sLONG nformule);
		VError CopyResultBackInto(ColumnFormulas* into);
		inline ColumnFormulae* GetNthFormule(sLONG n) { return &fActions[n]; };

		sLONG8 GetResultAsLong8(sLONG inColumnNumber) const;
		Real GetResultAsReal(sLONG inColumnNumber) const;
		void GetResultAsFloat(sLONG inColumnNumber, VFloat& outResult) const;
		void GetResultAsDuration(sLONG inColumnNumber, VDuration& outResult) const;

		VValueSingle* GetResult(sLONG inColumnNumber) const;

		VError Finalize();

		VError WriteToStream(VStream* toStream, Base4D* db);
		VError ReadFromStream(VStream* fromStream, Base4D* db);

		VError WriteResultsToStream(VStream* toStream);
		VError ReadResultsFromStream(VStream* fromStream);

		inline Table* GetTarget() const
		{
			return fTarget;
		}

		void SetCurrec(FicheInMem* currec);

		inline sLONG GetNbActions() const
		{
			return (sLONG)fActions.GetCount();
		}

		inline ColumnFormulae* GetAction(sLONG n)
		{
			return &(fActions[n]);
		}

		inline sLONG GetCurrec() const
		{
			return fCurrec;
		}

	protected:
		Table* fTarget;
		Vx0ArrayOf<ColumnFormulae, 10> fActions;
		IndexInfo* fInd;
		sLONG fCurrec;
};



//=================================================================================



class QueryOptions : public VObject, public IRefCountable
{
	public:

		QueryOptions()
		{
			fFilterTable = nil;
			fFilter = nil;
			fLimit = 0;
			fHowToLock = DB4D_Do_Not_Lock;
			fRecordHowToLock = DB4D_Do_Not_Lock;
			fWantsLockedSet = false;
			fWantsFirstRecord = false;
			fDestination = DB4D_QueryDestination_Selection;
			fWantToDescribe = 2;

		}

		virtual ~QueryOptions()
		{
			ReleaseRefCountable(&fFilter);
		}

		void SetFilter( Selection* inFilter)
		{
			CopyRefCountable(&fFilter, inFilter);
		}

		void SetLimit(sLONG8 inNewLimit)
		{
			fLimit = inNewLimit;
		}

		void SetWayOfLocking(DB4D_Way_of_Locking HowToLock)
		{
			fHowToLock = HowToLock;
		}

		void SetRecordWayOfLocking(DB4D_Way_of_Locking HowToLock)
		{
			fRecordHowToLock = HowToLock;
		}

		void SetWantsLockedSet(Boolean WantsLockedSet)
		{
			fWantsLockedSet = WantsLockedSet;
		}

		void SetWantsFirstRecord(Boolean WantsFirstRecord)
		{
			fWantsFirstRecord = WantsFirstRecord;
		}

		void SetDestination(DB4D_QueryDestination inDestination)
		{
			fDestination = inDestination;
		}

		void SetDescribeQuery(Boolean describe)
		{
			fWantToDescribe = describe ? 1 : 0;
		}

		void SetFilterTable(Table* inFiltertable)
		{
			fFilterTable = inFiltertable;
		}

		VError ToServer(VStream* into, BaseTaskInfo* context);

		VError FromClient(VStream* from, Base4D* inBase, Table* inTable, BaseTaskInfo* context);

		inline Selection* GetFilter() const
		{
			return fFilter;
		}

		inline sLONG8 GetLimit() const
		{
			return fLimit;
		}

		inline DB4D_Way_of_Locking GetWayOfLocking() const
		{
			return fHowToLock;
		}

		inline DB4D_Way_of_Locking GetRecordWayOfLocking() const
		{
			return fRecordHowToLock;
		}

		inline Boolean GetWantsLockedSet() const
		{
			return fWantsLockedSet;
		}

		inline Boolean GetWantsFirstRecord() const
		{
			return fWantsFirstRecord;
		}

		inline DB4D_QueryDestination GetDestination() const
		{
			return fDestination;
		}

		inline Boolean ShouldDescribeQuery() const
		{
			return fWantToDescribe == 1;
		}

		inline Boolean IsDescribeUndef() const
		{
			return fWantToDescribe == 2;
		}

		inline Table* GetFiltertable() const
		{
			return fFilterTable;
		}

	protected:
		Table* fFilterTable;
		Selection* fFilter;
		sLONG8 fLimit;
		DB4D_Way_of_Locking fHowToLock, fRecordHowToLock;
		Boolean fWantsLockedSet, fWantsFirstRecord;
		uBYTE fWantToDescribe;
		DB4D_QueryDestination fDestination;

};

class VDB4DSet;

class QueryResult : public VObject, public IRefCountable
{
	public:

		QueryResult()
		{
			fSel = nil;
			fSet = nil;
			fCount = -1;
			fLockedSet = nil;
			fFirstRec = nil;
		}

		virtual ~QueryResult()
		{
			ReleaseRefCountable(&fSel);
			ReleaseRefCountable(&fSet);
			ReleaseRefCountable(&fLockedSet);
			ReleaseRefCountable(&fFirstRec);
		}

		Selection* GetSelection()
		{
			return fSel;
		}

		Bittab* GetSet()
		{
			return fSet;
		}

		sLONG8 GetCount()
		{
			return fCount;
		}

		Bittab* GetLockedSet()
		{
			return fLockedSet;
		}

		FicheInMem* GetFirstRecord()
		{
			return fFirstRec;
		}

		VString& GetQueryDescription()
		{
			return fQueryDescription;
		}

		VString& GetQueryExecutionXML()
		{
			return fQueryExecutionXML;
		}

		VString& GetQueryExecution()
		{
			return fQueryExecution;
		}

		VError ToClient(VStream* into, BaseTaskInfo* context);

		VError FromServer(VStream* from, Base4D* inBase, Table* inTable, BaseTaskInfo* context);

		void SetSet(Bittab* set)
		{
			CopyRefCountable(&fSet, set);
		}

		void SetSelection(Selection* sel)
		{
			CopyRefCountable(&fSel, sel);
		}

		void SetLockedSet(Bittab* set)
		{
			CopyRefCountable(&fLockedSet, set);
		}

		void SetFirstRecord(FicheInMem* rec)
		{
			CopyRefCountable(&fFirstRec, rec);
		}

		void SetCount(sLONG8 newcount)
		{
			fCount = newcount;
		}

		void SetQueryDescription(VString& s)
		{
			fQueryDescription = s;
		}

		void SetQueryExecutionXML(VString& s)
		{
			fQueryExecutionXML = s;
		}

		void SetQueryExecution(VString& s)
		{
			fQueryExecution = s;
		}

	protected:
		Selection* fSel;
		Bittab* fSet;
		sLONG8 fCount;
		Bittab* fLockedSet;
		FicheInMem* fFirstRec;
		VString fQueryDescription;
		VString fQueryExecutionXML;
		VString fQueryExecution;

};



#endif
