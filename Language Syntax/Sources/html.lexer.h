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

enum { 
		scommentend_col = 1,
		comment_col,
		keyword_col,
		string_col,
		char_col,
		allnum_col,
	   	specialicon_col,
		tagbeginflag,
		normal_col, 
		tag_col,
		separator_col
	 };






enum {
	enterCreateCommon     =  1,
	enterCreateOnlyStart  =  2,
	enterCreateOnlyEnd    =  3,
	enterCreateBoth       =  4,
	commonAddStart        =  12,
	commonDeleteStart     =  13,
	onlyStartDeleteStart  =  21,
	endStartDeleteStart   =  23,
	onlyStartAddEnd       =  24,
	onlyEndDeleteEnd      =  31,
	onlyEndRightAddStart  =  32,
	onlyEndAddStart       =  34,
	bothToCommon          =  41,
	bothDeleteEnd         =  42,
	bothDeleteStart       =  43,
	insideType            =  51,
	insideAddStart        =  52,
	insideAddEnd          =  53,
	insideCommentToBoth   =  54,
	checkAgain            =  99

};

//linetype

enum {
	common           = 1,
	startWithoutEnd  = 2,
	endWithoutStart  = 3,
	startWithEnd     = 4,
	insideComment    = 5, 
	
	ordinarycomment  = 0,
	htmlsytlecomment = 1
};

enum {

	tagbegin = 2,
	tagend  
};




enum {
	vcol_red = 1,
	vcol_orange,
	vcol_yellow,
	vcol_green,
	vcol_dark_green,
	vcol_blue,
	vcol_purple,
	vcol_gray
};


struct htmlLexStyleInfo
{
	int line;
	int indent;
	int length;
	int style;
};

struct htmlLexeme {
	struct htmlLexeme *fNext;
	int fOffset;
	int fLength;
	int fStyle;
#if _DEBUG
	const char *fText;
#endif
};

enum {
	kCompleteComment = 1,
	kCommentOpen,
	kCommentClose,

	kTagOpen,
	kEndTagOpen,
	kTagClose,
	kTagSelfClose,

	kKeyword,
	kNonKeyword,

	kString,
	kNumber,

	kSymbolicChar,	// Things like =, +, -, etc

};

int IsKeyword( const char *inText );
