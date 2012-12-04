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
#ifndef __ENTITYMODEL__
#define __ENTITYMODEL__


const VErrorDB4D	VE_DB4D_ENTITY_ATTRIBUTE_NOT_FOUND	= MAKE_VERROR(CDB4DManager::Component_Type, 1500);
const VErrorDB4D	VE_DB4D_ENTITY_NAME_ALREADY_EXIST	= MAKE_VERROR(CDB4DManager::Component_Type, 1501);
const VErrorDB4D	VE_DB4D_ENTITY_NAME_MISSING			= MAKE_VERROR(CDB4DManager::Component_Type, 1502);
const VErrorDB4D	VE_DB4D_ENTITY_WRONG_TABLE_REF		= MAKE_VERROR(CDB4DManager::Component_Type, 1503);
const VErrorDB4D	VE_DB4D_ENTITY_ATTRIBUTE_NAME_MISSING = MAKE_VERROR(CDB4DManager::Component_Type, 1504);
const VErrorDB4D	VE_DB4D_ENTITY_ATTRIBUTE_TYPE_MISSING = MAKE_VERROR(CDB4DManager::Component_Type, 1505);
const VErrorDB4D	VE_DB4D_ENTITY_ATTRIBUTE_IS_MALFORMED = MAKE_VERROR(CDB4DManager::Component_Type, 1506);
const VErrorDB4D	VE_DB4D_ENTITY_NOT_FOUND_FOR_EXTENDS	= MAKE_VERROR(CDB4DManager::Component_Type, 1507);
const VErrorDB4D	VE_DB4D_ENTITY_ATTRIBUTE_IS_NOT_A_VVALUE = MAKE_VERROR(CDB4DManager::Component_Type, 1508);
const VErrorDB4D	VE_DB4D_ENTITY_RELATION_IS_MALFORMED = MAKE_VERROR(CDB4DManager::Component_Type, 1509);

const VErrorDB4D	VE_DB4D_ENTITY_RELATION_ALREADY_EXIST	= MAKE_VERROR(CDB4DManager::Component_Type, 1510);
const VErrorDB4D	VE_DB4D_ENTITY_RELATION_DOES_NOT_EXIST	= MAKE_VERROR(CDB4DManager::Component_Type, 1511);

const VErrorDB4D	VE_DB4D_TYPE_ALREADY_EXIST	= MAKE_VERROR(CDB4DManager::Component_Type, 1512);
const VErrorDB4D	VE_DB4D_TYPE_DOES_NOT_EXIST	= MAKE_VERROR(CDB4DManager::Component_Type, 1513);

const VErrorDB4D	VE_DB4D_ENUMERATION_ALREADY_EXIST	= MAKE_VERROR(CDB4DManager::Component_Type, 1514);
const VErrorDB4D	VE_DB4D_ENUMERATION_DOES_NOT_EXIST	= MAKE_VERROR(CDB4DManager::Component_Type, 1515);

const VErrorDB4D	VE_DB4D_ENTITY_ATTRIBUTE_IS_READ_ONLY = MAKE_VERROR(CDB4DManager::Component_Type, 1516);
const VErrorDB4D	VE_DB4D_ENTITY_RECORD_CANNOT_BE_SAVED = MAKE_VERROR(CDB4DManager::Component_Type, 1517);

const VErrorDB4D	VE_DB4D_CANNOT_BUILD_ENUM_FROM_DEF = MAKE_VERROR(CDB4DManager::Component_Type, 1519);
const VErrorDB4D	VE_DB4D_CANNOT_BUILD_TYPE_FROM_DEF = MAKE_VERROR(CDB4DManager::Component_Type, 1520);
const VErrorDB4D	VE_DB4D_CANNOT_BUILD_EM_FROM_DEF = MAKE_VERROR(CDB4DManager::Component_Type, 1521);
const VErrorDB4D	VE_DB4D_CANNOT_BUILD_EM_ATT_FROM_DEF = MAKE_VERROR(CDB4DManager::Component_Type, 1522);
const VErrorDB4D	VE_DB4D_CANNOT_BUILD_EM_RELATION_FROM_DEF = MAKE_VERROR(CDB4DManager::Component_Type, 1523);

const VErrorDB4D	VE_DB4D_ENTITY_RELATION_SOURCE_IS_MISSING = MAKE_VERROR(CDB4DManager::Component_Type, 1524);
const VErrorDB4D	VE_DB4D_ENTITY_RELATION_DEST_IS_MISSING = MAKE_VERROR(CDB4DManager::Component_Type, 1525);
const VErrorDB4D	VE_DB4D_ENTITY_RELATION_KIND_IS_MISSING = MAKE_VERROR(CDB4DManager::Component_Type, 1526);

const VErrorDB4D	VE_DB4D_ENTITY_RELATION_IS_MISSING = MAKE_VERROR(CDB4DManager::Component_Type, 1527);
const VErrorDB4D	VE_DB4D_ENTITY_RELATION_IS_EMPTY = MAKE_VERROR(CDB4DManager::Component_Type, 1528);
const VErrorDB4D	VE_DB4D_ATT_FIELD_IS_MISSING = MAKE_VERROR(CDB4DManager::Component_Type, 1529);
const VErrorDB4D	VE_DB4D_ATT_RELATED_ENTITY_IS_MISSING = MAKE_VERROR(CDB4DManager::Component_Type, 1530);
const VErrorDB4D	VE_DB4D_ATT_RELATED_ENTITY_NOT_FOUND = MAKE_VERROR(CDB4DManager::Component_Type, 1531);

const VErrorDB4D	VE_DB4D_CANNOT_LOAD_ENTITY_CATALOG = MAKE_VERROR(CDB4DManager::Component_Type, 1532);
const VErrorDB4D	VE_DB4D_ATT_FIELD_NOT_FOUND = MAKE_VERROR(CDB4DManager::Component_Type, 1533);

const VErrorDB4D	VE_DB4D_NEW_ENTITY_RECORD_CANNOT_BE_SAVED = MAKE_VERROR(CDB4DManager::Component_Type, 1534);
const VErrorDB4D	VE_DB4D_RELATION_PATH_IS_MULTISEGMENT = MAKE_VERROR(CDB4DManager::Component_Type, 1535);

const VErrorDB4D	VE_DB4D_RELATED_ENTITY_DOES_NOT_BELONG_TO_MODEL = MAKE_VERROR(CDB4DManager::Component_Type, 1536);
const VErrorDB4D	VE_DB4D_CANNOT_SAVE_RELATED_ENTITY = MAKE_VERROR(CDB4DManager::Component_Type, 1537);

const VErrorDB4D	VE_DB4D_WRONG_ATTRIBUTE_TYPE_FOR_IDENT = MAKE_VERROR(CDB4DManager::Component_Type, 1538);
const VErrorDB4D	VE_DB4D_WRONG_ATTRIBUTE_TYPE_FOR_PRIMKEY = MAKE_VERROR(CDB4DManager::Component_Type, 1539);

const VErrorDB4D	VE_DB4D_ENTITY_HAS_NO_PRIMKEY = MAKE_VERROR(CDB4DManager::Component_Type, 1540);
const VErrorDB4D	VE_DB4D_PRIMKEY_MALFORMED = MAKE_VERROR(CDB4DManager::Component_Type, 1541);
const VErrorDB4D	VE_DB4D_PRIMKEY_NOT_FOUND = MAKE_VERROR(CDB4DManager::Component_Type, 1542);

const VErrorDB4D	VE_DB4D_CANNOT_LOAD_ENTITY = MAKE_VERROR(CDB4DManager::Component_Type, 1543);
const VErrorDB4D	VE_DB4D_PRIMKEY_IS_NULL = MAKE_VERROR(CDB4DManager::Component_Type, 1544);
const VErrorDB4D	VE_DB4D_PRIMKEY_IS_NOT_UNIQUE = MAKE_VERROR(CDB4DManager::Component_Type, 1545);
const VErrorDB4D	VE_DB4D_PRIMKEY_IS_NULL_2 = MAKE_VERROR(CDB4DManager::Component_Type, 1546);

const VErrorDB4D	VE_DB4D_ENTITY_HAS_NO_IDENTKEY = MAKE_VERROR(CDB4DManager::Component_Type, 1547);
const VErrorDB4D	VE_DB4D_IDENTKEY_MALFORMED = MAKE_VERROR(CDB4DManager::Component_Type, 1548);
const VErrorDB4D	VE_DB4D_IDENTKEY_NOT_FOUND = MAKE_VERROR(CDB4DManager::Component_Type, 1549);

const VErrorDB4D	VE_DB4D_SCRIPT_STATEMENT_IS_EMPTY = MAKE_VERROR(CDB4DManager::Component_Type, 1550);
const VErrorDB4D	VE_DB4D_SCRIPT_KIND_IS_MISSING = MAKE_VERROR(CDB4DManager::Component_Type, 1551);
const VErrorDB4D	VE_DB4D_SCRIPT_KIND_IS_UNKNOWN = MAKE_VERROR(CDB4DManager::Component_Type, 1552);

const VErrorDB4D	VE_DB4D_WRONG_PERM_REF = MAKE_VERROR(CDB4DManager::Component_Type, 1553);
const VErrorDB4D	VE_DB4D_ENTITY_CANNOT_BE_FOUND = MAKE_VERROR(CDB4DManager::Component_Type, 1554);

const VErrorDB4D	VE_DB4D_NO_UAGDIRECTORY = MAKE_VERROR(CDB4DManager::Component_Type, 1555);
const VErrorDB4D	VE_DB4D_CANNOT_LOAD_ENTITIES_PERMS = MAKE_VERROR(CDB4DManager::Component_Type, 1556);

const VErrorDB4D	VE_DB4D_NO_PERM_TO_READ = MAKE_VERROR(CDB4DManager::Component_Type, 1557);
const VErrorDB4D	VE_DB4D_NO_PERM_TO_UPDATE = MAKE_VERROR(CDB4DManager::Component_Type, 1558);
const VErrorDB4D	VE_DB4D_NO_PERM_TO_CREATE = MAKE_VERROR(CDB4DManager::Component_Type, 1559);
const VErrorDB4D	VE_DB4D_NO_PERM_TO_DELETE = MAKE_VERROR(CDB4DManager::Component_Type, 1560);
const VErrorDB4D	VE_DB4D_NO_PERM_TO_EXECUTE = MAKE_VERROR(CDB4DManager::Component_Type, 1561);

const VErrorDB4D	VE_DB4D_JS_ERR = MAKE_VERROR(CDB4DManager::Component_Type, 1562);

const VErrorDB4D	VE_DB4D_ENTITY_METHOD_NAME_MISSING = MAKE_VERROR(CDB4DManager::Component_Type, 1563);
const VErrorDB4D	VE_DB4D_ENTITY_METHOD_TYPE_INVALID = MAKE_VERROR(CDB4DManager::Component_Type, 1564);
const VErrorDB4D	VE_DB4D_CANNOT_BUILD_EM_METH_FROM_DEF = MAKE_VERROR(CDB4DManager::Component_Type, 1565);
const VErrorDB4D	VE_DB4D_METHOD_STATEMENT_IS_EMPTY = MAKE_VERROR(CDB4DManager::Component_Type, 1566);
const VErrorDB4D	VE_DB4D_METHOD_PARAMETER_NAME_IS_INVALID = MAKE_VERROR(CDB4DManager::Component_Type, 1567);

