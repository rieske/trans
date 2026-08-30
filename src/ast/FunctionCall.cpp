#include "FunctionCall.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "ConstantExpression.h"
#include "GnuBuiltinFunctions.h"
#include "IdentifierExpression.h"
#include "ParseEnvironment.h"
#include "StringLiteralExpression.h"
#include "types/IntegerConstant.h"
#include "types/TypeQuery.h"

namespace ast {

namespace {

bool isConstantPOperand(const Expression& expr) {
    type::IntegerConstant unused;
    if (expr.evaluateConstant(unused)) {
        return true;
    }
    return dynamic_cast<const StringLiteralExpression*>(&expr) != nullptr
            || dynamic_cast<const ConstantExpression*>(&expr) != nullptr;
}

} // namespace

FunctionCall::FunctionCall(std::unique_ptr<Expression> postfixExpression,
        std::vector<std::unique_ptr<Expression>> argumentList) :
        SingleOperandExpression { std::move(postfixExpression) },
        argumentList { std::move(argumentList) }
{
}

void FunctionCall::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

bool FunctionCall::isGnuConstantP() const {
    auto* id = dynamic_cast<const IdentifierExpression*>(_operand.get());
    return id && isGnuConstantPBuiltin(id->getIdentifier());
}

bool FunctionCall::evaluateConstant(type::IntegerConstant& value) const {
    if (!isGnuConstantP() || argumentList.size() != 1) {
        return false;
    }
    value = type::fromLiteralBits(isConstantPOperand(*argumentList[0]) ? 1 : 0,
            type::signedInteger());
    return true;
}

std::optional<type::Type> FunctionCall::typeAtParseTime(const ParseEnvironment& environment) const {
    if (isGnuConstantP()) {
        return type::signedInteger();
    }
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
