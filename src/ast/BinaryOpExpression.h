#ifndef BINARYOPEXPRESSION_H_
#define BINARYOPEXPRESSION_H_

#include <memory>
#include <utility>

#include "ast/DoubleOperandExpression.h"

namespace ast {

template<typename Op>
class BinaryOpExpression: public DoubleOperandExpression {
public:
    BinaryOpExpression(std::unique_ptr<Expression> leftOperand, Op op,
            std::unique_ptr<Expression> rightOperand) :
            DoubleOperandExpression(std::move(leftOperand), std::move(rightOperand)),
            op_ { op } {
    }

    Op op() const {
        return op_;
    }

private:
    Op op_;
};

} // namespace ast

#endif // BINARYOPEXPRESSION_H_
