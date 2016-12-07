#include "ast.h"
#include "symbol.h"
#include "common.h"
#include <sstream>
#include <map>
#include <iostream>

std::unique_ptr<RegAllocator> reg_allocator(new RegAllocator);
std::map <std::string, Register*> varRegMap;


class ConditionalAsgnStmt	// per variable
{
public:
	ConditionalAsgnStmt(){};
	~ConditionalAsgnStmt(){};

	Register realDestination;
	Register tempDestination;
	static void merge(Register& conditionLTZero, ConditionalAsgnStmt& A, ConditionalAsgnStmt& B, std::map<std::string, ConditionalAsgnStmt>* ifTable);

private:
	static int comparisonDepth;
};
int ConditionalAsgnStmt::comparisonDepth = 0;

//ifValue.realDestination must be equal to elseValue.realDestination, unless one does not exist
void ConditionalAsgnStmt::merge(Register& conditionLTZero, ConditionalAsgnStmt& ifValue, ConditionalAsgnStmt& elseValue, std::map<std::string, ConditionalAsgnStmt>* ifTable){
	std::ostringstream oss;

	if (ifValue.tempDestination.id == -1 && elseValue.tempDestination.id == -1)
		return; // nothing done, since we're merging garbage
	if (elseValue.tempDestination.id == -1) // only ifValue exists
	{
		if (ifTable)
		{
			// if( ifTable), push it upwards into the if statement table.
			ConditionalAsgnStmt &temp = (*ifTable)[ifValue.realDestination.name];

			if (temp.tempDestination.id == -1)
			{
				temp.realDestination = ifValue.realDestination;
				temp.tempDestination = reg_allocator->getNewReg();
			}
			// CMP result, condition, trueValue(if), falseValue(else)
			oss << "CMP " << temp.tempDestination.name << ", " << conditionLTZero.name << ", " << ifValue.tempDestination.name << ", " << ifValue.realDestination.name << ";\n";
			OUTPUT_ARB("%s", oss.str().c_str());
		}
		else
		{
			oss << "CMP " << ifValue.realDestination.name << ", " << conditionLTZero.name << ", " << ifValue.tempDestination.name << ", " << ifValue.realDestination.name << ";\n";
			OUTPUT_ARB("%s", oss.str().c_str());
		}
		return;
	}


	if (ifValue.tempDestination.id == -1) // only else exists
	{
		if (ifTable)
		{
			// if( ifTable), push it upwards into the if statement table.
			ConditionalAsgnStmt &temp = (*ifTable)[elseValue.realDestination.name];

			if (temp.tempDestination.id == -1)
			{
				temp.realDestination = elseValue.realDestination;
				temp.tempDestination = reg_allocator->getNewReg();
			}
			// CMP result, condition, trueValue(if), falseValue(else)
			oss << "CMP " << temp.tempDestination.name << ", " << conditionLTZero.name << ", " << elseValue.realDestination.name << ", " << elseValue.tempDestination.name << ";\n";
			OUTPUT_ARB("%s", oss.str().c_str());
		}
		else
		{
			oss << "CMP " << elseValue.realDestination.name << ", " << conditionLTZero.name << ", " << elseValue.realDestination.name <<  ", " <<elseValue.tempDestination.name << ";\n";
			OUTPUT_ARB("%s", oss.str().c_str());
		}
		return;
	}


	//both exist
	if (ifTable)
	{
		// if( ifTable), push it upwards into the if statement table.
		ConditionalAsgnStmt &temp = (*ifTable)[ifValue.realDestination.name]; //ifValue.realDestination must be equal to elseValue.realDestination

		if (temp.tempDestination.id == -1)
		{
			temp.realDestination = ifValue.realDestination;
			temp.tempDestination = reg_allocator->getNewReg();
		}
		// CMP result, condition, trueValue(if), falseValue(else)
		oss << "CMP " << temp.tempDestination.name << ", " << conditionLTZero.name << ", " << ifValue.tempDestination.name <<  ", " <<elseValue.tempDestination.name << ";\n";
		OUTPUT_ARB("%s", oss.str().c_str());
	}
	else
	{
		oss << "CMP " << elseValue.realDestination.name << ", " << conditionLTZero.name << ", " << ifValue.tempDestination.name << ", " << elseValue.tempDestination.name << ";\n";
		OUTPUT_ARB("%s", oss.str().c_str());
	}
	return;

}

