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
#ifndef _JAVASCRIPTLEXER_H_
#define _JAVASCRIPTLEXER_H_

namespace JavaScriptStringType {
	enum {
		kApostropheString = 1,
		kQuoteString
	};
};

namespace JavaScriptTokenValues {
	enum {
		// Keywords
		BREAK = 300,	// We have to start > 255, otherwise there may be conflicts with single-character tokens
		CASE,
		CATCH,
		CONTINUE,
		DEFAULT,
		KWORD_DELETE,
		DO,
		ELSE,
		FINALLY,
		FOR,
		FUNCTION,
		IF,
		KWORD_IN,
		INSTANCEOF,
		NEW,
		RETURN,
		SWITCH,
		KWORD_THIS,
		THROW,
		TRY,
		TYPEOF,
		VAR,
		KWORD_VOID,
		WHILE,
		WITH,
		// Special reserved words
		KWORD_FALSE,
		KWORD_NULL,
		KWORD_TRUE,
		// Future reserved words
		ABSTRACT,
		BOOLEAN,
		BYTE,
		CHAR,
		CLASS,
		KWORD_CONST,
		DEBUGGER,
		DOUBLE,
		ENUM,
		EXPORT,
		EXTENDS,
		FINAL,
		FLOAT,
		GOTO,
		IMPLEMENTS,
		IMPORT,
		INT,
		KWORD_INTERFACE,
		LONG,
		NATIVE,
		PACKAGE,
		PRIVATE,
		PROTECTED,
		PUBLIC,
		SHORT,
		STATIC,
		SUPER,
		SYNCHRONIZED,
		THROWS,
		TRANSIENT,
		VOLATILE,
		// Types
		NUMBER,
		STRING_APOSTROPHE,
		STRING_QUOTATION_MARK,
		IDENTIFIER,
		// Punctuators
		INCREMENTOR,
		DECREMENTOR,
		LEFTSHIFT,
		SIGNEDRIGHTSHIFT,
		UNSIGNEDRIGHTSHIFT,
		LOGICALOR,
		LOGICALAND,
		ASSIGNADD,
		ASSIGNSUBTRACT,
		ASSIGNMULTIPLY,
		ASSIGNDIVIDE,
		ASSIGNMODULUS,
		ASSIGNBITXOR,
		ASSIGNBITAND,
		ASSIGNBITOR,
		ASSIGNLEFTSHIFT,
		ASSIGNSIGNEDRIGHTSHIFT,
		ASSIGNUNSIGNEDRIGHTSHIFT,
		LESSTHANOREQUALTO,
		GREATERTHANOREQUALTO,
		EQUALITY,
		STRICTEQUALITY,
		NONEQUALITY,
		STRICTNONEQUALITY,
		REGEXP,
		// Conceptual constructs
		ENDL,	// The end of a line
	};
}

class JavaScriptLexer : public VLexerBase {
private:
	bool AdvanceOneToken( int &outToken, TokenList *outTokens, bool inIgnoreNewLines, bool inIgnoreRegExp );
	int SkipSemiColons();
	int ConsumePossiblePunctuation( UniChar inChar, TokenList *outTokens );

	VString fLastTokenText, fScriptDocText, fRegionName, fLastCommentText;
	bool fPeeked;
	bool fConsumedNewLine;
	int fLineNumber, fRegionLineNumber;
	bool fLexedScriptDocComment, fLexedRegionComment;
	int fMultilineCommentStart, fMultilineCommentEnd;
	VLexerStringInput* fStrLexerInput;

protected:
	virtual void ReportLexerError( VError inError, const char *inMessage );

	virtual bool IsWhitespace( UniChar inChar );
	virtual bool IsLineEnding( UniChar inChar );

	// If the comment start or end requires more than one character of lookahead,
	// it is up to the subclass to peek at subsequent characters in the stream.
	// Different languages have different commenting needs, including multiple
	// ways of expressing a comment.  For instance, JavaScript can use /* */ as
	// well as <!-- --> for its comments.  So the "type" parameter is used to identify
	// what type of comment was found so that the appropriate match can be located.  Each
	// lexer should choose a unique type identifier (such as a four char code) for the language.
	virtual bool IsMultiLineCommentStart( UniChar inChar, sLONG &outType );
	virtual bool IsMultiLineCommentEnd( UniChar inChar, sLONG &outCharsToConsume, sLONG inType );
	virtual bool IsSingleLineCommentStart( UniChar inChar );
	virtual void CommentConsumed( const VString &inText, sLONG inType );

