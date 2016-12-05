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
#include "ast.h"
// #include "symbol.h"
#include "semantic.h"

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
  double as_float;
  char *as_str;
  int as_func;
  ASTNode *as_ast;
}

%token          FLOAT_T
%token          INT_T
%token          BOOL_T
%token          CONST
%token          FALSE_C TRUE_C
%token          IF ELSE
%token          AND OR NEQ EQ LEQ GEQ

// links specific values of tokens to yyval
%token <as_vec>   VEC_T
%token <as_vec>   BVEC_T
%token <as_vec>   IVEC_T
%token <as_float> FLOAT_C
%token <as_int>   INT_C
%token <as_str>   ID
%token <as_func>  FUNC

// operator precdence
%left     OR                        // 7
%left     AND                       // 6
%left     EQ NEQ '<' LEQ '>' GEQ    // 5
%left     '+' '-'                   // 4
%left     '*' '/'                   // 3
%right    '^'                       // 2
%right    '!' UMINUS                // 1
%left     '(' '['                   // 0

// resolve dangling else shift/reduce conflict with associativity
%left     WITHOUT_ELSE
%left     WITH_ELSE

// type declarations
// TODO: fill this out

%type <as_ast> program
%type <as_ast> scope
%type <as_ast> declarations
%type <as_ast> statements
%type <as_ast> declaration
%type <as_ast> statement
%type <as_ast> type
%type <as_ast> expression
%type <as_ast> variable
%type <as_ast> arguments
%type <as_ast> arguments_opt

// expect one shift/reduce conflict, where Bison chooses to shift
// the ELSE.
%expect 1

%start    program

%%

/***********************************************************************
 *  Yacc/Bison rules
 *  Phase 2:
 *    1. Replace grammar found here with something reflecting the source
 *       language grammar
 *    2. Implement the trace parser option of the compiler
 ***********************************************************************/
program
  : scope 
      { 
      	ast = $1;
      	yTRACE("program -> scope\n") 
      } 
  ;

scope
  : '{' declarations statements '}'
      {
      	$$ = astAllocate(SCOPE, yyline, $2, $3);
      	yTRACE("scope -> { declarations statements }\n") 
      }
  ;

declarations
  : declaration declarations 
      { 
      	$$ = astAllocate(MULTI_NODE, yyline, $2, $1);
      	yTRACE("declarations -> declarations declaration\n") 
      }
  | 
      {
      	$$ = NULL;
      	yTRACE("declarations -> \n") 
      }
  ;

statements
  : statement statements  
      { 
      	$$ = astAllocate(MULTI_NODE, yyline, $2, $1);
      	yTRACE("statements -> statements statement\n") 
      }
  | 
      { 
      	$$ = NULL;
      	yTRACE("statements -> \n") 
      }
  ;

/* 4 parameters are const or not, type, ID, expression*/
declaration
  : type ID ';' 
      {
      	$$ = astAllocate(DECLARATION, yyline, 0, $1, $2, NULL);
      	yTRACE("declaration -> type ID ;\n")
      }
  | type ID '=' expression ';'
      {
      	$$ = astAllocate(DECLARATION, yyline, 0, $1, $2, $4);
      	yTRACE("declaration -> type ID = expression ;\n") 
      }
  | CONST type ID '=' expression ';'
      {
      	$$ = astAllocate(DECLARATION, yyline, 1, $2, $3, $5);
      	yTRACE("declaration -> CONST type ID = expression ;\n") 
      }
  ;

statement
  : variable '=' expression ';'
      {
      	$$ = astAllocate(ASSIGN_STATEMENT, yyline, $1, $3);
      	yTRACE("statement -> variable = expression ;\n")
      }
  | IF '(' expression ')' statement ELSE statement %prec WITH_ELSE
      {
      	$$ = astAllocate(IF_STATEMENT, yyline, $3, $5, $7);
      	yTRACE("statement -> IF ( expression ) statement ELSE statement \n")
      }
  | IF '(' expression ')' statement %prec WITHOUT_ELSE
      {
      	$$ = astAllocate(IF_STATEMENT, yyline, $3, $5, NULL);
      	yTRACE("statement -> IF ( expression ) statement \n")
      }
  | scope 
      { 
      	$$ = $1;
      	yTRACE("statement -> scope \n") 
      }
  | ';'
      { 
      	$$ = NULL;
      	yTRACE("statement -> ; \n") 
      }
  ;

