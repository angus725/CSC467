/* A Bison parser, made by GNU Bison 2.5.  */

/* Bison interface for Yacc-like parsers in C
   
      Copyright (C) 1984, 1989-1990, 2000-2011 Free Software Foundation, Inc.
   
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.
   
   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */


/* Tokens.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
   /* Put the tokens into the symbol table, so that GDB and other debuggers
      know about them.  */
   enum yytokentype {
     IDENTIFIER = 258,
     QUALIFIER_CONST = 259,
     SEPERATOR = 260,
     COMPLEXDATA = 261,
     IF = 262,
     ELSE = 263,
     FOR = 264,
     WHILE = 265,
     BOOL = 266,
     INT = 267,
     FLOAT = 268,
     AND = 269,
     OR = 270,
     EQ = 271,
     NEQ = 272,
     LT = 273,
     LE = 274,
     GT = 275,
     GE = 276
   };
#endif
/* Tokens.  */
#define IDENTIFIER 258
#define QUALIFIER_CONST 259
#define SEPERATOR 260
#define COMPLEXDATA 261
#define IF 262
#define ELSE 263
#define FOR 264
#define WHILE 265
#define BOOL 266
#define INT 267
#define FLOAT 268
#define AND 269
#define OR 270
#define EQ 271
#define NEQ 272
#define LT 273
#define LE 274
#define GT 275
#define GE 276




#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
typedef union YYSTYPE
{

/* Line 2068 of yacc.c  */
#line 57 "parser.y"

  int num;
  char* str;
  float fpn;
  



/* Line 2068 of yacc.c  */
#line 101 "y.tab.h"
} YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define yystype YYSTYPE /* obsolescent; will be withdrawn */
# define YYSTYPE_IS_DECLARED 1
#endif

extern YYSTYPE yylval;


