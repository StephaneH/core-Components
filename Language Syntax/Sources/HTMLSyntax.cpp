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
#include "html.lexer.h"
#include "HTMLSyntax.h"

// Note that the caller is responsible for freeing the memory
static char *CreateUTF8String( const VString &inSource )
{
	VFromUnicodeConverter_UTF8 converter;
	VIndex charsConsumed;
	VSize bytesProduced;
	char *ret = NULL;
	if (converter.Convert( inSource.GetCPointer(), inSource.GetLength() + 1, &charsConsumed, NULL, MaxLongInt, &bytesProduced )) {
		ret = new char[ bytesProduced ];
		converter.Convert( inSource.GetCPointer(), inSource.GetLength() + 1, &charsConsumed, ret, bytesProduced, &bytesProduced );
	}
	return ret;
}

void FreeUTF8String( char *inUTF8String )
{
	delete [] inUTF8String;
}

class ParsingCookie : public IRefCountable {
public:
	ParsingCookie( ICodeEditorDocument *inDoc, sLONG inLine ) : fDocument( inDoc ), fLineNumber( inLine ) {}

	ICodeEditorDocument *fDocument;
	sLONG fLineNumber;
};

static int htmlcolorShadow[12] = {0,5,5,1,5,5,0,7,7,8,6,9};    //store the colors 

 extern "C"
{
	struct htmlLexeme* parseHTML( const char * );
	void FreeLexemeList( struct htmlLexeme *inList );
};

VArrayOf<VString> allhtmltags; 

VHTMLSyntax::VHTMLSyntax()
{
	fAutoInsertBlock = false;
	fAutoInsertClosingChar = false;
	fAutoInsertTabs = false;
	fInsertSpaces = false;
	fTabWidth = 4;
}

VHTMLSyntax::~VHTMLSyntax()
{
}

void VHTMLSyntax::Init( ICodeEditorDocument* inDocument )
{
	inDocument->SetStyle( 0, false, false, false,   0,   0,   0 );
	inDocument->SetStyle( 1, false, false, false, 255,   0,   0 );
	inDocument->SetStyle( 2, false, false, false, 255, 128,   0 );
	inDocument->SetStyle( 3, false, false, false, 255, 210,   0 );
	inDocument->SetStyle( 4, false, false, false,  95, 162, 135 );
	inDocument->SetStyle( 5, false, false, false,  46, 139,  87 );
	inDocument->SetStyle( 6, false, false, false,   0,   0, 255 );
	inDocument->SetStyle( 7, false, false, false, 155,  48, 255 );
	inDocument->SetStyle( 8, false, false, false, 128, 128, 128 );
	inDocument->SetStyle( 9, false, false, false,   0,   0,   0 );
}

void VHTMLSyntax::Load( ICodeEditorDocument* inDocument, VString& ioContent )
{
}

bool VHTMLSyntax::CanHiliteSameWord( sBYTE inStyle )
{
	return inStyle == 1 || inStyle == 0;
}

void VHTMLSyntax::Save( ICodeEditorDocument* inDocument, VString& ioContent )
{
}

void VHTMLSyntax::SaveLine( ICodeEditorDocument* inDocument, sLONG inLineIndex, VString& ioContent )
{
}


void VHTMLSyntax::Close( ICodeEditorDocument* inDocument )
{
}

class VHTMLSyntaxParams : public VObject, public ISyntaxParams
{
public:
	typedef std::map< int, HTMLParser::State * > StateMap;

	VHTMLSyntaxParams() { fMap = new StateMap; }
	virtual ~VHTMLSyntaxParams() { 
		for (StateMap::iterator iter = fMap->begin(); iter != fMap->end(); ++iter) {
			iter->second->Release();
		}
		delete fMap;
	}
	StateMap *GetStateMap() const { return fMap; }

private:
	void SelfDelete() {delete this;}
	StateMap *fMap;
};

// This class is used to hold per-line information that's more in-depth than the line
// kind can hold.  We need this for the HTML syntax because a line can be multitple
// "kinds" at the same time.  For instance, a line that has both HTML code, and comment 
// code in it.
class VLineSyntaxParams : public VObject, public ISyntaxParams
{
private:
	void SelfDelete() { delete this; }

	bool fIsOpenComment;
	bool fIsProcessingTag, fIsStartTag;

	std::vector< VString > fTagStack;
	std::vector< VString > fUnmatchedOpenTags, fUnmatchedCloseTags;

public:
	VLineSyntaxParams() {
		fIsOpenComment = false;
		fIsProcessingTag = false;
		fIsStartTag = false;
	}

	virtual ~VLineSyntaxParams() {

	}

