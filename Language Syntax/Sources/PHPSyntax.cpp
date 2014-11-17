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
#include "php.lexer.h"


extern "C"
{
	struct PHPStyleInfo * startPHPParse(char *strr);
	int getPHPkeywordNum();
	signed long getPHPLexLineType();
	char * getPHPLastInput();
	char * getPHPExtraInfo();

}




/************************************************************************/
/* below two char[] work for the getSuggestion function                 */
/************************************************************************/

const int PHPtagbuffersize = 9000;      //smaller than 9999 is ok
VArrayOf<VString> PHPextrainfo_array;     //to store the extra info(function name   var name)of the line

VArrayOf<VString>  PHPfunctionarray;
VArrayOf<VString>  PHPVariablearray;

VArrayOf<VString>  PHPfunctionarray_copy;
VArrayOf<VString>  PHPVariablearray_copy;

bool               PHPsuggestionVectorIschanged = false;




// static int PHPKeyWordNum = 137;
// 
// static char *PHPkeywordReal[137] = {
// 
// 	"abstract","alert","arguments","Array","blur","boolean","break","byte","callee","caller","captureEvents","case","char","class",
// 	"clearInterval","clearTimeout","close","closed","confirm","const","constructor","continue","Date","debugger","default","defaultStatus",
// 	"delete","do","document","double","else","enum","escape","eval","export","extends","final","finally","find","float","focus","for","frames",
// 	"Function","function","goto","history","home","if","implements","import","in","Infinity","innerHeight",
// 	"InnerWith","Instaceof","int","interface","isFinite","isNaN","java","length","location","locationbar","long","Math","menubar","moveBy",
// 	"moveTo","name","NaN","native","netscape","new","null","Number","Object","open","opener","outerHeight","outerWidth","package",
// 	"Packages","pageXOffset","pageYOffset","parent","parseFloat","parseInt","personlbar","print","private","prompt","protected","prototype",
// 	"public","RegExp","releaseEvents","resizeBy","resizeTo","return","routeEvent","scroll","scrollbars","scrollBy","scrollTo","self","setInterval","setTimeout","Short","static",
// 	"status","statusbar","stop","String","super","switch","synchronized","this","throw","throws","toobar","top","toString","transient",
// 	"try","typeof","unescape","unwatch","valueOf","var","void","watch","while","window","with","FALSE","TRUE"
// 
// };
// 
// static char *PHPkeywordFake[137] = {
// 
// 	"abstract","alert","arguments","array","blur","boolean","break","byte","callee","caller","captureevents","case","char","class",
// 	"clearinterval","cleartimeout","close","closed","confirm","const","constructor","continue","date","debugger","default","defaultstatus",
// 	"delete","do","document","double","else","enum","escape","eval","export","extends","final","finally","find","float","focus","for","frames",
// 	"function","function","goto","history","home","if","implements","import","in","infinity","innerheight",
// 	"innerwith","instaceof","int","interface","isfinite","isnan","java","length","location","locationbar","long","math","menubar","moveby",
// 	"moveto","name","nan","native","netscape","new","null","number","object","open","opener","outerheight","outerwidth","package",
// 	"packages","pagexoffset","pageyoffset","parent","parsefloat","parseint","personlbar","print","private","prompt","protected","prototype",
// 	"public","regexp","releaseevents","resizeby","resizeto","return","routeevent","scroll","scrollbars","scrollby","scrollto","self","setinterval","settimeout","short","static",
// 	"status","statusbar","stop","string","super","switch","synchronized","this","throw","throws","toobar","top","tostring","transient",
// 	"try","typeof","unescape","unwatch","valueof","var","void","watch","while","window","with","false","true"
// 
// };








// ----------------------------------------------------------------------------------------------------------------------------------

VPHPSyntax::VPHPSyntax()
{
	fTabWidth=4;
}


VPHPSyntax::~VPHPSyntax()
{
}

void VPHPSyntax::Init( ICodeEditorDocument* inDocument )
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
}

void VPHPSyntax::Load( ICodeEditorDocument* inDocument, VString& ioContent )
{
}

void VPHPSyntax::Save( ICodeEditorDocument* inDocument, VString& ioContent )
{
}

void VPHPSyntax::SaveLine( ICodeEditorDocument* inDocument, sLONG inLineIndex, VString& ioContent )
{
}

void VPHPSyntax::Close( ICodeEditorDocument* inDocument )
{
}

