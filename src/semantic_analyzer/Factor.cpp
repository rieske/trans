#include "Factor.h"

#include <string>
#include <vector>

#include "../code_generator/quadruple.h"
#include "../code_generator/symbol_table.h"
#include "TerminalSymbol.h"

namespace semantic_analyzer {

const std::string Factor::ID { "<factor>" };

Factor::Factor(Expression* factor, TerminalSymbol multiplicationOperator, Expression* castExpression, SymbolTable *st, unsigned ln) :
		Expression(ID, { factor, castExpression }, st, ln) {
	code = factor->getCode();
	value = "rval";
	basic_type = factor->getBasicType();
	extended_type = factor->getExtendedType();
	SymbolEntry *arg1 = factor->getPlace();
	SymbolEntry *arg2 = castExpression->getPlace();
	string check = s_table->typeCheck(arg1, arg2);
	if (check != "ok") {
		semanticError(check);
	} else {
		vector<Quadruple *> arg2code = castExpression->getCode();
		code.insert(code.end(), arg2code.begin(), arg2code.end());
		SymbolEntry *res = s_table->newTemp(basic_type, extended_type);
		place = res;
		switch (multiplicationOperator.value.at(0)) {
		case '*':
			code.push_back(new Quadruple(MUL, arg1, arg2, res));
			break;
		case '/':
			code.push_back(new Quadruple(DIV, arg1, arg2, res));
			break;
		case '%':
			code.push_back(new Quadruple(MOD, arg1, arg2, res));
			break;
		default:
			semanticError("unidentified m_op operator!\n");
		}
	}
}

}
