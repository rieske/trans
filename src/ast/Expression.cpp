#include "Expression.h"

#include "ConstantExpression.h"
#include "GenericSelection.h"
#include "IdentifierExpression.h"
#include "InitializerListExpression.h"
#include "StringLiteralExpression.h"
#include "UnaryExpression.h"

#include <cassert>
#include <stdexcept>

namespace ast {

IdentifierExpression* Expression::asIdentifier() {
    return exprKind() == ExprKind::Identifier
            ? static_cast<IdentifierExpression*>(this) : nullptr;
}

const ConstantExpression* Expression::asConstant() const {
    return exprKind() == ExprKind::Constant
            ? static_cast<const ConstantExpression*>(this) : nullptr;
}

const InitializerListExpression* Expression::asInitList() const {
    return exprKind() == ExprKind::InitList
            ? static_cast<const InitializerListExpression*>(this) : nullptr;
}

InitializerListExpression* Expression::asInitList() {
    return const_cast<InitializerListExpression*>(
            static_cast<const Expression*>(this)->asInitList());
}

const StringLiteralExpression* Expression::asStringLiteral() const {
    return exprKind() == ExprKind::StringLiteral
            ? static_cast<const StringLiteralExpression*>(this) : nullptr;
}

StringLiteralExpression* Expression::asStringLiteral() {
    return const_cast<StringLiteralExpression*>(
            static_cast<const Expression*>(this)->asStringLiteral());
}

void Expression::setType(const type::Type& type) {
    this->type = type;
}

type::Type Expression::expressionType() const {
    if (!type) {
        throw std::runtime_error { "expression type is not set" };
    }
    return *type;
}

type::Type Expression::valueType(const symbols::AnnotationStore& store) const {
    if (const auto* r = store.value(this, symbols::ValueSlot::Result)) {
        return r->getType();
    }
    return expressionType();
}

void Expression::setTypeAndResult(symbols::AnnotationStore& store, symbols::ValueEntry result) {
    setType(result.getType());
    form = ValueForm::Scalar;
    store.setResult(this, std::move(result));
}

void Expression::setAggregateAddressResult(symbols::AnnotationStore& store,
        symbols::ValueEntry addressSymbol, const type::Type& aggregateType) {
    setType(aggregateType);
    form = ValueForm::AggregateAddress;
    store.setResult(this, std::move(addressSymbol));
}

void Expression::setFunctionDesignatorResult(symbols::AnnotationStore& store,
        symbols::ValueEntry addressSymbol, const type::Type& functionType) {
    setType(functionType);
    form = ValueForm::FunctionDesignator;
    if (exprKind() == ExprKind::Identifier) {
        static_cast<IdentifierExpression*>(this)->lval_ = false;
    }
    store.setResult(this, std::move(addressSymbol));
}

void Expression::takeValueFrom(Expression& src, symbols::AnnotationStore& store) {
    assert(src.hasResultSymbol(store));
    if (src.holdsAggregateAddress()) {
        setAggregateAddressResult(store, *src.getResultSymbol(store), src.expressionType());
    } else if (src.holdsFunctionDesignator()) {
        setFunctionDesignatorResult(store, *src.getResultSymbol(store), src.expressionType());
    } else {
        setTypeAndResult(store, *src.getResultSymbol(store));
    }
    if (auto* addr = src.getLvalueSymbol(store)) {
        setLvalueSymbol(store, *addr);
    }
    if (const auto* plan = store.addressPlan(&src)) {
        store.setAddressPlan(this, *plan);
    }
    if (exprKind() == ExprKind::Identifier) {
        static_cast<IdentifierExpression*>(this)->lval_ = src.isLval();
    } else if (exprKind() == ExprKind::GenericSelection) {
        static_cast<GenericSelection*>(this)->lval_ = src.isLval();
    }
}

bool Expression::hasResultSymbol(const symbols::AnnotationStore& store) const {
    return store.hasResult(this);
}

bool Expression::hasAnalyzedValue(const symbols::AnnotationStore& store) const {
    return isVoidValue() || hasResultSymbol(store);
}

symbols::ValueEntry* Expression::getResultSymbol(symbols::AnnotationStore& store) const {
    return store.result(this);
}

bool Expression::isLval() const {
    switch (exprKind()) {
    case ExprKind::Identifier:
        return static_cast<const IdentifierExpression*>(this)->lval_;
    case ExprKind::GenericSelection:
        return static_cast<const GenericSelection*>(this)->lval_;
    case ExprKind::ArrayAccess:
    case ExprKind::MemberAccess:
    case ExprKind::StringLiteral:
    case ExprKind::CompoundLiteral:
        return true;
    case ExprKind::Unary:
        return static_cast<const UnaryExpression*>(this)->op() == type::UnaryOp::Deref;
    default:
        return false;
    }
}

void Expression::setLvalueSymbol(symbols::AnnotationStore& store, symbols::ValueEntry address) {
    store.setLvalue(this, std::move(address));
}

symbols::ValueEntry* Expression::getLvalueSymbol(symbols::AnnotationStore& store) const {
    return store.lvalue(this);
}

symbols::ValueEntry* Expression::addressSymbol(symbols::AnnotationStore& store) const {
    if (!valueType(store).isPointer()) {
        if (auto* lv = getLvalueSymbol(store); lv && lv->getType().isPointer()) {
            return lv;
        }
    }
    return getResultSymbol(store);
}

} // namespace ast
