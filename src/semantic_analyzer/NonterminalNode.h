#ifndef _AST_NONTERMINAL_NODE_H_
#define _AST_NONTERMINAL_NODE_H_

#include <iostream>
#include <string>
#include <vector>

#include "../code_generator/quadruple.h"
#include "AbstractSyntaxTreeNode.h"

class SymbolTable;

namespace semantic_analyzer {

class NonterminalNode: public AbstractSyntaxTreeNode {
public:
	NonterminalNode(std::string label, std::vector<AbstractSyntaxTreeNode *> children, SymbolTable *st, unsigned lineNumber);
	NonterminalNode(std::string l, std::vector<AbstractSyntaxTreeNode *> children);

	bool getErrorFlag() const;

	std::string getValue() const;

	std::vector<Quadruple *> getCode() const;
protected:

	void semanticError(std::string description);

	SymbolTable *s_table;
	unsigned sourceLine;

	std::string attr;

	bool error { false };


protected:
	std::string label;
	std::vector<AbstractSyntaxTreeNode*> subtrees;

	std::vector<Quadruple *> code;

private:
	void assign_label(std::string &l);
	void assign_children(std::vector<AbstractSyntaxTreeNode *> children);
};

}

#endif // _AST_NONTERMINAL_NODE_H_
