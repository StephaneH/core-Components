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

#include "HTMLParser.h"
#include "HTMLLexer.h"

HTMLParser::State::State()
{
	fCurrentState = kDefault;
}

HTMLParser::State *HTMLParser::State::Clone() const
{
	State *ret = new State();
	ret->fCallback = fCallback;
	ret->fCallbackCookie = fCallbackCookie;
	ret->fCurrentState = fCurrentState;

	return ret;
}

HTMLParser::HTMLParser()
{
	fLexer = new HTMLLexer();
}

HTMLParser::~HTMLParser()
{
	delete fLexer;
}

void HTMLParser::Parse( const VString &inSource, const State *inState, State **outState, const ICallback *inCallback, const void *inCallbackCookie )
{
	// First, we want to take the source code we've been fed and hand it over to the lexer
	// to turn into a list of lexemes.  This makes our parsing job much easier while still
	// retaining positional information.
	TokenList lexemes;
	fLexer->SetLexerInput( const_cast< VString * >( &inSource ) );
	fLexer->Tokenize( lexemes );		// TODO: check the state for comment continuation

	if (inState) {
		*outState = inState->Clone();
	} else {
		*outState = new State();
	}
	State *retState = *outState;

	retState->fCallback = const_cast< ICallback * >( inCallback );
	retState->fCallbackCookie = inCallbackCookie;

	// One assumption that needs to be made is that there are really two modes that the user
	// is parsing in.  The first mode is parsing a completed document, where all of the tokens
	// are available to read.  The second mode is a stuttering parse, where the parse stops and
	// starts multiple times over the course of the document.  It's easy to code with the first
	// case in mind, but much harder for the second case.  That's why we have a state object that
	// is passed around.  It allows us a place to store stateful information so that stuttering
	// parses can still be completed.  But that doesn't excuse us from all responsibility when it
	// comes to the parser design.  One problem that's easy to overlook is partially completed state
	// changes.  For instance, parsing whitespace is typically something that happens between interesting
	// tokens, but could split across parse boundaries.  To this end, you basically can only assume that
	// you have one token to parse at any given point in time.  For things which require multiple tokens
	// to be fully parsed, you will have to make use of the state object to store existing parsed information
	// so that restarting the parse can pick up from where it left off.

	// Now that we've got the lexemes, we can start to parse from our previous state information.
	for (TokenList::iterator iter = lexemes.begin(); iter != lexemes.end();) {
		switch (retState->fCurrentState) {
			case State::kEndOfInput:
			case State::kDefault: {
				// We're just consolidating tokens at this point.  We're combining text and whitespace
				// tokens into a single text token, until we find something that's not text or whitespace.
				// We treat the end of input state as being "default" because they both basically mean the
				// same thing: "no idea, you figure it out."  However, this allows us to restart parsing once
				// we've reached the end of the input stream without having to reset anything.
				ProcessDefault( iter, lexemes.end(), retState );
			} break;

			case State::kParsingEntities: {
				ICallback::BasicInformation bi = { 0 };
				bi.fCookie = retState->fCallbackCookie;
				bi.fStartPosition = (*iter)->GetPosition();
				bi.fLength = (*iter)->GetLength();
				retState->fCallback->EntityParsed( &bi, (*iter)->GetText() );
				retState->fCurrentState = State::kDefault;
				++iter;
			} break;
			case State::kParsingComments: {
				ICallback::BasicInformation bi = { 0 };
				bi.fCookie = retState->fCallbackCookie;
				bi.fStartPosition = (*iter)->GetPosition();
				bi.fLength = (*iter)->GetLength();
				retState->fCallback->CommentParsed( &bi, (*iter)->GetType() == ILexerToken::TT_OPEN_COMMENT );
				retState->fCurrentState = State::kDefault;
				++iter;
			} break;

			case State::kParsingEndTag: {
				ProcessEndTag( ++iter, lexemes.end(), retState );
			} break;

			case State::kParsingStartTag: {
				ProcessStartTag( ++iter, lexemes.end(), retState );
			} break;

			case State::kParsingAttributeName: {
				ProcessAttributeName( iter, lexemes.end(), retState );
			} break;

			case State::kParsingAttributeValue: {
				ProcessAttributeValue( iter, lexemes.end(), retState );
			} break;

			case State::kParsingBangTag: {
				ProcessBangTag( ++iter, lexemes.end(), retState );
			} break;

			case State::kSkipToCloseBracket: {
				SkipToCloseBracket( iter, lexemes.end(), retState );
			} break;

			case State::kParsingDocType: {
				// We're processing a DOCTYPE declaration -- and we've found the DOCTYPE tag and already eaten it
				// Next, we can eat whitespace
				SkipWhitespace( ++iter, lexemes.end(), retState );

				// Then we expect to get a name.  Generally speaking, this should be HTML, but it's up to the
				// user to deal with it
				if (ExpectType( iter, lexemes.end(), ILexerToken::TT_NAME )) {
					ICallback::BasicInformation bi = { 0 };
					bi.fCookie = retState->fCallbackCookie;
					bi.fStartPosition = (*iter)->GetPosition();
					bi.fLength = (*iter)->GetLength();
					retState->fCallback->DocTypeNameParsed( &bi, (*iter)->GetText() );
					retState->fCurrentState = State::kParsingDocTypeExternalType;
				}
			} break;

			case State::kParsingDocTypeExternalType: {
				// We're going to parse the SGML external type information next
				SkipWhitespace( ++iter, lexemes.end(), retState );

				// Now we expect to get PUBLIC or SYSTEM
				if (ExpectType( iter, lexemes.end(), ILexerToken::TT_NAME ) &&
					((*iter)->GetText() != "PUBLIC" || (*iter)->GetText() != "SYSTEM")) {
					retState->fCurrentState = State::kParsingDocTypePublicIdentifier;
				}
			} break;

			case State::kParsingDocTypePublicIdentifier: {
				// We've already processed the PUBLIC or SYSTEM information, now we are looking for
				// a public identifier.  This is a string literal -- and it could be followed by a
				// second string literal.
				SkipWhitespace( ++iter, lexemes.end(), retState );

				// And now we want our string literal
				if (ExpectValue( iter, lexemes.end(), HTMLLexemes::STRING )) {
					ICallback::BasicInformation bi = { 0 };
					bi.fCookie = retState->fCallbackCookie;
					bi.fStartPosition = (*iter)->GetPosition();
					bi.fLength = (*iter)->GetLength();
					retState->fCallback->DocTypeValueParsed( &bi, (*iter)->GetText() );
					retState->fCurrentState = State::kParsingSystemIdentifier;
				}
			} break;

			case State::kParsingSystemIdentifier: {
				// We're either going to be getting another string for the system identifier, or a closing
				// bracket.  If we get the string, we report it.  If we get the bracket, we ignore it
				SkipWhitespace( ++iter, lexemes.end(), retState );

				// And now we want our string literal
				if (ExpectValue( iter, lexemes.end(), HTMLLexemes::STRING )) {
					ICallback::BasicInformation bi = { 0 };
					bi.fCookie = retState->fCallbackCookie;
					bi.fStartPosition = (*iter)->GetPosition();
					bi.fLength = (*iter)->GetLength();
					retState->fCallback->DocTypeValueParsed( &bi, (*iter)->GetText() );
					retState->fCurrentState = State::kSkipToCloseBracket;
				} else if (ExpectValue( iter, lexemes.end(), CHAR_GREATER_THAN_SIGN )) {
					retState->fCurrentState = State::kDefault;
				} else {
					retState->fCurrentState = State::kSkipToCloseBracket;
				}
			} break;
		}
	}
}

