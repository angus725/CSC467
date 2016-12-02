#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string.h>

#include "ast.h"
#include "common.h"
#include "parser.tab.h"

#include "semantic.h"

#define DEBUG_PRINT_TREE 0

node *ast = NULL;


enum data_type var_type_to_base (enum data_type var_type) {
	if (var_type >= TYPE_INT && var_type <= TYPE_IVEC4)
		return TYPE_INT;
	if (var_type >= TYPE_BOOL && var_type <= TYPE_BVEC4)
		return TYPE_BOOL;
	if (var_type >= TYPE_FLOAT && var_type <= TYPE_VEC4)
		return TYPE_FLOAT;

	return TYPE_UNKNOWN;
}

node *ast_allocate(node_kind kind, ...)
{
    va_list args;

    // make the node
    node *ast = (node *)malloc(sizeof(node));
    memset(ast, 0, sizeof *ast);
    ast->kind = kind;

    va_start(args, kind);

    ast->line_num = va_arg(args, int);

    switch (kind)
    {
    case SCOPE:
        ast->scope.declarations = va_arg(args, node *);
        ast->scope.statements = va_arg(args, node *);
        break;
    case MULTI_NODE:
        ast->multi_node.nodes = va_arg(args, node *);
        ast->multi_node.cur_node = va_arg(args, node *);
        break;
    case DECLARATION:
        ast->declaration.is_const = va_arg(args, int);
        ast->declaration.type = va_arg(args, node *);
        ast->declaration.identifier = va_arg(args, char *);
        ast->declaration.expression = va_arg(args, node *);
        break;
    case ASSIGN_STATEMENT:
        ast->assignment_statement.variable = va_arg(args, node *);
        ast->assignment_statement.expression = va_arg(args, node *);
        break;
    case IF_STATEMENT:
        ast->if_statement.if_confition = va_arg(args, node *);
        ast->if_statement.if_body = va_arg(args, node *);
        ast->if_statement.else_body = va_arg(args, node *);
        break;
    case TYPE:
        ast->type.var_type = (enum data_type)va_arg(args, int);
        ast->type.array_bound = va_arg(args, int);
        break;
    case FUNC_CALL_EXP:
        ast->func_call_exp.func = (enum func_type)va_arg(args, int);
        ast->func_call_exp.args_opt = va_arg(args, node *);
        ast->func_call_exp.result_type = TYPE_UNKNOWN;
        break;
    case CONSTRUCTOR_EXP:
        ast->constructor_exp.type = va_arg(args, node *);
        ast->constructor_exp.args_opt = va_arg(args, node *);
        break;
    case UNARY_EXP:
        ast->unary_exp.uopt = (enum unary_opt)va_arg(args, int);
        ast->unary_exp.operand = va_arg(args, node *);
        break;
    case BINARY_EXP:
        ast->binary_exp.bopt = (enum binary_opt)va_arg(args, int);
        ast->binary_exp.operand1 = va_arg(args, node *);
        ast->binary_exp.operand2 = va_arg(args, node *);
        break;
    case LITERAL_EXP:
        ast->literal_exp.lit_type = (enum data_type)va_arg(args, int);
        switch (ast->literal_exp.lit_type)
        {
        case TYPE_BOOL:
            ast->literal_exp.val_bool = va_arg(args, int);

            break;
        case TYPE_INT:
            ast->literal_exp.val_int = va_arg(args, int);

            break;
        case TYPE_FLOAT:
            ast->literal_exp.val_float = va_arg(args, double);
            break;
        default:
            break;
        }
        break;
    case VARIABLE:
        ast->variable.identifier = va_arg(args, char *);
        ast->variable.has_index = va_arg(args, int);
        if (ast->variable.has_index)
            ast->variable.array_index = va_arg(args, int);
        break;
    default:
        break;
    }

    va_end(args);

    return ast;
}

void ast_traversal(
    node *ast,
    void (*pre_order_func)(node *N),
    void (*post_order_func)(node *N))
{
    if (!ast)
        return;

    if (pre_order_func)
        pre_order_func(ast);

    switch (ast->kind)
    {
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
    default:
        break;
    }

    if (post_order_func)
        post_order_func(ast);
}

