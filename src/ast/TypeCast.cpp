#include "TypeCast.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "ParseEnvironment.h"
#include "types/IntegerConstant.h"
#include "types/TypeQuery.h"

namespace ast {

TypeCast::TypeCast(TypeName typeName, std::unique_ptr<Expression> castExpression) :
        SingleOperandExpression { std::move(castExpression), std::unique_ptr<Operator> { new Operator(typeName.getName()) } },
        typeName { std::move(typeName) } {
}

TypeCast::~TypeCast() {
}

void TypeCast::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

std::optional<type::Type> TypeCast::typeAtParseTime(const ParseEnvironment& environment) const {
    return typeName.tryResolve(environment);
}

TypeName& TypeCast::getTypeName() {
    return typeName;
}

const TypeName& TypeCast::getTypeName() const {
    return typeName;
}

bool TypeCast::isLval() const {
    return false;
}

bool TypeCast::evaluateConstant(type::IntegerConstant& value) const {
    if (!_operand || !_operand->evaluateConstant(value)) {
        return false;
    }
    if (!hasExpressionType() && !typeName.spec.hasType()) {
        return false;
    }
    const type::Type dest = hasExpressionType() ? getType() : typeName.spec.getType();
    if (!type::isIntegral(dest) && !type::isBoolean(dest) && !dest.isPointer()) {
        return false;
    }
    value = type::convert(value, dest);
    return true;
}

} // namespace ast
