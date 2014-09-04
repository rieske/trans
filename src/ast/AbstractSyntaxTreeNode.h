#ifndef ABSTRACTSYNTAXTREENODE_H_
#define ABSTRACTSYNTAXTREENODE_H_

namespace ast {

class AbstractSyntaxTreeVisitor;

class AbstractSyntaxTreeNode {
public:
    virtual ~AbstractSyntaxTreeNode() {
    }

    virtual void accept(AbstractSyntaxTreeVisitor& visitor) = 0;
};

} /* namespace ast */

#endif /* ABSTRACTSYNTAXTREENODE_H_ */
