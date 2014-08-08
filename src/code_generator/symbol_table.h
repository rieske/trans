#ifndef _SYMBOL_TABLE_H_
#define _SYMBOL_TABLE_H_

#include <map>
#include <string>
#include <vector>

#include "../semantic_analyzer/BasicType.h"
#include "symbol_entry.h"

using std::map;
using std::vector;

const unsigned varSize = 4;

class SymbolTable {
public:
    SymbolTable();
    ~SymbolTable();

    int insert(string name, semantic_analyzer::BasicType basicType, string extended_type, unsigned line);
    int insertParam(string name, semantic_analyzer::BasicType basicType, string extended_type, unsigned line);
    bool hasSymbol(std::string symbolName) const;
    SymbolEntry *lookup(string name) const;
    SymbolEntry *newTemp(semantic_analyzer::BasicType basicType, string extended_type);
    SymbolEntry *newLabel();
    SymbolTable *newScope();
    SymbolTable *getOuterScope() const;
    vector<SymbolTable *> getInnerScopes() const;

    string typeCheck(SymbolEntry *v1, SymbolEntry *v2);

    unsigned getTableSize() const;

    void printTable() const;

    SymbolTable *next();

private:
    SymbolTable(const SymbolTable *outer);

    void addOffset(unsigned extra);
    void removeOffset(unsigned extra);

    void generateTempName();
    void generateLabelName();

    string decorate(semantic_analyzer::BasicType type, string etype);

    map<string, SymbolEntry *> symbols;
    SymbolTable *outer_scope;
    vector<SymbolTable *> inner_scopes;
    vector<SymbolTable *>::iterator scopeIt;
    string nextTemp;
    string *nextLabel;
    unsigned offset;
    unsigned paramOffset;
    unsigned labels;
};

#endif // _SYMBOL_TABLE_H_
