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

#include <set>

//DateBlock nulldate= { 0,0,0 };


template <>
Boolean DB4DArrayOfConstDirectValues<uBYTE>::Find(XBOX::VValueSingle& inValue, const XBOX::VCompareOptions& inOptions) const
{
	uBYTE b = inValue.GetByte();
	return FindDirect(&b, inOptions);
}


template <>
Boolean DB4DArrayOfConstDirectValues<sBYTE>::Find(XBOX::VValueSingle& inValue, const XBOX::VCompareOptions& inOptions) const
{
	sBYTE b = inValue.GetByte();
	return FindDirect(&b, inOptions);
}


template <>
Boolean DB4DArrayOfConstDirectValues<Real>::Find(XBOX::VValueSingle& inValue, const XBOX::VCompareOptions& inOptions) const
{
	Real r = inValue.GetReal();
	return FindDirect(&r, inOptions);
}

template <>
Boolean DB4DArrayOfConstDirectValues<sWORD>::Find(XBOX::VValueSingle& inValue, const XBOX::VCompareOptions& inOptions) const
{
	sWORD n = inValue.GetWord();
	return FindDirect(&n, inOptions);
}

template <>
Boolean DB4DArrayOfConstDirectValues<sLONG>::Find(XBOX::VValueSingle& inValue, const XBOX::VCompareOptions& inOptions) const
{
	sLONG l = inValue.GetLong();
	return FindDirect(&l, inOptions);
}

template <>
Boolean DB4DArrayOfConstDirectValues<sLONG8>::Find(XBOX::VValueSingle& inValue, const XBOX::VCompareOptions& inOptions) const
{
	sLONG8 l8 = inValue.GetLong8();
	return FindDirect(&l8, inOptions);
}

template <>
Boolean DB4DArrayOfConstDirectValues<uLONG8>::Find(XBOX::VValueSingle& inValue, const XBOX::VCompareOptions& inOptions) const
{
	VTime tt;
	inValue.GetTime(tt);
	uLONG8 xtt = tt.GetMilliseconds();
	return FindDirect(&xtt, inOptions);
}

template <>
Boolean DB4DArrayOfConstDirectValues<xTime>::Find(XBOX::VValueSingle& inValue, const XBOX::VCompareOptions& inOptions) const
{
	VTime tt;
	inValue.GetTime(tt);
	xTime xtt(tt);
	return FindDirect(&xtt, inOptions);
}


template <>
Boolean DB4DArrayOfConstDirectValues<VUUIDBuffer>::Find(XBOX::VValueSingle& inValue, const XBOX::VCompareOptions& inOptions) const
{
	VUUID xid;
	inValue.GetVUUID(xid);
	return FindDirect(&(xid.GetBuffer()), inOptions);
}


template <>
Boolean DB4DArrayOfConstDirectValues<VInlineString>::Find(XBOX::VValueSingle& inValue, const XBOX::VCompareOptions& inOptions) const
{
	VInlineString val;
	VString ss;
	inValue.GetString(ss);
	val.InitWithString(ss);
	MyLess pred(inOptions);
	MyLess& predref = pred;
	return binary_search(fDataArray, fDataArray + fSize, val, predref);
}


template <>
Boolean DB4DArrayOfConstDirectValues<QuickString>::Find(XBOX::VValueSingle& inValue, const XBOX::VCompareOptions& inOptions) const
{
	MyLess pred(inOptions);
	MyLess& predref = pred;
	QuickString xs;
	VString s;
	inValue.GetString(s);
	sLONG len = s.GetLength();
	const UniChar* source = s.GetCPointer();
	xs.fString = (UniChar*)VObject::GetMainMemMgr()->Malloc(len*2 + 4, false, 'qstr');
	if (len > 0)
		copy(source, source+len, xs.fString+2);
	*((sLONG*)xs.fString) = len;
	Boolean result = binary_search(fDataArray, fDataArray + fSize, xs, predref);
	VObject::GetMainMemMgr()->Free(xs.fString);

	return result;
}


DB4DArrayOfValues* GenerateConstArrayOfValues(DB4DArrayOfValues* values, sLONG datakind, const VCompareOptions& inOptions)
{
	assert(values != nil);
	sLONG signature = values->GetSignature();
	if (signature == 'cons')
	{
		values->Retain();
		return values;
	}
	else if (signature == 'rawx')
	{
		DB4DArrayOfValues* newvals = ((QuickDB4DArrayOfValues*)values)->GenerateConstArrayOfValues();
		return newvals;
	}
	else
	{
		DB4DArrayOfValues* newvals = nil;
		assert(signature == 'dscv');
		switch(datakind)
		{
			case VK_TEXT_UTF8:
			case VK_STRING_UTF8:
			case VK_TEXT:
			case VK_STRING:
				newvals = new DB4DArrayOfConstDirectValues<VInlineString>(values, datakind, inOptions);
				break;

			case VK_BOOLEAN:
				newvals = new DB4DArrayOfConstDirectValues<uBYTE>(values, datakind, inOptions);
				break;

			case VK_BYTE:
				newvals = new DB4DArrayOfConstDirectValues<sBYTE>(values, datakind, inOptions);
				break;

			case VK_WORD:
				newvals = new DB4DArrayOfConstDirectValues<sWORD>(values, datakind, inOptions);
				break;

			case VK_LONG:
				newvals = new DB4DArrayOfConstDirectValues<sLONG>(values, datakind, inOptions);
				break;

			case VK_TIME:
				newvals = new DB4DArrayOfConstDirectValues<uLONG8>(values, datakind, inOptions);
				break;

			case VK_UUID:
			//	newvals = new DB4DArrayOfConstDirectValues<VUUIDBuffer>(values, datakind, inOptions);
				/*	2009-09-10, T.A.
					values is supposed to be a VDB4DDataSourceArray_cv (we're in signature == 'dscv')
							=> it encapsulates a VInlineString* array, not a VUUIDBuffer* array
							=> we create the VUUIDBuffer* array
				*/
				{
					DB4DArrayOfDirectValues<VUUIDBuffer> *dbaodv = new DB4DArrayOfDirectValues<VUUIDBuffer>(inOptions, datakind);

					sLONG	count = values->Count();
					dbaodv->Reserve(count);
					VInlineString *currentInlineStr = (VInlineString *) values->GetFirstPtr();
					for(sLONG i = 0; i < count; ++i)
					{
						VUUID	uid;
						uid.FromString( VString(*currentInlineStr) );
						dbaodv->AddOneElem( uid.GetBuffer() );
						++currentInlineStr;
					}
					dbaodv->Sort();
					newvals = new DB4DArrayOfConstDirectValues<VUUIDBuffer>( dbaodv );
					dbaodv->Release();
				}
				break;
				
			case VK_LONG8:
			case VK_SUBTABLE:
			case VK_SUBTABLE_KEY:
			case VK_DURATION:
				newvals = new DB4DArrayOfConstDirectValues<sLONG8>(values, datakind, inOptions);
				break;

			case VK_REAL:
				newvals = new DB4DArrayOfConstDirectValues<Real>(values, datakind, inOptions);
				break;

			default:
				assert(false);
				newvals = values;
				newvals->Retain();
				break;
		}
		return newvals;
	}
}

DB4DArrayOfValues* GenerateConstArrayOfValues(VStream& inStream, sLONG datakind, const VCompareOptions& inOptions)
{
	DB4DArrayOfValues *finalConstArray = NULL;

	QuickDB4DArrayOfValues *dbaodv = CreateDB4DArrayOfDirectValues(datakind, inOptions, (VJSArray*)nil );

	if(dbaodv != NULL)
	{
		if(dbaodv->GetFrom(inStream) == VE_OK)
		{
			switch(datakind)
			{
			case VK_UUID:
				finalConstArray = new DB4DArrayOfConstDirectValues<VUUIDBuffer>( (DB4DArrayOfDirectValues<VUUIDBuffer> *) dbaodv );
				dbaodv->Release();
				break;

			// Other kinds: to be implemented when it becomes necessary
			default:
				assert(false);
				break;
			}
		}
	}

	return finalConstArray; 
}



DB4DArrayOfValues* GenerateConstArrayOfValues(VJSArray& jsarr, sLONG datakind, const VCompareOptions& inOptions)
{
	DB4DArrayOfValues* result = nil;
	VCompareOptions options = inOptions;
	if (options.GetIntlManager() == nil)
		options.SetIntlManager(VTask::GetCurrentIntlManager());

	QuickDB4DArrayOfValues* newvals = CreateDB4DArrayOfDirectValues(datakind, options, &jsarr);
	if (newvals != nil)
	{
		result = newvals->GenerateConstArrayOfValues();
		newvals->Release();
	}

	return result;
}



DB4DArrayOfValues* GenerateConstArrayOfValues(VJSONArray* jsarr, sLONG datakind, const VCompareOptions& inOptions)
{
	DB4DArrayOfValues* result = nil;
	VCompareOptions options = inOptions;
	if (options.GetIntlManager() == nil)
		options.SetIntlManager(VTask::GetCurrentIntlManager());

	QuickDB4DArrayOfValues* newvals = CreateDB4DArrayOfDirectValues(datakind, options, jsarr);
	if (newvals != nil)
	{
		result = newvals->GenerateConstArrayOfValues();
		newvals->Release();
	}

	return result;
}





#if 0
DB4DArrayOfValues* DBAVConsCreator(sLONG signature, VStream *inStream, XBOX::VError& outError)
{
	outError = VE_OK;
	assert(signature == 'cons');
	DB4DArrayOfValues* result = nil;

	sLONG sourcesig;
	outError = inStream->GetLong(sourcesig);
	DB4DArrayOfValuesCreator dbc = VDBMgr::GetManager()->GetDB4DArrayOfValuesCreator(sig);
	if (dbc == nil)
		outError = ThrowBaseError(VE_DB4D_ARRAYOFVALUES_CREATOR_IS_MISSING);
	else
	{
		DB4DArrayOfValues* source = (*dbc)(sourcesig, inStream, outError);
		if (outError == VE_OK)
		{
			result = GenerateConstArrayOfValues(source, cri->GetTyp(), inOptions);
		}
		QuickReleaseRefCountable(source);
	}
	return result;
}
#endif


QuickDB4DArrayOfValues* CreateDB4DArrayOfDirectValues(sLONG inDataKind, const VCompareOptions& inOptions, VJSArray* jsarr)
{
	switch(inDataKind)
	{
		case VK_BOOLEAN:
			return new DB4DArrayOfDirectValues<uBYTE>(inOptions, inDataKind, jsarr);
			break;

		case VK_BYTE:
			return new DB4DArrayOfDirectValues<sBYTE>(inOptions, inDataKind, jsarr);
			break;

		case VK_WORD:
			return new DB4DArrayOfDirectValues<sWORD>(inOptions, inDataKind, jsarr);
			break;

		case VK_TIME:
			return new DB4DArrayOfDirectValues<uLONG8>(inOptions, inDataKind, jsarr);
			break;

		case VK_DURATION:
		case VK_SUBTABLE:
		case VK_SUBTABLE_KEY:
		case VK_LONG8:
			return new DB4DArrayOfDirectValues<sLONG8>(inOptions, inDataKind, jsarr);
			break;

		case VK_LONG:
			return new DB4DArrayOfDirectValues<sLONG>(inOptions, inDataKind, jsarr);
			break;

		case VK_REAL:
			return new DB4DArrayOfDirectValues<Real>(inOptions, inDataKind, jsarr);
			break;

		case VK_STRING:
		case VK_STRING_UTF8:
			return new DB4DArrayOfDirectValues<QuickString>(inOptions, inDataKind, jsarr);
			break;

		case VK_UUID:
			return new DB4DArrayOfDirectValues<VUUIDBuffer>(inOptions, inDataKind, jsarr);
			break;
	}

	assert(false);
	return nil;
}



QuickDB4DArrayOfValues* CreateDB4DArrayOfDirectValues(sLONG inDataKind, const VCompareOptions& inOptions, VJSONArray* jsarr)
{
	switch(inDataKind)
	{
	case VK_BOOLEAN:
		return new DB4DArrayOfDirectValues<uBYTE>(inOptions, inDataKind, jsarr);
		break;

	case VK_BYTE:
		return new DB4DArrayOfDirectValues<sBYTE>(inOptions, inDataKind, jsarr);
		break;

	case VK_WORD:
		return new DB4DArrayOfDirectValues<sWORD>(inOptions, inDataKind, jsarr);
		break;

	case VK_TIME:
		return new DB4DArrayOfDirectValues<uLONG8>(inOptions, inDataKind, jsarr);
		break;

	case VK_DURATION:
	case VK_SUBTABLE:
	case VK_SUBTABLE_KEY:
	case VK_LONG8:
		return new DB4DArrayOfDirectValues<sLONG8>(inOptions, inDataKind, jsarr);
		break;

	case VK_LONG:
		return new DB4DArrayOfDirectValues<sLONG>(inOptions, inDataKind, jsarr);
		break;

	case VK_REAL:
		return new DB4DArrayOfDirectValues<Real>(inOptions, inDataKind, jsarr);
		break;

	case VK_STRING:
	case VK_STRING_UTF8:
		return new DB4DArrayOfDirectValues<QuickString>(inOptions, inDataKind, jsarr);
		break;

	case VK_UUID:
		return new DB4DArrayOfDirectValues<VUUIDBuffer>(inOptions, inDataKind, jsarr);
		break;
	}

	assert(false);
	return nil;
}



DB4DArrayOfValues* DBAVVCreator(sLONG signature, VStream *inStream, XBOX::VError& outError)
{
	outError = VE_UNIMPLEMENTED;
	assert(signature == kDBAVSIG);
	//return new DB4DArrayOfVValues();
	return nil;
}


#if 0
void DB4DArrayOfVValues::Sort(Boolean ascending)
{
	count = GetCount();
	subsort(0,GetCount()-1);
}


void DB4DArrayOfVValues::subsort(sLONG l, sLONG r)
{
	sLONG i;
	sLONG j;
	sLONG xx;
	VValueSingle* pivot;

	i = l;
	j = r;
	xx = (l + r) / 2;
	pivot = (*this)[xx];
	do {
		while ( (i < count) && (*pivot > *((*this)[i])) )
		{
			i++;
		}

		while ( (j >= 0) && (*pivot < *((*this)[j])) )
		{
			j--;
		}

		if ( i <= j )
		{
			VValueSingle* temp = (*this)[i];
			(*this)[i] = (*this)[j];
			(*this)[j] = temp;

			i++;
			j--;
		}
	} while ( i <= j );


	if ( l < j ) subsort( l, j );

	if ( i < r  ) subsort( i, r );
}


Boolean DB4DArrayOfVValues::Find(VValueSingle& inValue, const VCompareOptions& inOptions) const
{
	Boolean res = false;

	sLONG nb = GetCount();

	sLONG l = 0, r = nb, k;

	while (r-l>=0)
	{
		k = l + ((r-l) / 2);
		if ((k<0) || (k>=nb))
			break;
		ValPtr ch = (*this)[k];
		if (ch != nil)
		{
			CompareResult comp = inValue.CompareToSameKindWithOptions(ch, inOptions);
			if (comp == CR_EQUAL)
			{
				res = true;
				break;
			}
			else
			{
				if (comp == CR_SMALLER)
				{
					r = k-1;
				}
				else
				{
					l = k+1;
				}
			}
		}
		else
			break;
	}

	return res;
}


Boolean DB4DArrayOfVValues::FindWithDataPtr(void* inDataPtr, const VCompareOptions& inOptions) const
{
	Boolean res = false;

	sLONG nb = GetCount();

	sLONG l = 0, r = nb, k;

	while (r-l>=0)
	{
		k = l + ((r-l) / 2);
		if ((k<0) || (k>=nb))
			break;

		ValPtr ch = (*this)[k];
		if (ch != nil)
		{
			CompareResult comp = XBOX::VValue::InvertCompResult(ch->Swap_CompareToSameKindPtrWithOptions(inDataPtr, inOptions));
			if (comp == CR_EQUAL)
			{
				res = true;
				break;
			}
			else
			{
				if (comp == CR_BIGGER)
				{
					r = k-1;  // si la valeur de l'element de tableau est superieure a la valeur recherchee
				}
				else
				{
					l = k+1;
				}
			}
		}
		else
			break;
	}

	return res;
}



