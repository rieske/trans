#include "symbol_table.h"

#include <iostream>
#include <utility>

#include "ValueEntry.h"
#include "ValueScope.h"

using ast::BaseType;

const std::string LABEL_PREFIX = "__L";

namespace code_generator {

SymbolTable::SymbolTable() {
    valueScopes.push_back(std::make_unique<ValueScope>(currentScope));
    currentScope = valueScopes.back().get();
}

SymbolTable::~SymbolTable() {
}

int SymbolTable::insert(std::string name, ast::Type type, unsigned line) {
    return currentScope->insert(name, type, line);
}

void SymbolTable::insertFunctionArgument(std::string name, ast::Type type, unsigned line) {
    currentScope->insertFunctionArgument(name, type, line);
}

FunctionEntry SymbolTable::insertFunction(std::string name, ast::Function functionType, translation_unit::Context context) {
    FunctionEntry function = functions.insert(std::make_pair(name, FunctionEntry { name, functionType, context })).first->second;
    insert(function.getName(), ast::Type { function.getType().clone(), 1 }, function.getContext().getOffset());
    return function;
}

FunctionEntry SymbolTable::findFunction(std::string name) const {
    return functions.at(name);
}

bool SymbolTable::hasSymbol(std::string symbolName) const {
    return currentScope->hasSymbol(symbolName);
}

ValueEntry SymbolTable::lookup(std::string name) const {
    return currentScope->lookup(name);
}

ValueEntry SymbolTable::newTemp(ast::Type type) {
    return currentScope->newTemp(type);
}

LabelEntry SymbolTable::newLabel() {
    std::string labelName = generateLabelName();
    LabelEntry label { labelName };
    labels.insert(std::make_pair(labelName, label));
    return label;
}

std::string SymbolTable::generateLabelName() {
    return LABEL_PREFIX + std::to_string(++nextLabel);
}

void SymbolTable::startScope() {
    valueScopes.push_back(std::make_unique<ValueScope>(currentScope));
    currentScope = valueScopes.back().get();
}

void SymbolTable::endScope() {
    currentScope = currentScope->getParentScope();
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

unsigned SymbolTable::getTableSize() const {
    return currentScope->getTableSize();
}

}
