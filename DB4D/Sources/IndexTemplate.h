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
#ifndef __INDEXTEMPLATE__
#define __INDEXTEMPLATE__

/*
const sLONG kNbKeysForLong = 256;
const sLONG kNbKeysForLong8 = 256;
const sLONG kNbKeysForWord = 256;
const sLONG kNbKeysForByte = 256;
const sLONG kNbKeysForReal = 256;
*/
#if VERSIONWIN
#pragma warning( push )
#pragma warning (disable: 4263)	// IndexHeader::PlaceCle and CompareKeys are hiding inherited function
#pragma warning (disable: 4264)
#endif

class Transaction;

template <class Type>
class BtreePageDataHeader { public: enum { DBOH_BtreePageTemplate = 'xxxx', IndexInfo_Type = 0 }; };

template <>
class BtreePageDataHeader<sLONG> { public: enum { DBOH_BtreePageTemplate = 'btp1', IndexInfo_Type = DB4D_Index_OnOneField_Scalar_Long }; };

template <>
class BtreePageDataHeader<sLONG8> { public: enum { DBOH_BtreePageTemplate = 'btp2', IndexInfo_Type = DB4D_Index_OnOneField_Scalar_Long8 }; };

template <>
class BtreePageDataHeader<sWORD> { public: enum { DBOH_BtreePageTemplate = 'btp3', IndexInfo_Type = DB4D_Index_OnOneField_Scalar_Word }; };

template <>
class BtreePageDataHeader<sBYTE> { public: enum { DBOH_BtreePageTemplate = 'btp4', IndexInfo_Type = DB4D_Index_OnOneField_Scalar_Byte }; };

template <>
class BtreePageDataHeader<Real> { public: enum { DBOH_BtreePageTemplate = 'btp5', IndexInfo_Type = DB4D_Index_OnOneField_Scalar_Real }; };

template <>
class BtreePageDataHeader<uBYTE> { public: enum { DBOH_BtreePageTemplate = 'btp6', IndexInfo_Type = DB4D_Index_OnOneField_Scalar_Bool }; };

template <>
class BtreePageDataHeader<uLONG8> { public: enum { DBOH_BtreePageTemplate = 'btp7', IndexInfo_Type = DB4D_Index_OnOneField_Scalar_Time }; };

template <>
class BtreePageDataHeader<VUUIDBuffer> { public: enum { DBOH_BtreePageTemplate = 'btp8', IndexInfo_Type = DB4D_Index_OnOneField_Scalar_UUID }; };

template <class Type, sLONG MaxCles> class BtreePage;

template <class Type, sLONG MaxCles> class FullCleIndex;


template <class Type>
class TransUniqCleIndex
{
	public:
		TransUniqCleIndex(Type cle, Transaction* owner, bool temporary)
		{
			fCle = cle;
			fOwner = owner;
			fIsTemp = temporary;
		}

		inline bool operator < (const TransUniqCleIndex<Type>& other) const
		{
			return fCle < other.fCle;
		}

		Type fCle;
		Transaction* fOwner;
		mutable bool fIsTemp;
};


template <class Type>
class TransCleIndex
{
	public:

		/*
		inline TransCleIndex()
		{
			isnull = false;
		}
		*/

		/*
		inline TransCleIndex(const BTitemIndex* val)
		{
			if (val == nil)
			{
				qui = -1;
				//isnull = true;
			}
			else
			{
				//isnull = false;
				qui = val->qui;
				cle = *((Type*)(&val->data));
			}
		}
		*/
		
		inline TransCleIndex<Type>& operator = (const CleIndex<Type>& From)
		{
			qui = From.qui;
			cle = From.cle;
			//isnull = false;
			return *this;
		}
		
						
		inline BTitemIndex* BuildBTItem(IndexInfo* ind) const
		{
			BTitemIndex *res = ind->AllocateKey(sizeof(BTitemIndex) - sizeof(void*) + sizeof(Type));
			if (res != nil)
			{
				*((Type*)(&res->data)) = cle;
				res->qui = qui;
			}
			return NULL;
		}
		
		
		inline bool operator < (const TransCleIndex<Type>& other) const
		{
			/*
			if (other.isnull)
				return false;
			else
			{
				if (isnull)
					return true;
				else
					return cle < other.cle;
			}
			*/
			if (cle == other.cle)
				return qui < other.qui;
			else
				return cle < other.cle;
		}
		
		
		//inline void SetNull(Boolean null) { isnull = null; };
		//inline Boolean IsNull() const { return isnull; };
		
		sLONG qui;
		Type cle;
		//Boolean isnull;
};



template <class Type, sLONG MaxCles>
class FullCleIndex
{
	public:
	
		inline FullCleIndex()
		{
			souspage = -1;
			isnull = false;
			sousBT = nil;
		}


		inline FullCleIndex(const VValueSingle* val)
		{
			souspage = -1;
			sousBT = nil;
			isnull = false;
			qui = -1;
			val->WriteToPtr(&cle);
		}

		inline FullCleIndex(const TransCleIndex<Type>& val)
		{
			souspage = -1;
			sousBT = nil;
			isnull = false;
			qui = val.qui;
			cle = val.cle;
		}

		inline FullCleIndex<Type, MaxCles>& operator = (const TransCleIndex<Type>& val)
		{
			souspage = -1;
			sousBT = nil;
			isnull = false;
			qui = val.qui;
			cle = val.cle;
			return *this;
		}

		inline FullCleIndex(const BTitemIndex* val)
		{
			sousBT = nil;
			if (val == nil)
			{
				qui = -1;
				isnull = true;
				//souspage = -1;
				//cle = Type();
			}
			else
			{
				isnull = false;
				qui = val->qui;
				cle = *((Type*)(&val->data));
				souspage = val->souspage;
			}
		}
	
		inline TransCleIndex<Type> ConvertToTransCle() const
		{
			TransCleIndex<Type> Result;
			Result.qui = qui;
			Result.cle = cle;
			return Result;
		}
	
		void SwapBytes()
		{
			ByteSwap(&souspage);
			ByteSwap(&qui);
			ByteSwap(&cle);
		}
		
		
		inline FullCleIndex<Type, MaxCles>& operator = (const CleIndex<Type>& From)
		{
			sousBT = nil;
			souspage = From.souspage;
			qui = From.qui;
			cle = From.cle;
			isnull = false;
			return *this;
		}
		
		inline void CopyTo(TransCleIndex<Type>& into)
		{
			into.cle = cle;
			into.qui = qui;
		}

		inline void CopyTo(CleIndex<Type>& into)
		{
			into.cle = cle;
			into.qui = qui;
			into.souspage = souspage;
		}

		inline BTitemIndex* BuildBTItem(IndexInfo* ind) const
		{
			BTitemIndex *res = ind->AllocateKey(sizeof(BTitemIndex) - sizeof(void*) + sizeof(Type));
			if (res != nil)
			{
				*((Type*)(&res->data)) = cle;
				res->qui = qui;
			}
			return res;
		}
		
