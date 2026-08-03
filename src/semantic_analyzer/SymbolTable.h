#ifndef _SYMBOL_TABLE_H_
#define _SYMBOL_TABLE_H_

#include <map>
#include <string>
#include <vector>

#include "types/Type.h"
#include "symbols/FunctionEntry.h"
#include "symbols/LabelEntry.h"
#include "symbols/ValueEntry.h"
#include "ValueScope.h"

namespace semantic_analyzer {

using symbols::ValueEntry;
using symbols::LabelEntry;
using symbols::FunctionEntry;

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
    // TU-level object: first insert, or merge a compatible redecl (extern then definition).
    // Also used for block-scope extern, which names the same file-scope object.
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
    // Static-storage objects in this TU: file-scope Global/Static/Extern and
    // function-scope statics (mangled homes in globalScope).
    std::vector<ValueEntry> getDataHomes() const;
    void setGlobalInitializer(const std::string& name, symbols::GlobalInitializer init);
    void setType(const std::string& name, const type::Type& type);
    bool hasFunction(const std::string& name) const;
    bool hasGlobalVariable(const std::string& name) const;
    bool isAtFileScope() const;

private:
    void insertFunctionArgument(std::string name, type::Type type, translation_unit::Context context);

    std::map<std::string, FunctionEntry> functions;
    std::map<std::string, LabelEntry> labels;
    std::map<std::string, std::string> constants;
    std::map<std::string, long> enumConstants;

    std::vector<ValueScope> functionScopes;
    ValueScope globalScope;

    // Stack of monotonic scope ids (siblings never reuse an id).
    unsigned nextScopeId { 0 };
    std::vector<unsigned> scopeIdStack;

    std::string scopePrefix(unsigned scopeId) const;
    unsigned currentScopeId() const;

    static const std::string SCOPE_PREFIX;
};

} // namespace semantic_analyzer

#endif // _SYMBOL_TABLE_H_