struct initdBool
{
	initdBool() { Bool = false; }
	bool Bool;
};

// ifTable is for upper level ifTable to be merged into. If it doesn't exist, that's OK
void popAndMerge(Register conditionLTZero, std::map<std::string, ConditionalAsgnStmt>& ifValue, std::map<std::string, ConditionalAsgnStmt> &elseValue, std::map<std::string, ConditionalAsgnStmt>* ifTable)
{
	std::map<std::string, initdBool> mergedEnteries;
	for (std::map<std::string, ConditionalAsgnStmt>::iterator ifIt = ifValue.begin(); ifIt != ifValue.end(); ifIt++) {
		ConditionalAsgnStmt::merge(conditionLTZero, ifValue[ifIt->first], elseValue[ifIt->first], ifTable);
		mergedEnteries[ifIt->first].Bool = true;
	}
	for (std::map<std::string, ConditionalAsgnStmt>::iterator elseIt = elseValue.begin(); elseIt != elseValue.end(); elseIt++) {
		if (mergedEnteries[elseIt->first].Bool) // already merged
			continue;
		ConditionalAsgnStmt::merge(conditionLTZero, ifValue[elseIt->first], elseValue[elseIt->first], ifTable);
	}
	return;
}



int Scope::genARB(std::map<std::string, ConditionalAsgnStmt>* ifTable)
{
	if (this->declarations)
		this->declarations->genARB(NULL); // declarations within a if statement cannot affect outside it
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
		this->expression->genARB(NULL);  // declarations within a if statement cannot affect outside it
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

	popAndMerge(if_confition->reg, ifstatementResolutionTable, elsestatementResolutionTable, ifTable);
	return 0;
}


int AssignStatement::genARB(std::map<std::string, ConditionalAsgnStmt>* ifTable)
{
	std::ostringstream oss;

	this->variable->genARB(NULL);
	this->expression->genARB(NULL);

	if (ifTable)
	{
		// if( ifTable), push it upwards into the if statement table.
		ConditionalAsgnStmt &temp = (*ifTable)[variable->reg->name];

		if (temp.tempDestination.id == -1)
		{
			temp.realDestination = variable->reg;
			temp.tempDestination = reg_allocator->getNewReg();
		}

		oss << "MOV " << temp.tempDestination.name << ", " << this->expression->reg->name << ";\n";
		OUTPUT_ARB("%s", oss.str().c_str());

	}
	else // standard case
	{
		oss << "MOV " << this->variable->reg->name << ", " << this->expression->reg->name << ";\n";
		OUTPUT_ARB("%s", oss.str().c_str());
	}
	return 0;

}