		inline void SetNull(Boolean null) { isnull = null; };
		inline Boolean IsNull() const { return isnull; };
		
		BtreePage<Type, MaxCles>* sousBT;
		sLONG souspage;
		sLONG qui;
		Type cle;
		Boolean isnull;
	
};


class distinctvalue_iterator
{
public:
	distinctvalue_iterator()
	{
		nbelem = 0;
		truenbelem = 0;
	};

	sLONG nbelem;
	sLONG truenbelem;
};


inline void operator += (sLONG8& one, VUUIDBuffer& two)
{
}


inline void operator += (Real& one, VUUIDBuffer& two)
{
}

template <class Type, sLONG MaxCles>
class BtreePage : public ObjInCacheMem, public IObjToFlush
{
	public:


		BtreePage(IndexInfo* xentete, sLONG xnum, Boolean iscluster, sWORD DefaultAccess = ObjCache::PageIndexAccess, BtreePageData<Type, MaxCles>* fromBTP = nil);

		inline const CleIndex<Type>* GetItemPtr(sLONG n) const
		{
			xbox_assert(n >= 0 && n < btp.nkeys);
			return &(btp.cles[n]);
		}
		
		inline CleIndex<Type>* GetItemPtr(sLONG n)
		{
			xbox_assert(n >= 0 && n < btp.nkeys);
			return &(btp.cles[n]);
		}

		void GetItem(OccupableStack* curstack, sLONG n, FullCleIndex<Type, MaxCles>& outItem, Boolean chargesous, BaseTaskInfo* context, VError& err);
		
		VError ThrowError( VError inErrCode, ActionDB4D inAction);

		inline sLONG CompareKeys(const CleIndex<Type> *val, sLONG inIndex, const VCompareOptions& inOptions)
		{
			CleIndex<Type> *v = GetItemPtr(inIndex);
			if (v->cle == val->cle)
				return CR_EQUAL;
			else
				return val->cle < v->cle ? CR_SMALLER : CR_BIGGER;
		}
		
		inline sLONG CompareKeys(sLONG inIndex, const CleIndex<Type> *val, const VCompareOptions& inOptions)
		{
			CleIndex<Type> *v = GetItemPtr(inIndex);
			if (v->cle == val->cle)
				return CR_EQUAL;
			else
				return v->cle < val->cle  ? CR_SMALLER : CR_BIGGER;
		}

		inline sLONG CompareKeysStrict(const CleIndex<Type> *val, sLONG inIndex)
		{
			CleIndex<Type> *v = GetItemPtr(inIndex);
			if (v->cle == val->cle)
				return CR_EQUAL;
			else
				return val->cle < v->cle ? CR_SMALLER : CR_BIGGER;
		}
		
		inline sLONG CompareKeysStrict(sLONG inIndex, const CleIndex<Type> *val)
		{
			CleIndex<Type> *v = GetItemPtr(inIndex);
			if (v->cle == val->cle)
				return CR_EQUAL;
			else
				return v->cle < val->cle  ? CR_SMALLER : CR_BIGGER;
		}
		

		inline sLONG CompareKeys(const FullCleIndex<Type, MaxCles>& val, sLONG inIndex, const VCompareOptions& inOptions)
		{
			CleIndex<Type> *v = GetItemPtr(inIndex);
			if (v->cle == val.cle)
				return CR_EQUAL;
			else
				return val.cle < v->cle ? CR_SMALLER : CR_BIGGER;
		}
		
		inline sLONG CompareKeys(sLONG inIndex, const FullCleIndex<Type, MaxCles>& val, const VCompareOptions& inOptions)
		{
			CleIndex<Type> *v = GetItemPtr(inIndex);
			if (v->cle == val.cle)
				return CR_EQUAL;
			else
				return v->cle < val.cle  ? CR_SMALLER : CR_BIGGER;
		}

		inline sLONG CompareKeysPtr(sLONG inIndex, const Type* val, const VCompareOptions& inOptions)
		{
			CleIndex<Type> *v = GetItemPtr(inIndex);
			if (v->cle == *val)
				return CR_EQUAL;
			else
				return v->cle < *val  ? CR_SMALLER : CR_BIGGER;
		}

		inline sLONG CompareKeysStrict(const FullCleIndex<Type, MaxCles>& val, sLONG inIndex)
		{
			CleIndex<Type> *v = GetItemPtr(inIndex);
			if (v->cle == val.cle)
				return CR_EQUAL;
			else
				return val.cle < v->cle ? CR_SMALLER : CR_BIGGER;
		}
		
		inline sLONG CompareKeysStrict(sLONG inIndex, const FullCleIndex<Type, MaxCles>& val)
		{
			CleIndex<Type> *v = GetItemPtr(inIndex);
			if (v->cle == val.cle)
				return CR_EQUAL;
			else
				return v->cle < val.cle  ? CR_SMALLER : CR_BIGGER;
		}

		static inline VValueSingle* ConvertKeyToVValue(Type cle)  
		{
#if __GNUC__
#else
			tagada++; // compiler error
#endif
			xbox_assert(false); // cette methode doit absolument etre specialisee
			return nil;
		}
		
		VError loadobj(DataAddr4D xaddr, sLONG len);
		VError savepage(OccupableStack* curstack, BaseTaskInfo* context);
				
		virtual bool SaveObj(VSize& outSizeSaved);

		virtual bool MustCheckOccupiedStamp() const
		{
			return false; // use the new algorithm to check if the object is valid while flushing
		}

		virtual void setmodif(uBOOL xismodif, Base4D* bd, BaseTaskInfo* context)
		{
			/*if (xismodif)
				fStopWrite = 0;*/
			IObjToFlush::setmodif(xismodif, bd, context);
		}

		void LiberePage(OccupableStack* curstack, BaseTaskInfo* context);
		static BtreePage<Type, MaxCles>* LoadPage(OccupableStack* curstack, sLONG n, IndexInfo* xentete, BaseTaskInfo* context, VError& err);
		static BtreePage<Type, MaxCles>* AllocatePage(OccupableStack* curstack, IndexInfo* xentete, BaseTaskInfo* context, VError& err);
		
		VError Place(OccupableStack* curstack, uBOOL *h, FullCleIndex<Type, MaxCles>& v, BaseTaskInfo* context);
		
		void underflow(OccupableStack* curstack, BtreePage<Type, MaxCles>* pc, sLONG s, uBOOL *h, uBOOL *doisdel, BaseTaskInfo* context);
		void del(OccupableStack* curstack, BtreePage<Type, MaxCles>* from, sLONG k, uBOOL *h, uBOOL *doisdel, BaseTaskInfo* context);
		VError Detruire(OccupableStack* curstack, uBOOL *h, const FullCleIndex<Type, MaxCles>& v, uBOOL *doisdel, BaseTaskInfo* context);
		
		inline sLONG GetNum(void) { return(num); };
		
