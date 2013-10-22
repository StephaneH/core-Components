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
#include "unicode/uchar.h"

class JavaScriptLexerToken : public ILexerToken
{
	public :

		JavaScriptLexerToken ( ILexerToken::TYPE inType, VIndex inPosition, VIndex inLength, VString inText = "", int inValue = -1) :
													m_nType ( inType ), m_nPosition ( inPosition ), m_nLength ( inLength ),
													m_nTokenValue( inValue ), m_vstrText( inText )
													{ ; }
		virtual ~JavaScriptLexerToken ( ) { ; }

		virtual ILexerToken::TYPE GetType ( ) { return m_nType; }
		virtual XBOX::VIndex GetPosition ( ) { return m_nPosition; }
		virtual XBOX::VIndex GetLength ( ) { return m_nLength; }
		virtual VString GetText( ) { return m_vstrText; }
		virtual int GetValue( ) { return m_nTokenValue; }

		virtual void SelfDelete() { delete this; }

	private :

		ILexerToken::TYPE			m_nType;
		VIndex						m_nPosition;
		VIndex						m_nLength;
		VString 					m_vstrText;
		int							m_nTokenValue;
};

struct JS_KEYWORD
{
	VString*		vstrName;
	int				nCode;
};

/*
	NOTE: technically I should delete these VString constants when I'm done using them.
	However, I need them as long as the program runs. So I need to delete them if there will
	be a mechanism to "reload" a component.
*/
struct JS_KEYWORD JS_KEYWORDS [] = {
	{ new VString ( "break" ), JavaScriptTokenValues::BREAK },
	{ new VString ( "case" ), JavaScriptTokenValues::CASE },
	{ new VString ( "catch" ), JavaScriptTokenValues::CATCH },
	{ new VString ( "continue" ), JavaScriptTokenValues::CONTINUE},
	{ new VString ( "default" ), JavaScriptTokenValues::DEFAULT },
	{ new VString ( "delete" ), JavaScriptTokenValues::KWORD_DELETE },
	{ new VString ( "do" ), JavaScriptTokenValues::DO },
	{ new VString ( "else" ), JavaScriptTokenValues::ELSE},
	{ new VString ( "false" ), JavaScriptTokenValues::KWORD_FALSE },
	{ new VString ( "finally" ), JavaScriptTokenValues::FINALLY},
	{ new VString ( "for" ), JavaScriptTokenValues::FOR },
	{ new VString ( "function" ), JavaScriptTokenValues::FUNCTION },
	{ new VString ( "if" ), JavaScriptTokenValues::IF },
	{ new VString ( "in" ), JavaScriptTokenValues::KWORD_IN },
	{ new VString ( "instanceof" ), JavaScriptTokenValues::INSTANCEOF },
	{ new VString ( "new" ), JavaScriptTokenValues::NEW },
	{ new VString ( "null" ), JavaScriptTokenValues::KWORD_NULL },
	{ new VString ( "return" ), JavaScriptTokenValues::RETURN },
	{ new VString ( "switch" ), JavaScriptTokenValues::SWITCH },
	{ new VString ( "this" ), JavaScriptTokenValues::KWORD_THIS },
	{ new VString ( "throw" ), JavaScriptTokenValues::THROW },
	{ new VString ( "true" ), JavaScriptTokenValues::KWORD_TRUE },
	{ new VString ( "try" ), JavaScriptTokenValues::TRY },
	{ new VString ( "typeof" ), JavaScriptTokenValues::TYPEOF },
	{ new VString ( "var" ), JavaScriptTokenValues::VAR },
	{ new VString ( "void" ), JavaScriptTokenValues::KWORD_VOID },
	{ new VString ( "while" ), JavaScriptTokenValues::WHILE },
	{ new VString ( "with" ), JavaScriptTokenValues::WITH },
};
#define	JS_KEYWORD_COUNT	(sizeof( JS_KEYWORDS ) / sizeof( JS_KEYWORD ))

struct JS_KEYWORD JS_FUTURE_KEYWORDS [] = {
	{ new VString ( "abstract" ), JavaScriptTokenValues::ABSTRACT },
	{ new VString ( "boolean" ), JavaScriptTokenValues::BOOLEAN },
	{ new VString ( "byte" ), JavaScriptTokenValues::BYTE},
	{ new VString ( "char" ), JavaScriptTokenValues::CHAR },
	{ new VString ( "class" ), JavaScriptTokenValues::CLASS },
	{ new VString ( "const" ), JavaScriptTokenValues::KWORD_CONST },
	{ new VString ( "debugger" ), JavaScriptTokenValues::DEBUGGER },
	{ new VString ( "double" ), JavaScriptTokenValues::DOUBLE },
	{ new VString ( "enum" ), JavaScriptTokenValues::ENUM },
	{ new VString ( "export" ), JavaScriptTokenValues::EXPORT },
	{ new VString ( "extends" ), JavaScriptTokenValues::EXTENDS },
	{ new VString ( "final" ), JavaScriptTokenValues::FINAL },
	{ new VString ( "float" ), JavaScriptTokenValues::FLOAT },
	{ new VString ( "goto" ), JavaScriptTokenValues::GOTO },
	{ new VString ( "implements" ), JavaScriptTokenValues::IMPLEMENTS },
	{ new VString ( "import" ), JavaScriptTokenValues::IMPORT },
	{ new VString ( "int" ), JavaScriptTokenValues::INT },
	{ new VString ( "interface" ), JavaScriptTokenValues::KWORD_INTERFACE },
	{ new VString ( "long" ), JavaScriptTokenValues::LONG },
	{ new VString ( "native" ), JavaScriptTokenValues::NATIVE },
	{ new VString ( "package" ), JavaScriptTokenValues::PACKAGE },
	{ new VString ( "private" ), JavaScriptTokenValues::PRIVATE },
	{ new VString ( "protected" ), JavaScriptTokenValues::PROTECTED },
	{ new VString ( "public" ), JavaScriptTokenValues::PUBLIC },
	{ new VString ( "short" ), JavaScriptTokenValues::SHORT },
	{ new VString ( "static" ), JavaScriptTokenValues::STATIC },
	{ new VString ( "super" ), JavaScriptTokenValues::SUPER },
	{ new VString ( "synchronized" ), JavaScriptTokenValues::SYNCHRONIZED },
	{ new VString ( "throws" ), JavaScriptTokenValues::THROWS },
	{ new VString ( "transient" ), JavaScriptTokenValues::TRANSIENT },
	{ new VString ( "volatile" ), JavaScriptTokenValues::VOLATILE },
};
#define	JS_FUTURE_KEYWORD_COUNT	(sizeof( JS_FUTURE_KEYWORDS ) / sizeof( JS_KEYWORD ))

static int IsKeyword( VString* vstrIn )
{
	VString vstrFirstChar;
	vstrFirstChar.AppendUniChar( vstrIn-> GetUniChar( 1 ) );
	UniChar	nFirstChar = vstrFirstChar.GetUniChar( 1 );
	
	for (int i = 0; i < JS_KEYWORD_COUNT; i++) {
		if (nFirstChar > JS_KEYWORDS[ i ].vstrName->GetUniChar( 1 ) )		continue;
		else if (nFirstChar < JS_KEYWORDS [ i ].vstrName->GetUniChar( 1 ))	break;
		
		if (vstrIn->EqualToString( *(JS_KEYWORDS [ i ].vstrName), true ))	return JS_KEYWORDS[ i ].nCode;
	}

	return 0;
}

