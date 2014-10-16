

%{
#include <ctype.h>
#include <stdio.h>
#include <string.h>
#include "xml.lexer.h"


struct xmlLexStyleInfo xmlStyleInfoList[200] = {0};
const int inlex_xmlkeywordslistsize = 200;


const int  inlex_xmlstringbuffersize = 100;
char xmltagname[100] = "";

int cformer = 0;  
int xmllengthtemp = 0;
int xmlindenttemp = 0;
int xmlkeyWordNum = 0;
int xmlbufferlength = 0;


int ableftnum = 0;
int abrightnum = 0;
int abendleftnum = 0;
int abendrightnum = 0;

int xmllinetype_comment  = common;    /* store	 type of comment of the line */
int xmllinetype_comment2 = common;

int xmllinetype_ab      = common;    /* store	 state of tag of the line    */ 


%}

%option caseless


normal        [a-zA-Z_]+[a-z0-9A-Z_]*        
digit         [0-9]
xdigit        [0-9a-fA-F]
odigit        [0-7]
decnum        (0(\.{digit}+)?)|([1-9]{digit}*(\.{digit}+)?)
octnum        0{odigit}+
hexnum        0(x|X){xdigit}+
number        {decnum}|{octnum}|{hexnum}   


scommentend    "*/"
scommentend2   "-->"


ableft         <
abright        >
abendleft      "</"
abendright     "/>"        





