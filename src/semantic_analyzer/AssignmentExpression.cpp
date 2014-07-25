#include "AssignmentExpression.h"

#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <vector>

#include "../code_generator/quadruple.h"
#include "../code_generator/symbol_entry.h"
#include "../code_generator/symbol_table.h"
#include "AbstractSyntaxTreeVisitor.h"

namespace semantic_analyzer {

const std::string AssignmentExpression::ID = "<a_expr>";

AssignmentExpression::AssignmentExpression(std::unique_ptr<Expression> leftHandSide, TerminalSymbol assignmentOperator,
		std::unique_ptr<Expression> rightHandSide, SymbolTable *st) :
		Expression(ID, { }, st, assignmentOperator.line),
		leftHandSide { std::move(leftHandSide) },
		assignmentOperator { assignmentOperator },
		rightHandSide { std::move(rightHandSide) } {
	saveExpressionAttributes(*this->leftHandSide);
	place = this->leftHandSide->getPlace();
	value = this->leftHandSide->getValue();
	bool deref = false;
	for (vector<Quadruple *>::iterator it = code.begin(); it != code.end(); it++) {
		if ((*it)->getOp() == DEREF) {
			deref = true;
		}
	}
	if (value != "lval") {
		if (lval == NULL) {
			semanticError("lvalue required on the left side of assignment\n");
		}
		place = lval;
	}
	SymbolEntry *arg_place = this->rightHandSide->getPlace();
	vector<Quadruple *> a_code = this->rightHandSide->getCode();
	code.insert(code.begin(), a_code.begin(), a_code.end());
	string check = s_table->typeCheck(arg_place, place);
	if (check != "ok") {
		semanticError(check);
	} else {
		if (assignmentOperator.value == "+=")
			code.push_back(new Quadruple(ADD, place, arg_place, place));
		else if (assignmentOperator.value == "-=")
			code.push_back(new Quadruple(SUB, place, arg_place, place));
		else if (assignmentOperator.value == "*=")
			code.push_back(new Quadruple(MUL, place, arg_place, place));
		else if (assignmentOperator.value == "/=")
			code.push_back(new Quadruple(DIV, place, arg_place, place));
		else if (assignmentOperator.value == "%=")
			code.push_back(new Quadruple(MOD, place, arg_place, place));
		else if (assignmentOperator.value == "&=")
			code.push_back(new Quadruple(AND, place, arg_place, place));
		else if (assignmentOperator.value == "^=")
			code.push_back(new Quadruple(XOR, place, arg_place, place));
		else if (assignmentOperator.value == "|=")
			code.push_back(new Quadruple(OR, place, arg_place, place));
		else if (assignmentOperator.value == "<<=")
			code.push_back(new Quadruple(SHL, place, arg_place, place));
		else if (assignmentOperator.value == ">>=")
			code.push_back(new Quadruple(SHR, place, arg_place, place));
		else if (assignmentOperator.value == "=") {
			if (deref) {
				Quadruple *quad = new Quadruple(DEREF_LVAL, arg_place, NULL, code.back()->getArg1());
				code.pop_back();
				code.push_back(quad);
			} else {
				code.push_back(new Quadruple(ASSIGN, arg_place, NULL, place));
			}
		} else {
			throw std::runtime_error { "unidentified assignment operator: " + assignmentOperator.value };
		}
	}
}

void AssignmentExpression::accept(const AbstractSyntaxTreeVisitor& visitor) const {
	visitor.visit(*this);
}

}

