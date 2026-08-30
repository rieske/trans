#ifndef _BITWISE_EXPRESSION_NODE_H_
#define _BITWISE_EXPRESSION_NODE_H_

#include <memory>
#include <string>

#include "BinaryOpExpression.h"

namespace ast {

class BitwiseExpression: public BinaryOpExpression {
public:
    BitwiseExpression(std::unique_ptr<Expression> leftHandSide, std::string lexeme,
            std::unique_ptr<Expression> rightHandSide);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    std::optional<type::Type> typeAtParseTime(const ParseEnvironment& environment) const override;
    bool evaluateConstant(type::IntegerConstant& value) const override {
        return foldOperands(value, lexeme());
    }
};

} // namespace ast

#endif // _BITWISE_EXPRESSION_NODE_H_
