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

#if 0

typedef struct sql_token_def
{
	const char *name;
	long		value;	
} sql_token_def;


static sql_token_def	sql_keyword_def_array[] =
{	
	"SELECT",	sql_select,
	"FROM",	sql_from,
	"WHERE",	sql_where,
	0,				0
};


static sql_token_def	sql_oper_def_array[] =
{	
	"=",	sql_oper_egal,
	"!=",	sql_oper_diff,
	"<>",	sql_oper_diff,
	"^=",	sql_oper_diff,
	"¬=",	sql_oper_diff,
	">",	sql_oper_sup,
	">=",	sql_oper_supegal,
	"<",	sql_oper_inf,
	"<=",	sql_oper_infegal,
	"+",	sql_oper_plus,
	"-",	sql_oper_moins,
	"*",	sql_oper_multiplie,
	"/",	sql_oper_divise,
	"||",	sql_oper_concat,
	"AND",	sql_oper_and,
	"OR",	sql_oper_or,
	"LIKE",	sql_oper_like,
	"IN",	sql_oper_in,
	"NOT",	sql_oper_not,
	"ANY",	sql_oper_any,
	"SOME",	sql_oper_any,
	"ALL",	sql_oper_all,
	"EXISTS",	sql_oper_exists,
	"BETWEEN",	sql_oper_between,
	"NULL",	sql_oper_null,
	0,				0
};


static sql_token_def	sql_other_def_array[] =
{	
	"(",	sql_leftParent,
	")",	sql_rightParent,
	",",	sql_virgule,
	"*",	sql_etoile,
	"AND",	sql_etoile,
	"OR",	sql_etoile,
	"*",	sql_etoile,
	0,				0
};

									/*  ********************************************** */


SqlTokenArray SqlKeywords;
SqlTokenArray SqlOthers;
SqlTokenArray SqlOperators;

SqlTokensTree SqlTokens;


VError BuildSqlTokenList(sql_token_def *p, sLONG SqlTokenType, SqlTokenArray *arr)
{
	SqlTokenSimple *sq;
	VError err = VE_OK;
	
	while ((p->name != nil) && (err == VE_OK))
	{
		sq = new SqlTokenSimple(SqlTokenType, p->value);
		if (sq == nil) err = memfull;
		else
		{
			sq->SetName((uCHAR*)p->name);
			if (!arr->Add(sq)) err = memfull;
			if (err == VE_OK)
			{
				SqlTokens.Put(sq);
#if VERSIONDEBUG
				SqlTokens.checktree();
#endif				
			}
			++p;
		}
	}

	return err;
}


									/*  ********************************************** */
									

SqlTokenExpr::~SqlTokenExpr()
{
	if (exprResult != nil) delete exprResult;
}


									/*  ********************************************** */


sLONG SqlTokenSimple::Compare(const SqlTokenSimple *other, Boolean strict) const
{
//					  	==> return -1 si *this < other
// 						==> return 0  si *this = other
// 						==> return 1  si *this > other
	return(TokenName.CompareTo(other->TokenName));
}


									/*  ********************************************** */

SqlTableContext::SqlTableContext(Base4D *bd, Table* table)
{
		SourceBase = bd;
		SourceFile = table;
		CurrentRecord = nil;
}


ValPtr SqlTableContext::GetFieldValue(Field* cri)
{
	ValPtr result = nil;
	
	if ((CurrentRecord != nil) && (cri != nil))
	{
		result = CurrentRecord->GetFieldValue(cri);
	}
	
	return result;
}



// L.E. 20/12/00 added pour pouvoir linker
ValPtr SqlContext::GetFieldValue(Field* cri)
{
	return nil;
}


VError SqlContext::PerformSeq(SqlNode* root, Bittab& sel)
{
	VError err = VE_OK;
	sLONG currec, tot;
	DataFile* df;
	
	if (fTarget != nil)
	{
		df = fTarget->GetDF();
		
		if (sel.IsEmpty())
		{
			tot = df->GetMaxRecords(nil);
			err = sel.aggrandit(tot);
			for (currec = 0; currec<tot && err == VE_OK; currec++)
			{
				fCurrec = df->LoadRecord(currec, err, true);
				if (fCurrec != nil)
				{
					if (root->ComputeBoolValue(*this))
					{
						sel.Set(currec);
					}
					fCurrec->Release();
				}
			}
		}
		else
		{
			currec = 0;
			//tot = sel.Compte();
			while ((currec != -1) && err == VE_OK)
			{
				currec = sel.FindNextBit(currec);
				if (currec >= 0)
				{
					fCurrec = df->LoadRecord(currec, err, true);
					if (fCurrec != nil)
					{
						if (!root->ComputeBoolValue(*this))
						{
							sel.Clear(currec);
						}
						fCurrec->Release();
					}
					currec++;
				}
			}
		}
		
	}
	
	return err;
}



									/*  ********************************************** */
								
									
void SqlColumnField::GetValueAsText(VString& into)
{
	ValPtr cv;
	
	cv = context->GetFieldValue(SourceField);
	if (cv == nil)
	{
		into.Clear();
	}
	else
	{
		cv->GetString(into);
	}
}




SqlColumnExpr::~SqlColumnExpr()
{
	if (Expr != nil) delete Expr;
}

void SqlColumnExpr::GetValueAsText(VString& into)
{
	/* a faire */
	// penser au context
}

									/*  ********************************************** */

Boolean SqlNode::IsAllSeq()
{	
	return true;
}


Boolean SqlNode::ComputeBoolValue(SqlContext &context)
{
	return false;
}


void SqlNode::DisposeTree(void)
{
	if (left != nil) left->DisposeTree();
	delete left;
	left = nil;
	if (right != nil) right->DisposeTree();
	delete right;
	right = nil;
}


