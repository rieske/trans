#ifndef ABSTRACTSYNTAXTREENODE_H_
#define ABSTRACTSYNTAXTREENODE_H_

namespace ast {

class AbstractSyntaxTreeVisitor;

class AbstractSyntaxTreeNode {
public:
    AbstractSyntaxTreeNode() = default;
    virtual ~AbstractSyntaxTreeNode() = default;

    virtual void accept(AbstractSyntaxTreeVisitor& visitor) = 0;

protected:
    AbstractSyntaxTreeNode(const AbstractSyntaxTreeNode&) = default;
    AbstractSyntaxTreeNode(AbstractSyntaxTreeNode&&) = default;
    AbstractSyntaxTreeNode& operator=(const AbstractSyntaxTreeNode&) = default;
    AbstractSyntaxTreeNode& operator=(AbstractSyntaxTreeNode&&) = default;
};

} /* namespace ast */

#endif /* ABSTRACTSYNTAXTREENODE_H_ */
