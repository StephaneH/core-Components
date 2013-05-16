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
#include "LanguageSyntaxHeaders.h"

#include "JavaScriptLexer.h"
#include "JavaScriptParser.h"

JavaScriptParser::JavaScriptParser()
: fLexer( NULL )
, fWasError( false )
, fDelegate( NULL )
, kParsingError( -50 )
, fSuggestions( NULL )
, fCompletionSymbol( NULL )
, fCompletionSymbolBackup( NULL )
, fSymbolTable( NULL )
, fContextSymbol( NULL )
, fEntityCompletionSymbol( NULL )
, fContextFile( NULL )
{
	// Note that the JavaScriptParser class does not own the symbol table!  It
	// merely holds onto a reference that is used as the caller requires it.  
	// 
	// Also note that the parser does not own the suggestion list.  It merely stores
	// it in the same fashion as the symbol table.
	//
	// You should also note that the parser is a singleton and not meant to be used
	// by multiple threads at the same time.  However, we do own the lexer object.
	fLexer = new JavaScriptLexer();
}

JavaScriptParser::~JavaScriptParser()
 {
	if (fLexer)
		delete fLexer;
	
	SetCompletionSymbol(NULL);
	SetEntityCompletionSymbol(NULL);
	ResetCompletionSymbolList();
}

void JavaScriptParser::RecoverFromError( int &tk )
{
	// Error recovery boils down to finding the next safe statement for us to parse.  
	// JavaScript has a few boundaries at which we know we're safe.  If we find a semi-colon,
	// then we know we just finished the end of a statement (except in the case of a for loop)
	// If we find a curly brace, then we know we've found a new statement block (or a new 
	// object literal).  Our goal isn't to be perfect, since that's basically impossible given
	// the nature of the JavaScript language.  But we want to give the parser a fighting chance 
	// recover from an error so that it can keep going.
	while (tk != -1) {
		tk = fLexer->GetNextTokenForParser();

		
		if (tk == ';' || tk == '}' || tk == JavaScriptTokenValues::ENDL)
		{
			// We've found a token that designates a safe place to resume from.  So we want to
			// go one token *past* this (which we hope is the start of a new statement), and let
			// the parser resume parsing.
			tk = fLexer->GetNextTokenForParser();
			break;
		}
		else if (tk == '{')
		{
			// This is another token which designates a safe place to resume from.  However, unlike
			// the previous if block, we want to resume parsing from here without skipping past this
			// token.  That's because this is the start of what we know to be a valid production.
			break;
		}
	}
}

JavaScriptAST::Node *JavaScriptParser::GetAST( VFilePath inFilePath, VString *inInput, bool& outError )
{
	fFilePath = inFilePath;
	
	// Set up the lexer with the input we've been given
	fLexer->SetLexerInput( inInput );

	fSuggestions = NULL;
	fWasError = false;
	SetCompletionSymbol(NULL);

	// Now parse the document
	JavaScriptAST::Node *program = NULL;
	try {
		program = ParseProgram();
	} catch ( ParsingErrorException e ) {
		// This means the exception would have otherwise been unhandled, which is a bug 
		// in our code.  To prevent this exception from escaping, we handle it here, but
		// are going to throw an assertion to bark at the programmer.  You need to ensure
		// any statement or source element parsing handles exceptions!
		xbox_assert( false );
	}

	// We're also done caring about suggestions, so clear it
	fSuggestions = NULL;
	
	outError = fWasError;

	return program;
}

void JavaScriptParser::GetSuggestions( VString *inInput, SuggestionList *inSuggestions, Symbols::ISymbol *inContext, Symbols::IFile *inContextFile, ISymbolTable *inTable ){
	xbox_assert( inTable );

	// Set up the lexer with the input we've been given
	fLexer->SetLexerInput( inInput );

	// Set up the suggestion list so we can add ideas to it
	fSuggestions = inSuggestions;
	fSymbolTable = inTable;
	fSymbolTable->Retain();

	fWasError = false;
	SetCompletionSymbol(NULL);
	fContextSymbol = inContext;

	fContextFile = inContextFile;

	// Now parse the document
	try {
		JavaScriptAST::Node *program = ParseProgram();
		delete program;
	} catch ( ParsingErrorException e ) {
		// This means the exception would have otherwise been unhandled, which is a bug 
		// in our code.  To prevent this exception from escaping, we handle it here, but
		// are going to throw an assertion to bark at the programmer.  You need to ensure
		// any statement or source element parsing handles exceptions!
		xbox_assert( false );
	}
	
	fSymbolTable->Release();

	fContextFile = NULL;

	// We're also done caring about suggestions, so clear it
	fSuggestions = NULL;
}

bool JavaScriptParser::ParseStatementCompletion( int &tk )
{
	// One the statement has completed, we need to reset our
	// completion token
	SetCompletionSymbol(NULL);

	if (';' == tk) {
		tk = fLexer->GetNextTokenForParser();
		return true;
	}

	// We can also support auto-matic semi-colon insertion here by following the specific
	// rules laid out in section 7.9 of the ECMAScript specification.

	// If the current token is a }, then we're allowed to put a semi-colon here
	if ('}' == tk)	return true;

	// If we've hit the end of input, we can insert one as well
	if (-1 == tk)	return true;

	// One or more line terminators preceeding this token, so we can insert the semi-colon
	if (fLexer->ConsumedNewlineBeforeToken())	return true;

	ThrowError( "Syntax Error" );

	return false;
}

JavaScriptAST::Node *JavaScriptParser::ParseFunctionDeclaration( int &tk, const VString &inFunctionName)
{
	JavaScriptAST::Node *ret = NULL;

	// Function name can be given as argument when an anonym function 
	// with "foo:function(){}" syntax is parsed or to override
	// the function identifier with the property name value like in
	// "foo:function bar(){}" syntax
	VString functionName(inFunctionName);

	if (JavaScriptTokenValues::FUNCTION == tk)
	{
		int beginLineNumber = fLexer->GetCurrentLineNumber();
		tk = fLexer->GetNextTokenForParser();

		// No function identifier has been parsed. If no name parameter was given
		// for the current function we assume there is a syntax error...
		if (JavaScriptTokenValues::IDENTIFIER != tk && inFunctionName == "") {
			ThrowError( "Expected Identifier" );
			return NULL;
		}

		// Store the identifier text off for later, since we need to validate more of the
		// function declaration.
		if (JavaScriptTokenValues::IDENTIFIER == tk)
		{
			// We found a function identifier but want to use it only
			// if no function name parameter has been given
			if (functionName == "")
				functionName = fLexer->GetTokenText();
			tk = fLexer->GetNextTokenForParser();
		}

		// After the identifier comes the parameter list, enclosed in parenthesis.
		if ('(' != tk) {
			ThrowError( "Syntax Error" );
			return NULL;
		}

		// Now comes the formal parameter list, which is just a comma separated list of 
		// zero or more identifiers
		JavaScriptAST::FunctionDeclarationArgumentsNode *args = ParseFormalParameterList( tk );
		if (!args) {
			return NULL;
		}

		// Now we expect the token to be pointing to the closing paren
		if (')' != tk)
		{
			delete args;
			ThrowError( "Syntax Error" );
			return NULL;
		}
		tk = fLexer->GetNextTokenForParser();

		// At this point, there should be an opening curly brace, which means we can start
		// parsing statements again, looking for more functions and variables to handle.
		if ('{' != tk)
		{
			delete args;
			ThrowError( "Syntax Error" );
			return NULL;
		}

		// Before we parse the body, let's pull any ScriptDoc comments out of the lexer so that we
		// can store them on the return node.
		ScriptDocComment *comment = NULL;
		if (IsScriptDocElement()) {
			comment = ParseScriptDocElement();
		}

		tk = fLexer->GetNextTokenForParser();

		JavaScriptAST::Node *body = NULL;
		try
		{
			if ('}' != tk)
			{
				if ( ! (body = ParseFunctionBody( tk ) ) )
				{
					delete args;
					return NULL;
				}
			}
		}
		catch (ParsingErrorException err)
		{
			if (err == kParsingError)
			{
				delete args;
				delete body;
				throw;
			}
		}

		if ('}' != tk)
		{
			delete args;
			delete body;
			ThrowError( "Syntax Error" );
			return NULL;
		}

		int endLineNumber = fLexer->GetCurrentLineNumber();

		tk = fLexer->GetNextTokenForParser();

		ret = new JavaScriptAST::FunctionDeclarationStatementNode( functionName, args, body, beginLineNumber, endLineNumber );
		ret->AttachScriptDocComment( comment );
	}

	return ret;
}

JavaScriptAST::FunctionDeclarationArgumentsNode *JavaScriptParser::ParseFormalParameterList( int &tk )
{
	JavaScriptAST::FunctionDeclarationArgumentsNode *ret = new JavaScriptAST::FunctionDeclarationArgumentsNode( fLexer->GetCurrentLineNumber() );
	do {
		tk = fLexer->GetNextTokenForParser();

		// It is fine if tk isn't an identifier, since it could also be a closing
		// paren.  We will check that at the end of the function though
		if (JavaScriptTokenValues::IDENTIFIER != tk)	break;

		ret->AddArgument( new JavaScriptAST::IdentifierNode( fLexer->GetTokenText(), fLexer->GetCurrentLineNumber() ) );
		
		// Get the next token so we can test to see whether it's a , or a )
		tk = fLexer->GetNextTokenForParser();
	} while (',' == tk);

	// The reason we left the parameter list can only be because of a closing paren
	if (')' == tk)	return ret;
	delete ret;
	return NULL;
}

JavaScriptAST::Node *JavaScriptParser::ParseVariableStatement( int &tk )
{
	// We've been given an explicit variable declaration.  The exact syntax is described in
	// section 12.2 of the ECMAScript specifiction.  Basically, it's: var declarationList;
	// The caller has already parsed the var clause, so now it's up to us to parse the declaration list.
	// As we parse declarations, we need to add them to the parent symbol.  If the parent symbol
	// is NULL, then they belong as a top-level declaration directly in the symbol table.
	//
	// A declarationList is a list of one or more comma-separated declaration clauses.  Each 
	// declaration is an Identifier, optionally followed by an initializer clause.  We're going
	// to ignore the initializer for now, though a future iteration may wish to gather type information
	// from the initializer if possible.
	//
	// So the simple form of this is to get an identifier, and if there's anything past it aside from a
	// comma or semi-colon, ignore that stuff.  A comma means the process will start over.
	if (JavaScriptTokenValues::VAR != tk)	return NULL;

	// Before we parse the body, let's pull any ScriptDoc comments out of the lexer so that we
	// can store them on the return node.
	ScriptDocComment *comment = NULL;
	if (IsScriptDocElement()) {
		comment = ParseScriptDocElement();
	}

	tk = fLexer->GetNextTokenForParser();
	JavaScriptAST::Node *ret = ParseVariableDeclarationList( tk, false );
	if (!ret)	return NULL;

	// Attach WAF comment if we are in a namespace declaration
	bool bStart; VString regionName; int line; 

	fLexer->GetRegionInformation(bStart, regionName, line);
	if ( regionName.Find("namespaceDeclaration") )
	{
		TokenList ioTokens;

		fLexer->ConsumeWhiteSpaces(&ioTokens);
		if ( ioTokens.size() > 1
			&& ILexerToken::TT_WHITESPACE == ioTokens[0]->GetType() 
			&& ILexerToken::TT_COMMENT == ioTokens[1]->GetType() )
			ret->AttachWAFComment( ioTokens[1]->GetText() );
	}

	ret->AttachScriptDocComment( comment );
	if (!ParseStatementCompletion( tk )) {
		delete ret;
		return NULL;
	}

	return ret;
}

JavaScriptAST::Node *JavaScriptParser::ParseVariableDeclarationList( int &tk, bool noIn, int *outNumberOfDecls )
{
	JavaScriptAST::Node *ret = ParseVariableDeclaration( tk, noIn );
	JavaScriptAST::VariableDeclarationListStatementNode *list = NULL;
	if (!ret)	return NULL;

	bool done = true;
	do {
		if (outNumberOfDecls)	(*outNumberOfDecls)++;
		switch (tk) {
			case ',': {
				if (!list) {
					list = new JavaScriptAST::VariableDeclarationListStatementNode( static_cast< JavaScriptAST::VariableDeclarationStatementNode * >( ret ), fLexer->GetCurrentLineNumber() );
					ret = list;
				}
				tk = fLexer->GetNextTokenForParser();
				JavaScriptAST::Node *varDecl = ParseVariableDeclaration( tk, noIn );
				if (!varDecl) {
					delete ret;
					return NULL;
				} else {
					list->AddDeclaration( static_cast< JavaScriptAST::VariableDeclarationStatementNode * >( varDecl ) );
				}
				done = false;
			} break;
			default:	done = true;
		}
	} while (!done);
	
	return ret;
}

JavaScriptAST::Node *JavaScriptParser::ParseVariableDeclaration( int &tk, bool noIn )
{
	// The first thing we expect to see is an identifier
	if (JavaScriptTokenValues::IDENTIFIER != tk) {
		ThrowError( "Expected Identifier" );
		return NULL;
	}

	VString tkText =  fLexer->GetTokenText();
	int currentLineNumber = fLexer->GetCurrentLineNumber();
	JavaScriptAST::IdentifierNode *ident = new JavaScriptAST::IdentifierNode( tkText, currentLineNumber );

	// Optionally, there may be an assignment
	tk = fLexer->GetNextTokenForParser();
	JavaScriptAST::Node *assign = NULL;
	if ('=' == tk) {
		// This means we have an assignment expression
		tk = fLexer->GetNextTokenForParser();

		try {
			// Modif SLA
			// If we found an anonym function declaration, we want to return a function
			// named with the variable identifier. A little bit kludge but usefull.
			// Manage syntax like : var foo = function (arg1, arg2) { var bar = "sometext"; };
			if (JavaScriptTokenValues::FUNCTION == tk)
				if (assign = ParseFunctionDeclaration(tk, tkText))
					return assign;
			// EOModif SLA

			if (!(assign = ParseAssignmentExpression( tk, noIn ))) {
				delete ident;
				return NULL;
			}
		} catch (ParsingErrorException err) {
			if (err == kParsingError) {
				delete ident;
				throw;
			}
		}
	}

	return new JavaScriptAST::VariableDeclarationStatementNode( ident, assign, ident->GetLineNumber() );
}

JavaScriptAST::Node *JavaScriptParser::ParseLiteral( int &tk )
{
	JavaScriptAST::LiteralNode* ret = NULL;
	Symbols::ISymbol* coreSymbol = NULL;			

	switch (tk) {
		case JavaScriptTokenValues::KWORD_NULL:	{
			ret = new JavaScriptAST::NullLiteralNode( fLexer->GetCurrentLineNumber() );
			tk = fLexer->GetNextTokenForParser();
		} break;
		case JavaScriptTokenValues::KWORD_TRUE: {
			ret = new JavaScriptAST::BooleanLiteralNode( true, fLexer->GetCurrentLineNumber() );
			tk = fLexer->GetNextTokenForParser();
			coreSymbol = GetCoreSymbolByName("Boolean");
			
		} break;
		case JavaScriptTokenValues::KWORD_FALSE: {
			ret = new JavaScriptAST::BooleanLiteralNode( false, fLexer->GetCurrentLineNumber() );
			tk = fLexer->GetNextTokenForParser();
			coreSymbol = GetCoreSymbolByName("Boolean");
		} break;
		case JavaScriptTokenValues::STRING_APOSTROPHE:
		case JavaScriptTokenValues::STRING_QUOTATION_MARK: {
			ret = new JavaScriptAST::StringLiteralNode( fLexer->GetTokenText(), fLexer->GetCurrentLineNumber() );
			tk = fLexer->GetNextTokenForParser();
			coreSymbol = GetCoreSymbolByName("String");
		} break;
		case JavaScriptTokenValues::NUMBER:	{
			ret = new JavaScriptAST::NumericLiteralNode( fLexer->GetTokenText(), fLexer->GetCurrentLineNumber() );
			tk = fLexer->GetNextTokenForParser();
			coreSymbol = GetCoreSymbolByName("Number");
		} break;
		case JavaScriptTokenValues::REGEXP: {
			VString	body, flags; // Aren't used for now
            
            VString szRegExpValue = fLexer->GetTokenText();

			ret = new JavaScriptAST::RegExLiteralNode( body, flags, fLexer->GetCurrentLineNumber() );
			tk = fLexer->GetNextTokenForParser();
			coreSymbol = GetCoreSymbolByName("RegExp");
		} break;
		case -1: {
			// In this case, we're at the end of input, so see if we can make
			// some suggestions
			if (fSuggestions) {
				fSuggestions->Suggest( "null", SuggestionInfo::eKeyword );
				fSuggestions->Suggest( "true", SuggestionInfo::eKeyword );
				fSuggestions->Suggest( "false", SuggestionInfo::eKeyword );
			}
		} break;
	}

	if (NULL != coreSymbol )
	{
		SetCompletionSymbol( coreSymbol );
		coreSymbol->Release();
	}

	return ret;
}

