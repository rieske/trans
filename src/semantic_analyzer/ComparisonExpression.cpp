#include "ComparisonExpression.h"

#include <algorithm>
#include <vector>

#include "../code_generator/quadruple.h"
#include "../code_generator/symbol_table.h"
#include "AbstractSyntaxTreeVisitor.h"

namespace semantic_analyzer {

const std::string ComparisonExpression::DIFFERENCE { "<ml_expr>" };
const std::string ComparisonExpression::EQUALITY { "<eq_expr>" };

ComparisonExpression::ComparisonExpression(std::unique_ptr<Expression> leftHandSide, TerminalSymbol comparisonOperator,
		std::unique_ptr<Expression> rightHandSide, SymbolTable *st) :
		Expression("comparisonExpression", { }, st, comparisonOperator.line),
		leftHandSide { std::move(leftHandSide) },
		comparisonOperator { comparisonOperator },
		rightHandSide { std::move(rightHandSide) } {
	code = this->leftHandSide->getCode();
	value = "rval";
	basic_type = "int";
	SymbolEntry *arg1 = this->leftHandSide->getPlace();
	SymbolEntry *arg2 = this->rightHandSide->getPlace();
	string check = s_table->typeCheck(arg1, arg2);
	if (check != "ok") {
		semanticError(check);
	} else {
		vector<Quadruple *> arg2code = this->rightHandSide->getCode();
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
		} else if (comparisonOperator.value == "==") {
			code.push_back(new Quadruple(JE, true_label, NULL, NULL));
		} else if (comparisonOperator.value == "!=") {
			code.push_back(new Quadruple(JNE, true_label, NULL, NULL));
		} else {
			throw std::runtime_error { "unidentified ml_op operator!\n" };
		}
		code.push_back(new Quadruple("0", place));
		code.push_back(new Quadruple(GOTO, exit_label, NULL, NULL));
		code.push_back(new Quadruple(LABEL, true_label, NULL, NULL));
		code.push_back(new Quadruple("1", place));
		code.push_back(new Quadruple(LABEL, exit_label, NULL, NULL));
	}
}

void ComparisonExpression::accept(const AbstractSyntaxTreeVisitor& visitor) const {
	visitor.visit(*this);
}

}

