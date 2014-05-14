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
#include "XML/VXML.h"
#include "XML/Sources/VCSSParser.h"
#include "CSSSyntax.h"
//Ne sert pas et empeche la compil x64 #include "GUI/VGUI.h"

const char *cssRefNameIndex[] = {
	"@import",						// This is a faked entry
	"azimuth",
	"background",
	"background-attachment",
	"background-color",
	"background-image",
	"background-position",
	"background-repeat",
	"border",
	"border-bottom",
	"border-bottom-color",
	"border-bottom-style",
	"border-bottom-width",
	"border-collapse",
	"border-color",
	"border-left",
	"border-left-color",
	"border-left-style",
	"border-left-width",
	"border-right",
	"border-right-color",
	"border-right-style",
	"border-right-width",
	"border-spacing",
	"border-style",
	"border-top",
	"border-top-color",
	"border-top-style",
	"border-top-width",
	"border-width",
	"bottom",
	"caption-side",
	"clear",
	"clip",
	"color",
	"content",
	"counter-increment",
	"counter-reset",
	"cue",
	"cue-after",
	"cue-before",
	"cursor",
	"direction",
	"display",
	"elevation",
	"empty-cells",
	"float",
	"font",
	"font-family",
	"font-size",
	"font-size-adjust",
	"font-stretch",
	"font-style",
	"font-variant",
	"font-weight",
	"height",
	"left",
	"letter-spacing",
	"line-height",
	"list-style",
	"list-style-image",
	"list-style-position",
	"list-style-type",
	"margin",
	"margin-bottom",
	"margin-left",
	"margin-right",
	"margin-top",
	"marker-offset",
	"marks",
	"max-height",
	"max-width",
	"min-height",
	"min-width",
	"orphans",
	"outline",
	"outline-color",
	"outline-style",
	"outline-width",
	"overflow",
	"padding",
	"padding-bottom",
	"padding-left",
	"padding-right",
	"padding-top",
	"page",
	"page-break-after",
	"page-break-before",
	"page-break-inside",
	"pause",
	"pause-after",
	"pause-before",
	"pitch",
	"pitch-range",
	"play-during",
	"position",
	"quotes",
	"richness",
	"right",
	"size",
	"speak",
	"speak-header",
	"speak-numeral",
	"speak-punctuation",
	"speech-rate",
	"stress",
	"table-layout",
	"text-align",
	"text-decoration",
	"text-indent",
	"text-shadow",
	"text-transform",
	"top",
	"unicode-bidi",
	"vertical-align",
	"visibility",
	"voice-family",
	"volume",
	"white-space",
	"widows",
	"width",
	"word-spacing",
	"z-index",
	NULL,
};

// This is used for properties that have no entries
const char *css_generic_no_entries[] = { "inherit", NULL };

const char *css_background_attachment[] = {"scroll","fixed", "inherit", NULL};
const char *css_background_color[] = { "transparent", "inherit", NULL};
const char *css_background_image[] = { "none", "inherit", NULL };
const char *css_background_position[] = {"top left","top center","top right","center left","center center","center right","bottom left","bottom center","bottom right", "inherit", NULL};
const char *css_background_repeat[] = {"repeat","repeat-x","repeat-y","no-repeat", "inherit", NULL};
const char *css_border[] = {"thin","medium","thick","none","hidden","dotted","dashed","solid","double","groove","ridge","inset","outset", "inherit", NULL};
const char *css_border_bottom[] = {"thin","medium","thick","","none","hidden","dotted","dashed","solid","double","groove","ridge","inset","outset", "inherit", NULL};
const char *css_border_bottom_color[] = { "transparent", "inherit", NULL };
const char *css_border_bottom_style[] = {"none","hidden","dotted","dashed","solid","double","groove","ridge","inset","outset", "inherit", NULL};
const char *css_border_bottom_width[] = {"thin","medium","thick", "inherit", NULL};
const char *css_border_color[] = {"transparent", "inherit", NULL};
const char *css_border_left[] = {"thin","medium","thick","none","hidden","dotted","dashed","solid","double","groove","ridge","inset","outset", "inherit", NULL};
const char *css_border_left_color[] = { "transparent", "inherit", NULL };
const char *css_border_left_style[] = {"none","hidden","dotted","dashed","solid","double","groove","ridge","inset","outset", "inherit", NULL};
const char *css_border_left_width[] = {"thin","medium","thick", "inherit", NULL};
const char *css_border_right[] = {"thin","medium","thick","none","hidden","dotted","dashed","solid","double","groove","ridge","inset","outset", "inherit", NULL};
const char *css_border_right_color[] = { "transparent", "inherit", NULL };
const char *css_border_right_style[] = {"none","hidden","dotted","dashed","solid","double","groove","ridge","inset","outset", "inherit", NULL};
const char *css_border_right_width[] = {"thin","medium","thick", "inherit", NULL};
const char *css_border_style[] = {"none","hidden","dotted","dashed","solid","double","groove","ridge","inset","outset", "inherit", NULL};
const char *css_border_top[] = {"thin","medium","thick","none","hidden","dotted","dashed","solid","double","groove","ridge","inset","outset", "inherit", NULL};
const char *css_border_top_color[] = { "transparent", "inherit", NULL };
const char *css_border_top_style[] = {"none","hidden","dotted","dashed","solid","double","groove","ridge","inset","outset", "inherit", NULL};
const char *css_border_top_width[] = {"thin","medium","thick", "inherit", NULL};
const char *css_border_width[] = {"thin","medium","thick", "inherit", NULL};
const char *css_clear[] = {"left","right","both","none", "inherit", NULL};
const char *css_clip[] = {"auto", "inherit", NULL};
const char *css_color[] = { "aqua", "black", "blue", "fuchsia", "gray", "green", "lime", "maroon", "navy", "olive", "orange", "purple", "red", "silver", "teal", "white", "yellow", "inherit", NULL };
const char *css_counter_increment[] = { "none", "inherit", NULL };
const char *css_counter_reset[] = { "none", "inherit", NULL };
const char *css_cue[] = { "none", "inherit", NULL };
const char *css_cue_after[] = { "none", "inherit", NULL };
const char *css_cue_before[] = { "none", "inherit", NULL };
const char *css_cursor[] = {"auto","crosshair","default","pointer","move","e-resize","ne-resize","nw-resize","n-resize","se-resize","sw-resize","s-resize","w-resize","TEXT","wait","help", "inherit", NULL};
const char *css_display[] = {"none","inline","block","list-item","run-in","compact","marker","table","inline-table","table-row-group","table-header-group","table-footer-group","table-row","table-column-group","table-column","table-cell","table-caption", "inherit", NULL};
const char *css_float[] = {"left","right","none", "inherit", NULL};
const char *css_position[] = {"static","relative","absolute","fixed", "inherit", NULL};
const char *css_visiabiltiy[] = {"visible","hidden","collapse", "inherit", NULL};
const char *css_font[] = {"caption","icon","menu","message-box","small-caption","status-bar", "inherit", NULL};
const char *css_font_family[] = {"serif", "sans-serif", "cursive", "monospace", "fantasy", "Arial, Helvetica, sans-serif", "'Arial Black', Gadget, sans-serif", "'Bookman Old Style', serif", "'Comic Sans MS', cursive", "Courier, monospace", "'Courier New', Courier, monospace",
	"Garamond, serif", "Georgia, serif", "Impact, Charcoal, sans-serif", "'Lucida Console', Monaco, monospace", "'Lucida Sans Unicode', 'Lucida Grande', sans-serif", "'MS Sans Serif', Geneva, sans-serif", "'MS Serif', 'New York', sans-serif",
	"'Palatino Linotype', 'Book Antiqua', Palatino, serif", "Symbol, sans-serif", "Tahoma, Geneva, sans-serif", "'Times New Roman', Times, serif", "'Trebuchet MS', Helvetica, sans-serif", "Verdana, Geneva, sans-serif", "Webdings, sans-serif",
	"Wingdings, 'Zapf Dingbats', sans-serif", "inherit", NULL };