void JavaScriptParser::ParseElision( int &tk )
{
	while (',' == tk) {
		tk = fLexer->GetNextTokenForParser();
	}
}

bool JavaScriptParser::ParseElementList( int &tk )
{
	do {
		ParseElision( tk );
		JavaScriptAST::Node *expr = ParseAssignmentExpression( tk, false );
		if (!expr)	return false;
		delete expr;
	} while (',' == tk);
	return true;
}

JavaScriptAST::Node * JavaScriptParser::ParseArrayLiteral( int &tk )
{
	if ('[' != tk)	return NULL;
	int lineNumber = fLexer->GetCurrentLineNumber();
	tk = fLexer->GetNextTokenForParser();

	ParseElision( tk );

	ParseElementList( tk );

	ParseElision( tk );

	if (']' != tk) {
		ThrowError( "Syntax Error" );
		return NULL;
	}

	tk = fLexer->GetNextTokenForParser();

	// Find the Array object and set our completion symbol as appropriate
	Symbols::ISymbol* coreSym = GetCoreSymbolByName("Array");
	if ( NULL != coreSym )
	{
		SetCompletionSymbol( coreSym );
		coreSym->Release();
	}

	return new JavaScriptAST::ArrayLiteralNode( lineNumber );
}

JavaScriptAST::Node *JavaScriptParser::ParsePropertyName( int &tk )
{
	VString tokenText = fLexer->GetTokenText();
	int lineNumber = fLexer->GetCurrentLineNumber();
	switch (tk) {
		case JavaScriptTokenValues::IDENTIFIER: {
			tk = fLexer->GetNextTokenForParser();
			return new JavaScriptAST::IdentifierNode( tokenText, lineNumber );
		} break;
		case JavaScriptTokenValues::STRING_APOSTROPHE:
		case JavaScriptTokenValues::STRING_QUOTATION_MARK: {
			tk = fLexer->GetNextTokenForParser();
			return new JavaScriptAST::StringLiteralNode( tokenText, lineNumber );
		} break;
		case JavaScriptTokenValues::NUMBER: {
			tk = fLexer->GetNextTokenForParser();
			return new JavaScriptAST::NumericLiteralNode( fLexer->GetTokenText(), lineNumber );
		} break;
	}

	ThrowError( "Syntax Error" );
	return NULL;
}

bool JavaScriptParser::ParsePropertyNameAndValueList( int &tk, JavaScriptAST::ObjectLiteralNode *inNode )
{
	do {
		bool isFunction = false;
		ScriptDocComment *comment = NULL;
		if (IsScriptDocElement()) {
			comment = ParseScriptDocElement();
		}

		// We expect a property name, followed by a colon, and a property value
		VString tokenText = fLexer->GetTokenText();
		int lineNumber = fLexer->GetCurrentLineNumber();
		JavaScriptAST::Node *ident = ParsePropertyName( tk );
		if (!ident)	return false;

		if (':' != tk)
		{
			delete ident;
			ThrowError( "Syntax Error" );
			return false;
		}

		tk = fLexer->GetNextTokenForParser();

		JavaScriptAST::Node *expr;

		// Property name is parsed and we know that a colon is following.
		// We have to checked if the currect token is JavaScriptTokenValues::FUNCTION
		// to handle the "property:[name]function(){}" syntax.
		if (JavaScriptTokenValues::FUNCTION == tk)
		{
			//int nextTk = fLexer->PeekAtNextTokenForParser();
			//if ('(' == nextTk || JavaScriptTokenValues::IDENTIFIER == nextTk)
			//{
				isFunction = true;
				expr = ParseFunctionDeclaration(tk, tokenText);
			//}
		}
		else
		{
			expr = ParseAssignmentExpression( tk, false );
			if (fSuggestions)
				fSuggestions->SuggestJavaScriptIdentifiers();
		}

		if (comment) {
			// Now that we have a ScriptDoc comment and we've parsed the right-hand side, we know where
			// we'd like to attach that information.  If we have a function expression, then it needs
			// the comment.  Otherwise, the identifier can deal with it.
			if (dynamic_cast< JavaScriptAST::FunctionExpressionNode * >( expr ) || dynamic_cast< JavaScriptAST::FunctionDeclarationStatementNode * >( expr )) {
				expr->AttachScriptDocComment( comment );
			} else {
				ident->AttachScriptDocComment( comment );
			}
		}

		if (!expr)
		{
				delete ident;
				return false;
		}

		// literal is a function name with "name:function(){}" syntax.
		if (isFunction)
		{
			isFunction = false;
			inNode->AddField( (JavaScriptAST::ObjectLiteralFieldNode *)expr );
		}
		else
			inNode->AddField( new JavaScriptAST::ObjectLiteralFieldNode( ident, expr, lineNumber ) );

		// If we have a comma, then we need to grab the next token and start over
		if (',' == tk) {
			tk = fLexer->GetNextTokenForParser();
		} else {
			break;
		}
	} while (true);

	return true;
}

JavaScriptAST::Node *JavaScriptParser::ParseObjectLiteral( int &tk )
{
	if ('{' != tk)	return NULL;

	// An object literal can either be an empty pair of brackets, or it can 
	// have a list of property names and values, followed by a closing bracket.  So
	// grab the next token to see what's up
	JavaScriptAST::ObjectLiteralNode *node = new JavaScriptAST::ObjectLiteralNode( fLexer->GetCurrentLineNumber() );
	tk = fLexer->GetNextTokenForParser();
	if ('}' != tk) {
		try {
			if (!ParsePropertyNameAndValueList( tk, node )) {
				delete node;
				return NULL;
			}
		} catch (ParsingErrorException err) {
			if (err == kParsingError) {
				delete node;
				throw;
			}
		}
	}
	// {} expression is used. We search and assign the global "Object" symbol to completion symbol
	else
	{
		Symbols::ISymbol* coreSym = GetCoreSymbolByName("Object");
		if ( NULL != coreSym )
		{
			SetCompletionSymbol( coreSym );
			coreSym->Release();
		}
	}

	if ('}' != tk) {
		delete node;
		ThrowError( "Syntax Error" );
		return NULL;
	}

	tk = fLexer->GetNextTokenForParser();

	return node;
}

JavaScriptAST::Node *JavaScriptParser::ParsePrimaryExpression( int &tk )
{
	int lineNumber = fLexer->GetCurrentLineNumber();
	if (JavaScriptTokenValues::KWORD_THIS == tk)
	{
		tk = fLexer->GetNextTokenForParser();
		// The "this" keyword represents a special symbol for the current
		// function in scope.  However, we want to return a symbol whose prototype
		// points to the same prototype as the function.  This way, we can treat
		// this as a local instance variable
		SetCompletionSymbol( NULL );

		if (fContextSymbol && fSymbolTable)
			SetCompletionSymbol( fContextSymbol );
		
		return new JavaScriptAST::ThisNode( lineNumber );
	}

	if (JavaScriptTokenValues::IDENTIFIER == tk) {
		VString ident = fLexer->GetTokenText();
		if (fSymbolTable) {
			// Search the context object for the name.  If there is no context symbol, 
			// this will search the global symbol table.
			Symbols::ISymbol* contextSym = UnqualifiedLookup( ident );
			if ( NULL != contextSym )
			{
				SetCompletionSymbol( contextSym );
				contextSym->Release();
			}
			
			if ( fCompletionSymbol && ! CheckCompatibleFileContext(fContextFile, fCompletionSymbol->GetFile() ) )
				SetCompletionSymbol( NULL );
		}
		tk = fLexer->GetNextTokenForParser();

		// Modif SLA
		// Parse anonym function
		/*int nextTk = fLexer->PeekAtNextTokenForParser();
		if (JavaScriptTokenValues::FUNCTION == nextTk) {
			tk = fLexer->GetNextTokenForParser();
			return ParseFunctionDeclaration(tk, ident);
		}
		else*/
		// EOModif SLA
			return new JavaScriptAST::IdentifierNode( ident, lineNumber );
	}

	JavaScriptAST::Node *ret = NULL;
	if (ret = ParseLiteral( tk ))						return ret;
	if (ret = ParseArrayLiteral( tk ))					return ret;
	if (ret = ParseObjectLiteral( tk ))					return ret;
	if ('(' == tk) {
		tk = fLexer->GetNextTokenForParser();
		JavaScriptAST::Node *ret = ParseExpression( tk, false );
		if (!ret)	return NULL;
		if (')' != tk) {
			delete ret;
			ThrowError( "Syntax Error" );
			return NULL;
		}
		tk = fLexer->GetNextTokenForParser();
		return ret;
	}

	// If we are staring at the end of the token stream, then we
	// can make a few suggestions for the user.
	if (fSuggestions && -1 == tk) {
		fSuggestions->Suggest( "this", SuggestionInfo::eKeyword );
		fSuggestions->SuggestJavaScriptIdentifiers();
		// If we're currently in the process of parsing a method body, we want the magical
		// "arguments" variable to be available.
		if (fContextSymbol != NULL) {
			fSuggestions->Suggest( "arguments", SuggestionInfo::eName );
		}
	}

	// It's acceptable to be none of the above without actually
	// being an error.  We've handled actual error cases above
	// already.
	return NULL;
}

JavaScriptAST::Node *JavaScriptParser::ParseFunctionBody( int &tk )
{
	return ParseSourceElements( tk );
}

JavaScriptAST::Node *JavaScriptParser::ParseFunctionExpression( int &tk )
{
	if (JavaScriptTokenValues::FUNCTION != tk)
		return NULL;

	int beginLineNumber = fLexer->GetCurrentLineNumber();

	// There is an optional identifier, or the start of the formal parameter list
	tk = fLexer->GetNextTokenForParser();
	VString ident;
	if (JavaScriptTokenValues::IDENTIFIER == tk)
	{
		ident = fLexer->GetTokenText();
		// Eat the identifier for now
		tk = fLexer->GetNextTokenForParser();
	}

	// Now we expect an open paren, but if we have it, we don't have to grab the
	// next token since ParseFormalParameterList will do that for us
	if ('(' != tk)
	{
		ThrowError( "Syntax Error" );
		return NULL;
	}

	JavaScriptAST::FunctionDeclarationArgumentsNode *args = ParseFormalParameterList( tk );
	if (!args)	return NULL;

	// The next token needs to be the closing paren
	if (')' != tk)
	{
		delete args;
		ThrowError( "Syntax Error" );
		return NULL;
	}
	tk = fLexer->GetNextTokenForParser();

	if ('{' != tk)
	{
		delete args;
		ThrowError( "Syntax Error" );
		return NULL;
	}

	tk = fLexer->GetNextTokenForParser();

	JavaScriptAST::Node *body = NULL;
	if ('}' != tk)
	{
		body = ParseFunctionBody( tk );
		if (!body)
		{
			delete args;
			return NULL;
		}
	}

	if ('}' != tk)
	{
		delete args;
		delete body;
		ThrowError( "Syntax Error" );
		return NULL;
	}

	
	int endLineNumber = fLexer->GetCurrentLineNumber();

	tk = fLexer->GetNextTokenForParser();

	return new JavaScriptAST::FunctionExpressionNode( ident, args, body, beginLineNumber, endLineNumber );
}

JavaScriptAST::FunctionCallArgumentsNode *JavaScriptParser::ParseArguments( int &tk, int &outArgsCount )
{
	// We expect to see an open paren
	if ('(' != tk) {
		ThrowError( "Syntax Error" );
		return NULL;
	}

	JavaScriptAST::FunctionCallArgumentsNode *ret = ParseArgumentList( tk, outArgsCount );

	if (!ret)
		return NULL;

	if (')' != tk)
	{
		delete ret;
		ThrowError( "Syntax Error" );
		return NULL;
	}
	tk = fLexer->GetNextTokenForParser();
	
	return ret;
}

JavaScriptAST::FunctionCallArgumentsNode *JavaScriptParser::ParseArgumentList( int &tk, int &outArgsCount )
{
	outArgsCount = 0;

	JavaScriptAST::FunctionCallArgumentsNode *ret = new JavaScriptAST::FunctionCallArgumentsNode( fLexer->GetCurrentLineNumber() );
	do {
		tk = fLexer->GetNextTokenForParser();

		// No need to parse assignment expression node if current token is "-1"
		// because it will result in an incorrect suggestion list
		if( -1 != tk )
		{
			// Check to see if we have an assignment expression
			JavaScriptAST::Node *node = ParseAssignmentExpression( tk, false );
			if (!node)	break;
			ret->AddArgument( node );
		}
		else
			break;
	} while (',' == tk);

	if (NULL != ret)
		outArgsCount = ret->ArgumentCount();

	// The reason we left the parameter list can only be because of a closing paren
	if (')' == tk) {
		return ret;
	} else {
		delete ret;
		return NULL;
	}
}

Symbols::ISymbol *JavaScriptParser::GetCoreSymbolByName( const VString &inName )
{
	Symbols::ISymbol *coreSym = NULL;

	if (!fSymbolTable)	return NULL;

	std::vector< Symbols::ISymbol * > syms = fSymbolTable->GetSymbolsByName(0, inName);
	if ( ! syms.empty())
	{
		coreSym = syms.front();
		coreSym->Retain();

		for (std::vector< Symbols::ISymbol * >::iterator iter = syms.begin(); iter != syms.end(); ++iter)
			(*iter)->Release();
	}

	return coreSym;
}