int Variable::genARB(std::map<std::string, ConditionalAsgnStmt>* ifTable)
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
	case (3) :
		oss << "MOV " << result_reg->name << ".w, "
		<< static_cast <Expression *>(this->args_opt->get_ith_node(3))->reg->name << ";\n";
	case (2) :
		oss << "MOV " << result_reg->name << ".z, "
		<< static_cast <Expression *>(this->args_opt->get_ith_node(2))->reg->name << ";\n";
	case (1) :
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
		/* Logical not, since operand is either 1 or -1 it's just multiply by -1*/
		oss << "MUL " << result_reg->name << ", " << "-1, " << this->operand->reg->name;
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

	Register *temp_reg = reg_allocator->getNewReg();

	/*done: finish this*/

	switch (this->bopt)
	{
	case BOPT_AND://done
		oss << "MUL " << result_reg->name << ", " << operand1->reg->name << ", " << operand2->reg->name << ";\n";
		oss << "ABS " << result_reg->name << ", " << result_reg->name << ";\n";
		break;
	case BOPT_OR://done
		oss << "ABS " << result_reg->name << ", " << operand1->reg->name << ";\n";
		oss << "ABS " << temp_reg->name << ", " << operand2->reg->name << ";\n";
		oss << "ADD " << temp_reg->name << ", " << result_reg->name << ", " << temp_reg->name << ";\n";
		oss << "SUB " << temp_reg->name << ", " << "0, " << temp_reg->name;
		oss << "SLT " << result_reg->name << ", " << temp_reg->name << "0.0; \n";
		break;
	case BOPT_EXPO://done
		oss << "POW " << result_reg->name << this->operand1->reg->name
			<< this->operand2->reg->name << ";\n";
		break;
	case BOPT_EQ: //done, verification needed
		oss << "SUB " << result_reg->name << ", " << operand1->reg->name << ", " << operand2->reg->name << ";\n";
		oss << "SUB " << temp_reg->name << ", " << operand2->reg->name << ", " << operand1->reg->name << ";\n";
		oss << "CMP " << temp_reg->name << ", " << temp_reg->name << ", 0.0, 1.0;\n";
		oss << "CMP " << result_reg->name << ", " << temp_reg->name << ", 0.0, " << temp_reg->name << "; \n";
		break;
	case BOPT_NEQ: //done, verification needed
		oss << "SUB " << result_reg->name << ", " << operand1->reg->name << ", " << operand2->reg->name << ";\n";
		oss << "SUB " << temp_reg->name << ", " << operand2->reg->name << ", " << operand1->reg->name << ";\n";
		oss << "CMP " << temp_reg->name << ", " << temp_reg->name << ", 0.0, 1.0;\n";
		oss << "CMP " << result_reg->name << ", " << temp_reg->name << ", 0.0, " << temp_reg->name << "; \n"; // equal
		oss << "SUB " << result_reg->name << ", " << "0, " << result_reg->name; // not
		break;
	case BOPT_LT://done
		oss << "SLT " << result_reg->name << ", " << operand1->reg->name << ", " << operand2->reg->name << "; \n";
		break;
	case BOPT_LEQ://done
		oss << "SGE " << result_reg->name << ", " << operand2->reg->name << ", " << operand1->reg->name << "; \n";
		break;
	case BOPT_GT://done
		oss << "SLT " << result_reg->name << ", " << operand2->reg->name << ", " << operand1->reg->name << "; \n";
		break;
	case BOPT_GEQ://done
		oss << "SGE " << result_reg->name << ", " << operand1->reg->name << ", " << operand2->reg->name << "; \n";
		break;
	case BOPT_ADD://done
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


	varRegMap["gl_FragColor"] = new Register("result.color", INT32_MAX - 1);
	varRegMap["gl_FragDepth"] = new Register("result.depth", INT32_MAX - 2);
	varRegMap["gl_FragCoord"] = new Register("fragment.position", INT32_MAX - 3);
	varRegMap["gl_TexCoord"] = new Register("fragment.texcoord", INT32_MAX - 4);
	varRegMap["gl_Color"] = new Register("fragment.color", INT32_MAX - 5);
	varRegMap["gl_Secondary"] = new Register("fragment.color.secondary", INT32_MAX - 6);
	varRegMap["gl_FogFragCoord"] = new Register("fragment.fogcoord", INT32_MAX - 7);
	varRegMap["gl_Light_Half"] = new Register("state.light[0].half", INT32_MAX - 8);
	varRegMap["gl_Light_Ambient"] = new Register("state.lightmodel.ambient", INT32_MAX - 9);
	varRegMap["gl_Material_Shininess"] = new Register("state.material.shininess", INT32_MAX - 10);
	varRegMap["env1"] = new Register("program.env[1]", INT32_MAX - 11);
	varRegMap["env2"] = new Register("program.env[2]", INT32_MAX - 12);
	varRegMap["env3"] = new Register("program.env[3]", INT32_MAX - 13);




	OUTPUT_ARB("!!ARBfp1.0\n")
		ast->genARB(NULL);
	OUTPUT_ARB("END\n")
		return 0;
}