void VPHPSyntax::SetLine( ICodeEditorDocument* inDocument, sLONG inLineNumber, bool inLoading )
{

	int linekind_comment_cur = getLineKind_Comment(inDocument,inLineNumber);
	int linekind_comment_above = 0;

	if ( inLineNumber > 0 )
	{
		linekind_comment_above = getLineKind_Comment(inDocument,inLineNumber-1);
	}

	bool checkCommet = false;    //use as a flag whether we should check scommet   
	int checkType = 0;              //check in which situation of the scomment

	struct PHPStyleInfo * lexstrptr = 0;
	VString xstr;
	inDocument->GetLine(inLineNumber,xstr);
	const UniChar * unistring  = xstr.GetCPointer();
	char *lexinput;
	sLONG len = xstr.GetLength();
	lexinput = new char[len+1];
	lexinput[len] = '\0';


	for(int i = 0 ; i < len ; i++)     //to change unicode to char
	{
		if (unistring[i] == 9)
			lexinput[i] = ' ';
		else if (unistring[i] > 127)
			lexinput[i] = 'X';
		else
			lexinput[i] = (char)unistring[i];			
	}

	if ( abs(getPHPLexLineType()) > 1000 )
	{
		PHPsuggestionVectorIschanged = true;							//where certain line a new function or var added or modified, 
		//we need to handle the suggestion vector 
	}

	int linekind_comment_former = linekind_comment_cur;
	lexstrptr = startPHPParse(lexinput); 
	setLineKind_Lex(inDocument,inLineNumber,getPHPLexLineType());
	linekind_comment_cur = getLineKind_Comment(inDocument,inLineNumber);


	/************************************************************************/
	/* here to store the new function name or variable name if there is     */
	/************************************************************************/

	if ( PHPextrainfo_array.GetCount() < PHPtagbuffersize )
	{
		if ( abs(getPHPLexLineType()) > 1000 )
		{
			if ( getLineKind_InfoLoc(inDocument,inLineNumber) == 0 )
			{
				PHPextrainfo_array.Push(getPHPExtraInfo());
				setLineKind_InfoLoc(inDocument,inLineNumber,PHPextrainfo_array.GetCount());
			}
			else
			{
				// 				int extrainfoloc = getLineKind_InfoLoc(inDocument,inLineNumber);
				// 				PHPextrainfo_array[extrainfoloc-1] = getPHPExtraInfo();

				int extrainfoloc = getLineKind_InfoLoc(inDocument,inLineNumber);
				VString temptemptemp = "";
				temptemptemp = PHPextrainfo_array[extrainfoloc-1];
				temptemptemp = getPHPExtraInfo();
				PHPextrainfo_array[extrainfoloc-1] = temptemptemp;

			}										
		}
	}
	else														 // reparse the whole file  and reset the content of the PHP extra info vector
	{
				
		int PHPNBline = inDocument->GetNbLines();
		PHPextrainfo_array.Destroy();
		PHPsuggestionVectorIschanged = true; 

		for ( int i = 0; i < PHPNBline; i++ )				     //initialization linekind of each line
		{
			inDocument->SetLineKind(i,0);
		}

		for ( int i = 0; i < PHPNBline; i++)
		{
			parseOneLineByLex(inDocument,i,lexstrptr);
			inDocument->SetLineKind(i,getPHPLexLineType());          //here setline kind is the same as setlinekind_lex
			if ( abs(getPHPLexLineType())/1000 > 0 )     
			{
				PHPextrainfo_array.Push(getPHPExtraInfo());
				setLineKind_InfoLoc(inDocument,i,PHPextrainfo_array.GetCount());
			}
		}
	}

	/************************************************************************/
	/* finished                                                             */
	/************************************************************************/

	inDocument->SetLineStyle(inLineNumber,0,len,0);    //initiate the line


	for (int j = 0; j < getPHPkeywordNum(); j++ )						//set the keywords color	
	{      
		inDocument->SetLineStyle(inLineNumber,lexstrptr->indent,(lexstrptr->indent)+(lexstrptr->length),PHPcolorShadow[lexstrptr->style]);							
		lexstrptr++;
	}	


	if ( linekind_comment_former != linekind_comment_cur)		    //if current linekind is changed, we need to check comment folding
	{  
		checkCommet = true;
		linekind_comment_former = linekind_comment_former*10 + linekind_comment_cur;     //getLineKind_Comment(inDocument,inLineNumber);
		switch(linekind_comment_former)
		{

		case 1:  checkType = enterCreateCommon;break;
		case 2:  checkType = enterCreateOnlyStart;break;
		case 3:  checkType = enterCreateOnlyEnd;break;
		case 4:  checkType = enterCreateBoth;break;
		case 12: checkType = commonAddStart;break;                     
		case 21: checkType = onlyStartDeleteStart;break;
		case 23: checkType = endStartDeleteStart;break;
		case 24: checkType = onlyStartAddEnd;	break;
		case 31: checkType = onlyEndDeleteEnd; break;
		case 32: checkType = onlyEndRightAddStart;break;
		case 34: checkType = onlyEndAddStart;break; 
		case 41: checkType = bothToCommon;break;
		case 42: checkType = bothDeleteEnd;break;
		case 43: checkType = bothDeleteStart;break;
		case 51: checkType = insideType;break;
		case 52: checkType = insideAddStart;break;
		case 53: checkType = insideAddEnd;break;
		case 54: checkType = insideCommentToBoth;break;

		default: checkType = 99;
		}
	}
	else																			  // the line kind comment is unchanged
	{
		if (inLineNumber > 0)
		{
			if (linekind_comment_above == insideComment || linekind_comment_above == startWithoutEnd)
			{	  	
				switch (linekind_comment_cur)
				{
				case common:           inDocument->SetLineStyle(inLineNumber,0,len,0);setLineKind_Comment(inDocument,inLineNumber,insideComment);linekind_comment_cur = insideComment;break;
				case insideComment:    inDocument->SetLineStyle(inLineNumber,0,len,4);break;
				case startWithoutEnd : inDocument->SetLineStyle(inLineNumber,0,findStopLoc(inDocument,inLineNumber),4);
				default:               inDocument->SetLineStyle(inLineNumber,0,findStopLoc(inDocument,inLineNumber),4);
				}
			}	  
		}
	}



	if (checkCommet)				 //need to handle comment 
	{
		int line_i_type = 0;         
		
		int totallinenum = 0;
		totallinenum = inDocument->GetNbLines();




		//----------------------------------------------------------------------------------------------------------------


		if (checkType == enterCreateCommon || checkType == bothToCommon)
		{
			if (inLineNumber > 0 &&  (linekind_comment_above == insideComment || linekind_comment_above == startWithoutEnd ))
			{
				inDocument->SetLineStyle(inLineNumber,0,len,4);	
				setLineKind_Comment(inDocument,inLineNumber,insideComment);
			}
		}


		//-----------------------------------------------------------------------------------------------------------------


		else if (checkType == commonAddStart || checkType == onlyEndRightAddStart  || checkType == bothDeleteEnd || checkType == enterCreateOnlyStart)     //case 12,42:  comment add a start     AAAAA  ->  AAA /* AAA
		{      
			if (checkType != commonAddStart)
			{	
				if (inLineNumber > 0 &&  (linekind_comment_above == insideComment || linekind_comment_above == startWithoutEnd ))
				{	
					inDocument->SetLineStyle(inLineNumber,0,findStopLoc(inDocument,inLineNumber),4);
				}

			}

			for (int i = inLineNumber+1; i < totallinenum; i++)
			{
				line_i_type = getLineKind_Comment(inDocument,i);
				inDocument->GetLine(i,xstr);
				if (line_i_type == endWithoutStart || line_i_type == startWithEnd ||line_i_type == startWithoutEnd)
				{			
					inDocument->SetLineStyle(i,0, findStopLoc(inDocument,i),4);						
					break;
				}
				else
				{
					inDocument->SetLineStyle(i,0,xstr.GetLength(),4);
					if (line_i_type == common)
					{
						setLineKind_Comment(inDocument,i,insideComment);
					}
				}
			}
		}



		//-----------------------------------------------------------------------------------------------------------------


		else if (checkType == onlyStartDeleteStart )   //case 21:       AAA /*  AA  ->  AAAAAA
		{
			if (inLineNumber > 0 && (linekind_comment_above == insideComment || linekind_comment_above ==startWithoutEnd))
			{
				inDocument->SetLineStyle(inLineNumber,0,len,4);
				setLineKind_Comment(inDocument,inLineNumber,insideComment);
			}
			else
			{
				for (int i = inLineNumber+1;i < inDocument->GetNbLines();i++)
				{
					line_i_type = getLineKind_Comment(inDocument,i);
					inDocument->GetLine(i,xstr);					
					const UniChar * unistringtemp = xstr.GetCPointer();
					sLONG len = xstr.GetLength();
					lexinput = new char[len+1];
					lexinput[len] = '\0';
					for(int j = 0 ; j <len ; j++)     //to change unicode to char
					{
						if (unistringtemp[j] == 9)
							lexinput[j] = ' ';
						else if (unistringtemp[j] > 127)
							lexinput[j] = 'X';
						else
							lexinput[j] = (char)unistringtemp[j];			
					}
					lexstrptr = startPHPParse(lexinput);  
					setLineKind_Lex(inDocument,i,getPHPLexLineType());
					inDocument->SetLineStyle(i,0,len,4);  //initate the line
					for (int j = 0; j < getPHPkeywordNum(); j++ )
					{      

						inDocument->SetLineStyle(i,lexstrptr->indent,(lexstrptr->indent)+(lexstrptr->length),PHPcolorShadow[lexstrptr->style]);				
						lexstrptr++;
					}	

					if (line_i_type == startWithEnd || line_i_type == startWithoutEnd || line_i_type == endWithoutStart)
					{

						break;
					}

				}
			}
		}



		//--------------------------------------------------------------------------------------------------------------------------------------------



		else if (checkType == endStartDeleteStart || checkType == onlyStartAddEnd || checkType == insideAddEnd || checkType == enterCreateOnlyEnd || checkType == enterCreateBoth)    //23  24
		{

			if (inLineNumber > 0 && (linekind_comment_above == startWithoutEnd || linekind_comment_above == insideComment))
			{ 
				inDocument->SetLineStyle(inLineNumber,0,findStopLoc(inDocument,inLineNumber),4);
			}

			for (int i = inLineNumber +1; i < totallinenum; i++)
			{
				line_i_type = getLineKind_Comment(inDocument,i);

				inDocument->GetLine(i,xstr);					
				const UniChar * unistringtemp = xstr.GetCPointer();
				sLONG len = xstr.GetLength();
				lexinput = new char[len+1];
				lexinput[len] = '\0';
				for(int j = 0 ; j < len ; j++)     //to change unicode to char
				{
					if (unistringtemp[j] == 9)
						lexinput[j] = ' ';
					else if (unistringtemp[j] > 127)
						lexinput[j] = 'X';
					else
						lexinput[j] = (char)unistringtemp[j];				
				}
				lexstrptr = startPHPParse(lexinput);  
				setLineKind_Lex(inDocument,i,getPHPLexLineType());
				inDocument->SetLineStyle(i,0,len,0);  //initate the line
				for (int j = 0; j < getPHPkeywordNum(); j++ )
				{      
					inDocument->SetLineStyle(i,lexstrptr->indent,(lexstrptr->indent)+(lexstrptr->length),PHPcolorShadow[lexstrptr->style]);						
					lexstrptr++;
				}	

				if (line_i_type == startWithEnd || line_i_type == startWithoutEnd || line_i_type == endWithoutStart)
				{
					break;
				}			 
			}		
		}



		//---------------------------------------------------------------------------------------------------------------------------------------------


		else if (checkType == onlyEndDeleteEnd)      //31
		{
			if (inLineNumber > 0 &&  (linekind_comment_above == insideComment || linekind_comment_above == startWithoutEnd ))
			{
				for (int i =inLineNumber; i < totallinenum ;i++)
				{
					line_i_type = getLineKind_Comment(inDocument,i);
					inDocument->GetLine(i,xstr);
					if (line_i_type == startWithEnd || line_i_type == endWithoutStart)
					{							
						inDocument->SetLineStyle(i,0,findStopLoc(inDocument,i),4);
						break;
					}
					else
					{
						inDocument->SetLineStyle(i,0,xstr.GetLength(),4);
						if (line_i_type == common)
						{
							setLineKind_Comment(inDocument,i,insideComment);
						}
					}
				}
			}
		}



		//---------------------------------------------------------------------------------------------------------------------------------------------



		else if (checkType == insideType || checkType == insideAddStart || checkType == bothDeleteStart)     //51
		{
			if (checkType == insideType)
			{
				inDocument->SetLineStyle(inLineNumber,0,len,4);
			}
			else
			{
				if (inLineNumber > 0 &&  (linekind_comment_above == insideComment || linekind_comment_above == startWithoutEnd ))
				{		
					inDocument->SetLineStyle(inLineNumber,0,findStopLoc(inDocument,inLineNumber),4);
				}
			}

			if (checkType == insideType)
			{	
				setLineKind_Comment(inDocument,inLineNumber,insideComment);
			}

		}

		//---------------------------------------------------------------------------------------------------------------------------------------------

		delete lexinput;	
	}
}