Symbols::ISymbol *JavaScriptParser::UnqualifiedLookup( const VString &inName )
{
	xbox_assert( fSymbolTable );
	
	// If we're currently parsing a method body, we want to support the magical "arguments" symbol.
	// But we're going to have to mock-up a fake symbol to represent it
	if (fContextSymbol != NULL && inName.EqualTo( CVSTR( "arguments" ), true )) {
		// We have an "arguments" symbol, so let's find it in our symbol table.  It's a truly odd symbol
		// to locate too, since it's part of an anonymous function expression in JS Core.  It's an object
		// literal assigned to a local variable.  So we want to find all of the nameless function expressions
		// in the file, and find one with a local variable named arguments.  Ew.
		static sLONG sArgsID = -1;

		Symbols::ISymbol *ret = NULL;
		if (sArgsID == -1) {
			std::vector< Symbols::IFile * > files = fSymbolTable->GetFilesByName( CVSTR( "JSCore.js" ) );
			if (!files.empty()) {
				for (std::vector< Symbols::IFile * >::iterator file = files.begin(); !ret && file != files.end(); ++file) {
					std::vector< Symbols::ISymbol * > syms = fSymbolTable->GetSymbolsByName( NULL, CVSTR( "" ), *file );

					for (std::vector< Symbols::ISymbol * >::iterator iter = syms.begin(); !ret && iter != syms.end(); ++iter) {
						// Look at this nameless symbol's children to see if we can find a local variable named "arguments"
						std::vector< Symbols::ISymbol * > children = fSymbolTable->GetSymbolsByName( *iter, CVSTR( "arguments" ) );
						if (!children.empty()) {
							ret = children.front();
							sArgsID = ret->GetID();
							ret->Retain();
							for (std::vector< Symbols::ISymbol * >::iterator iter2 = children.begin(); iter2 != children.end(); ++iter2) (*iter2)->Release();
						}
					}
					for (std::vector< Symbols::ISymbol * >::iterator iter = syms.begin(); iter != syms.end(); ++iter)	(*iter)->Release();
				}
				for (std::vector< Symbols::IFile * >::iterator iter = files.begin(); iter != files.end(); ++iter)	(*iter)->Release();
			}
		} else {
			ret = fSymbolTable->GetSymbolByID( sArgsID );
		}
		return ret;
	}

	// We want to loop our way up the context symbol hierarchy until we reach the root symbol table.  We
	// start from the given context object and walk up the Owner list.  When we encounter the first symbol
	// whose name matches the given identifier, we will bail out
	Symbols::ISymbol *search = fContextSymbol;
	while (true) {
		// Get the matching symbols from the given context
		std::vector< Symbols::ISymbol * > syms = fSymbolTable->GetSymbolsByName( search, inName );
		
		// If we found a symbol that matches, retain it so we can return it to the caller, but only if it
		// is one of the local variables.
		Symbols::ISymbol *ret = NULL;
		for (std::vector< Symbols::ISymbol * >::iterator iter = syms.begin(); iter != syms.end(); ++iter) {
			int kind = (*iter)->GetKind();
			if (kind == Symbols::ISymbol::kKindFunctionParameterDeclaration ||
				kind == Symbols::ISymbol::kKindLocalVariableDeclaration     ||
				kind == Symbols::ISymbol::kKindCatchBlock                   ||
				( (*iter)->IsFunctionKind() && kind != Symbols::ISymbol::kKindClassConstructor ) ||
				(*iter)->IsEntityModelKind() )
			{

				// It is possible that we've actually already saved a symbol in our return slot.  However, we
				// want to give preference to symbols that live in the same file as the one being parsed.  So
				// check to see if that's the case here.  If our new symbol is in the same file as the one being
				// parsed, we'll use it instead.
				if (!ret) {
					// We don't have a return symbol yet, so we just want to keep this one regardless
					ret = *iter;
				} else {
					// We already have a symbol stored here, so let's see if this one is better.  The only way it can
					// be better is if we have a context file, and if the context file's path matches the iterator's
					// file's path.
					if (fContextFile) {
						bool swapSymbol = false;

						VString contextFilePath = fContextFile->GetPath();
						VString symbolFilePath = ((*iter)->GetFile()) ? (*iter)->GetFile()->GetPath() : CVSTR( "" );
						VString oldSymbolFilePath = (ret->GetFile()) ? ret->GetFile()->GetPath() : CVSTR( "" );

						if (!oldSymbolFilePath.EqualTo( contextFilePath, true ))
						{
							// The old symbol was in a different file from the context file
							if (symbolFilePath.EqualTo( contextFilePath, true ))
								swapSymbol = true;
							// Test if the new symbol execution context is more appropriate than the old one
							else
							{
								ESymbolFileExecContext execContext = fContextFile->GetExecutionContext();
								ESymbolFileExecContext symbolExecContext = ((*iter)->GetFile()) ? (*iter)->GetFile()->GetExecutionContext() : 0;
								ESymbolFileExecContext oldSymbolExecContext = (ret->GetFile()) ? ret->GetFile()->GetExecutionContext() : 0;

								if (oldSymbolExecContext != execContext && oldSymbolExecContext != eSymbolFileExecContextClientServer)
									if (symbolExecContext == execContext || symbolExecContext == eSymbolFileExecContextClientServer)
										swapSymbol = true;
							}
						}

						if (swapSymbol)
						{
							// The new symbol is more appropriate, so we found a replacement!
							ret->Release();
							ret = *iter;
						}
					}
				}
			} else {
				(*iter)->Release();
			}
		}

		// If we found a symbol from this pass, return it to the caller
		if (ret)
		{
			Symbols::ISymbol* symRef = const_cast< Symbols::ISymbol * >( ret->RetainReferencedSymbol() );
			ret->Release();
			return symRef;
		}

		// If we just finished searching the global context, then we're done trying
		if (!search)	break;

		// Otherwise, move on to the next owner
		search = search->GetOwner();
	}

	return NULL;
}

Symbols::ISymbol *JavaScriptParser::QualifiedLookup( Symbols::ISymbol *inSym, const VString &inMember )
{
	if (!inSym)	return NULL;

	Symbols::ISymbol *ret = NULL;
	std::vector< Symbols::ISymbol * > lookup_list;
	lookup_list.push_back( const_cast< Symbols::ISymbol * >( inSym->RetainReferencedSymbol() ) );
	Symbols::ISymbol *temp = lookup_list.back();
	while (!lookup_list.empty()) {
		Symbols::ISymbol *lookup = lookup_list.back();
		lookup_list.pop_back();

		// See if we can find the symbol with the proper name
		std::vector< Symbols::ISymbol * > syms = fSymbolTable->GetSymbolsByName( lookup, inMember );

		for (std::vector< Symbols::ISymbol * >::iterator iter = syms.begin(); iter != syms.end(); ++iter)
		{
			// We only care about public properties, since this is a qualified lookup
			if (!ret && ( (*iter)->IsPublicMethodKind() || (*iter)->IsPublicPropertyKind() ) )
				ret = const_cast< Symbols::ISymbol * >( (*iter)->RetainReferencedSymbol() );
			(*iter)->Release();
		}

		// If we already found a member, we're done
		if (ret)
			break;
		
		// Otherwise, move down the prototype chain
		std::vector< Symbols::ISymbol * > downPrototypes = fSymbolTable->GetSymbolsByName( lookup, CVSTR("prototype") );
		if ( ! downPrototypes.empty() )
			lookup_list.insert( lookup_list.begin(), downPrototypes.begin(), downPrototypes.end() );

		// Otherwise, move up the prototype chain
		lookup_list.insert( lookup_list.begin(), lookup->GetPrototypes().begin(), lookup->GetPrototypes().end() );
	}
	temp->Release();

	return ret;
}


void JavaScriptParser::SuggestAppropriateIdentifiers( bool inIsFunctionCall )
{
	if (!fSuggestions)
		return;

	if ( fCompletionSymbolList.empty() )
	{
		// Without a completion symbol, we just want the symbol table to suggest anything
		// from the base suggestion list.
		fSuggestions->SuggestJavaScriptIdentifiers();
		return;
	}

	// If we have a function call, then we want to perform the lookup on all possible return types.
	std::vector< Symbols::ISymbol * >	lookup_list, returnTypes;
	sLONG entityCompletionType = EntityModelCompletionType::kNone;

	if (inIsFunctionCall)
	{
		returnTypes = GetSuggestionReturnTypes(entityCompletionType);
		lookup_list = returnTypes;
	}
	else
		lookup_list = fCompletionSymbolList;

	std::map<VString, VectorOfVString>	suggestionList;
	std::map<sLONG, bool>				mapOfSymsID;
	VString								className, mainClassName;

	while ( ! lookup_list.empty() )
	{
		// Find all of the symbols associated with the lookup symbol
		Symbols::ISymbol *lookupSym = lookup_list.back();
		Symbols::ISymbol *refLookupSym = const_cast< Symbols::ISymbol * >( lookupSym->RetainReferencedSymbol() );

		bool	isaClass = lookupSym->IsaClass();
		VString	currentClassName = lookupSym->GetClass();

		// Calculate current suggestions class name: if lookup symbol belongs to a class it will be easy to
		// travel its ownership stack until the class and get its name. If lookup symbol is a symbol
		// referencing an object literal save the symbol name because we won't be able to travel back to it
		// and get a valid name when we will explore anonym object prototype symbols list. Main class is
		// supposed to be the first one we visit.
		if ( currentClassName.GetLength() )
			className = currentClassName;
		if ( mainClassName.IsEmpty() && ! className.IsEmpty() )
			mainClassName = className;

		lookup_list.pop_back();

		if (refLookupSym)
		{
			if ( mapOfSymsID[ refLookupSym->GetID() ] != true )
			{
				mapOfSymsID[ refLookupSym->GetID() ] = true;
				
				// First, move down to lookup sym children
				std::vector< Symbols::ISymbol * > subSyms = fSymbolTable->GetNamedSubSymbols( refLookupSym );
				for (std::vector< Symbols::ISymbol * >::iterator itSub = subSyms.begin(); itSub != subSyms.end(); ++itSub)
				{
					// Suggest any public properties the lookup symbol owns (kludge: suggest also function declaration
					// as they are not stored as public properties)
					if ( (*itSub)->IsPublicKind() )
					{	
						// For a Class completion (for example "String.") we don't want to display anything else 
						// than static properties and methods and prototype
						if ( isaClass &&  ( ! (*itSub)->IsStaticKind() && (*itSub)->GetName() != CVSTR("prototype") ) )
							continue;

						// Skip Collection methods if we calculate completion list on an Entity. Also skip property
						// tha aren't method
						if ( entityCompletionType == EntityModelCompletionType::kCollection)
							if ( ! (*itSub)->IsFunctionKind() || (*itSub)->GetAuxillaryKindInformation() & Symbols::ISymbol::kKindEntityModelMethodEntity )		 
							continue;

						// Skip Entity methods if we calculate completion list on an Collection
						if ( (*itSub)->GetAuxillaryKindInformation() & Symbols::ISymbol::kKindEntityModelMethodEntityCollection && 
							entityCompletionType == EntityModelCompletionType::kEntity)
							continue;

						VString displayText = (*itSub)->GetName();
						Symbols::ISymbol *functionSym = (*itSub)->IsFunctionKind() ? *itSub : fSymbolTable->GetAssignedFunction(*itSub);
						//fSymbolTable->GetSymbolSignature( (NULL != functionSym) ? functionSym : *itSub, displayText);
						fSymbolTable->GetSymbolSignature( *itSub, displayText);

						// Skip suggestion if symbol or assigned function expression is marked as
						// deprecated in related JSDoc
						Symbols::ISymbol *scriptDocSym = (NULL != functionSym) ? functionSym : *itSub;
						if ( ! scriptDocSym->IsDeprecated() && ! scriptDocSym->IsPrivate() )
						{	
							displayText += JAVASCRIPT_PARSER_SUGGESTION_SEPARATOR + className + JAVASCRIPT_PARSER_SUGGESTION_SEPARATOR + (*itSub)->GetName();
							suggestionList[className].insert(suggestionList[className].begin(), displayText);
			
							if ((*itSub)->GetName() == "prototype" && ! isaClass)
							{
								(*itSub)->Retain();
								lookup_list.insert( lookup_list.begin(), *itSub);
							}
						}
					}
					(*itSub)->Release();
				}

				// Then move up the prototype chain enlarging search to all prototypes owner 
				// with the same name. Usefull to find classes prototype extension for example.
				std::vector< Symbols::ISymbol * >::const_iterator itRefLookupSymProto;
				for (itRefLookupSymProto = refLookupSym->GetPrototypes().begin(); itRefLookupSymProto != refLookupSym->GetPrototypes().end(); ++itRefLookupSymProto)
				{
					Symbols::ISymbol *owner = (*itRefLookupSymProto)->GetOwner();
					if ( NULL != owner && ! owner->GetName().IsEmpty() )
					{
						std::vector< Symbols::ISymbol * > protoOwners = fSymbolTable->GetSymbolsByName(NULL, owner->GetName());
						for (std::vector< Symbols::ISymbol * >::iterator itProtoOwner = protoOwners.begin(); itProtoOwner != protoOwners.end(); ++itProtoOwner)
						{
							if ( CheckCompatibleFileContext(owner->GetFile(), (*itProtoOwner)->GetFile() ) )
							{
								std::vector< Symbols::ISymbol * > protoChildren = fSymbolTable->GetSymbolsByName( *itProtoOwner, CVSTR("prototype") );
								for (std::vector< Symbols::ISymbol * >::iterator itProtoChild = protoChildren.begin(); itProtoChild != protoChildren.end(); ++itProtoChild)
								{
									// Turn the Entity or EntityCollection complation mode on if needed
									if ( (*itProtoChild)->GetOwner() )
									{
										if ( (*itProtoChild)->GetOwner()->GetName() == CVSTR("Entity") )
											entityCompletionType = EntityModelCompletionType::kEntity;
										else if ( (*itProtoChild)->GetOwner()->GetName() == CVSTR("EntityCollection") )
											entityCompletionType = EntityModelCompletionType::kCollection;
									}
									lookup_list.insert( lookup_list.begin(), *itProtoChild );
								}
							}
							(*itProtoOwner)->Release();
						}
					}
					
					lookup_list.insert( lookup_list.begin(), *itRefLookupSymProto );
				}
			}
			refLookupSym->Release();
		}
	}

	for (std::vector<Symbols::ISymbol *>::iterator it = returnTypes.begin(); it != returnTypes.end(); ++it)
		(*it)->Release();

	// Time to sort our suggestion list. First of all we will sort 
	// all entries of each class table. Then we  will insert the main 
	// class entries, then the others classes entries and finally 
	//object class entries.
	VectorOfVString			order;
	bool					suggestObjectClass = false, suggestFunctionClass = false;

	order.push_back(mainClassName);

	for (std::map<VString, VectorOfVString>::iterator it = suggestionList.begin(); it != suggestionList.end(); ++it)
	{
		VString	className = (*it).first;
		
		if ( className == CVSTR("Object") )
			suggestObjectClass = true;
		else if ( className == CVSTR("Function") )
			suggestFunctionClass = true;
		else if ( className != mainClassName)
			order.push_back( className );
		sort( (*it).second.begin(), (*it).second.end() );
	}

	if (suggestFunctionClass)
		order.push_back( CVSTR("Function") );
	if (suggestObjectClass)
		order.push_back( CVSTR("Object") );

	for (VectorOfVString::iterator itKey = order.begin(); itKey != order.end(); ++itKey)
	{
		for (VectorOfVString::iterator itValue = suggestionList[*itKey].begin(); itValue != suggestionList[*itKey].end(); ++itValue)
		{
			VectorOfVString parts;

			(*itValue).GetSubStrings(JAVASCRIPT_PARSER_SUGGESTION_SEPARATOR, parts);
			if (parts.size() > 2)
			{
				VString displayText = parts[0] + JAVASCRIPT_PARSER_SUGGESTION_SEPARATOR + parts[1];
				fSuggestions->Suggest(displayText, parts[2], SuggestionInfo::eName );
			}
		}
	}
}

