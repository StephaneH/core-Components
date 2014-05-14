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
const VErrorDB4D	VE_DB4D_ENTITY_CANNOT_BE_DELETED = MAKE_VERROR(CDB4DManager::Component_Type, 1518);

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

const VErrorDB4D	VE_DB4D_ATTRIBUTE_CANNOT_BE_USED_FOR_SORTING = MAKE_VERROR(CDB4DManager::Component_Type, 1594);

const VErrorDB4D	VE_DB4D_CANNOT_CREATE_A_REMOTE_CLASS = MAKE_VERROR(CDB4DManager::Component_Type, 1595);

const VErrorDB4D	VE_DB4D_CANNOT_USE_ADD_REMOTE = MAKE_VERROR(CDB4DManager::Component_Type, 1596);

const VErrorDB4D	VE_DB4D_MISSING_CATALOG_LOG_FILE = MAKE_VERROR(CDB4DManager::Component_Type, 1597);

const VErrorDB4D	VE_DB4D_ENTITY_IS_NOT_OF_THE_RIGHT_MODEL = MAKE_VERROR(CDB4DManager::Component_Type, 1600);
const VErrorDB4D	VE_DB4D_QUERY_NOT_THE_RIGHT_MODEL = MAKE_VERROR(CDB4DManager::Component_Type, 1601);

const VErrorDB4D	VE_DB4D_INVALID_COLLECTION_IN_RESTRICTING_EVENT = MAKE_VERROR(CDB4DManager::Component_Type, 1602);
const VErrorDB4D	VE_DB4D_ENTITY_COLLECTION_IS_NOT_OF_THE_RIGHT_MODEL = MAKE_VERROR(CDB4DManager::Component_Type, 1603);

const VErrorDB4D	VE_DB4D_MISSING_SQL_CONNECTOR = MAKE_VERROR(CDB4DManager::Component_Type, 1604);
const VErrorDB4D	VE_DB4D_CANNOT_ESTABLISH_SQL_CONNECTION = MAKE_VERROR(CDB4DManager::Component_Type, 1605);

const VErrorDB4D	VE_DB4D_NOT_A_LOCALENTITYMODEL = MAKE_VERROR(CDB4DManager::Component_Type, 1606);

const VErrorDB4D	VE_DB4D_NO_PERM_TO_READ_ATTRIBUTE_VALUE = MAKE_VERROR(CDB4DManager::Component_Type, 1607);
const VErrorDB4D	VE_DB4D_NO_PERM_TO_UPDATE_ATTRIBUTE_VALUE = MAKE_VERROR(CDB4DManager::Component_Type, 1608);

const VErrorDB4D	VE_DB4D_NO_PERM_TO_DESCRIBE = MAKE_VERROR(CDB4DManager::Component_Type, 1609);

const VErrorDB4D	VE_DB4D_A_REMOTE_REQUEST_HAS_FAILED = MAKE_VERROR(CDB4DManager::Component_Type, 1610);

const VErrorDB4D	VE_DB4D_GETINFO_PERM = MAKE_VERROR(CDB4DManager::Component_Type, 1611);

const sLONG kDefaultTopSize = 100;

const UniChar kEntityTablePrefixChar = '$';
const VString kEntityTablePrefixString = L"$";

ENTITY_API extern const Enumeration EattTypes;

ENTITY_API extern const Enumeration ScriptTypes;

typedef enum { script_none, script_db4d, script_javascript, script_4d } script_type;

ENTITY_API extern const Enumeration ERelTypes;

ENTITY_API extern const Enumeration EmethTypes;

ENTITY_API extern const Enumeration EValPredefinedTypes;

ENTITY_API extern const Enumeration EFormulaeActions;

ENTITY_API extern const Enumeration EIndexKinds;

ENTITY_API extern const Enumeration EScopeKinds;

ENTITY_API extern const Enumeration EPermResourceType;

typedef enum 
{ 
	perm_none = 0,
	perm_model,
	perm_dataClass, 
	perm_method,
	perm_attribute
} PermResourceType;


ENTITY_API extern const Enumeration EPermAction;

ENTITY_API extern const Enumeration OldEDBEventKinds;

ENTITY_API extern const Enumeration EDBEventKinds;

ENTITY_API extern const Enumeration EAttributeProperty;

typedef enum
{
	attprop_none,
	attprop_path
} AttributeProperty;


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


ENTITY_API extern const Correspondance CatCorres;

ENTITY_API extern const Enumeration ERechToken;


									// ---------------------------------------------


class EntityModelCatalog;
class RestTools;

class ENTITY_API ModelErrorReporter
{
	public:
		ModelErrorReporter(EntityModelCatalog* inCatalog);

		EntityModelCatalog* GetCatalog()
		{
			return fCatalog;
		}

		void AddError(VErrorBase* error);
		const VErrorStack& GetErrors()
		{
			return fErrors;
		}

	protected:
		EntityModelCatalog* fCatalog;
		VErrorStack fErrors;
};


class ENTITY_API ModelErrorContextInstaller
{
	public:
		ModelErrorContextInstaller(ModelErrorReporter* reporter);
		~ModelErrorContextInstaller();

	protected:
		ModelErrorReporter* fErrorReporter;
		StErrorContextInstaller fErrors;
};


class ENTITY_API EmEnum : public IRefCountable
{
	public:

		inline EmEnum(EntityModelCatalog* owner)
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

		inline EntityModelCatalog* GetOwner() const
		{
			return fOwner;
		}

		VError ThrowError( VError inErrCode, const VString* p1 = nil) const;

		VError FromBag(const VValueBag* bag, EntityModelCatalog* catalog, ModelErrorReporter* errorReporter);
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

		EntityModelCatalog* fOwner;
		VString fName;
		EnumMap fEnums;
		EnumIDMap fEnumsID;
		const VValueBag* fExtraProperties;
		

};


									// ---------------------------------------------


class ENTITY_API AttributeType : public IDebugRefCountable
{
	public:
		typedef list<AttributeType*> ListOfTypes;

		AttributeType(EntityModelCatalog* owner);
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

		inline EntityModelCatalog* GetOwner() const
		{
			return fOwner;
		}

		VError FromBag(const VValueBag* bag, EntityModelCatalog* catalog, bool fullyLoad, ModelErrorReporter* errorReporter);
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
		EntityModelCatalog* fOwner;
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


class ENTITY_API EntityRelation : public IDebugRefCountable
{
	public:

		inline EntityRelation(const EntityAttribute* source, const EntityAttribute* dest, EntityRelationKind kind)
		{
			fSourceAtt = source;
			fDestAtt = dest;
			fType = kind;
			xbox_assert(fSourceAtt != nil);
			xbox_assert(fDestAtt != nil);
		}

		inline EntityRelation()
		{
			fSourceAtt = nil;
			fDestAtt = nil;
			fType = erel_none;
		}

		inline EntityRelation(const EntityRelationCollection& path, bool nTo1);

		VError ThrowError( VError inErrCode, const VString* p1 = nil) const;

		inline bool operator == (const EntityRelation& other) const
		{
			return (fSourceAtt == other.fSourceAtt) && (fDestAtt == other.fDestAtt) && (fType == other.fType);
		}

		inline const VString& GetName() const
		{
			return fName;
		}

		/*
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
		*/

		const EntityAttribute* GetSourceAtt() const
		{
			return fSourceAtt;
		}

		const EntityAttribute* GetDestAtt() const
		{
			return fDestAtt;
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
		//Field* fSource;
		//Field* fDest;
		const EntityAttribute* fSourceAtt;
		const EntityAttribute* fDestAtt;
		EntityRelationKind fType;
		EntityRelationCollection fSubPath;
		EntityRelationCollection fPath;
		VString fMissingSourceTable, fMissingSourceField, fMissingDestTable, fMissingDestField;
};



class EntityModel;
class OptimizedQuery;
class EntityRecord;



									// ---------------------------------------------


class EntityCollectionIterator;
class WafSelection;

class ENTITY_API EntityCollection  : public CDB4DEntityCollection
{
	public:
		EntityCollection(EntityModel* inOwner)
		{
			fOwner = inOwner;
			fQueryPlan = nil;
			fQueryPath = nil;
		}

		virtual VError AddEntity(EntityRecord* entity, bool atTheEnd = false) = 0;

		virtual VError AddCollection(EntityCollection* other, BaseTaskInfo* context, bool atTheEnd = false);

		virtual CDB4DEntityRecord* LoadEntityRecord(RecIDType posInCol, CDB4DBaseContext* context, VErrorDB4D& outError, DB4D_Way_of_Locking HowToLock = DB4D_Do_Not_Lock);

		virtual EntityRecord* LoadEntity(RecIDType posInCol, BaseTaskInfo* context, VError& outError, DB4D_Way_of_Locking HowToLock = DB4D_Do_Not_Lock) = 0;
		virtual RecIDType GetLength(BaseTaskInfo* context) = 0;

