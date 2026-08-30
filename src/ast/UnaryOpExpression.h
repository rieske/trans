#ifndef UNARYOPEXPRESSION_H_
#define UNARYOPEXPRESSION_H_

#include <memory>
#include <string>
#include <utility>

#include "ast/SingleOperandExpression.h"

namespace ast {

class UnaryOpExpression: public SingleOperandExpression {
public:
    UnaryOpExpression(std::unique_ptr<Expression> operand, std::string lexeme) :
            SingleOperandExpression(std::move(operand)),
            lexeme_ { std::move(lexeme) } {
    }

    const std::string& lexeme() const {
        return lexeme_;
    }

private:
    std::string lexeme_;
};

} // namespace ast

#endif // UNARYOPEXPRESSION_H_
