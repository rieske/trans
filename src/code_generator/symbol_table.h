#ifndef _SYMBOL_TABLE_H_
#define _SYMBOL_TABLE_H_

#include <map>
#include <string>
#include <vector>

#include "../ast/BasicType.h"
#include "../ast/TypeInfo.h"

namespace code_generator {

class SymbolEntry;
class ValueEntry;
class LabelEntry;

class SymbolTable {
public:
    SymbolTable();
    ~SymbolTable();

    int insert(std::string name, ast::TypeInfo typeInfo, unsigned line);
    void insertParam(std::string name, ast::TypeInfo typeInfo, unsigned line);
    bool hasSymbol(std::string symbolName) const;
    ValueEntry *lookup(std::string name) const;
    ValueEntry *newTemp(ast::TypeInfo typeInfo);
    LabelEntry *newLabel();
    SymbolTable *newScope();
    SymbolTable *getOuterScope() const;
    std::vector<SymbolTable *> getInnerScopes() const;

    std::string typeCheck(ValueEntry *v1, ValueEntry *v2);

    unsigned getTableSize() const;

    void printTable() const;

    SymbolTable *nextScope();

private:
    SymbolTable(const SymbolTable *outer);

    void addOffset(unsigned extra);
    void removeOffset(unsigned extra);

    void generateTempName();
    void generateLabelName();

    std::string decorate(ast::BasicType type, int etype);

    std::map<std::string, ValueEntry*> symbols;
    std::map<std::string, LabelEntry*> labels;
    SymbolTable *outer_scope;
    std::vector<SymbolTable *> inner_scopes;
    std::vector<SymbolTable *>::iterator scopeIt;
    std::string nextTemp;
    std::string *nextLabel;
    unsigned offset;
    unsigned paramOffset;
};

}

#endif // _SYMBOL_TABLE_H_