		virtual RecIDType GetLength(CDB4DBaseContext* context);

		virtual bool IsSorted() const = 0;
		virtual bool IsSafeRef() const = 0;

		virtual VError GetDistinctValues(EntityAttribute* att, DB4DCollectionManager& outCollection, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, const VCompareOptions& inOptions) = 0;
		
		virtual EntityCollection* ProjectCollection(EntityAttribute* att, VError& err, BaseTaskInfo* context) = 0;
		virtual VJSValue ProjectAttribute(EntityAttribute* att, VError& err, BaseTaskInfo* context, JS4D::ContextRef jscontext);

		virtual EntityCollection* SortCollection(const VString& orderby, BaseTaskInfo* context, VError err, VDB4DProgressIndicator* InProgress);

		virtual VJSArray ToJsArray(BaseTaskInfo* context, JS4D::ContextRef jscontext, EntityAttributeSortedSelection& whichAttributes, EntityAttributeSelection* expandAttributes, 
									EntityAttributeSortedSelection* sortingAttributes, bool withKey, bool allowEmptyAttList, sLONG from, sLONG count, VError& err, EntityCollection* withinCollection);

		virtual VError Compute(EntityAttributeSortedSelection& atts, VJSObject& outObj, BaseTaskInfo* context, JS4D::ContextRef jscontext, bool distinct);
		virtual VError ComputeOnOneAttribute(const EntityAttribute* att, DB4D_ColumnFormulae action, VJSValue& outVal, BaseTaskInfo* context, JS4D::ContextRef jscontext);

		virtual VError DropEntities(BaseTaskInfo* context, VDB4DProgressIndicator* InProgress = nil, EntityCollection* *outLocked = nil) = 0;
		virtual VError DeleteEntities(CDB4DBaseContext* context, VDB4DProgressIndicator* InProgress = nil, CDB4DEntityCollection* *outLocked = nil);

		virtual EntityCollection* SortSel(VError& err, EntityAttributeSortedSelection* sortingAtt, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress = nil, WafSelection* inWafSel = nil, WafSelection* outWafSel = nil) = 0;

		virtual RecIDType NextNotNull(BaseTaskInfo* context, RecIDType startFrom) = 0;

		virtual EntityCollection* And(EntityCollection* other, VError& err, BaseTaskInfo* context) = 0;
		virtual EntityCollection* Or(EntityCollection* other, VError& err, BaseTaskInfo* context) = 0;
		virtual EntityCollection* Minus(EntityCollection* other, VError& err, BaseTaskInfo* context) = 0;
		virtual bool Intersect(EntityCollection* other, VError& err, BaseTaskInfo* context) = 0;

		virtual EntityCollectionIterator* NewIterator(BaseTaskInfo* context);

		virtual VError RemoveSelectedRange(sLONG inRecordIndex1, sLONG inRecordIndex2, BaseTaskInfo* context) = 0;

		virtual EntityCollection* NewFromWafSelection(WafSelection* wafSel, BaseTaskInfo* context) = 0;

		virtual RecIDType FindKey(VectorOfVString& vals, BaseTaskInfo* context, VError& err) = 0;

		virtual VError Refresh(BaseTaskInfo* context)
		{
			return VE_OK; // will be overridden for remote collections
		}

		inline EntityModel* GetModel() const
		{
			return fOwner;
		}

		void SetQueryPlan(VValueBag* inQueryPlan)
		{
			CopyRefCountable(&fQueryPlan, inQueryPlan);
		}

		void SetQueryPath(VValueBag* inQuerypath)
		{
			CopyRefCountable(&fQueryPath, inQuerypath);
		}

		VValueBag* GetQueryPlan() const
		{
			return fQueryPlan;
		}

		VValueBag* GetQueryPath() const
		{
			return fQueryPath;
		}

		virtual bool MatchAllocationNumber(sLONG allocationNumber) = 0;
		virtual size_t CalcLenOnDisk(void) = 0;


	protected:
		virtual ~EntityCollection()
		{
			QuickReleaseRefCountable(fQueryPlan);
			QuickReleaseRefCountable(fQueryPath);
		}

		EntityModel* fOwner;
		VValueBag* fQueryPlan;
		VValueBag* fQueryPath;
};


class ENTITY_API EntityCollectionIterator : public IDebugRefCountable
{
	public:
		EntityCollectionIterator(EntityCollection* collection, BaseTaskInfo* context);

		virtual ~EntityCollectionIterator();

		virtual EntityRecord* First(VError& err);
		virtual EntityRecord* Next(VError& err);
		virtual RecIDType GetCurPos() const;
		virtual EntityRecord* SetCurPos(RecIDType pos, VError& err);

	protected:
		EntityRecord* _loadEntity(RecIDType atPos, VError& err);

		EntityCollection* fCollection;
		RecIDType fCurpos, fNbElems;
		BaseTaskInfo* fContext;

};


class ENTITY_API EntityCollectionIter
{
	public:

		inline EntityCollectionIter(EntityCollection* collection, BaseTaskInfo* context)
		{
			fIter = collection->NewIterator(context);
		}

		inline ~EntityCollectionIter()
		{
			fIter->Release();
		}

		inline EntityRecord* First(VError& err)
		{
			return fIter->First(err);
		}

		inline EntityRecord* Next(VError& err)
		{
			return fIter->Next(err);
		}

		inline RecIDType GetCurPos() const
		{
			return fIter->GetCurPos();
		}

		inline EntityRecord* SetCurPos(RecIDType pos, VError& err)
		{
			return fIter->SetCurPos(pos, err);
		}


	protected:
		EntityCollectionIterator* fIter;
};



									// ---------------------------------------------



class ENTITY_API EntityMethod : public CDB4DEntityMethod
{
	public:

		EntityMethod(EntityModel* owner);

		EntityMethod(EntityModel* owner, const VString& name, EntityMethodKind kind); // to build from 4D method definition

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

		VError FromBag(const VValueBag* bag, EntityModelCatalog* catalog, ModelErrorReporter* errorReporter);

		VError FromJS(const VString& name, EntityMethodKind kind, const VString& from, EntityAttributeScope scope, EntityModelCatalog* catalog, ModelErrorReporter* errorReporter);

		VError ToBag(VValueBag& outBag, bool forDax, bool forSave, bool forJSON) const;

		virtual EntityMethod* Clone(EntityModel* inModel) const;
		void BaseClone(EntityMethod* result) const;

		virtual bool GetScriptObjFunc(VJSGlobalContext* jsglobcontext, BaseTaskInfo* context, VJSObject& outObjFunc) const;

		VError Execute(EntityCollection* inSel, const vector<VJSValue>* inParams, BaseTaskInfo* context, VJSValue& outResult) const;

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

		VError ResolveType(EntityModelCatalog* catalog, ModelErrorReporter* errorReporter);

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

		bool Is4DMethod() const
		{
			return fIs4DMethod;
		}

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
		bool fOverWrite, fReturnTypeIsSelection, fUserDefined, fIs4DMethod;
};

									// ---------------------------------------------




class ENTITY_API DBEvent
{
	public:
		DBEvent();
		//~DBEvent();

		VError FromBag(EntityModel* inOwner, const VValueBag* bag, EntityModelCatalog* catalog, ModelErrorReporter* errorReporter);
		VError ToBag(VValueBag& outBag, bool forDax, bool forSave, bool forJSON, bool firstlevel) const;

		VError SetEvent(EntityModel* inOwner, DBEventKind eventKind, const VString& from, bool oldWay)
		{
			fOwner = inOwner;
			fKind = eventKind;
			fFrom = from;
			fUserDefined = true;
			fOldWay = oldWay;
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

		VError Call(EntityRecord* inRec, BaseTaskInfo* context, const EntityAttribute* inAtt, const EntityModel* inDataClass, EntityCollection* *outSel = nil) const;

		DBEventKind GetKind() const
		{
			return fKind;
		}


	protected:
		DBEventKind fKind;
		EntityModel* fOwner;
		VString fFrom;
		bool fOverWrite, fUserDefined, fOldWay;

};


typedef DBEvent* DBEventIterator;
typedef const DBEvent* DBEventConstIterator;


class ENTITY_API DBEventMap : public multimap<DBEventKind, DBEvent>
{
	public:
		void AddEvent(const DBEvent& event);

		VError CallEvent(DBEventKind eventkind, EntityRecord* inRec, BaseTaskInfo* context, const EntityAttribute* inAtt, const EntityModel* inDataClass, EntityCollection* *outSel = nil) const;

		VError CopyFrom(EntityModel* inOwner, const DBEventMap& from);
		VError ToBag(VValueBag& outBag, bool forDax, bool forSave, bool forJSON, bool firstlevel) const;

		bool HasEvent(DBEventKind eventkind) const
		{
			return (count(eventkind) != 0);
		}

	protected:

};