const VErrorDB4D	VE_DB4D_ENTITY_VALUE_LESS_THAN_MIN = MAKE_VERROR(CDB4DManager::Component_Type, 1568);
const VErrorDB4D	VE_DB4D_ENTITY_VALUE_MORE_THAN_MAX = MAKE_VERROR(CDB4DManager::Component_Type, 1569);
const VErrorDB4D	VE_DB4D_ENTITY_RECORD_FAILS_VALIDATION = MAKE_VERROR(CDB4DManager::Component_Type, 1570);
const VErrorDB4D	VE_DB4D_ENTITY_VALUE_DOES_NOT_MATCH_PATTERN = MAKE_VERROR(CDB4DManager::Component_Type, 1571);

const VErrorDB4D	VE_DB4D_ENTITY_TABLENAME_DUPLICATED		= MAKE_VERROR(CDB4DManager::Component_Type, 1572);

const VErrorDB4D	VE_DB4D_ENTITY_METHOD_NAME_UNKNOWN		= MAKE_VERROR(CDB4DManager::Component_Type, 1573);

const VErrorDB4D	VE_DB4D_WRONG_ATTRIBUTE_KIND		= MAKE_VERROR(CDB4DManager::Component_Type, 1574);

const VErrorDB4D	VE_DB4D_PRIMKEY_MUST_BE_ONE_FIELD_ONLY		= MAKE_VERROR(CDB4DManager::Component_Type, 1575);
const VErrorDB4D	VE_DB4D_NAVIGATION_ATTRIBUTE_MUST_BE_Nto1	= MAKE_VERROR(CDB4DManager::Component_Type, 1576);
const VErrorDB4D	VE_DB4D_NAVIGATION_ATTRIBUTE_MUST_BE_1toN	= MAKE_VERROR(CDB4DManager::Component_Type, 1577);

const VErrorDB4D	VE_DB4D_ENTITY_ATTRIBUTE_IS_NOT_NAVIGATION = MAKE_VERROR(CDB4DManager::Component_Type, 1578);
const VErrorDB4D	VE_DB4D_NAVIGATION_PATH_IS_MALFORMED = MAKE_VERROR(CDB4DManager::Component_Type, 1579);
const VErrorDB4D	VE_DB4D_NAVIGATION_PATH_IS_EMPTY = MAKE_VERROR(CDB4DManager::Component_Type, 1580);

const VErrorDB4D	VE_DB4D_CANNOT_RESOLVE_NAVIGATION_PATH = MAKE_VERROR(CDB4DManager::Component_Type, 1581);

const VErrorDB4D	VE_DB4D_MISSING_OR_INVALID_EVENTKIND = MAKE_VERROR(CDB4DManager::Component_Type, 1582);
const VErrorDB4D	VE_DB4D_MISSING_OR_INVALID_EVENT_METHOD = MAKE_VERROR(CDB4DManager::Component_Type, 1583);

const VErrorDB4D	VE_DB4D_SCALAR_NEEDED_FOR_FLATTENED = MAKE_VERROR(CDB4DManager::Component_Type, 1584);

const VErrorDB4D	VE_DB4D_JSTEXT_ERR = MAKE_VERROR(CDB4DManager::Component_Type, 1585);

const VErrorDB4D	VE_DB4D_NOT_AN_ENTITY_COLLECTION = MAKE_VERROR(CDB4DManager::Component_Type, 1586);

const VErrorDB4D	VE_DB4D_COLLECTION_ON_INCOMPATIBLE_DATACLASSES = MAKE_VERROR(CDB4DManager::Component_Type, 1587);

const VErrorDB4D	VE_DB4D_ENTITY_ATTRIBUTE_ALREADY_EXISTS = MAKE_VERROR(CDB4DManager::Component_Type, 1588);

const VErrorDB4D	VE_DB4D_ENTITY_ATTRIBUTE_IS_FROM_ANOTHER_DATACLASS = MAKE_VERROR(CDB4DManager::Component_Type, 1589);

const VErrorDB4D	VE_DB4D_RELATION_PATH_IS_RECURSIVE = MAKE_VERROR(CDB4DManager::Component_Type, 1590);

const VErrorDB4D	VE_DB4D_ENTITY_STRING_LESS_THAN_MIN = MAKE_VERROR(CDB4DManager::Component_Type, 1591);
const VErrorDB4D	VE_DB4D_ENTITY_STRING_GREATER_THAN_MAX = MAKE_VERROR(CDB4DManager::Component_Type, 1592);
const VErrorDB4D	VE_DB4D_ENTITY_STRING_LENGTH_EQUAL = MAKE_VERROR(CDB4DManager::Component_Type, 1593);


const sLONG kDefaultTopSize = 100;

const UniChar kEntityTablePrefixChar = '$';
const VString kEntityTablePrefixString = L"$";

const ArrayOfConstUniCharPtr xEatt_types = { L"storage", L"alias", L"calculated", L"relatedEntity", L"relatedEntities", L"removed", L"altered", /*L"field",*/ L"composition", L"" };
const Enumeration EattTypes(xEatt_types);

const ArrayOfConstUniCharPtr xScript_types = { L"db4d", L"javascript", L"4d", L"" };
const Enumeration ScriptTypes(xScript_types);
typedef enum { script_none, script_db4d, script_javascript, script_4d } script_type;

const ArrayOfConstUniCharPtr xERelTypes = { L"manyToOne", L"oneToMany", L"" };
const Enumeration ERelTypes(xERelTypes);

const ArrayOfConstUniCharPtr xEmeth_types = { L"dataClass", L"entity", L"entityCollection", L"" };
const Enumeration EmethTypes(xEmeth_types);

const ArrayOfConstUniCharPtr xEValPredefinedTypes = 
{ 
	L"bool", 
	L"byte", 
	L"word", 
	L"long", 
	L"long64", 
	L"number", 
	L"float", 
	L"date", 
	L"duration", 
	L"string", 
	L"VK_BLOB", 
	L"image",
	L"uuid", 
	L"VK_TEXT", 
	L"VK_SUBTABLE", 
	L"VK_SUBTABLE_KEY", 
	L"VK_OBSOLETE_STRING_DB4D", 
	L"blob", // VK_BLOB_DB4D
	L"VK_STRING_UTF8", 
	L"VK_TEXT_UTF8", 
	L"" 
};
const Enumeration EValPredefinedTypes(xEValPredefinedTypes);

const ArrayOfConstUniCharPtr xIndexKinds = 
{
	L"btree",
	L"hash",
	L"cluster",
	L"unused4",
	L"unused5",
	L"unused6",
	L"auto",
	L"keywords",
	L""
};

const Enumeration EIndexKinds(xIndexKinds);


const ArrayOfConstUniCharPtr xScopeKinds = 
{
	L"public",
	L"publicOnServer",
	L"protected",
	L"private",
	L""
};
const Enumeration EScopeKinds(xScopeKinds);


const ArrayOfConstUniCharPtr xPermResourceType = 
{
	L"model",
	L"dataClass",
	L"method",
	L""
};

const Enumeration EPermResourceType(xPermResourceType);

typedef enum 
{ 
	perm_none = 0,
	perm_model,
	perm_dataClass, 
	perm_method
} PermResourceType;



const ArrayOfConstUniCharPtr xPermAction = 
{
	L"read",
	L"create",
	L"update",
	L"remove",
	L"execute",
	L"promote",
	L""
};

const Enumeration EPermAction(xPermAction);
/*
typedef enum 
{ 
	permaction_none = 0,
	permaction_read,
	permaction_create, 
	permaction_update,
	permaction_delete,
	permaction_execute,
	permaction_promote
} PermAction;
*/

/*
const ArrayOfConstUniCharPtr xOldDBEventKinds = 
{
	L"save",
	L"load",
	L"init",
	L"remove",
	L"validate",
	L"set",
	L"get",
	L""
};

const Enumeration oldEDBEventKinds(xOldDBEventKinds);
*/

const ArrayOfConstUniCharPtr xDBEventKinds = 
{
	L"onSave",
	L"onLoad",
	L"onInit",
	L"onRemove",
	L"onValidate",
	L"onRestrictingQuery",
	L"onSet",
	L"onGet",
	L""
};

const Enumeration EDBEventKinds(xDBEventKinds);

typedef enum 
{ 
	dbev_none = 0,
	dbev_save,
	dbev_load, 
	dbev_init, 
	dbev_remove, 
	dbev_validate,
	dbev_restrict,
	dbev_set,
	dbev_get,
	dbev_firstEvent = dbev_save,
	dbev_lastEvent = dbev_get
} DBEventKind;



const ArrayOfConstUniCharPtr ecat_correspondance = 
{
	L"entityModel", L"entityModels",
	L"type", L"types",
	L"relationship", L"relationships",
	L"", L""
};

const Correspondance CatCorres(ecat_correspondance);


class DataSet : public IRefCountable
{
	public:

		DataSet(Table* inTable, Selection* inSel, uLONG timeout = 0)
		{
			fTable = RetainRefCountable(inTable);
			fSel = RetainRefCountable(inSel);
			fTimeout = timeout;
			fID.Regenerate();
			fStartingTime = VSystem::GetCurrentTime();
		}

		inline void SetID(const VUUID& xid)
		{
			fID = xid;
		}

		inline VUUID& GetID()
		{
			return fID;
		}

		inline const VUUID& GetID() const
		{
			return fID;
		}

		inline Selection* GetSel() const
		{
			return fSel;
		}

		inline Table* GetTable() const
		{
			return fTable;
		}

		inline void ResetTimer(uLONG curtime)
		{
			fStartingTime = curtime;
		}

		inline bool Expired(uLONG curtime) const
		{
			if (fTimeout == 0)
				return false;
			else
			{
				uLONG delay;
				if (curtime > fStartingTime)
					delay = curtime - fStartingTime;
				else
					delay = fStartingTime - curtime;
				return delay > fTimeout;
			}
		}

		inline uLONG GetExpirationTime() const
		{
			return fStartingTime + fTimeout;
		}

		void GetInfo(VValueBag& outBag)
		{
			VString tablename;
			fTable->GetName(tablename);
			outBag.SetVUUID(L"id", fID);
			outBag.SetString(L"tableName", tablename);
			outBag.SetLong("selectionSize", fSel->GetQTfic());
			bool sorted = true;
			if (fSel->GetTypSel() == sel_bitsel)
				sorted = false;
			outBag.SetBool(L"sorted", sorted);
			uLONG depart = VSystem::GetCurrentTime()-fStartingTime;
			VTime ts;
			ts.FromSystemTime();
			VDuration sdur(depart);
			ts.Substract(sdur);
			outBag.SetTime(L"refreshed", ts);
			VDuration dur(fTimeout);
			ts.Add(dur);
			outBag.SetTime(L"expires", ts);
		}


	protected:
		virtual ~DataSet();

		Selection* fSel;
		Table* fTable;
		VUUID fID;
		uLONG fTimeout;
		uLONG fStartingTime;
};


									// ---------------------------------------------


class EntityModelCatalog;
class RestTools;


class EmEnum : public IRefCountable
{
	public:

		inline EmEnum(Base4D* owner)
		{
			fOwner = owner;
			fExtraProperties = nil;
		}

		inline const VString& GetName() const
		{
			return fName;
		}

		inline void ClearName()
		{
			fName.Clear();
		}

		inline Base4D* GetOwner() const
		{
			return fOwner;
		}

		VError ThrowError( VError inErrCode, const VString* p1 = nil) const;

		VError FromBag(const VValueBag* bag, EntityModelCatalog* catalog, bool devMode);
		VError ToBag(VValueBag& outBag, bool forDax, bool forSave, bool forJSON, bool firstlevel) const;

		bool GetValue(const VString& inName, sLONG& outValue) const;
		bool GetValueName(sLONG inValue, VString& outName) const;

	protected:
		virtual ~EmEnum()
		{
			QuickReleaseRefCountable(fExtraProperties);
		}

