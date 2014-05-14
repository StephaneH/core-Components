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
#ifndef _JAVASCRIPTPARSER_H_
#define _JAVASCRIPTPARSER_H_

// We have to include ScriptDoc in order for destructors to fire properly
#include "ScriptDoc.h"
#include "JavaScriptAST.h"

#define JAVASCRIPT_PARSER_SUGGESTION_SEPARATOR CVSTR("||")
#define JAVASCRIPT_PARSER_SUGGESTION_COLUMNS_NUMBER 2

namespace EntityModelCompletionType {
	enum {
		kNone = 0,
		kEntity,
		kCollection,
		kClass
	};
};




class JavaScriptParser {
public:
	JavaScriptParser();
	virtual ~JavaScriptParser();
	
	JavaScriptAST::Node*	GetAST(	VFilePath inFilePath,
									VString* inInput,
									bool& outError);
	
	
	void	GetSuggestions(	VString* inInput,
							SuggestionList* inSuggestions,
							Symbols::ISymbol* inContext,
							Symbols::IFile* inContextFile,
							ISymbolTable* inTable=NULL);
	
	
	void	AssignDelegate(JavaScriptParserDelegate* del)	{ xbox_assert( !del || !fDelegate );  fDelegate = del; }
	
	
	static XBOX::VString	GetSuggestionsSeparator()		{ return JAVASCRIPT_PARSER_SUGGESTION_SEPARATOR; }
	static sLONG			GetSuggestionsNumberOfColumns()	{ return JAVASCRIPT_PARSER_SUGGESTION_COLUMNS_NUMBER; }
	
	
	
private:
	// This is a helper type that is used for exception handling purposes only
	typedef int ParsingErrorException;
	const ParsingErrorException		kParsingError;
	
