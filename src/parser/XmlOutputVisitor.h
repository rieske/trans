#ifndef XMLOUTPUTVISITOR_H_
#define XMLOUTPUTVISITOR_H_

#include <iostream>

#include "../parser/ParseTreeNode.h"
#include "../parser/ParseTreeNodeVisitor.h"

namespace parser {

class TerminalNode;

class XmlOutputVisitor: public parser::ParseTreeNodeVisitor {
public:
	XmlOutputVisitor(std::ostream* ostream);
	virtual ~XmlOutputVisitor();

	void visit(const parser::ParseTreeNode& node) const override;
	void visit(const parser::TerminalNode& node) const override;

private:
	void ident() const;

	std::ostream* ostream;

	mutable int identation { 0 };
};

} /* namespace semantic_analyzer */

#endif /* XMLOUTPUTVISITOR_H_ */