		void MoveAndAddItems(sLONG from, sLONG howmany, BtreePage<Type, MaxCles>* into);
		void MoveAndInsertItemsAtBegining(sLONG from, sLONG howmany, BtreePage<Type, MaxCles>* into);
		void InsertKey(sLONG n, CleIndex<Type>* u);
		void InsertKey(sLONG n, FullCleIndex<Type, MaxCles>& u, Boolean AvecSousPage = false);
		void SetKey(sLONG n, FullCleIndex<Type, MaxCles>& u, Boolean AvecSousPage = false);
		void AddKey(Type key, sLONG qui, BtreePage<Type, MaxCles>* sousBT);
		void SupKey(sLONG n);
		void SupKeys(sLONG n, sLONG howmany);
		void DelKeys(sLONG from);
		
		inline sLONG GetNKeys() const { return btp.nkeys; };
		
		void SetSousPage(sLONG n, BtreePage<Type, MaxCles>* sousBT);
		
		BtreePage<Type, MaxCles>* GetSousPage(OccupableStack* curstack, sLONG n, VError &err, BaseTaskInfo* context);
		
		
		inline sLONG GetSousPageNum(sLONG n)
		{
			if (n == 0)
				return btp.souspage0;
			else
			{
				return btp.cles[n-1].souspage;
			}
		}
		
		inline sLONG GetQui(sLONG n)
		{
			xbox_assert(n >= 0 && n < btp.nkeys);
			return btp.cles[n].qui;
		}
		
		void SetQui(sLONG n, sLONG xqui)
		{
			xbox_assert(n >= 0 && n < btp.nkeys);
			btp.cles[n].qui = xqui;
		}
		
		//inline void SetEncours(uBOOL b) { encours=b; };
		
		void LibereEspaceMem(OccupableStack* curstack);
		void DelFromFlush(OccupableStack* curstack);
		
		VError SelectAllKeys(OccupableStack* curstack, Bittab* b, BaseTaskInfo* context, ProgressEncapsuleur* InProgress, FullCleIndex<Type, MaxCles>* outVal = nil);

		VError FourcheScalar(OccupableStack* curstack, Bittab* b, const FullCleIndex<Type, MaxCles>& val1, uBOOL xstrict1, const FullCleIndex<Type, MaxCles>& val2, uBOOL xstrict2, BaseTaskInfo* context, 
										ProgressEncapsuleur* InProgress, const VCompareOptions& inOptions, FullCleIndex<Type, MaxCles>* outVal = nil);

		VError FindKeyInArray(OccupableStack* curstack, Bittab* b, DB4DArrayOfValues* values, BaseTaskInfo* context, const VCompareOptions& inOptions, ProgressEncapsuleur* InProgress);
		VError FindKeyInArrayScalar(OccupableStack* curstack, Bittab* b, DB4DArrayOfConstDirectValues<Type>* values, const Type* &CurVal, BaseTaskInfo* context, const VCompareOptions& inOptions, ProgressEncapsuleur* InProgress);
													

		//VError SortSel(OccupableStack* curstack, sLONG& curfic, sLONG maxnb, Bittab* filtre, Selection* sel, uBOOL ascent, BaseTaskInfo* context, 
														//ProgressEncapsuleur* InProgress, Boolean TestUnicite, BTitemIndex* &curkey);

		VError GetDistinctValues(OccupableStack* curstack, DB4DCollectionManager& outCollection, BaseTaskInfo* context, Bittab* filtre, 
								sLONG &curpage, sLONG &curvalue, Type &curkey, Boolean& dejaval, ProgressEncapsuleur* InProgress, 
								VCompareOptions& inOptions, distinctvalue_iterator& xx);

		VError QuickGetDistinctValues(OccupableStack* curstack, DB4DArrayOfDirectValues<Type> *outCollection, BaseTaskInfo* context, Bittab* filtre, 
									sLONG &curpage, sLONG &curvalue, Type &curkey, Boolean& dejaval, ProgressEncapsuleur* InProgress, 
									VCompareOptions& inOptions);

		VError CalculateColumnFormulas(OccupableStack* curstack, ColumnFormulas* formules, BaseTaskInfo* context, Bittab* filtre, sLONG &curpage, 
																	 ProgressEncapsuleur* InProgress, Boolean& stop);

		template <class ResultType>
		VError CalculateSum(OccupableStack* curstack, BaseTaskInfo* context, Bittab* filtre, sLONG &curpage, 
							ProgressEncapsuleur* InProgress, Boolean& stop, sLONG& outCount, ResultType& outSum)
		{
			BtreePage<Type, MaxCles> *sousBT;
			sLONG i,nb,l;
			VError err = VE_OK;

			if (InProgress != nil)
			{
				if (!InProgress->Progress(curpage))
					err = ThrowError(VE_DB4D_ACTIONCANCELEDBYUSER, DBaction_CalculateFomulasWithIndex);
			}

			nb = btp.nkeys;

			if (err == VE_OK)
			{
				sousBT = GetSousPage(curstack, 0, err, context);
				if (sousBT != nil && err == VE_OK)
				{
					curpage++;
					err = sousBT->CalculateSum(curstack, context, filtre, curpage, InProgress, stop, outCount, outSum);
					sousBT->Free(curstack, true);
				}

				if (!stop)
				{
					for (i=0;i<nb && err == VE_OK;i++)
					{
						Type curkey = GetItemPtr(i)->cle;
						l = GetQui(i);
						if (IsCluster)
						{
							if (l>=0)
							{
								if (filtre == nil || filtre->isOn(l))
								{
									outCount++;
									outSum += curkey;
									if (stop)
										break;
								}
							}
							else
							{
								Selection* sel = entete->GetClusterSel(curstack)->GetSel(curstack, l, context, err);
								if (sel != nil)
								{
									SelectionIterator itersel(sel);
									sLONG currec = itersel.FirstRecord();
									while (currec != -1)
									{
										if (filtre == nil || filtre->isOn(currec))
										{
											outCount++;
											outSum += curkey;
										}
										if (stop)
											break;
										currec = itersel.NextRecord();
									}

									sel->Release();
								}
								if (stop)
									break;
							}
						}
						else
						{
							if (filtre == nil || filtre->isOn(l))
							{
								outCount++;
								outSum += curkey;
								if (stop)
									break;
							}

						}

						if (!stop)
						{
							sousBT = GetSousPage(curstack, i+1, err, context);
							if (sousBT != nil && err == VE_OK)
							{
								curpage++;
								err = sousBT->CalculateSum(curstack, context, filtre, curpage, InProgress, stop, outCount, outSum);
								sousBT->Free(curstack, true);
								if (stop)
									break;
							}
						}

					}

				}
			}
			return err;
		}

		/*
		VError CalculateMin(OccupableStack* curstack, BaseTaskInfo* context, Bittab* filtre, sLONG &curpage, 
												ProgressEncapsuleur* InProgress, Boolean& stop, VValueSingle* &result);

		VError CalculateMax(OccupableStack* curstack, BaseTaskInfo* context, Bittab* filtre, sLONG &curpage, 
												ProgressEncapsuleur* InProgress, Boolean& stop, VValueSingle* &result);
												*/

