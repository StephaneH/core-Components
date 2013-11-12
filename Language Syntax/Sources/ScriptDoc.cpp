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

#include "LanguageSyntaxBagKeys.h"
#include "ScriptDoc.h"
#include "unicode/uchar.h"


ScriptDocLexer::Element *ScriptDocLexer::GetElement()
{
	return fLastElement;
}

bool ScriptDocLexer::IsScriptDocStart( UniChar ch )
{
	// The start of a script doc must be: /**
	if (CHAR_SOLIDUS != ch)	return false;
	
	// We need to eat the solidus before we can move on to test the other stuff
	fLexerInput->MoveToNextChar();

	// Now test to see if there is an asterisk
	ch = fLexerInput->HasMoreChars() ? fLexerInput->MoveToNextChar() : -1;
	if (ch != CHAR_ASTERISK) {
		fLexerInput->MoveToPreviousChar();	// The character we just read
		fLexerInput->MoveToPreviousChar();	// The solidus
		return false;
	}

	// Now test to see if there is a second asterisk
	ch = fLexerInput->HasMoreChars() ? fLexerInput->MoveToNextChar() : -1;
	if (ch != CHAR_ASTERISK) {
		fLexerInput->MoveToPreviousChar();	// The character we just read
		fLexerInput->MoveToPreviousChar();	// The asterisk
		fLexerInput->MoveToPreviousChar();	// The solidus
		return false;
	}

	// Put back the three characters we read
	fLexerInput->MoveToPreviousChar();	// The second asterisk
	fLexerInput->MoveToPreviousChar();	// The first asterisk
	fLexerInput->MoveToPreviousChar();	// The solidus

	return true;
}

bool ScriptDocLexer::ConsumeScriptDocStart()
{
	// Consume the /**.  We can make these assertions
	// because the IsScriptDocStart function has already ascertained that this data
	// is true by peeking at characters.  We're merely eating them.
	if (!testAssert( fLexerInput->MoveToNextChar() == CHAR_SOLIDUS ))	return false;
	if (!testAssert( fLexerInput->MoveToNextChar() == CHAR_ASTERISK ))	return false;
	if (!testAssert( fLexerInput->MoveToNextChar() == CHAR_ASTERISK ))	return false;

	// Anything else up until the end of the line is simply eaten
	while (fLexerInput->HasMoreChars()) {
		UniChar uChar = fLexerInput->MoveToNextChar();
		if (IsLineEnding( uChar )) {
			ConsumeLineEnding( uChar );
			break;
		}
	}

	return fLexerInput->HasMoreChars();
}

bool ScriptDocLexer::ConsumeTypes( VString &outTypes, bool &outDoFurtherProcessing )
{
	outDoFurtherProcessing = true;

	// We expect the next character to be an open curly bracket if the user has specified
	// type information for the parameter
	bool curlyBracketMode = CHAR_LEFT_CURLY_BRACKET == fLexerInput->PeekAtNextChar();

	if ( curlyBracketMode )
	{
		fLexerInput->MoveToNextChar();
		if ( ! fLexerInput->HasMoreChars() )
			return false;
	}

	// Now we expect to read the type list
	while (true)
	{
		if ( ! fLexerInput->HasMoreChars() )
			return false;

		UniChar ch = fLexerInput->MoveToNextChar();

		if ( IsLineEnding( ch ) )
		{ 
			ConsumeLineEnding( ch );
			if (curlyBracketMode)
				outDoFurtherProcessing = false; 
			break;
		}

		if (curlyBracketMode && CHAR_RIGHT_CURLY_BRACKET == ch)
			break;

		outTypes.AppendUniChar( ch );
	}
	return true;
}

bool ScriptDocLexer::ConsumeName( VString &outName, bool &outIsOptional, bool &outDoFurtherProcessing )
{
	outIsOptional = false;
	outDoFurtherProcessing = true;
	while (true) {
		if (!fLexerInput->HasMoreChars())	return false;
		UniChar ch = fLexerInput->MoveToNextChar();

		if (CHAR_LEFT_SQUARE_BRACKET == ch) {
			// If the name is enclosed in [], then it's considered to be an optional
			// parameter.
			if (outName.GetLength() == 0) {
				outIsOptional = true;
			} else {
				return false;
			}
		} else if (IsLineEnding( ch )) {
			// We've reached the end of the tag, and the user has not specified a comment, which
			// is fine.  We'll just break out now and not do any further processing.
			ConsumeLineEnding( ch );
			outDoFurtherProcessing = false;
			break;
		} else if (IsWhitespace( ch ) || (outIsOptional && CHAR_RIGHT_SQUARE_BRACKET == ch)) {
			break;
		} else {
			outName.AppendUniChar( ch );
		}
	}
	return true; 
}