	JavaScriptLexer*				fLexer;
	// This delegate is used for error reporting, and it is not a pointer that we own
	JavaScriptParserDelegate*		fDelegate;
	bool							fWasError;
	XBOX::VFilePath					fFilePath;
	
	
	//
	// Expressions productions
	//	(as it is required for ECMA 262 5.1)
	JavaScriptAST::Node*						ParsePrimaryExpression			(int& inoutToken);
	JavaScriptAST::Node*						ParseLiteral					(int& inoutToken);
	JavaScriptAST::Node*						ParseArrayLiteral				(int& inoutToken);
	bool										ParseElementList				(int& inoutToken);
	void										ParseElision					(int& inoutToken);
	JavaScriptAST::Node*						ParseObjectLiteral				(int& inoutToken);
	bool										ParsePropertyNameAndValueList	(int& inoutToken, JavaScriptAST::ObjectLiteralNode* inNode);
	JavaScriptAST::Node*						ParsePropertyName				(int& inoutToken);
	JavaScriptAST::Node*						ParseMemberExpression			(int& inoutToken, bool inArgsRequired);
	JavaScriptAST::Node*						ParseCallExpressionSubscript	(int& inoutToken, JavaScriptAST::Node* inNode);
	JavaScriptAST::FunctionCallArgumentsNode*	ParseArguments					(int& inoutToken, int &outArgsCount);
	JavaScriptAST::FunctionCallArgumentsNode*	ParseArgumentList				(int& inoutToken, int &outArgsCount);
	JavaScriptAST::Node*						ParseLeftHandSideExpression		(int& inoutToken);
	JavaScriptAST::Node*						ParsePostFixExpression			(int& inoutToken);
	JavaScriptAST::Node*						ParseUnaryExpression			(int& inoutToken);
	JavaScriptAST::Node*						ParseMultiplicativeExpression	(int& inoutToken);
	JavaScriptAST::Node*						ParseAdditiveExpression			(int& inoutToken);
	JavaScriptAST::Node*						ParseShiftExpression			(int& inoutToken);
	JavaScriptAST::Node*						ParseRelationalExpression		(int& inoutToken, bool inNoIn);
	JavaScriptAST::Node*						ParseEqualityExpression			(int& inoutToken, bool inNoIn);
	JavaScriptAST::Node*						ParseBitwiseAndExpression		(int& inoutToken, bool inNoIn);
	JavaScriptAST::Node*						ParseBitwiseXorExpression		(int& inoutToken, bool inNoIn);
	JavaScriptAST::Node*						ParseBitwiseOrExpression		(int& inoutToken, bool inNoIn);
	JavaScriptAST::Node*						ParseLogicalAndExpression		(int& inoutToken, bool inNoIn);
	JavaScriptAST::Node*						ParseLogicalOrExpression		(int& inoutToken, bool inNoIn);
	JavaScriptAST::Node*						ParseConditionalExpression		(int& inoutToken, bool inNoIn);
	JavaScriptAST::Node*						ParseAssignmentExpression		(int& inoutToken, bool inNoIn);
	JavaScriptAST::Node*						ParseExpression					(int& inoutToken, bool inNoIn);
	//
	// Statements productions
	//	(as it is required for ECMA 262 5.1)
	JavaScriptAST::Node*	ParseStatement					(int& inoutToken);
	JavaScriptAST::Node*	ParseStatementBlock				(int& inoutToken);
	JavaScriptAST::Node*	ParseStatementList				(int& inoutToken);
	JavaScriptAST::Node*	ParseVariableStatement			(int& inoutToken);
	JavaScriptAST::Node*	ParseVariableDeclarationList	(int& inoutToken, bool inNoIn, int* outNumberOfDecls=NULL);
	JavaScriptAST::Node*	ParseVariableDeclaration		(int& inoutToken, bool inNoIn);
	JavaScriptAST::Node*	ParseEmptyStatement				(int& inoutToken);
	JavaScriptAST::Node*	ParseExpressionStatement		(int& inoutToken);
	JavaScriptAST::Node*	ParseIfStatement				(int& inoutToken);
	JavaScriptAST::Node*	ParseIterationStatement			(int& inoutToken);
	JavaScriptAST::Node*	ParseForStatement				(int& inoutToken);
	JavaScriptAST::Node*	ParseContinueStatement			(int& inoutToken);
	JavaScriptAST::Node*	ParseBreakStatement				(int& inoutToken);
	JavaScriptAST::Node*	ParseReturnStatement			(int& inoutToken);
	JavaScriptAST::Node*	ParseWithStatement				(int& inoutToken);
	JavaScriptAST::Node*	ParseSwitchStatement			(int& inoutToken);
	bool					ParseCaseBlock					(int& inoutToken, JavaScriptAST::SwitchStatementNode* inSwitch);
	bool					ParseCaseClauses				(int& inoutToken, JavaScriptAST::SwitchStatementNode* inSwitch);
	JavaScriptAST::Node*	ParseCaseClause					(int& inoutToken);
	JavaScriptAST::Node*	ParseDefaultClause				(int& inoutToken);
	JavaScriptAST::Node*	ParseLabeledStatement			(int& inoutToken);
	JavaScriptAST::Node*	ParseThrowStatement				(int& inoutToken);
	JavaScriptAST::Node*	ParseTryStatement				(int& inoutToken);
	JavaScriptAST::Node*	ParseCatch						(int& inoutToken);
	JavaScriptAST::Node*	ParseFinally					(int& inoutToken);
	JavaScriptAST::Node*	ParseDebuggerStatement			(int& inoutToken);
	// The following ones doe not exist in the ECMA 262 5.1 requirements but adds automatic semi colon insertion
	bool					ParseStatementCompletion		(int& inoutToken);
	bool					IsStatementBlock				(int inToken);
	bool					IsIfStatement					(int inToken);
	bool					IsIterationStatement			(int inToken);
	bool					IsContinueStatement				(int inToken);
	bool					IsBreakStatement				(int inToken);
	bool					IsDebuggerStatement				(int inToken);
	bool					IsReturnStatement				(int inToken);
	bool					IsWithStatement					(int inToken);
	bool					IsSwitchStatement				(int inToken);
	bool					IsThrowStatement				(int inToken);
	bool					IsTryStatement					(int inToken);
	bool					IsVariableStatement				(int inToken);
	bool					IsFunctionStatement				(int inToken);
	bool					IsScriptDocElement				();
	ScriptDocComment*		ParseScriptDocElement			();
	
	//
	// Functions and Programs productions
	//	(as it is required for ECMA 262 5.1)
	//
	JavaScriptAST::Node*								ParseFunctionDeclaration	(int& inoutToken, const VString& inFunctionName="");
	JavaScriptAST::Node*								ParseFunctionExpression		(int& inoutToken);
	JavaScriptAST::FunctionDeclarationArgumentsNode*	ParseFormalParameterList	(int& inoutToken);
	JavaScriptAST::Node*								ParseFunctionBody			(int& inoutToken);
	JavaScriptAST::Node*								ParseProgram				(void);
	JavaScriptAST::Node*								ParseSourceElements			(int& inoutToken);
	JavaScriptAST::Node*								ParseSourceElement			(int& inoutToken);
	// The following one does not exist in ECMA 262 5.1 but as the only difference between FunctionDeclaration and FunctionExpression rules is
	// that the identifier is optionnal for FunctionExpression rule. As a consequence, we create a common method which takes an argument to
	// know if the identifier is optionnal or not
	JavaScriptAST::Node*								ParseFunction				(int& inoutToken, const VString& inFunctionName="", bool inFunctionIdentifierIsMandatory=true);

