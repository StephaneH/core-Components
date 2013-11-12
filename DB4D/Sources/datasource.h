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
#ifndef __DataSource__
#define __DataSource__

#include <map>
#include <vector>

class FicheInMem;
class BaseTaskInfo;
class VirtualArrayIterator;
class EmptyArrayIterator;
class Variables4DArray;

//extern DateBlock nulldate;

class BlobText;
class BlobWithPtr;
class DataBaseObjectHeader;

const sLONG kDBAVSIG = (sLONG)'DBVV';

/*
class DB4DArrayOfVValues : public DB4DArrayOfValues, public VArrayPtrOf<VValueSingle*>
{
public:
	virtual Boolean Find(XBOX::VValueSingle& inValue, const XBOX::VCompareOptions& inOptions) const;
	virtual Boolean FindWithDataPtr(void* inDataPtr, const XBOX::VCompareOptions& inOptions) const;
	virtual const void* GetFirst() const; 
	virtual const void* GetLast() const;
	virtual const void* GetNext() const;
	virtual sLONG Count() const;
	virtual VError PutInto(VStream& outStream) const;
	virtual VError GetFrom(VStream& inStream);
	virtual sLONG GetSignature() const;
	virtual Boolean IsAllNull() const;
	virtual Boolean AtLeastOneNull() const;

	void Sort(Boolean ascending = true);

protected:
	void subsort(sLONG l, sLONG r);
	sLONG count;
};
*/


class QuickString
{
public:

		UniChar* fString;
};


class QuickDB4DArrayOfValues : public DB4DArrayOfValues
{
	public:
		virtual void AddStringToConvert(const VString& s) = 0;
		virtual bool AddOneElemPtr(void* data) = 0;
		virtual DB4DArrayOfValues* GenerateConstArrayOfValues() = 0;
		virtual XBOX::VError GetFrom(XBOX::VStream& inStream) = 0;
};


class DB4DArrayOfConstValues : public DB4DArrayOfValues
{
	public:
		virtual const void* GetNextPtr() const = 0;
		virtual CompareResult CompareKeyWithValue(const void* key, const void* val, const VCompareOptions& inOptions) const = 0;
		
};

template <class Type> class DB4DArrayOfConstDirectValues;


template <class Type>
class DB4DArrayOfDirectValues : public QuickDB4DArrayOfValues
{
	friend class DB4DArrayOfConstValues;
	friend class DB4DArrayOfConstDirectValues<Type>;
	
	public:

		inline DB4DArrayOfDirectValues(const XBOX::VCompareOptions& inOptions, sLONG inDataKind, VJSArray* jsarr = nil)
		{
			fOneNull = false;
			fOptions = inOptions;
			fDataKind = inDataKind;
			if (jsarr != nil)
			{
				sLONG nbelem = (sLONG)jsarr->GetLength();
				for (sLONG i = 0; i  < nbelem; i++)
				{
					VJSValue jsval(jsarr->GetValueAt(i));
					if (jsval.IsNull() || jsval.IsUndefined())
					{
						// ne rien faire
					}
					else
						AddJSVal(jsval);
				}
				Sort();
			}
		}

		inline DB4DArrayOfDirectValues(const XBOX::VCompareOptions& inOptions, sLONG inDataKind, VJSONArray* jsarr)
		{
			fOneNull = false;
			fOptions = inOptions;
			fDataKind = inDataKind;
			if (jsarr != nil)
			{
				sLONG nbelem = (sLONG)jsarr->GetCount();
				for (sLONG i = 0; i  < nbelem; i++)
				{
					VJSONValue jsval = (*jsarr)[i];
					if (jsval.IsNull() || jsval.IsUndefined())
					{
						// ne rien faire
					}
					else
						AddJSONVal(jsval);
				}
				Sort();
			}
		}

		virtual ~DB4DArrayOfDirectValues() { ; };

		class MyLess
		{
			public:
				const VCompareOptions& fxOptions;
				inline MyLess(const VCompareOptions& inOptions):fxOptions(inOptions) { ; }

				inline bool operator()(const Type& val1, const Type& val2)
				{
					return val1 < val2;
				}
		};

		virtual const void* GetFirstPtr() const
		{
			fCurrent = fArray.begin();
			if (fCurrent == fArray.end())
				return nil;
			else
				return &(*fCurrent);
		}

		virtual const void* GetNextPtr() const
		{
			fCurrent++;
			if (fCurrent == fArray.end())
				return nil;
			else
				return &(*fCurrent);
		}

		virtual const void* GetLastPtr() const
		{
			if (fArray.empty())
				return nil;
			else
				return &(fArray[fArray.size()-1]);
		}

		virtual Boolean IsAllNull() const
		{
			return fArray.empty() && fOneNull;
		}

		virtual Boolean AtLeastOneNull() const
		{
			return fOneNull;
		}


		inline void Sort()
		{
			MyLess pred(fOptions);
			MyLess& predref = pred;
			sort(fArray.begin(), fArray.end(), predref);
		}

		inline const Type* First() const
		{
			fCurrent = fArray.begin();
			if (fCurrent == fArray.end())
				return nil;
			else
				return &(*fCurrent);
		}

		inline const Type* Next() const
		{
			fCurrent++;
			if (fCurrent == fArray.end())
				return nil;
			else
				return &(*fCurrent);
		}

		inline virtual sLONG Count() const
		{
			return (sLONG)fArray.size();
		}

		virtual Boolean FindWithDataPtr(void* inDataPtr, const XBOX::VCompareOptions& inOptions) const
		{
			MyLess pred(inOptions);
			MyLess& predref = pred;
			return binary_search(fArray.begin(), fArray.end(), *((Type*)inDataPtr), predref);
		}

		inline Boolean FindDirect(const Type* val, const XBOX::VCompareOptions& inOptions) const
		{
			MyLess pred(inOptions);
			MyLess& predref = pred;
			return binary_search(fArray.begin(), fArray.end(), *val, predref);
		}

		virtual	const XBOX::VValueSingle* GetFirst()
		{
			assert(false);  // must be specialized
			return nil;
		}

		virtual	const XBOX::VValueSingle* GetLast()
		{
			assert(false);  // must be specialized
			return nil;
		}

		virtual Boolean Find(XBOX::VValueSingle& inValue, const XBOX::VCompareOptions& inOptions) const
		{
			assert(false);  // must be specialized
			return false;
		}

		virtual XBOX::VError PutInto(XBOX::VStream& outStream) const
		{
			assert(false);  // must be specialized
			return VE_DB4D_NOTIMPLEMENTED;
		}

		virtual XBOX::VError GetFrom(XBOX::VStream& inStream)
		{
			assert(false);  // must be specialized
			return VE_DB4D_NOTIMPLEMENTED;
		}

		virtual sLONG GetSignature() const
		{
			return 'rawx';
		}

		virtual Boolean CanBeUsedWithIndex(XBOX::VIntlMgr* inIntlMgr) const
		{
			return true;
		}

		inline void Reserve(sLONG inHowMany)
		{
			fArray.reserve(inHowMany);
		}

		inline bool AddOneElem(const Type& data) 
		{
			try
			{
				fArray.push_back(data);
			}
			catch (...)
			{
				return false;
			}

			return true;
		}

		virtual bool AddOneElemPtr(void* data)
		{
			try
			{
				fArray.push_back(*((Type*)data));
			}
			catch (...)
			{
				return false;
			}

			return true;
		}


		inline bool AddOneElemNull()
		{
			fOneNull = true;
		}

		inline void AddJSVal(VJSValue& jsval)
		{
			#if COMPIL_VISUAL	// Visual doesn't compile templates until instantiation. GCC does.
			tagada++; // comilation error on purpose, needs to be specialized;
			#endif
		}

		inline void AddJSONVal(VJSONValue& jsval)
		{
#if COMPIL_VISUAL	// Visual doesn't compile templates until instantiation. GCC does.
			tagada++; // comilation error on purpose, needs to be specialized;
#endif
		}

		virtual void AddStringToConvert(const VString& s)
		{
#if COMPIL_VISUAL	// Visual doesn't compile templates until instantiation. GCC does.
			tagada++; // comilation error on purpose, needs to be specialized;
#endif
		}

		virtual DB4DArrayOfConstValues* GenerateConstArrayOfValues();