std::vector< Symbols::ISymbol * > JavaScriptParser::GetSuggestionReturnTypes(sLONG& outEntityCompletionType)
{
	std::vector< Symbols::ISymbol * > returnTypes;

	outEntityCompletionType = EntityModelCompletionType::kNone;

	for (std::vector<Symbols::ISymbol *>::iterator it = fCompletionSymbolList.begin(); it != fCompletionSymbolList.end(); ++it)
	{
		Symbols::ISymbol *sym = *it;

		// We first verify if symbol is an object property that has been assigned
		// with a function expression and replace it if true 
		Symbols::ISymbol *functionSym = fSymbolTable->GetAssignedFunction(sym);
		if (NULL != functionSym)
			sym = functionSym;

		// Current sym is an entity: we add it as return type
		if ( sym->IsEntityModelKind() )
		{
			outEntityCompletionType = EntityModelCompletionType::kEntity;

			// Add current Entity Model prototype to returns list
			Symbols::ISymbol* emPrototype = NULL;
			if ( GetPrototypeSubSymbol(sym, &emPrototype) )
				returnTypes.insert(returnTypes.begin(), emPrototype);
	
			// Add Entity class prototypes to returns list
			std::vector<Symbols::IFile*> files = fSymbolTable->GetFilesByName( CVSTR("datastore.js") );
			if ( files.size() )
			{
				Symbols::IFile* file = files.front();
				file->Retain();
				for (std::vector<Symbols::IFile *>::iterator it = files.begin(); it != files.end(); ++it)
						(*it)->Release();

				std::vector<Symbols::ISymbol*> entitySyms = fSymbolTable->GetSymbolsByName(0, CVSTR("Entity"), file);
				if ( entitySyms.size() )
				{
					Symbols::ISymbol* entitySym = entitySyms.front();
					entitySym->Retain();
					for (std::vector<Symbols::ISymbol *>::iterator it = entitySyms.begin(); it != entitySyms.end(); ++it)
						(*it)->Release();

					if (entitySym)
					{
						Symbols::ISymbol *refLookupSym = const_cast< Symbols::ISymbol * >( entitySym->RetainReferencedSymbol() );
						Symbols::ISymbol* entityPrototype = NULL;

						if ( GetPrototypeSubSymbol(refLookupSym, &entityPrototype) )
							returnTypes.insert(returnTypes.begin(), entityPrototype);

						refLookupSym->Release();
						entitySym->Release();
					}
				}
				file->Release();
			}
		}
		else
		{
            std::vector<Symbols::IFile*> files;
            std::vector<Symbols::ISymbol*> modules;
            
            if( sym->GetName() == "require" )
            {
                // Try to get the module name
                VString moduleName = this->fLexer->GetTokenText();
                // Strip ' from module name
                if( moduleName.BeginsWith("'") || moduleName.BeginsWith("\"") )  moduleName.Remove(1, 1);
                if( moduleName.EndsWith("'") || moduleName.EndsWith("\"") )    moduleName.Remove(moduleName.GetLength(), 1);
                
                // Try to get the module file symbol
                files = this->fSymbolTable->GetFilesByName(moduleName);
                if( files.size() >= 1 )
                {
                    std::vector<Symbols::IFile*>::const_iterator file;
                    for(file=files.begin(); file!=files.end(); file++)
                    {
						if(		(*file)->GetBaseFolder() == eSymbolFileBaseFolderServerModules ||
								((*file)->GetBaseFolder() == eSymbolFileBaseFolderProject && (*file)->GetPath().BeginsWith("Modules/")) )
                        {
                            // Try to get the "exports" symbols of the specified module and add it to the return types list
                            modules = this->fSymbolTable->GetSymbolsByName(NULL, "exports", (*file));
                            if( modules.size() == 1 )
                            {
                                sym = modules.at(0);
                                break;
                            }
                        }
                    }
					
					// Release memory
					for(file=files.begin(); file!=files.end(); file++)
						(*file)->Release();
                }
                
                // At this time, "exports" symbols do not have return types nor prototype, so the return type will be the "exports" symbo itself
                // A better thing would be to think about "exports" creation, and must or mustn't we have to add information such as return type or prototype
                returnTypes.insert(returnTypes.begin(), sym);
                return returnTypes;
            }
            
            // Else we add the symbol return types and their prototypes to the list
			std::vector< Symbols::ISymbol * > currentReturnTypes = sym->GetReturnTypes();
			for (std::vector<Symbols::ISymbol*>::iterator itRet = currentReturnTypes.begin(); itRet != currentReturnTypes.end(); ++itRet)
			{
				// If one of the return type is an "Entity" we add saved "Entity Model" 
				// symbol prototype to return list
				Symbols::ISymbol* entitySym = GetEntityCompletionSymbol();
				if ( entitySym && (*itRet)->GetOwner() )
				{ 
					VString className = (*itRet)->GetOwner()->GetName();
					if ( className == CVSTR("Entity") || className == CVSTR("EntityCollection") )
					{
						outEntityCompletionType = className == CVSTR("Entity") ? EntityModelCompletionType::kEntity : EntityModelCompletionType::kCollection;

						Symbols::ISymbol* entityPrototype = NULL;

						if ( GetPrototypeSubSymbol(entitySym, &entityPrototype) )
							returnTypes.insert(returnTypes.begin(), entityPrototype);
					}
				}

				returnTypes.insert(returnTypes.begin(), *itRet);
				(*itRet)->Retain();
			}
		}

		if (functionSym)
			functionSym->Release();
	}

	return returnTypes;
}

bool JavaScriptParser::GetPrototypeSubSymbol(Symbols::ISymbol* inOwner, Symbols::ISymbol** outPrototype)
{
	if (NULL != inOwner)
	{
		std::vector<Symbols::ISymbol *> prototypes = fSymbolTable->GetLikeNamedSubSymbols( inOwner, CVSTR("prototype" ), true);
		if ( prototypes.size() )
		{
			prototypes.front()->Retain();
			*outPrototype = prototypes.front();

			for (std::vector<Symbols::ISymbol *>::iterator it = prototypes.begin(); it != prototypes.end(); ++it)
				(*it)->Release();

			return true;
		}
	}
	
	return false;
}

bool JavaScriptParser::CheckCompatibleFileContext(Symbols::IFile* inFile1, Symbols::IFile* inFile2)
{
	bool ret = true;

	if (NULL != inFile1 && NULL != inFile2)
	{
		ESymbolFileExecContext inFile1Context = inFile1->GetExecutionContext();		
		ESymbolFileExecContext inFile2Context = inFile2->GetExecutionContext();

		if ( inFile1Context != eSymbolFileExecContextClientServer && inFile2Context != eSymbolFileExecContextClientServer )
			if (inFile1Context != inFile2Context)
				ret = false;
	}

	return ret;
}

JavaScriptAST::Node *JavaScriptParser::ParseMemberExpression( int &tk, bool argsRequired )
{
	// If we're at the end of input, we can also suggest the new keyword here.
	if (fSuggestions && -1 == tk) {
		fSuggestions->Suggest( "new", SuggestionInfo::eKeyword );
		fSuggestions->Suggest( "function", SuggestionInfo::eKeyword );
	}

	JavaScriptAST::Node *ret = NULL;
	if (tk != JavaScriptTokenValues::NEW) {
		if (!ret)	ret = ParsePrimaryExpression( tk );
		if (!ret)	ret = ParseFunctionExpression( tk );
		if (!ret)	return NULL;
	}

	sLONG loop = 0;

	bool done = true;
	do {
		int lineNumber = fLexer->GetCurrentLineNumber();
		switch (tk)
		{
			case JavaScriptTokenValues::NEW:
				{
				tk = fLexer->GetNextTokenForParser();
				JavaScriptAST::Node *rhs = ParseMemberExpression( tk, true );
				if (!rhs)	return NULL;
				if (argsRequired)
				{
					int argsCount;

					JavaScriptAST::FunctionCallArgumentsNode *args = ParseArguments( tk, argsCount);
					if (args)
						return new JavaScriptAST::NewNode( rhs, args, lineNumber );
					delete rhs;
					return NULL;
				}
				else
				{
					// In this case, the args are optional -- so we're always good to go, but we only
					// want to attempt to parse arguments if we see the start of the argument list
					JavaScriptAST::FunctionCallArgumentsNode *args = NULL;
					int argsCount;

					if ('(' == tk)
						args = ParseArguments( tk, argsCount );
					
					done = false;
					ret = new JavaScriptAST::NewNode( rhs, args, lineNumber );
				}
			} break;
			case '[': {
				tk = fLexer->GetNextTokenForParser();
				JavaScriptAST::Node *expr = ParseExpression( tk, false );
				if (!expr) {
					delete ret;
					return NULL;
				}
				if (']' != tk) {
					delete ret;
					delete expr;
					ThrowError( "Syntax Error" );
					return NULL;
				}

				tk = fLexer->GetNextTokenForParser();
				done = false;
				ret = new JavaScriptAST::ArrayExpressionNode( ret, expr, lineNumber );
			} break;
			case '.': {
				// If we are at first loop of a dot expression we extend the completion
				// symbol list to all symbols with the same name and context. By this way
				// we can handle the case of an global object defined in multiple files.
				if (!loop)
				{
					ResetCompletionSymbolList();
					ExpandCompletionSymbolList();
				}
				// After first loop we continue enlarging our completion research 
				// scope with children of completion symbols of superior level
				else
				{
					std::vector<Symbols::ISymbol*> subSymbols;
					
					for (std::vector<Symbols::ISymbol*>::iterator it = fCompletionSymbolList.begin(); it != fCompletionSymbolList.end(); ++it)
					{
 						Symbols::ISymbol* sym = QualifiedLookup( *it, fLexer->GetTokenText() );
						if (NULL != sym)
							subSymbols.insert(subSymbols.begin(), sym);
					}
					SetCompletionSymbolList( subSymbols );
					for (std::vector<Symbols::ISymbol*>::iterator it = subSymbols.begin(); it != subSymbols.end(); ++it)
						(*it)->Release();
				}

				tk = fLexer->GetNextTokenForParser();
				if (JavaScriptTokenValues::IDENTIFIER != tk)
				{
					// If we're at the end of the input, the user can enter any identifier at this point
					if (fSuggestions)
						SuggestAppropriateIdentifiers();
					delete ret;
					ThrowError( "Expected Identifier" );
					return NULL;
				}

				// At first loop we can release completion symbol as it is 
				// now replaced by completion symbols list
				if ( ! loop && fCompletionSymbol)
					SetCompletionSymbol( NULL );
				ret = new JavaScriptAST::DotExpressionNode( ret, new JavaScriptAST::IdentifierNode( fLexer->GetTokenText(), fLexer->GetCurrentLineNumber() ), lineNumber );
				tk = fLexer->GetNextTokenForParser();
				done = false;
			} break;
			default:	done = true;
		}
		loop++;

	} while (!done);
	
	return ret;
}

JavaScriptAST::Node *JavaScriptParser::ParseCallExpressionSubscript( int &tk, JavaScriptAST::Node *inNode )
{
	// Now we have the possibility of having another set of arguments,
	// or an array, or a dot.  Let's see which
	JavaScriptAST::Node *ret = inNode;
	
	VString	tokenText, callExprText;
	bool	done = true;
	sLONG	loop = 0;

	do
	{
		VString	tokenText = fLexer->GetTokenText();
		int		lineNumber = fLexer->GetCurrentLineNumber();
		
		// Use called function name as last token text has potentially 
		// been overriden by function call arguments
		if ( ! callExprText.IsEmpty() )
		{
			tokenText = callExprText;
			callExprText = CVSTR("");
		}

		switch (tk)
		{
			case '(':
				{
					int argsCount;

					callExprText = tokenText;

					// Backup, parse then restore completions symbols
					Symbols::ISymbol* completionSymbolBackup = NULL;
					std::vector<Symbols::ISymbol *> completionSymbolListBackup;

					BackupCompletionSymbols( &completionSymbolBackup, completionSymbolListBackup );
					JavaScriptAST::FunctionCallArgumentsNode *args = ParseArguments( tk, argsCount );
					RestoreCompletionSymbols( completionSymbolBackup, completionSymbolListBackup );

					if ( NULL != fSuggestions)
					{
						SuggestCallExpressionParameterValues( argsCount + 1 );
						fSuggestions->UnsuggestByType( SuggestionInfo::eKeyword );
					}

					if (!args)
					{
						delete ret;
						return NULL;
					}

					done = false;
					ret = new JavaScriptAST::FunctionCallExpressionNode( ret, args, lineNumber );
				}
				break;

			case '[':
				{
					tk = fLexer->GetNextTokenForParser();
					JavaScriptAST::Node *expr = ParseExpression( tk, false );

					if (!expr)
					{
						delete ret;
						return NULL;
					}

					if (']' != tk)
					{
						delete ret;
						delete expr;
						ThrowError( "Syntax Error" );
						return NULL;
					}

					tk = fLexer->GetNextTokenForParser();
					ret = new JavaScriptAST::ArrayExpressionNode( ret, expr, lineNumber );
					done = false;
				}
				break;

			case '.':
				{
                    if( loop == 1 && dynamic_cast<JavaScriptAST::IdentifierNode*>(inNode) )
                    {
						ExpandCompletionSymbolList();
                    }
					else
					{
						std::vector<Symbols::ISymbol*> subSymbols;

						for (std::vector<Symbols::ISymbol*>::iterator it = fCompletionSymbolList.begin(); it != fCompletionSymbolList.end(); ++it)
						{
							if ( (*it)->IsEntityModelKind() )
								SetEntityCompletionSymbol( *it );

							Symbols::ISymbol* sym = QualifiedLookup( *it, tokenText );
							if (NULL != sym)
								subSymbols.insert(subSymbols.begin(), sym);
						}
						SetCompletionSymbolList( subSymbols );
						for (std::vector<Symbols::ISymbol*>::iterator it = subSymbols.begin(); it != subSymbols.end(); ++it)
							(*it)->Release();
					}

					// Suggest if we are at end of input
					tk = fLexer->GetNextTokenForParser();
					if (JavaScriptTokenValues::IDENTIFIER != tk)
					{
						if (fSuggestions)
							SuggestAppropriateIdentifiers( dynamic_cast< JavaScriptAST::FunctionCallExpressionNode * >( ret ) ? true : false );

						delete ret;
						ThrowError( "Expected Identifier" );
						return NULL;
					}

					// Set return types as completion symbols if we're parsing a function call expression
					if ( dynamic_cast< JavaScriptAST::FunctionCallExpressionNode * >( ret ) )
					{
						sLONG entityCompletionType;

						std::vector< Symbols::ISymbol * > returnTypes = GetSuggestionReturnTypes(entityCompletionType) ;
						SetCompletionSymbolList( returnTypes );
						for (std::vector<Symbols::ISymbol*>::iterator it = returnTypes.begin(); it != returnTypes.end(); ++it)
							(*it)->Release();
					}

					// Create a new dot expression to be used at next loop
					ret = new JavaScriptAST::DotExpressionNode( ret, new JavaScriptAST::IdentifierNode( tokenText, fLexer->GetCurrentLineNumber() ), lineNumber );
					tk = fLexer->GetNextTokenForParser();

					done = false;
				}
				break;

			default:
				{
					done = true;
				}
				break;
		}
		loop++;
	} 
	while ( ! done );

	return ret;
}

void JavaScriptParser::SuggestCallExpressionParameterValues(sLONG inIndex)
{
	if ( NULL != fSuggestions)
	{
		std::vector<VString> values;

		Symbols::ISymbol* completionSymbol = (NULL != fCompletionSymbol) ? fCompletionSymbol : fCompletionSymbolList.size() ? fCompletionSymbolList[0] : NULL;
		if ( NULL != completionSymbol )
		{
			completionSymbol->GetParamValue(inIndex, values);
			for (std::vector< VString >::const_iterator it = values.begin(); it != values.end(); ++it)
				fSuggestions->Suggest( *it + CVSTR("||") + completionSymbol->GetName(), *it, SuggestionInfo::eName );
		}
	}
}


void JavaScriptParser::ExpandCompletionSymbolList()
{
	if (fCompletionSymbol)
	{
		Symbols::ISymbol *refSym =  const_cast< Symbols::ISymbol * >( fCompletionSymbol->RetainReferencedSymbol() );

		// As we want to exclude JSF symbols duplication, we limit symbol search 
		// extension to "named" symbols only defined in project
		if ( !refSym->GetOwner() && 
			 refSym->GetName().GetLength() &&
			 refSym->GetFile() &&
			 refSym->GetFile()->GetBaseFolder() == eSymbolFileBaseFolderProject)
		{
			// WAK0079751 start
			Symbols::IFile* pOwnerFile = refSym->GetFile();
			std::vector<Symbols::ISymbol *> projectSyms = fSymbolTable->GetSymbolsByName( refSym->GetOwner(), refSym->GetName(), pOwnerFile );
			if( projectSyms.size() == 0 )
			{
				// We extend the symbol search not only to the current file
				projectSyms = fSymbolTable->GetSymbolsByName( refSym->GetOwner(), refSym->GetName(), NULL );
			}
			// WAK0079751 stop
			SetCompletionSymbolList( projectSyms );
			for (std::vector<Symbols::ISymbol*>::iterator it = projectSyms.begin(); it != projectSyms.end(); ++it)
				(*it)->Release();
		}
		else
			SetCompletionSymbolList(refSym);

		refSym->Release();
	}
}

