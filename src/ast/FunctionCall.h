#ifndef FUNCTIONCALL_H_
#define FUNCTIONCALL_H_

#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "ast/SingleOperandExpression.h"
#include "symbols/FunctionEntry.h"
#include "types/Type.h"

namespace ast {

class FunctionCall: public SingleOperandExpression {
public:
    enum class BuiltinKind {
        None,
        ConstantP,
        AllocaAsMalloc,
        Bswap16,
        Bswap32,
        Bswap64,
        Ctz,
        VaStart,
        VaEnd,
        VaCopy,
        VaArg
    };

    FunctionCall(std::unique_ptr<Expression> callExpression, std::vector<std::unique_ptr<Expression>> argumentList = { });
    virtual ~FunctionCall() = default;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    void visitArguments(AbstractSyntaxTreeVisitor& visitor);

    const std::vector<std::unique_ptr<Expression>>& getArgumentList() const;

    void setSymbol(symbols::FunctionEntry symbol);
    symbols::FunctionEntry* getSymbol() const;

    void setBuiltinKind(BuiltinKind kind);
    BuiltinKind getBuiltinKind() const;

    void setVaArgResultType(type::Type type);
    const type::Type* getVaArgResultType() const;

    void setIndirect(bool indirect);
    bool isIndirect() const;

private:
    std::vector<std::unique_ptr<Expression>> argumentList;
    BuiltinKind builtinKind { BuiltinKind::None };
    std::optional<type::Type> vaArgResultType;
    bool indirectCall { false };
};

} // namespace ast

#endif // FUNCTIONCALL_H_