		typedef map<VString, sLONG> EnumMap;
		typedef multimap<sLONG, VString> EnumIDMap;

		Base4D* fOwner;
		VString fName;
		EnumMap fEnums;
		EnumIDMap fEnumsID;
		const VValueBag* fExtraProperties;
		

};


									// ---------------------------------------------


class AttributeType : public IRefCountable
{
	public:
		typedef list<AttributeType*> ListOfTypes;

		AttributeType(Base4D* owner);
		~AttributeType();

		VError ThrowError( VError inErrCode, const VString* p1 = nil) const;

		inline const VString& GetName() const
		{
			return fName;
		}

		inline AttributeType* GetBaseType() const
		{
			return fFrom;
		}

		inline Base4D* GetOwner() const
		{
			return fOwner;
		}

		VError FromBag(const VValueBag* bag, EntityModelCatalog* catalog, bool fullyLoad, bool devMode);
		VError ToBag(VValueBag& outBag, bool forDax, bool forSave, bool forJSON, bool firstlevel) const;
		VError SaveToBag(VValueBag& CatalogBag, bool forJSON) const;

		bool IsEmpty() const;
		void ReCalc();
		void CopyFrom(const AttributeType* from);
		void Clear();
		void ExtendsFrom(AttributeType* extendfrom);

		inline AttributeType* Clone() const
		{
			AttributeType* result = new AttributeType(fOwner);
			if (result != nil)
			{
				result->CopyFrom(this);
			}
			return result;
		}

		inline const VValueSingle* GetMin() const
		{
			if (fResultType == nil)
				return fMin;
			else
				return fResultType->GetMin();
		}

		inline const VValueSingle* GetMax() const
		{
			if (fResultType == nil)
				return fMax;
			else
				return fResultType->GetMax();
		}

		inline const VValueSingle* GetDefaultValue() const
		{
			if (fResultType == nil)
				return fDefaultValue;
			else
				return fResultType->GetDefaultValue();
		}

		inline const VString& GetPattern() const
		{
			if (fResultType == nil)
				return fPattern;
			else
				return fResultType->GetPattern();
		}

		inline const VRegexMatcher* GetPatternReg() const
		{
			if (fRegexPattern == nil && !fPattern.IsEmpty())
			{
				fRegexPattern = VRegexMatcher::Create(fPattern, nil);
			}
			return fRegexPattern;
		}

		inline const VString& GetFormat() const
		{
			if (fResultType == nil)
				return fFormat;
			else
				return fResultType->GetFormat();
		}

		inline sLONG GetFixedLength() const
		{
			if (fResultType == nil)
				return fFixedLength;
			else
				return fResultType->GetFixedLength();
		}

		inline sLONG GetMaxLength() const
		{
			if (fResultType == nil)
				return fMaxLength;
			else
				return fResultType->GetMaxLength();
		}

		inline sLONG GetMinLength() const
		{
			if (fResultType == nil)
				return fMinLength;
			else
				return fResultType->GetMinLength();
		}

		inline const EmEnum* GetEnumeration() const
		{
			if (fResultType == nil)
				return fEnumeration;
			else
				return fResultType->GetEnumeration();
		}

		sLONG ComputeScalarType() const
		{
			if (fScalarType == 0)
			{
				if (fFrom == nil)
					return 0;
				else
					return fFrom->ComputeScalarType();
			}
			else
				return fScalarType;
		}

		bool NeedValidation() const;



	protected:
		Base4D* fOwner;
		VString fName;
		sLONG fScalarType;
		VString fExtends;
		AttributeType* fFrom;
		ListOfTypes fDerivateds;

		VString fPattern;
		sLONG fFixedLength;
		sLONG fMaxLength;
		sLONG fMinLength;
		VValueSingle* fMin;
		VValueSingle* fMax;
		VValueSingle* fDefaultValue;
		VString fFormat;
		VString fPresentation;
		VString fLocale;
		VString fSliderMin;
		VString fSliderMax;
		VString fSliderInc;
		EmEnum* fEnumeration;
		const VValueBag* fExtraProperties;
		uBOOL fAutoComplete;

		AttributeType* fResultType;
		mutable VRegexMatcher* fRegexPattern;
};


									// ---------------------------------------------

typedef vector<EntityRelation*> EntityRelationCollection;


class EntityRelation : public IRefCountable
{
	public:
		inline EntityRelation(Field* source, Field* dest, EntityRelationKind kind)
		{
			fSource = source;
			fDest = dest;
			fType = kind;
			assert(fSource != nil);
			assert(fDest != nil);
		}

		inline EntityRelation()
		{
			fSource = nil;
			fDest = nil;
			fType = erel_none;
		}

		inline EntityRelation(const EntityRelationCollection& path, bool nTo1);

		VError ThrowError( VError inErrCode, const VString* p1 = nil) const;

		inline bool operator == (const EntityRelation& other) const
		{
			return (fSource == other.fSource) && (fDest == other.fDest) && (fType == other.fType);
		}

		inline const VString& GetName() const
		{
			return fName;
		}

		inline Table* GetSourceTable() const
		{
			return fSource->GetOwner();
		}

		inline Table* GetDestTable() const
		{
			return fDest->GetOwner();
		}


		inline Field* GetSourceField() const
		{
			return fSource;
		}

		inline Field* GetDestField() const
		{
			return fDest;
		}


		inline EntityRelationKind GetKind() const
		{
			return fType;
		}

		/*
		VError FromBag(const VValueBag* bag, Base4D* owner, EntityModelCatalog* catalog);
		VError FromBagArray(const VBagArray* bagarray, Base4D* owner, EntityModelCatalog* catalog);
		VError ToBag(VValueBag& outBag, bool forDax, bool forSave, bool forJSON, bool firstlevel) const;
		*/

		inline const EntityRelationCollection& GetPath() const
		{
			return fPath;
		}

		bool IsEmpty() const
		{
			return fPath.empty();
		}

		bool IsSimple() const
		{
			return fPath.size() == 1;
		}

		void RecalcPath();

		bool MatchesBeginingOf(const EntityRelation* otherRel) const;

		//VError ResolveMissingTables(EntityModelCatalog* catalog);


	protected:

		virtual ~EntityRelation()
		{
			for (EntityRelationCollection::iterator cur = fSubPath.begin(), end = fSubPath.end(); cur != end; cur++)
				(*cur)->Release();
		}

		VString fName;
		Field* fSource;
		Field* fDest;
		EntityRelationKind fType;
		EntityRelationCollection fSubPath;
		EntityRelationCollection fPath;
		VString fMissingSourceTable, fMissingSourceField, fMissingDestTable, fMissingDestField;
};



class EntityModel;
class OptimizedQuery;
class EntityRecord;



									// ---------------------------------------------


class EntityMethod : public VComponentImp<CDB4DEntityMethod>
{
	public:

		EntityMethod(EntityModel* owner);

		VError ThrowError( VError inErrCode, const VString* p1 = nil) const;

		inline EntityModel* GetOwner() const
		{
			return fOwner;
		}

		virtual CDB4DEntityModel* GetModel() const;

		inline EntityMethodKind GetKind() const
		{
			return fKind;
		}

		inline void SetKind(EntityMethodKind kind)
		{
			fKind = kind;
		}

		virtual EntityMethodKind GetMethodKind() const
		{
			return fKind;
		}

		inline const VString& GetName() const
		{
			return fName;
		}

		VError FromBag(const VValueBag* bag, EntityModelCatalog* catalog, bool devMode);

		VError FromJS(const VString& name, EntityMethodKind kind, const VString& from, EntityAttributeScope scope, EntityModelCatalog* catalog, bool devMode);

		VError ToBag(VValueBag& outBag, bool forDax, bool forSave, bool forJSON) const;

		EntityMethod* Clone(EntityModel* inModel) const;

		bool GetScriptObjFunc(VJSGlobalContext* jsglobcontext, BaseTaskInfo* context, VJSObject& outObjFunc) const;

		VError Execute(Selection* inSel, const vector<VJSValue>* inParams, BaseTaskInfo* context, VJSValue& outResult) const;

		VError Execute(EntityRecord* inRec, const vector<VJSValue>* inParams, BaseTaskInfo* context, VJSValue& outResult) const;

		VError Execute(const vector<VJSValue>* inParams, BaseTaskInfo* context, VJSValue& outResult) const;

		VJSObject* getFuncObject(BaseTaskInfo* context, VJSObject& outObjFunc) const;

		virtual VJSObject* GetFuncObject(CDB4DBaseContext* inContext, VJSObject& outObjFunc) const;

		bool IsOverWritten() const
		{
			return fOverWrite;
		};

		void SetOverWrite(bool b)
		{
			fOverWrite = b;
		}

		VError ResolveType(EntityModelCatalog* catalog, bool devMode);

		virtual VError SetPermission(DB4D_EM_Perm inPerm, const VUUID& inGroupID);
		virtual VError GetPermission(DB4D_EM_Perm inPerm, VUUID& outGroupID) const;
		virtual bool PermissionMatch(DB4D_EM_Perm inPerm, CUAGSession* inSession) const;
		bool permissionMatch(DB4D_EM_Perm inPerm, CUAGSession* inSession) const;


		const EntityAttributeScope getScope() const
		{
			return fScope;
		}

		virtual const EntityAttributeScope GetScope() const
		{
			return fScope;
		}

		void ResolvePermissionsInheritance();

	protected:
		VString fName;
		EntityMethodKind fKind;
		EntityAttributeScope fScope;
		EntityModel* fOwner;
		VString fStatement;
		VString fFrom;
		VString fReturnTypeString;
		VectorOfVString fParams;
		EntityModel* fReturnTypeModel;
		sLONG fReturnTypeScalar;
		VUUID fExecutePerm;
		VUUID fPromotePerm;
		mutable sLONG fScriptNum;
		bool fOverWrite, fReturnTypeIsSelection, fUserDefined;
};

									// ---------------------------------------------




class DBEvent
{
	public:
		DBEvent();
		//~DBEvent();

		VError FromBag(EntityModel* inOwner, const VValueBag* bag, EntityModelCatalog* catalog, bool devMode);
		VError ToBag(VValueBag& outBag, bool forDax, bool forSave, bool forJSON, bool firstlevel) const;

		VError SetEvent(EntityModel* inOwner, DBEventKind eventKind, const VString& from)
		{
			fOwner = inOwner;
			fKind = eventKind;
			fFrom = from;
			fUserDefined = true;
			return VE_OK;
		}

		bool IsOverWritten() const
		{
			return fOverWrite;
		}

		bool IsValid() const
		{
			return fOwner != nil;
		}

		VError CopyFrom(EntityModel* inOwner, const DBEvent* from);

		VError Call(EntityRecord* inRec, BaseTaskInfo* context, const EntityAttribute* inAtt, const EntityModel* inDataClass, Selection* *outSel = nil) const;


	protected:
		DBEventKind fKind;
		EntityModel* fOwner;
		VString fFrom;
		bool fOverWrite, fUserDefined;

};


typedef DBEvent* DBEventIterator;
typedef const DBEvent* DBEventConstIterator;




									// ---------------------------------------------


class EntityModel;

typedef pair<EntityModel*, VString> SubPathRef;
typedef set<SubPathRef> SubPathRefSet;



typedef enum { script_attr_get = 0, script_attr_set, script_attr_query, script_attr_sort, script_attr_last = script_attr_sort, script_attr_none } script_attr;

class EntityAttribute : public VComponentImp<CDB4DEntityAttribute>
{
	public:

		EntityAttribute(EntityModel* owner);

		virtual ~EntityAttribute();

		VError ThrowError( VError inErrCode, const VString* p1 = nil) const;

		inline EntityModel* GetOwner() const
		{
			return fOwner;
		}

