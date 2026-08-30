#include "PostfixExpression.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

PostfixExpression::PostfixExpression(std::unique_ptr<Expression> postfixExpression,
        std::string lexeme) :
        UnaryOpExpression { std::move(postfixExpression), std::move(lexeme) } {
}

void PostfixExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

void PostfixExpression::setPreOperationSymbol(symbols::AnnotationStore& store, symbols::ValueEntry resultSymbol) {
    setType(resultSymbol.getType());
    store.setPreOperation(this, std::move(resultSymbol));
}

symbols::ValueEntry* PostfixExpression::getPreOperationSymbol(symbols::AnnotationStore& store) const {
    // Soft probe (nullptr when missing); CG asserts after successful SA.
    return store.preOperation(this);
}

} // namespace ast
