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
	"align-content",
	"align-items",
	"align-self",
	"animation",
	"animation-delay",
	"animation-direction",
	"animation-duration",
	"animation-fill-mode",
	"animation-iteration-count",
	"animation-name",
	"animation-play-state",
	"animation-timing-function",
	"backface-visibility",
	"background",
	"background-attachment",
	"background-clip",
	"background-color",
	"background-image",
	"background-origin",
	"background-position",
	"background-repeat",
	"background-size",
	"border",
	"border-bottom",
	"border-bottom-color",
	"border-bottom-left-radius",
	"border-bottom-right-radius",
	"border-bottom-style",
	"border-bottom-width",
	"border-collapse",
	"border-color",
	"border-image",
	"border-image-outset",
	"border-image-repeat",
	"border-image-slice",
	"border-image-source",
	"border-image-width",
	"border-left",
	"border-left-color",
	"border-left-style",
	"border-left-width",
	"border-radius",
	"border-right",
	"border-right-color",
	"border-right-style",
	"border-right-width",
	"border-spacing",
	"border-style",
	"border-top",
	"border-top-color",
	"border-top-left-radius",
	"border-top-right-radius",
	"border-top-style",
	"border-top-width",
	"border-width",
	"bottom",
	"box-shadow",
	"box-sizing",
	"caption-side",
	"clear",
	"clip",
	"color",
	"column-count",
	"column-fill",
	"column-gap",
	"column-rule",
	"column-rule-color",
	"column-rule-style",
	"column-rule-width",
	"column-span",
	"column-width",
	"columns",
	"content",
	"counter-increment",
	"counter-reset",
	"cursor",
	"direction",
	"display",
	"empty-cells",
	"flex",
	"flex-basis",
	"flex-direction",
	"flex-flow",
	"flex-grow",
	"flex-shrink",
	"flex-wrap",
	"float",
	"font",
	"font-family",
	"font-size",
	"font-size-adjust",
	"font-stretch",
	"font-style",
	"font-variant",
	"font-weight",
	"hanging-punctuation",
	"height",
	"icon",
	"justify-content",
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
	"max-height",
	"max-width",
	"min-height",
	"min-width",
	"nav-down",
	"nav-index",
	"nav-left",
	"nav-right",
	"nav-up",
	"opacity",
	"order",
	"outline",
	"outline-color",
	"outline-offset",
	"outline-style",
	"outline-width",
	"overflow",
	"overflow-x",
	"overflow-y",
	"padding",
	"padding-bottom",
	"padding-left",
	"padding-right",
	"padding-top",
	"page-break-after",
	"page-break-before",
	"page-break-inside",
	"perspective",
	"perspective-origin",
	"position",
	"quotes",
	"resize",
	"right",
	"tab-size",
	"table-layout",
	"text-align",
	"text-align-last",
	"text-decoration",
	"text-decoration-color",
	"text-decoration-line",
	"text-indent",
	"text-justify",
	"text-overflow",
	"text-shadow",
	"text-transform",
	"top",
	"transform",
	"transform-origin",
	"transform-style",
	"transition",
	"transition-delay",
	"transition-duration",
	"transition-property",
	"transition-timing-function",
	"unicode-bidi",
	"vertical-align",
	"visibility",
	"white-space",
	"width",
	"word-break",
	"word-spacing",
	"word-wrap",
	"z-index",
	NULL,
};

// This is used for properties that have no entries
const char *css_generic_no_entries[] =			{ "initial", "inherit", NULL };
const char *css_generic_none[] =				{ "none", "initial", "inherit", NULL };
const char *css_generic_auto[] =				{ "auto", "initial", "inherit", NULL };
const char *css_generic_normal[] =				{ "normal", "initial", "inherit", NULL };

const char *css_import[] =						{ "url()", NULL }; 
const char *css_align_content[ ] =				{ "stretch", "center", "flex-start", "flex-end", "space-between", "space-around", "initial", "inherit", NULL };
const char *css_align_items[] =					{ "stretch", "center", "flex-start", "flex-end", "baseline", "initial", "inherit", NULL };
const char *css_align_self[] =					{ "auto", "stretch", "center", "flex-start", "flex-end", "baseline", "initial", "inherit", NULL };
const char *css_animation[ ] =					{ "none",																	// name
												  "linear", "ease", "ease-in", "ease-out", "ease-in-out", "cubic-bezier",	// timing-function
												  "infinite",																// iteration-count
												  "normal", "reverse", "alternate", "alternate-reverse",					// direction
												  "forwards", "backwards", "both",											// fill-mode
												  "paused", "running",														// play-state
												  "initial", "inherit", NULL }; 