const char *css_font_size[] = {"xx-small","x-small","small","medium","large","x-large","xx-large","smaller","larger", "inherit", NULL};
const char *css_font_stretch[] = {"normal","wider","narrower","ultra-condensed","extra-condensed","condensed","semi-condensed","semi-expanded","expanded","extra-expanded","ultra-expanded", "inherit", NULL};
const char *css_font_style[] = {"normal","italic","oblique", "inherit", NULL};
const char *css_font_variant[] = {"normal","small-caps", "inherit", NULL};
const char *css_font_weight[] = {"normal","bold","bolder","lighter","100","200","300","400","500","600","700","800","900", "inherit", NULL};
const char *css_height[] = {"auto", "inherit", NULL };
const char *css_left[] = { "auto", "inherit", NULL };
const char *css_content[] = {"counter(X,Y)","counters(X,Y,Z)","attr(X)","open-quote","close-quote","no-open-quote","no-close-quote", "inherit", NULL};
const char *css_list_style[] = {"inside","outside","none","disc","circle","square","decimal","decimal-leading-zero","lower-roman","upper-roman","lower-alpha","upper-alpha","lower-greek","lower-latin","upper-latin","hebrew","armenian","georgian","cjk-ideographic","hiragana","katakana","hiragana-iroha","katakana-iroha", "inherit", NULL};
const char *css_list_style_image[] = { "none", "inherit", NULL };
const char *css_list_style_position[] = {"inside","outside", "inherit", NULL};
const char *css_list_style_type[] = {"none","disc","circle","square","decimal","decimal-leading-zero","lower-roman","upper-roman","lower-alpha","upper-alpha","lower-greek","lower-latin","upper-latin","hebrew","armenian","georgian","cjk-ideographic","hiragana","katakana","hiragana-iroha","katakana-iroha", "inherit", NULL};
const char *css_marker_offset[] = {"auto", "inherit", NULL};
const char *css_max_height[] = { "none", "inherit", NULL };
const char *css_max_width[] = { "none", "inherit", NULL };
const char *css_min_height[] = { "none", "inherit", NULL };
const char *css_min_width[] = { "none", "inherit", NULL };
const char *css_outline[] = {"none","dotted","dashed","solid","double","groove","ridge","inset","outset","invert","thin","medium","thick", "inherit", NULL};
const char *css_outline_color[] = {"invert", "inherit", NULL};
const char *css_outline_style[] = {"none","dotted","dashed","solid","double","groove","ridge","inset","outset", "inherit", NULL};
const char *css_outline_width[] = {"thin","medium","thick", "inherit", NULL};
const char *css_overflow[] = {"visible","hidden","scroll","","auto", "inherit", NULL};
const char *css_vertical_align[] = {"baseline","sub","super","top","text-top","middle","bottom","text-bottom", "inherit", NULL};
const char *css_border_collapse[] = {"collapse","separate", "inherit", NULL};
const char *css_caption_side[] = {"top","bottom","left","right", "inherit", NULL};
const char *css_empty_cells[] = {"show","hide", "inherit", NULL};
const char *css_table_layout[] = {"auto","fixed", "inherit", NULL};
const char *css_direction[] = {"ltr","rtl", "inherit", NULL};
const char *css_line_height[] = {"normal", "inherit", NULL};
const char *css_letter_spacing[] = {"normal", "inherit", NULL};
const char *css_text_align[] = {"left","right","center","justify", "inherit", NULL};
const char *css_text_decoration[] = {"none","underline","overline","line-through","blink", "inherit", NULL};
const char *css_text_transform[] = {"none","capitalize","uppercase","lowercase", "inherit", NULL};
const char *css_unicode_bidi[] = {"normal","embed","bidi-override", "inherit", NULL};
const char *css_white_space[] = {"normal","pre","nowrap", "inherit", NULL};
const char *css_word_spacing[] = {"normal", "inherit", NULL};
const char *css_marks[] = {"none","crop","cross", "inherit", NULL};
const char *css_page_break_after[] = {"auto","always","avoid","left","right", "inherit", NULL};
const char *css_page_break_before[] = {"auto","always","avoid","left","right", "inherit", NULL};
const char *css_page_break_inside[] = {"auto","avoid", "inherit", NULL};
const char *css_size[] = {"auto","portrait","landscape", "inherit", NULL};
const char *css_azimuth[] = {"left-side","far-left","left","center-left","center","center-right","right","far-right","right-side","behind","leftwards","rightwards", "inherit", NULL};
const char *css_elevation[] = {"angle","below","level","above","higher","lower", "inherit", NULL};
const char *css_pitch[] = {"x-low","low","medium","high","x-high", "inherit", NULL};
const char *css_play_during[] = {"auto","none","mix","repeat", "inherit", NULL};
const char *css_speak[] = {"normal","none","spell-out", "inherit", NULL};
const char *css_speak_header[] = {"always","once", "inherit", NULL};
const char *css_speak_numeral[] = {"digits","continuous", "inherit", NULL};
const char *css_speak_punctuation[] = {"none","code", "inherit", NULL};
const char *css_speech_rate[] = {"x-slow","slow","medium","fast","x-fast","faster","slower", "inherit", NULL};
const char *css_volume[] = {"number","silent","x-soft","soft","medium","loud","x-loud", "inherit", NULL};
const char *css_quotes[] = { "none", "inherit", NULL };
const char *css_right[] = { "auto", "inherit", NULL };
const char *css_top[] = { "auto", "inherit", NULL };
const char *css_voice_family[] = { "male", "female", "child", "inherit", NULL };
const char *css_width[] = { "auto", "inherit", NULL };
const char *css_z_index[] = { "auto", "inherit", NULL };
const char *css_import[] = { "url()", NULL };

const char *css_bottom[] = { "auto", "inherit", NULL };

const char **cssRefPtr[] = {
	css_import,						// This is a faked entry
	css_azimuth,
	css_generic_no_entries,
	css_background_attachment,
	css_background_color,
	css_background_image,
	css_background_position,
	css_background_repeat,
	css_border,
	css_border_bottom,
	css_border_bottom_color,
	css_border_bottom_style,
	css_border_bottom_width,
	css_border_collapse,
	css_border_color,
	css_border_left,
	css_border_left_color,
	css_border_left_style,
	css_border_left_width,
	css_border_right,
	css_border_right_color,
	css_border_right_style,
	css_border_right_width,
	css_generic_no_entries,
	css_border_style,
	css_border_top,
	css_border_top_color,
	css_border_top_style,
	css_border_top_width,
	css_border_width,
	css_bottom,
	css_caption_side,
	css_clear,
	css_clip,
	css_color,
	css_content,
	css_counter_increment,
	css_counter_reset,
	css_cue,
	css_cue_after,
	css_cue_before,
	css_cursor,
	css_direction,
	css_display,
	css_elevation,
	css_empty_cells,
	css_float,
	css_font,
	css_font_family,
	css_font_size,
	css_generic_no_entries,
	css_font_stretch,
	css_font_style,
	css_font_variant,
	css_font_weight,
	css_height,
	css_left,
	css_letter_spacing,
	css_line_height,
	css_list_style,
	css_list_style_image,
	css_list_style_position,
	css_list_style_type,
	css_generic_no_entries,
	css_generic_no_entries,
	css_generic_no_entries,
	css_generic_no_entries,
	css_generic_no_entries,
	css_marker_offset,
	css_marks,
	css_max_height,
	css_max_width,
	css_min_height,
	css_min_width,
	css_generic_no_entries,
	css_outline,
	css_outline_color,
	css_outline_style,
	css_outline_width,
	css_overflow,
	css_generic_no_entries,
	css_generic_no_entries,
	css_generic_no_entries,
	css_generic_no_entries,
	css_generic_no_entries,
	css_generic_no_entries,
	css_page_break_after,
	css_page_break_before,
	css_page_break_inside,
	css_generic_no_entries,
	css_generic_no_entries,
	css_generic_no_entries,
	css_pitch,
	css_generic_no_entries,
	css_play_during,
	css_position,
	css_quotes,
	css_generic_no_entries,
	css_right,
	css_size,
	css_speak,
	css_speak_header,
	css_speak_numeral,
	css_speak_punctuation,
	css_speech_rate,
	css_generic_no_entries,
	css_table_layout,
	css_text_align,
	css_text_decoration,
	css_generic_no_entries,
	css_generic_no_entries,
	css_text_transform,
	css_top,
	css_unicode_bidi,
	css_vertical_align,
	css_visiabiltiy,
	css_voice_family,
	css_volume,
	css_white_space,
	css_generic_no_entries,
	css_width,
	css_word_spacing,
	css_z_index,
	NULL,
};

