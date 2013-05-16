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
#ifndef __RELATION__
#define __RELATION__


class FieldRef
{
	public:
	
	Boolean operator == (const FieldRef& other) const { return crit == other.crit; };
	
	Table *fic;
	Field *crit;
};


class FieldRefArray : public Vx0ArrayOf<FieldRef, 10>
{
};

/*
class FieldRefArray : public VArrayVIT
{
public:
	inline FieldRefArray(sLONG pNbInit = 0,sLONG pNbToAdd = 1):VArrayVIT(sizeof(FieldRef),pNbInit,pNbToAdd){;};
	inline FieldRef& operator[](sLONG pIndex) {CALLRANGECHECK(pIndex);return (((FieldRef *)(*fTabHandle))[pIndex]);};
};
*/


class FieldNuplet : public Obj4D, public IObjCounter
{
	public:
		FieldNuplet(sLONG maxfield, uBOOL rien);
		FieldNuplet(FieldNuplet* from);
		virtual ~FieldNuplet();
		inline sLONG GetLen(void) const { return((nbfield*sizeof(FieldDef))+4); };
		inline FieldDef* GetDataPtr(void) { return(fields); };
		inline sLONG GetNbField(void) const { return(nbfield); };
		inline void SetNbField(sLONG nb) { nbfield=nb; };
		void SetNthField(sLONG n, Field* cri);
		inline FieldRefArray* GetFieldRefs(void) { return(&fieldarr); };
		inline sLONG GetFieldNum(sLONG n) const { return(fields[n].numfield); };
		inline sLONG GetFileNum(sLONG n) const { return(fields[n].numfile); };
		void CopyFrom(FieldNuplet* from);
		VError UpdateCritFic(Base4D *bd);
		uBOOL Match(const FieldNuplet* other) const;
		uBOOL Match(const NumFieldArray& xother, sLONG numTable) const;
		void SwapBytes();
		VError PutInto( VStream &inStream);
		static FieldNuplet* CreateFrom( VStream &inStream, Base4D *inBase, VError &outError);
	#if autocheckobj
		virtual uBOOL CheckObjInMem(void);
	#endif


	
	private:
		sLONG nbfield;
		FieldDef *fields;
		FieldRefArray fieldarr;
};



		/* ************************************************************************************************ */



class Relation : public Obj4D, public IObjCounter, public IDebugRefCountable, public IBaggable
{
	//friend bool comparelessrel(const Relation* r1, const Relation* r2);

	public:
		inline	Relation( Base4D *inBase, sLONG inPosInList, Boolean inIsRemote = false) : fExtra(&fExtraAddr, &fExtraLen, inBase->GetStructure(), -1, -1, DBOH_ExtraRelationProperties, inIsRemote), fIsRemote( inIsRemote)
				{
					fIsValid = true; fRelVarNto1 = nil; fRelVar1toN = nil; bd = inBase; fExtraAddr = 0; fExtraLen = 0; fStamp = 1; fPosInBase = inPosInList;
					fAutoLoadNto1 = false; fAutoLoad1toN = false; fCanSave = true; fSource = NULL; fDestination = NULL; fIsForSubTable = false; fIsForeignKey = false;
				};

				Relation(const VString* name, const VString* nameopposite, Field* source, Field* dest, sLONG inPosInList);
		virtual ~Relation();

		VError ExtendRelationFields(const CDB4DFieldArray& inSourceFields, const CDB4DFieldArray& inDestinationFields);

		VError ActivateManyToOneS(FicheInMem* rec, Selection* &result, BaseTaskInfo* context, 
															Boolean OldOne = false, DB4D_Way_of_Locking HowToLock = DB4D_Do_Not_Lock);

		VError ActivateManyToOne(FicheInMem* rec, FicheInMem* &result, BaseTaskInfo* context, 
														 Boolean OldOne = false, Boolean NoCache = false, DB4D_Way_of_Locking HowToLock = DB4D_Do_Not_Lock);

		VError ActivateOneToMany(FicheInMem* rec, Selection* &result, BaseTaskInfo* context, 
														 Boolean OldOne = false, Boolean NoCache = false, DB4D_Way_of_Locking HowToLock = DB4D_Do_Not_Lock);

		//VError ActivateDestToSource(FicheInMem* rec, Selection* &result, BaseTaskInfo* context, 
		//												 Boolean OldOne = false, DB4D_Way_of_Locking HowToLock = DB4D_Do_Not_Lock);

		VError ThrowError( VError inErrCode, ActionDB4D inAction) const;
		inline void InvalidRelation(){ fIsValid = false; };
		inline void GetName(VString& outName) const { outName = fName; };
		inline void GetOppositeName(VString& outName) const { outName = fOppositeName; };

