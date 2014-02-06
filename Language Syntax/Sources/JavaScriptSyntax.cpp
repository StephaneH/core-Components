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
#if _DEBUG
	#include "HTMLParser.h"
#endif

using namespace std;

#define kLockTag CVSTR("// @lock")
#define kLockTagLen	8
#define kStartOfBlockLockTag CVSTR("// @startlock")
#define kStartOfBlockLockTagLen 13
#define kEndOfBlockLockTag CVSTR("// @endlock")
#define kEndOfBlockLockTagLen 11

VJavaScriptSyntax::VJavaScriptSyntax(DisplayDocumentationSignal& inSignal):
fRequestDocumentationSignal(inSignal),
fSymTable(NULL),
fDeclarationParsingContextSymbol(NULL),
fDocumentFile(NULL),
fBreakPointManager(NULL)
{
	fAutoInsertBlock = false;
	fAutoInsertClosingChar = false;
	fAutoInsertTabs = false;
	fInsertSpaces = false;
	fTabWidth = 4;
	fActiveBreakPoints = true;
}

VJavaScriptSyntax::~VJavaScriptSyntax()
{
	if (fDocumentFile)	fDocumentFile->Release();
	if (fSymTable)	fSymTable->Release();
}

void VJavaScriptSyntax::CleanTokens( std::vector< class ILexerToken * > &inTokens )
{
	while (!inTokens.empty()) {
		inTokens.back()->SelfDelete();
		inTokens.pop_back();
	}
}

void VJavaScriptSyntax::Init( ICodeEditorDocument* inDocument )
{
	// init colors, will be most of the time overided by preferences but unless user
	// does not setup any preferences it will be kind to give him some colors
	// WARNING, please keep those settings synchronized with /Resources/default.waPreferences
		
	inDocument->SetStyle( 14, true,  false, false,   0,  77,   3 );		// keywords
	inDocument->SetStyle(  9, false, false, false,  83,  83,  83 );		// comments
	inDocument->SetStyle( 51, false, false, false,   0,  35,  83 );		// numbers
	inDocument->SetStyle( 52, true,  true,  false, 136,   0,   0 );		// strings
	inDocument->SetStyle( 53, false, false, false, 255,  49,   0 );		// names
	inDocument->SetStyle( 57, false, false, false, 128,  0,   255 );	// regular expressions

	if (fDocumentFile)	fDocumentFile->Release();
	fDocumentFile = NULL;

	fDeclarationParsingContextSymbol = NULL;
}

void VJavaScriptSyntax::Load( ICodeEditorDocument* inDocument, VString& ioContent )
{
}

void VJavaScriptSyntax::Save( ICodeEditorDocument* inDocument, VString& ioContent )
{
}

void VJavaScriptSyntax::SaveLine( ICodeEditorDocument* inDocument, sLONG inLineIndex, VString& ioContent )
{
	ICodeEditorDocument::LockStatus status = inDocument->GetProtectedStatus( inLineIndex );

	switch ( status )
	{
		case ICodeEditorDocument::ProtectedLine :
			ioContent += kLockTag;
			break;

		case ICodeEditorDocument::startOfProtectedBlock :
			ioContent += kStartOfBlockLockTag;
			break;

		case ICodeEditorDocument::endOfProtectedBlock :
			ioContent += kEndOfBlockLockTag;
			break;
			
		case ICodeEditorDocument::None:
			break;
	}
}

void VJavaScriptSyntax::Close( ICodeEditorDocument* inDocument )
{
}

bool VJavaScriptSyntax::DetermineMorphemeBoundary( ICodeEditorDocument *inDocument, sLONG inLineIndex, sLONG inOffset, sLONG &outLeftBoundary, sLONG &outLength, bool inAlternateKey )
{
	return false;
	/*
	// Initialize the output parameters, in case the caller didn't bother
	outLeftBoundary = outLength = 0;

	VString xstr;
	inDocument->GetLine( inLineIndex, xstr );

	// Tokenize that line
	TokenList tokens;
	JavaScriptLexer *lexer = new JavaScriptLexer();
	lexer->SetLexerInput( &xstr );
	lexer->Tokenize( tokens, false );
	delete lexer;

	// Now that we have the tokens, look for the one containing the offset
	ILexerToken *morpheme = NULL;
	for (TokenList::iterator iter = tokens.begin(); iter != tokens.end(); ++iter) {
		ILexerToken *token = *iter;
		if (token->GetPosition() <= inOffset && token->GetPosition() + token->GetLength() > inOffset) {
			// This token has semantic meaning, so we're already set
			morpheme = token;
			break;
		}
	}

	// Sanity check -- this really shouldn't happen, but if it does, we can't do much about it.
	if (!morpheme)	return true;

	// Now that we have the morpheme, we're set
	outLeftBoundary = morpheme->GetPosition();
	outLength = morpheme->GetLength();

	return true;
	*/
}

void VJavaScriptSyntax::GetColoringForTesting( const VString &inSourceLine, std::vector< size_t > &outOffsets, std::vector< size_t > &outLengths, std::vector< sLONG > &outStyles )
{
	// Tokenize that line
	vector< ILexerToken * > tokens;
	JavaScriptLexer *lexer = new JavaScriptLexer();
	lexer->SetLexerInput( const_cast< VString * >( &inSourceLine ) );
	lexer->Tokenize( tokens, false );
	delete lexer;

	// Iterate over the tokens, and ask the document to highlight 
	// them as needed.
	for (vector< ILexerToken * >::iterator iter = tokens.begin(); 
						iter != tokens.end(); iter++) {
		ILexerToken *current = *iter;
		sBYTE style = eNormal;
		switch (current->GetType()) {
			case ILexerToken::TT_OPEN_COMMENT:						style = eComment; break;
			case ILexerToken::TT_NUMBER:							style = eNumber; break;
			case ILexerToken::TT_OPEN_STRING:						style = eString; break;
			case ILexerToken::TT_STRING:							style = eString; break;
			case ILexerToken::TT_JAVASCRIPT_KEYWORD:				style = eKeyword; break;
			case ILexerToken::TT_JAVASCRIPT_FUTURE_RESERVED_WORD:	style = eKeyword; break;
			case ILexerToken::TT_COMPARISON:						style = eComparison; break;
			case ILexerToken::TT_OPEN_NAME:							style = eName; break;
			case ILexerToken::TT_NAME:								style = eName; break;
			case ILexerToken::TT_COMMENT:							style = eComment; break;
		}
		// Set the style for that token
		outOffsets.push_back( current->GetPosition() );
		outLengths.push_back( current->GetLength() );
		outStyles.push_back( style );
	}

	CleanTokens( tokens );
}

void VJavaScriptSyntax::GetTokensForTesting( VString& inSourceCode, std::vector<VString>& outTokens )
{
	// Tokenize that line
	std::vector< ILexerToken * >	tokens;
	
	JavaScriptLexer* lexer = new JavaScriptLexer();
	lexer->SetLexerInput( &inSourceCode);
	lexer->Tokenize( tokens);
	delete lexer;

	for (std::vector<ILexerToken*>::const_iterator it = tokens.begin(); it != tokens.end(); ++it)
		outTokens.push_back( (*it)->GetTypeString() );

	CleanTokens( tokens );
}

void VJavaScriptSyntax::GetDefinitionsForTesting( VString& inSelection, ISymbolTable *inSymTable, const VString &inFilePathStr, sLONG inLineNumber, std::vector<IDefinition>& outDefinitions )
{
	if (NULL != inSymTable)
	{
		ISymbolTable *backupSymTable = fSymTable;

		if (NULL != backupSymTable)
			backupSymTable->Retain();
		SetSymbolTable( inSymTable );

		VFilePath		filePath(inFilePathStr);
		VectorOfVString	parts;
	
		inSelection.GetSubStrings( CVSTR("."), parts);
		std::vector< Symbols::IFile * > files = inSymTable->RetainFilesByPathAndBaseFolder( filePath, eSymbolFileBaseFolderProject );
		if ( ! files.empty() )
		{
			Symbols::IFile *file = files.front();
			file->Retain();
			inSymTable->ReleaseFiles(files);

			if (NULL != file)
			{
				// Gets symbol corresponding to given parameters
				Symbols::ISymbol* owner = inSymTable->GetSymbolOwnerByLine(file, inLineNumber);
				GetDefinitions(parts, owner, file, outDefinitions);
				if (NULL != owner)
					owner->Release();
				file->Release();	
			}
		}

		SetSymbolTable( backupSymTable );
		if (NULL != backupSymTable)
			backupSymTable->Release();
	}
}

void VJavaScriptSyntax::GetASTForTesting(const XBOX::VString& inSourceLine, const XBOX::VString& inTypeOfASTComputationType, std::vector<XBOX::VString>& outDeclarations)
{
#if ACTIVATE_NEXT_JS_AST_WALKER
	JavaScriptAST::Visitor* declParser = JavaScriptAST::Visitor::GetDeclarationParser( NULL, NULL, NULL );
	if( declParser )
	{
		JavaScriptParser* parser = new JavaScriptParser();
		if( parser )
		{
			VFilePath filePath;
			bool errors;
			VString input = inSourceLine;
			JavaScriptAST::Node* program = parser->GetAST( filePath, &input, errors );
			if( program )
			{
				program->Accept( declParser );
				
				const JavaScriptAST::SymbolScopeDelaration* declarations = declParser->GetScopeDeclaration();
				if( declarations )
				{
					std::vector<JavaScriptAST::ISymbolDeclaration*> results;
					
					if( inTypeOfASTComputationType == "ASSIGNMENTS" )
						results = declarations->GetAssignmentDeclarations();
					else if( inTypeOfASTComputationType == "VARIABLES" )
						results = declarations->GetVariableDeclarations();
					else if( inTypeOfASTComputationType == "FUNCTION_DECLARATIONS" )
						results = declarations->GetFunctionDeclarations();
					
					for(std::vector<JavaScriptAST::ISymbolDeclaration*>::const_iterator it=results.begin(); it!=results.end(); ++it)
					{
						JavaScriptAST::ISymbolDeclaration* current = (*it);
						if( current )
						{
							XBOX::VString temp;
							current->ToString(temp);
							
							outDeclarations.push_back(temp);
						}
					}
				}
				
				delete program;
				program = NULL;
			}
			
			delete parser;
			parser = NULL;
		}
		
		delete declParser;
		declParser = NULL;
	}
#else
#endif
}

