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
#include "xml.lexer.h"
#include "XMLSyntax.h"

const int xmltagbuffersize = 9000;  //  smaller than 9999 is ok


const char *xmlfourdkeywords[9] = {"4DVAR","4DHTMLVAR","4DSCRIPT","4DINCLUDE","4DIF","4DLOOP","4DENDIF","4DENDLOOP","4DELSE"};

 extern "C"
{
 	struct xmlLexStyleInfo * startxmlParse(char *);
 	int getxmlKeywordNum();
 	signed long getxmlLinetype();
	char * getxmlTagName();
	
};


VArrayOf<VString> allxmltags; 
int xmlfoldingcheckflag = 0;
static int xmlcolorShadow[11] = {0,5,5,1,5,5,0,8,8,8,6};

VXMLSyntax::VXMLSyntax()
{
	fAutoInsertBlock = false;
	fAutoInsertClosingChar = false;
	fAutoInsertTabs = false;
	fInsertSpaces = false;
	fTabWidth = 4;
}


VXMLSyntax::~VXMLSyntax()
{
}


void VXMLSyntax::Init( ICodeEditorDocument* inDocument )
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

void VXMLSyntax::Load( ICodeEditorDocument* inDocument, VString& ioContent )
{
}

void VXMLSyntax::Save( ICodeEditorDocument* inDocument, VString& ioContent )
{
}


void VXMLSyntax::SaveLine( ICodeEditorDocument* inDocument, sLONG inLineIndex, VString& ioContent )
{
}


void VXMLSyntax::Close( ICodeEditorDocument* inDocument )
{
}














