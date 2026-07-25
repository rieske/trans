#include "InitializerListExpression.h"
#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

InitializerListExpression::InitializerListExpression(std::vector<std::unique_ptr<Expression>> elements) :
        elements { std::move(elements) } {
}

void InitializerListExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

translation_unit::Context InitializerListExpression::getContext() const {
    if (!elements.empty() && elements.front()) {
        return elements.front()->getContext();
    }
    return translation_unit::Context { "", 0 };
}

const std::vector<std::unique_ptr<Expression>>& InitializerListExpression::getElements() const {
    return elements;
}

void InitializerListExpression::visitElements(AbstractSyntaxTreeVisitor& visitor) {
    for (auto& e : elements) {
        if (e) {
            e->accept(visitor);
        }
    }
}

} // namespace ast