/*
void SqlNode::DelFromTree(void)
{
	SqlNode *pere, *grandpere;
	
	pere = parent;
	if (testAssert(pere != nil))
	{
		grandpere = pere->parent;
		if (grandpere == nil)
		{
		}
		else
		{
			if (grandpere->left == pere)
			{
				if (pere->left == this)
				{
					grandpere->SetLeft(pere->right);
				}
				else
				{
					grandpere->SetLeft(pere->left);
				}
			}
			else
			{
				if (pere->left == this)
				{
					grandpere->SetRight(pere->right);
				}
				else
				{
					grandpere->SetRight(pere->left);
				}
			}
			delete pere;
		}
	}
}
*/


void SqlNode::DisplayTreeForDebug(sLONG level)
{
	if (left != nil) left->DisplayTreeForDebug(level+1);
	DisplayForDebug(level);
	if (right != nil) right->DisplayTreeForDebug(level+1);
}


VError SqlNode::Perform(Bittab &sel, SqlContext &context)
{
	VError err = VE_OK;
	
	return err;
}


									/* ------------------- */

void SqlNodeOper::DisplayForDebug(sLONG level)
{
	VStr<64> s;
	sLONG i;
	
	for (i = 0; i < level; i++) s.AppendUniChar(9);
	s.AppendUniCString(L"Operator :");
	switch (subkind)
	{
		case sql_oper_and:
			s.AppendUniCString(L" And ");
			break;
			
		case sql_oper_or:
			s.AppendUniCString(L" Or ");
			break;
	}
	
	s.AppendUniCString(L"\n");
	DebugMsg(s);
}


Boolean SqlNodeOper::IsAllSeq()
{
	Boolean Result = true;
	Boolean Resultleft = true;
	Boolean Resultright = true;
	
	if (left != nil)
	{
		Resultleft = left->IsAllSeq();
	}

	if (right != nil)
	{
		Resultright = right->IsAllSeq();
	}

	switch (subkind)
	{
		case sql_oper_and:
			Result = Resultleft && Resultright;
			break;
			
		case sql_oper_or:
			Result = Resultleft || Resultright;
			break;
			
		default:
			break;
		
	}
	
	return Result;
}


Boolean SqlNodeOper::ComputeBoolValue(SqlContext &context)
{
	Boolean Result = true;
	Boolean Resultleft = true;
	Boolean Resultright = true;

	if (left != nil)
	{
		Resultleft = left->ComputeBoolValue(context);
	}

	if (right != nil)
	{
		Resultright = right->ComputeBoolValue(context);
	}

	switch (subkind)
	{
		case sql_oper_and:
			Result = Resultleft && Resultright;
			break;
			
		case sql_oper_or:
			Result = Resultleft || Resultright;
			break;
			
		default:
			break;
		
	}
	return Result;
}


VError SqlNodeOper::Perform(Bittab &sel, SqlContext &context)
{
	VError err = VE_OK;
	
	if (IsAllSeq())
	{
		err = context.PerformSeq(this, sel);
	}
	else
	{
		if ((left != nil) && !left->IsAllSeq())
		{
			left->Perform(sel, context);
			if ((right != nil) && right->IsAllSeq())
			{
				err = context.PerformSeq(right, sel);
			}
			else
			{
				if (right != nil)
				{
					Bittab sel2;
					right->Perform(sel2, context);
					
					switch (subkind)
					{
						case sql_oper_and:
							sel.And(&sel2);
							break;
							
						case sql_oper_or:
							sel.Or(&sel2);
							break;
							
						default:
							break;
						
					}
				}
			}
		}
		else
		{
			if (right != nil)
			{
				right->Perform(sel, context);
				if ((left != nil) && left->IsAllSeq())
				{
					err = context.PerformSeq(left, sel);
				}
				
			}
		}
	}
	
	return err;
}



									/* ------------------- */
									
									
Boolean SqlNodeComp::ComputeBoolValue(SqlContext &context)
{
	Boolean res = false;
	ValPtr cv;
	FicheInMem* fic = context.GetCurrec();
	if (fic != nil)
	{
		cv = fic->GetFieldValue(cri);
		if (cv != nil)
		{
			switch (oper)
			{
				case sql_oper_egal:
						res = cv->CompareToSameKind(v1) == CR_EQUAL;
					break;
					
				case sql_oper_diff:
						res = cv->CompareToSameKind(v1) != CR_EQUAL;
					break;
					
				case sql_oper_sup:
						res = cv->CompareToSameKind(v1) == CR_BIGGER;
					break;
					
				case sql_oper_supegal:
						res = cv->CompareToSameKind(v1) != CR_SMALLER;
					break;
					
				case sql_oper_inf:
						res = cv->CompareToSameKind(v1) == CR_SMALLER;
					break;
					
				case sql_oper_infegal:
						res = cv->CompareToSameKind(v1) != CR_BIGGER;
					break;
					
				case sql_oper_like:
						res = cv->CompareToSameKind(v1, true) == CR_EQUAL;
					
				case sql_oper_between:
						res = cv->CompareToSameKind(v1) != CR_SMALLER && cv->CompareToSameKind(v2) != CR_BIGGER;
					break;
					
				case sql_oper_outside:
						res = cv->CompareToSameKind(v1) == CR_SMALLER || cv->CompareToSameKind(v2) == CR_BIGGER;
					break;
								
					
				default:
					res = false;
					break;
			}
		}
	}
	
	return res;
}


void SqlNodeComp::SetComp(Field *xcri, sLONG xoper, ValPtr xv)
{
	cri = xcri;
	oper = xoper;
	strict1 = strict2 = false;
	v1 = xv;
	v2 = nil;
}


void SqlNodeComp::SetFourchette(Field *xcri, ValPtr xv1, ValPtr xv2, sLONG xoper, Boolean xstrict1, Boolean xstrict2 )
{
	cri = xcri;
	oper = xoper;
	strict1 = xstrict1;
	strict2 = xstrict2;
	v1 = xv1;
	v2 = xv2;
}


