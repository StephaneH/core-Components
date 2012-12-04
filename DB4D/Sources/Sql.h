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
#ifndef __SQL__
#define __SQL__

#if 0
enum { aucuntokensql=0L, sql_keyword, sql_operator, sql_field, sql_table, sql_func, sql_other, sql_expr, sql_unresolved };

enum { aucunopersql=0L, sql_oper_egal, sql_oper_diff, sql_oper_sup, sql_oper_supegal, sql_oper_inf, sql_oper_infegal, 
			sql_oper_like, sql_oper_in, sql_oper_not, sql_oper_any, sql_oper_all,	sql_oper_exists,	sql_oper_between, sql_oper_outside,
			sql_oper_null, sql_oper_plus, sql_oper_moins, sql_oper_multiplie, sql_oper_divise, sql_oper_concat, 
			sql_oper_and, sql_oper_or };
			
enum { aucunkeywordsql=0L, sql_select, sql_from, sql_where };

enum { aucunothersql=0L, sql_leftParent, sql_rightParent, sql_virgule, sql_etoile };

			 
class SqlToken : public ObjInCacheMemory
{
	public:
		SqlToken(sLONG type) { TokenType = type; };
		sLONG GetTokenType(void) { return TokenType; };
		
	protected:
		sLONG TokenType;
};

class SqlTokenSimple : public SqlToken
{
	public:
		SqlTokenSimple(sLONG type = 0, sLONG subtype = 0):SqlToken(type) { TokenSubType = subtype; };
		void SetName(uBYTE* name) { TokenName.FromCString((const char*)name); };
		void SetName(VString& name) { TokenName = name; };
		sLONG Compare(const SqlTokenSimple *other, Boolean strict) const;
		sLONG GetTokenSubType(void) { return TokenSubType; };
		
	protected:
		sLONG TokenSubType;
		VStr<16> TokenName;
};


class SqlTokenField : public SqlToken
{
	public:
		SqlTokenField(Field* ref):SqlToken(sql_field) { FieldRef = ref; };
		Field* GetField(void) { return FieldRef; };
		
	protected:
		Field* FieldRef;
};


class SqlTokenTable : public SqlToken
{
	public:
		SqlTokenTable(Table* ref):SqlToken(sql_table) { FileRef = ref; };
		
	protected:
		Table* FileRef;
};


class SqlTokenUnResolved : public SqlToken
{
	public:
		SqlTokenUnResolved(void):SqlToken(sql_unresolved) { };
		void SetRef(uBYTE* ref) { UnresolvedRef.FromCString((const char*)ref); };
		void SetRef(VString& ref) { UnresolvedRef = ref; };
		VString* GetRef(void) { return(&UnresolvedRef); };
		
	protected:
		VStr<64> UnresolvedRef;
};


class SqlTokenExpr : public SqlToken
{
	public:
		SqlTokenExpr(ValPtr ref):SqlToken(sql_expr) { exprResult = ref; };
		virtual ~SqlTokenExpr();
		SqlTokenExpr(void):SqlToken(sql_expr) { exprResult = nil; };
		ValPtr GetExprValue(void) { if (exprResult == nil) return nil/*DefaultValue*/; else return exprResult; };
		ValPtr StealExprValue(void) { ValPtr res = exprResult; exprResult = nil; return res; };
		
	protected:
		ValPtr exprResult;
};


						/* ------------------------------------------------------------- */
						

class SqlTableContext : public ObjInCacheMemory
{
	public:
		SqlTableContext(Base4D *bd, Table* table);
		ValPtr GetFieldValue(Field* cri);
		
	protected:
		Base4D *SourceBase;
		Table *SourceFile;
		FicheInMem *CurrentRecord;
		
};

class SqlNode;
class SqlQuery;

class SqlContext : public ObjInCacheMemory
{
	public:
		SqlContext(Base4D *bd):fBaseContext(bd) { SourceBase = bd; fTarget = nil; fCurrec = nil; }
		ValPtr GetFieldValue(Field* cri);
		inline BaseTaskInfo* GetData() { return &fBaseContext; };
		inline Table* GetTarget() { return fTarget; };
		inline void SetTarget(Table* inTarget) { fTarget = inTarget; };
		inline FicheInMem* GetCurrec() { return fCurrec; };
		
		VError PerformSeq(SqlNode* root, Bittab& sel);
		
