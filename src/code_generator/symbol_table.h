#ifndef _SYMBOL_TABLE_H_
#define _SYMBOL_TABLE_H_

#include <map>
#include <string>
#include <vector>

#include "../ast/types/BaseType.h"
#include "../ast/types/Type.h"

namespace code_generator {

class SymbolEntry;
class ValueEntry;
class LabelEntry;

class SymbolTable {
public:
    SymbolTable();
    ~SymbolTable();

    int insert(std::string name, ast::Type typeInfo, unsigned line);
    void insertParam(std::string name, ast::Type typeInfo, unsigned line);
    bool hasSymbol(std::string symbolName) const;
    ValueEntry *lookup(std::string name) const;
    ValueEntry *newTemp(ast::Type typeInfo);
    LabelEntry *newLabel();
    SymbolTable *newScope();
    SymbolTable *getOuterScope() const;

    unsigned getTableSize() const;

    void printTable() const;

private:
    SymbolTable(const SymbolTable *outer);

    void generateTempName();
    void generateLabelName();

    std::map<std::string, ValueEntry*> symbols;
    std::map<std::string, LabelEntry*> labels;
    SymbolTable *outer_scope;
    std::vector<SymbolTable *> inner_scopes;
    std::string nextTemp;
    std::string *nextLabel;
    unsigned offset;
    unsigned paramOffset;
};

}

#endif // _SYMBOL_TABLE_H_