sLONG VPHPSyntax::GetIndentWidth()
{
	return 0;
}





bool VPHPSyntax::CheckFolding(ICodeEditorDocument* inDocument, sLONG inLineNumber)
{
	if (  (abs(inDocument->GetLineKind(inLineNumber))%1000) > 9 )
	{
		return true;
	}
	else
	{
		return false;
	}
}





void VPHPSyntax::ComputeFolding( ICodeEditorDocument* inDocument )
{

	/************************************************************************/
	/*   intialize                                                          */
	/************************************************************************/

	for (int atline_i = 0 ;atline_i < inDocument->GetNbLines(); atline_i++)
	{
		inDocument->SetFoldable(atline_i,false);
	}

	/************************************************************************/
	/* recalculate the num of bracket  if necessary, and reset line kind    */
	/************************************************************************/

	for (int atline_i = 1;atline_i < inDocument->GetNbLines(); atline_i++)
	{

		int line_i_kind = inDocument->GetLineKind(atline_i);
		int line_i_bracket = (abs(line_i_kind)%1000)/10;	
		int line_i_commentkind = (abs(line_i_kind))%10;
		int line_iabove_commentkind = (abs(inDocument->GetLineKind(atline_i-1)))%10;

		if (( line_iabove_commentkind == insideComment || line_iabove_commentkind==startWithoutEnd ) && line_i_bracket > 0  &&  line_i_commentkind > 1)
		{         
			if (line_i_commentkind == insideComment)
			{
				setLineKind_Lex(inDocument,atline_i,line_i_commentkind);
			} 
			else
			{
				int stopat = findStopLoc(inDocument,atline_i);			
				VString xstr;
				inDocument->GetLine(atline_i,xstr);
				const UniChar * unistringtemp  = xstr.GetCPointer();

				for (int i = 0;i < stopat ;i++)
				{
					if (unistringtemp[i] == '{')
					{  
						line_i_kind = line_i_kind - 10;
					}
					if (unistringtemp[i] == '}' )
					{
						line_i_kind = line_i_kind + 10;
					}
				}
				inDocument->SetLineKind(atline_i,line_i_kind);
			}			
		}	
	}

	/************************************************************************/
	/* below is to check the folding                                        */
	/************************************************************************/

	VArrayOf<int> bracketStack;
	for (int atline_i = 0;atline_i < inDocument->GetNbLines(); atline_i++ )
	{
		int line_i_kind = inDocument->GetLineKind(atline_i);
		int line_i_bracket = (abs(line_i_kind)%100)/10;

		if(abs(line_i_kind) > 10)      
		{ 

			if (line_i_kind > 0)
			{
				for(int lbnum = 1;lbnum <= line_i_bracket ;lbnum++)       //lbnum means the num of leftbracket
				{
					bracketStack.Push(atline_i);
				}
			} 
			else
			{

				for (int rbnum = 1; rbnum <= line_i_bracket;rbnum++)
				{
					if (bracketStack.IsEmpty())
					{
						break;
					}
					int lbline = bracketStack.Pop();
					inDocument->SetFoldable(lbline,true);
					inDocument->SetNbFoldedLines(lbline,atline_i-lbline-1);
				}
			}
		}
	}
}




