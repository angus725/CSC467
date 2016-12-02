
#ifndef AST_H_
#define AST_H_ 1

#ifdef __GNUC__
#define DEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER)
#define DEPRECATED __declspec(deprecated)
#else
#pragma message("WARNING: You need to implement DEPRECATED for this compiler")
#define DEPRECATED
#endif

#include <cstdarg>
#include <string>
#include <iostream>
#include <stdint.h>



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

typedef enum { // deprecated
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

enum data_type //deprecated
{
	TYPE_UNKNOWN = 0,
	TYPE_BOOL,
	TYPE_BVEC2,
	TYPE_BVEC3,
	TYPE_BVEC4,
	TYPE_INT,
	TYPE_IVEC2,
	TYPE_IVEC3,
	TYPE_IVEC4,
	TYPE_FLOAT,
	TYPE_VEC2,
	TYPE_VEC3,
	TYPE_VEC4,
	TYPE_ANY, // for bypassing errors

	TYPE_INVALID = -1,
	WRITE_ONLY = -2,
	NOT_FOUND = -3,
};

enum func_type // deprecated
{
	FUNC_DP3 = 0,
	FUNC_LIT = 1,
	FUNC_RSQ = 2,
	FUNC_ANY = 3, // for bypassing errors

};

enum unary_opt // deprecated
{
	UOPT_NEG,
	UOPT_NOT,
};

enum binary_opt // deprecated
{
	BOPT_AND,
	BOPT_OR,
	BOPT_EXPO,
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


struct node_ // deprecated
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
			enum data_type var_type;
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
			enum data_type result_type;
			node *args_opt;
		} func_call_exp;

		struct
		{
			enum unary_opt uopt;
			node *operand;
			enum data_type result_type;
		} unary_exp;

		struct
		{
			enum binary_opt bopt;
			node *operand1;
			node *operand2;
			enum data_type result_type; //Need to be filled from semantic analysis
		} binary_exp;

		struct
		{
			enum data_type lit_type; // used by symantic checker "getExpressionResultType"
			union {
				int val_bool;
				int val_int;
				double val_float;
			};
		} literal_exp;

		struct
		{
			char *identifier;
			int array_index; // ie, the second 4 in vec4 vector[4]
			int has_index;
			int array_bound;	//Need to be filled from sematic analysis
			enum data_type var_type; //Need to be filled from sematic analysis
		} variable;
	};
};

DEPRECATED node *ast_allocate(node_kind type, ...);
DEPRECATED void ast_free(node *ast);
void ast_print(node *ast);
void ast_traversal(
	node *ast,
	void(*pre_order_func)(node *N),
	void(*post_order_func)(node *N));
int is_expression(node_kind kind);
char *type_to_str(enum data_type type);
enum data_type var_type_to_base(enum data_type var_type);




class Node
{


public:
	Node();
	virtual ~Node() {}; // do nothing
	int line_num;

protected:

	enum data_type
	{
		TYPE_UNKNOWN = 0,
		TYPE_BOOL,
		TYPE_BVEC2,
		TYPE_BVEC3,
		TYPE_BVEC4,
		TYPE_INT,
		TYPE_IVEC2,
		TYPE_IVEC3,
		TYPE_IVEC4,
		TYPE_FLOAT,
		TYPE_VEC2,
		TYPE_VEC3,
		TYPE_VEC4,
		TYPE_ANY, // for bypassing errors

		TYPE_INVALID = -1,
		WRITE_ONLY = -2,
		NOT_FOUND = -3,
	};

private:

};

class Scope : public Node
{
public:
	Scope(va_list vaList);
	~Scope();
	Node* declarations;
	Node* statements;
private:

};

class MultiNode : public Node
{
public:
	MultiNode(va_list vaList);
	~MultiNode();
	Node *nodes;
	Node *cur_node;
protected:
	bool constantValue;

private:

};

class Declaration : public Node
{
public:
	Declaration(va_list vaList);
	~Declaration();


	int is_const;
	std::string identifier;
	Node *type;
	Node *expression;
	bool constantValue;

protected:
private:

};

class IfStatement : public Node
{
public:
	IfStatement(va_list vaList);
	~IfStatement();
	Node *if_confition;
	Node *if_body;
	Node *else_body;
private:

};

class AssignStatement : public Node
{
public:
	AssignStatement(va_list vaList);
	~AssignStatement();
	Node *variable;
	Node *expression;
	bool constantValue;

protected:

private:

};

class Type : public Node
{
public:
	Type(va_list vaList);
	~Type();
	enum data_type var_type;
	int array_bound;
private:

};

class Expression : public Node
{
public:
	Expression();
	virtual ~Expression() {};
	virtual bool isConst() {
		return constantValue;
	};
	bool constantValue;
protected:


private:

};

class Variable : public Expression
{
public:
	Variable(va_list vaList);
	~Variable();
	std::string identifier;
	int array_index; // ie, the second 4 in vec4 vector[4]
	int has_index;
	int array_bound;	//Need to be filled from sematic analysis
	data_type var_type; //Need to be filled from sematic analysis
private:

};

class FunctionCall : public Expression
{

public:
	FunctionCall(va_list vaList);
	~FunctionCall();

	enum func_type
	{
		FUNC_DP3 = 0,
		FUNC_LIT = 1,
		FUNC_RSQ = 2,
		FUNC_ANY = 3, // for bypassing errors

	};
	func_type func;
	data_type result_type;
	Node *args_opt;
private:

};

class Constructor : public Expression
{
public:
	Constructor(va_list vaList);
	~Constructor();

private:
	Node *type;
	Node *args_opt;
};

class UnaryOP : public Expression
{
public:
	UnaryOP(va_list vaList);
	~UnaryOP();

	enum unary_opt
	{
		UOPT_NEG,
		UOPT_NOT,
	};
	unary_opt uopt;
	Node *operand;
	data_type result_type;
private:

};

class BinaryOP : public Expression
{
public:
	BinaryOP(va_list vaList);
	~BinaryOP();
	enum binary_opt
	{
		BOPT_AND,
		BOPT_OR,
		BOPT_EXPO,
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
	enum binary_opt bopt;
	Node *operand1;
	Node *operand2;
	enum data_type result_type; //Need to be filled from semantic analysis
private:

};


class LiteralExp : public Expression
{
public:
	LiteralExp(va_list vaList);
	~LiteralExp();
	//static const bool constantValue = true; // might not be needed
	enum data_type lit_type; // used by symantic checker "getExpressionResultType"
	union {
		int val_bool;
		int val_int;
		double val_float;
	};
private:

};


Node* astAllocate(node_kind kind, ...);






#endif /* AST_H_ */
