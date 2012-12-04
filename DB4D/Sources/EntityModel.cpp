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


namespace d4
{
	CREATE_BAGKEY(allow);
	CREATE_BAGKEY(publishAsJSGlobal);
	CREATE_BAGKEY(allowOverrideStamp);
}


DataSet::~DataSet()
{
	QuickReleaseRefCountable(fSel);
	QuickReleaseRefCountable(fTable);
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


VError EmEnum::FromBag(const VValueBag* bag, EntityModelCatalog* catalog, bool devMode)
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

AttributeType::AttributeType(Base4D* owner)
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


VError AttributeType::FromBag(const VValueBag* bag, EntityModelCatalog* catalog, bool fullyLoad, bool devMode)
{
	VError err = VE_OK;

	fExtraProperties = bag->RetainUniqueElement(d4::extraProperties);

	if (fullyLoad)
	{
		bag->GetString(d4::name, fName);
		if (bag->GetString(d4::extends, fExtends))
		{
			fScalarType = EValPredefinedTypes[fExtends];
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
			if (!devMode)
				err = ThrowError(VE_DB4D_ENUMERATION_DOES_NOT_EXIST, &enumname);
			else
				catalog->AddError();
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
			EmEnum* en = catalog->BuildEnumeration(enumbag, err, devMode);
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

	fSource = fPath[0]->GetSourceField();
	fDest = fPath[fPath.size()-1]->GetDestField();
	if (nTo1)
		fType = erel_Nto1;
	else
		fType = erel_1toN;
}


VError EntityRelation::ThrowError( VError inErrCode, const VString* p1) const
{
	VErrorDB4D_OnEMRelation *err = new VErrorDB4D_OnEMRelation(inErrCode, noaction, fSource == nil ? nil : fSource->GetOwner()->GetOwner(), this);
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
			if ((*cur)->fSource != (*curOther)->fSource || (*cur)->fDest != (*curOther)->fDest)
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
}


EntityMethod* EntityMethod::Clone(EntityModel* inModel) const
{
	EntityMethod* result = new EntityMethod(inModel);
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
	return result;
}


CDB4DEntityModel* EntityMethod::GetModel() const
{
	return fOwner;
}



VError EntityMethod::ThrowError( VError inErrCode, const VString* p1) const
{
	
	VErrorDB4D_OnEMMethod *err = new VErrorDB4D_OnEMMethod(inErrCode, noaction, fOwner->GetOwner(), fOwner, this);
	if (p1 != nil)
		err->GetBag()->SetString(Db4DError::Param1, *p1);
	VTask::GetCurrent()->PushRetainedError( err);
	return inErrCode;
}


VError EntityMethod::FromJS(const VString& name, EntityMethodKind kind, const VString& from, EntityAttributeScope scope, EntityModelCatalog* catalog, bool devMode)
{
	VError err = VE_OK;

	fUserDefined = true;
	fName = name;
	fKind = kind;
	fFrom = from;
	fScope = scope;

	return err;
}


VError EntityMethod::FromBag(const VValueBag* bag, EntityModelCatalog* catalog, bool devMode)
{
	VError err = VE_OK;

	if (! bag->GetString(d4::name, fName))
	{
		if (!devMode)
			err = fOwner->ThrowError(VE_DB4D_ENTITY_METHOD_NAME_MISSING);
		else
			catalog->AddError();
	}

	sLONG xtype;
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
				if (devMode)
				{
					xtype = emeth_rec;
					catalog->AddError();
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
					if (!devMode)
						err = ThrowError(VE_DB4D_METHOD_PARAMETER_NAME_IS_INVALID);
					else
						catalog->AddError();
				}
			}
			else
			{
				if (!devMode)
					err = ThrowError(VE_DB4D_METHOD_PARAMETER_NAME_IS_INVALID);
				else
					catalog->AddError();
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
				if (!devMode)
					err = ThrowError(VE_DB4D_METHOD_STATEMENT_IS_EMPTY);
				else
					catalog->AddError();
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


VError EntityMethod::ResolveType(EntityModelCatalog* catalog, bool devMode)
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


bool EntityMethod::PermissionMatch(DB4D_EM_Perm inPerm, CUAGSession* inSession) const
{
	return permissionMatch(inPerm, inSession);
}


bool EntityMethod::permissionMatch(DB4D_EM_Perm inPerm, CUAGSession* inSession) const
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
			ok = inSession->BelongsTo(*id);
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
		BagElement scriptbag(outBag, d4::source);
		scriptbag->SetCData(fStatement);
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
				result.GetObject(outObjFunc);
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
						VJSValue glueresult(jscontext);
						bool okGlue = false;
						JSEntityMethodReference glueref;
						if (!context->GetJSMethod(glueref, glueFunc))
						{
							VFolder* compfolder = VDBMgr::GetManager()->RetainResourceFolder();
							if (compfolder != nil)
							{
								VFile scriptfile(*compfolder, "methodCall.js");
								if (scriptfile.Exists())
								{
									StErrorContextInstaller errs(false);
									VJSValue xresult(jscontext);
									jscontext.EvaluateScript(&scriptfile, &xresult, nil, nil);
									if (!result.IsUndefined() && result.IsObject())
									{
										xresult.GetObject(glueFunc);
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
						jscontext.GetGlobalObject().CallFunction(glueFunc, &params, &newresult, nil);

						if (!newresult.IsUndefined() && newresult.IsObject())
						{
							newresult.GetObject(outObjFunc);
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


VError EntityMethod::Execute(Selection* inSel, const vector<VJSValue>* inParams, BaseTaskInfo* context, VJSValue& outResult) const
{
	VError err = VE_OK;
	if (okperm(context, fExecutePerm))
	{
		if (context->GetJSContext() != nil)
		{
			VJSContext jscontext(context->GetJSContext());
			CDB4DBase* xbase = fOwner->GetOwner()->RetainBaseX();
			if (inSel != nil)
				inSel->Retain();
			CDB4DSelection* xsel = new VDB4DSelection(VDBMgr::GetManager(), VImpCreator<VDB4DBase>::GetImpObject(xbase), fOwner->GetMainTable(), inSel);
			xbase->Release();
			xsel->SetAssociatedModel(fOwner);

			VJSObject therecord(jscontext, VJSEntitySelection::CreateInstance(jscontext, xsel));
			xsel->Release();

			VJSObject* objfunc = nil;
			VJSObject localObjFunc(jscontext);
			if (GetScriptObjFunc(context->GetJSContext(), context, localObjFunc))
				objfunc = &localObjFunc;
			if (objfunc == nil)
				objfunc = context->getContextOwner()->GetJSFunction(fScriptNum, fStatement, &fParams);

			JS4D::ExceptionRef excep;
			if (objfunc != nil)
			{
				therecord.CallFunction(*objfunc, inParams, &outResult, &excep);
			}
			else
			{
				err = ThrowError(VE_DB4D_JS_ERR);
			}

			if (excep != nil)
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
			VJSObject therecord(jscontext, VJSEntitySelectionIterator::CreateInstance(jscontext, new EntitySelectionIterator(inRec, context->GetEncapsuleur())));
			VJSObject* objfunc = nil;
			VJSObject localObjFunc(jscontext);
			if (GetScriptObjFunc(context->GetJSContext(), context, localObjFunc))
				objfunc = &localObjFunc;
			if (objfunc == nil)
				objfunc = context->getContextOwner()->GetJSFunction(fScriptNum, fStatement, &fParams);

			JS4D::ExceptionRef excep;
			if (objfunc != nil)
			{
				therecord.CallFunction(*objfunc, inParams, &outResult, &excep);
			}
			else
				err = ThrowError(VE_DB4D_JS_ERR);

			if (excep != nil)
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
			VJSObject therecord(jscontext, VJSEntityModel::CreateInstance(jscontext, fOwner));
			VJSObject* objfunc = nil;
			VJSObject localObjFunc(jscontext);
			if (GetScriptObjFunc(context->GetJSContext(), context, localObjFunc))
				objfunc = &localObjFunc;
			if (objfunc == nil)
				objfunc = context->getContextOwner()->GetJSFunction(fScriptNum, fStatement, &fParams);

			JS4D::ExceptionRef excep;
			if (objfunc != nil)
			{
				therecord.CallFunction(*objfunc, inParams, &outResult, &excep);
			}
			else
				err = ThrowError(VE_DB4D_JS_ERR);

			if (excep != nil)
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


EntityAttribute::EntityAttribute(EntityModel* owner)
{
	fOwner = owner;
	fFrom = nil;
	fRelationPathID = -1;
	fPosInOwner = 0;
	fFieldPos = 0;
	fKind = eattr_none;
	fScope = escope_public;
	fSubEntity = nil;
	fFlattenTableDest = nil;
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
	fFlattenFromAField = true;
	fIsMultiLine = false;
	std::fill(&fScriptUserDefined[0], &fScriptUserDefined[script_attr_last+1], 0);
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
	result->fFlattenTableDest = fFlattenTableDest;
	result->fFlattenColumnName = fFlattenColumnName;
	result->fFlattenFromAField = fFlattenFromAField;
	result->fRelation = RetainRefCountable(fRelation);
	result->fType = RetainRefCountable(fType);
	result->fCrit_UUID = fCrit_UUID;
	result->fIdentifying = fIdentifying;
	result->fIsMultiLine = fIsMultiLine;
	if (fLocalType == nil)
		result->fLocalType = nil;
	else
		result->fLocalType = fLocalType->Clone();
	if (fScriptQuery == nil)
		result->fScriptQuery = nil;
	else
	{
		result->fScriptQuery = new SearchTab(inModel->GetMainTable());
		result->fScriptQuery->From(*fScriptQuery);
	}
	if (fScriptDB4D == nil)
		result->fScriptDB4D = nil;
	else
	{
		result->fScriptDB4D = new SearchTab(inModel->GetMainTable());
		result->fScriptDB4D->From(*fScriptDB4D);
	}
	result->fScriptKind = fScriptKind;
	result->fIsForeignKey = fIsForeignKey;
	copy(&fScriptStatement[0], &fScriptStatement[script_attr_last+1], &(result->fScriptStatement[0]));
	copy(&fScriptFrom[0], &fScriptFrom[script_attr_last+1], &(result->fScriptFrom[0]));
	copy(&fScriptUserDefined[0], &fScriptUserDefined[script_attr_last+1], &(result->fScriptUserDefined[0]));

	DBEventIterator curdest = &result->fEvents[dbev_firstEvent];
	for (DBEventConstIterator cur = &fEvents[dbev_firstEvent], end = &fEvents[dbev_lastEvent+1]; cur != end; cur++, curdest++)
	{
		if (cur->IsValid())
			curdest->CopyFrom(inModel, cur);
	}
	//CopyRefCountable(&(result->fExtraProperties), fExtraProperties);
	return result;
}


VError EntityAttribute::ThrowError( VError inErrCode, const VString* p1) const
{
	VErrorDB4D_OnEMAttribute *err = new VErrorDB4D_OnEMAttribute(inErrCode, noaction, fOwner->GetOwner(), fOwner, this);
	if (p1 != nil)
		err->GetBag()->SetString(Db4DError::Param1, *p1);
	VTask::GetCurrent()->PushRetainedError( err);

	return inErrCode;
}


CDB4DEntityModel* EntityAttribute::GetModel() const
{
	return fOwner;
}


CDB4DEntityModel* EntityAttribute::GetRelatedEntityModel() const
{
	return fSubEntity;
}


ValueKind EntityAttribute::GetDataKind() const
{
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
				result.GetObject(outObjFunc);
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


Field* EntityAttribute::RetainDirectField() const
{
	if (fKind == eattr_storage && fOwner->GetMainTable() != nil)
		return fOwner->GetMainTable()->RetainField(fFieldPos);
	else
		return nil;
}


Field* EntityAttribute::RetainField() const
{
	Field* result = nil;
	switch (fKind)
	{
		case eattr_storage:
			if (fOwner->GetMainTable() != nil)
				result = fOwner->GetMainTable()->RetainField(fFieldPos);
			break;

		case eattr_alias:
			if (fFlattenTableDest != nil)
				result = fFlattenTableDest->RetainField(fFieldPos);
			break;
	}
	return result;
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

	if (fKind == eattr_alias || fKind == eattr_storage || fKind == eattr_computedField)
	{
		ValueKind xtype = 0;
		Field* cri = nil;
		if (fKind == eattr_alias)
		{
			if (fFlattenTableDest != nil)
			{
				cri = fFlattenTableDest->RetainField(fFieldPos);
				xtype = fFlattenTableDest->GetFieldType(fFieldPos);
			}
		}
		else if (fKind == eattr_computedField)
		{
			xtype = fScalarType;
		}
		else
		{
			cri = fOwner->GetMainTable()->RetainField(fFieldPos);
			xtype = fOwner->GetMainTable()->GetFieldType(fFieldPos);
		}

		if (cri != nil && fKind != eattr_alias)
		{
			if (cri->IsIndexed())
				outBag.SetBool(d4::indexed, true);
		}
		if (cri != nil && forSave)
		{
			if (fKind == eattr_storage && isTableDef)
			{
				
				VUUID xid;
				cri->GetUUID(xid);
				outBag.SetVUUID(d4::uuid, xid);
				
				
				outBag.SetLong(d4::fieldPos, cri->GetPosInRec());
				Critere* xcri = cri->GetCritere();

				if (xtype == VK_TEXT || xtype == VK_TEXT_UTF8)
					outBag.SetBool(d4::textAsBlob, true);

				if (cri->GetLimitingLen() != 0 && (xtype == VK_STRING || xtype == VK_STRING_UTF8))
					outBag.SetLong(DB4DBagKeys::limiting_length, cri->GetLimitingLen());

				if (cri->getCRD()->unique)
					outBag.SetBool(DB4DBagKeys::unique, cri->getCRD()->unique);

				if (cri->getCRD()->not_null)
					outBag.SetBool(DB4DBagKeys::not_null, cri->getCRD()->not_null);

				if (cri->getCRD()->fNeverNull)
					outBag.SetBool(DB4DBagKeys::never_null, cri->getCRD()->fNeverNull);

				if (cri->getCRD()->fAutoSeq)
					outBag.SetBool(DB4DBagKeys::autosequence, cri->getCRD()->fAutoSeq);

				if (cri->getCRD()->fAutoGenerate)
					outBag.SetBool(DB4DBagKeys::autogenerate, cri->getCRD()->fAutoGenerate);

				if (xtype == VK_STRING_UTF8 || xtype == VK_TEXT_UTF8)
					outBag.SetBool(DB4DBagKeys::store_as_utf8, true);

				if ((xtype == VK_BLOB || xtype == VK_BLOB_DB4D || xtype == VK_IMAGE) && cri->GetBlobSwitchSize() != 0)
					outBag.SetLong(DB4DBagKeys::blob_switch_size, cri->GetBlobSwitchSize());

				if ((xtype == VK_TEXT_UTF8 || xtype == VK_TEXT) && cri->GetTextSwitchSize() != 0)
					outBag.SetLong(DB4DBagKeys::text_switch_size, cri->GetTextSwitchSize());

				if (cri->getCRD()->fOuterData)
					outBag.SetBool(DB4DBagKeys::outside_blob, cri->getCRD()->fOuterData);

				if (cri->getCRD()->fOuterData)
					outBag.SetBool(DB4DBagKeys::styled_text, cri->getCRD()->fTextIsWithStyle);


			}
			else
			{
				VString columnname;
				cri->GetName(columnname);
				outBag.SetString(d4::columnName, columnname);
			}
		}

		if (xtype == VK_STRING_UTF8 || xtype == VK_TEXT || xtype == VK_TEXT_UTF8)
			xtype = VK_STRING;

		QuickReleaseRefCountable(cri);
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
			outBag.SetVUUID(d4::uuid, fCrit_UUID);
			outBag.SetLong(d4::fieldPos, GetFieldPos());
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

	for (DBEventConstIterator cur = &fEvents[dbev_firstEvent], end = &fEvents[dbev_lastEvent+1]; cur != end && err == VE_OK; cur++)
	{
		if (cur->IsValid() && (cur->IsOverWritten() || !forSave))
		{
			BagElement evBag(outBag, d4::events);
			err = cur->ToBag(*evBag, forDax, forSave, forJSON, true);
		}
	}

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


VError EntityAttribute::GetScriptFromBag(const VValueBag* bag, EntityModelCatalog* catalog, const VValueBag::StKey& inScriptKey, script_attr inWhatScript, bool devMode)
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
				if (!devMode)
					err = ThrowError(VE_DB4D_SCRIPT_STATEMENT_IS_EMPTY);
				else
					catalog->AddError();
			}
		}
	}
	return err;
}


VError EntityAttribute::FromBag(const VValueBag* bag, EntityModelCatalog* catalog, bool fieldmustexits, bool devMode)
{
	VError err = VE_OK;
	bool uuidWasGenerated = false;

	fExtraProperties = bag->RetainUniqueElement(d4::extraProperties);
	
	VUUID xid;
	if (!bag->GetVUUID(d4::uuid, xid))
	{
		xid.Regenerate();
		uuidWasGenerated = true;
	}
	fCrit_UUID = xid;
	
	if (! bag->GetString(d4::name, fName))
	{
		if (!devMode)
			err = fOwner->ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_NAME_MISSING);
		else
			catalog->AddError();
	}

	sLONG xtype = (sLONG)fKind;
	VString kindname;
	if (err == VE_OK)
	{
		if (! bag->GetString(d4::kind, kindname))
		{
			if (fKind == eattr_none)
			{
				if (devMode)
				{
					xtype = 1;
					catalog->AddError();
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
				if (devMode)
				{
					xtype = 1;
					catalog->AddError();
				}
				else
					err = ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_TYPE_MISSING);
			}
		}
	}

	EntityAttributeKind xkind = (EntityAttributeKind)xtype;
	/*
	if (xkind == eattr_field)
		xkind = eattr_storage;
		*/

	bag->GetString(d4::indexKind, fIndexKind);
	fIdentifying = false;
	bag->GetBool(d4::identifying, fIdentifying);

	

	if (err == VE_OK)
	{
		if (xkind == eattr_relation_Nto1 || xkind == eattr_relation_1toN || xkind == eattr_alias || xkind == eattr_composition)
		{
			VString relpath;
			if (bag->GetString(d4::path, relpath))
			{
				fRelPath = relpath;
			}
		}
		else if (xkind == eattr_computedField)
		{
			err = GetScriptFromBag(bag, catalog, d4::script, script_attr_get, devMode);
			err = GetScriptFromBag(bag, catalog, d4::onGet, script_attr_get, devMode);
			if (err == VE_OK)
				err = GetScriptFromBag(bag, catalog, d4::onSet, script_attr_set, devMode);
			if (err == VE_OK)
				err = GetScriptFromBag(bag, catalog, d4::onQuery, script_attr_query, devMode);
			if (err == VE_OK)
				err = GetScriptFromBag(bag, catalog, d4::onSort, script_attr_sort, devMode);

			if (err == VE_OK)
			{
				VString ss;
				if (bag->GetString(d4::scriptKind, ss))
				{
					fScriptKind = (script_type)ScriptTypes[ss];
					if (fScriptKind == script_none)
					{
						if (devMode)
						{
							fScriptKind = script_javascript;
							catalog->AddError();
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

	if (err == VE_OK)
	{
		if (xkind == eattr_storage || xkind == eattr_alias)
		{
			if (xkind == eattr_alias)
			{
				/* resolution retardee
				*/
			}

			sLONG npos = -1;
			VString fieldname;
			if (bag->GetString(d4::columnName, fieldname))
			{
				if (xkind == eattr_storage)
					npos = fOwner->GetMainTable()->FindField(fieldname);
				else
				{
					fFlattenColumnName = fieldname;
				}
				if (npos > 0)
					fFieldPos = npos;
			}

			if (npos == -1)
			{
				if (! bag->GetLong(d4::fieldPos, npos))
				{
					if (fFieldPos == 0 && fieldmustexits && xkind == eattr_storage && !devMode)
					{
						if (fieldname.IsEmpty())
							err = ThrowError(VE_DB4D_ATT_FIELD_IS_MISSING);
						else
							err = ThrowError(VE_DB4D_ATT_FIELD_NOT_FOUND, &fieldname);
					}
				}
				else
				{
					fFieldPos = npos;
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
							evkind = (DBEventKind)EDBEventKinds[ss];
							/*
							if (evkind == dbev_none)
								evkind = (DBEventKind)oldEDBEventKinds[ss];
								*/
						}
						if (evkind == dbev_none)
						{
							if (!devMode)
								err = ThrowError(VE_DB4D_MISSING_OR_INVALID_EVENTKIND);
							else
								catalog->AddError();
						}
						else
						{
							err = fEvents[evkind].FromBag(fOwner, eventBag, catalog, devMode);
						}
					}
				}
			}
		}
	}

	if (err == VE_OK)
	{
		if (xkind == eattr_relation_Nto1 || xkind == eattr_relation_1toN || xkind == eattr_composition)
		{
			fReversePath = false;
			bag->GetBool(d4::reversePath, fReversePath);
			if (! bag->GetString(d4::type, fSubEntityName))
			{
				if (fSubEntityName.IsEmpty())
				{
					if (!devMode)
						err = ThrowError(VE_DB4D_ATT_RELATED_ENTITY_IS_MISSING);
					else
						catalog->AddError();

				}
			}
		}
		else
		{
			VString TypeName;
			if (bag->GetString(d4::type, TypeName))
			{
				sLONG scalartype = EValPredefinedTypes[TypeName];
				if (scalartype == 0)
				{
					AttributeType* atttype = catalog->FindType(TypeName);
					if (atttype == nil)
					{
						if (devMode)
						{
							fScalarType = 1;
							catalog->AddError();
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
				tempType = new AttributeType(fOwner->GetOwner());
			}
			tempType->FromBag(bag, catalog, false, devMode);
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

	if (err == VE_OK)
		fKind = xkind;

	if (err == VE_OK)
	{
		bool readonly;
		if (bag->GetBool(d4::readOnly, readonly))
			fCanBeModified = !readonly;
		else
			fCanBeModified = true;
	}

	if (err == VE_OK)
	{
		bool multiline;
		if (bag->GetBool(d4::multiLine, multiline))
			fIsMultiLine = multiline;
		else
			fIsMultiLine = false;

		if (xkind == eattr_relation_Nto1 || xkind == eattr_storage /*|| xkind == eattr_field*/)
		{
			if (uuidWasGenerated)
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
	fScope = escope_public;
	if (bag->GetString(d4::scope, ss))
	{
		fScope = (EntityAttributeScope)EScopeKinds[ss];
		if (fScope == escope_none)
			fScope = escope_public;
	}

	if (err == VE_OK)
	{
		VJSContext	jscontext(*(catalog->getLoadingContext()));
		VJSObject	result(jscontext);
		VString		path	= "model."+fOwner->GetName()+"."+fName;

		/*
		jscontext.DisableDebugger();
		jscontext.EvaluateScript(path, nil, &result, nil, nil);
		*/

		bool				isOk		= false;
		JS4D::ExceptionRef	exception	= NULL;
		
		result = jscontext.GetGlobalObject().GetPropertyAsObject("model", &exception);
		if (exception == NULL && result.IsObject()) {

			result = result.GetPropertyAsObject(fOwner->GetName(), &exception);
			if (exception == NULL && result.IsObject()) {

				result = result.GetPropertyAsObject(fName, &exception);
				isOk = exception == NULL && result.IsObject();

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
					VJSValue eventsVal(curprop.GetProperty());
					if (eventsVal.IsObject())
					{
						VJSObject eventsObj(eventsVal.GetObject());
						VJSPropertyIterator curevent(eventsObj);
						while (curevent.IsValid())
						{
							VString eventName;
							curevent.GetPropertyName(eventName);
							DBEventKind evkind = (DBEventKind)EDBEventKinds[eventName];
							if (evkind != dbev_none)
							{
								VJSValue eventFunc(curevent.GetProperty());
								if (eventFunc.IsFunction())
								{
									VString path2 = path + ".events."+eventName;
									fEvents[evkind].SetEvent(fOwner, evkind, path2);
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
							VJSValue funcobj(curprop.GetProperty());
							if (funcobj.IsFunction())
							{
								fScriptFrom[whichscript] = path2;
								fScriptUserDefined[whichscript] = true;
							}
						}
					}
					
				}

				++curprop;
			}

		}
	}

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOT_BUILD_EM_ATT_FROM_DEF);
	return err;
}

VError EntityAttribute::ResolveRelatedEntities(SubPathRefSet& already, EntityModelCatalog* catalog, bool devMode, BaseTaskInfo* context)
{
	VError err = VE_OK;

	if (fRelation == nil)
	{
		if (fFrom != nil)
		{
			((EntityAttribute*)fFrom)->ResolveRelatedEntities(already, catalog, devMode, context);
			fRelation = RetainRefCountable(fFrom->fRelation);
			if (fSubEntity == nil)
				fSubEntity = RetainRefCountable(fFrom->fSubEntity);
			fFlattenTableDest = fFrom->fFlattenTableDest;
			fFieldPos = fFrom->fFieldPos;
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
					if (!devMode)
						err = ThrowError(VE_DB4D_ATT_RELATED_ENTITY_NOT_FOUND);
					else
						catalog->AddError();
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
							EntityModel* otherEntity = catalog->RetainEntity(s);
							if (otherEntity == nil)
								otherEntity = catalog->RetainEntityByCollectionName(s);
							if (otherEntity != nil)
							{
								fIsForeignKey = true;
								mustBuildForeignKey = true;
								Table* towner = fOwner->GetMainTable();

								sLONG pos = GetFieldPos();
								if (pos == 0)
								{
									pos = towner->FindNextFieldFree();
									if (pos == -1)
										pos = towner->GetNbCrit()+1;
									SetFieldPos(pos);
								}

								const IdentifyingAttributeCollection* otherPrim = otherEntity->GetPrimaryAtts();
								if (otherPrim->empty())
								{
									if (!devMode)
										err = otherEntity->ThrowError(VE_DB4D_ENTITY_HAS_NO_PRIMKEY);
									else
										catalog->AddError();
								}
								else
								{
									if (otherPrim->size() != 1)
									{
										if (!devMode)
											otherEntity->ThrowError(VE_DB4D_PRIMKEY_MUST_BE_ONE_FIELD_ONLY);
										else
											catalog->AddError();
									}
									else
									{
										const EntityAttribute* otherAtt = (*otherPrim)[0].fAtt;
										sLONG typ = otherAtt->ComputeScalarType();

										Field* cri = new Field(typ, pos, towner);
										Critere* xcri = cri->GetCritere();
										xcri->SetName(fName);
										cri->SetUUID(fCrit_UUID);
										towner->AddFieldSilently(cri, pos);

										fOwner->GetOwner()->AddObjectID(objInBase_Field, cri, fCrit_UUID);


										IndexInfoFromField* ind = new IndexInfoFromField(fOwner->GetOwner(), towner->GetNum(), cri->GetPosInRec(), DB4D_Index_BtreeWithCluster, false, true);
										fOwner->GetOwner()->AddXMLIndex(ind);
										ind->Release();


										Field* dest = otherAtt->RetainDirectField();
										if (testAssert(dest != nil))
										{
											EntityRelation* rel = new EntityRelation(cri, dest, erel_Nto1);
											rel->RecalcPath();
											fRelationPathID = fOwner->AddRelationPath(rel);
											catalog->AddOneEntityRelation(rel, false);
											fRelation = rel;

											dest->Release();
										}


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
							err = fOwner->BuildRelPath(already, catalog, path, relpath, lastpart, allNto1, devMode, lastDest);

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
										if (!devMode)
											err = ThrowError(VE_DB4D_NAVIGATION_PATH_IS_MALFORMED, &fRelPath);
										else
											catalog->AddError();
									}
								}
								else
								{
									if (!devMode)
										err = ThrowError(VE_DB4D_NAVIGATION_ATTRIBUTE_MUST_BE_Nto1, &fRelPath);
									else
										catalog->AddError();
								}

							}

						}
					}
					else
					{
						if (path.empty())
						{
							if (!devMode)
								err = ThrowError(VE_DB4D_NAVIGATION_PATH_IS_EMPTY);
							else
								catalog->AddError();
						}
						else
						{
							EntityRelationCollection relpath;
							VString lastpart;
							bool allNto1 = true;
							EntityModel* lastDest = nil;
							if (fReversePath)
								err = fSubEntity->BuildRelPath(already, catalog, path, relpath, lastpart, allNto1, devMode, lastDest);
							else
								err = fOwner->BuildRelPath(already, catalog, path, relpath, lastpart, allNto1, devMode, lastDest);
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
									if (!devMode)
										err = ThrowError(VE_DB4D_NAVIGATION_PATH_IS_MALFORMED, &fRelPath);
									else
										catalog->AddError();
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
					if (!devMode)
						err = ThrowError(VE_DB4D_NAVIGATION_PATH_IS_MALFORMED, &fRelPath);
					else
						catalog->AddError();
				}
				else
				{
					EntityRelationCollection relpath;
					VString lastpart;
					bool allNto1 = true;
					err = fOwner->BuildRelPath(already, catalog, path, relpath, lastpart, allNto1, devMode, lastDest);
					if (allNto1)
					{
						if (lastpart.IsEmpty())
						{
							if (!devMode)
								err = ThrowError(VE_DB4D_ATT_FIELD_IS_MISSING);
							else
								catalog->AddError();
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
							}
						}
					}
					else
					{
						if (!devMode)
							err = ThrowError(VE_DB4D_NAVIGATION_ATTRIBUTE_MUST_BE_Nto1, &fRelPath);
						else
							catalog->AddError();
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
						fFlattenTableDest = fRelation->GetPath()[fRelation->GetPath().size()-1]->GetDestTable();
						if (!fFlattenColumnName.IsEmpty())
						{
							fFieldPos = fFlattenTableDest->FindField(fFlattenColumnName);
							if (fFieldPos <= 0)
							{
								const EntityAttribute* flattenDestAtt = nil;
								if (lastDest != nil)
								{
									flattenDestAtt = lastDest->getAttribute(fFlattenColumnName);
								}
								if (flattenDestAtt == nil)
								{
									if (!devMode)
										err = ThrowError(VE_DB4D_ATT_FIELD_NOT_FOUND, &fFlattenColumnName);
									else
										catalog->AddError();
								}
								else
								{
									if (flattenDestAtt->IsScalar())
									{
										fFlattenFromAField = false;
									}
									else
									{
										if (!devMode)
											err = ThrowError(VE_DB4D_ATT_FIELD_NOT_FOUND, &fFlattenColumnName);
										else
											catalog->AddError();
									}
								}
							}
							else
								fFlattenFromAField = true;
						}
						else
						{
							if (!devMode)
								err = ThrowError(VE_DB4D_ATT_FIELD_IS_MISSING);
							else
								catalog->AddError();
						}
					}
					else
					{
						if (!devMode)
							err = ThrowError(VE_DB4D_ENTITY_RELATION_IS_EMPTY);
						else
							catalog->AddError();
					}
				}
				else
				{
					if (!devMode)
						err = ThrowError(VE_DB4D_ENTITY_RELATION_IS_EMPTY);
					else
						catalog->AddError();
				}
			}
		}
	}

	if (err != VE_OK)
		err = ThrowError(VE_DB4D_CANNOT_RESOLVE_NAVIGATION_PATH, &fName);

	return err;
}


VError EntityAttribute::ResolveQueryStatements(EntityModelCatalog* catalog, bool devMode, BaseTaskInfo* context)
{
	VError err = VE_OK;
	VString ss;
	if (fKind == eattr_computedField)
	{
		if (fScriptKind == script_db4d)
		{
			if (fScriptDB4D == nil)
			{
				fScriptDB4D = new SearchTab(fOwner->GetMainTable());
				fScriptDB4D->AllowJSCode(true);
				if (devMode)
				{
					StErrorContextInstaller errs(false);
					err = fScriptDB4D->BuildFromString(fScriptStatement[script_attr_get], ss, context, fOwner, true);
					if (err != VE_OK)
						catalog->AddError();
					err = VE_OK;
				}
				else
					err = fScriptDB4D->BuildFromString(fScriptStatement[script_attr_get], ss, context, fOwner, true);
			}

			if (fScriptQuery == nil && err == VE_OK)
			{
				fScriptQuery = new SearchTab(fOwner->GetMainTable());
				fScriptQuery->AllowJSCode(true);
				if (devMode)
				{
					StErrorContextInstaller errs(false);
					if (fScriptStatement[script_attr_query].IsEmpty())
						err = fScriptQuery->BuildFromString(fScriptStatement[script_attr_get], ss, context, fOwner, false);
					else
						err = fScriptQuery->BuildFromString(fScriptStatement[script_attr_query], ss, context, fOwner, false);
					if (err != VE_OK)
						catalog->AddError();
					err = VE_OK;
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

void EntityAttribute::SetRelation(Relation* rel, EntityRelationKind kind, EntityModelCatalog* catalog)
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
	fSubEntity = EntityModel::BuildEntityModel(desttable, catalog);
	fRelation = new EntityRelation(source, dest, kind);
	fRelation->RecalcPath();
	fRelationPathID = fOwner->FindRelationPath(fRelation);
	if (fRelationPathID == -1)
		fRelationPathID = fOwner->AddRelationPath(fRelation);
	fSubEntityName = fSubEntity->GetName();
}
#endif


VError EntityAttribute::CallDBEvent(DBEventKind kind, EntityRecord* inRec, BaseTaskInfo* context) const
{
	VError err = VE_OK;
	if (kind != dbev_none)
	{
		const DBEvent& ev = fEvents[kind];
		if (ev.IsValid())
		{
			err = ev.Call(inRec, context, this, nil);
		}
	}
	return err;
}


bool EntityAttribute::HasEvent(DBEventKind kind) const
{
	bool result = false;
	if (kind != dbev_none)
	{
		const DBEvent& ev = fEvents[kind];
		if (ev.IsValid())
			result = true;
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
				Field* cri = RetainField();
				if (cri != nil)
				{
					res = cri->IsSortable();
				}
			}
			break;
		case eattr_computedField:
			{
				sLONG scalartype = ComputeScalarType();
				res = (scalartype != VK_IMAGE && scalartype != VK_BLOB && scalartype != VK_BLOB_DB4D); // temporaire
			}
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

	if (fEvents[dbev_validate].IsValid())
		needvalidate = true;

	return needvalidate;
}





// ----------------------------------------------------------------------------------------


AttributeStringPath::AttributeStringPath(const VString& inPath)
{
	inPath.GetSubStrings('.', fParts);
}


void AttributeStringPath::GetString(VString& outString) const
{
	outString.Clear();
	bool first = true;
	for (VectorOfVString::const_iterator cur = fParts.begin(), end = fParts.end(); cur != end; cur++)
	{
		if (first)
			first = false;
		else
			outString += L".";
		outString += *cur;
	}
}


// --------------------------------


AttributePath::AttributePath(EntityModel* model, const VString& inPath)
{
	fIsValid = FromPath(model, inPath);
}


AttributePath::AttributePath(EntityModel* model, const AttributeStringPath& inPath, bool fromstart)
{
	fIsValid = FromPath(model, inPath, false, fromstart);
}


bool AttributePath::FromPath(EntityModel* model, const VString& inPath, bool expandAliases)
{
	AttributeStringPath path(inPath);
	return FromPath(model, path, expandAliases);
}


bool AttributePath::FromPath(EntityModel* model, const AttributeStringPath& inPath, bool expandAliases, bool fromstart)
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
		result = false;

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
						JS4D::ExceptionRef excep = nil;
						vector<VJSValue> params;
						VJSValue ascentVal(jscontext);
						ascentVal.SetBool(ascent);
						params.push_back(ascentVal);
						VJSObject emjs(jscontext, VJSEntityModel::CreateInstance(jscontext, sousEm));
						emjs.CallFunction(*objfunc, &params, &result, &excep);
						if (excep != nil)
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


bool EntityAttributeSortedSelection::AddAttribute(EntityAttribute* att, RestTools* req)
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





// ----------------------------------------------------------------------------------------


DBEvent::DBEvent()
{
	fKind = dbev_none;
	fOwner = nil;
	fOverWrite = true;
	fUserDefined = false;
}


VError DBEvent::CopyFrom(EntityModel* inOwner, const DBEvent* from)
{
	fOverWrite = false;
	fOwner = inOwner;
	fFrom = from->fFrom;
	fKind = from->fKind;
	fUserDefined = from->fUserDefined;
	return VE_OK;
}


VError DBEvent::ToBag(VValueBag& outBag, bool forDax, bool forSave, bool forJSON, bool firstlevel) const
{
	VError err = VE_OK;

	if (fKind != dbev_none)
		outBag.SetString(d4::kind, EDBEventKinds[fKind]);
	if (!fFrom.IsEmpty())
		outBag.SetString(d4::from, fFrom);
	if (fUserDefined)
		outBag.SetBool(d4::userDefined, fUserDefined);

	return err;
}



VError DBEvent::FromBag(EntityModel* inOwner, const VValueBag* bag, EntityModelCatalog* catalog, bool devMode)
{
	VError err = VE_OK;
	VString ss;
	if (bag->GetString(d4::kind, ss) && !ss.IsEmpty())
	{
		fKind = (DBEventKind)EDBEventKinds[ss];
		/*
		if (fKind == dbev_none)
			fKind = (DBEventKind)oldEDBEventKinds[ss];
			*/
	}
	if (fKind == dbev_none)
	{
		if (!devMode)
			err = inOwner->ThrowError(VE_DB4D_MISSING_OR_INVALID_EVENTKIND);
		else
			catalog->AddError();
	}

	bag->GetString(d4::from, fFrom);
	if (fFrom.IsEmpty())
	{
		if (!devMode)
			err = inOwner->ThrowError(VE_DB4D_MISSING_OR_INVALID_EVENT_METHOD);
		else
			catalog->AddError();
	}

	if (err == VE_OK)
		fOwner = inOwner;

	fUserDefined = false;

	return err;
}


VError DBEvent::Call(EntityRecord* inRec, BaseTaskInfo* context, const EntityAttribute* inAtt, const EntityModel* inDataClass, Selection* *outSel) const
{
	VError err = VE_OK;
	Selection* selresult = nil;
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
				JSEntityMethodReference methref(fOwner, fKind, inAtt);
				VJSObject ObjFunc(jscontext);
				bool okfunc = false;

				if (!context->GetJSMethod(methref, ObjFunc))
				{
					VJSValue result(jscontext);
					jscontext.EvaluateScript(fFrom, nil, &result, nil, nil);
					if (!result.IsUndefined() && result.IsObject())
					{
						result.GetObject(ObjFunc);
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
						CDB4DEntityModel* model = (EntityModel*)inDataClass;
						therecord = VJSEntityModel::CreateInstance(jscontext, model);
					}
					else
						therecord = VJSEntitySelectionIterator::CreateInstance(jscontext, new EntitySelectionIterator(inRec, context->GetEncapsuleur()));
					JS4D::ExceptionRef excep;
					VJSValue outResult(jscontext);
					vector<VJSValue> Params;
					if (inAtt != nil)
					{
						VJSValue vAttName(jscontext);
						vAttName.SetString(inAtt->GetName());
						Params.push_back(vAttName);
					}
					therecord.CallFunction(ObjFunc, &Params, &outResult, &excep);

					if (excep != nil)
					{
						if (fKind != dbev_init)
						{
							ThrowJSExceptionAsError(jscontext, excep);
							err = fOwner->ThrowError(VE_DB4D_JS_ERR);
						}
					}
					else
					{
						CDB4DSelection* selres = outResult.GetObjectPrivateData<VJSEntitySelection>();
						if (selres != nil)
						{
							selresult = VImpCreator<VDB4DSelection>::GetImpObject(selres)->GetSel();
							if (selresult != nil)
								selresult->Retain();
						}
						else if (!outResult.IsNull() && outResult.IsObject() && (fKind != dbev_init))
						{
							VJSObject objres(jscontext);
							outResult.GetObject(objres);
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
							}
						}
					}

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
		*outSel = selresult;
	return err;
}




// ----------------------------------------------------------------------------------------



EntityModel::EntityModel(Base4D* inOwner, Table* inMainTable)
{
	fOwner = inOwner;
	fMainTable = RetainRefCountable(inMainTable);
	fQueryLimit = 0;
	fQueryApplyToEM = true;
	fBaseEm = nil;
	fDefaultTopSize = 0;
	fMatchPrimKey = 2;
	fRestrictingQuery = nil;
	fAlreadyResolvedComputedAtts = false;
	fIsTableDef = false;
	fExtraProperties = nil;
	fScope = escope_public;
	fHasDeleteEvent = false;
	fWithRestriction = false;
	fOneAttributeHasDeleteEvent = false;
	fPublishAsGlobal = false;
	fPublishAsGlobalDefined = false;
	fAllowOverrideStamp = false;
	fill(&fForced[DB4D_EM_None_Perm], &fForced[DB4D_EM_Promote_Perm+1], 0);
}


EntityModel::~EntityModel()
{
	QuickReleaseRefCountable(fMainTable);
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
	VErrorDB4D_OnEM *err = new VErrorDB4D_OnEM(inErrCode, noaction, fOwner, this);
	if (p1 != nil)
		err->GetBag()->SetString(Db4DError::Param1, *p1);
	VTask::GetCurrent()->PushRetainedError( err);

	return inErrCode;
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


sLONG EntityModel::FindAttribute(const VString& AttributeName) const
{
	EntityAttribute* ea = getAttribute(AttributeName);
	if (ea == nil)
		return 0;
	else
		return ea->GetPosInOwner();
}


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


CDB4DTable* EntityModel::RetainTable() const
{
	CDB4DBase* base = fOwner->RetainBaseX();
	CDB4DTable* result = new VDB4DTable(VDBMgr::GetManager(), VImpCreator<VDB4DBase>::GetImpObject(base), fMainTable);
	base->Release();
	return result;
}


CDB4DSelection* EntityModel::ProjectSelection(CDB4DSelection* sel, CDB4DEntityAttribute* att, VError& err, CDB4DBaseContext* context)
{
	Selection* result = nil;
	VDB4DSelection* xresult = nil;
	EntityAttribute* xatt = VImpCreator<EntityAttribute>::GetImpObject(att);
	EntityModel* destModel = xatt->GetSubEntityModel();
	Selection* xsel = VImpCreator<VDB4DSelection>::GetImpObject(sel)->GetSel();
	result = projectSelection(xsel, xatt, err, ConvertContext(context));
	if (result != nil)
	{
		CDB4DBase* xbase = fOwner->RetainBaseX();
		xresult = new VDB4DSelection(VDBMgr::GetManager(), VImpCreator<VDB4DBase>::GetImpObject(xbase), destModel->GetMainTable(), result, destModel);
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


VError EntityModel::ActivatePath(EntityRecord* erec, sLONG inPathID, SubEntityCache& outResult, bool Nto1, EntityModel* subEntityModel, BaseTaskInfo* context)
{
	VError err = VE_OK;
	EntityRelation* relpath = nil;
	
	outResult.SetActivated();
	outResult.SetModel(subEntityModel);

	relpath = fRelationPaths[inPathID];

	const EntityRelationCollection& path = relpath->GetPath();

	if (testAssert(relpath != nil && erec != nil))
	{
		Table* finaltarget = nil;
		if (!path.empty())
			finaltarget = path[path.size()-1]->GetDestTable();

		if (testAssert(finaltarget != nil))
		{
			if (subEntityModel == nil || testAssert(subEntityModel->GetMainTable() == finaltarget))
			{
				bool emptyResult = false;
				SearchTab query(finaltarget);
				InstanceMap instances;
				sLONG previnstance = 0;

				sLONG nbrel = path.size(), i = 0;

				for (EntityRelationCollection::const_reverse_iterator cur = path.rbegin(), end = path.rend(); cur != end && !emptyResult; ++cur, ++i)
				{
					if (i == nbrel - 1)
					{
						VValueSingle* cv = erec->getRecord()->GetFieldValue((*cur)->GetSourceField(), err, false, true);
						if (cv == nil || cv->IsNull())
							emptyResult = true;
						else
							query.AddSearchLineSimple((*cur)->GetDestField(), DB4D_Equal, cv, false, previnstance );
					}
					else
					{
						sLONG sourcetablenum = (*cur)->GetSourceField()->GetOwner()->GetNum();
						sLONG desttablenum = (*cur)->GetDestField()->GetOwner()->GetNum();
		
						sLONG destinstance = previnstance;
						if (desttablenum == sourcetablenum)
							instances.GetInstanceAndIncrement(desttablenum);
						sLONG sourceinstance = instances.GetInstanceAndIncrement(sourcetablenum);
						previnstance = sourceinstance;

						query.AddSearchLineJoin((*cur)->GetSourceField(), DB4D_Equal, (*cur)->GetDestField(), false, sourceinstance, destinstance);
						query.AddSearchLineBoolOper(DB4D_And);
					}
				}

				if (emptyResult)
				{
					if (Nto1)
						outResult.SetRecord(nil);
					else
						outResult.SetSel(nil);
				}
				else
				{
					SearchTab restsearch(subEntityModel->GetMainTable());
					if (subEntityModel->AddRestrictionToQuery(restsearch, context, err))
					{
						query.Add(restsearch);
					}
					if (subEntityModel->WithRestrictingQuery())
					{
						query.Add(*(subEntityModel->fRestrictingQuery));
					}
					OptimizedQuery xquery;
#if 0 && debuglr
					Boolean olddescribe = context->ShouldDescribeQuery();
					context->MustDescribeQuery(true);
#endif

					err = xquery.AnalyseSearch(&query, context);
					if (err == VE_OK)
					{
						if (okperm(context, subEntityModel, DB4D_EM_Read_Perm) || okperm(context, subEntityModel, DB4D_EM_Update_Perm) || okperm(context, subEntityModel, DB4D_EM_Delete_Perm))
						{
							Selection* sel = xquery.Perform((Bittab*)nil, nil, context, err, DB4D_Do_Not_Lock);
							if (sel != nil)
							{
								if (Nto1)
								{
									FicheInMem* rec = nil;
									if (sel->GetQTfic() > 0)
										rec = finaltarget->GetDF()->LoadRecord(sel->GetFic(0), err, DB4D_Do_Not_Lock, context);
									QuickReleaseRefCountable(sel);
									outResult.SetRecord(rec);
								}
								else
									outResult.SetSel(sel);
							}
						}
						else
						{
							err = subEntityModel->ThrowError(VE_DB4D_NO_PERM_TO_READ);
							context->SetPermError();
						}
					}
#if 0 && debuglr
					context->MustDescribeQuery(olddescribe);
#endif
				}
			}
		}
	}
	return err;
}


VError EntityModel::BuildRelPath(SubPathRefSet& already, EntityModelCatalog* catalog, const VectorOfVString& path, EntityRelationCollection& outRelPath, VString& outLastpart, bool& outAllNto1, bool devMode, EntityModel* &outLastDest)
{
	VError err = VE_OK;

	VString attname = path[0];

	EntityAttribute* att = getAttribute(attname);
	if (att == nil)
	{
		if (!devMode)
			err = ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_NOT_FOUND, &attname);
		else
			catalog->AddError();
	}
	else
	{
		SubPathRef part(this, attname);
		if (already.find(part) != already.end())
		{
			if (!devMode)
				err = ThrowError(VE_DB4D_RELATION_PATH_IS_RECURSIVE, &attname);
			else
				catalog->AddError();
		}
		else
		{
			already.insert(part);
			EntityAttributeKind what = att->GetKind();
			if (what == eattr_relation_Nto1 || what == eattr_relation_1toN || what == eattr_composition)
			{
				if (what != eattr_relation_Nto1)
					outAllNto1 = false;
				err = att->ResolveRelatedEntities(already, catalog, devMode, nil);
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
								err = dest->BuildRelPath(already, catalog, subpath, outRelPath, outLastpart, outAllNto1, devMode, outLastDest);
							}
						}
						else
						{
							if (!devMode)
								err = ThrowError(VE_DB4D_ENTITY_RELATION_DOES_NOT_EXIST, &attname);
							else
								catalog->AddError();
						}
					}
					else
					{
						if (!devMode)
							err = ThrowError(VE_DB4D_ENTITY_RELATION_DOES_NOT_EXIST, &attname);
						else
							catalog->AddError();

					}
				}
			}
			else
			{
				if (path.size() != 1)
				{
					if (!devMode)
						err = ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_IS_NOT_NAVIGATION, &attname);
					else
						catalog->AddError();
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
	if (fScope != escope_none)
		outBag.SetString(d4::scope, EScopeKinds[(sLONG)fScope]);

	VString tablename;
	fMainTable->GetName(tablename);

	if (forSave)
	{
		if (fAllowOverrideStamp)
			outBag.SetBool(d4::allowOverrideStamp, fAllowOverrideStamp);
		if (fPublishAsGlobalDefined)
			outBag.SetBool(d4::publishAsJSGlobal, fPublishAsGlobal);
		if (fIsTableDef)
		{
			VUUID xid;
			fMainTable->GetUUID(xid);
			outBag.SetVUUID(d4::uuid, xid);
			outBag.SetLong(d4::tablePos, fMainTable->GetNum());
		}
		else
		{
			if (fExtends.IsEmpty())
				outBag.SetString(d4::dataSource, tablename);
			else
				outBag.SetString(d4::extends, fExtends);
		}
	}
	/*
	else
	{
		outBag.SetString(d4::dataSource, tablename);
	}
	*/

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

	if (WithRestrictingQuery() && forSave)
	{
		VValueBag* queryBag = new VValueBag();
		if (forJSON)
			queryBag->SetBool(____objectunic, true);

		if (forSave)
		{
			queryBag->SetString(d4::queryStatement, fRestrictingQueryString);
			if (fQueryApplyToEM)
				queryBag->SetBool(d4::applyToModel, true);
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
			att->ToBag(*attBag, forDax, forSave, forJSON, fIsTableDef && forSave);
			outBag.AddElement(d4::attributes, attBag);
			attBag->Release();
		}
	}

	
	for (DBEventConstIterator cur = &fEvents[dbev_firstEvent], end = &fEvents[dbev_lastEvent+1]; cur != end && err == VE_OK; cur++)
	{
		if (cur->IsValid() && (cur->IsOverWritten() || !forSave))
		{
			BagElement evBag(outBag, d4::events);
			err = cur->ToBag(*evBag, forDax, forSave, forJSON, true);
		}
	}


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


VError EntityModel::FromBag(const VValueBag* inBag, EntityModelCatalog* catalog, bool devMode)
{
	VError err = VE_OK;

	fExtraProperties = inBag->RetainUniqueElement(d4::extraProperties);

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
			if (!devMode)
				err = fOwner->ThrowError(VE_DB4D_ENTITY_NAME_MISSING, noaction);
			else
				catalog->AddError();
		}
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
	VString tablename;
	Table* maintable = nil;
	bool mustbuildtable = false;
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
				fMainTable = RetainRefCountable(otherEm->fMainTable);
				fRelationPaths.resize(otherEm->fRelationPaths.size());
				if (otherEm->fRestrictingQuery != nil)
				{
					fRestrictingQuery = new SearchTab(fMainTable);
					fRestrictingQuery->From(*(otherEm->fRestrictingQuery));
				}
				else
					fRestrictingQuery = nil;
				fQueryLimit = otherEm->fQueryLimit;
				fQueryApplyToEM = true;
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
				DBEventIterator curdest = &fEvents[dbev_firstEvent];
				for (DBEventConstIterator cur = &otherEm->fEvents[dbev_firstEvent], end = &otherEm->fEvents[dbev_lastEvent+1]; cur != end && err == VE_OK; cur++, curdest++)
				{
					if (cur->IsValid())
						curdest->CopyFrom(this, cur);
				}
				cumulQuery = otherEm->GetCumulatedQuery();
				fCumulatedRestrictingQueryString = cumulQuery;
				fIdentifyingAtts = otherEm->fIdentifyingAtts;
				fPrimaryAtts = otherEm->fPrimaryAtts;
				for (IdentifyingAttributeCollection::iterator cur = fPrimaryAtts.begin(), end = fPrimaryAtts.end(); cur != end; cur++)
				{
					cur->fAtt = FindAttributeByFieldPos(cur->fAtt->GetFieldPos());
				}
				for (IdentifyingAttributeCollection::iterator cur = fIdentifyingAtts.begin(), end = fIdentifyingAtts.end(); cur != end; cur++)
				{
					cur->fAtt = FindAttributeByFieldPos(cur->fAtt->GetFieldPos());
				}
				otherEm->AddDerivated(this);
				//otherEm->Release();
			}
			else
			{
				if (devMode)
					catalog->AddError();
				else
					err = ThrowError(VE_DB4D_ENTITY_NOT_FOUND_FOR_EXTENDS, &otherEMname);
			}
		}
		else
		{
			if (inBag->GetString(d4::dataSource, tablename))
			{
				maintable = fOwner->FindAndRetainTableRef(tablename);
				if (maintable != nil)
				{
					fMainTable = maintable;
				}
				else
				{
					if (devMode)
						catalog->AddError();
					else
						err = ThrowError(VE_DB4D_ENTITY_WRONG_TABLE_REF, &tablename);
				}
			}
			else
			{
				mustbuildtable = true;
				maintable = fOwner->FindAndRetainTableRef(fName);
				if (maintable != nil)
				{
					if (devMode)
						catalog->AddError();
					else
						err = ThrowError(VE_DB4D_ENTITY_TABLENAME_DUPLICATED, &fName);					
					maintable->Release();
				}
				else
				{
					fMainTable = new TableRegular(fOwner, 0, true);
					fMainTable->SetNameSilently(fName);
					VUUID xid;
					if (!inBag->GetVUUID(d4::uuid, xid))
					{
						xid.Regenerate();
						catalog->TouchXML();
					}
					fMainTable->SetUUID(xid);
					fIsTableDef = true;
				}
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
						err = entMeth->FromBag(methodBag, catalog, devMode);
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
							evkind = (DBEventKind)EDBEventKinds[ss];
							/*
							if (evkind == dbev_none)
								evkind = (DBEventKind)oldEDBEventKinds[ss];
								*/
						}
						if (evkind == dbev_none)
						{
							if (!devMode)
								err = ThrowError(VE_DB4D_MISSING_OR_INVALID_EVENTKIND);
							else
								catalog->AddError();
						}
						else
						{
							err = fEvents[evkind].FromBag(this, eventBag, catalog, devMode);
						}
					}
				}
			}
		}
	}

	if (err == VE_OK)
	{
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
					else if ((dejaAtt == nil) && (EattTypes[kindname] == (sLONG)eattr_storage) && !fExtends.IsEmpty())
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
							if (dejaAtt != nil && fBaseEm != nil)
							{
								entAtt = dejaAtt;
							}
							else
							{
								EntityAttributeMap::iterator found = fAttributesByName.find(attname);
								if (found != fAttributesByName.end())
								{
									//entAtt = found->second->Clone(this); // old code
									if (!devMode)
										err = ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_ALREADY_EXISTS, &attname);
									else
									{
										catalog->AddError();
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
							err = entAtt->FromBag(attributeBag, catalog, !mustbuildtable, devMode);
						if (err == VE_OK && !entAtt->GetName().IsEmpty())
						{
							if (mustChangeName)
								entAtt->SetName(attname);
							if (mustbuildtable && dejaAtt == nil)
							{
								if (entAtt->GetAttributeKind() == eattr_storage)
								{
									sLONG typ = entAtt->ComputeScalarType();
									if (typ != 0)
									{
										sLONG pos = entAtt->GetFieldPos();
										if (pos == 0)
										{
											pos = fMainTable->FindNextFieldFree();
											if (pos == -1)
												pos = fMainTable->GetNbCrit()+1;
											entAtt->SetFieldPos(pos);
										}
										if (typ == VK_STRING && d4::textAsBlob(attributeBag))
											typ = VK_TEXT;
										bool utf8 = DB4DBagKeys::store_as_utf8(attributeBag);
										if (typ == VK_STRING && utf8)
											typ = VK_STRING_UTF8;
										if (typ == VK_TEXT && utf8)
											typ = VK_TEXT_UTF8;

										Field* cri = new Field(typ, pos, fMainTable);
										attname = entAtt->GetName();
										Critere* xcri = cri->GetCritere();
										xcri->SetName(attname);

										if ((typ == VK_STRING) || (typ == VK_STRING_UTF8))
											xcri->SetLimitingLen( DB4DBagKeys::limiting_length( attributeBag));
										xcri->SetAutoSeq( DB4DBagKeys::autosequence( attributeBag));
										xcri->SetAutoGenerate( DB4DBagKeys::autogenerate( attributeBag));
										xcri->SetStoreUTF8( DB4DBagKeys::store_as_utf8( attributeBag));
										xcri->SetNot_Null( DB4DBagKeys::not_null( attributeBag));
										xcri->SetNeverNull( DB4DBagKeys::never_null( attributeBag));
										xcri->SetUnique( DB4DBagKeys::unique( attributeBag));
										xcri->SetOutsideData(DB4DBagKeys::outside_blob( attributeBag));
										xcri->SetStyledText(DB4DBagKeys::styled_text( attributeBag));
										if ((typ == VK_TEXT) || (typ == VK_TEXT_UTF8))
											xcri->SetTextSwitchSize( DB4DBagKeys::text_switch_size( attributeBag));
										if ((typ == VK_BLOB) || (typ == VK_BLOB_DB4D) || (typ == VK_IMAGE))
											xcri->SetBlobSwitchSize( DB4DBagKeys::blob_switch_size( attributeBag));

										
										VUUID xid;									
										if (!attributeBag->GetVUUID(d4::uuid, xid))
										{
											xid.Regenerate();
											catalog->TouchXML();
										}
										cri->SetUUID(xid);
										

										fMainTable->AddFieldSilently(cri, pos);

										
									}
								}
							}

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

			bool ApplyToEm = true;
			//QueryBag->GetBool(d4::applyToModel, ApplyToEm);
			fQueryApplyToEM = ApplyToEm;

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

	if (err == VE_OK)
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
							if (!devMode)
								err = att->ThrowError(VE_DB4D_WRONG_ATTRIBUTE_TYPE_FOR_IDENT);
							else
								catalog->AddError();
						}
					}
					else
					{
						if (!devMode)
							err = ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_NOT_FOUND, &s);
						else
							catalog->AddError();
					}

				}
				else
				{
					if (!devMode)
						err = ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_NAME_MISSING);
					else
						catalog->AddError();
				}
			}
		}
	}


	if (err == VE_OK)
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
							if (!devMode)
								err = att->ThrowError(VE_DB4D_WRONG_ATTRIBUTE_TYPE_FOR_PRIMKEY);
							else
								catalog->AddError();
						}
					}
					else
					{
						if (!devMode)
							err = ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_NOT_FOUND, &s);
						else
							catalog->AddError();
					}

				}
				else
				{
					if (!devMode)
						err = ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_NAME_MISSING);
					else
						catalog->AddError();
				}
			}
		}
	}

	if (err == VE_OK)
	{
		if (!fOwnedIdentifyingAtts.empty())
			fIdentifyingAtts = fOwnedIdentifyingAtts;
		if (!fOwnedPrimaryAtts.empty())
			fPrimaryAtts = fOwnedPrimaryAtts;
	}

	if (err == VE_OK)
	{
		for (IdentifyingAttributeCollection::iterator cur = fPrimaryAtts.begin(), end = fPrimaryAtts.end(); cur != end; cur++)
		{
			cur->fAtt->SetPartOfPrimKey();
		}

		if (mustbuildtable)
		{
			NumFieldArray prim;
			if (!fPrimaryAtts.empty())
			{
				prim.SetCount(fPrimaryAtts.size());
				sLONG i = 1;
				for (IdentifyingAttributeCollection::iterator cur = fPrimaryAtts.begin(), end = fPrimaryAtts.end(); cur != end; cur++, i++)
				{
					prim[i] = cur->fAtt->GetFieldPos();
				}
				if (!devMode)
					err = fMainTable->SetPrimaryKeySilently(prim, nil);
				else
				{
					StErrorContextInstaller errs(false);
					VError err2 = fMainTable->SetPrimaryKeySilently(prim, nil);
					if (err2 != VE_OK)
					{
						catalog->AddError();
					}
				}
			}
			if (err == VE_OK)
			{
				sLONG tpos = 0;
				inBag->GetLong(d4::tablePos, tpos);

				err = fOwner->AddTable(fMainTable, false, nil, true, false, false, tpos, false);
				VUUID xid;
				fMainTable->GetUUID(xid);
				fOwner->AddObjectID(objInBase_Table, fMainTable, xid);
				for (sLONG i = 1, nb = fMainTable->GetNbCrit(); i <= nb; i++)
				{
					Field* cri = fMainTable->RetainField(i);
					if (cri != nil)
					{
						cri->GetUUID(xid);
						fOwner->AddObjectID(objInBase_Field, cri, xid);
					}
					QuickReleaseRefCountable(cri);
				}
			}

			if (err == VE_OK)
			{
				if (prim.GetCount() == 1)
				{
					IndexInfoFromField* ind = new IndexInfoFromField(fOwner, fMainTable->GetNum(), prim[1], DB4D_Index_Btree, true, true);
					fOwner->AddXMLIndex(ind);
					ind->Release();
				}
			}

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
						attributeBag->GetString(d4::name, attname);
						EntityAttribute* att = getAttribute(attname);
						if (att != nil)
						{
							EntityAttributeKind attkind = att->GetKind();
							if (attkind == eattr_storage)
							{
								VString IndexKind = att->GetIndexKind();
								VectorOfVString indexKinds;
								IndexKind.GetSubStrings(',', indexKinds, false, true);
								for (VectorOfVString::iterator cur = indexKinds.begin(), end = indexKinds.end(); cur != end; cur++)
								{
									sLONG indextype = EIndexKinds[*cur];
									if (indextype == 8 /* keywords */)
									{
										IndexInfoFromFieldLexico* ind = new IndexInfoFromFieldLexico(fOwner, fMainTable->GetNum(), att->GetFieldPos(), DB4D_Index_Btree);
										fOwner->AddXMLIndex(ind);
										ind->Release();
									}
									else
									{
										if (indextype == DB4D_Index_Btree || indextype == DB4D_Index_BtreeWithCluster || indextype == DB4D_Index_AutoType)
										{
											IndexInfoFromField* ind = new IndexInfoFromField(fOwner, fMainTable->GetNum(), att->GetFieldPos(), indextype, false, true);
											fOwner->AddXMLIndex(ind);
											ind->Release();
										}
									}
								}
							}
						}
					}
				}
			}

		}
	}

	if (err == VE_OK)
	{
		if (fMainTable != nil && fMainTable->HasPrimKey() && fPrimaryAtts.empty())
		{
			NumFieldArray primkey;
			fMainTable->CopyPrimaryKey(primkey);
			for (NumFieldArray::Iterator cur = primkey.First(), end = primkey.End(); cur != end; cur++)
			{
				sLONG numfield = *cur;
				EntityAttribute* att = FindAttributeByFieldPos(numfield);
				if (att != nil)
					fPrimaryAtts.push_back(IdentifyingAttribute(att));
			}
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

		}
	}

	if (err == VE_OK)
	{
		VJSContext	jscontext(*(catalog->getLoadingContext()));
		VJSObject	result(jscontext);
		VString		path	= "model."+fName;

		/*		
		jscontext.DisableDebugger();		
		jscontext.EvaluateScript(path, nil, &result, nil, nil);
		*/

		bool				isOk		= false;
		JS4D::ExceptionRef	exception	= NULL;
		
		result = jscontext.GetGlobalObject().GetPropertyAsObject("model", &exception);
		if (exception == NULL && result.IsObject()) {

			result = result.GetPropertyAsObject(fName, &exception);
			isOk = exception == NULL && result.IsObject();

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
							curevent.GetPropertyName(eventName);
							DBEventKind evkind = (DBEventKind)EDBEventKinds[eventName];
							if (evkind != dbev_none)
							{
								VJSValue eventFunc(curevent.GetProperty());
								if (eventFunc.IsFunction())
								{
									VString path2 = path + ".events."+eventName;
									fEvents[evkind].SetEvent(this, evkind, path2);
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
									curmethod.GetPropertyName(funcname);
									VString path2 = path+"."+propName+"."+funcname;
									EntityMethod* entMeth = new EntityMethod(this);
									err = entMeth->FromJS(funcname, applyTo, path2, scope,  catalog, devMode);
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

				}

				++curprop;
			}

		}

	}

	fWithRestriction = false;
	if (fEvents[dbev_restrict].IsValid())
		fWithRestriction = true;

	fHasDeleteEvent = false;
	fOneAttributeHasDeleteEvent = false;
	if (fEvents[dbev_remove].IsValid())
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


VError EntityModel::ResolveRelatedEntities(SubPathRefSet& already, EntityModelCatalog* catalog, bool devMode, BaseTaskInfo* context)
{
	VError err = VE_OK;

	for (EntityAttributeCollection::iterator cur = fAttributes.begin(), end = fAttributes.end(); cur != end && err == VE_OK; cur++)
	{
		SubPathRefSet already2;
		err = (*cur)->ResolveRelatedEntities(already2, catalog, devMode, context);
	}

	for (EntityMethodMap::iterator cur = fMethodsByName.begin(), end = fMethodsByName.end(); cur != end && err == VE_OK; cur++)
	{
		err = cur->second->ResolveType(catalog, devMode);
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

VError EntityModel::ResolveQueryStatements(EntityModelCatalog* catalog, bool devMode, BaseTaskInfo* context)
{
	VError err = VE_OK;

	VString orderbystring;

	if (fRestrictingQuery == nil && !fCumulatedRestrictingQueryString.IsEmpty())
	{

		if (fBaseEm != nil)
		{
			err = fBaseEm->ResolveQueryStatements(catalog, devMode, context);
			if (fBaseEm->fRestrictingQuery != nil && err == VE_OK)
			{
				fRestrictingQuery = new SearchTab(fMainTable);
				fRestrictingQuery->From(*(fBaseEm->fRestrictingQuery));
			}
		}


		if (err == VE_OK)
		{
			if (fRestrictingQuery == nil)
			{
				fRestrictingQuery = new SearchTab(fMainTable);
				fRestrictingQuery->AllowJSCode(true);
				StErrorContextInstaller errs(!devMode);
				err = fRestrictingQuery->BuildFromString(fRestrictingQueryString, orderbystring, context, fQueryApplyToEM ? this : nil);
				if (err != VE_OK)
					catalog->AddError();
				if (devMode)
					err = VE_OK;
			}
			else
			{
				StErrorContextInstaller errs(!devMode);
				SearchTab newquery(fMainTable);
				newquery.AllowJSCode(true);
				err = newquery.BuildFromString(fRestrictingQueryString, orderbystring, context, fQueryApplyToEM ? this : nil);
				if (err == VE_OK)
					err = fRestrictingQuery->Add(newquery);
				if (err != VE_OK)
					catalog->AddError();
				if (devMode)
					err = VE_OK;
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
			err = (*cur)->ResolveQueryStatements(catalog, devMode, context);
		}
	}

	return err;
}


EntityRecord* EntityModel::LoadEntityRecord(sLONG n, VError& err, DB4D_Way_of_Locking HowToLock, BaseTaskInfo* context, bool autoexpand)
{
	if (okperm(context, fPerms[DB4D_EM_Read_Perm]) || okperm(context, fPerms[DB4D_EM_Update_Perm]) || okperm(context, fPerms[DB4D_EM_Delete_Perm]))
	{
		FicheInMem* rec = fMainTable->GetDF()->LoadRecord(n, err, HowToLock, context);
		if (rec == nil)
			return nil;
		else
		{
			EntityRecord* erec = new EntityRecord(this, rec, context->GetEncapsuleur(), HowToLock);
			rec->Release();
			CallDBEvent(dbev_load, erec, context);
			return erec;
		}
	}
	else
	{
		err = ThrowError(VE_DB4D_NO_PERM_TO_READ);
		context->SetPermError();
		return nil;
	}
}


CDB4DEntityRecord* EntityModel::LoadEntity(sLONG n, VError& err, DB4D_Way_of_Locking HowToLock, CDB4DBaseContext* context, bool autoexpand)
{
	if (okperm(context, fPerms[DB4D_EM_Read_Perm]) || okperm(context, fPerms[DB4D_EM_Update_Perm]) || okperm(context, fPerms[DB4D_EM_Delete_Perm]))
	{
		BaseTaskInfo* xcontext = ConvertContext(context);
		FicheInMem* rec = fMainTable->GetDF()->LoadRecord(n, err, HowToLock, xcontext);
		if (rec == nil)
			return nil;
		else
		{
			EntityRecord* erec = new EntityRecord(this, rec, context, HowToLock);
			CallDBEvent(dbev_load, erec, xcontext);
			return erec;
		}
	}
	else
	{
		BaseTaskInfo* xcontext = ConvertContext(context);
		err = ThrowError(VE_DB4D_NO_PERM_TO_READ);
		xcontext->SetPermError();
		return nil;
	}
}


sLONG EntityModel::CountEntities(CDB4DBaseContext* inContext)
{
	sLONG result = 0;
	if (okperm(inContext, fPerms[DB4D_EM_Read_Perm]) || okperm(inContext, fPerms[DB4D_EM_Update_Perm]) || okperm(inContext, fPerms[DB4D_EM_Delete_Perm]))
	{
		if (WithRestriction())
		{
			VError err = VE_OK;
			Selection* sel = BuildRestrictingSelection(ConvertContext(inContext), err);
			if (sel != nil)
			{
				result = sel->GetQTfic();
				sel->Release();
			}
		}
		else if (WithRestrictingQuery())
		{
			BaseTaskInfo* context = ConvertContext(inContext);
			Base4D *db;
			DataTable *DF;
			DF=fMainTable->GetDF();
			db=DF->GetDB();
			OptimizedQuery rech;
			db->LockIndexes();
			VError err = rech.AnalyseSearch(fRestrictingQuery, context);
			db->UnLockIndexes();
			if (err == VE_OK)
			{
				Selection* sel = rech.Perform((Bittab*)nil, nil, context, err, DB4D_Do_Not_Lock, 0, nil);
				if (sel != nil)
				{
					result = sel->GetQTfic();
					sel->Release();
				}
			}
		}
		else
			result = fMainTable->GetDF()->GetNbRecords(ConvertContext(inContext));
	}
	else
		result = 0;
	return result;
}


sLONG EntityModel::getEntityNumWithPrimKey(const VString& primkey, BaseTaskInfo* context, VError& err)
{
	VectorOfVString xprimkey;
	Wordizer ww(primkey);
	ww.ExctractStrings(xprimkey, true, '&', false);
	return getEntityNumWithPrimKey(xprimkey, context, err);
}


sLONG EntityModel::getEntityNumWithPrimKey(const VectorOfVString& primkey, BaseTaskInfo* context, VError& err)
{
	sLONG result = -1;
	err = VE_OK;
	if (!fPrimaryAtts.empty())
	{
		if (fPrimaryAtts.size() == primkey.size())
		{
			SearchTab query(fMainTable);
			VectorOfVString::const_iterator curkey = primkey.begin();
			bool first = true;
			for (IdentifyingAttributeCollection::iterator cur = fPrimaryAtts.begin(), end = fPrimaryAtts.end(); cur != end; cur++, curkey++)
			{
				if (first)
					first = false;
				else
					query.AddSearchLineBoolOper(DB4D_And);
				query.AddSearchLineSimple(fMainTable->GetNum(), cur->fAtt->GetFieldPos(), DB4D_Equal, &(*curkey));
			}
			Selection* sel = ExecuteQuery(&query, context, nil, nil, DB4D_Do_Not_Lock, 0, nil, &err);
			if (sel != nil)
			{
				if (!sel->IsEmpty())
				{
					result = sel->GetFic(0);
				}
				sel->Release();
			}
		}
		else
			err = ThrowError(VE_DB4D_PRIMKEY_MALFORMED);
	}
	else
		err = ThrowError(VE_DB4D_ENTITY_HAS_NO_PRIMKEY);

	return result;
}


sLONG EntityModel::getEntityNumWithPrimKey(const VValueBag& primkey, BaseTaskInfo* context, VError& err, bool ErrOnNull)
{
	err = VE_OK;
	sLONG result = -1;
	if (!fPrimaryAtts.empty())
	{
		SearchTab query(fMainTable);
		bool first = true;
		for (IdentifyingAttributeCollection::iterator cur = fPrimaryAtts.begin(), end = fPrimaryAtts.end(); cur != end && err == VE_OK; cur++)
		{
			EntityAttribute* att = cur->fAtt;
			const VValueSingle* cv = primkey.GetAttribute(att->GetName());
			if (cv != nil)
			{
				if (first)
					first = false;
				else
					query.AddSearchLineBoolOper(DB4D_And);
				query.AddSearchLineSimple(fMainTable->GetNum(), cur->fAtt->GetFieldPos(), DB4D_Equal, cv);
			}
			else
			{
				if (ErrOnNull)
					err = ThrowError(VE_DB4D_PRIMKEY_MALFORMED);
				else
					err = VE_DB4D_PRIMKEY_MALFORMED;
			}
		}
		if (err == VE_OK)
		{
			Selection* sel = ExecuteQuery(&query, context, nil, nil, DB4D_Do_Not_Lock, 0, nil, &err);
			if (sel != nil)
			{
				if (!sel->IsEmpty())
				{
					result = sel->GetFic(0);
				}
				sel->Release();
			}
		}
	}
	else
	{
		if (ErrOnNull)
			err = ThrowError(VE_DB4D_ENTITY_HAS_NO_PRIMKEY);
	}

	return result;
}



sLONG EntityModel::getEntityNumWithPrimKey(const VectorOfVValue& primkey, BaseTaskInfo* context, VError& err)
{
	sLONG result = -1;
	err = VE_OK;
	if (!fPrimaryAtts.empty())
	{
		if (fPrimaryAtts.size() == primkey.size())
		{
			SearchTab query(fMainTable);
			VectorOfVValue::const_iterator curkey = primkey.begin();
			bool first = true;
			for (IdentifyingAttributeCollection::iterator cur = fPrimaryAtts.begin(), end = fPrimaryAtts.end(); cur != end; cur++, curkey++)
			{
				if (first)
					first = false;
				else
					query.AddSearchLineBoolOper(DB4D_And);
				query.AddSearchLineSimple(fMainTable->GetNum(), cur->fAtt->GetFieldPos(), DB4D_Equal, *curkey);
			}
			Selection* sel = ExecuteQuery(&query, context, nil, nil, DB4D_Do_Not_Lock, 0, nil, &err);
			if (sel != nil)
			{
				if (!sel->IsEmpty())
				{
					result = sel->GetFic(0);
				}
				sel->Release();
			}
		}
		else
			err = ThrowError(VE_DB4D_PRIMKEY_MALFORMED);
	}
	else
		err = ThrowError(VE_DB4D_ENTITY_HAS_NO_PRIMKEY);

	return result;
}


sLONG EntityModel::getEntityNumWithPrimKey(VJSObject objkey, BaseTaskInfo* context, VError& err)
{
	sLONG result = -1;
	VJSValue jsval(objkey.GetContextRef());
	jsval = objkey.GetProperty("__RECID");
	if (jsval.IsNumber())
	{
		jsval.GetLong(&result);
	}
	else
	{
		if (!fPrimaryAtts.empty())
		{
			SearchTab query(fMainTable);
			bool first = true;
			bool okcont = true;
			for (IdentifyingAttributeCollection::iterator cur = fPrimaryAtts.begin(), end = fPrimaryAtts.end(); cur != end && okcont ; cur++)
			{
				EntityAttribute* att = cur->fAtt;
				jsval = objkey.GetProperty(att->GetName());
				if (!jsval.IsUndefined() && !jsval.IsNull())
				{
					VValueSingle* cv = jsval.CreateVValue();
					if (cv != nil)
					{
						if (first)
							first = false;
						else
							query.AddSearchLineBoolOper(DB4D_And);
						query.AddSearchLineSimple(fMainTable->GetNum(), cur->fAtt->GetFieldPos(), DB4D_Equal, cv);
					}
					else
						okcont = false;
				}
				else
					okcont = false;
			}

			if (okcont)
			{
				Selection* sel = ExecuteQuery(&query, context, nil, nil, DB4D_Do_Not_Lock, 0, nil, &err);
				if (sel != nil)
				{
					if (!sel->IsEmpty())
					{
						result = sel->GetFic(0);
					}
					sel->Release();
				}
			}

		}
	}
	return result;
}


EntityRecord* EntityModel::findEntityWithPrimKey(VJSObject objkey, BaseTaskInfo* context, VError& err, DB4D_Way_of_Locking HowToLock)
{
	err = VE_OK;
	EntityRecord* erec = nil;
	sLONG numrec = getEntityNumWithPrimKey(objkey, context, err);
	if (numrec != -1)
	{
		erec = LoadEntityRecord(numrec, err, HowToLock, context, false);
	}

	return erec;
}


EntityRecord* EntityModel::findEntityWithPrimKey(const VString& primkey, BaseTaskInfo* context, VError& err, DB4D_Way_of_Locking HowToLock)
{
	err = VE_OK;
	EntityRecord* erec = nil;
	sLONG numrec = getEntityNumWithPrimKey(primkey, context, err);
	if (numrec != -1)
	{
		erec = LoadEntityRecord(numrec, err, HowToLock, context, false);
	}

	return erec;
}


EntityRecord* EntityModel::findEntityWithPrimKey(const VectorOfVString& primkey, BaseTaskInfo* context, VError& err, DB4D_Way_of_Locking HowToLock)
{
	err = VE_OK;
	EntityRecord* erec = nil;
	sLONG numrec = getEntityNumWithPrimKey(primkey, context, err);
	if (numrec != -1)
	{
		erec = LoadEntityRecord(numrec, err, HowToLock, context, false);
	}

	return erec;
}

EntityRecord* EntityModel::findEntityWithPrimKey(const VValueBag& primkey, BaseTaskInfo* context, VError& err, DB4D_Way_of_Locking HowToLock)
{
	err = VE_OK;
	EntityRecord* erec = nil;
	sLONG numrec = getEntityNumWithPrimKey(primkey, context, err);
	if (numrec != -1)
	{
		erec = LoadEntityRecord(numrec, err, HowToLock, context, false);
	}

	return erec;
}


EntityRecord* EntityModel::findEntityWithPrimKey(const VectorOfVValue& primkey, BaseTaskInfo* context, VError& err, DB4D_Way_of_Locking HowToLock)
{
	err = VE_OK;
	EntityRecord* erec = nil;
	sLONG numrec = getEntityNumWithPrimKey(primkey, context, err);
	if (numrec != -1)
	{
		erec = LoadEntityRecord(numrec, err, HowToLock, context, false);
	}

	return erec;
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

CDB4DEntityRecord* EntityModel::FindEntityWithPrimKey(const VectorOfVValue& primkey, CDB4DBaseContext* inContext, VErrorDB4D& err, DB4D_Way_of_Locking HowToLock)
{
	return findEntityWithPrimKey(primkey, ConvertContext(inContext), err, HowToLock);
}


bool EntityModel::MatchPrimKeyWithDataSource() const
{
	if (fMatchPrimKey == 2)
	{
		if (fPrimaryAtts.empty())
			fMatchPrimKey = 0;
		else
		{
			FieldArray primkeyfields;
			fMainTable->RetainPrimaryKey(primkeyfields);
			if (primkeyfields.GetCount() == 0 || primkeyfields.GetCount() != fPrimaryAtts.size())
				fMatchPrimKey = 0;
			else
			{
				fMatchPrimKey = 1;
				IdentifyingAttributeCollection::const_iterator curatt = fPrimaryAtts.begin();
				for (FieldArray::Iterator cur = primkeyfields.First(), end = primkeyfields.End(); cur != end; cur++, curatt++)
				{
					if ((*cur)->GetPosInRec() != curatt->fAtt->GetFieldPos())
						fMatchPrimKey = 0;
				}
				
			}
			for (FieldArray::Iterator cur = primkeyfields.First(), end = primkeyfields.End(); cur != end; cur++)
				(*cur)->Release();
		}
	}
	return fMatchPrimKey == 1;
}


sLONG EntityModel::getEntityNumWithIdentifyingAtts(const VectorOfVString& idents, BaseTaskInfo* context, VError& err)
{
	sLONG result = -1;
	err = VE_OK;
	if (!fIdentifyingAtts.empty())
	{
		SearchTab query(fMainTable);
		bool enough = true;
		VectorOfVString::const_iterator curkey = idents.begin();
		bool first = true;
		for (IdentifyingAttributeCollection::iterator cur = fIdentifyingAtts.begin(), end = fIdentifyingAtts.end(); cur != end; cur++, curkey++)
		{
			if (curkey == idents.end())
			{
				if (!cur->fOptionnel)
					enough = false;
				break;
			}
			if (first)
				first = false;
			else
				query.AddSearchLineBoolOper(DB4D_And);
			query.AddSearchLineSimple(fMainTable->GetNum(), cur->fAtt->GetFieldPos(), DB4D_Equal, &(*curkey));
		}
		if (enough)
		{
			Selection* sel = ExecuteQuery(&query, context, nil, nil, DB4D_Do_Not_Lock, 0, nil, &err);
			if (sel != nil)
			{
				if (!sel->IsEmpty())
				{
					result = sel->GetFic(0);
				}
				sel->Release();
			}
		}
		else
			err = ThrowError(VE_DB4D_IDENTKEY_MALFORMED);
	}
	else
		err = ThrowError(VE_DB4D_ENTITY_HAS_NO_IDENTKEY);

	return result;
}


sLONG EntityModel::getEntityNumWithIdentifyingAtts(const VValueBag& bagData, BaseTaskInfo* context, VError& err, bool ErrOnNull)
{
	sLONG result = -1;
	bool enough = true;
	const IdentifyingAttributeCollection* idents = GetIdentifyingAtts();
	if (!idents->empty())
	{
		SearchTab xquery(fMainTable);
		bool first = true;
		for (IdentifyingAttributeCollection::const_iterator cur = idents->begin(), end = idents->end(); cur != end; cur++)
		{
			const EntityAttribute* att = cur->fAtt;
			const VValueSingle* cv = bagData.GetAttribute(att->GetName());
			if (cv != nil)
			{
				if (first)
					first = false;
				else
					xquery.AddSearchLineBoolOper(DB4D_And);
				xquery.AddSearchLineSimple(fMainTable->GetNum(), att->GetFieldPos(), DB4D_Like, cv);
			}
			else
			{
				if (!cur->fOptionnel)
				{
					enough = false;
					break;
				}
			}
		}
		if (enough)
		{
			Selection* sel = ExecuteQuery(&xquery, context, nil, nil, DB4D_Do_Not_Lock);
			if (sel != nil && !sel->IsEmpty())
			{
				result = sel->GetFic(0);
			}
			QuickReleaseRefCountable(sel);
		}
	}
	return result;
}


sLONG EntityModel::getEntityNumWithIdentifyingAtts(const VectorOfVValue& idents, BaseTaskInfo* context, VError& err)
{
	sLONG result = -1;
	err = VE_OK;
	if (!fIdentifyingAtts.empty())
	{
		SearchTab query(fMainTable);
		bool enough = true;
		VectorOfVValue::const_iterator curkey = idents.begin();
		bool first = true;
		for (IdentifyingAttributeCollection::iterator cur = fIdentifyingAtts.begin(), end = fIdentifyingAtts.end(); cur != end; cur++, curkey++)
		{
			if (curkey == idents.end())
			{
				if (!cur->fOptionnel)
					enough = false;
				break;
			}
			if (first)
				first = false;
			else
				query.AddSearchLineBoolOper(DB4D_And);
			query.AddSearchLineSimple(fMainTable->GetNum(), cur->fAtt->GetFieldPos(), DB4D_Equal, *curkey);
		}
		if (enough)
		{
			Selection* sel = ExecuteQuery(&query, context, nil, nil, DB4D_Do_Not_Lock, 0, nil, &err);
			if (sel != nil)
			{
				if (!sel->IsEmpty())
				{
					result = sel->GetFic(0);
				}
				sel->Release();
			}
		}
		else
			err = ThrowError(VE_DB4D_IDENTKEY_MALFORMED);
	}
	else
		err = ThrowError(VE_DB4D_ENTITY_HAS_NO_IDENTKEY);

	return result;
}



EntityRecord* EntityModel::findEntityWithIdentifyingAtts(const VectorOfVString& idents, BaseTaskInfo* context, VError& err, DB4D_Way_of_Locking HowToLock)
{
	err = VE_OK;
	EntityRecord* erec = nil;
	sLONG numrec = getEntityNumWithIdentifyingAtts(idents, context, err);
	if (numrec != -1)
	{
		erec = LoadEntityRecord(numrec, err, HowToLock, context, false);
	}

	return erec;
}

EntityRecord* EntityModel::findEntityWithIdentifyingAtts(const VValueBag& idents, BaseTaskInfo* context, VError& err, DB4D_Way_of_Locking HowToLock)
{
	err = VE_OK;
	EntityRecord* erec = nil;
	sLONG numrec = getEntityNumWithIdentifyingAtts(idents, context, err);
	if (numrec != -1)
	{
		erec = LoadEntityRecord(numrec, err, HowToLock, context, false);
	}

	return erec;
}


EntityRecord* EntityModel::findEntityWithIdentifyingAtts(const VectorOfVValue& idents, BaseTaskInfo* context, VError& err, DB4D_Way_of_Locking HowToLock)
{
	err = VE_OK;
	EntityRecord* erec = nil;
	sLONG numrec = getEntityNumWithIdentifyingAtts(idents, context, err);
	if (numrec != -1)
	{
		erec = LoadEntityRecord(numrec, err, HowToLock, context, false);
	}

	return erec;
}



CDB4DEntityRecord* EntityModel::FindEntityWithIdentifyingAtts(const XBOX::VectorOfVString& idents, CDB4DBaseContext* inContext, VErrorDB4D& err, DB4D_Way_of_Locking HowToLock)
{
	return findEntityWithIdentifyingAtts(idents, ConvertContext(inContext), err, HowToLock);
}

CDB4DEntityRecord* EntityModel::FindEntityWithIdentifyingAtts(const XBOX::VValueBag& idents, CDB4DBaseContext* inContext, VErrorDB4D& err, DB4D_Way_of_Locking HowToLock)
{
	return findEntityWithIdentifyingAtts(idents, ConvertContext(inContext), err, HowToLock);
}

CDB4DEntityRecord* EntityModel::FindEntityWithIdentifyingAtts(const VectorOfVValue& idents, CDB4DBaseContext* inContext, VErrorDB4D& err, DB4D_Way_of_Locking HowToLock)
{
	return findEntityWithIdentifyingAtts(idents, ConvertContext(inContext), err, HowToLock);
}



Selection* EntityModel::SelectAllEntities(BaseTaskInfo* context, VErrorDB4D* outErr, DB4D_Way_of_Locking HowToLock, Bittab* outLockSet)
{
	VError err = VE_OK;
	Selection* sel = nil;
	if (okperm(context, fPerms[DB4D_EM_Read_Perm]) || okperm(context, fPerms[DB4D_EM_Update_Perm]) || okperm(context, fPerms[DB4D_EM_Delete_Perm]))
	{
		if (WithRestrictingQuery())
		{
			Base4D *db;
			DataTable *DF;
			DF=fMainTable->GetDF();
			db=DF->GetDB();
			OptimizedQuery rech;
			db->LockIndexes();
			err = rech.AnalyseSearch(fRestrictingQuery, context);
			db->UnLockIndexes();
			if (err == VE_OK)
			{
				sel = rech.Perform((Bittab*)nil, nil, context, err, HowToLock, 0, nil);
				if (err == VE_OK)
				{
					if (WithRestriction())
					{
						Selection* sel2 = BuildRestrictingSelection(context, err);
						if (err == VE_OK)
						{
							Bittab* b1 = sel->GenereBittab(context, err);
							Bittab* b2 = sel2->GenereBittab(context, err);

							if (b1 != nil && b2 != nil)
							{
								Bittab* b3 = b1->Clone(err);
								if (b3 != nil)
								{
									err = b3->And(b2);
									if (err == VE_OK)
									{
										Selection* sel3 = new BitSel(sel->GetParentFile(), b3);
										QuickReleaseRefCountable(sel);
										sel = sel3;
									}
								}
								QuickReleaseRefCountable(b3);
							}

							QuickReleaseRefCountable(b1);
							QuickReleaseRefCountable(b2);

						}
						if (err != VE_OK)
							ReleaseRefCountable(&sel);
						QuickReleaseRefCountable(sel2);
					}
				}
			}
		}
		else if (WithRestriction())
		{
			sel = BuildRestrictingSelection(context, err);
		}
		else
			sel = fMainTable->GetDF()->AllRecords(context, err);
	}
	else
	{
		err = ThrowError(VE_DB4D_NO_PERM_TO_READ);
		context->SetPermError();
	}
	if (outErr != nil)
		*outErr = err;
	return sel;
}


CDB4DSelection* EntityModel::SelectAllEntities(CDB4DBaseContextPtr inContext, VErrorDB4D* outErr, DB4D_Way_of_Locking HowToLock, CDB4DSet* outLockSet)
{
	CDB4DSelection* result = nil;
	Selection* sel = SelectAllEntities(ConvertContext(inContext), outErr, HowToLock, nil);
	if (sel != nil)
	{
		CDB4DBase* xbase = fOwner->RetainBaseX();
		result = new VDB4DSelection(VDBMgr::GetManager(), VImpCreator<VDB4DBase>::GetImpObject(xbase), fMainTable, sel);
		xbase->Release();
		result->SetAssociatedModel(this);
	}
	return result;
}


CDB4DQuery* EntityModel::NewQuery()
{
	return new VDB4DQuery(VDBMgr::GetManager(), fMainTable, nil, this);
}


Selection* EntityModel::query( const VString& inQuery, BaseTaskInfo* context, VErrorDB4D& err, const VValueSingle* param1, const VValueSingle* param2, const VValueSingle* param3)
{
	Selection* sel = nil;
	if (okperm(context, fPerms[DB4D_EM_Read_Perm]) || okperm(context, fPerms[DB4D_EM_Update_Perm]) || okperm(context, fPerms[DB4D_EM_Delete_Perm]))
	{
		SearchTab laquery(fMainTable);
		
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
			sel = ExecuteQuery(&laquery, context, nil, nil, DB4D_Do_Not_Lock, 0, nil, &err);
		}
	}
	else
	{
		context->SetPermError();
		err = ThrowError(VE_DB4D_NO_PERM_TO_READ);
	}

	return sel;
}


CDB4DSelection* EntityModel::Query(const VString& inQuery, CDB4DBaseContext* inContext, VErrorDB4D& err, const VValueSingle* param1, const VValueSingle* param2, const VValueSingle* param3)
{
	Selection* sel = query(inQuery, ConvertContext(inContext), err, param1, param2, param3);
	CDB4DSelection* result = nil;
	
	if (sel != nil)
	{
		CDB4DBase* xbase = fOwner->RetainBaseX();
		result = new VDB4DSelection(VDBMgr::GetManager(), VImpCreator<VDB4DBase>::GetImpObject(xbase), fMainTable, sel);
		xbase->Release();
		result->SetAssociatedModel(this);
	}

	return result;
}


CDB4DEntityRecord* EntityModel::Find(const VString& inQuery, CDB4DBaseContext* inContext, VErrorDB4D& err, const VValueSingle* param1, const VValueSingle* param2, const VValueSingle* param3)
{
	CDB4DEntityRecord* result = nil;
	CDB4DSelection* sel = Query(inQuery, inContext, err, param1, param2, param3);
	if (sel != nil && sel->CountRecordsInSelection(inContext) > 0)
	{
		result = sel->LoadEntity(1, DB4D_Do_Not_Lock, inContext);
	}
	QuickReleaseRefCountable(sel);
	return result;
}


bool EntityModel::AddRestrictionToQuery(SearchTab& query, BaseTaskInfo* context, VError& err)
{
	err = VE_OK;
	bool ok = false;
	if (fWithRestriction)
	{
		ok = true;
		Selection* sel = BuildRestrictingSelection(context, err);
		if (sel != nil)
		{
			query.AddSearchLineSel(sel);
			sel->Release();
		}
	}
	return ok;
}


Selection* EntityModel::BuildRestrictingSelection(BaseTaskInfo* context, VError& err)
{
	Selection* result = nil;
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
				result = fMainTable->GetDF()->AllRecords(context, err);
		}
	}

	return result;
}


Selection* EntityModel::ExecuteQuery( SearchTab* querysearch, BaseTaskInfo* context, Selection* Filter, 
										  VDB4DProgressIndicator* InProgress, DB4D_Way_of_Locking HowToLock, 
										  sLONG limit, Bittab* outLockSet, VErrorDB4D *outErr)
{
	VError err = VE_OK;
	Selection *sel = nil;
	if (okperm(context, fPerms[DB4D_EM_Read_Perm]) || okperm(context, fPerms[DB4D_EM_Update_Perm]) || okperm(context, fPerms[DB4D_EM_Delete_Perm]))
	{
		Base4D *db;
		DataTable *DF;
		OptimizedQuery rech;

		{
			Selection* oldsel = nil;

			if (testAssert( querysearch != nil))
			{

				DF=fMainTable->GetDF();
				db=DF->GetDB();

				SearchTab* xsearch;
				SearchTab locsearch(fMainTable);
				SearchTab restsearch(fMainTable);
				bool alreadyloc = false;

				if (AddRestrictionToQuery(restsearch, context, err))
				{
					if (!alreadyloc)
					{
						locsearch.From(*querysearch);
						xsearch = &locsearch;
					}
					locsearch.Add(restsearch);
					alreadyloc = true;
				}
				if (WithRestrictingQuery())
				{
					if (!alreadyloc)
					{
						locsearch.From(*querysearch);
						xsearch = &locsearch;
					}
					locsearch.Add(*fRestrictingQuery);
					alreadyloc = true;
				}
				if (!alreadyloc)
					xsearch = querysearch;

				db->LockIndexes();
				err = rech.AnalyseSearch(xsearch, context);
				db->UnLockIndexes();
				if (err == VE_OK)
				{
					sel = rech.Perform(Filter, InProgress, context, err, HowToLock, 0, outLockSet);
				}


			}
		}
	}
	else
	{
		context->SetPermError();
		err = ThrowError(VE_DB4D_NO_PERM_TO_READ);
	}

	if (outErr != nil)
		*outErr = err;
	return sel;

}


CDB4DSelection* EntityModel::ExecuteQuery( CDB4DQuery *inQuery, CDB4DBaseContextPtr inContext, CDB4DSelectionPtr Filter, 
									 VDB4DProgressIndicator* InProgress, DB4D_Way_of_Locking HowToLock, 
									 sLONG limit, CDB4DSet* outLockSet, VErrorDB4D *outErr)
{
	Selection *sel;
	CDB4DSelection* result = nil;
	Base4D *db;
	DataTable *DF;
	OptimizedQuery rech;
	VError err = VE_OK;
	
	{
		assert(inQuery != nil);
		VDB4DQuery *query = VImpCreator<VDB4DQuery>::GetImpObject(inQuery);
		VDB4DSelection *xfilter;
		if (Filter == nil) 
			xfilter = nil;
		else 
			xfilter = VImpCreator<VDB4DSelection>::GetImpObject(Filter);
		Selection* oldsel = nil;
		
		BaseTaskInfoPtr context = ConvertContext(inContext);
		
		Bittab* lockedset = nil;
		if (outLockSet != nil)
			lockedset = (VImpCreator<VDB4DSet>::GetImpObject(outLockSet))->GetBittab();
		
		if (testAssert( query != nil)) {
			
			if (xfilter != nil) 
				oldsel = xfilter->GetSel();

			sel = ExecuteQuery(query->GetSearchTab(), context, oldsel, InProgress, HowToLock, limit, lockedset, &err);
			
			if (sel != nil)
			{
				CDB4DBase* xbase = RetainDataBase();
				result = new VDB4DSelection(VDBMgr::GetManager(), VImpCreator<VDB4DBase>::GetImpObject(xbase), fMainTable, sel, this);
				xbase->Release();
				if (result == nil)
				{
					err = memfull;
					sel->Release();
				}
			}
			
		}
	}
	
	if (outErr != nil)
		*outErr = err;
	return result;
	
}



CDB4DSelection* EntityModel::NewSelection(DB4D_SelectionType inSelectionType) const
{
	Selection* sel = nil;
	CDB4DSelectionPtr result = nil;
	
	if (inSelectionType == DB4D_Sel_OneRecSel)
		inSelectionType = DB4D_Sel_SmallSel;
	
	switch (inSelectionType)
	{
		case DB4D_Sel_SmallSel:
			sel = new PetiteSel(fMainTable->GetDF(), fMainTable->GetOwner(), fMainTable->GetNum());
			break;
			
		case DB4D_Sel_LongSel:
			sel = new LongSel(fMainTable->GetDF(), fMainTable->GetOwner());
			((LongSel*)sel)->PutInCache();
			break;
			
		case DB4D_Sel_Bittab:
			sel = new BitSel(fMainTable->GetDF(), fMainTable->GetOwner());
			break;
	}
	
	if (sel != nil)
	{
		CDB4DBase* xbase = RetainDataBase();
		result = new VDB4DSelection(VDBMgr::GetManager(), VImpCreator<VDB4DBase>::GetImpObject(xbase), fMainTable, sel, (EntityModel*)this);
		xbase->Release();
	}
	
	return result;
}


CDB4DEntityRecord* EntityModel::NewEntity(CDB4DBaseContextPtr inContext, DB4D_Way_of_Locking HowToLock) const
{
	VError err;
	BaseTaskInfo* context = ConvertContext(inContext);
	FicheInMem* rec = fMainTable->GetDF()->NewRecord(err, context);
	if (rec == nil)
		return nil;
	else
	{
		EntityRecord* erec = new EntityRecord((EntityModel*)this, rec, inContext, HowToLock);
		rec->Release();
		//CallDBEvent(dbev_init, erec, context);
		erec->CallInitEvent(context);
		return erec;
	}
}


EntityRecord* EntityModel::NewEntity(BaseTaskInfo* inContext, DB4D_Way_of_Locking HowToLock) const
{
	VError err;
	FicheInMem* rec = fMainTable->GetDF()->NewRecord(err, inContext);
	if (rec == nil)
		return nil;
	else
	{
		EntityRecord* erec = new EntityRecord((EntityModel*)this, rec, inContext->GetEncapsuleur(), HowToLock);
		rec->Release();
		//CallDBEvent(dbev_init, erec, inContext);
		erec->CallInitEvent(inContext);
		return erec;
	}
}

/*
map<Table*, EntityModel*> EntityModel::sEMbyTable;
VCriticalSection EntityModel::sEMbyTableMutex;
*/

#if BuildEmFromTable

EntityModel* EntityModel::BuildEntityModel(Table* from, EntityModelCatalog* catalog)
{
	assert(from != nil);
	VString tablename;
	from->GetName(tablename);
	EntityModel* em = catalog->RetainEntity(tablename);
	if (em == nil)
	{
		em = new EntityModel(from->GetOwner(), from);
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
				if (cri != nil)
				{
					VString fieldname;
					cri->GetName(fieldname);
					EntityAttribute* att = new EntityAttribute(em);
					att->SetKind(eattr_storage);
					att->SetFieldPos(i);
					att->SetName(fieldname);
					att->SetOverWrite(true);

					em->fAttributes.push_back(att);
					em->fAttributesByName[att->GetName()] = att;
					att->SetPosInOwner((sLONG)em->fAttributes.size());

					cri->Release();
				}
			}

			catalog->AddOneEntityModel(em, true);

			DepRelationArrayIncluded* relsNto1 = from->GetRelNto1Deps();
			for (DepRelationArrayIncluded::Iterator cur = relsNto1->First(), end = relsNto1->End(); cur != end; cur++)
			{
				Relation* rel = *cur;
				VString relname = rel->GetName();
				if (!relname.IsEmpty())
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

			//if (false)
			{
				DepRelationArrayIncluded* rels1toN = from->GetRel1toNDeps();
				for (DepRelationArrayIncluded::Iterator cur = rels1toN->First(), end = rels1toN->End(); cur != end; cur++)
				{
					Relation* rel = *cur;
					VString relname = rel->GetOppositeName();
					if (!relname.IsEmpty())
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

			if (from->HasPrimKey())
			{
				NumFieldArray primkey;
				from->CopyPrimaryKey(primkey);
				for (NumFieldArray::Iterator cur = primkey.First(), end = primkey.End(); cur != end; cur++)
				{
					sLONG numfield = *cur;
					EntityAttribute* att = em->FindAttributeByFieldPos(numfield);
					if (att != nil)
						em->fOwnedPrimaryAtts.push_back(IdentifyingAttribute(att));
				}
				if (!em->fOwnedPrimaryAtts.empty())
					em->fPrimaryAtts = em->fOwnedPrimaryAtts;
			}
		}

	}
	
	return em;
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


bool EntityModel::PermissionMatch(DB4D_EM_Perm inPerm, CUAGSession* inSession) const
{
	return permissionMatch(inPerm, inSession);
}


bool EntityModel::permissionMatch(DB4D_EM_Perm inPerm, CUAGSession* inSession) const
{
	if (inPerm >= DB4D_EM_Read_Perm && inPerm <= DB4D_EM_Promote_Perm)
	{
		const VUUID* id = &(fPerms[inPerm]);
		if (id->IsNull())
			return true;
		else
			return inSession->BelongsTo(*id);
	}
	else
		return false;
}


VError EntityModel::callMethod(const VString& inMethodName, const VectorOfVString& params, VJSValue& result, CDB4DBaseContext* inContext, Selection* inSel, EntityRecord* inRec)
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
		cur->GetJSONString(xjs, JSON_FormatDateIso | JSON_WithQuotesIfNecessary);
		jsonparams += xjs;
	}
	jsonparams += L"]";
	return callMethod(inMethodName, jsonparams, result, inContext, inSel, inRec);
}


VError EntityModel::callMethod(const VString& inMethodName, const VString& jsonparams, VJSValue& result, CDB4DBaseContext* inContext, Selection* inSel, EntityRecord* inRec)
{
	VError err = VE_OK;
	EntityMethod* meth = getMethod(inMethodName);
	if (meth != nil)
	{
		BaseTaskInfo* context = ConvertContext(inContext);
		if (context != nil && context->GetJSContext() != nil)
		{
			VJSContext jscontext(context->GetJSContext());
			VUUID idexec, idpromote;
			meth->GetPermission(DB4D_EM_Execute_Perm, idexec);
			meth->GetPermission(DB4D_EM_Promote_Perm, idpromote);
			if (okperm(context, idexec))
			{
				VJSJSON json(jscontext);
				JS4D::ExceptionRef excep = nil;
				VJSValue params(json.Parse(jsonparams, &excep));
				if (excep == nil)
				{
					vector<VJSValue> paramsValues;
					if (!jsonparams.IsEmpty())
					{
						if (params.IsInstanceOf( CVSTR( "Array")))
						{
							VJSObject objarray(jscontext);
							params.GetObject(objarray);
							VJSArray xarray(objarray);
							for (sLONG i = 0, nb = xarray.GetLength(); i < nb; i++)
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

				if (excep != nil)
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


VError EntityModel::callMethod(const VString& inMethodName, const VValueBag& bagparams, VJSValue& result, CDB4DBaseContext* inContext, Selection* inSel, EntityRecord* inRec)
{
	VString jsonstring;
	bagparams.GetJSONString(jsonstring);
	return callMethod(inMethodName, jsonstring, result, inContext);
}


class computeResult
{
	public:
		computeResult()
		{
			fSum = 0;
			first = true;
			fMinVal = nil;
			fMaxVal = nil;
			fCount = 0;
		}

		~computeResult()
		{
			if (fMinVal != nil)
				delete fMinVal;
			if (fMaxVal != nil)
				delete fMaxVal;
		}

		VValueSingle* fMinVal;
		VValueSingle* fMaxVal;
		Real fSum;
		sLONG8 fCount;
		bool first;
};

VError EntityModel::compute(EntityAttributeSortedSelection& atts, Selection* sel, VJSObject& outObj, BaseTaskInfo* context, JS4D::ContextRef jscontext)
{
	outObj.MakeEmpty();

	vector<computeResult> results;
	results.resize(atts.size());
	VError err = VE_OK;
	if (okperm(context, fPerms[DB4D_EM_Read_Perm]) || okperm(context, fPerms[DB4D_EM_Update_Perm]) || okperm(context, fPerms[DB4D_EM_Delete_Perm]))
	{
		if (sel == nil)
			sel = SelectAllEntities(context, &err);
		else
			sel->Retain();

		if (err == VE_OK)
		{
			SelectionIterator itersel(sel);
			sLONG currec = itersel.FirstRecord();

			while (currec != -1 && err == VE_OK)
			{
				EntityRecord* rec = LoadEntityRecord(currec, err, DB4D_Do_Not_Lock, context, false);
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
				currec = itersel.NextRecord();
			}


		}

		sLONG respos = 0;
		for (EntityAttributeSortedSelection::const_iterator cur = atts.begin(), end = atts.end(); cur != end && err == VE_OK; cur++, respos++)
		{
			computeResult& result = results[respos];
			VJSObject subobject(outObj, cur->fAttribute->GetName());
			subobject.SetProperty("count", result.fCount);
			if (!result.first)
			{
				if (cur->fAttribute->IsSummable())
				{
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
					subobject.SetNullProperty("average");
					subobject.SetProperty("sum", 0.0);
				}
				subobject.SetNullProperty("min");
				subobject.SetNullProperty("max");
			}
		}

		QuickReleaseRefCountable(sel);
	}
	else
	{
		context->SetPermError();
		err = ThrowError(VE_DB4D_NO_PERM_TO_READ);
	}
	return err;
}


VError EntityModel::CallDBEvent(DBEventKind kind, EntityRecord* inRec, BaseTaskInfo* context) const
{
	VError err = VE_OK;
	if (kind != dbev_none)
	{
		const DBEvent& ev = fEvents[kind];
		if (ev.IsValid())
		{
			err = ev.Call(inRec, context, nil, nil);
		}
	}
	return err;
}


VError EntityModel::CallDBEvent(DBEventKind kind, BaseTaskInfo* context, Selection* *outSel) const
{
	VError err = VE_OK;
	if (kind != dbev_none)
	{
		const DBEvent& ev = fEvents[kind];
		if (ev.IsValid())
		{
			err = ev.Call(nil, context, nil, this, outSel);
		}
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
				err = att->CallDBEvent(kind, inRec, context);
			}
		}
	}
	return err;
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

}



// ----------------------------------------------------------------------------------------------------


SubEntityCache::~SubEntityCache()
{
	QuickReleaseRefCountable(fErec);
	QuickReleaseRefCountable(fSel);
	QuickReleaseRefCountable(fRec);
}


void SubEntityCache::Clear()
{
	ReleaseRefCountable(&fErec);
	ReleaseRefCountable(&fSel);
	ReleaseRefCountable(&fRec);
	fAlreadyActivated = false;
}



EntityRecord* SubEntityCache::GetErec()
{
	if (fErec == nil && fRec != nil)
	{
		fErec = new EntityRecord(fModel, fRec, fRec->GetContext()->GetEncapsuleur(), fLockWay);
		fErec->CallDBEvent(dbev_load, fRec->GetContext());
	}
	return fErec;
}



// ----------------------------------------------------------------------------------------------------


#if debuglr
CComponent* EntityAttributeValue::Retain(const char* DebugInfo)
{
	return VComponentImp<CDB4DEntityAttributeValue>::Retain(DebugInfo);
}

void EntityAttributeValue::Release(const char* DebugInfo)
{
	VComponentImp<CDB4DEntityAttributeValue>::Release(DebugInfo);
}
#endif


EntityAttributeValue::EntityAttributeValue(EntityRecord* owner, const EntityAttribute* attribute, EntityAttributeValueKind kind, VValueSingle* inVal, bool isowned ):fJSObject(nil)
{
	_Init(kind);
	fValue = inVal;
	fIsValueOwned = isowned;
	fOwner = owner;
	fAttribute = attribute;
}


EntityAttributeValue::EntityAttributeValue(EntityRecord* owner, const EntityAttribute* attribute, EntityAttributeValueKind kind, EntityRecord* erec, EntityModel* inSubModel):fJSObject(nil)
{
	_Init(kind);
	fSubEntity = RetainRefCountable(erec);
	fSubModel = inSubModel;
	fOwner = owner;
	fAttribute = attribute;
	if (fSubEntity != nil)
		fSubModificationStamp = fSubEntity->GetModificationStamp();
}


EntityAttributeValue::EntityAttributeValue(EntityRecord* owner, const EntityAttribute* attribute, EntityAttributeValueKind kind, Selection* sel, EntityModel* inSubModel):fJSObject(nil)
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
		JS4D::UnprotectValue(fJSObject.GetContextRef(), fJSObject);
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
	QuickReleaseRefCountable(fCDB4DSel);
	QuickReleaseRefCountable(fSel);
	QuickReleaseRefCountable(fValueBag);
	QuickReleaseRefCountable(fOldSubEntity);
	if (fRelatedKey != nil)
		delete fRelatedKey;
}


EntityAttributeValue* EntityAttributeValue::Clone(EntityRecord* newOwner)
{
	EntityAttributeValue* result = new EntityAttributeValue(newOwner, fAttribute, fType, nil, false);

	result->fJsonString = fJsonString;
	result->fValueBag = RetainRefCountable(fValueBag);
	result->fOldSubEntity = RetainRefCountable(fOldSubEntity);
	VValueSingle* cv = fValue;
	if (cv != nil)
		cv = cv->Clone();
	result->fValue = cv;
	fIsValueOwned = true;

	if (fRelatedKey != nil)
		result->fRelatedKey = fRelatedKey->Clone();
	result->fSubEntity = RetainRefCountable(fSubEntity);
	result->fSubModel = fSubModel;
	result->fSel = RetainRefCountable(fSel);
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


CDB4DEntityRecord* EntityAttributeValue::GetRelatedEntity() const
{
	return ((EntityAttributeValue*)this)->getRelatedEntity();
}


CDB4DSelection* EntityAttributeValue::GetRelatedSelection() const
{
	if (fSel == nil)
		return nil;
	else
	{
		if (fCDB4DSel == nil)
		{
			CDB4DBase* base = fSubModel->GetOwner()->RetainBaseX();
			fCDB4DSel = new VDB4DSelection(VDBMgr::GetManager(), VImpCreator<VDB4DBase>::GetImpObject(base), fSubModel->GetMainTable(), fSel, fSubModel);
			base->Release();
			fSel->Retain();
		}
		return fCDB4DSel;
	}
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
				VJSArray arr(outJSObject.GetContextRef());
				EntityAttributeSortedSelection atts(fOwner->GetOwner());
				SelToJSObject(fOwner->getContext(), arr, fOwner->GetOwner(), fSel, atts, nil, nil, false, false, 0, fSel->GetQTfic());
				outJSObject = arr;
			}
			if (err == VE_OK)
			{
				fJSObject = outJSObject;
				JS4D::ProtectValue(outJSObject.GetContextRef(), fJSObject);
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
		JS4D::UnprotectValue(fJSObject.GetContextRef(), fJSObject);
	}
	fJSObject = inJSObject;
	JS4D::ProtectValue(inJSObject.GetContextRef(), fJSObject);
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

	}

	if (err == VE_OK)
	{
		err = fAttribute->CallDBEvent(dbev_validate, fOwner, context);
	}
	return err;
}



VError EntityAttributeValue::Save(Transaction* &trans, BaseTaskInfo* context)
{
	VError err = VE_OK;

	if (fCanBeModified)
	{
		switch(fType)
		{
			case eav_vvalue:
				{
					if (fAttribute->GetKind() != eattr_computedField)
					{
						if (fStamp != 0)
						{
							FicheInMem* rec = fOwner->GetRecordForAttribute(fAttribute);
							if (rec != nil)
							{
								err = fAttribute->CallDBEvent(dbev_save, fOwner, context);
								if (err == VE_OK)
									rec->Touch(fAttribute->GetFieldPos());
							}
						}
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
						if (fSubEntity->GetStamp() != 0)
						{
							if (trans == nil && context != nil)
								trans = context->StartTransaction(err);
							sLONG modifstamp = fSubEntity->GetModificationStamp();
							if (modifstamp == 0 && fSubEntity->IsNew())
								modifstamp = 1;
							err = fSubEntity->Save(&trans, context, modifstamp);
						}
					}

					if (fStamp != 0 && err == VE_OK)
					{
						err = fAttribute->CallDBEvent(dbev_save, fOwner, context);
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
								FicheInMem* rec = fOwner->getRecord();
								if (rec != nil)
								{
									VValueSingle* cv = rec->GetFieldValue(rel->GetSourceField(), err);

									FicheInMem* relatedRec = nil;
									if (fSubEntity != nil)
										relatedRec = fSubEntity->getRecord();
									if (relatedRec == nil)
									{
										if (fRelatedKey != nil)
										{
											cv->FromValue(*fRelatedKey);
										}
										else
										{
											cv->Clear();
											cv->SetNull(true);
										}
									}
									else
									{
										VValueSingle* relatedCV = relatedRec->GetFieldValue(rel->GetDestField(), err);
										if (relatedCV == nil)
										{
											cv->Clear();
											cv->SetNull(true);
										}
										else
										{
											cv->FromValue(*relatedCV);
										}
									}
									rec->Touch(rel->GetSourceField());
								}
							}
						}
						else
							err = fAttribute->ThrowError(VE_DB4D_ENTITY_RELATION_IS_EMPTY);
					}

				}
				break;

			case eav_composition:
				{
					Selection* sel = getRelatedSelection();
					EntityModel* relEm = getRelatedEntityModel();
					if (relEm != nil)
					{
						err = fAttribute->CallDBEvent(dbev_save, fOwner, context);

						if (err == VE_OK && sel != nil && sel->GetQTfic() > 0)
						{
							if (trans == nil && context != nil)
								trans = context->StartTransaction(err);
							err = sel->DeleteRecords(context, nil, nil, nil, nil, relEm);
						}
						if (err == VE_OK)
						{
							if (fJSObjectIsValid)
							{
								if (fJSObject.IsInstanceOf("Array"))
								{
									VJSArray arr(fJSObject, false);
									VJSValue jsval(fJSObject.GetContextRef());
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
											VJSObject objrec(fJSObject.GetContextRef());
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
			if (fSubEntity->GetNum() == relatedEntity->GetNum())
				return true;
			else
				return false;
		}
	}
}

void EntityAttributeValue::Touch(BaseTaskInfo* context)
{
	fAttribute->CallDBEvent(dbev_set, fOwner, context);
	fStamp++;
	fOwner->Touch(context);
}


VError EntityAttributeValue::SetRelatedEntity(EntityRecord* relatedEntity, BaseTaskInfo* context)
{
	VError err = VE_OK;
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

			fSubEntity = RetainRefCountable(relatedEntity);
			if (fSubEntity != nil)
				fSubModificationStamp = fSubEntity->GetModificationStamp();
			fOwner->ClearSubEntityCache(fAttribute->GetPathID());
		}
		Touch(context);
	}

	return err;
}




// -----------------------------------------------


#if debuglr
CComponent* EntityRecord::Retain(const char* DebugInfo)
{
	return VComponentImp<CDB4DEntityRecord>::Retain(DebugInfo);
}

void EntityRecord::Release(const char* DebugInfo)
{
	VComponentImp<CDB4DEntityRecord>::Release(DebugInfo);
}
#endif



EntityRecord::~EntityRecord()
{
	QuickReleaseRefCountable(fCDB4DMainRec);
	QuickReleaseRefCountable(fMainRec);
	for (EntityAttributeValueCollection::iterator cur = fValues.begin(), end = fValues.end(); cur != end; cur++)
	{
		if (*cur != nil)
			(*cur)->Release();
	}
}


EntityRecord::EntityRecord(EntityModel* inModel, FicheInMem* inMainRec, CDB4DBaseContext* inContext, DB4D_Way_of_Locking HowToLock)
{
	fWayOfLock = HowToLock;
	fStamp = 0;
	fCDB4DMainRec = nil;
	fContext = inContext;
	assert(inModel != nil && inMainRec != nil);
	fModel = inModel;
	fMainRec = RetainRefCountable(inMainRec);
	fModificationStamp = fMainRec->GetModificationStamp();
	assert( inMainRec->GetOwner() == fModel->GetMainTable());
	sLONG nbelem = fModel->CountAttributes();
	fValues.resize(nbelem, nil);

	sLONG nbpaths = fModel->CountRelationPaths();
	fSubEntitiesCache.resize(nbpaths);
	std::fill(&fAlreadyCallEvent[0], &fAlreadyCallEvent[dbev_lastEvent+1], 0);
	fAlreadySaving = false;
	fAlreadyDeleting = false;
}


CDB4DRecord* EntityRecord::GetRecord() const
{
	if (fMainRec == nil)
		return nil;
	else
	{
		if (fCDB4DMainRec == nil)
		{
			fCDB4DMainRec = new VDB4DRecord(VDBMgr::GetManager(), fMainRec, fContext);
			fMainRec->Retain();
		}
		return fCDB4DMainRec;
	}
}


VError EntityRecord::ThrowError( VError inErrCode, const VString* p1) const
{
	VErrorDB4D_OnEMRec *err = new VErrorDB4D_OnEMRec(inErrCode, noaction, fModel->GetOwner(), fModel, GetNum());
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
			for (sLONG i = 0, nbpath = fSubEntitiesCache.size(); i < nbpath; ++i)
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
			//assert ?
			break;
	}
	return result;
}


EntityAttributeValue* EntityRecord::getAttributeValue(const EntityAttribute* inAttribute, VError& err, BaseTaskInfo* context, bool restrictValue)
{
	err = VE_OK;
	EntityAttributeValue* result = nil;
	assert(inAttribute != nil);
	if (inAttribute == nil)
		return nil;
	assert(inAttribute->GetOwner() == fModel);

	sLONG pos = inAttribute->GetPosInOwner();
	assert(pos > 0 && pos <= fValues.size());

	result = fValues[pos-1];
	EntityAttributeKind kind = inAttribute->GetKind();

	if (result == nil || kind == eattr_computedField || kind == eattr_alias)
	{
		bool mustCallEvent = (result == nil);

		switch(kind)
		{
			case eattr_storage:
			case eattr_alias:
				{
					if (kind == eattr_alias/* && !inAttribute->isFlattenedFromField()*/)
					{
						bool needReload = true /*SubEntityCacheNeedsActivation(inAttribute->GetPathID()) ||  result == nil */;
						if (needReload)
						{
							VValueSingle* cv3 = nil;
							if (result == nil)
								result = new EntityAttributeValue(this, inAttribute, eav_vvalue, nil, true);
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
							result->setVValue(cv3);
						}
					}
					else
					{
						FicheInMem* rec = nil;
						if (kind == eattr_storage)
							rec = fMainRec;
						else
						{
							SubEntityCache* cache = GetSubEntityCache(inAttribute->GetPathID(), err, true, inAttribute->GetSubEntityModel(), context);
							if (cache != nil)
								rec = cache->GetRecord();
						}
						if (rec != nil)
						{
							sLONG fieldtyp = 0;
							if (kind == eattr_storage)
								fieldtyp = fModel->GetMainTable()->GetFieldType(inAttribute->GetFieldPos());
							else
								fieldtyp = inAttribute->getFlattenTableDest()->GetFieldType(inAttribute->GetFieldPos());
							if (restrictValue && (fieldtyp == VK_IMAGE || fieldtyp == VK_BLOB || fieldtyp == VK_BLOB_DB4D))
							{
								if (!ContainsBlobData(inAttribute, context))
									result = nil;
								else
								{
									if (fieldtyp == VK_IMAGE)
										result = (EntityAttributeValue*)-2;
									else
										result = (EntityAttributeValue*)-3;
								}
							}
							else
							{
								VValueSingle* cv = rec->GetNthField(inAttribute->GetFieldPos(), err);
								if (err == VE_OK && cv != nil)
								{
									result = new EntityAttributeValue(this, inAttribute, eav_vvalue, cv);
									if (result == nil)
										err = ThrowBaseError(memfull);
								}
							}
						}
					}
				}
				break;

			case eattr_relation_Nto1:
				{
					/* pas pour l'instant
					if (restrictValue)
						result = (EntityAttributeValue*)-4;
					else
					*/
					{
						SubEntityCache* cache = GetSubEntityCache(inAttribute->GetPathID(), err, true, inAttribute->GetSubEntityModel(), context);
						if (cache != nil)
						{
							EntityRecord* erec = cache->GetErec();
							result = new EntityAttributeValue(this, inAttribute, eav_subentity, erec, inAttribute->GetSubEntityModel());
							if (result == nil)
								err = ThrowBaseError(memfull);
						}
						/* old code
						FicheInMem* rec = nil;
						SubEntityCache* cache = GetSubEntityCache(inAttribute->GetPathID(), err, true, inAttribute->GetSubEntityModel(), context);
						if (cache != nil)
							rec = cache->GetRecord();
						EntityRecord* erec = nil;
						if (rec != nil)
						{
							erec = new EntityRecord(inAttribute->GetSubEntityModel(), rec, fContext, fWayOfLock);
							erec->CallDBEvent(dbev_load, context);

						}
						result = new EntityAttributeValue(this, inAttribute, eav_subentity, erec, inAttribute->GetSubEntityModel());
						if (result == nil)
							err = ThrowBaseError(memfull);
						QuickReleaseRefCountable(erec);
						*/
					}
				}
				break;

			case eattr_relation_1toN:
				{
					if (restrictValue)
						result = (EntityAttributeValue*)-5;
					else
					{
						Selection* sel = nil;
						SubEntityCache* cache = GetSubEntityCache(inAttribute->GetPathID(), err, false, inAttribute->GetSubEntityModel(), context);
						if (cache != nil)
						sel = cache->GetSel();
						result = new EntityAttributeValue(this, inAttribute, eav_selOfSubentity, sel, inAttribute->GetSubEntityModel());
						if (result == nil)
							err = ThrowBaseError(memfull);
					}

				}
				break;

			case eattr_composition:
				{
					Selection* sel = nil;
					SubEntityCache* cache = GetSubEntityCache(inAttribute->GetPathID(), err, false, inAttribute->GetSubEntityModel(), context);
					if (cache != nil)
						sel = cache->GetSel();
					result = new EntityAttributeValue(this, inAttribute, eav_composition, sel, inAttribute->GetSubEntityModel());
					if (result == nil)
						err = ThrowBaseError(memfull);
				}
				break;

			case eattr_computedField:
				{
					if (result == nil)
						result = new EntityAttributeValue(this, inAttribute, eav_vvalue, nil, true);
					VValueSingle* cv = nil;

					if (inAttribute->GetScriptKind() == script_db4d)
					{
						OptimizedQuery* script = inAttribute->GetScriptDB4D(context);
						if (script != nil)
						{
							cv = script->Compute(this, context, err);
						}
					}
					else
					{
						if (context->GetJSContext() != nil)
						{
							VJSContext jscontext(context->GetJSContext());
							VJSObject therecord(jscontext, VJSEntitySelectionIterator::CreateInstance(jscontext, new EntitySelectionIterator(this, context->GetEncapsuleur())));
							VJSValue result(jscontext);
							VJSObject* objfunc = nil;
							VJSObject localObjFunc(jscontext);
							if (inAttribute->GetScriptObjFunc(script_attr_get, context->GetJSContext(), context, localObjFunc))
								objfunc = &localObjFunc;
							if (objfunc == nil)
								objfunc = context->getContextOwner()->GetJSFunction(inAttribute->GetScriptNum(script_attr_get), inAttribute->GetScriptStatement(script_attr_get), nil);
							
							//JSObjectSetProperty(*jscontext, therecord, name, funcref, kJSPropertyAttributeDontEnum, nil);
							JS4D::ExceptionRef excep = nil;
							if (objfunc != nil && therecord.CallFunction(*objfunc, nil, &result, &excep))
							{
								cv = result.CreateVValue(nil);
							}

							if (excep != nil)
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

		}

		if (result != nil && result != (EntityAttributeValue*)-2 && result != (EntityAttributeValue*)-3 && result != (EntityAttributeValue*)-4 && result != (EntityAttributeValue*)-5)
		{
			if (mustCallEvent)
			{
				inAttribute->CallDBEvent(dbev_load, this, context);
			}

			result->AllowModifications(inAttribute->CanBeModified());
			fValues[pos-1] = result;
		}
	}


	return result;
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



CDB4DEntityAttributeValue* EntityRecord::GetAttributeValue(const VString& inAttributeName, VError& err)
{
	return getAttributeValue(inAttributeName, err, ConvertContext(fContext));
}

CDB4DEntityAttributeValue* EntityRecord::GetAttributeValue(sLONG pos, VError& err)
{
	return getAttributeValue(pos, err, ConvertContext(fContext));
}


CDB4DEntityAttributeValue* EntityRecord::GetAttributeValue(const CDB4DEntityAttribute* inAttribute, VError& err)
{
	return getAttributeValue(VImpCreator<EntityAttribute>::GetImpObject(inAttribute), err, ConvertContext(fContext));
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
			err = inAttribute->CallDBEvent(dbev_set, this, ConvertContext(fContext));
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


VError EntityRecord::setAttributeValue(const EntityAttribute* inAttribute, const VValueSingle* inValue)
{
	VError err = VE_OK;

	BaseTaskInfo* context = ConvertContext(fContext);
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
							VJSObject therecord(jscontext, VJSEntitySelectionIterator::CreateInstance(jscontext, new EntitySelectionIterator(this, fContext)));
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

								JS4D::ExceptionRef excep;
								if (therecord.CallFunction(*objfunc, &paramvalues, &result, &excep))
								{
									
								}

								if (excep != nil)
								{
									ThrowJSExceptionAsError(jscontext, excep);
									err = ThrowError(VE_DB4D_JS_ERR);
								}
								else
									err = inAttribute->CallDBEvent(dbev_set, this, context);
							}
						}
					}

				}
				else
				{
					VValueSingle* cv = val->getVValue();
					if (cv != nil)
					{
						if (cv != inValue)
						{
							if (inValue == nil)
								cv->SetNull(true);
							else
							{
								if (cv->GetValueKind() == VK_IMAGE && inValue->GetValueKind() == VK_STRING)
								{
									VString path;
									inValue->GetString(path);
									cv->SetOutsidePath(path);
									VString computedPath;
									cv->GetOutsidePath(computedPath);
									VFile xfile(computedPath, FPS_POSIX);
									if (xfile.Exists())
										cv->ReloadFromOutsidePath();
								}
								else
									cv->FromValue(*inValue);
							}
						}
						val->Touch(context);
						//err = CallDBEvent(dbev_set, ConvertContext(fContext));
					}
					else
						err = inAttribute->ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_IS_NOT_A_VVALUE);
				}
			}
			else
				err = inAttribute->ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_IS_READ_ONLY);
		}
		else if (val->GetAttributeKind() == eav_subentity)
		{
			if (inValue == nil || inValue->IsNull())
				err = setAttributeValue(inAttribute, (EntityRecord*)nil);
			else
			{
				VectorOfVValue key(false);
				key.push_back(inValue);
				err = setAttributeValue(inAttribute, key);
			}
			ClearSubEntityCache(inAttribute->GetPathID());
		}
		else if (val->GetAttributeKind() == eav_composition)
		{
			if (inValue == nil || inValue->IsNull())
			{
				BaseTaskInfo* context = ConvertContext(fContext);
				if (context->GetJSContext() != nil)
				{
					VJSContext jscontext(context->GetJSContext());
					VJSArray jsemptyArray(jscontext);
					val->SetJSObject(jsemptyArray);
				}
				val->Touch(context);
				//err = CallDBEvent(dbev_set, context);
			}
		}
		else
			err = inAttribute->ThrowError(VE_DB4D_ENTITY_ATTRIBUTE_IS_NOT_A_VVALUE);
	}

	return err;
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
				/*
				if (relatedEM->HasIdentifyingAtts())
				{
					EntityRecord* rec = relatedEM->findEntityWithIdentifyingAtts(inIdentKey, context, err, DB4D_Do_Not_Lock);
					if (rec == nil)
					{
						rec = relatedEM->NewEntity(context, DB4D_Do_Not_Lock);
						rec->setIdentifyingAtts(inIdentKey);
					}
					if (rec != nil)
					{
						val->SetRelatedEntity(rec);
						rec->Release();
					}
				}
				else 
				*/
				if (relatedEM->HasPrimKey())
				{
					val->setRelatedKey(inIdentKey);
					/*
					EntityRecord* rec = relatedEM->findEntityWithPrimKey(inIdentKey, context, err, DB4D_Do_Not_Lock);
					if (rec != nil)
					{
						val->SetRelatedEntity(rec, context);
						rec->Release();
					}
					*/
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
		//err = CallDBEvent(dbev_set, ConvertContext(fContext));
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


bool EntityRecord::ContainsBlobData(const EntityAttribute* inAttribute, BaseTaskInfo* context)
{
	bool result = false;

	EntityAttributeKind kind = inAttribute->GetKind();
	if (kind == eattr_storage || kind == eattr_alias)
	{
		FicheInMem* rec = nil;
		if (kind == eattr_storage)
			rec = fMainRec;
		else
		{
			VError err;
			SubEntityCache* cache = GetSubEntityCache(inAttribute->GetPathID(), err, true, inAttribute->GetSubEntityModel(), context);
			if (cache != nil)
				rec = cache->GetRecord();
		}
		if (rec != nil)
		{
			FicheOnDisk* assoc = rec->GetAssoc();
			if (assoc != nil)
			{
				void* p = assoc->GetDataPtr(inAttribute->GetFieldPos());
				if (p != nil)
				{
					if (*((sLONG*)p) != -1)
						result = true;
				}
			}
		}
	}

	return result;
}

VErrorDB4D EntityRecord::Save(uLONG stamp, bool allowOverrideStamp)
{
	return Save(nil, ConvertContext(fContext), stamp, allowOverrideStamp);
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

	/*
	for (EntityAttributeValueCollection::iterator cur = fValues.begin(), end = fValues.end(); cur != end && err == VE_OK; cur++)
	{
		EntityAttributeValue* val = *cur;
		if (val != nil)
		{
			err = val->Validate(context);
			if (err != VE_OK)
				validFail = true;
		}
	}
	*/

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


VError EntityRecord::Save(Transaction* *trans, BaseTaskInfo* context, uLONG stamp, bool allowOverrideStamp)
{
	VError err = VE_OK;

	if (!fAlreadySaving)
	{
		fAlreadySaving = true;
		VUUID xGroupID;
		bool forced;
		bool isnew = false;
		if (fMainRec == nil || fMainRec->IsNew())
		{
			fModel->GetPermission(DB4D_EM_Create_Perm, xGroupID, forced);
			isnew = true;
		}
		else
			fModel->GetPermission(DB4D_EM_Update_Perm, xGroupID, forced);

		if (okperm(context, xGroupID))
		{
			bool firstlevel = false;
			Transaction* localtrans = nil;
			if (trans == nil)
			{
				firstlevel = true;
				trans = &localtrans;
			}

			err = Validate(context);

			if (err == VE_OK)
			{
				err = CallDBEvent(dbev_save, context);
			}

			if (err == VE_OK)
			{
				for (EntityAttributeValueCollection::iterator cur = fValues.begin(), end = fValues.end(); cur != end && err == VE_OK; cur++)
				{
					EntityAttributeValue* val = *cur;
					if (val != nil)
					{
						err = val->Save(*trans, context);
					}
				}
			}

			if (err == VE_OK)
			{
				if (fModel->HasPrimKey() && !fModel->MatchPrimKeyWithDataSource())
				{
					const IdentifyingAttributeCollection* primkey = fModel->GetPrimaryAtts();
					VValueBag keyval;
					for (IdentifyingAttributeCollection::const_iterator cur = primkey->begin(), end = primkey->end(); cur != end && err == VE_OK; cur++)
					{
						EntityAttributeValue* val = getAttributeValue(cur->fAtt, err, context);
						if (err == VE_OK)
						{
							if (val == nil || val->getVValue() == nil || val->GetVValue()->IsNull())
							{
								if (GetNum() >= 0)
									err = ThrowError(VE_DB4D_PRIMKEY_IS_NULL);
								else
									err = ThrowError(VE_DB4D_PRIMKEY_IS_NULL_2);
							}
							else
							{
								keyval.SetAttribute(cur->fAtt->GetName(), val->getVValue()->Clone());
							}
						}
					}
					if (err == VE_OK)
					{
						sLONG otherrecid = fModel->getEntityNumWithPrimKey(keyval, context, err);
						if (otherrecid != -1 && otherrecid != GetNum())
						{
							VString skey;
							bool first = true;
							for (IdentifyingAttributeCollection::const_iterator cur = primkey->begin(), end = primkey->end(); cur != end && err == VE_OK; cur++)
							{
								EntityAttributeValue* val = getAttributeValue(cur->fAtt, err, context);
								VString s;
								val->getVValue()->GetString(s);
								if (first)
									first = false;
								else
									skey += L";";
								skey += s;
							}
							err = ThrowError(VE_DB4D_PRIMKEY_IS_NOT_UNIQUE, &skey);
						}
					}
				}
				if (err == VE_OK)
					err = fMainRec->GetDF()->SaveRecord(fMainRec, context, stamp, allowOverrideStamp);
				if (err == VE_OK)
				{
					fStamp = 0;
					for (EntityAttributeValueCollection::iterator cur = fValues.begin(), end = fValues.end(); cur != end; cur++)
					{
						EntityAttributeValue* val = *cur;
						if (val != nil)
							val->UnTouch();
					}
				}
			}

			if (*trans != nil && firstlevel)
			{
				if (err == VE_OK)
					err = context->CommitTransaction();
				else
					context->RollBackTransaction();
			}
		}
		else
		{
			context->SetPermError();
			if (isnew)
				err = ThrowError(VE_DB4D_NO_PERM_TO_CREATE);
			else
				err = ThrowError(VE_DB4D_NO_PERM_TO_UPDATE);
		}
		fAlreadySaving = false;
	}

	if (err != VE_OK)
	{
		if (GetNum() >= 0)
			err = ThrowError(VE_DB4D_ENTITY_RECORD_CANNOT_BE_SAVED);
		else
			err = ThrowError(VE_DB4D_NEW_ENTITY_RECORD_CANNOT_BE_SAVED);
	}
	return err;
}


Boolean EntityRecord::IsNew() const
{
	return fMainRec->IsNew();
}


Boolean EntityRecord::IsProtected() const
{
	return fMainRec->ReadOnlyState();
}


Boolean EntityRecord::IsModified() const
{
	return fStamp != 0;
}


void EntityRecord::GetTimeStamp(VTime& outValue)
{
	uLONG8 quand = fMainRec->GetAssoc()->GetTimeStamp();
	outValue.FromStamp(quand);
}


VErrorDB4D EntityRecord::Drop()
{
	BaseTaskInfo* context = ConvertContext(fContext);
	VError err = VE_OK;
	if (!fAlreadyDeleting)
	{
		fAlreadyDeleting = true;
		if (okperm(context, fModel, DB4D_EM_Delete_Perm))
		{
			err = CallDBEvent(dbev_remove, context);
			if (err == VE_OK)
			{
				if (fModel->HasDeleteEvent(true))
					err = fModel->CallAttributesDBEvent(dbev_remove, this, context);
				if (err == VE_OK)
					err = fMainRec->GetDF()->DelRecord(fMainRec, context, fModificationStamp);
			}
		}
		else
		{
			err =  ThrowError(VE_DB4D_NO_PERM_TO_DELETE);
			context->SetPermError();
		}
		fAlreadyDeleting = false;
	}
	return err;
}


void EntityRecord::GetPrimKeyValue(VValueBag& outBagKey)
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


void EntityRecord::GetPrimKeyValue(VJSObject& outObj)
{
	VError err = VE_OK;
	outObj.MakeEmpty();
	const IdentifyingAttributeCollection* primatts = fModel->GetPrimaryAtts();
	if (primatts->empty())
	{
		VJSValue jsval(outObj.GetContextRef());
		RecIDType numrec = GetNum();
		jsval.SetNumber(numrec);
		outObj.SetProperty("__RECID", jsval, JS4D::PropertyAttributeNone);
	}
	else
	{
		for (IdentifyingAttributeCollection::const_iterator cur = primatts->begin(), end = primatts->end(); cur != end; cur++)
		{
			EntityAttributeValue* val = getAttributeValue(cur->fAtt, err, ConvertContext(fContext));
			if (val != nil && val->getVValue() != nil)
			{
				VJSValue jsval(outObj.GetContextRef());
				jsval.SetVValue(*(val->getVValue()));
				outObj.SetProperty(cur->fAtt->GetName(), jsval, JS4D::PropertyAttributeNone);
			}
		}
	}
	VJSValue jsval(outObj.GetContextRef());
	sLONG stamp = GetModificationStamp();
	jsval.SetNumber(stamp);
	outObj.SetProperty("__STAMP", jsval, JS4D::PropertyAttributeNone);
}


void EntityRecord::GetPrimKeyValue(VString& outKey, bool autoQuotes)
{
	StErrorContextInstaller errs(false);
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

}


void EntityRecord::GetPrimKeyValue(VectorOfVValue& outPrimkey)
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
	VJSValue jsval(objkey.GetContextRef());
	const IdentifyingAttributeCollection* primatts = fModel->GetPrimaryAtts();
	
	if (!primatts->empty())
	{
		SearchTab query(fModel->GetMainTable());
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
	return ConvertContext(fContext);
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


VValueSingle* EntityRecord::getFieldValue(Field* cri, VError& err)
{
	err = VE_OK;
	VValueSingle* result = nil;
	assert(cri != nil);
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
	assert(cri != nil);
	assert(inValue != nil);
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

	JS4D::ContextRef jscontext = outObj.GetContextRef();
	BaseTaskInfo* context = ConvertContext(fContext);

	if (err == VE_OK)
	{
		if (GetNum() >= 0 && withKey)
		{
			VJSObject keyobj(jscontext);
			GetPrimKeyValue(keyobj);
			outObj.SetProperty("__KEY", keyobj, JS4D::PropertyAttributeNone);
		}

		sLONG nb = (sLONG)whichAttributes.size(), i = 1;
		for (EntityAttributeSortedSelection::iterator cur = whichAttributes.begin(), end = whichAttributes.end(); cur != end && err == VE_OK; cur++, i++)
		{
			EntityAttribute* attribute = cur->fAttribute;
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

							VJSValue valueJS(jscontext);
							VValueSingle* cv = val->GetVValue();
							if (cv != nil)
							{
								if (cv->GetValueKind() == VK_IMAGE)
								{
#if !VERSION_LINUX   // Postponed Linux Implementation !
									VJSObject subObj(outObj, attribute->GetName());
									valueJS = subObj;


									/*
									void* extradata = val->GetExtraData();
									if (extradata == nil)
									{
										db4dJSPictContainer* xpic = new db4dJSPictContainer(cv, jscontext, this, (EntityAttribute*)attribute);
										valueJS = VJSImage::CreateInstance(jscontext, xpic);
										val->SetExtraData(xpic);
										xpic->Release();
									}
									else
									{
										valueJS = VJSImage::CreateInstance(jscontext, (db4dJSPictContainer*)extradata);
									}
									*/
#else
									valueJS.SetUndefined();
#endif
								}
								else
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

									err = VImpCreator<EntityRecord>::GetImpObject(subrec)->ConvertToJSObject(subObj, *cur->fSousSelection, eai->fSousSelection, subSortingAttributes, false, true);
								}
								else
								{
									EntityAttributeSortedSelection subAtts(submodel);
									err = VImpCreator<EntityRecord>::GetImpObject(subrec)->ConvertToJSObject(subObj, subAtts, nil, nil, true, true);
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
							Selection* sel = val->getRelatedSelection();
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
											countelem =  sel->GetQTfic();
										else
											countelem = submodel->GetDefaultTopSizeInUse();
									}
									VJSArray subarr(outObj.GetContextRef());
									outObj.SetProperty(attribute->GetName(), subarr);
									err = SelToJSObject(context, subarr, submodel, sel, *cur->fSousSelection, eai->fSousSelection, subSortingAttributes, false, true, eai->fSkip, countelem);
								}
								else
								{
									if (kind == eav_composition)
									{
										VJSArray subarr(outObj.GetContextRef());
										EntityAttributeSortedSelection subatts(submodel);
										err = SelToJSObject(context, subarr, submodel, sel, subatts, nil, nil, false, true, 0, sel->GetQTfic());
									}
									else
									{
										VJSObject subObj( outObj, attribute->GetName());
										subObj.SetProperty("__COUNT", sel->GetQTfic());
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
				VJSValue jsval(recobj.GetContextRef());
				jsval = recobj.GetProperty(*cur);
				switch (eVal->GetKind())
				{
					case eav_vvalue:
						{
							VValueSingle* cv = jsval.CreateVValue();
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
								VJSObject objcompo(recobj.GetContextRef());
								jsval.GetObject(objcompo);
								if (objcompo.IsInstanceOf("Array"))
									eVal->SetJSObject(objcompo);
								else
								{
									VJSArray emptyarray(recobj.GetContextRef());
									eVal->SetJSObject(emptyarray);
								}
							}
							else
							{
								VJSArray emptyarray(recobj.GetContextRef());
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
	VError err = CallDBEvent(dbev_init, context);
	if (err == VE_OK)
	{
		const EntityAttributeCollection& attsWithInit = fModel->GetAttributesWithInitEvent();
		for (EntityAttributeCollection::const_iterator cur = attsWithInit.begin(), end = attsWithInit.end(); cur != end; cur++)
		{
			const EntityAttribute* att = *cur;
			att->CallDBEvent(dbev_init, this, context);
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
	assert(fAlreadyCallEvent[kind] == 1);
	fAlreadyCallEvent[kind] = 0;
}







// ------------------------------------------------------------------------------------------



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
			err = fOwner->ThrowError(VE_DB4D_ENTITY_NAME_ALREADY_EXIST, noaction, &name);
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


EntityModel* EntityModelCatalog::BuildEntityModel(const VValueBag* bag, VError& err, bool devMode)
{
	err = VE_OK; 
	EntityModel* model = new EntityModel(fOwner);
	err = model->FromBag(bag, this, devMode);
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
			err = fOwner->ThrowError(VE_DB4D_TYPE_ALREADY_EXIST, noaction);
	}
	else
	{
		newType->Retain();
		fTypes[newType->GetName()] = newType;
	}
	return err;
}


AttributeType* EntityModelCatalog::BuildType(const VValueBag* bag, VError& err, bool devMode)
{
	err = VE_OK; 
	AttributeType* typ = new AttributeType(fOwner);
	err = typ->FromBag(bag, this, true, devMode);
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
			err = fOwner->ThrowError(VE_DB4D_ENUMERATION_ALREADY_EXIST, noaction);
	}
	else
	{
		newEnum->Retain();
		fEnumerations[newEnum->GetName()] = newEnum;
	}
	return err;
}


EmEnum* EntityModelCatalog::BuildEnumeration(const VValueBag* bag, VError& err, bool devMode)
{
	err = VE_OK; 
	EmEnum* en = new EmEnum(fOwner);
	err = en->FromBag(bag, this, devMode);
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


VError EntityModelCatalog::LoadEntityModels(const VValueBag& bagEntities, bool devMode, unordered_map_VString<VRefPtr<VFile> >* outIncludedFiles)
{
	VError err = VE_OK;
	ClearErrors();

#if withGlobalContext
	IDB4D_ApplicationIntf *applicationIntf = VDBMgr::GetManager()->GetApplicationInterface();
	VJSGlobalContext *globalJSContext = applicationIntf->RetainJSContext( fApplicationRef, err, false, nil);
	if (globalJSContext != NULL &&  err == VE_OK)
#endif
	{
#if withGlobalContext
		VJSContext jscontext( globalJSContext);
		VJSObject globalObject( jscontext.GetGlobalObject());
#endif

		CDB4DBaseContext* xcontext;
		BaseTaskInfo* context;
		DB4DJSRuntimeDelegate* xJSDelegate;
		VJSGlobalContext* xJSContext;

		context = new BaseTaskInfo(fOwner, nil, nil, nil);
		xJSDelegate = new DB4DJSRuntimeDelegate(context);
		xJSContext = VJSGlobalContext::Create( xJSDelegate);

		context->SetJSContext(xJSContext);

		VJSContext jscontext( xJSContext);
		VJSObject globalObject( jscontext.GetGlobalObject());

		fLoadingContext = &jscontext;
		fLoadingGlobalObject = &globalObject;
		//		VJSGlobalClass::CreateGlobalClasses();

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

		//VFile* fileJs = context->GetBase()->RetainCatalogJSFile();
		VFile* fileJs = fOwner->RetainCatalogJSFile();
		if (fileJs != nil)
		{
			if (fileJs->Exists())
			{
				StErrorContextInstaller errs(false);
				VJSValue result(jscontext);
				JS4D::ExceptionRef e = nil;
				jscontext.EvaluateScript(fileJs, &result, &e, nil);
				if (e != nil)
				{
					if (devMode)
					{
						VJSValue ev(jscontext, e);
	#if debuglr
						VJSJSON json(jscontext);
						VString s;
						json.Stringify(ev, s);
						sLONG xdebug = 1; // put a break here
	#endif
						fParseUserDefined = true;
						fSomeErrorsInCatalog = true;
						VJSObject eo(ev.GetObject());
						fParseLineError = eo.GetPropertyAsLong("line", nil, nil);
						eo.GetPropertyAsString("message", nil, fParseMessageError);
						VString path;
						eo.GetPropertyAsString("sourceURL", nil, path);
						VString spath;
						path.GetSubString(1,8, spath);
						if (spath.EqualToString_Like("file:///"))
							path.Remove(1, 8);
						fParseFileError = new VFile(path, FPS_POSIX);
					}
					else
					{
						err = ThrowBaseError(VE_DB4D_JS_ERR);
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
			fileJs->Release();
		}

		if (err == VE_OK)
		{
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
						EmEnum* newEnum = BuildEnumeration(OneEnum, err, devMode);
						if (newEnum != nil)
							err = AddOneEnumeration(newEnum, devMode);
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
						AttributeType* newType = BuildType(OneType, err, devMode);
						if (newType != nil)
							err = AddOneType(newType, devMode);
						QuickReleaseRefCountable(newType);
					}
				}
			}
		}
	/*
		if (err == VE_OK)
		{
			const VBagArray* relations = bagEntities.GetElements(d4::relationship);
			if (relations != nil)
			{
				VIndex nbrelations = relations->GetCount();
				for (VIndex i = 1; i <= nbrelations && err == VE_OK; i++)
				{
					const VValueBag* OneRelation = relations->GetNth(i);
					if (OneRelation != nil)
					{
						EntityRelation* newRel = BuildEntityRelation(OneRelation, err);
						if (newRel != nil)
							err = AddOneEntityRelation(newRel, false);
						QuickReleaseRefCountable(newRel);
					}
				}
			}
		}
	*/
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
						EntityModel* newEntity = BuildEntityModel(OneEntity, err, devMode);
						if (newEntity != nil && newEntity->GetMainTable() != nil)
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
				err =cur->second->ResolveRelatedEntities(already, this, devMode, context);
			}

			/*
			for (EntityModelMap::iterator cur = fEntityModels.begin(), end = fEntityModels.end(); cur != end && err == VE_OK; cur++)
			{
				err =cur->second->ResolveRelatedPath(this);
			}
			*/

			for (EntityModelMap::iterator cur = fEntityModels.begin(), end = fEntityModels.end(); cur != end && err == VE_OK; cur++)
			{
				err =cur->second->ResolveQueryStatements(this, devMode, context);
			}
		}

		fLoadingContext = nil;
		fLoadingGlobalObject = nil;

		QuickReleaseRefCountable(xJSContext);
		delete xJSDelegate;
	}

	// sc 28/05/2012, ensure the VJSContext object is destroyed before release the context
#if withGlobalContext
	applicationIntf->ReleaseJSContext( fApplicationRef, globalJSContext, nil);
#endif

	return err;
}


VError EntityModelCatalog::LoadEntityModels(const VFile& inFile, bool inXML, bool devMode, const VString* inXmlContent, unordered_map_VString<VRefPtr<VFile> >* outIncludedFiles)
{
	VError err = VE_OK;
	if (inFile.Exists())
	{
		/*
		VFolder* parent = inFile.RetainParentFolder();
		VString name;
		inFile.GetNameWithoutExtension(name);
		VFile dirFile(*parent, name+".waUAG");
		parent->Release();
		
		if (dirFile.Exists())
		{
			VError err2 = VE_OK;

			CUAGDirectory* directory = VDBMgr::GetUAGManager()->RetainDirectory(dirFile, FA_READ, nil, nil, &err);

			directory->Release();
		}
		*/

		bool okbag = false;
		VValueBag bagEntities;
		if (inXML)
		{
			if ( inXmlContent )
				err = LoadBagFromXML( *inXmlContent, L"EntityModelCatalog", bagEntities);
			else
				err = LoadBagFromXML(inFile, L"EntityModelCatalog", bagEntities);

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
						//SaveBagToXML(bagEntities, L"EntityModelCatalog", VFile(*fStructFolder, L"EntityModels.xml"));
						okbag = true;
					}

				}
			}
			input.CloseReading();
		}

		if (okbag)
		{
			const VValueBag* dbBag = bagEntities.RetainUniqueElement(d4::dbInfo);
			if (dbBag != nil)
			{
				err = fOwner->FromBag(*dbBag, this);
				dbBag->Release();
			}
			else
				TouchXML();
			if (err == VE_OK)
				err = LoadEntityModels(bagEntities, devMode, outIncludedFiles);
			/*
			if (err == VE_OK)
			{
				err = fOwner->LoadIndicesFromBag(&bagEntities);
			}
			*/
		}
	}

	if (err != VE_OK)
		err = fOwner->ThrowError(VE_DB4D_CANNOT_LOAD_ENTITY_CATALOG, noaction);

	return err;
}


VError EntityModelCatalog::SaveEntityModels(VValueBag& catalogBag, bool inXML, bool withDBInfo) const
{
	VError err = VE_OK;

	if (fPublishDataClassesAsGlobalsDefined)
	{
		catalogBag.SetBool(d4::publishAsJSGlobal, fPublishDataClassesAsGlobals);
	}

	if (withDBInfo)
	{
		VString unused;
		BagElement dbBag(catalogBag, d4::dbInfo);
		err = fOwner->SaveToBag(*dbBag, unused);
	}

	if (fExtraProperties != nil)
	{
		catalogBag.AddElement(d4::extraProperties, (VValueBag*)fExtraProperties);
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
		if (inXML)
		{
			err = SaveBagToXML(catalogBag, L"EntityModelCatalog", inFile, true);
		}
		else
		{
			err = SaveBagToJson(catalogBag, inFile, true);
		}
	}

	return err;
}


VError EntityModelCatalog::LoadEntityPermissions(const VFile& inFile, CUAGDirectory* inDirectory, bool inXML, bool devMode)
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
			err = LoadEntityPermissions(bagEntities, inDirectory, devMode);
	}

	if (err != VE_OK)
		err = fOwner->ThrowError(VE_DB4D_CANNOT_LOAD_ENTITY_CATALOG, noaction);

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

VError EntityModelCatalog::LoadEntityPermissions(const VValueBag& bagEntities, CUAGDirectory* inDirectory, bool devMode)
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
						}
						QuickReleaseRefCountable(group);
					}
				}
			}

			if (!devMode)
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
		err = fOwner->ThrowError(VE_DB4D_CANNOT_LOAD_ENTITIES_PERMS, noaction);
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
				em = EntityModel::BuildEntityModel(tt);
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


VError EntityModelCatalog::GetAllEntityModels(vector<CDB4DEntityModel*>& outList, CDB4DBaseContext* context) const
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
		err = fOwner->ThrowError(memfull, noaction);
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
					if ((*cur1)->GetSourceField() != (*cur2)->GetSourceField())
					{
						egal = false;
						break;
					}
					if ((*cur1)->GetDestField() != (*cur2)->GetDestField())
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
			EntityRelation* reverseRel = new EntityRelation(rel->GetDestField(), rel->GetSourceField(), erel_1toN);
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









