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
#ifndef __ENTITYMODELLOCAL__
#define __ENTITYMODELLOCAL__




class LocalEntityModelCatalog;
class LocalEntityModel;

class LocalEntityCollection : public EntityCollection
{
	public:
		LocalEntityCollection(LocalEntityModel* inOwner, Selection* inSel);

		virtual ~LocalEntityCollection()
		{
			QuickReleaseRefCountable(fSel);
		}

		virtual VError AddCollection(EntityCollection* other, BaseTaskInfo* context, bool atTheEnd = false);
		virtual VError AddEntity(EntityRecord* entity, bool atTheEnd = false);

		virtual EntityRecord* LoadEntity(RecIDType posInCol, BaseTaskInfo* context, VError& outError, DB4D_Way_of_Locking HowToLock = DB4D_Do_Not_Lock);
		virtual RecIDType GetLength(BaseTaskInfo* context);

		virtual EntityCollection* SortSel(VError& err, EntityAttributeSortedSelection* sortingAtt, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress = nil, WafSelection* inWafSel = nil, WafSelection* outWafSel = nil);

		virtual VError DropEntities(BaseTaskInfo* context, VDB4DProgressIndicator* InProgress = nil, EntityCollection* *outLocked = nil);

		inline Selection* GetSel() const
		{
			return fSel;
		}

		virtual bool IsSorted() const
		{
			return (fSel->GetTypSel() != sel_bitsel);
		}

		virtual bool IsSafeRef() const
		{
			return false;
		}

		virtual RecIDType NextNotNull(BaseTaskInfo* context, RecIDType startFrom);

		virtual EntityCollection* And(EntityCollection* other, VError& err, BaseTaskInfo* context);
		virtual EntityCollection* Or(EntityCollection* other, VError& err, BaseTaskInfo* context);
		virtual EntityCollection* Minus(EntityCollection* other, VError& err, BaseTaskInfo* context);
		virtual bool Intersect(EntityCollection* other, VError& err, BaseTaskInfo* context);

		virtual EntityCollectionIterator* NewIterator(BaseTaskInfo* context);

		virtual VError GetDistinctValues(EntityAttribute* att, DB4DCollectionManager& outCollection, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, const VCompareOptions& inOptions);

		virtual VError RemoveSelectedRange(sLONG inRecordIndex1, sLONG inRecordIndex2, BaseTaskInfo* context);

		virtual EntityCollection* NewFromWafSelection(WafSelection* wafSel, BaseTaskInfo* context);

		virtual RecIDType FindKey(VectorOfVString& vals, BaseTaskInfo* context, VError& err);

		virtual EntityCollection* ProjectCollection(EntityAttribute* att, VError& err, BaseTaskInfo* context);

		virtual VError ComputeOnOneAttribute(const EntityAttribute* att, DB4D_ColumnFormulae action, VJSValue& outVal, BaseTaskInfo* context, JS4D::ContextRef jscontext);

		VError InstallBittab(Bittab* b);

		virtual bool MatchAllocationNumber(sLONG allocationNumber);
		virtual size_t CalcLenOnDisk(void);


	protected:
		EntityCollection* _Mix(EntityCollection* other, VError& err, BaseTaskInfo* context, DB4DConjunction action);
			
		Selection* fSel;
};


class LocalEntityCollectionIterator : public EntityCollectionIterator
{
	public:
		LocalEntityCollectionIterator(LocalEntityCollection* collection, BaseTaskInfo* context);

		virtual EntityRecord* First(VError& err);
		virtual EntityRecord* Next(VError& err);
		virtual RecIDType GetCurPos() const;
		virtual EntityRecord* SetCurPos(RecIDType pos, VError& err);

	protected:
		SelectionIterator fSelIter;
};


						// ---------------------------------------------------------------------------- 


class LocalEntityModel : public EntityModel
{
	public:
		LocalEntityModel(LocalEntityModelCatalog* inOwner, Table* inMainTable = nil);

		inline Base4D* GetDB() const
		{
			return fDB;
		}

