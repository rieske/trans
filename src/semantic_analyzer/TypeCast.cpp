#include "TypeCast.h"

#include <algorithm>
#include <string>
#include <vector>

#include "../code_generator/quadruple.h"
#include "../code_generator/symbol_table.h"
#include "AbstractSyntaxTreeVisitor.h"

namespace semantic_analyzer {

const std::string TypeCast::ID { "<cast_expr>" };

TypeCast::TypeCast(TerminalSymbol typeSpecifier, std::unique_ptr<Expression> castExpression, SymbolTable *st) :
		Expression(st, typeSpecifier.line),
		typeSpecifier { typeSpecifier },
		castExpression { std::move(castExpression) } {
	basic_type = typeSpecifier.value;
	extended_type = "";
	SymbolEntry *arg = this->castExpression->getPlace();
	place = s_table->newTemp(basic_type, extended_type);
	code = this->castExpression->getCode();
	code.push_back(new Quadruple(ASSIGN, arg, NULL, place));
	value = "rval";
}

TypeCast::~TypeCast() {
}

void TypeCast::accept(const AbstractSyntaxTreeVisitor& visitor) const {
	visitor.visit(*this);
}

} /* namespace semantic_analyzer */
