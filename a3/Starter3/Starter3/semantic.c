#include "semantic.h"


#define SEMANTIC_ERROR(...) fprintf(errorFile,  __VA_ARGS__); semantic_fail = 1;

static int semantic_fail = 0; // 0success, 1 fail

static int nestedIfCount = 0;

int isWriteOnly(node* N)
{
	if(N == NULL)
		return 0;
	if(N->kind == LITERAL_EXP)
		return 0; // false
	if(N->kind == VARIABLE )
	{
	    Symbol *pTempSymbol = symbolCactus->find(N->variable.identifier);
	    if(pTempSymbol)
	    	return (pTempSymbol->attribute & WRITE_ONLY);
	    else
	    	abort(); // not found, wtf
	}


	switch (N->kind) {
		case     UNARY_EXP:
			return isWriteOnly(N->unary_exp.operand);
		case     BINARY_EXP:
			return isWriteOnly(N->binary_exp.operand1) | isWriteOnly(N->binary_exp.operand2);
		case     MULTI_NODE:
			return isWriteOnly(N->multi_node.cur_node) | isWriteOnly(N->multi_node.nodes);
		default:
			break;
	}
	return 0; // false
}

int isScalar(enum data_type dType) // output for type_Unknown and type_any is undefined
{
	if(dType == TYPE_ANY || (dType== var_type_to_base(dType)) || dType == TYPE_UNKNOWN)
		return 1;
	return 0;
}

enum data_type getExpressionResultType(node *N)
{
    //inputs: N must be an "EXPRESSION"
    // all nodes below N has already been traversed by semantic check
    // outputs: return must be the type the expression N returns
    Symbol *pTempSymbol;

    if (!is_expression(N->kind))
        return TYPE_INVALID;

    switch (N->kind)
    {
    case VARIABLE:
        pTempSymbol = symbolCactus->find(N->variable.identifier);
        if (pTempSymbol)
            return pTempSymbol->var_type;
        return TYPE_UNKNOWN;


    case FUNC_CALL_EXP:
    	return N->func_call_exp.result_type;
    case CONSTRUCTOR_EXP:
    	return N->constructor_exp.type->type.var_type;
    case UNARY_EXP:
    	return N->unary_exp.result_type;
    case BINARY_EXP:
    	return N->binary_exp.result_type;

    case LITERAL_EXP:
        return N->literal_exp.lit_type;
    default:
        return TYPE_INVALID;
    }
}

static enum data_type get_func_result_type(node* N)
{
	switch(N->func_call_exp.func) {
	case FUNC_LIT:
		return TYPE_VEC4;
	case FUNC_DP3:
		return  var_type_to_base(getExpressionResultType(N->func_call_exp.args_opt->multi_node.cur_node));
	case FUNC_RSQ:
		return getExpressionResultType(N->func_call_exp.args_opt->multi_node.cur_node);
	default:
		break;
	}
	return TYPE_UNKNOWN;
}

int semantic_check(node *ast)
{

    ast_traversal(ast, pre_check, post_check);

    return semantic_fail; // 0 is OK, 1 is fail
}

int count_nodes(node *multi_node)
{
	if(multi_node == NULL)
		return 0;
	if (multi_node->kind != MULTI_NODE)
		return 1;
	else {
		return count_nodes(multi_node->multi_node.nodes) +count_nodes(multi_node->multi_node.cur_node);
	}
}

int get_array_bound_from_type(enum data_type type)
{
	enum data_type dType = var_type_to_base(type);
	if (type == TYPE_ANY || dType <= 0)
		return -1;
	return type - dType;
}

void pre_check(node *N)
{
    switch (N->kind)
    {
    case SCOPE:
        symbolCactus->pushScope();
        break;
    case IF_STATEMENT:
        nestedIfCount++;
        break;
    default:
        break;
    }
}