static void free_node(node* N) {
	free(N);
}
void ast_free(node *ast)
{
	ast_traversal(ast, NULL, free_node);
}

static char *uopt_to_string(enum unary_opt opt)
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

static char *bopt_to_string(enum binary_opt opt)
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

char *type_to_str(enum data_type type)
{
	    switch(type)
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

static char *func_to_str(enum func_type type)
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

static void print_node_pre(node *ast)
{
    // std::string temp;
    switch (ast->kind)
    {
    case SCOPE:
        printf("(SCOPE ");
        break;
    case MULTI_NODE:
        switch (ast->multi_node.cur_node->kind)
        {
        case DECLARATION:
            printf("(DECLARATIONS ");
            break;
        case ASSIGN_STATEMENT:
        case IF_STATEMENT:
            printf("(STATEMENTS ");
            break;
        default:
            break;
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
        printf("%s ", type_to_str(ast->type.var_type));
        break;
    case FUNC_CALL_EXP:
        printf("(CALL %s ", func_to_str(ast->func_call_exp.func));
        break;
    case CONSTRUCTOR_EXP:
        printf("(CALL ");
        break;
    case UNARY_EXP:
        printf("(UNARY %s %s ", type_to_str(ast->unary_exp.result_type), uopt_to_string(ast->unary_exp.uopt));
        break;
    case BINARY_EXP:
        printf("(BINARY %s %s ", type_to_str(ast->binary_exp.result_type), bopt_to_string(ast->binary_exp.bopt));
        break;
    case LITERAL_EXP:
        switch (ast->literal_exp.lit_type)
        {
        case TYPE_BOOL:
            printf("%s ", (ast->literal_exp.val_bool) ? "true" : "false");
            break;
        case TYPE_INT:
            printf("%d ", ast->literal_exp.val_int);
            break;
        case TYPE_FLOAT:
            printf("%f ", ast->literal_exp.val_float);
            break;
        default:
            break;
        }
        break;
    case VARIABLE:
        if (ast->variable.has_index)
        {

            printf("(INDEX %s %s %d ", type_to_str(ast->variable.var_type),
                   ast->variable.identifier, ast->variable.array_index);
        }
        else
            printf("%s %s ", type_to_str(ast->variable.var_type), ast->variable.identifier);
        break;
    default:
        break;
    }
}

int is_expression(node_kind kind)
{
    if (kind > EXPRESSION_START && kind < EXPRESSION_END)
        return 1;
    return 0;
}
static void print_node_post(node *ast)
{

    switch (ast->kind)
    {
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
    default:
        break;
    }
    printf(")");
}
void ast_print(node *ast)
{
    ast_traversal(ast, print_node_pre, print_node_post);
    printf("\n");

}


Node::Node()
{
}

Node::~Node()
{
}

ostream& operator<<(ostream& os, const Node& node)
{
	return os;
}

Scope::Scope(...)
{
}

Scope::~Scope()
{
}

MultiNode::MultiNode(...)
{
}

MultiNode::~MultiNode()
{
}

Declaration::Declaration(...)
{
}

Declaration::~Declaration()
{
}

IfStatement::IfStatement(...)
{
}

IfStatement::~IfStatement()
{
}

AssignStatement::AssignStatement(...)
{
}

AssignStatement::~AssignStatement()
{
}


Type::Type(...)
{
}

Type::~Type()
{
}

Expression::Expression()
{
}

Expression::~Expression()
{
}

Variable::Variable(...)
{
}

Variable::~Variable()
{
}


FunctionCall::FunctionCall(...)
{
}

FunctionCall::~FunctionCall()
{
}

Constructor::Constructor(...)
{
}

Constructor::~Constructor()
{
}

UnaryOP::UnaryOP(...)
{
}

UnaryOP::~UnaryOP()
{
}

BinaryOP::BinaryOP(...)
{
}

BinaryOP::~BinaryOP()
{
}

LiteralExp::LiteralExp(...)
{
}

LiteralExp::~LiteralExp()
{
}