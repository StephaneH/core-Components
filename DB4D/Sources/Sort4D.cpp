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



VError SortLine::PutInto(VStream* into)
{
	VError err = into->PutLong(numfield);
	if (err == VE_OK)
		err = into->PutLong(numfile);
	if (err == VE_OK)
		err = into->PutByte(ascendant);
	if (err == VE_OK)
		err = into->PutByte(isfield);
	if (err == VE_OK && !isfield)
	{
		xbox_assert(expression != nil);
		err = expression->PutInto(*into);
	}
	return err;
}


VError SortLine::GetFrom(VStream* from)
{
	VError err = from->GetLong(numfield);
	if (err == VE_OK)
		err = from->GetLong(numfile);
	if (err == VE_OK)
		err = from->GetByte(ascendant);
	if (err == VE_OK)
		err = from->GetByte(isfield);
	if (err == VE_OK && !isfield)
	{
		expression = (*(VDBMgr::GetBuildLanguageExpressionMethod()))(*from, err);
	}
	return err;
}



SortTab::~SortTab()
{
	for (SortLineArray::iterator cur = li.begin(), end = li.end(); cur != end; cur++)
	{
		if (cur->expression != nil)
			cur->expression->Release();
		if (cur->fAttPath != nil)
			QuickReleaseRefCountable(cur->fAttPath);
	}
}


VError SortTab::AddTriLineField(Field* cri, uBOOL ascendant)
{
	xbox_assert(cri != nil);
	return AddTriLineField(cri->GetOwner()->GetNum(), cri->GetPosInRec(), ascendant);
}


VError SortTab::AddTriLineField(sLONG numfile, sLONG numfield, uBOOL ascendant)
{
	SortLine xli;
	VError err = VE_OK;
	Table* table = fBD->RetainTable(numfile);
	if (table == nil)
		err = fBD->ThrowError(VE_DB4D_WRONGTABLEREF, dbaction_BuildingSortCriterias);
	else
	{
		Field* cri = table->RetainField(numfield);
		if (cri == nil)
			err = fBD->ThrowError(VE_DB4D_WRONGFIELDREF, dbaction_BuildingSortCriterias);
		else
		{
			xli.isfield = true;
			xli.numfile = numfile;
			xli.numfield = numfield;
			xli.ascendant = ascendant;
			li.push_back(xli);
			cri->Release();
		}
		table->Release();
	}
		
	return err;
}


Boolean SortTab::AddExpression(DB4DLanguageExpression* inExpression, Boolean ascendant)
{
	xbox_assert(inExpression != nil);
	SortLine xli;

	xli.expression = inExpression;
	xli.isfield = false;
	xli.ascendant = ascendant;
	li.push_back(xli);
	inExpression->Retain();

	return true;
}


Boolean SortTab::AddAttribute(EntityAttribute* inAtt, Boolean ascendant)
{
	SortLine xli;

	xli.att = inAtt;
	xli.isfield = false;
	xli.ascendant = ascendant;
	li.push_back(xli);

	return true;
}


Boolean SortTab::AddAttributePath(AttributePath* inAttPath, Boolean ascendant)
{
	SortLine xli;

	xli.fAttPath = RetainRefCountable(inAttPath);
	xli.isfield = false;
	xli.ascendant = ascendant;
	li.push_back(xli);

	return true;
}



VError SortTab::PutInto(VStream* into)
{
	VError err = VE_OK;
	sLONG nb = (sLONG)li.size();

	err = into->PutLong(nb);
	for (SortLineArray::iterator cur = li.begin(), end = li.end(); cur != end && err == VE_OK; cur++)
	{
		err = cur->PutInto(into);
	}
	return err;
}


VError SortTab::GetFrom(VStream* from)
{
	sLONG nb;

	VError err = from->GetLong(nb);
	if (err == VE_OK)
	{
		for (sLONG i = 0; i < nb && err == VE_OK; i++)
		{
			SortLine xli;
			err = xli.GetFrom(from);
			if (err == VE_OK)
			{
				li.push_back(xli);
			}
		}
	}

	return err;
}



static void FreeXString(TypeSortElemArray<xString> &tempsort, sLONG lenmax)
{
	TypeSortElem<xString>* p = (TypeSortElem<xString>*)tempsort.GetDataPtr();
	sLONG nb = tempsort.GetCount();

	if (p != nil)
	{
		sLONG i;
		for (i = 0; i < nb; i++)
		{
			p->value.Free();
			p = TypeSortElemArray<xString>::NextDataElem(p, lenmax);
		}
	}

}



static void FreeXStringUTF8(TypeSortElemArray<xStringUTF8> &tempsort, sLONG lenmax)
{
	TypeSortElem<xStringUTF8>* p = (TypeSortElem<xStringUTF8>*)tempsort.GetDataPtr();
	sLONG nb = tempsort.GetCount();

	if (p != nil)
	{
		sLONG i;
		for (i = 0; i < nb; i++)
		{
			p->value.Free();
			p = TypeSortElemArray<xStringUTF8>::NextDataElem(p, lenmax);
		}
	}

}

static void FreeXMulti(TypeSortElemArray<xMultiFieldData> &tempsort, sLONG lenmax, const xMultiFieldDataOffsets& off)
{
	Boolean OKLoop = false;

	xMultiFieldDataOffset_constiterator cur = off.Begin(), end = off.End();
	for (;cur != end; cur++)
	{
		if (cur->GetDataType() == VK_STRING || cur->GetDataType() == VK_STRING_UTF8) 
			OKLoop = true;
	}

	if (OKLoop)
	{
		TypeSortElem<xMultiFieldData>* p = (TypeSortElem<xMultiFieldData>*)tempsort.GetDataPtr();
		sLONG nb = tempsort.GetCount();
		if (p != nil)
		{
			sLONG i;
			for (i = 0; i < nb; i++)
			{
				p->value.Free();
				p = TypeSortElemArray<xMultiFieldData>::NextDataElem(p, lenmax);
			}
		}
	}

}



												/*  --------------------------------------------------------- */


template <class Type>
void* TypeSortElemOffsetArray<Type>::AddOffSet(sLONG numrec, sLONG lendata)
{
	if (CalcFreeSpace() >= (sLONG)(sizeof(TypeSortElemOffset<Type>) + lendata))
	{
		TypeSortElemOffset<Type> *p = &fOffsets[fNbElem];
		p->SetNumRec(numrec);
		p->SetOffSet(GetLastOffset()-lendata, lendata);
		fNbElem++;
		void* res = p->GetAlawaysValidDataPtr((char*)this);
		/*
		if (lendata == 0)
			p->SetDataNull();
			*/
		return res;
	}
	else
		return nil;
}


template <class Type>
VError TypeSortElemOffsetArray<Type>::WriteToStream(VStream* out)
{
	sLONG i;
	TypeSortElemOffset<Type>* off = &fOffsets[0];
	out->PutLong(fNbElem);
	for (i = 0; i < fNbElem; i++, off++)
	{
		sLONG numrec = off->GetNumRec();
		void* p = off->GetDataPtr((char*)this);
		VError err = out->PutLong(numrec);
		if (err != VE_OK)
			return err;
		if (p != nil)
		{
			sLONG len = off->GetLen();

			err = out->PutLong(len);
			if (err != VE_OK)
				return err;
			err = out->PutData(p, len);
		}
		else
			err = out->PutLong(0);

		if (err != VE_OK)
			return err;
	}

	return VE_OK;
}

class PartSortPredicateHolder
{
	public:

		inline PartSortPredicateHolder(const xMultiFieldDataOffsets& criterias, char* base, Boolean TestUniq, Boolean PourIndex, const VCompareOptions& inOptions):fCriterias(criterias) , fOptions(inOptions)
		{
			fBase = base; 
			fTestUniq = TestUniq; 
			fWasNotUniq = false; 
			fPourIndex = PourIndex;
		}

		inline Boolean WasNotUnique() const { return fWasNotUniq; };

		const xMultiFieldDataOffsets& fCriterias;
		char* fBase;
		Boolean fTestUniq, fWasNotUniq, fPourIndex;
		VCompareOptions fOptions;


};


template <class Type>
class PartSortPredicate
{
	public:
		inline PartSortPredicate(PartSortPredicateHolder *holder)
		{ 
			fHolder = holder;
		};


		bool operator ()(const TypeSortElemOffset<Type>& val1, const TypeSortElemOffset<Type>& val2)
		{
			CompareResult result = CR_EQUAL;

			bool cont = false;
			bool wasdiac = fHolder->fOptions.IsDiacritical();
			bool isdiac = wasdiac;

			do 
			{
				if (cont) // c'est le deuxieme passage et il faut comparer normalement
				{
					cont = false;
					isdiac = wasdiac;
				}
				else
				{
					// c'est le premier passage et il faut forcer en non diacritique si c'est une cle composite
					if (fHolder->fOptions.IsDiacritical() && fHolder->fCriterias.IsMulti())
					{
						cont = true;
						isdiac = false;
					}
				}

				char* p1 = val1.GetDataPtr(fHolder->fBase);
				char* p2 = val2.GetDataPtr(fHolder->fBase);

				if (p1 == nil)
				{
					if (p2 == nil)
						return false;
					else
						return true;
				}
				else
				{
					if (p2 == nil)
						return false;
				}

				sLONG len1,len2;

				xMultiFieldDataOffset_constiterator cur = fHolder->fCriterias.Begin(), end = fHolder->fCriterias.End();
				for (; cur != end; cur++)
				{
					switch (cur->GetDataType())
					{
						case VK_LONG:
							if (*(sLONG*)p1 == *(sLONG*)p2)
								result = CR_EQUAL;
							else
							{
								if (*(sLONG*)p1 < *(sLONG*)p2)
									result = CR_SMALLER;
								else
									result = CR_BIGGER;
							}
							p1 = p1 + sizeof(sLONG);
							p2 = p2 + sizeof(sLONG);
							break;

						case VK_LONG8:
						case VK_DURATION:
						case VK_SUBTABLE:
						case VK_SUBTABLE_KEY:
							if (*(sLONG8*)p1 == *(sLONG8*)p2)
								result = CR_EQUAL;
							else
							{
								if (*(sLONG8*)p1 < *(sLONG8*)p2)
									result = CR_SMALLER;
								else
									result = CR_BIGGER;
							}
							p1 = p1 + sizeof(sLONG8);
							p2 = p2 + sizeof(sLONG8);
							break;

						case VK_REAL:
							result = VReal::sInfo.CompareTwoPtrToDataWithOptions(p1, p2, fHolder->fOptions);
							p1 = p1 + sizeof(Real);
							p2 = p2 + sizeof(Real);
							break;

						case VK_WORD:
							if (*(sWORD*)p1 == *(sWORD*)p2)
								result = CR_EQUAL;
							else
							{
								if (*(sWORD*)p1 < *(sWORD*)p2)
									result = CR_SMALLER;
								else
									result = CR_BIGGER;
							}
							p1 = p1 + sizeof(sWORD);
							p2 = p2 + sizeof(sWORD);
							break;

						case VK_BOOLEAN:
							if (*(uCHAR*)p1 == *(uCHAR*)p2)
								result = CR_EQUAL;
							else
							{
								if (*(uCHAR*)p1 < *(uCHAR*)p2)
									result = CR_SMALLER;
								else
									result = CR_BIGGER;
							}
							p1 = p1 + sizeof(uCHAR);
							p2 = p2 + sizeof(uCHAR);
							break;

						case VK_BYTE:
							if (*(sBYTE*)p1 == *(sBYTE*)p2)
								result = CR_EQUAL;
							else
							{
								if (*(sBYTE*)p1 < *(sBYTE*)p2)
									result = CR_SMALLER;
								else
									result = CR_BIGGER;
							}
							p1 = p1 + sizeof(sBYTE);
							p2 = p2 + sizeof(sBYTE);
							break;

						case VK_TIME:
							if (*(xTime*)p1 == *(xTime*)p2)
								result = CR_EQUAL;
							else
							{
								if (*(xTime*)p1 < *(xTime*)p2)
									result = CR_SMALLER;
								else
									result = CR_BIGGER;
							}
							p1 = p1 + sizeof(xTime);
							p2 = p2 + sizeof(xTime);
							break;

						case VK_UUID:
							if (*(VUUIDBuffer*)p1 == *(VUUIDBuffer*)p2)
								result = CR_EQUAL;
							else
							{
								if (*(VUUIDBuffer*)p1 < *(VUUIDBuffer*)p2)
									result = CR_SMALLER;
								else
									result = CR_BIGGER;
							}
							p1 = p1 + sizeof(VUUIDBuffer);
							p2 = p2 + sizeof(VUUIDBuffer);
							break;
							
						case VK_STRING_UTF8:
							fHolder->fOptions.SetDiacritical(isdiac);
							result = VStringUTF8::sInfo.CompareTwoPtrToDataWithOptions(p1, p2, fHolder->fOptions);
							fHolder->fOptions.SetDiacritical(wasdiac);
							p1 = p1 + VStringUTF8::sInfo.GetSizeOfValueDataPtr(p1);
							p2 = p2 + VStringUTF8::sInfo.GetSizeOfValueDataPtr(p2);
							break;

						case VK_STRING:
							fHolder->fOptions.SetDiacritical(isdiac);
							result = VString::sInfo.CompareTwoPtrToDataWithOptions(p1, p2, fHolder->fOptions);
							fHolder->fOptions.SetDiacritical(wasdiac);
							p1 = p1 + VString::sInfo.GetSizeOfValueDataPtr(p1);
							p2 = p2 + VString::sInfo.GetSizeOfValueDataPtr(p2);
							break;
					}

					if (result != CR_EQUAL)
					{
						if (cur->IsAscent())
						{
							if (result == CR_SMALLER)
								return true;
							else
								return false;
						}
						else
						{
							if (result == CR_BIGGER)
								return true;
							else
								return false;
						}

					}

				}
			} while (cont);

			if (fHolder->fTestUniq)
				fHolder->fWasNotUniq = true;

			if (fHolder->fPourIndex)
			{
				if (val1.GetNumRec() < val2.GetNumRec())
					return true;
			}

			return false;

		};


	protected:
		PartSortPredicateHolder* fHolder;

};

template <class Type>
VError TypeSortElemOffsetArray<Type>::Sort(const xMultiFieldDataOffsets& criterias, Boolean TestUniq, Boolean BreakOnNonUnique, 
										   Boolean& outUniq, Boolean PourIndex, const VCompareOptions& inOptions)
{
	VError err = VE_OK;
	PartSortPredicateHolder holder(criterias, (char*)this, TestUniq, PourIndex, inOptions);
	PartSortPredicate<Type> partsort(&holder);
	std::sort(&fOffsets[0], &fOffsets[fNbElem], partsort );
	if (TestUniq && BreakOnNonUnique && holder.WasNotUnique())
		err = VE_DB4D_DUPLICATED_KEY;
	outUniq = !holder.WasNotUnique();
	return err;
}



											/*  ------------------------------------------- */


const sLONG kAuxDataBufferSize = 16384;
//const kMaxTypeElemBuff = 128;

/*
Constructor and Destructor moved to the header file, because of a gcc linking error with optimisation
*/

template <class Type>
void TypeSortElemArray<Type>::SubSort( sLONG l, sLONG r, VDB4DProgressIndicator* InProgress, sLONG TypeSize)
{
	sLONG i;
	sLONG j;
	sLONG xx;

	char pivotbuff[256];
	TypeSortElem<Type>* pivot = (TypeSortElem<Type>*)&pivotbuff;
	TypeSortElem<Type>* temp = (TypeSortElem<Type>*)auxbuff;

	i = l;
	j = r;
	xx = (l + r) / 2;
	*pivot = *GetDataElem(xx, TypeSize);
	do {
		//while ( (i < count) && issuptri(-1, i, &err, tri) )
		while ( (i < count) && (pivot->value > GetDataElem(i, TypeSize)->value) )
		{
			i++;
		}

		//while ( (j >= 0) && issuptri(j, -1, &err, tri) && (err==VE_OK) )
		while ( (j >= 0) && (pivot->value < GetDataElem(j, TypeSize)->value) )
		{
			j--;
		}

		if ( i <= j )
		{
			*temp = *GetDataElem(i, TypeSize);
			*GetDataElem(i, TypeSize) = *GetDataElem(j, TypeSize);
			*GetDataElem(j, TypeSize) = *temp;

			i++;
			j--;
		}
	} while ( i <= j );


	if ( l < j ) SubSort( l, j, InProgress, TypeSize );

	if ( i < r  ) SubSort( i, r, InProgress, TypeSize );
}


typedef vector<VFileStream*> TempFileArray;


template <class Type>
VError TypeSortElemArray<Type>::SubSortWithUnicite( sLONG l, sLONG r, VDB4DProgressIndicator* InProgress, sLONG TypeSize)
{
	sLONG i;
	sLONG j;
	sLONG xx;
	VError err = VE_OK;

	char pivotbuff[256];
	TypeSortElem<Type>* pivot = (TypeSortElem<Type>*)&pivotbuff;
	TypeSortElem<Type>* temp = (TypeSortElem<Type>*)auxbuff;

	i = l;
	j = r;
	xx = (l + r) / 2;
	*pivot = *GetDataElem(xx, TypeSize);
	do {
		//while ( (i < count) && issuptri(-1, i, &err, tri) )
		while ( (i < count) && (pivot->value > GetDataElem(i, TypeSize)->value) )
		{
			i++;
		}
		if (pivot->value == GetDataElem(i, TypeSize)->value && pivot->recnum != GetDataElem(i, TypeSize)->recnum)
		{
			err = VE_DB4D_DUPLICATED_KEY;
			break;
		}

		//while ( (j >= 0) && issuptri(j, -1, &err, tri) && (err==VE_OK) )
		while ( (j >= 0) && (pivot->value < GetDataElem(j, TypeSize)->value) )
		{
			j--;
		}
		if (pivot->value == GetDataElem(j, TypeSize)->value && pivot->recnum != GetDataElem(i, TypeSize)->recnum)
		{
			err = VE_DB4D_DUPLICATED_KEY;
			break;
		}

		if ( i <= j )
		{
			*temp = *GetDataElem(i, TypeSize);
			*GetDataElem(i, TypeSize) = *GetDataElem(j, TypeSize);
			*GetDataElem(j, TypeSize) = *temp;

			i++;
			j--;
		}
	} while ( i <= j );

	if (err == VE_OK)
	{
		if ( l < j ) err = SubSortWithUnicite( l, j, InProgress, TypeSize );

		if ( i < r  && err == VE_OK) err = SubSortWithUnicite( i, r, InProgress, TypeSize );
	}

	return err;
}


class SortPredicateHolder
{
	public:
		inline SortPredicateHolder(const VCompareOptions& inOptions, const xMultiFieldDataOffsets& criterias, Boolean TestUnicite)
		{
			fCriterias = &criterias;
			fTestUnicite = TestUnicite;
			fNonUnique = false;
			fOptions = inOptions;
		}

		const xMultiFieldDataOffsets* fCriterias;
		VCompareOptions fOptions;
		Boolean fTestUnicite;
		mutable Boolean fNonUnique;
};

template <class Type>
class SortPredicate
{
	public:
		inline SortPredicate(SortPredicateHolder *holder)
		{
			fHolder = holder;
		}

		bool operator ()(const TypeSortElem<Type>& val1, const TypeSortElem<Type>& val2) const;

		SortPredicateHolder *fHolder;
};

template<class Type>
bool SortPredicate<Type>::operator ()(const TypeSortElem<Type>& val1, const TypeSortElem<Type>& val2) const
{
	if (val1.value == val2.value)
	{
		if (fHolder->fTestUnicite)
		{
			if (val1.recnum != val2.recnum)
			{
				fHolder->fNonUnique = true;
			}
		}
		return val1.recnum < val2.recnum;
	}
	return val1.value < val2.value;
}

template<>
bool SortPredicate<xString>::operator ()(const TypeSortElem<xString>& val1, const TypeSortElem<xString>& val2) const
{
	CompareResult compres = val1.value.CompareTo(val2.value, fHolder->fOptions);

	if (compres == CR_EQUAL)
	{
		if (fHolder->fTestUnicite)
		{
			if (val1.recnum != val2.recnum)
			{
				fHolder->fNonUnique = true;
			}
		}
		return val1.recnum < val2.recnum;
	}
	else
		return compres == CR_SMALLER;
}

template<>
bool SortPredicate<xStringUTF8>::operator ()(const TypeSortElem<xStringUTF8>& val1, const TypeSortElem<xStringUTF8>& val2) const
{
	CompareResult compres = val1.value.CompareTo(val2.value, fHolder->fOptions);

	if (compres == CR_EQUAL)
	{
		if (fHolder->fTestUnicite)
		{
			if (val1.recnum != val2.recnum)
			{
				fHolder->fNonUnique = true;
			}
		}
		return val1.recnum < val2.recnum;
	}
	else
		return compres == CR_SMALLER;
}


template<>
bool SortPredicate<xMultiFieldData>::operator ()(const TypeSortElem<xMultiFieldData>& val1, const TypeSortElem<xMultiFieldData>& val2) const
{
	CompareResult compres = val1.value.CompareTo(val2.value, fHolder->fOptions, val1.recnum, val2.recnum);

	if (compres == CR_EQUAL)
	{
		if (fHolder->fTestUnicite)
		{
			if (val1.recnum != val2.recnum)
			{
				fHolder->fNonUnique = true;
			}
		}
		return val1.recnum < val2.recnum;
	}
	else
		return compres == CR_SMALLER;
}


template<>
bool SortPredicate<Real>::operator ()(const TypeSortElem<Real>& val1, const TypeSortElem<Real>& val2) const
{
	CompareResult compres = VReal::sInfo.CompareTwoPtrToDataWithOptions(&val1.value, &val2.value, fHolder->fOptions);

	if (compres == CR_EQUAL)
	{
		if (fHolder->fTestUnicite)
		{
			if (val1.recnum != val2.recnum)
			{
				fHolder->fNonUnique = true;
			}
		}
		return val1.recnum < val2.recnum;
	}
	else
		return compres == CR_SMALLER;
}

 
template <class Type>
class SortIndirectionPredicate
{
public:
	inline SortIndirectionPredicate(SortPredicate<Type> *sortpred)
	{
		fSortPredicate = sortpred;
	}

	bool operator ()(const TypeSortElem<Type>* val1, const TypeSortElem<Type>* val2) const
	{
		return (*fSortPredicate)(*val1, *val2);
	}

	SortPredicate<Type>* fSortPredicate;
};


CompareResult Compare2SortElems(void* xp1, void* xp2, const xMultiFieldDataOffsets& criterias, const VCompareOptions& inOptions)
{
	CompareResult result = CR_EQUAL;

	if (inOptions.IsDiacritical() && criterias.IsMulti())
	{
		VCompareOptions nondiac = inOptions;
		nondiac.SetDiacritical(false);

		if (result != CR_EQUAL)
		{
			return result;
		}
		// on continue s'il y a egalite sur le non diacritique
	}

	char* p1 = (char*)xp1;
	char* p2 = (char*)xp2;

	xMultiFieldDataOffset_constiterator cur = criterias.Begin(), end = criterias.End();
	for (; cur != end; cur++)
	{
		switch (cur->GetDataType())
		{
		case VK_LONG:
			if (*(sLONG*)p1 == *(sLONG*)p2)
				result = CR_EQUAL;
			else
			{
				if (*(sLONG*)p1 < *(sLONG*)p2)
					result = CR_SMALLER;
				else
					result = CR_BIGGER;
			}
			p1 = p1 + sizeof(sLONG);
			p2 = p2 + sizeof(sLONG);
			break;

		case VK_LONG8:
		case VK_DURATION:
		case VK_SUBTABLE:
		case VK_SUBTABLE_KEY:
			if (*(sLONG8*)p1 == *(sLONG8*)p2)
				result = CR_EQUAL;
			else
			{
				if (*(sLONG8*)p1 < *(sLONG8*)p2)
					result = CR_SMALLER;
				else
					result = CR_BIGGER;
			}
			p1 = p1 + sizeof(sLONG8);
			p2 = p2 + sizeof(sLONG8);
			break;

		case VK_REAL:
			result = VReal::sInfo.CompareTwoPtrToDataWithOptions(p1, p2, inOptions);
			p1 = p1 + sizeof(Real);
			p2 = p2 + sizeof(Real);
			break;

		case VK_WORD:
			if (*(sWORD*)p1 == *(sWORD*)p2)
				result = CR_EQUAL;
			else
			{
				if (*(sWORD*)p1 < *(sWORD*)p2)
					result = CR_SMALLER;
				else
					result = CR_BIGGER;
			}
			p1 = p1 + sizeof(sWORD);
			p2 = p2 + sizeof(sWORD);
			break;

		case VK_BOOLEAN:
			if (*(uCHAR*)p1 == *(uCHAR*)p2)
				result = CR_EQUAL;
			else
			{
				if (*(uCHAR*)p1 < *(uCHAR*)p2)
					result = CR_SMALLER;
				else
					result = CR_BIGGER;
			}
			p1 = p1 + sizeof(uCHAR);
			p2 = p2 + sizeof(uCHAR);
			break;

		case VK_BYTE:
			if (*(sBYTE*)p1 == *(sBYTE*)p2)
				result = CR_EQUAL;
			else
			{
				if (*(sBYTE*)p1 < *(sBYTE*)p2)
					result = CR_SMALLER;
				else
					result = CR_BIGGER;
			}
			p1 = p1 + sizeof(sBYTE);
			p2 = p2 + sizeof(sBYTE);
			break;

		case VK_TIME:
			if (*(xTime*)p1 == *(xTime*)p2)
				result = CR_EQUAL;
			else
			{
				if (*(xTime*)p1 < *(xTime*)p2)
					result = CR_SMALLER;
				else
					result = CR_BIGGER;
			}
			p1 = p1 + sizeof(xTime);
			p2 = p2 + sizeof(xTime);
			break;

		case VK_UUID:
			if (*(VUUIDBuffer*)p1 == *(VUUIDBuffer*)p2)
				result = CR_EQUAL;
			else
			{
				if (*(VUUIDBuffer*)p1 < *(VUUIDBuffer*)p2)
					result = CR_SMALLER;
				else
					result = CR_BIGGER;
			}
			p1 = p1 + sizeof(VUUIDBuffer);
			p2 = p2 + sizeof(VUUIDBuffer);
			break;
				
		case VK_STRING_UTF8:
			result = VStringUTF8::sInfo.CompareTwoPtrToDataWithOptions(p1, p2, inOptions);
			p1 = p1 + VStringUTF8::sInfo.GetSizeOfValueDataPtr(p1);
			p2 = p2 + VStringUTF8::sInfo.GetSizeOfValueDataPtr(p2);
			break;

		case VK_STRING:
			result = VString::sInfo.CompareTwoPtrToDataWithOptions(p1, p2, inOptions);
			p1 = p1 + VString::sInfo.GetSizeOfValueDataPtr(p1);
			p2 = p2 + VString::sInfo.GetSizeOfValueDataPtr(p2);
			break;
		}

		if (result != CR_EQUAL)
		{
			if (!cur->IsAscent())
			{
				if (result == CR_SMALLER)
					result = CR_BIGGER;
				else
					result = CR_SMALLER;
			}
			break;
		}

	} // fin de la boucle de comparaison

	return result;
}

