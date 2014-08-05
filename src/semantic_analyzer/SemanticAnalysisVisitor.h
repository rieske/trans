#ifndef SEMANTICANALYSISVISITOR_H_
#define SEMANTICANALYSISVISITOR_H_

#include "AbstractSyntaxTreeVisitor.h"

namespace semantic_analyzer {

class SemanticAnalysisVisitor: public AbstractSyntaxTreeVisitor {
public:
	SemanticAnalysisVisitor();
	virtual ~SemanticAnalysisVisitor();

	void visit(const ParameterList& parameterList) const override;
	void visit(const AssignmentExpressionList& expressions) const override;
	void visit(const DeclarationList& declarations) const override;
	void visit(const ArrayAccess& arrayAccess) const override;
	void visit(const FunctionCall& functionCall) const override;
	void visit(const NoArgFunctionCall& functionCall) const override;
	void visit(const Term& term) const override;
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

	void visit(const TranslationUnit& translationUnit) const override;
};

} /* namespace semantic_analyzer */

#endif /* SEMANTICANALYSISVISITOR_H_ */
