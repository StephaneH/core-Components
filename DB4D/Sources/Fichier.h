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
#ifndef __FICHIER4D__
#define __FICHIER4D__

#include "TabAddr.h"


									/* -----------------------------------------------  */



typedef AddrDBObj TabAddrDisk[kNbElemTabAddr];

class IndexInfo;
class Relation;

typedef IndexInfo* IndexInfoptr;
typedef Relation* Relationptr;

typedef Vx1ArrayOfRetainedPtr<IndexInfoptr, 20> IndexArray;
typedef Vx1ArrayOfRetainedPtr<Relationptr, 10> DepRelationArray;

typedef IndexArray IndexArrayIncluded;
typedef DepRelationArray DepRelationArrayIncluded;

/*
typedef Vx1ArrayOf<IndexInfoptr, 20> IndexNonRetainArray;
typedef Vx1ArrayOf<Relationptr, 10> DepRelationNonRetainArray;
*/


class Table;
class CVariable;
class CClass;
class CMethod;
class AutoSeqNumber;
class IReplicationInputFormatter;

const sLONG kMaxXStringSize = kMaxXStringLen*2+8;

class Critere : public Obj4D, public IObjCounter
{
	public:
		inline Critere(CritereDISK* crd) { CRD = crd; };
#ifdef __MWERKS__
		//inline Critere(void) : IndexDep() { ; };	BG; 06/04/98
#endif
		void GetUUID(VUUID& outID) const;
		void SetUUID(const VUUID& inID);
		inline Boolean EqualUUID( const VUUID& inUUID) const	{return inUUID.EqualToSameKindPtr( &CRD->ID, false);}

		UniChar* GetName(void) const { return((UniChar*)&CRD->nom); };
		void GetName(VString& name) const;
		VError SetName(const VString& name);
		sLONG GetTyp(void) const { return(CRD->typ); };
		void setCRD( const CritereDISK *crd1);
		CritereDISK* getCRD(void) { return(CRD); };
		//virtual sLONG GetMaxLen()=0;
		//virtual sLONG GetMaxChars()=0;
		
		inline sLONG GetLimitingLen(void) const 
		{ 

			if (CRD->LimitingLength < 0)
				return 0;
			else
				return(CRD->LimitingLength); 
		};

		inline void SetLimitingLen(sLONG len) 
		{ 
			if (len<0) 
				len = 0; 
			CRD->LimitingLength = len; 
		};
		
		inline sLONG GetTextSwitchSize(void) const 
		{ 
			if (CRD->LimitingLength > 0)
				return 0;
			else
				return(-CRD->LimitingLength); 
		};

		inline void SetTextSwitchSize(sLONG len) 
		{ 
			if (len<0) 
				len = 0; 
			CRD->LimitingLength = -len; 
		};

		inline sLONG GetBlobSwitchSize(void) const 
		{ 
			if (CRD->LimitingLength > 0)
				return 0;
			else
				return(-CRD->LimitingLength); 
		};

		inline void SetBlobSwitchSize(sLONG len) 
		{ 
			if (len<0) 
				len = 0; 
			CRD->LimitingLength = -len; 
		};

		virtual void CalcDependences(Table* fic, sLONG numfield);
		
		inline Boolean GetNot_Null(void) { return((Boolean)CRD->not_null); };
		inline void SetNot_Null(Boolean x) { CRD->not_null = x; };

		inline Boolean GetUnique(void) { return((Boolean)CRD->unique);};
		inline void SetUnique(Boolean x) { CRD->unique = x; };

		inline Boolean GetAutoSeq(void) { return((Boolean)CRD->fAutoSeq);};
		inline void SetAutoSeq(Boolean x) { CRD->fAutoSeq = x; };

		inline Boolean GetAutoGenerate(void) { return((Boolean)CRD->fAutoGenerate);};
		inline void SetAutoGenerate(Boolean x) { CRD->fAutoGenerate = x; };

		inline Boolean GetStoreUTF8(void) { return((Boolean)CRD->fStoreAsUTF8);};
		inline void SetStoreUTF8(Boolean x) { CRD->fStoreAsUTF8 = x; };

		inline Boolean IsNeverNull(void) { return((Boolean)CRD->fNeverNull);};
		inline void SetNeverNull(Boolean x) { CRD->fNeverNull = x; };

		inline Boolean GetOutsideData(void) { return((Boolean)CRD->fOuterData);};
		inline void SetOutsideData(Boolean x) { CRD->fOuterData = x; };
		
		inline Boolean GetStyledText(void) { return((Boolean)CRD->fTextIsWithStyle);};
		inline void SetStyledText(Boolean x) { CRD->fTextIsWithStyle = x; };

		inline Boolean GetHideInRest(void) { return((Boolean)CRD->fHideInRest);};
		inline void SetHideInRest(Boolean x) { CRD->fHideInRest = x; };

		virtual Boolean IsIndexable(void) { return(true); };
		virtual Boolean IsFullTextIndexable(void) { return(false); };
		virtual Boolean IsTextable(void) { return(true); };
		virtual Boolean IsSortable(void) { return(true); };
		virtual Boolean IsANum(void) { return(false); };
		virtual Boolean IsSimple(void) { return(true); };
		virtual Boolean IsABlobHandle(void) { return(false); };
		virtual Boolean IsPictable(void) { return(false); };
		virtual Boolean IsDataDirect(void) const { return true; };
		virtual Boolean CanBeUnique(void) const { return true; };
		virtual Boolean CanBeNot_Null(void) const { return true; };
		virtual Boolean CanBePartOfRelation() const { return true; };
		
//		virtual VVariableKind GetLangKind(void) = 0;
//		virtual CClass* GetLangKindClass(void) = 0;

		virtual sLONG GetTypeSize() const = 0;

		virtual ValueKind GetTypeForLexicalIndex() const
		{
			return VK_STRING;
		}

		inline sLONG GetStamp() const { return CRD == nil ? 0 : CRD->fStamp; };
		inline void SetStamp(sLONG stamp) 
		{ 
			if (CRD != nil)
				CRD->fStamp = stamp; 
		};
						
		
	protected:
		CritereDISK* CRD;
};


class Field : public Obj4D, public IObjCounter, public IDebugRefCountable, public IBaggable
{
	public:
		Field(sLONG typ, sLONG pos, Table* owner, Boolean isremote = false);
		Field( const CritereDISK *criD, sLONG pos, Table* owner, Boolean isremote = false);

		// remote constructor
		Field( const CritereDISK *criD, VValueBag *inExtraProperties, sLONG pos, Table* owner);
		
		// pour creer un champ qui sera initialise dans LoadFromBag
		Field( Table *inTable, Boolean isremote = false);
		
		virtual ~Field();
		
		void xInit();
		
		VError CopyFromField(Field *other, CDB4DBaseContext* inContext, VProgressIndicator* progress = nil);
		
		void RemoveUniqueFromIndexes();
		VError TryUnique(VProgressIndicator* progress);
		VError TryNot_Null(VProgressIndicator* progress);

		VError SetTyp(sLONG typ);

		VError setCRD( const CritereDISK *crd1, CDB4DBaseContext* inContext, Boolean inNotify = true, VProgressIndicator* progress = nil);
		VError UpdateField(const CritereDISK *crd1, VValueBag *inExtraProperties);
		CritereDISK* getCRD(void) { return(&fCRD); };
		sLONG GetPosInRec(void) const { return PosInRec; };
		void SetPosInRec(sLONG p);

		void GetUUID(VUUID& outID) const;
		void SetUUID(const VUUID& inID);
		inline Boolean EqualUUID( const VUUID& inUUID) const	{return cri->EqualUUID( inUUID);}

		Table* GetOwner(void) const { return Owner; };
		UniChar* GetName(void) const { return(cri->GetName()); };
		void GetName(VString& name) const { cri->GetName(name); };
		VError SetName(const VString& name, CDB4DBaseContext* inContext);
		sLONG GetTyp(void) const { return(cri->GetTyp()); };
		//sLONG GetMaxLen() { return(cri->GetMaxLen()); };
		void AddIndexDep(IndexInfo* ind);
		void DelIndexDep(IndexInfo* ind);
		void ClearIndexDep(void);
		uBOOL IsIndexe(void) const { return(IndexDep.GetCount()!=0); };
		uBOOL IsIndexed(void) const;
		uBOOL IsFullTextIndexed(void) const;
		uBOOL IsPrimIndexe(void) const { return(IndexDep.GetCount()!=0); };
		void CalcDependences(Table* fic, sLONG numfield);
		
		inline Table* GetFirstLinkedFile(void) { return nil; }; // il faudra le faire avec les Relations
		inline Boolean IsFirstLinkAuto(void) { return false; }; // il faudra le faire avec les Relations
		virtual Table* GetSubFile(void) { return nil; };
		
		sLONG GetMaxLenForSort() const;

		sLONG GetSwitchSize() const;

		inline sLONG GetTextSwitchSize(void) const { return(cri->GetTextSwitchSize()); };
		VError SetTextSwitchSize(sLONG len, CDB4DBaseContext* inContext);

		inline sLONG GetBlobSwitchSize(void) const { return(cri->GetBlobSwitchSize()); };
		VError SetBlobSwitchSize(sLONG len, CDB4DBaseContext* inContext);

		inline sLONG GetLimitingLen(void) const { return(cri->GetLimitingLen()); };
		void SetLimitingLen(sLONG len, CDB4DBaseContext* inContext);

		inline Boolean GetNot_Null(void) { return(cri->GetNot_Null()); };
		VError SetNot_Null(Boolean x, VProgressIndicator* progress, CDB4DBaseContext* inContext);
	
		inline Boolean GetUnique(void) { return(cri->GetUnique()); };
		VError SetUnique(Boolean x, VProgressIndicator* progress, CDB4DBaseContext* inContext);
	
		inline Boolean GetAutoSeq(void) { return(cri->GetAutoSeq() || cri->GetTyp() == VK_SUBTABLE); };
		void SetAutoSeq(Boolean x, CDB4DBaseContext* inContext);

		inline Boolean GetAutoGenerate(void) { return(cri->GetAutoGenerate()); };
		void SetAutoGenerate(Boolean x, CDB4DBaseContext* inContext);

		inline Boolean GetStoreUTF8(void) { return(cri->GetStoreUTF8()); };
		void SetStoreUTF8(Boolean x, CDB4DBaseContext* inContext)  { cri->SetStoreUTF8(x); };

		inline Boolean IsNeverNull(void) const { return cri->IsNeverNull(); };
		VError SetNeverNull(Boolean x, CDB4DBaseContext* inContext);

		inline Boolean GetOutsideData(void) { return cri->GetOutsideData(); };
		void SetOutsideData(Boolean x, CDB4DBaseContext* inContext);

		inline Boolean GetStyledText(void) { return cri->GetStyledText(); };
		void SetStyledText(Boolean x, CDB4DBaseContext* inContext, VDB4DProgressIndicator* InProgress);

		inline Boolean GetHideInRest(void) { return cri->GetHideInRest(); };
		void SetHideInRest(Boolean x, CDB4DBaseContext* inContext, VDB4DProgressIndicator* InProgress);

		Boolean IsIndexable(void) { return(cri->IsIndexable()); };
		Boolean IsFullTextIndexable(void) { return(cri->IsFullTextIndexable()); };
		Boolean IsTextable(void) { return(cri->IsTextable()); };
		Boolean IsSortable(void) { return(cri->IsSortable()); };
		Boolean IsANum(void) { return(cri->IsANum()); };
		Boolean IsSimple(void) { return(cri->IsSimple()); };
		Boolean IsABlobHandle(void) { return(cri->IsABlobHandle()); };
		Boolean IsPictable(void) { return(cri->IsPictable()); };
		Boolean IsDataDirect(void) const { return cri->IsDataDirect(); };
		Boolean CanBeUnique(void) const { return cri->CanBeUnique(); };
		Boolean CanBeNot_Null(void) const { return cri->CanBeNot_Null(); };
		Boolean CanBePartOfRelation() const { return cri->CanBePartOfRelation(); };
			
		// from IBaggable
		virtual VError	LoadFromBagWithLoader( const VValueBag& inBag, VBagLoader *inLoader, void* inContext = nil);
		virtual VError	SaveToBag( VValueBag& ioBag, VString& outKind) const;
		
		// build a field_ref element referencing this field.
		// if inForExport is true, additionnal info may be added to be able to find this field again after export/import.
		VValueBag *CreateReferenceBag( bool inForExport) const;
		
		void RegisterForLang();
		void UnRegisterForLang(void);
		
		void DropAllRelations(CDB4DBaseContext* inContext);

		void DropAllRelatedIndex(CDB4DBaseContext* inContext, VProgressIndicator* progress = nil);
		void RebuildAllRelatedIndex(CDB4DBaseContext* inContext, sLONG NeverNullChange = 0, VProgressIndicator* progress = nil);

		sLONG CalcAvgSize() const;
		sLONG CalcDataSize(const void* p) const;
		CompareResult CompareKeys(const void *val1, const void *val2, const VCompareOptions& inOptions);
		
		void AddRelationNto1Dep(Relation* rel);
		void DelRelationNto1Dep(Relation* rel);
		void ClearRelationNto1Dep(void);

		void AddRelation1toNDep(Relation* rel);
		void DelRelation1toNDep(Relation* rel);
		void ClearRelation1toNDep(void);

		Relation* GetSubTableRel();

		void ClearAllDependencies();

		VError ThrowError( VError inErrCode, ActionDB4D inAction);

		inline const VValueSingle* GetEmptyValue() const { return util; };
		inline const void* GetEmptyValPtr() const { return fEmptyValPtr; };

		inline const VValueBag* RetainExtraProperties(VError &err, CDB4DBaseContext* inContext) { return fExtra.RetainExtraProperties(err); };
		VError SetExtraProperties(VValueBag* inExtraProperties, bool inNotify, CDB4DBaseContext* inContext, bool loadonly = false);

		inline DepRelationArrayIncluded* GetRel1toNDeps() { return &RelDep1toN; };
		inline DepRelationArrayIncluded* GetRelNto1Deps() { return &RelDepNto1; };

		inline sLONG GetTypeSize() const { return cri->GetTypeSize(); };

		void Touch();
		inline sLONG GetStamp() const { return fStamp; };

		VError RetainIndexes(ArrayOf_CDB4DIndex& outIndexes);
		IndexInfo* FindAndRetainIndexSimple(uBOOL sortable, Boolean MustBeValid, BaseTaskInfo* context = nil) const;
		IndexInfo* FindAndRetainIndexLexico(Boolean MustBeValid, BaseTaskInfo* context = nil) const;

		Boolean IsRemote() const { return fIsRemote; };

		ValueKind GetTypeForLexicalIndex() const
		{
			return cri->GetTypeForLexicalIndex();
		}

		inline Critere* GetCritere()
		{
			return cri;
		}

		bool IsPrimKey();

		inline bool IsValid() const
		{
			return (cri != nil);
		}

#if debugLeaksAll
		virtual bool OKToTrackDebug() const
		{
			return false;
		}

		virtual void GetDebugInfo(VString& outText) const
		{
			VString s;
			GetName(s);
			outText = "Field : "+s;
		}

#endif

	protected:
		void	CreateEmptyValue();
		
		Critere* cri;
		IndexArrayIncluded IndexDep;
		DepRelationArrayIncluded RelDepNto1;
		DepRelationArrayIncluded RelDep1toN;
		sLONG PosInRec;
		Table* Owner;
		CVariable* FieldVar;
		CVariable* FieldRefVar;
		sLONG oldtypEnregistre;
		Boolean fRegistered;
		Boolean fIsRemote;
		const VValueSingle* util;
		const void* fEmptyValPtr;
		const VValueInfo*	fValueInfo;
		ExtraPropertiesHeader fExtra;
		CritereDISK fCRD;
		sLONG fStamp;
};


typedef Vx1ArrayOf<Field*, 20> FieldsArray;


														/* ------------------------ */


class CritereAlpha : public Critere
{
	public:
		CritereAlpha(CritereDISK* crd):Critere(crd) { CRD->typ = DB4D_StrFix; /*CRD.lenalpha = 30;*/ };
		
//		virtual VVariableKind GetLangKind(void);
//		virtual CClass* GetLangKindClass(void);

		virtual Boolean IsFullTextIndexable(void) { return(true); };

		virtual sLONG GetTypeSize() const
		{
			if (GetLimitingLen() == 0)
				return 0;
			else
				return 4 + 2*GetLimitingLen();
		}

		//virtual sLONG GetMaxLen();
		//virtual sLONG GetMaxChars() { return(CRD.lenalpha); };

	static Critere *NewCritere(CritereDISK* crd) {return new CritereAlpha(crd);}
};


class CritereAlphaUTF8 : public Critere
{
public:
	CritereAlphaUTF8(CritereDISK* crd):Critere(crd) { CRD->typ = VK_STRING_UTF8; /*CRD.lenalpha = 30;*/ };

//	virtual VVariableKind GetLangKind(void);
//	virtual CClass* GetLangKindClass(void);

	virtual Boolean IsFullTextIndexable(void) { return(true); };

	virtual sLONG GetTypeSize() const
	{
		if (GetLimitingLen() == 0)
			return 0;
		else
			return 4 + 2*GetLimitingLen();
	}

	//virtual sLONG GetMaxLen();
	//virtual sLONG GetMaxChars() { return(CRD.lenalpha); };

	virtual ValueKind GetTypeForLexicalIndex() const
	{
		return VK_STRING_UTF8;
	}

	static Critere *NewCritere(CritereDISK* crd) {return new CritereAlphaUTF8(crd);}
};

class CritereReel : public Critere
{
	public:
		CritereReel(CritereDISK* crd):Critere(crd) { CRD->typ = DB4D_Real; };
		
//		virtual VVariableKind GetLangKind(void);
//		virtual CClass* GetLangKindClass(void);
				