		VError CalculateColumnFormulasOnOneKey(OccupableStack* curstack, ColumnFormulas* formules, BTitemIndex* u, Boolean& stop, BaseTaskInfo* context);

		inline IndexInfo* GetEntete() { return entete; };

		VError FindKeyAndBuildPath(OccupableStack* curstack, BTitemIndex* val, const VCompareOptions& inOptions, BaseTaskInfo* context, VDB4DIndexKey* outKey, Boolean& trouve);
		VError NextKey(OccupableStack* curstack, const VDB4DIndexKey* inKey, sLONG level, BaseTaskInfo* context, VDB4DIndexKey* outKey, Boolean& outlimit);
		VError GetFirstKey(OccupableStack* curstack, BaseTaskInfo* context, VDB4DIndexKey* outKey);

		VError ScanIndex(OccupableStack* curstack, Selection* into, sLONG& currec, sLONG inMaxRecords, Boolean KeepSorted, Boolean Ascent, BaseTaskInfo* context, Bittab* filtre, ProgressEncapsuleur* InProgress);
		VError SortSel(OccupableStack* curstack, sLONG& curfic, sLONG maxnb, Bittab* filtre, Selection* sel, uBOOL ascent, BaseTaskInfo* context, 
							ProgressEncapsuleur* InProgress, Bittab* inWafSelbits, WafSelection* outWafSel);

		Boolean FreePageMem(sLONG allocationBlockNumber, VSize combien, VSize& tot);

		inline void DoNotVerify()
		{
			fDebugVerify = false;
		}

		VSize GetSize() const
		{
			return sizeof(btp);
		}
		/*
		inline void ForbidWrite()
		{
			if (fStopWrite == 0)
				VInterlocked::Increment(&fStopWrite);
		}
		*/

#if debugindex_strong
		void debug_checkKeyOrder(CleIndex<Type>* &curkey);
#endif

#if debuglr
		void Display(void);
		void checktabmem(void);

		void DisplayKeys(const VString& message);

		void checkPosSousBT(BtreePage<Type, MaxCles>* sousBT);

		void CheckPageKeys();
		void CheckPageOwner();

		void checkWrongRecordRef(sLONG limit, CleIndex<Type>* &curval);

#if debugCheckIndexPageOnDiskInDestructor
		void DebugCheckPageOnDisk();
#endif

#endif
	
	protected:
		virtual ~BtreePage();

		IndexInfo *entete;
		BtreePage<Type, MaxCles>* tabmem[MaxCles+1];
		sLONG num;
		//sLONG fStopWrite;
		uBOOL IsCluster/*, encours*/;
		uBOOL fDebugVerify;
		VCriticalSection fLoadPageMutex;
		BtreePageData<Type, MaxCles> btp;

};



				/* ------------------------- */
				


template <>
inline VValueSingle* BtreePage<sLONG, kNbKeysForScalar>::ConvertKeyToVValue(sLONG cle)  
{
	return new VLong(cle);
}


				/* ------------------------- */



template <>
inline VValueSingle* BtreePage<sLONG8, kNbKeysForScalar>::ConvertKeyToVValue(sLONG8 cle)  
{
	return new VLong8(cle);
}

				/* ------------------------- */


template <>
inline VValueSingle* BtreePage<sWORD, kNbKeysForScalar>::ConvertKeyToVValue(sWORD cle)  
{
	return new VWord(cle);
}


				/* ------------------------- */


template <>
inline VValueSingle* BtreePage<sBYTE, kNbKeysForScalar>::ConvertKeyToVValue(sBYTE cle)  
{
	return new VByte(cle);
}


				/* ------------------------- */


template <>
inline VValueSingle* BtreePage<uBYTE, kNbKeysForScalar>::ConvertKeyToVValue(uBYTE cle)  
{
	return new VBoolean(cle);
}



				/* ------------------------- */


template <>
inline VValueSingle* BtreePage<uLONG8, kNbKeysForScalar>::ConvertKeyToVValue(uLONG8 cle)  
{
	return new VTime(cle);
}


				/* ------------------------- */


template <>
inline VValueSingle* BtreePage<VUUIDBuffer, kNbKeysForScalar>::ConvertKeyToVValue(VUUIDBuffer cle)  
{
	return new VUUID(cle);
}


				/* ------------------------- */


template <>
inline VValueSingle* BtreePage<Real, kNbKeysForScalar>::ConvertKeyToVValue(Real cle)  
{
	return new VReal(cle);
}


template <>
inline sLONG BtreePage<Real, kNbKeysForScalar>::CompareKeys(const CleIndex<Real> *val, sLONG inIndex, const VCompareOptions& inOptions)
{
	CleIndex<Real> *v = GetItemPtr(inIndex);
	if (fabs(v->cle - val->cle) < inOptions.GetEpsilon())
		return CR_EQUAL;
	else
		return val->cle < v->cle ? CR_SMALLER : CR_BIGGER;
}

template <>
inline sLONG BtreePage<Real, kNbKeysForScalar>::CompareKeys(sLONG inIndex, const CleIndex<Real> *val, const VCompareOptions& inOptions)
{
	CleIndex<Real> *v = GetItemPtr(inIndex);
	if (fabs(v->cle - val->cle) < inOptions.GetEpsilon())
		return CR_EQUAL;
	else
		return v->cle < val->cle  ? CR_SMALLER : CR_BIGGER;
}


template <>
inline sLONG BtreePage<Real, kNbKeysForScalar>::CompareKeys(const FullCleIndex<Real, kNbKeysForScalar>& val, sLONG inIndex, const VCompareOptions& inOptions)
{
	CleIndex<Real> *v = GetItemPtr(inIndex);
	if (fabs(v->cle - val.cle) < inOptions.GetEpsilon())
		return CR_EQUAL;
	else
		return val.cle < v->cle ? CR_SMALLER : CR_BIGGER;
}

template <>
inline sLONG BtreePage<Real, kNbKeysForScalar>::CompareKeys(sLONG inIndex, const FullCleIndex<Real, kNbKeysForScalar>& val, const VCompareOptions& inOptions)
{
	CleIndex<Real> *v = GetItemPtr(inIndex);
	if (fabs(v->cle - val.cle) < inOptions.GetEpsilon())
		return CR_EQUAL;
	else
		return v->cle < val.cle  ? CR_SMALLER : CR_BIGGER;
}


template <>
inline sLONG BtreePage<Real, kNbKeysForScalar>::CompareKeysPtr(sLONG inIndex, const Real* val, const VCompareOptions& inOptions)
{
	CleIndex<Real> *v = GetItemPtr(inIndex);
	if (fabs(v->cle - *val) < inOptions.GetEpsilon())
		return CR_EQUAL;
	else
		return v->cle < *val  ? CR_SMALLER : CR_BIGGER;
}



				/* ------------------------------------------------------------ */