void SqlNodeComp::DisplayForDebug(sLONG level)
{
	VStr<255> s;
	VStr<64> s2;
	sLONG i;
	
	for (i = 0; i < level; i++) s.AppendUniChar(9);
	if (cri != nil)
	{
		cri->GetName(s2);
		s.AppendString(s2);
	}
	
	switch (oper)
	{
		case sql_oper_egal:
			s.AppendUniCString(L" = ");
			break;

		case sql_oper_diff:
			s.AppendUniCString(L" != ");
			break;

		case sql_oper_sup:
			s.AppendUniCString(L" > ");
			break;

		case sql_oper_supegal:
			s.AppendUniCString(L" >= ");
			break;

		case sql_oper_inf:
			s.AppendUniCString(L" < ");
			break;

		case sql_oper_infegal:
			s.AppendUniCString(L" <= ");
			break;

		case sql_oper_in:
			s.AppendUniCString(L" in ");
			break;

		case sql_oper_between:
			s.AppendUniCString(L" Between ");
			break;
	}
	
	if (v1 != nil)
	{
		v1->GetString(s2);
		s.AppendString(s2);
	}
	
	if (v2 != nil)
	{
		v2->GetString(s2);
		s.AppendUniCString(L" , ");
		s.AppendString(s2);
	}
	
	s.AppendUniCString(L"\n");
	DebugMsg(s);
}


VError SqlNodeComp::Perform(Bittab &sel, SqlContext &context)
{
	VError err = VE_OK;
	
	err = context.PerformSeq(this, sel);
	
	return err;
}



									
									/* ------------------- */

SqlNodeIndex::~SqlNodeIndex()
{
	if (v1 != nil) delete v1;
	if (v2 != nil) delete v2;
	if (fSubstitue != nil)
	{
		fSubstitue->DisposeTree();
		delete fSubstitue;
		fSubstitue = nil;
	}

	if (ind != nil)
		ind->Release();
}


Boolean SqlNodeIndex::IsAllSeq()
{
	return false;
}


Boolean SqlNodeIndex::ComputeBoolValue(SqlContext &context)
{
	Boolean res = false;
#if checkoldcompileindex

	if (fSubstitue == nil && ind != nil)
	{
		if (ind->GetTyp() == simplefield)
		{
			IndexInfoFromField* xind = (IndexInfoFromField*)ind;
			fSubstitue = new SqlNodeComp(xind->GetField(), oper, ((IndexValueFromField*)v1)->GetVal(), v2 == nil ? nil : ((IndexValueFromField*)v2)->GetVal(), strict1, strict2);
			
		}
		else
		{
			if (ind->GetTyp() == multiplefield)
			{
				IndexValueFromMultipleField *xv1 = (IndexValueFromMultipleField*)v1;
				IndexValueFromMultipleField *xv2 = (IndexValueFromMultipleField*)v2;
				
				IndexInfoFromMultipleField* xind = (IndexInfoFromMultipleField*)ind;
				FieldRefArray *frefs = xind->GetFields();
				assert(frefs != nil);
				sLONG i, nb = frefs->GetCount();
				fSubstitue = new SqlNodeComp((*frefs)[0].crit, sql_oper_egal, xv1->GetVal(0), v2 == nil ? nil : xv2->GetVal(0), true, true);
				
				for (i = 1; i<nb; i++)
				{
					SqlNodeOper *op = new SqlNodeOper(sql_oper_and);
					op->SetLeft(fSubstitue);
					if (i == (nb-1))
					{
						fSubstitue = new SqlNodeComp((*frefs)[i].crit, oper, xv1->GetVal(i), v2 == nil ? nil : xv2->GetVal(i), strict1, strict2);
					}
					else
					{
						fSubstitue = new SqlNodeComp((*frefs)[i].crit, sql_oper_egal, xv1->GetVal(i), v2 == nil ? nil : xv2->GetVal(i), true, true);
					}
					op->SetRight(fSubstitue);
					fSubstitue = op;
				}
			}
			
		}
	}
	
	if (fSubstitue != nil)
	{
		res = fSubstitue->ComputeBoolValue(context);
	}
#endif
	return res;
}


void SqlNodeIndex::DisplayForDebug(sLONG level)
{
	VStr<255> s;
	VStr<255> s2;
	sLONG i;
	
	for (i = 0; i < level; i++) s.AppendUniChar(9);
	s.AppendUniCString(L"Use Index : ");
	
	if (ind != nil)
	{
		ind->GetDebugString(s2);
		s.AppendString(s2);
	}
	
	switch (oper)
	{
		case sql_oper_egal:
			s.AppendUniCString(L" = ");
			break;

		case sql_oper_diff:
			s.AppendUniCString(L" != ");
			break;

		case sql_oper_sup:
			s.AppendUniCString(L" > ");
			break;

		case sql_oper_supegal:
			s.AppendUniCString(L" >= ");
			break;

		case sql_oper_inf:
			s.AppendUniCString(L" < ");
			break;

		case sql_oper_infegal:
			s.AppendUniCString(L" <= ");
			break;

		case sql_oper_in:
			s.AppendUniCString(L" in ");
			break;

		case sql_oper_between:
			s.AppendUniCString(L" Between ");
			break;
	}
	
	if (v1 != nil)
	{
		v1->GetDebugString(s2);
		s.AppendString(s2);
	}
	
	if (v2 != nil)
	{
		v2->GetDebugString(s2);
		s.AppendUniCString(L" ; ");
		s.AppendString(s2);
	}
	
	s.AppendUniCString(L"\n");
	DebugMsg(s);
}


