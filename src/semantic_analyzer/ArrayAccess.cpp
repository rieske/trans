#include "ArrayAccess.h"

#include <algorithm>

#include "AbstractSyntaxTreeVisitor.h"

namespace semantic_analyzer {

ArrayAccess::ArrayAccess(std::unique_ptr<Expression> postfixExpression, std::unique_ptr<Expression> subscriptExpression) :
        postfixExpression { std::move(postfixExpression) },
        subscriptExpression { std::move(subscriptExpression) } {
    lval = this->postfixExpression->isLval();
    setTypeInfo( { this->postfixExpression->getBasicType(), this->postfixExpression->getExtendedType() });
}

ArrayAccess::~ArrayAccess() {
}

void ArrayAccess::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

void ArrayAccess::setLvalue(SymbolEntry* lvalue) {
    this->lvalue = lvalue;
}

SymbolEntry* ArrayAccess::getLvalue() const {
    return lvalue;
}

} /* namespace semantic_analyzer */
