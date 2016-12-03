#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>
#include <stdarg.h>



#include "ast.h"
#include "common.h"
#include "parser.tab.h"

#include "semantic.h"

#define DEBUG_PRINT_TREE 0

ASTNode *ast = NULL;

/*********************************************************************
***** 		all that new CPP stuff		******************************
**********************************************************************/

ASTNode::ASTNode()
{
	line_num = 0;
	constantValue = 0;
}

char * type_to_str(data_type type)
{
	switch (type)
	{
	case TYPE_INT:
		return "int";
		break;
	case TYPE_BOOL:
		return "bool";
		break;
	case TYPE_FLOAT:
		return "float";
		break;
	case TYPE_VEC2:
		return "vec2";
		break;
	case TYPE_VEC3:
		return "vec3";
		break;
	case TYPE_VEC4:
		return "vec4";
		break;
	case TYPE_IVEC2:
		return "ivec2";
		break;
	case TYPE_IVEC3:
		return "ivec3";
		break;
	case TYPE_IVEC4:
		return "ivec4";
		break;
	case TYPE_BVEC2:
		return "bvec2";
		break;
	case TYPE_BVEC3:
		return "bvec3";
		break;
	case TYPE_BVEC4:
		return "bvec4";
		break;
	case TYPE_ANY: // for bypassing errors
	case TYPE_INVALID:
	default:
		return "unknownType";
		break;
	}

}

data_type var_type_to_base(data_type var_type) {
	if (var_type >= TYPE_INT && var_type <= TYPE_IVEC4)
		return TYPE_INT;
	if (var_type >= TYPE_BOOL && var_type <= TYPE_BVEC4)
		return TYPE_BOOL;
	if (var_type >= TYPE_FLOAT && var_type <= TYPE_VEC4)
		return TYPE_FLOAT;

	return TYPE_UNKNOWN;
}

int isScalar(data_type dType) // output for type_Unknown and type_any is undefined
{
	if (dType == TYPE_ANY || (dType == var_type_to_base(dType)) || dType == TYPE_UNKNOWN)
		return 1;
	return 0;
}

Scope::Scope(va_list &args)
{
	line_num = va_arg(args, int);
	declarations = va_arg(args, ASTNode *);
	statements = va_arg(args, ASTNode *);
}

Scope::~Scope()
{
	if(declarations)
		delete declarations;
	if(statements)
		delete statements;
}

MultiNode::MultiNode(node_kind kind_, va_list &args)
{
	line_num = va_arg(args, int);
	nodes = va_arg(args, ASTNode *);
	cur_node = va_arg(args, ASTNode *);
	kind = kind_;
}

MultiNode::~MultiNode()
{
	if(nodes)
		delete nodes;
	if(cur_node)
		delete cur_node;
}

Declaration::Declaration(va_list &args)
{
	line_num = va_arg(args, int);
	is_const = va_arg(args, int);
	type = static_cast<Type*> (va_arg(args, ASTNode *));
	identifier = std::string(va_arg(args, char *));
	expression = static_cast<Expression*> (va_arg(args, ASTNode *));
}

Declaration::~Declaration()
{
	if(type)
		delete type;
	if(expression)
		delete expression;
}

IfStatement::IfStatement(va_list &args)
{
	line_num = va_arg(args, int);
	if_confition = va_arg(args, ASTNode *);
	if_body = va_arg(args, ASTNode *);
	else_body = va_arg(args, ASTNode *);
}

IfStatement::~IfStatement()
{
	if(if_confition)
		delete if_confition;
	if(if_body)
		delete if_body;
	if(else_body)
		delete else_body;
}

AssignStatement::AssignStatement(va_list &args)
{
	line_num = va_arg(args, int);
	variable = static_cast<Variable*> (va_arg(args, ASTNode *));
	expression = static_cast<Expression*> (va_arg(args, ASTNode *));
}

AssignStatement::~AssignStatement()
{
	if(variable)
		delete variable;
	if(expression)
		delete expression;
}


Type::Type(va_list &args)
{
	line_num = va_arg(args, int);
	var_type = (data_type)va_arg(args, int);
	array_bound = va_arg(args, int);
}

Type::~Type()
{
	// no children to delete
}

Expression::Expression()
{
	constantValue = false; // default false
}

Variable::Variable(va_list &args)
{
	line_num = va_arg(args, int);
	identifier = std::string(va_arg(args, char *));
	has_index = va_arg(args, int);
	if (has_index)
		array_index = va_arg(args, int);
}