		inline Table* GetMainTable() const
		{
			return fMainTable;
		}

		virtual bool IsValid() const
		{
			return fMainTable != nil;
		}

		virtual RecIDType countEntities(BaseTaskInfo* inContext);

		virtual EntityCollection* SelectAllEntities(BaseTaskInfo* context, VErrorDB4D* outErr = NULL, 
			DB4D_Way_of_Locking HowToLock = DB4D_Do_Not_Lock, EntityCollection* outLockSet = NULL, bool allowRestrict = true);

		virtual EntityCollection* executeQuery( SearchTab* querysearch, BaseTaskInfo* context, EntityCollection* filter = nil, 
			VDB4DProgressIndicator* InProgress = NULL, DB4D_Way_of_Locking HowToLock = DB4D_Do_Not_Lock, 
			sLONG limit = 0, EntityCollection* outLockSet = NULL, VError *outErr = NULL, EntityAttributeSortedSelection* sortingAtt = NULL);

		virtual EntityCollection* executeQuery( const VString& queryString, VJSParms_callStaticFunction& ioParms, BaseTaskInfo* context, EntityCollection* filter = nil, 
			VDB4DProgressIndicator* InProgress = NULL, DB4D_Way_of_Locking HowToLock = DB4D_Do_Not_Lock, 
			sLONG limit = 0, EntityCollection* outLockSet = NULL, VError *outErr = NULL);

		virtual EntityCollection* executeQuery( const VString& queryString, VJSONArray* params, BaseTaskInfo* context, EntityCollection* filter = nil, 
			VDB4DProgressIndicator* InProgress = NULL, DB4D_Way_of_Locking HowToLock = DB4D_Do_Not_Lock, 
			sLONG limit = 0, EntityCollection* outLockSet = NULL, VError *outErr = NULL);

		virtual EntityCollection* executeQuery( VJSObject queryObj, BaseTaskInfo* context, EntityCollection* filter = nil, 
			VDB4DProgressIndicator* InProgress = NULL, DB4D_Way_of_Locking HowToLock = DB4D_Do_Not_Lock, 
			sLONG limit = 0, EntityCollection* outLockSet = NULL, VError *outErr = NULL);


		virtual EntityCollection* NewCollection(bool ordered = false, bool safeRef = false) const;
		virtual EntityCollection* NewCollection(const VectorOfVString& primKeys, VError& err, BaseTaskInfo* context) const;

		virtual EntityRecord* newEntity(BaseTaskInfo* inContext, bool forClone = false) const;

		
		RecIDType getEntityNumWithPrimKey(const VString& primkey, BaseTaskInfo* context, VError& err);
		RecIDType getEntityNumWithPrimKey(const VectorOfVString& primkey, BaseTaskInfo* context, VError& err);
		RecIDType getEntityNumWithPrimKey(const VValueBag& primkey, BaseTaskInfo* context, VError& err, bool ErrOnNull = true);
		RecIDType getEntityNumWithPrimKey(const VectorOfVValue& primkey, BaseTaskInfo* context, VError& err);
		RecIDType getEntityNumWithPrimKey(VJSObject objkey, BaseTaskInfo* context, VError& err);
		
		RecIDType getEntityNumWithIdentifyingAtts(const VectorOfVString& idents, BaseTaskInfo* context, VError& err);
		RecIDType getEntityNumWithIdentifyingAtts(const VValueBag& idents, BaseTaskInfo* context, VError& err, bool ErrOnNull = true);
		RecIDType getEntityNumWithIdentifyingAtts(const VectorOfVValue& idents, BaseTaskInfo* context, VError& err);
		