JavaScriptAST::Node *JavaScriptParser::ParseLeftHandSideExpression( int &tk )
{
	// We do not follow the reference grammar in this situation because it is
	// incredibly obtuse and twisty.  However, what we've implemented is functionally
	// equivalent.  Basically, we know that NewExpression and CallExpression always 
	// start with a MemberExpression.  The only interesting thing NewExpression provides
	// is a way to make arguments optional instead of required.  So we will tell the 
	// member expression that arguments are optional in the case where a new clause is
	// found.  However, if it recurses, then the arguments become required.
	JavaScriptAST::Node *ret = NULL;
	ret = ParseMemberExpression( tk, false );
	if (!ret)	return NULL;

	if ('(' == tk || '.' == tk || '[' == tk) {
		ret = ParseCallExpressionSubscript( tk, ret );
	}
	return ret;
}

JavaScriptAST::Node *JavaScriptParser::ParsePostFixExpression( int &tk )
{
	// We always start with a left-hand side expression
	JavaScriptAST::Node *node = ParseLeftHandSideExpression( tk );
	if (!node)	return NULL;

	// Now we require a ++ or a --
	bool bInc = (JavaScriptTokenValues::INCREMENTOR == tk) ? true : false;
	int lineNumber = fLexer->GetCurrentLineNumber();
	if (JavaScriptTokenValues::INCREMENTOR == tk ||
		JavaScriptTokenValues::DECREMENTOR == tk) {
		// If the last token we parsed included a newline, then we have
		// to automatically insert a semi-colon for the user and let the
		// caller handle the incrementor or decrementor as part of the
		// next statement.
		if (fLexer->ConsumedNewlineBeforeToken()) {
			return node;
		}

		// Since we processed that token, we need to grab the
		// next one since the caller is expecting tk to point to
		// the next token in the stream.
		tk = fLexer->GetNextTokenForParser();
	} else {
		return node;
	}

	// However, it is acceptable if we didn't get an incrementor or
	// decrementor since part of this production is just the left-hand
	// side expression
	if (bInc)	return new JavaScriptAST::PostIncrementorNode( node, lineNumber );
	else		return new JavaScriptAST::PostDecrementorNode( node, lineNumber );
}

JavaScriptAST::Node *JavaScriptParser::ParseUnaryExpression( int &tk )
{
	JavaScriptAST::Node *ret = NULL;

	#define	HANDLE_CASE( nodeType )												\
			int lineNum = fLexer->GetCurrentLineNumber();						\
			tk = fLexer->GetNextTokenForParser();								\
			JavaScriptAST::Node *node = ParseUnaryExpression( tk );				\
			if (node)	ret = new JavaScriptAST::nodeType( node, lineNum );
				
	switch (tk) {
		case JavaScriptTokenValues::KWORD_DELETE: {
			HANDLE_CASE( DeleteExpressionNode );
		} break;
		case JavaScriptTokenValues::KWORD_VOID: {
			HANDLE_CASE( VoidExpressionNode );
		} break;
		case JavaScriptTokenValues::TYPEOF: {
			HANDLE_CASE( TypeOfExpressionNode );
		} break;
		case JavaScriptTokenValues::INCREMENTOR: {
			HANDLE_CASE( PreIncrementorExpressionNode );
		} break;
		case JavaScriptTokenValues::DECREMENTOR: {
			HANDLE_CASE( PreDecrementorExpressionNode );
		} break;
		case '+': {
			HANDLE_CASE( UnaryPlusExpressionNode );
		} break;
		case '-': {
			HANDLE_CASE( UnaryNegateExpressionNode );
		} break;
		case '~': {
			HANDLE_CASE( BitwiseNotExpressionNode );
		} break;
		case '!': {
			HANDLE_CASE( LogicalNotExpressionNode );
		} break;
		case -1: {
			// We're at the end of input, so suggest possible keywords
			if (fSuggestions) {
				fSuggestions->Suggest( "delete", SuggestionInfo::eKeyword );
				fSuggestions->Suggest( "void", SuggestionInfo::eKeyword );
				fSuggestions->Suggest( "typeof", SuggestionInfo::eKeyword );
			}
		}	// We *want* to fall through so that we can gather other suggestions
		default: {
			return ParsePostFixExpression( tk );
		} break;
	}

#undef HANDLE_CASE

	return ret;
}

JavaScriptAST::Node *JavaScriptParser::ParseMultiplicativeExpression( int &tk )
{
	JavaScriptAST::Node *ret = ParseUnaryExpression( tk );
	if (!ret)	return NULL;

#define HANDLE_CASE( nodeType )										\
			int lineNum = fLexer->GetCurrentLineNumber();			\
			tk = fLexer->GetNextTokenForParser();					\
			JavaScriptAST::Node *rhs = ParseUnaryExpression( tk );	\
			if (!rhs) {												\
				delete ret;											\
				return NULL;										\
			}														\
			ret = new JavaScriptAST::nodeType( ret, rhs, fLexer->GetCurrentLineNumber() );	\
			done = false;											
			
	bool done = true;
	do {
		switch (tk) {
			case '*': {
				HANDLE_CASE( MultiplicationExpressionNode );
			} break;
			case '/':
            case JavaScriptTokenValues::REGEXP: {
				HANDLE_CASE( DivisionExpressionNode );
			} break;
			case '%': {
				HANDLE_CASE( ModulusExpressionNode );
			} break;
			default:	done = true;
		}
	} while (!done);

#undef HANDLE_CASE

	return ret;
}

JavaScriptAST::Node *JavaScriptParser::ParseAdditiveExpression( int &tk )
{
	JavaScriptAST::Node *ret = ParseMultiplicativeExpression( tk );
	if (!ret)	return NULL;

#define HANDLE_CASE( nodeType )												\
			int lineNum = fLexer->GetCurrentLineNumber();					\
			tk = fLexer->GetNextTokenForParser();							\
			JavaScriptAST::Node *rhs = ParseMultiplicativeExpression( tk );	\
			if (!rhs) {														\
				delete ret;													\
				return NULL;												\
			}																\
			ret = new JavaScriptAST::nodeType( ret, rhs, fLexer->GetCurrentLineNumber() );	\
			done = false;													

	bool done = true;
	do {
		switch (tk) {
			case '+': {
				HANDLE_CASE( AdditionExpressionNode );
			} break;
			case '-': {
				HANDLE_CASE( SubtractionExpressionNode );
			} break;
			default:	done = true;
		}
	} while (!done);

#undef HANDLE_CASE

	return ret;
}

JavaScriptAST::Node *JavaScriptParser::ParseShiftExpression( int &tk )
{
	JavaScriptAST::Node *ret = ParseAdditiveExpression( tk );
	if (!ret)	return NULL;

#define HANDLE_CASE( nodeType )												\
			int lineNum = fLexer->GetCurrentLineNumber();					\
			tk = fLexer->GetNextTokenForParser();							\
			JavaScriptAST::Node *rhs = ParseAdditiveExpression( tk );		\
			if (!rhs) {														\
				delete ret;													\
				return NULL;												\
			}																\
			ret = new JavaScriptAST::nodeType( ret, rhs, fLexer->GetCurrentLineNumber() );	\
			done = false;													

	bool done = true;
	do {
		switch (tk) {
			case JavaScriptTokenValues::LEFTSHIFT: {
				HANDLE_CASE( LeftShiftExpressionNode );
			} break;
			case JavaScriptTokenValues::SIGNEDRIGHTSHIFT: {
				HANDLE_CASE( SignedRightShiftExpressionNode );
			} break;
			case JavaScriptTokenValues::UNSIGNEDRIGHTSHIFT: {
				HANDLE_CASE( UnsignedRightShiftExpressionNode );
			} break;
			default:	done = true;
		}
	} while (!done);
	
#undef HANDLE_CASE

	return ret;
}

JavaScriptAST::Node *JavaScriptParser::ParseRelationalExpression( int &tk, bool noIn )
{
	JavaScriptAST::Node *ret = ParseShiftExpression( tk );
	if (!ret)	return NULL;

#define HANDLE_CASE( nodeType )												\
			int lineNum = fLexer->GetCurrentLineNumber();					\
			tk = fLexer->GetNextTokenForParser();							\
			JavaScriptAST::Node *rhs = ParseShiftExpression( tk );			\
			if (!rhs) {														\
				delete ret;													\
				return NULL;												\
			}																\
			ret = new JavaScriptAST::nodeType( ret, rhs, fLexer->GetCurrentLineNumber() );	\
			done = false;													

	bool done = true;
	do {
		switch (tk) {
			case JavaScriptTokenValues::KWORD_IN: {
				// If we're told we don't want an IN, but we found one anyways, that
				// may not actually be a syntax error -- it's up to the caller to decide
				// that, not this production.
				if (noIn)							return ret;

				HANDLE_CASE( InExpressionNode );
			} break;
			case '<': {
				HANDLE_CASE( LessThanExpressionNode );
			} break;
			case '>': {
				HANDLE_CASE( GreaterThanExpressionNode );
			} break;
			case JavaScriptTokenValues::LESSTHANOREQUALTO: {
				HANDLE_CASE( LessThanOrEqualToExpressionNode );
			} break;
			case JavaScriptTokenValues::GREATERTHANOREQUALTO: {
				HANDLE_CASE( GreaterThanOrEqualToExpressionNode );
			} break;
			case JavaScriptTokenValues::INSTANCEOF: {
				HANDLE_CASE( InstanceOfExpressionNode );
			} break;
			case -1: {
				// We're at the end of input, so suggest possible keywords
				if (fSuggestions) {
					fSuggestions->Suggest( "instanceof", SuggestionInfo::eKeyword );
					if (!noIn) {
						fSuggestions->Suggest( "in", SuggestionInfo::eKeyword );
					}
				}
				done = true;
			} break;
			default:	done = true;
		}
	} while (!done);

#undef HANDLE_CASE

	return ret;
}

JavaScriptAST::Node *JavaScriptParser::ParseEqualityExpression( int &tk, bool noIn )
{
	JavaScriptAST::Node *ret = ParseRelationalExpression( tk, noIn );
	if (!ret)	return NULL;

#define HANDLE_CASE( nodeType )																\
			int lineNum = fLexer->GetCurrentLineNumber();									\
			tk = fLexer->GetNextTokenForParser();											\
			JavaScriptAST::Node *rhs = ParseRelationalExpression( tk, noIn );				\
			if (!rhs) {																		\
				delete ret;																	\
				return NULL;																\
			}																				\
			ret = new JavaScriptAST::nodeType( ret, rhs, fLexer->GetCurrentLineNumber() );	\
			done = false;													

	bool done = true;
	do {
		switch (tk) {
			case JavaScriptTokenValues::EQUALITY: {
				HANDLE_CASE( EqualityExpressionNode );
			} break;
			case JavaScriptTokenValues::NONEQUALITY: {
				HANDLE_CASE( NonEqualityExpressionNode );
			} break;
			case JavaScriptTokenValues::STRICTEQUALITY: {
				HANDLE_CASE( StrictEqualityExpressionNode );
			} break;
			case JavaScriptTokenValues::STRICTNONEQUALITY: {
				HANDLE_CASE( StrictNonEqualityExpressionNode );
			} break;
			default:	done = true;
		}
	} while (!done);

#undef HANDLE_CASE

	return ret;
}

JavaScriptAST::Node *JavaScriptParser::ParseBitwiseAndExpression( int &tk, bool noIn )
{
	JavaScriptAST::Node *ret = ParseEqualityExpression( tk, noIn );
	if (!ret)	return NULL;

#define HANDLE_CASE( nodeType )																\
			int lineNum = fLexer->GetCurrentLineNumber();									\
			tk = fLexer->GetNextTokenForParser();											\
			JavaScriptAST::Node *rhs = ParseEqualityExpression( tk, noIn );					\
			if (!rhs) {																		\
				delete ret;																	\
				return NULL;																\
			}																				\
			ret = new JavaScriptAST::nodeType( ret, rhs, fLexer->GetCurrentLineNumber() );	\
			done = false;													

	bool done = true;
	do {
		switch (tk) {
			case '&': {
				HANDLE_CASE( BitwiseAndExpressionNode );
			} break;
			default:	done = true;
		}
	} while (!done);
	
#undef HANDLE_CASE

	return ret;
}

JavaScriptAST::Node *JavaScriptParser::ParseBitwiseXorExpression( int &tk, bool noIn )
{
	JavaScriptAST::Node *ret = ParseBitwiseAndExpression( tk, noIn );
	if (!ret)	return NULL;

#define HANDLE_CASE( nodeType )																\
			int lineNum = fLexer->GetCurrentLineNumber();									\
			tk = fLexer->GetNextTokenForParser();											\
			JavaScriptAST::Node *rhs = ParseBitwiseAndExpression( tk, noIn );				\
			if (!rhs) {																		\
				delete ret;																	\
				return NULL;																\
			}																				\
			ret = new JavaScriptAST::nodeType( ret, rhs, fLexer->GetCurrentLineNumber() );	\
			done = false;													

	bool done = true;
	do {
		switch (tk) {
			case '^': {
				HANDLE_CASE( BitwiseXorExpressionNode );
			} break;
			default:	done = true;
		}
	} while (!done);

#undef HANDLE_CASE

	return ret;
}

JavaScriptAST::Node *JavaScriptParser::ParseBitwiseOrExpression( int &tk, bool noIn )
{
	JavaScriptAST::Node *ret = ParseBitwiseXorExpression( tk, noIn );
	if (!ret)	return NULL;

#define HANDLE_CASE( nodeType )																\
			int lineNum = fLexer->GetCurrentLineNumber();									\
			tk = fLexer->GetNextTokenForParser();											\
			JavaScriptAST::Node *rhs = ParseBitwiseXorExpression( tk, noIn );				\
			if (!rhs) {																		\
				delete ret;																	\
				return NULL;																\
			}																				\
			ret = new JavaScriptAST::nodeType( ret, rhs, fLexer->GetCurrentLineNumber() );	\
			done = false;													

	bool done = true;
	do {
		switch (tk) {
			case '|': {
				HANDLE_CASE( BitwiseOrExpressionNode );
			} break;
			default:	done = true;
		}
	} while (!done);

#undef HANDLE_CASE

	return ret;
}

JavaScriptAST::Node *JavaScriptParser::ParseLogicalAndExpression( int &tk, bool noIn )
{
	JavaScriptAST::Node *ret = ParseBitwiseOrExpression( tk, noIn );
	if (!ret)	return NULL;

#define HANDLE_CASE( nodeType )																\
			int lineNum = fLexer->GetCurrentLineNumber();									\
			tk = fLexer->GetNextTokenForParser();											\
			JavaScriptAST::Node *rhs = ParseBitwiseOrExpression( tk, noIn );				\
			if (!rhs) {																		\
				delete ret;																	\
				return NULL;																\
			}																				\
			ret = new JavaScriptAST::nodeType( ret, rhs, fLexer->GetCurrentLineNumber() );	\
			done = false;													

	bool done = true;
	do {
		switch (tk) {
			case JavaScriptTokenValues::LOGICALAND: {
				HANDLE_CASE( LogicalAndExpressionNode );
			} break;
			default:	done = true;
		}
	} while (!done);

#undef HANDLE_CASE

	return ret;
}

