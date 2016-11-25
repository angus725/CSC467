#ifndef _SYMBOL_H
#define _SYMBOL_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unordered_map>
#include <list>
#include <iterator>
#include <utility>
#include <string>
#include <memory>
#include <vector>
#include "ast.h"
#include <cassert>

using namespace std;

class SymbolCactus
{

  public:
    SymbolCactus();
    ~SymbolCactus();
    int insert(node *N); // see defines in symbol.c for return value
    node *find(const char *ID);
    node *find(std::string ID);
    void pushScope();
    void popScope();

  private:
    list<unordered_map<string, node *>> symbolsTable; // each node must be of type variable
    list<unordered_map<string, node *>>::iterator symbolTableIt;

    node* preDefNodes = new node[13];


    SymbolCactus(const SymbolCactus &);            // disable copy-construction
    SymbolCactus &operator=(const SymbolCactus &); // disable assignment
};



// std::list<int> ast;

#endif

// variable arrtibutes, use a bitmask
//      const, result, attribute, uniform, initialized