#include "AbstractSyntaxTreeNode.h"

#include "Block.h"

namespace ast {

Block* AbstractSyntaxTreeNode::asBlock() {
    return nodeKind() == NodeKind::Block
            ? static_cast<Block*>(this) : nullptr;
}

} // namespace ast