const char *css_animation_direction[] =			{ "normal", "reverse", "alternate", "alternate-reverse", "initial", "inherit", NULL };
const char *css_animation_fill_mode[] =			{ "none", "forwards", "backwards", "both", "initial", "inherit", NULL };
const char *css_animation_iteration_count[] =	{ "infinite", "initial", "inherit", NULL };
const char *css_animation_play_state[] =		{ "paused", "running", "initial", "inherit", NULL };
const char *css_animation_timing_function[] =	{ "linear", "ease", "ease-in", "ease-out", "ease-in-out", "cubic-bezier", "initial", "inherit", NULL };
const char *css_backface_visibility[] =			{ "visible", "hidden", "initial", "inherit", NULL };
const char *css_background[] =					{ "transparent",															// image
												  "left top", "left center", "left bottom", "right top", "right center",	// position
												  "right bottom", "center top", "center center", "center bottom",			// position
												  "auto", "cover", "contain",												// size
												  "repeat", "repeat-x", "repeat-y", "no-repeat",							// repeat
												  "padding-box", "border-box", "content-box",								// origin + clip
												  "scroll", "fixed", "local",												// attachment
												  "initial", "inherit", NULL };
const char *css_background_attachment[] =		{ "scroll", "fixed", "local", "inherit", NULL};
const char *css_background_clip[] =				{ "padding-box", "border-box", "content-box", "initial", "inherit", NULL };
const char *css_background_image[] =			{ "url", "none", "initial", "inherit", NULL };
const char *css_background_origin[] =			{ "padding-box", "border-box", "content-box", "initial", "inherit", NULL };
const char *css_background_position[] =			{ "left top", "left center", "left bottom", "right top", "right center", "right bottom", 
												  "center top", "center center", "center bottom", "initial", "inherit", NULL};
const char *css_background_repeat[] =			{ "repeat", "repeat-x", "repeat-y", "no-repeat", "initial", "inherit", NULL};
const char *css_background_size[] =				{ "auto", "cover", "contain", "initial", "inherit", NULL };
const char *css_border[] =						{ "medium", "thin", "thick",																			// border-width
												  "none", "hidden", "dotted", "dashed", "solid", "double", "groove", "ridge", "inset", "outset",		// border-style
												  "transparent",																						// border-color
												  "initial", "inherit", NULL };
const char *css_border_style[] =				{ "none", "hidden", "dotted", "dashed", "solid", "double", "groove", "ridge", "inset", "outset", "initial", "inherit", NULL};
const char *css_border_width[] =				{ "medium", "thin", "thick", "initial", "inherit", NULL};
const char *css_border_collapse[] =				{ "separate", "collapse", "initial", "inherit", NULL };
const char *css_border_image[] =				{ "none",									// border-image-source
												  "fill",									// border-image-slice
												  "auto",									// border-image-width
												  "stretch", "repeat", "round", "space",	// border-image-repeat 
												  "initial", "inherit", NULL };
