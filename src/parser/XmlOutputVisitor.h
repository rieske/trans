#ifndef XMLOUTPUTVISITOR_H_
#define XMLOUTPUTVISITOR_H_

#include <ostream>
#include <string>

#include "ParseTreeNodeVisitor.h"

namespace parser {

class TerminalNode;

class XmlOutputVisitor: public ParseTreeNodeVisitor {
public:
	XmlOutputVisitor(std::ostream* ostream);
	virtual ~XmlOutputVisitor();

	void visit(const ParseTreeNode& node) const override;
	void visit(const TerminalNode& node) const override;

private:
	std::string stripLabel(std::string label) const;
	void ident() const;

	std::ostream* ostream;

	mutable int identation { 0 };
};

} // namespace parser

#endif // XMLOUTPUTVISITOR_H_