class VCSSSyntaxParams : public VObject, public ISyntaxParams
{
public:
	VCSSSyntaxParams () { fMap = new std::map< int, bool >; }
	virtual ~VCSSSyntaxParams () { delete fMap; }
	std::map< int, bool > *GetCommentMap() const { return fMap; }

private:
	void SelfDelete() {delete this;}
	std::map< int, bool > *fMap;
};

class CSSColorTipInfo : public VCodeEditorTipInfo
{
public:
	CSSColorTipInfo( ICodeEditorDocument *inDocument, VString inDisplayText, sBYTE inKind ) : VCodeEditorTipInfo( inDocument, inDisplayText, inKind ) {}

	virtual ITipInfo *Clone() const { return new CSSColorTipInfo( fDocument, fDisplayText, fKind ); }

	virtual void GetContentText( XBOX::VString &outText ) const 
	{
		outText.Clear();
		CCodeEditorComponent *codeEditor = VComponentManager::RetainComponentOfType< CCodeEditorComponent >();
		if (codeEditor) 
		{
			uBYTE red, green, blue;
			if ( codeEditor->RunColorPicker( GetUIView(), red, green, blue ) )
			{
				VString str_red, str_green, str_blue;
				str_red.FromWord( red );
				str_green.FromWord( green );
				str_blue.FromWord( blue );
				outText = CVSTR( " rgb( " ) + str_red + CVSTR( ", " ) + str_green + CVSTR( ", " ) + str_blue + CVSTR( " );" );

			}
			codeEditor->Release();
		}
	}
};

class CSSURLTipInfo : public VCodeEditorTipInfo
{
public:
	CSSURLTipInfo( ICodeEditorDocument *inDocument, VString inDisplayText, sBYTE inKind ) : VCodeEditorTipInfo( inDocument, inDisplayText, inKind ) {}
	virtual ITipInfo *Clone() const { return new CSSURLTipInfo( fDocument, fDisplayText, fKind ); }

	virtual void GetContentText( XBOX::VString &outText ) const 
	{
		VFilePath cssFilePath, path;
		VString relativePath;
		fDocument->GetPath( cssFilePath );
		CCodeEditorComponent *codeEditor = VComponentManager::RetainComponentOfType< CCodeEditorComponent >();
		if (codeEditor) 
		{
			fDocument->GetPath( cssFilePath );
			if ( codeEditor->RunGetFileDialog( GetUIView(), cssFilePath, relativePath ) )
				outText = CVSTR( " url( \"" ) + relativePath + CVSTR( "\" );" );
			codeEditor->Release();
		}
	}
};

VCSSSyntax::VCSSSyntax() : kLineKindInsideCurlyBraces( -2 )
{
	fAutoInsertBlock = false;
	fAutoInsertClosingChar = false;
	fAutoInsertTabs = false;
	fInsertSpaces = false;
	fTabWidth = 4;
}

VCSSSyntax::~VCSSSyntax()
{
}

void VCSSSyntax::Init( ICodeEditorDocument* inDocument )
{
	inDocument->SetStyle( eSelector,	true,  false, false,	0,	0,		0 );
	inDocument->SetStyle( eKeyword,		true,  false, false,	0,	255,	128 );
	inDocument->SetStyle( eNormal,		false, false, false,	0,	0,		0);
}

void VCSSSyntax::Load( ICodeEditorDocument* inDocument, VString& ioContent )
{
}

void VCSSSyntax::Save( ICodeEditorDocument* inDocument, VString& ioContent )
{
}

void VCSSSyntax::SaveLine( ICodeEditorDocument* inDocument, sLONG inLineIndex, VString& ioContent )
{
}

void VCSSSyntax::Close( ICodeEditorDocument* inDocument )
{
}

bool VCSSSyntax::DetermineMorphemeBoundary( ICodeEditorDocument *inDocument, sLONG inLineIndex, sLONG inOffset, sLONG &outLeftBoundary, sLONG &outLength, bool inAlternateKey )
{
	return false;
	/*
	VString xstr;
	inDocument->GetLine( inLineIndex, xstr );

	// Tokenize that line
	VCSSLexParser lexer;
	lexer.SetTabSize( 1 );
	lexer.Start( xstr.GetCPointer() );

	// Now that we have the tokens, look for the one containing the offset
	sLONG lastPos = 0;
	while (true) {
		try {
			if (CSSToken::END == lexer.Next( false ))	break;
		} catch (VCSSException err) {
			// We want to ignore it and keep trying
		}

		if (lastPos <= inOffset && lexer.GetCurColumn() - 1 >= inOffset) {
			// We've found the token we care about
			outLeftBoundary = lastPos;
			outLength = lexer.GetCurColumn() - lastPos - 1;	// We subtract one to account for the lexer's column count being 1-based
			return true;
		}
		lastPos = lexer.GetCurColumn() - 1;
	}

	// If we got here, then we've gotten to the end of the line.
	outLeftBoundary = xstr.GetLength();
	outLength = 0;

	return true;
	*/
}

std::map< int, bool > *VCSSSyntax::GetCommentMap( ICodeEditorDocument *inDocument )
{
	VCSSSyntaxParams *params = dynamic_cast< VCSSSyntaxParams * >( inDocument->GetSyntaxParams( 1 ) );

	if (NULL == params) {
		params = new VCSSSyntaxParams();
		inDocument->SetSyntaxParams( params, 1 );
	}
	return params->GetCommentMap();
}

void VCSSSyntax::GetColoringForTesting( const XBOX::VString &inSourceLine, std::vector< size_t > &outOffsets, std::vector< size_t > &outLengths, std::vector< sLONG > &outStyles )
{
	VCSSLexParser lexer;
	lexer.SetTabSize( 1 );
	lexer.Start( inSourceLine.GetCPointer() );

	bool bIsOpenString = false, bIsOpenComment = false;
	while (true) {
		try {
			if (CSSToken::END == lexer.Next( false ))	break;
		} catch (VCSSException exp) {
			// We want to eat the exception and keep trying.  However, the exception
			// will also give us some clues as to what we may have been dealing with.
			// For instance, a bad string or bad comment will have an error code set
			// letting us know what the problem is.
			bIsOpenString = (exp.GetError() == VE_CSS_LEX_PARSER_BAD_STRING);
			bIsOpenComment = (exp.GetError() == VE_CSS_LEX_PARSER_BAD_COMMENT);
		}

		// The token holds all of the information we need to care about.  We can tell
		// whether it's the start of a comment, and identifier, etc.  Only certain tokens
		// define a style change.  However, we need to watch out for comments since those
		// are returned as a comment start token, comment text, and comment end token.
		CSSToken::eCSSToken token = lexer.GetCurToken();
		VString tokenValue = lexer.GetCurTokenDumpValue();
		sBYTE style = eNormal;
		int lengthAdjustment = 0;

		switch (token) {
			case CSSToken::ATKEYWORD:
			case CSSToken::CLASS:
			case CSSToken::PSEUDO_CLASS:
			case CSSToken::FUNCTION:
			case CSSToken::IDENT: 	style = eName; break;
			case CSSToken::STRING: {
				style = eString;
				// Strings need to have their lengths adjusted because we want to
				// syntax highlight the quotes as well as the string content.
				lengthAdjustment = 2;
			} break;
			case CSSToken::HASH:
			case CSSToken::PERCENTAGE:
			case CSSToken::DIMENSION:
			case CSSToken::NUMBER:	style = eNumber; break;
			case CSSToken::COMMENT:	style = eComment; break;
			case CSSToken::IMPORTANT_SYMBOL: style = eNormal; break;
		}

		int pos = lexer.GetCurColumn() - 1;	// We subtract one because column counts are 1 based, but the code editor is 0 based
		if (bIsOpenString || bIsOpenComment) {
			// Open strings and comments cannot rely on the token being correct, but we are able to
			// make assumptions about them.  When you reach one of these states, you know that the state
			// holds until the end of the line.
			if (bIsOpenString)	style = eString;
			if (bIsOpenComment)	style = eComment;
			outOffsets.push_back( pos );
			outLengths.push_back( inSourceLine.GetLength() );
			outStyles.push_back( style );
			break;
		} else {
			// Set the style for that token
			outOffsets.push_back( pos );
			outLengths.push_back( pos + tokenValue.GetLength() + lengthAdjustment );
			outStyles.push_back( style );
		}
	}
}

