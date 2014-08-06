#ifndef ABSTRACTSYNTAXTREEVISITOR_H_
#define ABSTRACTSYNTAXTREEVISITOR_H_

namespace semantic_analyzer {

class AbstractSyntaxTreeNode;
class TypeSpecifier;
class ParameterList;
class AssignmentExpressionList;
class DeclarationList;
class ArrayAccess;
class FunctionCall;
class NoArgFunctionCall;
class Term;
class PostfixExpression;
class PrefixExpression;
class UnaryExpression;
class TypeCast;
class PointerCast;
class ArithmeticExpression;
class ShiftExpression;
class ComparisonExpression;
class BitwiseExpression;
class LogicalAndExpression;
class LogicalOrExpression;
class AssignmentExpression;
class ExpressionList;
class JumpStatement;
class ReturnStatement;
class IOStatement;
class WhileLoopHeader;
class ForLoopHeader;
class IfStatement;
class IfElseStatement;
class LoopStatement;
class Pointer;
class Identifier;
class FunctionDeclaration;
class ParameterDeclaration;
class ArrayDeclaration;
class FunctionDefinition;
class VariableDeclaration;
class VariableDefinition;
class Block;
class ListCarrier;
class TranslationUnit;

class AbstractSyntaxTreeVisitor {
public:
	virtual ~AbstractSyntaxTreeVisitor() {
	}

	virtual void visit(const TypeSpecifier& typeSpecifier) = 0;

	virtual void visit(const ParameterList& parameterList) = 0;
	virtual void visit(const AssignmentExpressionList& expressions) = 0;
	virtual void visit(const DeclarationList& declarations) = 0;
	virtual void visit(const ArrayAccess& arrayAccess) = 0;
	virtual void visit(const FunctionCall& functionCall) = 0;
	virtual void visit(const NoArgFunctionCall& functionCall) = 0;
	virtual void visit(const Term& term) = 0;
	virtual void visit(const PostfixExpression& expression) = 0;
	virtual void visit(const PrefixExpression& expression) = 0;
	virtual void visit(const UnaryExpression& expression) = 0;
	virtual void visit(const TypeCast& expression) = 0;
	virtual void visit(const PointerCast& expression) = 0;
	virtual void visit(const ArithmeticExpression& expression) = 0;
	virtual void visit(const ShiftExpression& expression) = 0;
	virtual void visit(const ComparisonExpression& expression) = 0;
	virtual void visit(const BitwiseExpression& expression) = 0;
	virtual void visit(const LogicalAndExpression& expression) = 0;
	virtual void visit(const LogicalOrExpression& expression) = 0;
	virtual void visit(const AssignmentExpression& expression) = 0;
	virtual void visit(const ExpressionList& expression) = 0;

	virtual void visit(const JumpStatement& statement) = 0;
	virtual void visit(const ReturnStatement& statement) = 0;
	virtual void visit(const IOStatement& statement) = 0;
	virtual void visit(const IfStatement& statement) = 0;
	virtual void visit(const IfElseStatement& statement) = 0;
	virtual void visit(const LoopStatement& statement) = 0;

	virtual void visit(const ForLoopHeader& loopHeader) = 0;
	virtual void visit(const WhileLoopHeader& loopHeader) = 0;

	virtual void visit(const Pointer& pointer) = 0;

	virtual void visit(const Identifier& identifier) = 0;
	virtual void visit(const FunctionDeclaration& declaration) = 0;
	virtual void visit(const ArrayDeclaration& declaration) = 0;

	virtual void visit(const ParameterDeclaration& parameter) = 0;

	virtual void visit(const FunctionDefinition& function) = 0;

	virtual void visit(const VariableDeclaration& declaration) = 0;
	virtual void visit(const VariableDefinition& definition) = 0;

	virtual void visit(const Block& block) = 0;
	virtual void visit(const ListCarrier& listCarrier) = 0;

	virtual void visit(const TranslationUnit& translationUnit) = 0;
};

} /* namespace semantic_analyzer */

#endif /* ABSTRACTSYNTAXTREEVISITOR_H_ */
