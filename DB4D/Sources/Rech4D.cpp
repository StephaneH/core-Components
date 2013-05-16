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

const sLONG maxvaluetodisplay = 8;

const VSize minStackSize = 25000;

#include <map>
#include <algorithm>
using namespace std;


void CheckForNullOn(sLONG& comp, bool& checkfornull)
{
	if (comp == DB4D_IsNull)
	{
		comp = DB4D_Equal;
		checkfornull = true;
	}
	if (comp == DB4D_IsNotNull)
	{
		comp = DB4D_NotEqual;
		checkfornull = true;
	}
}



VDB4DQueryPathModifiers::VDB4DQueryPathModifiers()
{
	fLineDelimiter = L"\n";
	fTabDelimiter = L" ";
	fTabSize = 4;
	fOutHTML = false;
	fVerbose = false;
	fSimpleTree = true;
	fTempVarDisplay = false;
}


void VDB4DQueryPathModifiers::SetLineDelimiter(const VString& inDelimiter)
{
	fLineDelimiter = inDelimiter;
}


void VDB4DQueryPathModifiers::SetTabDelimiter(const VString& inDelimiter)
{
	fTabDelimiter = inDelimiter;
}


void VDB4DQueryPathModifiers::SetTabSize(sLONG inTabSize)
{
	fTabSize = inTabSize;
}


void VDB4DQueryPathModifiers::SetHTMLOutPut(Boolean inState)
{
	fOutHTML = inState;
}


void VDB4DQueryPathModifiers::SetVerbose(Boolean inState)
{
	fVerbose = inState;
}


void VDB4DQueryPathModifiers::SetSimpleTree(Boolean inState, Boolean inWithTempVariables)
{
	fSimpleTree = inState;
	fTempVarDisplay = inWithTempVariables;
}



				/* ---------------------------------------- */



VError RechToken::PutInto(VStream& buf)
{
	VError err;
	
	err = buf.PutWord(TypToken);
	return(err);
}

VError RechToken::GetFrom(VStream& buf, Table* target)
{
	return(VE_OK);
}


RechToken* RechToken::Clone() const
{
	RechToken* result = new RechToken();
	result->TypToken = TypToken;

	return result;
}


					/* ---------------------------------------- */


VError RechTokenBoolOper::PutInto(VStream& buf)
{
	VError err;
	
	err = RechToken::PutInto(buf);
	if (err == VE_OK)
	{
		err = buf.PutWord(BoolLogic);
	}
	return(err);
}

VError RechTokenBoolOper::GetFrom(VStream& buf, Table* target)
{
	VError err;
	
	err = RechToken::GetFrom(buf, target);
	if (err == VE_OK)
	{
		err = buf.GetWord(BoolLogic);
	}
	
	return(err);
}


RechToken* RechTokenBoolOper::Clone() const
{
	RechTokenBoolOper* result = new RechTokenBoolOper();
	result->BoolLogic = BoolLogic;

	return result;
}


VError RechTokenBoolOper::BuildString(VString& outStr)
{
	switch (BoolLogic)
	{
		case DB4D_And:
			outStr = " and ";
			break;
		case DB4D_OR:
			outStr = " or ";
			break;
		case DB4D_Except:
			outStr = " except ";
			break;
	}
	return VE_OK;
}



					/* ---------------------------------------- */




VError RechTokenInterpOper::PutInto(VStream& buf)
{
	VError err;

	err = RechToken::PutInto(buf);
	if (err == VE_OK)
	{
		err = buf.PutWord(oper);
	}
	return(err);
}

VError RechTokenInterpOper::GetFrom(VStream& buf, Table* target)
{
	VError err;

	err = RechToken::GetFrom(buf, target);
	if (err == VE_OK)
	{
		err = buf.GetWord(oper);
	}

	return(err);
}


RechToken* RechTokenInterpOper::Clone() const
{
	RechTokenInterpOper* result = new RechTokenInterpOper();
	result->oper = oper;

	return result;
}


void BuildCompOperString(sLONG oper, VString& outStr)
{
	switch (oper)
	{
		case DB4D_Equal:
			outStr = " === ";
			break;
		case DB4D_NotEqual:
			outStr = " !== ";
			break;
		case DB4D_Greater:
			outStr = " > ";
			break;
		case DB4D_GreaterOrEqual:
			outStr = " >= ";
			break;
		case DB4D_Lower:
			outStr = " < ";
			break;
		case DB4D_LowerOrEqual:
			outStr = " <= ";
			break;
		case DB4D_Contains_KeyWord:
			outStr = " %% ";
			break;
		case DB4D_DoesntContain_KeyWord:
			outStr = " except ";
			break;
		case DB4D_BeginsWith:
			outStr = " begin ";
			break;
		case DB4D_Contains_KeyWord_BeginingWith:
			outStr = " %% ";
			break;
		case DB4D_Regex_Match:
			outStr = " matches ";
			break;
		case DB4D_IN:
			outStr = " in ";
			break;
		case DB4D_Like:
			outStr = " == ";
			break;
		case DB4D_NotLike:
			outStr = " != ";
			break;
		case DB4D_Regex_Not_Match:
			outStr = " !%* ";
			break;
	}
}


					/* ---------------------------------------- */



VError RechTokenSimpleComp::PutInto(VStream& buf)
{
	VError err;
	
	err = RechToken::PutInto(buf);
	if (err == VE_OK) 
		err = buf.PutLong(numfile);
	if (err == VE_OK) 
		err = buf.PutLong(numfield);
	if (err == VE_OK) 
		err = buf.PutLong(numinstance);
	if (err == VE_OK) 
		err = buf.PutWord(comparaison);
	if (err == VE_OK) 
		err = buf.PutValue(*ch, true);
	if (err == VE_OK)
		err = PutVCompareOptionsIntoStream(fOptions, buf);
	
	return(err);
}

VError RechTokenSimpleComp::GetFrom(VStream& buf, Table* target)
{
	VError err;
	
	err = RechToken::GetFrom(buf, target);
	if (err == VE_OK) 
		err = buf.GetLong(numfile);
	if (err == VE_OK) 
		err = buf.GetLong(numfield);
	if (err == VE_OK) 
		err = buf.GetLong(numinstance);
	if (err == VE_OK) 
		err = buf.GetWord(comparaison);
	if (err == VE_OK) 
	{
		ch = (ValPtr)buf.GetValue();
		if (ch == nil)
			err = ThrowBaseError(memfull, DBaction_LoadingQuery);
	}
	if (err == VE_OK)
		CheckIfDataIsDirect(target);

	if (err == VE_OK)
		err = GetVCompareOptionsFromStream(fOptions, buf);

	return(err);
}


void RechTokenSimpleComp::CheckIfDataIsDirect(Table* target)
{
	if (numfile == 0)
		fIsDataDirect = target->IsDataDirect(numfield);
	else
	{
		Table* t = target->GetOwner()->RetainTable(numfile);
		if (t != nil)
		{
			fIsDataDirect = t->IsDataDirect(numfield);
			t->Release();
		}
	}
}


void RechTokenSimpleComp::Dispose(void)
{
	if (ch != nil) 
		delete ch;
	fIsNeverNull = 2;
	QuickReleaseRefCountable(fRegMatcher);
}


RechToken* RechTokenSimpleComp::Clone() const
{
	RechTokenSimpleComp* result = new RechTokenSimpleComp();

	result->numfield = numfield;
	result->numfile = numfile;
	result->numinstance = numinstance;
	result->comparaison = comparaison;
	if (ch == nil)
		result->ch = nil;
	else
		result->ch = ch->Clone();
	result->fIsDataDirect = fIsDataDirect;
	result->fOptions = fOptions;
	result->fIsNeverNull = fIsNeverNull;
	result->fParam = fParam;
	result->fExpectedType = fExpectedType;
	result->fCheckForNull = fCheckForNull;
	result->fRegMatcher = RetainRefCountable(fRegMatcher);

	return result;
}



					/* ---------------------------------------- */




VError RechTokenEMSimpleComp::PutInto(VStream& buf)
{
	VError err = VE_DB4D_NOTIMPLEMENTED;
	return(err);
}

VError RechTokenEMSimpleComp::GetFrom(VStream& buf, Table* target)
{
	VError err = VE_DB4D_NOTIMPLEMENTED;
	return(err);
}


void RechTokenEMSimpleComp::CheckIfDataIsDirect(Table* target)
{
}


void RechTokenEMSimpleComp::Dispose(void)
{
	if (ch != nil) 
		delete ch;
	fIsNeverNull = 2;
	QuickReleaseRefCountable(fRegMatcher);
	if (fCopyForLocalModel != nil)
	{
		fCopyForLocalModel->Dispose();
		delete fCopyForLocalModel;
	}
}

RechTokenSimpleComp* RechTokenEMSimpleComp::CopyIntoRechTokenSimpleComp()
{
	if (fCopyForLocalModel == nil)
	{
		fCopyForLocalModel = new RechTokenSimpleComp();
		LocalEntityModel* locmodel = dynamic_cast<LocalEntityModel*>(fAtt->GetModel());
		if (locmodel != nil)
		{
			fCopyForLocalModel->numfile = locmodel->GetMainTable()->GetNum();
			fCopyForLocalModel->numfield = fAtt->GetFieldPos();
		}
		else
		{
			fCopyForLocalModel->numfile = 0;
			fCopyForLocalModel->numfield = 0;
		}
		fCopyForLocalModel->numinstance = numinstance;
		fCopyForLocalModel->comparaison = comparaison;
		if (ch == nil)
			fCopyForLocalModel->ch = nil;
		else
			fCopyForLocalModel->ch = ch->Clone();
		fCopyForLocalModel->fIsDataDirect = fIsDataDirect;
		fCopyForLocalModel->fOptions = fOptions;
		fCopyForLocalModel->fIsNeverNull = fIsNeverNull;
		fCopyForLocalModel->fParam = fParam;
		fCopyForLocalModel->fExpectedType = fExpectedType;
		fCopyForLocalModel->fCheckForNull = fCheckForNull;
		fCopyForLocalModel->fRegMatcher = RetainRefCountable(fRegMatcher);
	}

	return fCopyForLocalModel;
}


RechToken* RechTokenEMSimpleComp::Clone() const
{
	RechTokenEMSimpleComp* result = new RechTokenEMSimpleComp();

	result->fAtt = fAtt;
	result->numinstance = numinstance;
	result->comparaison = comparaison;
	if (ch == nil)
		result->ch = nil;
	else
		result->ch = ch->Clone();
	result->fIsDataDirect = fIsDataDirect;
	result->fOptions = fOptions;
	result->fIsNeverNull = fIsNeverNull;
	result->fParam = fParam;
	result->fExpectedType = fExpectedType;
	result->fSimpleDate = fSimpleDate;
	result->fCheckForNull = fCheckForNull;
	result->fRegMatcher = RetainRefCountable(fRegMatcher);
	return result;
}

VError RechTokenEMSimpleComp::BuildString(VString& outStr)
{
	VString soper;
	VString attname;
	fAtt->GetName(attname);
	BuildCompOperString(comparaison, soper);
	VString lastpart;
	if (!fParam.IsEmpty())
	{
		lastpart = fParam;
	}
	else if (fRegMatcher != nil)
	{
	}
	else if (comparaison == DB4D_IsNull || comparaison == DB4D_IsNotNull)
	{
		lastpart = "null";
		if (comparaison == DB4D_IsNull)
			soper = " is ";
		else
			soper = " is not ";
	}
	else if (ch != nil)
	{
		ch->GetString(lastpart);
	}

	outStr = attname + soper + lastpart;

	return VE_OK;
}





					/* ---------------------------------------- */



VError RechTokenRecordExist::PutInto(VStream& buf)
{
	VError err;

	err = RechToken::PutInto(buf);
	if (err == VE_OK) 
		err = buf.PutLong(numfile);
	if (err == VE_OK) 
		err = buf.PutLong(numinstance);
	if (err == VE_OK) 
		err = buf.PutByte(fCheckIfExists);

	return(err);
}

VError RechTokenRecordExist::GetFrom(VStream& buf, Table* target)
{
	VError err;

	err = RechToken::GetFrom(buf, target);
	if (err == VE_OK) 
		err = buf.GetLong(numfile);
	if (err == VE_OK) 
		err = buf.GetLong(numinstance);
	if (err == VE_OK) 
		err = buf.GetByte(fCheckIfExists);

	return(err);
}


RechToken* RechTokenRecordExist::Clone() const
{
	RechTokenRecordExist* result = new RechTokenRecordExist();

	result->numfile = numfile;
	result->numinstance = numinstance;
	result->fCheckIfExists = fCheckIfExists;

	return result;
}



					/* ---------------------------------------- */



VError RechTokenEntityRecordExist::PutInto(VStream& buf)
{
	VError err = VE_DB4D_NOTIMPLEMENTED;

	return(err);
}

VError RechTokenEntityRecordExist::GetFrom(VStream& buf, Table* target)
{
	VError err = VE_DB4D_NOTIMPLEMENTED;

	return(err);
}


RechToken* RechTokenEntityRecordExist::Clone() const
{
	RechTokenEntityRecordExist* result = new RechTokenEntityRecordExist();

	result->fModel = fModel;
	result->numinstance = numinstance;
	result->fCheckIfExists = fCheckIfExists;

	return result;
}

void RechTokenEntityRecordExist::Dispose(void)
{
	if (fCopyForLocalModel != nil)
	{
		fCopyForLocalModel->Dispose();
		delete fCopyForLocalModel;
	}
}

RechTokenRecordExist* RechTokenEntityRecordExist::CopyIntoRechTokenRecordExist()
{
	if (fCopyForLocalModel == nil)
	{
		fCopyForLocalModel = new RechTokenRecordExist();

		fCopyForLocalModel->numfile = 0;
		LocalEntityModel* locmodel = dynamic_cast<LocalEntityModel*>(fModel);
		if (locmodel != nil)
			fCopyForLocalModel->numfile = locmodel->GetMainTable()->GetNum();
		fCopyForLocalModel->numinstance = numinstance;
		fCopyForLocalModel->fCheckIfExists = fCheckIfExists;
	}

	return fCopyForLocalModel;
}


					/* ---------------------------------------- */



void RechTokenEmComp::Dispose(void)
{
	if (ch != nil) 
		delete ch;
	QuickReleaseRefCountable(fRegMatcher);
}

VError RechTokenEmComp::PutInto(VStream& buf)
{
	return ThrowBaseError(VE_DB4D_NOTIMPLEMENTED);
}

VError RechTokenEmComp::GetFrom(VStream& buf, Table* target)
{
	return ThrowBaseError(VE_DB4D_NOTIMPLEMENTED);
}


RechToken* RechTokenEmComp::Clone() const
{
	RechTokenEmComp* result = new RechTokenEmComp();

	result->fAttPath = fAttPath;
	result->comparaison = comparaison;
	if (ch == nil)
		result->ch = nil;
	else
		result->ch = ch->Clone();
	result->fOptions = fOptions;
	result->fParam = fParam;
	result->fRegMatcher = RetainRefCountable(fRegMatcher);
	result->fExpectedType = fExpectedType;
	result->fSimpleDate = fSimpleDate;
	result->numinstance = numinstance;


	return result;
}


VError RechTokenEmComp::BuildString(VString& outStr)
{
	VString soper;
	VString attname;
	fAttPath.GetString(attname);
	BuildCompOperString(comparaison, soper);
	VString lastpart;
	if (!fParam.IsEmpty())
	{
		lastpart = fParam;
	}
	else if (fRegMatcher != nil)
	{
	}
	else if (comparaison == DB4D_IsNull || comparaison == DB4D_IsNotNull)
	{
		lastpart = "null";
		if (comparaison == DB4D_IsNull)
			soper = " is ";
		else
			soper = " is not ";
	}
	else if (ch != nil)
	{
		ch->GetString(lastpart);
	}

	outStr = attname + soper + lastpart;

	return VE_OK;
}



void RechTokenEmComp::addInstance(const EntityModel* model, sLONG howmany)
{
	const EntityAttributeInstance* attinst = fAttPath.FirstPart();
	if (model == attinst->fAtt->GetModel()->GetRootBaseEm())
		numinstance += howmany;
}

					/* ---------------------------------------- */


VError RechTokenScriptComp::PutInto(VStream& buf)
{
	VError err;
	
	err = RechToken::PutInto(buf);
	if (err == VE_OK)
		err = buf.PutLong(numtable);
	
	if (expression == nil)
		err = ThrowBaseError(VE_DB4D_THIS_IS_NULL, noaction);
	else
	{
		if (err == VE_OK)
			err = expression->PutInto(buf);
	}
	
	return(err);
}

VError RechTokenScriptComp::GetFrom(VStream& buf, Table* target)
{
	VError err;
	
	err = RechToken::GetFrom(buf, target);
	if (err == VE_OK)
		err = buf.GetLong(numtable);
	if (err == VE_OK)
		expression = (*(VDBMgr::GetBuildLanguageExpressionMethod()))(buf, err);
	
	return(err);
}


void RechTokenScriptComp::Dispose(void)
{
	if (expression != nil)
		expression->Release();
}


RechToken* RechTokenScriptComp::Clone() const
{
	RechTokenScriptComp* result = new RechTokenScriptComp(expression, numtable);

	return result;
}



				/* --------------------------------------------------------- */


VError RechTokenArrayComp::PutInto(VStream& buf)
{
	VError err;

	err = RechToken::PutInto(buf);
	if (err == VE_OK) 
		err = buf.PutLong(numfile);
	if (err == VE_OK) 
		err = buf.PutLong(numfield);
	if (err == VE_OK) 
		err = buf.PutLong(numinstance);
	if (err == VE_OK) 
		err = buf.PutWord(comparaison);

	if (err == VE_OK)
		err = PutVCompareOptionsIntoStream(fOptions, buf);

	if (err == VE_OK)
	{
		if (values == nil)
		{
			err = buf.PutLong(0);
		}
		else
		{
			err = buf.PutLong(values->GetSignature());
			if (err == VE_OK) 
				err = values->PutInto(buf);
		}
	}


	return(err);
}


VError RechTokenArrayComp::GetFrom(VStream& buf, Table* target)
{
	VError err;
	sLONG sig;

	err = RechToken::GetFrom(buf, target);
	if (err == VE_OK) 
		err = buf.GetLong(numfile);
	if (err == VE_OK) 
		err = buf.GetLong(numfield);
	if (err == VE_OK) 
		err = buf.GetLong(numinstance);
	if (err == VE_OK) 
		err = buf.GetWord(comparaison);

	if (err == VE_OK)
		err = GetVCompareOptionsFromStream(fOptions, buf);

	if (values != nil)
		values->Release();
	values = nil;

	if (err == VE_OK)
		err = buf.GetLong(sig);

	if (err == VE_OK)
	{
		if (sig != 0)
		{
			if (sig == 'cons')
			{
				Field* cri = target->GetOwner()->RetainField(numfile, numfield);

				sLONG sourcesig;
				err = buf.GetLong(sourcesig);

				if(sourcesig == 'rawx')
				{
					values = GenerateConstArrayOfValues(buf, cri == nil ? 0 : cri->GetTyp(), fOptions);
				}
				else
				{
					DB4DArrayOfValuesCreator dbc = VDBMgr::GetManager()->GetDB4DArrayOfValuesCreator(sourcesig);
					if (dbc == nil)
						err = ThrowBaseError(VE_DB4D_ARRAYOFVALUES_CREATOR_IS_MISSING);
					else
					{
						DB4DArrayOfValues* source = (*dbc)(sourcesig, &buf, err);
						values = GenerateConstArrayOfValues(source, cri == nil ? 0 : cri->GetTyp(), fOptions);
						QuickReleaseRefCountable(source);
					}
				}
				QuickReleaseRefCountable(cri);
			}
			else
			{
				DB4DArrayOfValuesCreator dbc = VDBMgr::GetManager()->GetDB4DArrayOfValuesCreator(sig);
				if (dbc == nil)
					err = ThrowBaseError(VE_DB4D_ARRAYOFVALUES_CREATOR_IS_MISSING, DBaction_LoadingQuery);
				else
				{
					values = (*dbc)(sig, &buf, err);
				}
			}
		}
	}

	if (err == VE_OK)
		CheckIfDataIsDirect(target);


	return(err);
}


void RechTokenArrayComp::CheckIfDataIsDirect(Table* target)
{
	if (numfile == 0)
		fIsDataDirect = target->IsDataDirect(numfield);
	else
	{
		Table* t = target->GetOwner()->RetainTable(numfile);
		if (t != nil)
		{
			fIsDataDirect = t->IsDataDirect(numfield);
			t->Release();
		}
	}
}


void RechTokenArrayComp::Dispose(void)
{
	fIsNeverNull = 2;
	if (values != nil) 
		values->Release();
}


RechToken* RechTokenArrayComp::Clone() const
{
	RechTokenArrayComp* result = new RechTokenArrayComp();

	result->numfield = numfield;
	result->numfile = numfile;
	result->numinstance = numinstance;
	result->comparaison = comparaison;
	result->values = RetainRefCountable(values);
	result->fIsDataDirect = fIsDataDirect;
	result->fOptions = fOptions;
	result->fIsNeverNull = fIsNeverNull;
	result->fParam = fParam;
	result->fExpectedType = fExpectedType;

	return result;
}


			/* --------------------------------------------------------- */



VError RechTokenEmArrayComp::PutInto(VStream& buf)
{
	VError err = VE_DB4D_NOTIMPLEMENTED;
	return(err);
}


VError RechTokenEmArrayComp::GetFrom(VStream& buf, Table* target)
{
	VError err = VE_DB4D_NOTIMPLEMENTED;
	return(err);
}


void RechTokenEmArrayComp::CheckIfDataIsDirect(Table* target)
{
}


void RechTokenEmArrayComp::Dispose(void)
{
	fIsNeverNull = 2;
	if (values != nil) 
		values->Release();
	if (fCopyForLocalModel != nil)
	{
		fCopyForLocalModel->Dispose();
		delete fCopyForLocalModel;
	}
}



RechTokenArrayComp* RechTokenEmArrayComp::CopyIntoRechTokenArrayComp()
{
	if (fCopyForLocalModel == nil)
	{
		fCopyForLocalModel = new RechTokenArrayComp();
		LocalEntityModel* locmodel = dynamic_cast<LocalEntityModel*>(fAtt->GetModel());
		if (locmodel != nil)
		{
			fCopyForLocalModel->numfile = locmodel->GetMainTable()->GetNum();
			fCopyForLocalModel->numfield = fAtt->GetFieldPos();
		}
		else
		{
			fCopyForLocalModel->numfile = 0;
			fCopyForLocalModel->numfield = 0;
		}
		fCopyForLocalModel->numinstance = numinstance;
		fCopyForLocalModel->comparaison = comparaison;
		fCopyForLocalModel->values = RetainRefCountable(values);
		fCopyForLocalModel->fIsDataDirect = fIsDataDirect;
		fCopyForLocalModel->fOptions = fOptions;
		fCopyForLocalModel->fIsNeverNull = fIsNeverNull;
		fCopyForLocalModel->fParam = fParam;
		fCopyForLocalModel->fExpectedType = fExpectedType;
	}

	return fCopyForLocalModel;
}


RechToken* RechTokenEmArrayComp::Clone() const
{
	RechTokenEmArrayComp* result = new RechTokenEmArrayComp();

	result->fAtt = fAtt;
	result->numinstance = numinstance;
	result->comparaison = comparaison;
	result->values = RetainRefCountable(values);
	result->fIsDataDirect = fIsDataDirect;
	result->fOptions = fOptions;
	result->fIsNeverNull = fIsNeverNull;
	result->fParam = fParam;
	result->fExpectedType = fExpectedType;
	result->fSimpleDate = fSimpleDate;
	return result;
}


				/* --------------------------------------------------------- */


VError RechTokenJoin::PutInto(VStream& buf)
{
	VError err;

	err = RechToken::PutInto(buf);
	if (err == VE_OK) 
		err = buf.PutLong(numfile);
	if (err == VE_OK) 
		err = buf.PutLong(numfield);
	if (err == VE_OK) 
		err = buf.PutLong(numinstance);
	if (err == VE_OK) 
		err = buf.PutWord(comparaison);
	if (err == VE_OK) 
		err = buf.PutLong(numfileOther);
	if (err == VE_OK) 
		err = buf.PutLong(numfieldOther);
	if (err == VE_OK) 
		err = buf.PutLong(numinstanceOther);
	if (err == VE_OK)
		err = PutVCompareOptionsIntoStream(fOptions, buf);

	return(err);
}

VError RechTokenJoin::GetFrom(VStream& buf, Table* target)
{
	VError err;

	err = RechToken::GetFrom(buf, target);
	if (err == VE_OK) 
		err = buf.GetLong(numfile);
	if (err == VE_OK) 
		err = buf.GetLong(numfield);
	if (err == VE_OK) 
		err = buf.GetLong(numinstance);
	if (err == VE_OK) 
		err = buf.GetWord(comparaison);
	if (err == VE_OK) 
		err = buf.GetLong(numfileOther);
	if (err == VE_OK) 
		err = buf.GetLong(numfieldOther);
	if (err == VE_OK) 
		err = buf.GetLong(numinstanceOther);
	if (err == VE_OK)
		err = GetVCompareOptionsFromStream(fOptions, buf);

	return(err);
}


void RechTokenJoin::Dispose(void)
{
}


RechToken* RechTokenJoin::Clone() const
{
	RechTokenJoin* result = new RechTokenJoin();

	result->numfield = numfield;
	result->numfile = numfile;
	result->numinstance = numinstance;
	result->comparaison = comparaison;
	result->numfieldOther = numfieldOther;
	result->numfileOther = numfileOther;
	result->numinstanceOther = numinstanceOther;
	result->fOptions = fOptions;

	return result;
}



				/* --------------------------------------------------------- */


VError RechTokenEmJoin::PutInto(VStream& buf)
{
	VError err = VE_OK;

	return(err);
}

VError RechTokenEmJoin::GetFrom(VStream& buf, Table* target)
{
	VError err = VE_OK;

	return(err);
}


void RechTokenEmJoin::Dispose(void)
{
}


RechToken* RechTokenEmJoin::Clone() const
{
	RechTokenEmJoin* result = new RechTokenEmJoin();

	result->fRootBaseEm = fRootBaseEm;
	result->fAtt = fAtt;
	result->numinstance = numinstance;
	result->comparaison = comparaison;
	result->fRootBaseEmOther = fRootBaseEmOther;
	result->fAttOther = fAttOther;
	result->numinstanceOther = numinstanceOther;
	result->fOptions = fOptions;

	return result;
}



				/* --------------------------------------------------------- */



void RechTokenSel::Dispose(void)
{
	fSel->Release();
}



VError RechTokenSel::PutInto(VStream& buf)
{
	VError err;

	err = RechToken::PutInto(buf);
	err = VE_DB4D_NOTIMPLEMENTED;

	return(err);
}

VError RechTokenSel::GetFrom(VStream& buf, Table* target)
{
	VError err;

	err = RechToken::GetFrom(buf, target);
	err = VE_DB4D_NOTIMPLEMENTED;

	return(err);
}


RechToken* RechTokenSel::Clone() const
{
	RechTokenSel* result = new RechTokenSel(fSel, fNumInstance);

	return result;
}



				/* --------------------------------------------------------- */


void RechTokenEmSel::Dispose(void)
{
	fSel->Release();
}



VError RechTokenEmSel::PutInto(VStream& buf)
{
	VError err;

	err = RechToken::PutInto(buf);
	err = VE_DB4D_NOTIMPLEMENTED;

	return(err);
}

VError RechTokenEmSel::GetFrom(VStream& buf, Table* target)
{
	VError err;

	err = RechToken::GetFrom(buf, target);
	err = VE_DB4D_NOTIMPLEMENTED;

	return(err);
}


RechToken* RechTokenEmSel::Clone() const
{
	RechTokenEmSel* result = new RechTokenEmSel(fSel, fNumInstance);

	return result;
}



				/* --------------------------------------------------------- */


SearchTab::~SearchTab()
{
	if (fCanDelete)
		Dispose();
	/*
	for (QueryValueMap::iterator cur = fParamValues.begin(), end = fParamValues.end(); cur != end; cur++)
	{
		cur->second.Dispose();
	}
	*/
	if (destFile != nil)
		destFile->Release();
}


void SearchTab::Dispose(void)
{
	RechToken *rt;
	sLONG i;
	
	for (i = (sLONG)fLines.size(); i>0; i--)
	{
		rt = fLines[i-1];
		if (rt != nil)
		{
			rt->Dispose();
			delete rt;
		}
	}
	fLines.clear();
}


void SearchTab::AddSearchLineExpression(DB4DLanguageExpression* inExpression, sLONG inTableID)
{
	RechTokenScriptComp *rt;
	
	rt = new RechTokenScriptComp(inExpression, inTableID);
	fLines.push_back(rt);
}


void SearchTab::AddSearchLineEm(AttributePath& attpath, sLONG comp, const VString& inParamToCompare, const VCompareOptions& inOptions, sLONG numinstance)
{
	RechTokenEmComp* rt = new RechTokenEmComp;
	rt->fOptions = inOptions;
	rt->fAttPath = attpath;
	rt->comparaison = comp;
	rt->numinstance = numinstance;
	SetCompOptionWithOperator(rt->fOptions, comp);

	ValueKind datakind = attpath.LastPart()->fAtt->GetDataKind();
	bool simpleDate = attpath.LastPart()->fAtt->isSimpleDate();

	rt->ch = nil;
	rt->fParam = inParamToCompare;
	fParamValues[inParamToCompare] = QueryParamElement();
	rt->fExpectedType = (sLONG)datakind;
	rt->fSimpleDate = simpleDate;

	rt->fCheckForNull = false;

	fLines.push_back(rt);

}


void SearchTab::AddSearchLineEm(AttributePath& attpath, sLONG comp, const VValueSingle* ValueToCompare, const VCompareOptions& inOptions, bool checkfornull, sLONG numinstance)
{
	CheckForNullOn(comp, checkfornull);

	RechTokenEmComp* rt = new RechTokenEmComp;
	rt->fOptions = inOptions;
	rt->fAttPath = attpath;
	rt->comparaison = comp;
	rt->numinstance = numinstance;
	SetCompOptionWithOperator(rt->fOptions, comp);
	ValueKind datakind = attpath.LastPart()->fAtt->GetDataKind();
	if (datakind == VK_EMPTY)
		rt->ch = ValueToCompare->Clone();
	else
	{
		if (datakind == VK_IMAGE)
			datakind = VK_STRING;
		rt->ch = ValueToCompare->ConvertTo(datakind);
	}
	

	rt->fCheckForNull = checkfornull;
	if (rt->comparaison == DB4D_Regex_Not_Match || rt->comparaison == DB4D_Regex_Match)
	{
		VString s;
		ValueToCompare->GetString(s);
		VError err;
		rt->fRegMatcher = VRegexMatcher::Create(s, &err);
	}
		
	fLines.push_back(rt);
}


void SearchTab::AddSearchLineEm(AttributePath& attpath, sLONG comp, const VValueSingle* ValueToCompare, Boolean isDiacritic, bool checkfornull, sLONG numinstance)
{
	VCompareOptions options;
	options.SetDiacritical(isDiacritic);
	AddSearchLineEm(attpath, comp, ValueToCompare, options, checkfornull, numinstance);
}


void SearchTab::AddSearchLineEm(const VString& attpath, sLONG comp, const VValueSingle* ValueToCompare, const VCompareOptions& inOptions, bool checkfornull, sLONG numinstance)
{
	if (fModel != nil)
	{
		AttributePath path(fModel, attpath);
		if (path.IsValid())
			AddSearchLineEm(path, comp, ValueToCompare, inOptions, checkfornull, numinstance);
	}
}


void SearchTab::AddSearchLineEm(const VString& attpath, sLONG comp, const VValueSingle* ValueToCompare, Boolean isDiacritic, bool checkfornull, sLONG numinstance)
{
	if (fModel != nil)
	{
		AttributePath path(fModel, attpath);
		if (path.IsValid())
			AddSearchLineEm(path, comp, ValueToCompare, isDiacritic, checkfornull, numinstance);
	}
}



void SearchTab::AddSearchLineSimple(Field* cri, sLONG comp, const VValueSingle* ValueToCompare, Boolean isDiacritic, sLONG numinstance, bool checkfornull)
{
	CheckForNullOn(comp, checkfornull);
	if (testAssert(cri != nil))
	{
		RechTokenSimpleComp *rt;
		
		assert(cri != nil);
		rt = new RechTokenSimpleComp;
		rt->fOptions.SetDiacritical(isDiacritic);
		rt->numfield = cri->GetPosInRec();
		rt->numfile = cri->GetOwner()->GetNum();
		rt->numinstance = numinstance;
		rt->comparaison = comp;
		SetCompOptionWithOperator(rt->fOptions, comp);
		
		sLONG tt = cri->GetTyp();
		if (tt == VK_IMAGE)
			tt = VK_STRING;
		rt->ch = ValueToCompare->ConvertTo(tt);
		if (rt->ch != nil)
		{
			fLines.push_back(rt);
		}
		rt->fIsDataDirect = cri->IsDataDirect();
		rt->fCheckForNull = checkfornull;
		if (rt->comparaison == DB4D_Regex_Not_Match || rt->comparaison == DB4D_Regex_Match)
		{
			rt->fIsDataDirect = false;
			VString s;
			ValueToCompare->GetString(s);
			VError err;
			rt->fRegMatcher = VRegexMatcher::Create(s, &err);
		}
	}
}


void SearchTab::AddSearchLineSimple(sLONG numfile, sLONG numfield, sLONG comp, const VValueSingle* ValueToCompare, Boolean isDiacritic, sLONG numinstance, bool checkfornull)
{
	Field* cri;
	if (testAssert(destFile != nil))
	{
		if (numfile == 0)
			cri = destFile->RetainField(numfield);
		else
			cri = destFile->GetOwner()->RetainField(numfile, numfield);
		AddSearchLineSimple(cri, comp, ValueToCompare, isDiacritic, numinstance, checkfornull);
		if (cri != nil)
			cri->Release();
	}
}


void SearchTab::AddSearchLineSimple(Field* cri, sLONG comp, const VValueSingle* ValueToCompare, const VCompareOptions& inOptions, sLONG numinstance, bool checkfornull)
{
	CheckForNullOn(comp, checkfornull);
	if (testAssert(cri != nil))
	{
		RechTokenSimpleComp *rt;

		assert(cri != nil);
		rt = new RechTokenSimpleComp;
		rt->fOptions = inOptions;
		rt->numfield = cri->GetPosInRec();
		rt->numfile = cri->GetOwner()->GetNum();
		rt->numinstance = numinstance;
		rt->comparaison = comp;
		SetCompOptionWithOperator(rt->fOptions, comp);
		sLONG tt = cri->GetTyp();
		if (tt == VK_IMAGE)
			tt = VK_STRING;
		rt->ch = ValueToCompare->ConvertTo(tt);
		if (rt->ch != nil)
		{
			fLines.push_back(rt);
		}
		rt->fCheckForNull = checkfornull;
		rt->fIsDataDirect = cri->IsDataDirect();
		if (rt->comparaison == DB4D_Regex_Not_Match || rt->comparaison == DB4D_Regex_Match)
		{
			rt->fIsDataDirect = false;
			VString s;
			ValueToCompare->GetString(s);
			VError err;
			rt->fRegMatcher = VRegexMatcher::Create(s, &err);
		}
	}
}


void SearchTab::AddSearchLineSimple(sLONG numfile, sLONG numfield, sLONG comp, const VValueSingle* ValueToCompare, const VCompareOptions& inOptions, sLONG numinstance, bool checkfornull)
{
	Field* cri;
	if (testAssert(destFile != nil))
	{
		if (numfile == 0)
			cri = destFile->RetainField(numfield);
		else
			cri = destFile->GetOwner()->RetainField(numfile, numfield);
		AddSearchLineSimple(cri, comp, ValueToCompare, inOptions, numinstance, checkfornull);
		if (cri != nil)
			cri->Release();
	}
}


void SearchTab::AddSearchLineSimple(sLONG numfile, sLONG numfield, sLONG comp, const VString& inParamToCompare, const VCompareOptions& inOptions, sLONG numinstance, bool checkfornull)
{
	Field* cri;
	if (testAssert(destFile != nil))
	{
		if (numfile == 0)
			cri = destFile->RetainField(numfield);
		else
			cri = destFile->GetOwner()->RetainField(numfile, numfield);
		AddSearchLineSimple(cri, comp, inParamToCompare, inOptions, numinstance, checkfornull);
		if (cri != nil)
			cri->Release();
	}
	
}


void SearchTab::AddSearchLineSimple(Field* cri, sLONG comp, const VString& inParamToCompare, const VCompareOptions& inOptions, sLONG numinstance, bool checkfornull)
{
	CheckForNullOn(comp, checkfornull);
	if (testAssert(cri != nil))
	{
		RechTokenSimpleComp *rt;
		
		assert(cri != nil);
		rt = new RechTokenSimpleComp;
		rt->fOptions = inOptions;
		rt->numfield = cri->GetPosInRec();
		rt->numfile = cri->GetOwner()->GetNum();
		rt->numinstance = numinstance;
		rt->comparaison = comp;
		SetCompOptionWithOperator(rt->fOptions, comp);
		rt->ch = nil;
		rt->fParam = inParamToCompare;
		fParamValues[inParamToCompare] = QueryParamElement();
		rt->fExpectedType = cri->GetTyp();
		fLines.push_back(rt);
		rt->fIsDataDirect = cri->IsDataDirect();
	}
}


void SearchTab::AddSearchLineSel(Selection* sel, sLONG numinstance)
{
	if (testAssert(sel != nil))
	{
		RechTokenSel *rt = new RechTokenSel(sel, numinstance);
		fLines.push_back(rt);
	}
}


void SearchTab::AddSearchLineEmSel(EntityCollection* sel, sLONG numinstance)
{
	if (testAssert(sel != nil))
	{
		RechTokenEmSel *rt = new RechTokenEmSel(sel, numinstance);
		fLines.push_back(rt);
	}
}

void SearchTab::AddSearchLineArray(Field* cri, sLONG comp, DB4DArrayOfValues *Values, Boolean isDiacritic, sLONG numinstance)
{
	VCompareOptions options;
	options.SetDiacritical( isDiacritic);
	AddSearchLineArray( cri, comp, Values, options, numinstance);
}


void SearchTab::AddSearchLineEmSimple(const EntityAttribute* att, sLONG comp, const VValueSingle* ValueToCompare, const VCompareOptions& inOptions, sLONG numinstance, bool checkfornull)
{
	CheckForNullOn(comp, checkfornull);
	if (testAssert(att != nil))
	{
		RechTokenEMSimpleComp *rt;

		rt = new RechTokenEMSimpleComp;
		rt->fOptions = inOptions;
		rt->fAtt = att;
		rt->numinstance = numinstance;
		rt->comparaison = comp;
		SetCompOptionWithOperator(rt->fOptions, comp);
		sLONG tt = att->ComputeScalarType();
		if (tt == VK_IMAGE)
			tt = VK_STRING;
		rt->ch = ValueToCompare->ConvertTo(tt);
		rt->fExpectedType = tt;
		rt->fSimpleDate = att->isSimpleDate();
		if (rt->ch != nil)
		{
			fLines.push_back(rt);
		}
		rt->fCheckForNull = checkfornull;
		rt->fIsDataDirect = false;
		if (rt->comparaison == DB4D_Regex_Not_Match || rt->comparaison == DB4D_Regex_Match)
		{
			rt->fIsDataDirect = false;
			VString s;
			ValueToCompare->GetString(s);
			VError err;
			rt->fRegMatcher = VRegexMatcher::Create(s, &err);
		}
	}
}


void SearchTab::AddSearchLineEmSimple(const EntityAttribute* att, sLONG comp, const VString& inParamToCompare, const VCompareOptions& inOptions, sLONG numinstance, bool checkfornull)
{
	CheckForNullOn(comp, checkfornull);
	if (testAssert(att != nil))
	{
		RechTokenEMSimpleComp *rt;

		rt = new RechTokenEMSimpleComp;
		rt->fOptions = inOptions;
		rt->fAtt = att;
		rt->numinstance = numinstance;
		rt->comparaison = comp;
		SetCompOptionWithOperator(rt->fOptions, comp);
		rt->ch = nil;
		rt->fParam = inParamToCompare;
		fParamValues[inParamToCompare] = QueryParamElement();
		rt->fExpectedType = att->ComputeScalarType();
		rt->fSimpleDate = att->isSimpleDate();
		fLines.push_back(rt);
		rt->fIsDataDirect = false;
	}
}


void SearchTab::AddSearchLineEmArray(const EntityAttribute* att, sLONG comp, DB4DArrayOfValues *Values, const VCompareOptions& inOptions, sLONG numinstance)
{
	RechTokenEmArrayComp *rt;

	assert(att != nil);
	rt = new RechTokenEmArrayComp;
	rt->fOptions = inOptions;
	rt->fAtt = att;
	rt->numinstance = numinstance;
	rt->comparaison = comp;
	rt->fExpectedType = att->ComputeScalarType();
	rt->fSimpleDate = att->isSimpleDate();
	SetCompOptionWithOperator(rt->fOptions, comp);

	if ( rt->fOptions.IsLike() && ( (rt->fExpectedType == VK_TEXT) || (rt->fExpectedType == VK_STRING) || (rt->fExpectedType == VK_TEXT_UTF8) || (rt->fExpectedType == VK_STRING_UTF8) ) )
	{
		// si aucune des valeurs ne contient de wildcard, on enleve le like
		const VInlineString *begin = (const VInlineString*) Values->GetFirstPtr();
		const VInlineString *end = begin + Values->Count();
		bool found = false;
		for( const VInlineString *i = begin ; (i != end) && !found ; ++i)
			found = std::find( i->fString, i->fString + i->fLength, GetWildChar(nil)) != i->fString + i->fLength;
		if (!found)
			rt->fOptions.SetLike( false);
	}

	rt->values = GenerateConstArrayOfValues(Values, rt->fExpectedType, inOptions);
	rt->fIsDataDirect = false;
	fLines.push_back(rt);
}


void SearchTab::AddSearchLineEmArray(const EntityAttribute* att, sLONG comp, const VString& inParamToCompare, const VCompareOptions& inOptions, sLONG numinstance)
{
	RechTokenEmArrayComp *rt;

	assert(att != nil);
	rt = new RechTokenEmArrayComp;
	rt->fOptions = inOptions;
	rt->fAtt = att;
	rt->numinstance = numinstance;
	rt->comparaison = comp;
	rt->fExpectedType = att->ComputeScalarType();
	SetCompOptionWithOperator(rt->fOptions, comp);
	rt->fSimpleDate = att->isSimpleDate();

	rt->values = nil;
	rt->fParam = inParamToCompare;
	fParamValues[inParamToCompare] = QueryParamElement();

	rt->fIsDataDirect = false;
	fLines.push_back(rt);

}


void SearchTab::AddSearchLineArray(Field* cri, sLONG comp, DB4DArrayOfValues *Values, const VCompareOptions& inOptions, sLONG numinstance)
{
	RechTokenArrayComp *rt;

	assert(cri != nil);
	rt = new RechTokenArrayComp;
	rt->fOptions = inOptions;
	rt->numfield = cri->GetPosInRec();
	rt->numfile = cri->GetOwner()->GetNum();
	rt->numinstance = numinstance;
	rt->comparaison = comp;
	rt->fExpectedType = cri->GetTyp();
	SetCompOptionWithOperator(rt->fOptions, comp);

	if ( rt->fOptions.IsLike() && ( (cri->GetTyp() == VK_TEXT) || (cri->GetTyp() == VK_STRING) || (cri->GetTyp() == VK_TEXT_UTF8) || (cri->GetTyp() == VK_STRING_UTF8) ) )
	{
		// si aucune des valeurs ne contient de wildcard, on enleve le like
		const VInlineString *begin = (const VInlineString*) Values->GetFirstPtr();
		const VInlineString *end = begin + Values->Count();
		bool found = false;
		for( const VInlineString *i = begin ; (i != end) && !found ; ++i)
			found = std::find( i->fString, i->fString + i->fLength, GetWildChar(nil)) != i->fString + i->fLength;
		if (!found)
			rt->fOptions.SetLike( false);
	}

	rt->values = GenerateConstArrayOfValues(Values, cri->GetTyp(), inOptions);
	rt->fIsDataDirect = cri->IsDataDirect();
	fLines.push_back(rt);
}


void SearchTab::AddSearchLineArray(Field* cri, sLONG comp, const VString& inParamToCompare, const VCompareOptions& inOptions, sLONG numinstance)
{
	RechTokenArrayComp *rt;

	assert(cri != nil);
	rt = new RechTokenArrayComp;
	rt->fOptions = inOptions;
	rt->numfield = cri->GetPosInRec();
	rt->numfile = cri->GetOwner()->GetNum();
	rt->numinstance = numinstance;
	rt->comparaison = comp;
	rt->fExpectedType = cri->GetTyp();
	SetCompOptionWithOperator(rt->fOptions, comp);

	rt->values = nil;
	rt->fParam = inParamToCompare;
	fParamValues[inParamToCompare] = QueryParamElement();

	rt->fIsDataDirect = cri->IsDataDirect();
	fLines.push_back(rt);
}


void SearchTab::AddSearchLineArray(Field* cri, sLONG comp, const VString& inParamToCompare, Boolean isDiacritic, sLONG numinstance)
{
	VCompareOptions options;
	options.SetDiacritical( isDiacritic);
	AddSearchLineArray( cri, comp, inParamToCompare, options, numinstance);
}



void SearchTab::AddSearchLineJoin(Field* cri, sLONG comp, Field* OtherCri, Boolean isDiacritic, sLONG numinstance, sLONG numinstanceOther)
{
	RechTokenJoin *rt;

	assert(cri != nil && OtherCri != nil);
	rt = new RechTokenJoin;
	rt->numfield = cri->GetPosInRec();
	rt->numfile = cri->GetOwner()->GetNum();
	rt->numinstance = numinstance;
	rt->comparaison = comp;
	rt->numfieldOther = OtherCri->GetPosInRec();
	rt->numfileOther = OtherCri->GetOwner()->GetNum();
	rt->numinstanceOther = numinstanceOther;
	rt->fOptions.SetDiacritical( isDiacritic);
	//SetCompOptionWithOperator(rt->fOptions, comp);
	rt->fOptions.SetLike(false);
	rt->fOptions.SetBeginsWith(false);
	fLines.push_back(rt);
}


void SearchTab::AddSearchLineJoinEm(const EntityAttribute* att, sLONG comp, const EntityAttribute* OtherAtt, Boolean isDiacritic, sLONG numinstance, sLONG numinstanceOther)
{
	RechTokenEmJoin *rt;

	assert(att != nil && OtherAtt != nil);
	rt = new RechTokenEmJoin;
	rt->fAtt = att;
	rt->fRootBaseEm = att->GetOwner()->GetRootBaseEm();
	rt->numinstance = numinstance;
	rt->comparaison = comp;
	rt->fAttOther = OtherAtt;
	rt->fRootBaseEmOther = OtherAtt->GetOwner()->GetRootBaseEm();
	rt->numinstanceOther = numinstanceOther;
	rt->fOptions.SetDiacritical( isDiacritic);
	//SetCompOptionWithOperator(rt->fOptions, comp);
	rt->fOptions.SetLike(false);
	rt->fOptions.SetBeginsWith(false);
	fLines.push_back(rt);
}



void SearchTab::AddSearchLineRecordExists(sLONG numfile, uBOOL checkIfExists, sLONG numinstance)
{
	RechTokenRecordExist* rt = new RechTokenRecordExist;
	rt->numfile = numfile;
	rt->numinstance = numinstance;
	rt->fCheckIfExists = checkIfExists;
	fLines.push_back(rt);
}


void SearchTab::AddSearchLineRecordExists(Table* inTable, uBOOL checkIfExists, sLONG numinstance)
{
	if (testAssert(inTable != nil))
		AddSearchLineRecordExists(inTable->GetNum(), checkIfExists, numinstance);
}


void SearchTab::AddSearchLineEntityRecordExists(EntityModel* model, uBOOL checkIfExists, sLONG numinstance)
{
	RechTokenEntityRecordExist* rt = new RechTokenEntityRecordExist;
	rt->fModel = model;
	rt->numinstance = numinstance;
	rt->fCheckIfExists = checkIfExists;
	fLines.push_back(rt);
}



void SearchTab::AddSearchLineBoolOper(sLONG BoolOper)
{
	RechTokenBoolOper *rt;
	
	rt = new RechTokenBoolOper;
	rt->BoolLogic = BoolOper;
	fLines.push_back(rt);
}

void SearchTab::AddSearchLineNotOperator(void)
{
	RechToken *rt;
	
	rt = new RechToken;
	rt->TypToken = r_Not;
	fLines.push_back(rt);
}


void SearchTab::AddSearchLineOpenParenthesis(void)
{
	RechToken *rt;

	rt = new RechToken;
	rt->TypToken = r_ParentG;
	fLines.push_back(rt);
}


void SearchTab::AddSearchLineCloseParenthesis(void)
{
	RechToken *rt;
	
	rt = new RechToken;
	rt->TypToken = r_ParentD;
	fLines.push_back(rt);
}


VError SearchTab::PutInto(VStream& buf)
{
	sLONG nb,i;
	VError err;
	
	nb = (sLONG)fLines.size();
	err = buf.PutLong(nb);
	for (i = 0; i < nb && err == VE_OK; i++)
	{
		err = fLines[i]->PutInto(buf);
	}
	
	return(err);
}


VError SearchTab::GetFrom(VStream& buf)
{
	sLONG nb,i;
	VError err;
	RechToken *rt;
	sWORD typ;
	
	err = buf.GetLong(nb);
	for (i = 0; i < nb && err == VE_OK; i++)
	{
		err = buf.GetWord(typ);
		rt = creRechToken(typ);
		if (rt != nil)
		{
			rt->GetFrom(buf, destFile);
			fLines.push_back(rt);
		}
		else 
			err = ThrowBaseError(memfull, DBaction_LoadingQuery);
	}

	return(err);
}



RechToken* creRechToken(sWORD typ)
{
	RechToken *rt = nil;

	switch(typ)
	{
	case r_InterpOper:
		rt = new RechTokenInterpOper;
		break;

	case r_BoolOperator:
		rt = new RechTokenBoolOper;
		break;

	case r_Line:
		rt = new RechTokenSimpleComp;
		break;

	case r_Script:
		rt = new RechTokenScriptComp(nil, 0);
		break;

	case r_LineArray:
		rt = new RechTokenArrayComp;
		break;

	case r_LineJoin:
		rt = new RechTokenJoin;
		break;

default:
		rt = new RechToken();
		rt->TypToken = typ;
		break;
	}

	return(rt);
}


static Boolean isterminator(UniChar c)
{ 
	if (c == '<' || c == '>' || c == '=' || c == '(' || c == ')' || c == '#' || c == '&' || c == '^' || c == '|' /*|| c == '['*/ || c == '%' || c == '!')
		return true;
	else
		return false;
}

static void removeextraspace(VString& s)
{
	Boolean cont = true;

	sLONG len = s.GetLength();
	while (len>0 && cont)
	{
		if (s[len-1] == 32)
			len--;
		else
			cont = false;
	}
	s.Truncate(len);
}


static bool GetJSCode(const VString& input, sLONG& curpos, VString& result)
{
	bool ok = true;
	UniChar c;
	bool insidequotes = false, insidedoublequotes = false;
	sLONG insideParenthesis = 0;
	if (curpos < input.GetLength())
	{
		c = input[curpos];
		if (c != '(')
			ok = false;
		else
		{
			insideParenthesis = 1;
			curpos++;
		}		
	}
	if (ok)
	{
		do {
			if (curpos < input.GetLength())
			{
				c = input[curpos];
				if (insidequotes)
				{
					if (c == 39)
						insidequotes = false;
				}
				else if (insidedoublequotes)
				{
					if (c == 34)
						insidedoublequotes = false;
				}
				else
				{
					if (c == '(')
						insideParenthesis++;
					else if (c == ')')
					{
						insideParenthesis--;
						if (insideParenthesis == 0)
						{
							curpos++;
							c = 0;
						}
					}
				}

				if (c != 0)
				{
					result.AppendUniChar(c);
					curpos++;
				}
			}
			else
				c = 0;
		} while (c != 0);

		if (insideParenthesis != 0)
			ok = false;
	}
	return ok;
}


bool GetNextWord(const VString& input, sLONG& curpos, VString& result)
{
	UniChar c;
	result.Clear();
	bool first = true;
	bool isregularword;
	bool insidequotes = false, insidedoublequotes = false;
	bool WasInQuotes = false;

	do {
		if (curpos<input.GetLength())
		{
			c = input[curpos];
			if (insidequotes && c == 39)
			{
				curpos++;
				c = 0;
			}
			else
			{
				if (insidedoublequotes && c == 34)
				{
					curpos++;
					c = 0;
				}
				else
				{
					if (first && (c == 32 || c == 9 || c == 10 || c == 13))
					{
						curpos++;
					}
					else
					{
						if (!insidequotes && !insidedoublequotes && isterminator(c))
						{
							if (first)
							{
								isregularword = false;
								result.AppendUniChar(c);
								curpos++;
								first = false;
								if (c=='(' || c==')')
									c = 0;
							}
							else
							{
								if (isregularword)
								{
									c = 0;
								}
								else
								{
									result.AppendUniChar(c);
									curpos++;
								}
							}
						}
						else
						{
							if (first)
							{
								if (c == 39)
								{
									WasInQuotes = true;
									insidequotes = true;
								}
								else
								{
									if (c == 34)
									{
										WasInQuotes = true;
										insidedoublequotes = true;
									}
									else
										result.AppendUniChar(c);
								}
								isregularword = true;
								curpos++;
								first = false;
							}
							else
							{
								if (isregularword)
								{
									if ((c == 32 || c == 9 || c == 10 || c == 13) && !result.IsEmpty() && !insidequotes && !insidedoublequotes)
										c = 0;
									else
										result.AppendUniChar(c);
									curpos++;
								}
								else
								{
									c = 0;
								}
							}
						}
					}
				}
			}
		}
		else
			c = 0;
		
	} while(c != 0);
	return WasInQuotes;
}

enum { keyword_not = 1, keyword_openparenthesis, keyword_closeparenthesis, keyword_order, keyword_by, 
		keyword_like, keyword_notlike, keyword_equal, keyword_notequal, keyword_greater, keyword_greaterequal, keyword_less, keyword_lessequal, 
		keyword_begin, keyword_keyword, keyword_and, keyword_or, keyword_except, keyword_null, keyword_match, keyword_notmatch, 
		keyword_plus, keyword_moins, keyword_multiplie, keyword_divise, keyword_modulo, keyword_round, keyword_diviseint, keyword_integer, keyword_javascript,
		keyword_in
};

const CoupleCharLongArray xrech_keywords =
{
	"not", keyword_not,
	"!", keyword_not,
	"(", keyword_openparenthesis,
	")", keyword_closeparenthesis,
	"order", keyword_order,
	"by", keyword_by,
	"=", keyword_like,
	"==", keyword_like,
	"eq", keyword_like,
	"like", keyword_like,
	"===", keyword_equal,
	"is", keyword_equal,
	"eqeq", keyword_equal,
	"#", keyword_notlike,
	"!=", keyword_notlike,
	"!==", keyword_notequal,
	"nene", keyword_notequal,
	"isnot", keyword_notequal,
	"##", keyword_notequal,
	">", keyword_greater,
	"gt", keyword_greater,
	">=", keyword_greaterequal,
	"gteq", keyword_greaterequal,
	"gte", keyword_greaterequal,
	"<", keyword_less,
	"lt", keyword_less,
	"<=", keyword_lessequal,
	"lteq", keyword_lessequal,
	"lte", keyword_lessequal,
	"begin", keyword_begin,
	"%%", keyword_keyword,
	"&", keyword_and,
	"&&", keyword_and,
	"and", keyword_and,
	"|", keyword_or,
	"||", keyword_or,
	"or", keyword_or,
	"^", keyword_except,
	"except", keyword_except,
	"null", keyword_null,
	"matches", keyword_match,
	"=%", keyword_match,
	"!=%", keyword_notmatch,
	"%*", keyword_match,
	"!%*", keyword_notmatch,
	"+", keyword_plus,
	"-", keyword_moins,
	"/", keyword_divise,
	"\\", keyword_diviseint,
	"*", keyword_multiplie,
	"%", keyword_modulo,
	"round", keyword_round,
	"int", keyword_integer,
	"$",keyword_javascript,
	"in",keyword_in,
	"", 0
};

static void GetCompOperLiteral(DB4DComparator keyword, VString& outKeywordString)
{
	switch (keyword)
	{
		case DB4D_Like:
			outKeywordString = "==";
			break;
		case DB4D_Equal:
			outKeywordString = "===";
			break;
		case DB4D_NotLike:
			outKeywordString = "!=";
			break;
		case DB4D_NotEqual:
			outKeywordString = "!==";
			break;
		case DB4D_Greater:
			outKeywordString = ">";
			break;
		case DB4D_GreaterOrEqual:
			outKeywordString = ">=";
			break;
		case DB4D_Lower:
			outKeywordString = "<";
			break;
		case DB4D_LowerOrEqual:
			outKeywordString = "<=";
			break;
		case DB4D_Contains_KeyWord_Like:
		case DB4D_Contains_KeyWord:
			outKeywordString = "%%";
			break;
	}
}


const StrHashTable RechKeywords(xrech_keywords, false);

class JoinPart
{
	public:
		const EntityAttribute* fSourceAtt;
		sLONG fSourceInstance;
		const EntityAttribute* fDestAtt;
		sLONG fDestInstance;
};

typedef vector<JoinPart> Join;

class CachedJoinPath
{
	public:
		CachedJoinPath& operator = (const CachedJoinPath& other)
		{
			fPath = other.fPath;
			return *this;
		}

		Join fPath;
};

typedef map<AttributePath, CachedJoinPath> CachedJoinPathMap;

class JoinCache
{
	public:
		Join& GetPath(const AttributePath& inAttPath)
		{
			Join* join = FindPath(inAttPath);
			if (join == nil)
			{
				Join newjoin;
				AttributePath subpath = inAttPath;
				subpath.RemoveLast();
				while (!subpath.IsEmpty() && join == nil)
				{
					join = FindPath(subpath);
					if (join == nil)
						subpath.RemoveLast();
				}

				const EntityAttributeInstanceCollection* full = inAttPath.GetAll();
				EntityAttributeInstanceCollection::const_iterator curAttInst = full->begin();

				sLONG previnstance = 0;
				if (join != nil)
				{
					newjoin = *join;
					curAttInst = curAttInst + subpath.GetAll()->size();
					previnstance = (*join)[join->size()-1].fDestInstance;
				}
				else
					subpath.Clear();

				while (curAttInst != full->end())
				{
					if (curAttInst->IsPartOfARelation())
					{
						subpath.Add(*curAttInst);
						EntityRelation* relpath = curAttInst->fAtt->GetRelPath();
						if (testAssert(relpath != nil))
						{
							sLONG lastdestnum = -1;
							const vector<EntityRelation*>& path = relpath->GetPath();
							for (vector<EntityRelation*>::const_iterator cur = path.begin(), end = path.end(); cur != end; cur++)
							{
								JoinPart part;
								part.fSourceAtt = (*cur)->GetSourceAtt();
								part.fDestAtt = (*cur)->GetDestAtt();

								EntityModel*  sourceEm = part.fSourceAtt->GetOwner()->GetRootBaseEm();
								EntityModel* destEm = part.fDestAtt->GetOwner()->GetRootBaseEm();

								part.fSourceInstance = previnstance;
								if (sourceEm == destEm)
									dejainstance.GetInstanceAndIncrement(sourceEm);

								part.fDestInstance = dejainstance.GetInstanceAndIncrement(destEm);

								previnstance = part.fDestInstance;

								
								newjoin.push_back(part);

							}

						}
					}
					++curAttInst;
				}

				join = &(fCache[subpath].fPath);
				join->swap(newjoin);
			}

			return *join;
		}

		void InsertJoinIntoQuery(Join& join, SearchTab& query, sLONG& outLastDestInstance)
		{
			outLastDestInstance = 0;
			for (Join::iterator cur = join.begin(), end = join.end(); cur != end; ++cur)
			{
				outLastDestInstance = cur->fDestInstance;
				query.AddSearchLineJoinEm(cur->fSourceAtt, DB4D_Equal, cur->fDestAtt, false, cur->fSourceInstance, cur->fDestInstance);
				query.AddSearchLineBoolOper(DB4D_And);				
			}
		}

		const InstanceMap& GetInstances() const
		{
			return dejainstance;
		}

	protected:
		Join* FindPath(const AttributePath& inAttPath)
		{
			CachedJoinPathMap::iterator found = fCache.find(inAttPath);
			if (found == fCache.end())
			{
				return nil;
			}
			else
			{
				return &(found->second.fPath);
			}
		}

		CachedJoinPathMap fCache;
		InstanceMap dejainstance;
		

};


DB4DArrayOfValues* BuildArrayOfValuesFromString(VString s, sLONG expectedType, VCompareOptions& inOptions)
{
	VCompareOptions options = inOptions;
	if (options.GetIntlManager() == nil)
		options.SetIntlManager(VTask::GetCurrentIntlManager());
	QuickDB4DArrayOfValues* result = CreateDB4DArrayOfDirectValues(expectedType, options, nil);

	if (result != nil)
	{
		sLONG len = s.GetLength();
		sLONG substrlen = len, substrpos = 1;
		if (len >= 2 && s[len-1] == ']')
			substrlen--;
		if (len >= 1 && s[0] == '[')
		{
			substrlen--;
			substrpos++;
		}
		VString s2;
		s.GetSubString(substrpos, substrlen, s2);

		Wordizer ww(s2);
		VectorOfVString elems;
		ww.ExctractStrings(elems, true, ',', false);
		for (VectorOfVString::iterator cur = elems.begin(), end = elems.end(); cur != end; cur++)
		{
			result->AddStringToConvert(*cur);
		}
	}
	
	return result;
}

VError SearchTab::BuildFormulaFromString(const VString& input, sLONG& curpos, EntityModel* model, RechTokenFormula* formula, sLONG curkeyword)
{
	return VE_OK;
}



VError SearchTab::BuildFromString(const VString& input, VString& outOrderBy, BaseTaskInfo* context, EntityModel* model, bool keepmodel, QueryParamElementVector* qparams)
{
	sLONG curpos = 0;
	Boolean stop = false;
	VStr<80> s;
	Boolean waitforfield = true;
	Boolean waitforoper = false;
	Boolean waitforvalue = false;
	Boolean waitforcunj = false;
	bool autofillValue = false;
	VError err = VE_OK;
	AttributePath attpath;
	DB4DComparator oper = (DB4DComparator)0;
	sLONG curkeyword;
	fModel = model;
	sLONG lastFieldpos = -1;
	JoinCache cachePath;
	const InstanceMap& dejainstance = cachePath.GetInstances();

	assert(model != nil);

	SetDisplayTree(true);

	fLines.clear();
	do {
		bool wasInQuotes = false;
		sLONG previouspos = curpos;
		if (autofillValue)
		{
			s = L"true";
			autofillValue = false;
		}
		else
			wasInQuotes = GetNextWord(input, curpos, s);
		if (!wasInQuotes)
			removeextraspace(s);

		if (s.IsEmpty() && !wasInQuotes)
			stop = true;
		else
		{
			curkeyword = RechKeywords[s];
			if (curkeyword == keyword_like || curkeyword == keyword_equal)
			{
				VString s2;
				sLONG curpos2 = curpos;
				bool wasInQuotes2 = GetNextWord(input, curpos2, s2);
				if (!s2.IsEmpty() && !wasInQuotes2)
				{
					removeextraspace(s2);
					if (RechKeywords[s2] == keyword_not)
					{
						curpos = curpos2;
						if (curkeyword == keyword_equal)
							curkeyword = keyword_notequal;
						else
							curkeyword = keyword_notlike;
					}
				}
			}
			if (waitforfield)
			{
				if (curkeyword == keyword_javascript)
				{
					if (fAllowJSCode)
					{
						VString jscode;
						bool wasInParenthesis = GetJSCode(input, curpos, jscode);
						if (!wasInParenthesis)
						{
							err = VE_DB4D_PARENTHESIS_ERROR;
						}
						else
						{
							RechTokenJavaScript* rtscript = new RechTokenJavaScript(jscode);
							fLines.push_back(rtscript);
							waitforfield = false;
							waitforcunj = true;
						}
					}
					else
					{
						err = VE_DB4D_JS_NOT_ALLOWED_IN_THAT_QUERY;
					}
				}
				else
				{
					if (curkeyword == keyword_not)
					{
						AddSearchLineNotOperator();
					}
					else
					{
						if (curkeyword == keyword_openparenthesis)
						{
							AddSearchLineOpenParenthesis();
						}
						else
						{
							if (curkeyword == keyword_order)
							{
								VString s2;
								sLONG newpos = curpos;
								GetNextWord(input, newpos, s2);
								if (RechKeywords[s2] == keyword_by)
								{
								// get all reminder statement until the end of the query string
									if(input.GetLength() > newpos)
									{
										outOrderBy.Clear();
										input.GetSubString(newpos, input.GetLength() - newpos + 1, outOrderBy);
										outOrderBy.RemoveWhiteSpaces();
									}

									stop = true;
								}
							}

							if (!stop)
							{
								lastFieldpos = previouspos;
								if (attpath.FromPath(model, s, true))
								{
									waitforfield = false;
									waitforoper = true;
								}
								else
									err = VE_DB4D_WRONGFIELDREF;
							}
						}
					}
				}
			}
			else
			{
				if (waitforoper)
				{
					oper = (DB4DComparator)0;
					{
						switch(curkeyword)
						{
							case keyword_like:
								oper = DB4D_Like;
								break;
							case keyword_equal:
								oper = DB4D_Equal;
								break;
							case keyword_notlike:
								oper = DB4D_NotLike;
								break;
							case keyword_notequal:
								oper = DB4D_NotEqual;
								break;
							case keyword_greater:
								oper = DB4D_Greater;
								break;
							case keyword_greaterequal:
								oper = DB4D_GreaterOrEqual;
								break;
							case keyword_lessequal:
								oper = DB4D_LowerOrEqual;
								break;
							case keyword_less:
								oper = DB4D_Lower;
								break;
							case keyword_begin:
								oper = DB4D_BeginsWith;
								break;
							case keyword_keyword:
								oper = DB4D_Contains_KeyWord_Like;
								break;
							case keyword_match:
								oper = DB4D_Regex_Match;
								break;
							case keyword_notmatch:
								oper = DB4D_Regex_Not_Match;
								break;
							case keyword_in:
								oper = DB4D_IN;
								break;
						}
					}

					if (oper == 0)
					{
						bool tryformula = false;
						
						if (attpath.LastPart() != nil && attpath.LastPart()->fAtt->GetDataKind() == VK_BOOLEAN)
						{
							oper = DB4D_Like;
							waitforoper = false;
							waitforvalue = true;
							autofillValue = true;
						}
						else
						{
							tryformula = true;
							//err = VE_DB4D_WRONG_COMP_OPERATOR;
						}
						

						if (tryformula)
						{
							RechTokenFormula* formula = new RechTokenFormula();

							RechTokenInterpAtt* rtatt = new RechTokenInterpAtt(attpath);
							formula->fLines.push_back(rtatt);
							err = BuildFormulaFromString(input, curpos, model, formula, curkeyword);
							fLines.push_back(formula);

							waitforoper = false;
							waitforfield = true;
						}
					}
					else
					{
						waitforoper = false;
						waitforvalue = true;
					}

				}
				else
				{
					if (waitforvalue)
					{
						bool checkfornull = false;
						if (!wasInQuotes)
						{
							if (RechKeywords[s] == keyword_null)
								checkfornull = true;
						}

						if (keepmodel)
						{
							AddSearchLineEm(attpath, oper, &s, false, checkfornull);
						}
						else
						{
							sLONG lastdestinstance = 0;
							const EntityAttributeInstance* attinst = attpath.FirstPart();
							const EntityAttributeInstance* next = attpath.NextPart();
							bool needparenthesis = (next != nil);
							if (needparenthesis)
								AddSearchLineOpenParenthesis();

							if (testAssert(attinst != nil))
							{
								const EntityAttribute* att = attinst->fAtt;
								EntityAttributeKind kind = att->GetKind();
								if ( (next != nil) || (kind == eattr_relation_1toN || kind == eattr_relation_Nto1 || kind == eattr_composition))
								{
									Join& join = cachePath.GetPath(attpath);
									cachePath.InsertJoinIntoQuery(join, *this, lastdestinstance);
									attinst = attpath.LastPart();
								}
								bool okfield = false;

								att = attinst->fAtt;
								kind = att->GetKind();
								uBOOL checkExists = 2;
								if (kind == eattr_relation_1toN || kind == eattr_relation_Nto1 || kind == eattr_composition)
								{
									if (checkfornull)
									{
										if (oper == DB4D_Equal || oper == DB4D_Like)
											checkExists = 0;
										else if (oper == DB4D_NotEqual || oper == DB4D_NotLike)
											checkExists = 1;
									}
								}

								if (att->GetKind() == eattr_computedField && !att->BehavesAsStorage())
								{
									if (att->GetScriptKind() == script_javascript && context != nil)
									{
										VJSContext jscontext(context->GetJSContext());

										VJSObject* objfunc = nil;
										VJSObject localObjFunc(jscontext);
										VectorOfVString params;
										if (att->GetScriptObjFunc(script_attr_query, context->GetJSContext(), context, localObjFunc))
											objfunc = &localObjFunc;
										if (objfunc == nil)
										{
											params.push_back("compOper");
											params.push_back("compValue");
											objfunc = context->getContextOwner()->GetJSFunction(att->GetScriptNum(script_attr_query), att->GetScriptStatement(script_attr_query), &params);
										}
										if (objfunc != nil)
										{
											okfield = true;
											EntityModel* sousEm = att->GetOwner();
											VJSValue result(jscontext);
											JS4D::ExceptionRef excep;
											vector<VJSValue> params;
											VJSValue compOper(jscontext);
											VJSValue compValue(jscontext);
											VString sOper;
											GetCompOperLiteral(oper, sOper);
											compOper.SetString(sOper);
											if (checkfornull)
												compValue.SetNull();
											else
											{
												VValueSingle* cvx = nil;
												bool isarray = false;
												if (!s.IsEmpty() && s[0] == ':')
												{
													if (qparams != nil)
													{
														s.Remove(1,1);
														sLONG paramNum = s.GetLong();
														if (paramNum > 0 && paramNum <= qparams->size())
														{
															QueryParamElement* xqp = &(*qparams)[paramNum-1];
															if (xqp->fType == DB4D_QPE_scalar)
															{
																const VValueSingle* xcvx = xqp->fScalar;
																if (xcvx != nil)
																	cvx = xcvx->ConvertTo(att->ComputeScalarType());
															}
															else if (xqp->fType == DB4D_QPE_array)
															{
																isarray = true;
																compValue = *(xqp->fArray);
															}
														}
													}
												}
												else
													cvx = s.ConvertTo(att->ComputeScalarType());

												if (!isarray)
												{
													if (cvx == nil)
														compValue.SetNull();
													else
													{
														compValue.SetVValue(*cvx);
														delete cvx;
													}
												}
											}
											params.push_back(compOper);
											params.push_back(compValue);
											VJSObject emjs(jscontext, VJSEntityModel::CreateInstance(jscontext, sousEm));
											emjs.CallFunction(*objfunc, &params, &result, &excep);
											VString sresult;
											result.GetString(sresult);
											if (sresult.IsEmpty())
												err = VE_DB4D_WRONGFIELDREF;
											else
											{

												SearchTab subquery(sousEm, true);
												VString orderbys;
												subquery.AllowJSCode(fAllowJSCode);
												err = subquery.BuildFromString(sresult, orderbys, context, sousEm, false);
												if (err == VE_OK)
												{
													if (subquery.GetNbLine() > 1)
														AddSearchLineOpenParenthesis();
													for (RechTokenVector::const_iterator cursous = subquery.fLines.begin(), endsous = subquery.fLines.end(); cursous != endsous; cursous++)
													{
														const RechToken* rt = *cursous;
														RechToken* xrt = rt->Clone();
														xrt->addInstance(sousEm->GetRootBaseEm(), lastdestinstance);
														fLines.push_back(xrt);
													}
													if (subquery.GetNbLine() > 1)
														AddSearchLineCloseParenthesis();
												}
											}
										}
										else
										{
											okfield = true;
											if (!s.IsEmpty() && s[0] == ':')
											{
												s.Remove(1,1);
												VCompareOptions options;
												options.SetDiacritical(false);
												AddSearchLineEm(attpath, oper, s, options, lastdestinstance);
											}
											else
												AddSearchLineEm(attpath, oper, &s, false, checkfornull, lastdestinstance);
										}

									}
									else
									{
										okfield = true;
										if (s == L"false" || s == L"0")
											AddSearchLineNotOperator();
										const SearchTab* sousQuery = att->GetScriptQuery();
										if (sousQuery == nil || sousQuery->GetNbLine() == 0)
											err = VE_DB4D_WRONGFIELDREF;
										else
										{
											if (sousQuery->GetNbLine() > 1)
												AddSearchLineOpenParenthesis();
											for (RechTokenVector::const_iterator cursous = sousQuery->fLines.begin(), endsous = sousQuery->fLines.end(); cursous != endsous; cursous++)
											{
												const RechToken* rt = *cursous;
												RechToken* xrt = rt->Clone();
												xrt->addInstance(sousQuery->GetModel()->GetRootBaseEm(), lastdestinstance);
												fLines.push_back(xrt);
											}
											if (sousQuery->GetNbLine() > 1)
												AddSearchLineCloseParenthesis();
										}
									}
								}

								if (att->GetKind() == eattr_alias  && !att->BehavesAsStorage())
								{
									assert(att->GetKind() != eattr_alias);  // break here
								}

								if (checkExists != 2 || att->GetKind() == eattr_storage || att->BehavesAsStorage())
								{
									okfield = true;

									if (checkExists != 2)
									{
										EntityModel* submodel = att->GetSubEntityModel();
										if (testAssert(submodel != nil))
										{
											AddSearchLineEntityRecordExists(submodel, checkExists, lastdestinstance);
										}
									}
									else
									{
										if (!s.IsEmpty() && s[0] == ':')
										{
											s.Remove(1,1);
											VCompareOptions options;
											options.SetDiacritical(false);
											if (oper == DB4D_IN)
												AddSearchLineEmArray(att, oper, s, options, lastdestinstance);
											else
												AddSearchLineEmSimple(att, oper, s, options, lastdestinstance);
										}
										else
										{
											if (oper == DB4D_IN)
											{
												VCompareOptions options;
												options.SetDiacritical(false);
												DB4DArrayOfValues* values = BuildArrayOfValuesFromString(s, att->ComputeScalarType(), options);
												AddSearchLineEmArray(att, oper, values, options, lastdestinstance);
												QuickReleaseRefCountable(values);
											}
											else
											{
												VCompareOptions options;
												options.SetDiacritical(false);
												AddSearchLineEmSimple(att, oper, &s, options, lastdestinstance, checkfornull);
											}
										}
									}
								}

								
								if (!okfield)
									err = VE_DB4D_WRONGFIELDREF;
							}

							if (needparenthesis)
								AddSearchLineCloseParenthesis();
						}

						oper = (DB4DComparator)0;
						waitforvalue = false;
						waitforcunj = true;
					}
					else
					{
						if (waitforcunj)
						{
							if (curkeyword == keyword_closeparenthesis)
							{
								AddSearchLineCloseParenthesis();
							}
							else
							{
								if (curkeyword == keyword_order)
								{
									VString s2;
									sLONG newpos = curpos;
									GetNextWord(input, newpos, s2);
									if (RechKeywords[s2] == keyword_by)
									{
									// get all reminder statement until the end of the query string:
									//		query('name != null order by name, town desc, road asc')
									//			=> outOrderBy is filled with 'name, town desc, road asc'
										if(input.GetLength() > newpos)
										{
											outOrderBy.Clear();
											input.GetSubString(newpos, input.GetLength() - newpos + 1, outOrderBy);
											outOrderBy.RemoveWhiteSpaces();
										}
										stop = true;
									}
								}
								if (!stop)
								{
									sLONG cunj = 0;
									{
										if (curkeyword == keyword_and)
											cunj = DB4D_And;
										else if (curkeyword == keyword_or)
											cunj = DB4D_OR;
										else if (curkeyword == keyword_except)
											cunj = DB4D_Except;
									}
									if (cunj == 0)
										err = VE_DB4D_WRONG_COMP_OPERATOR;
									else
									{
										AddSearchLineBoolOper(cunj);
										waitforcunj = false;
										waitforfield = true;
									}
								}
							}
						}
					}
				}
			}

		}

	} while(!stop && err == VE_OK);

	if (err != VE_OK)
	{
		if (fModel != nil)
			err = fModel->ThrowError(err);
		else if (destFile != nil)
			err = destFile->ThrowError(err, DBaction_BuildingQueryFromString);
		else
			ThrowBaseError(err);
	}
	return err;
}


Boolean SearchTab::WithFormulas() const
{
	Boolean result = false;
	for (RechTokenVector::const_iterator cur = fLines.begin(), end = fLines.end(); cur != end; cur++)
	{
		RechToken* tok = *cur;
		if (tok != nil)
		{
			if (tok->GetTyp() == r_Script)
			{
				result = true;
				break;
			}
		}
	}

	return result;
}


VError SearchTab::ReplaceParams(BaseTaskInfo* context)
{
	VError err = VE_OK;
	
	for (RechTokenVector::const_iterator cur = fLines.begin(), end = fLines.end(); cur != end && err == VE_OK; cur++)
	{
		RechToken* tok = *cur;
		if (tok != nil)
		{
			switch (tok->GetTyp())
			{
				case r_EMComp:
				{
					RechTokenEmComp* tokem = (RechTokenEmComp*)tok;
					if (! tokem->fParam.IsEmpty())
					{
						delete tokem->ch;
						bool deja = false;
						if (tokem->fParam.GetUniChar(1) == '$')
						{
							if (tokem->fParam == L"$userid" || tokem->fParam == L"$currentUserID")
							{
								deja = true;
								VUUID userid;
								if (context != nil)
									context->GetCurrentUserID(userid);
								tokem->ch = (VValueSingle*) VValueSingle::NewValueFromValueKind(tokem->fExpectedType);
								tokem->ch->FromVUUID(userid);
							}
							else if (tokem->fParam == L"$username" || tokem->fParam == L"$currentUser")
							{
								deja = true;
								VString username;
								if (context != nil)
									context->GetCurrentUserName(username);
								tokem->ch = (VValueSingle*) VValueSingle::NewValueFromValueKind(tokem->fExpectedType);
								tokem->ch->FromString(username);
							}
							else if (tokem->fParam == L"$currentdate")
							{
								VTime now;
								VTime::Now(now);
								tokem->ch = (VValueSingle*) VValueSingle::NewValueFromValueKind(tokem->fExpectedType);
								tokem->ch->FromTime(now);
							}
						}
						if (!deja)
						{
							QueryParamElement* qpe = &fParamValues[tokem->fParam];
							if (qpe->fType != DB4D_QPE_scalar || qpe->fScalar == nil)
							{
								tokem->ch = (VValueSingle*) VValueSingle::NewValueFromValueKind(tokem->fExpectedType);
							}
							else
							{
								if (tokem->fSimpleDate && qpe->fSimpleDateScalar != nil)
									tokem->ch = qpe->fSimpleDateScalar->ConvertTo(tokem->fExpectedType);
								else
								{
									if (tokem->fSimpleDate && tokem->fExpectedType == VK_TIME && qpe->fScalar->GetValueKind() == VK_STRING)
									{
										VString* s = (VString*)qpe->fScalar;
										VectorOfVString arr;
										s->GetSubStrings(',', arr, false, false);
										if (arr.size() > 1)
											tokem->ch = arr[1].ConvertTo(tokem->fExpectedType);
										else
											tokem->ch = qpe->fScalar->ConvertTo(tokem->fExpectedType);
									}
									else
										tokem->ch = qpe->fScalar->ConvertTo(tokem->fExpectedType);
								}
							}
						}
						if (tokem->ch == nil)
							err = ThrowBaseError(memfull);
						else
						{
							if (tokem->comparaison == DB4D_Regex_Not_Match || tokem->comparaison == DB4D_Regex_Match)
							{
								VString s;
								tokem->ch->GetString(s);
								tokem->fRegMatcher = VRegexMatcher::Create(s, &err);
							}
						}
					}
				}
				break;

				case r_Line:
				{
					RechTokenSimpleComp* toksimple = (RechTokenSimpleComp*)tok;
					if (! toksimple->fParam.IsEmpty())
					{
						delete toksimple->ch;
						bool deja = false;
						if (toksimple->fParam.GetUniChar(1) == '$')
						{
							if (toksimple->fParam == L"$userid" || toksimple->fParam == L"$currentUserID")
							{
								deja = true;
								VUUID userid;
								if (context != nil)
									context->GetCurrentUserID(userid);
								toksimple->ch = (VValueSingle*) VValueSingle::NewValueFromValueKind(toksimple->fExpectedType);
								toksimple->ch->FromVUUID(userid);
							}
							else if (toksimple->fParam == L"$username" || toksimple->fParam == L"$currentUser")
							{
								deja = true;
								VString username;
								if (context != nil)
									context->GetCurrentUserName(username);
								toksimple->ch = (VValueSingle*) VValueSingle::NewValueFromValueKind(toksimple->fExpectedType);
								toksimple->ch->FromString(username);
							}
							else if (toksimple->fParam == L"$currentdate")
							{
							}
						}
						if (!deja)
						{
							QueryParamElement* qpe = &fParamValues[toksimple->fParam];
							if (qpe->fType != DB4D_QPE_scalar || qpe->fScalar == nil)
							{
								toksimple->ch = (VValueSingle*) VValueSingle::NewValueFromValueKind(toksimple->fExpectedType);
							}
							else
							{
								toksimple->ch = qpe->fScalar->ConvertTo(toksimple->fExpectedType);
							}
						}
						if (toksimple->ch == nil)
							err = ThrowBaseError(memfull);
						else
						{
							if (toksimple->comparaison == DB4D_Regex_Not_Match || toksimple->comparaison == DB4D_Regex_Match)
							{
								toksimple->fIsDataDirect = false;
								VString s;
								toksimple->ch->GetString(s);
								toksimple->fRegMatcher = VRegexMatcher::Create(s, &err);
							}
						}
					}
				}
				break;

				case r_LineEM:
				  {
					  RechTokenEMSimpleComp* toksimple = (RechTokenEMSimpleComp*)tok;
					  if (! toksimple->fParam.IsEmpty())
					  {
						  delete toksimple->ch;
						  bool deja = false;
						  if (toksimple->fParam.GetUniChar(1) == '$')
						  {
							  if (toksimple->fParam == L"$userid" || toksimple->fParam == L"$currentUserID")
							  {
								  deja = true;
								  VUUID userid;
								  if (context != nil)
									  context->GetCurrentUserID(userid);
								  toksimple->ch = (VValueSingle*) VValueSingle::NewValueFromValueKind(toksimple->fExpectedType);
								  toksimple->ch->FromVUUID(userid);
							  }
							  else if (toksimple->fParam == L"$username" || toksimple->fParam == L"$currentUser")
							  {
								  deja = true;
								  VString username;
								  if (context != nil)
									  context->GetCurrentUserName(username);
								  toksimple->ch = (VValueSingle*) VValueSingle::NewValueFromValueKind(toksimple->fExpectedType);
								  toksimple->ch->FromString(username);
							  }
							  else if (toksimple->fParam == L"$currentdate")
							  {
							  }
						  }
						  if (!deja)
						  {
							  QueryParamElement* qpe = &fParamValues[toksimple->fParam];
							  if (qpe->fType != DB4D_QPE_scalar || qpe->fScalar == nil)
							  {
								  toksimple->ch = (VValueSingle*) VValueSingle::NewValueFromValueKind(toksimple->fExpectedType);
							  }
							  else
							  {
								  if (toksimple->fSimpleDate && qpe->fSimpleDateScalar != nil)
								  {
									  toksimple->ch = qpe->fSimpleDateScalar->ConvertTo(toksimple->fExpectedType);
								  }
								  else
								  {
									  if (toksimple->fSimpleDate && toksimple->fExpectedType == VK_TIME && qpe->fScalar->GetValueKind() == VK_STRING)
									  {
										  VString* s = (VString*)qpe->fScalar;
										  VectorOfVString arr;
										  s->GetSubStrings(',', arr, false, false);
										  if (arr.size() > 1)
											  toksimple->ch = arr[1].ConvertTo(toksimple->fExpectedType);
										  else
											  toksimple->ch = qpe->fScalar->ConvertTo(toksimple->fExpectedType);
									  }
									  else
										  toksimple->ch = qpe->fScalar->ConvertTo(toksimple->fExpectedType);
								  }
							  }
						  }
						  if (toksimple->ch == nil)
							  err = ThrowBaseError(memfull);
						  else
						  {
							  if (toksimple->comparaison == DB4D_Regex_Not_Match || toksimple->comparaison == DB4D_Regex_Match)
							  {
								  toksimple->fIsDataDirect = false;
								  VString s;
								  toksimple->ch->GetString(s);
								  toksimple->fRegMatcher = VRegexMatcher::Create(s, &err);
							  }
						  }
					  }
				  }
				  break;

				case r_LineArray:
				{
					RechTokenArrayComp* tokarray = (RechTokenArrayComp*)tok;
					if (! tokarray->fParam.IsEmpty())
					{
						QueryParamElement* qpe = &fParamValues[tokarray->fParam];
						if (qpe->fType == DB4D_QPE_array && qpe->fArray != nil)
						{
							QuickReleaseRefCountable(tokarray->values);
							tokarray->values = GenerateConstArrayOfValues(*qpe->fArray, tokarray->fExpectedType, tokarray->fOptions);
						}
					}
				}
				break;

				case r_LineEMArray:
				  {
					  RechTokenEmArrayComp* tokarray = (RechTokenEmArrayComp*)tok;
					  if (! tokarray->fParam.IsEmpty())
					  {
						  QueryParamElement* qpe = &fParamValues[tokarray->fParam];
						  if (qpe->fType == DB4D_QPE_array && qpe->fArray != nil)
						  {
							  QuickReleaseRefCountable(tokarray->values);
							  tokarray->values = GenerateConstArrayOfValues(*qpe->fArray, tokarray->fExpectedType, tokarray->fOptions);
						  }
					  }
				  }
				  break;
			}
		}
	}
	
	
	return err;
}


VError SearchTab::GetParams(vector<VString>& outParamNames, QueryParamElementVector& outParamValues)
{
	VError err = VE_OK;
	outParamNames.clear();
	outParamValues.clear();
	outParamValues.MustNotDisposeElements();
	try
	{
		for (QueryValueMap::iterator cur = fParamValues.begin(), end = fParamValues.end(); cur != end; cur++)
		{
			outParamNames.push_back(cur->first);
			outParamValues.push_back(cur->second);
		}
	}
	catch (...)
	{
		err = ThrowBaseError(memfull);
		outParamNames.clear();
		outParamValues.clear();
	}
	return err;
}


VError SearchTab::SetParams(const vector<VString>& inParamNames, const QueryParamElementVector& inParamValues)
{
	VSize nbelem = inParamNames.size();
	if (nbelem > inParamValues.size())
		nbelem = inParamValues.size();
	
	for (sLONG i = 0; i < nbelem; i++)
	{
		QueryValueMap::iterator found = fParamValues.find(inParamNames[i]);
		if (found != fParamValues.end())
		{
			found->second.Dispose();
			found->second = inParamValues[i];
		}
	}
	return VE_OK;
}


VError SearchTab::From(const SearchTab& other)
{
	VError err = VE_OK;
	sLONG nb = (sLONG)other.fLines.size();
	fLines.resize(nb, nil);
	{
		RechTokenVector::iterator curdest = fLines.begin();
		for (RechTokenVector::const_iterator cur = other.fLines.begin(), end = other.fLines.end(); cur != end && err == VE_OK; cur++, curdest++)
		{
			*curdest = (*cur)->Clone();
			if (*curdest == nil)
				err = ThrowBaseError(memfull);
		}

		try
		{
			fParamValues = other.fParamValues;
			for (QueryValueMap::iterator cur = fParamValues.begin(), end = fParamValues.end(); cur != end; cur++)
			{
				cur->second.Duplicate();
			}

			sousselect = other.sousselect;
			CopyRefCountable(&destFile, other.destFile);
			curtoken = other.curtoken;
			fCanDelete = true;
			fMustDisplay = other.fMustDisplay;
			fModel = other.fModel;

		}
		catch (...)
		{
			err = ThrowBaseError(memfull);		
		}
	}



	return err;
}



VError SearchTab::Add(const SearchTab& other)
{
	VError err = VE_OK;
	sLONG nbother = (sLONG)other.fLines.size();

	if (nbother > 0)
	{
		bool withparenthesis = false;
		if (fLines.size() > 0)
		{
			withparenthesis = true;
		}

		if (withparenthesis)
		{
			AddSearchLineBoolOper(DB4D_And);
			AddSearchLineOpenParenthesis();
		}
		for (RechTokenVector::const_iterator cur = other.fLines.begin(), end = other.fLines.end(); cur != end && err == VE_OK; cur++)
		{
			RechToken* tok = (*cur)->Clone();
			if (tok == nil)
				err = ThrowBaseError(memfull);
			else
				fLines.push_back(tok);
		}
		if (withparenthesis)
		{
			AddSearchLineCloseParenthesis();
		}
	}

	return err;
}


VError SearchTab::BuildQueryString(BaseTaskInfo* context, VString outQuery)
{
	outQuery.Clear();
	for (RechTokenVector::iterator cur = fLines.begin(), end = fLines.end(); cur != end; ++cur)
	{
		VString s;
		(*cur)->BuildString(s);
		outQuery += s;
	}
	return VE_OK;
}






/* --------------------------------------------------------- */


VError RechNode::Perform(OptimizedQuery *query, Bittab **super, Bittab* filtre, VDB4DProgressIndicator* InProgress,  VValueBag* curdesc,
						 BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, Bittab &Nulls, RechNode* &transformation)
{
	Bittab* b;
	VError err = VE_OK;
	
	uLONG start = query->GetStartingTime();
	b = query->GetTargetFile()->GetDF()->Search(err, this, filtre, InProgress, context, HowToLock, exceptions, limit, Nulls, query->GetModel());

	if (query->IsQueryDescribed())
	{
		VString s;
		FullyDescribe(query->GetTargetFile()->GetOwner(), s);
		query->AddToDescription(curdesc, s, start, b);
	}

	*super = b;
	transformation = this;

	return(err);
}


void RechNode::FillLevelString(sLONG level, VString& s)
{
	s.Clear();
	sLONG i;
	for (i=0;i<level;i++)
		s.AppendString(L"    ");
}



												/* =================================== */

RechNodeJSScript::~RechNodeJSScript()
{
	if (funcInited && !parseError)
		JS4D::UnprotectValue(funcobj.GetContextRef(), funcobj);
	QuickReleaseRefCountable(cachefiche);
}


uBOOL RechNodeJSScript::PerformSeq(FicheOnDisk* ficD, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, 
						 BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, LocalEntityModel* model)
{
	uBOOL result = 2;
	if (!parseError)
	{
		if (cachefiche == nil)
		{
			Table* tt = ficD->GetOwner()->RetainTable();
			cachefiche = new FicheInMem(context, tt, nil);
			if (tt != nil)
				tt->Release();
		}

		if (cachefiche != nil)
		{
			cachefiche->SetAssocData(ficD);
			result = PerformSeq(cachefiche->GetOwner()->GetNum(), cachefiche, tfb, InProgress, context, HowToLock, exceptions, limit, model);
			cachefiche->SetAssocData(nil);
		}
	}

	return result;
}


uBOOL RechNodeJSScript::PerformSeq(sLONG nf, FicheInMem* curfic, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, 
						 BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, LocalEntityModel* model)
{
	uBOOL result = 2;

	if (curfic != nil && context != nil && model != nil && !parseError)
	{
		LocalEntityRecord* erec = new LocalEntityRecord(model, curfic, context);
		if (erec != nil)
		{
			erec->CallDBEvent(dbev_load, context);

			VJSContext jscontext(context->GetJSContext());

			if (!funcInited)
			{
				VString s = L"$$zarbi = function(){ return "+rt->fJSCode+L" }";
				VJSValue result(jscontext);
				JS4D::ExceptionRef excep;
				jscontext.EvaluateScript(s, nil, &result, &excep, nil);

				if (excep != nil)
				{
					VJSValue xexception(jscontext, excep);
					VJSJSON json(jscontext);
					VString mys;
					json.Stringify(xexception, mys);
					parseError = true;
				}

				if (!parseError)
				{
					funcobj.SetContext(jscontext);
					result.GetObject(funcobj);
					JS4D::ProtectValue(funcobj.GetContextRef(), funcobj);
				}
				funcInited = true;
			}

			if (!parseError)
			{
				VJSObject therecord(jscontext, VJSEntitySelectionIterator::CreateInstance(jscontext, new EntitySelectionIterator(erec, context)));

				JS4D::ExceptionRef excep;
				VJSValue outResult(jscontext);
				vector<VJSValue> Params;
				assert(funcobj.IsFunction());
				therecord.CallFunction(funcobj, &Params, &outResult, &excep);

				if (excep != nil)
				{
					ThrowJSExceptionAsError(jscontext, excep);
					ThrowBaseError(VE_DB4D_JS_ERR);
				}
				else
				{
					if (!outResult.IsNull())
					{
						bool res = false;
						outResult.GetBool(&res);
						result = res;
					}
				}
			}

			erec->Release();
		}
	}

	return result;
}


void RechNodeJSScript::Describe(Base4D* bd, VString& result)
{
	result = L"$("+rt->fJSCode+L")";
}



void RechNodeJSScript::DisplayTree(Base4D* bd, sLONG level, VString* result, VValueBag* bagResult)
{
	VString s2;
	Describe(bd, s2);
	if (bagResult != nil)
		bagResult->SetString(d4::item, s2);
	VString s;
	FillLevelString(level, s);
	s += s2;
	AddCR(s);

	if (result == nil)
		DebugMsg(s);
	else
		*result += s;

}


CDB4DQueryPathNode* RechNodeJSScript::BuildQueryPath(Boolean inBefore, Table* target, Base4D* bd, sLONG DejaSel, CDB4DQueryPathNode* parent, sLONG& curvar, sLONG &outvarnum, const CDB4DQueryPathModifiers* inModifiers)
{
	return nil;
}



									
												/* =================================== */

RechNodeScript::RechNodeScript(void)
{
	TypNode=NoeudScript;
	fLanguageContext = nil;
	cacherec = nil;
	cachefiche = nil;
}
	
RechNodeScript::~RechNodeScript()
{
	if (cachefiche != nil)
		cachefiche->Release();

	if (cacherec != nil)
		cacherec->Release();
}

uBOOL RechNodeScript::PerformSeq(sLONG nf, FicheInMem* curfic, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, 
								 BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, LocalEntityModel* model)
{
	Boolean result = false;
	VError err = VE_OK;
	// executer le script qui doit retourner une boolean
	// le contexte d'execution doit etre remplis avec tfb
	
	if (curfic != nil)
	{
		if (cacherec == nil)
		{
			cacherec = new VDB4DRecord(VDBMgr::GetManager(), nil, context->GetEncapsuleur());
		}
		
		if (cacherec != nil)
		{
			dynamic_cast<VDB4DRecord*>(cacherec)->SetRec(curfic);

			uBOOL subres = rt->expression->ExecuteForQuery(context->GetEncapsuleur(), fLanguageContext, cacherec, err);
			if (subres)
				result = true;
			dynamic_cast<VDB4DRecord*>(cacherec)->SetRec(nil);
		}
	}

	
	return(result);
}


uBOOL RechNodeScript::PerformSeq(FicheOnDisk* ficD, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, 
								 BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, LocalEntityModel* model)
{
	// executer le script qui doit retourner une boolean
	// le contexte d'execution doit etre remplis avec tfb

	Boolean result = false;
	VError err = VE_OK;
	// executer le script qui doit retourner une boolean
	// le contexte d'execution doit etre remplis avec tfb

	if (ficD != nil)
	{
		if (cachefiche == nil)
		{
			Table* tt = ficD->GetOwner()->RetainTable();
			cachefiche = new FicheInMem(context, tt, nil);
			if (tt != nil)
				tt->Release();
		}

		if (cachefiche != nil)
		{
			cachefiche->SetAssocData(ficD);
			if (cacherec == nil)
			{
				cacherec = new VDB4DRecord(VDBMgr::GetManager(), nil, context->GetEncapsuleur());
			}

			if (cacherec != nil)
			{
				dynamic_cast<VDB4DRecord*>(cacherec)->SetRec(cachefiche);

				uBOOL subres = rt->expression->ExecuteForQuery(context->GetEncapsuleur(), fLanguageContext, cacherec, err);
				if (subres)
					result = true;
				dynamic_cast<VDB4DRecord*>(cacherec)->SetRec(nil);
			}
			cachefiche->SetAssocData(nil);
		}
	}


	return(result);
}


void RechNodeScript::Describe(Base4D* bd, VString& result)
{
	result = L"4D Script";
}


void RechNodeScript::DisplayTree(Base4D* bd, sLONG level, VString* result, VValueBag* bagResult)
{
	VString s2;
	Describe(bd, s2);
	if (bagResult != nil)
		bagResult->SetString(d4::item, s2);
	VString s;
	FillLevelString(level, s);
	s += s2;
	AddCR(s);

	if (result == nil)
		DebugMsg(s);
	else
		*result += s;

}


CDB4DQueryPathNode* RechNodeScript::BuildQueryPath(Boolean inBefore, Table* target, Base4D* bd, sLONG DejaSel, CDB4DQueryPathNode* parent, sLONG& curvar, sLONG &outvarnum, const CDB4DQueryPathModifiers* inModifiers)
{
	return nil;
}

								

												/* =================================== */

static VString tabsuffix = L"        ";

static sLONG CalcTableNum(sLONG tablenum, Table* target)
{
	if (tablenum == 0)
	{
		if (target == nil)
			return 0;
		else
			return target->GetNum();
	}
	else
		return tablenum;
}


static void BuildSelectionName(sLONG selnum, VString& outString, const CDB4DQueryPathModifiers* inModifiers)
{
	if (selnum == -1)
	{
		outString = L"the current selection";
	}
	else
	{
		VString s2;
		s2.FromLong(selnum);
		outString = L"TempSel_";
		outString += s2;
	}
}


static void BuildArrayName(sLONG arraynum, VString& outString, const CDB4DQueryPathModifiers* inModifiers)
{
	VString s2;
	s2.FromLong(arraynum);
	outString = L"TempArray_";
	outString += s2;
}


static void BuildAddFieldName(Base4D* bd, sLONG tablenum, sLONG fieldnum, VString& outString, const CDB4DQueryPathModifiers* inModifiers, sLONG numinstance)
{
	Table* target = bd->RetainTable(tablenum);
	Field* cri;
	if (target != nil)
	{
		VString s2, s3;
		target->GetName(s2);
		if (numinstance != 0)
			s2+=L"("+ToString(numinstance)+L")";
		cri = target->RetainField(fieldnum);
		if (cri != nil)
		{
			cri->GetName(s3);
			cri->Release();
			outString += s2;
			outString += L".";
			outString += s3;
		}
		target->Release();
	}
}


static void BuildAddTableName(Base4D* bd, sLONG tablenum, VString& outString, const CDB4DQueryPathModifiers* inModifiers, sLONG numinstance)
{
	Table* target = bd->RetainTable(tablenum);
	if (target != nil)
	{
		VString s2;
		target->GetName(s2);
		if (numinstance != 0)
			s2+=L"("+ToString(numinstance)+L")";
		outString += s2;
		target->Release();
	}
}


static void BuildSeqString(Base4D* bd, sLONG tablenum, sLONG DejaSel, sLONG newsel, VString& outString, const CDB4DQueryPathModifiers* inModifiers, sLONG numinstance)
{
	if (DejaSel == 0)
	{
		outString = L"Sequentially scan the whole table ";
	}
	else
	{
		outString = L"Use ";
		VString selname;
		BuildSelectionName(DejaSel, selname, inModifiers);
		outString += selname;
		outString += L" to scan table ";
	}

	BuildAddTableName(bd, tablenum, outString, inModifiers, numinstance);
	outString += L" to build ";
	VString selname2;
	BuildSelectionName(newsel, selname2, inModifiers);
	outString += selname2;
}


static void BuildSeqStringNonVerbose(Base4D* bd, sLONG tablenum, sLONG DejaSel, sLONG newsel, const VString& formula, VString& outString, const CDB4DQueryPathModifiers* inModifiers, sLONG numinstance)
{
	VString selname2, selname;
	BuildSelectionName(newsel, selname2, inModifiers);

	if (inModifiers->isSimpleTree())
	{
		outString = L"Seq : ";
	}
	else
	{
		outString = selname2;
		outString+= L" := Sequentially scan ";
		BuildAddTableName(bd, tablenum, outString, inModifiers, numinstance);
	
	outString += L" { ";
	}

	outString += formula;

	if (!inModifiers->isSimpleTree())
		outString += L" } ";
	
	if (DejaSel == 0)
	{
		if (!inModifiers->isSimpleTree())
			outString += L"(on the whole table) ";
	}
	else
	{
		if (inModifiers->isSimpleTree())
		{
			if (inModifiers->isTempVarDisplay())
			{
				outString += tabsuffix;
				outString += L"(on ";
			}
		}
		else
			outString += L"(filtered by ";
		if (inModifiers->isTempVarDisplay())
		{
			BuildSelectionName(DejaSel, selname, inModifiers);
			outString += selname;
			outString += L") ";
		}
	}
	if (inModifiers->isSimpleTree() && inModifiers->isTempVarDisplay())
	{
		outString += L"  -->  ";
		outString += selname2;
	}
}


static void BuildBeginIndexString(const IndexInfo* ind, sLONG newsel, VString& outString, const CDB4DQueryPathModifiers* inModifiers)
{
	outString = L"Scans the ";
	VString sind;
	ind->IdentifyIndex(sind, true, true);
	outString += sind;
	outString +=  L" for keys ";
}


static void BuildEndIndexString(const IndexInfo* ind, sLONG newsel, Boolean inverse, VString& outString, const CDB4DQueryPathModifiers* inModifiers)
{
	outString = L" to build ";
	VString selname;
	BuildSelectionName(newsel, selname, inModifiers);
	outString += selname;
	if (inverse)
	{
		outString += L", then performs a NOT on ";
		outString += selname;
	}
}



static void BuildBeginIndexStringNonVerbose(const IndexInfo* ind, sLONG newsel, VString& outString, const CDB4DQueryPathModifiers* inModifiers)
{
	VString selname;
	BuildSelectionName(newsel, selname, inModifiers);

	if (!inModifiers->isSimpleTree())
	{
		outString = selname;
		outString += L" := scan ";
	}

	VString sind;
	ind->IdentifyIndex(sind, true, true);
	outString += sind;

	if (!inModifiers->isSimpleTree())
	{
		outString +=  L" for keys ";
	}
}


static void BuildEndIndexStringNonVerbose(const IndexInfo* ind, sLONG newsel, Boolean inverse, VString& outString, const CDB4DQueryPathModifiers* inModifiers)
{
	//outString.Clear();
	VString selname;
	BuildSelectionName(newsel, selname, inModifiers);
	if (inModifiers->isSimpleTree())
	{
		if (inModifiers->isTempVarDisplay())
		{
			outString += L"  -->  ";
			if (inverse)
				outString += L" NOT ";
			outString += selname;
		}
	}
	else
	{
		if (inverse)
		{
			outString += L", then ";
			outString += selname;
			outString += L" := NOT ";
			outString += selname;
		}
	}
}


static void BuildAddFormula(VString& outString, const CDB4DQueryPathModifiers* inModifiers)
{
	outString += "With the criterias: ";
}



												/* =================================== */


void GetOperString(sWORD comparaison, VString& soper)
{

	switch (comparaison)
	{
		case DB4D_BeginsWith:
			soper = L" Begin with ";
			break;

		case DB4D_Equal:
			soper = L" === ";
			break;

		case DB4D_Like:
			soper = L" == ";
			break;

		case DB4D_NotEqual:
			soper = L" !== ";
			break;

		case DB4D_NotLike:
			soper = L" != ";
			break;

		case DB4D_Greater:
		case DB4D_Greater_Like:
			soper = L" > ";
			break;

		case DB4D_GreaterOrEqual:
		case DB4D_GreaterOrEqual_Like:
			soper = L" >= ";
			break;

		case DB4D_Lower:
		case DB4D_Lower_Like:
			soper = L" < ";
			break;

		case DB4D_LowerOrEqual:
		case DB4D_LowerOrEqual_Like:
			soper = L" <= ";
			break;

		case DB4D_Contains_KeyWord:
			soper = L" contains ";
			break;

		case DB4D_Contains_KeyWord_Like:
			soper = L" contains like ";
			break;

		case DB4D_Regex_Match:
			soper = L" matches ";
			break;

		case DB4D_Regex_Not_Match:
			soper = L" does not matche ";
			break;

		default:
			soper = L" <Nop> ";
			break;
	}
}
									
									

RechNodeSeq::RechNodeSeq(void)
{
	TypNode=NoeudSeq;
	rt = nil;
}


uBOOL RechNodeSeq::PerformSeq(sLONG nf, FicheInMem* curfic, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, 
							  BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, LocalEntityModel* model)
{
	uBOOL ok = false;
	ValPtr cv;
	VError err;
	ValueKind vk;

	cv=nil;
	assert((rt->numfile==0) || (rt->numfile==nf));
	
	cv=curfic->GetNthField(rt->numfield, err, false, true);
	
	if (cv!=nil)
	{
		if (cv->IsNull())
		{
			if (rt->fCheckForNull)
			{
				ok = (rt->comparaison == DB4D_Like) || (rt->comparaison == DB4D_Equal);
			}
			else
				ok = 2;
		}
		else
		{
			if (rt->fCheckForNull)
			{
				ok = !((rt->comparaison == DB4D_Like) || (rt->comparaison == DB4D_Equal));
			}
			else
			{
				switch (rt->comparaison)
				{
					case DB4D_BeginsWith:
					case DB4D_Equal:
					case DB4D_Like:
						ok = (cv->CompareToSameKindWithOptions(rt->ch, rt->fOptions) == CR_EQUAL);
						break;
						
					case DB4D_NotEqual:
					case DB4D_NotLike:
						ok = (cv->CompareToSameKindWithOptions(rt->ch, rt->fOptions) != CR_EQUAL);
						break;

					case DB4D_Greater_Like:
					case DB4D_Greater:
						ok = (cv->CompareToSameKindWithOptions(rt->ch, rt->fOptions) == CR_BIGGER);
						break;

					case DB4D_GreaterOrEqual_Like:
					case DB4D_GreaterOrEqual:
						ok = (cv->CompareToSameKindWithOptions(rt->ch, rt->fOptions) != CR_SMALLER);
						break;

					case DB4D_Lower_Like:
					case DB4D_Lower:
						ok = (cv->CompareToSameKindWithOptions(rt->ch, rt->fOptions) == CR_SMALLER);
						break;

					case DB4D_LowerOrEqual_Like:
					case DB4D_LowerOrEqual:
						ok = (cv->CompareToSameKindWithOptions(rt->ch, rt->fOptions) != CR_BIGGER);
						break;

					case DB4D_Regex_Match:
						if (rt->fRegMatcher != nil)
						{
							VString s;
							cv->GetString(s);
							ok = rt->fRegMatcher->Find(s, 1, false, &err);
						}
						else
							ok = 2;
						break;

					case DB4D_Regex_Not_Match:
						if (rt->fRegMatcher != nil)
						{
							VString s;
							cv->GetString(s);
							ok = ! rt->fRegMatcher->Find(s, 1, false, &err);
						}
						else
							ok = 2;
						break;

					case DB4D_Contains_KeyWord:	
					case DB4D_Contains_KeyWord_Like:
					case DB4D_Contains_KeyWord_BeginingWith:
						vk = cv->GetValueKind();
						if (vk == VK_STRING || vk == VK_TEXT || vk == VK_STRING_UTF8 || vk == VK_TEXT_UTF8) 
						{
							ValueKind vk2 = rt->ch->GetValueKind();
							if (vk2 == VK_STRING || vk2 == VK_TEXT || vk2 == VK_STRING_UTF8 || vk2 == VK_TEXT_UTF8)
							{
								VError err2;
								ok = VDBMgr::ContainsKeyword( *(VString*)(cv), *(VString*)(rt->ch), rt->fOptions, err2);
								/*
								if (rt->comparaison == DB4D_Contains_KeyWord)
									ok = VDBMgr::ContainsKeyword( *(VString*)(cv), *(VString*)(rt->ch), rt->fOptions, VIntlMgr::GetDefaultMgr(), err2);
								else
									ok = VDBMgr::ContainsKeyword_Like( *(VString*)(cv), *(VString*)(rt->ch), rt->fIsDiacritic, VIntlMgr::GetDefaultMgr(), err2);
								*/
							}
						}
						break;
				}
			}
		}
		
	} // du if cv!=nil
	else
	{
		if (rt->fCheckForNull)
		{
			ok = (rt->comparaison == DB4D_Like) || (rt->comparaison == DB4D_Equal);
		}
		else
			ok = 2;
	}
	
	return(ok);
	
}



uBOOL RechNodeSeq::PerformSeq(FicheOnDisk* ficD, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, 
							  BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, LocalEntityModel* model)
{
	uBOOL ok = false;
	sLONG typondisk;
	void* data = (void*)(ficD->GetDataPtrForQuery(rt->numfield, &typondisk, false));
	uLONG tempdata[4];
	VError err;

	if (data == nil)
	{
		if (rt->fIsNeverNull == 2)
		{
			Field* cri = nil;
			VRefPtr<Table> assoctable(ficD->GetOwner()->RetainTable(), false);
			if (assoctable.Get() != nil)
				cri = assoctable->RetainField(rt->numfield);
			if (cri != nil)
			{
				rt->fIsNeverNull = cri->IsNeverNull();
				cri->Release();
			}
			else
				rt->fIsNeverNull = false;
		}
		if (rt->fIsNeverNull)
		{
			data = &tempdata;
			typondisk = (sLONG)rt->ch->GetTrueValueKind();
			std::fill(&tempdata[0], &tempdata[4], 0);
		}
	}

	if (data!=nil)
	{
		if (rt->fCheckForNull)
		{
			ok = !((rt->comparaison == DB4D_Like) || (rt->comparaison == DB4D_Equal));
		}
		else
		{
			if (rt->ch->IsNull())
			{
				ok = 2;
			}
			else
			{
				sLONG rechtyp = (sLONG)rt->ch->GetTrueValueKind();
				if (rt->comparaison == DB4D_Contains_KeyWord || rt->comparaison == DB4D_Contains_KeyWord_Like || rt->comparaison == DB4D_Contains_KeyWord_BeginingWith)
				{
					if (rechtyp == VK_STRING || rechtyp == VK_TEXT || rechtyp == VK_STRING_UTF8 || rechtyp == VK_TEXT_UTF8) 
					{
						ValPtr cv = NewValPtr(VK_STRING, data, typondisk, ficD->GetOwner(), context);
						if (cv != nil)
						{
							VError err2;
							ok = VDBMgr::ContainsKeyword( *(VString*)(cv), *(VString*)(rt->ch), rt->fOptions, err2);
							delete cv;
						}
					}

				}
				else
				{
					if (typondisk == rechtyp && rt->fIsDataDirect)
					{
						switch (rt->comparaison)
						{
						case DB4D_BeginsWith:
						case DB4D_Like:
							ok = rt->ch->Swap_CompareToSameKindPtrWithOptions(data, rt->fOptions) == CR_EQUAL;
							break;

						case DB4D_Equal:
							ok = rt->ch->CompareToSameKindPtrWithOptions(data, rt->fOptions) == CR_EQUAL;
							break;

						case DB4D_NotLike:
							ok = rt->ch->Swap_CompareToSameKindPtrWithOptions(data, rt->fOptions) != CR_EQUAL;
							break;

						case DB4D_NotEqual:
							ok = rt->ch->CompareToSameKindPtrWithOptions(data, rt->fOptions) != CR_EQUAL;
							break;

						case DB4D_Greater:
							ok = rt->ch->CompareToSameKindPtrWithOptions(data, rt->fOptions) == CR_SMALLER;
							break;

						case DB4D_Greater_Like:
							ok = XBOX::VValue::InvertCompResult(rt->ch->Swap_CompareToSameKindPtrWithOptions(data, rt->fOptions)) == CR_SMALLER;
							break;

						case DB4D_GreaterOrEqual:
							ok = rt->ch->CompareToSameKindPtrWithOptions(data, rt->fOptions) != CR_BIGGER;
							break;

						case DB4D_GreaterOrEqual_Like:
							ok = XBOX::VValue::InvertCompResult(rt->ch->Swap_CompareToSameKindPtrWithOptions(data, rt->fOptions)) != CR_BIGGER;
							break;

						case DB4D_Lower:
							ok = rt->ch->CompareToSameKindPtrWithOptions(data, rt->fOptions) == CR_BIGGER;
							break;

						case DB4D_Lower_Like:
							ok = XBOX::VValue::InvertCompResult(rt->ch->Swap_CompareToSameKindPtrWithOptions(data, rt->fOptions)) == CR_BIGGER;
							break;

						case DB4D_LowerOrEqual:
							ok = rt->ch->CompareToSameKindPtrWithOptions(data, rt->fOptions) != CR_SMALLER;
							break;

						case DB4D_LowerOrEqual_Like:
							ok = XBOX::VValue::InvertCompResult(rt->ch->Swap_CompareToSameKindPtrWithOptions(data, rt->fOptions)) != CR_SMALLER;
							break;

						}
					}
					else
					{
						ValPtr cv = NewValPtr(rechtyp, data, typondisk, ficD->GetOwner(), context);
						if (cv != nil)
						{
							switch (rt->comparaison)
							{
								case DB4D_Like:
								case DB4D_Equal:
								case DB4D_BeginsWith:
									ok = (cv->CompareToSameKindWithOptions(rt->ch, rt->fOptions) == CR_EQUAL);
									break;

								case DB4D_NotLike:
								case DB4D_NotEqual:
									ok = (cv->CompareToSameKindWithOptions(rt->ch, rt->fOptions) != CR_EQUAL);
									break;

								case DB4D_Greater_Like:
								case DB4D_Greater:
									ok=(cv->CompareToSameKindWithOptions(rt->ch, rt->fOptions)==CR_BIGGER);
									break;

								case DB4D_GreaterOrEqual_Like:
								case DB4D_GreaterOrEqual:
									ok=(cv->CompareToSameKindWithOptions(rt->ch, rt->fOptions)!=CR_SMALLER);
									break;

								case DB4D_Lower_Like:
								case DB4D_Lower:
									ok=(cv->CompareToSameKindWithOptions(rt->ch, rt->fOptions)==CR_SMALLER);
									break;

								case DB4D_LowerOrEqual_Like:
								case DB4D_LowerOrEqual:
									ok=(cv->CompareToSameKindWithOptions(rt->ch, rt->fOptions)!=CR_BIGGER);
									break;

								case DB4D_Regex_Match:
									if (rt->fRegMatcher != nil)
									{
										VString s;
										cv->GetString(s);
										ok = rt->fRegMatcher->Find(s, 1, false, &err);
									}
									else
										ok = 2;
									break;

								case DB4D_Regex_Not_Match:
									if (rt->fRegMatcher != nil)
									{
										VString s;
										cv->GetString(s);
										ok = ! rt->fRegMatcher->Find(s, 1, false, &err);
									}
									else
										ok = 2;
									break;

							}
							
							delete cv;
						}
					}
				}
			}
		}
	}
	else // quand la valeur sur disque est null
	{
		if (rt->fCheckForNull)
		{
			ok = (rt->comparaison == DB4D_Like) || (rt->comparaison == DB4D_Equal);
		}
		else
			ok = 2;
	}

	return(ok);

}

void RechNodeSeq::BuildQueryPathSeq(Boolean inBefore, sLONG& curvar, CDB4DQueryPathNode* parent, Base4D* bd, VString& outString, const CDB4DQueryPathModifiers* inModifiers, Boolean first) 
{
	VString sfield, soper, svalue;

	if (first)
		BuildAddFormula(outString, inModifiers);

	Field *f = bd->RetainField(rt->numfile, rt->numfield);

	if (f != nil)
	{
		f->GetName(sfield);
		if (rt->numinstance != 0)
			sfield += L"("+ToString(rt->numinstance)+L")";
		f->Release();
	}

	GetOperString(rt->comparaison, soper);

	if (rt->ch != nil)
	{
		if (rt->ch->IsNull() || rt->fCheckForNull)
			svalue = L"null";
		else
			svalue.FromValue(*(rt->ch));
	}
	else
		svalue = L"null";

	outString += sfield;
	outString += soper;
	outString += svalue;
}



CDB4DQueryPathNode* RechNodeSeq::BuildQueryPath(Boolean inBefore, Table* target, Base4D* bd, sLONG DejaSel, CDB4DQueryPathNode* parent, sLONG& curvar, sLONG &outvarnum, const CDB4DQueryPathModifiers* inModifiers)
{
	CDB4DQueryPathNode* result = nil;
	VString smess, smess2;

	curvar++;
	outvarnum = curvar;
	if (inModifiers->isVerbose())
	{
		BuildSeqString(bd, CalcTableNum(fTargetTable, target), DejaSel, outvarnum, smess, inModifiers, GetInstance());
		result = new VDB4DQueryPathNode(parent, smess, inModifiers, true);
		BuildQueryPathSeq(true, curvar, nil, bd, smess2, inModifiers, true);
		CDB4DQueryPathNode* subline = new VDB4DQueryPathNode(result, smess2, inModifiers, false);
	}
	else
	{
		BuildQueryPathSeq(false, curvar, nil, bd, smess2, inModifiers, false);
		BuildSeqStringNonVerbose(bd, CalcTableNum(fTargetTable, target), DejaSel, outvarnum, smess2, smess, inModifiers, GetInstance());
		result = new VDB4DQueryPathNode(parent, smess, inModifiers, inBefore);
	}
	return result;
}


void RechNodeSeq::Describe(Base4D* bd, VString& result)
{
	VString s, s2, s3;

	Table* t = bd->RetainTable(rt->numfile);
	Field* f = nil;
	s2 = L"<Null field>";
	if (t != nil)
	{
		f = t->RetainField(rt->numfield);
		if (f != nil)
		{
			t->GetName(s2);
			if (rt->numinstance != 0)
				s2 += L"("+ToString(rt->numinstance)+L")";
			s2.AppendString(L".");
			f->GetName(s3);
			s2.AppendString(s3);
			f->Release();
		}
		t->Release();
	}
	s.AppendString(s2);

	if (rt->fCheckForNull)
	{
		if (rt->comparaison == DB4D_NotEqual || rt->comparaison == DB4D_NotLike)
			s2 = " is not null";
		else
			s2 = " is null";
	}
	else
	{
		GetOperString(rt->comparaison, s2);

		s.AppendString(s2);

		if (rt->ch != nil && !rt->fCheckForNull)
			s2.FromValue(*(rt->ch));
		else
			s2 = L"null";
	}

	s.AppendString(s2);
	result = s;
}



void RechNodeSeq::DisplayTree(Base4D* bd, sLONG level, VString* result, VValueBag* bagResult)
{
	VString s, s2, s3;
	FillLevelString(level, s);
	Describe(bd, s2);
	if (bagResult != nil)
		bagResult->SetString(d4::item, s2);
	s += s2;
	AddCR(s);

	if (result == nil)
		DebugMsg(s);
	else
		*result += s;

}

								

												/* =================================== */



uBOOL RechNodeEmSeq::PerformSeq(FicheOnDisk* ficD, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, 
								   BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, LocalEntityModel* model)
{
	uBOOL result = 2;
	if (cachefiche == nil)
	{
		Table* tt = ficD->GetOwner()->RetainTable();
		cachefiche = new FicheInMem(context, tt, nil);
		if (tt != nil)
			tt->Release();
	}

	if (cachefiche != nil)
	{
		cachefiche->SetAssocData(ficD);
		result = PerformSeq(cachefiche->GetOwner()->GetNum(), cachefiche, tfb, InProgress, context, HowToLock, exceptions, limit, model);
		cachefiche->SetAssocData(nil);
	}

	return result;
}


uBOOL RechNodeEmSeq::PerformSeq(sLONG nf, FicheInMem* curfic, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, 
								   BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, LocalEntityModel* model)
{
	uBOOL ok = 2;
	VError err = VE_OK;
	ValueKind vk;
	if (curfic != nil && context != nil && model != nil)
	{
		LocalEntityRecord* erec = new LocalEntityRecord(model, curfic, context);
		if (erec != nil)
		{
			erec->CallDBEvent(dbev_load, context);

			EntityAttributeValue* val = erec->getAttributeValue(rt->fAttPath, err, context);
			if (val != nil)
			{
				if (val->getVValue() != nil)
				{
					VValueSingle* cv = val->getVValue();
					if (rt->ch != nil && !rt->ch->IsNull())
					{
						switch (rt->comparaison)
						{
						case DB4D_BeginsWith:
						case DB4D_Equal:
						case DB4D_Like:
							ok = (cv->CompareToSameKindWithOptions(rt->ch, rt->fOptions) == CR_EQUAL);
							break;

						case DB4D_NotEqual:
						case DB4D_NotLike:
							ok = (cv->CompareToSameKindWithOptions(rt->ch, rt->fOptions) != CR_EQUAL);
							break;

						case DB4D_Greater_Like:
						case DB4D_Greater:
							ok = (cv->CompareToSameKindWithOptions(rt->ch, rt->fOptions) == CR_BIGGER);
							break;

						case DB4D_GreaterOrEqual_Like:
						case DB4D_GreaterOrEqual:
							ok = (cv->CompareToSameKindWithOptions(rt->ch, rt->fOptions) != CR_SMALLER);
							break;

						case DB4D_Lower_Like:
						case DB4D_Lower:
							ok = (cv->CompareToSameKindWithOptions(rt->ch, rt->fOptions) == CR_SMALLER);
							break;

						case DB4D_LowerOrEqual_Like:
						case DB4D_LowerOrEqual:
							ok = (cv->CompareToSameKindWithOptions(rt->ch, rt->fOptions) != CR_BIGGER);
							break;

						case DB4D_Regex_Match:
							if (rt->fRegMatcher != nil)
							{
								VString s;
								cv->GetString(s);
								ok = rt->fRegMatcher->Find(s, 1, false, &err);
							}
							else
								ok = 2;
							break;

						case DB4D_Regex_Not_Match:
							if (rt->fRegMatcher != nil)
							{
								VString s;
								cv->GetString(s);
								ok = !rt->fRegMatcher->Find(s, 1, false, &err);
							}
							else
								ok = 2;
							break;

						case DB4D_Contains_KeyWord:	
						case DB4D_Contains_KeyWord_Like:
						case DB4D_Contains_KeyWord_BeginingWith:
							vk = cv->GetValueKind();
							if (vk == VK_STRING || vk == VK_TEXT || vk == VK_STRING_UTF8 || vk == VK_TEXT_UTF8) 
							{
								ValueKind vk2 = rt->ch->GetValueKind();
								if (vk2 == VK_STRING || vk2 == VK_TEXT || vk2 == VK_STRING_UTF8 || vk2 == VK_TEXT_UTF8)
								{
									VError err2;
									ok = VDBMgr::ContainsKeyword( *(VString*)(cv), *(VString*)(rt->ch), rt->fOptions, err2);
								}
							}
							break;
						}
					}
				}
				else if (rt->fCheckForNull)
				{
					if (val->GetKind() == eav_subentity)
					{
						if (rt->comparaison == DB4D_Equal || rt->comparaison == DB4D_Like)
						{
							ok = val->getRelatedEntity() == nil;
						}
						else
							ok = val->getRelatedEntity() != nil;
					}
					else if (val->GetKind() == eav_selOfSubentity)
					{
						if (rt->comparaison == DB4D_Equal || rt->comparaison == DB4D_Like)
						{
							ok = val->getRelatedSelection() == nil || val->getRelatedSelection()->GetLength(context) == 0;
						}
						else
							ok = !(val->getRelatedSelection() == nil || val->getRelatedSelection()->GetLength(context) == 0);
					}
					if (ok == 2)
						ok = 3;
				}
			}


			{
				if (ok == 2 && rt->fCheckForNull)
				{
					ok = (rt->comparaison == DB4D_Like) || (rt->comparaison == DB4D_Equal);
				}
			}

			erec->Release();
		}
	}

	if (ok == 3)
		ok = 2;
	return ok;
}


VValueSingle* RechNodeEmSeq::Compute(LocalEntityRecord* erec, BaseTaskInfo* context, VError& err)
{
	Boolean ok = 2;
	ValueKind vk;
	VValueSingle* result = nil;
	err = VE_OK;
	EntityAttributeValue* val = erec->getAttributeValue(rt->fAttPath, err, context);
	if (val != nil)
	{
		if (val->getVValue() != nil)
		{
			VValueSingle* cv = val->getVValue();
			if (rt->ch != nil && !rt->ch->IsNull())
			{
				switch (rt->comparaison)
				{
					case DB4D_BeginsWith:
					case DB4D_Equal:
					case DB4D_Like:
						ok = (cv->CompareToSameKindWithOptions(rt->ch, rt->fOptions) == CR_EQUAL);
						break;

					case DB4D_NotEqual:
					case DB4D_NotLike:
						ok = (cv->CompareToSameKindWithOptions(rt->ch, rt->fOptions) != CR_EQUAL);
						break;

					case DB4D_Greater_Like:
					case DB4D_Greater:
						ok = (cv->CompareToSameKindWithOptions(rt->ch, rt->fOptions) == CR_BIGGER);
						break;

					case DB4D_GreaterOrEqual_Like:
					case DB4D_GreaterOrEqual:
						ok = (cv->CompareToSameKindWithOptions(rt->ch, rt->fOptions) != CR_SMALLER);
						break;

					case DB4D_Lower_Like:
					case DB4D_Lower:
						ok = (cv->CompareToSameKindWithOptions(rt->ch, rt->fOptions) == CR_SMALLER);
						break;

					case DB4D_LowerOrEqual_Like:
					case DB4D_LowerOrEqual:
						ok = (cv->CompareToSameKindWithOptions(rt->ch, rt->fOptions) != CR_BIGGER);
						break;

					case DB4D_Regex_Match:
						if (rt->fRegMatcher != nil)
						{
							VString s;
							cv->GetString(s);
							ok = rt->fRegMatcher->Find(s, 1, false, &err);
						}
						else
							ok = 2;
						break;

					case DB4D_Regex_Not_Match:
						if (rt->fRegMatcher != nil)
						{
							VString s;
							cv->GetString(s);
							ok = !rt->fRegMatcher->Find(s, 1, false, &err);
						}
						else
							ok = 2;
						break;

					case DB4D_Contains_KeyWord:	
					case DB4D_Contains_KeyWord_Like:
					case DB4D_Contains_KeyWord_BeginingWith:
						vk = cv->GetValueKind();
						if (vk == VK_STRING || vk == VK_TEXT || vk == VK_STRING_UTF8 || vk == VK_TEXT_UTF8) 
						{
							ValueKind vk2 = rt->ch->GetValueKind();
							if (vk2 == VK_STRING || vk2 == VK_TEXT || vk2 == VK_STRING_UTF8 || vk2 == VK_TEXT_UTF8)
							{
								VError err2;
								ok = VDBMgr::ContainsKeyword( *(VString*)(cv), *(VString*)(rt->ch), rt->fOptions, err2);
							}
						}
						break;
				}
				if (ok != 2)
					result = new VBoolean(ok);
			}
		}
		else if (rt->fCheckForNull)
		{
			if (val->GetKind() == eav_subentity)
			{
				if (rt->comparaison == DB4D_Equal || rt->comparaison == DB4D_Like)
				{
					ok = val->getRelatedEntity() == nil;
				}
				else
					ok = val->getRelatedEntity() != nil;
			}
			else if (val->GetKind() == eav_selOfSubentity)
			{
				if (rt->comparaison == DB4D_Equal || rt->comparaison == DB4D_Like)
				{
					ok = val->getRelatedSelection() == nil || val->getRelatedSelection()->GetLength(context) == 0;
				}
				else
					ok = !(val->getRelatedSelection() == nil || val->getRelatedSelection()->GetLength(context) == 0);
			}
			if (ok != 2)
				result = new VBoolean(ok);
			else
				ok = 3;
		}
	}
	

	{
		if (ok == 2 && rt->fCheckForNull)
		{
			ok = (rt->comparaison == DB4D_Like) || (rt->comparaison == DB4D_Equal);
			result = new VBoolean(ok);
		}
	}

	return result;
}



void RechNodeEmSeq::BuildQueryPathSeq(Boolean inBefore, sLONG& curvar, CDB4DQueryPathNode* parent, Base4D* bd, VString& outString, const CDB4DQueryPathModifiers* inModifiers, Boolean first) 
{
	/*
	VString sfield, soper, svalue;

	if (first)
		BuildAddFormula(outString, inModifiers);

	Field *f = bd->RetainField(rt->numfile, rt->numfield);

	if (f != nil)
	{
		f->GetName(sfield);
		if (rt->numinstance != 0)
			sfield += L"("+ToString(rt->numinstance)+L")";
		f->Release();
	}

	GetOperString(rt->comparaison, soper);

	if (rt->ch != nil)
	{
		if (rt->ch->IsNull())
			svalue = L"null";
		else
			svalue.FromValue(*(rt->ch));
	}
	else
		svalue = L"null";

	outString += sfield;
	outString += soper;
	outString += svalue;
	*/
}



CDB4DQueryPathNode* RechNodeEmSeq::BuildQueryPath(Boolean inBefore, Table* target, Base4D* bd, sLONG DejaSel, CDB4DQueryPathNode* parent, sLONG& curvar, sLONG &outvarnum, const CDB4DQueryPathModifiers* inModifiers)
{
	CDB4DQueryPathNode* result = nil;
	/*
	VString smess, smess2;

	curvar++;
	outvarnum = curvar;
	if (inModifiers->isVerbose())
	{
		BuildSeqString(bd, CalcTableNum(fTargetTable, target), DejaSel, outvarnum, smess, inModifiers, GetInstance());
		result = new VDB4DQueryPathNode(parent, smess, inModifiers, true);
		BuildQueryPathSeq(true, curvar, nil, bd, smess2, inModifiers, true);
		CDB4DQueryPathNode* subline = new VDB4DQueryPathNode(result, smess2, inModifiers, false);
	}
	else
	{
		BuildQueryPathSeq(false, curvar, nil, bd, smess2, inModifiers, false);
		BuildSeqStringNonVerbose(bd, CalcTableNum(fTargetTable, target), DejaSel, outvarnum, smess2, smess, inModifiers, GetInstance());
		result = new VDB4DQueryPathNode(parent, smess, inModifiers, inBefore);
	}
	*/
	return result;
}


void RechNodeEmSeq::Describe(Base4D* bd, VString& result)
{
	
	VString s, s2, s3;
	rt->fAttPath.GetString(s2);

	s.AppendString(s2);

	if (rt->fCheckForNull)
	{
		if (rt->comparaison == DB4D_NotEqual || rt->comparaison == DB4D_NotLike)
			s2 = " is not null";
		else
			s2 = " is null";
	}
	else
	{
		GetOperString(rt->comparaison, s2);

		s.AppendString(s2);

		if (rt->ch != nil && !rt->fCheckForNull)
			s2.FromValue(*(rt->ch));
		else
			s2 = L"null";
	}

	s.AppendString(s2);
	result = s;
	
}



void RechNodeEmSeq::DisplayTree(Base4D* bd, sLONG level, VString* result, VValueBag* bagResult)
{
	VString s, s2, s3;
	FillLevelString(level, s);
	Describe(bd, s2);
	if (bagResult != nil)
		bagResult->SetString(d4::item, s2);
	s += s2;
	AddCR(s);

	if (result == nil)
		DebugMsg(s);
	else
		*result += s;

}


void RechNodeEmSeq::CalcTarget(sLONG nf)
{
	sLONG target = 0;
	if (rt->fAttPath.Count() > 0)
	{
		const EntityAttributeInstance* attinst = rt->fAttPath.FirstPart();
		target = ((db4dEntityAttribute*)(attinst->fAtt))->GetTable()->GetNum();
	}
	if (target != nf || rt->numinstance != 0)
	{
		SetTarget(target, rt->numinstance);
	}
}





										/* =================================== */




RechNodeWithArraySeq::RechNodeWithArraySeq(void)
{
	TypNode=NoeudWithArraySeq;
	rt = nil;
	values = nil;
	rechtyp = -1;
}


RechNodeWithArraySeq::~RechNodeWithArraySeq(void)
{
	if (values != nil)
		values->Release();
}



uBOOL RechNodeWithArraySeq::PerformSeq(sLONG nf, FicheInMem* curfic, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, 
									   BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, LocalEntityModel* model)
{
	uBOOL ok = false;
	ValPtr cv;
	VError err;

	cv=nil;
	assert((rt->numfile==0) || (rt->numfile==nf));
	cv=curfic->GetNthField(rt->numfield, err, false, true);

	if (cv!=nil)
	{
		if (cv->IsNull())
			ok = 2;
		else
			ok = values->Find(*cv, rt->fOptions);
	} // du if cv!=nil
	else
		ok = 2;

	return(ok);

}



uBOOL RechNodeWithArraySeq::PerformSeq(FicheOnDisk* ficD, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, 
									   DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, LocalEntityModel* model)
{
	uBOOL ok = false;
	sLONG typondisk;
	void* data = (void*)(ficD->GetDataPtr(rt->numfield, &typondisk));
	uLONG tempdata[4];

	if (data == nil)
	{
		if (rt->fIsNeverNull == 2)
		{
			Field* cri = nil;
			VRefPtr<Table> assoctable(ficD->GetOwner()->RetainTable(), false);
			if (assoctable.Get() != nil)
				cri = assoctable->RetainField(rt->numfield);
			if (cri != nil)
			{
				rt->fIsNeverNull = cri->IsNeverNull();
				cri->Release();
			}
			else
				rt->fIsNeverNull = false;
		}
		if (rt->fIsNeverNull)
		{
			data = &tempdata;
			typondisk = rechtyp;
			std::fill(&tempdata[0], &tempdata[4], 0);
		}
	}

	if (data!=nil)
	{
		if (rechtyp == -1)
		{
			rechtyp = -2;

			Field* cri = nil;
			VRefPtr<Table> assoctable(ficD->GetOwner()->RetainTable(), false);
			if (assoctable.Get() != nil)
				cri = assoctable->RetainField(rt->numfield);
			if (cri != nil)
			{
				rechtyp = cri->GetTyp();
				cri->Release();
			}
		}

		if (typondisk == rechtyp && rt->fIsDataDirect)
		{
			ok = values->FindWithDataPtr(data, rt->fOptions);
		}
		else
		{
			ValPtr cv = NewValPtr(rechtyp, data, typondisk, ficD->GetOwner(), context);
			if (cv != nil)
			{
				ok = values->Find(*cv, rt->fOptions);
				delete cv;
			}
		}
	} 
	else
		ok = 2;

	return(ok);

}


void RechNodeWithArraySeq::BuildQueryPathSeq(Boolean inBefore, sLONG& curvar, CDB4DQueryPathNode* parent, Base4D* bd, VString& outString, const CDB4DQueryPathModifiers* inModifiers, Boolean first) 
{
	VString sfield, soper, svalue;

	if (first)
		BuildAddFormula(outString, inModifiers);

	Field *f = bd->RetainField(rt->numfile, rt->numfield);

	if (f != nil)
	{
		f->GetName(sfield);
		if (rt->numinstance != 0)
			sfield += L"("+ToString(rt->numinstance)+L")";
		f->Release();
	}

	switch (rt->comparaison)
	{
	case DB4D_BeginsWith:
		soper = L" Begin with one of the values of ";
		break;

	case DB4D_Equal:
		soper = L" in ";
		break;

	case DB4D_Like:
		soper = L" LIKE in ";
		break;

	case DB4D_NotEqual:
		soper = L" not in ";
		break;

	case DB4D_NotLike:
		soper = L" not LIKE in ";
		break;

	case DB4D_Contains_KeyWord:
		soper = L" contains one the the values of ";
		break;

	case DB4D_Contains_KeyWord_Like:
		soper = L" contains like one the the values of ";
		break;

	default:
		soper = L" <Nop> ";
		break;
	}

	sLONG nb,i,nbval = rt->values->Count();
	if ( nbval == 0)
	{
		svalue = L"<Empty Array>";
	}
	else
	{
		svalue = L"[ ";
		/*
		if (nbval > maxvaluetodisplay)
		{
			nb = maxvaluetodisplay;
		}
		else
		{
			nb = nbval;
		}
		for (i=0; i<nb; i++)
		{
			VString ssubvalue;
			ValPtr cv = (*rt->values)[i];
			if (cv == nil || cv->IsNull())
				ssubvalue = L"null";
			else
				ssubvalue.FromValue(*cv);
			svalue += ssubvalue;
			if (i<nbval)
				svalue += L" ; ";
		}

		if (nbval>maxvaluetodisplay)
			svalue += L"...";
		*/
		/*
		VString ssubvalue;
		const VValueSingle* cv = rt->values->GetFirst();
		if (cv == nil || cv->IsNull())
			ssubvalue = L"null";
		else
			ssubvalue.FromValue(*cv);
		svalue += ssubvalue;
		svalue += L"...";

		cv = rt->values->GetLast();
		if (cv == nil || cv->IsNull())
			ssubvalue = L"null";
		else
			ssubvalue.FromValue(*cv);
		svalue += ssubvalue;
		*/

		svalue += L" ]";
	}

	outString += sfield;
	outString += soper;
	outString += svalue;

}


CDB4DQueryPathNode* RechNodeWithArraySeq::BuildQueryPath(Boolean inBefore, Table* target, Base4D* bd, sLONG DejaSel, CDB4DQueryPathNode* parent, sLONG& curvar, sLONG &outvarnum, const CDB4DQueryPathModifiers* inModifiers)
{
	CDB4DQueryPathNode* result = nil;
	VString smess, smess2;

	curvar++;
	outvarnum = curvar;
	if (inModifiers->isVerbose())
	{
		BuildSeqString(bd, CalcTableNum(fTargetTable, target), DejaSel, outvarnum, smess, inModifiers, GetInstance());
		result = new VDB4DQueryPathNode(parent, smess, inModifiers, true);
		BuildQueryPathSeq(false, curvar, nil, bd, smess2, inModifiers);
		CDB4DQueryPathNode* subline = new VDB4DQueryPathNode(result, smess2, inModifiers, false);
	}
	else
	{
		BuildQueryPathSeq(false, curvar, nil, bd, smess2, inModifiers, false);
		BuildSeqStringNonVerbose(bd, CalcTableNum(fTargetTable, target), DejaSel, outvarnum, smess2, smess, inModifiers, GetInstance());
		result = new VDB4DQueryPathNode(parent, smess, inModifiers, inBefore);
	}
	return result;
}



void RechNodeWithArraySeq::Describe(Base4D* bd, VString& result)
{
	VString s, s2, s3;

	Table* t = bd->RetainTable(rt->numfile);
	Field* f = nil;
	s2 = L"<Null field>";
	if (t != nil)
	{
		f = t->RetainField(rt->numfield);
		if (f != nil)
		{
			t->GetName(s2);
			if (rt->numinstance != 0)
				s2 += L"("+ToString(rt->numinstance)+L")";
			s2.AppendString(L".");
			f->GetName(s3);
			s2.AppendString(s3);
			f->Release();
		}
		t->Release();
	}
	s.AppendString(s2);

	switch (rt->comparaison)
	{
	case DB4D_BeginsWith:
		s2 = L" Begin with ";
		break;

	case DB4D_Equal:
		s2 = L" === ";
		break;

	case DB4D_Like:
		s2 = L" == ";
		break;

	case DB4D_NotEqual:
		s2 = L" !== ";
		break;

	case DB4D_NotLike:
		s2 = L" != ";
		break;

	case DB4D_Greater:
	case DB4D_Greater_Like:
		s2 = L" > ";
		break;

	case DB4D_GreaterOrEqual:
	case DB4D_GreaterOrEqual_Like:
		s2 = L" >= ";
		break;

	case DB4D_Lower:
	case DB4D_Lower_Like:
		s2 = L" < ";
		break;

	case DB4D_LowerOrEqual:
	case DB4D_LowerOrEqual_Like:
		s2 = L" <= ";
		break;

	case DB4D_Contains_KeyWord:
		s2 = L" contains ";
		break;

	case DB4D_Contains_KeyWord_Like:
		s2 = L" contains like ";
		break;

	default:
		s2 = L" <Nop> ";
		break;
	}

	s.AppendString(s2);

	s2 = L"<Array Of Values>";

	s.AppendString(s2);

	result = s;
}


void RechNodeWithArraySeq::DisplayTree(Base4D* bd, sLONG level, VString* result, VValueBag* bagResult)
{
	VString s, s2, s3;
	FillLevelString(level, s);
	Describe(bd,s2);
	if (bagResult != nil)
		bagResult->SetString(d4::item, s2);
	s += s2;

	AddCR(s);

	if (result == nil)
		DebugMsg(s);
	else
		*result += s;

}

												/* =================================== */


RechNodeIndex::RechNodeIndex(void)
{
	TypNode=NoeudIndexe;
	cle1=nil;
	cle2=nil;
	xstrict1=false;
	xstrict2=false;
	inverse=false;
	isfourche=false;
	//fIsBeginWith = false;
	//fIsLike = false;
	ind = nil;
	rt = nil;
	rt2 = nil;
	fIncludingFork = nil;
	fParseAllIndex = false;
	//fIsDiacritic = false;
}


RechNodeIndex::~RechNodeIndex()
{
	if (cle1 != cle2)
	{
		if (cle2 != nil) ind->FreeKey(cle2);
	}
	if (cle1 != nil) ind->FreeKey(cle1);

	if (ind != nil)
	{
		ind->ReleaseValid();
		ind->Release();
	}

	if (fIncludingFork != nil)
		delete fIncludingFork;

	for (ArrayNode::Iterator cur = Nodes.First(), end = Nodes.End(); cur != end; cur++)
	{
		if (testAssert(*cur != nil))
		{
			delete (*cur);
		}
	}
}


VError RechNodeIndex::SubBuildCompFrom(sLONG comp, BTitemIndex *val)
{
	switch (comp)
	{
		case DB4D_Equal:
		case DB4D_NotEqual:
		case DB4D_Like:
		case DB4D_NotLike:
		case DB4D_Contains_KeyWord:
		case DB4D_DoesntContain_KeyWord:
		case DB4D_Contains_KeyWord_Like:
		case DB4D_Doesnt_Contain_KeyWord_Like:
			cle1=val;
			cle2=val;
			xstrict1=false;
			xstrict2=false;
			inverse=(comp==DB4D_NotEqual || comp==DB4D_DoesntContain_KeyWord 
				|| comp == DB4D_NotLike || comp == DB4D_Doesnt_Contain_KeyWord_Like);

			fOptions.SetLike(comp == DB4D_NotLike || comp == DB4D_Like 
				|| comp == DB4D_Contains_KeyWord_Like || comp == DB4D_Doesnt_Contain_KeyWord_Like);
			break;

		case DB4D_Greater:
		case DB4D_GreaterOrEqual:
		case DB4D_Greater_Like:
		case DB4D_GreaterOrEqual_Like:
			cle1=val;
			xstrict1=(comp==DB4D_Greater || comp==DB4D_Greater_Like);
			//cle2=nil;
			//xstrict2=false;
			inverse=false;
			fOptions.SetLike(comp == DB4D_Greater_Like || comp == DB4D_GreaterOrEqual_Like);
			break;

		case DB4D_Lower:
		case DB4D_LowerOrEqual:
		case DB4D_Lower_Like:
		case DB4D_LowerOrEqual_Like:
			//cle1=nil;
			//xstrict1=false;
			cle2=val;
			xstrict2=(comp==DB4D_Lower || comp==DB4D_Lower_Like);
			inverse=false;
			fOptions.SetLike(comp == DB4D_LowerOrEqual_Like || comp == DB4D_Lower_Like);
			break;

		case DB4D_Contains_KeyWord_BeginingWith:
		case DB4D_BeginsWith:
			cle1=val;
			cle2=val;
			xstrict1=false;
			xstrict2=false;
			fOptions.SetBeginsWith(true);
			break;
		default:
			xbox_assert(false);	// check for mem. leaks
			break;
	}

	return VE_OK;
}


VError RechNodeIndex::BuildFrom(RechTokenSimpleComp *xrt)
{
	VError err = VE_OK;
	BTitemIndex *val;
	
	//fIsBeginWith = false;
	//fIsLike = false;
	rt = xrt;
	rt2 = nil;
	cle1 = nil;
	cle2 = nil;
	xstrict1 = false;
	xstrict2 = false;
	//fIsDiacritic = xrt->fIsDiacritic;
	fOptions = xrt->fOptions;
	fOptions.SetBeginsWith(false);
	fOptions.SetLike(false);

	val=((IndexInfoFromField*)ind)->BuildKeyFromVValue(rt->ch, err);
	SubBuildCompFrom(rt->comparaison, val);
	return(err);
}


VError RechNodeIndex::BuildFromMultiple(RechNode *IncludingFork, BaseTaskInfo* context)
{
	VError err = VE_OK;
	BTitemIndex *val = nil, *val2 = nil;

	//fIsBeginWith = false;
	//fIsLike = false;
	rt = nil;
	rt2 = nil;
	cle1 = nil;
	cle2 = nil;
	xstrict1 = false;
	xstrict2 = false;

	assert(ind->MatchType(DB4D_Index_OnMultipleFields));
	IndexInfoFromMultipleField* indm = (IndexInfoFromMultipleField*)ind;

	Vx0ArrayOf<VValueSingle*, 10> values;
	sLONG ival = 0;
	sLONG ilast = Nodes.GetCount()-1;
	fParseAllIndex = false;
	for (ArrayNode::Iterator cur = Nodes.First(), end = Nodes.End(); cur != end; cur++,ival++)
	{
		VValueSingle* val = ((RechNodeSeq*)*cur)->rt->ch;
		if (val->GetValueKind() == VK_STRING)
		{
			VString* s = (VString*)val;
			sLONG p = s->FindUniChar(GetWildChar(context));
			bool compatible = GetContextIntl(context)->GetCollator()->IsPatternCompatibleWithDichotomyAndDiacritics( s->GetCPointer(), s->GetLength());

			if (ival != ilast)
			{
				if (!(p == 0 || p == -1))				
					fParseAllIndex = true;
				if (!compatible)
					fParseAllIndex = true;
			}
			else
			{
				if (!(p == 0 || p == -1 || (p == s->GetLength() && compatible) ))
					fParseAllIndex = true;
			}
		}

		values.Add(val);
		//fIsDiacritic = ((RechNodeSeq*)*cur)->rt->fIsDiacritic;
		fOptions = ((RechNodeSeq*)*cur)->rt->fOptions;
	}

	fOptions.SetBeginsWith(false);
	fOptions.SetLike(false);

	val = indm->BuildKeyFromVValues(values, err);
	if (err == VE_OK)
	{
		fIncludingFork = IncludingFork;

		if (IncludingFork != nil)
		{
			values.SetCount(values.GetCount()-1);
			values.Add(((RechNodeSeq*)IncludingFork)->rt->ch);
			val2 = indm->BuildKeyFromVValues(values, err);
		}

		if (err == VE_OK)
		{
			SubBuildCompFrom(((RechNodeSeq*)Nodes.GetLast())->rt->comparaison, val);
			if (val2 != nil)
			{
				isfourche = true;
				SubBuildCompFrom(((RechNodeSeq*)IncludingFork)->rt->comparaison, val2);
			}
		}
	}

	if (err != VE_OK)
	{
		if (val != nil)
			ind->FreeKey(val);
		if (val2 != nil)
			ind->FreeKey(val2);
	}

	return err;
}


VError RechNodeIndex::Perform(OptimizedQuery *query, Bittab **super, Bittab* filtre, VDB4DProgressIndicator* InProgress,  VValueBag* curdesc,
							  BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, Bittab &Nulls, RechNode* &transformation)
{
	Bittab *b = nil;
	VError err = VE_OK;
	bool checknulls = false;

	transformation = this;

	if (rt != nil && rt->fCheckForNull)
	{
		checknulls = true;
	}

	if ((ind->GetNBDiskAccessForAScan(false) /5) > query->CalculateNBDiskAccessForSeq(filtre, context, ind->SourceIsABlob() ? 1 : 0))
	{
		transformation = query->xTransformSeq(this, true);
		if (transformation != this)
		{
			if (transformation != nil)
			{
				transformation->AdjusteIntl(GetContextIntl(context));
				err = transformation->Perform(query, super, filtre, InProgress, curdesc, context, HowToLock, exceptions, limit, Nulls, transformation);
			}
			return err;
		}
	}

	{
		//## pour tester performance
		//InProgress = nil;
		uLONG start = query->GetStartingTime();

		ind->Open(index_read);
		OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
		if (checknulls)
		{
			BitSel* sel = ind->GetHeader()->RetainNulls(curstack, err, context);
			if (err == VE_OK)
			{
				if (sel == nil)
					b = new Bittab();
				else
				{
					b = sel->GetBittab()->Clone(err);
				}
			}
			QuickReleaseRefCountable(sel);
		}
		else if (fParseAllIndex)
		{
			b = ind->ParseAll(cle1, context, err, fOptions, InProgress);
		}
		else
		{
			b = ind->Fourche(cle1,xstrict1,cle2,xstrict2, context, err, InProgress, fOptions, nil, nil);
		}

		if (b!=nil && err == VE_OK)
		{
			if (inverse)
			{
				ind->Close();
				b->aggrandit(query->GetMaxRecords());
				err=b->Invert();
				query->GetTargetFile()->GetDF()->TakeOffBits(b, context);
				if (err == VE_OK)
					err = query->MaximizeBittab(b);
				if (err == VE_OK && !checknulls)
				{
					ind->Open(index_read);
					BitSel* sel = ind->GetHeader()->RetainNulls(curstack, err, context);
					if (sel != nil)
					{
						err = b->moins(sel->GetBittab(), false);
						if (err == VE_OK)
							err = Nulls.Or(sel->GetBittab(), true);
						sel->Release();
					}
					ind->Close();
				}
			}
			else
			{
				if (!checknulls)
				{
					BitSel* sel = ind->GetHeader()->RetainNulls(curstack, err, context);
					if (sel != nil)
					{
						err = Nulls.Or(sel->GetBittab(), true);
						sel->Release();
					}
				}
				ind->Close();
			}
			
		}
		else
			ind->Close();

		
		if (err == VE_OK)
		{
			if (filtre != nil) 
				err = b->And(filtre);
		}
		
		if (HowToLock == DB4D_Keep_Lock_With_Transaction && err == VE_OK)
		{
			if (context != nil)
			{
				Transaction* trans = context->GetCurrentTransaction();
				if (trans != nil)
				{
					err = trans->TryToLockSel(query->GetTargetFile()->GetDF(), b, exceptions);
				}
			}
		}

		if (query->IsQueryDescribed())
		{
			VString s;
			FullyDescribe(query->GetTargetFile()->GetOwner(), s);
			query->AddToDescription(curdesc, s, start, b);
		}

		if (err != VE_OK)
		{
			ReleaseRefCountable(&b);
		}

		*super = b;
	}

	return(err);
}



uBOOL RechNodeIndex::PeutFourche(void)
{
	if ( (cle1==nil) || (cle2==nil) )
	{
		return(true);
	}
	else
	{
		return(false);
	}
}


uBOOL RechNodeIndex::PeutFourche(RechNodeIndex* autre)
{
	uBOOL ok;
	
	ok=false;
	if (ind->MayBeSorted() && ind==autre->ind)
	{
		if (   ((cle1==nil) && (autre->cle1!=nil) && (autre->cle2==nil))
			  || ((cle2==nil) && (autre->cle2!=nil) && (autre->cle1==nil))  )
		{
			ok=true;
			rt2=autre->rt;
			isfourche=true;
			if (cle1==nil)
			{
				cle1=autre->cle1;
				autre->cle1=nil;
				xstrict1=autre->xstrict1;
			}
			else
			{
				cle2=autre->cle2;
				autre->cle2=nil;
				xstrict2=autre->xstrict2;
			}
		}
	}
	
	return(ok);
}


sLONG8 RechNodeIndex::CountDiskAccess()
{
	if (ind == nil)
		return 0;
	else
		return ind->GetNBDiskAccessForAScan(false);
}


sLONG RechNodeIndex::CountBlobSources()
{
	if (ind == nil)
		return 0;
	else
	{
		if (ind->SourceIsABlob())
			return 1;
		else
			return 0;
	}
}


CDB4DQueryPathNode* RechNodeIndex::BuildQueryPath(Boolean inBefore, Table* target, Base4D* bd, sLONG DejaSel, CDB4DQueryPathNode* parent, sLONG& curvar, sLONG &outvarnum, const CDB4DQueryPathModifiers* inModifiers)
{
	CDB4DQueryPathNode* result = nil;
	Boolean twokeys = false;
	VString smess, s, smess2;

	curvar++;
	outvarnum = curvar;

	if (inModifiers->isVerbose())
		BuildBeginIndexString(ind, outvarnum, smess, inModifiers);
	else
		BuildBeginIndexStringNonVerbose(ind, outvarnum, smess, inModifiers);

	if (cle1 == nil)
	{
		if (xstrict2)
			smess += L" < ";
		else
			smess += L" <= ";
	}
	else
	{
		if (cle2 == nil)
		{
			if (xstrict1)
				smess += L" > ";
			else
				smess += L" >= ";
		}
		else
		{
			if (cle1 != cle2)
			{
				smess += L" between ";
				twokeys = true;
			}
			else
			{
				if (fOptions.IsBeginsWith())
					smess += " begining with ";
				else
					smess += " = ";
			}
		}
	}

	if (cle1 != nil)
	{
		cle1->GetDebugString(ind, s);
		smess += s;
	}
	if (twokeys)
		smess += L" and ";

	if (cle2 != nil && cle2 != cle1)
	{
		cle2->GetDebugString(ind, s);
		smess += s;
	}

	if (inModifiers->isVerbose())
		BuildEndIndexString(ind, outvarnum, inverse, smess2, inModifiers);
	else
		BuildEndIndexStringNonVerbose(ind, outvarnum, inverse, smess2, inModifiers);

	smess += smess2;

	result = new VDB4DQueryPathNode(parent, smess, inModifiers, inBefore);
	return result;
}


void RechNodeIndex::Describe(Base4D* bd, VString& result)
{
	VString s, s2, s3;

	s.AppendString(L"[index : ");


	if (ind != nil)
	{
		ind->GetDebugString(s2, rt == nil ? 0 : rt->numinstance);
	}
	else
		s2 = L"<Null Index>";

	s.AppendString(s2);
	s.AppendString(L" ]");

	if (rt != nil && rt->fCheckForNull)
	{
		if (inverse)
			s+=L" is not null";
		else
			s+=L" is null";
	}
	else
	{

		if (cle1 == cle2)
		{
			if (fOptions.IsBeginsWith())
				s2 = L" Begin with ";
			else
			{
				if (inverse)
					s2 = L" # ";
				else
				{
					if (fOptions.IsLike())
						s2 = L" LIKE ";
					else
						s2 = L" = ";
				}
			}
		}
		else
		{
			if (cle1 == nil)
			{
				if (cle2 != nil)
				{
					if (xstrict2)
						s2 = L" < ";
					else
						s2 = L" <= ";
				}
				else
				{
					s2 = L" <Nop> ";
				}
			}
			else
			{
				if (cle2 == nil)
				{
					if (xstrict1)
						s2 = L" > ";
					else
						s2 = L" >= ";
				}
				else
				{
					s2 = L" Between ";
				}
			}
		}

		s.AppendString(s2);

		if (cle1 != nil)
		{
			cle1->GetDebugString(ind, s2);
			s.AppendString(s2);
			if (cle2 != nil && cle2 != cle1)
			{
				s.AppendString(L" -And- ");
				cle2->GetDebugString(ind, s2);
				s.AppendString(s2);
			}
		}
		else
		{
			if (cle2 != nil)
			{
				cle2->GetDebugString(ind, s2);
				s.AppendString(s2);
			}
			else
				s.AppendString(L"null");
		}
	}

	result = s;
}


void RechNodeIndex::DisplayTree(Base4D* bd, sLONG level, VString* result, VValueBag* bagResult)
{
	VString s, s2, s3;
	FillLevelString(level, s);

	Describe(bd, s2);
	if (bagResult != nil)
		bagResult->SetString(d4::item, s2);
	s += s2;

	AddCR(s);

	if (result == nil)
		DebugMsg(s);
	else
		*result += s;
}



/* =================================== */



VError RechNodeConst::Perform(OptimizedQuery *query, Bittab **super, Bittab* filtre, VDB4DProgressIndicator* InProgress,  VValueBag* curdesc,
							  BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, Bittab &Nulls, RechNode* &transformation)
{
	assert(false); // on ne devrait jamais arriver la
	Bittab *b = nil;
	VError err = VE_OK;

	transformation = this;
	*super = b;

	return(err);
}


CDB4DQueryPathNode* RechNodeConst::BuildQueryPath(Boolean inBefore, Table* target, Base4D* bd, sLONG DejaSel, CDB4DQueryPathNode* parent, sLONG& curvar, sLONG &outvarnum, const CDB4DQueryPathModifiers* inModifiers)
{
	return nil;
}


void RechNodeConst::Describe(Base4D* bd, VString& result)
{
	result = L"Constant Value: ";
	if (fResult == 2)
		result += L"<Always NULL>";
	else
	{
		if (fResult)
			result += L"<Always True>";
		else
			result += L"<Always False>";
	}
}


VValueSingle* RechNodeConst::Compute(LocalEntityRecord* erec, BaseTaskInfo* context, VError& err)
{
	VValueSingle* result = nil;

	if (fResult != 2)
		result = new VBoolean(fResult);

	return result;
}



void RechNodeConst::DisplayTree(Base4D* bd, sLONG level, VString* result, VValueBag* bagResult)
{
	VString s, s2, s3;
	FillLevelString(level, s);
	Describe(bd, s2);
	if (bagResult != nil)
		bagResult->SetString(d4::item, s2);
	s += s;

	AddCR(s);

	if (result == nil)
		DebugMsg(s);
	else
		*result += s;
}



/* =================================== */



CDB4DQueryPathNode* RechNodeRecordExist::BuildQueryPath(Boolean inBefore, Table* target, Base4D* bd, sLONG DejaSel, CDB4DQueryPathNode* parent, sLONG& curvar, sLONG &outvarnum, const CDB4DQueryPathModifiers* inModifiers)
{
	return nil;
}


VValueSingle* RechNodeRecordExist::Compute(LocalEntityRecord* erec, BaseTaskInfo* context, VError& err)
{
	// a faire
	assert(false);
	return nil;
}


void RechNodeRecordExist::Describe(Base4D* bd, VString& result)
{
	Table* t = bd->RetainTable(rt->numfile);

	result = L"Check if record of table ";

	VString tablename = L"<null table>";
	if (t != nil)
	{
		t->GetName(tablename);
		if (rt->numinstance != 0)
		{
			tablename += L"("+ToString(rt->numinstance)+L")";
		}
	}
	result += tablename;

	if (rt->fCheckIfExists)
		result += " exists.";
	else
		result += " does not exist.";

	QuickReleaseRefCountable(t);

}


void RechNodeRecordExist::DisplayTree(Base4D* bd, sLONG level, VString* result, VValueBag* bagResult)
{
	VString s, s2, s3;
	FillLevelString(level, s);
	Describe(bd, s2);
	if (bagResult != nil)
		bagResult->SetString(d4::item, s2);
	s += s2;

	AddCR(s);

	if (result == nil)
		DebugMsg(s);
	else
		*result += s;
}


/* =================================== */




RechNodeWithArrayIndex::RechNodeWithArrayIndex(void)
{
	TypNode=NoeudWithArrayIndex;
	inverse = false;
	ind = nil;
}

RechNodeWithArrayIndex::~RechNodeWithArrayIndex()
{
	if (values != nil)
		values->Release();
	if (ind != nil)
	{
		ind->ReleaseValid();
		ind->Release();
	}
}


VError RechNodeWithArrayIndex::BuildFrom(RechTokenArrayComp *xrt)
{
	VError err = VE_OK;

	inverse = false;
	fOptions = xrt->fOptions;
	fOptions.SetBeginsWith(false);
	fOptions.SetLike(false);

	values = xrt->values;
	if (values != nil)
		values->Retain();

	rt = xrt;

	switch (rt->comparaison)
	{
		case DB4D_Like:
			fOptions.SetLike(true);
			break;

		case DB4D_NotLike:
			fOptions.SetLike(true);
			inverse = true;
			break;

		case DB4D_NotEqual:
			inverse=true;
			break;

		case DB4D_BeginsWith:
			fOptions.SetBeginsWith(true);
			break;
	}
	return(err);
}



VError RechNodeWithArrayIndex::Perform(OptimizedQuery *query, Bittab **super, Bittab* filtre, VDB4DProgressIndicator* InProgress,  VValueBag* curdesc,
									   BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, Bittab &Nulls, RechNode* &transformation)
{
	Bittab *b = nil;
	VError err = VE_OK;

	transformation = this;

	if ((ind->GetNBDiskAccessForAScan(false)/5) > query->CalculateNBDiskAccessForSeq(filtre, context, ind->SourceIsABlob() ? 1 : 0))
	{
		transformation = query->xTransformSeq(this, true);
		if (transformation != this)
		{
			if (transformation != nil)
			{
				err = transformation->Perform(query, super, filtre, InProgress, curdesc, context, HowToLock, exceptions, limit, Nulls, transformation);
			}
			return err;
		}
	}

	{
		//## pour tester performance
		//InProgress = nil;
		uLONG start = query->GetStartingTime();

		ind->Open(index_read);
		OccupableStack* curstack = gOccPool.GetStackForCurrentThread();

		b=ind->FindKeyInArray(values, context, err, InProgress, fOptions, nil);
		if (b!=nil && err == VE_OK)
		{
			if (inverse)
			{
				ind->Close();
				b->aggrandit(query->GetMaxRecords());
				err=b->Invert();
				query->GetTargetFile()->GetDF()->TakeOffBits(b, context);
				if (err == VE_OK)
					err = query->MaximizeBittab(b);
				if (err == VE_OK)
				{
					ind->Open(index_read);
					BitSel* sel = ind->GetHeader()->RetainNulls(curstack, err, context);
					if (sel != nil)
					{
						err = b->moins(sel->GetBittab(), false);
						if (err == VE_OK)
							err = Nulls.Or(sel->GetBittab(), true);
						sel->Release();
					}
					ind->Close();
				}
			}
			else
			{
				BitSel* sel = ind->GetHeader()->RetainNulls(curstack, err, context);
				if (sel != nil)
				{
					if (err == VE_OK)
						err = Nulls.Or(sel->GetBittab(), true);
					sel->Release();
				}
				ind->Close();
			}

		}
		else
			ind->Close();


		if ( (filtre != nil) && (b != nil) )
			b->And(filtre);

		if ( (HowToLock == DB4D_Keep_Lock_With_Transaction) && (err == VE_OK) )
		{
			if (context != nil)
			{
				Transaction* trans = context->GetCurrentTransaction();
				if (trans != nil)
				{
					err = trans->TryToLockSel(query->GetTargetFile()->GetDF(), b, exceptions);
				}
			}
		}

		if (query->IsQueryDescribed())
		{
			VString s;
			FullyDescribe(query->GetTargetFile()->GetOwner(), s);
			query->AddToDescription(curdesc, s, start, b);
		}

		if (err != VE_OK)
		{
			ReleaseRefCountable(&b);
		}

		*super = b;
	}

	return(err);
}


sLONG8 RechNodeWithArrayIndex::CountDiskAccess()
{
	if (ind == nil)
		return 0;
	else
		return ind->GetNBDiskAccessForAScan(false);
}


sLONG RechNodeWithArrayIndex::CountBlobSources()
{
	if (ind == nil)
		return 0;
	else
	{
		if (ind->SourceIsABlob())
			return 1;
		else
			return 0;
	}
}


CDB4DQueryPathNode* RechNodeWithArrayIndex::BuildQueryPath(Boolean inBefore, Table* target, Base4D* bd, sLONG DejaSel, CDB4DQueryPathNode* parent, sLONG& curvar, sLONG &outvarnum, const CDB4DQueryPathModifiers* inModifiers)
{

	CDB4DQueryPathNode* result = nil;
	Boolean twokeys = false;
	VString smess,s,smess2;

	curvar++;
	outvarnum = curvar;
	if (inModifiers->isVerbose())
		BuildBeginIndexString(ind, outvarnum, smess, inModifiers);
	else
		BuildBeginIndexStringNonVerbose(ind, outvarnum, smess, inModifiers);


	if (fOptions.IsBeginsWith())
		smess += L" begining with one of those values : ";
	else
		smess += L" in ";

	VString svalue;

	sLONG nb,i,nbval = rt->values->Count();
	if ( nbval == 0)
	{
		svalue = L"<Empty Array>";
	}
	else
	{
		svalue = L"[ ";

		/*
		if (nbval > maxvaluetodisplay)
		{
			nb = maxvaluetodisplay;
		}
		else
		{
			nb = nbval;
		}
		for (i=0; i<nb; i++)
		{
			VString ssubvalue;
			ValPtr cv = (*rt->values)[i];
			if (cv == nil || cv->IsNull())
				ssubvalue = L"null";
			else
				ssubvalue.FromValue(*cv);
			svalue += ssubvalue;
			if (i<nbval)
				svalue += L" ; ";
		}

		if (nbval>maxvaluetodisplay)
			svalue += L"...";
		*/

		/*
		VString ssubvalue;
		const VValueSingle* cv = rt->values->GetFirst();
		if (cv == nil || cv->IsNull())
			ssubvalue = L"null";
		else
			ssubvalue.FromValue(*cv);
		svalue += ssubvalue;
		svalue += L"...";

		cv = rt->values->GetLast();
		if (cv == nil || cv->IsNull())
			ssubvalue = L"null";
		else
			ssubvalue.FromValue(*cv);
		svalue += ssubvalue;
		*/

		svalue += L" ]";
	}

	if (inModifiers->isVerbose())
		BuildEndIndexString(ind, outvarnum, inverse, smess2, inModifiers);
	else
		BuildEndIndexStringNonVerbose(ind, outvarnum, inverse, smess2, inModifiers);

	smess += smess2;

	result = new VDB4DQueryPathNode(parent, smess, inModifiers, inBefore);
	return result;
}


void RechNodeWithArrayIndex::Describe(Base4D* bd, VString& result)
{
	VString s, s2, s3;
	s.AppendString(L"[index : ");

	if (ind != nil)
	{
		ind->GetDebugString(s2, rt->numinstance);
	}
	else
		s2 = L"<Null Index>";

	s.AppendString(s2);
	s.AppendString(L" ]");

	if (fOptions.IsBeginsWith())
		s2 = L" Begin with ";
	else
	{
		if (inverse)
			s2 = L" # ";
		else
			s2 = L" = ";
	}

	s.AppendString(s2);

	s.AppendString(L"<Array of Values>");
	result = s;
}


void RechNodeWithArrayIndex::DisplayTree(Base4D* bd, sLONG level, VString* result, VValueBag* bagResult)
{
	VString s, s2, s3;
	FillLevelString(level, s);
	Describe(bd, s2);
	if (bagResult != nil)
		bagResult->SetString(d4::item, s2);
	s += s2;
	AddCR(s);


	if (result == nil)
		DebugMsg(s);
	else
		*result += s;
}


												/* =================================== */


RechNodeExistingSelection::~RechNodeExistingSelection()
{
	ReleaseRefCountable(&fConverted);
	fSel->Release();
}

VError RechNodeExistingSelection::Perform(OptimizedQuery *query, Bittab **super, Bittab* filtre, VDB4DProgressIndicator* InProgress,  VValueBag* curdesc,
										  BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, Bittab &Nulls, RechNode* &transformation)
{
	uLONG start = query->GetStartingTime();
	transformation = this;

	VError err = VE_OK;
	Bittab* b = fSel->GenereBittab(context, err);
	if (err == VE_OK)
	{
		if (b->GetRefCount() != 1)
		{
			Bittab* b2 = new Bittab;
			if (b2 == nil)
				err = ThrowBaseError(memfull, DBaction_ExecutingQuery);
			else
			{
				err = b2->Or(b);
				if (err != VE_OK)
				{
					b2->Release();
					b2 = nil;
				}
			}
			b->Release();
			b = b2;
		}

		if (b != nil)
		{
			if (filtre != nil)
			{
				err = b->And(filtre);
			}

			if (err != VE_OK)
			{
				b->Release();
				b = nil;
			}
			else
			{
				if (HowToLock == DB4D_Keep_Lock_With_Transaction)
				{
					if (context != nil)
					{
						Transaction* trans = context->GetCurrentTransaction();
						if (trans != nil)
						{
							err = trans->TryToLockSel(query->GetTargetFile()->GetDF(), b, exceptions);
							if (err != VE_OK)
							{
								b->Release();
								b = nil;
							}
						}
					}
				}
			}
		}
	}
	*super = b;

	if (query->IsQueryDescribed())
	{
		VString s;
		FullyDescribe(query->GetTargetFile()->GetOwner(), s);
		query->AddToDescription(curdesc, s, start, b);
	}

	return err;
}


uBOOL RechNodeExistingSelection::sub_PerformSeq(sLONG numfiche, BaseTaskInfo* context)
{
	VError err;
	if (fConverted == nil)
		fConverted = fSel->GenereBittab(context, err);
	
	if (fConverted != nil)
	{
		return fConverted->isOn(numfiche);
	}
	else
		return false;
}


uBOOL RechNodeExistingSelection::PerformSeq(sLONG nf, FicheInMem* curfic, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, 
											BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, LocalEntityModel* model)
{
	return sub_PerformSeq(curfic->GetNum(), context);
}


uBOOL RechNodeExistingSelection::PerformSeq(FicheOnDisk* ficD, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, 
											DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, LocalEntityModel* model)
{
	return sub_PerformSeq(ficD->getnumfic(), context);
}


void RechNodeExistingSelection::Describe(Base4D* bd, VString& result)
{
	VString s2;
	result = L"On Current Selection of ";
	fSel->GetParentFile()->GetTable()->GetName(s2);
	if (GetInstance() != 0)
		s2 += L"("+ToString(GetInstance())+L")";
	result += s2;
}


void RechNodeExistingSelection::DisplayTree(Base4D* bd, sLONG level, VString* result, VValueBag* bagResult)
{
	VString s, s2;
	FillLevelString(level, s);
	Describe(bd, s2);
	if (bagResult != nil)
		bagResult->SetString(d4::item, s2);
	s += s2;
	AddCR(s);
	if (result == nil)
		DebugMsg(s);
	else
		*result += s;
}


CDB4DQueryPathNode* RechNodeExistingSelection::BuildQueryPath(Boolean inBefore, Table* target, Base4D* bd, sLONG DejaSel, CDB4DQueryPathNode* parent, sLONG& curvar, sLONG &outvarnum, const CDB4DQueryPathModifiers* inModifiers)
{
	return nil;
}


void RechNodeExistingSelection::BuildQueryPathSeq(Boolean inBefore, sLONG& curvar, CDB4DQueryPathNode* parent, Base4D* bd, VString& outString, const CDB4DQueryPathModifiers* inModifiers, Boolean first)
{
}


												/* =================================== */



void RechNodeOperator::DisposeTree(void)
{
	if (left != nil) left->DisposeTree();
	delete left;
	left = nil;
	if (right != nil) right->DisposeTree();
	delete right;
	right = nil;
}


void RechNodeOperator::AdjusteIntl(VIntlMgr* inIntl)
{
	if (left != nil)
		left->AdjusteIntl(inIntl);
	if (right != nil)
		right->AdjusteIntl(inIntl);
}

bool RechNodeOperator::NeedToLoadFullRec()
{
	bool result = false;
	if (left != nil)
		result = result || left->NeedToLoadFullRec();
	if (right != nil)
		result = result || right->NeedToLoadFullRec();
	return result;
}


uBOOL RechNodeOperator::IsIndexe(void)
{
	uBOOL result = true;
	uBOOL result2 = true;

	if (recalcIndex)
	{
		if (left != nil) result = left->IsIndexe();
		if (right != nil) result2 = right->IsIndexe();

		if (OperBool == DB4D_OR)
		{
			result = result && result2;
		}
		else
		{
			result = result || result2;
		}

		recalcIndex = false;
		isindexe = result;
	}
	else
	{
		result = isindexe;
	}

	return result;
}


uBOOL RechNodeOperator::IsAllIndexe(void)
{
	uBOOL result = true;
	
	if (recalcAllIndex)
	{
		if (left != nil) result = left->IsAllIndexe();
		if (right != nil) result = result && right->IsAllIndexe();
		isallindexe = result;
		recalcAllIndex = false;
	}
	else
	{
		result = isallindexe;
	}
	
	return result;
}


uBOOL RechNodeOperator::IsAllJoin(void)
{
	uBOOL result = true;

	if (recalcAllJoin)
	{
		if (left != nil) result = left->IsAllJoin();
		if (right != nil) result = result && right->IsAllJoin();
		isalljoin = result;
		recalcAllJoin = false;
	}
	else
	{
		result = isalljoin;
	}

	return result;
}


void RechNodeOperator::SetLeft(RechNode *xleft) 
{ 
	if (left != xleft)
	{
		recalcIndex = true;
		recalcAllIndex = true;
		recalcAllJoin = true;
	}
	left = xleft;
	if (left != nil) left->SetParent(this);
};


void RechNodeOperator::SetRight(RechNode *xright) 
{ 
	if (right != xright)
	{
		recalcIndex = true;
		recalcAllIndex = true;
		recalcAllJoin = true;
	}
	right = xright;
	if (right != nil) right->SetParent(this);
};


VError RechNodeOperator::Perform(OptimizedQuery *query, Bittab **super, Bittab* filtre, VDB4DProgressIndicator* InProgress,  VValueBag* curdesc,
								 BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, Bittab &Nulls, RechNode* &transformation)
{
	Bittab *b1, *b2;
	VError err = VE_OK;
	
	transformation = this;
	RechNode* inutile;

	assert(false); // on ne devrait plus jamais passer par la

	if (!IsIndexe())
	{
		err = RechNode::Perform(query, super, filtre, InProgress, curdesc, context, HowToLock, exceptions, limit, Nulls, inutile);
	}
	else
	{
		if (left == nil)
		{
			if (right == nil)
			{
				*super = nil;
			}
			else
			{
				err = right->Perform(query, super, filtre, InProgress, curdesc, context, HowToLock, exceptions, limit, Nulls, inutile);
			}
		}
		else
		{
			if (right == nil)
			{
				err = left->Perform(query, super, filtre, InProgress, curdesc, context, HowToLock, exceptions, limit, Nulls, inutile);
			}
			else
			{
				b1 = nil;
				b2 = nil;
				err = left->Perform(query, &b1, filtre, InProgress, curdesc, context, HowToLock, exceptions, limit, Nulls, inutile);
				if (err == VE_OK)
				{
					Bittab subNulls;
					if (OperBool == DB4D_And) 
						err = right->Perform(query, &b2, b1, InProgress, curdesc, context, HowToLock, exceptions, limit, Nulls, inutile);
					else 
					{
						err = right->Perform(query, &b2, filtre, InProgress, curdesc, context, HowToLock, exceptions, limit, subNulls, inutile);
					}
						
					switch (OperBool)
					{
						case DB4D_And:
						case DB4D_NOTCONJ:
							/* pas necessaire si screening fait dans le perform precedent 
							if (HowToLock > DB4D_Keep_Lock_With_Record)
							{
								Bittab b3;
								b3.Or(b1):
								b3.XOr(b2);
								query->GetTargetFile()->GetDF()->UnlockBitSel(&b3, context, HowToLock);
							}
							*/
							err=b1->And(b2);
							break;
			
						case DB4D_OR:
							err=b1->Or(b2);
							if (err == VE_OK)
							{
								err = Nulls.And(&subNulls, true);
							}
							break;
			
						case DB4D_Except:
							{
								Bittab b4;
								if (HowToLock > DB4D_Keep_Lock_With_Record)
								{
									b4.Or(b1);
									b4.Or(b2);
								}
								err=b1->moins(b2);
								if (HowToLock > DB4D_Keep_Lock_With_Record)
								{
									b4.moins(b1);
									//query->GetTargetFile()->GetDF()->UnlockBitSel(&b4, context, HowToLock);
								}
							}
							break;
					}
				}
				
				*super = b1;
				ReleaseRefCountable(&b2);
			}
		}
	}
	
	return(err);
}


uBOOL RechNodeOperator::PerformSeq(sLONG nf, FicheInMem* curfic, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, 
								   BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, LocalEntityModel* model)
{
	uBOOL ok,ok2;

	if (left != nil) ok = left->PerformSeq(nf, curfic, tfb, InProgress, context, HowToLock, exceptions, limit, model);
	else ok = true;

	if (right != nil) ok2 = right->PerformSeq(nf, curfic, tfb, InProgress, context, HowToLock, exceptions, limit, model);
	else ok2 = true;

	switch (OperBool)
	{
	case DB4D_OR:
	case DB4D_NOTCONJ:
		if (ok == 2)
		{
			if (ok2 == 1)
				ok = true;
		}
		else
		{
			if (ok2 == 2)
			{
				if (ok == 1)
					ok = true;
				else
					ok = 2;
			}
			else
				ok=ok || ok2;
		}

		break;

	case DB4D_And:
		if (ok == 2)
		{
			if (ok2 == false)
				ok = false;
		}
		else
		{
			if (ok2 == 2)
			{
				if (ok == false)
					ok = false;
				else
					ok2 = 2;
			}
			else
				ok=ok && ok2;
		}
		break;

	case DB4D_Except:
		ok=ok && (!ok2);
	} 

	return(ok);
}



uBOOL RechNodeOperator::PerformSeq(FicheOnDisk* ficD, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, 
								   DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, LocalEntityModel* model)
{
	uBOOL ok,ok2;

	if (left != nil) ok = left->PerformSeq(ficD, tfb, InProgress, context, HowToLock, exceptions, limit, model);
	else ok = true;

	if (right != nil) ok2 = right->PerformSeq(ficD, tfb, InProgress, context, HowToLock, exceptions, limit, model);
	else ok2 = true;

	switch (OperBool)
	{
	case DB4D_OR:
	case DB4D_NOTCONJ:
		if (ok == 2)
		{
			if (ok2 == 1)
				ok = true;
		}
		else
		{
			if (ok2 == 2)
			{
				if (ok == 1)
					ok = true;
				else
					ok = 2;
			}
			else
				ok=ok || ok2;
		}
		break;

	case DB4D_And:
		if (ok == 2)
		{
			if (ok2 == false)
				ok = false;
		}
		else
		{
			if (ok2 == 2)
			{
				if (ok == false)
					ok = false;
				else
					ok2 = 2;
			}
			else
				ok=ok && ok2;
		}
		break;

	case DB4D_Except:
		ok=ok && (!ok2);
	} 

	return(ok);
}


void RechNodeOperator::BuildQueryPathSeq(Boolean inBefore, sLONG& curvar, CDB4DQueryPathNode* parent, Base4D* bd, VString& outString, const CDB4DQueryPathModifiers* inModifiers, Boolean first) 
{
	if (first)
		BuildAddFormula(outString, inModifiers);
	outString += L"( ";
	
	VString sleft;
	VString sright;
	VString soper;

	if (left != nil)
		left->BuildQueryPathSeq(true, curvar, parent, bd, sleft, inModifiers, false);
	if (right != nil)
		right->BuildQueryPathSeq(true, curvar, parent, bd, sright, inModifiers, false);

	outString += sleft;

	switch (OperBool)
	{
	case DB4D_OR:
		soper = L" OR ";
		break;

	case DB4D_And:
		soper = L" AND ";
		break;

	case DB4D_Except:
		soper = L" MINUS ";
		break;
	}

	outString += soper;
	outString += sright;
	outString += L" )";

}



CDB4DQueryPathNode* RechNodeOperator::BuildQueryPath(Boolean inBefore, Table* target, Base4D* bd, sLONG DejaSel, CDB4DQueryPathNode* parent, sLONG& curvar, sLONG &outvarnum, const CDB4DQueryPathModifiers* inModifiers)
{
	CDB4DQueryPathNode* leftpath = nil;
	CDB4DQueryPathNode* rightpath = nil;
	CDB4DQueryPathNode* result = nil;
	VString smess,smess2;
	sLONG sel1,sel2;

	if (IsAllIndexe() || (IsIndexe() && (inModifiers->isSimpleTree() || !(left != nil && left->IsIndexe() && right != nil && !right->IsIndexe())) ))
	{
		if (left == nil)
		{
			return right->BuildQueryPath(inBefore, target, bd, DejaSel, parent, curvar, outvarnum, inModifiers);
		}
		else
		{
			if (right == nil)
			{
				return left->BuildQueryPath(inBefore, target, bd, DejaSel, parent, curvar, outvarnum, inModifiers);
			}
			else
			{
				result = new VDB4DQueryPathNode(parent, L"", inModifiers, inBefore);
				leftpath = left->BuildQueryPath(true, target, bd, 0, result, curvar, sel1, inModifiers);
				rightpath = right->BuildQueryPath(false, target, bd, sel1, result, curvar, sel2, inModifiers);
				curvar++;
				outvarnum = curvar;

				VString selname1, selname2, selname3, soper;
				BuildSelectionName(sel1, selname1, inModifiers);
				BuildSelectionName(sel2, selname2, inModifiers);
				BuildSelectionName(outvarnum, selname3, inModifiers);

				switch (OperBool)
				{
				case DB4D_OR:
					soper = L" OR ";
					break;

				case DB4D_And:
					soper = L" AND ";
					break;

				case DB4D_Except:
					soper = L" MINUS ";
					break;
				}

				if (inModifiers->isSimpleTree())
				{
					smess += soper;
					if (inModifiers->isTempVarDisplay())
					{
						smess += tabsuffix;
						smess += tabsuffix;
						/*
						smess += selname1;
						smess += soper;
						smess += selname2;
						*/
						smess += L"  -->  ";
						smess += selname3;
					}
				}
				else
				{
					if (inModifiers->isVerbose())
					{
						smess = L"Performs ";
						smess += selname1;
					}
					else
					{
						smess = selname3;
						smess += L" := ";
						smess += selname1;
					}
					
					smess += soper;

					if (inModifiers->isVerbose())
					{
						smess += selname2;
						smess += L" into ";
						smess += selname3;
					}
					else
					{
						smess += selname2;
					}
				}

				result->SetString(smess);

			}
		}
	}
	else
	{
		RechNode* other = nil;
		if (left == nil)
		{
			other = right;
		}
		else
		{
			if (right == nil)
			{
				other = left;
			}
		}

		if (other != nil)
		{
			if (other->IsIndexe())
			{
				return other->BuildQueryPath(inBefore, target, bd, DejaSel, parent, curvar, outvarnum, inModifiers);
			}
			else
			{
				curvar++;
				outvarnum = curvar;
				if (inModifiers->isVerbose())
				{
					BuildSeqString(bd, CalcTableNum(fTargetTable, target), DejaSel, outvarnum, smess, inModifiers, GetInstance());
					other->BuildQueryPathSeq(true, curvar, parent, bd, smess2, inModifiers);
					result = new VDB4DQueryPathNode(parent, smess, inModifiers, true);
					CDB4DQueryPathNode* subline = new VDB4DQueryPathNode(result, smess2, inModifiers, false);
				}
				else
				{
					other->BuildQueryPathSeq(inBefore, curvar, parent, bd, smess2, inModifiers, false);
					BuildSeqStringNonVerbose(bd, CalcTableNum(fTargetTable, target), DejaSel, outvarnum, smess2, smess, inModifiers, GetInstance());
					result = new VDB4DQueryPathNode(parent, smess, inModifiers, inBefore);
				}
			}
		}
		else
		{
			if (IsIndexe())
			{
				assert(left->IsIndexe() && !right->IsIndexe());
				leftpath = left->BuildQueryPath(true, target, bd, DejaSel, parent, curvar, sel1, inModifiers);

				curvar++;
				outvarnum = curvar;
				if (inModifiers->isVerbose())
				{
					BuildSeqString(bd, CalcTableNum(fTargetTable, target), sel1, outvarnum, smess, inModifiers, GetInstance());
					right->BuildQueryPathSeq(true, curvar, parent, bd, smess2, inModifiers);
					result = new VDB4DQueryPathNode(parent, smess, inModifiers, true);
					CDB4DQueryPathNode* subline = new VDB4DQueryPathNode(result, smess2, inModifiers, false);
				}
				else
				{
					right->BuildQueryPathSeq(false, curvar, parent, bd, smess2, inModifiers, false);
					BuildSeqStringNonVerbose(bd, CalcTableNum(fTargetTable, target), sel1, outvarnum, smess2, smess, inModifiers, GetInstance());
					result = new VDB4DQueryPathNode(parent, smess, inModifiers, false);
				}
				result = parent;			
			}
			else
			{
				curvar++;
				outvarnum = curvar;
				if (inModifiers->isVerbose())
				{
					BuildSeqString(bd, CalcTableNum(fTargetTable, target), DejaSel, outvarnum, smess, inModifiers, GetInstance());
					BuildQueryPathSeq(true, curvar, parent, bd, smess2, inModifiers);
					result = new VDB4DQueryPathNode(parent, smess, inModifiers, true);
					CDB4DQueryPathNode* subline = new VDB4DQueryPathNode(result, smess2, inModifiers, false);
				}
				else
				{
					BuildQueryPathSeq(inBefore, curvar, parent, bd, smess2, inModifiers, false);
					BuildSeqStringNonVerbose(bd, CalcTableNum(fTargetTable, target), DejaSel, outvarnum, smess2, smess, inModifiers, GetInstance());
					result = new VDB4DQueryPathNode(parent, smess, inModifiers, inBefore);
				}
			}
		}

	}


	return result;
}


void RechNodeOperator::Describe(Base4D* bd, VString& result)
{
	switch (OperBool)
	{
	case DB4D_OR:
		result = L"Or";
		break;

	case DB4D_And:
		result= L"And";
		break;

	case DB4D_Except:
		result = L"Except";
		break;

	default:
		result = L"<Null cunjunction";
		break;
	}
}


void RechNodeOperator::DisplayTree(Base4D* bd, sLONG level, VString* result, VValueBag* bagResult)
{
	VString s, s2;

	if (left != nil)
	{
		BagElement subbag(*bagResult, OperBool == DB4D_OR ? d4::Or : d4::And);
		left->DisplayTree(bd, level+1, result, subbag);
	}

	FillLevelString(level, s);

	Describe(bd, s2);
	s += s2;
	AddCR(s);

	if (result == nil)
		DebugMsg(s);
	else
		*result += s;

	if (right != nil)
	{
		BagElement subbag(*bagResult, OperBool == DB4D_OR ? d4::Or : d4::And);
		right->DisplayTree(bd, level+1, result, subbag);
	}
}


/* =================================== */


void RechNodeMultiOperator::AddNode(RechNode* rn)
{
	if (rn != nil)
		Nodes.Add(rn);
}


void RechNodeMultiOperator::DeleteNode(ArrayNode::Iterator NodePos)
{
	sLONG pos = NodePos - Nodes.First();
	Nodes.DeleteNth(pos);
}


void RechNodeMultiOperator::DeleteNode(sLONG pos)
{
	Nodes.DeleteNth(pos);
}


void RechNodeMultiOperator::DisposeTree(void)
{
	ArrayNode::Iterator cur = Nodes.First(), end = Nodes.End();
	for (;cur != end; cur++)
	{
		if (*cur != nil)
		{
			(*cur)->DisposeTree();
			delete *cur;
		}
	}
	Nodes.SetCount(0);
}


void RechNodeMultiOperator::AdjusteIntl(VIntlMgr* inIntl)
{
	ArrayNode::Iterator cur = Nodes.First(), end = Nodes.End();
	for (;cur != end; cur++)
	{
		if (testAssert(*cur != nil))
		{
			(*cur)->AdjusteIntl(inIntl);
		}
	}
}


bool RechNodeMultiOperator::NeedToLoadFullRec()
{
	bool result = false;

	ArrayNode::Iterator cur = Nodes.First(), end = Nodes.End();
	for (;cur != end; cur++)
	{
		if (testAssert(*cur != nil))
		{
			result = result || (*cur)->NeedToLoadFullRec();
		}
	}

	return result;
}


uBOOL RechNodeMultiOperator::IsIndexe(void)
{
	uBOOL result = (OperBool == DB4D_OR);
	uBOOL result2 = true;

	if (recalcIndex)
	{
		ArrayNode::Iterator cur = Nodes.First(), end = Nodes.End();
		for (;cur != end; cur++)
		{
			if (testAssert(*cur != nil))
			{
				result2 = (*cur)->IsIndexe();
				if (OperBool == DB4D_OR)
				{
					result = result && result2;
				}
				else
				{
					result = result || result2;
				}
			}
		}
		recalcIndex = false;
		isindexe = result;
	}
	else
	{
		result = isindexe;
	}

	return result;
}


uBOOL RechNodeMultiOperator::IsAllIndexe(void)
{
	uBOOL result = true;

	if (recalcAllIndex)
	{
		ArrayNode::Iterator cur = Nodes.First(), end = Nodes.End();
		for (;cur != end; cur++)
		{
			if (testAssert(*cur != nil))
			{
				result = result && (*cur)->IsAllIndexe();
			}
		}
		isallindexe = result;
		recalcAllIndex = false;
	}
	else
	{
		result = isallindexe;
	}

	return result;
}


uBOOL RechNodeMultiOperator::IsConst()
{
	uBOOL result = false;

	if (recalcConst)
	{
		ArrayNode::Iterator cur = Nodes.First(), end = Nodes.End();
		for (;cur != end; cur++)
		{
			if (testAssert(*cur != nil))
			{
				if ((*cur)->IsConst())
				{
					if (OperBool == DB4D_OR)
					{
						resultconst = (*cur)->GetResultConst();
						if (resultconst == 1)
						{
							result = true;
							break;
						}
					}
					else
					{
						resultconst = (*cur)->GetResultConst();
						if (resultconst == 0 || resultconst == 2)
						{
							result = true;
							break;
						}

					}
				}
			}
		}
		isconst = result;
		recalcConst = false;
	}
	else
	{
		result = isconst;
	}

	return result;
}


uBOOL RechNodeMultiOperator::IsAllJoin(void)
{
	uBOOL result = true;

	if (recalcAllJoin)
	{
		ArrayNode::Iterator cur = Nodes.First(), end = Nodes.End();
		for (;cur != end; cur++)
		{
			if (testAssert(*cur != nil))
			{
				result = result && (*cur)->IsAllJoin();
			}
		}
		isalljoin = result;
		recalcAllJoin = false;
	}
	else
	{
		result = isalljoin;
	}

	return result;
}



VError RechNodeMultiOperator::Perform(OptimizedQuery *query, Bittab **super, Bittab* filtre, VDB4DProgressIndicator* InProgress,  VValueBag* curdesc,
									  BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, Bittab &Nulls, RechNode* &transformation)
{
	Bittab *b1, *b2;
	VError err = VE_OK;
	transformation = this;

	if (IsIndexe() && ((CountDiskAccess()/5) > query->CalculateNBDiskAccessForSeq(filtre, context, CountBlobSources())))
	{
		transformation = query->xTransformSeq(this, true);
		if (transformation != this)
		{
			if (transformation != nil)
			{
				err = transformation->Perform(query, super, filtre, InProgress, curdesc, context, HowToLock, exceptions, limit, Nulls, transformation);
			}
			return err;
		}
	}

	{
		if (!IsIndexe())
		{
			err = RechNode::Perform(query, super, filtre, InProgress, curdesc, context, HowToLock, exceptions, limit, Nulls, transformation);
		}
		else
		{
			if (Nodes.GetCount() == 0)
			{
				*super = nil;
			}
			else
			{
				if (Nodes.GetCount() == 1)
				{
					RechNode* nod = Nodes[0];
					err = nod->Perform(query, super, filtre, InProgress, curdesc, context, HowToLock, exceptions, limit, Nulls, nod);
					Nodes[0] = nod;
				}
				else
				{
					uLONG start = query->GetStartingTime();

					VValueBag* subline = nil;
					if (query->IsQueryDescribed())
					{
						VString s;
						if (OperBool == DB4D_And)
							s = L"AND";
						else
							s = L"OR";
						subline = query->AddToDescription(curdesc, s, start, nil);
					}

					b1 = nil;
					ArrayNode::Iterator cur = Nodes.First(), end = Nodes.End();
					bool canperformindex = true;

					for (;cur != end && err == VE_OK && canperformindex; cur++)
					{
						if (testAssert(*cur != nil))
						{
							RechNode* nod = *cur;
							if (nod->IsConst())
							{
								// normalement rien a faire car on se retrouve dans les cas non filtres par RechNodeMultiOperator::IsConst
								// donc la constante ne va rien modifier a l'operateur
							}
							else
							{
								if (b1 == nil)
								{
									err = nod->Perform(query, &b1, filtre, InProgress, subline, context, HowToLock, exceptions, limit, Nulls, nod);
									*cur = nod;
								}
								else
								{
									b2 = nil;

									if (OperBool == DB4D_And)
									{
										sLONG recfiltre = b1->Compte();
										if (recfiltre == 0)
										{
											canperformindex = false;
										}
										else
										{
											if ((cur+1) != end)
											{
												sLONG8 tot = 0;
												for (ArrayNode::Iterator curx = cur, endx = Nodes.End(); curx != endx; curx++)
												{
													RechNode* nodx = *curx;
													if (nodx != nil)
														tot = tot + nodx->CountDiskAccess();
												}
												if ( ((sLONG8)recfiltre * ((sLONG8)CountBlobSources()*3+1)) < (tot/10))
												{
													RechNodeMultiOperator* rnoper = new RechNodeMultiOperator;
													rnoper->OperBool = DB4D_And;
													for (ArrayNode::Iterator curx = cur, endx = Nodes.End(); curx != endx; curx++)
													{
														RechNode* nodx = *curx;
														if (nodx != nil)
															rnoper->AddNode(nodx);
														*curx = nil;
													}
													RechNode* newnode = query->xTransformSeq(rnoper, true);
													*cur = newnode;
													err = newnode->Perform(query, &b2, b1, InProgress, subline, context, HowToLock, exceptions, limit, Nulls, newnode);
													*cur = newnode;
													if (err == VE_OK)
													{
														err=b1->And(b2);
													}
													canperformindex = false;
												}
											}
										}

										if (canperformindex)
										{
											err = nod->Perform(query, &b2, b1, InProgress, subline, context, HowToLock, exceptions, limit, Nulls, nod);
											*cur = nod;
											if (err == VE_OK)
											{
												err=b1->And(b2);
											}
										}
									}
									else
									{
										Bittab subNulls;
										err = nod->Perform(query, &b2, filtre, InProgress, subline, context, HowToLock, exceptions, limit, subNulls, nod);
										*cur = nod;
										if (err == VE_OK)
										{
											err=b1->Or(b2);
											if (err == VE_OK)
												err = Nulls.And(&subNulls, true);
										}
									}
									ReleaseRefCountable(&b2);
								}
							}
						}
					}
					*super = b1;

					if (query->IsQueryDescribed())
					{
						subline->SetLong(QueryKeys::time, VSystem::GetCurrentTime() - start);
						subline->SetLong(QueryKeys::recordsfounds, b1 == nil ? 0 : b1->Compte());
					}
				}
			}
		}
	}

	return(err);
}


VValueSingle* RechNodeMultiOperator::Compute(LocalEntityRecord* erec, BaseTaskInfo* context, VError& err)
{
	VValueSingle* result = nil;
	uBOOL ok = OperBool == DB4D_And, ok2;

	ArrayNode::Iterator cur = Nodes.First(), end = Nodes.End();
	for (;cur != end; cur++)
	{
		if (testAssert(*cur != nil))
		{
			RechNode* nod = *cur;
			VValueSingle* cv = nod->Compute(erec, context, err);
			if (cv == nil)
				ok2 = 2;
			else
			{
				if (cv->GetValueKind() == VK_BOOLEAN)
				{
					if (cv->IsNull())
						ok2 = 2;
					else if (cv->GetBoolean())
						ok2 = 1;
					else ok2 = 0;
				}
				else
					ok2 = 2;
				delete cv;
			}

			switch (OperBool)
			{
				case DB4D_OR:
				case DB4D_NOTCONJ:
					if (ok == 2)
					{
						if (ok2 == 1)
							ok = true;
					}
					else
					{
						if (ok2 == 2)
						{
							if (ok == 1)
								ok = true;
							else
								ok = 2;
						}
						else
							ok=ok || ok2;
					}
					break;

				case DB4D_And:
					if (ok == 2)
					{
						if (ok2 == false)
							ok = false;
					}
					else
					{
						if (ok2 == 2)
						{
							if (ok == false)
								ok = false;
							else
								ok = 2;
						}
						else
							ok=ok && ok2;
					}
					break;
			}
		}

		if (ok == 0 && OperBool == DB4D_And)
			break;
		else if (ok == 1 && OperBool == DB4D_OR)
			break;
	}

	if (ok2 != 2)
	{
		result = new VBoolean(ok);
	}

	return result;
}


uBOOL RechNodeMultiOperator::PerformSeq(sLONG nf, FicheInMem* curfic, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, 
										BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, LocalEntityModel* model)
{
	uBOOL ok = OperBool == DB4D_And, ok2;

	ArrayNode::Iterator cur = Nodes.First(), end = Nodes.End();
	for (;cur != end; cur++)
	{
		if (testAssert(*cur != nil))
		{
			RechNode* nod = *cur;
			ok2 = nod->PerformSeq(nf, curfic, tfb, InProgress, context, HowToLock, exceptions, limit, model);
			switch (OperBool)
			{
				case DB4D_OR:
				case DB4D_NOTCONJ:
					if (ok == 2)
					{
						if (ok2 == 1)
							ok = true;
					}
					else
					{
						if (ok2 == 2)
						{
							if (ok == 1)
								ok = true;
							else
								ok = 2;
						}
						else
							ok=ok || ok2;
					}
					break;

				case DB4D_And:
					if (ok == 2)
					{
						if (ok2 == false)
							ok = false;
					}
					else
					{
						if (ok2 == 2)
						{
							if (ok == false)
								ok = false;
							else
								ok = 2;
						}
						else
							ok=ok && ok2;
					}
					break;
			}
		}
	}

	return(ok);
}


uBOOL RechNodeMultiOperator::PerformFullSeq(sLONG nf, FicheInMem* curfic, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, 
										BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, LocalEntityModel* model)
{
	uBOOL ok = OperBool == DB4D_And, ok2;

	ArrayNode::Iterator cur = Nodes.First(), end = Nodes.End();
	for (;cur != end; cur++)
	{
		if (testAssert(*cur != nil))
		{
			RechNode* nod = *cur;
			ok2 = nod->PerformFullSeq(nf, curfic, tfb, InProgress, context, HowToLock, exceptions, limit, model);
			switch (OperBool)
			{
			case DB4D_OR:
			case DB4D_NOTCONJ:
				if (ok == 2)
				{
					if (ok2 == 1)
						ok = true;
				}
				else
				{
					if (ok2 == 2)
					{
						if (ok == 1)
							ok = true;
						else
							ok = 2;
					}
					else
						ok=ok || ok2;
				}
				break;

			case DB4D_And:
				if (ok == 2)
				{
					if (ok2 == false)
						ok = false;
				}
				else
				{
					if (ok2 == 2)
					{
						if (ok == false)
							ok = false;
						else
							ok = 2;
					}
					else
						ok=ok && ok2;
				}
				break;
			}

			if (ok != 2)
			{
				if (ok)
				{
					if (OperBool == DB4D_OR)
						break;
				}
				else
				{
					if (OperBool == DB4D_And)
						break;
				}
			}
		}
	}

	return(ok);
}


uBOOL RechNodeMultiOperator::PerformSeq(FicheOnDisk* ficD, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, 
										BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, LocalEntityModel* model)
{
	uBOOL ok = OperBool == DB4D_And, ok2;

	ArrayNode::Iterator cur = Nodes.First(), end = Nodes.End();
	for (;cur != end; cur++)
	{
		if (testAssert(*cur != nil))
		{
			RechNode* nod = *cur;
			ok2 = nod->PerformSeq(ficD, tfb, InProgress, context, HowToLock, exceptions, limit, model);
			switch (OperBool)
			{
			case DB4D_OR:
			case DB4D_NOTCONJ:
				if (ok == 2)
				{
					if (ok2 == 1)
						ok = true;
				}
				else
				{
					if (ok2 == 2)
					{
						if (ok == 1)
							ok = true;
						else
							ok = 2;
					}
					else
						ok=ok || ok2;
				}
				break;

			case DB4D_And:
				if (ok == 2)
				{
					if (ok2 == false)
						ok = false;
				}
				else
				{
					if (ok2 == 2)
					{
						if (ok == false)
							ok = false;
						else
							ok = 2;
					}
					else
						ok=ok && ok2;
				}
				break;
			}
		}
	}

	return(ok);
}


sLONG8 RechNodeMultiOperator::CountDiskAccess()
{
	sLONG8 tot = 0;
	ArrayNode::Iterator cur = Nodes.First(), end = Nodes.End();
	for (;cur != end; cur++)
	{
		RechNode* nod = *cur;
		if (nod != nil)
			tot = tot + nod->CountDiskAccess();
	}

	return tot;
}



sLONG RechNodeMultiOperator::CountBlobSources()
{
	sLONG tot = 0;
	ArrayNode::Iterator cur = Nodes.First(), end = Nodes.End();
	for (;cur != end; cur++)
	{
		RechNode* nod = *cur;
		if (nod != nil)
			tot = tot + nod->CountBlobSources();
	}

	return tot;
}


void RechNodeMultiOperator::BuildQueryPathSeq(Boolean inBefore, sLONG& curvar, CDB4DQueryPathNode* parent, Base4D* bd, VString& outString, const CDB4DQueryPathModifiers* inModifiers, Boolean first) 
{
	if (first)
		BuildAddFormula(outString, inModifiers);
	outString += L"( ";

	VString ss;
	VString soper;

	switch (OperBool)
	{
		case DB4D_OR:
			soper = L" OR ";
			break;

		case DB4D_And:
			soper = L" AND ";
			break;
	}

	Boolean deja = false;
	ArrayNode::Iterator cur = Nodes.First(), end = Nodes.End();
	for (;cur != end; cur++)
	{
		if (deja)
		{
			outString += soper;
		}
		RechNode* nod = *cur;
		assert(nod != nil);
		nod->BuildQueryPathSeq(true, curvar, parent, bd, ss, inModifiers, false);
		outString += ss;
		deja = true;
	}

	outString += L" )";

}



CDB4DQueryPathNode* RechNodeMultiOperator::BuildQueryPath(Boolean inBefore, Table* target, Base4D* bd, sLONG DejaSel, CDB4DQueryPathNode* parent, sLONG& curvar, sLONG &outvarnum, const CDB4DQueryPathModifiers* inModifiers)
{
	CDB4DQueryPathNode* leftpath = nil;
	CDB4DQueryPathNode* rightpath = nil;
	CDB4DQueryPathNode* result = nil;
	VString smess,smess2;
	sLONG sel1,sel2;

#if 0
	if (IsAllIndexe() || (IsIndexe() && (inModifiers->isSimpleTree() || !(left != nil && left->IsIndexe() && right != nil && !right->IsIndexe())) ))
	{
		if (left == nil)
		{
			return right->BuildQueryPath(inBefore, target, bd, DejaSel, parent, curvar, outvarnum, inModifiers);
		}
		else
		{
			if (right == nil)
			{
				return left->BuildQueryPath(inBefore, target, bd, DejaSel, parent, curvar, outvarnum, inModifiers);
			}
			else
			{
				result = new VDB4DQueryPathNode(parent, L"", inModifiers, inBefore);
				leftpath = left->BuildQueryPath(true, target, bd, 0, result, curvar, sel1, inModifiers);
				rightpath = right->BuildQueryPath(false, target, bd, sel1, result, curvar, sel2, inModifiers);
				curvar++;
				outvarnum = curvar;

				VString selname1, selname2, selname3, soper;
				BuildSelectionName(sel1, selname1, inModifiers);
				BuildSelectionName(sel2, selname2, inModifiers);
				BuildSelectionName(outvarnum, selname3, inModifiers);

				switch (OperBool)
				{
				case DB4D_OR:
					soper = L" OR ";
					break;

				case DB4D_And:
					soper = L" AND ";
					break;

				case DB4D_Except:
					soper = L" MINUS ";
					break;
				}

				if (inModifiers->isSimpleTree())
				{
					smess += soper;
					if (inModifiers->isTempVarDisplay())
					{
						smess += tabsuffix;
						smess += tabsuffix;
						/*
						smess += selname1;
						smess += soper;
						smess += selname2;
						*/
						smess += L"  -->  ";
						smess += selname3;
					}
				}
				else
				{
					if (inModifiers->isVerbose())
					{
						smess = L"Performs ";
						smess += selname1;
					}
					else
					{
						smess = selname3;
						smess += L" := ";
						smess += selname1;
					}

					smess += soper;

					if (inModifiers->isVerbose())
					{
						smess += selname2;
						smess += L" into ";
						smess += selname3;
					}
					else
					{
						smess += selname2;
					}
				}

				result->SetString(smess);

			}
		}
	}
	else
	{
		RechNode* other = nil;
		if (left == nil)
		{
			other = right;
		}
		else
		{
			if (right == nil)
			{
				other = left;
			}
		}

		if (other != nil)
		{
			if (other->IsIndexe())
			{
				return other->BuildQueryPath(inBefore, target, bd, DejaSel, parent, curvar, outvarnum, inModifiers);
			}
			else
			{
				curvar++;
				outvarnum = curvar;
				if (inModifiers->isVerbose())
				{
					BuildSeqString(bd, CalcTableNum(fTargetTable, target), DejaSel, outvarnum, smess, inModifiers);
					other->BuildQueryPathSeq(true, curvar, parent, bd, smess2, inModifiers);
					result = new VDB4DQueryPathNode(parent, smess, inModifiers, true);
					CDB4DQueryPathNode* subline = new VDB4DQueryPathNode(result, smess2, inModifiers, false);
				}
				else
				{
					other->BuildQueryPathSeq(inBefore, curvar, parent, bd, smess2, inModifiers, false);
					BuildSeqStringNonVerbose(bd, CalcTableNum(fTargetTable, target), DejaSel, outvarnum, smess2, smess, inModifiers);
					result = new VDB4DQueryPathNode(parent, smess, inModifiers, inBefore);
				}
			}
		}
		else
		{
			if (IsIndexe())
			{
				assert(left->IsIndexe() && !right->IsIndexe());
				leftpath = left->BuildQueryPath(true, target, bd, DejaSel, parent, curvar, sel1, inModifiers);

				curvar++;
				outvarnum = curvar;
				if (inModifiers->isVerbose())
				{
					BuildSeqString(bd, CalcTableNum(fTargetTable, target), sel1, outvarnum, smess, inModifiers);
					right->BuildQueryPathSeq(true, curvar, parent, bd, smess2, inModifiers);
					result = new VDB4DQueryPathNode(parent, smess, inModifiers, true);
					CDB4DQueryPathNode* subline = new VDB4DQueryPathNode(result, smess2, inModifiers, false);
				}
				else
				{
					right->BuildQueryPathSeq(false, curvar, parent, bd, smess2, inModifiers, false);
					BuildSeqStringNonVerbose(bd, CalcTableNum(fTargetTable, target), sel1, outvarnum, smess2, smess, inModifiers);
					result = new VDB4DQueryPathNode(parent, smess, inModifiers, false);
				}
				result = parent;			
			}
			else
			{
				curvar++;
				outvarnum = curvar;
				if (inModifiers->isVerbose())
				{
					BuildSeqString(bd, CalcTableNum(fTargetTable, target), DejaSel, outvarnum, smess, inModifiers);
					BuildQueryPathSeq(true, curvar, parent, bd, smess2, inModifiers);
					result = new VDB4DQueryPathNode(parent, smess, inModifiers, true);
					CDB4DQueryPathNode* subline = new VDB4DQueryPathNode(result, smess2, inModifiers, false);
				}
				else
				{
					BuildQueryPathSeq(inBefore, curvar, parent, bd, smess2, inModifiers, false);
					BuildSeqStringNonVerbose(bd, CalcTableNum(fTargetTable, target), DejaSel, outvarnum, smess2, smess, inModifiers);
					result = new VDB4DQueryPathNode(parent, smess, inModifiers, inBefore);
				}
			}
		}

	}

#endif
	result = new VDB4DQueryPathNode(parent, L"a faire", inModifiers, inBefore);
	return result;
}


void RechNodeMultiOperator::Describe(Base4D* bd, VString& result)
{
	switch (OperBool)
	{
	case DB4D_OR:
		result = L"Or";
		break;

	case DB4D_And:
		result = L"And";
		break;

	case DB4D_Except:
		result = L"Except";
		break;

	default:
		result = L"<Null cunjunction";
		break;
	}
}


void RechNodeMultiOperator::FullyDescribe(Base4D* bd, VString& result)
{
	ArrayNode::Iterator cur = Nodes.First(), end = Nodes.End();
	Boolean first = true;
	for (;cur != end; cur++)
	{
		if (first)
		{
			first = false;
		}
		else
		{
			if (OperBool == DB4D_And)
				result += L" And ";
			else
				result += L" Or ";
		}
		if (testAssert(*cur != nil))
		{
			RechNode* nod = *cur;
			if (nod->IsComposed())
			{
				result += L"(";
			}
			VString s;
			nod->FullyDescribe(bd, s);
			result += s;
			if (nod->IsComposed())
			{
				result += L")";
			}
		}
	}

}


void RechNodeMultiOperator::DisplayTree(Base4D* bd, sLONG level, VString* result, VValueBag* bagResult)
{
	VString s, s2;

	FillLevelString(level, s);
	Describe(bd, s2);
	s += s2;
	AddCR(s);

	Boolean deja = false;
	ArrayNode::Iterator cur = Nodes.First(), end = Nodes.End();
	for (;cur != end; cur++)
	{
		if (deja)
		{
			if (result == nil)
				DebugMsg(s);
			else
				*result += s;
		}
		RechNode* nod = *cur;
		assert(nod != nil);
		BagElement subbag(*bagResult, OperBool == DB4D_OR ? d4::Or : d4::And);
		nod->DisplayTree(bd, level+1, result, subbag);
		deja = true;
	}

}


/* =================================== */


uBOOL RechNodeNot::IsConst() 
{
	if (left != nil)
	{
		return left->IsConst();
	}
	else
		return false;
}


uBOOL RechNodeNot::GetResultConst()
{
	uBOOL res = 0;
	if (left != nil)
	{
		res = left->GetResultConst();
		if (res != 2)
		{
			res = ! res;
		}
	}
	return res;
}



uBOOL RechNodeNot::IsIndexe(void)
{
	uBOOL result = true;

	if (recalcIndex)
	{
		if (left != nil) 
			result = left->IsIndexe();
		recalcIndex = false;
		isindexe = result;
	}
	else
	{
		result = isindexe;
	}

	return result;
}


uBOOL RechNodeNot::IsAllIndexe(void)
{
	uBOOL result = true;
	
	if (recalcAllIndex)
	{
		if (left != nil) result = left->IsAllIndexe();
		isallindexe = result;
		recalcAllIndex = false;
	}
	else
	{
		result = isallindexe;
	}
	
	return result;
}


uBOOL RechNodeNot::IsAllJoin(void)
{
	uBOOL result = true;

	if (recalcAllJoin)
	{
		if (left != nil) result = left->IsAllJoin();
		isalljoin = result;
		recalcAllJoin = false;
	}
	else
	{
		result = isalljoin;
	}

	return result;
}


void RechNodeNot::AdjusteIntl(VIntlMgr* inIntl)
{
	if (left != nil)
		left->AdjusteIntl(inIntl);
}



VError RechNodeNot::Perform(OptimizedQuery *query, Bittab **super, Bittab* filtre, VDB4DProgressIndicator* InProgress,  VValueBag* curdesc,
							BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit,Bittab &Nulls, RechNode* &transformation)
{
	Bittab *b1, *b2;
	VError err = VE_OK;
	transformation = this;

	if (!IsIndexe())
	{
		err = RechNode::Perform(query, super, filtre, InProgress, curdesc, context, HowToLock, exceptions, limit, Nulls, transformation);
	}
	else
	{
		if (left != nil)
		{
			uLONG start = query->GetStartingTime();
			VValueBag* subline = nil;
			if (query->IsQueryDescribed())
			{
				VString s;
				FullyDescribe(query->GetTargetFile()->GetOwner(), s);
				subline = query->AddToDescription(curdesc, s, start, nil);
			}

			b1 = nil;
			err = left->Perform(query, &b1, nil, InProgress, subline, context, HowToLock, exceptions, limit, Nulls, left);
			if (b1 != nil)
			{
				b1->aggrandit(query->GetMaxRecords());
				err = b1->Invert();
				query->GetTargetFile()->GetDF()->TakeOffBits(b1, context);
				if (err == VE_OK)
					err = query->MaximizeBittab(b1);
				if (err == VE_OK)
				{
					err = b1->moins(&Nulls, true);
					if (filtre != nil) 
						err = b1->And(filtre);
				}
			}
			*super = b1;

			if (query->IsQueryDescribed())
			{
				subline->SetLong(QueryKeys::time, VSystem::GetCurrentTime() - start);
				subline->SetLong(QueryKeys::recordsfounds, b1 == nil ? 0 : b1->Compte());
			}
		}
		else
		{
			*super = nil;
		}
	}

	return(err);
}


VValueSingle* RechNodeNot::Compute(LocalEntityRecord* erec, BaseTaskInfo* context, VError& err)
{
	VValueSingle* result = nil;
	if (left != nil)
	{
		result = left->Compute(erec, context, err);
		if (result != nil)
		{
			if (result->GetValueKind() == VK_BOOLEAN && !result->IsNull())
			{
				result->FromBoolean(!result->GetBoolean());
			}
			else
				delete result;
			result = nil;
		}
	}
	return result;
}


uBOOL RechNodeNot::PerformSeq(sLONG nf, FicheInMem* curfic, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, 
							  BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, LocalEntityModel* model)
{
	uBOOL ok;

	if (left != nil) 
		ok = left->PerformSeq(nf, curfic, tfb, InProgress, context, HowToLock, exceptions, limit, model);
	else 
		ok = true;

	if (ok != 2)
		ok = !ok;

	return(ok);
}


uBOOL RechNodeNot::PerformFullSeq(sLONG nf, FicheInMem* curfic, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, 
							  BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, LocalEntityModel* model)
{
	uBOOL ok;

	if (left != nil) 
		ok = left->PerformFullSeq(nf, curfic, tfb, InProgress, context, HowToLock, exceptions, limit, model);
	else 
		ok = true;

	if (ok != 2)
		ok = !ok;

	return(ok);
}


uBOOL RechNodeNot::PerformSeq(FicheOnDisk* ficD, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, 
							  BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, LocalEntityModel* model)
{
	uBOOL ok;

	if (left != nil) 
		ok = left->PerformSeq(ficD, tfb, InProgress, context, HowToLock, exceptions, limit, model);
	else 
		ok = true;

	if (ok != 2)
		ok = !ok;

	return(ok);
}

sLONG8 RechNodeNot::CountDiskAccess()
{
	if (left == nil)
		return 0;
	else
		return left->CountDiskAccess();
}



sLONG RechNodeNot::CountBlobSources()
{
	if (left == nil)
		return 0;
	else
		return left->CountBlobSources();
}


void RechNodeNot::BuildQueryPathSeq(Boolean inBefore, sLONG& curvar, CDB4DQueryPathNode* parent, Base4D* bd, VString& outString, const CDB4DQueryPathModifiers* inModifiers, Boolean first) 
{
	if (first)
		BuildAddFormula(outString, inModifiers);
	outString += L" NOT ( ";

	VString sleft;

	if (left != nil)
		left->BuildQueryPathSeq(inBefore, curvar, parent, bd, sleft, inModifiers, false);

	outString += sleft;

	outString += L" )";

}


CDB4DQueryPathNode* RechNodeNot::BuildQueryPath(Boolean inBefore, Table* target, Base4D* bd, sLONG DejaSel, CDB4DQueryPathNode* parent, sLONG& curvar, sLONG &outvarnum, const CDB4DQueryPathModifiers* inModifiers)
{
	CDB4DQueryPathNode* result = nil;
	if (left != nil)
	{
		left->BuildQueryPath(inBefore, target, bd, DejaSel, parent, curvar, outvarnum, inModifiers);
		VString smess, selname;
		BuildSelectionName(outvarnum, selname, inModifiers);
		smess = selname;
		smess += L" := NOT ";
		smess += selname;
		result = new VDB4DQueryPathNode(parent, smess, inModifiers, inBefore);
	}
	return result;
}


void RechNodeNot::Describe(Base4D* bd, VString& result)
{
	result = L"Not";
}


void RechNodeNot::DisplayTree(Base4D* bd, sLONG level, VString* result, VValueBag* bagResult)
{
	VString s;

	FillLevelString(level, s);

	s.AppendString(L"Not");
	AddCR(s);

	if (result == nil)
		DebugMsg(s);
	else
		*result += s;

	if (left != nil)
	{
		BagElement subbag(*bagResult, d4::Not);
		left->DisplayTree(bd, level+1, result, subbag);
	}
}


void RechNodeNot::FullyDescribe(Base4D* bd, VString& result)
{
	if (left != nil)
	{
		result += L"Not ";
		if (left->IsComposed())
		{
			result += L"(";
		}
		left->FullyDescribe(bd, result);
		if (left->IsComposed())
		{
			result += L")";
		}
	}
}



/* =================================== */


CDB4DQueryPathNode* RechNodeBaseJoin::BuildQueryPath(Boolean inBefore, Table* target, Base4D* bd, sLONG DejaSel, CDB4DQueryPathNode* parent, sLONG& curvar, sLONG &outvarnum, const CDB4DQueryPathModifiers* inModifiers)
{
	return nil;
}


void RechNodeBaseJoin::Describe(Base4D* bd, VString& result)
{
	VString s, s2, s3;
	{
		Table* t = bd->RetainTable(fNumTable1);
		Field* f = nil;
		s2 = L"<Null field>";
		if (t != nil)
		{
			f = t->RetainField(fNumField1);
			if (f != nil)
			{
				t->GetName(s2);
				if (fNumInstance1 != 0)
					s2 += L"("+ToString(fNumInstance1)+L")";
				s2.AppendString(L".");
				f->GetName(s3);
				s2.AppendString(s3);
				f->Release();
			}
			t->Release();
		}
		s.AppendString(s2);
	}

	s.AppendString(L" = ");

	{
		Table* t = bd->RetainTable(fNumTable2);
		Field* f = nil;
		s2 = L"<Null field>";
		if (t != nil)
		{
			f = t->RetainField(fNumField2);
			if (f != nil)
			{
				t->GetName(s2);
				if (fNumInstance2 != 0)
					s2 += L"("+ToString(fNumInstance2)+L")";
				s2.AppendString(L".");
				f->GetName(s3);
				s2.AppendString(s3);
				f->Release();
			}
			t->Release();
		}
		s.AppendString(s2);
	}
	result = s;
}


void RechNodeBaseJoin::DisplayTree(Base4D* bd, sLONG level, VString* result, VValueBag* bagResult)
{
	VString s, s2, s3;

	FillLevelString(level, s);
	Describe(bd, s2);
	if (bagResult != nil)
		bagResult->SetString(d4::item, s2);
	s += s2;
	AddCR(s);

	if (result == nil)
		DebugMsg(s);
	else
		*result += s;
}






/* =================================== */


Boolean JoinPath::AllReadyIn(sLONG xnumtable1, sLONG xnumfield1, sLONG xnumtable2, sLONG xnumfield2, sLONG xnuminstance1, sLONG xnuminstance2)
{
	Boolean trouve = false;
	sLONG i, nb = (sLONG)fPath.size();
	for (i=0; i<nb; i++)
	{
		JoinRefCouple* couple = &fPath[i];
		if (/*couple->fNumField1 == xnumfield1 && couple->fNumField2 == xnumfield2
			&& */couple->fNumTable1 == xnumtable1 && couple->fNumTable2 == xnumtable2
			&& couple->fNumInstance1 == xnuminstance1 && couple->fNumInstance2 == xnuminstance2)
		{
			trouve = true;

			bool trouveField = false;
			if (couple->fNumField1 == xnumfield1 && couple->fNumField2 == xnumfield2)
				trouveField = true;
			else
			{
				for (sLONG j = 0; j < couple->fNbOtherFields1; j++)
				{
					if (couple->fOtherFields1[j] == xnumfield1 && couple->fOtherFields2[j] == xnumfield2)
					{
						trouveField = true;
						break;
					}
				}
			}
			if (!trouveField)
			{
				if (couple->fNbOtherFields1 < 10)
				{
					couple->fOtherFields1[couple->fNbOtherFields1] = xnumfield1;
					couple->fOtherFields2[couple->fNbOtherFields2] = xnumfield2;
					couple->fNbOtherFields1++;
					couple->fNbOtherFields2++;
				}

			}
			break;
		}

		if (/*couple->fNumField2 == xnumfield1 && couple->fNumField1 == xnumfield2
			&& */couple->fNumTable2 == xnumtable1 && couple->fNumTable1 == xnumtable2
			&& couple->fNumInstance2 == xnuminstance1 && couple->fNumInstance1 == xnuminstance2)
		{
			trouve = true;

			bool trouveField = false;
			if (couple->fNumField1 == xnumfield2 && couple->fNumField2 == xnumfield1)
				trouveField = true;
			else
			{
				for (sLONG j = 0; j < couple->fNbOtherFields1; j++)
				{
					if (couple->fOtherFields1[j] == xnumfield2 && couple->fOtherFields2[j] == xnumfield1)
					{
						trouveField = true;
						break;
					}
				}
			}
			if (!trouveField)
			{
				if (couple->fNbOtherFields1 < 10)
				{
					couple->fOtherFields1[couple->fNbOtherFields1] = xnumfield2;
					couple->fOtherFields2[couple->fNbOtherFields2] = xnumfield1;
					couple->fNbOtherFields1++;
					couple->fNbOtherFields2++;
				}

			}
			break;
		}

	}
	return trouve;
}


VError JoinPath::AddJoin(sLONG xnumtable1, sLONG xnumfield1, sLONG xnumtable2, sLONG xnumfield2, sLONG xnuminstance1, sLONG xnuminstance2)
{
	VError err = VE_OK;
	if (!AllReadyIn(xnumtable1, xnumfield1, xnumtable2, xnumfield2, xnuminstance1, xnuminstance2))
	{
		JoinRefCouple couple;
		couple.fNumField1 = xnumfield1;
		couple.fNumField2 = xnumfield2;
		couple.fNumTable1 = xnumtable1;
		couple.fNumTable2 = xnumtable2;
		couple.fNumInstance1 = xnuminstance1;
		couple.fNumInstance2 = xnuminstance2;
		couple.fNbOtherFields1 = 0;
		couple.fNbOtherFields2 = 0;
		fPath.push_back(couple);
	}
	return err;
}


VError JoinPath::AddJoin(JoinRefCouple& joinref)
{
	fPath.push_back(joinref);
	return VE_OK;
}

VError JoinPath::AddJoin(const FieldArray& sources, const FieldArray& dests, sLONG xnuminstance1, sLONG xnuminstance2)
{
	VError err = VE_OK;
	JoinRefCouple couple;
	assert(sources.GetCount() > 0 && sources.GetCount() == dests.GetCount() );
	sLONG i = 0;

	Field* cri = *(sources.First());
	couple.fNumField1 = cri->GetPosInRec();
	couple.fNumTable1 = cri->GetOwner()->GetNum();

	cri = *(dests.First());
	couple.fNumField2 = cri->GetPosInRec();
	couple.fNumTable2 = cri->GetOwner()->GetNum();

	couple.fNbOtherFields1 = sources.GetCount() - 1;
	couple.fNbOtherFields2 = dests.GetCount() - 1;

	couple.fNumInstance1 = xnuminstance1;
	couple.fNumInstance2 = xnuminstance2;

	for (FieldArray::ConstIterator cur = sources.First()+1, end = sources.End(), curdest = dests.First()+1; cur != end; cur ++, curdest++, i++)
	{
		couple.fOtherFields1[i] = (*cur)->GetPosInRec();
		couple.fOtherFields2[i] = (*curdest)->GetPosInRec();
	}
	fPath.push_back(couple);
	return err;
}


sLONG JoinPath::FindTarget(sLONG from, sLONG numtable, SmallArrayOfBoolean &Deja, sLONG numinstance)
{
	sLONG trouve = -1, i,nb = (sLONG)fPath.size();

	for (i = from; i<nb; i++)
	{
		if (!Deja[i])
		{
			JoinRefCouple* co = &(fPath[i]);
			if ((co->fNumTable1 == numtable && co->fNumInstance1 == numinstance) || (co->fNumTable2 == numtable && co->fNumInstance2 == numinstance))
			{
				trouve = i;
				break;
			}
		}
	}
	return trouve;
}



/* =================================== */


RechNodeSubQuery::RechNodeSubQuery(RechNode* root)
{
	TypNode = NoeudSubQuery;
	fRoot = root;
	fUnSolved = true;
	fIsIndexed = false;
	fInd = nil;
	fOtherInd = nil;
	data = nil;
	rechtyp = -1;
	fContainsSubTables = false;
	mustinverse = false;
}


RechNodeSubQuery::~RechNodeSubQuery()
{
	if (fInd != nil)
	{
		fInd->ReleaseValid();
		fInd->Release();
	}
	if (fOtherInd != nil)
	{
		fOtherInd->ReleaseValid();
		fOtherInd->Release();
	}
	if (data != nil)
		data->Release();
}


RechNodeSubQuery* RechNodeSubQuery::Clone()
{
	RechNodeSubQuery* clone = new RechNodeSubQuery(nil);
	clone->fUnSolved = fUnSolved;
	clone->fIsIndexed = fIsIndexed;
	clone->fInd = RetainRefCountable(fInd);
	if (clone->fInd != nil)
		clone->fInd->IncAskForValid();
	clone->fOtherInd = RetainRefCountable(fOtherInd);
	if (clone->fOtherInd != nil)
		clone->fOtherInd->IncAskForValid();
	clone->data = RetainRefCountable(data);
	clone->rechtyp = rechtyp;
	clone->fOptions = fOptions;
	clone->fFinalPath.CopyFrom(fFinalPath);
	clone->fPath.CopyFrom(fPath);

	return clone;
}


void RechNodeSubQuery::SetOneLevelPath(JoinRefCouple& couple, BaseTaskInfo* context) 
{ 
	fPath.Clear();
	fFinalPath.Clear();
	fPath.AddJoin(couple); 
	fFinalPath.AddJoin(couple); 
	fUnSolved = false;
	if (fInd != nil)
	{
		fInd->ReleaseValid();
		fInd->Release();
	}
	if (fOtherInd != nil)
	{
		fOtherInd->ReleaseValid();
		fOtherInd->Release();
	}
	if (couple.fNumTable1 == fTargetTable && couple.fNumInstance1 == fTargetInstance)
	{
		fInd = context->GetBase()->FindAndRetainIndexSimple(couple.fNumTable2, couple.fNumField2, true, true, context);
		fOtherInd = context->GetBase()->FindAndRetainIndexSimple(couple.fNumTable1, couple.fNumField1, true, true, context);
	}
	else
	{
		fInd = context->GetBase()->FindAndRetainIndexSimple(couple.fNumTable1, couple.fNumField1, true, true, context);
		fOtherInd = context->GetBase()->FindAndRetainIndexSimple(couple.fNumTable2, couple.fNumField2, true, true, context);
	}
	fIsIndexed = fInd != nil;

};


void RechNodeSubQuery::ResetIndex(BaseTaskInfo* context)
{
	if (fInd != nil)
	{
		fInd->ReleaseValid();
		fInd->Release();
	}
	fInd = nil;
	if (fOtherInd != nil)
	{
		fOtherInd->ReleaseValid();
		fOtherInd->Release();
	}
	fOtherInd = nil;
	if (fFinalPath.GetCount() > 0)
	{
		JoinRefCouple& couple = fFinalPath.GetCoupleRef(fFinalPath.GetCount()-1);
		if (couple.fNumTable1 == fTargetTable && couple.fNumInstance1 == fTargetInstance)
		{
			fInd = context->GetBase()->FindAndRetainIndexSimple(couple.fNumTable2, couple.fNumField2, true, true, context);
			fOtherInd = context->GetBase()->FindAndRetainIndexSimple(couple.fNumTable1, couple.fNumField1, true, true, context);
		}
		else
		{
			fInd = context->GetBase()->FindAndRetainIndexSimple(couple.fNumTable1, couple.fNumField1, true, true, context);
			fOtherInd = context->GetBase()->FindAndRetainIndexSimple(couple.fNumTable2, couple.fNumField2, true, true, context);
		}
	}
	fIsIndexed = fInd != nil;
}



void RechNodeSubQuery::SolvePath(sLONG QueryTarget, sLONG QueryInstance, BaseTaskInfo* context)
{
	SmallArrayOfBoolean Deja; // limite de 2048 jointures simultanees
	sLONG i,nb = fPath.GetCount();
	assert(nb<2048);
	for (i=0; i<nb; i++)
	{
		Deja[i] = false;
	}

	if (BuildFinalPath(QueryTarget, QueryInstance, Deja, GetTarget(), GetInstance()))
	{
		// on a trouve ce qui est necessaire pour construire le chemin final
		fUnSolved = false;
	}
	else
	{
		if (context->GetBase()->GetCachedRelPath(GetTarget(), QueryTarget, fFinalPath))
		{
			fUnSolved = false;
		}
		else
		{
			RelationPath rp;
			VError err;
			vector<uBYTE> none;
			if (rp.BuildPath(context, QueryTarget, GetTarget(), err, false, false, none))
			{
				if (rp.ContainsSubTables())
					fContainsSubTables = true;
				err = rp.CopyInto(fFinalPath);
				fUnSolved = false;
				if (!fContainsSubTables)
					context->GetBase()->AddCachedRelPath(GetTarget(), QueryTarget, fFinalPath);
			}
			else
				fUnSolved = true;
		}
	}

	if (!fUnSolved)
	{
		JoinRefCouple* couple = fFinalPath.GetCouple(fFinalPath.GetCount()-1);
		sLONG xnumfield = couple->fNumField1;
		assert(couple->fNumTable1 == QueryTarget && couple->fNumInstance1 == 0);
		IndexInfo* ind = context->GetBase()->FindAndRetainIndexSimple(QueryTarget, xnumfield, true, true, context);
		if (ind != nil)
		{
			fIsIndexed = true;
			fInd = ind; 
		}
		fOtherInd = context->GetBase()->FindAndRetainIndexSimple(couple->fNumTable2, couple->fNumField2, true, true, context);
	}
}


uBOOL RechNodeSubQuery::IsIndexe(void)
{
	return fIsIndexed;
}

uBOOL RechNodeSubQuery::IsAllIndexe(void)
{
	Boolean subIndex = true;
	if (fRoot != nil)
		subIndex = fRoot->IsAllIndexe();
	return fIsIndexed && subIndex && fOtherInd != nil;
}


bool RechNodeSubQuery::NeedToLoadFullRec()
{
	bool result = false;
	if (fRoot != nil)
		result = fRoot->NeedToLoadFullRec();
	return result;
}


Boolean RechNodeSubQuery::BuildFinalPath(sLONG QueryTarget, sLONG QueryInstance, SmallArrayOfBoolean &Deja, sLONG target, sLONG numinstance)
{
	Boolean ok = false;
	if (target == QueryTarget && numinstance == QueryInstance)
		ok = true;
	else
	{
		sLONG from = 0;
		while (from != -1)
		{
			sLONG trouve = fPath.FindTarget(from, target, Deja, numinstance);
			if (trouve == -1)
				from = -1;
			else
			{
				sLONG newtarget;
				sLONG newtargetinstance;
				from = trouve+1;
				if (from >= fPath.GetCount())
					from = -1;
				Deja[trouve] = true;
				JoinRefCouple* couple = fPath.GetCouple(trouve);
				if (couple->fNumTable1 == target && couple->fNumInstance1 == numinstance)
				{
					JoinRefCouple invCouple;
					invCouple.fNumTable2 = couple->fNumTable1;
					invCouple.fNumField2 = couple->fNumField1;
					invCouple.fNumInstance2 = couple->fNumInstance1;
					invCouple.fNbOtherFields2 = couple->fNbOtherFields1;
					copy(&couple->fOtherFields1[0], &couple->fOtherFields1[9], &invCouple.fOtherFields2[0]);
					invCouple.fNumTable1 = couple->fNumTable2;
					invCouple.fNumField1 = couple->fNumField2;
					invCouple.fNumInstance1 = couple->fNumInstance2;
					invCouple.fNbOtherFields1 = couple->fNbOtherFields2;
					copy(&couple->fOtherFields2[0], &couple->fOtherFields2[9], &invCouple.fOtherFields1[0]);
					fFinalPath.AddJoin(invCouple);

					//fFinalPath.AddJoin(couple->fNumTable2, couple->fNumField2, couple->fNumTable1, couple->fNumField1, couple->fNumInstance2, couple->fNumInstance1);
					newtarget = couple->fNumTable2;
					newtargetinstance = couple->fNumInstance2;
				}
				else
				{
					//fFinalPath.AddJoin(couple->fNumTable1, couple->fNumField1, couple->fNumTable2, couple->fNumField2, couple->fNumInstance1, couple->fNumInstance2);
					fFinalPath.AddJoin(*couple);
					newtarget = couple->fNumTable1;
					newtargetinstance = couple->fNumInstance1;
				}
				if (BuildFinalPath(QueryTarget, QueryInstance, Deja, newtarget, newtargetinstance))
				{
					// plus rien a faire tout est construit
					ok = true;
				}
				else
				{
					fFinalPath.RemoveLast(); // on retire le chemin intermediaire trouve
					Deja[trouve] = false; // et on libere son acces pour d'autres essais
				}
			}
		}
	}
	return ok;
}


void RechNodeSubQuery::BuildQueryPathSeq(Boolean inBefore, sLONG& curvar, CDB4DQueryPathNode* parent, Base4D* bd, VString& outString, const CDB4DQueryPathModifiers* inModifiers, Boolean first) 
{
	sLONG arrnum;
	Table* target = bd->RetainTable(GetTarget());
	BuildQueryPath(inBefore, target, bd, -2, parent, curvar, arrnum, inModifiers);
	if (target != nil)
		target->Release();

	if (!fUnSolved)
	{
		JoinRefCouple* co = fFinalPath.GetCouple(0);
		VString sarrname;
		BuildArrayName(arrnum, sarrname, inModifiers);
		BuildAddFieldName(bd, co->fNumTable1, co->fNumField1, outString, inModifiers, co->fNumInstance1);
		outString += L" in ";
		outString += sarrname;
	}
}


CDB4DQueryPathNode* RechNodeSubQuery::BuildQueryPath(Boolean inBefore, Table* target, Base4D* bd, sLONG DejaSel, CDB4DQueryPathNode* parent, sLONG& curvar, sLONG &outvarnum, const CDB4DQueryPathModifiers* inModifiers)
{
	CDB4DQueryPathNode* result = nil;
	sLONG curarr;
	sLONG cursel;
	Boolean finalisarray = false;

	if (DejaSel == -2)
	{
		finalisarray = true;
		DejaSel = 0;
	}

	if (GetTarget() == 0)
	{
		if (fRoot != nil)
			result = fRoot->BuildQueryPath(inBefore, target, bd, DejaSel, parent, curvar, outvarnum, inModifiers);
	}
	else
	{
		VString smess = L"SubQuery on ";
		BuildAddTableName(bd, GetTarget(), smess, inModifiers, GetInstance());

		Table* subtarget = bd->RetainTable(GetTarget());
		if (subtarget != nil && !fUnSolved)
		{
			result = new VDB4DQueryPathNode(parent, smess, inModifiers, inBefore);
			fRoot->BuildQueryPath(false, subtarget, bd, false, result, curvar, cursel, inModifiers);
			sLONG i,nb = fFinalPath.GetCount();
			for (i=0; i<nb; i++)
			{
				VString sarrname, ssubmess, sfieldname, selname;
				JoinRefCouple* co = fFinalPath.GetCouple(i);
				curvar++;
				curarr = curvar;
				BuildArrayName(curarr, sarrname, inModifiers);
				ssubmess += sarrname;
				ssubmess += L" := Get Distinct Values of ";
				BuildAddFieldName(bd, co->fNumTable2, co->fNumField2, ssubmess, inModifiers, co->fNumInstance2);
				if (inModifiers->isTempVarDisplay())
				{
					ssubmess += L" (in ";
					BuildSelectionName(cursel, selname, inModifiers);
					ssubmess += selname;
					ssubmess += L")";
				}
				CDB4DQueryPathNode* sub = new VDB4DQueryPathNode(result, ssubmess, inModifiers, false);

				if (finalisarray && (i == (nb-1)))
				{
					// on s'arrete sur le dernier tableau sans generer la selection
				}
				else
				{
					VString smess2, selname2;
					curvar++;
					cursel = curvar;

					IndexInfo* ind = bd->FindAndRetainIndexSimple(co->fNumTable1, co->fNumField1, false, false);
					if (ind != nil)
					{
						if (inModifiers->isVerbose())
							BuildBeginIndexString(ind, cursel, smess2, inModifiers);
						else
							BuildBeginIndexStringNonVerbose(ind, cursel, smess2, inModifiers);
						smess2 += L" in ";
						smess2 += sarrname;
						ind->Release();
					}
					else
					{
						VString sformula;
						BuildAddFieldName(bd, co->fNumTable1, co->fNumField1, sformula, inModifiers, co->fNumInstance1);
						sformula += L" in ";
						sformula += sarrname;
						sLONG selnum = 0;
						if (i == 0)
							selnum = DejaSel;
						BuildSeqStringNonVerbose(bd, co->fNumTable1, selnum, cursel, sformula, smess2, inModifiers, co->fNumInstance1);
					}
						
					sub = new VDB4DQueryPathNode(result, smess2, inModifiers, false);		
				}
			}
			
			if (finalisarray)
				outvarnum = curarr;
			else
				outvarnum = cursel;
		}
		QuickReleaseRefCountable(subtarget);
	}

	return result;
}


void RechNodeSubQuery::FullyDescribe(Base4D* bd, VString& result)
{
	VString s;
	Describe(bd, s);
	result += s;
	if (fRoot != nil)
	{
		result += L" with filter {";
		fRoot->FullyDescribe(bd, s);
		result += s;
		result += L"}";
	}
}


void RechNodeSubQuery::AdjusteIntl(VIntlMgr* inIntl)
{
	fOptions.SetIntlManager(inIntl);
	if (fRoot != nil)
		fRoot->AdjusteIntl(inIntl);
}


void RechNodeSubQuery::Describe(Base4D* bd, VString& result)
{
	VString s,s2,s3;

	if (GetTarget() != 0)
	{
		s.AppendString(L"Join on Table : ");
		Table* tt = bd->RetainTable(GetTarget());
		if (tt != nil)
		{
			tt->GetName(s2);
			if (GetInstance() != 0)
				s2 += L"("+ToString(GetInstance())+L")";
			tt->Release();
		}
		s.AppendString(s2);
		s+=L"  :  ";

		sLONG i,nb = fFinalPath.GetCount();
		for (i=0; i<nb; i++)
		{
			JoinRefCouple* co = fFinalPath.GetCouple(i);
			if (i != 0)
				s.AppendString(L"  ,  ");
			{
				Table* t = bd->RetainTable(co->fNumTable1);
				Field* f = nil;
				s2 = L"<Null field>";
				if (t != nil)
				{
					f = t->RetainField(co->fNumField1);
					if (f != nil)
					{
						t->GetName(s2);
						if (co->fNumInstance1 != 0)
							s2 += L"("+ToString(co->fNumInstance1)+L")";
						s2.AppendString(L".");
						f->GetName(s3);
						s2.AppendString(s3);
						f->Release();
					}
					t->Release();
				}
			}
			s.AppendString(s2);
			s.AppendString(L" = ");

			{
				Table* t = bd->RetainTable(co->fNumTable2);
				Field* f = nil;
				s2 = L"<Null field>";
				if (t != nil)
				{
					f = t->RetainField(co->fNumField2);
					if (f != nil)
					{
						t->GetName(s2);
						if (co->fNumInstance2 != 0)
							s2 += L"("+ToString(co->fNumInstance2)+L")";
						s2.AppendString(L".");
						f->GetName(s3);
						s2.AppendString(s3);
						f->Release();
					}
					t->Release();
				}
			}
			s.AppendString(s2);

			for (sLONG j = 0; j < co->fNbOtherFields1; j++)
			{
				s += L" & ";
				Table* t = bd->RetainTable(co->fNumTable1);
				Field* f = nil;
				s2 = L"<Null field>";
				if (t != nil)
				{
					f = t->RetainField(co->fOtherFields1[j]);
					if (f != nil)
					{
						t->GetName(s2);
						if (co->fNumInstance1 != 0)
							s2 += L"("+ToString(co->fNumInstance1)+L")";
						s2.AppendString(L".");
						f->GetName(s3);
						s2.AppendString(s3);
						f->Release();
					}
					t->Release();
				}
				s += s2;
				s += L" = ";

				t = bd->RetainTable(co->fNumTable2);
				f = nil;
				s2 = L"<Null field>";
				if (t != nil)
				{
					f = t->RetainField(co->fOtherFields2[j]);
					if (f != nil)
					{
						t->GetName(s2);
						if (co->fNumInstance2 != 0)
							s2 += L"("+ToString(co->fNumInstance2)+L")";
						s2.AppendString(L".");
						f->GetName(s3);
						s2.AppendString(s3);
						f->Release();
					}
					t->Release();
				}
				s += s2;

			}

		}
	}

	result = s;
}


void RechNodeSubQuery::DisposeTree(void)
{
	if (fRoot != nil) 
		fRoot->DisposeTree();
	delete fRoot;
	fRoot = nil;
}


void RechNodeSubQuery::DisplayTree(Base4D* bd, sLONG level, VString* result, VValueBag* bagResult)
{
	VString s,s2,s3;

	if (GetTarget() == 0)
	{
		if (fRoot != nil)
			fRoot->DisplayTree(bd, level, result, bagResult);
	}
	else
	{
		FillLevelString(level, s);

		Describe(bd, s2);
		if (bagResult != nil)
			bagResult->SetString(d4::item, s2);
		s += s2;
		AddCR(s);

		if (result == nil)
			DebugMsg(s);
		else
			*result += s;

		if (fRoot != nil)
		{
			BagElement subbag(*bagResult, d4::subquery);
			fRoot->DisplayTree(bd, level+2, result, subbag);
		}
	}
}


VError RechNodeSubQuery::GenereData(VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, VValueBag* curdesc, OptimizedQuery *query)
{
	VError err = VE_OK;

	Base4D* db = context->GetBase();
	DB4DArrayOfValues* temp = nil;

	//## pour tester performance
	//InProgress = nil;

	if (fUnSolved)
		err = -1;
	else
	{
		sLONG i,nb = fFinalPath.GetCount();
		assert(nb>0);

		for (i=0; i<nb && err == VE_OK; i++)
		{
			JoinRefCouple* co = fFinalPath.GetCouple(i);
			Table* subtarget = db->RetainTable(co->fNumTable2);
			if (subtarget == nil)
			{
				// on arrete tout
				err = VE_DB4D_WRONGTABLEREF;
			}
			else
			{
				Field* subcri = subtarget->RetainField(co->fNumField2);
				if (subcri == nil)
				{
					// on arrete tout
					err = VE_DB4D_WRONGFIELDREF;
				}
				else
				{
					Selection* sel = nil;
					if (i == 0 && fRoot == nil)
					{
						sel = subtarget->GetDF()->AllRecords(context, err);
					}
					else
					{
						OptimizedQuery subq;
						SearchTab subqx(subtarget);
						if (query != nil)
							subq.DescribeQuery(query->IsQueryDescribed());

						if (i == 0)
						{
							subq.SetTarget(subtarget, co->fNumInstance2);
							subq.SetRoot(fRoot);
						}
						else
						{
							JoinRefCouple* co2 = fFinalPath.GetCouple(i-1);
							Table* subtarget2 = db->RetainTable(co2->fNumTable1);
							assert(subtarget2 == subtarget);
							Field* subcri2 = subtarget2->RetainField(co2->fNumField1);
							subqx.AddSearchLineArray(subcri2, DB4D_Equal, temp, fOptions);
							VError err2 = subq.AnalyseSearch(&subqx, context);
							subcri2->Release();
							subtarget2->Release();
							assert(err2 == VE_OK);
						}

						sel = subq.Perform((Bittab*)nil, InProgress, context, err, HowToLock, 0);
						if (i == 0)
							subq.SetRoot(nil);
						if (query != nil && query->IsQueryDescribed())
						{
							curdesc->AddElement(QueryKeys::steps, subq.GetExecutionDescription());
						}
					}

					if (i!=0)
						temp->Release();

					temp = nil;

					QuickDB4DArrayOfValues* temp2 = CreateDB4DArrayOfDirectValues(subcri->GetTyp(), fOptions);
					if (temp2 != nil)
					{
						if (sel != nil)
						{
							err = sel->QuickGetDistinctValues(subcri, temp2, context, InProgress, fOptions);
							if (err == VE_OK)
							{
								temp = temp2->GenerateConstArrayOfValues();
							}
							temp2->Release();

							sel->Release();
						}
					}
					else
					{
						err = ThrowBaseError(VE_DB4D_WRONGFIELDREF);
						QuickReleaseRefCountable(sel);
					}

					subcri->Release();
				}

				subtarget->Release();
			}

			if (i==(nb-1))
				data = temp;
		}
	}

	if (err != VE_OK)
	{
		if (temp != nil)
			temp->Release();
		data = nil;
	}

	return err;
}


VError RechNodeSubQuery::Perform(OptimizedQuery *query, Bittab **super, Bittab* filtre, VDB4DProgressIndicator* InProgress,  VValueBag* curdesc,
								 BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, Bittab &Nulls, RechNode* &transformation)
{
	Bittab *b = nil;
	VError err = VE_OK;
	transformation = this;

	//## pour tester performance
	//InProgress = nil;

	if (fUnSolved)
		err = -1;
	else
	{
		if (fInd != nil)
		{
			bool alternateway = false;
			bool seqway = false;
			sLONG nbmain = 0;
			if (fOtherInd != nil)
			{
				if (fRoot != nil && !fRoot->IsIndexe())
				{
					nbmain = fInd->GetTargetTable()->GetDF()->GetNbRecords(context, false);
					if (filtre != nil)
					{
						sLONG ratio = nbmain / 10;
						sLONG ratioseq = nbmain / 200;
						if (filtre->Compte() < ratio)
						{
							//alternateway = true; // je le retire pour l'instant, l'optimisation doit aussi porte sur JoinWithOtherIndexNotNull
						}
						if (filtre->Compte() < ratioseq)
						{
							seqway = true;
						}
					}
				}
			}

			uLONG start = query->GetStartingTime();
			VValueBag* subline = nil;
			if (query->IsQueryDescribed())
			{
				VString s;
				Describe(query->GetTargetFile()->GetOwner(), s);
				if (alternateway)
					s += "  (Join partly sequential)";
				if (seqway)
					s += "  (Join sequential)";
				subline = query->AddToDescription(curdesc, s, start, nil);
			}

			if (alternateway)
			{
				Bittab bOther;
				Table* subtarget = fOtherInd->GetTargetTable();
				sLONG nbMaxOther = subtarget->GetDF()->GetMaxRecords(context);
				bOther.aggrandit(nbMaxOther);
				err = fOtherInd->JoinWithOtherIndexNotNull(fInd, nil, filtre, &bOther, context, fOptions, InProgress);
				if (err == VE_OK)
				{
					OptimizedQuery subq;
					SearchTab subqx(subtarget);
					if (query != nil)
						subq.DescribeQuery(query->IsQueryDescribed());
					subq.SetTarget(subtarget, fTargetInstance);
					subq.SetRoot(fRoot);

					Selection* sel = subq.Perform(&bOther, InProgress, context, err, HowToLock, 0);
#if debuglr
					sLONG selcount = sel->GetQTfic();
#endif
					subq.SetRoot(nil);
					if (query != nil && query->IsQueryDescribed())
					{
						curdesc->AddElement(QueryKeys::steps, subq.GetExecutionDescription());
					}
					Bittab* bOtherFiltre = sel->GenereBittab(context, err);
					if (err == VE_OK)
					{
						b = new Bittab();
						b->aggrandit(fInd->GetTargetTable()->GetDF()->GetMaxRecords(context));
						err = fInd->JoinWithOtherIndexNotNull(fOtherInd, filtre, bOtherFiltre, b, context, fOptions, InProgress);
						*super = b;
					}
					QuickReleaseRefCountable(bOtherFiltre);
				}
			}
			else if (seqway)
			{
				b = new Bittab();
				b->aggrandit(fInd->GetTargetTable()->GetDF()->GetMaxRecords(context));
				sLONG i = filtre->FindNextBit(0);
				while (i != -1)
				{
					uBOOL ok = 2;
					{
						FicheInMem* rec = fInd->GetTargetTable()->GetDF()->LoadRecord(i, err, HowToLock, context, false);
						if (rec != nil)
						{
							ok = PerformFullSeq(fInd->GetTargetTable()->GetNum(), rec, context, InProgress, context, HowToLock, exceptions, limit, nil);
							rec->Release();
						}
					}
					if (ok != 2)
					{
						if (ok)
							b->Set(i);
							
					}
					i = filtre->FindNextBit(i+1);
				}
				*super = b;
			}
			else
			{
				if (data == nil)
					err = GenereData(InProgress, context, HowToLock, subline, query);
				if (err == VE_OK)
				{
					b=fInd->FindKeyInArray(data, context, err, InProgress, fOptions, nil);
					if (err == VE_OK)
					{
						OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
						BitSel* sel = fInd->GetHeader()->RetainNulls(curstack, err, context);
						if (err == VE_OK && sel != nil)
						{
							err = Nulls.Or(sel->GetBittab(), true);
						}
						if (err == VE_OK)
						{
							if (filtre != nil) 
								err = b->And(filtre);
						}
						if (sel != nil)
							sel->Release();
					}

					if (mustinverse)
					{
						b->aggrandit(query->GetMaxRecords());
						err=b->Invert();
						query->GetTargetFile()->GetDF()->TakeOffBits(b, context);
					}

					if (err == VE_OK && HowToLock == DB4D_Keep_Lock_With_Transaction)
					{
						if (context != nil)
						{
							Transaction* trans = context->GetCurrentTransaction();
							if (trans != nil)
							{
								err = trans->TryToLockSel(query->GetTargetFile()->GetDF(), b, exceptions);
								if (err != VE_OK)
								{
									ReleaseRefCountable(&b);
								}
							}
						}
					}

					*super = b;
				}
			}


			if (query->IsQueryDescribed())
			{
				subline->SetLong(QueryKeys::time, VSystem::GetCurrentTime() - start);
				subline->SetLong(QueryKeys::recordsfounds, b == nil ? 0 : b->Compte());
			}
		}
		else
		{
			err = RechNode::Perform(query, super, filtre, InProgress, curdesc, context, HowToLock, exceptions, limit, Nulls, transformation);
		}
	}

	return(err);
}



uBOOL RechNodeSubQuery::PerformSeq(sLONG nf, FicheInMem* curfic, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, 
								   BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, LocalEntityModel* model)
{
	uBOOL ok = false;
	ValPtr cv;
	VError err = VE_OK;

	if (!fUnSolved)
	{
		if (data == nil)
			err = GenereData(InProgress, context, HowToLock, nil, nil);
		if (err == VE_OK)
		{
			cv=nil;
			assert(fFinalPath.GetCount()>0);
			sLONG numfield = fFinalPath.GetCouple(fFinalPath.GetCount()-1)->fNumField1;
			assert(fFinalPath.GetCouple(fFinalPath.GetCount()-1)->fNumTable1 == nf);
			cv=curfic->GetNthField(numfield, err, false, true);

			if (cv!=nil)
			{
				if (cv->IsNull())
					ok = 2;
				else
					ok = data->Find(*cv, fOptions);
			} // du if cv!=nil
		}
	}

	return(ok);

}


uBOOL RechNodeSubQuery::PerformFullSeq(sLONG nf, FicheInMem* curfic, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, 
										BaseTaskInfo* context, DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, LocalEntityModel* model)
{
	uBOOL ok = false;
	VError err = VE_OK;
	if (!fUnSolved)
	{
		if (fOtherInd == nil)
			ok = PerformSeq(nf, curfic, tfb, InProgress, context, HowToLock, exceptions, limit, model);
		else
		{
			BTitemIndex* val = fInd->BuildKey(curfic, err);
			Bittab* b = fOtherInd->Fourche(val, false, val, false, context, err, InProgress, fOptions, nil, nil);
			Table* otherTarget = fOtherInd->GetTargetTable();

			if (b != nil)
			{
				sLONG i = b->FindNextBit(0);
				while (i != -1)
				{
					if (fRoot == nil)
						ok = true;
					else
					{
						FicheInMem* rec = otherTarget->GetDF()->LoadRecord(i, err, HowToLock, context, false);
						if (rec != nil)
						{
							ok = fRoot->PerformFullSeq(otherTarget->GetNum(), rec, context, InProgress, context, HowToLock, nil, 0, nil);
							rec->Release();
						}
					}
					if (ok != 2)
					{
						if (ok)
							break;
					}
					i = b->FindNextBit(i+1);
				}
				b->Release();
			}
		}
	}

	return(ok);
}



uBOOL RechNodeSubQuery::PerformSeq(FicheOnDisk* ficD, BaseTaskInfo *tfb, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, 
								   DB4D_Way_of_Locking HowToLock, Bittab *exceptions, sLONG limit, LocalEntityModel* model)
{
	uBOOL ok = false;
	sLONG typondisk;
	VError err = VE_OK;
	if (!fUnSolved)
	{
		assert(fFinalPath.GetCount()>0);
		sLONG numfield = fFinalPath.GetCouple(fFinalPath.GetCount()-1)->fNumField1;
		void* datax = (void*)(ficD->GetDataPtrForQuery(numfield, &typondisk, false));

		if (data == nil)
			err = GenereData(InProgress, context, HowToLock, nil, nil);

		if (datax!=nil && data != nil)
		{
			if (rechtyp == -1)
			{
				rechtyp = -2;

				Field* cri = nil;
				VRefPtr<Table> assoctable(ficD->GetOwner()->RetainTable(), false);
				if (assoctable.Get() != nil)
					cri = assoctable->RetainField(numfield);
				if (cri != nil)
				{
					rechtyp = cri->GetTyp();
					cri->Release();
				}

			}

			if (rechtyp == -2)
				ok = false;
			else
			{
				if (typondisk == rechtyp)
				{
					ok = data->FindWithDataPtr(datax, fOptions);
				}
				else
				{
					ValPtr cv = NewValPtr(rechtyp, datax, typondisk, ficD->GetOwner(), context);
					if (cv != nil)
					{
						ok = data->Find(*cv, fOptions);
						delete cv;
					}
				}
			}
		} 
		else
			ok = 2;
	}

	return(ok);

}






							/* ====================================================================== */






OptimizedQuery::~OptimizedQuery()
{
	if (root != nil)
	{ 
		root->DisposeTree();
		delete root;
	}

	if (fExecutionDescription != nil)
	{
		fExecutionDescription->Release();
	}

	QuickReleaseRefCountable(fQueryPlan);
}



// super chiant a faire, je suis content d'en avoir fini. (L.R)
RechNode* OptimizedQuery::xBuildNodes(SearchTab *model, RechNode* dejaleft)
{
	RechNode* rn = nil;
	RechNode* rn2;
	RechToken* rt;
	RechNodeSeq* rnseq;
	RechNodeWithArraySeq* rnarrayseq;
	RechNodeScript *rnscript;
	RechNodeOperator *rnoper;
	RechNodeBaseJoin* rnjoin;
	RechNodeEmSeq* rnEmSeq;
	RechNodeRecordExist* rnRecExist;
	RechTokenJoin* xrt;
	RechTokenEmJoin* yrt;
	RechNodeExistingSelection* rnsel;
	Selection* sel;
	sLONG seltarget;
	
	sWORD tt;
	
	if (VTask::GetCurrentFreeStackSize() < minStackSize)
	{
		ThrowBaseError(VE_DB4D_STACK_OVERFLOW);
		return nil;
	}

	rt = model->GetNextToken();
	if ((rt == nil) || (rt->GetTyp() == r_ParentD))
	{
		 // on ferme la parenthese ou on arrive a la fin de la query 
		 rn = dejaleft;
	}
	else
	{
		tt = rt->GetTyp();	
		{
			if (tt == r_Line || tt == r_LineEM || tt == r_Script || tt == r_LineArray || tt == r_LineEMArray || tt == r_ParentG || tt == r_Not || tt == r_LineJoin || tt == r_LineEMJoin 
				|| tt == r_Sel || tt == r_EmSel || tt == r_EMComp || tt == r_RecordExist || tt == r_JSScript || tt == r_RecordEntityExist)
			{
				if (tt == r_Not)
				{
					rn2 = xBuildNodes(model, (RechNode*)-1);
					if (rn2 == (RechNode*)-1)
					{
						rn2 = nil;
						rn = nil;
						errQuery = kErr4DQueryMissingArgument;
					}
					rnoper = new RechNodeNot;
					rnoper->SetLeft(rn2);
					rn2 = rnoper;
				}
				else
				{
					if (tt == r_ParentG)
					{
						rn2 = xBuildNodes(model, nil);
					}
					else
					{
						switch (tt)
						{
							case r_Sel:
								sel = ((RechTokenSel*)rt)->fSel;
								assert(sel != nil);
								seltarget = sel->GetParentFile()->GetNum();
								rnsel = new RechNodeExistingSelection(sel);
								if (seltarget != nf || ((RechTokenSel*)rt)->fNumInstance != fDefaultInstance)
								{	
									rnsel->SetTarget(seltarget, ((RechTokenSel*)rt)->fNumInstance);
								}
								
								rn2 = rnsel;
								break;

							case r_EmSel:
								{
									LocalEntityCollection* EMsel = dynamic_cast<LocalEntityCollection*>(((RechTokenEmSel*)rt)->fSel);
									sel = EMsel->GetSel();
									assert(sel != nil);
									seltarget = sel->GetParentFile()->GetNum();
									rnsel = new RechNodeExistingSelection(sel);
									if (seltarget != nf || ((RechTokenEmSel*)rt)->fNumInstance != fDefaultInstance)
									{
										rnsel->SetTarget(seltarget, ((RechTokenEmSel*)rt)->fNumInstance);
									}

									rn2 = rnsel;
								}
								break;

							case r_Line:
								rnseq = new RechNodeSeq;
								rnseq->rt = (RechTokenSimpleComp*)rt;
								if (((RechTokenSimpleComp*)rt)->numfile != nf || ((RechTokenSimpleComp*)rt)->numinstance != fDefaultInstance)
									rnseq->SetTarget(((RechTokenSimpleComp*)rt)->numfile, ((RechTokenSimpleComp*)rt)->numinstance);
								rn2 = rnseq;
							break;

							case r_LineEM:
								rnseq = new RechNodeSeq;
								rnseq->rt = ((RechTokenEMSimpleComp*)rt)->CopyIntoRechTokenSimpleComp();
								if (rnseq->rt->numfile != nf || rnseq->rt->numinstance != fDefaultInstance)
									rnseq->SetTarget(rnseq->rt->numfile, rnseq->rt->numinstance);
								rn2 = rnseq;
							break;

							case r_LineArray:
								rnarrayseq = new RechNodeWithArraySeq;
								rnarrayseq->rt = (RechTokenArrayComp*)rt;
								rnarrayseq->values = ((RechTokenArrayComp*)rt)->values;
								if (rnarrayseq->values != nil)
									rnarrayseq->values->Retain();
								if (((RechTokenArrayComp*)rt)->numfile != nf || ((RechTokenSimpleComp*)rt)->numinstance != fDefaultInstance)
									rnarrayseq->SetTarget(((RechTokenArrayComp*)rt)->numfile, ((RechTokenSimpleComp*)rt)->numinstance);
								rn2 = rnarrayseq;
								break;

							case r_LineEMArray:
								rnarrayseq = new RechNodeWithArraySeq;
								rnarrayseq->rt = ((RechTokenEmArrayComp*)rt)->CopyIntoRechTokenArrayComp();
								rnarrayseq->values = rnarrayseq->rt->values;
								if (rnarrayseq->values != nil)
									rnarrayseq->values->Retain();
								if (rnarrayseq->rt->numfile != nf || rnarrayseq->rt->numinstance != fDefaultInstance)
									rnarrayseq->SetTarget(rnarrayseq->rt->numfile, rnarrayseq->rt->numinstance);
								rn2 = rnarrayseq;
								break;

							case r_Script:
								rnscript = new RechNodeScript;
								rnscript->rt = (RechTokenScriptComp*)rt;
								rnscript->SetTarget(((RechTokenScriptComp*)rt)->numtable, 0);
								rn2 = rnscript;
								break;

							case r_LineJoin:
								xrt = (RechTokenJoin*)rt;
								rnjoin = new RechNodeBaseJoin(xrt->numfile, xrt->numfield, xrt->numfileOther, xrt->numfieldOther, xrt->fOptions, xrt->numinstance, xrt->numinstanceOther);
								rn2 = rnjoin;
								break;

							case r_LineEMJoin:
								{
									yrt = (RechTokenEmJoin*)rt;
									sLONG numfile = 0, numfileother = 0;
									LocalEntityModel* locmodel = dynamic_cast<LocalEntityModel*>(yrt->fAtt->GetModel());
									LocalEntityModel* locmodelother = dynamic_cast<LocalEntityModel*>(yrt->fAttOther->GetModel());
									if (locmodel != nil)
										numfile = locmodel->GetMainTable()->GetNum();
									if (locmodelother != nil)
										numfileother = locmodelother->GetMainTable()->GetNum();

									rnjoin = new RechNodeBaseJoin(numfile, yrt->fAtt->GetFieldPos(), numfileother, yrt->fAttOther->GetFieldPos(), yrt->fOptions, yrt->numinstance, yrt->numinstanceOther);
									rn2 = rnjoin;
								}
								break;

							case r_EMComp:
								rnEmSeq = new RechNodeEmSeq;
								rnEmSeq->rt = (RechTokenEmComp*)rt;
								rnEmSeq->CalcTarget(nf);
								rn2 = rnEmSeq;
								break;

							case r_RecordExist:
								rnRecExist = new RechNodeRecordExist;
								rnRecExist->rt = (RechTokenRecordExist*)rt;
								if (((RechTokenRecordExist*)rt)->numfile != nf || ((RechTokenRecordExist*)rt)->numinstance != fDefaultInstance)
									rnRecExist->SetTarget(((RechTokenRecordExist*)rt)->numfile, ((RechTokenRecordExist*)rt)->numinstance);
								rn2 = rnRecExist;
								break;

							case r_RecordEntityExist:
								rnRecExist = new RechNodeRecordExist;
								rnRecExist->rt = ((RechTokenEntityRecordExist*)rt)->CopyIntoRechTokenRecordExist();
								if (rnRecExist->rt->numfile != nf || rnRecExist->rt->numinstance != fDefaultInstance)
									rnRecExist->SetTarget(rnRecExist->rt->numfile, rnRecExist->rt->numinstance);
								rn2 = rnRecExist;
								break;

							case r_JSScript:
								{
									RechNodeJSScript* rnJS = new RechNodeJSScript();
									rnJS->rt = (RechTokenJavaScript*)rt;
									rn2 = rnJS;
								}
								break;

						}
					}
				}
				
				if (rn2 != nil)
				{
					if (dejaleft == (RechNode*)-1) // quand on vient d'un NOT plus haut
					{
						rn = rn2;
					}
					else
					{
						if (dejaleft == nil)
						{
							rn = xBuildNodes(model, rn2);
						}
						else
						{
							if (dejaleft->GetTyp() == NoeudOper)
							{
								((RechNodeOperator*)dejaleft)->SetRight(rn2);
								//rn2->SetParent(dejaleft);
								rn = xBuildNodes(model, dejaleft);
							}
							else
							{
								errQuery = kErr4DQueryMissingOperator;
							}
						}
					}
				}
			}
			else
			{
				if (tt == r_BoolOperator)
				{
					if (dejaleft == nil)
					{
						errQuery = kErr4DQueryMissingArgument;
						rn = nil;
					}
					else
					{
						rnoper = new RechNodeOperator();
						rnoper->OperBool = ((RechTokenBoolOper*)rt)->BoolLogic;
						rnoper->SetLeft(dejaleft);
						rnoper->SetRight(nil);
						// dejaleft->SetParent(rnoper);
						
						rn = xBuildNodes(model, rnoper);
					}
				}
			}
		}
	}
	
	return(rn);
}


RechNode* OptimizedQuery::xBuildSubQueryFrom(RechNodeArray *ListOfNodes, JoinPath& xPath)
{
	if (VTask::GetCurrentFreeStackSize() < minStackSize)
	{
		ThrowBaseError(VE_DB4D_STACK_OVERFLOW);
		return nil;
	}

	RechNode* result = nil;
	if (testAssert(ListOfNodes->GetCount()>0))
	{
		sLONG i,nb = ListOfNodes->GetCount();

		RechNode* first = (*ListOfNodes)[0];
		if (first->GetTyp() == NoeudSubQuery)
		{
			((RechNodeSubQuery*)first)->GetPathFrom(xPath);
		}

		if (nb>1)
		{
			RechNodeMultiOperator* rnop = new RechNodeMultiOperator;
			rnop->OperBool = DB4D_And;
			rnop->AddNode(first);
			result = rnop;
			for (i=1; i<nb; i++)
			{
				RechNode* xnode = (*ListOfNodes)[i];
				if (xnode->GetTyp() == NoeudSubQuery)
				{
					((RechNodeSubQuery*)xnode)->GetPathFrom(xPath);
				}
				rnop->AddNode(xnode);
			}
		}
		else
			result = first;
	}
	
	return result;
}


sLONG OptimizedQuery::xGetSubQueryTarget(RechNode* rn, sLONG& outTargetInstance)
{
	if (rn->GetTyp() == NoeudOper)
		return xGetSubQueryTarget(((RechNodeOperator*)rn)->GetLeft(), outTargetInstance);
	else
	{
		outTargetInstance = rn->GetInstance();
		return rn->GetTarget();
	}
}


RechNode* OptimizedQuery::xStripSubQuery(RechNode* rn)
{
	if (rn->GetTyp() == NoeudSubQuery)
	{
		RechNode* rn2 = ((RechNodeSubQuery*)rn)->GetRoot();
		delete rn;
		return rn2;
	}
	else 
		return rn;
}


void OptimizedQuery::xAddToSubQuery(RechNodeArray *ListOfNodes, sLONG numtable, RechNode* rn, sLONG numinstance)
{
	if (numtable == -1)
	{
		ListOfNodes->Add(rn);
	}
	else
	{
		sLONG i,nb = ListOfNodes->GetCount();
		RechNode* found = nil;
		for (i=0; i<nb; i++)
		{
			RechNode* sub = (*ListOfNodes)[i];
			if (sub->GetTyp() == NoeudSubQuery)
			{
				if (sub->GetTarget() == numtable && sub->GetInstance() == numinstance)
				{
					RechNodeSubQuery* xsub = (RechNodeSubQuery*)sub;
					RechNode* root = xsub->GetRoot();
					assert(root != nil);
					RechNodeMultiOperator* rnoper;
				
					if (root->GetTyp() == NoeudMultiOper && (((RechNodeMultiOperator*)root)->OperBool == DB4D_And) )
					{
						rnoper = (RechNodeMultiOperator*)root;
					}
					else
					{
						rnoper = new RechNodeMultiOperator;
						rnoper->OperBool = DB4D_And;
						rnoper->AddNode(root);
					}
					rnoper->AddNode(rn);
					xsub->SetRoot(rnoper);
					found = sub;
					break;
				}
			}
		}

		if (found == nil)
		{
			RechNodeSubQuery* xsub = new RechNodeSubQuery(rn);
			if (xsub == nil)
			{
				// not enough mem
			}
			else
			{
				xsub->SetTarget(numtable, numinstance);
				ListOfNodes->Add(xsub);
			}
		}

	}
}

typedef pair<RechNodeArray*, JoinPath*> pairpath;
typedef map<RechNode*, pairpath, less<RechNode*>, cache_allocator<pair<RechNode*const, pairpath> > > mappath;

void OptimizedQuery::xBuildSubQueries(RechNode* rn, RechNodeArray *deja, JoinPath& xPath)
{
	RechNodeOperator *rnoper;

	if (VTask::GetCurrentFreeStackSize() < minStackSize)
	{
		ThrowBaseError(VE_DB4D_STACK_OVERFLOW);
		return;
	}

	if (rn != nil)
	{
		if (rn->GetTyp() == NoeudMultiOper)
		{
			RechNodeMultiOperator* rnop = (RechNodeMultiOperator*)rn;
			ArrayNode& Nodes = rnop->GetArrayNodes();
			if (rnop->OperBool == DB4D_OR)
			{
				assert(Nodes.GetCount() > 1);
				mappath allpath;
				Boolean OKSingle = true;
				ArrayNode::Iterator cur = Nodes.First(), end = Nodes.End();
				for (;cur != end; cur++)
				{
					if (testAssert(*cur != nil))
					{
						RechNode* nod = *cur;

						RechNodeArray* newdeja = new RechNodeArray;
						JoinPath* newpath = new JoinPath;
						xBuildSubQueries(nod, newdeja, *newpath);
						allpath[nod] = pairpath(newdeja, newpath);
					}
				}

				cur = Nodes.First();
				cur++;
				for (;cur != end; cur++)
				{
					RechNode* nod = *cur;
					ArrayNode::Iterator cur2 = Nodes.First();
					for (;cur2 != cur; cur2++)
					{
						RechNode* nod2 = *cur2;

						RechNodeArray* newdejaleft = allpath[nod].first;
						JoinPath* newpathleft = allpath[nod].second;
						RechNodeArray* newdejaright = allpath[nod2].first;
						JoinPath* newpathright = allpath[nod2].second;

						if (newdejaleft->GetCount() == 1 && newdejaright->GetCount() == 1 && newpathleft->GetCount() == 0 && newpathright->GetCount() == 0)
						{
							sLONG numinstanceleft, numinstanceright;
							sLONG numtableleft = xGetSubQueryTarget((*newdejaleft)[0], numinstanceleft);
							sLONG numtableright = xGetSubQueryTarget((*newdejaright)[0], numinstanceright);
							if (numtableleft == numtableright && numinstanceleft == numinstanceright)
							{
							}
							else
								OKSingle = false;
						}
						else
							OKSingle = false;
					}
				}

				RechNodeMultiOperator* rnop2 = new RechNodeMultiOperator;
				rnop2->OperBool = DB4D_OR;
				if (OKSingle)
				{
					sLONG numinstancesub;
					sLONG numtablesub = xGetSubQueryTarget((*(allpath.begin()->second.first))[0], numinstancesub);
					cur = Nodes.First();
					for (;cur != end; cur++)
					{
						RechNode* nod = *cur;
						rnop2->AddNode(xStripSubQuery((*(allpath[nod].first))[0]));
					}
					xAddToSubQuery(deja, numtablesub, rnop2, numinstancesub);
					delete rnop;
				}
				else
				{
					cur = Nodes.First();
					for (;cur != end; cur++)
					{
						RechNode* nod = *cur;
						rnop2->AddNode(xBuildSubQueryFrom(allpath[nod].first,*(allpath[nod].second)));
					}
					xAddToSubQuery(deja, -1, rnop2, 0);
				}

				mappath::iterator cur3 = allpath.begin(), end3 = allpath.end();
				for (;cur3!=end3; cur3++)
				{
					delete cur3->second.first;
					delete cur3->second.second;
				}

			}
			else
			{
				ArrayNode::Iterator cur = Nodes.First(), end = Nodes.End();
				for (;cur != end; cur++)
				{
					if (testAssert(*cur != nil))
					{
						RechNode* nod = *cur;
						xBuildSubQueries(nod, deja, xPath);
					}
				}
				delete rn;
			}
		}
		else
		{
			if (rn->GetTyp() == NoeudNot)
			{
				RechNodeNot* rnnot = (RechNodeNot*)rn;
				RechNodeArray newdeja;
				JoinPath newpath;
				xBuildSubQueries(rnnot->GetLeft(), &newdeja, newpath);

				if (newdeja.GetCount() == 1 && newpath.GetCount() == 0)
				{
					RechNode* sub = newdeja[0];
					sLONG numinstance;
					sLONG numtable = xGetSubQueryTarget(sub, numinstance);
					rnnot->SetLeft(xStripSubQuery(sub));
					xAddToSubQuery(deja, numtable, rnnot, numinstance);
				}
				else
				{
					rnnot->SetLeft(xBuildSubQueryFrom(&newdeja, newpath));
					xAddToSubQuery(deja, -1, rnnot, 0);
				}
			}
			else
			{
				if (rn->GetTyp() == NoeudOper)
				{
					rnoper = (RechNodeOperator*)rn;
					if (rnoper->OperBool == DB4D_OR) // dans le cas d'un OU, on verifie que les deux branche du OU portent sur la meme table
																			// (et celle ci seulement). Alors dans ce cas on peut les merger avec un OU et les rattacher au tableau
																			// des subqueries dans l'entree qui correspont a cette table
																			// si les deux branches ne portent pas sur la meme table, il faut alors creer deux nouvelles subqueries
																			// puis les relier par un OU et creer forcement une nouvelle entree dans le tableau
					{
						Boolean OKSingle = false;

						RechNodeArray newdejaleft;
						RechNodeArray newdejaright;
						JoinPath newpathleft;
						JoinPath newpathright;
						xBuildSubQueries(rnoper->GetLeft(), &newdejaleft, newpathleft);
						xBuildSubQueries(rnoper->GetRight(), &newdejaright, newpathright);
						if (newdejaleft.GetCount() == 1 && newdejaright.GetCount() == 1 && newpathleft.GetCount() == 0 && newpathright.GetCount() == 0)
						{
							sLONG numinstanceleft, numinstanceright;
							sLONG numtableleft = xGetSubQueryTarget(newdejaleft[0], numinstanceleft);
							sLONG numtableright = xGetSubQueryTarget(newdejaright[0], numinstanceright);
							if (numtableleft == numtableright && numinstanceleft == numinstanceright)
							{
								OKSingle = true;
								rnoper->SetLeft(xStripSubQuery(newdejaleft[0]));
								rnoper->SetRight(xStripSubQuery(newdejaright[0]));

								xAddToSubQuery(deja, numtableleft, rnoper, numinstanceleft);
							
							}
						}

						if (!OKSingle)
						{
							rnoper->SetLeft(xBuildSubQueryFrom(&newdejaleft, newpathleft));
							rnoper->SetRight(xBuildSubQueryFrom(&newdejaright, newpathright));
							xAddToSubQuery(deja, -1, rnoper, 0);
						}
						
					}
					else // dans le cas d'un ET on continue d'accumuler les elements (qui sont permutables) dans les 
							// differents tableaux correspondant a chaque table sur laquelle se pose la query
					{
						xBuildSubQueries(rnoper->GetLeft(), deja, xPath);
						xBuildSubQueries(rnoper->GetRight(), deja, xPath);
						delete rnoper;
					}
				}
				else
				{
					if (rn->GetTyp() == NoeudJoin)
					{
						RechNodeBaseJoin* rnjoin = (RechNodeBaseJoin*)rn;
						xPath.AddJoin(rnjoin->fNumTable1, rnjoin->fNumField1, rnjoin->fNumTable2, rnjoin->fNumField2, rnjoin->fNumInstance1, rnjoin->fNumInstance2);
						delete rn;
					}
					else
					{
						sLONG numtable = rn->GetTarget();
						sLONG numinstance = rn->GetInstance();
						xAddToSubQuery(deja, numtable, rn, numinstance);
					}
				}
			}
		}
	}
}

//typedef map<sLONG, IndexInfoFromMultipleField*, less<sLONG> , cache_allocator<pair<const sLONG, IndexInfoFromMultipleField*> > > indexmap;
typedef multimap<sLONG, IndexInfoFromMultipleField*> indexmap;

const sLONG kMaxCompositeParts = 16;

RechNode* OptimizedQuery::xTransformIndex(RechNode* rn, const Table* curtarget, BaseTaskInfo* context)
{
	RechNodeMultiOperator *rnop;
	RechNodeOperator *rnoper;
	RechTokenSimpleComp *rt;
	RechTokenArrayComp *rta;
	IndexInfo *ind;
	RechNodeIndex *rnind;
	RechNodeWithArrayIndex *rninda;
	
	if (VTask::GetCurrentFreeStackSize() < minStackSize)
	{
		ThrowBaseError(VE_DB4D_STACK_OVERFLOW);
		return nil;
	}

	if (rn != nil)
	{
		if (rn->GetTyp() == NoeudMultiOper)
		{
			rnop = (RechNodeMultiOperator*)rn;
			if (rnop->OperBool == DB4D_And)  // on va d'abord chercher tous les index composites sur une serie de AND
			{
				curtarget->occupe();
				
				IndexArray indexes;
				curtarget->CopyIndexDep(indexes);
				curtarget->libere();
				indexmap multiindexes;
				IndexArray::Iterator cur = indexes.First(), end = indexes.End();
				
				for (; cur != end; cur++)
				{
					IndexInfo* ind = *cur;
					if (ind != nil)
					{
						if (ind->MatchType(DB4D_Index_OnMultipleFields))
						{
							if (ind->AskForValid(context))
							{
								IndexInfoFromMultipleField* indm = (IndexInfoFromMultipleField*)ind;
								multiindexes.insert( make_pair(indm->GetFieldNums()->GetNbField(), indm));
							}
						}
					}
				}
							// on a trier les index composites concernant cette table par ordre du nombre de champs concernes

				indexmap::reverse_iterator curm = multiindexes.rbegin(), endm = multiindexes.rend();
				for (; curm != endm; curm++)  // puis pour chque index composite on recherche si l'on trouve une combinasion des champs correspondant dans les AND
				{
					IndexInfoFromMultipleField* indm = curm->second;
					const FieldNuplet* indexfields = indm->GetFieldNums();

					Boolean found = false;
					sLONG goodpos[kMaxCompositeParts];
					sLONG dejapos[kMaxCompositeParts];
					sLONG posinindex[kMaxCompositeParts];
					sLONG poslevel = 0, pos = 0;
					sLONG maxlevel = indexfields->GetNbField();
					if (maxlevel > kMaxCompositeParts)
						continue;
					
					assert(maxlevel<=kMaxCompositeParts);
					sLONG lastfield, lastcomp;

					ArrayNode& Nodes = rnop->GetArrayNodes();
					ArrayNode::ConstIterator curnode = Nodes.First(), endnode = Nodes.End();
					for (; curnode != endnode && !found; curnode++, pos++)  // donc on bloucle sur l'ensemble des AND
					{
						RechNode* nod = *curnode;
						if (nod != nil)
						{
							if (nod->GetTyp() == NoeudSeq)
							{
								rt = ((RechNodeSeq*)nod)->rt;
								Boolean okcomp;
#if 0 // ce n'est pas completement vrai LR le 5 fev 2008
								if (poslevel==maxlevel-1)  // pour le dernier niveau de champ on peut comparer sur autre chose que =
								{
									okcomp = ((rt->comparaison >= DB4D_Equal) && (rt->comparaison <= DB4D_LowerOrEqual))
										|| ((rt->comparaison >= DB4D_Like) && (rt->comparaison <= DB4D_LowerOrEqual_Like));
								}
								else
#endif
								{
									okcomp = (rt->comparaison == DB4D_Equal) || (rt->comparaison == DB4D_Like);
								}
								if (okcomp)
								{
									Boolean okfield = false;
									sLONG i;
									for (i = 0; i< maxlevel; i++)
									{
										sLONG numfield = indexfields->GetFieldNum(i);
										if (numfield == rt->numfield)
										{
											sLONG* enddeja = &dejapos[poslevel];
											if (std::find(&dejapos[0], enddeja, numfield) == enddeja)  // si le champ n'est pas deja selection pour tenter l'index
											{
												okfield = true;
												dejapos[poslevel] = numfield;
												posinindex[poslevel] = i;
												break;
											}
										}
									}
									if (okfield)
									{
										goodpos[poslevel] = pos;
										poslevel++;
										if (poslevel==maxlevel)  // quand on arrive au nombre de champs de l'index alors la combinaison reference par goodpos est la bonne
										{
											lastfield = rt->numfield;
											lastcomp = rt->comparaison;
											found = true;
										}
									}
								}
							}
						}
					}

					sLONG forkpos = 0;
					sLONG otherPartofFork1, otherPartofFork2;

					if (found)
					{
						switch(lastcomp) 
						{
							case DB4D_LowerOrEqual:
							case DB4D_Lower:
								otherPartofFork1 = DB4D_GreaterOrEqual;
								otherPartofFork2 = DB4D_Greater;
								break;

							case DB4D_GreaterOrEqual:
							case DB4D_Greater:
								otherPartofFork1 = DB4D_LowerOrEqual;
								otherPartofFork2 = DB4D_Lower;
								break;

							case DB4D_LowerOrEqual_Like:
							case DB4D_Lower_Like:
								otherPartofFork1 = DB4D_GreaterOrEqual_Like;
								otherPartofFork2 = DB4D_Greater_Like;
								break;

							case DB4D_GreaterOrEqual_Like:
							case DB4D_Greater_Like:
								otherPartofFork1 = DB4D_LowerOrEqual_Like;
								otherPartofFork2 = DB4D_Lower_Like;
								break;

							default:
								otherPartofFork1 = 0;
								otherPartofFork2 = 0;
								break;
						}

						if (otherPartofFork1 != 0) // on reboucle pour voir s'il est possible de faire une fouchette avec le dernier champ de l'index composite
						{
							curnode = Nodes.First(), endnode = Nodes.End();
							pos = 0;
							for (; curnode != endnode && !found; curnode++, pos++)
							{
								RechNode* nod = *curnode;
								if (nod != nil)
								{
									if (nod->GetTyp() == NoeudSeq)
									{
										rt = ((RechNodeSeq*)rn)->rt;
										if (rt->numfield == lastfield)
										{
											if (rt->comparaison == otherPartofFork1 || rt->comparaison == otherPartofFork2)
											{
												forkpos = pos;
											}
										}
									}
								}
							}
						}

						rnind = new RechNodeIndex;
						indm->Retain();
						indm->AskForValid(context);
						rnind->SetIndex((IndexInfo*)indm);
						ArrayNode& NodesForIndex = rnind->Nodes;

						//typedef map<sLONG, RechNode*, less<sLONG> , cache_allocator<pair<const sLONG, RechNode*> > > type_mapnode;
						typedef map<sLONG, RechNode*> type_mapnode;
						type_mapnode mapnodes;  // on va trier les AND concernes par place du champ dans l'index composite
						for (sLONG i=0; i<maxlevel; i++)
						{
							mapnodes.insert(make_pair(posinindex[i], Nodes[goodpos[i]]));
							Nodes[goodpos[i]] = nil;
						}
						type_mapnode::iterator curinmap = mapnodes.begin();
						for (; curinmap != mapnodes.end(); curinmap++)
						{
							NodesForIndex.Add(curinmap->second);
						}

						RechNode* forkother;
						if (forkpos != 0)
						{
							forkother = Nodes[forkpos];
							Nodes[forkpos] = nil;
						}
						else
							forkother = nil;

						rnind->BuildFromMultiple(forkother, context);

						ArrayNode::Iterator result = std::remove(Nodes.First(), Nodes.End(), (RechNodePtr)nil);
						sLONG nbdeleted = Nodes.End() - result;
						assert(nbdeleted == maxlevel + (forkpos == 0 ? 0 : 1));
						Nodes.SetCount(Nodes.GetCount() - nbdeleted);
						Nodes.Add(rnind);
					}

				}

				for (indexmap::iterator curm1 = multiindexes.begin(), endm1 = multiindexes.end(); curm1 != endm1; curm1++)
				{
					curm1->second->ReleaseValid();
				}
			}

			ArrayNode& Nodes = rnop->GetArrayNodes();  // puis on transforme les AND qui restent en index simple si possible
			for (ArrayNode::Iterator curnode = Nodes.First(), endnode = Nodes.End(); curnode != endnode; curnode++)
			{
				RechNode* nod = *curnode;
				if (nod != nil)
				{
					*curnode = xTransformIndex(nod, curtarget, context);
				}
			}

			if (rnop->OperBool == DB4D_And) // et on va rechercher les fourchettes sur les index simples
			{
				for (ArrayNode::Iterator curnode = Nodes.First(), endnode = Nodes.End(); curnode != endnode; curnode++)
				{
					RechNode* nod = *curnode;
					if (nod != nil)
					{
						if (nod->GetTyp() == NoeudIndexe)
						{
							RechNodeIndex* rnind = (RechNodeIndex*)nod;

							if (rnind->PeutFourche())
							{
								for (ArrayNode::Iterator curnode2 = curnode+1; curnode2 != endnode; curnode2++)
								{
									RechNode* nod2 = *curnode2;
									if (nod2 != nil && nod2->GetTyp() == NoeudIndexe)
									{
										RechNodeIndex* rnind2 = (RechNodeIndex*)(*curnode2);
										if (rnind2->PeutFourche(rnind))
										{
											delete rnind;
											*curnode = nil;
										}
									}
								}
							}
						}
					}
				}

				ArrayNode::Iterator result = std::remove(Nodes.First(), Nodes.End(), (RechNodePtr)nil);
				sLONG nbdeleted = Nodes.End() - result;
				Nodes.SetCount(Nodes.GetCount() - nbdeleted);
			}

			if (Nodes.GetCount() == 1)
			{
				rn = Nodes.GetFirst();
				delete rnop;
			}

		}
		else
		{
			if (rn->GetTyp() == NoeudNot)
			{
				RechNodeNot* rnnot = (RechNodeNot*)rn;
				rnnot->SetLeft(xTransformIndex(rnnot->GetLeft(), curtarget, context));
			}
			else
			{
				if (rn->GetTyp() == NoeudOper)
				{
					rnoper = (RechNodeOperator*)rn;
					rnoper->SetLeft(xTransformIndex(rnoper->GetLeft(), curtarget, context));
					rnoper->SetRight(xTransformIndex(rnoper->GetRight(), curtarget, context));
				}
				else
				{
					if (rn->GetTyp() == NoeudSeq)
					{
						Boolean canbeindexed = true;
						Boolean ParseIndexSeq = false;
						rt = ((RechNodeSeq*)rn)->rt;
						ValueKind ktyp = VK_UNDEFINED;
						if (rt->ch != nil)
							ktyp = rt->ch->GetValueKind();
						if (rt->ch == nil || rt->ch->IsNull())
						{
							delete rn;
							rn = new RechNodeConst(2);
						}
						else
						{
							if (rt->comparaison == DB4D_Regex_Match || rt->comparaison == DB4D_Regex_Not_Match)
								canbeindexed = false;
							else
							{
								if (ktyp == VK_STRING || ktyp == VK_TEXT || ktyp == VK_STRING_UTF8 || ktyp == VK_TEXT_UTF8)
								{
									if (InLikeRange(rt->comparaison))
									{
										VString* s = (VString*)rt->ch;
										sLONG p = s->FindUniChar(GetWildChar(context));

										/*
											for japanese hiragana: NOSA < NOZA < NOSAKI < NOZAKI
											This breaks the binary search when there's a wildchar at the end of the pattern.
											remember to also change ComplexOptimizedQuery::xTransformIndex.
										*/
										bool compatible = GetContextIntl(context)->GetCollator()->IsPatternCompatibleWithDichotomyAndDiacritics( s->GetCPointer(), s->GetLength());

										if (p == 0 || p == -1 || (p == s->GetLength() && compatible) )
											ParseIndexSeq = false;
										else
											ParseIndexSeq = true;
										if (p == 0)
										{
											rt->comparaison = RemoveTheLike(rt->comparaison);
											SetCompOptionWithOperator(rt->fOptions, rt->comparaison);
										}
									}
								}
							}
							if (canbeindexed)
							{
								Boolean tryindexregular;

								if (rt->comparaison == DB4D_Contains_KeyWord || rt->comparaison == DB4D_DoesntContain_KeyWord 
									|| rt->comparaison == DB4D_Contains_KeyWord_Like || rt->comparaison == DB4D_Doesnt_Contain_KeyWord_Like
									|| ParseIndexSeq)
								{
									tryindexregular = ParseIndexSeq;

									if ( ((rt->numfile==nf) || (rt->numfile==0)) && (rt->numinstance == 0) )
									{
										ind=fic->FindAndRetainIndexLexico(rt->numfield, true, context);
										if (ind != nil)
										{
											DB4DKeyWordList words;
											if (ParseIndexSeq)
											{
												rt->fOptions.SetIntlManager(GetContextIntl(context));
												BuildKeyWords(*((VString*)rt->ch), words, rt->fOptions);
											}
											if (!ParseIndexSeq || (words.GetCount() == 1 && (*(words[0]) == *((VString*)rt->ch))))
											{
												rnind = new RechNodeIndex;
												rnind->SetIndex(ind);
												rnind->BuildFrom(rt);
												rnind->SetParseAllIndex(ParseIndexSeq);

												delete rn;
												rn = rnind;
												tryindexregular = false;
											}
											else
											{
												ind->ReleaseValid();
												ind->Release();
											}
										}
									}
									else
									{
										Table* xtab = fic->GetOwner()->RetainTable(rt->numfile);
										if (xtab != nil)
										{
											ind = xtab->FindAndRetainIndexLexico(rt->numfield, true, context);
											xtab->Release();
											if (ind != nil)
											{
												DB4DKeyWordList words;
												if (ParseIndexSeq)
												{
													rt->fOptions.SetIntlManager(GetContextIntl(context));
													BuildKeyWords(*((VString*)rt->ch), words, rt->fOptions);
												}
												if (!ParseIndexSeq || (words.GetCount() == 1 && (*(words[0]) == *((VString*)rt->ch))))
												{
													rnind = new RechNodeIndex;
													rnind->SetIndex(ind);
													rnind->BuildFrom(rt);
													rnind->SetTarget(rt->numfile, rt->numinstance);
													rnind->SetParseAllIndex(ParseIndexSeq);

													delete rn;
													rn = rnind;
													tryindexregular = false;
												}
												else
												{
													ind->ReleaseValid();
													ind->Release();
												}
											}
										}
									}
								}
								else
									tryindexregular = true;

								if (tryindexregular)
								{
									if ( ((rt->numfile==nf) || (rt->numfile==0)) && (rt->numinstance == 0) )
									{
										ind=fic->FindAndRetainIndexSimple(rt->numfield, false, true, context);
										if (ind != nil)
										{
											rnind = new RechNodeIndex;
											rnind->SetIndex(ind);
											rnind->BuildFrom(rt);
											rnind->SetParseAllIndex(ParseIndexSeq);
											
											delete rn;
											rn = rnind;
										}
									}
									else
									{
										Table* xtab = fic->GetOwner()->RetainTable(rt->numfile);
										if (xtab != nil)
										{
											ind = xtab->FindAndRetainIndexSimple(rt->numfield, false, true, context);
											xtab->Release();
											if (ind != nil)
											{
												rnind = new RechNodeIndex;
												rnind->SetIndex(ind);
												rnind->BuildFrom(rt);
												rnind->SetTarget(rt->numfile, rt->numinstance);
												rnind->SetParseAllIndex(ParseIndexSeq);

												delete rn;
												rn = rnind;
											}
										}
									}
								}
							}
						}
					}
					else
					{
						if (rn->GetTyp() == NoeudWithArraySeq)
						{
							rta = ((RechNodeWithArraySeq*)rn)->rt;

							if (rta->values == nil || rta->values->Count() == 0 || rta->values->IsAllNull())
							{
								delete rn;
								rn = new RechNodeConst(2);
							}
							else
							{
								if (rta->values->CanBeUsedWithIndex(GetContextIntl(context)))
								{
									if ( ((rta->numfile==nf) || (rta->numfile==0)) && (rta->numinstance == 0) )
									{
										ind=fic->FindAndRetainIndexSimple(rta->numfield, false, true, context);
										if (ind != nil)
										{
											rninda = new RechNodeWithArrayIndex;
											rninda->SetIndex(ind);
											rninda->BuildFrom(rta);

											delete rn;
											rn = rninda;
										}
									}
									else
									{
										Table* xtab = fic->GetOwner()->RetainTable(rta->numfile);
										if (xtab != nil)
										{
											ind = xtab->FindAndRetainIndexSimple(rta->numfield, false, true, context);
											xtab->Release();
											if (ind != nil)
											{
												rninda = new RechNodeWithArrayIndex;
												rninda->SetIndex(ind);
												rninda->BuildFrom(rta);
												rninda->SetTarget(rta->numfile, rta->numinstance);

												delete rn;
												rn = rninda;
											}
										}
									}
								}
							}

						}
						else
						{
							if (rn->GetTyp() == NoeudSubQuery)
							{
								RechNodeSubQuery* rnsub = (RechNodeSubQuery*)rn;
								if (rnsub->GetTarget() != 0)
								{
									Table* subtarget = fic->GetOwner()->RetainTable(rnsub->GetTarget());
									if (testAssert(subtarget != nil))
									{
										rnsub->SetRoot(xTransformIndex(rnsub->GetRoot(), subtarget, context));
										subtarget->Release();
									}
								}
								else
								{
									rnsub->SetRoot(xTransformIndex(rnsub->GetRoot(), fic, context));
								}
							}
						}
					}
				}
			}
		}
	}
	
	return(rn);
}

/*
RechNode* OptimizedQuery::xCreerFourche(RechNode* rn, RechNodeArray *deja)
{
	RechNodeOperator *rnoper;
	RechNodeIndex *rnind, *rnind2;
	sLONG i;
	Boolean trouvefourche;
	RechNodeArray *newdeja;
 
	if (rn != nil)
	{
		if (rn->GetTyp() == NoeudNot)
		{
			RechNodeNot* rnnot = (RechNodeNot*)rn;
			rnnot->SetLeft(xCreerFourche(rnnot->GetLeft(), deja));
		}
		else
		{
			if (rn->GetTyp() == NoeudOper)
			{
				rnoper = (RechNodeOperator*)rn;
				if (rnoper->OperBool == DB4D_OR)
				{
					newdeja = new RechNodeArray;
					rnoper->SetLeft(xCreerFourche(rnoper->GetLeft(), newdeja));
					delete newdeja;
					
					newdeja = new RechNodeArray;
					rnoper->SetRight(xCreerFourche(rnoper->GetRight(), newdeja));
					delete newdeja;
				}
				else
				{
					rnoper->SetLeft(xCreerFourche(rnoper->GetLeft(), deja));
					rnoper->SetRight(xCreerFourche(rnoper->GetRight(), deja));
				}
			}
			else
			{
				if (rn->GetTyp() == NoeudIndexe)
				{
					rnind = (RechNodeIndex*)rn;
					
					if (rnind->PeutFourche())
					{
						trouvefourche = false;
						
						for (i = deja->GetCount(); i>0; i--)
						{
							rnind2 = (RechNodeIndex*)((*deja)[i-1]);
							if (rnind2->PeutFourche(rnind))
							{
								trouvefourche = true;
								delete rn;
								rn = nil;
							}
						}
						
						if (!trouvefourche)
						{
							deja->Add(rn);
						}
						
					}
				}
				else
				{
					if (rn->GetTyp() == NoeudSubQuery)
					{
						newdeja = new RechNodeArray;
						RechNodeSubQuery* rnsub = (RechNodeSubQuery*)rn;
						rnsub->SetRoot(xCreerFourche(rnsub->GetRoot(), newdeja));
						delete newdeja;
					}
				}
			}
		}
	}
	
	return(rn);
	
}
*/

RechNode* OptimizedQuery::xPurgeNilAndSolvePath(RechNode* rn, BaseTaskInfo* context)
{
	RechNodeOperator *rnoper;

	if (VTask::GetCurrentFreeStackSize() < minStackSize)
	{
		ThrowBaseError(VE_DB4D_STACK_OVERFLOW);
		return nil;
	}

	if (rn != nil)
	{
		if (rn->GetTyp() == NoeudMultiOper)
		{
			RechNodeMultiOperator* rnop = (RechNodeMultiOperator*)rn;
			ArrayNode& Nodes = rnop->GetArrayNodes();
			for (ArrayNode::Iterator cur = Nodes.First(), end = Nodes.End(); cur != end; cur++)
			{
				if (*cur != nil)
					*cur = xPurgeNilAndSolvePath(*cur, context);
			}
		}
		else
		{
			if (rn->GetTyp() == NoeudNot)
			{
				rnoper = (RechNodeOperator*)rn;
				rnoper->SetLeft(xPurgeNilAndSolvePath(rnoper->GetLeft(), context));
				
				if (rnoper->GetLeft() == nil)
				{
					rn = nil;
					delete rnoper;
				}
			}
			else
			{
				if (rn->GetTyp() == NoeudSubQuery)
				{
					RechNodeSubQuery* rnsub = (RechNodeSubQuery*)rn;
					rnsub->SetRoot(xPurgeNilAndSolvePath(rnsub->GetRoot(), context));
					if (rnsub->GetTarget() == 0 && rnsub->GetInstance() == 0)
					{
						rn = rnsub->GetRoot();
						delete rnsub;
					}
					else
					{
						rnsub->SolvePath(nf, fDefaultInstance, context);
					}
				}
			}
		}
	}
	
	return(rn);
}


inline Boolean RechNodeLess(RechNode* r1, RechNode* r2)
{
	if (r1 == nil)
		return false;
	if (r2 == nil)
		return true;
	return r1->PoidsIndex() > r2->PoidsIndex();
}


RechNode* OptimizedQuery::xRegroupJoinPaths(RechNode* rn, bool checkForSubQueryAlone, BaseTaskInfo* context, sLONG subTarget)
{
	if (VTask::GetCurrentFreeStackSize() < minStackSize)
	{
		ThrowBaseError(VE_DB4D_STACK_OVERFLOW);
		return nil;
	}

	if (rn != nil)
	{
		if (rn->GetTyp() == NoeudSubQuery && checkForSubQueryAlone)
		{
			if ( ((RechNodeSubQuery*)rn)->GetFinalPath().GetCount() > 1 )
			{
				RechNodeMultiOperator* rnoper = new RechNodeMultiOperator();
				rnoper->OperBool = DB4D_And;
				rnoper->AddNode(rn);
				rn = rnoper;
			}
		}

		if (rn->GetTyp() == NoeudMultiOper)
		{
			RechNodeMultiOperator* rnop = (RechNodeMultiOperator*)rn;
			ArrayNode& Nodes = rnop->GetArrayNodes();
			if (rnop->OperBool == DB4D_And)
			{
				bool withSubQuery = false;
				vector<RechNodeSubQuery*> firstlevels;

				for (ArrayNode::Iterator cur = Nodes.First(), end = Nodes.End(); cur != end; cur++) // premiere passe, on recherche les premiers niveaux
				{
					RechNode* rn2 = *cur;
					if (rn2 != nil && rn2->GetTyp() == NoeudSubQuery)
					{
						RechNodeSubQuery* rnsub = (RechNodeSubQuery*)rn2;
						if (rnsub->IsSolved())
						{
							withSubQuery = true;
							JoinPath& path = rnsub->GetFinalPath();
							if (path.GetCount() == 1)
							{
								firstlevels.push_back(rnsub);
							}
						}

					}
				}

				if (withSubQuery) // evitons une deuxieme passe inutile si aucune subquery detectee
				{
					sLONG nbfirsts = (sLONG)firstlevels.size();
					bool aLeastOneCreated = false;

					for (ArrayNode::Iterator cur = Nodes.First(), end = Nodes.End(); cur != end; cur++) // deuxieme passe, on recherche tout les niveaux superieurs a 1 pour les associer aux premiers niveaux correspondant.
					{
						RechNode* rn2 = *cur;
						if (rn2 != nil && rn2->GetTyp() == NoeudSubQuery)
						{
							RechNodeSubQuery* rnsub = (RechNodeSubQuery*)rn2;
							if (rnsub->IsSolved())
							{
								JoinPath& path = rnsub->GetFinalPath();
								sLONG pathLen = path.GetCount();
								if (pathLen > 1) // on prend les niveaux superieurs a 1
								{
									RechNodeSubQuery* found = nil;
									for (vector<RechNodeSubQuery*>::iterator curx = firstlevels.begin(), endx = firstlevels.end(); curx != endx; curx++)
									{
										JoinPath& pathx = (*curx)->GetFinalPath();
										if (pathx.GetCoupleRef(0) == path.GetCoupleRef(pathLen-1))
										{
											found = *curx;
										}
									}
									if (found == nil)
									{
										aLeastOneCreated = true;
										found = new RechNodeSubQuery(nil);
										JoinRefCouple jcouple = path.GetCoupleRef(pathLen-1);
										if (jcouple.fNumInstance1 == fDefaultInstance && jcouple.fNumTable1 == subTarget)
										{
											found->SetTarget(jcouple.fNumTable2, jcouple.fNumInstance2);
										}
										else
										{
											found->SetTarget(jcouple.fNumTable1, jcouple.fNumInstance1);
										}
										found->SetOneLevelPath(jcouple, context);
										firstlevels.push_back(found);
									}

									path.RemoveLast();
									JoinRefCouple& jcouple2 = path.GetCoupleRef(pathLen-2);
									if (found->GetTarget() == jcouple2.fNumTable1 && found->GetInstance() == jcouple2.fNumInstance1)
									{
										rnsub->SetTarget(jcouple2.fNumTable2, jcouple2.fNumInstance2);
									}
									else
									{
										rnsub->SetTarget(jcouple2.fNumTable1, jcouple2.fNumInstance1);
									}

									rnsub->ResetIndex(context);

									*cur = nil;

									if (found->GetRoot() == nil)
										found->SetRoot(rnsub);
									else // il faut faire un AND de la sourequete existante avec celle que l'on vient d'associer
									{
										RechNodeMultiOperator* rnoper = new RechNodeMultiOperator();
										rnoper->OperBool = DB4D_And;
										rnoper->AddNode(found->GetRoot());
										rnoper->AddNode(rnsub);
										
										found->SetRoot(rnoper);
									}
								}
							}
						}
					}

					// on retire les trou crees par les deplacements des subqueries dans des sous niveaux
					sLONG nbremain = 0;
					for (ArrayNode::Iterator cur = Nodes.First(), end = Nodes.End(), cur2 = Nodes.First(); cur != end; cur++)
					{
						if (*cur != nil)
						{
							*cur2 = *cur;
							cur2++;
							nbremain++;
						}
					}
					Nodes.SetCount(nbremain);

					if (aLeastOneCreated) // maintenant on ajoute les subquery avec un seul couple crees precedemment
					{
						for (vector<RechNodeSubQuery*>::iterator curx = firstlevels.begin()+nbfirsts, endx = firstlevels.end(); curx != endx; curx++)
						{
							Nodes.Add(*curx);
						}
					}
				}

				for (ArrayNode::Iterator cur = Nodes.First(), end = Nodes.End(); cur != end; cur++) // puis on fait une derniere passe pour la gestion recursive
				{
					RechNode* rn2 = *cur;
					if (rn2 != nil)
						*cur = xRegroupJoinPaths(rn2, false, context, subTarget);
				}

				// si le AND n'a plus qu'une seule branche on le remplace par cette branche
				if (Nodes.GetCount() == 1)
				{
					rn = Nodes[0];
					delete rnop;
				}

			}
			else // pas d'optimisation possible sur les OU entre jointures
			{
				for (ArrayNode::Iterator cur = Nodes.First(), end = Nodes.End(); cur != end; cur++)
				{
					RechNode* rn2 = *cur;
					if (rn2 != nil)
						*cur = xRegroupJoinPaths(rn2, true, context, subTarget);
				}
			}
		}
		else
		{
			if (rn->GetTyp() == NoeudNot)
			{
				RechNodeOperator* rnoper = (RechNodeOperator*)rn;
				rnoper->SetLeft(xRegroupJoinPaths(rnoper->GetLeft(), true, context, subTarget));

				if (rnoper->GetLeft() == nil)
				{
					rn = nil;
					delete rnoper;
				}
			}
			else
			{
				if (rn->GetTyp() == NoeudSubQuery)
				{
					RechNodeSubQuery* rnsub = (RechNodeSubQuery*)rn;
					rnsub->SetRoot(xRegroupJoinPaths(rnsub->GetRoot(), true, context, rnsub->GetTarget()));
				}
			}
		}
	}

	return rn;
}


RechNode* OptimizedQuery::xRemoveRecordExist(RechNode* rn, uBOOL& mustinverse)
{
	mustinverse = 2;
	if (rn != nil)
	{
		if (rn->GetTyp() == NoeudMultiOper)
		{
			uBOOL submustinverse;
			RechNodeMultiOperator* rnop = (RechNodeMultiOperator*)rn;
			ArrayNode& Nodes = rnop->GetArrayNodes();
			if (rnop->OperBool == DB4D_And)
			{
				if (Nodes.GetCount() > 0)
				{
					RechNode* rn2 = Nodes[0];
					if (rn2->GetTyp() == NoeudRecordExist)
					{
						RechNodeRecordExist* rnRecExist = (RechNodeRecordExist*)rn2;
						if (rnRecExist->rt->fCheckIfExists)
						{
							sLONG nb = Nodes.GetCount()-1;
							if (nb == 0)
							{
								rn->DisposeTree();
								delete rn;
								rn = nil;
								mustinverse = 0;
							}
							else
							{
								for (sLONG i = 0; i < nb; i++)
								{
									Nodes[i] = xRemoveRecordExist(Nodes[i+1], submustinverse);
								}
								Nodes[nb] = nil;
								Nodes.SetCount(nb);
								delete rnRecExist;
							}
						}
						else
						{
							if (Nodes.GetCount() == 1)
							{
								rn->DisposeTree();
								delete rn;
								rn = nil;
								mustinverse = 1;
							}
							else
							{
								rn->DisposeTree();
								delete rn;
								RechNodeConst* rnconst = new RechNodeConst(false);
								rn = rnconst;
							}
							
						}
					}
					else
					{
						sLONG nb = Nodes.GetCount();
						for (sLONG i = 0; i < nb; i++)
						{
							Nodes[i] = xRemoveRecordExist(Nodes[i], submustinverse);
						}
					}
				}
			}
			else
			{
				sLONG nb = Nodes.GetCount();
				for (sLONG i = 0; i < nb; i++)
				{
					Nodes[i] = xRemoveRecordExist(Nodes[i], submustinverse);
				}
			}

		}
		else
		{
			if (rn->GetTyp() == NoeudSubQuery)
			{
				RechNodeSubQuery* rnsub = (RechNodeSubQuery*)rn;
				uBOOL submustinverse = 2;
				RechNode* rn2 = xRemoveRecordExist(rnsub->GetRoot(), submustinverse);
				if (rn2 != nil && rn2->GetTyp() == NoeudRecordExist)
				{
					if (!((RechNodeRecordExist*)rn2)->rt->fCheckIfExists)
						submustinverse = true;
					rn2->DisposeTree();
					delete rn2;
					rn2 = nil;
				}
				rnsub->SetRoot(rn2);
				if (submustinverse == 1)
				{
					rnsub->MustInverse(true);
				}
			}
			else
			{
				if (rn->GetTyp() == NoeudNot)
				{
					RechNodeOperator* rnoper = (RechNodeOperator*)rn;
					RechNode* rn2 = xRemoveRecordExist(rnoper->GetLeft(), mustinverse);
					rnoper->SetLeft(rn2);
					if (rn2 == nil)
					{
						if (mustinverse != 2)
							mustinverse = !mustinverse;
						rnoper->DisposeTree();
						delete rnoper;
						rn = nil;
					}
					else
					{
						if (rn2->GetTyp() == NoeudRecordExist)
						{
							mustinverse = !(((RechNodeRecordExist*)rn2)->rt->fCheckIfExists);
							rnoper->DisposeTree();
							delete rnoper;
							rn = nil;
						}
					}
				}
			}
		}
	}

	return rn;
}



RechNode* OptimizedQuery::xPermuteAnds(RechNode* rn)
{
	if (VTask::GetCurrentFreeStackSize() < minStackSize)
	{
		ThrowBaseError(VE_DB4D_STACK_OVERFLOW);
		return nil;
	}

	if (rn != nil)
	{
		if (rn->GetTyp() == NoeudMultiOper)
		{
			RechNodeMultiOperator* rnop = (RechNodeMultiOperator*)rn;
			ArrayNode& Nodes = rnop->GetArrayNodes();
			if (rnop->OperBool == DB4D_And)
			{
				std::sort(Nodes.First(), Nodes.End(), RechNodeLess);
			}
			for (ArrayNode::Iterator cur = Nodes.First(), end = Nodes.End(); cur != end; cur++)
			{
				RechNode* nod = *cur;
				if (nod != nil)
					xPermuteAnds(nod);
			}
		}
		else
		{
			if (rn->GetTyp() == NoeudSubQuery)
			{
				RechNodeSubQuery* rnsub = (RechNodeSubQuery*)rn;
				xPermuteAnds(rnsub->GetRoot());
			}
			else
			{
				if (rn->GetTyp() == NoeudOper || rn->GetTyp() == NoeudNot)
				{
					RechNodeOperator* rnoper = (RechNodeOperator*)rn;
					xPermuteAnds(rnoper->GetLeft());
					xPermuteAnds(rnoper->GetRight());
				}
			}
		}
	}

	return rn;
}


/*
uBOOL OptimizedQuery::xPermuteAnds_PartialIndex(RechNode* rn, RechNode** Permutant)
{
	RechNodeOperator *rnoper, *permutParent, *rnParent;
	RechNode* sousPermutant;
	uBOOL result = false;
	uBOOL TestPermute = true;

	if (rn != nil)
	{
		if (rn->GetTyp() == NoeudSubQuery)
		{
			RechNodeSubQuery* rnsub = (RechNodeSubQuery*)rn;
			do
			{
				sousPermutant = nil;
			} while (xPermuteAnds_PartialIndex(rnsub->GetRoot(), &sousPermutant));
		}
		

	
		if (rn->GetTyp() == NoeudOper || rn->GetTyp() == NoeudNot)
		{
			rnoper = (RechNodeOperator*)rn;
			
			if (rnoper->OperBool == DB4D_OR || rn->GetTyp() == NoeudNot)
			{
				do
				{
					sousPermutant = nil;
				} while (xPermuteAnds_PartialIndex(rnoper->GetLeft(), &sousPermutant));

				do
				{
					sousPermutant = nil;
				} while (xPermuteAnds_PartialIndex(rnoper->GetRight(), &sousPermutant));
				
				
				
			}
			else
			{
				result = xPermuteAnds_PartialIndex(rnoper->GetLeft(), Permutant);
				result = result || xPermuteAnds_PartialIndex(rnoper->GetRight(), Permutant);
				TestPermute = false;
				
			}
			
			
		}
		
		
		if (TestPermute)
		{
			if (*Permutant == nil)
			{
				if (!rn->IsAllIndexe() && !rn->IsIndexe())
				{
					*Permutant = rn;
				}
			}
			else
			{
				if (rn->IsAllIndexe() || rn->IsIndexe())
				{
					result = true; // on permute la comparaison non indexee pour la mettre plus a droite
					rnParent = (RechNodeOperator*)(rn->GetParent());
					permutParent = (RechNodeOperator*)((*Permutant)->GetParent());
					
					if (permutParent == rnParent)
					{
						if (permutParent->GetLeft() == *Permutant)
						{
							permutParent->SetLeft(rn);
							permutParent->SetRight(*Permutant);
						}
						else
						{
							permutParent->SetRight(rn);
							rnParent->SetLeft(*Permutant);
						}
					}
					else
					{
						if (permutParent->GetLeft() == *Permutant) permutParent->SetLeft(rn); else permutParent->SetRight(rn);
						if (rnParent->GetLeft() == rn) rnParent->SetLeft(*Permutant); else rnParent->SetRight(*Permutant);
					}
				}
			}
		}
	}
	
	
	return(result);
}


uBOOL OptimizedQuery::xPermuteAnds_FullIndex(RechNode* rn, RechNode** Permutant)
{
	RechNodeOperator *rnoper, *permutParent, *rnParent;
	RechNode* sousPermutant;
	uBOOL result = false;
	uBOOL TestPermute = true;

	if (rn != nil)
	{
		if (rn->GetTyp() == NoeudSubQuery)
		{
			RechNodeSubQuery* rnsub = (RechNodeSubQuery*)rn;
			do
			{
				sousPermutant = nil;
			} while (xPermuteAnds_FullIndex(rnsub->GetRoot(), &sousPermutant));
		}
	
		if (rn->GetTyp() == NoeudOper || rn->GetTyp() == NoeudNot)
		{
			rnoper = (RechNodeOperator*)rn;
			
			if (rnoper->OperBool == DB4D_OR || rn->GetTyp() == NoeudNot)
			{
				do
				{
					sousPermutant = nil;
				} while (xPermuteAnds_FullIndex(rnoper->GetLeft(), &sousPermutant));

				do
				{
					sousPermutant = nil;
				} while (xPermuteAnds_FullIndex(rnoper->GetRight(), &sousPermutant));
				
				
				
			}
			else
			{
				result = xPermuteAnds_FullIndex(rnoper->GetLeft(), Permutant);
				result = result || xPermuteAnds_FullIndex(rnoper->GetRight(), Permutant);
				TestPermute = false;
				
			}
			
			
		}
		
		
		if (TestPermute)
		{
			if (*Permutant == nil)
			{
				if (!rn->IsAllIndexe())
				{
					*Permutant = rn;
				}
			}
			else
			{
				if (rn->IsAllIndexe())
				{
					result = true; // on permute la comparaison non indexee pour la mettre plus a droite
					rnParent = (RechNodeOperator*)(rn->GetParent());
					permutParent = (RechNodeOperator*)((*Permutant)->GetParent());
					
					if (permutParent == rnParent)
					{
						if (permutParent->GetLeft() == *Permutant)
						{
							permutParent->SetLeft(rn);
							permutParent->SetRight(*Permutant);
						}
						else
						{
							permutParent->SetRight(rn);
							rnParent->SetLeft(*Permutant);
						}
					}
					else
					{
						if (permutParent->GetLeft() == *Permutant) permutParent->SetLeft(rn); else permutParent->SetRight(rn);
						if (rnParent->GetLeft() == rn) rnParent->SetLeft(*Permutant); else rnParent->SetRight(*Permutant);
					}
				}
			}
		}
	}
	
	
	return(result);
}



uBOOL OptimizedQuery::xPermuteAnds_Joins(RechNode* rn, RechNode** Permutant)
{
	RechNodeOperator *rnoper, *permutParent, *rnParent;
	RechNode* sousPermutant;
	uBOOL result = false;
	uBOOL TestPermute = true;

	if (rn != nil)
	{
		if (rn->GetTyp() == NoeudSubQuery)
		{
			RechNodeSubQuery* rnsub = (RechNodeSubQuery*)rn;
			do
			{
				sousPermutant = nil;
			} while (xPermuteAnds_FullIndex(rnsub->GetRoot(), &sousPermutant));
		}
		else
		{
			if (rn->GetTyp() == NoeudOper || rn->GetTyp() == NoeudNot)
			{
				rnoper = (RechNodeOperator*)rn;

				if (rnoper->OperBool == DB4D_OR || rn->GetTyp() == NoeudNot)
				{
					do
					{
						sousPermutant = nil;
					} while (xPermuteAnds_Joins(rnoper->GetLeft(), &sousPermutant));

					do
					{
						sousPermutant = nil;
					} while (xPermuteAnds_Joins(rnoper->GetRight(), &sousPermutant));



				}
				else
				{
					result = xPermuteAnds_Joins(rnoper->GetLeft(), Permutant);
					result = result || xPermuteAnds_Joins(rnoper->GetRight(), Permutant);
					TestPermute = false;

				}


			}


			if (TestPermute)
			{
				if (*Permutant == nil)
				{
					if (!rn->IsAllJoin())
					{
						*Permutant = rn;
					}
				}
				else
				{
					if (rn->IsAllJoin())
					{
						result = true; // on permute la partie non jointure pour la mettre plus a droite
						rnParent = (RechNodeOperator*)(rn->GetParent());
						permutParent = (RechNodeOperator*)((*Permutant)->GetParent());

						if (permutParent == rnParent)
						{
							if (permutParent->GetLeft() == *Permutant)
							{
								permutParent->SetLeft(rn);
								permutParent->SetRight(*Permutant);
							}
							else
							{
								permutParent->SetRight(rn);
								rnParent->SetLeft(*Permutant);
							}
						}
						else
						{
							if (permutParent->GetLeft() == *Permutant) permutParent->SetLeft(rn); else permutParent->SetRight(rn);
							if (rnParent->GetLeft() == rn) rnParent->SetLeft(*Permutant); else rnParent->SetRight(*Permutant);
						}
					}
				}
			}
		}
	}


	return(result);
}

*/


RechNode* OptimizedQuery::xTransformSeq(RechNode* rn, uBOOL seq)
{
	RechNodeOperator *rnoper;
	RechNodeIndex *rnind;
	RechNodeWithArrayIndex *rninda;
	RechNode *xleft,*xright;
	RechNodeSeq *rn1,*rn2;
	RechNodeWithArraySeq *rna;
	
	if (VTask::GetCurrentFreeStackSize() < minStackSize)
	{
		ThrowBaseError(VE_DB4D_STACK_OVERFLOW);
		return nil;
	}

	if (rn != nil)
	{
		if (rn->GetTyp() == NoeudMultiOper)
		{
			RechNodeMultiOperator* rnop = (RechNodeMultiOperator*)rn;
			if ( !rnop->IsIndexe() ) seq = true;

			ArrayNode& Nodes = rnop->GetArrayNodes();
			for (ArrayNode::Iterator cur = Nodes.First(), end = Nodes.End(); cur != end; cur++)
			{
				RechNode* nod = *cur;
				if (nod != nil)
				{
					nod = xTransformSeq(nod, seq);
					/*
					if (nod != nil)
					{
						if (nod->GetTyp() == NoeudMultiOper && ((RechNodeMultiOperator*)nod)->OperBool == rnop->OperBool)
						{
							RechNodeMultiOperator* rnop2 = (RechNodeMultiOperator*)nod;
							ArrayNode& Nodes2 = rnop2->GetArrayNodes();
							for (ArrayNode::Iterator cur2 = Nodes2.First(), end2 = Nodes2.End(); cur2 != end2; cur2++)
							{
								RechNode* nod2 = *cur2;
								if (nod2 != nil)
								{
									Nodes.Add(nod2);
								}
							}
							delete nod;
							nod = nil;
							*cur = nil;
						}
						else
							*cur = nod;
					}
					else
						*/
						*cur = nod;
				}
			}

			/*
			ArrayNode::Iterator result = std::remove(Nodes.First(), Nodes.End(), (RechNode*)nil);
			sLONG nbdeleted = Nodes.End()-result;
			Nodes.SetCount(Nodes.GetCount()-nbdeleted);
			*/
			rnop->ForceRecalcIndex();

		}
		else
		{
			if (rn->GetTyp() == NoeudSubQuery)
			{
				RechNodeSubQuery* rnsub = (RechNodeSubQuery*)rn;
				rnsub->SetRoot(xTransformSeq(rnsub->GetRoot(), false));
			}
			else
			{
				if (rn->GetTyp() == NoeudOper || rn->GetTyp() == NoeudNot)
				{
					rnoper = (RechNodeOperator*)rn;
					
					xleft = rnoper->GetLeft();
					xright = rnoper->GetRight();
					
					if ( !rnoper->IsIndexe() ) seq = true;
						
					rnoper->SetLeft(xTransformSeq(xleft, seq));
					rnoper->SetRight(xTransformSeq(xright, seq));
					rnoper->ForceRecalcIndex();
				}
				else
				{
					if (seq && (rn->GetTyp() == NoeudIndexe))
					{
						rnind = (RechNodeIndex*)rn;
						if (rnind->rt == nil) // nous sommes dans le cas d'un index multiple
						{
						// retransforme le noeud indexe en noeud sequentiel en utilisant les RechNode* conserves
						if (testAssert(rnind->ind->MatchType(DB4D_Index_OnMultipleFields)))
							{
								RechNodeMultiOperator* rnop = new RechNodeMultiOperator;
								rnop->OperBool = DB4D_And;
								ArrayNode& Nodes = rnind->Nodes;
								for (ArrayNode::Iterator cur = Nodes.First(), end = Nodes.End(); cur != end; cur++)
								{
									rnop->AddNode(*cur);
								}
								Nodes.SetCount(0);
								if (rnind->fIncludingFork != nil)
								{
									rnop->AddNode(rnind->fIncludingFork);
									rnind->fIncludingFork = nil;
								}
							}
						}
						else
						{
							// retransforme le noeud indexe en noeud sequentiel en utilisant les RechTokenSimpleComp conserves
							if (rnind->isfourche)
							{
								RechNodeMultiOperator* rnop = new RechNodeMultiOperator;
								rnop->OperBool = DB4D_And;
								
								rn1 = new RechNodeSeq;
								rn1->rt = rnind->rt;
								if (rn1->rt->numfile != nf || rn1->rt->numinstance != fDefaultInstance)
									rn1->SetTarget(rn1->rt->numfile, rn1->rt->numinstance);
								
								rn2 = new RechNodeSeq;
								rn2->rt = rnind->rt2;
								if (rn2->rt->numfile != nf || rn2->rt->numinstance != fDefaultInstance)
									rn2->SetTarget(rn2->rt->numfile, rn2->rt->numinstance);
								
								rnop->AddNode(rn1);
								rnop->AddNode(rn2);
								
								delete rnind;
								rn = rnop;
							}
							else
							{
								rn1 = new RechNodeSeq;
								rn1->rt = rnind->rt;
								if (rn1->rt->numfile != nf || rn1->rt->numinstance != fDefaultInstance)
									rn1->SetTarget(rn1->rt->numfile, rn1->rt->numinstance);
								
								delete rnind;
								rn = rn1;
							}
						}
					}
					else
					{
						if (seq && (rn->GetTyp() == NoeudWithArrayIndex))
						{
							// retransforme le noeud indexe en noeud sequentiel en utilisant les RechTokenArrayComp conserves
							rninda = (RechNodeWithArrayIndex*)rn;
							rna = new RechNodeWithArraySeq;
							rna->rt = rninda->rt;
							rna->values = rninda->values;
							if (rna->values != nil)
								rna->values->Retain();
							if (rna->rt->numfile != nf || rna->rt->numinstance != fDefaultInstance)
								rna->SetTarget(rna->rt->numfile, rna->rt->numinstance);

							delete rninda;
							rn = rna;
						}
					}

				}
			}
		}
	}
	
	return(rn);
}


RechNode* OptimizedQuery::xFlattenNodes(RechNode* rn, RechNodeMultiOperator* encours)
{
	RechNode* result;

	if (VTask::GetCurrentFreeStackSize() < minStackSize)
	{
		ThrowBaseError(VE_DB4D_STACK_OVERFLOW);
		return nil;
	}

	if (rn != nil && rn->GetTyp() == NoeudOper)
	{
		RechNodeOperator *rnop = (RechNodeOperator*)rn;
		if (rnop->OperBool == DB4D_Except)
		{
			rnop->OperBool = DB4D_And;
			if (rnop->GetRight() != nil)
			{
				RechNodeNot* rnnot = new RechNodeNot;
				rnnot->SetLeft(rnop->GetRight());
				rnop->SetRight(rnnot);
			}
		}

		if (rnop->OperBool == DB4D_And || rnop->OperBool == DB4D_OR)
		{
			if (encours != nil && rnop->OperBool == encours->OperBool)
			{
				encours->AddNode(xFlattenNodes(rnop->GetLeft(), encours));
				encours->AddNode(xFlattenNodes(rnop->GetRight(), encours));
				result = nil;
			}
			else
			{
				RechNodeMultiOperator* newbranch = new RechNodeMultiOperator;
				newbranch->OperBool = rnop->OperBool;
				newbranch->AddNode(xFlattenNodes(rnop->GetLeft(), newbranch));
				newbranch->AddNode(xFlattenNodes(rnop->GetRight(), newbranch));
				result = newbranch;
			}
			delete rn;
		}
		else
		{
			result = rn;
		}
	}
	else
	{
		result = rn;
	}
	return result;
}


VError OptimizedQuery::MaximizeBittab(Bittab* b)
{
	VError err = VE_OK;
	if (fMaxRecords != -1 && b != nil)
	{
		b->ClearFrom(fMaxRecords);
	}
	return err;
}


sLONG8 OptimizedQuery::CalculateNBDiskAccessForSeq(Bittab* filtre, BaseTaskInfo* context, sLONG CountBlobSources)
{
	if (filtre == nil)
		return ((sLONG8)(1+CountBlobSources*3)) * (sLONG8)(GetTargetFile()->GetDF()->GetNbRecords(context, false)) * GetTargetFile()->GetSeqRatioCorrector();
	else
		return ((sLONG8)(1+CountBlobSources*3)) * (sLONG8)(filtre->Compte()) * GetTargetFile()->GetSeqRatioCorrector();
}


VValueBag* OptimizedQuery::AddToDescription(VValueBag* root, const VString s, uLONG startingMillisecs, Bittab* b)
{
	uLONG curtime = VSystem::GetCurrentTime();

	VValueBag* sub = new VValueBag;
	sub->SetString(QueryKeys::description, s);
	sub->SetLong(QueryKeys::time, curtime-startingMillisecs);
	sub->SetLong(QueryKeys::recordsfounds, b == nil ? 0 : b->Compte());
	root->AddElement(QueryKeys::steps, sub);
	sub->Release(); // sub est retained par root
	return sub;
}



RechNode* OptimizedQuery::xSplitSubQueryOnSubTables(RechNode* rn, BaseTaskInfo* context, RechNodeSubQuery* DejaDansSousTables )
{
	RechNodeOperator *rnoper;

	if (rn != nil)
	{
		if (rn->GetTyp() == NoeudMultiOper)
		{
			RechNodeMultiOperator* rnop = (RechNodeMultiOperator*)rn;
			ArrayNode& Nodes = rnop->GetArrayNodes();
			for (ArrayNode::Iterator cur = Nodes.First(), end = Nodes.End(); cur != end; cur++)
			{
				if (*cur != nil)
					*cur = xSplitSubQueryOnSubTables(*cur, context, DejaDansSousTables);
			}
		}
		else
		{
			if (rn->GetTyp() == NoeudOper /*|| rn->GetTyp() == NoeudNot*/)
			{
				rnoper = (RechNodeOperator*)rn;
				rnoper->SetLeft(xSplitSubQueryOnSubTables(rnoper->GetLeft(), context, DejaDansSousTables));
				rnoper->SetRight(xSplitSubQueryOnSubTables(rnoper->GetRight(), context, DejaDansSousTables));

				if (rnoper->GetLeft() == nil)
				{
					rn = rnoper->GetRight();
					delete rnoper;
				}
				else
				{
					if (rnoper->GetRight() == nil)
					{
						rn = rnoper->GetLeft();
						delete rnoper;
					}
				}
			}
			else
			{
				if (rn->GetTyp() == NoeudSubQuery)
				{
					RechNodeSubQuery* rnsub = (RechNodeSubQuery*)rn;
					if (rnsub->ContainsSubTables())
					{
						rn = xSplitSubQueryOnSubTables(rnsub->GetRoot(), context, rnsub);
						delete rnsub;
					}
					else
					{
						rnsub->SetRoot(xSplitSubQueryOnSubTables(rnsub->GetRoot(), context, nil));
					}
				}
				else
				{
					if (DejaDansSousTables != nil)
					{
						RechNodeSubQuery* rnsub = DejaDansSousTables->Clone();
						rnsub->SetRoot(rn);
						rn = rnsub;
					}
				}
			}
		}
	}

	return(rn);
}


VError OptimizedQuery::AnalyseSearch(SearchTab *model, BaseTaskInfo* context, bool forcompute)
{
	RechNodeArray deja;
	RechNode* Permutant;
	VError err = VE_OK;

	fModel = dynamic_cast<LocalEntityModel*>(model->GetModel());

	if (fModel != nil)
	{
		fic = fModel->GetMainTable();
		model->SetTarget(fic);
	}
	else
		fic = model->GetTargetFile();

	if (fic != nil)
	{
		StErrorContextInstaller errs(true);
		
		nf = fic->GetNum();

		if (!forcompute)
		{
			if (fQueryPlan != nil)
				fQueryPlan->Release();

			if (model->MustDisplay())
			{
				fQueryPlan = new VValueBag();
			}
			else
				fQueryPlan = nil;


			if (fic != nil)
			{
				if (context != nil && context->ShouldDescribeQuery() && fic->GetOwner()->GetStructure() != nil)
				{
					model->SetDisplayTree(true);
				}
			}
		}
		
		model->ReplaceParams(context);
		
		errQuery = 0;
		model->PosToFirstToken();

		root = xBuildNodes(model, nil); // construit un arbre a partir de la formule lineaire (avec parentheses)
		err = VTask::GetCurrent()->GetLastError();

		if (fic != nil)
		{
			Base4D *bd = fic->GetOwner();
			if (model->MustDisplay())
			{
				DebugMsg(L"  \n\n");
				if (root != nil)
					root->DisplayTree(bd, 0, nil);
				DebugMsg(L"  \n");
			}
		}

		if (err == VE_OK)
			root = xFlattenNodes(root, nil); // remplace les RechNodeOperator par des RechNodeMultiOperator

		err = VTask::GetCurrent()->GetLastError();

		if (fic != nil)
		{
			Base4D *bd = fic->GetOwner();
			if (false && model->MustDisplay())
			{
				DebugMsg(L"  \n\n");
				if (root != nil)
					root->DisplayTree(bd, 0, nil);
				DebugMsg(L"  \n");
			}
		}

		if (err == VE_OK && !forcompute)
		{
			{
				RechNodeArray ListOfSub;
				JoinPath xpath;
				xBuildSubQueries(root, &ListOfSub, xpath);
				root = xBuildSubQueryFrom(&ListOfSub, xpath);
				err = VTask::GetCurrent()->GetLastError();
			}

			if (fic != nil)
			{
				Base4D *bd = fic->GetOwner();
				if (false && model->MustDisplay())
				{
					DebugMsg(L"  \n\n");
					if (root != nil)
						root->DisplayTree(bd, 0, nil);
					DebugMsg(L"  \n");
				}
			}

			if (err == VE_OK)
				root = xTransformIndex(root, fic, context); // trouve les comparaison qui peuvent etre indexees
			err = VTask::GetCurrent()->GetLastError();
			//root = xCreerFourche(root, &deja); // recherche les fourchettes indexees
			if (err == VE_OK)
				root = xPurgeNilAndSolvePath(root, context); // retire les branche vides crees par les fourchettes indexees et les subqueries sur la target principale
															// resoud aussi les chemins d'acces des jointures
			err = VTask::GetCurrent()->GetLastError();
			
			if (err == VE_OK)
				root = xSplitSubQueryOnSubTables(root, context, nil); // si il y a plusieurs comparaisons sur des champs de sous tables, il faut les gerer separement
			err = VTask::GetCurrent()->GetLastError();

			if (fic != nil)
			{
				Base4D *bd = fic->GetOwner();
				if (false && model->MustDisplay())
				{
					DebugMsg(L"  \n\n");
					if (root != nil)
						root->DisplayTree(bd, 0, nil);
					DebugMsg(L"  \n");
				}
			}

			if (err == VE_OK)
				xPermuteAnds(root); // permute les comparaisons separees par des AND pour mettre les index full avant les partiels
														// eux meme avant les comparaisons sequentielles
			err = VTask::GetCurrent()->GetLastError();

			if (fic != nil)
			{
				Base4D *bd = fic->GetOwner();
				if (model->MustDisplay())
				{
					DebugMsg(L"  \n\n");
					if (root != nil)
						root->DisplayTree(bd, 0, nil);
					DebugMsg(L"  \n");
				}
			}

			/*
			do
			{
				Permutant = nil;
			} while (xPermuteAnds_PartialIndex(root, &Permutant)); // permute les comparaisons separees par des AND 
																// pour mettres les index partiels et full avant les comparaisons sequentielles
			do
			{
				Permutant = nil;
			} while (xPermuteAnds_FullIndex(root, &Permutant)); // permute les comparaisons separees par des AND 
																// pour mettres les full index d'abord
			do
			{
				Permutant = nil;
			} while (xPermuteAnds_Joins(root, &Permutant)); // permute les jointures separees par des AND 
																										// pour mettres celles des autres tables d'abord

		*/

			uBOOL mustinverse;
			root = xRemoveRecordExist(root, mustinverse);

			root = xRegroupJoinPaths(root, true, context, nf);

			if (err == VE_OK)
				root = xTransformSeq(root, false); // regarde si les comparaisons indexees sont reliees par des OU a des comparaison sequentielles
											// et si oui, les transforme en sequentielles
			err = VTask::GetCurrent()->GetLastError();
				
			if (fic != nil)
			{
				Base4D *bd = fic->GetOwner();
				if (model->MustDisplay())
				{
					DebugMsg(L"  \n\n");
					if (root != nil)
						root->DisplayTree(bd, 0, &fResult, fQueryPlan);
					DebugMsg(fResult);
					if (context != nil)
					{
						context->SetQueryDescription(fResult);
						context->SetQueryPlan(fQueryPlan);
					}
					DebugMsg(L"  \n\n");



		#if 0
					CDB4DQueryPathNode* queryplan = BuildQueryPath(true, nil);
					if (testAssert(queryplan != nil))
					{
						VString sss;
						queryplan->BuildFullString(sss, 0);
						DebugMsg(sss);
						DebugMsg(L"  \n\n");

						queryplan->Release();
					}
		#endif
				}
			}
		}
	}
	else
		err = ThrowBaseError(VE_DB4D_QUERY_NOT_THE_RIGHT_MODEL, DBaction_ExecutingQuery);

	if (err != VE_OK)
	{
		err = ThrowBaseError(VE_DB4D_CANNOT_ANALYZE_QUERY, DBaction_ExecutingQuery);
	}

	return(err);
}


Selection* OptimizedQuery::Perform(Bittab* DejaSel, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, VError& err, 
								   DB4D_Way_of_Locking HowToLock, sLONG limit, Bittab* exceptions)
{
	Selection* sel = nil;
	Bittab *b = nil;
	err = VE_OK;

	if (context != nil && context->ShouldDescribeQuery() && fic->GetOwner()->GetStructure() != nil)
	{
		DescribeQuery(true);
	}

	if (fExecutionDescription != nil)
		fExecutionDescription->Release();
	
	if (IsQueryDescribed())
	{
		fExecutionDescription = new VValueBag();
	}
	else
		fExecutionDescription = nil;

	if (fic != nil && fic->GetDF() != nil)
		fMaxRecords = fic->GetDF()->GetMaxRecords(context);
	else
		fMaxRecords = -1;

	if (root != nil) 
	{
		root->AdjusteIntl(GetContextIntl(context));
		if (root->IsConst())
		{
			uBOOL res = root->GetResultConst();
			if (res == 1)
			{
				sel = fic->GetDF()->AllRecords(context, err);
			}
			else
			{
				sel = new PetiteSel(fic->GetDF());
			}
		}
		else
		{
			if (root->IsIndexe() && root->GetTyp() != NoeudIndexe && root->GetTyp() != NoeudWithArrayIndex)
				limit = 0;

			Bittab dejaLocked;
			Transaction* trans = nil;

			if (HowToLock == DB4D_Keep_Lock_With_Transaction)
			{
				trans = GetCurrentTransaction(context);
				if (trans != nil)
				{
					Bittab *locks = trans->GetKeptLocks(fic->GetDF(), err, false);
					if (locks != nil)
						err = dejaLocked.Or(locks);
				}
			}

			if (err == VE_OK)
			{
				Bittab nulls;
				err = root->Perform(this, &b, DejaSel, InProgress, fExecutionDescription, context, HowToLock, exceptions, limit, nulls, root);
			}

			if (b != nil && err == VE_OK)
			{
				Table* t = GetTargetFile();
				DataTable* df = t->GetDF();

				if (HowToLock > DB4D_Keep_Lock_With_Record)
				{
					if (HowToLock == DB4D_Keep_Lock_With_Transaction && trans != nil)
					{
						Bittab* locks = trans->GetKeptLocks(fic->GetDF(), err, false);
						if (locks != nil)
						{
							Bittab xlocks;
							err = xlocks.Or(locks);
							if (err == VE_OK)
								err = xlocks.moins(&dejaLocked);
							if (err == VE_OK)
								err = xlocks.moins(b);
							if (err == VE_OK)
								trans->UnlockSel(fic->GetDF(), &xlocks, false);
						}
					}

					if (err == VE_OK)
					{
						Boolean owned = false;
						sel = CreFromBittabPtr(df, b);
					}			
				}
				else
				{
					Boolean owned = false;
					sel = CreFromBittabPtr(df, b);
				}
			}

			if (err != VE_OK)
			{
				if (HowToLock == DB4D_Keep_Lock_With_Transaction && trans != nil)
				{
					StErrorContextInstaller errs(false);
					VError err2 = VE_OK;
					Bittab* locks = trans->GetKeptLocks(fic->GetDF(), err2, false);
					if (locks != nil)
					{
						Bittab xlocks;
						err2 = xlocks.Or(locks);
						if (err2 == VE_OK)
							err2 = xlocks.moins(&dejaLocked);
						if (err2 == VE_OK)
							trans->UnlockSel(fic->GetDF(), &xlocks, false);
					}
				}
			}

		}
	}
	
	if (b != nil)
		b->Release();

	if (context != nil && IsQueryDescribed())
	{
		VString xml;
		if (fExecutionDescription != nil)
		{
			context->SetQueryPath(fExecutionDescription);
			fExecutionDescription->DumpXML(xml, L"QueryExecution", true);
			context->SetQueryExecutionXML(xml);

#if debuglr
			DebugMsg(L"\n");
			DebugMsg(xml);
			DebugMsg(L"\n\n");
#endif
			VString fulltext;
			BuildQueryFulltext(fExecutionDescription, fulltext, 0);
			context->SetQueryExecution(fulltext);
#if debuglr
			DebugMsg(fulltext);
			DebugMsg(L"\n\n");
#endif

		}
	}

	if (err != VE_OK)
	{
		Table* t = GetTargetFile();
		if (t != nil)
			err = t->ThrowError(VE_DB4D_CANNOT_COMPLETE_QUERY, DBaction_ExecutingQuery);
		else
			err = ThrowBaseError(VE_DB4D_CANNOT_COMPLETE_QUERY, DBaction_ExecutingQuery);
	}

	return(sel);
}


VValueSingle* OptimizedQuery::Compute(LocalEntityRecord* erec, BaseTaskInfo* context, VError& err)
{
	if (root == nil)
		return nil;
	else
	{
		root->AdjusteIntl(GetContextIntl(context));
		return root->Compute(erec, context, err);
	}
}


void OptimizedQuery::BuildQueryFulltext(VValueBag* step, VString& fulltext, sLONG level)
{
	VBagArray* soussteps = step->GetElements(QueryKeys::steps);
	VString s,s2;
	Boolean okstep = true;
	if (!step->GetString(QueryKeys::description, s2))
		okstep = false;
	RechNode::FillLevelString(level, s);
	s += s2;
	fulltext += s;

	s.Clear();
	sLONG nb;
	step->GetLong(QueryKeys::recordsfounds, nb);
	s2.FromLong(nb);
	s += s2;
	s += L" record";
	if (nb > 1)
		s+=L"s";
	s += L" found in ";
	uLONG t;
	step->GetLong(QueryKeys::time, t);
	s2.FromLong(t);
	s += s2;
	s += " ms";
	
	if (soussteps == nil)
	{
		fulltext += L"   (";
		fulltext += s;
		fulltext += L")\n";
	}
	else
	{
		if (okstep)
			fulltext += L"\n";
		VIndex nbsous;
		nbsous = soussteps->GetCount();
		for (VIndex i = 1; i <= nbsous; i++)
		{
			VValueBag* sousstep = soussteps->GetNth(i);
			if (sousstep != nil)
			{
				BuildQueryFulltext(sousstep, fulltext, level+1);
			}
		}
		if (okstep)
		{
			RechNode::FillLevelString(level, s2);
			fulltext += s2;
			fulltext += L"--> ";
			fulltext += s;
			fulltext += L"\n";
		}
	}
}


Selection* OptimizedQuery::Perform(Selection* DejaSel, VDB4DProgressIndicator* InProgress, BaseTaskInfo* context, VError& err, 
								   DB4D_Way_of_Locking HowToLock, sLONG limit, Bittab* exceptions)
{
	Bittab *bb;
	Selection* sel;
	
	if (DejaSel == nil)
	{
		bb = nil;
		sel = Perform(bb, InProgress, context, err, HowToLock, limit, exceptions);
	}
	else
	{
		bb = DejaSel->GenereBittab(context, err);
		if (err == VE_OK)
			sel = Perform(bb, InProgress, context, err, HowToLock, limit, exceptions);
		ReleaseRefCountable(&bb);
	}
	
	return(sel);
}


CDB4DQueryPathNode* OptimizedQuery::BuildQueryPath(Boolean DejaSel, const CDB4DQueryPathModifiers* inModifiers)
{
	CDB4DQueryPathNode* result = nil;
	sLONG curvar = 0, finalsel = 0;
	sLONG numdejasel = 0;

	if (inModifiers == nil)
	{
		inModifiers = new VDB4DQueryPathModifiers();
	}
	else
		((CDB4DQueryPathModifiers*)inModifiers)->Retain();

	if (DejaSel)
		numdejasel = -1;

	if (root != nil)
	{
		result = new VDB4DQueryPathNode(nil, L"Query Plan", inModifiers, false);
		CDB4DQueryPathNode* subline = root->BuildQueryPath(false, fic, fic->GetOwner(), numdejasel, result, curvar, finalsel, inModifiers);
		//assert(subline != nil);
		if (inModifiers->isTempVarDisplay())
		{
			VString smess = L"Returns ";
			VString selnamefinal;
			BuildSelectionName(finalsel, selnamefinal, inModifiers);
			smess += selnamefinal;
			smess += L" as the resulting selection";
			subline = new VDB4DQueryPathNode(result, smess, inModifiers, false);
		}
		else
		{
			subline = new VDB4DQueryPathNode(result, L"End Query Plan", inModifiers, false);
		}
	}

	((CDB4DQueryPathModifiers*)inModifiers)->Release();

	return result;
}


VError OptimizedQuery::BuildQueryPathString(Boolean DejaSel, VString& outResult, const CDB4DQueryPathModifiers* inModifiers)
{
	VError err = VE_OK;

	if (root != nil) 
	{
		//err = root->BuildQueryPathString(DejaSel, outResult, inDelimiter, inTabulationSize);
	}

	return err;
}



/* ============================================================================== */


VDB4DQueryPathNode::VDB4DQueryPathNode(CDB4DQueryPathNode* parent, const VString& inData, const CDB4DQueryPathModifiers* inModifiers, Boolean inBefore)
{ 
	assert(inModifiers != nil);
	((CDB4DQueryPathModifiers*)inModifiers)->Retain();
	fModifiers = inModifiers;
	fParent = parent; 
	fData = inData; 
	if (parent != nil)
		parent->AddChild(this, inBefore);
}


VDB4DQueryPathNode::~VDB4DQueryPathNode()
{
	((CDB4DQueryPathModifiers*)fModifiers)->Release();
}


CDB4DQueryPathNode* VDB4DQueryPathNode::GetParent() const
{
	return fParent;
}


VError VDB4DQueryPathNode::BuildFullString(VString& outResult, sLONG level)
{
	VError err = VE_OK;

	ArrayQueryPathNode::Iterator cur = fBeforeChildren.First(), end = fBeforeChildren.End();
	for (;cur != end; cur++)
	{
		err = (*cur)->BuildFullString(outResult, level+1);
	}

	sLONG i,nb = fModifiers->GetTabSize() * level;
	VStr<64> Prefix;
	for (i=0; i<nb; i++)
		Prefix += fModifiers->GetTabDelimiter();

	outResult += Prefix;
	outResult += fData;
	outResult += fModifiers->GetLineDelimiter();

	cur = fAfterChildren.First();
	end = fAfterChildren.End();
	for (;cur != end; cur++)
	{
		err = (*cur)->BuildFullString(outResult, level+1);
	}


	return err;
}


const VString& VDB4DQueryPathNode::GetThisString() const
{
	return fData;
}


CDB4DQueryPathNode* VDB4DQueryPathNode::GetNthChild(sLONG Nth, Boolean inBefore) const
{
	if (inBefore)
	{
		if (testAssert(Nth >=0 && Nth < fBeforeChildren.GetCount()))
		{
			return fBeforeChildren[Nth];
		}
		else
			return nil;
	}
	else
	{
		if (testAssert(Nth >=0 && Nth < fAfterChildren.GetCount()))
		{
			return fAfterChildren[Nth];
		}
		else
			return nil;
	}
}


sLONG VDB4DQueryPathNode::GetCountChildren(Boolean inBefore) const
{
	if (inBefore)
		return fBeforeChildren.GetCount();
	else
		return fAfterChildren.GetCount();
}


VError VDB4DQueryPathNode::AddChild(CDB4DQueryPathNode* child, Boolean inBefore)
{
	if (inBefore)
	{
		if (fBeforeChildren.Add(child))
			return VE_OK;
		else
			return ThrowBaseError(memfull, DBaction_ExecutingQuery);
	}
	else
	{
		if (fAfterChildren.Add(child))
			return VE_OK;
		else
			return ThrowBaseError(memfull, DBaction_ExecutingQuery);
	}
}


void VDB4DQueryPathNode::SetString(const VString& data)
{
	fData = data;
}


/* ============================================================================== */


ColumnFormulae::ColumnFormulae()
{ 
	fWhatToDo = DB4D_Sum; 
	fCri = nil; 
	/*fInd = nil;*/ 
	count = 0;
	countDistinct = 0;
	fResultType = VK_EMPTY;
	fResult1 = 0;
	fResult2 = 0;
	fFirstTime = true;
	fCurrecValue = nil;
	fGenericResult = nil;
	fSpecialBool = false;
}


ColumnFormulae::~ColumnFormulae()
{
	/*
	if (fCurrecValue != nil)
		delete fCurrecValue;

	if (fCri != nil)
	{
		fCri->Release();
	}
	*/
	/*
	if (fInd != nil)
	{
		fInd->Release();
	}
	*/
}


void ColumnFormulae::Dispose()
{
	if (fCurrecValue != nil)
		delete fCurrecValue;

	if (fCri != nil)
	{
		fCri->Release();
	}

	if (fGenericResult != nil)
		delete fGenericResult;
}


VError ColumnFormulae::SetAction(DB4D_ColumnFormulae inWhatToDo, Field* InField)
{
	VError err = VE_OK;

	fWhatToDo = inWhatToDo;
	if (fCri != nil)
		fCri->Release();
	fCri = InField;
	if (fCri != nil)
		fCri->Retain();

	fResult1 = 0;
	fResult2 = 0.0;
	count = 0;
	fFirstTime = true;
	fSpecialBool = false;

	if (fCri == nil)
	{
		fResultType = VK_EMPTY;
		err = VE_DB4D_WRONGFIELDREF;
	}
	else
	{
		fResultType = (ValueKind)fCri->GetTyp();
		switch (fResultType)
		{
			case VK_BYTE:
			case VK_WORD:
			case VK_LONG:
			case VK_LONG8:
			case VK_SUBTABLE:
			case VK_SUBTABLE_KEY:
			case VK_BOOLEAN:
				if (fResultType == VK_BOOLEAN)
					fSpecialBool = true;
				fResultType = VK_LONG8;
				break;

			case VK_FLOAT:
			case VK_REAL:
			case VK_DURATION:
				// on ne fait rien car meme type
				break;

			default:
				//fResultType = VK_REAL;
				// now handles any kind
				if (fGenericResult != nil)
					delete fGenericResult;
				fGenericResult = (VValueSingle*)VValue::NewValueFromValueKind(fResultType);
				break;
		}
	}
	return err;
}


VError ColumnFormulae::CopyFrom(ColumnFormulae* other, sLONG nformule)
{
	fCri = other->fCri;
	if (fCri != nil)
		fCri->Retain();
	fResultType = other->fResultType;

	fWhatToDo = other->fWhatToDo;
	count = 0;
	countDistinct = 0;
	fResult2 = 0;
	fResult1 = 0;
	fFromNthFormule = nformule;
	fFirstTime = true;

	if (other->fCurrecValue == nil)
		fCurrecValue = nil;
	else
		fCurrecValue = other->fCurrecValue->Clone();

	if (other->fGenericResult == nil)
		fGenericResult = nil;
	else
		fGenericResult = (VValueSingle*)VValue::NewValueFromValueKind(other->fGenericResult->GetValueKind());

	return VE_OK;
}


VError ColumnFormulae::CopyResultBackInto(ColumnFormulas* into)
{
	ColumnFormulae* other = into->GetNthFormule(fFromNthFormule);
	switch (fResultType)
	{
		case VK_SUBTABLE:
		case VK_SUBTABLE_KEY:
		case VK_LONG8:
			other->fResult1 = fResult1;
			other->fPreviousValue1 = fPreviousValue1;
			break;

		case VK_FLOAT:
			other->fResult3 = fResult3;
			other->fPreviousValue3 = fPreviousValue3;
			break;

		case VK_DURATION:
			other->fResult4 = fResult4;
			other->fPreviousValue4 = fPreviousValue4;
			break;

		case VK_REAL:
			other->fResult2 = fResult2;
			other->fPreviousValue2 = fPreviousValue2;
			break;

		default:
			if (other->fGenericResult == nil)
				delete other->fGenericResult;
			other->fGenericResult = fGenericResult == nil ? nil : fGenericResult->Clone();
	}
	other->fFirstTime = fFirstTime;
	other->count = count;
	other->countDistinct = countDistinct;
	return VE_OK;
}


VError ColumnFormulae::Finalize()
{
	switch(fWhatToDo)
	{
		case DB4D_Count:
			fResultType = VK_LONG8;
			fResult1 = count;
			break;

		case DB4D_Count_distinct:
			fResultType = VK_LONG8;
			fResult1 = countDistinct;
			break;

		case DB4D_Average:
			if (count != 0 && !fFirstTime)
			{
				VFloat temp;
				switch (fResultType)
				{
					case VK_SUBTABLE:
					case VK_SUBTABLE_KEY:
					case VK_LONG8:
						fResultType = VK_REAL;
						fResult2 = (Real)fResult1 / (Real)count;
						fResult1 = fResult1 / count;
						break;

					case VK_REAL:
						fResult2 = fResult2 / (Real)count;
						break;

					case VK_FLOAT:
						temp = fResult3;
						temp.Divide(fResult3, (VFloat)count);
						break;

					case VK_DURATION:
						fResult4.Divide(count);
						break;
				}
				
			}
			break;

		case DB4D_Average_distinct:
			if (countDistinct != 0 && !fFirstTime)
			{
				VFloat temp;
				switch (fResultType)
				{
					case VK_SUBTABLE:
					case VK_SUBTABLE_KEY:
					case VK_LONG8:
						fResultType = VK_REAL;
						fResult2 = (Real)fResult1 / (Real)countDistinct;
						fResult1 = fResult1 / countDistinct;
						break;

					case VK_REAL:
						fResult2 = fResult2 / (Real)countDistinct;
						break;

					case VK_FLOAT:
						temp = fResult3;
						temp.Divide(fResult3, (VFloat)countDistinct);
						break;

					case VK_DURATION:
						fResult4.Divide(countDistinct);
						break;
					}

			}
			break;
	}

	return VE_OK;
}


VValueSingle* ColumnFormulae::GetResult() const
{
	VValueSingle* cv = nil;

	switch (fResultType)
	{
		case VK_SUBTABLE:
		case VK_SUBTABLE_KEY:
		case VK_LONG8:
			if (fSpecialBool && fWhatToDo != DB4D_Count && fWhatToDo != DB4D_Count_distinct)
			{
				cv = new VBoolean(fResult1 != 0);
			}
			else
				cv = new VLong8(fResult1);
			break;

		case VK_REAL:
			cv = new VReal(fResult2);
			break;

		case VK_FLOAT:
			cv = new VFloat(fResult3);
			break;

		case VK_DURATION:
			cv = new VDuration(fResult4);
			break;

		default:
			if (fGenericResult != nil)
				cv = fGenericResult->Clone();
	}

	if (cv != nil && fFirstTime && fWhatToDo != DB4D_Sum && fWhatToDo != DB4D_Sum_distinct && fWhatToDo != DB4D_Count && fWhatToDo != DB4D_Count_distinct)
	{
		cv->SetNull(true);
	}

	return cv;
}


void ColumnFormulae::SetResult(sLONG8 result, sLONG xcount)
{
	count = xcount;
	fFirstTime = (xcount == 0);
	fResult1 = result;
	fResult2 = result;
	fResult3.FromLong8(result);
	fResult4.FromLong8(result);
}


void ColumnFormulae::SetResult(Real result, sLONG xcount)
{
	count = xcount;
	fFirstTime = (xcount == 0);
	fResult1 = result;
	fResult2 = result;
	fResult3.FromReal(result);
	fResult4.FromReal(result);
}


VError ColumnFormulae::WriteToStream(VStream* toStream, Table* tt)
{
	VError err = toStream->PutLong(fCri == nil ? 0 :fCri->GetPosInRec());
	if (err == VE_OK)
	{
		err = toStream->PutLong((sLONG)fWhatToDo);
	}
	if (err == VE_OK)
		err = toStream->PutLong((sLONG)fResultType);
	if (err == VE_OK)
	{
		if (fCurrecValue == nil || fCurrecValue->IsNull())
			err = toStream->PutLong(0);
		else
		{
			err = toStream->PutLong(fCurrecValue->GetValueKind());
			if (err == VE_OK)
				err = fCurrecValue->WriteToStream(toStream);

		}
	}
	return err;
}


VError ColumnFormulae::ReadFromStream(VStream* fromStream, Table* tt)
{
	sLONG numcrit, xWhatToDo, xResultType;

	VError err = fromStream->GetLong(numcrit);
	if (err == VE_OK)
	{
		QuickReleaseRefCountable(fCri);
		if (numcrit == 0)
			fCri = nil;
		else
			fCri = tt->RetainField(numcrit);

		err = fromStream->GetLong(xWhatToDo);
		if (err == VE_OK)
		{
			fWhatToDo = (DB4D_ColumnFormulae)xWhatToDo;
			err = fromStream->GetLong(xResultType);
			fResultType = (ValueKind)xResultType;
		}
		if (err == VE_OK)
		{
			sLONG valtype;
			err = fromStream->GetLong(valtype);
			if (err == VE_OK)
			{
				if (fCurrecValue != nil)
					delete fCurrecValue;
				if (valtype == 0)
					fCurrecValue = nil;
				else
				{
					fCurrecValue = (VValueSingle*)VValue::NewValueFromValueKind((ValueKind)valtype);
					if (fCurrecValue == nil)
						err = ThrowBaseError(memfull);
					else
					{
						err = fCurrecValue->ReadFromStream(fromStream);
					}
				}
			}
		}
	}

	return err;
}


VError ColumnFormulae::WriteResultsToStream(VStream* toStream)
{
	VError err = toStream->PutLong8(fResult1);
	if (err == VE_OK)
		err = toStream->PutReal(fResult2);
	if (err == VE_OK)
		err = fResult3.WriteToStream(toStream);
	if (err == VE_OK)
		err = fResult4.WriteToStream(toStream);
	if (err == VE_OK)
		err = toStream->PutLong8(count);
	if (err == VE_OK)
		err = toStream->PutLong8(countDistinct);
	if (err == VE_OK)
		err = toStream->PutLong((sLONG)fResultType);
	if (err == VE_OK)
		err = toStream->PutByte(fFirstTime ? 1 : 0);
	return err;
}


VError ColumnFormulae::ReadResultsFromStream(VStream* fromStream)
{
	VError err = fromStream->GetLong8(fResult1);
	if (err == VE_OK)
		err = fromStream->GetReal(fResult2);
	if (err == VE_OK)
		err = fResult3.ReadFromStream(fromStream);
	if (err == VE_OK)
		err = fResult4.ReadFromStream(fromStream);
	if (err == VE_OK)
		err = fromStream->GetLong8(count);
	if (err == VE_OK)
		err = fromStream->GetLong8(countDistinct);
	sLONG xrestype;
	if (err == VE_OK)
		err = fromStream->GetLong(xrestype);
	fResultType = (ValueKind)xrestype;
	uBYTE cc;
	if (err == VE_OK)
		err = fromStream->GetByte(cc);
	fFirstTime = (cc == 1);
	return err;

}




/*
void ColumnFormulae::SetIndex(IndexInfo* ind)
{
	if (fInd != nil)
		fInd->Release();
	fInd = ind;
	if (fInd != nil)
		fInd->Retain();
}
*/


// ******************************************


ColumnFormulas::ColumnFormulas(Table* target) 
{ 
	fInd = nil;
	fCurrec = -1;
	fTarget = target; 
	if (fTarget != nil)
	{
		fTarget->Retain();
	}
}


ColumnFormulas::~ColumnFormulas() 
{ 
	for (Vx0ArrayOf<ColumnFormulae, 10>::Iterator cur = fActions.First(), end = fActions.End(); cur != end; cur++)
		cur->Dispose();

	if (fTarget != nil)
	{
		fTarget->Release();
	}
	if (fInd != nil)
	{
		fInd->ReleaseValid();
		fInd->Release();
	}
}


void ColumnFormulas::SetIndex(IndexInfo* ind)
{
	if (fInd != nil)
	{
		fInd->ReleaseValid();
		fInd->Release();
	}
	fInd = ind;
	if (fInd != nil)
		fInd->Retain();
}


void ColumnFormulas::SetTarget(Table* target)
{
	if (fTarget != nil)
		fTarget->Release();
	fTarget = target;
	if (fTarget != nil)
		fTarget->Retain();
}


VError ColumnFormulas::AddAction(DB4D_ColumnFormulae inWhatToDo, Field* InField)
{
	VError err = VE_OK;

	sLONG nb = fActions.GetCount();
	if (fActions.SetCount(nb+1))
	{
		err = fActions[nb].SetAction(inWhatToDo, InField);
	}
	else
		err = memfull;

	return err;
}


VError ColumnFormulas::ExecuteOnOneBtreeKey(IndexInfo* ind, const BTitemIndex* u, Boolean& stop, BaseTaskInfo* context)
{
	VError err = VE_OK;

	FicheInMem* dejaChargee = nil;
	if (context != nil)
	{
		dejaChargee = context->RetainRecAlreadyIn(ind->GetTargetTable()->GetDF(), u->GetQui());
	}
	sLONG i,nb = fActions.GetCount();
	for (i = 0; i<nb && err == VE_OK; i++)
	{
		ColumnFormulae* formule = &fActions[i];
		Field* cri = formule->GetField();
		sLONG typondisk = cri->GetTyp();
		void* data = (void*) &(u->data);
		DB4D_ColumnFormulae action = formule->GetAction();
		ValueKind typresult = formule->GetResultType();
		BTitemIndex* key = nil;
		Boolean isnull = false;

		if (u->GetQui() == fCurrec)
		{
			ValPtr cv = formule->GetCurrecValue();
			if (cv == nil || cv->IsNull())
				isnull = true;
			if (!isnull)
			{
				key = ind->BuildKey(dejaChargee, err, false);
				if (err == VE_OK)
					data = (void*) &(key->data);
			}
		}
		else
		{
			if (dejaChargee != nil)
			{
				ValPtr cv = dejaChargee->GetFieldValue(cri, err, false, true);
				if (cv == nil || cv->IsNull())
					isnull = true;
				if (!isnull)
				{
					key = ind->BuildKey(dejaChargee, err, false);
					if (err == VE_OK)
						data = (void*) &(key->data);
				}
			}
		}

		if (isnull || err != VE_OK /* tester si la cle est une valeur nulle*/)
		{
		}
		else // la cle n'est pas une valeur nulle
		{
			formule->AddCount();
			if (action != DB4D_Count)
			{
				sLONG8 x1;
				Real x2;
				Boolean isint = true;

				if (typondisk == DB4D_Integer16 || typondisk == DB4D_Integer32 || typondisk == DB4D_Integer64 
					|| typondisk == DB4D_Duration || typondisk == DB4D_Real)
				{
					switch(typondisk) 
					{
					case DB4D_Integer16:
						x1 = *((sWORD*)data);
						break;
					case DB4D_Integer32:
						x1 = *((sLONG*)data);
						break;
					case DB4D_Integer64:
						x1 = *((sLONG8*)data);
						break;
					case DB4D_Duration:
						x1 = *((sLONG8*)data);
						break;
					case DB4D_Real:
						x2 = *((Real*)data);
						isint = false;
						break;
					}

					switch (action)
					{
					case DB4D_Max:
						switch (typresult)
						{
						case VK_DURATION:
							if (isint)
								*(formule->GetResultDurationPtr()) = VDuration(x1);
							else
								*(formule->GetResultDurationPtr()) = VDuration((sLONG8)x2);

							break;

						case VK_FLOAT:
							if (isint)
								*(formule->GetResultFloatPtr()) = VFloat(x1);
							else
								*(formule->GetResultFloatPtr()) = VFloat(x2);
							break;

						case VK_REAL:
							if (isint)
								*(formule->GetResultRealPtr()) = x1;
							else
								*(formule->GetResultRealPtr()) = x2;
							break;

						case VK_SUBTABLE:
						case VK_SUBTABLE_KEY:
						case VK_LONG8:
							if (isint)
								*(formule->GetResultLong8Ptr()) = x1;
							else
								*(formule->GetResultLong8Ptr()) = (sLONG8)x2;
							break;

						}
						break;

					case DB4D_Min:
						if (formule->FirstTime())
						{
							switch (typresult)
							{
							case VK_DURATION:
								if (isint)
									*(formule->GetResultDurationPtr()) = VDuration(x1);
								else
									*(formule->GetResultDurationPtr()) = VDuration((sLONG8)x2);

								break;

							case VK_FLOAT:
								if (isint)
									*(formule->GetResultFloatPtr()) = VFloat(x1);
								else
									*(formule->GetResultFloatPtr()) = VFloat(x2);
								break;

							case VK_REAL:
								if (isint)
									*(formule->GetResultRealPtr()) = x1;
								else
									*(formule->GetResultRealPtr()) = x2;
								break;

							case VK_SUBTABLE:
							case VK_SUBTABLE_KEY:
							case VK_LONG8:
								if (isint)
									*(formule->GetResultLong8Ptr()) = x1;
								else
									*(formule->GetResultLong8Ptr()) = (sLONG8)x2;
								break;

							}
						}
						break;

					case DB4D_Average:
					case DB4D_Sum:
						switch (typresult)
						{
						case VK_DURATION:
							if (isint)
								formule->GetResultDurationPtr()->Add(VDuration(x1));
							else
								formule->GetResultDurationPtr()->Add(VDuration((sLONG8)x2));

							break;

						case VK_FLOAT:
							if (isint)
								formule->GetResultFloatPtr()->Add(*(formule->GetResultFloatPtr()), VFloat(x1));
							else
								formule->GetResultFloatPtr()->Add(*(formule->GetResultFloatPtr()), VFloat(x2));
							break;

						case VK_REAL:
							if (isint)
								*(formule->GetResultRealPtr()) = *(formule->GetResultRealPtr()) + x1;
							else
								*(formule->GetResultRealPtr()) = *(formule->GetResultRealPtr()) + x2;
							break;

						case VK_SUBTABLE:
						case VK_SUBTABLE_KEY:
						case VK_LONG8:
							if (isint)
								*(formule->GetResultLong8Ptr()) = *(formule->GetResultLong8Ptr()) + x1;
							else
								*(formule->GetResultLong8Ptr()) = *(formule->GetResultLong8Ptr()) + (sLONG8)x2;
							break;

						}
						break;

					case DB4D_Average_distinct:
					case DB4D_Sum_distinct:
					case DB4D_Count_distinct:
						switch (typresult)
						{
						case VK_DURATION:
							if (isint)
								formule->AddDistinctDuration(VDuration(x1));
							else
								formule->AddDistinctDuration(VDuration((sLONG8)x2));
							break;

						case VK_FLOAT:
							if (isint)
								formule->AddDistinctFloat(VFloat(x1));
							else
								formule->AddDistinctFloat(VFloat((sLONG8)x2));
							break;

						case VK_REAL:
							if (isint)
								formule->AddDistinctReal((Real)x1);
							else
								formule->AddDistinctReal(x2);
							break;

						case VK_SUBTABLE:
						case VK_SUBTABLE_KEY:
						case VK_LONG8:
							if (isint)
								formule->AddDistinctLong8(x1);
							else
								formule->AddDistinctLong8((sLONG8)x2);
							break;

						}
						break;

					} // end of switch action
				}
				else
				{
					ValPtr ch = ind->CreateVValueWithKey(u, err);
					if (ch != nil)
					{
						switch (action)
						{
							case DB4D_Max:
								switch (typondisk)
								{
								case VK_FLOAT:
									formule->GetResultFloatPtr()->FromValue(*ch);
									break;
								default:
									formule->SetGenericValue(ch);
									break;
								}
								break;

							case DB4D_Min:
								switch (typondisk)
								{
								case VK_FLOAT:
									if (formule->FirstTime())
										formule->GetResultFloatPtr()->FromValue(*ch);
									break;
								default:
									if (formule->FirstTime())
										formule->SetGenericValue(ch);
								}
								break;

							case DB4D_Average:
							case DB4D_Sum:
								switch (typondisk)
								{
								case VK_FLOAT:
									{
										VFloat temp;
										temp.FromValue(*ch);
										formule->GetResultFloatPtr()->Add(*(formule->GetResultFloatPtr()), temp);
									}
									break;
								}
								break;

							case DB4D_Average_distinct:
							case DB4D_Sum_distinct:
							case DB4D_Count_distinct:
								switch (typondisk)
								{
								case VK_FLOAT:
									{
										VFloat temp;
										temp.FromValue(*ch);
										formule->AddDistinctFloat(temp);
									}
									break;
								default:
									formule->AddDistinctGeneric(ch);
									break;
								}
								break;

						}
					}


				}

			}

			formule->FirstTime();
		}

		if (key != nil)
			ind->FreeKey(key);
	}

	if (dejaChargee != nil)
		dejaChargee->Release();

	return err;
}


Boolean ColumnFormulas::ExecuteOnOneRec(FicheOnDisk* ficD, BaseTaskInfo* context)
{
	Boolean ok = true;
	char buf[4096];

	sLONG i,nb = fActions.GetCount();
	for (i = 0; i<nb; i++)
	{
		ColumnFormulae* formule = &fActions[i];
		Field* cri = formule->GetField();

		if (cri != nil)
		{
			DB4D_ColumnFormulae action = formule->GetAction();
			sLONG typondisk;
			ValueKind typresult = formule->GetResultType();
			void* data = nil;
			
			if (ficD->getnumfic() == fCurrec)
			{
				ValPtr cv = formule->GetCurrecValue();
				if (cv == nil || cv->IsNull())
					data = nil;
				else
				{
					data = &buf[0];
					cv->WriteToPtr(data);
					typondisk = cv->GetValueKind();
				}
			}
			else
				data = (void*)(ficD->GetDataPtrForQuery(cri->GetPosInRec(), &typondisk, false));
		 
			if (data == nil)
			{
				 // ne rien faire si champ est null
			}
			else
			{
				formule->AddCount();
				if (action != DB4D_Count)
				{
					sLONG8 x1;
					Real x2;
					Boolean isint = true;

					if (typondisk == DB4D_Integer16 || typondisk == DB4D_Integer32 || typondisk == DB4D_Integer64 
						|| typondisk == DB4D_Duration || typondisk == DB4D_Real)
					{
						switch(typondisk) 
						{
							case DB4D_Integer16:
								x1 = *((sWORD*)data);
								break;
							case DB4D_Integer32:
								x1 = *((sLONG*)data);
								break;
							case DB4D_Integer64:
								x1 = *((sLONG8*)data);
								break;
							case DB4D_Duration:
								x1 = *((sLONG8*)data);
								break;
							case DB4D_Real:
								x2 = *((Real*)data);
								isint = false;
								break;
						}

						switch (action)
						{
							case DB4D_Average_distinct:
							case DB4D_Sum_distinct:
							case DB4D_Count_distinct:
								switch (typresult)
								{
								case VK_DURATION:
									if (isint)
										formule->AddDistinctDuration(VDuration(x1));
									else
										formule->AddDistinctDuration(VDuration((sLONG8)x2));
									break;

								case VK_FLOAT:
									if (isint)
										formule->AddDistinctFloat(VFloat(x1));
									else
										formule->AddDistinctFloat(VFloat((sLONG8)x2));
									break;

								case VK_REAL:
									if (isint)
										formule->AddDistinctReal((Real)x1);
									else
										formule->AddDistinctReal(x2);
									break;

								case VK_SUBTABLE:
								case VK_SUBTABLE_KEY:
								case VK_LONG8:
									if (isint)
										formule->AddDistinctLong8(x1);
									else
										formule->AddDistinctLong8((sLONG8)x2);
									break;

								}
								break;

							case DB4D_Average:
							case DB4D_Sum:
								switch (typresult)
								{
									case VK_DURATION:
										if (isint)
											formule->GetResultDurationPtr()->Add(VDuration(x1));
										else
											formule->GetResultDurationPtr()->Add(VDuration((sLONG8)x2));

										break;

									case VK_FLOAT:
										if (isint)
											formule->GetResultFloatPtr()->Add(*(formule->GetResultFloatPtr()), VFloat(x1));
										else
											formule->GetResultFloatPtr()->Add(*(formule->GetResultFloatPtr()), VFloat(x2));
										break;

									case VK_REAL:
										if (isint)
											*(formule->GetResultRealPtr()) = *(formule->GetResultRealPtr()) + x1;
										else
											*(formule->GetResultRealPtr()) = *(formule->GetResultRealPtr()) + x2;
										break;

									case VK_SUBTABLE:
									case VK_SUBTABLE_KEY:
									case VK_LONG8:
										if (isint)
											*(formule->GetResultLong8Ptr()) = *(formule->GetResultLong8Ptr()) + x1;
										else
											if (isint)
												*(formule->GetResultLong8Ptr()) = *(formule->GetResultLong8Ptr()) + (sLONG8)x2;
										break;

								}
								break;

							case DB4D_Min:
								switch (typresult)
								{
									case VK_DURATION:
										if (isint)
										{
											if (formule->FirstTime() || *(formule->GetResultDurationPtr()) > VDuration(x1) )
												*(formule->GetResultDurationPtr()) = VDuration(x1);
										}
										else
										{
											if (formule->FirstTime() || *(formule->GetResultDurationPtr()) > VDuration((sLONG8)x2) )
												*(formule->GetResultDurationPtr()) = VDuration((sLONG8)x2);
										}
										break;

									case VK_FLOAT:
										if (isint)
										{
											if (formule->FirstTime() || *(formule->GetResultFloatPtr()) > VFloat(x1) )
												*(formule->GetResultFloatPtr()) = VFloat(x1);
										}
										else
										{
											if (formule->FirstTime() || *(formule->GetResultFloatPtr()) > VFloat(x2) )
												*(formule->GetResultFloatPtr()) = VFloat(x2);
										}
										break;

									case VK_REAL:
										if (isint)
											x2 = x1;
										if (formule->FirstTime() || *(formule->GetResultRealPtr()) > x2 )
											*(formule->GetResultRealPtr()) = x2;
										break;

									case VK_SUBTABLE:
									case VK_SUBTABLE_KEY:
									case VK_LONG8:
										if (!isint)
											x1 = (sLONG8)x2;
										if (formule->FirstTime() || *(formule->GetResultLong8Ptr()) > x1 )
											*(formule->GetResultLong8Ptr()) = x1;
										break;

								}
								break;

							case DB4D_Max:
								switch (typresult)
								{
								case VK_DURATION:
									if (isint)
									{
										if (formule->FirstTime() || *(formule->GetResultDurationPtr()) < VDuration(x1) )
											*(formule->GetResultDurationPtr()) = VDuration(x1);
									}
									else
									{
										if (formule->FirstTime() || *(formule->GetResultDurationPtr()) < VDuration((sLONG8)x2) )
											*(formule->GetResultDurationPtr()) = VDuration((sLONG8)x2);
									}
									break;

								case VK_FLOAT:
									if (isint)
									{
										if (formule->FirstTime() || *(formule->GetResultFloatPtr()) < VFloat(x1) )
											*(formule->GetResultFloatPtr()) = VFloat(x1);
									}
									else
									{
										if (formule->FirstTime() || *(formule->GetResultFloatPtr()) < VFloat(x2) )
											*(formule->GetResultFloatPtr()) = VFloat(x2);
									}
									break;

								case VK_REAL:
									if (isint)
										x2 = x1;
									if (formule->FirstTime() || *(formule->GetResultRealPtr()) < x2 )
										*(formule->GetResultRealPtr()) = x2;
									break;

								case VK_SUBTABLE:
								case VK_SUBTABLE_KEY:
								case VK_LONG8:
									if (!isint)
										x1 = (sLONG8)x2;
									if (formule->FirstTime() || *(formule->GetResultLong8Ptr()) < x1 )
										*(formule->GetResultLong8Ptr()) = x1;
									break;

								}
								break;

						} // end of switch action
					}
					else
					{
						ValPtr ch = NewValPtr(typresult, data, typondisk, ficD->GetOwner(), context);
						switch (action)
						{
							case DB4D_Average_distinct:
							case DB4D_Sum_distinct:
							case DB4D_Count_distinct:
								switch (typresult)
								{
									case VK_DURATION:
										formule->AddDistinctDuration(*(VDuration*)ch);
										break;

									case VK_FLOAT:
										formule->AddDistinctFloat(*(VFloat*)ch);
										break;

									case VK_REAL:
										formule->AddDistinctReal(((VReal*)ch)->GetReal());
										break;

									case VK_SUBTABLE:
									case VK_SUBTABLE_KEY:
									case VK_LONG8:
										formule->AddDistinctLong8(((VLong8*)ch)->GetLong8());
										break;

									default:
										formule->AddDistinctGeneric(ch);
										break;

								}
								break;

							case DB4D_Average:
							case DB4D_Sum:
								switch (typresult)
								{
									case VK_DURATION:
										formule->GetResultDurationPtr()->Add(*(VDuration*)ch);
										break;

									case VK_FLOAT:
										formule->GetResultFloatPtr()->Add(*(formule->GetResultFloatPtr()), *(VFloat*)ch);
										break;

									case VK_REAL:
										*(formule->GetResultRealPtr()) = *(formule->GetResultRealPtr()) + ((VReal*)ch)->GetReal();
										break;

									case VK_SUBTABLE:
									case VK_SUBTABLE_KEY:
									case VK_LONG8:
										*(formule->GetResultLong8Ptr()) = *(formule->GetResultLong8Ptr()) + ((VLong8*)ch)->GetLong8();
										break;

								}
								break;

							case DB4D_Min:
								switch (typresult)
								{
								case VK_DURATION:
									if (formule->FirstTime() || *(formule->GetResultDurationPtr()) > *(VDuration*)ch )
										*(formule->GetResultDurationPtr()) = *(VDuration*)ch;
									break;

								case VK_FLOAT:
									if (formule->FirstTime() || *(formule->GetResultFloatPtr()) > *(VFloat*)ch )
										*(formule->GetResultFloatPtr()) = *(VFloat*)ch;
									break;

								case VK_REAL:
									if (formule->FirstTime() || *(formule->GetResultRealPtr()) > ((VReal*)ch)->GetReal() )
										*(formule->GetResultRealPtr()) = ((VReal*)ch)->GetReal();
									break;

								case VK_SUBTABLE:
								case VK_SUBTABLE_KEY:
								case VK_LONG8:
									if (formule->FirstTime() || *(formule->GetResultLong8Ptr()) > ((VLong8*)ch)->GetLong8() )
										*(formule->GetResultLong8Ptr()) = ((VLong8*)ch)->GetLong8();
									break;

								default:
									if (formule->FirstTime() || formule->GetGenericValue()->CompareTo(*ch, true) == CR_BIGGER)
										formule->SetGenericValue(ch);
									break;

								}
								break;

							case DB4D_Max:
								switch (typresult)
								{
								case VK_DURATION:
									if (formule->FirstTime() || *(formule->GetResultDurationPtr()) < *(VDuration*)ch )
										*(formule->GetResultDurationPtr()) = *(VDuration*)ch;
									break;

								case VK_FLOAT:
									if (formule->FirstTime() || *(formule->GetResultFloatPtr()) < *(VFloat*)ch )
										*(formule->GetResultFloatPtr()) = *(VFloat*)ch;
									break;

								case VK_REAL:
									if (formule->FirstTime() || *(formule->GetResultRealPtr()) < ((VReal*)ch)->GetReal() )
										*(formule->GetResultRealPtr()) = ((VReal*)ch)->GetReal();
									break;

								case VK_SUBTABLE:
								case VK_SUBTABLE_KEY:
								case VK_LONG8:
									if (formule->FirstTime() || *(formule->GetResultLong8Ptr()) < ((VLong8*)ch)->GetLong8() )
										*(formule->GetResultLong8Ptr()) = ((VLong8*)ch)->GetLong8();
									break;

								default:
									if (formule->FirstTime() || formule->GetGenericValue()->CompareTo(*ch, true) == CR_SMALLER)
										formule->SetGenericValue(ch);
									break;

								}
								break;


						} // end of switch action

						delete ch;
					}

				}
				formule->FirstTime();
			}
		}
	}

	return ok;
}



Boolean ColumnFormulas::ExecuteOnOneRec(FicheInMem* fic, BaseTaskInfo* context)
{
	Boolean ok = true;

	sLONG i,nb = fActions.GetCount();
	for (i = 0; i<nb; i++)
	{
		ColumnFormulae* formule = &fActions[i];
		Field* cri = formule->GetField();

		if (cri != nil)
		{
			DB4D_ColumnFormulae action = formule->GetAction();
			//sLONG typondisk;
			ValueKind typresult = formule->GetResultType();
			VError err2 = VE_OK;
			VValueSingle* cv;
			if (fic->GetNum() == fCurrec)
				cv = formule->GetCurrecValue();
			else
				cv = fic->GetFieldValue(cri, err2, false, true);

			if (cv == nil || cv->IsNull())
			{
				// ne rien faire si champ est null
			}
			else
			{
				formule->AddCount();
				if (action != DB4D_Count)
				{
					VDuration tempdur;
					VFloat tempfloat;
					VReal temprealx;
					Real tempreal;
					VLong8 templong8x;
					sLONG8 templong8;

					switch (typresult)
					{
						case VK_DURATION:
							tempdur.FromValue(*cv);
							break;

						case VK_FLOAT:
							tempfloat.FromValue(*cv);
							break;

						case VK_REAL:
							temprealx.FromValue(*cv);
							tempreal = temprealx.GetReal();
							break;

						case VK_SUBTABLE:
						case VK_SUBTABLE_KEY:
						case VK_LONG8:
							templong8x.FromValue(*cv);
							templong8 = templong8x.GetLong8();
							break;
					}

					switch (action)
					{
						case DB4D_Average:
						case DB4D_Sum:
							switch (typresult)
							{
								case VK_DURATION:
									formule->GetResultDurationPtr()->Add(tempdur);
									break;

								case VK_FLOAT:
									formule->GetResultFloatPtr()->Add(*(formule->GetResultFloatPtr()), tempfloat);
									break;

								case VK_REAL:
									*(formule->GetResultRealPtr()) = *(formule->GetResultRealPtr()) + tempreal;
									break;

								case VK_SUBTABLE:
								case VK_SUBTABLE_KEY:
								case VK_LONG8:
									*(formule->GetResultLong8Ptr()) = *(formule->GetResultLong8Ptr()) + templong8;
									break;
							}
							break;

						case DB4D_Min:
							switch (typresult)
							{
								case VK_DURATION:
									if (formule->FirstTime() || *(formule->GetResultDurationPtr()) > tempdur )
										*(formule->GetResultDurationPtr()) = tempdur;
									break;

								case VK_FLOAT:
										if (formule->FirstTime() || *(formule->GetResultFloatPtr()) >tempfloat )
											*(formule->GetResultFloatPtr()) = tempfloat;
									break;

								case VK_REAL:
									if (formule->FirstTime() || *(formule->GetResultRealPtr()) > tempreal )
										*(formule->GetResultRealPtr()) = tempreal;
									break;

								case VK_SUBTABLE:
								case VK_SUBTABLE_KEY:
								case VK_LONG8:
									if (formule->FirstTime() || *(formule->GetResultLong8Ptr()) > templong8 )
										*(formule->GetResultLong8Ptr()) = templong8;
									break;

								default:
									if (formule->FirstTime() || formule->GetGenericValue()->CompareTo(*cv, true) == CR_BIGGER)
										formule->SetGenericValue(cv);
									break;

							}
							break;

						case DB4D_Max:
							switch (typresult)
							{
								case VK_DURATION:
									if (formule->FirstTime() || *(formule->GetResultDurationPtr()) < tempdur )
										*(formule->GetResultDurationPtr()) = tempdur;
									break;

								case VK_FLOAT:
									if (formule->FirstTime() || *(formule->GetResultFloatPtr()) <tempfloat )
										*(formule->GetResultFloatPtr()) = tempfloat;
									break;

								case VK_REAL:
									if (formule->FirstTime() || *(formule->GetResultRealPtr()) < tempreal )
										*(formule->GetResultRealPtr()) = tempreal;
									break;

								case VK_SUBTABLE:
								case VK_SUBTABLE_KEY:
								case VK_LONG8:
									if (formule->FirstTime() || *(formule->GetResultLong8Ptr()) < templong8 )
										*(formule->GetResultLong8Ptr()) = templong8;
									break;

								default:
									if (formule->FirstTime() || formule->GetGenericValue()->CompareTo(*cv, true) == CR_SMALLER)
										formule->SetGenericValue(cv);
									break;

							}
							break;

					} // end of switch action
				}

				formule->FirstTime();
			}
		}
	}

	return ok;
}


VError ColumnFormulas::CopyActionFrom(ColumnFormulae *from, sLONG nformule)
{
	fActions.AddNSpacesWithConstructor(1);
	fActions[fActions.GetCount()-1].CopyFrom(from, nformule);
	
	return VE_OK;
}


VError ColumnFormulas::CopyResultBackInto(ColumnFormulas* into)
{
	sLONG i,nb = fActions.GetCount();
	for (i = 0; i<nb; i++)
	{
		fActions[i].CopyResultBackInto(into);
	}
	return VE_OK;
}


VError ColumnFormulas::Finalize()
{
	sLONG i,nbcol = fActions.GetCount();
	for (i = 0; i<nbcol; i++)
	{
		fActions[i].Finalize();
	}

	return VE_OK;
}


VError ColumnFormulas::WriteToStream(VStream* toStream, Base4D* db)
{
	VError err = toStream->PutLong(fTarget->GetNum());
	if (err == VE_OK)
		err = toStream->PutLong(fCurrec);
	if (err == VE_OK)
	{
		sLONG nb = fActions.GetCount();
		err = toStream->PutLong(nb);
		for (sLONG i = 0; i < nb && err == VE_OK; i++)
		{
			err = fActions[i].WriteToStream(toStream, fTarget);
		}
	}
	return err;
}


VError ColumnFormulas::ReadFromStream(VStream* fromStream, Base4D* db)
{
	sLONG numtable;
	VError err = fromStream->GetLong(numtable);
	if (err == VE_OK)
		err = fromStream->GetLong(fCurrec);
	if (err == VE_OK)
	{
		QuickReleaseRefCountable(fTarget);
		fTarget = db->RetainTable(numtable);
		if (fTarget != nil)
		{
			sLONG nb;
			err = fromStream->GetLong(nb);
			if (err == VE_OK)
			{
				if (fActions.SetCount(nb))
				{
					for (sLONG i = 0; i < nb && err == VE_OK; i++)
					{
						err = fActions[i].ReadFromStream(fromStream, fTarget);
					}
				}
				else
					err = ThrowBaseError(memfull, noaction);
			}
		}
		else
			err = ThrowBaseError(VE_DB4D_WRONGTABLEREF, noaction);
	}
	return err;
}


VError ColumnFormulas::WriteResultsToStream(VStream* toStream)
{
	sLONG nb = fActions.GetCount();
	VError err = toStream->PutLong(nb);
	for (sLONG i = 0; i < nb && err == VE_OK; i++)
	{
		err = fActions[i].WriteResultsToStream(toStream);
	}

	return err;
}


VError ColumnFormulas::ReadResultsFromStream(VStream* fromStream)
{
	sLONG nb;
	VError err = fromStream->GetLong(nb);
	if (err == VE_OK)
	{
		assert(nb == fActions.GetCount());
		if (nb > fActions.GetCount())
			nb = fActions.GetCount();

		for (sLONG i = 0; i < nb && err == VE_OK; i++)
		{
			err = fActions[i].ReadResultsFromStream(fromStream);
		}
	}

	return err;
}


void ColumnFormulas::SetCurrec(FicheInMem* currec)
{
	if (currec != nil && currec->GetNum() >= 0)
	{
		fCurrec = currec->GetNum();
		sLONG nb = fActions.GetCount();
		for (sLONG i = 0; i < nb; i++)
		{
			VError err = VE_OK;
			ColumnFormulae* formule = &fActions[i];
			ValueKind resultType = formule->GetResultType();
			bool mustdelete = false;
			ValPtr cv = currec->GetFieldValue(formule->GetField(), err, false, true);
			if (cv != nil)
			{
				if (cv->GetValueKind() != resultType)
				{
					cv = cv->ConvertTo(resultType);
					mustdelete = true;
				}
			}
			formule->SetCurrecvalue(cv);
			if (mustdelete)
				delete cv;
		}
	}
	else
		fCurrec = -1;
}


VError ColumnFormulas::Execute(Selection* Sel, BaseTaskInfo* context, VDB4DProgressIndicator* InProgress, FicheInMem* currec, bool keepSel)
{
	Vx0ArrayOf<ColumnFormulas, 10> indFormules;
	if (currec != (FicheInMem*)-1)
		SetCurrec(currec);

	VError err = VE_OK;
	if (fTarget != nil)
	{
		if (fTarget->IsRemote())
		{
			Base4D* db = fTarget->GetOwner();

			IRequest* req = db->CreateRequest(context == nil ? nil : context->GetEncapsuleur(), kRangeReqDB4D + Req_ExecuteColumnFormulas);
			if (req == nil)
				err = ThrowBaseError(memfull, noaction);
			else
			{
				req->PutBaseParam(db);
				req->PutThingsToForget(VDBMgr::GetManager(), context);
				err = req->PutColumnFormulasParam(this, db);
				req->PutSelectionParam(Sel, context == nil ? nil : context->GetEncapsuleur());
				req->PutProgressParam(InProgress);
				if (err == VE_OK)
					err = req->GetLastError();
				if (err == VE_OK)
				{
					err = req->Send();
					if (err== VE_OK)
						err = req->GetUpdatedInfo(db, context);

					if (err == VE_OK)
					{
						err = req->GetColumnFormulasResultReply(this);
					}
				}
				req->Release();
			}
		}
		else
		{
			DataTable* df = fTarget->GetDF();
			if (df != nil)
			{
				Boolean toutindex = true;
				sLONG i,nbcol = fActions.GetCount();
				Base4D* db = fTarget->GetOwner();
				
				if (nbcol>0)
				{
					for (i = 0; i<nbcol; i++)
					{
						Field* cri = fActions[i].GetField();
						IndexInfo* ind = fTarget->FindAndRetainIndexSimple(cri->GetPosInRec(), true, true, context);
						if (ind == nil)
						{
							toutindex = false;
							break;
						}
						else
						{
							if ((ind->GetNBDiskAccessForAScan(true)/3) > (sLONG8)Sel->GetQTfic() * fTarget->GetSeqRatioCorrector() )
							{
								toutindex = false;
								ind->ReleaseValid();
								ind->Release();
								break;
							}
							ColumnFormulas* deja = nil;
							sLONG j,nb = indFormules.GetCount();
							for (j = 0; j < nb; j++)
							{
								if (indFormules[j].GetIndex() == ind)
								{
									deja = &indFormules[j];
									break;
								}
							}
							if (deja == nil)
							{
								indFormules.AddNSpacesWithConstructor(1);
								deja = &indFormules[nb];
								deja->SetIndex(ind);
							}
							deja->CopyActionFrom(&fActions[i],i);
							deja->fCurrec = fCurrec;
							ind->Release();
						}

					}

					Bittab* filtre = nil;
					if (!keepSel)
						filtre = Sel->GenereBittab(context, err);
					if (filtre != nil || keepSel)
					{
						if (toutindex)
						{
							if (keepSel)
								filtre = Sel->GenereBittab(context, err);
							StErrorContextInstaller errs(false);

							sLONG nbsub = indFormules.GetCount();
							for (i = 0; i<nbsub; i++)
							{
								IndexInfo* ind = indFormules[i].GetIndex();
								err = ind->CalculateColumnFormulas(&indFormules[i], context, filtre, InProgress);
								if (err != VE_OK)
								{
									toutindex = false;
									break;
								}
							}

							if (toutindex)
							{
								for (i = 0; i<nbsub; i++)
								{
									indFormules[i].CopyResultBackInto(this);
								}
							}
							else
							{
								err = VE_OK; // on remet l'erreur a zero pour reessayer en sequentiel
							}
						}
					
				
						if (!toutindex)
						{
							Bittab deja;
							Bittab alreadyloaded;

							if (!keepSel)
							{
								err = context->GetLoadedRecords(df, alreadyloaded, filtre);

								if (err == VE_OK)
									err = deja.aggrandit(df->GetMaxRecords(context));

								if (err == VE_OK && filtre->GetRefCount() != 1)
								{
									Bittab* filtre2 = new Bittab;
									if (filtre2 == nil)
										err = df->ThrowError(memfull, DBaction_ExecutingColumnFormula);
									else
									{
										err = filtre2->Or(filtre);
										filtre->Release();
										filtre = filtre2;
									}
								}

								if (err == VE_OK)
									err = filtre->moins(&alreadyloaded);

								if (false && err == VE_OK)
								{
									Transaction* trans = GetCurrentTransaction(context);
									Bittab* b = nil;
									Bittab* b2 = nil;
									if (trans != nil)
									{
										b = trans->GetSavedRecordIDs(fTarget->GetNum(), err, false);
										b2 = trans->GetDeletedRecordIDs(fTarget->GetNum(), err, false);
									}

									if (b != nil || b2 != nil)
									{
										Bittab newfiltre;
										err = newfiltre.Or(filtre);
										if (b != nil && err == VE_OK)
										{
											err = newfiltre.moins(b);
										}

										if (b2 != nil && err == VE_OK)
										{
											err = newfiltre.moins(b2);
										}
										if (err == VE_OK)
										{
											StLockerRead lock(df);
											OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
											df->CalculateFormulaFromCache(this, &newfiltre, deja, context, curstack);
										}

									}
									else
									{
										StLockerRead lock(df);
										OccupableStack* curstack = gOccPool.GetStackForCurrentThread();
										df->CalculateFormulaFromCache(this, filtre, deja, context, curstack);
									}
								}

								if (err == VE_OK)
								{
									err = filtre->moins(&deja);
								}

							}

							if (err == VE_OK)
							{
								ReadAheadBuffer* buf = db->NewReadAheadBuffer();
								Selection* reelsel = nil;

								if (keepSel)
								{
									reelsel = RetainRefCountable(Sel);
								}
								else
								{
									reelsel = new BitSel(df, filtre);
								}

								SelectionIterator seliter(reelsel);

								if (df->AcceptNotFullRecord())
								{
									FicheOnDisk *ficD;
									if(InProgress)
									{
										XBOX::VString session_title;
										gs(1005,30,session_title);	// computing statistics
										InProgress->BeginSession(filtre->NbTotalBits(),session_title);
									}
									i = seliter.FirstRecord();
									//i=filtre->FindNextBit(0);
									while (i != -1)
									{
										if (InProgress != nil)
											InProgress->Progress(i);

										if (MustStopTask(InProgress))
											err = df->ThrowError(VE_DB4D_ACTIONCANCELEDBYUSER, DBaction_ExecutingColumnFormula);

										if (err == VE_OK)
										{
											ficD = df->LoadNotFullRecord(i, err, DB4D_Do_Not_Lock, context, false, buf);
											if (ficD != nil)
											{
												if (err == VE_OK)
												{
													ExecuteOnOneRec(ficD, context);
												}
												//ficD->FreeAfterUse();
												ficD->Release();
											}
										}
										//i=filtre->FindNextBit(i+1);
										i = seliter.NextRecord();
										if (err != VE_OK)
											break;
									}
									if(InProgress)
										InProgress->EndSession();
								}
								else
								{
									FicheInMem *fic;
									if(InProgress)
									{
										XBOX::VString session_title;
										gs(1005,30,session_title);	// computing statistics
										InProgress->BeginSession(filtre->NbTotalBits(),session_title);
									}
									//i=filtre->FindNextBit(0);
									i = seliter.FirstRecord();
									while (i != -1)
									{
										if (InProgress != nil)
											InProgress->Progress(i);

										if (MustStopTask(InProgress))
											err = df->ThrowError(VE_DB4D_ACTIONCANCELEDBYUSER, DBaction_ExecutingColumnFormula);

										if (err == VE_OK)
										{
											fic = df->LoadRecord(i, err, DB4D_Do_Not_Lock, context, false, false, buf);
											if (fic != nil)
											{
												if (err == VE_OK)
												{
													ExecuteOnOneRec(fic, context);
												}
												fic->Release();
											}
										}
										//i=filtre->FindNextBit(i+1);
										i = seliter.NextRecord();
										if (err != VE_OK)
											break;
									}
									if(InProgress)
										InProgress->EndSession();
								}

								if (err == VE_OK && !keepSel)
								{
									i=alreadyloaded.FindNextBit(0);
									while (i != -1)
									{
										FicheInMem *fic = df->LoadRecord(i, err, /*DB4D_Keep_Lock_With_Record*/ DB4D_Do_Not_Lock, context, false, false, nil);
										if (fic != nil)
										{
											if (err == VE_OK)
											{
												ExecuteOnOneRec(fic, context);
											}
											fic->Release();
										}
										i=alreadyloaded.FindNextBit(i+1);
										if (err != VE_OK)
											break;
									}
								}

								if (buf != nil)
									buf->Release();

								QuickReleaseRefCountable(reelsel);
							}

						}

						if (err == VE_OK)
						{
							Finalize();
						}

						ReleaseRefCountable(&filtre);
					}
				}
			}
		}
	}
	else
		err = ThrowBaseError(VE_DB4D_WRONGTABLEREF, DBaction_ExecutingColumnFormula);

	if (err != VE_OK)
		err = ThrowBaseError(VE_DB4D_CANNOT_COMPLETE_FORMULA_ON_COLUMN, DBaction_ExecutingColumnFormula);

	return err;
}


sLONG8 ColumnFormulas::GetResultAsLong8(sLONG inColumnNumber) const
{
	if (inColumnNumber>=0 && inColumnNumber<fActions.GetCount())
	{
		ColumnFormulae* fo = const_cast<ColumnFormulae*>(&fActions[inColumnNumber]);
		if (fo->GetAction() == DB4D_Count)
			return fo->GetCount();
		else
			return *(fo->GetResultLong8Ptr());
	}
	else
		return 0;
}


Real ColumnFormulas::GetResultAsReal(sLONG inColumnNumber) const
{
	if (inColumnNumber>=0 && inColumnNumber<fActions.GetCount())
	{
		ColumnFormulae* fo = const_cast<ColumnFormulae*>(&fActions[inColumnNumber]);
		return *(fo->GetResultRealPtr());
	}
	else
		return 0.0;
}


void ColumnFormulas::GetResultAsFloat(sLONG inColumnNumber, VFloat& outResult) const
{
	if (inColumnNumber>=0 && inColumnNumber<fActions.GetCount())
	{
		ColumnFormulae* fo = const_cast<ColumnFormulae*>(&fActions[inColumnNumber]);
		outResult = *(fo->GetResultFloatPtr());
	}
}


void ColumnFormulas::GetResultAsDuration(sLONG inColumnNumber, VDuration& outResult) const
{
	if (inColumnNumber>=0 && inColumnNumber<fActions.GetCount())
	{
		ColumnFormulae* fo = const_cast<ColumnFormulae*>(&fActions[inColumnNumber]);
		outResult = *(fo->GetResultDurationPtr());
	}
}

VValueSingle* ColumnFormulas::GetResult(sLONG inColumnNumber) const
{
	if (inColumnNumber>=0 && inColumnNumber<fActions.GetCount())
	{
		return fActions[inColumnNumber].GetResult();
	}
	else
	{
		return nil;
	}
}




//=================================================================================


#if 0
VErrorDB4D DB4DArrayOfValuesCollection::SetCollectionSize(sLONG size)
{
	if (fValues->SetCount(size, nil))
		return VE_OK;
	else
		return ThrowBaseError(memfull, DBaction_BuildingArrayOfValues);
}


sLONG DB4DArrayOfValuesCollection::GetCollectionSize()
{
	return fValues->GetCount();
}


sLONG DB4DArrayOfValuesCollection::GetNumberOfColumns()
{
	return 1;
}


bool DB4DArrayOfValuesCollection::AcceptRawData()
{
	return false;
}

VErrorDB4D DB4DArrayOfValuesCollection::SetNthElementRawData(RecIDType ElemNumber, sLONG ColumnNumber, const void *inRawValue, ValueKind inRawValueKind, bool *outRejected)
{
	return VE_UNIMPLEMENTED;
}


CDB4DField* DB4DArrayOfValuesCollection::GetColumnRef(sLONG ColumnNumber)
{
	return nil;
}


VErrorDB4D DB4DArrayOfValuesCollection::SetNthElement(sLONG ElemNumber, sLONG ColumnNumber, const XBOX::VValueSingle& inValue)
{
	VError err = VE_OK;

	assert(ColumnNumber == 1);
	VValueSingle* cv = (*fValues)[ElemNumber-1];
	if (cv == nil)
	{
		cv = (VValueSingle*)(inValue.Clone());
		(*fValues)[ElemNumber-1] = cv;
		if (cv == nil)
			err = ThrowBaseError(memfull, DBaction_BuildingArrayOfValues);
	}
	else
	{
		cv->FromValue(inValue);
	}
	return err;
}


VErrorDB4D DB4DArrayOfValuesCollection::AddOneElement(sLONG ColumnNumber, const XBOX::VValueSingle& inValue)
{
	VError err = VE_OK;


	sLONG ElemNumber = fValues->GetCount()+1;
	err = SetCollectionSize(ElemNumber);
	if (err == VE_OK)
	{
		assert(ColumnNumber == 1);
		VValueSingle* cv = (VValueSingle*)(inValue.Clone());
		(*fValues)[ElemNumber-1] = cv;
		if (cv == nil)
			err = ThrowBaseError(memfull, DBaction_BuildingArrayOfValues);
	}
	return err;
}


VErrorDB4D DB4DArrayOfValuesCollection::GetNthElement(sLONG ElemNumber, sLONG ColumnNumber, const XBOX::VValueSingle*& outValue, bool *outDisposeIt)
{
	assert(ColumnNumber == 1);
	outValue = (*fValues)[ElemNumber-1];
	*outDisposeIt = false;
	return VE_OK;
}

#endif


//=================================================================================



VError QueryOptions::ToServer(VStream* into, BaseTaskInfo* context)
{
	VError err = into->PutLong8(fLimit);

	if (err == VE_OK)
	{
		if (fFilter == nil)
			err = into->PutLong(sel_nosel);
		else
		{
			err = fFilter->ToServer(into, context == nil ? nil : context->GetEncapsuleur());
		}
	}

	if (err == VE_OK)
		err = into->PutByte(fWantsFirstRecord);
	if (err == VE_OK)
		err = into->PutByte(fWantsLockedSet);
	if (err == VE_OK)
		err = into->PutByte(fWantToDescribe);
	if (err == VE_OK)
		err = into->PutWord(fHowToLock);
	if (err == VE_OK)
		err = into->PutWord(fRecordHowToLock);
	if (err == VE_OK)
		err = into->PutWord(fDestination);
	return err;
}


VError QueryOptions::FromClient(VStream* from, Base4D* inBase, Table* inTable, BaseTaskInfo* context)
{
	VError err = from->GetLong8(fLimit);

	if (err == VE_OK)
	{
		fFilter = inBase->BuildSelectionFromClient(from, context == nil ? nil : context->GetEncapsuleur(), inTable, err);
	}

	if (err == VE_OK)
		err = from->GetByte(fWantsFirstRecord);
	if (err == VE_OK)
		err = from->GetByte(fWantsLockedSet);
	if (err == VE_OK)
		err = from->GetByte(fWantToDescribe);
	if (err == VE_OK)
		fHowToLock = (DB4D_Way_of_Locking)from->GetWord();
	if (err == VE_OK)
		fRecordHowToLock = (DB4D_Way_of_Locking)from->GetWord();
	if (err == VE_OK)
		err = from->GetWord(fDestination);
	return err;
}


									// -----------------------------------


VError QueryResult::ToClient(VStream* into, BaseTaskInfo* context)
{
	VError err = VE_OK;

	if (fSel == nil)
		err = into->PutLong(sel_nosel);
	else
		err = fSel->ToClient(into, context == nil ? nil : context->GetEncapsuleur());

	if (fFirstRec == nil)
	{
		err = into->PutByte('.');
	}
	else
	{
		err = into->PutByte('+');
		if (err == VE_OK)
			err = fFirstRec->ToClient(into, context);
	}

	if (fSet == nil)
		err = into->PutLong(sel_nosel);
	else
	{
		err = into->PutLong(sel_bitsel);
		err = fSet->ToClient(into, context == nil ? nil : context->GetEncapsuleur());
		if (err == VE_OK)
		{
			fSet->Release();
			fSet = nil;
		}
	}

	if (fLockedSet == nil)
		err = into->PutLong(sel_nosel);
	else
	{
		err = into->PutLong(sel_bitsel);
		err = fLockedSet->ToClient(into, context == nil ? nil : context->GetEncapsuleur());
		if (err == VE_OK)
		{
			fLockedSet->Release();
			fLockedSet = nil;
		}
	}

	if (err == VE_OK)
		err = into->PutLong8(fCount);

	if (err == VE_OK)
		err = fQueryDescription.WriteToStream(into);
	if (err == VE_OK)
		err = fQueryExecutionXML.WriteToStream(into);
	if (err == VE_OK)
		err = fQueryExecution.WriteToStream(into);

	return err;
}



VError QueryResult::FromServer(VStream* from, Base4D* inBase, Table* inTable, BaseTaskInfo* context)
{
	VError err = VE_OK;

	fSel = inBase->BuildSelectionFromServer(from, context == nil ? nil : context->GetEncapsuleur(), inTable, err);

	uBYTE c;
	if (err == VE_OK)
		err = from->GetByte(c);

	if (err == VE_OK)
	{
		assert( c == '+' || c == '.' );
		if (c == '+')
		{
			fFirstRec = inBase->BuildRecordFromServer(from, context, inTable, err);
		}
	}

	fSet = inBase->BuildSetFromServer(from, context == nil ? nil : context->GetEncapsuleur(), err);
	fLockedSet = inBase->BuildSetFromServer(from, context == nil ? nil : context->GetEncapsuleur(), err);

	if (err == VE_OK)
		err = from->GetLong8(fCount);

	if (err == VE_OK)
		err = fQueryDescription.ReadFromStream(from);
	if (err == VE_OK)
		err = fQueryExecutionXML.ReadFromStream(from);
	if (err == VE_OK)
		err = fQueryExecution.ReadFromStream(from);

	return err;
}



//=================================================================================






