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
        std::unique_ptr<AssignmentExpressionList> argumentList, SymbolTable *st, unsigned ln) :
        Expression(st, ln),
        callExpression { std::move(postfixExpression) },
        argumentList { std::move(argumentList) } {
    resultPlace = this->callExpression->getPlace();
    value = "rval";
    basicType = resultPlace->getBasicType();
    extended_type = resultPlace->getExtendedType();
    auto& exprs = this->argumentList->getExpressions();
    if (exprs.size() != resultPlace->getParamCount()) {
        semanticError("no match for function " + resultPlace->getName());
        return;
    } else {
        vector<SymbolEntry *> params = resultPlace->getParams();
        code = this->argumentList->getCode();
        for (unsigned i = 0; i < exprs.size(); i++) {
            SymbolEntry *param = exprs[i]->getPlace();
            string check;
            if ("ok" != (check = s_table->typeCheck(params[i], param))) {
                semanticError(check);
            }
            code.push_back(new Quadruple(PARAM, param, NULL, NULL));
        }
    }
    code.push_back(new Quadruple(CALL, resultPlace, NULL, NULL));
    if (basicType != BasicType::VOID || extended_type != "") {
        extended_type = extended_type.substr(0, extended_type.size() - 1);
        resultPlace = s_table->newTemp(basicType, extended_type);
        code.push_back(new Quadruple(RETRIEVE, resultPlace, NULL, NULL));
    }
}

FunctionCall::~FunctionCall() {
}

void FunctionCall::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

} /* namespace semantic_analyzer */
