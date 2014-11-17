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
#ifndef __SQLSYNTAX__
#define __SQLSYNTAX__

class VSQLSyntaxParams : public VObject, public ISyntaxParams
{
public:
	VSQLSyntaxParams() { fMap = new std::map< int, bool >; }
	virtual ~VSQLSyntaxParams() { delete fMap; }
	std::map< int, bool >* GetCommentMap() const { return fMap; }
private:
	void SelfDelete() {delete this;}
	std::map< int, bool >* fMap;
};

class VSQLSyntax : public VObject, public ISyntaxInterface
{
private:
	IAutoCompleteSymbolTable *fSymTable;
	sLONG fTabWidth;

	enum ParserReturnValue {
		eParseSucceeded = 0,
		eNothingLeftToParse,
		eIllegalValueParsed,
		eUnexpectedTokenParsed,		// Used when parsing optional clauses
	};

	SQLTokenizeFuncPtr			fTokenizeFuncPtr;
	std::vector< VString * >	fKeywords;
	std::vector< VString * >	fFunctions;

	std::map< int, bool>* GetCommentMap( ICodeEditorDocument* inDocument );
	void CleanTokens( std::vector< class ILexerToken * > &inTokens );

	class TokenListIterator *fTokenList;
	bool ParseTokenList( std::vector< class ILexerToken * > inTokens, class SuggestionList &outSuggestions );
	class ILexerToken *GetNextToken();
	class ILexerToken *PeekNextToken();
	void PutBackToken();

	void MakeSQLSafeName( VString &ioName );
	bool IsSQLSafeName( VString inName );

	bool FindStatementStart( ICodeEditorDocument* inDocument, sLONG inLineIndex, sLONG &outStart );
	void UpdateCompileErrors( ICodeEditorDocument *inDocument, sLONG inStartIndex, sLONG inLastIndex );
	void ComputeFolding( ICodeEditorDocument *inDocument, sLONG inStartIndex, sLONG inLastIndex );

	bool ParseTopLevel( class SuggestionList &outSuggestions );
	bool ParseDebugStatement( class SuggestionList &outSuggestions );
	bool ParseLockStatement( class SuggestionList &outSuggestions );
	bool ParseCreateIndexStatement( class SuggestionList &outSuggestions );
	bool ParseCreateTableStatement( class SuggestionList &outSuggestions );
	bool ParseAlterTableStatement( class SuggestionList &outSuggestions );
	bool ParseExecuteImmediateStatement( class SuggestionList &outSuggestions );
	bool ParseReplicateStatement( class SuggestionList &outSuggestions );
	bool ParseUpdateStatement( class SuggestionList &outSuggestions );
	bool ParseDeleteStatement( class SuggestionList &outSuggestions );
	bool ParseCreateViewStatement( class SuggestionList &outSuggestions );
	bool ParseInsertStatement( class SuggestionList &outSuggestions );
	bool ParseSynchronizeStatement( class SuggestionList &outSuggestions );
	
