#include "StatementExpression.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "ParseEnvironment.h"
#include "types/Type.h"

namespace ast {

StatementExpression::StatementExpression(translation_unit::Context context, std::unique_ptr<Block> body) :
        context_ { std::move(context) },
        body_ { std::move(body) } {
}

void StatementExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

std::optional<type::Type> StatementExpression::typeAtParseTime(const ParseEnvironment& environment) const {
    ParseEnvironment inner = ParseEnvironment::nestedIn(environment);
    inner.bindBlockDeclarations(*body_);
    const auto& items = body_->getItems();
    if (items.empty()) {
        return type::voidType();
    }
    if (const auto* last = items.back() ? items.back()->asExpression() : nullptr) {
        return last->typeAtParseTime(inner);
    }
    return type::voidType();
}

translation_unit::Context StatementExpression::getContext() const {
    return context_;
}

} // namespace ast