		bool AddOneElemPtrForXString(const xString& xs)
		{
			assert(false); // must be specialized for QuickStrings
			return false;
		}

		bool AddOneElemPtrForXString(const xStringUTF8& xs)
		{
			assert(false); // must be specialized for QuickStrings
			return false;
		}

		VJSONValue GetJSONValue(const Type& val, UniChar wildcharReplace) const
		{
			return VJSONValue(val);
		}

		virtual VJSONArray* ToJSON(CDB4DBaseContext* context, UniChar wildcharReplace) const
		{
			VJSONArray* arr = new VJSONArray();
			for (typename vector<Type>::const_iterator cur = fArray.begin(), end = fArray.end(); cur != end; ++cur)
			{
				arr->Push(GetJSONValue(*cur, wildcharReplace));
			}
			return arr;
		}



	protected:
		vector<Type> fArray;
		mutable typename vector<Type>::const_iterator fCurrent;
		VCompareOptions fOptions;
		sLONG fDataKind;
		Boolean fOneNull;
};




			/* ---------------------- */



template <>
inline void DB4DArrayOfDirectValues<xTime>::AddStringToConvert(const VString& s)
{	
	VTime tt;
	tt.FromString(s);
	xTime xt(tt);
	AddOneElem(xt);

}


template <>
inline VJSONValue DB4DArrayOfDirectValues<xTime>::GetJSONValue(const xTime& val, UniChar wildcharReplace) const
{
	VTime tt;
	tt.FromStamp(val.GetStamp());
	VString s;
	tt.GetXMLString(s, XSO_Default);
	return VJSONValue(s);
}



template <>
inline void DB4DArrayOfDirectValues<xTime>::AddJSVal(VJSValue& jsval)
{
	VString s;
	if (jsval.GetString(s))
	{
		VTime tt;
		tt.FromString(s);
		xTime xt(tt);
		AddOneElem(xt);
	}
}


template <>
inline void DB4DArrayOfDirectValues<xTime>::AddJSONVal(VJSONValue& jsval)
{
	VString s;
	jsval.GetString(s);
	{
		VTime tt;
		tt.FromString(s);
		xTime xt(tt);
		AddOneElem(xt);
	}
}



			/* ---------------------- */


template <>
inline void DB4DArrayOfDirectValues<uLONG8>::AddJSVal(VJSValue& jsval)
{
	Real r;
	if (jsval.GetReal(&r))
	{
		uLONG8 l8 = r;
		AddOneElem(l8);
	}
}


template <>
inline void DB4DArrayOfDirectValues<uLONG8>::AddJSONVal(VJSONValue& jsval)
{
	Real r = jsval.GetNumber();
	{
		uLONG8 l8 = r;
		AddOneElem(l8);
	}
}



template <>
inline void DB4DArrayOfDirectValues<uLONG8>::AddStringToConvert(const VString& s)
{	
	uLONG8 l8 = s.GetLong8();
	AddOneElem(l8);
	
}




			/* ---------------------- */

template <>
inline void DB4DArrayOfDirectValues<sLONG8>::AddJSVal(VJSValue& jsval)
{
	Real r;
	if (jsval.GetReal(&r))
	{
		sLONG8 l8 = r;
		AddOneElem(l8);
	}
}


template <>
inline void DB4DArrayOfDirectValues<sLONG8>::AddJSONVal(VJSONValue& jsval)
{
	Real r = jsval.GetNumber();
	{
		sLONG8 l8 = r;
		AddOneElem(l8);
	}
}


template <>
inline void DB4DArrayOfDirectValues<sLONG8>::AddStringToConvert(const VString& s)
{	
	sLONG8 l8 = s.GetLong8();
	AddOneElem(l8);
}



			/* ---------------------- */

template <>
inline void DB4DArrayOfDirectValues<sLONG>::AddJSVal(VJSValue& jsval)
{
	sLONG n;
	if (jsval.GetLong(&n))
	{
		AddOneElem(n);
	}
}

template <>
inline void DB4DArrayOfDirectValues<sLONG>::AddJSONVal(VJSONValue& jsval)
{
	sLONG n = (sLONG)jsval.GetNumber();
	AddOneElem(n);
}


template <>
inline void DB4DArrayOfDirectValues<sLONG>::AddStringToConvert(const VString& s)
{	
	sLONG n = s.GetLong();
	AddOneElem(n);
}



			/* ---------------------- */

template <>
inline void DB4DArrayOfDirectValues<sWORD>::AddJSVal(VJSValue& jsval)
{
	sWORD sw;
	sLONG n;
	if (jsval.GetLong(&n))
	{
		sw = (sWORD)n;
		AddOneElem(sw);
	}
}


template <>
inline void DB4DArrayOfDirectValues<sWORD>::AddJSONVal(VJSONValue& jsval)
{
	sWORD sw;
	sLONG n = (sLONG)jsval.GetNumber();
	{
		sw = (sWORD)n;
		AddOneElem(sw);
	}
}



template <>
inline void DB4DArrayOfDirectValues<sWORD>::AddStringToConvert(const VString& s)
{	
	sWORD n = s.GetWord();
	AddOneElem(n);
}


			/* ---------------------- */

template <>
inline void DB4DArrayOfDirectValues<sBYTE>::AddJSVal(VJSValue& jsval)
{
	sBYTE sb;
	sLONG n;
	if (jsval.GetLong(&n))
	{
		sb = (sBYTE)n;
		AddOneElem(sb);
	}
}


template <>
inline void DB4DArrayOfDirectValues<sBYTE>::AddJSONVal(VJSONValue& jsval)
{
	sBYTE sb;
	sLONG n = (sLONG)jsval.GetNumber();
	{
		sb = (sBYTE)n;
		AddOneElem(sb);
	}
}


template <>
inline void DB4DArrayOfDirectValues<sBYTE>::AddStringToConvert(const VString& s)
{	
	sBYTE n = s.GetByte();
	AddOneElem(n);
}



			/* ---------------------- */

template <>
inline VJSONValue DB4DArrayOfDirectValues<uBYTE>::GetJSONValue(const uBYTE& val, UniChar wildcharReplace) const
{
	if (val == 1)
		return VJSONValue(JSON_true);
	else
		return VJSONValue(JSON_false);
}


template <>
inline void DB4DArrayOfDirectValues<uBYTE>::AddJSVal(VJSValue& jsval)
{
	uBYTE ub;
	bool b;
	if (jsval.GetBool(&b))
	{
		ub = b ? 1 : 0;
		AddOneElem(ub);
	}
}


template <>
inline void DB4DArrayOfDirectValues<uBYTE>::AddJSONVal(VJSONValue& jsval)
{
	uBYTE ub;
	bool b = jsval.GetBool();
	{
		ub = b ? 1 : 0;
		AddOneElem(ub);
	}
}


template <>
inline void DB4DArrayOfDirectValues<uBYTE>::AddStringToConvert(const VString& s)
{	
	uBYTE n = s.GetBoolean();
	AddOneElem(n);
}

			/* ---------------------- */

template <>
inline void DB4DArrayOfDirectValues<Real>::AddJSVal(VJSValue& jsval)
{
	Real r;
	if (jsval.GetReal(&r))
		AddOneElem(r);
}


template <>
inline void DB4DArrayOfDirectValues<Real>::AddJSONVal(VJSONValue& jsval)
{
	Real r = jsval.GetNumber();
	AddOneElem(r);
}


template <>
inline bool DB4DArrayOfDirectValues<Real>::MyLess::operator ()(const Real& val1, const Real& val2)
{
	if (fabs(val1 - val2) < fxOptions.GetEpsilon())
		return false;
	else
		return val1 < val2;
}


template <>
inline void DB4DArrayOfDirectValues<Real>::AddStringToConvert(const VString& s)
{	
	Real r = s.GetReal();
	AddOneElem(r);
}


			/* ---------------------- */


template <>
inline VJSONValue DB4DArrayOfDirectValues<VInlineString>::GetJSONValue(const VInlineString& val, UniChar wildcharReplace) const
{
	VString s(val);
	if (wildcharReplace != 0 && fOptions.IsLike() && fOptions.GetIntlManager() != nil)
	{
		s.ExchangeAll(fOptions.GetIntlManager()->GetWildChar(), wildcharReplace);
	}
	return VJSONValue(s);
}



