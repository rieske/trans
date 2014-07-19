#ifndef ABSTRACTSYNTAXTREEVISITOR_H_
#define ABSTRACTSYNTAXTREEVISITOR_H_

namespace semantic_analyzer {

class AbstractSyntaxTreeNode;

class AbstractSyntaxTreeVisitor {
public:
	AbstractSyntaxTreeVisitor();
	virtual ~AbstractSyntaxTreeVisitor();

	virtual void visit(const AbstractSyntaxTreeNode& node) const;
};

} /* namespace semantic_analyzer */

#endif /* ABSTRACTSYNTAXTREEVISITOR_H_ */