void VJavaScriptSyntax::SetLine( ICodeEditorDocument* inDocument, sLONG inLineNumber, bool inLoading )
{
	// Get the state of the previous line (assuming we're not the first line in the
	// document) to see whether it ended with an unfinished comment or string.  If
	// it does, then we want to tell the lexer about it so that it can resume lexing
	// the unfinish comment instead of assuming these are non-comment tokens.
	bool	previousLineEndsWithComment = false;
	bool	previousLineEndsWithString = false;
	sLONG	previousLineOpenStringType = 0;
	bool	openedCurlyBracket = false;

	// get previous line comment and open string states
	JavaScriptSyntaxLineKind previousLineKind = { 0 };
	if (inLineNumber > 0) {
		sLONG previousKind = inDocument->GetLineKind( inLineNumber - 1 );
		previousLineKind = *( (JavaScriptSyntaxLineKind*)&previousKind);

		previousLineEndsWithComment = previousLineKind.fEndsWithOpenComment ? true : false;
		previousLineOpenStringType = previousLineKind.fOpenStringType;
		previousLineEndsWithString = previousLineKind.fOpenStringType ? true : false;
	}

	// Aaron : you can make this line more clever to make executable only lines out of comments
	// Being executable for a line allows user to set breakpoints
	inDocument->SetExecutable( inLineNumber, true );

	// This line can no longer have a compile error since the user is modifying it.  If there
	// really is an error, we will catch it later
	inDocument->SetLineCompilerError( inLineNumber, CVSTR( "" ) );

	// Ask the document to give us the text for the given line
	VString xstr;
	inDocument->GetLine(inLineNumber,xstr);

	// check if the line is protected or not
	if ( inLoading )
	{
		ICodeEditorDocument::LockStatus status = ICodeEditorDocument::None;
		VString result;
		sLONG len = xstr.GetLength();

		if ( len >= kLockTagLen )
		{
			xstr.GetSubString( len - kLockTagLen + 1, kLockTagLen, result );
			if ( result == kLockTag )
			{
				xstr.Truncate( len - kLockTagLen );
				inDocument->SetLineText( inLineNumber, xstr );
				status = ICodeEditorDocument::ProtectedLine;
			}
			else if ( len >= kEndOfBlockLockTagLen )
			{
				xstr.GetSubString( len - kEndOfBlockLockTagLen + 1, kEndOfBlockLockTagLen, result );
				if ( result == kEndOfBlockLockTag )
				{
					xstr.Truncate( len - kEndOfBlockLockTagLen );
					inDocument->SetLineText( inLineNumber, xstr );
					status = ICodeEditorDocument::endOfProtectedBlock;
				}
				else if ( len >= kStartOfBlockLockTagLen )
				{
					xstr.GetSubString( len - kStartOfBlockLockTagLen + 1, kStartOfBlockLockTagLen, result );
					if ( result == kStartOfBlockLockTag )
					{
						xstr.Truncate( len - kStartOfBlockLockTagLen );
						inDocument->SetLineText( inLineNumber, xstr );
						status = ICodeEditorDocument::startOfProtectedBlock;
					}
				}
			}
		}

		inDocument->SetProtectedStatus( inLineNumber, status );
	}

	// Initialize the document line
	inDocument->SetLineStyle( inLineNumber, 0, xstr.GetLength(), eNormal );

	// Tokenize that line
	vector< ILexerToken * > tokens;
	JavaScriptLexer *lexer = new JavaScriptLexer();
	lexer->SetLexerInput( &xstr );
	lexer->Tokenize( tokens, previousLineEndsWithComment, previousLineOpenStringType);
	delete lexer;

	// Iterate over the tokens, and ask the document to highlight 
	// them as needed.
	JavaScriptSyntaxLineKind kind = { 0 };

	for (vector< ILexerToken * >::iterator iter = tokens.begin(); iter != tokens.end(); iter++)
	{
		ILexerToken *current = *iter;
		sBYTE style = eNormal;
		switch (current->GetType()) {
			case ILexerToken::TT_NUMBER:							style = eNumber; break;
			case ILexerToken::TT_STRING:							style = eString; break;
			case ILexerToken::TT_JAVASCRIPT_FUTURE_RESERVED_WORD:	style = eKeyword; break;
			case ILexerToken::TT_COMPARISON:						style = eComparison; break;
			case ILexerToken::TT_OPEN_NAME:							style = eName; break;
			case ILexerToken::TT_NAME:								style = eName; break;
			case ILexerToken::TT_JAVASCRIPT_KEYWORD:				style = eKeyword; break;
			case ILexerToken::TT_JAVASCRIPT_REGEXP:					style = eRegExp; break;
			case ILexerToken::TT_OPEN_STRING:
				{
					if (current->GetValue() == JavaScriptTokenValues::STRING_APOSTROPHE)
						kind.fOpenStringType = JavaScriptStringType::kApostropheString;
					else if (current->GetValue() == JavaScriptTokenValues::STRING_QUOTATION_MARK)
						kind.fOpenStringType = JavaScriptStringType::kQuoteString;
					style = eString; 
				}
				break;
			case ILexerToken::TT_OPEN_COMMENT:
			{
				style = eComment;
				if( !previousLineKind.fEndsWithOpenComment )
					kind.fLineKindStartBlocks++;
			}
				break;
			case ILexerToken::TT_COMMENT:
				{
					style = eComment; 
					// We also have to special case when the user types // @region or // @endregion.  Note that we are
					// not handling whitespace at all for this.  It has to be exact.
					if (current->GetText().BeginsWith( CVSTR("// @region") ))
					{
						if ( inLoading )
						{
							inDocument->SetFoldable( inLineNumber, true );
							inDocument->SetFolded( inLineNumber, true );
						}
						kind.fLineKindStartBlocks++;

					} 
					else if (current->GetText().BeginsWith( CVSTR("// @endregio") ))
					{	
						// It's not a typo, it's working around a lexer oddity that I'm not in the mood to fix.
						kind.fLineKindEndsBlocks++;
					}
					else if( current->GetText().BeginsWith( CVSTR("/") ) )
					{
						kind.fIsSingleLineComment = 1;
					}
					else if( current->GetText().EndsWith( CVSTR("*/") ) )
					{
						kind.fLineKindEndsBlocks++;
					}
				}
				break;
			case ILexerToken::TT_PUNCTUATION:
				{
					if (current->GetValue() == CHAR_LEFT_CURLY_BRACKET)
					{
						kind.fLineKindStartBlocks++;
						openedCurlyBracket = true;
					}
					else if (current->GetValue() == CHAR_RIGHT_CURLY_BRACKET)
					{
						if ( openedCurlyBracket )
						{
							openedCurlyBracket = false;
							kind.fLineKindStartBlocks--;
						}
						else
							kind.fLineKindEndsBlocks++;
					}
				} 
				break;
		}
		// Set the style for that token
		inDocument->SetLineStyle( inLineNumber, current->GetPosition(), 
			current->GetPosition() + current->GetLength(), style );
	}

	// Check to see if the last token on the line is an open comment.  This is a
	// special case that we need to track to support multi-line comments.  When
	// tokenizing a line, we will look at the previous line's state to see if it
	// ends in an opened comment.  If it does, then we will alert the lexer that
	// this token stream is assumed to start with a comment so that it can continue
	// to lex appropriately.
	//
	// Note that if we have no tokens on this line, we automatically assume the state 
	// of the line before this one.
	bool endsWithOpenComment = (!tokens.empty()) ? (tokens.back()->GetType() == ILexerToken::TT_OPEN_COMMENT) : (previousLineEndsWithComment);
	bool endsWithOpenString = (!tokens.empty()) ? (tokens.back()->GetType() == ILexerToken::TT_OPEN_STRING) : (previousLineEndsWithString);
	
	kind.fEndsWithOpenComment = endsWithOpenComment ? 1 : 0;
	
	bool previousOpenCommentState = false;
	bool previousOpenStringState  = false;

	// Get current line previous open string or comment state
	if ( ! inLoading)
	{
		sLONG lCurrentLineKind = inDocument->GetLineKind(inLineNumber);
		JavaScriptSyntaxLineKind currentLineKind = *( (JavaScriptSyntaxLineKind *) &lCurrentLineKind);
		previousOpenCommentState = currentLineKind.fEndsWithOpenComment ? true : false;
		previousOpenStringState  = currentLineKind.fOpenStringType ? true : false;
	}

	inDocument->SetLineKind( inLineNumber, *((sLONG*)&kind) );
	
	CleanTokens( tokens );

	if ( ! inLoading)
	{
		// There are four cases we really need to care about.  If the line now ends in
		// an open comment (and didn't used to), we want to colorize down the document.
		// Also, if the line no longer ends in an open comment (but used to), we want to
		// colorize down the document.  In either case, we want to keep colorizing subsequent
		// lines until the comment is ended or the end of the document is reached. The same
		// reasoning applies to open string.
		if ( ( !previousOpenCommentState && endsWithOpenComment ||	// Now ends with open comment, didn't used to
			previousOpenCommentState && !endsWithOpenComment ||	// Used to end with an open comment, but no longer does
			!previousOpenStringState && endsWithOpenString   ||	// Now ends with open string, didn't used to
			previousOpenStringState && !endsWithOpenString ) &&	// Used to end with an open string, but no longer does
			inLineNumber + 1 < inDocument->GetNbLines() )
		{
			SetLine( inDocument, inLineNumber + 1, inLoading );
		}
	}
}

sLONG VJavaScriptSyntax::GetIndentWidth()
{
	return 0;
}

bool VJavaScriptSyntax::CheckFolding(ICodeEditorDocument* inDocument, sLONG inLineNumber)
{
	int kind = inDocument->GetLineKind( inLineNumber );
	return kind != 0;
}

void VJavaScriptSyntax::ComputeFolding( ICodeEditorDocument *inDocument )
{
	std::vector< sLONG > indentStack;
	std::vector< sLONG > commentStack;

	for (sLONG i = 0; i < inDocument->GetNbLines(); i++) {
		// Reset the line's indent information, since we're recalculating it on the fly
		inDocument->SetLineIndent( i, 0 );
		inDocument->SetFoldable( i, false );

		// Then, get the line kind so that we can determine how to handle it
		sLONG kind = inDocument->GetLineKind( i );
		JavaScriptSyntaxLineKind lineKind = *( (JavaScriptSyntaxLineKind*)&kind );

		if ( lineKind.fLineKindEndsBlocks )
		{
			// Now we go back an indentation level for each end block on the current line
			for (int count = 0; count < lineKind.fLineKindEndsBlocks; count++)
			{
				if (indentStack.empty())
					break;		// Sanity check

				// Get the last block opener from the stack
				sLONG opener = indentStack.back();
				indentStack.pop_back();

				// The opener and the closer are on the same level, but everything between them
				// is actually set to the current indent level
				for (sLONG j = opener + 1; j < i; j++) { if (j < inDocument->GetNbLines()) {
						inDocument->SetLineIndent( j, inDocument->GetLineIndent( j ) + 1);
					}
				}

				if (opener < inDocument->GetNbLines()) {

					sLONG toFold = i - opener - ( lineKind.fLineKindStartBlocks ? 1 : 0 );

					if ( toFold > 0 )
					{
						// The opener is the foldable line
						inDocument->SetFoldable( opener, true );

						// Tell the document how many lines are foldable into this group
						inDocument->SetNbFoldedLines( opener, toFold );
					}
				}
			}
		}

		if ( lineKind.fLineKindStartBlocks )
		{
			for (int count = 0; count < lineKind.fLineKindStartBlocks; count++)
				indentStack.push_back( i );
		}
		
		// Single Line Comments folding management
		if( lineKind.fIsSingleLineComment )
			commentStack.push_back(i);
		
		if( !lineKind.fIsSingleLineComment ||
			(lineKind.fIsSingleLineComment && (i==inDocument->GetNbLines()-1)) )
		{
			if( commentStack.size() > 1 )
			{
				// Set the first line of a single line comment block as the foldable line
				sLONG start = commentStack[0];
				inDocument->SetFoldable(start, true);
				
				// Compute the number of foldable line
				// Be aware that if we are on the last line AND if the last line is a single line comment
				// then we must decrementby one this number
				sLONG foldedLinesCount = i - start - ( !lineKind.fIsSingleLineComment ? 1 : 0 );
				inDocument->SetNbFoldedLines(start, foldedLinesCount);

				// Compute the line ident of the single line comments block in order to have the vertical line
				// drawn on the left of the block
				sLONG stop = start + commentStack.size()-1;
				for(sLONG index=start+1; index<stop; index++)
					inDocument->SetLineIndent(index, inDocument->GetLineIndent(index)+1);
			}
			
			commentStack.clear();
		}
	}
}

bool VJavaScriptSyntax::CheckOutline(ICodeEditorDocument* inDocument, sLONG inLineNumber)
{
	return false;
}

void VJavaScriptSyntax::ComputeOutline( ICodeEditorDocument* inDocument )
{
}

void VJavaScriptSyntax::TokenizeLine( ICodeEditorDocument* inDocument, sLONG inLineIndex, std::vector<XBOX::VString>& ioLines )
{
/*	if (!fSymTable)	return;

	// This is a cheap way for us to update our declaration table, however, it's also 
	// incorrect.  The problem is that we're not capturing the context information 
	// such as functions within functions, by declaration parsing just this line.  But
	// this is a good first pass.  We should re-parse the entire file at some point to
	// ensure the context stays in sync.
	VString source;
	for (std::vector< VString >::iterator iter = ioLines.begin(); iter != ioLines.end(); ++iter) {
		source.AppendString( *iter );
	}

	Symbols::IFile *file = FileFromDocument( inDocument );
	fDeclarationParsingContextSymbol = fSymTable->GetSymbolOwnerByLine( file, inLineIndex );

	// We are parsing the declarations to see if there's anything new to add to the symbol
	// table, but all of the entries are being added to the root level.  However, this can
	// have incorrect behavior, so I am going to rely on the fact that the document
	// should be syncing the declarations periodically by reparsing the entire file.
	JavaScriptParser *parser = new JavaScriptParser;
	parser->ParseDeclarations( &source, fSymTable, file, inDocument );
	delete parser;

	file->Release();
*/
}

