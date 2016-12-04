#include "ast.h"
#include "symbol.h"
#include "common.h"
#include <sstream>
#include <map>


#define DEBUG_CODEGEN 0
#define OUTPUT_ARB(...) {fprintf(outputFile,  __VA_ARGS__); if (DEBUG_CODEGEN) fprintf(errorFile,  __VA_ARGS__);}

std::unique_ptr<RegAllocator> reg_allocator(new RegAllocator);
std::map <std::string, Register*> varRegMap;
int Scope::genARB()
{
	if (this->declarations)
		this->declarations->genARB();
	if (this->statements)
		this->statements->genARB();
	return 0;
}


int MultiNode::genARB()
{
	if (this->nodes)
		this->nodes->genARB();

	if (this->cur_node)
		this->cur_node->genARB();

	return 0;

}


int Declaration::genARB()
{
	Register *result_reg = NULL;
	std::ostringstream oss;

	if (this->expression) {
		this->expression->genARB();
		result_reg = this->expression->reg;
	}

	if (!result_reg)
		result_reg = reg_allocator->getNewReg();

	varRegMap[this->identifier] = result_reg;
	/* TODO: store result_reg into the symbol table for this variable*/

	return 0;


}


int IfStatement::genARB()
{


	return 0;
}


int AssignStatement::genARB()
{
	std::ostringstream oss;

	this->variable->genARB();
	this->expression->genARB();

	oss << "MOV " << this->variable->reg->name << ", " << this->expression->reg->name;
	OUTPUT_ARB("%s\n", oss.str().c_str())
	return 0;

}

int Variable::genARB()
{
	this->reg = varRegMap[this->identifier];
	return 0;

}


int FunctionCall::genARB()
{


	return 0;
}


int Constructor::genARB()
{
//	switch (this->type->array_bound)

	return 0;
}


int UnaryOP::genARB()
{
	Register *result_reg;
	std::ostringstream oss;

	this->operand->genARB();

	result_reg = this->reclaimReg();
	if (!result_reg)
		result_reg = reg_allocator->getNewReg();

	switch (this->uopt) {
	case UOPT_NEG:
		/* -x is equivalent to SUB result_reg, 0, x*/
		oss << "SUB " << result_reg->name << ", " << "0, " << this->operand->reg->name;
		break;
	case UOPT_NOT:
		/* Logical not, since operand is either 1 or -1, this is just inversion,
		 * or 1's complement*/
		oss << "SUB " << result_reg->name << ", " << "1, " << this->operand->reg->name;
		break;
	default:
		break;
	}

	this->reg = result_reg;
	OUTPUT_ARB("%s\n", oss.str().c_str());

	return 0;

}


int BinaryOP::genARB()
{


	return 0;

}


int LiteralExp::genARB()
{
	Register *result_reg = reg_allocator->getNewReg();
	double val = 0.0;
	std::ostringstream oss;

	switch (this->lit_type) {
	case TYPE_BOOL:
		val = float(this->val_bool);	//Using -1 for true and 1 for false following CMP convention
		break;
	case TYPE_INT:
		val = float(this->val_int);
		break;
	case TYPE_FLOAT:
		val = float(this->val_bool);
		break;
	default:
		break;
	}

	this->reg = result_reg;

	oss << "MOV " << result_reg->name << ", " << val;

	OUTPUT_ARB("%s\n", oss.str().c_str())
	return 0;

}

int codeGen(ASTNode *ast) {
	ast->genARB();
	return 0;
}
