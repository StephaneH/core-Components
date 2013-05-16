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
#ifndef __WEB__
#define __WEB__



namespace rest
{
	EXTERN_BAGKEY(top);
	EXTERN_BAGKEY(expand);
	EXTERN_BAGKEY(subExpand);
	EXTERN_BAGKEY(all);
	EXTERN_BAGKEY(format);
	EXTERN_BAGKEY(imageformat);
	EXTERN_BAGKEY(prettyformatting);
	EXTERN_BAGKEY(prettyprint);
	EXTERN_BAGKEY(method);
	EXTERN_BAGKEY(atOnce);
	EXTERN_BAGKEY(atomic);
	EXTERN_BAGKEY(metadata);
	//	CREATE_BAGKEY_2(max, "$max");
	EXTERN_BAGKEY(limit);
	EXTERN_BAGKEY(skip);
	EXTERN_BAGKEY(filter);
	EXTERN_BAGKEY(params);
	EXTERN_BAGKEY(savedfilter);
	EXTERN_BAGKEY(querypath);
	EXTERN_BAGKEY(queryplan);
	EXTERN_BAGKEY(orderby);
	EXTERN_BAGKEY(savedorderby);
	EXTERN_BAGKEY(timeout);
	EXTERN_BAGKEY(changestamp);
	EXTERN_BAGKEY(imageinfo);
	EXTERN_BAGKEY(emMethod);
	EXTERN_BAGKEY(kind);
	EXTERN_BAGKEY(path);
	EXTERN_BAGKEY(distinct);
	EXTERN_BAGKEY(refresh);
	EXTERN_BAGKEY(progressinfo);
	EXTERN_BAGKEY(stop);
	EXTERN_BAGKEY(asArray);
	EXTERN_BAGKEY(noKey);
	EXTERN_BAGKEY(removeFromSet);
	EXTERN_BAGKEY(removeRefOnly);
	EXTERN_BAGKEY(addToSet);
	EXTERN_BAGKEY(queryLimit);
	EXTERN_BAGKEY(fromSel);
	EXTERN_BAGKEY(keepSel);
	EXTERN_BAGKEY(rawPict);
	EXTERN_BAGKEY(findKey);
	EXTERN_BAGKEY(compute);
	EXTERN_BAGKEY(logicOperator);
	EXTERN_BAGKEY(otherCollection);

	enum
	{
		http_bad_request = 400,
		http_perm_error = 401,
		http_not_found = 404,
		http_internal_error = 500
	};

	enum
	{
		rest_signature = 'dbmg'
	};

	const VError entity_not_found = MAKE_VERROR(rest_signature, 1800); 
	const VError unsupported_format = MAKE_VERROR(rest_signature, 1801); 
	const VError dataset_not_found = MAKE_VERROR(rest_signature, 1802); 
	const VError dataset_not_matching_entitymodel = MAKE_VERROR(rest_signature, 1803); 
	const VError cannot_build_list_of_attribute = MAKE_VERROR(rest_signature, 1804); 
	const VError cannot_build_list_of_attribute_for_expand = MAKE_VERROR(rest_signature, 1805); 
	const VError url_is_malformed = MAKE_VERROR(rest_signature, 1806); 
	const VError ampersand_instead_of_questionmark = MAKE_VERROR(rest_signature, 1807); 
	const VError expecting_closing_single_quote = MAKE_VERROR(rest_signature, 1808); 
	const VError expecting_closing_double_quote = MAKE_VERROR(rest_signature, 1809); 
	const VError wrong_list_of_attribute_to_order_by = MAKE_VERROR(rest_signature, 1810); 
	const VError unknown_rest_query_keyword = MAKE_VERROR(rest_signature, 1811); 
	const VError unknown_rest_method = MAKE_VERROR(rest_signature, 1812); 
	const VError method_not_applicable = MAKE_VERROR(rest_signature, 1813); 
	const VError uag_db_does_not_exist = MAKE_VERROR(rest_signature, 1814); 
	const VError subentityset_cannot_be_applied_here = MAKE_VERROR(rest_signature, 1815); 
	const VError empty_attribute_list = MAKE_VERROR(rest_signature, 1816); 
	const VError compute_action_does_not_exist = MAKE_VERROR(rest_signature, 1817); 
	const VError wrong_logic_operator = MAKE_VERROR(rest_signature, 1818); 
	const VError missing_other_collection_ref = MAKE_VERROR(rest_signature, 1819); 
	const VError wrong_other_collection_ref = MAKE_VERROR(rest_signature, 1820); 

};