template <class Type>
Boolean TypeSortElemArray<Type>::TryToSort(Selection* From, Selection* &into, VError& err, const xMultiFieldDataOffsets& criterias, BaseTaskInfo* context, 
											VDB4DProgressIndicator* InProgress, Boolean TestUnicite, sLONG TypeSize, IndexInfo* ind)
{
	// si from est nil alors on ne genere pas de selection dans into et l'on travaille sur toute la table
	// si into == -1 alors on trie pour valeur distinct

 
	Boolean pourindex = ((into == nil) || (into == (Selection*)-2)) && (From == nil);
	if (TestUnicite)
		From = nil;
	criterias.SetLength(TypeSize);
	Boolean result = false, indexalreadyclosed = false;;
	VError err2 = VE_OK;
	Boolean BuildASel = (From != nil), OKLock = true, PourValeursDistinctes = false, PourKeyWords = false, goThroughCache = (From == nil);
	Table* sourcetable = nil;
	DataTable *DF = nil;
	void* langcontext = nil;

	bool needFullRecords = false;

	if (criterias.WithExpression() || criterias.WithAttributes())
		needFullRecords = true;

	fNulls = new Bittab();

	if (into == (Selection*)-1)
	{
		into = nil;
		PourValeursDistinctes = true;
	}

	if (into == (Selection*)-2)
	{
		into = nil;
		PourKeyWords = true;
	}

	err = VE_OK;

	fIntlMgr = GetContextIntl(context);
	if (!BuildASel)
	{
		sourcetable = criterias.GetTarget();
		DF = sourcetable->GetDF();
		if (DF != nil)
		{
			if (pourindex)
			{
				OKLock = true;
			}
			else
				OKLock = DF->LockTable(context);
		}
		else
			OKLock = false;
	}

	fOptions.SetIntlManager(fIntlMgr);
	fOptions.SetDiacritical(true);
	fOptions.SetBeginsWith(false);
	fOptions.SetLike(false);

	if (OKLock)
	{
		//sLONG size = sizeof(TypeSortElem<Type>);
		sLONG size = TypeSize + 4;

		if (!BuildASel)
		{
			From=new BitSel(DF);
			if (From==nil)
			{
				err = ThrowBaseError(memfull, DBaction_SortingSelection);
			}
			else
			{
				if (From->FixFic(DF->GetMaxRecords(context)))
				{
					Bittab* tb=((BitSel*)From)->GetTB();
					tb->ClearOrSetAll(true);
					err = DF->TakeOffBits(tb, context);
					count = tb->Compte();
				}
				else
				{
					err = ThrowBaseError(memfull, DBaction_SortingSelection);
				}
			}
		}
		else
			count = From->GetQTfic();

		if (err == VE_OK)
		{
			sLONG tailledebug = sizeof(TypeSortElem<Type>);

			// DH 12-Mar-2013 ACI0080562 There are several places (like TypeSortElemArray::GetDataElem()) where offsets are computed on sLONG so don't allocate buffer bigger than kMAX_sLONG
			// This provides a 2GB buffer hard limit (most often it will be further limited by database parameter "Maximum Temporary Memory Size" which defaults to 500MB)
			uLONG8 taillemax = (uLONG8)kMAX_sLONG;
			VSize maxPerSort = VDBMgr::GetManager()->GetLimitPerSort();
			if (maxPerSort != 0)
			{
				uLONG8 tempmaxPerSort = (uLONG8)maxPerSort * (uLONG8)(1024*1024);
				if (tempmaxPerSort < taillemax)
					taillemax = tempmaxPerSort;
			}

			sLONG8 tailledemandee = (sLONG8)count * (sLONG8)size;
			if (criterias.WithExpression() || PourKeyWords)
			{
				data = nil;
				tailledemandee = -1;
			}
			else
			{
				if (tailledemandee > taillemax || tailledemandee < 0)
					data = nil;
				else
					data = (TypeSortElem<Type>*)GetTempMem(tailledemandee, false, 'tsrt');
			}

			Boolean assez_de_memoire = (data != nil);

			if (data != nil)
			{
				DataTable* df = From->GetParentFile();
				Bittab deja;
				Bittab *filtre;
				sLONG nb = 0;
				Boolean stop = false;

				filtre = From->GenereBittab(context, err);
				if (goThroughCache && filtre != nil && !needFullRecords)
				{
					Bittab newfiltre;
					Bittab* xfiltre;
					Transaction* trans = GetCurrentTransaction(context);
					if (trans != nil)
					{
						Bittab* b1 = trans->GetSavedRecordIDs(df->GetNum(), err, false);
						Bittab* b2 = trans->GetDeletedRecordIDs(df->GetNum(), err, false);
						if (b1 == nil && b2 == nil)
						{
							xfiltre = filtre;
						}
						else
						{
							xfiltre = &newfiltre;
							err2 = xfiltre->Or(filtre);
							if (err2 == VE_OK && b1 != nil)
							{
								err2 = xfiltre->moins(b1);
							}
							if (err2 == VE_OK && b2 != nil)
							{
								err2 = xfiltre->moins(b2);
							}
						}
					}
					else
						xfiltre = filtre;

					if (err2 == VE_OK)
						err2 = deja.aggrandit(df->GetMaxRecords(context));

					if (err2 == VE_OK)
					{
						void* data2 = (void*)data;
						if (ind != nil)
						{
							ind->Close();
							indexalreadyclosed = true;
						}
						df->LockRead();
						OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
						df->FillArrayFromCache(data2, size, xfiltre, deja, criterias, curstack, fNulls);
						df->Unlock();
						nb = (sLONG8)(((char*)data2) - ((char*)data)) / (sLONG8)size;
						xbox_assert(nb>=0 && nb <= count);
					}
					else
					{	
						stop = true;
						assez_de_memoire = false;
					}
					
				}
				else 
				{
					if (filtre == nil)
					{
						stop = true;
						assez_de_memoire = false;
					}
				}

				if (! stop)
				{
					ReadAheadBuffer* buf = df->GetDB()->NewReadAheadBuffer();

					if (InProgress != nil)
					{
						VString temps;
						gs(1005,1,temps);	// Loading data

						/*
						InProgress->SetTitle(L"Sequential Sort");
						InProgress->Start();
						*/
						
						InProgress->BeginSession(count, temps,!pourindex );
					}

					//if (From->GetTypSel() == sel_bitsel)
					{
						if (ind != nil && !indexalreadyclosed)
						{
							ind->Close();
							indexalreadyclosed = true;
						}
						Bittab* b;

						if (goThroughCache)
						{
							deja.Invert();
							deja.And(filtre);
							b = &deja;
						}
						else
							b = filtre;

						sLONG i = 0;
						while (i >= 0 && !stop)
						{
							if (InProgress != nil)
								InProgress->Progress(nb);

							if (MustStopTask(InProgress))
							{
								err = ThrowBaseError(VE_DB4D_ACTIONCANCELEDBYUSER);
								stop = true;
								break;
							}

							i = b->FindNextBit(i);
							if (i >= 0)
							{
								//if (! deja.isOn(i))
								{
									GetDataElem(nb, TypeSize)->recnum = i;
									if (criterias.IsMulti())
										((xMultiFieldData*)&(GetDataElem(nb, TypeSize)->value))->SetHeader(&criterias);

									if (df->AcceptNotFullRecord() && !needFullRecords)
									{
										FicheOnDisk *ficD;
										{
											StErrorContextInstaller errs(true);
											ficD = df->LoadNotFullRecord(i, err, DB4D_Do_Not_Lock, context, false, buf, nil, &assez_de_memoire);
											if (err != VE_OK)
											{
												if (VTaskMgr::Get()->GetCurrentTask()->FailedForMemory())
												{
													assez_de_memoire = false;
													errs.Flush();
													err = VE_OK;
												}
												else
												{
													/* if (pourindex)
														stop = true; */
												}
											}
										}
										if (!assez_de_memoire)
											stop = true;
										if (ficD != nil)
										{
											bool isnull = false, allnull = true;
											xMultiFieldDataOffset_constiterator cur = criterias.Begin(), end = criterias.End();
											for (; cur != end; cur++)
											{
												sLONG TypeOnDisk;
												void* p = (void*) ficD->GetDataPtrForQuery(cur->GetNumCrit(), &TypeOnDisk);
												if (p == nil)
												{
													if (criterias.IsMulti())
														cur->SetNull(i);
													else
													{
														isnull = true;
														break;
													}
													//memset(GetSubDataPtr(nb, TypeSize, cur->GetOffset()), 0, cur->GetSize());
												}
												else
												{
													allnull = false;
													if (TypeOnDisk == cur->GetDataType())
													{
														if (cur->GetDataType() == VK_STRING)
														{
															xString* xs = (xString*)GetSubDataPtr(nb, TypeSize, cur->GetOffset());
															xs->SetSize(cur->GetSize());
															if (!xs->CopyFrom(p))
															{
																// plus assez de memoire
																stop = true;
																assez_de_memoire = false;
															}
														}
														else
														{
															if (cur->GetDataType() == VK_STRING_UTF8)
															{
																xStringUTF8* xs = (xStringUTF8*)GetSubDataPtr(nb, TypeSize, cur->GetOffset());
																xs->SetSize(cur->GetSize());
																if (!xs->CopyFrom(p))
																{
																	// plus assez de memoire
																	stop = true;
																	assez_de_memoire = false;
																}
															}
															else
																vBlockMove(p, GetSubDataPtr(nb, TypeSize, cur->GetOffset()), cur->GetSize());
														}
													}
													else
													{
														ValPtr cv = NewValPtr(cur->GetDataType(), p, TypeOnDisk, ficD->GetOwner(), context);
														if (cv == nil)
														{
															memset(GetSubDataPtr(nb, TypeSize, cur->GetOffset()), 0, cur->GetSize());
														}
														else
														{
															if (cur->GetDataType() == VK_STRING)
															{
																xString* xs = (xString*)GetSubDataPtr(nb, TypeSize, cur->GetOffset());
																xs->SetSize(cur->GetSize());
																if (!xs->CopyFromString((VString*)cv))
																{
																	// plus assez de memoire
																	stop = true;
																	assez_de_memoire = false;
																}
															}
															else
															{
																if (cur->GetDataType() == VK_STRING_UTF8)
																{
																	xStringUTF8* xs = (xStringUTF8*)GetSubDataPtr(nb, TypeSize, cur->GetOffset());
																	xs->SetSize(cur->GetSize());
																	if (!xs->CopyFromString((VString*)cv))
																	{
																		// plus assez de memoire
																		stop = true;
																		assez_de_memoire = false;
																	}
																}
																else
																	cv->WriteToPtr(GetSubDataPtr(nb, TypeSize, cur->GetOffset()));
															}
															delete cv;
														}
													}
												}
											}

											//ficD->FreeAfterUse();
											ficD->Release();

											if (isnull || allnull)
												fNulls->Set(i);
											else
												nb++;
											xbox_assert(nb <= count);
										}
									}
									else
									{
										FicheInMem *fic;
										{
											StErrorContextInstaller errs(true);
											fic = df->LoadRecord(i, err, DB4D_Do_Not_Lock, context, false, false, buf, &assez_de_memoire);
											if (err != VE_OK)
											{
												if (VTaskMgr::Get()->GetCurrentTask()->FailedForMemory())
												{
													assez_de_memoire = false;
													errs.Flush();
													err = VE_OK;
												}
												else
												{
													/* if (pourindex)
														stop = true; */
												}
											}
										}
										if (!assez_de_memoire)
											stop = true;
										if (fic != nil)
										{
											bool isnull = false, allnull = true;
											EntityRecord *erec = nil;
											xMultiFieldDataOffset_constiterator cur = criterias.Begin(), end = criterias.End();
											for (; cur != end; cur++)
											{
												db4dEntityAttribute* att = cur->GetAttribute();
												AttributePath* attpath = cur->GetAttributePath();
												VValueSingle* cv = nil;
												if (attpath != nil)
												{
													const db4dEntityAttribute* firstpart = (db4dEntityAttribute*)(attpath->FirstPart()->fAtt);
													if (erec == nil)
														erec = new LocalEntityRecord(firstpart->GetLocalModel(), fic, context);
													EntityAttributeValue* val = erec->getAttributeValue(*attpath, err, context);
													if (val != nil)
														cv = val->getVValue();
												}
												else if (att == nil)
													cv = fic->GetNthField(cur->GetNumCrit(), err, false, true);
												else
												{
													if (erec == nil)
														erec = new LocalEntityRecord(att->GetLocalModel(), fic, context);
													EntityAttributeValue* val = erec->getAttributeValue(att, err, context);
													if (val != nil)
														cv = val->getVValue();
												}
												
												if (cv == nil || cv->IsNull())
												{
													if (criterias.IsMulti())
														cur->SetNull(i);
													else
													{
														isnull = true;
														break;
													}													
													//memset(GetSubDataPtr(nb, TypeSize, cur->GetOffset()), 0, cur->GetSize());
												}
												else
												{
													allnull = false;
													VValueSingle* cv2 = nil;
													if (cv->GetValueKind() != cur->GetDataType() || cv->GetTrueValueKind() == VK_TEXT)
													{
														cv2 = cv->ConvertTo(cur->GetDataType());
														cv = cv2;
													}
													if (cv == nil || cv->IsNull())
													{
														memset(GetSubDataPtr(nb, TypeSize, cur->GetOffset()), 0, cur->GetSize());
													}
													else
													{
														if (cur->GetDataType() == VK_STRING)
														{
															xString* xs = (xString*)GetSubDataPtr(nb, TypeSize, cur->GetOffset());
															xs->SetSize(cur->GetSize());
															if (!xs->CopyFromString((VString*)cv))
															{
																// plus assez de memoire
																stop = true;
																assez_de_memoire = false;
															}
														}
														else
														{
															if (cur->GetDataType() == VK_STRING_UTF8)
															{
																xStringUTF8* xs = (xStringUTF8*)GetSubDataPtr(nb, TypeSize, cur->GetOffset());
																xs->SetSize(cur->GetSize());
																if (!xs->CopyFromString((VString*)cv))
																{
																	// plus assez de memoire
																	stop = true;
																	assez_de_memoire = false;
																}
															}
															else
																cv->WriteToPtr(GetSubDataPtr(nb, TypeSize, cur->GetOffset()));
														}
													}
													if (cv2 != nil)
														delete cv2;
												}
											}

											QuickReleaseRefCountable(erec);
											fic->Release();
											if (isnull || allnull)
												fNulls->Set(i);
											else
												nb++;
											xbox_assert(nb <= count);
										}
									}

								}

								i++;
							}
						}
					}

					ReleaseRefCountable(&filtre);

					if (buf != nil)
						buf->Release();

					/*
					if (InProgress != nil)
					{
						InProgress->EndSession();
					}
					*/

					count = nb;
					if (!stop)
					{
						if ((TypeSize+4) > 127 && auxdata == nil)
						{
							auxdata = GetTempMem(kAuxDataBufferSize, false, 'tAux');
							xbox_assert(auxdata != nil);
							auxbuff = auxdata;
						}

						ptrvect indirection;
						bool useIndirection = false;

						if (count > 0)
						{
							if (auxbuff != nil && criterias.IsMulti())
								new (auxbuff) TypeSortElem<Type>();

							criterias.SetAuxBuffer((void*)auxbuff);

							if (TypeSize>8 && !pourindex && !PourValeursDistinctes)
							{
								try
								{
									indirection.resize(count, nil);
									typename ptrvect::iterator cur = indirection.begin();
									for (sLONG i = 0; i < count; i++, cur++)
									{
										*cur = GetDataElem(i, TypeSize);
									}
									useIndirection = true;

								}
								catch (...)
								{
								}
							}
							
							TypeSortElemIterator<Type> First(GetDataElem(0, TypeSize), TypeSize+4), Last(GetDataElem(count, TypeSize), TypeSize+4);
							if (TestUnicite)
							{
								SortPredicateHolder holder(fOptions, criterias, true);
								SortPredicate<Type> sortpred(&holder);
								if (useIndirection)
								{
									SortIndirectionPredicate<Type> sortindpred(&sortpred);
									std::sort(indirection.begin(), indirection.end(), sortindpred);
								}
								else
									std::sort(First, Last, sortpred);
								if (holder.fNonUnique)
								{
									fAllUnique = false;
									if (fBreakOnNonUnique)
										err = ThrowBaseError(VE_DB4D_DUPLICATED_KEY);
								}
							}
							else
							{
								if (pourindex)
								{
									SortPredicateHolder holder(fOptions, criterias, false);
									SortPredicate<Type> sortpred(&holder);
									if (useIndirection)
									{
										SortIndirectionPredicate<Type> sortindpred(&sortpred);
										std::sort(indirection.begin(), indirection.end(), sortindpred);
									}
									else
										std::sort(First, Last, sortpred);
								}
								else
								{
									SortPredicateHolder holder(fOptions, criterias, false);
									SortPredicate<Type> sortpred(&holder); // pour l'instant le meme que pour index, pourrait etre legerement optimise dans le cas d'un tri normal, 
																							// car on se fiche de l'ordre des No de fiches si les valeurs sont egales
									if (useIndirection)
									{
										SortIndirectionPredicate<Type> sortindpred(&sortpred);
										std::sort(indirection.begin(), indirection.end(), sortindpred);
									}
									else
										std::sort(First, Last, sortpred);
								}
							}

#if debuglr
							sLONG gdebug = 0;
							sLONG xdebug = 1;

							if (gdebug)
							{
								DebugMsg("sorted elems \n");
								TypeSortElemIterator<Type> curelem(GetDataElem(0, TypeSize), TypeSize+4);
								while (curelem != Last)
								{
									DebugMsg("value : ");
									VString s;
									curelem->GetString(s);
									DebugMsg(s);
									DebugMsg("  ,  recnum : ");
									DebugMsg(ToString(curelem->recnum));
									DebugMsg("\n");
									++curelem;
								}

								DebugMsg("\n");
								DebugMsg("\n");
							}

#endif


						}

						if (err == VE_OK)
						{
							if (BuildASel && !PourValeursDistinctes)
							{
								sLONG nbnulls = 0;
								if (fNulls != nil)
								{
									nbnulls = fNulls->Compte();
								}
								if ((count+nbnulls)>kNbElemInSel)
								{
									into = new LongSel(df);
									((LongSel*)into)->PutInCache();
								}
								else
								{
									into = new PetiteSel(df);
								}

								if (into != nil)
								{
									sLONG startfrom = 0;
									if (fNulls != nil)
									{
										if (nbnulls != 0 && !criterias.IsAllDescent())
										{
											if (!into->FixFic(nbnulls))
												err = ThrowBaseError(memfull);
											else
											{
												if (inWafSelbits != nil && outWafSel != nil)
												{
													sLONG n = fNulls->FindNextBit(0);
													while (n != -1 && err == VE_OK)
													{
														err = into->PutFicWithWafSelection(startfrom, n, inWafSelbits, outWafSel);
														startfrom++;
														n = fNulls->FindNextBit(n+1);
													}
												}
												else
												{
													into->UpdateFromBittab(fNulls);
													startfrom = nbnulls;
												}
											}
										}
									}
									bool okgo = false;
									if (useIndirection)
										okgo = count == 0 || into->FillWith(&indirection[0], sizeof(void*), count, criterias.IsAscent(), df->GetMaxRecords(context), startfrom, true, inWafSelbits, outWafSel) == VE_OK;
									else
										okgo = into->FillWith(data, size, count, criterias.IsAscent(), df->GetMaxRecords(context), startfrom, false, inWafSelbits, outWafSel) == VE_OK;

									if (okgo)
									{
										if (nbnulls != 0 && criterias.IsAllDescent())
										{
											sLONG curqtfic = count;
											if (!into->FixFic(count+nbnulls))
												err = ThrowBaseError(memfull);
											sLONG n = fNulls->FindNextBit(0);
											while (n != -1 && err == VE_OK)
											{
												err = into->PutFicWithWafSelection(curqtfic, n, inWafSelbits, outWafSel);
												curqtfic++;
												n = fNulls->FindNextBit(n+1);
											}
										}
										if (err != VE_OK)
										{
											into->Release();;
											into = nil;
										}
										else
											result = true;
									}
									else
									{
										into->Release();;
										into = nil;
									}
								}
							}
							else result = true;
						}
					}

					if (InProgress != nil)
					{
						InProgress->EndSession();
					}

				}
				
				/*
				FreeFastMem(data);
				data = nil;
				*/
			} // du if data != nil


			if (err == VE_OK && !assez_de_memoire)  
			{

				if (auxdata != nil)
				{
					FreeTempMem((void*)auxdata);
					auxdata = nil;
				}
				if (auxdata2 != nil)
				{
					FreeTempMem((void*)auxdata2);
					auxdata2 = nil;
				}

				if (data != nil)
				{
					if (criterias.IsMulti())
					{
						FreeXMulti(*((TypeSortElemArray<xMultiFieldData>*)this), TypeSize, criterias);
					}
					else
					{
						if (criterias.GetOffset(0)->GetDataType() == VK_STRING)
						{
							FreeXString(*((TypeSortElemArray<xString>*)this), TypeSize);
						}
						else
						{
							if (criterias.GetOffset(0)->GetDataType() == VK_STRING_UTF8)
							{
								FreeXStringUTF8(*((TypeSortElemArray<xStringUTF8>*)this), TypeSize);
							}
						}
					}

					FreeTempMem(data);
					data = nil;
				}

				// si pas assez de memoire pour tout trier en memoire, on va essayer de trier par blocs sur disque et de les merger
				// il faut un espace disque temporaire suffisant.
				if (count == 0)
				{
					result = true;
				}
				else
				{
#if oldtempmem
					VSize totmem = VDBMgr::GetManager()->GetTempMemMgr()->GetAllocatedMem();
					VSize taillebloc = totmem / 3;

					void* partdata = nil;

					partdata = GetTempMem(taillebloc, false, 'tSrt');
					if (partdata == nil)
					{
						VMemStats stats;
						VDBMgr::GetManager()->GetTempMemMgr()->GetStatistics(stats);
						taillebloc = stats.fBiggestBlockFree - 1000000; // un peu de gras pour diverses autres donnees

						do 
						{
							partdata = GetTempMem(taillebloc, false, 'tSrt');
							if (partdata == nil)
							{
								taillebloc = taillebloc / 2;
								if (taillebloc < 2000000)
									break;
							}
						} while(partdata == nil);
					}

#else
					uLONG8 totmem = VSystem::GetPhysicalMemSize();
					uLONG8 xtaillebloc = totmem / 4;

					if (xtaillebloc > taillemax)
						xtaillebloc = taillemax;
					VSize taillebloc = (VSize)xtaillebloc;


					uLONG8 nbelemtot = count;
					uLONG8 tailleelem = criterias.GetLength() + 4;
					
					if (PourKeyWords)
						tailleelem = tailleelem * 30;

					uLONG8 taillepresumee = (nbelemtot * tailleelem) + 30000;
					if (taillepresumee < (uLONG8)taillebloc)
						taillebloc = (VSize)taillepresumee;

					void* partdata = nil;
					if (taillebloc < 2000000)
						taillebloc = 2000000;

					// no more than 1Gb
					if (taillebloc > 1024*1024*1024)
						taillebloc = 1024*1024*1024;

					do 
					{
						partdata = GetTempMem(taillebloc, false, 'tSrt');
						if (partdata == nil)
						{
							taillebloc = taillebloc / 2;
							if (taillebloc < 2000000)
								break;
						}
					} while(partdata == nil);

#endif


					if (partdata != nil)
					{
						Base4D* db = nil;
						TypeSortElemOffsetArray<Type>* curpart = new (partdata) TypeSortElemOffsetArray<Type>((sLONG)taillebloc);

						if (BuildASel)
						{
							if (From->IsRemoteLike())
							{
								db = criterias.GetTarget()->GetOwner();
							}
							else
								db = From->GetParentFile()->GetDB();
						}
						else
							db = sourcetable->GetOwner();

						if (db != nil)
						{
							VFolder *tempfolder = db->RetainTemporaryFolder( true, &err);
							if (tempfolder != nil)
							{
								sLONG8 freespaceondisk;
								VError err2 = tempfolder->GetVolumeFreeSpace(&freespaceondisk);
								tailledemandee = tailledemandee * 3;
								if (tailledemandee > freespaceondisk)
								{
									// pas assez d'espace disque temporaire, il faudrait trouver un autre volume
								}
								else
								{
									CDB4DBaseContext* encapsuleur = nil;
									if (context != nil)
										encapsuleur = context->GetEncapsuleur();
									if (encapsuleur != nil)
										encapsuleur->Retain();
									TempFileArray tempFiles;

									VFileStream* curfile = nil;

									DataTable* df = nil;
									Bittab *filtre = nil;
									sLONG nb = 0;
									Boolean stop = false;
									CDB4DFieldCacheCollection* fieldcache = nil;
									Boolean trionclient = From != nil && From->IsRemoteLike();
									CDB4DRemoteRecordCache* remotecache = nil;
									CDB4DSelection* xFrom = nil;


									if (From->IsRemoteLike())
									{
										From->Retain();
										xFrom = new VDB4DSelection(VDBMgr::GetManager(), nil, criterias.GetTarget(), From);
										fieldcache = new VDB4DFieldCacheCollection(criterias.GetTarget());
										vector<uBYTE> none;
										remotecache = context->GetEncapsuleur()->StartCachingRemoteRecords(xFrom, 0, 0, fieldcache, none);
										if (remotecache == nil)
										{
											err = ThrowBaseError(VE_DB4D_CANNOT_COMPLETE_SEQ_SORT);
											stop = true;
										}
									}
									else
									{
										df = From->GetParentFile();
										filtre = From->GenereBittab(context, err);
										if (filtre == nil)
											stop = true;
									}

									if (!stop)
									{
										count = From->GetQTfic();
										ReadAheadBuffer* buf = db->NewReadAheadBuffer();

										if (InProgress != nil)
										{
											VString temps;
											gs(1005,1,temps);	// Loading data

											/*
											InProgress->SetTitle(L"Sequential Sort");
											InProgress->Start();
											*/
											InProgress->BeginSession(count, temps ,!pourindex);
											
										}

										if (ind != nil && !indexalreadyclosed)
										{
											ind->Close();
											indexalreadyclosed = true;
										}

										//sLONG debugcount1 = 0, debugcount2 = 0;
										sLONG i = 0;
										while (i >= 0 && !stop)
										{
											if (InProgress != nil)
												InProgress->Progress(nb);

											if (MustStopTask(InProgress))
											{
												err = VE_DB4D_ACTIONCANCELEDBYUSER;
												stop = true;
												break;
											}
											if (trionclient)
											{
												if ((i%1000) == 0 || i == 1)
												{
													if (i == 0)
													{
														// rien a faire car remotecache a ete cree plus haut
													}
													else
													{
														QuickReleaseRefCountable(remotecache);
														sLONG i2;
														if (i == 1)
															i2 = i + 998;
														else
															i2 = i + 999;
														if (i2 >= count)
															i2 = count - 1;
														vector<uBYTE> none;
														remotecache = context->GetEncapsuleur()->StartCachingRemoteRecords(xFrom, i, i2, fieldcache, none);
														if (remotecache == nil)
														{
															err = ThrowBaseError(VE_DB4D_CANNOT_COMPLETE_SEQ_SORT);
															stop = true;
															break;
														}
													}
												}
											}
											else
											{
												i = filtre->FindNextBit(i);
											}
											if (i >= 0)
											{	
												sLONG xRecordNumber = i;
												FicheInMem *fic = nil;
												vector<CachedRelatedRecord> relatedrecs;
												CDB4DRecord* xfic = nil;

												if (trionclient)
												{
													xRecordNumber = -1;
													err = remotecache->RetainCacheRecord(i, xfic, relatedrecs);
													if (xfic != nil)
													{
														fic = dynamic_cast<VDB4DRecord*>(xfic)->GetRec();
														if (fic != nil)
														{
															fic->Retain();
															xRecordNumber = fic->GetNum();
														}
													}
												}
												else
													fic = df->LoadRecord(i, err, DB4D_Do_Not_Lock, context, false, false, buf);
												if (err != VE_OK)
												{
													if (VTaskMgr::Get()->GetCurrentTask()->FailedForMemory())
													{
														stop = true;
														FlushErrors();
														err = VE_OK;
														break;
													}
													else
													{
														/*if (pourindex)
															stop = true;*/
													}
												}
												if (fic != nil)
												{
													typedef map<DB4DLanguageExpression*, const VValueSingle*> ValuesMap;

													ValuesMap valexpr;
													EntityModel* em = nil;
													EntityRecord* erec = nil;

													Boolean stopsub = false;
													VValueSingle* allwords = nil;
													DB4DKeyWordList keywords;
													if (PourKeyWords)
													{
														xMultiFieldDataOffset_constiterator cri = criterias.Begin();
														allwords = fic->GetNthField(cri->GetNumCrit(), err, false, true);
														if (allwords != nil)
														{
															if (allwords->GetValueKind() == VK_STRING || allwords->GetValueKind() == VK_TEXT)
															{
																err = BuildKeyWords(*((VString*)allwords), keywords, fOptions);
															}
															else
																err = ThrowBaseError(memfull, DBaction_SortingSelection);
														}
														else
														{
															if (err == VE_OK)
																err = ThrowBaseError(memfull, DBaction_SortingSelection);
														}
														stopsub = keywords.GetCount() == 0;
													}

													sLONG curkeyword = 0;

													if (err == VE_OK)
													{
														while (!stopsub)
														{
															//debugcount1++;
															Boolean isnull = false;
															VSize len = 0;

															if (PourKeyWords)
															{
																VString* s = keywords[curkeyword];
																s->Truncate(kMaxXStringPtrLen);
																len = s->GetSpace();
															}
															else
															{

																xMultiFieldDataOffset_constiterator cur = criterias.Begin(), end = criterias.End();
																for (; cur != end; cur++)
																{
																	const VValueSingle* cv = nil;
																	DB4DLanguageExpression* expr = cur->GetExpression();
																	if (expr != nil)
																	{
																		/*
																		if (encapsuleur == nil && context != nil)
																		{
																			CDB4DBase* basex = db->RetainBaseX();
																			xbox_assert(basex != nil);
																			encapsuleur = new BaseTaskInfo(db, basex);
																			basex->Release();
																		}
																		*/
																		CDB4DRecord* encapsulerec;
																		if (trionclient)
																		{
																			encapsulerec = xfic;
																			if (encapsulerec != nil)
																				encapsulerec->Retain();
																		}
																		else
																		{

																			fic->Retain();	// L.E. 06/04/06 ca me rappelle qqchose pour les triggers ;-)
																			encapsulerec = new VDB4DRecord(VDBMgr::GetManager(), fic, encapsuleur);
																		}
																		if (encapsulerec == nil)
																		{
																			err = ThrowBaseError(memfull, DBaction_SortingSelection);
																			stop = true;
																			break;
																		}
																		else
																		{
																			if (trionclient)
																				cv = expr->Execute(encapsuleur, langcontext, encapsulerec, err, &relatedrecs);
																			else
																				cv = expr->Execute(encapsuleur, langcontext, encapsulerec, err);
																			if (err != VE_OK)
																			{
																				stop = true;
																			}
																			else
																			{
																				if (cur->GetDataType() == (sLONG)VK_UNDEFINED)
																				{
																					if (cv != nil)
																					{
																						ValueKind cvtyp = cv->GetValueKind();
																						if (cv->GetTrueValueKind() == VK_TEXT || cvtyp == VK_TEXT || cvtyp == VK_STRING_UTF8 || cvtyp == VK_TEXT_UTF8)
																							cvtyp = VK_STRING;
																						((xMultiFieldDataOffset_iterator)cur)->SetDataType((sLONG)(cvtyp));
																					}
																				}
																				valexpr[expr] = cv;
																			}
																			encapsulerec->Release();
																		}

																		if (err != VE_OK)
																		{
																			if (cv != nil)
																				delete cv;
																			break;
																		}
																	}
																	else
																	{
																		db4dEntityAttribute* att = cur->GetAttribute();
																		if (att != nil)
																		{
																			cv = nil;
																			if (erec == nil)
																			{
																				em = att->GetOwner();
																				erec = new LocalEntityRecord(att->GetLocalModel(), fic, context);
																			}
																			EntityAttributeValue* val = erec->getAttributeValue(att, err, context);
																			if (val != nil)
																				cv = val->getVValue();
																		}
																		else
																			cv = fic->GetNthField(cur->GetNumCrit(), err, false, true);
																	}

																	if (cv == nil || cv->IsNull())
																	{
																		isnull = true;
																	}
																	else
																	{
																		VValueSingle* cv2 = nil;
																		if (cv->GetValueKind() != cur->GetDataType() || cv->GetTrueValueKind() == VK_TEXT || cv->GetTrueValueKind() == VK_TEXT_UTF8)
																		{
																			cv2 = cv->ConvertTo(cur->GetDataType());
																			cv = cv2;
																		}
																		if (cv == nil || cv->IsNull())
																		{
																			isnull = true;
																		}
																		else
																		{
																			if (cv->GetValueKind() == VK_STRING)
																				((VString*)cv)->Truncate(kMaxXStringPtrLen);
																			len = len + cv->GetSpace();
																		}
																		if (cv2 != nil)
																			delete cv2;
																	}
																	
																	/*
																	if (expr != nil && cv != nil)
																		delete cv;
																		*/
																}
															}

															void* dest = nil;

															if (isnull && fNulls != nil)
															{
																len = 0;
																fNulls->Set(xRecordNumber);
															}
															else
															{
																if (isnull)
																	len = 0;

																if (curfile != nil)
																	dest = curpart->AddOffSet(xRecordNumber, (sLONG) len);

																if (dest == nil)
																{
																	if (curfile != nil)
																	{
																		//debugcount2 = debugcount2 + curpart->GetNbElem();
																		Boolean okuniq;
																		err = curpart->Sort(criterias, TestUnicite, fBreakOnNonUnique, okuniq, pourindex, fOptions);
																		if (!okuniq)
																			fAllUnique = false;
																		curfile->SetNeedSwap(false);
																		if (err == VE_OK)
																			err = curpart->WriteToStream(curfile);
																		VError err2 = curfile->CloseWriting();
																		curpart->SetNbElem(0);
																		dest = curpart->AddOffSet(xRecordNumber, (sLONG) len);
																		if (err == VE_OK)
																			err = err2;
																		if (err != VE_OK)
																		{
																			// si on ne peut pas stocker dans le fichier temp alors il faut annuler toute l'operation
																			stop = true;
																			break;
																		}
																	}

																	curfile = CreateTempFile(db, nil, taillebloc);
																	if (curfile == nil)
																	{
																		stop = true;
																		err = VE_DB4D_NOT_ENOUGH_FREE_SPACE_ON_DISK;
																		break;
																		// si on ne peut plus creer de fichier temp alors il faut annuler toute l'operation
																	}
																	else
																	{
																		tempFiles.push_back(curfile);
																		curfile->OpenWriting();
																		curpart->SetNbElem(0);
																		dest = curpart->AddOffSet(xRecordNumber, (sLONG) len);
																	}

																}
															}

															if (isnull)
															{
																//curpart->SetLastElemNull();
																// deja fait dans le AddOffSet quand len == 0
															}
															else
															{
																if (PourKeyWords)
																{
																	VString* s = keywords[curkeyword];
																	dest = s->WriteToPtr(dest);
																}
																else
																{
																	void* olddest = dest;
																	xMultiFieldDataOffset_constiterator cur = criterias.Begin(), end = criterias.End();
																	for (; cur != end; cur++)
																	{
																		const VValueSingle* cv = nil;
																		DB4DLanguageExpression* expr = cur->GetExpression();
																		if (expr != nil)
																		{
																			cv = valexpr[expr];
																		}
																		else
																		{
																			db4dEntityAttribute* att = cur->GetAttribute();
																			if (att != nil)
																			{
																				cv = nil;
																				EntityAttributeValue* val = erec->getAttributeValue(att, err, context);
																				if (val != nil)
																					cv = val->getVValue();
																			}
																			else
																				cv = fic->GetNthField(cur->GetNumCrit(), err, false, true);
																		}
																		if (cv == nil)
																		{
																			err = ThrowBaseError(memfull, DBaction_SortingSelection);
																			break;
																		}
																		else
																		{
																			VValueSingle* cv2 = nil;
																			if (cv->GetValueKind() != cur->GetDataType() || cv->GetTrueValueKind() == VK_TEXT)
																			{
																				cv2 = cv->ConvertTo(cur->GetDataType());
																				cv = cv2;
																			}
																			if (cv == nil)
																			{
																				err = ThrowBaseError(memfull, DBaction_SortingSelection);
																				break;
																			}
																			else
																			{
																				if (cv->GetValueKind() == VK_STRING)
																					((VString*)cv)->Truncate(kMaxXStringPtrLen);
																				dest = cv->WriteToPtr(dest);
																			}
																			if (cv2 != nil)
																				delete cv2;
																		}
																	}

																	if (err == VE_OK)
																		xbox_assert(((char*)olddest) + len == (char*)dest);
																	else
																	{
																		stop = true;
																		break;
																	}
																}
															} 

															if (PourKeyWords)
															{
																curkeyword++;
																if (curkeyword >= keywords.GetCount())
																	stopsub = true;
															}
															else
															{
																stopsub = true;
															}

														} // while (!stopsub)
													}
													else
														stop = true;

													nb++;
													xbox_assert(nb <= count);

													if (erec != nil)
														erec->Release();
													fic->Release();

													for (ValuesMap::iterator cur = valexpr.begin(), end = valexpr.end(); cur != end; cur++)
													{
														if (cur->second != nil)
															delete cur->second;
													}


													if (trionclient)
													{
														xMultiFieldDataOffset_constiterator cur = criterias.Begin(), end = criterias.End();
														for (; cur != end; cur++)
														{
															const VValueSingle* cv = nil;
															DB4DLanguageExpression* expr = cur->GetExpression();
															if (expr != nil)
															{
																for (vector<CachedRelatedRecord>::iterator curx = relatedrecs.begin(), endx = relatedrecs.end(); curx != endx; curx++)
																{
																	CachedRelatedRecord temp;
																	temp.Steal(*curx);
																}
																expr->ResetSelectionsAndRecords(encapsuleur, langcontext, criterias.GetTarget()->GetNum(), err, &relatedrecs);
															}
														}
													}

												} // du if fic != nil
											
												i++;
												if (trionclient)
												{
													QuickReleaseRefCountable(xfic);
													if (i >= count)
														i = -1;
												}
											} // du if i > 0

										} // du (while i>=0)


										if (curfile != nil)
										{
											if (err == VE_OK)
											{
												//debugcount2 = debugcount2 + curpart->GetNbElem();
												Boolean okuniq;
												err = curpart->Sort(criterias, TestUnicite, fBreakOnNonUnique, okuniq, pourindex, fOptions);
												if (!okuniq)
													fAllUnique = false;
												curfile->SetNeedSwap(false);
												if (err == VE_OK)
													err = curpart->WriteToStream(curfile);
											}
											VError err2 = curfile->CloseWriting();
											if (err == VE_OK)
												err = err2;
											if (err != VE_OK)
											{
												// si on ne peut pas stocker dans le fichier temp alors il faut annuler toute l'operation
												stop = true;
											}
										}

										//xbox_assert(debugcount2 == count);
										//xbox_assert(debugcount1 == count);
										ReleaseRefCountable(&filtre);

										if (buf != nil)
											buf->Release();

										if (!stop) // si tout c'est bien passe pendant la creation des differentes parties a merger
										{
											Real rcount = tempFiles.size();
											Real rln2 = log(rcount)/log(2.0);
											Real rNbIterComp = ((Real)count)*rln2;
											sLONG8 NbIterComp = rNbIterComp, curIterComp = 0;
											if (InProgress != nil)
											{
												XBOX::VString session_title;
												gs(1005,13,session_title);	// Merging items
												InProgress->BeginSession(NbIterComp,session_title,!pourindex);
											}
											while (tempFiles.size() > 1 && !stop)
											{
												TempFileArray tempfiles2;
												sLONG NbCouplesToMerge = (sLONG)tempFiles.size() / 2;
												TempFileArray::iterator cur = tempFiles.begin();
												for (sLONG i = 0; i < NbCouplesToMerge; i++)
												{
													VFileStream* f1 = *cur++;
													VFileStream* f2 = *cur++;

													err = f1->OpenReading();
													if (err == VE_OK)
														err = f2->OpenReading();
													if (err == VE_OK)
													{
														f1->SetBufferSize(512000);
														f2->SetBufferSize(512000);
														VSize totsize = f1->GetSize() + f2->GetSize();
														VFileStream* merged = CreateTempFile(db, nil, (uLONG8)totsize);
														if (merged == nil)
														{
															stop = true;
															err = VE_DB4D_NOT_ENOUGH_FREE_SPACE_ON_DISK;
														}
														else
														{
															tempfiles2.push_back(merged);
															merged->OpenWriting();
															merged->SetBufferSize(512000);
															
															sLONG nbelem1 = 0, nbelem2 = 0;
															sLONG numrec1 = -1, numrec2 = -1;
															sLONG totlen1, totlen2;

															char* buffer1[1024];
															char* buffer2[1024];

															void* xp1 = nil;
															void* xp2 = nil;

															err = f1->GetLong(nbelem1);
															if (err == VE_OK)
																err = f2->GetLong(nbelem2);
															if (err == VE_OK)
																err = merged->PutLong(nbelem1+nbelem2);

															sLONG debugcount = 0;
															sLONG sumcount = nbelem1+nbelem2;
															while ((nbelem1 > 0 || nbelem2 > 0) && err == VE_OK)
															{
																debugcount++;
																if (numrec1 == -1)
																{
																	if (nbelem1 > 0)
																	{
																		totlen1 = 0;
																		xp1 = &buffer1[0];

																		err = f1->GetLong(numrec1);
																		if (err == VE_OK)
																			err = f1->GetLong(totlen1);
																		if (err == VE_OK && totlen1 > 1024)
																		{
																			xp1 = GetTempMem(totlen1, false, 'xp1 ');
																			if (xp1 == nil)
																			{
																				err = ThrowBaseError(memfull, DBaction_SortingSelection);
																				totlen1 = 0;
																			}
																		}
																		if (err == VE_OK)
																		{
																			uLONG len = totlen1;
																			err = f1->GetData(xp1, len);
																			if (err == VE_OK && len != totlen1)
																				err = ThrowBaseError(memfull, DBaction_SortingSelection);
																		}

																	}
																}

																if (numrec2 == -1 && err == VE_OK)
																{
																	if (nbelem2 > 0)
																	{
																		totlen2 = 0;
																		xp2 = &buffer2[0];

																		err = f2->GetLong(numrec2);
																		if (err == VE_OK)
																			err = f2->GetLong(totlen2);
																		if (err == VE_OK && totlen2 > 1024)
																		{
																			xp2 = GetTempMem(totlen2, false, 'xp2 ');
																			if (xp2 == nil)
																			{
																				err = ThrowBaseError(memfull, DBaction_SortingSelection);
																				totlen2 = 0;
																			}
																		}
																		if (err == VE_OK)
																		{
																			uLONG len = totlen2;
																			err = f2->GetData(xp2, len);
																			if (err == VE_OK && len != totlen2)
																				err = ThrowBaseError(memfull, DBaction_SortingSelection);
																		}

																	}
																}

																Boolean takeone = true;

																if (err == VE_OK)
																{
																	if (numrec1 == -1)
																		takeone = false;
																	else
																	{
																		if (numrec2 != -1)
																		{
																			CompareResult result = Compare2SortElems(xp1, xp2, criterias, fOptions);
																			if (TestUnicite)
																			{
																				if (result == CR_EQUAL)
																				{
																					fAllUnique = false;
																					if (fBreakOnNonUnique)
																						err = VE_DB4D_DUPLICATED_KEY;
																				}
																			}
																			else
																			{
																				if (pourindex)
																				{
																					if (result == CR_EQUAL)
																					{
																						if (numrec1 < numrec2)
																							result = CR_SMALLER;
																						else
																							result = CR_BIGGER;
																					}
																				}
																			}
																					
																			if (result == CR_SMALLER)
																				takeone = true;
																			else
																				takeone = false;

																		}
																	}


																} // fin du test de comparaison pour savoir s'il faut prendre la ligne du fichier 1 ou celle du 2
																if (err == VE_OK)
																{
																	if (InProgress != nil)
																	{
																		curIterComp++;
																		InProgress->Progress(curIterComp);
																	}

																	if (MustStopTask(InProgress))
																		err = VE_DB4D_ACTIONCANCELEDBYUSER;
																}

																if (err == VE_OK)
																{

																	if (takeone)
																	{
																		err = merged->PutLong(numrec1);
																		if (err == VE_OK)
																			err = merged->PutLong(totlen1);
																		if (err == VE_OK && totlen1 > 0)
																			err = merged->PutData(xp1, totlen1);
																		numrec1 = -1;
																		if (totlen1 > 1024)
																			FreeTempMem(xp1);
																		xp1 = nil;
																		nbelem1--;
																	}
																	else
																	{
																		xbox_assert(numrec2 != -1);
																		err = merged->PutLong(numrec2);
																		if (err == VE_OK)
																			err = merged->PutLong(totlen2);
																		if (err == VE_OK && totlen2 > 0)
																			err = merged->PutData(xp2, totlen2);
																		numrec2 = -1;
																		if (totlen2 > 1024)
																			FreeTempMem(xp2);
																		xp2 = nil;
																		nbelem2--;
																	}
																}

															} // du while (nbelem1 > 0 || nbelem2 > 0)

															xbox_assert(debugcount == sumcount);

															merged->CloseWriting();
														} // fin du merge de f1 et f2 dans merged
													}

													f1->CloseReading();
													f2->CloseReading();

													if (err != VE_OK)
													{
														stop = true;
														break;
													}

												} // du for i  to NbCouplesToMerge

												if (cur != tempFiles.end()) // il y avait un nombre impair de parties a merger
												{
													tempfiles2.push_back(*cur);;
													tempFiles.pop_back();
												}

												for (TempFileArray::iterator cur = tempFiles.begin(), end = tempFiles.end(); cur != end; cur++)
												{
													VFileStream* fs = *cur;
													const VFile* ff = fs->GetVFile();
													if (ff != nil)
														ff->Delete();
													delete fs;
												}

												tempFiles = tempfiles2;

											} // du while tempFiles.size() > 1

											if (InProgress != nil)
												InProgress->EndSession();

											if (!stop && err == VE_OK)
											{
												if (tempFiles.size() == 0)
												{
													count = 0;
													result = true;
													if (BuildASel && !PourValeursDistinctes)
													{
														sLONG nbnulls = 0;
														if (fNulls != nil)
														{
															nbnulls = fNulls->Compte();
														}
														if (From->IsRemoteLike())
														{
															if ((count+nbnulls)>kNbElemInSel)
															{
																into = new LongSel(nil, db);
																((LongSel*)into)->PutInCache();
															}
															else
															{
																into = new PetiteSel(nil, db);
															}
														}
														else
														{
															if ((count+nbnulls)>kNbElemInSel)
															{
																into = new LongSel(df);
																((LongSel*)into)->PutInCache();
															}
															else
															{
																into = new PetiteSel(df);
															}
														}

														if (!into->FixFic(count+nbnulls))
														{
															err = ThrowBaseError(memfull, DBaction_SortingSelection);
															into->Release();
															into = nil;
														}
														else
														{
															sLONG startsel = 0;
															if (nbnulls != 0 && !criterias.IsAllDescent())
															{
																startsel = nbnulls;
																sLONG curinsel = 0;
																sLONG n = fNulls->FindNextBit(0);
																while (n != -1 && err == VE_OK)
																{
																	err = into->PutFicWithWafSelection(curinsel, n, inWafSelbits, outWafSel);
																	curinsel++;
																	n = fNulls->FindNextBit(n+1);
																}

															}
														}
													}
												}
												else
												{
													xbox_assert(tempFiles.size() == 1);
													filedata = tempFiles[0];
													if (BuildASel && !PourValeursDistinctes)
													{
														sLONG nbnulls = 0;
														if (fNulls != nil)
														{
															nbnulls = fNulls->Compte();
														}
														if (From->IsRemoteLike())
														{
															if ((count+nbnulls)>kNbElemInSel)
															{
																into = new LongSel(nil, db);
																((LongSel*)into)->PutInCache();
															}
															else
															{
																into = new PetiteSel(nil, db);
															}
														}
														else
														{
															if ((count+nbnulls)>kNbElemInSel)
															{
																into = new LongSel(df);
																((LongSel*)into)->PutInCache();
															}
															else
															{
																into = new PetiteSel(df);
															}
														}

														if (into != nil)
														{
															if (filedata != nil)
															{
																err = filedata->OpenReading();
																if (err == VE_OK)
																{
																	filedata->SetBufferSize(512000);
																	sLONG qt = 0;
																	err = filedata->GetLong(qt);
																	if (err == VE_OK)
																	{
																		xbox_assert(count == qt);
																		count = qt;
																		if (!into->FixFic(qt+nbnulls))
																			err = ThrowBaseError(memfull, DBaction_SortingSelection);
																	}
																	if (err == VE_OK)
																	{
																		if (InProgress != nil)
																		{
																			XBOX::VString session_title;
																			gs(1005,14,session_title);	// Building Selection
																			InProgress->BeginSession(NbIterComp,session_title,!pourindex);
																		}

																		sLONG startsel = 0;
																		if (nbnulls != 0 && !criterias.IsAllDescent())
																		{
																			startsel = nbnulls;
																			sLONG curinsel = 0;
																			sLONG n = fNulls->FindNextBit(0);
																			while (n != -1 && err == VE_OK)
																			{
																				err = into->PutFicWithWafSelection(curinsel, n, inWafSelbits, outWafSel);
																				curinsel++;
																				n = fNulls->FindNextBit(n+1);
																			}

																		}

																		for (sLONG i=0; i<qt && err == VE_OK; i++)
																		{
																			sLONG numrec = -1, len = 0;
																			err = filedata->GetLong(numrec);
																			if (err == VE_OK)
																				err = filedata->GetLong(len);
																			
																			if (err == VE_OK)
																				err = into->PutFicWithWafSelection(i+startsel, numrec, inWafSelbits, outWafSel);
																			
																			err = filedata->SetPosByOffset(len);

																			if (InProgress != nil)
																				InProgress->Progress(i);
																			if (MustStopTask(InProgress))
																				err = VE_DB4D_ACTIONCANCELEDBYUSER;
																		}

																		if (err == VE_OK && nbnulls != 0 && criterias.IsAllDescent())
																		{
																			sLONG curqtfic = qt;
																			sLONG n = fNulls->FindNextBit(0);
																			while (n != -1 && err == VE_OK)
																			{
																				err = into->PutFicWithWafSelection(curqtfic, n, inWafSelbits, outWafSel);
																				curqtfic++;
																				n = fNulls->FindNextBit(n+1);
																			}
																		}

																		if (InProgress != nil)
																		{
																			InProgress->EndSession();
																		}
																	}
																	filedata->CloseReading();
																}
																if (err == VE_OK)
																	result = true;
															}
															else
															{
																into->Release();;
																into = nil;
															}
														}
													}
													else
													{
														err = filedata->OpenReading();
														if (err == VE_OK)
														{
															sLONG qt = 0;
															err = filedata->GetLong(qt);
															if (err == VE_OK)
															{
#if debuglr
																if (!PourKeyWords)
																	xbox_assert(count == qt);
#endif
																count = qt;
																result = true;
															}
															else
																filedata->CloseReading();
														}
													}
												
													if (result)
														tempFiles.pop_back();
												}
											}

											for (TempFileArray::iterator cur = tempFiles.begin(), end = tempFiles.end(); cur != end; cur++)
											{
												VFileStream* fs = *cur;
												const VFile* ff = fs->GetVFile();
												if (ff != nil)
													ff->Delete();
												delete fs;
											}

										} // du if not stop

										if (InProgress != nil)
										{
											InProgress->EndSession();
										}

									} // du if not stop

									QuickReleaseRefCountable(remotecache);
									QuickReleaseRefCountable(fieldcache);
									QuickReleaseRefCountable(xFrom);

									if (encapsuleur != nil)
										encapsuleur->Release();
								}
								ReleaseRefCountable( &tempfolder);
							} // du tempfolder != nil							
						}
						FreeTempMem(partdata);
					} // du partdata != nil
				}

			} // fin du cas ou l'on a travaille par blocs sur disque

		}
		if (!BuildASel && !pourindex)
			DF->UnLockTable(context);
	}

	if (ind != nil && !indexalreadyclosed)
	{
		ind->Close();
		indexalreadyclosed = true;
	}

	if (!BuildASel && From != nil)
	{
		From->Release();
	}

	if (err != VE_OK)
		err = ThrowBaseError(VE_DB4D_CANNOT_COMPLETE_SEQ_SORT, DBaction_SortingSelection);
	return result;
}


