#include "PostfixExpression.h"

#include <algorithm>
#include <cassert>

#include "AbstractSyntaxTreeVisitor.h"
#include "Operator.h"

namespace ast {

const std::string PostfixExpression::ID { "<postfix_exp>" };

PostfixExpression::PostfixExpression(std::unique_ptr<Expression> postfixExpression, std::unique_ptr<Operator> postfixOperator) :
        SingleOperandExpression(std::move(postfixExpression), std::move(postfixOperator)) {
}

void PostfixExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

void PostfixExpression::setPreOperationSymbol(semantic_analyzer::ValueEntry resultSymbol) {
    this->preOperationSymbol = std::make_unique<semantic_analyzer::ValueEntry>(resultSymbol);
    setType(this->preOperationSymbol->getType());
}

semantic_analyzer::ValueEntry* PostfixExpression::getPreOperationSymbol() const {
    assert(preOperationSymbol);
    return preOperationSymbol.get();
}

}