JavaScriptAST::Node *JavaScriptParser::ParseLogicalOrExpression( int &tk, bool noIn )
{
	JavaScriptAST::Node *ret = ParseLogicalAndExpression( tk, noIn );
	if (!ret)	return NULL;

#define HANDLE_CASE( nodeType )																\
			int lineNum = fLexer->GetCurrentLineNumber();									\
			tk = fLexer->GetNextTokenForParser();											\
			JavaScriptAST::Node *rhs = ParseLogicalAndExpression( tk, noIn );				\
			if (!rhs) {																		\
				delete ret;																	\
				return NULL;																\
			}																				\
			ret = new JavaScriptAST::nodeType( ret, rhs, fLexer->GetCurrentLineNumber() );	\
			done = false;													

	bool done = true;
	do {
		switch (tk) {
			case JavaScriptTokenValues::LOGICALOR: {
				HANDLE_CASE( LogicalOrExpressionNode );
			} break;
			default:	done = true;
		}
	} while (!done);

#undef HANDLE_CASE

	return ret;
}

JavaScriptAST::Node *JavaScriptParser::ParseConditionalExpression( int &tk, bool noIn )
{
	// Either we have a logical OR expression, or we have the ?: operator, but either
	// way, the first part of this expression tests for OR
	JavaScriptAST::Node *ret = ParseLogicalOrExpression( tk, noIn );
	if (!ret)	return NULL;

	if (tk == '?') {
		int lineNumber = fLexer->GetCurrentLineNumber();
		// Parse the assignment expression followed by the : and another assignment expression
		tk = fLexer->GetNextTokenForParser();
		JavaScriptAST::Node *trueNode = ParseAssignmentExpression( tk, noIn );
		if (!trueNode)	{
			delete ret;
			return NULL;
		}

		if (tk != ':') {
			delete ret;
			delete trueNode;
			ThrowError( "Syntax Error" );
			return NULL;
		}
		tk = fLexer->GetNextTokenForParser();
		JavaScriptAST::Node *falseNode = ParseAssignmentExpression( tk, noIn );
		if (!falseNode) {
			delete ret;
			delete trueNode;
			return NULL;
		}

		ret = new JavaScriptAST::ConditionalExpressionNode( ret, trueNode, falseNode, lineNumber );
	}

	return ret;
}

JavaScriptAST::Node *JavaScriptParser::ParseAssignmentExpression( int &tk, bool noIn )
{
	JavaScriptAST::Node *lhs = 	ParseConditionalExpression( tk, noIn );

	// If the expression we parsed wasn't a valid left-hand side expression,
	// then we don't need to check for assignment operators
	if (!lhs || !lhs->IsLeftHandSideExpression())	return lhs;

	// We did parse a valid LHS, so we need to see if the next token is an
	// assignment for us to handle
	bool done = true;
	do {
		switch (tk) {
			case JavaScriptTokenValues::ASSIGNMULTIPLY:
			case JavaScriptTokenValues::ASSIGNDIVIDE: 
			case JavaScriptTokenValues::ASSIGNMODULUS:
			case JavaScriptTokenValues::ASSIGNADD:
			case JavaScriptTokenValues::ASSIGNSUBTRACT:
			case JavaScriptTokenValues::ASSIGNLEFTSHIFT:
			case JavaScriptTokenValues::ASSIGNSIGNEDRIGHTSHIFT:
			case JavaScriptTokenValues::ASSIGNUNSIGNEDRIGHTSHIFT:
			case JavaScriptTokenValues::ASSIGNBITAND:
			case JavaScriptTokenValues::ASSIGNBITXOR:
			case JavaScriptTokenValues::ASSIGNBITOR:
			case '=': {
				if (!lhs->IsLeftHandSideExpression() || lhs->IsLiteral()) {
					// It's possible that there was a chain assignment, like a = b = c, which means
					// we need to test whether there was an invalid LHS in there or not.  If we did
					// hit an invalid LHS, then we need to terminate all assignments as this is illegal.
					// The other test case we have to care about are assignments to literals.  While a literal
					// is a left-hand side expression, it is not a legal left-hand side value.
					delete lhs;
					ThrowError( "Illegal Left Hand Side" );
					return NULL;
				} else {
					// Before we parse the rhs, let's pull any ScriptDoc comments out of the lexer so that we
					// can store them on the return node.  However, which side of the assignment we assign the
					// comment to depends on the left and right hand sides.  For instance, with a function expression,
					// the rhs needs the information (so it can set up parameters and return type information).  But
					// for a simple assignment (eg: foo.prototype.bar = 12), the lhs is what needs the information.
					// For right now, function expressions are the only special case where the right-hand side will 
					// receive the ScriptDoc object.
					ScriptDocComment *comment = NULL;
					if (IsScriptDocElement()) {
						comment = ParseScriptDocElement();
					}

					int typeToCreate = tk;
					int lineNumber = fLexer->GetCurrentLineNumber();
					tk = fLexer->GetNextTokenForParser();
					JavaScriptAST::Node *rhs = ParseAssignmentExpression( tk, noIn );
					if (!rhs) {
						delete lhs;
						return NULL;
					}

					if (comment) {
						// Now that we have a ScriptDoc comment and we've parsed the right-hand side, we know where
						// we'd like to attach that information.
						if (dynamic_cast< JavaScriptAST::FunctionExpressionNode * >( rhs )) {
							rhs->AttachScriptDocComment( comment );
						} else {
							lhs->AttachScriptDocComment( comment );
						}
					}
					
					switch (typeToCreate) {
						case JavaScriptTokenValues::ASSIGNMULTIPLY:				lhs = new JavaScriptAST::AssignMultiplyExpressionNode( lhs, rhs, lineNumber ); break;
						case JavaScriptTokenValues::ASSIGNDIVIDE:				lhs = new JavaScriptAST::AssignDivideExpressionNode( lhs, rhs, lineNumber ); break;
						case JavaScriptTokenValues::ASSIGNMODULUS:				lhs = new JavaScriptAST::AssignModulusExpressionNode( lhs, rhs, lineNumber ); break;
						case JavaScriptTokenValues::ASSIGNADD:					lhs = new JavaScriptAST::AssignAddExpressionNode( lhs, rhs, lineNumber ); break;
						case JavaScriptTokenValues::ASSIGNSUBTRACT:				lhs = new JavaScriptAST::AssignSubtractExpressionNode( lhs, rhs, lineNumber ); break;
						case JavaScriptTokenValues::ASSIGNLEFTSHIFT:			lhs = new JavaScriptAST::AssignLeftShiftExpressionNode( lhs, rhs, lineNumber ); break;
						case JavaScriptTokenValues::ASSIGNSIGNEDRIGHTSHIFT:		lhs = new JavaScriptAST::AssignSignedRightShiftExpressionNode( lhs, rhs, lineNumber ); break;
						case JavaScriptTokenValues::ASSIGNUNSIGNEDRIGHTSHIFT:	lhs = new JavaScriptAST::AssignUnsignedRightShiftExpressionNode( lhs, rhs, lineNumber ); break;
						case JavaScriptTokenValues::ASSIGNBITAND:				lhs = new JavaScriptAST::AssignBitwiseAndExpressionNode( lhs, rhs, lineNumber ); break;
						case JavaScriptTokenValues::ASSIGNBITXOR:				lhs = new JavaScriptAST::AssignBitwiseXorExpressionNode( lhs, rhs, lineNumber ); break;
						case JavaScriptTokenValues::ASSIGNBITOR:				lhs = new JavaScriptAST::AssignBitwiseOrExpressionNode( lhs, rhs, lineNumber ); break;
						case '=':												lhs = new JavaScriptAST::AssignExpressionNode( lhs, rhs, lineNumber ); break;
					}
					done = false;
				}
			} break;
			default:	done = true;
		}
	} while (!done);
	
	return lhs;
}

JavaScriptAST::Node *JavaScriptParser::ParseExpression( int &tk, bool noIn )
{
	JavaScriptAST::Node *ret = ParseAssignmentExpression( tk, noIn );
	if (!ret)	return NULL;

	bool done = true;
	do {
		switch (tk) {
			case ',': {
				int lineNumber = fLexer->GetCurrentLineNumber();
				tk = fLexer->GetNextTokenForParser();
				JavaScriptAST::Node *rhs = ParseAssignmentExpression( tk, noIn );
				if (!rhs) {
					delete ret;
					return NULL;
				}
				ret = new JavaScriptAST::CommaExpressionNode( ret, rhs, lineNumber );
				done = false;
			} break;
			default:	done = true;
		}
	} while (!done);
	
	return ret;
}

bool JavaScriptParser::IsStatementBlock( int tk )
{
	return ('{' == tk);
}

JavaScriptAST::Node *JavaScriptParser::ParseStatementBlock( int &tk )
{
	if ('{' != tk)	return NULL;
	int lineNumber = fLexer->GetCurrentLineNumber();
	int nextTk = fLexer->PeekAtNextTokenForParser();

	JavaScriptAST::Node *ret = NULL;
	if ('}' != nextTk)
	{
		tk = (nextTk == -1) ? nextTk : fLexer->GetNextTokenForParser();
		ret = ParseStatementList( tk );
		if (!ret)	return NULL;
	} 
	else
	{	
		// Test if we are finding the Object symbol "{}"
		if ( ! fLexer->GetTokenText().GetLength())
			ret = ParseMemberExpression(tk,false);
		if (ret)
			return ret;

		// It just so happens to be an empty statement list, that's all
		tk = fLexer->GetNextTokenForParser();
		ret = new JavaScriptAST::StatementList( lineNumber );
	}

	if ('}' != tk) {
		delete ret;
		ThrowError( "Syntax Error" );
		return NULL;
	}

	tk = fLexer->GetNextTokenForParser();

	return ret;
}

JavaScriptAST::Node *JavaScriptParser::ParseStatementList( int &tk )
{
	bool keepGoing = true;
	JavaScriptAST::StatementList *list = NULL;

	// Due to error recovery, we assume that any statement or source
	// element can throw a ParsingErrorException.  We need to catch 
	// that exception if it's thrown so that we can attempt to recover
	// from the error and resume parsing from a safe place.
	do {
		try {
			int lineNumber = fLexer->GetCurrentLineNumber();
			JavaScriptAST::Node *node = ParseStatement( tk );
			if (node) {
				keepGoing = true;
				if (!list)	list = new JavaScriptAST::StatementList( lineNumber );
				list->AddStatement( node );
			} else {
				if (list)	list->SetListCompletionLine( fLexer->GetCurrentLineNumber() );
				keepGoing = false;
			}
		} catch (ParsingErrorException) {
			RecoverFromError( tk );
			keepGoing = (-1 != tk);
		}
	} while (keepGoing);

	return list;
}

bool JavaScriptParser::IsVariableStatement( int tk )
{
	return (JavaScriptTokenValues::VAR == tk);
}

JavaScriptAST::Node *JavaScriptParser::ParseStatement( int &tk )
{
	// If the thread is dying, we don't want to continue parsing.  We want to bail out
	// as quickly as possible so that the thread can continue to die.
	if (VTask::GetCurrent()->IsDying())	return NULL;

	if (fSuggestions && -1 == tk) {
		// We've reached the end of input while expecting a statement, so
		// let's suggest anything which can open a statement.  This needs
		// to happen before we parse any other statements because those statements
		// may reach the end of input themselves and fail (such as ParseExpressionStatement),
		// and we don't want to add statement openers in the middle of an expression statement.
		fSuggestions->Suggest( "function", SuggestionInfo::eKeyword );
		fSuggestions->Suggest( "var", SuggestionInfo::eKeyword );
		fSuggestions->Suggest( "if", SuggestionInfo::eKeyword );
		fSuggestions->Suggest( "do", SuggestionInfo::eKeyword );
		fSuggestions->Suggest( "while", SuggestionInfo::eKeyword );
		fSuggestions->Suggest( "for", SuggestionInfo::eKeyword );
		fSuggestions->Suggest( "break", SuggestionInfo::eKeyword );
		fSuggestions->Suggest( "continue", SuggestionInfo::eKeyword );
		fSuggestions->Suggest( "return", SuggestionInfo::eKeyword );
		fSuggestions->Suggest( "with", SuggestionInfo::eKeyword );
		fSuggestions->Suggest( "switch", SuggestionInfo::eKeyword );
		fSuggestions->Suggest( "throw", SuggestionInfo::eKeyword );
		fSuggestions->Suggest( "try", SuggestionInfo::eKeyword );
		fSuggestions->SuggestJavaScriptIdentifiers();
		// If we're currently in the process of parsing a method body, we want the magical
		// "arguments" variable to be available.
		if (fContextSymbol != NULL) {
			fSuggestions->Suggest( "arguments", SuggestionInfo::eName );
		}
	} else if (IsStatementBlock( tk )) {
		return ParseStatementBlock( tk );
	} else if (IsVariableStatement( tk )) {
		return ParseVariableStatement( tk );
	} else if (IsIfStatement( tk )) {
		return ParseIfStatement( tk );
	} else if (IsIterationStatement( tk )) {
		return ParseIterationStatement( tk );
	} else if (IsContinueStatement( tk )) {
		return ParseContinueStatement( tk );
	} else if (IsBreakStatement( tk )) {
		return ParseBreakStatement( tk );
	} else if (IsDebuggerStatement( tk )) {
		return ParseDebuggerStatement( tk );
	} else if (IsReturnStatement( tk )) {
		return ParseReturnStatement( tk );
	} else if (IsWithStatement( tk )) {
		return ParseWithStatement( tk );
	} else if (IsSwitchStatement( tk )) {
		return ParseSwitchStatement( tk );
	} else if (IsThrowStatement( tk )) {
		return ParseThrowStatement( tk );
	} else if (IsTryStatement( tk )) {
		return ParseTryStatement( tk );
	} else if (IsFunctionStatement( tk )) {
		return ParseFunctionDeclaration( tk );
	} else {
		JavaScriptAST::Node *ret = NULL;
		if (!ret)	ret = ParseExpressionStatement( tk );
		if (!ret)	ret = ParseEmptyStatement( tk );
		if (!ret)	ret = ParseLabeledStatement( tk );
		
		return ret;
	}

	return NULL;
}

bool JavaScriptParser::IsFunctionStatement( int tk )
{
	return (JavaScriptTokenValues::FUNCTION == tk);
}

JavaScriptAST::Node *JavaScriptParser::ParseEmptyStatement( int &tk )
{
	if (';' != tk)	return NULL;
	int lineNumber = fLexer->GetCurrentLineNumber();
	tk = fLexer->GetNextTokenForParser();
	return new JavaScriptAST::EmptyStatementNode( lineNumber );
}

JavaScriptAST::Node *JavaScriptParser::ParseExpressionStatement( int &tk )
{
	// According to the spec, we are supposed to lookahead to ensure that we don't
	// have a { or the function keyword.
	if ('{' == tk || JavaScriptTokenValues::FUNCTION == tk)	return  NULL;
	JavaScriptAST::Node *ret = ParseExpression( tk, false );
	if (ret && !ParseStatementCompletion( tk )) {
		delete ret;
		ret = NULL;
	}
	return ret;
}

bool JavaScriptParser::IsIfStatement( int tk )
{
	return (JavaScriptTokenValues::IF == tk);
}

JavaScriptAST::Node *JavaScriptParser::ParseIfStatement( int &tk )
{
	if (JavaScriptTokenValues::IF != tk)	return NULL;
	int lineNumber = fLexer->GetCurrentLineNumber();
	tk = fLexer->GetNextTokenForParser();

	if ('(' != tk) {
		ThrowError( "Syntax Error" );
		return NULL;
	}
	tk = fLexer->GetNextTokenForParser();

	JavaScriptAST::Node *expr = ParseExpression( tk, false );
	if (!expr)	return NULL;

	if (')' != tk) {
		delete expr;
		ThrowError( "Syntax Error" );
		return NULL;
	}
	tk = fLexer->GetNextTokenForParser();

	JavaScriptAST::Node *trueStatements = ParseStatement( tk );
	if (!trueStatements) {
		delete expr;
		ThrowError( "Syntax Error" );
		return NULL;
	}

	JavaScriptAST::Node *falseStatements = NULL;
	if (JavaScriptTokenValues::ELSE == tk) {
		tk = fLexer->GetNextTokenForParser();

		falseStatements = ParseStatement( tk );
		if (!falseStatements) {
			delete expr;
			delete trueStatements;
			ThrowError( "Syntax Error" );
			return NULL;
		}
	}

	return new JavaScriptAST::IfStatementNode( expr, trueStatements, falseStatements, lineNumber );
}