VError SqlNodeIndex::Perform(Bittab &sel, SqlContext &context)
{
	VError err = VE_OK;
	IndexValue *xiv1, *xiv2;
	
	xiv1 = v1;
	xiv2 = v2;
	switch (oper)
	{
		case sql_oper_egal:
		case sql_oper_diff:
			xiv2 = xiv1;
			break;
	}

	if (oper == sql_oper_diff || oper == sql_oper_outside)
	{
		Bittab* temp = ind->Fourche(xiv1,strict1,xiv2,strict2,false, &sel);
		if (temp != nil)
		{
			temp->Invert();
			sel.And(temp);
			delete temp;
		}
	}
	else
	{
		ind->Fourche(xiv1,strict1,xiv2,strict2,false, &sel);
	}
	
	return err;
}




									
									/* ------------------- */
									

void SqlNodeJoint::SetJoint(Field *xcri1, sLONG xoper, Field *xcri2 )
{
	cri1 = xcri1;
	oper = xoper;
	cri2 = xcri2;
	
}
			
			
void SqlNodeJoint::DisplayForDebug(sLONG level)
{
	VStr<255> s;
	VStr<64> s2;
	sLONG i;
	
	for (i = 0; i < level; i++) s.AppendUniChar(9);
	
	s.AppendUniCString(L"Jointure :");
	if (cri1 != nil)
	{
		cri1->GetName(s2);
		s.AppendString(s2);
	}
	
	switch (oper)
	{
		case sql_oper_egal:
			s.AppendUniCString(L" = ");
			break;

		case sql_oper_diff:
			s.AppendUniCString(L" != ");
			break;

		case sql_oper_sup:
			s.AppendUniCString(L" > ");
			break;

		case sql_oper_supegal:
			s.AppendUniCString(L" >= ");
			break;

		case sql_oper_inf:
			s.AppendUniCString(L" < ");
			break;

		case sql_oper_infegal:
			s.AppendUniCString(L" <= ");
			break;

		case sql_oper_in:
			s.AppendUniCString(L" in ");
			break;

		case sql_oper_between:
			s.AppendUniCString(L" Between ");
			break;
	}
	
	if (cri2 != nil)
	{
		cri2->GetName(s2);
		s.AppendString(s2);
	}
	
	s.AppendUniCString(L"\n");
	DebugMsg(s);
}
						

									/*  ********************************************** */



SqlQuery::SqlQuery(Base4D* bd) 
{
	TargetBase = bd;
	curtoken = 0;
	context = new SqlContext(bd);
	NodeRoot = nil;
}


SqlQuery::~SqlQuery()
{
	sLONG i, nb = Tokens.GetCount();
	SqlToken *sq;
	
	for (i = 1; i <= nb; i++)
	{
		sq = Tokens[i];
		if (sq != nil) delete sq;
	}
	
	if (NodeRoot != nil)
	{ 
		NodeRoot->DisposeTree();
		delete NodeRoot;
	}
	if (context != nil) delete context;
}


VError SqlQuery::AddToken(SqlToken *tok)
{
	VError err = VE_OK;
	if (!Tokens.Add(tok)) err = memfull;

	return err;
}


Field* SqlQuery::FindAndRetainField(VString& fieldname)
{
	Field* result = nil;
	
	sLONG i,nb = FromTable.GetCount();
	
	for (i=1; i<=nb; i++)
	{
		result = (FromTable[i])->FindAndRetainFieldRef(fieldname);
		if (result != nil) break;
	}
	
	return result;
}


VError SqlQuery::ResolveUnResolvedRef(void)
{
	sLONG i, nb = Tokens.GetCount();
	VError err = VE_OK;
	SqlToken *sq;
	VString* vs;
	SqlTokenField *FieldTok;
	Field *cri;
	
	for (i = 1; i <= nb; i++)
	{
		sq = Tokens[i];
		if (sq->GetTokenType() == sql_unresolved)
		{
			vs = ((SqlTokenUnResolved*)sq)->GetRef();
			cri = FindAndRetainField(*vs);
			if (cri != nil)
			{
				FieldTok = new SqlTokenField(cri);
				Tokens[i] = FieldTok;
				delete sq;
				cri->Release();
			}
			else
			{
				err = VE_DB4D_SQLTOKENNOTFOUND;
			}
		}
	}
	
	return err;
}


