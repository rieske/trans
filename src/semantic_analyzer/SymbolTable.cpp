#include "SymbolTable.h"
#include "util/Logger.h"
#include "util/LogManager.h"

namespace {

static Logger& out = LogManager::getOutputLogger();

const std::string LABEL_PREFIX = "__L";
unsigned nextLabel { 0 };

const std::string CONSTANT_PREFIX = "$c";
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

bool SymbolTable::insertSymbol(std::string name, const type::Type& type, translation_unit::Context context) {
    if (isAtFileScope()) {
        return globalScope.insertSymbol(name, type, context, true);
    }
    return functionScopes.back().insertSymbol(scopePrefix(currentScopeId()) + name, type, context);
}

bool SymbolTable::insertStaticLocal(std::string name, const type::Type& type, translation_unit::Context context) {
    // Mangled label is unique per block scope; same bare name in different
    // functions (or nested blocks) must not collide in .data.
    const std::string mangled = scopePrefix(currentScopeId()) + name;
    if (!globalScope.insertSymbol(mangled, type, context, true)) {
        return false;
    }
    globalScope.setStaticStorage(mangled, true);
    // Function-scope entry with isGlobal so codegen homes it in .data, not the frame.
    if (!functionScopes.back().insertSymbol(mangled, type, context, true)) {
        return false;
    }
    functionScopes.back().setStaticStorage(mangled, true);
    return true;
}

bool SymbolTable::insertExternLocal(std::string name, const type::Type& type, translation_unit::Context context) {
    // Do not create a stack local: that would shadow the real object and leave
    // an uninitialized pointer (git run-command.c: extern char **environ in prep_childenv).
    if (hasFunction(name)) {
        return false;
    }
    if (globalScope.isSymbolDefined(name)) {
        if (globalScope.lookup(name).getType().isFunction()) {
            return false;
        }
        // Already declared/defined in this TU; block-scope extern refers to it.
        return true;
    }
    if (!globalScope.insertSymbol(name, type, context, true)) {
        return false;
    }
    globalScope.setExternal(name, true);
    return true;
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

FunctionEntry SymbolTable::insertFunction(std::string name, type::Function functionType, translation_unit::Context context) {
    FunctionEntry function = functions.insert(std::make_pair(name, FunctionEntry { name, functionType, context })).first->second;
    globalScope.insertSymbol(function.getName(), type::function(functionType.getReturnType(), functionType.getArguments()), function.getContext());
    return function;
}

FunctionEntry SymbolTable::findFunction(std::string name) const {
    return functions.at(name);
}

bool SymbolTable::hasFunction(const std::string& name) const {
    return functions.find(name) != functions.end();
}

bool SymbolTable::isAtFileScope() const {
    return scopeIdStack.empty();
}

bool SymbolTable::hasGlobalVariable(const std::string& name) const {
    if (!globalScope.isSymbolDefined(name)) {
        return false;
    }
    return !globalScope.lookup(name).getType().isFunction();
}

void SymbolTable::setGlobalInitializer(const std::string& name, long constantValue) {
    globalScope.setConstantInitializer(name, constantValue);
}

void SymbolTable::setGlobalStringInitializer(const std::string& name, std::string value) {
    globalScope.setStringInitializer(name, std::move(value));
}

void SymbolTable::setGlobalAddressInitializer(const std::string& name, std::string symbolName) {
    globalScope.setAddressInitializer(name, std::move(symbolName));
}

void SymbolTable::setGlobalMultiWordInitializer(const std::string& name, std::vector<std::string> words) {
    globalScope.setMultiWordInitializer(name, std::move(words));
}

void SymbolTable::setGlobalType(const std::string& name, const type::Type& type) {
    globalScope.setSymbolType(name, type);
}

void SymbolTable::setLocalType(const std::string& name, const type::Type& type) {
    if (functionScopes.empty()) {
        return;
    }
    for (auto it = scopeIdStack.rbegin(); it != scopeIdStack.rend(); ++it) {
        const std::string scoped = scopePrefix(*it) + name;
        if (functionScopes.back().isSymbolDefined(scoped)) {
            functionScopes.back().setSymbolType(scoped, type);
            return;
        }
    }
}

void SymbolTable::setGlobalExternal(const std::string& name, bool value) {
    globalScope.setExternal(name, value);
}

void SymbolTable::setGlobalStaticStorage(const std::string& name, bool value) {
    globalScope.setStaticStorage(name, value);
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
            if (functionScopes.back().isSymbolDefined(scopePrefix(*it) + symbolName)) {
                return true;
            }
        }
    }
    return globalScope.isSymbolDefined(symbolName);
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
    return functionScopes.back().getSymbols();
}

std::vector<ValueEntry> SymbolTable::getCurrentScopeArguments() const {
    return functionScopes.back().getArguments();
}

std::map<std::string, std::string> SymbolTable::getConstants() const {
    return constants;
}

std::vector<ValueEntry> SymbolTable::getGlobalVariables() const {
    std::vector<ValueEntry> globals;
    for (const auto& entry : globalScope.getSymbols()) {
        if (entry.second.isGlobal()) {
            globals.push_back(entry.second);
        }
    }
    return globals;
}

void SymbolTable::printTable() const {
    for (auto constant : constants) {
        out << "\t" << constant.first << "\t\t\t\t" << constant.second << "\n";
    }
    for (auto function : functions) {
        out << "\t" << function.first << "\t\t\t\t" << function.second.getType().to_string() << "\n";
    }
    for (auto label : labels) {
        out << "\t" << label.second.getName() << "\t\ttemp\t0\t\tlabel\n";
    }
    for (unsigned i = 0; i < functionScopes.size(); i++) {
        out << "BEGIN SCOPE\n"
            << "--arguments--\n";
        for (const auto& value : functionScopes[i].getArguments()) {
            out << value.to_string();
        }
        out << "--locals--\n";
        for (const auto& value : functionScopes[i].getSymbols()) {
            out << value.second.to_string();
        }
        out << "END SCOPE\n";
    }
}

std::string SymbolTable::scopePrefix(unsigned scopeId) const {
    return SCOPE_PREFIX + std::to_string(scopeId);
}

} // namespace semantic_analyzer