Variable::~Variable()
{
	// no children to delete
}

FunctionCall::FunctionCall(va_list &args)
{
	line_num = va_arg(args, int);
	func = (enum func_type)va_arg(args, int);
	args_opt = va_arg(args, ASTNode *);
	result_type = TYPE_UNKNOWN;
}

FunctionCall::~FunctionCall()
{
	if(args_opt)
		delete args_opt;
}

char *FunctionCall::func_to_str(enum func_type type)
{
	switch (type)
	{
	case FUNC_DP3:
		return "DP3";
	case FUNC_LIT:
		return "LIT";
	case FUNC_RSQ:
		return "RSQ";
	default:
		return "";
	}
}

Constructor::Constructor(va_list &args)
{
	line_num = va_arg(args, int);
	type = static_cast<Type*>(va_arg(args, ASTNode *));
	args_opt = va_arg(args, ASTNode *);
}

Constructor::~Constructor()
{
	if(type)
		delete type;
	if(args_opt)
		delete args_opt;
}

UnaryOP::UnaryOP(va_list &args)
{
	line_num = va_arg(args, int);
	uopt = (enum unary_opt)va_arg(args, int);
	operand = static_cast<Expression*> (va_arg(args, ASTNode *));
}

UnaryOP::~UnaryOP()
{
	if(operand)
		delete operand;
}

char *UnaryOP::uopt_to_string(enum unary_opt opt)
{
	switch (opt)
	{
	case UOPT_NEG:
		return "-";
	case UOPT_NOT:
		return "!";
	default:
		return "";
	}
}

BinaryOP::BinaryOP(va_list &args)
{
	line_num = va_arg(args, int);
	bopt = (enum binary_opt)va_arg(args, int);
	operand1 = static_cast<Expression*>(va_arg(args, ASTNode *));
	operand2 = static_cast<Expression*>(va_arg(args, ASTNode *));

}

BinaryOP::~BinaryOP()
{
	if(operand1)
		delete operand1;
	if(operand2)
			delete operand2;

}