	// Similar to comments, strings can be enclosed in multiple ways within a single language.
	// For instance, in SQL, a string can either be quoted with '', or with ``.  So the "type"
	// parameter is used to distinguish what type of string is being lexed so that we can match
	// it appropriately.  The "type" passed around should be the token's type as it will be assigned
	// to the token added to the token list.
	virtual bool IsStringStart( UniChar inChar, sLONG &outType, sLONG &outValue );
	virtual bool IsStringEnd( UniChar inChar, sLONG inType );
	virtual bool IsStringEscapeSequence( UniChar inChar, sLONG inType );
	virtual bool IsStringContinuationSequence( UniChar inChar, sLONG inType );
	virtual bool ConsumeEscapeSequence( sLONG inType, UniChar &outCharacterToAdd );

	virtual bool IsIdentifierStart( UniChar inChar );
	virtual bool IsIdentifierCharacter( UniChar inChar );
	virtual bool IsRegularExpressionStart( TokenList* inTokens, UniChar inChar );

	virtual int GetNumericTokenValueFromNumericType( ENumericType inType );
	virtual bool IsNumericStart( UniChar inChar, ENumericType &outType );
	virtual bool ConsumeNumber( VString *ioNumber, TokenList *ioTokens, ENumericType &ioType );
	
	bool ConsumePossibleRegularExpression( XBOX::VString& inBody, XBOX::VString& inFlags);

public:
	explicit JavaScriptLexer() : VLexerBase(), fPeeked( false ), fConsumedNewLine( false ), fLineNumber( 0 ), 
										fLexedScriptDocComment( false ), fLexedRegionComment( false ), fRegionLineNumber( 0 ), fStrLexerInput(NULL) {}

	~JavaScriptLexer() { if (NULL != fStrLexerInput) delete fStrLexerInput; }
	int PeekAtNextTokenForParser();
	virtual int GetNextTokenForParser() { return GetNextTokenForParser( true, false ); }
	virtual int GetNextTokenForParser( bool inIgnoreNewLines, bool inIgnoreRegExp );
	virtual VError Tokenize( TokenList &outTokens, bool inContinuationOfComment = false, sLONG inOpenStringType = 0);
	VString GetTokenText() { return fLastTokenText; }

	VString GetCommentText() { return fLastCommentText; }

	virtual void ConsumeWhiteSpaces(TokenList* ioTokens) { bool consumedWhitespace = false; SkipWhitespaces(consumedWhitespace, ioTokens); }

	void SetLexerInput( VString *inInput );
	bool GetNextRegularExpressionLiteral( VString *ioRegExBody, VString *ioRegExFlags );

	bool ConsumedNewlineBeforeToken() { return fConsumedNewLine; }

	bool ProcessedScriptDocToken() { return fLexedScriptDocComment; }
	VString GetScriptDocText() {
		// NOTE: We clear out the fact that we have processed a ScriptDoc comment here because that allows the 
		// consumer to attach a ScriptDoc comment to any parser element instead of just the next token after the 
		// initial comment.
		fLexedScriptDocComment = false;
		return fScriptDocText;
	}

	bool ProcessedRegionToken() { return fLexedRegionComment; }
	void GetRegionInformation( bool &outIsStart, VString &outStartRegionName, int &outLineNumber ) { outIsStart = !fRegionName.IsEmpty(); outStartRegionName = fRegionName; outLineNumber = fRegionLineNumber; }

	bool ProcessedMultilineComment( int &outStartLine, int &outEndLine ) { 
		outStartLine = fMultilineCommentStart; outEndLine = fMultilineCommentEnd;
		fMultilineCommentStart = fMultilineCommentEnd = -1;
		return (outStartLine != -1);
	}

	sLONG GetCurrentLineNumber() { return fLineNumber; }
	sLONG GetCurrentCharacterOffset() { return fLexerInput->GetCurrentPosition(); }

	virtual void SetLexerInput( ILexerInput *inInput ) { fScriptDocText = ""; fLexedScriptDocComment = false; fLineNumber = 0;  
														fPeeked = false;  fConsumedNewLine = false;  fMultilineCommentStart = -1;
														fMultilineCommentEnd = -1; fLexedRegionComment = false; fRegionLineNumber = 0;
														VLexerBase::SetLexerInput( inInput ); }

	static void GetKeywordList( std::vector< VString * > &outKeywords );
};

#endif // _JAVASCRIPTLEXER_H_