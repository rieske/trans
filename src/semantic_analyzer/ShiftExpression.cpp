#include "ShiftExpression.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

#include "../code_generator/quadruple.h"
#include "../code_generator/symbol_table.h"
#include "../parser/GrammarSymbol.h"

namespace semantic_analyzer {

const std::string ShiftExpression::ID { "<s_expr>" };

ShiftExpression::ShiftExpression(Expression* shiftExpression, ParseTreeNode* shiftOperator, Expression* additionExpression, SymbolTable *st,
		unsigned ln) :
		Expression(ID, { shiftExpression, shiftOperator, additionExpression }, st, ln) {
	code = shiftExpression->getCode();
	value = "rval";
	basic_type = shiftExpression->getBasicType();
	extended_type = shiftExpression->getExtendedType();
	SymbolEntry *arg1 = shiftExpression->getPlace();
	SymbolEntry *arg2 = additionExpression->getPlace();
	string arg2type = additionExpression->getBasicType();
	string arg2etype = additionExpression->getExtendedType();
	if (arg2type == "int" && arg2etype == "") {
		char op = shiftOperator->getAttr().at(0);
		vector<Quadruple *> arg2code = additionExpression->getCode();
		code.insert(code.end(), arg2code.begin(), arg2code.end());
		SymbolEntry *res = s_table->newTemp(basic_type, extended_type);
		place = res;
		switch (op) {
		case '<':   // <<
			code.push_back(new Quadruple(SHL, arg1, arg2, res));
			break;
		case '>':   // >>
			code.push_back(new Quadruple(SHR, arg1, arg2, res));
			break;
		default:
			semanticError("unidentified add_op operator!\n");
		}
	} else {
		semanticError(" argument of type int required for shift expression\n");
	}
}

ShiftExpression::ShiftExpression(Expression* additionExpression, SymbolTable *st, unsigned ln) :
		Expression(ID, { additionExpression }, st, ln) {
	getAttributes(0);
}

}