static int IsFutureReservedWord( VString* vstrIn )
{
	VString vstrFirstChar;
	vstrFirstChar.AppendUniChar( vstrIn-> GetUniChar( 1 ) );
	UniChar	nFirstChar = vstrFirstChar.GetUniChar( 1 );
	
	for (int i = 0; i < JS_FUTURE_KEYWORD_COUNT; i++) {
		if (nFirstChar > JS_FUTURE_KEYWORDS[ i ].vstrName->GetUniChar( 1 ) )		continue;
		else if (nFirstChar < JS_FUTURE_KEYWORDS [ i ].vstrName->GetUniChar( 1 ))	break;
		
		if (vstrIn->EqualToString( *(JS_FUTURE_KEYWORDS [ i ].vstrName), true ))	return JS_FUTURE_KEYWORDS[ i ].nCode;
	}

	return 0;
}

void JavaScriptLexer::GetKeywordList( std::vector< VString * > &outKeywords )
{
	for (int i = 0; i < JS_KEYWORD_COUNT; i++) {
		outKeywords.push_back( JS_KEYWORDS[ i ].vstrName );
	}
	for (int i = 0; i < JS_FUTURE_KEYWORD_COUNT; i++) {
		outKeywords.push_back( JS_FUTURE_KEYWORDS[ i ].vstrName );
	}
}

void JavaScriptLexer::ReportLexerError( VError inError, const char *inMessage )
{
	// We don't actually have to report any lexer errors since this is not being
	// used for actual compilation.  Basically, errors are expected and will be
	// ignored anyways!
}

bool JavaScriptLexer::IsWhitespace( UniChar inChar )
{
	// As per section 7.2 of the ECMAScript specification
	switch (inChar) {
		case CHAR_SPACE:			// Space
		case CHAR_CONTROL_0009:		// Tab
		case CHAR_CONTROL_000B:		// Vertical Tab
		case CHAR_CONTROL_000C:		// Form Feed
		case CHAR_NO_BREAK_SPACE:	// Non-breaking space
			return true;
	}

	// We also want to check the Zs "whitespace" category
	if (U_GET_GC_MASK( inChar ) & (U_GC_ZS_MASK))	// Letter number
		return true;

	return false;
}

bool JavaScriptLexer::IsLineEnding( UniChar inChar )
{
	// As per section 7.3 of the ECMAScript specification
	switch (inChar) {
		case CHAR_CONTROL_000D:		// Carriage return
		case CHAR_CONTROL_000A:		// Line feed
		case CHAR_LINE_SEPARATOR:
		case CHAR_PARAGRAPH_SEPARATOR: {
			// We're going to be sneaky here and say that seeing a new line is close
			// enough to consuming it as well.  This way, when the base lexer handles a
			// single line comment, we flag it as the end of a line.  Without doing this,
			// automatic semi-colon insertion doesn't work properly.
			fLineNumber++;
			fConsumedNewLine = true;
			return true;
		} break;
	}

	return false;
}

bool JavaScriptLexer::IsSingleLineCommentStart( UniChar inChar )
{
	// As per section 7.4 of the ECMAScript specification
	if (CHAR_SOLIDUS != inChar)	return false;
	if (!fLexerInput->HasMoreChars())	return false;
	if (CHAR_SOLIDUS != fLexerInput->PeekAtNextChar())	return false;
	return true;
}

const sLONG kJavaScriptMultilineComment = 'JSml';
const sLONG kScriptDocComment = 'SDoc';
bool JavaScriptLexer::IsMultiLineCommentStart( UniChar inChar, sLONG &outType )
{
	// As per section 7.4 of the ECMAScript specification
	if (CHAR_SOLIDUS != inChar)	return false;
	if (!fLexerInput->HasMoreChars())	return false;
	// Get the next character to see if it's an asterisk
	if (CHAR_ASTERISK == fLexerInput->MoveToNextChar()) {
		outType = kJavaScriptMultilineComment;

		// Move to the next character to see if it's another asterisk, which
		// tells us that we may have a script doc comment, but only if it is
		// not followed by anything other than a newline
		if (fLexerInput->HasMoreChars()) {
			if (CHAR_ASTERISK == fLexerInput->MoveToNextChar()) {
				if (fLexerInput->HasMoreChars() && IsLineEnding( fLexerInput->PeekAtNextChar() )) {
					// If we found a line ending, we don't want to trigger it as being a newline
					// since it is part of the comment consumption process that watches newlines.  This
					// is a bit of a kludge, but we need to remove a line ending from our list
					fLineNumber--;
					outType = kScriptDocComment;
				}
			}
			// Be sure to put back the character we just ate while testing for ScriptDoc
			fLexerInput->MoveToPreviousChar();
		}

		fMultilineCommentStart = fLineNumber;

		// Be sure to put back the character we just ate while testing for multiline comment
		fLexerInput->MoveToPreviousChar();
		return true;
	}

	// Be sure to put back the character we ate while looking for a multiline comment
	fLexerInput->MoveToPreviousChar();

	return false;
}

bool JavaScriptLexer::IsMultiLineCommentEnd( UniChar inChar, sLONG &outCharsToConsume, sLONG inType )
{
	// We have to check for newlines for no other reason than bookkeeping.  This records when we
	// locate a newline so that the fLineNumber property remains properly in-sync.  We will consume
	// the newline for the caller if that's what we've gotten.
	if (IsLineEnding( inChar ))	ConsumeLineEnding( inChar );

	if (inType != kCommentContinuationType &&
		inType != kScriptDocComment &&
		inType != kJavaScriptMultilineComment)			return false;
	if (CHAR_ASTERISK != inChar)						return false;
	if (!fLexerInput->HasMoreChars())					return false;
	if (CHAR_SOLIDUS != fLexerInput->PeekAtNextChar())	return false;

	// We're at the end of the comment, but the caller still needs to consume
	// one character
	outCharsToConsume = 1;

	fMultilineCommentEnd = fLineNumber;

	return true;
}

void JavaScriptLexer::CommentConsumed( const VString &inText, sLONG inType )
{
	if (kScriptDocComment == inType) {
		// We've got a ScriptDoc comment that we need to handle.  We handle it by storing the text of
		// the comment off in a special buffer, so the caller has the option to recall it.  We will also
		// set a flag letting the caller know that we processed a ScriptDoc comment when trying to lex
		// the next token.
		fLexedScriptDocComment = true;
		fScriptDocText = inText;
	} else if (0 == inType) {
		// This is a single-line comment, and we want to do a bit of extra parsing to see whether we
		// have a region or endregion tag.
		bool bIsStartRegion = false, bIsEndRegion = false;
		
		// The first two characters will be //, so we want to skip those
		VIndex loc = 3;		// indexes are one-based...

		// Next, we want to skip any whitespace we find
		while (loc <= inText.GetLength() && IsWhitespace( inText.GetUniChar( loc ) )) {
			loc++;
		}

		// Next, check to see if we have "@region" or "@endregion" at the given location
		const VString kStartRegion = CVSTR( "@region " );
		const VString kEndRegion = CVSTR( "@endregion" );

		// Check to see if we have the start region.  Note that all start regions must
		// come with a name, which is why we also handle at least one space after the
		// region token.
		VString temp;
		sLONG length = (loc + kStartRegion.GetLength() - 1 > inText.GetLength()) ? (inText.GetLength() - loc - 1) : (kStartRegion.GetLength());
		if (length > 0)		inText.GetSubString( loc, length, temp );
		if (temp.EqualTo( kStartRegion, false )) {
			bIsStartRegion = true;
			loc += kStartRegion.GetLength();
		} else {
			// Check to see if we possibly have an endregion tag
			sLONG length = (loc + kEndRegion.GetLength() - 1 > inText.GetLength()) ? (inText.GetLength() - loc - 1) : (kEndRegion.GetLength());
			if (length > 0)	inText.GetSubString( loc, length, temp );
			if (temp.EqualTo( kEndRegion, false )) {
				// We *might* have an endregion token.  However, we require there to be whitespace
				// or a newline after it.  This handles the case where the user types something like
				// @endregionthatisn'ttrulyended
				loc += kEndRegion.GetLength();
				if (loc > inText.GetLength() || IsWhitespace( inText.GetUniChar( loc ) )) {
					// We really do have an end region
					bIsEndRegion = true;
				}
			}
		}

		if (bIsStartRegion) {
			// The location is currenly pointing to the start of the name.  We want to eat any whitespace that
			// might still remain though.
			while (IsWhitespace( inText.GetUniChar( loc ) )) {
				loc++;
			}

			// Now we expect there to be a name, which is from here until the end of the line (sans the line ending)
			inText.GetSubString( loc, inText.GetLength() - loc + 1, fRegionName );

			if (!fRegionName.IsEmpty()) {
				fLexedRegionComment = true;
				fRegionLineNumber = fLineNumber - 1;	// We subtract one because we've processed the end of line for the comment already
			}			
		} else if (bIsEndRegion) {
			fLexedRegionComment = true;
			fRegionName = "";
			fRegionLineNumber = fLineNumber - 1;	// We subtract one because we've processed the end of line for the comment already
		} else {
			fLastCommentText = inText;
		}
	}
}