	void RecoverFromError(int &inoutToken);
	
	// NOTE: ThrowError will actually throw a C++ exception, so callers are expected to
	// have a C++ exception handler on their call stack to handle it.  Nothing should
	// ever generate an unhandled exception.
	void ThrowError(const VString& inMessage)
	{
		fWasError = true;
		if (fDelegate)
			fDelegate->Error( fFilePath, fLexer->GetCurrentLineNumber(), fLexer->GetCurrentCharacterOffset(), inMessage );
		
		throw kParsingError;
	}
	
	

	// Specific to suggestion computation
	ISymbolTable*					fSymbolTable;
	Symbols::ISymbol*				fCompletionSymbol;
	std::vector<Symbols::ISymbol*>	fCompletionSymbolList;
	Symbols::ISymbol*				fEntityCompletionSymbol;
	Symbols::ISymbol*				fContextSymbol;
	Symbols::IFile*					fContextFile;
	class SuggestionList*			fSuggestions;

	
	
	void	_SuggestIdentifiers(bool inIsWidget=false, bool inIsFunctionCall=false);
	void	_SuggestCallParameters(sLONG inIndex);
	void	_SuggestReturnTypes(sLONG& outEntityCompletionType, std::vector<Symbols::ISymbol*>& outReturnTypes);
	void	_SuggestWidgetIdentifiers();
	Symbols::ISymbol*	_GetWafOrSourcesSymbol(const VString& sourceId);
	bool	_SuggestWidgetPrototype(const VString& widgetId);

	void _SuggestUndefined	(class SuggestionList* ioSuggestions) {	if( ioSuggestions )		ioSuggestions->Suggest( "undefined",	SuggestionInfo::eKeyword );	};
	void _SuggestNull		(class SuggestionList* ioSuggestions) {	if( ioSuggestions )		ioSuggestions->Suggest( "null",			SuggestionInfo::eKeyword );	};
	void _SuggestTrue		(class SuggestionList* ioSuggestions) {	if( ioSuggestions )		ioSuggestions->Suggest( "true",			SuggestionInfo::eKeyword );	};
	void _SuggestFalse		(class SuggestionList* ioSuggestions) {	if( ioSuggestions )		ioSuggestions->Suggest( "false",		SuggestionInfo::eKeyword );	};
	void _SuggestThis		(class SuggestionList* ioSuggestions) {	if( ioSuggestions )		ioSuggestions->Suggest( "this",			SuggestionInfo::eKeyword );	};
	void _SuggestArguments	(class SuggestionList* ioSuggestions) {	if( ioSuggestions )		ioSuggestions->Suggest( "arguments",	SuggestionInfo::eName );	};
	void _SuggestNew		(class SuggestionList* ioSuggestions) {	if( ioSuggestions )		ioSuggestions->Suggest( "new",			SuggestionInfo::eKeyword );	};
	void _SuggestFunction	(class SuggestionList* ioSuggestions) {	if( ioSuggestions )		ioSuggestions->Suggest( "function",		SuggestionInfo::eKeyword );	};
	void _SuggestDelete		(class SuggestionList* ioSuggestions) {	if( ioSuggestions )		ioSuggestions->Suggest( "delete",		SuggestionInfo::eKeyword );	};
	void _SuggestVoid		(class SuggestionList* ioSuggestions) {	if( ioSuggestions )		ioSuggestions->Suggest( "void",			SuggestionInfo::eKeyword );	};
	void _SuggestTypeof		(class SuggestionList* ioSuggestions) {	if( ioSuggestions )		ioSuggestions->Suggest( "typeof",		SuggestionInfo::eKeyword );	};
	void _SuggestInstanceof	(class SuggestionList* ioSuggestions) {	if( ioSuggestions )		ioSuggestions->Suggest( "instanceof",	SuggestionInfo::eKeyword );	};
	void _SuggestIn			(class SuggestionList* ioSuggestions) {	if( ioSuggestions )		ioSuggestions->Suggest( "in",			SuggestionInfo::eKeyword );	};
	void _SuggestVar		(class SuggestionList* ioSuggestions) {	if( ioSuggestions )		ioSuggestions->Suggest( "var",			SuggestionInfo::eKeyword );	};
	void _SuggestIf			(class SuggestionList* ioSuggestions) {	if( ioSuggestions )		ioSuggestions->Suggest( "if",			SuggestionInfo::eKeyword );	};
	void _SuggestDo			(class SuggestionList* ioSuggestions) {	if( ioSuggestions )		ioSuggestions->Suggest( "do",			SuggestionInfo::eKeyword );	};
	void _SuggestWhile		(class SuggestionList* ioSuggestions) {	if( ioSuggestions )		ioSuggestions->Suggest( "while",		SuggestionInfo::eKeyword );	};
	void _SuggestFor		(class SuggestionList* ioSuggestions) {	if( ioSuggestions )		ioSuggestions->Suggest( "for",			SuggestionInfo::eKeyword );	};
	void _SuggestBreak		(class SuggestionList* ioSuggestions) {	if( ioSuggestions )		ioSuggestions->Suggest( "break",		SuggestionInfo::eKeyword );	};
	void _SuggestContinue	(class SuggestionList* ioSuggestions) {	if( ioSuggestions )		ioSuggestions->Suggest( "continue",		SuggestionInfo::eKeyword );	};
	void _SuggestReturn		(class SuggestionList* ioSuggestions) {	if( ioSuggestions )		ioSuggestions->Suggest( "return",		SuggestionInfo::eKeyword );	};
	void _SuggestWith		(class SuggestionList* ioSuggestions) {	if( ioSuggestions )		ioSuggestions->Suggest( "with",			SuggestionInfo::eKeyword );	};
	void _SuggestSwitch		(class SuggestionList* ioSuggestions) {	if( ioSuggestions )		ioSuggestions->Suggest( "switch",		SuggestionInfo::eKeyword );	};
	void _SuggestThrow		(class SuggestionList* ioSuggestions) {	if( ioSuggestions )		ioSuggestions->Suggest( "throw",		SuggestionInfo::eKeyword );	};
	void _SuggestTry		(class SuggestionList* ioSuggestions) {	if( ioSuggestions )		ioSuggestions->Suggest( "try",			SuggestionInfo::eKeyword );	};
	void _SuggestCase		(class SuggestionList* ioSuggestions) {	if( ioSuggestions )		ioSuggestions->Suggest( "case",			SuggestionInfo::eKeyword );	};
	void _SuggestDefault	(class SuggestionList* ioSuggestions) {	if( ioSuggestions )		ioSuggestions->Suggest( "default",		SuggestionInfo::eKeyword );	};
	void _SuggestCatch		(class SuggestionList* ioSuggestions) {	if( ioSuggestions )		ioSuggestions->Suggest( "catch",		SuggestionInfo::eKeyword );	};
	void _SuggestFinally	(class SuggestionList* ioSuggestions) {	if( ioSuggestions )		ioSuggestions->Suggest( "finally",		SuggestionInfo::eKeyword );	};
	