template <>
inline void DB4DArrayOfDirectValues<VInlineString>::AddJSVal(VJSValue& jsval)
{
	VInlineString xs;
	VString s;
	if (jsval.GetString(s))
	{
		xs.FromString(s);
		AddOneElem(xs);
	}
}


template <>
inline void DB4DArrayOfDirectValues<VInlineString>::AddJSONVal(VJSONValue& jsval)
{
	VInlineString xs;
	VString s;
	jsval.GetString(s);
	{
		xs.FromString(s);
		AddOneElem(xs);
	}
}


template <>
inline void DB4DArrayOfDirectValues<VInlineString>::AddStringToConvert(const VString& s)
{	
	VInlineString xs;
	xs.FromString(s);
	AddOneElem(xs);
}

template <>
inline bool DB4DArrayOfDirectValues<VInlineString>::MyLess::operator ()(const VInlineString& val1, const VInlineString& val2)
{
	return fxOptions.GetIntlManager()->CompareString(val1.fString, val1.fLength, val2.fString, val2.fLength, fxOptions) == CR_SMALLER;
}


			/* ---------------------- */


template <>
inline VJSONValue DB4DArrayOfDirectValues<QuickString>::GetJSONValue(const QuickString& val, UniChar wildcharReplace) const
{
	VString s;
	s.LoadFromPtr(val.fString);
	if (wildcharReplace != 0 && fOptions.IsLike() && fOptions.GetIntlManager() != nil)
	{
		s.ExchangeAll(fOptions.GetIntlManager()->GetWildChar(), wildcharReplace);
	}
	return VJSONValue(s);
}



template <>
inline void DB4DArrayOfDirectValues<QuickString>::AddJSVal(VJSValue& jsval)
{
	VString s;
	if (jsval.GetString(s))
	{
		QuickString qs;
		sLONG len = s.GetLength();
		const UniChar* source = s.GetCPointer();
		qs.fString = (UniChar*)VObject::GetMainMemMgr()->Malloc(len*2 + 4, false, 'qstr');
		if (len > 0)
			copy(source, source+len, qs.fString+2);
		*((sLONG*)qs.fString) = len;
		fArray.push_back(qs);
	}
}


template <>
inline void DB4DArrayOfDirectValues<QuickString>::AddJSONVal(VJSONValue& jsval)
{
	VString s;
	jsval.GetString(s);
	{
		QuickString qs;
		sLONG len = s.GetLength();
		const UniChar* source = s.GetCPointer();
		qs.fString = (UniChar*)VObject::GetMainMemMgr()->Malloc(len*2 + 4, false, 'qstr');
		if (len > 0)
			copy(source, source+len, qs.fString+2);
		*((sLONG*)qs.fString) = len;
		fArray.push_back(qs);
	}
}


template <>
inline void DB4DArrayOfDirectValues<QuickString>::AddStringToConvert(const VString& s)
{	
	QuickString qs;
	sLONG len = s.GetLength();
	const UniChar* source = s.GetCPointer();
	qs.fString = (UniChar*)VObject::GetMainMemMgr()->Malloc(len*2 + 4, false, 'qstr');
	if (len > 0)
		copy(source, source+len, qs.fString+2);
	*((sLONG*)qs.fString) = len;
	fArray.push_back(qs);
}


template <>
inline Boolean DB4DArrayOfDirectValues<QuickString>::CanBeUsedWithIndex(XBOX::VIntlMgr* inIntlMgr) const
{
	Boolean result = true;
	const QuickString* cur = First();

	for (sLONG i = 0,nb = Count(); (i < nb) && result; i++,cur++)
	{
		sLONG nbchar = *((sLONG*)cur->fString);
		const UniChar *pc = cur->fString+2;
		result = inIntlMgr->GetCollator()->IsPatternCompatibleWithDichotomyAndDiacritics( pc, nbchar);
	}
	return result;
}

template <>
inline bool DB4DArrayOfDirectValues<QuickString>::MyLess::operator ()(const QuickString& val1, const QuickString& val2)
{
	return fxOptions.GetIntlManager()->CompareString(val1.fString + 2, *((sLONG*)val1.fString), val2.fString + 2, *((sLONG*)val2.fString), fxOptions) == CR_SMALLER;
}


template <>
inline DB4DArrayOfDirectValues<QuickString>::~DB4DArrayOfDirectValues()
{
	for (vector<QuickString>::iterator cur = fArray.begin(), end = fArray.end(); cur != end; cur++)
	{
		VObject::GetMainMemMgr()->Free((void*)cur->fString);
	}
}

template <>
inline bool DB4DArrayOfDirectValues<QuickString>::AddOneElemPtr(void* data)
{
	QuickString qs;
	qs.fString = nil;

	try
	{
		sLONG len = *((sLONG*)data);
		UniChar* source = (UniChar*)data;

		qs.fString = (UniChar*)VObject::GetMainMemMgr()->Malloc(len*2 + 4, false, 'qstr');
		if (qs.fString == nil)
			return false;
		copy(source, source+len+2, qs.fString);
		fArray.push_back(qs);
	}
	catch (...)
	{
		if (qs.fString != nil)
			VObject::GetMainMemMgr()->Free((void*)qs.fString);
		return false;
	}

	return true;
}



template <>
inline bool DB4DArrayOfDirectValues<QuickString>::AddOneElemPtrForXString(const xString& xs)
{
	QuickString qs;
	qs.fString = nil;

	try
	{ 
		sLONG len = xs.GetLength();
		UniChar* source = (UniChar*)xs.GetPtr();

		qs.fString = (UniChar*)VObject::GetMainMemMgr()->Malloc(len*2 + 4, false, 'qstr');
		if (qs.fString == nil)
			return false;
		*((sLONG*)(qs.fString)) = len;
		copy(source, source+len, qs.fString+2);
		fArray.push_back(qs);
	}
	catch (...)
	{
		if (qs.fString != nil)
			VObject::GetMainMemMgr()->Free((void*)qs.fString);
		return false;
	}

	return true;
}


template <>
inline bool DB4DArrayOfDirectValues<QuickString>::AddOneElemPtrForXString(const xStringUTF8& xs)
{
	QuickString qs;
	qs.fString = nil;

	try
	{ 
		sLONG len = xs.GetLength();
		UniChar* source = (UniChar*)xs.GetPtr();

		qs.fString = (UniChar*)VObject::GetMainMemMgr()->Malloc(len*2 + 4, false, 'qstr');
		if (qs.fString == nil)
			return false;
		*((sLONG*)(qs.fString)) = len;
		copy(source, source+len, qs.fString+2);
		fArray.push_back(qs);
	}
	catch (...)
	{
		if (qs.fString != nil)
			VObject::GetMainMemMgr()->Free((void*)qs.fString);
		return false;
	}

	return true;
}


			/* ---------------------- */


template <>
inline VJSONValue DB4DArrayOfDirectValues<VUUIDBuffer>::GetJSONValue(const VUUIDBuffer& val, UniChar wildcharReplace) const
{
	VString s;
	VUUID xid(val);
	xid.GetString(s);
	return VJSONValue(s);
}


template <>
inline void DB4DArrayOfDirectValues<VUUIDBuffer>::AddJSVal(VJSValue& jsval)
{
	VUUID xid;
	VString s;
	if (jsval.GetString(s))
	{
		xid.FromString(s);
		AddOneElem(xid.GetBuffer());
	}
}


template <>
inline void DB4DArrayOfDirectValues<VUUIDBuffer>::AddJSONVal(VJSONValue& jsval)
{
	VUUID xid;
	VString s;
	jsval.GetString(s);
	{
		xid.FromString(s);
		AddOneElem(xid.GetBuffer());
	}
}

template <>
inline void DB4DArrayOfDirectValues<VUUIDBuffer>::AddStringToConvert(const VString& s)
{	
	VUUID xid;
	xid.FromString(s);
	AddOneElem(xid.GetBuffer());
}

template<>
inline XBOX::VError DB4DArrayOfDirectValues<VUUIDBuffer>::PutInto(XBOX::VStream& outStream) const
{
	sLONG	count = (sLONG) fArray.size();
	outStream.PutLong(count);
	outStream.PutByte( fOneNull ? 1 : 0 );
	for(sLONG i = 0; i < count; ++i)
	{
		outStream.PutData( fArray[i].fBytes, 16 );
	}

	return outStream.GetLastError(); 
}

