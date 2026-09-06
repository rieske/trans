#include "BlockItem.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

BlockItem::BlockItem(std::unique_ptr<Declaration> declaration) :
        item_ { std::move(declaration) } {
}

BlockItem::BlockItem(std::unique_ptr<Expression> expression) :
        item_ { std::move(expression) } {
}

BlockItem::BlockItem(std::unique_ptr<Statement> statement) :
        item_ { std::move(statement) } {
}

const Declaration* BlockItem::asDeclaration() const {
    if (const auto* held = std::get_if<std::unique_ptr<Declaration>>(&item_)) {
        return held->get();
    }
    return nullptr;
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

const Statement* BlockItem::asStatement() const {
    if (const auto* held = std::get_if<std::unique_ptr<Statement>>(&item_)) {
        return held->get();
    }
    return nullptr;
}

std::unique_ptr<Expression> BlockItem::takeExpression() {
    if (auto* held = std::get_if<std::unique_ptr<Expression>>(&item_)) {
        return std::move(*held);
    }
    return nullptr;
}

std::unique_ptr<Statement> BlockItem::takeStatement() {
    if (auto* held = std::get_if<std::unique_ptr<Statement>>(&item_)) {
        return std::move(*held);
    }
    return nullptr;
}

void BlockItem::accept(AbstractSyntaxTreeVisitor& visitor) const {
    std::visit([&](const auto& held) {
        if (held) {
            held->accept(visitor);
        }
    }, item_);
}

} // namespace ast
