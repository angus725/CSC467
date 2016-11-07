%{
/***********************************************************************
 * --YOUR GROUP INFO SHOULD GO HERE--
 * 
 *   Interface to the parser module for CSC467 course project.
 * 
 *   Phase 2: Implement context free grammar for source language, and
 *            parse tracing functionality.
 *   Phase 3: Construct the AST for the source language program.
 ***********************************************************************/

/***********************************************************************
 *  C Definitions and external declarations for this module.
 *
 *  Phase 3: Include ast.h if needed, and declarations for other global or
 *           external vars, functions etc. as needed.
 ***********************************************************************/

#include <string.h>
#include "common.h"
//#include "ast.h"
//#include "symbol.h"
//#include "semantic.h"
#define YYERROR_VERBOSE
#define yTRACE(x)    { if (traceParser) fprintf(traceFile, "%s\n", x); }

void yyerror(const char* s);    /* what to do in case of error            */

int yylex();              /* procedure for calling lexical analyzer */
extern int yyline;        /* variable holding current line number   */

enum {
  DP3 = 0, 
  LIT = 1, 
  RSQ = 2
};

%}

/***********************************************************************
 *  Yacc/Bison declarations.
 *  Phase 2:
 *    1. Add precedence declarations for operators (after %start declaration)
 *    2. If necessary, add %type declarations for some nonterminals
 *  Phase 3:
 *    1. Add fields to the union below to facilitate the construction of the
 *       AST (the two existing fields allow the lexical analyzer to pass back
 *       semantic info, so they shouldn't be touched).
 *    2. Add <type> modifiers to appropriate %token declarations (using the
 *       fields of the union) so that semantic information can by passed back
 *       by the scanner.
 *    3. Make the %type declarations for the language non-terminals, utilizing
 *       the fields of the union as well.
 ***********************************************************************/

%{
#define YYDEBUG 1
%}

// defines the yyval union
%union {
  int as_int;
  int as_vec;
  float as_float;
  char *as_str;
  int as_func;
}

%token          FLOAT_T
%token          INT_T
%token          BOOL_T
%token          CONST
%token          FALSE_C TRUE_C
%token          FUNC
%token          IF WHILE ELSE
%token          AND OR NEQ EQ LEQ GEQ

// links specific values of tokens to yyval
%token <as_vec>   VEC_T
%token <as_vec>   BVEC_T
%token <as_vec>   IVEC_T
%token <as_float> FLOAT_C
%token <as_int>   INT_C
%token <as_str>   ID

%left     OR
%left     AND
%nonassoc '=' NEQ '<' LEQ '>' GEQ EQ
%left     '+' '-'
%left     '*' '/'
%right    '^'
%nonassoc '!' UMINUS
%left		'[' '('
%right		']' ')'

%nonassoc NO_ELSE
%nonassoc ELSE

%start    program

/***********************************************************************
 *  Yacc/Bison rules
 *  Phase 2:
 *    1. Replace grammar found here with something reflecting the source
 *       language grammar
 *    2. Implement the trace parser option of the compiler
 ***********************************************************************/
%%

program
  : scope { yTRACE("program -> scope");}
  ;
scope
	:	'{' declarations statements '}' { yTRACE("scope -> '{' declarations statements '}'");}
	;
declarations
	:	declarations declaration { yTRACE("declarations -> declarations declaration");}
	|
	;
statements
	:	statements statement { yTRACE("statements -> statements statement");}
	|
	;
declaration
	:	type ID ';' { yTRACE("declaration -> type ID ';'");}
	|	type ID	'=' expression ';' { yTRACE("declaration -> type ID	'=' expression ';'");}
	|	CONST type ID '=' expression ';' { yTRACE("declaration -> 'const' type ID '=' expression ';'");}
	;
statement
	:	variable '=' expression ';' { yTRACE("statement -> variable '=' expression ';'");}
	|	IF '(' expression ')' statement %prec NO_ELSE { yTRACE("statement -> IF '(' expression ')' statement");}
	|	IF '(' expression ')' statement ELSE statement %prec ELSE { yTRACE("statement -> IF '(' expression ')' statement ELSE statement");}
	|	WHILE '(' expression ')' statement { yTRACE("statement -> WHILE '(' expression ')' statement");}
	|	scope { yTRACE("statement -> scope");}
	|	';'
	;
	