VError SqlQuery::BuildFromText(VString *source)
{
	UniChar c;
	sLONG len,curchar,errfound;
	VError err = VE_OK;
	VStr<512> curword;
	VStr<128> s1,s2;
	SqlTokenSimple *SimpleTok = nil, *orig;
	SqlTokenTable *TableTok;
	SqlTokenField *FieldTok;
	SqlTokenExpr *ExprTok;
	SqlTokenUnResolved *UnresTok;
	Table *f;
	Field *cri;
	sLONG p;
	UniChar ch;
	VReal *r;
	VString *txt;
	Boolean waitingFROM = false, waitingWHERE = false;

	StAllocateInCache alloc;

	len = source->GetLength();
	curchar = 0;
	while ( (curchar<=len) && (err == VE_OK))
	{
		if (curchar == len) 
			c = 32;
		else
			c = source->GetChar(curchar+1);
		if (c == 32 || c == CHAR_COMMA)
		{
			if (curword.GetLength() != 0)
			{
				// nouveau token pas vide
				if (SimpleTok == nil) SimpleTok = new SqlTokenSimple(0,0);
				SimpleTok->SetName(curword);
				orig = SimpleTok;
				errfound  = SqlTokens.Get(SimpleTok);
				if (errfound == VE_OK)
				{
					// on a trouve un token SQL non variant 
					*orig = (*SimpleTok);
					SimpleTok = orig;
					
					err = AddToken(SimpleTok);
					if (SimpleTok->GetTokenType() == sql_keyword)
					{
						switch (SimpleTok->GetTokenSubType())
						{
							case sql_select:
								waitingFROM = true;
								break;

							case sql_from:
								waitingFROM = false;
								waitingWHERE = true;
								break;

							case sql_where:
								waitingFROM = false;
								waitingWHERE = false;
								err = ResolveUnResolvedRef();
								break;
						}
					}
					SimpleTok = nil;
				}
				else
				{
					// essayons une ref 4D
					f = TargetBase->FindAndRetainTableRef(curword);
					if (f != nil)
					{
						TableTok = new SqlTokenTable(f);
						err = AddToken(TableTok);
						if (waitingWHERE)
						{
							if (!FromTable.Add(f)) err = memfull;
						}
						f->Release();
					}
					else
					{
						ch = (curword[0]);
						if ( ((ch >= CHAR_DIGIT_ZERO) && (ch <= CHAR_DIGIT_NINE)) || (ch == CHAR_FULL_STOP) || (ch == CHAR_HYPHEN_MINUS) )
						{
							// c'est un nombre
							r = new VReal(curword.GetReal());	// L.E. 22/12 GetReal
							ExprTok = new SqlTokenExpr(r);
							err = AddToken(ExprTok);
						}
						else
						{
							if ( (ch == CHAR_QUOTATION_MARK) || ( ch == CHAR_APOSTROPHE ) )
							{
								// c'est une chaine de char
								curword.Remove(curword.GetLength(), 1);
								curword.Remove(1, 1);
								txt = new VString( curword);
								ExprTok = new SqlTokenExpr(txt);
								err = AddToken(ExprTok);
							}
							else
							{
								p = curword.FindUniChar(CHAR_FULL_STOP);
								if (p>0)
								{
									curword.GetSubString(1,p-1,s1);
									curword.GetSubString(p+1,curword.GetLength()-p,s2);
									cri = TargetBase->FindAndRetainFieldRef(s1,s2);
									if (cri != nil)
									{
										FieldTok = new SqlTokenField(cri);
										err = AddToken(FieldTok);
										cri->Release();
									}
									else
									{
										err = VE_DB4D_SQLTOKENNOTFOUND;
									}
								}
								else
								{
									if (waitingFROM)
									{
										UnresTok = new SqlTokenUnResolved();
										UnresTok->SetRef(curword);
										err = AddToken(UnresTok);
									}
									else
									{
										cri = FindAndRetainField(curword);
										if (cri != nil)
										{
											FieldTok = new SqlTokenField(cri);
											err = AddToken(FieldTok);
											cri->Release();
										}
										else
										{
											err = VE_DB4D_SQLTOKENNOTFOUND;
										}
									}
								}
							}
						}
					}
				}
				
				curword.Clear();
			}
		}
		else
		{
			curword.AppendUniChar(c);
		}
		++curchar;
	}
	
	if (SimpleTok != nil) delete SimpleTok;
	
	return (err);
}



SqlToken* SqlQuery::NextToken(void)
{
	SqlToken* result = nil;
	
	if (curtoken<Tokens.GetCount())
	{
		++curtoken;
		result = Tokens[curtoken];
	}
	
	return result;
}


void SqlQuery::StepBackOneToken(void)
{
	if (curtoken>0)
	{
		--curtoken;
	}
}



VError SqlQuery::GetSubexpress(SqlSubExpress& subexpr, SqlToken** lasttok)
{
	VError err = VE_OK;
	SqlToken *sqltok;
	Boolean stop = false;
	sLONG level = 0;
	
	// subexpr.oReinitArray();
	sqltok = NextToken();
	*lasttok = sqltok;
	while ( (sqltok != nil) && (err == VE_OK) && (!stop))
	{
		switch (sqltok->GetTokenType())
		{
			case sql_unresolved:
				err = VE_DB4D_SQLTOKENNOTFOUND;
				break;
				
			case sql_other:
				switch(((SqlTokenSimple*)sqltok)->GetTokenSubType())
				{
					case sql_leftParent:
						++level;
						break;
						
					case sql_rightParent:
						--level;
						if (level<0)
						{
							stop = true;
							StepBackOneToken();
						}
						break;
						
					case sql_virgule:
						if (level == 0)
						{
							stop = true;
							// StepBackOneToken();
						}
						break;
				}
				break;
				
				case sql_keyword:
					if (level == 0)
					{
						stop = true;
						StepBackOneToken();
					}
					break;				
		}
		
		if (!stop)
		{
			if (!subexpr.Add(sqltok)) err = memfull;
			sqltok = NextToken();
		}
		*lasttok = sqltok;
	}

	return err;
	
}


VError SqlQuery::GetColumns(void)
{
	VError err = 0;
	SqlToken *sqltok,*sqltok2;
	Boolean stop = false;
	SqlSubExpress *subexpress;
	SqlColumnExpr *colexp;
	SqlColumnField *colfield;
	SqlColumn *col;
	Field* cri;
	
	
	do
	{
		subexpress = new SqlSubExpress;
		err = GetSubexpress(*subexpress, &sqltok);
		if (err == VE_OK)
		{
			if (subexpress->GetCount()>0)
			{
				colfield = nil;
				
				if (subexpress->GetCount() == 1)
				{
					sqltok2 = (*subexpress)[1];
					if (sqltok2->GetTokenType() == sql_field)
					{
						cri = ((SqlTokenField*)sqltok2)->GetField();
					}
					colfield = new SqlColumnField(cri, context);
					col = colfield;
				}
				
				if (colfield == nil)
				{
					colexp = new SqlColumnExpr(subexpress, context);
					if (colexp != nil) subexpress = nil;
					col = colexp;
				}
				
				if (col == nil) err = memfull;
				else
				{
					if (!Columns.Add(col)) err = memfull;
				}
			}
			
			if (sqltok == nil)
			{
				stop = true;
			}
			else
			{
				if ( (sqltok->GetTokenType() == sql_other) && (((SqlTokenSimple*)sqltok)->GetTokenSubType() == sql_virgule) )
					stop = false;
				else 
					stop = true;
			}
		}
		
		if (subexpress!=nil) delete subexpress;
	} while ((err == VE_OK) && !stop);

	return err;
}