		//virtual sLONG GetMaxLen();
		virtual Boolean IsANum(void) { return(true); };
		//virtual sLONG GetMaxChars() { return(12); };
		virtual sLONG GetTypeSize() const { return sizeof(Real); };

	static Critere *NewCritere(CritereDISK* crd) {return new CritereReel(crd);}
};


class CritereLong8 : public Critere
{
	public:
		CritereLong8(CritereDISK* crd):Critere(crd) { CRD->typ = DB4D_Integer64; };
		
//		virtual VVariableKind GetLangKind(void);
//		virtual CClass* GetLangKindClass(void);
				
		//virtual sLONG GetMaxLen();
		virtual Boolean IsANum(void) { return(true); };
		//virtual sLONG GetMaxChars() { return(10); };

		virtual sLONG GetTypeSize() const { return sizeof(sLONG8); };

	static Critere *NewCritere(CritereDISK* crd) {return new CritereLong8(crd);}
};

/*
class CritereMoney : public Critere
{
	public:
		CritereMoney(void) { CRD->typ = DB4D_Money; };
		
		virtual VVariableKind GetLangKind(void);
		virtual CClass* GetLangKindClass(void);
				
		virtual sLONG GetMaxLen();
		virtual Boolean IsANum(void) { return(true); };
		virtual sLONG GetMaxChars() { return(12); };

	static Critere *NewCritere(CritereDISK* crd) {return new CritereMoney;}
};
*/


class CritereFloat : public Critere
{
	public:
		CritereFloat(CritereDISK* crd):Critere(crd) { CRD->typ = DB4D_Float; };
		
//		virtual VVariableKind GetLangKind(void);
//		virtual CClass* GetLangKindClass(void);
				
		//virtual sLONG GetMaxLen();
		virtual Boolean IsANum(void) { return(true); };
		//virtual sLONG GetMaxChars() { return(20); };

		virtual sLONG GetTypeSize() const { return 0; };
		virtual Boolean CanBePartOfRelation() const { return false; };

	static Critere *NewCritere(CritereDISK* crd) {return new CritereFloat(crd);}
};


class CritereShort : public Critere
{
	public:
		CritereShort(CritereDISK* crd):Critere(crd) { CRD->typ = DB4D_Integer16; };
		
//		virtual VVariableKind GetLangKind(void);
//		virtual CClass* GetLangKindClass(void);
				
		//virtual sLONG GetMaxLen();
		virtual Boolean IsANum(void) { return(true); };
		//virtual sLONG GetMaxChars() { return(6); };
		virtual sLONG GetTypeSize() const { return sizeof(sWORD); };

	static Critere *NewCritere(CritereDISK* crd) {return new CritereShort(crd);}
};

class CritereLong : public Critere
{
	public:
		CritereLong(CritereDISK* crd):Critere(crd) { CRD->typ = DB4D_Integer32; };
		
//		virtual VVariableKind GetLangKind(void);
//		virtual CClass* GetLangKindClass(void);
				
		//virtual sLONG GetMaxLen();
		virtual Boolean IsANum(void) { return(true); };
		//virtual sLONG GetMaxChars() { return(8); };
		virtual sLONG GetTypeSize() const { return sizeof(sLONG); };

	static Critere *NewCritere(CritereDISK* crd) {return new CritereLong(crd);}
};

class CritereBool : public Critere
{
	public:
		CritereBool(CritereDISK* crd):Critere(crd) { CRD->typ = DB4D_Boolean; };
		
//		virtual VVariableKind GetLangKind(void);
//		virtual CClass* GetLangKindClass(void);
				
		//virtual sLONG GetMaxLen();
		//virtual sLONG GetMaxChars() { return(8); };
		virtual sLONG GetTypeSize() const { return sizeof(char); };

	static Critere *NewCritere(CritereDISK* crd) {return new CritereBool(crd);}
};


class CritereByte : public Critere
{
public:
	CritereByte(CritereDISK* crd):Critere(crd) { CRD->typ = DB4D_Byte; };

//	virtual VVariableKind GetLangKind(void);
//	virtual CClass* GetLangKindClass(void);

	//virtual sLONG GetMaxLen();
	//virtual sLONG GetMaxChars() { return(8); };
	virtual sLONG GetTypeSize() const { return sizeof(char); };

	static Critere *NewCritere(CritereDISK* crd) {return new CritereByte(crd);}
};


class CritereTime : public Critere
{
	public:
		CritereTime(CritereDISK* crd):Critere(crd) { CRD->typ = DB4D_Time; };
		
//		virtual VVariableKind GetLangKind(void);
//		virtual CClass* GetLangKindClass(void);
				
		//virtual sLONG GetMaxLen();
		//virtual sLONG GetMaxChars() { return(10); };

		virtual sLONG GetTypeSize() const { return sizeof(uLONG8); };
	static Critere *NewCritere(CritereDISK* crd) {return new CritereTime(crd);}
};

class CritereDuration : public Critere
{
	public:
		CritereDuration(CritereDISK* crd):Critere(crd) { CRD->typ = DB4D_Duration; };
		
//		virtual VVariableKind GetLangKind(void);
//		virtual CClass* GetLangKindClass(void);
				
		//virtual sLONG GetMaxLen();
		//virtual sLONG GetMaxChars() { return(10); };
		virtual sLONG GetTypeSize() const { return sizeof(uLONG8); };

	static Critere *NewCritere(CritereDISK* crd) {return new CritereDuration(crd);}
};

class CritereBlob : public Critere
{
	public:
		CritereBlob(CritereDISK* crd):Critere(crd) { CRD->typ = DB4D_Blob; };
		
//		virtual VVariableKind GetLangKind(void);
//		virtual CClass* GetLangKindClass(void);
				
		//virtual sLONG GetMaxLen();
		virtual void CalcDependences(Table* fic, sLONG numfield);
		virtual Boolean IsIndexable(void) { return(false); };
		virtual Boolean IsFullTextIndexable(void) { return(false); };
		virtual Boolean IsTextable(void) { return(false); };
		virtual Boolean IsSortable(void) { return(false); };
		virtual Boolean IsSimple(void) { return(false); };
		virtual Boolean IsABlobHandle(void) { return(true); };
		//virtual sLONG GetMaxChars() { return(1); };
		virtual Boolean IsDataDirect(void) const { return false; };
		virtual sLONG GetTypeSize() const { return 0; };

		virtual Boolean CanBeUnique(void) const { return false; };
		virtual Boolean CanBeNot_Null(void) const { return false; };
		virtual Boolean CanBePartOfRelation() const { return false; };

		static Critere *NewCritere(CritereDISK* crd) {return new CritereBlob(crd);}
};


class CritereImage : public CritereBlob
{
	public:
//		virtual VVariableKind GetLangKind(void);
//		virtual CClass* GetLangKindClass(void);

		CritereImage(CritereDISK* crd):CritereBlob(crd) { CRD->typ = DB4D_Picture; };
		virtual Boolean IsPictable(void) { return(true); };
		virtual Boolean IsDataDirect(void) const { return false; };
		virtual sLONG GetTypeSize() const { return 0; };

		virtual Boolean IsFullTextIndexable(void) { return(true); };

	
		static Critere *NewCritere(CritereDISK* crd) {return new CritereImage(crd);}
};

class CritereText : public CritereBlob
{
	public:
		CritereText(CritereDISK* crd):CritereBlob(crd) { CRD->typ = DB4D_Text; };

//		virtual VVariableKind GetLangKind(void);
//		virtual CClass* GetLangKindClass(void);
				
		virtual Boolean IsIndexable(void) { return(false); };
		virtual Boolean IsFullTextIndexable(void) { return(true); };

		virtual Boolean IsTextable(void) { return(true); };
		virtual Boolean IsABlobHandle(void) { return(false); }; // les textes sont traites differement
		virtual sLONG GetMaxChars() { return(80); };
		virtual Boolean IsDataDirect(void) const { return false; };
		virtual sLONG GetTypeSize() const { return 0; };
	
		static Critere *NewCritere(CritereDISK* crd) {return new CritereText(crd);}
};


class CritereTextUTF8 : public CritereText
{
public:
	CritereTextUTF8(CritereDISK* crd):CritereText(crd) { CRD->typ = VK_TEXT_UTF8; };

	virtual ValueKind GetTypeForLexicalIndex() const
	{
		return VK_STRING_UTF8;
	}

	static Critere *NewCritere(CritereDISK* crd) {return new CritereTextUTF8(crd);}

};



class CritereUUID : public Critere
{
	public:
		CritereUUID(CritereDISK* crd):Critere(crd) { CRD->typ = DB4D_UUID; }

//		virtual VVariableKind GetLangKind(void);
//		virtual CClass* GetLangKindClass(void);
				
		//virtual sLONG GetMaxLen();
		//virtual sLONG GetMaxChars() { return(32); }
		virtual sLONG GetTypeSize() const { return 16; };
	
		static Critere *NewCritere(CritereDISK* crd) {return new CritereUUID(crd);}
};


class CritereSubTable : public Critere
{
public:
	CritereSubTable(CritereDISK* crd):Critere(crd) { SubTable = nil; SubTableNum = 0; CRD->typ = DB4D_SubTable; };

//	virtual VVariableKind GetLangKind(void) { return eVK_Long8;};
//	virtual CClass* GetLangKindClass(void) { return nil; };
				
	//virtual sLONG GetMaxLen() { return (0); };
	virtual Boolean IsTextable(void) { return(false); };
	virtual Boolean IsSortable(void) { return(false); };
	virtual Boolean IsSimple(void) { return(false); };
	//virtual sLONG GetMaxChars() { return(1); };
	virtual Boolean IsDataDirect(void) const { return false; };
	virtual sLONG GetTypeSize() const { return sizeof(uLONG8); };

	static Critere *NewCritere(CritereDISK* crd) {return new CritereSubTable(crd);}
	
protected:
	Table* SubTable;
	sLONG SubTableNum;
};



class CritereSubTableKey : public Critere
{
public:
	CritereSubTableKey(CritereDISK* crd):Critere(crd) { CRD->typ = DB4D_SubTableKey; };

//	virtual VVariableKind GetLangKind(void) { return eVK_Long8;};
//	virtual CClass* GetLangKindClass(void) { return nil; };

	//virtual sLONG GetMaxLen() { return (0); };
	virtual Boolean IsTextable(void) { return(false); };
	virtual Boolean IsSortable(void) { return(false); };
	virtual Boolean IsSimple(void) { return(false); };
	//virtual sLONG GetMaxChars() { return(1); };
	virtual Boolean IsDataDirect(void) const { return false; };
	virtual sLONG GetTypeSize() const { return sizeof(uLONG8); };

	static Critere *NewCritere(CritereDISK* crd) {return new CritereSubTableKey(crd);}

protected:
};



typedef Critere* (*CreCritere_Code)(CritereDISK* crd);


extern CodeReg *cri_CodeReg;

inline void RegisterCri(uLONG id, CreCritere_Code Code) { cri_CodeReg->Register(id,(void*)Code); };
inline CreCritere_Code FindCri(uLONG id) { return( (CreCritere_Code)(cri_CodeReg->GetCodeFromID(id)) ); };

void InitCritere();
void DeInitCritere();

typedef Field* FieldPtr;

typedef Vx1ArrayOf<FieldPtr, 20> FieldArray;

Boolean operator ==(const FieldArray& x1, const FieldArray& x2);


Critere* CreateCritere( const CritereDISK *criD, CritereDISK *crd);

Critere* CreateCritere(sLONG typ, CritereDISK *crd);




											/* ------------------------------------------------------------------ */

typedef enum { Sync_None, Sync_Update, Sync_Delete } Sync_Action;

enum { TrigAjout = 0, TrigModif, TrigSup, TrigLoad };
enum { PermOwner = 0, PermAjout, PermModif, PermSup, PermLoad };

class DataTable;

class FicheInMem;

class FieldNuplet;

class xEmptyVal
{
public:
	VValueSingle* fEmptyVal;
	void* fEmptyPtr;
};

typedef map<sLONG, xEmptyVal> MapOfEmptyVal;

class FicheOnDisk;

class Table : public Obj4D, public IObjCounter, public IDebugRefCountable, public IBaggable
{
	public:
		Table(Base4D *owner, sLONG xnres=0, Boolean canBeSaved = true, Boolean isRemote = false );
		virtual ~Table();
#if autocheckobj
		virtual uBOOL CheckObjInMem(void);
#endif
		inline Base4D* GetOwner(void) const { return Owner; };

#if debuglr == 111
		virtual	sLONG		Retain(const char* DebugInfo = 0) const;
		virtual	sLONG		Release(const char* DebugInfo = 0) const;
#endif
		
		inline sLONG GetNum() const { return(numfile); };
		inline void SetNum(sLONG n) { numfile=n; };
		inline void setDF(DataTable *xDF) 
		{ 
			DF=xDF; 
		};
		inline DataTable* GetDF() const { return(DF); };
		inline sLONG getresnum() const { return(nres); };

		VError InitDataFileOfFields();

		void ClearAllDependencies();

		void AddIndexDep(IndexInfo* ind);
		void DelIndexDep(IndexInfo* ind);
		void ClearIndexDep(void);

		inline const IndexArrayIncluded& GetIndexDep() const { return IndexDep; };
		inline IndexArrayIncluded& GetIndexDep() { return IndexDep; };
		void CopyIndexDep(IndexArray& outDeps) const;	// L.E. 27/10/05 ne peut etre inline car CW ne peut instantier le template CopyAndRetainFrom pour le header precompile.

		inline Boolean NonIndexed() const
		{
			return IndexDep.GetCount() == 0;
		}

		VError UpdateIndexKey(FicheInMem *fic, BaseTaskInfo* context);
		VError DeleteIndexKey(FicheInMem *fic, BaseTaskInfo* context);

		VError UpdateIndexKeyForTrans(sLONG n, FicheOnDisk *ficD, BaseTaskInfo* context);
		VError DeleteIndexKeyForTrans(sLONG n, BaseTaskInfo* context);

		IndexInfo* FindAndRetainIndexLexico(sLONG nc, Boolean MustBeValid = true, BaseTaskInfo* context = nil) const;
		IndexInfo* FindAndRetainIndexSimple(sLONG nc, uBOOL sortable=false, Boolean MustBeValid = true, BaseTaskInfo* context = nil)  const;
		IndexInfo* FindAndRetainIndex(FieldNuplet* fn, uBOOL sortable=false, Boolean MustBeValid = true, BaseTaskInfo* context = nil)  const;
		IndexInfo* FindAndRetainIndex(const NumFieldArray& fields, uBOOL sortable=false, Boolean MustBeValid = true, BaseTaskInfo* context = nil)  const;
		Boolean IsIndexed(const NumFieldArray& fields) const;
		void ReCalc(void);

		VError AddBlobDep(sLONG fieldnum);
		void DelBlobDep(sLONG fieldnum);
		void ClearBlobDep(void);
		void SetRecNumHintForBlobs(FicheInMem *fic, BaseTaskInfo* context, sLONG numrec);
		VError UpdateBlobs(FicheInMem *fic, BaseTaskInfo* context);
		VError DeleteBlobs(FicheInMem *fic, BaseTaskInfo* context);
		
		void CalcDependencesField(void);
		
		sLONG FindNextFieldFree();
		sLONG FindField( const VUUID& inFieldID) const;
		sLONG FindField( const VString& pFieldName) const;
		Field* FindAndRetainFieldRef(const VString& pFieldName) const;
		Field* FindAndRetainFieldRef( const VUUID& pRefID) const;
	
				
		inline Table* GetParent(void) { return fParent; };
		sLONG PosInParent(void);

		void RegisterForLang(Boolean inWithFields = false);
		void UnRegisterForLang(void);

//		static VError RecordPushMember (CLanguageContext* inContext, VClassInstance& ioClassInstance, CVariable* inVariable, StackPtr* ioStack);
//		static VError RecordPopMember  (CLanguageContext* inContext, VClassInstance& ioClassInstance, VVariableStamp* ptStamp, CVariable* inVariable, StackPtr* ioStack);

//		static VError TablePushMember (CLanguageContext* inContext, VClassInstance& ioClassInstance, CVariable* inVariable, StackPtr* ioStack);
//		static VError TablePopMember  (CLanguageContext* inContext, VClassInstance& ioClassInstance, VVariableStamp* ptStamp, CVariable* inVariable, StackPtr* ioStack);
		
//		CNameSpace* GetNameSpace(void);
//		CClass* GetTableRef(void) { return TableRef; };
//		CClass* GetRecordRef(void) { return RecordRef; };
//		CClass* GetSelectionRef(void) { return SelectionRef; };
		
		inline void SetNotifyState(Boolean inNotifyState) { fIsNotifying = inNotifyState; };
		Boolean IsNotifying();
		
		VError DropAllRelatedIndex(CDB4DBaseContext* inContext, VProgressIndicator* progress = nil);
		void RebuildAllRelatedIndex(CDB4DBaseContext* inContext, VProgressIndicator* progress = nil);

		Boolean CanRegister();
		VError ThrowError( VError inErrCode, ActionDB4D inAction) const;

		/*
		static VError	FakeRecCreatorMethod (CLanguageContext* inContext, CMethod* inMethod, StackPtr inStack);
		inline CMethod* GetFakeRecordCreator() const { return fFakeRecCreator; };
		*/

		void RecalcAtLeastOneUnique();
		void SetAtLeastOneUnique(Field* cri);
		inline Boolean AtLeastOneUniqueField() const { return fAtLeastOneUnique; };

		void RecalcAtLeastOneNot_Null();
		inline void SetAtLeastOneNot_Null(Field* cri);
		inline Boolean AtLeastOneNot_NullField() const { return fAtLeastOneNot_Null; };