bool ScriptDocLexer::ConsumeTypeTag()
{
	bool toss;

	if ( ! SkipWhitespaces( toss ) )
		return false;

	if (!fLexerInput->HasMoreChars() )
		return false;

	VString	typeList;
	bool	bDoFurtherProcessing;
	bool	bracketMode = (fLexerInput->PeekAtNextChar() == CHAR_LEFT_CURLY_BRACKET);

	ConsumeTypes( typeList, bDoFurtherProcessing );

	if ( ! SkipWhitespaces( toss ) )
		return false;

	UniChar ch = fLexerInput->MoveToNextChar();
	if (IsLineEnding( ch ))
		ConsumeLineEnding( ch );
	else
		fLexerInput->MoveToPreviousChar();

	fLastElement = new TypeElement( typeList );
	return true;
}


bool ScriptDocLexer::ConsumeParamTag()
{
	// Param tags are one of the most complex ScriptDoc tags.  It consists of a list of
	// types followed by a name and a description.  We've already processed the @param
	// part of the element, so we want to find the type list.
	UniChar ch;
	bool toss;
	if (!SkipWhitespaces( toss ))		return false;
	if (!fLexerInput->HasMoreChars())	return false;

	// Unfortunately, there's no real concensus on how this tag is handled.  There's the
	// ScriptDoc Way, but then there's also a very common variation.  Basically, it can
	// either be @param {types} Name Comments or @param Name {types} Comments, and we
	// really want to support both versions.  Normal order is the ScriptDoc way.
	bool bIsNormalOrder = (fLexerInput->PeekAtNextChar() == CHAR_LEFT_CURLY_BRACKET);

	bool bDoFurtherProcessing;
	VString typeList, paramName, description;
	bool isOptional;

	if (bIsNormalOrder)
	{
		ConsumeTypes( typeList, bDoFurtherProcessing );
		if (!SkipWhitespaces( toss ))
			return false;
		if (!ConsumeName( paramName, isOptional, bDoFurtherProcessing ) || paramName.IsEmpty() )
			return false;
	}
	else
	{
		if (!ConsumeName( paramName, isOptional, bDoFurtherProcessing ))
			return false;
		if (bDoFurtherProcessing)
			ConsumeTypes( typeList, bDoFurtherProcessing );
	}

	if (bDoFurtherProcessing)
	{
		if (!SkipWhitespaces( toss ))
			return false;
		// Now we can find the description of the parameter
		ConsumeComment( description );
	}

	fLastElement = new ParamElement( paramName, typeList, description, isOptional );
	return true;
}

bool ScriptDocLexer::ConsumeParamValueTag()
{
	// Param value tags consists of a list of values followed by a name.
	UniChar ch;
	bool toss;
	if (!SkipWhitespaces( toss ))		return false;
	if (!fLexerInput->HasMoreChars())	return false;

	VString	valueList, paramName;
	bool	isOptional, bDoFurtherProcessing;

	ConsumeTypes( valueList, bDoFurtherProcessing );
	if (!SkipWhitespaces( toss ))
		return false;
	if (!ConsumeName( paramName, isOptional, bDoFurtherProcessing ) || paramName.IsEmpty() )
		return false;

	fLastElement = new ParamValueElement( paramName, valueList );
	return true;
}


