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

PointerCast::PointerCast(TypeSpecifier type, std::unique_ptr<Pointer> pointer, std::unique_ptr<Expression> castExpression,
		SymbolTable *st) :
		Expression(st, 0),
		type { type },
		pointer { std::move(pointer) },
		castExpression { std::move(castExpression) } {
	basicType = type.getType();
	extended_type = this->pointer->getDereferenceCount();
	SymbolEntry *arg = this->castExpression->getPlace();
	resultPlace = s_table->newTemp(basicType, extended_type);
	code = this->castExpression->getCode();
	code.push_back(new Quadruple(ASSIGN, arg, NULL, resultPlace));
	value = "rval";
}

void PointerCast::accept(AbstractSyntaxTreeVisitor& visitor) {
	visitor.visit(*this);
}

}
