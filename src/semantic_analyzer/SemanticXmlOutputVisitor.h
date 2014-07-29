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
	void visit(const AssignmentExpressionList& expressions) const override;
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
	void visit(const LogicalAndExpression& expression) const override;
	void visit(const LogicalOrExpression& expression) const override;
	void visit(const AssignmentExpression& expression) const override;
	void visit(const ExpressionList& expression) const override;

	void visit(const JumpStatement& statement) const override;
	void visit(const ReturnStatement& statement) const override;
	void visit(const IOStatement& statement) const override;
	void visit(const IfStatement& statement) const override;
	void visit(const IfElseStatement& statement) const override;
	void visit(const LoopStatement& statement) const override;

	void visit(const ForLoopHeader& loopHeader) const override;
	void visit(const WhileLoopHeader& loopHeader) const override;

	void visit(const Pointer& pointer) const override;

	void visit(const Identifier& identifier) const override;
	void visit(const FunctionDeclaration& declaration) const override;
	void visit(const ArrayDeclaration& declaration) const override;

	void visit(const ParameterDeclaration& parameter) const override;

	void visit(const FunctionDefinition& function) const override;

	void visit(const VariableDeclaration& declaration) const override;
	void visit(const VariableDefinition& definition) const override;

	void visit(const Block& block) const override;
	void visit(const ListCarrier& listCarrier) const override;

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