Symbols::ISymbol *VJavaScriptSyntax::LocateSymbol( Symbols::ISymbol *inContext, const VString &inText )
{
	if (!fSymTable)	return NULL;	// This shouldn't happen, but perhaps the DB couldn't be opened

	// Break the string up into sections based on the dot operator.  Each section correlates to 
	// a symbol in the symbol table.
	std::vector< Symbols::ISymbol * >	syms;
	VectorOfVString						parts;
	Symbols::ISymbol					*sym = NULL;
	
	if (inText.GetSubStrings( (UniChar)CHAR_FULL_STOP, parts )) 
	{
		// Iterate over the parts in the string to find the symbol in the table
		if (parts.front().EqualToString( CVSTR("this"), true ))
		{
			syms = fSymTable->RetainSymbolsByName( inContext, CVSTR("prototype"), inContext ? inContext->GetFile() : NULL );
			if (!syms.empty())

				sym = syms.front();
		} 
		else
		{
			syms = fSymTable->RetainSymbolsByName( inContext, parts.front() );
			if (!syms.empty())	sym = syms.front();
		}
		
		if (sym)
			sym->Retain();

		fSymTable->ReleaseSymbols(syms);

		VectorOfVString::iterator partIter = parts.begin();
		
		++partIter;

		for (; sym && partIter != parts.end(); ++partIter)
		{
			Symbols::ISymbol *original = sym;
			original->Retain();

			std::vector< Symbols::ISymbol * > lookup_list;

			lookup_list.push_back( const_cast< Symbols::ISymbol * >( sym->RetainReferencedSymbol() ) );

			while (!lookup_list.empty())
			{
				Symbols::ISymbol *lookup = lookup_list.back();
				lookup_list.pop_back();

				// See if we can find the symbol with the proper name
				syms = fSymTable->RetainSymbolsByName( lookup, *partIter );

				Symbols::ISymbol *ret = NULL;
				for (std::vector< Symbols::ISymbol * >::iterator symIter = syms.begin(); symIter != syms.end(); ++symIter)
				{
					// We only care about public properties, since this is a qualified lookup
					if (!ret && ( (*symIter)->GetKind() == Symbols::ISymbol::kKindPublicProperty ||
						 (*symIter)->GetKind() == Symbols::ISymbol::kKindStaticProperty) )
					{
						// We found the one we want to return
						ret = (*symIter);
					}
					else
					{
						(*symIter)->Release();
					}
				}

				// If we already found a member, we're done
				if (ret)
				{
					sym = ret;
					// Clear out the prototype list, since we found what we are looking for
					lookup_list.clear();
					break;
				}
				
				// get object prototype child
				std::vector< Symbols::ISymbol * > prototypesChildren = fSymTable->RetainSymbolsByName( lookup, CVSTR("prototype") );
				if ( ! prototypesChildren.empty() )
					lookup_list.insert( lookup_list.end(), prototypesChildren.begin(), prototypesChildren.end() );

				// Otherwise, move up the prototype chain
				lookup_list.insert( lookup_list.end(), lookup->GetPrototypes().begin(), lookup->GetPrototypes().end() );
			}

			if (original)
			{
				if (original == sym)
				{	
					sym->Release();
					sym = NULL;
				}	
				original->Release();
			}
		}
	}
	else
	{
		// We didn't find any dots in the name, so we only have one symbol to find
		if (inText.EqualToString( CVSTR("this"), true ))
		{
			sym = inContext;
			if (sym)
				sym->Retain();
		}
		else
		{
			syms = fSymTable->RetainSymbolsByName( inContext, inText );
			if (!syms.empty())	sym = syms.front();
			if (sym)
				sym->Retain();

			fSymTable->ReleaseSymbols(syms);
		}
	}

	return sym;
}

void VJavaScriptSyntax::GetTip( ICodeEditorDocument* inDocument, sLONG inLine, sLONG inPos, VString& outText, Boolean& outRtfText )
{
	// Given the information we have, we need to determine what morpheme the user is hovering
	// over so that we can look the information up in the symbol table.
	VString xstr;
	inDocument->GetLine( inLine, xstr );

	// We will tokenize the entire line so that we can walk through the tokens, looking for the
	// one the user is hovering over
	TokenList tokens;
	JavaScriptLexer *lexer = new JavaScriptLexer;
	lexer->SetLexerInput( &xstr );
	lexer->Tokenize( tokens );
	delete lexer;

	VString	lastTokenText, lastTokenValue;
	bool	lastTokenIsIdentifier = false;

	for (vector< ILexerToken * >::reverse_iterator iter = tokens.rbegin(); iter != tokens.rend(); ++iter) {
		if ((*iter)->GetPosition() <= inPos && (*iter)->GetPosition() + (*iter)->GetLength() >= inPos) {
			vector< VString > parts;
			lastTokenIsIdentifier = ((*iter)->GetType() == ILexerToken::TT_NAME || 
				(*iter)->GetValue() == JavaScriptTokenValues::KWORD_THIS);
			do {
				parts.push_back( (*iter)->GetText() );
				++iter;
				if (iter == tokens.rend() || (*iter)->GetValue() != '.')	break;
				parts.push_back( "." );
				++iter;
			} while (iter != tokens.rend() && ((*iter)->GetType() == ILexerToken::TT_NAME ||
				(*iter)->GetValue() == JavaScriptTokenValues::KWORD_THIS));
			for (vector< VString >::reverse_iterator strIter = parts.rbegin(); strIter != parts.rend(); ++strIter)	lastTokenText += *strIter;
			break;
		}
	}

	if ( lastTokenIsIdentifier )
	{
		// TODO: We don't support RTF text currently
		outRtfText = false;

		// If we've got a debugger we can try to evaluate the selected expression
		IDebuggerClient* debuggerClient = inDocument->GetDebuggerClient();
		if (NULL != debuggerClient)
		{
			debuggerClient->Evaluate(lastTokenText, lastTokenValue);
			if ( ! lastTokenValue.IsEmpty() )
				outText = lastTokenText + CVSTR(": ") + lastTokenValue;
		}
		else if ( NULL != fSymTable )
		{
			Symbols::IFile *file = FileFromDocument( inDocument );
			if ( file )
			{
				Symbols::ISymbol *contextSymbol = fSymTable->GetSymbolOwnerByLine( file, inLine );
				file->Release();

				// See if we can locate the symbol in the symbol table
				Symbols::ISymbol *sym = LocateSymbol( contextSymbol, lastTokenText );

				if (!sym && contextSymbol && contextSymbol->GetName() == lastTokenText)
				{
					sym = contextSymbol;
					contextSymbol = NULL;
				}

				if (sym)  
				{
					// We found a symbol, so now we want to see if it has a ScriptDoc tag associated
					// with it.  If it does, then we have a lot of good information to be able to
					// show the user.
					const Symbols::ISymbol *temp = sym->RetainReferencedSymbol();
					
					ScriptDocComment *comment = ScriptDocComment::Create( temp->GetScriptDocComment() );
					temp->Release();
					if (comment) 
					{
						// We have a ScriptDoc comment, so let's grab the information out of it and
						// format it in a friendly way for the user.
						fSymTable->GetSymbolSignature(sym, outText);
						outText += (UniChar)CHAR_CONTROL_000D;
						sLONG count = comment->ElementCount();
						for (sLONG i = 0; i < count; i++) {
							outText += comment->GetElement( i )->FormatTextForDisplay();
							outText += (UniChar)CHAR_CONTROL_000D;
						}
						comment->Release();
					} 
					else
					{
						Symbols::ISymbol* assignedSym = NULL;

						VString signature = sym->GetName();

						if ( ! sym->IsFunctionKind() )
							assignedSym = fSymTable->GetAssignedFunction(sym);

						if (NULL != assignedSym)
						{
							sym->Release();
							sym = assignedSym;
						}

						fSymTable->GetSymbolSignature(sym, signature);
						if ( signature != sym->GetName() )
							outText = signature;
					}

					sym->Release();
				}

				if (NULL != contextSymbol)
					contextSymbol->Release();
			}
		}
	}

	CleanTokens( tokens );
}

void VJavaScriptSyntax::GetDefinitionsSelection(ICodeEditorDocument* inDocument, VString& outSymbolName, std::vector<VString>& outParts)
{
	sLONG	startSelection, endSelection, firstLine, firstVisibleLine, lastVisibleLine, pos;
	VString	line, selection; 

	// First we have to get text to define: it can be selected or just pointed by cursor
	inDocument->GetSelection(startSelection, endSelection, firstVisibleLine, lastVisibleLine);
	firstLine = inDocument->GetLineIndex(firstVisibleLine);
	inDocument->GetLine(firstLine, line);
	inDocument->GetSelectedText(selection);
	pos = selection.IsEmpty() ? endSelection : endSelection > startSelection ? endSelection - selection.GetLength() : startSelection - selection.GetLength();

	GetDefinitionsSelection( inDocument, line, selection, pos, outSymbolName, outParts );
}

void VJavaScriptSyntax::GetDefinitionsSelection(ICodeEditorDocument* inDocument, VString& inLine, const VString& inSelection, sLONG& ioPos, VString& outSymbolName, std::vector<VString>& outParts)
{

	// Then we have to tokenize the entire line so that we can walk through the tokens, looking for the
	// one the user is looking for
	VectorOfVString	parts;
	TokenList		tokens;
	bool			lastTokenIsIdentifier = false;

	JavaScriptLexer *lexer = new JavaScriptLexer;
	lexer->SetLexerInput( &inLine );
	lexer->Tokenize( tokens );
	delete lexer;

	for (vector< ILexerToken * >::reverse_iterator iter = tokens.rbegin(); iter != tokens.rend(); ++iter)
	{
		if ((*iter)->GetPosition() <= ioPos && (*iter)->GetPosition() + (*iter)->GetLength() >= ioPos)
		{

			lastTokenIsIdentifier = ((*iter)->GetType() == ILexerToken::TT_NAME || (*iter)->GetValue() == JavaScriptTokenValues::KWORD_THIS);
			do {
				parts.push_back( (*iter)->GetText() );
				ioPos = (*iter)->GetPosition();
				++iter;
				if (iter == tokens.rend() || (*iter)->GetValue() != '.')
					break;
				++iter;
			} 
			while (iter != tokens.rend() && (*iter)->GetType() == ILexerToken::TT_NAME || (*iter)->GetValue() == JavaScriptTokenValues::KWORD_THIS);
			break;
		}
	}

	if (lastTokenIsIdentifier)
	{

		// Reverse the selection while we will search symbol definition in a descendant way
		// and replace last element with selection (splitted or not) if needed
		reverse(parts.begin(), parts.end()); 
		if ( inSelection.GetLength() )
		{
			VectorOfVString selectionTab;

			outSymbolName = inSelection;
			inSelection.GetSubStrings( CVSTR("."), selectionTab);
			if ( ! parts.empty() )
				parts.pop_back();
			parts.insert(parts.end(), selectionTab.begin(), selectionTab.end());

		}
		else
		{	
			sLONG count = 0;
			for (VectorOfVString::const_iterator it = parts.begin(); it != parts.end(); ++it, count++)
			{
				if (count)
					outSymbolName += CVSTR(".");
				outSymbolName += *it;
			}
		}

	}

	outParts = parts;

	CleanTokens( tokens );
}