Boolean DB4DArrayOfVValues::Find(VValueSingle& inValue, Boolean isBeginWith, Boolean isLike) const
{
	Boolean res = false;

	sLONG nb = GetCount();

	sLONG l = 0, r = nb, k;

	while (r-l>=0)
	{
		k = l + ((r-l) / 2);
		if ((k<0) || (k>=nb))
			break;
		ValPtr ch = (*this)[k];
		if (ch != nil)
		{
			CompareResult comp;
			if (isLike)
			{
				if (isBeginWith)
					comp = inValue.CompareBeginingToSameKind_Like(ch);
				else
					comp = inValue.CompareToSameKind_Like(ch);
			}
			else
			{
				if (isBeginWith)
					comp = inValue.CompareBeginingToSameKind(ch);
				else
					comp = inValue.CompareToSameKind(ch);
			}
			if (comp == CR_EQUAL)
			{
				res = true;
				break;
			}
			else
			{
				if (comp == CR_SMALLER)
				{
					r = k-1;
				}
				else
				{
					l = k+1;
				}
			}
		}
		else
			break;
	}

	return res;
}




Boolean DB4DArrayOfVValues::FindWithDataPtr(void* inDataPtr, Boolean isBeginWith, Boolean isLike) const
{
	Boolean res = false;

	sLONG nb = GetCount();

	sLONG l = 0, r = nb, k;

	while (r-l>=0)
	{
		k = l + ((r-l) / 2);
		if ((k<0) || (k>=nb))
			break;

		ValPtr ch = (*this)[k];
		if (ch != nil)
		{
			CompareResult comp;
			if (isLike)
			{
				if (isBeginWith)
					comp = ch->IsTheBeginingToSameKindPtr_Like(inDataPtr);
				else
					comp = XBOX::VValue::InvertCompResult(ch->Swap_CompareToSameKindPtr_Like(inDataPtr));
			}
			else
			{
				if (isBeginWith)
					comp = ch->IsTheBeginingToSameKindPtr(inDataPtr);
				else
					comp = XBOX::VValue::InvertCompResult(ch->Swap_CompareToSameKindPtr(inDataPtr));
			}
			if (comp == CR_EQUAL)
			{
				res = true;
				break;
			}
			else
			{
				if (comp == CR_BIGGER)
				{
					r = k-1;  // si la valeur de l'element de tableau est superieure a la valeur recherchee
				}
				else
				{
					l = k+1;
				}
			}
		}
		else
			break;
	}

	return res;
}


const VValueSingle* DB4DArrayOfVValues::GetFirst()
{
	if (GetCount() > 0)
		return (*this)[0];
	else
		return nil;
}


const VValueSingle* DB4DArrayOfVValues::GetLast()
{
	sLONG n = GetCount();
	if (n > 0)
		return (*this)[n-1];
	else
		return nil;
}


sLONG DB4DArrayOfVValues::Count() const
{
	return GetCount();
}



Boolean DB4DArrayOfVValues::IsAllNull() const
{
	for (VArrayPtrOf<VValueSingle*>::ConstIterator cur = First(), end = End(); cur != end; cur++)
	{
		if (!(*cur)->IsNull())
			return false;
	}
	return true;
}


Boolean DB4DArrayOfVValues::AtLeastOneNull() const
{
	for (VArrayPtrOf<VValueSingle*>::ConstIterator cur = First(), end = End(); cur != end; cur++)
	{
		if ((*cur)->IsNull())
			return true;
	}
	return false;
}


VError DB4DArrayOfVValues::PutInto(VStream& outStream) const
{
	VError err;
	sLONG i, nb = 0;
	nb = GetCount();
	err = outStream.PutLong(nb);

	if (err == VE_OK)
	{
		for (i = 0; i < nb; i++)
		{
			const VValueSingle* ch = (*this)[i];
			if (ch == nil)
				err = ThrowBaseError(memfull, DBaction_SavingArrayOfValues);
			else
			{
				err = outStream.PutValue(*ch, true);
			}
			if (err != VE_OK)
				break;
		}
	}

	if (err != VE_OK)
		err = ThrowBaseError(VE_DB4D_CANNOT_SAVE_ARRAY, DBaction_SavingArrayOfValues);

	return err;
}


VError DB4DArrayOfVValues::GetFrom(VStream& inStream)
{
	VError err;
	sLONG i, nb = 0;
	err = inStream.GetLong(nb);
	{
		if (SetAllocatedSize(nb))
		{
			for (i = 0; i < nb; i++)
			{
				ValPtr ch = (ValPtr)inStream.GetValue();
				if (ch == nil)
				{
					err = ThrowBaseError(memfull, DBaction_LoadingArrayOfValues);
					break;
				}
				else
					Add(ch);
			}
		}
		else
		{
			err = ThrowBaseError(memfull, DBaction_LoadingArrayOfValues);
		}

	}

	if (err != VE_OK)
		err = ThrowBaseError(VE_DB4D_CANNOT_LOAD_ARRAY, DBaction_LoadingArrayOfValues);
	return err;
}


sLONG DB4DArrayOfVValues::GetSignature() const
{
	return kDBAVSIG;
}

#endif



//=================================================================================



AutoSeqNumber::AutoSeqNumber(Base4D* owner, sLONG num)
{
	fIsValid = false;
	fOwner = owner;
	fNum = num;
	fCurToken = 1;
}


sLONG8 AutoSeqNumber::GetStartingValue() const 
{ 
	occupe();
	sLONG8 result = fInitialValue; 
	libere();
	return result;
}

void AutoSeqNumber::SetStartingValue(sLONG8 initialvalue) 
{ 
	occupe();

	fInitialValue = initialvalue; 
	if (fIsValid) 
		setmodif(true, fOwner, nil); 

	libere();
}


VError AutoSeqNumber::InitForNew()
{
	fInitialValue = 1;
	fID.Regenerate();
	return VE_OK;
}


// ------------------------------------


void AutoSeqSimpleOnDisk::SwapBytes()
{
	ByteSwap(&typ);
	ByteSwap(&filler);
	ID.SwapBytes();
	ByteSwap(&InitialValue);
	ByteSwap(&CurValue);
}


VError AutoSeqNumberSimple::LibereEspaceDisk(VDB4DProgressIndicator* InProgress)
{
	VError err = VE_OK;

	if (testAssert(fIsValid))
	{
		assert(getaddr()>0);
		setmodif(false, fOwner, nil);
		err = fOwner->libereplace(getaddr(), sizeof(AutoSeqSimpleOnDisk) + kSizeDataBaseObjectHeader, nil, this);
	}

	return err;
}


sLONG AutoSeqNumberSimple::saveobj(void)
{
#if debuglogwrite
	VString wherefrom(L"AutoSeqNumberSimple SaveObj");
	VString* whx = &wherefrom;
#else
	VString* whx = nil;
#endif

	VError err;
	AutoSeqSimpleOnDisk ASSOD;
	ASSOD.typ = (sLONG)DB4D_AutoSeq_Simple;
	ASSOD.filler = 0;
	ASSOD.CurValue = fCurValue;
	ASSOD.InitialValue = fInitialValue;
	fID.ToBuffer(ASSOD.ID);

	assert(fIsValid);

	DataBaseObjectHeader tag(&ASSOD, sizeof(ASSOD), DBOH_AutoSeqNumberSimple, fNum, -1);
	err = tag.WriteInto(fOwner, getaddr(), whx);
	if (err == VE_OK)
		err=fOwner->writelong(&ASSOD, sizeof(ASSOD), getaddr(), kSizeDataBaseObjectHeader, whx);

	return(sizeof(ASSOD));

}


VError AutoSeqNumberSimple::loadobj(DataAddr4D addr, DataBaseObjectHeader& tag)
{
	VError err;

	assert(addr>0);
	AutoSeqSimpleOnDisk ASSOD;
	setaddr(addr);

	err=fOwner->readlong(&ASSOD, sizeof(ASSOD), addr, kSizeDataBaseObjectHeader);
	if (err == VE_OK)
		err = tag.ValidateCheckSum(&ASSOD, sizeof(ASSOD));

	if (err == VE_OK)
	{
		if (tag.NeedSwap())
			ASSOD.SwapBytes();
		fCurValue = ASSOD.CurValue;
		fInitialValue = ASSOD.InitialValue;
		fID.FromBuffer(ASSOD.ID);
		fIsValid = true;
	}

	return err;
}


VError AutoSeqNumberSimple::InitForNew()
{
	VError err = AutoSeqNumber::InitForNew();
	fCurValue = fInitialValue;
	if (err == VE_OK)
	{
		DataAddr4D ou = fOwner->findplace(kSizeDataBaseObjectHeader + sizeof(AutoSeqSimpleOnDisk), nil, err, 0, this);
		if (ou > 0)
		{
			sLONG inutile;
			err = fOwner->SetSeqNumAddr(fNum, ou, kSizeDataBaseObjectHeader + sizeof(AutoSeqSimpleOnDisk), inutile);
			if (err == VE_OK)

			{
				setaddr(ou);
				setmodif(true, fOwner, nil);
				fIsValid = true;
			}

			if (err != VE_OK)
			{
				setmodif(false, fOwner, nil);
				fOwner->libereplace(ou, kSizeDataBaseObjectHeader + sizeof(AutoSeqSimpleOnDisk), nil, this);
			}

		}
	}

	return err;
}


void AutoSeqNumberSimple::SetCurrentValue(sLONG8 currentvalue, bool canBeLower) 
{ 
	occupe();

	if (canBeLower || fCurValue < currentvalue)
	{
		fCurValue = currentvalue; 
		if (fIsValid) 
			setmodif(true, fOwner, nil);
	}

	libere();
}

sLONG8 AutoSeqNumberSimple::GetCurrentValue() const 
{ 
	occupe();
	sLONG8 result = fCurValue; 
	libere();
	return result;
}

sLONG8 AutoSeqNumberSimple::GetNewValue(DB4D_AutoSeqToken& ioToken)
{
	sLONG8 result;

	occupe();
	if (fIsValid)
	{
		if (ioToken == 0)
		{
			fCurToken++;
			if (fCurToken == 0)
				fCurToken++;
			ioToken = fCurToken;
			result = fCurValue;
			fCurValue++;
			fWaitingForValidate[ioToken] = result;
		}
		else
		{
			MapOfContextAndValue::iterator deja = fWaitingForValidate.find(ioToken);

			if (deja != fWaitingForValidate.end())
			{
				result = deja->second;
			}
			else
			{
				//assert(false);
				result = -1;
			}
		}
	}
	else
		result = -1;

	libere();

	return result;
}


void AutoSeqNumberSimple::ValidateValue(DB4D_AutoSeqToken inToken, Table* inTable, BaseTaskInfo* context)
{
	bool canlog = false;
	occupe();
	sLONG8 lastvalue = fCurValue;
	if (fIsValid)
	{
		MapOfContextAndValue::iterator deja = fWaitingForValidate.find(inToken);
		if (deja != fWaitingForValidate.end())
		{
			fWaitingForValidate.erase(deja);
			setmodif(true, fOwner, nil);
			canlog = true;
		}
	}
	libere();
	if (canlog && inTable != nil)
		inTable->WriteSeqNumToLog(context, lastvalue);
}


void AutoSeqNumberSimple::InvalidateValue(DB4D_AutoSeqToken inToken)
{
	occupe();
	if (fIsValid)
	{
		MapOfContextAndValue::iterator deja = fWaitingForValidate.find(inToken);
		if (deja != fWaitingForValidate.end())
		{
			if (deja->second == (fCurValue-1))
			{
				--fCurValue;
			}
			fWaitingForValidate.erase(deja);
			//setmodif(true, fOwner, nil);
		}
	}
	libere();
}



// ------------------------------------


void AutoSeqNoHoleOnDisk::SwapBytes()
{
	ByteSwap(&typ);
	ByteSwap(&filler);
	ID.SwapBytes();
	ByteSwap(&InitialValue);
	ByteSwap(&CurValue);
	ByteSwap(&nbHoles);
}


sLONG AutoSeqNumberNoHole::CalcLen()
{
	return (sLONG)(sizeof(AutoSeqNoHoleOnDisk) + kSizeDataBaseObjectHeader + (fHoles.size() * sizeof(sLONG8)));
}


VError AutoSeqNumberNoHole::LibereEspaceDisk(VDB4DProgressIndicator* InProgress)
{
	VError err = VE_OK;

	if (testAssert(fIsValid))
	{
		assert(getaddr()>0);
		setmodif(false, fOwner, nil);
		err = fOwner->libereplace(getaddr(), CalcLen(), nil, this);
	}

	return err;
}


sLONG AutoSeqNumberNoHole::saveobj(void)
{
#if debuglogwrite
	VString wherefrom(L"AutoSeqNumberNoHole Saveobj");
	VString* whx = &wherefrom;
#else
	VString* whx = nil;
#endif

	VError err;
	AutoSeqNoHoleOnDisk ASNHOD;
	ASNHOD.typ = (sLONG)DB4D_AutoSeq_Simple;
	ASNHOD.filler = 0;
	ASNHOD.CurValue = fCurValue;
	ASNHOD.InitialValue = fInitialValue;
	fID.ToBuffer(ASNHOD.ID);
	ASNHOD.nbHoles = (sLONG)fHoles.size();

	assert(fIsValid);

	DataBaseObjectHeader tag(&ASNHOD, sizeof(ASNHOD), DBOH_AutoSeqNumberNoHole, fNum, -1);
	err = tag.WriteInto(fOwner, getaddr(), whx);
	if (err == VE_OK)
		err=fOwner->writelong(&ASNHOD, sizeof(ASNHOD), getaddr(), kSizeDataBaseObjectHeader, whx);
	if (err == VE_OK && ASNHOD.nbHoles>0)
		err=fOwner->writelong(&fHoles[0], ASNHOD.nbHoles * sizeof(sLONG8), getaddr(), kSizeDataBaseObjectHeader + sizeof(ASNHOD), whx );


	return(CalcLen());

}


VError AutoSeqNumberNoHole::loadobj(DataAddr4D addr, DataBaseObjectHeader& tag)
{
	VError err;

	assert(addr>0);
	AutoSeqNoHoleOnDisk ASNHOD;
	setaddr(addr);

	err=fOwner->readlong(&ASNHOD, sizeof(ASNHOD), addr, kSizeDataBaseObjectHeader);
	if (err == VE_OK)
		err = tag.ValidateCheckSum(&ASNHOD, sizeof(ASNHOD));

	if (err == VE_OK)
	{
		if (tag.NeedSwap())
			ASNHOD.SwapBytes();
		fCurValue = ASNHOD.CurValue;
		fInitialValue = ASNHOD.InitialValue;
		fID.FromBuffer(ASNHOD.ID);
		try
		{
			fHoles.assign(ASNHOD.nbHoles, 0);
		}
		catch (...)
		{
			err = fOwner->ThrowError(memfull, DBaction_LoadingAutoSeqHeader);
		}
		if (err == VE_OK)
		{
			err=fOwner->readlong(&fHoles[0], ASNHOD.nbHoles * sizeof(sLONG8), addr, kSizeDataBaseObjectHeader + sizeof(ASNHOD));
			if (err == VE_OK)
			{
				if (tag.NeedSwap())
					ByteSwap(&fHoles[0], ASNHOD.nbHoles);
				fAntelen = CalcLen();
				fIsValid = true;
			}
		}
	}

	if (err != VE_OK)
		err = fOwner->ThrowError(VE_DB4D_CANNOT_LOAD_AUTOSEQ, DBaction_LoadingAutoSeqHeader);
	return err;
}

void AutoSeqNumberNoHole::SetCurrentValue(sLONG8 currentvalue, bool canBeLower) 
{ 
	occupe();

	if (canBeLower || fCurValue < currentvalue)
	{
		fCurValue = currentvalue; 
		if (fIsValid) 
			setmodif(true, fOwner, nil); 
	}

	libere();
}

sLONG8 AutoSeqNumberNoHole::GetCurrentValue() const 
{ 
	occupe();
	sLONG8 result = fCurValue; 
	libere();
	return result;
}


VError AutoSeqNumberNoHole::InitForNew()
{
	VError err = AutoSeqNumber::InitForNew();
	fCurValue = fInitialValue;
	if (err == VE_OK)
	{
		DataAddr4D ou = fOwner->findplace(kSizeDataBaseObjectHeader + sizeof(AutoSeqNoHoleOnDisk), nil, err, 0, this);
		if (ou > 0)
		{
			sLONG inutile;
			err = fOwner->SetSeqNumAddr(fNum, ou, kSizeDataBaseObjectHeader + sizeof(AutoSeqNoHoleOnDisk), inutile);
			if (err == VE_OK)
			{
				setaddr(ou);
				fAntelen = CalcLen();
				setmodif(true, fOwner, nil);
				fIsValid = true;
			}

			if (err != VE_OK)
			{
				setmodif(false, fOwner, nil);
				fOwner->libereplace(ou, kSizeDataBaseObjectHeader + sizeof(AutoSeqNoHoleOnDisk), nil, this);
			}

		}
	}

	return err;
}