template <class Type, sLONG MaxCles>
class IndexHeaderBTreeScalar : public IndexHeaderBTreeRoot
{
	public:
		
		inline IndexHeaderBTreeScalar(IndexInfo *xentete, Boolean iscluster):IndexHeaderBTreeRoot(xentete),cls(xentete)
		{ 
			firstpage = nil;
			fIsCluster = iscluster;
			cls.Init(xentete->GetDB(),this, xentete->GetTargetTable());
			if (iscluster)
				typindex = DB4D_Index_BtreeWithCluster;

		};


		virtual void SetAssoc(IndexInfo* x)
		{
			IndexHeaderBTreeRoot::SetAssoc(x);
			cls.Init(x->GetDB(), this, x->GetTargetTable());
		}

		virtual BTreePageIndex* CrePage(void)
		{
			xbox_assert(false);
			return nil;
		}
		
		virtual BTreePageIndex* LoadPage(OccupableStack* curstack, DataAddr4D addr, sLONG len, BaseTaskInfo* context, VError& err, sLONG xnum)
		{
			xbox_assert(false);
			return nil;
		}

		virtual ClusterSel* GetClusterSel(OccupableStack* curstack) { return (&cls) ;};
		
		virtual const ClusterSel* GetClusterSel(OccupableStack* curstack) const { return (&cls) ;};


		virtual VError LibereEspaceDisk(OccupableStack* curstack, VDB4DProgressIndicator* InProgress = nil)
		{
			VError err = VE_OK;
			
			Occupy(curstack, true);
			if (firstpage != nil) 
			{
				firstpage->DelFromFlush(curstack);
			}
			if (fIsCluster)
				err = cls.LibereEspaceDisk(curstack, InProgress);
			if (err == VE_OK)
			{
				err = IndexHeaderBTreeRoot::LibereEspaceDisk(curstack, InProgress);
			}
			HBT.FirstPage = -1;
			Free(curstack, true);

			return err;
		}

		virtual void LibereEspaceMem(OccupableStack* curstack)
		{
			Occupy(curstack, true);
			if (fIsCluster)
				cls.LibereEspaceMem(curstack);
			if (firstpage != nil) 
			{
				firstpage->LibereEspaceMem(curstack);
				firstpage->Release();
			}
			firstpage=nil;
			IndexHeaderBTreeRoot::LibereEspaceMem(curstack);
			Free(curstack, true);
		}

		virtual void TryToFreeMem(sLONG allocationBlockNumber, VSize combien, VSize& tot);

		virtual sLONG8 GetNBDiskAccessForAScan(Boolean WithSelFilter) { return IHD.nbpage + (WithSelFilter && fIsCluster ? cls.GetNBDiskAccessForAScan() : 0); };

		virtual VError InitTablesAddr(OccupableStack* curstack)
		{
			VError err = IndexHeaderBTreeRoot::InitTablesAddr(curstack);
			if (err == VE_OK && fIsCluster)
			{
				err = cls.InitTableAddr(curstack);
			}
			return err;
		}
		
		
		virtual VError NormalizeTablesAddr(OccupableStack* curstack)
		{
			VError err = IndexHeaderBTreeRoot::NormalizeTablesAddr(curstack);
			if (err == VE_OK && fIsCluster)
			{
				err = cls.NormalizeTableAddr(curstack);
			}
			return err;
		}
		

		virtual VError SetClusterAddr(OccupableStack* curstack, sLONG numclust, DataAddr4D SelAddr, sLONG SelLen)
		{
			if (numclust == kSpecialClustNulls)
				return IndexHeader::SetClusterAddr(curstack, numclust, SelAddr, SelLen);
			else
			{
				if (fIsCluster)
					return cls.SetClusterAddr(curstack, numclust, SelAddr, SelLen);
				else
					return VE_DB4D_NOTIMPLEMENTED;
			}
		}

		virtual sLONG GetLen(void)
		{
			if (fIsCluster)
				return IndexHeaderBTreeRoot::GetLen();
			else
				return(IndexHeaderBTreeRoot::GetLen()+sizeof(ClusterDISK));
		}
		
		
		virtual VError PutInto(VStream& buf)
		{
			VError err;
			
			err=IndexHeaderBTreeRoot::PutInto(buf);
			if (err==VE_OK && fIsCluster) 
				err=buf.PutData(cls.getIHCLUST(),sizeof(ClusterDISK));

			return(err);
		}
		
		
		virtual VError GetFrom(VStream& buf)
		{
			VError err;
			
			err=IndexHeaderBTreeRoot::GetFrom(buf);
			if (fIsCluster)
			{
				if (err==VE_OK) 
					err=buf.GetData(cls.getIHCLUST(),sizeof(ClusterDISK));
				if (err == VE_OK && buf.NeedSwap())
					cls.getIHCLUST()->SwapBytes();
			}

			return(err);
		}
		

		VError PlaceCleScalar(OccupableStack* curstack, FullCleIndex<Type, MaxCles>& Key, sLONG xqui, BaseTaskInfo* context);
		VError DetruireCleScalar(OccupableStack* curstack, FullCleIndex<Type, MaxCles>& Key, sLONG xqui, BaseTaskInfo* context);

		VError ChargeFirstPage(OccupableStack* curstack, BaseTaskInfo* context, uBOOL doiscreer);
		void SetFirstPage(BtreePage<Type, MaxCles>* first, BaseTaskInfo* context);

		Bittab* FourcheScalar(OccupableStack* curstack, const FullCleIndex<Type, MaxCles>& val1, uBOOL xstrict1, const FullCleIndex<Type, MaxCles>& val2, uBOOL xstrict2, BaseTaskInfo* context, VError& err, 
									VDB4DProgressIndicator* InProgress, const VCompareOptions& inOptions, Bittab* dejasel, FullCleIndex<Type, MaxCles>* outVal);

		virtual Selection* SortSel(OccupableStack* curstack, Selection* sel, uBOOL ascent, BaseTaskInfo* context, VError& err, 
										VDB4DProgressIndicator* InProgress = nil, Boolean TestUnicite = false, Bittab* inWafSelbits = nil, WafSelection* outWafSel = nil);
		
		virtual VError GetDistinctValues(OccupableStack* curstack, DB4DCollectionManager& outCollection, BaseTaskInfo* context, Bittab* filtre, 
										VDB4DProgressIndicator* InProgress, VCompareOptions& inOptions);

		VError QuickGetDistinctValues(OccupableStack* curstack, QuickDB4DArrayOfValues *outCollection, BaseTaskInfo* context, Bittab* filtre, 
												VDB4DProgressIndicator* InProgress, VCompareOptions& inOptions);

		virtual Bittab* FindKeyInArray(OccupableStack* curstack, DB4DArrayOfValues* values, BaseTaskInfo* context, VError& err, VDB4DProgressIndicator* InProgress, const VCompareOptions& inOptions, Bittab* dejasel=nil);