	void CopyState( const VLineSyntaxParams *inCopy ) {
		if (inCopy) {
			fIsOpenComment = inCopy->fIsOpenComment;
			fIsProcessingTag = inCopy->fIsProcessingTag;
			fIsStartTag = inCopy->fIsStartTag;

			fTagStack.clear();
			fTagStack.reserve( inCopy->fTagStack.size() );
			for (std::vector< VString >::const_iterator iter = inCopy->fTagStack.begin(); iter != inCopy->fTagStack.end(); ++iter) {
				fTagStack.push_back( *iter );
			}

			// Note that the unmatched open and close tag lists are not considered part of
			// the state of a line -- we don't want to copy them!  They're associated only 
			// with a single line, and should never be copied over.
		}
	}

	void SetIsOpenComment( bool inSet) { fIsOpenComment = inSet; }
	bool IsOpenComment() const { return fIsOpenComment; }

	void SetIsProcessingTag( bool inSet ) { fIsProcessingTag = inSet; }
	bool IsProcessingTag() const { return fIsProcessingTag; }

	void SetIsProcessingStartTag( bool inSet ) { fIsStartTag = inSet; }
	bool IsProcessingStartTag() const { return fIsStartTag; }

	void PushTag( const VString &inTag ) { fTagStack.push_back( inTag ); }
	void PopTag( VString &outTag ) { if (!fTagStack.empty()) {outTag = fTagStack.back();  fTagStack.pop_back(); } }
	void LastTag( VString &outTag ) { if (!fTagStack.empty()) { outTag = fTagStack.back(); } }

	void AddUnmatchedOpenTag( const VString &inTag ) { fUnmatchedOpenTags.push_back( inTag ); }
	void AddUnmatchedCloseTag( const VString &inTag ) { fUnmatchedCloseTags.push_back( inTag ); }
	bool MatchesCloseTag( const VString &inTag ) {
		for (std::vector< VString >::iterator iter = fUnmatchedCloseTags.begin(); iter != fUnmatchedCloseTags.end(); ++iter) {
			if (inTag.EqualTo( *iter, false ))	return true;
		}
		return false;
	}
	const std::vector< VString > &GetUnmatchedOpenTags() const { return fUnmatchedOpenTags; }
	bool HasUnmatchedOpenOrCloseTags() const { return !fUnmatchedOpenTags.empty() || !fUnmatchedCloseTags.empty(); }
};

HTMLParser::State *VHTMLSyntax::GetStateForLine( ICodeEditorDocument *inDocument, sLONG inLine )
{
	VHTMLSyntaxParams *params = dynamic_cast< VHTMLSyntaxParams * >( inDocument->GetSyntaxParams( 1 ) );
	if (NULL == params) {
		params = new VHTMLSyntaxParams();
		inDocument->SetSyntaxParams( params , 1 );
	}

	VHTMLSyntaxParams::StateMap::iterator iter = params->GetStateMap()->find( inLine );
	if (iter != params->GetStateMap()->end()) {
		return iter->second;
	}

	return NULL;
}

void VHTMLSyntax::SetStateForLine( ICodeEditorDocument *inDocument, sLONG inLine, HTMLParser::State *inState )
{
	VHTMLSyntaxParams *params = dynamic_cast< VHTMLSyntaxParams * >( inDocument->GetSyntaxParams( 1 ) );
	if (NULL == params) {
		params = new VHTMLSyntaxParams();
		inDocument->SetSyntaxParams( params , 1 );
	}

	VHTMLSyntaxParams::StateMap::iterator iter = params->GetStateMap()->find( inLine );
	if (iter != params->GetStateMap()->end()) {
		iter->second->Release();
	}

	(*(params->GetStateMap()))[ inLine ] = inState;
}

static bool IsTagWithoutClose( const VString &inTag )
{
	static const char *sList[] = {
		"!DOCTYPE",
		"AREA",
		"BASE",
		"BASEFONT",
		"BR",
		"COL",
		"FRAME",
		"HR"
		"IMG",
		"INPUT",
		"ISINDEX",
		"LINK",
		"META",
		"PARAM",
	};

	for (sLONG i = 0; i < sizeof( sList ) / sizeof( const char * ); i++) {
		if (inTag.EqualTo( VString( sList[ i ] ), false ))	return true;
	}

	return false;
}