		virtual EntityRecord* findEntityWithPrimKey(const VString& primkey, BaseTaskInfo* context, VError& err, DB4D_Way_of_Locking HowToLock);
		virtual EntityRecord* findEntityWithPrimKey(const VectorOfVString& primkey, BaseTaskInfo* context, VError& err, DB4D_Way_of_Locking HowToLock);
		virtual EntityRecord* findEntityWithPrimKey(const VValueBag& primkey, BaseTaskInfo* context, VError& err, DB4D_Way_of_Locking HowToLock);
		virtual EntityRecord* findEntityWithPrimKey(const VectorOfVValue& primkey, BaseTaskInfo* context, VError& err, DB4D_Way_of_Locking HowToLock);
		virtual EntityRecord* findEntityWithPrimKey(VJSObject objkey, BaseTaskInfo* context, VError& err, DB4D_Way_of_Locking HowToLock);

		virtual EntityRecord* findEntityWithIdentifyingAtts(const VectorOfVString& idents, BaseTaskInfo* context, VError& err, DB4D_Way_of_Locking HowToLock);
		virtual EntityRecord* findEntityWithIdentifyingAtts(const VValueBag& idents, BaseTaskInfo* context, VError& err, DB4D_Way_of_Locking HowToLock);
		virtual EntityRecord* findEntityWithIdentifyingAtts(const VectorOfVValue& idents, BaseTaskInfo* context, VError& err, DB4D_Way_of_Locking HowToLock);

		virtual VJSValue call_Method(const VString& inMethodName,VJSArray& params, VJSObject& thisObj, BaseTaskInfo* context, JS4D::ContextRef jscontext, VError& err);
		virtual VError SetAutoSequenceNumber(sLONG8 newnum, BaseTaskInfo* context);

		EntityRecord* LoadEntity(sLONG n, VError& err, DB4D_Way_of_Locking HowToLock, BaseTaskInfo* context);
		bool MatchPrimKeyWithDataSource() const;

		//virtual VError compute(EntityAttributeSortedSelection& atts, EntityCollection* collec, VJSObject& outObj, BaseTaskInfo* context, JS4D::ContextRef jscontext);

		virtual bool IsIndexed(const EntityAttribute* att) const;
		virtual bool IsFullTextIndexed(const EntityAttribute* att) const;

		virtual bool IsIndexable(const EntityAttribute* att) const;
		virtual bool IsFullTextIndexable(const EntityAttribute* att) const;

		virtual bool IsRemote() const
		{
			return false;
		};

		Field* RetainDirectField(const EntityAttribute* att) const;

		Field* RetainField(const EntityAttribute* att, bool includingForeignKey = false) const;

		VError MatchExtendedTable();
		VError MatchWithTable();
		VError BuildIndexes();

		Table* GetBaseTable();


		VError BuildLocalEntityModelRelations(LocalEntityModelCatalog* catalog);



	protected:
		Table* fMainTable;
		Base4D* fDB;

};




					// ---------------------------------------------------------------------------- 



class db4dEntityAttribute : public EntityAttribute // attention cette classe ne doit contenir aucune donnee privee ou protected
{												// cette classe n'existe que comme access rapide au LocalEntityModel
public:

	LocalEntityModel* GetLocalModel() const
	{
#if debuglr
		LocalEntityModel* locModel = dynamic_cast<LocalEntityModel*>(fOwner);
		xbox_assert(fOwner != nil);
#else
		LocalEntityModel* locModel = (LocalEntityModel*)fOwner;
#endif
		return locModel;
	}

	Table* GetTable() const
	{
		LocalEntityModel* locmodel = GetLocalModel();
		return locmodel->GetMainTable();
	}

	Field* RetainField(bool includingForeignKey = false) const
	{
		LocalEntityModel* locmodel = GetLocalModel();
		return locmodel->RetainField(this, includingForeignKey);
	}

	Field* RetainDirectField() const
	{
		LocalEntityModel* locmodel = GetLocalModel();
		return locmodel->RetainDirectField(this);
	}

	VError MatchField(Table* tt);

	VError BuildIndexes(Table* tt);

};


					// ---------------------------------------------------------------------------- 



class LocalEntityModelCatalog : public EntityModelCatalog
{
	public:
		LocalEntityModelCatalog(Base4D* inDB):EntityModelCatalog(inDB)
		{
			fOwner = inDB;
		}

		virtual void GetName(VString& outName) const;
		virtual EntityModel* NewModel();