bool VJavaScriptSyntax::CanGotoDefinition( ICodeEditorDocument* inDocument, sLONG inLineIndex, sLONG inOffset, sLONG& outStartOffset, sLONG& outEndOffset )
{
	VString	line, empty, symbolName; 
	sLONG pos = inOffset;
	inDocument->GetLine(inLineIndex, line);
	std::vector<VString> parts;
	GetDefinitionsSelection( inDocument, line, empty, pos, symbolName, parts );
	if ( ! symbolName.IsEmpty() )
	{
		outStartOffset = pos;
		outEndOffset = pos + symbolName.GetLength();
		std::vector<IDefinition> definitions;
		GetDefinitions( inDocument, inLineIndex, parts, definitions );
		return definitions.size() > 0;
	}
	else
		return false;
}

void VJavaScriptSyntax::GetDefinitions(ICodeEditorDocument* inDocument, VString& outSymbolName, std::vector<IDefinition> & outDefinitions)
{
	sLONG startSelection, endSelection, firstVisibleLine, lastVisibleLine;
	inDocument->GetSelection(startSelection, endSelection, firstVisibleLine, lastVisibleLine);
	sLONG lineIndex = inDocument->GetLineIndex(lastVisibleLine);

	std::vector<VString>	parts;
	GetDefinitionsSelection(inDocument, outSymbolName, parts);
	GetDefinitions(inDocument, lineIndex, parts, outDefinitions );
}

void VJavaScriptSyntax::GetDefinitions( ICodeEditorDocument* inDocument, sLONG inLineIndex, std::vector<VString>& inParts, std::vector<IDefinition>& outDefinitions ) 
{
	if ( inParts.size() )
	{
		// First we'll try to find a unique symbol in the document file extending the line
		// context from bottom to top until we reach the global context (which is exclude 
		// from search for now)
		Symbols::IFile *file = FileFromDocument( inDocument );
		if ( file )
		{
			Symbols::ISymbol *contextSymbol = inDocument->GetSymbolTable()->GetSymbolOwnerByLine( file, inLineIndex );

			// Kludge to handle the "ds.someModel.someMethod lookup in "guided" and "free" mode. As we are
			// not aware of javascript mode ("guided" or "free") and method kind ("class", "entity" or
			// "collection") we format 6 vectors of strings containing the potential paths to symbol and 
			// return the first results we found
			if (inParts.front() == CVSTR("ds") && inParts.size() > 2)
			{
				VectorOfVString models, methodKinds;

				models.push_back( CVSTR("guidedModel") );	// guided mode
				models.push_back( CVSTR("model") );			// free mode

				methodKinds.push_back( CVSTR("methods") );				// class methods
				methodKinds.push_back( CVSTR("entityMethods") );		// entity methods
				methodKinds.push_back( CVSTR("collectionMethods") );	// collection methods

				for (VectorOfVString::iterator itModels = models.begin(); itModels != models.end() && !outDefinitions.size(); ++itModels)
				{
					for (VectorOfVString::iterator itMethodKinds = methodKinds.begin(); itMethodKinds != methodKinds.end() && !outDefinitions.size(); ++itMethodKinds)
					{
						VectorOfVString hackedParts = inParts;

						hackedParts[0] = *itModels;
						hackedParts.insert( hackedParts.begin() + 2, *itMethodKinds);
						GetDefinitions(hackedParts, contextSymbol, file, outDefinitions);	
					}
				}
			}
			// Standard case
			else
			{
				GetDefinitions(inParts, contextSymbol, file, outDefinitions);	
			}
			file->Release();

			if (NULL != contextSymbol)
				contextSymbol->Release();
		}
	}
}

void VJavaScriptSyntax::GetDefinitions(VectorOfVString& inParts, std::vector<IDefinition> & outDefinitions)
{
	GetDefinitions(inParts, 0, 0, outDefinitions);
}

void VJavaScriptSyntax::GetDefinitions(VectorOfVString& inParts, Symbols::ISymbol *inContextSymbol, Symbols::IFile *inFile, std::vector<IDefinition> & outDefinitions)
{
	Symbols::ISymbol *sym = NULL;
	Symbols::ISymbol *contextSymbol = inContextSymbol;

	do
	{
		sym = FindNamedDescendant(contextSymbol, inFile, inParts, 0);
		if (NULL != contextSymbol)
		{
			Symbols::ISymbol *tmp = contextSymbol->GetOwner();
			contextSymbol = tmp;
		}
	}
	while (!sym && contextSymbol);

	// If a symbol has been found in the primary context, we suppose this is 
	// the right one and add it to our definitions list. Else we will try to 
	// find one or several symbols in global context extending search to
	// every files
	if (NULL != sym)
	{
		outDefinitions.push_back( IDefinition(sym) );
		if (sym != inContextSymbol)
			sym->Release();
	}
	else
	{
		// We seach every symbol matching our symbol name in global context and every files
		std::vector<Symbols::ISymbol *> syms = fSymTable->RetainSymbolsByName(0, inParts[0], 0);
		for (std::vector< Symbols::ISymbol * >::const_iterator it = syms.begin(); it != syms.end(); ++it)
		{
			Symbols::ISymbol *sym = NULL;

			if (inParts.size() == 1)
				sym = *it;
			else
				sym = FindNamedDescendant( (*it), 0, inParts, 1);

			if (NULL != sym)
			{
				// Exclude symbols that aren't from a project file (JSF, WAF Library).
				if (sym->GetFile()->GetBaseFolder() == eSymbolFileBaseFolderProject)
				{
					// Special case when found symbol belongs to model file: we 
					// verify that we are in server context and that first part
					// of lineage is  "ds" to accept this symbol
					if ( ! sym->GetFile()->GetPath().EndsWith( CVSTR(".js") ) )
						if ( sym->GetFile()->GetExecutionContext() != eSymbolFileExecContextServer)
							if ( inParts[0] != CVSTR("ds") )
								continue;

					// Exclude the symbols not owning to server context when file is NULL (EM Editor context)
					if (!inFile && sym->GetFile()->GetExecutionContext() == eSymbolFileExecContextClientServer)
						outDefinitions.push_back( IDefinition(sym) );
					// Exclude the symbols not matching the document file context
					else if ( inFile->GetExecutionContext() == eSymbolFileExecContextClientServer ||
						sym->GetFile()->GetExecutionContext() == eSymbolFileExecContextClientServer ||
						( inFile->GetExecutionContext() == sym->GetFile()->GetExecutionContext() ) )
						outDefinitions.push_back( IDefinition(sym) );
				}
				if (sym != *it)
					sym->Release();
			}
			(*it)->Release();
		}
	}
}

Symbols::ISymbol* VJavaScriptSyntax::FindNamedDescendant( Symbols::ISymbol* inOwner, Symbols::IFile *inFile, VectorOfVString& inSymbolLineage, sLONG inLevel, bool inKeepUndefined)
{	
	Symbols::ISymbol* sym = NULL;

	if ( ! inLevel && inSymbolLineage[inLevel] == CVSTR("this") )
	{
		return FindNamedDescendant(inOwner, inFile, inSymbolLineage, inLevel + 1, inKeepUndefined);
	}

	if ( inLevel + 1 <= inSymbolLineage.size() )
	{
		// First part: verify that owner is not the expected symbol
		if (inOwner && inOwner->GetName() == inSymbolLineage[inLevel] )
			if ( inLevel + 1 == inSymbolLineage.size() )	
				sym = inOwner;

		// Second part part: We will first search symbol in owner's prototype
		// if this one is a literal object
		if (! sym && inOwner && ! inOwner->GetPrototypes().empty())
		{
			Symbols::ISymbol *proto = inOwner->GetPrototypes().front();

			if (proto && proto->GetKind() == Symbols::ISymbol::kKindObjectLiteral) 
				sym = FindNamedDescendant(proto, inFile, inSymbolLineage, inLevel);
		}

		if ( inSymbolLineage[inLevel] != CVSTR("prototype") )
		{
			std::vector<Symbols::ISymbol *> prototypes = fSymTable->RetainSymbolsByName(inOwner, CVSTR("prototype"), inFile);
			if (prototypes.size())
				sym = FindNamedDescendant(prototypes[0], inFile, inSymbolLineage, inLevel, inKeepUndefined);
			
			fSymTable->ReleaseSymbols(prototypes);
		}

		// Third part: searching in owner's prototype doesn't return any appropriate symbol (or owner was empty).
		// We will keep on searching in the owner's named subsymbols list
		if (!sym)
		{
			std::vector<Symbols::ISymbol *> children = fSymTable->RetainSymbolsByName(inOwner, inSymbolLineage[inLevel], inFile);
			for (std::vector<Symbols::ISymbol *>::iterator it = children.begin(); it != children.end(); ++it)
			{
				Symbols::ISymbol* refSym = const_cast<Symbols::ISymbol *>( (*it)->RetainReferencedSymbol() );

				if (inKeepUndefined ||  !refSym->GetUndefinedState() )
				{
					// We found the appropriate named symbol at the requested level:
					// do not use the referenced symbol name but the original one!
					if ( (*it)->GetName() == inSymbolLineage[inLevel] )
					{
						// This is the last level in lineage: we found our definition !!!
						if ( inLevel + 1 == inSymbolLineage.size() )
						{
							// Note that for now we exclude symbols that aren't from a project 
							// file (JSF, WAF Library). This behaviour could change in future...
							if ( refSym->GetFile()->GetBaseFolder() == eSymbolFileBaseFolderProject )
							{
								// Verify that current symbol is not an undefined one (defined by "toto.titi =..."
								// for example. If it is we extend the research context to others files
								if ( refSym->GetUndefinedState() )
									sym = FindNamedDescendant(0, NULL, inSymbolLineage, 0, false);
								
								if (NULL == sym)
									sym = refSym;
							}
						}
						// This is not the last level in lineage: let's try a search at a deeper level
						else
							sym = FindNamedDescendant(refSym, inFile, inSymbolLineage, inLevel + 1);

						if (NULL != sym)
							break;
					}
				}
				refSym->Release();
			}

			fSymTable->ReleaseSymbols(children);
		}
	}

	return sym;
}

VIndex VJavaScriptSyntax::FindLineNamedToken( ICodeEditorDocument* inInterface, sLONG inLineIndex, const XBOX::VString& inText )
{
	VString line;
	
	inInterface->GetLine(inLineIndex, line);

	TokenList tokens;
	JavaScriptLexer *lexer = new JavaScriptLexer();
	lexer->SetLexerInput( &line );
	lexer->Tokenize( tokens, false );
	delete lexer;

	VIndex position = 0;

	for (TokenList::iterator iter = tokens.begin(); iter != tokens.end(); iter++)
	{
		ILexerToken *current = *iter;
		if ( current->GetText() == inText && current->GetType() == ILexerToken::TT_NAME)
		{
			position = current->GetPosition();
			break;
		}
	}

	CleanTokens( tokens );

	return position;
}
	