	ParserReturnValue ParseFrom( class SuggestionList &outSuggestions, class ILexerToken * &outToken );
	ParserReturnValue ParseSynchronizeMergeMode( class SuggestionList &outSuggestions );
	ParserReturnValue ParseLiteral( class SuggestionList &outSuggestions, class ILexerToken * &outToken );
	ParserReturnValue ParseTable( class SuggestionList &outSuggestions, class ILexerToken * &outToken, VString *outTableName = NULL, bool inIncludeTables = true, bool inIncludeViews = true );
	ParserReturnValue ParseColumnRef( class SuggestionList &outSuggestions, class ILexerToken * &outToken );
	ParserReturnValue ParseColumnRefList( class SuggestionList &outSuggestions, class ILexerToken * &outToken );
	ParserReturnValue ParseAlterTableAdd( class SuggestionList &outSuggestions, class ILexerToken * &outToken );
	ParserReturnValue ParsePrimaryOrForeignKeyDef( class SuggestionList &outSuggestions, class ILexerToken * &outToken );
	ParserReturnValue ParseForeignKeyReferences( class SuggestionList &outSuggestions, class ILexerToken * &outToken );
	ParserReturnValue ParseReplicateItem( class SuggestionList &outSuggestions, class ILexerToken * &outToken );
	ParserReturnValue ParseReplicateList( class SuggestionList &outSuggestions, class ILexerToken * &outToken );
	ParserReturnValue Parse4DReference( class SuggestionList &outSuggestions, class ILexerToken * &outToken );
	ParserReturnValue Parse4DReferenceList( class SuggestionList &outSuggestions, class ILexerToken * &outToken );
	ParserReturnValue ParseDataType( class SuggestionList &outSuggestions, class ILexerToken * &outToken );
	ParserReturnValue ParseDataTypeEncoding( class SuggestionList &outSuggestions, class ILexerToken * &outToken );
	ParserReturnValue ParseColumnDef( class SuggestionList &outSuggestions, class ILexerToken * &outToken );
	ParserReturnValue ParseOptionalColumnDefOption( class SuggestionList &outSuggestions, class ILexerToken * &outToken );
	ParserReturnValue ParseOptionalColumnDefOptionsList( class SuggestionList &outSuggestions, class ILexerToken * &outToken );
	ParserReturnValue ParseBaseTableElement( class SuggestionList &outSuggestions, class ILexerToken * &outToken );
	ParserReturnValue ParseBaseTableElementList( class SuggestionList &outSuggestions, class ILexerToken * &outToken );
	ParserReturnValue ParseTerminal( class SuggestionList &outSuggestions, class ILexerToken * &outToken );
	ParserReturnValue ParseScalarExpression( class SuggestionList &outSuggestions, class ILexerToken * &outToken );
	ParserReturnValue ParseScalarFunction( class SuggestionList &outSuggestions, class ILexerToken * &outToken );
	ParserReturnValue ParseCaseExpression( class SuggestionList &outSuggestions, class ILexerToken * &outToken );
	ParserReturnValue ParseWhenConditionThenExpressionList( class SuggestionList &outSuggestions, class ILexerToken * &outToken );
	ParserReturnValue ParseWhenConditionThenExpression( class SuggestionList &outSuggestions, class ILexerToken * &outToken );
	ParserReturnValue ParseOptionalElseExpression( class SuggestionList &outSuggestions, class ILexerToken * &outToken );
	ParserReturnValue ParseThenExpression( class SuggestionList &outSuggestions, class ILexerToken * &outToken );
	ParserReturnValue ParseSearchCondition( class SuggestionList &outSuggestions, class ILexerToken * &outToken );
	ParserReturnValue ParseSearchConditionTerminal( class SuggestionList &outSuggestions, class ILexerToken * &outToken );
	ParserReturnValue ParsePredicate( class SuggestionList &outSuggestions, class ILexerToken * &outToken );
	ParserReturnValue ParseSubquery( class SuggestionList &outSuggestions, class ILexerToken * &outToken, bool inJustBody = false );
	ParserReturnValue ParseSelectStatement( class SuggestionList &outSuggestions, class ILexerToken * &outToken, bool inForSubquery );
	ParserReturnValue ParseSelection( class SuggestionList &outSuggestions, class ILexerToken * &outToken );
	ParserReturnValue ParseTableRefTerminal( class SuggestionList &outSuggestions, class ILexerToken* &outToken, bool inIncludeViews = false );
	ParserReturnValue ParseTableRef( class SuggestionList &outSuggestions, class ILexerToken * &outToken, bool inIncludeViews = false );
	ParserReturnValue ParseTableRefList( class SuggestionList &outSuggestions, class ILexerToken * &outToken );
	ParserReturnValue ParseOrderingSpecification( class SuggestionList &outSuggestions, class ILexerToken * &outToken );
	ParserReturnValue ParseOrderingSpecificationList( class SuggestionList &outSuggestions, class ILexerToken * &outToken );
	ParserReturnValue ParseAssignment( class SuggestionList &outSuggestions, class ILexerToken * &outToken );
	ParserReturnValue ParseAssignmentList( class SuggestionList &outSuggestions, class ILexerToken * &outToken );
	ParserReturnValue ParseAon( class SuggestionList &outSuggestions, class ILexerToken * &outToken );
	ParserReturnValue ParseListOfAonLists( class SuggestionList &outSuggestions, class ILexerToken * &outToken );
	ParserReturnValue ParseAonList( class SuggestionList &outSuggestions, class ILexerToken * &outToken );
	ParserReturnValue ParseOptionalTableProperty( class SuggestionList &outSuggestions, class ILexerToken *&outToken );
	ParserReturnValue ParseOptionalTablePropertyList( class SuggestionList &outSuggestions, class ILexerToken *&outToken );
	ParserReturnValue ParseOptionalReplicationMergeMode( class SuggestionList &outSuggestions, class ILexerToken *&outToken );
	ParserReturnValue ParseCommandParameter( class SuggestionList &outSuggestions, class ILexerToken *&outToken );
	ParserReturnValue ParseForStampClause( class SuggestionList &outSuggestions, class ILexerToken *&outToken );
	ParserReturnValue ParseOptionalDestStampClause( class SuggestionList &outSuggestions, class ILexerToken *&outToken );
	ParserReturnValue ParseOptionalLatestStampClause( class SuggestionList &outSuggestions, class ILexerToken *&outToken );
	ParserReturnValue ParseLimitClause( class SuggestionList &outSuggestions, class ILexerToken *&outToken );
	ParserReturnValue ParseOffsetClause( class SuggestionList &outSuggestions, class ILexerToken *&outToken );

	enum OnClause {
		eAll = 0x3,  // A mask of all possible values in the enumeration
		eDelete = 1 << 0,
		eUpdate = 1 << 1,
	};

