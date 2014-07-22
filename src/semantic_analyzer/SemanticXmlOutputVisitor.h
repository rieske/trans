#ifndef SEMANTICXMLOUTPUTVISITOR_H_
#define SEMANTICXMLOUTPUTVISITOR_H_

#include <iostream>
#include <string>

#include "AbstractSyntaxTreeVisitor.h"

namespace semantic_analyzer {

class SemanticXmlOutputVisitor: public AbstractSyntaxTreeVisitor {
public:
	SemanticXmlOutputVisitor(std::ostream* outputStream);
	virtual ~SemanticXmlOutputVisitor();

	void visit(const AbstractSyntaxTreeNode& node) const override;
	void visit(const NonterminalNode& node) const override;

	void visit(const ParameterList& parameterList) const override;
	void visit(const AssignmentExpressionList& assignmentExpressions) const override;

private:
	void openXmlNode(const std::string& nodeName) const;
	void closeXmlNode(const std::string& nodeName) const;

	std::string stripLabel(std::string label) const;
	void ident() const;

	std::ostream* outputStream;

	mutable int identation { 0 };
};

} /* namespace semantic_analyzer */

#endif /* SEMANTICXMLOUTPUTVISITOR_H_ */
