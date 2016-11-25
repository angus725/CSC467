#include "semantic.h"

// wtf did these comments and code come from?
// if (getExpressionResultType(N->if_statement.if_confition) != TYPE_BOOL)
//     d postck is being   read  from(node *N); // does the actual checking

static int semantic_fail = 0; // 0success, 1 fail

static int nestedIfCount = 0;

Var_type getExpressionResultType(node *N)
{
    //inputs: N must be an "EXPRESSION"
    // all nodes below N has already been traversed by semantic check
    // outputs: return must be the type the expression N returns
    Symbol *pTempSymbol;

    if (is_expression(N->kind))
        return INVALID;

    switch (N->kind)
    {
    case TYPE:
        return N->type.var_type;

    case LITERAL_EXP:
        return N->literal_exp.var_type;

    case MULTI_NODE:
        if (getExpressionResultType(N->multi_node.nodes) == getExpressionResultType(N->multi_node.cur_node))
            return getExpressionResultType(N->multi_node.cur_node);
        else
            return TYPE_ANY; // TODO logic to make this print something that makes sense

    case VARIABLE:
        pTempSymbol = symbolCactus->find(N->variable.identifier);
        if (pTempSymbol)
            return pTempSymbol->var_type;
        return NOT_FOUND;

    default:
        return INVALID;
    }
    return INVALID;
}

int getExpressionResultArrayBound(node *N)
{
    //does the same as getExpressionResultType, but returns the ArrayBound instead of type
    if (is_expression(ast->kind))
        return -1;
    // TODO
    return 0;
}

int semantic_check(node *ast)
{

    ast_traversal(ast, pre_check, post_check);

    return semantic_fail; // 0 is OK, 1 is fail
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
    node *temp;
    Symbol tempSymbol;
    Symbol *pTempSymbol;
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
        if (N->type.var_type != getExpressionResultType(N->declaration.expression))
        {
            fprintf(errorFile, "ERROR on line %i, expecting %s but got %s\n",
                    N->declaration.type->line_num,
                    type_to_str(N->declaration.type->type.var_type,
                                N->declaration.type->type.array_bound),
                    type_to_str(getExpressionResultType(N->declaration.expression),
                                getExpressionResultArrayBound(N->declaration.expression)));
        }

        // const expressions are allowed, variables are not
        if (N->declaration.is_const && (N->declaration.expression != nullptr) && !(N->declaration.expression->constantValue))
            fprintf(errorFile, "ERROR on line %i, cannot assign a variable value to a const variable\n", N->line_num);

        //set the info for the new symbol
        if (N->declaration.is_const)
            N->constantValue = 1;
        tempSymbol.line_num = N->line_num;
        tempSymbol.isConstant = N->constantValue;
        tempSymbol.name = N->declaration.identifier;
        tempSymbol.index_size = N->declaration.type->type.array_bound;
        if (N->declaration.expression != nullptr)
            tempSymbol.attribute = INITIALIZED;

        intTemp = symbolCactus->insert(tempSymbol); // add to variable table

        // variable must not have been declared before in current scope
        if (intTemp == ERROR_DUPLICATE_VARIABLE)
            fprintf(errorFile, "ERROR on line %i, variable %s already exists, previously defined on line %i\n", N->line_num, N->declaration.identifier, tempSymbol.line_num);
        if (intTemp != 0)
        {
            fprintf(errorFile, "ERROR on line %i, variable %s is causing a symbolTable error, please investigate futher\n", N->line_num, N->declaration.identifier);
        }
        break;
    case ASSIGN_STATEMENT:
        // make sure the variable has been declared
        pTempSymbol = symbolCactus->find(N->assignment_statement.variable->variable.identifier);
        if (pTempSymbol == nullptr)
        {
            fprintf(errorFile, "ERROR on line %i, variable %s was not been declared\n", N->line_num, N->assignment_statement.variable->variable.identifier);

            tempSymbol.var_type = getExpressionResultType(N->assignment_statement.expression);
            tempSymbol.line_num = N->assignment_statement.variable->line_num;
            tempSymbol.isConstant = N->assignment_statement.expression->constantValue;
            tempSymbol.name = N->assignment_statement.variable->variable.identifier;
            tempSymbol.index_size = getExpressionResultArrayBound(N->assignment_statement.expression);
            intTemp = symbolCactus->insert(tempSymbol); // add to variable table
            if (intTemp != 0)
            {
                fprintf(errorFile, "Something horrible is happening, aborting.\n");
                abort();
            }
        }

        // match types of assigner and assignee
        if (pTempSymbol->var_type != getExpressionResultType(N->assignment_statement.expression))
        {
            fprintf(errorFile, "ERROR on line %i, %s is type %s and cannot be assigned type %s\n",
                    N->assignment_statement.variable->line_num,
                    pTempSymbol->name.c_str(),
                    type_to_str(pTempSymbol->var_type, pTempSymbol->index_size),
                    type_to_str(getExpressionResultType(N->assignment_statement.expression), getExpressionResultArrayBound(N->assignment_statement.expression)));
        }

        // consts cannot be assigned, only declared.
        if (pTempSymbol->isConstant)
            fprintf(errorFile, "ERROR on line %i, %s is a const and cannot be reassigned values\n", N->assignment_statement.variable->line_num, pTempSymbol->name.c_str());

        if (pTempSymbol->attribute & (ATTRIBUTE | UNIFORM))
            fprintf(errorFile, "ERROR on line %i, %s is a predefined value that cannot be written to\n", N->assignment_statement.variable->line_num, pTempSymbol->name.c_str());

        // check variable attributes ( see "Pre Defined Variables")
        if (pTempSymbol->attribute & RESULT && nestedIfCount > 0)
            fprintf(errorFile, "ERROR on line %i, %s is a predefined value that cannot be written to in the scope of an IF statement\n", N->assignment_statement.variable->line_num, pTempSymbol->name.c_str());

        if (getExpressionResultType(N->assignment_statement.expression) == WRITE_ONLY)
            fprintf(errorFile, "ERROR on line %i, a predefined value that is WRITE ONLY is being read from\n", N->assignment_statement.expression->line_num);

        break;
    case IF_STATEMENT:
        // check if bool is used
        if (getExpressionResultType(N->if_statement.if_confition) != TYPE_BOOL)
            fprintf(errorFile, "ERROR on line %i, branch statements must have an expression returning a bool\n", N->if_statement.if_confition->line_num);
        nestedIfCount--;
        break;
    case TYPE:
        break;
    case FUNC_CALL_EXP:
        // make sure certain functions use only certain types - look at handout
        // make sure returns are assigned
        break;
    case CONSTRUCTOR_EXP:
        //TODO type must equal argument
        // TODO const expressions are allowed
        break;
    case UNARY_EXP:
        // TODO assign operand type to result type;
        // scalar and vectors only
        // check variable attributes ( see "Pre Defined Variables")
        break;
    case BINARY_EXP:

        // TODO
        // logical operators need to have true/false types
        // arithmatic operators - arithmatic types
        // comparison ops - arithmatic types
        // vector types are equal size
        // match table with operators
        // figure out resullt_type
        // check variable attributes ( see "Pre Defined Variables")
        break;
    case LITERAL_EXP:
        break;
    case VARIABLE:
        //
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

    semantic_fail = 0; // 0 means OK, 1 means FAIL
}