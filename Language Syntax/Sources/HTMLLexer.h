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
#ifndef _HTMLLEXER_H_
#define _HTMLLEXER_H_

namespace HTMLLexemes {
	enum {
		TEXT = 300,	// We have to start > 255, otherwise there may be conflicts with single-character tokens
		STRING,
		SELF_CLOSE,
		START_OF_CLOSE_TAG,
		BANG_START,			// <!
		QUESTION_START,		// <?
	};
}

class HTMLLexer : public VLexerBase {
private:
	typedef enum State {
		kDefault,
		kInComment,
	} State;

	State fState;

	bool AdvanceOneToken( int &outToken, TokenList *outTokens );
	int ConsumePossiblePunctuation( UniChar inChar, TokenList *outTokens );

	VString fLastTokenText;
	bool fPeeked;
	int fLineNumber;

	#if _DEBUG
		// The VJavaScriptSyntax constructor has friend acess to the HTMLLexer
		// class so that it can call the Test static function to do unit testing.
		friend class VJavaScriptSyntax;
		static void Test( void );
	#endif

	friend class HTMLParser;
	explicit HTMLLexer() : VLexerBase(), fPeeked( false ), fLineNumber( 0 ), fState( kDefault ) {}

	virtual void ReportLexerError( VError inError, const char *inMessage );

	virtual bool IsWhitespace( UniChar inChar );
	virtual bool IsLineEnding( UniChar inChar );

	// If the comment start or end requires more than one character of lookahead,
	// it is up to the subclass to peek at subsequent characters in the stream.
	// Different languages have different commenting needs, including multiple
	// ways of expressing a comment.  So the "type" parameter is used to identify
	// what type of comment was found so that the appropriate match can be located.  Each
	// lexer should choose a unique type identifier (such as a four char code) for the language.
	virtual bool IsMultiLineCommentStart( UniChar inChar, sLONG &outType );
	virtual bool IsMultiLineCommentEnd( UniChar inChar, sLONG &outCharsToConsume, sLONG inType );
	virtual bool IsSingleLineCommentStart( UniChar inChar ) { return false; }
	virtual bool ConsumeMultiLineComment( TokenList *ioTokens, sLONG inType );

	// Similar to comments, strings can be enclosed in multiple ways within a single language.
	// For instance, in SQL, a string can either be quoted with '', or with ``.  So the "type"
	// parameter is used to distinguish what type of string is being lexed so that we can match
	// it appropriately.  The "type" passed around should be the token's type as it will be assigned
	// to the token added to the token list.
	virtual bool IsStringStart( UniChar inChar, sLONG &outType, sLONG &outValue );
	virtual bool IsStringEnd( UniChar inChar, sLONG inType );
	virtual bool IsStringEscapeSequence( UniChar inChar, sLONG inType );
	virtual bool IsStringContinuationSequence( UniChar inChar, sLONG inType ) { return false; }
	virtual bool ConsumeEscapeSequence( sLONG inType, UniChar &outCharacterToAdd );

	virtual bool IsIdentifierStart( UniChar inChar );
	virtual bool IsIdentifierCharacter( UniChar inChar );

	virtual bool IsNumericStart( UniChar inChar, ENumericType &outType );
	virtual bool ConsumeNumber( VString *ioNumber, TokenList *ioTokens, ENumericType &ioType );
	virtual int GetNumericTokenValueFromNumericType( ENumericType inType ) { return 0; }

	bool ConsumePossibleEntity( VString &outEntityConsumed, UniChar &outCharacterToAdd );

	int PeekAtNextTokenForParser();
	virtual int GetNextTokenForParser();
	virtual VError Tokenize( TokenList &outTokens, bool inContinuationOfComment = false, sLONG inOpenStringType = 0 );
	VString GetTokenText() { return fLastTokenText; }

	void SetLexerInput( VString *inInput );

	sLONG GetCurrentLineNumber() { return fLineNumber; }
	sLONG GetCurrentCharacterOffset() { return fLexerInput->GetCurrentPosition(); }

	virtual void SetLexerInput( ILexerInput *inInput ) { fLineNumber = 0;  fState = kDefault;
														fPeeked = false; VLexerBase::SetLexerInput( inInput ); }
};

#endif // _HTMLLEXER_H_