		void RecalcAtLeastOneSubTable();
		inline Boolean AtLeastOneSubTable() const { return SubTableDep.GetCount() > 0; };

		void RecalcAtLeastOneAutoSeq();
		void SetAtLeastOneAutoSeq(Field* cri);
		inline Boolean AtLeastOneAutoSeqField() const { return AutoSeqDep.GetCount() > 0; };
	
		void RecalcAtLeastOneAutoGenerate();
		void SetAtLeastOneAutoGenerate(Field* cri);
		inline Boolean AtLeastOneAutoGenerateField() const { return AutoGenerateDep.GetCount() > 0; };
	
		inline bool AtLeastOneBlob() const
		{
			return BlobDep.GetCount() > 0;
		}

		inline void CopyBlobDep(NumFieldArray& into) const { occupe();into.CopyFrom(BlobDep); libere();};
		inline void CopySubTableDep(NumFieldArray& into) const { occupe();into.CopyFrom(SubTableDep); libere();};
		inline void CopyNot_NullDep(NumFieldArray& into) const { occupe();into.CopyFrom(Not_NullDep); libere();};
		inline void CopyUniqueDep(NumFieldArray& into) const { occupe();into.CopyFrom(UniqueDep); libere();};
		inline void CopyAutoSeqDep(NumFieldArray& into) const { occupe();into.CopyFrom(AutoSeqDep); libere();};
		inline void CopyAutoGenerateDep(NumFieldArray& into) const { occupe();into.CopyFrom(AutoGenerateDep); libere();};

		Boolean CheckUniqueness(FicheInMem* fic, const NumFieldArray& deps, BaseTaskInfo* context, sLONG beginprimkey);
		inline void CanNowBeSaved() { fTableCanBesaved = true; };
		inline Boolean CanBeSaved() const { return fTableCanBesaved; };

		virtual void CanNowKeepStamp(uBOOL inKeepStamp)
		{
			
		}

		void AddRelationNto1Dep(Relation* rel);
		void DelRelationNto1Dep(Relation* rel);
		void ClearRelationNto1Dep(void);

		void AddRelation1toNDep(Relation* rel);
		void DelRelation1toNDep(Relation* rel);
		void ClearRelation1toNDep(void);

		void AddRefIntegrityDep(Relation* rel);
		void DelRefIntegrityDep(Relation* rel);
		void ClearRefIntegrityDep(void);

		void AddRefIntegrityDepForeign(Relation* rel);
		void DelRefIntegrityDepForeign(Relation* rel);
		void ClearRefIntegrityDepForeign(void);

		inline Boolean AtLeastOneReferentialIntegrity() { return RefIntegrityDep.GetCount() > 0; };
		inline Boolean AtLeastOneReferentialIntegrityForeign() { return RefIntegrityDepForeign.GetCount() > 0; };
		VError CheckReferentialIntegrity(FicheInMem *fic, BaseTaskInfo* context, Boolean OnModify);
		VError CheckReferentialIntegrityForForeignKey(FicheInMem *fic, BaseTaskInfo* context, Boolean OnModify);

		VError GetListOfTablesForCascadingDelete(set<sLONG>& outSet);

		inline DepRelationArrayIncluded* GetRel1toNDeps() { return &RelDep1toN; };
		inline DepRelationArrayIncluded* GetRelNto1Deps() { return &RelDepNto1; };

		inline void CopyRel1toNDeps(DepRelationArrayIncluded& into)
		{
			occupe();
			into.CopyAndRetainFrom(RelDep1toN);
			libere();
		}

		inline void CopyRelNto1Deps(DepRelationArrayIncluded& into)
		{
			occupe();
			into.CopyAndRetainFrom(RelDepNto1);
			libere();
		}

		AutoSeqNumber* GetSeqNum(CDB4DBaseContext* inContext);

		VError SetPrimaryKeySilently(const NumFieldArray& PrimKey, VString* inPrimaryKeyName);
		VError SetPrimaryKey(const NumFieldArray& PrimKey, VProgressIndicator* InProgress, Boolean CanReplaceExistingOne, CDB4DBaseContext* inContext, VString* inPrimaryKeyName);
		inline void CopyPrimaryKey(NumFieldArray& into) const { occupe();into.CopyFrom(PrimaryKey); libere();};
		VError RetainPrimaryKey(FieldArray& outPrimKey) const;
		inline const VString& GetPrimaryKeyName() const { return fPrimaryKeyName; };

		bool HasPrimKey() const
		{
			return PrimaryKey.GetCount() > 0;
		}

		inline sLONG GetLimitingLen(sLONG fieldNum) const { if (tc[fieldNum] == nil) return 0; else return(tc[fieldNum]->GetLimitingLen()); };
		inline sLONG GetTextSwitchSize(sLONG fieldNum) const { if (tc[fieldNum] == nil) return 0; else return(tc[fieldNum]->GetTextSwitchSize()); };
		inline sLONG GetBlobSwitchSize(sLONG fieldNum) const { if (tc[fieldNum] == nil) return 0; else return(tc[fieldNum]->GetBlobSwitchSize()); };
		
		void DelayIndexes();
		void AwakeIndexes(VDB4DProgressIndicator* inProgress, vector<IndexInfo*> *outIndexList);
		Boolean AreIndexesDelayed() const;
		void AddOneIndexToDelay(IndexInfo* indexToAdd);

		Boolean IsDataDirect(sLONG numfield) const;
		Boolean IsNeverNull(sLONG numfield) const;

		virtual void GetUUID(VUUID& outID) const = 0;
		virtual void SetUUID(const VUUID& InID) = 0;
		virtual void GetName(VString& s) const = 0;
		virtual VError SetName(const VString& s, CDB4DBaseContext* inContext, Boolean inNotify = true, Boolean inWithNameCheck = true) = 0;
		virtual	VError SetNameSilently( const VString& inName) = 0;
		virtual sLONG GetNbCrit() const = 0;

		virtual VError SetFullyDeleteRecords(Boolean FullyDeleteState, CDB4DBaseContextPtr inContext = nil) = 0;
		virtual Boolean GetFullyDeleteRecordsState() const = 0;

		virtual sLONG GetTyp() const = 0;

		virtual void GetRecordName(VString& s) const = 0;
		virtual VError SetRecordName(const VString& s, CDB4DBaseContext* inContext, Boolean inNotify = true) = 0;

		//virtual Boolean GetInvisible(void) = 0;
		//virtual void SetInvisible(Boolean inv, CDB4DBaseContext* inContext) = 0;

		// from IBaggable
		virtual VError	LoadFromBagWithLoader( const VValueBag& inBag, VBagLoader *inLoader, void* inContext = nil) = 0;
		virtual VError	LoadPrimKeyFromBagWithLoader( const VValueBag& inBag, VBagLoader *inLoader, void* inContext = nil) = 0;
				// Returns the primary key from the bag without changing the table
		virtual VError	ExtractPrimKeyFromBagWithLoader( NumFieldArray& outPrimaryKey, const VValueBag& inBag, VBagLoader *inLoader, void *inContext = nil) const = 0;
		virtual VError	SaveToBag( VValueBag& ioBag, VString& outKind) const = 0;

		virtual VValueBag *CreateReferenceBag( bool inForExport) const = 0;

		virtual VError SetCrit(sLONG n, const VValueBag& inFieldDefinition, CDB4DBaseContext* inContext, VProgressIndicator* progress = nil) = 0;

		// L.E. pourquoi en virtual ? (pb de maskage)
		virtual sLONG AddField(const VString &name, sLONG fieldtyp, sLONG fieldlen, DB4DFieldAttributes inAttributes, VError& err, CDB4DBaseContext* inContext, VProgressIndicator* progress = nil) = 0;
		virtual sLONG AddField(Field* cri, VError& err, CDB4DBaseContext* inContext, VProgressIndicator* progress = nil) = 0;
		virtual sLONG AddFields(CritereDISK* cd, sLONG nbFieldsToAdd, VError& err, CDB4DBaseContext* inContext, VProgressIndicator* progress = nil) = 0;
		virtual VError AddFields( Field **inFirst, sLONG inCount, sLONG *outFirstAddedFieldNo, Boolean inWithNamesCheck, CDB4DBaseContext* inContext, VProgressIndicator* progress = nil) = 0;
		virtual VError AddFieldSilently( Field* cri, sLONG pos)
		{
			return ThrowError(VE_DB4D_TABLEISLOCKED, noaction);
		}

		virtual VError CreateFields( const VBagArray *inFieldsBags, sLONG *outFirstAddedFieldNo, VBagLoader *inLoader, CDB4DBaseContext* inContext, VProgressIndicator* progress = nil) = 0;
		virtual VError LoadFields( const VBagArray *inFieldsBags, VBagLoader *inLoader, CDB4DBaseContext* inContext) = 0;

		virtual Field* RetainField(sLONG nc) const;
		virtual VError save(uBOOL inDatas = false, bool cantouch = true) = 0;
		virtual VError change( const CritereDISK* p, sLONG nbcrit, sLONG start, Boolean inWithNamesCheck, CDB4DBaseContext* inContext, 
								Boolean inNotify = true, VProgressIndicator* progress = nil, Boolean isTrueStructField = true, bool cantouch = true) = 0;
		virtual VError load(Boolean inDatas = false, sLONG nresindata = 0, bool FromDataToStruct = false) = 0;

		virtual void ReleaseAllFields();

		virtual const VValueBag* RetainExtraProperties(VError &err, CDB4DBaseContext* inContext) = 0;
		virtual VError SetExtraProperties(VValueBag* inExtraProperties, bool inNotify, CDB4DBaseContext* inContext, bool loadonly = false) = 0;
		virtual void InitExtraPropHeaderLater(sLONG pos1, sLONG pos2) = 0;
		virtual void InitExtraPropHeaderLaterForFields() { ; };

		virtual Boolean DeleteField(Field* inFieldToDelete, CDB4DBaseContext* inContext, VProgressIndicator* progress = nil, Boolean inOnlyLocal = false) = 0;
		virtual VError Drop(CDB4DBaseContext* inContext, VProgressIndicator* progress = nil) = 0;

		virtual void Touch();
		inline sLONG GetStamp() const { return fStamp; };

		void GetFieldsStamps(StampsVector& outStamps) const;

		VError RetainIndexes(ArrayOf_CDB4DIndex& outIndexes);

		VError WriteSeqNumToLog(BaseTaskInfo* context, sLONG8 curnum);

		inline Boolean IsRemote() const { return fIsRemote; };

		static const void* xGetEmptyPtr(sLONG typ);
		static const VValueSingle* xGetEmptyVal(sLONG typ);

		virtual VError Update(VStream* dataget) = 0;
		virtual VError SendToClient(VStream* datasend) = 0;

		virtual VError RetainExistingFields(vector<CDB4DField*>& outFields);
		virtual sLONG CountExistingFields();

		virtual VError Truncate(BaseTaskInfo* context, VProgressIndicator* progress, Boolean ForADeleteSelection, bool& mustunlock) = 0;

		VError AddConstraint();
		VError AddConstraintCols();

		VError DelConstraint();
		VError DelConstraintCols();

		inline sLONG GetFieldType(sLONG numfield)
		{
			if (numfield<=0 || numfield>tc.GetCount())
				return nil;
			else
			{
				Field* ff = tc[numfield];
				if (ff == nil)
					return VK_EMPTY;
				else
					return ff->GetTyp();
			}
		}

		inline Boolean IsFieldStyledText(sLONG numfield)
		{
			if (numfield<=0 || numfield>tc.GetCount())
				return false;
			else
			{
				Field* ff = tc[numfield];
				if (ff == nil)
					return false;
				else
					return ff->GetStyledText();
			}
		}

		VError ActivateAutomaticRelations_N_To_1(FicheInMem* mainrec, vector<CachedRelatedRecord>& outResult, BaseTaskInfo* context, const vector<uBYTE>& inWayOfLocking);

		VError ActivateAutomaticRelations_1_To_N(FicheInMem* mainrec, vector<CachedRelatedRecord>& outResult, BaseTaskInfo* context, const vector<uBYTE>& inWayOfLocking,
													Field* onField, Boolean oldvalues, Boolean AutomaticOnly, Boolean OneLevelOnly);

		inline RecIDType GetRemoteMaxRecordsInTable() const
		{
			return fRemoteMaxRecords;
		}

		inline void SetRemoteMaxRecordsInTable(RecIDType nbmax)
		{
			fRemoteMaxRecords = nbmax;
		}

		RecIDType GetMaxRecords(BaseTaskInfo* context);

		void WaitToBuildIndex();

		void FinishedBuildingIndex();

		void WaitToAlterRecord()
		{
			//fBuildIndexMutex.Lock(RWS_ReadOnly); // peut sembler paradoxale qu'une modification ou destruction de fiche ne demande qu'un acces en lecture au semaphore
												 // mais celui ci represente l'acces a la creation d'index
		}

		void FinishAlterRecord()
		{
			//fBuildIndexMutex.Unlock();
		}


		virtual VError SetSchema( DB4D_SchemaID inSchemaID, CDB4DBaseContext* inContext)
		{
			return ThrowError(VE_DB4D_TABLEISLOCKED, noaction);
		}

		virtual DB4D_SchemaID GetSchema() const
		{
			return 0;
		}

		virtual bool GetKeepStamp() const
		{
			return false;
		}

		virtual VError SetKeepStamp( CDB4DBaseContext* inContext, bool inKeepStamp, VDB4DProgressIndicator* InProgress = nil)
		{
			return ThrowError( VE_DB4D_TABLEISLOCKED, DBaction_ChangingTableProperties);
		}

		bool GetFieldsNum(vector<sLONG>& outFieldsNum);
#if AllowSyncOnRecID
		VError GetOneRow(VStream& inStream, BaseTaskInfo* context, sBYTE& outAction, uLONG& outStamp, RecIDType& outRecID, vector<VValueSingle*>& outValues);
#endif
		VError GetOneRow(IReplicationInputFormatter& inFormatter, BaseTaskInfo* context, sBYTE& outAction, uLONG8& outStamp, VTime& outTimeStamp, vector<VValueSingle*>& outPrimKey, vector<VValueSingle*>& outValues);

		bool IsPrimKey(Field* cri)
		{
			if (PrimaryKey.GetCount() == 1 && cri->GetPosInRec() == PrimaryKey[1])
				return true;
			else
				return false;
		}

		virtual bool GetKeepRecordSyncInfo() const
		{
			return false;
		}

		virtual VError SetKeepRecordSyncInfo( CDB4DBaseContext* inContext, bool inKeepSyncInfo, VDB4DProgressIndicator* inProgress = nil)
		{
			return ThrowError( VE_DB4D_TABLEISLOCKED, DBaction_ChangingTableProperties);
		}

		virtual bool HasSyncInfo() const
		{
			return false;
		}

		virtual VError SetHideInRest( CDB4DBaseContext* inContext, bool x, VDB4DProgressIndicator* inProgress = nil)
		{
			return ThrowError( VE_DB4D_TABLEISLOCKED, DBaction_ChangingTableProperties);
		}

		virtual bool GetHideInRest() const
		{
			return false;
		}

		virtual void MarkRecordAsPushed(sLONG numrec)
		{
		}

		virtual void UnMarkRecordAsPushed(sLONG numrec)
		{
		}

		VError GetFragmentation(sLONG8& outTotalRec, sLONG8& outFrags, CDB4DBaseContext* inContext);

		virtual VError ImportRecords(VFolder* folder, BaseTaskInfo* context, VDB4DProgressIndicator* inProgress, ExportOption& options)
		{
			return ThrowBaseError(VE_DB4D_TABLEISLOCKED, noaction);
		}

		virtual Field* RetainPseudoField(DB4D_Pseudo_Field_Kind kind);

		sLONG8 GetSeqRatioCorrector() const;
	
#if debugLeaksAll
		virtual bool OKToTrackDebug() const
		{
			return false;
		}

		virtual void GetDebugInfo(VString& outText) const
		{
			VString s;
			GetName(s);
			outText = "Table : "+s;
		}

#endif


	protected:
		VString fPrimaryKeyName;
		FieldArray tc;
		sLONG nbuse;
		sLONG nres;
		sLONG numfile;
		DataTable *DF;

		DepRelationArrayIncluded RefIntegrityDepForeign;
		DepRelationArrayIncluded RefIntegrityDep;
		DepRelationArrayIncluded RelDepNto1;
		DepRelationArrayIncluded RelDep1toN;
		IndexArrayIncluded IndexDep;
		NumFieldArray BlobDep;
		NumFieldArray Not_NullDep;
		NumFieldArray UniqueDep;
		NumFieldArray SubTableDep;
		NumFieldArray AutoSeqDep;
		NumFieldArray AutoGenerateDep;
		NumFieldArray PrimaryKey;
		Table *fParent;
		Field* fPseudoRecNumField;
		
		Boolean fIsNotifying;
		Base4D *Owner;

		/*
		CVariable* TableVar;
		CClass* TableRef;
		CClass* RecordRef;
		CClass* SelectionRef;
		*/
		//CMethod* fFakeRecCreator;

		Boolean fRegistered;
		Boolean fTableCanBesaved;
		Boolean fAtLeastOneUnique;
		Boolean fAtLeastOneNot_Null;
		//Boolean fIsIndexDelayed;
		Boolean fIsRemote;

		sLONG fStamp;
		sLONG fAllIndexDelayRequestCount;
		mutable VCriticalSection fAllIndexDelayRequestCountMutex;
		RecIDType fRemoteMaxRecords;

		//ReadWriteSemaphore fBuildIndexMutex;

		static MapOfEmptyVal sEmptyVals;

};

typedef Table *TablePtr;

typedef V1ArrayOf<TablePtr> TableArray;


																			/* ============= */


class TableRegular : public Table
{
	public:
		TableRegular(Base4D *owner, sLONG xnres=0, Boolean canBeSaved = true, Boolean isRemote = false );
		virtual ~TableRegular();

