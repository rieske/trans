#include "BitwiseXorExpression.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "../code_generator/quadruple.h"
#include "../code_generator/symbol_table.h"
#include "../parser/GrammarSymbol.h"

namespace semantic_analyzer {

const std::string BitwiseXorExpression::ID { "<xor_expr>" };

BitwiseXorExpression::BitwiseXorExpression(Expression* bitwiseXorExpression, Expression* bitwiseAndExpression, SymbolTable *st, unsigned ln) :
		Expression(ID, { bitwiseXorExpression, bitwiseAndExpression }, st, ln) {
	code = bitwiseXorExpression->getCode();
	value = "rval";
	basic_type = bitwiseXorExpression->getBasicType();
	extended_type = bitwiseXorExpression->getExtendedType();
	SymbolEntry *arg1 = bitwiseXorExpression->getPlace();
	SymbolEntry *arg2 = bitwiseAndExpression->getPlace();
	string check = s_table->typeCheck(arg1, arg2);
	if (check != "ok") {
		semanticError(check);
	} else {
		vector<Quadruple *> arg2code = bitwiseAndExpression->getCode();
		code.insert(code.end(), arg2code.begin(), arg2code.end());
		place = s_table->newTemp(basic_type, extended_type);
		code.push_back(new Quadruple(XOR, arg1, arg2, place));
	}
}

BitwiseXorExpression::BitwiseXorExpression(Expression* bitwiseAndExpression, SymbolTable *st, unsigned ln) :
		Expression(ID, { bitwiseAndExpression }, st, ln) {
	saveExpressionAttributes(*bitwiseAndExpression);
}

}
