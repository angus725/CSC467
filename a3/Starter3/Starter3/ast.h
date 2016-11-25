
#ifndef AST_H_
#define AST_H_ 1

#include <stdarg.h>
#include <string>
#include <stdint.h>
// #include "semantic.h"

// Dummy node just so everything compiles, create your own node/nodes
//
// The code provided below is an example ONLY. You can use/modify it,
// but do not assume that it is correct or complete.
//
// There are many ways of making AST nodes. The approach below is an example
// of a descriminated union. If you choose to use C++, then I suggest looking
// into inheritance.

// forward declare
struct node_;
typedef struct node_ node;
extern node *ast;

#define RESULT 0x1
#define ATTRIBUTE 0x2
#define UNIFORM 0x4
#define INITIALIZED 0x8

typedef enum {
    UNKNOWN,
    SCOPE,
    MULTI_NODE,
    DECLARATION,
    IF_STATEMENT,
    ASSIGN_STATEMENT,
    TYPE,
    EXPRESSION_START,
    VARIABLE,
    FUNC_CALL_EXP,
    CONSTRUCTOR_EXP,
    UNARY_EXP,
    BINARY_EXP,
    LITERAL_EXP,
    EXPRESSION_END,
    ARGUMENTS,
    ARGUMENTS_OPT
} node_kind;

enum Var_type
{
    NONE = 0,
    TYPE_INT,
    TYPE_BOOL,
    TYPE_FLOAT,
    TYPE_VEC2,
    TYPE_VEC3,
    TYPE_VEC4,
    TYPE_VINT2,
    TYPE_VINT3,
    TYPE_VINT4,
    TYPE_VBOOL2,
    TYPE_VBOOL3,
    TYPE_VBOOL4,
    TYPE_ANY, // for bypassing errors

    INVALID = -1
};

enum func_type
{
    FUNC_DP3 = 0,
    FUNC_LIT = 1,
    FUNC_RSQ = 2,
    FUNC_ANY = 3, // for bypassing errors

};

enum unary_opt
{
    UOPT_NEG,
    UOPT_NOT,
};

enum binary_opt
{
    BOPT_AND,
    BOPT_OR,
    BOPT_XOR,
    BOPT_EQ,
    BOPT_NEQ,
    BOPT_LT,
    BOPT_LEQ,
    BOPT_GT,
    BOPT_GEQ,
    BOPT_ADD,
    BOPT_SUB,
    BOPT_MUL,
    BOPT_DIV,
};

enum literal_type
{
    LIT_BOOL,
    LIT_INT,
    LIT_FLOAT,
    LIT_ANY, // for bypassing errors

};

void varTypeToText(Var_type var_type, std::string &result);


struct node_
{

    //  node_(): kind(UNKNOWN), line_num(0), constantValue(0), variable{} { };
    // ~node_(){};

    // an example of tagging each node with a type
    node_kind kind;
    int line_num;
    int constantValue; // for symatic checker, don't worry about this during parsing.
    union {
	struct
	{
	    node *declarations;
	    node *statements;
	} scope;

	struct
	{
	    node *nodes;
	    node *cur_node;
	    // char *last_var_result_type; //Need to be filled from semantic analysis
	    // if the var types are not identical, it should be TYPE_ANY
	    // if there is no result type, it should be 0
	} multi_node;

	struct
	{
	    int is_const;
	    node *type;
	    char *identifier;
	    node *expression;
	} declaration;

	struct
	{
	    node *variable;
	    node *expression;
	} assignment_statement;

	struct
	{
	    node *if_confition;
	    node *if_body;
	    node *else_body;
	} if_statement;

	struct
	{
	    enum Var_type var_type;
	    int array_bound;
	} type;

	struct
	{
	    node *type;
	    node *args_opt;
	} constructor_exp;

	struct
	{
	    enum func_type func;
	    node *args_opt;
	} func_call_exp;

	struct
	{
	    enum unary_opt uopt;
	    node *operand;
	    char *result_type;
	} unary_exp;

	struct
	{
	    enum binary_opt bopt;
	    node *operand1;
	    node *operand2;
	    char *result_type; //Need to be filled from semantic analysis
	} binary_exp;

	struct
	{
	    enum literal_type lit_type;
	    union {
		int val_bool;
		int val_int;
		double val_float;
	    };
	} literal_exp;

	struct
	{
	    char *identifier;
	    int array_index; // ie, the 4 in vec4 vector[4]
	    int has_index;
	    int8_t attribute;       //Need to be filled from sematic analysis
	    enum Var_type var_type; //Need to be filled from sematic analysis
	} variable;
    };
};

node *ast_allocate(node_kind type, ...);
void ast_free(node *ast);
void ast_print(node *ast);
void ast_traversal(
    node *ast,
    void (*pre_order_func)(node *N),
    void (*post_order_func)(node *N));



#endif /* AST_H_ */
