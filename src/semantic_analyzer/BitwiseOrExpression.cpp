#include "BitwiseOrExpression.h"

#include <string>
#include <vector>

#include "../code_generator/quadruple.h"
#include "../code_generator/symbol_table.h"

namespace semantic_analyzer {

const std::string BitwiseOrExpression::ID { "<or_expr>" };

BitwiseOrExpression::BitwiseOrExpression(Expression* bitwiseOrExpression, Expression* xorExpression,
		SymbolTable *st, unsigned ln) :
		Expression(ID, { bitwiseOrExpression, xorExpression }, st, ln) {
	code = bitwiseOrExpression->getCode();
	value = "rval";
	basic_type = bitwiseOrExpression->getBasicType();
	extended_type = bitwiseOrExpression->getExtendedType();
	SymbolEntry *arg1 = bitwiseOrExpression->getPlace();
	SymbolEntry *arg2 = xorExpression->getPlace();
	string check = s_table->typeCheck(arg1, arg2);
	if (check != "ok") {
		semanticError(check);
	} else {
		vector<Quadruple *> arg2code = xorExpression->getCode();
		code.insert(code.end(), arg2code.begin(), arg2code.end());
		place = s_table->newTemp(basic_type, extended_type);
		code.push_back(new Quadruple(OR, arg1, arg2, place));
	}
}

}
