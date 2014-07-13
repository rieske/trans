#include "BitwiseAndExpression.h"

#include <vector>

#include "../code_generator/quadruple.h"
#include "../code_generator/symbol_table.h"
#include "../parser/TerminalNode.h"

namespace semantic_analyzer {

const std::string BitwiseAndExpression::ID = "<and_expr>";

BitwiseAndExpression::BitwiseAndExpression(Expression* andExpression, TerminalNode* ampersand, Expression* equalityExpression, SymbolTable *st, unsigned ln) :
		Expression(ID, { andExpression, ampersand, equalityExpression }, st, ln) {
	code = andExpression->getCode();
	value = "rval";
	basic_type = andExpression->getBasicType();
	extended_type = andExpression->getExtendedType();
	SymbolEntry *arg1 = andExpression->getPlace();
	SymbolEntry *arg2 = equalityExpression->getPlace();
	string check = s_table->typeCheck(arg1, arg2);
	if (check != "ok") {
		semanticError(check);
	} else {
		vector<Quadruple *> arg2code = equalityExpression->getCode();
		code.insert(code.end(), arg2code.begin(), arg2code.end());
		place = s_table->newTemp(basic_type, extended_type);
		code.push_back(new Quadruple(AND, arg1, arg2, place));
	}
}

BitwiseAndExpression::BitwiseAndExpression(Expression* equalityExpression, SymbolTable *st, unsigned ln) :
		Expression(ID, { equalityExpression }, st, ln) {
	getAttributes(0);
}

}
