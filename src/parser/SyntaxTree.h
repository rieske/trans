#ifndef _SYNTAX_TREE_H_
#define _SYNTAX_TREE_H_

#include <iostream>
#include <string>
#include <vector>

#include "../code_generator/quadruple.h"

class ParseTreeNode;
class SymbolTable;

/**
 * @author Vaidotas Valuckas
 * Sintaksinio medžio klasė
 **/

class SyntaxTree {
public:
	SyntaxTree(ParseTreeNode *top, SymbolTable* symbolTable);
	~SyntaxTree();

	static const char *getFileName();

	void setFileName(const char *);

	bool hasSemanticErrors() const;

	void outputCode(ostream &of) const;

	SymbolTable *getSymbolTable() const;
	vector<Quadruple *> getCode() const;

	string asXml() const;

	void printTables() const;

	void logXml();
	void logTables();
	void logCode();

private:
	ParseTreeNode *tree;

	SymbolTable *s_table;
	vector<Quadruple *> code;

	static const char *filename;
};

#endif // _SYNTAX_TREE_H_