void HTMLParser::SkipWhitespace( TokenList::iterator &ioStart, TokenList::iterator end, State *ioState )
{
	// We want to skip any optional whitespaces
	while (ioStart != end) {
		if ((*ioStart)->GetType() != ILexerToken::TT_WHITESPACE)	break;
		++ioStart;
	}
}

void HTMLParser::ProcessDefault( TokenList::iterator &ioStart, TokenList::iterator end, State *ioState )
{
	VIndex startPosition = (*ioStart)->GetPosition();
	VString buffer;
	while (ioStart != end) {
		// Combine all text and whitespace into a single token representing a TEXT token
		ILexerToken *curToken = *ioStart;

		if (curToken->GetType() == ILexerToken::TT_WHITESPACE ||
			curToken->GetType() == ILexerToken::TT_NAME) {
			// This is part of the current text token
			buffer.AppendString( curToken->GetText() );
		} else {
			break;
		}
		++ioStart;
	}

	// At this point, we may have consolidated several tokens into a single text token, which
	// we can now pass back to the caller
	if (!buffer.IsEmpty()) {
		ICallback::BasicInformation bi = { 0 };
		bi.fCookie = ioState->fCallbackCookie;
		bi.fStartPosition = startPosition;
		bi.fLength = buffer.GetLength();
		ioState->fCallback->TextParsed( &bi );
	}

	// Now, we want to look at the current token (as defined by start) to yield us the next state.  This assumes
	// that we're not at the end of input
	State::Category retState = State::kDefault;
	if (ioStart != end) {
		switch ((*ioStart)->GetType()) {
			case ILexerToken::TT_COMMENT:
			case ILexerToken::TT_OPEN_COMMENT:
			case ILexerToken::TT_SPECIAL_4D_COMMENT: {
				retState = State::kParsingComments;
			} break;

			case ILexerToken::TT_HTML_ENTITY: {
				retState = State::kParsingEntities;
			} break;

			case ILexerToken::TT_PUNCTUATION: {
				// We got some punctuation, which means we're parsing the start of some sort of tag.  If we 
				// have a < token, then it's the start of a tag.  If it's a </ then it's the end tag.  Note, we
				// don't support parsing <! or <? at this point.
				if ((*ioStart)->GetValue() == HTMLLexemes::START_OF_CLOSE_TAG) {
					retState = State::kParsingEndTag;
				} else if ((*ioStart)->GetValue() == CHAR_LESS_THAN_SIGN) {
					retState = State::kParsingStartTag;
				} else if ((*ioStart)->GetValue() == HTMLLexemes::BANG_START) {
					retState = State::kParsingBangTag;
				} else if ((*ioStart)->GetValue() == HTMLLexemes::QUESTION_START) {
				}
			} break;
		}
	} else {
		retState = State::kEndOfInput;
	}

	ioState->fCurrentState = retState;
}

