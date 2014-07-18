#include "AdditionExpression.h"

#include <string>
#include <vector>

#include "../code_generator/quadruple.h"
#include "../code_generator/symbol_table.h"
#include "TerminalSymbol.h"

namespace semantic_analyzer {

const std::string AdditionExpression::ID = "<add_expr>";

AdditionExpression::AdditionExpression(Expression* addExpression, TerminalSymbol addittionOperator, Expression* factor, SymbolTable *st, unsigned ln) :
		Expression(ID, { addExpression, factor }, st, ln) {
	code = addExpression->getCode();
	value = "rval";
	basic_type = addExpression->getBasicType();
	extended_type = addExpression->getExtendedType();
	SymbolEntry *arg1 = addExpression->getPlace();
	SymbolEntry *arg2 = factor->getPlace();
	string check = s_table->typeCheck(arg1, arg2);
	if (check != "ok") {
		semanticError(check);
	} else {
		char op = addittionOperator.value.at(0);
		vector<Quadruple *> arg2code = factor->getCode();
		code.insert(code.end(), arg2code.begin(), arg2code.end());
		SymbolEntry *res = s_table->newTemp(basic_type, extended_type);
		place = res;
		switch (op) {
		case '+':
			code.push_back(new Quadruple(ADD, arg1, arg2, res));
			break;
		case '-':
			code.push_back(new Quadruple(SUB, arg1, arg2, res));
			break;
		default:
			semanticError("unidentified add_op operator!\n");
		}
	}
}

}
