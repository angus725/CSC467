#ifndef _SEMANTIC_H
#define _SEMANTIC_H

#include "ast.h"
#include "symbol.h"

int semantic_fail = 0; // default to not-fail
int nestedIfCount = 0; 

int semantic_check( node *ast); //wrapper
void pre_check(node* N);// does the actual checking
void check_semantic(node* N); // does the actual checking

#endif