const char *css_border_image_repeat[] =			{ "stretch", "repeat", "round", "space", "initial", "inherit", NULL };
const char *css_border_image_slice[] =			{ "fill", "initial", "inherit", NULL };
const char *css_box_shadow[] =					{ "none", "inset", "initial", "inherit", NULL };
const char *css_box_sizing[] =					{ "content-box", "border-box", "inset", "initial", "inherit", NULL };
const char *css_caption_side[] =				{ "top", "bottom", "initial", "inherit", NULL };
const char *css_clear[] =						{ "none", "left", "right", "both", "initial", "inherit", NULL };
const char *css_color[] =						{ "transparent", "AliceBlue", "AntiqueWhite", "Aqua", "Aquamarine", "Azure", "Beige", "Bisque", "Black", "BlanchedAlmond", "Blue",
												  "BlueViolet", "Brown", "BurlyWood", "CadetBlue", "Chartreuse", "Chocolate", "Coral", "CornflowerBlue", "Cornsilk",
												  "Crimson", "Cyan", "DarkBlue", "DarkCyan", "DarkGoldenRod", "DarkGray", "DarkGreen", "DarkKhaki", "DarkMagenta",
												  "DarkOliveGreen", "DarkOrange", "DarkOrchid", "DarkRed", "DarkSalmon", "DarkSeaGreen", "DarkSlateBlue", "DarkSlateGray",
												  "DarkTurquoise", "DarkViolet", "DeepPink", "DeepSkyBlue", "DimGray", "DodgerBlue", "FireBrick", "FloralWhite", "ForestGreen",
												  "Fuchsia", "Gainsboro", "GhostWhite", "Gold", "GoldenRod", "Gray", "Green", "GreenYellow", "HoneyDew", "HotPink", "IndianRed",
												  "Indigo", "Ivory", "Khaki", "Lavender", "LavenderBlush", "LawnGreen", "LemonChiffon", "LightBlue", "LightCoral", "LightCyan",
												  "LightGoldenRodYellow", "LightGray", "LightGreen", "LightPink", "LightSalmon", "LightSeaGreen", "LightSkyBlue", "LightSlateGray",
												  "LightSteelBlue", "LightYellow", "Lime", "LimeGreen", "Linen", "Magenta", "Maroon", "MediumAquaMarine", "MediumBlue", "MediumOrchid",
												  "MediumPurple", "MediumSeaGreen", "MediumSlateBlue", "MediumSpringGreen", "MediumTurquoise", "MediumVioletRed", "MidnightBlue", 
												  "MintCream", "MistyRose", "Moccasin", "NavajoWhite", "Navy", "OldLace", "Olive", "OliveDrab", "Orange", "OrangeRed", "Orchid", 
												  "PaleGoldenRod", "PaleGreen", "PaleTurquoise", "PaleVioletRed", "PapayaWhip", "PeachPuff", "Peru", "Pink", "Plum", "PowderBlue", 
												  "Purple", "Red", "RosyBrown", "RoyalBlue", "SaddleBrown", "Salmon", "SandyBrown", "SeaGreen", "SeaShell", "Sienna", "Silver", 
												  "SkyBlue", "SlateBlue", "SlateGray", "Snow", "SpringGreen", "SteelBlue", "Tan", "Teal", "Thistle", "Tomato", "Turquoise", 
												  "Violet", "Wheat", "White", "WhiteSmoke", "Yellow", "YellowGreen", "initial", "inherit", NULL };
const char *css_column_count[] =				{ "auto", "initial", "inherit", NULL };
const char *css_column_fill[] =					{ "balance", "auto", "initial", "inherit", NULL };
const char *css_column_gap[] =					{ "normal", "auto", "initial", "inherit", NULL };
const char *css_column_rule[] =					{ "medium", "thin", "thick",																		// column-rule-width
												  "none", "hidden", "dotted", "dashed", "solid", "double", "groove", "ridge", "inset", "outset",	// colum-rule-style
												  "initial", "inherit", NULL };
const char *css_column_rule_style[] =			{ "none", "hidden", "dotted", "dashed", "solid", "double", "groove", "ridge", "inset", "outset",	"initial", "inherit", NULL };
const char *css_column_rule_width[] =			{ "medium", "thin", "thick", "initial", "inherit", NULL };
const char *css_column_span[] =					{ "1", "all", "initial", "inherit", NULL };
const char *css_column[] =						{ "auto", "initial", "inherit", NULL };
const char *css_content[] =						{ "normal", "none", "counter", "attr", "open-quote", "close-quote", "no-open-quote", "no-close-quote", "url", "initial", "inherit", NULL };
const char *css_cursor[] =						{ "alias", "all-scroll", "auto", "cell", "context-menu", "col-resize", "copy", "crosshair", "default", "e-resize", "ew-resize", "help", "move",
												  "n-resize", "ne-resize", "nesw-resize", "ns-resize", "nw-resize", "nwse-resize", "no-drop", "none", "not-allowed", "pointer", "progress",
												  "row-resize", "s-resize", "se-resize", "sw-resize", "text", "vertical-text", "w-resize", "wait", "zoom-in", "zoom-out", "initial", "inherit", NULL };