bool JavaScriptLexer::IsStringStart( UniChar inChar, sLONG &outType, sLONG &outValue )
{
	// As per section 7.8.4 of the ECMASCript specification
	if (CHAR_APOSTROPHE == inChar) {
		outType = JavaScriptStringType::kApostropheString;
		outValue = JavaScriptTokenValues::STRING_APOSTROPHE;
		return true;
	} else if (CHAR_QUOTATION_MARK == inChar) {
		outType = JavaScriptStringType::kQuoteString;
		outValue = JavaScriptTokenValues::STRING_QUOTATION_MARK;
		return true;
	}

	return false;
}

bool JavaScriptLexer::IsStringEnd( UniChar inChar, sLONG inType )
{
	// As per section 7.8.4 of the ECMAScript specification
	if (JavaScriptStringType::kApostropheString == inType && CHAR_APOSTROPHE == inChar) {
		return true;
	} else if (JavaScriptStringType::kQuoteString == inType && CHAR_QUOTATION_MARK == inChar) {
		return true;
	}

	return false;
}

bool JavaScriptLexer::IsStringEscapeSequence( UniChar inChar, sLONG inType )
{
	// As per section 7.8.4 of the ECMAScript specification, the string type
	// doesn't matter since the escape sequence is the same either way
	if (CHAR_REVERSE_SOLIDUS == inChar)	return true;
	return false;
}

bool JavaScriptLexer::IsStringContinuationSequence( UniChar inChar, sLONG inType )
{
	// As per section 7.8.4 of the ECMAScript specification, the string type
	// doesn't matter since the escape sequence is the same either way
	if (CHAR_REVERSE_SOLIDUS == inChar)	return true;
	return false;
}


static bool isHexDigit( UniChar c )
{
	return ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'));
}

static sLONG getHexValueFromCharacter( UniChar c )
{
	if (c >= '0' && c <= '9') {
		return c - '0';
	} else if (c >= 'a' && c <= 'f') {
		return 10 + (c - 'a');
	} else if (c >= 'A' && c <= 'F') {
		return 10 + (c - 'A');	
	}

	xbox_assert( false );
	return 0;
}

static unsigned short sixteenToThePowerOf( sLONG i )
{
	// This is a specialized helper function that is called to quickly get
	// us powers of sixteen.  Basically, this is used when lexing the stream
	// of escape sequences.  We know there will never be an escape sequence of
	// more than four characters, so we only need four powers of sixteen.
	switch (i) {
		case 0:	return 1;
		case 1: return 16;
		case 2: return 256;
		case 3: return 4096;
	}
	xbox_assert( false );
	return 0;
}

static unsigned short eightToThePowerOf( sLONG i )
{
	// Similiar to sixteenToThePowerOf, for same reasons
	switch (i) {
		case 0:	return 1;
		case 1: return 8;
		case 2: return 64;
	}
	xbox_assert( false );
	return 0;	
}

bool JavaScriptLexer::ConsumeEscapeSequence( sLONG inType, UniChar &outCharacterToAdd )
{
	// As per section 7.8.4 of the ECMAScript specification.  We've found an escape
	// sequence start, so we need to continue to lexer characters from the stream
	// to determine what to add to the given buffer.  For instance, if we get \n,
	// we need to add a line feed (0x000A) to the buffer.  Some of the escape sequences
	// are a single character long.  Others are hex or unicode code points.

	// Consume the current character to see what sort of escape sequence we're dealing with
	UniChar escapeType = fLexerInput->MoveToNextChar();
	switch (escapeType) {
		case CHAR_APOSTROPHE:
		case CHAR_QUOTATION_MARK:
		case CHAR_REVERSE_SOLIDUS: {
			// These are all simple escape characters where we simply want to add the
			// character we found to the stream.
			outCharacterToAdd = escapeType;
			return true;
		} break;

		case CHAR_LATIN_SMALL_LETTER_B: {
			// Adding a BACKSPACE character to the stream
			outCharacterToAdd = (UniChar)CHAR_CONTROL_000B;
			return true;
		} break;

		case CHAR_LATIN_SMALL_LETTER_T: {
			// Adding a TAB character to the stream
			outCharacterToAdd = (UniChar)CHAR_CONTROL_0009;
			return true;
		} break;

		case CHAR_LATIN_SMALL_LETTER_N: {
			// Adding a LINE FEED character to the stream
			outCharacterToAdd = (UniChar)CHAR_CONTROL_000A;
			return true;
		} break;

		case CHAR_LATIN_SMALL_LETTER_V: {
			// Adding a VERTICAL TAB character to the stream
			outCharacterToAdd = (UniChar)CHAR_CONTROL_000B;
			return true;
		} break;

		case CHAR_LATIN_SMALL_LETTER_F: {
			// Adding a FORM FEED character to the stream
			outCharacterToAdd = (UniChar)CHAR_CONTROL_000C;
			return true;
		} break;

		case CHAR_LATIN_SMALL_LETTER_R: {
			// Adding a CARRIAGE RETURN character to the stream
			outCharacterToAdd = (UniChar)CHAR_CONTROL_000D;
			return true;
		} break;

		case CHAR_LATIN_SMALL_LETTER_X: {
			// Adding a two hex character sequence to the stream, where the value of
			// the character added is 16 * char1 + char2
			unsigned short value = 0;

			// We need to grab the next two characters from the stream
			for (sLONG i = 1; i >= 0; i--) {
				// Grab the next character and make sure it's a hex digit
				UniChar c = fLexerInput->MoveToNextChar();
				if (!isHexDigit( c ))	return false;

				// Now we want to add the hex digit we got into our ultimate value
				value += sixteenToThePowerOf( i ) * getHexValueFromCharacter( c );
			}

			// Add the value as a UniChar to our stream
			outCharacterToAdd = (UniChar)value;
			return true;
		} break;

		case CHAR_LATIN_SMALL_LETTER_U: {
			// Adding a four hex character sequence to the stream, where the value of
			// the character added is 16 ^ 3 * char1 + 16 ^ 2 * char2 + 16 * char3 + char4
			unsigned short value = 0;

			// We need to grab the next two characters from the stream
			for (sLONG i = 3; i >= 0; i--) {
				// Grab the next character and make sure it's a hex digit
				UniChar c = fLexerInput->MoveToNextChar();
				if (!isHexDigit( c ))	return false;

				// Now we want to add the hex digit we got into our ultimate value
				value += sixteenToThePowerOf( i ) * getHexValueFromCharacter( c );
			}

			// Add the value as a UniChar to our stream
			outCharacterToAdd = (UniChar)value;
			return true;
		} break;

		case CHAR_DIGIT_ZERO: {
			// If the next character is an octal digit, then we are not adding
			// this as a null character and instead we're adding an octal escape sequence
			UniChar c = fLexerInput->PeekAtNextChar();
			if (c >= '0' && c <= '7') {
				unsigned short value = 0;

				// We need to grab the next three characters from the stream and see if
				// they are octal or not
				for (sLONG i = 2; i >= 0; i--) {
					// Grab the next character and make sure it's a oct digit
					UniChar c = fLexerInput->MoveToNextChar();
					if (c < '0' && c > '7' )	return true;	// Any non-oct values are fine since we know we have at least one

					// Now we want to add the oct digit we got into our ultimate value
					value += eightToThePowerOf( i ) * (c - '0');
				}

				// Add the value as a UniChar to our stream
				outCharacterToAdd = (UniChar)value;
			} else if (c == '8' || c == '9') {
				return false;
			} else {
				// Adding a NULL character to the stream
				outCharacterToAdd = (UniChar)CHAR_CONTROL_0000;
			}
			return true;
		} break;

		default: {
			// For anything else, we just add the character we found directly to the stream
			outCharacterToAdd = escapeType;
			return true;
		} break;
	}

	return false;
}