		inline const VString& GetName() const { return fName; };
		inline const VString& GetOppositeName() const { return fOppositeName; };

		VError SetName(const VString& inName, CDB4DBaseContext* inContext);
		VError SetOppositeName(const VString& inName, CDB4DBaseContext* inContext);
		inline void GetDef(Field* &outSource, Field* &outDest) const { outSource = fSource; outDest = fDestination; };
		//inline DB4D_RelationType GetType() const { return fType; };
		inline Base4D* GetOwner() const { return bd; };
		inline void GetUUID(VUUID& outID) const { outID = fID; };
		inline void SetUUID(const VUUID& inID) { fID = inID; };
		inline const VUUID& GetUUID() const {return fID;}
		inline const VUUIDBuffer& GetUUIDBuffer() const { return fID.GetBuffer(); };
		inline CVariable* GetRelVarNto1() const { return fRelVarNto1; };
		inline CVariable* GetRelVar1toN() const { return fRelVar1toN; };
		void RegisterForLang();
		void UnRegisterForLang();
		VError PutInto(VStream& buf, const Base4D* bd) const;
		VError GetFrom(VStream& buf, const Base4D* bd);
		VError oldGetFrom(VStream& buf, const Base4D* bd);
		Boolean IsAutoLoadNto1(BaseTaskInfo* context = nil) const;
		Boolean IsAutoLoad1toN(BaseTaskInfo* context = nil) const;
				VError SetAutoNto1Load(Boolean state, CDB4DBaseContext* inContext);
				VError SetAuto1toNLoad(Boolean state, CDB4DBaseContext* inContext);
		inline	DB4D_RelationState GetState() const { return fState; };
		inline	VError SetState(DB4D_RelationState state) { fState = state; return VE_OK;};
				VError SetReferentialIntegrity(Boolean ReferentialIntegrity, Boolean AutoDeleteRelatedRecords, CDB4DBaseContext* inContext);
		inline	Boolean WithReferentialIntegrity() const { return fWithReferentialIntegrity; };
		inline	Boolean AutoDeleteRelatedRecords() const { return fAutoDelete; };

				VError SetForeignKey(Boolean on, CDB4DBaseContext* inContext);
		inline	Boolean IsForeignKey() const { return fIsForeignKey; };

		// from IBaggable
		virtual VError	LoadFromBagWithLoader( const VValueBag& inBag, VBagLoader *inLoader, void* inContext = nil);
		virtual VError	SaveToBag( VValueBag& ioBag, VString& outKind) const;

		inline	Field* GetSource() const { return fSource; };
		inline	Field* GetDest() const { return fDestination; };

		inline FieldArray& GetSources() { return fSources; };
		inline const FieldArray& GetSources() const { return fSources; };

		inline FieldArray& GetDestinations() { return fDestinations; };
		inline const FieldArray& GetDestinations() const { return fDestinations; };

		void CalculDependence();
		void RemoveFromDependence();

		void RecalcDepIntegrity();

		inline Boolean IsForSubtable() const { return fIsForSubTable; };
		inline void SetForSubTable(Boolean x) { fIsForSubTable = x; };

		inline const VValueBag* RetainExtraProperties(VError &err, CDB4DBaseContext* inContext) { return fExtra.RetainExtraProperties(err); };
		VError SetExtraProperties(VValueBag* inExtraProperties, bool inNotify, CDB4DBaseContext* inContext, bool loadonly = false);

		Boolean Match(const FieldArray& sources, const FieldArray& dests);

		VError CopyFields(FicheInMem* fichesource, FicheInMem* fichedest, Boolean OldOne = false);

		 void Touch();

		inline	sLONG GetStamp() const { return fStamp; };

		inline	sLONG GetPosInList() const { return fPosInBase; };
		inline	void SetPosInList(sLONG pos) { fPosInBase = pos; };

				VError Save();
				VError Load(StructElemDef* e = nil);

				VError Update( VStream *inDataGet);
				VError SendToClient( VStream *inDataSend);

				VError	SendReqSetAttributes( const VValueBag& inBag, CDB4DBaseContext *inContext);
				VError	SendReqSetExtraProperties( const VValueBag& inBag, CDB4DBaseContext *inContext);

		static	void	ExecReqSetAttributes( IRequestReply *inRequest, CDB4DBaseContext *inContext);
		static	void	ExecReqSetExtraProperties( IRequestReply *inRequest, CDB4DBaseContext *inContext);

		inline	sLONG GetOldType() const { return fOldType; };

		inline	void NotOkToSave() { fCanSave = false; };
		inline	void OkToSave() { fCanSave = true; };

				void SwapSourceDest();

				VError AddConstraint();
				VError AddConstraintCols();

				VError DelConstraint();
				VError DelConstraintCols();

