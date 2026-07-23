#ifndef _SYMBOL_TABLE_H_
#define _SYMBOL_TABLE_H_

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "types/Type.h"
#include "symbols/FunctionEntry.h"
#include "symbols/LabelEntry.h"
#include "symbols/ValueEntry.h"
#include "ValueScope.h"
#include "Symbols.h"

namespace semantic_analyzer {


class SymbolTable {
public:
    bool insertSymbol(std::string name, const type::Type& type, translation_unit::Context context);
    // Function-scope static: place in global .data/.bss under a mangled name
    // (static storage duration, not exported). Lookup still uses the bare name.
    bool insertStaticLocal(std::string name, const type::Type& type, translation_unit::Context context);
    // Block-scope extern: external linkage to a file-scope object (no stack storage).
    // Uses the bare name so codegen/linker resolve libc symbols (git: prep_childenv's environ).
    bool insertExternLocal(std::string name, const type::Type& type, translation_unit::Context context);
    std::string newConstant(const std::string& value);
    FunctionEntry insertFunction(std::string name, type::Function functionType, translation_unit::Context line);
    FunctionEntry findFunction(std::string name) const;
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
    // File-scope variables (isGlobal); constant init is on each ValueEntry when present.
    std::vector<ValueEntry> getGlobalVariables() const;
    void setGlobalInitializer(const std::string& name, long constantValue);
    void setGlobalStringInitializer(const std::string& name, std::string value);
    void setGlobalAddressInitializer(const std::string& name, std::string symbolName);
    void setGlobalMultiWordInitializer(const std::string& name, std::vector<std::string> words);
    void setGlobalType(const std::string& name, const type::Type& type);
    // Complete incomplete local arrays (e.g. struct option options[] = {...}).
    void setLocalType(const std::string& name, const type::Type& type);
    void setGlobalExternal(const std::string& name, bool value);
    void setGlobalStaticStorage(const std::string& name, bool value);
    bool hasFunction(const std::string& name) const;
    bool hasGlobalVariable(const std::string& name) const;
    bool isAtFileScope() const;

    void printTable() const;

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