template <class Type>
void TypeSortElemArray<Type>::GenerePageIndex(OccupableStack* curstack, BTreePageIndex* page, VError &err, sLONG &curelem, sLONG maxlevel,
											 sLONG level, IndexInfo* ind, 
											 sLONG MaxElemsInPage, sLONG RequiredElemsInPage, BaseTaskInfo* context, 
											 VDB4DProgressIndicator* InProgress, sLONG typ, sLONG TypeSize, sLONG maxElemsToCompute, Boolean CheckRemain, Boolean islast)
{
	err = VE_OK;
	sLONG subrequired, submaxtocompute = -1;
	Boolean subcheckremain = false, issublast = false;
	BTreePageIndex *sousBT = nil;
	sLONG nbkey = 0, nbkeytobuild = RequiredElemsInPage;
	if (level == maxlevel)
		nbkeytobuild = MaxElemsInPage;

	//sLONG8 nbelemparlevel = ind->ComputeMaxKeyToFit(level, MaxElemsInPage, RequiredElemsInPage);

	//page = BTreePageIndex::AllocatePage(ind);
	if (InProgress != nil)
		InProgress->Progress(curelem);
	if (MustStopTask(InProgress))
		err = ind->ThrowError(VE_DB4D_ACTIONCANCELEDBYUSER, DBaction_BuildingIndex);

	if (err == VE_OK)
	{
		{
			sLONG nbkeys_in_last = nbkeytobuild, nbkeys_in_antelast = nbkeytobuild;
			sLONG pos_of_last = -1, pos_of_antelast = -1;
			sLONG maxtocompute_in_last = -1, maxtocompute_in_antelast = -1;

			if (level > 1)
			{
				sLONG maxforthislevel = ind->ComputeMaxKeyToFit(level, MaxElemsInPage, MaxElemsInPage, false);
				if (count - curelem < maxforthislevel)
				{
					CheckRemain = true;
					islast = true;
				}
			}

			if (level > 1 && CheckRemain)
			{
				//if (level != maxlevel)
				{
					sLONG maxkeys_level_moins_un = ind->ComputeMaxKeyToFit(level-1, MaxElemsInPage, MaxElemsInPage, false);
					sLONG maxkeys_level_moins_deux = ind->ComputeMaxKeyToFit(level - 2, MaxElemsInPage, MaxElemsInPage, false);
					if (islast)
					{
						maxElemsToCompute = count - curelem;
					}
					sLONG nbkeys = maxElemsToCompute / maxkeys_level_moins_un;
					sLONG maxelemsmoins = maxElemsToCompute-nbkeys;
					nbkeys = maxelemsmoins / maxkeys_level_moins_un;
					sLONG remain = maxElemsToCompute - (maxkeys_level_moins_un * nbkeys + nbkeys);
					xbox_assert(remain >= 0);
					//sLONG nbkey_for_remain = (remain + maxkeys_level_moins_deux - 1) / maxkeys_level_moins_deux;
					sLONG nbkey_for_remain = remain / maxkeys_level_moins_deux;
					if (level > 2)
						nbkey_for_remain = (remain - nbkey_for_remain) / maxkeys_level_moins_deux;
					
					if (nbkey_for_remain < HalfPage)
					{
						nbkeys_in_last = (MaxElemsInPage + nbkey_for_remain) / 2;
						nbkeys_in_antelast = MaxElemsInPage + nbkey_for_remain - nbkeys_in_last;
						pos_of_last = nbkeys;
						pos_of_antelast = pos_of_last - 1;
						sLONG remain2 = remain + maxkeys_level_moins_un;
						maxtocompute_in_antelast = nbkeys_in_antelast * maxkeys_level_moins_deux;
						if (maxkeys_level_moins_deux > 1)
						{
							maxtocompute_in_antelast = maxtocompute_in_antelast 
																					+ nbkeys_in_antelast /* pour le nb de cle dans la page mere */ 
																					+ maxkeys_level_moins_deux /* pour le nombre de cles dans les pages filles de l'element 0*/;
						}
						maxtocompute_in_last = remain2 - maxtocompute_in_antelast;
					}
					nbkeytobuild = (maxElemsToCompute + maxkeys_level_moins_un - 1)/maxkeys_level_moins_un;
				}
			}

			// on va d'abord calculer le nombre de cles a ce niveau
			if (level > 1)
			{
				if (pos_of_antelast == 0)
				{
					subrequired = nbkeys_in_antelast;
					subcheckremain = false;
					submaxtocompute = maxtocompute_in_antelast;
				}
				else
				{
					subrequired = MaxElemsInPage;
					subcheckremain = false;
					submaxtocompute = -1;
				}

				sousBT = ind->GetHeader()->CrePage();
				if (sousBT == nil)
					err = ind->ThrowError(memfull, noaction);
				else
				{
					sousBT->Occupy(curstack, true);
					page->SetSousPage(0, sousBT);
					GenerePageIndex(curstack, sousBT, err, curelem, maxlevel, level-1, ind, MaxElemsInPage, subrequired, context, InProgress, typ, TypeSize, submaxtocompute, subcheckremain, false);
					page->SetSousPage(0, sousBT);
					xbox_assert(sousBT->GetNKeys() >= HalfPage);
					sousBT->Free(curstack, true);
				}
			}

			while (curelem < count && nbkey < nbkeytobuild && err == VE_OK)
			{
				TypeSortElem<Type>* elem = nil;
				sLONG numrec = -1;
				
				if (filedata != nil)
				{
					err = filedata->GetLong(numrec);
					if (err == VE_OK)
					{
						if (auxdata == nil)
						{
							auxdata = GetTempMem(kAuxDataBufferSize, false, 'tAux');
						}
						sLONG len = 0;
						err = filedata->GetLong(len);
						xbox_assert(len > 0 && len < kAuxDataBufferSize);
						if (err == VE_OK)
						{
							err = filedata->GetData(auxdata, len);
						}
					}
					if (err == VE_OK)
					{
						err = page->AddKey(auxdata, numrec, nil); // on ajoute maintenant car auxdata va changer
					}
					if (err != VE_OK)
						break;
				}
				else
				{
					elem = GetDataElem(curelem, TypeSize);

					void* x = &elem->value;
					if (typ == -1) // multiple fields
					{
						if (auxdata == nil)
						{
							auxdata = GetTempMem(kAuxDataBufferSize, false, 'tAux');
						}
						((xMultiFieldData*)(&elem->value))->CopyTo(auxdata);
						x = auxdata;
					}
					else
					{
						if (typ == VK_STRING)
						{
							if (auxdata == nil)
							{
								auxdata = GetTempMem(kAuxDataBufferSize, false, 'tAux');
							}
							((xString*)(&elem->value))->copyto(auxdata);
							x = auxdata;
						}
						else
						{
							if (typ == VK_STRING_UTF8)
							{
								if (auxdata == nil)
								{
									auxdata = GetTempMem(kAuxDataBufferSize, false, 'tAux');
								}
								((xStringUTF8*)(&elem->value))->copyto(auxdata);
								x = auxdata;
							}
						}
					}
					err = page->AddKey(x, elem->recnum, nil);
				}

				curelem++;

				if (level > 1)
				{
					if (pos_of_antelast == nbkey + 1)
					{
						subrequired = nbkeys_in_antelast;
						submaxtocompute = maxtocompute_in_antelast;
						subcheckremain = false;
					}
					else
					{
						if (pos_of_last == nbkey + 1)
						{
							issublast = true;
							subrequired = nbkeys_in_last;
							submaxtocompute = maxtocompute_in_last;
							subcheckremain = true;
						}
						else
						{
							subrequired = MaxElemsInPage;
							subcheckremain = false;
							submaxtocompute = -1;
						}
					}
					sousBT = ind->GetHeader()->CrePage();
					if (sousBT == nil)
					{
						err = ind->ThrowError(memfull, noaction);
						break;
					}
					else
					{
						sousBT->Occupy(curstack, true);
						page->SetSousPage(page->GetNKeys(), sousBT);
						GenerePageIndex(curstack, sousBT, err, curelem, maxlevel, level-1, ind, MaxElemsInPage, subrequired, context, InProgress, typ, TypeSize, submaxtocompute, subcheckremain, issublast);
						page->SetSousPage(page->GetNKeys(), sousBT);
						xbox_assert(sousBT->GetNKeys() >= HalfPage);
						sousBT->Free(curstack, true);
					}
				}

				nbkey++;

			}
		}

		if (err == VE_OK)
		{
			//ind->GetDB()->DelayForFlush();
			err = page->savepage(curstack, context);
		}

	}
		
}