template<>
inline XBOX::VError DB4DArrayOfDirectValues<VUUIDBuffer>::GetFrom(XBOX::VStream& inStream)
{
	VError		error = VE_OK;
	sLONG		count;

	count = inStream.GetLong();
	fOneNull = inStream.GetByte() != 0 ? true : false;

	fArray.reserve(count);

	VUUIDBuffer		uidBuf;
	for(sLONG i = 0; i < count; ++i)
	{
		VSize len = 16;
		inStream.GetData( uidBuf.fBytes, &len );
		assert(len == 16);
		fArray.push_back(uidBuf);
	}
	//Sort();

	return inStream.GetLastError();
}


			/* ---------------------- */


QuickDB4DArrayOfValues* CreateDB4DArrayOfDirectValues(sLONG inDataKind, const VCompareOptions& inOptions, VJSArray* jsarr = nil);

QuickDB4DArrayOfValues* CreateDB4DArrayOfDirectValues(sLONG inDataKind, const VCompareOptions& inOptions, VJSONArray* jsarr = nil);

DB4DArrayOfValues* DBAVVCreator(sLONG signature, VStream *inStream, XBOX::VError& outError);


			/* ------------------------------------------------------------------- */



template <class Type>
class DB4DArrayOfConstDirectValues : public DB4DArrayOfConstValues
{
	public:

		inline DB4DArrayOfConstDirectValues(DB4DArrayOfValues* inSource, sLONG inDataKind, const VCompareOptions& inOptions)
		{
			fOneNull = inSource->AtLeastOneNull();
			fOptions = inOptions;
			fDataKind = inDataKind;
			fSize = inSource->Count();
			fDataArray = (Type*)(inSource->GetFirstPtr());
			fSource = inSource;
			fSource->Retain();
		}

		inline DB4DArrayOfConstDirectValues(DB4DArrayOfDirectValues<Type>* inSource)
		{
			fOneNull = inSource->fOneNull;
			fOptions = inSource->fOptions;
			fDataKind = inSource->fDataKind;
			fSize = (sLONG)inSource->fArray.size();
			if (fSize == 0)
				fDataArray = nil;
			else
				fDataArray = &(inSource->fArray[0]);
			fSource = inSource;
			fSource->Retain();
		}

		virtual ~DB4DArrayOfConstDirectValues() 
		{ 
			if (fSource != nil)
				fSource->Release();
		}

		class MyLess
		{
		public:
			const VCompareOptions& fxOptions;
			inline MyLess(const VCompareOptions& inOptions):fxOptions(inOptions) { ; }

			inline bool operator()(const Type& val1, const Type& val2)
			{
				return val1 < val2;
			}
		};

		virtual const void* GetFirstPtr() const
		{
			fCurrent = fDataArray;
			return fCurrent;
		}

		virtual const void* GetNextPtr() const
		{
			fCurrent++;
			if (fCurrent >= (fDataArray + fSize))
				return nil;
			else
				return fCurrent;
		}

		virtual const void* GetLastPtr() const
		{
			return fDataArray + fSize - 1;
		}

		virtual Boolean IsAllNull() const
		{
			return fSize == 0 && fOneNull;
		}

		virtual Boolean AtLeastOneNull() const
		{
			return fOneNull;
		}


		inline void Sort()
		{
			MyLess pred(fOptions);
			MyLess& predref = pred;
			sort(fDataArray, fDataArray + fSize, predref);
		}

		inline const Type* First() const
		{
			fCurrent = fDataArray;
			return fCurrent;
		}

		inline const Type* Next() const
		{
			fCurrent++;
			if (fCurrent >= (fDataArray + fSize))
				return nil;
			else
				return fCurrent;
		}

		inline virtual sLONG Count() const
		{
			return fSize;
		}

		virtual Boolean FindWithDataPtr(void* inDataPtr, const XBOX::VCompareOptions& inOptions) const
		{
			MyLess pred(inOptions);
			MyLess& predref = pred;
			return binary_search(fDataArray, fDataArray + fSize, *((Type*)inDataPtr), predref);
		}

		inline Boolean FindDirect(const Type* val, const XBOX::VCompareOptions& inOptions) const
		{
			MyLess pred(inOptions);
			MyLess& predref = pred;
			return binary_search(fDataArray, fDataArray + fSize, *val, predref);
		}

		virtual Boolean Find(XBOX::VValueSingle& inValue, const XBOX::VCompareOptions& inOptions) const
		{
#if COMPIL_VISUAL
			tagada++;
#endif
			assert(false);  // must be specialized
			return false;
		}

		virtual XBOX::VError PutInto(XBOX::VStream& outStream) const
		{
			if (fSource == nil)
				return ThrowBaseError(VE_DB4D_THIS_IS_NULL);
			else
			{
				outStream.PutLong(fSource->GetSignature());
				return fSource->PutInto(outStream);
			}
		}

		virtual XBOX::VError GetFrom(XBOX::VStream& inStream)
		{
			assert(false);
			return VE_DB4D_NOTIMPLEMENTED;
		}

		virtual sLONG GetSignature() const
		{
			return 'cons';
		}

		virtual CompareResult CompareKeyWithValue(const void* key, const void* val, const VCompareOptions& inOptions) const
		{
			if ( *((Type*)key) == *((Type*)val) )
				return CR_EQUAL;
			else
			{
				if ( *((Type*)key) < *((Type*)val) )
					return CR_SMALLER;
				else
					return CR_BIGGER;
			}
		}

		virtual Boolean CanBeUsedWithIndex(XBOX::VIntlMgr* inIntlMgr) const
		{
			return true;
		}

		VJSONValue GetJSONValue(const Type& val, UniChar wildcharReplace) const
		{
			return VJSONValue(val);
		}

		virtual VJSONArray* ToJSON(CDB4DBaseContext* context, UniChar wildcharReplace) const
		{
			VJSONArray* arr = new VJSONArray();
			for (const Type *cur = fDataArray, *end = fDataArray+fSize; cur != end; ++cur)
			{
				arr->Push(GetJSONValue(*cur, wildcharReplace));
			}
			return arr;
		}



	protected:
		const Type* fDataArray;
		mutable const Type* fCurrent;
		VCompareOptions fOptions;
		sLONG fDataKind;
		sLONG fSize;
		DB4DArrayOfValues* fSource;
		uBYTE fvalues[2];
		Boolean fOneNull;
};



template <class Type>
DB4DArrayOfConstValues* DB4DArrayOfDirectValues<Type>::GenerateConstArrayOfValues()
{
	return new DB4DArrayOfConstDirectValues<Type>(this);
}


//DB4DArrayOfValues* DBAVConsCreator(sLONG signature, VStream *inStream, XBOX::VError& outError);

						/* ---------------------- */


template <>
inline VJSONValue DB4DArrayOfConstDirectValues<uBYTE>::GetJSONValue(const uBYTE& val, UniChar wildcharReplace) const
{
	if (val == 1)
		return VJSONValue(JSON_true);
	else
		return VJSONValue(JSON_false);
}


template <>
Boolean DB4DArrayOfConstDirectValues<uBYTE>::Find(XBOX::VValueSingle& inValue, const XBOX::VCompareOptions& inOptions) const;


template <>
inline DB4DArrayOfConstDirectValues<uBYTE>::DB4DArrayOfConstDirectValues(DB4DArrayOfValues* inSource, sLONG inDataKind, const VCompareOptions& inOptions)
{
	typedef struct LE_BOOL
	{
		uBYTE				fTrue;
		uBYTE				fFalse;
	} LE_BOOL;

	fOneNull = inSource->AtLeastOneNull();
	fOptions = inOptions;
	fDataKind = inDataKind;
	fSize = 0;
	LE_BOOL* sourceboolptr = (LE_BOOL*)(inSource->GetFirstPtr());
	if (sourceboolptr->fFalse)
	{
		fvalues[fSize] = 0;
		fSize++;
	}
	if (sourceboolptr->fTrue)
	{
		fvalues[fSize] = 1;
		fSize++;
	}

	fDataArray = &fvalues[0];
	fSource = inSource;
	fSource->Retain();
}


						/* ---------------------- */

