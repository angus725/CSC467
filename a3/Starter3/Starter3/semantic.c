
#include "semantic.h"

void pre_check(node *N);  // does the actual checking
void post_check(node *N); // does the actual checking

static int semantic_fail = 0; // default to not-fail
static int nestedIfCount = 0;

Var_type getExpressionResult(node* N);// TODO, N must be expression

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
    default:
        break;
    }
}

void post_check(node *N)
{
    std::string tempErrorString,tempErrorStringB;
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
        // if( N->type.var_type != getExpressionResult(N))
        // {
        //     varTypeToText(N->type.var_type, tempErrorString);
        //     varTypeToText(getExpressionResult(N), tempErrorStringB);
        //     fprintf(errorFile, "ERROR on line %i, expecting %s but got %s\n",N->line_num, tempErrorString.c_str(),tempErrorStringB.c_str());
        // }
            
        //TODO type must equal argument
        // TODO const expressions are allowed
        //TODO variable must not have been declared before in current scope
        //set variable type for checking stuff later
        // add to variable table
        break;
    case ASSIGN_STATEMENT:
        // TODO match operators
        // TODO match input variables/literal types - aka look up scope table
        // consts cannot be assigned, only declared.
        // make sure the variable exists
        // check variable attributes ( see "Pre Defined Variables")
        break;
    case IF_STATEMENT:
        //TODO check if bool is used
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

    //if leaf node, simply return?

    // recursively traverse tree

    // check if symatic is correct
    //yes - > return OK
    // no -> return not OK

    semantic_fail = 0; // 0 means OK, 1 means FAIL
}