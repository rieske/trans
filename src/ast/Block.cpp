#include "Block.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "Declaration.h"

namespace ast {

Block::Block(std::vector<std::unique_ptr<AbstractSyntaxTreeNode>> items) :
        items { std::move(items) }
{
}

Block::Block(std::vector<std::unique_ptr<Declaration>> declarations,
        std::vector<std::unique_ptr<AbstractSyntaxTreeNode>> statements) {
    items.reserve(declarations.size() + statements.size());
    for (auto& declaration : declarations) {
        items.push_back(std::move(declaration));
    }
    for (auto& statement : statements) {
        items.push_back(std::move(statement));
    }
}

void Block::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

void Block::visitChildren(AbstractSyntaxTreeVisitor& visitor) {
    for (const auto& item : items) {
        item->accept(visitor);
    }
}

const std::vector<std::unique_ptr<AbstractSyntaxTreeNode>>& Block::getItems() const {
    return items;
}

} // namespace ast
