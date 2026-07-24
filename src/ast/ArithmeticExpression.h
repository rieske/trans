#ifndef _ADD_EXPR_NODE_H_
#define _ADD_EXPR_NODE_H_

#include <memory>
#include <optional>
#include <string>

#include "DoubleOperandExpression.h"

namespace ast {

class ArithmeticExpression: public DoubleOperandExpression {
public:
    ArithmeticExpression(std::unique_ptr<Expression> leftHandSide, std::unique_ptr<Operator> arithmeticOperator, std::unique_ptr<Expression> rightHandSide);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    // Pointer +/- integer: scale the integer by sizeof(pointee) (C 6.5.6).
    // scaleTemp holds (integer * scale) before Add/Sub into the result.
    void setPointerArithmetic(int scale, std::string scaleTempName, bool pointerIsLeft);
    // Pointer - pointer: (p - q) / sizeof(pointee).
    void setPointerDifference(int scale, std::string scaleTempName);
    int getPointerScale() const;
    const std::string* getScaleTempName() const;
    bool pointerIsLeftOperand() const;
    bool isPointerDifference() const;

private:
    int pointerScale { 0 };
    std::optional<std::string> scaleTempName;
    bool pointerOnLeft { true };
    bool pointerDifference { false };
};

} // namespace ast

#endif // _ADD_EXPR_NODE_H_
