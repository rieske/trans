#ifndef _SYMBOL_TABLE_H_
#define _SYMBOL_TABLE_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "types/FunctionType.h"
#include "types/FundamentalType.h"
#include "FunctionEntry.h"
#include "LabelEntry.h"
#include "ValueEntry.h"
#include "ValueScope.h"

namespace semantic_analyzer {

class SymbolTable {
public:
    bool insertSymbol(std::string name, const ast::FundamentalType& type, translation_unit::Context context);
    FunctionEntry insertFunction(std::string name, ast::FunctionType functionType, translation_unit::Context line);
    FunctionEntry findFunction(std::string name) const;
    bool hasSymbol(std::string symbolName) const;
    ValueEntry lookup(std::string name) const;
    ValueEntry createTemporarySymbol(std::unique_ptr<ast::FundamentalType> type);
    LabelEntry newLabel();
    void startFunction(std::string name, std::vector<std::string> formalArguments);
    void endFunction();

    std::map<std::string, ValueEntry> getCurrentScopeSymbols() const;
    std::vector<ValueEntry> getCurrentScopeArguments() const;

    void printTable() const;

private:
    void insertFunctionArgument(std::string name, ast::FundamentalType& type, translation_unit::Context context);

    std::map<std::string, FunctionEntry> functions;
    std::map<std::string, LabelEntry> labels;

    std::vector<ValueScope> functionScopes;
    ValueScope globalScope;

    unsigned currentScopeIndex { 0 };
    std::string scopePrefix(unsigned scopeIndex) const;

    static const std::string SCOPE_PREFIX;
};

}

#endif // _SYMBOL_TABLE_H_
