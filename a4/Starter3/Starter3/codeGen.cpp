#include "ast.h"
#include "symbol.h"
#include "common.h"
#include <sstream>
#include <map>
#include <iostream>

std::unique_ptr<RegAllocator> reg_allocator(new RegAllocator);
std::map <std::string, Register*> varRegMap;


std::map<std::string, ConditionalAsgnStmt> popAndMerge(Register conditionLTZero, std::map<std::string, ConditionalAsgnStmt>& A, std::map<std::string, ConditionalAsgnStmt> &B)
{
	return std::map<std::string, ConditionalAsgnStmt>();
}


class ConditionalAsgnStmt	// per variable
{
public:
	ConditionalAsgnStmt();
	~ConditionalAsgnStmt();
	ConditionalAsgnStmt(const ConditionalAsgnStmt &cpy){
		comparationInst_Before_Condition << cpy.comparationInst_Before_Condition.str();
		comparationInst_After_Condition << cpy.comparationInst_After_Condition.str();
		tempDestination = cpy.tempDestination;
	}

	//friend ostream& operator<<(ostream& os, const ConditionalAsgnStmt& dt);

	std::ostringstream comparationInst_Before_Condition;
	std::ostringstream comparationInst_After_Condition;

	//actual destination register is the map hash
	Register tempDestination;
private:
	ConditionalAsgnStmt merge(ConditionalAsgnStmt A, ConditionalAsgnStmt B);
	static int comparisonDepth;

};
int ConditionalAsgnStmt::comparisonDepth = 0;




//std::string ConditionalAsgnStmt::print()
//{
//
//}
ConditionalAsgnStmt::ConditionalAsgnStmt()
{
}

ConditionalAsgnStmt::~ConditionalAsgnStmt()
{
}




int Scope::genARB(std::map<std::string, ConditionalAsgnStmt>* ifTable)
{
	if (this->declarations)
		this->declarations->genARB(ifTable);
	if (this->statements)
		this->statements->genARB(ifTable);
	return 0;
}


int MultiNode::genARB(std::map<std::string, ConditionalAsgnStmt>* ifTable)
{
	if (this->cur_node)
		this->cur_node->genARB(ifTable);

	if (this->nodes)
		this->nodes->genARB(ifTable);



	return 0;

}


int Declaration::genARB(std::map<std::string, ConditionalAsgnStmt>* ifTable)
{
	Register *result_reg = NULL;
	std::ostringstream oss;

	if (this->expression) {
		this->expression->genARB(ifTable);
		result_reg = this->expression->reg;
	}

	if (!result_reg)
		result_reg = reg_allocator->getNewReg();

	varRegMap[this->identifier] = result_reg;
	/* TODO: store result_reg into the symbol table for this variable*/

	return 0;


}


int IfStatement::genARB(std::map<std::string, ConditionalAsgnStmt>* ifTable)
{
	std::map<std::string, ConditionalAsgnStmt> ifstatementResolutionTable; // create a new table for each if statement going downwards
	std::map<std::string, ConditionalAsgnStmt> elsestatementResolutionTable; // create a new table for each if statement going downwards


	if_confition->genARB(NULL);
	
	if_body->genARB(&ifstatementResolutionTable);

	if (else_body)
		else_body->genARB(&elsestatementResolutionTable);

	if (ifTable)
		*ifTable = popAndMerge(if_confition->reg, ifstatementResolutionTable, elsestatementResolutionTable);
	return 0;
}


int AssignStatement::genARB(std::map<std::string, ConditionalAsgnStmt>* ifTable)
{
	std::ostringstream oss;

	this->variable->genARB(ifTable);
	this->expression->genARB(ifTable); // TODO: map expression returns to variable register (temporary if ifTable exists)

	oss << "MOV " << this->variable->reg->name << ", " << this->expression->reg->name;
	// TODO: if( ifTable), don't print, but push it upwards into the if statement table.
	OUTPUT_ARB("%s;\n", oss.str().c_str())
	return 0;

}

int Variable::genARB(std::map<std::string, ConditionalAsgnStmt>* ifTable)
{
	//TODO ifTable exists, then allocate a temporary register instead of using the real one. set this as an entry to the ifTable, and update as required.
	Register *regCopy = new Register(varRegMap[this->identifier]);

	if (this->has_index) {
		regCopy->component = this->index_to_reg_component(this->array_index);
		regCopy->hasComponent = true;
		regCopy->name = regCopy->name + regCopy->component;
	}

	this->reg = regCopy;
	return 0;

}


int FunctionCall::genARB(std::map<std::string, ConditionalAsgnStmt>* ifTable)
{
	Register *result_reg;
	std::ostringstream oss;
	int i;
	Expression *arg1;
	Expression *arg2;
	int num_arg = this->args_opt ? this->args_opt->countParameters() : 0;


	for (i = 0; i < num_arg; i++) {
		this->args_opt->get_ith_node(i)->genARB(ifTable);
	}

	result_reg = this->reclaimReg();
	if (!result_reg)
		result_reg = reg_allocator->getNewReg();

	enum func_type
	{
		FUNC_DP3 = 0,
		FUNC_LIT = 1,
		FUNC_RSQ = 2,
		FUNC_ANY = 3, // for bypassing errors

	};

	switch (this->func) {
	case FUNC_DP3:
		arg1 = static_cast<Expression*>(this->args_opt->get_ith_node(0));
		arg2 = static_cast<Expression*>(this->args_opt->get_ith_node(1));
		oss << "DP3 " << result_reg->name << ", " << arg1->reg->name << ", " << arg2->reg->name << ";\n";
		break;
	case FUNC_LIT:
		arg1 = static_cast<Expression*>(this->args_opt->get_ith_node(0));
		oss << "LIT " << arg1->reg->name << ", " << arg1->reg->name << ";\n";
		if (result_reg != arg1->reg)
			oss << "MOV " << result_reg->name << ", " << arg1->reg->name << ";\n";
		break;
	case FUNC_RSQ:
		arg1 = static_cast<Expression*>(this->args_opt->get_ith_node(0));
		oss << "RSQ " << arg1->reg->name << ", " << arg1->reg->name << ";\n";
		if (result_reg != arg1->reg)
			oss << "MOV " << result_reg->name << ", " << arg1->reg->name << ";\n";
		break;
	default:
		return 0;
	}

	this->reg = result_reg;

	OUTPUT_ARB("%s", oss.str().c_str())
	return 0;
}


int Constructor::genARB(std::map<std::string, ConditionalAsgnStmt>* ifTable)
{
	Register *result_reg;
	std::ostringstream oss;
	int i;
	int array_bound = this->type->get_array_bound();

	for (i = 0; i <= array_bound; i++)
		this->args_opt->get_ith_node(i)->genARB(ifTable);

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


int UnaryOP::genARB(std::map<std::string, ConditionalAsgnStmt>* ifTable)
{
	Register *result_reg;
	std::ostringstream oss;

	this->operand->genARB(ifTable);

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


int BinaryOP::genARB(std::map<std::string, ConditionalAsgnStmt>* ifTable)
{
	Register *result_reg;
	std::ostringstream oss;

	this->operand1->genARB(ifTable);
	this->operand2->genARB(ifTable);

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


int LiteralExp::genARB(std::map<std::string, ConditionalAsgnStmt>* ifTable)
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
	ast->genARB(NULL);
	OUTPUT_ARB("END\n")
	return 0;
}
