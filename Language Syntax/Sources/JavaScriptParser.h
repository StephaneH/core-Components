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
private:
	// This is a helper type that is used for exception handling purposes only
	typedef int ParsingErrorException;
	const ParsingErrorException kParsingError;

	JavaScriptLexer *fLexer;
	class SuggestionList *fSuggestions;
	ISymbolTable *fSymbolTable;
	std::vector<Symbols::ISymbol *> fCompletionSymbolList, fCompletionSymbolListBackup;
	Symbols::ISymbol *fCompletionSymbol, *fCompletionSymbolBackup, *fContextSymbol, *fEntityCompletionSymbol;
	Symbols::IFile *fContextFile;

	// This delegate is used for error reporting, and it is not a pointer that we own
	JavaScriptParserDelegate *fDelegate;
	bool fWasError;
	XBOX::VFilePath fFilePath;

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

	void SetCompletionSymbol(Symbols::ISymbol *inCompletionSymbol)
	{ 
		if ( NULL != inCompletionSymbol )
			inCompletionSymbol->Retain();

		if ( NULL != fCompletionSymbol )
			fCompletionSymbol->Release();

		fCompletionSymbol = inCompletionSymbol; 
	}

	void SetCompletionSymbolList(std::vector<Symbols::ISymbol *>& inCompletionSymbolList)
	{ 
		for (std::vector<Symbols::ISymbol *>::iterator it = inCompletionSymbolList.begin(); it != inCompletionSymbolList.end(); ++it)
			(*it)->Retain();

		for (std::vector<Symbols::ISymbol *>::iterator it = fCompletionSymbolList.begin(); it != fCompletionSymbolList.end(); ++it)
			(*it)->Release();

		fCompletionSymbolList = inCompletionSymbolList; 
	}

	void SetCompletionSymbolList(Symbols::ISymbol * inSymbol)
	{
		std::vector<Symbols::ISymbol *> list;
		if (NULL != inSymbol)
			list.push_back(inSymbol);
		SetCompletionSymbolList( list );
	}

	void ResetCompletionSymbolList()
	{
		for (std::vector<Symbols::ISymbol *>::iterator it = fCompletionSymbolList.begin(); it != fCompletionSymbolList.end(); ++it)
			(*it)->Release();
		fCompletionSymbolList.clear();
	}

	void SetEntityCompletionSymbol(Symbols::ISymbol *inEntityCompletionSymbol)
	{ 
		if ( NULL != inEntityCompletionSymbol )
			inEntityCompletionSymbol->Retain();

		if ( NULL != fEntityCompletionSymbol )
			fEntityCompletionSymbol->Release();
	
		fEntityCompletionSymbol = inEntityCompletionSymbol;
	}

	void BackupCompletionSymbols(Symbols::ISymbol** ioCompletionSymbolBackup, std::vector<Symbols::ISymbol *>& ioCompletionSymbolListBackup)
	{ 
		*ioCompletionSymbolBackup = fCompletionSymbol;
		if (NULL != *ioCompletionSymbolBackup)
			(*ioCompletionSymbolBackup)->Retain();

		ioCompletionSymbolListBackup = fCompletionSymbolList;
		for (std::vector<Symbols::ISymbol *>::iterator it = ioCompletionSymbolListBackup.begin(); it != ioCompletionSymbolListBackup.end(); ++it)
			(*it)->Retain();
	}

	void RestoreCompletionSymbols(Symbols::ISymbol* ioCompletionSymbolBackup, std::vector<Symbols::ISymbol *> ioCompletionSymbolListBackup)
	{ 
		SetCompletionSymbol( ioCompletionSymbolBackup );
		SetCompletionSymbolList( ioCompletionSymbolListBackup );
	}

	Symbols::ISymbol* GetEntityCompletionSymbol() { return fEntityCompletionSymbol; }

	void ExpandCompletionSymbolList();
	void RecoverFromError( int &tk );

	void								SuggestAppropriateIdentifiers( bool inIsFunctionCall = false );
	void								SuggestCallExpressionParameterValues(sLONG inIndex);
	std::vector< Symbols::ISymbol * >	GetSuggestionReturnTypes(sLONG& outEntityCompletionType );
	bool								GetPrototypeSubSymbol(Symbols::ISymbol* inOwner, Symbols::ISymbol** outPrototype);
	bool								CheckCompatibleFileContext(Symbols::IFile* inFile1, Symbols::IFile* inFile2);

	Symbols::ISymbol *GetCoreSymbolByName( const VString &inName );
	Symbols::ISymbol *QualifiedLookup( Symbols::ISymbol *inSym, const VString &inMember );
	Symbols::ISymbol *UnqualifiedLookup( const VString &inName );

	bool IsScriptDocElement();
	ScriptDocComment *ParseScriptDocElement();

	bool ParseStatementCompletion( int &tk );

	void ParseElision( int &tk );
	bool ParseElementList( int &tk );
	JavaScriptAST::Node *ParseArrayLiteral( int &tk );
	JavaScriptAST::Node *ParseLiteral( int &tk );
	JavaScriptAST::Node *ParsePrimaryExpression( int &tk );

	bool ParsePropertyNameAndValueList( int &tk, JavaScriptAST::ObjectLiteralNode *inNode );
	JavaScriptAST::Node *ParsePropertyName( int &tk );
	JavaScriptAST::Node *ParseObjectLiteral( int &tk );

	JavaScriptAST::Node *ParseMemberExpression( int &tk, bool argsRequired );
	JavaScriptAST::Node *ParseLeftHandSideExpression( int &tk );
	JavaScriptAST::Node *ParseCallExpressionSubscript( int &tk, JavaScriptAST::Node *inNode );
	JavaScriptAST::Node *ParsePostFixExpression( int &tk );
	JavaScriptAST::Node *ParseUnaryExpression( int &tk );
	JavaScriptAST::Node *ParseMultiplicativeExpression( int &tk );
	JavaScriptAST::Node *ParseAdditiveExpression( int &tk );
	JavaScriptAST::Node *ParseShiftExpression( int &tk );
	JavaScriptAST::Node *ParseRelationalExpression( int &tk, bool noIn );
	JavaScriptAST::Node *ParseEqualityExpression( int &tk, bool noIn );
	JavaScriptAST::Node *ParseBitwiseAndExpression( int &tk, bool noIn );
	JavaScriptAST::Node *ParseBitwiseXorExpression( int &tk, bool noIn );
	JavaScriptAST::Node *ParseBitwiseOrExpression( int &tk, bool noIn );
	JavaScriptAST::Node *ParseLogicalAndExpression( int &tk, bool noIn );
	JavaScriptAST::Node *ParseLogicalOrExpression( int &tk, bool noIn );
	JavaScriptAST::Node *ParseConditionalExpression( int &tk, bool noIn );
	JavaScriptAST::Node *ParseAssignmentExpression( int &tk, bool noIn );
	JavaScriptAST::Node *ParseExpression( int &tk, bool noIn );
	JavaScriptAST::Node *ParseFunctionExpression( int &tk );

	JavaScriptAST::FunctionCallArgumentsNode *ParseArguments( int &tk, int &outArgsCount );
	JavaScriptAST::FunctionCallArgumentsNode *ParseArgumentList( int &tk, int &outArgsCount );
	JavaScriptAST::FunctionDeclarationArgumentsNode *ParseFormalParameterList( int &tk );

	bool IsStatementBlock( int tk );
	bool IsIfStatement( int tk );
	bool IsIterationStatement( int tk );
	bool IsContinueStatement( int tk );
	bool IsBreakStatement( int tk );
	bool IsDebuggerStatement( int tk );
	bool IsReturnStatement( int tk );
	bool IsWithStatement( int tk );
	bool IsSwitchStatement( int tk );
	bool IsThrowStatement( int tk );
	bool IsTryStatement( int tk );
	bool IsVariableStatement( int tk );
	bool IsFunctionStatement( int tk );

	JavaScriptAST::Node *ParseStatementBlock( int &tk );
	JavaScriptAST::Node *ParseStatementList( int &tk );
	JavaScriptAST::Node *ParseStatement( int &tk );
	JavaScriptAST::Node *ParseEmptyStatement( int &tk );
	JavaScriptAST::Node *ParseExpressionStatement( int &tk );
	JavaScriptAST::Node *ParseIfStatement( int &tk );
	JavaScriptAST::Node *ParseIterationStatement( int &tk );
	JavaScriptAST::Node *ParseForStatement( int &tk );
	JavaScriptAST::Node *ParseContinueStatement( int &tk );
	JavaScriptAST::Node *ParseBreakStatement( int &tk );
	JavaScriptAST::Node *ParseDebuggerStatement( int &tk );
	JavaScriptAST::Node *ParseReturnStatement( int &tk );
	JavaScriptAST::Node *ParseWithStatement( int &tk );
	JavaScriptAST::Node *ParseLabeledStatement( int &tk );
	JavaScriptAST::Node *ParseThrowStatement( int &tk );
	JavaScriptAST::Node *ParseTryStatement( int &tk );
	JavaScriptAST::Node *ParseCatch( int &tk );
	JavaScriptAST::Node *ParseFinally( int &tk );
	JavaScriptAST::Node *ParseVariableStatement( int &tk );
	JavaScriptAST::Node *ParseVariableDeclaration( int &tk, bool noIn );
	JavaScriptAST::Node *ParseVariableDeclarationList( int &tk, bool noIn, int *outNumberOfDecls = NULL );
	JavaScriptAST::Node *ParseFunctionDeclaration( int &tk, const VString &inFunctionName = "");
	JavaScriptAST::Node *ParseFunctionBody( int &tk );
	JavaScriptAST::Node *ParseSourceElements( int &tk );
	JavaScriptAST::Node *ParseSourceElement( int &tk );
	JavaScriptAST::Node *ParseProgram( void );

	bool ParseCaseBlock( int &tk, JavaScriptAST::SwitchStatementNode *inSwitch );
	bool ParseCaseClauses( int &tk, JavaScriptAST::SwitchStatementNode *inSwitch );
	JavaScriptAST::Node *ParseCaseClause( int &tk );
	JavaScriptAST::Node *ParseDefaultClause( int &tk );
	JavaScriptAST::Node *ParseSwitchStatement( int &tk );

public:
	JavaScriptParser();
	virtual ~JavaScriptParser();

	JavaScriptAST::Node *GetAST( VFilePath inFilePath, VString *inInput, bool& outError );
	void GetSuggestions( VString *inInput, SuggestionList *inSuggestions, Symbols::ISymbol *inContext, Symbols::IFile *inContextFile, ISymbolTable *inTable = NULL );
	void AssignDelegate( JavaScriptParserDelegate *del ) { xbox_assert( !del || !fDelegate );  fDelegate = del; }

	static XBOX::VString GetSuggestionsSeparator( ) { return JAVASCRIPT_PARSER_SUGGESTION_SEPARATOR; }
	static sLONG GetSuggestionsNumberOfColumns( ) { return JAVASCRIPT_PARSER_SUGGESTION_COLUMNS_NUMBER; }
};

#endif // _JAVASCRIPTPARSER_H_