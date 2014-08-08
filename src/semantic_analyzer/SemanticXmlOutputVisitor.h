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

    void visit(TypeSpecifier& typeSpecifier) override;

	void visit(ParameterList& parameterList) override;
	void visit(AssignmentExpressionList& expressions) override;
	void visit(DeclarationList& declarations) override;
	void visit(ArrayAccess& arrayAccess) override;
	void visit(FunctionCall& functionCall) override;
	void visit(NoArgFunctionCall& functionCall) override;
	void visit(Term& term) override;
	void visit(PostfixExpression& expression) override;
	void visit(PrefixExpression& expression) override;
	void visit(UnaryExpression& expression) override;
	void visit(TypeCast& expression) override;
	void visit(PointerCast& expression) override;
	void visit(ArithmeticExpression& expression) override;
	void visit(ShiftExpression& expression) override;
	void visit(ComparisonExpression& expression) override;
	void visit(BitwiseExpression& expression) override;
	void visit(LogicalAndExpression& expression) override;
	void visit(LogicalOrExpression& expression) override;
	void visit(AssignmentExpression& expression) override;
	void visit(ExpressionList& expression) override;

	void visit(JumpStatement& statement) override;
	void visit(ReturnStatement& statement) override;
	void visit(IOStatement& statement) override;
	void visit(IfStatement& statement) override;
	void visit(IfElseStatement& statement) override;
	void visit(LoopStatement& statement) override;

	void visit(ForLoopHeader& loopHeader) override;
	void visit(WhileLoopHeader& loopHeader) override;

	void visit(Pointer& pointer) override;

	void visit(Identifier& identifier) override;
	void visit(FunctionDeclaration& declaration) override;
	void visit(ArrayDeclaration& declaration) override;

	void visit(ParameterDeclaration& parameter) override;

	void visit(FunctionDefinition& function) override;

	void visit(VariableDeclaration& declaration) override;
	void visit(VariableDefinition& definition) override;

	void visit(Block& block) override;
	void visit(ListCarrier& listCarrier) override;

	void visit(TranslationUnit& translationUnit) override;

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