VError SqlQuery::SkipTables(void)
{
	VError err = VE_OK;
	SqlToken *sqltok;
	Boolean stop = false;
	SqlSubExpress subexpress;

	do
	{
		err = GetSubexpress(subexpress, &sqltok);
		if ( (sqltok->GetTokenType() == sql_other) && (((SqlTokenSimple*)sqltok)->GetTokenSubType() == sql_virgule) )
			stop = false;
		else 
			stop = true;
	} while ((err == VE_OK) && !stop);
	
	return err;
}


SqlNode* SqlQuery::BuildComp(Field *xcri1, Field *xcri2, ValPtr xv1, ValPtr xv2, sLONG xoper, VError *error)
{
	SqlNode* rn;
	SqlNodeJoint* joint;
	SqlNodeComp* comp;
	VError err = VE_OK;
	
	rn = nil;
	if ((xoper != 0) && (xcri1 != nil))
	{
		if (xcri2 != nil)
		{
			joint = new SqlNodeJoint;
			if (joint == nil) err = memfull;
			else
			{
				joint->SetJoint(xcri1, xoper, xcri2);
				rn = joint;
			}
		}
		else
		{
			if (xv1 == nil)
			{
				err = VE_DB4D_SQLSYNTAXERROR;
			}
			else
			{
				comp = new SqlNodeComp;
				rn = comp;
				if (comp == nil) err = memfull;
				else
				{
					if (xv2 == nil)
					{
						comp->SetComp(xcri1, xoper, xv1);
					}
					else
					{
						comp->SetFourchette(xcri1, xv1, xv2);
					}
				}
			}
		}
	}
	
	*error = err;
	return rn;
}


VError SqlQuery::CheckForIndex(SqlNode* &root, SqlNodeArray *Nodes, sLONG startNode, sLONG curlen, sLONG maxlen, FieldsArray *FieldsToCheck, sLONGArray *Indices)
{
	sLONG i, nbnodes, j;
	Field *cri;
	VError err = VE_OK;
	SqlNode *node;
	IndexInfo *ind;
	IndexValue *iv,*iv_2;
	IndexValueFromField *iv1, *iv2;
	IndexValueFromMultipleField *ivx;
	SqlNodeIndex *NodeInd;
	SqlNodeComp *NodeComp;
	
	nbnodes = Nodes->GetCount();
	
	for (i = startNode; i <= nbnodes; i++)
	{
		node = (*Nodes)[i];
		if (node  != nil && node->GetNodeType() == sqlnode_seqcomp)
		{
			NodeComp = (SqlNodeComp*)node;
			if ( (NodeComp->GetOper() ==  sql_oper_egal) || curlen == maxlen )
			{
				cri = NodeComp->GetField();
				(*FieldsToCheck)[curlen] = cri;
				(*Indices)[curlen] = i;
				if (curlen == maxlen)
				{
					ind = TargetBase->FindAndRetainIndex(FieldsToCheck,maxlen);
					if (ind != nil)
					{
						// on a trouver un index qui matche les champs
						// il faut creer la valueinfo
	
						sLONG xoper;
						Boolean xstrict1,xstrict2;
	
						iv = nil;
						iv_2 = nil;
						if (maxlen == 1)
						{
							NodeComp = (SqlNodeComp*)node;
							iv1 = new IndexValueFromField(NodeComp->GetValue1());
							if (iv1 == nil) err = memfull;
							else iv = iv1;
							xoper = NodeComp->GetOper();
							xstrict1 = NodeComp->GetStrict1();
							xstrict2 = NodeComp->GetStrict2();
							
							if (NodeComp->GetValue2() == nil)
								iv2 = nil;
							else
								iv2 = new IndexValueFromField(NodeComp->GetValue2());
							iv_2 = iv2;
						}
						else
						{
							ivx = new IndexValueFromMultipleField();
							if (ivx == nil) err = memfull;
							else
							{
								for (j = 1; j<= maxlen; j++)
								{
									err = ivx->AddValue(((SqlNodeComp*)((*Nodes)[(*Indices)[j]]))->GetValue1());
									if (err != 0) break;
								}
								if (err == VE_OK) iv = ivx;
							}
							if (err != 0) delete ivx;
							
							NodeComp = (SqlNodeComp*)((*Nodes)[(*Indices)[1]]);
							xoper = NodeComp->GetOper();
							xstrict1 = NodeComp->GetStrict1();
							xstrict2 = NodeComp->GetStrict2();
						}
						
						if (err == 0)
						{
							NodeInd = new SqlNodeIndex(ind, iv, iv_2, xoper, xstrict1, xstrict2 );
							NodeInd->SetParent(NodeComp->GetParent());
							NodeInd->SetRight(NodeComp->GetRight());
							NodeInd->SetLeft(NodeComp->GetLeft());
							
							SqlNode* parent = NodeComp->GetParent();
							if (NodeComp == root) root = NodeInd;
							if (parent != nil)
							{
								if (parent->GetLeft() == NodeComp)
									parent->SetLeft(NodeInd);
								if (parent->GetRight() == NodeComp)
									parent->SetRight(NodeInd);
							}
	
							delete NodeComp;
							(*Nodes)[(*Indices)[1]] = nil;
							for (j = 2; j<= maxlen; j++)
							{
								((SqlNodeComp*)((*Nodes)[(*Indices)[j]]))->MarkForDelete();
								(*Nodes)[(*Indices)[j]] = nil;
							}
	
						}
						else 
							ind->Release();
					}
				}
				else
				{
					err = CheckForIndex(root, Nodes, startNode+1, curlen+1, maxlen, FieldsToCheck, Indices);
				}
			}
		}
	}
	
	return err;
}