static bool isASCIIAlpha( UniChar c )
{ 
	return (c | 0x20) >= 'a' && (c | 0x20) <= 'z'; 
}

static bool isASCII( UniChar c )
{
	return !(c & ~0x7F);
}

bool JavaScriptLexer::IsRegularExpressionStart( TokenList* inTokens, UniChar inChar )
{
    bool ret = false;
	if (CHAR_SOLIDUS == inChar)
	{
        ret = true;
        
		if ( inTokens )
		{
			sLONG i = inTokens->size() - 1;

			while ( i >= 0 )
			{
				ILexerToken* lastToken = (*inTokens)[ i ];
				ILexerToken::TYPE type = lastToken->GetType();
				VString str = lastToken->GetText();

				if (    type == ILexerToken::TT_PUNCTUATION
					 && (    str == "="
					      || str == "("
						  || str == "["
						  || str == ","
						  || str == "*"
						  || str == "+"
						  || str == "-" ) )
				{
					break;
				}
				else if ( type == ILexerToken::TT_WHITESPACE )
				{
					i--;
				}
				else
				{
					ret = false;
					break;
				}
			}
		}
	}
	return ret;
}

bool JavaScriptLexer::IsIdentifierStart( UniChar inChar )
{
	// As per section 7.6 of the ECMAScript specification.  Most identifier starts
	// are very simple to deal with.  Any unicode letter, $, _ or \u escape sequence
	// may start an identifier.

	// If we have an ASCII alpha character
    if (isASCIIAlpha( inChar ))	return true;
	// If we have the underscore character
	if (CHAR_LOW_LINE == inChar)	return true;
	// If we have the dollar sign character
	if (CHAR_DOLLAR_SIGN == inChar)	return true;
	// If we have a unicode letter (Uppercase, Lowercase, Titlecase, Modifier, Other, or Letter number category)
	if ((!isASCII( inChar ) && 
		(U_GET_GC_MASK( inChar ) & (U_GC_LU_MASK |		// Uppercase
									U_GC_LL_MASK |		// Lowercase
									U_GC_LT_MASK |		// Titlecase
									U_GC_LM_MASK |		// Letter modifier
									U_GC_LO_MASK |		// Other letter
									U_GC_NL_MASK))))	// Letter number
		return true;

	// TODO: support unicode escape sequences.  I am not supporting them currently because they are
	// infrequently used, and it would complicate the design of the lexer considerably to support them.
	// That would mean that we can no longer just peek at the character to determine whether it belongs
	// to an identifier or not -- instead, we would have to allow this call to be able to consume multiple
	// characters in order to add a single character into the stream.

	// If it's none of these, we don't have an identifier start
	return false;
}

bool JavaScriptLexer::IsIdentifierCharacter( UniChar inChar )
{
	// As per section 7.6 of the ECMAScript specification.  Anything that can start
	// an identifier can also be found within an identifier.  Beyond that, a combining mark, 
	// a digit, or connector punctuation is also allowed.

	if (IsIdentifierStart( inChar ))	return true;
	if (U_GET_GC_MASK( inChar ) & (U_GC_MN_MASK |	// Non-spacing mark
									U_GC_MC_MASK |	// Combining spacing mark
									U_GC_ND_MASK |	// Decimal number
									U_GC_PC_MASK))	// Connector punctuation
			return true;
	
	// If it's not one of these, it's not allowed as part of an identifier
	return false;
}

bool JavaScriptLexer::IsNumericStart( UniChar inChar, ENumericType &outType )
{
	// As per section 7.8.3 of the ECMAScript specification, a number can either
	// start with a ".", a decimal number, or "0x".  To see a regular expression
	// for what constitutes a numeric literal, see the comments at the start of
	// the ConsumeNumber method.
	if (CHAR_FULL_STOP == inChar) {
		outType = eNumericReal;
		return true;
	}

	// If we get a zero, we have to look to see whether the next character in the stream
	// is an x or X, because it could be that we've gotten the start of a hex digit.  So
	// we will ignore zeros for the simple "is it a decimal number" test.
	if (inChar != CHAR_DIGIT_ZERO &&
		U_GET_GC_MASK( inChar ) & U_GC_ND_MASK) {	// Decimal digit number, other than zero
		outType = eNumericInteger;
		return true;
	}

	if (inChar == CHAR_DIGIT_ZERO) {
		// It is assumed that the caller was using PeekAtNextChar to get the inChar used
		// to call this method, so we need to move past it to be able to test the char 
		// beyond it.
		//
		// It may seem a bit odd that we get a zero followed by another non-Xx character, and
		// claim that the zero isn't an integer.  However, according to the ECMA script, the
		// number cannot be followed by the start of an identifier if it is to be considered
		// a legal number.  This check is normally handled by the ConsumeNumber call, but in
		// this case, we already know the answer.  0 followed by the start of an identifier (aside
		// from X or x) or followed by another number, isn't considered a legal numeric literal.
		UniChar test = fLexerInput->MoveToNextChar();	// Move beyond the current char, which was the inChar
		xbox_assert( test == inChar );
		bool ret = false;
		if (fLexerInput->HasMoreChars()) {
			if (fLexerInput->PeekAtNextChar() == CHAR_LATIN_SMALL_LETTER_X ||
				fLexerInput->PeekAtNextChar() == CHAR_LATIN_CAPITAL_LETTER_X) {
				
				outType = eNumericHex;
				ret = true;
			} else if (fLexerInput->PeekAtNextChar() >= CHAR_DIGIT_ONE && 
						fLexerInput->PeekAtNextChar() <= CHAR_DIGIT_SEVEN) {
				outType = eNumericOct;
				ret = true;
			} else if( !IsIdentifierStart( fLexerInput->PeekAtNextChar() ) &&
						!(U_GET_GC_MASK( fLexerInput->PeekAtNextChar() ) & U_GC_ND_MASK)) {
				outType = eNumericInteger;
				ret = true;
			}
		} else {
			// A zero with no more characters in the stream is still a number
			outType = eNumericInteger;
			ret = true;
		}
		// Move back to the previous char in the stream so that the post-conditions are true
		fLexerInput->MoveToPreviousChar();
		return ret;
	}

	return false;
}

int JavaScriptLexer::GetNumericTokenValueFromNumericType( ENumericType inType )
{
	if (inType != eNumericUnknown)	return JavaScriptTokenValues::NUMBER;
	return 0;
}

static void DrainBuffer( VString *str, UniChar buffer[], VIndex &ioSize )
{
	if (ioSize)	str->AppendUniChars( buffer, ioSize );
	ioSize = 0;
}

