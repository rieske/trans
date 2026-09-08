#include "StatementExpression.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "ParseEnvironment.h"
#include "types/Type.h"
#include "types/TypeQuery.h"

namespace ast {

StatementExpression::StatementExpression(translation_unit::Context context, std::unique_ptr<Block> body) :
        context_ { std::move(context) },
        body_ { std::move(body) } {
}

void StatementExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

Expression* StatementExpression::valueExpression() {
    auto& items = body_->getItems();
    return items.empty() ? nullptr : items.back().asExpression();
}

const Expression* StatementExpression::valueExpression() const {
    const auto& items = body_->getItems();
    return items.empty() ? nullptr : items.back().asExpression();
}

std::optional<type::Type> StatementExpression::typeAtParseTime(const ParseEnvironment& environment) const {
    ParseEnvironment inner = ParseEnvironment::nestedIn(environment);
    inner.bindBlockDeclarations(*body_);
    if (const auto* last = valueExpression()) {
        const auto type = last->typeAtParseTime(inner);
        return type ? type::afterLvalueConversion(*type) : type;
    }
    return type::voidType();
}

translation_unit::Context StatementExpression::getContext() const {
    return context_;
}

} // namespace ast
