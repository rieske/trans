#include "SymbolTable.h"
#include "util/Logger.h"
#include "util/LogManager.h"

namespace {

const std::string LABEL_PREFIX = "__L";
unsigned nextLabel { 0 };

std::string generateLabelName() {
    return LABEL_PREFIX + std::to_string(++nextLabel);
}

static Logger& out = LogManager::getOutputLogger();

} // namespace

namespace semantic_analyzer {

const std::string SymbolTable::SCOPE_PREFIX = "$s";

bool SymbolTable::insertSymbol(std::string name, const ast::FundamentalType& type, translation_unit::Context context) {
    return functionScopes.back().insertSymbol(scopePrefix(currentScopeIndex) + name, type, context);
}

void SymbolTable::insertFunctionArgument(std::string name, ast::FundamentalType& type, translation_unit::Context context) {
    functionScopes.back().insertFunctionArgument(scopePrefix(currentScopeIndex + 1) + name, type, context);
}

FunctionEntry SymbolTable::insertFunction(std::string name, ast::FunctionType functionType, translation_unit::Context context) {
    FunctionEntry function = functions.insert(std::make_pair(name, FunctionEntry { name, functionType, context })).first->second;
    globalScope.insertSymbol(function.getName(), functionType, function.getContext());
    return function;
}

FunctionEntry SymbolTable::findFunction(std::string name) const {
    return functions.at(name);
}

bool SymbolTable::hasSymbol(std::string symbolName) const {
    return functionScopes.back().isSymbolDefined(scopePrefix(currentScopeIndex) + symbolName) || globalScope.isSymbolDefined(symbolName);
}

ValueEntry SymbolTable::lookup(std::string name) const {
    try {
        return functionScopes.back().lookup(scopePrefix(currentScopeIndex) + name);
    } catch (std::out_of_range&) {
        return globalScope.lookup(name);
    }
}

ValueEntry SymbolTable::createTemporarySymbol(std::unique_ptr<ast::FundamentalType> type) {
    return functionScopes.back().createTemporarySymbol(std::move(type));
}

LabelEntry SymbolTable::newLabel() {
    std::string labelName = generateLabelName();
    LabelEntry label { labelName };
    labels.insert(std::make_pair(labelName, label));
    return label;
}

void SymbolTable::startFunction(std::string name, std::vector<std::string> formalArguments) {
    functionScopes.push_back(ValueScope { });
    auto function = findFunction(name);
    size_t i { 0 };
    for (auto& argument : function.arguments()) {
        if (i < formalArguments.size()) {
            insertFunctionArgument(formalArguments.at(i), *argument, function.getContext());
        }
        ++i;
    }
    ++currentScopeIndex;
}

void SymbolTable::endFunction() {
    --currentScopeIndex;
}

std::map<std::string, ValueEntry> SymbolTable::getCurrentScopeSymbols() const {
    return functionScopes.back().getSymbols();
}

std::vector<ValueEntry> SymbolTable::getCurrentScopeArguments() const {
    return functionScopes.back().getArguments();
}

void SymbolTable::printTable() const {
    for (auto function : functions) {
        out << "\t" << function.first << "\t\t\t\t" << function.second.getType().toString() << "\n";
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

std::string SymbolTable::scopePrefix(unsigned scopeIndex) const {
    return SCOPE_PREFIX + std::to_string(scopeIndex);
}

} // namespace semantic_analyzer