		virtual CDB4DEntityModel* GetModel() const;

		inline EntityAttributeKind GetKind() const
		{
			return fKind;
		}

		inline void SetKind(EntityAttributeKind kind)
		{
			fKind = kind;
		}

		virtual EntityAttributeKind GetAttributeKind() const
		{
			return fKind;
		}

		virtual ValueKind GetDataKind() const;


		inline sLONG GetPathID() const
		{
			return fRelationPathID;
		}

		inline sLONG GetPosInOwner() const
		{
			return fPosInOwner;
		}

		virtual sLONG GetPosInModel() const
		{
			return fPosInOwner;
		}

		inline void SetPosInOwner(sLONG pos)
		{
			fPosInOwner = pos;
		}

		inline sLONG GetFieldPos() const
		{
			return fFieldPos;
		}

		inline void SetFieldPos(sLONG pos)
		{
			fFieldPos = pos;
		}

		inline void GetName(VString& outName) const
		{
			outName = fName;
		}

		inline void SetName(const VString& inName)
		{
			fName = inName;
		}

		inline const VString& GetName() const
		{
			return fName;
		}

		virtual void GetAttibuteName(VString& outName) const
		{
			outName = fName;
		}

		virtual const VString& GetAttibuteName() const
		{
			return fName;
		}

		inline EntityModel* GetSubEntityModel() const
		{
			return fSubEntity;
		}

		inline Table* getFlattenTableDest() const
		{
			return fFlattenTableDest;
		}

		inline const VString& getFlattenAttributeName() const
		{
			return fFlattenColumnName;
		}

		inline bool isFlattenedFromField() const
		{
			return fFlattenFromAField;
		}

		virtual CDB4DEntityModel* GetRelatedEntityModel() const;

		EntityRelation* GetRelPath() const
		{
			return fRelation;
		}

		VError GetScriptFromBag(const VValueBag* bag, EntityModelCatalog* catalog, const VValueBag::StKey& inScriptKey, script_attr inWhatScript, bool devMode);
		VError FromBag(const VValueBag* bag, EntityModelCatalog* catalog, bool fieldmustexits, bool devMode);

		VError ScriptToBag(VValueBag& outBag, const VValueBag::StKey& inWhatScript, const VString& inStatement, const VString& inFrom, bool userDefined) const;
		VError ToBag(VValueBag& outBag, bool forDax, bool forSave, bool forJSON, bool isTableDef) const;

		VError ResolveRelatedEntities(SubPathRefSet& already, EntityModelCatalog* catalog, bool devMode, BaseTaskInfo* context);
		VError ResolveQueryStatements(EntityModelCatalog* catalog, bool devMode, BaseTaskInfo* context);

		EntityAttribute* Clone(EntityModel* inModel) const;

		bool IsOverWritten() const
		{
			return fOverWrite;
		};

		void SetOverWrite(bool b)
		{
			fOverWrite = b;
		}

		bool CanBeModified() const
		{
			if (fKind == eattr_alias)
				return false;
			else if (fKind == eattr_computedField)
			{
				return !fScriptStatement[script_attr_set].IsEmpty() || !fScriptFrom[script_attr_set].IsEmpty();
			}
			else
				return fCanBeModified;
		}

#if BuildEmFromTable
		void SetRelation(Relation* rel, EntityRelationKind kind, EntityModelCatalog* catalog);
#endif

		inline SearchTab* GetScriptQuery() const
		{
			return fScriptQuery;
		}

		OptimizedQuery* GetScriptDB4D(BaseTaskInfo* context) const;

		inline script_type GetScriptKind() const
		{
			return fScriptKind;
		}

		const VString& GetScriptStatement(script_attr whichscript) const
		{
			return fScriptStatement[whichscript];
		}

		inline sLONG& GetScriptNum(script_attr whichscript) const
		{
			return fScriptNum[whichscript];
		}

		
		bool GetScriptObjFunc(script_attr whichscript, VJSGlobalContext* jsglobcontext, BaseTaskInfo* context, VJSObject& outObjFunc) const;

		bool getMinValue(const VValueSingle* &outVal) const
		{
			if (fLocalType == nil || fLocalType->GetMin() == nil)
			{
				if (fType == nil || fType->GetMin() == nil)
					return false;
				else
				{
					outVal = fType->GetMin();
					return true;
				}
			}
			else
			{
				outVal = fLocalType->GetMin();
				return true;
			}
		}


		bool getMaxValue(const VValueSingle* &outVal) const
		{
			if (fLocalType == nil || fLocalType->GetMax() == nil)
			{
				if (fType == nil || fType->GetMax() == nil)
					return false;
				else
				{
					outVal = fType->GetMax();
					return true;
				}
			}
			else
			{
				outVal = fLocalType->GetMax();
				return true;
			}
		}


		bool getDefaultValue(const VValueSingle* &outVal) const
		{
			if (fLocalType == nil || fLocalType->GetDefaultValue() == nil)
			{
				if (fType == nil || fType->GetDefaultValue() == nil)
					return false;
				else
				{
					outVal = fType->GetDefaultValue();
					return true;
				}
			}
			else
			{
				outVal = fLocalType->GetDefaultValue();
				return true;
			}
		}


		bool getMinLength(sLONG &outVal) const
		{
			if (fLocalType == nil || fLocalType->GetMinLength() == 0)
			{
				if (fType == nil || fType->GetMinLength() == 0)
					return false;
				else
				{
					outVal = fType->GetMinLength();
					return true;
				}
			}
			else
			{
				outVal = fLocalType->GetMinLength();
				return true;
			}
		}


		bool getMaxLength(sLONG &outVal) const
		{
			if (fLocalType == nil || fLocalType->GetMaxLength() == 0)
			{
				if (fType == nil || fType->GetMaxLength() == 0)
					return false;
				else
				{
					outVal = fType->GetMaxLength();
					return true;
				}
			}
			else
			{
				outVal = fLocalType->GetMaxLength();
				return true;
			}
		}


		bool getFixedLength(sLONG &outVal) const
		{
			if (fLocalType == nil || fLocalType->GetFixedLength() == 0)
			{
				if (fType == nil || fType->GetFixedLength() == 0)
					return false;
				else
				{
					outVal = fType->GetFixedLength();
					return true;
				}
			}
			else
			{
				outVal = fLocalType->GetFixedLength();
				return true;
			}
		}


		bool getPattern(VString &outVal) const
		{
			if (fLocalType == nil || fLocalType->GetPattern().IsEmpty())
			{
				if (fType == nil || fType->GetPattern().IsEmpty())
					return false;
				else
				{
					outVal = fType->GetPattern();
					return true;
				}
			}
			else
			{
				outVal = fLocalType->GetPattern();
				return true;
			}
		}


		bool getPatternReg(const VRegexMatcher* &outVal) const
		{
			if (fLocalType == nil || fLocalType->GetPatternReg() == nil)
			{
				if (fType == nil || fType->GetPatternReg() == nil)
					return false;
				else
				{
					outVal = fType->GetPatternReg();
					return true;
				}
			}
			else
			{
				outVal = fLocalType->GetPatternReg();
				return true;
			}
		}


		bool getFormat(VString &outVal) const
		{
			if (fLocalType == nil || fLocalType->GetFormat().IsEmpty())
			{
				if (fType == nil || fType->GetFormat().IsEmpty())
					return false;
				else
				{
					outVal = fType->GetFormat();
					return true;
				}
			}
			else
			{
				outVal = fLocalType->GetFormat();
				return true;
			}
		}

		sLONG ComputeScalarType() const
		{
			if (fScalarType == 0)
			{
				if (fType != NULL)
					return fType->ComputeScalarType();
				else
				{
					if (fFrom != NULL)
						return fFrom->ComputeScalarType();
					else
						return 0;
				}
			}
			else
				return fScalarType;
		}

		Field* RetainDirectField() const;

		Field* RetainField() const;

		inline const VString& GetIndexKind() const
		{
			return fIndexKind;
		}

		inline void SetPartOfPrimKey()
		{
			fPartOfPrimKey = true;
		}

		VError CallDBEvent(DBEventKind kind, EntityRecord* inRec, BaseTaskInfo* context) const;
		bool HasEvent(DBEventKind kind) const;

		bool IsScalar() const;

		const EntityAttributeScope getScope() const
		{
			return fScope;
		}

		virtual const EntityAttributeScope GetScope() const
		{
			return fScope;
		}

		bool IsSortable() const;

		bool IsStatable() const;
		bool IsSummable() const;


		const VString& GetRelPathAsString() const
		{
			return fRelPath;
		}

		bool NeedValidation() const;

	protected:
		VString fName;
		EntityAttributeKind fKind;
		EntityAttributeScope fScope;
		sLONG fRelationPathID;  // un indice dans EntityModel.fRelationPaths et dans EntityRecord.fSubEntitiesCache
		EntityRelation* fRelation;
		const EntityAttribute* fFrom;
		Table* fFlattenTableDest;
		VString fFlattenColumnName;
		sLONG fPosInOwner;
		sLONG fFieldPos;
		EntityModel* fOwner;
		VString fRelPath;
		VString fSubEntityName;
		VString fIndexKind;
		VUUID fCrit_UUID;
		EntityModel* fSubEntity;
		AttributeType* fType;
		AttributeType* fLocalType;
		const VValueBag* fExtraProperties;
		DBEvent fEvents[dbev_lastEvent+1];
		VString fScriptStatement[script_attr_last+1];
		VString fScriptFrom[script_attr_last+1];
		uBOOL fScriptUserDefined[script_attr_last+1];
		SearchTab* fScriptDB4D;
		SearchTab* fScriptQuery;
		script_type fScriptKind;
		sLONG fScalarType;
		bool fOverWrite,fCanBeModified,fReversePath,fIsForeignKey,fIdentifying,fPartOfPrimKey,fFlattenFromAField, fIsMultiLine;
		mutable OptimizedQuery* fOptimizedScriptDB4D;
		mutable sLONG fScriptNum[script_attr_last+1];

};



									// ---------------------------------------------

class EntityAttributeInstance
{
	public:
		EntityAttributeInstance(const EntityAttribute* inAtt, sLONG inInstance = 0)
		{
			fAtt = inAtt;
			fInstance = inInstance;
		}

		const EntityAttribute* fAtt;
		sLONG fInstance;

		inline bool operator < (const EntityAttributeInstance& other) const
		{
			if (fAtt == other.fAtt)
				return fInstance < other.fInstance;
			else
				return fAtt < other.fAtt;
		}

		inline bool operator == (const EntityAttributeInstance& other) const
		{
			return ( (fInstance == other.fInstance) && (fAtt == other.fAtt) );
		}

		inline bool IsPartOfARelation() const
		{
			EntityAttributeKind kind = fAtt->GetKind();
			return (kind == eattr_relation_Nto1 || kind == eattr_relation_1toN || kind == eattr_composition);
		}
};

typedef vector<EntityAttributeInstance> EntityAttributeInstanceCollection;


typedef vector<EntityAttribute*> EntityAttributeCollection;
typedef map<VString, EntityAttribute*, CompareLessVStringStrict> EntityAttributeMap;

typedef map<VString, EntityMethod*, CompareLessVStringStrict> EntityMethodMap;

class SubEntityCache;
class EntityRecord;

class AttributeStringPath : public IPartable<VectorOfVString, const VString*>
{
	public:
		AttributeStringPath(const VString& inPath);

		void GetString(VString& outString) const;

};


template<>
inline const VString* IPartable<VectorOfVString, const VString*>::GetElem(VectorOfVString::const_iterator iter) const
{
	return &(*iter);
}


