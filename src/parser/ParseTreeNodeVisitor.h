#ifndef PARSETREENODEVISITOR_H_
#define PARSETREENODEVISITOR_H_

#include "ParseTreeNode.h"
#include "parser/TerminalNode.h"

namespace parser {

class ParseTreeNodeVisitor {
public:
	virtual ~ParseTreeNodeVisitor() {}

	virtual void visit(const ParseTreeNode& node) const = 0;
	virtual void visit(const TerminalNode& node) const = 0;
};

} // namespace parser

#endif // PARSETREENODEVISITOR_H_
