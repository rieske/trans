#ifndef _NONTERMINAL_NODE_H_
#define _NONTERMINAL_NODE_H_

#include <iostream>
#include <string>
#include <vector>

#include "ParseTreeNode.h"

class SymbolTable;

namespace parser {

class NonterminalNode: public ParseTreeNode {
public:
	NonterminalNode(string label, vector<ParseTreeNode *> children, SymbolTable *st, unsigned lineNumber);
	NonterminalNode(string l, vector<ParseTreeNode *> children);

	string getAttr() const override;
	virtual std::ostringstream &asXml(std::ostringstream &oss, unsigned depth) const;
protected:

	void semanticError(std::string description);

	SymbolTable *s_table;
	unsigned sourceLine;

	string attr;
};

}

#endif // _NONTERMINAL_NODE_H_