									// ---------------------------------------------


class EntityModel;

typedef pair<EntityModel*, VString> SubPathRef;
typedef set<SubPathRef> SubPathRefSet;



typedef enum { script_attr_get = 0, script_attr_set, script_attr_query, script_attr_sort, script_attr_last = script_attr_sort, script_attr_none } script_attr;

class ENTITY_API EntityAttribute : public CDB4DEntityAttribute
{
	public:
		void xinit();

		EntityAttribute(EntityModel* owner);

		virtual ~EntityAttribute();

		VError ThrowError( VError inErrCode, const VString* p1 = nil) const;

		inline EntityModel* GetOwner() const
		{
			return fOwner;
		}

		inline EntityModel* GetModel() const
		{
			return fOwner;
		}

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

		/*
		virtual sLONG GetPosInModel() const
		{
			return fPosInOwner;
		}
		*/

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

		inline const VString& getFlattenAttributeName() const
		{
			return fFlattenColumnName;
		}

		inline EntityModel* getFlattenLastDest() const
		{
			return fFlattenLastDest;
		}

		EntityAttribute* getFlattenAttribute() const;

		EntityRelation* GetRelPath() const
		{
			return fRelation;
		}

		bool isSimpleDate() const
		{
			return fSimpleDate && (ComputeScalarType() == VK_TIME);
		}

		void SetReversePath(bool b)
		{
			fReversePath = b;
		}


		VError GetScriptFromBag(const VValueBag* bag, EntityModelCatalog* catalog, const VValueBag::StKey& inScriptKey, script_attr inWhatScript, ModelErrorReporter* errorReporter);
		VError FromBag(const VValueBag* bag, EntityModelCatalog* catalog, bool fieldmustexits, ModelErrorReporter* errorReporter, bool limitedChange);

		VError ScriptToBag(VValueBag& outBag, const VValueBag::StKey& inWhatScript, const VString& inStatement, const VString& inFrom, bool userDefined) const;
		VError ToBag(VValueBag& outBag, bool forDax, bool forSave, bool forJSON, bool isTableDef) const;

		VError ResolveRelatedEntities(SubPathRefSet& already, EntityModelCatalog* catalog, ModelErrorReporter* errorReporter, BaseTaskInfo* context);
		VError ResolveQueryStatements(EntityModelCatalog* catalog, ModelErrorReporter* errorReporter, BaseTaskInfo* context);

		virtual EntityAttribute* Clone(EntityModel* inModel) const;
		void BaseClone(EntityAttribute* result) const;

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
			if (fBehavesAsStorage)
				return fCanBeModified;
			else if (fKind == eattr_alias)
				return false;
			else if (fKind == eattr_computedField)
			{
				return !fScriptStatement[script_attr_set].IsEmpty() || !fScriptFrom[script_attr_set].IsEmpty();
			}
			else
				return fCanBeModified;
		}

#if BuildEmFromTable
		void SetRelation(Relation* rel, EntityRelationKind kind, LocalEntityModelCatalog* catalog);
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

		inline const VString& GetIndexKind() const
		{
			return fIndexKind;
		}

		inline void SetIndexKind(const VString& indexkind)
		{
			if (fIndexKind.IsEmpty())
				fIndexKind = indexkind;
		}

		inline void SetScalarType(sLONG typ)
		{
			fScalarType = typ;
		}

		inline void SetSimpleDate(bool b)
		{
			fSimpleDate = b;
		}

		bool IsIndexed() const;
		bool IsFullTextIndexed() const;

		bool IsIndexable() const;
		bool IsFullTextIndexable() const;

		inline void SetPartOfPrimKey()
		{
			fPartOfPrimKey = true;
		}

		VError CallDBEvent(DBEventKind kind, EntityRecord* inRec, BaseTaskInfo* context, bool includesInherited, const EntityAttribute* realAtt = nil) const;
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

		virtual CDB4DEntityModel* GetRelatedEntityModel() const;

		bool BehavesAsStorage() const
		{
			return fBehavesAsStorage;
		}

		bool IsAutoGen() const
		{
			return fAutoGen;
		}

		bool IsAutoSeq() const
		{
			return fAutoSeq;
		}

		bool IsForeignKey() const
		{
			return fIsForeignKey;
		}

		bool RelIsNotOnPrimKey() const
		{
			return fRelIsNotOnPrimKey;
		}

		virtual VError SetPermission(DB4D_EM_Perm inPerm, const VUUID& inGroupID);
		virtual VError GetPermission(DB4D_EM_Perm inPerm, VUUID& outGroupID) const;
		virtual bool PermissionMatch(DB4D_EM_Perm inPerm, CUAGSession* inSession) const;
		bool permissionMatch(DB4D_EM_Perm inPerm, CUAGSession* inSession) const;
		bool permissionMatch(DB4D_EM_Perm inPerm, BaseTaskInfo* context) const;

		void ResolvePermissionsInheritance();

		sLONG GetCacheDuration() const
		{
			return fPictCacheDuration;
		}

		const VUUID& GetUUID() const
		{
			return fDB4DUUID;
		}

		const VectorOfVString& GetAlternateFieldNames() const
		{
			return fAlternateFieldNames;
		}

	protected:
		VString fName;
		EntityAttributeKind fKind;
		EntityAttributeScope fScope;
		sLONG fRelationPathID;  // un indice dans EntityModel.fRelationPaths et dans EntityRecord.fSubEntitiesCache
		EntityRelation* fRelation;
		const EntityAttribute* fFrom;
		VString fFlattenColumnName;
		VUUID fDB4DUUID;
		VectorOfVString fAlternateFieldNames;
		EntityModel* fFlattenLastDest;
		sLONG fPosInOwner;
		sLONG fFieldPos;
		EntityModel* fOwner;
		VString fRelPath;
		VString fSubEntityName;
		VString fIndexKind;
		//VUUID fCrit_UUID;
		EntityModel* fSubEntity;
		AttributeType* fType;
		AttributeType* fLocalType;
		const VValueBag* fExtraProperties;
		//DBEvent fEvents[dbev_lastEvent+1];
		DBEventMap fDBEvents;
		VString fScriptStatement[script_attr_last+1];
		VString fScriptFrom[script_attr_last+1];
		uBOOL fScriptUserDefined[script_attr_last+1];
		SearchTab* fScriptDB4D;
		SearchTab* fScriptQuery;
		script_type fScriptKind;
		sLONG fScalarType;
		bool fOverWrite,fCanBeModified,fReversePath,fIsForeignKey,fIdentifying,fPartOfPrimKey, fIsMultiLine, fRelIsNotOnPrimKey;
		bool fAutoGen, fAutoSeq, fStyledText, fOuterBlob, fNotNull, fNullToEmpty, fUnique, fBehavesAsStorage, fSimpleDate;
		sLONG fLimitingLen, fBlobSwitchSize;
		sLONG fPictCacheDuration; // in seconds
		mutable OptimizedQuery* fOptimizedScriptDB4D;
		mutable sLONG fScriptNum[script_attr_last+1];
		VUUID fPerms[DB4D_EM_Delete_Perm];


};



									// ---------------------------------------------

class ENTITY_API EntityAttributeInstance
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


template<>
const VString* IPartable<VectorOfVString, const VString*>::GetElem(VectorOfVString::const_iterator iter) const;

class AttributePath : public IPartable<EntityAttributeInstanceCollection, const EntityAttributeInstance*>, public IDebugRefCountable
{
	public:
		AttributePath()
		{
			fIsValid = false;
		}

		AttributePath(EntityModel* model, const VString& inPath);
		AttributePath(EntityModel* model, const AttributeStringPath& inPath, bool fromstart = true, VString* outRemain = nil);

		bool FromPath(EntityModel* model, const VString& inPath, bool expandAliases = false, VString* outRemain = nil);
		bool FromPath(EntityModel* model, const AttributeStringPath& inPath, bool expandAliases = false, bool fromstart = true, VString* outRemain = nil);

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
const EntityAttributeInstance* IPartable<EntityAttributeInstanceCollection, const EntityAttributeInstance*>::GetElem(EntityAttributeInstanceCollection::const_iterator iter) const;



class EntityAttributeSortedSelection;
class EntityAttributeSelection;

class ENTITY_API EntityAttributeItem
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


class ENTITY_API EntityAttributeSelection : public vector<EntityAttributeItem>  // le vector est toujours dimensionne au nombre d'attributs du model et les non-trous representent les attributs selectionnes
{
	public:

		EntityAttributeSelection(EntityModel* inModel);

		EntityAttributeSelection(EntityModel* inModel, const VString& inListOfAtttibutes, bool FirstLevelOnly = false, RestTools* req = nil);

		bool BuildFromString(const VString& inListOfAtttibutes, bool FirstLevelOnly, RestTools* req);

		bool AddAttribute(const VString& inAttributePath, RestTools* req);
		bool AddAttribute(const AttributeStringPath& inAttributePath, RestTools* req);

		void GetString(VString& outString, bool FirstLevelOnly, const VString* prefix = nil);