VError AutoSeqNumberNoHole::save()
{
	VError err = VE_OK;
	sLONG len = CalcLen();
	if (adjuste(len) != adjuste(fAntelen))
	{
		DataAddr4D oldaddr = getaddr();
		DataAddr4D ou = fOwner->findplace(len, nil, err, 0, this);
		if (ou>0)
		{
			sLONG inutile;
			fOwner->SetSeqNumAddr(fNum, ou, len, inutile);
			ChangeAddr(ou, fOwner, nil);
			fOwner->libereplace(oldaddr, fAntelen, nil, this);
			fAntelen = len;
		}
	}
	else
	{
		sLONG inutile;
		if (len != fAntelen)
			fOwner->SetSeqNumAddr(fNum, getaddr(), len, inutile);
		fAntelen = len;
		setmodif(true, fOwner, nil);
	}

	return err;
}


sLONG8 AutoSeqNumberNoHole::GetNewValue(DB4D_AutoSeqToken& ioToken)
{
	sLONG8 result;

	occupe();
	if (fIsValid)
	{
		if (ioToken == 0)
		{
			fCurToken++;
			if (fCurToken == 0)
				fCurToken++;
			ioToken = fCurToken;
			if (fHoles.size() > 0)
			{
				result = fHoles[fHoles.size()-1];
				fHoles.pop_back();
			}
			else
			{
				result = fCurValue;
				fCurValue++;
			}
			fWaitingForValidate[ioToken] = result;
		}
		else
		{
			MapOfContextAndValue::iterator deja = fWaitingForValidate.find(ioToken);

			if (deja != fWaitingForValidate.end())
			{
				result = deja->second;
			}
			else
			{
				assert(false);
				result = -1;
			}
		}
	}
	else
		result = -1;

	libere();

	return result;
}


void AutoSeqNumberNoHole::ValidateValue(DB4D_AutoSeqToken inToken, Table* inTable, BaseTaskInfo* context)
{
	bool canlog = false;
	occupe();
	sLONG8 lastvalue = fCurValue;
	if (fIsValid)
	{
		MapOfContextAndValue::iterator deja = fWaitingForValidate.find(inToken);
		if (deja != fWaitingForValidate.end())
		{
			fWaitingForValidate.erase(deja);
			save();
			canlog = true;
		}
	}
	libere();
	if (canlog && inTable != nil)
		inTable->WriteSeqNumToLog(context, lastvalue);
		
}


void AutoSeqNumberNoHole::InvalidateValue(DB4D_AutoSeqToken inToken)
{
	occupe();
	if (fIsValid)
	{
		MapOfContextAndValue::iterator deja = fWaitingForValidate.find(inToken);
		if (deja != fWaitingForValidate.end())
		{
			if (deja->second == (fCurValue-1))
			{
				--fCurValue;
			}
			else
			{
				try 
				{
					fHoles.push_back(deja->second);
				}
				catch (...)
				{
					// on ne peut pas inserer le numero dans le tableau de trous
				}
			}
			fWaitingForValidate.erase(deja);
			save();
		}
	}
	libere();
}






// ----------------------------------------------------------------------------------- 


class StrRefByPos
{
public:
	sLONG fPos;
	sLONG fLen;
	const VString* fRef;
	VCompareOptions* fOptions;

};


struct CompareStrRefByPos
{
	bool operator()(const StrRefByPos& p1, const StrRefByPos& p2) const
	{
		const UniChar* c1 = p1.fRef->GetCPointer() + p1.fPos;
		const UniChar* c2 = p2.fRef->GetCPointer() + p2.fPos;
		
		return p1.fOptions->GetIntlManager()->CompareString(c1, p1.fLen, c2, p2.fLen, *(p1.fOptions)) == CR_SMALLER;
	}
};


//typedef set<StrRefByPos, CompareStrRefByPos, cache_allocator<StrRefByPos> > KeyPosMap;
typedef set<StrRefByPos, CompareStrRefByPos> KeyPosMap;

//VCriticalSection buildkeywordsMutex;

VError BuildKeyWords(const VString& source, DB4DKeyWordList& outKeyWords, const VCompareOptions& inOptions)
{
	VError err = VE_OK;

	KeyPosMap KeysPos;

	#if USE_OLD_WORD_BOUNDARIES
	sLONG curpos = 0;
	sLONG totlen = source.GetLength();
	const UniChar* curC = source.GetCPointer();

	try 
	{
		while (curpos<totlen)
		{
			UniChar c = *curC;
			if (VDBMgr::IsItaDeadChar(c, inOptions.GetIntlManager()))
			{
				curpos++;
				curC++;
			}
			else
			{
				StrRefByPos KP;
				KP.fOptions = (VCompareOptions*)&inOptions;
				KP.fLen = 0;
				KP.fPos = curpos;
				KP.fRef = &source;
				do 
				{
					KP.fLen++;
					curpos++;
					curC++;

					if (curpos<totlen)
					{
						c = *curC;
						if (VDBMgr::IsItaDeadChar(c, inOptions.GetIntlManager()))
							break;
					}
				} while(curpos<totlen);
				KeysPos.insert(KP);
				curpos++;
				curC++;
			}
		}
	}
	catch (...)
	{
		err = ThrowBaseError(memfull, DBaction_BuildingKeywords);
	}

	#else

	bool ok;
	std::vector<std::pair<VIndex,VIndex> > boundaries;
	{
	//	VTaskLock lock(&buildkeywordsMutex);
		ok = inOptions.GetIntlManager()->GetWordBoundaries( source, boundaries);
	}
	
	if (ok)
	{
		try 
		{
			for( std::vector<std::pair<VIndex,VIndex> >::const_iterator i = boundaries.begin() ; i != boundaries.end() ; ++i)
			{
				StrRefByPos KP;
				KP.fOptions = (VCompareOptions*)&inOptions;
				KP.fPos = i->first - 1;
				KP.fLen = i->second;
				KP.fRef = &source;
				KeysPos.insert(KP);
			}
		}
		catch (...)
		{
			err = ThrowBaseError(memfull, DBaction_BuildingKeywords);
		}
	}
	else
	{
		err = ThrowBaseError(memfull, DBaction_BuildingKeywords);
	}
	#endif

	if (err == VE_OK)
	{
		KeyPosMap::iterator p;
		KeyPosMap::iterator pEnd = KeysPos.end();

		for (p = KeysPos.begin(); p != pEnd; p++)
		{
			VString* s = new VString;
			if (s == nil)
			{
				err = ThrowBaseError(memfull, DBaction_BuildingKeywords);
			}
			else
			{
				s->FromBlock(source.GetCPointer()+(*p).fPos, (*p).fLen * 2, VTC_UTF_16);
				if (s->GetLength() == 0)
				{
					err = ThrowBaseError(memfull, DBaction_BuildingKeywords);
				}
				else
				{
					if (!outKeyWords.Add(s))
					{
						err = ThrowBaseError(memfull, DBaction_BuildingKeywords);
					}
				}
			}

			if (err != VE_OK)
			{
				if (s != nil)
					delete s;

				break;
			}
		}
	}

	return err;
}




// ----------------------------------------------------------------------------------- 



VValue *VStringUTF8_info::Generate() const
{
	return new VStringUTF8();
}


VValue *VStringUTF8_info::Generate(VBlob* inBlob) const
{
	return new VStringUTF8();
}


VValue *VStringUTF8_info::LoadFromPtr( const void *inBackStore, bool /*inRefOnly*/) const
{
	const char* string = (reinterpret_cast<const char*>(inBackStore))+4;
	VIndex nbbytes = *((uLONG*)inBackStore);
	VString *s = new VStringUTF8;
	if (s)
		s->FromBlock(string, nbbytes, VTC_UTF_8);
	return s;
}


void* VStringUTF8_info::ByteSwapPtr( void *inBackStore, bool inFromNative) const
{
	char* string = (reinterpret_cast<char*>(inBackStore))+4;
	uLONG nbbytes = *((uLONG*)inBackStore);
	if (!inFromNative)
		ByteSwap( &nbbytes);

	ByteSwap( (uLONG*) inBackStore);

	return string + nbbytes + 4;

}


VSize VStringUTF8_info::GetSizeOfValueDataPtr( const void* inPtrToValueData) const
{
	return (*(sLONG*)inPtrToValueData)+4;
}


VSize VStringUTF8_info::GetAvgSpace() const
{
	return 32;
}


CompareResult VStringUTF8_info::CompareTwoPtrToData( const void* inPtrToValueData1, const void* inPtrToValueData2, Boolean inDiacritical) const
{
	Utf8ToUtf16 buffer1(inPtrToValueData1), buffer2(inPtrToValueData2);

	return VIntlMgr::GetDefaultMgr()->CompareString(
		buffer1.GetPtr(), buffer1.GetLength(), 
		buffer2.GetPtr(), buffer2.GetLength(), inDiacritical != 0);
}


CompareResult VStringUTF8_info::CompareTwoPtrToDataBegining( const void* inPtrToValueData1, const void* inPtrToValueData2, Boolean inDiacritical) const
{
	Utf8ToUtf16 buffer1(inPtrToValueData1), buffer2(inPtrToValueData2);

	sLONG len = buffer1.GetLength();
	if (len > buffer2.GetLength()) len = buffer2.GetLength();
	return VIntlMgr::GetDefaultMgr()->CompareString(buffer1.GetPtr(), len, buffer2.GetPtr(), buffer2.GetLength(), inDiacritical != 0);
}



CompareResult VStringUTF8_info::CompareTwoPtrToData_Like( const void* inPtrToValueData1, const void* inPtrToValueData2, Boolean inDiacritical) const
{
	Utf8ToUtf16 buffer1(inPtrToValueData1), buffer2(inPtrToValueData2);

	return VIntlMgr::GetDefaultMgr()->CompareString_Like(
		buffer1.GetPtr(), buffer1.GetLength(), 
		buffer2.GetPtr(), buffer2.GetLength(), inDiacritical != 0);
}


CompareResult VStringUTF8_info::CompareTwoPtrToDataBegining_Like( const void* inPtrToValueData1, const void* inPtrToValueData2, Boolean inDiacritical) const
{
	Utf8ToUtf16 buffer1(inPtrToValueData1), buffer2(inPtrToValueData2);

	sLONG len = buffer1.GetLength();
	if (len > buffer2.GetLength()) len = buffer2.GetLength();
	return VIntlMgr::GetDefaultMgr()->CompareString_Like(buffer1.GetPtr(), len, buffer2.GetPtr(), buffer2.GetLength(), inDiacritical != 0);
}


CompareResult VStringUTF8_info::CompareTwoPtrToDataWithOptions( const void* inPtrToValueData1, const void* inPtrToValueData2, const VCompareOptions& inOptions) const
{
	Utf8ToUtf16 buffer1(inPtrToValueData1), buffer2(inPtrToValueData2);

	if (inOptions.IsBeginsWith())
	{
		sLONG len = Min(buffer1.GetLength(), buffer2.GetLength());
		if (inOptions.IsLike())
			return inOptions.GetIntlManager()->CompareString_Like( buffer1.GetPtr(), len, buffer2.GetPtr(), len, inOptions.IsDiacritical());
		else
			return inOptions.GetIntlManager()->CompareString( buffer1.GetPtr(), buffer1.GetLength(), buffer2.GetPtr(), buffer2.GetLength(), inOptions.IsDiacritical());
	}
	else
	{
		if (inOptions.IsLike())
			return inOptions.GetIntlManager()->CompareString_Like( buffer1.GetPtr(), buffer1.GetLength(), buffer2.GetPtr(), buffer2.GetLength(), inOptions.IsDiacritical());
		else
			return inOptions.GetIntlManager()->CompareString( buffer1.GetPtr(), buffer1.GetLength(), buffer2.GetPtr(), buffer2.GetLength(), inOptions.IsDiacritical());
	}
}




								// --------------------------------


const VValueInfo* VStringUTF8::GetValueInfo() const
{
	return &sInfo;
}

/*
void VStringUTF8::_AdjusteUTF8Buffer() const
{
	if (fUTF8Buffer == nil || IsDirty())
	{
		if (fUTF8Buffer != nil)
		{
			VObject::GetMainMemMgr()->Free(fUTF8Buffer);
		}
		VSize nbbytes = GetSpace() - 4;
		fUTF8Buffer = (char*)VObject::GetMainMemMgr()->Malloc(nbbytes + 8, false, 'utf8');
		VIndex nbbytesconsumed = 0;
		VSize nbbytesproduced = 0;
		if (fLength > 0)
			VTextConverters::Get()->GetUTF8FromUnicodeConverter()->Convert(fString, fLength, &nbbytesconsumed, fUTF8Buffer, nbbytes + 8, &nbbytesproduced);
		assert(nbbytesproduced == nbbytes);
		assert(nbbytesconsumed == fLength * 2);
		fUTF8Length = nbbytesproduced;
	}
}
*/


VSize VStringUTF8::GetSpace(VSize inMax) const
{
	VSize result;
	if (fUTF8NbBytes == -1 || IsDirty())
	{
		VIndex nbbytesconsumed;
		VSize nbbytes = 0;
		if (fLength > 0)
			VTextConverters::Get()->GetUTF8FromUnicodeConverter()->Convert(fString, fLength, &nbbytesconsumed, nil, 1000000000, &nbbytes);
		fUTF8NbBytes = (sLONG)nbbytes;
	}
	
	result = fUTF8NbBytes + 4;

	if (inMax == 0)
		return result;
	else
	{
		if (inMax < result)
			return inMax;
		else
			return result;
	}
}


void* VStringUTF8::LoadFromPtr( const void* inData, Boolean inRefOnly)
{
	char* UTF8Buffer;
	sLONG nbbytes = *((sLONG*)inData);
	if (nbbytes > 0)
	{
		uLONG* source = ((uLONG*)inData)+1;
		FromBlock((uCHAR*)source, nbbytes, VTC_UTF_8);
	}
	else
		Clear();
	fUTF8NbBytes = nbbytes;
	ClearDirty();
	return ((char*)inData) + nbbytes + 4;
}


void* VStringUTF8::WriteToPtr( void* ioData, Boolean inRefOnly, VSize inMax) const
{
	Utf16toUtf8 buffer(fString, fLength);
	*((sLONG*)ioData) = buffer.GetNbBytes();
	std::copy(buffer.GetPtr(), buffer.GetPtr()+buffer.GetNbBytes(), ((char*)ioData)+4);
	return ((char*)ioData)+4+buffer.GetNbBytes();
}



CompareResult VStringUTF8::CompareToSameKindPtr( const void* inPtrToValueData, Boolean inDiacritical) const
{
	Utf8ToUtf16 buffer(inPtrToValueData);
	return VIntlMgr::GetDefaultMgr()->CompareString(GetCPointer(), GetLength(), buffer.GetPtr(), buffer.GetLength(), inDiacritical != 0);
}


CompareResult VStringUTF8::Swap_CompareToSameKindPtr( const void* inPtrToValueData, Boolean inDiacritical) const
{
	Utf8ToUtf16 buffer(inPtrToValueData);
	return VIntlMgr::GetDefaultMgr()->CompareString(buffer.GetPtr(), buffer.GetLength(), GetCPointer(), GetLength(), inDiacritical != 0);
}


CompareResult VStringUTF8::CompareBeginingToSameKindPtr( const void* inPtrToValueData, Boolean inDiacritical) const
{
	Utf8ToUtf16 buffer(inPtrToValueData);
	sLONG len = GetLength();
	if (len > buffer.GetLength()) 
		len = buffer.GetLength();
	return VIntlMgr::GetDefaultMgr()->CompareString(GetCPointer(), len, buffer.GetPtr(), buffer.GetLength(), inDiacritical != 0);
}


CompareResult VStringUTF8::IsTheBeginingToSameKindPtr( const void* inPtrToValueData, Boolean inDiacritical) const
{
	Utf8ToUtf16 buffer(inPtrToValueData);
	sLONG	len = Min(buffer.GetLength(), (sLONG)GetLength());
	return VIntlMgr::GetDefaultMgr()->CompareString(GetCPointer(), GetLength(), buffer.GetPtr(), len, inDiacritical != 0);
}


