#include "PostfixExpression.h"

#include <iostream>
#include <vector>

#include "../code_generator/quadruple.h"
#include "../code_generator/symbol_entry.h"
#include "../code_generator/symbol_table.h"
#include "AssignmentExpression.h"
#include "AssignmentExpressionList.h"
#include "TerminalSymbol.h"

namespace semantic_analyzer {

const std::string PostfixExpression::ID { "<postfix_expr>" };

PostfixExpression::PostfixExpression(Expression* postfixExpression, Expression* expression, SymbolTable *st, unsigned ln) :
		Expression(ID, { postfixExpression, expression }, st, ln) {
	saveExpressionAttributes(*postfixExpression);
	if (extended_type.size() && (extended_type.at(0) == 'p' || extended_type.at(0) == 'a')) {
		value = "rval";
		extended_type = extended_type.substr(1, extended_type.size());
		SymbolEntry *rval = s_table->newTemp(basic_type, extended_type);
		lval = s_table->newTemp(basic_type, extended_type);
		SymbolEntry *offset = expression->getPlace();
		vector<Quadruple *> more_code = expression->getCode();
		code.insert(code.end(), more_code.begin(), more_code.end());
		code.push_back(new Quadruple(INDEX, place, offset, rval));
		code.push_back(new Quadruple(INDEX_ADDR, place, offset, lval));
		place = rval;
	} else {
		semanticError("invalid type for operator[]\n");
	}
}

PostfixExpression::PostfixExpression(Expression* postfixExpression, AssignmentExpressionList* assignmentExpressionList, SymbolTable *st,
		unsigned ln) :
		Expression(ID, { postfixExpression, assignmentExpressionList }, st, ln) {
	place = postfixExpression->getPlace();
	if ( NULL != place) {
		value = "rval";
		basic_type = place->getBasicType();
		extended_type = place->getExtendedType();
		vector<AssignmentExpression *> exprs = assignmentExpressionList->getAssignmentExpressions();
		if (exprs.size() != place->getParamCount()) {
			semanticError("no match for function " + basic_type);
			if (extended_type.size()) {
				for (unsigned i = 0; i < extended_type.size() - 1; i++)
					std::cerr << extended_type.at(i);
			}
			std::cerr << " " << place->getName() << "(";
			if (exprs.size()) {
				for (unsigned i = 0; i < exprs.size() - 1; i++)
					std::cerr << exprs.at(i)->getBasicType() << exprs.at(i)->getExtendedType() << ", ";
				std::cerr << exprs.at(exprs.size() - 1)->getBasicType() << exprs.at(exprs.size() - 1)->getExtendedType();
			}
			std::cerr << ")\n";
			return;
		} else {
			vector<SymbolEntry *> params = place->getParams();
			code = assignmentExpressionList->getCode();
			for (unsigned i = 0; i < exprs.size(); i++) {
				SymbolEntry *param = exprs[i]->getPlace();
				string check;
				if ("ok" != (check = s_table->typeCheck(params[i], param))) {
					semanticError(check);
				}
				code.push_back(new Quadruple(PARAM, param, NULL, NULL));
			}
		}
		code.push_back(new Quadruple(CALL, place, NULL, NULL));
		if (basic_type != "void" || extended_type != "") {
			extended_type = extended_type.substr(0, extended_type.size() - 1);
			place = s_table->newTemp(basic_type, extended_type);
			code.push_back(new Quadruple(RETRIEVE, place, NULL, NULL));
		}
	} else {
		semanticError("symbol " + value + " is not defined");
	}
}

PostfixExpression::PostfixExpression(Expression* postfixExpression, SymbolTable *st, unsigned ln) :
		Expression(ID, { postfixExpression }, st, ln) {
	place = postfixExpression->getPlace();
	if ( NULL != place) {
		if (place->getParamCount() == 0) {
			value = "rval";
			basic_type = place->getBasicType();
			extended_type = place->getExtendedType();
			code.push_back(new Quadruple(CALL, place, NULL, NULL));
			if (basic_type != "void" || extended_type != "") {
				extended_type = extended_type.substr(0, extended_type.size() - 1);
				place = s_table->newTemp(basic_type, extended_type);
				code.push_back(new Quadruple(RETRIEVE, place, NULL, NULL));
			}
		} else {
			semanticError("no match for function " + basic_type + " " + place->getName() + "()\n");
			return;
		}
	} else {
		semanticError("symbol " + value + " is not defined\n");
	}
}

PostfixExpression::PostfixExpression(Expression* postfixExpression, TerminalSymbol postfixOperator, SymbolTable *st, unsigned ln) :
		Expression(ID, { postfixExpression }, st, ln) {
	saveExpressionAttributes(*postfixExpression);
	if (place == NULL || value == "rval") {   // dirbama su konstanta
		semanticError("lvalue required as increment operand\n");
	} else {   // dirbama su kinmamuoju simbolių lentelėje
		if (postfixOperator.value == "++") {
			code.push_back(new Quadruple(INC, place, NULL, place));
		} else if (postfixOperator.value == "--") {
			code.push_back(new Quadruple(DEC, place, NULL, place));
		}
		value = "rval";
	}
}

}
