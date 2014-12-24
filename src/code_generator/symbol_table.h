#ifndef _SYMBOL_TABLE_H_
#define _SYMBOL_TABLE_H_

#include <map>
#include <string>
#include <vector>

#include "../ast/types/Function.h"
#include "../ast/types/Type.h"
#include "FunctionEntry.h"
#include "LabelEntry.h"
#include "ValueEntry.h"

namespace code_generator {

class ValueEntry;
class LabelEntry;

class SymbolTable {
public:
    SymbolTable();
    ~SymbolTable();

    int insert(std::string name, ast::Type type, unsigned line);
    FunctionEntry insertFunction(std::string name, ast::Function functionType, unsigned line);
    FunctionEntry findFunction(std::string name) const;
    void insertFunctionArgument(std::string name, ast::Type type, unsigned line);
    bool hasSymbol(std::string symbolName) const;
    ValueEntry lookup(std::string name) const;
    ValueEntry newTemp(ast::Type type);
    LabelEntry newLabel();
    SymbolTable *newScope();
    SymbolTable *getOuterScope() const;

    unsigned getTableSize() const;

    void printTable() const;

private:
    SymbolTable(SymbolTable *outer);

    std::string generateTempName();
    std::string generateLabelName();

    static const std::string TEMP_PREFIX;
    static const std::string LABEL_PREFIX;

    std::map<std::string, FunctionEntry> functions;
    std::map<std::string, ValueEntry> values;
    std::map<std::string, LabelEntry> labels;
    SymbolTable *outer_scope;
    std::vector<SymbolTable *> inner_scopes;
    unsigned nextTemp { 0 };
    unsigned nextLabel { 0 };
    unsigned offset;
    unsigned paramOffset;
};

}

#endif // _SYMBOL_TABLE_H_
