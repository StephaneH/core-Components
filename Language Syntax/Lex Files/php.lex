

%{
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "php.lexer.h"

const int  inlex_phpkeywordslistsize = 200;
struct PHPStyleInfo PHPStyleInfoList[200] = {0};

const int  inlex_phpstringbuffersize = 100;
char PHPlastInput[100] = "";
char PHPextraInfo[100] = "";

int cformer = 0;  
int PHPlengthtemp = 0;
int PHPindenttemp = 0;
int PHPkeyWordNum = 0;
int PHPbufferlength = 0;
int PHPhasFunction = 0;
int PHPhasVariable = 0;
int PHPextrainfotype = 0;

int PHPlinetype_comment = common;
int PHPlinetype_fold = 0;

%}




normal       [a-zA-Z_]+[a-z0-9A-Z_]*        
digit         [0-9]
xdigit        [0-9a-fA-F]
odigit        [0-7]
decnum        (0(\.{digit}+)?)|([1-9]{digit}*(\.{digit}+)?)
octnum        0{odigit}+
hexnum        0(x|X){xdigit}+
number        {decnum}|{octnum}|{hexnum}
lcomment    \/\/.*     
scommentend    "*/"

specialicon1   [`~!@%^&*()_+-=|\\;:<,>.?\/"]                     
specialicon2   "["*|"-"*|"]"*
leftbracket    "{"
rightbracket    "}"
vardefine      "$"
specialicon    {specialicon1}|{specialicon2}



char         (\'[^']*)|(\'[^']*\')

preproc        #.*

allnum      {digit}|{xdigit}|{odigit}|{decnum}|{octnum}|{hexnum}|{number}|{lcomment}|{char}|{preproc}|{scommentend}|{specialicon}

/*allnum:type1*/












dataDefine       var
projectDefine    include_once|include|namespace|declare|enddeclare|:first-letter
classDefine      class|FILE|extends|interface|implements|public|private|clone|protected|import|php_user_filter|require_once|requirearray|list|global|static|const|final|eval
functionDefine	 function|new|use|return|method|abstract|line|cfunction|old_function 
allControls	     break|switch|case|default|die|do|echo|else|elseif|endfor|foreach|for|endforeach|exit|isset|while|goto|continue|if|endif|endwhile|endswitch
others           and|or|xor|as|exception|empty|try|catch|this|throw|unset|print



dataType          {dataDefine}|{vardefine}

keyword           {dataType}|{projectDefine}|{classDefine}|{allControls}|{functionDefine}
othericons        .
suggestionGroup   {keyword}|{normal}
all               {keyword}|{allnum}|{normal}|{leftbracket}|{rightbracket}


            
%%
                               

"/*" {
     register int c = "";
      int curloctemp = 1;
     PHPlengthtemp = 2;
    
     
   
     
      if(PHPkeyWordNum == 0)
	   {
			curloctemp = PHPindenttemp + 2;
	   }
	else
	   {
			curloctemp = PHPStyleInfoList[PHPkeyWordNum-1].indent +PHPStyleInfoList[PHPkeyWordNum-1].length + 2 + PHPindenttemp;	      	
	   }
 
 
	 	
 
    for(;;)
     {
		
		 while( (curloctemp < PHPbufferlength ) &&  PHPlengthtemp++ && curloctemp++  && ( c = input()) != '*' ) 
			;
			
			if(c == '*')
			{
				while( PHPlengthtemp++ && curloctemp++  && (curloctemp < PHPbufferlength ) &&(c = input()) == '*')
				  ;
				  
				   if(c == '/')
					 {
					 PHPlinetype_comment= startWithEnd;
					 PHPStyleInfoList[PHPkeyWordNum].style = lcomment_col;
					
					 break;				    
			         }
			 }	    
			
			    if(curloctemp >= PHPbufferlength )
				 {	
					 
					PHPlinetype_comment= startWithoutEnd;
					PHPStyleInfoList[PHPkeyWordNum].style = lcomment_col;
					break;
				 } 
	
    	 } 	
     
    	  if(PHPkeyWordNum==0)
	   {
	       PHPStyleInfoList[PHPkeyWordNum].indent = PHPindenttemp; 
		   PHPStyleInfoList[PHPkeyWordNum].length = PHPlengthtemp;	
		   
	   }
	   else
	   {	
			
			PHPStyleInfoList[PHPkeyWordNum].indent =PHPStyleInfoList[PHPkeyWordNum-1].indent +PHPStyleInfoList[PHPkeyWordNum-1].length+PHPindenttemp;			
			PHPStyleInfoList[PHPkeyWordNum].length =PHPlengthtemp;
					
	   }  
	   PHPindenttemp = 0;	
	  if( PHPkeyWordNum < inlex_phpkeywordslistsize )
		PHPkeyWordNum++; 
    
}




"\"" {
	

			
	register int C;
    int stringindenttemp = 1;
	PHPlengthtemp = 1;
	if(PHPkeyWordNum == 0)
	   {
			stringindenttemp = 1;
	   }
	else
	   {
			stringindenttemp = PHPStyleInfoList[PHPkeyWordNum-1].indent +PHPStyleInfoList[PHPkeyWordNum-1].length+1+PHPindenttemp;	      	
	   }
		

         for(;stringindenttemp<PHPbufferlength;stringindenttemp++)
         {                	
               if((C=input())=='\"'&& cformer!='\\')    
                   { PHPlengthtemp++;break;}
               PHPlengthtemp++;    
               cformer = C;               
         }
	
	
	   if(PHPkeyWordNum==0) 
	   {
	       PHPStyleInfoList[PHPkeyWordNum].indent = PHPindenttemp; 
		   PHPStyleInfoList[PHPkeyWordNum].length = PHPlengthtemp;	
		   
	   }
	   else
	   {
			PHPStyleInfoList[PHPkeyWordNum].indent =PHPStyleInfoList[PHPkeyWordNum-1].indent +PHPStyleInfoList[PHPkeyWordNum-1].length+PHPindenttemp;			
			PHPStyleInfoList[PHPkeyWordNum].length =PHPlengthtemp;
					
	   }  	
	   
	PHPStyleInfoList[PHPkeyWordNum].style = string_col;
    PHPindenttemp = 0;	
	if( PHPkeyWordNum < inlex_phpkeywordslistsize )
			PHPkeyWordNum++;
}





{all} {	  
	   if(PHPkeyWordNum==0)
	   {
	       PHPStyleInfoList[PHPkeyWordNum].indent = PHPindenttemp; 
		   PHPStyleInfoList[PHPkeyWordNum].length = strlen(yytext);	
		   
	   }
	   else
	   {
			PHPStyleInfoList[PHPkeyWordNum].indent =PHPStyleInfoList[PHPkeyWordNum-1].indent +PHPStyleInfoList[PHPkeyWordNum-1].length+PHPindenttemp;			
			PHPStyleInfoList[PHPkeyWordNum].length =strlen(yytext);
					
	   }
	   PHPStyleInfoList[PHPkeyWordNum].style = 0;	  
	   PHPindenttemp = 0;	
	   if( PHPkeyWordNum < inlex_phpkeywordslistsize )
			PHPkeyWordNum++;
	   REJECT; 
	    
}



{suggestionGroup} {
	if(strlen(yytext) < inlex_phpstringbuffersize)
      strcpy(PHPlastInput,yytext);
      REJECT;
}



{scommentend} {
	PHPlinetype_comment = endWithoutStart;	
	PHPStyleInfoList[PHPkeyWordNum-1].style = scommentend_col;	
 }
 
{lcomment} {
	PHPStyleInfoList[PHPkeyWordNum-1].style = lcomment_col;
}



{dataType} {
    PHPStyleInfoList[PHPkeyWordNum-1].style = datatype_col;  
	PHPhasVariable = 1;  
}


{projectDefine} {
	PHPStyleInfoList[PHPkeyWordNum-1].style = projectDefine_col; 
}



{classDefine} {
	PHPStyleInfoList[PHPkeyWordNum-1].style = classDefine_col; 
}



{allControls} {
	PHPStyleInfoList[PHPkeyWordNum-1].style = AllControls_col; 
}


{functionDefine} {
	PHPStyleInfoList[PHPkeyWordNum-1].style = functionDefine_col; 
	if(!strcmp(yytext,"function"))
	   {PHPhasFunction = 1;}
}



{char} {
    PHPStyleInfoList[PHPkeyWordNum-1].style = char_col;
}



{allnum} {
	PHPStyleInfoList[PHPkeyWordNum-1].style = allnum_col;
}



{leftbracket} {
	PHPlinetype_fold++;
	PHPStyleInfoList[PHPkeyWordNum-1].style = specialicon_col;  
}



{rightbracket} {
	PHPlinetype_fold--;
	PHPStyleInfoList[PHPkeyWordNum-1].style = specialicon_col;  
}



{specialicon} {
	PHPStyleInfoList[PHPkeyWordNum-1].style = specialicon_col;
}




{normal} {
    PHPStyleInfoList[PHPkeyWordNum-1].style = normal_col;
    
    if( PHPhasFunction == 1  && PHPkeyWordNum >1 && PHPStyleInfoList[PHPkeyWordNum-2].style == functionDefine_col  )
		{
			if(strlen(yytext) < inlex_phpstringbuffersize)
			     strcpy(PHPextraInfo,yytext);
						
			PHPextrainfotype = PHPfunctioninfo;
			PHPStyleInfoList[PHPkeyWordNum-1].style = functionName_col;    
		}
		
			
    if( PHPhasVariable == 1  && PHPkeyWordNum >1 && PHPStyleInfoList[PHPkeyWordNum-2].style == datatype_col  )
		{
			if(strlen(yytext) < inlex_phpstringbuffersize)
				strcpy(PHPextraInfo,yytext);																				
			
			PHPextrainfotype = PHPvariableinfo;
			PHPStyleInfoList[PHPkeyWordNum-1].style = variableName_col;
		}		
}




" "*  {
		   PHPindenttemp = strlen(yytext);  
}

othericons {
	PHPindenttemp++;
    PHPStyleInfoList[PHPkeyWordNum-1].style = specialicon_col;
}







%%




int yywrap()
{   
    return 1;
}







struct PHPStyleInfo * startPHPParse(char *strr)
{
  cformer = 0;	
  PHPbufferlength = strlen(strr);
  PHPindenttemp = 0;
  PHPlinetype_fold= 0;
  PHPkeyWordNum = 0;
  PHPhasVariable = 0;
  PHPlinetype_comment = common;
  
  strcpy(PHPextraInfo,"");
  
  yy_switch_to_buffer(yy_scan_string(strr)) ;
  yylex(); 
  
  if(PHPStyleInfoList[PHPkeyWordNum-1].style ==normal_col || PHPStyleInfoList[PHPkeyWordNum-1].style  == AllControls_col 
     || PHPStyleInfoList[PHPkeyWordNum-1].style == datatype_col || PHPStyleInfoList[PHPkeyWordNum-1].style ==projectDefine_col )
     {
		;
     }
  else
     { strcpy(PHPlastInput,"");}
     
 
  yy_delete_buffer(YY_CURRENT_BUFFER);
  PHPbufferlength = 0;
  return PHPStyleInfoList;   
}




int getPHPkeywordNum()
{
	return PHPkeyWordNum;
}



signed long getPHPLexLineType()
{	
	if(PHPlinetype_fold>=0)
	   return   PHPextrainfotype*1000 + PHPlinetype_fold*10 + PHPlinetype_comment;
	else 
	   return  -PHPextrainfotype*1000 + PHPlinetype_fold*10 - PHPlinetype_comment;
}




char * getPHPLastInput()
{
	return  PHPlastInput;
}



char * getPHPExtraInfo()
{
	return PHPextraInfo;
}