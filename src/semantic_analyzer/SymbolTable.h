#ifndef _SYMBOL_TABLE_H_
#define _SYMBOL_TABLE_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "types/Type.h"
#include "FunctionEntry.h"
#include "LabelEntry.h"
#include "ValueEntry.h"
#include "ValueScope.h"

namespace semantic_analyzer {

class SymbolTable {
public:
    bool insertSymbol(std::string name, const type::Type& type, translation_unit::Context context);
    std::string newConstant(const std::string& value);
    FunctionEntry insertFunction(std::string name, type::Function functionType, translation_unit::Context line);
    FunctionEntry findFunction(std::string name) const;
    bool hasSymbol(std::string symbolName) const;
    ValueEntry lookup(std::string name) const;
    ValueEntry createTemporarySymbol(type::Type type);
    LabelEntry newLabel();
    void startFunction(std::string name, std::vector<std::string> formalArguments);
    void endFunction();

    std::map<std::string, ValueEntry> getCurrentScopeSymbols() const;
    std::vector<ValueEntry> getCurrentScopeArguments() const;
    std::map<std::string, std::string> getConstants() const;

    void printTable() const;

private:
    void insertFunctionArgument(std::string name, type::Type type, translation_unit::Context context);

    std::map<std::string, FunctionEntry> functions;
    std::map<std::string, LabelEntry> labels;
    std::map<std::string, std::string> constants;

    std::vector<ValueScope> functionScopes;
    ValueScope globalScope;

    unsigned currentScopeIndex { 0 };
    std::string scopePrefix(unsigned scopeIndex) const;

    static const std::string SCOPE_PREFIX;
};

} // namespace semantic_analyzer

#endif // _SYMBOL_TABLE_H_
