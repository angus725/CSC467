
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
//struct node_;
//typedef struct node_ Node;
//extern node *ast;
class Node;
class Type;
class Expression;
class Variable;
extern Node *ast;


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

enum data_type
{
	TYPE_UNKNOWN = 0,
	TYPE_BOOL, // should be 1
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


Node* astAllocate(node_kind kind, ...); // calls each constructor
data_type var_type_to_base(data_type var_type);
char* type_to_str(data_type);


class Node
{

public:
	Node();
	virtual ~Node() {}; // do nothing
	virtual void printSyntaxTree() = 0;
	virtual int checkSemantic()=0;
	virtual int isWriteOnly() { return -1; }; //default to invalid return
	virtual int is_expression() { return 0; }; // default to no
	virtual int countParameters() { return 1; }; // only overloaded by multinode (arg list)
	virtual data_type getResultType() { return TYPE_INVALID; };
	virtual int isConst() { return constantValue; };
	int getLineNum() { return line_num; };

	static int nestedIfCount;
protected:
	int line_num;
	int constantValue;

private:

};

class Scope : public Node
{
public:
	Scope(va_list &args);
	~Scope();
	void printSyntaxTree();
	int checkSemantic();
	//int isWriteOnly(); // not sure what this should evaluate to. Default to -1
	//	data_type getResultType() { return TYPE_INVALID; };

private:
	Node* declarations;
	Node* statements;
};

class MultiNode : public Node
{
public:
	MultiNode(node_kind kind, va_list &args);
	~MultiNode();
	void printSyntaxTree();
	int checkSemantic();
	int isWriteOnly();
	data_type getResultType();
	int countParameters();

protected:
	node_kind kind;
	Node *nodes;
	Node *cur_node;
private:

};

class Declaration : public Node
{
public:
	Declaration(va_list &args);
	~Declaration();
	void printSyntaxTree();
	int checkSemantic();
	//int isWriteOnly(); // totally invalid call. makes no sense. returning -1 as per default
	//data_type getResultType() { return TYPE_INVALID; };
	int isConst() { return is_const; }; // special case
protected:
private:
	int is_const;
	std::string identifier;
	Type *type;
	Expression *expression;
	bool constantValue;
};

class IfStatement : public Node
{
public:
	IfStatement(va_list &args);
	~IfStatement();
	void printSyntaxTree();
	int checkSemantic();
	//int isWriteOnly(); // totally invalid call. makes no sense. returning -1 as per default
	//	data_type getResultType() { return TYPE_INVALID; };

private:

	Node *if_confition;
	Node *if_body;
	Node *else_body;
};

class AssignStatement : public Node
{
public:
	AssignStatement(va_list &args);
	~AssignStatement();
	void printSyntaxTree();
	int checkSemantic();
	//int isWriteOnly(); // totally invalid call. makes no sense. returning -1 as per default
	//	data_type getResultType() { return TYPE_INVALID; };

protected:

private:

