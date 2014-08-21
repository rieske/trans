#include "ArithmeticExpression.h"

#include <algorithm>

#include "AbstractSyntaxTreeVisitor.h"
#include "BasicType.h"

namespace semantic_analyzer {

const std::string ArithmeticExpression::ADDITION { "<add_expr>" };
const std::string ArithmeticExpression::MULTIPLICATION { "<factor>" };

ArithmeticExpression::ArithmeticExpression(std::unique_ptr<Expression> leftHandSide, TerminalSymbol arithmeticOperator,
        std::unique_ptr<Expression> rightHandSide) :
        leftHandSide { std::move(leftHandSide) },
        arithmeticOperator { arithmeticOperator },
        rightHandSide { std::move(rightHandSide) } {
}

}

void semantic_analyzer::ArithmeticExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}