	Symbols::ISymbol*	_QualifiedLookup(Symbols::ISymbol* inSym, const VString &inMember);
	Symbols::ISymbol*	_UnqualifiedLookup(const VString& inName);

	// Used only within _SuggestReturnTypes
	bool _GetPrototypeSubSymbol(Symbols::ISymbol* inOwner, Symbols::ISymbol** outPrototype);

	// Used only to get the core symbol of "Boolean", "String", "Number", "RegExp", "Array" or "Object"
	// It could be better to get each of those symbols once
	Symbols::ISymbol* _GetCoreSymbolByName(const VString& inName);

	bool _CheckCompatibleFileContext(Symbols::IFile* inFile1, Symbols::IFile* inFile2);

	void _SetEntityCompletionSymbol(Symbols::ISymbol* inEntityCompletionSymbol);
	Symbols::ISymbol* _GetEntityCompletionSymbol();
	
	void _SetCompletionSymbol(Symbols::ISymbol* inCompletionSymbol);
	void _SetCompletionSymbolList(std::vector<Symbols::ISymbol*>& inCompletionSymbolList);
	void _SetCompletionSymbolList(Symbols::ISymbol* inSymbol);

	void _ResetCompletionSymbolList();
	void _ExpandCompletionSymbolList();

	void _BackupCompletionSymbols(Symbols::ISymbol** ioCompletionSymbolBackup, std::vector<Symbols::ISymbol*>& ioCompletionSymbolListBackup);
	void _RestoreCompletionSymbols(Symbols::ISymbol* ioCompletionSymbolBackup, std::vector<Symbols::ISymbol*> ioCompletionSymbolListBackup);
	
	bool _IsJsScriptFromWaPage() const;
	bool _GetHtmlRelativeFilePathString(XBOX::VString& outPathString);
	
	bool _IsWidgetSelector(const XBOX::VString& inId) const;
	bool _IsSourceSelector(const XBOX::VString& inId) const;
};

#endif // _JAVASCRIPTPARSER_H_