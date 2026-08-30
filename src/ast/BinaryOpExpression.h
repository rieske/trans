#ifndef BINARYOPEXPRESSION_H_
#define BINARYOPEXPRESSION_H_

#include <memory>
#include <string>
#include <utility>

#include "ast/DoubleOperandExpression.h"

namespace ast {

class BinaryOpExpression: public DoubleOperandExpression {
public:
    BinaryOpExpression(std::unique_ptr<Expression> leftOperand, std::string lexeme,
            std::unique_ptr<Expression> rightOperand) :
            DoubleOperandExpression(std::move(leftOperand), std::move(rightOperand)),
            lexeme_ { std::move(lexeme) } {
    }

    const std::string& lexeme() const {
        return lexeme_;
    }

    bool evaluateConstant(type::IntegerConstant& value) const override {
        return foldOperands(value, lexeme_);
    }

private:
    std::string lexeme_;
};

} // namespace ast

#endif // BINARYOPEXPRESSION_H_