class AttributePath : public IPartable<EntityAttributeInstanceCollection, const EntityAttributeInstance*>, public IRefCountable
{
	public:
		AttributePath()
		{
			fIsValid = false;
		}

		AttributePath(EntityModel* model, const VString& inPath);
		AttributePath(EntityModel* model, const AttributeStringPath& inPath, bool fromstart = true);

		bool FromPath(EntityModel* model, const VString& inPath, bool expandAliases = false);
		bool FromPath(EntityModel* model, const AttributeStringPath& inPath, bool expandAliases = false, bool fromstart = true);

		bool IsValid() const
		{
			return fIsValid;
		};

		AttributePath& operator = (const AttributePath& other)
		{
			copyfrom(other);
			fIsValid = other.fIsValid;
			return *this;
		}

		void GetString(VString& outString) const;

		bool operator < (const AttributePath& other) const;

		bool IsEmpty() const
		{
			return fParts.empty();
		}

		void RemoveLast()
		{
			fParts.pop_back();
		}

		bool NeedAJoin() const
		{
			bool res = false;

			if (fParts.size() > 1)
				res = true;
			else
			{
				const EntityAttributeInstance* last = LastPart();
				if (last != nil)
				{
					res = last->IsPartOfARelation();
				}
			}

			return res;
		}

		void Clear()
		{
			fIsValid = true;
			fParts.clear();
		}

		void Add(const EntityAttributeInstance& attInst)
		{
			fParts.push_back(attInst);
		}

	protected:
		bool fIsValid;

};


template<>
inline const EntityAttributeInstance* IPartable<EntityAttributeInstanceCollection, const EntityAttributeInstance*>::GetElem(EntityAttributeInstanceCollection::const_iterator iter) const
{
	return &(*iter);
}



class EntityAttributeSortedSelection;
class EntityAttributeSelection;

class EntityAttributeItem
{
	public:
		inline EntityAttributeItem()
		{
			fAttribute = nil;
			fSousSelection = nil;
			fCount = -1;
			fSkip = -1;
		}

		inline EntityAttributeItem(EntityAttribute* inAttribute)
		{
			fAttribute = inAttribute;
			fSousSelection = nil;
			fCount = -1;
			fSkip = -1;
		}

		EntityAttribute* fAttribute;
		EntityAttributeSelection* fSousSelection;
		sLONG fCount;
		sLONG fSkip;
};


class EntityAttributeSelection : public vector<EntityAttributeItem>  // le vector est toujours dimensionne au nombre d'attributs du model et les non-trous representent les attributs selectionnes
{
	public:

		EntityAttributeSelection(EntityModel* inModel);

		EntityAttributeSelection(EntityModel* inModel, const VString& inListOfAtttibutes, bool FirstLevelOnly = false, RestTools* req = nil);

		bool BuildFromString(const VString& inListOfAtttibutes, bool FirstLevelOnly, RestTools* req);

		bool AddAttribute(const VString& inAttributePath, RestTools* req);
		bool AddAttribute(const AttributeStringPath& inAttributePath, RestTools* req);

	protected:
		EntityModel* fModel;
		void Dispose();

};




class EntityAttributeSortedItem
{
public:
	inline EntityAttributeSortedItem()
	{
		fAttribute = nil;
		fSousSelection = nil;
		fAscent = true;
		fAttPath = nil;
	}

	inline EntityAttributeSortedItem(EntityAttribute* inAttribute, bool ascent = true)
	{
		fAttribute = inAttribute;
		fSousSelection = nil;
		fAscent = ascent;
		fAttPath = nil;
	}

	inline EntityAttributeSortedItem(AttributePath* inAttributePath, bool ascent = true)
	{
		fAttPath = inAttributePath;
		fAttribute = (EntityAttribute*) inAttributePath->FirstPart()->fAtt;
		fSousSelection = nil;
		fAscent = ascent;
	}

	EntityAttribute* fAttribute;
	AttributePath* fAttPath;
	EntityAttributeSortedSelection* fSousSelection;
	bool fAscent;
};


class EntityAttributeSortedSelection : public vector<EntityAttributeSortedItem>
{
	public:
		
		EntityAttributeSortedSelection()
		{
			fModel = nil;
		}

		EntityAttributeSortedSelection(EntityModel* inModel)
		{
			fModel = inModel;
		}

		EntityAttributeSortedSelection(EntityModel* inModel, const VString& inListOfAtttibutes, BaseTaskInfo* context, bool FirstLevelOnly = false, RestTools* req = nil)
		{
			fModel = inModel;
			BuildFromString(inListOfAtttibutes, context, FirstLevelOnly, false, req);
		}

		~EntityAttributeSortedSelection()
		{
			Dispose();
		}

		inline void SetModel(EntityModel* inModel)
		{
			fModel = inModel;
		}

		inline EntityModel* GetModel()
		{
			return fModel;
		}

		inline const EntityModel* GetModel() const
		{
			return fModel;
		}

		bool BuildFromString(const VString& inListOfAtttibutes, BaseTaskInfo* context, bool FirstLevelOnly, bool forSorting, RestTools* req);

		bool AddAttribute(const VString& inAttributePath, bool ascent, bool forSorting, RestTools* req);
		bool AddAttribute(const AttributeStringPath& inAttributePath, bool ascent, bool forSorting, RestTools* req);

		bool AddAttribute(EntityAttribute* att, RestTools* req);

		EntityAttributeSortedSelection::iterator FindAttribute(const EntityAttribute* att);
		EntityAttributeSortedSelection::const_iterator FindAttribute(const EntityAttribute* att) const;

		EntityAttributeSortedSelection* FindSubSelection(const EntityAttribute* att) const;

		void ToString(VString& outString);


	protected:
		EntityModel* fModel;
		void Dispose();
		

};


class IdentifyingAttribute
{
	public:
		IdentifyingAttribute(EntityAttribute* att = nil)
		{
			fAtt = att;
			fOptionnel = false;
		}

		VRefPtr<EntityAttribute> fAtt;
		bool fOptionnel;
};

typedef vector<IdentifyingAttribute> IdentifyingAttributeCollection;



									// ---------------------------------------------


class RestTools;

class EntityModel : public VComponentImp<CDB4DEntityModel>
{
	public:

		typedef list<EntityModel*> ListOfModels;

		EntityModel(Base4D* inOwner, Table* inMainTable = nil);

		VError ThrowError( VError inErrCode, const VString* p1 = nil) const;

		inline Base4D* GetOwner() const
		{
			return fOwner;
		}

		virtual CDB4DBase* RetainDataBase() const
		{
			return fOwner->RetainBaseX();
		}

		virtual sLONG CountAttributes() const
		{
			return (sLONG)fAttributes.size();
		}

		inline Table* GetMainTable() const
		{
			return fMainTable;
		}

		virtual CDB4DTable* RetainTable() const;

		inline sLONG CountRelationPaths() const
		{
			return (sLONG)fRelationPaths.size();
		}

		inline EntityRelation* GetPath(sLONG inPathID) const
		{
			return fRelationPaths[inPathID];
		}

		inline void SetName(const VString& name)
		{
			fName = name;
		}

		inline void GetName(VString& outName) const
		{
			outName = fName;
		}

		inline const VString& GetName() const
		{
			return fName;
		}

		virtual void GetEntityName(VString& outName) const
		{
			outName = fName;
		}

		virtual const VString& GetEntityName() const
		{
			return fName;
		}


		inline const VString& GetCollectionName() const
		{
			return fCollectionName;
		}

		inline void SetCollectionName(const VString& colName)
		{
			fCollectionName = colName;
		}

		VError BuildRelPath(SubPathRefSet& already, EntityModelCatalog* catalog, const VectorOfVString& path, EntityRelationCollection& outRelPath, VString& outLastpart, bool& outAllNto1, bool devMode, EntityModel* &outLastDest);
		VError ActivatePath(EntityRecord* erec, sLONG inPathID, SubEntityCache& outResult, bool Nto1, EntityModel* subEntityModel, BaseTaskInfo* context);

		Selection* projectSelection(Selection* sel, EntityAttribute* att, VError& err, BaseTaskInfo* context);
		virtual CDB4DSelection* ProjectSelection(CDB4DSelection* sel, CDB4DEntityAttribute* att, VError& err, CDB4DBaseContext* context);

		EntityAttribute* getAttribute(sLONG pos) const;
		EntityAttribute* getAttribute(const VString& AttributeName) const;

		virtual CDB4DEntityAttribute* GetAttribute(sLONG pos) const
		{
			return getAttribute(pos);
		}

		virtual CDB4DEntityAttribute* GetAttribute(const VString& AttributeName) const
		{
			return getAttribute(AttributeName);
		}

		EntityAttribute* FindAttributeByFieldPos(sLONG FieldPos);

		VError ToBag(VValueBag& outBag, bool forDax, bool forSave, bool forJSON, RestTools* req = nil) const;
		VError FromBag(const VValueBag* inBag, EntityModelCatalog* catalog, bool devMode);

		VError SaveToBag(VValueBag& CatalogBag, bool forJSON) const;

		virtual XBOX::VValueBag *CreateDefinition() const
		{
			VValueBag *bag = new VValueBag;
			if (bag != nil)
			{
				ToBag( *bag, false /* fordax */, false /* forSave */, true /* forJSON */, nil);
			}
			return bag;
		}

		sLONG FindRelationPath(const EntityRelation* relpath) const;

		sLONG AddRelationPath(EntityRelation* relpath);

		VError ResolveRelatedEntities(SubPathRefSet& already, EntityModelCatalog* catalog, bool devMode, BaseTaskInfo* context);
		VError ResolveQueryStatements(EntityModelCatalog* catalog, bool devMode, BaseTaskInfo* context);
		//VError ResolveRelatedPath(EntityModelCatalog* catalog);

		sLONG FindAttribute(const VString& AttributeName) const;

		bool GetAllSortedAttributes(EntityAttributeSortedSelection& outAtts, RestTools* req) const;
		bool BuildListOfSortedAttributes(const VString& inAttributeList, EntityAttributeSortedSelection& outAtts, BaseTaskInfo* context, bool FirstLevelOnly, bool forSorting, RestTools* req) const;

		bool GetAllAttributes(EntityAttributeSelection& outAtts, RestTools* req) const;
		bool BuildListOfAttributes(const VString& inAttributeList, EntityAttributeSelection& outAtts, bool FirstLevelOnly, RestTools* req) const;

		EntityRecord* LoadEntityRecord(sLONG n, VError& err, DB4D_Way_of_Locking HowToLock, BaseTaskInfo* context, bool autoexpand);

		virtual CDB4DEntityRecord* LoadEntity(sLONG n, VError& err, DB4D_Way_of_Locking HowToLock, CDB4DBaseContext* context, bool autoexpand);


		EntityAttribute* GetAttributeByPath(const VString& inPath) const;
		EntityAttribute* GetAttributeByPath(const AttributeStringPath& inPath) const;

		virtual sLONG CountEntities(CDB4DBaseContext* inContext);
		
		virtual CDB4DSelection* SelectAllEntities(CDB4DBaseContextPtr inContext, VErrorDB4D* outErr = NULL, 
												  DB4D_Way_of_Locking HowToLock = DB4D_Do_Not_Lock, CDB4DSet* outLockSet = NULL);
		
		Selection* SelectAllEntities(BaseTaskInfo* context, VErrorDB4D* outErr = NULL, 
										DB4D_Way_of_Locking HowToLock = DB4D_Do_Not_Lock, Bittab* outLockSet = NULL);

		virtual CDB4DQuery *NewQuery();
		
