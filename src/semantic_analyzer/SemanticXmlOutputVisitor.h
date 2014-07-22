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

private:
	void openXmlNode(const NonterminalNode& node) const;
	void closeXmlNode(const NonterminalNode& node) const;

	std::string stripLabel(std::string label) const;
	void ident() const;

	std::ostream* outputStream;

	mutable int identation { 0 };
};

} /* namespace semantic_analyzer */

#endif /* SEMANTICXMLOUTPUTVISITOR_H_ */