Boolean VStringUTF8::EqualToSameKindPtr( const void* inPtrToValueData, Boolean inDiacritical) const
{
	Utf8ToUtf16 buffer(inPtrToValueData);
	return VIntlMgr::GetDefaultMgr()->EqualString(GetCPointer(), GetLength(), buffer.GetPtr(), buffer.GetLength(), inDiacritical != 0);
}



CompareResult VStringUTF8::CompareToSameKindPtr_Like( const void* inPtrToValueData, Boolean inDiacritical) const
{
	Utf8ToUtf16 buffer(inPtrToValueData);
	return VIntlMgr::GetDefaultMgr()->CompareString_Like(GetCPointer(), GetLength(), buffer.GetPtr(), buffer.GetLength(), inDiacritical != 0);
}


CompareResult VStringUTF8::Swap_CompareToSameKindPtr_Like( const void* inPtrToValueData, Boolean inDiacritical) const
{
	Utf8ToUtf16 buffer(inPtrToValueData);
	return VIntlMgr::GetDefaultMgr()->CompareString_Like(buffer.GetPtr(), buffer.GetLength(), GetCPointer(), GetLength(), inDiacritical != 0);
}


CompareResult VStringUTF8::CompareBeginingToSameKindPtr_Like( const void* inPtrToValueData, Boolean inDiacritical) const
{
	Utf8ToUtf16 buffer(inPtrToValueData);
	sLONG len = GetLength();
	if (len > buffer.GetLength()) len = buffer.GetLength();
	return VIntlMgr::GetDefaultMgr()->CompareString_Like(GetCPointer(), len, buffer.GetPtr(), buffer.GetLength(), inDiacritical != 0);
}


CompareResult VStringUTF8::IsTheBeginingToSameKindPtr_Like( const void* inPtrToValueData, Boolean inDiacritical) const
{
	Utf8ToUtf16 buffer(inPtrToValueData);
	sLONG	len = Min(buffer.GetLength(), (sLONG)GetLength());
	return VIntlMgr::GetDefaultMgr()->CompareString_Like(GetCPointer(), GetLength(), buffer.GetPtr(), len, inDiacritical != 0);
}


Boolean VStringUTF8::EqualToSameKindPtr_Like( const void* inPtrToValueData, Boolean inDiacritical) const
{
	Utf8ToUtf16 buffer(inPtrToValueData);
	return VIntlMgr::GetDefaultMgr()->EqualString_Like(GetCPointer(), GetLength(), buffer.GetPtr(), buffer.GetLength(), inDiacritical != 0);
}


Boolean VStringUTF8::Swap_EqualToSameKindPtr_Like( const void* inPtrToValueData, Boolean inDiacritical) const
{
	Utf8ToUtf16 buffer(inPtrToValueData);
	return VIntlMgr::GetDefaultMgr()->EqualString_Like(buffer.GetPtr(), buffer.GetLength(), GetCPointer(), GetLength(), inDiacritical != 0);
}



CompareResult VStringUTF8::CompareToSameKindPtrWithOptions( const void* inPtrToValueData, const VCompareOptions& inOptions) const
{
	Utf8ToUtf16 buffer(inPtrToValueData);
	if (inOptions.IsBeginsWith())
	{
		sLONG len = Min(buffer.GetLength(), (sLONG)GetLength());
		if (inOptions.IsLike())
			return inOptions.GetIntlManager()->CompareString_Like( GetCPointer(), len, buffer.GetPtr(), len, inOptions.IsDiacritical());
		else
			return inOptions.GetIntlManager()->CompareString( GetCPointer(), len, buffer.GetPtr(), len, inOptions.IsDiacritical());
	}
	else
	{
		if (inOptions.IsLike())
			return inOptions.GetIntlManager()->CompareString_Like( GetCPointer(), GetLength(), buffer.GetPtr(), buffer.GetLength(), inOptions.IsDiacritical());
		else
			return inOptions.GetIntlManager()->CompareString( GetCPointer(), GetLength(), buffer.GetPtr(), buffer.GetLength(), inOptions.IsDiacritical());
	}
}


CompareResult VStringUTF8::Swap_CompareToSameKindPtrWithOptions( const void* inPtrToValueData, const VCompareOptions& inOptions) const
{
	Utf8ToUtf16 buffer(inPtrToValueData);
	if (inOptions.IsBeginsWith())
	{
		sLONG len = Min(buffer.GetLength(), (sLONG)GetLength());
		if (inOptions.IsLike())
			return inOptions.GetIntlManager()->CompareString_Like( buffer.GetPtr(), len, GetCPointer(), len, inOptions.IsDiacritical());
		else
			return inOptions.GetIntlManager()->CompareString( buffer.GetPtr(), len, GetCPointer(), len, inOptions.IsDiacritical());
	}
	else
	{
		if (inOptions.IsLike())
			return inOptions.GetIntlManager()->CompareString_Like( buffer.GetPtr(), buffer.GetLength(), GetCPointer(), GetLength(), inOptions.IsDiacritical());
		else
			return inOptions.GetIntlManager()->CompareString( buffer.GetPtr(), buffer.GetLength(), GetCPointer(), GetLength(), inOptions.IsDiacritical());
	}
}


bool VStringUTF8::EqualToSameKindPtrWithOptions( const void* inPtrToValueData, const VCompareOptions& inOptions) const
{
	Utf8ToUtf16 buffer(inPtrToValueData);
	if (inOptions.IsLike())
		return inOptions.GetIntlManager()->EqualString_Like( GetCPointer(), GetLength(), buffer.GetPtr(), buffer.GetLength(), inOptions.IsDiacritical());
	else
		return inOptions.GetIntlManager()->EqualString( GetCPointer(), GetLength(), buffer.GetPtr(), buffer.GetLength(), inOptions.IsDiacritical());
}



VStringUTF8* VStringUTF8::Clone() const
{
	return new VStringUTF8( *this);
}


const VStringUTF8::InfoType	VStringUTF8::sInfo;



// ----------------------------------------------------------------------------------- 



const VBlobTextUTF8::InfoType	VBlobTextUTF8::sInfo;

VValue *VBlobTextUTF8Info::Generate() const
{
	return new VBlobTextUTF8(0, nil);
}


VValue *VBlobTextUTF8Info::Generate(VBlob* inBlob) const
{
	assert(false);// should never be called
	return new VBlobTextUTF8(0, nil);
}




VBlobTextUTF8::VBlobTextUTF8( sLONG * /*inNumBlob*/, BlobTextUTF8 *inBlob)
{
	fTextSwitchSize = 0;
	fBlob = inBlob;
	if (inBlob != nil)
	{
		assert( inBlob->GetRefCount() > 0);
		inBlob->Retain();

		void* string = nil;
		sLONG nbbytes;
		inBlob->GetRawData( &string, &nbbytes);
		if (string != nil)
		{
			FromBlock(string, nbbytes, VTC_UTF_8);
		}
	}
}


VError VBlobTextUTF8::ReloadFromOutsidePath()
{
	if (fBlob == nil)
		return VE_OK;
	else
	{
		VError err = fBlob->ReloadFromOutsidePath();
		if (err == VE_OK)
		{
			void* string = nil;
			sLONG nbbytes;
			((BlobTextUTF8*)fBlob)->GetRawData( &string, &nbbytes);
			if (string != nil)
			{
				FromBlock(string, nbbytes, VTC_UTF_8);
			}
		}
		else
			Clear();
		return err;
	}
}


// ---------------------------




const VBlobText::InfoType	VBlobText::sInfo;

VValue *VBlobTextInfo::Generate() const
{
	return new VBlobText(0, nil);
}


VValue *VBlobTextInfo::Generate(VBlob* inBlob) const
{
	assert(false);// should never be called
	return new VBlobText(0, nil);
}

void* VBlobTextInfo::ByteSwapPtr( void *inBackStore, bool inFromNative) const
{
	sLONG lentext = *(sLONG*)inBackStore;
	if (!inFromNative)
		ByteSwap(&lentext);

	ByteSwap((sLONG*)inBackStore);
	if (lentext <= -10)
	{
		lentext = -lentext - 10;
		ByteSwap(((sWORD*)inBackStore)+2, lentext);
		return ((sWORD*)inBackStore) + 2 + lentext;
	}
	else
		return ((sLONG*)inBackStore)+1;
}


VBlobText::VBlobText( sLONG * /*inNumBlob*/, BlobText *inBlob)
{
	fTextSwitchSize = 0;
	fBlob = inBlob;
	fForcePathIfEmpty = false;
	if (inBlob != nil)
	{
		assert( inBlob->GetRefCount() > 0);
		inBlob->Retain();
	
		UniPtr string = nil;
		sLONG length;
		inBlob->GetCString( &string, &length);
		if (string != nil)
		{
			//SetCPointer( string, length, (length + 1) * sizeof( UniChar));
			FromBlock(string, length*sizeof(UniChar), VTC_UTF_16);
		}
	}
}

VBlobText::~VBlobText()
{
	if (fBlob != nil)
		fBlob->Release();
}


VError VBlobText::ReloadFromOutsidePath()
{
	if (fBlob == nil)
		return VE_OK;
	else
	{
		VError err = fBlob->ReloadFromOutsidePath();
		if (err == VE_OK)
		{
			UniPtr string = nil;
			sLONG length;
			fBlob->GetCString( &string, &length);
			if (string != nil)
			{
				FromBlock(string, length*sizeof(UniChar), VTC_UTF_16);
			}
		}
		else
			Clear();
		return err;
	}
}


void VBlobText::SetHintRecNum(sLONG TableNum, sLONG FieldNum, sLONG inHint)
{
	if (fBlob != nil)
		fBlob->SetHintRecNum(TableNum, FieldNum, inHint);
}


bool VBlobText::ForcePathIfEmpty() const
{
	return fBlob!= nil && fBlob->IsOutsidePath() && fForcePathIfEmpty;
}

void VBlobText::SetForcePathIfEmpty(bool x)
{
	fForcePathIfEmpty = x;
}

void VBlobText::AssociateRecord(void* primkey, sLONG FieldNum)
{
	if (fBlob != nil)
		fBlob->AssociateRecord((PrimKey*)primkey, FieldNum);
}




const VValueInfo *VBlobText::GetValueInfo() const
{
	return &sInfo;
}


VBlobText *VBlobText::Clone() const
{
	VBlobText* result = new VBlobText;
	if (result != nil)
	{
		result->fTextSwitchSize = fTextSwitchSize;
		result->FromString(*this);
		if (fBlob != nil)
		{
			result->fBlob = new BlobText((DataTable*)fBlob->GetDF());
		}
		else
			result->fBlob = nil;
	}
	return result;
}


VValue* VBlobText::FullyClone(bool ForAPush) const
{
	VBlobText* result = Clone();
	if (ForAPush)
	{
		if (fBlob == nil)
			result->fBlob = nil;
		else
		{
			fBlob->CopyBaseInto(result->fBlob);
			result->fBlob->SetInTrans(fBlob->InTrans());
			result->fBlob->SetNewInTrans(fBlob->IsNewInTrans());
		}
	}
	return result;
}


void VBlobText::RestoreFromPop(void* context)
{
	fBlob->RestoreFromPop(context);
}



void VBlobText::Detach(Boolean inMayEmpty)
{
	if (testAssert(fBlob != nil))
	{
		DataTable* df = fBlob->GetDF();

		/*
		if (GetCPointer() == fBlob->GetCPointer())
		{
			if (inMayEmpty)
			{
				Clear();
			}
			else
			{
				sLONG len = GetLength();
				Clear();
				FromBlock(fBlob->GetCPointer(), len*sizeof(UniChar), VTC_UTF_16);
			}
		}
		*/

		fBlob->Release();
		fBlob = new BlobText(df);
	}
}


void VBlobText::SetReservedBlobNumber(sLONG inBlobNum)
{
	if (fBlob != nil)
		fBlob->SetReservedBlobNumber(inBlobNum);
}

VError VBlobText::Flush( void *inDataPtr, void *InContext) const
{

	VError err = VE_OK;
	if (fBlob != nil)
	{
		// inDataPtr points to the numblob

		// the VString is no longer the owner of the string pointer
		VBlobText *thethis = const_cast<VBlobText*>(this);

		if (fBlob != nil)
		{
			if (fBlob->NeedDuplicate() || GetCurrentTransaction((BaseTaskInfo*)InContext) != nil)
			{
				//CheckAgainst = GetCPointer();
				BlobText* newblob = (BlobText*)fBlob->Clone(false);
				if (newblob == nil)
					err = fBlob->GetDF()->ThrowError(memfull, DBaction_CopyingBlob);
				else
				{
					//newblob->SetInTrans(fBlob->InTrans());
					newblob->SetNewInTrans(fBlob->IsNewInTrans());
				}
				fBlob->Release();
				((VBlobText*)this)->fBlob = newblob;
			}
		}

		if (fBlob != nil)
		{
			fBlob->GetFrom(thethis->GetCPointer(), thethis->GetLength()*2);

			if  (!ForcePathIfEmpty() && (IsEmpty() || (fTextSwitchSize != 0 && GetLength() <= fTextSwitchSize)))
			{
				if (fBlob->SomethingToDelete())
				{
					fBlob->GetDF()->DelBlob( fBlob, (BaseTaskInfo*)InContext);	
				}

				if (IsEmpty())
				{
					if (inDataPtr != nil)
						*(sLONG *)inDataPtr = -1;

				}
				else
				{
					// rien a faire, tout a ete fait dans le WriteToPtr
				}
			}
			else
			{		
				err = fBlob->GetDF()->SaveBlob( fBlob, (BaseTaskInfo*)InContext, ForcePathIfEmpty());

				if (inDataPtr != nil && !fBlob->IsOutsidePath())
					*(sLONG *)inDataPtr = fBlob->GetNum();
			}
		}

		if (fBlob == nil && inDataPtr != nil)
			*(sLONG *)inDataPtr = -1;
	}
	else
	{
		//err = VE_DB4D_NOTIMPLEMENTED;
	}

	if (fBlob == nil)
		((VBlobText*)this)->SetNull(true);

	return err;
}



VSize VBlobText::GetSpace(VSize /* inMax */) const
{
	assert(fBlob != nil);
	if (!ForcePathIfEmpty())
	{
		if (IsEmpty())
			return sizeof(sLONG);

		if (fTextSwitchSize != 0)
		{
			if (GetLength() <= fTextSwitchSize && !IsEmpty())
			{
				return sizeof(sLONG) + GetLength() * sizeof(UniChar); // quand le text est assez petit, il sera directement stoque dans la fiche
			}
		}
	}
	if (fBlob != nil && fBlob->IsOutsidePath())
	{
		return fBlob->GetPathLength();
	}
	return sizeof(sLONG); // un blob ne stoque que son numero dans une fiche
}


VSize VBlobText::GetFullSpaceOnDisk() const
{
	return GetLength() * sizeof(UniChar) + sizeof(sLONG);
}


BlobText* VBlobText::GetBlob4D(BaseTaskInfo* context, bool duplicateIfNecessary) 
{
	if (duplicateIfNecessary)
	{
		if (fBlob->NeedDuplicate() || GetCurrentTransaction(context) != nil)
		{
			BlobText* newblob = (BlobText*)fBlob->Clone(true);
			if (newblob == nil)
			{
				ThrowBaseError(memfull);
				return nil;
			}
			else
			{
				newblob->SetNewInTrans(fBlob->IsNewInTrans());
			}
			fBlob->Release();
			fBlob = newblob;
		}
	}
	return fBlob;
}


void* VBlobText::WriteToPtr(void* pData, Boolean pRefOnly, VSize inMax ) const
{
	sLONG len = sizeof(sLONG);

	bool forceifempty = fBlob != nil && fBlob->IsOutsidePath() && fForcePathIfEmpty;

	if (!forceifempty && IsEmpty())
		*(sLONG *) pData = -1;
	else
	{
		if ( !forceifempty && ((fTextSwitchSize != 0) && (GetLength() <= fTextSwitchSize)) )
		{
			len = len + GetLength() * 2;
			*(sLONG *) pData = -GetLength() - 10; // on reserve les 10 premieres valeurs negatives ( -1 sert deja pour les blobs vides)
			::memcpy( ((UniChar*)pData)+2, GetCPointer(), GetLength()*sizeof(UniChar));
			return (char *) pData + (sizeof(sLONG) + GetLength() * sizeof(UniChar));
		}
		else
		{
			if (fBlob != nil && fBlob->IsOutsidePath())
			{
				fBlob->SavePathTo(pData);
				return (char *) pData + fBlob->GetPathLength();
			}
			else
			{
				if (fBlob == nil)
					*(sLONG *) pData = -1;
				else
					*(sLONG *) pData = fBlob->GetNum(); // un blob ne stoque que son numero dans une fiche
			}
		}
	}
	return (char *) pData + len;
}