// the four functions below done later......
bool VPHPSyntax::CheckOutline(ICodeEditorDocument* inDocument, sLONG inLineNumber)
{
	return false;
}

void VPHPSyntax::ComputeOutline( ICodeEditorDocument* inDocument )
{

}

void VPHPSyntax::TokenizeLine( ICodeEditorDocument* inDocument, sLONG inLineIndex, std::vector<XBOX::VString>& ioLines )
{

}



/*

void VPHPSyntax::TokenizeOneLine( ICodeEditorDocument* inDocument )
{

	sLONG start, end, firstLine, endLine;
	inDocument->GetSelection(start,end,firstLine,endLine);

	if ( testAssert( start == 0 && end == 0 && firstLine == endLine && firstLine > 0 ) )
	{
		int indentlengthtab = 0;
		int indentlengthsp =0;

		int extraIndent = 0;
		int startlineabtype = 0;
		sLONG setLineIndex = inDocument->GetLineIndex( endLine );
		sLONG startLineIndex = setLineIndex - 1;
		VString str;

		inDocument->GetLine(startLineIndex,str);

		startlineabtype = inDocument->GetLineKind(startLineIndex);
		startlineabtype = (startlineabtype%1000)/10;

		if ( startlineabtype  > 0 )
		{
			if ( inDocument->GetLineKind(setLineIndex) > 0 )
			{
				extraIndent = 1;
			}			
		}

		for (int i = 0;i < str.GetLength(); i++ )
		{

			if (str[i] ==  9 )
			{
				indentlengthtab++;
				indentlengthsp = (indentlengthsp/4)*4;         //important!!  to eliminate unnecessary space
			}
			else if ( str[i] == 32 )
			{
				indentlengthsp++;
			}
			else
			{
				break;
			}
		}

		indentlengthtab = indentlengthtab + extraIndent;

		for ( int j = 0; j < indentlengthtab ; j++ )
		{
			inDocument->InsertText( VString( (UniChar) 9 ) );
		}

		for ( int i = 1; i < indentlengthsp; i++ )
		{
			inDocument->InsertText( VString( (UniChar) 32 ));
		}

	}




	int PHPNBLines = inDocument->GetNbLines();
	int linekind_i = 0;
	int extrainfotype = 0;
	if (PHPsuggestionVectorIschanged)
	{
		PHPfunctionarray.Destroy();
		PHPVariablearray.Destroy();

		for ( int atlinei = 0; atlinei < PHPNBLines; atlinei++ )
		{
			linekind_i = inDocument->GetLineKind(atlinei);
			if ( abs(linekind_i) > 1000 )
			{
				extrainfotype = (abs(linekind_i)%10000)/1000;
				VString infoatlinei = PHPextrainfo_array.GetNth(getLineKind_InfoLoc(inDocument,atlinei));

				switch( extrainfotype )
				{  
				case PHPfunctioninfo: PHPfunctionarray.Push(infoatlinei);
					break; 
				case PHPvariableinfo: PHPVariablearray.Push(infoatlinei);
					break;
				default: break;
				}
			}
		}
		//make the two array in order  according to alphabetical order ( cap senseless )

		PHPQuickSort(PHPfunctionarray,0,PHPfunctionarray.GetCount()-1);
		PHPQuickSort(PHPVariablearray,0,PHPVariablearray.GetCount()-1);

	}

	PHPsuggestionVectorIschanged = false;




}
*/



