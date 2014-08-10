#include "ArrayAccess.h"

#include <algorithm>
#include <string>
#include <vector>

#include "../code_generator/quadruple.h"
#include "../code_generator/symbol_table.h"
#include "AbstractSyntaxTreeVisitor.h"
#include "BasicType.h"

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
		SymbolEntry *rval = s_table->newTemp(basicType, extended_type);
		lval = s_table->newTemp(basicType, extended_type);
		SymbolEntry *offset = this->subscriptExpression->getPlace();
		vector<Quadruple *> more_code = this->subscriptExpression->getCode();
		code.insert(code.end(), more_code.begin(), more_code.end());
		code.push_back(new Quadruple(INDEX, resultPlace, offset, rval));
		code.push_back(new Quadruple(INDEX_ADDR, resultPlace, offset, lval));
		resultPlace = rval;
	} else {
		semanticError("invalid type for operator[]\n");
	}
}

ArrayAccess::~ArrayAccess() {
}

void ArrayAccess::accept(AbstractSyntaxTreeVisitor& visitor) {
	visitor.visit(*this);
}

void ArrayAccess::setLvalue(SymbolEntry* lvalue) {
    this->lvalue = lvalue;
}

} /* namespace semantic_analyzer */