template <class Type>
VError TypeSortElemArray<Type>::GenereIndex(IndexInfo* ind, sLONG MaxElemsInPage, sLONG RequiredElemsInPage, BaseTaskInfo* context, 
											VDB4DProgressIndicator* InProgress, sLONG typ, sLONG TypeSize)
{
	VError err = VE_OK;
	BTreePageIndex* first;
		
	if (count > 0)
	{
		sLONG curelem = 0;

#if debugIndexPage
		ind->Cannot_Debug_AddPage();
#endif

		if (InProgress != nil)
		{
			XBOX::VString session_title;
			gs(1005,15,session_title);	// Generating Index Pages
			InProgress->BeginSession(count,session_title,false);
		}

		ind->Open(index_write);
		OccupableStack* curstack = gOccPool.GetStackForCurrentThread();

		sLONG nblevel = ind->CalculateGenerationsLevels(count, MaxElemsInPage, RequiredElemsInPage);
		first = ind->GetHeader()->CrePage();
		if (first == nil)
		{
			err = ind->ThrowError(memfull, noaction);
			if (filedata != nil)
				filedata->CloseReading();
		}
		else
		{
			first->Occupy(curstack, true);
			IndexHeaderBTree* header = ((IndexHeaderBTree*)(ind->GetHeader()));
			header->SetFirstPage(first, context);
			GenerePageIndex(curstack, first, err, curelem, nblevel, nblevel, ind, MaxElemsInPage, RequiredElemsInPage, context, InProgress, typ, TypeSize, count, true, true);
			header->SetFirstPage(first, context);
			if (err == VE_OK && fNulls != nil && fNulls->Compte() != 0)
				err = header->AddToNulls(curstack, fNulls, context);
			ind->GetHeader()->setmodif(true, ind->GetDB(), context);
			if (filedata != nil)
				filedata->CloseReading();
			if (err != VE_OK)
			{
				// l'index est mal genere, il faut le detruire
				first->LibereEspaceMem(curstack);
				header->SetFirstPage(nil, context);
				first->Free(curstack, true);
				first->Release();
				header->LibereEspaceDisk(curstack, InProgress);
			}
			else
				first->Free(curstack, true);
		}

		ind->Close();

		if (InProgress != nil)
		{
			InProgress->EndSession();
		}

#if debugIndexPage
		ind->Can_Debug_AddPage();
#endif
	}
	else
	{
		OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
		if (fNulls != nil && fNulls->Compte() != 0)
			err = ind->GetHeader()->AddToNulls(curstack, fNulls, context);
		ind->GetHeader()->setmodif(true, ind->GetDB(), context);
	}

	if (err != VE_OK)
		err = ind->ThrowError(VE_DB4D_CANNOT_BUILD_QUICK_INDEX, DBaction_BuildingIndex);

	return err;
}




template <class Type>
void TypeSortElemArray<Type>::GenerePageClusterIndex(OccupableStack* curstack, BTreePageIndex *page, VFileStream* keysdata, VError &err, sLONG &curelem, sLONG &curcluster, sLONG maxlevel,
									 sLONG level, IndexInfo* ind, 
									 sLONG MaxElemsInPage, sLONG RequiredElemsInPage, BaseTaskInfo* context, 
									 VDB4DProgressIndicator* InProgress, sLONG typ, sLONG TypeSize, sLONG maxElemsToCompute, Boolean CheckRemain, Boolean islast)
{
	err = VE_OK;
	sLONG subrequired, submaxtocompute = -1;
	Boolean subcheckremain = false, issublast = false;
	BTreePageIndex *sousBT = nil;
	sLONG nbkey = 0, nbkeytobuild = RequiredElemsInPage;
	if (level == maxlevel)
		nbkeytobuild = MaxElemsInPage;

	//sLONG8 nbelemparlevel = ind->ComputeMaxKeyToFit(level, MaxElemsInPage, RequiredElemsInPage);

	//page = BTreePageIndex::AllocatePage(ind);
	if (InProgress != nil)
		InProgress->Progress(curelem);

	if (MustStopTask(InProgress))
		err = ind->ThrowError(VE_DB4D_ACTIONCANCELEDBYUSER, DBaction_BuildingIndex);

	if (err == VE_OK)
	{
		{
			sLONG nbkeys_in_last = nbkeytobuild, nbkeys_in_antelast = nbkeytobuild;
			sLONG pos_of_last = -1, pos_of_antelast = -1;
			sLONG maxtocompute_in_last = -1, maxtocompute_in_antelast = -1;

			if (level > 1)
			{
				sLONG maxforthislevel = ind->ComputeMaxKeyToFit(level, MaxElemsInPage, MaxElemsInPage, false);
				if (count - curelem < maxforthislevel)
				{
					CheckRemain = true;
					islast = true;
				}
			}

			if (level > 1 && CheckRemain)
			{
				//if (level != maxlevel)
				{
					sLONG maxkeys_level_moins_un = ind->ComputeMaxKeyToFit(level-1, MaxElemsInPage, MaxElemsInPage, false);
					sLONG maxkeys_level_moins_deux = ind->ComputeMaxKeyToFit(level - 2, MaxElemsInPage, MaxElemsInPage, false);
					if (islast)
					{
						maxElemsToCompute = count - curelem;
					}
					sLONG nbkeys = maxElemsToCompute / maxkeys_level_moins_un;
					sLONG maxelemsmoins = maxElemsToCompute-nbkeys;
					nbkeys = maxelemsmoins / maxkeys_level_moins_un;
					sLONG remain = maxElemsToCompute - (maxkeys_level_moins_un * nbkeys + nbkeys);
					xbox_assert(remain >= 0);
					//sLONG nbkey_for_remain = (remain + maxkeys_level_moins_deux - 1) / maxkeys_level_moins_deux;
					sLONG nbkey_for_remain = remain / maxkeys_level_moins_deux;
					if (level > 2)
						nbkey_for_remain = (remain - nbkey_for_remain) / maxkeys_level_moins_deux;

					if (nbkey_for_remain < HalfPage)
					{
						nbkeys_in_last = (MaxElemsInPage + nbkey_for_remain) / 2;
						nbkeys_in_antelast = MaxElemsInPage + nbkey_for_remain - nbkeys_in_last;
						pos_of_last = nbkeys;
						pos_of_antelast = pos_of_last - 1;
						sLONG remain2 = remain + maxkeys_level_moins_un;
						maxtocompute_in_antelast = nbkeys_in_antelast * maxkeys_level_moins_deux;
						if (maxkeys_level_moins_deux > 1)
						{
							maxtocompute_in_antelast = maxtocompute_in_antelast 
								+ nbkeys_in_antelast /* pour le nb de cle dans la page mere */ 
								+ maxkeys_level_moins_deux /* pour le nombre de cles dans les pages filles de l'element 0*/;
						}
						maxtocompute_in_last = remain2 - maxtocompute_in_antelast;
					}
					nbkeytobuild = (maxElemsToCompute + maxkeys_level_moins_un - 1)/maxkeys_level_moins_un;
				}
			}

			// on va d'abord calculer le nombre de cles a ce niveau
			if (level > 1)
			{
				if (pos_of_antelast == 0)
				{
					subrequired = nbkeys_in_antelast;
					subcheckremain = false;
					submaxtocompute = maxtocompute_in_antelast;
				}
				else
				{
					subrequired = MaxElemsInPage;
					subcheckremain = false;
					submaxtocompute = -1;
				}

				sousBT = ind->GetHeader()->CrePage();
				if (sousBT == nil)
				{
					err = ind->ThrowError(memfull, noaction);
				}
				else
				{
					sousBT->Occupy(curstack, true);
					page->SetSousPage(0, sousBT);
					GenerePageClusterIndex(curstack, sousBT, keysdata, err, curelem, curcluster, maxlevel, level-1, ind, MaxElemsInPage, subrequired, context, InProgress, typ, TypeSize, submaxtocompute, subcheckremain, false);
					page->SetSousPage(0, sousBT);
					xbox_assert(sousBT->GetNKeys() >= HalfPage);
					sousBT->Free(curstack, true);
				}
			}

			while (curelem < count && nbkey < nbkeytobuild && err == VE_OK)
			{
				sLONG numrec = -1;

				sLONG len = 0;
				err = keysdata->GetLong(len);
				xbox_assert(len > 0 && len < kAuxDataBufferSize);
				if (err == VE_OK)
				{
					err = keysdata->GetData(auxdata, len);
				}
				sLONG qt = 0;
				if (err == VE_OK)
					err = keysdata->GetLong(qt);
				if (err == VE_OK && qt == 1)
					err = keysdata->GetLong(numrec);
				else
				{
					numrec = -(curcluster+2);
					curcluster++;
				}
				if (err == VE_OK)
				{
					err = page->AddKey(auxdata, numrec, nil); // on ajoute maintenant car auxdata va changer
				}
				if (err != VE_OK)
					break;

				curelem++;

				if (level > 1)
				{
					if (pos_of_antelast == nbkey + 1)
					{
						subrequired = nbkeys_in_antelast;
						submaxtocompute = maxtocompute_in_antelast;
						subcheckremain = false;
					}
					else
					{
						if (pos_of_last == nbkey + 1)
						{
							issublast = true;
							subrequired = nbkeys_in_last;
							submaxtocompute = maxtocompute_in_last;
							subcheckremain = true;
						}
						else
						{
							subrequired = MaxElemsInPage;
							subcheckremain = false;
							submaxtocompute = -1;
						}
					}
					sousBT = ind->GetHeader()->CrePage();
					if (sousBT == nil)
					{
					}
					else
					{
						sousBT->Occupy(curstack, true);
						page->SetSousPage(page->GetNKeys(), sousBT);
						GenerePageClusterIndex(curstack, sousBT, keysdata, err, curelem, curcluster, maxlevel, level-1, ind, MaxElemsInPage, subrequired, context, InProgress, typ, TypeSize, submaxtocompute, subcheckremain, issublast);
						page->SetSousPage(page->GetNKeys(), sousBT);
						xbox_assert(sousBT->GetNKeys() >= HalfPage);
						sousBT->Free(curstack, true);
					}
				}

				nbkey++;

			}
		}

		if (err == VE_OK)
		{
			//ind->GetDB()->DelayForFlush();
			err = page->savepage(curstack, context);
		}

	}
}