char *BinaryOP::bopt_to_string(enum binary_opt opt)
{
	switch (opt)
	{
	case BOPT_AND:
		return "&&";
	case BOPT_OR:
		return "||";
	case BOPT_EXPO:
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

LiteralExp::LiteralExp(va_list &args)
{
	line_num = va_arg(args, int);
	lit_type = (enum data_type)va_arg(args, int);
			switch (lit_type) {
			case TYPE_BOOL:
				val_bool = va_arg(args, int);

				break;
			case TYPE_INT:
				val_int = va_arg(args, int);

				break;
			case TYPE_FLOAT:
				val_float = va_arg(args, double);
				break;
			default:
				break;
			}
}

LiteralExp::~LiteralExp()
{
	// no children to delete
}

ASTNode* astAllocate(node_kind kind, ...) {
	va_list args;
	va_start(args, kind);

	ASTNode* pNode = NULL;
	//	ast->line_num = va_arg(args, int); // handled within each constructor.
	switch (kind) {
	case SCOPE:
		pNode = new Scope(args);
		break;
	case MULTI_NODE:
		pNode = new MultiNode(kind, args);
		break;
	case DECLARATION:
		pNode = new Declaration(args);
		break;
	case ASSIGN_STATEMENT:
		pNode = new AssignStatement(args);
		break;
	case IF_STATEMENT:
		pNode = new IfStatement(args);
		break;
	case TYPE:
		pNode = new Type(args);
		break;
	case FUNC_CALL_EXP:
		pNode = new FunctionCall(args);
		break;
	case CONSTRUCTOR_EXP:
		pNode = new Constructor(args);
		break;
	case UNARY_EXP:
		pNode = new UnaryOP(args);
		break;
	case BINARY_EXP:
		pNode = new BinaryOP(args);
		break;
	case LITERAL_EXP:
		pNode = new LiteralExp(args);
		break;
	case VARIABLE:
		pNode = new Variable(args);
		break;
	default:
		break;
	}

	va_end(args);

	return pNode;
}

#include "printAst.cpp"
#include "semanticCheck.cpp"
#include "createAndInsertIRNode.cpp"

/*********************************************************************
***** 		all that old C stuff		******************************
**********************************************************************/
//
//namespace deprecated
//{
//
//int is_expression(node_kind kind)
//{
//    if (kind > EXPRESSION_START && kind < EXPRESSION_END)
//        return 1;
//    return 0;
//}
//	node *ast_allocate(node_kind kind, ...)
//	{
//		va_list &args;
//
//		// make the node
//		node *ast = (node *)malloc(sizeof(node));
//		memset(ast, 0, sizeof *ast);
//		ast->kind = kind;
//
//		va_start(args, kind);
//
//		ast->line_num = va_arg(args, int);
//
//		switch (kind)
//		{
//		case SCOPE:
//			ast->scope.declarations = va_arg(args, node *);
//			ast->scope.statements = va_arg(args, node *);
//			break;
//		case MULTI_NODE:
//			ast->multi_node.nodes = va_arg(args, node *);
//			ast->multi_node.cur_node = va_arg(args, node *);
//			break;
//		case DECLARATION:
//
//			break;
//		case ASSIGN_STATEMENT:
//			ast->assignment_statement.variable = va_arg(args, node *);
//			ast->assignment_statement.expression = va_arg(args, node *);
//			break;
//		case IF_STATEMENT:
//			ast->if_statement.if_confition = va_arg(args, node *);
//			ast->if_statement.if_body = va_arg(args, node *);
//			ast->if_statement.else_body = va_arg(args, node *);
//			break;
//		case TYPE:
//			ast->type.var_type = (enum data_type)va_arg(args, int);
//			ast->type.array_bound = va_arg(args, int);
//			break;
//		case FUNC_CALL_EXP:
//			ast->func_call_exp.func = (enum func_type)va_arg(args, int);
//			ast->func_call_exp.args_opt = va_arg(args, node *);
//			ast->func_call_exp.result_type = TYPE_UNKNOWN;
//			break;
//		case CONSTRUCTOR_EXP:
//			ast->constructor_exp.type = va_arg(args, node *);
//			ast->constructor_exp.args_opt = va_arg(args, node *);
//			break;
//		case UNARY_EXP:
//			ast->unary_exp.uopt = (enum unary_opt)va_arg(args, int);
//			ast->unary_exp.operand = va_arg(args, node *);
//			break;
//		case BINARY_EXP:
//			ast->binary_exp.bopt = (enum binary_opt)va_arg(args, int);
//			ast->binary_exp.operand1 = va_arg(args, node *);
//			ast->binary_exp.operand2 = va_arg(args, node *);
//			break;
//		case LITERAL_EXP:
//			ast->literal_exp.lit_type = (enum data_type)va_arg(args, int);
//			switch (ast->literal_exp.lit_type)
//			{
//			case TYPE_BOOL:
//				ast->literal_exp.val_bool = va_arg(args, int);
//
//				break;
//			case TYPE_INT:
//				ast->literal_exp.val_int = va_arg(args, int);
//
//				break;
//			case TYPE_FLOAT:
//				ast->literal_exp.val_float = va_arg(args, double);
//				break;
//			default:
//				break;
//			}
//			break;
//		case VARIABLE:
//			ast->variable.identifier = va_arg(args, char *);
//			ast->variable.has_index = va_arg(args, int);
//			if (ast->variable.has_index)
//				ast->variable.array_index = va_arg(args, int);
//			break;
//		default:
//			break;
//		}
//
//		va_end(args);
//
//		return ast;
//	}
//
//	void ast_traversal(
//		node *ast,
//		void(*pre_order_func)(node *N),
//		void(*post_order_func)(node *N))
//	{
//		if (!ast)
//			return;
//
//		if (pre_order_func)
//			pre_order_func(ast);
//
//		switch (ast->kind)
//		{
//		case SCOPE:
//			ast_traversal(ast->scope.declarations, pre_order_func, post_order_func);
//			ast_traversal(ast->scope.statements, pre_order_func, post_order_func);
//			break;
//		case MULTI_NODE:
//			ast_traversal(ast->multi_node.cur_node, pre_order_func, post_order_func);
//			ast_traversal(ast->multi_node.nodes, pre_order_func, post_order_func);
//			break;
//		case DECLARATION:
//			ast_traversal(ast->declaration.type, pre_order_func, post_order_func);
//			ast_traversal(ast->declaration.expression, pre_order_func, post_order_func);
//			break;
//		case ASSIGN_STATEMENT:
//			ast_traversal(ast->assignment_statement.variable, pre_order_func, post_order_func);
//			ast_traversal(ast->assignment_statement.expression, pre_order_func, post_order_func);
//			break;
//		case IF_STATEMENT:
//			ast_traversal(ast->if_statement.if_confition, pre_order_func, post_order_func);
//			ast_traversal(ast->if_statement.if_body, pre_order_func, post_order_func);
//			ast_traversal(ast->if_statement.else_body, pre_order_func, post_order_func);
//			break;
//		case TYPE:
//			break;
//		case FUNC_CALL_EXP:
//			ast_traversal(ast->func_call_exp.args_opt, pre_order_func, post_order_func);
//			break;
//		case CONSTRUCTOR_EXP:
//			ast_traversal(ast->constructor_exp.type, pre_order_func, post_order_func);
//			ast_traversal(ast->constructor_exp.args_opt, pre_order_func, post_order_func);
//			break;
//		case UNARY_EXP:
//			ast_traversal(ast->unary_exp.operand, pre_order_func, post_order_func);
//			break;
//		case BINARY_EXP:
//			ast_traversal(ast->binary_exp.operand1, pre_order_func, post_order_func);
//			ast_traversal(ast->binary_exp.operand2, pre_order_func, post_order_func);
//			break;
//		case LITERAL_EXP:
//			break;
//		case VARIABLE:
//			break;
//		default:
//			break;
//		}
//
//		if (post_order_func)
//			post_order_func(ast);
//	}
//
//	static void free_node(node* N) {
//		free(N);
//	}
//	void ast_free(node *ast)
//	{
//		ast_traversal(ast, NULL, free_node);
//	}
//
//
//static void print_node_pre(node *ast)
//{
//	// std::string temp;
//	switch (ast->kind)
//	{
//	case SCOPE:
//		printf("(SCOPE ");
//		break;
//	case MULTI_NODE:
//		switch (ast->multi_node.cur_node->kind)
//		{
//		case DECLARATION:
//			printf("(DECLARATIONS ");
//			break;
//		case ASSIGN_STATEMENT:
//		case IF_STATEMENT:
//			printf("(STATEMENTS ");
//			break;
//		default:
//			break;
//		}
//		break;
//	case DECLARATION:
//		printf("(DECLARATION %s ", ast->declaration.identifier);
//		break;
//	case ASSIGN_STATEMENT:
//		printf("(ASSIGN ");
//		break;
//	case IF_STATEMENT:
//		printf("(IF ");
//		break;
//	case TYPE:
//		printf("%s ", type_to_str(ast->type.var_type));
//		break;
//	case FUNC_CALL_EXP:
//		printf("(CALL %s ", func_to_str(ast->func_call_exp.func));
//		break;
//	case CONSTRUCTOR_EXP:
//		printf("(CALL ");
//		break;
//	case UNARY_EXP:
//		printf("(UNARY %s %s ", type_to_str(ast->unary_exp.result_type), uopt_to_string(ast->unary_exp.uopt));
//		break;
//	case BINARY_EXP:
//		printf("(BINARY %s %s ", type_to_str(ast->binary_exp.result_type), bopt_to_string(ast->binary_exp.bopt));
//		break;
//	case LITERAL_EXP:
//		switch (ast->literal_exp.lit_type)
//		{
//		case TYPE_BOOL:
//			printf("%s ", (ast->literal_exp.val_bool) ? "true" : "false");
//			break;
//		case TYPE_INT:
//			printf("%d ", ast->literal_exp.val_int);
//			break;
//		case TYPE_FLOAT:
//			printf("%f ", ast->literal_exp.val_float);
//			break;
//		default:
//			break;
//		}
//		break;
//	case VARIABLE:
//		if (ast->variable.has_index)
//		{
//
//			printf("(INDEX %s %s %d ", type_to_str(ast->variable.var_type),
//				ast->variable.identifier, ast->variable.array_index);
//		}
//		else
//			printf("%s %s ", type_to_str(ast->variable.var_type), ast->variable.identifier);
//		break;
//	default:
//		break;
//	}
//}
//
//}
//static void print_node_post(node *ast)
//{
//
//	switch (ast->kind)
//	{
//	case TYPE:
//		return;
//	case LITERAL_EXP:
//		return;
//	case MULTI_NODE:
//		if (is_expression(ast->multi_node.cur_node->kind))
//			return;
//		break;
//	case VARIABLE:
//		if (!ast->variable.has_index)
//			return;
//		break;
//	default:
//		break;
//	}
//	printf(")");
//}
//void ast_print(node *ast)
//{
//	ast_traversal(ast, print_node_pre, print_node_post);
//	printf("\n");
//
//}