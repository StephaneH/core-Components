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


const ArrayOfConstUniCharPtr xEatt_types = { L"storage", L"alias", L"calculated", L"relatedEntity", L"relatedEntities", L"removed", L"altered", /*L"field",*/ L"composition", L"" };
const Enumeration EattTypes(xEatt_types);

const ArrayOfConstUniCharPtr xScript_types = { L"db4d", L"javascript", L"4d", L"" };
const Enumeration ScriptTypes(xScript_types);

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
	L"object", 
	L"" 
};
const Enumeration EValPredefinedTypes(xEValPredefinedTypes);



const ArrayOfConstUniCharPtr xFormulaeActions = 
{
	L"sum",
	L"average",
	L"min",
	L"max",
	L"count",
	L"countDistinct",
	L"averageDistinct",
	L"sumDistinct",
	L""
};
const Enumeration EFormulaeActions(xFormulaeActions);


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
	L"objectPath",
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
	L"attribute",
	L""
};

const Enumeration EPermResourceType(xPermResourceType);


const ArrayOfConstUniCharPtr xPermAction = 
{
	L"read",
	L"describe",
	L"create",
	L"update",
	L"remove",
	L"execute",
	L"promote",
	L""
};

const Enumeration EPermAction(xPermAction);



const ArrayOfConstUniCharPtr xOldDBEventKinds = 
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

const Enumeration OldEDBEventKinds(xOldDBEventKinds);


const ArrayOfConstUniCharPtr xDBEventKinds =
{
	L"save",
	L"load",
	L"init",
	L"remove",
	L"validate",
	L"restrict",
	L"set",
	L"get",
	L"validateremove",
	L"restload",
	L"restget",
	L"clientrefresh",
	L"transactioncommitted",
	L""
};

const Enumeration EDBEventKinds(xDBEventKinds);



const ArrayOfConstUniCharPtr ecat_correspondance = 
{
	L"entityModel", L"entityModels",
	L"type", L"types",
	L"relationship", L"relationships",
	L"", L""
};

const Correspondance CatCorres(ecat_correspondance);

const ArrayOfConstUniCharPtr xERechToken = 
{ 
	L"leftParenthesis", 
	L"rightParenthesis", 
	L"endOfQuery", 
	L"simpleComp", 
	L"simpleCompWithEm", 
	L"boolOper", 
	L"script", 
	L"not", 
	L"simpleCompOnArray", 
	L"simpleCompOnArrayWithEm", 
	L"join", 
	L"joinWithEm",
	L"onSelection", 
	L"onCollection", 
	L"onEmComputed", 
	L"interpConst", 
	L"interpAtt", 
	L"interpOper",
	L"recordExists", 
	L"entityExists", 
	L"interpFormula", 
	L"JSScript", 
	L"" 
};
const Enumeration ERechToken(xERechToken);

const ArrayOfConstUniCharPtr xEAttributeProperty =
{
	L"path",
	L""
};

const Enumeration EAttributeProperty(xEAttributeProperty);



// ---------------------------------------------------------------------------------------


template<>
const VString* IPartable<VectorOfVString, const VString*>::GetElem(VectorOfVString::const_iterator iter) const
{
	return &(*iter);
}


template<>
const EntityAttributeInstance* IPartable<EntityAttributeInstanceCollection, const EntityAttributeInstance*>::GetElem(EntityAttributeInstanceCollection::const_iterator iter) const
{
	return &(*iter);
}


// ----------------------------------------------------------------------------------------

DataSet::~DataSet()
{
	QuickReleaseRefCountable(fSel);
	QuickReleaseRefCountable(fModel);
}



// ----------------------------------------------------------------------------------------


ModelErrorReporter::ModelErrorReporter(EntityModelCatalog* inCatalog)
{
	fCatalog = inCatalog;
}


void ModelErrorReporter::AddError(VErrorBase* error)
{
	fErrors.push_back(error);
}


// ------------------


ModelErrorContextInstaller::ModelErrorContextInstaller(ModelErrorReporter* reporter):fErrors(false)
{
	fErrorReporter = reporter;
}


ModelErrorContextInstaller::~ModelErrorContextInstaller()
{
	VErrorContext* errcontext = fErrors.GetContext();
	const VErrorStack& errstack = errcontext->GetErrorStack();
	if (errstack.size() > 0)
	{
		fErrorReporter->GetCatalog()->AddError();
		for (VErrorStack::const_iterator cur = errstack.begin(), end = errstack.end(); cur != end; ++cur)
		{
			fErrorReporter->AddError(*cur);
		}
	}
}



// ----------------------------------------------------------------------------------------



VError EmEnum::ThrowError( VError inErrCode, const VString* p1) const
{
	VErrorDB4D_OnEMEnum *err = new VErrorDB4D_OnEMEnum(inErrCode, noaction, fOwner, this);
	if (p1 != nil)
		err->GetBag()->SetString(Db4DError::Param1, *p1);
	VTask::GetCurrent()->PushRetainedError( err);

	return inErrCode;
}


VError EmEnum::FromBag(const VValueBag* bag, EntityModelCatalog* catalog, ModelErrorReporter* errorReporter)
{
	VError err = VE_OK;

	fExtraProperties = bag->RetainUniqueElement(d4::extraProperties);
	bag->GetString(d4::name, fName);
	const VBagArray* items = bag->GetElements(d4::item);
	if (items != nil)
	{
		VSize nb = items->GetCount();
		for (VIndex i = 1; i <= nb && err == VE_OK; i++)
		{
			const VValueBag* item = items->GetNth(i);
			if (item != nil)
			{
				VString name;
				sLONG value;
				if (item->GetString(d4::name, name) && item->GetLong(d4::value, value))
				{
					try
					{
						fEnums[name] = value;
						fEnumsID.insert(make_pair(value, name));
					}
					catch (...)
					{
						err = ThrowBaseError(memfull);
					}
				}
			}
		}
	}

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOT_BUILD_ENUM_FROM_DEF);

	return err;
}


VError EmEnum::ToBag(VValueBag& outBag, bool forDax, bool forSave, bool forJSON, bool firstlevel) const
{
	VError err = VE_OK;

	if (fExtraProperties != nil)
		outBag.AddElement(d4::extraProperties, (VValueBag*)fExtraProperties);

	if (!fName.IsEmpty())
	{
		outBag.SetString(d4::name, fName);
	}
	for (EnumIDMap::const_iterator cur = fEnumsID.begin(), end = fEnumsID.end(); cur != end && err == VE_OK; cur++)
	{
		BagElement item(outBag, d4::item);
		item->SetString(d4::name, cur->second);
		item->SetLong(d4::value, cur->first);
	}

	return err;
}


bool EmEnum::GetValue(const VString& inName, sLONG& outValue) const
{
	EnumMap::const_iterator found = fEnums.find(inName);
	if (found != fEnums.end())
	{
		outValue = found->second;
		return true;
	}
	else
		return false;
}

bool EmEnum::GetValueName(sLONG inValue, VString& outName) const
{
	EnumIDMap::const_iterator found = fEnumsID.find(inValue);
	if (found != fEnumsID.end())
	{
		outName = found->second;
		return true;
	}
	else
		return false;
}




// ----------------------------------------------------------------------------------------

AttributeType::AttributeType(EntityModelCatalog* owner)
{
	fOwner = owner;
	fScalarType = 0;

	fFixedLength = 0;
	fMinLength = 0;
	fMaxLength = 0;

	fMin = nil;
	fMax = nil;
	fDefaultValue = nil;
	fEnumeration = nil;

	fFrom = nil;
	fResultType = nil;
	fRegexPattern = nil;
	fExtraProperties = nil;

	fAutoComplete = 2;

}


AttributeType::~AttributeType()
{
	Clear();
	QuickReleaseRefCountable(fExtraProperties);
}


VError AttributeType::ThrowError( VError inErrCode, const VString* p1) const
{
	VErrorDB4D_OnEMType *err = new VErrorDB4D_OnEMType(inErrCode, noaction, fOwner, this);
	if (p1 != nil)
		err->GetBag()->SetString(Db4DError::Param1, *p1);
	VTask::GetCurrent()->PushRetainedError( err);

	return inErrCode;
}


void AttributeType::Clear()
{
	if (fMax != nil)
		delete fMax;
	fMax = nil;

	if (fMin != nil)
		delete fMin;
	fMin = nil;

	if (fDefaultValue != nil)
		delete fDefaultValue;
	fDefaultValue = nil;

	fFormat.Clear();
	fPattern.Clear();
	fFixedLength = 0;
	fMinLength = 0;
	fMaxLength = 0;
	fAutoComplete = 2;

	ReleaseRefCountable(&fEnumeration);
	ReleaseRefCountable(&fResultType);
	ReleaseRefCountable(&fRegexPattern);
}


void AttributeType::CopyFrom(const AttributeType* from)
{
	fFormat = from->fFormat;
	fPattern = from->fPattern;
	fFixedLength = from->fFixedLength;
	fMinLength = from->fMinLength;
	fMaxLength = from->fMaxLength;
	fEnumeration = RetainRefCountable(from->fEnumeration);
	fMin = from->fMin == nil ? nil : from->fMin->Clone();
	fMax = from->fMax == nil ? nil : from->fMax->Clone();
	fDefaultValue = from->fDefaultValue == nil ? nil : from->fDefaultValue->Clone();
	fAutoComplete = from->fAutoComplete;
}


void AttributeType::ReCalc()
{
	if (fResultType == nil)
		fResultType = new AttributeType(fOwner);
	else
		fResultType->Clear();

	fResultType->CopyFrom(this);

	if (fFrom != nil)
	{
		if (fAutoComplete == 2 && fFrom->fAutoComplete != 2)
			fAutoComplete = fFrom->fAutoComplete;

		if (fFormat.IsEmpty() && !fFrom->fFormat.IsEmpty())
		{
			fResultType->fFormat = fFrom->fFormat;
		}

		if (fPattern.IsEmpty() && !fFrom->fPattern.IsEmpty())
		{
			fResultType->fPattern = fFrom->fPattern;
		}

		if (fDefaultValue == nil && fFrom->fDefaultValue != nil)
		{
			fResultType->fDefaultValue = fFrom->fDefaultValue->Clone();
		}

		if (fMin == nil && fFrom->fMin != nil)
		{
			fResultType->fMin = fFrom->fMin->Clone();
		}

		if (fMax == nil && fFrom->fMax != nil)
		{
			fResultType->fMax = fFrom->fMax->Clone();
		}

		if (fFixedLength == 0 && fFrom->fFixedLength != 0)
		{
			fResultType->fFixedLength = fFrom->fFixedLength;
		}

		if (fMinLength == 0 && fFrom->fMinLength != 0)
		{
			fResultType->fMinLength = fFrom->fMinLength;
		}

		if (fMaxLength == 0 && fFrom->fMaxLength != 0)
		{
			fResultType->fMaxLength = fFrom->fMaxLength;
		}

		if (fEnumeration == nil && fFrom->fEnumeration != nil)
		{
			fResultType->fEnumeration = RetainRefCountable(fFrom->fEnumeration);
		}

	}

}


bool AttributeType::IsEmpty() const
{
	if (!fFormat.IsEmpty())
		return false;

	if (!fPattern.IsEmpty())
		return false;

	if (fFixedLength != 0)
		return false;

	if (fMinLength != 0)
		return false;

	if (fMaxLength != 0)
		return false;

	if (fEnumeration != nil)
		return false;

	if (fMin != nil)
		return false;

	if (fMax != nil)
		return false;

	if (fDefaultValue != nil)
		return false;

	if (fAutoComplete != 2)
		return false;

	return true;
}


void AttributeType::ExtendsFrom(AttributeType* extendfrom)
{
	fFrom = extendfrom;
}


VError AttributeType::FromBag(const VValueBag* bag, EntityModelCatalog* catalog, bool fullyLoad, ModelErrorReporter* errorReporter)
{
	VError err = VE_OK;

	fExtraProperties = bag->RetainUniqueElement(d4::extraProperties);

	if (fullyLoad)
	{
		bag->GetString(d4::name, fName);
		if (bag->GetString(d4::extends, fExtends))
		{
			fScalarType = EValPredefinedTypes[fExtends];
			if (fScalarType == VK_FLOAT)
				fScalarType = VK_REAL;
			if (fScalarType == 0)
			{
				fFrom = catalog->FindType(fExtends);
				if (fFrom != nil)
				{
					fFrom->fDerivateds.push_back(this);
					fScalarType = fFrom->ComputeScalarType();
				}
			}
			else
				fFrom = nil;
		}
	}
	bag->GetLong(d4::fixedLength, fFixedLength);
	bag->GetLong(d4::minLength, fMinLength);

	bag->GetLong(d4::maxLength, fMaxLength);

	VValueSingle* cv = nil;
	const VValueSingle* cvconst = bag->GetAttribute(d4::minValue);
	if (cvconst != nil)
		cv = cvconst->Clone();
	fMin = cv;

	cv = nil;
	cvconst = bag->GetAttribute(d4::maxValue);
	if (cvconst != nil)
		cv = cvconst->Clone();
	fMax = cv;

	cv = nil;
	cvconst = bag->GetAttribute(d4::defaultValue);
	if (cvconst != nil)
		cv = cvconst->Clone();
	fDefaultValue = cv;

	const VValueBag* formatBag = bag->GetUniqueElement(d4::defaultFormat);
	if (formatBag != nil)
	{
		formatBag->GetString(d4::format, fFormat);
		formatBag->GetString(d4::locale, fLocale);
		formatBag->GetString(d4::presentation, fPresentation);
		formatBag->GetString(d4::sliderMin, fSliderMin);
		formatBag->GetString(d4::sliderMax, fSliderMax);
		formatBag->GetString(d4::sliderInc, fSliderInc);
	}	

	bag->GetString(d4::pattern, fPattern);

	bool autocomplete = false;
	if (bag->GetBool(d4::autoComplete, autocomplete))
		fAutoComplete = autocomplete;
	else
		fAutoComplete = 2;

	VString enumname;
	if (bag->GetString(d4::enumeration, enumname))
	{
		EmEnum* en = catalog->FindEnumeration(enumname);
		if (en == nil)
		{
			if (errorReporter == nil)
				err = ThrowError(VE_DB4D_ENUMERATION_DOES_NOT_EXIST, &enumname);
			else
			{
				ModelErrorContextInstaller errs(errorReporter);
				ThrowError(VE_DB4D_ENUMERATION_DOES_NOT_EXIST, &enumname);
			}
		}
		else
		{
			en->Retain();
			fEnumeration = en;
		}
	}
	else
	{
		const VValueBag* enumbag = bag->GetUniqueElement(d4::enumeration);
		if (enumbag != nil)
		{
			EmEnum* en = catalog->BuildEnumeration(enumbag, err, errorReporter);
			if (en != nil)
			{
				en->ClearName();
				fEnumeration = en;
			}
		}
	}

	if (fullyLoad && err == VE_OK)
		ReCalc();

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOT_BUILD_TYPE_FROM_DEF);

	return err;
}

VError AttributeType::ToBag(VValueBag& outBag, bool forDax, bool forSave, bool forJSON, bool firstlevel) const
{
	VError err = VE_OK;

	if (fExtraProperties != nil)
		outBag.AddElement(d4::extraProperties, (VValueBag*)fExtraProperties);

	if (forSave || fResultType == nil)
	{

		if (!fName.IsEmpty())
			outBag.SetString(d4::name, fName);

		if (!fExtends.IsEmpty())
			outBag.SetString(d4::extends, fExtends);

		if (!fPattern.IsEmpty())
			outBag.SetString(d4::pattern, fPattern);

		if (fFixedLength > 0)
			outBag.SetLong(d4::fixedLength, fFixedLength);

		if (fMinLength > 0)
			outBag.SetLong(d4::minLength, fMinLength);

		if (fMaxLength > 0)
			outBag.SetLong(d4::maxLength, fMaxLength);

		if (fMin != nil)
			outBag.SetAttribute(d4::minValue, fMin->Clone());

		if (fMax != nil)
			outBag.SetAttribute(d4::maxValue, fMax->Clone());

		if (fDefaultValue != nil)
			outBag.SetAttribute(d4::defaultValue, fDefaultValue->Clone());

		if (fAutoComplete != 2)
			outBag.SetBool(d4::autoComplete, fAutoComplete);

		if (!fFormat.IsEmpty() || !fLocale.IsEmpty() || !fPresentation.IsEmpty() || !fSliderMin.IsEmpty() || !fSliderMax.IsEmpty() || !fSliderInc.IsEmpty())
		{
			BagElement formatBag(outBag, d4::defaultFormat);
			if (forJSON)
				formatBag->SetBool(____objectunic, true);
			if (!fFormat.IsEmpty())
				formatBag->SetString(d4::format, fFormat);
			if (!fLocale.IsEmpty())
				formatBag->SetString(d4::locale, fLocale);
			if (!fPresentation.IsEmpty())
				formatBag->SetString(d4::presentation, fPresentation);
			if (!fSliderMin.IsEmpty())
				formatBag->SetString(d4::sliderMin, fSliderMin);
			if (!fSliderMax.IsEmpty())
				formatBag->SetString(d4::sliderMax, fSliderMax);
			if (!fSliderInc.IsEmpty())
				formatBag->SetString(d4::sliderInc, fSliderInc);
		}

		if (fEnumeration != nil)
		{
			if (fEnumeration->GetName().IsEmpty() || !forSave)
			{
				BagElement enumbag(outBag, d4::enumeration);
				if (forJSON)
					enumbag->SetBool(____objectunic, true);
				fEnumeration->ToBag(*enumbag, forDax, forSave, forJSON, firstlevel);
			}
			else
			{
				outBag.SetString(d4::enumeration, fEnumeration->GetName());
			}
		}
	}
	else
	{
		err = fResultType->ToBag(outBag, forDax, forSave, forJSON, firstlevel);
	}

	return err;
}


VError AttributeType::SaveToBag(VValueBag& CatalogBag, bool forJSON) const
{
	VValueBag* typeBag = new VValueBag();
	VError err = ToBag(*typeBag, false, true, forJSON, true);
	if (err == VE_OK)
	{
		CatalogBag.AddElement(d4::type, typeBag);
	}
	typeBag->Release();

	if (err == VE_OK)
	{
		for (ListOfTypes::const_iterator cur = fDerivateds.begin(), end = fDerivateds.end(); cur != end && err == VE_OK; cur++)
		{
			err = (*cur)->SaveToBag(CatalogBag, forJSON);
		}
	}

	return err;
}


bool AttributeType::NeedValidation() const
{
	bool needvalidate = false;

	if (fMin != nil)
		needvalidate = true;
	if (fMax != nil)
		needvalidate = true;
	if (!fPattern.IsEmpty())
		needvalidate = true;
	if (fFixedLength > 0)
		needvalidate = true;
	if (fMinLength > 0)
		needvalidate = true;
	if (fMaxLength > 0)
		needvalidate = true;
	return needvalidate;
}





// ----------------------------------------------------------------------------------------


inline EntityRelation::EntityRelation(const EntityRelationCollection& path, bool nTo1)
{
	fSubPath = path;
	fPath = path;

	for (EntityRelationCollection::iterator cur = fSubPath.begin(), end = fSubPath.end(); cur != end; cur++)
		(*cur)->Retain();

	fSourceAtt = fPath[0]->fSourceAtt;
	fDestAtt = fPath[fPath.size()-1]->fDestAtt;
	if (nTo1)
		fType = erel_Nto1;
	else
		fType = erel_1toN;
}


VError EntityRelation::ThrowError( VError inErrCode, const VString* p1) const
{
	VErrorDB4D_OnEMRelation *err = new VErrorDB4D_OnEMRelation(inErrCode, noaction, fSourceAtt == nil ? nil : fSourceAtt->GetOwner()->GetCatalog(), this);
	if (p1 != nil)
		err->GetBag()->SetString(Db4DError::Param1, *p1);
	VTask::GetCurrent()->PushRetainedError( err);

	return inErrCode;
}

/*
VError EntityRelation::FromBagArray(const VBagArray* bagarray, Base4D* owner, EntityModelCatalog* catalog)
{
	VError err = VE_OK;

	sLONG nbpart = bagarray->GetCount();
	fSubPath.reserve(nbpart);
	for (sLONG i = 1; i <= nbpart && err == VE_OK; i++)
	{
		const VValueBag* bagpart = bagarray->GetNth(i);
		if (bagpart != nil)
		{
			EntityRelation* subpart = new EntityRelation();
			err = subpart->FromBag(bagpart, owner, catalog);
			if (err == VE_OK)
			{
				const VString& subname = subpart->GetName();
				if (subname.IsEmpty())
				{
					if (subpart->fMissingSourceField.IsEmpty() || subpart->fMissingSourceTable.IsEmpty() || subpart->fMissingDestField.IsEmpty() || subpart->fMissingDestTable.IsEmpty())
					{
						err = ThrowError(VE_DB4D_ENTITY_RELATION_IS_MALFORMED);
					}
					else
					{
						subpart->Retain();
						fSubPath.push_back(subpart);
					}

				}
				else
				{
					EntityRelation* otherrel = catalog->FindRelation(subname);
					if (otherrel == nil)
					{
						if (subpart->fMissingSourceField.IsEmpty() || subpart->fMissingSourceTable.IsEmpty() || subpart->fMissingDestField.IsEmpty() || subpart->fMissingDestTable.IsEmpty())
						{
							err = ThrowError(VE_DB4D_ENTITY_RELATION_IS_MALFORMED);
						}
						else
						{
							subpart->fName.Clear();
							subpart->Retain();
							fSubPath.push_back(subpart);
						}
					}
					else
					{
						otherrel->Retain();
						fSubPath.push_back(otherrel);
					}
				}
			}
			QuickReleaseRefCountable(subpart);
		}
	}

	if (err == VE_OK)
	{
		RecalcPath();
	}


	return err;
}
*/

void EntityRelation::RecalcPath()
{
	fPath.clear();
	if (fSubPath.empty())
		fPath.push_back(this);
	else
	{
		for (EntityRelationCollection::iterator cur = fSubPath.begin(), end = fSubPath.end(); cur != end; cur++)
		{
			EntityRelation* subpart = *cur;
			for (EntityRelationCollection::iterator cur2 = subpart->fPath.begin(), end2 = subpart->fPath.end(); cur2 != end2; cur2++)
			{
				fPath.push_back(*cur2);
			}
		}
	}
}


bool EntityRelation::MatchesBeginingOf(const EntityRelation* otherRel) const
{
	bool res = true;
	if (fPath.size() > otherRel->fPath.size())
		res = false;
	else
	{
		for (EntityRelationCollection::const_iterator cur = fPath.begin(), end = fPath.end(), curOther = otherRel->fPath.begin(); cur != end; ++cur, ++curOther)
		{
			if ((*cur)->fSourceAtt != (*curOther)->fSourceAtt || (*cur)->fDestAtt != (*curOther)->fDestAtt)
			{
				res = false;
				break;
			}
		}
	}
	return res;
}

/*
VError EntityRelation::FromBag(const VValueBag* bag, Base4D* owner, EntityModelCatalog* catalog)
{
	VError err = VE_OK;

	if (!bag->GetString(d4::name, fName))
		bag->GetString(d4::relationship, fName);

	const VBagArray* relsbag = bag->GetElements(d4::relationPath);
	if (relsbag == nil)
	{
		VString tablenamesource, fieldnamesource, tablenamedest, fieldnamedest;
		sLONG xkind;
		VString kindname;
		if (bag->GetString(d4::kind, kindname) && bag->GetString(d4::sourceTable, tablenamesource) && bag->GetString(d4::sourceColumn, fieldnamesource)
			&& bag->GetString(d4::destinationTable, tablenamedest) && bag->GetString(d4::destinationColumn, fieldnamedest))
		{
			xkind = ERelTypes[kindname];
			fMissingDestField = fieldnamedest;
			fMissingDestTable = tablenamedest;
			fMissingSourceField = fieldnamesource;
			fMissingSourceTable = tablenamesource;

			if ((xkind == erel_Nto1 || xkind == erel_1toN))
			{
				fType = (EntityRelationKind)xkind;
				RecalcPath();
			}
			else
			{
				err = ThrowError(VE_DB4D_ENTITY_RELATION_KIND_IS_MISSING);
			}
		}
	}
	else
		err = FromBagArray(relsbag, owner, catalog);

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOT_BUILD_EM_RELATION_FROM_DEF);

	return err;
}
*/

/*
VError EntityRelation::ResolveMissingTables(EntityModelCatalog* catalog)
{
	VError err = VE_OK;
	for (EntityRelationCollection::iterator cur = fSubPath.begin(), end = fSubPath.end(); cur != end && err == VE_OK; cur++)
	{
		EntityRelation* subpart = *cur;
		err = subpart->ResolveMissingTables(catalog);
	}

	if (err == VE_OK)
	{
		if (!fMissingSourceTable.IsEmpty() && !fMissingSourceField.IsEmpty() && !fMissingDestTable.IsEmpty() && !fMissingDestField.IsEmpty())
		{
			Base4D* owner = catalog->GetOwner();
			Field* source = owner->FindAndRetainFieldRef(fMissingSourceTable, fMissingSourceField);
			Field* dest = owner->FindAndRetainFieldRef(fMissingDestTable, fMissingDestField);
			if (source == nil)
				err = ThrowError(VE_DB4D_ENTITY_RELATION_SOURCE_IS_MISSING);
			else
			{
				if (dest == nil)
					err = ThrowError(VE_DB4D_ENTITY_RELATION_DEST_IS_MISSING);
				else
				{
					fSource = source;
					fDest = dest;
					fMissingDestField.Clear();
					fMissingDestTable.Clear();
					fMissingSourceField.Clear();
					fMissingSourceTable.Clear();
				}
			}
			QuickReleaseRefCountable(source);
			QuickReleaseRefCountable(dest);
		}
	}
	return err;
}
*/

/*
VError EntityRelation::ToBag(VValueBag& outBag, bool forDax, bool forSave, bool forJSON, bool firstlevel) const
{
	VError err = VE_OK;

	if (!fName.IsEmpty())
	{
		if (firstlevel)
			outBag.SetString(d4::name, fName);
		else
			outBag.SetString(d4::relationship, fName);
	}

	if (fSource != nil && fDest != nil)
	{
		if (firstlevel || fName.IsEmpty())
		{
			VString tablenamesource, fieldnamesource, tablenamedest, fieldnamedest;
			fSource->GetName(fieldnamesource);
			fSource->GetOwner()->GetName(tablenamesource);
			fDest->GetName(fieldnamedest);
			fDest->GetOwner()->GetName(tablenamedest);

			outBag.SetString(d4::sourceTable, tablenamesource);
			outBag.SetString(d4::sourceColumn, fieldnamesource);
			outBag.SetString(d4::destinationTable, tablenamedest);
			outBag.SetString(d4::destinationColumn, fieldnamedest);
			outBag.SetString(d4::kind, ERelTypes[fType]);
		}
	}

	for (EntityRelationCollection::const_iterator cur = fSubPath.begin(), last = fSubPath.end(); cur != last && err == VE_OK; cur++)
	{
		VValueBag *relpartBag = new VValueBag();
		err = (*cur)->ToBag(*relpartBag, forDax, forSave, forJSON, false);

		outBag.AddElement(d4::relationPath, relpartBag);
		relpartBag->Release();
	}

	return err;
}
*/

// ----------------------------------------------------------------------------------------

CDB4DEntityRecord* EntityCollection::LoadEntityRecord(RecIDType posInCol, CDB4DBaseContext* context, VErrorDB4D& outError, DB4D_Way_of_Locking HowToLock)
{
	return LoadEntity(posInCol, ConvertContext(context), outError, HowToLock);
}

RecIDType EntityCollection::GetLength(CDB4DBaseContext* context)
{
	return GetLength(ConvertContext(context));
}


VError EntityCollection::DeleteEntities(CDB4DBaseContext* context, VDB4DProgressIndicator* InProgress, CDB4DEntityCollection* *outLocked)
{
	VError err = DropEntities(ConvertContext(context), InProgress, nil);
	return err;
}


VError EntityCollection::AddCollection(EntityCollection* other, BaseTaskInfo* context, bool atTheEnd)
{
	sLONG nbelem = other->GetLength(context);
	VError err = VE_OK;
	for (sLONG i = 0; i < nbelem && err == VE_OK; i++)
	{
		EntityRecord* erec = other->LoadEntity(i, context, err, DB4D_Do_Not_Lock);
		if (erec != nil)
		{
			AddEntity(erec, atTheEnd);
			erec->Release();
		}
	}
	return err;
}



VJSArray EntityCollection::ToJsArray(BaseTaskInfo* context, JS4D::ContextRef jscontext, EntityAttributeSortedSelection& whichAttributes, EntityAttributeSelection* expandAttributes, 
										EntityAttributeSortedSelection* sortingAttributes, bool withKey, bool allowEmptyAttList, sLONG from, sLONG count, VError& err, EntityCollection* withinCollection)
{
	VJSContext	vjsContext(jscontext);
	VJSArray outArr(vjsContext);
	err = VE_OK;
	EntityCollection* trueSel = sortingAttributes == nil || sortingAttributes->size() == 0 ? RetainRefCountable(this) : SortSel(err, sortingAttributes, context, nil);

	if (err == VE_OK && trueSel != nil)
	{
		Selection* withinSel = nil;
		if (withinCollection != nil)
		{
			LocalEntityCollection* localCol = dynamic_cast<LocalEntityCollection*>(withinCollection);
			if (localCol != nil)
			{
				withinSel = localCol->GetSel();
			}
		}
		EntityCollectionIter selIter(trueSel, context);
		if (from < 0)
			from = 0;
		sLONG qt = trueSel->GetLength(context);
		if (count < 0)
			count = qt;
		sLONG lastrow = from+count-1;
		if (lastrow >= qt)
			lastrow = qt-1;

		RecIDType curpos = from;
		EntityRecord* erec = selIter.SetCurPos(from, err);
		while (curpos <= lastrow && err == VE_OK)
		{
			if (erec != nil)
			{
				VJSObject subObj(outArr, -1);
				err = erec->ConvertToJSObject(subObj, whichAttributes, expandAttributes, sortingAttributes, withKey, allowEmptyAttList);
				if (err == VE_OK)
				{
					LocalEntityRecord* localRec = dynamic_cast<LocalEntityRecord*>(erec);
					if (localRec != nil && withinSel != nil)
					{
						RecIDType posInOldSel = withinSel->GetRecordPos(localRec->GetNum(), context);
						if (posInOldSel >= 0)
							subObj.SetProperty("__POSITION", posInOldSel);
					}
				}

			}
			QuickReleaseRefCountable(erec);
			erec = selIter.Next(err);
			++curpos;
		}

		QuickReleaseRefCountable(erec);
	}

	QuickReleaseRefCountable(trueSel);
	return outArr;
}


EntityCollection* EntityCollection::ProjectCollection(EntityAttribute* att, VError& err, BaseTaskInfo* context)
{
	EntityCollection* result = nil;

	EntityRelation* relpath = att->GetRelPath();
	EntityModel* relmodel = att->GetSubEntityModel();

	if (testAssert(relpath != nil) && relmodel != nil)
	{
		const vector<EntityRelation*>& path = relpath->GetPath();
		SearchTab query(relmodel);
		map<const EntityModel*, sLONG> dejainstance;

		bool first = true;
		for (vector<EntityRelation*>::const_iterator cur = path.begin(), end = path.end(); cur != end; cur++)
		{
			EntityModel* sourcemodel = (*cur)->GetSourceAtt()->GetOwner();
			EntityModel* destmodel = (*cur)->GetDestAtt()->GetOwner();
			sLONG instancedest = dejainstance[destmodel];
			if (sourcemodel == destmodel)
				dejainstance[destmodel]++;
			sLONG instancesource = dejainstance[sourcemodel];
			query.AddSearchLineJoinEm((*cur)->GetSourceAtt(), DB4D_Equal, (*cur)->GetDestAtt(), false, instancesource, instancedest);
			query.AddSearchLineBoolOper(DB4D_And);
		}

		query.AddSearchLineEmSel(this, dejainstance[GetModel()]);

		result = relmodel->executeQuery(&query, context, nil, nil, DB4D_Do_Not_Lock, 0, nil, &err);
	}

	return result;
}


VJSValue EntityCollection::ProjectAttribute(EntityAttribute* att, VError& err, BaseTaskInfo* context, JS4D::ContextRef jscontext)
{
	sLONG nbelem = GetLength(context);
	VJSContext	vjsContext(jscontext);
	VJSArray result(vjsContext);
	err = VE_OK;
	for (sLONG i = 0; i < nbelem && err == VE_OK; i++)
	{
		EntityRecord* erec = LoadEntity(i, context, err, DB4D_Do_Not_Lock);
		if (erec != nil)
		{
			EntityAttributeValue* xval = erec->getAttributeValue(att, err, context);
			if (err == VE_OK && xval != nil)
			{
				VValueSingle* cv = xval->getVValue();
				result.PushValue(*cv);
			}
			erec->Release();
		}
	}
	if (err != VE_OK)
	{
		VJSValue val(vjsContext);
		val.SetNull();
		return val;
	}
	else
	{
		return result;
	}

}


EntityCollection* EntityCollection::SortCollection(const VString& orderby, BaseTaskInfo* context, VError err, VDB4DProgressIndicator* InProgress)
{
	err = VE_OK;
	//EntityAttributeSortedSelection sortingAtts(fOwner, orderby, context, false);
	EntityAttributeSortedSelection sortingAtts(fOwner);
	sortingAtts.BuildFromString(orderby, context, false, true, nil);
	EntityCollection* newcol = SortSel(err, &sortingAtts, context, InProgress);
	return newcol;
}




VError EntityCollection::Compute(EntityAttributeSortedSelection& atts, VJSObject& outObj, BaseTaskInfo* context, JS4D::ContextRef jscontext, bool distinct)
{
	VJSContext	vjsContext(jscontext);
	VJSObject resultObj(vjsContext);
	outObj = resultObj;

	outObj.MakeEmpty();

	vector<computeResult> results;
	results.resize(atts.size());
	VError err = VE_OK;
	if (okperm(context, fOwner, DB4D_EM_Read_Perm) || okperm(context, fOwner, DB4D_EM_Update_Perm) || okperm(context, fOwner, DB4D_EM_Delete_Perm))
	{
		EntityCollection* sel = this;
		EntityCollection* selsorted = nil;
		if (distinct && !atts.empty())
		{
			const EntityAttribute* att = atts[0].fAttribute;
			EntityAttributeSortedSelection sortingAtt(fOwner);
			sortingAtt.AddAttribute(att, nil);
			selsorted = SortSel(err, &sortingAtt, context);
			if (selsorted != nil)
				sel = selsorted;
		}

		RecIDType nbelem = sel->GetLength(context);
		for (RecIDType i = 0; i < nbelem && err == VE_OK; ++i)
		{
			EntityRecord* rec = sel->LoadEntity(i, context, err, DB4D_Do_Not_Lock);
			if (rec != nil)
			{
				sLONG respos = 0;
				for (EntityAttributeSortedSelection::const_iterator cur = atts.begin(), end = atts.end(); cur != end && err == VE_OK; cur++, respos++)
				{
					computeResult& result = results[respos];
					EntityAttributeValue* eval = rec->getAttributeValue(cur->fAttribute, err, context);
					if (err == VE_OK)
					{
						if (eval->GetAttributeKind() == eav_vvalue)
						{
							VValueSingle* cv = eval->getVValue();
							if (cv != nil && !cv->IsNull())
							{
								if (result.fPrevious == nil || !result.fPrevious->EqualToSameKind(cv))
								{
									if (result.fPrevious == nil)
										result.fPrevious = cv->Clone();
									else
										result.fPrevious->FromValueSameKind(cv);
									result.fCountDistinct++;
									result.fSumDistinct += cv->GetReal();
								}
								result.fCount++;
								result.fSum += cv->GetReal();
								if (result.first)
								{
									result.fMinVal = cv->Clone();
									result.fMaxVal = cv->Clone();
									result.first = false;
								}
								else
								{
									if (result.fMinVal->CompareToSameKind(cv, true) == CR_BIGGER)
										result.fMinVal->FromValueSameKind(cv);
									if (result.fMaxVal->CompareToSameKind(cv, true) == CR_SMALLER)
										result.fMaxVal->FromValueSameKind(cv);
								}
							}
						}
					}
				}
				QuickReleaseRefCountable(rec);
			}
		}

		sLONG respos = 0;
		for (EntityAttributeSortedSelection::const_iterator cur = atts.begin(), end = atts.end(); cur != end && err == VE_OK; cur++, respos++)
		{
			computeResult& result = results[respos];
			VJSObject subobject(outObj, cur->fAttribute->GetName());
			subobject.SetProperty("count", result.fCount);
			if (distinct && respos == 0)
				subobject.SetProperty("countDistinct", result.fCountDistinct);
			if (!result.first)
			{
				if (cur->fAttribute->IsSummable())
				{
					if (distinct && respos == 0)
					{
						subobject.SetProperty("sumDistinct", result.fSumDistinct);
						if (result.fCountDistinct != 0)
							subobject.SetProperty("averageDistinct", result.fSumDistinct / (Real)result.fCountDistinct);
						else
							subobject.SetNullProperty("averageDistinct");
					}
					subobject.SetProperty("sum", result.fSum);
					if (result.fCount != 0)
						subobject.SetProperty("average", result.fSum / (Real)result.fCount);
					else
						subobject.SetNullProperty("average");
				}
				subobject.SetProperty("min", result.fMinVal);
				subobject.SetProperty("max", result.fMaxVal);
			}
			else
			{	
				if (cur->fAttribute->IsSummable())
				{
					if (distinct && respos == 0)
					{
						subobject.SetNullProperty("averageDistinct");
						subobject.SetProperty("sumDistinct", 0.0);
					}
					subobject.SetNullProperty("average");
					subobject.SetProperty("sum", 0.0);
				}
				subobject.SetNullProperty("min");
				subobject.SetNullProperty("max");
			}
		}
		QuickReleaseRefCountable(selsorted);
	}
	else
	{
		context->SetPermError();
		err = fOwner->ThrowError(VE_DB4D_NO_PERM_TO_READ);
	}
	return err;

}


VError EntityCollection::ComputeOnOneAttribute(const EntityAttribute* att, DB4D_ColumnFormulae action, VJSValue& outVal, BaseTaskInfo* context, JS4D::ContextRef jscontext)
{
	bool okresult = false;
	EntityModel* model = fOwner;
	EntityModel* em = model;
	EntityCollection* sel = this;
	VJSContext vjsContext(jscontext);
	VJSValue resultVal(vjsContext);
	resultVal.SetNull();
	VError err = VE_OK;

	if (att->GetModel() != model)
	{
		VString s = em->GetName()+"."+att->GetName();
		err = ThrowBaseError(VE_DB4D_ENTITY_ATTRIBUTE_IS_FROM_ANOTHER_DATACLASS, s);
	}
	else
	{
		if (action == DB4D_Count || action == DB4D_Count_distinct || action == DB4D_Min || action == DB4D_Max)
		{
			if (!att->IsStatable())
			{
				err = att->ThrowError(VE_DB4D_WRONG_ATTRIBUTE_KIND);
			}
		}
		else
		{
			if (!att->IsSummable())
			{
				err = att->ThrowError(VE_DB4D_WRONG_ATTRIBUTE_KIND);
			}
		}
	}

	if (err == VE_OK)
	{
		EntityCollection* selsorted = nil;
		bool withdistinct = false;

		if (action >= DB4D_Count_distinct && action <=DB4D_Sum_distinct)
		{
			withdistinct = true;
			EntityAttributeSortedSelection sortingAtt(em);
			sortingAtt.AddAttribute(att, nil);
			selsorted = SortSel(err, &sortingAtt, context);
			if (selsorted != nil)
				sel = selsorted;
		}

		/*
		if (att->GetKind() == eattr_storage)
		{
			Field* cri = att->RetainDirectField();
			if (cri != nil)
			{
				ColumnFormulas formules(em->GetMainTable());
				formules.AddAction(action, cri);
				VError err = formules.Execute(sel, ConvertContext(context), nil, nil, sel == selsorted);
				if (err == VE_OK)
				{
					VValueSingle* result = formules.GetResult(0);
					if (result != nil)
					{
						if (result->IsNull() && (action == DB4D_Sum || action == DB4D_Count))
							ioParms.ReturnNumber(0);
						else
							ioParms.ReturnVValue(*result);
						okresult = true;
					}
				}
				cri->Release();
			}
		}
		else
			*/
		{
			EntityAttributeSortedSelection attlist(em);
			attlist.AddAttribute(att, nil);
			VJSContext	vjsContext(jscontext);
			VJSObject result(vjsContext);

			err  = sel->Compute(attlist, result, context, jscontext, withdistinct);
			if (err == VE_OK)
			{
				VJSValue subatt(vjsContext);
				subatt = result.GetProperty(att->GetName());
				if (subatt.IsObject())
				{
					VJSObject subattobj = subatt.GetObject();

					VJSValue subresult(vjsContext);
					switch (action)
					{
						case DB4D_Sum:
							subresult = subattobj.GetProperty("sum");
							break;
						case DB4D_Average:
							subresult = subattobj.GetProperty("average");
							break;
						case DB4D_Min:
							subresult = subattobj.GetProperty("min");
							break;
						case DB4D_Max:
							subresult = subattobj.GetProperty("max");
							break;
						case DB4D_Count:
							subresult = subattobj.GetProperty("count");
							break;
						case DB4D_Sum_distinct:
							subresult = subattobj.GetProperty("sumDistinct");
							break;
						case DB4D_Average_distinct:
							subresult = subattobj.GetProperty("averageDistinct");
							break;
						case DB4D_Count_distinct:
							subresult = subattobj.GetProperty("countDistinct");
							break;
						default:
							break;
					}
					if ( (subresult.IsUndefined() || subresult.IsNull()) && (action == DB4D_Sum || action == DB4D_Count) )
					{
						resultVal.SetNumber(0);
					}
					else
					{
						if (subresult.IsUndefined())
							resultVal.SetNull();
						else
							resultVal = subresult;
					}
					okresult = true;
				}
			}
		}
		QuickReleaseRefCountable(selsorted);
	}

	outVal = resultVal;
	return err;
}


EntityCollectionIterator* EntityCollection::NewIterator(BaseTaskInfo* context)
{
	return new EntityCollectionIterator(this, context);
}



		// -----------------------------------


EntityCollectionIterator::EntityCollectionIterator(EntityCollection* collection, BaseTaskInfo* context)
{
	fContext = RetainRefCountable(context);
	fCollection = RetainRefCountable(collection);
	fCurpos = 0;
	fNbElems = collection->GetLength(context);
}


EntityCollectionIterator::~EntityCollectionIterator()
{
	QuickReleaseRefCountable(fContext);
	QuickReleaseRefCountable(fCollection);
}


EntityRecord* EntityCollectionIterator::_loadEntity(RecIDType atPos, VError& err)
{
	err = VE_OK;
	if (atPos == -1)
		return nil;
	else
		return fCollection->LoadEntity(atPos, fContext, err, DB4D_Do_Not_Lock);
}



EntityRecord* EntityCollectionIterator::First(VError& err)
{
	fCurpos = 0;
	if (fCurpos >= fNbElems)
		fCurpos = -1;
	return _loadEntity(fCurpos, err);
}


EntityRecord* EntityCollectionIterator::Next(VError& err)
{
	if (fCurpos != -1)
	{
		++fCurpos;
		if (fCurpos >= fNbElems)
			fCurpos = -1;
	}
	return _loadEntity(fCurpos, err);
}


RecIDType EntityCollectionIterator::GetCurPos() const
{
	return fCurpos;
}


EntityRecord* EntityCollectionIterator::SetCurPos(RecIDType pos, VError& err)
{
	if (pos >= fNbElems)
	{
		fCurpos = -1;
	}
	else
	{
		fCurpos = pos;
	}

	return _loadEntity(fCurpos, err);
}





// ----------------------------------------------------------------------------------------


EntityMethod::EntityMethod(EntityModel* owner)
{
	fOwner = owner;
	fKind = emeth_none;
	fScriptNum = 0;
	fOverWrite = false;
	fReturnTypeScalar = 0;
	fReturnTypeModel = nil;
	fReturnTypeIsSelection = false;
	fScope = escope_public_server;
	fUserDefined = false;
	fIs4DMethod = false;
}


EntityMethod::EntityMethod(EntityModel* owner, const VString& name, EntityMethodKind kind)
{
	fOwner = owner;
	fKind = kind;
	fScriptNum = 0;
	fOverWrite = false;
	fReturnTypeScalar = 0;
	fReturnTypeModel = nil;
	fReturnTypeIsSelection = false;
	fScope = escope_public;
	fUserDefined = false;
	fIs4DMethod = true;
	fName = name;
}


EntityMethod* EntityMethod::Clone(EntityModel* inModel) const
{
	EntityMethod* result = new EntityMethod(inModel);
	BaseClone(result);
	return result;
}

void EntityMethod::BaseClone(EntityMethod* result) const
{
	result->fStatement = fStatement;
	result->fFrom = fFrom;
	result->fName = fName;
	result->fKind = fKind;
	result->fScope = fScope;
	result->fReturnTypeScalar = fReturnTypeScalar;
	result->fReturnTypeModel = fReturnTypeModel;
	result->fReturnTypeIsSelection = fReturnTypeIsSelection;
	result->fReturnTypeString = fReturnTypeString;
	result->fOverWrite = false;
	result->fUserDefined = fUserDefined;
	result->fIs4DMethod = fIs4DMethod;
}


CDB4DEntityModel* EntityMethod::GetModel() const
{
	return fOwner;
}



VError EntityMethod::ThrowError( VError inErrCode, const VString* p1) const
{
	
	VErrorDB4D_OnEMMethod *err = new VErrorDB4D_OnEMMethod(inErrCode, noaction, fOwner->GetCatalog(), fOwner, this);
	if (p1 != nil)
		err->GetBag()->SetString(Db4DError::Param1, *p1);
	VTask::GetCurrent()->PushRetainedError( err);
	return inErrCode;
}


VError EntityMethod::FromJS(const VString& name, EntityMethodKind kind, const VString& from, EntityAttributeScope scope, EntityModelCatalog* catalog, ModelErrorReporter* errorReporter)
{
	VError err = VE_OK;

	fUserDefined = true;
	fName = name;
	fKind = kind;
	fFrom = from;
	fScope = scope;

	return err;
}


VError EntityMethod::FromBag(const VValueBag* bag, EntityModelCatalog* catalog, ModelErrorReporter* errorReporter)
{
	VError err = VE_OK;

	if (! bag->GetString(d4::name, fName))
	{
		if (errorReporter == nil)
			err = fOwner->ThrowError(VE_DB4D_ENTITY_METHOD_NAME_MISSING);
		else
		{
			ModelErrorContextInstaller errs(errorReporter);
			fOwner->ThrowError(VE_DB4D_ENTITY_METHOD_NAME_MISSING);
		}
	}

	sLONG xtype = 0;
	VString kindname;
	if (err == VE_OK)
	{
		if (! bag->GetString(d4::applyTo, kindname))
		{
			xtype = emeth_rec;
		}
		else
		{
			xtype = EmethTypes[kindname];
			if (xtype == 0)
			{
				if (errorReporter != nil)
				{
					xtype = emeth_rec;
					ModelErrorContextInstaller errs(errorReporter);
					ThrowError(VE_DB4D_ENTITY_METHOD_TYPE_INVALID);
				}
				else
					err = ThrowError(VE_DB4D_ENTITY_METHOD_TYPE_INVALID);
			}
		}
	}
	fKind = (EntityMethodKind)xtype;

	const VBagArray* paramsBag = bag->GetElements(d4::parameter);
	if (paramsBag != nil)
	{
		for (VIndex i = 1, nbparams = paramsBag->GetCount(); i <= nbparams; i++)
		{
			const VValueBag* parambag = paramsBag->GetNth(i);
			VString ss;
			if (parambag != nil && parambag->GetString(d4::name, ss))
			{
				if (!ss.IsEmpty())
				{
					fParams.push_back(ss);
				}
				else
				{
					if (errorReporter == nil)
						err = ThrowError(VE_DB4D_METHOD_PARAMETER_NAME_IS_INVALID);
					else
					{
						ModelErrorContextInstaller errs(errorReporter);
						ThrowError(VE_DB4D_METHOD_PARAMETER_NAME_IS_INVALID);
					}
				}
			}
			else
			{
				if (errorReporter == nil)
					err = ThrowError(VE_DB4D_METHOD_PARAMETER_NAME_IS_INVALID);
				else
				{
					ModelErrorContextInstaller errs(errorReporter);
					ThrowError(VE_DB4D_METHOD_PARAMETER_NAME_IS_INVALID);
				}
			}

		}
	}

	VString ss;
	if (bag->GetString(d4::from,ss))
	{
		fFrom = ss;
	}
	else if (bag->GetString(d4::source, ss))
	{
		fStatement = ss;
	}
	else
	{
		const VValueBag* sourcebag = bag->GetUniqueElement(d4::source);
		if (sourcebag != nil)
		{
			if (!sourcebag->GetCData(fStatement))
			{
				if (errorReporter == nil)
					err = ThrowError(VE_DB4D_METHOD_STATEMENT_IS_EMPTY);
				else
				{
					ModelErrorContextInstaller errs(errorReporter);
					ThrowError(VE_DB4D_METHOD_STATEMENT_IS_EMPTY);
				}
			}
		}
	}

	if (bag->GetString(d4::returnType, fReturnTypeString))
	{
		/*
		fReturnTypeScalar = 0;
		fReturnTypeModel = catalog->FindEntityBySingleEntityName(ss);
		if (fReturnTypeModel != nil)
		{
			fReturnTypeIsSelection = false;
		}
		else
		{
			fReturnTypeModel = catalog->FindEntity(ss);
			if (fReturnTypeModel != nil)
			{
				fReturnTypeIsSelection = true;
			}
			else
			{
				fReturnTypeScalar = EValPredefinedTypes[ss];
			}
		}
		*/

	}

	fScope = escope_public_server;
	if (bag->GetString(d4::scope, ss))
	{
		fScope = (EntityAttributeScope)EScopeKinds[ss];
		if (fScope == escope_none)
			fScope = escope_public_server;
	}

	fUserDefined = false;

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOT_BUILD_EM_METH_FROM_DEF);
	return err;
}


VError EntityMethod::ResolveType(EntityModelCatalog* catalog, ModelErrorReporter* errorReporter)
{
	VError err = VE_OK;
	if (fReturnTypeModel == nil && fReturnTypeScalar == 0 && !fReturnTypeString.IsEmpty())
	{
		fReturnTypeScalar = 0;
		fReturnTypeModel = catalog->FindEntityByCollectionName(fReturnTypeString);
		if (fReturnTypeModel != nil)
		{
			fReturnTypeIsSelection = false;
		}
		else
		{
			fReturnTypeModel = catalog->FindEntity(fReturnTypeString);
			if (fReturnTypeModel != nil)
			{
				fReturnTypeIsSelection = true;
			}
			else
			{
				fReturnTypeScalar = EValPredefinedTypes[fReturnTypeString];
			}
		}
	}
	return err;
}



VError EntityMethod::SetPermission(DB4D_EM_Perm inPerm, const VUUID& inGroupID)
{
	VError err = VE_OK;
	if (inPerm == DB4D_EM_Execute_Perm)
		fExecutePerm = inGroupID;
	else if (inPerm == DB4D_EM_Promote_Perm)
		fPromotePerm = inGroupID;
	else
		err = ThrowError(VE_DB4D_WRONG_PERM_REF);

	return err;
}

VError EntityMethod::GetPermission(DB4D_EM_Perm inPerm, VUUID& outGroupID) const
{
	VError err = VE_OK;
	if (inPerm == DB4D_EM_Execute_Perm)
		outGroupID = fExecutePerm;
	else if (inPerm == DB4D_EM_Promote_Perm)
		outGroupID = fPromotePerm;
	else
		err = ThrowError(VE_DB4D_WRONG_PERM_REF);
	return err;
}


bool EntityMethod::PermissionMatch(DB4D_EM_Perm inPerm, CUAGSession* inSession, CUAGThreadPrivilege* inThreadPrivilege) const
{
	return permissionMatch(inPerm, inSession, inThreadPrivilege);
}


bool EntityMethod::permissionMatch(DB4D_EM_Perm inPerm, CUAGSession* inSession, CUAGThreadPrivilege* inThreadPrivilege) const
{
	const VUUID* id = nil;
	bool ok = true;
	if (inPerm == DB4D_EM_Execute_Perm)
		id = &fExecutePerm;
	else if (inPerm == DB4D_EM_Promote_Perm)
		id = &fPromotePerm;
	else
		ok = false;

	if (ok)
	{
		if (id->IsNull())
			ok = true;
		else
			ok = inSession->BelongsTo(*id, inThreadPrivilege);
	}
	return ok;
}


void EntityMethod::ResolvePermissionsInheritance()
{
	VUUID xid;
	bool forced = false;
	fOwner->GetPermission(DB4D_EM_Execute_Perm, xid, forced);
	if ((fExecutePerm.IsNull() || forced) && !xid.IsNull())
	{
		fExecutePerm = xid;
	}

	VUUID pid;
	fOwner->GetPermission(DB4D_EM_Promote_Perm, pid, forced);
	if ((fPromotePerm.IsNull() || forced) && !pid.IsNull())
	{
		fPromotePerm = pid;
	}
}


VError EntityMethod::ToBag(VValueBag& outBag, bool forDax, bool forSave, bool forJSON) const
{
	VError err = VE_OK;
	outBag.SetString(d4::name, fName);
	outBag.SetString(d4::applyTo, EmethTypes[fKind]);
	if (fReturnTypeScalar != 0)
	{
		outBag.SetString(d4::returnType, EValPredefinedTypes[fReturnTypeScalar]);
	}
	else
	{
		if (fReturnTypeModel != nil)
		{
			if (fReturnTypeIsSelection)
			{
				outBag.SetString(d4::returnType, fReturnTypeModel->GetName());
			}
			else
			{
				outBag.SetString(d4::returnType, fReturnTypeModel->GetCollectionName());
			}
		}
	}

	if (fScope != escope_none)
	{
		outBag.SetString(d4::scope, EScopeKinds[(sLONG)fScope]);
	}

	if (!fParams.empty())
	{
		for (VectorOfVString::const_iterator cur = fParams.begin(), end = fParams.end(); cur != end; cur++)
		{
			BagElement params(outBag, d4::parameter);
			params->SetString(d4::name, *cur);
		}
	}
	if (fFrom.IsEmpty())
	{
		if (fStatement.IsEmpty())
		{
			outBag.SetString(d4::from, "remoteServer");
		}
		else
		{
			BagElement scriptbag(outBag, d4::source);
			scriptbag->SetCData(fStatement);
		}
	}
	else
		outBag.SetString(d4::from, fFrom);

	if (fUserDefined)
		outBag.SetBool(d4::userDefined, fUserDefined);

	return err;
}



bool EntityMethod::GetScriptObjFunc(VJSGlobalContext* jsglobcontext, BaseTaskInfo* context, VJSObject& outObjFunc) const
{
	bool found = false;
	if (!fFrom.IsEmpty())
	{
		JSEntityMethodReference methref(fOwner, this, fKind);

		if (!context->GetJSMethod(methref, outObjFunc))
		{
			VJSContext jscontext(jsglobcontext);
			VJSValue result(jscontext);
			jscontext.EvaluateScript(fFrom, nil, &result, nil, nil);
			if (!result.IsUndefined() && result.IsObject())
			{
				outObjFunc = result.GetObject();
				if (outObjFunc.IsFunction())
				{
					if (fExecutePerm.IsNull() && fPromotePerm.IsNull())
					{
						found = true;
						context->SetJSMethod(methref, outObjFunc);
					}
					else
					{
						VJSObject glueFunc(jscontext);
						//VJSValue glueresult(jscontext);
						bool okGlue = false;
						JSEntityMethodReference glueref;
						if (!context->GetJSMethod(glueref, glueFunc))
						{
							VFolder* compfolder = VDBMgr::GetManager()->RetainJSCodeResourceFolder();
							if (compfolder != nil)
							{
								VFile scriptfile(*compfolder, "methodCall.js");
								if (scriptfile.Exists())
								{
									StErrorContextInstaller errs(false);
									VJSValue xresult(jscontext);
									jscontext.EvaluateScript(&scriptfile, &xresult, nil, nil);
									if (!xresult.IsUndefined() && xresult.IsObject())
									{
										glueFunc = xresult.GetObject();
										if (glueFunc.IsFunction())
										{
											okGlue = true;
											context->SetJSMethod(glueref, glueFunc);
										}
									}

								}
							}
							QuickReleaseRefCountable(compfolder);

						}
						else
							okGlue = true;

						VJSValue promoteID(jscontext);
						if (fPromotePerm.IsNull())
							promoteID.SetNull();
						else
						{
							VString s;
							fPromotePerm.GetString(s);
							promoteID.SetString(s);
						}

						VJSValue execID(jscontext);
						if (fExecutePerm.IsNull())
							execID.SetNull();
						else
						{
							VString s;
							fExecutePerm.GetString(s);
							execID.SetString(s);
						}

						VJSValue newresult(jscontext);

						vector<VJSValue> params;
						params.push_back(outObjFunc);
						params.push_back(execID);
						params.push_back(promoteID);
						jscontext.GetGlobalObject().CallFunction(glueFunc, &params, &newresult);

						if (!newresult.IsUndefined() && newresult.IsObject())
						{
							outObjFunc = newresult.GetObject();
							if (outObjFunc.IsFunction())
							{		
								found = true;
								context->SetJSMethod(methref, outObjFunc);
							}
						}
					}
				}
			}
		}
		else found = true;
	}
	return found;
}


VError EntityMethod::Execute(EntityCollection* inSel, const vector<VJSValue>* inParams, BaseTaskInfo* context, VJSValue& outResult) const
{
	VError err = VE_OK;
	if (okperm(context, fExecutePerm))
	{
		if (context->GetJSContext() != nil)
		{
			VJSContext jscontext(context->GetJSContext());
			VJSObject therecord(VJSEntitySelection::CreateInstance(jscontext, inSel));

			VJSObject* objfunc = nil;
			VJSObject localObjFunc(jscontext);
			if (GetScriptObjFunc(context->GetJSContext(), context, localObjFunc))
				objfunc = &localObjFunc;
			if (objfunc == nil)
				objfunc = context->getContextOwner()->GetJSFunction(fScriptNum, fStatement, &fParams);

			XBOX::VJSException excep;
			if (objfunc != nil)
			{
				therecord.CallFunction(*objfunc, inParams, &outResult, excep);
			}
			else
			{
				err = ThrowError(VE_DB4D_JS_ERR);
			}

			if (!excep.IsEmpty())
			{
				ThrowJSExceptionAsError(jscontext, excep);
				err = ThrowError(VE_DB4D_JS_ERR);
			}
		}
		else
		{
			err = ThrowError(VE_DB4D_JS_ERR);
		}
	}
	else
	{
		err = ThrowError(VE_DB4D_NO_PERM_TO_EXECUTE);
		context->SetPermError();
	}
	return err;
}


VError EntityMethod::Execute(EntityRecord* inRec, const vector<VJSValue>* inParams, BaseTaskInfo* context, VJSValue& outResult) const
{
	VError err = VE_OK;
	if (okperm(context, fExecutePerm))
	{
		if (context->GetJSContext() != nil)
		{
			VJSContext jscontext(context->GetJSContext());
			VJSObject therecord(VJSEntitySelectionIterator::CreateInstance(jscontext, new EntitySelectionIterator(inRec,context)));
			VJSObject* objfunc = nil;
			VJSObject localObjFunc(jscontext);
			if (GetScriptObjFunc(context->GetJSContext(), context, localObjFunc))
				objfunc = &localObjFunc;
			if (objfunc == nil)
				objfunc = context->getContextOwner()->GetJSFunction(fScriptNum, fStatement, &fParams);

			XBOX::VJSException excep;
			if (objfunc != nil)
			{
				therecord.CallFunction(*objfunc, inParams, &outResult, excep);
			}
			else
				err = ThrowError(VE_DB4D_JS_ERR);

			if (!excep.IsEmpty())
			{
				ThrowJSExceptionAsError(jscontext, excep);
				err = ThrowError(VE_DB4D_JS_ERR);
			}
		}
		else
		{
			err = ThrowError(VE_DB4D_JS_ERR);
		}
	}
	else
	{
		err = ThrowError(VE_DB4D_NO_PERM_TO_EXECUTE);
		context->SetPermError();
	}
	return err;
}


VError EntityMethod::Execute(const vector<VJSValue>* inParams, BaseTaskInfo* context, VJSValue& outResult) const
{
	VError err = VE_OK;
	if (okperm(context, fExecutePerm))
	{
		if (context->GetJSContext() != nil)
		{
			VJSContext jscontext(context->GetJSContext());
			VJSObject therecord(VJSEntityModel::CreateInstance(jscontext, fOwner));
			VJSObject* objfunc = nil;
			VJSObject localObjFunc(jscontext);
			if (GetScriptObjFunc(context->GetJSContext(), context, localObjFunc))
				objfunc = &localObjFunc;
			if (objfunc == nil)
				objfunc = context->getContextOwner()->GetJSFunction(fScriptNum, fStatement, &fParams);

			XBOX::VJSException excep;
			if (objfunc != nil)
			{
				therecord.CallFunction(*objfunc, inParams, &outResult, excep);
			}
			else
				err = ThrowError(VE_DB4D_JS_ERR);

			if (!excep.IsEmpty())
			{
				ThrowJSExceptionAsError(jscontext, excep);
				err = ThrowError(VE_DB4D_JS_ERR);
			}
		}
		else
		{
			err = ThrowError(VE_DB4D_JS_ERR);
		}
	}
	else
	{
		err = ThrowError(VE_DB4D_NO_PERM_TO_EXECUTE);
		context->SetPermError();
	}
	return err;
}


VJSObject* EntityMethod::getFuncObject(BaseTaskInfo* context, VJSObject& outObjFunc) const
{
	VJSObject* objfunc = nil;
	if (context->GetJSContext() != nil)
	{
		if (GetScriptObjFunc(context->GetJSContext(), context, outObjFunc))
			objfunc = &outObjFunc;
		if (objfunc == nil)
			objfunc = context->getContextOwner()->GetJSFunction(fScriptNum, fStatement, &fParams);
		if (!fExecutePerm.IsNull() || !fPromotePerm.IsNull())
		{
			
		}
	}
	else
		ThrowError(VE_DB4D_JS_ERR);

	return objfunc;
}


VJSObject* EntityMethod::GetFuncObject(CDB4DBaseContext* inContext, VJSObject& outObjFunc) const
{
	return getFuncObject(ConvertContext(inContext), outObjFunc);
}


// ----------------------------------------------------------------------------------------

void EntityAttribute::xinit()
{
	fFrom = nil;
	fRelationPathID = -1;
	fPosInOwner = 0;
	fFieldPos = 0;
	fKind = eattr_none;
	fScope = escope_public;
	fSubEntity = nil;
	fOverWrite = false;
	fRelation = nil;
	fType = nil;
	fLocalType = nil;
	fCanBeModified = true;
	fScriptQuery = nil;
	fScriptDB4D = nil;
	fOptimizedScriptDB4D = nil;
	fScalarType = 0;
	fill(&fScriptNum[0], &fScriptNum[script_attr_last+1], 0);
	fExtraProperties = nil;
	fReversePath = false;
	fIsForeignKey = false;
	fIdentifying = false;
	fPartOfPrimKey = false;
	fIsMultiLine = false;
	fFlattenLastDest = nil;
	std::fill(&fScriptUserDefined[0], &fScriptUserDefined[script_attr_last+1], 0);
	fAutoGen = false;
	fAutoSeq = false;
	fStyledText = false;
	fOuterBlob = false;
	fNotNull = false;
	fNullToEmpty = false;
	fUnique = false;
	fLimitingLen = 0;
	fBlobSwitchSize = 0;
	fBehavesAsStorage = false;
	fSimpleDate = false;
	fRelIsNotOnPrimKey = false;
	fPictCacheDuration = 0;
}

EntityAttribute::EntityAttribute(EntityModel* owner)
{
	fOwner = owner;
	xinit();
}


EntityAttribute::~EntityAttribute()
{
	QuickReleaseRefCountable(fSubEntity);
	QuickReleaseRefCountable(fRelation);
	if (fOptimizedScriptDB4D != nil)
		delete fOptimizedScriptDB4D;
	if (fScriptQuery != nil)
		delete fScriptQuery;
	if (fScriptDB4D != nil)
		delete fScriptDB4D;
	QuickReleaseRefCountable(fExtraProperties);

}


EntityAttribute* EntityAttribute::Clone(EntityModel* inModel) const
{
	EntityAttribute* result = new EntityAttribute(inModel);
	BaseClone(result);
	return result;
}

void EntityAttribute::BaseClone(EntityAttribute* result) const
{
	result->fFrom = this;
	result->fName = fName;
	result->fKind = fKind;
	result->fScope = fScope;
	result->fRelationPathID = fRelationPathID;
	result->fPosInOwner = fPosInOwner;
	result->fFieldPos = fFieldPos;
	result->fSubEntityName = fSubEntityName;
	result->fReversePath = fReversePath;
	result->fRelPath = fRelPath;
	result->fSubEntity = RetainRefCountable(fSubEntity);
	result->fOverWrite = false;
	result->fFlattenColumnName = fFlattenColumnName;
	result->fFlattenLastDest = fFlattenLastDest;
	result->fRelation = RetainRefCountable(fRelation);
	result->fType = RetainRefCountable(fType);
	//	result->fCrit_UUID = fCrit_UUID;
	result->fIdentifying = fIdentifying;
	result->fIsMultiLine = fIsMultiLine;
	result->fAutoGen = fAutoGen;
	result->fAutoSeq = fAutoSeq;
	result->fStyledText = fStyledText;
	result->fOuterBlob = fOuterBlob;
	result->fNotNull = fNotNull;
	result->fNullToEmpty = fNullToEmpty;
	result->fUnique = fUnique;
	result->fLimitingLen = fLimitingLen;
	result->fBlobSwitchSize = fBlobSwitchSize;
	result->fBehavesAsStorage = fBehavesAsStorage;
	result->fSimpleDate = fSimpleDate;
	result->fPictCacheDuration = fPictCacheDuration;

	if (fLocalType == nil)
		result->fLocalType = nil;
	else
		result->fLocalType = fLocalType->Clone();
	if (fScriptQuery == nil)
		result->fScriptQuery = nil;
	else
	{
		result->fScriptQuery = new SearchTab(fOwner);
		result->fScriptQuery->From(*fScriptQuery);
	}
	if (fScriptDB4D == nil)
		result->fScriptDB4D = nil;
	else
	{
		result->fScriptDB4D = new SearchTab(fOwner);
		result->fScriptDB4D->From(*fScriptDB4D);
	}
	result->fScriptKind = fScriptKind;
	result->fIsForeignKey = fIsForeignKey;
	copy(&fScriptStatement[0], &fScriptStatement[script_attr_last+1], &(result->fScriptStatement[0]));
	copy(&fScriptFrom[0], &fScriptFrom[script_attr_last+1], &(result->fScriptFrom[0]));
	copy(&fScriptUserDefined[0], &fScriptUserDefined[script_attr_last+1], &(result->fScriptUserDefined[0]));

	//result->fDBEvents.CopyFrom(fOwner, fDBEvents);
	/*
	DBEventIterator curdest = &result->fEvents[dbev_firstEvent];
	for (DBEventConstIterator cur = &fEvents[dbev_firstEvent], end = &fEvents[dbev_lastEvent+1]; cur != end; cur++, curdest++)
	{
		if (cur->IsValid())
			curdest->CopyFrom(fOwner, cur);
	}
	*/
	//CopyRefCountable(&(result->fExtraProperties), fExtraProperties);
}


VError EntityAttribute::ThrowError( VError inErrCode, const VString* p1) const
{
	VErrorDB4D_OnEMAttribute *err = new VErrorDB4D_OnEMAttribute(inErrCode, noaction, fOwner->GetCatalog(), fOwner, this);
	if (p1 != nil)
		err->GetBag()->SetString(Db4DError::Param1, *p1);
	VTask::GetCurrent()->PushRetainedError( err);

	return inErrCode;
}



ValueKind EntityAttribute::GetDataKind() const
{
	/*
	if (fKind == eattr_alias)
	{
		return getFlattenTableDest()->GetFieldType(fFieldPos);
	}
	else if (fKind == eattr_storage)
	{
		return fOwner->GetMainTable()->GetFieldType(fFieldPos);
	}
	else if (fKind == eattr_computedField)
		return ComputeScalarType();
	else
		return VK_EMPTY;
		*/
	return ComputeScalarType();
}


OptimizedQuery* EntityAttribute::GetScriptDB4D(BaseTaskInfo* context) const
{
	OptimizedQuery* result = fOptimizedScriptDB4D;
	if (result == nil)
	{
		if (fScriptDB4D != nil)
		{
			result = new OptimizedQuery();
			VError err = result->AnalyseSearch(fScriptDB4D, context, true);
			if (err != VE_OK)
			{
				delete result;
				result = nil;
			}
			fOptimizedScriptDB4D = result;
		}
	}
	return result;
}


bool EntityAttribute::GetScriptObjFunc(script_attr whichscript, VJSGlobalContext* jsglobcontext, BaseTaskInfo* context, VJSObject& outObjFunc) const
{
	bool found = false;
	if (!fScriptFrom[whichscript].IsEmpty())
	{
		JSEntityMethodReference methref(fOwner, this, whichscript);

		if (!context->GetJSMethod(methref, outObjFunc))
		{
			VJSContext jscontext(jsglobcontext);
			VJSValue result(jscontext);
			jscontext.EvaluateScript(fScriptFrom[whichscript], nil, &result, nil, nil);
			if (!result.IsUndefined() && result.IsObject())
			{
				outObjFunc = result.GetObject();
#if debuglr
				VString sss;
				result.GetString(sss);
				VJSJSON json(jscontext);
				VString sjson;
				json.Stringify(result, sjson);
#endif
				if (outObjFunc.IsFunction())
				{
					found = true;
					context->SetJSMethod(methref, outObjFunc);
				}
			}
		}
		else
			found = true;
	}
	return found;
}


bool EntityAttribute::IsIndexed() const
{
	return fOwner->IsIndexed(this);
}


bool EntityAttribute::IsFullTextIndexed() const
{
	return fOwner->IsFullTextIndexed(this);
}



bool EntityAttribute::IsIndexable() const
{
	return fOwner->IsIndexable(this);
}


bool EntityAttribute::IsFullTextIndexable() const
{
	return fOwner->IsFullTextIndexable(this);
}




/*
EntityRelation* EntityAttribute::GetRelPath() const
{
	return fRelation;
}
*/

VError EntityAttribute::ScriptToBag(VValueBag& outBag, const VValueBag::StKey& inWhatScript, const VString& inStatement, const VString& inFrom, bool userDefined) const
{
	if (!inFrom.IsEmpty())
	{
		BagElement scriptbag(outBag, inWhatScript);
		scriptbag->SetString(d4::from, inFrom);
		if (userDefined)
			scriptbag->SetBool(d4::userDefined, userDefined);
	}
	else if (!inStatement.IsEmpty())
	{
		BagElement scriptbag(outBag, inWhatScript);
		scriptbag->SetCData(inStatement);
	}
	return VE_OK;
}


VError EntityAttribute::ToBag(VValueBag& outBag, bool forDax, bool forSave, bool forJSON, bool isTableDef) const
{
	VError err = VE_OK;

	if (fExtraProperties != nil)
		outBag.AddElement(d4::extraProperties, (VValueBag*)fExtraProperties);

	outBag.SetString(d4::name, fName);
	outBag.SetString(d4::kind, EattTypes[(sLONG)fKind]);
	if (!fAlternateFieldNames.empty())
	{
		VString s;
		for (VectorOfVString::const_iterator cur = fAlternateFieldNames.begin(), end = fAlternateFieldNames.end(); cur != end; ++cur)
		{
			if (!s.IsEmpty())
				s += ",";
			s += *cur;
		}
		outBag.SetString(d4::matchColumn, s);
	}
	/*
	if (fKind == eattr_storage && isTableDef)
		outBag.SetString(d4::kind, EattTypes[(sLONG)eattr_field]);
	else
		outBag.SetString(d4::kind, EattTypes[(sLONG)fKind]);
	*/

	if (fScope != escope_none)
	{
		outBag.SetString(d4::scope, EScopeKinds[(sLONG)fScope]);
	}

	if (!fDB4DUUID.IsNull())
		outBag.SetVUUID(d4::uuid, fDB4DUUID);

	if (fKind == eattr_alias || fKind == eattr_storage || fKind == eattr_computedField)
	{
		ValueKind xtype = ComputeScalarType();

		if (fKind != eattr_alias)
		{
			if (IsIndexed())
				outBag.SetBool(d4::indexed, true);
		}
		if (forSave)
		{
			if (fKind == eattr_storage/* && isTableDef*/)
			{
				//outBag.SetLong(d4::fieldPos, GetFieldPos());

				if (xtype == VK_TEXT || xtype == VK_TEXT_UTF8)
					outBag.SetBool(d4::textAsBlob, true);

				if (fLimitingLen != 0)
					outBag.SetLong(DB4DBagKeys::limiting_length, fLimitingLen);

				if (fBlobSwitchSize != 0)
					outBag.SetLong(DB4DBagKeys::blob_switch_size, fBlobSwitchSize);

				if (fUnique)
					outBag.SetBool(DB4DBagKeys::unique, fUnique);

				if (fNotNull)
					outBag.SetBool(DB4DBagKeys::not_null, fNotNull);

				if (fNullToEmpty)
					outBag.SetBool(DB4DBagKeys::never_null, fNullToEmpty);

				if (fAutoSeq)
					outBag.SetBool(DB4DBagKeys::autosequence, fAutoSeq);

				if (fAutoGen)
					outBag.SetBool(DB4DBagKeys::autogenerate, fAutoGen);

				if (fOuterBlob)
					outBag.SetBool(DB4DBagKeys::outside_blob, fOuterBlob);

				if (fStyledText)
					outBag.SetBool(DB4DBagKeys::styled_text, fStyledText);

			}
		}

		if (fPictCacheDuration > 0)
			outBag.SetLong(d4::cacheDuration, fPictCacheDuration);

		if (xtype == VK_STRING_UTF8 || xtype == VK_TEXT || xtype == VK_TEXT_UTF8)
			xtype = VK_STRING;

		if (forDax)
		{
			if (xtype >= VK_BYTE && xtype <= VK_FLOAT)
				xtype = VK_REAL;
		}

		bool putRealType = false;

		if (forSave)
		{
			if (fType == nil)
				putRealType = true;
			else
			{
				outBag.SetString(d4::type, fType->GetName());
			}
		}
		else
			putRealType = true;
		
		if (putRealType && xtype != 0)
		{
			outBag.SetString(d4::type, EValPredefinedTypes[xtype]);
		}

		if (xtype == VK_TIME)
			outBag.SetBool(d4::simpleDate, fSimpleDate);
	}
	else
	{
		if (fSubEntity != nil)
		{
			if (fKind == eattr_relation_Nto1)
				outBag.SetString(d4::type, fSubEntity->GetName());
			else
				outBag.SetString(d4::type, fSubEntity->GetCollectionName());
		}
	}
	
	if (forSave)
	{
		if (!fIndexKind.IsEmpty())
		{
			outBag.SetString(d4::indexKind, fIndexKind);
		}

		if (fIsForeignKey)
		{
		}
	}
	if (fReversePath)
		outBag.SetBool(d4::reversePath, true);
	if (!fRelPath.IsEmpty())
		outBag.SetString(d4::path, fRelPath);
/*
	if (fRelation != nil && forSave)
	{
		if (fRelation->GetName().IsEmpty())
		{
			VValueBag* relbag = new VValueBag();
			err = fRelation->ToBag(*relbag, forDax, forSave, forJSON, false);
			if (err == VE_OK)
				outBag.AddElement(d4::relationship, relbag);
			relbag->Release();
		}
		else
			outBag.SetString(d4::relationship, fRelation->GetName());
	}
	*/

	err = fDBEvents.ToBag(outBag, forDax, forSave, forJSON, true);
	/*
	for (DBEventConstIterator cur = &fEvents[dbev_firstEvent], end = &fEvents[dbev_lastEvent+1]; cur != end && err == VE_OK; cur++)
	{
		if (cur->IsValid() && (cur->IsOverWritten() || !forSave))
		{
			BagElement evBag(outBag, d4::events);
			err = cur->ToBag(*evBag, forDax, forSave, forJSON, true);
		}
	}
	*/

	if (forSave)
	{
		if (fKind == eattr_computedField && fScriptKind != script_none)
		{
			outBag.SetString(d4::scriptKind, ScriptTypes[fScriptKind]);
		}
		ScriptToBag(outBag, d4::onGet, fScriptStatement[script_attr_get], fScriptFrom[script_attr_get], fScriptUserDefined[script_attr_get]);
		ScriptToBag(outBag, d4::onSet, fScriptStatement[script_attr_set], fScriptFrom[script_attr_set], fScriptUserDefined[script_attr_set]);
		ScriptToBag(outBag, d4::onQuery, fScriptStatement[script_attr_query], fScriptFrom[script_attr_query], fScriptUserDefined[script_attr_query]);
		ScriptToBag(outBag, d4::onSort, fScriptStatement[script_attr_sort], fScriptFrom[script_attr_sort], fScriptUserDefined[script_attr_sort]);

		if (fLocalType != nil)
		{
			err = fLocalType->ToBag(outBag, forDax, forSave, forJSON, false);
		}
	}
	else
	{ 
		if (fLocalType != nil)
		{
			err = fLocalType->ToBag(outBag, forDax, forSave, forJSON, false);
		}
		else
		{
			if (fType != nil)
				err = fType->ToBag(outBag, forDax, forSave, forJSON, false);

		}
	}

	if (err == VE_OK)
	{
		if (fIdentifying)
			outBag.SetBool(d4::identifying, true);
		else
		{
			if (!forSave)
			{
				if (fPartOfPrimKey)
					outBag.SetBool(d4::identifying, true);
			}
		}
	}

	if (err == VE_OK)
	{
		if (fIsMultiLine)
		{
			outBag.SetBool(d4::multiLine, true);
		}
	}

	if (err == VE_OK)
	{
		if (forSave)
		{
			if (!fCanBeModified)
				outBag.SetBool(d4::readOnly, true);
		}
		else
		{
			if (!CanBeModified())
				outBag.SetBool(d4::readOnly, true);

		}
	}

	return err;
}


EntityAttribute* EntityAttribute::getFlattenAttribute() const
{
	if (fFlattenLastDest != nil)
		return fFlattenLastDest->getAttribute(fFlattenColumnName);
	else
		return nil;
}


VError EntityAttribute::GetScriptFromBag(const VValueBag* bag, EntityModelCatalog* catalog, const VValueBag::StKey& inScriptKey, script_attr inWhatScript, ModelErrorReporter* errorReporter)
{
	VError err = VE_OK;
	VString ss;
	if (bag->GetString(inScriptKey, ss))
	{
		fScriptStatement[inWhatScript] = ss;
	}
	else
	{
		const VValueBag* sourcebag = bag->GetUniqueElement(inScriptKey);
		if (sourcebag != nil)
		{
			if (sourcebag->GetString(d4::from, ss))
			{
				fScriptFrom[inWhatScript] = ss;
				fScriptUserDefined[inWhatScript] = false;
			}
			else if (!sourcebag->GetCData(fScriptStatement[inWhatScript]))
			{
				if (errorReporter == nil)
					err = ThrowError(VE_DB4D_SCRIPT_STATEMENT_IS_EMPTY);
				else
				{
					ModelErrorContextInstaller errs(errorReporter);
					ThrowError(VE_DB4D_SCRIPT_STATEMENT_IS_EMPTY);
				}
			}
		}
	}
	return err;
}



VError EntityAttribute::SetPermission(DB4D_EM_Perm inPerm, const VUUID& inGroupID)
{
	VError err = VE_OK;
	if (inPerm >= DB4D_EM_Read_Perm && inPerm <= DB4D_EM_Update_Perm)
	{
		fPerms[inPerm] = inGroupID;
	}
	else
		err = ThrowError(VE_DB4D_WRONG_PERM_REF);

	return err;
}

VError EntityAttribute::GetPermission(DB4D_EM_Perm inPerm, VUUID& outGroupID) const
{
	VError err = VE_OK;
	if (inPerm >= DB4D_EM_Read_Perm && inPerm <= DB4D_EM_Update_Perm)
	{
		outGroupID = fPerms[inPerm];
	}
	else
		err = ThrowError(VE_DB4D_WRONG_PERM_REF);

	return err;
}


bool EntityAttribute::PermissionMatch(DB4D_EM_Perm inPerm, CUAGSession* inSession, CUAGThreadPrivilege* inThreadPrivilege) const
{
	return permissionMatch(inPerm, inSession, inThreadPrivilege);
}


bool EntityAttribute::permissionMatch(DB4D_EM_Perm inPerm, CUAGSession* inSession, CUAGThreadPrivilege* inThreadPrivilege) const
{
	if (inPerm >= DB4D_EM_Read_Perm && inPerm <= DB4D_EM_Update_Perm)
	{
		const VUUID* id = &(fPerms[inPerm]);
		if (id->IsNull())
			return true;
		else
		{
			if (inSession == nil)
				return false;
			else
				return inSession->BelongsTo(*id, inThreadPrivilege);
		}
	}
	else
		return false;
}


bool EntityAttribute::permissionMatch(DB4D_EM_Perm inPerm, BaseTaskInfo* context) const
{
	if (context != nil)
	{
		return permissionMatch(inPerm, context->GetCurrentUserSession(), context->GetThreadPrivileges());
	}
	else
		return false;
}


void EntityAttribute::ResolvePermissionsInheritance()
{
	VUUID xid;
	bool forced = false;

	for (DB4D_EM_Perm perm = DB4D_EM_Read_Perm; perm < DB4D_EM_Delete_Perm; perm = (DB4D_EM_Perm)(perm+1))
	{
		VUUID xid;
		bool forced = false;
		fOwner->GetPermission(perm, xid, forced);
		if ((fPerms[perm].IsNull() || forced) && !xid.IsNull())
		{
			fPerms[perm] = xid;
		}
	}
}


VError EntityAttribute::FromBag(const VValueBag* bag, EntityModelCatalog* catalog, bool fieldmustexits, ModelErrorReporter* errorReporter, bool limitedChange)
{
	VError err = VE_OK;

	if (!limitedChange)
	{
		fExtraProperties = bag->RetainUniqueElement(d4::extraProperties);
		
		VUUID xid;
		if (bag->GetVUUID(d4::uuid, xid))
		{
			fDB4DUUID = xid;
		}
		else
			fDB4DUUID.SetNull();


		
		if (! bag->GetString(d4::name, fName))
		{
			if (errorReporter == nil)
				err = fOwner->ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_NAME_MISSING);
			else
			{
				ModelErrorContextInstaller errs(errorReporter);
				fOwner->ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_NAME_MISSING);
			}
		}
	}

	VString sFields;
	if (bag->GetString(d4::matchColumn, sFields) && !sFields.IsEmpty())
	{
		VectorOfVString fieldnames;
		sFields.GetSubStrings(',', fieldnames, false, true);
		if (!fieldnames.empty())
		{
			if (fAlternateFieldNames.empty())
				fAlternateFieldNames = fieldnames;
			else
			{
				for (VectorOfVString::iterator cur = fieldnames.begin(), end = fieldnames.end(); cur != end; ++cur)
				{
					fAlternateFieldNames.push_back(*cur);
				}
			}
		}
	}

	sLONG xtype = (sLONG)fKind;
	VString kindname;
	if (err == VE_OK && !limitedChange)
	{
		if (! bag->GetString(d4::kind, kindname))
		{
			if (fKind == eattr_none)
			{
				if (errorReporter != nil)
				{
					xtype = 1;
					ModelErrorContextInstaller errs(errorReporter);
					ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_TYPE_MISSING);
				}
				else
					err = ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_TYPE_MISSING);
			}
		}
		else
		{
			xtype = EattTypes[kindname];
			if (xtype == 0)
			{
				if (errorReporter != nil)
				{
					xtype = 1;
					ModelErrorContextInstaller errs(errorReporter);
					ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_TYPE_MISSING);
				}
				else
					err = ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_TYPE_MISSING);
			}
		}
	}

	EntityAttributeKind xkind;
	
	if (limitedChange)
		xkind = fKind;
	else
		xkind = (EntityAttributeKind)xtype;
	/*
	if (xkind == eattr_field)
		xkind = eattr_storage;
		*/

	if (!limitedChange)
	{
		bag->GetString(d4::indexKind, fIndexKind);
		fIdentifying = false;
		bag->GetBool(d4::identifying, fIdentifying);

		fSimpleDate = false;
		bag->GetBool(d4::simpleDate, fSimpleDate);
	}

	

	if (err == VE_OK)
	{
		if (xkind == eattr_relation_Nto1 || xkind == eattr_relation_1toN || xkind == eattr_alias || xkind == eattr_composition)
		{
			VString relpath;
			if (bag->GetString(d4::path, relpath))
			{
				if (!relpath.IsEmpty())
				{
					fRelPath = relpath;
				}
			}
		}
		else if (xkind == eattr_computedField)
		{
			if (!limitedChange)
			{
				err = GetScriptFromBag(bag, catalog, d4::script, script_attr_get, errorReporter);
				/*
				err = GetScriptFromBag(bag, catalog, d4::onGet, script_attr_get, errorReporter);
				if (err == VE_OK)
					err = GetScriptFromBag(bag, catalog, d4::onSet, script_attr_set, errorReporter);
				if (err == VE_OK)
					err = GetScriptFromBag(bag, catalog, d4::onQuery, script_attr_query, errorReporter);
				if (err == VE_OK)
					err = GetScriptFromBag(bag, catalog, d4::onSort, script_attr_sort, errorReporter);
					*/

				if (err == VE_OK)
				{
					VString ss;
					if (bag->GetString(d4::scriptKind, ss))
					{
						fScriptKind = (script_type)ScriptTypes[ss];
						if (fScriptKind == script_none)
						{
							if (errorReporter != nil)
							{
								fScriptKind = script_javascript;
								ModelErrorContextInstaller errs(errorReporter);
								ThrowError(VE_DB4D_SCRIPT_KIND_IS_UNKNOWN);
							}
							else
								err = ThrowError(VE_DB4D_SCRIPT_KIND_IS_UNKNOWN);
						}
					}
					else
						fScriptKind = script_javascript;
				}
			}
		}
	}

	if (err == VE_OK)
	{
		const VBagArray* DBEventsBag = bag->GetElements(d4::events);
		if (DBEventsBag != nil)
		{
			VIndex nbevents = DBEventsBag->GetCount();
			for (VIndex i = 1; i <= nbevents && err == VE_OK; i++)
			{
				const VValueBag* eventBag = DBEventsBag->GetNth(i);
				if (eventBag != nil)
				{
					DBEventKind evkind = dbev_none;
					VString ss;
					bool userDefined = false;
					eventBag->GetBool(d4::userDefined, userDefined);
					if (!userDefined || catalog->AllowParseUsedDefined())
					{
						if (eventBag->GetString(d4::kind, ss) && !ss.IsEmpty())
						{
							evkind = (DBEventKind)OldEDBEventKinds[ss];
							if (evkind == dbev_none)
								evkind = (DBEventKind)EDBEventKinds[ss];
						}
						if (evkind == dbev_none)
						{
							if (errorReporter == nil)
								err = ThrowError(VE_DB4D_MISSING_OR_INVALID_EVENTKIND);
							else
							{
								ModelErrorContextInstaller errs(errorReporter);
								ThrowError(VE_DB4D_MISSING_OR_INVALID_EVENTKIND);
							}
						}
						else
						{
							DBEvent event;
							err = event.FromBag(fOwner, eventBag, catalog, errorReporter);
							if (err == VE_OK && event.IsValid())
								fDBEvents.AddEvent(event);
							//err = fEvents[evkind].FromBag(fOwner, eventBag, catalog, errorReporter);
						}
					}
				}
			}
		}
	}

	if (xkind == eattr_storage)
	{
		sLONG limitlen = 0;
		if (bag->GetLong(DB4DBagKeys::limiting_length, limitlen))
			fLimitingLen = limitlen;

		sLONG switchsize = 0;
		if (bag->GetLong(DB4DBagKeys::text_switch_size, switchsize))
			fBlobSwitchSize = switchsize;

		switchsize = 0;
		if (bag->GetLong(DB4DBagKeys::blob_switch_size, switchsize))
			fBlobSwitchSize = switchsize;

		bool uniq = false;
		if (bag->GetBool(DB4DBagKeys::unique, uniq))
			fUnique = uniq;

		bool notnull = false;
		if (bag->GetBool(DB4DBagKeys::not_null, notnull))
			fNotNull = notnull;

		bool nulltoempty = false;
		if (bag->GetBool(DB4DBagKeys::never_null, nulltoempty))
			fNullToEmpty = nulltoempty;

		bool autotseq = false;
		if (bag->GetBool(DB4DBagKeys::autosequence, autotseq))
			fAutoSeq = autotseq;

		bool autogen = false;
		if (bag->GetBool(DB4DBagKeys::autogenerate, autogen))
			fAutoGen = autogen;

		bool outsideblob = false;
		if (bag->GetBool(DB4DBagKeys::outside_blob, outsideblob))
			fOuterBlob = outsideblob;

		bool styledtext = false;
		if (bag->GetBool(DB4DBagKeys::styled_text, styledtext))
			fStyledText = styledtext;		
	}

	sLONG cacheDuration = 0;
	if (bag->GetLong(d4::cacheDuration, cacheDuration))
		fPictCacheDuration = cacheDuration;

	if (err == VE_OK)
	{
		if (xkind == eattr_relation_Nto1 || xkind == eattr_relation_1toN || xkind == eattr_composition)
		{
			if (!limitedChange)
				fReversePath = false;
			bool reversepath = false;
			if (bag->GetBool(d4::reversePath, reversepath))
				fReversePath = reversepath;
			VString subentityname;
			if (bag->GetString(d4::type, subentityname))
			{
				fSubEntityName = subentityname;
			}
			{
				if (fSubEntityName.IsEmpty())
				{
					if (errorReporter == nil)
						err = ThrowError(VE_DB4D_ATT_RELATED_ENTITY_IS_MISSING);
					else
					{
						ModelErrorContextInstaller errs(errorReporter);
						ThrowError(VE_DB4D_ATT_RELATED_ENTITY_IS_MISSING);
					}

				}
			}

		}
		else
		{
			if (!limitedChange)
			{
				VString TypeName;
				if (bag->GetString(d4::type, TypeName))
				{
					sLONG scalartype = EValPredefinedTypes[TypeName];
					if (scalartype == VK_FLOAT)
						scalartype = VK_REAL;
					if (scalartype == 0)
					{
						AttributeType* atttype = catalog->FindType(TypeName);
						if (atttype == nil)
						{
							if (errorReporter != nil)
							{
								fScalarType = 1;
								ModelErrorContextInstaller errs(errorReporter);
								ThrowError(VE_DB4D_TYPE_DOES_NOT_EXIST, &TypeName);
							}
							else
								err = ThrowError(VE_DB4D_TYPE_DOES_NOT_EXIST, &TypeName);
						}
						else
						{
							CopyRefCountable(&fType, atttype);
							fScalarType = fType->ComputeScalarType();
						}
					}
					else
						fScalarType = scalartype;
				}
				AttributeType* tempType = fLocalType;
				if (tempType == nil)
				{
					tempType = new AttributeType(fOwner->GetCatalog());
				}
				tempType->FromBag(bag, catalog, false, errorReporter);
				if (tempType->IsEmpty())
				{
					if (tempType != fLocalType)
						tempType->Release();
				}
				else
				{
					if (fType != nil)
						tempType->ExtendsFrom(fType);
					fLocalType = tempType;
					fLocalType->ReCalc();
				}
			}
		}
	}

	if (err == VE_OK && !limitedChange)
		fKind = xkind;

	if (err == VE_OK)
	{
		bool readonly;
		if (bag->GetBool(d4::readOnly, readonly))
			fCanBeModified = !readonly;
		else
		{
			if (!limitedChange)
				fCanBeModified = true;
		}
	}

	if (err == VE_OK)
	{
		bool multiline;
		if (bag->GetBool(d4::multiLine, multiline))
			fIsMultiLine = multiline;
		else
		{
			if (!limitedChange)
				fIsMultiLine = false;
		}

		if (xkind == eattr_relation_Nto1 || xkind == eattr_storage)
		{
			{
				bool oktouch = true;
				if (xkind == eattr_relation_Nto1)
				{
					VectorOfVString path;
					fRelPath.GetSubStrings('.', path, false, true);
					if (path.size() > 1)
						oktouch = false;
				}
				if (oktouch)
					catalog->TouchXML();
			}
		}
	}

	VString ss;
	if (!limitedChange)
		fScope = escope_public;
	if (bag->GetString(d4::scope, ss))
	{
		fScope = (EntityAttributeScope)EScopeKinds[ss];
		if (fScope == escope_none)
			fScope = escope_public;
	}

	if (err == VE_OK)
	{
		err = AssociateJSEvents(catalog, errorReporter);
	}

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOT_BUILD_EM_ATT_FROM_DEF);
	return err;
}


VError EntityAttribute::AssociateJSEvents(EntityModelCatalog* catalog, ModelErrorReporter* errorReporter)
{
	VError err = VE_OK;

	VJSContext	jscontext(*(catalog->getLoadingContext()));
	VJSObject	result(jscontext);
	VString		path = catalog->GetModelJSName()+"." + fOwner->GetName() + "." + fName;

	/*
	jscontext.DisableDebugger();
	jscontext.EvaluateScript(path, nil, &result, nil, nil);
	*/

	bool				isOk = false;
	VJSException		exception;

	result = jscontext.GetGlobalObject().GetPropertyAsObject("model", exception);
	if (exception.IsEmpty() && result.IsObject()) {

		result = result.GetPropertyAsObject(fOwner->GetName(), exception);
		if (exception.IsEmpty() && result.IsObject()) {

			result = result.GetPropertyAsObject(fName, exception);
			isOk = exception.IsEmpty() && result.IsObject();

		}

	}

	if (isOk /*result.IsObject() */)
	{
		VJSObject attObj(result);
		VJSPropertyIterator curprop(attObj);
		while (curprop.IsValid())
		{
			VString propName;
			curprop.GetPropertyName(propName);
			if (propName.EqualToUSASCIICString("events"))
			{
				VJSValue eventsVal = curprop.GetProperty();
				if (eventsVal.IsObject())
				{
					VJSObject eventsObj(eventsVal.GetObject());
					VJSPropertyIterator curevent(eventsObj);
					while (curevent.IsValid())
					{
						VString eventName;
						curevent.GetPropertyName(eventName);
						bool oldway = false;
						DBEventKind evkind = (DBEventKind)OldEDBEventKinds[eventName];
						if (evkind == dbev_none)
							evkind = (DBEventKind)EDBEventKinds[eventName];
						else
							oldway = true;
						if (evkind != dbev_none)
						{
							VJSValue eventFunc = curevent.GetProperty();
							if (eventFunc.IsFunction())
							{
								VString path2 = path + ".events." + eventName;
								DBEvent event;
								event.SetEvent(fOwner, evkind, path2, oldway);
								fDBEvents.AddEvent(event);
								SetOverWrite(true);
								//fEvents[evkind].SetEvent(fOwner, evkind, path2, oldway);
							}
							else if (eventFunc.IsArray())
							{
								VJSArray arr(eventFunc, false);
								for (sLONG i = 0, nb = arr.GetLength(); i < nb; ++i)
								{
									VJSValue eventFunc2 = arr.GetValueAt(i);
									if (eventFunc2.IsFunction())
									{
										VString path2 = path + ".events." + eventName + "[" + ToString(i) + "]";
										DBEvent event;
										event.SetEvent(fOwner, evkind, path2, oldway);
										fDBEvents.AddEvent(event);
										SetOverWrite(true);
										//fEvents[evkind].SetEvent(fOwner, evkind, path2, oldway);
									}
								}
							}
						}

						++curevent;
					}
				}
			}
			else
			{
				if (fKind == eattr_computedField)
				{
					script_attr whichscript = script_attr_none;
					if (propName.EqualToUSASCIICString("onGet"))
						whichscript = script_attr_get;
					else if (propName.EqualToUSASCIICString("onSet"))
						whichscript = script_attr_set;
					else if (propName.EqualToUSASCIICString("onQuery"))
						whichscript = script_attr_query;
					else if (propName.EqualToUSASCIICString("onSort") || propName.EqualToUSASCIICString("onOrderBy"))
						whichscript = script_attr_sort;
					if (whichscript != script_attr_none)
					{
						VString path2 = path + "." + propName;
						VJSValue funcobj = curprop.GetProperty();
						if (funcobj.IsFunction())
						{
							SetOverWrite(true);
							fScriptFrom[whichscript] = path2;
							fScriptUserDefined[whichscript] = true;
						}
					}
				}

			}

			++curprop;
		}

	}

	return err;
}


VError EntityAttribute::ResolveRelatedEntities(SubPathRefSet& already, EntityModelCatalog* catalog, ModelErrorReporter* errorReporter, BaseTaskInfo* context)
{
	VError err = VE_OK;

	if (fRelation == nil)
	{
		if (fFrom != nil)
		{
			((EntityAttribute*)fFrom)->ResolveRelatedEntities(already, catalog, errorReporter, context);
			fRelation = RetainRefCountable(fFrom->fRelation);
			if (fSubEntity == nil)
				fSubEntity = RetainRefCountable(fFrom->fSubEntity);
			//fFlattenTableDest = fFrom->fFlattenTableDest;
			fFieldPos = fFrom->fFieldPos;
			fFlattenColumnName = fFrom->fFlattenColumnName;
			fFlattenLastDest = fFrom->fFlattenLastDest;
			fSubEntityName = fFrom->fSubEntityName;
			if (fRelation != nil)
			{
				fRelationPathID = fOwner->FindRelationPath(fRelation);
				if (fRelationPathID == -1)
					fRelationPathID = fOwner->AddRelationPath(fRelation);
			}
		}
		else
		{
			if (fKind == eattr_relation_Nto1 || fKind == eattr_relation_1toN || fKind == eattr_composition)
			{
				if (fSubEntity == nil)
					fSubEntity = catalog->RetainEntity(fSubEntityName);
				if (/*fKind == eattr_relation_Nto1 && */fSubEntity == nil)
				{
					fSubEntity = catalog->RetainEntityByCollectionName(fSubEntityName);
				}
				if (fSubEntity == nil)
				{
					if (errorReporter == nil)
						err = ThrowError(VE_DB4D_ATT_RELATED_ENTITY_NOT_FOUND);
					else
					{
						ModelErrorContextInstaller errs(errorReporter);
						ThrowError(VE_DB4D_ATT_RELATED_ENTITY_NOT_FOUND);
					}
				}
				else
				{
					VectorOfVString path;
					fRelPath.GetSubStrings('.', path, false, true);

					if (fKind == eattr_relation_Nto1)
					{
						if (path.empty())
							path.push_back(fSubEntityName);
						bool mustBuildForeignKey = false;
						if (path.size() == 1)
						{
							VString s = path[0];
							VString s2;
							VectorOfVString subpath;
							s.GetSubStrings(':', subpath, false, true);
							if (subpath.size() > 1)
							{
								s = subpath[0];
								s2 = subpath[1];
							}
							EntityModel* otherEntity = catalog->RetainEntity(s);
							if (otherEntity == nil)
								otherEntity = catalog->RetainEntityByCollectionName(s);
							if (otherEntity != nil)
							{
								fIsForeignKey = true;
								mustBuildForeignKey = true;

								if (s2.IsEmpty())
								{
									const IdentifyingAttributeCollection* otherPrim = otherEntity->GetPrimaryAtts();
									if (otherPrim->empty())
									{
										if (errorReporter == nil)
											err = otherEntity->ThrowError(VE_DB4D_ENTITY_HAS_NO_PRIMKEY);
										else
										{
											ModelErrorContextInstaller errs(errorReporter);
											otherEntity->ThrowError(VE_DB4D_ENTITY_HAS_NO_PRIMKEY);
										}
									}
									else
									{
										if (otherPrim->size() != 1)
										{
											if (errorReporter == nil)
												otherEntity->ThrowError(VE_DB4D_PRIMKEY_MUST_BE_ONE_FIELD_ONLY);
											else
											{
												ModelErrorContextInstaller errs(errorReporter);
												otherEntity->ThrowError(VE_DB4D_PRIMKEY_MUST_BE_ONE_FIELD_ONLY);
											}
										}
										else
										{
											const EntityAttribute* otherAtt = (*otherPrim)[0].fAtt;

											sLONG typ = otherAtt->ComputeScalarType();
											fScalarType = typ;

											EntityRelation* rel = new EntityRelation(this, otherAtt, erel_Nto1);
											rel->RecalcPath();
											fRelationPathID = fOwner->AddRelationPath(rel);
											catalog->AddOneEntityRelation(rel, false);
											fRelation = rel;
										}
									}
								}
								else
								{
									const EntityAttribute* otherAtt = otherEntity->getAttribute(s2);
									if (otherAtt == nil)
									{
										if (errorReporter == nil)
											otherEntity->ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_NOT_FOUND, &s2);
										else
										{
											ModelErrorContextInstaller errs(errorReporter);
											otherEntity->ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_NOT_FOUND, &s2);
										}
									}
									else
									{
										sLONG typ = otherAtt->ComputeScalarType();
										fScalarType = typ;
										EntityRelation* rel = new EntityRelation(this, otherAtt, erel_Nto1);
										rel->RecalcPath();
										fRelationPathID = fOwner->AddRelationPath(rel);
										catalog->AddOneEntityRelation(rel, false);
										fRelation = rel;
										fRelIsNotOnPrimKey = true;
									}

								}

								otherEntity->Release();
							}

						}
						if (!mustBuildForeignKey)
						{
							EntityRelationCollection relpath;
							VString lastpart;
							bool allNto1 = true;
							EntityModel* lastDest = nil;
							err = fOwner->BuildRelPath(already, catalog, path, relpath, lastpart, allNto1, errorReporter, lastDest);

							if (err == VE_OK)
							{
								if (allNto1)
								{
									if (lastpart.IsEmpty())
									{
										fRelation = catalog->FindOrBuildRelation(relpath, allNto1);
										if (testAssert(fRelation != nil))
										{
											fRelation->Retain();
											fRelationPathID = fOwner->FindRelationPath(fRelation);
											if (fRelationPathID == -1)
												fRelationPathID = fOwner->AddRelationPath(fRelation);
										}
									}
									else
									{
										if (errorReporter == nil)
											err = ThrowError(VE_DB4D_NAVIGATION_PATH_IS_MALFORMED, &fRelPath);
										else
										{
											ModelErrorContextInstaller errs(errorReporter);
											ThrowError(VE_DB4D_NAVIGATION_PATH_IS_MALFORMED, &fRelPath);
										}
									}
								}
								else
								{
									if (errorReporter == nil)
										err = ThrowError(VE_DB4D_NAVIGATION_ATTRIBUTE_MUST_BE_Nto1, &fRelPath);
									else
									{
										ModelErrorContextInstaller errs(errorReporter);
										ThrowError(VE_DB4D_NAVIGATION_ATTRIBUTE_MUST_BE_Nto1, &fRelPath);
									}
								}

							}

						}
					}
					else
					{
						if (path.empty())
						{
							if (errorReporter == nil)
								err = ThrowError(VE_DB4D_NAVIGATION_PATH_IS_EMPTY);
							else
							{
								ModelErrorContextInstaller errs(errorReporter);
								ThrowError(VE_DB4D_NAVIGATION_PATH_IS_EMPTY);
							}
						}
						else
						{
							EntityRelationCollection relpath;
							VString lastpart;
							bool allNto1 = true;
							EntityModel* lastDest = nil;
							if (fReversePath)
								err = fSubEntity->BuildRelPath(already, catalog, path, relpath, lastpart, allNto1, errorReporter, lastDest);
							else
								err = fOwner->BuildRelPath(already, catalog, path, relpath, lastpart, allNto1, errorReporter, lastDest);
							if (err == VE_OK)
							{
								if (lastpart.IsEmpty())
								{
									EntityRelationCollection reversePath;
									if (fReversePath)
									{
										catalog->ReversePath(relpath, reversePath);
										fRelation = catalog->FindOrBuildRelation(reversePath, false);
									}
									else
										fRelation = catalog->FindOrBuildRelation(relpath, false);
									if (testAssert(fRelation != nil))
									{
										fRelation->Retain();
										fRelationPathID = fOwner->FindRelationPath(fRelation);
										if (fRelationPathID == -1)
											fRelationPathID = fOwner->AddRelationPath(fRelation);
									}
									if (fReversePath)
									{
										for (EntityRelationCollection::iterator cur = reversePath.begin(), end = reversePath.end(); cur != end; cur++)
											(*cur)->Release();
									}
								}
								else
								{
									if (errorReporter == nil)
										err = ThrowError(VE_DB4D_NAVIGATION_PATH_IS_MALFORMED, &fRelPath);
									else
									{
										ModelErrorContextInstaller errs(errorReporter);
										ThrowError(VE_DB4D_NAVIGATION_PATH_IS_MALFORMED, &fRelPath);
									}
								}
							}
						}
					}
				}
			}
			else if (fKind == eattr_alias)
			{
				EntityModel* lastDest = nil;
				VectorOfVString path;
				fRelPath.GetSubStrings('.', path, false, true);
				if (path.size() < 2)
				{
					if (errorReporter == nil)
						err = ThrowError(VE_DB4D_NAVIGATION_PATH_IS_MALFORMED, &fRelPath);
					else
					{
						ModelErrorContextInstaller errs(errorReporter);
						ThrowError(VE_DB4D_NAVIGATION_PATH_IS_MALFORMED, &fRelPath);
					}
				}
				else
				{
					EntityRelationCollection relpath;
					VString lastpart;
					bool allNto1 = true;
					err = fOwner->BuildRelPath(already, catalog, path, relpath, lastpart, allNto1, errorReporter, lastDest);
					if (allNto1)
					{
						if (lastpart.IsEmpty())
						{
							if (errorReporter == nil)
								err = ThrowError(VE_DB4D_ATT_FIELD_IS_MISSING);
							else
							{
								ModelErrorContextInstaller errs(errorReporter);
								ThrowError(VE_DB4D_ATT_FIELD_IS_MISSING);
							}
						}
						else
						{
							fRelation = catalog->FindOrBuildRelation(relpath, allNto1);
							if (testAssert(fRelation != nil))
							{
								fRelation->Retain();
								fRelationPathID = fOwner->FindRelationPath(fRelation);
								if (fRelationPathID == -1)
									fRelationPathID = fOwner->AddRelationPath(fRelation);
								fFlattenColumnName = lastpart;
								fFlattenLastDest = lastDest;
							}
						}
					}
					else
					{
						if (errorReporter == nil)
							err = ThrowError(VE_DB4D_NAVIGATION_ATTRIBUTE_MUST_BE_Nto1, &fRelPath);
						else
						{
							ModelErrorContextInstaller errs(errorReporter);
							ThrowError(VE_DB4D_NAVIGATION_ATTRIBUTE_MUST_BE_Nto1, &fRelPath);
						}
					}
				}

				if (fRelation != nil)
				{
					if (!fRelation->GetPath().empty())
					{
						if (lastDest != nil)
						{
							fSubEntity = RetainRefCountable(lastDest);
							fSubEntityName = fSubEntity->GetName();
						}
						if (!fFlattenColumnName.IsEmpty())
						{
							const EntityAttribute* flattenDestAtt = nil;
							if (lastDest != nil)
							{
								flattenDestAtt = lastDest->getAttribute(fFlattenColumnName);
							}
							if (flattenDestAtt == nil)
							{
								if (errorReporter == nil)
									err = ThrowError(VE_DB4D_ATT_FIELD_NOT_FOUND, &fFlattenColumnName);
								else
								{
									ModelErrorContextInstaller errs(errorReporter);
									ThrowError(VE_DB4D_ATT_FIELD_NOT_FOUND, &fFlattenColumnName);
								}
							}
							else
							{
								if (flattenDestAtt->IsScalar())
								{
								}
								else
								{
									if (errorReporter == nil)
										err = ThrowError(VE_DB4D_ATT_FIELD_NOT_FOUND, &fFlattenColumnName);
									else
									{
										ModelErrorContextInstaller errs(errorReporter);
										ThrowError(VE_DB4D_ATT_FIELD_NOT_FOUND, &fFlattenColumnName);
									}
								}
							}
							
						}
						else
						{
							if (errorReporter == nil)
								err = ThrowError(VE_DB4D_ATT_FIELD_IS_MISSING);
							else
							{
								ModelErrorContextInstaller errs(errorReporter);
								ThrowError(VE_DB4D_ATT_FIELD_IS_MISSING);
							}
						}
					}
					else
					{
						if (errorReporter == nil)
							err = ThrowError(VE_DB4D_ENTITY_RELATION_IS_EMPTY);
						else
						{
							ModelErrorContextInstaller errs(errorReporter);
							ThrowError(VE_DB4D_ENTITY_RELATION_IS_EMPTY);
						}
					}
				}
				else
				{
					if (errorReporter == nil)
						err = ThrowError(VE_DB4D_ENTITY_RELATION_IS_EMPTY);
					else
					{
						ModelErrorContextInstaller errs(errorReporter);
						ThrowError(VE_DB4D_ENTITY_RELATION_IS_EMPTY);
					}
				}
			}
		}
	}

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOT_RESOLVE_NAVIGATION_PATH, &fName);

	return err;
}


VError EntityAttribute::ResolveQueryStatements(EntityModelCatalog* catalog, ModelErrorReporter* errorReporter, BaseTaskInfo* context)
{
	VError err = VE_OK;
	VString ss;
	if (fKind == eattr_computedField)
	{
		if (fScriptKind == script_db4d)
		{
			if (fScriptDB4D == nil)
			{
				fScriptDB4D = new SearchTab(fOwner);
				fScriptDB4D->AllowJSCode(true);
				if (errorReporter != nil)
				{
					ModelErrorContextInstaller errs(errorReporter);
					fScriptDB4D->BuildFromString(fScriptStatement[script_attr_get], ss, context, fOwner, true);
				}
				else
					err = fScriptDB4D->BuildFromString(fScriptStatement[script_attr_get], ss, context, fOwner, true);
			}

			if (fScriptQuery == nil && err == VE_OK)
			{
				fScriptQuery = new SearchTab(fOwner);
				fScriptQuery->AllowJSCode(true);
				if (errorReporter != nil)
				{
					ModelErrorContextInstaller errs(errorReporter);
					if (fScriptStatement[script_attr_query].IsEmpty())
						fScriptQuery->BuildFromString(fScriptStatement[script_attr_get], ss, context, fOwner, false);
					else
						fScriptQuery->BuildFromString(fScriptStatement[script_attr_query], ss, context, fOwner, false);
				}
				else
				{
					if (fScriptStatement[script_attr_query].IsEmpty())
						err = fScriptQuery->BuildFromString(fScriptStatement[script_attr_get], ss, context, fOwner, false);
					else
						err = fScriptQuery->BuildFromString(fScriptStatement[script_attr_query], ss, context, fOwner, false);
				}
			}
		}
	}
	return err;
}

#if BuildEmFromTable

void EntityAttribute::SetRelation(Relation* rel, EntityRelationKind kind, LocalEntityModelCatalog* catalog)
{
	Field* source;
	Field* dest;
	Table* desttable;
	if (kind == erel_Nto1)
	{
		desttable = rel->GetDestTable();
		source = rel->GetSource();
		dest = rel->GetDest();
	}
	else
	{
		desttable = rel->GetSourceTable();
		dest = rel->GetSource();
		source = rel->GetDest();
	}
	fSubEntity = EntityModel::BuildLocalEntityModel(desttable, catalog);
	if (fSubEntity != nil)
	{
		VString sourcename,destname;
		source->GetName(sourcename);
		dest->GetName(destname);
		fFieldPos = source->GetPosInRec();
		EntityAttribute* sourceatt = fOwner->getAttribute(sourcename) /*fSubEntity->getAttribute(sourcename)*/;
		EntityAttribute* destatt = fSubEntity->getAttribute(destname);
		if (sourceatt != nil && destatt != nil)
		{
			fRelation = new EntityRelation(sourceatt, destatt, kind);
			fRelation->RecalcPath();
			fRelationPathID = fOwner->FindRelationPath(fRelation);
			if (fRelationPathID == -1)
				fRelationPathID = fOwner->AddRelationPath(fRelation);
		}
		fSubEntityName = fSubEntity->GetName();
		if (kind == erel_1toN)
		{
			fReversePath = true;
			rel->GetName(fRelPath);
		}
		else
		{
			fRelPath = fSubEntityName;
			if (!dest->IsPrimKey())
			{
				VString s;
				dest->GetName(s);
				fRelPath += ":";
				fRelPath += s;
				fRelIsNotOnPrimKey = true;
				sourceatt->fRelIsNotOnPrimKey = true;
			}
		}

	}
}
#endif


VError EntityAttribute::CallDBEvent(DBEventKind kind, EntityRecord* inRec, BaseTaskInfo* context, bool includesInherited, const EntityAttribute* realAtt) const
{
	VError err = VE_OK;
	if (kind != dbev_none)
	{
		if (realAtt == nil)
			realAtt = this;
		if (includesInherited && fFrom != nil)
			err = fFrom->CallDBEvent(kind, inRec, context, true, realAtt);
		err = fDBEvents.CallEvent(kind, inRec, context, realAtt, nil, nil);
		/*
		const DBEvent& ev = fEvents[kind];
		if (ev.IsValid())
		{
			err = ev.Call(inRec, context, this, nil);
		}
		*/
	}
	return err;
}


bool EntityAttribute::HasEvent(DBEventKind kind) const
{
	bool result = false;
	if (kind != dbev_none)
	{
		if (fFrom != nil)
			result = fFrom->HasEvent(kind);
		if (!result)
			result = fDBEvents.HasEvent(kind);
		/*
		const DBEvent& ev = fEvents[kind];
		if (ev.IsValid())
			result = true;
			*/
	}
	return result;
}


bool EntityAttribute::IsScalar() const
{
	bool res = false;
	switch (fKind)
	{
		case eattr_storage:
		//case eattr_field:
		case eattr_alias:
			res = true;
			break;
		case eattr_computedField:
			res = true; // a gerer pour les champs calcule qui retourne un EM ou une selection
			break;
		default:
			break;
	}
	return res;
}


bool EntityAttribute::IsSortable() const
{
	bool res = false;
	switch (fKind)
	{
		case eattr_storage:
		case eattr_alias:
			{
				sLONG scalartype = ComputeScalarType();
				res = (scalartype != 0 && scalartype != VK_IMAGE && scalartype != VK_BLOB && scalartype != VK_BLOB_DB4D); // temporaire
				/*
				Field* cri = RetainField();
				if (cri != nil)
				{
					res = cri->IsSortable();
				}
				*/
			}
			break;
		case eattr_computedField:
			{
				sLONG scalartype = ComputeScalarType();
				res = (scalartype != 0 && scalartype != VK_IMAGE && scalartype != VK_BLOB && scalartype != VK_BLOB_DB4D); // temporaire
			}
			break;
		default:
			break;
	}
	return res;
}


bool EntityAttribute::IsStatable() const
{
	bool res = false;
	sLONG scalartype = ComputeScalarType();
	if (scalartype != 0 && scalartype != VK_BLOB && scalartype != VK_IMAGE && scalartype != VK_BLOB_DB4D)
		res = true;
	return res;
}

bool EntityAttribute::IsSummable() const
{
	bool res = false;
	sLONG scalartype = ComputeScalarType();
	if (scalartype == VK_BYTE || scalartype == VK_WORD || scalartype == VK_LONG || scalartype == VK_LONG8 || scalartype == VK_REAL || scalartype == VK_FLOAT || scalartype == VK_DURATION)
		res = true;
	return res;
}



bool EntityAttribute::NeedValidationOnRemove() const
{
	bool needvalidate = false;

	if (HasEvent(dbev_validateremove))
		needvalidate = true;

	return needvalidate;
}

bool EntityAttribute::NeedValidation() const
{
	bool needvalidate = false;
	if (fType != nil)
	{
		needvalidate = needvalidate || fType->NeedValidation();
	}

	if (fLocalType != nil)
	{
		needvalidate = needvalidate || fLocalType->NeedValidation();
	}

	if (HasEvent(dbev_validate))
		needvalidate = true;

	return needvalidate;
}


CDB4DEntityModel* EntityAttribute::GetRelatedEntityModel() const
{
	return GetSubEntityModel();
}




// ----------------------------------------------------------------------------------------



AttributePath::AttributePath(EntityModel* model, const VString& inPath)
{
	fIsValid = FromPath(model, inPath);
}


AttributePath::AttributePath(EntityModel* model, const AttributeStringPath& inPath, bool fromstart, VString* outRemain)
{
	fIsValid = FromPath(model, inPath, false, fromstart, outRemain);
}


bool AttributePath::FromPath(EntityModel* model, const VString& inPath, bool expandAliases, VString* outRemain)
{
	AttributeStringPath path(inPath);
	return FromPath(model, path, expandAliases, true, outRemain);
}


bool AttributePath::FromPath(EntityModel* model, const AttributeStringPath& inPath, bool expandAliases, bool fromstart, VString* outRemain)
{
	bool result = true;
	EntityModel* curmodel = model;
	fParts.clear();
	fParts.reserve(inPath.Count());
	const VString* s = fromstart ? inPath.FirstPart() : inPath.CurPart();

	/* cela permettait d'avoir une syntaxe model.attribute, mais ou etait ce utilise
	if (s != nil && curmodel != nil && *s == curmodel->GetName())
		s = inPath.NextPart();
		*/

	while (s != nil && curmodel != nil)
	{
		EntityAttribute* att = nil;
		sLONG instance = 0;
		sLONG p = s->FindUniChar('{');
		if (p <= 0)
			att = curmodel->getAttribute(*s);
		else
		{
			VString s1, s2;
			s->GetSubString(1, p-1, s1);
			s->GetSubString(p+1, s->GetLength()-1-p, s2);
			att = curmodel->getAttribute(s1);
			instance = s2.GetLong();
		}
		if (att != nil)
		{
			if (att->GetKind() == eattr_alias)
			{
				AttributePath subpath;
				if (subpath.FromPath(curmodel, att->GetRelPathAsString(), true))
				{
					for (EntityAttributeInstanceCollection::iterator cur = subpath.fParts.begin(), end = subpath.fParts.end(); cur != end; ++cur)
					{
						//EntityAttributeInstance* subattinst = &(*cur);
						fParts.push_back(*cur);
					}
				}
				else
					result = false;

				curmodel = nil;
			}
			else
			{
				EntityAttributeInstance attinst(att, instance);

				fParts.push_back(attinst);
				curmodel = att->GetSubEntityModel();
			}
			s = inPath.NextPart();
		}
		else
		{
			result = false;
			curmodel = nil;
		}
	}

	if (s != nil)
	{
		if (outRemain != nil)
		{
			*outRemain = *s;
			while (s != nil)
			{
				s = inPath.NextPart();
				if (s != nil)
					(*outRemain) += "." + *s;
			}

		}
		else
			result = false;
	}
	else
	{
		if (outRemain != nil)
			outRemain->Clear();
	}

	if (fParts.empty())
		result = false;

	return result;
}


void AttributePath::GetString(VString& outString) const
{
	outString.Clear();
	bool first = true;
	for (EntityAttributeInstanceCollection::const_iterator cur = fParts.begin(), end = fParts.end(); cur != end; cur++)
	{
		if (first)
			first = false;
		else
			outString += L".";
		const EntityAttributeInstance* attinst = &(*cur);
		outString += attinst->fAtt->GetName();
		sLONG instance = attinst->fInstance;
		if (instance != 0)
		{
			outString += "{"+ToString(instance)+"}";
		}
	}
}


bool AttributePath::operator < (const AttributePath& other) const
{
	EntityAttributeInstanceCollection::const_iterator cur1 = fParts.begin(), end1 = fParts.end(), 
													  cur2 = other.fParts.begin(), end2 = other.fParts.end();
	for (; cur1 != end1 && cur2 != end2; ++cur1, ++cur2)
	{
		if ( cur1->IsPartOfARelation() && cur2->IsPartOfARelation() )
		{
			if (*cur1 == *cur2)
			{
				// on continue
			}
			else
				return *cur1 < *cur2;
		}
		else
			return false;
	}
	if (cur2 != end2)
		return cur2->IsPartOfARelation();
	else
		return false;
}




// ----------------------------------------------------------------------------------------


static bool BuildVectorOfSortedAttribute(EntityModel* inModel, const VString& inListOfAtttibutes, VectorOfVString& attribs2, BaseTaskInfo* context)
{
	bool ok = true;
	VectorOfVString attribs;
	inListOfAtttibutes.GetSubStrings( ',', attribs, false, true);

	attribs2.reserve(attribs.size());
	for (VectorOfVString::iterator cur = attribs.begin(), end = attribs.end(); cur != end; cur++)
	{
		bool ascent = true;
		VString ss = *cur;
		sLONG p = ss.Find(L" desc");
		if (p > 0)
		{
			ascent = false;
			ss.SubString(1, p-1);
		}
		else
		{
			p = ss.Find(L" asc");
			if (p > 0)
			{
				ascent = true;
				ss.SubString(1, p-1);
			}
		}
		EntityAttribute* att = inModel->getAttribute(ss);
		bool dejapris = false;
		if (att != nil)
		{
			if (att->GetAttributeKind() == eattr_computedField)
			{
				if (att->GetScriptKind() == script_javascript && context != nil)
				{
					VJSContext jscontext(context->GetJSContext());

					VJSObject* objfunc = nil;
					VJSObject localObjFunc(jscontext);
					VectorOfVString params;
					if (att->GetScriptObjFunc(script_attr_sort, context->GetJSContext(), context, localObjFunc))
						objfunc = &localObjFunc;
					if (objfunc == nil)
					{
						params.push_back("ascent");
						objfunc = context->getContextOwner()->GetJSFunction(att->GetScriptNum(script_attr_sort), att->GetScriptStatement(script_attr_sort), &params);
					}
					if (objfunc != nil)
					{
						EntityModel* sousEm = att->GetOwner();
						VJSValue result(jscontext);
						VJSException excep;
						vector<VJSValue> params;
						VJSValue ascentVal(jscontext);
						ascentVal.SetBool(ascent);
						params.push_back(ascentVal);
						VJSObject emjs(VJSEntityModel::CreateInstance(jscontext,sousEm));
						emjs.CallFunction(*objfunc, &params, &result, excep);
						if (!excep.IsEmpty())
							ok = false;
						VString sresult;
						result.GetString(sresult);
						if (!sresult.IsEmpty())
						{
							dejapris = true;
							VectorOfVString attribs3;
							BuildVectorOfSortedAttribute(inModel, sresult, attribs3, context);
							for (VectorOfVString::iterator curx = attribs3.begin(), endx = attribs3.end(); curx != endx; curx++)
							{
								attribs2.push_back(*curx);
							}
						}
					}

				}
			}
		}
		else
		{
			//inModel->ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_NOT_FOUND, &ss);
			//ok = false;
		}

		if (!dejapris)
			attribs2.push_back(*cur);
	}
	return ok;
}


bool EntityAttributeSortedSelection::BuildFromString(const VString& inListOfAtttibutes, BaseTaskInfo* context, bool FirstLevelOnly, bool forSorting, RestTools* req)
{
	bool result = true;
	VectorOfVString attribs;

	inListOfAtttibutes.GetSubStrings( ',', attribs, false, true);
	if (forSorting)
	{
		VectorOfVString attribs2;
		result = BuildVectorOfSortedAttribute(fModel, inListOfAtttibutes, attribs2, context);

		attribs.swap(attribs2);
	}

	reserve(attribs.size());
	for (VectorOfVString::iterator cur = attribs.begin(), end = attribs.end(); cur != end && result; cur++)
	{
		bool ascent = true;
		VString ss = *cur;

		if (forSorting)
		{
			sLONG p = ss.Find(L" desc");
			if (p > 0)
			{
				ascent = false;
				ss.SubString(1, p-1);
			}
			else
			{
				p = ss.Find(L" asc");
				if (p > 0)
				{
					ascent = true;
					ss.SubString(1, p-1);
				}
			}
		}

		sLONG p = ss.FindUniChar(':');
		if (p > 0)
			ss.Truncate(p-1);

		if (FirstLevelOnly)
		{
			EntityAttribute* att = fModel->getAttribute(ss);
			if (att != nil)
			{
				if (req == nil || att->getScope() == escope_public)
				{
					if (forSorting && !att->IsSortable())
					{
						att->ThrowError(VE_DB4D_WRONG_ATTRIBUTE_KIND);
						result = false;
					}
					else
						push_back(EntityAttributeSortedItem(att, ascent));
				}
				else
				{
					fModel->ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_NOT_FOUND, &ss);
					result = false;
				}
			}
			else
			{
				fModel->ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_NOT_FOUND, &ss);
				result = false;			
			}
		}
		else
		{
			if (!AddAttribute(ss, ascent, forSorting, req))
			{
				fModel->ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_NOT_FOUND, &ss);
				result = false;
			}
		}
	}

	return result;
}


bool EntityAttributeSortedSelection::AddAttribute(const VString& inAttributePath, bool ascent, bool forSorting, RestTools* req)
{
	AttributeStringPath attpath(inAttributePath);
	return AddAttribute(attpath, ascent, forSorting, req);
}


EntityAttributeSortedSelection::const_iterator EntityAttributeSortedSelection::FindAttribute(const EntityAttribute* att) const
{
	for (const_iterator cur = begin(), last = end(); cur != last; cur++)
	{
		if (cur->fAttribute == att)
			return cur;
	}
	return end();
}


EntityAttributeSortedSelection::iterator EntityAttributeSortedSelection::FindAttribute(const EntityAttribute* att)
{
	for (iterator cur = begin(), last = end(); cur != last; cur++)
	{
		if (cur->fAttribute == att)
			return cur;
	}
	return end();
}


EntityAttributeSortedSelection* EntityAttributeSortedSelection::FindSubSelection(const EntityAttribute* att) const
{
	const_iterator found = FindAttribute(att);
	if (found == end())
		return nil;
	else
		return found->fSousSelection;
}


void EntityAttributeSortedSelection::ToString(VString& outString)
{
	bool first = true;
	for (iterator cur = begin(), last = end(); cur != last; cur++)
	{
		const EntityAttribute* att = cur->fAttribute;
		VString name = att->GetName();
		if (first)
			first = false;
		else
			outString += ",";
		outString += name;
	}
}

void EntityAttributeSortedSelection::GetString(VString& outString, bool FirstLevelOnly, bool forSorting, const VString* prefix)
{
	bool first = true;
	for (iterator cur = begin(), last = end(); cur != last; cur++)
	{
		VString name;

		EntityAttributeSortedSelection* soussel = cur->fSousSelection;
		if (soussel != nil)
		{
			VString sousprefix;
			const EntityAttribute* att = cur->fAttribute;
			sousprefix = att->GetName()+".";
			if (prefix != nil)
				sousprefix = *prefix+sousprefix;
			if (!FirstLevelOnly)
			{
				soussel->GetString(name, false, forSorting, &sousprefix);
			}
		}
		else
		{
			AttributePath* attpath = cur->fAttPath;
			if (attpath != nil)
			{
				attpath->GetString(name);
			}
			else
			{
				const EntityAttribute* att = cur->fAttribute;
				name = att->GetName();
			}
			if (prefix != nil)
				name = *prefix + name;
			if (forSorting)
			{
				if (cur->fAscent)
					name += " asc";
				else
					name += " desc";
			}
		}
		if (!name.IsEmpty())
		{
			if (first)
				first = false;
			else
				outString += ",";
			outString += name;
		}
	}

}



bool EntityAttributeSortedSelection::AddAttribute(const EntityAttribute* att, RestTools* req)
{
	if (req == nil || att->getScope() == escope_public)
	{
		push_back(EntityAttributeSortedItem(att, true));
		return true;
	}
	else
		return true; // pour l'instant
}


bool EntityAttributeSortedSelection::AddAttribute(const AttributeStringPath& inAttributePath, bool ascent, bool forSorting, RestTools* req)
{
	bool result = true;
	const VString* s = inAttributePath.CurPart();
	if (s != nil)
	{
		EntityAttribute* att = fModel->getAttribute(*s);
		if (att != nil)
		{
			if (req == nil || att->getScope() == escope_public)
			{
				if (forSorting && att->GetKind() == eattr_relation_Nto1)
				{
					AttributePath* attpath = new AttributePath(fModel, inAttributePath, false);
					if (attpath->IsValid())
					{
						const EntityAttribute* lastpartatt = attpath->LastPart()->fAtt;
						if (forSorting)
						{
							if (lastpartatt->IsSortable())
							{
								attpath->Retain();
								push_back(EntityAttributeSortedItem(attpath, ascent));
							}
							else
							{
								VString ss;
								attpath->GetString(ss);
								fModel->ThrowError(VE_DB4D_WRONG_ATTRIBUTE_KIND, &ss);
							}
						}
					}
					else
					{
						VString ss;
						attpath->GetString(ss);
						fModel->ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_NOT_FOUND, &ss);
					}
						QuickReleaseRefCountable(attpath);
				}
				else
				{
					EntityAttributeSortedItem* eai = nil;
					for (iterator cur = begin(), last = end(); cur != last && eai == nil; cur++)
					{
						if (cur->fAttribute == att)
						{
							if (forSorting)
							{
								if (att->GetKind() == eattr_relation_1toN || att->GetKind() == eattr_composition)
									eai = &(*cur);
							}
							else
								eai = &(*cur);
						}
					}
					if (eai == nil)
					{
						push_back(EntityAttributeSortedItem(att, ascent));
						eai = &((*this)[size()-1]);
					}
					s = inAttributePath.NextPart();
					EntityModel* sousmodel = att->GetSubEntityModel();
					if (sousmodel != nil)
					{
						if (s != nil)
						{
							if (eai->fSousSelection == nil)
							{
								eai->fSousSelection = new EntityAttributeSortedSelection(sousmodel);
							}
							result = eai->fSousSelection->AddAttribute(inAttributePath, ascent, forSorting, req);
						}
						else
						{
							if (forSorting)
								att->ThrowError(VE_DB4D_WRONG_ATTRIBUTE_KIND);
						}
					}
				}
			}
			else
			{
				fModel->ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_NOT_FOUND, s);
				result = false;
			}
		}
		else
		{
			fModel->ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_NOT_FOUND, s);
			result = false;
		}
	}
	else
		result = false;
	return result;
}


void EntityAttributeSortedSelection::Dispose()
{
	for (iterator cur = begin(), last = end(); cur != last; cur++)
	{
		EntityAttributeSortedItem* attitem = &(*cur);
		if (attitem->fSousSelection != nil)
		{
			delete attitem->fSousSelection;
		}
		if (attitem->fAttPath != nil)
		{
			QuickReleaseRefCountable(attitem->fAttPath);
		}
	}
}



// ------------------------------------------------


EntityAttributeSelection::EntityAttributeSelection(EntityModel* inModel)
{
	fModel = inModel;
	if (inModel != nil)
		resize(inModel->CountAttributes());
}

EntityAttributeSelection::EntityAttributeSelection(EntityModel* inModel, const VString& inListOfAtttibutes, bool FirstLevelOnly, RestTools* req)
{
	fModel = inModel;
	if (inModel != nil)
		resize(inModel->CountAttributes());
	BuildFromString(inListOfAtttibutes, FirstLevelOnly, req);
}


bool EntityAttributeSelection::BuildFromString(const VString& inListOfAtttibutes, bool FirstLevelOnly, RestTools* req)
{
	bool result = true;
	VectorOfVString attribs;

	inListOfAtttibutes.GetSubStrings( ',', attribs, false, true);
	for (VectorOfVString::iterator cur = attribs.begin(), end = attribs.end(); cur != end; cur++)
	{
		if (FirstLevelOnly)
		{
			EntityAttribute* att = fModel->getAttribute(*cur);
			if (att != nil)
			{
				if (att->getScope() == escope_public || req == nil)
				{
					EntityAttributeItem* eai = &((*this)[att->GetPosInOwner()-1]);
					eai->fAttribute = att;
				}
			}
		}
		else
		{
			if (!AddAttribute(*cur, req))
			{
				VString s = *cur;
				sLONG p = s.FindUniChar(':');
				if (p > 0)
					s.Truncate(p-1);
				fModel->ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_NOT_FOUND, &s);
				result = false;
			}
		}
	}

	return result;
}


bool EntityAttributeSelection::AddAttribute(const VString& inAttributePath, RestTools* req)
{
	AttributeStringPath attpath(inAttributePath);
	return AddAttribute(attpath, req);
}


bool EntityAttributeSelection::AddAttribute(const AttributeStringPath& inAttributePath, RestTools* req)
{
	bool result = true;
	const VString* s = inAttributePath.CurPart();
	if (s != nil)
	{
		VString attname = *s;
		sLONG count = -1;
		sLONG skip = -1;
		sLONG p = attname.FindUniChar(':');
		if (p > 0)
		{
			VString s2;
			attname.GetSubString(p+1, attname.GetLength() - p, s2);
			attname.Truncate(p-1);
			sLONG p2 = s2.FindUniChar('-');
			if (p2 > 0)
			{
				VString s3;
				s2.GetSubString(p2+1, s2.GetLength() - p2, s3);
				s2.Truncate(p2-1);
				skip = s3.GetLong();
				count = s2.GetLong();
			}
			else
				count = s2.GetLong();
		}
		EntityAttribute* att = fModel->getAttribute(attname);
		if (att != nil)
		{
			if (att->getScope() == escope_public || req == nil)
			{
				EntityAttributeItem* eai = &((*this)[att->GetPosInOwner()-1]);
				eai->fAttribute = att;
				if (eai->fCount == -1)
					eai->fCount = count;
				if (eai->fSkip == -1)
					eai->fSkip = skip;
				s = inAttributePath.NextPart();
				EntityModel* sousmodel = att->GetSubEntityModel();
				if (s != nil && sousmodel != nil)
				{
					if (eai->fSousSelection == nil)
					{
						eai->fSousSelection = new EntityAttributeSelection(sousmodel);
					}
					result = eai->fSousSelection->AddAttribute(inAttributePath, nil);
				}
			}
			else
				result = false;
		}
		else
			result = false;
	}
	else
		result = false;
	return result;
}


void EntityAttributeSelection::Dispose()
{
	for (iterator cur = begin(), last = end(); cur != last; cur++)
	{
		EntityAttributeItem* attitem = &(*cur);
		if (attitem->fSousSelection != nil)
		{
			delete attitem->fSousSelection;
		}
	}
}



void EntityAttributeSelection::GetString(VString& outString, bool FirstLevelOnly, const VString* prefix)
{
	bool first = true;
	for (iterator cur = begin(), last = end(); cur != last; cur++)
	{
		VString name;

		EntityAttributeSelection* soussel = cur->fSousSelection;
		if (soussel != nil)
		{
			VString sousprefix;
			const EntityAttribute* att = cur->fAttribute;
			sousprefix = att->GetName()+".";
			if (prefix != nil)
				sousprefix = *prefix+sousprefix;
			if (!FirstLevelOnly)
			{
				soussel->GetString(name, false, &sousprefix);
			}
		}
		else
		{
			const EntityAttribute* att = cur->fAttribute;
			if (att != nil)
			{
				name = att->GetName();
				sLONG skip = cur->fSkip;
				sLONG count = cur->fCount;
				if (skip != -1)
				{
					name +=":"+ToString(skip)+"-"+ToString(count);
				}
				else if (count != -1)
				{
					name +=":"+ToString(count);
				}
				if (prefix != nil)
					name = *prefix + name;
			}
		}
		if (!name.IsEmpty())
		{
			if (first)
				first = false;
			else
				outString += ",";
			outString += name;
		}
	}

}




// ----------------------------------------------------------------------------------------


DBEvent::DBEvent()
{
	fKind = dbev_none;
	fOwner = nil;
	fOverWrite = true;
	fUserDefined = false;
	fOldWay = false;
}


VError DBEvent::CopyFrom(EntityModel* inOwner, const DBEvent* from)
{
	fOverWrite = false;
	fOwner = inOwner;
	fFrom = from->fFrom;
	fKind = from->fKind;
	fUserDefined = from->fUserDefined;
	fOldWay = from->fOldWay;
	return VE_OK;
}


VError DBEvent::ToBag(VValueBag& outBag, bool forDax, bool forSave, bool forJSON, bool firstlevel) const
{
	VError err = VE_OK;

	if (fKind != dbev_none)
	{
		if (fOldWay)
			outBag.SetString(d4::kind, OldEDBEventKinds[fKind]);
		else
			outBag.SetString(d4::kind, EDBEventKinds[fKind]);
	}
	if (!fFrom.IsEmpty())
		outBag.SetString(d4::from, fFrom);
	if (fUserDefined)
		outBag.SetBool(d4::userDefined, fUserDefined);

	return err;
}



VError DBEvent::FromBag(EntityModel* inOwner, const VValueBag* bag, EntityModelCatalog* catalog, ModelErrorReporter* errorReporter)
{
	VError err = VE_OK;
	VString ss;
	if (bag->GetString(d4::kind, ss) && !ss.IsEmpty())
	{
		fKind = (DBEventKind)OldEDBEventKinds[ss];
		if (fKind == dbev_none)
		{
			fKind = (DBEventKind)EDBEventKinds[ss];
			fOldWay = false;
		}
		else
			fOldWay = true;
		/*
		if (fKind == dbev_none)
			fKind = (DBEventKind)oldEDBEventKinds[ss];
			*/
	}
	if (fKind == dbev_none)
	{
		if (errorReporter == nil)
			err = inOwner->ThrowError(VE_DB4D_MISSING_OR_INVALID_EVENTKIND);
		else
		{
			ModelErrorContextInstaller errs(errorReporter);
			inOwner->ThrowError(VE_DB4D_MISSING_OR_INVALID_EVENTKIND);
		}
	}

	bag->GetString(d4::from, fFrom);
	if (fFrom.IsEmpty())
	{
		if (errorReporter == nil)
			err = inOwner->ThrowError(VE_DB4D_MISSING_OR_INVALID_EVENT_METHOD);
		else
		{
			ModelErrorContextInstaller errs(errorReporter);
			inOwner->ThrowError(VE_DB4D_MISSING_OR_INVALID_EVENT_METHOD);
		}
	}

	if (err == VE_OK)
		fOwner = inOwner;

	fUserDefined = false;

	return err;
}


VError DBEvent::Call(EntityRecord* inRec, BaseTaskInfo* context, const EntityAttribute* inAtt, const EntityModel* inDataClass, EntityCollection* *outSel, sLONG eventpos) const
{
	VError err = VE_OK;
	EntityCollection* selresult = nil;
	if (IsValid())
	{
		bool go = false;
		if (inDataClass != nil)
		{
			go = true;
		}
		else
		{
			if (inRec != nil && inRec->OKToCallEvent(fKind))
				go = true;
		}
		if (go)
		{
			if (!fFrom.IsEmpty())
			{
				VJSContext jscontext(context->GetJSContext());
				JSEntityMethodReference methref(fOwner, fKind, inAtt, eventpos);
				VJSObject ObjFunc(jscontext);
				bool okfunc = false;

				if (!context->GetJSMethod(methref, ObjFunc))
				{
					VJSValue result(jscontext);
					jscontext.EvaluateScript(fFrom, nil, &result, nil, nil);
					if (!result.IsUndefined() && result.IsObject())
					{
						ObjFunc = result.GetObject();
						if (ObjFunc.IsFunction())
						{
							okfunc = true;
							context->SetJSMethod(methref, ObjFunc);
						}
					}
				}
				else
					okfunc = true;

				if (okfunc)
				{
					VJSObject therecord(jscontext);
					if (inDataClass != nil)
					{
						EntityModel* model = (EntityModel*)inDataClass;
						therecord = VJSEntityModel::CreateInstance(jscontext, model);
					}
					else
						therecord = VJSEntitySelectionIterator::CreateInstance(jscontext, new EntitySelectionIterator(inRec, context));
					XBOX::VJSException excep;
					VJSValue outResult(jscontext);
					vector<VJSValue> Params;
					if (fOldWay)
					{
						if (inAtt != nil)
						{
							VJSValue vAttName(jscontext);
							vAttName.SetString(inAtt->GetName());
							Params.push_back(vAttName);
						}
					}
					else
					{
						VJSObject evobj(jscontext);
						evobj.MakeEmpty();
						
						evobj.SetProperty("eventKind", EDBEventKinds[fKind]);

						if (inAtt != nil)
						{
							evobj.SetProperty("attributeName", inAtt->GetName());
						}

						if (inDataClass != nil)
						{
							evobj.SetProperty("dataClassName", inDataClass->GetName());
						}
						else if (inRec != nil)
						{
							evobj.SetProperty("dataClassName", inRec->GetModel()->GetName());
						}
						Params.push_back(evobj);
					}
					therecord.CallFunction(ObjFunc, &Params, &outResult, excep);

					if (!excep.IsEmpty())
					{
						if (fKind != dbev_init)
						{
							VJSValue valex(jscontext, excep);
							VJSObject obj(valex.GetObject());
							VJSValue val(obj.GetProperty("error"));
							if (!val.IsNull() && !val.IsUndefined())
							{
								sLONG errnum = 0;
								val.GetLong(&errnum);
								if (errnum != 0)
								{
									VString errorMessage;
									VJSValue valMess(obj.GetProperty("errorMessage"));
									if (!valMess.IsNull() && !valMess.IsUndefined())
									{
										valMess.GetString(errorMessage);
									}
									err = vThrowUserGeneratedError(MAKE_VERROR('dbev', errnum), errorMessage);
								}
							}
							else
							{
								ThrowJSExceptionAsError(jscontext, excep);
								err = fOwner->ThrowError(VE_DB4D_JS_ERR);
							}
						}
					}
					else
					{
						EntityCollection* selres = outResult.GetObjectPrivateData<VJSEntitySelection>();
						if (selres != nil)
						{
							selresult = selres;
							if (selresult != nil)
								selresult->Retain();
						}
						else if (!outResult.IsNull() && outResult.IsObject() && (fKind != dbev_init))
						{
							VJSObject objres = outResult.GetObject();
							VJSValue val(objres.GetProperty("error"));
							if (!val.IsNull() && !val.IsUndefined())
							{
								sLONG errnum = 0;
								val.GetLong(&errnum);
								if (errnum != 0)
								{
									//err = fOwner->ThrowError(MAKE_VERROR('dbev', errnum));
									VString errorMessage;
									VJSValue valMess(objres.GetProperty("errorMessage"));
									if (!valMess.IsNull() && !valMess.IsUndefined())
									{
										valMess.GetString(errorMessage);
									}
									err = vThrowUserGeneratedError(MAKE_VERROR('dbev', errnum), errorMessage);
								}
								else if (fKind == dbev_restrict)
								{
									err = ThrowBaseError(VE_DB4D_INVALID_COLLECTION_IN_RESTRICTING_EVENT, fOwner->GetName());
								}
							}
							else if (fKind == dbev_restrict)
							{
								err = ThrowBaseError(VE_DB4D_INVALID_COLLECTION_IN_RESTRICTING_EVENT, fOwner->GetName());
							}
						}
						else if (fKind == dbev_restrict)
						{
							err = ThrowBaseError(VE_DB4D_INVALID_COLLECTION_IN_RESTRICTING_EVENT, fOwner->GetName());
						}
					}

				}
				else
				{
					err = ThrowBaseError(VE_DB4D_ENTITY_METHOD_NAME_UNKNOWN, fFrom);
				}
				
			}
			if (inDataClass != nil)
			{
			}
			else
				inRec->ReleaseCallEvent(fKind);
		}
	}

	if (outSel != nil)
	{
		QuickReleaseRefCountable(*outSel);
		*outSel = selresult;
	}
	return err;
}



void DBEventMap::AddEvent(const DBEvent& event)
{
	insert(make_pair(event.GetKind(), event));

}


VError DBEventMap::CallEvent(DBEventKind eventkind, EntityRecord* inRec, BaseTaskInfo* context, const EntityAttribute* inAtt, const EntityModel* inDataClass, EntityCollection* *outSel) const
{
	VError err = VE_OK;
	pair<DBEventMap::const_iterator, DBEventMap::const_iterator> result = equal_range(eventkind);
	sLONG pos = 0;
	for (DBEventMap::const_iterator cur = result.first, end = result.second; cur != end && err == VE_OK; ++cur, ++pos)
	{
		err = cur->second.Call(inRec, context, inAtt, inDataClass, outSel, pos);
	}
	return err;
}


VError DBEventMap::CopyFrom(EntityModel* inOwner, const DBEventMap& from)
{
	VError err = VE_OK;
	clear();
	for (DBEventMap::const_iterator cur = from.begin(), end = from.end(); cur != end && err == VE_OK; ++cur)
	{
		DBEvent ev;
		err = ev.CopyFrom(inOwner, &(cur->second));
		if (err == VE_OK)
			insert(make_pair(ev.GetKind(), ev));
	}
	return err;
}


VError DBEventMap::ToBag(VValueBag& outBag, bool forDax, bool forSave, bool forJSON, bool firstlevel) const
{
	VError err = VE_OK;
	for (DBEventMap::const_iterator cur = begin(), last = end(); cur != last && err == VE_OK; cur++)
	{
		const DBEvent* ev = &(cur->second);
		if (ev->IsValid() && (ev->IsOverWritten() || !forSave))
		{
			BagElement evBag(outBag, d4::events);
			err = ev->ToBag(*evBag, forDax, forSave, forJSON, firstlevel);
		}
	}
	return err;
}





// ----------------------------------------------------------------------------------------



EntityModel::EntityModel(EntityModelCatalog* inOwner)
{
	fCatalog = inOwner;
	fQueryLimit = 0;
	fBaseEm = nil;
	fDefaultTopSize = 0;
	fMatchPrimKey = 2;
	fRestrictingQuery = nil;
	fAlreadyResolvedComputedAtts = false;
	fExtraProperties = nil;
	fScope = escope_public;
	fHasDeleteEvent = false;
	fWithRestriction = false;
	fOneAttributeHasDeleteEvent = false;
	fPublishAsGlobal = false;
	fPublishAsGlobalDefined = false;
	fAllowOverrideStamp = false;
	fNoEdit = false;
	fNoSave = false;
	fill(&fForced[DB4D_EM_None_Perm], &fForced[DB4D_EM_Promote_Perm+1], 0);
}


EntityModel::~EntityModel()
{
	QuickReleaseRefCountable(fBaseEm);
	for (EntityAttributeCollection::iterator cur = fAttributes.begin(), end = fAttributes.end(); cur != end; cur++)
	{
		(*cur)->Release();
	}
	for (EntityMethodMap::iterator cur = fMethodsByName.begin(), end = fMethodsByName.end(); cur != end; cur++)
	{
		cur->second->Release();
	}
	QuickReleaseRefCountable(fExtraProperties);
}


VError EntityModel::ThrowError( VError inErrCode, const VString* p1) const
{
	VErrorDB4D_OnEM *err = new VErrorDB4D_OnEM(inErrCode, noaction, fCatalog, this);
	if (p1 != nil)
		err->GetBag()->SetString(Db4DError::Param1, *p1);
	VTask::GetCurrent()->PushRetainedError( err);

	return inErrCode;
}


CDB4DEntityRecord* EntityModel::NewEntity(CDB4DBaseContextPtr inContext) const
{
	return newEntity(ConvertContext(inContext));
}

RecIDType EntityModel::CountEntities(CDB4DBaseContext* inContext)
{
	return countEntities(ConvertContext(inContext));
}


CDB4DEntityRecord* EntityModel::FindEntityWithPrimKey(const XBOX::VString& primkey, CDB4DBaseContext* inContext, VErrorDB4D& err, DB4D_Way_of_Locking HowToLock)
{
	return findEntityWithPrimKey(primkey, ConvertContext(inContext), err, HowToLock);
}

CDB4DEntityRecord* EntityModel::FindEntityWithPrimKey(const XBOX::VectorOfVString& primkey, CDB4DBaseContext* inContext, VErrorDB4D& err, DB4D_Way_of_Locking HowToLock)
{
	return findEntityWithPrimKey(primkey, ConvertContext(inContext), err, HowToLock);
}

CDB4DEntityRecord* EntityModel::FindEntityWithPrimKey(const XBOX::VValueBag& primkey, CDB4DBaseContext* inContext, VErrorDB4D& err, DB4D_Way_of_Locking HowToLock)
{
	return findEntityWithPrimKey(primkey, ConvertContext(inContext), err, HowToLock);
}

CDB4DEntityRecord* EntityModel::FindEntityWithPrimKey(const XBOX::VectorOfVValue& primkey, CDB4DBaseContext* inContext, VErrorDB4D& err, DB4D_Way_of_Locking HowToLock)
{
	return findEntityWithPrimKey(primkey, ConvertContext(inContext), err, HowToLock);
}


CDB4DEntityRecord* EntityModel::FindEntityWithIdentifyingAtts(const XBOX::VectorOfVString& idents, CDB4DBaseContext* inContext, VErrorDB4D& err, DB4D_Way_of_Locking HowToLock)
{
	return findEntityWithIdentifyingAtts(idents, ConvertContext(inContext), err, HowToLock);
}

CDB4DEntityRecord* EntityModel::FindEntityWithIdentifyingAtts(const XBOX::VValueBag& idents, CDB4DBaseContext* inContext, VErrorDB4D& err, DB4D_Way_of_Locking HowToLock)
{
	return findEntityWithIdentifyingAtts(idents, ConvertContext(inContext), err, HowToLock);
}

CDB4DEntityRecord* EntityModel::FindEntityWithIdentifyingAtts(const XBOX::VectorOfVValue& idents, CDB4DBaseContext* inContext, VErrorDB4D& err, DB4D_Way_of_Locking HowToLock)
{
	return findEntityWithIdentifyingAtts(idents, ConvertContext(inContext), err, HowToLock);
}



EntityMethod* EntityModel::getMethod(const VString& inMethodName, bool publicOnly) const
{
	EntityMethodMap::const_iterator found = fMethodsByName.find(inMethodName);
	if (found == fMethodsByName.end())
		return nil;
	else
	{
		EntityMethod* result = found->second;
		if (publicOnly && result->GetScope() != escope_public)
			result = nil;
		return result;
	}
}


EntityAttribute* EntityModel::getAttribute(sLONG pos) const
{
	if (pos>0 && pos<=fAttributes.size())
		return fAttributes[pos-1];
	else
		return nil;
}


EntityAttribute* EntityModel::getAttribute(const VString& AttributeName) const
{
	EntityAttributeMap::const_iterator found = fAttributesByName.find(AttributeName);
	if (found == fAttributesByName.end())
		return nil;
	else
		return found->second;
}


/*
sLONG EntityModel::FindAttribute(const VString& AttributeName) const
{
	EntityAttribute* ea = getAttribute(AttributeName);
	if (ea == nil)
		return 0;
	else
		return ea->GetPosInOwner();
}
*/


/*
EntityAttribute* EntityModel::FindAttributeByFieldPos(sLONG FieldPos)
{
	EntityAttribute* res = nil;
	for (EntityAttributeCollection::iterator cur = fAttributes.begin(), end = fAttributes.end(); cur != end; cur++)
	{
		EntityAttribute* att = *cur;
		if (att->GetFieldPos() == FieldPos && att->GetKind() == eattr_storage)
		{
			res = att;
			break;
		}
	}
	return res;
}
*/



EntityAttribute* EntityModel::GetAttributeByPath(const VString& inPath) const
{
	AttributeStringPath path(inPath);
	return GetAttributeByPath(path);
}


EntityAttribute* EntityModel::GetAttributeByPath(const AttributeStringPath& inPath) const
{
	const VString* s = inPath.CurPart();
	if (s == nil)
		return nil;
	else
	{
		EntityAttribute* eatt = getAttribute(*s);
		if (eatt == nil)
			return nil;
		else
		{
			s = inPath.NextPart();
			if (s == nil)
				return eatt;
			else
			{
				EntityModel* subem = eatt->GetSubEntityModel();
				if (subem == nil)
					return nil;
				else
					return subem->GetAttributeByPath(inPath);
			}
		}
	}
}


bool EntityModel::GetAllSortedAttributes(EntityAttributeSortedSelection& outAtts, RestTools* req) const
{
	try
	{
		outAtts.reserve(fAttributes.size());
		for (EntityAttributeCollection::const_iterator cur = fAttributes.begin(), end = fAttributes.end(); cur != end; cur++)
		{
			if (req == nil || (*cur)->getScope() == escope_public)
				outAtts.push_back(EntityAttributeSortedItem(*cur));
		}
	}
	catch (...) 
	{
		return false;
	}
	return true;
}


bool EntityModel::BuildListOfSortedAttributes(const VString& inAttributeList, EntityAttributeSortedSelection& outAtts, BaseTaskInfo* context, bool FirstLevelOnly, bool forSorting, RestTools* req) const
{
	bool result = outAtts.BuildFromString(inAttributeList, context, FirstLevelOnly, forSorting, req);
	return result;
}



bool EntityModel::GetAllAttributes(EntityAttributeSelection& outAtts, RestTools* req) const
{
	try
	{
		for (EntityAttributeCollection::const_iterator cur = fAttributes.begin(), end = fAttributes.end(); cur != end; cur++)
		{
			if (req == nil || (*cur)->getScope() == escope_public)
				outAtts[(*cur)->GetPosInOwner()-1].fAttribute = *cur;
		}
	}
	catch (...) 
	{
		return false;
	}
	return true;
}


bool EntityModel::BuildListOfAttributes(const VString& inAttributeList, EntityAttributeSelection& outAtts, bool FirstLevelOnly, RestTools* req) const
{
	bool result = outAtts.BuildFromString(inAttributeList, FirstLevelOnly, req);
	return result;
}


/*
CDB4DSelection* EntityModel::ProjectSelection(CDB4DSelection* sel, CDB4DEntityAttribute* att, VError& err, CDB4DBaseContext* context)
{
	Selection* result = nil;
	VDB4DSelection* xresult = nil;
	EntityAttribute* xatt = dynamic_cast<EntityAttribute*>(att);
	EntityModel* destModel = xatt->GetSubEntityModel();
	Selection* xsel = dynamic_cast<VDB4DSelection*>(sel)->GetSel();
	result = projectSelection(xsel, xatt, err, ConvertContext(context));
	if (result != nil)
	{
		CDB4DBase* xbase = fOwner->RetainBaseX();
		xresult = new VDB4DSelection(VDBMgr::GetManager(), dynamic_cast<VDB4DBase*>(xbase), destModel->GetMainTable(), result, destModel);
		xbase->Release();
	}
	return xresult;
}


Selection* EntityModel::projectSelection(Selection* sel, EntityAttribute* att, VError& err, BaseTaskInfo* context)
{
	err = VE_OK;
	Selection* result = nil;

	EntityRelation* relpath = att->GetRelPath();
	if (testAssert(relpath != nil) && att->GetSubEntityModel() != nil)
	{
		const vector<EntityRelation*>& path = relpath->GetPath();

		Table* finaltarget = nil;
		if (!path.empty())
			finaltarget = path[path.size()-1]->GetDestTable();

		if (testAssert(finaltarget != nil))
		{
			vector<sLONG> dejainstance;
			dejainstance.resize(GetOwner()->GetNBTable()+1, 0);

			SearchTab query(finaltarget);
			bool first = true;
			for (vector<EntityRelation*>::const_iterator cur = path.begin(), end = path.end(); cur != end; cur++)
			{
				sLONG sourcetablenum = (*cur)->GetSourceField()->GetOwner()->GetNum();
				sLONG desttablenum = (*cur)->GetDestField()->GetOwner()->GetNum();
				sLONG instancedest = dejainstance[desttablenum];
				if (sourcetablenum == desttablenum)
					dejainstance[desttablenum]++;
				sLONG instancesource = dejainstance[sourcetablenum];
				query.AddSearchLineJoin((*cur)->GetSourceField(), DB4D_Equal, (*cur)->GetDestField(), false, instancesource, instancedest);
				query.AddSearchLineBoolOper(DB4D_And);
			}

			query.AddSearchLineSel(sel, dejainstance[sel->GetParentFile()->GetNum()]);

			OptimizedQuery xquery;
#if 0 && debuglr
			Boolean olddescribe = context->ShouldDescribeQuery();
			context->MustDescribeQuery(true);
#endif

			err = xquery.AnalyseSearch(&query, context);
			if (err == VE_OK)
			{
				if (okperm(context, att->GetSubEntityModel(), DB4D_EM_Read_Perm) || okperm(context, att->GetSubEntityModel(), DB4D_EM_Update_Perm) || okperm(context, att->GetSubEntityModel(), DB4D_EM_Delete_Perm))
				{
					result = xquery.Perform((Bittab*)nil, nil, context, err, DB4D_Do_Not_Lock);
				}
				else
				{
					err = att->GetSubEntityModel()->ThrowError(VE_DB4D_NO_PERM_TO_READ);
					context->SetPermError();
				}
			}
#if 0 && debuglr
			context->MustDescribeQuery(olddescribe);
#endif
		}
	}

	return result;
}
*/

VError EntityModel::ActivatePath(EntityRecord* erec, sLONG inPathID, SubEntityCache& outResult, bool Nto1, EntityModel* subEntityModel, BaseTaskInfo* context)
{
	VError err = VE_OK;
	
	outResult.Clear();
	outResult.SetActivated();
	outResult.SetModel(subEntityModel);

	EntityRelation* relpath = fRelationPaths[inPathID];
	const EntityRelationCollection& path = relpath->GetPath();

	if (relpath->IsSimple() && Nto1)
	{
		if (erec != nil)
		{
			const EntityRelation* rel = path[0];
			outResult.SetErec(erec->do_LoadRelatedEntity(rel->GetSourceAtt(), rel->GetDestAtt(), context, err, DB4D_Do_Not_Lock));
		}

	}
	else
	{
		EntityRecord* curErec = erec;

		sLONG posInrel = -1;
		bool found1ToNinPath = false, allstop = false;

		for (EntityRelationCollection::const_iterator cur = path.begin(), end = path.end(); cur != end && !found1ToNinPath && !allstop && err == VE_OK; ++cur, ++posInrel)
		{
			if (curErec == nil)
				allstop = true;
			else
			{
				const EntityRelation* rel = *cur;
				if (rel->GetKind() == erel_Nto1)
				{
					EntityAttributeValue* val = curErec->getAttributeValue(rel->GetSourceAtt(), err, context, false);
					if (err == VE_OK)
					{
						curErec = val->getRelatedEntity();
					}
					else
					{
						allstop = true;
						curErec = nil;
					}
				}
				else
					found1ToNinPath = true;
			}
		}

		if (allstop)
		{
		}
		else if (found1ToNinPath)
		{
			SearchTab query(subEntityModel);
			InstanceMap instances;
			sLONG previnstance = 0;
			sLONG nbrel = (sLONG)path.size(), i = nbrel-1;
			bool emptyResult = false;
			for (EntityRelationCollection::const_reverse_iterator cur = path.rbegin(), end = path.rend(); cur != end && !emptyResult && i >= posInrel; ++cur, --i)
			{
				const EntityRelation* rel = *cur;
				const EntityAttribute* sourceAtt = rel->GetSourceAtt();
				const EntityAttribute* destAtt = rel->GetDestAtt();

				if (i == posInrel)
				{
					VValueSingle* cv = nil;

					VCompareOptions options;
					options.SetDiacritical(false);

					ObjectPath objpath;
					EntityAttributeValue* val = curErec->getAttributeValue(sourceAtt, err, context, false);
					if (val != nil)
						cv = val->getVValue();
					if (cv == nil || cv->IsNull())
						emptyResult = true;
					else
						query.AddSearchLineEmSimple(destAtt, objpath, DB4D_Equal, cv, options, previnstance );
				}
				else
				{
					EntityModel* sourceModel = sourceAtt->GetModel();
					EntityModel* destModel = destAtt->GetModel();
					sLONG destinstance = previnstance;
					if (destModel == sourceModel)
						instances.GetInstanceAndIncrement(destModel);
					sLONG sourceinstance = instances.GetInstanceAndIncrement(sourceModel);
					previnstance = sourceinstance;

					query.AddSearchLineJoinEm(sourceAtt, DB4D_Equal, destAtt, false, sourceinstance, destinstance);
					query.AddSearchLineBoolOper(DB4D_And);
				}
			}

			if (emptyResult)
			{
				/*
				if (Nto1)
					outResult.SetErec(nil);
				else
					outResult.SetSel(nil);
					*/
			}
			else
			{
				/*
				SearchTab restsearch(subEntityModel);
				if (subEntityModel->AddRestrictionToQuery(restsearch, context, err))
				{
					query.Add(restsearch);
				}
				if (subEntityModel->WithRestrictingQuery())
				{
					query.Add(*(subEntityModel->fRestrictingQuery));
				}
				*/

				if (okperm(context, subEntityModel, DB4D_EM_Read_Perm) || okperm(context, subEntityModel, DB4D_EM_Update_Perm) || okperm(context, subEntityModel, DB4D_EM_Delete_Perm))
				{
					EntityCollection* col = subEntityModel->executeQuery(&query, context, nil, nil, DB4D_Do_Not_Lock, 0, nil, &err);
					if (err == VE_OK && col != nil)
					{
						if (Nto1)
						{
							xbox_assert(!Nto1); // ne devrait plus passer par ici
							if (col->GetLength(context) > 0)
							{
								EntityRecord* rec = col->LoadEntity(0, context, err, DB4D_Do_Not_Lock);
								if (rec != nil)
								{
									outResult.SetErec(rec);
								}
							}
							QuickReleaseRefCountable(col);
						}
						else
							outResult.SetSel(col);
					}
					else
						QuickReleaseRefCountable(col);
				}
				else
				{
					err = subEntityModel->ThrowError(VE_DB4D_NO_PERM_TO_READ);
					context->SetPermError();
				}
			}

		}
		else
		{
			xbox_assert(Nto1);
			if (curErec != nil)
			{
				xbox_assert(subEntityModel == curErec->GetModel());
				outResult.SetErec(RetainRefCountable(curErec));
			}
		}
	}
	return err;
}


VError EntityModel::BuildRelPath(SubPathRefSet& already, EntityModelCatalog* catalog, const VectorOfVString& path, EntityRelationCollection& outRelPath, VString& outLastpart, bool& outAllNto1, ModelErrorReporter* errorReporter, EntityModel* &outLastDest)
{
	VError err = VE_OK;

	VString attname = path[0];

	EntityAttribute* att = getAttribute(attname);
	if (att == nil)
	{
		if (errorReporter == nil)
			err = ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_NOT_FOUND, &attname);
		else
		{
			ModelErrorContextInstaller errs(errorReporter);
			ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_NOT_FOUND, &attname);
		}
	}
	else
	{
		bool deja = false;
		SubPathRef part(this, attname);
		if (already.find(part) != already.end())
		{
			/*
			if (errorReporter == nil)
				err = ThrowError(VE_DB4D_RELATION_PATH_IS_RECURSIVE, &attname);
			else
				catalog->AddError();
				*/
			deja = true;
		}
		
		{
			already.insert(part);
			EntityAttributeKind what = att->GetKind();
			if (what == eattr_relation_Nto1 || what == eattr_relation_1toN || what == eattr_composition)
			{
				if (what != eattr_relation_Nto1)
					outAllNto1 = false;
				if (deja)
					err = VE_OK;
				else
					err = att->ResolveRelatedEntities(already, catalog, errorReporter, nil);
				if (err == VE_OK)
				{
					EntityRelation* rel = att->GetRelPath();
					if (rel != nil)
					{
						outRelPath.push_back(rel);
						EntityModel* dest = att->GetSubEntityModel();
						if (dest != nil)
						{
							outLastDest = dest;
							if (path.size() != 1)
							{
								VectorOfVString subpath = path;
								subpath.erase(subpath.begin());
								err = dest->BuildRelPath(already, catalog, subpath, outRelPath, outLastpart, outAllNto1, errorReporter, outLastDest);
							}
						}
						else
						{
							if (errorReporter == nil)
								err = ThrowError(VE_DB4D_ENTITY_RELATION_DOES_NOT_EXIST, &attname);
							else
							{
								ModelErrorContextInstaller errs(errorReporter);
								ThrowError(VE_DB4D_ENTITY_RELATION_DOES_NOT_EXIST, &attname);
							}
						}
					}
					else
					{
						if (errorReporter == nil)
							err = ThrowError(VE_DB4D_ENTITY_RELATION_DOES_NOT_EXIST, &attname);
						else
						{
							ModelErrorContextInstaller errs(errorReporter);
							ThrowError(VE_DB4D_ENTITY_RELATION_DOES_NOT_EXIST, &attname);
						}

					}
				}
			}
			else
			{
				if (path.size() != 1)
				{
					if (errorReporter == nil)
						err = ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_IS_NOT_NAVIGATION, &attname);
					else
					{
						ModelErrorContextInstaller errs(errorReporter);
						ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_IS_NOT_NAVIGATION, &attname);
					}
				}
				else
				{
					outLastpart = attname;
				}
			}
		}
	}

	return err;
}


VError EntityModel::SaveToBag(VValueBag& CatalogBag, bool forJSON) const
{
	VValueBag* entityBag = new VValueBag();
	VError err = ToBag(*entityBag, false, true, forJSON);
	if (err == VE_OK)
	{
		CatalogBag.AddElement(d4::dataClasses, entityBag);
	}
	entityBag->Release();

	
	if (err == VE_OK)
	{
		for (ListOfModels::const_iterator cur = fDerivateds.begin(), end = fDerivateds.end(); cur != end && err == VE_OK; cur++)
		{
			err = (*cur)->SaveToBag(CatalogBag, forJSON);
		}
	}
	

	return err;
}



VError EntityModel::ToBag(VValueBag& outBag, bool forDax, bool forSave, bool forJSON, RestTools* req) const
{
	VError err = VE_OK;

	if (fExtraProperties != nil)
		outBag.AddElement(d4::extraProperties, (VValueBag*)fExtraProperties);

	outBag.SetString(d4::name, fName);
	outBag.SetString(d4::className, fName);
	outBag.SetString(d4::collectionName, fCollectionName);
	if (!fAlternateTableNames.empty())
	{
		VString s;
		for (VectorOfVString::const_iterator cur = fAlternateTableNames.begin(), end = fAlternateTableNames.end(); cur != end; ++cur)
		{
			if (!s.IsEmpty())
				s += ",";
			s += *cur;
		}
		outBag.SetString(d4::matchTable, s);
	}
	if (fScope != escope_none)
		outBag.SetString(d4::scope, EScopeKinds[(sLONG)fScope]);
	if (!fDB4DUUID.IsNull())
		outBag.SetVUUID(d4::uuid, fDB4DUUID);

	if (forSave)
	{
		if (fAllowOverrideStamp)
			outBag.SetBool(d4::allowOverrideStamp, fAllowOverrideStamp);
		if (fPublishAsGlobalDefined)
			outBag.SetBool(d4::publishAsJSGlobal, fPublishAsGlobal);
		if (!fExtends.IsEmpty())
			outBag.SetString(d4::extends, fExtends);
	}

	if (!forSave && req != nil)
	{
		VString uri;
		req->CalculateDataURI(uri, this, L"", false, false);
		outBag.SetString(d4::dataURI, uri);
	}

	if (fDefaultTopSize != 0)
	{
		outBag.SetLong(d4::defaultTopSize, fDefaultTopSize);
	}

	if (fNoEdit)
		outBag.SetBool(d4::noEdit, true);
	if (fNoSave)
		outBag.SetBool(d4::noSave, true);

	if (WithRestrictingQuery() && forSave)
	{
		VValueBag* queryBag = new VValueBag();
		if (forJSON)
			queryBag->SetBool(____objectunic, true);

		if (forSave)
		{
			queryBag->SetString(d4::queryStatement, fRestrictingQueryString);
		}
		else
		{
			queryBag->SetString(d4::queryStatement, fCumulatedRestrictingQueryString);
		}

		if (fQueryLimit != 0)
			queryBag->SetLong(d4::top, fQueryLimit);

		if (!fRestrictingOrderByString.IsEmpty())
		{
			queryBag->SetString(d4::orderBy, fRestrictingOrderByString);
		}

		outBag.AddElement(d4::restrictingQuery, queryBag);
		queryBag->Release();
	}

	if (forSave)
	{
		for (list<VString>::const_iterator cur = fRemoveAttributes.begin(), end = fRemoveAttributes.end(); cur != end && err == VE_OK; cur++)
		{
			VValueBag* attBag = new VValueBag();
			attBag->SetString(d4::name, *cur);
			attBag->SetString(d4::kind, EattTypes[eattr_remove]);
			outBag.AddElement(d4::attributes, attBag);
			attBag->Release();
		}
	}

	for (EntityAttributeCollection::const_iterator cur = fAttributes.begin(), end = fAttributes.end(); cur != end && err == VE_OK; cur++)
	{
		const EntityAttribute* att = *cur;
		if ((att->IsOverWritten() || !forSave) && (req == nil || att->getScope() == escope_public))
		{
			VValueBag* attBag = new VValueBag();
			att->ToBag(*attBag, forDax, forSave, forJSON, forSave);
			outBag.AddElement(d4::attributes, attBag);
			attBag->Release();
		}
	}

	err = fDBEvents.ToBag(outBag, forDax, forSave, forJSON, true);
	/*
	for (DBEventConstIterator cur = &fEvents[dbev_firstEvent], end = &fEvents[dbev_lastEvent+1]; cur != end && err == VE_OK; cur++)
	{
		if (cur->IsValid() && (cur->IsOverWritten() || !forSave))
		{
			BagElement evBag(outBag, d4::events);
			err = cur->ToBag(*evBag, forDax, forSave, forJSON, true);
		}
	}
	*/

	for (EntityMethodMap::const_iterator cur = fMethodsByName.begin(), end = fMethodsByName.end(); cur != end && err == VE_OK; cur++)
	{
		const EntityMethod* meth = cur->second;
		if ((meth->IsOverWritten() || !forSave) && (req == nil || meth->getScope() == escope_public))
		{
			BagElement methBag(outBag, d4::methods);
			err = meth->ToBag(*methBag, forDax, forSave, forJSON);
		}
	}

	const IdentifyingAttributeCollection* prims;
	if (forSave)
		prims = &fOwnedPrimaryAtts;
	else
		prims = &fPrimaryAtts;

	if (!prims->empty())
	{
		for (IdentifyingAttributeCollection::const_iterator cur = prims->begin(), end = prims->end(); cur != end; cur++)
		{
			BagElement identbag(outBag, d4::key);
			if ( cur->fAtt )
				identbag->SetString(d4::name, cur->fAtt->GetName());
		}
	}

	const IdentifyingAttributeCollection* idents;
	if (forSave)
		idents = &fOwnedIdentifyingAtts;
	else
		idents = &fIdentifyingAtts;

	if (!idents->empty())
	{
		for (IdentifyingAttributeCollection::const_iterator cur = idents->begin(), end = idents->end(); cur != end; cur++)
		{
			BagElement identbag(outBag, d4::identifyingAttribute);
			identbag->SetString(d4::name, cur->fAtt->GetName());
			if (cur->fOptionnel)
				identbag->SetBool(d4::optionnal, true);
		}
	}

	return err;
}


const VString ScopeDefName("scope");
const VString ApplyDefName("applyTo");


VError EntityModel::ExtendsFrom(EntityModel* otherEm)
{
	VError err = VE_OK;
	fRelationPaths.resize(otherEm->fRelationPaths.size());
	if (otherEm->fRestrictingQuery != nil)
	{
		fRestrictingQuery = new SearchTab(this);
		fRestrictingQuery->From(*(otherEm->fRestrictingQuery));
	}
	else
		fRestrictingQuery = nil;
	fQueryLimit = otherEm->fQueryLimit;
	fRelationPaths = otherEm->fRelationPaths;
	fDefaultTopSize = otherEm->fDefaultTopSize;
	for (EntityAttributeCollection::const_iterator cur = otherEm->fAttributes.begin(), end = otherEm->fAttributes.end(); cur != end; cur++)
	{
		EntityAttribute* Att = (*cur)->Clone(this);
		fAttributes.push_back(Att);
		fAttributesByName[Att->GetName()] = Att;
		Att->SetPosInOwner((sLONG)fAttributes.size());
	}
	for (EntityMethodMap::const_iterator cur = otherEm->fMethodsByName.begin(), end = otherEm->fMethodsByName.end(); cur != end; cur++)
	{
		EntityMethod* Meth = cur->second->Clone(this);
		fMethodsByName[cur->first] = Meth;
	}

	//fDBEvents.CopyFrom(this, otherEm->fDBEvents);
	/*
	DBEventIterator curdest = &fEvents[dbev_firstEvent];
	for (DBEventConstIterator cur = &otherEm->fEvents[dbev_firstEvent], end = &otherEm->fEvents[dbev_lastEvent+1]; cur != end && err == VE_OK; cur++, curdest++)
	{
		if (cur->IsValid())
			curdest->CopyFrom(this, cur);
	}
	*/
	VString cumulQuery = otherEm->GetCumulatedQuery();
	fCumulatedRestrictingQueryString = cumulQuery;
	fIdentifyingAtts = otherEm->fIdentifyingAtts;
	fPrimaryAtts = otherEm->fPrimaryAtts;
	for (IdentifyingAttributeCollection::iterator cur = fPrimaryAtts.begin(), end = fPrimaryAtts.end(); cur != end; cur++)
	{
		cur->fAtt = getAttribute(cur->fAtt->GetName());
	}
	for (IdentifyingAttributeCollection::iterator cur = fIdentifyingAtts.begin(), end = fIdentifyingAtts.end(); cur != end; cur++)
	{
		cur->fAtt = getAttribute(cur->fAtt->GetName());
	}
	fBaseModelName = otherEm->GetBaseModelName();
	otherEm->AddDerivated(this);
	//otherEm->Release();
	return err;
}



VError EntityModel::FromBag(const VValueBag* inBag, EntityModelCatalog* catalog, ModelErrorReporter* errorReporter, bool addToClassOnly, bool allowAttributeChange)
{
	VError err = VE_OK;

	if (!addToClassOnly)
	{
		fExtraProperties = inBag->RetainUniqueElement(d4::extraProperties);

		VUUID xid;
		if (inBag->GetVUUID(d4::uuid, xid))
		{
			fDB4DUUID = xid;
		}
		else
			fDB4DUUID.SetNull();

		bool AsGlobal;
		if (inBag->GetBool(d4::publishAsJSGlobal, AsGlobal))
		{
			fPublishAsGlobal = AsGlobal;
			fPublishAsGlobalDefined = true;
		}
		else
		{
			fPublishAsGlobal = catalog->publishDataClassesAsGlobals();
			fPublishAsGlobalDefined = false;
		}

		bool allowStamps;
		if (inBag->GetBool(d4::allowOverrideStamp, allowStamps))
		{
			fAllowOverrideStamp = allowStamps;
		}
		else
		{
			fAllowOverrideStamp = false;
		}
	}

	VString sTables;
	if (inBag->GetString(d4::matchTable, sTables) && !sTables.IsEmpty())
	{
		VectorOfVString tablenames;
		sTables.GetSubStrings(',', tablenames, false, true);
		if (!tablenames.empty())
		{
			if (fAlternateTableNames.empty())
				fAlternateTableNames = tablenames;
			else
			{
				for (VectorOfVString::iterator cur = tablenames.begin(), end = tablenames.end(); cur != end; ++cur)
				{
					fAlternateTableNames.push_back(*cur);
				}
			}
		}
	}

	bool noedit;
	if (inBag->GetBool(d4::noEdit, noedit))
	{
		fNoEdit = noedit;
	}
	else
	{
		fNoEdit = noedit;
	}

	bool nosave;
	if (inBag->GetBool(d4::noSave, nosave))
	{
		fNoSave = nosave;
	}
	else
	{
		fNoSave = false;
	}

	if (!addToClassOnly)
	{
		if (inBag->GetString(d4::className, fName))
		{
			inBag->GetString(d4::collectionName, fCollectionName);
			if (fCollectionName.IsEmpty())
				fCollectionName = fName + "Collection";
		}
		else
		{
			VString xName;
			if (inBag->GetString(d4::name, xName))
			{
				VString xSingleEntityName;
				inBag->GetString(d4::singleEntityName, xSingleEntityName);
				if (xSingleEntityName.IsEmpty())
				{
					xSingleEntityName = fName + L"_single";
					sLONG ll = xName.GetLength();
					if (ll > 1)
					{
						if (xName[ll-1] == 's' || xName[ll-1] == 'S')
						{
							xName.GetSubString(1, ll-1, xSingleEntityName);
						}
					}
				}
				fName = xSingleEntityName;
				fCollectionName = xName;
			}
			else
			{
				if (errorReporter == nil)
					err = fCatalog->ThrowError(VE_DB4D_ENTITY_NAME_MISSING, noaction);
				else
				{
					ModelErrorContextInstaller errs(errorReporter);
					fCatalog->ThrowError(VE_DB4D_ENTITY_NAME_MISSING, noaction);
				}
			}
		}
		fBaseModelName = fName;
	}

	if (err == VE_OK)
	{
		VString ss;
		fScope = escope_public;
		if (inBag->GetString(d4::scope, ss))
		{
			fScope = (EntityAttributeScope)EScopeKinds[ss];
			if (fScope == escope_none)
				fScope = escope_public;
		}
	}

	VString cumulQuery;
	if (err == VE_OK)
	{
		VString otherEMname;
		if (inBag->GetString(d4::extends, otherEMname))
		{
			fExtends = otherEMname;
			EntityModel* otherEm = catalog->RetainEntity(otherEMname);
			if (otherEm == nil)
				otherEm = catalog->RetainEntityByCollectionName(otherEMname);
			fBaseEm = otherEm;
			if (otherEm != nil)
			{
				err = ExtendsFrom(otherEm);
				cumulQuery = otherEm->GetCumulatedQuery();
				if (err == VE_OK)
				{
					for (EntityAttributeCollection::iterator cur = fAttributes.begin(), end = fAttributes.end(); cur != end && err == VE_OK; cur++)
					{
						EntityAttribute* Att = *cur;
						err = Att->AssociateJSEvents(catalog, errorReporter);
					}
				}
			}
			else
			{
				if (errorReporter != nil)
				{
					ModelErrorContextInstaller errs(errorReporter);
					ThrowError(VE_DB4D_ENTITY_NOT_FOUND_FOR_EXTENDS, &otherEMname);
				}
				else
					err = ThrowError(VE_DB4D_ENTITY_NOT_FOUND_FOR_EXTENDS, &otherEMname);
			}
		}
	}

	if (err == VE_OK)
	{
		if (inBag->GetLong(d4::defaultTopSize, fDefaultTopSize))
		{
			if (fDefaultTopSize < 0)
				fDefaultTopSize = 0;
		}
	}

	if (err == VE_OK)
	{
		const VBagArray* EntityMethodsBag = inBag->GetElements(d4::methods);
		if (EntityMethodsBag != nil)
		{
			VIndex nbmeth = EntityMethodsBag->GetCount();
			for (VIndex i = 1; i <= nbmeth && err == VE_OK; i++)
			{
				const VValueBag* methodBag = EntityMethodsBag->GetNth(i);
				if (methodBag != nil)
				{
					bool userdefined = false;
					methodBag->GetBool(d4::userDefined, userdefined);
					if (!userdefined  || catalog->AllowParseUsedDefined())
					{
						EntityMethod* entMeth = new EntityMethod(this);
						err = entMeth->FromBag(methodBag, catalog, errorReporter);
						if (err == VE_OK && !entMeth->GetName().IsEmpty())
						{
							entMeth->Retain();
							entMeth->SetOverWrite(true);
							fMethodsByName[entMeth->GetName()] = entMeth;
						}
						entMeth->Release();
					}
				}
			}
		}
	}

	if (err == VE_OK)
	{
		const VBagArray* DBEventsBag = inBag->GetElements(d4::events);
		if (DBEventsBag != nil)
		{
			VIndex nbevents = DBEventsBag->GetCount();
			for (VIndex i = 1; i <= nbevents && err == VE_OK; i++)
			{
				const VValueBag* eventBag = DBEventsBag->GetNth(i);
				if (eventBag != nil)
				{
					DBEventKind evkind = dbev_none;
					VString ss;
					bool userDefined = false;
					eventBag->GetBool(d4::userDefined, userDefined);
					if (!userDefined  || catalog->AllowParseUsedDefined())
					{
						if (eventBag->GetString(d4::kind, ss) && !ss.IsEmpty())
						{
							evkind = (DBEventKind)OldEDBEventKinds[ss];
							if (evkind == dbev_none)
								evkind = (DBEventKind)EDBEventKinds[ss];
							/*
							if (evkind == dbev_none)
								evkind = (DBEventKind)oldEDBEventKinds[ss];
								*/
						}
						if (evkind == dbev_none)
						{
							if (errorReporter == nil)
								err = ThrowError(VE_DB4D_MISSING_OR_INVALID_EVENTKIND);
							else
							{
								ModelErrorContextInstaller errs(errorReporter);
								ThrowError(VE_DB4D_MISSING_OR_INVALID_EVENTKIND);
							}
						}
						else
						{
							DBEvent event;
							err = event.FromBag(this, eventBag, catalog, errorReporter);
							if (err == VE_OK)
								fDBEvents.AddEvent(event);
							//err = fEvents[evkind].FromBag(this, eventBag, catalog, errorReporter);
						}
					}
				}
			}
		}
	}

	if (err == VE_OK)
	{
		set<VString> dejaParse;
		const VBagArray* EntityAttsBag = inBag->GetElements(d4::attributes);
		if (EntityAttsBag != nil)
		{
			VIndex nbatts = EntityAttsBag->GetCount();
			for (VIndex i = 1; i <= nbatts && err == VE_OK; i++)
			{
				const VValueBag* attributeBag = EntityAttsBag->GetNth(i);
				if (attributeBag != nil)
				{
					VString attname;
					EntityAttribute* dejaAtt = nil;
					if (attributeBag->GetString(d4::name, attname))
					{
						EntityAttributeMap::iterator found = fAttributesByName.find(attname);
						if (found != fAttributesByName.end())
						{
							dejaAtt = found->second;
						}
					}
					dejaParse.insert(attname);
					bool remove = false;
					VString kindname;
					attributeBag->GetString(d4::kind, kindname);
					if (EattTypes[kindname] == (sLONG)eattr_remove)
					{
						VString attname;
						if (attributeBag->GetString(d4::name, attname))
						{
							fRemoveAttributes.push_back(attname);
							EntityAttributeMap::iterator found = fAttributesByName.find(attname);
							if (found != fAttributesByName.end())
							{
								EntityAttribute* entAtt = found->second;
								fAttributesByName.erase(found);
								EntityAttributeCollection::iterator placeinatts = fAttributes.begin()+(entAtt->GetPosInOwner()-1);
								fAttributes.erase(placeinatts);
								sLONG pos = 1;
								for (EntityAttributeCollection::iterator cur = fAttributes.begin(), end = fAttributes.end(); cur != end; cur++, pos++)
								{
									(*cur)->SetPosInOwner(pos);
								}
								entAtt->Release();
							}

						}
					}
					else if ((dejaAtt == nil) && (EattTypes[kindname] == (sLONG)eattr_storage) && (!fExtends.IsEmpty() || addToClassOnly) )
					{
						// pour l'instant on ne peut pas avoir des attributs qui soient de vrai champs sur un entity model derive
					}
					else
					{
						EntityAttribute* entAtt = nil;
						VString attname;
						bool mustChangeName = false;
						if (attributeBag->GetString(d4::name, attname))
						{
							if (dejaAtt != nil && (fBaseEm != nil || allowAttributeChange))
							{
								entAtt = dejaAtt;
							}
							else
							{
								EntityAttributeMap::iterator found = fAttributesByName.find(attname);
								if (found != fAttributesByName.end())
								{
									//entAtt = found->second->Clone(this); // old code
									if (errorReporter == nil)
										err = ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_ALREADY_EXISTS, &attname);
									else
									{
										ModelErrorContextInstaller errs(errorReporter);
										ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_ALREADY_EXISTS, &attname);
										mustChangeName = true;
										for (sLONG i = 0; i < 100000000; ++i)
										{
											VString s = attname+"_"+ToString(i);
											if (fAttributesByName.find(s) == fAttributesByName.end())
											{
												attname = s;
												break;
											}
										}
									}
								}
							}
						}
						
						if (entAtt == nil)
							entAtt = new EntityAttribute(this);
						if (err == VE_OK)
							err = entAtt->FromBag(attributeBag, catalog, false, errorReporter, allowAttributeChange && (dejaAtt != nil));
						if (err == VE_OK && !entAtt->GetName().IsEmpty())
						{
							if (mustChangeName)
								entAtt->SetName(attname);

							entAtt->SetOverWrite(true);
							EntityAttributeMap::iterator found = fAttributesByName.find(entAtt->GetName());
							if (found == fAttributesByName.end())
							{
								fAttributes.push_back(entAtt);
								fAttributesByName[entAtt->GetName()] = entAtt;
								entAtt->SetPosInOwner((sLONG)fAttributes.size());
							}
							/*
							else
							{
								EntityAttribute* oldAtt = found->second;
								sLONG pos = oldAtt->GetPosInOwner();
								entAtt->SetPosInOwner(pos);
								fAttributes[pos-1] = entAtt;
								found->second = entAtt;
								oldAtt->Release();
							}
							*/

							if (err == VE_OK)
							{
								bool primkey = false;
								if (attributeBag->GetBool(d4::primKey, primkey))
								{
									if (primkey)
									{
										fOwnedPrimaryAtts.clear();
										fOwnedPrimaryAtts.push_back(IdentifyingAttribute(entAtt));
									}
								}
							}
						}
						else
							entAtt->Release();
					}
				}
			}
		}

		if (err == VE_OK)
		{
			for (EntityAttributeCollection::iterator cur = fAttributes.begin(), end = fAttributes.end(); cur != end && err == VE_OK; ++cur)
			{
				EntityAttribute* att = *cur;
				if (dejaParse.find(att->GetName()) == dejaParse.end()) // if the attribute had not bag, it may have events in JS
				{
					VValueBag attbag;
					err = att->FromBag(&attbag, catalog, false, errorReporter, true);
				}
			}
		}
	}

	if (err == VE_OK)
	{
		const VValueBag* QueryBag = inBag->GetUniqueElement(d4::restrictingQuery);
		if (QueryBag != nil)
		{
			sLONG limit = 0;
			if (QueryBag->GetLong(d4::top, limit))
			{
				if (fQueryLimit == 0)
					fQueryLimit = limit;
				else
				{
					if (limit > 0 && limit < fQueryLimit)
						fQueryLimit = limit;
				}
			}

			VString querystring, orderbystring;
			QueryBag->GetString(d4::queryStatement, querystring);
			fRestrictingQueryString = querystring;
			if (cumulQuery.IsEmpty())
				fCumulatedRestrictingQueryString = fRestrictingQueryString;
			else
				fCumulatedRestrictingQueryString = cumulQuery +L" and ("+fRestrictingQueryString+L")";

			if (QueryBag->GetString(d4::orderBy, fRestrictingOrderByString))
			{
				// mettre un check ici ?
			}

		}
	}

	if (err == VE_OK && !addToClassOnly)
	{
		const VBagArray* idents = inBag->GetElements(d4::identifyingAttribute);
		if (idents != nil)
		{
			fOwnedIdentifyingAtts.clear();
			for (VIndex i = 1, nb = idents->GetCount(); i <= nb && err == VE_OK; i++)
			{
				const VValueBag* identbag = idents->GetNth(i);
				VString s;
				if (identbag->GetString(d4::name, s))
				{
					EntityAttribute* att = getAttribute(s);
					if (att != nil)
					{
						if (att->GetKind() == eattr_storage)
						{
							bool optionnal = false;
							identbag->GetBool(d4::optionnal, optionnal);
							IdentifyingAttribute idatt;
							idatt.fAtt = att;
							idatt.fOptionnel = optionnal;
							fOwnedIdentifyingAtts.push_back(idatt);
						}
						else
						{
							if (errorReporter == nil)
								err = att->ThrowError(VE_DB4D_WRONG_ATTRIBUTE_TYPE_FOR_IDENT);
							else
							{
								ModelErrorContextInstaller errs(errorReporter);
								att->ThrowError(VE_DB4D_WRONG_ATTRIBUTE_TYPE_FOR_IDENT);
							}
						}
					}
					else
					{
						if (errorReporter == nil)
							err = ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_NOT_FOUND, &s);
						else
						{
							ModelErrorContextInstaller errs(errorReporter);
							ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_NOT_FOUND, &s);
						}
					}

				}
				else
				{
					if (errorReporter == nil)
						err = ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_NAME_MISSING);
					else
					{
						ModelErrorContextInstaller errs(errorReporter);
						ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_NAME_MISSING);
					}
				}
			}
		}
	}


	if (err == VE_OK && !addToClassOnly)
	{
		const VBagArray* prims = inBag->GetElements(d4::key);
		if (prims != nil)
		{
			fOwnedPrimaryAtts.clear();
			for (VIndex i = 1, nb = prims->GetCount(); i <= nb && err == VE_OK; i++)
			{
				const VValueBag* identbag = prims->GetNth(i);
				VString s;
				if (identbag->GetString(d4::name, s))
				{
					EntityAttribute* att = getAttribute(s);
					if (att != nil)
					{
						if (att->GetKind() == eattr_storage)
						{
							IdentifyingAttribute idatt;
							idatt.fAtt = att;
							idatt.fOptionnel = false;
							fOwnedPrimaryAtts.push_back(idatt);
						}
						else
						{
							if (errorReporter == nil)
								err = att->ThrowError(VE_DB4D_WRONG_ATTRIBUTE_TYPE_FOR_PRIMKEY);
							else
							{
								ModelErrorContextInstaller errs(errorReporter);
								att->ThrowError(VE_DB4D_WRONG_ATTRIBUTE_TYPE_FOR_PRIMKEY);
							}
						}
					}
					else
					{
						if (errorReporter == nil)
							err = ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_NOT_FOUND, &s);
						else
						{
							ModelErrorContextInstaller errs(errorReporter);
							ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_NOT_FOUND, &s);
						}
					}

				}
				else
				{
					if (errorReporter == nil)
						err = ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_NAME_MISSING);
					else
					{
						ModelErrorContextInstaller errs(errorReporter);
						ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_NAME_MISSING);
					}
				}
			}
		}
	}

	if (err == VE_OK && !addToClassOnly)
	{
		if (!fOwnedIdentifyingAtts.empty())
			fIdentifyingAtts = fOwnedIdentifyingAtts;
		if (!fOwnedPrimaryAtts.empty())
			fPrimaryAtts = fOwnedPrimaryAtts;
	}

	if (err == VE_OK && !addToClassOnly)
	{
		for (IdentifyingAttributeCollection::iterator cur = fPrimaryAtts.begin(), end = fPrimaryAtts.end(); cur != end; cur++)
		{
			if ( cur->fAtt )
				cur->fAtt->SetPartOfPrimKey();
		}
	}

	if (err == VE_OK)
	{
		for (EntityAttributeCollection::iterator cur = fAttributes.begin(), end = fAttributes.end(); cur != end; cur++)
		{
			sLONG pos = cur - fAttributes.begin();
			EntityAttribute* att = *cur;
			if (att->HasEvent(dbev_init))
			{
				fAttributesWithInitEvent.push_back(att);
			}
			const VValueSingle* cv = nil;
			if (att->IsScalar() && att->getDefaultValue(cv))
			{
				fAttributesWithDefaultValue.push_back(att);
			}
		}
	}

	if (err == VE_OK)
	{
		err = addJSExtensions(catalog, errorReporter, addToClassOnly, allowAttributeChange);
	}

	fWithRestriction = false;

	if (HasEvent(dbev_restrict))
	//if (fEvents[dbev_restrict].IsValid())
		fWithRestriction = true;

	fHasDeleteEvent = false;
	fOneAttributeHasDeleteEvent = false;
	if (HasEvent(dbev_remove))
	//if (fEvents[dbev_remove].IsValid())
		fHasDeleteEvent = true;
	
	{
		for (EntityAttributeCollection::iterator cur = fAttributes.begin(), end = fAttributes.end(); cur != end && !fOneAttributeHasDeleteEvent; ++cur)
		{
			EntityAttribute* att = *cur;
			if (att != nil && att->HasEvent(dbev_remove))
			{
				fHasDeleteEvent = true;
				fOneAttributeHasDeleteEvent = true;
			}
		}
	}

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOT_BUILD_EM_FROM_DEF);
	return err;
}


VError EntityModel::addJSExtensions( EntityModelCatalog* catalog, ModelErrorReporter* errorReporter, bool addToClassOnly, bool allowAttributeChange)
{
	VError err = VE_OK;
	VJSContext	jscontext(*(catalog->getLoadingContext()));
	VJSObject	result(jscontext);
	VString		path = catalog->GetModelJSName() + "." + fName;

	bool				isOk		= false;
	VJSException		exception;

	result = jscontext.GetGlobalObject().GetPropertyAsObject("model", exception);
	if (exception.IsEmpty() && result.IsObject()) {

		result = result.GetPropertyAsObject(fName, exception);
		isOk = exception.IsEmpty() && result.IsObject();

	}

	if (isOk /* result.IsObject() */)
	{
		VJSObject attObj(result);
		VJSPropertyIterator curprop(attObj);
		while (curprop.IsValid())
		{
			VString propName;
			curprop.GetPropertyName(propName);
			if (propName.EqualToUSASCIICString("events"))
			{
				VJSValue eventsVal(curprop.GetProperty());
				if (eventsVal.IsObject())
				{
					VJSObject eventsObj(eventsVal.GetObject());
					VJSPropertyIterator curevent(eventsObj);
					while (curevent.IsValid())
					{
						VString eventName;
						bool oldway = false;
						curevent.GetPropertyName(eventName);
						DBEventKind evkind = (DBEventKind)OldEDBEventKinds[eventName];
						if (evkind == dbev_none)
							evkind = (DBEventKind)EDBEventKinds[eventName];
						else
							oldway = true;

						if (evkind != dbev_none)
						{
							VJSValue eventFunc(curevent.GetProperty());
							if (eventFunc.IsFunction())
							{
								VString path2 = path + ".events."+eventName;
								DBEvent event;
								event.SetEvent(this, evkind, path2, oldway);
								fDBEvents.AddEvent(event);
								//fEvents[evkind].SetEvent(this, evkind, path2, oldway);
							}
							else if (eventFunc.IsArray())
							{
								VJSArray arr(eventFunc, false);
								for (sLONG i = 0, nb = arr.GetLength(); i < nb; ++i)
								{
									VJSValue eventFunc2 = arr.GetValueAt(i);
									if (eventFunc2.IsFunction())
									{
										VString path2 = path + ".events." + eventName + "[" + ToString(i) + "]";
										DBEvent event;
										event.SetEvent(this, evkind, path2, oldway);
										fDBEvents.AddEvent(event);
										//fEvents[evkind].SetEvent(this, evkind, path2, oldway);
									}
								}
							}
						}

						++curevent;
					}
				}
			}
			else
			{
				EntityMethodKind applyTo =  emeth_none;

				if (propName.EqualToUSASCIICString("methods"))
				{
					applyTo = emeth_static;
				}
				else if (propName.EqualToUSASCIICString("entityMethods"))
				{
					applyTo = emeth_rec;
				}
				else if (propName.EqualToUSASCIICString("collectionMethods"))
				{
					applyTo = emeth_sel;
				}

				if (applyTo != emeth_none)
				{
					VJSValue methodsVal(curprop.GetProperty());
					if (methodsVal.IsObject())
					{
						VJSObject methodsObj(methodsVal.GetObject());
						VJSPropertyIterator curmethod(methodsObj);
						while (curmethod.IsValid())
						{
							VJSValue funcVal(curmethod.GetProperty());
							if (funcVal.IsFunction())
							{
								VJSObject funcObj(funcVal.GetObject());
								VString funcname;
								VString scopestring;
								EntityAttributeScope scope = escope_public_server;

								if (funcObj.HasProperty(ScopeDefName))
								{
									if (funcObj.GetPropertyAsString(ScopeDefName, nil, scopestring))
									{
										scope = (EntityAttributeScope)EScopeKinds[scopestring];
										if (scope == escope_none)
											scope = escope_public_server;
									}
								}
								else
								{
									if (funcObj.HasProperty("name"))
									{
										if (funcObj.GetPropertyAsString("name", nil, scopestring))
										{
											scope = (EntityAttributeScope)EScopeKinds[scopestring];
											if (scope == escope_none)
												scope = escope_public_server;
										}
									}
								}
								curmethod.GetPropertyName(funcname);
								VString path2 = path+"."+propName+"."+funcname;
								EntityMethod* entMeth = new EntityMethod(this);
								err = entMeth->FromJS(funcname, applyTo, path2, scope,  catalog, errorReporter);
								if (err == VE_OK)
								{
									entMeth->Retain();
									entMeth->SetOverWrite(true);
									fMethodsByName[funcname] = entMeth;
								}
								entMeth->Release();
							}

							++curmethod;
						}
					}
				}
				else
				{
					VJSValue funcVal(curprop.GetProperty());
					if (funcVal.IsFunction())
					{
						if (propName.EqualToUSASCIICString("setRestrictingQuery") || propName.EqualToUSASCIICString("setProperties") || 
							propName.EqualToUSASCIICString("Attribute") || propName.EqualToUSASCIICString("addRelatedEntity") || 
							propName.EqualToUSASCIICString("addRelatedEntities") || propName.EqualToUSASCIICString("addAttribute") ||
							propName.EqualToUSASCIICString("addMethod") || propName.EqualToUSASCIICString("addEventListener") || propName.EqualToUSASCIICString("removeAttribute"))
						{
							// predefined keyword, not allowed for a custom function
						}
						else
						{
							applyTo = emeth_static;
							VJSObject funcObj(funcVal.GetObject());
							VString funcname;
							VString scopestring;
							EntityAttributeScope scope = escope_public_server;

							VString applyStr;
							if (funcObj.HasProperty(ApplyDefName))
							{
								if (funcObj.GetPropertyAsString(ApplyDefName, nil, applyStr))
								{
									applyTo = (EntityMethodKind)EmethTypes[applyStr];
									if (applyTo == emeth_none)
										applyTo = emeth_static;
								}
							}

							if (funcObj.HasProperty(ScopeDefName))
							{
								if (funcObj.GetPropertyAsString(ScopeDefName, nil, scopestring))
								{
									scope = (EntityAttributeScope)EScopeKinds[scopestring];
									if (scope == escope_none)
										scope = escope_public_server;
								}
							}
							else
							{
								if (funcObj.HasProperty("name"))
								{
									if (funcObj.GetPropertyAsString("name", nil, scopestring))
									{
										scope = (EntityAttributeScope)EScopeKinds[scopestring];
										if (scope == escope_none)
											scope = escope_public_server;
									}
								}
							}
							funcname = propName;
							VString path2 = path+"."+funcname;
							EntityMethod* entMeth = new EntityMethod(this);
							err = entMeth->FromJS(funcname, applyTo, path2, scope,  catalog, errorReporter);
							if (err == VE_OK)
							{
								entMeth->Retain();
								entMeth->SetOverWrite(true);
								fMethodsByName[funcname] = entMeth;
							}
							entMeth->Release();
						}
					}

				}

			}

			++curprop;
		}

	}
	return err;
}

bool EntityModel::HasDeleteEvent(bool onlyCheckAttributes) const
{
	if (onlyCheckAttributes)
		return fOneAttributeHasDeleteEvent;
	else
		return fHasDeleteEvent;
}


void EntityModel::AddDerivated(EntityModel* em)
{
	fDerivateds.push_back(em);
}


VError EntityModel::AddMethod(EntityMethod* meth)
{
	meth->Retain();
	meth->SetOverWrite(true);
	fMethodsByName[meth->GetName()] = meth;
	return VE_OK;
}


bool EntityModel::isExtendedFrom(const EntityModel* otherEM) const
{
	bool res = false;
	if (this == otherEM)
		res = true;
	else
	{
		if (fBaseEm != nil)
			res = fBaseEm->isExtendedFrom(otherEM);
	}
	return res;
}



sLONG EntityModel::FindRelationPath(const EntityRelation* relpath) const
{
	sLONG result = -1;
	sLONG i = 0;
	for (EntityRelationCollection::const_iterator cur = fRelationPaths.begin(), end = fRelationPaths.end(); cur != end && result == -1; cur++, i++)
	{
		if (*cur == relpath)
			result = i;
	}
	return result;
}


sLONG EntityModel::AddRelationPath(EntityRelation* relpath)
{
	sLONG result = (sLONG)fRelationPaths.size();
	try
	{
		fRelationPaths.push_back(relpath);
	}
	catch (...)
	{
		result = -1;
	}
	return result;
}


VError EntityModel::ResolveRelatedEntities(SubPathRefSet& already, EntityModelCatalog* catalog, ModelErrorReporter* errorReporter, BaseTaskInfo* context)
{
	VError err = VE_OK;

	for (EntityAttributeCollection::iterator cur = fAttributes.begin(), end = fAttributes.end(); cur != end && err == VE_OK; cur++)
	{
		SubPathRefSet already2;
		err = (*cur)->ResolveRelatedEntities(already2, catalog, errorReporter, context);
	}

	for (EntityMethodMap::iterator cur = fMethodsByName.begin(), end = fMethodsByName.end(); cur != end && err == VE_OK; cur++)
	{
		err = cur->second->ResolveType(catalog, errorReporter);
	}

	return err;
}


/*
VError EntityModel::ResolveRelatedPath(EntityModelCatalog* catalog)
{
	VError err = VE_OK;

	for (EntityAttributeCollection::iterator cur = fAttributes.begin(), end = fAttributes.end(); cur != end && err == VE_OK; cur++)
	{
		err = (*cur)->ResolveRelatedPath(catalog);
	}

	return err;
}
*/

VError EntityModel::ResolveQueryStatements(EntityModelCatalog* catalog, ModelErrorReporter* errorReporter, BaseTaskInfo* context)
{
	VError err = VE_OK;

	VString orderbystring;

	if (fRestrictingQuery == nil && !fCumulatedRestrictingQueryString.IsEmpty())
	{

		if (fBaseEm != nil)
		{
			err = fBaseEm->ResolveQueryStatements(catalog, errorReporter, context);
			if (fBaseEm->fRestrictingQuery != nil && err == VE_OK)
			{
				fRestrictingQuery = new SearchTab(this);
				fRestrictingQuery->From(*(fBaseEm->fRestrictingQuery));
			}
		}


		if (err == VE_OK)
		{
			if (fRestrictingQuery == nil)
			{
				fRestrictingQuery = new SearchTab(this);
				fRestrictingQuery->AllowJSCode(true);
				if (errorReporter == nil)
				{
					err = fRestrictingQuery->BuildFromString(fRestrictingQueryString, orderbystring, context, this);
				}
				else
				{
					ModelErrorContextInstaller errs(errorReporter);
					fRestrictingQuery->BuildFromString(fRestrictingQueryString, orderbystring, context, this);
				}
			}
			else
			{
				SearchTab newquery(this);
				newquery.AllowJSCode(true);
				if (errorReporter == nil)
				{
					err = newquery.BuildFromString(fRestrictingQueryString, orderbystring, context, this);
					if (err == VE_OK)
						err = fRestrictingQuery->Add(newquery);
				}
				else
				{
					ModelErrorContextInstaller errs(errorReporter);
					VError err2 = newquery.BuildFromString(fRestrictingQueryString, orderbystring, context, this);
					if (err2 == VE_OK)
						fRestrictingQuery->Add(newquery);
				}
			}

			if (err == VE_OK)
			{
				if (!orderbystring.IsEmpty())
				{

				}
			}
		}
	}

	if (!fAlreadyResolvedComputedAtts && err == VE_OK)
	{
		fAlreadyResolvedComputedAtts = true;

		for (EntityAttributeCollection::iterator cur = fAttributes.begin(), end = fAttributes.end(); cur != end && err == VE_OK; cur++)
		{
			err = (*cur)->ResolveQueryStatements(catalog, errorReporter, context);
		}
	}

	return err;
}


CDB4DEntityCollection* EntityModel::Query( const XBOX::VString& inQuery, CDB4DBaseContext* inContext, VErrorDB4D& err, const XBOX::VValueSingle* param1, const XBOX::VValueSingle* param2, const XBOX::VValueSingle* param3)
{
	return query(inQuery, ConvertContext(inContext), err, param1, param2, param3);
}

CDB4DEntityRecord* EntityModel::Find(const XBOX::VString& inQuery, CDB4DBaseContext* inContext, VErrorDB4D& err, const XBOX::VValueSingle* param1, const XBOX::VValueSingle* param2, const XBOX::VValueSingle* param3)
{
	return find(inQuery, ConvertContext(inContext), err, param1, param2, param3);
}




EntityCollection* EntityModel::query( const VString& inQuery, BaseTaskInfo* context, VErrorDB4D& err, const VValueSingle* param1, const VValueSingle* param2, const VValueSingle* param3)
{
	EntityCollection* result = nil;
	if (okperm(context, fPerms[DB4D_EM_Read_Perm]) || okperm(context, fPerms[DB4D_EM_Update_Perm]) || okperm(context, fPerms[DB4D_EM_Delete_Perm]))
	{
		SearchTab laquery(this);
		
		VString orderby;
		err = laquery.BuildFromString(inQuery, orderby, context, this, false);
		if (err == VE_OK)
		{
			vector<VString> params;
			QueryParamElementVector values;
			laquery.GetParams(params, values);
			QueryParamElementVector::iterator curvalue = values.begin();
			for (vector<VString>::iterator cur = params.begin(), end = params.end(); cur != end; ++cur, ++curvalue)
			{
				VString *s = &(*cur);

				if (s->GetLength() == 2 && (*s)[0] == 'p')
				{
					if ((*s)[1] == '1')
					{
						if (param1 != nil)
						{
							*curvalue = QueryParamElement(param1->Clone());
						}
					}
					else if ((*s)[1] == '2')
					{
						if (param2 != nil)
						{
							*curvalue = QueryParamElement(param2->Clone());
						}
					}
					else if ((*s)[1] == '3')
					{
						if (param3 != nil)
						{
							*curvalue = QueryParamElement(param3->Clone());
						}
					}	
				}
			}
			laquery.SetParams(params, values);
			result = executeQuery(&laquery, context, nil, nil, DB4D_Do_Not_Lock, 0, nil, &err);
		}
	}
	else
	{
		context->SetPermError();
		err = ThrowError(VE_DB4D_NO_PERM_TO_READ);
	}

	return result;
}



EntityRecord* EntityModel::find(const VString& inQuery, BaseTaskInfo* inContext, VErrorDB4D& err, const VValueSingle* param1, const VValueSingle* param2, const VValueSingle* param3)
{
	EntityRecord* result = nil;
	err = VE_OK;
	EntityCollection* sel = query(inQuery, inContext, err, param1, param2, param3);
	if (sel != nil && sel->GetLength(inContext) > 0)
	{
		result = sel->LoadEntity(0, inContext, err, DB4D_Do_Not_Lock);
	}
	QuickReleaseRefCountable(sel);
	return result;
}


EntityRecord* EntityModel::findEntityWithAttribute(const EntityAttribute* inAtt, const VValueSingle* value, BaseTaskInfo* context, VError& err, DB4D_Way_of_Locking HowToLock)
{
	EntityRecord* result = nil;
	err = VE_OK;
	SearchTab xquery(this);
	VCompareOptions options;
	options.SetDiacritical(false);
	ObjectPath objpath;
	xquery.AddSearchLineEmSimple(inAtt, objpath, DB4D_Equal, value, options, 0, false);
	EntityCollection* col = executeQuery(&xquery, context, nil, nil, DB4D_Do_Not_Lock, 0, nil, &err);
	if (col != nil)
	{
		if (col->GetLength(context) > 0)
			result = col->LoadEntity(0, context, err, DB4D_Do_Not_Lock);
		col->Release();
	}

	return result;
}


bool EntityModel::AddRestrictionToQuery(SearchTab& query, BaseTaskInfo* context, VError& err)
{
	err = VE_OK;
	bool ok = false;
	if (fWithRestriction)
	{
		ok = true;
		EntityCollection* sel = BuildRestrictingSelection(context, err);
		if (sel != nil)
		{
			query.AddSearchLineEmSel(sel);
			sel->Release();
		}
	}
	return ok;
}


EntityCollection* EntityModel::BuildRestrictingSelection(BaseTaskInfo* context, VError& err)
{
	EntityCollection* result = nil;
	err = VE_OK;
	if (fWithRestriction)
	{
		if (! context->AlreadyCalledRestrict(this))
		{
			err = CallDBEvent(dbev_restrict, context, &result);
			context->ReleaseCalledRestrict(this);
		}
		else
		{
			if (fBaseEm != nil)
				result = fBaseEm->SelectAllEntities(context, &err);
			else
				result = SelectAllEntities(context, &err, DB4D_Do_Not_Lock, nil, false);
		}
	}

	return result;
}


EntityCollection* EntityModel::NewCollection(const VectorOfVString& primKeys, VError& err, BaseTaskInfo* context) const
{
	err = VE_OK;
	EntityCollection* result = NewCollection(false, false);
	if (result != nil)
	{
		for (VectorOfVString::const_iterator cur = primKeys.begin(), end = primKeys.end(); cur != end && err == VE_OK; ++cur)
		{
			EntityRecord* erec = ((EntityModel*)this)->findEntityWithPrimKey(*cur, context, err, DB4D_Do_Not_Lock);
			if (erec != nil)
			{
				result->AddEntity(erec);
			}
			QuickReleaseRefCountable(erec);
		}
	}
	return result;
}



EntityCollection* EntityModel::FromArray(VJSArray& arr, BaseTaskInfo* context, VError& err, VDB4DProgressIndicator* InProgress)
{
	err = VE_OK;
	EntityCollection* sel = NewCollection(false, false);

	sLONG nbelem = (sLONG)arr.GetLength();
	VJSValue jsval(arr.GetContext());
	EntityModel* model = this;

	for (sLONG i = 0; i < nbelem && err == VE_OK; i++)
	{
		jsval = arr.GetValueAt(i);
		if (jsval.IsObject())
		{
			VJSObject objrec = jsval.GetObject();
			sLONG stamp = 0;
			
			EntityRecord* rec = nil;
			jsval = objrec.GetProperty("__KEY");
			if (jsval.IsObject())
			{
				VJSObject objkey = jsval.GetObject();

				jsval = objkey.GetProperty("__STAMP");
				if (jsval.IsNumber())
					jsval.GetLong(&stamp);

				rec = model->findEntityWithPrimKey(objkey, context, err, DB4D_Do_Not_Lock);
				if (err == VE_OK)
				{
					if (rec == nil)
					{
						rec = model->newEntity(context);
						if (rec != nil)
						{
							rec->setPrimKey(objkey);
						}
					}
				}
			}
			else if (jsval.IsString())
			{
				VString keyString;
				jsval.GetString(keyString);
				jsval = objrec.GetProperty("__STAMP");
				if (jsval.IsNumber())
					jsval.GetLong(&stamp);
				rec = model->findEntityWithPrimKey(keyString, context,err, DB4D_Do_Not_Lock);
				if (err == VE_OK)
				{
					if (rec == nil)
					{
						rec = model->newEntity(context);
						/*
						if (rec != nil)
						{
							rec->setPrimKey(objkey);
						}
						*/
					}
				}

			}
			else
			{
				rec = model->newEntity(context);
			}

			if (rec != nil)
			{
				err = rec->convertFromJSObj(objrec);
			}
			if (err == VE_OK)
			{
				err = rec->Save(stamp);
				if (err == VE_OK)
				{
					err = sel->AddEntity(rec);
				}
			}

			QuickReleaseRefCountable(rec);
		}

	}


	return sel;
}



CDB4DEntityCollection* EntityModel::NewSelection(bool ordered, bool safeRef) const
{
	return NewCollection(ordered, safeRef);
}

CDB4DEntityCollection* EntityModel::SelectAllEntities(CDB4DBaseContextPtr inContext, VErrorDB4D* outErr, DB4D_Way_of_Locking HowToLock, CDB4DEntityCollection* outLockSet)
{
	EntityCollection* locked = nil;
	if (outLockSet != nil)
		locked = dynamic_cast<EntityCollection*>(outLockSet);
	return SelectAllEntities(ConvertContext(inContext), outErr, HowToLock, locked);
}


VError EntityModel::getQParams(VJSParms_callStaticFunction& ioParms, sLONG firstparam, QueryParamElementVector& outParams, SearchTab* inQuery)
{
	VError err = VE_OK;
	SearchTab* query = inQuery;
	for (sLONG i = firstparam, nb = (sLONG)ioParms.CountParams(); i <= nb; i++)
	{
		if (ioParms.IsObjectParam(i))
		{
			VJSObject obj(ioParms.GetParamValue(i).GetObject());
			if (obj.IsArray())
			{
				VJSArray arr(obj, false);
				outParams.push_back(QueryParamElement(arr));
			}
			else
			{
				if (obj.HasProperty("allowJavascript"))
				{
					bool allowjs = obj.GetPropertyAsBool("allowJavascript", nil, nil);
					query->AllowJSCode(allowjs);
				}
				if (obj.HasProperty("queryPlan"))
				{
					bool queryplan = obj.GetPropertyAsBool("queryPlan", nil, nil);
				}
				if (obj.HasProperty("queryPath"))
				{
					bool querypath = obj.GetPropertyAsBool("queryPath", nil, nil);	
				}

			}
		}
		else if (!ioParms.IsNullOrUndefinedParam(i))
		{
			VJSValue jsval(ioParms.GetParamValue(i));
			VValueSingle* cv = jsval.CreateVValue();
			if (cv == nil)
			{
				cv = new VString();
				cv->SetNull(true);
			}
			if (cv->GetValueKind() == VK_TIME)
			{
				VValueSingle* cv2 = jsval.CreateVValue(true);
				outParams.push_back(QueryParamElement(cv, cv2));
			}
			else
				outParams.push_back(QueryParamElement(cv));
		}
	}
	return err;
}


VError EntityModel::FillQueryWithParams(SearchTab* query, VJSParms_callStaticFunction& ioParms, sLONG firstparam)
{
	vector<VString> ParamNames;
	QueryParamElementVector ParamValues;

	VError err = query->GetParams(ParamNames, ParamValues);
	if (err == VE_OK)
	{
		QueryParamElementVector::iterator curvalue = ParamValues.begin();
		for (vector<VString>::iterator curname = ParamNames.begin(), endname = ParamNames.end(); curname != endname; curname++, curvalue++)
		{
			const VString& s = *curname;
			bool isAParam = false;
			if (s.GetLength() >= 1)
			{
				UniChar c = s[0];
				if (c >= '1' && c <= '9')
				{
					isAParam = true;
					sLONG paramnum = s.GetLong() - 1 + firstparam;
					if (ioParms.CountParams() >= paramnum)
					{
						VJSValue jsval(ioParms.GetParamValue(paramnum));
						if (jsval.IsArray())
						{
							VJSArray jarr(jsval, false);
							curvalue->Dispose();
							*curvalue = QueryParamElement(jarr);
						}
						else
						{
							VValueSingle* cv = jsval.CreateVValue();
							if (cv != nil)
							{
								curvalue->Dispose();
								if (cv->GetValueKind() == VK_TIME)
								{
									VValueSingle* cv2 = jsval.CreateVValue(true);
									*curvalue = QueryParamElement(cv, cv2);
								}
								else
									*curvalue = QueryParamElement(cv);
							}
						}
					}
				}
				else if (c == '$')
				{
					isAParam = true;
				}
			}
			if (!isAParam)
			{
				curvalue->Dispose();
				VValueSingle* cv;
				ioParms.EvaluateScript(*curname, &cv);
				*curvalue = QueryParamElement(cv);
			}
		}
		err = query->SetParams(ParamNames, ParamValues);
	}

	return err;
}


VError EntityModel::FillQueryWithParams(SearchTab* query, VJSONArray* inArray)
{
	vector<VString> ParamNames;
	QueryParamElementVector ParamValues;

	sLONG nbelem = (sLONG)inArray->GetCount();

	VError err = query->GetParams(ParamNames, ParamValues);
	if (err == VE_OK)
	{
		QueryParamElementVector::iterator curvalue = ParamValues.begin();
		for (vector<VString>::iterator curname = ParamNames.begin(), endname = ParamNames.end(); curname != endname; curname++, curvalue++)
		{
			const VString& s = *curname;
			bool isAParam = false;
			if (s.GetLength() >= 1)
			{
				UniChar c = s[0];
				if (c >= '1' && c <= '9')
				{
					isAParam = true;
					sLONG paramnum = s.GetLong() - 1;
					if (paramnum < nbelem)
					{
						VJSONValue jsval = (*inArray)[paramnum];
						if (jsval.IsArray())
						{
							VJSONArray* jarr = jsval.GetArray();
							curvalue->Dispose();
							*curvalue = QueryParamElement(jarr);
						}
						else
						{
							VValueSingle* cv = jsval.CreateVValue();
							if (cv != nil)
							{
								curvalue->Dispose();
								*curvalue = QueryParamElement(cv);
							}
						}
					}
				}
				else if (c == '$')
				{
					isAParam = true;
				}
			}
		}
		err = query->SetParams(ParamNames, ParamValues);
	}

	return err;
}

CDB4DEntityCollection* EntityModel::ExecuteQuery( CDB4DQuery *inQuery, CDB4DBaseContextPtr inContext, CDB4DEntityCollection* Filter, 
											VDB4DProgressIndicator* InProgress, DB4D_Way_of_Locking HowToLock, 
											sLONG limit, CDB4DEntityCollection* outLockSet, VErrorDB4D *outErr)
{
	EntityCollection* lockedset = nil;
	EntityCollection* filter = nil;
	if (outLockSet != nil)
	{
		lockedset = dynamic_cast<EntityCollection*>(outLockSet);
	}
	if (Filter != nil)
	{
		filter = dynamic_cast<EntityCollection*>(Filter);
	}

	EntityCollection* result = executeQuery(dynamic_cast<VDB4DQuery*>(inQuery)->GetSearchTab(), ConvertContext(inContext), filter, InProgress, HowToLock, limit, lockedset, outErr);
	return result;
}


/*
map<Table*, EntityModel*> EntityModel::sEMbyTable;
VCriticalSection EntityModel::sEMbyTableMutex;
*/

#if BuildEmFromTable


EntityModel* EntityModel::BuildLocalEntityModel(Table* from, LocalEntityModelCatalog* catalog)
{
	xbox_assert(from != nil);
	VString tablename;
	from->GetName(tablename);
	EntityModel* xem = catalog->RetainEntity(tablename);
	if (xem == nil)
	{
		LocalEntityModel* em = new LocalEntityModel(catalog, from);
		if (em != nil)
		{
#if AllowDefaultEMBasedOnTables
			em->SetName(kEntityTablePrefixString + tablename);
#else
			em->SetName(tablename);
			em->SetCollectionName(tablename+"Collection");
#endif
			for (sLONG i = 1, nb = from->GetNbCrit(); i <= nb; i++)
			{
				Field* cri = from->RetainField(i);
				if (cri != nil && !cri->GetHideInRest())
				{
					VString fieldname;
					cri->GetName(fieldname);
					EntityAttribute* att = new EntityAttribute(em);
					att->SetKind(eattr_storage);
					att->SetFieldPos(i);
					att->SetName(fieldname);
					att->SetOverWrite(true);
					att->SetScalarType(cri->GetTyp());
					if (cri->GetTyp() == VK_TIME)
					{
						att->SetSimpleDate(true);
					}

					em->fAttributes.push_back(att);
					em->fAttributesByName[att->GetName()] = att;
					att->SetPosInOwner((sLONG)em->fAttributes.size());

					cri->Release();
				}
			}

			catalog->AddOneEntityModel(em, true);
/*
			DepRelationArrayIncluded* relsNto1 = from->GetRelNto1Deps();
			for (DepRelationArrayIncluded::Iterator cur = relsNto1->First(), end = relsNto1->End(); cur != end; cur++)
			{
				Relation* rel = *cur;
				VString relname = rel->GetName();
				if (!relname.IsEmpty())
				{
					Field* source = rel->GetSource();
					Field* dest = rel->GetDest();
					if (source != nil && dest != nil && !source->GetHideInRest() && !dest->GetHideInRest() 
						&& !source->GetOwner()->GetHideInRest() && !dest->GetOwner()->GetHideInRest() )
					{
						EntityAttribute* att = new EntityAttribute(em);
						att->SetKind(eattr_relation_Nto1);
						att->SetName(relname);
						att->SetOverWrite(true);
						att->SetRelation(rel, erel_Nto1, catalog);

						em->fAttributes.push_back(att);
						em->fAttributesByName[att->GetName()] = att;
						att->SetPosInOwner((sLONG)em->fAttributes.size());
					}
				}
			}

			//if (false)
			{
				DepRelationArrayIncluded* rels1toN = from->GetRel1toNDeps();
				for (DepRelationArrayIncluded::Iterator cur = rels1toN->First(), end = rels1toN->End(); cur != end; cur++)
				{
					Relation* rel = *cur;
					VString relname = rel->GetOppositeName();
					if (!relname.IsEmpty())
					{
						Field* source = rel->GetSource();
						Field* dest = rel->GetDest();
						if (source != nil && dest != nil && !source->GetHideInRest() && !dest->GetHideInRest() 
							&& !source->GetOwner()->GetHideInRest() && !dest->GetOwner()->GetHideInRest() )
						{
							EntityAttribute* att = new EntityAttribute(em);
							att->SetKind(eattr_relation_1toN);
							att->SetName(relname);
							att->SetOverWrite(true);
							att->SetRelation(rel, erel_1toN, catalog);

							em->fAttributes.push_back(att);
							em->fAttributesByName[att->GetName()] = att;
							att->SetPosInOwner((sLONG)em->fAttributes.size());
						}
					}
				}
			}
			*/

			if (from->HasPrimKey())
			{
				NumFieldArray primkey;
				from->CopyPrimaryKey(primkey);
				for (NumFieldArray::Iterator cur = primkey.First(), end = primkey.End(); cur != end; cur++)
				{
					sLONG numfield = *cur;
					Field* cri = from->RetainField(numfield);
					if (cri != nil)
					{
						VString criname;
						cri->GetName(criname);
						EntityAttribute* att = em->getAttribute(criname);
						if (att != nil)
						{
							em->fOwnedPrimaryAtts.push_back(IdentifyingAttribute(att));
							att->SetPartOfPrimKey();
						}
						cri->Release();
					}
				}
				if (!em->fOwnedPrimaryAtts.empty())
					em->fPrimaryAtts = em->fOwnedPrimaryAtts;
			}
		}

		xem = em;

	}

	return xem;
}

#endif

/*
void EntityModel::ClearCacheTableEM()
{
	
	VTaskLock lock(&sEMbyTableMutex);
	for (map<Table*, EntityModel*>::iterator cur = sEMbyTable.begin(), end = sEMbyTable.end(); cur != end; cur++)
		cur->second->Release();
	sEMbyTable.clear();
	
}
*/




VError EntityModel::SetPermission(DB4D_EM_Perm inPerm, const VUUID& inGroupID, bool forced)
{
	VError err = VE_OK;
	if (inPerm >= DB4D_EM_Read_Perm && inPerm <= DB4D_EM_Promote_Perm)
	{
		fPerms[inPerm] = inGroupID;
		fForced[inPerm] = forced ? 1 : 0;
	}
	else
		err = ThrowError(VE_DB4D_WRONG_PERM_REF);

	return err;
}

VError EntityModel::GetPermission(DB4D_EM_Perm inPerm, VUUID& outGroupID, bool& forced) const
{
	VError err = VE_OK;
	if (inPerm >= DB4D_EM_Read_Perm && inPerm <= DB4D_EM_Promote_Perm)
	{
		outGroupID = fPerms[inPerm];
		forced = fForced[inPerm] == 1 ? true : false;
	}
	else
		err = ThrowError(VE_DB4D_WRONG_PERM_REF);

	return err;
}


bool EntityModel::PermissionMatch(DB4D_EM_Perm inPerm, CUAGSession* inSession, CUAGThreadPrivilege* inThreadPrivilege) const
{
	return permissionMatch(inPerm, inSession, inThreadPrivilege);
}


bool EntityModel::permissionMatch(DB4D_EM_Perm inPerm, CUAGSession* inSession, CUAGThreadPrivilege* inThreadPrivilege) const
{
	if (inPerm >= DB4D_EM_Read_Perm && inPerm <= DB4D_EM_Promote_Perm)
	{
		const VUUID* id = &(fPerms[inPerm]);
		if (id->IsNull())
			return true;
		else
		{
			if (inSession->BelongsTo(*id, inThreadPrivilege))
				return true;
			else
			{
				if (inPerm == DB4D_EM_Describe_Perm)
					return permissionMatch(DB4D_EM_Read_Perm, inSession, inThreadPrivilege);
				else if (inPerm == DB4D_EM_Read_Perm)
					return permissionMatch(DB4D_EM_Update_Perm, inSession, inThreadPrivilege) || permissionMatch(DB4D_EM_Delete_Perm, inSession, inThreadPrivilege);
				else
					return false;
			}
		}
	}
	else
		return false;
}


bool EntityModel::permissionMatch(DB4D_EM_Perm inPerm, BaseTaskInfo* context) const
{
	if (context != nil)
	{
		return permissionMatch(inPerm, context->GetCurrentUserSession(), context->GetThreadPrivileges());
	}
	else
		return false;
}


VError EntityModel::callMethod(const VString& inMethodName, const VectorOfVString& params, VJSValue& result, BaseTaskInfo* inContext, EntityCollection* inSel, EntityRecord* inRec)
{
	VString jsonparams = L"[";
	bool first = true;
	for (VectorOfVString::const_iterator cur = params.begin(), end = params.end(); cur != end; cur++)
	{
		if (first)
			first = false;
		else
			jsonparams += L",";
		VString xjs;
		cur->GetJSONString(xjs, JSON_WithQuotesIfNecessary);
		jsonparams += xjs;
	}
	jsonparams += L"]";
	return callMethod(inMethodName, jsonparams, result, inContext, inSel, inRec);
}


VError EntityModel::callMethod(const VString& inMethodName, const VString& jsonparams, VJSValue& result, BaseTaskInfo* inContext, EntityCollection* inSel, EntityRecord* inRec)
{
	VError err = VE_OK;
	EntityMethod* meth = getMethod(inMethodName);
	if (meth != nil)
	{
		BaseTaskInfo* context = inContext;
		if (context != nil && context->GetJSContext() != nil)
		{
			VJSContext jscontext(context->GetJSContext());
			VUUID idexec, idpromote;
			meth->GetPermission(DB4D_EM_Execute_Perm, idexec);
			meth->GetPermission(DB4D_EM_Promote_Perm, idpromote);
			if (okperm(context, idexec))
			{
				VJSJSON json(jscontext);
				XBOX::VJSException excep;
				VJSValue params(jscontext);
				json.Parse(params,jsonparams, &excep);
				if (excep.IsEmpty())
				{
					vector<VJSValue> paramsValues;
					if (!jsonparams.IsEmpty())
					{
						if (params.IsInstanceOf( CVSTR( "Array")))
						{
							VJSObject objarray = params.GetObject();
							VJSArray xarray(objarray);
							for (sLONG i = 0, nb = (sLONG)xarray.GetLength(); i < nb; i++)
							{
								paramsValues.push_back(xarray.GetValueAt(i));
							}
						}
						else
						{
							paramsValues.push_back(params);
						}
					}

					if (inSel != nil)
						err = meth->Execute(inSel, &paramsValues, context, result);
					else if (inRec != nil)
						err = meth->Execute(inRec, &paramsValues, context, result);
					else err = meth->Execute(&paramsValues, context, result);
				}

				if (!excep.IsEmpty())
				{
					ThrowJSExceptionAsError(jscontext, excep);
					err = ThrowError(VE_DB4D_JS_ERR);
				}

				context->ClearPermError();
			}
			else
			{
				err = meth->ThrowError(VE_DB4D_NO_PERM_TO_EXECUTE);
			}
		}
		
	}
	else
		err = ThrowError(VE_DB4D_ENTITY_METHOD_NAME_UNKNOWN, &inMethodName);
	return err;
}


VError EntityModel::callMethod(const VString& inMethodName, const VValueBag& bagparams, VJSValue& result, BaseTaskInfo* inContext, EntityCollection* inSel, EntityRecord* inRec)
{
	VString jsonstring;
	bagparams.GetJSONString(jsonstring);
	return callMethod(inMethodName, jsonstring, result, inContext);
}


VError EntityModel::CallDBEvent(DBEventKind kind, EntityRecord* inRec, BaseTaskInfo* context) const
{
	VError err = VE_OK;
	if (kind != dbev_none)
	{
		if (fBaseEm != nil && kind != dbev_restrict)
		{
			err = fBaseEm->CallDBEvent(kind, inRec, context);
		}
		err = fDBEvents.CallEvent(kind, inRec, context, nil, nil, nil);
		/*
		const DBEvent& ev = fEvents[kind];
		if (ev.IsValid())
		{
			err = ev.Call(inRec, context, nil, nil);
		}
		*/
	}
	return err;
}


VError EntityModel::CallDBEvent(DBEventKind kind, BaseTaskInfo* context, EntityCollection* *outSel) const
{
	VError err = VE_OK;
	if (kind != dbev_none)
	{
		if (fBaseEm != nil && kind != dbev_restrict)
		{
			err = fBaseEm->CallDBEvent(kind, context, outSel);
		}
		err = fDBEvents.CallEvent(kind, nil, context, nil, this, outSel);
		/*
		const DBEvent& ev = fEvents[kind];
		if (ev.IsValid())
		{
			err = ev.Call(nil, context, nil, this, outSel);
		}
		*/
	}
	return err;
}


VError EntityModel::CallAttributesDBEvent(DBEventKind kind, EntityRecord* inRec, BaseTaskInfo* context) const
{
	VError err = VE_OK;
	if (kind != dbev_none)
	{
		for (EntityAttributeCollection::const_iterator cur = fAttributes.begin(), end = fAttributes.end(); cur != end && err == VE_OK; ++cur)
		{
			const EntityAttribute* att = *cur;
			if (att != nil)
			{
				err = att->CallDBEvent(kind, inRec, context, false);
			}
		}
	}
	return err;
}


bool EntityModel::HasEvent(DBEventKind kind) const
{
	if (fBaseEm != nil)
		return fBaseEm->HasEvent(kind) || fDBEvents.HasEvent(kind);
	else
		return fDBEvents.HasEvent(kind);
}



void EntityModel::ResolvePermissionsInheritance(EntityModelCatalog* catalog)
{
	for (DB4D_EM_Perm perm = DB4D_EM_Read_Perm; perm <= DB4D_EM_Promote_Perm; perm = (DB4D_EM_Perm)(perm+1))
	{
		VUUID xid;
		bool forced = false;
		catalog->GetPermission(perm, xid, forced);
		if ((fPerms[perm].IsNull() || forced) && !xid.IsNull())
		{
			fPerms[perm] = xid;
			fForced[perm] = forced;
		}
	}

	for (EntityMethodMap::iterator cur = fMethodsByName.begin(), end = fMethodsByName.end(); cur != end; ++cur)
	{
		EntityMethod* meth = cur->second;
		if (meth != nil)
			meth->ResolvePermissionsInheritance();
	}

	for (EntityAttributeCollection::iterator cur = fAttributes.begin(), end = fAttributes.end(); cur != end; ++cur)
	{
		EntityAttribute* att = *cur;
		if (att != nil)
			att->ResolvePermissionsInheritance();
	}


}


VError EntityModel::BuildQueryString(SearchTab* querysearch, BaseTaskInfo* context, VString outQuery)
{
	return querysearch->BuildQueryString(context, outQuery);
}


VFileSystemNamespace* EntityModel::GetFileSystemNamespace()
{
	VFileSystemNamespace* result = nil;
	Base4D* bd = fCatalog->GetAssocBase();
	if (bd != nil)
	{
		result = bd->GetFileSystemNamespace();
	}
	return result;
}




// ----------------------------------------------------------------------------------------------------


SubEntityCache::~SubEntityCache()
{
	QuickReleaseRefCountable(fErec);
	QuickReleaseRefCountable(fSel);
}


void SubEntityCache::Clear()
{
	ReleaseRefCountable(&fErec);
	ReleaseRefCountable(&fSel);
	fAlreadyActivated = false;
}



// ----------------------------------------------------------------------------------------------------


#if debuglr
sLONG EntityAttributeValue::Retain(const char* DebugInfo) const
{
	return IRefCountable::Retain(DebugInfo);
}

sLONG EntityAttributeValue::Release(const char* DebugInfo) const
{
	return IRefCountable::Release(DebugInfo);
}
#endif


EntityAttributeValue::EntityAttributeValue(EntityRecord* owner, const EntityAttribute* attribute, EntityAttributeValueKind kind, VValueSingle* inVal, bool isowned ):fJSObject((VJSContext*)nil)
{
	_Init(kind);
	fValue = inVal;
	fIsValueOwned = isowned;
	fOwner = owner;
	fAttribute = attribute;
	if (inVal != nil && inVal->GetValueKind() == VK_TIME)
	{
		VTime* dt = (VTime*)inVal;
		dt->NullIfZero();
	}
}


EntityAttributeValue::EntityAttributeValue(EntityRecord* owner, const EntityAttribute* attribute, EntityAttributeValueKind kind, EntityRecord* erec, EntityModel* inSubModel):fJSObject((VJSContext*)nil)
{
	_Init(kind);
	fSubEntity = RetainRefCountable(erec);
	fSubModel = inSubModel;
	fOwner = owner;
	fAttribute = attribute;
	if (fSubEntity != nil)
		fSubModificationStamp = fSubEntity->GetModificationStamp();
}


EntityAttributeValue::EntityAttributeValue(EntityRecord* owner, const EntityAttribute* attribute, EntityAttributeValueKind kind, EntityCollection* sel, EntityModel* inSubModel):fJSObject((VJSContext*)nil)
{
	_Init(kind);
	fSel = RetainRefCountable(sel);
	fSubModel = inSubModel;
	fOwner = owner;
	fAttribute = attribute;
}


EntityAttributeValue::~EntityAttributeValue()
{
	if (fJSObjectIsValid)
	{
		fJSObject.Unprotect();
	}
	/*
	if (fExtraData != nil)
	{
#if !VERSION_LINUX   // Postponed Linux Implementation !
		db4dJSPictContainer* xpic = (db4dJSPictContainer*)fExtraData;
		xpic->ClearEntityRef();
		//fExtraData = nil;
#endif
	}
	*/
	if (fValue != nil && fIsValueOwned)
		delete fValue;
	QuickReleaseRefCountable(fSubEntity);
	QuickReleaseRefCountable(fSel);
	QuickReleaseRefCountable(fValueBag);
	QuickReleaseRefCountable(fOldSubEntity);
	if (fRelatedKey != nil)
		delete fRelatedKey;
}


EntityAttributeValue* EntityAttributeValue::Clone(EntityRecord* newOwner, bool fullClone)
{
	EntityAttributeValue* result = new EntityAttributeValue(newOwner, fAttribute, fType, nil, false);

	result->fJsonString = fJsonString;
	result->fValueBag = RetainRefCountable(fValueBag);

	if (fullClone)
		result->fOldSubEntity = nil;
	else
		result->fOldSubEntity = RetainRefCountable(fOldSubEntity);

	VValueSingle* cv = fValue;
	if (cv != nil)
		cv = cv->Clone();
	result->fValue = cv;
	result->fIsValueOwned = true;

	if (fRelatedKey != nil)
		result->fRelatedKey = fRelatedKey->Clone();

	if (fullClone)
	{
		VError suberr = VE_OK;
		if (fSubEntity == nil)
			result->fSubEntity = nil;
		else
			result->fSubEntity = fSubEntity->Clone(suberr);
		result->fSel = nil;
	}
	else
	{
		result->fSubEntity = RetainRefCountable(fSubEntity);
		result->fSel = RetainRefCountable(fSel);
	}
	result->fSubModel = fSubModel;
	return result;
}


VError EntityAttributeValue::setRelatedKey(const VectorOfVValue& key)
{
	fSubEntityCanTryToReload = true;

	if (fRelatedKey != nil)
		delete fRelatedKey;
	fRelatedKey = key[0]->Clone();

	if (fAttribute->HasEvent(dbev_set))
	{
		ReleaseRefCountable(&fSubEntity);
		getRelatedEntity();
	}
	else
	{
		SetRelatedEntity(nil, fOwner->getContext());
	}

	return VE_OK;
}


CDB4DEntityRecord* EntityAttributeValue::GetRelatedEntity() const
{
	return const_cast<EntityAttributeValue*>(this)->getRelatedEntity();
}

EntityRecord* EntityAttributeValue::getRelatedEntity()
{
	if (fSubEntity == nil)
	{
		StErrorContextInstaller erss(false);
		VError err;
		if (fRelatedKey != nil && fSubEntityCanTryToReload)
		{
			BaseTaskInfo* context = fOwner->getContext();
			fSubEntityCanTryToReload = false;
			VectorOfVValue key(false);
			key.push_back(fRelatedKey);
			EntityRecord* rec = fSubModel->findEntityWithPrimKey(key, context, err, DB4D_Do_Not_Lock);
			if (rec != nil)
			{
				SetRelatedEntity(rec, context);
				rec->Release();
			}
		}
	}
	return fSubEntity;
}



VError EntityAttributeValue::GetJsonString(VString& outJsonValue)
{
	VError err = VE_OK;

	

	return err;
}


VError EntityAttributeValue::GetJSObject(VJSObject& outJSObject)
{
	VError err = VE_OK;

	if (fJSObjectIsValid)
	{
		outJSObject = fJSObject;
	}
	else
	{
		if (fType == eav_composition)
		{
			if (fSel != nil)
			{
				VJSArray arr(outJSObject.GetContext());
				EntityAttributeSortedSelection atts(fOwner->GetOwner());
				outJSObject = fSel->ToJsArray(fOwner->getContext(), outJSObject.GetContext(), atts, nil, nil, false, false, 0, fSel->GetLength(fOwner->getContext()), err, nil);
				outJSObject = arr;
			}
			if (err == VE_OK)
			{
				fJSObject = outJSObject;
				fJSObject.Protect();
				fJSObjectIsValid = true;
			}
		}
		else
			err = fAttribute->ThrowError(VE_DB4D_WRONG_ATTRIBUTE_KIND);
	}
	return err;
}


VError EntityAttributeValue::SetJSObject(const VJSObject& inJSObject)
{
	if (fJSObjectIsValid)
	{
		fJSObject.Unprotect();
	}
	fJSObject = inJSObject;
	fJSObject.Protect();
	fJSObjectIsValid = true;
	return VE_OK;
}


/*
VError EntityAttributeValue::convertToJSObject(VJSObject& outObj)
{
	VError err = VE_OK;
	if (fType == eav_composition)
	{
		BaseTaskInfo* context = ConvertContext(fOwner->GetContext());
		VJSContext jscontext(context->GetJSContext());
		VJSArray result(jscontext);
		if (fSel != nil)
		{
			for (sLONG i=0, nb = fSel->GetQTfic(); i < nb && err == VE_OK; i++)
			{
				EntityRecord* erec = fSubModel->LoadEntityRecord(fSel->GetFic(i), err, DB4D_Do_Not_Lock, context, true);
				if (erec != nil)
				{
					VJSObject objrec(jscontext);
					err = erec->convertToJSObject(objrec);
					if (err == VE_OK)
					{
						result.PushValue(objrec);
					}
				}
			}
		}
		if (err == VE_OK)
			outObj = result;
	}
	else
		err = ThrowBaseError(VE_DB4D_WRONG_ATTRIBUTE_KIND);
	return err;
}
*/


VError EntityAttributeValue::ValidateRemove(BaseTaskInfo* context)
{
	VError err = VE_OK;
	err = fAttribute->CallDBEvent(dbev_validateremove, fOwner, context, true);
	return err;
}


VError EntityAttributeValue::Validate(BaseTaskInfo* context)
{
	VError err = VE_OK;
	switch(fType)
	{
		case eav_vvalue:
			{
				if (fValue != nil)
				{
					const VValueSingle* minval = nil;
					const VValueSingle* maxval = nil;
					if (fAttribute->getMinValue(minval))
					{
						if (fValue->IsNull() || fValue->CompareTo(*minval) == CR_SMALLER)
						{
							VString s;
							minval->GetString(s);
							err = fAttribute->ThrowError(VE_DB4D_ENTITY_VALUE_LESS_THAN_MIN,&s);
						}
					}
					if (fAttribute->getMaxValue(maxval))
					{
						if (fValue->IsNull() || fValue->CompareTo(*maxval) == CR_BIGGER)
						{
							VString s;
							maxval->GetString(s);
							err = fAttribute->ThrowError(VE_DB4D_ENTITY_VALUE_MORE_THAN_MAX,&s);
						}
					}
					const VRegexMatcher* pattern;
					if (fAttribute->getPatternReg(pattern))
					{
						VString s;
						fValue->GetString(s);
						if (fValue->IsNull() || !((VRegexMatcher*)pattern)->Find(s,1,false, &err))  // attention il faudra changer cela, les pattern ne sont pas thread safe
						{
							VString patternString;
							fAttribute->getPattern(patternString);
							err = fAttribute->ThrowError(VE_DB4D_ENTITY_VALUE_DOES_NOT_MATCH_PATTERN,&patternString);
						}
					}

					sLONG minlength,maxlength,fixedlength;
					if (fAttribute->getMinLength(minlength))
					{
						VString s;
						fValue->GetString(s);
						if (fValue->IsNull() || s.GetLength() < minlength)
						{
							VString s2;
							s2.FromLong(minlength);
							err = fAttribute->ThrowError(VE_DB4D_ENTITY_STRING_LESS_THAN_MIN,&s2);
						}
					}

					if (fAttribute->getMaxLength(maxlength))
					{
						VString s;
						fValue->GetString(s);
						if (s.GetLength() > maxlength)
						{
							VString s2;
							s2.FromLong(maxlength);
							err = fAttribute->ThrowError(VE_DB4D_ENTITY_STRING_GREATER_THAN_MAX,&s2);
						}
					}

					if (fAttribute->getFixedLength(fixedlength))
					{
						VString s;
						fValue->GetString(s);
						if (fValue->IsNull() || s.GetLength() != fixedlength)
						{
							VString s2;
							s2.FromLong(fixedlength);
							err = fAttribute->ThrowError(VE_DB4D_ENTITY_STRING_LENGTH_EQUAL,&s2);
						}
					}

				}
			}
			break;

		default:
			break;
	}

	if (err == VE_OK)
	{
		err = fAttribute->CallDBEvent(dbev_validate, fOwner, context, true);
	}
	return err;
}



VError EntityAttributeValue::Save(Transaction* &trans, BaseTaskInfo* context, bool refreshOnly)
{
	VError err = VE_OK;

	if (fCanBeModified)
	{
		switch(fType)
		{
			case eav_vvalue:
				{
					if (fAttribute->GetKind() == eattr_computedField || fStamp != 0)
					{
						err = fAttribute->CallDBEvent(refreshOnly ? dbev_clientrefresh : dbev_save, fOwner, context, true);
						fOwner->TouchAttribute(fAttribute);
					}
				}
				break;

			case eav_subentity:
				{
					if (fOldSubEntity != nil)
					{
						fOldSubEntity->Release();
						fOldSubEntity = nil;
					}

					if (fSubEntity != nil)
					{
						if (fSubEntity->GetStamp() != 0 && fSubEntity->IsNew() && !fSubEntity->AlreadySaving())
						{
							if (trans == nil && context != nil)
								trans = context->StartTransaction(err);
							sLONG modifstamp = fSubEntity->GetModificationStamp();
							if (modifstamp == 0 && fSubEntity->IsNew())
								modifstamp = 1;
							err = fSubEntity->Save(&trans, context, modifstamp, refreshOnly);
						}
					}

					if (fStamp != 0 && err == VE_OK)
					{
						err = fAttribute->CallDBEvent(refreshOnly ? dbev_clientrefresh : dbev_save, fOwner, context, true);
					}

					if (fStamp != 0 && err == VE_OK)
					{
						EntityRelation* rel = fAttribute->GetRelPath();
						if (rel != nil)
						{
							if (rel->IsEmpty())
								err = fAttribute->ThrowError(VE_DB4D_ENTITY_RELATION_IS_EMPTY);
							else if (!rel->IsSimple())
								err = fAttribute->ThrowError(VE_DB4D_ENTITY_RELATION_IS_EMPTY);
							else
							{
								err = fOwner->SetForeignKey(fAttribute, fSubEntity, fRelatedKey);
							}
						}
						else
							err = fAttribute->ThrowError(VE_DB4D_ENTITY_RELATION_IS_EMPTY);
					}

				}
				break;

			case eav_composition:
				{
					EntityCollection* sel = getRelatedSelection();
					EntityModel* relEm = getRelatedEntityModel();
					if (relEm != nil)
					{
						err = fAttribute->CallDBEvent(refreshOnly ? dbev_clientrefresh : dbev_save, fOwner, context, true);

						if (err == VE_OK && sel != nil && sel->GetLength(context) > 0)
						{
							if (trans == nil && context != nil)
								trans = context->StartTransaction(err);
							//err = sel->DeleteRecords(context, nil, nil, nil, nil, relEm);
						}
						if (err == VE_OK)
						{
							if (fJSObjectIsValid)
							{
								if (fJSObject.IsInstanceOf("Array"))
								{
									/*
									VJSArray arr(fJSObject, false);
									VJSValue jsval(fJSObject.GetContext());
									sLONG nbrec = arr.GetLength();
									EntityRelation* rel = fAttribute->GetRelPath();
									Field* destfield = rel->GetDestField();
									Field* sourcefield = rel->GetSourceField();
									VValueSingle* cv = fOwner->getFieldValue(sourcefield, err);

									for (sLONG i = 0; i < nbrec && err == VE_OK; i++)
									{
										jsval = arr.GetValueAt(i);
										if (jsval.IsObject())
										{
											VJSObject objrec(fJSObject.GetContext());
											jsval.GetObject(objrec);
											EntityRecord* rec = relEm->NewEntity(context, DB4D_Do_Not_Lock);
											if (rec != nil)
											{
												err = rec->convertFromJSObj(objrec);
												if (err == VE_OK)
												{
													if (trans == nil && context != nil)
														trans = context->StartTransaction(err);
													if (err == VE_OK && destfield != nil && cv != nil)
													{
														err = rec->setFieldValue(destfield, cv);
													}
													if (err == VE_OK)
														err = rec->Save(&trans, context, 1);
												}
											}
											QuickReleaseRefCountable(rec);
										}
									}
									*/
								}
							}
						}
					}
				}
				break;

			case eav_selOfSubentity:
				break;
		}
	}

	return err;
}
 

bool EntityAttributeValue::equalValue(const VValueSingle* inValue)
{
	if (fType == eav_vvalue && fValue != nil)
	{
		if (fValue->IsNull())
		{
			if (inValue->IsNull())
				return true;
			else
				return false;
		}
		else
		{
			if (inValue->IsNull())
				return false;
			else
			{
				if (inValue->GetValueKind() == fValue->GetValueKind())
				{
					return fValue->EqualToSameKind(inValue, true);
				}
				else
				{
					VValueSingle* cv = inValue->ConvertTo(fValue->GetValueKind());
					bool equal = fValue->EqualToSameKind(cv, true);
					delete cv;
					return equal;
				}
			}
		}
	}
	else
		return false;
}


bool EntityAttributeValue::equalRelatedEntity(EntityRecord* relatedEntity)
{
	if (relatedEntity == nil)
	{
		if (fSubEntity == nil)
			return true;
		else
			return false;
	}
	else
	{
		if (fSubEntity == nil)
			return false;
		else
		{
			VectorOfVValue prim1,prim2;
			fSubEntity->GetPrimKeyValue(prim1);
			relatedEntity->GetPrimKeyValue(prim2);

			if (prim1 == prim2)
				return true;
			else
				return false;
		}
	}
}

void EntityAttributeValue::Touch(BaseTaskInfo* context)
{
	fAttribute->CallDBEvent(dbev_set, fOwner, context, true);
	fStamp++;
	fOwner->Touch(context);
}


VError EntityAttributeValue::SetRelatedEntity(EntityRecord* relatedEntity, BaseTaskInfo* context)
{
	VError err = VE_OK;
	if (fAttribute->GetSubEntityModel() == nil)
	{
		VString attname = fAttribute->GetOwner()->GetName()+"."+fAttribute->GetName();
		fAttribute->GetOwner()->ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_IS_NOT_NAVIGATION, &attname);
	}
	else
	{
		if (relatedEntity != nil)
		{
			if (!relatedEntity->GetOwner()->isExtendedFrom(fAttribute->GetSubEntityModel()))
				err = fAttribute->ThrowError(VE_DB4D_RELATED_ENTITY_DOES_NOT_BELONG_TO_MODEL, &(fAttribute->GetSubEntityModel()->GetName()) );
		}

		if (err == VE_OK)
		{
			if (fSubEntity != relatedEntity)
			{
				if (fOldSubEntity == nil)
				{
					fOldSubEntity = fSubEntity;
				}
				else
					QuickReleaseRefCountable(fSubEntity);

				//fSubEntity = RetainRefCountable(relatedEntity);
				
				if (relatedEntity != nil)
					fSubEntity = relatedEntity->Clone(err);
				else
					fSubEntity = nil;
				

				if (fSubEntity != nil)
					fSubModificationStamp = fSubEntity->GetModificationStamp();
				if (fAttribute->GetPathID() >= 0)
					fOwner->ClearSubEntityCache(fAttribute->GetPathID());
				//# il faudrait ici faire un ClearSubEntityCache de tous les chemins qui utilisent celui ci
			}
			Touch(context);
		}
	}

	return err;
}




// -----------------------------------------------

#if debug_entityRecord
sLONG EntityRecord::sNbEntityRecordsInMem = 0;
#endif

#if debuglr
sLONG EntityRecord::Retain(const char* DebugInfo) const
{
	return IRefCountable::Retain(DebugInfo);
}

sLONG EntityRecord::Release(const char* DebugInfo) const
{
	return IRefCountable::Release(DebugInfo);
}
#endif



EntityRecord::~EntityRecord()
{
#if debug_entityRecord
	--sNbEntityRecordsInMem;
	DebugMsg("delete EntityRecord : "+ToString((uLONG8)this)+ " , nb in mem : "+ToString(sNbEntityRecordsInMem)+"\n");
#endif
	for (EntityAttributeValueCollection::iterator cur = fValues.begin(), end = fValues.end(); cur != end; cur++)
	{
		if (*cur != nil)
			(*cur)->Release();
	}
}


EntityRecord::EntityRecord(EntityModel* inModel, BaseTaskInfo* inContext)
{
#if debug_entityRecord
	++sNbEntityRecordsInMem;
	DebugMsg("create EntityRecord : "+ToString((uLONG8)this)+ " , nb in mem : "+ToString(sNbEntityRecordsInMem)+"\n");
#endif
	fContext = inContext;
	fModel = inModel;
	sLONG nbelem = fModel->CountAttributes();
	fValues.resize(nbelem, nil);

	sLONG nbpaths = fModel->CountRelationPaths();
	fSubEntitiesCache.resize(nbpaths);
	std::fill(&fAlreadyCallEvent[0], &fAlreadyCallEvent[dbev_lastEvent+1], 0);
	fAlreadySaving = false;
	fAlreadyDeleting = false;
}


VError EntityRecord::ThrowError( VError inErrCode, const VString* p1) const
{
	VString skey;
	const_cast<EntityRecord*>(this)->GetPrimKeyValue(skey, false);
	VErrorDB4D_OnEMRec *err = new VErrorDB4D_OnEMRec(inErrCode, noaction, fModel->GetCatalog(), fModel, skey);
	if (p1 != nil)
		err->GetBag()->SetString(Db4DError::Param1, *p1);
	VTask::GetCurrent()->PushRetainedError( err);

	return inErrCode;
}


SubEntityCache* EntityRecord::GetSubEntityCache(sLONG inPathID, VError& err, bool Nto1, EntityModel* subEntityModel, BaseTaskInfo* context)
{
	err = VE_OK;
	if (testAssert(inPathID >= 0 && inPathID < fSubEntitiesCache.size()))
	{
		SubEntityCache* cache = &fSubEntitiesCache[inPathID];
		if (!cache->AlreadyActivated())
		{
			err = fModel->ActivatePath(this, inPathID, *cache, Nto1, subEntityModel, context);
		}
		return cache;
	}
	else
		return nil;
}


bool EntityRecord::SubEntityCacheNeedsActivation(sLONG inPathID)
{
	bool res = false;
	if (testAssert(inPathID >= 0 && inPathID < fSubEntitiesCache.size()))
	{
		SubEntityCache* cache = &fSubEntitiesCache[inPathID];
		res = !cache->AlreadyActivated();
	}
	return res;
}


void EntityRecord::ClearSubEntityCache(sLONG inPathID)
{
	if (testAssert(inPathID >= 0 && inPathID < fSubEntitiesCache.size()))
	{
		EntityRelation* relPath = fModel->GetPath(inPathID);
		if (relPath != nil)
		{
			for (sLONG i = 0, nbpath = (sLONG)fSubEntitiesCache.size(); i < nbpath; ++i)
			{
				EntityRelation* relPath2 = fModel->GetPath(i);
				if (relPath->MatchesBeginingOf(relPath2))
				{
					SubEntityCache* cache = &fSubEntitiesCache[i];
					cache->Clear();
				}
			}
		}
	}
}


void EntityRecord::ClearAllSubEntityCaches()
{
	for (sLONG i = 0, nbpath = (sLONG)fSubEntitiesCache.size(); i < nbpath; ++i)
	{
		SubEntityCache* cache = &fSubEntitiesCache[i];
		cache->Clear();
	}
}


/*
FicheInMem* EntityRecord::GetRecordForAttribute(const EntityAttribute* inAttribute)
{
	FicheInMem* result = nil;
	EntityAttributeKind kind = inAttribute->GetKind();
	switch(kind)
	{
		case eattr_storage:
			result = fMainRec;
			break;

		case eattr_alias:
			{
				VError err = VE_OK;
				SubEntityCache* cache = GetSubEntityCache(inAttribute->GetPathID(), err, true, inAttribute->GetSubEntityModel(), ConvertContext(fContext));
				if (cache != nil)
					result = cache->GetRecord();
			}
			break;

		default:
			result = nil;
			//xbox_assert ?
			break;
	}
	return result;
}
*/


EntityRecord* EntityRecord::do_LoadRelatedEntity(const EntityAttribute* inAttribute, const EntityAttribute* relatedAttribute, BaseTaskInfo* context, VError& err, DB4D_Way_of_Locking HowToLock)
{
	err = VE_OK;
	EntityRecord* result = nil;

	if (inAttribute != nil && relatedAttribute != nil)
	{
		VectorOfVValue primkey;
		EntityAttributeValue* val = getAttributeValue(inAttribute, err, context, false);
		VValueSingle* cv = nil;
		if (val != nil)
			cv = val->getVValue();
		if (cv != nil && !cv->IsNull())
		{
			primkey.push_back(cv);
			result = relatedAttribute->GetModel()->findEntityWithPrimKey(primkey, context, err, HowToLock);
		}
	}
	else
		err = ThrowError(VE_DB4D_WRONG_ATTRIBUTE_KIND);

	return result;
}


VError EntityRecord::Reload()
{
	VError err = do_Reload();
	if (err == VE_OK)
	{
		for (EntityAttributeValueCollection::iterator cur = fValues.begin(), end = fValues.end(); cur != end; cur++)
		{
			if (*cur != nil)
				(*cur)->Release();
			*cur = nil;
		}
	}
	ClearAllSubEntityCaches();
	return err;
}



EntityAttributeValue* EntityRecord::do_getAttributeValue(const EntityAttribute* inAttribute, VError& err, BaseTaskInfo* context, bool restrictValue)
{
	err = VE_OK;
	EntityAttributeValue* result = nil;
	xbox_assert(inAttribute != nil);
	if (inAttribute == nil)
		return nil;
	xbox_assert(fModel != nil && fModel->isExtendedFrom(inAttribute->GetOwner()));

	sLONG pos = inAttribute->GetPosInOwner();
	xbox_assert(pos > 0 && pos <= fValues.size());

	result = fValues[pos-1];
	EntityAttributeKind kind = inAttribute->GetKind();

	if (result == nil || kind == eattr_computedField || kind == eattr_alias)
	{
		bool mustCallEvent = (result == nil);

		switch(kind)
		{
			case eattr_alias:
				{
					VValueSingle* cv3 = nil;
					if (result == nil)
						result = new EntityAttributeValue(this, inAttribute, eav_vvalue, nil, true);

					if (inAttribute->GetPathID() != -1)
					{
						SubEntityCache* cache = GetSubEntityCache(inAttribute->GetPathID(), err, true, inAttribute->GetSubEntityModel(), context);
						if (cache != nil)
						{
							EntityRecord* erec = cache->GetErec();
							if (erec != nil)
							{
								EntityAttributeValue* result2 = erec->getAttributeValue(inAttribute->getFlattenAttributeName(), err, context, restrictValue);
								if (result2 != nil)
								{
									VValueSingle* cv2 = result2->getVValue();
									if (cv2 != nil)
										cv3 = cv2->Clone();
								}
							}
						}
					}
					else
					{
						VString pathS = inAttribute->GetRelPathAsString();
						AttributePath path(fModel, pathS);
						EntityAttributeValue* relval = getAttributeValue(path, err, context);
						if (relval != nil)
						{
							VValueSingle* relcv = relval->getVValue();
							if (relcv != nil)
								cv3 = relcv->Clone();
						}

					}
					result->setVValue(cv3);
				}

				break;

			case eattr_computedField:
				{
					if (result == nil)
						result = new EntityAttributeValue(this, inAttribute, eav_vvalue, nil, true);
					VValueSingle* cv = nil;

					/*
					if (inAttribute->GetScriptKind() == script_db4d)
					{
						OptimizedQuery* script = inAttribute->GetScriptDB4D(context);
						if (script != nil)
						{
							cv = script->Compute(this, context, err);
						}
					}
					else
					*/
					{
						if (context->GetJSContext() != nil)
						{
							VJSContext jscontext(context->GetJSContext());
							VJSObject therecord(VJSEntitySelectionIterator::CreateInstance(jscontext, new EntitySelectionIterator(this, context)));
							VJSValue result(jscontext);
							VJSObject* objfunc = nil;
							VJSObject localObjFunc(jscontext);
							if (inAttribute->GetScriptObjFunc(script_attr_get, context->GetJSContext(), context, localObjFunc))
								objfunc = &localObjFunc;
							if (objfunc == nil)
								objfunc = context->getContextOwner()->GetJSFunction(inAttribute->GetScriptNum(script_attr_get), inAttribute->GetScriptStatement(script_attr_get), nil);
							
							//JSObjectSetProperty(*jscontext, therecord, name, funcref, kJSPropertyAttributeDontEnum, nil);
							XBOX::VJSException excep;
							if (objfunc != nil && therecord.CallFunction(*objfunc, nil, &result, excep))
							{
								cv = result.CreateVValue(inAttribute->isSimpleDate());
							}

							if (!excep.IsEmpty())
							{
								ThrowJSExceptionAsError(jscontext, excep);
								err = ThrowError(VE_DB4D_JS_ERR);
							}

						}
					}

					if (cv != nil)
					{
						if (cv->GetValueKind() != inAttribute->ComputeScalarType())
						{
							VValueSingle* cv2 = cv->ConvertTo(inAttribute->ComputeScalarType());
							result->setVValue(cv2);
						}
						else
						{
							result->setVValue(cv);
						}
					}
					else
						result->setVValue(cv);

				}
				break;

			default:
				break;
		}

		if (result != nil && result != (EntityAttributeValue*)-2 && result != (EntityAttributeValue*)-3 && result != (EntityAttributeValue*)-4 && result != (EntityAttributeValue*)-5)
		{
			if (mustCallEvent)
			{
				inAttribute->CallDBEvent(dbev_load, this, context, true);
			}

			result->AllowModifications(inAttribute->CanBeModified());
			fValues[pos-1] = result;
		}
	}

	return result;
}



VError EntityRecord::do_setAttributeValue(const EntityAttribute* inAttribute, const VValueSingle* inValue)
{
	VError err = VE_OK;

	BaseTaskInfo* context = fContext;
	EntityAttributeValue* val = getAttributeValue(inAttribute, err, context);
	if (err == VE_OK && val != nil)
	{
		if (val->GetAttributeKind() == eav_vvalue)
		{
			if (val->CanBeModified())
			{
				if (inAttribute->GetKind() == eattr_computedField)
				{
					if (inAttribute->GetScriptKind() == script_javascript)
					{
						//BaseTaskInfo* context = ConvertContext(fContext);
						if (context->GetJSContext() != nil)
						{
							VJSContext jscontext(context->GetJSContext());
							VJSObject therecord(VJSEntitySelectionIterator::CreateInstance(jscontext, new EntitySelectionIterator(this, fContext)));
							VJSValue result(jscontext);
							VectorOfVString params;
							params.push_back(L"paramValue");

							VJSObject* objfunc = nil;
							VJSObject localObjFunc(jscontext);
							if (inAttribute->GetScriptObjFunc(script_attr_set, context->GetJSContext(), context, localObjFunc))
								objfunc = &localObjFunc;
							if (objfunc == nil)
								objfunc = context->getContextOwner()->GetJSFunction(inAttribute->GetScriptNum(script_attr_set), inAttribute->GetScriptStatement(script_attr_set), &params);

							//JSObjectSetProperty(*jscontext, therecord, name, funcref, kJSPropertyAttributeDontEnum, nil);
							if (objfunc != nil)
							{
								vector<VJSValue> paramvalues;
								VJSValue valjs(jscontext);
								if (inValue == nil)
									valjs.SetNull(nil);
								else
									valjs.SetVValue(*inValue, nil);
								paramvalues.push_back(valjs);

								XBOX::VJSException excep;
								if (therecord.CallFunction(*objfunc, &paramvalues, &result, excep))
								{

								}

								if (!excep.IsEmpty())
								{
									ThrowJSExceptionAsError(jscontext, excep);
									err = ThrowError(VE_DB4D_JS_ERR);
								}
								else
									err = inAttribute->CallDBEvent(dbev_set, this, context, true);
							}
						}
					}

				}
			}
			else
				err = inAttribute->ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_IS_READ_ONLY);
		}
		else
			err = inAttribute->ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_IS_NOT_A_VVALUE);
	}

	return err;
}


EntityAttributeValue* EntityRecord::getAttributeValue(const EntityAttribute* inAttribute, VError& err, BaseTaskInfo* context, bool restrictValue)
{
	EntityAttributeValue* res = do_getAttributeValue(inAttribute, err, context, restrictValue);
	if (res != nil)
	{
		if (inAttribute->GetKind() == eattr_storage)
		{
			if (inAttribute->IsAutoGen())
			{
				VValueSingle* cv = res->getVValue();
				if (cv != nil && cv->GetValueKind() == VK_UUID)
				{
					if (cv->IsNull())
						((VUUID*)cv)->Regenerate();
				}
				
			}
		}
	}
	return res;
}


EntityAttributeValue* EntityRecord::getAttributeValue(const AttributePath& inAttPath, VError& err, BaseTaskInfo* context, bool restrictValue)
{
	EntityAttributeValue* result = nil;
	EntityRecord* curErec = this;
	const EntityAttributeInstanceCollection* atts = inAttPath.GetAll();
	for (EntityAttributeInstanceCollection::const_iterator cur = atts->begin(), end = atts->end(); cur != end && curErec != nil && err == VE_OK; cur++)
	{
		const EntityAttribute* att = cur->fAtt;
		result = curErec->getAttributeValue(att, err, context);
		if (result == nil)
			curErec = nil;
		else
			curErec = result->getRelatedEntity();
	}
	return result;
}


EntityAttributeValue* EntityRecord::getAttributeValue(const VString& inAttributeName, VError& err, BaseTaskInfo* context, bool restrictValue)
{
	EntityAttribute* cri = fModel->getAttribute(inAttributeName);
	if (cri == nil)
	{
		err = ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_NOT_FOUND, &inAttributeName);
		return nil;
	}
	else
		return getAttributeValue(cri, err, context);
}



EntityAttributeValue* EntityRecord::getAttributeValue(sLONG pos, VError& err, BaseTaskInfo* context, bool restrictValue)
{
	EntityAttribute* cri = fModel->getAttribute(pos);
	if (cri == nil)
	{
		VString s;
		s.FromLong(pos);
		err = ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_NOT_FOUND, &s);
		return nil;
	}
	else
		return getAttributeValue(cri, err, context);
}




bool EntityRecord::equalAttributeValue(const VString& inAttributeName, const VValueSingle* inValue)
{
	EntityAttribute* cri = fModel->getAttribute(inAttributeName);
	if (cri == nil)
	{
		return false;
	}
	else
		return equalAttributeValue(cri, inValue);
}


bool EntityRecord::equalAttributeValue(const EntityAttribute* inAttribute, const VValueSingle* inValue)
{
	VError err = VE_OK;

	EntityAttributeValue* val = getAttributeValue(inAttribute, err, ConvertContext(fContext));
	if (err == VE_OK && val != nil)
	{
		return val->equalValue(inValue);
	}
	else
		return false;

}


VError EntityRecord::setAttributeValue(const VString& inAttributeName, const VString& inJsonValue)
{
	EntityAttribute* cri = fModel->getAttribute(inAttributeName);
	if (cri == nil)
	{
		return ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_NOT_FOUND, &inAttributeName);
	}
	else
		return setAttributeValue(cri, inJsonValue);
}


VError EntityRecord::setAttributeValue(const EntityAttribute* inAttribute, const VString& inJsonValue)
{
	VError err = VE_OK;

	return err;
}



VError EntityRecord::setAttributeValue(const VString& inAttributeName, const VJSObject& inJSObject)
{
	EntityAttribute* cri = fModel->getAttribute(inAttributeName);
	if (cri == nil)
	{
		return ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_NOT_FOUND, &inAttributeName);
	}
	else
		return setAttributeValue(cri, inJSObject);
}


VError EntityRecord::setAttributeValue(const EntityAttribute* inAttribute, const VJSObject& inJSObject)
{
	VError err = VE_OK;

	EntityAttributeValue* val = getAttributeValue(inAttribute, err, ConvertContext(fContext));
	if (err == VE_OK && val != nil)
	{
		if (val->GetAttributeKind() == eav_composition)
		{
			val->SetJSObject(inJSObject);
			err = inAttribute->CallDBEvent(dbev_set, this, ConvertContext(fContext), true);
		}
	}


	return err;
}



VError EntityRecord::setAttributeValue(const VString& inAttributeName, const VValueSingle* inValue)
{
	EntityAttribute* cri = fModel->getAttribute(inAttributeName);
	if (cri == nil)
	{
		return ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_NOT_FOUND, &inAttributeName);
	}
	else
		return setAttributeValue(cri, inValue);
}


VError EntityRecord::setAttributeValue(const VString& inAttributeName, const VectorOfVValue& inIdentKey)
{
	EntityAttribute* cri = fModel->getAttribute(inAttributeName);
	if (cri == nil)
	{
		return ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_NOT_FOUND, &inAttributeName);
	}
	else
		return setAttributeValue(cri, inIdentKey);
}


VError EntityRecord::setAttributeValue(const EntityAttribute* inAttribute, const VectorOfVValue& inIdentKey)
{
	VError err = VE_OK;
	BaseTaskInfo* context = ConvertContext(fContext);

	EntityAttributeValue* val = getAttributeValue(inAttribute, err, context);
	if (err == VE_OK && val != nil)
	{
		if (val->GetAttributeKind() == eav_subentity)
		{
			EntityModel* relatedEM = inAttribute->GetSubEntityModel();
			if (testAssert(relatedEM != nil))
			{
				if (relatedEM->HasPrimKey())
				{
					val->setRelatedKey(inIdentKey);
				}
				else
					err = inAttribute->ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_IS_NOT_A_VVALUE);
			}

		}
	}

	return err;
}



VError EntityRecord::setAttributeValue(const VString& inAttributeName, EntityRecord* inRelatedEntity)
{
	EntityAttribute* cri = fModel->getAttribute(inAttributeName);
	if (cri == nil)
	{
		return ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_NOT_FOUND, &inAttributeName);
	}
	else
		return setAttributeValue(cri, inRelatedEntity);
}


VError EntityRecord::setAttributeValue(const EntityAttribute* inAttribute, EntityRecord* inRelatedEntity)
{
	VError err = VE_OK;

	BaseTaskInfo* context = ConvertContext(fContext);
	EntityAttributeValue* eVal = getAttributeValue(inAttribute, err, context);
	if (eVal != nil && err == VE_OK)
	{
		err = eVal->SetRelatedEntity(inRelatedEntity, context);
	}

	return err;
}


VError EntityRecord::touchAttributeValue(const EntityAttribute* inAttribute)
{
	VError err = VE_OK;

	BaseTaskInfo* context = ConvertContext(fContext);
	EntityAttributeValue* val = getAttributeValue(inAttribute, err, context);
	if (err == VE_OK && val != nil)
	{
		val->Touch(context);
	}

	return err;
}


VErrorDB4D EntityRecord::resetAttributeValue(const EntityAttribute* inAttribute)
{
	VError err = VE_OK;
	EntityAttributeValue* val = nil;
	xbox_assert(inAttribute != nil);
	if (inAttribute != nil)
	{
		xbox_assert(inAttribute->GetOwner() == fModel);
		sLONG pos = inAttribute->GetPosInOwner();
		xbox_assert(pos > 0 && pos <= fValues.size());
		val = fValues[pos-1];
		if (val != nil)
		{
			val->Release();
			fValues[pos-1] = nil;
		}
		sLONG pathID = inAttribute->GetPathID();
		if (pathID >= 0)
			ClearSubEntityCache(pathID);
	}
	return err;
}


VErrorDB4D EntityRecord::resetAttributeValue(const VString& inAttributeName)
{
	EntityAttribute* cri = fModel->getAttribute(inAttributeName);
	if (cri == nil)
	{
		return ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_NOT_FOUND, &inAttributeName);
	}
	else
		return resetAttributeValue(cri);
}






void EntityRecord::GetPrimKeyValue(VValueBag& outBagKey)
{
	if (!do_GetPrimKeyValueAsBag(outBagKey))
	{
		StErrorContextInstaller errs(false);
		VError err;
		const IdentifyingAttributeCollection* primatts = fModel->GetPrimaryAtts();
		for (IdentifyingAttributeCollection::const_iterator cur = primatts->begin(), end = primatts->end(); cur != end; cur++)
		{
			EntityAttributeValue* val = getAttributeValue(cur->fAtt, err, ConvertContext(fContext));
			if (val != nil && val->getVValue() != nil && !val->getVValue()->IsNull())
				outBagKey.SetAttribute(cur->fAtt->GetName(), val->getVValue()->Clone());
		}
	}
}


void EntityRecord::GetPrimKeyValue(VJSObject& outObj)
{
	if (!do_GetPrimKeyValueAsObject(outObj))
	{
		const IdentifyingAttributeCollection* primatts = fModel->GetPrimaryAtts();
		VError err = VE_OK;
		outObj.MakeEmpty();
		for (IdentifyingAttributeCollection::const_iterator cur = primatts->begin(), end = primatts->end(); cur != end; cur++)
		{
			EntityAttributeValue* val = getAttributeValue(cur->fAtt, err, ConvertContext(fContext));
			if (val != nil && val->getVValue() != nil)
			{
				VJSValue jsval(outObj.GetContext());
				jsval.SetVValue(*(val->getVValue()));
				outObj.SetProperty(cur->fAtt->GetName(), jsval, JS4D::PropertyAttributeNone);
			}
		}
		VJSValue jsval(outObj.GetContext());
		sLONG stamp = GetModificationStamp();
		jsval.SetNumber(stamp);
		outObj.SetProperty("__STAMP", jsval, JS4D::PropertyAttributeNone);
	}
}


void EntityRecord::GetPrimKeyAsString(const VectorOfVValue& values, VString& outKey, bool autoQuotes)
{
	bool first = true;
	for (VectorOfVValue::const_iterator cur = values.begin(), end = values.end(); cur != end; ++cur)
	{
		bool mustsimplequotes = false, mustdoublequotes = false, mustquotes = false;
		UniChar c;
		VString s;
		(*cur)->GetString(s);
		if (autoQuotes)
		{
			sLONG curpos = 0;
			while (curpos < s.GetLength())
			{
				c = s[curpos];
				curpos++;
				if (c == '&' || c == '/' || c == '?' || c == 34 || c == 39 || c == ')' || c == ',')
				{
					mustquotes = true;
					if (c == 34)
						mustsimplequotes = true;
					if (c == 39)
						mustdoublequotes = true;
				}
			}
			if (mustquotes && !mustsimplequotes && !mustdoublequotes)
				mustsimplequotes = true;

		}
		if (first)
			first = false;
		else
			outKey.AppendUniChar('&');

		if (mustsimplequotes)
			outKey.AppendUniChar(39);
		else if (mustdoublequotes)
			outKey.AppendUniChar(34);

		outKey += s;

		if (mustsimplequotes)
			outKey.AppendUniChar(39);
		else if (mustdoublequotes)
			outKey.AppendUniChar(34);

	}
}


void EntityRecord::GetPrimKeyValue(VString& outKey, bool autoQuotes)
{
	if (!do_GetPrimKeyValueAsString(outKey, autoQuotes))
	{
		StErrorContextInstaller errs(false);
		VectorOfVValue vals;
		const IdentifyingAttributeCollection* primatts = fModel->GetPrimaryAtts();
		for (IdentifyingAttributeCollection::const_iterator cur = primatts->begin(), end = primatts->end(); cur != end; cur++)
		{
			VError err = VE_OK;
			EntityAttributeValue* val = getAttributeValue(cur->fAtt, err, ConvertContext(fContext));
			if (testAssert(val != nil && val->getVValue() != nil))
			{
				vals.push_back(val->getVValue());
			}
		}
		GetPrimKeyAsString(vals, outKey, autoQuotes);

#if 0
		VError err;
		UniChar c;
		const IdentifyingAttributeCollection* primatts = fModel->GetPrimaryAtts();
		bool first = true;
		for (IdentifyingAttributeCollection::const_iterator cur = primatts->begin(), end = primatts->end(); cur != end; cur++)
		{
			EntityAttributeValue* val = getAttributeValue(cur->fAtt, err, ConvertContext(fContext));
			if (val != nil && val->getVValue() != nil)
			{
				VString s;
				val->getVValue()->GetString(s);
				bool mustsimplequotes = false, mustdoublequotes = false, mustquotes = false;
				if (autoQuotes)
				{
					sLONG curpos = 0;
					while (curpos < s.GetLength())
					{
						c = s[curpos];
						curpos++;
						if (c == '&' || c == '/' || c == '?' || c == 34 || c == 39 || c == ')' || c == ',')
						{
							mustquotes = true;
							if (c == 34)
								mustsimplequotes = true;
							if (c == 39)
								mustdoublequotes = true;
						}
					}
					if (mustquotes && !mustsimplequotes && !mustdoublequotes)
						mustsimplequotes = true;
						
				}
				if (first)
					first = false;
				else
					outKey.AppendUniChar('&');

				if (mustsimplequotes)
					outKey.AppendUniChar(39);
				else if (mustdoublequotes)
					outKey.AppendUniChar(34);

				outKey += s;

				if (mustsimplequotes)
					outKey.AppendUniChar(39);
				else if (mustdoublequotes)
					outKey.AppendUniChar(34);
			}
		}
#endif
	}
}


void EntityRecord::GetPrimKeyValue(VectorOfVValue& outPrimkey)
{
	if (!do_GetPrimKeyValueAsVValues(outPrimkey))
	{
		StErrorContextInstaller errs(false);
		VError err;
		const IdentifyingAttributeCollection* primatts = fModel->GetPrimaryAtts();
		for (IdentifyingAttributeCollection::const_iterator cur = primatts->begin(), end = primatts->end(); cur != end; cur++)
		{
			EntityAttributeValue* val = getAttributeValue(cur->fAtt, err, ConvertContext(fContext));
			if (val != nil && val->getVValue() != nil)
			{
				outPrimkey.push_back(val->getVValue());
			}
		}
	}
}



VError EntityRecord::setIdentifyingAtts(const VectorOfVValue& idents)
{
	VError err = VE_OK;
	const IdentifyingAttributeCollection* primatts = fModel->GetIdentifyingAtts();
	VectorOfVValue::const_iterator curkey = idents.begin();
	for (IdentifyingAttributeCollection::const_iterator cur = primatts->begin(), end = primatts->end(); cur != end && err == VE_OK; cur++, curkey++)
	{
		if (curkey == idents.end())
		{
			if (!cur->fOptionnel)
				err = ThrowError(VE_DB4D_PRIMKEY_MALFORMED);
			break;
		}
		err = setAttributeValue(cur->fAtt, *curkey);
	}

	return err;
}


VError EntityRecord::setPrimKey(const VectorOfVValue& primkey)
{
	VError err = VE_OK;
	const IdentifyingAttributeCollection* primatts = fModel->GetPrimaryAtts();
	VectorOfVValue::const_iterator curkey = primkey.begin();
	for (IdentifyingAttributeCollection::const_iterator cur = primatts->begin(), end = primatts->end(); cur != end && err == VE_OK; cur++, curkey++)
	{
		if (curkey == primkey.end())
		{
			err = ThrowError(VE_DB4D_PRIMKEY_MALFORMED);
			break;
		}
		err = setAttributeValue(cur->fAtt, *curkey);
	}

	return err;
}

VError EntityRecord::setPrimKey(VJSObject objkey)
{
	VError err = VE_OK;
	VJSValue jsval(objkey.GetContext());
	const IdentifyingAttributeCollection* primatts = fModel->GetPrimaryAtts();
	
	if (!primatts->empty())
	{
		bool first = true;
		bool okcont = true;
		for (IdentifyingAttributeCollection::const_iterator cur = primatts->begin(), end = primatts->end(); cur != end && err == VE_OK ; cur++)
		{
			const EntityAttribute* att = cur->fAtt;
			jsval = objkey.GetProperty(att->GetName());
			if (!jsval.IsUndefined() && !jsval.IsNull())
			{
				VValueSingle* cv = jsval.CreateVValue();
				if (cv != nil)
				{
					err = setAttributeValue(att, cv);
					delete cv;
				}
				else
					okcont = false;
			}
			else
				okcont = false;
		}
	}

	return err;
}


BaseTaskInfo* EntityRecord::getContext()
{
	return fContext;
}

CDB4DBaseContext* EntityRecord::GetContext()
{
	return fContext;
}



VError EntityRecord::Validate(BaseTaskInfo* context)
{
	VError err = VE_OK;
	bool validFail = false;

	for (EntityAttributeCollection::const_iterator cur = fModel->getAllAttributes().begin(), end = fModel->getAllAttributes().end(); cur != end; ++cur)
	{
		const EntityAttribute* att = *cur;
		if (att->NeedValidation())
		{
			EntityAttributeValue* val = getAttributeValue(att, err, context, false);
			if (err == VE_OK)
			{
				err = val->Validate(context);
			}
			if (err != VE_OK)
				validFail = true;
		}
	}


	if (!validFail)
	{
		err = CallDBEvent(dbev_validate, context);
	}

	if (validFail)
		err = ThrowError(VE_DB4D_ENTITY_RECORD_FAILS_VALIDATION);

	return err;
}

VError EntityRecord::Validate()
{
	return Validate(ConvertContext(fContext));
}


VError EntityRecord::CallSaveEvents(BaseTaskInfo* context, bool refreshOnly)
{
	VError err = CallDBEvent(dbev_save, context);
	if (err == VE_OK)
	{
		for (EntityAttributeValueCollection::iterator cur = fValues.begin(), end = fValues.end(); cur != end && err == VE_OK; cur++)
		{
			EntityAttributeValue* val = *cur;
			if (val != nil)
			{
				const EntityAttribute* att = val->GetAttribute();
				if (att->GetKind() == eattr_computedField || val->GetStamp() != 0)
				{
					err = att->CallDBEvent(refreshOnly ? dbev_clientrefresh : dbev_save, this, context, true);
				}
			}
		}
	}
	return err;
}




VError EntityRecord::ValidateRemove(BaseTaskInfo* context)
{
	VError err = VE_OK;
	bool validFail = false;

	for (EntityAttributeCollection::const_iterator cur = fModel->getAllAttributes().begin(), end = fModel->getAllAttributes().end(); cur != end; ++cur)
	{
		const EntityAttribute* att = *cur;
		if (att->NeedValidationOnRemove())
		{
			EntityAttributeValue* val = getAttributeValue(att, err, context, false);
			if (err == VE_OK)
			{
				err = val->ValidateRemove(context);
			}
			if (err != VE_OK)
				validFail = true;
		}
	}


	if (!validFail)
	{
		err = CallDBEvent(dbev_validateremove, context);
	}

	if (validFail)
		err = ThrowError(VE_DB4D_ENTITY_RECORD_FAILS_VALIDATION);

	return err;
}


VError EntityRecord::ValidateRemove()
{
	return Validate(ConvertContext(fContext));
}


/*
void EntityRecord::ReleaseExtraDatas()
{
	for (EntityAttributeValueCollection::iterator cur = fValues.begin(), end = fValues.end(); cur != end; cur++)
	{
		EntityAttributeValue* val = *cur;
		if (val != nil)
		{
			void* extradata = val->GetExtraData();
			if (extradata != nil)
			{
#if !VERSION_LINUX   // Postponed Linux Implementation !
				db4dJSPictContainer* xpic = (db4dJSPictContainer*)extradata;
				xpic->ClearEntityRef();
				val->SetExtraData(nil);extradata
#endif
			}
		}
	}
}
*/

/*
VValueSingle* EntityRecord::getFieldValue(Field* cri, VError& err)
{
	err = VE_OK;
	VValueSingle* result = nil;
	xbox_assert(cri != nil);
	if (cri != nil)
	{
		EntityAttribute* att = fModel->FindAttributeByFieldPos(cri->GetPosInRec());
		if (att != nil)
		{
			EntityAttributeValue *val = getAttributeValue(att, err, ConvertContext(fContext));
			if (val != nil)
			{
				result = val->getVValue();
			}
		}
		else
		{
			if (fMainRec != nil)
			{
				result = fMainRec->GetFieldValue(cri, err);
			}
		}
	}

	return result;
}



VError EntityRecord::setFieldValue(Field* cri, const VValueSingle* inValue)
{
	VError err = VE_OK;
	xbox_assert(cri != nil);
	xbox_assert(inValue != nil);
	if (cri != nil && inValue != nil)
	{
		EntityAttribute* att = fModel->FindAttributeByFieldPos(cri->GetPosInRec());
		if (att != nil)
		{
			err = setAttributeValue(att, inValue);
		}
	}
	else
	{
		VValueSingle* cv = fMainRec->GetFieldValue(cri, err);
		if (cv != nil)
		{
			cv->FromValue(*inValue);
			fMainRec->Touch(cri);
		}
	}
	return err;
}
*/


VError EntityRecord::ConvertToJSObject(VJSObject& outObj, const VString& inAttributeList, bool withKey, bool allowEmptyAttList)
{
	return VE_OK;
}


VError EntityRecord::ConvertToJSObject(VJSObject& outObj, EntityAttributeSortedSelection& whichAttributes, EntityAttributeSelection* expandAttributes, 
						 EntityAttributeSortedSelection* sortingAttributes, bool withKey, bool allowEmptyAttList)
{
	VError err = VE_OK;
	EntityModel* em = GetOwner();

	if (whichAttributes.empty() && !allowEmptyAttList)
	{
		if (!em->GetAllSortedAttributes(whichAttributes, nil))
			err = ThrowBaseError(memfull);
	}

	JS4D::ContextRef jscontext = outObj.GetContext();
	BaseTaskInfo* context = ConvertContext(fContext);

	if (err == VE_OK)
	{
		if (withKey && !IsNew())
		{
			/*
			VJSObject keyobj(jscontext);
			GetPrimKeyValue(keyobj);
			outObj.SetProperty("__KEY", keyobj, JS4D::PropertyAttributeNone);
			*/
			VString keyString;
			GetPrimKeyValue(keyString, false);
			outObj.SetProperty("__KEY", keyString, JS4D::PropertyAttributeNone);
			outObj.SetProperty("__STAMP", GetModificationStamp(), JS4D::PropertyAttributeNone);
		}

		sLONG nb = (sLONG)whichAttributes.size(), i = 1;
		for (EntityAttributeSortedSelection::iterator cur = whichAttributes.begin(), end = whichAttributes.end(); cur != end && err == VE_OK; cur++, i++)
		{
			const EntityAttribute* attribute = cur->fAttribute;
			if (attribute != nil)
			{
				bool restrictValue = false;
				EntityAttributeItem* eai = nil;
				if (expandAttributes != nil)
					eai = &(*expandAttributes)[attribute->GetPosInOwner()-1];
				if (eai == nil || eai->fAttribute == nil)
					restrictValue = true;

				EntityAttributeValue* val = getAttributeValue(attribute, err, context, false);
				if (val != nil)
				{
					EntityAttributeValueKind kind = val->GetKind();
					switch(kind)
					{
					case eav_vvalue:
						{
							VJSContext vjsContext(jscontext);
							VJSValue valueJS(vjsContext);
							VValueSingle* cv = val->GetVValue();
							if (cv != nil)
							{
								valueJS.SetVValue(*cv);
							}
							else
								valueJS.SetUndefined();

							outObj.SetProperty(attribute->GetName(), valueJS, JS4D::PropertyAttributeNone);
						}
						break;

					case eav_subentity:
						{
							CDB4DEntityRecord* subrec = val->GetRelatedEntity();
							if (subrec != nil)
							{
								EntityModel* submodel = val->getRelatedEntityModel();
								VJSObject subObj( outObj, attribute->GetName());
								EntityAttributeItem* eai = nil;
								if (expandAttributes != nil)
									eai = &(*expandAttributes)[attribute->GetPosInOwner()-1];
								if (eai != nil && eai->fAttribute != nil)
								{
									if (cur->fSousSelection == nil)
									{
										cur->fSousSelection = new EntityAttributeSortedSelection(submodel);
										submodel->GetAllSortedAttributes(*cur->fSousSelection, nil);
									}

									EntityAttributeSortedSelection* subSortingAttributes = nil;
									if (sortingAttributes != nil)
										subSortingAttributes = sortingAttributes->FindSubSelection(attribute);

									err = dynamic_cast<EntityRecord*>(subrec)->ConvertToJSObject(subObj, *cur->fSousSelection, eai->fSousSelection, subSortingAttributes, false, true);
								}
								else
								{
									EntityAttributeSortedSelection subAtts(submodel);
									err = dynamic_cast<EntityRecord*>(subrec)->ConvertToJSObject(subObj, subAtts, nil, nil, true, true);
								}

							}
							/*
							else
							{
								VJSValue valueJS(jscontext);
								valueJS.SetNull();
								outObj.SetProperty(attribute->GetName(), valueJS, JS4D::PropertyAttributeNone);
							}
							*/
						}
						break;

					case eav_selOfSubentity:
					case eav_composition:
						{
							EntityModel* submodel = val->getRelatedEntityModel();
							EntityCollection* sel = val->getRelatedSelection();
							if (sel != nil && submodel != nil)
							{
								EntityAttributeItem* eai = nil;
								if (expandAttributes != nil)
									eai = &(*expandAttributes)[attribute->GetPosInOwner()-1];
								if (eai != nil && eai->fAttribute != nil)
								{
									if (cur->fSousSelection == nil)
									{
										cur->fSousSelection = new EntityAttributeSortedSelection(submodel);
										submodel->GetAllSortedAttributes(*cur->fSousSelection, nil);
									}

									EntityAttributeSortedSelection* subSortingAttributes = nil;
									if (sortingAttributes != nil)
										subSortingAttributes = sortingAttributes->FindSubSelection(attribute);

									
									sLONG countelem = eai->fCount;
									if (countelem == -1)
									{
										if (kind == eav_composition)
											countelem =  sel->GetLength(context);
										else
											countelem = submodel->GetDefaultTopSizeInUse();
									}
									VJSArray subarr(sel->ToJsArray(context, outObj.GetContext(), *cur->fSousSelection, eai->fSousSelection, subSortingAttributes, false, true, eai->fSkip, countelem, err, nil));
									outObj.SetProperty(attribute->GetName(), subarr);
								}
								else
								{
									if (kind == eav_composition)
									{
										EntityAttributeSortedSelection subatts(submodel);
										VJSArray subarr(sel->ToJsArray(context, outObj.GetContext(), subatts, nil, nil, false, true, 0, sel->GetLength(context), err, nil));
									}
									else
									{
										VJSObject subObj( outObj, attribute->GetName());
										subObj.SetProperty("__COUNT", sel->GetLength(context));
									}
								}
							}
						}
						break;

					}
				}
			}
		}

	}

	return err;
}

/*
VError SelToJSObject(BaseTaskInfo* context, VJSArray& outArr, EntityModel* em, Selection* inSel, const VString& inAttributeList , bool withKey, bool allowEmptyAttList, sLONG from, sLONG count)
{
	return VE_OK;
}


VError SelToJSObject(BaseTaskInfo* context, VJSArray& outArr, EntityModel* em, Selection* inSel, EntityAttributeSortedSelection& whichAttributes, EntityAttributeSelection* expandAttributes, 
						   EntityAttributeSortedSelection* sortingAttributes, bool withKey, bool allowEmptyAttList, sLONG from, sLONG count)
{
	VError err = VE_OK;
	Selection* trueSel = sortingAttributes == nil ? RetainRefCountable(inSel) : inSel->SortSel(err, em, sortingAttributes, context, nil);
	if (err == VE_OK && trueSel != nil)
	{
		inSel = trueSel;

		if (from < 0)
			from = 0;
		sLONG qt = inSel->GetQTfic();
		if (count < 0)
			count = qt;
		sLONG lastrow = from+count-1;
		if (lastrow >= qt)
			lastrow = qt-1;

		SelectionIterator itersel(inSel);
		sLONG lastrec = itersel.SetCurrentRecord(lastrow+1);
		sLONG currec = itersel.SetCurrentRecord(from);
		while (currec != -1 && currec != lastrec && err == VE_OK)
		{
			EntityRecord* erec = em->LoadEntityRecord(currec, err, DB4D_Do_Not_Lock, context, false);
			if (erec != nil)
			{
				VJSObject subObj(outArr, -1);
				err = erec->ConvertToJSObject(subObj, whichAttributes, expandAttributes, sortingAttributes, withKey, allowEmptyAttList);
				erec->Release();
			}
			currec = itersel.NextRecord();
		}
	}

	QuickReleaseRefCountable(trueSel);
	return err;
}
*/


/*
VError EntityRecord::convertToJSObject(VJSObject& outObj)
{
	VError err = VE_OK;
	outObj.MakeEmpty();
	BaseTaskInfo* context = ConvertContext(fContext);
	VJSContext jscontext(context->GetJSContext());
	const EntityAttributeCollection& allAtts = fModel->getAllAttributes();
	for (EntityAttributeCollection::const_iterator cur = allAtts.begin(), end = allAtts.end(); cur != end && err == VE_OK; cur++)
	{
		const EntityAttribute* att = *cur;
		EntityAttributeValue* eVal = getAttributeValue(att, err, context);
		if (eVal != nil)
		{
			switch (eVal->GetKind())
			{
				case eav_vvalue:
					{
						VJSValue valueJS(jscontext);
						VValueSingle* cv = eVal->GetVValue();
						if (cv != nil)
						{
							if (cv->GetValueKind() == VK_IMAGE)
							{
#if !VERSION_LINUX   // Postponed Linux Implementation !
								void* extradata = eVal->GetExtraData();
								if (extradata == nil)
								{
									db4dJSPictContainer* xpic = new db4dJSPictContainer(cv, jscontext, this, (EntityAttribute*)att);
									valueJS = VJSImage::CreateInstance(jscontext, xpic);
									eVal->SetExtraData(xpic);
									xpic->Release();
								}
								else
								{
									valueJS = VJSImage::CreateInstance(jscontext, (db4dJSPictContainer*)extradata);
								}
#endif
							}
							else
								valueJS.SetVValue(*cv);
						}
						else
							valueJS.SetUndefined();

						outObj.SetProperty(att->GetName(), valueJS, JS4D::PropertyAttributeNone);
					}
					break;

				case eav_subentity:
					{
						VJSValue valueJS(jscontext);
						CDB4DEntityRecord* subrec = eVal->GetRelatedEntity();
						if (subrec != nil)
						{
							EntitySelectionIterator* subiter = new EntitySelectionIterator(subrec, context);
							valueJS = VJSEntitySelectionIterator::CreateInstance(jscontext, subiter);
						}
						else
							valueJS.SetNull();
						outObj.SetProperty(att->GetName(), valueJS, JS4D::PropertyAttributeNone);
					}
					break;

				case eav_selOfSubentity:
					{
						VJSValue valueJS(jscontext);
						CDB4DSelection* sel = eVal->GetRelatedSelection();
						if (sel != nil)
						{
							valueJS = VJSEntitySelection::CreateInstance(jscontext, sel);								
						}
						else
							valueJS.SetNull();
						outObj.SetProperty(att->GetName(), valueJS, JS4D::PropertyAttributeNone);
					}
					break;

				case eav_composition:
					{
						VJSObject objJS(jscontext);
						err = eVal->GetJSObject(objJS);
						if (err == VE_OK)
						{
							outObj.SetProperty(att->GetName(), objJS, JS4D::PropertyAttributeNone);
						}
					}
					break;
			}
		}
	}
	return err;
}
*/

VError EntityRecord::convertFromJSObj(VJSObject recobj)
{
	VError err = VE_OK;
	BaseTaskInfo* context = ConvertContext(fContext);

	vector<VString> props;
	recobj.GetPropertyNames(props);

	for (vector<VString>::const_iterator cur = props.begin(), end = props.end(); cur != end && err == VE_OK; cur++)
	{
		const EntityAttribute* att = fModel->getAttribute(*cur);
		if (att != nil)
		{
			EntityAttributeValue* eVal = getAttributeValue(att, err, context);
			if (eVal != nil)
			{
				VJSValue jsval(recobj.GetContext());
				jsval = recobj.GetProperty(*cur);
				switch (eVal->GetKind())
				{
					case eav_vvalue:
						{
							VValueSingle* cv = jsval.CreateVValue(att->isSimpleDate());
							setAttributeValue(att, cv);
							if (cv != nil)
								delete cv;
						}
						break;

					case eav_subentity:
						{
							EntitySelectionIterator* xrelatedRec = jsval.GetObjectPrivateData<VJSEntitySelectionIterator>();
							if (xrelatedRec == nil)
							{
								if (jsval.IsNull() || jsval.IsUndefined())
									setAttributeValue(att, (EntityRecord*)nil);
								else
								{
									VValueSingle* cv = jsval.CreateVValue();
									if (cv != nil)
									{
										setAttributeValue(att, cv);
										delete cv;
									}
								}
							}
							else
								SetAttributeValue(att, xrelatedRec->GetCurRec(fContext));
						}
						break;

					case eav_composition:
						{
							if (jsval.IsObject())
							{
								VJSObject objcompo = jsval.GetObject();
								if (objcompo.IsInstanceOf("Array"))
									eVal->SetJSObject(objcompo);
								else
								{
									VJSArray emptyarray(recobj.GetContext());
									eVal->SetJSObject(emptyarray);
								}
							}
							else
							{
								VJSArray emptyarray(recobj.GetContext());
								eVal->SetJSObject(emptyarray);
							}
						}
						break;

					default:
						break;
				}
			}
			
		}
	}

	return err;
}


VError EntityRecord::CallDBEvent(DBEventKind kind, BaseTaskInfo* context)
{
	return fModel->CallDBEvent(kind, this, context == nil ? ConvertContext(fContext) : context);
}


VError EntityRecord::CallInitEvent(BaseTaskInfo* context)
{
	if (context == nil)
		context = ConvertContext(fContext);

	{
		const EntityAttributeCollection& attsWithDefaultValue = fModel->GetAttributesWithDefaultValue();
		for (EntityAttributeCollection::const_iterator cur = attsWithDefaultValue.begin(), end = attsWithDefaultValue.end(); cur != end; cur++)
		{
			const EntityAttribute* att = *cur;
			const VValueSingle* cv = nil;
			if (att->IsScalar() && att->getDefaultValue(cv) && cv != nil)
			{
				setAttributeValue(att, cv);
			}
		}
	}

	VError err = CallDBEvent(dbev_init, context);
	if (err == VE_OK)
	{
		const EntityAttributeCollection& attsWithInit = fModel->GetAttributesWithInitEvent();
		for (EntityAttributeCollection::const_iterator cur = attsWithInit.begin(), end = attsWithInit.end(); cur != end; cur++)
		{
			const EntityAttribute* att = *cur;
			att->CallDBEvent(dbev_init, this, context, true);
		}
	}
	return err;
}


bool EntityRecord::OKToCallEvent(DBEventKind kind)
{
	bool result = false;
	if (fAlreadyCallEvent[kind] == 0)
	{
		fAlreadyCallEvent[kind] = 1;
		result = true;
	}
	return result;
}


void EntityRecord::ReleaseCallEvent(DBEventKind kind)
{
	xbox_assert(fAlreadyCallEvent[kind] == 1);
	fAlreadyCallEvent[kind] = 0;
}


VError EntityRecord::GetModifiedAttributes(EntityAttributeCollection& outatts)
{
	for (EntityAttributeValueCollection::iterator cur = fValues.begin(), end = fValues.end(); cur != end; ++cur)
	{
		EntityAttributeValue* val = *cur;
		if (val != nil)
		{
			if (val->GetStamp() != 0)
			{
				outatts.push_back(const_cast<EntityAttribute*>(val->GetAttribute()));
			}
		}
	}
	return VE_OK;
}


EntityRecord* EntityRecord::Clone(VError& outErr)
{
	EntityRecord* newrec = fModel->newEntity(fContext, true);
	outErr = DuplicateInto(newrec);
	return newrec;
}


VError EntityRecord::DuplicateInto(EntityRecord* otherRec)
{
	VError err = VE_OK;

	otherRec->fAlreadySaving = fAlreadySaving;
	otherRec->fAlreadyDeleting = fAlreadyDeleting;
	//otherRec->fAlreadyCallEvent fAlreadyCallEvent;

	sLONG pos = 0;
	for (EntityAttributeValueCollection::iterator cur = fValues.begin(), end = fValues.end(); cur != end; ++cur, ++pos)
	{
		EntityAttributeValue* val = *cur;
		if (val != nil)
		{
			EntityAttributeValue* val2 = val->Clone(otherRec, false);
			otherRec->fValues[pos] = val2;
		}
	}
	return err;
}









// ------------------------------------------------------------------------------------------


EntityModelCatalog::EntityModelCatalog(Base4D* assocBase)
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
	fill(&fForced[DB4D_EM_None_Perm], &fForced[DB4D_EM_Promote_Perm + 1], 0);
	fJsonFormat = true;
	fErrorReporter = nil;
	fLogger = nil;
	fModelFile = nil;
	fJSFile = nil;
	fModelFolder = nil;
	fModelJSName = "$mainModel";

}


EntityModelCatalog::~EntityModelCatalog()
{
	for (SubProjectMap::iterator cur = fSubProjects.begin(), end = fSubProjects.end(); cur != end; ++cur)
	{
		cur->second->Release();
	}

	for (EntityMethodMap::iterator cur = fMethodsByName.begin(), end = fMethodsByName.end(); cur != end; cur++)
	{
		cur->second->Release();
	}


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
	QuickReleaseRefCountable(fModelFile);
	QuickReleaseRefCountable(fJSFile);
	QuickReleaseRefCountable(fModelFolder);

	ReleaseJSModelName(fModelJSName);

	//EntityModel::ClearCacheTableEM();
}


VError EntityModelCatalog::ThrowError( VError inErrCode, ActionDB4D inAction, const VString* p1) const
{
	if (this != nil)
	{
		VUUID nullid;
		VErrorDB4D_OnBase *err = new VErrorDB4D_OnBase(inErrCode, inAction, this);
		if (p1 != nil)
			err->GetBag()->SetString(Db4DError::Param1, *p1);
		VTask::GetCurrent()->PushRetainedError( err);
	}

	return inErrCode;
}


void EntityModelCatalog::DisposeEntityModels()
{
	VTaskLock lock(&fEntityModelMutex);
	for (EntityModelMap::iterator cur = fEntityModels.begin(), end = fEntityModels.end(); cur != end; cur++)
	{
		cur->second->Release();
	}
	fEntityModels.clear();
}


void EntityModelCatalog::DisposeEntityRelations()
{
	VTaskLock lock(&fEntityModelMutex);
	for (EntityRelationCollection::iterator cur = fRelations.begin(), end = fRelations.end(); cur != end; cur++)
	{
		
		(*cur)->Release();
	}
	fRelations.clear();
}


void EntityModelCatalog::DisposeEntityTypes()
{
	VTaskLock lock(&fEntityModelMutex);
	for (AttributeTypeMap::iterator cur = fTypes.begin(), end = fTypes.end(); cur != end; cur++)
	{
		cur->second->Release();
	}
	fTypes.clear();
}


void EntityModelCatalog::DisposeEnumerations()
{
	VTaskLock lock(&fEntityModelMutex);
	for (EmEnumMap::iterator cur = fEnumerations.begin(), end = fEnumerations.end(); cur != end; cur++)
	{
		cur->second->Release();
	}
	fEnumerations.clear();
}




VError EntityModelCatalog::AddOneEntityModel(EntityModel* newEntity, bool canReplace)
{
	VError err = VE_OK;
	VTaskLock lock(&fEntityModelMutex);
	VString name;
	newEntity->GetName(name);
	EntityModelMap::iterator found = fEntityModels.find(name);

	if (found != fEntityModels.end())
	{
		if (canReplace)
		{
			found->second->Release();
			found->second = newEntity;
			newEntity->Retain();
		}
		else
			err = ThrowError(VE_DB4D_ENTITY_NAME_ALREADY_EXIST, noaction, &name);
	}
	else
	{
		newEntity->Retain();
		fEntityModels[name] = newEntity;
	}

	if (err == VE_OK)
	{
		const VString& collectionName = newEntity->GetCollectionName();
		fEntityModelsByCollectionName[collectionName] = newEntity;
	}

	return err;
}


EntityModel* EntityModelCatalog::BuildEntityModel(const VValueBag* bag, VError& err, ModelErrorReporter* errorReporter)
{
	err = VE_OK; 
	EntityModel* model = NewModel();
	err = model->FromBag(bag, this, errorReporter, false, false);
	if (err != VE_OK)
		ReleaseRefCountable(&model);
	return model;
}


VError EntityModelCatalog::AddOneEntityRelation(EntityRelation* newRelation, bool canReplace)
{
	VError err = VE_OK;
	VTaskLock lock(&fEntityModelMutex);
	fRelations.push_back(newRelation);
	newRelation->Retain();
	return err;
}

/*
EntityRelation* EntityModelCatalog::BuildEntityRelation(const VValueBag* bag, VError& err)
{
	err = VE_OK; 
	EntityRelation* rel = new EntityRelation();
	err = rel->FromBag(bag, fOwner, this);
	if (err != VE_OK)
		ReleaseRefCountable(&rel);
	return rel;
}
*/


VError EntityModelCatalog::AddOneType(AttributeType* newType, bool canReplace)
{
	VError err = VE_OK;
	VTaskLock lock(&fEntityModelMutex);
	AttributeTypeMap::iterator found = fTypes.find(newType->GetName());

	if (found != fTypes.end())
	{
		if (canReplace)
		{
			found->second->Release();
			found->second = newType;
			newType->Retain();
		}
		else
			err = ThrowError(VE_DB4D_TYPE_ALREADY_EXIST, noaction);
	}
	else
	{
		newType->Retain();
		fTypes[newType->GetName()] = newType;
	}
	return err;
}


AttributeType* EntityModelCatalog::BuildType(const VValueBag* bag, VError& err, ModelErrorReporter* errorReporter)
{
	err = VE_OK; 
	AttributeType* typ = new AttributeType(this);
	err = typ->FromBag(bag, this, true, errorReporter);
	if (err != VE_OK)
		ReleaseRefCountable(&typ);
	return typ;
}


AttributeType* EntityModelCatalog::FindType(const VString& typeName) const
{
	VTaskLock lock(&fEntityModelMutex);
	AttributeTypeMap::const_iterator found = fTypes.find(typeName);
	if (found == fTypes.end())
		return nil;
	else
		return found->second;
}


VError EntityModelCatalog::AddOneEnumeration(EmEnum* newEnum, bool canReplace)
{
	VError err = VE_OK;
	VTaskLock lock(&fEntityModelMutex);
	EmEnumMap::iterator found = fEnumerations.find(newEnum->GetName());

	if (found != fEnumerations.end())
	{
		if (canReplace)
		{
			found->second->Release();
			found->second = newEnum;
			newEnum->Retain();
		}
		else
			err = ThrowError(VE_DB4D_ENUMERATION_ALREADY_EXIST, noaction);
	}
	else
	{
		newEnum->Retain();
		fEnumerations[newEnum->GetName()] = newEnum;
	}
	return err;
}


EmEnum* EntityModelCatalog::BuildEnumeration(const VValueBag* bag, VError& err, ModelErrorReporter* errorReporter)
{
	err = VE_OK; 
	EmEnum* en = new EmEnum(this);
	err = en->FromBag(bag, this, errorReporter);
	if (err != VE_OK)
		ReleaseRefCountable(&en);
	return en;
}


EmEnum* EntityModelCatalog::FindEnumeration(const VString& enumName) const
{
	VTaskLock lock(&fEntityModelMutex);
	EmEnumMap::const_iterator found = fEnumerations.find(enumName);
	if (found == fEnumerations.end())
		return nil;
	else
		return found->second;
}


VError EntityModelCatalog::SecondPassLoadEntityModels(ModelErrorReporter* errorReporter, bool dataWasAlreadyThere)
{
	VError err = VE_OK;

	err = do_SecondPassLoadEntityModels(errorReporter, dataWasAlreadyThere);
	return err;
}


VError _addOneAttribute( const VJSValue& attVal, VValueBag* dataClassBag, const VString* attname, bool& outEmpty, bool forARemoteCatalog)
{
	bool res, exists;
	sLONG ll;
	VString s;
	outEmpty = !forARemoteCatalog;

	if (!attVal.IsNull() && attVal.IsObject() && !attVal.IsFunction())
	{
		VValueBag* attBag = nil;
		bool mustadd = false;
		VJSObject attObj(attVal.GetObject());

		VString attributeName;

		if (attname == nil)
		{
			if (attObj.GetPropertyAsString("name", nil, s))
				attributeName = s;
				//attBag->SetString(d4::name, s);
		}
		else
			attributeName = *attname;
			//attBag->SetString(d4::name, *attname);

		attBag = dataClassBag->RetainUniqueElementWithAttribute(d4::attributes, d4::name, attributeName);
		if (attBag == nil)
		{
			attBag = new VValueBag();
			attBag->SetString(d4::name, attributeName);
			mustadd = true;
		}
		else
			outEmpty = false;

		if (attObj.GetPropertyAsString("kind", nil, s))
		{
			attBag->SetString(d4::kind, s);
			outEmpty = false;
		}

		if (attObj.GetPropertyAsString("type", nil, s))
		{
			attBag->SetString(d4::type, s);
			outEmpty = false;
		}

		res = attObj.GetPropertyAsBool("simpleDate", nil, &exists);
		if (exists)
			attBag->SetBool(d4::simpleDate, res);
		else if (s == "date")
			attBag->SetBool(d4::simpleDate, true);

		if (attObj.GetPropertyAsString("scope", nil, s))
			attBag->SetString(d4::scope, s);

		if (attObj.GetPropertyAsString("indexKind", nil, s))
			attBag->SetString(d4::indexKind, s);

		if (attObj.GetPropertyAsString("path", nil, s))
			attBag->SetString(d4::path, s);

		ll = attObj.GetPropertyAsLong("cacheDuration", nil, &exists);
		if (exists)
			attBag->SetLong(d4::cacheDuration, ll);

		ll = attObj.GetPropertyAsLong("limiting_length", nil, &exists);
		if (exists)
			attBag->SetLong(DB4DBagKeys::limiting_length, ll);

		ll = attObj.GetPropertyAsLong("text_switch_size", nil, &exists);
		if (exists)
			attBag->SetLong(DB4DBagKeys::text_switch_size, ll);

		ll = attObj.GetPropertyAsLong("blob_switch_size", nil, &exists);
		if (exists)
			attBag->SetLong(DB4DBagKeys::blob_switch_size, ll);

		res = attObj.GetPropertyAsBool("unique", nil, &exists);
		if (exists)
			attBag->SetBool(DB4DBagKeys::unique, res);

		res = attObj.GetPropertyAsBool("not_null", nil, &exists);
		if (exists)
			attBag->SetBool(DB4DBagKeys::not_null, res);

		res = attObj.GetPropertyAsBool("never_null", nil, &exists);
		if (exists)
			attBag->SetBool(DB4DBagKeys::never_null, res);

		res = attObj.GetPropertyAsBool("autosequence", nil, &exists);
		if (exists)
			attBag->SetBool(DB4DBagKeys::autosequence, res);

		res = attObj.GetPropertyAsBool("autogenerate", nil, &exists);
		if (exists)
			attBag->SetBool(DB4DBagKeys::autogenerate, res);

		res = attObj.GetPropertyAsBool("autoComplete", nil, &exists);
		if (exists)
			attBag->SetBool(d4::autoComplete, res);

		res = attObj.GetPropertyAsBool("outside_blob", nil, &exists);
		if (exists)
			attBag->SetBool(DB4DBagKeys::outside_blob, res);

		res = attObj.GetPropertyAsBool("styled_text", nil, &exists);
		if (exists)
			attBag->SetBool(DB4DBagKeys::styled_text, res);

		res = attObj.GetPropertyAsBool("identifying", nil, &exists);
		if (exists)
			attBag->SetBool(d4::identifying, res);

		res = attObj.GetPropertyAsBool("primKey", nil, &exists);
		if (exists)
			attBag->SetBool(d4::primKey, res);

		res = attObj.GetPropertyAsBool("reversePath", nil, &exists);
		if (exists)
			attBag->SetBool(d4::reversePath, res);

		res = attObj.GetPropertyAsBool("readOnly", nil, &exists);
		if (exists)
			attBag->SetBool(d4::readOnly, res);

		res = attObj.GetPropertyAsBool("multiLine", nil, &exists);
		if (exists)
			attBag->SetBool(d4::multiLine, res);

		VJSValue valx(attObj.GetProperty("defaultFormat"));
		if (valx.IsObject())
		{
			VJSObject valobj(valx.GetObject());
			VValueBag* formatBag = new VValueBag();
			if (valobj.GetPropertyAsString("format", nil, s))
				formatBag->SetString(d4::format, s);
			if (valobj.GetPropertyAsString("presentation", nil, s))
				formatBag->SetString(d4::presentation, s);
			attBag->AddElement(d4::defaultFormat, formatBag);
			formatBag->Release();
			
		}
		else
		{
			if (valx.IsString())
			{
				valx.GetString(s);
				VValueBag* formatBag = new VValueBag();
				formatBag->SetString(d4::format, s);
				attBag->AddElement(d4::defaultFormat, formatBag);
				formatBag->Release();
			}
		}

		if (attObj.GetPropertyAsString("minValue", nil, s))
			attBag->SetString(d4::minValue, s);

		if (attObj.GetPropertyAsString("maxValue", nil, s))
			attBag->SetString(d4::maxValue, s);

		if (attObj.GetPropertyAsString("defaultValue", nil, s))
			attBag->SetString(d4::defaultValue, s);

		if (attObj.GetPropertyAsString("minLength", nil, s))
			attBag->SetString(d4::minLength, s);

		if (attObj.GetPropertyAsString("maxLength", nil, s))
			attBag->SetString(d4::maxLength, s);

		if (attObj.GetPropertyAsString("fixedLength", nil, s))
			attBag->SetString(d4::fixedLength, s);

		/*
		if (attObj.GetPropertyAsString("limiting_length", nil, s))
			attBag->SetString(d4::limiting_length, s);
		if (attObj.GetPropertyAsString("limitingLength", nil, s))
			attBag->SetString(d4::limiting_length, s);
			*/

		if (attObj.GetPropertyAsString("pattern", nil, s))
			attBag->SetString(d4::pattern, s);

		if (mustadd && !outEmpty)
			dataClassBag->AddElement(d4::attributes, attBag);
		attBag->Release();
	}

	return VE_OK;
}


typedef map<VString, VValueBag*, CompareLessVStringStrict> bagsMap;



ModelJSTransformer::ModelJSTransformer(Base4D* base)
{
	fParseLineError = 0;
	fParseFileError = nil;

	context = new BaseTaskInfo(base, nil, nil, nil);
	xJSDelegate = new DB4DJSRuntimeDelegate(context);
	xJSContext = VJSGlobalContext::Create( xJSDelegate);
	context->SetJSContext(xJSContext);
}


ModelJSTransformer::~ModelJSTransformer()
{
	QuickReleaseRefCountable(fParseFileError);

	QuickReleaseRefCountable(xJSContext);
	delete xJSDelegate;
	context->Release();
}


VError ModelJSTransformer::TransformJSIntoBag(VFile& inJSFile, const VString& modelRoot, VValueBag& bagEntities, unordered_map_VString<VRefPtr<VFile> >* outIncludedFiles, ModelErrorReporter* errorReporter, bool forARemoteCatalog)
{
	VError err = VE_OK;
	VJSContext jscontext( xJSContext);
	VJSObject globalObject( jscontext.GetGlobalObject());

	VFile* fileJs = &inJSFile;
	if (fileJs != nil)
	{
		if (fileJs->Exists())
		{
			StErrorContextInstaller errs(false, false);
			VJSValue result(jscontext);
			VJSException e, e1;
			VFolder* ResourceFolder = VDBMgr::GetManager()->RetainJSCodeResourceFolder();
			if (ResourceFolder != nil)
			{
				VFile libfile(*ResourceFolder, "ModelLoadTime.js");
				if (libfile.Exists())
				{
					jscontext.EvaluateScript(&libfile, &result, e1, nil);
					if (!e1.IsEmpty())
					{
						VJSValue ev(jscontext, e1);
#if debuglr
						VJSJSON json(jscontext);
						VString s;
						json.Stringify(ev, s);
						sLONG xdebug = 1; // put a break here
#endif						
					}
				}
				VFile libfile2(*ResourceFolder, "ModelPrepare.js");
				if (libfile2.Exists())
				{
					jscontext.EvaluateScript(&libfile2, &result, e1, nil);
					if (!e1.IsEmpty())
					{
						VJSValue ev(jscontext, e1);
#if debuglr
						VJSJSON json(jscontext);
						VString s;
						json.Stringify(ev, s);
						sLONG xdebug = 1; // put a break here
#endif						
					}
				}
			}

			jscontext.EvaluateScript(fileJs, &result, e, nil);
			if (!e.IsEmpty())
			{
				if (errorReporter != nil)
				{
					VJSValue ev(jscontext, e);
#if debuglr
					VJSJSON json(jscontext);
					VString s;
					json.Stringify(ev, s);
					sLONG xdebug = 1; // put a break here
#endif
					VJSObject eo(ev.GetObject());
					fParseLineError = eo.GetPropertyAsLong("line", nil, nil);
					eo.GetPropertyAsString("message", nil, fParseMessageError);
					VString path;
					eo.GetPropertyAsString("sourceURL", nil, path);
					 
					VURL url(path, true);

					VFilePath urlPath;
					url.GetFilePath(urlPath);
					
					fParseFileError = new VFile(urlPath.GetPath());
				}
				else
				{
					//err = ThrowBaseError(VE_DB4D_JS_ERR);
					err = e.ThrowVErrorForException(jscontext);
				}
			}
			if (outIncludedFiles != nil)
			{
				VJSGlobalObject* globobj = globalObject.GetPrivateData<VJSGlobalClass>();
				if (globobj != nil)
				{
					*outIncludedFiles = globobj->GetIncludedFiles();
				}
			}
		}
	}

	
	jscontext.GetGlobalObjectPrivateInstance()->SetSpecific( 'db4x', (void*)-1, VJSSpecifics::DestructorVoid);

	VJSObject			result(jscontext);
	VJSException		exception;

	bagsMap classesBags;
	const VBagArray* entities = bagEntities.GetElements(d4::dataClasses);
	if (entities != nil)
	{
		VIndex nbentities = entities->GetCount();
		for (VIndex i = 1; i <= nbentities; i++)
		{
			VValueBag* OneEntity = const_cast<VBagArray*>(entities)->GetNth(i);
			if (OneEntity != nil)
			{
				VString name;
				OneEntity->GetString(d4::className, name);
				classesBags[name] = OneEntity;
			}
		}
	}

	result = jscontext.GetGlobalObject().GetPropertyAsObject(modelRoot, exception);
	if (exception.IsEmpty() && result.IsObject()) 
	{
		VJSObject attObj(result);
		VJSPropertyIterator curprop(attObj);
		while (curprop.IsValid())
		{
			VString propName;
			curprop.GetPropertyName(propName);

			VJSValue dataClassVal(curprop.GetProperty());
			if (dataClassVal.IsObject() && !dataClassVal.IsFunction())
			{
				if (propName.EqualToUSASCIICString("_outsideSQLCatalogs"))
				{
					if (dataClassVal.IsArray())
					{
						VJSArray outCats(dataClassVal, false);
						for (sLONG i = 0, nb = (sLONG)outCats.GetLength(); i < nb; ++i)
						{
							VJSValue val = outCats.GetValueAt(i);
							if (val.IsObject())
							{
								VJSObject outCat(val.GetObject());
								VString catname;
								VString catpath;
								VString username;
								VString password;
								VString jsFilePath;
								if (outCat.GetPropertyAsString(khostname, nil, catpath))
								{
									VString s;
									sLONG port;
									VValueBag* bag = new VValueBag();
									bag->SetString(khostname, catpath);
									bool exists, withssl = false;
									port = outCat.GetPropertyAsLong(kport, nil, &exists);
									if (exists)
										bag->SetLong(kport, port);
									if (outCat.GetPropertyAsString(kLocalJSPath, nil, jsFilePath))
										bag->SetString(kLocalJSPath, jsFilePath);
									if (outCat.GetPropertyAsString(kdatabase, nil, username))
										bag->SetString(kdatabase, username);
									if (outCat.GetPropertyAsString(kuser, nil, username))
										bag->SetString(d4::user, username);
									if (outCat.GetPropertyAsString(kpassword, nil, password))
										bag->SetString(d4::password, password);
									withssl = outCat.GetPropertyAsBool(kssl, nil, &exists);
									if (exists)
										bag->SetBool(kssl, withssl);
									sLONG remoteSessionLength = outCat.GetPropertyAsLong(ktimeout, nil, &exists); // in minutes
									if (exists)
										bag->SetLong(ktimeout, remoteSessionLength);
									if (outCat.GetPropertyAsString("catalogName", nil, catname))
										bag->SetString("catalogName", catname);
									const_cast<VValueBag&>(bagEntities).AddElement("outsideSQLCatalogs", bag);
								}
							}
						}
					}
				}
				else if (propName.EqualToUSASCIICString("_outsideCatalogs"))
				{
					if (dataClassVal.IsArray())
					{
						VJSArray outCats(dataClassVal, false);
						for (sLONG i = 0, nb = (sLONG)outCats.GetLength(); i < nb; ++i)
						{
							VJSValue val = outCats.GetValueAt(i);
							if (val.IsObject())
							{
								VJSObject outCat(val.GetObject());
								VString catname;
								VString catpath;
								VString username;
								VString password;
								VString jsFilePath;
								if (outCat.GetPropertyAsString(khostname, nil, catpath))
								{
									VValueBag* bag = new VValueBag();
									bag->SetString(khostname, catpath);
									if (outCat.GetPropertyAsString("name", nil, catname))
										bag->SetString(d4::name, catname);
									if (outCat.GetPropertyAsString("catalogName", nil, catname))
										bag->SetString("catalogName", catname);
									if (outCat.GetPropertyAsString(kLocalJSPath, nil, jsFilePath))
										bag->SetString(kLocalJSPath, jsFilePath);
									if (outCat.GetPropertyAsString(kuser, nil, username))
										bag->SetString(d4::user, username);
									if (outCat.GetPropertyAsString(kpassword, nil, password))
										bag->SetString(d4::password, password);
									bool exists = false;
									sLONG remoteSessionLength = outCat.GetPropertyAsLong(ktimeout, nil, &exists); // in minutes
									if (exists)
										bag->SetLong(ktimeout, remoteSessionLength);
									const_cast<VValueBag&>(bagEntities).AddElement(d4::outsideCatalogs, bag);
								}
							}
						}
					}
				}
				else
				{
					VJSObject dataClassObj(dataClassVal.GetObject());
					VValueBag* dataClassBag = nil;
					bool mustaddclass = false;
					bool allEmpty = true;

					bagsMap::iterator found = classesBags.find(propName);
					if (found != classesBags.end())
					{
						dataClassBag = found->second;
						allEmpty = false;
					}
					else
					{
						mustaddclass = true;
						dataClassBag = new VValueBag();
					}
					dataClassBag->SetString(d4::name, propName);
					dataClassBag->SetString(d4::className, propName);

					if (mustaddclass)
					{
						dataClassBag->SetBool(d4::noEdit, true);
						dataClassBag->SetBool(d4::noSave, true);
					}

					VJSValue dataClassPropVal(dataClassObj.GetProperty("properties"));
					if (dataClassPropVal.IsObject())
					{
						VJSObject dataClassProp(dataClassPropVal.GetObject());
						VString s;
						if (dataClassProp.GetPropertyAsString("collectionName", nil, s))
						{
							dataClassBag->SetString(d4::collectionName, s);
						}
						if (s.IsEmpty())
							dataClassBag->SetString(d4::collectionName, propName+"Collection");

						if (dataClassProp.GetPropertyAsString("singleEntityName", nil, s))
						{
							dataClassBag->SetString(d4::singleEntityName, s);
						}

						if (dataClassProp.GetPropertyAsString("scope", nil, s))
						{
							dataClassBag->SetString(d4::scope, s);
						}

						if (dataClassProp.GetPropertyAsString("extends", nil, s))
						{
							dataClassBag->SetString(d4::extends, s);
							allEmpty = false;
						}

						bool exists = false;
						bool res = dataClassProp.GetPropertyAsBool("publishAsJSGlobal", nil, &exists);
						if (exists)
							dataClassBag->SetBool(d4::publishAsJSGlobal, res);

						res = dataClassProp.GetPropertyAsBool("allowOverrideStamp", nil, &exists);
						if (exists)
							dataClassBag->SetBool(d4::allowOverrideStamp, res);

						sLONG ll;
						ll = dataClassProp.GetPropertyAsLong("defaultTopSize", nil, &exists);
						if (exists)
							dataClassBag->SetLong(d4::defaultTopSize, ll);

						VJSValue restrictingQueryVal(dataClassProp.GetProperty("restrictingQuery"));
						if (!restrictingQueryVal.IsNull())
						{
							VValueBag* QueryBag = nil;
							if (restrictingQueryVal.IsString())
							{
								restrictingQueryVal.GetString(s);
								QueryBag = new VValueBag();
								QueryBag->SetString(d4::queryStatement, s);
							}
							else if (restrictingQueryVal.IsObject())
							{
								VJSObject restrictingQueryobj(restrictingQueryVal.GetObject());
								if (restrictingQueryobj.GetPropertyAsString("queryStatement", nil, s))
								{
									QueryBag = new VValueBag();
									QueryBag->SetString(d4::queryStatement, s);
									ll = restrictingQueryobj.GetPropertyAsLong("top", nil, &exists);
									if (exists)
										QueryBag->SetLong(d4::top, ll);
								}
							}
							if (QueryBag != nil)
							{
								dataClassBag->AddElement("restrictingQuery", QueryBag);
								QueryBag->Release();
							}
						}

						VJSValue primkeyval(dataClassProp.GetProperty("key"));
						if (primkeyval.IsArray())
						{
							VJSArray primkeyarr(primkeyval, false);
							for (sLONG i = 0, nb = (sLONG)primkeyarr.GetLength(); i < nb; ++i)
							{
								VJSValue val1(primkeyarr.GetValueAt(i));
								if (val1.IsString())
								{
									allEmpty = false;
									val1.GetString(s);
									VValueBag* keybag = new VValueBag();
									keybag->SetString(d4::name, s);
									dataClassBag->AddElement(d4::key, keybag);
									keybag->Release();
								}
							}
						}
						else if (primkeyval.IsString())
						{
							allEmpty = false;
							primkeyval.GetString(s);
							VValueBag* keybag = new VValueBag();
							keybag->SetString(d4::name, s);
							dataClassBag->AddElement(d4::key, keybag);
							keybag->Release();
						}
					}
					
					VJSPropertyIterator curatt(dataClassObj);
					while (curatt.IsValid())
					{
						VString attname;
						curatt.GetPropertyName(attname);
						if (attname.EqualToUSASCIICString("properties") /*|| attname.EqualToUSASCIICString("attributes")*/ || attname.EqualToUSASCIICString("methods") 
							|| attname.EqualToUSASCIICString("collectionMethods") || attname.EqualToUSASCIICString("entityMethods") || attname.EqualToUSASCIICString("events"))
						{
						}
						else
						{
							bool empty = true;
							VJSValue	tmpVal(curatt.GetProperty());
							_addOneAttribute(tmpVal, dataClassBag, &attname, empty, forARemoteCatalog);
							allEmpty = allEmpty && empty;
						}
						++curatt;
					}


					if (mustaddclass)
					{
						if (!allEmpty)
						{
							const_cast<VValueBag&>(bagEntities).AddElement(d4::dataClasses, dataClassBag);
						}
						dataClassBag->Release();
					}
				}

			}

			++curprop;
		}


	}

	jscontext.GetGlobalObjectPrivateInstance()->SetSpecific( 'db4x', (void*)nil, VJSSpecifics::DestructorVoid);

	return err;
}



VError EntityModelCatalog::addJSExtensions(Base4D* associate, VFile* fileJS, ModelErrorReporter* errorReporter)
{
	VError err = VE_OK;


	if (fileJS != nil)
	{
		ModelJSTransformer jstrans(fAssociatedBase);

		VJSContext jscontext( jstrans.GetJSGlobalContext());
		VJSObject globalObject( jscontext.GetGlobalObject());

		fLoadingContext = &jscontext;
		fLoadingGlobalObject = &globalObject;

		set<EntityModel*> alreadyProcessed;

		VValueBag bagEntities;

		for (EntityModelMap::iterator cur = fEntityModels.begin(), end = fEntityModels.end(); cur != end; ++cur)
		{
			VValueBag* ebag = new VValueBag();
			EntityModel* em = cur->second;
			ebag->SetString(d4::name, em->GetName());
			ebag->SetString(d4::className, em->GetName());
			ebag->SetString(d4::collectionName, em->GetCollectionName());
			ebag->SetString(d4::scope, EScopeKinds[(sLONG)em->getScope()]);
			bagEntities.AddElement(d4::dataClasses, ebag);
			ebag->Release();
		}

		err = jstrans.TransformJSIntoBag(*fileJS, "model", bagEntities, nil, errorReporter, true);
#if debuglr
		VString modelstr;
		bagEntities.GetJSONString(modelstr);
		VJSONValue val;
		val.ParseFromString(modelstr);
		sLONG xdebug = 1;
#endif
		if (err == VE_OK || errorReporter != nil)
		{
			if (errorReporter != nil)
			{
				if (jstrans.ParsingJSError(fParseFileError, fParseMessageError, fParseLineError))
				{
					fParseUserDefined = true;
					fSomeErrorsInCatalog = true;
				}
			}

			/*
			if (err == VE_OK && fileJS->Exists())
			{
				IDB4D_ApplicationIntf *applicationIntf = VDBMgr::GetManager()->GetApplicationInterface();
				if (applicationIntf != nil)
				{
					VFilePath path;
					fileJS->GetPath(path);
					RIApplicationRef projectref = associate->GetRiaServerProjectRef();
					if (projectref != nil)
						applicationIntf->AppendJSContextRequiredScript(projectref, path);
				}
			}
			*/

			const VBagArray* entities = bagEntities.GetElements(d4::dataClasses);
			if (entities != nil)
			{
				VIndex nbentities = entities->GetCount();
				for (VIndex i = 1; i <= nbentities && err == VE_OK; i++)
				{
					const VValueBag* OneEntity = entities->GetNth(i);
					if (OneEntity != nil)
					{
						VString name;
						OneEntity->GetString(d4::name, name);
						if (name.IsEmpty())
							OneEntity->GetString(d4::className, name);
						EntityModel* em = RetainEntity(name);
						if (em != nil)
						{
							alreadyProcessed.insert(em);
							err = em->FromBag(OneEntity, this, errorReporter, true, true);
						}
						else
						{
							VString extendFrom;
							OneEntity->GetString(d4::extends, extendFrom);
							if (extendFrom.IsEmpty())
							{
								err = ThrowBaseError(VE_DB4D_CANNOT_CREATE_A_REMOTE_CLASS, name);
							}
							else
							{
								EntityModel* baseem = RetainEntity(extendFrom);
								if (baseem == nil)
								{
									if (errorReporter == nil)
										err = ThrowBaseError(VE_DB4D_ENTITY_NOT_FOUND_FOR_EXTENDS, extendFrom);
									else
									{
										ModelErrorContextInstaller errs(errorReporter);
										ThrowBaseError(VE_DB4D_ENTITY_NOT_FOUND_FOR_EXTENDS, extendFrom);
									}
								}
								else
								{
									//em = baseem->Extends(OneEntity);
									em = NewModel();
									alreadyProcessed.insert(em);
									err = em->FromBag(OneEntity, this, errorReporter, false, false);
									em->Retain();
									fEntityModels[name] = em;
									const VString& collectionName = em->GetCollectionName();
									fEntityModelsByCollectionName[collectionName] = em;
									QuickReleaseRefCountable(baseem);
								}
							}
						}
						QuickReleaseRefCountable(em);
					}
				}
			}

		}

		if (err == VE_OK)
		{
			for (EntityModelMap::iterator cur = fEntityModels.begin(), end = fEntityModels.end(); cur != end && err == VE_OK; ++cur)
			{
				EntityModel* em = cur->second;
				if (alreadyProcessed.find(em) == alreadyProcessed.end())
				{
					err = em->addJSExtensions(this, errorReporter, true, true);
				}
			}
		}

		fLoadingContext = nil;
		fLoadingGlobalObject = nil;
	}

	return err;
}


ModelErrorReporter* EntityModelCatalog::CreateErrorReporter()
{
	if (fErrorReporter != nil)
		delete fErrorReporter;
	fErrorReporter = new ModelErrorReporter(this);
	return fErrorReporter;
}


VError EntityModelCatalog::LoadEntityModels(const VValueBag& bagEntities, ModelErrorReporter* errorReporter, unordered_map_VString<VRefPtr<VFile> >* outIncludedFiles, bool allowFolderParsing)
{
	VError err = VE_OK;

	{
		ModelJSTransformer jstrans(fAssociatedBase);

		VJSContext jscontext( jstrans.GetJSGlobalContext());
		VJSObject globalObject( jscontext.GetGlobalObject());

		fLoadingContext = &jscontext;
		fLoadingGlobalObject = &globalObject;

		bool allowGlobalPublish = false;
		if (bagEntities.GetBool(d4::publishAsJSGlobal, allowGlobalPublish))
		{
			fPublishDataClassesAsGlobals = allowGlobalPublish;
			fPublishDataClassesAsGlobalsDefined = true;
		}
		else
		{
			fPublishDataClassesAsGlobals = false;
			fPublishDataClassesAsGlobalsDefined = false;
		}

		VFolder* folderToScan = fModelFolder;
		if (folderToScan == nil)
			folderToScan = fAssociatedBase->GetStructFolder();
		if (allowFolderParsing && folderToScan != nil)
		{
			VFileIterator iter(folderToScan, FI_NORMAL_FILES | FI_WANT_INVISIBLES | FI_ITERATE_DELETE);
			bool cont = true;
			while (iter.IsValid() && cont)
			{
				VFile* file = iter.Current();
				VString ext;
				file->GetExtension(ext);
				if (ext == "waRemoteConfig")
				{
					StErrorContextInstaller errs(false, false);
					VJSONValue val;
					err = val.LoadFromFile(file);
					if (err == VE_OK && val.IsObject())
					{
						VJSONObject* cat = val.GetObject();
						VJSONValue valAccess = cat->GetProperty(kremoteAccess);
						if (valAccess.IsObject())
						{
							bool merge = false;
							valAccess.GetObject()->GetPropertyAsBool("mergeWithMain", &merge);
							if (merge)
							{
								VString s;
								valAccess.Stringify(s);
								VValueBag* catbag = new VValueBag();
								err = catbag->FromJSONString(s);
								if (err == VE_OK)
								{
									VString catname;
									file->GetNameWithoutExtension(catname);
									catbag->SetString(d4::name, catname);
									catbag->SetString("catalogName", catname);
								}
								((VValueBag&)bagEntities).AddElement(d4::outsideCatalogs, catbag);
								catbag->Release();
							}
						}
					}
				}
				++iter;
			}
		}

		VFile* fileJs = RetainRefCountable(fJSFile);
		if (fileJs == nil)
			fileJs = fAssociatedBase->RetainCatalogJSFile();
		if (fileJs != nil)
		{
			err = jstrans.TransformJSIntoBag(*fileJs, "model", (VValueBag&) bagEntities, outIncludedFiles, errorReporter, false);
			if (err == VE_OK || errorReporter != nil)
			{
				if (errorReporter != nil)
				{
					if (jstrans.ParsingJSError(fParseFileError, fParseMessageError, fParseLineError))
					{
						fParseUserDefined = true;
						fSomeErrorsInCatalog = true;
					}
				}
			}
			fileJs->Release();
		}



		if (err == VE_OK)
		{
			QuickReleaseRefCountable(fRemoteAccess);
			fRemoteAccess = bagEntities.RetainUniqueElement(kremoteAccess);
			QuickReleaseRefCountable(fExtraProperties);
			fExtraProperties = bagEntities.RetainUniqueElement(d4::extraProperties);
			const VBagArray* enums = bagEntities.GetElements(d4::enumeration);
			if (enums != nil)
			{
				VIndex nbenums = enums->GetCount();
				for (VIndex i = 1; i <= nbenums && err == VE_OK; i++)
				{
					const VValueBag* OneEnum = enums->GetNth(i);
					if (OneEnum != nil)
					{
						EmEnum* newEnum = BuildEnumeration(OneEnum, err, errorReporter);
						if (newEnum != nil)
							err = AddOneEnumeration(newEnum, errorReporter);
						QuickReleaseRefCountable(newEnum);
					}
				}
			}
		}

		if (err == VE_OK)
		{
			const VBagArray* types = bagEntities.GetElements(d4::type);
			if (types != nil)
			{
				VIndex nbtypes = types->GetCount();
				for (VIndex i = 1; i <= nbtypes && err == VE_OK; i++)
				{
					const VValueBag* OneType = types->GetNth(i);
					if (OneType != nil)
					{
						AttributeType* newType = BuildType(OneType, err, errorReporter);
						if (newType != nil)
							err = AddOneType(newType, errorReporter);
						QuickReleaseRefCountable(newType);
					}
				}
			}
		}

		if (err == VE_OK)
		{
			const VBagArray* entities = bagEntities.GetElements(d4::dataClasses);
			if (entities != nil)
			{
				VIndex nbentities = entities->GetCount();
				for (VIndex i = 1; i <= nbentities && err == VE_OK; i++)
				{
					const VValueBag* OneEntity = entities->GetNth(i);
					if (OneEntity != nil)
					{
						EntityModel* newEntity = BuildEntityModel(OneEntity, err, errorReporter);
						if (newEntity != nil)
							err = AddOneEntityModel(newEntity, false);
						QuickReleaseRefCountable(newEntity);
					}
				}
			}
		}


		if (err == VE_OK)
		{
			VTaskLock lock(&fEntityModelMutex);
			/*
			for (EntityRelationMap::iterator cur = fEntityRelations.begin(), end = fEntityRelations.end(); cur != end && err == VE_OK; cur++)
			{
				err =cur->second->ResolveMissingTables(this);
			}
			*/

			for (EntityModelMap::iterator cur = fEntityModels.begin(), end = fEntityModels.end(); cur != end && err == VE_OK; cur++)
			{
				SubPathRefSet already;
				err =cur->second->ResolveRelatedEntities(already, this, errorReporter, jstrans.GetBaseContext());
			}

			/*
			for (EntityModelMap::iterator cur = fEntityModels.begin(), end = fEntityModels.end(); cur != end && err == VE_OK; cur++)
			{
				err =cur->second->ResolveRelatedPath(this);
			}
			*/

			for (EntityModelMap::iterator cur = fEntityModels.begin(), end = fEntityModels.end(); cur != end && err == VE_OK; cur++)
			{
				err =cur->second->ResolveQueryStatements(this, errorReporter, jstrans.GetBaseContext());
			}
		}

		if (err == VE_OK)
		{
			const VBagArray* outsideCatalogs = bagEntities.GetElements(d4::outsideCatalogs);
			if (outsideCatalogs != nil)
			{
				VIndex nbcatalogs = outsideCatalogs->GetCount();
				for (VIndex i = 1; i <= nbcatalogs && err == VE_OK; i++)
				{
					const VValueBag* oneCatRef = outsideCatalogs->GetNth(i);
					if (oneCatRef != nil)
					{
						if (errorReporter == nil)
						{
							err = fAssociatedBase->AddOutsideCatalog(oneCatRef);
						}
						else
						{
							ModelErrorContextInstaller errs(errorReporter);
							VError err2 = fAssociatedBase->AddOutsideCatalog(oneCatRef);
						}
					}
				}
			}

			const VBagArray* outsideSQLCatalogs = bagEntities.GetElements("outsideSQLCatalogs");
			if (outsideSQLCatalogs != nil)
			{
				VIndex nbcatalogs = outsideSQLCatalogs->GetCount();
				for (VIndex i = 1; i <= nbcatalogs && err == VE_OK; i++)
				{
					const VValueBag* oneCatRef = outsideSQLCatalogs->GetNth(i);
					if (oneCatRef != nil)
					{
						if (errorReporter == nil)
						{
							err = fAssociatedBase->AddOutsideSQLCatalog(oneCatRef);
						}
						else
						{
							ModelErrorContextInstaller errs(errorReporter);
							VError err2 = fAssociatedBase->AddOutsideSQLCatalog(oneCatRef);
						}
					}
				}
			}
		}

		if (err == VE_OK)
			err = do_LoadEntityModels(bagEntities, errorReporter, outIncludedFiles, jstrans.GetBaseContext());

		// another pass for outside catalogs


		// end of outside catalog pass

		fLoadingContext = nil;
		fLoadingGlobalObject = nil;

	}

	return err;
}

VError EntityModelCatalog::LoadEntityModels(const VFile& inFile, bool inXML, ModelErrorReporter* errorReporter, const VString* inXmlContent, unordered_map_VString<VRefPtr<VFile> >* outIncludedFiles)
{
	VError err = VE_OK;
	if (inFile.Exists())
	{
		VString allEntities;
		if (inXmlContent)
			allEntities = *inXmlContent;
		else
		{
			VFileStream input(&inFile);
			err = input.OpenReading();
			if (err == VE_OK)
			{
				err = input.GuessCharSetFromLeadingBytes(VTC_UTF_8);
				err = input.GetText(allEntities);
			}
			input.CloseReading();
		}

		if (err == VE_OK)
		{
			bool okbag = false;

			VString firstpart, secondpart, thirdpart;
			sLONG curpos = 0;
			GetNextWord(allEntities, curpos, firstpart);
			GetNextWord(allEntities, curpos, secondpart);
			if (!(firstpart == "<" && secondpart == "?xml"))
				inXML = false;

			VValueBag bagEntities;
			if (inXML)
			{
				fJsonFormat = false;
				if ( inXmlContent )
					err = LoadBagFromXML( *inXmlContent, L"EntityModelCatalog", bagEntities);
				else
					err = LoadBagFromXML(inFile, L"EntityModelCatalog", bagEntities);

				if (err == VE_OK)
					okbag = true;
			}
			else
			{
				fJsonFormat = true;
				err = bagEntities.FromJSONString(allEntities);
				if (err == VE_OK)
				{
					//SaveBagToXML(bagEntities, L"EntityModelCatalog", VFile(*fStructFolder, L"EntityModels.xml"));
					okbag = true;
				}
			}

			if (okbag)
			{
				/*
				const VValueBag* dbBag = bagEntities.RetainUniqueElement(d4::dbInfo);
				if (dbBag != nil)
				{
					err = fAssociatedBase->FromBag(*dbBag, this);
					dbBag->Release();
				}
				else
				*/
					TouchXML();
				if (err == VE_OK)
				{
					bool isremote = false;
					bagEntities.GetBool("isRemote", isremote);
					VString fileext;
					inFile.GetExtension(fileext);
					if (fileext == kWaRemoteModel)
						isremote = true;

					err = LoadEntityModels(bagEntities, errorReporter, outIncludedFiles, !isremote);
					if (err == VE_OK)
					{
						if (fileext == kWaRemoteModel)
							bagEntities.SetBool("isRemote", true);
					}
				}
				/*
				if (err == VE_OK)
				{
					err = fOwner->LoadIndicesFromBag(&bagEntities);
				}
				*/
			}
		}
	}

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOT_LOAD_ENTITY_CATALOG, noaction);

	return err;
}


VError EntityModelCatalog::SaveEntityModels(VValueBag& catalogBag, bool inXML, bool withDBInfo) const
{
	VError err = VE_OK;

	if (fPublishDataClassesAsGlobalsDefined)
	{
		catalogBag.SetBool(d4::publishAsJSGlobal, fPublishDataClassesAsGlobals);
	}

	/*
	if (withDBInfo)
	{
		VString unused;
		BagElement dbBag(catalogBag, d4::dbInfo);
		err = fOwner->SaveToBag(*dbBag, unused);
	}
	*/

	if (fRemoteAccess != nil)
	{
		catalogBag.AddElement(kremoteAccess, (VValueBag*)fRemoteAccess);
	}

	if (fExtraProperties != nil)
	{
		catalogBag.AddElement(d4::extraProperties, (VValueBag*)fExtraProperties);
	}

	if (fJsonFormat || !inXML)
		catalogBag.SetBool("toJSON", true);

	if (fAssociatedBase != nil)
	{
		VFile* structfile = fAssociatedBase->GetStructFile();
		if (structfile != nil)
		{
			VString ext;
			structfile->GetExtension(ext);
			if (ext == kWaRemoteModel)
				catalogBag.SetBool("isRemote", true);
		}
	}

	for (EmEnumMap::const_iterator cur = fEnumerations.begin(), end = fEnumerations.end(); cur != end && err == VE_OK; cur++)
	{
		EmEnum* en = cur->second;
		BagElement bag(catalogBag, d4::enumeration);
		err = en->ToBag(*bag, false, true, !inXML, true);
	}

	for (AttributeTypeMap::const_iterator cur = fTypes.begin(), end = fTypes.end(); cur != end && err == VE_OK; cur++)
	{
		AttributeType* attType = cur->second;
		if (attType->GetBaseType() == nil)
			err = attType->SaveToBag(catalogBag, !inXML);
	}

	/*
	for (EntityRelationMap::const_iterator cur = fEntityRelations.begin(), end = fEntityRelations.end(); cur != end && err == VE_OK; cur++)
	{
		EntityRelation* rel = cur->second;
		VValueBag* relbag = new VValueBag();
		err = rel->ToBag(*relbag, false, true, !inXML, true);
		if (err == VE_OK)
			catalogBag.AddElement(d4::relationship, relbag);
		relbag->Release();
	}
	*/

	for (EntityModelMap::const_iterator cur = fEntityModels.begin(), end = fEntityModels.end(); cur != end && err == VE_OK; cur++)
	{
		EntityModel* model = cur->second;
		if (model->GetBaseEm() == nil)
			err = model->SaveToBag(catalogBag, !inXML);
	}

	/*
	if (withDBInfo)
	{
		fOwner->PutIndexDefIntoBag(catalogBag);
	}
	*/

	return err;
}


VError EntityModelCatalog::SaveEntityModels(VFile& inFile, bool inXML, bool withDBInfo) const
{
	VError err = VE_OK;

	VValueBag catalogBag;

	err = SaveEntityModels(catalogBag, inXML, withDBInfo);

	if (err == VE_OK)
	{
		/*
		if (inXML)
		{
			err = SaveBagToXML(catalogBag, L"EntityModelCatalog", inFile, true);
		}
		else
		*/
		{
			err = SaveBagToJson(catalogBag, inFile, true);
		}
	}

	return err;
}


VError EntityModelCatalog::LoadEntityPermissions(const VFile& inFile, CUAGDirectory* inDirectory, bool inXML, ModelErrorReporter* errorReporter)
{
	VError err = VE_OK;
	if (inFile.Exists())
	{
		bool okbag = false;
		VValueBag bagEntities;
		if (inXML)
		{
			err = LoadBagFromXML(inFile, L"Permissions", bagEntities);
			if (err == VE_OK)
				okbag = true;
		}
		else
		{
			VFileStream input(&inFile);
			err = input.OpenReading();
			if (err == VE_OK)
			{
				VString allEntities;
				err = input.GetText(allEntities);
				if (err == VE_OK)
				{
					err = bagEntities.FromJSONString(allEntities);
					if (err == VE_OK)
					{
						okbag = true;
					}

				}
			}
			input.CloseReading();
		}

		if (okbag)
			err = LoadEntityPermissions(bagEntities, inDirectory, errorReporter);
	}

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOT_LOAD_ENTITY_CATALOG, noaction);

	return err;
}


VError EntityModelCatalog::xSetEntityPerm(CUAGDirectory* uagdir, const VValueBag::StKey key, const VValueBag* OnePerm, EntityModel* em, DB4D_EM_Perm perm)
{
	VError err = VE_OK;
	VString s;
	if (OnePerm->GetString(key, s))
	{
		VUUID gid;
		if (s == L"$all")
			gid.SetNull(true);
		else
		{
			if (!uagdir->GetGroupID(s, gid))
			{
				err = ThrowBaseError(VE_UAG_GROUPNAME_DOES_NOT_EXIST, s);
			}
		}

		if (err == VE_OK)
			err = em->SetPermission(perm, gid, false);

	}
	return err;
}

VError EntityModelCatalog::LoadEntityPermissions(const VValueBag& bagEntities, CUAGDirectory* inDirectory, ModelErrorReporter* errorReporter)
{
	VError err = VE_OK;

	CUAGDirectory* uagdir = inDirectory;

	if (uagdir == nil)
	{
		//err = fOwner->ThrowError(VE_DB4D_NO_UAGDIRECTORY, noaction);
	}
	else
	{
		const VBagArray* perms = bagEntities.GetElements(d4::allow);
		if (perms != nil)
		{
			VIndex nbperms = perms->GetCount();
			for (VIndex i = 1; i <= nbperms && err == VE_OK; i++)
			{
				const VValueBag* OnePerm = perms->GetNth(i);
				if (OnePerm != nil)
				{
					VString sAction, sGroup, sType, sResource, sGroupID;
					bool forced = false;
					OnePerm->GetString(d4::action, sAction);
					OnePerm->GetString(d4::group, sGroup);
					OnePerm->GetString(d4::groupID, sGroupID);
					OnePerm->GetString(d4::type, sType);
					OnePerm->GetString(d4::resource, sResource);
					OnePerm->GetBool(d4::temporaryForcePermissions, forced);
					PermResourceType rType = (PermResourceType)EPermResourceType[sType];
					DB4D_EM_Perm action = (DB4D_EM_Perm)EPermAction[sAction];

					CUAGGroup* group = nil;
					if (!sGroupID.IsEmpty())
					{
						VUUID xid;
						xid.FromString(sGroupID);
						group = uagdir->RetainGroup(xid);
					}
					if (group == nil)
					{
						if (!sGroup.IsEmpty())
							group = uagdir->RetainGroup(sGroup, &err);
					}
					if (group != nil)
					{
						VUUID groupID;
						group->GetID(groupID);
						switch (rType)
						{
							case perm_model:
								SetPermission(action, groupID, forced);
								break;
							case perm_dataClass:
								{
									VectorOfVString parts;
									sResource.GetSubStrings('.', parts, false, true);
									if (parts.size() == 2)
									{
										EntityModel* em = RetainEntity(parts[1]);
										if (em != nil)
										{
											err = em->SetPermission(action, groupID, forced);
											QuickReleaseRefCountable(em);
										}
									}
								}
								break;
							case perm_method:
								{
									VectorOfVString parts;
									sResource.GetSubStrings('.', parts, false, true);
									if (parts.size() == 3)
									{
										EntityModel* em = RetainEntity(parts[1]);
										if (em != nil)
										{
											EntityMethod* method = em->getMethod(parts[2]);
											if (method != nil)
											{
												err = method->SetPermission(action, groupID);
											}
											QuickReleaseRefCountable(em);
										}
									}
								}
								break;
							case perm_attribute:
								{
									VectorOfVString parts;
									sResource.GetSubStrings('.', parts, false, true);
									if (parts.size() == 3)
									{
										EntityModel* em = RetainEntity(parts[1]);
										if (em != nil)
										{
											EntityAttribute* att = em->getAttribute(parts[2]);
											if (att != nil)
											{
												err = att->SetPermission(action, groupID);
											}
											QuickReleaseRefCountable(em);
										}
									}
								}
								break;
							default:
								break;
						}
						QuickReleaseRefCountable(group);
					}
				}
			}

			if (errorReporter == nil)
			{
				for (EntityModelMap::iterator cur = fEntityModels.begin(), end = fEntityModels.end(); cur != end; ++cur)
				{
					EntityModel* em = cur->second;
					if (em != nil)
					{
						em->ResolvePermissionsInheritance(this);
					}
				}
			}
		}


	}


	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOT_LOAD_ENTITIES_PERMS, noaction);
	return err;
}


EntityModel* EntityModelCatalog::FindEntity(const VString& entityName) const
{
	VTaskLock lock(&fEntityModelMutex);
	EntityModelMap::const_iterator found = fEntityModels.find(entityName);
	if (found == fEntityModels.end())
		return nil;
	else
		return found->second;
}



EntityModel* EntityModelCatalog::FindEntityByCollectionName(const VString& entityName) const
{
	VTaskLock lock(&fEntityModelMutex);
	EntityModelMap::const_iterator found = fEntityModelsByCollectionName.find(entityName);
	if (found == fEntityModelsByCollectionName.end())
		return nil;
	else
		return found->second;
}


EntityModel* EntityModelCatalog::RetainEntity(const VString& entityName) const
{
	EntityModel* em = FindEntity(entityName);
	if (em == nil)
	{
#if AllowDefaultEMBasedOnTables
		if (entityName.GetLength() > 0 && entityName.GetUniChar(1) == kEntityTablePrefixChar)
		{
			VString tablename;
			entityName.GetSubString(2, entityName.GetLength()-1, tablename);
			Table* tt = fOwner->FindAndRetainTableRef(tablename);
			if (tt != nil)
			{
				if (!tt->GetHideInRest())
					em = EntityModel::BuildLocalEntityModel(tt);
				tt->Release();
			}
		}
#endif
	}
	else
		em->Retain();
	return em;
}



EntityModel* EntityModelCatalog::RetainEntityByCollectionName(const VString& entityName) const
{
	EntityModel* em = FindEntityByCollectionName(entityName);
	if (em != nil)
		em->Retain();
	return em;
}

/*
EntityRelation* EntityModelCatalog::FindRelation(const VString& relationName) const
{
	VTaskLock lock(&fEntityModelMutex);
	EntityRelationMap::const_iterator found = fEntityRelations.find(relationName);
	if (found == fEntityRelations.end())
		return nil;
	else
		return found->second;
}
*/


VError EntityModelCatalog::GetAllEntityModels(vector<EntityModel*>& outList) const
{
	VTaskLock lock(&fEntityModelMutex);
	VError err = VE_OK;
	outList.clear();
	try
	{
		outList.reserve(fEntityModels.size());
		for (EntityModelMap::const_iterator cur = fEntityModels.begin(), end = fEntityModels.end(); cur != end; cur++)
		{
			outList.push_back(cur->second);
		}
	}
	catch (...)
	{
		err = ThrowError(memfull, noaction);
	}

	return err;
}


VError EntityModelCatalog::RetainAllEntityModels(vector<VRefPtr<EntityModel> >& outList) const
{
	VTaskLock lock(&fEntityModelMutex);
	VError err = VE_OK;
	outList.clear();
	try
	{
		outList.reserve(fEntityModels.size());
		for (EntityModelMap::const_iterator cur = fEntityModels.begin(), end = fEntityModels.end(); cur != end; cur++)
		{
			outList.push_back(cur->second);
		}
	}
	catch (...)
	{
		err = ThrowError(memfull, noaction);
	}

	return err;
}


VError EntityModelCatalog::GetAllEntityModels(vector<CDB4DEntityModel*>& outList) const
{
	VTaskLock lock(&fEntityModelMutex);
	VError err = VE_OK;
	outList.clear();
	try
	{
		outList.reserve(fEntityModels.size());
		for (EntityModelMap::const_iterator cur = fEntityModels.begin(), end = fEntityModels.end(); cur != end; cur++)
		{
			outList.push_back(cur->second);
		}
	}
	catch (...)
	{
		err = ThrowError(memfull, noaction);
	}

	return err;
}


EntityRelation* EntityModelCatalog::FindOrBuildRelation(const EntityRelationCollection& relpath, bool nTo1)
{
	EntityRelation* result = nil;

	for (EntityRelationCollection::const_iterator cur = fRelations.begin(), end = fRelations.end(); cur != end; cur++)
	{
		const EntityRelationCollection& curpath = (*cur)->GetPath();
		if (curpath.size() == relpath.size())
		{
			if ( (nTo1 && (*cur)->GetKind() == erel_Nto1) || (/*!nTo1 && */(*cur)->GetKind() == erel_1toN) )
			{
				bool egal = true;
				for (EntityRelationCollection::const_iterator cur1 = curpath.begin(), cur2 = relpath.begin(), end1 = curpath.end(); cur1 != end1; cur1++, cur2++)
				{
					if ((*cur1)->GetSourceAtt() != (*cur2)->GetSourceAtt())
					{
						egal = false;
						break;
					}
					if ((*cur1)->GetDestAtt() != (*cur2)->GetDestAtt())
					{
						egal = false;
						break;
					}
				}

				if (egal)
				{
					result = *cur;
					break;
				}
			}
		}
	}

	if (result == nil && !relpath.empty())
	{
		result = new EntityRelation(relpath, nTo1);
		fRelations.push_back(result);
	}

	return result;
}


void EntityModelCatalog::ReversePath(const EntityRelationCollection& relpath, EntityRelationCollection& outReversePath)
{ 
	for (EntityRelationCollection::const_iterator cur = relpath.begin(), end = relpath.end(); cur != end; cur++)
	{
		const EntityRelation* rel = *cur;
		if (rel->IsSimple())
		{
			EntityRelation* reverseRel = new EntityRelation(rel->GetDestAtt(), rel->GetSourceAtt(), erel_1toN);
			outReversePath.push_back(reverseRel);
		}
		else
		{
			const EntityRelationCollection& subpath = rel->GetPath();
			EntityRelationCollection reverseSubPath;
			ReversePath(subpath, reverseSubPath);
			EntityRelation* reverseRel = new EntityRelation(reverseSubPath, erel_1toN);
			for (EntityRelationCollection::const_iterator cur1 = reverseSubPath.begin(), end1 = reverseSubPath.end(); cur1 != end1; cur1++)
				(*cur1)->Release();
			outReversePath.push_back(reverseRel);
		}
	}
}



VError EntityModelCatalog::SetPermission(DB4D_EM_Perm inPerm, const VUUID& inGroupID, bool forced)
{
	VError err = VE_OK;
	if (inPerm >= DB4D_EM_Read_Perm && inPerm <= DB4D_EM_Promote_Perm)
	{
		fPerms[inPerm] = inGroupID;
		fForced[inPerm] = forced ? 1 : 0;
	}
	else
		err = ThrowBaseError(VE_DB4D_WRONG_PERM_REF);

	return err;
}

VError EntityModelCatalog::GetPermission(DB4D_EM_Perm inPerm, VUUID& outGroupID, bool& forced) const
{
	VError err = VE_OK;
	if (inPerm >= DB4D_EM_Read_Perm && inPerm <= DB4D_EM_Promote_Perm)
	{
		outGroupID = fPerms[inPerm];
		forced = fForced[inPerm] == 1 ? true : false;
	}
	else
		err = ThrowBaseError(VE_DB4D_WRONG_PERM_REF);

	return err;
}


VError EntityModelCatalog::ResolveRelatedEntities(BaseTaskInfo* context)
{
	VError err = VE_OK;
	for (EntityModelMap::iterator cur = fEntityModels.begin(), end = fEntityModels.end(); cur != end && err == VE_OK; cur++)
	{
		SubPathRefSet already;
		err =cur->second->ResolveRelatedEntities(already, this, nil, context);
	}
	return err;
}


VError EntityModelCatalog::SetLogFile(VFile* logfile, bool append, VJSONObject* options)
{
	QuickReleaseRefCountable(fLogger);
	fLogger = new DB4DLogger(logfile, append, options);
	return VE_OK;
}

VError EntityModelCatalog::StartLogging()
{
	if (fLogger != nil)
		return fLogger->StartLogging();
	else
		return ThrowError(VE_DB4D_MISSING_CATALOG_LOG_FILE, noaction);
}

VError EntityModelCatalog::StopLogging()
{
	if (fLogger != nil)
		return fLogger->StopLogging();
	else
		return VE_OK;
}


VError EntityModelCatalog::FlushLog()
{
	if (fLogger != nil)
		return fLogger->Flush();
	else
		return VE_OK;
}

VError EntityModelCatalog::AcceptLogger(DB4DLogger* logger)
{
	CopyRefCountable(&fLogger, logger);
	return VE_OK;
}



void EntityModelCatalog::SetModelFile(const VFile* inProjectFile)
{
	if (inProjectFile != nil)
	{
		VFolder* parent = inProjectFile->RetainParentFolder();
		if (parent != nil)
		{
			VTaskLock lock(&fSubProjectsMutex);
			VString name;
			inProjectFile->GetNameWithoutExtension(name);
			VString modelname;
			GetNewJSModelName(name, modelname);
			fModelJSName = modelname;
			VFile* jsfile = new VFile(*parent, name + ".js");
			QuickReleaseRefCountable(fModelFile);
			QuickReleaseRefCountable(fModelFolder);
			QuickReleaseRefCountable(fJSFile);
			fModelFile = RetainRefCountable(const_cast<VFile*>(inProjectFile));
			fModelFolder = parent;
			fJSFile = jsfile;
		}
	}
}


EntityModelCatalog* EntityModelCatalog::AddSubProject(const VFile* inProjectFile, VError& outError)
{
	EntityModelCatalog* result = nil;
	VError err = VE_OK;
	VString path;
	inProjectFile->GetPath(path, FPS_POSIX, false);

	{
		VTaskLock lock(&fSubProjectsMutex);
		SubProjectMap::iterator found = fSubProjects.find(path);
		if (found != fSubProjects.end())
		{
			result = found->second;
		}
		else
		{
			EntityModelCatalog* subcat = new LocalEntityModelCatalog(GetAssocBase());
			subcat->SetModelFile(inProjectFile);
			VError err = subcat->LoadEntityModels(*inProjectFile, false, nil, nil, nil);
			if (err == VE_OK)
			{
				err = subcat->SecondPassLoadEntityModels(nil, true);
				fSubProjects[path] = subcat;
				result = subcat;
				VString s, s2;
				inProjectFile->GetNameWithoutExtension(s);
				s.GetCleanJSName(s2);
				GetAssocBase()->AddCatalogRef(s2, subcat);
			}
			else
				ReleaseRefCountable(&subcat);
		}

	}

	outError = err;
	return result;
}



VError EntityModelCatalog::EvaluateModelScript(const VJSObject& globalObject, BaseTaskInfo* context, void* ProjectRef, std::vector<XBOX::VFile*> &outModelJSFiles)
{
	VError err = VE_OK;
	VFile* scriptfile = RetainRefCountable(fJSFile);
	if (scriptfile == nil)
		scriptfile = GetAssocBase()->RetainCatalogJSFile();
	if (scriptfile != nil)
	{
		if (scriptfile->Exists())
		{
			VJSContext jscontext(globalObject.GetContext());
			VJSValue result(jscontext);
			VJSException e, e1;
			VFolder* ResourceFolder = VDBMgr::GetManager()->RetainJSCodeResourceFolder();
			if (ResourceFolder != nil)
			{
				VFile libfile(*ResourceFolder, "ModelPrepare.js");
				if (libfile.Exists())
				{
					jscontext.EvaluateScript(&libfile, &result, e1, nil);
					if (!e1.IsEmpty())
					{
						VJSValue ev(jscontext, e1);
#if debuglr
						VJSJSON json(jscontext);
						VString s;
						json.Stringify(ev, s);
						sLONG xdebug = 1; // put a break here
#endif						
					}
				}
			}

			outModelJSFiles.push_back(RetainRefCountable(scriptfile));
			jscontext.EvaluateScript(scriptfile, &result, e1, nil);
			if (!e1.IsEmpty())
			{
				VJSValue ev(jscontext, e1);
#if debuglr
				VJSJSON json(jscontext);
				VString s;
				json.Stringify(ev, s);
				sLONG xdebug = 1; // put a break here
#endif						
			}

			VString finalpart = fModelJSName + " = model;";
			jscontext.EvaluateScript(finalpart, nil, &result, e1, nil);
		}

		scriptfile->Release();
	}

	vector<EntityModelCatalog*> subcats;
	{
		VTaskLock lock(&fSubProjectsMutex);
		subcats.reserve(fSubProjects.size());
		for (SubProjectMap::iterator cur = fSubProjects.begin(), end = fSubProjects.end(); cur != end; ++cur)
		{
			subcats.push_back(cur->second);
		}
	}
	for (vector<EntityModelCatalog*>::iterator cur = subcats.begin(), end = subcats.end(); cur != end; ++cur)
	{
		(*cur)->EvaluateModelScript(globalObject, context, ProjectRef, outModelJSFiles);
	}

	return err;
}




EntityMethod* EntityModelCatalog::getMethod(const VString& inMethodName, bool publicOnly) const
{
	EntityMethodMap::const_iterator found = fMethodsByName.find(inMethodName);
	if (found == fMethodsByName.end())
		return nil;
	else
	{
		EntityMethod* result = found->second;
		if (publicOnly && result->GetScope() != escope_public)
			result = nil;
		return result;
	}
}





ModelCatalogFactoryMap EntityModelCatalog::sModelFactories;

void EntityModelCatalog::RegisterFactory(sLONG signature, ModelCatalogFactory code)
{
	sModelFactories[signature] = code;
}


ModelCatalogFactory EntityModelCatalog::GetFactory(sLONG signature)
{
	ModelCatalogFactoryMap::iterator found = sModelFactories.find(signature);
	if (found == sModelFactories.end())
		return nil;
	else
		return found->second;
}

EntityModelCatalog* EntityModelCatalog::NewCatalog(sLONG signature, Base4D* inOwner, VJSONObject* params, VError& outErr)
{
	EntityModelCatalog* result = nil;
	ModelCatalogFactory code = GetFactory(signature);
	if (code != nil)
		result = (*code)(inOwner, params, outErr);
	else
		outErr = VE_DB4D_NOT_FOUND;
	return result;
}


set<VString> EntityModelCatalog::sCatalogNames;
VCriticalSection EntityModelCatalog::sCatalogNamesMutex;

void EntityModelCatalog::GetNewJSModelName(const VString& hint, VString& outName)
{
	bool again = true;
	outName.Clear();
	hint.GetCleanJSName(outName);

	if (outName.IsEmpty())
		outName = "component_model";
	sLONG iter = 0;

	VTaskLock lock(&sCatalogNamesMutex);
	while (again)
	{
		VString s = outName;
		if (iter != 0)
			s += ToString(iter);
		if (sCatalogNames.find(s) == sCatalogNames.end())
		{
			again = false;
			outName = s;
			sCatalogNames.insert(s);
		}
		++iter;
	}
}


void EntityModelCatalog::ReleaseJSModelName(const VString& modelname)
{
	VTaskLock lock(&sCatalogNamesMutex);
	sCatalogNames.erase(modelname);
}


EntityCollection* EntityModelCatalog::BuildCollectionFromJSON(const VJSONObject* from, VError& outErr)
{
	EntityCollection* result = nil;
	outErr = VE_OK;
	VString emname;
	from->GetPropertyAsString("emName", emname);
	EntityModel* em = FindEntity(emname);
	if (em == nil)
	{
		outErr = ThrowBaseError(VE_DB4D_ENTITY_ATTRIBUTE_NOT_FOUND);
	}
	else
	{
		result = em->BuildCollectionFromJSON(from, outErr);
	}

	return result;
}




