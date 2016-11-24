#include "symbol.h"

using namespace std;

std::unique_ptr<SymbolCactus> symbolCactus(new SymbolCactus);
std::vector<node> preDefNodes;

SymbolCactus::SymbolCactus()
{

    symbolTableIt = symbolsTable.begin();

    preDefNodes[0].kind = VARIABLE;
    preDefNodes[0].line_num = 0;
    preDefNodes[0].constantValue = 0;
    preDefNodes[0].variable = {"gl_FragColor", 0, 1, RESULT, TYPE_VEC4, "vec4"};
    insert(&(preDefNodes[0]));

    preDefNodes[1].kind = VARIABLE;
    preDefNodes[1].line_num = 0;
    preDefNodes[1].constantValue = 0;
    preDefNodes[1].variable = {"gl_FragCoord", 0, 1, RESULT, TYPE_VEC4, "vec4"};
    insert(&(preDefNodes[1]));

    preDefNodes[2].kind = VARIABLE;
    preDefNodes[2].line_num = 0;
    preDefNodes[2].constantValue = 0;
    preDefNodes[2].variable = {"gl_FragDepth", 0, 0, RESULT, TYPE_BOOL, "bool"}; // only bool here
    insert(&(preDefNodes[2]));

    preDefNodes[3].kind = VARIABLE;
    preDefNodes[3].line_num = 0;
    preDefNodes[3].constantValue = 0;
    preDefNodes[3].variable = {"gl_TexCoord", 0, 1, ATTRIBUTE | INITIALIZED, TYPE_VEC4, "vec4"};
    insert(&(preDefNodes[3]));

    preDefNodes[4].kind = VARIABLE;
    preDefNodes[4].line_num = 0;
    preDefNodes[4].constantValue = 0;
    preDefNodes[4].variable = {"gl_Color", 0, 1, ATTRIBUTE | INITIALIZED, TYPE_VEC4, "vec4"};
    insert(&(preDefNodes[4]));

    preDefNodes[5].kind = VARIABLE;
    preDefNodes[5].line_num = 0;
    preDefNodes[5].constantValue = 0;
    preDefNodes[5].variable = {"gl_Secondary", 0, 1, ATTRIBUTE | INITIALIZED, TYPE_VEC4, "vec4"};
    insert(&(preDefNodes[5]));

    preDefNodes[6].kind = VARIABLE;
    preDefNodes[6].line_num = 0;
    preDefNodes[6].constantValue = 0;
    preDefNodes[6].variable = {"gl_FogFragCoord", 0, 1, ATTRIBUTE | INITIALIZED, TYPE_VEC4, "vec4"};
    insert(&(preDefNodes[6]));

    preDefNodes[7].kind = VARIABLE;
    preDefNodes[7].line_num = 0;
    preDefNodes[7].constantValue = 1;
    preDefNodes[7].variable = {"gl_Light_Half", 0, 1, UNIFORM | INITIALIZED, TYPE_VEC4, "vec4"};
    insert(&(preDefNodes[7]));

    preDefNodes[8].kind = VARIABLE;
    preDefNodes[8].line_num = 0;
    preDefNodes[8].constantValue = 1;
    preDefNodes[8].variable = {"gl_Light_Ambient", 0, 1, UNIFORM | INITIALIZED, TYPE_VEC4, "vec4"};
    insert(&(preDefNodes[8]));

    preDefNodes[9].kind = VARIABLE;
    preDefNodes[9].line_num = 0;
    preDefNodes[9].constantValue = 1;
    preDefNodes[9].variable = {"gl_Material_Shininess", 0, 1, UNIFORM | INITIALIZED, TYPE_VEC4, "vec4"};
    insert(&(preDefNodes[9]));

    preDefNodes[10].kind = VARIABLE;
    preDefNodes[10].line_num = 0;
    preDefNodes[10].constantValue = 1;
    preDefNodes[10].variable = {"env1", 0, 1, UNIFORM | INITIALIZED, TYPE_VEC4, "vec4"};
    insert(&(preDefNodes[10]));

    preDefNodes[11].kind = VARIABLE;
    preDefNodes[11].line_num = 0;
    preDefNodes[11].constantValue = 1;
    preDefNodes[11].variable = {"env2", 0, 1, UNIFORM | INITIALIZED, TYPE_VEC4, "vec4"};
    insert(&(preDefNodes[11]));

    preDefNodes[12].kind = VARIABLE;
    preDefNodes[12].line_num = 0;
    preDefNodes[12].constantValue = 1;
    preDefNodes[12].variable = {"env3", 0, 1, UNIFORM | INITIALIZED, TYPE_VEC4, "vec4"};
    insert(&(preDefNodes[12]));

    pushScope(); // move from super-global scope to global scope
}

SymbolCactus::~SymbolCactus()
{
    symbolsTable.clear(); // get rid of all the pointers to to-be-removed nodes
    //preDefNodes is automatically deallocated at end of runtime
}

#define SUCCESS 0
#define ERROR_DUPLICATE_VARIABLE -1
#define ERROR_NOT_A_VARIABLE -2
#define ERROR_SCOPE_FAILURE -3

int SymbolCactus::insert(node *N)
{
    if (N->kind != VARIABLE)
        return ERROR_NOT_A_VARIABLE;
    std::string sIdentifier(N->variable.identifier);

    if (symbolTableIt == symbolsTable.end())
        return ERROR_SCOPE_FAILURE;

    unordered_map<string, node *> *localSymbolTable = &(*symbolTableIt);

    auto findResult = localSymbolTable->find(sIdentifier); // find returns a std::pair with strange syntax
    if (findResult != localSymbolTable->end()) // duplicate found
        return ERROR_DUPLICATE_VARIABLE;

    localSymbolTable->insert({sIdentifier, N});
    return SUCCESS;
}

node *SymbolCactus::find(const char *ID)
{
    std::string sIdentifier(ID);
    return find(sIdentifier);
}
node *SymbolCactus::find(std::string ID)
{
    auto search = symbolTableIt->find(ID);
    if (search == symbolTableIt->end()) // not found
        return NULL;
    return search->second; // search is a std::pair< key, value>, where value is node*
}

void SymbolCactus::pushScope()
{
    symbolTableIt++;
    if (symbolTableIt == symbolsTable.end())
    {
        std::unordered_map<std::string, node *> newTable;
        symbolsTable.push_back(newTable);
        assert(symbolTableIt != symbolsTable.end()); // abort if the iterator is pointing to the end after a pushback
    }
    assert(0); // symbolTableIt was not at 1 past end of list
}

void SymbolCactus::popScope()
{
    assert(symbolTableIt != symbolsTable.begin());
    symbolsTable.pop_back();
    symbolTableIt--;

}
