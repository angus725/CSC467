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
	  ast->type.array_bound = va_arg(args, int);
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
	  ast->variable.has_index = va_arg(args, int);
	  if (ast->variable.has_index)
		  ast->variable.array_index = va_arg(args, int);
	  break;
  default: break;
  }

  va_end(args);

  return ast;
}


static void ast_traversal (
		node *ast,
		void (*pre_order_func)(node *N),
		void (*post_order_func)(node *N))
{
	if (!ast)
		return;

	if (pre_order_func)
		pre_order_func(ast);

	  switch(ast->kind) {
	  case SCOPE:
		  ast_traversal(ast->scope.declarations, pre_order_func, post_order_func);
		  ast_traversal(ast->scope.statements, pre_order_func, post_order_func);
		  break;
	  case MULTI_NODE:
		  ast_traversal(ast->multi_node.cur_node, pre_order_func, post_order_func);
		  ast_traversal(ast->multi_node.nodes, pre_order_func, post_order_func);
		  break;
	  case DECLARATION:
		  ast_traversal(ast->declaration.type, pre_order_func, post_order_func);
		  ast_traversal(ast->declaration.expression, pre_order_func, post_order_func);
		  break;
	  case ASSIGN_STATEMENT:
		  ast_traversal(ast->assignment_statement.variable, pre_order_func, post_order_func);
		  ast_traversal(ast->assignment_statement.expression, pre_order_func, post_order_func);
		  break;
	  case IF_STATEMENT:
		  ast_traversal(ast->if_statement.if_confition, pre_order_func, post_order_func);
		  ast_traversal(ast->if_statement.if_body, pre_order_func, post_order_func);
		  ast_traversal(ast->if_statement.else_body, pre_order_func, post_order_func);
		  break;
	  case TYPE:
		  break;
	  case FUNC_CALL_EXP:
		  ast_traversal(ast->func_call_exp.args_opt, pre_order_func, post_order_func);
		  break;
	  case CONSTRUCTOR_EXP:
		  ast_traversal(ast->constructor_exp.type, pre_order_func, post_order_func);
		  ast_traversal(ast->constructor_exp.args_opt, pre_order_func, post_order_func);
		  break;
	  case UNARY_EXP:
		  ast_traversal(ast->unary_exp.operand, pre_order_func, post_order_func);
		  break;
	  case BINARY_EXP:
		  ast_traversal(ast->binary_exp.operand1, pre_order_func, post_order_func);
		  ast_traversal(ast->binary_exp.operand2, pre_order_func, post_order_func);
		  break;
	  case LITERAL_EXP:
		  break;
	  case VARIABLE:
		  break;
	  default: break;
	  }

	if (post_order_func)
		post_order_func(ast);
}
void ast_free(node *ast) {

}

static char *uopt_to_string(enum unary_opt opt)
{
	switch (opt) {
	case UOPT_NEG:
		return "-";
	case UOPT_NOT:
		return "!";
	default:
		return "";
	}
}

static char *bopt_to_string(enum binary_opt opt)
{
	switch (opt) {
	case BOPT_AND:
		return "&&";
	case BOPT_OR:
		return "||";
	case BOPT_XOR:
		return "^";
	case BOPT_EQ:
		return "==";
	case BOPT_NEQ:
		return "!=";
	case BOPT_LT:
		return "<";
	case BOPT_LEQ:
		return "<=";
	case BOPT_GT:
		return ">";
	case BOPT_GEQ:
		return ">=";
	case BOPT_ADD:
		return "+";
	case BOPT_SUB:
		return "-";
	case BOPT_MUL:
		return "*";
	case BOPT_DIV:
		return "/";
	default:
		return "";
	}
}

static char *type_to_str(enum var_type type, int array_bound) {
	switch (type) {
	case TYPE_INT:
		if (array_bound)
			return "ivec";
		else
			return "int";
	case TYPE_BOOL:
		if (array_bound)
			return "bvec";
		else
			return "bool";
	case TYPE_FLOAT:
		if (array_bound)
			return "vec";
		else
			return "float";
	default: return "";
	}
}

static char*func_to_str(enum func_type type)
{
	switch (type) {
	case FUNC_DP3:
		return "DP3";
	case FUNC_LIT:
		return "LIT";
	case FUNC_RSQ:
		return "RSQ";
	default: return "";

	}
}

static void print_node_pre(node *ast)
{
	  switch(ast->kind) {
	  case SCOPE:
		  printf("(SCOPE ");
		  break;
	  case MULTI_NODE:
		  switch (ast->multi_node.cur_node->kind) {
		  case DECLARATION:
			  printf("(DECLARATIONS ");
			  break;
		  case ASSIGN_STATEMENT:
		  case IF_STATEMENT:
			  printf("(STATEMENTS ");
			  break;
		  default: break;
		  }
		  break;
	  case DECLARATION:
		  printf("(DECLARATION %s ", ast->declaration.identifier);
		  break;
	  case ASSIGN_STATEMENT:
		  printf("(ASSIGN ");
		  break;
	  case IF_STATEMENT:
		  printf("(IF ");
		  break;
	  case TYPE:
		  printf("%s%c ", type_to_str(ast->type.var_type, ast->type.array_bound),
				  (ast->type.array_bound) ? ast->type.array_bound + '1' : ' ');
		  break;
	  case FUNC_CALL_EXP:
		  printf("(CALL %s ", func_to_str(ast->func_call_exp.func));
		  break;
	  case CONSTRUCTOR_EXP:
		  printf("(CALL ");
		  break;
	  case UNARY_EXP:
		  printf("(UNARY %s %s ", ast->unary_exp.result_type, uopt_to_string(ast->unary_exp.uopt));
		  break;
	  case BINARY_EXP:
		  printf("(BINARY %s %s ", ast->binary_exp.result_type, bopt_to_string(ast->binary_exp.bopt));
		  break;
	  case LITERAL_EXP:
		  switch (ast->literal_exp.lit_type) {
		  case LIT_BOOL:
			  printf("%s ", (ast->literal_exp.val_bool)?"true":"false");
			  break;
		  case LIT_INT:
			  printf("%d ", ast->literal_exp.val_int);
			  break;
		  case LIT_FLOAT:
			  printf("%f ", ast->literal_exp.val_float);
			  break;
		  default: break;
		  }
		  break;
	  case VARIABLE:
		  if (ast->variable.has_index) {
			  printf("(INDEX %s %s %d ", ast->variable.var_type,
					  ast->variable.identifier, ast->variable.array_index);
		  } else
			  printf("%s %s ", ast->variable.var_type, ast->variable.identifier);
		  break;
	  default: break;
	  }
}

static int is_expression(node_kind kind) {
	if (kind > EXPRESSION_START && kind < EXPRESSION_END)
		return 1;
	return 0;
}
static void print_node_post(node *ast) {
	if (is_expression(ast->kind))
		return;
	  switch(ast->kind) {
	  case TYPE:
		  return;
	  case LITERAL_EXP:
		  return;
	  case MULTI_NODE:
		  if (is_expression(ast->multi_node.cur_node->kind))
		  		return;
		  break;
	  case VARIABLE:
		  if (!ast->variable.has_index)
			  return;
		  break;
	  default: break;
	  }
	printf(")");
}
void ast_print(node * ast) {
	ast_traversal(ast, print_node_pre, print_node_post);
}