void VXMLSyntax::SetLine( ICodeEditorDocument* inDocument, sLONG inLineNumber, bool inLoading )
{

	int linekind_comment_aboveline      = 0;
	int linekind_comment_curline_cur    = 0;
	int linekind_comment_curline_former = 0;


	bool NeedTocheckCommet  = false;	//use as a flag whether we should check scommet 
	int checkType	        = 0;                  //check in which situation of the scomment



	if ( inLineNumber > 0 )
	{
		linekind_comment_aboveline = getFullLindKind_Comment(inDocument,inLineNumber-1);
	}
	linekind_comment_curline_former = getFullLindKind_Comment(inDocument,inLineNumber);





	//begin parsing
	struct xmlLexStyleInfo * lexstrptr = 0;
	VString xstr;
	inDocument->GetLine(inLineNumber,xstr);
	const UniChar * unistring  = xstr.GetCPointer();
	char *lexinput;
	sLONG len = xstr.GetLength();
	lexinput = new char[len+1];
	lexinput[len] = '\0';
	for(int i = 0 ; i < len; i++)     //to change unicode to char
	{
		if (unistring[i] == 9)
			lexinput[i] = ' ';
		else if (unistring[i] > 127)
			lexinput[i] = 'X';
		else
			lexinput[i] = (char)unistring[i];			
	}
	lexstrptr = startxmlParse(lexinput); 

	linekind_comment_curline_cur = getxmlLinetype();
	setLineKind_Lex(inDocument,inLineNumber,linekind_comment_curline_cur);
	linekind_comment_curline_cur = linekind_comment_curline_cur%100;






	if ( (inDocument->GetLineKind(inLineNumber)%1000)/100 > common )       //if the line tag begin or tagend
	{

		if ( getLindKind_Tagnameloc(inDocument,inLineNumber) == 0 )
		{
			allxmltags.Push(getxmlTagName());

			setLindKind_Tagnameloc(inDocument,inLineNumber,allxmltags.GetCount());	
		}
		else
		{
			int line_i_tag_loc = getLindKind_Tagnameloc(inDocument,inLineNumber);		
			allxmltags[line_i_tag_loc-1] = getxmlTagName();
		}

	}




	inDocument->SetLineStyle(inLineNumber,0,len,0);		//initiate the line
	for (int j = 0; j < getxmlKeywordNum(); j++ )						//set the keywords color	
	{      
		inDocument->SetLineStyle(inLineNumber,lexstrptr->indent,Min<sLONG>( len, (lexstrptr->indent)+(lexstrptr->length) ),xmlcolorShadow[lexstrptr->style]);							
		lexstrptr++;
	}	




	/************************************************************************/
	/* the following code to                                                                     */
	/************************************************************************/



	for (int commenttypei = 0; commenttypei <= htmlsytlecomment ; commenttypei++  )
	{	

		int linekind_comment_specific_cur    = 0;
		int linekind_comment_specific_fomer  = 0;

		int	linkkind_comment_specific_above  = 0; 


		if ( commenttypei == ordinarycomment )
		{
			linekind_comment_specific_cur   = linekind_comment_curline_cur%10;                     //both ..cur and ..former >= 0
			linekind_comment_specific_fomer = linekind_comment_curline_former%10;
			linkkind_comment_specific_above = linekind_comment_aboveline%10;
		}
		else if ( commenttypei == htmlsytlecomment )
		{
			linekind_comment_specific_cur   = linekind_comment_curline_cur/10;                     //both ..cur and ..former >= 0
			linekind_comment_specific_fomer = linekind_comment_curline_former/10;
			linkkind_comment_specific_above = linekind_comment_aboveline/10;
		}



		if ( linekind_comment_curline_former != linekind_comment_curline_cur)		    //if current linekind is changed, we need to check comment folding
		{  

			NeedTocheckCommet = true;

			linekind_comment_specific_fomer = linekind_comment_specific_fomer*10 + linekind_comment_specific_cur;     //getLineKind_Comment(inDocument,inLineNumber);		

			switch(linekind_comment_specific_fomer)
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
				if ( linkkind_comment_specific_above == insideComment || linkkind_comment_specific_above == startWithoutEnd )
				{	  	
					switch (linekind_comment_specific_cur)
					{
					case common:           inDocument->SetLineStyle(inLineNumber,0,len,0);setLineKind_Comment(inDocument,inLineNumber,insideComment,commenttypei);linekind_comment_specific_cur = insideComment;break;
					case insideComment:    inDocument->SetLineStyle(inLineNumber,0,len,4);break;
					case startWithoutEnd : inDocument->SetLineStyle(inLineNumber,0,findStopLoc(inDocument,inLineNumber,commenttypei),4);
					default:               inDocument->SetLineStyle(inLineNumber,0,findStopLoc(inDocument,inLineNumber,commenttypei),4);
					}
				}	  
			}
		}



		if (NeedTocheckCommet)				 //need to handle comment 
		{
			int line_i_type = 0;         
			int totallinenum = 0;

			totallinenum = inDocument->GetNbLines();


			//----------------------------------------------------------------------------------------------------------------


			if (checkType == enterCreateCommon || checkType == bothToCommon)
			{
				if (inLineNumber > 0 &&  (linkkind_comment_specific_above == insideComment || linkkind_comment_specific_above == startWithoutEnd ))
				{
					inDocument->SetLineStyle(inLineNumber,0,len,4);	
					setLineKind_Comment(inDocument,inLineNumber,insideComment,commenttypei);
				}
			}


			//-----------------------------------------------------------------------------------------------------------------


			else if (checkType == commonAddStart || checkType == onlyEndRightAddStart  || checkType == bothDeleteEnd || checkType == enterCreateOnlyStart)     //case 12,42:  comment add a start     AAAAA  ->  AAA /* AAA
			{      
				if (checkType != commonAddStart)
				{	
					if (inLineNumber > 0 &&  (linkkind_comment_specific_above == insideComment || linkkind_comment_specific_above == startWithoutEnd ))
					{	
						inDocument->SetLineStyle(inLineNumber,0,findStopLoc(inDocument,inLineNumber,commenttypei),4);
					}

				}

				for (int i = inLineNumber+1; i < totallinenum; i++)
				{
					line_i_type = getLineKind_Comment(inDocument,i,commenttypei);
					inDocument->GetLine(i,xstr);
					if (line_i_type == endWithoutStart || line_i_type == startWithEnd ||line_i_type == startWithoutEnd)
					{			
						inDocument->SetLineStyle(i,0, findStopLoc(inDocument,i,commenttypei),4);						
						break;
					}
					else
					{
						inDocument->SetLineStyle(i,0,xstr.GetLength(),4);
						if (line_i_type == common)
						{
							setLineKind_Comment(inDocument,i,insideComment,commenttypei);
						}
					}
				}
			}



			//-----------------------------------------------------------------------------------------------------------------


			else if (checkType == onlyStartDeleteStart )   //case 21:       AAA /*  AA  ->  AAAAAA
			{
				if (inLineNumber > 0 && (linkkind_comment_specific_above == insideComment || linkkind_comment_specific_above ==startWithoutEnd))
				{
					inDocument->SetLineStyle(inLineNumber,0,len,4);
					setLineKind_Comment(inDocument,inLineNumber,insideComment,commenttypei);
				}
				else
				{
					for (int i = inLineNumber+1;i < inDocument->GetNbLines();i++)
					{
						line_i_type = getLineKind_Comment(inDocument,i,commenttypei);
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
						lexstrptr = startxmlParse(lexinput);  
						setLineKind_Lex(inDocument,i,getxmlLinetype());
						inDocument->SetLineStyle(i,0,len,4);  //initate the line
						for (int j = 0; j < getxmlKeywordNum(); j++ )
						{      
							inDocument->SetLineStyle(i,lexstrptr->indent,(lexstrptr->indent)+(lexstrptr->length),xmlcolorShadow[lexstrptr->style]);				
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

				if (inLineNumber > 0 && (linkkind_comment_specific_above == startWithoutEnd || linkkind_comment_specific_above == insideComment))
				{ 
					inDocument->SetLineStyle(inLineNumber,0,findStopLoc(inDocument,inLineNumber,commenttypei),4);
				}

				for (int i = inLineNumber +1; i < totallinenum; i++)
				{
					line_i_type = getLineKind_Comment(inDocument,i,commenttypei);

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
					lexstrptr = startxmlParse(lexinput);  
					setLineKind_Lex(inDocument,i,getxmlLinetype());
					inDocument->SetLineStyle(i,0,len,0);  //initate the line
					for (int j = 0; j < getxmlKeywordNum(); j++ )
					{      
						inDocument->SetLineStyle(i,lexstrptr->indent,(lexstrptr->indent)+(lexstrptr->length),xmlcolorShadow[lexstrptr->style]);						
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
				if (inLineNumber > 0 &&  (linkkind_comment_specific_above == insideComment || linkkind_comment_specific_above == startWithoutEnd ))
				{
					for (int i =inLineNumber; i < totallinenum ;i++)
					{
						line_i_type = getLineKind_Comment(inDocument,i,commenttypei);
						inDocument->GetLine(i,xstr);
						if (line_i_type == startWithEnd || line_i_type == endWithoutStart)
						{							
							inDocument->SetLineStyle(i,0,findStopLoc(inDocument,i,commenttypei),4);
							break;
						}
						else
						{
							inDocument->SetLineStyle(i,0,xstr.GetLength(),4);
							if (line_i_type == common)
							{
								setLineKind_Comment(inDocument,i,insideComment,commenttypei);
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
					if (inLineNumber > 0 &&  (linkkind_comment_specific_above == insideComment || linkkind_comment_specific_above == startWithoutEnd ))
					{		
						inDocument->SetLineStyle(inLineNumber,0,findStopLoc(inDocument,inLineNumber,commenttypei),4);
					}
				}

				if (checkType == insideType)
				{	
					setLineKind_Comment(inDocument,inLineNumber,insideComment,commenttypei);
				}

			}

			//---------------------------------------------------------------------------------------------------------------------------------------------
		}
	}

	delete lexinput;



	
}
















bool VXMLSyntax::CheckFolding( ICodeEditorDocument* inDocument, sLONG inLineNumber )
{
	if (inDocument->GetLineKind(inLineNumber) >= 100 )
	{
		return true;
	}
	else 
	{
		return false;
	}
}








void VXMLSyntax::ComputeFolding( ICodeEditorDocument* inDocument )
{ 

	if (allxmltags.GetCount() > xmltagbuffersize)     //if the vector's length is larger than 9000, parse the whole file again,to get better performernce
	{

		struct xmlLexStyleInfo * lexstrptr = 0;
		int nbline = inDocument->GetNbLines();
		allxmltags.Destroy();

		for (int j = 0 ;j < nbline;j++)
		{
			inDocument->SetLineKind(j,0);
		}

		for ( sLONG j = 0;j < nbline; j++ )
		{
			parseOneLineByLex(inDocument,j,lexstrptr);
			int xmllinetype = getxmlLinetype();
			inDocument->SetLineKind(j,xmllinetype);

			if ( ( xmllinetype%1000 )/100 > common)
			{
				allxmltags.Push(getxmlTagName());
				setLindKind_Tagnameloc(inDocument,j,allxmltags.GetCount());	
			}
		}
	}
	


 	int xmlNbLines = inDocument->GetNbLines();
	int wheretagbegin = 0;
	int line_i_ab = 0;                         //angle bracket type of line i

	VArrayOf<int>  stack_tagline;
	VArrayOf<VString> stack_tagnanme;

	VString line_i_tagname_temp = "";          //to store the tag name of the line i
	VString stack_tagname_last_temp = "";	   //to store the last tag name of stack tag name  		



	

	for (int atline_j = 0; atline_j < xmlNbLines; atline_j++ )   //initialization
	{
		inDocument->SetFoldable(atline_j,false);
		if ( inDocument->GetLineKind(atline_j) > 100)
		{
			setIsInpair(inDocument,atline_j,false);
		}
	}
 



	for (int atline_i = 0; atline_i < xmlNbLines; atline_i++)
	{
		line_i_ab = inDocument->GetLineKind(atline_i);
		line_i_ab = ((inDocument->GetLineKind(atline_i))%1000)/100;
	
		if (line_i_ab == tagbegin)                          //if line atline_i is tagbegin
		{
			stack_tagline.Push(atline_i);
			stack_tagnanme.Push( allxmltags.GetNth( getLindKind_Tagnameloc(inDocument,atline_i)) );		
		}
		else
			if (line_i_ab == tagend)                        //if line atline_i is tagb
			{
				if (stack_tagline.IsEmpty()) 
				{
					break;
				}
				
				line_i_tagname_temp = allxmltags.GetNth( getLindKind_Tagnameloc(inDocument,atline_i) );
				wheretagbegin =  stack_tagline.GetLast();
				stack_tagname_last_temp = allxmltags.GetNth( getLindKind_Tagnameloc(inDocument,wheretagbegin) );
				
				if ( line_i_tagname_temp == stack_tagname_last_temp )
				{

					inDocument->SetFoldable( wheretagbegin,true );
					inDocument->SetNbFoldedLines( wheretagbegin,atline_i - wheretagbegin );

 					setIsInpair(inDocument,wheretagbegin,true);
 					setIsInpair(inDocument,atline_i,true);
				
					stack_tagline.Pop();
					stack_tagnanme.Pop();

				}
				else
				{

					int whereformertagbegin = stack_tagnanme.FindPos( line_i_tagname_temp );
							
					if ( whereformertagbegin >0 && line_i_tagname_temp != "" )
					{
						while ( stack_tagnanme.GetCount() >= whereformertagbegin )
						{
							wheretagbegin = stack_tagline.Pop();    
						                    stack_tagnanme.Pop();
						}

 						inDocument->SetFoldable( wheretagbegin,true );
 						inDocument->SetNbFoldedLines( wheretagbegin,atline_i-wheretagbegin );
						setIsInpair( inDocument,wheretagbegin,true );
						setIsInpair( inDocument,atline_i,true );
					}
				}
			}

	}
}

















bool VXMLSyntax::CheckOutline( ICodeEditorDocument* inInterface, sLONG inLineNumber )
{
	return false;
}

void VXMLSyntax::ComputeOutline( ICodeEditorDocument* inInterface )
{
}

void VXMLSyntax::TokenizeLine( ICodeEditorDocument* inDocument, sLONG inLineIndex, std::vector<XBOX::VString>& ioLines )
{
}

void VXMLSyntax::GetTip( ICodeEditorDocument* inDocument, sLONG inLine, sLONG inPos, VString& outText, Boolean& outRtfText )
{
}

void VXMLSyntax::GetSuggestions( ICodeEditorDocument* inDocument, sLONG inLineIndex, sLONG inPos, ITipInfoArray *outSuggestions, sLONG& outStartOffset, bool inAll )
{
}

sLONG VXMLSyntax::GetIndentWidth()
{
	return 0;
}
bool VXMLSyntax::UseTab() 
{return true;}



/*
void VXMLSyntax::TokenizeOneLine( ICodeEditorDocument* inDocument )
{
	sLONG start, end, firstLine, endLine;
	inDocument->GetSelection(start,end,firstLine,endLine);


	if ( testAssert( start == 0 && end == 0 && firstLine == endLine && firstLine > 0 ) )
	{
		int indentlengthtab = 0;
		int indentlengthsp = 0;

		int extraIndent = 0;
		int startlineabtype = 0;
		sLONG setLineIndex = inDocument->GetLineIndex( endLine );
		sLONG startLineIndex = setLineIndex - 1;
		VString str;

		inDocument->GetLine(startLineIndex,str);

		startlineabtype = (inDocument->GetLineKind(startLineIndex)%1000)/100;
		
		if ( startlineabtype == tagbegin )
		{
			extraIndent = 1;
		}
		else if ( startlineabtype == tagend )
		{
			extraIndent = -1;
		}

		for ( int i = 0;i < str.GetLength(); i++ )
		{
			if ( str[i] ==  9 )
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


		if (indentlengthtab>0)
		{
			indentlengthtab = indentlengthtab + extraIndent;
		}
		else
		{
			indentlengthsp = indentlengthsp - 4;
		}


		for ( int j = 0; j < indentlengthtab ; j++ )
		{
			inDocument->InsertText( VString( (UniChar) 9 ) );  //insert tabs
		}

		for ( int i = 0; i < indentlengthsp;i++)
		{
			inDocument->InsertText( VString( (UniChar) 32 ) ); //insert spaces
		}

	}
}
*/






void VXMLSyntax::InsertChar( ICodeEditorDocument* inDocument, UniChar inUnichar, sLONG inLineIndex, sLONG inPosition, ITipInfoArray *outSuggestions, sLONG& outStartOffset )
{
	if ( inUnichar == '/' )
	{

		int xmlNbLines = inDocument->GetNbLines();
		VString str;
		inDocument->GetLine(inLineIndex,str);
		
	


		if (inPosition>1 && str[inPosition-2]=='<' ) 
		{
			VString tagtemp = "";

			for (int linei = xmlNbLines-1;linei >= 0;linei--)
			{
				int ilinekind = inDocument->GetLineKind(linei);
				
				if (ilinekind < 1000000000 && ((ilinekind%1000)/100) ==tagbegin )
				{
					tagtemp = allxmltags.GetNth(getLindKind_Tagnameloc(inDocument,linei));

					if (tagtemp.GetLength()>0)
					{	
						outSuggestions->AddTip( new VCodeEditorTipInfo( inDocument, tagtemp, 0 ) );
					}
		
				}
			}
		}  		
	}
	else if ( inUnichar == '<' )
	{
		inDocument->InsertText( VString( (UniChar) 62 ) ); //insert spaces       62  is  >
		sLONG startloc = 0;
		sLONG endloc = 0;
		sLONG atline = 0;	
		
		inDocument->GetSelection(startloc,endloc,atline,atline);
		if ( startloc>0 )
		{
			inDocument->Select(startloc-1,endloc-1,atline,atline);
		}
	}
	else if ( inUnichar == '=' )
	{
		inDocument->InsertText( CVSTR( "\"\"" ) ); 
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
	else if ( inUnichar == '!' )
	{

		VString strtemp = "";
		inDocument->GetLine( inLineIndex,strtemp );

		if ( inPosition >= 2 && strtemp[inPosition-2] == '<' )
		{
			inDocument->InsertText( "----"); //insert spaces       39 is '
			sLONG startloc = 0;
			sLONG endloc = 0;
			sLONG atline = 0;	

			inDocument->GetSelection(startloc,endloc,atline,atline);
			if ( startloc>0 )
			{
				inDocument->Select(startloc-2,endloc-2,atline,atline);
			}
		}
	}
	else if ( inUnichar == '#' )
	{
		VString strtemp = "";
		inDocument->GetLine( inLineIndex,strtemp );

		if ( inPosition >= 5 && strtemp[inPosition-2] == '-' && strtemp[inPosition-3] == '-' && strtemp[inPosition-4] == '!' && strtemp[inPosition-5] == '<')
		{
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










	int comment2kind = getLineKind_Comment(inDocument,inLineIndex,htmlsytlecomment);
	if ( comment2kind == startWithEnd2 || comment2kind == startWithEnd2 )
	{
		VString strtemp = "";

		inDocument->GetLine( inLineIndex,strtemp );

		for (int i = inPosition-1; i > 4; i--  )
		{
			if ( strtemp[i-5] == '<' && strtemp[i-4] == '!' && strtemp[i-3] == '-' && strtemp[i-2] == '-' && strtemp[i-1] == '#'  )
			{

				int extractedstrlength = inPosition - i;
				if (extractedstrlength > 6  )
				{
					extractedstrlength = 6;
				}	

				char *extractedstr; 
				extractedstr = new char[extractedstrlength+1];
				extractedstr[extractedstrlength] = '\0';

				for ( int j = 0; j < extractedstrlength; j++ )
				{
					extractedstr[j] = strtemp[i+j];
				}

				for ( int j = 0; j < 9; j++ )
				{
					if ( VSTRNICMP(extractedstr,xmlfourdkeywords[j],extractedstrlength) == 0 )
					{
						outSuggestions->AddTip( new VCodeEditorTipInfo( inDocument, xmlfourdkeywords[ j ], vcol_blue ) );
					}
				}
				break;
			}
		}

	}





}









/************************************************************************/
/* 

explanation of meaning of each number of the line kind  for xmlsyntax


signed long                2,147,483,648
									   8--->   comment linekind :  startwithoutend  startwithend...... to deal with the comment  
									  4---->   linekind  <!--
									 6 ---->   starttag(  < >  )   or endtag(  </ >  )							
							   7,483   ---->   to store the string that connect to this line, the string can hold the value of tagname 
							 14  ---------->   reserved, for future use						
						   2--------------->   whether this line is in pair
										*/
/************************************************************************/







int VXMLSyntax::getFullLindKind_Comment(ICodeEditorDocument *inDocument, int inLineNumber)
{
	return inDocument->GetLineKind(inLineNumber)%100;
}









int VXMLSyntax::getLineKind_Comment(ICodeEditorDocument* inDocument,int inLineNumber,int commenttype)
{
	if (commenttype == ordinarycomment)
	{
		return inDocument->GetLineKind(inLineNumber)%10;
	}
	else if (commenttype == htmlsytlecomment)
	{
		return  (inDocument->GetLineKind(inLineNumber)%100)/10;
	}
	else
	{
		xbox_assert( false);
		return 0;
	}
}




void VXMLSyntax::setLineKind_Comment(ICodeEditorDocument * inDocument, int inLineNumber,int comment_value, int commenttype)
{

	signed long full_line_i_kind = inDocument->GetLineKind(inLineNumber);
	
	if ( commenttype == ordinarycomment )
	{
		full_line_i_kind = full_line_i_kind - full_line_i_kind%10 + comment_value;
	}
	else if ( commenttype == htmlsytlecomment )
	{
		full_line_i_kind = full_line_i_kind - full_line_i_kind%100 + comment_value*10 + full_line_i_kind%10;
	}
	
	inDocument->SetLineKind(inLineNumber,full_line_i_kind);

}














int VXMLSyntax::findStopLoc(ICodeEditorDocument* inDocument,int inLineNumber,int commenttype)
{
	VString xstr;
	inDocument->GetLine(inLineNumber,xstr);
	const UniChar * unistringtemp  = xstr.GetCPointer();
	int stoploc = 0;


	if ( commenttype == ordinarycomment)
	{
		for (int i = 0;i < xstr.GetLength()-1;i++)
		{
			if (unistringtemp[i] == '*' && unistringtemp[i+1] == '/')
			{  
				stoploc = i+1;
				break;
			}
			if (unistringtemp[i] == '/' && unistringtemp[i+1] == '*')
			{
				stoploc = i +1;
				break;
			}
		}
	}
	else if ( commenttype == htmlsytlecomment )
	{
		for (int i = 0;i < xstr.GetLength()-3;i++)
		{ 
			//		char c = unistringtemp[i];
			if (unistringtemp[i+1] == '-' && unistringtemp[i+2] == '-' && unistringtemp[i+3] == '>')
			{  
				stoploc = i+2;
				break;
			}
			if (unistringtemp[i] == '<' && unistringtemp[i+1] == '!' && unistringtemp[i+2] == '-'&& unistringtemp[i+3] == '-')
			{
				stoploc = i + 3;
				break;
			}
		}

	}

	return stoploc;
}







void VXMLSyntax::UpdateBreakPointsLineNumbers( ICodeEditorDocument* inDocument, const std::vector<sWORD>& inBreakIDs, const std::vector<sLONG>& inLineNumbers )
{
}

void VXMLSyntax::AddBreakPoint( ICodeEditorDocument* inDocument, sLONG inLineNumber, sWORD& outID, bool& outDisabled )
{
	outID = 0;
}

bool VXMLSyntax::EditBreakPoint( ICodeEditorDocument* inDocument, sWORD inBreakID, bool& outDisabled )
{
	return false;
}

void VXMLSyntax::RemoveBreakPoint( ICodeEditorDocument* inDocument, sLONG inLineNumber )
{
}

bool VXMLSyntax::GetBreakPoints( ICodeEditorDocument* inDocument, std::vector<sLONG>& outLineNumbers, std::vector<sWORD>& outIDs, std::vector<bool>& outDisabled )
{
	return false;
}


void VXMLSyntax::UpdateCompilerErrors( ICodeEditorDocument* inDocument )
{
}

void VXMLSyntax::GotoNextMethodError( ICodeEditorDocument* inDocument, bool inNext )
{
}

void VXMLSyntax::SwapAssignments( ICodeEditorDocument* inDocument, VString& ioString )
{
}

bool VXMLSyntax::IsComment( ICodeEditorDocument* inDocument, const VString& inString )
{
	return false;
}

void VXMLSyntax::SwapComment( ICodeEditorDocument* inDocument, VString& ioString, bool inComment )
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








int VXMLSyntax::getLindKind_Tagnameloc(ICodeEditorDocument* inDocument,int inLineNumber)
{
	signed long i_linekind = inDocument->GetLineKind(inLineNumber);    /*  2,147,483,648  */
                                                                       //	   7,483      for number for tagname location
	i_linekind = i_linekind%10000000;
	i_linekind = i_linekind/1000;
	return i_linekind;
}





void VXMLSyntax::setLindKind_Tagnameloc(ICodeEditorDocument* inDocument, int inLineNumber,int tagname_location)
{
	signed long i_linekind = inDocument->GetLineKind(inLineNumber);
	if (i_linekind < 1000 )
	{
		inDocument->SetLineKind(inLineNumber,i_linekind + tagname_location*1000);
	}
	else
	{
		i_linekind = i_linekind - i_linekind%10000000 + tagname_location*1000 + i_linekind%1000; 
		inDocument->SetLineKind(inLineNumber,i_linekind);
	}
}





void VXMLSyntax::setLineKind_Lex(ICodeEditorDocument* inDocument, int inLineNumber,int lex_kind)
{
	signed long i_linekind = inDocument->GetLineKind(inLineNumber);
	i_linekind = i_linekind - i_linekind%1000 + lex_kind;
	inDocument->SetLineKind(inLineNumber,i_linekind);
}






int VXMLSyntax::getLineKind_Lex(ICodeEditorDocument* inDocument,int inLineNumber) 
{
	signed long i_linekind = inDocument->GetLineKind(inLineNumber);
	return i_linekind%1000;
}










int VXMLSyntax::getIsInpair(ICodeEditorDocument* inDocument, int inLineNumber)
{
	/*  2,147,483,648  */
	//  2      this num store the whether this line is already in pair

	//if 0     not in pair
	//if 1      is in pair

	signed long i_linekind = inDocument->GetLineKind(inLineNumber);
	i_linekind = i_linekind/1000000000;

	return i_linekind;
}





void VXMLSyntax::setIsInpair(ICodeEditorDocument* inDocument, int inLineNumber,bool isinpair)
{
	signed long i_linekind = inDocument->GetLineKind(inLineNumber);

	if (isinpair)
	{
		if (i_linekind < 1000000000)
		{
			i_linekind = i_linekind + 1000000000;
		}
	}
	else
	{	
		if(i_linekind > 1000000000 )
		{
			i_linekind = i_linekind - 1000000000; 
		}
	}
	inDocument->SetLineKind(inLineNumber,i_linekind);
}
   


















void VXMLSyntax::parseOneLineByLex(ICodeEditorDocument *&inDocument, sLONG inLineNumber,struct xmlLexStyleInfo *&inputstruct)
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
	inputstruct = startxmlParse(lexinput);  
	delete lexinput;
}





int VXMLSyntax::VSTRNICMP(const char *s1, const char *s2, int maxlen)
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




/*

void VXMLSyntax::removeDuplicate(VArrayOf<VString> &source)
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





void VXMLSyntax::xmlQuickSort( VArrayOf<VString> & thearray, int low, int high )       //to make "thearray" in order, "low" and "high" mean from where to where
{
	int i,j;
	VString pivot = "";

	int tempinsort = thearray.GetCount();

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
		xmlQuickSort(thearray,low,i-1);
		xmlQuickSort(thearray,i+1,high);
	}
}






void VXMLSyntax::binarySearch(VArrayOf<VString> &source, VArrayString* ioSuggestions, VArrayByte *ioSuggestionsIndex, VString userinput, int maxsuggestionNum, int color )
{

	int bslocbeg = 0;								 //binary search location begin
	int sourcesize = source.GetCount()-1;
	int bslocend = sourcesize;
	int bslocmid = 0;
	int bsresult=0;
	int inputlength = userinput.GetLength();

	if ( inputlength>0 )
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

		int xmlSuggestionCount = 0;

		if ( bsresult!=0 || ( bsresult==0 && sourcesize>0 &&userinput > source[0] ) )
		{
			bsresult++;
		}

		VString currentstr = "";
		for (int i = bsresult ; i < sourcesize && xmlSuggestionCount < maxsuggestionNum; i++)
		{
			currentstr = source.GetNth(i+1);
			if ( currentstr.BeginsWith( userinput ) )
			{	
				ioSuggestions->AppendString( currentstr );
				ioSuggestionsIndex->AppendByte(color);        
				xmlSuggestionCount++;
			}
			else
			{
				break;
			}
		}
	}
}

*/


void VXMLSyntax::CallHelp( ICodeEditorDocument* inDocument )
{
}


bool VXMLSyntax::IsMatchingCharacter( ICodeEditorDocument* inDocument, sLONG inLineIndex, sLONG inOffset, UniChar inChar )
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

bool VXMLSyntax::ShouldValidateTipWindow( UniChar inChar )
{
	return inChar == '(' || inChar == ':' || inChar == '=' || inChar == ';' || 
		   inChar == '<' || inChar == '>' || inChar == '+' || inChar == '-' || 
		   inChar == '{' || inChar == '/' || inChar == '#' || inChar == '[';
}


bool VXMLSyntax::DoubleClick( ICodeEditorDocument* inDocument, sLONG inLineIndex, sLONG inOffset, bool inAlternateKey )
{
	return false;
}

bool VXMLSyntax::DetermineMorphemeBoundary( ICodeEditorDocument *inDocument, sLONG inLineIndex, sLONG inOffset, sLONG &outLeftBoundary, sLONG &outLength, bool inAlternateKey )
{
	return false;
}