const char *css_direction[] =					{ "ltr", "rtl", "initial", "inherit", NULL };
const char *css_display[] =						{ "inline", "block", "flex", "inline-block", "inline-flex", "inline-table", "list-item", "run-in", "table", "table-caption",
												  "table-column-group", "table-header-group", "table-row-group", "table-cell", "table-column", "table-row", "none", "initial", "inherit", NULL };
const char *css_empty_cells[] =					{ "show", "hide", "initial", "inherit", NULL };
const char *css_flex[] =						{ "auto", "none", "initial", "inherit", NULL };
const char *css_flex_direction[] =				{ "row", "row-reverse", "column", "column-reverse", "initial", "inherit", NULL };
const char *css_flex_flow[] =					{ "row", "row-reverse", "column", "column-reverse", "nowrap", "wrap", "wrap-reverse", "initial", "inherit", NULL };
const char *css_flex_wrap[] =					{ "nowrap", "wrap", "wrap-reverse", "initial", "inherit", NULL };
const char *css_float[] =						{ "none", "left", "right", "initial", "inherit", NULL };
const char *css_font[] =						{ "normal", "italic", "oblique",																	// font-style
												  "small-caps",																						// font-variant
												  "bold", "bolder", "lighter", "100", "200", "300", "400", "500", "600", "700", "800", "900",		// font-weight
												  "medium", "xx-small", "x-small", "small", "large", "x-large", "xx-large", "smaller", "larger",	// font-size
												  "caption", "icon", "menu", "message-box", "small-caption", "status-bar", "initial", "inherit", NULL };
const char *css_font_family[] =					{	"serif", "sans-serif", "cursive", "monospace", "fantasy", "Arial, Helvetica, sans-serif", "'Arial Black', Gadget, sans-serif", "'Bookman Old Style', serif", "'Comic Sans MS', cursive", "Courier, monospace", "'Courier New', Courier, monospace",
													"Garamond, serif", "Georgia, serif", "Impact, Charcoal, sans-serif", "'Lucida Console', Monaco, monospace", "'Lucida Sans Unicode', 'Lucida Grande', sans-serif", "'MS Sans Serif', Geneva, sans-serif", "'MS Serif', 'New York', sans-serif",
													"'Palatino Linotype', 'Book Antiqua', Palatino, serif", "Symbol, sans-serif", "Tahoma, Geneva, sans-serif", "'Times New Roman', Times, serif", "'Trebuchet MS', Helvetica, sans-serif", "Verdana, Geneva, sans-serif", "Webdings, sans-serif",
													"Wingdings, 'Zapf Dingbats', sans-serif", "initial", "inherit", NULL };
const char *css_font_size[] =					{ "medium", "xx-small", "x-small", "small", "large", "x-large", "xx-large", "smaller", "larger", "initial", "inherit", NULL };
const char *css_font_stretch[] =				{ "wider", "narrower", "ultra-condensed", "extra-condensed", "condensed", "semi-condensed", "normal", "semi-expanded", "expanded",
												  "extra-expanded", "ultra-expanded", "initial", "inherit", NULL };
const char *css_font_style[] =					{ "normal", "italic", "oblique", "initial", "inherit", NULL };
const char *css_font_variant[ ] =				{ "normal", "small-caps", "initial", "inherit", NULL };
const char *css_font_weight[] =					{ "normal", "bold", "bolder", "lighter", "100", "200", "300", "400", "500", "600", "700", "800", "900", "initial", "inherit", NULL };
const char *css_hanging_ponctuation[] =			{ "none", "first", "last", "allow-end", "force-end", "initial", "inherit", NULL };
const char *css_justify_content[] =				{ "flex-start", "flex-end", "center", "space-between", "space-around", "initial", "inherit", NULL };
const char *css_letter_spacing[] =				{ "flex-start", "flex-end", "center", "space-between", "space-around", "initial", "inherit", NULL };
const char *css_list_style[] =					{ "disc", "armenian", "circle", "cjk-ideographic", "decimal", "decimal-leading-zero", "georgian", 
												  "hebrew", "hiragana", "hiragana-iroha", "katakana", "katakana-iroha", "lower-alpha",  "lower-greek",
												  "lower-latin", "lower-roman", "none", "square", "upper-alpha", "upper-latin", "upper-roman",			// list-style-type
												  "inside", "outside",																					// list-style-position
												  "url",																								// list-style-image
												  "initial", "inherit", NULL };
