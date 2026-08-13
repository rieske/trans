#include "SymbolTable.h"

#include <cassert>

namespace {

const std::string LABEL_PREFIX = "__L";
unsigned nextLabel { 0 };

// Not a C identifier; legal non-local label in NASM and gas.
const std::string CONSTANT_PREFIX = "L$str";
unsigned nextConstant { 0 };

std::string generateLabelName() {
    return LABEL_PREFIX + std::to_string(++nextLabel);
}

std::string generateConstantName() {
    return CONSTANT_PREFIX + std::to_string(++nextConstant);
}

} // namespace

namespace semantic_analyzer {

const std::string SymbolTable::SCOPE_PREFIX = "$s";

bool SymbolTable::insertSymbol(std::string name, const type::Type& type, translation_unit::Context context,
        symbols::Storage storage) {
    if (isAtFileScope()) {
        return globalScope.insertSymbol(name, type, context, storage, name);
    }
    const std::string scoped = scopePrefix(currentScopeId()) + name;
    std::string objectName = scoped;
    if (storage == symbols::Storage::Static) {
        objectName = "L$st" + std::to_string(currentScopeId()) + "_" + name;
    } else if (storage == symbols::Storage::Extern) {
        // Block-scope extern has external linkage; the object name is the global symbol.
        objectName = name;
    }
    return functionScopes.back().insertSymbol(scoped, type, context, storage, std::move(objectName));
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
    // Dual table invariant: every function is both
    //   1) functions[name] -> FunctionEntry (return type, formals, linkage, designator metadata)
    //   2) global ValueEntry with bare function type (ordinary-identifier visibility / hiding)
    // hasFunction and bare-function ValueEntry lookup must agree for names written here.
    // Parameters never use this path; they are adjustedParameterType to pointer-to-function.
    FunctionEntry function { name, functionType, context, internalLinkage };
    functions.insert(std::make_pair(name, function));
    globalScope.insertSymbol(function.getName(),
            type::function(functionType.getReturnType(), functionType.getArguments()), function.getContext(),
            symbols::Storage::Global, function.getName());
    return functions.at(name);
}

FunctionEntry SymbolTable::updateFunction(std::string name, type::Function functionType, translation_unit::Context context) {
    const bool internalLinkage = functions.at(name).hasInternalLinkage();
    FunctionEntry entry { name, std::move(functionType), context, internalLinkage };
    functions.insert_or_assign(name, entry);
    return functions.at(name);
}

FunctionEntry SymbolTable::findFunction(std::string name) const {
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
        return !globalScope.lookup(name).getType().isFunction();
    } catch (std::out_of_range&) {
        return false;
    }
}

void SymbolTable::setStaticInit(const std::string& name, std::vector<symbols::StaticInitValue> words) {
    if (isAtFileScope()) {
        globalScope.setStaticInit(name, std::move(words));
        return;
    }
    functionScopes.back().setStaticInit(scopePrefix(currentScopeId()) + name, std::move(words));
}

ObjectBind SymbolTable::bindFileScopeObject(std::string name, const type::Type& type,
        translation_unit::Context context, symbols::Storage storage, bool hasInitializer) {
    assert(isAtFileScope());
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
        globalScope.refineType(name, *merged);
    }
    if (existing.isExtern() && storage == symbols::Storage::Global) {
        globalScope.promoteExternToDefinition(name);
    }
    if (hasInitializer) {
        globalScope.markDefiningInitializer(name);
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

ValueEntry SymbolTable::lookup(std::string name) const {
    if (!functionScopes.empty()) {
        for (auto it = scopeIdStack.rbegin(); it != scopeIdStack.rend(); ++it) {
            try {
                return functionScopes.back().lookup(scopePrefix(*it) + name);
            } catch (std::out_of_range&) {
            }
        }
    }
    return globalScope.lookup(name);
}

ValueEntry SymbolTable::createTemporarySymbol(type::Type type) {
    if (functionScopes.empty()) {
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
    for (const auto& entry : functionScopes.back().getSymbols()) {
        // Non-automatic symbols (static local, block-scope extern) use data homes.
        if (entry.second.isGlobal()) {
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

std::map<std::string, ValueEntry> SymbolTable::getCurrentScopeSymbols() const {
    std::map<std::string, ValueEntry> symbols;
    for (const auto& entry : functionScopes.back().getSymbols()) {
        // Automatic only for frame locals (non-automatic use data homes).
        if (!entry.second.isGlobal()) {
            symbols.insert(entry);
        }
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
        if (!entry.second.isGlobal() || entry.second.getType().isFunction()) {
            continue;
        }
        objects.push_back(entry.second);
    }
    objects.insert(objects.end(), functionScopeDataHomes.begin(), functionScopeDataHomes.end());
    return objects;
}

std::string SymbolTable::scopePrefix(unsigned scopeId) const {
    return SCOPE_PREFIX + std::to_string(scopeId);
}

bool SymbolTable::defineEnumConstant(const std::string& name, long value) {
    if (enumConstants.find(name) != enumConstants.end()) {
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

} // namespace semantic_analyzer