template <class Type>
VError TypeSortElemArray<Type>::GenereClusterIndex(IndexInfo* ind, sLONG MaxElemsInPage, sLONG RequiredElemsInPage, BaseTaskInfo* context, 
												VDB4DProgressIndicator* InProgress, sLONG typ, sLONG TypeSize, xMultiFieldDataOffsets& criterias)
{
	VError err = VE_OK;
	BTreePageIndex* first;
#if debugIndexPage
	ind->Cannot_Debug_AddPage();
#endif

	if (count > 0)
	{
		sLONG curelem = 0;
		sLONG nbrealkeys = 0;
		sLONG debugcountsel = 0;

		VFileStream* keysdata = CreateTempFile(ind->GetDB(), nil, 0);
		VFileStream* selsdata = CreateTempFile(ind->GetDB(), nil, 0);
		if (keysdata != nil && selsdata != nil)
		{
			keysdata->OpenWriting();
			keysdata->SetBufferSize(512000);
			selsdata->OpenWriting();
			selsdata->SetBufferSize(512000);
			// premiere partie : regouper les numero de fiche des cles egales 

			//if (filedata != nil)
			{
				if (auxdata == nil)
				{
					auxdata = GetTempMem(kAuxDataBufferSize, false, 'taux');
				}
				if (auxdata2 == nil)
				{
					auxdata2 = GetTempMem(kAuxDataBufferSize, false, 'taux');
				}
				if (auxdata == nil || auxdata2 == nil)
					err = ind->ThrowError(memfull, DBaction_BuildingIndex);
			}

			if (err == VE_OK)
			{
				void* curdata = nil;
				sLONG curlen = 0;
				Bittab* cursel = nil;

				if (InProgress != nil)
				{
					XBOX::VString session_title;
					gs(1005,16,session_title);	// Generating Clusters
					InProgress->BeginSession(count,session_title,false);
				}

				sLONG maxrecord = ind->GetTargetTable()->GetDF()->GetMaxRecords(nil);

				sLONG maxrecordforpetitesel = maxrecord/32;

				for (sLONG i = 0; i<=count && err == VE_OK; i++)
				{
					if (InProgress != nil)
						InProgress->Progress(i);

					if (MustStopTask(InProgress))
					{
						err = VE_DB4D_ACTIONCANCELEDBYUSER;
						break;
					}

					sLONG numrec = -1, len = 0;
					void* curread = nil;

					if (i != count)
					{
						if (curdata == nil || curdata == auxdata2)
							curread = auxdata;
						else
							curread = auxdata2;

						if (filedata != nil)
						{
							err = filedata->GetLong(numrec);
							if (err == VE_OK)
							{
								len = 0;
								err = filedata->GetLong(len);
								xbox_assert(len > 0 && len < kAuxDataBufferSize);
								if (err == VE_OK)
								{
									err = filedata->GetData(curread, len);
								}
							}
						}
						else
						{
							TypeSortElem<Type>* elem = GetDataElem(i, TypeSize);
							numrec = elem->recnum;
							if (typ == -1) // multiple fields
							{
								void* x = ((xMultiFieldData*)(&elem->value))->CopyTo(curread);
								len = ((char*)x) - (char*)curread;
							}
							else
							{
								if (typ == VK_STRING)
								{
									void* x = ((xString*)(&elem->value))->copyto(curread);
									len = ((char*)x) - (char*)curread;
								}
								else
								{
									if (typ == VK_STRING_UTF8)
									{
										void* x = ((xStringUTF8*)(&elem->value))->copyto(curread);
										len = ((char*)x) - (char*)curread;
									}
									else
									{
										curread = &elem->value;
										len = TypeSize;
									}
								}
							}
						}
					}
					else
						len = 0;

					if (err == VE_OK)
					{
						if (curdata == nil || curread == nil || /*curlen != len ||*/ Compare2SortElems(curread, curdata, criterias, fOptions) != CR_EQUAL)
						{
							if (curread != nil )
								nbrealkeys++;
/*
							DebugMsg(L"\n");
							DebugMsg(L"-------------------------------------\n");
*/
							if (curdata != nil)
							{
								err = keysdata->PutLong(curlen);
								if (err == VE_OK)
									err = keysdata->PutData(curdata, curlen);
								if (err == VE_OK)
								{
									cursel->Epure();
									sLONG qt = cursel->Compte();
									xbox_assert(qt > 0);
									err = keysdata->PutLong(qt);
									if (err == VE_OK)
									{
										if (qt == 1)
										{
											err = keysdata->PutLong(cursel->FindNextBit(0));
										}
										else
										{
											debugcountsel++;
											if (qt <= maxrecordforpetitesel)
											{
												err = selsdata->PutLong(sel_petitesel);
												if (err == VE_OK)
													err = selsdata->PutLong(qt);
												if (err == VE_OK)
												{
													sLONG debugcount = 0;
													sLONG x = cursel->FindNextBit(0);
													while (x != -1 && err == VE_OK)
													{
														debugcount++;
														err = selsdata->PutLong(x);
														x = cursel->FindNextBit(x+1);
													}
													xbox_assert(debugcount == qt);
												}
											}
											else
											{
												err = selsdata->PutLong(sel_bitsel);
												if (err == VE_OK)
													err = cursel->PutInto(*selsdata);
											}
										}
									}
								}
								ReleaseRefCountable(&cursel);
							}

							curdata = curread;
							curlen = len;
							if (curread != nil)
							{
								cursel = new Bittab;
								if (cursel == nil)
									err = ind->ThrowError(memfull, DBaction_BuildingIndex);
								else
								{
									err = cursel->aggrandit(maxrecord);
									if (err == VE_OK)
										err = cursel->Set(numrec, true);
								}
							}
						}
						else
							err = cursel->Set(numrec, true);
					}
				}

				if (InProgress != nil)
				{
					InProgress->EndSession();
				}

				ReleaseRefCountable(&cursel);

			}

			keysdata->CloseWriting();
			selsdata->CloseWriting();

			if (err == VE_OK)
			{
				count = nbrealkeys;
				sLONG curcluster = 0;

				keysdata->OpenReading();
				selsdata->OpenReading();

				if (InProgress != nil)
				{
					XBOX::VString session_title;
					gs(1005,15,session_title);	// Generating Index Pages
					InProgress->BeginSession(nbrealkeys,session_title,false);
				}

				ind->Open(index_write);

				OccupableStack* curstack = gOccPool.GetStackForCurrentThread();

				sLONG nblevel = ind->CalculateGenerationsLevels(nbrealkeys, MaxElemsInPage, RequiredElemsInPage);

				first = ind->GetHeader()->CrePage();
				if (first == nil)
				{
					err = ind->ThrowError(memfull, noaction);
				}
				else
				{
					first->Occupy(curstack, true);
					((IndexHeaderBTreeCluster*)(ind->GetHeader()))->SetFirstPage(first, context);
					GenerePageClusterIndex(curstack, first, keysdata, err, curelem, curcluster, nblevel, nblevel, ind, MaxElemsInPage, RequiredElemsInPage, context, InProgress, typ, TypeSize, count, true, true);
					((IndexHeaderBTreeCluster*)(ind->GetHeader()))->SetFirstPage(first, context);
					if (err == VE_OK && fNulls != nil && fNulls->Compte() != 0)
						err = ind->GetHeader()->AddToNulls(curstack, fNulls, context);
					ind->GetHeader()->setmodif(true, ind->GetDB(), context);
					if (err != VE_OK)
					{
						// l'index est mal genere, il faut le detruire
						first->LibereEspaceMem(curstack);
						((IndexHeaderBTreeCluster*)(ind->GetHeader()))->SetFirstPage(nil, context);
						first->Free(curstack, true);
						first->Release();
						first = nil;
						((IndexHeaderBTreeCluster*)(ind->GetHeader()))->LibereEspaceDisk(curstack, InProgress);
					}
				}
				if (InProgress != nil)
				{
					InProgress->EndSession();
				}
				if (first != nil)
				{
					first->Free(curstack, true);

					if (InProgress != nil)
					{
						XBOX::VString session_title;
						gs(1005,18,session_title); // Generating Index Clusters
						InProgress->BeginSession(curcluster,session_title,false);
					}
					for (sLONG i = 0; i < curcluster && err == VE_OK; i++)
					{
						sLONG typsel = 0;
						if (InProgress != nil)
							InProgress->Progress(i);

						if (MustStopTask(InProgress))
							err = VE_DB4D_ACTIONCANCELEDBYUSER;

						if (err == VE_OK)
							err = selsdata->GetLong(typsel);
						if (err == VE_OK)
						{
							if (typsel == sel_petitesel)
							{
								sLONG qt = 0;
								err = selsdata->GetLong(qt);
								if (err == VE_OK)
								{
									PetiteSel* sel = new PetiteSel(ind->GetTargetTable()->GetDF());
									if (sel == nil)
										err = ind->ThrowError(memfull, DBaction_BuildingIndex);
									else
									{
										if (sel->FixFic(qt))
										{
											sLONG qt2 = qt;
											err = selsdata->GetLongs(sel->GetTabMem(), &qt2);
											xbox_assert(qt == qt2);
										}
										else
											err = ind->ThrowError(memfull, DBaction_BuildingIndex);

										if (err == VE_OK)
										{
											err = ind->GetClusterSel(curstack)->AddSel(curstack, sel, context);
										}

										sel->Release();
									}
								}
							}
							else
							{
								if (typsel == sel_bitsel)
								{
									BitSel* sel = new BitSel(ind->GetTargetTable()->GetDF());
									if (sel == nil)
										err = ind->ThrowError(memfull, DBaction_BuildingIndex);
									{
										Bittab* b = sel->GetBittab();
										err = b->GetFrom(*selsdata);
										if (err == VE_OK)
										{
											sel->Touch();
											sel->GetQTfic();
											err = ind->GetClusterSel(curstack)->AddSel(curstack, sel, context);
										}
										sel->Release();
									}
								}
								else
									xbox_assert(false); // wrong selection type
							}
						}
					}

					ind->Close();

					if (InProgress != nil)
					{
						InProgress->EndSession();
					}

					if (err != VE_OK)
					{
						((IndexHeaderBTreeCluster*)(ind->GetHeader()))->LibereEspaceMem(curstack);
						first = nil;
						((IndexHeaderBTreeCluster*)(ind->GetHeader()))->LibereEspaceDisk(curstack, InProgress);
					}
				}
				keysdata->CloseReading();
				selsdata->CloseReading();
			
			}

		}
		else
			err = ind->ThrowError(memfull, DBaction_BuildingIndex);

		if (keysdata != nil)
		{
			const VFile* ff = keysdata->GetVFile();
			if (ff != nil)
				ff->Delete();
			delete keysdata;
		}
		if (selsdata != nil)
		{
			const VFile* ff = selsdata->GetVFile();
			if (ff != nil)
				ff->Delete();
			delete selsdata;
		}

	}
	else
	{
		OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
		if (fNulls != nil && fNulls->Compte() != 0)
			err = ind->GetHeader()->AddToNulls(curstack, fNulls, context);
		ind->GetHeader()->setmodif(true, ind->GetDB(), context);
	}


	if (filedata != nil)
		filedata->CloseReading();

#if debugIndexPage
	ind->Can_Debug_AddPage();
#endif

	if (err != VE_OK)
		err = ind->ThrowError(VE_DB4D_CANNOT_BUILD_QUICK_INDEX, DBaction_BuildingIndex);

	return err;
}


template <class Type>
VError TypeSortElemArray<Type>::BuildDistinctKeys(DB4DCollectionManager& outCollection, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, sLONG typ, sLONG TypeSize, const VCompareOptions& inOptions)
{
	VError err = VE_OK;
	sLONG curvaluepos = 0;
	TypeSortElem<Type> *curvalueptr = nil;
	TypeSortElem<Type> *iptr = (TypeSortElem<Type>*)data;

	CreVValue_Code Code = FindCV(typ);
	if (Code == nil)
		err = ThrowBaseError(VE_DB4D_FIELDDEFCODEMISSING, DBaction_BuildingDistinctKeys);
	else
	{
		if (filedata != nil)
		{
			if (!filedata->IsReading())
			{
				filedata->OpenReading();
			}
			filedata->SetBufferSize(512000);
			filedata->SetPos(0);
			void* aux1 = GetTempMem(kAuxDataBufferSize, false, 'tAux');
			void* aux2 = GetTempMem(kAuxDataBufferSize, false, 'tAux');
			sLONG qt = 0;
			err = filedata->GetLong(qt);

			void* prevxp = nil;
			void* xp = aux1;

			xMultiFieldDataOffsets criterias(nil, typ, true, TypeSize);

			for (sLONG i = 0; (i < qt) && (err == VE_OK); i++)
			{
				sLONG numrec = -1, len = 0;
				err = filedata->GetLong(numrec);
				if (err == VE_OK)
					err = filedata->GetLong(len);
				if (err == VE_OK)
					err = filedata->GetData(xp, len);
				if (err == VE_OK)
				{
					bool mustadd = false;

					if (prevxp == nil)
						mustadd = true;
					else
					{
						CompareResult result = Compare2SortElems(xp, prevxp, criterias, fOptions);
						if (result != CR_EQUAL)
							mustadd = true;
					}
					if (mustadd)
					{
						ValPtr cv = (*Code)(nil,-1, xp, false, false, NULL, creVValue_default);
						if (cv == nil)
							err = ThrowBaseError(memfull, DBaction_BuildingDistinctKeys);
						else
						{
							err = outCollection.AddOneElement(1, *cv);
							curvaluepos++;
							delete cv;
						}
						
					}
					if (xp == aux1)
					{
						prevxp = aux1;
						xp = aux2;
					}
					else
					{
						prevxp = aux2;
						xp = aux1;
					}
				}
			}

			FreeTempMem(aux1);
			FreeTempMem(aux2);
			filedata->CloseReading();
		}
		else
		{
		sLONG i;
		for (i = 0; (i < count) && (err == VE_OK); i++)
		{
			Boolean mustadd = false;
			if (curvalueptr == nil)
			{
				mustadd = true;
				curvalueptr = iptr;
			}
			else
			{
				if (typ == VK_REAL)
				{
					if (VReal::sInfo.CompareTwoPtrToDataWithOptions(&curvalueptr->value, &iptr->value, inOptions) != CR_EQUAL)
					{
						mustadd = true;
						curvalueptr = iptr;
					}
				}
				else
				{
					if (curvalueptr->value != iptr->value)
					{
						mustadd = true;
						curvalueptr = iptr;
					}
				}
			}
			if (mustadd)
			{
				ValPtr cv = nil;
				{
					cv = (*Code)(nil,-1, &curvalueptr->value, false, false, NULL, creVValue_default);
					if (cv == nil)
						err = ThrowBaseError(memfull, DBaction_BuildingDistinctKeys);
					else
					{
						err = outCollection.AddOneElement(1, *cv);
						curvaluepos++;
						delete cv;
					}
				}


			}
			iptr = NextDataElem(iptr, TypeSize);
		}
		}
	}

	if (err != VE_OK)
	{
		err = ThrowBaseError(VE_DB4D_CANNOT_BUILD_DISTINCT_KEYS, DBaction_BuildingDistinctKeys);
	}

	return err;
}


template <>
VError TypeSortElemArray<xString>::BuildDistinctKeys(DB4DCollectionManager& outCollection, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, sLONG typ, sLONG TypeSize, const VCompareOptions& inOptions)
{
	VError err = VE_OK;
	sLONG curvaluepos = 0;
	TypeSortElem<xString> *curvalueptr = nil;
	TypeSortElem<xString> *iptr = (TypeSortElem<xString>*)data;
	VString s;

	CreVValue_Code Code = FindCV(typ);
	if (Code == nil)
		err = ThrowBaseError(VE_DB4D_FIELDDEFCODEMISSING, DBaction_BuildingDistinctKeys);
	else
	{
		if (filedata != nil)
		{
			if (!filedata->IsReading())
			{
				filedata->OpenReading();
			}
			filedata->SetBufferSize(512000);
			filedata->SetPos(0);
			void* aux1 = GetTempMem(kAuxDataBufferSize, false, 'tAux');
			void* aux2 = GetTempMem(kAuxDataBufferSize, false, 'tAux');
			sLONG qt = 0;
			err = filedata->GetLong(qt);

			void* prevxp = nil;
			void* xp = aux1;

			xMultiFieldDataOffsets criterias(nil, typ, true, TypeSize);

			for (sLONG i = 0; (i < qt) && (err == VE_OK); i++)
			{
				sLONG numrec = -1, len = 0;
				err = filedata->GetLong(numrec);
				if (err == VE_OK)
					err = filedata->GetLong(len);
				if (err == VE_OK)
					err = filedata->GetData(xp, len);
				if (err == VE_OK)
				{
					bool mustadd = false;

					if (prevxp == nil)
						mustadd = true;
					else
					{
						CompareResult result = Compare2SortElems(xp, prevxp, criterias, fOptions);
						if (result != CR_EQUAL)
							mustadd = true;
					}
					if (mustadd)
					{
						ValPtr cv = (*Code)(nil,-1, xp, false, false, NULL, creVValue_default);
						if (cv == nil)
							err = ThrowBaseError(memfull, DBaction_BuildingDistinctKeys);
						else
						{
							err = outCollection.AddOneElement(1, *cv);
							curvaluepos++;
							delete cv;
						}

					}
					if (xp == aux1)
					{
						prevxp = aux1;
						xp = aux2;
					}
					else
					{
						prevxp = aux2;
						xp = aux1;
					}
				}
			}

			FreeTempMem(aux1);
			FreeTempMem(aux2);
			filedata->CloseReading();
		}
		else
		{
		sLONG i;
		for (i = 0; (i < count) && (err == VE_OK); i++)
		{
			Boolean mustadd = false;
			if (curvalueptr == nil)
			{
				mustadd = true;
				curvalueptr = iptr;
			}
			else
			{
				if (curvalueptr->value.CompareTo(iptr->value, inOptions) != CR_EQUAL)
				{
					mustadd = true;
					curvalueptr = iptr;
				}
			}
			if (mustadd)
			{
				err = iptr->value.CopyToVString(s);
				if (err == VE_OK)
				{
					err = outCollection.AddOneElement(1, s);
					curvaluepos++;
				}

			}
			iptr = NextDataElem(iptr, TypeSize);
		}
		}
	}

	if (err != VE_OK)
	{
		err = ThrowBaseError(VE_DB4D_CANNOT_BUILD_DISTINCT_KEYS, DBaction_BuildingDistinctKeys);
	}

	return err;
}



template <>
VError TypeSortElemArray<xStringUTF8>::BuildDistinctKeys(DB4DCollectionManager& outCollection, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, sLONG typ, sLONG TypeSize, const VCompareOptions& inOptions)
{
	VError err = VE_OK;
	sLONG curvaluepos = 0;
	TypeSortElem<xStringUTF8> *curvalueptr = nil;
	TypeSortElem<xStringUTF8> *iptr = (TypeSortElem<xStringUTF8>*)data;
	VString s;

	CreVValue_Code Code = FindCV(typ);
	if (Code == nil)
		err = ThrowBaseError(VE_DB4D_FIELDDEFCODEMISSING, DBaction_BuildingDistinctKeys);
	else
	{
		if (filedata != nil)
		{
			if (!filedata->IsReading())
			{
				filedata->OpenReading();
			}
			filedata->SetBufferSize(512000);
			filedata->SetPos(0);
			void* aux1 = GetTempMem(kAuxDataBufferSize, false, 'tAux');
			void* aux2 = GetTempMem(kAuxDataBufferSize, false, 'tAux');
			sLONG qt = 0;
			err = filedata->GetLong(qt);

			void* prevxp = nil;
			void* xp = aux1;

			xMultiFieldDataOffsets criterias(nil, typ, true, TypeSize);

			for (sLONG i = 0; (i < qt) && (err == VE_OK); i++)
			{
				sLONG numrec = -1, len = 0;
				err = filedata->GetLong(numrec);
				if (err == VE_OK)
					err = filedata->GetLong(len);
				if (err == VE_OK)
					err = filedata->GetData(xp, len);
				if (err == VE_OK)
				{
					bool mustadd = false;

					if (prevxp == nil)
						mustadd = true;
					else
					{
						CompareResult result = Compare2SortElems(xp, prevxp, criterias, fOptions);
						if (result != CR_EQUAL)
							mustadd = true;
					}
					if (mustadd)
					{
						ValPtr cv = (*Code)(nil,-1, xp, false, false, NULL, creVValue_default);
						if (cv == nil)
							err = ThrowBaseError(memfull, DBaction_BuildingDistinctKeys);
						else
						{
							err = outCollection.AddOneElement(1, *cv);
							curvaluepos++;
							delete cv;
						}

					}
					if (xp == aux1)
					{
						prevxp = aux1;
						xp = aux2;
					}
					else
					{
						prevxp = aux2;
						xp = aux1;
					}
				}
			}

			FreeTempMem(aux1);
			FreeTempMem(aux2);
			filedata->CloseReading();
		}
		else
		{
		sLONG i;
		for (i = 0; (i < count) && (err == VE_OK); i++)
		{
			Boolean mustadd = false;
			if (curvalueptr == nil)
			{
				mustadd = true;
				curvalueptr = iptr;
			}
			else
			{
				if (curvalueptr->value.CompareTo(iptr->value, inOptions) != CR_EQUAL)
				{
					mustadd = true;
					curvalueptr = iptr;
				}
			}
			if (mustadd)
			{
				err = iptr->value.CopyToVString(s);
				if (err == VE_OK)
				{
					err = outCollection.AddOneElement(1, s);
					curvaluepos++;
				}

			}
			iptr = NextDataElem(iptr, TypeSize);
		}
		}
	}

	if (err != VE_OK)
	{
		err = ThrowBaseError(VE_DB4D_CANNOT_BUILD_DISTINCT_KEYS, DBaction_BuildingDistinctKeys);
	}

	return err;
}



template <class Type>
VError TypeSortElemArray<Type>::QuickBuildDistinctKeys(QuickDB4DArrayOfValues *outCollection, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, sLONG typ, sLONG TypeSize, const VCompareOptions& inOptions)
{
	VError err = VE_OK;
	sLONG curvaluepos = 0;
	TypeSortElem<Type> *curvalueptr = nil;
	TypeSortElem<Type> *iptr = (TypeSortElem<Type>*)data;

	if (filedata != nil)
	{
		if (!filedata->IsReading())
		{
			filedata->OpenReading();
		}
		filedata->SetBufferSize(512000);
		filedata->SetPos(0);
		void* aux1 = GetTempMem(kAuxDataBufferSize, false, 'tAux');
		void* aux2 = GetTempMem(kAuxDataBufferSize, false, 'tAux');
		sLONG qt = 0;
		err = filedata->GetLong(qt);

		void* prevxp = nil;
		void* xp = aux1;

		xMultiFieldDataOffsets criterias(nil, typ, true, TypeSize);

		for (sLONG i = 0; (i < qt) && (err == VE_OK); i++)
		{
			sLONG numrec = -1, len = 0;
			err = filedata->GetLong(numrec);
			if (err == VE_OK)
				err = filedata->GetLong(len);
			if (err == VE_OK)
				err = filedata->GetData(xp, len);
			if (err == VE_OK)
			{
				bool mustadd = false;

				if (prevxp == nil)
					mustadd = true;
				else
				{
					CompareResult result = Compare2SortElems(xp, prevxp, criterias, fOptions);
					if (result != CR_EQUAL)
						mustadd = true;
				}
				if (mustadd)
				{
					if (! ((DB4DArrayOfDirectValues<Type>*)outCollection)->AddOneElem(*((Type*)xp)))
						err = ThrowBaseError(memfull, noaction);
					curvaluepos++;
				}
				if (xp == aux1)
				{
					prevxp = aux1;
					xp = aux2;
				}
				else
				{
					prevxp = aux2;
					xp = aux1;
				}
			}
		}

		FreeTempMem(aux1);
		FreeTempMem(aux2);
		filedata->CloseReading();
	}
	else
	{
		sLONG i;
		for (i = 0; (i < count) && (err == VE_OK); i++)
		{
			Boolean mustadd = false;
			if (curvalueptr == nil)
			{
				mustadd = true;
				curvalueptr = iptr;
			}
			else
			{
				if (typ == VK_REAL)
				{
					if (VReal::sInfo.CompareTwoPtrToDataWithOptions(&curvalueptr->value, &iptr->value, inOptions) != CR_EQUAL)
					{
						mustadd = true;
						curvalueptr = iptr;
					}
				}
				else
				{
					if (curvalueptr->value != iptr->value)
					{
						mustadd = true;
						curvalueptr = iptr;
					}
				}
			}
			if (mustadd)
			{
				if (! ((DB4DArrayOfDirectValues<Type>*)outCollection)->AddOneElem(curvalueptr->value))
					err = ThrowBaseError(memfull, noaction);
				curvaluepos++;
			}
			iptr = NextDataElem(iptr, TypeSize);
		}
	}

	if (err != VE_OK)
	{
		err = ThrowBaseError(VE_DB4D_CANNOT_BUILD_DISTINCT_KEYS, DBaction_BuildingDistinctKeys);
	}

	return err;
}


template <>
VError TypeSortElemArray<xString>::QuickBuildDistinctKeys(QuickDB4DArrayOfValues *outCollection, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, sLONG typ, sLONG TypeSize, const VCompareOptions& inOptions)
{
	VError err = VE_OK;
	sLONG curvaluepos = 0;
	TypeSortElem<xString> *curvalueptr = nil;
	TypeSortElem<xString> *iptr = (TypeSortElem<xString>*)data;
	VString s;

	if (filedata != nil)
	{
		if (!filedata->IsReading())
		{
			filedata->OpenReading();
		}
		filedata->SetBufferSize(512000);
		filedata->SetPos(0);
		void* aux1 = GetTempMem(kAuxDataBufferSize, false, 'tAux');
		void* aux2 = GetTempMem(kAuxDataBufferSize, false, 'tAux');
		sLONG qt = 0;
		err = filedata->GetLong(qt);

		void* prevxp = nil;
		void* xp = aux1;

		xMultiFieldDataOffsets criterias(nil, typ, true, TypeSize);

		for (sLONG i = 0; (i < qt) && (err == VE_OK); i++)
		{
			sLONG numrec = -1, len = 0;
			err = filedata->GetLong(numrec);
			if (err == VE_OK)
				err = filedata->GetLong(len);
			if (err == VE_OK)
				err = filedata->GetData(xp, len);
			if (err == VE_OK)
			{
				bool mustadd = false;

				if (prevxp == nil)
					mustadd = true;
				else
				{
					CompareResult result = Compare2SortElems(xp, prevxp, criterias, fOptions);
					if (result != CR_EQUAL)
						mustadd = true;
				}
				if (mustadd)
				{
					xString xs;
					xs.CopyFrom(xp);
					if ( !((DB4DArrayOfDirectValues<QuickString>*)outCollection)->AddOneElemPtrForXString(xs))
						err = ThrowBaseError(memfull, noaction);

					curvaluepos++;
				}
				if (xp == aux1)
				{
					prevxp = aux1;
					xp = aux2;
				}
				else
				{
					prevxp = aux2;
					xp = aux1;
				}
			}
		}

		FreeTempMem(aux1);
		FreeTempMem(aux2);
		filedata->CloseReading();
	}
	else
	{
		sLONG i;
		for (i = 0; (i < count) && (err == VE_OK); i++)
		{
			Boolean mustadd = false;
			if (curvalueptr == nil)
			{
				mustadd = true;
				curvalueptr = iptr;
			}
			else
			{
				if (curvalueptr->value.CompareTo(iptr->value, inOptions) != CR_EQUAL)
				{
					mustadd = true;
					curvalueptr = iptr;
				}
			}
			if (mustadd)
			{
				if ( !((DB4DArrayOfDirectValues<QuickString>*)outCollection)->AddOneElemPtrForXString(curvalueptr->value))
					err = ThrowBaseError(memfull, noaction);

				curvaluepos++;
			}
			iptr = NextDataElem(iptr, TypeSize);
		}
	}

	if (err != VE_OK)
	{
		err = ThrowBaseError(VE_DB4D_CANNOT_BUILD_DISTINCT_KEYS, DBaction_BuildingDistinctKeys);
	}

	return err;
}


