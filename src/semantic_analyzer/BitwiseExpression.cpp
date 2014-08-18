#include "BitwiseExpression.h"

#include <algorithm>

#include "AbstractSyntaxTreeVisitor.h"
#include "BasicType.h"

namespace semantic_analyzer {

const std::string BitwiseExpression::AND { "<and_expr>" };
const std::string BitwiseExpression::OR { "<or_expr>" };
const std::string BitwiseExpression::XOR { "<xor_expr>" };

BitwiseExpression::BitwiseExpression(std::unique_ptr<Expression> leftHandSide, TerminalSymbol bitwiseOperator,
        std::unique_ptr<Expression> rightHandSide) :
        leftHandSide { std::move(leftHandSide) },
        bitwiseOperator { bitwiseOperator },
        rightHandSide { std::move(rightHandSide) } {
    basicType = this->leftHandSide->getBasicType();
    extended_type = this->leftHandSide->getExtendedType();
}

void BitwiseExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

}