void VCSSSyntax::GetTokensForTesting( VString& inSourceCode, std::vector<VString>& outTokens )
{

}

void VCSSSyntax::SetLine( ICodeEditorDocument *inDocument, sLONG inLineNumber, bool inLoading )
{
	// Get the state of the previous line (assuming we're not the first line in the 
	// document) to see whether it ended with an unfinished comment.  If it does, then
	// we want to tell the lexer about it so that it can resume lexing the unfinish
	// comment instead of assuming these are non-comment tokens.
	bool previousLineEndsWithComment = false;
	if (inLineNumber > 0) {
		std::map< int, bool > *lineMap = GetCommentMap( inDocument );
		if (lineMap) {
			previousLineEndsWithComment = (*lineMap)[ inLineNumber - 1 ];
		}
	}

	VString xstr;
	inDocument->GetLine( inLineNumber, xstr );

	// If the previous line ended with an open comment, we are going to fudge the
	// lexer slightly by prepending the open comment characters to it.  This will
	// allow us to continue the comment as expected without having to modify the
	// lexer at all.  However, it does mean we have to be careful about character
	// offsets for setting the style, since what we prepend is not actually part of
	// the document
	sLONG styleOffset = 0;
	if (previousLineEndsWithComment) {
		xstr = "/*" + xstr;
		styleOffset = -2;	// Two because we added two characters to the stream
	}

	VCSSLexParser lexer;
	lexer.SetTabSize( 1 );
	lexer.Start( xstr.GetCPointer() );

	std::vector<CSSToken::eCSSToken> tokens;
	bool bIsOpenString = false, bIsOpenComment = false;
	while (true) {
		try {
			if (CSSToken::END == lexer.Next( false ))	break;
		} catch (VCSSException exp) {
			// We want to eat the exception and keep trying.  However, the exception
			// will also give us some clues as to what we may have been dealing with.
			// For instance, a bad string or bad comment will have an error code set
			// letting us know what the problem is.
			bIsOpenString = (exp.GetError() == VE_CSS_LEX_PARSER_BAD_STRING);
			bIsOpenComment = (exp.GetError() == VE_CSS_LEX_PARSER_BAD_COMMENT);
		}

		// The token holds all of the information we need to care about.  We can tell
		// whether it's the start of a comment, and identifier, etc.  Only certain tokens
		// define a style change.  However, we need to watch out for comments since those
		// are returned as a comment start token, comment text, and comment end token.
		CSSToken::eCSSToken token = lexer.GetCurToken();
		VString tokenValue = lexer.GetCurTokenDumpValue();
		sBYTE style = eNormal;

		tokens.push_back(token);
		
		switch (token)
		{
			// In first instance, we want to colorize the following elements :
			// 1. Comments
			case CSSToken::COMMENT:
			{
				style = eComment;
				break;
			}

			// 2. Selectors
			case CSSToken::HASH:
			case CSSToken::CLASS:
			case CSSToken::PSEUDO_CLASS:
			{
				style = eSelector;
				break;
			}
				
			// 3. Properties
			case CSSToken::IDENT:
			{
				// Default value for identifier
				style = eNormal;
				
				// Some identifiers can be found in cssRefNameIndex an in cssRefPtr
				// Each category has its own style
				bool isPropertyToken = true;
				if( tokens.size() > 2 )
				{
					// Get the previous token
					int index = tokens.size()-2;
					CSSToken::eCSSToken curTok = tokens[index];
				
					// Skip space and comments
					while( index > 0 && (curTok == CSSToken::S || curTok == CSSToken::COMMENT) )
					{
						index--;
						curTok = tokens[index];
					}

					if( curTok == CSSToken::COLON )
					{
						// Get the previous token
						index--;
						curTok = tokens[index];

						// Skip space and comments
						while( index > 0 && (curTok == CSSToken::S || curTok == CSSToken::COMMENT) )
						{
							index--;
							curTok = tokens[index];
						}

						if( curTok == CSSToken::IDENT )
							isPropertyToken = false;
					}
				}

				if( isPropertyToken )
				{
					// Check if the identifier is a property name
					for (size_t i = 0; cssRefNameIndex[ i ] != NULL; i++)
					{
						VString test(cssRefNameIndex[ i ]);
						if( tokenValue.EqualTo( test, true ) )
						{
							style = eKeyword;
							break;
						}
					}
				}
				else
				{
					// Check if the identifier is a CSS defined name
					for (size_t i = 0; cssRefPtr[ i ] != NULL; i++)
					{
						for (size_t j = 0; cssRefPtr[ i ][ j ] != NULL; j++)
						{
							VString test(cssRefPtr[ i ][ j ]);
							if( tokenValue.EqualTo( test, true ) )
							{
								style = eNormal;
								break;
							}
						}
					}
				}
				
				break;
			}

			// 4. Functions
			case CSSToken::URI:
			case CSSToken::FUNCTION:
			{
				style = eName;
				
				for (size_t i = 0; cssRefNameIndex[ i ] != NULL; i++)
				{
					if (tokenValue.EqualTo( VString( cssRefNameIndex[ i ] ), true ))
					{
						style = eKeyword;
						break;
					}
				}
				
				break;
			}

			// 5. Strings
			case CSSToken::STRING:
			{
				style = eString;
				break;
			}
				
			// 6. Numbers
			case CSSToken::PERCENTAGE:
			case CSSToken::DIMENSION:
			case CSSToken::NUMBER:
			{
				style = eNumber;
				break;
			}
	

			// The default style is eNormal
			case CSSToken::ATKEYWORD:
			case CSSToken::INCLUDES:
			case CSSToken::DASHMATCH:
			case CSSToken::CDO:
			case CSSToken::CDC:
			case CSSToken::DOT:
			case CSSToken::COMMA:
			case CSSToken::COLON:
			case CSSToken::SEMI_COLON:
			case CSSToken::LEFT_CURLY_BRACE:
			case CSSToken::RIGHT_CURLY_BRACE:
			case CSSToken::LEFT_BRACE:
			case CSSToken::RIGHT_BRACE:
			case CSSToken::LEFT_BRACKET:
			case CSSToken::RIGHT_BRACKET:
			case CSSToken::MINUS:
			case CSSToken::PLUS:
			case CSSToken::DIV:
			case CSSToken::MUL:
			case CSSToken::GREATER:
			case CSSToken::EQUAL:
			case CSSToken::S:
			case CSSToken::IMPORTANT_SYMBOL:
			case CSSToken::DELIM:
			{
				style = eNormal;
				break;
			}

			default:
			{
				style = eNormal;
				break;
			}
		}

		int pos = lexer.GetCurColumn() + styleOffset - 1;	// We subtract one because column counts are 1 based, but the code editor is 0 based
		if (bIsOpenString || bIsOpenComment) {
			// Open strings and comments cannot rely on the token being correct, but we are able to
			// make assumptions about them.  When you reach one of these states, you know that the state
			// holds until the end of the line.
			if (bIsOpenString)	style = eString;
			if (bIsOpenComment)	style = eComment;
			inDocument->SetLineStyle( inLineNumber, Max( 0, pos ), xstr.GetLength() + styleOffset, style );	// Set the style to the end of the line
			break;
		} else {
			sLONG start = Max( 0, pos );
			sLONG end = Min( xstr.GetLength() - 1, pos + tokenValue.GetLength() );
			inDocument->SetLineStyle( inLineNumber, start, end, style );
		}
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
	std::map< int, bool > *lineMap = GetCommentMap( inDocument );
	bool previousOpenCommentState = (*lineMap)[ inLineNumber ];
	(*lineMap)[ inLineNumber ] = bIsOpenComment;

	// There are two cases we really need to care about.  If the line now ends in
	// an open comment (and didn't used to), we want to colorize down the document.
	// Also, if the line no longer ends in an open comment (but used to), we want to
	// colorize down the document.  In either case, we want to keep colorizing subsequent
	// lines until the comment is ended or the end of the document is reached.
	if ((!previousOpenCommentState && bIsOpenComment ||		// Now ends with open comment, didn't used to
		previousOpenCommentState && !bIsOpenComment) &&		// Used to end with an open comment, but no longer does
		inLineNumber + 1 < inDocument->GetNbLines()) {
		SetLine( inDocument, inLineNumber + 1, inLoading );
	}
}

bool VCSSSyntax::CheckFolding( ICodeEditorDocument* inDocument, sLONG inLineNumber )
{
	VString xstr;
	inDocument->GetLine( inLineNumber, xstr );
	return (xstr.FindUniChar( (UniChar)'{' ) != 0) || (xstr.FindUniChar( (UniChar)'}' ) != 0);
}

void VCSSSyntax::ComputeFolding( ICodeEditorDocument* inDocument )
{ 
	// Walk over all the lines in the document, looking for open curly braces and 
	// close curly braces.  These determine our foldable sections of code. We have to generate
	// the document ourselves instead of calling ICodeEditorDocument::GetCodeText because
	// GetCodeText returns us lines terminated with a \r, but the lexer expects newlines
	// to be terminated with a \n.  So we build the document ourselves.
	VString xstr;
	inDocument->GetCodeText( xstr );

	VCSSLexParser lexer;
	lexer.Start( xstr.GetCPointer() );

	// Loop over the tokens, and when we find an open bracket, we can push its location
	// onto a stack.  When we reach a closing bracket, we can pop the starting location from
	// the stack and mark those lines as foldable.
	std::vector< int > openBracketLocationStack;
	int lastPos = 0;
	while (true) {
		try {
			if (CSSToken::END == lexer.Next( false ))	break;
		} catch (VCSSException err) {
			// Eat it and keep going
		}

		if (CSSToken::LEFT_CURLY_BRACE == lexer.GetCurToken())	openBracketLocationStack.push_back( lexer.GetCurLine() - 1 );
		if (CSSToken::RIGHT_CURLY_BRACE == lexer.GetCurToken() && !openBracketLocationStack.empty()) {
			// Get the starting position for the curly brace
			int startLocation = openBracketLocationStack.back();
			int stopLocation = lexer.GetCurLine() - 1;
			openBracketLocationStack.pop_back();

			if (startLocation != stopLocation) {
				inDocument->SetFoldable( startLocation, true );
				inDocument->SetNbFoldedLines( startLocation, stopLocation - startLocation );

				// Now we want to modify each line in the set so that it knows whether it is inside
				// of the curly brace or outside of the curly brace.  Note that the start and stop
				// locations are both considered to be *outside* of the curly braces.
				for (int i = startLocation + 1; i <= stopLocation - 1; i++) {
					inDocument->SetLineKind( i, kLineKindInsideCurlyBraces );
				}
			}
		}
		if (CSSToken::COMMENT == lexer.GetCurToken()) {
			// If we found a comment, then we can set the foldable lines based on the last position
			// and the current position right now.
			int startLocation = lastPos;
			int stopLocation = lexer.GetCurLine() - 1;
			
			if (startLocation != stopLocation) {
				inDocument->SetFoldable( startLocation, true );
				inDocument->SetNbFoldedLines( startLocation, stopLocation - startLocation );
			}
		}

		lastPos = lexer.GetCurLine() - 1;
	}
}

bool VCSSSyntax::CheckOutline( ICodeEditorDocument* inInterface, sLONG inLineNumber )
{
	return false;
}

void VCSSSyntax::ComputeOutline( ICodeEditorDocument* inInterface )
{
}

void VCSSSyntax::TokenizeLine( ICodeEditorDocument* inDocument, sLONG inLineIndex, std::vector<XBOX::VString>& ioLines )
{
}

void VCSSSyntax::GetTip( ICodeEditorDocument *inDocument, sLONG inLine, sLONG inPos, VString &outText, Boolean &outRtfText )
{
	// We want to figure out what morpheme the user is after so that we can locate it in our XLIFF file.  If
	// we are able to do that, then we can display a helpful tip to the user.
	sLONG left = 0, length = 0;
	DetermineMorphemeBoundary( inDocument, inLine, inPos, left, length, false );

	// If we want to provide tips for property values as well as properties themselves, we can look to see if
	// there is a : preceeding this morpheme to determine the context for the value.  However, I don't think
	// many people are going to care about values since they can just hover over the property to get information
	// about the possible values anyways.

	if (length != 0) {
		VString xstr;
		inDocument->GetLine( inLine, xstr );

		xstr.GetSubString( left + 1, length, xstr );		// We add one to the left because VString is one-based...

		// We want to ensure that this name is unique to CSS tip messages
		xstr = CVSTR( "CSSTips%%" ) + xstr;

		VString aLocalizedString;
		extern VLanguageSyntaxComponent* gLanguageSyntax;
		if (gLanguageSyntax->GetLocalizationMgr()->LocalizeStringWithKey( xstr, aLocalizedString )) {
			outText = aLocalizedString;
			outRtfText = true;
		}
	}
}

const char *htmlTagNameIndex[] = {
	"@import",					// This lives here so we can complete it on the global level
	"a",
	"acronym",
	"address",
	"applet",
	"area",
	"b",
	"base",
	"basefont",
	"bgsound",
	"big",
	"blockquote",
	"body",
	"br",
	"button",
	"caption",
	"center",
	"cite",
	"code",
	"col",
	"colgroup",
	"comment",
	"dd",
	"del",
	"dfn",
	"dir",
	"div",
	"dl",
	"dt",
	"em",
	"embed",
	"fieldset",
	"font",
	"form",
	"frame",
	"frameset",
	"head",
	"h1",
	"h2",
	"h3",
	"h4",
	"h5",
	"h6",
	"hr",
	"html",
	"i",
	"iframe",
	"img",
	"input",
	"ins",
	"isindex",
	"kbd",
	"label",
	"legend",
	"li",
	"link",
	"listing",
	"map",
	"marquee",
	"menu",
	"meta",
	"nobr",
	"noframes",
	"noscript",
	"object",
	"ol",
	"option",
	"p",
	"param",
	"plaintext",
	"pre",
	"q",
	"s",
	"samp",
	"script",
	"select",
	"small",
	"span",
	"strike",
	"strong",
	"style",
	"sub",
	"sup",
	"table",
	"tbody",
	"td",
	"textarea",
	"tfoot",
	"th",
	"thead",
	"title",
	"tr",
	"tt",
	"u",
	"ul",
	"var",
	"wbr",
	"xmp",
	NULL,
};

void VCSSSyntax::GetSuggestionsForTesting( const XBOX::VString &inSourceLine, ISymbolTable *inSymTable, const XBOX::VString &inFilePath, sLONG inLineIndex, EJSCompletionTestingMode inSuggestionMode, std::vector< XBOX::VString > &outSuggestions )
{
	// Get the line text
	VString xstr = inSourceLine;

	// We are going to lex the tokens to determine what our completion context is, as well
	// as text.  However, don't let "context" fool you.  We're doing a very simple test to
	// see whether the completion is on the left or the right of a colon (:).  If it's on the left,
	// then we can suggest items from the CSS reference array.  If it's on the right, then we can
	// use the token to the left to make suggestions from the appropriate category.  For instance,
	// if the left-hand side of the colon is "volume", then we can suggest items from the css_volume
	// array, such as "number" or "silent."
	VCSSLexParser lexer;
	lexer.Start( xstr.GetCPointer() );

	// The "inLineIndex" parameter is actually a misnomer.  We use it as a flag to determine whether the
	// line is meant to be completed within curly braces or not.
	bool bFoundReference = false;
	bool bInCurlyBraces = (inLineIndex != 0);
	VString referenceText, completionText, lastTokenText;
	while (true) {
		try {
			if (CSSToken::END == lexer.Next()) {
				// If we've not found the reference, then we are going to assume the
				// last identifier we found is our completion text.
				if (!bFoundReference)	completionText = lastTokenText;
				break;
			}
		} catch (VCSSException exp) {
			// We want to eat the exception and keep trying.
		}

		// The token holds all of the information we need to care about.  We can tell
		// whether it's the start of a comment, and identifier, etc.  Only certain tokens
		// define a style change.  However, we need to watch out for comments since those
		// are returned as a comment start token, comment text, and comment end token.
		CSSToken::eCSSToken token = lexer.GetCurToken();
		VString tokenValue = lexer.GetCurTokenDumpValue();

		switch (token) {
			case CSSToken::ATKEYWORD: {
				if (tokenValue == "@import") {
					bInCurlyBraces = true;
					bFoundReference = true;
					referenceText = "@import";
					completionText = "";
				} else {
					lastTokenText = tokenValue;
				}
			} break;
			case CSSToken::LEFT_CURLY_BRACE: {
				bInCurlyBraces = true;
				completionText = lastTokenText = "";
			} break;
			case CSSToken::RIGHT_CURLY_BRACE: {
				bInCurlyBraces = false;
				completionText = lastTokenText = "";
			} break;
			case CSSToken::IDENT: {
				if (bFoundReference) {
					// Since we already found the reference, this must be the completion text
					completionText = tokenValue;
				} else {
					// If we've not found the reference yet, then we have no idea what this is,
					// so we are just going to store it as the last token's text.  When we find the 
					// colon, then we can set the reference text.
					lastTokenText = tokenValue;
				}
			} break;
			case CSSToken::PSEUDO_CLASS: {
				if (bInCurlyBraces && !bFoundReference) {
					bFoundReference = true;

					// The reference text is whatever the last token text was, and whatever our
					// completion text might have been should be wiped out
					referenceText = lastTokenText;
					completionText = tokenValue;
					completionText.SubString(2, completionText.GetLength()-1);
				}
			} break;
			case CSSToken::COLON: {
				if (bInCurlyBraces && !bFoundReference) {
					bFoundReference = true;

					// The reference text is whatever the last token text was, and whatever our
					// completion text might have been should be wiped out
					referenceText = lastTokenText;
					completionText = "";
				}
			} break;
			default: {
				// If we found something other than an ident or a colon, then we have no idea what we're
				// looking at, and we should start over.
				bFoundReference = false;
				completionText = "";
				referenceText = "";
				lastTokenText = tokenValue;
			} break;
		}
	}
	
	// If we found a reference, then we want to make suggestions based on the reference, otherwise we want
	// to suggest references themselves.  Either way, the end result is an array of const char * entries.
	const char **completions = NULL;
	if (bInCurlyBraces && bFoundReference) {
		// We found a reference, so let's search over the reference list and see if we can find a match.  If
		// we find the match, then we can set up the completions from it.
		for (size_t i = 0; cssRefNameIndex[ i ] != NULL; i++) {
			if (referenceText.EqualTo( VString( cssRefNameIndex[ i ] ), true )) {
				// We found a match to an entry in the reference list, so we're set!
				completions = cssRefPtr[ i ];
				break;
			}
		}
	} else if (!bInCurlyBraces) {
		completions = htmlTagNameIndex;
	} else {
		// Since we didn't find any references, we want to use the reference list as our completion list
		completions = cssRefNameIndex;
	}

	// If we have a completion list, then we can start the matching process
	if (!completions)	return;

	completionText.AppendChar( '^' );

	// We may have to post-process some entries in case there are special browsers required.  For instance, a color
	// picker may be needed, or a font dialog, etc.  In those cases, we will always add the browser to the list, regardless
	// of what the user has typed so far.  We will look up the Browse string from our localization file just once (since it's
	// not going to change as the application runs anyways).
	static VString sBrowseStr;
	extern VLanguageSyntaxComponent* gLanguageSyntax;
	if (!sBrowseStr.GetLength()) {
		gLanguageSyntax->GetLocalizationMgr()->LocalizeStringWithKey( "Browse", sBrowseStr );
	}

	if (completions == css_color) {
		outSuggestions.push_back( sBrowseStr );
	} else if (completions == css_background_image ||
				completions == css_list_style_image ||
				completions == css_import) {
		outSuggestions.push_back( sBrowseStr );
	}

	// Loop over all of the items in the keyword list and see if we can find any matches
	VCollator *collator = VIntlMgr::GetDefaultMgr()->GetCollator();
	UniChar oldLikeChar = collator->GetWildChar();
	collator->SetWildChar( '^' );
	for (size_t i = 0; completions[ i ] != NULL; i++) {
		VString completion = VString( completions[ i ] );
		// Notice that since JavaScript is a case-sensitive language, we want our comparisons to pay attention
		// to the case of the suggestions
		if (collator->EqualString_Like( completion.GetCPointer(), completion.GetLength(), completionText.GetCPointer(), completionText.GetLength(), true )) {
			// If this is a duplicate entry, then we do not want to add it to the list.  Duplicates
			// are normal, but fairly rare.
			bool bAlreadyInList = false;
			for (VectorOfVString::iterator iter2 = outSuggestions.begin(); !bAlreadyInList && iter2 != outSuggestions.end(); ++iter2) {
				if ((*iter2) == completion)	bAlreadyInList = true;
			}
			if (!bAlreadyInList)	outSuggestions.push_back( completion );
		}
	}
	collator->SetWildChar( oldLikeChar );
}

void VCSSSyntax::GetSuggestions( ICodeEditorDocument* inDocument, sLONG inLineIndex, sLONG inPos, ITipInfoArray *outSuggestions, sLONG& outStartOffset, bool inAll )
{
	// Get the line text
	VString xstr;
	inDocument->GetLine( inLineIndex, xstr );

	// Truncate it at the proper place
	xstr.Truncate( inPos );

	// We are going to lex the tokens to determine what our completion context is, as well
	// as text.  However, don't let "context" fool you.  We're doing a very simple test to
	// see whether the completion is on the left or the right of a colon (:).  If it's on the left,
	// then we can suggest items from the CSS reference array.  If it's on the right, then we can
	// use the token to the left to make suggestions from the appropriate category.  For instance,
	// if the left-hand side of the colon is "volume", then we can suggest items from the css_volume
	// array, such as "number" or "silent."
	VCSSLexParser lexer;
	lexer.Start( xstr.GetCPointer() );

	bool bFoundReference = false;
	bool bInCurlyBraces = (inDocument->GetLineKind( inLineIndex ) == kLineKindInsideCurlyBraces);
	VString referenceText, completionText, lastTokenText;
	while (true) {
		try {
			if (CSSToken::END == lexer.Next()) {
				// If we've not found the reference, then we are going to assume the
				// last identifier we found is our completion text.
				if (!bFoundReference)	completionText = lastTokenText;
				break;
			}
		} catch (VCSSException exp) {
			// We want to eat the exception and keep trying.
		}

		// The token holds all of the information we need to care about.  We can tell
		// whether it's the start of a comment, and identifier, etc.  Only certain tokens
		// define a style change.  However, we need to watch out for comments since those
		// are returned as a comment start token, comment text, and comment end token.
		CSSToken::eCSSToken token = lexer.GetCurToken();
		VString tokenValue = lexer.GetCurTokenDumpValue();

		switch (token) {
			case CSSToken::ATKEYWORD: {
				if (tokenValue == "@import") {
					bInCurlyBraces = true;
					bFoundReference = true;
					referenceText = "@import";
					completionText = "";
				} else {
					lastTokenText = tokenValue;
				}
			} break;
			case CSSToken::LEFT_CURLY_BRACE: {
				bInCurlyBraces = true;
				completionText = lastTokenText = "";
			} break;
			case CSSToken::RIGHT_CURLY_BRACE: {
				bInCurlyBraces = false;
				completionText = lastTokenText = "";
			} break;
			case CSSToken::IDENT: {
				if (bFoundReference) {
					// Since we already found the reference, this must be the completion text
					completionText = tokenValue;
				} else {
					// If we've not found the reference yet, then we have no idea what this is,
					// so we are just going to store it as the last token's text.  When we find the 
					// colon, then we can set the reference text.
					lastTokenText = tokenValue;
				}
			} break;
			case CSSToken::PSEUDO_CLASS: {
				if (bInCurlyBraces && !bFoundReference) {
					bFoundReference = true;

					// The reference text is whatever the last token text was, and whatever our
					// completion text might have been should be wiped out
					referenceText = lastTokenText;
					completionText = tokenValue;
					completionText.SubString(2, completionText.GetLength()-1);
				}
			} break;
			case CSSToken::COLON: {
				if (bInCurlyBraces && !bFoundReference) {
					bFoundReference = true;

					// The reference text is whatever the last token text was, and whatever our
					// completion text might have been should be wiped out
					referenceText = lastTokenText;
					completionText = "";
				}
			} break;
			default: {
				// If we found something other than an ident or a colon, then we have no idea what we're
				// looking at, and we should start over.
				bFoundReference = false;
				completionText = "";
				referenceText = "";
				lastTokenText = tokenValue;
			} break;
		}
	}
	
	// If we found a reference, then we want to make suggestions based on the reference, otherwise we want
	// to suggest references themselves.  Either way, the end result is an array of const char * entries.
	const char **completions = NULL;
	if (bInCurlyBraces && bFoundReference) {
		// We found a reference, so let's search over the reference list and see if we can find a match.  If
		// we find the match, then we can set up the completions from it.
		for (size_t i = 0; cssRefNameIndex[ i ] != NULL; i++) {
			if (referenceText.EqualTo( VString( cssRefNameIndex[ i ] ), true )) {
				// We found a match to an entry in the reference list, so we're set!
				completions = cssRefPtr[ i ];
				break;
			}
		}
	} else if (!bInCurlyBraces) {
		completions = htmlTagNameIndex;
	} else {
		// Since we didn't find any references, we want to use the reference list as our completion list
		completions = cssRefNameIndex;
	}

	// If we have a completion list, then we can start the matching process
	if (!completions)	return;

	completionText.AppendChar( '^' );
	outStartOffset = inPos - completionText.GetLength() + 1;

	// We may have to post-process some entries in case there are special browsers required.  For instance, a color
	// picker may be needed, or a font dialog, etc.  In those cases, we will always add the browser to the list, regardless
	// of what the user has typed so far.  We will look up the Browse string from our localization file just once (since it's
	// not going to change as the application runs anyways).
	static VString sBrowseStr;
	extern VLanguageSyntaxComponent* gLanguageSyntax;
	if (!sBrowseStr.GetLength()) {
		gLanguageSyntax->GetLocalizationMgr()->LocalizeStringWithKey( "Browse", sBrowseStr );
	}

	if (completions == css_color) {
		outSuggestions->AddTip( new CSSColorTipInfo( inDocument, sBrowseStr, 0 ) );
	} else if (completions == css_background_image ||
				completions == css_list_style_image ||
				completions == css_import) {
		outSuggestions->AddTip( new CSSURLTipInfo( inDocument, sBrowseStr, 0 ) );
	}

	// Loop over all of the items in the keyword list and see if we can find any matches
	VCollator *collator = VIntlMgr::GetDefaultMgr()->GetCollator();
	UniChar oldLikeChar = collator->GetWildChar();
	collator->SetWildChar( '^' );
	for (size_t i = 0; completions[ i ] != NULL; i++) {
		VString completion = VString( completions[ i ] );
		// Notice that since JavaScript is a case-sensitive language, we want our comparisons to pay attention
		// to the case of the suggestions
		if (collator->EqualString_Like( completion.GetCPointer(), completion.GetLength(), completionText.GetCPointer(), completionText.GetLength(), true )) {
			// If this is a duplicate entry, then we do not want to add it to the list.  Duplicates
			// are normal, but fairly rare.
			if (!outSuggestions->Contains( completion )) {
				VString display = completion;
				if (completions == cssRefNameIndex) {
					// We are completing the name of a CSS property, so we can go ahead and add a colon
					// for the user.
					completion.AppendString( ":" );
				} else if (completions != htmlTagNameIndex) {
					// We are completing a CSS property value, so let's close it off with the semi-colon
					completion.AppendString( ";" );
				}

				outSuggestions->AddTip( new VCodeEditorTipInfo( inDocument, display, completion, eName ) );
			}
		}
	}
	
	// START WAK0070381
	if(	completions == css_color ||
		completions == css_background_image ||
		completions == css_list_style_image ||
		completions == css_import)
	{
		// We don't want to propose the "<BROWSE>" suggestion in the case it is the only suggestion we have because
		// if it is the only suggestion we have, it will trigger the color picker / file chooser when the editor view
		// will try to call the GetContent method of CSSColorTipInfo / CSSURLTipInfo class
		//
		// To fix this behaviour issue, we don't suggest the color picker / file chooser in the two following cases :
		// 1. "<BROWWSE>" is the only suggestion
		// 2. the user has started to wrote something ... we assume he knows what he wants
		if( (outSuggestions->GetCount() == 1 && outSuggestions->Contains(sBrowseStr, true)) || 	(completionText != "^")	)
			outSuggestions->RemoveTip(0);
	}
	// END WAK0070381
	
	collator->SetWildChar( oldLikeChar );
}

static VString DetermineWhitespace( const VString &inText )
{
	// We want to look at the given text to figure out what leading whitespace there is, and return
	// that whitespace to the caller.
	VString ret;
	if (inText.GetLength() > 0) {
		const UniChar *p = inText.GetCPointer();
		const UniChar *end = p + inText.GetLength();
		while (p != end) {
			if ((*p == ' ') || (*p == '\t')) {		// TODO: we may want to consider other forms of whitespace based on the CSS specification.
				ret.AppendUniChar( *p );
			} else {
				break;
			}
			p++;
		}
	}

	return ret;
}

void VCSSSyntax::InsertChar( ICodeEditorDocument* inDocument, UniChar inUnichar, sLONG inLineIndex, sLONG inPosition, ITipInfoArray *outSuggestions, sLONG& outStartOffset )
{
	// We do special processing if the user is currently inside of a comment.  Namely, when
	// they are editing a comment, we don't do any special processing of character insertions.
	if (inUnichar != '\r' &&
		inUnichar != '{' &&
		inUnichar != '"' &&
		inUnichar != '[' &&
		inUnichar != '(' &&
		inUnichar != ':') {
		// There's no special processing to do, so let's just bail out and skip all the extra work
		return;
	}

	// Get the state of the previous line (assuming we're not the first line in the 
	// document) to see whether it ended with an unfinished comment.  If it does, then
	// we want to tell the lexer about it so that it can resume lexing the unfinish
	// comment instead of assuming these are non-comment tokens.
	bool previousLineEndsWithComment = false;
	if (inLineIndex > 0) {
		std::map< int, bool > *lineMap = GetCommentMap( inDocument );
		if (lineMap) {
			previousLineEndsWithComment = (*lineMap)[ inLineIndex - 1 ];
		}
	}

	// What we need to test is whether the current position within the given  line is inside 
	// of a comment or not.  Now that we know whether or not the previous line ended with a comment
	// we can lex the current line up to the insertion position to see whether we want to do any 
	// special processing of the inserted character.  We truncate at the character before the 
	// insertion position because the character has already been inserted in the stream.
	VString xstr;
	inDocument->GetLine( inLineIndex, xstr );
	if (inPosition > 0)		xstr.Truncate( inPosition - 1 );

	// If the previous line ended with an open comment, we are going to fudge the
	// lexer slightly by prepending the open comment characters to it.  This will
	// allow us to continue the comment as expected without having to modify the
	// lexer at all.  However, it does mean we have to be careful about character
	// offsets for setting the style, since what we prepend is not actually part of
	// the document
	if (previousLineEndsWithComment)	xstr = "/*" + xstr;

	VCSSLexParser lexer;
	lexer.Start( xstr.GetCPointer() );

	while (true) {
		try {
			if (CSSToken::END == lexer.Next( false ))	break;
		} catch (VCSSException exp) {
			if (exp.GetError() == VE_CSS_LEX_PARSER_BAD_STRING ||
				exp.GetError() == VE_CSS_LEX_PARSER_BAD_COMMENT) {
				// If we are part of an open string or comment, then we want to bail out and do no
				// special processing for this character
				return;
			}
		}
	}

	// Now that we know it's safe to do, let's do our final processing!  
	//////////
	//	Performance note
	//	If you add any cases to the switch, you must also add them to the check at the
	//	top of the function.  This allows us to do less processing on the majority of 
	//	user input while still providing special behaviors for non-commented cases.
	//////////
	switch (inUnichar) {
		case '\r': {
			// The user is entering a newline, so we want to ensure that the indentation
			// matches whatever the previous line's indentation is.  The line number passed
			// in to us is the actual line we're currently on, since the newline has already
			// been inserted.  So we want to look at the previous line's text to see how much
			// preceeding whitespace there is.
			VString preceedingLine;
			inDocument->GetLine( inLineIndex - 1, preceedingLine );

			// If we have any whitespace to insert, then insert it
			VString whitespace = DetermineWhitespace( preceedingLine );

			// If the preceeding line ends with a {, then we want to indent once more
			if (!preceedingLine.IsEmpty() && preceedingLine.GetUniChar( preceedingLine.GetLength() ) == (UniChar)'{')	whitespace.AppendChar( '\t' );
			if (!whitespace.IsEmpty())	inDocument->InsertText( whitespace );

			// We also want to make suggestions as to what the user can enter
			GetSuggestions( inDocument, inLineIndex, inPosition, outSuggestions, outStartOffset, false );
		} break;

		case '(': {
			inDocument->InsertText( ")" );
			inDocument->Select( inPosition, inPosition, inLineIndex, inLineIndex );
		} break;

		case '[': {
			inDocument->InsertText( "]" );
			inDocument->Select( inPosition, inPosition, inLineIndex, inLineIndex );
		} break;

		case '"': {
			inDocument->InsertText( "\"" );
			inDocument->Select( inPosition, inPosition, inLineIndex, inLineIndex );
		} break;

		case '{': {
			// First, we're adding a newline character
			VString insertText = CVSTR( "\r" );

			// Then a particular amount of whitespace based on the current line's indent depth
			VString lineText;
			inDocument->GetLine( inLineIndex, lineText );
			VString whitespace = DetermineWhitespace( lineText );
			insertText += whitespace + CVSTR( "\t" );

			// And then another newline with the closing brace
			insertText += CVSTR( "\r" ) + whitespace + CVSTR( "}" );
			inDocument->InsertText( insertText );
			inDocument->Select( whitespace.GetLength() + 1, whitespace.GetLength() + 1, inLineIndex + 1, inLineIndex + 1 );
		} break;

		case ':': {
			// When the user enters a colon, we want to automatically pop up the suggestion
			// list for them
			GetSuggestions( inDocument, inLineIndex, inPosition, outSuggestions, outStartOffset, false );

			// Also, we take a look at the morpheme directly before the colon so that we can update
			// the status bar text
			sLONG left = 0, length = 0;
			DetermineMorphemeBoundary( inDocument, inLineIndex, inPosition - 2, left, length, false );	// We subtract 2 to put us in the preceeding token

			if (length != 0) {
				VString xstr;
				inDocument->GetLine( inLineIndex, xstr );

				xstr.GetSubString( left + 1, length, xstr );		// We add one to the left because VString is one-based...

				// We want to ensure that this name is unique to CSS status bar messages
				xstr = CVSTR( "CSSStatusMessages%%" ) + xstr;

				VString aLocalizedString;
				extern VLanguageSyntaxComponent* gLanguageSyntax;
				if (gLanguageSyntax->GetLocalizationMgr()->LocalizeStringWithKey( xstr, aLocalizedString )) {
					// Now that we have the localized string, we can add it to the status bar
					inDocument->SetCommentText( aLocalizedString );
				}
			}
		} break;
	}
}

void VCSSSyntax::UpdateBreakPointsLineNumbers( ICodeEditorDocument* inDocument, const std::vector<sWORD>& inBreakIDs, const std::vector<sLONG>& inLineNumbers )
{

}

void VCSSSyntax::AddBreakPoint( ICodeEditorDocument* inDocument, sLONG inLineNumber, sWORD& outID, bool& outDisabled )
{
	outID = 0;
}

bool VCSSSyntax::EditBreakPoint( ICodeEditorDocument* inDocument, sWORD inBreakID, bool& outDisabled )
{
	return false;
}

void VCSSSyntax::RemoveBreakPoint( ICodeEditorDocument* inDocument, sLONG inLineNumber )
{

}

bool VCSSSyntax::GetBreakPoints( ICodeEditorDocument* inDocument, std::vector<sLONG>& outLineNumbers, std::vector<sWORD>& outIDs, std::vector<bool>& outDisabled )
{
	return false;
}

void VCSSSyntax::UpdateCompilerErrors( ICodeEditorDocument* inDocument )
{
	// Walk over all the lines in the document, looking for open curly braces and 
	// close curly braces.  These determine our foldable sections of code. We have to generate
	// the document ourselves instead of calling ICodeEditorDocument::GetCodeText because
	// GetCodeText returns us lines terminated with a \r, but the lexer expects newlines
	// to be terminated with a \n.  So we build the document ourselves.
	VString xstr;
	inDocument->GetCodeText( xstr );

	VCSSParser parser( true );
	CSSMedia::Set medias;
	medias.insert((int)CSSMedia::ALL);
	CSSRuleSet::MediaRules rules;

	try {
		parser.Parse( xstr, "", &rules, &medias );
	} catch (VCSSExceptionList errList) {
		std::vector< VCSSException > list = errList.GetErrors();
		for (std::vector< VCSSException >::iterator iter = list.begin(); iter != list.end(); ++iter) {
			// We subtract one from the exception's line number because exception line numbers
			// are one-based, but the document's line numbers are zero-based.
			sLONG lineNumber = (*iter).GetLine() - 1;
			xbox_assert( lineNumber >= 0 );
			xbox_assert( lineNumber < inDocument->GetNbLines() );
			inDocument->SetLineCompilerError( lineNumber, "Syntax error" );
		}
	}
}

void VCSSSyntax::GotoNextMethodError( ICodeEditorDocument* inDocument, bool inNext )
{
}

void VCSSSyntax::SwapAssignments( ICodeEditorDocument* inDocument, VString& ioString )
{
}

bool VCSSSyntax::IsComment( ICodeEditorDocument* inDocument, const VString& inString )
{
	// We will hand the string off to the lexer, and if a single token comes back that 
	// says "this is a comment", then we're set.  If multiple tokens come back that are
	// non-white space, we know it's not a comment.
	VCSSLexParser lexer;
	lexer.Start( inString.GetCPointer() );

	// Loop over the tokens and see what we've got
	bool isComment = false;

	while (true) {
		try {
			if (CSSToken::END == lexer.Next( false )) break;
		} catch (VCSSException exp) {
			// We want to eat the exception and keep trying.
		}

		switch (lexer.GetCurToken()) {
			case CSSToken::S:			/* Whitespace can be safely ignored */ break;
			case CSSToken::COMMENT:		isComment = true; break;
			default:					return false;
		}
	}

	return isComment;
}

void VCSSSyntax::SwapComment( ICodeEditorDocument* inDocument, VString& ioString, bool inComment )
{
	// We've been given a string that we either need to comment, or uncomment, depending
	// on the state of the "inComment" parameter.  We will do a simple validation if the caller
	// is asking us to uncomment something which isn't commented.
	if (inComment) {
		// This is the easy case -- we just need to take the string and wrap it with the appropriate
		// comment characters.
		ioString = VString( "/*" ) + ioString + VString( "*/" );
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
		for (VIndex i = 0; i < ioString.GetLength(); ++i) {
			// We know the first character we will happen upon will be the "/" since we've already
			// validated that this is a comment.
			if (ioString[ i ] == (UniChar)'/') {
				// Remove this character and the one that follows it.  Except that i need to be
				// base one for this call
				ioString.Remove( i + 1, 2 );
				// We're done with the opening comment indicator
				break;
			}
		}

		// Handle the close comment
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

void VCSSSyntax::CallHelp( ICodeEditorDocument* inDocument )
{
}

bool VCSSSyntax::ShouldValidateTipWindow( UniChar inChar )
{
	return inChar == '(' || inChar == ':' || inChar == '=' || inChar == ';' || 
		   inChar == '<' || inChar == '>' || /*inChar == '+' || inChar == '-' ||*/
		   inChar == '{' || inChar == '/' || inChar == '#' || inChar == '[';
}


bool VCSSSyntax::IsMatchingCharacter( ICodeEditorDocument* inDocument, sLONG inLineIndex, sLONG inOffset, UniChar inChar )
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

bool VCSSSyntax::DoubleClick( ICodeEditorDocument* inDocument, sLONG inLineIndex, sLONG inOffset, bool inAlternateKey )
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
