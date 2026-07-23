#include "ArithmeticExpression.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "Operator.h"

namespace ast {

ArithmeticExpression::ArithmeticExpression(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Operator> arithmeticOperator,
        std::unique_ptr<Expression> rightHandSide) :
        DoubleOperandExpression(std::move(leftHandSide), std::move(rightHandSide), std::move(arithmeticOperator)) {
}

void ArithmeticExpression::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

void ArithmeticExpression::setPointerArithmetic(int scale, std::string scaleTemp, bool pointerIsLeft) {
    pointerScale = scale;
    scaleTempName = std::move(scaleTemp);
    pointerOnLeft = pointerIsLeft;
    pointerDifference = false;
}

void ArithmeticExpression::setPointerDifference(int scale, std::string scaleTemp) {
    pointerScale = scale;
    scaleTempName = std::move(scaleTemp);
    pointerDifference = true;
}

int ArithmeticExpression::getPointerScale() const {
    return pointerScale;
}

const std::string* ArithmeticExpression::getScaleTempName() const {
    return scaleTempName ? &*scaleTempName : nullptr;
}

bool ArithmeticExpression::pointerIsLeftOperand() const {
    return pointerOnLeft;
}

bool ArithmeticExpression::isPointerDifference() const {
    return pointerDifference;
}

} // namespace ast
