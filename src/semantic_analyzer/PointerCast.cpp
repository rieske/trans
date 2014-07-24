#include "PointerCast.h"

#include <algorithm>
#include <string>
#include <vector>

#include "../code_generator/quadruple.h"
#include "../code_generator/symbol_table.h"
#include "AbstractSyntaxTreeVisitor.h"
#include "Pointer.h"

namespace semantic_analyzer {

PointerCast::PointerCast(TerminalSymbol typeSpecifier, std::unique_ptr<Pointer> pointer, std::unique_ptr<Expression> castExpression,
		SymbolTable *st) :
		Expression(ID, { }, st, typeSpecifier.line),
		typeSpecifier { typeSpecifier },
		pointer { std::move(pointer) },
		castExpression { std::move(castExpression) } {
	basic_type = typeSpecifier.value;
	extended_type = this->pointer->getType();
	SymbolEntry *arg = this->castExpression->getPlace();
	place = s_table->newTemp(basic_type, extended_type);
	code = this->castExpression->getCode();
	code.push_back(new Quadruple(ASSIGN, arg, NULL, place));
	value = "rval";
}

void PointerCast::accept(const AbstractSyntaxTreeVisitor& visitor) const {
	visitor.visit(*this);
}

}
