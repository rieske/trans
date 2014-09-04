#include "PointerCast.h"

#include <algorithm>

#include "AbstractSyntaxTreeVisitor.h"
#include "Operator.h"
#include "Pointer.h"

namespace ast {

PointerCast::PointerCast(TypeSpecifier type, std::unique_ptr<Pointer> pointer, std::unique_ptr<Expression> castExpression) :
        SingleOperandExpression(std::move(castExpression), std::unique_ptr<Operator> { new Operator(type.getName()) }), type { type }, pointer { std::move(
                pointer) } {
}

void PointerCast::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

TypeSpecifier PointerCast::getType() const {
    return type;
}

Pointer* PointerCast::getPointer() const {
    return pointer.get();
}

}