template <>
Boolean DB4DArrayOfConstDirectValues<Real>::Find(XBOX::VValueSingle& inValue, const XBOX::VCompareOptions& inOptions) const;


template <>
inline bool DB4DArrayOfConstDirectValues<Real>::MyLess::operator ()(const Real& val1, const Real& val2)
{
	if (fabs(val1 - val2) < fxOptions.GetEpsilon())
		return false;
	else
		return val1 < val2;
}


template <>
inline CompareResult DB4DArrayOfConstDirectValues<Real>::CompareKeyWithValue(const void* key, const void* val, const VCompareOptions& inOptions) const
{
	if ( fabs(*((Real*)key) - *((Real*)val)) <inOptions.GetEpsilon() )
		return CR_EQUAL;
	else
	{
		if ( *((Real*)key) < *((Real*)val) )
			return CR_SMALLER;
		else
			return CR_BIGGER;
	}
}


						/* ---------------------- */

template <>
inline VJSONValue DB4DArrayOfConstDirectValues<QuickString>::GetJSONValue(const QuickString& val, UniChar wildcharReplace) const
{
	VString s;
	s.LoadFromPtr(val.fString);
	if (wildcharReplace != 0 && fOptions.IsLike() && fOptions.GetIntlManager() != nil)
	{
		s.ExchangeAll(fOptions.GetIntlManager()->GetWildChar(), wildcharReplace);
	}
	return VJSONValue(s);
}


template <>
inline Boolean DB4DArrayOfConstDirectValues<QuickString>::CanBeUsedWithIndex(XBOX::VIntlMgr* inIntlMgr) const
{
	Boolean result = true;
	const QuickString* cur = First();

	for (sLONG i = 0,nb = Count(); (i < nb) && result ; i++,cur++)
	{
		sLONG nbchar = *((sLONG*)cur->fString);
		const UniChar *pc = cur->fString+2;
		result = inIntlMgr->GetCollator()->IsPatternCompatibleWithDichotomyAndDiacritics( pc, nbchar);
	}
	return result;
}

template <>
inline CompareResult DB4DArrayOfConstDirectValues<QuickString>::CompareKeyWithValue(const void* key, const void* val, const VCompareOptions& inOptions) const
{
	const UniChar* val1 = (const UniChar*)key;
	const QuickString* val2 = (const QuickString*)val;

	return inOptions.GetIntlManager()->CompareString(val1 + 2, *((sLONG*)val1), val2->fString + 2, *((sLONG*)val2->fString), inOptions);
}



template <>
inline bool DB4DArrayOfConstDirectValues<QuickString>::MyLess::operator ()(const QuickString& val1, const QuickString& val2)
{
	return fxOptions.GetIntlManager()->CompareString(val1.fString + 2, *((sLONG*)val1.fString), val2.fString + 2, *((sLONG*)val2.fString), fxOptions) == CR_SMALLER;
}


template <>
inline Boolean DB4DArrayOfConstDirectValues<QuickString>::FindWithDataPtr(void* inDataPtr, const XBOX::VCompareOptions& inOptions) const
{
	MyLess pred(inOptions);
	MyLess& predref = pred;
	QuickString xs;
	xs.fString = (UniChar*)inDataPtr;
	return binary_search(fDataArray, fDataArray + fSize, xs, predref);
}


template <>
Boolean DB4DArrayOfConstDirectValues<QuickString>::Find(XBOX::VValueSingle& inValue, const XBOX::VCompareOptions& inOptions) const;


						/* ---------------------- */


template <>
inline Boolean DB4DArrayOfConstDirectValues<VInlineString>::CanBeUsedWithIndex(XBOX::VIntlMgr* inIntlMgr) const
{
	Boolean result = true;
	const VInlineString* cur = First();

	for (sLONG i = 0,nb = Count(); (i < nb) && result ; i++,cur++)
	{
		result = inIntlMgr->GetCollator()->IsPatternCompatibleWithDichotomyAndDiacritics( cur->GetCPointer(), cur->GetLength());
	}

	return result;
}


template <>
inline Boolean DB4DArrayOfConstDirectValues<VInlineString>::FindWithDataPtr(void* inDataPtr, const XBOX::VCompareOptions& inOptions) const
{
	MyLess pred(inOptions);
	MyLess& predref = pred;
	VInlineString temp;
	UniChar* val = (UniChar*)inDataPtr;
	temp.fString = val + 2;
	temp.fLength = *((sLONG*)val);
	temp.fMaxLength = temp.fLength;
	temp.fTag = 0;
	
	bool found;
	if (inOptions.IsLike())
	{
		found = false;
		VCollator *collator = inOptions.GetIntlManager()->GetCollator();
		for( const VInlineString *i = fDataArray ; (i != fDataArray + fSize) && !found ; ++i)
			found = collator->EqualString_Like( temp.fString, temp.fLength, i->fString, i->fLength, inOptions.IsDiacritical());
	}
	else
	{
		found = binary_search(fDataArray, fDataArray + fSize, temp, predref);
	}

	return found;
}


template <>
inline CompareResult DB4DArrayOfConstDirectValues<VInlineString>::CompareKeyWithValue(const void* key, const void* val, const VCompareOptions& inOptions) const
{
	const UniChar* val1 = (const UniChar*)key;
	const VInlineString* val2 = (const VInlineString*)val;

	return inOptions.GetIntlManager()->CompareString(val1 + 2, *((sLONG*)val1), val2->fString, val2->fLength, inOptions);
}


template <>
inline bool DB4DArrayOfConstDirectValues<VInlineString>::MyLess::operator ()(const VInlineString& val1, const VInlineString& val2)
{
	return fxOptions.GetIntlManager()->CompareString(val1.fString, val1.fLength, val2.fString, val2.fLength, fxOptions) == CR_SMALLER;
}


template <>
Boolean DB4DArrayOfConstDirectValues<VInlineString>::Find(XBOX::VValueSingle& inValue, const XBOX::VCompareOptions& inOptions) const;

template <>
inline VJSONValue DB4DArrayOfConstDirectValues<VInlineString>::GetJSONValue(const VInlineString& val, UniChar wildcharReplace) const
{
	VString s(val);
	if (wildcharReplace != 0 && fOptions.IsLike() && fOptions.GetIntlManager() != nil)
	{
		s.ExchangeAll(fOptions.GetIntlManager()->GetWildChar(), wildcharReplace);
	}
	return VJSONValue(s);
}



						// -------------------------------------------

template <>
Boolean DB4DArrayOfConstDirectValues<sBYTE>::Find(XBOX::VValueSingle& inValue, const XBOX::VCompareOptions& inOptions) const;

template <>
Boolean DB4DArrayOfConstDirectValues<sWORD>::Find(XBOX::VValueSingle& inValue, const XBOX::VCompareOptions& inOptions) const;

template <>
Boolean DB4DArrayOfConstDirectValues<sLONG>::Find(XBOX::VValueSingle& inValue, const XBOX::VCompareOptions& inOptions) const;

template <>
Boolean DB4DArrayOfConstDirectValues<sLONG8>::Find(XBOX::VValueSingle& inValue, const XBOX::VCompareOptions& inOptions) const;

template <>
Boolean DB4DArrayOfConstDirectValues<uLONG8>::Find(XBOX::VValueSingle& inValue, const XBOX::VCompareOptions& inOptions) const;

template <>
Boolean DB4DArrayOfConstDirectValues<xTime>::Find(XBOX::VValueSingle& inValue, const XBOX::VCompareOptions& inOptions) const;



template <>
inline VJSONValue DB4DArrayOfConstDirectValues<xTime>::GetJSONValue(const xTime& val, UniChar wildcharReplace) const
{
	VTime tt;
	tt.FromStamp(val.GetStamp());
	VString s;
	tt.GetXMLString(s, XSO_Default);
	return VJSONValue(s);
}


template <>
Boolean DB4DArrayOfConstDirectValues<VUUIDBuffer>::Find(XBOX::VValueSingle& inValue, const XBOX::VCompareOptions& inOptions) const;



template <>
inline VJSONValue DB4DArrayOfConstDirectValues<VUUIDBuffer>::GetJSONValue(const VUUIDBuffer& val, UniChar wildcharReplace) const
{
	VString s;
	VUUID xid(val);
	xid.GetString(s);
	return VJSONValue(s);
}

						// -------------------------------------------


