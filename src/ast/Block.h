#ifndef _BLOCK_NODE_H_
#define _BLOCK_NODE_H_

#include <memory>
#include <string>
#include <vector>

#include "AbstractSyntaxTreeNode.h"

namespace ast {

class Block: public AbstractSyntaxTreeNode {
public:
    Block(std::unique_ptr<AbstractSyntaxTreeNode> subblock);
    Block(std::unique_ptr<AbstractSyntaxTreeNode> firstSubblock, std::unique_ptr<AbstractSyntaxTreeNode> secondSubblock);
    virtual ~Block();

    const std::vector<std::unique_ptr<AbstractSyntaxTreeNode>>& getChildren() const;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    static const std::string ID;

    void setSize(int size);
    int getSize() const;

private:
    std::vector<std::unique_ptr<AbstractSyntaxTreeNode>> children;

    int size { 0 };
};

}

#endif // _BLOCK_NODE_H_
