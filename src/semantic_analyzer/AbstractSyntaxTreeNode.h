#ifndef ABSTRACTSYNTAXTREENODE_H_
#define ABSTRACTSYNTAXTREENODE_H_

namespace semantic_analyzer {

class AbstractSyntaxTreeVisitor;

class AbstractSyntaxTreeNode {
public:
    virtual ~AbstractSyntaxTreeNode() {
    }

    virtual void accept(AbstractSyntaxTreeVisitor& visitor) = 0;
};

} /* namespace semantic_analyzer */

#endif /* ABSTRACTSYNTAXTREENODE_H_ */