bool HTMLParser::ExpectValue( TokenList::iterator &ioStart, TokenList::iterator end, int inValue )
{
	if (ioStart != end && (*ioStart)->GetValue() == inValue)	return true;
	return false;
}

bool HTMLParser::ExpectType( TokenList::iterator &ioStart, TokenList::iterator end, ILexerToken::TYPE inType )
{
	if (ioStart != end && (*ioStart)->GetType() == inType)	return true;
	return false;
}

void HTMLParser::ProcessEndTag( TokenList::iterator &ioStart, TokenList::iterator end, State *ioState )
{
	// An end tag is something in the form </text>, with optional whitespace.  So we want to process until we find a >.
	// We have already processed the </ previously, so now we just need to find the end.
	VIndex startPosition = (*ioStart)->GetPosition();
	VString buffer;

	// We can skip any whitespaces between the </ and the tag that's being closed
	SkipWhitespace( ioStart, end, ioState );

	// Now we expect to find the name of the tag being closed. 
	if (ExpectType( ioStart, end, ILexerToken::TT_NAME )) {
		buffer = (*ioStart)->GetText();
		++ioStart;
	}

	// Again, we can skip any whitespace between the tag name and the closing brace
	SkipWhitespace( ioStart, end, ioState );

	// And finally, we expect to get the close of the tag
	if (ExpectValue( ioStart, end, CHAR_GREATER_THAN_SIGN )) {
		++ioStart;
	}

	// At this point, we may have consolidated several tokens into a single text token, which
	// we can now pass back to the caller
	if (!buffer.IsEmpty()) {
		ICallback::BasicInformation bi = { 0 };
		bi.fCookie = ioState->fCallbackCookie;
		bi.fStartPosition = startPosition;
		bi.fLength = buffer.GetLength();
		ioState->fCallback->ClosingTagParsed( &bi, buffer );
	}

	// We're back to default processing now
	ioState->fCurrentState = State::kDefault;
}

