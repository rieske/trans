#include "SymbolTable.h"

#include <cassert>
#include <stdexcept>

namespace {

const std::string LABEL_PREFIX = "__L";
unsigned nextLabel { 0 };

// Not a C identifier; legal non-local label in NASM and gas.
const std::string CONSTANT_PREFIX = "L$str";
unsigned nextConstant { 0 };

const std::string UNNAMED_STATIC_PREFIX = "L$cl";
unsigned nextUnnamedStatic { 0 };

std::string generateLabelName() {
    return LABEL_PREFIX + std::to_string(++nextLabel);
}

std::string generateConstantName() {
    return CONSTANT_PREFIX + std::to_string(++nextConstant);
}

std::string generateUnnamedStaticName() {
    return UNNAMED_STATIC_PREFIX + std::to_string(++nextUnnamedStatic);
}

} // namespace

namespace semantic_analyzer {

namespace {

std::string localObjectName(unsigned scopeId, const std::string& source) {
    return "L$loc" + std::to_string(scopeId) + "_" + source;
}

} // namespace

bool SymbolTable::insertSymbol(std::string name, const type::Type& type, translation_unit::Context context,
        symbols::Storage storage) {
    if (isAtFileScope()) {
        return globalScope.insertSymbol({ 0, name }, type, context, storage, name, name);
    }
    const unsigned scopeId = currentScopeId();
    if (scopeId == scopeIdStack.front() && functionScopes.back().findArgumentBySource(name)) {
        return false;
    }
    std::string objectName = localObjectName(scopeId, name);
    if (storage == symbols::Storage::Static) {
        objectName = "L$st" + std::to_string(scopeId) + "_" + name;
    } else if (storage == symbols::Storage::Extern) {
        objectName = name;
        if (bindBlockScopeExtern(name, type, context) != ObjectBind::Bound) {
            return false;
        }
    }
    return functionScopes.back().insertSymbol(
            { scopeId, name }, type, context, storage, std::move(objectName), name);
}

ObjectBind SymbolTable::bindBlockScopeExtern(const std::string& name, const type::Type& type,
        translation_unit::Context context) {
    try {
        const symbols::ValueEntry existing = globalScope.lookup({ 0, name });
        if (existing.getType().isFunction()) {
            return ObjectBind::TypeConflict;
        }
        const auto merged = existing.getType().composite(type);
        if (!merged) {
            return ObjectBind::TypeConflict;
        }
        if (!merged->sameQualifiedType(existing.getType())) {
            globalScope.refineType({ 0, name }, *merged);
        }
        return ObjectBind::Bound;
    } catch (std::out_of_range&) {
        globalScope.insertSymbol({ 0, name }, type, context, symbols::Storage::Extern, name, name);
        return ObjectBind::Bound;
    }
}

std::string SymbolTable::newConstant(const std::string& value) {
    std::string constantSymbol = generateConstantName();
    constants.insert({constantSymbol, value});
    return constantSymbol;
}

symbols::ValueEntry SymbolTable::createUnnamedStaticObject(type::Type type, translation_unit::Context context) {
    const std::string name = generateUnnamedStaticName();
    if (!globalScope.insertSymbol({ 0, name }, type, context, symbols::Storage::Static, name, {})) {
        throw std::logic_error("duplicate unnamed static object name: " + name);
    }
    return globalScope.lookup({ 0, name });
}

void SymbolTable::insertFunctionArgument(std::string name, type::Type type, translation_unit::Context context) {
    // Abstract parameters have an empty name; give each a unique source key so multiple
    // abstract formals do not collapse to a single symbol-table slot.
    std::string source = name;
    if (source.empty()) {
        source = "__arg" + std::to_string(functionScopes.back().getArguments().size());
    }
    const std::string objectName = localObjectName(currentScopeId(), source);
    functionScopes.back().insertFunctionArgument(objectName, type, context, std::move(source));
}

symbols::FunctionEntry SymbolTable::insertFunction(std::string name, type::Function functionType, translation_unit::Context context,
        bool internalLinkage) {
    // Dual table invariant: every function is both
    //   1) functions[name] -> symbols::FunctionEntry (return type, formals, linkage, designator metadata)
    //   2) global symbols::ValueEntry with bare function type (ordinary-identifier visibility / hiding)
    // hasFunction and bare-function symbols::ValueEntry lookup must agree for names written here.
    // Parameters never use this path; they are adjustedParameterType to pointer-to-function.
    symbols::FunctionEntry function { name, functionType, context, internalLinkage };
    functions.insert(std::make_pair(name, function));
    globalScope.insertSymbol({ 0, function.getName() },
            type::function(functionType.getReturnType(), functionType.getArguments()), function.getContext(),
            symbols::Storage::Global, function.getName(), function.getName());
    return functions.at(name);
}

symbols::FunctionEntry SymbolTable::updateFunction(std::string name, type::Function functionType, translation_unit::Context context) {
    const bool internalLinkage = functions.at(name).hasInternalLinkage();
    symbols::FunctionEntry entry { name, std::move(functionType), context, internalLinkage };
    functions.insert_or_assign(name, entry);
    return functions.at(name);
}

symbols::FunctionEntry SymbolTable::findFunction(std::string name) const {
    return functions.at(name);
}

bool SymbolTable::hasFunction(const std::string& name) const {
    return functions.find(name) != functions.end();
}

bool SymbolTable::isFunctionDefined(const std::string& name) const {
    return functionDefined.find(name) != functionDefined.end();
}

void SymbolTable::markFunctionDefined(const std::string& name) {
    functionDefined.insert(name);
}

bool SymbolTable::isAtFileScope() const {
    return scopeIdStack.empty();
}

bool SymbolTable::hasGlobalVariable(const std::string& name) const {
    try {
        return !globalScope.lookup({ 0, name }).getType().isFunction();
    } catch (std::out_of_range&) {
        return false;
    }
}

void SymbolTable::setStaticInit(const std::string& name, std::vector<symbols::StaticInitValue> words) {
    if (!isAtFileScope()) {
        const SymbolKey key { currentScopeId(), name };
        if (functionScopes.back().contains(key)) {
            functionScopes.back().setStaticInit(key, std::move(words));
            return;
        }
    }
    globalScope.setStaticInit({ 0, name }, std::move(words));
}

ObjectBind SymbolTable::bindFileScopeObject(std::string name, const type::Type& type,
        translation_unit::Context context, symbols::Storage storage, bool hasInitializer) {
    assert(isAtFileScope());
    if (globalScope.insertSymbol({ 0, name }, type, context, storage, name, name)) {
        if (hasInitializer) {
            globalScope.markDefiningInitializer({ 0, name });
        }
        return ObjectBind::Bound;
    }
    const symbols::ValueEntry existing = globalScope.lookup({ 0, name });
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
        globalScope.refineType({ 0, name }, *merged);
    }
    if (existing.isExtern() && storage == symbols::Storage::Global) {
        globalScope.promoteExternToDefinition({ 0, name });
    }
    if (hasInitializer) {
        globalScope.markDefiningInitializer({ 0, name });
    }
    return ObjectBind::Bound;
}