typedef map<VString, VString> UrlQueryInfo;

typedef enum { RestMethodeRetrieve = 0, RestMethodeUpdate = 1, RestMethodeDelete = 2, RestMethodeBuildDataSet = 3, RestMethodeReleaseDataSet = 4, RestMethodeRefresh = 5, RestMethodeValidate = 6,
				RestMethodeBuildSubDataSet = 7, RestMethodeFlush = 8} RestMethodType;


class RestTools
{

	public:
		typedef enum { jsonNone = 0, jsonBeginObject, jsonEndObject, jsonBeginArray, jsonEndArray, jsonSeparator, jsonAssigne, jsonString } JsonToken;

		RestTools(BaseTaskInfo* context, const VStream* inputStream, VStream* outputStream, const VString& hostName, const VString& inURLstring, IHTTPResponse* inResponse, VJSGlobalContext* inGlobalContext, RIApplicationRef inApplicationRef)
		{
			_staticInit();
			Init(context, inputStream, outputStream, hostName, inURLstring, inResponse, inGlobalContext, inApplicationRef);
		}

		~RestTools()
		{
			DeInit();
		}

		void DeInit();

		inline void SetContext(BaseTaskInfo* context)
		{
			fContext = context;
		}

		void Init(BaseTaskInfo* context, const VStream* inputStream, VStream* outputStream, const VString& hostName, const VString& inURLstring, IHTTPResponse* inResponse, VJSGlobalContext* inGlobalContext, RIApplicationRef inApplicationRef)
		{
			fResponse = inResponse;
			url.ParseURL(inURLstring, false);
			fJSGlobalContext = inGlobalContext;
			fApplicationRef = inApplicationRef;
			fContext = context;
			if (context != nil)
			{
				labase = context->GetBase();
				if (fJSGlobalContext == nil)
					fJSGlobalContext = context->GetJSContext();
				else
					assert(fJSGlobalContext == context->GetJSContext());
			}
			else
			{
				labase = nil;
			}

#if debuglr
			if (!VDBMgr::GetManager()->IsRunning4D())
				assert(inGlobalContext != NULL);
#endif

			fInput = (VStream*)inputStream;
			fOutput = outputStream;
			fHostName = hostName;
			fOutput->SetCharSet(VTC_UTF_8 );
			fOutput->SetCarriageReturnMode(eCRM_NATIVE );
			fOutput->OpenWriting();
			fInput->OpenReading();
			fCurlevel = 0;
			fLevelHasChanged = false;
			microsoft = false;
			fPrettyFormatting = false;
			fImportAtomic = false;
			fInput->GetText(fInputString, fInput->GetSize(), true);
			fStartChar = fInputString.GetCPointer();
			fCurChar = fStartChar;
			fInputLen = fInputString.GetLength();
			fURL = &url;
			fWithQueryPath = false;
			fWithQueryPlan = false;
			fToXML = false;
			fQueryPath = nil;
			fQueryPlan = nil;
			fWithImageInfo = false;
			fUserID.SetNull(true);
			fUserSession = nil;
			fUAGDirectory = nil;
			fHTTPError = 0;

			if (context != nil)
			{
				fUAGDirectory = RetainRefCountable( context->GetBase()->GetUAGDirectory());
			}
			else if (fApplicationRef != nil)
			{
				VError lError = VE_OK;
				IDB4D_ApplicationIntf *applicationIntf = VDBMgr::GetManager()->GetApplicationInterface();
				fUAGDirectory = applicationIntf->RetainUAGDirectory( fApplicationRef, lError);
			}

			curdataset = nil;
			getDef = false;
			em = nil;
			meth = nil;
			getAllEntitiesDef = false;
			getAllEntitiesDef2 = false;
			//getdatabasedef = false;
			dejaselected = false;
			expandattributes = nil;
			onepictfield = false;
			waitforimagemime = false;
			refreshOnly = false;
			dejaerror = false;
			fProgressIndicator = nil;
			fToArray = false;
			fQueryLimit = 0;
			newwafkeepselptr = nil;
			fAllowAutoModel = false;
			selectedEntities = nil;
		}


