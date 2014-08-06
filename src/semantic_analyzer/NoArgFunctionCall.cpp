#include "NoArgFunctionCall.h"

#include <algorithm>
#include <string>
#include <vector>

#include "../code_generator/quadruple.h"
#include "../code_generator/symbol_entry.h"
#include "../code_generator/symbol_table.h"
#include "AbstractSyntaxTreeVisitor.h"

namespace semantic_analyzer {

NoArgFunctionCall::NoArgFunctionCall(std::unique_ptr<Expression> callExpression, SymbolTable *st, unsigned ln) :
		Expression(st, ln),
		callExpression { std::move(callExpression) } {
	place = this->callExpression->getPlace();
	if ( NULL != place) {
		if (place->getParamCount() == 0) {
			value = "rval";
			basicType = place->getBasicType();
			extended_type = place->getExtendedType();
			code.push_back(new Quadruple(CALL, place, NULL, NULL));
			if (basicType != BasicType::VOID || extended_type != "") {
				extended_type = extended_type.substr(0, extended_type.size() - 1);
				place = s_table->newTemp(basicType, extended_type);
				code.push_back(new Quadruple(RETRIEVE, place, NULL, NULL));
			}
		} else {
			semanticError("no match for function " + place->getName() + "()\n");
			return;
		}
	} else {
		semanticError("symbol " + value + " is not defined\n");
	}
}

NoArgFunctionCall::~NoArgFunctionCall() {
}

void NoArgFunctionCall::accept(AbstractSyntaxTreeVisitor& visitor) const {
	visitor.visit(*this);
}

} /* namespace semantic_analyzer */