const char *css_list_style_image[] =			{ "none", "url", "initial", "inherit", NULL };
const char *css_list_style_position[] =			{ "inside","outside", "initial", "inherit", NULL };
const char *css_list_style_type[] =				{ "disc", "armenian", "circle", "cjk-ideographic", "decimal", "decimal-leading-zero", "georgian",
												  "hebrew", "hiragana", "hiragana-iroha", "katakana", "katakana-iroha", "lower-alpha", "lower-greek",
												  "lower-latin", "lower-roman", "none", "square", "upper-alpha", "upper-latin", "upper-roman",
												  "initial", "inherit", NULL };
const char *css_outline[] =						{ "invert",																								// outline-color
												  "none", "hidden", "dotted", "dashed", "solid", "double", "groove", "ridge", "inset", "outset",		// outline-style
												  "medium", "thin", "thick",																			// outline-width
												  "initial", "inherit", NULL };
const char *css_outline_color[] =				{ "invert", "initial", "inherit", NULL };
const char *css_outline_style[] =				{ "none", "hidden", "dotted", "dashed", "solid", "double", "groove", "ridge", "inset", "outset", "initial", "inherit", NULL };
const char *css_outline_width[] =				{ "medium", "thin", "thick", "initial", "inherit", NULL };
const char *css_overflow[] =					{ "visible", "hidden", "scroll", "auto", "initial", "inherit", NULL };
const char *css_page_break_after[] =			{ "auto", "always", "avoid", "left", "right", "initial", "inherit", NULL };
const char *css_page_break_before[] =			{"auto", "always", "avoid", "left", "right", "initial", "inherit", NULL };
const char *css_page_break_inside[] =			{ "auto", "avoid", "initial", "inherit", NULL };
const char *css_perspective_origine[] =			{ "left", "center", "right", "top", "bottom", "initial", "inherit", NULL };
const char *css_position[] =					{ "static", "absolute", "fixed", "relative", "initial", "inherit", NULL };
const char *css_quotes[] =						{ "none", "'\u2039' '\u203a'", "'\u00ab' '\u00bb'", "'\u2018' '\u2019'", "\"'\" \"'\"", "'\"' '\"'", "'\u201c' '\u201d'", "'\u201c' '\u201e'", "initial", "inherit", NULL };
const char *css_resize[] =						{ "none", "both", "horizontal", "vertical", "initial", "inherit", NULL };
const char *css_table_layout[] =				{ "auto", "fixed", "initial", "inherit", NULL };
const char *css_text_align[] =					{ "left", "right", "center", "justify", "initial", "inherit", NULL };
const char *css_text_align_last[] =				{ "auto", "left", "right", "center", "justify", "start", "end", "initial", "inherit", NULL };
const char *css_text_decoration[] =				{ "none", "underline", "overline", "line-through", "initial", "inherit", NULL };
const char *css_text_justify[] =				{ "auto", "inter-word", "inter-ideograph", "inter-cluster", "distribute", "kashida", "trim", "none", "initial", "inherit", NULL };
const char *css_text_overflow[] =				{ "clip", "ellipsis", "initial", "inherit", NULL };
const char *css_text_transform[] =				{ "none", "capitalize", "uppercase", "lowercase", "initial", "inherit", NULL };
const char *css_transform[] =					{ "none", "matrix", "matrix3d", "translate", "translate3d", "translateX",
												  "translateY", "translateZ", "scale", "scale3d", "scaleX", "scaleY", "scaleZ",
												  "rotate", "rotate3d", "rotateX", "rotateY", "rotateZ", "skew", "skewX", "skewY",
												  "perspective", "initial", "inherit", NULL };
