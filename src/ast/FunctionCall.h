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
    void visitArguments(AbstractSyntaxTreeVisitor& visitor);

    const std::vector<std::unique_ptr<Expression>>& getArgumentList() const;

    // Parse-time type_name argument for builtins like __builtin_va_arg(ap, T).
    // Lives on the call node (not a PE/AST side map).
    void setBuiltinTypeArgument(type::Type type);
    const type::Type* builtinTypeArgument() const;

private:
    std::vector<std::unique_ptr<Expression>> argumentList;
    std::optional<type::Type> builtinTypeArgument_;
};

} // namespace ast

#endif // FUNCTIONCALL_H_
