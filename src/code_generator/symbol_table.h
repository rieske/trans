#ifndef _SYMBOL_TABLE_H_
#define _SYMBOL_TABLE_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "../ast/types/Function.h"
#include "../ast/types/Type.h"
#include "FunctionEntry.h"
#include "LabelEntry.h"

namespace code_generator {
class ValueEntry;
class ValueScope;
} /* namespace code_generator */

namespace code_generator {

class SymbolTable {
public:
    SymbolTable();
    ~SymbolTable();

    bool insertSymbol(std::string name, ast::Type type, translation_unit::Context context);
    void insertFunctionArgument(std::string name, ast::Type type, translation_unit::Context context);
    FunctionEntry insertFunction(std::string name, ast::Function functionType, translation_unit::Context line);
    FunctionEntry findFunction(std::string name) const;
    bool hasSymbol(std::string symbolName) const;
    ValueEntry lookup(std::string name) const;
    ValueEntry createTemporarySymbol(ast::Type type);
    LabelEntry newLabel();
    void startScope();
    void endScope();

    std::map<std::string, ValueEntry> getCurrentScopeSymbols() const;
    std::map<std::string, ValueEntry> getCurrentScopeArguments() const;

    void printTable() const;

private:
    std::map<std::string, FunctionEntry> functions;
    std::map<std::string, LabelEntry> labels;

    std::vector<std::unique_ptr<ValueScope>> valueScopes;
    ValueScope* currentScope { nullptr };
};

}

#endif // _SYMBOL_TABLE_H_
