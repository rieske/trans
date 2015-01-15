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

SymbolTable::SymbolTable() {
    valueScopes.push_back(std::make_unique<ValueScope>(currentScope));
    currentScope = valueScopes.back().get();
}

SymbolTable::~SymbolTable() {
}

bool SymbolTable::insertSymbol(std::string name, ast::Type type, translation_unit::Context context) {
    return currentScope->insertSymbol(name, type, context);
}

void SymbolTable::insertFunctionArgument(std::string name, ast::Type type, translation_unit::Context context) {
    currentScope->insertFunctionArgument(name, type, context);
}

FunctionEntry SymbolTable::insertFunction(std::string name, ast::Function functionType, translation_unit::Context context) {
    FunctionEntry function = functions.insert(std::make_pair(name, FunctionEntry { name, functionType, context })).first->second;
    insertSymbol(function.getName(), ast::Type { function.getType().clone(), 1 }, function.getContext());
    return function;
}

FunctionEntry SymbolTable::findFunction(std::string name) const {
    return functions.at(name);
}

bool SymbolTable::hasSymbol(std::string symbolName) const {
    return currentScope->isSymbolDefined(symbolName);
}

ValueEntry SymbolTable::lookup(std::string name) const {
    return currentScope->lookup(name);
}

ValueEntry SymbolTable::createTemporarySymbol(ast::Type type) {
    return currentScope->createTemporarySymbol(type);
}

LabelEntry SymbolTable::newLabel() {
    std::string labelName = generateLabelName();
    LabelEntry label { labelName };
    labels.insert(std::make_pair(labelName, label));
    return label;
}

void SymbolTable::startScope() {
    valueScopes.push_back(std::make_unique<ValueScope>(currentScope));
    currentScope = valueScopes.back().get();
}

void SymbolTable::endScope() {
    currentScope = currentScope->getParentScope();
}

std::map<std::string, ValueEntry> SymbolTable::getCurrentScopeSymbols() const {
    return currentScope->getSymbols();
}

std::map<std::string, ValueEntry> SymbolTable::getCurrentScopeArguments() const {
    return currentScope->getArguments();
}

void SymbolTable::printTable() const {
    for (auto function : functions) {
        std::cout << "\t" << function.first << "\t\t\t\t" << function.second.getType().toString() << std::endl;
    }
    for (auto label : labels) {
        label.second.print();
    }
    for (unsigned i = 0; i < valueScopes.size(); i++) {
        valueScopes[i]->print();
    }
}

}
