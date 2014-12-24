#include "symbol_table.h"
#include <iostream>
#include <stdexcept>
#include <utility>

using std::cerr;
using std::endl;
using std::cout;
using ast::BaseType;

const unsigned VARIABLE_SIZE = 4;

namespace code_generator {

const std::string SymbolTable::TEMP_PREFIX = "_t";
const std::string SymbolTable::LABEL_PREFIX = "__L";

SymbolTable::SymbolTable() {
    outer_scope = NULL;
    offset = 0;
    paramOffset = 8;
}

SymbolTable::SymbolTable(SymbolTable *outer) {
    outer_scope = outer;
    functions = outer->functions;
    nextLabel = outer->nextLabel;
    offset = 0;
    paramOffset = 8;
}

SymbolTable::~SymbolTable() {
    // FIXME: these need not belong here:
    /*for (auto scope : inner_scopes) {
     delete scope;
     }*/
}

int SymbolTable::insert(std::string name, ast::Type typeInfo, unsigned line) {
    try {
        return values.at(name).getLine();
    } catch (std::out_of_range &ex) {
        ValueEntry entry {name, typeInfo, false, line};
        entry.setOffset(offset);
        offset += VARIABLE_SIZE;
        values.insert(std::make_pair(name, entry));
    }
    return 0;
}

FunctionEntry SymbolTable::insertFunction(std::string name, ast::Function functionType, unsigned line) {
    return functions.insert(std::make_pair(name, FunctionEntry { name, functionType, line })).first->second;
}

FunctionEntry SymbolTable::findFunction(std::string name) const {
    return functions.at(name);
}

void SymbolTable::insertFunctionArgument(std::string name, ast::Type typeInfo, unsigned line) {
    try {
        values.at(name);
    } catch (std::out_of_range &ex) {
        ValueEntry entry { name, typeInfo, false, line };
        entry.setOffset(paramOffset);
        paramOffset += VARIABLE_SIZE;
        entry.setParam();
        values.insert(std::make_pair(name, entry));
    }
}

bool SymbolTable::hasSymbol(std::string symbolName) const {
    try {
        lookup(symbolName);
        return true;
    } catch (std::out_of_range &ex) {
        return false;
    }
}

ValueEntry SymbolTable::lookup(std::string name) const {
    try {
        return values.at(name);
    } catch (std::out_of_range &ex) {
        if (outer_scope) {
            return outer_scope->lookup(name);
        }
        throw;
    }
}

ValueEntry SymbolTable::newTemp(ast::Type type) {
    std::string tempName = generateTempName();
    ValueEntry temp { tempName, type, true, 0 };
    temp.setOffset(offset);
    offset += VARIABLE_SIZE;
    values.insert(std::make_pair(tempName, temp));
    return temp;
}

LabelEntry SymbolTable::newLabel() {
    std::string labelName = generateLabelName();
    LabelEntry label { labelName };
    labels.insert(std::make_pair(labelName, label));
    return label;
}

std::string SymbolTable::generateTempName() {
    return TEMP_PREFIX + std::to_string(++nextTemp);
}

std::string SymbolTable::generateLabelName() {
    return LABEL_PREFIX + std::to_string(++nextLabel);
}

SymbolTable *SymbolTable::newScope() {
    SymbolTable *inner = new SymbolTable(this);
    inner_scopes.push_back(inner);
    return inner;
}

SymbolTable *SymbolTable::getOuterScope() const {
    return outer_scope;
}

void SymbolTable::printTable() const {
    if (values.size() || inner_scopes.size()) {
        cout << "BEGIN SCOPE\t" << getTableSize() << endl;
        for (auto function : functions) {
            std::cout << "\t" << function.first << "\t\t\t\t" << function.second.getType().toString() << std::endl;
        }
        for (auto symbol : values) {
            symbol.second.print();
        }
        for (auto label : labels) {
            label.second.print();
        }
        for (unsigned i = 0; i < inner_scopes.size(); i++)
            inner_scopes[i]->printTable();
        cout << "END SCOPE" << endl;
    }
}

unsigned SymbolTable::getTableSize() const {
    return values.size() * VARIABLE_SIZE - (paramOffset - 8);
}

}