DB4DArrayOfValues* GenerateConstArrayOfValues(DB4DArrayOfValues* values, sLONG datakind, const VCompareOptions& inOptions);

DB4DArrayOfValues* GenerateConstArrayOfValues(VStream& inStream, sLONG datakind, const VCompareOptions& inOptions);

DB4DArrayOfValues* GenerateConstArrayOfValues(VJSArray& jsarr, sLONG datakind, const VCompareOptions& inOptions);

DB4DArrayOfValues* GenerateConstArrayOfValues(VJSONArray* jsarr, sLONG datakind, const VCompareOptions& inOptions);


						// ---------------------------------------------------------------------------






class DB4DKeyWordList : public VArrayOf<VString*>
{
public:
	inline DB4DKeyWordList() { mustdispose = true; };
	inline void DontDisposeData() { mustdispose = false; };
	virtual ~DB4DKeyWordList();

protected:
	Boolean mustdispose; 
};

VError BuildKeyWords(const VString& source, DB4DKeyWordList& outKeyWords, const VCompareOptions& inOptions);


class AutoSeqNumber : public ObjAlmostInCache
{
	public:

		AutoSeqNumber(Base4D* owner, sLONG num);
		inline const VUUID& GetID() const { return fID; };
		inline void SetID(const VUUID& newID) { fID = newID; };
		inline void SetInvalid() { fIsValid = false; };
		inline Base4D* GetOwner() const { return fOwner; };

		void SetStartingValue(sLONG8 initialvalue);
		sLONG8 GetStartingValue() const;

		virtual void SetCurrentValue(sLONG8 currentvalue, bool canBeLower = true) = 0;
		virtual sLONG8 GetCurrentValue() const = 0;

		virtual uBOOL okdel(void) { return false; };
		//virtual sLONG saveobj(void);
		virtual VError loadobj(DataAddr4D addr, DataBaseObjectHeader& tag) = 0;
		virtual VError InitForNew();

		virtual sLONG8 GetNewValue(DB4D_AutoSeqToken& ioToken) = 0;
		virtual void ValidateValue(DB4D_AutoSeqToken inToken, Table* inTable, BaseTaskInfo* context) = 0;
		virtual void InvalidateValue(DB4D_AutoSeqToken inToken) = 0;

		inline sLONG GetNum() const { return fNum; };

#if debugLeaksAll
		virtual bool OKToTrackDebug() const
		{
			return false;
		}

		virtual void GetDebugInfo(VString& outText) const
		{
			outText = "AutoSequence";
		}

#endif

	protected:
		VUUID fID;
		sLONG8 fInitialValue;
		DB4D_AutoSeqToken fCurToken;
		Base4D* fOwner;
		sLONG fNum;
		Boolean fIsValid;

};


typedef std::map<DB4D_AutoSeqToken, sLONG8, less<DB4D_AutoSeqToken>, cache_allocator<pair<const DB4D_AutoSeqToken, sLONG8> > > MapOfContextAndValue;


class AutoSeqNumberSimple : public AutoSeqNumber
{
	public:
		inline AutoSeqNumberSimple(Base4D* owner, sLONG num):AutoSeqNumber(owner, num) { ; };

		virtual VError LibereEspaceDisk(VDB4DProgressIndicator* InProgress);
		virtual sLONG saveobj(void);
		virtual VError loadobj(DataAddr4D addr, DataBaseObjectHeader& tag);
		virtual VError InitForNew();

		virtual void SetCurrentValue(sLONG8 currentvalue, bool canBeLower = true);
		sLONG8 GetCurrentValue() const;

		virtual sLONG8 GetNewValue(DB4D_AutoSeqToken& ioToken);
		virtual void ValidateValue(DB4D_AutoSeqToken inToken, Table* inTable, BaseTaskInfo* context);
		virtual void InvalidateValue(DB4D_AutoSeqToken inToken);

	protected:
		sLONG8 fCurValue;
		MapOfContextAndValue fWaitingForValidate;
};


typedef std::vector<sLONG8> xArrayOfLong8;

class AutoSeqNumberNoHole : public AutoSeqNumber
{
public:
	inline AutoSeqNumberNoHole(Base4D* owner, sLONG num):AutoSeqNumber(owner, num) { ; };

	sLONG CalcLen();
	VError save();

	virtual VError LibereEspaceDisk(VDB4DProgressIndicator* InProgress);
	virtual sLONG saveobj(void);
	virtual VError loadobj(DataAddr4D addr, DataBaseObjectHeader& tag);
	virtual VError InitForNew();

	virtual void SetCurrentValue(sLONG8 currentvalue, bool canBeLower = true);
	sLONG8 GetCurrentValue() const;

	virtual sLONG8 GetNewValue(DB4D_AutoSeqToken& ioToken);
	virtual void ValidateValue(DB4D_AutoSeqToken inToken, Table* inTable, BaseTaskInfo* context);
	virtual void InvalidateValue(DB4D_AutoSeqToken inToken);

protected:
	sLONG8 fCurValue;
	MapOfContextAndValue fWaitingForValidate;
	xArrayOfLong8 fHoles;
	sLONG fAntelen;
};


																// -----------------------------------------


class VBlobTextInfo : public VString_info
{
public:
	VBlobTextInfo(ValueKind typ = VK_TEXT):VString_info( typ)	{;}
	virtual	VValue*	Generate() const;
	virtual	VValue*	Generate(VBlob* inBlob) const;
	virtual void*	ByteSwapPtr( void *inBackStore, bool inFromNative) const;
};


class VBlobText : public VString
{
public:
	typedef VBlobTextInfo	InfoType;
	static const InfoType	sInfo;

	virtual const VValueInfo*	GetValueInfo() const;

	VBlobText( sLONG *inNumBlob, BlobText *inBlob);
	inline VBlobText() { fBlob = nil; fTextSwitchSize = 0; fForcePathIfEmpty = false;};
	virtual ~VBlobText();
	virtual VError Flush( void *inDataPtr, void *InContext) const;
	virtual void SetReservedBlobNumber(sLONG inBlobNum);
	virtual VBlobText *Clone() const;
	virtual VSize	GetSpace (VSize inMax = 0) const;
	virtual VSize	GetFullSpaceOnDisk() const;
	virtual void Detach(Boolean inMayEmpty = false);
	virtual VValue* FullyClone(bool ForAPush = false) const;
	virtual void RestoreFromPop(void* context);

	virtual void* WriteToPtr(void* pData,Boolean pRefOnly, VSize inMax = 0 ) const;

	BlobText* GetBlob4D(BaseTaskInfo* context = nil, bool duplicateIfNecessary = false);

	inline void SetTextSwitchSize(sLONG len) { fTextSwitchSize = len; };

	virtual void SetOutsidePath(const VString& inPosixPath, bool inIsRelative = false);

	virtual void GetOutsidePath(VString& outPosixPath, bool* outIsRelative = NULL);

	virtual void GetTrueOutsidePath(VString& outPosixPath, bool* outIsRelative = NULL);

	virtual void SetOutsideSuffixe(const VString& inExtention);

	virtual bool IsOutsidePath();

	virtual VError ReloadFromOutsidePath();

	virtual void SetHintRecNum(sLONG TableNum, sLONG FieldNum, sLONG inHint);

	virtual bool ForcePathIfEmpty() const;

	virtual void SetForcePathIfEmpty(bool x);

	virtual void AssociateRecord(void* primkey, sLONG FieldNum);


protected:
	virtual void DoNullChanged();

	BlobText *fBlob;
	sLONG fTextSwitchSize;
	bool fForcePathIfEmpty;
};


class BlobTextUTF8;

class VBlobTextUTF8Info : public VBlobTextInfo
{
public:
	VBlobTextUTF8Info():VBlobTextInfo( VK_TEXT_UTF8)	{;}
	virtual	VValue*	Generate() const;
	virtual	VValue*	Generate(VBlob* inBlob) const;
};

class VBlobTextUTF8 : public VBlobText
{
public:
	typedef VBlobTextUTF8Info	InfoType;
	static const InfoType	sInfo;

	virtual const VValueInfo*	GetValueInfo() const
	{
		return &sInfo;
	}

	VBlobTextUTF8( sLONG *inNumBlob, BlobTextUTF8 *inBlob);