		virtual VError FindKeyAndBuildPath(OccupableStack* curstack, BTitemIndex* val, const VCompareOptions& inOptions, BaseTaskInfo* context, VDB4DIndexKey* outKey)
		{
			// ## a faire
			return VE_DB4D_NOTIMPLEMENTED;
		}
		virtual VError NextKey(OccupableStack* curstack, const VDB4DIndexKey* inKey, BaseTaskInfo* context, VDB4DIndexKey* outKey)
		{
			// ## a faire
			return VE_DB4D_NOTIMPLEMENTED;
		}
		
		virtual VError ScanIndex(OccupableStack* curstack, Selection* into, sLONG inMaxRecords, Boolean KeepSorted, Boolean Ascent, BaseTaskInfo* context, Bittab* filtre,sLONG &nbtrouves, VDB4DProgressIndicator* InProgress);

		virtual VError CalculateColumnFormulas(OccupableStack* curstack, ColumnFormulas* formules, BaseTaskInfo* context, Bittab* filtre, VDB4DProgressIndicator* InProgress);

		virtual VError CalculateMin(OccupableStack* curstack, BaseTaskInfo* context, Bittab* filtre, VDB4DProgressIndicator* InProgress, VValueSingle* &result)
		{
			// pour l'instant ne devrait jamais passer la
			xbox_assert(false);
			return VE_DB4D_NOTIMPLEMENTED;
		}

		virtual VError CalculateMax(OccupableStack* curstack, BaseTaskInfo* context, Bittab* filtre, VDB4DProgressIndicator* InProgress, VValueSingle* &result)
		{
			// pour l'instant ne devrait jamais passer la
			xbox_assert(false);
			return VE_DB4D_NOTIMPLEMENTED;
		}
		
		inline Boolean IsCluster() const { return fIsCluster; };

		inline BtreePage<Type, MaxCles>* _GetFirstPage() const { return firstpage; };

		virtual void Update(Table* inTable);


		
	protected:
		BtreePage<Type, MaxCles> *firstpage;
		ClusterSel cls;
		Boolean fIsCluster;
};

				
			
				/* ------------------------------------------------------------ */

VError EnlargeCollection(const VValueSingle& val, DB4DCollectionManager& outCollection, distinctvalue_iterator& xx);


template <class Type>
inline VError AddElemIntoCollection(Type data, DB4DCollectionManager& outCollection, sLONG RealType, distinctvalue_iterator& xx)
{
#if __GNUC__
#else
	tagada++; // should generate a compiler error (only use specialization for this method)
#endif
	return VE_UNIMPLEMENTED;
}

template<>
inline VError AddElemIntoCollection(sLONG data, DB4DCollectionManager& outCollection, sLONG RealType, distinctvalue_iterator& xx)
{
	VLong xdata(data);
	//return outCollection.AddOneElement(1, xdata);
	return EnlargeCollection(xdata, outCollection, xx);
}


template<>
inline VError AddElemIntoCollection(VUUIDBuffer data, DB4DCollectionManager& outCollection, sLONG RealType, distinctvalue_iterator& xx)
{
	VUUID xdata(data);
	VString s;
	xdata.GetString(s);
	//return outCollection.AddOneElement(1, xdata);
	return EnlargeCollection(s, outCollection, xx);
}


template<>
inline VError AddElemIntoCollection(sWORD data, DB4DCollectionManager& outCollection, sLONG RealType, distinctvalue_iterator& xx)
{
	VWord xdata(data);
	//return outCollection.AddOneElement(1, xdata);
	return EnlargeCollection(xdata, outCollection, xx);
}


template<>
inline VError AddElemIntoCollection(sBYTE data, DB4DCollectionManager& outCollection, sLONG RealType, distinctvalue_iterator& xx)
{
	VByte xdata(data);
	//return outCollection.AddOneElement(1, xdata);
	return EnlargeCollection(xdata, outCollection, xx);
}


template<>
inline VError AddElemIntoCollection(uBYTE data, DB4DCollectionManager& outCollection, sLONG RealType, distinctvalue_iterator& xx)
{
	VBoolean xdata(data);
	//return outCollection.AddOneElement(1, xdata);
	return EnlargeCollection(xdata, outCollection, xx);
}


template<>
inline VError AddElemIntoCollection(uLONG8 data, DB4DCollectionManager& outCollection, sLONG RealType, distinctvalue_iterator& xx)
{
	VTime xdata(data);
	//return outCollection.AddOneElement(1, xdata);
	return EnlargeCollection(xdata, outCollection, xx);
}


template<>
inline VError AddElemIntoCollection(sLONG8 data, DB4DCollectionManager& outCollection, sLONG RealType, distinctvalue_iterator& xx)
{
	switch(RealType)
	{
		case VK_DURATION:
		{
			VDuration xdata(data);
			//return outCollection.AddOneElement(1, xdata);
			return EnlargeCollection(xdata, outCollection, xx);
			break;
		}
		default:
		{
			VLong8 xdata(data);
			//return outCollection.AddOneElement(1, xdata);
			return EnlargeCollection(xdata, outCollection, xx);
			break;
		}
	}
}


template<>
inline VError AddElemIntoCollection(Real data, DB4DCollectionManager& outCollection, sLONG RealType, distinctvalue_iterator& xx)
{
	VReal xdata(data);
	//return outCollection.AddOneElement(1, xdata);
	return EnlargeCollection(xdata, outCollection, xx);
}





					/* ------------------------------------------------------------ */

template <class Type>
inline VValueSingle* CreateVValueWithType(Type data, sLONG RealType)
{
#if __GNUC__
#else
	tagada++; // should generate a compiler error (only use specialization for this method)
#endif
	return NULL;
}


template <>
inline VValueSingle* CreateVValueWithType(sWORD data, sLONG RealType)
{
	return new VWord(data);
}


template <>
inline VValueSingle* CreateVValueWithType(sLONG data, sLONG RealType)
{
	return new VLong(data);
}


template <>
inline VValueSingle* CreateVValueWithType(Real data, sLONG RealType)
{
	return new VReal(data);
}


template <>
inline VValueSingle* CreateVValueWithType(uLONG8 data, sLONG RealType)
{
	return new VTime(data);
}


template <>
inline VValueSingle* CreateVValueWithType(sLONG8 data, sLONG RealType)
{
	switch(RealType)
	{
		case VK_TIME:
		{
			return new VTime(data);
			break;
		}
		case VK_DURATION:
		{
			return new VDuration(data);
			break;
		}
		default:
		{
			return new VLong8(data);
			break;
		}
	}
}


template <>
inline VValueSingle* CreateVValueWithType(uBYTE data, sLONG RealType)
{
	return new VBoolean(data);
}


template <>
inline VValueSingle* CreateVValueWithType(sBYTE data, sLONG RealType)
{

	return new VByte(data);
}


template <>
inline VValueSingle* CreateVValueWithType(VUUIDBuffer data, sLONG RealType)
{
	
	return new VUUID(data);
}


					/* ------------------------------------------------------------ */



