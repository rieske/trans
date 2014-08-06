#include "TypeCast.h"

#include <algorithm>
#include <string>
#include <vector>

#include "../code_generator/quadruple.h"
#include "../code_generator/symbol_table.h"
#include "AbstractSyntaxTreeVisitor.h"

namespace semantic_analyzer {

const std::string TypeCast::ID { "<cast_expr>" };

TypeCast::TypeCast(TypeSpecifier typeSpecifier, std::unique_ptr<Expression> castExpression, SymbolTable *st) :
		Expression(st, 0),
		typeSpecifier { typeSpecifier },
		castExpression { std::move(castExpression) } {
	basicType = typeSpecifier.getType();
	extended_type = "";
	SymbolEntry *arg = this->castExpression->getPlace();
	place = s_table->newTemp(basicType, extended_type);
	code = this->castExpression->getCode();
	code.push_back(new Quadruple(ASSIGN, arg, NULL, place));
	value = "rval";
}

TypeCast::~TypeCast() {
}

void TypeCast::accept(AbstractSyntaxTreeVisitor& visitor) const {
	visitor.visit(*this);
}

} /* namespace semantic_analyzer */
