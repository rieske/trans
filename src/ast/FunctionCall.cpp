#include "FunctionCall.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "ParseEnvironment.h"
#include "types/TypeQuery.h"

namespace ast {

FunctionCall::FunctionCall(std::unique_ptr<Expression> postfixExpression,
        std::vector<std::unique_ptr<Expression>> argumentList) :
        SingleOperandExpression { std::move(postfixExpression), std::unique_ptr<Operator> { new Operator("()") } },
        argumentList { std::move(argumentList) }
{
}

void FunctionCall::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

std::optional<type::Type> FunctionCall::typeAtParseTime(const ParseEnvironment& environment) const {
    if (builtinTypeArgument_) {
        return *builtinTypeArgument_;
    }
    auto callee = _operand->typeAtParseTime(environment);
    if (!callee) {
        return std::nullopt;
    }
    const type::Type decayed = type::afterLvalueConversion(*callee);
    if (!decayed.isPointer()) {
        return std::nullopt;
    }
    const type::Type fn = decayed.dereference();
    if (!fn.isFunction()) {
        return std::nullopt;
    }
    return fn.getFunction().getReturnType();
}

void FunctionCall::visitArguments(AbstractSyntaxTreeVisitor& visitor) {
    for (const auto& argument : argumentList) {
        argument->accept(visitor);
    }
}

const std::vector<std::unique_ptr<Expression>>& FunctionCall::getArgumentList() const {
    return argumentList;
}

void FunctionCall::setBuiltinTypeArgument(type::Type type) {
    builtinTypeArgument_ = std::move(type);
}

const type::Type* FunctionCall::builtinTypeArgument() const {
    return builtinTypeArgument_ ? &*builtinTypeArgument_ : nullptr;
}

} // namespace ast