bool ScriptDocLexer::ConsumeReturnTag()
{
	UniChar	ch;
	bool	toss;

	if ( ! SkipWhitespaces( toss ) )
		return false;

	if ( ! fLexerInput->HasMoreChars() )
		return false;

	// We expect the next character to be an open curly bracket.
	if ( CHAR_LEFT_CURLY_BRACKET != fLexerInput->MoveToNextChar() )
		return false;

	if ( ! fLexerInput->HasMoreChars() )
		return false;

	// Now we expect to read the type list
	VString typeList;
	while (true)
	{
		if ( ! fLexerInput->HasMoreChars() )
			return false;

		ch = fLexerInput->MoveToNextChar();

		if ( IsLineEnding( ch ) )
		{
			ConsumeLineEnding( ch );
			return false; 
		}

		if (CHAR_RIGHT_CURLY_BRACKET == ch)
			break;
		typeList.AppendUniChar( ch );
	}

	if ( ! SkipWhitespaces( toss ) )
		return false;

	// Now we can find the description of the return value
	VString description;
	ConsumeComment( description );

	fLastElement = new ReturnElement( typeList, description );
	return true;
}

bool ScriptDocLexer::ConsumeScriptDocTag()
{
	// We need to consume the @ token for the tag.  We know it exists because
	// we've already peeked at it
	if (!testAssert( fLexerInput->MoveToNextChar() == CHAR_COMMERCIAL_AT ))	return false;

	// After the @ symbol comes the name of the tag, followed by some whitespace.  Any
	// whitespace between the @ and the tag name will be eaten
	bool toss = false;
	if (!SkipWhitespaces( toss ))	return false;

	UniChar ch;
	VString text;
	bool bLineEnded = false;
	while (true) {
		if (!fLexerInput->HasMoreChars())	return false;
		ch = fLexerInput->MoveToNextChar();
		if (IsWhitespace( ch )) break;
		if (IsLineEnding( ch )) {
			ConsumeLineEnding( ch );
			bLineEnded = true;
			break;
		}
		text.AppendUniChar( ch );
	}

	// Skip any whitespace between the tag name and the tag contents
	if (!SkipWhitespaces( toss ))	return false;

	// Now that we have the tag name, we can create the appropriate class to represent
	// it.
	VString simpleText;
	if (text == "class" && ConsumeComment( simpleText ))
	{
		fLastElement = new ClassElement( simpleText );
	}
	else if (text == "constructor")
	{
		fLastElement = new ConstructorElement();
	}
	else if (text == "deprecated" && ConsumeComment( simpleText ))
	{
		fLastElement = new DeprecatedElement(simpleText);
	}
	else if ( (text == "extends" || text == "augments" || text == "inherits") && ConsumeComment( simpleText ))
	{
		fLastElement = new ExtendsElement(simpleText);
	}
	else if (text == "inherits" && ConsumeComment( simpleText ))
	{
		fLastElement = new InheritsElement( simpleText );
	}
	else if (text == "param")
	{
		return ConsumeParamTag();
	}
	else if (text == "paramValue")
	{
		return ConsumeParamValueTag();
	}
	else if (text == "private")
	{
		fLastElement = new PrivateElement();
	}
	else if (text == "return" || text == "returns" || text == "result")
	{
		return ConsumeReturnTag();
	}
	else if (text == "type")
	{
		return ConsumeTypeTag();
	}
	else
	{
		// This is an unsupported tag, so make an Unknown tag to handle it.  Unknown tags can
		// have comments following them, or just a newline signifying it's time for another tag.
		// We don't know which it is supposed to be, so we will skip whitespaces and check to see
		// if there's a newline.  If there is, then it's assumed to be a tag with no comments.
		if (bLineEnded || ConsumeComment( simpleText ))
		{
			fLastElement = new UnknownElement( text, simpleText );
		}
		else
		{
			return false;
		}
	}
	return true;
}

