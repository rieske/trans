#include "FunctionCall.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "ConstantExpression.h"
#include "IdentifierExpression.h"
#include "ParseEnvironment.h"
#include "StringLiteralExpression.h"
#include "TypeCast.h"
#include "types/IntegerConstant.h"
#include "types/TypeQuery.h"

namespace ast {

namespace {

bool isConstantPOperand(const Expression& expr) {
    type::IntegerConstant unused;
    if (expr.evaluateConstant(unused)) {
        return true;
    }
    if (dynamic_cast<const StringLiteralExpression*>(&expr)
            || dynamic_cast<const ConstantExpression*>(&expr)) {
        return true;
    }
    if (const auto* cast = dynamic_cast<const TypeCast*>(&expr)) {
        return cast->getOperandExpression() && isConstantPOperand(*cast->getOperandExpression());
    }
    return false;
}

} // namespace

FunctionCall::FunctionCall(std::unique_ptr<Expression> callExpression,
        std::vector<std::unique_ptr<Expression>> argumentList)
    : SingleOperandExpression(std::move(callExpression), nullptr),
      argumentList{std::move(argumentList)} {}

void FunctionCall::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

bool FunctionCall::isGnuConstantP() const {
    auto* id = dynamic_cast<const IdentifierExpression*>(_operand.get());
    return id && id->getIdentifier() == "__builtin_constant_p";
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
    if (builtinTypeName_) {
        if (auto builtin = builtinTypeName_->tryResolve(environment)) {
            return builtin;
        }
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
    for (auto& arg : argumentList) {
        arg->accept(visitor);
    }
}

const std::vector<std::unique_ptr<Expression>>& FunctionCall::getArgumentList() const {
    return argumentList;
}

void FunctionCall::setBuiltinTypeName(TypeName name) {
    builtinTypeName_ = std::move(name);
}

TypeName* FunctionCall::builtinTypeName() {
    return builtinTypeName_ ? &*builtinTypeName_ : nullptr;
}

const TypeName* FunctionCall::builtinTypeName() const {
    return builtinTypeName_ ? &*builtinTypeName_ : nullptr;
}

} // namespace ast
