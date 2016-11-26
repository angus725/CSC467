#include "symbol.h"

using namespace std;

extern char* type_to_str(enum data_type dType);

std::unique_ptr<SymbolCactus> symbolCactus(new SymbolCactus);

Symbol::Symbol()
{
  var_type = TYPE_INVALID;
  attribute = 0;
  line_num = 0;
  name = "";
  isConstant = 0;
}

ostream& operator<<(ostream& os, const Symbol& symbol){

    os << "variable: " << std::string(type_to_str(symbol.var_type)) << " \"" << symbol.name << "\" declared at line " << symbol.line_num << " with attribute " << std::bitset<8>(symbol.attribute) << " is ";
    if(symbol.isConstant)
    	os << "a constant value.";
    else
    	os << "a variable value.";
    os << std::endl;
    return os;
}



SymbolCactus::SymbolCactus()
{
	symbolsTable.clear();
    std::unordered_map<std::string, Symbol> newTable;
    symbolsTable.push_back(newTable);

    symbolTableIt = symbolsTable.begin();
//    symbolTableIt++;
//    if(symbolTableIt != symbolsTable.end())
//    {
//    	cout << "here";
//    	abort();
//    }
//
//    symbolTableIt--;


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

//    std::cout << *this;
//    std::cout << "printed init" <<std::endl;
    pushScope();
}

SymbolCactus::~SymbolCactus()
{
    symbolsTable.clear(); // get rid of all the pointers to to-be-removed nodes
    //preDefNodes is automatically deallocated at end of runtime
}

ostream& operator<<(ostream& os, const SymbolCactus& symbolCactus){
	os << "Printing lowest symbol tree scope" << std::endl;

    auto symbolTableReIt = symbolCactus.symbolsTable.rbegin(); // reverse iterator
    while (symbolTableReIt != symbolCactus.symbolsTable.rend())
    {
    	for (auto itr = symbolTableReIt->begin(); itr != symbolTableReIt->end(); ++itr) {
    	    /* ... process *itr ... */
			os << itr->second;
    	}
		os << "Increasing scope..." << std::endl;
		symbolTableReIt++; // this should move the iterator backwards towards the head..
    }
    os << "Global scope was reached in the last print cycle, print complete." << std::endl;
    return os;
}


int SymbolCactus::insert(Symbol &symbol)
{

    if (symbolTableIt == symbolsTable.end() )
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
        auto search = (*symbolTableIt).find(identifer);

        if (search != (*symbolTableIt).end()) // found
            return &(search->second);          // search is a std::pair< key, value>, where value is Symbol*

        symbolTableReIt++; // this should move the iterator backwards towards the head..
    }

    return NULL;
}

void SymbolCactus::pushScope()
{
//    if (symbolTableIt == symbolsTable.end())
//    {
        std::unordered_map<std::string, Symbol> newTable;
        symbolsTable.push_back(newTable);
//        assert(symbolTableIt == symbolsTable.end()); // abort if the iterator is pointing to the end after a pushback
//        return;
//    }
      symbolTableIt = symbolsTable.end();
//      symbolTableIt++;
//      symbolTableIt++;
      if(symbolTableIt != symbolsTable.end())
    	  abort();
      symbolTableIt--;
//      if(symbolTableIt!=)

//    std::cout << "this aborted";
//    abort(); // symbolTableIt was not at 1 past end of list
}

void SymbolCactus::popScope()
{
    // assert(symbolTableIt == symbolsTable.begin());
    symbolsTable.pop_back();
    symbolTableIt--;
}
