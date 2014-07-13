#include "EqualityExpression.h"

#include <cstdlib>
#include <vector>

#include "../code_generator/quadruple.h"
#include "../code_generator/symbol_table.h"
#include "ComparisonExpression.h"

namespace semantic_analyzer {

const std::string EqualityExpression::ID { "<eq_expr>" };

EqualityExpression::EqualityExpression(EqualityExpression* equalityExpression, ParseTreeNode* equalityOperator,
		ComparisonExpression* comparisonExpression, SymbolTable *st, unsigned ln) :
		Expression(ID, { equalityExpression, equalityOperator, comparisonExpression }, st, ln) {
	code = equalityExpression->getCode();
	value = "rval";
	basic_type = "int";
	SymbolEntry *arg1 = equalityExpression->getPlace();
	SymbolEntry *arg2 = comparisonExpression->getPlace();
	string check = s_table->typeCheck(arg1, arg2);
	if (check != "ok") {
		semanticError(check);
	} else {
		vector<Quadruple *> arg2code = comparisonExpression->getCode();
		code.insert(code.end(), arg2code.begin(), arg2code.end());
		code.push_back(new Quadruple(CMP, arg1, arg2, NULL));
		string op = equalityOperator->getAttr();
		place = s_table->newTemp("int", "");
		SymbolEntry *true_label = s_table->newLabel();
		SymbolEntry *exit_label = s_table->newLabel();
		if (op == "==") {
			code.push_back(new Quadruple(JE, true_label, NULL, NULL));
		} else if (op == "!=") {
			code.push_back(new Quadruple(JNE, true_label, NULL, NULL));
		} else {
			semanticError("unidentified eq_op operator!\n");
			exit(1);
		}
		code.push_back(new Quadruple("0", place));
		code.push_back(new Quadruple(GOTO, exit_label, NULL, NULL));
		code.push_back(new Quadruple(LABEL, true_label, NULL, NULL));
		code.push_back(new Quadruple("1", place));
		code.push_back(new Quadruple(LABEL, exit_label, NULL, NULL));
	}
}

EqualityExpression::EqualityExpression(ComparisonExpression* comparisonExpression, SymbolTable *st, unsigned ln) :
		Expression(ID, { comparisonExpression }, st, ln) {
	getAttributes(0);
}

}