void VHTMLSyntax::SetLine( ICodeEditorDocument *inDocument, sLONG inLineNumber, bool inLoading )
{
#if 0
	VString source;
	inDocument->GetLine( inLineNumber, source );
	
	HTMLParser parser;
	HTMLParser::State *state = NULL;
	HTMLParser::State *prevLineState = NULL;
	if (inLineNumber > 0)	prevLineState = GetStateForLine( inDocument, inLineNumber - 1 );
	ParsingCookie *cookie = new ParsingCookie( inDocument, inLineNumber );
	parser.Parse( source, prevLineState, &state, this, (const void *)cookie );
	SetStateForLine( inDocument, inLineNumber, state );
	cookie->Release();
#else
	// Get the params for the current line so that we can set them up properly
	VLineSyntaxParams *currentLineParams = static_cast< VLineSyntaxParams * >( inDocument->GetLineSyntaxParams( inLineNumber ) );
	if (!currentLineParams) {
		currentLineParams = new VLineSyntaxParams();
		inDocument->AssignLineSyntaxParams( inLineNumber, currentLineParams );
	}
	bool previousOpenCommentState = currentLineParams->IsOpenComment();

	// We also want the params for the preceeding line, in case we're the continuation of
	// a comment.
	VLineSyntaxParams *previousLineParams = NULL;
	if (inLineNumber > 0) {
		previousLineParams = static_cast< VLineSyntaxParams * >( inDocument->GetLineSyntaxParams( inLineNumber - 1 ) );
	}

	VString xstr;
	inDocument->GetLine(inLineNumber,xstr);
	inDocument->SetLineStyle(inLineNumber,0,xstr.GetLength(),0);		//initiate the line

	char *lexinput = CreateUTF8String( xstr );
	struct htmlLexeme *list = parseHTML( lexinput );

	// If we used to be in comment continuation mode, the assumption is that we're still in 
	// comment continuation mode.  We'll switch this off if the comment ends though
	currentLineParams->CopyState( previousLineParams );

	// We are going to keep track of which open and close tags we've seen on the line.  This allows
	// us to determine which unmatched open and close tags exist so we can associate that data with
	// the line.  As we find open tags, we'll push them onto the open tag list.  As we find close tags,
	// we will scan the open tag list and *remove* any that match.  If there's no match, then we'll add
	// the tag to the close list.
	std::vector< VString > openList, closeList;

	// Given the list of HTML tokens, let's walk over the list and try to make some sense
	// of them.  Walk over the list one token at a time, and see if we can make sense of 
	// what we've got.
	struct htmlLexeme *cur = list;
	while (cur) {
		// There are only three types of comments we need to worry about.  Full comments, 
		// open comments and close comments.  We'll get a token representing any one of the
		// three.  However, we need to pay special attention to multi-line comments, since
		// they won't lex out entirely correct.  If the previous line was part of an open
		// comment, then we want to keep walking over the tokens, marking them as part of
		// the comment, until we run out of tokens, or we find a kCommentClose token.
		if (currentLineParams->IsOpenComment()) {
			if (kCommentClose == cur->fStyle) {
				// We found the end of the comment, so we can highlight it appropriately, 
				// and go back to our regularly scheduled lexing
				inDocument->SetLineStyle( inLineNumber, cur->fOffset, cur->fOffset + cur->fLength, htmlcolorShadow[ comment_col ] );

				// We're also done being a part of the comment continuation train
				currentLineParams->SetIsOpenComment( false );
			} else {
				// This is just another part of the comment
				inDocument->SetLineStyle( inLineNumber, cur->fOffset, cur->fOffset + cur->fLength, htmlcolorShadow[ comment_col ] );
			}
			// Advance
			cur = cur->fNext;
			continue;
		}
		if (kCompleteComment == cur->fStyle) {
			// A complete comment is the easiest of the three cases.  Just highlight it
			inDocument->SetLineStyle( inLineNumber, cur->fOffset, cur->fOffset + cur->fLength, htmlcolorShadow[ comment_col ] );
		} else if (kCommentOpen == cur->fStyle) {
			// An open comment must be the last token in the list
			xbox_assert( !cur->fNext );

			// We want to highlight from here to the end of the line
			inDocument->SetLineStyle( inLineNumber, cur->fOffset, cur->fOffset + cur->fLength, htmlcolorShadow[ comment_col ] );
			// We also want to flag that this line ends with an open comment
			currentLineParams->SetIsOpenComment( true );
		} else if (kCommentClose == cur->fStyle) {
			// If we got a close comment token, then something's off.  That means the user put in a close comment
			// token, but they never opened it.  We're going to ignore that state, and flag this as being normal
			inDocument->SetLineStyle( inLineNumber, cur->fOffset, cur->fOffset + cur->fLength, htmlcolorShadow[ scommentend_col ] );
		}
		else if ( kString == cur->fStyle ) {
			inDocument->SetLineStyle( inLineNumber, cur->fOffset, cur->fOffset + cur->fLength, htmlcolorShadow[ string_col ] );
		} else if ( kSymbolicChar == cur->fStyle ) {
			inDocument->SetLineStyle( inLineNumber, cur->fOffset, cur->fOffset + cur->fLength, htmlcolorShadow[ separator_col ] );
		} else if (kKeyword == cur->fStyle) {
			// Keywords a bit trickier than you might think because we need to be sure they're actually part of a
			// tag.  If the user types something like: <b>This table rocks</b>, we only want to highlight the b in the
			// begin and end tag, and not the "table" in the user's text.  To deal with this, we have an "in tag" flag
			// that basically turns keyword highlighting on and off.
			if (currentLineParams->IsProcessingTag()) {
				inDocument->SetLineStyle( inLineNumber, cur->fOffset, cur->fOffset + cur->fLength, htmlcolorShadow[ keyword_col ] );

				// If we're processing an opening tag, then we want to push the keyword onto the tag stack.  But if we're
				// processing a closing tag, then we want to pop the last keyword off the tag stack and try to match it up
				// to what we just processed.  If they match, we're golden.  If not, we just assume the user's mismatching
				// their tags because they're an idiot.
				VString tagName;
				xstr.GetSubString( cur->fOffset + 1, cur->fLength, tagName );

				if (currentLineParams->IsProcessingStartTag()) {
					if (!IsTagWithoutClose( tagName )) {
						openList.push_back( tagName );
					}
					currentLineParams->PushTag( tagName );

					// Note that we are no longer processing the start of a tag.  This allows us to handle attributes
					// separately from the tag itself.
					currentLineParams->SetIsProcessingStartTag( false );
				} else {
					// Check to see if this closed tag is on the open list.  If it is, we want to remove it from the
					// list.  Otherwise, we want to add it to the close list.
					bool bAddToClose = true;
					for (std::vector< VString >::iterator iter = openList.begin(); bAddToClose && iter != openList.end();) {
						if (tagName.EqualTo( *iter, false )) {
							iter = openList.erase( iter );
							bAddToClose = false;
						} else {
							++iter;
						}
					}
					if (bAddToClose)	closeList.push_back( tagName );

					VString lastTag;
					currentLineParams->PopTag( lastTag );

					if (!lastTag.EqualTo( tagName, false )) {
						// The tags don't match, so we're just going to ignore the issue
						// TODO: do something more sensible here
					}
				}
			}
		} else if (kNumber == cur->fStyle) {
			inDocument->SetLineStyle( inLineNumber, cur->fOffset, cur->fOffset + cur->fLength, htmlcolorShadow[ allnum_col ] );
		} else if (kTagOpen == cur->fStyle || kEndTagOpen == cur->fStyle) {
			currentLineParams->SetIsProcessingTag( true );
			currentLineParams->SetIsProcessingStartTag( kTagOpen == cur->fStyle );
		} else if (kTagClose == cur->fStyle || kTagSelfClose == cur->fStyle) {
			currentLineParams->SetIsProcessingTag( false );

			// If we just handled a self-closing tag (like <br />), then we want to pop it from the stack
			VString lastTag;
			currentLineParams->LastTag( lastTag );
			if (kTagSelfClose == cur->fStyle || IsTagWithoutClose( lastTag )) {
				VString toss;
				currentLineParams->PopTag( toss );

				// We also do not want to add it to our list of open tags for the line, since it's self-closed
				for (std::vector< VString >::iterator iter = openList.begin(); iter != openList.end(); ++iter) {
					if (lastTag.EqualTo( *iter, false )) {
						iter = openList.erase( iter );
						break;
					}
				}
			}
		}

		cur = cur->fNext;
	}
	FreeLexemeList( list );

	// Now that we have an open and a close list, we want to associate them with the line.
	for (std::vector< VString >::iterator iter = openList.begin(); iter != openList.end(); ++iter) {
		currentLineParams->AddUnmatchedOpenTag( *iter );
	}
	for (std::vector< VString >::iterator iter = closeList.begin(); iter != closeList.end(); ++iter) {
		currentLineParams->AddUnmatchedCloseTag( *iter );
	}

	// There are two cases we really need to care about.  If the line now ends in
	// an open comment (and didn't used to), we want to colorize down the document.
	// Also, if the line no longer ends in an open comment (but used to), we want to
	// colorize down the document.  In either case, we want to keep colorizing subsequent
	// lines until the comment is ended or the end of the document is reached.
	if ((!previousOpenCommentState && currentLineParams->IsOpenComment() ||		// Now ends with open comment, didn't used to
		previousOpenCommentState && !currentLineParams->IsOpenComment()) &&		// Used to end with an open comment, but no longer does
		inLineNumber + 1 < inDocument->GetNbLines()) {
		SetLine( inDocument, inLineNumber + 1, inLoading );
	}
#endif // old code
}


