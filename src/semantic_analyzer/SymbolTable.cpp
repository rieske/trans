#include "SymbolTable.h"

namespace {

const std::string LABEL_PREFIX = "__L";
unsigned nextLabel { 0 };

// Not a C identifier; legal non-local label in NASM and gas.
const std::string CONSTANT_PREFIX = "__str";
unsigned nextConstant { 0 };

std::string generateLabelName() {
    return LABEL_PREFIX + std::to_string(++nextLabel);
}

std::string generateConstantName() {
    return CONSTANT_PREFIX + std::to_string(++nextConstant);
}

} // namespace

namespace semantic_analyzer {

using symbols::ValueEntry;
using symbols::LabelEntry;
using symbols::FunctionEntry;


const std::string SymbolTable::SCOPE_PREFIX = "__s";

bool SymbolTable::insertSymbol(std::string name, const type::Type& type, translation_unit::Context context,
        symbols::Storage storage) {
    if (isAtFileScope()) {
        if (hasFunction(name)) {
            return false;
        }
        return globalScope.insertSymbol(name, type, context, storage, name);
    }

    const std::string scoped = scopePrefix(currentScopeId()) + name;
    if (functionScopes.back().isSymbolDefined(scoped) || globalScope.isSymbolDefined(scoped)) {
        return false;
    }
    if (storage == symbols::Storage::Static) {
        return globalScope.insertSymbol(scoped, type, context, symbols::Storage::Static, scoped);
    }
    if (storage == symbols::Storage::Extern) {
        if (hasFunction(name)) {
            return false;
        }
        // Bind the TU object first so a type mismatch leaves no function-scope marker.
        if (bindFileScopeObject(name, type, context, symbols::Storage::Extern, false)
                != ObjectBind::Bound) {
            return false;
        }
        return functionScopes.back().insertSymbol(scoped, type, context, symbols::Storage::Extern, name);
    }
    return functionScopes.back().insertSymbol(scoped, type, context, symbols::Storage::Automatic, scoped);
}

ObjectBind SymbolTable::bindFileScopeObject(std::string name, const type::Type& type,
        translation_unit::Context context, symbols::Storage storage, bool hasInitializer) {
    if (hasFunction(name)) {
        return ObjectBind::TypeConflict;
    }
    if (globalScope.insertSymbol(name, type, context, storage, name)) {
        if (hasInitializer) {
            globalScope.markDefiningInitializer(name);
        }
        return ObjectBind::Bound;
    }
    const ValueEntry existing = globalScope.lookup(name);
    if (!existing.isStatic() && storage == symbols::Storage::Static) {
        return ObjectBind::StaticAfterNonStatic;
    }
    if (existing.isStatic() && storage == symbols::Storage::Global) {
        return ObjectBind::NonStaticAfterStatic;
    }
    const auto merged = existing.getType().composite(type);
    if (!merged) {
        return ObjectBind::TypeConflict;
    }
    if (hasInitializer && existing.hasDefiningInitializer()) {
        return ObjectBind::SecondDefinition;
    }
    if (!merged->sameQualifiedType(existing.getType())) {
        globalScope.setSymbolType(name, *merged);
    }
    if (existing.isExtern() && storage == symbols::Storage::Global) {
        globalScope.promoteExternToDefinition(name);
    }
    if (hasInitializer) {
        globalScope.markDefiningInitializer(name);
    }
    return ObjectBind::Bound;
}

std::string SymbolTable::newConstant(const std::string& value) {
    std::string constantSymbol = generateConstantName();
    constants.insert({constantSymbol, value});
    return constantSymbol;
}

void SymbolTable::insertFunctionArgument(std::string name, type::Type type, translation_unit::Context context) {
    // Abstract parameters have an empty name; give each a unique scoped key so multiple
    // abstract formals do not collapse to a single symbol-table slot.
    std::string scopedName;
    if (name.empty()) {
        scopedName = scopePrefix(currentScopeId()) + "__arg" + std::to_string(functionScopes.back().getArguments().size());
    } else {
        scopedName = scopePrefix(currentScopeId()) + name;
    }
    functionScopes.back().insertFunctionArgument(scopedName, type, context);
}

FunctionEntry SymbolTable::insertFunction(std::string name, type::Function functionType, translation_unit::Context context,
        bool internalLinkage) {
    FunctionEntry function { name, functionType, context, internalLinkage };
    functions.insert(std::make_pair(name, function));
    return functions.at(name);
}

FunctionEntry SymbolTable::updateFunction(std::string name, type::Function functionType, translation_unit::Context context) {
    const bool internalLinkage = functions.at(name).hasInternalLinkage();
    const bool defined = functions.at(name).isDefined();
    FunctionEntry entry { name, std::move(functionType), std::move(context), internalLinkage };
    if (defined) {
        entry.markDefined();
    }
    functions.insert_or_assign(name, entry);
    return functions.at(name);
}

FunctionEntry SymbolTable::findFunction(std::string name) const {
    return functions.at(name);
}

bool SymbolTable::isFunctionDefined(const std::string& name) const {
    auto it = functions.find(name);
    return it != functions.end() && it->second.isDefined();
}

void SymbolTable::markFunctionDefined(const std::string& name) {
    functions.at(name).markDefined();
}

bool SymbolTable::hasFunction(const std::string& name) const {
    return functions.find(name) != functions.end();
}

bool SymbolTable::isAtFileScope() const {
    return scopeIdStack.empty();
}

bool SymbolTable::hasGlobalVariable(const std::string& name) const {
    return globalScope.isSymbolDefined(name);
}

void SymbolTable::setGlobalInitializer(const std::string& name, symbols::GlobalInitializer init) {
    if (globalScope.isSymbolDefined(name)) {
        globalScope.setGlobalInitializer(name, std::move(init));
        return;
    }
    if (!functionScopes.empty()) {
        for (auto it = scopeIdStack.rbegin(); it != scopeIdStack.rend(); ++it) {
            const std::string scoped = scopePrefix(*it) + name;
            if (globalScope.isSymbolDefined(scoped)) {
                globalScope.setGlobalInitializer(scoped, std::move(init));
                return;
            }
        }
    }
    globalScope.setGlobalInitializer(name, std::move(init));
}

void SymbolTable::setType(const std::string& name, const type::Type& type) {
    if (!functionScopes.empty()) {
        for (auto it = scopeIdStack.rbegin(); it != scopeIdStack.rend(); ++it) {
            const std::string scoped = scopePrefix(*it) + name;
            if (functionScopes.back().isSymbolDefined(scoped)) {
                functionScopes.back().setSymbolType(scoped, type);
                return;
            }
            if (globalScope.isSymbolDefined(scoped)) {
                globalScope.setSymbolType(scoped, type);
                return;
            }
        }
    }
    if (globalScope.isSymbolDefined(name)) {
        globalScope.setSymbolType(name, type);
    }
}

bool SymbolTable::defineEnumConstant(const std::string& name, long value) {
    if (enumConstants.find(name) != enumConstants.end()) {
        return false;
    }
    if (hasSymbol(name)) {
        return false;
    }
    enumConstants[name] = value;
    return true;
}

bool SymbolTable::hasEnumConstant(const std::string& name) const {
    return enumConstants.find(name) != enumConstants.end();
}

long SymbolTable::getEnumConstant(const std::string& name) const {
    return enumConstants.at(name);
}

bool SymbolTable::hasSymbol(std::string symbolName) const {
    if (!functionScopes.empty()) {
        for (auto it = scopeIdStack.rbegin(); it != scopeIdStack.rend(); ++it) {
            const std::string scoped = scopePrefix(*it) + symbolName;
            if (functionScopes.back().isSymbolDefined(scoped)
                    || globalScope.isSymbolDefined(scoped)) {
                return true;
            }
        }
    }
    return globalScope.isSymbolDefined(symbolName);
}

ValueEntry SymbolTable::lookup(std::string name) const {
    if (!functionScopes.empty()) {
        for (auto it = scopeIdStack.rbegin(); it != scopeIdStack.rend(); ++it) {
            const std::string scoped = scopePrefix(*it) + name;
            if (functionScopes.back().isSymbolDefined(scoped)) {
                return functionScopes.back().lookup(scoped);
            }
            if (globalScope.isSymbolDefined(scoped)) {
                return globalScope.lookup(scoped);
            }
        }
    }
    return globalScope.lookup(name);
}

ValueEntry SymbolTable::createTemporarySymbol(type::Type type) {
    if (isAtFileScope()) {
        return globalScope.createTemporarySymbol(type);
    }
    return functionScopes.back().createTemporarySymbol(type);
}

LabelEntry SymbolTable::newLabel() {
    std::string labelName = generateLabelName();
    LabelEntry label { labelName };
    labels.insert(std::make_pair(labelName, label));
    return label;
}

void SymbolTable::startFunction(std::string name, std::vector<std::string> formalArguments) {
    functionScopes.push_back(ValueScope { });
    scopeIdStack.clear();
    scopeIdStack.push_back(++nextScopeId);
    auto function = findFunction(name);
    size_t i { 0 };
    for (auto& argument : function.arguments()) {
        if (i < formalArguments.size()) {
            insertFunctionArgument(formalArguments.at(i), argument, function.getContext());
        }
        ++i;
    }
}

void SymbolTable::endFunction() {
    if (!functionScopes.empty()) {
        functionScopes.pop_back();
    }
    scopeIdStack.clear();
}

void SymbolTable::enterBlockScope() {
    scopeIdStack.push_back(++nextScopeId);
}

void SymbolTable::exitBlockScope() {
    if (!scopeIdStack.empty()) {
        scopeIdStack.pop_back();
    }
}

unsigned SymbolTable::currentScopeId() const {
    return scopeIdStack.back();
}

std::map<std::string, ValueEntry> SymbolTable::getCurrentScopeSymbols() const {
    std::map<std::string, ValueEntry> symbols;
    for (const auto& entry : functionScopes.back().getSymbols()) {
        if (entry.second.isGlobal()) {
            continue;
        }
        symbols.insert(entry);
    }
    return symbols;
}

std::vector<ValueEntry> SymbolTable::getCurrentScopeArguments() const {
    return functionScopes.back().getArguments();
}

std::map<std::string, std::string> SymbolTable::getConstants() const {
    return constants;
}

std::vector<ValueEntry> SymbolTable::getDataHomes() const {
    std::vector<ValueEntry> objects;
    for (const auto& entry : globalScope.getSymbols()) {
        if (!entry.second.isGlobal()) {
            continue;
        }
        objects.push_back(entry.second);
    }
    return objects;
}

std::string SymbolTable::scopePrefix(unsigned scopeId) const {
    return SCOPE_PREFIX + std::to_string(scopeId);
}

} // namespace semantic_analyzer
