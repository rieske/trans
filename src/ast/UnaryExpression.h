#ifndef _U_EXPR_NODE_H_
#define _U_EXPR_NODE_H_

#include <memory>
#include <optional>

#include "ast/SingleOperandExpression.h"

namespace ast {

class UnaryExpression: public SingleOperandExpression {
public:
    UnaryExpression(std::unique_ptr<Operator> unaryOperator, std::unique_ptr<Expression> castExpression);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    bool isLval() const override;
    bool evaluateConstant(long& value) const override;

    // Folded integer after SA: sizeof result bytes, or offsetof (&((T*)0)->m).
    void setFoldedInteger(long value);
    bool hasFoldedInteger() const;
    long foldedInteger() const;
    bool isSizeof() const;

private:
    std::optional<long> foldedInteger_;
};

} // namespace ast

#endif // _U_EXPR_NODE_H_
