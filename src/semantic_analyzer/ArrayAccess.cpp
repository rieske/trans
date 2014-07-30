#include "ArrayAccess.h"

#include <algorithm>
#include <string>
#include <vector>

#include "../code_generator/quadruple.h"
#include "../code_generator/symbol_table.h"
#include "AbstractSyntaxTreeVisitor.h"

namespace semantic_analyzer {

ArrayAccess::ArrayAccess(std::unique_ptr<Expression> postfixExpression, std::unique_ptr<Expression> subscriptExpression, SymbolTable *st,
		unsigned ln) :
		Expression(st, ln),
		postfixExpression { std::move(postfixExpression) },
		subscriptExpression { std::move(subscriptExpression) } {
	saveExpressionAttributes(*this->postfixExpression);
	if (extended_type.size() && (extended_type.at(0) == 'p' || extended_type.at(0) == 'a')) {
		value = "rval";
		extended_type = extended_type.substr(1, extended_type.size());
		SymbolEntry *rval = s_table->newTemp(basic_type, extended_type);
		lval = s_table->newTemp(basic_type, extended_type);
		SymbolEntry *offset = this->subscriptExpression->getPlace();
		vector<Quadruple *> more_code = this->subscriptExpression->getCode();
		code.insert(code.end(), more_code.begin(), more_code.end());
		code.push_back(new Quadruple(INDEX, place, offset, rval));
		code.push_back(new Quadruple(INDEX_ADDR, place, offset, lval));
		place = rval;
	} else {
		semanticError("invalid type for operator[]\n");
	}
}

ArrayAccess::~ArrayAccess() {
}

void ArrayAccess::accept(const AbstractSyntaxTreeVisitor& visitor) const {
	visitor.visit(*this);
}

} /* namespace semantic_analyzer */