bool JavaScriptParser::IsIterationStatement( int tk )
{
	return (JavaScriptTokenValues::DO == tk ||
		JavaScriptTokenValues::WHILE == tk ||
		JavaScriptTokenValues::FOR == tk);
}

JavaScriptAST::Node *JavaScriptParser::ParseIterationStatement( int &tk )
{
	int lineNumber = fLexer->GetCurrentLineNumber();

	if (JavaScriptTokenValues::DO == tk) {
		tk = fLexer->GetNextTokenForParser();
		JavaScriptAST::Node *statement = ParseStatement( tk );
		if (!statement) {
			ThrowError( "Syntax Error" );
			return NULL;
		}
		if (JavaScriptTokenValues::WHILE != tk) {
			if (fSuggestions && -1 == tk) {
				fSuggestions->Suggest( "while", SuggestionInfo::eKeyword );
			}
			delete statement;
			ThrowError( "Syntax Error" );
			return NULL;
		}
		tk = fLexer->GetNextTokenForParser();
		
		if ('(' != tk) {
			delete statement;
			ThrowError( "Syntax Error" );
			return NULL;
		}
		tk = fLexer->GetNextTokenForParser();

		JavaScriptAST::Node *expr = ParseExpression( tk, false );
		if (!expr) {
			delete statement;
			return NULL;
		}

		if (')' != tk) {
			delete statement;
			delete expr;
			ThrowError( "Syntax Error" );
			return NULL;
		}
		tk = fLexer->GetNextTokenForParser();

		if (!ParseStatementCompletion( tk )) {
			delete expr;
			delete statement;
			return NULL;
		}
		return new JavaScriptAST::DoStatementNode( expr, statement, lineNumber );
	} else if (JavaScriptTokenValues::WHILE == tk) {
		tk = fLexer->GetNextTokenForParser();

		if ('(' != tk) {
			ThrowError( "Syntax Error" );
			return NULL;
		}
		tk = fLexer->GetNextTokenForParser();

		JavaScriptAST::Node *expr = ParseExpression( tk, false );
		if (!expr)	return NULL;

		if (')' != tk) {
			delete expr;
			ThrowError( "Syntax Error" );
			return NULL;
		}
		tk = fLexer->GetNextTokenForParser();

		JavaScriptAST::Node *statement = ParseStatement( tk );
		if (!statement) {
			delete expr;
			ThrowError( "Syntax Error" );
			return NULL;
		}

		return new JavaScriptAST::WhileStatementNode( expr, statement, lineNumber );
	} else if (JavaScriptTokenValues::FOR == tk) {
		tk = fLexer->GetNextTokenForParser();
		return ParseForStatement( tk );
	}

	return NULL;
}

JavaScriptAST::Node *JavaScriptParser::ParseForStatement( int &tk )
{
	// We've already parsed the FOR token, but since there are four different
	// variations of the for statement, I've moved this into its own production
	// All variants are similar in that there's a for ( XXXX ) Statement, the only
	// different between them being the XXXX tokens.  However, the tricky part is 
	// that the declarations seem to require multiple tokens of lookahead.  For 
	// instance: 
	//		ExpressionNoIn (opt);
	// is very similar to
	//		LeftHandSideExpression in Expression
	// since the ExpressionNoIn production includes the LeftHandSideExpression production.
	// So we are going to try to guess at the
	int lineNumber = fLexer->GetCurrentLineNumber();
	if ('(' != tk) {
		ThrowError( "Syntax Error" );
		return NULL;
	}
	tk = fLexer->GetNextTokenForParser();

	enum ForType {
		kVarNoIn,
		kVarWithIn,
		kExprNoIn,
		kExprWithIn,
		kInvalid,
	};

	ForType type = kInvalid;
	JavaScriptAST::Node *node = NULL;
	if (JavaScriptTokenValues::VAR == tk) {
		// Multiple tokens of lookahead strikes again.  We expect either a single variable followed by
		// an 'in' token, or a variable declaration list followed by a semi-colon.  Lovely, eh?  So we
		// are going to cheat -- we'll parse a declaration list for both cases, but pay attention to the
		// number of declarations parsed.  If it's more than one, we can require the semi-colon list, and
		// if it's only one, we can peek at the next token to see what's happening
		int numberOfDecls = 0;
		tk = fLexer->GetNextTokenForParser();
		node = ParseVariableDeclarationList( tk, true, &numberOfDecls );
		if (!node)	return NULL;

		if (numberOfDecls > 1) {
			type = kVarNoIn;
		} else if (';' == tk) {
			type = kVarNoIn;
		} else {
			type = kVarWithIn;
		}
	} else if (';' == tk) {
		type = kExprNoIn;
	} else if (node = ParseExpression( tk, true )) {
		// In order to determine the type of for statement, we need to look at the next
		// token to see whether it's an IN or a ;.  Once we have that, we can look to see
		// whether the token matches the expression we parsed.
		if (';' == tk) {
			type = kExprNoIn;
		} else if (JavaScriptTokenValues::KWORD_IN == tk) {
			if (node->IsLeftHandSideExpression()) {
				type = kExprWithIn;
			}
		} else if (fSuggestions && -1 == tk) {
			fSuggestions->Suggest( "in", SuggestionInfo::eKeyword );
		}
	} else if (fSuggestions && -1 == tk) {
		fSuggestions->Suggest( "var", SuggestionInfo::eKeyword );
	}

	// Now that we know which version of the for loop we're after, we can do some more straight-forward parsing
	if (kInvalid == type) {
		delete node;
		ThrowError( "Syntax Error" );
		return NULL;
	}
	if (kVarNoIn == type || kExprNoIn == type) {
		// We expect the next token we parse to be the semi-colon that separates the expressions.
		if (';' != tk) {
			delete node;
			ThrowError( "Syntax Error" );
			return NULL;
		}
		tk = fLexer->GetNextTokenForParser();

		// Now comes an optional expression followed by a semi-colon
		JavaScriptAST::Node *testExpr = NULL;
		if (';' != tk) {
			testExpr = ParseExpression( tk, false );
			if (!testExpr) {
				delete node;
				return NULL;
			}
		}

		// We expect the semi-colon again
		if (';' != tk) {
			delete testExpr;
			delete node;
			ThrowError( "Syntax Error" );
			return NULL;
		}
		tk = fLexer->GetNextTokenForParser();

		// Now comes the final optional expression, followed by a closing paren
		JavaScriptAST::Node *loopCounter = NULL;
		if (')' != tk) {
			loopCounter = ParseExpression( tk, false );
			if (!loopCounter) {
				delete node;
				delete testExpr;
				return NULL;
			}
		}

		node = new JavaScriptAST::ForExpressionTriClauseNode( node, testExpr, loopCounter, fLexer->GetCurrentLineNumber() );
	} else if (kVarWithIn == type || kExprWithIn == type) {
		// We expect the next token to be the 'in' keyword, followed by another
		// expression.
		if (JavaScriptTokenValues::KWORD_IN != tk) {
			if (fSuggestions && -1 == tk) {
				fSuggestions->Suggest( "in", SuggestionInfo::eKeyword );
			}
			delete node;
			ThrowError( "Syntax Error" );
			return NULL;
		}
		tk = fLexer->GetNextTokenForParser();

		JavaScriptAST::Node *inExpr = ParseExpression( tk, false );
		if (!inExpr) {
			delete node;
			return NULL;
		}

		node = new JavaScriptAST::ForExpressionInClauseNode( node, inExpr, fLexer->GetCurrentLineNumber() );
	}

	// Now we expect the closing paren
	if (')' != tk) {
		delete node;
		ThrowError( "Syntax Error" );
		return NULL;
	}
	tk = fLexer->GetNextTokenForParser();

	// Finally, we have to deal with the statement
	JavaScriptAST::Node *statement = ParseStatement( tk );
	if (!statement) {
		delete node;
		ThrowError( "Syntax Error" );
		return NULL;
	}
	return new JavaScriptAST::ForStatementNode( node, statement, lineNumber );
}

bool JavaScriptParser::IsContinueStatement( int tk )
{
	return (JavaScriptTokenValues::CONTINUE == tk);
}

JavaScriptAST::Node *JavaScriptParser::ParseContinueStatement( int &tk )
{
	if (JavaScriptTokenValues::CONTINUE != tk)	return NULL;
	int lineNumber = fLexer->GetCurrentLineNumber();

	// The next token *cannot* be a line terminator, so we need to ask the lexer
	// to care about line endings for this case
	tk = fLexer->GetNextTokenForParser( false );
	if (JavaScriptTokenValues::ENDL == tk) {
		// The effect here is that we automatically insert a semi-colon for the user
		tk = fLexer->GetNextTokenForParser();
		return new JavaScriptAST::ContinueStatementNode( NULL, lineNumber );
	}

	// We can have an optional identifier here
	JavaScriptAST::IdentifierNode *ident = NULL;
	if (JavaScriptTokenValues::IDENTIFIER == tk) {
		ident = new JavaScriptAST::IdentifierNode( fLexer->GetTokenText(), fLexer->GetCurrentLineNumber() );
		tk = fLexer->GetNextTokenForParser();
	} else if (fSuggestions && -1 == tk) {
		fSuggestions->SuggestJavaScriptIdentifiers();
	}

	if (!ParseStatementCompletion( tk )) {
		delete ident;
		return NULL;
	}
	return new JavaScriptAST::ContinueStatementNode( ident, lineNumber );
}

bool JavaScriptParser::IsBreakStatement( int tk )
{
	return (JavaScriptTokenValues::BREAK == tk);
}

JavaScriptAST::Node *JavaScriptParser::ParseBreakStatement( int &tk )
{
	if (JavaScriptTokenValues::BREAK != tk)	return NULL;
	int lineNumber = fLexer->GetCurrentLineNumber();

	// The next token *cannot* be a line terminator, so we need to ask the lexer
	// to care about line endings for this case
	tk = fLexer->GetNextTokenForParser( false );
	if (JavaScriptTokenValues::ENDL == tk) {
		// The effect here is that we automatically insert a semi-colon for the user
		tk = fLexer->GetNextTokenForParser();
		return new JavaScriptAST::BreakStatementNode( NULL, lineNumber );
	}

	// We can have an optional identifier here
	JavaScriptAST::IdentifierNode *ident = NULL;
	if (JavaScriptTokenValues::IDENTIFIER == tk) {
		ident = new JavaScriptAST::IdentifierNode( fLexer->GetTokenText(), fLexer->GetCurrentLineNumber() );
		tk = fLexer->GetNextTokenForParser();
	} else if (fSuggestions && -1 == tk) {
		fSuggestions->SuggestJavaScriptIdentifiers();
	}

	if (!ParseStatementCompletion( tk )) {
		delete ident;
		return NULL;
	}
	return new JavaScriptAST::BreakStatementNode( ident, lineNumber );
}

bool JavaScriptParser::IsDebuggerStatement( int tk )
{
	return (JavaScriptTokenValues::DEBUGGER == tk);
}

JavaScriptAST::Node *JavaScriptParser::ParseDebuggerStatement( int &tk )
{
	if (JavaScriptTokenValues::DEBUGGER != tk)	return NULL;
	int lineNumber = fLexer->GetCurrentLineNumber();

	// The next token *cannot* be a line terminator, so we need to ask the lexer
	// to care about line endings for this case
	tk = fLexer->GetNextTokenForParser( false );
	if (JavaScriptTokenValues::ENDL == tk) {
		// The effect here is that we automatically insert a semi-colon for the user
		tk = fLexer->GetNextTokenForParser();
		return new JavaScriptAST::DebuggerStatementNode( lineNumber );
	}

	return new JavaScriptAST::DebuggerStatementNode( lineNumber );
}

bool JavaScriptParser::IsReturnStatement( int tk )
{
	return (JavaScriptTokenValues::RETURN == tk);
}

JavaScriptAST::Node *JavaScriptParser::ParseReturnStatement( int &tk )
{
	if (JavaScriptTokenValues::RETURN != tk)	return NULL;
	int lineNumber = fLexer->GetCurrentLineNumber();

	// The next token *cannot* be a line terminator, so we need to ask the lexer
	// to care about line endings for this case
	tk = fLexer->GetNextTokenForParser( false );
	if (JavaScriptTokenValues::ENDL == tk) {
		// The effect here is that we automatically insert a semi-colon for the user
		tk = fLexer->GetNextTokenForParser();
		return new JavaScriptAST::ReturnStatementNode( NULL, lineNumber );
	}

	// We can have an optional expression here
	JavaScriptAST::Node *expr = ParseExpression( tk, false );

	if (!ParseStatementCompletion( tk )) {
		delete expr;
		return NULL;
	}
	return new JavaScriptAST::ReturnStatementNode( expr, lineNumber );
}

bool JavaScriptParser::IsWithStatement( int tk )
{
	return (JavaScriptTokenValues::WITH == tk);
}

JavaScriptAST::Node *JavaScriptParser::ParseWithStatement( int &tk )
{
	if (JavaScriptTokenValues::WITH != tk)	return NULL;
	int lineNumber = fLexer->GetCurrentLineNumber();
	tk = fLexer->GetNextTokenForParser();

	if ('(' != tk) {
		ThrowError( "Syntax Error" );
		return NULL;
	}
	tk = fLexer->GetNextTokenForParser();

	JavaScriptAST::Node *expr = ParseExpression( tk, false );
	if (!expr)	return NULL;

	if (')' != tk) {
		delete expr;
		ThrowError( "Syntax Error" );
		return NULL;
	}
	tk = fLexer->GetNextTokenForParser();

	JavaScriptAST::Node *statement = ParseStatement( tk );
	if (!statement) {
		delete expr;
		ThrowError( "Syntax Error" );
		return NULL;
	}
	return new JavaScriptAST::WithStatementNode( expr, statement, lineNumber );
}

JavaScriptAST::Node *JavaScriptParser::ParseLabeledStatement( int &tk )
{
	if (JavaScriptTokenValues::IDENTIFIER != tk)	return NULL;
	int lineNumber = fLexer->GetCurrentLineNumber();
	VString ident = fLexer->GetTokenText();
	tk = fLexer->GetNextTokenForParser();

	if (':' != tk) {
		ThrowError( "Syntax Error" );
		return NULL;
	}
	tk = fLexer->GetNextTokenForParser();

	JavaScriptAST::Node *statement = ParseStatement( tk );
	if (!statement) {
		ThrowError( "Syntax Error" );
		return NULL;
	}
	
	return new JavaScriptAST::LabeledStatementNode( ident, statement, lineNumber );
}

bool JavaScriptParser::IsSwitchStatement( int tk )
{
	return (JavaScriptTokenValues::SWITCH == tk);
}

JavaScriptAST::Node *JavaScriptParser::ParseSwitchStatement( int &tk )
{
	if (JavaScriptTokenValues::SWITCH != tk)	return NULL;
	int lineNumber = fLexer->GetCurrentLineNumber();
	tk = fLexer->GetNextTokenForParser();

	if ('(' != tk) {
		ThrowError( "Syntax Error" );
		return NULL;
	}
	tk = fLexer->GetNextTokenForParser();

	JavaScriptAST::Node *expr = ParseExpression( tk, false );
	if (!expr)	return NULL;
	
	if (')' != tk) {
		delete expr;
		ThrowError( "Syntax Error" );
		return NULL;
	}
	tk = fLexer->GetNextTokenForParser();

	JavaScriptAST::SwitchStatementNode *switchNode = new JavaScriptAST::SwitchStatementNode( expr, lineNumber );
	if (!ParseCaseBlock( tk, switchNode )) {
		delete switchNode;
		return NULL;
	}
	return switchNode;
}