		VError SetCurrentUser(const VString& inUserName, const VString& inPassword);
		void IncrementLevel();
		void DecrementLevel();
		VError PutTabLevel();
		VError NewLine();

		VError PutJsonPropertyName(const VString& inPropName);
		VError PutJsonPropertyString(const VString& inPropVal);

		VError PutJsonPropertyLong(sLONG inPropVal);
		VError PutJsonPropertyLong8(sLONG8 inPropVal);

		VError PutJsonPropertyValue(VValueSingle& inPropVal);
		VError PutJsonBeginObject(bool newlinefirst = true);
		VError PutJsonEndObject();
		VError PutJsonBeginArray(bool newlinefirst = true);
		VError PutJsonEndArray();
		VError PutJsonSeparator();
		VError PutJsonQueryPlan();

		VError PutJsonProperty(const VString& inPropName, sLONG inPropVal, bool withSeparator);
		VError PutJsonPropertybool(const VString& inPropName, bool inPropVal, bool withSeparator);
		VError PutJsonProperty(const VString& inPropName, const VString& inPropVal, bool withSeparator);
		VError PutJsonProperty(const VString& inPropName, const VValueBag* bag, bool withSeparator);

		VError PutDeferred(const VString& uri, bool ispict, VValueSingle* cv = nil, VString* skey = nil);
		VValueBag* PutDeferred(VError& outErr, const VString& uri, bool ispict, VValueSingle* cv = nil, VString* skey = nil);

		VError AddFormatToURI(VString& outURI, bool dejaargument);
		VError CalculateURI(VString& outURI, const EntityModel* em, const VString& additionnalPart, bool WithUri, bool withquotes);
		VError CalculateURI(VString& outURI, DataSet* inDataSet, const EntityModel* em, const VString& additionnalPart, bool WithUri, bool withquotes);
		VError CalculateURI(VString& outURI, EntityRecord* erec, const EntityAttribute* attribute, const VString& additionnalPart, bool WithUri, bool withquotes, bool withformat, bool forJSON);
		VError CalculateDataURI(VString& outURI, const EntityModel* em, const VString& additionnalPart, bool WithUri, bool withquotes);

		VError EntityRecordToJSON(EntityRecord* erec, EntityAttributeSortedSelection& whichAttributes, EntityAttributeSelection* expandAttributes, 
									EntityAttributeSortedSelection* sortingAttributes, bool withheader = false, bool nofields = false, sLONG* errToSend = nil);

		VError SelToJSON(EntityModel* em, EntityCollection* inSel, EntityAttributeSortedSelection& whichAttributes, EntityAttributeSelection* expandAttributes, 
							EntityAttributeSortedSelection* sortingAttributes, bool withheader = true, sLONG from = 0, sLONG count = -1, DataSet* inDataSet = nil, bool dejatriee = false,
							bool isAComposition = false);

		VError EntityRecordToBag(VValueBag& outBag, EntityRecord* erec, EntityAttributeSortedSelection& whichAttributes, EntityAttributeSelection* expandAttributes, 
									EntityAttributeSortedSelection* sortingAttributes, bool withheader = false, bool nofields = false, sLONG* errToSend = nil);