void VJavaScriptSyntax::GetSuggestionsForTesting( const VString &inSourceLine, ISymbolTable *inSymTable, const XBOX::VString &inFilePathStr, sLONG inLineIndex, EJSCompletionTestingMode inSuggestionMode, std::vector< XBOX::VString > &outSuggestions )
{
	// If this code looks familiar, it's because it's an evil twin to the GetSuggestions code.  Because everyone knows
	// that the best testing code is code that's been duplicated from elsewhere due to design issues and poor planning
	VFilePath filePath(inFilePathStr);
	
	std::vector< Symbols::IFile * > files = inSymTable->RetainFilesByPathAndBaseFolder( filePath, eSymbolFileBaseFolderProject );
	if ( ! files.empty() )
	{
		Symbols::IFile *file = files.front();
		file->Retain();

		inSymTable->ReleaseFiles(files);

		VString xstr(inSourceLine), original;
		std::vector< Completion > completions;

		_GetSuggestions(inSymTable, file, inLineIndex, xstr, completions, original);

		// Loop over all of the items in the keyword list and see if we can find any matches
		VCollator *collator = VIntlMgr::GetDefaultMgr()->GetCollator();
		
		
		// We use a smart algorithm as follow :
		// * we suggest both lower and upper case choices if the user input starts with a lower case letter
		// * we suggest only upper case choices if the user input starts with an upper case letter
		bool suggestUpperCaseOnly = false;
		VString firstLetter;
		bool widgetKludge = false;
		if( xstr.GetLength() > 0 )
		{
			firstLetter = xstr[0];
			if( firstLetter != CVSTR("\"") && firstLetter != CVSTR("\'") )
			{
				VString upperCase(firstLetter);
				upperCase.ToUpperCase();
				
				suggestUpperCaseOnly = firstLetter.EqualToString(upperCase, true);
			}

			if( xstr.Find("$$") )
				widgetKludge = true;
		}
		
		
		std::map<VString, bool>	mapOfSuggestions;
		for (vector< Completion >::iterator iter = completions.begin(); iter != completions.end(); iter++)
		{
			Completion comp = *iter;
			
			// Special kludge for widget ids on $$(
			VString userInput;
			if( comp.fStyle == eWidget )
			{
				if( widgetKludge )
					userInput.AppendString( CVSTR("\"") );
			}
			// End of the kludge
			userInput.AppendString(xstr);

			// We do not do case sensitive completions so that the user can accidentally type something
			// in the wrong case, but still get reasonable results back
			if (collator->EqualString_Like( comp.fText.GetCPointer(), comp.fText.GetLength(), userInput.GetCPointer(), userInput.GetLength(), suggestUpperCaseOnly ))
			{
				// As a sanity check, let's make sure that none of the suggestions we're giving the
				// caller are empty.
				assert( !comp.fText.IsEmpty() );
				if (comp.fText != original)
				{
					// Check to make sure this isn't a duplicate entry in the list either
					bool bAlreadyInList = false;

					for (VectorOfVString::iterator iter2 = outSuggestions.begin(); !bAlreadyInList && iter2 != outSuggestions.end(); ++iter2)
						if ( (*iter2).EqualTo( inSuggestionMode == eText ? comp.fText : comp.fDisplayText, true ) )
							bAlreadyInList = true;

					if ( ! bAlreadyInList )
						outSuggestions.push_back( inSuggestionMode == eText ? comp.fText : comp.fDisplayText );
				}
			}
		}
		file->Release();
		
		if( outSuggestions.size() == 1 && widgetKludge )
			outSuggestions.clear();
	}
}

void VJavaScriptSyntax::GetSuggestions( ICodeEditorDocument* inDocument, sLONG inLineIndex, sLONG inPos, ITipInfoArray *outSuggestions, sLONG& outStartOffset, bool inAll )
{
	// This shouldn't happen, but if the DB couldn't be opened, we can't ask for suggestions
	if (!fSymTable)
		return;	

	// to get faster results, we stop all the threads that adds symbols so we get access to db at full speed
	fSymTable->LockUpdates();

	std::vector< Completion > completions;
	VString xstr, original;
	
	inDocument->GetLine( inLineIndex, xstr );
	xstr.Truncate( inPos );
	
	// In some cases, we may want to skip suggestions
	bool dontSkipSuggestions = true;
	if( xstr.EndsWith( CVSTR("[")) )
	{
		dontSkipSuggestions = false;
	}
	
	if( dontSkipSuggestions )
	{
#if ACTIVATE_JSLOG
		JSLog log(true);
#endif

		Symbols::IFile *file = FileFromDocument( inDocument );
		
		// Call internal part shared whith SSJS test APIo
		_GetSuggestions(fSymTable, file, inLineIndex, xstr, completions, original);
		
		// Loop over all of the items in the keyword list and see if we can find any matches
		VCollator *collator = VIntlMgr::GetDefaultMgr()->GetCollator();
		
		
		// We use a smart algorithm as follow :
		// * we suggest both lower and upper case choices if the user input starts with a lower case letter
		// * we suggest only upper case choices if the user input starts with an upper case letter
		bool suggestUpperCaseOnly = false;
		VString firstLetter;
		bool widgetKludge = false;
		if( xstr.GetLength() > 0 )
		{
			firstLetter = xstr[0];
			if( firstLetter != CVSTR("\"") && firstLetter != CVSTR("\'") )
			{
				VString upperCase(firstLetter);
				upperCase.ToUpperCase();
				
				suggestUpperCaseOnly = firstLetter.EqualToString(upperCase, true);
			}
			
			if( xstr.Find("$$") )
				widgetKludge = true;
		}
		
		
		std::map<VString, bool>	mapOfSuggestions;
		for (vector< Completion >::iterator iter = completions.begin(); iter != completions.end(); iter++)
		{
			Completion comp = *iter;
			
			// Special kludge for widget ids on $$(
			VString userInput;
			if( comp.fStyle == eWidget )
			{
				if( widgetKludge )
					userInput.AppendString( CVSTR("\"") );
			}
			// End of the kludge
			userInput.AppendString(xstr);
			
			// We do not do case sensitive completions so that the user can accidentally type something
			// in the wrong case, but still get reasonable results back
			if (collator->EqualString_Like( comp.fText.GetCPointer(), comp.fText.GetLength(), userInput.GetCPointer(), userInput.GetLength(), suggestUpperCaseOnly ))
			{
				// As a sanity check, let's make sure that none of the suggestions we're giving the
				// caller are empty.
				assert( !comp.fText.IsEmpty() );
				if (!outSuggestions->Contains( comp.fDisplayText, true ) && comp.fText != original)
				{
					outSuggestions->AddTip( new VCodeEditorTipInfo( inDocument, comp.fDisplayText, comp.fText, (comp.fStyle == eWidget) ? eName : comp.fStyle ) );
					outStartOffset = inPos - xstr.GetLength() + 1;
				}
			}
		}
		file->Release();
		
		if( outSuggestions->GetCount() == 1 && widgetKludge )
			outSuggestions->Clear();
		
		// si l'on a que deux tips egaux, differents juste par la casse (par ex Function et function)
		// alors on ne garde que celui qui a la casse qui correspond au debut de la frappe
		if ( outSuggestions->GetCount() == 2 )
		{
			VString text1, text2;
			ITipInfo* tip1 = (*outSuggestions)[0];
			ITipInfo* tip2 = (*outSuggestions)[1];
			
			tip1->GetContentText( text1 );
			tip2->GetContentText( text2 );
			
			if ( text1 == text2 )
			{
				if ( text1.BeginsWith( original, true ) )
					outSuggestions->RemoveTip( 1 );
				else
					outSuggestions->RemoveTip( 0 );
			}
		}
		
#if ACTIVATE_JSLOG
		log.SetTitle( CVSTR("Suggesting Done") );
		log.Append( CVSTR("File"),		file->GetPath());
		log.Append( CVSTR("Line"),		xstr);
		log.Append( CVSTR("Count"),		outSuggestions->GetCount());
		log.Append( CVSTR("InAll"),		inAll);
#endif
	}

	fSymTable->UnlockUpdates();
}

void VJavaScriptSyntax::_GetSuggestions( ISymbolTable *inSymTable, Symbols::IFile *inFile, sLONG inLineIndex, VString &ioXstr, std::vector< Completion >& inCompletions, VString& outOriginal)
{
	VString completionLine(ioXstr), newCompletionLine;
	bool	bMakeSuggestions = true, bLastTokenIsWhitespace;

	if (NULL != inFile)
	{
		inFile->Retain();

		// Now we will lex our string into a bunch of tokens.  It allows us to tell what
		// the immediate preceeding token is.  Then we can take this information, along with
		// the partial text from the final token, and use it to filter the list of possible
		// results.
		TokenList tokens;

		JavaScriptLexer *lexer = new JavaScriptLexer;
		lexer->SetLexerInput( &ioXstr );
		lexer->Tokenize( tokens );
		delete lexer;

		bool bHasToSuggestConstructor = HasToSuggestConstructor(tokens, ioXstr);

		bool foundDollarFunctionId			= false;
		VString widgetIdQuote( CVSTR("\"") );

		if ( ! bHasToSuggestConstructor )
		{
			// We need to find the last semantic token in the text we've been given.  This is used to
			// determine what our completion string starts with.
			bLastTokenIsWhitespace = (tokens.size())?(tokens.back()->GetValue() == -1):(true);	
			ILexerToken::TYPE lastTokenType = ILexerToken::TT_INVALID;

			VString lastTokenText;
			VIndex	lastTokenPosition = 0;

			for (vector< ILexerToken * >::reverse_iterator iter = tokens.rbegin(); iter != tokens.rend(); ++iter)
			{
				if ( iter == tokens.rbegin() && -1 != (*iter)->GetValue() )
				{
					lastTokenType = (*iter)->GetType();
					lastTokenText = (*iter)->GetText();
					lastTokenPosition = (*iter)->GetPosition();
				}

				// Create a new completion line not including ':' to allow
				// completion on invalid (or incomplete) JavaScript syntax 
				// (see WAK0074773).
				VString text = (*iter)->GetText();
				if ( text == CVSTR(":") )
					break;
				
				newCompletionLine = text + newCompletionLine;
				
				if( (*iter)->GetType() == ILexerToken::TT_NAME && text == CVSTR("$$") )
					foundDollarFunctionId = true;
			}

			if( foundDollarFunctionId && !newCompletionLine.EndsWith(CVSTR(".")) && lastTokenType != ILexerToken::TT_NAME )
			{
				VString temp = completionLine;
				
				temp.RemoveWhiteSpaces();
				VIndex startIndex = temp.Find( CVSTR("(") );
				VIndex stopIndex = temp.Find( CVSTR(")"), startIndex);
				
				if( stopIndex == 0 )
				{
					newCompletionLine = temp;
					newCompletionLine.SubString(1, startIndex);
				
					ioXstr = CVSTR("");
					if( temp.GetLength() > startIndex )
					{
						ioXstr = temp;
						ioXstr.SubString(startIndex+1, temp.GetLength() - startIndex);
					
						if( ioXstr.Find( CVSTR("\'") ) )
							widgetIdQuote = "\'";	//WAK0082752 REMOVE CVSTR("\'") BECAUSE IT CAUSE A STUDIO CRASH ON WINDOWS
					}
				}
			}
			else
			{
				if (	bLastTokenIsWhitespace ||
						lastTokenType == ILexerToken::TT_INVALID ||
						lastTokenType == ILexerToken::TT_COMPARISON ||
						lastTokenType == ILexerToken::TT_PUNCTUATION ||
						lastTokenType == ILexerToken::TT_NUMBER ||
						lastTokenType == ILexerToken::TT_OPEN_STRING )
				{
					// There is no text to complete, so we do not want to modify the token list
					ioXstr = "";
				}
				else
				{
					// Truncate the completion line where we found the last semantic token so that we can
					// start the parsing from that point onward. Basically, this erases the last partial
					// semantic token from the text stream.
					newCompletionLine.Truncate( lastTokenPosition - (completionLine.GetLength() - newCompletionLine.GetLength() ) );
					
					// The last token is text that we want to try to complete, so use it and remove the
					// last token from our list.
					ioXstr = lastTokenText;
				}
			}


			if (ioXstr.IsEmpty() && ( bLastTokenIsWhitespace ||
				lastTokenType == ILexerToken::TT_INVALID || 
				/*lastTokenType == ILexerToken::TT_COMPARISON ||
				lastTokenType == ILexerToken::TT_PUNCTUATION || */
				lastTokenType == ILexerToken::TT_NUMBER ||
				lastTokenType == ILexerToken::TT_OPEN_STRING ) ) {
					bMakeSuggestions = false;
			}
		}

		if (bMakeSuggestions)
		{
			// Add on the wildcard character so that we can do a LIKE search
			outOriginal = ioXstr;
			
			Symbols::ISymbol *contextSymbol = inSymTable->GetSymbolOwnerByLine( inFile, inLineIndex );

			// Ask the parser for a list of suggestions.  Some of these suggestions are going to be literal,
			// like names or keywords.  Others are going to be placeholders, like "all identifiers", etc.
			SuggestionList suggestions;

			// Completion "new [someClass]" mode
			if (bHasToSuggestConstructor)
			{
				// If the the last token is an identifier we limit the class lookup
				// to the right
				std::vector< Symbols::ISymbol * > syms = inSymTable->RetainClassSymbols(0, ioXstr, 0);
				for (std::vector< Symbols::ISymbol * >::iterator iter = syms.begin(); iter != syms.end(); ++iter)
				{
					if ( ! (*iter)->IsPrivate() && _AreExecutionContextCompatible(inFile, (*iter)->GetFile() ) )
					{
						VString signature;

						inSymTable->GetSymbolSignature(*iter, signature);
						suggestions.Suggest( signature + CVSTR("||") + (*iter)->GetClass(), (*iter)->GetName(), SuggestionInfo::eName );
					}
					
					(*iter)->Release();
				}
			}
			// Completion standard mode
			else
			{ 
				ioXstr.AppendChar( VTask::GetWildChar() );
#if ACTIVATE_SUGGESTION_STRATEGY_CLASS
				DefaultJsSuggestionStrategy doSuggest(*inSymTable, *inFile, suggestions);
				JavaScriptParser* parser = new JavaScriptParser(doSuggest);
#else
				JavaScriptParser *parser = new JavaScriptParser;
#endif//ACTIVATE_SUGGESTION_STRATEGY_CLASS
				
#if ACTIVATE_JSLOG
				JSLog logCoreSuggesting(true);
				logCoreSuggesting.SetTitle("CoreSuggesting");
				logCoreSuggesting.Append("CompletionLine", newCompletionLine);
#endif
				
				parser->GetSuggestions( &newCompletionLine, &suggestions, contextSymbol, inFile, inSymTable );

#if ACTIVATE_JSLOG
				logCoreSuggesting.Print();
#endif
				delete parser;
			}

			bool bSetIdentifiers = false;

			for (SuggestionList::iterator iter = suggestions.begin(); iter != suggestions.end(); ++iter) {
				SuggestionInfo *suggestion = *iter;

				switch (suggestion->fType)
				{
					// Names and keywords are simple.  We just add their text to the list of suggestions
					case SuggestionInfo::eName:
					{
						inCompletions.push_back( Completion( suggestion->fDisplayText, suggestion->fText, eName ) );
					} 
					break;
						
					case SuggestionInfo::eKeyword:
					{
						inCompletions.push_back( Completion( suggestion->fText, eKeyword ) );
						//inCompletions.push_back( Completion( suggestion->fText + JAVASCRIPT_PARSER_SUGGESTION_SEPARATOR + CVSTR("Keyword"), suggestion->fText, eKeyword ) );
					} 
					break;
						
					case SuggestionInfo::eJavaScriptIdentifier:
					{
						// We want to do an unqualified lookup, which searches up the ownership chain.  This type of search is
						// used to find local variables, function declarations and parameters -- things which are considered to be
						// private properties.
						if (!bSetIdentifiers )
						{
							bSetIdentifiers  = true;
							
							_SuggestLocalLikeNamedSubSymbols(inSymTable, contextSymbol, ioXstr, inCompletions);
							_SuggestGlobalLikeNamedSubSymbols(inSymTable, inFile, ioXstr, inCompletions);
						}
					} 
					break;
						
					case SuggestionInfo::eWidget:
					{
						VString suggestedText = widgetIdQuote + suggestion->fText + widgetIdQuote;
						inCompletions.push_back( Completion( suggestion->fDisplayText, suggestedText, eWidget ) );
					}
					break;
				}
			}
			
			if (NULL != contextSymbol)
				contextSymbol->Release();

		}

		inFile->Release();
		CleanTokens( tokens );
	}
}

