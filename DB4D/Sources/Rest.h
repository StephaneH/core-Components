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
#ifndef __REST__
#define __REST__


#if UseDB4DJSContext
// temporary code

const VSize kMaxContextStores = 30;

class tempContextStore
{
public:
	CDB4DBaseContext* xcontext;
	DB4DJSRuntimeDelegate* xJSDelegate;
	VJSGlobalContext* xJSContext;
	VSize pos;
	VTaskID usedBy;
};

extern VCriticalSection tempContextStoreMutex;
extern vector<tempContextStore> tempStores;

// end of temporary code
#endif

class WafSelectionRange
{
	public:
		sLONG start;
		sLONG end;
};

typedef vector<WafSelectionRange> WafSelectionRangeVector;

class WafSelection
{
	public:
		vector<sLONG> rows;
		WafSelectionRangeVector ranges;
		vector<sLONG> butRows;

		WafSelection(VJSValue& val)
		{
			buildFromJS(val);
			curelem = 0;
			lastadded = -1;
		}

		WafSelection()
		{
			curelem = 0;
			lastadded = -1;
		}

		VError buildFromJS(VJSValue& val);

		sLONG count() const;

		VError AddElem();

		void SkipElem()
		{
			++curelem;
		}

		WafSelection& operator = (const WafSelection& other);

		const VString& GetMode() const
		{
			return fMode;
		};

	private:
		sLONG curelem;
		sLONG lastadded;
		VString fMode;
};

class RestRequestHandler : public IHTTPRequestHandler
{
	public:

		RestRequestHandler( CDB4DBase* inBase, const VString& inPattern, bool inEnabled, RIApplicationRef inApplicationRef) : fPattern(inPattern), fEnable(inEnabled), fApplicationRef(inApplicationRef)
		{
			fxBase = RetainRefCountable(inBase);
			fBase = VImpCreator<VDB4DBase>::GetImpObject(inBase)->GetBase();
		}

		virtual ~RestRequestHandler()
		{
			QuickReleaseRefCountable(fxBase);
		}

				VError			SetPattern( const VString& inPattern);
		virtual VError			GetPatterns( vector<VString>* outPatterns) const;
		virtual	VError			HandleRequest( IHTTPResponse* inResponse);

		virtual bool			GetEnable();
		virtual void			SetEnable( bool inEnable);
		virtual	void*			GetPrivateData() const { return fxBase; }


	private:
		CDB4DBase* fxBase;
		Base4D* fBase;
		bool fEnable;
		VString fPattern;
		RIApplicationRef fApplicationRef;

		mutable	VCriticalSection fMutex;
};

#if UseDB4DJSContext
extern void releaseAllTempStores(); // temporary code
#endif

#endif