		VError SelToBag(VValueBag& outBag, EntityModel* em, EntityCollection* inSel, EntityAttributeSortedSelection& whichAttributes, EntityAttributeSelection* expandAttributes, 
						EntityAttributeSortedSelection* sortingAttributes, bool withheader = true, sLONG from = 0, sLONG count = -1, DataSet* inDataSet = nil, bool dejatriee = false,
						bool isAComposition = false, const VString* arrayname = nil);

		inline void SetPureJSon(bool pureJSon)
		{
			microsoft = !pureJSon;
		}

		inline void SetPrettyFormatting(bool x)
		{
			fPrettyFormatting = x;
		}

		inline bool IsPrettyFormatting() const
		{
			return fPrettyFormatting;
		}

		inline VError PutText(const VString& inText)
		{
			return fOutput->PutText(inText);
		}

		inline void SetAtomic(bool x)
		{
			fImportAtomic = x;
		}

		UniChar GetNextChar(bool& eof);

		void StringToHTML( const VString& inString, VString& processed_string );

		VError PutHTMLEntete();
		VError PutHTMLFin();
		VError PutHTMLTableBegin();
		VError PutHTMLTableEnd();
		VError PutHTMLTableRowBegin();
		VError PutHTMLTableRowEnd();
		VError PutHTMLTableColBegin();
		VError PutHTMLTableColEnd();

		VError PutHTMLString(const VString& inText);

		VError SelToHTML(EntityModel* em, EntityCollection* inSel, EntityAttributeSortedSelection& whichFields, bool withheader = true, sLONG from = 0, sLONG count = -1, DataSet* inDataSet = nil);

		bool CalculateHtmlBaseURI(VString& outUri);


		JsonToken GetNextJsonToken(VString& outString, bool* withquotes = nil);


		VError ImportEntities(EntityModel* em);
		VError ImportEntitiesSel(EntityModel* em, const VValueBag& bagData, VValueBag& bagResult, EntityAttributeValue* parent, bool firstLevel);
		VError ImportEntityRecord(EntityModel* em, const VValueBag& bagData, VValueBag& bagResult, EntityAttributeValue* parent, bool onlyone, const EntityAttribute* relDestAtt, EntityAttributeValue* relValue, bool firstLevel);

		VError DropEntities(EntityModel* em, EntityCollection* sel);


		inline void SetWithQueryPath(bool x)
		{
			fWithQueryPath = x;
		}

		inline void SetWithQueryPlan(bool x)
		{
			fWithQueryPlan = x;
		}

		inline bool WithQueryPath() const
		{
			return fWithQueryPath;
		}

		inline bool WithQueryPlan() const
		{
			return fWithQueryPlan;
		}

		inline void SetQueryPath(VValueBag* path)
		{
			CopyRefCountable(&fQueryPath, path);
		}

		inline void SetQueryPlan(VValueBag* plan)
		{
			CopyRefCountable(&fQueryPlan, plan);
		}

		inline void SetToXML(bool xml)
		{
			fToXML = xml;
		}

		inline void SetWithImageInfo(bool b)
		{
			fWithImageInfo = b;
		}

		VError ThrowError(VError err, const VString* param1 = nil, const VString* param2 = nil, const VString* param3 = nil);

		VError ThrowError(VError err, const VString& param1)
		{
			return ThrowError(err, &param1);
		}

		VError ThrowError(VError err, const VString& param1, const VString& param2)
		{
			return ThrowError(err, &param1, &param2);
		}

		VError ThrowError(VError err, const VString& param1, const VString& param2, const VString& param3)
		{
			return ThrowError(err, &param1, &param2, &param3);
		}

		void SetHTTPError(sLONG err)
		{
			fHTTPError = err;
		}

		sLONG GetHTTPError() const
		{
			return fHTTPError;
		}

		inline const VString& GetInputString() const
		{
			return fInputString;
		}

		void BuildErrorStack(VValueBag& outBag);

		void GenereErrorReport();

		void GenerateInfoIfEmpty();

		
		void ExecuteRequest();

