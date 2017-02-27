#include "ConstantExpression.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

ConstantExpression::ConstantExpression(Constant constant) :
        constant { constant }
{
    auto type = constant.getType();
    setType(*type);
}

ConstantExpression::~ConstantExpression() {
}

void ConstantExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

translation_unit::Context ConstantExpression::getContext() const {
    return constant.getContext();
}

std::string ConstantExpression::getValue() const {
    return constant.getValue();
}

} /* namespace ast */