void VPHPSyntax::GetTip( ICodeEditorDocument* inDocument, sLONG inLine, sLONG inPos, VString& outText, Boolean& outRtfText )
{

}





void VPHPSyntax::GetSuggestions( ICodeEditorDocument* inDocument, sLONG inLineIndex, sLONG inPos, ITipInfoArray *outSuggestions, sLONG& outStartOffset, bool inAll )
{

	struct PHPStyleInfo * lexstrptr = 0;               //reparse the line at line parseLocalVarAti to deal with the localvar
	VString xstr;
	inDocument->GetLine(inLineIndex,xstr);
	xstr.Truncate( inPos );
	const UniChar * unistring  = xstr.GetCPointer();
	char *lexinput;
	
	outStartOffset = inPos;

	sLONG len = xstr.GetLength();
	lexinput = new char[len+1];
	lexinput[len] = '\0';

	for(int i = 0 ; i < len ; i++)     //to change unicode to char
	{
		if (unistring[i] == 9)
			lexinput[i] = ' ';
		else if (unistring[i] > 127)
			lexinput[i] = 'X';
		else
			lexinput[i] = (char)unistring[i];			
	}
	lexstrptr = startPHPParse(lexinput); 		

	char * cPHPLastInput = getPHPLastInput();
	VString vPHPLastInput = getPHPLastInput();

// 	int extrainfotype = 0;
// 	int PHPNBLines = inDocument->GetNbLines();
// 	int linekind_i = 0;
// 	int linekind_extrainfo_i = 0;



	/************************************************************************/
	/*below: add function name to the  iosuggestion                         */
	/************************************************************************/
	//use binary search
	binarySearch( inDocument, PHPfunctionarray,outSuggestions,vPHPLastInput,3,vcol_red );    //3 means, we  get at most three suggestions from this group


	/************************************************************************/
	/*below: add variable name to the iosuggestion                          */
	/************************************************************************/
	//use binary search

	binarySearch( inDocument, PHPVariablearray,outSuggestions,vPHPLastInput,3,vcol_purple );
	removeDuplicate(PHPVariablearray);	                                    //for javascript,  we only need to remove the duplication from the 



	/************************************************************************/
	/*below: get suggestion from the phpfunindex                            */
	/************************************************************************/


	//attributes for binary search
	int bslocbeg = 0;								 //binary search location begin
	int bslocend = 0;
	int bslocmid = 0;
	int bsresult=0;
	int inputlength = strlen(cPHPLastInput);
	
	char inputbeginwith = cPHPLastInput[0];

	if ( inputbeginwith <=90 && inputbeginwith>= 65)
	{
		inputbeginwith = inputbeginwith + 32;
	}
	
	
	if ( inputbeginwith <= 122 && inputbeginwith >= 97 )
	{
	
		const char * const * phpfunindexfake = phpFunIndexIndex[inputbeginwith - 97];	
		int PHPKeyWordNum = phpFunIndexSize[inputbeginwith - 97];
		bslocend = PHPKeyWordNum;


		if (NULL != outSuggestions && strlen(cPHPLastInput)>0)
		{
			

			for(int i = 0; i < 10; i++)
			{

				bslocmid = (bslocbeg+bslocend)/2;

				if(strcmp(cPHPLastInput,phpfunindexfake[bslocmid]) > 0)
				{
					bslocbeg = bslocmid;
				}
				else
				{
					bslocend = bslocmid; 
				}

				if (bslocend-bslocbeg<=1)
				{
					bsresult = bslocbeg;
					break;
				}

			}

			if (bsresult != 0)           // in case it is the first one
			{
				bsresult++;
			}

			int PHPSuggestionCount = 1;

			outStartOffset -= strlen(cPHPLastInput);

			for (int i = bsresult ; i < PHPKeyWordNum && PHPSuggestionCount < 3; i++,PHPSuggestionCount++)
			{
				if (strlen(cPHPLastInput) > strlen(phpfunindexfake[i]))
				{
					break;
				}


				if (VSTRNICMP(cPHPLastInput,phpfunindexfake[i],inputlength) == 0)
				{
					outSuggestions->AddTip( new VCodeEditorTipInfo( inDocument, phpfunindexfake[ i ], vcol_dark_green ) );
				}
				else
				{
					break;
				}

			}

		}
		


	}

			





	/************************************************************************/
	/* three types of get suggestion function finished                      */
	/************************************************************************/


	delete lexinput;

}





