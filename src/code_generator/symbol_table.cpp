#include "symbol_table.h"

#include <cstdlib>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <utility>

using std::cerr;
using std::endl;
using std::cout;
using ast::BasicType;

const unsigned VARIABLE_SIZE = 4;

SymbolTable::SymbolTable() {
    nextTemp = "_t0";
    nextLabel = new string("__L0");
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
    for (auto symbol : symbols) {
        delete symbol.second;
    }
    for (auto label : labels) {
        delete label.second;
    }
    for (auto scope : inner_scopes) {
        delete scope;
    }
}

int SymbolTable::insert(string name, ast::TypeInfo typeInfo, unsigned line) {
    SymbolEntry *entry;
    try {
        entry = symbols.at(name);
        return entry->getLine();
    } catch (std::out_of_range &ex) {
        entry = new SymbolEntry(name, typeInfo, false, line);
        entry->setOffset(offset);
        offset += VARIABLE_SIZE;
        symbols[name] = entry;
    }
    return 0;
}

void SymbolTable::insertParam(string name, ast::TypeInfo typeInfo, unsigned line) {
    SymbolEntry *entry;
    try {
        entry = symbols.at(name);
    } catch (std::out_of_range &ex) {
        entry = new SymbolEntry(name, typeInfo, false, line);
        entry->setOffset(paramOffset);
        paramOffset += VARIABLE_SIZE;
        entry->setParam();
        symbols[name] = entry;
    }
}

bool SymbolTable::hasSymbol(std::string symbolName) const {
    return this->lookup(symbolName);
}

SymbolEntry *SymbolTable::lookup(string name) const {
    SymbolEntry *entry = NULL;
    try {
        entry = symbols.at(name);
    } catch (std::out_of_range &ex) {
        if (outer_scope != NULL)
            entry = outer_scope->lookup(name);
    }
    return entry;
}

SymbolEntry *SymbolTable::newTemp(ast::TypeInfo typeInfo) {
    SymbolEntry *temp;
    generateTempName();
    temp = new SymbolEntry(nextTemp, typeInfo, true, 0);
    temp->setOffset(offset);
    offset += VARIABLE_SIZE;
    symbols[nextTemp] = temp;
    return temp;
}

SymbolEntry *SymbolTable::newLabel() {
    SymbolEntry *label;
    generateLabelName();
    label = new SymbolEntry(*nextLabel, { BasicType::LABEL }, true, 0);
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
    scopeIt = inner_scopes.begin();
    return inner;
}

SymbolTable *SymbolTable::nextScope() {
    SymbolTable *retVal = NULL;
    if (scopeIt != inner_scopes.end()) {
        retVal = *scopeIt;
        scopeIt++;
        addOffset(retVal->getTableSize());
    } else {
        retVal = outer_scope;
        outer_scope->removeOffset(getTableSize());
    }

    return retVal;
}

SymbolTable *SymbolTable::getOuterScope() const {
    return outer_scope;
}

string SymbolTable::typeCheck(SymbolEntry *v1, SymbolEntry *v2) {
    BasicType type1 = v1->getTypeInfo().getBasicType();
    BasicType type2 = v2->getTypeInfo().getBasicType();
    auto etype1 = v1->getTypeInfo().getDereferenceCount();
    auto etype2 = v2->getTypeInfo().getDereferenceCount();

    if ((etype1 == etype2) && (type1 == type2))
        return "ok";
    return "type mismatch: can't convert " + decorate(type1, etype1) + " to " + decorate(type2, etype2) + "\n";
}

string SymbolTable::decorate(BasicType type, int etype) {
    string typeStr { };
    switch (type) {
    case BasicType::INTEGER:
        typeStr = "integer";
        break;
    case BasicType::CHARACTER:
        typeStr = "character";
        break;
    case BasicType::FLOAT:
        typeStr = "float";
        break;
    case BasicType::VOID:
        typeStr = "void";
        break;
    case BasicType::FUNCTION:
        typeStr = "()";
        break;
    }

    while (etype--) {
        typeStr += "*";
    }
    return typeStr;
}

void SymbolTable::printTable() const {
    if (symbols.size() || inner_scopes.size()) {
        cout << "BEGIN SCOPE\t" << getTableSize() << endl;
        for (auto symbol : symbols) {
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

vector<SymbolTable *> SymbolTable::getInnerScopes() const {
    return inner_scopes;
}

unsigned SymbolTable::getTableSize() const {
    unsigned paramCount = (paramOffset - 8) / 4;
    return symbols.size() * VARIABLE_SIZE - paramCount * 4;
}

void SymbolTable::addOffset(unsigned extra) {
    for (auto symbol : symbols) {
        SymbolEntry *entry = symbol.second;
        if (!entry->isParam()) {
            entry->setOffset(entry->getOffset() + extra);
        }
    }
    if (outer_scope) {
        outer_scope->addOffset(extra);
    }
}

void SymbolTable::removeOffset(unsigned extra) {
    for (auto symbol : symbols) {
        SymbolEntry *entry = symbol.second;
        if (!entry->isParam()) {
            entry->setOffset(entry->getOffset() - extra);
        }
    }
    if (outer_scope) {
        outer_scope->removeOffset(extra);
    }
}