void HTMLParser::ProcessStartTag( TokenList::iterator &ioStart, TokenList::iterator end, State *ioState )
{
	// A start tag is something in the form <text>, with optional whitespace.  It could also be a self-closing
	// tag, we don't really know.  So it could also be <text/>, with optional whitespace.  Finally, starting tags
	// are allowed to have attributes as well, which we will need to process.  We've already processed the < by
	// the time this is called.
	VIndex startPosition = (*ioStart)->GetPosition();

	// Whitespace at the start of the tag is eaten
	SkipWhitespace( ioStart, end, ioState );

	// Now we expect to get the HTML tag name as a single token
	if (ExpectType( ioStart, end, ILexerToken::TT_NAME )) {
		ioState->fLastTagName = (*ioStart)->GetText();
		ICallback::BasicInformation bi = { 0 };
		bi.fCookie = ioState->fCallbackCookie;
		bi.fStartPosition = startPosition;
		bi.fLength = ioState->fLastTagName.GetLength();
		ioState->fCallback->OpenTagOpened( &bi, ioState->fLastTagName );
		++ioStart;
	}

	// Now there is optional whitespace
	SkipWhitespace( ioStart, end, ioState );

	// At this point, we either expect to find an attribute name, or
	// the end of the tag (which could be a self-closing tag, too!).
	if (ioStart != end) {
		if ((*ioStart)->GetType() == ILexerToken::TT_NAME) {
			// We're at an attribute name, which is parsed elsewhere
			ioState->fCurrentState = State::kParsingAttributeName;
		} else if ((*ioStart)->GetValue() == CHAR_GREATER_THAN_SIGN ||
					(*ioStart)->GetValue() == HTMLLexemes::SELF_CLOSE) {
			// We found the end of the tag, so we can fire another message to the caller
			bool bIsSelfClose = (*ioStart)->GetValue() == HTMLLexemes::SELF_CLOSE;
			ICallback::BasicInformation bi = { 0 };
			bi.fCookie = ioState->fCallbackCookie;
			bi.fStartPosition = (*ioStart)->GetPosition();
			bi.fLength = (*ioStart)->GetLength();
			ioState->fCallback->OpenTagClosed( &bi, bIsSelfClose );
			ioState->fCurrentState = State::kDefault;
			++ioStart;
		}
	}
}

void HTMLParser::ProcessAttributeName( TokenList::iterator &ioStart, TokenList::iterator end, State *ioState )
{
	// We know we've got an attribute name.  Now we need to process it and move on.  However, moving on
	// could mean closing the tag, or finding another attribute, OR finding an attribute value -- all with
	// optional whitespace involved.
	xbox_assert( (*ioStart)->GetType() == ILexerToken::TT_NAME );

	// We have a name, which is the name of the attribute.  So eat that, and move on
	ioState->fLastAttributeName = (*ioStart)->GetText();

	ICallback::BasicInformation bi = { 0 };
	bi.fCookie = ioState->fCallbackCookie;
	bi.fStartPosition = (*ioStart)->GetPosition();
	bi.fLength = (*ioStart)->GetLength();
	ioState->fCallback->AttributeNameParsed( &bi, ioState->fLastAttributeName );
	++ioStart;

	// Now we have some optional whitespace that we can eat
	SkipWhitespace( ioStart, end, ioState );

	// Time to figure out what comes next.  If it's an equals sign, then we expect an attribute value.  If it
	// is another name, we expect another attribute.  If it's a > or />, then we've closed the tag!
	if (ioStart != end) {
		if ((*ioStart)->GetType() == ILexerToken::TT_NAME) {
			ioState->fCurrentState = State::kParsingAttributeName;
		} else if ((*ioStart)->GetValue() == CHAR_EQUALS_SIGN) {
			ioState->fCurrentState = State::kParsingAttributeValue;
		} else if ((*ioStart)->GetValue() == CHAR_GREATER_THAN_SIGN ||
					(*ioStart)->GetValue() == HTMLLexemes::SELF_CLOSE) {
			// We found the end of the tag, so we can fire another message to the caller
			bool bIsSelfClose = (*ioStart)->GetValue() == HTMLLexemes::SELF_CLOSE;
			ICallback::BasicInformation bi = { 0 };
			bi.fCookie = ioState->fCallbackCookie;
			bi.fStartPosition = (*ioStart)->GetPosition();
			bi.fLength = (*ioStart)->GetLength();
			ioState->fCallback->OpenTagClosed( &bi, bIsSelfClose );
			ioState->fCurrentState = State::kDefault;
			++ioStart;
		}
	}
}