		virtual void GetUUID(VUUID& outID) const;
		virtual void SetUUID(const VUUID& InID);
		virtual void GetName(VString& s) const;
		virtual VError SetName(const VString& s, CDB4DBaseContext* inContext, Boolean inNotify = true, Boolean inWithNameCheck = true);
		virtual	VError SetNameSilently( const VString& inName);		// to set the name of a newly created table
		virtual sLONG GetNbCrit() const { return(FID.nbcrit); };

		virtual VError SetFullyDeleteRecords(Boolean FullyDeleteState, CDB4DBaseContextPtr inContext = nil);
		virtual Boolean GetFullyDeleteRecordsState() const;

		virtual sLONG GetTyp() const { return(FID.typ); };

		virtual void Touch();

		virtual void GetRecordName(VString& s) const;
		virtual VError SetRecordName(const VString& s, CDB4DBaseContext* inContext, Boolean inNotify = true);

		//virtual Boolean GetInvisible(void) { return((Boolean)FID.fInvisible); };
		//virtual void SetInvisible(Boolean inv, CDB4DBaseContext* inContext) { FID.fInvisible = inv; };

		// from IBaggable
		virtual VError	LoadFromBagWithLoader( const VValueBag& inBag, VBagLoader *inLoader, void* inContext = nil);
		virtual VError	LoadPrimKeyFromBagWithLoader( const VValueBag& inBag, VBagLoader *inLoader, void* inContext = nil);
		virtual VError	ExtractPrimKeyFromBagWithLoader( NumFieldArray& outPrimaryKey, const VValueBag& inBag, VBagLoader *inLoader, void *inContext = nil) const;
		virtual VError	SaveToBag( VValueBag& ioBag, VString& outKind) const;

		virtual VValueBag *CreateReferenceBag( bool inForExport) const;

		virtual VError SetCrit(sLONG n, const VValueBag& inFieldDefinition, CDB4DBaseContext* inContext, VProgressIndicator* progress = nil);

		// L.E. pourquoi en virtual ? (pb de maskage)
		virtual sLONG AddField(const VString &name, sLONG fieldtyp, sLONG fieldlen, DB4DFieldAttributes inAttributes, VError& err, CDB4DBaseContext* inContext, VProgressIndicator* progress = nil);
		virtual sLONG AddField(Field* cri, VError& err, CDB4DBaseContext* inContext, VProgressIndicator* progress = nil);
		virtual sLONG AddFields(CritereDISK* cd, sLONG nbFieldsToAdd, VError& err, CDB4DBaseContext* inContext, VProgressIndicator* progress = nil);
		virtual VError AddFields( Field **inFirst, sLONG inCount, sLONG *outFirstAddedFieldNo, Boolean inWithNamesCheck, CDB4DBaseContext* inContext, VProgressIndicator* progress = nil);
		virtual VError AddFieldSilently( Field* cri, sLONG pos);


		virtual VError CreateFields( const VBagArray *inFieldsBags, sLONG *outFirstAddedFieldNo, VBagLoader *inLoader, CDB4DBaseContext* inContext, VProgressIndicator* progress = nil);
		virtual VError LoadFields( const VBagArray *inFieldsBags, VBagLoader *inLoader, CDB4DBaseContext* inContext);

		virtual VError save(uBOOL inDatas = false, bool cantouch = true);
		virtual VError change( const CritereDISK* p, sLONG nbcrit, sLONG start, Boolean inWithNamesCheck, CDB4DBaseContext* inContext, 
								Boolean inNotify = true, VProgressIndicator* progress = nil, Boolean isTrueStructField = true, bool cantouch = true);
		virtual VError load(Boolean inDatas = false, sLONG nresindata = 0, bool FromDataToStruct = false);

		virtual const VValueBag* RetainExtraProperties(VError &err, CDB4DBaseContext* inContext);
		virtual VError SetExtraProperties(VValueBag* inExtraProperties, bool inNotify, CDB4DBaseContext* inContext, bool loadonly = false);
		virtual void InitExtraPropHeaderLater(sLONG pos1, sLONG pos2) { fExtra.InitLater(pos1, pos2); };
		virtual void InitExtraPropHeaderLaterForFields();

		virtual Boolean DeleteField(Field* inFieldToDelete, CDB4DBaseContext* inContext, VProgressIndicator* progress = nil, Boolean inOnlyLocal = false);
		virtual VError Drop(CDB4DBaseContext* inContext, VProgressIndicator* progress = nil);

		virtual VError Update(VStream* dataget);
		virtual VError SendToClient(VStream* datasend);

		virtual VError Truncate(BaseTaskInfo* context, VProgressIndicator* progress, Boolean ForADeleteSelection, bool& mustunlock);

		virtual VError SetSchema(DB4D_SchemaID inSchemaID, CDB4DBaseContext* inContext);

		virtual DB4D_SchemaID GetSchema() const
		{

			if (FID.fSchemaID == 0)
				return 1;
			else
				return FID.fSchemaID;
		}

		virtual void CanNowKeepStamp(uBOOL inKeepStamp)
		{
			FID.fKeepStamps = inKeepStamp;
		}

		virtual bool GetKeepStamp() const
		{
			return FID.fKeepStamps == 1;
		}

		virtual VError SetKeepStamp( CDB4DBaseContext* inContext, bool inKeepStamp, VDB4DProgressIndicator* InProgress = nil);


		virtual bool GetKeepRecordSyncInfo() const;

		virtual VError SetKeepRecordSyncInfo( CDB4DBaseContext* inContext, bool inKeepSyncInfo, VDB4DProgressIndicator* inProgress = nil);

		virtual bool HasSyncInfo() const;

		virtual VError SetHideInRest( CDB4DBaseContext* inContext, bool x, VDB4DProgressIndicator* inProgress = nil);

		virtual bool GetHideInRest() const;


		virtual void MarkRecordAsPushed(sLONG numrec);

		virtual void UnMarkRecordAsPushed(sLONG numrec);

		virtual VError ImportRecords(VFolder* folder, BaseTaskInfo* context, VDB4DProgressIndicator* progress, ExportOption& options);

protected:
	FichierDISK FID;
	ExtraPropertiesHeader fExtra;
	VStr<32> fName;
	VStr<32> fRecordName;

};


																			/* ============= */


class TableSystem : public Table
{
	public:
		TableSystem(Base4D *owner, sLONG num, const VString& name, const VUUID& id, const VString& recordname);
		virtual ~TableSystem();

		virtual void GetUUID(VUUID& outID) const { outID = fID; };
		virtual void GetName(VString& s) const { s = fName; };
		virtual void GetRecordName(VString& s) const { s = fRecordName; };

		virtual void SetUUID(const VUUID& InID) { ; };
		virtual VError SetName(const VString& s, CDB4DBaseContext* inContext, Boolean inNotify = true, Boolean inWithNameCheck = true) { return ThrowError(VE_DB4D_TABLEISLOCKED, noaction); };
		virtual	VError SetNameSilently( const VString& inName) { return ThrowError(VE_DB4D_TABLEISLOCKED, noaction); }

		virtual sLONG GetNbCrit() const { return tc.GetCount(); };
		virtual sLONG GetTyp() const { return -1; };

		virtual VError SetRecordName(const VString& s, CDB4DBaseContext* inContext, Boolean inNotify = true) { return ThrowError(VE_DB4D_TABLEISLOCKED, noaction); };

		virtual VError SetFullyDeleteRecords(Boolean FullyDeleteState, CDB4DBaseContextPtr inContext = nil)
		{
			return ThrowError(VE_DB4D_TABLEISLOCKED, noaction);
		}

		virtual Boolean GetFullyDeleteRecordsState() const
		{
			return false;
		}

		//virtual Boolean GetInvisible(void) { return true; };
		//virtual void SetInvisible(Boolean inv, CDB4DBaseContext* inContext) {; };

		// from IBaggable
		virtual VError	LoadFromBagWithLoader( const VValueBag& inBag, VBagLoader *inLoader, void* inContext = nil) { return ThrowError(VE_DB4D_TABLEISLOCKED, noaction); };
		virtual VError	LoadPrimKeyFromBagWithLoader( const VValueBag& inBag, VBagLoader *inLoader, void* inContext = nil) { return ThrowError(VE_DB4D_TABLEISLOCKED, noaction); };
		virtual VError	ExtractPrimKeyFromBagWithLoader( NumFieldArray& outPrimaryKey, const VValueBag& inBag, VBagLoader *inLoader, void *inContext = nil) const { return ThrowError(VE_DB4D_TABLEISLOCKED, noaction); };
		virtual VError	SaveToBag( VValueBag& ioBag, VString& outKind) const;

		virtual VValueBag *CreateReferenceBag( bool inForExport) const;

		virtual VError SetCrit(sLONG n, const VValueBag& inFieldDefinition, CDB4DBaseContext* inContext, VProgressIndicator* progress = nil) { return ThrowError(VE_DB4D_TABLEISLOCKED, noaction); };

		// L.E. pourquoi en virtual ? (pb de maskage)
		virtual sLONG AddField(const VString &name, sLONG fieldtyp, sLONG fieldlen, DB4DFieldAttributes inAttributes, VError& err, CDB4DBaseContext* inContext, VProgressIndicator* progress = nil)
			 { err = ThrowError(VE_DB4D_TABLEISLOCKED, noaction); return -1;};

		virtual sLONG AddField(Field* cri, VError& err, CDB4DBaseContext* inContext, VProgressIndicator* progress = nil)
		{ err = ThrowError(VE_DB4D_TABLEISLOCKED, noaction); return -1;};

		virtual sLONG AddFields(CritereDISK* cd, sLONG nbFieldsToAdd, VError& err, CDB4DBaseContext* inContext, VProgressIndicator* progress = nil)
		{ err = ThrowError(VE_DB4D_TABLEISLOCKED, noaction); return -1;};

		virtual VError AddFields( Field **inFirst, sLONG inCount, sLONG *outFirstAddedFieldNo, Boolean inWithNamesCheck, CDB4DBaseContext* inContext, VProgressIndicator* progress = nil)
		{ return ThrowError(VE_DB4D_TABLEISLOCKED, noaction);};

		virtual VError CreateFields( const VBagArray *inFieldsBags, sLONG *outFirstAddedFieldNo, VBagLoader *inLoader, CDB4DBaseContext* inContext, VProgressIndicator* progress = nil)
		{ return ThrowError(VE_DB4D_TABLEISLOCKED, noaction);};

		virtual VError LoadFields( const VBagArray *inFieldsBags, VBagLoader *inLoader, CDB4DBaseContext* inContext)
		{
			return ThrowError(VE_DB4D_TABLEISLOCKED, noaction);
		}

		virtual VError save(uBOOL inDatas = false, bool cantouch = true)  
		{ return VE_OK; };

		virtual VError change( const CritereDISK* p, sLONG nbcrit, sLONG start, Boolean inWithNamesCheck, CDB4DBaseContext* inContext,
								Boolean inNotify = true, VProgressIndicator* progress = nil, Boolean isTrueStructField = true, bool cantouch = true)
		{ return ThrowError(VE_DB4D_TABLEISLOCKED, noaction);};

		virtual VError load(Boolean inDatas = false, sLONG nresindata = 0, bool FromDataToStruct = false) { return ThrowError(VE_DB4D_TABLEISLOCKED, noaction);};

		virtual const VValueBag* RetainExtraProperties(VError &err, CDB4DBaseContext* inContext)
		{ err = ThrowError(VE_DB4D_TABLEISLOCKED, noaction); return nil; };

		virtual VError SetExtraProperties(VValueBag* inExtraProperties, bool inNotify, CDB4DBaseContext* inContext, bool loadonly = false) { return ThrowError(VE_DB4D_TABLEISLOCKED, noaction); };
		virtual void InitExtraPropHeaderLater(sLONG pos1, sLONG pos2) { ; };

		virtual Boolean DeleteField(Field* inFieldToDelete, CDB4DBaseContext* inContext, VProgressIndicator* progress = nil, Boolean inOnlyLocal = false) { return false; };
		virtual VError Drop(CDB4DBaseContext* inContext, VProgressIndicator* progress = nil) { return ThrowError(VE_DB4D_TABLEISLOCKED, noaction); };

		void System_AddField(Field* cri);
		sLONG System_AddField(sLONG typ, const VString& name);

		virtual VError Update(VStream* dataget) { return ThrowError(VE_DB4D_TABLEISLOCKED, noaction); };
		virtual VError SendToClient(VStream* datasend) { return VE_DB4D_INTERNAL_ERROR_ILLEGAL_ON_REMOTE; };

		virtual VError Truncate(BaseTaskInfo* context, VProgressIndicator* progress, Boolean ForADeleteSelection, bool& mustunlock)
		{
			return ThrowError(VE_DB4D_TABLEISLOCKED, noaction);
		}

#if debugLeaksAll
		virtual bool OKToTrackDebug() const
		{
			return false;
		}

		virtual void GetDebugInfo(VString& outText) const
		{
			outText = "System Table : "+fName;
		}

#endif


	protected:
		VString fName;
		VString fRecordName;
		VUUID fID;
};

																			/* ============= */

class TableOfTable : public TableSystem
{
	public:
		TableOfTable(Base4D* owner);

		sLONG id_Table_Name;
		sLONG id_Temporary;
		sLONG id_Table_ID;
		sLONG id_Schema_ID;
		sLONG id_Replication;

	protected:

};



class TableOfField : public TableSystem
{
public:
	TableOfField(Base4D* owner);

	sLONG id_Table_Name;
	sLONG id_Column_Name;
	sLONG id_Data_Type;
	sLONG id_Data_Length;
	sLONG id_Nullable;
	sLONG id_Column_ID;
	sLONG id_Table_ID;
	sLONG id_Old_Data_Type;

protected:
};



class TableOfIndexes : public TableSystem
{
public:
	TableOfIndexes(Base4D* owner);

	sLONG id_Index_UUID;
	sLONG id_Index_Name;
	sLONG id_Index_Type;
	sLONG id_Table_Name;
	sLONG id_Uniq;
	sLONG id_Table_ID;
	sLONG id_Index_ID;

protected:
};



class TableOfIndexCols : public TableSystem
{
public:
	TableOfIndexCols(Base4D* owner);

	sLONG id_Index_UUID;
	sLONG id_Index_Name;
	sLONG id_Table_Name;
	sLONG id_Column_Name;
	sLONG id_Column_Position;
	sLONG id_Table_ID;
	sLONG id_Column_ID;
	sLONG id_Index_ID;

protected:
};


class TableOfConstraints : public TableSystem
{
public:
	TableOfConstraints(Base4D* owner);

	sLONG id_Constraint_ID;
	sLONG id_Constraint_Name;
	sLONG id_Constraint_Type;
	sLONG id_Table_Name;
	sLONG id_Table_ID;
	sLONG id_Delete_Rule;
	sLONG id_Related_Table;
	sLONG id_Related_Table_ID;

protected:
};


class TableOfConstraintCols : public TableSystem
{
public:
	TableOfConstraintCols(Base4D* owner);

	sLONG id_Constraint_ID;
	sLONG id_Constraint_Name;
	sLONG id_Table_Name;
	sLONG id_Table_ID;
	sLONG id_Column_Name;
	sLONG id_Column_ID;
	sLONG id_Position;
	sLONG id_Related_Column_Name;
	sLONG id_Related_Column_ID;

protected:
};


class TableOfSchemas : public TableSystem
{
public:
	TableOfSchemas(Base4D* owner);

	sLONG id_Schema_ID;
	sLONG id_Schema_Name;
	sLONG id_Read_Group_ID;
	sLONG id_Read_Group_Name;
	sLONG id_Read_Write_Group_ID;
	sLONG id_Read_Write_Group_Name;
	sLONG id_All_Group_ID;
	sLONG id_All_Group_Name;

protected:
};

// Definition of _USER_VIEWS.

class TableOfViews : public TableSystem
{
public:

			TableOfViews (Base4D *inOwner);

	sLONG	fViewName;				// VIEW_NAME		VARCHAR 
	sLONG	fSchemaID;				// SCHEMA_ID		INT32
	
	protected:
};

// Definition of _USER_VIEW_COLUMNS.

class TableOfViewFields : public TableSystem
{
public:

			TableOfViewFields (Base4D *owner);

	sLONG	fViewName;				// VIEW_NAME		VARCHAR
	sLONG	fColumnName;			// COLUMN_NAME		VARCHAR
	sLONG	fDataType;				// DATA_TYPE		INT32
	sLONG	fDataLength;			// DATA_LENGTH		INT32
	sLONG	fNullable;				// NULLABLE			BOOLEAN

protected:
};


/*
_USER_CONS_COLUMNS  - columns of all constraints of a database

CONSTRAINT_NAME        VString         Name associated with the constraint definition
TABLE_NAME             VString         Name associated with table with constraint definition
COLUMN_NAME            VString         Name associated with column specified in the constraint definition
POSITION               VWord           Original position of column in definition



_USER_CONSTRAINTS             - all integrity constraints in a database

CONSTRAINT_NAME        VString         Name associated with constraint definition
CONSTRAINT_TYPE        VString         Type of constraint definition – 
•	P (primary key)
•	R (referential integrity – foreign key)
•	4DR (4D relation)
TABLE_NAME             VString         Name associated with table with constraint definition
DELETE_RULE            VString         The delete rule for a referential constraint: "CASCADE","RESTRICT"

*/

																			/* ==================================== */


class BaseTaskInfo;

class Bittab;

class Blob4D;
class AddrDeBlobTable;
class TreeDeBlobInMem;

typedef Blob4D* (*CreBlob_Code)(DataTable *df);

class RechNode;

#if 0
class FullyDeletedRecordInfo : public ObjCacheInTree
{
	public:
		
};


