#include "symbol_table.h"

#include <cstdlib>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <utility>

#include "LabelEntry.h"
#include "ValueEntry.h"

using std::cerr;
using std::endl;
using std::cout;
using ast::BaseType;

const unsigned VARIABLE_SIZE = 4;

namespace code_generator {

SymbolTable::SymbolTable() {
    nextTemp = "_t0";
    nextLabel = new std::string("__L0");
    outer_scope = NULL;
    offset = 0;
    paramOffset = 8;
}

SymbolTable::SymbolTable(const SymbolTable *outer) {
    outer_scope = (SymbolTable *) outer;
    nextTemp = "_t0";
    nextLabel = outer->nextLabel;
    offset = 0;
    paramOffset = 8;
}

SymbolTable::~SymbolTable() {
    // FIXME: these need not belong here:
   /* for (auto symbol : symbols) {
        delete symbol.second;
    }
    for (auto label : labels) {
        delete label.second;
    }
    for (auto scope : inner_scopes) {
        delete scope;
    }*/
}

int SymbolTable::insert(std::string name, ast::Type typeInfo, unsigned line) {
    ValueEntry *entry;
    try {
        entry = values.at(name);
        return entry->getLine();
    } catch (std::out_of_range &ex) {
        entry = new ValueEntry(name, typeInfo, false, line);
        entry->setOffset(offset);
        offset += VARIABLE_SIZE;
        values[name] = entry;
    }
    return 0;
}

void SymbolTable::insertFunctionArgument(std::string name, ast::Type typeInfo, unsigned line) {
    ValueEntry *entry;
    try {
        entry = values.at(name);
    } catch (std::out_of_range &ex) {
        entry = new ValueEntry(name, typeInfo, false, line);
        entry->setOffset(paramOffset);
        paramOffset += VARIABLE_SIZE;
        entry->setParam();
        values[name] = entry;
    }
}

bool SymbolTable::hasSymbol(std::string symbolName) const {
    return this->lookup(symbolName);
}

ValueEntry* SymbolTable::lookup(std::string name) const {
    ValueEntry *entry = NULL;
    try {
        entry = values.at(name);
    } catch (std::out_of_range &ex) {
        if (outer_scope != NULL)
            entry = outer_scope->lookup(name);
    }
    return entry;
}

ValueEntry *SymbolTable::newTemp(ast::Type typeInfo) {
    ValueEntry *temp;
    generateTempName();
    temp = new ValueEntry(nextTemp, typeInfo, true, 0);
    temp->setOffset(offset);
    offset += VARIABLE_SIZE;
    values[nextTemp] = temp;
    return temp;
}

LabelEntry *SymbolTable::newLabel() {
    LabelEntry *label;
    generateLabelName();
    label = new LabelEntry(*nextLabel);
    labels[*nextLabel] = label;
    return label;
}

void SymbolTable::generateTempName() {
    unsigned intVal;
    nextTemp = nextTemp.substr(2, nextTemp.size());
    intVal = atoi(nextTemp.c_str());
    intVal++;
    nextTemp = "_t";
    nextTemp += std::to_string(intVal);
}

void SymbolTable::generateLabelName() {
    unsigned intVal;
    *nextLabel = nextLabel->substr(3, nextLabel->size());
    intVal = atoi(nextLabel->c_str());
    intVal++;
    *nextLabel = "__L";
    *nextLabel += std::to_string(intVal);
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
        for (auto symbol : values) {
            symbol.second->print();
        }
        for (auto label : labels) {
            label.second->print();
        }
        for (unsigned i = 0; i < inner_scopes.size(); i++)
            inner_scopes[i]->printTable();
        cout << "END SCOPE" << endl;
    }
}

unsigned SymbolTable::getTableSize() const {
    unsigned paramCount = (paramOffset - 8) / 4;
    return values.size() * VARIABLE_SIZE - paramCount * 4;
}

}
