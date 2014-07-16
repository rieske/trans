#ifndef PARSETREENODEVISITOR_H_
#define PARSETREENODEVISITOR_H_

#include "ParseTreeNode.h"

namespace parser {

class ParseTreeNodeVisitor {
public:
	ParseTreeNodeVisitor();
	virtual ~ParseTreeNodeVisitor();

	void visit(ParseTreeNode& node);
};

} /* namespace parser */

#endif /* PARSETREENODEVISITOR_H_ */
