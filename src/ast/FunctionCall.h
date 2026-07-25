#ifndef FUNCTIONCALL_H_
#define FUNCTIONCALL_H_

#include <memory>
#include <vector>

#include "ast/SingleOperandExpression.h"

namespace ast {

class FunctionCall: public SingleOperandExpression {
public:
    FunctionCall(std::unique_ptr<Expression> callExpression, std::vector<std::unique_ptr<Expression>> argumentList = { });
    virtual ~FunctionCall() = default;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    void visitArguments(AbstractSyntaxTreeVisitor& visitor);

    const std::vector<std::unique_ptr<Expression>>& getArgumentList() const;

    // Call shape (Direct/Indirect + callee name) is symbols::CallPlan on the store.

private:
    std::vector<std::unique_ptr<Expression>> argumentList;
};

} // namespace ast

#endif // FUNCTIONCALL_H_
