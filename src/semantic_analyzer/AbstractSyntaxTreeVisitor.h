#ifndef ABSTRACTSYNTAXTREEVISITOR_H_
#define ABSTRACTSYNTAXTREEVISITOR_H_

namespace semantic_analyzer {

class NonterminalNode;
class AbstractSyntaxTreeNode;
class ParameterList;
class AssignmentExpressionList;
class DeclarationList;

class AbstractSyntaxTreeVisitor {
public:
	virtual ~AbstractSyntaxTreeVisitor() {
	}

	virtual void visit(const AbstractSyntaxTreeNode& node) const = 0;
	virtual void visit(const NonterminalNode& node) const = 0;

	virtual void visit(const ParameterList& parameterList) const = 0;
	virtual void visit(const AssignmentExpressionList& assignmentExpressions) const = 0;
	virtual void visit(const DeclarationList& declarations) const = 0;
};

} /* namespace semantic_analyzer */

#endif /* ABSTRACTSYNTAXTREEVISITOR_H_ */
