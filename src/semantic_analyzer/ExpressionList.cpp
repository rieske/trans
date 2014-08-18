#include "ExpressionList.h"

#include <algorithm>

#include "AbstractSyntaxTreeVisitor.h"

namespace semantic_analyzer {

ExpressionList::ExpressionList(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Expression> rightHandSide) :
        leftHandSide { std::move(leftHandSide) },
        rightHandSide { std::move(rightHandSide) } {
    lval = this->leftHandSide->isLval();
    basicType = this->leftHandSide->getBasicType();
    extended_type = this->leftHandSide->getExtendedType();
}

ExpressionList::~ExpressionList() {
}

void ExpressionList::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

} /* namespace semantic_analyzer */
