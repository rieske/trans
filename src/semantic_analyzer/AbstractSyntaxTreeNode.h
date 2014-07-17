#ifndef ABSTRACTSYNTAXTREENODE_H_
#define ABSTRACTSYNTAXTREENODE_H_

namespace semantic_analyzer {

class AbstractSyntaxTreeVisitor;

class AbstractSyntaxTreeNode {
public:
	AbstractSyntaxTreeNode();
	virtual ~AbstractSyntaxTreeNode();

	virtual void accept(AbstractSyntaxTreeVisitor& visitor) const;
};

} /* namespace semantic_analyzer */

#endif /* ABSTRACTSYNTAXTREENODE_H_ */
