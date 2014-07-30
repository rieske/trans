#ifndef _AST_NONTERMINAL_NODE_H_
#define _AST_NONTERMINAL_NODE_H_

#include <string>
#include <vector>

#include "../code_generator/quadruple.h"
#include "AbstractSyntaxTreeNode.h"

class SymbolTable;

namespace semantic_analyzer {

class NonterminalNode: public AbstractSyntaxTreeNode {
public:
	// FIXME: remove me
	NonterminalNode(std::string typeId);

	// FIXME: will be removed once the SemanticCheckVisitor is created
	bool getErrorFlag() const;

	std::vector<Quadruple *> getCode() const override;


	std::string typeId() const override;

	void accept(const AbstractSyntaxTreeVisitor& visitor) const override;

protected:
	NonterminalNode() {}
	NonterminalNode(std::string label, std::vector<AbstractSyntaxTreeNode *> children, SymbolTable *st, unsigned lineNumber);
	NonterminalNode(std::string l, std::vector<AbstractSyntaxTreeNode *> children);

	void semanticError(std::string description);

	SymbolTable *s_table;
	unsigned sourceLine;

	bool error { false };

	std::vector<Quadruple *> code;

private:
	std::string label;
};

}

#endif // _AST_NONTERMINAL_NODE_H_