const char *css_transform_origin[] =			{ "left", "center", "right", "top", "bottom", "initial", "inherit", NULL };
const char *css_transform_style[] =				{ "flat", "preserve-3d", "initial", "inherit", NULL };
const char *css_transition[] =					{ "none", "all",															// transition-property
												  "ease", "linear", "ease-in", "ease-out", "ease-in-out", "cubic-bezier",	// transition-timing-function
												  "initial", "inherit", NULL };
const char *css_transition_property[] =			{ "none", "all", "initial", "inherit", NULL };
const char *css_transition_timing_function[] =	{ "ease", "linear", "ease-in", "ease-out", "ease-in-out", "cubic-bezier", "initial", "inherit", NULL };
const char *css_unicode_bidi[] =				{ "normal", "embed", "bidi-override", "initial", "inherit", NULL };
const char *css_vertical_align[] =				{ "baseline", "sub", "super", "top", "text-top", "middle", "bottom", "text-bottom", "initial", "inherit", NULL };
const char *css_visibility[] =					{ "visible", "hidden", "collapse", "initial", "inherit", NULL };
const char *css_white_space[] =					{ "normal", "nowrap", "pre", "pre-line", "pre-wrap", "initial", "inherit", NULL };
const char *css_word_break[] =					{ "normal", "break-all", "keep-all", "initial", "inherit", NULL };
const char *css_word_wrap[] =					{ "normal", "break-word", "initial", "inherit", NULL };

