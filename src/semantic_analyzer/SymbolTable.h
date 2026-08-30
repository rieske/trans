#ifndef _SYMBOL_TABLE_H_
#define _SYMBOL_TABLE_H_

#include <map>
#include <set>
#include <string>
#include <vector>

#include "types/Type.h"
#include "symbols/FunctionEntry.h"
#include "symbols/LabelEntry.h"
#include "symbols/ValueEntry.h"
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
    // Unnamed static-duration object. Always a TU data home.
    symbols::ValueEntry createUnnamedStaticObject(type::Type type, translation_unit::Context context);
    // Writes functions[name] and a global bare-function symbols::ValueEntry (dual-table invariant).
    symbols::FunctionEntry insertFunction(std::string name, type::Function functionType, translation_unit::Context line,
            bool internalLinkage = false);
    symbols::FunctionEntry updateFunction(std::string name, type::Function functionType, translation_unit::Context line);
    symbols::FunctionEntry findFunction(std::string name) const;
    bool isFunctionDefined(const std::string& name) const;
    void markFunctionDefined(const std::string& name);
    bool hasSymbol(std::string symbolName) const;
    symbols::ValueEntry lookup(std::string name) const;
    symbols::ValueEntry createTemporarySymbol(type::Type type);
    symbols::LabelEntry newLabel();
    void startFunction(std::string name, std::vector<std::string> formalArguments);
    void endFunction();
    void enterBlockScope();
    void exitBlockScope();

    std::map<std::string, symbols::ValueEntry> getCurrentScopeSymbols() const;
    std::vector<symbols::ValueEntry> getCurrentScopeArguments() const;
    std::map<std::string, std::string> getConstants() const;
    std::vector<symbols::ValueEntry> getDataHomes() const;
    // Current-scope object by source name, else a TU data home of that exact name.
    void setStaticInit(const std::string& name, std::vector<symbols::StaticInitValue> words);
    bool hasFunction(const std::string& name) const;
    bool hasGlobalVariable(const std::string& name) const;
    bool isAtFileScope() const;

private:
    void insertFunctionArgument(std::string name, type::Type type, translation_unit::Context context);
    // Block-scope extern is the file-scope object of that name (create if missing).
    ObjectBind bindBlockScopeExtern(const std::string& name, const type::Type& type,
            translation_unit::Context context);

    std::map<std::string, symbols::FunctionEntry> functions;
    std::set<std::string> functionDefined;
    std::map<std::string, symbols::LabelEntry> labels;
    std::map<std::string, std::string> constants;

    std::vector<ValueScope> functionScopes;
    ValueScope globalScope;
    std::vector<symbols::ValueEntry> functionScopeDataHomes;

    // Stack of monotonic scope ids (siblings never reuse an id).
    unsigned nextScopeId { 0 };
    std::vector<unsigned> scopeIdStack;

    unsigned currentScopeId() const;
};

} // namespace semantic_analyzer

#endif // _SYMBOL_TABLE_H_