	ParserReturnValue ParseOnClause( class SuggestionList &outSuggestions, class ILexerToken * &outToken, OnClause inAllowed, OnClause &outFound );
public:
	explicit VSQLSyntax();
	virtual ~VSQLSyntax();

	void SetSQLTokenizer ( SQLTokenizeFuncPtr inPtr, const std::vector< XBOX::VString * >& vctrSQLKeywords, const std::vector< XBOX::VString * >& vctrSQLFunctions );

	// la ligne 
	virtual void Init( ICodeEditorDocument* inDocument );
	virtual void Load( ICodeEditorDocument* inDocument, VString& ioContent );
	virtual void Save( ICodeEditorDocument* inDocument, VString& ioContent );
	virtual void SaveLine( ICodeEditorDocument* inDocument, sLONG inLineIndex, VString& ioContent );
	virtual void Close( ICodeEditorDocument* inDocument );
	virtual void SetLine( ICodeEditorDocument* inDocument, sLONG inLineNumber, bool inLoading );
	virtual sLONG GetIndentWidth();
	virtual bool CheckFolding( ICodeEditorDocument* inDocument, sLONG inLineNumber );
	virtual void ComputeFolding( ICodeEditorDocument* inDocument );
	virtual bool CheckOutline( ICodeEditorDocument* inInterface, sLONG inLineNumber );
	virtual void ComputeOutline( ICodeEditorDocument* inInterface );
	virtual void TokenizeLine( ICodeEditorDocument* inDocument, sLONG inLineIndex, std::vector<XBOX::VString>& ioLines );
	virtual void GetTip( ICodeEditorDocument* inDocument, sLONG inLine, sLONG inPos, VString& outText, Boolean& outRtfText );
	virtual void GetSuggestions( ICodeEditorDocument* inDocument, sLONG inLineIndex, sLONG inPos, ITipInfoArray *outSuggestions, sLONG& outStartOffset, bool inAll );
	virtual void SetSymbolTable( IAutoCompleteSymbolTable *inTable ) { if (inTable) {inTable->Retain();} if (fSymTable) { fSymTable->Release(); } fSymTable = inTable; }
	virtual bool DetermineMorphemeBoundary( ICodeEditorDocument *inDocument, sLONG inLineIndex, sLONG inOffset, sLONG &outLeftBoundary, sLONG &outLength, bool inAlternateKey );
	virtual bool DoubleClick( ICodeEditorDocument* inDocument, sLONG inLineIndex, sLONG inOffset, bool inAlternateKey );
	virtual bool UseTab();	
	virtual void InsertChar( ICodeEditorDocument* inDocument, UniChar inUnichar, sLONG inLineIndex, sLONG inPosition, ITipInfoArray *outSuggestions, sLONG& outStartOffset );
	virtual void UpdateBreakPointsLineNumbers( ICodeEditorDocument* inDocument, const std::vector<sWORD>& inBreakIDs, const std::vector<sLONG>& inLineNumbers );
	virtual void AddBreakPoint( ICodeEditorDocument* inDocument, sLONG inLineNumber, sWORD& outID, bool& outDisabled );
	virtual bool EditBreakPoint( ICodeEditorDocument* inDocument, sWORD inBreakID, bool& outDisabled );
	virtual void RemoveBreakPoint( ICodeEditorDocument* inDocument, sLONG inLineNumber );
	virtual bool GetBreakPoints( ICodeEditorDocument* inDocument, std::vector<sLONG>& outLineNumbers, std::vector<sWORD>& outIDs, std::vector<bool>& outDisabled );
	virtual void UpdateBreakPointContent( ICodeEditorDocument* inDocument, sWORD inBreakID, const XBOX::VString& inNewLineContent ) {}
	virtual void UpdateCompilerErrors( ICodeEditorDocument* inDocument );
	virtual void GotoNextMethodError( ICodeEditorDocument* inDocument, bool inNext );
	virtual void SwapAssignments( ICodeEditorDocument* inDocument, XBOX::VString& ioString );
	virtual bool IsComment( ICodeEditorDocument* inDocument, const XBOX::VString& inString );
	virtual void SwapComment( ICodeEditorDocument* inDocument, XBOX::VString& ioString, bool inComment );
	virtual void CallHelp( ICodeEditorDocument* inDocument );
	virtual bool ShouldValidateTipWindow( UniChar inChar );
	virtual bool IsMatchingCharacter( ICodeEditorDocument* inDocument, sLONG inLineIndex, sLONG inOffset, UniChar inChar );
	virtual bool AllowsThreading() { return true; }

	virtual void SetTabWidth( sLONG inTabWidth ) {fTabWidth = inTabWidth;}
	virtual sLONG GetTabWidth() const {return fTabWidth;}

	virtual void PerformIdleProcessing( ICodeEditorDocument *inDocument );

	virtual bool TokenizeOnKeyEnter() { return false; }
};

#endif