int VPHPSyntax::findStopLoc(ICodeEditorDocument* inDocument,int inLineNumber)
{

	VString xstr;
	inDocument->GetLine(inLineNumber,xstr);
	const UniChar * unistringtemp  = xstr.GetCPointer();
	int starloc = 0;


	for (int i = 0;i < xstr.GetLength()-1;i++)
	{
		if (unistringtemp[i] == '*' && unistringtemp[i+1] == '/')
		{  
			starloc = i+1;
			break;
		}
		if (unistringtemp[i] == '/' && unistringtemp[i+1] == '*')
		{
			starloc = i +1;
			break;
		}
	}

	return starloc;
}





void VPHPSyntax::binarySearch( ICodeEditorDocument *inDocument, VArrayOf<VString> &source, ITipInfoArray *outSuggestions, VString userinput, int maxsuggestionNum, int color )
{

	int bslocbeg = 0;								 //binary search location begin
	int sourcesize = source.GetCount();
	int bslocend = sourcesize-1;
	int bslocmid = 0;
	int bsresult=0;
	int inputlength = userinput.GetLength();


	VString justfortest = "";
	for (int i = 0; i < sourcesize;i++ )
	{
		justfortest = source[i];
	}

	if ( inputlength>0 && sourcesize > 0 )
	{
		for(int i = 0; i < 14; i++)						 //   2 power by 14 = 16384 >  9999
		{
			bslocmid = (bslocbeg+bslocend)/2;

			if( userinput > source[bslocmid]) 
			{
				bslocbeg = bslocmid;
			}
			else
			{
				bslocend = bslocmid; 
			}

			if (bslocend-bslocbeg<=1)
			{
				bsresult = bslocbeg;
				break;
			}
		}

		int PHPSuggestionCount = 0;

		if ( bsresult!=0 || ( bsresult==0 && sourcesize>0 &&userinput > source[0] ) )
		{
			bsresult++;
		}

		VString currentstr = "";
		for (int i = bsresult ; i < sourcesize && PHPSuggestionCount < maxsuggestionNum; i++)
		{
			currentstr = source.GetNth(i+1);
			if ( currentstr.BeginsWith( userinput ) )
			{	
				outSuggestions->AddTip( new VCodeEditorTipInfo( inDocument, currentstr, color ) );
				PHPSuggestionCount++;
			}
			else
			{
				break;
			}
		}
	}
}



/************************************************************************/
/* 

explanation of meaning of each number of the line kind

signed long                2,147,483,648
8--->   comment linekind :  startwithoutend  startwithend...... to deal with the comment  
64---->   angle bracket num : to store the  bracket num in a certain line   if >0   means have more { ; 0: no bracket or >=<
3------->   to store whether this line define a variable or function		1: a new variable   2: a new function     		
47,48 ------->   to store the string that connect to this line, the string can hold the value of variable name or functionname 
3 648--->   defined by lex.c file       	         
47,48 ------->   defined by javascriptsyntax.cpp file						
2,1------------->   reserved  for future use
*/
/************************************************************************/





int VPHPSyntax::getLineKind_Lex(ICodeEditorDocument* inDocument,int inLineNumber)
{
	signed long linekindtemp = inDocument->GetLineKind(inLineNumber);
	if (linekindtemp > 10000)
	{
		linekindtemp = linekindtemp%10000;
	}
	return  linekindtemp;
}