	protected:
		Base4D *SourceBase;
		BaseTaskInfo fBaseContext;
		Table* fTarget;
		FicheInMem* fCurrec;
};
						
						/* ------------------------------------------------------------- */
						

typedef V1ArrayOf<SqlToken*> SqlSubExpress;


enum { aucunColType=0L, sql_column_field, sql_column_expr };
						
class SqlColumn : public ObjInCacheMemory
{
	public:
		SqlColumn(sLONG typ, SqlContext* thecontext) { kind = typ; context = thecontext;};
		sLONG GetColumnType(void) { return kind; };
		virtual void GetValueAsText(VString& into) = 0;
		
	protected:
		sLONG kind;
		SqlContext *context;
};



class SqlColumnField : public SqlColumn
{
	public:
		SqlColumnField(Field* cri, SqlContext* thecontext):SqlColumn(sql_column_field, thecontext) { SourceField = cri;};
		Field* GetField(void) { return SourceField; };
		virtual void GetValueAsText(VString& into);
		
	protected:
		Field* SourceField;
};


class SqlColumnExpr : public SqlColumn
{
	public:
		SqlColumnExpr(SqlSubExpress *e, SqlContext* thecontext):SqlColumn(sql_column_expr, thecontext) { Expr = e; };
		virtual ~SqlColumnExpr();
		virtual void GetValueAsText(VString& into);
		
	protected:
		SqlSubExpress *Expr;
};


						
						/* ------------------------------------------------------------- */

enum { aucunsqlnode=0L, sqlnode_seqcomp, sqlnode_indexcomp, sqlnode_jointure, sqlnode_oper};

class SqlNode : public ObjInCacheMemory
{
	public:
		SqlNode(sLONG typ) { kind = typ; left = nil; right = nil; parent = nil; ToBeDeleted = false;};
		sLONG GetNodeType(void) { return kind; };
		void SetLeft(SqlNode* xleft) { left = xleft; if (xleft != nil) xleft->SetParent(this); };
		void SetRight(SqlNode* xright) { right = xright; if (xright != nil) xright->SetParent(this); };
		void SetParent(SqlNode* pere) { parent = pere; };
		SqlNode* GetLeft(void) { return left; };
		SqlNode* GetRight(void) { return right; };
		SqlNode* GetParent(void) { return parent; };
		void DisposeTree(void);
		// void DelFromTree(void);
		void MarkForDelete(void) { ToBeDeleted = true; };
		Boolean IsToBeDeleted(void) { return ToBeDeleted; };
		
		virtual Boolean IsAllSeq();
		virtual VError Perform(Bittab &sel, SqlContext &context);
		virtual Boolean ComputeBoolValue(SqlContext &context);

		void DisplayTreeForDebug(sLONG level);
		virtual void DisplayForDebug(sLONG level) = 0;
		
		
	protected:
		sLONG kind;
		SqlNode* left;
		SqlNode* right;
		SqlNode* parent;
		Boolean ToBeDeleted;
};


class SqlNodeOper : public SqlNode
{
	public:
		SqlNodeOper(sLONG subtype):SqlNode(sqlnode_oper) { subkind = subtype;};
		sLONG GetOper(void) { return subkind; };
		virtual void DisplayForDebug(sLONG level);
		
		virtual VError Perform(Bittab &sel, SqlContext &context);
		virtual Boolean IsAllSeq();
		virtual Boolean ComputeBoolValue(SqlContext &context);
		
	protected:
		sLONG subkind;
};

						
class SqlNodeComp : public SqlNode
{
	public:
		SqlNodeComp():SqlNode(sqlnode_seqcomp) { cri = nil; v1 = nil; v2 = nil; };
		SqlNodeComp(Field *xcri, sLONG xoper, ValPtr xv1, ValPtr xv2, Boolean xstrict1, Boolean xstrict2):SqlNode(sqlnode_seqcomp) 
						{ cri = xcri; oper = xoper; v1 = xv1; v2 = xv2; strict1 = xstrict1; strict2 = xstrict2; };
		void SetComp(Field *xcri, sLONG xoper, ValPtr xv);
		void SetFourchette(Field *xcri, ValPtr xv1, ValPtr xv2, sLONG xoper = sql_oper_between, Boolean xstrict1 = false, Boolean xstrict2 = false );
		virtual void DisplayForDebug(sLONG level);
		
