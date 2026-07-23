#ifndef _BLOCK_NODE_H_
#define _BLOCK_NODE_H_

#include <memory>
#include <vector>

#include "AbstractSyntaxTreeNode.h"

namespace ast {

// Compound statement body. Items are declarations and/or statements in source order
// (C99 allows interleaving).
class Block: public AbstractSyntaxTreeNode {
public:
    explicit Block(std::vector<std::unique_ptr<AbstractSyntaxTreeNode>> items);
    // Compatibility constructors used by older call sites.
    Block(std::vector<std::unique_ptr<class Declaration>> declarations,
            std::vector<std::unique_ptr<AbstractSyntaxTreeNode>> statements);
    virtual ~Block() = default;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    void visitChildren(AbstractSyntaxTreeVisitor& visitor) override;

    const std::vector<std::unique_ptr<AbstractSyntaxTreeNode>>& getItems() const;

private:
    std::vector<std::unique_ptr<AbstractSyntaxTreeNode>> items;
};

}

#endif // _BLOCK_NODE_H_
