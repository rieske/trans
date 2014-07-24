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
	void visit(const DeclarationList& declarations) const override;
	void visit(const ArrayAccess& arrayAccess) const override;
	void visit(const FunctionCall& functionCall) const override;
	void visit(const NoArgFunctionCall& functionCall) const override;
	void visit(const PostfixExpression& expression) const override;
	void visit(const PrefixExpression& expression) const override;
	void visit(const UnaryExpression& expression) const override;
	void visit(const TypeCast& expression) const override;
	void visit(const PointerCast& expression) const override;
	void visit(const ArithmeticExpression& expression) const override;
	void visit(const ShiftExpression& expression) const override;
	void visit(const ComparisonExpression& expression) const override;
	void visit(const BitwiseExpression& expression) const override;

private:
	void openXmlNode(const std::string& nodeName) const;
	void closeXmlNode(const std::string& nodeName) const;

	void createLeafNode(std::string nodeName, std::string value) const;

	std::string stripLabel(std::string label) const;
	void ident() const;

	std::ostream* outputStream;

	mutable int identation { 0 };
};

} /* namespace semantic_analyzer */

#endif /* SEMANTICXMLOUTPUTVISITOR_H_ */
