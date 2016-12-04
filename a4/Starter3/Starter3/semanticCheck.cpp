#include "ast.h"
#include "symbol.h"
#include "common.h"

#define SEMANTIC_ERROR(...) {fprintf(errorFile,  __VA_ARGS__); semantic_fail = 1;}


extern char* type_to_str(enum data_type dType);

extern std::unique_ptr<SymbolCactus> symbolCactus;

int ASTNode::nestedIfCount = 0;

int Scope::checkSemantic()
{
	int semantic_fail = 0;
	symbolCactus->pushScope();

	if (declarations)
		semantic_fail = declarations->checkSemantic();
	if (statements)
		semantic_fail = semantic_fail |	statements->checkSemantic();

	symbolCactus->popScope();
	return semantic_fail;
}

int MultiNode::checkSemantic()
{
	int semantic_fail = 0;

	if (cur_node)
		semantic_fail = cur_node->checkSemantic();

	if (nodes)
		semantic_fail = semantic_fail | nodes->checkSemantic();

	if (cur_node) {
		constantValue = cur_node->isConst();
		if (nodes)
			constantValue = constantValue & nodes->isConst();

	}
	//printf("LINE: %i, MultiNode CONSTANT: %i\n", line_num, constantValue);


	return semantic_fail;
}

int MultiNode::countParameters() // non-multiNode leafs
{
	if (nodes && cur_node)
		return nodes->countParameters() + cur_node->countParameters();
	if (nodes)
		return nodes->countParameters();
	return cur_node->countParameters();
}

int MultiNode::isWriteOnly(){	return (cur_node->isWriteOnly() | nodes->isWriteOnly());}

data_type MultiNode::getResultType()
{
	data_type ret1 = TYPE_UNKNOWN, ret2 = TYPE_UNKNOWN;
	if (nodes)
		ret1 = nodes->getResultType();
	if (cur_node)
		ret2 = cur_node->getResultType();
	if (ret1 == TYPE_UNKNOWN)
		return ret2;
	if (ret2 == TYPE_UNKNOWN)
		return ret1;

	if (ret1 != ret2)
		return TYPE_ANY;
	return ret1;
}

int Declaration::checkSemantic()
{
	int semantic_fail = 0;
	Symbol tempSymbol;



	semantic_fail = type->checkSemantic();
	if (expression)
		semantic_fail = expression->checkSemantic() | semantic_fail;

	// type must equal argument
	if (expression && type->getResultType() != expression->getResultType())
	{
		SEMANTIC_ERROR("ERROR on line %i, expecting %s but got %s\n",
			type->getLineNum(),
			type_to_str(type->getResultType()),
			type_to_str(expression->getResultType()));
	}

	// const expressions are allowed, variables are not
	if (this->isConst() && expression && !expression->isConst())
		SEMANTIC_ERROR("ERROR on line %i, cannot assign a variable value to a const variable\n", this->line_num);

	

	//set the info for the new symbol
	if (this->isConst())
		constantValue = 1;
	tempSymbol.line_num = line_num;
	tempSymbol.isConstant = constantValue;
	tempSymbol.name = identifier;
	tempSymbol.var_type = type->getResultType();
	tempSymbol.attribute = INITIALIZED;

	semantic_fail =  symbolCactus->insert(tempSymbol); // add to variable table

	// variable must not have been declared before in current scope
	if (semantic_fail == ERROR_DUPLICATE_VARIABLE)
		SEMANTIC_ERROR("ERROR on line %i, variable %s already exists\n", line_num, identifier.c_str());
	if (semantic_fail != 0 && semantic_fail != ERROR_DUPLICATE_VARIABLE)
	{
		SEMANTIC_ERROR("ERROR on line %i, variable %s is causing an unexpected symbolTable error, please investigate further\n", line_num, identifier.c_str());
	}
	//        std::cout << *symbolCactus << std::endl;

	return semantic_fail;
}

int IfStatement::checkSemantic()
{
	int semantic_fail = 0;
	nestedIfCount++;

	semantic_fail = if_confition->checkSemantic();
	semantic_fail = semantic_fail | if_body->checkSemantic();
	if (else_body)
		semantic_fail = semantic_fail | else_body->checkSemantic();

	// check if bool is used
	if (if_confition->getResultType() != TYPE_BOOL)
		SEMANTIC_ERROR("ERROR on line %i, branch statements must have an expression returning a bool\n", 
		if_confition->getLineNum());
	
	nestedIfCount--;
	return semantic_fail;
}

