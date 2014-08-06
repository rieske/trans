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

    void visit(const TypeSpecifier& typeSpecifier) override;

	void visit(const ParameterList& parameterList) override;
	void visit(const AssignmentExpressionList& expressions) override;
	void visit(const DeclarationList& declarations) override;
	void visit(const ArrayAccess& arrayAccess) override;
	void visit(const FunctionCall& functionCall) override;
	void visit(const NoArgFunctionCall& functionCall) override;
	void visit(const Term& term) override;
	void visit(const PostfixExpression& expression) override;
	void visit(const PrefixExpression& expression) override;
	void visit(const UnaryExpression& expression) override;
	void visit(const TypeCast& expression) override;
	void visit(const PointerCast& expression) override;
	void visit(const ArithmeticExpression& expression) override;
	void visit(const ShiftExpression& expression) override;
	void visit(const ComparisonExpression& expression) override;
	void visit(const BitwiseExpression& expression) override;
	void visit(const LogicalAndExpression& expression) override;
	void visit(const LogicalOrExpression& expression) override;
	void visit(const AssignmentExpression& expression) override;
	void visit(const ExpressionList& expression) override;

	void visit(const JumpStatement& statement) override;
	void visit(const ReturnStatement& statement) override;
	void visit(const IOStatement& statement) override;
	void visit(const IfStatement& statement) override;
	void visit(const IfElseStatement& statement) override;
	void visit(const LoopStatement& statement) override;

	void visit(const ForLoopHeader& loopHeader) override;
	void visit(const WhileLoopHeader& loopHeader) override;

	void visit(const Pointer& pointer) override;

	void visit(const Identifier& identifier) override;
	void visit(const FunctionDeclaration& declaration) override;
	void visit(const ArrayDeclaration& declaration) override;

	void visit(const ParameterDeclaration& parameter) override;

	void visit(const FunctionDefinition& function) override;

	void visit(const VariableDeclaration& declaration) override;
	void visit(const VariableDefinition& definition) override;

	void visit(const Block& block) override;
	void visit(const ListCarrier& listCarrier) override;

	void visit(const TranslationUnit& translationUnit) override;

private:
	void openXmlNode(const std::string& nodeName);
	void closeXmlNode(const std::string& nodeName);

	void createLeafNode(std::string nodeName, std::string value) const;
	void createLeafNode(std::string nodeName, std::string typeAttribute, std::string value) const;

	std::string stripLabel(std::string label) const;
	void ident();

	std::ostream* outputStream;

	int identation { 0 };
};

} /* namespace semantic_analyzer */

#endif /* SEMANTICXMLOUTPUTVISITOR_H_ */
