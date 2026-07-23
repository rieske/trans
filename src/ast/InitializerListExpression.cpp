#include "InitializerListExpression.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

InitializerListExpression::InitializerListExpression(std::vector<InitializerElement> elements) :
        elements { std::move(elements) }
{
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
    for (auto& element : elements) {
        if (element.value) {
            element.value->accept(visitor);
        }
    }
}

void InitializerListExpression::appendElement(InitializerElement element) {
    elements.push_back(std::move(element));
}

bool InitializerListExpression::evaluateConstantElements(std::vector<long>& values) const {
    values.clear();
    values.reserve(elements.size());
    for (const auto& element : elements) {
        if (!element.memberPath.empty() || element.arrayIndex) {
            return false;
        }
        long value = 0;
        if (!element.value || !element.value->evaluateConstant(value)) {
            return false;
        }
        values.push_back(value);
    }
    return true;
}

} // namespace ast
