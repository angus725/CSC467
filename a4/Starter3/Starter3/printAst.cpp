#include "ast.h"

/*********************************************************************
***** 		all functions required to print the syntax tree		******
**********************************************************************/

void Scope::printSyntaxTree()
{
	printf("(SCOPE ");
	if (declarations)
		declarations->printSyntaxTree();
	if (statements)
		statements->printSyntaxTree();
	printf(")");
}

void MultiNode::printSyntaxTree()
{
	//printf("MULTI_NODE CONSTANT: %i\n", constantValue);

	switch (kind)
	{
	case DECLARATION:
		printf("(DECLARATIONS ");
		break;
	case ASSIGN_STATEMENT:
	case IF_STATEMENT:
		printf("(STATEMENTS ");
		break;
	default:
		break;
	}

	cur_node->printSyntaxTree();
	if (nodes)
		nodes->printSyntaxTree();
	if (cur_node->is_expression())
		return;


	printf(")");
}

void Declaration::printSyntaxTree()
{
	printf("(DECLARATION %s ", identifier.c_str());
	type->printSyntaxTree();
	if (expression)
		expression->printSyntaxTree();
	printf(")");
}

void IfStatement::printSyntaxTree()
{
	printf("(IF ");
	if_confition->printSyntaxTree();
	if_body->printSyntaxTree();
	if (else_body)
		else_body->printSyntaxTree();
	printf(")");
}

void AssignStatement::printSyntaxTree()
{
	printf("(ASSIGN ");
	variable->printSyntaxTree();
	expression->printSyntaxTree();
	printf(")");
}

void Type::printSyntaxTree()
{
	printf("%s ", type_to_str(var_type));
}

void Variable::printSyntaxTree()
{
	if (has_index)
	{
		printf("(INDEX %s %s %d ", type_to_str(var_type), identifier.c_str(), array_index);
	}
	else
		printf("%s %s ", type_to_str(var_type), identifier.c_str());
	if (has_index)
		return;
	printf(")");

}

void FunctionCall::printSyntaxTree()
{
	printf("(CALL %s ", func_to_str(func));
	args_opt->printSyntaxTree();
	printf(")");
}

void Constructor::printSyntaxTree()
{
	printf("(CALL ");
	type->printSyntaxTree();
	args_opt->printSyntaxTree();
	printf(")");
}

void UnaryOP::printSyntaxTree()
{
	printf("(UNARY %s %s ", type_to_str(result_type), uopt_to_string(uopt));
	operand->printSyntaxTree();
	printf(")");
}

void BinaryOP::printSyntaxTree()
{
	printf("(BINARY %s %s ", type_to_str(result_type), bopt_to_string(bopt));
	operand1->printSyntaxTree();
	operand2->printSyntaxTree();
	printf(")");
}

void LiteralExp::printSyntaxTree()
{
	switch (lit_type)
	{
	case TYPE_BOOL:
		printf("%s ", (val_bool) ? "true" : "false");
		break;
	case TYPE_INT:
		printf("%d ", val_int);
		break;
	case TYPE_FLOAT:
		printf("%f ", val_float);
		break;
	default:
		break;
	}
}

