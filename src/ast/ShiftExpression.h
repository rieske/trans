#ifndef _S_EXPR_NODE_H_
#define _S_EXPR_NODE_H_

#include <memory>
#include <string>

#include "ast/BinaryOpExpression.h"

namespace ast {

class ShiftExpression: public BinaryOpExpression {
public:
    ShiftExpression(std::unique_ptr<Expression> shiftExpression, std::string lexeme,
            std::unique_ptr<Expression> additionExpression);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    std::optional<type::Type> typeAtParseTime(const ParseEnvironment& environment) const override;
};

} // namespace ast

#endif // _S_EXPR_NODE_H_