	protected:
		EntityModel* fModel;
		void Dispose();

};




class ENTITY_API EntityAttributeSortedItem
{
public:
	inline EntityAttributeSortedItem()
	{
		fAttribute = nil;
		fSousSelection = nil;
		fAscent = true;
		fAttPath = nil;
	}

	inline EntityAttributeSortedItem(const EntityAttribute* inAttribute, bool ascent = true)
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

	const EntityAttribute* fAttribute;
	AttributePath* fAttPath;
	EntityAttributeSortedSelection* fSousSelection;
	bool fAscent;
};


class ENTITY_API EntityAttributeSortedSelection : public vector<EntityAttributeSortedItem>
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

		bool AddAttribute(const EntityAttribute* att, RestTools* req);

		EntityAttributeSortedSelection::iterator FindAttribute(const EntityAttribute* att);
		EntityAttributeSortedSelection::const_iterator FindAttribute(const EntityAttribute* att) const;

		EntityAttributeSortedSelection* FindSubSelection(const EntityAttribute* att) const;

		void ToString(VString& outString);
		void GetString(VString& outString, bool FirstLevelOnly, bool forSorting, const VString* prefix = nil);


	protected:
		EntityModel* fModel;
		void Dispose();
		

};


class ENTITY_API IdentifyingAttribute
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


class ENTITY_API computeResult
{
public:
	computeResult()
	{
		fSum = 0;
		first = true;
		fMinVal = nil;
		fMaxVal = nil;
		fPrevious = nil;
		fCount = 0;
		fSumDistinct = 0;
		fCountDistinct = 0;
	}

	~computeResult()
	{
		if (fMinVal != nil)
			delete fMinVal;
		if (fMaxVal != nil)
			delete fMaxVal;
		if (fPrevious != nil)
			delete fPrevious;
	}

	VValueSingle* fMinVal;
	VValueSingle* fMaxVal;
	VValueSingle* fPrevious;
	Real fSum;
	sLONG8 fCount;
	Real fSumDistinct;
	sLONG8 fCountDistinct;
	bool first;
};




									// ---------------------------------------------


class RestTools;

class EntityModelCatalog;

class ENTITY_API EntityModel : public CDB4DEntityModel
{
	public:

		typedef list<EntityModel*> ListOfModels;

		EntityModel(EntityModelCatalog* inOwner);

		VError ThrowError( VError inErrCode, const VString* p1 = nil) const;

		virtual bool IsValid() const = 0;

		virtual sLONG CountAttributes() const
		{
			return (sLONG)fAttributes.size();
		}

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