void VPHPSyntax::setLineKind_Lex(ICodeEditorDocument * inDocument, int inLineNumber,int lexlinekind )
{
	signed long linekindtemp = inDocument->GetLineKind(inLineNumber);
	linekindtemp = abs(linekindtemp);
	if ( linekindtemp>10000 )
	{
		if (lexlinekind > 0)
		{
			linekindtemp = linekindtemp - linekindtemp%10000 + lexlinekind;
		}
		else
		{
			linekindtemp = linekindtemp - lexlinekind%10000 -lexlinekind;
			linekindtemp = 0 - linekindtemp;
		}
	}
	else
	{
		linekindtemp = lexlinekind;
	}
	inDocument->SetLineKind(inLineNumber,linekindtemp);
}




int VPHPSyntax::getLineKind_Comment(ICodeEditorDocument* inDocument,int inLineNumber)
{
	return (abs(inDocument->GetLineKind(inLineNumber)))%10;
}



void VPHPSyntax::setLineKind_Comment(ICodeEditorDocument *inDocument, int inLineNumber, int comment_kind)
{
	int full_line_i_kind = inDocument->GetLineKind(inLineNumber);

	if (abs(full_line_i_kind > 10))
	{
		if (full_line_i_kind >= 0)
		{
			full_line_i_kind = full_line_i_kind - full_line_i_kind%10;
			full_line_i_kind = full_line_i_kind + comment_kind;
		}
		else
		{
			full_line_i_kind = full_line_i_kind - full_line_i_kind%10;
			full_line_i_kind = full_line_i_kind - comment_kind;	
		}
	}
	else
	{
		full_line_i_kind = comment_kind;
	}
	inDocument->SetLineKind(inLineNumber,full_line_i_kind);
}





void VPHPSyntax::setLineKind_InfoLoc(ICodeEditorDocument * inDocument, int inLineNumber, int extraInfoLoc)
{
	int linekindtemp = inDocument->GetLineKind(inLineNumber);

	if ( abs(linekindtemp) > 10000 )
	{
		if ( linekindtemp > 0 )
		{
			linekindtemp = linekindtemp - linekindtemp%100000000 + extraInfoLoc*10000 + linekindtemp%10000;
		}
		else
		{
			linekindtemp = abs( linekindtemp );
			linekindtemp = linekindtemp - linekindtemp%100000000 + extraInfoLoc*10000+ linekindtemp%10000;
			linekindtemp = -linekindtemp;
		}
	}
	else
	{
		if (linekindtemp > 0)
		{
			linekindtemp = linekindtemp + 10000*extraInfoLoc;
		}
		else
		{
			linekindtemp = linekindtemp - 10000*extraInfoLoc;
		}
	}

	inDocument->SetLineKind(inLineNumber,linekindtemp);

}




int VPHPSyntax::getLineKind_InfoLoc(ICodeEditorDocument* inDocument, int inLineNumber)
{
	int linekindtemp = inDocument->GetLineKind(inLineNumber);
	linekindtemp = abs( linekindtemp );
	linekindtemp = (linekindtemp%100000000)/10000;
	return linekindtemp;
}




void VPHPSyntax::parseOneLineByLex(ICodeEditorDocument* inDocument,int inLineNumber,struct PHPStyleInfo * inputstruct)
{

	VString xstr;
	inDocument->GetLine(inLineNumber,xstr);
	const UniChar * unistring  = xstr.GetCPointer();
	char *lexinput;
	sLONG len = xstr.GetLength();
	lexinput = new char[len+1];
	lexinput[len] = '\0';

	for(int i = 0 ; i < len ; i++)     //to change unicode to char
	{
		if ( unistring[i] == 9 )
		{
			lexinput[i] = ' ';
		}
		else if (unistring[i] > 127)
		{	
			lexinput[i] = 'X';
		}
		else
		{	
			lexinput[i] = (char)unistring[i];
		}			
	}
	inputstruct = startPHPParse(lexinput);  
	delete lexinput;
}




void VPHPSyntax::removeDuplicate(VArrayOf<VString> &source)
{
	int sourcesize = source.GetCount();

	VString formerstr = "";
	VString currentstr = "";

	int current = 1;
	int former = 0;

	if ( sourcesize > 0 )
	{
		for ( ; current < sourcesize ; )
		{
			formerstr = source[current-1];
			currentstr = source[current];

			if( formerstr == currentstr )     // the == of vstring is capital senseless , but we need senseable 
			{
				int curstrlen = formerstr.GetLength();
				for (int i = 0; i < curstrlen;i++)
				{
					if ( i == curstrlen-1 )
					{
						source.DeleteNth(current+1);
						sourcesize--;
					}

					if ( formerstr[i] != currentstr[i]  )
					{
						former++;
						current++;
						break;
					}
				}						
			}
			else
			{
				former++;
				current++;
			}
		}
	}	
}





void VPHPSyntax::PHPQuickSort( VArrayOf<VString> & thearray, int low, int high )       //to make "thearray" in order, "low" and "high" mean from where to where
{
	int i,j;
	VString pivot = "";

	if(low<high)
	{
		pivot = thearray[low];
		i = low;
		j = high;

		while(i<j)
		{
			while(i<j && thearray[j]>pivot)
				j--;
			if(i<j)
			{
				thearray[i++] = thearray[j];
			}
			while (i<j && thearray[i]<pivot)
				i++;
			if(i<j)
			{
				thearray[j--] = thearray[i];
			}
		}

		thearray[i] = pivot;
		PHPQuickSort(thearray,low,i-1);
		PHPQuickSort(thearray,i+1,high);
	}
}




