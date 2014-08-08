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
	resultPlace = this->callExpression->getPlace();
	if ( NULL != resultPlace) {
		if (resultPlace->getParamCount() == 0) {
			value = "rval";
			basicType = resultPlace->getBasicType();
			extended_type = resultPlace->getExtendedType();
			code.push_back(new Quadruple(CALL, resultPlace, NULL, NULL));
			if (basicType != BasicType::VOID || extended_type != "") {
				extended_type = extended_type.substr(0, extended_type.size() - 1);
				resultPlace = s_table->newTemp(basicType, extended_type);
				code.push_back(new Quadruple(RETRIEVE, resultPlace, NULL, NULL));
			}
		} else {
			semanticError("no match for function " + resultPlace->getName() + "()\n");
			return;
		}
	} else {
		semanticError("symbol " + value + " is not defined\n");
	}
}

NoArgFunctionCall::~NoArgFunctionCall() {
}

void NoArgFunctionCall::accept(AbstractSyntaxTreeVisitor& visitor) {
	visitor.visit(*this);
}

} /* namespace semantic_analyzer */