		inline	Boolean IsRemoteLike()
		{
			return fIsRemote || bd->IsRemote();
		}

		inline Table* GetSourceTable()
		{
			if (fSource == nil)
				return nil;
			else
				return fSource->GetOwner();
		}

		inline Table* GetDestTable()
		{
			if (fDestination == nil)
				return nil;
			else
				return fDestination->GetOwner();
		}

	private:

				VError	_LoadAttributesFromBag( const VValueBag &inBag);
				VError	_SaveAttributesToBag( VValueBag &ioBag) const;

		VUUID fID;
		//DB4D_RelationType fType;
		DB4D_RelationState fState;
		Field *fSource;
		Field *fDestination;
		VStr<64> fName;
		VStr<64> fOppositeName;
		CVariable* fRelVarNto1;
		CVariable* fRelVar1toN;
		Base4D *bd;
		ExtraPropertiesHeader fExtra;
		DataAddr4D fExtraAddr;
		sLONG fExtraLen;
		Boolean fAutoLoadNto1;
		Boolean fAutoLoad1toN;
		Boolean fWithReferentialIntegrity;
		Boolean fAutoDelete;
		Boolean fIsValid;
		Boolean fIsForSubTable;
		Boolean fIsForeignKey;
		Boolean fCanSave;
		FieldArray fSources;
		FieldArray fDestinations;
		sLONG fStamp;
		sLONG fPosInBase;
		sLONG fOldType;
		Boolean fIsRemote;

};


/* ************************************************************************************************ */

class RelationInList
{
	public:
		inline RelationInList(Relation* rel, Boolean isNto1) 
		{ 
			assert(rel!=nil); 
			rel->Retain(); 
			fRel = rel; 
			fIsNto1 = isNto1;
		};

		~RelationInList() 
		{ 
			fRel->Release(); 
		};

		inline RelationInList(const RelationInList& other)
		{
			fRel = other.fRel;
			if (fRel != nil)
				fRel->Retain();
			fIsNto1 = other.fIsNto1;
		}

		RelationInList& operator = ( const RelationInList& inOther)
		{
			XBOX::CopyRefCountable( &fRel, inOther.fRel);
			fIsNto1 = inOther.fIsNto1;
			return *this;
		}

		inline Relation* GetRel() const { return fRel; };
		inline Boolean IsNto1() const { return fIsNto1; };

	protected:
		Relation* fRel;
		Boolean fIsNto1;
};

typedef list<RelationInList /*, cache_allocator<RelationInList>*/ > ListOfRelation;

class JoinPath;

class RelationPath : public VObject
{
	public:
		virtual ~RelationPath();
		Boolean BuildPath(BaseTaskInfo* context, sLONG source, sLONG dest, VError& err, Boolean Only_N_To_1, Boolean Only_Auto, const vector<uBYTE>& inWayOfLocking);
		Boolean BuildPath(BaseTaskInfo* context, Table* source, Table* dest, VError& err, Boolean Only_N_To_1, Boolean Only_Auto, const vector<uBYTE>& inWayOfLocking);
		VError ActivateRelation(FicheInMem* source, FicheInMem* &dest, BaseTaskInfo* context, Boolean readonly);
		VError CopyInto(JoinPath& outPath);

		Boolean IsEmpty() const
		{
			return fPath.empty();
		}

		inline bool OKTable(const vector<uBYTE>& inWayOfLocking, sLONG tablenum) const
		{
			if (tablenum <= inWayOfLocking.size() && inWayOfLocking[tablenum-1] == 255)
				return false;
			else
				return true;
		}

		inline bool OKTable(const vector<uBYTE>& inWayOfLocking, Table* inTable) const
		{
			return OKTable(inWayOfLocking, inTable->GetNum());
		}

		inline bool ContainsSubTables() const
		{
			return fContainsSubTables;
		}

	protected:
		Boolean BuildSubPath(BaseTaskInfo* context, Bittab* deja, Table* source_intermediaire, Table* finaldest, VError& err, Boolean Only_N_To_1, Boolean Only_Auto, sLONG curlevel, const vector<uBYTE>& inWayOfLocking);
		Boolean BuildSubPathNto1(BaseTaskInfo* context, Bittab* deja, Relation* Rel, Table* finaldest, VError& err, Boolean Only_N_To_1, Boolean Only_Auto, sLONG curlevel, const vector<uBYTE>& inWayOfLocking);
		Boolean BuildSubPath1toN(BaseTaskInfo* context, Bittab* deja, Relation* Rel, Table* finaldest, VError& err, Boolean Only_N_To_1, Boolean Only_Auto, sLONG curlevel, const vector<uBYTE>& inWayOfLocking);

		ListOfRelation fPath;
		bool fContainsSubTables;
};



#endif