bool SymbolTable::hasSymbol(std::string symbolName) const {
    try {
        lookup(symbolName);
        return true;
    } catch (std::out_of_range&) {
        return false;
    }
}

symbols::ValueEntry SymbolTable::lookup(std::string name) const {
    if (!functionScopes.empty()) {
        for (auto it = scopeIdStack.rbegin(); it != scopeIdStack.rend(); ++it) {
            const SymbolKey key { *it, name };
            if (functionScopes.back().contains(key)) {
                return functionScopes.back().lookup(key);
            }
        }
        if (const symbols::ValueEntry* argument = functionScopes.back().findArgumentBySource(name)) {
            return *argument;
        }
    }
    return globalScope.lookup({ 0, name });
}

symbols::ValueEntry SymbolTable::createTemporarySymbol(type::Type type) {
    if (functionScopes.empty()) {
        return globalScope.createTemporarySymbol(type);
    }
    return functionScopes.back().createTemporarySymbol(type);
}

symbols::LabelEntry SymbolTable::newLabel() {
    std::string labelName = generateLabelName();
    symbols::LabelEntry label { labelName };
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
    for (const auto& entry : functionScopes.back().getSymbols()) {
        if (entry.second.isStatic()) {
            functionScopeDataHomes.push_back(entry.second);
        }
    }
    functionScopes.pop_back();
    scopeIdStack.clear();
}

void SymbolTable::enterBlockScope() {
    scopeIdStack.push_back(++nextScopeId);
}

void SymbolTable::exitBlockScope() {
    scopeIdStack.pop_back();
}

unsigned SymbolTable::currentScopeId() const {
    return scopeIdStack.back();
}

std::map<std::string, symbols::ValueEntry> SymbolTable::getCurrentScopeSymbols() const {
    std::map<std::string, symbols::ValueEntry> symbols;
    for (const auto& entry : functionScopes.back().getSymbols()) {
        // Automatic only for frame locals (non-automatic use data homes).
        if (!entry.second.isGlobal()) {
            symbols.emplace(entry.second.getName(), entry.second);
        }
    }
    return symbols;
}

std::vector<symbols::ValueEntry> SymbolTable::getCurrentScopeArguments() const {
    return functionScopes.back().getArguments();
}

std::map<std::string, std::string> SymbolTable::getConstants() const {
    return constants;
}

std::vector<symbols::ValueEntry> SymbolTable::getDataHomes() const {
    std::vector<symbols::ValueEntry> objects;
    for (const auto& entry : globalScope.getSymbols()) {
        if (!entry.second.isGlobal() || entry.second.getType().isFunction()) {
            continue;
        }
        objects.push_back(entry.second);
    }
    objects.insert(objects.end(), functionScopeDataHomes.begin(), functionScopeDataHomes.end());
    return objects;
}

} // namespace semantic_analyzer

