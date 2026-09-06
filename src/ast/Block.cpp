#include "Block.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

Block::Block(std::vector<BlockItem> items) :
        items { std::move(items) }
{
}

void Block::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

void Block::visitChildren(AbstractSyntaxTreeVisitor& visitor) {
    for (const auto& item : items) {
        item.accept(visitor);
    }
}

} // namespace ast