class FullyDeletedRecordInfoTreeInMem : public TreeInMem
{
	public:
		inline LockEntityTreeInMem(typobjcache typ):TreeInMem(LockEntityTreeAcces , typ, /*false*/true ) {;};
	protected:
		virtual TreeInMem* CreTreeInMem();
};


class FullyDeletedRecordInfoTreeInMemHeader : public TreeInMemHeader
{
	protected:
		virtual TreeInMem* CreTreeInMem();
};
#endif

						//------------------------------------

#if 0

class BlobTreeInMem : public TreeInMem
{
	public:
		inline BlobTreeInMem(sWORD DefaultAccess, typobjcache typ /* = t_Obj4D*/):TreeInMem(DefaultAccess,typ, true) {;};
	protected:
		virtual TreeInMem* CreTreeInMem();
		virtual void DeleteElem( ObjCacheInTree *inObject);
};


class BlobTreeInMemHeader : public TreeInMemHeader
{
	protected:
		virtual TreeInMem* CreTreeInMem();
};


						//------------------------------------



class LockEntityTreeInMem : public TreeInMem
{
	public:
		inline LockEntityTreeInMem(typobjcache typ):TreeInMem(LockEntityTreeAcces , typ, /*false*/true ) {;};
	protected:
		virtual TreeInMem* CreTreeInMem();
};


class LockEntityTreeInMemHeader : public TreeInMemHeader
{
	public:
		inline LockEntityTreeInMemHeader()
		{
			//fNeedLock = false;
			fNeedLock = true;
			// pour l'instant je remet le lock en attendant de completement templater les treeinmem
		}

	protected:
		virtual TreeInMem* CreTreeInMem();
};

#endif

						//------------------------------------

class RecordTreeMemHeader : public TreeMemHeader
{
	public:
		virtual ~RecordTreeMemHeader()
		{
			OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
			PreDestructor(curstack);
		}

		virtual void PreDestructor(OccupableStack* curstack)
		{
			LibereEspaceMem(curstack);
		}

		virtual void KillElem(void* inObject);

		virtual bool IsElemPurged(void* InObject)
		{
			return false;
		}

		virtual bool TryToFreeElem(sLONG allocationBlockNumber, void* InObject, VSize& outFreed);

		virtual void RetainElem(void* inObject);

		virtual void OccupyElem(void* inObject, OccupableStack* curstack);

};



class BlobTreeMemHeader : public TreeMemHeader
{
public:
	virtual ~BlobTreeMemHeader()
	{
		OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
		PreDestructor(curstack);
	}

	virtual void PreDestructor(OccupableStack* curstack)
	{
		LibereEspaceMem(curstack);
	}

	virtual void KillElem(void* inObject);

	virtual bool IsElemPurged(void* InObject)
	{
		return false;
	}

	virtual bool TryToFreeElem(sLONG allocationBlockNumber, void* InObject, VSize& outFreed);

	virtual void RetainElem(void* inObject);

	virtual void OccupyElem(void* inObject, OccupableStack* curstack);

};



class LockEntityTreeMemHeader : public TreeMemHeader
{
public:
	virtual ~LockEntityTreeMemHeader()
	{
		OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
		PreDestructor(curstack);
	}

	virtual void PreDestructor(OccupableStack* curstack)
	{
		LibereEspaceMem(curstack);
	}

	virtual void KillElem(void* inObject)
	{
		; // do nothing
	}

	virtual bool IsElemPurged(void* InObject)
	{
		LockEntity* lle = (LockEntity*)InObject;
		if (lle->GetOwner() != nil)
			return false;
		else
			return true;
	}

	virtual bool TryToFreeElem(sLONG allocationBlockNumber, void* InObject, VSize& outFreed)
	{
		outFreed = 0;
		return IsElemPurged(InObject);
	}

	virtual void RetainElem(void* inObject)
	{
		;
	}

	virtual void OccupyElem(void* inObject, OccupableStack* curstack)
	{
		;
	}

};



class CoupleSelAndContext : public IChainable<CoupleSelAndContext>
{
	public:
		CoupleSelAndContext(Selection *xsel, BaseTaskInfo* xcontext) { fSel = xsel; fContext = xcontext; };
		Selection* GetSel() { return fSel; };
		BaseTaskInfo* GetContext() { return fContext; };
	protected:
		Selection* fSel;
		BaseTaskInfo* fContext;
};



																			/* ==================================== */




class CheckAndRepairAgent;
class ReadAheadBuffer;
class FicheOnDisk;
class ColumnFormulas;

class DataTable : public ObjInCacheMem, public IObjToFlush, public IObjToFree, public IReadWriteSemaphorable
{ 
	//friend class DataTools;

	public:
		DataTable();
		DataTable(Base4D *xdb, Table* xcrit, sLONG xnum);
		inline sLONG GetNum(void) const { /*return(num);*/ return crit == nil ? 0 : crit->GetNum(); };
		inline sLONG GetTrueNum() const { return fRealNum; };
		inline sLONG GetTableDefNumInData() const { return fTableDefNumInData; };
		inline void SetTableDefNumInData(sLONG xnum) { fTableDefNumInData = xnum; };
		inline Base4D* GetDB(void) const { return(db); };
		inline void SetDB(Base4D* xdb) { db=xdb; };
		inline Table* GetTable(void) const { /*VObjLock lock(this);*/return(crit); };
		Table* RetainTable() const;
		inline void ClearAssoc() { VTaskLock lock(&fAdminAccess); crit = nil; };

		// from IObjToFree
		virtual bool FreeMem(sLONG allocationBlockNumber, VSize combien, VSize& outSizeFreed)
		{
			return false;
		}

		// from  IObjToFlush
		virtual bool SaveObj(VSize& outSizeSaved)
		{
			return false;
		}

		/*
		inline void InterditLibereMemFiche(void) { pastouchefiche=true; };
		inline void PermetLibereMemFiche(void) { pastouchefiche=false; };
		inline uBOOL PasToucheFiche(void) { return(pastouchefiche); };
		*/

		VError ThrowError( VError inErrCode, ActionDB4D inAction) const;

		virtual VError SaveRecord(FicheInMem *fic, BaseTaskInfo* context, uLONG stamp = 0, bool allowOverrideStamp = false) = 0;
		virtual VError SaveRecordInTrans(FicheOnDisk *ficD, BaseTaskInfo* context) = 0;
		virtual FicheInMem* LoadRecord(sLONG n, VError& err, DB4D_Way_of_Locking HowToLock = DB4D_Do_Not_Lock, BaseTaskInfo* Context = nil, Boolean WithSubRecords = false, 
										Boolean BitLockOnly = false, ReadAheadBuffer* buffer = nil, Boolean* outEnoughMem = nil) = 0;
		virtual FicheInMem* NewRecord(VError& err, BaseTaskInfo* Context = nil) = 0;

		virtual FicheOnDisk* LoadNotFullRecord(sLONG n, VError& err, DB4D_Way_of_Locking HowToLock = DB4D_Do_Not_Lock, BaseTaskInfo* Context = nil, Boolean BitLockOnly = false, 
												ReadAheadBuffer* buffer = nil, Boolean* CouldLock = nil, Boolean* outEnoughMem = nil)
		{ err = VE_DB4D_NOTIMPLEMENTED; return nil; };
		virtual Boolean AcceptNotFullRecord() const { return false; }; 

		virtual sLONG GetNbRecords(BaseTaskInfo* context, bool lockread = true) = 0;
		virtual sLONG GetMaxRecords(BaseTaskInfo* context) const = 0;
		virtual sLONG GetMaxBlobs(BaseTaskInfo* context) const
		{
			return 0;
		};

		virtual VError DelRecord(FicheInMem *rec, BaseTaskInfo* context, uLONG stamp = 0xFFFFFFFF) = 0;
		virtual VError DelRecordForTrans(sLONG n, BaseTaskInfo* context) = 0;
		virtual VError QuickDelRecord(sLONG n, BaseTaskInfo* context) = 0;

		virtual VError TakeOffBits(Bittab *tb, BaseTaskInfo* context) = 0;

		virtual VError DelBlob(Blob4D *inBlob, BaseTaskInfo* context) = 0;
		virtual VError SaveBlob(Blob4D *inBlob, BaseTaskInfo* context, bool forcePathIfEmpty) = 0;
		virtual VError DelBlobForTrans(sLONG n, BaseTaskInfo* context, bool alterModifState) = 0;
		virtual VError SaveBlobForTrans(Blob4D *inBlob, BaseTaskInfo* context) = 0;
		virtual Blob4D* LoadBlob(sLONG n, CreBlob_Code Code, uBOOL formodif, VError& err, BaseTaskInfo* context) = 0;

		virtual uBOOL LockRecord(sLONG n, BaseTaskInfo* Context = nil, DB4D_Way_of_Locking HowToLock = DB4D_Keep_Lock_With_Record,
									DB4D_KindOfLock* outLockType = nil, const VValueBag **outLockingContextRetainedExtraData = nil) = 0;

		virtual void UnlockRecord(sLONG n, BaseTaskInfo* Context = nil, DB4D_Way_of_Locking HowToLock = DB4D_Keep_Lock_With_Record) = 0;
		//virtual void UnlockBitSel(Bittab *b, BaseTaskInfo* Context, DB4D_Way_of_Locking HowToLock = DB4D_Keep_Lock_With_Record) = 0;
		//virtual BaseTaskInfo* WhoLockedRecord(sLONG n, DB4D_KindOfLock& outLockType, BaseTaskInfo* Context, const VValueBag **outLockingContextRetainedExtraData) = 0;
		virtual void WhoLockedRecord(sLONG n, VError& err, BaseTaskInfo* Context, DB4D_KindOfLock *outLockType, const VValueBag **outLockingContextRetainedExtraData)
		{
			if (outLockType)
				*outLockType = DB4D_LockedByNone;
			if (outLockingContextRetainedExtraData )
				*outLockingContextRetainedExtraData = nil;
		}

		virtual uBOOL LockTable(BaseTaskInfo* Context, Boolean WaitForEndOfRecordLocks = false, sLONG TimeToWaitForEndOfRecordLocks = 0) = 0;
		virtual void UnLockTable(BaseTaskInfo* Context) = 0;

		//virtual sLONG GetAutoSequence(void) = 0;

		virtual void LibereEspaceMem(OccupableStack* curstack) = 0;
		virtual void IncNbLoadedRecords() = 0;
		virtual void DecNbLoadedRecords() = 0;

		//virtual Boolean AddLockSel(Bittab* b, Selection* sel, BaseTaskInfo* context) = 0;
		//virtual Boolean RemoveLockSel(Bittab* b, Selection* sel, BaseTaskInfo* context) = 0;

		virtual Boolean HasSomeRecordsInCache() const = 0;

		virtual void FillArrayFromCache(void* &into, sLONG sizeelem, Bittab* filtre, Bittab& outDeja, const xMultiFieldDataOffsets& criterias, OccupableStack* curstack, Bittab* nulls) = 0;

		virtual Boolean CheckForNonUniqueField(const NumFieldArray& fields, VProgressIndicator* progress, VError &err, CDB4DBaseContext* inContext) = 0;

		virtual void CalculateFormulaFromCache(ColumnFormulas* Formulas, Bittab* filtre, Bittab& outDeja, BaseTaskInfo* context, OccupableStack* curstack) = 0;

		virtual AutoSeqNumber* GetSeqNum(CDB4DBaseContext* inContext) = 0;

		virtual VError SetRecordEntryAsFree(sLONG numrec, BaseTaskInfo* context)
		{
			return ThrowError(VE_DB4D_TABLEISLOCKED, noaction); 
		}

		virtual sLONG ReserveNewRecAddr(BaseTaskInfo* context, VError& err) = 0;
		virtual VError LibereAddrRec(OccupableStack* curstack, sLONG numrec, BaseTaskInfo* context) = 0;
		virtual VError LibereAddrBlob(OccupableStack* curstack, sLONG numblob, BaseTaskInfo* context) = 0;
		virtual Bittab* GetNewRecsInTransID() = 0;
		virtual VError MarkAddrAsNewForTrans(sLONG numrec, BaseTaskInfo* context) = 0;
		virtual VError UnMarkAddrAsNewForTrans(sLONG numrec, BaseTaskInfo* context) = 0;

		virtual VError PerformRech(Bittab* ReelFiltre, RechNode* rn, Bittab *cursel, Bittab *filtre, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, 
									DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, sLONG& nbfound, Bittab &Nulls, Bittab* dejalocked, OccupableStack* curstack, LocalEntityModel* model) = 0;

		virtual VError PerformRech(Bittab* ReelFiltre, SimpleQueryNode* rn, Bittab *cursel, Bittab *filtre, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, 
									DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, sLONG& nbfound, Bittab &Nulls, Bittab* dejalocked, OccupableStack* curstack) = 0;



		virtual Bittab* Search(VError& err, RechNode* rn, Bittab *cursel, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, 
								DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, Bittab &Nulls, LocalEntityModel* model);
		virtual Bittab* Search(VError& err, SimpleQueryNode* rn, Bittab *cursel, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, 
								DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, Bittab &Nulls);

		virtual VError CollectionToData(Selection* sel, DB4DCollectionManager& collection, BaseTaskInfo* context, 
			Boolean AddToSel, Boolean CreateAlways, Selection* &outSel, Bittab* &outLockSet, VDB4DProgressIndicator* InProgress = nil);

		virtual VError DataToCollection(Selection* sel, DB4DCollectionManager& collection, sLONG FromRecInSel, sLONG ToRecInSel,
			BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, bool pictAsRaw, bool& FullyCompleted, sLONG& maxElems, bool TestLimit );

		virtual Selection* SortSel(SortTab& inSort, Selection* inSel, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, VError& err);
		virtual Selection* AllRecords(BaseTaskInfo* context, VError& err);
		virtual VError ExecuteQuery(SearchTab* query, QueryOptions* options, QueryResult& outResult, BaseTaskInfo* context, VDB4DProgressIndicator* inProgress);

		virtual Boolean NotDeleted(VError& outErr, ActionDB4D action = noaction) const;

		inline void LibereDelete() const 
		{ 
			VInterlocked::Decrement(&fNotDeletedRequest); 
		};

		inline uBOOL IsInvalid() const { return fIsInvalid; };

		virtual VError ResizeTableBlobs(OccupableStack* curstack, sLONG nbblobs) { return ThrowError(VE_DB4D_TABLEISLOCKED, noaction); };
		virtual VError NormalizeTableBlobs(OccupableStack* curstack) { return ThrowError(VE_DB4D_TABLEISLOCKED, noaction); };

		virtual VError ResizeTableRecs(OccupableStack* curstack, sLONG nbrecs) { return ThrowError(VE_DB4D_TABLEISLOCKED, noaction); };
		virtual VError NormalizeTableRecs(OccupableStack* curstack) { return ThrowError(VE_DB4D_TABLEISLOCKED, noaction); };

		virtual VError SaveRawRecord(OccupableStack* curstack, sLONG numrec, void* rawdata, sLONG lendata, sLONG nbfields, BaseTaskInfo* context)
		{
			return ThrowError(VE_DB4D_TABLEISLOCKED, noaction);
		}

		virtual VError SaveRawBlob(OccupableStack* curstack, DataBaseObjectType inType,sLONG numblob, void* rawdata, sLONG lendata, BaseTaskInfo* context)
		{
			return ThrowError(VE_DB4D_TABLEISLOCKED, noaction);
		}

		virtual Boolean isRecordDeleted(sLONG recordnum)
		{
			return false;
		}

		virtual DataAddr4D GetRecordPos(sLONG numrec, sLONG& outOldLen, VError& err)
		{
			outOldLen = 0;
			return 0;
			return ThrowError(VE_DB4D_TABLEISLOCKED, noaction);
		}

		virtual VError PutMaxRecordRemoteInfo(sLONG curstamp, VStream* outstream)
		{
			return VE_OK;
		}

		virtual VError FillCollection(DB4DCollectionManager& collection, Bittab& dejapris, SelPosIterator& PosIter, BaseTaskInfo* context, VArrayRetainedOwnedPtrOf<Field*>& cols, OccupableStack* curstack)
		{
			return VE_OK;
		}

		inline Boolean MustFullyDeleteRecords() const
		{
			return (crit != nil) && crit->GetFullyDeleteRecordsState();
		}

		virtual VError Truncate(BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, Boolean ForADeleteSelection, IndexArray& indexdeps)
		{
			return ThrowError(VE_DB4D_TABLEISLOCKED, noaction);
		}

		virtual VError EnoughDiskSpaceForRecord(BaseTaskInfo* context, Transaction* trans, VSize recSize)
		{
			return ThrowError(VE_DB4D_TABLEISLOCKED, noaction);
		}

		virtual bool GetKeepStamp() const
		{
			return false;
		}

		virtual VError SetKeepStamp(uBYTE inKeepStamp, VDB4DProgressIndicator* InProgress = nil)
		{
			return ThrowError(VE_DB4D_TABLEISLOCKED, noaction);
		}
#if AllowSyncOnRecID
		virtual VError GetModificationsSinceStamp(uLONG stamp, VStream& outStream, uLONG& outLastStamp, sLONG& outNbRows, BaseTaskInfo* context, vector<sLONG>& cols)
		{
			return ThrowError(VE_DB4D_TABLEISLOCKED, noaction);
		}

		virtual VError IntegrateModifications(VStream& inStream, BaseTaskInfo* context, vector<sLONG>& cols)
		{
			return ThrowError(VE_DB4D_TABLEISLOCKED, noaction);
		}
#endif
		/*
		virtual uLONG GetNewRecordStamp()
		{
			return 0;
		}
		*/

		virtual uLONG GetRecordStampInAddressTable(sLONG numrec)
		{
			return 0;
		}

		virtual uLONG8 GetNewRecordSync()
		{
			return 0;
		}

