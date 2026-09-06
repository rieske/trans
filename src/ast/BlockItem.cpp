#include "BlockItem.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

BlockItem::BlockItem(std::unique_ptr<Declaration> declaration) :
        item_ { std::move(declaration) } {
}

BlockItem::BlockItem(std::unique_ptr<Expression> expression) :
        item_ { std::move(expression) } {
}

BlockItem::BlockItem(std::unique_ptr<AbstractSyntaxTreeNode> statement) :
        item_ { std::move(statement) } {
}

BlockItem BlockItem::fromNode(std::unique_ptr<AbstractSyntaxTreeNode> node) {
    if (!node) {
        return BlockItem { std::unique_ptr<AbstractSyntaxTreeNode> {} };
    }
    if (auto* declaration = node->asDeclaration()) {
        node.release();
        return BlockItem { std::unique_ptr<Declaration> { declaration } };
    }
    if (auto* expression = node->asExpression()) {
        node.release();
        return BlockItem { std::unique_ptr<Expression> { expression } };
    }
    return BlockItem { std::move(node) };
}

const Declaration* BlockItem::asDeclaration() const {
    if (const auto* held = std::get_if<std::unique_ptr<Declaration>>(&item_)) {
        return held->get();
    }
    return nullptr;
}

Declaration* BlockItem::asDeclaration() {
    return const_cast<Declaration*>(
            static_cast<const BlockItem*>(this)->asDeclaration());
}

const Expression* BlockItem::asExpression() const {
    if (const auto* held = std::get_if<std::unique_ptr<Expression>>(&item_)) {
        return held->get();
    }
    return nullptr;
}

Expression* BlockItem::asExpression() {
    return const_cast<Expression*>(
            static_cast<const BlockItem*>(this)->asExpression());
}

void BlockItem::accept(AbstractSyntaxTreeVisitor& visitor) const {
    std::visit([&](const auto& held) {
        if (held) {
            held->accept(visitor);
        }
    }, item_);
}

} // namespace ast