type
	:	FLOAT_T { yTRACE("type -> FLOAT_T");}
	|	INT_T { yTRACE("type -> INT_T");}
	|	BOOL_T { yTRACE("type -> BOOL_T");}
	|	VEC_T { yTRACE("type -> VEC_T");}
	|	IVEC_T { yTRACE("type -> IVEC_T");}
	|	BVEC_T { yTRACE("type -> BVEC_T");}

expression
	:	variable	{ yTRACE("expression -> variable");}
	| constructor	{ yTRACE("expression -> constructor");}
	| function	{ yTRACE("expression -> function");}
	| INT_C 		{ yTRACE("expression -> INT_C");}
	|	FLOAT_C		{ yTRACE("expression -> FLOAT_C");}
	| '!' expression %prec '!' { yTRACE("expression -> '!' expression");}
	| UMINUS expression %prec UMINUS { yTRACE("expression -> UMINUS expression");}
	| expression OR expression %prec OR { yTRACE("expression -> expression OR expression");}
	| expression AND expression %prec AND { yTRACE("expression -> expression AND expression");}
	| expression '=' expression %prec '=' { yTRACE("expression -> expression '=' expression");}
	| expression NEQ expression %prec NEQ { yTRACE("expression -> expression NEQ expression");}
	| expression '<' expression %prec '<' { yTRACE("expression -> expression '<' expression");}
	| expression LEQ expression %prec LEQ { yTRACE("expression -> expression LEQ expression");}
	| expression '>' expression %prec '>' { yTRACE("expression -> expression '>' expression");}
	| expression GEQ expression %prec GEQ { yTRACE("expression -> expression GEQ expression");}
	| expression EQ expression %prec EQ { yTRACE("expression -> expression EQ expression");}
	| expression '+' expression %prec '+' { yTRACE("expression -> expression '+' expression");}
	| expression '-' expression %prec '-' { yTRACE("expression -> expression '-' expression");}
	| expression '*' expression %prec '*' { yTRACE("expression -> expression '*' expression");}
	| expression '/' expression %prec '/' { yTRACE("expression -> expression '/' expression");}
	| expression '^' expression %prec '^' { yTRACE("expression -> expression '^' expression");}
	| FALSE_C	 	{ yTRACE("expression -> FALSE_C");}
	| TRUE_C		{ yTRACE("expression -> TRUE_C");}
	| '(' expression ')'	{ yTRACE("expression -> '(' expression ')' ");}
	;
variable
	:	ID 			{ yTRACE("variable -> ID");}
	|	ID '[' INT_C ']' { yTRACE("variable -> ID '[' INT_C ']'");}
	;

constructor
	:	type '(' arguments ')' { yTRACE("constructor -> type '(' arguments ')' ");}
	;
function
	:	function_name '(' arguments_opt ')' { yTRACE("function_name -> FUNC '(' arguments_opt ')' ");}
	;
function_name
	:	FUNC	{ yTRACE("function_name -> FUNC");}
	;
arguments_opt
	:	arguments  { yTRACE("arguments_opt -> arguments");}
	|
	;
arguments
	: arguments ',' expression {yTRACE("arguments -> arguments ',' expression arguments");}
	| expression	 {yTRACE("arguments -> expression");}
	;
%%

/***********************************************************************ol
 * Extra C code.
 *
 * The given yyerror function should not be touched. You may add helper
 * functions as necessary in subsequent phases.
 ***********************************************************************/
void yyerror(const char* s) {
  if(errorOccurred) {
    return;    /* Error has already been reported by scanner */
  } else {
    errorOccurred = 1;
  }

  fprintf(errorFile, "\nPARSER ERROR, LINE %d", yyline);
  
  if(strcmp(s, "parse error")) {
    if(strncmp(s, "parse error, ", 13)) {
      fprintf(errorFile, ": %s\n", s);
    } else {
      fprintf(errorFile, ": %s\n", s+13);
    }
  } else {
    fprintf(errorFile, ": Reading token %s\n", yytname[YYTRANSLATE(yychar)]);
  }
}