		virtual VError GetModificationsSinceStamp(uLONG8 stamp, VStream& outStream, uLONG8& outLastStamp, sLONG& outNbRows, BaseTaskInfo* context, vector<sLONG>& cols,
													Selection* filter, sLONG8 skip, sLONG8 top, std::vector<XBOX::VString*>* inImageFormats)
		{
			return ThrowError(VE_DB4D_TABLEISLOCKED, noaction);
		}

		virtual VError IntegrateModificationsWithPrimKey(VStream& inStream, BaseTaskInfo* context, vector<sLONG>& cols, bool sourceOverDest,
															uLONG8& ioFirstDestStampToCheck, uLONG8& outErrorStamp, bool inBinary = true)
		{
			return ThrowError(VE_DB4D_TABLEISLOCKED, noaction);
		}

		virtual VError ResetSeqNum()
		{
			return ThrowError(VE_DB4D_TABLEISLOCKED, noaction);			
		}

		virtual void MarkRecordAsPushed(sLONG numrec)
		{
		}

		virtual void UnMarkRecordAsPushed(sLONG numrec)
		{
		}

		virtual bool IsRecordMarkedAsPushed(sLONG numrec)
		{
			return false;
		}

		virtual void SetAssociatedTable(Table* tt, sLONG TableDefNumInData);

		bool CanBeResurected() const
		{
			return (crit == nil && fTableDefNumInData != 0);
		}

		virtual VError GetFragmentation(sLONG8& outTotalRec, sLONG8& outFrags)
		{
			outTotalRec = 0;
			outFrags = 0;
			return VE_OK;
		}

		virtual void MarkOutsideBlobToDelete(const VString& inID, const VString& inPath)
		{
		}

		virtual void UnMarkOutsideBlobToDelete(const VString& inPath)
		{
		}

		virtual void UnCacheOutsideBlob(const VString& inPath)
		{
		}

		virtual Blob4D* LoadBlobFromOutsideCache(const void* from, CreBlob_Code Code, VError& err, BaseTaskInfo* context)
		{
			return nil;
			err = ThrowBaseError(VE_DB4D_NOTIMPLEMENTED, noaction);
		}


		sLONG8 GetSeqRatioCorrector() const
		{
			return fSeqRatioCorrector;
		}

		void SetSeqRatioCorrector(sLONG newValue)
		{
			if (newValue < 1)
				newValue = 1;
			fSeqRatioCorrector = newValue;
		}

		virtual VError SetSeqID(const VUUIDBuffer& inID)
		{
			return ThrowError(VE_DB4D_TABLEISLOCKED, noaction);
		}

#if debugOverlaps_strong
		virtual Boolean Debug_CheckBlobAddrMatching(DataAddr4D addrToCheck, sLONG blobID)
		{
			return false;
		}

		virtual Boolean Debug_CheckAddrOverlap(DataAddr4D addrToCheck, sLONG lenToCheck, sLONG numblobToCheck = -1, sLONG numrecToCheck = -1) { return false; };
#endif

#if debuglr
		virtual void CheckRecordRef(FicheOnDisk* RecToCheck) { ; };
#endif

#if debug_Addr_Overlap
		virtual void FillDebug_DBObjRef()
		{
			;
		}
#endif

		void LockQuickDel()
		{
			fQuickDelAccess.Lock();
		}

		void UnLockQuickDel()
		{
			fQuickDelAccess.Unlock();
		}

	protected:
		virtual ~DataTable();

		Table *crit;
		Base4D *db;
		sLONG fRealNum;
		sLONG fTableDefNumInData;
		sLONG fSeqRatioCorrector;
		mutable sLONG fNotDeletedRequest;
		uBOOL fIsDeleted, fIsInvalid, fDefIsInDataPart;
		//uBOOL pastouchefiche;

		mutable VCriticalSection fAdminAccess;
		mutable VCriticalSection fQuickDelAccess;
};



																			/* ============= */

class BaseTaskInfo;

typedef map<const BaseTaskInfo*, sLONG, less<const BaseTaskInfo*>, cache_allocator<pair<const BaseTaskInfo*const, sLONG> > > mapofcount;

class LockerCount
{
public:

	Boolean IncCount(const BaseTaskInfo* context);
	Boolean DecCount(const BaseTaskInfo* context);
	sLONG GetCountFor(const BaseTaskInfo* context);
	sLONG RemoveContext(const BaseTaskInfo* context);

protected:
	mapofcount fCounts;
};


#if debugblobs
typedef map<sLONG, sLONG> debug_MapOfBlobNums;
#endif

/*
class ReadWriteMutex
{
public:
	inline ReadWriteMutex()
	{
		fWaitingEvent = nil;
	};

	VSyncEvent* fWaitingEvent;
	
};

typedef map<sLONG, ReadWriteMutex> ReadWriteMutexMap;


class ReadWriteMutexTable
{
	public:
		void Lock(sLONG n);
		void Unlock(sLONG n);

	protected:
		ReadWriteMutexMap fMap;
		VCriticalSection fMapMutex;

};
*/

typedef map<VString, Blob4D*> cacheOutsideBlob;

class DataTableRegular : public DataTable
{ 
	friend class DataTools;

	public:
		DataTableRegular();
		DataTableRegular(Base4D *xdb, Table* xcrit, sLONG xnum, DataAddr4D ou, Boolean ForInit, DataTableDISK* dejaDFD = nil);

#if debuglr == 114
		virtual	sLONG		Retain(const char* DebugInfo = 0) const;
		virtual	sLONG		Release(const char* DebugInfo = 0) const;
#endif

		// from IObjToFree
		virtual bool FreeMem(sLONG allocationBlockNumber, VSize combien, VSize& outSizeFreed);

		// from  IObjToFlush
		virtual bool SaveObj(VSize& outSizeSaved);

		// form ObjCache
		virtual VError LibereEspaceDisk(OccupableStack* curstack, VDB4DProgressIndicator* InProgress = nil);

		virtual VError SaveRecord(FicheInMem *fic, BaseTaskInfo* context, uLONG stamp = 0, bool allowOverrideStamp = false);
		virtual VError SaveRecordInTrans(FicheOnDisk *ficD, BaseTaskInfo* context);
		virtual FicheInMem* LoadRecord(sLONG n, VError& err, DB4D_Way_of_Locking HowToLock = DB4D_Do_Not_Lock, BaseTaskInfo* Context = nil, Boolean WithSubRecords = false, 
										Boolean BitLockOnly = false, ReadAheadBuffer* buffer = nil, Boolean* outEnoughMem = nil);
		virtual FicheOnDisk* LoadNotFullRecord(sLONG n, VError& err, DB4D_Way_of_Locking HowToLock = DB4D_Do_Not_Lock, BaseTaskInfo* Context = nil, Boolean BitLockOnly = false, 
												ReadAheadBuffer* buffer = nil, Boolean* CouldLock = nil, Boolean* outEnoughMem = nil);
		virtual FicheInMem* NewRecord(VError& err, BaseTaskInfo* Context = nil);
		virtual Boolean AcceptNotFullRecord() const { return true; }; 

		virtual sLONG GetNbRecords(BaseTaskInfo* context, bool lockread = true);
		virtual sLONG GetMaxRecords(BaseTaskInfo* context) const;
		virtual sLONG GetMaxBlobs(BaseTaskInfo* context) const
		{
			return DFD.nbBlob;
		}

		virtual VError DelRecord(FicheInMem *rec, BaseTaskInfo* context, uLONG stamp = 0xFFFFFFFF);
		virtual VError DelRecordForTrans(sLONG n, BaseTaskInfo* context);
		virtual VError QuickDelRecord(sLONG n, BaseTaskInfo* context);

		virtual VError TakeOffBits(Bittab *tb, BaseTaskInfo* context);

		virtual VError DelBlob(Blob4D *inBlob, BaseTaskInfo* context);
		virtual VError SaveBlob(Blob4D *inBlob, BaseTaskInfo* context, bool forcePathIfEmpty);
		virtual VError DelBlobForTrans(sLONG n, BaseTaskInfo* context, bool alterModifState);
		virtual VError SaveBlobForTrans(Blob4D *inBlob, BaseTaskInfo* context);
		virtual Blob4D* LoadBlob(sLONG n, CreBlob_Code Code, uBOOL formodif, VError& err, BaseTaskInfo* context);

		virtual uBOOL LockRecord(sLONG n, BaseTaskInfo* Context = nil, DB4D_Way_of_Locking HowToLock = DB4D_Keep_Lock_With_Record,
									DB4D_KindOfLock* outLockType = nil, const VValueBag **outLockingContextRetainedExtraData = nil);

		virtual void UnlockRecord(sLONG n, BaseTaskInfo* Context = nil, DB4D_Way_of_Locking HowToLock = DB4D_Keep_Lock_With_Record);
		//virtual void UnlockBitSel(Bittab *b, BaseTaskInfo* Context, DB4D_Way_of_Locking HowToLock = DB4D_Keep_Lock_With_Record);

		//virtual BaseTaskInfo* WhoLockedRecord(sLONG n, DB4D_KindOfLock& outLockType, BaseTaskInfo* Context, const VValueBag **outLockingContextRetainedExtraData);
		virtual void WhoLockedRecord(sLONG n, VError& err, BaseTaskInfo* Context, DB4D_KindOfLock *outLockType, const VValueBag **outLockingContextRetainedExtraData);

		virtual uBOOL LockTable(BaseTaskInfo* Context, Boolean WaitForEndOfRecordLocks = false, sLONG TimeToWaitForEndOfRecordLocks = 0);
		virtual void UnLockTable(BaseTaskInfo* Context);

		//virtual sLONG GetAutoSequence(void);

		virtual void LibereEspaceMem(OccupableStack* curstack);
		virtual void IncNbLoadedRecords();
		virtual void DecNbLoadedRecords();

		//virtual Boolean AddLockSel(Bittab* b, Selection* sel, BaseTaskInfo* context);
		//virtual Boolean RemoveLockSel(Bittab* b, Selection* sel, BaseTaskInfo* context);

		virtual Boolean HasSomeRecordsInCache() const { return ! FicInMem.IsEmpty(); };

		virtual void FillArrayFromCache(void* &into, sLONG sizeelem, Bittab* filtre, Bittab& outDeja, const xMultiFieldDataOffsets& criterias, OccupableStack* curstack, Bittab* nulls) 
		{ FicInMem.FillArrayFromCache(into, sizeelem, filtre, outDeja, criterias, curstack, nulls); };

		virtual Boolean CheckForNonUniqueField(const NumFieldArray& PrimKey, VProgressIndicator* progress, VError &err, CDB4DBaseContext* inContext);

		virtual void CalculateFormulaFromCache(ColumnFormulas* Formulas, Bittab* filtre, Bittab& outDeja, BaseTaskInfo* context, OccupableStack* curstack)
		{ FicInMem.CalculateFormulaFromCache(Formulas, filtre, outDeja, context, curstack); };

		virtual AutoSeqNumber* GetSeqNum(CDB4DBaseContext* inContext);

		virtual VError SetRecordEntryAsFree(sLONG numrec, BaseTaskInfo* context);

		virtual sLONG ReserveNewRecAddr(BaseTaskInfo* context, VError& err);
		virtual VError LibereAddrRec(OccupableStack* curstack, sLONG numrec, BaseTaskInfo* context);
		virtual VError LibereAddrBlob(OccupableStack* curstack, sLONG numblob, BaseTaskInfo* context);
		virtual Bittab* GetNewRecsInTransID() { return &fNewRecsInTransID; };
		virtual VError MarkAddrAsNewForTrans(sLONG numrec, BaseTaskInfo* context);
		virtual VError UnMarkAddrAsNewForTrans(sLONG numrec, BaseTaskInfo* context);

		virtual VError PerformRech(Bittab* ReelFiltre, RechNode* rn, Bittab *cursel, Bittab *filtre, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, 
									DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, sLONG& nbfound, Bittab &Nulls, Bittab* dejalocked, OccupableStack* curstack, LocalEntityModel* model)
		{ 
			return FicInMem.PerformRech(ReelFiltre, rn, cursel, filtre, InProgress, context, HowToLock, exceptions, limit, nbfound, Nulls, dejalocked, curstack, model); 
		};

		virtual VError PerformRech(Bittab* ReelFiltre, SimpleQueryNode* rn, Bittab *cursel, Bittab *filtre, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, 
									DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, sLONG& nbfound, Bittab &Nulls, Bittab* dejalocked, OccupableStack* curstack)
		{
			return FicInMem.PerformRech(ReelFiltre, rn, cursel, filtre, InProgress, context, HowToLock, exceptions, limit, nbfound, Nulls, dejalocked, curstack); 
		}

		VError CheckData(CheckAndRepairAgent* inCheck);

		VError DeleteAll(VDB4DProgressIndicator* InProgress = nil, VSyncEvent* event = nil, bool mustFullyDelete = false, sLONG numtable = -1);
		Boolean IsBusyDeleting();
		bool StartDeleting();
		void StopDeleting();

		void Debug_CheckBlobsAddr(BaseTaskInfo* context);

		void StartDataModif(BaseTaskInfo* context);
		void EndDataModif(BaseTaskInfo* context);

		void IncDataModif(BaseTaskInfo* context);
		void DecDataModif(BaseTaskInfo* context);
		sLONG GetDataModifCount(BaseTaskInfo* context);

		inline const VUUIDBuffer& GetUUID() const { return DFD.TableDefID; };

		virtual VError ResizeTableBlobs(OccupableStack* curstack, sLONG nbblobs);
		virtual VError NormalizeTableBlobs(OccupableStack* curstack);

		virtual VError ResizeTableRecs(OccupableStack* curstack, sLONG nbrecs);
		virtual VError NormalizeTableRecs(OccupableStack* curstack);

		virtual VError SaveRawRecord(OccupableStack* curstack, sLONG numrec, void* rawdata, sLONG lendata, sLONG nbfields, BaseTaskInfo* context);

		virtual VError SaveRawBlob(OccupableStack* curstack, DataBaseObjectType inType,sLONG numblob, void* rawdata, sLONG lendata, BaseTaskInfo* context);

		virtual Boolean isRecordDeleted(sLONG recordnum);

		virtual DataAddr4D GetRecordPos(sLONG numrec, sLONG& outOldLen, VError& err);

		VError CheckForNonEmptyTransHolesList();

		/*
		inline void LockForWriteRec(sLONG n) { fRecReadWriteMutexTable.Lock(n); };
		inline void UnlockForWriteRec(sLONG n) { fRecReadWriteMutexTable.Unlock(n); };

		inline void LockForWriteBlob(sLONG n) { fBlobReadWriteMutexTable.Lock(n); };
		inline void UnlockForWriteBlob(sLONG n) { fBlobReadWriteMutexTable.Unlock(n); };

		inline void LockForReadRec(sLONG n) { fRecReadWriteMutexTable.Lock(n); };
		inline void UnlockForReadRec(sLONG n) { fRecReadWriteMutexTable.Unlock(n); };

		inline void LockForReadBlob(sLONG n) { fBlobReadWriteMutexTable.Lock(n); };
		inline void UnlockForReadBlob(sLONG n) { fBlobReadWriteMutexTable.Unlock(n); };
		*/


		void ClearLockCount(BaseTaskInfo* context);

		virtual Boolean NotDeleted(VError& outErr, ActionDB4D action = noaction) const;

		virtual VError PutMaxRecordRemoteInfo(sLONG curstamp, VStream* outstream);

		inline void ResetMaxRecordRemoteInfo()
		{
			VInterlocked::Exchange(&fRemoteMaxRecordsStamp, 1);
		}

		virtual VError FillCollection(DB4DCollectionManager& collection, Bittab& dejapris, SelPosIterator& PosIter, BaseTaskInfo* context, VArrayRetainedOwnedPtrOf<Field*>& cols, OccupableStack* curstack)
		{
			return FicInMem.FillCollection(collection, dejapris, PosIter, context, cols, curstack);
		}

		virtual VError Truncate(BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, Boolean ForADeleteSelection, IndexArray& indexdeps);

		virtual VError EnoughDiskSpaceForRecord(BaseTaskInfo* context, Transaction* trans, VSize recSize);

		VError WriteDeleteToLog(BaseTaskInfo* context, sLONG numfic);

		virtual bool GetKeepStamp() const
		{
			return DFD.fKeepStamps;
		}

		virtual VError SetKeepStamp(uBYTE inKeepStamp, VDB4DProgressIndicator* InProgress = nil);

#if debuglr == 2
		void CheckBlobRef(Blob4D* blobToCheck) {  BlobInMem.CheckObjRef((ObjCacheInTree*)blobToCheck); };
		virtual void CheckRecordRef(FicheOnDisk* RecToCheck) {  FicInMem.CheckObjRef((ObjCacheInTree*)RecToCheck); };
#endif

#if debugblobs
		void debug_putblobnum(sLONG blobnum, sLONG recnum);
		void debug_delblobnum(sLONG blobnum, sLONG recnum);
#endif

#if debug_Addr_Overlap
		virtual void FillDebug_DBObjRef();
#endif

#if debugOverlaps_strong
		virtual Boolean Debug_CheckBlobAddrMatching(DataAddr4D addrToCheck, sLONG blobID);
		virtual Boolean Debug_CheckAddrOverlap(DataAddr4D addrToCheck, sLONG lenToCheck, sLONG numblobToCheck = -1, sLONG numrecToCheck = -1);
#endif

		/*
		virtual uLONG GetNewRecordStamp()
		{
			uLONG result;
			{
				VTaskLock lock(&fStampMutex);
				DFD.fLastRecordStamp++;
				if (DFD.fLastRecordStamp == 0)
					DFD.fLastRecordStamp = 1;
				result = DFD.fLastRecordStamp;
			}
			setmodif(true, db, nil);
			return result;
		}
		*/