const char **cssRefPtr[] = {
	css_import,						// This is a faked entry
	css_align_content,				// align-content
	css_align_items,				// align-items
	css_align_self,					// align-self
	css_animation,					// animation
	css_generic_no_entries,			// animation-delay
	css_animation_direction,		// animation-direction
	css_generic_no_entries,			// animation-duration
	css_animation_fill_mode,		// animation-fill-mode
	css_animation_iteration_count,	// animation-iteration-count
	css_generic_no_entries,			// animation-name
	css_animation_play_state,		// animation-play-state
	css_animation_timing_function,	// animation-timing-function
	css_backface_visibility,		// backface-visibility
	css_background,					// background,
	css_background_attachment,		// background-attachment,
	css_background_clip,			// background-clip,
	css_color,						// background-color,
	css_background_image,			// background-image,
	css_background_origin,			// background-origin,
	css_background_position,		// background-position,
	css_background_repeat,			// background-repeat,
	css_background_size,			// background-size,
	css_border,						// border
	css_border,						// border-bottom
	css_color,						// border_bottom_color
	css_generic_no_entries,			// border-bottom-left-radius
	css_generic_no_entries,			// border-bottom-right-radius
	css_border_style,				// border-bottom-style
	css_border_width,				// border-bottom-width
	css_border_collapse,			// border-bottom-collapse
	css_color,						// border-color
	css_border_image,				// border-image
	css_generic_no_entries,			// border-image-outset
	css_border_image_repeat,		// border-image-repeat
	css_border_image_slice,			// border-image-slice
	css_generic_none,				// border-image-source
	css_generic_auto,				// border-image-width
	css_border,						// border-left
	css_color,						// border-left-color
	css_border_style,				// border-left-style
	css_border_width,				// border-left-width
	css_generic_no_entries,			// border-radius
	css_border,						// border-right
	css_color,						// border-right-color
	css_border_style,				// border-right-style
	css_border_width,				// border-right-width
	css_generic_no_entries,			// border-spacing
	css_border_style,				// border-style
	css_border,						// border-top
	css_color,						// border-top-color
	css_generic_no_entries,			// border-top-left-radius
	css_generic_no_entries,			// border-top-right-radius
	css_border_style,				// border-top-style
	css_border_width,				// border-top-width
	css_border_width,				// border-width
	css_generic_auto,				// bottom
	css_box_shadow,					// box-shadow
	css_box_sizing,					// box-sizing
	css_caption_side,				// caption-side
	css_clear,						// clear
	css_generic_auto,				// clip
	css_color,						// color
	css_column_count,				// column-count
	css_column_fill,				// column-fill
	css_column_gap,					// column-gap
	css_column_rule,				// column-rule
	css_color,						// column-rule-color			
	css_column_rule_style,			// column-rule-style
	css_column_rule_width,			// column-rule-width
	css_column_span,				// column-span
	css_column,						// column-width
	css_column,						// columns
	css_content,					// content
	css_generic_none,				// counter-increment
	css_generic_none,				// counter-reset
	css_cursor,						// cursor
	css_direction,					// direction
	css_display,					// display
	css_empty_cells,				// empty-cells
	css_flex,						// flex
	css_generic_auto,				// flex-basis
	css_flex_direction,				// flex-direction
	css_flex_flow,					// flex-flow
	css_generic_no_entries,			// flex-grow
	css_generic_no_entries,			// flex-shrink
	css_flex_wrap,					// flex-wrap
	css_float,						// float
	css_font,						// font
	css_font_family,				// font-family
	css_font_size,					// font-size
	css_generic_no_entries,			// font-size-adjust
	css_font_stretch,				// font-stretch
	css_font_style,					// font-style
	css_font_variant,				// font-variant
	css_font_weight,				// font-weight
	css_hanging_ponctuation,		// hanging-ponctuation
	css_generic_auto,				// height
	css_generic_auto,				// icon
	css_justify_content,			// justify-content
	css_generic_auto,				// left
	css_generic_normal,				// letter-spacing
	css_generic_normal,				// line-height
	css_list_style,					// list-style
	css_list_style_image,			// list-style-image
	css_list_style_position,		// list-style-position
	css_list_style_type,			// list-style-type
	css_generic_auto,				// margin
	css_generic_auto,				// margin-bottom
	css_generic_auto,				// margin-left
	css_generic_auto,				// margin-right
	css_generic_auto,				// margin-top
	css_generic_none,				// max-height
	css_generic_none,				// max-width
	css_generic_no_entries,			// min-height
	css_generic_no_entries,			// min-width
	css_generic_auto,				// nav-down
	css_generic_auto,				// nav-index
	css_generic_auto,				// nav-left
	css_generic_auto,				// nav-right
	css_generic_auto,				// nav-up
	css_generic_no_entries,			// opacity
	css_generic_no_entries,			// order
	css_outline,					// outline
	css_outline_color,				// outline-color
	css_generic_no_entries,			// outline-offset
	css_outline_style,				// outline-style
	css_outline_width,				// outline-width
	css_overflow,					// overflow
	css_overflow,					// overflow-x
	css_overflow,					// overflow-y
	css_generic_no_entries,			// padding
	css_generic_no_entries,			// padding-bottom
	css_generic_no_entries,			// padding-left
	css_generic_no_entries,			// padding-right
	css_generic_no_entries,			// padding-top
	css_page_break_after,			// page-break-after
	css_page_break_before,			// page-break-before
	css_page_break_inside,			// page-break-inside
	css_generic_none,				// perspective
	css_perspective_origine,		// perspective-origin
	css_position,					// position
	css_quotes,						// quotes
	css_resize,						// resize
	css_generic_auto,				// right
	css_generic_no_entries,			// tab-size
	css_table_layout,				// table-layout
	css_text_align,					// text-align
	css_text_align_last,			// text-align-last
	css_text_decoration,			// text-decoration
	css_color,						// text-decoration-color
	css_text_decoration,			// text-decoration-line
	css_generic_no_entries,			// text-indent
	css_text_justify,				// text-justify
	css_text_overflow,				// text-overflow
	css_generic_none,				// text-shadow
	css_text_transform,				// text-transform
	css_generic_auto,				// top
	css_transform,					// transform
	css_transform_origin,			// transform-origin
	css_transform_style,			// transform-style
	css_transition,					// transition
	css_generic_no_entries,			// transition-delay
	css_generic_no_entries,			// transition-duration
	css_transition_property,		// transition-property
	css_transition_timing_function,	// transition-timing-function
	css_unicode_bidi,				// unicode-bidi
	css_vertical_align,				// vertical-align
	css_visibility,					// visibility
	css_white_space,				// white-space
	css_generic_auto,				// width
	css_word_break,					// word-break
	css_generic_normal,				// word-spacing
	css_word_wrap,					// word-wrap
	css_generic_auto,				// z-index
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
	inDocument->SetStyle( eSelector,	true,  false, false,	150, 50,	50  );
	inDocument->SetStyle( eKeyword,		true,  false, false,	0,	 255,	128 );
	inDocument->SetStyle( eNormal,      false, false, false,    0,   0,     0   );
	inDocument->SetStyle( eNumber,      false, false, false,    255, 0,     0   );
	inDocument->SetStyle( eValue,       false, false, false,    0,   100,   0   );
	inDocument->SetStyle( eSeparator,   true,  false, false,    150, 50,    50  );
	inDocument->SetStyle( eProperty,    false,  false, false,   50,  150,   50  );
	inDocument->SetStyle( eIdentifier,  false,  false, false,   50,  50,    150  );
}

