#ifndef FUNCTIONCALL_H_
#define FUNCTIONCALL_H_

#include <memory>
#include <optional>
#include <vector>

#include "ast/SingleOperandExpression.h"
#include "ast/TypeName.h"

namespace ast {

class FunctionCall: public SingleOperandExpression {
public:
    FunctionCall(std::unique_ptr<Expression> callExpression, std::vector<std::unique_ptr<Expression>> argumentList = { });
    virtual ~FunctionCall() = default;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    std::optional<type::Type> typeAtParseTime(const ParseEnvironment& environment) const override;
    bool evaluateConstant(type::IntegerConstant& value) const override;
    bool isGnuConstantP() const;
    void visitArguments(AbstractSyntaxTreeVisitor& visitor);

    const std::vector<std::unique_ptr<Expression>>& getArgumentList() const;

    // type_name for __builtin_va_arg (not an expression argument).
    void setBuiltinTypeName(TypeName name);
    TypeName* builtinTypeName();
    const TypeName* builtinTypeName() const;

private:
    std::vector<std::unique_ptr<Expression>> argumentList;
    std::optional<TypeName> builtinTypeName_;
};

} // namespace ast

#endif // FUNCTIONCALL_H_
