#ifndef XMLOUTPUTVISITOR_H_
#define XMLOUTPUTVISITOR_H_

#include <iostream>

#include "AbstractSyntaxTreeVisitor.h"

namespace semantic_analyzer {

class XmlOutputVisitor: public AbstractSyntaxTreeVisitor {
public:
	XmlOutputVisitor(std::ostream* ostream);
	virtual ~XmlOutputVisitor();

	void visit(const AbstractSyntaxTreeNode& node) const override;

private:
	void ident() const;

	std::ostream* ostream;

	mutable int identation { 0 };
};

} /* namespace semantic_analyzer */

#endif /* XMLOUTPUTVISITOR_H_ */
