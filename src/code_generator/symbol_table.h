#ifndef _SYMBOL_TABLE_H_
#define _SYMBOL_TABLE_H_

#include <map>
#include <vector>
#include <string.h>
#include <stdlib.h>
#include "symbol_entry.h"

using std::map;
using std::vector;

const unsigned varSize = 4;

class SymbolTable {
public:
	SymbolTable();
	~SymbolTable();

	int insert(string name, string basic_type, string extended_type, unsigned line);
	int insertParam(string name, string basic_type, string extended_type, unsigned line);
	SymbolEntry *lookup(string name) const;
	SymbolEntry *newTemp(string basic_type, string extended_type);
	SymbolEntry *newLabel();
	SymbolTable *newScope();
	SymbolTable *getOuterScope() const;
	vector<SymbolTable *> getInnerScopes() const;

	string typeCheck(SymbolEntry *v1, SymbolEntry *v2);

	unsigned getTableSize() const;

	void printTable() const;

	SymbolTable *next();

private:
	SymbolTable(const SymbolTable *outer);

	void addOffset(unsigned extra);
	void removeOffset(unsigned extra);

	void generateTempName();
	void generateLabelName();

	string decorate(string type, string etype);

	map<string, SymbolEntry *> symbols;
	SymbolTable *outer_scope;
	vector<SymbolTable *> inner_scopes;
	vector<SymbolTable *>::iterator scopeIt;
	string nextTemp;
	string *nextLabel;
	unsigned offset;
	unsigned paramOffset;
	unsigned labels;
};

#endif // _SYMBOL_TABLE_H_