int AssignStatement::checkSemantic()
{
	int semantic_fail = 0;

	semantic_fail = variable->checkSemantic();
	semantic_fail = semantic_fail | expression->checkSemantic();

	// make sure the variable has been declared
	Symbol *pTempSymbol = symbolCactus->find(variable->getID());
	if (pTempSymbol == nullptr)
	{
		SEMANTIC_ERROR("ERROR on line %i, variable %s was not been declared\n", line_num, (variable->getID()).c_str());

		Symbol tempSymbol;
		tempSymbol.var_type = expression->getResultType();
		tempSymbol.line_num = variable->getLineNum();
		tempSymbol.isConstant = expression->isConst();
		tempSymbol.name = variable->getID();
		semantic_fail = symbolCactus->insert(tempSymbol); // add to variable table
	}

	// match types of assigner and assignee
	if (pTempSymbol->var_type != expression->getResultType())
	{
		SEMANTIC_ERROR("ERROR on line %i, %s is type %s and cannot be assigned type %s\n",
			variable->getLineNum(),
			pTempSymbol->name.c_str(),
			type_to_str(pTempSymbol->var_type),
			type_to_str(expression->getResultType()));
	}

	// consts cannot be assigned, only declared.
	if (pTempSymbol->isConstant)
		SEMANTIC_ERROR("ERROR on line %i, %s is a const and cannot be reassigned values\n",
		variable->getLineNum(), pTempSymbol->name.c_str());

	if (pTempSymbol->attribute & (ATTRIBUTE | UNIFORM))
		SEMANTIC_ERROR("ERROR on line %i, %s is a predefined value that cannot be written to\n", 
		variable->getLineNum(), pTempSymbol->name.c_str());

	// check variable attributes ( see "Pre Defined Variables")
	if (pTempSymbol->attribute & RESULT && nestedIfCount > 0)
		SEMANTIC_ERROR("ERROR on line %i, %s is a predefined value that cannot be written to in the scope of an IF statement\n",
		variable->getLineNum(), pTempSymbol->name.c_str());

	if (expression->isWriteOnly())
		SEMANTIC_ERROR("ERROR on line %i, a predefined value that is WRITE ONLY is being read from\n",
		expression->getLineNum());

	return semantic_fail;
}

int Type::checkSemantic()
{
	int semantic_fail = 0;
	constantValue = 1; // hack to get around stuff. Means a Type Cast is a constant cast

	return semantic_fail;
}

int Variable::checkSemantic()
{
	int semantic_fail = 0;

	Symbol *pTempSymbol = symbolCactus->find(identifier);
	if (!pTempSymbol) {
		SEMANTIC_ERROR("ERROR on line %i, reference to undefined variable %s\n", line_num, identifier.c_str())
			abort();
	}

	var_type = pTempSymbol->var_type;

	if (has_index) {
		if (isScalar(var_type) || array_index > get_array_bound()) {
			SEMANTIC_ERROR("ERROR on line %i, array index out of bound\n", line_num)
		}
		var_type = var_type_to_base(var_type);
	}
	
	if (pTempSymbol->isConstant)
		constantValue = 1;
	// TODO check bounds on array_index
	// TODO assign array element type to var_type

	return semantic_fail;
}

int Variable::isWriteOnly()
{
	Symbol *pTempSymbol = symbolCactus->find(identifier);
	if (pTempSymbol)
		return (pTempSymbol->attribute & WRITE_ONLY);
	abort(); // not found, but this should exception have been handled before this is called.
}

int Variable::get_array_bound() // shouldn't this also go into... Type? Nope. That's not valid syntax
{
	data_type dType = var_type_to_base(var_type);
	if (var_type == TYPE_ANY || dType <= 0)
		return -1;
	return var_type - dType;
}