	virtual VError ReloadFromOutsidePath();

};



class VBlob4DWithPtrInfo : public VBlob::TypeInfo
{
public:
	VBlob4DWithPtrInfo():VBlob::TypeInfo( VK_BLOB_DB4D)	{;}
	VBlob4DWithPtrInfo(ValueKind kind):VBlob::TypeInfo( kind)	{;}
	virtual void*	ByteSwapPtr( void *inBackStore, bool inFromNative) const;
};


class ENTITY_API VBlob4DWithPtr : public VBlob
{
public:
	typedef VBlob4DWithPtrInfo	InfoType;
	static const InfoType	sInfo;

	virtual const VValueInfo*	GetValueInfo() const;

	VBlob4DWithPtr( sLONG *inNumBlob, BlobWithPtr *inBlob);
	VBlob4DWithPtr( const VBlob4DWithPtr& inBlob);
	virtual ~VBlob4DWithPtr();

	virtual void* WriteToPtr(void* pData,Boolean pRefOnly, VSize inMax = 0 ) const;
	virtual VSize GetSpace (VSize inMax = 0) const;

	// Returns bytes count
	virtual VSize	GetSize () const;

	// Changes the size. (not needed before PutData).
	virtual VError	SetSize (VSize inNewSize);

	// Reads some bytes, returns number of bytes copied.
	virtual VError	GetData (void* inBuffer, VSize inNbBytes, VIndex inOffset = 0, VSize* outNbBytesCopied = NULL) const;

	// Writes some bytes. Resizes if necessary.
	virtual VError	PutData (const void* inBuffer, VSize inNbBytes, VIndex inOffset = 0);

	virtual void Detach(Boolean inMayEmpty = false);
	virtual Boolean FromValueSameKind( const VValue* inValue);
	virtual VBlob4DWithPtr *Clone() const;
	virtual VValue* FullyClone(bool ForAPush = false) const;
	virtual void RestoreFromPop(void* context);
	virtual VError Flush( void *inDataPtr, void *InContext) const;
	virtual void SetReservedBlobNumber(sLONG inBlobNum);

	BlobWithPtr *GetBlob4D() const {return fBlob;}

	virtual void GetValue( VValueSingle& outValue) const;
	virtual void FromValue( const VValueSingle& inValue);

	virtual	Boolean					EqualToSameKind( const VValue* inValue, Boolean inDiacritical = false) const;

	// Inherited from IStreamable
	virtual	VError					ReadFromStream( VStream* ioStream, sLONG inParam = 0);
	virtual VError					WriteToStream( VStream* ioStream, sLONG inParam = 0) const;

	inline void SetBlobSwitchSize(sLONG len) { fBlobSwitchSize = len; };

	virtual void SetOutsidePath(const VString& inPosixPath, bool inIsRelative = false);

	virtual void GetOutsidePath(VString& outPosixPath, bool* outIsRelative = NULL);

	virtual void GetTrueOutsidePath(VString& outPosixPath, bool* outIsRelative = NULL);

	virtual void SetOutsideSuffixe(const VString& inExtention);

	virtual bool IsOutsidePath();

	virtual VError ReloadFromOutsidePath();

	virtual void SetHintRecNum(sLONG TableNum, sLONG FieldNum, sLONG inHint);

	virtual bool ForcePathIfEmpty() const;

	virtual void SetForcePathIfEmpty(bool x);

	virtual void AssociateRecord(void* primkey, sLONG FieldNum);

protected:
	virtual void DoNullChanged();
	VError _CheckNeedDuplicate(Boolean withdata);

	BlobWithPtr *fBlob;
	sLONG fBlobSwitchSize;
	bool fForcePathIfEmpty;
};



class VBlob4DWithPtrAsRawPictInfo : public VBlob4DWithPtrInfo
{
public:
	VBlob4DWithPtrAsRawPictInfo():VBlob4DWithPtrInfo( VK_OBSOLETE_STRING_DB4D)	{ _SetKind( VK_IMAGE);}
};

class VBlob4DWithPtrAsRawPict : public VBlob4DWithPtr
{
public:
	typedef VBlob4DWithPtrAsRawPictInfo	InfoType;
	static const InfoType	sInfo;

	virtual const VValueInfo*	GetValueInfo() const;
	VBlob4DWithPtrAsRawPict( sLONG *inNumBlob, BlobWithPtr *inBlob):VBlob4DWithPtr(inNumBlob, inBlob) { ; }
	VBlob4DWithPtrAsRawPict( const VBlob4DWithPtrAsRawPict& inBlob):VBlob4DWithPtr(inBlob) { ; }

	virtual VError WriteToStream( VStream* ioStream, sLONG inParam = 0) const;

	virtual VBlob4DWithPtrAsRawPict *Clone() const
	{
		return new VBlob4DWithPtrAsRawPict( *this);
	}
};




class FicheInMem;
class Table;
class SearchTab;
class SortTab;

typedef Vx1ArrayOf<FicheInMem*, 20> ListOfSubRecord;
typedef Vx1ArrayOf<sLONG,20> SelOfSubRecords;

class VSubTableDB4D : public VSubTable
{
	public:
		typedef VValueInfo_scalar<VSubTableDB4D,VK_SUBTABLE,sLONG8>	InfoType;
		static const InfoType		sInfo;

		VSubTableDB4D (sLONG8 inValue = 0);
		VSubTableDB4D (const VSubTableDB4D& inOriginal);
		VSubTableDB4D (sLONG8* inDataPtr, Boolean inInit);

		void xInit();

		virtual ~VSubTableDB4D();

		const VValueInfo*		GetValueInfo() const;

		void Detach();
		VError LoadSubRecords(FicheInMem* parent, Field* cri, DB4D_Way_of_Locking HowToLock, BaseTaskInfo* context, Boolean& outSomeLocked, Boolean* outEnoughMem);
		VError SaveSubRecords(Field* cri, BaseTaskInfo* context, Transaction* &ioSubTrans);
		VError InitForNewRecord(FicheInMem* parent, Field* cri, BaseTaskInfo* context);
		sLONG FindSubRecNumInSubSel(sLONG nfic);
		Boolean CompareSubRecords(sLONG n1, sLONG n2);

		FicheInMem* GetNthSubRecord(sLONG n, VError& err,  BaseTaskInfo* context, Boolean inSubSelection = true);
		sLONG CountRecord(BaseTaskInfo* context, Boolean inSubSelection = true);
		FicheInMem* AddSubRecord(VError& err, BaseTaskInfo* context, Boolean inSubSelection = true);
		VError DeleteNthRecord(sLONG n, BaseTaskInfo* context, Boolean inSubSelection = true);
		VError QuerySubRecord(const SearchTab* query, BaseTaskInfo* context);
		VError SortSubRecord(const SortTab* criterias, BaseTaskInfo* context);
		VError AllSubRecords(BaseTaskInfo* context);

		VError AddOrUpdateSubRecord(FicheInMem* subrec);
		VError DeleteSubRecord(FicheInMem* subrec);

		sLONG CountDeletedRecord()
		{
			return fDeletedSubRecs.GetCount();
		}

		FicheInMem* GetNthDeletedSubRecord(sLONG n)
		{
			return fDeletedSubRecs[n];
		}

		void UnMarkAllSubRecords();
		FicheInMem* FindSubRecNum(sLONG recnum);
		VError DeleteSubRecordsNotMarked();


		virtual CDB4DRecord* RetainNthSubRecord(sLONG inIndex, VErrorDB4D& err, CDB4DBaseContextPtr inContext, Boolean inSubSelection);
		virtual sLONG CountSubRecords(CDB4DBaseContextPtr inContext, Boolean inSubSelection);
		virtual CDB4DRecord* AddSubRecord(VErrorDB4D& err, CDB4DBaseContextPtr inContext, Boolean inSubSelection);
		virtual VErrorDB4D DeleteNthRecord(sLONG inIndex, CDB4DBaseContextPtr inContext, Boolean inSubSelection);
		virtual VErrorDB4D QuerySubRecords(const CDB4DQuery* inQuery, CDB4DBaseContextPtr inContext);
		virtual VErrorDB4D SortSubRecords(const CDB4DSortingCriterias* inCriterias, CDB4DBaseContextPtr inContext);
		virtual VErrorDB4D AllSubRecords(CDB4DBaseContextPtr inContext);
		virtual VErrorDB4D SetSubSel(CDB4DBaseContextPtr inContext, const SubRecordIDVector& inRecords);
		virtual sLONG GetSubRecordIDFromSubSel(CDB4DBaseContextPtr inContext, sLONG PosInSel);

