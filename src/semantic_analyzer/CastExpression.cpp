#include "CastExpression.h"

#include <vector>

#include "../code_generator/quadruple.h"
#include "../code_generator/symbol_table.h"
#include "Pointer.h"
#include "TerminalSymbol.h"

namespace semantic_analyzer {

const std::string CastExpression::ID { "<cast_expr>" };

CastExpression::CastExpression(TerminalSymbol typeSpecifier, Expression* castExpression, SymbolTable *st, unsigned ln) :
		Expression(ID, { castExpression }, st, ln) {
	basic_type = typeSpecifier.value;
	extended_type = "";
	SymbolEntry *arg = castExpression->getPlace();
	place = s_table->newTemp(basic_type, extended_type);
	code = castExpression->getCode();
	code.push_back(new Quadruple(ASSIGN, arg, NULL, place));
	value = "rval";
}

CastExpression::CastExpression(TerminalSymbol typeSpecifier, Pointer* pointer, Expression* castExpression, SymbolTable *st, unsigned ln) :
		Expression(ID, { pointer, castExpression }, st, ln) {
	basic_type = typeSpecifier.value;
	extended_type = pointer->getType();
	SymbolEntry *arg = castExpression->getPlace();
	place = s_table->newTemp(basic_type, extended_type);
	code = castExpression->getCode();
	code.push_back(new Quadruple(ASSIGN, arg, NULL, place));
	value = "rval";
}

}