		virtual CDB4DSelection* ExecuteQuery( CDB4DQuery *inQuery, CDB4DBaseContextPtr inContext, CDB4DSelectionPtr Filter = NULL, 
											 VDB4DProgressIndicator* InProgress = NULL, DB4D_Way_of_Locking HowToLock = DB4D_Do_Not_Lock, 
											 sLONG limit = 0, CDB4DSet* outLockSet = NULL, VErrorDB4D *outErr = NULL);
		
		Selection* ExecuteQuery( SearchTab* querysearch, BaseTaskInfo* context, Selection* filter = nil, 
									VDB4DProgressIndicator* InProgress = NULL, DB4D_Way_of_Locking HowToLock = DB4D_Do_Not_Lock, 
									sLONG limit = 0, Bittab* outLockSet = NULL, VError *outErr = NULL);

	
		virtual CDB4DSelection* NewSelection(DB4D_SelectionType inSelectionType) const;
		
		virtual CDB4DEntityRecord* NewEntity(CDB4DBaseContextPtr inContext, DB4D_Way_of_Locking HowToLock) const;

		EntityRecord* NewEntity(BaseTaskInfo* inContext, DB4D_Way_of_Locking HowToLock) const;

#if BuildEmFromTable
		static EntityModel* BuildEntityModel(Table* from, EntityModelCatalog* catalog);
#endif

		//static void ClearCacheTableEM();

		bool AddRestrictionToQuery(SearchTab& query, BaseTaskInfo* context, VError& err);
		Selection* BuildRestrictingSelection(BaseTaskInfo* context, VError& err);

		bool WithRestriction() const
		{
			return fWithRestriction;
		}

		bool WithRestrictingQuery() const
		{
			return fRestrictingQuery != nil;
		}

		const VString& GetCumulatedQuery() const
		{
			return fCumulatedRestrictingQueryString;
		}

		inline const VString& GetBaseEmName() const
		{
			return fExtends;
		}

		inline EntityModel* GetBaseEm() const
		{
			return fBaseEm;
		}

		void AddDerivated(EntityModel* em);

		inline const ListOfModels& GetAllDerivateds() const
		{
			return fDerivateds;
		}

		inline sLONG GetDefaultTopSize() const
		{
			return fDefaultTopSize;
		}

		inline sLONG GetDefaultTopSizeInUse() const
		{
			if (fDefaultTopSize <= 0)
				return kDefaultTopSize;
			else
				return fDefaultTopSize;
		}

		inline const IdentifyingAttributeCollection* GetIdentifyingAtts() const
		{
			return &fIdentifyingAtts;
		}

		inline bool HasPrimKey() const
		{
			return !fPrimaryAtts.empty();
		}

		inline bool HasIdentifyingAtts() const
		{
			return !fIdentifyingAtts.empty();
		}

		inline const IdentifyingAttributeCollection* GetPrimaryAtts() const
		{
			return &fPrimaryAtts;
		}

		virtual bool HasPrimaryKey() const
		{
			return !fPrimaryAtts.empty();
		}

		virtual bool HasIdentifyingAttributes() const
		{
			return !fIdentifyingAtts.empty();
		}

		bool HasDeleteEvent(bool onlyCheckAttributes = false) const;

		const EntityAttributeScope getScope() const
		{
			return fScope;
		}

		virtual const EntityAttributeScope GetScope() const
		{
			return fScope;
		}

		bool isExtendedFrom(const EntityModel* otherEM) const;

		sLONG getEntityNumWithPrimKey(const VString& primkey, BaseTaskInfo* context, VError& err);
		sLONG getEntityNumWithPrimKey(const VectorOfVString& primkey, BaseTaskInfo* context, VError& err);
		sLONG getEntityNumWithPrimKey(const VValueBag& primkey, BaseTaskInfo* context, VError& err, bool ErrOnNull = true);
		sLONG getEntityNumWithPrimKey(const VectorOfVValue& primkey, BaseTaskInfo* context, VError& err);
		sLONG getEntityNumWithPrimKey(VJSObject objkey, BaseTaskInfo* context, VError& err);

		EntityRecord* findEntityWithPrimKey(const VString& primkey, BaseTaskInfo* context, VError& err, DB4D_Way_of_Locking HowToLock);
		EntityRecord* findEntityWithPrimKey(const VectorOfVString& primkey, BaseTaskInfo* context, VError& err, DB4D_Way_of_Locking HowToLock);
		EntityRecord* findEntityWithPrimKey(const VValueBag& primkey, BaseTaskInfo* context, VError& err, DB4D_Way_of_Locking HowToLock);
		EntityRecord* findEntityWithPrimKey(const VectorOfVValue& primkey, BaseTaskInfo* context, VError& err, DB4D_Way_of_Locking HowToLock);
		EntityRecord* findEntityWithPrimKey(VJSObject objkey, BaseTaskInfo* context, VError& err, DB4D_Way_of_Locking HowToLock);

		virtual CDB4DEntityRecord* FindEntityWithPrimKey(const VString& primkey, CDB4DBaseContext* inContext, VErrorDB4D& err, DB4D_Way_of_Locking HowToLock);

		virtual CDB4DEntityRecord* FindEntityWithPrimKey(const VectorOfVString& primkey, CDB4DBaseContext* inContext, VErrorDB4D& err, DB4D_Way_of_Locking HowToLock);

		virtual CDB4DEntityRecord* FindEntityWithPrimKey(const VValueBag& primkey, CDB4DBaseContext* inContext, VErrorDB4D& err, DB4D_Way_of_Locking HowToLock);

		virtual CDB4DEntityRecord* FindEntityWithPrimKey(const VectorOfVValue& primkey, CDB4DBaseContext* inContext, VErrorDB4D& err, DB4D_Way_of_Locking HowToLock);



		sLONG getEntityNumWithIdentifyingAtts(const VectorOfVString& idents, BaseTaskInfo* context, VError& err);
		sLONG getEntityNumWithIdentifyingAtts(const VValueBag& idents, BaseTaskInfo* context, VError& err, bool ErrOnNull = true);
		sLONG getEntityNumWithIdentifyingAtts(const VectorOfVValue& idents, BaseTaskInfo* context, VError& err);

		EntityRecord* findEntityWithIdentifyingAtts(const VectorOfVString& idents, BaseTaskInfo* context, VError& err, DB4D_Way_of_Locking HowToLock);
		EntityRecord* findEntityWithIdentifyingAtts(const VValueBag& idents, BaseTaskInfo* context, VError& err, DB4D_Way_of_Locking HowToLock);
		EntityRecord* findEntityWithIdentifyingAtts(const VectorOfVValue& idents, BaseTaskInfo* context, VError& err, DB4D_Way_of_Locking HowToLock);

		virtual CDB4DEntityRecord* FindEntityWithIdentifyingAtts(const VectorOfVString& idents, CDB4DBaseContext* inContext, VErrorDB4D& err, DB4D_Way_of_Locking HowToLock);

		virtual CDB4DEntityRecord* FindEntityWithIdentifyingAtts(const VValueBag& idents, CDB4DBaseContext* inContext, VErrorDB4D& err, DB4D_Way_of_Locking HowToLock);

		virtual CDB4DEntityRecord* FindEntityWithIdentifyingAtts(const VectorOfVValue& idents, CDB4DBaseContext* inContext, VErrorDB4D& err, DB4D_Way_of_Locking HowToLock);

		bool MatchPrimKeyWithDataSource() const;

		const VString& GetDefaultOrderBy() const
		{
			return fRestrictingOrderByString;
		}

		virtual Selection* query( const VString& inQuery, BaseTaskInfo* context, VErrorDB4D& err, const VValueSingle* param1 = nil, const VValueSingle* param2 = nil, const VValueSingle* param3 = nil);
		virtual CDB4DSelection* Query( const VString& inQuery, CDB4DBaseContext* inContext, VErrorDB4D& err, const VValueSingle* param1 = nil, const VValueSingle* param2 = nil, const VValueSingle* param3 = nil);
		virtual CDB4DEntityRecord* Find(const VString& inQuery, CDB4DBaseContext* inContext, VErrorDB4D& err, const VValueSingle* param1 = nil, const VValueSingle* param2 = nil, const VValueSingle* param3 = nil);

		virtual VError SetPermission(DB4D_EM_Perm inPerm, const VUUID& inGroupID, bool forced);
		virtual VError GetPermission(DB4D_EM_Perm inPerm, VUUID& outGroupID, bool& forced) const;
		virtual bool PermissionMatch(DB4D_EM_Perm inPerm, CUAGSession* inSession) const;
		bool permissionMatch(DB4D_EM_Perm inPerm, CUAGSession* inSession) const;

		EntityMethod* getMethod(const VString& inMethodName, bool publicOnly = false) const;

		virtual CDB4DEntityMethod* GetMethod(const VString& inMethodName) const
		{
			return getMethod(inMethodName);
		}

		VError callMethod(const VString& inMethodName, const VectorOfVString& params, VJSValue& result, CDB4DBaseContext* inContext, Selection* inSel = nil, EntityRecord* inRec = nil);
		VError callMethod(const VString& inMethodName, const VString& jsonparams, VJSValue& result, CDB4DBaseContext* inContext, Selection* inSel = nil, EntityRecord* inRec = nil);
		VError callMethod(const VString& inMethodName, const VValueBag& bagparams, VJSValue& result, CDB4DBaseContext* inContext, Selection* inSel = nil, EntityRecord* inRec = nil);

		inline const EntityAttributeCollection& getAllAttributes() const
		{
			return fAttributes;
		}

		VError compute(EntityAttributeSortedSelection& atts, Selection* sel, VJSObject& outObj, BaseTaskInfo* context, JS4D::ContextRef jscontext);

		VError CallDBEvent(DBEventKind kind, BaseTaskInfo* context, Selection* *outSel) const;
		VError CallDBEvent(DBEventKind kind, EntityRecord* inRec, BaseTaskInfo* context) const;
		VError CallAttributesDBEvent(DBEventKind kind, EntityRecord* inRec, BaseTaskInfo* context) const;

		const EntityAttributeCollection& GetAttributesWithInitEvent() const
		{
			return fAttributesWithInitEvent;
		}

		void ResolvePermissionsInheritance(EntityModelCatalog* catalog);

		bool publishAsGlobal(CDB4DBaseContext* context) const
		{
			return fPublishAsGlobal;
		}

		bool allowOverrideStamp() const
		{
			return fAllowOverrideStamp;
		}

	protected:
		virtual ~EntityModel();

		VString fName;
		VString fCollectionName;
		VString fExtends;
		EntityModel* fBaseEm;
		Table* fMainTable;
		Base4D* fOwner;
		ListOfModels fDerivateds;
		list<VString> fRemoveAttributes;
		EntityAttributeCollection fAttributes;
		EntityAttributeMap fAttributesByName;
		EntityMethodMap fMethodsByName;
		EntityRelationCollection fRelationPaths;
		IdentifyingAttributeCollection fIdentifyingAtts;
		IdentifyingAttributeCollection fOwnedIdentifyingAtts;
		IdentifyingAttributeCollection fPrimaryAtts;
		IdentifyingAttributeCollection fOwnedPrimaryAtts;
		EntityAttributeCollection fAttributesWithInitEvent;
		VString fRestrictingOrderByString;
		VString fRestrictingQueryString;
		VString fCumulatedRestrictingQueryString;
		SearchTab* fRestrictingQuery;
		const VValueBag* fExtraProperties;
		VUUID fPerms[DB4D_EM_Promote_Perm+1];
		uBYTE fForced[DB4D_EM_Promote_Perm+1];
		DBEvent fEvents[dbev_lastEvent+1];
		sLONG fQueryLimit;
		sLONG fDefaultTopSize;
		EntityAttributeScope fScope;
		bool fQueryApplyToEM, fAlreadyResolvedComputedAtts, fIsTableDef, fHasDeleteEvent, fOneAttributeHasDeleteEvent, 
			fPublishAsGlobal, fPublishAsGlobalDefined, fWithRestriction, fAllowOverrideStamp;
		mutable uBYTE fMatchPrimKey;