		inline Table* GetSubTable() const
		{
			return fSubTable;
		}

		inline Boolean HasBeenLoaded() const
		{
			return fWasLoaded;
		}

		virtual VValue* FullyClone(bool ForAPush = false) const;
		virtual void RestoreFromPop(void* context);

		inline void SetParent(FicheInMem *newparent)
		{
			fParent = newparent;
		}

	protected:
		Table* fSubTable;
		Field* fSource;
		Field* fDest;
		FicheInMem* fParent;
		const SortTab* fCurSort;
		ListOfSubRecord fSubRecs;
		ListOfSubRecord fDeletedSubRecs;
		SelOfSubRecords fSubSel;
		Boolean fIsRemote, fWasLoaded;
};


class VSubTableKeyDB4D : public VSubTableKey
{
	public:
		typedef VValueInfo_scalar<VSubTableKeyDB4D,VK_SUBTABLE_KEY,sLONG8>	InfoType;
		static const InfoType		sInfo;

		VSubTableKeyDB4D (sLONG8 inValue = 0);
		VSubTableKeyDB4D (const VSubTableKeyDB4D& inOriginal);
		VSubTableKeyDB4D (sLONG8* inDataPtr, Boolean inInit);

		const VValueInfo*		GetValueInfo() const;

};


template <class T>
class AutoDeletePtr {
public:
	AutoDeletePtr (T *ptr) : fPtr(ptr) { }
	T*	operator->() const { return (fPtr); }
	operator T*() const { return (fPtr); }
	~AutoDeletePtr () { delete (fPtr); }
private:
	T	*fPtr;
};



//	typedef AutoDeletePtr<ChampVar>	ADPChampVar;


													// *************************************

typedef uLONG creVValue_ParamType;

const creVValue_ParamType creVValue_default = 0;
const creVValue_ParamType creVValue_outsidepath = 1;

typedef ValPtr (*CreVValue_Code) (DataTable *df, sLONG maxlen,void* from, uBOOL init, uBOOL formodif, BaseTaskInfo* context, creVValue_ParamType parameters);

ValPtr CreVString(DataTable *df, sLONG maxlen,void* from, uBOOL init, uBOOL formodif, BaseTaskInfo* context = nil, creVValue_ParamType parameters = creVValue_default);
ValPtr CreVStringUTF8(DataTable *df, sLONG maxlen,void* from, uBOOL init, uBOOL formodif, BaseTaskInfo* context = nil, creVValue_ParamType parameters = creVValue_default);
ValPtr CreVBlobText(DataTable *df, sLONG maxlen,void* from, uBOOL init, uBOOL formodif, BaseTaskInfo* context = nil, creVValue_ParamType parameters = creVValue_default);
ValPtr CreVBlobTextUTF8(DataTable *df, sLONG maxlen,void* from, uBOOL init, uBOOL formodif, BaseTaskInfo* context = nil, creVValue_ParamType parameters = creVValue_default);
ValPtr CreVLong(DataTable *df, sLONG maxlen,void* from, uBOOL init, uBOOL formodif, BaseTaskInfo* context = nil, creVValue_ParamType parameters = creVValue_default);
ValPtr CreVShort(DataTable *df, sLONG maxlen,void* from, uBOOL init, uBOOL formodif, BaseTaskInfo* context = nil, creVValue_ParamType parameters = creVValue_default);
ValPtr CreVBoolean(DataTable *df, sLONG maxlen,void* from, uBOOL init, uBOOL formodif, BaseTaskInfo* context = nil, creVValue_ParamType parameters = creVValue_default);
ValPtr CreVByte(DataTable *df, sLONG maxlen,void* from, uBOOL init, uBOOL formodif, BaseTaskInfo* context = nil, creVValue_ParamType parameters = creVValue_default);
ValPtr CreVDuration(DataTable *df, sLONG maxlen,void* from, uBOOL init, uBOOL formodif, BaseTaskInfo* context = nil, creVValue_ParamType parameters = creVValue_default);
ValPtr CreVReal(DataTable *df, sLONG maxlen,void* from, uBOOL init, uBOOL formodif, BaseTaskInfo* context = nil, creVValue_ParamType parameters = creVValue_default);
ValPtr CreVLong8(DataTable *df, sLONG maxlen,void* from, uBOOL init, uBOOL formodif, BaseTaskInfo* context = nil, creVValue_ParamType parameters = creVValue_default);
ValPtr CreVTime(DataTable *df, sLONG maxlen,void* from, uBOOL init, uBOOL formodif, BaseTaskInfo* context = nil, creVValue_ParamType parameters = creVValue_default);
ValPtr CreVBlob4DWithPtr(DataTable *df, sLONG maxlen,void* from, uBOOL init, uBOOL formodif, BaseTaskInfo* context = nil, creVValue_ParamType parameters = creVValue_default);
ValPtr CreVBlobPicture(DataTable *df, sLONG maxlen,void* from, uBOOL init, uBOOL formodif, BaseTaskInfo* context = nil, creVValue_ParamType parameters = creVValue_default);
ValPtr CreVEmpty(DataTable *df, sLONG maxlen,void* from, uBOOL init, uBOOL formodif, BaseTaskInfo* context = nil, creVValue_ParamType parameters = creVValue_default);
ValPtr CreVUUID(DataTable *df, sLONG maxlen,void* from, uBOOL init, uBOOL formodif, BaseTaskInfo* context = nil, creVValue_ParamType parameters = creVValue_default);
ValPtr CreVSubTable(DataTable *df, sLONG maxlen,void* from, uBOOL init, uBOOL formodif, BaseTaskInfo* context = nil, creVValue_ParamType parameters = creVValue_default);
ValPtr CreVSubTableKey(DataTable *df, sLONG maxlen,void* from, uBOOL init, uBOOL formodif, BaseTaskInfo* context = nil, creVValue_ParamType parameters = creVValue_default);

extern CodeReg *CV_CodeReg;

inline CreVValue_Code FindCV(uLONG id) { return( (CreVValue_Code)(CV_CodeReg->GetCodeFromID(id)) ); };

inline void RegisterCV(uLONG id, CreVValue_Code Code) { CV_CodeReg->Register(id,(void*)Code); };



ValPtr CrePictAsRaw(DataTable *df, sLONG maxlen,void* from, uBOOL init, uBOOL formodif, BaseTaskInfo* context = nil, creVValue_ParamType parameters = creVValue_default);



													// *************************************
													
/*
ValPtr CreVValueFromBuffer(VStream& buf, sLONG typ=0);
ValPtr CreVValueFromPtr(sLONG typ, void* p, sLONG maxlen);
*/
ValPtr CreVValueFromNil(sLONG typ, sLONG maxlen, Boolean init = false);

sLONG InitChampVar();
void DeInitChampVar();




													// *************************************

class VirtualArrayIterator : public VObject {
public:
	virtual	Boolean			TextOnly(void) = 0;
	virtual sLONG			CountElems (void) = 0;
	virtual sLONG			CurElem (void) = 0;
	virtual Boolean			GetAlpha (VString *st, sLONG elem) = 0;
//	virtual Boolean			GetPicture (PicHandle &pic, sLONG elem) { pic = nil; return (false); }

	// FM CIconHandle ?
	//	virtual Boolean			GetCicon (CIconHandle &icn, sLONG elem) { icn = nil; return (false); }
	virtual Boolean			GetXicon (VIcon **icn, sLONG elem) { icn = nil; return (false); }
	virtual Boolean			ItemEnabled (sLONG elem) { return (true); }
	virtual sLONG			FindString (VString *st);
protected:
};

class EmptyArrayIterator : public VirtualArrayIterator {
public:
			EmptyArrayIterator (void) { }
	virtual	Boolean			TextOnly(void) { return (true); }
	virtual sLONG			CountElems (void) { return (0); }
	virtual sLONG			CurElem (void) { return (0); }
	virtual Boolean			GetAlpha (VString *st, sLONG elem) { st->Clear(); return (false); }
protected:
};

#endif