void post_check(node *N)
{
    Symbol tempSymbol;
    Symbol *pTempSymbol;
    enum data_type type;
    int array_len;
    node *cur_node;
    int intTemp; //used for complex return types
    if (N == nullptr)
        return; //wtf am I doing with a NULL ptr

    switch (N->kind)
    {
    case SCOPE:
        symbolCactus->popScope();
        break;
    case MULTI_NODE:
        // Don't do anything, let the functions that can traverse multi-nodes do the work
        // NO NEED: check and fill last_var_result_type to the struct multi_node definition
        break;
    case DECLARATION:
        // type must equal argument
        if (N->declaration.expression &&
        		N->declaration.type->type.var_type != getExpressionResultType(N->declaration.expression))
        {
            SEMANTIC_ERROR("ERROR on line %i, expecting %s but got %s\n",
                    N->declaration.type->line_num,
                    type_to_str(N->declaration.type->type.var_type),
                    type_to_str(getExpressionResultType(N->declaration.expression)));
        }

        // const expressions are allowed, variables are not
        if (N->declaration.is_const && (N->declaration.expression != nullptr) && !(N->declaration.expression->constantValue))
            SEMANTIC_ERROR("ERROR on line %i, cannot assign a variable value to a const variable\n", N->line_num);

        //set the info for the new symbol
        if (N->declaration.is_const)
            N->constantValue = 1;
        tempSymbol.line_num = N->line_num;
        tempSymbol.isConstant = N->constantValue;
        tempSymbol.name = N->declaration.identifier;
        if (N->declaration.expression != nullptr)
            tempSymbol.attribute = INITIALIZED;

        intTemp = symbolCactus->insert(tempSymbol); // add to variable table

        // variable must not have been declared before in current scope
        if (intTemp == ERROR_DUPLICATE_VARIABLE)
            SEMANTIC_ERROR("ERROR on line %i, variable %s already exists, previously defined on line %i\n", N->line_num, N->declaration.identifier, tempSymbol.line_num);
        if (intTemp != 0)
        {
            SEMANTIC_ERROR("ERROR on line %i, variable %s is causing a symbolTable error, please investigate futher\n", N->line_num, N->declaration.identifier);
        }
        std::cout << *symbolCactus << std::endl;
        break;
    case ASSIGN_STATEMENT:
        // make sure the variable has been declared
        pTempSymbol = symbolCactus->find(N->assignment_statement.variable->variable.identifier);
        if (pTempSymbol == nullptr)
        {
            SEMANTIC_ERROR("ERROR on line %i, variable %s was not been declared\n", N->line_num, N->assignment_statement.variable->variable.identifier);

            tempSymbol.var_type = getExpressionResultType(N->assignment_statement.expression);
            tempSymbol.line_num = N->assignment_statement.variable->line_num;
            tempSymbol.isConstant = N->assignment_statement.expression->constantValue;
            tempSymbol.name = N->assignment_statement.variable->variable.identifier;
            intTemp = symbolCactus->insert(tempSymbol); // add to variable table
            if (intTemp != 0)
            {
                SEMANTIC_ERROR("Something horrible is happening, aborting.\n");
                abort();
            }
        }

        // match types of assigner and assignee
        if (pTempSymbol->var_type != getExpressionResultType(N->assignment_statement.expression))
        {
            SEMANTIC_ERROR("ERROR on line %i, %s is type %s and cannot be assigned type %s\n",
                    N->assignment_statement.variable->line_num,
                    pTempSymbol->name.c_str(),
                    type_to_str(pTempSymbol->var_type),
                    type_to_str(getExpressionResultType(N->assignment_statement.expression)));
        }

        // consts cannot be assigned, only declared.
        if (pTempSymbol->isConstant)
            SEMANTIC_ERROR("ERROR on line %i, %s is a const and cannot be reassigned values\n", N->assignment_statement.variable->line_num, pTempSymbol->name.c_str());

        if (pTempSymbol->attribute & (ATTRIBUTE | UNIFORM))
            SEMANTIC_ERROR("ERROR on line %i, %s is a predefined value that cannot be written to\n", N->assignment_statement.variable->line_num, pTempSymbol->name.c_str());

        // check variable attributes ( see "Pre Defined Variables")
        if (pTempSymbol->attribute & RESULT && nestedIfCount > 0)
            SEMANTIC_ERROR("ERROR on line %i, %s is a predefined value that cannot be written to in the scope of an IF statement\n", N->assignment_statement.variable->line_num, pTempSymbol->name.c_str());

        if (getExpressionResultType(N->assignment_statement.expression) == WRITE_ONLY)
            SEMANTIC_ERROR("ERROR on line %i, a predefined value that is WRITE ONLY is being read from\n", N->assignment_statement.expression->line_num);

        break;
    case IF_STATEMENT:
        // check if bool is used
        if (getExpressionResultType(N->if_statement.if_confition) != TYPE_BOOL)
            SEMANTIC_ERROR("ERROR on line %i, branch statements must have an expression returning a bool\n", N->if_statement.if_confition->line_num);
        nestedIfCount--;
        break;
    case TYPE:
        break;
    case FUNC_CALL_EXP:
    	N->func_call_exp.result_type = get_func_result_type(N);
    	switch(N->func_call_exp.func) {
    	case FUNC_LIT:
    		if (count_nodes(N) != 1)
                SEMANTIC_ERROR("ERROR on line %i, wrong number of arguments to function lit\n", N->line_num);
    		if (getExpressionResultType(N->func_call_exp.args_opt) != TYPE_VEC4) {
    			SEMANTIC_ERROR("ERROR on line %i, wrong type of argument to function lit, expecting vec4\n", N->line_num);
    		}
    		break;
    	case FUNC_DP3:
    		if (count_nodes(N) != 2)
                SEMANTIC_ERROR("ERROR on line %i, wrong number of arguments to function lit\n", N->line_num);

    		if (getExpressionResultType(N->func_call_exp.args_opt->multi_node.cur_node) ==
    				getExpressionResultType(N->func_call_exp.args_opt->multi_node.nodes)) {
    			type = getExpressionResultType(N->func_call_exp.args_opt->multi_node.cur_node);

    			if (type != TYPE_VEC3 && type != TYPE_VEC4 && type != TYPE_IVEC3 && type!= TYPE_IVEC4)
    				SEMANTIC_ERROR("ERROR on line %i, wrong type of argument to function dp3\n", N->line_num);
    		}
    		break;
    	case FUNC_RSQ:
    		if (count_nodes(N) != 1)
                SEMANTIC_ERROR("ERROR on line %i, wrong number of arguments to function rsq\n", N->line_num);
			type = getExpressionResultType(N->func_call_exp.args_opt);
    		if (type != TYPE_INT && type != TYPE_FLOAT)
    			SEMANTIC_ERROR("ERROR on line %i, wrong type of argument to function rsq\n", N->line_num);
    		break;
    	default:
    		abort();
    	}
        // make sure certain functions use only certain types - look at handout
    	if(isWriteOnly(N))
    		SEMANTIC_ERROR("ERROR on line %i, argument to function is write only\n", N->line_num);
        break;
    case CONSTRUCTOR_EXP:
    	array_len = N->constructor_exp.type->type.var_type - var_type_to_base(N->constructor_exp.type->type.var_type) + 1;
    	if (array_len != count_nodes(N->constructor_exp.args_opt))
    		SEMANTIC_ERROR("ERROR on line %i, wrong number of arguments to constructor, expecting %d\n", N->line_num, array_len);
    	for (cur_node = N->constructor_exp.args_opt; cur_node->kind == MULTI_NODE; cur_node = cur_node->multi_node.nodes) {
    		if (getExpressionResultType(cur_node->multi_node.cur_node) != var_type_to_base(N->constructor_exp.type->type.var_type))
    			SEMANTIC_ERROR("ERROR on line %i, wrong type of arguments to constructor, expecting %s\n", N->line_num, type_to_str(N->constructor_exp.type->type.var_type))
    	}
		if (getExpressionResultType(cur_node) != var_type_to_base(N->constructor_exp.type->type.var_type))
				SEMANTIC_ERROR("ERROR on line %i, wrong type of arguments to constructor, expecting %s\n", N->line_num, type_to_str(N->constructor_exp.type->type.var_type))

		if(isWriteOnly(N))
				SEMANTIC_ERROR("ERROR on line %i, argument to constructor is write only\n", N->line_num);

        break;
    case UNARY_EXP:
    	if(N->unary_exp.uopt == UOPT_NEG)
    	{
    		enum data_type resultType = getExpressionResultType(N->unary_exp.operand);
    		if( resultType < TYPE_INT || resultType >= TYPE_ANY )
    		{
    			SEMANTIC_ERROR("ERROR on line %i, wrong type passed to unary operator, expecting %s\n", N->line_num, type_to_str(resultType) )
				N->unary_exp.result_type = TYPE_ANY;
    		}
    		else
    			N->unary_exp.result_type = resultType;
    	}
    	else if(N->unary_exp.uopt == UOPT_NOT)
    	{
    		enum data_type resultType = getExpressionResultType(N->unary_exp.operand);
    		if( resultType < TYPE_BOOL || resultType >= TYPE_INT )
    		{
    			SEMANTIC_ERROR("ERROR on line %i, wrong type passed to unary operator, expecting %s\n", N->line_num, type_to_str(resultType) )
				N->unary_exp.result_type = TYPE_ANY;
    		}
    		else
    			N->unary_exp.result_type = resultType;
    	}
    	else //ERROR
    		abort();
    	if(isWriteOnly(N->unary_exp.operand))
			SEMANTIC_ERROR("ERROR on line %i, varible passed to unary operator is write only.\n", N->line_num)
        // DONE assign operand type to result type;
        // DONE scalar and vectors only
        // DONE check variable attributes ( see "Pre Defined Variables")
        break;
    case BINARY_EXP:
    	{
    		enum data_type resultTypeA = getExpressionResultType(N->binary_exp.operand1);
    		enum data_type resultTypeB = getExpressionResultType(N->binary_exp.operand2);

    		switch (N->binary_exp.bopt) {
					//logical
    			case 	BOPT_AND:
				case    BOPT_OR:
					if(resultTypeA==resultTypeB && var_type_to_base(resultTypeA) == TYPE_BOOL)
		    			N->binary_exp.result_type = TYPE_BOOL;
					else
					{
		    			SEMANTIC_ERROR("ERROR on line %i, wrong type passed to binary operator\n", N->line_num )
						N->binary_exp.result_type = TYPE_ANY;
					}
					break;
					//comparison scalar only
				case    BOPT_LT:
				case    BOPT_LEQ:
				case    BOPT_GT:
				case    BOPT_GEQ:
					if(resultTypeA==resultTypeB && resultTypeB == var_type_to_base(resultTypeB))
		    			N->binary_exp.result_type = TYPE_BOOL;
					else
					{
		    			SEMANTIC_ERROR("ERROR on line %i, wrong type passed to binary operator\n", N->line_num )
						N->binary_exp.result_type = TYPE_ANY;
					}
					break;
				case    BOPT_EQ:
				case    BOPT_NEQ:
					if(resultTypeA==resultTypeB)
		    			N->binary_exp.result_type = TYPE_BOOL;
					else
					{
		    			SEMANTIC_ERROR("ERROR on line %i, wrong type passed to binary operator\n", N->line_num )
						N->binary_exp.result_type = TYPE_ANY;
					}
					break;
					// arithmatic any
				case    BOPT_MUL:
					if(resultTypeA==resultTypeB)
						N->binary_exp.result_type = resultTypeA;
					else if(isScalar(resultTypeB))
						N->binary_exp.result_type = resultTypeA;
					else if(isScalar(resultTypeA))
						N->binary_exp.result_type = resultTypeB;
					else
					{
						SEMANTIC_ERROR("ERROR on line %i, wrong type passed to binary operator\n", N->line_num )
						N->binary_exp.result_type = TYPE_ANY;
					}
					break;
					//arithmatic ss vv only
				case    BOPT_ADD:
				case    BOPT_SUB:
					if(resultTypeA==resultTypeB && var_type_to_base(resultTypeB) != TYPE_BOOL )
		    			N->binary_exp.result_type = resultTypeA;
					else
					{
		    			SEMANTIC_ERROR("ERROR on line %i, wrong type passed to binary operator\n", N->line_num )
						N->binary_exp.result_type = TYPE_ANY;
					}

					//arithmatic scalar only
				case    BOPT_EXPO:
				case    BOPT_DIV:
					if(resultTypeA==resultTypeB && isScalar(resultTypeB) && resultTypeB != TYPE_BOOL )
		    			N->binary_exp.result_type = resultTypeA;
					else
					{
		    			SEMANTIC_ERROR("ERROR on line %i, wrong type passed to binary operator\n", N->line_num )
						N->binary_exp.result_type = TYPE_ANY;
					}
					break;
				default:
					abort();
					break;

			}
			if(isWriteOnly(N->binary_exp.operand1) || isWriteOnly(N->binary_exp.operand2) )
			{
				SEMANTIC_ERROR("ERROR on line %i, reading from a write only variable\n", N->line_num )
			}
    	// logical operators need to have true/false types
    	        // arithmatic operators - arithmatic types
    	        // comparison ops - arithmatic types
    	        // vector types are equal size
    	        // match table with operators
    	        // figure out resullt_type
    	        // check variable attributes ( see "Pre Defined Variables")
    	}
        break;
    case LITERAL_EXP:
        break;
    case VARIABLE:
        pTempSymbol = symbolCactus->find(N->variable.identifier);
        if (!pTempSymbol) {
			SEMANTIC_ERROR("ERROR on line %i, reference to undefined variable %s\n", N->line_num, N->variable.identifier)
			abort();
        }

        N->variable.var_type = pTempSymbol->var_type;

        if (N->variable.has_index) {
        	if (isScalar(N->variable.var_type) || N->variable.array_index > get_array_bound_from_type(N->variable.var_type)) {
        		SEMANTIC_ERROR("ERROR on line %i, array index out of bound\n", N->line_num)
        	}
        }
        // TODO check bounds on array_index
        // TODO assign array element type to var_type
        break;

    default:
        break;
    }

    // recursively traverse tree

    // check if symatic is correct
    //yes - > return OK
    // no -> return not OK

}