bool VCSSSyntax::CanHiliteSameWord( sBYTE inStyle )
{
	return inStyle == eSelector || inStyle == eKeyword || inStyle == eNumber || inStyle == eValue || inStyle == eProperty || inStyle == eIdentifier;
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

	VString xstr;
	inDocument->GetLine( inLineIndex, xstr );

	// Tokenize that line
	VCSSLexParser lexer;
	lexer.SetTabSize( 1 );
	lexer.Start( xstr.GetCPointer() );

	// Now that we have the tokens, look for the one containing the offset
	sLONG lastPos = 0;
	CSSToken::eCSSToken token = lexer.GetCurToken();

	while (true) {
		try {
			if (CSSToken::END == lexer.Next( false ))	break;
		} catch (VCSSException err) {
			// We want to ignore it and keep trying
		}

		if (lastPos <= inOffset && lexer.GetCurColumn() - 1 >= inOffset) {


			if ( token == CSSToken::HASH || token == CSSToken::CLASS || token == CSSToken::PSEUDO_CLASS )
			{
				if ( inOffset == lexer.GetCurColumn() )
				{
					outLeftBoundary = lastPos;
					outLength = 1;
				}
				else
				{
					outLeftBoundary = lastPos + 1;
					outLength = lexer.GetCurColumn() - lastPos - 2;
				}
			}
			else
			{
				// We've found the token we care about
				outLeftBoundary = lastPos;
				outLength = lexer.GetCurColumn() - lastPos - 1;	// We subtract one to account for the lexer's column count being 1-based
			}
			return true;
		}
		lastPos = lexer.GetCurColumn() - 1;
		token = lexer.GetCurToken();
	}

	// If we got here, then we've gotten to the end of the line.
	outLeftBoundary = lastPos;
	outLength = xstr.GetLength() - lastPos;

	return true;
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

		switch (token)
		{
			case CSSToken::ATKEYWORD:
			case CSSToken::CLASS:
			case CSSToken::PSEUDO_CLASS:
			case CSSToken::FUNCTION:
			case CSSToken::IDENT:
				style = eName;
				break;
				
			case CSSToken::STRING:
			{
				style = eString;
				// Strings need to have their lengths adjusted because we want to
				// syntax highlight the quotes as well as the string content.
				lengthAdjustment = 2;
			}
				break;
				
			case CSSToken::HASH:
			case CSSToken::PERCENTAGE:
			case CSSToken::DIMENSION:
			case CSSToken::NUMBER:
				style = eNumber;
				break;
				
			case CSSToken::COMMENT:
				style = eComment;
				break;
				
			case CSSToken::IMPORTANT_SYMBOL:
				style = eNormal;
				break;
				
			default:
				break;
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
					// Default value for property identifier
					style = eProperty;

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
					style = eIdentifier;
					// Check if the identifier is a CSS defined name
					for (size_t i = 0; cssRefPtr[ i ] != NULL; i++)
					{
						for (size_t j = 0; cssRefPtr[ i ][ j ] != NULL; j++)
						{
							VString test(cssRefPtr[ i ][ j ]);
							if( tokenValue.EqualTo( test, true ) )
							{
								style = eValue;
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
			sLONG end = Min( xstr.GetLength(), pos + tokenValue.GetLength() );

			if ( style == eSelector )
			{
				inDocument->SetLineStyle( inLineNumber, start, start + 1, eSeparator );
				start++;
			}
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
	sLONG start, length;
	DetermineMorphemeBoundary( inDocument, inLineIndex, inOffset, start, length, inAlternateKey );

	sLONG line = inDocument->GetVisibleLine( inLineIndex );
	inDocument->Select( start, start + length, line, line );

	return true;
}
