#include "FunctionCall.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

#include "../code_generator/quadruple.h"
#include "../code_generator/symbol_entry.h"
#include "../code_generator/symbol_table.h"
#include "AbstractSyntaxTreeVisitor.h"
#include "AssignmentExpression.h"
#include "AssignmentExpressionList.h"

namespace semantic_analyzer {

FunctionCall::FunctionCall(std::unique_ptr<Expression> postfixExpression,
		std::unique_ptr<AssignmentExpressionList> assignmentExpressionList, SymbolTable *st, unsigned ln) :
		Expression(st, ln),
		postfixExpression { std::move(postfixExpression) },
		assignmentExpressionList { std::move(assignmentExpressionList) } {
	place = this->postfixExpression->getPlace();
	if ( NULL != place) {
		value = "rval";
		basic_type = place->getBasicType();
		extended_type = place->getExtendedType();
		auto& exprs = this->assignmentExpressionList->getExpressions();
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
			code = this->assignmentExpressionList->getCode();
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

FunctionCall::~FunctionCall() {
}

void FunctionCall::accept(const AbstractSyntaxTreeVisitor& visitor) const {
	visitor.visit(*this);
}

} /* namespace semantic_analyzer */
