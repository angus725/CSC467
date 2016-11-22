#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>

#include "ast.h"
#include "common.h"
#include "parser.tab.h"

#define DEBUG_PRINT_TREE 0

node *ast = NULL;



node *ast_allocate(node_kind kind, ...) {
  va_list args;

  // make the node
  node *ast = (node *) malloc(sizeof(node));
  memset(ast, 0, sizeof *ast);
  ast->kind = kind;

  va_start(args, kind);

  ast->line_num = va_arg(args, int);

  switch(kind) {
  case SCOPE:
	  ast->scope.declarations = va_arg(args, node*);
	  ast->scope.statements = va_arg(args, node*);
	  break;
  case MULTI_NODE:
	  ast->multi_node.nodes = va_arg(args, node*);
	  ast->multi_node.cur_node = va_arg(args, node*);
	  break;
  case DECLARATION:
	  ast->declaration.is_const = va_arg(args, int);
	  ast->declaration.type = va_arg(args, node*);
	  ast->declaration.identifier = va_arg(args, char*);
	  ast->declaration.expression = va_arg(args, node*);
	  break;
  case ASSIGN_STATEMENT:
	  ast->assignment_statement.variable = va_arg(args, node*);
	  ast->assignment_statement.expression = va_arg(args, node*);
	  break;
  case IF_STATEMENT:
	  ast->if_statement.if_confition = va_arg(args, node*);
	  ast->if_statement.if_body = va_arg(args, node*);
	  ast->if_statement.else_body = va_arg(args, node*);
	  break;
  case TYPE:
	  ast->type.var_type = (enum var_type)va_arg(args, int);
	  ast->type.array_len = va_arg(args, int);
	  break;
  case FUNC_CALL_EXP:
	  ast->func_call_exp.func = (enum func_type) va_arg(args, int);
	  ast->func_call_exp.args_opt = va_arg(args, node*);
	  break;
  case CONSTRUCTOR_EXP:
	  ast->constructor_exp.type = va_arg(args, node*);
	  ast->constructor_exp.args_opt = va_arg(args, node*);
	  break;
  case UNARY_EXP:
	  ast->unary_exp.uopt = (enum unary_opt) va_arg(args, int);
	  ast->unary_exp.operand = va_arg(args, node*);
	  break;
  case BINARY_EXP:
	  ast->binary_exp.bopt = (enum binary_opt) va_arg(args, int);
	  ast->binary_exp.operand1 = va_arg(args, node*);
	  ast->binary_exp.operand2 = va_arg(args, node*);
	  break;
  case LITERAL_EXP:
	  ast->literal_exp.lit_type = (enum literal_type) va_arg(args, int);
	  switch (ast->literal_exp.lit_type) {
	  case LIT_BOOL:
		  ast->literal_exp.val_bool = va_arg(args, int);
		  break;
	  case LIT_INT:
		  ast->literal_exp.val_int = va_arg(args, int);
		  break;
	  case LIT_FLOAT:
		  ast->literal_exp.val_float = va_arg(args, double);
		  break;
	  default: break;
	  }
	  break;
  case VARIABLE:
	  ast->variable.identifier = va_arg(args, char*);
	  ast->variable.array_index = va_arg(args, int);
	  break;
  default: break;
  }

  va_end(args);

  return ast;
}

void ast_free(node *ast) {

}

void ast_print(node * ast) {

}