	Variable *variable;
	Expression *expression;
	bool constantValue;
};

class Type : public Node
{
public:
	Type(va_list &args);
	~Type();
	void printSyntaxTree();
	int checkSemantic();
	//int isWriteOnly(); // totally invalid call. makes no sense. returning -1 as per default
	data_type getResultType() { return var_type; }; // not quite expressionResultType, but a type none the less


private:
	data_type var_type;
	int array_bound;
};

class Expression : public Node
{
public:
	Expression();
	virtual ~Expression() {};
	virtual void printSyntaxTree() = 0;
	virtual int checkSemantic() = 0;
	int is_expression() { return 1; };
protected:

private:

};

class Variable : public Expression
{
public:
	Variable(va_list &args);
	~Variable();
	void printSyntaxTree();
	int checkSemantic();
	int isWriteOnly();
	data_type getResultType() {return var_type;};
	int get_array_bound();
	std::string getID() { return identifier; };

private:
	std::string identifier;
	int array_index; // ie, the second 4 in vec4 vector[4]
	int has_index;
	int array_bound;	//Need to be filled from sematic analysis
	data_type var_type; //Need to be filled from sematic analysis
};

class FunctionCall : public Expression
{

public:
	FunctionCall(va_list &args);
	~FunctionCall();
	void printSyntaxTree();
	int checkSemantic();
	int isWriteOnly();
	data_type getResultType() { return result_type; };
	data_type CalcFuncResultType();

private:
	char *func_to_str(enum func_type type);
	func_type func;
	data_type result_type;
	Node *args_opt;
};

class Constructor : public Expression
{
public:
	Constructor(va_list &args);
	~Constructor();
	void printSyntaxTree();
	int checkSemantic();
	int isWriteOnly();
	data_type getResultType() { return type->getResultType(); };


private:
	Type *type;
	Node *args_opt;
};

class UnaryOP : public Expression
{
public:
	UnaryOP(va_list &args);
	~UnaryOP();
	void printSyntaxTree();
	int checkSemantic();
	int isWriteOnly();
	data_type getResultType() { return result_type; };


private:
	char *uopt_to_string(enum unary_opt opt);
	unary_opt uopt;
	Expression *operand;
	data_type result_type;
};

class BinaryOP : public Expression
{
public:
	BinaryOP(va_list &args);
	~BinaryOP();
	void printSyntaxTree();
	int checkSemantic();
	int isWriteOnly();
	data_type getResultType() { return result_type; };

private:
	char *bopt_to_string(enum binary_opt opt);
	binary_opt bopt;
	Expression *operand1;
	Expression *operand2;
	data_type result_type; //Need to be filled from semantic analysis
};


class LiteralExp : public Expression
{
public:
	LiteralExp(va_list &args);
	~LiteralExp();
	void printSyntaxTree();
	int checkSemantic();
	int isWriteOnly();
	data_type getResultType() { return lit_type; };

private:
	//static const bool constantValue = true; // might not be needed
	data_type lit_type; // used by symantic checker "getResultType"
	union {
		int val_bool;
		int val_int;
		double val_float;
	};
};


//
//namespace deprecated
//{
//
//	enum data_type //deprecated
//	{
//		TYPE_UNKNOWN = 0,
//		TYPE_BOOL,
//		TYPE_BVEC2,
//		TYPE_BVEC3,
//		TYPE_BVEC4,
//		TYPE_INT,
//		TYPE_IVEC2,
//		TYPE_IVEC3,
//		TYPE_IVEC4,
//		TYPE_FLOAT,
//		TYPE_VEC2,
//		TYPE_VEC3,
//		TYPE_VEC4,
//		TYPE_ANY, // for bypassing errors
//
//		TYPE_INVALID = -1,
//		WRITE_ONLY = -2,
//		NOT_FOUND = -3,
//	};
//
//	enum func_type // deprecated
//	{
//		FUNC_DP3 = 0,
//		FUNC_LIT = 1,
//		FUNC_RSQ = 2,
//		FUNC_ANY = 3, // for bypassing errors
//
//	};
//
//	enum unary_opt // deprecated
//	{
//		UOPT_NEG,
//		UOPT_NOT,
//	};
//
//	enum binary_opt // deprecated
//	{
//		BOPT_AND,
//		BOPT_OR,
//		BOPT_EXPO,
//		BOPT_EQ,
//		BOPT_NEQ,
//		BOPT_LT,
//		BOPT_LEQ,
//		BOPT_GT,
//		BOPT_GEQ,
//		BOPT_ADD,
//		BOPT_SUB,
//		BOPT_MUL,
//		BOPT_DIV,
//	};
//
//
//	struct node_ // deprecated
//	{
//
//		//  node_(): kind(UNKNOWN), line_num(0), constantValue(0), variable{} { };
//		// ~node_(){};
//
//		// an example of tagging each node with a type
//		node_kind kind;
//		int line_num;
//		int constantValue; // for symatic checker, don't worry about this during parsing.
//		union {
//			struct
//			{
//				node *declarations;
//				node *statements;
//			} scope;
//
//			struct
//			{
//				node *nodes;
//				node *cur_node;
//				// char *last_var_result_type; //Need to be filled from semantic analysis
//				// if the var types are not identical, it should be TYPE_ANY
//				// if there is no result type, it should be 0
//			} multi_node;
//
//			struct
//			{
//				int is_const;
//				node *type;
//				char *identifier;
//				node *expression;
//			} declaration;
//
//			struct
//			{
//				node *variable;
//				node *expression;
//			} assignment_statement;
//
//			struct
//			{
//				node *if_confition;
//				node *if_body;
//				node *else_body;
//			} if_statement;
//
//			struct
//			{
//				enum data_type var_type;
//				int array_bound;
//			} type;
//
//			struct
//			{
//				node *type;
//				node *args_opt;
//			} constructor_exp;
//
//			struct
//			{
//				enum func_type func;
//				enum data_type result_type;
//				node *args_opt;
//			} func_call_exp;
//
//			struct
//			{
//				enum unary_opt uopt;
//				node *operand;
//				enum data_type result_type;
//			} unary_exp;
//
//			struct
//			{
//				enum binary_opt bopt;
//				node *operand1;
//				node *operand2;
//				enum data_type result_type; //Need to be filled from semantic analysis
//			} binary_exp;
//
//			struct
//			{
//				enum data_type lit_type; // used by symantic checker "getExpressionResultType"
//				union {
//					int val_bool;
//					int val_int;
//					double val_float;
//				};
//			} literal_exp;
//
//			struct
//			{
//				char *identifier;
//				int array_index; // ie, the second 4 in vec4 vector[4]
//				int has_index;
//				int array_bound;	//Need to be filled from sematic analysis
//				enum data_type var_type; //Need to be filled from sematic analysis
//			} variable;
//		};
//	};

	//DEPRECATED node *ast_allocate(node_kind type, ...);
	//DEPRECATED void ast_free(node *ast);
	//DEPRECATED void ast_print(node *ast);
	//DEPRECATED void ast_traversal(
	//	node *ast,
	//	void(*pre_order_func)(node *N),
	//	void(*post_order_func)(node *N));
	//DEPRECATED int is_expression(node_kind kind);
	//DEPRECATED char *type_to_str(enum data_type type);
	//DEPRECATED enum data_type var_type_to_base(enum data_type var_type);

//}




#endif /* AST_H_ */