bool ScriptDocLexer::ConsumeComment( VString &outComment )
{
	// We found the start of a comment, so we want to consume it in its entirety.  The
	// only tricky part to comments is that they can span multiple lines, but we don't
	// want to include the starting asterisk as part of the comment itself.
	UniChar szUCharBuffer [ 256 ];
	VIndex nBufferSize = 0;

	while (true) {
		// If we're out of characters, we need to bail out
		if (!fLexerInput->HasMoreChars())	return false;

		// Check the next character -- if it's a line ending, we need to handle it specially,
		// but if it's not, then we can add it to our comment buffer
		UniChar ch = fLexerInput->MoveToNextChar();
		if (IsLineEnding( ch )) {
			ConsumeLineEnding( ch );

			// When we have a line ending, we don't know whether we're dealing with a continuation
			// of the comment, or the start of a tag section, or perhaps even the end of the 
			// ScriptDoc section itself.  So we need to proceed with caution.  For anything other than
			// a continuation of the comment, we want to bail out.
			//
			// Start by skipping any whitespaces we find
			bool toss = false;
			if (!SkipWhitespaces( toss ))	return false;

			// Store the line ending off so we can possibly stuff it into the user's buffer
			UniChar lineEnding = ch;

			// Next, we expect to see the asterisk that is required.  However, it could also be that
			// we have found the end of the script doc itself.
			ch = fLexerInput->HasMoreChars() ? fLexerInput->PeekAtNextChar() : -1;
			if (-1 == ch)	return false;
			if (ch != CHAR_ASTERISK)	return false;
			do {
				ch = fLexerInput->MoveToNextChar();
			} while (ch == CHAR_ASTERISK);
			if (ch == CHAR_SOLIDUS)		{ fLexerInput->MoveToPreviousChar(); fLexerInput->MoveToPreviousChar(); break; }
			fLexerInput->MoveToPreviousChar();

			// Now we want to skip any whitespaces we find between the asterisk and the rest of the comment.
			TokenList spaces;
			if (!SkipWhitespaces( toss, &spaces ))	return false;

			// The next trick is to see whether we've got a user tag, which follows the asterisk
			ch = fLexerInput->HasMoreChars() ? fLexerInput->PeekAtNextChar() : -1;
			if (-1 == ch)	return false;
			if (IsScriptDocTag( ch )) {
				// In order for this to lex properly on the next pass through AdvanceOneToken, we actually need
				// to put the asterisk back, as well as any of the spaces we've read.
				for (TokenList::iterator iter = spaces.begin(); iter != spaces.end(); ++iter) {
					ILexerToken *token = *iter;
					for (int i = 0; i < token->GetLength(); i++)	fLexerInput->MoveToPreviousChar();
				}

				// Put back the asterisk as well
				fLexerInput->MoveToPreviousChar();
				
				// We've processed the end of the comment, so we want to bail out now
				break;
			}

			// We are going to preserve the user's formatting by including the newlines they use.
			AppendUniCharWithBuffer( lineEnding, &outComment, szUCharBuffer, nBufferSize, 256 );
		} else {
			AppendUniCharWithBuffer( ch, &outComment, szUCharBuffer, nBufferSize, 256 );
		}
	}
	
	if (nBufferSize > 0)	outComment.AppendUniChars( szUCharBuffer, nBufferSize );

	return true;
}

bool ScriptDocLexer::AdvanceOneToken( int &outToken )
{
	// Clear out our last token's text and any tag values we've stored up
	fLastElement = NULL;

	// Start by skipping any whitespaces that we run into
	bool consumedWhitespace = false;
	if (!SkipWhitespaces( consumedWhitespace ))	return false;

	// Now snag the next character from the stream so we can determine how
	// to handle it.
	if (!fLexerInput->HasMoreChars())	return false;
	UniChar ch = fLexerInput->PeekAtNextChar();

	if (IsScriptDocStart( ch )) {
		ConsumeScriptDocStart();
		outToken = ScriptDocTokenValues::START;
		return true;
	}
	
	// Now that we've handled the start and stop cases for the tag, we need to consume
	// some more data from the line.  Every ScriptDoc line starts with at least one asterisk, 
	// but possibly more.  We will eat all of the asterisks we find.
	if (ch != CHAR_ASTERISK)	return false;
	do {
		ch = fLexerInput->MoveToNextChar();
	} while (ch == CHAR_ASTERISK);

	// If the very next character is a solidus, that means we've reached the end of the scriptdoc
	// comment
	if (ch == CHAR_SOLIDUS) {
		outToken = ScriptDocTokenValues::END;
		return true;
	}

	// Put the last token we grabbed during asterisk processing back onto the stream
	fLexerInput->MoveToPreviousChar();

	// Now we can skip any whitespaces that remain
	if (!SkipWhitespaces( consumedWhitespace ))	return false;

	// Having done that, we can continue to look for semantic information.
	ch = fLexerInput->HasMoreChars() ? fLexerInput->PeekAtNextChar() : -1;

	if (-1 == ch) {
		// This is a santity check for the end of input
		return false;
	} else if (IsLineEnding( ch )) {
		ConsumeLineEnding( fLexerInput->MoveToNextChar() );
		// If we hit a line ending, then we just want to start this whole process over again
		return AdvanceOneToken( outToken );
	} else if (IsScriptDocTag( ch )) {
		// We found a ScriptDoc tag, so let's eat it
		fFoundTags = true;
		if (!ConsumeScriptDocTag())	return false;
		outToken = ScriptDocTokenValues::ELEMENT;
		return true;
	} else {
		// The only thing left is the comment section.  Each ScriptDoc can have one and only
		// one comment to it, and it is supposed to be found before any tags.  It can span multiple
		// lines.
		if (fFoundComment || fFoundTags)				return false;
		fFoundComment = true;
		VString text;
		if (!ConsumeComment( text ))							return false;
		fLastElement = new CommentElement( text );
		outToken = ScriptDocTokenValues::ELEMENT;
		return true;
	}

	return true;
}

