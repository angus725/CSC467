#ifndef _SEMANTIC_H
#define _SEMANTIC_H

#include "ast.h"
#include "symbol.h"
extern std::unique_ptr<SymbolCactus> symbolCactus;


int semantic_check( node *ast); //wrapper
void pre_check(node* N);// does the actual checking
void post_check(node* N); // does the actual checking

#endif