void VJavaScriptSyntax::_SuggestLocalLikeNamedSubSymbols(ISymbolTable *inSymTable, Symbols::ISymbol *inContextSymbol, VString inName, std::vector< Completion >& inCompletions)
{
	if (NULL != inSymTable && NULL != inContextSymbol)
	{
		inContextSymbol->Retain();

		Symbols::ISymbol* contextSymbol = inContextSymbol;
		while (contextSymbol)
		{
			std::vector< Symbols::ISymbol * > syms = inSymTable->RetainLikeNamedSubSymbols( contextSymbol, inName, true );
			for (std::vector< Symbols::ISymbol * >::iterator iter = syms.begin(); iter != syms.end(); ++iter)
			{
				if ( ! (*iter)->IsFunctionExpressionKind() )
					_SuggestSymbol( inSymTable, *iter, CVSTR("Local"), inCompletions);
				(*iter)->Release();
			}
			contextSymbol = contextSymbol->GetOwner();
		}

		inContextSymbol->Release();
	}
}

void VJavaScriptSyntax::_SuggestGlobalLikeNamedSubSymbols(ISymbolTable *inSymTable, Symbols::IFile *inFile, VString inName, std::vector< Completion >& inCompletions)
{
	if (NULL != inSymTable && NULL != inFile)
	{
		inFile->Retain();

		// Once we've exhausted all of the symbols within a context, we still want to search the globals
		// in the right execution context
		std::vector< Symbols::ISymbol * > syms = inSymTable->RetainLikeNamedSubSymbols( NULL, inName, true );
		for (std::vector< Symbols::ISymbol * >::iterator iter = syms.begin(); iter != syms.end(); ++iter)
		{
			// We want to exclude symbols that aren't in the document execution context.
			// Symbols belonging to both client and server context are kept
			if ( ! (*iter)->IsFunctionExpressionKind() )
				if ( _AreExecutionContextCompatible( inFile, (*iter)->GetFile() ) ) 
					_SuggestSymbol( inSymTable, *iter, CVSTR("Global"), inCompletions );

			(*iter)->Release();
		}

		inFile->Release();
	}
}

void VJavaScriptSyntax::_SuggestSymbol(ISymbolTable *inSymTable, Symbols::ISymbol* inSymbol, const VString& inContextName, std::vector< Completion >& inCompletions)
{
	if (NULL != inSymbol && ! inSymbol->IsPrivate() )
	{
		int kind = inSymbol->GetKind();

		if ( kind == Symbols::ISymbol::kKindFunctionParameterDeclaration ||
			 kind == Symbols::ISymbol::kKindLocalVariableDeclaration     ||
			 kind == Symbols::ISymbol::kKindCatchBlock                   ||
			 inSymbol->IsFunctionKind()                                  ||
			 inSymbol->IsEntityModelKind() )
		{
			VString signature, displayText;

			inSymTable->GetSymbolSignature(inSymbol, signature);
			displayText = signature + JAVASCRIPT_PARSER_SUGGESTION_SEPARATOR + inContextName;
			inCompletions.push_back( Completion( displayText, inSymbol->GetName(), eName ) );
		}
	}
}

bool VJavaScriptSyntax::_AreExecutionContextCompatible(Symbols::IFile* inFile1, Symbols::IFile* inFile2)
{
	bool compatible = true;

	if (NULL != inFile1 && NULL != inFile2)
	{
		inFile1->Retain();
		inFile2->Retain();

		ESymbolFileExecContext context1 = inFile1->GetExecutionContext();
		ESymbolFileExecContext context2 = inFile2->GetExecutionContext();
		
		if ( context1 != eSymbolFileExecContextClientServer && context2 != eSymbolFileExecContextClientServer )
			if (context1 != context2)
				compatible = false;

		inFile1->Release();
		inFile2->Release();
	}

	return compatible;
}

bool VJavaScriptSyntax::HasToSuggestConstructor(std::vector< ILexerToken * >& inTokens, VString& ioXstr)
{
	bool ok = false;

	// Some conditions must be verified to allow completion on new:
	// - last token must be whitespace or name
	// - those last tokens must be preceeded by a the "new" keyword
	if (  inTokens.size() > 1 && 
		 ( inTokens[inTokens.size() - 1]->GetType() == ILexerToken::TT_WHITESPACE ||
		   inTokens[inTokens.size() - 1]->GetType() == ILexerToken::TT_NAME ) )
	{
		VString lastTokenName;
		
		if (inTokens[inTokens.size() - 1]->GetType() == ILexerToken::TT_NAME )
			lastTokenName = inTokens[inTokens.size() - 1]->GetText();

		vector< ILexerToken * >::reverse_iterator iter = inTokens.rbegin();
		for (iter++; iter != inTokens.rend(); ++iter)
		{
			if ( ILexerToken::TT_WHITESPACE == (*iter)->GetType() )
				continue;
			
			if ( ILexerToken::TT_JAVASCRIPT_KEYWORD == (*iter)->GetType() && CVSTR("new") == (*iter)->GetText() )
			{
				ioXstr = lastTokenName;
				ioXstr.AppendUniChar(VTask::GetWildChar());
				ok = true;

			}

			break;
		}
	}

	return ok;
}

static VString DetermineWhitespace( const VString &inText )
{
	// We want to look at the given text to figure out what leading whitespace there is, and return
	// that whitespace to the caller.
	VString ret;
	if (!inText.IsEmpty()) {
		const UniChar *p = inText.GetCPointer();
		const UniChar *end = p + inText.GetLength();
		while (p != end) {
			if ((*p == ' ') || (*p == '\t')) {		// TODO: we may want to consider other forms of whitespace based on the JS specification.
				ret.AppendUniChar( *p );
			} else {
				break;
			}
			p++;
		}
	}

	return ret;
}