		virtual VError do_LoadEntityModels(const VValueBag& bagEntities, ModelErrorReporter* errorReporter, unordered_map_VString<VRefPtr<VFile> >* outIncludedFiles, BaseTaskInfo* context);
		virtual VError do_SecondPassLoadEntityModels(ModelErrorReporter* errorReporter);

		VError BuildLocalEntityModelRelations();

		inline Base4D* GetDB() const
		{
			return fOwner;
		}

		virtual VError SetLogFile(VFile* logfile, bool append, VJSONObject* options);


	protected:
		Base4D* fOwner;
};



				// ---------------------------------------------------------------------------- 


class LocalEntityRecord : public EntityRecord
{
	public:
		LocalEntityRecord(LocalEntityModel* inModel, FicheInMem* inRec, BaseTaskInfo* inContext): EntityRecord((EntityModel*)inModel, inContext)
		{
			fMainRec = RetainRefCountable(inRec);
			if (fMainRec == nil)
				fModificationStamp = 0;
			else
			{
				fModificationStamp = fMainRec->GetModificationStamp();
				xbox_assert( fMainRec->GetOwner() == inModel->GetMainTable());
			}
			fStamp = 0;
		}

		inline FicheInMem* getRecord() const
		{
			return fMainRec;
		}

		virtual void TouchAttribute(const EntityAttribute* att);
		virtual bool ContainsBlobData(const EntityAttribute* inAttribute, BaseTaskInfo* context);

		virtual EntityAttributeValue* do_getAttributeValue(const EntityAttribute* inAttribute, VError& err, BaseTaskInfo* context, bool restrictValue = false) ;
		virtual VError do_setAttributeValue(const EntityAttribute* inAttribute, const VValueSingle* inValue);
		virtual EntityRecord* do_LoadRelatedEntity(const EntityAttribute* inAttribute, const EntityAttribute* relatedAttribute, BaseTaskInfo* context, VError& err, DB4D_Way_of_Locking HowToLock = DB4D_Do_Not_Lock);

		sLONG GetNum() const
		{
			if (fMainRec == nil)
				return -1;
			else
				return fMainRec->GetNum();
		}

		virtual sLONG GetModificationStamp() const
		{
			if (fMainRec == nil)
				return 0;
			else
				return fMainRec->GetModificationStamp();
		}


		virtual VError Save(Transaction* *trans, BaseTaskInfo* context, uLONG stamp, bool allowOverrideStamp = false);
		virtual VErrorDB4D Save(uLONG stamp, bool allowOverrideStamp = false);

//		virtual VError Validate(BaseTaskInfo* context);
//		virtual VErrorDB4D Validate();

		virtual Boolean IsNew() const;
		virtual Boolean IsProtected() const;
		virtual Boolean IsModified() const;

		virtual void GetTimeStamp(VTime& outValue);

		virtual VError do_Reload();

		virtual VErrorDB4D Drop();

		FicheInMem* GetRecordForAttribute(const EntityAttribute* inAttribute);

		virtual uLONG GetStamp() const
		{
			return fStamp;
		}

		virtual void Touch(BaseTaskInfo* context)
		{
			fStamp++;
			CallDBEvent(dbev_set, context);
		}

		virtual VError SetForeignKey(const EntityAttribute* att, EntityRecord* relatedEntity, const VValueSingle* inForeignKey);

		virtual bool do_GetPrimKeyValueAsObject(VJSObject& outObj);
		
		virtual bool do_GetPrimKeyValueAsBag(VValueBag& outBagKey);

		virtual bool do_GetPrimKeyValueAsString(VString& outKey, bool autoQuotes);

		virtual bool do_GetPrimKeyValueAsVValues(VectorOfVValue& outPrimkey);

		virtual VError DuplicateInto(EntityRecord* otherRec);


	protected:

		virtual ~LocalEntityRecord()
		{
			QuickReleaseRefCountable(fMainRec);
		}

		FicheInMem* fMainRec;
		uLONG fModificationStamp, fStamp;
		VLong8 fPseudoKey;

};



#endif