SqlNode* SqlQuery::xFactorizeIndex(SqlNode* node, VError *error, SqlNodeArray* deja)
{
	VError err = VE_OK;
	Boolean credeja = false;
	sLONG len,i,k,len2;
	SqlNodeComp *comp,*comp2;
	sLONG oper1, oper2, newoper;
	Boolean strict1,strict2;
	FieldsArray* x;
	sLONGArray *indices;
	StAllocateInCache alloc;
	
	if (node!=nil)
	{
		if (deja == nil)
		{
			deja = new SqlNodeArray;
			if (deja == nil) err = memfull;
			credeja = true;
		}
		
		if (err == VE_OK)
		{
			if (node->GetNodeType() == sqlnode_oper)
			{
				if ( ((SqlNodeOper*)node)->GetOper() == sql_oper_or )
				{
					xFactorizeIndex(node->GetLeft(), &err, nil);
					xFactorizeIndex(node->GetRight(), &err, nil);
				}
				else
				{
					xFactorizeIndex(node->GetLeft(), &err, deja);
					xFactorizeIndex(node->GetRight(), &err, deja);
				}
			}
			else
			{
				if (node->GetNodeType() == sqlnode_seqcomp)
				{
					comp = (SqlNodeComp*)node;
					if (comp->CouldUseIndex())
					{
						deja->Add(node);
					}
				}
			}
		}
			
		if (credeja && (deja != nil))
		{
			len = deja->GetCount();
			len2 = len;
			
			// d'abord on cherche les fourchettes
			
			for (i = 1; i<=len; i++)
			{
				comp = (SqlNodeComp*)((*deja)[i]);
				if (comp != nil && comp->CouldFork())
				{
					for (k = i+1; k<=len; k++)
					{
						comp2 = (SqlNodeComp*)((*deja)[k]);
						if (comp2 != nil)
						{
							if (comp2->CouldFork() && (comp2->GetField() == comp->GetField()) )
							{
								oper1 = comp->GetOper();
								oper2 = comp2->GetOper();
								if (  ( (oper2 == sql_oper_sup || oper2 == sql_oper_supegal) && (oper1 == sql_oper_inf || oper1 == sql_oper_infegal) )
										||
										  ( (oper1 == sql_oper_sup || oper1 == sql_oper_supegal) && (oper2 == sql_oper_inf || oper2 == sql_oper_infegal) )
									)
								{
									if (oper1 == sql_oper_sup || oper1 == sql_oper_supegal)
									{
										strict1 = oper1 == sql_oper_sup;
										strict2 = oper2 == sql_oper_inf;
										if (comp->GetValue1()->CompareTo(*(comp2->GetValue1())) == CR_SMALLER)
										{
											newoper = sql_oper_between;
										}
										else
										{
											newoper = sql_oper_outside;
										}
									}
									else
									{
										strict1 = oper1 == sql_oper_inf;
										strict2 = oper2 == sql_oper_sup;
										if (comp->GetValue1() < comp2->GetValue1())
										{
											newoper = sql_oper_outside;
										}
										else
										{
											newoper = sql_oper_between;
										}
									}
									
									comp->SetFourchette(comp->GetField(),comp->GetValue1(), comp2->GetValue1(), newoper, strict1, strict2);
									/*
									comp2->DelFromTree(&noderoot);
									delete comp2;
									*/
									comp2->MarkForDelete();
									(*deja)[k] = nil;
									len2--;
								}
							}
						}
					}
				}
			}
			
			node = CleanTree(node);
			
			x = new FieldsArray;
			indices = new sLONGArray;
			
			if (x != nil &&  indices != nil)
			{
				if (!x->AddNSpaces(len,false) || !indices->AddNSpaces(len,false)) err = memfull;
				else
				{
					for (k = len2; k>0 ; k--)
					{
						err = CheckForIndex(node, deja, 1, 1, k, x, indices);
					}
				}
			}
			else err = memfull;
			
			node = CleanTree(node);

			if (x != nil) delete x;
			if (indices != nil) delete indices;
			
			delete deja;
		}
	}
	*error = err;
	return node;
}


SqlNode* SqlQuery::CleanTree(SqlNode* node)
{
	Boolean DoisDelete = false;
	SqlNode *node2, *node3;
	
	node3 = node->GetLeft();
	if (node3 != nil)
	{
		node2 = CleanTree(node3);
		if (node2 == nil && node3 != nil) DoisDelete = true;
		node->SetLeft(node2);
	}
	
	node3 = node->GetRight();
	if (node3 != nil)
	{
		node2 = CleanTree(node3);
		if (node2 == nil && node3 != nil) DoisDelete = true;
		node->SetRight(node2);
	}
	
	if (node->IsToBeDeleted())
	{
		delete node;
		node = nil;
	}
	else
	{
		if (DoisDelete)
		{
			if (node->GetLeft() == nil) node2 = node->GetRight();
			else node2 = node->GetLeft();
			delete node;
			node = node2;
		}
	}
	
	return node;
}


VError SqlQuery::ParseWhere(void)
{
	VError err = VE_OK;
	
	NodeRoot = xParseWhere(&err);

	DebugMsg("\n");
	DebugMsg("\n");
	DebugMsg("*** Before Optimization ***\n");
	DebugMsg("\n");
	if (NodeRoot != nil)
	{
		NodeRoot->DisplayTreeForDebug(0);
	}
	
	DebugMsg("\n");
	DebugMsg("*****************\n");
	DebugMsg("*** After Optimization ***\n");
	DebugMsg("\n");
	NodeRoot = xFactorizeIndex(NodeRoot, &err, nil);
	
	if (NodeRoot != nil)
	{
		NodeRoot->DisplayTreeForDebug(0);
	}

	DebugMsg("\n");
	DebugMsg("\n");
	
	return err;
}


