#ifndef _SYMBOL_TABLE_H_
#define _SYMBOL_TABLE_H_

#include <map>
#include <set>
#include <memory>
#include <string>
#include <vector>

#include "types/Type.h"
#include "FunctionEntry.h"
#include "LabelEntry.h"
#include "ValueEntry.h"
#include "ValueScope.h"

namespace semantic_analyzer {

enum class ObjectBind {
    Bound,
    TypeConflict,
    SecondDefinition,
    StaticAfterNonStatic,
    NonStaticAfterStatic
};

class SymbolTable {
public:
    bool insertSymbol(std::string name, const type::Type& type, translation_unit::Context context,
            symbols::Storage storage = symbols::Storage::Automatic);
    // File-scope object: first insert, or merge a compatible redecl (extern then definition).
    ObjectBind bindFileScopeObject(std::string name, const type::Type& type,
            translation_unit::Context context, symbols::Storage storage, bool hasInitializer);
    std::string newConstant(const std::string& value);
    FunctionEntry insertFunction(std::string name, type::Function functionType, translation_unit::Context line,
            bool internalLinkage = false);
    FunctionEntry updateFunction(std::string name, type::Function functionType, translation_unit::Context line);
    FunctionEntry findFunction(std::string name) const;
    bool isFunctionDefined(const std::string& name) const;
    void markFunctionDefined(const std::string& name);
    bool hasSymbol(std::string symbolName) const;
    ValueEntry lookup(std::string name) const;
    // Enumerators: named integer constants (not storage-backed).
    // Product limit: TU-flat ordinary-namespace map (not C block-scoped enums).
    // Filled only via SemanticAnalyzer import of the AST parse snapshot
    // (session -> AST bag -> here); not a second write path from CSNB.
    bool defineEnumConstant(const std::string& name, long value);
    bool hasEnumConstant(const std::string& name) const;
    long getEnumConstant(const std::string& name) const;
    ValueEntry createTemporarySymbol(type::Type type);
    LabelEntry newLabel();
    void startFunction(std::string name, std::vector<std::string> formalArguments);
    void endFunction();
    void enterBlockScope();
    void exitBlockScope();

    std::map<std::string, ValueEntry> getCurrentScopeSymbols() const;
    std::vector<ValueEntry> getCurrentScopeArguments() const;
    std::map<std::string, std::string> getConstants() const;
    std::vector<ValueEntry> getDataHomes() const;
    void setStaticInit(const std::string& name, std::vector<symbols::StaticInitValue> words);
    bool hasFunction(const std::string& name) const;
    bool hasGlobalVariable(const std::string& name) const;
    bool isAtFileScope() const;

private:
    void insertFunctionArgument(std::string name, type::Type type, translation_unit::Context context);

    std::map<std::string, FunctionEntry> functions;
    std::set<std::string> functionDefined;
    std::map<std::string, LabelEntry> labels;
    std::map<std::string, std::string> constants;
    std::map<std::string, long> enumConstants;

    std::vector<ValueScope> functionScopes;
    ValueScope globalScope;
    std::vector<ValueEntry> functionScopeDataHomes;

    // Stack of monotonic scope ids (siblings never reuse an id).
    unsigned nextScopeId { 0 };
    std::vector<unsigned> scopeIdStack;

    std::string scopePrefix(unsigned scopeId) const;
    unsigned currentScopeId() const;

    static const std::string SCOPE_PREFIX;
};

} // namespace semantic_analyzer

#endif // _SYMBOL_TABLE_H_