specialicon1   [?`~!{}@$%^&#*()_+-=|\\;:,.\/]                     
specialicon2   "["*|"-"*|"]"


specialicon    {specialicon1}|{specialicon2}
othericons        .

char         (\'[^']*)|(\'[^']*\')
string       (\"[^"]*)|(\"[^"]*\") 

allnum      {digit}|{xdigit}|{odigit}|{decnum}|{octnum}|{hexnum}|{number}

/*allnum:type1*/



keyword       xml|xlink|xmlns|verion|standalone|encoding|element|html|attribute|pcdata



all           {keyword}|{allnum}|{normal}|{scommentend}|{scommentend2}|{specialicon}|{char}|{string}|{ableft}|{abright}|{abendleft}|{abendright}


            
%%
                               


"/*" {
		
     register int c = "";
     int curloctemp = 1;
	 xmllengthtemp = 2;
	 
	 if(xmlkeyWordNum == 0)
	   {
			curloctemp = xmlindenttemp + 2;
	   }
	else
	   {
			curloctemp = xmlStyleInfoList[xmlkeyWordNum-1].indent +xmlStyleInfoList[xmlkeyWordNum-1].length+2+xmlindenttemp;	      	
	   }
	 
	
		
     for(;;)
     {
		
		 while( (curloctemp < xmlbufferlength )&&  xmllengthtemp++ && curloctemp++  && ( c = input()) != '*' ) 
			;
			
			if(c == '*')
			{
				while( xmllengthtemp++ && curloctemp++  && (curloctemp < xmlbufferlength ) &&(c = input()) == '*')
				  ;
				  
				   if(c == '/')
					 {
					 xmllinetype_comment = startWithEnd;
					 xmlStyleInfoList[xmlkeyWordNum].style = comment_col;
					
					 break;				    
			         }
			 }	    
			 
			    if(curloctemp >= xmlbufferlength )
				 {	
					
					xmllinetype_comment = startWithoutEnd;
					xmlStyleInfoList[xmlkeyWordNum].style = comment_col;
					break;
				 } 
	
    	 } 	
     
    	  if(xmlkeyWordNum==0)
	   {
	       xmlStyleInfoList[xmlkeyWordNum].indent = xmlindenttemp; 
		   xmlStyleInfoList[xmlkeyWordNum].length = xmllengthtemp;	
		   
	   }
	   else
	   {	
			
			xmlStyleInfoList[xmlkeyWordNum].indent =xmlStyleInfoList[xmlkeyWordNum-1].indent +xmlStyleInfoList[xmlkeyWordNum-1].length+xmlindenttemp;			
			xmlStyleInfoList[xmlkeyWordNum].length =xmllengthtemp;
					
	   }  
	   xmlindenttemp = 0;	
	  if( xmlkeyWordNum < inlex_xmlkeywordslistsize )
		xmlkeyWordNum++; 
		
}





"<!--" {
     register int c = 0;
     int curloctemp = 1;
     int isend = 0;
     xmllengthtemp = 3;
	 
	
	if(xmlkeyWordNum == 0)
	   {
			curloctemp = xmlindenttemp + 3;
	   }
	else
	   {
			curloctemp = xmlStyleInfoList[xmlkeyWordNum-1].indent +xmlStyleInfoList[xmlkeyWordNum-1].length + 3 + xmlindenttemp;	      	
	   }
     
     
     
     for(;;curloctemp < xmlbufferlength)
     {
		
		 while( xmllengthtemp++ && curloctemp++  && (curloctemp < xmlbufferlength ) && (c = input()) != '-') 
			;		
			isend = 0;
			
			if(c == '-')
			{
				 
				while( xmllengthtemp++ && curloctemp++  &&  curloctemp < xmlbufferlength &&( c = input()) == '-')
				   {isend = 1;}
				  
				   if( c == '>' && isend == 1 )
					 {	
					     xmllinetype_comment2 = startWithEnd2;
					  	 xmllengthtemp++;			
						 xmlStyleInfoList[xmlkeyWordNum].style = comment_col;
						 break;						 			    
			         }
			 }	    
			 
			    if(xmllengthtemp >= xmlbufferlength )
				 {	
					xmllinetype_comment2 = startWithoutEnd2;				
					xmlStyleInfoList[xmlkeyWordNum].style = comment_col;
					break;
				 } 
			   
    	    
    	 } 	
    	 
    	 
     
    	  if(xmlkeyWordNum==0)
	   {
	       xmlStyleInfoList[xmlkeyWordNum].indent = xmlindenttemp; 
		   xmlStyleInfoList[xmlkeyWordNum].length = xmllengthtemp;			   
	   }
	   else
	   {
			xmlStyleInfoList[xmlkeyWordNum].indent =xmlStyleInfoList[xmlkeyWordNum-1].indent +xmlStyleInfoList[xmlkeyWordNum-1].length+xmlindenttemp;			
			xmlStyleInfoList[xmlkeyWordNum].length =xmllengthtemp;
					
	   }  
	   xmlindenttemp = 0;	
	   if(xmlkeyWordNum<inlex_xmlkeywordslistsize)
		 xmlkeyWordNum++;
}







{all} {	  	   
	   if(xmlkeyWordNum==0)
	   {
	       xmlStyleInfoList[xmlkeyWordNum].indent = xmlindenttemp; 
		   xmlStyleInfoList[xmlkeyWordNum].length = strlen(yytext);	
		   
	   }
	   else
	   {
			xmlStyleInfoList[xmlkeyWordNum].indent =xmlStyleInfoList[xmlkeyWordNum-1].indent +xmlStyleInfoList[xmlkeyWordNum-1].length+xmlindenttemp;			
			xmlStyleInfoList[xmlkeyWordNum].length =strlen(yytext);
					
	   }
	   xmlStyleInfoList[xmlkeyWordNum].style = 0;	  
	   xmlindenttemp = 0;	
	   if(xmlkeyWordNum<inlex_xmlkeywordslistsize)
		 xmlkeyWordNum++;
	   REJECT; 
}



{scommentend} {
    xmllinetype_comment = endWithoutStart;
	xmlStyleInfoList[xmlkeyWordNum-1].style = scommentend_col;	
 }
 


{keyword}  {
	xmlStyleInfoList[xmlkeyWordNum-1].style = keyword_col;
}





{ableft}  {
	xmlStyleInfoList[xmlkeyWordNum-1].style = tagbeginflag;
	if(xmllinetype_ab<5000)
		xmllinetype_ab = xmllinetype_ab*10 + 1; 
}


{abright}  {
	xmlStyleInfoList[xmlkeyWordNum-1].style = specialicon_col;
	if(xmllinetype_ab<5000)
		xmllinetype_ab = xmllinetype_ab*10 + 2; 
}


{abendleft}  {
	xmlStyleInfoList[xmlkeyWordNum-1].style = tagbeginflag;
	if(xmllinetype_ab<5000)
		xmllinetype_ab = xmllinetype_ab*10 + 3; 
}


{abendright}  {
	xmlStyleInfoList[xmlkeyWordNum-1].style = specialicon_col;
	if(xmllinetype_ab<5000)
		xmllinetype_ab = xmllinetype_ab*10 + 4; 
}






{specialicon} {
	xmlStyleInfoList[xmlkeyWordNum-1].style = specialicon_col;
}

 {scommentend2} {
	xmllinetype_comment2 = endWithoutStart2;	
	xmlStyleInfoList[xmlkeyWordNum-1].style = scommentend_col;	
 }


{char} {
    xmlStyleInfoList[xmlkeyWordNum-1].style = char_col;
}


{string} {
    xmlStyleInfoList[xmlkeyWordNum-1].style = string_col;
}




{allnum} {
	xmlStyleInfoList[xmlkeyWordNum-1].style = allnum_col;
}






{normal} {
    
	if( xmlkeyWordNum >= 2   &&  xmlStyleInfoList[xmlkeyWordNum-2].style == tagbeginflag )	
	{	
		
	    xmlStyleInfoList[xmlkeyWordNum-1].style = tag_col;    	    
	     if( xmlkeyWordNum == 2 && strlen(yytext) < inlex_xmlstringbuffersize)   
			strcpy(xmltagname,yytext);
			
	}
	else
	{
		xmlStyleInfoList[xmlkeyWordNum-1].style = normal_col;
	}
	
}



" "*  {
	xmlindenttemp = strlen(yytext);  
}


othericons {
  xmlindenttemp++;
}





%%


int yywrap()
{   
    return 1;
}

struct xmlLexStyleInfo * startxmlParse(char *strr)
{
 
  cformer  =  0;    
  xmllengthtemp  =  0;
  xmlindenttemp  =  0;
  xmlkeyWordNum  =  0;
  xmlbufferlength  =  strlen(strr);
  
  strcpy(xmltagname,"");
  xmllinetype_ab  =  common;
  xmllinetype_comment  = common; 
  xmllinetype_comment2 = common;

  	
  ableftnum  =  0;
  abrightnum  =  0;
  abendleftnum  =  0;
  abendrightnum  =  0;
 
  yy_switch_to_buffer(yy_scan_string(strr)) ;
  yylex(); 
  
  yy_delete_buffer(YY_CURRENT_BUFFER);
  xmlbufferlength = 0;
  return xmlStyleInfoList;   
	
}



int getxmlKeywordNum()
{
	return xmlkeyWordNum;
}




signed long getxmlLinetype()
{
	int linetypetemp = 0;
	switch(xmllinetype_ab)
	{
		case 112:   linetypetemp = tagbegin;break;
		case 132:   linetypetemp = tagend;break;
		default:    linetypetemp = common;
	}
	
	return  linetypetemp*100 + xmllinetype_comment2*10 + xmllinetype_comment;

} 




char * getxmlTagName()
{
	return xmltagname;
}