void HTMLParser::ProcessAttributeValue( TokenList::iterator &ioStart, TokenList::iterator end, State *ioState )
{
	// We're in the process of parsing an attribute value -- we should have an equals sign at this point.
	xbox_assert( (*ioStart)->GetValue() == CHAR_EQUALS_SIGN );

	// Eat the equals sign
	if (++ioStart == end)	return;

	// And skip any whitespace we were dealing with
	SkipWhitespace( ioStart, end, ioState );

	// Now we expect to get the attribute value, which is always going to be a string literal
	if (ExpectType( ioStart, end, ILexerToken::TT_STRING )) {
		VString value = (*ioStart)->GetText();
		ICallback::BasicInformation bi = { 0 };
		bi.fCookie = ioState->fCallbackCookie;
		bi.fStartPosition = (*ioStart)->GetPosition();
		bi.fLength = (*ioStart)->GetLength();
		ioState->fCallback->AttributeValueParsed( &bi, ioState->fLastAttributeName, value );
		++ioStart;
	}

	// At this point, we can skip more whitespace
	SkipWhitespace( ioStart, end, ioState );

	// And we're either parsing the end of the tag, or we're parsing the start of another attribute
	if (ioStart != end) {
		if ((*ioStart)->GetType() == ILexerToken::TT_NAME) {
			ioState->fCurrentState = State::kParsingAttributeName;
		} else if ((*ioStart)->GetValue() == CHAR_GREATER_THAN_SIGN ||
					(*ioStart)->GetValue() == HTMLLexemes::SELF_CLOSE) {
			// We found the end of the tag, so we can fire another message to the caller
			bool bIsSelfClose = (*ioStart)->GetValue() == HTMLLexemes::SELF_CLOSE;
			ICallback::BasicInformation bi = { 0 };
			bi.fCookie = ioState->fCallbackCookie;
			bi.fStartPosition = (*ioStart)->GetPosition();
			bi.fLength = (*ioStart)->GetLength();
			ioState->fCallback->OpenTagClosed( &bi, bIsSelfClose );
			ioState->fCurrentState = State::kDefault;
			++ioStart;
		}
	}
}

void HTMLParser::ProcessBangTag( TokenList::iterator &ioStart, TokenList::iterator end, State *ioState )
{
	// We've found a <! token that's not part of a comment, and we need to finish parsing it.  At this
	// point, we've already eaten the opening bracket.

	// There are some identifiers we expect to find at this point:
	// ENTITY, DOCTYPE, ELEMENT, ATTLIST, NOTATION, SHORTREF, USEMAP, LINKTYPE, LINK, IDLINK, USELINK, SGML, or SYSTEM
	// However, the only one we actually support it DOCTYPE.

	SkipWhitespace( ioStart, end, ioState );
	if (ExpectType( ioStart, end, ILexerToken::TT_NAME ) && (*ioStart)->GetText() == "DOCTYPE") {
		ICallback::BasicInformation bi = { 0 };
		bi.fCookie = ioState->fCallbackCookie;
		bi.fStartPosition = (*ioStart)->GetPosition();
		bi.fLength = (*ioStart)->GetLength();
		ioState->fCallback->DocTypeParsed( &bi );
		ioState->fCurrentState = State::kParsingDocType;
	} else {
		ioState->fCurrentState = State::kSkipToCloseBracket;
	}
}

void HTMLParser::SkipToCloseBracket( TokenList::iterator &ioStart, TokenList::iterator end, State *ioState )
{
	// For right now, we're just going to eat everything until we hit the >
	while (ioStart != end) {
		if ((*ioStart)->GetValue() == CHAR_GREATER_THAN_SIGN) {
			// We're done!
			++ioStart;
			ioState->fCurrentState = State::kDefault;
			break;
		}
		++ioStart;
	}
}