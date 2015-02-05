#include "symbol_table.h"

#include <iostream>
#include <utility>

#include "ValueEntry.h"
#include "ValueScope.h"

using ast::BaseType;

namespace {

const std::string LABEL_PREFIX = "__L";
unsigned nextLabel { 0 };

std::string generateLabelName() {
    return LABEL_PREFIX + std::to_string(++nextLabel);
}

}

namespace code_generator {

const std::string SymbolTable::SCOPE_PREFIX = "$s";

bool SymbolTable::insertSymbol(std::string name, ast::Type type, translation_unit::Context context) {
    return functionScopes.back().insertSymbol(scopePrefix(currentScopeIndex) + name, type, context);
}

void SymbolTable::insertFunctionArgument(std::string name, ast::Type type, translation_unit::Context context) {
    functionScopes.back().insertFunctionArgument(scopePrefix(currentScopeIndex + 1) + name, type, context);
}

FunctionEntry SymbolTable::insertFunction(std::string name, ast::Function functionType, translation_unit::Context context) {
    FunctionEntry function = functions.insert(std::make_pair(name, FunctionEntry { name, functionType, context })).first->second;
    globalScope.insertSymbol(function.getName(), ast::Type { function.getType().clone(), 1 }, function.getContext());
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

ValueEntry SymbolTable::createTemporarySymbol(ast::Type type) {
    return functionScopes.back().createTemporarySymbol(type);
}

LabelEntry SymbolTable::newLabel() {
    std::string labelName = generateLabelName();
    LabelEntry label { labelName };
    labels.insert(std::make_pair(labelName, label));
    return label;
}

void SymbolTable::startFunction(std::string name) {
    functionScopes.push_back(ValueScope { });
    auto function = findFunction(name);
    for (auto& argument : function.arguments()) {
        insertFunctionArgument(argument.first, argument.second, function.getContext());
    }
}

void SymbolTable::endFunction() {
}

void SymbolTable::startScope() {
    ++currentScopeIndex;
}

void SymbolTable::endScope() {
    --currentScopeIndex;
}

std::map<std::string, ValueEntry> SymbolTable::getCurrentScopeSymbols() const {
    return functionScopes.back().getSymbols();
}

std::map<std::string, ValueEntry> SymbolTable::getCurrentScopeArguments() const {
    return functionScopes.back().getArguments();
}

void SymbolTable::printTable() const {
    for (auto function : functions) {
        std::cout << "\t" << function.first << "\t\t\t\t" << function.second.getType().toString() << std::endl;
    }
    for (auto label : labels) {
        label.second.print();
    }
    for (unsigned i = 0; i < functionScopes.size(); i++) {
        functionScopes[i].print();
    }
}

std::string SymbolTable::scopePrefix(unsigned scopeIndex) const {
    return SCOPE_PREFIX + std::to_string(scopeIndex);
}

}
