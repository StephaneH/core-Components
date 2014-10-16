
%{
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "html.lexer.h"

#define YY_NEVER_INTERACTIVE	1

#ifdef _WINDOWS
	#define	strcasecmp	_strcmpi
#endif

struct htmlLexeme *lexemeList, *lexemeListTail;

enum FoundFlags {
	kFoundNothing = 0,
	kFoundTagOpen = 1 << 0,			// <
	kFoundTagClose = 1 << 1,		// >
	kFoundTagEndOpen = 1 << 2,		// </
	kFoundTagSelfClose = 1 << 3,	// />
	kFoundCommentStart = 1 << 4,	// <!--
	kFoundCommentEnd = 1 << 5,		// -->
	
	kFoundInvalid = 1 << 31,
};

enum FoundFlags foundStatus;

int BitTest( enum FoundFlags inValues, enum FoundFlags inMask ) {
	return ((inValues & inMask) == inMask);
}

size_t StrLenUTF8( const char *inInputString )
{
	// We loop over the bytes in the null-terminated string, and look at the first
	// byte of each character.  That will tell us how many subsequent bytes are encoded
	// into a single character, allowing us to advance over the string one character
	// at a time.
	const int kZeroBytes = 192;
	const int kOneByte = 224;
	const int kTwoBytes = 240;
	const int kThreeBytes = 248;
	const int kFourBytes = 252;
	const int kFiveBytes = 256;
	
	const char *endPos = inInputString + strlen( inInputString );
	const char *curPos = inInputString;
	size_t ret = 0;
	unsigned char c;
	while (curPos < endPos) {
		ret++;
		c = *curPos;
		
		// The current character tells us how many subsequent bytes we need to advance
		// to get to the start of the next character in the stream.  Note that we're also
		// using it to advance past the current character as well (hence, everything advances
		// by one more than you'd think).
		if (c < kZeroBytes)			curPos += 1;
		else if (c < kOneByte)		curPos += 2;
		else if (c < kTwoBytes)		curPos += 3;
		else if (c < kThreeBytes)	curPos += 4;
		else if (c < kFourBytes)	curPos += 5;
		else if (c < kFiveBytes)	curPos += 6;
	}
	return ret;
}

struct htmlLexeme *AppendLexeme( int inOffset, int inLength, int inStyle  ) {
	struct htmlLexeme *item = (struct htmlLexeme *)malloc( sizeof( struct htmlLexeme ) );
	item->fOffset = inOffset;
	item->fLength = inLength;
	item->fStyle = inStyle;
	item->fNext = NULL;
	
	#if _DEBUG
	if (yytext) {
		item->fText = strdup( yytext );
	} else {
		item->fText = NULL;
	}
	#endif
	
	if (!lexemeList) {
		lexemeList = item;
		lexemeListTail = item;
	} else {
		lexemeListTail->fNext = item;
		lexemeListTail = item;
	}
	return item;
}

int GetNewLexemeOffset() {
	// We can calculate the lexeme offset by looking at the last lexeme in the list.  Our next
	// offset is the last item's offset + the last item's length.
	if (lexemeListTail) {
		return lexemeListTail->fOffset + lexemeListTail->fLength;
	}
	return 0;
}
%}

%array
%option caseless
%option noyywrap

normal			[!a-zA-Z_\x80-\xff]+[a-z0-9A-Z_\x80-\xff]*

numprefix		[+|-]?
integer			0|[1-9]+[0-9]*
intnum			{numprefix}{integer}
floatnum		{numprefix}{integer}\.{integer}
hexnum			0(x|X)[0-9a-fA-F]+
number			{intnum}|{floatnum}|{hexnum}  

commentstart	"<!--" 
commentend		"-->"

ableft			<
abright			>
abendleft		"</"
abendright		"/>"        