void VBlobText::DoNullChanged()
{
	if (IsNull())
	{
		Clear();
		if (fBlob != nil)
		{
			fBlob->GetFrom(nil, 0);
		}
	}
}


void VBlobText::SetOutsidePath(const VString& inPosixPath, bool inIsRelative)
{
	if (fBlob != nil)
		fBlob->SetOutsidePath(inPosixPath, inIsRelative);
}

void VBlobText::GetOutsidePath(VString& outPosixPath, bool* outIsRelative)
{
	if (fBlob != nil)
	{
		outPosixPath = fBlob->GetComputedPath(true);
		if (outIsRelative != nil)
			*outIsRelative = fBlob->IsPathRelative();
	}
}


void VBlobText::GetTrueOutsidePath(VString& outPosixPath, bool* outIsRelative)
{
	if (fBlob != nil)
	{
		outPosixPath = fBlob->GetTrueOutsidePath();
		if (outIsRelative != nil)
			*outIsRelative = fBlob->IsPathRelative();
	}
}


void VBlobText::SetOutsideSuffixe(const VString& inExtention)
{
	if (fBlob != nil)
		fBlob->SetOutsideSuffixe(inExtention);
}

bool VBlobText::IsOutsidePath()
{
	if (fBlob == nil)
		return false;
	else
		return fBlob->IsOutsidePath();
}


//==============================================================================================



const VBlob4DWithPtrAsRawPict::InfoType	VBlob4DWithPtrAsRawPict::sInfo;


const VValueInfo *VBlob4DWithPtrAsRawPict::GetValueInfo() const
{
	return &sInfo;
}


//====================================================

VError VBlob4DWithPtrAsRawPict::WriteToStream( VStream* ioStream, sLONG inParam) const
{
	if (fBlob->GetDataLen() == 0)
	{
		return ioStream->PutLong(0);
	}
	else
		return fBlob->WriteToStream(ioStream, false);
}


//====================================================

const VBlob4DWithPtr::InfoType	VBlob4DWithPtr::sInfo;


const VValueInfo *VBlob4DWithPtr::GetValueInfo() const
{
	return &sInfo;
}


void* VBlob4DWithPtrInfo::ByteSwapPtr( void *inBackStore, bool inFromNative) const
{
	sLONG lenblob = *(sLONG*)inBackStore;
	if (!inFromNative)
		ByteSwap(&lenblob);

	ByteSwap((sLONG*)inBackStore);
	if (lenblob <= -10)
	{
		lenblob = -lenblob - 10;
		return ((uBYTE*)inBackStore) + 4 + lenblob;
	}
	else
		return ((sLONG*)inBackStore)+1;
}


void VBlob4DWithPtr::DoNullChanged()
{
	if (IsNull())
	{
		if (fBlob != nil)
		{
			fBlob->SetSize(0);
		}
	}
}


void* VBlob4DWithPtr::WriteToPtr(void* pData,Boolean pRefOnly, VSize inMax ) const
{
	sLONG len = sizeof(sLONG);

	bool forceifempty = fBlob != nil && fBlob->IsOutsidePath() && fForcePathIfEmpty;

	if ( !forceifempty && (fBlob == nil || fBlob->IsEmpty()) )
		*(sLONG *) pData = -1; // un blob ne stoque que son numero dans une fiche
	else
	{
		if (!forceifempty && ((fBlobSwitchSize != 0) && (GetSize() <= fBlobSwitchSize)) )
		{
			len = len + (sLONG)GetSize();
			*(sLONG *) pData = -(sLONG)GetSize() - 10; // on reserve les 10 premieres valeurs negatives ( -1 sert deja pour les blobs vides)
			::memcpy( ((UniChar*)pData)+2, fBlob->GetDataPtr(), GetSize());
			return (char *) pData + (sizeof(sLONG) + GetSize());
		}
		else
		{
			if (fBlob->IsOutsidePath())
			{
				fBlob->SavePathTo(pData);
				return (char *) pData + fBlob->GetPathLength();
			}
			else
			{
				if (fBlob == nil)
					*(sLONG *) pData = -1;
				else
					*(sLONG *) pData = fBlob->GetNum(); // un blob ne stoque que son numero dans une fiche
			}
		}
	}
	return (char *) pData + 4;
}


VSize VBlob4DWithPtr::GetSpace (VSize /* inMax */) const
{
	assert(fBlob != nil);
	if (!ForcePathIfEmpty())
	{
		if (fBlob == nil || fBlob->IsEmpty())
			return sizeof(sLONG);
		if (fBlobSwitchSize != 0)
		{
			if (GetSize() <= fBlobSwitchSize && GetSize() > 0)
			{
				return sizeof(sLONG) + GetSize(); // quand le blob est assez petit, il sera directement stoque dans la fiche
			}
		}
	}
	if (fBlob != nil && fBlob->IsOutsidePath())
	{
		return fBlob->GetPathLength();
	}
	return sizeof(sLONG); // un blob ne stoque que son numero dans une fiche
}


void VBlob4DWithPtr::SetReservedBlobNumber(sLONG inBlobNum)
{
	if (fBlob != nil)
		fBlob->SetReservedBlobNumber(inBlobNum);
}

Boolean VBlob4DWithPtr::FromValueSameKind( const VValue* inValue)
{
	Boolean isOk = false;

	Clear();

	VBlobWithPtr *blb = dynamic_cast<VBlobWithPtr*> ( ( VValueSingle* ) inValue );

	if (blb != NULL)
	{
		PutData(blb->GetDataPtr(),blb->GetSize());
		VError		vError = SetSize(blb->GetSize());
		isOk = (vError == VE_OK);
		xbox_assert(isOk);
	}
	else
	{
		VBlob4DWithPtr *blb4D = dynamic_cast<VBlob4DWithPtr*> ( ( VValueSingle* ) inValue );
		if (blb4D != NULL)
		{
			if (blb4D->fBlob != nil)
				PutData(blb4D->fBlob->GetDataPtr(), blb4D->fBlob->GetDataLength());
			isOk = true;
		}
		else
			xbox_assert(false);
	}

	return isOk;
}

void VBlob4DWithPtr::GetValue( VValueSingle& outValue) const
{
	xbox_assert(false);
}

void VBlob4DWithPtr::FromValue( const VValueSingle& inValue)
{
	bool	isOk = FromValueSameKind ( &inValue );

	xbox_assert(isOk);
}


void VBlob4DWithPtr::SetOutsidePath(const VString& inPosixPath, bool inIsRelative)
{
	if (fBlob != nil)
		fBlob->SetOutsidePath(inPosixPath, inIsRelative);
}

void VBlob4DWithPtr::GetOutsidePath(VString& outPosixPath, bool* outIsRelative)
{
	if (fBlob != nil)
	{
		outPosixPath = fBlob->GetComputedPath(true);
		if (outIsRelative != nil)
			*outIsRelative = fBlob->IsPathRelative();
	}
}


void VBlob4DWithPtr::GetTrueOutsidePath(VString& outPosixPath, bool* outIsRelative)
{
	if (fBlob != nil)
	{
		outPosixPath = fBlob->GetTrueOutsidePath();
		if (outIsRelative != nil)
			*outIsRelative = fBlob->IsPathRelative();
	}
}


void VBlob4DWithPtr::SetOutsideSuffixe(const VString& inExtention)
{
	if (fBlob != nil)
		fBlob->SetOutsideSuffixe(inExtention);
}

bool VBlob4DWithPtr::IsOutsidePath()
{
	if (fBlob == nil)
		return false;
	else
		return fBlob->IsOutsidePath();
}


VError VBlob4DWithPtr::ReloadFromOutsidePath()
{
	if (fBlob == nil)
		return VE_OK;
	else
		return fBlob->ReloadFromOutsidePath();
}


void VBlob4DWithPtr::SetHintRecNum(sLONG TableNum, sLONG FieldNum, sLONG inHint)
{
	if (fBlob != nil)
		fBlob->SetHintRecNum(TableNum, FieldNum, inHint);
}


bool VBlob4DWithPtr::ForcePathIfEmpty() const
{
	return fBlob!= nil && fBlob->IsOutsidePath() && fForcePathIfEmpty;
}

void VBlob4DWithPtr::SetForcePathIfEmpty(bool x)
{
	fForcePathIfEmpty = x;
}

void VBlob4DWithPtr::AssociateRecord(void* primkey, sLONG FieldNum)
{
	if (fBlob != nil)
		fBlob->AssociateRecord((PrimKey*)primkey, FieldNum);
}


VError VBlob4DWithPtr::Flush( void *inDataPtr, void *InContext) const
{
	// inDataPtr points to the numblob
	//fBlob->GetFrom(fData, fDataLen);
	/*
	if (fBlob->NeedDuplicate())
	{
		//CheckAgainst = GetCPointer();
		BlobWithPtr* newblob = (BlobWithPtr*)fBlob->Clone(false);
		if (newblob == nil)
			err = fBlob->GetDF()->ThrowError(memfull, DBaction_CopyingBlob);
		else
		{
			newblob->SetNewInTrans(fBlob->IsNewInTrans());
		}
		fBlob->Release();
		((VBlob4DWithPtr*)this)->fBlob = newblob;
	}
	*/


	VError err = VE_OK;
	
	if (fBlob != nil)
	{
		if (!ForcePathIfEmpty() && (fBlob->IsEmpty() || (fBlobSwitchSize != 0 && (sLONG)GetSize() <= fBlobSwitchSize)) )
		{
			if (fBlob->SomethingToDelete())
			{
				fBlob->GetDF()->DelBlob( fBlob, (BaseTaskInfo*)InContext);	
			}

			if (fBlob->IsEmpty())
			{
				if (inDataPtr != nil)
					*(sLONG *)inDataPtr = -1;

			}
			else
			{
				// rien a faire, tout a ete fait dans le WriteToPtr
			}
		}
		else
		{
			err = fBlob->GetDF()->SaveBlob( fBlob, (BaseTaskInfo*)InContext, ForcePathIfEmpty());
			if (inDataPtr != nil && !fBlob->IsOutsidePath())
				*(sLONG *)inDataPtr = fBlob->GetNum();
		}
	}
	else
	{
		err = ThrowBaseError(VE_DB4D_BLOB_IS_NULL, DBactionFinale);
		if (inDataPtr != nil)
			*(sLONG *)inDataPtr = 0;
	}


#if 0 && VERSIONDEBUG
	if (fBlob != nil && fBlob->GetDF()->GetDB()->GetStructure() == nil)
	{
		if (fBlob->GetNum() == 0)
		{
			uWORD* p = (uWORD*)fBlob->GetDataPtr();
			if (*p != 4444)
			{
				p = p; // put a break here
				assert(*p == 4444);
			}
		}
	}
#endif

	return err;
}

VBlob4DWithPtr::VBlob4DWithPtr( const VBlob4DWithPtr& inBlob)
{
	if (inBlob.fBlob != nil)
		inBlob.fBlob->Retain();
	fBlob = inBlob.fBlob;
	fBlobSwitchSize = inBlob.fBlobSwitchSize;
	fForcePathIfEmpty = inBlob.fForcePathIfEmpty;

}


VBlob4DWithPtr *VBlob4DWithPtr::Clone() const
{
	return new VBlob4DWithPtr( *this);
}


void VBlob4DWithPtr::RestoreFromPop(void* context)
{
	fBlob->RestoreFromPop(context);
}


VValue* VBlob4DWithPtr::FullyClone(bool ForAPush) const
{
	VValue* result = nil;

	Blob4D* newblob = (fBlob != nil) ? fBlob->Clone(true) : nil;
	if (newblob != nil)
	{
		if (ForAPush)
		{
			newblob->SetInTrans(fBlob->InTrans());
			newblob->SetNewInTrans(fBlob->IsNewInTrans());
		}
		else
			newblob->Detach();
		result = new VBlob4DWithPtr(nil, (BlobWithPtr*)newblob);
		newblob->Release();
	}

	return result;
}


void VBlob4DWithPtr::Detach(Boolean inMayEmpty)
{
	if (fBlob != nil)
	{
		DataTable* df = fBlob->GetDF();

		if (inMayEmpty)
		{
			fBlob->Release();
			fBlob = new BlobWithPtr(df);
		}
		else
		{
			Blob4D* newblob = fBlob->Clone(true);
			fBlob->Release();
			if (testAssert(newblob != nil))
			{
				newblob->Detach();
			}
			fBlob = (BlobWithPtr*)newblob;
		}
	}
}


VSize	VBlob4DWithPtr::GetSize () const
{
	if (fBlob == nil)
		return 0;
	else
		return (VSize)fBlob->GetDataLen();
}


VError VBlob4DWithPtr::_CheckNeedDuplicate(Boolean withdata)
{
	VError err = VE_OK;
	if ((fBlob != nil) && (fBlob->NeedDuplicate()))
	{
		Blob4D* newblob = fBlob->Clone(withdata);

		if (newblob == nil)
		{
			err = fBlob->GetDF()->ThrowError(memfull, DBaction_CopyingBlob);
			SetNull(true);
		}
		else
		{
			newblob->SetNewInTrans(fBlob->IsNewInTrans());
		}
		fBlob->Release();
		fBlob = (BlobWithPtr*)newblob;
	}
	return err;
}


VError VBlob4DWithPtr::SetSize (VSize inNewSize)
{
	if (fBlob == nil)
		return ThrowBaseError(VE_DB4D_BLOB_IS_NULL, DBactionFinale);
	else
	{
		VError err = _CheckNeedDuplicate(inNewSize != 0);
		if (err == VE_OK)
			err = fBlob->SetSize((sLONG)inNewSize);
		return err;
	}
}


VError VBlob4DWithPtr::GetData (void* inBuffer, VSize inNbBytes, VIndex inOffset, VSize* outNbBytesCopied) const
{
	VError err = VE_OK;
	sLONG actuallen = 0;
	if (fBlob == nil)
	{
		err = ThrowBaseError(VE_DB4D_BLOB_IS_NULL, DBactionFinale);
	}
	else
	{
		err = fBlob->PutInto(inBuffer, (sLONG)inNbBytes, (sLONG)inOffset, &actuallen);
	}
	if (outNbBytesCopied != nil)
		*outNbBytesCopied = (VSize)actuallen;

	return err;
}



VError VBlob4DWithPtr::PutData (const void* inBuffer, VSize inNbBytes, VIndex inOffset)
{
	VError err = VE_OK;
	if (fBlob == nil)
		err = ThrowBaseError(VE_DB4D_BLOB_IS_NULL, DBaction_CopyingBlob);
	else
	{
		err = _CheckNeedDuplicate(true);

		if (err == VE_OK)
		{
			err = fBlob->GetFrom(inBuffer, (sLONG)inNbBytes, (sLONG)inOffset);
		}
		if (err == VE_OK)
			SetNull(false);
	}

	if (err != VE_OK)
	{
		if (fBlob == nil)
			err = ThrowBaseError(VE_DB4D_CANNOTALLOCATEBLOB, DBaction_CopyingBlob);
		else
			err = fBlob->GetDF()->ThrowError(VE_DB4D_CANNOTALLOCATEBLOB, DBaction_CopyingBlob);
	}
	return err;
}


Boolean VBlob4DWithPtr::EqualToSameKind( const VValue* inValue, Boolean inDiacritical) const
{
	if ( inValue == 0 || inValue-> IsNull ( ) )
		return false;

	BlobWithPtr*			blobWPtrThis = GetBlob4D ( );
	if ( blobWPtrThis == 0 )
		return false;

	bool					bIsTypeSafe = true;

	const VBlobWithPtr*		blobPtr = dynamic_cast < const VBlobWithPtr* > ( inValue );
	VSize					nSize = 0;
	const void*				ptrData = 0;
	if ( blobPtr != 0 )
	{
		nSize = blobPtr-> GetSize ( );
		ptrData = blobPtr-> GetDataPtr ( );
	}
	else
	{
		const VBlob4DWithPtr*		blob4DPtr = dynamic_cast < const VBlob4DWithPtr* > ( inValue );
		if ( blob4DPtr != 0 )
		{
			nSize = blob4DPtr-> GetSize ( );
			BlobWithPtr*			blobWPtr = ( const_cast < VBlob4DWithPtr* > ( blob4DPtr ) ) -> GetBlob4D ( );
			if ( blobWPtr == 0 )
			{
				xbox_assert ( false );
				bIsTypeSafe = false;
			}
			else
				ptrData = blobWPtr-> GetDataPtr ( );
		}
		else
		{
			xbox_assert ( false );
			bIsTypeSafe = false;
		}
	}

	if ( !bIsTypeSafe )
		return false;

	if ( GetSize ( ) != nSize )
		return false;

	return ::memcmp ( GetBlob4D ( )-> GetDataPtr ( ), ptrData, nSize ) == 0 ? true : false;
}