template <class Type, sLONG MaxCles>
class IndexInfoScalar : public IndexInfoFromField
{
	public:
		typedef multiset<TransCleIndex<Type> > mapKeys;
		typedef set<TransUniqCleIndex<Type> > mapUniqKeys;

		inline IndexInfoScalar(Base4D* db, sLONG xnumfile, sLONG xnumfield, sLONG xtypindex, Boolean UniqueKeys):
					IndexInfoFromField(db, xnumfile, xnumfield, xtypindex, UniqueKeys, true) { InfoTyp = BtreePageDataHeader<Type>::IndexInfo_Type; };
					
		inline IndexInfoScalar():IndexInfoFromField() { InfoTyp = BtreePageDataHeader<Type>::IndexInfo_Type; };

		inline const IndexHeaderBTreeScalar<Type, MaxCles>* GetHeaderScalar() const { return (IndexHeaderBTreeScalar<Type, MaxCles>*)header; };
		inline IndexHeaderBTreeScalar<Type, MaxCles>* GetHeaderScalar() { return (IndexHeaderBTreeScalar<Type, MaxCles>*)header; };

		//inline const IndexHeaderBTreeScalar<Type, MaxCles>* GetHeaderScalar() const { return fHeader };
		//inline IndexHeaderBTreeScalar<Type, MaxCles>* GetHeaderScalar() { return fHeader; };

		virtual sLONG GetTemplateType() const
		{
			return BtreePageDataHeader<Type>::IndexInfo_Type;
		}

		virtual Boolean MatchOtherDataKind(IndexInfo* ind) const
		{
			if (ind->IsScalar())
			{
				return GetTemplateType() == ind->GetTemplateType();
			}
			else
				return false;
		};

		virtual sLONG GetScalarKind() const { return fDataKind; };

		virtual VError PlaceCle(OccupableStack* curstack, FicheInMem *rec, BaseTaskInfo* context);
		virtual VError DetruireCle(OccupableStack* curstack, FicheInMem *rec, BaseTaskInfo* context);

		virtual bool PlaceGlobalUniqKey(OccupableStack* curstack, FicheInMem *rec, BaseTaskInfo* context, bool temporary);
		virtual bool IsGlobalUniqKey(OccupableStack* curstack, FicheInMem *rec, BaseTaskInfo* context);
		virtual void ReleaseGlobalUniqKey(OccupableStack* curstack, FicheInMem *rec, BaseTaskInfo* context, bool tempOnly);
		virtual void ReleaseGlobalUniqKey(const BTitemIndex* val, Transaction* trans);

		void BuildFullCle(FicheInMem *rec, FullCleIndex<Type, MaxCles>& outKey, VError &err, Boolean OldOne);
		

		inline sLONG CompareKeysScalar(const FullCleIndex<Type, MaxCles>& val, const TransCleIndex<Type>& val2, const VCompareOptions& inOptions)
		{
			if (val2.cle == val.cle)
				return CR_EQUAL;
			else
				return val.cle < val2.cle ? CR_SMALLER : CR_BIGGER;
		}

		inline sLONG CompareKeysScalar(const TransCleIndex<Type>& val, const FullCleIndex<Type, MaxCles>& val2, const VCompareOptions& inOptions)
		{
			if (val2.cle == val.cle)
				return CR_EQUAL;
			else
				return val.cle < val2.cle ? CR_SMALLER : CR_BIGGER;
		}

		virtual void* NewMapKeys() const
		{
			return new mapKeys();
		}
		
		virtual void DeleteMapKeys(void* mapkey) const
		{
			delete (mapKeys*)mapkey;
		}

		virtual void CopyMapKeys(const void* source, void* dest) const
		{
			*((mapKeys*)dest) = *((const mapKeys*)source);
		}
		
		
		virtual VError PlaceCleAllForTrans(void* xvals, BaseTaskInfo* context);
		
		virtual VError DetruireAllCleForTrans(void* xvals, BaseTaskInfo* context);

		virtual Bittab* FindKeyInArray(DB4DArrayOfValues* values, BaseTaskInfo* context, VError& err, VDB4DProgressIndicator* InProgress, const VCompareOptions& inOptions, Bittab* dejasel=nil);

		virtual Bittab* Fourche(BTitemIndex* val1, uBOOL xstrict1, BTitemIndex* val2, uBOOL xstrict2, BaseTaskInfo* context, VError& err, 
										VDB4DProgressIndicator* InProgress, const VCompareOptions& inOptions, Bittab* dejasel=nil, BTitemIndex** outVal = nil);

		virtual sLONG FindKey(BTitemIndex* val, BaseTaskInfo* context, const VCompareOptions& inOptions, Bittab* dejasel, BTitemIndex** outVal); 

		virtual Boolean GenereQuickIndex(VError &err, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, sLONG MaxElemsInPage, sLONG RequiredElemsInPage);

		virtual VError GetDistinctValues(DB4DCollectionManager& outCollection, BaseTaskInfo* context, Bittab* filtre, 
											VDB4DProgressIndicator* InProgress, const VCompareOptions& inOptions);

		virtual VError QuickGetDistinctValues(QuickDB4DArrayOfValues *outCollection, BaseTaskInfo* context, Bittab* filtre, 
											VDB4DProgressIndicator* InProgress, const VCompareOptions& inOptions);

		virtual void CreateHeader()
		{
			if (header == nil)
			{
				header = new IndexHeaderBTreeScalar<Type, MaxCles>(this, typindex == DB4D_Index_BtreeWithCluster);
			}
		}

		virtual VError GenereIndex(BaseTaskInfo* context, VDB4DProgressIndicator* InProgress);

		virtual VError FindKeyAndBuildPath(BTitemIndex* val, const VCompareOptions& inOptions, BaseTaskInfo* context, VDB4DIndexKey* outKey)
		{
			// ## a faire
			return VE_DB4D_NOTIMPLEMENTED;
		}

		virtual VError NextKey(const VDB4DIndexKey* inKey, BaseTaskInfo* context, VDB4DIndexKey* outKey)
		{
			// ## a faire
			return VE_DB4D_NOTIMPLEMENTED;
		}

		virtual Selection* ScanIndex(sLONG inMaxRecords, Boolean KeepSorted, Boolean Ascent, BaseTaskInfo* context, VError& err, Selection* filtre, VDB4DProgressIndicator* InProgress);

		virtual VError CalculateColumnFormulas(ColumnFormulas* formules, BaseTaskInfo* context, Bittab* filtre, VDB4DProgressIndicator* InProgress);

		virtual VError CalculateMin(BaseTaskInfo* context, Bittab* filtre, VDB4DProgressIndicator* InProgress, VValueSingle* &result);

		virtual VError CalculateMax(BaseTaskInfo* context, Bittab* filtre, VDB4DProgressIndicator* InProgress, VValueSingle* &result);

