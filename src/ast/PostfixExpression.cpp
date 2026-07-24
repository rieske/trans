#include "PostfixExpression.h"
#include <cassert>
#include "AbstractSyntaxTreeVisitor.h"
#include "symbols/AnnotationStore.h"
namespace ast {
PostfixExpression::PostfixExpression(std::unique_ptr<Expression> postfixExpression, std::unique_ptr<Operator> postfixOperator)
    : SingleOperandExpression(std::move(postfixExpression), std::move(postfixOperator)) {}
void PostfixExpression::accept(AbstractSyntaxTreeVisitor& visitor) { visitor.visit(*this); }
void PostfixExpression::setPreOperationSymbol(symbols::ValueEntry resultSymbol) {
    setType(resultSymbol.getType());
    symbols::AnnotationStore::current().setValue(this, symbols::ValueSlot::PreOperation, std::move(resultSymbol));
}
symbols::ValueEntry* PostfixExpression::getPreOperationSymbol() const {
    auto* p = symbols::AnnotationStore::current().value(this, symbols::ValueSlot::PreOperation);
    assert(p);
    return p;
}
} // namespace ast
