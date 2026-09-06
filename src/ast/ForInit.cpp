#include "ForInit.h"

#include "AbstractSyntaxTreeVisitor.h"

#include <cassert>

namespace ast {

ForInit::ForInit(std::unique_ptr<Declaration> declaration) {
    assert(declaration);
    item_ = std::move(declaration);
}

ForInit::ForInit(std::unique_ptr<Expression> expression) {
    assert(expression);
    item_ = std::move(expression);
}

const Declaration* ForInit::asDeclaration() const {
    if (const auto* held = std::get_if<std::unique_ptr<Declaration>>(&item_)) {
        return held->get();
    }
    return nullptr;
}

const Expression* ForInit::asExpression() const {
    if (const auto* held = std::get_if<std::unique_ptr<Expression>>(&item_)) {
        return held->get();
    }
    return nullptr;
}

void ForInit::accept(AbstractSyntaxTreeVisitor& visitor) const {
    if (auto* held = std::get_if<std::unique_ptr<Declaration>>(&item_)) {
        (*held)->accept(visitor);
    } else if (auto* held = std::get_if<std::unique_ptr<Expression>>(&item_)) {
        (*held)->accept(visitor);
    }
}

} // namespace ast