bool JavaScriptParser::ParseCaseBlock( int &tk, JavaScriptAST::SwitchStatementNode *inSwitch )
{
	if ('{' != tk) {
		ThrowError( "Syntax Error" );
		return false;
	}
	tk = fLexer->GetNextTokenForParser();

	// The case clauses are optional
	bool hadCaseClauses = ParseCaseClauses( tk, inSwitch );

	// If there were no case clauses, we will require there to be a default clause
	JavaScriptAST::Node *defaultNode = ParseDefaultClause( tk );
	if (defaultNode) {
		inSwitch->AddCase( defaultNode );

		// There can be more case clauses after the default clause, if we had one
		hadCaseClauses |= ParseCaseClauses( tk, inSwitch );
	}

	if ('}' != tk) {
		ThrowError( "Syntax Error" );
		return false;
	}
	tk = fLexer->GetNextTokenForParser();

	return hadCaseClauses || (defaultNode != NULL);
}

bool JavaScriptParser::ParseCaseClauses( int &tk, JavaScriptAST::SwitchStatementNode *inSwitch )
{
	bool parsedCaseClause = false;
	JavaScriptAST::Node *caseClause = NULL;
	while (caseClause = ParseCaseClause( tk )) {
		inSwitch->AddCase( caseClause );
		parsedCaseClause = true;
	}
	return parsedCaseClause;
}

JavaScriptAST::Node *JavaScriptParser::ParseCaseClause( int &tk )
{
	if (fSuggestions && -1 == tk) {
		fSuggestions->Suggest( "case", SuggestionInfo::eKeyword );
	}

	if (JavaScriptTokenValues::CASE != tk)	return NULL;
	int lineNumber = fLexer->GetCurrentLineNumber();
	tk = fLexer->GetNextTokenForParser();
	
	JavaScriptAST::Node *expr = ParseExpression( tk, false );
	if (!expr)	return NULL;

	if (':' != tk) {
		delete expr;
		ThrowError( "Syntax Error" );
		return NULL;
	}
	tk = fLexer->GetNextTokenForParser();

	// The statement list is purely optional
	return new JavaScriptAST::CaseClauseNode( expr, ParseStatementList( tk ), lineNumber );
}

JavaScriptAST::Node *JavaScriptParser::ParseDefaultClause( int &tk )
{
	if (fSuggestions && -1 == tk) {
		fSuggestions->Suggest( "default", SuggestionInfo::eKeyword );
	}

	if (JavaScriptTokenValues::DEFAULT != tk)	return NULL;
	int lineNumber = fLexer->GetCurrentLineNumber();
	tk = fLexer->GetNextTokenForParser();
	
	if (':' != tk) {
		ThrowError( "Syntax Error" );
		return NULL;
	}
	tk = fLexer->GetNextTokenForParser();

	// The statement list is purely optional
	return new JavaScriptAST::DefaultClauseNode( ParseStatementList( tk ), lineNumber );
}

bool JavaScriptParser::IsThrowStatement( int tk )
{
	return (JavaScriptTokenValues::THROW == tk);
}

JavaScriptAST::Node *JavaScriptParser::ParseThrowStatement( int &tk )
{
	if (JavaScriptTokenValues::THROW != tk)	return NULL;
	int lineNumber = fLexer->GetCurrentLineNumber();

	// The next token *cannot* be a line terminator, so we need to ask the lexer
	// to care about line endings for this case
	tk = fLexer->GetNextTokenForParser( false );
	if (JavaScriptTokenValues::ENDL == tk) {
		// The effect here is that we automatically insert a semi-colon for the user
		tk = fLexer->GetNextTokenForParser();
		return new JavaScriptAST::ThrowStatementNode( NULL, lineNumber );
	}

	JavaScriptAST::Node *expr = ParseExpression( tk, false );
	if (!expr)	return NULL;

	if (!ParseStatementCompletion( tk )) {
		delete expr;
		return NULL;
	}
	return new JavaScriptAST::ThrowStatementNode( expr, lineNumber );
}

bool JavaScriptParser::IsTryStatement( int tk )
{
	return (JavaScriptTokenValues::TRY == tk);
}

JavaScriptAST::Node *JavaScriptParser::ParseTryStatement( int &tk )
{
	if (JavaScriptTokenValues::TRY != tk)	return NULL;
	int lineNumber = fLexer->GetCurrentLineNumber();
	tk = fLexer->GetNextTokenForParser();

	if (-1 == tk) {
		// We are out of tokens, which means this is not a legal try block.  So we
		// will throw an error and bail out early.
		ThrowError( "Syntax Error" );
		return NULL;
	}

	JavaScriptAST::Node *statements = ParseStatementBlock( tk );
	if (!statements)	return NULL;

	// We require either a catch by itself, a finally by itself, or a catch
	// followed by a finally.
	JavaScriptAST::Node *catchBlock = ParseCatch( tk );
	JavaScriptAST::Node *finallyBlock = ParseFinally( tk );

	if (!catchBlock && !finallyBlock) {
		delete statements;
		return NULL;
	}
	return new JavaScriptAST::TryStatementNode( statements, catchBlock, finallyBlock, lineNumber );
}

JavaScriptAST::Node *JavaScriptParser::ParseCatch( int &tk )
{
	if (fSuggestions && -1 == tk) {
		fSuggestions->Suggest( "catch", SuggestionInfo::eKeyword );
	}
	if (JavaScriptTokenValues::CATCH != tk)	return NULL;
	int lineNumber = fLexer->GetCurrentLineNumber();
	tk = fLexer->GetNextTokenForParser();

	if ('(' != tk) {
		ThrowError( "Syntax Error" );
		return NULL;
	}
	tk = fLexer->GetNextTokenForParser();

	if (JavaScriptTokenValues::IDENTIFIER != tk) {
		if (fSuggestions && -1 == tk) {
			fSuggestions->SuggestJavaScriptIdentifiers();
		}
		ThrowError( "Syntax Error" );
		return NULL;
	}

	JavaScriptAST::IdentifierNode *ident = new JavaScriptAST::IdentifierNode( fLexer->GetTokenText(), fLexer->GetCurrentLineNumber() );

	tk = fLexer->GetNextTokenForParser();

	if (')' != tk) {
		delete ident;
		ThrowError( "Syntax Error" );
		return NULL;
	}
	tk = fLexer->GetNextTokenForParser();

	JavaScriptAST::Node *statements = NULL;
	try {
		statements = ParseStatementBlock( tk );
	} catch (ParsingErrorException err) {
		if (err == kParsingError) {
			delete ident;
			throw;
		}
	}

	if (!statements) {
		delete ident;
		return NULL;
	}
	return new JavaScriptAST::CatchStatementNode( ident, statements, lineNumber );
}

JavaScriptAST::Node *JavaScriptParser::ParseFinally( int &tk )
{
	if (fSuggestions && -1 == tk) {
		fSuggestions->Suggest( "finally", SuggestionInfo::eKeyword );
	}
	if (JavaScriptTokenValues::FINALLY != tk)	return NULL;
	tk = fLexer->GetNextTokenForParser();

	return ParseStatementBlock( tk );
}

JavaScriptAST::Node *JavaScriptParser::ParseProgram( void )
{
	int tk = fLexer->GetNextTokenForParser();
	return new JavaScriptAST::ProgramNode( ParseSourceElements( tk ) );
}

JavaScriptAST::Node *JavaScriptParser::ParseSourceElements( int &tk )
{
	JavaScriptAST::StatementList *ret = NULL;
	bool keepGoing = true;

	// Due to error recovery, we assume that any statement or source
	// element can throw a ParsingErrorException.  We need to catch 
	// that exception if it's thrown so that we can attempt to recover
	// from the error and resume parsing from a safe place.
	do {
		try {
			JavaScriptAST::Node *element = ParseSourceElement( tk );
			if (element) {
				keepGoing = true;
				if (!ret)	ret = new JavaScriptAST::StatementList( fLexer->GetCurrentLineNumber() );
				ret->AddStatement( element );
			} else {
				if (ret)	ret->SetListCompletionLine( fLexer->GetCurrentLineNumber() );
				keepGoing = false;
			}
		} catch (ParsingErrorException) {
			RecoverFromError( tk );
			keepGoing = (-1 != tk);
		}
	} while (keepGoing);

	return ret;
}

JavaScriptAST::Node * JavaScriptParser::ParseSourceElement( int &tk )
{
	// We want the user to be able to generate block opener and closer
	// statements if we processed a multiline comment before this statement.
	int start, end;
	fLexer->ProcessedMultilineComment( start, end );

	// Also, we want to handle the case where we've hit a start or end region token
	if (fDelegate && fLexer->ProcessedRegionToken()) {
		bool isStart = false;
		VString name;
		int line = 0;
		fLexer->GetRegionInformation( isStart, name, line );
	}

	// There are two reasons that ParseStatement can return a NULL node -- the first is that there wasn't a
	// legal statement to parse and so we want to try to parse a function declaration.  The second was that
	// we were in the process of parsing a statement, but the statement wasn't completed yet.  In that case, we
	// DON'T want to try to parse a function declaration because then it will suggest "function" to the user,
	// even though that's not legal.  For instance, say you type in "try" but don't do the opening curly brace.
	// That's not a completed try statement, and we've hit the end of input.  So we return NULL from ParseStatement,
	// but then try to call ParseFunctionDeclaration, which obviously isn't correct.  So, if we hit a compile error
	// while processing the statement, then we won't bother trying to parse a function declaration.
	JavaScriptAST::Node *ret = ParseStatement( tk );
	if (!fWasError && !ret) ret = ParseFunctionDeclaration( tk );

	return ret;
}

bool JavaScriptParser::IsScriptDocElement()
{
	return fLexer->ProcessedScriptDocToken();
}

ScriptDocComment *JavaScriptParser::ParseScriptDocElement()
{
	// Now that we have the ScriptDoc comment, we can attach extra information to
	// the next symbol that follows.
	return ScriptDocComment::Create( fLexer->GetScriptDocText() );
}

#if _DEBUG
class SymbolTableAdapter
{
private:
	ISymbolTable *mTable;
	Symbols::IFile *mFile;

	void AddDefaultFile() {
		std::vector< Symbols::IFile * > files;
		files.push_back( mFile );
		xbox_assert( mTable->AddFiles( files ) );
	}

public:
	SymbolTableAdapter() { 
		mTable = gLanguageSyntax->CreateSymbolTable(); 
		VFile *temp = VFile::CreateTemporaryFile();
		xbox_assert( temp );

		// We don't want the temp file to actually *exist* because the database
		// will try to read it in as a .4db file.  So we delete it
		temp->Delete();

		xbox_assert( mTable->OpenSymbolDatabase( *temp ) );

		mFile = new VSymbolFile();
		VTime modTime;
		temp->GetTimeAttributes( &modTime );
		mFile->SetModificationTimestamp( modTime.GetStamp() );
		VString path;
		temp->GetPath( path );
		mFile->SetPath( path );
		mFile->SetBaseFolder( eSymbolFileBaseFolderProject );
		mFile->SetExecutionContext( eSymbolFileExecContextClientServer );

		AddDefaultFile();

		delete temp;
	}
	virtual ~SymbolTableAdapter() { mTable->Release(); mFile->Release(); }

	ISymbolTable *GetBase() const { return mTable; }
	Symbols::IFile *GetFile() const { return mFile; }

	void EmptyTable() {
		// We want to remove all of the symbols from the table -- we do that by simply removing
		// the only file the table references
		mTable->DeleteFile( mFile );

		// Then add the file back in
		AddDefaultFile();
	}

	size_t GetSymbolCount() {
		std::vector< Symbols::ISymbol * > syms;
		syms = mTable->GetNamedSubSymbols( NULL );

		size_t ret = syms.size();
		for (std::vector< Symbols::ISymbol * >::iterator iter = syms.begin(); iter != syms.end(); ++iter) (*iter)->Release();
		return ret;
	}

	Symbols::ISymbol *GetSymbol( size_t inIndex ) { 
		std::vector< Symbols::ISymbol * > syms;
		syms = mTable->GetNamedSubSymbols( NULL );
		Symbols::ISymbol *ret = NULL;
		if (inIndex < syms.size()) {
			ret = syms[ inIndex ];
			ret->Retain();
		}
		for (std::vector< Symbols::ISymbol * >::iterator iter = syms.begin(); iter != syms.end(); ++iter) (*iter)->Release();
		return ret;
	}

	Symbols::ISymbol *GetSymbolByName( XBOX::VString name ) {
		std::vector< Symbols::ISymbol * > syms;
		syms = mTable->GetNamedSubSymbols( NULL );

		Symbols::ISymbol *ret = NULL;
		for (std::vector< Symbols::ISymbol * >::iterator iter = syms.begin(); iter != syms.end(); ++iter) {
			XBOX::VString text = (*iter)->GetName();
			if (!ret && name.EqualToString( text, true )) {
				ret = *iter;
				ret->Retain();
			}
			(*iter)->Release();
		}
		return ret;
	}

	Symbols::ISymbol *GetSymbolByName( Symbols::ISymbol *inSym, VString inName ) {
		std::vector< Symbols::ISymbol * > syms;
		syms = mTable->GetNamedSubSymbols( inSym );

		Symbols::ISymbol *ret = NULL;
		for (std::vector< Symbols::ISymbol * >::iterator iter = syms.begin(); iter != syms.end(); ++iter) {
			XBOX::VString text = (*iter)->GetName();
			if (!ret && inName.EqualToString( text, true )) {
				ret = *iter;
				ret->Retain();
			}
			(*iter)->Release();
		}
		return ret;
	}

};

Symbols::ISymbol *GetFunctionPrototype( SymbolTableAdapter *table, Symbols::ISymbol *inFunc )
{
	
	xbox_assert( inFunc );
	xbox_assert( inFunc->IsFunctionKind() );

	Symbols::ISymbol *prototype = table->GetSymbolByName( inFunc, "prototype" );
	xbox_assert( prototype );
	return prototype;
}

void TestSubsymbolNames( SymbolTableAdapter *table, Symbols::ISymbol *owner, int count, ... )
{
	// TESTING NOTES:
	//
	//	If you ever want to debug why a test case is failing, there are some steps you can
	//	take to make it easier.  For starters, place a breakpoint on the line which checks
	//	the syntax or tokenizes your source string.  Then, run until you hit this breakpoint.
	//	At this point, you're about to run the unit test which is failing for you.  Go into
	//	JavaScriptParser.h and find the ThrowError method to place a breakpoint there.  When
	//	you hit run, the test will run until a syntax error is caught.  Now you can look at
	//	the call stack to see the calling order, and you can look at the fLexer property to
	//	inspect the last lexed token's text, and the line/character offset to the source code
	//	to pinpoint the problem area.  Obviously, this only works if you are experiencing an
	//	unexpected syntax error.  If you are expecting a syntax error and not getting one, you
	//	will have to step thru the parser by hand to locate where the issue might be.  Pay close
	//	attention to the fLexer state variables as they can at least keep you on track while in
	//	a complex production.
	xbox_assert( owner );

	std::vector< Symbols::ISymbol * > nodes = table->GetBase()->GetNamedSubSymbols( owner );

	long numberFound = 0;
	for (std::vector< Symbols::ISymbol * >::iterator 
			iter = nodes.begin(); iter != nodes.end(); ++iter) {

		VString name = (*iter)->GetName();
		
		va_list marker;
		va_start( marker, count );
		bool found = false;
		for (int i = 0; !found && i < count; i++) {
			if (name.EqualToString( va_arg( marker, char * ), true )) found = true;
		}
		va_end( marker );
		if (found)	numberFound++;
		(*iter)->Release();
	}

	xbox_assert( numberFound == count );
}
#endif // _DEBUG
