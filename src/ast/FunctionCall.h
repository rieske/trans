#ifndef FUNCTIONCALL_H_
#define FUNCTIONCALL_H_

#include <memory>
#include <optional>
#include <vector>

#include "ast/SingleOperandExpression.h"
#include "types/Type.h"

namespace ast {

class FunctionCall: public SingleOperandExpression {
public:
    FunctionCall(std::unique_ptr<Expression> callExpression, std::vector<std::unique_ptr<Expression>> argumentList = { });
    virtual ~FunctionCall() = default;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    ExprKind exprKind() const override { return ExprKind::FunctionCall; }
    std::optional<type::Type> typeAtParseTime(const ParseEnvironment& environment) const override;
    bool evaluateConstant(type::IntegerConstant& value) const override;
    void visitArguments(AbstractSyntaxTreeVisitor& visitor);

    const std::vector<std::unique_ptr<Expression>>& getArgumentList() const;

    // Type operand of `__builtin_va_arg(ap, T)` - not an expression argument.
    void setBuiltinTypeArgument(type::Type type);
    const type::Type* builtinTypeArgument() const;

    // Call shape (Direct/Indirect + callee name) is symbols::CallPlan on the store.
    // A call whose evaluateConstant succeeds is a folded ICE, not a CallPlan.

private:
    bool isGnuConstantP() const;

    std::vector<std::unique_ptr<Expression>> argumentList;
    std::optional<type::Type> builtinTypeArgument_;
};

} // namespace ast

#endif // FUNCTIONCALL_H_
