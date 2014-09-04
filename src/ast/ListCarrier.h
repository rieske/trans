#ifndef LISTCARRIER_H_
#define LISTCARRIER_H_

#include <memory>
#include <vector>

#include "AbstractSyntaxTreeNode.h"

namespace ast {

class ListCarrier: public AbstractSyntaxTreeNode {
public:
    ListCarrier(std::unique_ptr<AbstractSyntaxTreeNode> child);
    virtual ~ListCarrier();

    void addChild(std::unique_ptr<AbstractSyntaxTreeNode> child);

    const std::vector<std::unique_ptr<AbstractSyntaxTreeNode>>& getChildren() const;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

private:
    std::vector<std::unique_ptr<AbstractSyntaxTreeNode>> children;
};

} /* namespace ast */

#endif /* LISTCARRIER_H_ */