VError VBlob4DWithPtr::ReadFromStream( VStream* ioStream, sLONG inParam)
{
	VError err = _CheckNeedDuplicate(false);
	if (err == VE_OK)
		err = fBlob->ReadFromStream(ioStream);
	return err;
}


VError VBlob4DWithPtr::WriteToStream( VStream* ioStream, sLONG inParam) const
{
	return fBlob->WriteToStream(ioStream);
}


VBlob4DWithPtr::~VBlob4DWithPtr()
{
	//fData = nil; // because the fBlob owns the Handle
	//fDataLen = 0;
	if (fBlob != nil)
		fBlob->Release();
}



VBlob4DWithPtr::VBlob4DWithPtr( sLONG * /*inNumBlob*/, BlobWithPtr *inBlob)
{
	fBlobSwitchSize = 0;

	if (inBlob == nil)
	{
		inBlob = new BlobWithPtr(nil);
	}
	else
	{
		assert( inBlob->GetRefCount() > 0);

		inBlob->Retain();
	}
	fBlob = inBlob;
	fForcePathIfEmpty = false;

//	fData = (VPtr)fBlob->GetDataPtr();
//	fDataLen = fBlob->GetDataLen();
}





/* ------------------------------------------------------------------------------------------------ */

const VSubTableDB4D::InfoType VSubTableDB4D::sInfo;


void VSubTableDB4D::xInit()
{
	fSubTable = nil;
	fSource = nil;
	fDest = nil;
	fParent = nil;
	fIsRemote = false;
	fWasLoaded = false;
}


VSubTableDB4D::VSubTableDB4D(sLONG8 inValue)
{
	fValue = inValue;
	xInit();
}


VSubTableDB4D::VSubTableDB4D(const VSubTableDB4D& inOriginal)
{
	fValue = inOriginal.fValue;
	xInit();
	fIsRemote = inOriginal.fIsRemote;
}


VSubTableDB4D::VSubTableDB4D(sLONG8* inDataPtr, Boolean inInit)
{
	if (inInit)
		Clear();
	else
		fValue = *inDataPtr;
	xInit();
}


VSubTableDB4D::~VSubTableDB4D()
{
	sLONG i,nb = fSubRecs.GetCount();
	for (i=1; i<=nb; i++)
	{
		if (fSubRecs[i] != nil)
		{
			fSubRecs[i]->SetRecParent(nil, 0);
			//assert(fSubRecs[i]->GetRefCount() == 1);
			fSubRecs[i]->Release();
		}
	}

	nb = fDeletedSubRecs.GetCount();
	for (i=1; i<=nb; i++)
	{
		if (fDeletedSubRecs[i] != nil)
		{
			fDeletedSubRecs[i]->SetRecParent(nil, 0);
			//assert(fDeletedSubRecs[i]->GetRefCount() == 1);
			fDeletedSubRecs[i]->Release();
		}
	}

	if (fSubTable != nil)
		fSubTable->Release();
	
	if (fSource != nil)
		fSource->Release();

	if (fDest != nil)
		fDest->Release();
}


void VSubTableDB4D::Detach()
{
	sLONG i,nb = fSubRecs.GetCount();
	for (i=1; i<=nb; i++)
	{
		if (fSubRecs[i] != nil)
			fSubRecs[i]->Release();
	}
	fSubRecs.SetCount(0);

	nb = fDeletedSubRecs.GetCount();
	for (i=1; i<=nb; i++)
	{
		if (fDeletedSubRecs[i] != nil)
			fDeletedSubRecs[i]->Release();
	}
	fDeletedSubRecs.SetCount(0);
	FromLong8(0);
}


VValue* VSubTableDB4D::FullyClone(bool ForAPush) const
{
	BaseTaskInfo* context = nil;
	VError err = VE_OK;

	VSubTableDB4D* result = new VSubTableDB4D(*this);
	result->fWasLoaded = fWasLoaded;
	result->fDest = RetainRefCountable(fDest);
	result->fSource = RetainRefCountable(fSource);
	result->fSubTable = RetainRefCountable(fSubTable);
	result->fParent = fParent;

	if (fParent != nil)
		context = fParent->GetContext();

	sLONG i,nb = fSubRecs.GetCount();
	result->fSubRecs.SetCount(nb, nil);
	for (i=1; i<=nb; i++)
	{
		if (fSubRecs[i] != nil)
			result->fSubRecs[i] = fSubRecs[i]->CloneForPush(context, err);
	}

	nb = fDeletedSubRecs.GetCount();
	result->fDeletedSubRecs.SetCount(nb, nil);
	for (i=1; i<=nb; i++)
	{
		if (fDeletedSubRecs[i] != nil)
			result->fDeletedSubRecs[i] = fDeletedSubRecs[i]->CloneForPush(context, err);
	}

	result->fSubSel.CopyFrom(fSubSel);

	return result;
}


void VSubTableDB4D::RestoreFromPop(void* context)
{
	sLONG i,nb = fSubRecs.GetCount();
	for (i=1; i<=nb; i++)
	{
		if (fSubRecs[i] != nil)
			fSubRecs[i]->RestoreFromPop();
	}

	nb = fDeletedSubRecs.GetCount();
	for (i=1; i<=nb; i++)
	{
		if (fDeletedSubRecs[i] != nil)
			fDeletedSubRecs[i]->RestoreFromPop();
	}
}




const VValueInfo *VSubTableDB4D::GetValueInfo() const
{
	return &sInfo;
}


VError VSubTableDB4D::InitForNewRecord(FicheInMem* parent, Field* cri, BaseTaskInfo* context)
{
	VError err = VE_OK;

	assert(cri != nil);
	Relation* rel = cri->GetSubTableRel();
	if (rel != nil)
	{
		//fDest = rel->GetDest();
		fDest = rel->GetSource();
		if (testAssert(fDest != nil))
		{
			fDest->Retain();
			Table* xtab = fDest->GetOwner();
			if (testAssert(xtab != nil))
			{
				xtab->Retain();
			}
			fSubTable = xtab;
			fSource = cri;
			fSource->Retain();
			fParent = parent;

			if (testAssert(fParent != nil))
			{
				if (!fParent->IsRemote())
				{
					sLONG8 IDSUBREC;
					IDSUBREC = GetLong8();
					if (IDSUBREC == 0)
					{
						if (fParent->IsNew())
						{
							IDSUBREC = fParent->GetAutoSeqValue();
						}
						else
							IDSUBREC = fParent->GetNum()+1;
						FromLong8(IDSUBREC);
						fParent->Touch(fSource);
					}
				}
			}

		}
	}

	if (parent != nil)
		fIsRemote = parent->IsRemote();

	fDeletedSubRecs.SetCount(0);
	fWasLoaded = true;

	return err;
}


VError VSubTableDB4D::LoadSubRecords(FicheInMem* parent, Field* cri, DB4D_Way_of_Locking HowToLock, BaseTaskInfo* context, Boolean& outSomeLocked, Boolean* outEnoughMem)
{
	VError err = VE_OK;
	if (testAssert(!fIsRemote))
	{
		fWasLoaded = true;
		outSomeLocked = false;
		assert(cri != nil);
		Relation* rel = cri->GetSubTableRel();
		if (rel != nil)
		{
			//fDest = rel->GetDest();
			fDest = rel->GetSource();
			if (testAssert(fDest != nil))
			{
				fDest->Retain();
				Table* xtab = fDest->GetOwner();
				if (testAssert(xtab != nil))
				{
					xtab->Retain();
				}
				fSubTable = xtab;
				fSource = cri;
				fSource->Retain();
				fParent = parent;

				Selection	*sel;
				err = rel->ActivateOneToMany(parent, sel, context, false, false, DB4D_Do_Not_Lock);
				if (err == VE_OK)
				{
					if (sel != nil)
					{
						if (fSubRecs.SetAllocatedSize(sel->GetQTfic()+2) && fSubSel.SetAllocatedSize(sel->GetQTfic()+2))
						{
							DataTable* df = sel->GetParentFile();
							SelectionIterator itersel(sel);
							sLONG curfic = itersel.FirstRecord();
							while (curfic != -1)
							{
								FicheInMem* rec = df->LoadRecord(curfic, err, HowToLock, context, true, false, nil, outEnoughMem);
								if (err == VE_OK && rec != nil)
								{
									if (fSubRecs.Add(rec))
									{
										rec->SetRecParent(parent, fSource->GetPosInRec());
										if (HowToLock != DB4D_Do_Not_Lock && rec->ReadOnlyState())
										{
											outSomeLocked = true;
										}
									}
									else
										err = parent->GetDF()->ThrowError(memfull, DBaction_LoadingSubRecord);
								}
								
								if (err != VE_OK)
								{
									if (rec!=nil)
										rec->Release();
									break;
								}
								curfic = itersel.NextRecord();
							}

							if (err == VE_OK)
							{
								err = AllSubRecords(context);
							}
						}
						else
							err = parent->GetDF()->ThrowError(memfull, DBaction_LoadingSubRecord);

						sel->Release();
					}
				}
			}
		}

		fDeletedSubRecs.SetCount(0);

		if (err != VE_OK && parent != nil)
			err = parent->GetDF()->ThrowError(VE_DB4D_CANNOTLOADRECORD, DBaction_LoadingSubRecord);
	}
	return err;
}


VError VSubTableDB4D::SaveSubRecords(Field* cri, BaseTaskInfo* context, Transaction* &ioSubTrans)
{
	VError err = VE_OK;
	if (testAssert(!fIsRemote))
	{
		if (fSubTable != nil)
		{
			Relation* rel = cri->GetSubTableRel();
			sLONG i,nb = fSubRecs.GetCount();
			for (i=1; i<=nb; i++)
			{
				FicheInMem* rec = fSubRecs[i];
				if (rec != nil)
				{
					if (rec->IsRecordModified())
					{
						if (context != nil)
						{
							context->MustNotCheckRefInt(rel, rec);
							if (ioSubTrans == nil)
							{
								VError errtrans;
								ioSubTrans = context->StartTransaction(errtrans);
							}
						}
						VError err2 = fSubTable->GetDF()->SaveRecord(rec, context);
						if (context != nil)
							context->CheckRefIntAgain(rel, rec);
						if (err2 != VE_OK)
							err = err2;
					}
				}
			}

			nb = fDeletedSubRecs.GetCount();
			for (i=1; i<=nb; i++)
			{
				FicheInMem* rec = fDeletedSubRecs[i];
				if (rec != nil)
				{
					if (rec->IsNew())
					{
						rec->Release();
					}
					else
					{
						if (ioSubTrans == nil && context != nil)
						{
							VError errtrans;
							ioSubTrans = context->StartTransaction(errtrans);
						}
						VError err2 = fSubTable->GetDF()->DelRecord(rec, context);
						if (err2 != VE_OK)
							err = err2;
						rec->Release();
					}
				}
			}
			fDeletedSubRecs.SetCount(0);
		}

		if (err != VE_OK)
			err = ThrowBaseError(VE_DB4D_CANNOT_SAVE_SUBRECORD, DBaction_SavingSubRecord);
	}
	return err;
}


sLONG VSubTableDB4D::FindSubRecNumInSubSel(sLONG nfic)
{
	sLONG result = -1;

	sLONG i,nb = fSubSel.GetCount();
	for (i=1; i<=nb; i++)
	{
		if (fSubSel[i] == nfic)
		{
			result = i;
			break;
		}
	}

	return result;
}


VError VSubTableDB4D::AllSubRecords(BaseTaskInfo* context)
{
	VError err = VE_OK;

	sLONG nb = fSubRecs.GetCount();
	if (fSubSel.SetAllocatedSize(nb+2))
	{
		fSubSel.SetCount( nb);
		sLONG i;
		for (i=1; i<=nb; i++)
		{
			fSubSel[i] = i;
		}
	}
	else
		err = ThrowBaseError(memfull, DBaction_LoadingSubRecord);

	if (err != VE_OK)
		err = ThrowBaseError(VE_DB4D_CANNOTSELECTRECORD, DBaction_SelectinSubRecords);

	return err;
}


FicheInMem* VSubTableDB4D::GetNthSubRecord(sLONG n, VError& err, BaseTaskInfo* context, Boolean inSubSelection)
{
	FicheInMem* result = nil;
	err = VE_OK;

	if (inSubSelection)
	{
		if (n>0 && n<=fSubSel.GetCount())
		{
			n =fSubSel[n];
		}
		else
			err = ThrowBaseError(VE_DB4D_SUBRECORD_ID_OUT_OF_RANGE, DBaction_LoadingSubRecord);

	}
	
	if (n>0 && n<=fSubRecs.GetCount())
	{
		result = fSubRecs[n];
	}
	else if (!inSubSelection)
	{
		err = ThrowBaseError(VE_DB4D_SUBRECORD_ID_OUT_OF_RANGE, DBaction_LoadingSubRecord);
	}

	return result;
}


sLONG VSubTableDB4D::CountRecord(BaseTaskInfo* context, Boolean inSubSelection)
{
	sLONG result;

	if (inSubSelection)
		result = fSubSel.GetCount();
	else
		result = fSubRecs.GetCount();

	return result;
}


void VSubTableDB4D::UnMarkAllSubRecords()
{
	for (ListOfSubRecord::Iterator cur = fSubRecs.First(), end = fSubRecs.End(); cur != end; cur++)
	{
		FicheInMem* subrec = *cur;
		if (subrec != nil)
			subrec->UnMarkAsUsedAsSubRecordOnServer();
	}
}



FicheInMem* VSubTableDB4D::AddSubRecord(VError& err, BaseTaskInfo* context, Boolean inSubSelection)
{
	FicheInMem* result;
	if (fIsRemote)
		result = new FicheInMem(context, fSubTable->GetOwner(), fSubTable, err);
	else
		result = fSubTable->GetDF()->NewRecord(err, context);
	if (result != nil)
	{
		if (fIsRemote)
			result->SetAsSubRecordOnClient();
		result->SetRecParent(fParent, fSource->GetPosInRec());
		if (fSubRecs.Add(result))
		{
			if (inSubSelection)
			{
				if (fSubSel.Add(fSubRecs.GetCount()))
				{
					// tout ok
				}
				else
					err = ThrowBaseError(memfull, DBaction_CreatingSubRecord);
			}

			if (err == VE_OK )
			{
				if (fIsRemote)
				{
					fParent->Touch(fSource);
				}
				else
				{
					ValPtr cv = fParent->GetFieldValue(fSource, err);
					if (testAssert(cv != nil))
					{
						sLONG8 IDSUBREC;
						IDSUBREC = cv->GetLong8();
						if (IDSUBREC == 0)
						{
							if (fParent->IsNew())
							{
								IDSUBREC = fParent->GetAutoSeqValue();
							}
							else
								IDSUBREC = fParent->GetNum()+1;
							cv->FromLong8(IDSUBREC);
							fParent->Touch(fSource);
						}

						if (err == VE_OK)
						{
							ValPtr cv2 = result->GetFieldValue(fDest, err);
							if (testAssert(cv2 != nil))
							{
								cv2->FromLong8(IDSUBREC);
								result->Touch(fDest);
								//fParent->Touch(0);
							}
						}
					}
				}
			}
		}
		else
			err = ThrowBaseError(memfull, DBaction_CreatingSubRecord);

		if (err != VE_OK)
		{
			result->Release();
			result = nil;
		}
	}

	if (err != VE_OK)
	{
		err = ThrowBaseError(VE_DB4D_CANNOT_CREATE_RECORD, DBaction_CreatingSubRecord);
	}

	return result;
}


FicheInMem* VSubTableDB4D::FindSubRecNum(sLONG recnum)
{
	FicheInMem* result = nil;
	if (recnum >= 0)
	{
		for (ListOfSubRecord::Iterator cur = fSubRecs.First(), end = fSubRecs.End(); cur != end; cur++)
		{
			FicheInMem* subrec = *cur;
			if (subrec->GetNum() == recnum)
			{
				result = subrec;
				break;
			}
		}
	}
	return result;
}


