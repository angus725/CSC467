
#ifndef AST_H_
#define AST_H_ 1

#include <stdarg.h>

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

typedef enum {
  UNKNOWN,
  SCOPE,
  MULTI_NODE,
  DECLARATION,
  IF_STATEMENT,
  ASSIGN_STATEMENT,
  TYPE,
  VARIABLE,
  FUNC_CALL_EXP,
  CONSTRUCTOR_EXP,
  UNARY_EXP,
  BINARY_EXP,
  LITERAL_EXP,
  ARGUMENTS,
  ARGUMENTS_OPT
} node_kind;

enum var_type {
	TYPE_INT,
	TYPE_BOOL,
	TYPE_FLOAT,
};

enum func_type{
  FUNC_DP3 = 0,
  FUNC_LIT = 1,
  FUNC_RSQ = 2
};

enum unary_opt {
	UOPT_NEG,
	UOPT_NOT,
};

enum binary_opt {
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

enum literal_type {
	LIT_BOOL,
	LIT_INT,
	LIT_FLOAT
};

struct node_ {

  // an example of tagging each node with a type
  node_kind kind;
  int line_num;
  union {
    struct {
    	node *declarations;
    	node *statements;
    } scope;
  
    struct {
		node *nodes;
		node *cur_node;
    } multi_node;

    struct {
    	int is_const;
		node *type;
		char *identifier;
		node *expression;
    } declaration;

	struct{
		node* variable;
		node* expression;
	} assignment_statement;

	struct{
		node* if_confition;
		node* if_body;
		node* else_body;
	} if_statement;

	struct{
	   enum var_type var_type;
	   int array_len;
	} type;

	struct{
		node* type;
		node* args_opt;
	} constructor_exp;

	struct{
		enum func_type func;
		node* args_opt;
	} func_call_exp;


	struct {
		enum unary_opt uopt;
		node *operand;
	} unary_exp;

	struct {
		enum binary_opt bopt;
		node *operand1;
		node *operand2;
	} binary_exp;

	struct{
		enum literal_type lit_type;
		union {
			int val_bool;
			int val_int;
			double val_float;
		};
	} literal_exp;

	struct{
			char* identifier;
			int array_index;
	} variable;
  };
};

node *ast_allocate(node_kind type, ...);
void ast_free(node *ast);
void ast_print(node * ast);

#endif /* AST_H_ */