bool JavaScriptLexer::ConsumeNumber( VString *ioNumber, TokenList *ioTokens, ENumericType &ioType )
{
	// As per section 7.8.3 of the ECMAScript specification, a numeric literal is 
	// in the following RegEx form:
	//
	// Integer		[0-9]+
	//
	// Real			[0-9]+"."[0-9]* |
	//				"."[0-9]+
	//
	// Scientific	[0-9]+[Ee][+-]?[0-9]+ |
	//				[0-9]+"."[0-9]*[Ee][+-]?[0-9]+ |
	//				"."[0-9]+[Ee][+-]?[0-9]+
	//
	// Hexadecimal	0[Xx][0-9A-Fa-f]+
	//
	// Octal		0[1-7][0-7]*
	//
	// Note that it is illegal for an identifier to immediately follow a numeric value.  For
	// instance, 3in would be entirely illegal.  Technically speaking, the next input characters
	// for a numeric value cannot be an identifier start, or a numeric literal start.
	//
	// Also note that the given type is merely a guess given to us by the IsNumericStart function.
	// In some cases, it can be comprehensive, such as a hexadecimal number or real number.  In other
	// cases, it is only an estimate, such as an integer.  Upon returning from this call, the type will
	// specify more accurately what kind of numerical value was lexed.
	UniChar					szUCharBuffer [ 128 ];
	VIndex					nBufferSize = 0;
	VIndex					nStartingPosition = fLexerInput->GetCurrentPosition();

	// The only types we especially care about are octal and hexadecimal numbers.  The rest can be treated in a
	// generic manner, but oct & hex requires some special processing to get right.
	if (eNumericHex == ioType) {
		// We were given a hex value, so we need to consume the 0x first, and then a string of hex values
		if (fLexerInput->HasMoreChars() && fLexerInput->PeekAtNextChar() != CHAR_DIGIT_ZERO)	return false;
		AppendUniCharWithBuffer( fLexerInput->MoveToNextChar(), ioNumber, szUCharBuffer, nBufferSize, 128 );
		if (!fLexerInput->HasMoreChars())	return false;
		UniChar expectedX = fLexerInput->PeekAtNextChar();
		if (CHAR_LATIN_SMALL_LETTER_X != expectedX &&
			CHAR_LATIN_CAPITAL_LETTER_X != expectedX)	return false;
		AppendUniCharWithBuffer( fLexerInput->MoveToNextChar(), ioNumber, szUCharBuffer, nBufferSize, 128 );
		DrainBuffer( ioNumber, szUCharBuffer, nBufferSize );

		// Now we require there to be at least some hexadecimal digits
		if (!fLexerInput->HasMoreChars())				return false;
		if (!ConsumeHexNumber( ioNumber ))	return false;
	} else if (eNumericOct == ioType) {
		if (fLexerInput->HasMoreChars() && fLexerInput->PeekAtNextChar() != CHAR_DIGIT_ZERO)	return false;
		AppendUniCharWithBuffer( fLexerInput->MoveToNextChar(), ioNumber, szUCharBuffer, nBufferSize, 128 );
		if (!fLexerInput->HasMoreChars())	return false;

		DrainBuffer( ioNumber, szUCharBuffer, nBufferSize );

		// Now we require there to be at least some octal digits, the first of which cannot be zero as was verified
		// by the IsNumericStart call
		if (!fLexerInput->HasMoreChars())	return false;
		if (!ConsumeOctNumber( ioNumber ))	return false;		
	} else {
		// Peek at the first character to determine whether we're going to be consuming
		// an integer, or the start of a floating-point value.
		if (fLexerInput->PeekAtNextChar() != CHAR_FULL_STOP) {
			// We're dealing with an integer value thus far, so let's consume it
			ioType = eNumericInteger;
			if (!ConsumeDecimalNumbers( ioNumber ))	return false;
		}

		// Now we want to look to see whether we have a decimal point (which means we have a 
		// real number), an e (which means we have a scientific number) or something else (which
		// means we're at the end of the numeric stream).
		if (fLexerInput->HasMoreChars() && fLexerInput->PeekAtNextChar() == CHAR_FULL_STOP) {
			// We have a decimal point, so this is the start of a real number
			ioType = eNumericReal;
			AppendUniCharWithBuffer( fLexerInput->MoveToNextChar(), ioNumber, szUCharBuffer, nBufferSize, 128 );

			// Since there's a decimal point, it is possible there will be one or more integer
			// values to follow it, however, it is not required
			// If we have anything left in the buffer to add, we add it now
			DrainBuffer( ioNumber, szUCharBuffer, nBufferSize );
			if (fLexerInput->HasMoreChars())	ConsumeDecimalNumbers( ioNumber );
		}

		// We want to check again to see if there's an E or e, which means this is a scientific number
		if (fLexerInput->HasMoreChars() &&
			(fLexerInput->PeekAtNextChar() == CHAR_LATIN_CAPITAL_LETTER_E ||
			fLexerInput->PeekAtNextChar() == CHAR_LATIN_SMALL_LETTER_E)) {
			ioType = eNumericScientific;

			// Add the E to the stream
			AppendUniCharWithBuffer( fLexerInput->MoveToNextChar(), ioNumber, szUCharBuffer, nBufferSize, 128 );

			// Now there can be an optional + or - sign
			if (fLexerInput->HasMoreChars() &&
				(fLexerInput->PeekAtNextChar() == CHAR_HYPHEN_MINUS ||
				fLexerInput->PeekAtNextChar() == CHAR_PLUS_SIGN)) {
				// Add it to the stream since we found it
				AppendUniCharWithBuffer( fLexerInput->MoveToNextChar(), ioNumber, szUCharBuffer, nBufferSize, 128 );
			}

			// Now we require a set of integer numbers for the exponent
			if (!fLexerInput->HasMoreChars())		return false;
			// If we have anything left in the buffer to add, we add it now
			DrainBuffer( ioNumber, szUCharBuffer, nBufferSize );
			if (!ConsumeDecimalNumbers( ioNumber ))	return false;
		}
	}

	if (ioTokens) {
		DrainBuffer( ioNumber, szUCharBuffer, nBufferSize );
		ioTokens->push_back( new JavaScriptLexerToken( ILexerToken::TT_NUMBER, nStartingPosition,
								fLexerInput->GetCurrentPosition() - nStartingPosition, *ioNumber, 
								GetNumericTokenValueFromNumericType( ioType ) ) );
	}

	// We now have some post conditions that need to be met.  The next character in the stream
	// cannot be the start of an identifier or the start of a number
	if (!fLexerInput->HasMoreChars())	return true;
	ENumericType toss;
	return !IsIdentifierStart( fLexerInput->PeekAtNextChar() ) && 
			!IsNumericStart( fLexerInput->PeekAtNextChar(), toss );
}

bool JavaScriptLexer::ConsumePossibleRegularExpression( VString& inBody, VString& inFlags)
{
	bool	ok = true;
	VIndex	nStartingPosition = fLexerInput->GetCurrentPosition();

	fLexerInput->MoveToNextChar();
	if ( ! GetNextRegularExpressionLiteral( &inBody, &inFlags ) )
	{
		ok = false;
		while ( nStartingPosition != fLexerInput->GetCurrentPosition() )
			fLexerInput->MoveToPreviousChar();
	}
	
	return ok;
}