void VJavaScriptSyntax::InsertChar( ICodeEditorDocument* inDocument, UniChar inUnichar, sLONG inLineIndex, sLONG inPosition, ITipInfoArray *outSuggestions, sLONG& outStartOffset )
{
	// We do special processing if the user is currently inside of a comment.  Namely, when
	// they are editing a comment, we don't do any special processing of character insertions.
	if (inUnichar != '\r' &&
		inUnichar != '}' &&
		inUnichar != '{' && 
		inUnichar != '"' &&
		inUnichar != '\'' &&
		inUnichar != '(' &&
		inUnichar != '[' &&
		inUnichar != '@' &&
		inUnichar != '.') {
		// There's no special processing to do, so let's just bail out and skip all the extra work
		return;
	}

	sLONG lineNumber = inDocument->GetVisibleLine( inLineIndex );

	// Get the state of the previous line (assuming we're not the first line in the 
	// document) to see whether it ended with an unfinished comment.  If it does, then
	// we want to tell the lexer about it so that it can resume lexing the unfinish
	// comment instead of assuming these are non-comment tokens.
	bool previousLineEndsWithComment = false;
	if (inLineIndex > 0) {
		sLONG previousKind = inDocument->GetLineKind( inLineIndex - 1 );
		JavaScriptSyntaxLineKind previousLineKind = *( (JavaScriptSyntaxLineKind*)&previousKind);
		previousLineEndsWithComment = previousLineKind.fEndsWithOpenComment ? true : false;
	}

	// What we need to test is whether the current position within the given  line is inside 
	// of a comment or not.  Now that we know whether or not the previous line ended with a comment
	// we can lex the current line up to the insertion position to see whether we want to do any 
	// special processing of the inserted character.  We truncate at the character before the 
	// insertion position because the character has already been inserted in the stream.
	VString xstr;
	inDocument->GetLine( inLineIndex, xstr );
	
	bool endOfLine = inPosition == xstr.GetLength() || ( inPosition < xstr.GetLength() && ( xstr[ inPosition ] == ' ' || xstr[ inPosition ] == 9 ) );
	
	if (inPosition > 0)
		xstr.Truncate( inPosition - 1 );

	// Tokenize that line
	vector< ILexerToken * > tokens;
	JavaScriptLexer *lexer = new JavaScriptLexer();
	lexer->SetLexerInput( &xstr );
	lexer->Tokenize( tokens, previousLineEndsWithComment );
	delete lexer;

	// If the last token is an open comment or an open string, then we want to bail out because we are
	// not going to provide any special considerations in those situations
	ILexerToken *lastToken = (tokens.empty() ? NULL : tokens.back());
	if (lastToken && (
		lastToken->GetType() == ILexerToken::TT_OPEN_COMMENT ||
		lastToken->GetType() == ILexerToken::TT_OPEN_STRING))	
	{
		// TODO: Do some stuff here to handle JSDOC completion
		CleanTokens( tokens );
		return;
	}

	// We have one more test to perform to be able to handle single-line comments properly.  If the
	// last token on the line is a comment, and the first two non-whitespace characters of the token
	// are //, then it's a single-line comment
	if (lastToken && lastToken->GetType() == ILexerToken::TT_COMMENT) {
		VString text = lastToken->GetText();
		if (text.BeginsWith( CVSTR("//") ))
		{
			CleanTokens( tokens );
			return;
		}
	}

	VString spaces;
	for ( sLONG i = 0; i < GetTabWidth(); i++ )
		spaces.AppendChar( ' ' );

	VString str, whitespace, lineText, preceedingLine;

	// Now that we know it's safe to do, let's do our final processing!  
	//////////
	//	Performance note
	//	If you add any cases to the switch, you must also add them to the check at the
	//	top of the function.  This allows us to do less processing on the majority of 
	//	user input while still providing special behaviors for non-commented cases.
	//////////
	switch (inUnichar) {
		case '\r': {
			if ( fAutoInsertTabs || fAutoInsertBlock )
			{
				// The user is entering a newline, so we want to ensure that the indentation
				// matches whatever the previous line's indentation is.  The line number passed
				// in to us is the actual line we're currently on, since the newline has already
				// been inserted.  So we want to look at the previous line's text to see how much
				// preceeding whitespace there is.
				inDocument->GetLine( inLineIndex - 1, preceedingLine );

				// If we have any whitespace to insert, then insert it
				whitespace = DetermineWhitespace( preceedingLine );

				if ( fAutoInsertTabs )
				{
					// If the preceeding line ends with a {, then we want to indent once more
					if ( ! preceedingLine.IsEmpty() 
						 && preceedingLine.GetUniChar( preceedingLine.GetLength() ) == (UniChar)'{'
						 && ! ( xstr.GetLength() > 0 && xstr[0] == (UniChar)'}' ) )
					{
						if ( UseInsertSpacesForTabs() )
							whitespace += spaces;
						else
							whitespace.AppendChar( '\t' );
					}

					if (!whitespace.IsEmpty())
						inDocument->InsertText( whitespace );
				}

				if ( fAutoInsertBlock )
				{
					// if we start a comment, then insert end of it
					if (	previousLineEndsWithComment )
					{
						if (    (   preceedingLine.GetLength() == whitespace.GetLength() + 2
							     && preceedingLine[ preceedingLine.GetLength() - 2 ] == '/' 
							     && preceedingLine[ preceedingLine.GetLength() - 1 ] == '*' )
							 || (   preceedingLine.GetLength() == whitespace.GetLength() + 3
							     && preceedingLine[ preceedingLine.GetLength() - 3 ] == '/' 
							     && preceedingLine[ preceedingLine.GetLength() - 2 ] == '*'
								 && preceedingLine[ preceedingLine.GetLength() - 1 ] == '*' ) )
						{
							if ( fAutoInsertTabs )
								inDocument->InsertText( CVSTR(" * \r") + whitespace + CVSTR(" */") );
							else
								inDocument->InsertText( whitespace + CVSTR(" * \r") + whitespace + CVSTR(" */") );
							inDocument->Select( whitespace.GetLength() + 3, whitespace.GetLength() + 3, lineNumber, lineNumber );
						}
						// if we are in the middle of a comment, then insert a star if 
						else if (    preceedingLine.GetLength() >= whitespace.GetLength() + 2
								  && preceedingLine[ whitespace.GetLength()     ] == '*' 
								  && preceedingLine[ whitespace.GetLength() + 1 ] == ' ' )
						{
							if ( fAutoInsertTabs )
								inDocument->InsertText( CVSTR("* ") );
							else
								inDocument->InsertText( whitespace + CVSTR("* ") );

							inDocument->Select( whitespace.GetLength() + 2, whitespace.GetLength() + 2, lineNumber, lineNumber );
						}
					}
				}

			}
		} break;

		case '"': {
			if ( fAutoInsertClosingChar && endOfLine )
			{
				inDocument->InsertText( CVSTR("\"") );
				inDocument->Select( inPosition, inPosition, lineNumber, lineNumber );
			}
		} break;

		case '\'': {
			if ( fAutoInsertClosingChar && endOfLine )
			{
				inDocument->InsertText( CVSTR("'") );
				inDocument->Select( inPosition, inPosition, lineNumber, lineNumber );
			}
		} break;

		case '(': {
			if ( fAutoInsertClosingChar && endOfLine )
			{
				inDocument->InsertText( CVSTR(")") );
				inDocument->Select( inPosition, inPosition, lineNumber, lineNumber );
			}
		} break;

		case '[': {
			if ( fAutoInsertClosingChar && endOfLine )
			{
				inDocument->InsertText( CVSTR("]") );
				inDocument->Select( inPosition, inPosition, lineNumber, lineNumber );
			}
		} break;

		case '{': {
			if ( fAutoInsertBlock )
			{
				// First, we're adding a newline character
				str = CVSTR( "\r" );

				// Then a particular amount of whitespace based on the current line's indent depth
				inDocument->GetLine( inLineIndex, lineText );
				whitespace = DetermineWhitespace( lineText );
				str += whitespace;
				
				if ( UseInsertSpacesForTabs() )
					str += spaces;
				else
					str.AppendChar( '\t' );

				// And then another newline with the closing brace
				str += CVSTR( "\r" ) + whitespace + CVSTR( "}" );
				inDocument->InsertText( str );
				inDocument->Select( whitespace.GetLength() + 1, whitespace.GetLength() + 1, lineNumber + 1, lineNumber + 1 );
			}
		} break;

		case '}': {
			if ( fAutoInsertTabs && inPosition > 1 )
			{
				inDocument->GetLine( inLineIndex, lineText );
				lineText.GetSubString( inPosition - 1, 2, str );
				if ( str == CVSTR( "\t}" ) )
				{
					inDocument->GetLine( inLineIndex - 1, preceedingLine );
					sLONG len = preceedingLine.GetLength();
					sLONG i = 0, j = 0;

					while ( ( i < len ) && ( preceedingLine[ i ] == 9 ) )
						i++;

					while ( ( j < lineText.GetLength() ) && ( lineText[ j ] == 9 ) )
						j++;

					bool b = ( len > 0 ) && ( preceedingLine[ len - 1 ] == '{' );

					if (    ( ( i == j ) && ! b )
						 || ( ( i == j - 1 ) && b ) )
					{
						inDocument->Select( inPosition - 2, inPosition, lineNumber, lineNumber );
						inDocument->InsertText( CVSTR( "}" ) );
					}
				}
				else
				{
					lineText.GetSubString( inPosition - spaces.GetLength(), spaces.GetLength() + 1, str );
					if ( str == spaces + CVSTR( "}" ) )
					{

						inDocument->GetLine( inLineIndex - 1, preceedingLine );
						sLONG len = preceedingLine.GetLength();
						sLONG i = 0, j = 0;
						while ( i < len && preceedingLine[ i ] == ' ' )
							i++;

						while ( ( j < lineText.GetLength() ) && ( lineText[ j ] == ' ' ) )
							j++;

						bool b = ( len > 0 ) && ( preceedingLine[ len - 1 ] == '{' );

						if (    ( ( i == j ) && ! b )
							 || ( ( i == j - GetTabWidth() ) && b ) )
						{
							inDocument->Select( inPosition - 1 - spaces.GetLength(), inPosition, lineNumber, lineNumber );
							inDocument->InsertText( CVSTR( "}" ) );
						}
					}
				}
			}
		} break;
	}

	CleanTokens( tokens );
}

void VJavaScriptSyntax::UpdateBreakPointContent( ICodeEditorDocument* inDocument, sWORD inBreakID, const VString& inLineContent )
{
	if ( NULL != fBreakPointManager )
		fBreakPointManager->UpdateBreakPointContent( inDocument, inBreakID, inLineContent );
}

void VJavaScriptSyntax::UpdateBreakPointsLineNumbers( ICodeEditorDocument* inDocument, const std::vector<sWORD>& inBreakIDs, const std::vector<sLONG>& inLineNumbers )
{
	if ( NULL != fBreakPointManager )
		fBreakPointManager->UpdateBreakPointsLineNumbers( inDocument, inBreakIDs, inLineNumbers );
}

void VJavaScriptSyntax::AddBreakPoint( ICodeEditorDocument* inDocument, sLONG inLineNumber, sWORD& outID, bool& outDisabled )
{
	if ( NULL != fBreakPointManager )
		fBreakPointManager->AddBreakPoint( inDocument, inLineNumber, outID, outDisabled );
	else
		outID = 0;
}

bool VJavaScriptSyntax::EditBreakPoint( ICodeEditorDocument* inDocument, sWORD inBreakID, bool& outDisabled )
{
	if ( NULL != fBreakPointManager )
		return fBreakPointManager->EditBreakPoint( inDocument, inBreakID, outDisabled );
	else
		return false;
}

void VJavaScriptSyntax::RemoveBreakPoint( ICodeEditorDocument* inDocument, sLONG inLineNumber )
{
	if ( NULL != fBreakPointManager )
		fBreakPointManager->RemoveBreakPoint( inDocument, inLineNumber );
}

bool VJavaScriptSyntax::GetBreakPoints( ICodeEditorDocument* inDocument, std::vector<sLONG>& outLineNumbers, std::vector<sWORD>& outIDs, std::vector<bool>& outDisabled )
{
	if ( NULL != fBreakPointManager )
		return fBreakPointManager->GetBreakPoints( inDocument, outLineNumbers, outIDs, outDisabled );
	else
		return false;
}

Symbols::ISymbol *VJavaScriptSyntax::GetDeclarationContext( )
{
	return fDeclarationParsingContextSymbol;
}

Symbols::IFile *VJavaScriptSyntax::FileFromDocument( ICodeEditorDocument *inDocument )
{
	if (!fSymTable)	return NULL;


	VFilePath docPath;
	inDocument->GetPath( docPath );

	// If we've already cached the file, let's just return what we cached.
	if (fDocumentFile) {
		VString relativePath;

		fSymTable->GetRelativePath( docPath, inDocument->GetBaseFolder(), relativePath );
		if ( fDocumentFile->GetBaseFolder() == inDocument->GetBaseFolder() && fDocumentFile->GetPath() == relativePath)
		{
			fDocumentFile->Retain();
			return fDocumentFile;
		} else {
			fDocumentFile->Release();
			fDocumentFile = NULL;
		}
	}

	Symbols::IFile *ret = NULL;
	std::vector< Symbols::IFile * > files = fSymTable->RetainFilesByPathAndBaseFolder( docPath, inDocument->GetBaseFolder() );
	if (!files.empty()) {
		ret = files.front();
		ret->Retain();
	} else {
		// The file doesn't exist in the database, so we want to make a new one.  We won't
		// be setting the file time however, because we want to ensure that a declaration
		// parsing pass happens
		Symbols::IFile *file = new VSymbolFile();
		VString path;
		fSymTable->GetRelativePath( docPath, inDocument->GetBaseFolder(), path );
		file->SetPath( path );
		VFile actualFile( docPath );

		fSymTable->AddFile( file );
		ret = file;
	}

	fSymTable->ReleaseFiles(files);

	// Cache the file we grabbed.
	fDocumentFile = ret;
	fDocumentFile->Retain();

	return ret;
}

uLONG8 VJavaScriptSyntax::GetFileModificationStamp( ICodeEditorDocument *inDocument )
{
	VFilePath docPath;
	inDocument->GetPath( docPath );
	VFile actualFile( docPath );
	VTime time;
	if (actualFile.Exists()) {
		actualFile.GetTimeAttributes( &time );
		return time.GetStamp();
	} else {
		// It's fine to return it as negative one because we're testing for inequaltity,
		// not above or below comparisons.
		return -1;
	}
}

void VJavaScriptSyntax::PerformIdleProcessing( ICodeEditorDocument *inDocument )
{
	VFilePath path;
	inDocument->GetPath( path );

	VString contents;
	inDocument->GetCodeText( contents );

	IDocumentParserManager *manager = inDocument->GetParserManager();
	if (manager)
	{
		VSymbolFileInfos fileInfos(path, inDocument->GetBaseFolder(), inDocument->GetExecutionContext());

		manager->ScheduleTask(this, fileInfos, contents, inDocument->GetSymbolTable(), IDocumentParserManager::kPriorityAboveNormal);
	}
}