bool VHTMLSyntax::CheckFolding( ICodeEditorDocument* inDocument, sLONG inLineNumber )
{
	VLineSyntaxParams *currentLineParams = static_cast< VLineSyntaxParams * >( inDocument->GetLineSyntaxParams( inLineNumber ) );
	if (currentLineParams) {
		return currentLineParams->HasUnmatchedOpenOrCloseTags();
	}
	return false;
}

class list_item {
public:
	list_item( const VString &inTag, sLONG inLine ) : fTag( const_cast< VString & >( inTag ) ), fLine( inLine ) {}
	list_item( const list_item &inLeft ) : fTag( inLeft.fTag ), fLine( inLeft.fLine ) {}
	list_item& operator= ( const list_item &inLeft ) { fTag = inLeft.fTag; fLine = inLeft.fLine; return *this; }

	VString &fTag;
	sLONG fLine;
};

void VHTMLSyntax::ComputeFolding( ICodeEditorDocument *inDocument )
{
	// Walk over the lines in the document, and start matching up their open and close tags.  Each line in the document
	// has some params associated with it, and each one of those params comes with a list of the unmatched open and close
	// tags within it.  Whenever we find an open tag in that list, we want to indent further and start folding lines.  
	// Whenever we find a close tag in the list, we want to try to match it to our outstanding open tags in order to determine
	// how many lines to fold and where the indentation stops.
	std::vector< list_item > runningListOfOpenTags;
	sLONG count = inDocument->GetNbLines();
	sLONG currentIndent = 0;
	for (sLONG i = 0; i < count; i++) {
		VLineSyntaxParams *currentLineParams = static_cast< VLineSyntaxParams * >( inDocument->GetLineSyntaxParams( i ) );
		if (!currentLineParams)	continue;

		// First, walk over the list of current unmatched open tags, and see if this line closes any
		for (std::vector< list_item >::iterator iter = runningListOfOpenTags.begin(); iter != runningListOfOpenTags.end();) {
			if (currentLineParams->MatchesCloseTag( (*iter).fTag )) {
				// An open tag was closed now, so we can remove it from the list, and fold all of the lines from there to here
				inDocument->SetFoldable( (*iter).fLine, true );
				inDocument->SetNbFoldedLines( (*iter).fLine, i - (*iter).fLine );
				currentIndent--;

				// This automatically incrememnts our iterator, so we do not want to increment ourselves!
				iter = runningListOfOpenTags.erase( iter );
			} else {
				++iter;
			}
		}

		// Having matched all of the closed tags, we're ready to set the line indentation, and start running up our open
		// tag list
		inDocument->SetLineIndent( i, currentIndent );
		const std::vector< VString > &list = currentLineParams->GetUnmatchedOpenTags();
		for (std::vector< VString >::const_iterator iter = list.begin(); iter != list.end(); ++iter) {
			list_item li( *iter, i );
			runningListOfOpenTags.push_back( li );
			currentIndent++;
		}
	}
}

