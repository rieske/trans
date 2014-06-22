#ifndef _NONTERMINAL_NODE_H_
#define _NONTERMINAL_NODE_H_

#include <iostream>
#include <string>
#include <vector>

#include "GrammarSymbol.h"
#include "node.h"

class SymbolTable;

/**
 * @author Vaidotas Valuckas
 * Neterminalinio gramatikos simbolio mazgo sintaksiniame medyje klasė
 **/

class NonterminalNode: public Node {
public:
	NonterminalNode(string label, vector<Node *> &children, Production production, SymbolTable *st, unsigned lineNumber);
	NonterminalNode(string l, vector<Node *> &children, Production production);

	virtual string getAttr() const;
	virtual ostringstream &asXml(ostringstream &oss, unsigned depth) const;

	SymbolTable *getScope() const;

protected:

	void semanticError(std::string description);

	std::string productionStr(const Production& production) const;

	string reduction;
	SymbolTable *s_table;
	unsigned sourceLine;

private:
};

#endif // _NONTERMINAL_NODE_H_
