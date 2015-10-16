#include "ast/ArrayAccess.h"

#include <algorithm>

#include "ast/AbstractSyntaxTreeVisitor.h"
#include "ast/Operator.h"

namespace ast {

ArrayAccess::ArrayAccess(std::unique_ptr<Expression> postfixExpression, std::unique_ptr<Expression> subscriptExpression) :
        DoubleOperandExpression(std::move(postfixExpression), std::move(subscriptExpression), std::unique_ptr<Operator> { new Operator("[]") }) {
    lval = this->leftOperand->isLval();
}

void ArrayAccess::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

void ArrayAccess::setLvalue(semantic_analyzer::ValueEntry lvalue) {
    this->lvalue = std::make_unique<semantic_analyzer::ValueEntry>(lvalue);
}

semantic_analyzer::ValueEntry* ArrayAccess::getLvalue() const {
    return lvalue.get();
}

} /* namespace ast */
