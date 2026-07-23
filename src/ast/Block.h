#ifndef _BLOCK_NODE_H_
#define _BLOCK_NODE_H_

#include <memory>
#include <vector>

#include "AbstractSyntaxTreeNode.h"

namespace ast {

// Compound statement body. Items are declarations and/or statements in source order
// (C99 allows interleaving via <block_item_list>).
class Block: public AbstractSyntaxTreeNode {
public:
    explicit Block(std::vector<std::unique_ptr<AbstractSyntaxTreeNode>> items);
    virtual ~Block() = default;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    void visitChildren(AbstractSyntaxTreeVisitor& visitor) override;

private:
    std::vector<std::unique_ptr<AbstractSyntaxTreeNode>> items;
};

} // namespace ast

#endif // _BLOCK_NODE_H_