		void InitUAG(); 

		void GetServerAndBaseInfo();

		void GetWAFFiles();

		void SetRequestFormat();
		void SetVerb();
		void AnalyseGetCatalog();
		void AnalyseGetModelAndEntities();
		void AnalyseGetAttributes();
		void AnalyseGetExpandAttributes();
		void AnalyseGetOtherStuff();
		void WorkOnCatalog();
		void WorkOnData();
		void RetrieveSelDelimiters();
		void RetrieveOrDeleteOneEntity();
		void RetrieveOrDeleteEntitySel();
		void GetDistinctValues(EntityCollection* sel);
		void FindKeyInCollection(const VString& keyval, EntityCollection* sel);
		void ComputeOnCollection(const VString& computeval, EntityCollection* sel);
		bool MixCollection(EntityCollection* sel, const VString& logicOper, EntityCollection*& outSel);

		void ExecuteStaticRestMethod(const VString& JSNameSpace);

		void CallMethod(EntityCollection* sel, EntityRecord* erec, DataSet* inDataSet);
		void ReturnJSResult(VError err, VJSValue& result, JS4D::ExceptionRef excep, DataSet* inDataSet);

		VDB4DProgressIndicator* GetProgressIndicator();



		static bool IsValidRestKeyword(const VString& keyword)
		{
			return sAllRestKeywords.AttributeExists(keyword);
		}

		static bool IsValidRestKeyword(const VValueBag::StKey& keyword)
		{
			return sAllRestKeywords.AttributeExists(keyword);
		}

		static void _staticInit();

		inline void AllowAutoModel()
		{
			fAllowAutoModel = true;
		}

	protected :

		VFullURL* fURL;
		BaseTaskInfo* fContext;
		VJSGlobalContext* fJSGlobalContext;
		RIApplicationRef fApplicationRef;
		sLONG fCurlevel;
		VString fTabString;
		VStream* fOutput;
		VStream* fInput;
		VString fInputString;
		VString fHostName;
		sLONG fInputLen;
		const UniChar* fCurChar;
		const UniChar* fStartChar;
		bool fLevelHasChanged, microsoft, fPrettyFormatting, fImportAtomic, fImportNotFullySuccessfull;
		bool fWithQueryPath, fWithQueryPlan, fToXML, fWithImageInfo;
		VValueBag* fQueryPath;
		VValueBag* fQueryPlan;
		sLONG fHTTPError;

		VString fUserName;
		VString fPassword;
		VUUID fUserID;
		CUAGSession* fUserSession;
		CUAGDirectory* fUAGDirectory;
		IHTTPResponse* fResponse;

		sLONG countelem;
		sLONG skipfirst;
		VError err;

		VFullURL url;
		RestMethodType restmethod;
		VString restmethodName;
		bool toJSON;
		bool onepictfield;
		bool waitforimagemime;
		bool getDef;
		bool getAllEntitiesDef, getAllEntitiesDef2/*, getdatabasedef*/;
		bool dejaselected, refreshOnly;
		bool fToArray;
		bool fAllowAutoModel;
		VString tablename;
		VString fEntityKey;
		sLONG fQueryLimit;
		const VString* s;
		VString imageformatstring;
		DataSet* curdataset;
		EntityModel* em;
		EntityMethod* meth;
		VectorOfVString methParams;
		VString fJsonMethParams;
		vector<CDB4DEntityModel*> allEMs;
		EntityCollection* selectedEntities;
		EntityAttributeSelection* expandattributes;
		EntityAttributeSortedSelection attributes;
		VString fSavedQuery;
		VUUID fEntitySetID;
		VProgressIndicator* fProgressIndicator;

		WafSelection* newwafkeepselptr;



		CDB4DBaseContext* xUAGcontext;
		CDB4DBase* xUAGBase;
		Base4D* labase;

		bool wasapost, wasaput, wasadelete, dejaerror;

		static bool sIsInited;
		static VValueBag sAllRestKeywords;



};



#endif