VError VSubTableDB4D::AddOrUpdateSubRecord(FicheInMem* subrec)  // used for client server
{
	VError err = VE_OK;
	Boolean found = false;
	if (!fIsRemote)
	{
		for (ListOfSubRecord::Iterator cur = fSubRecs.First(), end = fSubRecs.End(); cur != end; cur++)
		{
			if (subrec == *cur /*|| subrec->GetID() == (*cur)->GetID()*/ )
			{
				found = true;
				break;
			}
		}
	}
	if (!found)
	{
		if (!fSubRecs.Add(subrec))
			err = ThrowBaseError(memfull, noaction);
		else
		{
			subrec->Retain();
			subrec->SetRecParent(fParent, fSource->GetPosInRec());
		}
	}
	if (err == VE_OK && !fIsRemote)
	{
		ValPtr cv = fParent->GetFieldValue(fSource, err);
		if (testAssert(cv != nil))
		{
			sLONG8 IDSUBREC;
			IDSUBREC = cv->GetLong8();
			if (IDSUBREC == 0)
			{
				if (fParent->IsNew())
				{
					IDSUBREC = fParent->GetAutoSeqValue();
				}
				else
					IDSUBREC = fParent->GetNum()+1;
				cv->FromLong8(IDSUBREC);
				fParent->Touch(fSource);
			}

			if (err == VE_OK)
			{
				ValPtr cv2 = subrec->GetFieldValue(fDest, err);
				if (testAssert(cv2 != nil))
				{
					cv2->FromLong8(IDSUBREC);
					subrec->Touch(fDest);
					//fParent->Touch(0);
				}
			}
		}
	}
	return err;
}


VError VSubTableDB4D::DeleteSubRecordsNotMarked()
{
	VError err = VE_OK;
	for (sLONG i = fSubRecs.GetCount(); i > 0; i--)
	{
		FicheInMem* subrec = fSubRecs[i];
		if (subrec != nil && !subrec->IsUsedAsSubRecordOnServer())
		{
			fDeletedSubRecs.Add(subrec);
			fSubRecs.DeleteNth(i);
		}
	}
	return err;
}


VError VSubTableDB4D::DeleteSubRecord(FicheInMem* subrec) // used for client server
{
	VError err = VE_OK;
	sLONG nfic = 0;
	for (ListOfSubRecord::Iterator cur = fSubRecs.First(), end = fSubRecs.End(); cur != end; cur++)
	{
		nfic++;
		if (subrec == *cur)
		{
			fParent->Touch(fSource);
			if (fDeletedSubRecs.Add(subrec))
			{
				fSubRecs.DeleteNth(nfic);
			}
			else
				err = ThrowBaseError(memfull, noaction);

			break;
		}
	}
	return err;
}



VError VSubTableDB4D::DeleteNthRecord(sLONG n, BaseTaskInfo* context, Boolean inSubSelection)
{
	VError err = VE_OK;
	sLONG nfic;

	if (inSubSelection)
	{
		if (n>0 && n<=fSubSel.GetCount())
		{
			nfic =fSubSel[n];
		}
		else
		{
			err = ThrowBaseError(VE_DB4D_SUBRECORD_ID_OUT_OF_RANGE, DBaction_DeletingSubRecord);
			nfic = 0; // L.E. 07/03/07 ACI0049533 init
		}
	}
	else
		nfic = n;

	if (nfic>0 && nfic<=fSubRecs.GetCount())
	{
		FicheInMem* fic = fSubRecs[nfic];
		if (fic != nil)
		{
			fParent->Touch(fSource);
			if (fDeletedSubRecs.Add(fic))
			{
				fSubRecs.DeleteNth(nfic);
			
				if (inSubSelection)
				{
					// update selection
					for( sLONG i = fSubSel.GetCount() ; i > n ; --i)
						fSubSel[i] -= 1;
				
					fSubSel.DeleteNth(n);
				}
				else
				{
					sLONG ninsel = FindSubRecNumInSubSel(nfic);
					if (ninsel>0)
					{
						// update selection
						for( sLONG i = fSubSel.GetCount() ; i > ninsel ; --i)
							fSubSel[i] -= 1;
			
						fSubSel.DeleteNth(ninsel);
					}
				}
			}
			else
				err = ThrowBaseError(memfull, DBaction_DeletingSubRecord);
		}
	}
	else
		err = ThrowBaseError(VE_DB4D_SUBRECORD_ID_OUT_OF_RANGE, DBaction_DeletingSubRecord);

	if (err != VE_OK)
	{
		err = ThrowBaseError(VE_DB4D_CANNOTDELETERECORD, DBaction_DeletingSubRecord);
	}

	return err;
}


VError VSubTableDB4D::QuerySubRecord(const SearchTab* query, BaseTaskInfo* context)
{
	return VE_DB4D_NOTIMPLEMENTED;
}


Boolean VSubTableDB4D::CompareSubRecords(sLONG n1, sLONG n2)
{
	VError err;
	FicheInMem* f1 = fSubRecs[n1];
	FicheInMem* f2 = fSubRecs[n2];

	sLONG i,nb = fCurSort->GetNbLine();
	for (i=1; i<=nb; i++)
	{
		const SortLine* li = fCurSort->GetTriLineRef(i);

		if (!testAssert( li->isfield))	// L.E. tri par formule sur les sous-fiches non implemente
			break;

		ValPtr cv1 = f1->GetNthField(li->numfield, err, false, true);
		ValPtr cv2 = f2->GetNthField(li->numfield, err, false, true);
		if (cv1 != nil && cv2 != nil)
		{
			CompareResult res = cv1->CompareToSameKind(cv2);
			if (res != CR_EQUAL)
			{
				if (res == CR_SMALLER)
				{
					if (li->ascendant)
					{
						return true;
					}
					else
					{
						return false;
					}
				}
				else
				{
					if (li->ascendant)
					{
						return false;
					}
					else
					{
						return true;
					}
				}
			}
		}
		else
			break;

	}

		return false;
}


class CompareSubRecordsEncapsulator
{
	public:
		inline CompareSubRecordsEncapsulator(VSubTableDB4D* data) { fdata = data; };
		inline Boolean operator()(sLONG n1, sLONG n2) { return fdata->CompareSubRecords(n1, n2); };
	protected:
		VSubTableDB4D *fdata;
};

VError VSubTableDB4D::SortSubRecord(const SortTab* criterias, BaseTaskInfo* context)
{
	fCurSort = criterias;
	CompareSubRecordsEncapsulator comp(this);
	std::sort(fSubSel.First(), fSubSel.First()+fSubSel.GetCount(), comp);
	return VE_OK;
}



CDB4DRecord* VSubTableDB4D::RetainNthSubRecord(sLONG inIndex, VErrorDB4D& err, CDB4DBaseContextPtr inContext, Boolean inSubSelection)
{
	BaseTaskInfoPtr context = nil;
	if (inContext != nil)
	{
		context = ConvertContext(inContext);
	}
	FicheInMem* fic = GetNthSubRecord(inIndex, err, context, inSubSelection);
	if (fic == nil)
		return nil;
	else
	{
		fic->Retain();
		return new VDB4DRecord(VDBMgr::GetManager(), fic, inContext);
	}
}


sLONG VSubTableDB4D::CountSubRecords(CDB4DBaseContextPtr inContext, Boolean inSubSelection)
{
	BaseTaskInfoPtr context = nil;
	if (inContext != nil)
	{
		context = ConvertContext(inContext);
	}
	return CountRecord(context, inSubSelection);
}

CDB4DRecord* VSubTableDB4D::AddSubRecord(VErrorDB4D& err, CDB4DBaseContextPtr inContext, Boolean inSubSelection)
{
	BaseTaskInfoPtr context = nil;
	if (inContext != nil)
	{
		context = ConvertContext(inContext);
	}
	FicheInMem* fic = AddSubRecord(err, context, inSubSelection);
	if (fic == nil)
		return nil;
	else
	{
		fic->Retain();
		return new VDB4DRecord(VDBMgr::GetManager(), fic, inContext);
	}
}


VErrorDB4D VSubTableDB4D::DeleteNthRecord(sLONG inIndex, CDB4DBaseContextPtr inContext, Boolean inSubSelection)
{
	BaseTaskInfoPtr context = nil;
	if (inContext != nil)
	{
		context = ConvertContext(inContext);
	}
	return DeleteNthRecord(inIndex, context, inSubSelection);
}


VErrorDB4D VSubTableDB4D::QuerySubRecords(const CDB4DQuery* inQuery, CDB4DBaseContextPtr inContext)
{
	BaseTaskInfoPtr context = nil;
	if (inContext != nil)
	{
		context = ConvertContext(inContext);
	}
	if (testAssert(inQuery != nil))
	{
		const SearchTab* query = (dynamic_cast<const VDB4DQuery*>(inQuery))->GetSearchTab();
		return QuerySubRecord(query, context);
	}
	else 
		return VE_DB4D_NOTIMPLEMENTED;
}


VErrorDB4D VSubTableDB4D::SortSubRecords(const CDB4DSortingCriterias* inCriterias, CDB4DBaseContextPtr inContext)
{
	BaseTaskInfoPtr context = nil;
	if (inContext != nil)
	{
		context = ConvertContext(inContext);
	}
	if (testAssert(inCriterias != nil))
	{
		const SortTab* criterias = (dynamic_cast<const VDB4DSortingCriterias*>(inCriterias))->GetSortTab();
		return SortSubRecord(criterias, context);
	}
	else 
		return VE_DB4D_NOTIMPLEMENTED;
}


VErrorDB4D VSubTableDB4D::AllSubRecords(CDB4DBaseContextPtr inContext)
{
	BaseTaskInfoPtr context = nil;
	if (inContext != nil)
	{
		context = ConvertContext(inContext);
	}
	return AllSubRecords(context);
}


VErrorDB4D VSubTableDB4D::SetSubSel(CDB4DBaseContextPtr inContext, const SubRecordIDVector& inRecords)
{
	VError err = VE_OK;

	/*
	BaseTaskInfoPtr context = nil;
	if (inContext != nil)
	{
		context = (VImpCreator<VDB4DBaseContext>::GetImpObject(inContext))->GetBaseTaskInfo();
	}
	*/

	sLONG nb = (sLONG)inRecords.size();
	if (fSubSel.SetAllocatedSize(nb+2))
	{
		fSubSel.SetCount( nb);
		sLONG i;
		SubRecordIDVector::const_iterator cur;
		for (cur = inRecords.begin(), i=1; i<=nb; i++, cur++)
		{
			fSubSel[i] = *cur;
		}
	}
	else
		err = ThrowBaseError(memfull, DBaction_SelectinSubRecords);

	return err;
}


sLONG VSubTableDB4D::GetSubRecordIDFromSubSel(CDB4DBaseContextPtr inContext, sLONG PosInSel)
{
	sLONG n;
	if (PosInSel>0 && PosInSel<=fSubSel.GetCount())
	{
		n =fSubSel[PosInSel];
	}
	else
		n = -1;

	return n;
}

									
/* ------------------------------------------------------------------------------------------------ */


const VSubTableKeyDB4D::InfoType VSubTableKeyDB4D::sInfo;

VSubTableKeyDB4D::VSubTableKeyDB4D(sLONG8 inValue)
{
	fValue = inValue;
}


VSubTableKeyDB4D::VSubTableKeyDB4D(const VSubTableKeyDB4D& inOriginal)
{
	fValue = inOriginal.fValue;
}


VSubTableKeyDB4D::VSubTableKeyDB4D(sLONG8* inDataPtr, Boolean inInit)
{
	if (inInit)
		Clear();
	else
		fValue = *inDataPtr;
}


const VValueInfo *VSubTableKeyDB4D::GetValueInfo() const
{
	return &sInfo;
}



/* ------------------------------------------------------------------------------------------------ */




ValPtr CreVString(DataTable *df, sLONG maxlen,void* from, uBOOL init, uBOOL formodif, BaseTaskInfo* context, creVValue_ParamType parameters)
{	
	StAllocateInCache alloc;
	
	VString *cv = new VString;
	if (from != nil && cv != nil) cv->LoadFromPtr(from);
	return cv;
}



ValPtr CreVStringUTF8(DataTable *df, sLONG maxlen,void* from, uBOOL init, uBOOL formodif, BaseTaskInfo* context, creVValue_ParamType parameters)
{
	StAllocateInCache alloc;

	VStringUTF8 *cv = new VStringUTF8;

	if (from != nil && cv != nil) cv->LoadFromPtr(from);
	return cv;
}


ValPtr CreVBlobText(DataTable *df, sLONG maxlen,void* from, uBOOL init, uBOOL formodif, BaseTaskInfo* context, creVValue_ParamType parameters)
{
	VBlobText *cv = nil; 
	BlobText *bt = nil;
	VError err = VE_OK;

	StAllocateInCache alloc;

	if (from == nil) 
	{
		bt = new BlobText(df);
		if ((parameters & creVValue_outsidepath) != 0)
		{
			bt->SetOutsidePath(L"*", true);
			bt->SetOutsideSuffixe(L".txt");
		}
#if debugoccupe_with_signature
		bt->MarkOccupe('cvbt');
#endif
	} 
	else if (init) 
	{
		bt = new BlobText(df);
#if debugoccupe_with_signature
		bt->MarkOccupe('cvbt');
#endif
		*((sLONG*)from) = -1;
	}
	else if (*(sLONG*)from == -2 || *(sLONG*)from == -3)
	{
		bt = (BlobText*)(df->LoadBlobFromOutsideCache( from, &CreBlobText, err, context));
		/*
		bt = new BlobText(df);
		bt->LoadPathFrom(from);
		bt->LoadDataFromPath();
		*/
	}
	else if (*(sLONG*)from != -1) 
	{
		if (*(sLONG*)from <= -10)
		{
			bt = new BlobText(df);
#if debugoccupe_with_signature
			bt->MarkOccupe('cvbt');
#endif
			sLONG lentext = (- *(sLONG*)from) - 10;
			bt->GetFrom(((sLONG*)from)+1, lentext * 2, 0);
		}
		else
			bt = (BlobText*)(df->LoadBlob( *(sLONG*)from, &CreBlobText, formodif, err, context));
	} 
	else 
	{
		bt = new BlobText(df);
		if ((parameters & creVValue_outsidepath) != 0)
		{
			bt->SetOutsidePath(L"*", true);
			bt->SetOutsideSuffixe(L".txt");
		}
#if debugoccupe_with_signature
		bt->MarkOccupe('cvbt');
#endif
	}

	if (bt != nil) {
		cv = new VBlobText( (sLONG*)from, bt);
		cv->SetTextSwitchSize(maxlen);
		if (bt->IsNull())
			cv->SetNull(true);
		bt->Release();
	}
	return cv;
}


ValPtr CreVBlobTextUTF8(DataTable *df, sLONG maxlen,void* from, uBOOL init, uBOOL formodif, BaseTaskInfo* context, creVValue_ParamType parameters)
{
	VBlobTextUTF8 *cv = nil; 
	BlobTextUTF8 *bt = nil;
	VError err = VE_OK;

	StAllocateInCache alloc;

	if (from == nil) 
	{
		bt = new BlobTextUTF8(df);
		if ((parameters & creVValue_outsidepath) != 0)
		{
			bt->SetOutsidePath(L"*", true);
			bt->SetOutsideSuffixe(L".txt");
		}
#if debugoccupe_with_signature
		bt->MarkOccupe('cvbt');
#endif
	} 
	else if (init) 
	{
		bt = new BlobTextUTF8(df);
#if debugoccupe_with_signature
		bt->MarkOccupe('cvbt');
#endif
		*((sLONG*)from) = -1;
	} 
	else if (*(sLONG*)from == -2 || *(sLONG*)from == -3)
	{
		bt = (BlobTextUTF8*)(df->LoadBlobFromOutsideCache( from, &CreBlobTextUTF8, err, context));
		/*
		bt = new BlobTextUTF8(df);
		bt->LoadPathFrom(from);
		bt->LoadDataFromPath();
		*/
	}
	else if (*(sLONG*)from != -1) 
	{
		if (*(sLONG*)from <= -10)
		{
			bt = new BlobTextUTF8(df);
#if debugoccupe_with_signature
			bt->MarkOccupe('cvbt');
#endif
			sLONG lentext = (- *(sLONG*)from) - 10;
			bt->GetFrom(((sLONG*)from)+1, lentext * 2, 0);
		}
		else
			bt = (BlobTextUTF8*)(df->LoadBlob( *(sLONG*)from, &CreBlobTextUTF8, formodif, err, context));
	} 
	else 
	{
		bt = new BlobTextUTF8(df);
		if ((parameters & creVValue_outsidepath) != 0)
		{
			bt->SetOutsidePath(L"*", true);
			bt->SetOutsideSuffixe(L".txt");
		}
#if debugoccupe_with_signature
		bt->MarkOccupe('cvbt');
#endif
	}

	if (bt != nil) {
		cv = new VBlobTextUTF8( (sLONG*)from, bt);
		cv->SetTextSwitchSize(maxlen);
		if (bt->IsNull())
			cv->SetNull(true);
		bt->Release();
	}
	return cv;
}



