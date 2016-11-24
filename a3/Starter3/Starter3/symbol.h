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

// using namespace std;

struct node;

class Symbol
{
public:
    Symbol();
    ~Symbol();

private:
    node *N;
};

Symbol::Symbol()
{
}

Symbol::~Symbol()
{
}

std::list<
    std::unordered_map<std::string,Symbol> 
> symbolTable; // use unordered map instead of map

// std::list<int> ast;

#endif

// variable arrtibutes, use a bitmask
//      const, result, attribute, uniform, initialized