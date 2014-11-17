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


class ENTITY_API WafSelectionRange
{
	public:
		sLONG start;
		sLONG end;
};

typedef vector<WafSelectionRange> WafSelectionRangeVector;

class ENTITY_API WafSelection
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

		VJSONValue toJSON();

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

class User4DSession
{

public:
	User4DSession()
	{
		fReqInfo = nil;
	}

	~User4DSession()
	{
		QuickReleaseRefCountable(fReqInfo);
	}

		VTime fExpiresAt;
		VString fName;
		VString fPassword;
		sLONG fHowManyMinutes;
		VJSONObject* fReqInfo;
};

typedef map<VUUID, User4DSession> UserSessionMap;

const sLONG kUser4DSessionDefaultExpire = 60; // 60 min
const sLONG kUser4DSessionMinLength = 5; // 5 min
const sLONG kUser4DSessionCheckInterval = 3; // 3 sec




// --------------------------------------------------------------------------------------------


class AdminRequestHandler;

class AdminRequest
{
	public:
		AdminRequest(IHTTPResponse* inResponse, RIApplicationRef inApplicationRef, AdminRequestHandler* owner);
		~AdminRequest();

		VError ExecuteRequest();
		VError ReturnAdminPage();

	private:
		AdminRequestHandler* fOwner;
		RIApplicationRef fApplicationRef;
		IHTTPResponse* fResponse;
		const IHTTPRequest* fRequest;
		VStream* fInput;
		VStream* fOutput;
		VFullURL fURL;
};


class AdminRequestHandler : public IHTTPRequestHandler
{
	public:
		AdminRequestHandler(CDB4DBase* inBase, const VString& inPattern, RIApplicationRef inApplicationRef)
		{
			if (inBase != nil)
			{
				fxBase = RetainRefCountable(inBase);
				fBase = dynamic_cast<VDB4DBase*>(inBase)->GetBase();
			}
			else
			{
				fxBase = nil;
				fBase = nil;
			}
			fPattern = inPattern;
			fApplicationRef = inApplicationRef;
			fPrefix = "admin";
		}

		virtual ~AdminRequestHandler()
		{
			QuickReleaseRefCountable(fxBase);
		}

		VError			SetPattern( const VString& inPattern);
		virtual VError			GetPatterns( vector<VString>* outPatterns) const;
		virtual	VError			HandleRequest( IHTTPResponse* inResponse);

		const VString& GetPrefix() const
		{
			return fPrefix;
		}

	private:
		CDB4DBase* fxBase;
		Base4D* fBase;
		VString fPattern;
		VString fPrefix;
		RIApplicationRef fApplicationRef;
		mutable	VCriticalSection fMutex;
};




// -------------------------------------------------------------------------------------------



class RestRequestHandler : public IHTTPRequestHandler
{
	public:

		RestRequestHandler( CDB4DBase* inBase, const VString& inPattern, bool inEnabled, RIApplicationRef inApplicationRef) : fxBase(nil), fBase(nil), fPattern(inPattern), fEnable(inEnabled), fApplicationRef(inApplicationRef)
		{
			if (inBase != nil)
			{
				fxBase = RetainRefCountable(inBase);
				fBase = dynamic_cast<VDB4DBase*>(inBase)->GetBase();
			}
			VTime::Now(fLastExpireCheck);
			VDBMgr::GetManager()->RegisterRestHandler(this);
		}

		virtual ~RestRequestHandler()
		{
			VDBMgr::GetManager()->UnRegisterRestHandler(this);
			VDBMgr::GetManager()->GetRestContexts().DisposeAllContextsOnBase(fxBase);
			QuickReleaseRefCountable(fxBase);
		}

				VError			SetPattern( const VString& inPattern);
		virtual VError			GetPatterns( vector<VString>* outPatterns) const;
		virtual	VError			HandleRequest( IHTTPResponse* inResponse);

		virtual bool			GetEnable();
		virtual void			SetEnable( bool inEnable);
		virtual	void*			GetPrivateData() const { return fxBase; }

		void DescribeUserSessions(VValueBag& outBag);
		bool GetUserSession(const VUUID& inSessionID, User4DSession& outUserSession);
		bool ExistsUserSession(const VUUID& inSessionID);
		bool ForceExpireUserSession(const VUUID& inSessionID);
		void CreateUserSession(const VString& userName, const VString& password, const VUUID& inSessionUUID, sLONG howManyMinutes);
		void CheckUserSessionExpirations(bool forcecheck = false);
		void GetAllUserSessionIDs(vector<VUUID>& outSessions);
		VJSONObject* RetainUserSessionRequestInfo(const VUUID& inSessionID);
		void SetUserSessionRequestInfo(const VUUID& inSessionID, VJSONObject* inRequestInfo);


	private:
		CDB4DBase* fxBase;
		Base4D* fBase;
		bool fEnable;
		VString fPattern;
		RIApplicationRef fApplicationRef;
		UserSessionMap fUserMap;
		VCriticalSection fUserMapMutex;
		VTime fLastExpireCheck;

		mutable	VCriticalSection fMutex;
};

#endif

