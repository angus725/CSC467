#include "symbol.h"

using namespace std;

std::unique_ptr<SymbolCactus> symbolCactus(new SymbolCactus);

Symbol::Symbol()
{
  var_type = TYPE_INVALID;
  attribute = 0;
  line_num = 0;
  name = "";
  isConstant = 0;
}

SymbolCactus::SymbolCactus()
{
    symbolTableIt = symbolsTable.begin();

    Symbol preDefNodes; // abusing the fact that insert uses the copy constuctor

    preDefNodes.var_type = TYPE_VEC4;
    preDefNodes.line_num = 0;
    preDefNodes.isConstant = 0;
    preDefNodes.name = "gl_FragColor";
    preDefNodes.attribute = RESULT;
    insert(preDefNodes);

    preDefNodes.name = "gl_FragCoord";
    insert(preDefNodes);

    preDefNodes.var_type = TYPE_BOOL;
    preDefNodes.name = "gl_FragDepth";
    insert(preDefNodes);

    preDefNodes.var_type = TYPE_VEC4;
    preDefNodes.name = "gl_TexCoord";
    preDefNodes.attribute = ATTRIBUTE | INITIALIZED;
    insert(preDefNodes);

    preDefNodes.name = "gl_Color";
    insert(preDefNodes);

    preDefNodes.name = "gl_Secondary";

    insert(preDefNodes);

    preDefNodes.name = "gl_FogFragCoord";
    insert(preDefNodes);

    preDefNodes.var_type = TYPE_VEC4;
    preDefNodes.isConstant = 1;
    preDefNodes.name = "gl_Light_Half";
    preDefNodes.attribute = UNIFORM | INITIALIZED;
    insert(preDefNodes);

    preDefNodes.name = "gl_Light_Ambient";
    insert(preDefNodes);

    preDefNodes.name = "gl_Material_Shininess";
    insert(preDefNodes);

    preDefNodes.name = "env1";
    insert(preDefNodes);

    preDefNodes.name = "env2";
    insert(preDefNodes);

    preDefNodes.name = "env3";
    insert(preDefNodes);

    pushScope(); // move from super-global scope to global scope
}

SymbolCactus::~SymbolCactus()
{
    symbolsTable.clear(); // get rid of all the pointers to to-be-removed nodes
    //preDefNodes is automatically deallocated at end of runtime
}



int SymbolCactus::insert(Symbol &symbol)
{

    if (symbolTableIt == symbolsTable.end())
        return ERROR_SCOPE_FAILURE;

    unordered_map<string, Symbol> *localSymbolTable = &(*symbolTableIt);

    auto findResult = localSymbolTable->find(symbol.name); // find returns a std::pair with strange syntax
    if (findResult != localSymbolTable->end())             // duplicate found
        return ERROR_DUPLICATE_VARIABLE;

    localSymbolTable->insert({symbol.name, symbol});
    return SUCCESS;
}

Symbol* SymbolCactus::find(const char *identifer)
{
    std::string sIdentifier(identifer);
    return find(sIdentifier);
}
Symbol* SymbolCactus::find(std::string identifer)
{
    list<unordered_map<string, Symbol>>::reverse_iterator symbolTableReIt = symbolsTable.rbegin(); // reverse iterator
    while (symbolTableReIt != symbolsTable.rend())
    {
        auto search = symbolTableIt->find(identifer);

        if (search != symbolTableIt->end()) // found
            return &(search->second);          // search is a std::pair< key, value>, where value is Symbol*

        symbolTableReIt++; // this should move the iterator backwards towards the head..
    }

    return NULL;
}

void SymbolCactus::pushScope()
{
    symbolTableIt++;
    if (symbolTableIt == symbolsTable.end())
    {
        std::unordered_map<std::string, Symbol> newTable;
        symbolsTable.push_back(newTable);
        assert(symbolTableIt == symbolsTable.end()); // abort if the iterator is pointing to the end after a pushback
    }
    // assert(0); // symbolTableIt was not at 1 past end of list
}

void SymbolCactus::popScope()
{
    // assert(symbolTableIt == symbolsTable.begin());
    symbolsTable.pop_back();
    symbolTableIt--;
}