whitespace		[\t ]+
symbol			[\{\}\(\)\[\]\?\-\+\*\\\^\.\|\/\$\,@%`~!&_=;:']
specialicon		{symbol}|{whitespace}

string			\"[^"\n]*[\"\n]
            
%%

{commentstart} {
	int c;
    int length = 4;
    int eofRead = 0;
	 
	// We are trying to look ahead to see if we can find the closing comment token "-->".  If we can find it, then we will
	// mark the block as being a closed comment in the HTML style list and continue to lex from where we left off.  But if
	// we don't find it, then we will make the entire buffer as being an "open" comment.  Either way, we are skipping forward
	// by many characters, without using the lexer at all.
	while (c = input()) {
		if (c == EOF) {
			eofRead = 1;
			break;
		}
		
		// We've read in a character, so advance
		length++;
		
		// Check to see if we've gotten -->  Note that we're checking the current character as - before we try to do further
		// processing instead of checking for --> in one block.  We do this on purpose, because calls to unput are expensive.
		// The thought is that if we get -, then there's a significantly higher chance that we'll also find -> than if we get
		// any random character.
		if (c == '-') {
			// Grab two more characters and see if we have a ->, which means we've ended the comment
			char buf[ 2 ];
			buf[ 0 ] = input();
			buf[ 1 ] = (buf[ 0 ] != EOF) ? input() : EOF;
		
			if (buf[ 0 ] == '-' && buf[ 1 ] == '>') {
				// We found the end of the comment, so we'll keep what we've read
				AppendLexeme( GetNewLexemeOffset(), length + 2, kCompleteComment );
				break;			
			} else if (buf[ 0 ] != EOF && buf[ 1 ] != EOF) {
				// This isn't the end of the comment, so put those characters back (in order!)
				unput( buf[ 1 ] );
				unput( buf[ 0 ] );
			} else {
				// If the first character we read wasn't an EOF, then that means the second one
				// was.  But we still need to increment the length so that we can get it right for
				// the token we generate.
				length += (buf[ 0 ] != EOF) ? 1 : 0;
				eofRead = 1;
				break;
			}
		}
	}
	
	if (eofRead) {
		// If we reached the end of our input (which we should have if there's no end comment
		// token), then we want to flag this line as ending without terminating the comment
		AppendLexeme( GetNewLexemeOffset(), length, kCommentOpen );
	}
	
	// We have to handle terminating manually if we've read an EOF.  That's because we're not able
	// to put an EOF character back onto the input stream.
	if (eofRead)	yyterminate();
}

{commentend} {
	// The length is 3 because the string is -->
	AppendLexeme( GetNewLexemeOffset(), 3, kCommentClose );
}

{ableft}  {
	AppendLexeme( GetNewLexemeOffset(), 1, kTagOpen );
	if (foundStatus == kFoundNothing) {
		foundStatus |= kFoundTagOpen;
	} else {
		foundStatus = kFoundInvalid;
	}
}

{abright}  {
	AppendLexeme( GetNewLexemeOffset(), 1, kTagClose );
	if (BitTest( foundStatus, kFoundTagOpen ) || BitTest( foundStatus, kFoundTagEndOpen )) {
		foundStatus |= kFoundTagClose;
	} else {
		foundStatus = kFoundInvalid;
	}
}

{abendleft}  {
	AppendLexeme( GetNewLexemeOffset(), 2, kEndTagOpen );
	if (foundStatus == kFoundNothing) {
		foundStatus |= kFoundTagEndOpen;
	} else {
		foundStatus = kFoundInvalid;
	}
}

{abendright}  {
	AppendLexeme( GetNewLexemeOffset(), 2, kTagSelfClose );
	if (BitTest( foundStatus, kFoundTagOpen )) {
		foundStatus |= kFoundTagSelfClose;
	} else {
		foundStatus = kFoundInvalid;
	}
}

{specialicon} {
	AppendLexeme( GetNewLexemeOffset(), StrLenUTF8( yytext ), kSymbolicChar );
}

{string} {
	AppendLexeme( GetNewLexemeOffset(), StrLenUTF8( yytext ), kString );
}

{number} {
	AppendLexeme( GetNewLexemeOffset(), StrLenUTF8( yytext ), kNumber );
}

{normal} {
	if (IsKeyword( yytext )) {
		AppendLexeme( GetNewLexemeOffset(), StrLenUTF8( yytext ), kKeyword );
	} else {
		AppendLexeme( GetNewLexemeOffset(), StrLenUTF8( yytext ), kNonKeyword );
	}
}

%%

struct htmlLexeme *parseHTML( const char *inStr )
{
	struct htmlLexeme *ret = NULL;

	lexemeList = NULL;
	lexemeListTail = NULL;

	foundStatus = kFoundNothing;

	yy_switch_to_buffer( yy_scan_string( inStr ) );
	yylex(); 

	yy_delete_buffer( YY_CURRENT_BUFFER );
	
	ret = lexemeList;
	lexemeList = NULL;
	lexemeListTail = NULL;
	
	return ret;
}

void FreeLexemeList( struct htmlLexeme *inList )
{
	while (inList) {
		struct htmlLexeme *nextItem = inList->fNext;
		#if _DEBUG
		if (inList->fText) {
			free( inList->fText );
		}
		#endif
		free( inList );
		inList = nextItem;
	}
}

int IsKeyword( const char *inText )
{
	const char *kKeywords[] = {
		"!DOCTYPE", 
		"A", "ACRONYM", "ADDRESS", "APPLET", "AREA", 
		"B", "BASE", "BASEFONT", "BGSOUND", "BIG", "BLOCKQUOTE", "BODY", "BR", "BUTTON", 
		"CAPTION", "CENTER", "CITE", "CODE", "COL", "COLGROUP", "COMMENT", 
		"DD", "DEL", "DFN", "DIR", "DIV", "DL", "DT", 
		"EM", "EMBED", 
		"FIELDSET", "FONT", "FORM", "FRAME", "FRAMESET", 
		"HEAD", "H1", "H2", "H3", "H4", "H5", "H6", "HR", "HTML", 
		"I", "IFRAME", "IMG", "INPUT", "INS", "ISINDEX", 
		"KBD", 
		"LABEL", "LEGEND", "LI", "LINK", "LISTING", 
		"MAP", "MARQUEE", "MENU", "META", 
		"NOBR", "NOFRAMES", "NOSCRIPT", 
		"OBJECT", "OL", "OPTION", 
		"P", "PARAM", "PLAINTEXT", "PRE", 
		"Q", 
		"S", "SAMP", "SCRIPT", "SELECT", "SMALL", "SPAN", "STRIKE", "STRONG", "STYLE", "SUB", "SUP", 
		"TABLE", "TBODY", "TD", "TEXTAREA", "TFOOT", "TH", "THEAD", "TITLE", "TR", "TT", 
		"U", "UL", 
		"VAR", 
		"WBR", 
		"XMP"
	};
	int i ;
	for (i = 0; i < (sizeof( kKeywords ) / sizeof( const char * )); i++) {
		if (strcasecmp( kKeywords[ i ], inText ) == 0) {
			return 1;
		}
	}	
	
	return 0;
}