		virtual uLONG GetRecordStampInAddressTable(sLONG numrec);

		virtual uLONG8 GetNewRecordSync()
		{
			uLONG8 result;
			{
				VTaskLock lock(&fStampMutex);
				DFD.fLastRecordSync++;
				result = DFD.fLastRecordSync;
			}
			setmodif(true, db, nil);
			return result;
		}

#if AllowSyncOnRecID
		virtual VError GetModificationsSinceStamp(uLONG stamp, VStream& outStream, uLONG& outLastStamp, sLONG& outNbRows, BaseTaskInfo* context, vector<sLONG>& cols);

		virtual VError IntegrateModifications(VStream& inStream, BaseTaskInfo* context, vector<sLONG>& cols);
#endif

				VError SendModificationValue ( VValueSingle* inValue, VStream& inOutputStream, bool inWithRawImages, std::vector<XBOX::VString*>* inImageFormats);
		virtual VError GetModificationsSinceStamp(uLONG8 stamp, VStream& outStream, uLONG8& outLastStamp, sLONG& outNbRows, BaseTaskInfo* context, vector<sLONG>& cols,
													Selection* filter, sLONG8 skip, sLONG8 top, std::vector<XBOX::VString*>* inImageFormats = 0);

		virtual VError IntegrateModificationsWithPrimKey(VStream& inStream, BaseTaskInfo* context, vector<sLONG>& cols, bool sourceOverDest,
															uLONG8& ioFirstDestStampToCheck, uLONG8& outErrorStamp, bool inBinary = true);

		virtual VError ResetSeqNum()
		{
			VUUID xid;
			xid.Clear();
			DFD.SeqNum_ID = xid.GetBuffer();
			return VE_OK;
		}

		virtual void MarkRecordAsPushed(sLONG numrec);

		virtual void UnMarkRecordAsPushed(sLONG numrec);

		virtual bool IsRecordMarkedAsPushed(sLONG numrec);

		virtual void SetAssociatedTable(Table* tt, sLONG TableDefNumInData);

		void GetAssociatedTableUUID(VUUID& outID)
		{
			outID.FromBuffer(DFD.TableDefID);
		}

		virtual VError GetFragmentation(sLONG8& outTotalRec, sLONG8& outFrags);

		virtual void MarkOutsideBlobToDelete(const VString& inID, const VString& inPath);
		virtual void UnMarkOutsideBlobToDelete(const VString& inPath);
		void CacheOutsideBlob(Blob4D* inBlob);
		virtual void UnCacheOutsideBlob(const VString& inPath);
		void FreeOutsideBlobCache(sLONG allocationBlockNumber, VSize combien, VSize& outSizeFreed);
		Blob4D* RetainOutsideBlobFromCache(const VString& inPath);

		virtual Blob4D* LoadBlobFromOutsideCache(const void* from, CreBlob_Code Code, VError& err, BaseTaskInfo* context);

		virtual VError SetSeqID(const VUUIDBuffer& inID);

	protected:
		virtual ~DataTableRegular();

		Blob4D* FindBlob(sLONG numblob, OccupableStack* curstack);

		Boolean LibereAllLock(OccupableStack* curstack, Boolean CheckAlsoLoadedRecords);

		void DecNbRecords(sLONG n);
		void IncNbRecords(sLONG n);

		//CoupleSelAndContext* FindLockingSel(Selection* sel, BaseTaskInfo* context);

		DataTableDISK DFD;
		AddressTableHeader FicTabAddr;
		RecordTreeMemHeader FicInMem;
		sLONG nbrecord;
		AddressTableHeader BlobTabAddr;
		BlobTreeMemHeader BlobInMem;
		LockEntityTreeMemHeader LockRec;
		LockEntity* fWholeTableIsLockedBy;
		VCriticalSection fLoadMutex;
		VCriticalSection fStampMutex;
		//FullyDeletedRecordInfoTreeInMemHeader fFullyDeletedRecords;
		sLONG fTotalLocks;
		sLONG fLockCount;
		sLONG fCountDataModif;
		LockerCount fDataModifCounts;
		LockerCount fAllLocks;
		sLONG NbLoadedRecords;
		sLONG fRemoteMaxRecordsStamp;
		Bittab fAllRecords;
		//Bittab fCacheLockSel;
		//VChainOf<CoupleSelAndContext> fLockingSels;
		AutoSeqNumber* fSeq;
		//mutable VMutex* fLockEvent;
		Bittab fNewRecsInTransID;
		Boolean fBusyDeleting;
		bool fMustFullyDeleteForLibereEspaceDisk;
		bool fAllRecordsInited;
		VCriticalSection fPushedRecordIDsMutex;
		map<sLONG, sLONG> fPushedRecordIDs;
		cacheOutsideBlob fCacheOutsideBlob;
		VCriticalSection fCacheOutsideBlobMutex;

		//ReadWriteMutexTable fRecReadWriteMutexTable;
		//ReadWriteMutexTable fBlobReadWriteMutexTable;


#if debugblobs
		debug_MapOfBlobNums fDebug_MapOfBlobs;
#endif

};



																			/* ============= */



class DataTableSystem : public DataTable
{ 
	public:

		DataTableSystem(Base4D *xdb, Table* xcrit);

		virtual VError SaveRecord(FicheInMem *fic, BaseTaskInfo* context, uLONG stamp = 0, bool allowOverrideStamp = false) { return ThrowError(VE_DB4D_TABLEISLOCKED, noaction); };
		virtual VError SaveRecordInTrans(FicheOnDisk *ficD, BaseTaskInfo* context) { return ThrowError(VE_DB4D_TABLEISLOCKED, noaction); };
		virtual FicheInMem* NewRecord(VError& err, BaseTaskInfo* Context = nil) { err = ThrowError(VE_DB4D_TABLEISLOCKED, noaction); return nil; };

		virtual VError DelRecord(FicheInMem *rec, BaseTaskInfo* context, uLONG stamp = 0xFFFFFFFF) { return ThrowError(VE_DB4D_TABLEISLOCKED, noaction); };
		virtual VError DelRecordForTrans(sLONG n, BaseTaskInfo* context) { return ThrowError(VE_DB4D_TABLEISLOCKED, noaction); };
		virtual VError QuickDelRecord(sLONG n, BaseTaskInfo* context) { return ThrowError(VE_DB4D_TABLEISLOCKED, noaction); };

		virtual VError TakeOffBits(Bittab *tb, BaseTaskInfo* context) { return VE_OK; };

		virtual VError DelBlob(Blob4D *inBlob, BaseTaskInfo* context) { return ThrowError(VE_DB4D_TABLEISLOCKED, noaction); };
		virtual VError SaveBlob(Blob4D *inBlob, BaseTaskInfo* context, bool forcePathIfEmpty) { return ThrowError(VE_DB4D_TABLEISLOCKED, noaction); };
		virtual VError DelBlobForTrans(sLONG n, BaseTaskInfo* context, bool alterModifState) { return ThrowError(VE_DB4D_TABLEISLOCKED, noaction); };
		virtual VError SaveBlobForTrans(Blob4D *inBlob, BaseTaskInfo* context) { return ThrowError(VE_DB4D_TABLEISLOCKED, noaction); };
		virtual Blob4D* LoadBlob(sLONG n, CreBlob_Code Code, uBOOL formodif, VError& err, BaseTaskInfo* context) { err = ThrowError(VE_DB4D_TABLEISLOCKED, noaction); return nil; };

		virtual uBOOL LockRecord(sLONG n, BaseTaskInfo* Context = nil, DB4D_Way_of_Locking HowToLock = DB4D_Keep_Lock_With_Record,
									DB4D_KindOfLock* outLockType = nil, const VValueBag **outLockingContextRetainedExtraData = nil)  { return false; };

		virtual void UnlockRecord(sLONG n, BaseTaskInfo* Context = nil, DB4D_Way_of_Locking HowToLock = DB4D_Keep_Lock_With_Record) { ; };
		//virtual void UnlockBitSel(Bittab *b, BaseTaskInfo* Context, DB4D_Way_of_Locking HowToLock = DB4D_Keep_Lock_With_Record) { ; };
		//virtual BaseTaskInfo* WhoLockedRecord(sLONG n, DB4D_KindOfLock& outLockType, BaseTaskInfo* Context, const VValueBag **outLockingContextRetainedExtraData) { return nil; };

		virtual uBOOL LockTable(BaseTaskInfo* Context, Boolean WaitForEndOfRecordLocks = false, sLONG TimeToWaitForEndOfRecordLocks = 0) { return false; };
		virtual void UnLockTable(BaseTaskInfo* Context) { ; };

		//virtual sLONG GetAutoSequence(void) { return 0; };

		virtual void LibereEspaceMem(OccupableStack* curstack) { ; };
		virtual void IncNbLoadedRecords() { ; };
		virtual void DecNbLoadedRecords() { ; };

		//virtual Boolean AddLockSel(Bittab* b, Selection* sel, BaseTaskInfo* context) { return false; };
		//virtual Boolean RemoveLockSel(Bittab* b, Selection* sel, BaseTaskInfo* context) { return false; };

		virtual Boolean HasSomeRecordsInCache() const { return false; };

		virtual void FillArrayFromCache(void* &into, sLONG sizeelem, Bittab* filtre, Bittab& outDeja, const xMultiFieldDataOffsets& criterias, OccupableStack* curstack, Bittab* nulls) { ; };

		virtual Boolean CheckForNonUniqueField(const NumFieldArray& PrimKey, VProgressIndicator* progress, VError &err, CDB4DBaseContext* inContext) { return false; };

		virtual void CalculateFormulaFromCache(ColumnFormulas* Formulas, Bittab* filtre, Bittab& outDeja, BaseTaskInfo* context, OccupableStack* curstack) { ; };

		virtual AutoSeqNumber* GetSeqNum(CDB4DBaseContext* inContext) { return nil; };

		virtual sLONG ReserveNewRecAddr(BaseTaskInfo* context, VError& err) { err = ThrowError(VE_DB4D_TABLEISLOCKED, noaction); return -1; };
		virtual VError LibereAddrRec(OccupableStack* curstack, sLONG numrec, BaseTaskInfo* context) { return ThrowError(VE_DB4D_TABLEISLOCKED, noaction); };
		virtual VError LibereAddrBlob(OccupableStack* curstack, sLONG numblob, BaseTaskInfo* context) { return ThrowError(VE_DB4D_TABLEISLOCKED, noaction); };
		virtual Bittab* GetNewRecsInTransID() { return nil; };
		virtual VError MarkAddrAsNewForTrans(sLONG numrec, BaseTaskInfo* context) { return ThrowError(VE_DB4D_TABLEISLOCKED, noaction); };
		virtual VError UnMarkAddrAsNewForTrans(sLONG numrec, BaseTaskInfo* context) { return ThrowError(VE_DB4D_TABLEISLOCKED, noaction); };

		virtual VError PerformRech(Bittab* ReelFiltre, RechNode* rn, Bittab *cursel, Bittab *filtre, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, 
			DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, sLONG& nbfound, Bittab &Nulls, Bittab* dejalocked, OccupableStack* curstack, LocalEntityModel* model) { return VE_OK; };

		virtual VError PerformRech(Bittab* ReelFiltre, SimpleQueryNode* rn, Bittab *cursel, Bittab *filtre, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, 
			DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, sLONG& nbfound, Bittab &Nulls, Bittab* dejalocked, OccupableStack* curstack) { return VE_OK; };
  
		virtual FicheOnDisk* LoadNotFullRecord(sLONG n, VError& err, DB4D_Way_of_Locking HowToLock = DB4D_Do_Not_Lock, BaseTaskInfo* Context = nil, Boolean BitLockOnly = false, 
			ReadAheadBuffer* buffer = nil, Boolean* CouldLock = nil, Boolean* outEnoughMem = nil);

	protected:
		virtual ~DataTableSystem();


};


																			/* ============= */



class DataTableOfTables : public DataTableSystem
{ 
	public:

		DataTableOfTables(Base4D *xdb, Table* xcrit);
		virtual ~DataTableOfTables();

		virtual FicheInMem* LoadRecord(sLONG n, VError& err, DB4D_Way_of_Locking HowToLock = DB4D_Do_Not_Lock, BaseTaskInfo* Context = nil, Boolean WithSubRecords = false, 
										Boolean BitLockOnly = false, ReadAheadBuffer* buffer = nil, Boolean* outEnoughMem = nil);
		virtual sLONG GetNbRecords(BaseTaskInfo* context, bool lockread = true);
		virtual sLONG GetMaxRecords(BaseTaskInfo* context) const;
		virtual VError TakeOffBits(Bittab *tb, BaseTaskInfo* context);

	protected:

};


																			/* ============= */

class FieldRec
{
	public:
		inline FieldRec(sLONG tablenum, sLONG fieldnum) 
		{
			fTableNum = tablenum;
			fFieldNum = fieldnum;
		};

		inline bool operator < (const FieldRec other) const
		{
			if (fTableNum == other.fTableNum)
				return fFieldNum < other.fFieldNum;
			else
				return fTableNum < other.fTableNum;
		}

		sLONG fTableNum;
		sLONG fFieldNum;
};

typedef vector<FieldRec> FieldRecArray;
typedef set<FieldRec> FieldRecSet;

class DataTableOfFields : public DataTableSystem
{ 
	public:

		DataTableOfFields(Base4D *xdb, Table* xcrit);
		virtual ~DataTableOfFields();

		virtual FicheInMem* LoadRecord(sLONG n, VError& err, DB4D_Way_of_Locking HowToLock = DB4D_Do_Not_Lock, BaseTaskInfo* Context = nil, Boolean WithSubRecords = false, 
										Boolean BitLockOnly = false, ReadAheadBuffer* buffer = nil, Boolean* outEnoughMem = nil);
		virtual sLONG GetNbRecords(BaseTaskInfo* context, bool lockread = true);
		virtual sLONG GetMaxRecords(BaseTaskInfo* context) const;
		virtual VError TakeOffBits(Bittab *tb, BaseTaskInfo* context);

		VError AddFieldRec(sLONG tablenum, sLONG fieldnum);
		VError DelFieldRec(sLONG tablenum, sLONG fieldnum);
		VError InitFieldRec();


	protected:
		FieldRecArray fFields;
		FieldRecSet fFieldsSorted;
		Boolean fFields_inited;
		sLONG fFirstTrou;
		VError fError;
};


																			/* ============= */

typedef map<sLONG, VUUIDBuffer> MapOfIndexRef;

class DataTableOfIndexes : public DataTableSystem
{ 
public:

	DataTableOfIndexes(Base4D *xdb, Table* xcrit);
	virtual ~DataTableOfIndexes();

	virtual FicheInMem* LoadRecord(sLONG n, VError& err, DB4D_Way_of_Locking HowToLock = DB4D_Do_Not_Lock, BaseTaskInfo* Context = nil, Boolean WithSubRecords = false, 
		Boolean BitLockOnly = false, ReadAheadBuffer* buffer = nil, Boolean* outEnoughMem = nil);
	virtual sLONG GetNbRecords(BaseTaskInfo* context, bool lockread = true);
	virtual sLONG GetMaxRecords(BaseTaskInfo* context) const;
	virtual VError TakeOffBits(Bittab *tb, BaseTaskInfo* context);

	VError AddIndexRef(IndexInfo* ind);
	VError DelIndexRef(IndexInfo* ind);
	VError InitIndexRefs();


protected:
	MapOfIndexRef fIndexRefs;
	Boolean fIndexRefs_inited;
	VError fError;
	sLONG fLastindexNum;
};

																			/* ============= */



class IndexColRec
{
	public:
		inline IndexColRec(const VUUIDBuffer& indexid, const VUUIDBuffer& fieldid, sLONG pos) 
		{
			fIndexId = indexid;
			fFieldId = fieldid;
			fPos = pos;
		};

		VUUIDBuffer fIndexId;
		VUUIDBuffer fFieldId;
		sLONG fPos;
};

typedef vector<IndexColRec> IndexColRecArray;


class DataTableOfIndexCols : public DataTableSystem
{ 
public:

	DataTableOfIndexCols(Base4D *xdb, Table* xcrit);
	virtual ~DataTableOfIndexCols();

	virtual FicheInMem* LoadRecord(sLONG n, VError& err, DB4D_Way_of_Locking HowToLock = DB4D_Do_Not_Lock, BaseTaskInfo* Context = nil, Boolean WithSubRecords = false, 
		Boolean BitLockOnly = false, ReadAheadBuffer* buffer = nil, Boolean* outEnoughMem = nil);
	virtual sLONG GetNbRecords(BaseTaskInfo* context, bool lockread = true);
	virtual sLONG GetMaxRecords(BaseTaskInfo* context) const;
	virtual VError TakeOffBits(Bittab *tb, BaseTaskInfo* context);

	VError AddIndexCol(IndexInfo* ind, const VUUID& fieldid, sLONG pos);
	VError DelIndexCol(IndexInfo* ind);
	VError InitIndexCols();


protected:
	Boolean fIndexCols_inited;
	VError fError;
	sLONG fFirstTrou;
	IndexColRecArray fIndexCols;
};


																			/* ============= */


class ConstraintRec
{
	public:
		inline ConstraintRec(const VUUIDBuffer& inID, Boolean isPrimKey)
		{
			fID = inID;
			fIsPrimKey = isPrimKey;
			next = 1;
		};

		VUUIDBuffer fID;
		sLONG next;
		Boolean fIsPrimKey;

};


typedef vector<ConstraintRec> ConstraintRecArray;


class DataTableOfConstraints : public DataTableSystem
{ 
public:

	DataTableOfConstraints(Base4D *xdb, Table* xcrit);
	virtual ~DataTableOfConstraints();