type
  : INT_T
      { 
      	$$ = astAllocate(TYPE, yyline, TYPE_INT);
      	yTRACE("type -> INT_T \n")
      }
  | IVEC_T
      {
      	enum data_type type = TYPE_UNKNOWN;
      	switch($1) {
      	case 1:
      		type = TYPE_IVEC2;
      		break;
      	case 2:
      		type = TYPE_IVEC3;
      		break;
      	case 3:
      		type = TYPE_IVEC4;
      		break;
      	default: break;
      	}
      	$$ = astAllocate(TYPE, yyline, type);
      	yTRACE("type -> IVEC_T \n") 
      }
  | BOOL_T
      { 
      	$$ = astAllocate(TYPE, yyline, TYPE_BOOL);
      	yTRACE("type -> BOOL_T \n") 
      }
  | BVEC_T
      { 
        enum data_type type = TYPE_UNKNOWN;
      	switch($1) {
      	case 1:
      		type = TYPE_BVEC2;
      		break;
      	case 2:
      		type = TYPE_BVEC3;
      		break;
      	case 3:
      		type = TYPE_BVEC4;
      		break;
      	default: break;
      	}
      	$$ = astAllocate(TYPE, yyline, type);
      	yTRACE("type -> BVEC_T \n") 
      }
  | FLOAT_T
      { 
      	$$ = astAllocate(TYPE, yyline, TYPE_FLOAT);
      	yTRACE("type -> FLOAT_T \n") 
      }
  | VEC_T
      {
        enum data_type type = TYPE_UNKNOWN;
      	switch($1) {
      	case 1:
      		type = TYPE_VEC2;
      		break;
      	case 2:
      		type = TYPE_VEC3;
      		break;
      	case 3:
      		type = TYPE_VEC4;
      		break;
      	default: break;
      	}
      	$$ = astAllocate(TYPE, yyline, type);
      	yTRACE("type -> VEC_T \n") 
      }
  ;