void VJavaScriptSyntax::UpdateCompilerErrors( ICodeEditorDocument* inDocument )
{

}

void VJavaScriptSyntax::GotoNextMethodError( ICodeEditorDocument* inDocument, bool inNext )
{
}

void VJavaScriptSyntax::SwapAssignments( ICodeEditorDocument* inDocument, VString& ioString )
{
}

bool VJavaScriptSyntax::IsComment( ICodeEditorDocument* inDocument, const VString& inString )
{
	// We will hand the string off to the lexer, and if a single token comes back that 
	// says "this is a comment", then we're set.  If multiple tokens come back that are
	// non-white space, we know it's not a comment.
	vector< ILexerToken * > tokens;
	JavaScriptLexer *lexer = new JavaScriptLexer;
	lexer->SetLexerInput( const_cast< VString * >( &inString ) );
	lexer->Tokenize( tokens );
	delete lexer;

	// Loop over the tokens and see what we've got
	bool isComment = false;

	for (vector< ILexerToken *>::iterator iter = tokens.begin(); 
						iter != tokens.end(); iter++) {
		ILexerToken *current = *iter;

		// If we have a comment token, then we still need to keep looking to make sure
		// there is not some other token on the same line.
		if (current->GetType() == ILexerToken::TT_COMMENT) {
			isComment = true;
		} else if (current->GetType() != ILexerToken::TT_WHITESPACE) {
			// Whitespace can be safely ignored as not important.  But if we have something 
			// that's not whitespace, nor is it a comment, then we know this line cannot
			// contain only a comment and so we can bail out.
			isComment = false;
			break;
		}
	}

	CleanTokens( tokens );

	return isComment;
}

void VJavaScriptSyntax::SwapComment( ICodeEditorDocument* inDocument, VString& ioString, bool inComment )
{
	// We've been given a string that we either need to comment, or uncomment, depending
	// on the state of the "inComment" parameter.  We will do a simple validation if the caller
	// is asking us to uncomment something which isn't commented.
	if (inComment) {
		// This is the easy case -- we just need to take the string and wrap it with the appropriate
		// comment characters.  If there are no newlines in the string, we will use a single-line comment,
		// otherwise we will use the multi-line comment.
		if (ioString.FindUniChar( (UniChar)CHAR_CONTROL_000A ) || ioString.FindUniChar( (UniChar)CHAR_CONTROL_000D )) {
			ioString = CVSTR("/*") + ioString + CVSTR("*/");
		} else {
			ioString = CVSTR("//") + ioString;
		}
	} else {
		// Make sure that what we're dealing with really is commented before we start making 
		// assumptions about the format of it.
		if (!IsComment( inDocument, ioString ))	return;

		// Now that we know it's commented, we need to strip off the leading and trailing 
		// comment markers.  We don't know whether there's whitespace in front or behind those
		// markers, so we cannot simply chop based on character positions.  So we will loop from 
		// the beginning of the string until we find the first non-whitespace which we know to be
		// the open comment marker, and remove those two characters.  We will then start from the
		// end of the string and work backwards to do the same for the close comment marker.
		
		// Handle the open comment
		bool isSingleLine = false;
		for (VIndex i = 0; i < ioString.GetLength(); ++i) {
			// We know the first character we will happen upon will be the "/" since we've already
			// validated that this is a comment.
			if (ioString[ i ] == (UniChar)'/') {
				isSingleLine = (ioString[ i + 1 ] == '/');
				// Remove this character and the one that follows it.  Except that i need to be
				// base one for this call
				ioString.Remove( i + 1, 2 );
				// We're done with the opening comment indicator
				break;
			}
		}

		// Handle the close comment
		if (!isSingleLine) {
			for (VIndex i = ioString.GetLength() - 1; i >= 0; --i) {
				// We know the first character we will happen upon will also be the "/"
				if (ioString[ i ] == (UniChar)'/') {
					// Remove this character and the one preceeding it.  Except that i is
					// actually base 1 in this case.
					ioString.Remove( i, 2 );
					// We're done with the closing comment indicator
					break;
				}
			}
		}
	}
}

void VJavaScriptSyntax::CallHelp( ICodeEditorDocument* inDocument )
{
	if( !inDocument )
		return;

	if( !inDocument->IsSelectionEmpty() )
	{
		VString str;
		inDocument->GetSelectedText( str );

		// Next steps
		// 1. compute an url
		VString url("http://doc.wakanda.org/home2.en.html#/SearchResults/");
		url.AppendString(str);
		url.AppendString("/0/false.en.html");

		// 2. trigger a message passing his url
		fRequestDocumentationSignal.Trigger(url);
	}
}


bool VJavaScriptSyntax::IsMatchingCharacter( ICodeEditorDocument* inDocument, sLONG inLineIndex, sLONG inOffset, UniChar inChar )
{
	const StylesVector* styles;
	inDocument->GetStyle( inLineIndex, styles );
	sLONG		nSize = styles-> size ( );
	if ( inOffset < 0 || inOffset > ( nSize - 1 ) )
	{
		xbox_assert ( false );

		return false;
	}

	uBYTE style = (*styles)[ inOffset ];
	if ( style != eComment && style != eString )
	{
		switch( inChar )
		{
			case '(':
			case ')':
			case '{':
			case '}':
			case '[':
			case ']':
				return true;
			default:
				return false;
		}
	}
	else
		return false;
}


bool VJavaScriptSyntax::ShouldValidateTipWindow( UniChar inChar )
{
	return inChar == '(' || inChar == ':' || inChar == '=' || inChar == ';' || 
		   inChar == '<' || inChar == '>' || inChar == '+' || inChar == '-' || 
		   inChar == '{' || inChar == '/' || inChar == '#' || inChar == '[';
}

bool VJavaScriptSyntax::DoubleClick( ICodeEditorDocument* inDocument, sLONG inLineIndex, sLONG inOffset, bool inAlternateKey )
{
	return false;
/*
	sLONG start, length;
	DetermineMorphemeBoundary( inDocument, inLineIndex, inOffset, start, length, inAlternateKey );

	sLONG line = inDocument->GetVisibleLine( inLineIndex );
	inDocument->Select( start, start + length, line, line );

	return true;
*/
}

#if _DEBUG
class CTipInfoArray : public XBOX::VObject, public ITipInfoArray
{
private:
	static inline bool TipsComparisonPredicate( const ITipInfo *left, const ITipInfo *right )
	{
		XBOX::VString leftText, rightText;
		left->GetDisplayText( leftText );
		right->GetDisplayText( rightText );
		return (leftText < rightText);
	}

public:
	CTipInfoArray()	{ }
	~CTipInfoArray() { 
		Clear();
	}

	virtual ITipInfoArray *Clone() {
		CTipInfoArray *ret = new CTipInfoArray();
		for (std::vector< ITipInfo * >::iterator iter = fArray.begin(); iter != fArray.end(); ++iter) {
			ret->AddTip( (*iter)->Clone() );
		}
		return ret;
	}

	virtual void AddTip( ITipInfo *tip ) {
		fArray.push_back( tip ); 
	}

	virtual void RemoveTip( sLONG inIndex ) {
		fArray[ inIndex ]->SelfDelete();
		fArray.erase( fArray.begin() + inIndex );
	}

	virtual ITipInfo *operator[]( sLONG inIndex ) {
		return fArray[ inIndex ];
	}

	virtual void Clear() {
		while (!fArray.empty()) { 
			fArray.back()->SelfDelete();
			fArray.pop_back();
		}
	}

	virtual sLONG GetCount() const {
		return (sLONG) fArray.size();
	}

	virtual bool Contains( const XBOX::VString &inText, bool inCaseSensitive = false ) {
		for (std::vector< ITipInfo * >::iterator iter = fArray.begin(); iter != fArray.end(); ++iter) {
			XBOX::VString text;
			(*iter)->GetDisplayText( text );
			if (text.EqualTo( inText, inCaseSensitive ))	return true;
		}
		return false;
	}

	virtual void Sort() {
		sort( fArray.begin(), fArray.end(), TipsComparisonPredicate );
	}

protected:
	std::vector< ITipInfo * > fArray;
};

//////////////////
// NOTE: Do not call this function directly.  Use TestPositiveSuggestions or TestNegativeSuggestions instead
//////////////////
static void TestSuggestions( bool inPositiveTest, ICodeEditorDocument *inDocument, ISymbolTable *inSymTable, 
							ISyntaxInterface *inSyntax, 
							const VString &inSourceString, 
							sLONG inLineOffset, sLONG inCharacterOffset,
							sLONG inExpectedNumberOfMatches, va_list marker )
{
	xbox_assert( inDocument );
	xbox_assert( inSyntax );

	ITipInfoArray *suggestions = new CTipInfoArray();
	sLONG offset = 0;

	VFilePath tempPath( CVSTR("c:\\depot\\tempdoc.js") );
	inDocument->OpenDocument( inSourceString, inSymTable, inSyntax, eCRM_LF, VTC_US_ASCII );
	inDocument->SetPath( tempPath );

	// The act of opening the document spawns off a thread which performs the declaration parsing.  We
	// need to wait for that thread to complete before we can call GetSuggestions.  We're going to cheat
	// for right now, and just go to sleep for a while.  This will cause the other thread to wake up and
	// perform its work.
	VTask::Sleep( 1000 );
	inSyntax->GetSuggestions( inDocument, inLineOffset, inCharacterOffset, suggestions, offset, false );

	// The number of suggestions that can come back is often significantly larger
	// than the number of expected suggestions.  That is because the suggestion list 
	// may contain keywords, or DOM elements, etc.  The caller passes in the matches
	// they expect to find, not the number of suggestions that come back.

	// Loop over the matches the user expects us to have
	for (int i = 0; i < inExpectedNumberOfMatches; i++) {
		// Loop over the suggestions to see if we can find this one
		bool found = false;
		VString argStr = va_arg( marker, char * );
		for (sLONG i = 0; !found && i < suggestions->GetCount(); i++) {
			ITipInfo *suggestion = (*suggestions)[ i ];
			VString suggestionString;
			suggestion->GetDisplayText( suggestionString );
	
			if (suggestionString.EqualToString( argStr, true )) {
				found = true;
			}
		}

		if (inPositiveTest) {
			xbox_assert( found );
		} else {
			xbox_assert( !found );
		}
	}

	delete suggestions;

	// Remove the file we just made from the symbol table so that the symbols which
	// we created are removed also
	/*std::vector< Symbols::IFile * > files = inSymTable->GetFilesByPath( tempPath.GetPath() );
	xbox_assert( files.size() == 1 );
	inSymTable->DeleteFile( files.front() );*/
}

static void TestPositiveSuggestions( ICodeEditorDocument *inDocument, ISymbolTable *inSymTable, ISyntaxInterface *inSyntax, 
							const VString &inSourceString, 
							sLONG inLineOffset, sLONG inCharacterOffset,
							sLONG inExpectedNumberOfMatches, ... ) 
{
	va_list list;
	va_start( list, inExpectedNumberOfMatches );
	TestSuggestions( true, inDocument, inSymTable, inSyntax, inSourceString, inLineOffset, inCharacterOffset, inExpectedNumberOfMatches, list );
	va_end( list );
}

static void TestNegativeSuggestions( ICodeEditorDocument *inDocument, ISymbolTable *inSymTable, ISyntaxInterface *inSyntax, 
							const VString &inSourceString, 
							sLONG inLineOffset, sLONG inCharacterOffset,
							sLONG inExpectedNumberOfMatches, ... )
{
	va_list list;
	va_start( list, inExpectedNumberOfMatches );
	TestSuggestions( false, inDocument, inSymTable, inSyntax, inSourceString, inLineOffset, inCharacterOffset, inExpectedNumberOfMatches, list );
	va_end( list );
}
#endif // _DEBUG