int ScriptDocLexer::GetNextTokenForParser()
{
	int ret = -1;
	if (AdvanceOneToken( ret )) {
		return ret;
	}
	return -1;
}

ScriptDocComment *ScriptDocComment::Create( const VString &text )
{
	ScriptDocLexer *lexer = new ScriptDocLexer( const_cast< VString * >( &text ) );

	// If the first token we lex isn't a start token, then we've got a problem
	if (ScriptDocTokenValues::START != lexer->GetNextTokenForParser())
	{	
		delete lexer;
		return NULL;
	}

	ScriptDocComment *ret = new ScriptDocComment( text );
	while (true) {
		int tk = lexer->GetNextTokenForParser();

		// If we reach the end of input before finding an END token, this isn't a valid
		// ScriptDoc comment
		if (-1 == tk) {
			delete lexer;
			delete ret;
			return NULL;
		}

		// If we find the ending token, then we're done parsing
		if (ScriptDocTokenValues::END == tk)	break;

		if (ScriptDocTokenValues::ELEMENT == tk) {
			ret->fElements.push_back( lexer->GetElement() );
		} else {
			delete lexer;
			delete ret;
			return NULL;
		}
	}

	// If there are more tokens past the end, then something has gone wrong
	if (-1 != lexer->GetNextTokenForParser()) {
		delete lexer;
		delete ret;
		return NULL;
	}

	delete lexer;

	return ret;
}

ScriptDocComment::~ScriptDocComment()
{
	while (!fElements.empty()) {
		fElements.back()->Release();
		fElements.pop_back();
	}
}

ScriptDocLexer::Element *ScriptDocComment::GetTargetElement( const VString &inTarget )
{
	// Loop over all of the elements and find the first one which
	// targets the given name
	for (std::vector< ScriptDocLexer::Element * >::iterator iter = fElements.begin(); iter != fElements.end(); ++iter) {
		if ((*iter)->Targets( inTarget ))	return *iter;
	}
	return NULL;
}

ScriptDocLexer::Element *ScriptDocComment::GetElement( IScriptDocCommentField::ElementType inType )
{
	for (std::vector< ScriptDocLexer::Element * >::iterator iter = fElements.begin(); iter != fElements.end(); ++iter) {
		if ((*iter)->Type() == inType)	return *iter;
	}
	return NULL;
}

void ScriptDocComment::GetElements( IScriptDocCommentField::ElementType inType, std::vector<ScriptDocLexer::Element *>& outElements )
{
	for (std::vector< ScriptDocLexer::Element * >::iterator iter = fElements.begin(); iter != fElements.end(); ++iter) {
		if ((*iter)->Type() == inType)
			outElements.push_back(*iter);
	}
}


VString ScriptDocLexer::ParamElement::FormatTextForDisplay()
{
	VString ret;

	if (fOptional)	ret += "Optional ";
	ret += "Parameter Name: " + fName;
	ret += ", Type: " + fTypes;
	if (!fDesc.IsEmpty()) {
		ret += VString( (UniChar)CHAR_CONTROL_000D ) + fDesc;
	}
	return ret;
}

VString ScriptDocLexer::ParamValueElement::FormatTextForDisplay()
{
	VString ret;

	ret += "Parameter Name: " + fName;
	ret += ", Values: " + fValues;
	return ret;
}