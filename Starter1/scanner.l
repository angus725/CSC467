%{
/**********************************************************************
 *
 * **YOUR GROUP INFO SHOULD GO HERE**
 *
 *  Interface to the scanner module for CSC467 course project
 **********************************************************************/


#include "common.h"
#include "parser.tab.h"
#include "string.h"
#include <math.h>

#define YY_USER_INIT { yyin = inputFile; }
#define	yyinput      input
#define yTRACE(x)    { if (traceScanner) fprintf(traceFile, "TOKEN %3d : %s\n", x, yytext);}
#define yERROR(x)    { fprintf(errorFile, "\nLEXICAL ERROR, LINE %d: %s\n", yyline, x); errorOccurred = TRUE; yyterminate(); }
#define yRET_TRACE(x) { yTRACE(x) return x;}

int CheckInt(void);
int yyline = 1;

%}
%option noyywrap

WS          [ \t]
DIGIT		[0-9]
BOL	true|false 	
%%
"/*"        {
              int c1      = 0;
              int c2      = yyinput();
              int curline = yyline;
              for(;;) {
                if (c2 == EOF) {
                  fprintf(errorFile, "\nLEXICAL ERROR, LINE %d: Unmatched /*\n", curline);
                  errorOccurred = TRUE;
                  yyterminate();
                }
                if (c1 == '*' && c2 == '/')
                  break;
                c1 = c2;
                c2 = yyinput();
                if (c1 == '\n' && c2 != EOF)
                  yyline++;
              }
            }
"\n"		{
			yyline++;
		}
{WS}*		{}

0|([1-9]{DIGIT}*)	{
			long value;
			
			if (yyleng > 10)
				yERROR("Exceed maximum int literal size")
			
			value = strtol(yytext, NULL, 10);
			if (value > MAX_INTEGER)
				yERROR("Exceed maximum int literal size")
			
			yylval.num = value;
			yRET_TRACE(INT)
		}

({DIGIT}*\.{DIGIT}+)|({DIGIT}+\.{DIGIT}*) {
			float value;
			
			if (yyleng > 10)
				yERROR("Exceed max float literal precision")
			
			value = atof(yytext);
			if (value > HUGE_VALF)
				yERROR("Exceed maxi float literal size")
			
			yylval.fpn = value;
			yRET_TRACE(FLOAT)
			
		}

&&		{yRET_TRACE(AND)}

"||"		{yRET_TRACE(OR)}

==		{yRET_TRACE(EQ)}

"!="		{yRET_TRACE(NEQ)}

"<="		{yRET_TRACE(LE)}

">="		{yRET_TRACE(GE)}

[+\-*/=()\[\]{}<>;!^]		{
				
				yRET_TRACE(yytext[0])
			}
BOL	{
				yRET_TRACE(BOOL)
			}
			
for			{
				yRET_TRACE(FOR)
			}
			
while			{
				yRET_TRACE(WHILE)
			}

if			{
				yRET_TRACE(IF)
			}
else			{
				yRET_TRACE(ELSE)
			}

const			{
				yRET_TRACE(QUALIFIER_CONST)
			}

(b|i)?vec[2-4] 		{
				yRET_TRACE(COMPLEXDATA)
			}

[_a-zA-Z][_a-zA-Z0-9]	{
				if(yyleng > 10)
					yERROR("Exceeded maxinum identifier size")
				yRET_TRACE(IDENTIFIER)
			}

\,			{
				yRET_TRACE(SEPERATOR)
			}

.           		{	 
				fprintf(errorFile, "string:\"%s\"\n", yytext);
				yERROR("Unknown token"); }
%%
