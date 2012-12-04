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
	common           = 1,
	startWithoutEnd  = 2,
	endWithoutStart  = 3,
	startWithEnd     = 4,
	insideComment    = 5
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

enum {
	scommentend_col = 1,     
	lcomment_col,        
	datatype_col,		
	projectDefine_col,   
	classDefine_col,     
	AllControls_col,     
	string_col,              
	char_col,            
	allnum_col,          
	specialicon_col,    
	normal_col,
	functionDefine_col,
	variableName_col,
	functionName_col
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


enum{
	PHPfunctioninfo = 2,
	PHPvariableinfo = 3
};



struct PHPStyleInfo
{
	int line;
	int indent;
	int length;
	int style;
};

/*extern const int PHPcolorShadow[15];*/ 
extern const int PHPcolorShadow[15];

extern const int  phpFunIndexSize[26];
extern const char * const phpFunIndexA[103];
extern const char * const phpFunIndexB[45];
extern const char * const phpFunIndexC[55];
extern const char * const phpFunIndexD[407];
extern const char * const phpFunIndexE[50];
extern const char * const phpFunIndexF[212];
extern const char * const phpFunIndexG[147];
extern const char * const phpFunIndexH[527];
extern const char * const phpFunIndexI[793];
extern const char * const phpFunIndexJ[15];
extern const char * const phpFunIndexK[13];
extern const char * const phpFunIndexL[74];
extern const char * const phpFunIndexM[693];
extern const char * const phpFunIndexN[300];
extern const char * const phpFunIndexO[298];
extern const char * const phpFunIndexP[595];
extern const char * const phpFunIndexQ[4];
extern const char * const phpFunIndexR[138];
extern const char * const phpFunIndexS[912];
extern const char * const phpFunIndexT[66];
extern const char * const phpFunIndexU[52];
extern const char * const phpFunIndexV[51];
extern const char * const phpFunIndexW[24];
extern const char * const phpFunIndexX[153];
extern const char * const phpFunIndexY[35];
extern const char * const phpFunIndexZ[42];

extern const char * const *phpFunIndexIndex[26];











