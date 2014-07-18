#include "ComparisonExpression.h"

#include <string>
#include <vector>

#include "../code_generator/quadruple.h"
#include "../code_generator/symbol_table.h"
#include "TerminalSymbol.h"

namespace semantic_analyzer {

const std::string ComparisonExpression::ID { "<ml_expr>" };

ComparisonExpression::ComparisonExpression(Expression* comparisonExpression, TerminalSymbol comparisonOperator, Expression* shiftExpression, SymbolTable *st,
		unsigned ln) :
		Expression(ID, { comparisonExpression, shiftExpression }, st, ln) {
	code = comparisonExpression->getCode();
	value = "rval";
	basic_type = "int";
	SymbolEntry *arg1 = comparisonExpression->getPlace();
	SymbolEntry *arg2 = shiftExpression->getPlace();
	string check = s_table->typeCheck(arg1, arg2);
	if (check != "ok") {
		semanticError(check);
	} else {
		vector<Quadruple *> arg2code = shiftExpression->getCode();
		code.insert(code.end(), arg2code.begin(), arg2code.end());
		code.push_back(new Quadruple(CMP, arg1, arg2, NULL));
		place = s_table->newTemp("int", "");
		SymbolEntry *true_label = s_table->newLabel();
		SymbolEntry *exit_label = s_table->newLabel();
		if (comparisonOperator.value == ">") {
			code.push_back(new Quadruple(JA, true_label, NULL, NULL));
		} else if (comparisonOperator.value == "<") {
			code.push_back(new Quadruple(JB, true_label, NULL, NULL));
		} else if (comparisonOperator.value == "<=") {
			code.push_back(new Quadruple(JBE, true_label, NULL, NULL));
		} else if (comparisonOperator.value == ">=") {
			code.push_back(new Quadruple(JAE, true_label, NULL, NULL));
		} else {
			semanticError("unidentified ml_op operator!\n");
		}
		code.push_back(new Quadruple("0", place));
		code.push_back(new Quadruple(GOTO, exit_label, NULL, NULL));
		code.push_back(new Quadruple(LABEL, true_label, NULL, NULL));
		code.push_back(new Quadruple("1", place));
		code.push_back(new Quadruple(LABEL, exit_label, NULL, NULL));
	}
}

}