int JavaScriptLexer::ConsumePossiblePunctuation( UniChar inChar, TokenList *outTokens )
{
	VIndex nStartingPosition = fLexerInput->GetCurrentPosition();
	int tk = 0;
	bool bIsComparison = false;
	VString tokenStr;
	switch (inChar) {
		case CHAR_PLUS_SIGN: {
			// Peek at the next char to see if it's another plus, which case
			// this is a ++ operator.  It's also possible we have a += sign, which
			// is an assignment
			tokenStr.AppendUniChar( fLexerInput->MoveToNextChar() );
			if (fLexerInput->HasMoreChars() && 
				CHAR_PLUS_SIGN == fLexerInput->PeekAtNextChar()) {
				tokenStr.AppendUniChar( fLexerInput->MoveToNextChar() );
				tk = JavaScriptTokenValues::INCREMENTOR;
			} else if (fLexerInput->HasMoreChars() &&
				CHAR_EQUALS_SIGN == fLexerInput->PeekAtNextChar()) {
				tokenStr.AppendUniChar( fLexerInput->MoveToNextChar() );
				tk =  JavaScriptTokenValues::ASSIGNADD;
			} else {
				tk = (int)inChar;
			}
		} break;
		case CHAR_HYPHEN_MINUS: {
			// Peek at the next char to see if it's another minus, which case
			// this is a -- operator.  It's also possible we have a -= sign, which
			// is an assignment
			tokenStr.AppendUniChar( fLexerInput->MoveToNextChar() );
			if (fLexerInput->HasMoreChars() && 
				CHAR_HYPHEN_MINUS == fLexerInput->PeekAtNextChar()) {
				tokenStr.AppendUniChar( fLexerInput->MoveToNextChar() );
				tk = JavaScriptTokenValues::DECREMENTOR;
			} else if (fLexerInput->HasMoreChars() &&
				CHAR_EQUALS_SIGN == fLexerInput->PeekAtNextChar()) {
				tokenStr.AppendUniChar( fLexerInput->MoveToNextChar() );
				tk = JavaScriptTokenValues::ASSIGNSUBTRACT;
			} else {
				tk = (int)inChar;
			}
		} break;
		case CHAR_ASTERISK: {
			// It is possible we have a *= sign, which is an assignment
			tokenStr.AppendUniChar( fLexerInput->MoveToNextChar() );
			if (fLexerInput->HasMoreChars() &&
				CHAR_EQUALS_SIGN == fLexerInput->PeekAtNextChar()) {
				tokenStr.AppendUniChar( fLexerInput->MoveToNextChar() );
				tk = JavaScriptTokenValues::ASSIGNMULTIPLY;
			} else {
				tk = (int)inChar;
			}
		} break;
		case CHAR_SOLIDUS: {
			// It is possible we have a /= sign, which is an assignment
			tokenStr.AppendUniChar( fLexerInput->MoveToNextChar() );
			if (fLexerInput->HasMoreChars() &&
				CHAR_EQUALS_SIGN == fLexerInput->PeekAtNextChar()) {
				tokenStr.AppendUniChar( fLexerInput->MoveToNextChar() );
				tk = JavaScriptTokenValues::ASSIGNDIVIDE;
			} else {
				tk = (int)inChar;
			}
		} break;
		case CHAR_PERCENT_SIGN: {
			// It is possible we have a %= sign, which is an assignment
			tokenStr.AppendUniChar( fLexerInput->MoveToNextChar() );
			if (fLexerInput->HasMoreChars() &&
				CHAR_EQUALS_SIGN == fLexerInput->PeekAtNextChar()) {
				tokenStr.AppendUniChar( fLexerInput->MoveToNextChar() );
				tk = JavaScriptTokenValues::ASSIGNMODULUS;
			} else {
				tk = (int)inChar;
			}
		} break;
		case CHAR_CIRCUMFLEX_ACCENT: {
			// It is possible we have a ^= sign, which is an assignment
			tokenStr.AppendUniChar( fLexerInput->MoveToNextChar() );
			if (fLexerInput->HasMoreChars() &&
				CHAR_EQUALS_SIGN == fLexerInput->PeekAtNextChar()) {
				tokenStr.AppendUniChar( fLexerInput->MoveToNextChar() );
				tk = JavaScriptTokenValues::ASSIGNBITXOR;
			} else {
				tk = (int)inChar;
			}
		} break;
		case CHAR_LESS_THAN_SIGN: {
			tokenStr.AppendUniChar( fLexerInput->MoveToNextChar() );
			// A less than by itself is a comparison and not a punctuator
			if (fLexerInput->HasMoreChars() && 
				CHAR_LESS_THAN_SIGN == fLexerInput->PeekAtNextChar()) {
				// We could have a left shift operator by itself, or we could
				// have an assignment here.  We need to look ahead one more token to see
				tokenStr.AppendUniChar( fLexerInput->MoveToNextChar() );
				int ret = JavaScriptTokenValues::LEFTSHIFT;
				if (fLexerInput->HasMoreChars() &&
					CHAR_EQUALS_SIGN == fLexerInput->PeekAtNextChar()) {
					tokenStr.AppendUniChar( fLexerInput->MoveToNextChar() );
					ret = JavaScriptTokenValues::ASSIGNLEFTSHIFT;
				}
				tk = ret;
			} else {
				// This is just a less than sign, which is a comparison
				bIsComparison = true;
				tk = (int)inChar;
				if (fLexerInput->HasMoreChars() &&
					CHAR_EQUALS_SIGN == fLexerInput->PeekAtNextChar()) {
					tokenStr.AppendUniChar( fLexerInput->MoveToNextChar() );
					tk = JavaScriptTokenValues::LESSTHANOREQUALTO;
				}
			}
		} break;
		case CHAR_GREATER_THAN_SIGN: {
			// A greater than by itself is a comparison and not a punctuator.  However,
			// there are two forms of right shifting -- signed and unsigned.
			tokenStr.AppendUniChar( fLexerInput->MoveToNextChar() );
			if (fLexerInput->HasMoreChars() && 
				CHAR_GREATER_THAN_SIGN == fLexerInput->PeekAtNextChar()) {
					// What we have is definitely a right shift, but is it signed or otherwise?
					// Also, we could have an assignment operator here too.  So possible values are
					// >>>, >>, >>=, >>>=
					// And we need to handle all of them!
					int ret = JavaScriptTokenValues::SIGNEDRIGHTSHIFT;
					tokenStr.AppendUniChar( fLexerInput->MoveToNextChar() );
					if (fLexerInput->HasMoreChars() &&
						CHAR_GREATER_THAN_SIGN == fLexerInput->PeekAtNextChar()) {
						tokenStr.AppendUniChar( fLexerInput->MoveToNextChar() );
						ret = JavaScriptTokenValues::UNSIGNEDRIGHTSHIFT;
						// Of course, there could be an equal sign hiding at the next token, so we
						// are not out of the woods yet!
						if (fLexerInput->HasMoreChars() &&
							CHAR_EQUALS_SIGN == fLexerInput->PeekAtNextChar()) {
							tokenStr.AppendUniChar( fLexerInput->MoveToNextChar() );
							ret = JavaScriptTokenValues::ASSIGNUNSIGNEDRIGHTSHIFT;
						}
					} else if (fLexerInput->HasMoreChars() &&
						CHAR_EQUALS_SIGN == fLexerInput->PeekAtNextChar()) {
						tokenStr.AppendUniChar( fLexerInput->MoveToNextChar() );
						ret = JavaScriptTokenValues::ASSIGNSIGNEDRIGHTSHIFT;
					}
					tk = ret;
			} else {
				// This is just a greater than sign, which is a comparison
				bIsComparison = true;
				tk = (int)inChar;
				if (fLexerInput->HasMoreChars() &&
					CHAR_EQUALS_SIGN == fLexerInput->PeekAtNextChar()) {
					tokenStr.AppendUniChar( fLexerInput->MoveToNextChar() );
					tk = JavaScriptTokenValues::GREATERTHANOREQUALTO;
				}
			}
		} break;
		case CHAR_AMPERSAND: {
			// Peek at the next char to see if it's another &, which case
			// this is a logical AND operator
			tokenStr.AppendUniChar( fLexerInput->MoveToNextChar() );
			if (fLexerInput->HasMoreChars() && 
				CHAR_AMPERSAND == fLexerInput->PeekAtNextChar()) {
				tokenStr.AppendUniChar( fLexerInput->MoveToNextChar() );
				tk = JavaScriptTokenValues::LOGICALAND;
			} else if (fLexerInput->HasMoreChars() &&
				CHAR_EQUALS_SIGN == fLexerInput->PeekAtNextChar()) {
				tokenStr.AppendUniChar( fLexerInput->MoveToNextChar() );
				tk = JavaScriptTokenValues::ASSIGNBITAND;
			} else {
				tk = (int)inChar;
			}			
		} break;
		case CHAR_VERTICAL_LINE: {
			// Peek at the next char to see if it's another &, which case
			// this is a logical OR operator
			tokenStr.AppendUniChar( fLexerInput->MoveToNextChar() );
			if (fLexerInput->HasMoreChars() && 
				CHAR_VERTICAL_LINE == fLexerInput->PeekAtNextChar()) {
				tokenStr.AppendUniChar( fLexerInput->MoveToNextChar() );
				tk = JavaScriptTokenValues::LOGICALOR;
			} else if (fLexerInput->HasMoreChars() &&
				CHAR_EQUALS_SIGN == fLexerInput->PeekAtNextChar()) {
				tokenStr.AppendUniChar( fLexerInput->MoveToNextChar() );
				tk = JavaScriptTokenValues::ASSIGNBITOR;
			} else {
				tk = (int)inChar;
			}			
		} break;
		case CHAR_EQUALS_SIGN: {
			tokenStr.AppendUniChar( fLexerInput->MoveToNextChar() );
			// An equal by itself is a punctuator, not a comparison
			if (fLexerInput->HasMoreChars() && 
				CHAR_EQUALS_SIGN == fLexerInput->PeekAtNextChar()) {
				bIsComparison = true;
				// We have == right now, but it could also be an ===
				tokenStr.AppendUniChar( fLexerInput->MoveToNextChar() );
				int ret = JavaScriptTokenValues::EQUALITY;
				if (fLexerInput->HasMoreChars() &&
					CHAR_EQUALS_SIGN == fLexerInput->PeekAtNextChar()) {
					tokenStr.AppendUniChar( fLexerInput->MoveToNextChar() );
					ret = JavaScriptTokenValues::STRICTEQUALITY;
				}
				tk = ret;
			} else {
				tk = (int)inChar;
			}
		} break;
		case CHAR_EXCLAMATION_MARK: {
			// We could have an !, !=, or !==
			tokenStr.AppendUniChar( fLexerInput->MoveToNextChar() );
			if (fLexerInput->HasMoreChars() && 
				CHAR_EQUALS_SIGN == fLexerInput->PeekAtNextChar()) {
				bIsComparison = true;
				// We have == right now, but it could also be an !==
				tokenStr.AppendUniChar( fLexerInput->MoveToNextChar() );
				int ret = JavaScriptTokenValues::NONEQUALITY;
				if (fLexerInput->HasMoreChars() &&
					CHAR_EQUALS_SIGN == fLexerInput->PeekAtNextChar()) {
					tokenStr.AppendUniChar( fLexerInput->MoveToNextChar() );
					ret = JavaScriptTokenValues::STRICTNONEQUALITY;
				}
				tk = ret;
			} else {
				tk = (int)inChar;
			}
		} break;
		case CHAR_FULL_STOP: {
			// The '.' token is a special case because it could be that this is really the start of
			// a number, in which case this isn't punctuation at all.  So we want to look at the next
			// character to see if we've gotten a number.  If so, we're not dealing with punctuation.
			fLexerInput->MoveToNextChar();	// Move past the period
			if (fLexerInput->HasMoreChars() &&
				fLexerInput->PeekAtNextChar() >= CHAR_DIGIT_ZERO &&
				fLexerInput->PeekAtNextChar() <= CHAR_DIGIT_NINE) {
				// We're actually dealing with a number, not a punctuator
				fLexerInput->MoveToPreviousChar();
				return 0;
			}
			tokenStr.AppendUniChar( CHAR_FULL_STOP );
			tk = (int)inChar;
		} break;
		case CHAR_COMMA:
		case CHAR_LEFT_PARENTHESIS:
		case CHAR_RIGHT_PARENTHESIS:
		case CHAR_SEMICOLON:
		case CHAR_COLON:
		case CHAR_QUESTION_MARK:
		case CHAR_LEFT_CURLY_BRACKET:
		case CHAR_RIGHT_CURLY_BRACKET:
		case CHAR_LEFT_SQUARE_BRACKET:
		case CHAR_RIGHT_SQUARE_BRACKET:
		case CHAR_TILDE: {
				tokenStr.AppendUniChar( fLexerInput->MoveToNextChar() );
				tk = (int)inChar;
		} break;
	}

	if (tk && outTokens) {
		outTokens->push_back( new JavaScriptLexerToken( bIsComparison ? (ILexerToken::TT_COMPARISON) : (ILexerToken::TT_PUNCTUATION),
									nStartingPosition, tokenStr.GetLength(), tokenStr, tk ) );
	}
	return tk;
}

