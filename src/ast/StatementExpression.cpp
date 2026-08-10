#include "StatementExpression.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

StatementExpression::StatementExpression(translation_unit::Context context, std::unique_ptr<Block> body) :
        context_ { std::move(context) },
        body_ { std::move(body) } {
}

void StatementExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

translation_unit::Context StatementExpression::getContext() const {
    return context_;
}

} // namespace ast