void VPHPSyntax::InsertChar( ICodeEditorDocument* inDocument, UniChar inUnichar, sLONG inLineIndex, sLONG inPosition, ITipInfoArray *outSuggestions, sLONG& outStartOffset )
{
	if ( inUnichar == '\"' )
	{
		inDocument->InsertText( VString( (UniChar) 34 ) ); //insert spaces       34 is "
		sLONG startloc = 0;
		sLONG endloc = 0;
		sLONG atline = 0;	

		inDocument->GetSelection(startloc,endloc,atline,atline);
		if ( startloc>0 )
		{
			inDocument->Select(startloc-1,endloc-1,atline,atline);
		}
	}
	else if ( inUnichar == '\'' )
	{
		inDocument->InsertText( VString( (UniChar) 39 ) ); //insert spaces       39 is '
		sLONG startloc = 0;
		sLONG endloc = 0;
		sLONG atline = 0;	

		inDocument->GetSelection(startloc,endloc,atline,atline);
		if ( startloc>0 )
		{
			inDocument->Select(startloc-1,endloc-1,atline,atline);
		}
	}
	else if ( inUnichar == '{' )
	{
		inDocument->InsertText( VString( (UniChar) 125 ) ); //insert spaces       125 is }
		sLONG startloc = 0;
		sLONG endloc = 0;
		sLONG atline = 0;	

		inDocument->GetSelection(startloc,endloc,atline,atline);
		if ( startloc>0 )
		{
			inDocument->Select(startloc-1,endloc-1,atline,atline);
		}
	}
}




void VPHPSyntax::UpdateBreakPointsLineNumbers( ICodeEditorDocument* inDocument, const std::vector<sWORD>& inBreakIDs, const std::vector<sLONG>& inLineNumbers )
{
}

void VPHPSyntax::AddBreakPoint( ICodeEditorDocument* inDocument, sLONG inLineNumber, sWORD& outID, bool& outDisabled )
{
	outID = 0;
}

bool VPHPSyntax::EditBreakPoint( ICodeEditorDocument* inDocument, sWORD inBreakID, bool& outDisabled )
{
	return false;
}

void VPHPSyntax::RemoveBreakPoint( ICodeEditorDocument* inDocument, sLONG inLineNumber )
{
}

bool VPHPSyntax::GetBreakPoints( ICodeEditorDocument* inDocument, std::vector<sLONG>& outLineNumbers, std::vector<sWORD>& outIDs, std::vector<bool>& outDisabled )
{
	return false;
}

void VPHPSyntax::UpdateCompilerErrors( ICodeEditorDocument* inDocument )
{
}

void VPHPSyntax::GotoNextMethodError( ICodeEditorDocument* inDocument, bool inNext )
{
}

void VPHPSyntax::SwapAssignments( ICodeEditorDocument* inDocument, VString& ioString )
{
}

bool VPHPSyntax::IsComment( ICodeEditorDocument* inDocument, const VString& inString )
{
	return false;
}

void VPHPSyntax::SwapComment( ICodeEditorDocument* inDocument, VString& ioString, bool inComment )
{
}

int VPHPSyntax::VSTRNCMP(const char *s1, const char *s2,int maxlen)
{
	char ch1,ch2; 
	for(int i = 0;i < maxlen; i++) 
	{ 
		ch1 = s1[i]; 
		ch2 = s2[i];

		if(ch1 != ch2) 
		{ 
			if(ch1 > ch2) return 1; 
			else return -1; 
		} 
		else 
		{ 
			if(ch1 == 0) return 0; 
			else continue; 
		} 
	} 
	return (0); 
}





int VPHPSyntax::VSTRNICMP(const char *s1, const char *s2, int maxlen)
{
	char ch1,ch2; 
	for(int i = 0;i < maxlen; i++) 
	{ 
		ch1 = s1[i]; 

		if(ch1 >= 'a' && ch1 <= 'z') 
			ch1 -= 'a' - 'A'; 
		ch2 = s2[i] ;
		if(ch2 >= 'a' && ch2 <= 'z') 
			ch2 -= 'a' - 'A'; 

		if(ch1 != ch2) 
		{ 
			if(ch1 > ch2) return 1; 
			else return -1; 
		} 
		else 
		{ 
			if(ch1 == 0) return 0; 
			else continue; 
		} 
	} 
	return (0); 
}



void VPHPSyntax::CallHelp( ICodeEditorDocument* inDocument )
{
}


bool VPHPSyntax::IsMatchingCharacter( ICodeEditorDocument* inDocument, sLONG inLineIndex, sLONG inOffset, UniChar inChar )
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



bool VPHPSyntax::ShouldValidateTipWindow( UniChar inChar )
{
	return inChar == '(' || inChar == ':' || inChar == '=' || inChar == ';' || 
		   inChar == '<' || inChar == '>' || inChar == '+' || inChar == '-' || 
		   inChar == '{' || inChar == '/' || inChar == '#' || inChar == '[';
}


bool VPHPSyntax::DoubleClick( ICodeEditorDocument* inDocument, sLONG inLineIndex, sLONG inOffset, bool inAlternateKey )
{
	return false;
}
