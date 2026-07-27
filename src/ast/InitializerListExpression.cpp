#include "InitializerListExpression.h"
#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

InitializerListExpression::InitializerListExpression(std::vector<InitializerElement> elements) :
        elements { std::move(elements) } {
}

void InitializerListExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

translation_unit::Context InitializerListExpression::getContext() const {
    if (!elements.empty() && elements.front().value) {
        return elements.front().value->getContext();
    }
    return translation_unit::Context { "", 0 };
}

const std::vector<InitializerElement>& InitializerListExpression::getElements() const {
    return elements;
}

void InitializerListExpression::visitElements(AbstractSyntaxTreeVisitor& visitor) {
    for (auto& e : elements) {
        // Foldable designator indexes (sizeof, etc.) need SA before lowering.
        for (auto& step : e.designator) {
            if (step.indexExpression) {
                step.indexExpression->accept(visitor);
            }
        }
        if (e.value) {
            e.value->accept(visitor);
        }
    }
}

} // namespace ast