		const VString& GetBaseModelName() const  // always to first root : applied on C, with A --> B --> C, would return A
		{
			return fBaseModelName;
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

		VError BuildRelPath(SubPathRefSet& already, EntityModelCatalog* catalog, const VectorOfVString& path, EntityRelationCollection& outRelPath, VString& outLastpart, bool& outAllNto1, ModelErrorReporter* errorReporter, EntityModel* &outLastDest);
		VError ActivatePath(EntityRecord* erec, sLONG inPathID, SubEntityCache& outResult, bool Nto1, EntityModel* subEntityModel, BaseTaskInfo* context);

		EntityAttribute* getAttribute(sLONG pos) const;
		EntityAttribute* getAttribute(const VString& AttributeName) const;

		CDB4DEntityAttribute* GetAttribute(const XBOX::VString& AttributeName) const
		{
			return getAttribute(AttributeName);
		}

		virtual CDB4DEntityAttribute* GetAttribute(sLONG pos) const
		{
			return getAttribute(pos);
		}

		//EntityAttribute* FindAttributeByFieldPos(sLONG FieldPos);

		VError ExtendsFrom(EntityModel* otherEm);

		VError ToBag(VValueBag& outBag, bool forDax, bool forSave, bool forJSON, RestTools* req = nil) const;
		VError FromBag(const VValueBag* inBag, EntityModelCatalog* catalog, ModelErrorReporter* errorReporter, bool addToClassOnly, bool allowAttributeChange);
		VError addJSExtensions( EntityModelCatalog* catalog, ModelErrorReporter* errorReporter, bool addToClassOnly, bool allowAttributeChange);


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

		VError ResolveRelatedEntities(SubPathRefSet& already, EntityModelCatalog* catalog, ModelErrorReporter* errorReporter, BaseTaskInfo* context);
		VError ResolveQueryStatements(EntityModelCatalog* catalog, ModelErrorReporter* errorReporter, BaseTaskInfo* context);
		//VError ResolveRelatedPath(EntityModelCatalog* catalog);

		//sLONG FindAttribute(const VString& AttributeName) const;

		bool GetAllSortedAttributes(EntityAttributeSortedSelection& outAtts, RestTools* req) const;
		bool BuildListOfSortedAttributes(const VString& inAttributeList, EntityAttributeSortedSelection& outAtts, BaseTaskInfo* context, bool FirstLevelOnly, bool forSorting, RestTools* req) const;

		bool GetAllAttributes(EntityAttributeSelection& outAtts, RestTools* req) const;
		bool BuildListOfAttributes(const VString& inAttributeList, EntityAttributeSelection& outAtts, bool FirstLevelOnly, RestTools* req) const;


		EntityAttribute* GetAttributeByPath(const VString& inPath) const;
		EntityAttribute* GetAttributeByPath(const AttributeStringPath& inPath) const;

		virtual RecIDType countEntities(BaseTaskInfo* inContext) = 0;

		virtual RecIDType CountEntities(CDB4DBaseContext* inContext);

		virtual bool QueriesAreProcessedRemotely() const
		{
			return false;
		}

		virtual EntityCollection* SelectAllEntities(BaseTaskInfo* context, VErrorDB4D* outErr = NULL, 
										DB4D_Way_of_Locking HowToLock = DB4D_Do_Not_Lock, EntityCollection* outLockSet = NULL, bool allowRestrict = true) = 0;
				
		virtual EntityCollection* executeQuery( SearchTab* querysearch, BaseTaskInfo* context, EntityCollection* filter = nil, 
									VDB4DProgressIndicator* InProgress = NULL, DB4D_Way_of_Locking HowToLock = DB4D_Do_Not_Lock, 
									sLONG limit = 0, EntityCollection* outLockSet = NULL, VError *outErr = NULL, EntityAttributeSortedSelection* sortingAtt = NULL) = 0;

		virtual EntityCollection* executeQuery( const VString& queryString, VJSParms_callStaticFunction& ioParms, BaseTaskInfo* context, EntityCollection* filter = nil, 
									VDB4DProgressIndicator* InProgress = NULL, DB4D_Way_of_Locking HowToLock = DB4D_Do_Not_Lock, 
									sLONG limit = 0, EntityCollection* outLockSet = NULL, VError *outErr = NULL) = 0;

		virtual EntityCollection* executeQuery( const VString& queryString, VJSONArray* params, BaseTaskInfo* context, EntityCollection* filter = nil, 
									VDB4DProgressIndicator* InProgress = NULL, DB4D_Way_of_Locking HowToLock = DB4D_Do_Not_Lock, 
									sLONG limit = 0, EntityCollection* outLockSet = NULL, VError *outErr = NULL) = 0;


		virtual EntityCollection* executeQuery( VJSObject queryObj, BaseTaskInfo* context, EntityCollection* filter = nil, 
									VDB4DProgressIndicator* InProgress = NULL, DB4D_Way_of_Locking HowToLock = DB4D_Do_Not_Lock, 
									sLONG limit = 0, EntityCollection* outLockSet = NULL, VError *outErr = NULL) = 0;

	
		virtual EntityCollection* NewCollection(bool ordered = false, bool safeRef = false) const = 0;
		virtual EntityCollection* NewCollection(const VectorOfVString& primKeys, VError& err, BaseTaskInfo* context) const;
		
		virtual EntityRecord* newEntity(BaseTaskInfo* inContext, bool forClone = false) const = 0;

		virtual EntityCollection* FromArray(VJSArray& arr, BaseTaskInfo* context, VError& err, VDB4DProgressIndicator* InProgress = nil);

		virtual CDB4DEntityRecord* NewEntity(CDB4DBaseContextPtr inContext) const;

		virtual CDB4DEntityCollection* NewSelection(bool ordered = false, bool safeRef = false) const;

		virtual CDB4DEntityCollection* SelectAllEntities(CDB4DBaseContextPtr inContext, VErrorDB4D* outErr = NULL, 
			DB4D_Way_of_Locking HowToLock = DB4D_Do_Not_Lock, CDB4DEntityCollection* outLockSet = NULL);

		virtual CDB4DEntityCollection* ExecuteQuery( CDB4DQuery *inQuery, CDB4DBaseContextPtr inContext, CDB4DEntityCollection* Filter = NULL, 
			VDB4DProgressIndicator* InProgress = NULL, DB4D_Way_of_Locking HowToLock = DB4D_Do_Not_Lock, 
			sLONG limit = 0, CDB4DEntityCollection* outLockSet = NULL, VErrorDB4D *outErr = NULL);

#if BuildEmFromTable
		static EntityModel* BuildLocalEntityModel(Table* from, LocalEntityModelCatalog* catalog);
#endif

		//static void ClearCacheTableEM();

		bool AddRestrictionToQuery(SearchTab& query, BaseTaskInfo* context, VError& err);
		EntityCollection* BuildRestrictingSelection(BaseTaskInfo* context, VError& err);

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

		inline const VString& GetBaseEmName() const  // always to previous root : applied on C, with A --> B --> C, would return B
		{
			return fExtends;
		}

		inline EntityModel* GetBaseEm() const
		{
			return fBaseEm;
		}

		const EntityModel* GetRootBaseEm() const
		{
			if (fBaseEm == nil)
				return this;
			else
				return fBaseEm->GetRootBaseEm();

		}

		EntityModel* GetRootBaseEm()
		{
			if (fBaseEm == nil)
				return this;
			else
				return fBaseEm->GetRootBaseEm();

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

		virtual EntityRecord* findEntityWithAttribute(const EntityAttribute* inAtt, const VValueSingle* value, BaseTaskInfo* context, VError& err, DB4D_Way_of_Locking HowToLock);

		virtual EntityRecord* findEntityWithPrimKey(const VString& primkey, BaseTaskInfo* context, VError& err, DB4D_Way_of_Locking HowToLock) = 0;
		virtual EntityRecord* findEntityWithPrimKey(const VectorOfVString& primkey, BaseTaskInfo* context, VError& err, DB4D_Way_of_Locking HowToLock) = 0;
		virtual EntityRecord* findEntityWithPrimKey(const VValueBag& primkey, BaseTaskInfo* context, VError& err, DB4D_Way_of_Locking HowToLock) = 0;
		virtual EntityRecord* findEntityWithPrimKey(const VectorOfVValue& primkey, BaseTaskInfo* context, VError& err, DB4D_Way_of_Locking HowToLock) = 0;
		virtual EntityRecord* findEntityWithPrimKey(VJSObject objkey, BaseTaskInfo* context, VError& err, DB4D_Way_of_Locking HowToLock) = 0;

		virtual EntityRecord* findEntityWithIdentifyingAtts(const VectorOfVString& idents, BaseTaskInfo* context, VError& err, DB4D_Way_of_Locking HowToLock) = 0;
		virtual EntityRecord* findEntityWithIdentifyingAtts(const VValueBag& idents, BaseTaskInfo* context, VError& err, DB4D_Way_of_Locking HowToLock) = 0;
		virtual EntityRecord* findEntityWithIdentifyingAtts(const VectorOfVValue& idents, BaseTaskInfo* context, VError& err, DB4D_Way_of_Locking HowToLock) = 0;


		virtual CDB4DEntityRecord* FindEntityWithPrimKey(const XBOX::VString& primkey, CDB4DBaseContext* inContext, VErrorDB4D& err, DB4D_Way_of_Locking HowToLock);
		virtual CDB4DEntityRecord* FindEntityWithPrimKey(const XBOX::VectorOfVString& primkey, CDB4DBaseContext* inContext, VErrorDB4D& err, DB4D_Way_of_Locking HowToLock);
		virtual CDB4DEntityRecord* FindEntityWithPrimKey(const XBOX::VValueBag& primkey, CDB4DBaseContext* inContext, VErrorDB4D& err, DB4D_Way_of_Locking HowToLock);
		virtual CDB4DEntityRecord* FindEntityWithPrimKey(const XBOX::VectorOfVValue& primkey, CDB4DBaseContext* inContext, VErrorDB4D& err, DB4D_Way_of_Locking HowToLock);

		virtual CDB4DEntityRecord* FindEntityWithIdentifyingAtts(const XBOX::VectorOfVString& idents, CDB4DBaseContext* inContext, VErrorDB4D& err, DB4D_Way_of_Locking HowToLock);
		virtual CDB4DEntityRecord* FindEntityWithIdentifyingAtts(const XBOX::VValueBag& idents, CDB4DBaseContext* inContext, VErrorDB4D& err, DB4D_Way_of_Locking HowToLock);
		virtual CDB4DEntityRecord* FindEntityWithIdentifyingAtts(const XBOX::VectorOfVValue& idents, CDB4DBaseContext* inContext, VErrorDB4D& err, DB4D_Way_of_Locking HowToLock);

		const VString& GetDefaultOrderBy() const
		{
			return fRestrictingOrderByString;
		}

		virtual CDB4DEntityCollection* Query( const XBOX::VString& inQuery, CDB4DBaseContext* inContext, VErrorDB4D& err, const XBOX::VValueSingle* param1 = nil, const XBOX::VValueSingle* param2 = nil, const XBOX::VValueSingle* param3 = nil);
		virtual CDB4DEntityRecord* Find(const XBOX::VString& inQuery, CDB4DBaseContext* inContext, VErrorDB4D& err, const XBOX::VValueSingle* param1 = nil, const XBOX::VValueSingle* param2 = nil, const XBOX::VValueSingle* param3 = nil);

		virtual EntityCollection* query( const VString& inQuery, BaseTaskInfo* context, VErrorDB4D& err, const VValueSingle* param1 = nil, const VValueSingle* param2 = nil, const VValueSingle* param3 = nil);
		virtual EntityRecord* find(const VString& inQuery, BaseTaskInfo* inContext, VErrorDB4D& err, const VValueSingle* param1 = nil, const VValueSingle* param2 = nil, const VValueSingle* param3 = nil);

		virtual VError SetPermission(DB4D_EM_Perm inPerm, const VUUID& inGroupID, bool forced);
		virtual VError GetPermission(DB4D_EM_Perm inPerm, VUUID& outGroupID, bool& forced) const;
		virtual bool PermissionMatch(DB4D_EM_Perm inPerm, CUAGSession* inSession) const;
		bool permissionMatch(DB4D_EM_Perm inPerm, CUAGSession* inSession) const;
		bool permissionMatch(DB4D_EM_Perm inPerm, BaseTaskInfo* context) const;

		EntityMethod* getMethod(const VString& inMethodName, bool publicOnly = false) const;

		virtual CDB4DEntityMethod* GetMethod(const XBOX::VString& inMethodName, bool publicOnly = false) const
		{
			return getMethod(inMethodName, publicOnly);
		}

		VError callMethod(const VString& inMethodName, const VectorOfVString& params, VJSValue& result, BaseTaskInfo* inContext, EntityCollection* inSel = nil, EntityRecord* inRec = nil);
		VError callMethod(const VString& inMethodName, const VString& jsonparams, VJSValue& result, BaseTaskInfo* inContext, EntityCollection* inSel = nil, EntityRecord* inRec = nil);
		VError callMethod(const VString& inMethodName, const VValueBag& bagparams, VJSValue& result, BaseTaskInfo* inContext, EntityCollection* inSel = nil, EntityRecord* inRec = nil);

		virtual VJSValue call_Method(const VString& inMethodName,VJSArray& params, VJSObject& thisObj, BaseTaskInfo* context, JS4D::ContextRef jscontext, VError& err) = 0;

		virtual VError SetAutoSequenceNumber(sLONG8 newnum, BaseTaskInfo* context) = 0;

		inline const EntityAttributeCollection& getAllAttributes() const
		{
			return fAttributes;
		}

		//virtual VError compute(EntityAttributeSortedSelection& atts, EntityCollection* sel, VJSObject& outObj, BaseTaskInfo* context, JS4D::ContextRef jscontext) = 0;

		VError CallDBEvent(DBEventKind kind, BaseTaskInfo* context, EntityCollection* *outSel) const;
		VError CallDBEvent(DBEventKind kind, EntityRecord* inRec, BaseTaskInfo* context) const;
		VError CallAttributesDBEvent(DBEventKind kind, EntityRecord* inRec, BaseTaskInfo* context) const;

		bool HasEvent(DBEventKind kind) const;

		const EntityAttributeCollection& GetAttributesWithInitEvent() const
		{
			return fAttributesWithInitEvent;
		}

		const EntityAttributeCollection& GetAttributesWithDefaultValue() const
		{
			return fAttributesWithDefaultValue;
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

		inline EntityModelCatalog* GetCatalog() const
		{
			return fCatalog;
		}

		virtual bool IsIndexed(const EntityAttribute* att) const = 0;
		virtual bool IsFullTextIndexed(const EntityAttribute* att) const = 0;

		virtual bool IsIndexable(const EntityAttribute* att) const = 0;
		virtual bool IsFullTextIndexable(const EntityAttribute* att) const = 0;

		VError BuildQueryString(SearchTab* querysearch, BaseTaskInfo* context, VString outQuery);

		virtual VError AddMethod(EntityMethod* meth);

		virtual bool IsRemote() const
		{
			return true;
		};

		VFileSystemNamespace* GetFileSystemNamespace();

		const VUUID& GetUUID() const
		{
			return fDB4DUUID;
		}

		const VectorOfVString& GetAlternateTableNames() const
		{
			return fAlternateTableNames;
		}

	protected:
		virtual ~EntityModel();


		VError getQParams(VJSParms_callStaticFunction& ioParms, sLONG firstparam, QueryParamElementVector& outParams, SearchTab* inQuery);
		VError FillQueryWithParams(SearchTab* query, VJSParms_callStaticFunction& ioParms, sLONG firstparam);
		VError FillQueryWithParams(SearchTab* query, VJSONArray* inArray);

		VString fName;
		VString fCollectionName;
		VString fExtends;
		VString fBaseModelName;
		EntityModel* fBaseEm;
		EntityModelCatalog* fCatalog;
		ListOfModels fDerivateds;
		list<VString> fRemoveAttributes;
		VectorOfVString fAlternateTableNames;
		VUUID fDB4DUUID;
		EntityAttributeCollection fAttributes;
		EntityAttributeMap fAttributesByName;
		EntityMethodMap fMethodsByName;
		EntityRelationCollection fRelationPaths;
		IdentifyingAttributeCollection fIdentifyingAtts;
		IdentifyingAttributeCollection fOwnedIdentifyingAtts;
		IdentifyingAttributeCollection fPrimaryAtts;
		IdentifyingAttributeCollection fOwnedPrimaryAtts;
		EntityAttributeCollection fAttributesWithInitEvent;
		EntityAttributeCollection fAttributesWithDefaultValue;
		VString fRestrictingOrderByString;
		VString fRestrictingQueryString;
		VString fCumulatedRestrictingQueryString;
		SearchTab* fRestrictingQuery;
		const VValueBag* fExtraProperties;
		VUUID fPerms[DB4D_EM_Promote_Perm+1];
		uBYTE fForced[DB4D_EM_Promote_Perm+1];
		//DBEvent fEvents[dbev_lastEvent+1];
		DBEventMap fDBEvents;
		sLONG fQueryLimit;
		sLONG fDefaultTopSize;
		EntityAttributeScope fScope;
		bool fAlreadyResolvedComputedAtts, fHasDeleteEvent, fOneAttributeHasDeleteEvent, 
			fPublishAsGlobal, fPublishAsGlobalDefined, fWithRestriction, fAllowOverrideStamp,
			fNoEdit,fNoSave;
		mutable uBYTE fMatchPrimKey;

		/*
		static map<Table*, EntityModel*> sEMbyTable;
		static VCriticalSection sEMbyTableMutex;
		*/
		

};



									// ---------------------------------------------

class EntityRecord;
class EntityCollection;


class ENTITY_API SubEntityCache
{
	public:
		inline SubEntityCache()
		{
			fSel = nil;
			fErec = nil;
			fAlreadyActivated = false;
			fModel = nil;
			fLockWay = DB4D_Do_Not_Lock;
		}

		inline void SetSel(EntityCollection* sel)
		{
			fSel = sel;
			fAlreadyActivated = true;
		}

		inline EntityCollection* GetSel() const
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

		inline EntityRecord* GetErec()
		{
			return fErec;
		}

		inline void SetErec(EntityRecord* erec)
		{
			fErec = erec;
			fAlreadyActivated = true;
		}


	protected:
		EntityCollection* fSel;
		EntityRecord* fErec;
		EntityModel* fModel;
		DB4D_Way_of_Locking fLockWay;
		bool fAlreadyActivated;
};




typedef vector<SubEntityCache> SubEntityCacheCollection;

//typedef enum { eav_vvalue = 1, eav_subentity, eav_selOfSubentity } EntityAttributeValueKind;

class ENTITY_API EntityAttributeValue : public CDB4DEntityAttributeValue
{
	public:
		
#if debuglr
		virtual	sLONG		Retain(const char* DebugInfo = 0) const;
		virtual	sLONG		Release(const char* DebugInfo = 0) const;
#endif

		EntityAttributeValue(EntityRecord* owner, const EntityAttribute* attribute, EntityAttributeValueKind kind, VValueSingle* inVal, bool isowned = false);

		EntityAttributeValue(EntityRecord* owner, const EntityAttribute* attribute, EntityAttributeValueKind kind, EntityRecord* erec, EntityModel* inSubModel);

		EntityAttributeValue(EntityRecord* owner, const EntityAttribute* attribute, EntityAttributeValueKind kind, EntityCollection* sel, EntityModel* inSubModel);


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

		inline EntityModel* getRelatedEntityModel() const
		{
			return fSubModel;
		}

		inline EntityCollection* getRelatedSelection() const
		{
			return fSel;
		}

		virtual CDB4DEntityRecord* GetRelatedEntity() const;

		virtual CDB4DEntityModel* GetRelatedEntityModel() const
		{
			return fSubModel;
		}

		virtual CDB4DEntityCollection* GetRelatedSelection() const
		{
			return getRelatedSelection();
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

		EntityAttributeValue* Clone(EntityRecord* newOwner, bool fullClone);

		inline const EntityAttribute* GetAttribute() const
		{
			return fAttribute;
		}


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
		EntityCollection* fSel;
		EntityAttributeValueKind fType;
		bool fIsValueOwned, fCanBeModified, fJSObjectIsValid, fSubEntityCanTryToReload;
		uLONG fStamp, fSubModificationStamp;
};

typedef vector<EntityAttributeValue*> EntityAttributeValueCollection;



									// ---------------------------------------------



class ENTITY_API EntityRecord : public CDB4DEntityRecord
{
	public:
		
		EntityRecord(EntityModel* inModel, BaseTaskInfo* inContext);

		inline EntityModel* GetOwner() const
		{
			return fModel;
		}

		inline EntityModel* GetModel() const
		{
			return fModel;
		}

		virtual void TouchAttribute(const EntityAttribute* att) = 0;

		VError ThrowError( VError inErrCode, const VString* p1 = nil) const;

		SubEntityCache* GetSubEntityCache(sLONG inPathID, VError& err, bool Nto1, EntityModel* subEntityModel, BaseTaskInfo* context);
		bool SubEntityCacheNeedsActivation(sLONG inPathID);
		void ClearSubEntityCache(sLONG inPathID);
		void ClearAllSubEntityCaches();

		virtual EntityAttributeValue* do_getAttributeValue(const EntityAttribute* inAttribute, VError& err, BaseTaskInfo* context, bool restrictValue = false);
		virtual VError do_setAttributeValue(const EntityAttribute* inAttribute, const VValueSingle* inValue);
		virtual EntityRecord* do_LoadRelatedEntity(const EntityAttribute* inAttribute, const EntityAttribute* relatedAttribute, BaseTaskInfo* context, VError& err, DB4D_Way_of_Locking HowToLock = DB4D_Do_Not_Lock);

		EntityAttributeValue* getAttributeValue(const EntityAttribute* inAttribute, VError& err, BaseTaskInfo* context, bool restrictValue = false);

		VError setAttributeValue(const EntityAttribute* inAttribute, const VValueSingle* inValue)
		{
			return do_setAttributeValue(inAttribute, inValue);
		}

		EntityAttributeValue* getAttributeValue(const VString& inAttributeName, VError& err, BaseTaskInfo* context, bool restrictValue = false);
		EntityAttributeValue* getAttributeValue(sLONG pos, VError& err, BaseTaskInfo* context, bool restrictValue = false);
		EntityAttributeValue* getAttributeValue(const AttributePath& inAttPath, VError& err, BaseTaskInfo* context, bool restrictValue = false);


		bool equalAttributeValue(const VString& inAttributeName, const VValueSingle* inValue);
		bool equalAttributeValue(const EntityAttribute* inAttribute, const VValueSingle* inValue);

		VError setAttributeValue(const VString& inAttributeName, const VValueSingle* inValue);

		VError setAttributeValue(const VString& inAttributeName, EntityRecord* inRelatedEntity);
		VError setAttributeValue(const EntityAttribute* inAttribute, EntityRecord* inRelatedEntity);

		VError setAttributeValue(const VString& inAttributeName, const VectorOfVValue& inIdentKey);
		VError setAttributeValue(const EntityAttribute* inAttribute, const VectorOfVValue& inIdentKey);

		VError setAttributeValue(const EntityAttribute* inAttribute, const VString& inJsonValue);
		VError setAttributeValue(const VString& inAttributeName, const VString& inJsonValue);

		VError setAttributeValue(const EntityAttribute* inAttribute, const VJSObject& inJSObject);
		VError setAttributeValue(const VString& inAttributeName, const VJSObject& inJSObject);


		VError touchAttributeValue(const EntityAttribute* inAttribute);

		virtual CDB4DEntityAttributeValue* GetAttributeValue(const XBOX::VString& inAttributeName, VErrorDB4D& err)
		{
			return getAttributeValue(inAttributeName, err, fContext);
		}

		virtual CDB4DEntityAttributeValue* GetAttributeValue(const CDB4DEntityAttribute* inAttribute, VErrorDB4D& err)
		{
			return getAttributeValue(dynamic_cast<const EntityAttribute*>(inAttribute), err, fContext);
		}

		virtual VErrorDB4D SetAttributeValue(const CDB4DEntityAttribute* inAttribute, const XBOX::VValueSingle* inValue)
		{
			return setAttributeValue(dynamic_cast<const EntityAttribute*>(inAttribute), inValue);
		}

		virtual VErrorDB4D SetAttributeValue(const XBOX::VString& inAttributeName, const XBOX::VValueSingle* inValue)
		{
			return setAttributeValue(inAttributeName, inValue);
		}

		virtual VErrorDB4D SetAttributeValue(const CDB4DEntityAttribute* inAttribute, CDB4DEntityRecord* inRelatedEntity)
		{
			return setAttributeValue(dynamic_cast<const EntityAttribute*>(inAttribute), inRelatedEntity == nil ? nil : dynamic_cast<EntityRecord*>(inRelatedEntity));
		}

		virtual VErrorDB4D SetAttributeValue(const XBOX::VString& inAttributeName, CDB4DEntityRecord* inRelatedEntity)
		{
			return setAttributeValue(inAttributeName, inRelatedEntity == nil ? nil : dynamic_cast<EntityRecord*>(inRelatedEntity));
		}

		virtual VErrorDB4D SetAttributeValue(const CDB4DEntityAttribute* inAttribute, const XBOX::VectorOfVValue* inValues)
		{
			return setAttributeValue(dynamic_cast<const EntityAttribute*>(inAttribute), inValues);
		}

		virtual VErrorDB4D SetAttributeValue(const XBOX::VString& inAttributeName, const XBOX::VectorOfVValue* inValues)
		{
			return setAttributeValue(inAttributeName, inValues);
		}

		virtual VErrorDB4D SetAttributeValue(const CDB4DEntityAttribute* inAttribute, const XBOX::VString& inJsonValue)
		{
			return setAttributeValue(dynamic_cast<const EntityAttribute*>(inAttribute), inJsonValue);
		}

		virtual VErrorDB4D SetAttributeValue(const XBOX::VString& inAttributeName, const XBOX::VString& inJsonValue)
		{
			return setAttributeValue(inAttributeName, inJsonValue);
		}

		virtual VErrorDB4D SetAttributeValue(const CDB4DEntityAttribute* inAttribute, const XBOX::VJSObject& inJSObject)
		{
			return setAttributeValue(dynamic_cast<const EntityAttribute*>(inAttribute), inJSObject);
		}

		virtual VErrorDB4D SetAttributeValue(const XBOX::VString& inAttributeName, const XBOX::VJSObject& inJSObject)
		{
			return setAttributeValue(inAttributeName, inJSObject);
		}

		virtual VErrorDB4D TouchAttributeValue(const CDB4DEntityAttribute* inAttribute)
		{
			return touchAttributeValue(dynamic_cast<const EntityAttribute*>(inAttribute));
		}

		virtual VErrorDB4D resetAttributeValue(const EntityAttribute* inAttribute);

		virtual VErrorDB4D ResetAttributeValue(const CDB4DEntityAttribute* inAttribute)
		{
			return resetAttributeValue(dynamic_cast<const EntityAttribute*>(inAttribute));
		}

		VErrorDB4D resetAttributeValue(const VString& inAttributeName);

		virtual VErrorDB4D ResetAttributeValue(const VString& inAttributeName)
		{
			return resetAttributeValue(inAttributeName);
		}


		virtual bool ContainsBlobData(const EntityAttribute* inAttribute, BaseTaskInfo* context) = 0;

		virtual VError Save(Transaction* *trans, BaseTaskInfo* context, uLONG stamp, bool allowOverrideStamp = false) = 0;
		virtual VErrorDB4D Save(uLONG stamp, bool allowOverrideStamp = false) = 0;

		virtual VError Validate(BaseTaskInfo* context);
		virtual VErrorDB4D Validate();
		virtual VError CallSaveEvents(BaseTaskInfo* context);
		
		virtual Boolean IsNew() const = 0;
		virtual Boolean IsProtected() const = 0;
		virtual Boolean IsModified() const = 0;
		
		virtual void GetTimeStamp(VTime& outValue) = 0;
		
		virtual VErrorDB4D Drop() = 0;

		virtual uLONG GetStamp() const = 0;
		virtual sLONG GetModificationStamp() const = 0;

		virtual VError Reload();
		virtual VError do_Reload() = 0;

		virtual void Touch(BaseTaskInfo* context) = 0;

		void GetPrimKeyValue(VJSObject& outObj);
		void GetPrimKeyValue(VValueBag& outBagKey);
		void GetPrimKeyValue(VString& outKey, bool autoQuotes);
		void GetPrimKeyValue(VectorOfVValue& outPrimkey);

		virtual bool do_GetPrimKeyValueAsObject(VJSObject& outObj)
		{
			return false;
		}

		virtual bool do_GetPrimKeyValueAsBag(VValueBag& outBagKey)
		{
			return false;
		}

		virtual bool do_GetPrimKeyValueAsString(VString& outKey, bool autoQuotes)
		{
			return false;
		}

		virtual bool do_GetPrimKeyValueAsVValues(VectorOfVValue& outPrimkey)
		{
			return false;
		}

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

		BaseTaskInfo* getContext();

		virtual CDB4DBaseContext* GetContext();
		
		//virtual void ReleaseExtraDatas();

		//VValueSingle* getFieldValue(Field* cri, VError& err);
		//VError setFieldValue(Field* cri, const VValueSingle* inValue);

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

		virtual VError SetForeignKey(const EntityAttribute* att, EntityRecord* relatedEntity, const VValueSingle* inForeignKey) = 0;

		VError GetModifiedAttributes(EntityAttributeCollection& outatts);

		virtual VError DuplicateInto(EntityRecord* otherRec);

		virtual EntityRecord* Clone(VError& outErr);

		bool AlreadySaving() const
		{
			return fAlreadySaving;
		}

		bool AlreadyDeleting() const
		{
			return fAlreadyDeleting;
		}

	
#if debuglr
		virtual	sLONG		Retain(const char* DebugInfo = 0) const;
		virtual	sLONG		Release(const char* DebugInfo = 0) const;
#endif

#if debug_entityRecord
		static void initDebug()
		{
			sNbEntityRecordsInMem = 0;
		}
#endif

	protected:

		virtual ~EntityRecord();

		EntityModel* fModel;
		BaseTaskInfo* fContext;
		SubEntityCacheCollection fSubEntitiesCache;  // a mettre en parallele avec EntityModel.fRelationPaths
		EntityAttributeValueCollection fValues;
		DB4D_Way_of_Locking fWayOfLock;
		uBYTE fAlreadyCallEvent[dbev_lastEvent+1];
		bool fAlreadySaving, fAlreadyDeleting;

#if debug_entityRecord
		static sLONG sNbEntityRecordsInMem;
#endif
};


/*
VError SelToJSObject(BaseTaskInfo* context, VJSArray& outArr, EntityModel* em, Selection* inSel, EntityAttributeSortedSelection& whichAttributes, EntityAttributeSelection* expandAttributes, 
					 EntityAttributeSortedSelection* sortingAttributes, bool withKey, bool allowEmptyAttList, sLONG from, sLONG count);

VError SelToJSObject(BaseTaskInfo* context, VJSArray& outArr, EntityModel* em, Selection* inSel, const VString& inAttributeList , bool withKey, bool allowEmptyAttList, sLONG from, sLONG count);
*/


// ----------------------------------------------------------------------------------------------------------------------------------

const sLONG kRemoteCatalogFactory = 1;
const sLONG kSQLCatalogFactory = 2;


typedef map<VString, EntityModel*, CompareLessVStringStrict> EntityModelMap;
//typedef map<VString, EntityRelation*> EntityRelationMap;
typedef map<VString, AttributeType*, CompareLessVStringStrict> AttributeTypeMap;
typedef map<VString, EmEnum*> EmEnumMap;

class Base4D;

class EntityModelCatalog;

typedef EntityModelCatalog* (*ModelCatalogFactory)(Base4D* inOwner, VJSONObject* inParams, VError& outErr);
typedef map<sLONG, ModelCatalogFactory> ModelCatalogFactoryMap;

class ENTITY_API EntityModelCatalog : public IDebugRefCountable
{

	public:
		EntityModelCatalog(Base4D* assocBase)
		{
			//fOwner = owner;
			fAssociatedBase = assocBase;
			fExtraProperties = nil;
			fRemoteAccess = nil;
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
			fJsonFormat = true;
			fErrorReporter = nil;
			fLogger = nil;
		}

		/*
		Base4D* GetOwner()
		{
			return fOwner;
		}

		const Base4D* GetOwner() const
		{
			return fOwner;
		}
		*/

		Base4D* GetAssocBase()
		{
			return fAssociatedBase;
		}

		const Base4D* GetAssocBase() const
		{
			return fAssociatedBase;
		}

		VError ThrowError( VError inErrCode, ActionDB4D inAction, const VString* p1 = nil) const;

		void DisposeEntityModels();
		void DisposeEntityRelations();
		void DisposeEntityTypes();
		void DisposeEnumerations();

		virtual void GetName(VString& outName) const = 0;

		virtual EntityModel* NewModel() = 0;

		virtual VError do_LoadEntityModels(const VValueBag& bagEntities, ModelErrorReporter* errorReporter, unordered_map_VString<VRefPtr<VFile> >* outIncludedFiles, BaseTaskInfo* context)
		{
			return VE_OK;
		}

		virtual VError do_SecondPassLoadEntityModels(ModelErrorReporter* errorReporter)
		{
			return VE_OK;
		}

		virtual VError StartTransaction(BaseTaskInfo* context)
		{
			return ThrowBaseError(VE_DB4D_NOTIMPLEMENTED);
		}

		virtual VError CommitTransaction(BaseTaskInfo* context)
		{
			return ThrowBaseError(VE_DB4D_NOTIMPLEMENTED);
		}

		virtual VError RollBackTransaction(BaseTaskInfo* context)
		{
			return ThrowBaseError(VE_DB4D_NOTIMPLEMENTED);
		}

		virtual VError ReleaseWhatNeedsToBeReleased()
		{
			return VE_OK;
		};

		virtual VError SetLogFile(VFile* logfile, bool append, VJSONObject* options);
		virtual VError StartLogging();
		virtual VError StopLogging();
		virtual VError FlushLog();
		virtual VError AcceptLogger(DB4DLogger* logger);

		VError SecondPassLoadEntityModels(ModelErrorReporter* errorReporter);

		VError addJSExtensions(Base4D* associate, VFile* jsFile, ModelErrorReporter* errorReporter);

		VError LoadEntityModels(const VValueBag& bagEntities, ModelErrorReporter* errorReporter, unordered_map_VString<VRefPtr<VFile> >* outIncludedFiles = nil, bool allowFolderParsing = true);
		VError LoadEntityModels(const VFile& inFile, bool inXML = true, ModelErrorReporter* errorReporter = nil, const VString* inXmlContent = nil, unordered_map_VString<VRefPtr<VFile> >* outIncludedFiles = nil);

		VError SaveEntityModels(VValueBag& catalogBag, bool inXML = true, bool withDBInfo = false) const;
		VError SaveEntityModels(VFile& inFile, bool inXML = true, bool withDBInfo = false) const;

		VError AddOneEntityModel(EntityModel* newEntity, bool canReplace);
		EntityModel* BuildEntityModel(const VValueBag* bag, VError& err, ModelErrorReporter* errorReporter);
		EntityModel* FindEntityByCollectionName(const VString& entityName) const;
		EntityModel* FindEntity(const VString& entityName) const;
		EntityModel* RetainEntityByCollectionName(const VString& entityName) const;
		EntityModel* RetainEntity(const VString& entityName) const;

		VError AddOneEntityRelation(EntityRelation* newRelation, bool canReplace);
		//EntityRelation* BuildEntityRelation(const VValueBag* bag, VError& err);
		//EntityRelation* FindRelation(const VString& relationName) const;

		VError AddOneType(AttributeType* newType, bool canReplace);
		AttributeType* BuildType(const VValueBag* bag, VError& err, ModelErrorReporter* errorReporter);
		AttributeType* FindType(const VString& typeName) const;

		VError AddOneEnumeration(EmEnum* newEnum, bool canReplace);
		EmEnum* BuildEnumeration(const VValueBag* bag, VError& err, ModelErrorReporter* errorReporter);
		EmEnum* FindEnumeration(const VString& enumName) const;

		sLONG CountEntityModels() const
		{
			return (sLONG)fEntityModels.size();
		}

		VError GetAllEntityModels(vector<CDB4DEntityModel*>& outList) const;
		VError GetAllEntityModels(vector<EntityModel*>& outList) const;
		VError RetainAllEntityModels(vector<VRefPtr<EntityModel> >& outList) const;

		VError LoadEntityPermissions(const VValueBag& bagEntities, CUAGDirectory* inDirectory, ModelErrorReporter* errorReporter);
		VError LoadEntityPermissions(const VFile& inFile, CUAGDirectory* inDirectory, bool inXML = true, ModelErrorReporter* errorReporter = nil);

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
			if (fErrorReporter != nil)
			{
				delete fErrorReporter;
				fErrorReporter = nil;
			}
		}

		ModelErrorReporter* CreateErrorReporter();
		ModelErrorReporter* GetErrorReporter()
		{
			return fErrorReporter;
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

		VError ResolveRelatedEntities(BaseTaskInfo* context);

		static ModelCatalogFactoryMap sModelFactories;
		static void RegisterFactory(sLONG signature, ModelCatalogFactory code);
		static ModelCatalogFactory GetFactory(sLONG signature);
		static EntityModelCatalog* NewCatalog(sLONG signature, Base4D* inOwner, VJSONObject* params, VError& outErr);


	protected:

		virtual ~EntityModelCatalog()
		{
			QuickReleaseRefCountable(fExtraProperties);
			QuickReleaseRefCountable(fRemoteAccess);
			DisposeEntityModels();
			DisposeEntityRelations();
			DisposeEntityTypes();
			DisposeEnumerations();
			QuickReleaseRefCountable(fParseFileError);
			if (fErrorReporter != nil)
				delete fErrorReporter;
			QuickReleaseRefCountable(fLogger);
			//EntityModel::ClearCacheTableEM();
		}

		VError xSetEntityPerm(CUAGDirectory* uagdir, const VValueBag::StKey key, const VValueBag* OnePerm, EntityModel* em, DB4D_EM_Perm perm);

		//Base4D* fOwner;
		Base4D* fAssociatedBase;
		DB4DLogger* fLogger;
		const VValueBag* fExtraProperties;
		const VValueBag* fRemoteAccess;
		EntityModelMap fEntityModels;
		EntityModelMap fEntityModelsByCollectionName;
		//EntityRelationMap fEntityRelations;
		EntityRelationCollection fRelations;
		AttributeTypeMap fTypes;
		EmEnumMap fEnumerations;
		mutable VCriticalSection fEntityModelMutex;
		mutable VJSContext* fLoadingContext;
		mutable VJSObject* fLoadingGlobalObject;
		bool fMustTouchXML, fSomeErrorsInCatalog, fParseUserDefined, fPublishDataClassesAsGlobals, fPublishDataClassesAsGlobalsDefined, fJsonFormat;
		sLONG fParseLineError;
		VString fParseMessageError;
		VFile* fParseFileError;
		ModelErrorReporter* fErrorReporter;
		VUUID fPerms[DB4D_EM_Promote_Perm+1];
		uBYTE fForced[DB4D_EM_Promote_Perm+1];

};



class ModelJSTransformer
{
	public:

		ModelJSTransformer(Base4D* base);

		~ModelJSTransformer();

		VError TransformJSIntoBag(VFile& inJSFile, const VString& modelRoot, VValueBag& bagEntities, unordered_map_VString<VRefPtr<VFile> >* outIncludedFiles, ModelErrorReporter* errorReporter, bool forARemoteCatalog);

		bool ParsingJSError(VFile* &outRetainedFile, VString& outMessage, sLONG& outLineNumber)
		{
			outRetainedFile = RetainRefCountable(fParseFileError);
			outMessage = fParseMessageError;
			outLineNumber = fParseLineError;
			return (fParseFileError != nil);
		}

		VJSGlobalContext* GetJSGlobalContext()
		{
			return xJSContext;
		}

		BaseTaskInfo* GetBaseContext()
		{
			return context;
		}

	protected:
		BaseTaskInfo* context;
		DB4DJSRuntimeDelegate* xJSDelegate;
		VJSGlobalContext* xJSContext;

		sLONG fParseLineError;
		VString fParseMessageError;
		VFile* fParseFileError;

};


						// ---------------------------------------------------------------------------- 



class ENTITY_API DataSet : public IDebugRefCountable
{
public:

	DataSet(EntityModel* inModel, EntityCollection* inSel, uLONG timeout = 0)
	{
		fModel = RetainRefCountable(inModel);
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

	inline EntityCollection* GetSel() const
	{
		return fSel;
	}

	inline EntityModel* GetModel() const
	{
		return fModel;
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
		tablename = fModel->GetName();
		outBag.SetVUUID(L"id", fID);
		outBag.SetString(L"dataClass", tablename);
		outBag.SetLong("selectionSize", fSel->GetLength((BaseTaskInfo*)nil));
		bool sorted = fSel->IsSorted();
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

	EntityCollection* fSel;
	EntityModel* fModel;
	VUUID fID;
	uLONG fTimeout;
	uLONG fStartingTime;
};




#endif

