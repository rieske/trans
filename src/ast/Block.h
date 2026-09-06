#ifndef _BLOCK_NODE_H_
#define _BLOCK_NODE_H_

#include <vector>

#include "BlockItem.h"
#include "Statement.h"

namespace ast {

// Compound statement body. Items are declarations and/or statements in source order
// (C99 allows interleaving via <block_item_list>).
class Block: public Statement {
public:
    Block() = default;
    explicit Block(std::vector<BlockItem> items);
    virtual ~Block() = default;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    NodeKind nodeKind() const override { return NodeKind::Block; }
    void visitChildren(AbstractSyntaxTreeVisitor& visitor) override;

    const std::vector<BlockItem>& getItems() const { return items; }
    std::vector<BlockItem>& getItems() { return items; }

private:
    std::vector<BlockItem> items;
};

} // namespace ast

#endif // _BLOCK_NODE_H_