int FunctionCall::checkSemantic()
{
	int semantic_fail = 0;

	semantic_fail = args_opt->checkSemantic();

	data_type unifiedType = args_opt->getResultType();
	result_type = CalcFuncResultType();

	switch (func) { // this can be merged into the CalcFuncResultType call's return 
	case FUNC_LIT:
		if (args_opt->countParameters() != 1)
			SEMANTIC_ERROR("ERROR on line %i, wrong number of arguments to function lit\n", line_num);
		if (unifiedType != TYPE_VEC4) {
			SEMANTIC_ERROR("ERROR on line %i, wrong type of argument to function lit, expecting vec4\n", line_num);
		}
		break;
	case FUNC_DP3:
		//    	std::cout << count_nodes(N);
		if (args_opt->countParameters() != 2)
			SEMANTIC_ERROR("ERROR on line %i, wrong number of arguments to function dp3\n", line_num);
		if (unifiedType != TYPE_VEC3 && unifiedType != TYPE_VEC4 && unifiedType != TYPE_IVEC3 && unifiedType != TYPE_IVEC4)
				SEMANTIC_ERROR("ERROR on line %i, wrong type of argument to function dp3\n", line_num);
		break;
	case FUNC_RSQ:
		if (args_opt->countParameters() != 1)
			SEMANTIC_ERROR("ERROR on line %i, wrong number of arguments to function rsq\n", line_num);
		unifiedType = args_opt->getResultType();
		if (unifiedType != TYPE_INT && unifiedType != TYPE_FLOAT)
			SEMANTIC_ERROR("ERROR on line %i, wrong type of argument to function rsq\n", line_num);
		break;
	default:
		abort();
	}
	// make sure certain functions use only certain types - look at handout
	if (args_opt->isWriteOnly())
		SEMANTIC_ERROR("ERROR on line %i, argument to function is write only\n", line_num);

	return semantic_fail;
}

int FunctionCall::isWriteOnly(){ return args_opt->isWriteOnly(); }

data_type FunctionCall::CalcFuncResultType()
{
	switch (func) {
	case FUNC_LIT:
		result_type = TYPE_VEC4;
		return TYPE_VEC4;
	case FUNC_DP3:
		result_type = var_type_to_base(args_opt->getResultType());
		return  result_type;
	case FUNC_RSQ:
		result_type = TYPE_FLOAT;
		return TYPE_FLOAT;
	default:
		result_type = TYPE_UNKNOWN;
		return TYPE_UNKNOWN;
	}
}

int Constructor::checkSemantic()
{
	int semantic_fail = 0;
	semantic_fail = type->checkSemantic();
	semantic_fail = args_opt->checkSemantic() | semantic_fail;

	data_type tempType = type->getResultType();

	int array_len = tempType - var_type_to_base(tempType) + 1;
	if (array_len != args_opt->countParameters())
		SEMANTIC_ERROR("ERROR on line %i, wrong number of arguments to constructor, expecting %d\n", line_num, array_len);
	data_type unifiedType = args_opt->getResultType();
	if (var_type_to_base(tempType) != unifiedType)
		SEMANTIC_ERROR("ERROR on line %i, wrong type of arguments to constructor, expecting %s\n", line_num, type_to_str(var_type_to_base(tempType)))
	if (args_opt->isWriteOnly())
		SEMANTIC_ERROR("ERROR on line %i, argument to constructor is write only\n", args_opt->getLineNum());
	
	if (args_opt->isConst())
		constantValue = 1;

	return semantic_fail;
}

int Constructor::isWriteOnly(){ return args_opt->isWriteOnly(); }

int UnaryOP::checkSemantic()
{
	int semantic_fail = 0;

	semantic_fail = operand->checkSemantic();


	data_type resultType = operand->getResultType();
	if (uopt == UOPT_NEG)
	{
		if (resultType < TYPE_INT || resultType >= TYPE_ANY)
		{
			SEMANTIC_ERROR("ERROR on line %i, wrong type passed to unary operator, expecting %s\n",line_num, type_to_str(resultType))
				result_type = TYPE_ANY;
		}
		else
			result_type = resultType;
	}
	else if (uopt == UOPT_NOT)
	{
		if (resultType < TYPE_BOOL || resultType >= TYPE_INT)
		{
			SEMANTIC_ERROR("ERROR on line %i, wrong type passed to unary operator, expecting %s\n", line_num, type_to_str(resultType))
				result_type = TYPE_ANY;
		}
		else
			result_type = resultType;
	}
	else //ERROR
		abort();
	if (operand->isWriteOnly())
		SEMANTIC_ERROR("ERROR on line %i, varible passed to unary operator is write only.\n", line_num)
		// DONE assign operand type to result type;
		// DONE scalar and vectors only
		// DONE check variable attributes ( see "Pre Defined Variables")

	constantValue = operand->isConst();

	return semantic_fail;
}