// Skips leading semicolons in the g_vstrSQL_LEX in such a way that g_nCurrentChar_LEX 
// contains the index of the last leading semicolon and g_nCurrentChar_LEX + 1 contains
// the index of the first non-semicolon character.
// Returns the amount of semicolons skipped.
int JavaScriptLexer::SkipSemiColons()
{
	int nCount = 0;
	while (fLexerInput->HasMoreChars()) {
		UniChar uChar = fLexerInput->MoveToNextChar();
		if (uChar != ';') {
			fLexerInput->MoveToPreviousChar();
			return nCount;
		}
		nCount++;
	}
	
	return nCount;
}

bool JavaScriptLexer::AdvanceOneToken( int &outToken, TokenList *outTokens, bool inIgnoreNewLines )
{
	outToken = -1;
	fConsumedNewLine = false;
	fLexedRegionComment = false;

	// NOTE: We do not clear out the fact that we may or may not have processed a ScriptDoc comment
	// because that happens only after the user has dealt with the previous one.  This allows the 
	// consumer to attach a ScriptDoc comment to any parser element instead of just the next token
	// after the comment.

	if (!fLexerInput)	return false;

	// There can be any combination of newlines and whitespaces preceeding semantic tokens,
	// so we're going to loop until we don't find either.
	while (fLexerInput->HasMoreChars()) {

		if (fOpenStringType)
		{
			VString	vstrQuoted;
			sLONG	stringValue = fOpenStringType == JavaScriptStringType::kApostropheString ? JavaScriptTokenValues::STRING_APOSTROPHE : JavaScriptTokenValues::STRING_QUOTATION_MARK;

			if (!ConsumeString( &vstrQuoted, outTokens, fOpenStringType, stringValue ))
				return false;

			outToken = stringValue;
			fLastTokenText = vstrQuoted;
			fOpenStringType = 0;
		}
		else
		{
			bool consumedWhitespace = false;
			if (!SkipWhitespaces( consumedWhitespace, outTokens ))	return false;

			// JavaScript also treats newlines as a whitespace, but sometimes we have to care.  So we
			// handle it specially.
			bool consumedNewLine = false;
			while (fLexerInput->HasMoreChars() &&
				IsLineEnding( fLexerInput->PeekAtNextChar() )) {
					// Eat the line ending
					UniChar uChar = fLexerInput->MoveToNextChar();
					ConsumeLineEnding( uChar );

					consumedNewLine = true;

					if (!inIgnoreNewLines) {
						outToken = JavaScriptTokenValues::ENDL;
						if (outTokens) {
							outTokens->push_back( new JavaScriptLexerToken( ILexerToken::TT_WHITESPACE,	fLexerInput->GetCurrentPosition() - 1, 1, 
								VString( uChar ), outToken ) );
						}
						SetLastToken( outToken );
						return true;
					}
			}

			// If we're done consuming newlines and whitespaces, then we're done with this loop
			if (!consumedWhitespace && !consumedNewLine)	break;
		}
	}

	if (!fLexerInput->HasMoreChars())	return false;

	// Take a peek at what sort of token we're about to deal with. 
	UniChar	uChar = fLexerInput->PeekAtNextChar();
	
	sLONG stringType;
	sLONG stringValue;
	ENumericType numberType;

	VString body, flags;

	if ( IsRegularExpressionStart(outTokens, uChar) && ConsumePossibleRegularExpression(body, flags) ) {
		ILexerToken::TYPE	tokenType;
		VString				regExp = CVSTR("/") + body + CVSTR("/") + flags;

		outToken = JavaScriptTokenValues::REGEXP;
		tokenType = ILexerToken::TT_JAVASCRIPT_REGEXP;

		if (outTokens)
			outTokens->push_back( new JavaScriptLexerToken( tokenType, fLexerInput->GetCurrentPosition() - regExp.GetLength(), 
			regExp.GetLength(), regExp, outToken ) );

	} else if (outToken = ConsumePossiblePunctuation( uChar, outTokens )) {
		// If we've got a semicolon, we want to skip over it
		if (outToken == ';') {
			SkipSemiColons();
		}
	} else if (IsIdentifierStart( uChar )) {
		// The base class assumes we've consumed the first character already for this call.  We should
		// rectify this some day, as it's very confusing.
		fLexerInput->MoveToNextChar();
		VString *vstrNAME = ConsumeIdentifier();
		if (!vstrNAME) {
			return false;
		}

		ILexerToken::TYPE tokenType;
		if (outToken = IsKeyword( vstrNAME )) {
			tokenType = ILexerToken::TT_JAVASCRIPT_KEYWORD;
		} else if (outToken = IsFutureReservedWord( vstrNAME )) {
			tokenType = ILexerToken::TT_JAVASCRIPT_FUTURE_RESERVED_WORD;
		} else {
			outToken = JavaScriptTokenValues::IDENTIFIER;
			tokenType = ILexerToken::TT_NAME;
		}

		if (outTokens)	outTokens->push_back( new JavaScriptLexerToken( tokenType, fLexerInput->GetCurrentPosition() - vstrNAME->GetLength(), 
			vstrNAME->GetLength(), *vstrNAME, outToken ) );

		fLastTokenText = *vstrNAME;
		delete vstrNAME;
	} else if (IsNumericStart( uChar, numberType )) {
		VString vstrNumber;
		if (!ConsumeNumber( &vstrNumber, outTokens, numberType )) {
			// ReportLexerError ( VE_SQL_LEXER_INVALID_NUMBER, "Failed to tokenize number" );
			outToken = -1;
			return false;
		} else {
			fLastTokenText = vstrNumber;
			outToken = GetNumericTokenValueFromNumericType( numberType );
		}
	} else if (IsStringStart( uChar, stringType, stringValue )) {
		VString	vstrQuoted;
		if (!ConsumeString( &vstrQuoted, outTokens, stringType, stringValue )) {
			return false;
		}
		outToken = stringValue;
		fLastTokenText = vstrQuoted;
	} else {
		// This isn't a legal lexical token, but that's fine, we'll just treat it as an "identifier"
		// for right now.  The parser can handle the fact that it doesn't match a legal pattern
		if (outTokens)	outTokens->push_back( new JavaScriptLexerToken( ILexerToken::TT_INVALID, fLexerInput->GetCurrentPosition()-1, 1, VString( fLexerInput->MoveToNextChar() ) ) );
		outToken = -1;
		return true;
	}

	SetLastToken( outToken );
	return true;
}

