#include "ast.h"
#include "symbol.h"
#include "common.h"
#include <sstream>
#include <map>
#include <iostream>

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
	if (this->cur_node)
		this->cur_node->genARB();

	if (this->nodes)
		this->nodes->genARB();



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
	OUTPUT_ARB("%s;\n", oss.str().c_str())
	return 0;

}

int Variable::genARB()
{
	Register *regCopy = new Register(varRegMap[this->identifier]);

	if (this->has_index) {
		regCopy->component = this->index_to_reg_component(this->array_index);
		regCopy->hasComponent = true;
		regCopy->name = regCopy->name + regCopy->component;
	}

	this->reg = regCopy;
	return 0;

}


int FunctionCall::genARB()
{


	return 0;
}


int Constructor::genARB()
{
	Register *result_reg;
	std::ostringstream oss;
	int i;
	int array_bound = this->type->get_array_bound();

	for (i = 0; i <= array_bound; i++)
		this->args_opt->get_ith_node(i)->genARB();

	result_reg = this->reclaimReg();
	if (!result_reg)
		result_reg = reg_allocator->getNewReg();

	switch (array_bound) {
	case (3):
		oss << "MOV " << result_reg->name << ".w, "
			<< static_cast <Expression *>(this->args_opt->get_ith_node(3))->reg->name << ";\n";
	case (2):
		oss << "MOV " << result_reg->name << ".z, "
			<< static_cast <Expression *>(this->args_opt->get_ith_node(2))->reg->name << ";\n";
	case (1):
		oss << "MOV " << result_reg->name << ".y, "
			<< static_cast <Expression *>(this->args_opt->get_ith_node(1))->reg->name << ";\n";
	default:
		oss << "MOV " << result_reg->name << ".x, "
					<< static_cast <Expression *>(this->args_opt->get_ith_node(0))->reg->name << ";\n";
	}

	this->reg = result_reg;
	OUTPUT_ARB("%s", oss.str().c_str())


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
	OUTPUT_ARB("%s;\n", oss.str().c_str());

	return 0;

}


int BinaryOP::genARB()
{
	Register *result_reg;
	std::ostringstream oss;

	this->operand1->genARB();
	this->operand2->genARB();

	result_reg = this->reclaimReg();
	if (!result_reg)
		result_reg = reg_allocator->getNewReg();

	/*TODO: finish this*/

	switch (this->bopt)
	{
	case BOPT_AND:
		return 0;
	case BOPT_OR:
		return 0;
	case BOPT_EXPO:
		return 0;
	case BOPT_EQ:
		return 0;
	case BOPT_NEQ:
		return 0;
	case BOPT_LT:
		return 0;
	case BOPT_LEQ:
		return 0;
	case BOPT_GT:
		return 0;
	case BOPT_GEQ:
		return 0;
	case BOPT_ADD:
		oss << "ADD " << result_reg->name << ", " << this->operand1->reg->name << ", "
			<< this->operand2->reg->name << ";\n";
		break;
	case BOPT_SUB:
		oss << "SUB " << result_reg->name << ", " << this->operand1->reg->name << ", "
			<< this->operand2->reg->name << ";\n";
		break;
	case BOPT_MUL:
		oss << "MUL " << result_reg->name << ", " << this->operand1->reg->name << ", "
			<< this->operand2->reg->name << ";\n";
		break;
	case BOPT_DIV:
		oss << "RCP " << this->operand2->reg->name << ", " << this->operand2->reg->name << ";\n";
		oss << "MUL " << result_reg->name << ", " << this->operand1->reg->name << ", "
					<< this->operand2->reg->name << ";\n";
		break;
	default:
		return 0;
	}

	this->reg = result_reg;
	OUTPUT_ARB("%s", oss.str().c_str());


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
		val = float(this->val_float);
		break;
	default:
		break;
	}

	this->reg = result_reg;

	oss << "MOV " << result_reg->name << ", " << val;

	OUTPUT_ARB("%s;\n", oss.str().c_str())
	return 0;

}

int codeGen(ASTNode *ast) {
	std::ostringstream oss;
	OUTPUT_ARB("!!ARBfp1.0\n")
	ast->genARB();
	OUTPUT_ARB("END\n")
	return 0;
}