		/*
		static map<Table*, EntityModel*> sEMbyTable;
		static VCriticalSection sEMbyTableMutex;
		*/
		

};



									// ---------------------------------------------

class EntityRecord;


class SubEntityCache
{
	public:
		inline SubEntityCache()
		{
			fSel = nil;
			fRec = nil;
			fErec = nil;
			fAlreadyActivated = false;
			fModel = nil;
			fLockWay = DB4D_Do_Not_Lock;
		}

		inline void SetRecord(FicheInMem* rec)
		{
			fRec = rec;
			fAlreadyActivated = true;
		}

		inline FicheInMem* GetRecord() const
		{
			return fRec;
		}

		inline void SetSel(Selection* sel)
		{
			fSel = sel;
			fAlreadyActivated = true;
		}

		inline Selection* GetSel() const
		{
			return fSel;
		}

		bool AlreadyActivated() const
		{
			return fAlreadyActivated;
		}

		void SetActivated()
		{
			fAlreadyActivated = true;
		}

		void SetModel(EntityModel* inModel)
		{
			fModel = inModel;
		}

		void SetWayOfLocking(DB4D_Way_of_Locking inLockWay)
		{
			fLockWay = inLockWay;
		}

		~SubEntityCache();

		void Clear();

		EntityRecord* GetErec();
			

	protected:
		Selection* fSel;
		FicheInMem* fRec;
		EntityRecord* fErec;
		EntityModel* fModel;
		DB4D_Way_of_Locking fLockWay;
		bool fAlreadyActivated;
};




typedef vector<SubEntityCache> SubEntityCacheCollection;

//typedef enum { eav_vvalue = 1, eav_subentity, eav_selOfSubentity } EntityAttributeValueKind;

class EntityAttributeValue : public VComponentImp<CDB4DEntityAttributeValue>
{
	public:
		
#if debuglr
		virtual CComponent*	Retain (const char* DebugInfo = 0);
		virtual void		Release (const char* DebugInfo = 0);
#endif

		EntityAttributeValue(EntityRecord* owner, const EntityAttribute* attribute, EntityAttributeValueKind kind, VValueSingle* inVal, bool isowned = false);

		EntityAttributeValue(EntityRecord* owner, const EntityAttribute* attribute, EntityAttributeValueKind kind, EntityRecord* erec, EntityModel* inSubModel);

		EntityAttributeValue(EntityRecord* owner, const EntityAttribute* attribute, EntityAttributeValueKind kind, Selection* sel, EntityModel* inSubModel);


		inline EntityAttributeValueKind GetKind() const
		{
			return fType;
		}

		virtual EntityAttributeValueKind GetAttributeKind() const
		{
			return fType;
		}

		inline VValueSingle* getVValue() const
		{
			return fValue;
		}

		inline void setVValue(VValueSingle* cv)
		{
			if (fValue != nil && fIsValueOwned)
				delete fValue;
			fValue = cv;
		}

		virtual VValueSingle* GetVValue() const
		{
			return fValue;
		}

		EntityRecord* getRelatedEntity();

		VError setRelatedKey(const VectorOfVValue& key);

		virtual CDB4DEntityRecord* GetRelatedEntity() const;

		inline EntityModel* getRelatedEntityModel() const
		{
			return fSubModel;
		}

		virtual CDB4DEntityModel* GetRelatedEntityModel() const
		{
			return fSubModel;
		}

		inline Selection* getRelatedSelection() const
		{
			return fSel;
		}

		inline uLONG GetStamp() const
		{
			return fStamp;
		}

		void Touch(BaseTaskInfo* context);

		inline void UnTouch()
		{
			fStamp = 0;
		}

		virtual CDB4DSelection* GetRelatedSelection() const;

		inline bool CanBeModified() const
		{
			return fCanBeModified;
		}

		inline void AllowModifications(bool x)
		{
			fCanBeModified = x;
		}

		VError Save(Transaction* &trans, BaseTaskInfo* context);

		VError SetRelatedEntity(EntityRecord* relatedEntity, BaseTaskInfo* context);

		bool equalRelatedEntity(EntityRecord* relatedEntity);
		bool equalValue(const VValueSingle* inValue);

		VError Validate(BaseTaskInfo* context);

		/*
		virtual void SetExtraData(void* ExtraData)
		{
			fExtraData = ExtraData;
		}

		virtual void* GetExtraData() const
		{
			return fExtraData;
		}
		*/

		virtual VError GetJsonString(VString& outJsonValue);
		VError GetJSObject(VJSObject& outJSObject);
		VError SetJSObject(const VJSObject& inJSObject);
		
		/*
		VError convertToJSObject(VJSObject& outObj);
		virtual VError ConvertToJSObject(VJSObject& outObj)
		{
			return convertToJSObject(outObj);
		}
		*/

		virtual VError SetJsonString(const VString& inJsonValue)
		{
			fJsonString = inJsonValue;
			ReleaseRefCountable(&fValueBag);
			return VE_OK;
			//penser a recalculer le JSobject
		}

		EntityAttributeValue* Clone(EntityRecord* newOwner);


	protected:

		virtual ~EntityAttributeValue();

		void _Init(EntityAttributeValueKind kind)
		{
			fValue = nil;
			fSubEntity = nil;
			fOldSubEntity = nil;
			fSel = nil;
			fSubModel = nil;
			fType = kind;
			fIsValueOwned = false;
			fCDB4DSel = nil;
			fStamp = 0;
			fCanBeModified = true;
			fSubModificationStamp = 0;
			//fExtraData = nil;
			fValueBag = nil;
			fJSObjectIsValid = false;
			fRelatedKey = nil;
			fSubEntityCanTryToReload = true;
		}

		EntityRecord* fOwner;
		const EntityAttribute* fAttribute;
		VString fJsonString;
		VValueBag* fValueBag;
		VValueSingle* fValue;
		EntityRecord* fSubEntity;
		EntityRecord* fOldSubEntity;
		EntityModel* fSubModel;
		VValueSingle* fRelatedKey;
		VJSObject fJSObject;
		//void* fExtraData;
		Selection* fSel;
		mutable CDB4DSelection* fCDB4DSel;
		EntityAttributeValueKind fType;
		bool fIsValueOwned, fCanBeModified, fJSObjectIsValid, fSubEntityCanTryToReload;
		uLONG fStamp, fSubModificationStamp;
};

typedef vector<EntityAttributeValue*> EntityAttributeValueCollection;



									// ---------------------------------------------



class EntityRecord : public VComponentImp<CDB4DEntityRecord>
{
	public:
		
#if debuglr
		virtual CComponent*	Retain (const char* DebugInfo = 0);
		virtual void		Release (const char* DebugInfo = 0);
#endif

		EntityRecord(EntityModel* inModel, FicheInMem* inMainRec, CDB4DBaseContext* inContext, DB4D_Way_of_Locking HowToLock);


		inline EntityModel* GetOwner() const
		{
			return fModel;
		}

		virtual CDB4DEntityModel* GetModel() const
		{
			return fModel;
		}

		inline FicheInMem* getRecord() const
		{
			return fMainRec;
		}

		virtual CDB4DRecord* GetRecord() const;

		VError ThrowError( VError inErrCode, const VString* p1 = nil) const;

		SubEntityCache* GetSubEntityCache(sLONG inPathID, VError& err, bool Nto1, EntityModel* subEntityModel, BaseTaskInfo* context);
		bool SubEntityCacheNeedsActivation(sLONG inPathID);
		void ClearSubEntityCache(sLONG inPathID);

		EntityAttributeValue* getAttributeValue(const VString& inAttributeName, VError& err, BaseTaskInfo* context, bool restrictValue = false);
		EntityAttributeValue* getAttributeValue(sLONG pos, VError& err, BaseTaskInfo* context, bool restrictValue = false);
		EntityAttributeValue* getAttributeValue(const EntityAttribute* inAttribute, VError& err, BaseTaskInfo* context, bool restrictValue = false);
		EntityAttributeValue* getAttributeValue(const AttributePath& inAttPath, VError& err, BaseTaskInfo* context, bool restrictValue = false);

		virtual CDB4DEntityAttributeValue* GetAttributeValue(const VString& inAttributeName, VError& err);

		virtual CDB4DEntityAttributeValue* GetAttributeValue(sLONG pos, VError& err);

		virtual CDB4DEntityAttributeValue* GetAttributeValue(const CDB4DEntityAttribute* inAttribute, VError& err);


		bool equalAttributeValue(const VString& inAttributeName, const VValueSingle* inValue);
		bool equalAttributeValue(const EntityAttribute* inAttribute, const VValueSingle* inValue);

		VError setAttributeValue(const VString& inAttributeName, const VValueSingle* inValue);
		VError setAttributeValue(const EntityAttribute* inAttribute, const VValueSingle* inValue);

		VError setAttributeValue(const VString& inAttributeName, EntityRecord* inRelatedEntity);
		VError setAttributeValue(const EntityAttribute* inAttribute, EntityRecord* inRelatedEntity);

		VError setAttributeValue(const VString& inAttributeName, const VectorOfVValue& inIdentKey);
		VError setAttributeValue(const EntityAttribute* inAttribute, const VectorOfVValue& inIdentKey);

		VError setAttributeValue(const EntityAttribute* inAttribute, const VString& inJsonValue);
		VError setAttributeValue(const VString& inAttributeName, const VString& inJsonValue);

		VError setAttributeValue(const EntityAttribute* inAttribute, const VJSObject& inJSObject);
		VError setAttributeValue(const VString& inAttributeName, const VJSObject& inJSObject);


		virtual VError SetAttributeValue(const CDB4DEntityAttribute* inAttribute, const VValueSingle* inValue)
		{
			return setAttributeValue(VImpCreator<EntityAttribute>::GetImpObject(inAttribute), inValue);
		}

		virtual VError SetAttributeValue(const VString& inAttributeName, const VValueSingle* inValue)
		{
			return setAttributeValue(inAttributeName, inValue);
		}


		virtual VError SetAttributeValue(const CDB4DEntityAttribute* inAttribute, CDB4DEntityRecord* inRelatedEntity)
		{
			return setAttributeValue(VImpCreator<EntityAttribute>::GetImpObject(inAttribute), VImpCreator<EntityRecord>::GetImpObject(inRelatedEntity));
		}

		virtual VError SetAttributeValue(const VString& inAttributeName, CDB4DEntityRecord* inRelatedEntity)
		{
			return setAttributeValue(inAttributeName, VImpCreator<EntityRecord>::GetImpObject(inRelatedEntity));
		}


		virtual VError SetAttributeValue(const CDB4DEntityAttribute* inAttribute, const VectorOfVValue* inValues)
		{
			return setAttributeValue(VImpCreator<EntityAttribute>::GetImpObject(inAttribute), *inValues);
		}

		virtual VError SetAttributeValue(const VString& inAttributeName, const VectorOfVValue* inValues)
		{
			return setAttributeValue(inAttributeName, *inValues);
		}


		VError SetAttributeValue(const CDB4DEntityAttribute* inAttribute, const VString& inJsonValue)
		{
			return setAttributeValue(VImpCreator<EntityAttribute>::GetImpObject(inAttribute), inJsonValue);
		}

		VError SetAttributeValue(const VString& inAttributeName, const VString& inJsonValue)
		{
			return setAttributeValue(inAttributeName, inJsonValue);
		}