bool JavaScriptLexer::GetNextRegularExpressionLiteral( VString *ioRegExBody, VString *ioRegExFlags )
{
	xbox_assert( ioRegExBody );
	xbox_assert( ioRegExFlags );

	// We need to clear the params out to ensure that they're clean
	*ioRegExBody = "";
	*ioRegExFlags = "";

	// At this point, the caller has read in a /, and determined that it is for a regular expression
	// literal instead of a division operation.  So we want to read the regular expression body, as well
	// as any possible flags and return them to the caller.
	UniChar	szUCharBuffer [ 128 ];
	VIndex	nBufferSize = 0;
	bool	endOfBody = false;

	// If there's no more text in the stream, this cannot be valid
	if (!fLexerInput->HasMoreChars())	return false;

	// The first character of the body can be any character except a line terminator, but not * or /.  If
	// we read a \, then it's an escape sequence which can be any character except a line terminator.
	UniChar uCurrentChar = fLexerInput->GetCurrentChar();
	UniChar uChar = fLexerInput->MoveToNextChar();
	
	if (CHAR_ASTERISK == uChar || CHAR_SOLIDUS == uChar ||
		U_GET_GC_MASK( uChar ) & U_GC_ZL_MASK)	return false;

	if (CHAR_REVERSE_SOLIDUS == uChar) {
		if (!fLexerInput->HasMoreChars()) return false;
		AppendUniCharWithBuffer( uChar, ioRegExBody, szUCharBuffer, nBufferSize, 128 );		
		uChar = fLexerInput->MoveToNextChar();
	}
	AppendUniCharWithBuffer( uChar, ioRegExBody, szUCharBuffer, nBufferSize, 128 );		

	// Now comes a string of characters, up until we find a /.  At that point, the body stops and the flags
	// begin.  Until then, we accept any source character aside from a line terminator.
	while (fLexerInput->HasMoreChars()) {
		uChar = fLexerInput->MoveToNextChar();

		// If we got a new line, we've failed to parse a legal regular expression
		if ( (U_GET_GC_MASK( uChar ) & U_GC_ZL_MASK) || (CHAR_CONTROL_000D == uChar) )	return false;
		
		// If we get a /, then we're on to the start of the flags
		if (CHAR_SOLIDUS == uChar)
		{
			endOfBody = true;
			break;
		}

		// If we get a \, then it's the start of an escape sequence
		if (CHAR_REVERSE_SOLIDUS == uChar) {
			if (!fLexerInput->HasMoreChars()) return false;
			AppendUniCharWithBuffer( uChar, ioRegExBody, szUCharBuffer, nBufferSize, 128 );		
			uChar = fLexerInput->MoveToNextChar();
		}
		AppendUniCharWithBuffer( uChar, ioRegExBody, szUCharBuffer, nBufferSize, 128 );		
	}

	if ( ! endOfBody )
		return false;

	// At this point, the regular expression body is finished, and it's time to start lexing the flags (if any).
	DrainBuffer( ioRegExBody, szUCharBuffer, nBufferSize );

	// We've already read in the \, so we are just looking for parts of an identifier body now
	while (fLexerInput->HasMoreChars()) {
		if (IsIdentifierCharacter( fLexerInput->PeekAtNextChar() )) {
			// This is a legal flag, so add it to our buffer
			AppendUniCharWithBuffer( fLexerInput->MoveToNextChar(), ioRegExFlags, szUCharBuffer, nBufferSize, 128 );	
		} else {
			break;
		}
	}

	// Now we've reached the end of the flags
	DrainBuffer( ioRegExFlags, szUCharBuffer, nBufferSize );

	return true;
}

int JavaScriptLexer::PeekAtNextTokenForParser()
{
	// If we've already peeked, then just return the peeked token
	if (fPeeked)	return fLastToken;

	// Otherwise, get the next token (which is assigned to fLastToken
	// automatically), and set our peeked flag
	int ret = GetNextTokenForParser( true );
	fPeeked = true;
	return ret;
}

int JavaScriptLexer::GetNextTokenForParser( bool inIgnoreNewLines )
{
	// If we peeked at a token, then just return the last one we got
	if (fPeeked) {
		int ret = fLastToken;
		fPeeked = false;	// We're no longer in peek mode though!
		return ret;
	}

	// Otherwise, get the next token in the stream
	int token;
	if (AdvanceOneToken( token, NULL, inIgnoreNewLines ))	return token;
	return -1;
}

VError JavaScriptLexer::Tokenize( TokenList &outTokens, bool inContinuationOfComment, sLONG inOpenStringType )
{
	fCommentContinuationMode = inContinuationOfComment;
	fOpenStringType = inOpenStringType;

	int	nToken = 0;
	while (AdvanceOneToken( nToken, &outTokens, true ))
		;	// Infinite loop!

	return VE_OK;
}

void JavaScriptLexer::SetLexerInput( VString *inInput )
{
	fStrLexerInput = new VLexerStringInput();
	fStrLexerInput->Init( inInput );
	SetLexerInput( fStrLexerInput );
}
