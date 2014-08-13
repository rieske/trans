#include "PointerCast.h"

#include <algorithm>
#include <string>
#include <vector>

#include "../code_generator/quadruple.h"
#include "../code_generator/symbol_table.h"
#include "AbstractSyntaxTreeVisitor.h"
#include "BasicType.h"
#include "Pointer.h"

namespace semantic_analyzer {

PointerCast::PointerCast(TypeSpecifier type, std::unique_ptr<Pointer> pointer,
        std::unique_ptr<Expression> castExpression) :
        type { type },
        pointer { std::move(pointer) },
        castExpression { std::move(castExpression) } {
    basicType = type.getType();
    extended_type = this->pointer->getExtendedType();
    value = "rval";
}

void PointerCast::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

}