template <>
VError TypeSortElemArray<xStringUTF8>::QuickBuildDistinctKeys(QuickDB4DArrayOfValues *outCollection, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, sLONG typ, sLONG TypeSize, const VCompareOptions& inOptions)
{
	VError err = VE_OK;
	sLONG curvaluepos = 0;
	TypeSortElem<xStringUTF8> *curvalueptr = nil;
	TypeSortElem<xStringUTF8> *iptr = (TypeSortElem<xStringUTF8>*)data;
	VString s;

	if (filedata != nil)
	{
		if (!filedata->IsReading())
		{
			filedata->OpenReading();
		}
		filedata->SetBufferSize(512000);
		filedata->SetPos(0);
		void* aux1 = GetTempMem(kAuxDataBufferSize, false, 'tAux');
		void* aux2 = GetTempMem(kAuxDataBufferSize, false, 'tAux');
		sLONG qt = 0;
		err = filedata->GetLong(qt);

		void* prevxp = nil;
		void* xp = aux1;

		xMultiFieldDataOffsets criterias(nil, typ, true, TypeSize);

		for (sLONG i = 0; (i < qt) && (err == VE_OK); i++)
		{
			sLONG numrec = -1, len = 0;
			err = filedata->GetLong(numrec);
			if (err == VE_OK)
				err = filedata->GetLong(len);
			if (err == VE_OK)
				err = filedata->GetData(xp, len);
			if (err == VE_OK)
			{
				bool mustadd = false;

				if (prevxp == nil)
					mustadd = true;
				else
				{
					CompareResult result = Compare2SortElems(xp, prevxp, criterias, fOptions);
					if (result != CR_EQUAL)
						mustadd = true;
				}
				if (mustadd)
				{
					xStringUTF8 xs;
					xs.CopyFrom(xp);
					if ( !((DB4DArrayOfDirectValues<QuickString>*)outCollection)->AddOneElemPtrForXString(xs))
						err = ThrowBaseError(memfull, noaction);

					curvaluepos++;
				}
				if (xp == aux1)
				{
					prevxp = aux1;
					xp = aux2;
				}
				else
				{
					prevxp = aux2;
					xp = aux1;
				}
			}
		}

		FreeTempMem(aux1);
		FreeTempMem(aux2);
		filedata->CloseReading();
	}
	else
	{
		sLONG i;
		for (i = 0; (i < count) && (err == VE_OK); i++)
		{
			Boolean mustadd = false;
			if (curvalueptr == nil)
			{
				mustadd = true;
				curvalueptr = iptr;
			}
			else
			{
				if (curvalueptr->value.CompareTo(iptr->value, inOptions) != CR_EQUAL)
				{
					mustadd = true;
					curvalueptr = iptr;
				}
			}
			if (mustadd)
			{
				if ( !((DB4DArrayOfDirectValues<QuickString>*)outCollection)->AddOneElemPtrForXString(curvalueptr->value))
					err = ThrowBaseError(memfull, noaction);

				curvaluepos++;
			}
			iptr = NextDataElem(iptr, TypeSize);
		}
	}

	if (err != VE_OK)
	{
		err = ThrowBaseError(VE_DB4D_CANNOT_BUILD_DISTINCT_KEYS, DBaction_BuildingDistinctKeys);
	}

	return err;
}



Boolean Selection::TryToSortLong(VError& err, Field* cri, Boolean ascent, BaseTaskInfo* context, 
									VDB4DProgressIndicator* InProgress, Selection* &into, sLONG typ, Boolean TestUnicite, Bittab* inWafSelbits, WafSelection* outWafSel)
{
	Boolean result = true;
	TypeSortElemArray<sLONG> tempsort;
	tempsort.inWafSelbits = inWafSelbits;
	tempsort.outWafSel = outWafSel;

	xMultiFieldDataOffsets off(cri, typ, ascent, sizeof(sLONG));
	result = tempsort.TryToSort(this, into, err, off, context, InProgress, TestUnicite, sizeof(sLONG));
	
	return result;
}


Boolean Selection::TryToSortTime(VError& err, Field* cri, Boolean ascent, BaseTaskInfo* context, 
									VDB4DProgressIndicator* InProgress, Selection* &into, sLONG typ, Boolean TestUnicite, Bittab* inWafSelbits, WafSelection* outWafSel)
{
	Boolean result = true;
	TypeSortElemArray<xTime> tempsort;
	tempsort.inWafSelbits = inWafSelbits;
	tempsort.outWafSel = outWafSel;

	xMultiFieldDataOffsets off(cri, typ, ascent, sizeof(xTime));
	result = tempsort.TryToSort(this, into, err, off, context, InProgress, TestUnicite, sizeof(xTime));

	return result;
}


Boolean Selection::TryToSortLong8(VError& err, Field* cri, Boolean ascent, BaseTaskInfo* context, 
									VDB4DProgressIndicator* InProgress, Selection* &into, sLONG typ, Boolean TestUnicite, Bittab* inWafSelbits, WafSelection* outWafSel)
{
	Boolean result = false;
	TypeSortElemArray<sLONG8> tempsort;
	tempsort.inWafSelbits = inWafSelbits;
	tempsort.outWafSel = outWafSel;

	xMultiFieldDataOffsets off(cri, typ, ascent, sizeof(sLONG8));
	result = tempsort.TryToSort(this, into, err, off, context, InProgress, TestUnicite, sizeof(sLONG8));

	return result;
}


Boolean Selection::TryToSortuLong8(VError& err, Field* cri, Boolean ascent, BaseTaskInfo* context, 
								  VDB4DProgressIndicator* InProgress, Selection* &into, sLONG typ, Boolean TestUnicite, Bittab* inWafSelbits, WafSelection* outWafSel)
{
	Boolean result = false;
	TypeSortElemArray<uLONG8> tempsort;
	tempsort.inWafSelbits = inWafSelbits;
	tempsort.outWafSel = outWafSel;

	xMultiFieldDataOffsets off(cri, typ, ascent, sizeof(uLONG8));
	result = tempsort.TryToSort(this, into, err, off, context, InProgress, TestUnicite, sizeof(uLONG8));

	return result;
}


Boolean Selection::TryToSortShort(VError& err, Field* cri, Boolean ascent, BaseTaskInfo* context, 
									VDB4DProgressIndicator* InProgress, Selection* &into, sLONG typ, Boolean TestUnicite, Bittab* inWafSelbits, WafSelection* outWafSel)
{
	Boolean result = false;
	TypeSortElemArray<sWORD> tempsort;
	tempsort.inWafSelbits = inWafSelbits;
	tempsort.outWafSel = outWafSel;

	xMultiFieldDataOffsets off(cri, typ, ascent, sizeof(sWORD));
	result = tempsort.TryToSort(this, into, err, off, context, InProgress, TestUnicite, sizeof(sWORD));

	return result;
}


Boolean Selection::TryToSortBoolean(VError& err, Field* cri, Boolean ascent, BaseTaskInfo* context, 
										VDB4DProgressIndicator* InProgress, Selection* &into, sLONG typ, Boolean TestUnicite, Bittab* inWafSelbits, WafSelection* outWafSel)
{
	Boolean result = false;
	TypeSortElemArray<uCHAR> tempsort;
	tempsort.inWafSelbits = inWafSelbits;
	tempsort.outWafSel = outWafSel;

	xMultiFieldDataOffsets off(cri, typ, ascent, sizeof(uCHAR));
	result = tempsort.TryToSort(this, into, err, off, context, InProgress, TestUnicite, sizeof(uCHAR));

	return result;
}



Boolean Selection::TryToSortByte(VError& err, Field* cri, Boolean ascent, BaseTaskInfo* context, 
									VDB4DProgressIndicator* InProgress, Selection* &into, sLONG typ, Boolean TestUnicite, Bittab* inWafSelbits, WafSelection* outWafSel)
{
	Boolean result = false;
	TypeSortElemArray<sBYTE> tempsort;
	tempsort.inWafSelbits = inWafSelbits;
	tempsort.outWafSel = outWafSel;

	xMultiFieldDataOffsets off(cri, typ, ascent, sizeof(uCHAR));
	result = tempsort.TryToSort(this, into, err, off, context, InProgress, TestUnicite, sizeof(uCHAR));

	return result;
}

Boolean Selection::TryToSortReal(VError& err, Field* cri, Boolean ascent, BaseTaskInfo* context, 
									VDB4DProgressIndicator* InProgress, Selection* &into, sLONG typ, Boolean TestUnicite, Bittab* inWafSelbits, WafSelection* outWafSel)
{
	Boolean result = false;
	TypeSortElemArray<Real> tempsort;
	tempsort.inWafSelbits = inWafSelbits;
	tempsort.outWafSel = outWafSel;

	xMultiFieldDataOffsets off(cri, typ, ascent, sizeof(Real));
	result = tempsort.TryToSort(this, into, err, off, context, InProgress, TestUnicite, sizeof(Real));

	return result;
}


Boolean Selection::TryToSortUUID(VError& err, Field* cri, Boolean ascent, BaseTaskInfo* context, 
								 VDB4DProgressIndicator* InProgress, Selection* &into, sLONG typ, Boolean TestUnicite, Bittab* inWafSelbits, WafSelection* outWafSel)
{
	Boolean result = false;
	TypeSortElemArray<VUUIDBuffer> tempsort;
	tempsort.inWafSelbits = inWafSelbits;
	tempsort.outWafSel = outWafSel;
	
	xMultiFieldDataOffsets off(cri, typ, ascent, sizeof(VUUIDBuffer));
	result = tempsort.TryToSort(this, into, err, off, context, InProgress, TestUnicite, sizeof(VUUIDBuffer));
	
	return result;
}



Boolean Selection::TryToSortAlpha(VError& err, Field* cri, Boolean ascent, BaseTaskInfo* context, 
									VDB4DProgressIndicator* InProgress, Selection* &into, sLONG typ, Boolean TestUnicite, Bittab* inWafSelbits, WafSelection* outWafSel)
{

	Boolean result = false;
	
	TypeSortElemArray<xString> tempsort;
	tempsort.inWafSelbits = inWafSelbits;
	tempsort.outWafSel = outWafSel;

	sLONG lenmax = cri->GetMaxLenForSort()+xString::NoDataSize();

	xMultiFieldDataOffsets off(cri, typ, ascent, lenmax);
	result = tempsort.TryToSort(this, into, err, off, context, InProgress, TestUnicite, lenmax);
	
	FreeXString(tempsort, lenmax);

	return result;
}


Boolean Selection::TryToSortAlphaUTF8(VError& err, Field* cri, Boolean ascent, BaseTaskInfo* context, 
								  VDB4DProgressIndicator* InProgress, Selection* &into, sLONG typ, Boolean TestUnicite, Bittab* inWafSelbits, WafSelection* outWafSel)
{

	Boolean result = false;

	TypeSortElemArray<xStringUTF8> tempsort;
	tempsort.inWafSelbits = inWafSelbits;
	tempsort.outWafSel = outWafSel;

	sLONG lenmax = cri->GetMaxLenForSort()+xStringUTF8::NoDataSize();

	xMultiFieldDataOffsets off(cri, typ, ascent, lenmax);
	result = tempsort.TryToSort(this, into, err, off, context, InProgress, TestUnicite, lenmax);

	FreeXStringUTF8(tempsort, lenmax);

	return result;
}


Boolean Selection::TryToSortMulti(VError& err, const SortTab* tabs, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, Selection* &into, 
								  Boolean TestUnicite, Table* remoteTable, Bittab* inWafSelbits, WafSelection* outWafSel)
{
	Boolean result = false, okallfields = true;
	sLONG i,nb = tabs->GetNbLine();
	Table* target;
	if (remoteTable != nil)
		target = RetainRefCountable(remoteTable);
	else
		target = parentfile->RetainTable();
	TypeSortElemArray<xMultiFieldData> tempsort;
	tempsort.inWafSelbits = inWafSelbits;
	tempsort.outWafSel = outWafSel;
	xMultiFieldDataOffsets off(target);
	err = VE_OK;
	sLONG ntarget = target == nil ? 0 : target->GetNum(), totlen = 0;

	for (i=1; i<=nb && okallfields; i++)
	{
		const SortLine* li = tabs->GetTriLineRef(i);
		if (li->isfield)
		{
			if (ntarget == li->numfile || li->numfile == 0)
			{
				Field* cri = nil;
				if (target != nil)
					cri = target->RetainField(li->numfield);
				if (cri == nil)
				{
					okallfields = false;
				}
				else
				{
					sLONG typ = cri->GetTyp();
					sLONG lenmax;
					switch(typ) 
					{
						case VK_TEXT:
						case VK_STRING:
							typ = VK_STRING;
							lenmax = cri->GetMaxLenForSort() + xString::NoDataSize();
							break;

						case VK_TEXT_UTF8:
						case VK_STRING_UTF8:
							typ = VK_STRING_UTF8;
							lenmax = cri->GetMaxLenForSort() + xStringUTF8::NoDataSize();
							break;

						case VK_TIME:
							lenmax = sizeof(xTime);
							break;

						case VK_UUID:
							lenmax = sizeof(VUUIDBuffer);
							break;
							
						case VK_LONG:
							lenmax = sizeof(sLONG);
							break;

						case VK_WORD:
							lenmax = sizeof(sWORD);
							break;

						case VK_BYTE:
						case VK_BOOLEAN:
							lenmax = sizeof(uCHAR);
							break;

						case VK_REAL:
							lenmax = sizeof(Real);
							break;

						case VK_LONG8:
						case VK_DURATION:
						case VK_SUBTABLE:
						case VK_SUBTABLE_KEY:
							lenmax = sizeof(sLONG8);
							break;

						default:
							lenmax = 0;
							xbox_assert(false);
							break;
					}

					if (okallfields)
					{
						off.AddOffset(totlen, typ, li->numfield, lenmax, li->ascendant);
						totlen = totlen + lenmax;
					}
					cri->Release();
				}
			}
			else
				okallfields = false;
		}
		else
		{
			if (li->fAttPath != nil)
			{
				totlen += off.AddOffset(totlen, li->fAttPath, li->ascendant);
			}
			else if (li->att != nil)
			{
				totlen += off.AddOffset(totlen, (db4dEntityAttribute*)(li->att), li->ascendant);
			}
			else
			{
				off.AddOffset(li->expression, li->ascendant);
			}
		}
	}

	totlen += xMultiFieldData::GetEmptySize();
	if (okallfields)
	{
		result = tempsort.TryToSort(this, into, err, off, context, InProgress, TestUnicite, totlen);

		FreeXMulti(tempsort, totlen, off);
	}

	if (target != nil)
		target->Release();
	return result;
}







					/*  --------------------------------------------------------- */


void SortElem::Activate(sLONG n, SortContext* xtri)
{
	rec = nil;
	tri = xtri;
	numrec = n;

	sLONG i,nb = tri->fVals.GetCount();
	if (nb>vals.GetCount())
	{
		vals.AddNSpaces(nb - vals.GetCount(), true);
	}
	for (i = 1; i<=nb; i++)
	{
			SortCol* col = tri->fVals[i];
			vals[i] = (*col)[n];
	}
}


void SortElem::DeActivate()
{
		if (rec != nil)
		{
			rec->Release();
			rec = nil;
		}
}


ValPtr SortElem::GetNthValue(sLONG n, VError& err)
{
	ValPtr res = nil;

	if (n > vals.GetCount())
	{
		SortLine *li = tri->st->GetTriLineRef(n);
		sLONG numfield = li->numfield;
		sLONG nn = tri->fSel->GetFic(numrec);
		if (nn!=-1)
		{
			if (rec == nil)
			{
				rec=tri->fSel->GetParentFile()->LoadRecord(nn, err, DB4D_Do_Not_Lock, tri->context, false);
				if (rec != nil)
				{
					if (li->numfile == nn || li->numfile == 0)
						res = rec->GetNthField(numfield, err, false, true);
					else
					{
						RelationPath* rp = tri->RelPaths[n-1];
						if (rp != nil)
						{
							FicheInMem* dest = nil;
							err = rp->ActivateRelation(rec, dest, tri->context, true);
							if (dest != nil)
							{
								res = dest->GetNthField(numfield, err, false, true);
								dest->Release();
							}
							else
							{
								Table* xtab = tri->fSel->GetParentFile()->GetDB()->RetainTable(li->numfile);
								if (xtab != nil)
								{
									Field* xcri = xtab->RetainField(numfield);
									if (xcri != nil)
									{
										res = const_cast<VValueSingle*>(xcri->GetEmptyValue());
										xcri->Release();
									}
									xtab->Release();
								}
							}
						}
					}
				}
			}
		}
	}
	else
	{
		res = vals[n];
	}

	return res;
}


					/*  --------------------------------------------------------- */

VError Selection::ChargeTri(SortElem &se, sLONG n, SortContext& tri)
{
	VError err = VE_OK;

	se.Activate(n,&tri);
	
	return(err);
}


void Selection::DeChargeTri(SortElem &se)
{
	se.DeActivate();
}


uBOOL Selection::issuptri( sLONG l, sLONG r, VError *err, SortContext& tri)
{
	sLONG i;
	uBOOL result, diff = false;
	ValPtr cv1,cv2;
	SortLine *li;
	CompareResult cmp;
	
	result=false;
	
	tri.fNbComp++;
	
	if (tri.fProgress != nil)
		tri.fProgress->Progress(tri.fNbComp);

	if (MustStopTask(tri.fProgress))
		*err = VE_DB4D_ACTIONCANCELEDBYUSER;
	
	if (*err == VE_OK)
	{
		if (l!=-1) 
		{
			*err=ChargeTri(tri.f1,l, tri);
		}
		if (r!=-1) 
		{
			*err=ChargeTri(tri.f2,r, tri);
		}
		
		for (i=1;i<=tri.st->GetNbLine() && *err == VE_OK;i++)
		{
			li = tri.st->GetTriLineRef(i);

			if (l == -1)
				cv1 = tri.pivot.GetNthValue(i,*err);
			else
				cv1 = tri.f1.GetNthValue(i,*err);

			if (r == -1)
				cv2 = tri.pivot.GetNthValue(i,*err);
			else
				cv2 = tri.f2.GetNthValue(i,*err);
			
			if (cv1 != nil && cv2 != nil)
			{
				cmp=cv1->CompareToSameKind(cv2,true);
				if (cmp!=CR_EQUAL)
				{
					diff = true;
					if (li->ascendant)
					{
						if (cmp==CR_BIGGER) result=true;
						break;
					}
					else
					{
						if (cmp==CR_SMALLER) result=true;
						break;
					}
				}
			}
		}

		if (!diff)
			tri.fWasNotUnique = true;
		if (tri.fTestUnicite && tri.fWasNotUnique)
			*err = VE_DB4D_DUPLICATED_KEY;

		
		if (l!=-1) DeChargeTri(tri.f1);
		if (r!=-1) DeChargeTri(tri.f2);
	}
	
	return(result);
}



VError Selection::SubSort( sLONG l, sLONG r, SortContext& tri)
{
	sLONG i;
	sLONG j;
	sLONG w;
	sLONG xx;
	sLONG w2;
	VError err;

	err=VE_OK;
	
	i = l;
	j = r;
	xx = (l + r) / 2;
	err=ChargeTri(tri.pivot,xx, tri);
		
	do {
	  while ( (i < tri.qt) && issuptri(-1, i, &err, tri) && (err==VE_OK) )
	  {
		  i++;
	  }

	  while ( (j >= 0) && issuptri(j, -1, &err, tri) && (err==VE_OK) )
	  {
		  j--;
	  }

	  if ( i <= j )
	  {
					sLONG nb = tri.fVals.GetCount();
					for (sLONG k = 1; k <= nb; k++)
					{
						SortCol* col = tri.fVals[k];
						ValPtr cv;
						cv = (*col)[i];
						(*col)[i] = (*col)[j];
						(*col)[j] = cv;
					}

		  w = GetFic(i);
		  w2 = GetFic(j);
		  PutFic(i, w2);
		  PutFic(j, w);
		  i++;
		  j--;
	  }
	} while ( (i<=j) && (err==VE_OK) );
	
	DeChargeTri(tri.pivot);
	
	if ( l < j && (err==VE_OK) ) err=SubSort( l, j, tri );
	
	if ( i < r && (err==VE_OK) ) err=SubSort( i, r, tri );
	
	return(err);
}


void Selection::TryToLoadIntoMem(SortContext& tri)
{
	StAllocateInCache alloc;
	Boolean stop = false;
	uLONG8 maxfree = VDBMgr::GetCacheManager()->GetMaxSize();
	SortLine li;
	tri.st->GetTriLine(1, &li);
	Field* cri = parentfile->GetTable()->RetainField(li.numfield);
	VError err = VE_OK;
	
	VArrayPtrOf<RelationPath*> RelPaths;
	RelPaths.SetOwnership(true);	// to avoid mem. leaks

	Base4D* db = parentfile->GetDB();
	sLONG target = parentfile->GetNum();

	if (cri != nil)
	{
		uLONG8 AvgSiz = cri->CalcAvgSize() + 8;
		cri->Release();
		if (AvgSiz > 0)
		{
				uLONG8 Reserve = (uLONG8)tri.qt * AvgSiz;
				if (Reserve < (maxfree >> 1))
				{
					sLONG i,nb = tri.st->GetNbLine();
					RelPaths.SetCount(nb, nil);

					for (i = 1; i <= nb; i++)
					{
						SortLine* sl = tri.st->GetTriLineRef(i);
						if (sl->numfile == 0 || sl->numfile == target)
							RelPaths[i-1] = nil;
						else
						{
							RelationPath* rp = new RelationPath;
							Table* xtab = db->RetainTable(sl->numfile);
							if (xtab != nil)
							{
								vector<uBYTE> none;
								if (rp->BuildPath(tri.context, parentfile->GetTable(), xtab, err, true, true, none))
								{
									RelPaths[i-1] = rp;
								}
								else
									delete rp;
								xtab->Release();
							}
						}

						SortCol* col = new SortCol;
						if (col != nil)
						{
								if (col->AddNSpaces(tri.qt,true))
								{
									if (!tri.fVals.Add(col))
										delete col;
								}
								else 
									delete col;
						}
					}

					nb = tri.fVals.GetCount();
					if (nb>0)
					{
						if (tri.fProgress != nil)
						{
							XBOX::VString session_title;
							VValueBag bag;
							VStr<64> tname;
							if (parentfile != nil)
							{
								Table* crit = parentfile->GetTable();
								if (crit != nil)
									crit->GetName(tname);
							}
							bag.SetString("TableName", tname);
							bag.SetString("curValue", L"{curValue}");
							bag.SetString("maxValue", L"{maxValue}");
							gs(1005,3,session_title);	// Sequential Sort: Loading %curValue of %maxValue Records
							session_title.Format(&bag);
							tri.fProgress->BeginSession(tri.qt,session_title,true);
							/*
							VString temps(L"Loading data");
							tri.fProgress->BeginSession(tri.qt, temps );
							*/
						}

						sLONG k,nb2 = tri.qt;

						ReadAheadBuffer* buf = db->NewReadAheadBuffer();

						for (k = 0; k < nb2 && !stop; k++)
						{
							if (tri.fProgress != nil)
								tri.fProgress->Progress(k+1);

							if (MustStopTask(tri.fProgress))
								err = VE_DB4D_ACTIONCANCELEDBYUSER;

							if (err == VE_OK)
							{
								FicheInMem* rec = parentfile->LoadRecord(GetFic(k),err,DB4D_Do_Not_Lock,tri.context, false, false, buf);
								if (rec != nil)
								{
									for (i = 1; i <= nb && !stop; i++)
									{
										SortLine* sl = tri.st->GetTriLineRef(i);
										ValPtr cv = nil;
										if (sl->numfile == 0 || sl->numfile == target)
										{
											cv = rec->GetNthField(sl->numfield, err, false, true);
										}
										else
										{
											RelationPath* rp = RelPaths[i-1];
											if (rp != nil)
											{
												FicheInMem* dest = nil;
												err = rp->ActivateRelation(rec, dest, tri.context, true);
												if (err == VE_OK)
												{
													if (dest == nil)
													{
														Table* xtab = db->RetainTable(sl->numfile);
														if (xtab != nil)
														{
															Field* xcri = xtab->RetainField(sl->numfield);
															if (xcri != nil)
															{
																cv = const_cast<VValueSingle*>(xcri->GetEmptyValue());
																xcri->Release();
															}
															xtab->Release();
														}
													}
													else
													{
														cv = dest->GetNthField(sl->numfield, err, false, true);
														dest->Release();
													}
												}
											}
										}

										{
											if (cv == nil)
												stop = true;
											else
											{
												ValPtr cv2 = (ValPtr)(cv->Clone());
												if (cv2 == nil)
													stop = true;
												else
												{
													SortCol* col = tri.fVals[i];
													(*col)[k] = cv2;
												}
											}
										}

									}
									rec->Release();
								}
								else
								{
									// if base err == memfull
									stop = true;
								}
							}
							else
								stop = true;
						}

						if (buf != nil)
							buf->Release();

						if (tri.fProgress != nil)
						{
							tri.fProgress->EndSession();
						}

						/*
						if (tri.fProgress != nil)
						{
							tri.fProgress->Stop();
						}
						*/
	
						if (stop)
						{
							FreeSortMem(tri);
						}
					}
				}
		}
	}
	 
}

