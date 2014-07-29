#ifndef ABSTRACTSYNTAXTREEVISITOR_H_
#define ABSTRACTSYNTAXTREEVISITOR_H_

namespace semantic_analyzer {

class NonterminalNode;
class AbstractSyntaxTreeNode;
class ParameterList;
class AssignmentExpressionList;
class DeclarationList;
class ArrayAccess;
class FunctionCall;
class NoArgFunctionCall;
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

class AbstractSyntaxTreeVisitor {
public:
	virtual ~AbstractSyntaxTreeVisitor() {
	}

	virtual void visit(const AbstractSyntaxTreeNode& node) const = 0;
	virtual void visit(const NonterminalNode& node) const = 0;

	virtual void visit(const ParameterList& parameterList) const = 0;
	virtual void visit(const AssignmentExpressionList& expressions) const = 0;
	virtual void visit(const DeclarationList& declarations) const = 0;
	virtual void visit(const ArrayAccess& arrayAccess) const = 0;
	virtual void visit(const FunctionCall& functionCall) const = 0;
	virtual void visit(const NoArgFunctionCall& functionCall) const = 0;
	virtual void visit(const PostfixExpression& expression) const = 0;
	virtual void visit(const PrefixExpression& expression) const = 0;
	virtual void visit(const UnaryExpression& expression) const = 0;
	virtual void visit(const TypeCast& expression) const = 0;
	virtual void visit(const PointerCast& expression) const = 0;
	virtual void visit(const ArithmeticExpression& expression) const = 0;
	virtual void visit(const ShiftExpression& expression) const = 0;
	virtual void visit(const ComparisonExpression& expression) const = 0;
	virtual void visit(const BitwiseExpression& expression) const = 0;
	virtual void visit(const LogicalAndExpression& expression) const = 0;
	virtual void visit(const LogicalOrExpression& expression) const = 0;
	virtual void visit(const AssignmentExpression& expression) const = 0;
	virtual void visit(const ExpressionList& expression) const = 0;

	virtual void visit(const JumpStatement& statement) const = 0;
	virtual void visit(const ReturnStatement& statement) const = 0;
	virtual void visit(const IOStatement& statement) const = 0;
	virtual void visit(const IfStatement& statement) const = 0;
	virtual void visit(const IfElseStatement& statement) const = 0;
	virtual void visit(const LoopStatement& statement) const = 0;

	virtual void visit(const ForLoopHeader& loopHeader) const = 0;
	virtual void visit(const WhileLoopHeader& loopHeader) const = 0;

	virtual void visit(const Pointer& pointer) const = 0;

	virtual void visit(const Identifier& identifier) const = 0;
	virtual void visit(const FunctionDeclaration& declaration) const = 0;
	virtual void visit(const ArrayDeclaration& declaration) const = 0;

	virtual void visit(const ParameterDeclaration& parameter) const = 0;

	virtual void visit(const FunctionDefinition& function) const = 0;

	virtual void visit(const VariableDeclaration& declaration) const = 0;
	virtual void visit(const VariableDefinition& definition) const = 0;

	virtual void visit(const Block& block) const = 0;
	virtual void visit(const ListCarrier& listCarrier) const = 0;
};

} /* namespace semantic_analyzer */

#endif /* ABSTRACTSYNTAXTREEVISITOR_H_ */