ValPtr CrePictAsRaw(DataTable *df, sLONG maxlen,void* from, uBOOL init, uBOOL formodif, BaseTaskInfo* context, creVValue_ParamType parameters)
{
	VBlob4DWithPtrAsRawPict *cv = nil; 
	BlobWithPtr *bt = nil;
	VError err = VE_OK;

	if (from == nil) {
		bt = new BlobWithPtr(df);
		if ((parameters & creVValue_outsidepath) != 0)
		{
			bt->SetOutsidePath(L"*", true);
			bt->SetOutsideSuffixe(L".4PCT");
		}
#if debugoccupe_with_signature
		bt->MarkOccupe('cvbw');
#endif
	} else if (init) {
		bt = new BlobWithPtr(df);
#if debugoccupe_with_signature
		bt->MarkOccupe('cvbw');
#endif
		//*((sLONG*)from) = -1;
	} 
	else if (*(sLONG*)from == -2 || *(sLONG*)from == -3)
	{
		bt = (BlobWithPtr*)(df->LoadBlobFromOutsideCache( from, &CreBlobWithPtr, err, context));
		/*
		bt = new BlobWithPtr(df);
		bt->LoadPathFrom(from);
		bt->LoadDataFromPath();
		*/
	}
	else if (*(sLONG*)from != -1) {
		if (*(sLONG*)from <= -10)
		{
			bt = new BlobWithPtr(df);
			sLONG lenblob = (- *(sLONG*)from) - 10;
			bt->GetFrom(((sLONG*)from)+1, lenblob, 0);
		}
		else
		{
			bt = (BlobWithPtr*)(df->LoadBlob( *(sLONG*)from, &CreBlobWithPtr, formodif, err, context));
			if (bt != nil && GetCurrentTransaction(context) != nil)
			{
				Blob4D* newblob = bt->Clone(true);

				if (newblob == nil)
				{
					err = bt->GetDF()->ThrowError(memfull, DBaction_CopyingBlob);
					bt = nil;
				}
				else
				{
					newblob->SetNewInTrans(bt->IsNewInTrans());
				}
				bt->Release();
				bt = (BlobWithPtr*)newblob;
			}
		}
	} else {
		bt = new BlobWithPtr(df);
		if ((parameters & creVValue_outsidepath) != 0)
		{
			bt->SetOutsidePath(L"*", true);
			bt->SetOutsideSuffixe(L".4PCT");
		}
#if debugoccupe_with_signature
		bt->MarkOccupe('cvbw');
#endif
	}

	if (bt != nil) {
		cv = new VBlob4DWithPtrAsRawPict( (sLONG*)from, bt);
		cv->SetBlobSwitchSize(maxlen);
		if (bt->IsNull())
			cv->SetNull(true);
		bt->Release();
	}
	return cv;
}



ValPtr CreVBlob4DWithPtr(DataTable *df, sLONG maxlen,void* from, uBOOL init, uBOOL formodif, BaseTaskInfo* context, creVValue_ParamType parameters)
{
	VBlob4DWithPtr *cv = nil; 
	BlobWithPtr *bt = nil;
	VError err = VE_OK;

	if (from == nil) {
		bt = new BlobWithPtr(df);
		if ((parameters & creVValue_outsidepath) != 0)
		{
			bt->SetOutsidePath(L"*", true);
			bt->SetOutsideSuffixe(L".blob");
		}
#if debugoccupe_with_signature
		bt->MarkOccupe('cvbw');
#endif
	} else if (init) {
		bt = new BlobWithPtr(df);
#if debugoccupe_with_signature
		bt->MarkOccupe('cvbw');
#endif
		//*((sLONG*)from) = -1;
	} 
	else if (*(sLONG*)from == -2 || *(sLONG*)from == -3)
	{
		bt = (BlobWithPtr*)(df->LoadBlobFromOutsideCache( from, &CreBlobWithPtr, err, context));
		/*
		bt = new BlobWithPtr(df);
		bt->LoadPathFrom(from);
		bt->LoadDataFromPath();
		*/
	}
	else if (*(sLONG*)from != -1) {
		if (*(sLONG*)from <= -10)
		{
			bt = new BlobWithPtr(df);
			sLONG lenblob = (- *(sLONG*)from) - 10;
			bt->GetFrom(((sLONG*)from)+1, lenblob, 0);
		}
		else
		{
			bt = (BlobWithPtr*)(df->LoadBlob( *(sLONG*)from, &CreBlobWithPtr, formodif, err, context));
			if (bt != nil && GetCurrentTransaction(context) != nil)
			{
				Blob4D* newblob = bt->Clone(true);

				if (newblob == nil)
				{
					err = bt->GetDF()->ThrowError(memfull, DBaction_CopyingBlob);
					bt = nil;
				}
				else
				{
					newblob->SetNewInTrans(bt->IsNewInTrans());
				}
				bt->Release();
				bt = (BlobWithPtr*)newblob;
			}
		}
	} else {
		bt = new BlobWithPtr(df);
		if ((parameters & creVValue_outsidepath) != 0)
		{
			bt->SetOutsidePath(L"*", true);
			bt->SetOutsideSuffixe(L".blob");
		}
#if debugoccupe_with_signature
		bt->MarkOccupe('cvbw');
#endif
	}

	if (bt != nil) {
		cv = new VBlob4DWithPtr( (sLONG*)from, bt);
		cv->SetBlobSwitchSize(maxlen);
		if (bt->IsNull())
			cv->SetNull(true);
		bt->Release();
	}
	return cv;
}


static const VValueInfo* sVPictureRef = nil;


ValPtr CreVBlobPicture(DataTable *df, sLONG maxlen,void* from, uBOOL init, uBOOL formodif, BaseTaskInfo* context, creVValue_ParamType parameters)
{
	VBlob4DWithPtr *blob = nil; 
	ValPtr cv = nil; 
	BlobWithPtr *bt = nil;
	VError err = VE_OK;

	if (from == nil) {
			bt = new BlobWithPtr(df);
			if ((parameters & creVValue_outsidepath) != 0)
			{
				bt->SetOutsidePath(L"*", true);
				bt->SetOutsideSuffixe(L".4PCT");
			}
#if debugoccupe_with_signature
			bt->MarkOccupe('cvbp');
#endif
	} else if (init) {
			bt = new BlobWithPtr(df);
#if debugoccupe_with_signature
			bt->MarkOccupe('cvbp');
#endif
			//*((sLONG*)from) = -1;
	} 
	else if (*(sLONG*)from == -2 || *(sLONG*)from == -3)
	{
		bt = (BlobWithPtr*)(df->LoadBlobFromOutsideCache( from, &CreBlobWithPtr, err, context));
		/*
		bt = new BlobWithPtr(df);
		bt->LoadPathFrom(from);
		bt->LoadDataFromPath();
		*/
	}
	else if (*(sLONG*)from != -1) {
		if (*(sLONG*)from <= -10)
		{
			bt = new BlobWithPtr(df);
			sLONG lenblob = (- *(sLONG*)from) - 10;
			bt->GetFrom(((sLONG*)from)+1, lenblob, 0);
		}
		else
		{
			bt = (BlobWithPtr*)(df->LoadBlob( *(sLONG*)from, &CreBlobWithPtr, formodif, err, context));
			if (bt != nil && GetCurrentTransaction(context) != nil)
			{
				Blob4D* newblob = bt->Clone(true);

				if (newblob == nil)
				{
					err = bt->GetDF()->ThrowError(memfull, DBaction_CopyingBlob);
					bt = nil;
				}
				else
				{
					newblob->SetNewInTrans(bt->IsNewInTrans());
				}
				bt->Release();
				bt = (BlobWithPtr*)newblob;
			}
		}
	} else {
			bt = new BlobWithPtr(df);
			if ((parameters & creVValue_outsidepath) != 0)
			{
				bt->SetOutsidePath(L"*", true);
				bt->SetOutsideSuffixe(L".4PCT");
			}
#if debugoccupe_with_signature
			bt->MarkOccupe('cvbp');
#endif
	}

	if (sVPictureRef == nil)
	{
		sVPictureRef = VValue::ValueInfoFromValueKind(VK_IMAGE);
	}

	if (bt != nil && sVPictureRef != nil) {
			blob = new VBlob4DWithPtr((sLONG*)from, bt);
			blob->SetBlobSwitchSize(maxlen);
			cv = (ValPtr)sVPictureRef->Generate(blob);
			if (bt->IsNull())
				cv->SetNull(true);
			bt->Release();
	}
	return cv;
}


ValPtr CreVEmpty(DataTable *df, sLONG maxlen,void* from, uBOOL init, uBOOL formodif, BaseTaskInfo* context, creVValue_ParamType parameters)
{
	StAllocateInCache alloc;
	return new VEmpty;
}

ValPtr CreVBoolean(DataTable *df, sLONG maxlen,void* from, uBOOL init, uBOOL formodif, BaseTaskInfo* context, creVValue_ParamType parameters)
{
	StAllocateInCache alloc;
	return (from == nil) ? new VBoolean : new VBoolean( (uBYTE*)from, init);
}

ValPtr CreVByte(DataTable *df, sLONG maxlen,void* from, uBOOL init, uBOOL formodif, BaseTaskInfo* context, creVValue_ParamType parameters)
{
	StAllocateInCache alloc;
	return (from == nil) ? new VByte : new VByte( (sBYTE*)from, init);
}


ValPtr CreVLong(DataTable *df, sLONG maxlen,void* from, uBOOL init, uBOOL formodif, BaseTaskInfo* context, creVValue_ParamType parameters)
{
	StAllocateInCache alloc;
	return (from == nil) ? new VLong : new VLong( (sLONG*)from, init);
}


ValPtr CreVShort(DataTable *df, sLONG maxlen,void* from, uBOOL init, uBOOL formodif, BaseTaskInfo* context, creVValue_ParamType parameters)
{
	StAllocateInCache alloc;
	return (from == nil) ? new VWord : new VWord( (sWORD*)from, init);
}


ValPtr CreVUUID(DataTable *df, sLONG maxlen,void* from, uBOOL init, uBOOL formodif, BaseTaskInfo* context, creVValue_ParamType parameters)
{
	StAllocateInCache alloc;
	ValPtr result = (from == nil || init) ? new VUUID( (Boolean)false) : new VUUID( *(VUUIDBuffer*)from);
	result->SetNull(false);
	return result;
}


ValPtr CreVDuration(DataTable *df, sLONG maxlen,void* from, uBOOL init, uBOOL formodif, BaseTaskInfo* context, creVValue_ParamType parameters)
{
	StAllocateInCache alloc;
	return (from == nil) ? new VDuration : new VDuration( (sLONG8*)from, init);
}


static ValPtr CreVFloat(DataTable *df, sLONG maxlen,void* from, uBOOL init, uBOOL formodif, BaseTaskInfo* context, creVValue_ParamType parameters)
{
	StAllocateInCache alloc;
	return (from == nil) ? new VFloat : new VFloat( (uBYTE*)from, init);
}


ValPtr CreVReal(DataTable *df, sLONG maxlen,void* from, uBOOL init, uBOOL formodif, BaseTaskInfo* context, creVValue_ParamType parameters)
{
	StAllocateInCache alloc;
	return (from == nil) ? new VReal : new VReal( (Real*)from, init);
}


ValPtr CreVLong8(DataTable *df, sLONG maxlen,void* from, uBOOL init, uBOOL formodif, BaseTaskInfo* context, creVValue_ParamType parameters)
{
	StAllocateInCache alloc;
	return (from == nil) ? new VLong8 : new VLong8( (sLONG8*)from, init);
}

/*
ValPtr CreVMoney(DataTable *df, sLONG maxlen,void* from, uBOOL init, uBOOL formodif)
{
	return (from == nil) ? new VMoney : new VMoney( (uBYTE*)from, init);
}
*/


ValPtr CreVTime(DataTable *df, sLONG maxlen,void* from, uBOOL init, uBOOL formodif, BaseTaskInfo* context, creVValue_ParamType parameters)
{
	StAllocateInCache alloc;
	VTime* res = (from == nil) ? new VTime : new VTime( (uLONG8*)from, init);
	//res->NullIfZero();
	return res;
}


ValPtr CreVSubTable(DataTable *df, sLONG maxlen,void* from, uBOOL init, uBOOL formodif, BaseTaskInfo* context, creVValue_ParamType parameters)
{
	StAllocateInCache alloc;
	return (from == nil) ? new VSubTableDB4D : new VSubTableDB4D( (sLONG8*)from, init);
}


ValPtr CreVSubTableKey(DataTable *df, sLONG maxlen,void* from, uBOOL init, uBOOL formodif, BaseTaskInfo* context, creVValue_ParamType parameters)
{
	StAllocateInCache alloc;
	return (from == nil) ? new VSubTableKeyDB4D : new VSubTableKeyDB4D( (sLONG8*)from, init);
}


CodeReg *CV_CodeReg;


																/* ******************************************* */


ValPtr CreVValueFromNil(sLONG typ, sLONG maxlen, Boolean init)
{
	ValPtr cv;
	VError err = VE_OK;
	CreVValue_Code Code;
	
	Code=FindCV(typ);
	cv=(*Code)(nil,maxlen,nil,init,false,NULL, creVValue_default);
	
	if ((err==VE_OK) && (cv==nil)) 
		err = ThrowBaseError(memfull, DBaction_BuildingValue);
	
	return(cv);
}


sLONG InitChampVar()
{
	assert( CV_CodeReg == nil);
	CV_CodeReg = new CodeReg;
	RegisterCV(DB4D_StrFix,&CreVString);
	RegisterCV(DB4D_Text,&CreVBlobText);
	RegisterCV(VK_TEXT_UTF8,&CreVBlobTextUTF8);
	RegisterCV(DB4D_Float,&CreVFloat);
	RegisterCV(DB4D_Real,&CreVReal);
	RegisterCV(DB4D_Time,&CreVTime);
	RegisterCV(DB4D_Duration,&CreVDuration);
	RegisterCV(DB4D_Integer16,&CreVShort);
	RegisterCV(DB4D_Integer32,&CreVLong);
	RegisterCV(DB4D_Boolean,&CreVBoolean);
	RegisterCV(DB4D_Byte,&CreVByte);
	RegisterCV(DB4D_Duration,&CreVDuration);
	RegisterCV(DB4D_Integer64,&CreVLong8);
	RegisterCV(DB4D_Blob,&CreVBlob4DWithPtr);
	RegisterCV(DB4D_Picture,&CreVBlobPicture);
	RegisterCV(DB4D_UUID,&CreVUUID);
	RegisterCV(DB4D_SubTable,&CreVSubTable);
	RegisterCV(DB4D_SubTableKey,&CreVSubTableKey);
	RegisterCV(DB4D_NoType,&CreVEmpty);
	RegisterCV(VK_BLOB,&CreVBlob4DWithPtr);
	RegisterCV(VK_STRING_UTF8,&CreVStringUTF8);

	return(0);
}

void DeInitChampVar()
{
	if (CV_CodeReg != nil) {
		delete CV_CodeReg;
		CV_CodeReg = nil;
	}
}


/* ------------------------------------------------------------------------------------------------ */

sLONG VirtualArrayIterator::FindString (VString *st) {
	sLONG		i, result;
	UniChar	foundBytes[256], curBytes[256];
	VString	foundStr(foundBytes, 256), curStr(curBytes, 256);

	result = 0;
	foundStr.Clear();

	for (i = 1; i <= CountElems (); ++i) {
		if (GetAlpha (&curStr, i)) {
			if (curStr > *st) {
				if (!foundStr.GetLength() || curStr < foundStr) {
					result = i;
					foundStr.FromString(curStr);
				}
			}
		}
	}
	return (result);
}

/* ------------------------------------------------------------------------------------------------ */
