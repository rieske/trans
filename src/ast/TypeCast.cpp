#include "TypeCast.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "types/TypeQuery.h"

namespace ast {
namespace {

long narrowIntegerConstant(long value, int size, bool isSigned) {
    const unsigned long bits = static_cast<unsigned long>(value);
    if (size == 1) {
        unsigned char narrowed = static_cast<unsigned char>(bits);
        return isSigned ? static_cast<long>(static_cast<signed char>(narrowed)) : narrowed;
    }
    if (size == 2) {
        unsigned short narrowed = static_cast<unsigned short>(bits);
        return isSigned ? static_cast<long>(static_cast<short>(narrowed)) : narrowed;
    }
    if (size == 4) {
        unsigned u = static_cast<unsigned>(bits);
        return isSigned ? static_cast<long>(static_cast<int>(u)) : static_cast<long>(u);
    }
    return value;
}

} // namespace

TypeCast::TypeCast(TypeName typeName, std::unique_ptr<Expression> castExpression) :
        SingleOperandExpression { std::move(castExpression), std::unique_ptr<Operator> { new Operator(typeName.getName()) } },
        typeName { std::move(typeName) } {
}

TypeCast::~TypeCast() {
}

void TypeCast::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
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

bool TypeCast::evaluateConstant(long& value) const {
    if (!_operand || !_operand->evaluateConstant(value)) {
        return false;
    }
    if (!typeName.spec.hasType()) {
        return false;
    }
    // Apply integer cast truncation when folding constants (e.g. (unsigned char)-1).
    type::Type target = typeName.spec.getType();
    if (type::isBoolean(target)) {
        value = type::convertScalarConstant(target, value);
    } else if (type::isIntegral(target)) {
        value = narrowIntegerConstant(value, target.getSize(), type::valueIsSigned(target));
    }
    return true;
}

} // namespace ast