expression

  /* function-like operators */
  : type '(' arguments_opt ')' %prec '('
      { 
      	$$ = astAllocate(CONSTRUCTOR_EXP, yyline, $1, $3);
      	yTRACE("expression -> type ( arguments_opt ) \n")
      }
  | FUNC '(' arguments_opt ')' %prec '('
      {
      	$$ = astAllocate(FUNC_CALL_EXP, yyline, $1, $3);
      	yTRACE("expression -> FUNC ( arguments_opt ) \n")
      }

  /* unary opterators */
  | '-' expression %prec UMINUS
      { 
      	$$ = astAllocate(UNARY_EXP, yyline, UOPT_NEG, $2);
      	yTRACE("expression -> - expression \n") 
      }
  | '!' expression %prec '!'
      { 
      	$$ = astAllocate(UNARY_EXP, yyline, UOPT_NOT, $2);
      	yTRACE("expression -> ! expression \n")
      }

  /* binary operators */
  | expression AND expression %prec AND
      {
      	$$ = astAllocate(BINARY_EXP, yyline, BOPT_AND, $1, $3);
      	yTRACE("expression -> expression AND expression \n")
      }
  | expression OR expression %prec OR
      {
      	$$ = astAllocate(BINARY_EXP, yyline, BOPT_OR, $1, $3);
      	yTRACE("expression -> expression OR expression \n")
      }
  | expression EQ expression %prec EQ
      {
      	$$ = astAllocate(BINARY_EXP, yyline, BOPT_EQ, $1, $3);
      	yTRACE("expression -> expression EQ expression \n")
      }
  | expression NEQ expression %prec NEQ
      { 
      	$$ = astAllocate(BINARY_EXP, yyline, BOPT_NEQ, $1, $3);
      	yTRACE("expression -> expression NEQ expression \n") 
      }
  | expression '<' expression %prec '<'
      { 
      	$$ = astAllocate(BINARY_EXP, yyline, BOPT_LT, $1, $3);
      	yTRACE("expression -> expression < expression \n") 
      }
  | expression LEQ expression %prec LEQ
      { 
      	$$ = astAllocate(BINARY_EXP, yyline, BOPT_LEQ, $1, $3);
      	yTRACE("expression -> expression LEQ expression \n") 
      }
  | expression '>' expression %prec '>'
      { 
      	$$ = astAllocate(BINARY_EXP, yyline, BOPT_GT, $1, $3);
      	yTRACE("expression -> expression > expression \n") 
      }
  | expression GEQ expression %prec GEQ
      {
      	$$ = astAllocate(BINARY_EXP, yyline, BOPT_GEQ, $1, $3);
      	yTRACE("expression -> expression GEQ expression \n")
      }
  | expression '+' expression %prec '+'
      {
      	$$ = astAllocate(BINARY_EXP, yyline, BOPT_ADD, $1, $3);
      	yTRACE("expression -> expression + expression \n") 
      }
  | expression '-' expression %prec '-'
      { 
      	$$ = astAllocate(BINARY_EXP, yyline, BOPT_SUB, $1, $3);
      	yTRACE("expression -> expression - expression \n") 
      }
  | expression '*' expression %prec '*'
      { 
      	$$ = astAllocate(BINARY_EXP, yyline, BOPT_MUL, $1, $3);
      	yTRACE("expression -> expression * expression \n") 
      }
  | expression '/' expression %prec '/'
      {
      	$$ = astAllocate(BINARY_EXP, yyline, BOPT_DIV, $1, $3);
      	yTRACE("expression -> expression / expression \n") 
      }
  | expression '^' expression %prec '^'
      {
      	$$ = astAllocate(BINARY_EXP, yyline, BOPT_EXPO, $1, $3);
      	yTRACE("expression -> expression ^ expression \n") 
      }

  /* literals */
  | TRUE_C
      {
      	$$ = astAllocate(LITERAL_EXP, yyline, TYPE_BOOL, -1);
      	yTRACE("expression -> TRUE_C \n")
      }
  | FALSE_C
      {
      	$$ = astAllocate(LITERAL_EXP, yyline, TYPE_BOOL, 1);
      	yTRACE("expression -> FALSE_C \n")
      }
  | INT_C
      {
      	$$ = astAllocate(LITERAL_EXP, yyline, TYPE_INT, $1);
      	yTRACE("expression -> INT_C \n")
      }
  | FLOAT_C
      {
      	$$ = astAllocate(LITERAL_EXP, yyline, TYPE_FLOAT, $1);
      	yTRACE("expression -> FLOAT_C \n")
      }

  /* misc */
  | '(' expression ')'
      {
      	$$ = $2;
      	yTRACE("expression -> ( expression ) \n")
      }
  | variable
    {
    	$$ = $1;
    	yTRACE("expression -> variable \n")
    }
  ;

variable
  : ID
      {
      	$$ = astAllocate(VARIABLE, yyline, $1, 0);
      	yTRACE("variable -> ID \n")
      }
  | ID '[' INT_C ']' %prec '['
      {
      	$$ = astAllocate(VARIABLE, yyline, $1, 1, $3);
      	yTRACE("variable -> ID [ INT_C ] \n")
      }
  ;

arguments
  : arguments ',' expression
      {
      	$$ = astAllocate(MULTI_NODE, yyline, $1, $3);
      	yTRACE("arguments -> arguments , expression \n")
      }
  | expression
      {
      	$$ = $1;
      	yTRACE("arguments -> expression \n")
      }
  ;

arguments_opt
  : arguments
      {
      	$$ = $1;
      	yTRACE("arguments_opt -> arguments \n")
      }
  |
      {
      	$$ = NULL;
      	yTRACE("arguments_opt -> \n")
      }
  ;

%%

/***********************************************************************ol
 * Extra C code.
 *
 * The given yyerror function should not be touched. You may add helper
 * functions as necessary in subsequent phases.
 ***********************************************************************/
void yyerror(const char* s) {
  if(errorOccurred){
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