bool VHTMLSyntax::CheckOutline( ICodeEditorDocument* inInterface, sLONG inLineNumber )
{
	return false;
}

void VHTMLSyntax::ComputeOutline( ICodeEditorDocument* inInterface )
{
}


void VHTMLSyntax::GetTip( ICodeEditorDocument* inDocument, sLONG inLine, sLONG inPos, VString& outText, Boolean& outRtfText )
{
}

void VHTMLSyntax::GetSuggestions( ICodeEditorDocument* inDocument, sLONG inLineNumber, sLONG inPos, ITipInfoArray *outSuggestions, sLONG& outStartOffset, bool inAll )
{
	// Get the text for the line up to the point of insertion, and we'll lex that to see if we can come up 
	// with some rational suggestions for the user.
	VString xstr;
	inDocument->GetLine( inLineNumber, xstr );
	xstr.Truncate( inPos );

	char *lexinput = CreateUTF8String( xstr );
	struct htmlLexeme *list = parseHTML( lexinput );

	// Gin up some line params for tracking state information
	VLineSyntaxParams *currentLineParams = currentLineParams = new VLineSyntaxParams();
	if (inLineNumber > 0) {
		// We're starting where we left off on the previous line
		currentLineParams->CopyState( static_cast< VLineSyntaxParams * >( inDocument->GetLineSyntaxParams( inLineNumber - 1 ) ) );
	}

	// Given the list of HTML tokens, let's walk over the list and try to make some sense
	// of them.  Walk over the list one token at a time, and see if we can make sense of 
	// what we've got.  This is going to be awfully similar to the way we do things in the
	// SetLine method, except that we're not actually updating the line state for the current
	// line.  Instead, we're working on a copy of the existing information.
	struct htmlLexeme *cur = list;
	int lastTokenProcessed = 0;
	while (cur) {
		if (kKeyword == cur->fStyle) {
			lastTokenProcessed = 3;

			// Keywords a bit trickier than you might think because we need to be sure they're actually part of a
			// tag.  If the user types something like: <b>This table rocks</b>, we only want to highlight the b in the
			// begin and end tag, and not the "table" in the user's text.  To deal with this, we have an "in tag" flag
			// that basically turns keyword highlighting on and off.
			if (currentLineParams->IsProcessingTag()) {
				// If we're processing an opening tag, then we want to push the keyword onto the tag stack.  But if we're
				// processing a closing tag, then we want to pop the last keyword off the tag stack and try to match it up
				// to what we just processed.  If they match, we're golden.  If not, we just assume the user's mismatching
				// their tags because they're an idiot.
				VString tagName;
				xstr.GetSubString( cur->fOffset + 1, cur->fLength, tagName );

				if (currentLineParams->IsProcessingStartTag()) {
					currentLineParams->PushTag( tagName );

					// Note that we are no longer processing the start of a tag.  This allows us to handle attributes
					// separately from the tag itself.
					currentLineParams->SetIsProcessingStartTag( false );
				} else {
					VString lastTag;
					currentLineParams->PopTag( lastTag );

					if (!lastTag.EqualTo( tagName, false )) {
						// The tags don't match, so we're just going to ignore the issue
						// TODO: do something more sensible here
					}
				}
			}
		} else if (kTagOpen == cur->fStyle || kEndTagOpen == cur->fStyle) {
			lastTokenProcessed = (kTagOpen == cur->fStyle) ? 1 : 2;

			currentLineParams->SetIsProcessingTag( true );
			currentLineParams->SetIsProcessingStartTag( kTagOpen == cur->fStyle );
		} else if (kTagClose == cur->fStyle || kTagSelfClose == cur->fStyle) {
			lastTokenProcessed = 0;

			currentLineParams->SetIsProcessingTag( false );

			// If we just handled a self-closing tag (like <br />), then we want to pop it from the stack
			// TODO: some tags can't have matching pairs, like <br>, so even if it's not self-closing, we want
			// to pop it off the tag stack.  Handle that here
			if (kTagSelfClose == cur->fStyle) {
				VString toss;
				currentLineParams->PopTag( toss );
			}
		} else {
			lastTokenProcessed = 0;
		}

		cur = cur->fNext;
	}

	if (lastTokenProcessed == 1) {
		// We processed a tag opener, but no keyword for the tag.  So let's make a bunch of suggestions!
	} else if (lastTokenProcessed == 2) {
		// We processed a tag closer, but no keyword for the tag.  Grab the last opened tag from the list
		// and suggest it as the closer
		VString suggestion;
		currentLineParams->LastTag( suggestion );
		outSuggestions->AddTip( new VCodeEditorTipInfo( inDocument, suggestion, htmlcolorShadow[ keyword_col ] ) );
	}

	delete currentLineParams;
	FreeLexemeList( list );
}

