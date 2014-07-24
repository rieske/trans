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

class AbstractSyntaxTreeVisitor {
public:
	virtual ~AbstractSyntaxTreeVisitor() {
	}

	virtual void visit(const AbstractSyntaxTreeNode& node) const = 0;
	virtual void visit(const NonterminalNode& node) const = 0;

	virtual void visit(const ParameterList& parameterList) const = 0;
	virtual void visit(const AssignmentExpressionList& assignmentExpressions) const = 0;
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
};

} /* namespace semantic_analyzer */

#endif /* ABSTRACTSYNTAXTREEVISITOR_H_ */