SqlNode* SqlQuery::xParseWhere(VError* error)
{
	VError err = VE_OK;
	SqlToken *sqltok;
	Boolean stop = false;
	// SqlSubExpress *subexpress = nil;
	SqlNode *result;
	SqlNode *rn;
	SqlNodeOper* oper = nil;
	Field* xcri1 = nil;
	Field* xcri2 = nil;
	sLONG xoper = 0, xoper2;
	ValPtr xv1 = nil;
	ValPtr xv2 = nil;
	SqlNode* dejaleft = nil;
	
	
	result = nil;
	// subexpress = new SqlSubExpress;
	do
	{
		sqltok = NextToken();
		if (sqltok != nil)
		{
			switch (sqltok->GetTokenType())
			{
				case sql_unresolved:
					err = VE_DB4D_SQLTOKENNOTFOUND;
					break;
					
				case sql_field:
					if (xcri1 == nil)
					{
						xcri1 = ((SqlTokenField*)sqltok)->GetField();
					}
					else
					{
						xcri2 = ((SqlTokenField*)sqltok)->GetField();
					}
					break;
					
				case sql_expr:
					if (xv1 == nil)
					{
						xv1 = ((SqlTokenExpr*)sqltok)->GetExprValue();
					}
					else
					{
						xv2 = ((SqlTokenExpr*)sqltok)->GetExprValue();
					}
					break;
					
				case sql_other:
					switch(((SqlTokenSimple*)sqltok)->GetTokenSubType())
					{
						case sql_leftParent:
							dejaleft = xParseWhere(&err);
							break;
							
						case sql_rightParent:
							stop = true;
							break;
							
						case sql_virgule:
							err = VE_DB4D_SQLSYNTAXERROR;
							break;
					}
					break;
						
				case sql_operator:
					xoper2 = ((SqlTokenSimple*)sqltok)->GetTokenSubType();
					if ( (xoper2 == sql_oper_and) || (xoper2 == sql_oper_or))
					{
						oper = new SqlNodeOper(xoper2);
						if (oper == nil) err = memfull;
						else
						{
							if (dejaleft == nil)
							{
								dejaleft = BuildComp(xcri1,xcri2,xv1,xv2,xoper,&err);
							}
								
							if (dejaleft == nil)
							{
								delete oper;
								err = VE_DB4D_SQLSYNTAXERROR;
							}
							else
							{
								oper->SetLeft(dejaleft);
								rn = xParseWhere(&err);
								if (err == VE_OK)
								{
									oper->SetRight(rn);
									dejaleft = oper;
								}
								else
								{
									delete oper;
									if (rn != nil) delete rn;
								}
							}
						}
						
						xoper = 0;
						xcri1 = nil;
						xcri2 = nil;
						xv1 = nil;
						xv2 = nil;
					}
					else
					{
						xoper = xoper2;
					}
					
					break;
			
				
				break;		
			}
			
		}
		else stop = true;
		
	} while ((err == VE_OK) && !stop);
	
	
	if (err == VE_OK && dejaleft == nil)
	{
		dejaleft = BuildComp(xcri1,xcri2,xv1,xv2,xoper,&err);
		
		if (dejaleft == nil)
		{
			err = VE_DB4D_SQLSYNTAXERROR;
		}
		
	}

	result = dejaleft;
	// if (subexpress != nil) delete subexpress;
	*error = err;
	return result;
}



VError SqlQuery::FetchData(Selection* &sel)
{
	VError err = VE_OK;
	Bittab *b;
	Boolean is_b_owned;
	DataFile *DF = nil;
	
	sel = nil;
	b = new Bittab;
	
	if (FromTable.GetCount()>0)
	{
		DF = FromTable[1]->GetDF();
	}
	
	if (DF != nil)
		context->SetTarget(DF->GetFichier());
		
	err = NodeRoot->Perform(*b, *context);
		
	if (DF != nil)
	{
		sel = CreFromBittabPtr(DF, b, &is_b_owned);
	}
	if (!is_b_owned)
		delete b;
	
	return err;
}


VError SqlQuery::ExecSql(Selection* &sel)
{
	SqlToken *sqltok;
	SqlTokenSimple *keyw;
	VError err = VE_OK;
	Boolean EndSubQuery = false;
	
	sel = nil;
	
	sqltok = NextToken();
	if (sqltok != nil)
	{
		if (sqltok->GetTokenType() == sql_keyword)
		{
			keyw = (SqlTokenSimple*)sqltok;
			if (keyw->GetTokenSubType() == sql_select)
			{
				err = GetColumns();
				sqltok = NextToken();
				while ( (sqltok != nil) && (err == VE_OK) && (!EndSubQuery))
				{
					if (sqltok->GetTokenType() == sql_keyword)
					{
						keyw = (SqlTokenSimple*)sqltok;
						switch(keyw->GetTokenSubType())
						{
							case sql_from:
								err = SkipTables();
								break;
								
							case sql_where:
								err = ParseWhere();
								break;
								
							default:
								err = VE_DB4D_SQLSYNTAXERROR;
								break;
						}
					}
					
					if (sqltok->GetTokenType() == sql_other)
					{
						keyw = (SqlTokenSimple*)sqltok;
						if (keyw->GetTokenSubType() == sql_rightParent)
						{
							EndSubQuery = true;
							StepBackOneToken();
						}
						else err = VE_DB4D_SQLSYNTAXERROR;
					}
					
					if (!EndSubQuery) sqltok = NextToken();
				}
				
			}
			else err = VE_DB4D_SQLSYNTAXERROR;
		}
		else err = VE_DB4D_SQLSYNTAXERROR;
	}
	
	if (err == VE_OK)
	{
		err = FetchData(sel);
	}
	
	return err;
}



									/*  ********************************************** */


									/*  ********************************************** */
									

VError InitSQL(void)
{
	VError err = VE_OK;
	
	err = BuildSqlTokenList(sql_keyword_def_array,sql_keyword, &SqlKeywords);
	if (err == VE_OK)
	{
		err = BuildSqlTokenList(sql_oper_def_array,sql_operator, &SqlOperators);
		if (err == VE_OK)
		{
			err = BuildSqlTokenList(sql_other_def_array,sql_other, &SqlOthers);
		}
	}
	
	return err;
}

#endif

