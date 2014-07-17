#ifndef PARSETREENODEVISITOR_H_
#define PARSETREENODEVISITOR_H_

#include "ParseTreeNode.h"

namespace parser {

class TerminalNode;

class ParseTreeNodeVisitor {
public:
	ParseTreeNodeVisitor();
	virtual ~ParseTreeNodeVisitor();

	virtual void visit(const ParseTreeNode& node) const = 0;
	virtual void visit(const TerminalNode& node) const = 0;
};

} /* namespace parser */

#endif /* PARSETREENODEVISITOR_H_ */
