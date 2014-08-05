#include "BitwiseExpression.h"

#include <algorithm>
#include <stdexcept>
#include <vector>

#include "../code_generator/quadruple.h"
#include "../code_generator/symbol_table.h"
#include "AbstractSyntaxTreeVisitor.h"

namespace semantic_analyzer {

const std::string BitwiseExpression::AND { "<and_expr>" };
const std::string BitwiseExpression::OR { "<or_expr>" };
const std::string BitwiseExpression::XOR { "<xor_expr>" };

BitwiseExpression::BitwiseExpression(std::unique_ptr<Expression> leftHandSide, TerminalSymbol bitwiseOperator,
		std::unique_ptr<Expression> rightHandSide, SymbolTable *st) :
		Expression(st, bitwiseOperator.line),
		leftHandSide { std::move(leftHandSide) },
		bitwiseOperator { bitwiseOperator },
		rightHandSide { std::move(rightHandSide) } {
	code = this->leftHandSide->getCode();
	value = "rval";
	basicType = this->leftHandSide->getBasicType();
	extended_type = this->leftHandSide->getExtendedType();
	SymbolEntry *arg1 = this->leftHandSide->getPlace();
	SymbolEntry *arg2 = this->rightHandSide->getPlace();
	string check = s_table->typeCheck(arg1, arg2);
	if (check != "ok") {
		semanticError(check);
	} else {
		oper quadrupleOperator;
		switch (bitwiseOperator.type.at(0)) {
		case '&':
			quadrupleOperator = oper::AND;
			break;
		case '|':
			quadrupleOperator = oper::OR;
			break;
		case '^':
			quadrupleOperator = oper::XOR;
			break;
		default:
			throw std::runtime_error { "no semantic actions defined for bitwise operator: " + bitwiseOperator.type };
		}
		vector<Quadruple *> arg2code = this->rightHandSide->getCode();
		code.insert(code.end(), arg2code.begin(), arg2code.end());
		place = s_table->newTemp(basicType, extended_type);
		code.push_back(new Quadruple(quadrupleOperator, arg1, arg2, place));
	}
}

void BitwiseExpression::accept(const AbstractSyntaxTreeVisitor& visitor) const {
	visitor.visit(*this);
}

}