		VError RemoveDeleteKeysFromFourche(Transaction* trans, const FullCleIndex<Type, MaxCles>& val1, uBOOL xstrict1, const FullCleIndex<Type, MaxCles>& val2, 
											uBOOL xstrict2, const VCompareOptions& inOptions, Bittab* dejasel);

		VError TransFourche(Transaction* trans, const FullCleIndex<Type, MaxCles>& val1, uBOOL xstrict1, const FullCleIndex<Type, MaxCles>& val2,  
							uBOOL xstrict2, const VCompareOptions& inOptions, Bittab* dejasel, Bittab* into, FullCleIndex<Type, MaxCles>* outVal);

		VError TransPlaceCle(Transaction* trans, FullCleIndex<Type, MaxCles>& val, sLONG numrec);
		VError TransDetruireCle(Transaction* trans, FullCleIndex<Type, MaxCles>& val, sLONG numrec);

		virtual VError JoinWithOtherIndex(IndexInfo* other, Bittab* filtre1, Bittab* filtre2,ComplexSelection* result, BaseTaskInfo* context, 
											VCompareOptions& inOptions, VProgressIndicator* inProgress = nil, bool leftjoin = nil, bool rightjoin = nil);

		virtual VError JoinWithOtherIndexNotNull(IndexInfo* other, Bittab* filtre1, Bittab* filtre2, Bittab* result, BaseTaskInfo* context, VCompareOptions& inOptions, VProgressIndicator* inProgress = nil);

#if debugindex_strong
		Boolean debug_mustcheck;
#endif
	
	protected:
		mapUniqKeys fUniqueKeysToBeProcessed;

		//IndexHeaderBTreeScalar<Type, MaxCles>* fHeader;
};


template <>
inline sLONG IndexInfoScalar<Real, kNbKeysForScalar>::CompareKeysScalar(const FullCleIndex<Real, kNbKeysForScalar>& val, const TransCleIndex<Real>& val2, const VCompareOptions& inOptions)
{
	if (fabs(val2.cle - val.cle) < inOptions.GetEpsilon())
		return CR_EQUAL;
	else
		return val.cle < val2.cle ? CR_SMALLER : CR_BIGGER;
}


template <>
inline sLONG IndexInfoScalar<Real, kNbKeysForScalar>::CompareKeysScalar(const TransCleIndex<Real>& val, const FullCleIndex<Real, kNbKeysForScalar>& val2, const VCompareOptions& inOptions)
{
	if (fabs(val2.cle - val.cle) < inOptions.GetEpsilon())
		return CR_EQUAL;
	else
		return val.cle < val2.cle ? CR_SMALLER : CR_BIGGER;
}




									/* -----------------------------------------------  */
									
									
template <class Type, sLONG MaxCles>
class IndexIteratorPathItem
{
	public:
		BtreePage<Type, MaxCles>* fPage;
		sLONG fPos;

};



template <class Type, sLONG MaxCles>
class IndexIterator
{
	public:
		IndexIterator(IndexInfoScalar<Type, MaxCles>* ind, Bittab* filter, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, const VString& inMess, OccupableStack* curstack);
		virtual ~IndexIterator();

		Boolean FirstKey(BaseTaskInfo* context, VError& err, sLONG* CurElemToProgress = nil);
		Boolean NextKey(BaseTaskInfo* context, VError& err, sLONG* CurElemToProgress = nil);

		Boolean LastKey(BaseTaskInfo* context, VError& err, sLONG* CurElemToProgress = nil) { return false; /* a faire */ };
		Boolean PreviousKey(BaseTaskInfo* context, VError& err, sLONG* CurElemToProgress = nil)  { return false; /* a faire */ };

		sLONG GetRecNum() const;
		const Type* GetKey() const;


		inline sLONG CompareKeysStrictWithRecNum(const CleIndex<Type>& val, const TransCleIndex<Type>& val2)
		{
			if (val2.cle == val.cle)
			{
				if (val.qui == val2.qui)
					return CR_EQUAL;
				else
					return val.qui < val2.qui ? CR_SMALLER : CR_BIGGER;
			}
			else
				return val.cle < val2.cle ? CR_SMALLER : CR_BIGGER;
		}


		inline sLONG CompareKeysStrictWithRecNum(const TransCleIndex<Type>& val, const CleIndex<Type>& val2)
		{
			if (val2.cle == val.cle)
			{
				if (val.qui == val2.qui)
					return CR_EQUAL;
				else
					return val.qui < val2.qui ? CR_SMALLER : CR_BIGGER;
			}
			else
				return val.cle < val2.cle ? CR_SMALLER : CR_BIGGER;
		}


	protected:


		inline void _DisposeSel()
		{
			if (fSel != nil)
			{
				fSel->Release();
				fSel = nil;
				fSelIter.Reset(nil);
			}
		}


		inline void _RetainKey()
		{
			if (fCleAddEnTrans)
			{
				if (fCurSavedKey != fEndSavedKey)
				{
					fKeyInTrans = &(*fCurSavedKey);
				}
				else
					fKeyInTrans = nil;
			}
			else
				fKeyInTrans = nil;
		}


		inline void _DisposeKey()
		{
			fKey = nil;
		}


		VError _IncCurPos(sLONG* CurElemToProgress = nil);
		void _DisposePages();

		VError _SetSel(Selection* sel);


		void _PosToFirstLeft(BtreePage<Type, MaxCles>* sousBT, BaseTaskInfo* context, VError& err);

		void _MatchAddWithCurKey();
		Boolean _MatchDelWithCurKey();


		IndexInfoScalar<Type, MaxCles> *fInd;
		CleIndex<Type> *fKey;
		CleIndex<Type> fAuxKeyForCluster;
		IndexIteratorPathItem<Type, MaxCles> fPagePath[kMaxPageLevelInKeyIterator];
		sLONG fNbLevel;
		Selection* fSel;
		SelectionIterator fSelIter;
		sLONG fRecNum;
		Boolean fWasJustReset, fCleAddEnTrans, fCleDelEnTrans;
		Boolean fCurKeyInTrans;
		Bittab* fFilter;
		BaseTaskInfo* fContext;
		ProgressEncapsuleur fProgress;
		sLONG fCurProgressPos;
		OccupableStack* fCurStack;

		typename IndexInfoScalar<Type, MaxCles>::mapKeys* fSavedKeys;
		typename IndexInfoScalar<Type, MaxCles>::mapKeys* fDeletedKeys;
		typename IndexInfoScalar<Type, MaxCles>::mapKeys::iterator fCurSavedKey, fCurDeletedKey;
		typename IndexInfoScalar<Type, MaxCles>::mapKeys::iterator fEndSavedKey, fEndDeletedKey;

		const TransCleIndex<Type> *fKeyInTrans;


};
									


			/*======================================================================================*/
			/*======================================================================================*/
			/*======================================================================================*/
			/*======================================================================================*/
			/*======================================================================================*/


#if VERSIONWIN
#pragma warning( pop)
#endif

#endif
