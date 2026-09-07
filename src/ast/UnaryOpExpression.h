#ifndef UNARYOPEXPRESSION_H_
#define UNARYOPEXPRESSION_H_

#include <memory>
#include <utility>

#include "ast/SingleOperandExpression.h"

namespace ast {

template<typename Op>
class UnaryOpExpression: public SingleOperandExpression {
public:
    UnaryOpExpression(std::unique_ptr<Expression> operand, Op op) :
            SingleOperandExpression(std::move(operand)),
            op_ { op } {
    }

    Op op() const {
        return op_;
    }

private:
    Op op_;
};

} // namespace ast

#endif // UNARYOPEXPRESSION_H_