void Selection::FreeSortMem(SortContext& tri)
{
	sLONG nb = tri.fVals.GetCount();
	sLONG i;

	for (i = 1; i<=nb; i++)
	{
		SortCol* col = tri.fVals[i];
		if (col != nil)
		{
			sLONG j,nb2 = col->GetCount();
			ValPtr *p = col->First();
			for (j = 1; j<=nb2; j++)
			{
					ValPtr v = *p;
					if (v != nil)
						delete v;
					p++;
			}
		}
		delete col;
	}

	tri.fVals.SetAllocatedSize(0);
}


Selection* Selection::SortSel(VError& err, LocalEntityModel* em, EntityAttributeSortedSelection* sortingAtt, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, WafSelection* inWafSel, WafSelection* outWafSel)
{
	Selection* result = nil;
	err = VE_OK;
	if (sortingAtt == nil || sortingAtt->empty() || GetQTfic() < 2)
	{
		result = this;
		result->Retain();
	}
	else
	{
		SortTab criterias(em->GetDB());
		for (EntityAttributeSortedSelection::iterator cur = sortingAtt->begin(), end = sortingAtt->end(); cur != end && err == VE_OK; cur++)
		{
			AttributePath* attpath = cur->fAttPath;
			if (attpath == nil)
			{
				db4dEntityAttribute* att = (db4dEntityAttribute*)(cur->fAttribute);
				if (att->IsSortable())
				{
					switch(att->GetKind())
					{
						case eattr_storage:
							if (att->HasEvent(dbev_load))
								criterias.AddAttribute(att,cur->fAscent);
							else
								criterias.AddTriLineField(att->GetTable()->GetNum(), att->GetFieldPos(), cur->fAscent);
							break;

						case eattr_alias:
						case eattr_computedField:
							criterias.AddAttribute(att,cur->fAscent);
							break;
					}
				}
				else
				{
					err = em->ThrowError(VE_DB4D_ATTRIBUTE_CANNOT_BE_USED_FOR_SORTING, &att->GetName());
				}
			}
			else
			{
				criterias.AddAttributePath(attpath, cur->fAscent);
			}
		}

		if (err == VE_OK)
		{
			if (criterias.GetNbLine() == 0)
			{
				result = this;
				result->Retain();
			}
			else
			{
				SortTab* tabs = &criterias;
				sLONG i,nb;
				FieldNuplet *fn;
				SortLine p1;
				FieldDef *p2;
				IndexInfo *ind;
				Base4D *bd;
				Selection *newsel = nil;
				Boolean ascent = true;
				Table* tt = em->GetMainTable();

				nb=tabs->GetNbLine();
				fn=new FieldNuplet(nb,false);
				p2=fn->GetDataPtr();
				bool okToLookForIndex = true;
				for (i=1;i<=nb;i++)
				{
					tabs->GetTriLine(i, &p1);
					if (i == 1) 
						ascent = p1.ascendant;
					else
					{
						if (p1.ascendant != ascent)
							okToLookForIndex = false;
					}
					if (p1.isfield)
					{
						p2->numfield=p1.numfield;
						p2->numfile=p1.numfile;
					}
					else
					{
						p2->numfield=0;
						p2->numfile=0;
					}
					p2++;
				}

				bd=tt->GetDF()->GetDB();

				fn->UpdateCritFic(bd);

				ind = nil;
				if (okToLookForIndex)
					ind=tt->FindAndRetainIndex(fn, true);
				if (ind != nil)
				{
					if (((sLONG8)GetQTfic() * tt->GetSeqRatioCorrector()) < (ind->GetNBDiskAccessForAScan(true) / 3))
					{
						ind->ReleaseValid();
						ind->Release();
						ind = nil;
					}
				}


				if (ind!=nil)
				{
					Bittab inWafSelBitSel;
					Bittab* inWafSelBitSelPtr = nil;
					if (inWafSel != nil)
					{
						inWafSelBitSelPtr = &inWafSelBitSel;
						FillBittabWithWafSel(inWafSel, inWafSelBitSel);
					}
					ind->Open(index_read);
					OccupableStack* curstack = gOccPool.GetStackForCurrentThread();

					newsel=ind->GetHeader()->SortSel(curstack, this, ascent, context, err, InProgress, false, inWafSelBitSelPtr, outWafSel);
					ind->Close();
					ind->ReleaseValid();
					ind->Release();
					result = newsel;
				}
				else
					result = SortSel(err, &criterias, context, InProgress, false, nil, inWafSel, outWafSel);
			}
		}
	}

	return result;
}


Selection* Selection::SortSel(VError& err, SortTab* tabs, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, Boolean TestUnicite, Table* remoteTable, WafSelection* inWafSel, WafSelection* outWafSel)
{
	sLONG nb;
	Selection* newsel = nil;
	SortContext tri;
	Boolean fullsort = true;

	err = VE_OK;
	nb = GetQTfic();
	tri.context = context;

	Table* assoctable;
	if (remoteTable != nil)
		assoctable = RetainRefCountable(remoteTable);
	else
		assoctable = parentfile->RetainTable();

	if (nb>1 && tabs->GetNbLine() > 0)
	{
		Bittab inWafSelBitSel;
		Bittab* inWafSelBitSelPtr = nil;
		if (inWafSel != nil)
		{
			inWafSelBitSelPtr = &inWafSelBitSel;
			FillBittabWithWafSel(inWafSel, inWafSelBitSel);
		}

		if (tabs->GetNbLine() == 1)
		{
			SortLine* li = tabs->GetTriLineRef(1);
			// L.E. 06/04/06 test li->isfield et ajout du else
			Field* cri = nil;
			if (assoctable != nil)
				cri = (li->isfield) ? assoctable->RetainField(li->numfield) : nil;
			else
				cri = nil;
			if (cri != nil)
			{
				Boolean ok = false;
				sLONG typ = cri->GetTyp();
				switch(typ) {

					case VK_STRING:
					case VK_TEXT:
						ok = TryToSortAlpha(err, cri, li->ascendant, context, InProgress, newsel, VK_STRING, TestUnicite, inWafSelBitSelPtr, outWafSel);
					break;

					case VK_STRING_UTF8:
					case VK_TEXT_UTF8:
						ok = TryToSortAlphaUTF8(err, cri, li->ascendant, context, InProgress, newsel, VK_STRING_UTF8, TestUnicite, inWafSelBitSelPtr, outWafSel);
						break;

					case VK_TIME:
						ok = TryToSortTime(err, cri, li->ascendant, context, InProgress, newsel, typ, TestUnicite, inWafSelBitSelPtr, outWafSel);
						break;

					case VK_UUID:
						ok = TryToSortUUID(err, cri, li->ascendant, context, InProgress, newsel, typ, TestUnicite, inWafSelBitSelPtr, outWafSel);
						break;
						
					case VK_LONG:
						ok = TryToSortLong(err, cri, li->ascendant, context, InProgress, newsel, typ, TestUnicite, inWafSelBitSelPtr, outWafSel);
						break;

					case VK_WORD:
						ok = TryToSortShort(err, cri, li->ascendant, context, InProgress, newsel, typ, TestUnicite, inWafSelBitSelPtr, outWafSel);
						break;

					case VK_BOOLEAN:
						ok = TryToSortBoolean(err, cri, li->ascendant, context, InProgress, newsel, typ, TestUnicite, inWafSelBitSelPtr, outWafSel);
						break;

					case VK_BYTE:
						ok = TryToSortByte(err, cri, li->ascendant, context, InProgress, newsel, typ, TestUnicite, inWafSelBitSelPtr, outWafSel);
						break;

					case VK_REAL:
						ok = TryToSortReal(err, cri, li->ascendant, context, InProgress, newsel, typ, TestUnicite, inWafSelBitSelPtr, outWafSel);
						break;

					case VK_LONG8:
					case VK_DURATION:
					case VK_SUBTABLE:
					case VK_SUBTABLE_KEY:
						ok = TryToSortLong8(err, cri, li->ascendant, context, InProgress, newsel, typ, TestUnicite, inWafSelBitSelPtr, outWafSel);
						break;

					default:
						ok = TryToSortMulti(err, tabs, context, InProgress, newsel, TestUnicite, remoteTable, inWafSelBitSelPtr, outWafSel);
						break;
				}

				cri->Release();
				if (ok)
					fullsort = false;
			}
			else
			{
				fullsort = ! TryToSortMulti(err, tabs, context, InProgress, newsel, TestUnicite, remoteTable, inWafSelBitSelPtr, outWafSel);
			}
		}
		else
		{
			fullsort = ! TryToSortMulti(err, tabs, context, InProgress, newsel, TestUnicite, remoteTable, inWafSelBitSelPtr, outWafSel);
		}

		if (TestUnicite/* && err == VE_DB4D_DUPLICATED_KEY*/)
			fullsort = false;

		if (fullsort && !MustStopTask() && (err == VE_OK) )
		{
			if (GetTypSel()==sel_bitsel)
			{
				newsel = CreFromBittab_nobitsel(parentfile, ((BitSel*)this)->GetBittab());
				if (newsel == nil)
					err = ThrowError(memfull, DBaction_SortingSelection);
				else
					newsel = newsel->SortSel(err, tabs, context, InProgress);
			}
			else
			{
				newsel = this;

				if (newsel == nil)
				{
					err = ThrowError(memfull, DBaction_SortingSelection);
				}
				else
				{
					newsel->Retain();

					tri.st=tabs;
					tri.fNbComp = 0;
					tri.fProgress = InProgress;
					tri.qt=nb;
					tri.fSel = newsel;
					tri.RelPaths.SetCount(tabs->GetNbLine(), nil);
					tri.fWasNotUnique = false;
					tri.fTestUnicite = TestUnicite;
					sLONG i;

					for (i = 1; i <= tabs->GetNbLine(); i++)
					{
						SortLine* sl = tri.st->GetTriLineRef(i);
						if (sl->numfile == 0 || sl->numfile == parentfile->GetNum())
							tri.RelPaths[i-1] = nil;
						else
						{
							RelationPath* rp = new RelationPath;
							Table* xtab = db->RetainTable(sl->numfile);
							if (xtab != nil)
							{
								vector<uBYTE> none;
								if (rp->BuildPath(tri.context, assoctable, xtab, err, true, true, none))
								{
									tri.RelPaths[i-1] = rp;
								}
								else
									delete rp;
								xtab->Release();
							}
						}
					}

					if (InProgress != nil)
					{
						XBOX::VString session_title;
						gs(1005,19,session_title);	// Sequential Sort
						InProgress->BeginSession(2,session_title,true);
						InProgress->Progress(1);
					}

					TryToLoadIntoMem(tri);

					if (tri.qt>1)
					{
						if (InProgress!=nil)
							InProgress->Progress(2);
						VString temps;
						gs(1005,20,temps);	// Sorting Data
						if (InProgress!=nil)
							InProgress->BeginSession(-1, temps );
						err=SubSort(0,tri.qt-1, tri);
						if (InProgress!=nil)
							InProgress->EndSession();
					}
						
					FreeSortMem(tri);

					if (InProgress != nil)
					{
						InProgress->EndSession();
					}
				}

			}
		}

		/*
		if (InProgress != nil)
		{
			InProgress->Stop();
		}
		*/
	}
	else
	{
		if (inWafSel != nil && outWafSel != nil)
		{
			*outWafSel = *inWafSel;
		}
		newsel = this;
		newsel->Retain();
	}

	if (assoctable != nil)
		assoctable->Release();

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOT_SORT_SELECTION, DBaction_SortingSelection);

	return newsel;
}



						// -------------------------------------------------
		


Boolean IndexInfoFromField::TryToBuildIndexLong(VError& err, sLONG MaxElemsInPage, sLONG RequiredElemsInPage, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, sLONG typ)
{
	
	Boolean result = true;
	
	TypeSortElemArray<sLONG> tempsort;
	Selection* into = nil;
	tempsort.SetBreakOnNonUnique(false);

	xMultiFieldDataOffsets off(crit, typ, true, sizeof(sLONG));
	result = tempsort.TryToSort((Selection*)nil, into, err, off, context, InProgress, fUniqueKeys, sizeof(sLONG));
	if (result)
	{
		if (fUniqueKeys && !tempsort.OKUnique())
		{
			fUniqueKeys = false;
			crit->SetUnique(false, InProgress, context == nil ? nil : context->GetEncapsuleur());
		}
		if (header->GetRealType() == DB4D_Index_Btree)
			err = tempsort.GenereIndex(this, MaxElemsInPage, RequiredElemsInPage, context, InProgress, typ, sizeof(sLONG));
		else
			err = tempsort.GenereClusterIndex(this, MaxElemsInPage, RequiredElemsInPage, context, InProgress, typ, sizeof(sLONG), off);
	}
	
	return result;
}


Boolean IndexInfoFromField::TryToBuildIndexTime(VError& err, sLONG MaxElemsInPage, sLONG RequiredElemsInPage, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, sLONG typ)
{
	Boolean result = true;
	
	TypeSortElemArray<xTime> tempsort;
	Selection* into = nil;
	tempsort.SetBreakOnNonUnique(false);

	xMultiFieldDataOffsets off(crit, typ, true, sizeof(xTime));
	result = tempsort.TryToSort((Selection*)nil, into, err, off, context, InProgress, fUniqueKeys, sizeof(xTime));
	if (result)
	{
		if (fUniqueKeys && !tempsort.OKUnique())
		{
			fUniqueKeys = false;
			crit->SetUnique(false, InProgress, context == nil ? nil : context->GetEncapsuleur());
		}
		if (header->GetRealType() == DB4D_Index_Btree)
			err = tempsort.GenereIndex(this, MaxElemsInPage, RequiredElemsInPage, context, InProgress, typ, sizeof(xTime));
		else
			err = tempsort.GenereClusterIndex(this, MaxElemsInPage, RequiredElemsInPage, context, InProgress, typ, sizeof(xTime), off);
}
	
	return result;
}


Boolean IndexInfoFromField::TryToBuildIndexLong8(VError& err, sLONG MaxElemsInPage, sLONG RequiredElemsInPage, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, sLONG typ)
{
	Boolean result = false;
	
	TypeSortElemArray<sLONG8> tempsort;
	Selection* into = nil;
	tempsort.SetBreakOnNonUnique(false);

	xMultiFieldDataOffsets off(crit, typ, true, sizeof(sLONG8));
	result = tempsort.TryToSort((Selection*)nil, into, err, off, context, InProgress, fUniqueKeys, sizeof(sLONG8));
	if (result)
	{
		if (fUniqueKeys && !tempsort.OKUnique())
		{
			fUniqueKeys = false;
			crit->SetUnique(false, InProgress, context == nil ? nil : context->GetEncapsuleur());
		}
		if (header->GetRealType() == DB4D_Index_Btree)
			err = tempsort.GenereIndex(this, MaxElemsInPage, RequiredElemsInPage, context, InProgress, typ, sizeof(sLONG8));
		else
			err = tempsort.GenereClusterIndex(this, MaxElemsInPage, RequiredElemsInPage, context, InProgress, typ, sizeof(sLONG8), off);
	}

	return result;
}


Boolean IndexInfoFromField::TryToBuildIndexShort(VError& err, sLONG MaxElemsInPage, sLONG RequiredElemsInPage, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, sLONG typ)
{
	Boolean result = false;
	
	TypeSortElemArray<sWORD> tempsort;
	Selection* into = nil;
	tempsort.SetBreakOnNonUnique(false);

	xMultiFieldDataOffsets off(crit, typ, true, sizeof(sWORD));
	result = tempsort.TryToSort((Selection*)nil, into, err, off, context, InProgress, fUniqueKeys, sizeof(sWORD));
	if (result)
	{
		if (fUniqueKeys && !tempsort.OKUnique())
		{
			fUniqueKeys = false;
			crit->SetUnique(false, InProgress, context == nil ? nil : context->GetEncapsuleur());
		}
		if (header->GetRealType() == DB4D_Index_Btree)
			err = tempsort.GenereIndex(this, MaxElemsInPage, RequiredElemsInPage, context, InProgress, typ, sizeof(sWORD));
		else
			err = tempsort.GenereClusterIndex(this, MaxElemsInPage, RequiredElemsInPage, context, InProgress, typ, sizeof(sWORD), off);
	}

	return result;
}


Boolean IndexInfoFromField::TryToBuildIndexBoolean(VError& err, sLONG MaxElemsInPage, sLONG RequiredElemsInPage, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, sLONG typ)
{
	Boolean result = false;

	TypeSortElemArray<uCHAR> tempsort;
	Selection* into = nil;
	tempsort.SetBreakOnNonUnique(false);

	xMultiFieldDataOffsets off(crit, typ, true, sizeof(uCHAR));
	result = tempsort.TryToSort((Selection*)nil, into, err, off, context, InProgress, fUniqueKeys, sizeof(uCHAR));
	if (result)
	{
		if (fUniqueKeys && !tempsort.OKUnique())
		{
			fUniqueKeys = false;
			crit->SetUnique(false, InProgress, context == nil ? nil : context->GetEncapsuleur());
		}
		if (header->GetRealType() == DB4D_Index_Btree)
			err = tempsort.GenereIndex(this, MaxElemsInPage, RequiredElemsInPage, context, InProgress, typ, sizeof(uCHAR));
		else
			err = tempsort.GenereClusterIndex(this, MaxElemsInPage, RequiredElemsInPage, context, InProgress, typ, sizeof(uCHAR), off);
	}

	return result;
}



Boolean IndexInfoFromField::TryToBuildIndexByte(VError& err, sLONG MaxElemsInPage, sLONG RequiredElemsInPage, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, sLONG typ)
{
	Boolean result = false;

	TypeSortElemArray<sBYTE> tempsort;
	Selection* into = nil;
	tempsort.SetBreakOnNonUnique(false);

	xMultiFieldDataOffsets off(crit, typ, true, sizeof(uCHAR));
	result = tempsort.TryToSort((Selection*)nil, into, err, off, context, InProgress, fUniqueKeys, sizeof(uCHAR));
	if (result)
	{
		if (fUniqueKeys && !tempsort.OKUnique())
		{
			fUniqueKeys = false;
			crit->SetUnique(false, InProgress, context == nil ? nil : context->GetEncapsuleur());
		}
		if (header->GetRealType() == DB4D_Index_Btree)
			err = tempsort.GenereIndex(this, MaxElemsInPage, RequiredElemsInPage, context, InProgress, typ, sizeof(uCHAR));
		else
			err = tempsort.GenereClusterIndex(this, MaxElemsInPage, RequiredElemsInPage, context, InProgress, typ, sizeof(uCHAR), off);
	}

	return result;
}


Boolean IndexInfoFromField::TryToBuildIndexReal(VError& err, sLONG MaxElemsInPage, sLONG RequiredElemsInPage, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, sLONG typ)
{
	Boolean result = false;
	
	TypeSortElemArray<Real> tempsort;
	Selection* into = nil;
	tempsort.SetBreakOnNonUnique(false);

	xMultiFieldDataOffsets off(crit, typ, true, sizeof(Real));
	result = tempsort.TryToSort((Selection*)nil, into, err, off, context, InProgress, fUniqueKeys, sizeof(Real));
	if (result)
	{
		if (fUniqueKeys && !tempsort.OKUnique())
		{
			fUniqueKeys = false;
			crit->SetUnique(false, InProgress, context == nil ? nil : context->GetEncapsuleur());
		}
		if (header->GetRealType() == DB4D_Index_Btree)
			err = tempsort.GenereIndex(this, MaxElemsInPage, RequiredElemsInPage, context, InProgress, typ, sizeof(Real));
		else
			err = tempsort.GenereClusterIndex(this, MaxElemsInPage, RequiredElemsInPage, context, InProgress, typ, sizeof(Real), off);
	}

	return result;
}


Boolean IndexInfoFromField::TryToBuildIndexUUID(VError& err, sLONG MaxElemsInPage, sLONG RequiredElemsInPage, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, sLONG typ)
{
	Boolean result = false;
	
	TypeSortElemArray<VUUIDBuffer> tempsort;
	Selection* into = nil;
	tempsort.SetBreakOnNonUnique(false);
	
	xMultiFieldDataOffsets off(crit, typ, true, sizeof(VUUIDBuffer));
	result = tempsort.TryToSort((Selection*)nil, into, err, off, context, InProgress, fUniqueKeys, sizeof(VUUIDBuffer));
	if (result)
	{
		if (fUniqueKeys && !tempsort.OKUnique())
		{
			fUniqueKeys = false;
			crit->SetUnique(false, InProgress, context == nil ? nil : context->GetEncapsuleur());
		}
		if (header->GetRealType() == DB4D_Index_Btree)
			err = tempsort.GenereIndex(this, MaxElemsInPage, RequiredElemsInPage, context, InProgress, typ, sizeof(VUUIDBuffer));
		else
			err = tempsort.GenereClusterIndex(this, MaxElemsInPage, RequiredElemsInPage, context, InProgress, typ, sizeof(VUUIDBuffer), off);
	}
	
	return result;
}


Boolean IndexInfoFromField::TryToBuildIndexAlpha(VError& err, Field* cri, sLONG MaxElemsInPage, sLONG RequiredElemsInPage, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, sLONG typ)
{

	Boolean result = false;
	
	TypeSortElemArray<xString> tempsort;
	Selection* into = nil;
	tempsort.SetBreakOnNonUnique(false);

	sLONG lenmax = cri->GetMaxLenForSort()+xString::NoDataSize();

	xMultiFieldDataOffsets off(crit, typ, true, lenmax);
	result = tempsort.TryToSort((Selection*)nil, into, err, off, context, InProgress, fUniqueKeys, lenmax);

	if (result)
	{
		if (fUniqueKeys && !tempsort.OKUnique())
		{
			fUniqueKeys = false;
			crit->SetUnique(false, InProgress, context == nil ? nil : context->GetEncapsuleur());
		}
#if VERSIONDEBUG
		// ##special test##
		if (tempsort.GetCount() != fic->GetDF()->GetNbRecords(context))
		{
			result = result;
		}
#endif

		if (header->GetRealType() == DB4D_Index_Btree)
			err = tempsort.GenereIndex(this, MaxElemsInPage, RequiredElemsInPage, context, InProgress, typ, lenmax);
		else
			err = tempsort.GenereClusterIndex(this, MaxElemsInPage, RequiredElemsInPage, context, InProgress, typ, lenmax, off);
	}

	FreeXString(tempsort, lenmax);

	if (err != VE_OK)
		result =false;
	return result;
}



Boolean IndexInfoFromField::TryToBuildIndexAlphaUTF8(VError& err, Field* cri, sLONG MaxElemsInPage, sLONG RequiredElemsInPage, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, sLONG typ)
{

	Boolean result = false;

	TypeSortElemArray<xStringUTF8> tempsort;
	Selection* into = nil;
	tempsort.SetBreakOnNonUnique(false);

	sLONG lenmax = cri->GetMaxLenForSort()+xStringUTF8::NoDataSize();

	xMultiFieldDataOffsets off(crit, typ, true, lenmax);
	result = tempsort.TryToSort((Selection*)nil, into, err, off, context, InProgress, fUniqueKeys, lenmax);

	if (result)
	{
		if (fUniqueKeys && !tempsort.OKUnique())
		{
			fUniqueKeys = false;
			crit->SetUnique(false, InProgress, context == nil ? nil : context->GetEncapsuleur());
		}
#if VERSIONDEBUG
		// ##special test##
		if (tempsort.GetCount() != fic->GetDF()->GetNbRecords(context))
		{
			result = result;
		}
#endif

		if (header->GetRealType() == DB4D_Index_Btree)
			err = tempsort.GenereIndex(this, MaxElemsInPage, RequiredElemsInPage, context, InProgress, typ, lenmax);
		else
			err = tempsort.GenereClusterIndex(this, MaxElemsInPage, RequiredElemsInPage, context, InProgress, typ, lenmax, off);
	}

	FreeXStringUTF8(tempsort, lenmax);

	if (err != VE_OK)
		result =false;
	return result;
}


															// -------------------------------------------------


