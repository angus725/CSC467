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

[1-9]{DIGIT}*	{
				yRET_TRACE(INT)
			}
			
[+\-*/=]		{
				
				yRET_TRACE(yytext[0])
			}

.           { 
				fprintf(errorFile, "string:\"%s\"\n", yytext);
				yERROR("Unknown token"); }

%%