sLONG VHTMLSyntax::GetIndentWidth()
{
	return 0;
}

bool VHTMLSyntax::UseTab() 
{
	return true;
}

void VHTMLSyntax::TokenizeLine( ICodeEditorDocument* inDocument, sLONG inLineIndex, std::vector<XBOX::VString>& ioLines )
{

}

void VHTMLSyntax::InsertChar( ICodeEditorDocument* inDocument, UniChar inUnichar, sLONG inLineIndex, sLONG inPosition, ITipInfoArray *outSuggestions, sLONG& outStartOffset )
{
	if ( inUnichar == '/'  && fAutoInsertClosingChar) {
		VString str;
		inDocument->GetLine(inLineIndex,str);
		
		if (inPosition>1 && str[inPosition-2]=='<' ) {
			GetSuggestions( inDocument, inLineIndex, inPosition, outSuggestions, outStartOffset, false );
		}  
	} else if ( inUnichar == '<' && fAutoInsertClosingChar) {
		inDocument->InsertText( VString( (UniChar) 62 ) ); //              62  is  >
		sLONG startloc = 0;
		sLONG endloc = 0;
		sLONG atline = 0;	

		inDocument->GetSelection(startloc,endloc,atline,atline);
		if ( startloc>0 ) {
			inDocument->Select(startloc-1,endloc-1,atline,atline);
		}
		GetSuggestions( inDocument, inLineIndex, startloc - 1, outSuggestions, outStartOffset, false );
	} else if ( inUnichar == '=' ) {

		// TBD : do it only after a property
		
/*		inDocument->InsertText( CVSTR( "\"\"" ) );
		sLONG startloc = 0;
		sLONG endloc = 0;
		sLONG atline = 0;	

		inDocument->GetSelection(startloc,endloc,atline,atline);
		if ( startloc>0 ) {
			inDocument->Select(startloc-1,endloc-1,atline,atline);
		}
*/
	} else if ( inUnichar == '\'' && fAutoInsertClosingChar ) {
		inDocument->InsertText( VString( ( UniChar ) '\'' ) );
		sLONG startloc = 0;
		sLONG endloc = 0;
		sLONG atline = 0;	

		inDocument->GetSelection(startloc,endloc,atline,atline);
		if ( startloc>0 ) {
			inDocument->Select(startloc-1,endloc-1,atline,atline);
		}
	}
	else if ( inUnichar == '\"' && fAutoInsertClosingChar ) {
		inDocument->InsertText( VString( (UniChar) '\"' ) );
		sLONG startloc = 0;
		sLONG endloc = 0;
		sLONG atline = 0;

		inDocument->GetSelection( startloc, endloc, atline, atline );
		if ( startloc>0 ) {
			inDocument->Select( startloc - 1, endloc - 1, atline, atline );
		}
	}
	else if ( inUnichar == '!'  && fAutoInsertClosingChar ) {
		VString strtemp = "";
		inDocument->GetLine( inLineIndex,strtemp );

		if ( inPosition >= 2 && strtemp[inPosition-2] == '<' ) {
			inDocument->InsertText( "----"); //insert spaces       39 is '
			sLONG startloc = 0;
			sLONG endloc = 0;
			sLONG atline = 0;	

			inDocument->GetSelection(startloc,endloc,atline,atline);
			if ( startloc>0 ) {
				inDocument->Select(startloc-2,endloc-2,atline,atline);
			}
		}
	} else if ( inUnichar == '#' ) {
		VString strtemp = "";
		inDocument->GetLine( inLineIndex,strtemp );

		if ( inPosition >= 5 && strtemp[inPosition-2] == '-' && strtemp[inPosition-3] == '-' && strtemp[inPosition-4] == '!' && strtemp[inPosition-5] == '<') {
			outSuggestions->AddTip( new VCodeEditorTipInfo( inDocument, "4DVAR", vcol_blue ) );
			outSuggestions->AddTip( new VCodeEditorTipInfo( inDocument, "4DHTMLVAR", vcol_blue ) );
			outSuggestions->AddTip( new VCodeEditorTipInfo( inDocument, "4DSCRIPT", vcol_blue ) );
			outSuggestions->AddTip( new VCodeEditorTipInfo( inDocument, "4DINCLUDE", vcol_blue ) );
			outSuggestions->AddTip( new VCodeEditorTipInfo( inDocument, "4DIF", vcol_blue ) );
			outSuggestions->AddTip( new VCodeEditorTipInfo( inDocument, "4DENDIF", vcol_blue ) );
			outSuggestions->AddTip( new VCodeEditorTipInfo( inDocument, "4DELSE", vcol_blue ) );
			outSuggestions->AddTip( new VCodeEditorTipInfo( inDocument, "4DLOOP", vcol_blue ) );
			outSuggestions->AddTip( new VCodeEditorTipInfo( inDocument, "4DENDLOOP", vcol_blue ) );
		}
	}
}