		VError SetAttributeValue(const CDB4DEntityAttribute* inAttribute, const VJSObject& inJSObject)
		{
			return setAttributeValue(VImpCreator<EntityAttribute>::GetImpObject(inAttribute), inJSObject);
		}

		VError SetAttributeValue(const VString& inAttributeName, const VJSObject& inJSObject)
		{
			return setAttributeValue(inAttributeName, inJSObject);
		}


		VError touchAttributeValue(const EntityAttribute* inAttribute);

		virtual VError TouchAttributeValue(const CDB4DEntityAttribute* inAttribute)
		{
			return touchAttributeValue(VImpCreator<EntityAttribute>::GetImpObject(inAttribute));
		}


		virtual sLONG GetNum() const
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

		bool ContainsBlobData(const EntityAttribute* inAttribute, BaseTaskInfo* context);

		VError Save(Transaction* *trans, BaseTaskInfo* context, uLONG stamp, bool allowOverrideStamp = false);
		virtual VErrorDB4D Save(uLONG stamp, bool allowOverrideStamp = false);

		VError Validate(BaseTaskInfo* context);
		virtual VErrorDB4D Validate();
		
		virtual Boolean IsNew() const;
		virtual Boolean IsProtected() const;
		virtual Boolean IsModified() const;
		
		virtual void GetTimeStamp(VTime& outValue);
		
		virtual VErrorDB4D Drop();

		FicheInMem* GetRecordForAttribute(const EntityAttribute* inAttribute);

		inline uLONG GetStamp() const
		{
			return fStamp;
		}

		inline void Touch(BaseTaskInfo* context)
		{
			fStamp++;
			CallDBEvent(dbev_set, context);
		}

		void GetPrimKeyValue(VJSObject& outObj);
		void GetPrimKeyValue(VValueBag& outBagKey);
		void GetPrimKeyValue(VString& outKey, bool autoQuotes);

		virtual void GetPrimKeyValue(VectorOfVValue& outPrimkey);

		VError setIdentifyingAtts(const VectorOfVValue& idents);
		VError setPrimKey(const VectorOfVValue& primkey);

		VError setPrimKey(VJSObject objkey);

		virtual VErrorDB4D SetIdentifyingAtts(const VectorOfVValue& idents)
		{
			return setIdentifyingAtts(idents);
		}

		virtual VErrorDB4D SetPrimKey(const VectorOfVValue& primkey)
		{
			return SetPrimKey(primkey);
		}

		virtual CDB4DBaseContext* GetContext()
		{
			return fContext;
		}

		BaseTaskInfo* getContext();

		//virtual void ReleaseExtraDatas();

		VValueSingle* getFieldValue(Field* cri, VError& err);
		VError setFieldValue(Field* cri, const VValueSingle* inValue);

		VError ConvertToJSObject(VJSObject& outObj, EntityAttributeSortedSelection& whichAttributes, EntityAttributeSelection* expandAttributes, 
			EntityAttributeSortedSelection* sortingAttributes, bool withKey, bool allowEmptyAttList);

		virtual VError ConvertToJSObject(VJSObject& outObj, const VString& inAttributeList, bool withKey, bool allowEmptyAttList);
		

		/*
		VError convertToJSObject(VJSObject& outObj);

		virtual VError ConvertToJSObject(VJSObject& outObj)
		{
			return convertToJSObject(outObj);
		}
		*/

		VError convertFromJSObj(VJSObject recobj);

		VError CallDBEvent(DBEventKind kind, BaseTaskInfo* context);

		VError CallInitEvent(BaseTaskInfo* context = nil);

		bool OKToCallEvent(DBEventKind kind);

		void ReleaseCallEvent(DBEventKind kind);


	protected:

		virtual ~EntityRecord();

		EntityModel* fModel;
		FicheInMem* fMainRec;
		mutable CDB4DRecord* fCDB4DMainRec;
		CDB4DBaseContext* fContext;
		SubEntityCacheCollection fSubEntitiesCache;  // a mettre en parallele avec EntityModel.fRelationPaths
		EntityAttributeValueCollection fValues;
		uLONG fModificationStamp, fStamp;
		DB4D_Way_of_Locking fWayOfLock;
		uBYTE fAlreadyCallEvent[dbev_lastEvent+1];
		bool fAlreadySaving, fAlreadyDeleting;
};


VError SelToJSObject(BaseTaskInfo* context, VJSArray& outArr, EntityModel* em, Selection* inSel, EntityAttributeSortedSelection& whichAttributes, EntityAttributeSelection* expandAttributes, 
					 EntityAttributeSortedSelection* sortingAttributes, bool withKey, bool allowEmptyAttList, sLONG from, sLONG count);

VError SelToJSObject(BaseTaskInfo* context, VJSArray& outArr, EntityModel* em, Selection* inSel, const VString& inAttributeList , bool withKey, bool allowEmptyAttList, sLONG from, sLONG count);



// ----------------------------------------------------------------------------------------------------------------------------------



typedef map<VString, EntityModel*, CompareLessVStringStrict> EntityModelMap;
//typedef map<VString, EntityRelation*> EntityRelationMap;
typedef map<VString, AttributeType*, CompareLessVStringStrict> AttributeTypeMap;
typedef map<VString, EmEnum*> EmEnumMap;

class Base4D;


class EntityModelCatalog : public IRefCountable
{

	public:
		EntityModelCatalog(Base4D* owner)
		{
			fOwner = owner;
			fExtraProperties = nil;
			fMustTouchXML = false;
			fSomeErrorsInCatalog = false;
			fLoadingContext = nil;
			fLoadingGlobalObject = nil;
			fParseUserDefined = false;
			fParseLineError = 0;
			fParseFileError = nil;
			fPublishDataClassesAsGlobals = false;
			fPublishDataClassesAsGlobalsDefined = false;
			fill(&fForced[DB4D_EM_None_Perm], &fForced[DB4D_EM_Promote_Perm+1], 0);
		}

		Base4D* GetOwner()
		{
			return fOwner;
		}

		const Base4D* GetOwner() const
		{
			return fOwner;
		}

		void DisposeEntityModels();
		void DisposeEntityRelations();
		void DisposeEntityTypes();
		void DisposeEnumerations();

		VError LoadEntityModels(const VValueBag& bagEntities, bool devMode, unordered_map_VString<VRefPtr<VFile> >* outIncludedFiles = nil);
		VError LoadEntityModels(const VFile& inFile, bool inXML = true, bool devMode = false, const VString* inXmlContent = nil, unordered_map_VString<VRefPtr<VFile> >* outIncludedFiles = nil);

		VError SaveEntityModels(VValueBag& catalogBag, bool inXML = true, bool withDBInfo = false) const;
		VError SaveEntityModels(VFile& inFile, bool inXML = true, bool withDBInfo = false) const;

		VError AddOneEntityModel(EntityModel* newEntity, bool canReplace);
		EntityModel* BuildEntityModel(const VValueBag* bag, VError& err, bool devMode);
		EntityModel* FindEntityByCollectionName(const VString& entityName) const;
		EntityModel* FindEntity(const VString& entityName) const;
		EntityModel* RetainEntityByCollectionName(const VString& entityName) const;
		EntityModel* RetainEntity(const VString& entityName) const;

		VError AddOneEntityRelation(EntityRelation* newRelation, bool canReplace);
		//EntityRelation* BuildEntityRelation(const VValueBag* bag, VError& err);
		//EntityRelation* FindRelation(const VString& relationName) const;

		VError AddOneType(AttributeType* newType, bool canReplace);
		AttributeType* BuildType(const VValueBag* bag, VError& err, bool devMode);
		AttributeType* FindType(const VString& typeName) const;

		VError AddOneEnumeration(EmEnum* newEnum, bool canReplace);
		EmEnum* BuildEnumeration(const VValueBag* bag, VError& err, bool devMode);
		EmEnum* FindEnumeration(const VString& enumName) const;

		sLONG CountEntityModels() const
		{
			return (sLONG)fEntityModels.size();
		}

		VError GetAllEntityModels(vector<CDB4DEntityModel*>& outList, CDB4DBaseContext* context) const;

		VError LoadEntityPermissions(const VValueBag& bagEntities, CUAGDirectory* inDirectory, bool devMode);
		VError LoadEntityPermissions(const VFile& inFile, CUAGDirectory* inDirectory, bool inXML = true, bool devMode = false);

		EntityRelation* FindOrBuildRelation(const EntityRelationCollection& relpath, bool nTo1);
		void ReversePath(const EntityRelationCollection& relpath, EntityRelationCollection& outReversePath);

		void TouchXML()
		{
			fMustTouchXML = true;
		}

		void ClearTouchXML()
		{
			fMustTouchXML = false;
		}

		bool IsXMLTouched() const
		{
			return fMustTouchXML;
		}

		void ClearErrors()
		{
			fSomeErrorsInCatalog = false;
			fParseUserDefined = false;
			fParseLineError = 0;
			ReleaseRefCountable(&fParseFileError);
			fParseMessageError.Clear();
		}

		void AddError(const VString& modelName, const VString& attributeName, sLONG messageNum)
		{
			fSomeErrorsInCatalog = true;
		}

		void AddError(const VString& typeName, sLONG messageNum)
		{
			fSomeErrorsInCatalog = true;
		}

		void AddError()
		{
			fSomeErrorsInCatalog = true;
		}

		bool someErrors() const
		{
			return fSomeErrorsInCatalog;
		}

		VJSContext* getLoadingContext()
		{
			return fLoadingContext;
		}

		VJSObject* getLoadingGlobalObject()
		{
			return fLoadingGlobalObject;
		}

		bool AllowParseUsedDefined() const
		{
			return fParseUserDefined;
		}

		bool ParsingJSError(VFile* &outRetainedFile, VString& outMessage, sLONG& outLineNumber)
		{
			outRetainedFile = RetainRefCountable(fParseFileError);
			outMessage = fParseMessageError;
			outLineNumber = fParseLineError;
			return (fParseFileError != nil);
		}

		VError SetPermission(DB4D_EM_Perm inPerm, const VUUID& inGroupID, bool forced);
		VError GetPermission(DB4D_EM_Perm inPerm, VUUID& outGroupID, bool& forced) const;

		bool publishDataClassesAsGlobals() const
		{
			return fPublishDataClassesAsGlobals;
		}

	protected:

		virtual ~EntityModelCatalog()
		{
			QuickReleaseRefCountable(fExtraProperties);
			DisposeEntityModels();
			DisposeEntityRelations();
			DisposeEntityTypes();
			DisposeEnumerations();
			QuickReleaseRefCountable(fParseFileError);
			//EntityModel::ClearCacheTableEM();
		}

		VError xSetEntityPerm(CUAGDirectory* uagdir, const VValueBag::StKey key, const VValueBag* OnePerm, EntityModel* em, DB4D_EM_Perm perm);

		Base4D* fOwner;
		const VValueBag* fExtraProperties;
		EntityModelMap fEntityModels;
		EntityModelMap fEntityModelsByCollectionName;
		//EntityRelationMap fEntityRelations;
		EntityRelationCollection fRelations;
		AttributeTypeMap fTypes;
		EmEnumMap fEnumerations;
		mutable VCriticalSection fEntityModelMutex;
		mutable VJSContext* fLoadingContext;
		mutable VJSObject* fLoadingGlobalObject;
		bool fMustTouchXML, fSomeErrorsInCatalog, fParseUserDefined, fPublishDataClassesAsGlobals, fPublishDataClassesAsGlobalsDefined;
		sLONG fParseLineError;
		VString fParseMessageError;
		VFile* fParseFileError;
		VUUID fPerms[DB4D_EM_Promote_Perm+1];
		uBYTE fForced[DB4D_EM_Promote_Perm+1];

};




#endif