int UnaryOP::isWriteOnly(){ return operand->isWriteOnly(); }

int isScalar(data_type dType); // declaring this for this file, actual definition is in ast.cpp

int BinaryOP::checkSemantic()
{
	int semantic_fail = 0;

	semantic_fail = operand1->checkSemantic();
	semantic_fail = semantic_fail | operand2->checkSemantic();


	enum data_type resultTypeA = operand1->getResultType();
	enum data_type resultTypeB = operand2->getResultType();

	switch (bopt) {
		//logical
	case 	BOPT_AND:
	case    BOPT_OR:
		if (resultTypeA == resultTypeB && var_type_to_base(resultTypeA) == TYPE_BOOL)
			result_type = TYPE_BOOL;
		else
		{
			SEMANTIC_ERROR("ERROR on line %i, wrong type passed to binary operator\n", line_num)
				result_type = TYPE_ANY;
		}
		break;
		//comparison scalar only
	case    BOPT_LT:
	case    BOPT_LEQ:
	case    BOPT_GT:
	case    BOPT_GEQ:
		if (resultTypeA == resultTypeB && resultTypeB == var_type_to_base(resultTypeB))
			result_type = TYPE_BOOL;
		else
		{
			SEMANTIC_ERROR("ERROR on line %i, wrong type passed to binary operator\n", line_num)
				result_type = TYPE_ANY;
		}
		break;
	case    BOPT_EQ:
	case    BOPT_NEQ:
		if (resultTypeA == resultTypeB)
			result_type = TYPE_BOOL;
		else
		{
			SEMANTIC_ERROR("ERROR on line %i, wrong type passed to binary operator\n", line_num)
				result_type = TYPE_ANY;
		}
		break;
		// arithmatic any
	case    BOPT_MUL:
		if (resultTypeA == resultTypeB)
			result_type = resultTypeA;
		else if (isScalar(resultTypeB) && var_type_to_base(resultTypeA) == resultTypeB)
			result_type = resultTypeA;
		else if (isScalar(resultTypeA) && var_type_to_base(resultTypeB) == resultTypeA)
			result_type = resultTypeB;
		else
		{
			SEMANTIC_ERROR("ERROR on line %i, wrong type passed to binary operator\n", line_num)
				result_type = TYPE_ANY;
		}
		break;
		//arithmatic ss vv only
	case    BOPT_ADD:
	case    BOPT_SUB:
		if (resultTypeA == resultTypeB && var_type_to_base(resultTypeB) != TYPE_BOOL)
			result_type = resultTypeA;
		else
		{
			SEMANTIC_ERROR("ERROR on line %i, wrong type passed to binary operator\n", line_num)
				result_type = TYPE_ANY;
		}

		//arithmatic scalar only
	case    BOPT_EXPO:
	case    BOPT_DIV:
		if (resultTypeA == resultTypeB && isScalar(resultTypeB) && resultTypeB != TYPE_BOOL)
			result_type = resultTypeA;
		else
		{
			SEMANTIC_ERROR("ERROR on line %i, wrong type passed to binary operator\n", line_num)
				result_type = TYPE_ANY;
		}
		break;
	default:
		abort();
		break;

	}
	if (operand1->isWriteOnly() || operand2->isWriteOnly())
	{
		SEMANTIC_ERROR("ERROR on line %i, reading from a write only variable\n", line_num)
	}
	// logical operators need to have true/false types
	// arithmatic operators - arithmatic types
	// comparison ops - arithmatic types
	// vector types are equal size
	// match table with operators
	// figure out resullt_type
	// check variable attributes ( see "Pre Defined Variables")
	if (operand1->isConst() && operand2->isConst())
		constantValue = 1;


	return semantic_fail;
}

int BinaryOP::isWriteOnly(){ return (operand1->isWriteOnly() | operand2->isWriteOnly()); }

int LiteralExp::checkSemantic()
{
	constantValue = 1;
	return 0; // success
}

int LiteralExp::isWriteOnly(){ return 0; }
