#include "ArrayAccess.h"

#include <algorithm>

#include "AbstractSyntaxTreeVisitor.h"
#include "Operator.h"

namespace ast {

ArrayAccess::ArrayAccess(std::unique_ptr<Expression> postfixExpression, std::unique_ptr<Expression> subscriptExpression) :
        DoubleOperandExpression(std::move(postfixExpression), std::move(subscriptExpression), std::unique_ptr<Operator> { new Operator("[]") }) {
    lval = this->leftOperand->isLval();
}

ArrayAccess::~ArrayAccess() {
}

void ArrayAccess::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

void ArrayAccess::setLvalue(code_generator::ValueEntry lvalue) {
    this->lvalue = std::make_unique<code_generator::ValueEntry>(lvalue);
}

code_generator::ValueEntry* ArrayAccess::getLvalue() const {
    return lvalue.get();
}

} /* namespace ast */