	virtual FicheInMem* LoadRecord(sLONG n, VError& err, DB4D_Way_of_Locking HowToLock = DB4D_Do_Not_Lock, BaseTaskInfo* Context = nil, Boolean WithSubRecords = false, 
		Boolean BitLockOnly = false, ReadAheadBuffer* buffer = nil, Boolean* outEnoughMem = nil);
	virtual sLONG GetNbRecords(BaseTaskInfo* context, bool lockread = true);
	virtual sLONG GetMaxRecords(BaseTaskInfo* context) const;
	virtual VError TakeOffBits(Bittab *tb, BaseTaskInfo* context);

	VError AddConstraint(Table* tab);
	VError DelConstraint(Table* tab);
	VError AddConstraint(Relation* rel);
	VError DelConstraint(Relation* rel);
	VError InitConstraints();

protected:
	VError _AddConstraint(const VUUIDBuffer& id, Boolean isprim);
	VError _DelConstraint(const VUUIDBuffer& id, Boolean isprim);

	Boolean fConstraints_inited;
	sLONG fFirstTrou;
	VError fError;
	ConstraintRecArray fConstraints;
};


																			/* ============= */


class ConstraintColRec
{
public:
	inline ConstraintColRec(const VUUIDBuffer& inID, Boolean isPrimKey, const VUUIDBuffer& fieldid, sLONG pos, const VUUIDBuffer& relatedid)
	{
		fID = inID;
		fIsPrimKey = isPrimKey;
		next = 1;
		fPos = pos;
		fFieldID = fieldid;
		fRelatedID = relatedid;
	};

	VUUIDBuffer fID;
	VUUIDBuffer fFieldID;
	VUUIDBuffer fRelatedID;
	sLONG next;
	sLONG fPos;
	Boolean fIsPrimKey;


};


typedef vector<ConstraintColRec> ConstraintColRecArray;


class DataTableOfConstraintCols : public DataTableSystem
{ 
public:

	DataTableOfConstraintCols(Base4D *xdb, Table* xcrit);
	virtual ~DataTableOfConstraintCols();

	virtual FicheInMem* LoadRecord(sLONG n, VError& err, DB4D_Way_of_Locking HowToLock = DB4D_Do_Not_Lock, BaseTaskInfo* Context = nil, Boolean WithSubRecords = false, 
		Boolean BitLockOnly = false, ReadAheadBuffer* buffer = nil, Boolean* outEnoughMem = nil);
	virtual sLONG GetNbRecords(BaseTaskInfo* context, bool lockread = true);
	virtual sLONG GetMaxRecords(BaseTaskInfo* context) const;
	virtual VError TakeOffBits(Bittab *tb, BaseTaskInfo* context);

	VError AddConstraintCol(Table* tab, Field* cri, sLONG pos);
	VError DelConstraintCol(Table* tab);
	VError AddConstraintCol(Relation* rel, Field* cri, sLONG pos, Field* relatedcri);
	VError DelConstraintCol(Relation* rel);
	VError InitConstraints();


protected:
	VError _AddConstraintCol(const VUUIDBuffer& id, Boolean isprim, const VUUIDBuffer& fieldid, sLONG pos, const VUUIDBuffer& relatedid);
	VError _DelConstraintCol(const VUUIDBuffer& id, Boolean isprim);

	Boolean fConstraints_inited;
	sLONG fFirstTrou;
	VError fError;
	ConstraintColRecArray fConstraints;
};

																			/* ============= */


class DataTableOfSchemas : public DataTableSystem
{ 
	public:

		DataTableOfSchemas(Base4D *xdb, Table* xcrit);
		virtual ~DataTableOfSchemas();

		virtual FicheInMem* LoadRecord(sLONG n, VError& err, DB4D_Way_of_Locking HowToLock = DB4D_Do_Not_Lock, BaseTaskInfo* Context = nil, Boolean WithSubRecords = false, 
										Boolean BitLockOnly = false, ReadAheadBuffer* buffer = nil, Boolean* outEnoughMem = nil);
		virtual sLONG GetNbRecords(BaseTaskInfo* context, bool lockread = true);
		virtual sLONG GetMaxRecords(BaseTaskInfo* context) const;
		virtual VError TakeOffBits(Bittab *tb, BaseTaskInfo* context);

	protected:

};


																			/* ============= */

class DataTableOfViews : public DataTableSystem
{ 
public:

						DataTableOfViews (Base4D *xdb, Table* xcrit);
	virtual				~DataTableOfViews ();

	virtual FicheInMem	*LoadRecord (sLONG n, 
							VError &err, 
							DB4D_Way_of_Locking HowToLock = DB4D_Do_Not_Lock, 
							BaseTaskInfo *Context = nil, 
							Boolean WithSubRecords = false, 
							Boolean BitLockOnly = false, 
							ReadAheadBuffer *buffer = nil, 
							Boolean *outEnoughMem = nil);
	virtual sLONG		GetNbRecords (BaseTaskInfo *context, bool lockread = true);
	virtual sLONG		GetMaxRecords (BaseTaskInfo *context) const;
};

class DataTableOfViewFields : public DataTableSystem
{ 
public:

						DataTableOfViewFields (Base4D *xdb, Table* xcrit);
	virtual				~DataTableOfViewFields ();

	virtual FicheInMem	*LoadRecord (sLONG n, 
							VError &err, 
							DB4D_Way_of_Locking HowToLock = DB4D_Do_Not_Lock, 
							BaseTaskInfo *Context = nil, 
							Boolean WithSubRecords = false, 
							Boolean BitLockOnly = false, 
							ReadAheadBuffer *buffer = nil, 
							Boolean *outEnoughMem = nil);
	virtual sLONG		GetNbRecords (BaseTaskInfo *context, bool lockread = true);
	virtual sLONG		GetMaxRecords (BaseTaskInfo *context) const;
};


																			/* ============= */
typedef DataTable* DataTablePtr;

typedef V1ArrayOf<DataTableRegular*> DataTableRegularArray;


																			/* ============= */


class Base4D_NotOpened;
class ToolLog;

#if debuglr
typedef map<sLONG,sLONG> mapofblobnums;
#endif


class DataFile_NotOpened : public ObjAlmostInCache, public IProblemReporterIntf
{
	public:
		DataFile_NotOpened(Base4D_NotOpened* xbd, DataAddr4D addr, sLONG numtable);
		virtual ~DataFile_NotOpened();

		VError Load(ToolLog* log);

		inline Boolean BlockIsValid() const { return fBlockIsValid; };
		inline Boolean BlockIsFullyValid() const { return fBlockIsFullyValid; };

		inline Boolean NbFicIsValid() const { return fNbFicIsValid; };
		inline Boolean TabRecAddrIsValid() const { return fTabRecAddrIsValid; };
		inline Boolean TabTrouRecIsValid() const { return fTabTrouRecIsValid; };

		inline Boolean NbBlobIsValid() const { return fNbBlobIsValid; };
		inline Boolean TabBlobAddrIsValid() const { return fTabBlobAddrIsValid; };
		inline Boolean TabTrouBlobIsValid() const { return fTabTrouBlobIsValid; };

		inline Boolean RecordsHaveBeenChecked() const { return fRecordsHaveBeenChecked; };
		inline Boolean RecordMapIsValid() const { return fRecordMapIsValid; };
		inline Bittab* RecordsDeleted() const { return fRecordsDeleted; };

		inline sLONG GetMaxRecords() const { return DFD.nbfic; };
		VError CountRecords(ToolLog* log, sLONG& result, Boolean& outInfoIsValid);

		inline sLONG GetNumTableDef() const { return fNumTableDef; }

		VError CheckAllRecords(ToolLog* log);
		VError CheckAllBlobs(ToolLog* log);

		VError CheckBlobs(DataAddr4D ou, sLONG nbblobmax, sLONG nbblobstocheck, sLONG pos1, sLONG pos2, ToolLog* log, sLONG mastermultiplicateur);
		VError CheckOneBlob(DataAddr4D ou, sLONG len, sLONG numblob, ToolLog* log);
		VError CheckBlobsTrous(sLONG numtrou, ToolLog* log);

		VError CheckRecords(DataAddr4D ou, sLONG nbficmax, sLONG nbfictocheck, sLONG pos1, sLONG pos2, ToolLog* log, sLONG mastermultiplicateur);
		VError CheckOneRecord(DataAddr4D ou, sLONG len, sLONG numrec, ToolLog* log);
		VError CheckRecordsTrous(sLONG numtrou, ToolLog* log);

		// from IProblemReporterIntf
		virtual VError ReportInvalidTabAddrAddr(ToolLog* log, sLONG selector);
		virtual VError ReportInvalidTabAddrTag(ToolLog* log, sLONG selector);
		virtual VError ReportInvalidTabAddrRead(ToolLog* log, sLONG selector);
		virtual VError ReportInvalidTabAddrChecksum(ToolLog* log, sLONG selector);

	protected:
		Base4D_NotOpened* bd;
		DataTableDISK DFD;
		DataAddr4D fAddr;
		sLONG fNumTable;
		sLONG fNumTableDef;
		Boolean fIsLoaded, fBlockIsValid, fBlockIsFullyValid, fSeqIDIsValid;
		Boolean fNbFicIsValid, fTabRecAddrIsValid, fTabTrouRecIsValid;
		Boolean fNbBlobIsValid, fTabBlobAddrIsValid, fTabTrouBlobIsValid;
		bool fWithStamps;

		Boolean fBlobsHaveBeenChecked, fBlobsTrouHaveBeenChecked, fBlobMapIsValid, fBlobsOrphanHaveBeenChecked;
		Bittab* fBlobsInMap;
		Bittab* fBlobsInRec;
		Bittab* fBlobsDeleted;
		Bittab* fBlobsOrphan;
		sLONG nbDeletedBlobInserted, nbBlobsInMap, nbBlobsInRec;

		Boolean fRecordsHaveBeenChecked, fRecordsTrouHaveBeenChecked, fRecordMapIsValid;
//		Bittab* fRecordsInMap;
		Bittab* fRecordsDeleted;
		sLONG nbDeletedRecordInserted;
		sLONG fNbRecords;

		Bittab* fTempBlobsDeleted;
		Bittab* fTempRecordsDeleted;

		TabAddrCache* fTempBlobAddrCache;
		TabAddrCache* fTempRecAddrCache;

#if debug_BlobsRefsWithTools
		mapofblobnums fMapBlobs;
#endif

};


class IReplicationOutputFormatter
{
	public:

		virtual void SetPrimaryKey ( FieldArray const & inPK ) = 0;
		virtual void SetFields ( Table* inTable, vector<sLONG> const & inFieldIDs ) = 0;

		virtual VError PutActionCount ( sLONG inCount ) = 0;
		virtual VError PutLatestStamp ( sLONG8 inStamp ) = 0;
		virtual VError PutAction ( sBYTE inAction ) = 0;
		virtual VError PutPrimaryKeyCount ( sBYTE inCount ) = 0;
		virtual VError PutVValue ( VValueSingle* inValue ) = 0;
		virtual VError PutStamp ( uLONG8 inStamp ) = 0;
		virtual VError PutTimeStamp ( VValueSingle& inStamp ) = 0;
		virtual VError PutFieldCount ( sLONG inCount ) = 0;
		virtual VError PutEmptyResponse ( ) = 0;
};

class ReplicationOutputBinaryFormatter : public IReplicationOutputFormatter
{
	public:
		ReplicationOutputBinaryFormatter ( VStream& inStream, bool inUseRawImages, vector<VString*> const * inImageFormats );
		~ReplicationOutputBinaryFormatter ( );

		virtual void SetPrimaryKey ( FieldArray const & inPK ) { ; }
		virtual void SetFields ( Table* inTable, vector<sLONG> const & inFieldIDs ) { ; }

		virtual VError PutActionCount ( sLONG inCount ) { return fStream. PutLong ( inCount ); }
		virtual VError PutLatestStamp ( sLONG8 inStamp ) { return VE_OK; }
		virtual VError PutAction ( sBYTE inAction ) { return fStream. PutByte ( inAction ); }
		virtual VError PutPrimaryKeyCount ( sBYTE inCount ) { return fStream. PutByte ( inCount ); }
		virtual VError PutVValue ( VValueSingle* inValue );
		virtual VError PutStamp ( uLONG8 inStamp ) { return fStream. PutLong8 ( inStamp ); }
		virtual VError PutTimeStamp ( VValueSingle& inStamp ) { return inStamp. WriteToStream ( &fStream ); }
		virtual VError PutFieldCount ( sLONG inCount ) { return fStream. PutLong ( inCount ); }
		virtual VError PutEmptyResponse ( );

	private:
		VStream&					fStream;
		bool						fUseRawImages;
		vector<VString*>			fImageFormats;
};


/* Does raw JSON output. Can be modified to use RestTools for JSON output. */
class ReplicationOutputJSONFormatter : public IReplicationOutputFormatter
{
	public:
		ReplicationOutputJSONFormatter ( VStream& inStream, bool inUseRawImages, vector<VString*> const * inImageFormats );
		~ReplicationOutputJSONFormatter ( );

		virtual void SetPrimaryKey ( FieldArray const & inPK );
		virtual void SetFields ( Table* inTable, vector<sLONG> const & inFieldIDs );

		virtual VError PutActionCount ( sLONG inCount );
		virtual VError PutLatestStamp ( sLONG8 inStamp );
		virtual VError PutAction ( sBYTE inAction );
		virtual VError PutPrimaryKeyCount ( sBYTE inCount );
		virtual VError PutVValue ( VValueSingle* inValue );
		virtual VError PutStamp ( uLONG8 inStamp );
		virtual VError PutTimeStamp ( VValueSingle& inStamp );
		virtual VError PutFieldCount ( sLONG inCount );
		virtual VError PutEmptyResponse ( );

	private:

		VError GetJSONValue ( VValueSingle* vval, VString& outValue );
		void EscapeJSONString ( VString& ioValue );

		VStream&					fStream;
		bool						fUseRawImages;
		vector<VString*>			fImageFormats;

		bool						fIsFirstAction;
		sLONG						fCurrentAction;
		vector<VString>				fPrimaryKey;
		VIndex						fPrimaryKeyIndex;
		vector<VString>				fFields;
		VIndex						fFieldIndex;
};

class IReplicationInputFormatter
{
	public:

		virtual void SetPrimaryKey ( FieldArray const & inPK ) = 0;
		virtual void SetFields ( Table* inTable, vector<sLONG> const & inFieldIDs ) = 0;

		virtual VError GetRowCount ( sLONG& outCount ) = 0;

		virtual VError GetAction ( sBYTE& outAction ) = 0;
		virtual VError GetPrimaryKeyCount ( sBYTE& outCount ) = 0;
		virtual VError GetPrimaryKeyType ( uWORD& outType ) = 0;
		virtual VError GetVValue ( VValueSingle& outValue ) = 0;
		virtual VError GetStamp ( uLONG8& outStamp ) = 0;
		virtual VError GetTimeStamp ( VTime& outTimeStamp ) = 0;
		virtual VError GetFieldCount ( sLONG& outCount ) = 0;
		virtual VError GetFieldType ( uWORD& outType ) = 0;
};

class ReplicationInputBinaryFormatter : public IReplicationInputFormatter
{
	public:
		ReplicationInputBinaryFormatter ( VStream& inStream );
		~ReplicationInputBinaryFormatter ( );

		virtual void SetPrimaryKey ( FieldArray const & inPK ) { ; }
		virtual void SetFields ( Table* inTable, vector<sLONG> const & inFieldIDs ) { ; }

		virtual VError GetRowCount ( sLONG& outCount ) { return fStream. GetLong ( outCount ); }

		virtual VError GetAction ( sBYTE& outAction ) { return fStream. GetByte ( outAction ); }
		virtual VError GetPrimaryKeyCount ( sBYTE& outCount ) { return fStream. GetByte ( outCount ); }
		virtual VError GetPrimaryKeyType ( uWORD& outType ) { return fStream. GetWord ( outType ); }
		virtual VError GetVValue ( VValueSingle& outValue ) { return outValue. ReadRawFromStream ( &fStream ); }
		virtual VError GetStamp ( uLONG8& outStamp ) { return fStream. GetLong8 ( outStamp ); }
		virtual VError GetTimeStamp ( VTime& outTimeStamp ) { return outTimeStamp. ReadFromStream ( &fStream ); }
		virtual VError GetFieldCount ( sLONG& outCount ) { return fStream. GetLong ( outCount ); }
		virtual VError GetFieldType ( uWORD& outType ) { return fStream. GetWord ( outType ); }

	private:
		VStream&					fStream;
};


class ReplicationInputJSONFormatter : public IReplicationInputFormatter
{
	public:
		ReplicationInputJSONFormatter ( VStream& inStream );
		~ReplicationInputJSONFormatter ( );

		virtual void SetPrimaryKey ( FieldArray const & inPK );
		virtual void SetFields ( Table* inTable, vector<sLONG> const & inFieldIDs );

		virtual VError GetRowCount ( sLONG& outCount );

		virtual VError GetAction ( sBYTE& outAction );
		virtual VError GetPrimaryKeyCount ( sBYTE& outCount );
		virtual VError GetPrimaryKeyType ( uWORD& outType );
		virtual VError GetVValue ( VValueSingle& outValue );
		virtual VError GetStamp ( uLONG8& outStamp );
		virtual VError GetTimeStamp ( VTime& outTimeStamp );
		virtual VError GetFieldCount ( sLONG& outCount );
		virtual VError GetFieldType ( uWORD& outType );

	private:

		VString						fJSON;
		VJSONImporter*				fjsonImporter;
		sLONG						fActionCount;
		sLONG						fCurrentAction;
		vector<ValueKind>			fPKTypes;
		sLONG						fCurrentPK;
		VString						fCurrentValue;
		vector<ValueKind>			fFieldTypes;
		sLONG						fCurrentField;
};


#endif