void VHTMLSyntax::UpdateBreakPointsLineNumbers( ICodeEditorDocument* inDocument, const std::vector<sWORD>& inBreakIDs, const std::vector<sLONG>& inLineNumbers )
{
}

void VHTMLSyntax::AddBreakPoint( ICodeEditorDocument* inDocument, sLONG inLineNumber, sWORD& outID, bool& outDisabled )
{
	outID = 0;
}

bool VHTMLSyntax::EditBreakPoint( ICodeEditorDocument* inDocument, sWORD inBreakID, bool& outDisabled )
{
	return false;
}

void VHTMLSyntax::RemoveBreakPoint( ICodeEditorDocument* inDocument, sLONG inLineNumber )
{
}

bool VHTMLSyntax::GetBreakPoints( ICodeEditorDocument* inDocument, std::vector<sLONG>& outLineNumbers, std::vector<sWORD>& outIDs, std::vector<bool>& outDisabled )
{
	return false;
}

void VHTMLSyntax::UpdateCompilerErrors( ICodeEditorDocument* inDocument )
{
}

void VHTMLSyntax::GotoNextMethodError( ICodeEditorDocument* inDocument, bool inNext )
{
}

void VHTMLSyntax::SwapAssignments( ICodeEditorDocument* inDocument, VString& ioString )
{
}

bool VHTMLSyntax::IsComment( ICodeEditorDocument* inDocument, const VString& inString )
{
	VString temp = inString;
	temp.RemoveWhiteSpaces();

	return temp.BeginsWith( CVSTR( "<!--" ) ) && temp.EndsWith( CVSTR( "-->" ) );
}

void VHTMLSyntax::SwapComment( ICodeEditorDocument* inDocument, VString& ioString, bool inComment )
{
	VString temp = ioString;
	temp.RemoveWhiteSpaces();
	
	if( temp.BeginsWith("<!--") && temp.EndsWith("-->") )
	{
		VIndex start = ioString.Find("<!--");
		ioString.Remove(start, 4);
		
		if( ioString.GetLength() == 3 )
			ioString.Remove(1, 3);
		else
		{
			for(VIndex current=ioString.GetLength(); current>0; --current)
			{
				start = ioString.Find("-->", current);
				if( start != 0 )
				{
					ioString.Remove(start, 3);
					break;
				}
			}
		}
	}
	else
	{
		VString start("<!--");
		start += ioString;
		start += "-->";
		ioString = start;
	}
}

void VHTMLSyntax::CallHelp( ICodeEditorDocument* inDocument )
{
}

