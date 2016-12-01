#ifndef _SYMBOL_H
#define _SYMBOL_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unordered_map>
#include <iostream>
#include <list>
#include <iterator>
#include <utility>
#include <string>
#include <memory>
#include <vector>
#include "ast.h"
#include <cassert>
#include <bitset>

using namespace std;

#define RESULT 0x1
#define WRITE_ONLY 0x1
#define ATTRIBUTE 0x2
#define UNIFORM 0x4
#define INITIALIZED 0x8

class Symbol
{
public:
  Symbol();
  ~Symbol(){};
  // Symbol(const Symbol &obj); // copy constructor cannot copy over isDynamic

  enum data_type var_type;
  int8_t attribute;
  int line_num;
  std::string name;
  int isConstant;
  friend ostream& operator<<(ostream& os, const Symbol& symbol);

private:
};


// insert() returns
#define SUCCESS 0
#define ERROR_DUPLICATE_VARIABLE -1
#define ERROR_NOT_A_VARIABLE -2
#define ERROR_SCOPE_FAILURE -3


class SymbolCactus
{

public:
  SymbolCactus();
  ~SymbolCactus();
  int insert(Symbol &symbol); // see defines in symbol.c for return value
  Symbol *find(const char *identifer);
  Symbol *find(std::string identifer);
  void pushScope();
  void popScope();
  friend ostream& operator<<(ostream& os, const SymbolCactus& symbolCactus);

private:
  list<unordered_map<string, Symbol>> symbolsTable; // each Symbol must be of type variable
  list<unordered_map<string, Symbol>>::iterator symbolTableIt;

  SymbolCactus(const SymbolCactus &);            // disable copy-construction
  SymbolCactus &operator=(const SymbolCactus &); // disable assignment
};

// std::list<int> ast;

#endif

// variable arrtibutes, use a bitmask
//      const, result, attribute, uniform, initialized
