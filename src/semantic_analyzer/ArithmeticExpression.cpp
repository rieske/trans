#include "ArithmeticExpression.h"

#include <algorithm>
#include <vector>

#include "../code_generator/quadruple.h"
#include "../code_generator/symbol_table.h"
#include "AbstractSyntaxTreeVisitor.h"

namespace semantic_analyzer {

const std::string ArithmeticExpression::ADDITION { "<add_expr>" };
const std::string ArithmeticExpression::MULTIPLICATION { "<factor>" };

ArithmeticExpression::ArithmeticExpression(std::unique_ptr<Expression> leftHandSide, TerminalSymbol arithmeticOperator,
		std::unique_ptr<Expression> rightHandSide, SymbolTable *st) :
		Expression("arithmeticExpression", { }, st, arithmeticOperator.line),
		leftHandSide { std::move(leftHandSide) },
		arithmeticOperator { arithmeticOperator },
		rightHandSide { std::move(rightHandSide) } {
	code = this->leftHandSide->getCode();
	value = "rval";
	basic_type = this->leftHandSide->getBasicType();
	extended_type = this->leftHandSide->getExtendedType();
	SymbolEntry *arg1 = this->leftHandSide->getPlace();
	SymbolEntry *arg2 = this->rightHandSide->getPlace();
	string check = s_table->typeCheck(arg1, arg2);
	if (check != "ok") {
		semanticError(check);
	} else {
		char op = this->arithmeticOperator.value.at(0);
		vector<Quadruple *> arg2code = this->rightHandSide->getCode();
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
			throw std::runtime_error { "unidentified arithmetic operator: " + arithmeticOperator.type };
		}
	}
}

}

void semantic_analyzer::ArithmeticExpression::accept(const AbstractSyntaxTreeVisitor& visitor) const {
	visitor.visit(*this);
}
