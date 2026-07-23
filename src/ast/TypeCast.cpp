#include "TypeCast.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "types/TypeQuery.h"

namespace ast {

TypeCast::TypeCast(TypeSpecifier typeSpecifier, std::unique_ptr<Expression> castExpression) :
        SingleOperandExpression { std::move(castExpression), std::unique_ptr<Operator> { new Operator(typeSpecifier.getName()) } }, typeSpecifier {
                typeSpecifier } {
}

TypeCast::~TypeCast() {
}

void TypeCast::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

TypeSpecifier TypeCast::getType() const {
    return typeSpecifier;
}

bool TypeCast::evaluateConstant(long& value) const {
    if (!_operand->evaluateConstant(value)) {
        return false;
    }
    // Apply integer cast truncation when folding constants (e.g. (unsigned char)-1).
    type::Type target = typeSpecifier.getType();
    if (type::isIntegral(target)) {
        const int size = target.getSize();
        if (size > 0 && size < 8) {
            const unsigned long bits = static_cast<unsigned long>(value);
            if (size == 1) {
                unsigned char narrowed = static_cast<unsigned char>(bits);
                if (type::valueIsSigned(target)) {
                    value = static_cast<signed char>(narrowed);
                } else {
                    value = narrowed;
                }
            } else if (size == 4) {
                unsigned u = static_cast<unsigned>(bits);
                if (type::valueIsSigned(target)) {
                    value = static_cast<int>(u);
                } else {
                    value = static_cast<long>(u);
                }
            }
        }
    }
    return true;
}

} // namespace ast

