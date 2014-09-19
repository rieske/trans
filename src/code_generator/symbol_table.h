#ifndef _SYMBOL_TABLE_H_
#define _SYMBOL_TABLE_H_

#include <map>
#include <string>
#include <vector>

#include "../ast/BasicType.h"
#include "../ast/TypeInfo.h"
#include "symbol_entry.h"

using std::map;
using std::vector;

class SymbolTable {
public:
    SymbolTable();
    ~SymbolTable();

    int insert(string name, ast::TypeInfo typeInfo, unsigned line);
    void insertParam(string name, ast::TypeInfo typeInfo, unsigned line);
    bool hasSymbol(std::string symbolName) const;
    SymbolEntry *lookup(string name) const;
    SymbolEntry *newTemp(ast::TypeInfo typeInfo);
    SymbolEntry *newLabel();
    SymbolTable *newScope();
    SymbolTable *getOuterScope() const;
    vector<SymbolTable *> getInnerScopes() const;

    string typeCheck(SymbolEntry *v1, SymbolEntry *v2);

    unsigned getTableSize() const;

    void printTable() const;

    SymbolTable *nextScope();

private:
    SymbolTable(const SymbolTable *outer);

    void addOffset(unsigned extra);
    void removeOffset(unsigned extra);

    void generateTempName();
    void generateLabelName();

    string decorate(ast::BasicType type, int etype);

    std::map<std::string, SymbolEntry*> symbols;
    std::map<std::string, SymbolEntry*> labels;
    SymbolTable *outer_scope;
    vector<SymbolTable *> inner_scopes;
    vector<SymbolTable *>::iterator scopeIt;
    string nextTemp;
    string *nextLabel;
    unsigned offset;
    unsigned paramOffset;
};

#endif // _SYMBOL_TABLE_H_