		Boolean CouldUseIndex(void) { return ( cri->IsIndexe() ); };
		Boolean CouldFork(void) { return ( oper == sql_oper_sup || oper == sql_oper_supegal || oper == sql_oper_inf || oper == sql_oper_infegal ); };
		Field* GetField(void) { return cri; };
		sLONG GetOper(void) { return oper; };
		ValPtr GetValue1(void) { return v1; };
		ValPtr GetValue2(void) { return v2; };
		Boolean GetStrict1(void) { return strict1; };
		Boolean GetStrict2(void) { return strict2; };

		virtual VError Perform(Bittab &sel, SqlContext &context);
		virtual Boolean ComputeBoolValue(SqlContext &context);
		
	protected:
		Field *cri;
		sLONG oper;
		Boolean strict1, strict2;
		ValPtr v1;
		ValPtr v2;
		
};


class SqlNodeIndex : public SqlNode
{
	public:
		SqlNodeIndex(IndexInfo* xind, IndexValue* xv1, IndexValue* xv2, sLONG xoper, Boolean xtrict1 = false, Boolean xtrict2 = false )
			:SqlNode(sqlnode_indexcomp) { ind = xind; v1 = xv1; v2 = xv2; oper = xoper; strict1 = xtrict1; strict2 = xtrict2; fSubstitue = nil; };
		virtual ~SqlNodeIndex();
		virtual void DisplayForDebug(sLONG level);

		virtual VError Perform(Bittab &sel, SqlContext &context);
		virtual Boolean IsAllSeq();
		virtual Boolean ComputeBoolValue(SqlContext &context);
		
	protected:
		IndexInfo *ind;
		sLONG oper;
		Boolean strict1, strict2;
		IndexValue *v1;
		IndexValue *v2;
		SqlNode* fSubstitue;
		
};


class SqlNodeJoint : public SqlNode
{
	public:
		SqlNodeJoint(void):SqlNode(sqlnode_jointure) { cri1 = nil; oper = sql_oper_egal; cri2 = nil; };
		void SetJoint(Field *xcri1, sLONG xoper, Field *xcri2 );
		virtual void DisplayForDebug(sLONG level);
		
	protected:
		Field *cri1;
		sLONG oper;
		Field *cri2;
};


				
						
						/* ------------------------------------------------------------- */
						
typedef V1ArrayOf<SqlNode*> SqlNodeArray;

typedef SqlTokenSimple *SqlTokenPtr;
typedef V1ArrayOf<SqlTokenPtr> SqlTokenArray;

typedef BinTreePtr<SqlTokenPtr> SqlTokensTree;

typedef SqlToken *AnySqlTokenPtr;
typedef V1ArrayOf<sLONG> sLONGArray;

class SqlQuery : public ObjInCacheMemory
{
	public:
		SqlQuery(Base4D* bd);
		virtual ~SqlQuery();
		VError BuildFromText(VString *source);
		VError ExecSql(Selection* &sel);
		
		VError GetSubexpress(SqlSubExpress& subexpr, SqlToken** lasttok);
		VError GetColumns(void);
		VError SkipTables(void);
		VError ParseWhere(void);
		VError FetchData(Selection* &sel);

		VError AddToken(SqlToken *tok);
		void StepBackOneToken(void);
		SqlToken* NextToken(void);
		sLONG SaveState(void) { return curtoken; };
		void RestoreState(sLONG newstate) { curtoken = newstate; };
		
		inline Base4D* GetTarget() { return TargetBase; };
		
	protected:
	
		Field* FindAndRetainField(VString& fieldname);
		VError ResolveUnResolvedRef(void);
		SqlNode* xFactorizeIndex(SqlNode* node, VError *error, SqlNodeArray* deja);
		SqlNode* xParseWhere(VError* error);
		SqlNode* BuildComp(Field *xcri1, Field *xcri2, ValPtr xv1, ValPtr xv2, sLONG xoper, VError *error);
		SqlNode* CleanTree(SqlNode* node);
		VError CheckForIndex(SqlNode* &root, SqlNodeArray *Nodes, sLONG startNode, sLONG curlen, sLONG maxlen, FieldsArray *FieldsToCheck, sLONGArray *Indices);
		
		V1ArrayOf<AnySqlTokenPtr> Tokens;
		Base4D *TargetBase;
		sLONG curtoken;
		V1ArrayOf<Table*> FromTable;
		V1ArrayOf<SqlColumn*> Columns;
		SqlContext* context;
		SqlNode* NodeRoot;
		
};

     /* ============================================================================== */

VError InitSQL(void);

#endif

#endif
