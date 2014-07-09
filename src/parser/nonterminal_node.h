#ifndef _NONTERMINAL_NODE_H_
#define _NONTERMINAL_NODE_H_

#include <iostream>
#include <string>
#include <vector>

#include "GrammarSymbol.h"
#include "ParseTreeNode.h"

class SymbolTable;

/**
 * @author Vaidotas Valuckas
 * Neterminalinio gramatikos simbolio mazgo sintaksiniame medyje klasė
 **/

class NonterminalNode: public ParseTreeNode {
public:
	NonterminalNode(string label, vector<ParseTreeNode *> &children, Production production, SymbolTable *st, unsigned lineNumber);
	NonterminalNode(string l, vector<ParseTreeNode *> &children, Production production);

	string getAttr() const override;
	virtual ostringstream &asXml(ostringstream &oss, unsigned depth) const;
protected:

	void semanticError(std::string description);

	std::string productionStr(const Production& production) const;

	string reduction;
	SymbolTable *s_table;
	unsigned sourceLine;

	string attr;
};

#endif // _NONTERMINAL_NODE_H_