Boolean IndexInfoFromMultipleField::TryToBuildIndexMulti(VError& err, sLONG MaxElemsInPage, sLONG RequiredElemsInPage, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress)
{

	Boolean result = false, okallfields = true;
	sLONG i,nb = frefs->GetCount();
	Table* fic = GetTargetTable();
	xMultiFieldDataOffsets off(fic);
	TypeSortElemArray<xMultiFieldData> tempsort;
	err = VE_OK;
	sLONG totlen = 0;
	tempsort.SetBreakOnNonUnique(true);

	for (i=0; i<nb && okallfields; i++)
	{
		if (fic == (*frefs)[i].fic)
		{
			Field* cri = (*frefs)[i].crit;
			if (cri == nil)
			{
				okallfields = false;
			}
			else
			{
				sLONG typ = cri->GetTyp();
				sLONG lenmax;
				switch(typ) 
				{
				case VK_STRING:
					lenmax = cri->GetMaxLenForSort() + xString::NoDataSize();
					break;

				case VK_STRING_UTF8:
					lenmax = cri->GetMaxLenForSort() + xStringUTF8::NoDataSize();
					break;

				case VK_TIME:
					lenmax = sizeof(xTime);
					break;

				case VK_UUID:
					lenmax = sizeof(VUUIDBuffer);
					break;
						
				case VK_LONG:
					lenmax = sizeof(sLONG);
					break;

				case VK_WORD:
					lenmax = sizeof(sWORD);
					break;

				case VK_BOOLEAN:
				case VK_BYTE:
					lenmax = sizeof(uCHAR);
					break;

				case VK_REAL:
					lenmax = sizeof(Real);
					break;

				case VK_LONG8:
				case VK_DURATION:
				case VK_SUBTABLE:
				case VK_SUBTABLE_KEY:
					lenmax = sizeof(sLONG8);
					break;

				default:
					okallfields = false;
					break;
				}

				if (okallfields)
				{
					off.AddOffset(totlen, typ, cri->GetPosInRec(), lenmax, true);
					totlen = totlen + lenmax;
				}
			}
		}
		else
			okallfields = false;
	}

	totlen += xMultiFieldData::GetEmptySize();
	if (okallfields)
	{
		Selection* into = nil;

		result = tempsort.TryToSort((Selection*)nil, into, err, off, context, InProgress, fUniqueKeys, totlen);

		if (result)
		{
			if (header->GetRealType() == DB4D_Index_Btree)
				err = tempsort.GenereIndex(this, MaxElemsInPage, RequiredElemsInPage, context, InProgress, -1, totlen);
			else
				err = tempsort.GenereClusterIndex(this, MaxElemsInPage, RequiredElemsInPage, context, InProgress, -1, totlen, off);
		}

		FreeXMulti(tempsort, totlen, off);
	}

	if (err != VE_OK)
		result =false;
	return result;
}



															// -------------------------------------------------

template <class Type>
sLONG Selection::SizeOfType(Field* cri)
{
	return sizeof(Type);
}

template <>
sLONG Selection::SizeOfType<xString>(Field* cri)
{
	return cri->GetMaxLenForSort()+xString::NoDataSize();
}


template <class Type>
void Selection::FreeAfter(TypeSortElemArray<Type>& tempsort, sLONG lenmax)
{
}


template <>
void Selection::FreeAfter(TypeSortElemArray<xString>& tempsort, sLONG lenmax)
{
	FreeXString(tempsort, lenmax);
}





template <class Type>
Boolean Selection::TryToFastGetDistinctValuesScalar(VError& err, Field* cri, QuickDB4DArrayOfValues *outCollection, BaseTaskInfo* context, 
												   VDB4DProgressIndicator* InProgress, sLONG typ, const VCompareOptions& inOptions)
{
	Boolean result = false;

	TypeSortElemArray<Type> tempsort;
	Selection* into = (Selection*)-1;

	sLONG lenmax = SizeOfType<Type>(cri);
	xMultiFieldDataOffsets off(cri, typ, true, lenmax);
	result = tempsort.TryToSort(this, into, err, off, context, InProgress, false, lenmax);

	if (result)
		err = tempsort.QuickBuildDistinctKeys(outCollection, context, InProgress, typ, lenmax, inOptions);

	FreeAfter(tempsort, lenmax);

	if (err != VE_OK)
		result =false;
	return result;

}




Boolean Selection::TryToFastQuickGetDistinctValues(VError& err, Field* cri, QuickDB4DArrayOfValues *outCollection, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, const VCompareOptions& inOptions)
{
	Boolean ok = false;
	err = VE_OK;
	sLONG typ = cri->GetTyp();
	switch(typ) {

		case VK_STRING:
		case VK_TEXT:
			ok = TryToFastGetDistinctValuesScalar<xString>(err, cri, outCollection, context, InProgress, VK_STRING, inOptions);
			break;

		case VK_STRING_UTF8:
		case VK_TEXT_UTF8:
			ok = TryToFastGetDistinctValuesScalar<xStringUTF8>(err, cri, outCollection, context, InProgress, VK_STRING_UTF8, inOptions);
			break;

		case VK_TIME:
			ok = TryToFastGetDistinctValuesScalar<xTime>(err, cri, outCollection, context, InProgress, typ, inOptions);
			break;

		case VK_LONG:
			ok = TryToFastGetDistinctValuesScalar<sLONG>(err, cri, outCollection, context, InProgress, typ, inOptions);
			break;

		case VK_WORD:
			ok = TryToFastGetDistinctValuesScalar<sWORD>(err, cri, outCollection, context, InProgress, typ, inOptions);
			break;

		case VK_BOOLEAN:
			ok = TryToFastGetDistinctValuesScalar<uBYTE>(err, cri, outCollection, context, InProgress, typ, inOptions);
			break;

		case VK_BYTE:
			ok = TryToFastGetDistinctValuesScalar<sBYTE>(err, cri, outCollection, context, InProgress, typ, inOptions);
			break;

		case VK_REAL:
			ok = TryToFastGetDistinctValuesScalar<Real>(err, cri, outCollection, context, InProgress, typ, inOptions);
			break;

		case VK_UUID:
			ok = TryToFastGetDistinctValuesScalar<VUUIDBuffer>(err, cri, outCollection, context, InProgress, typ, inOptions);
			break;
			
		case VK_LONG8:
		case VK_DURATION:
		case VK_SUBTABLE:
		case VK_SUBTABLE_KEY:
			ok = TryToFastGetDistinctValuesScalar<sLONG8>(err, cri, outCollection, context, InProgress, typ, inOptions);
			break;

			//default:
	}

	return ok;
}


															// -------------------------------------------------



Boolean Selection::TryToFastGetDistinctValues(VError& err, Field* cri, DB4DCollectionManager& outCollection, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, const VCompareOptions& inOptions)
{
	Boolean ok = false;
	err = VE_OK;
	sLONG typ = cri->GetTyp();
	switch(typ) {

		case VK_STRING:
		case VK_TEXT:
			ok = TryToFastGetDistinctValuesAlpha(err, cri, outCollection, context, InProgress, VK_STRING, inOptions);
			break;

		case VK_STRING_UTF8:
		case VK_TEXT_UTF8:
			ok = TryToFastGetDistinctValuesAlphaUTF8(err, cri, outCollection, context, InProgress, VK_STRING_UTF8, inOptions);
			break;

		case VK_TIME:
			ok = TryToFastGetDistinctValuesTime(err, cri, outCollection, context, InProgress, typ, inOptions);
			break;

		case VK_LONG:
			ok = TryToFastGetDistinctValuesLong(err, cri, outCollection, context, InProgress, typ, inOptions);
			break;

		case VK_WORD:
			ok = TryToFastGetDistinctValuesShort(err, cri, outCollection, context, InProgress, typ, inOptions);
			break;

		case VK_BOOLEAN:
			ok = TryToFastGetDistinctValuesBoolean(err, cri, outCollection, context, InProgress, typ, inOptions);
			break;

		case VK_BYTE:
			ok = TryToFastGetDistinctValuesByte(err, cri, outCollection, context, InProgress, typ, inOptions);
			break;

		case VK_REAL:
			ok = TryToFastGetDistinctValuesReal(err, cri, outCollection, context, InProgress, typ, inOptions);
			break;

		case VK_UUID:
			ok = TryToFastGetDistinctValuesUUID(err, cri, outCollection, context, InProgress, typ, inOptions);
			break;
			
		case VK_LONG8:
		case VK_DURATION:
		case VK_SUBTABLE:
		case VK_SUBTABLE_KEY:
			ok = TryToFastGetDistinctValuesLong8(err, cri, outCollection, context, InProgress, typ, inOptions);
			break;

			//default:
	}

	return ok;
}



Boolean Selection::TryToFastGetDistinctValues(VError& err, db4dEntityAttribute* att, DB4DCollectionManager& outCollection, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, const VCompareOptions& inOptions)
{
	Boolean ok = false;
	err = VE_OK;
	sLONG typ = att->ComputeScalarType();
	switch(typ) {

		case VK_STRING:
		case VK_TEXT:
			ok = TryToFastGetDistinctValuesAlpha(err, att, outCollection, context, InProgress, VK_STRING, inOptions);
			break;

			/*
		case VK_STRING_UTF8:
		case VK_TEXT_UTF8:
			ok = TryToFastGetDistinctValuesAlphaUTF8(err, att, outCollection, context, InProgress, VK_STRING_UTF8, inOptions);
			break;
			*/

		case VK_TIME:
			ok = TryToFastGetDistinctValuesTime(err, att, outCollection, context, InProgress, typ, inOptions);
			break;

		case VK_LONG:
			ok = TryToFastGetDistinctValuesLong(err, att, outCollection, context, InProgress, typ, inOptions);
			break;

		case VK_WORD:
			ok = TryToFastGetDistinctValuesShort(err, att, outCollection, context, InProgress, typ, inOptions);
			break;

		case VK_BOOLEAN:
			ok = TryToFastGetDistinctValuesBoolean(err, att, outCollection, context, InProgress, typ, inOptions);
			break;

		case VK_BYTE:
			ok = TryToFastGetDistinctValuesByte(err, att, outCollection, context, InProgress, typ, inOptions);
			break;

		case VK_REAL:
			ok = TryToFastGetDistinctValuesReal(err, att, outCollection, context, InProgress, typ, inOptions);
			break;

		case VK_UUID:
			ok = TryToFastGetDistinctValuesUUID(err, att, outCollection, context, InProgress, typ, inOptions);
			break;

		case VK_LONG8:
		case VK_DURATION:
		case VK_SUBTABLE:
		case VK_SUBTABLE_KEY:
			ok = TryToFastGetDistinctValuesLong8(err, att, outCollection, context, InProgress, typ, inOptions);
			break;

			//default:
	}

	return ok;
}




Boolean Selection::TryToFastGetDistinctValuesAlpha(VError& err, Field* cri, DB4DCollectionManager& outCollection, BaseTaskInfo* context, 
													VDB4DProgressIndicator* InProgress, sLONG typ, const VCompareOptions& inOptions)
{
	Boolean result = false;

	TypeSortElemArray<xString> tempsort;
	Selection* into = (Selection*)-1;

	sLONG lenmax = cri->GetMaxLenForSort()+xString::NoDataSize();
	xMultiFieldDataOffsets off(cri, typ, true, lenmax);
	result = tempsort.TryToSort(this, into, err, off, context, InProgress, false, lenmax);

	if (result)
		err = tempsort.BuildDistinctKeys(outCollection, context, InProgress, typ, lenmax, inOptions);

	FreeXString(tempsort, lenmax);

	if (err != VE_OK)
		result =false;
	return result;

}



Boolean Selection::TryToFastGetDistinctValuesAlpha(VError& err, db4dEntityAttribute* att, DB4DCollectionManager& outCollection, BaseTaskInfo* context, 
												   VDB4DProgressIndicator* InProgress, sLONG typ, const VCompareOptions& inOptions)
{
	Boolean result = false;

	TypeSortElemArray<xString> tempsort;
	Selection* into = (Selection*)-1;

	xMultiFieldDataOffsets off(att, true);
	result = tempsort.TryToSort(this, into, err, off, context, InProgress, false, off.GetLength());

	if (result)
		err = tempsort.BuildDistinctKeys(outCollection, context, InProgress, typ, off.GetLength(), inOptions);

	FreeXString(tempsort, off.GetLength());

	if (err != VE_OK)
		result =false;
	return result;

}


Boolean Selection::TryToFastGetDistinctValuesAlphaUTF8(VError& err, Field* cri, DB4DCollectionManager& outCollection, BaseTaskInfo* context, 
												   VDB4DProgressIndicator* InProgress, sLONG typ, const VCompareOptions& inOptions)
{
	Boolean result = false;

	TypeSortElemArray<xStringUTF8> tempsort;
	Selection* into = (Selection*)-1;

	sLONG lenmax = cri->GetMaxLenForSort()+xStringUTF8::NoDataSize();
	xMultiFieldDataOffsets off(cri, typ, true, lenmax);
	result = tempsort.TryToSort(this, into, err, off, context, InProgress, false, lenmax);

	if (result)
		err = tempsort.BuildDistinctKeys(outCollection, context, InProgress, typ, lenmax, inOptions);

	FreeXStringUTF8(tempsort, lenmax);

	if (err != VE_OK)
		result =false;
	return result;

}


Boolean Selection::TryToFastGetDistinctValuesTime(VError& err, Field* cri, DB4DCollectionManager& outCollection, BaseTaskInfo* context, 
												VDB4DProgressIndicator* InProgress, sLONG typ, const VCompareOptions& inOptions)
{
	Boolean result = false;

	TypeSortElemArray<xTime> tempsort;
	Selection* into = (Selection*)-1;

	xMultiFieldDataOffsets off(cri, typ, true, sizeof(xTime));
	result = tempsort.TryToSort(this, into, err, off, context, InProgress, false, sizeof(xTime));

	if (result)
		err = tempsort.BuildDistinctKeys(outCollection, context, InProgress, typ, sizeof(xTime), inOptions);

	if (err != VE_OK)
		result =false;
	return result;
}



Boolean Selection::TryToFastGetDistinctValuesTime(VError& err, db4dEntityAttribute* att, DB4DCollectionManager& outCollection, BaseTaskInfo* context, 
												  VDB4DProgressIndicator* InProgress, sLONG typ, const VCompareOptions& inOptions)
{
	Boolean result = false;

	TypeSortElemArray<xTime> tempsort;
	Selection* into = (Selection*)-1;

	xMultiFieldDataOffsets off(att, true);
	result = tempsort.TryToSort(this, into, err, off, context, InProgress, false, sizeof(xTime));

	if (result)
		err = tempsort.BuildDistinctKeys(outCollection, context, InProgress, typ, sizeof(xTime), inOptions);

	if (err != VE_OK)
		result =false;
	return result;
}



Boolean Selection::TryToFastGetDistinctValuesLong8(VError& err, Field* cri, DB4DCollectionManager& outCollection, BaseTaskInfo* context, 
													VDB4DProgressIndicator* InProgress, sLONG typ, const VCompareOptions& inOptions)
{
	Boolean result = false;

	TypeSortElemArray<sLONG8> tempsort;
	Selection* into = (Selection*)-1;

	xMultiFieldDataOffsets off(cri, typ, true, sizeof(sLONG8));
	result = tempsort.TryToSort(this, into, err, off, context, InProgress, false, sizeof(sLONG8));

	if (result)
		err = tempsort.BuildDistinctKeys(outCollection, context, InProgress, typ, sizeof(sLONG8), inOptions);

	if (err != VE_OK)
		result =false;
	return result;

}



Boolean Selection::TryToFastGetDistinctValuesLong8(VError& err, db4dEntityAttribute* att, DB4DCollectionManager& outCollection, BaseTaskInfo* context, 
												   VDB4DProgressIndicator* InProgress, sLONG typ, const VCompareOptions& inOptions)
{
	Boolean result = false;

	TypeSortElemArray<sLONG8> tempsort;
	Selection* into = (Selection*)-1;

	xMultiFieldDataOffsets off(att, true);
	result = tempsort.TryToSort(this, into, err, off, context, InProgress, false, sizeof(sLONG8));

	if (result)
		err = tempsort.BuildDistinctKeys(outCollection, context, InProgress, typ, sizeof(sLONG8), inOptions);

	if (err != VE_OK)
		result =false;
	return result;

}


Boolean Selection::TryToFastGetDistinctValuesLong(VError& err, Field* cri, DB4DCollectionManager& outCollection, BaseTaskInfo* context, 
													VDB4DProgressIndicator* InProgress, sLONG typ, const VCompareOptions& inOptions)
{
	Boolean result = false;

	TypeSortElemArray<sLONG> tempsort;
	Selection* into = (Selection*)-1;

	xMultiFieldDataOffsets off(cri, typ, true, sizeof(sLONG));
	result = tempsort.TryToSort(this, into, err, off, context, InProgress, false, sizeof(sLONG));

	if (result)
		err = tempsort.BuildDistinctKeys(outCollection, context, InProgress, typ, sizeof(sLONG), inOptions);

	if (err != VE_OK)
		result =false;
	return result;

}



Boolean Selection::TryToFastGetDistinctValuesLong(VError& err, db4dEntityAttribute* att, DB4DCollectionManager& outCollection, BaseTaskInfo* context, 
												  VDB4DProgressIndicator* InProgress, sLONG typ, const VCompareOptions& inOptions)
{
	Boolean result = false;

	TypeSortElemArray<sLONG> tempsort;
	Selection* into = (Selection*)-1;

	xMultiFieldDataOffsets off(att, true);
	result = tempsort.TryToSort(this, into, err, off, context, InProgress, false, sizeof(sLONG));

	if (result)
		err = tempsort.BuildDistinctKeys(outCollection, context, InProgress, typ, sizeof(sLONG), inOptions);

	if (err != VE_OK)
		result =false;
	return result;

}


Boolean Selection::TryToFastGetDistinctValuesShort(VError& err, Field* cri, DB4DCollectionManager& outCollection, BaseTaskInfo* context, 
													VDB4DProgressIndicator* InProgress, sLONG typ, const VCompareOptions& inOptions)
{
	Boolean result = false;

	TypeSortElemArray<sWORD> tempsort;
	Selection* into = (Selection*)-1;

	xMultiFieldDataOffsets off(cri, typ, true, sizeof(sWORD));
	result = tempsort.TryToSort(this, into, err, off, context, InProgress, false, sizeof(sWORD));

	if (result)
		err = tempsort.BuildDistinctKeys(outCollection, context, InProgress, typ, sizeof(sWORD), inOptions);

	if (err != VE_OK)
		result =false;
	return result;

}



Boolean Selection::TryToFastGetDistinctValuesShort(VError& err, db4dEntityAttribute* att, DB4DCollectionManager& outCollection, BaseTaskInfo* context, 
												   VDB4DProgressIndicator* InProgress, sLONG typ, const VCompareOptions& inOptions)
{
	Boolean result = false;

	TypeSortElemArray<sWORD> tempsort;
	Selection* into = (Selection*)-1;

	xMultiFieldDataOffsets off(att, true);
	result = tempsort.TryToSort(this, into, err, off, context, InProgress, false, sizeof(sWORD));

	if (result)
		err = tempsort.BuildDistinctKeys(outCollection, context, InProgress, typ, sizeof(sWORD), inOptions);

	if (err != VE_OK)
		result =false;
	return result;

}

Boolean Selection::TryToFastGetDistinctValuesBoolean(VError& err, Field* cri, DB4DCollectionManager& outCollection, BaseTaskInfo* context, 
														VDB4DProgressIndicator* InProgress, sLONG typ, const VCompareOptions& inOptions)
{
	Boolean result = false;

	TypeSortElemArray<uCHAR> tempsort;
	Selection* into = (Selection*)-1;

	xMultiFieldDataOffsets off(cri, typ, true, sizeof(uCHAR));
	result = tempsort.TryToSort(this, into, err, off, context, InProgress, false, sizeof(uCHAR));

	if (result)
		err = tempsort.BuildDistinctKeys(outCollection, context, InProgress, typ, sizeof(uCHAR), inOptions);

	if (err != VE_OK)
		result =false;
	return result;

}


Boolean Selection::TryToFastGetDistinctValuesBoolean(VError& err, db4dEntityAttribute* att, DB4DCollectionManager& outCollection, BaseTaskInfo* context, 
													 VDB4DProgressIndicator* InProgress, sLONG typ, const VCompareOptions& inOptions)
{
	Boolean result = false;

	TypeSortElemArray<uCHAR> tempsort;
	Selection* into = (Selection*)-1;

	xMultiFieldDataOffsets off(att, true);
	result = tempsort.TryToSort(this, into, err, off, context, InProgress, false, sizeof(uCHAR));

	if (result)
		err = tempsort.BuildDistinctKeys(outCollection, context, InProgress, typ, sizeof(uCHAR), inOptions);

	if (err != VE_OK)
		result =false;
	return result;

}


Boolean Selection::TryToFastGetDistinctValuesByte(VError& err, Field* cri, DB4DCollectionManager& outCollection, BaseTaskInfo* context, 
													 VDB4DProgressIndicator* InProgress, sLONG typ, const VCompareOptions& inOptions)
{
	Boolean result = false;

	TypeSortElemArray<sBYTE> tempsort;
	Selection* into = (Selection*)-1;

	xMultiFieldDataOffsets off(cri, typ, true, sizeof(uCHAR));
	result = tempsort.TryToSort(this, into, err, off, context, InProgress, false, sizeof(uCHAR));

	if (result)
		err = tempsort.BuildDistinctKeys(outCollection, context, InProgress, typ, sizeof(uCHAR), inOptions);

	if (err != VE_OK)
		result =false;
	return result;

}



Boolean Selection::TryToFastGetDistinctValuesByte(VError& err, db4dEntityAttribute* att, DB4DCollectionManager& outCollection, BaseTaskInfo* context, 
												  VDB4DProgressIndicator* InProgress, sLONG typ, const VCompareOptions& inOptions)
{
	Boolean result = false;

	TypeSortElemArray<sBYTE> tempsort;
	Selection* into = (Selection*)-1;

	xMultiFieldDataOffsets off(att, true);
	result = tempsort.TryToSort(this, into, err, off, context, InProgress, false, sizeof(uCHAR));

	if (result)
		err = tempsort.BuildDistinctKeys(outCollection, context, InProgress, typ, sizeof(uCHAR), inOptions);

	if (err != VE_OK)
		result =false;
	return result;

}


Boolean Selection::TryToFastGetDistinctValuesReal(VError& err, Field* cri, DB4DCollectionManager& outCollection, BaseTaskInfo* context, 
													VDB4DProgressIndicator* InProgress, sLONG typ, const VCompareOptions& inOptions)
{
	Boolean result = false;

	TypeSortElemArray<Real> tempsort;
	Selection* into = (Selection*)-1;

	xMultiFieldDataOffsets off(cri, typ, true, sizeof(Real));
	result = tempsort.TryToSort(this, into, err, off, context, InProgress, false, sizeof(Real));

	if (result)
		err = tempsort.BuildDistinctKeys(outCollection, context, InProgress, typ, sizeof(Real), inOptions);

	if (err != VE_OK)
		result =false;
	return result;

}


Boolean Selection::TryToFastGetDistinctValuesReal(VError& err, db4dEntityAttribute* att, DB4DCollectionManager& outCollection, BaseTaskInfo* context, 
												  VDB4DProgressIndicator* InProgress, sLONG typ, const VCompareOptions& inOptions)
{
	Boolean result = false;

	TypeSortElemArray<Real> tempsort;
	Selection* into = (Selection*)-1;

	xMultiFieldDataOffsets off(att, true);
	result = tempsort.TryToSort(this, into, err, off, context, InProgress, false, sizeof(Real));

	if (result)
		err = tempsort.BuildDistinctKeys(outCollection, context, InProgress, typ, sizeof(Real), inOptions);

	if (err != VE_OK)
		result =false;
	return result;

}


Boolean Selection::TryToFastGetDistinctValuesUUID(VError& err, Field* cri, DB4DCollectionManager& outCollection, BaseTaskInfo* context, 
												  VDB4DProgressIndicator* InProgress, sLONG typ, const VCompareOptions& inOptions)
{
	Boolean result = false;
	
	TypeSortElemArray<VUUIDBuffer> tempsort;
	Selection* into = (Selection*)-1;
	
	xMultiFieldDataOffsets off(cri, typ, true, sizeof(VUUIDBuffer));
	result = tempsort.TryToSort(this, into, err, off, context, InProgress, false, sizeof(VUUIDBuffer));
	
	if (result)
		err = tempsort.BuildDistinctKeys(outCollection, context, InProgress, typ, sizeof(VUUIDBuffer), inOptions);
	
	if (err != VE_OK)
		result =false;
	return result;
	
}



Boolean Selection::TryToFastGetDistinctValuesUUID(VError& err, db4dEntityAttribute* att, DB4DCollectionManager& outCollection, BaseTaskInfo* context, 
												  VDB4DProgressIndicator* InProgress, sLONG typ, const VCompareOptions& inOptions)
{
	Boolean result = false;

	TypeSortElemArray<VUUIDBuffer> tempsort;
	Selection* into = (Selection*)-1;

	xMultiFieldDataOffsets off(att, true);
	result = tempsort.TryToSort(this, into, err, off, context, InProgress, false, sizeof(VUUIDBuffer));

	if (result)
		err = tempsort.BuildDistinctKeys(outCollection, context, InProgress, typ, sizeof(VUUIDBuffer), inOptions);

	if (err != VE_OK)
		result =false;
	return result;

}