bool VHTMLSyntax::IsMatchingCharacter( ICodeEditorDocument* inDocument, sLONG inLineIndex, sLONG inOffset, UniChar inChar )
{
	switch( inChar )
	{
		case '(':
		case ')':
		case '{':
		case '}':
		case '[':
		case ']':
		case '<':
		case '>':
			return true;
		default:
			return false;
	}
}

bool VHTMLSyntax::ShouldValidateTipWindow( UniChar inChar )
{
	return inChar == '(' || inChar == ':' || inChar == '=' || inChar == ';' || 
		   inChar == '<' || inChar == '>' || inChar == '+' || inChar == '-' || 
		   inChar == '{' || inChar == '/' || inChar == '#' || inChar == '[';
}

bool VHTMLSyntax::DoubleClick( ICodeEditorDocument* inDocument, sLONG inLineIndex, sLONG inOffset, bool inAlternateKey )
{
	return false;
}

bool VHTMLSyntax::DetermineMorphemeBoundary( ICodeEditorDocument *inDocument, sLONG inLineIndex, sLONG inOffset, sLONG &outLeftBoundary, sLONG &outLength, bool inAlternateKey )
{
	return false;
}

void VHTMLSyntax::TextParsed( const BasicInformation *inInfo )
{
}

void VHTMLSyntax::OpenTagOpened( const BasicInformation *inInfo, const VString &inName )
{
	ParsingCookie *cookie = (ParsingCookie *)inInfo->fCookie;
	cookie->fDocument->SetLineStyle( cookie->fLineNumber, inInfo->fStartPosition, inInfo->fStartPosition + inInfo->fLength, 6 );
}

void VHTMLSyntax::OpenTagClosed( const BasicInformation *inInfo, bool inIsSelfClosed )
{
}

void VHTMLSyntax::AttributeNameParsed( const BasicInformation *inInfo, const VString &inName )
{
	ParsingCookie *cookie = (ParsingCookie *)inInfo->fCookie;
	cookie->fDocument->SetLineStyle( cookie->fLineNumber, inInfo->fStartPosition, inInfo->fStartPosition + inInfo->fLength, 5 );
}

void VHTMLSyntax::AttributeValueParsed( const BasicInformation *inInfo, const VString &inName, const VString &inValue )
{
	ParsingCookie *cookie = (ParsingCookie *)inInfo->fCookie;
	cookie->fDocument->SetLineStyle( cookie->fLineNumber, inInfo->fStartPosition, inInfo->fStartPosition + inInfo->fLength, 2 );
}

void VHTMLSyntax::ClosingTagParsed( const BasicInformation *inInfo, const VString &inName )
{
	ParsingCookie *cookie = (ParsingCookie *)inInfo->fCookie;
	cookie->fDocument->SetLineStyle( cookie->fLineNumber, inInfo->fStartPosition, inInfo->fStartPosition + inInfo->fLength, 6 );
}

void VHTMLSyntax::EntityParsed( const BasicInformation *inInfo, const VString &inName )
{
	ParsingCookie *cookie = (ParsingCookie *)inInfo->fCookie;
	cookie->fDocument->SetLineStyle( cookie->fLineNumber, inInfo->fStartPosition, inInfo->fStartPosition + inInfo->fLength, 1 );
}

void VHTMLSyntax::CommentParsed( const BasicInformation *inInfo, bool inIsUnfinished )
{
	ParsingCookie *cookie = (ParsingCookie *)inInfo->fCookie;
	cookie->fDocument->SetLineStyle( cookie->fLineNumber, inInfo->fStartPosition, inInfo->fStartPosition + inInfo->fLength, 8 );
}

void VHTMLSyntax::DocTypeParsed( const BasicInformation *inInfo )
{
	ParsingCookie *cookie = (ParsingCookie *)inInfo->fCookie;
	cookie->fDocument->SetLineStyle( cookie->fLineNumber, inInfo->fStartPosition, inInfo->fStartPosition + inInfo->fLength, 7 );
}

void VHTMLSyntax::DocTypeNameParsed( const BasicInformation *inInfo, const VString &inName )
{
	ParsingCookie *cookie = (ParsingCookie *)inInfo->fCookie;
	cookie->fDocument->SetLineStyle( cookie->fLineNumber, inInfo->fStartPosition, inInfo->fStartPosition + inInfo->fLength, 7 );
}

void VHTMLSyntax::DocTypeValueParsed( const BasicInformation *inInfo, const VString &inValue )
{
	ParsingCookie *cookie = (ParsingCookie *)inInfo->fCookie;
	cookie->fDocument->SetLineStyle( cookie->fLineNumber, inInfo->fStartPosition, inInfo->fStartPosition + inInfo->fLength, 2 );
}

void VHTMLSyntax::GetTokensForTesting( VString& inSourceCode, std::vector<VString>& outTokens )
{

}