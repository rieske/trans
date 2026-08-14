#include "StaticInitFold.h"

#include "SemanticAnalysisVisitorInternal.h"

#include "ast/ArithmeticExpression.h"
#include "ast/ArrayAccess.h"
#include "ast/ConstantExpression.h"
#include "ast/IdentifierExpression.h"
#include "ast/MemberAccess.h"
#include "ast/StringLiteralExpression.h"
#include "ast/TypeCast.h"
#include "ast/UnaryExpression.h"
#include "symbols/AddressPlan.h"
#include "types/ObjectAbi.h"
#include "types/TypeQuery.h"
#include "util/FloatingLiteral.h"

#include <variant>

namespace semantic_analyzer {
namespace {

std::optional<symbols::StaticInitValue> foldStaticInit(
        const ast::Expression& expr, const symbols::AnnotationStore& store);
std::optional<symbols::StaticAddress> foldAddress(
        const ast::Expression& expr, const symbols::AnnotationStore& store);
std::optional<symbols::StaticAddress> foldDesignatorAddress(
        const ast::Expression& expr, const symbols::AnnotationStore& store);

const symbols::ValueEntry* staticObjectHome(
        const ast::IdentifierExpression& id, const symbols::AnnotationStore& store) {
    if (const auto* lv = store.lvalue(&id); lv && lv->isGlobal()) {
        return lv;
    }
    if (store.hasResult(&id)) {
        const symbols::ValueEntry* entry = store.result(&id);
        if (entry->isGlobal()) {
            return entry;
        }
    }
    return nullptr;
}

std::optional<symbols::StaticAddress> functionLabel(
        const ast::Expression& expr, const symbols::AnnotationStore& store) {
    if (!expr.holdsFunctionDesignator()) {
        return std::nullopt;
    }
    const auto* plan = store.addressPlan(&expr);
    const auto* designator = symbols::get_if<symbols::FunctionDesignatorPlan>(plan);
    if (!designator) {
        return std::nullopt;
    }
    return symbols::StaticAddress { designator->functionName };
}

std::optional<symbols::StaticAddress> foldIdentifierDesignator(
        const ast::IdentifierExpression& id, const symbols::AnnotationStore& store) {
    if (id.hasStringConstantLabel()) {
        return symbols::StaticAddress { id.getStringConstantLabel() };
    }
    if (auto fn = functionLabel(id, store)) {
        return fn;
    }
    const symbols::ValueEntry* home = staticObjectHome(id, store);
    if (!home) {
        return std::nullopt;
    }
    return symbols::StaticAddress { home->getName() };
}

std::optional<symbols::StaticAddress> foldMemberDesignator(
        const ast::MemberAccess& member, const symbols::AnnotationStore& store) {
    const auto* field = symbols::get_if<symbols::FieldPlan>(store.addressPlan(&member));
    if (!field || field->isBitField()) {
        return std::nullopt;
    }
    auto base = member.isArrow() ? foldAddress(*member.getBase(), store)
                                 : foldDesignatorAddress(*member.getBase(), store);
    if (!base) {
        return std::nullopt;
    }
    base->addend += field->fieldOffsetBytes;
    return base;
}

std::optional<symbols::StaticAddress> foldIndexDesignator(
        const ast::ArrayAccess& access, const symbols::AnnotationStore& store) {
    const auto* index = symbols::get_if<symbols::IndexPlan>(store.addressPlan(&access));
    if (!index) {
        return std::nullopt;
    }
    long i = 0;
    if (!access.getRightOperand()->evaluateConstant(i)) {
        return std::nullopt;
    }
    auto base = index->baseMode == symbols::AddressBaseMode::LeaObject
            ? foldDesignatorAddress(*access.getLeftOperand(), store)
            : foldAddress(*access.getLeftOperand(), store);
    if (!base) {
        return std::nullopt;
    }
    base->addend += i * static_cast<long>(index->elementSize);
    return base;
}

std::optional<symbols::StaticAddress> foldPointerArithmetic(
        const ast::ArithmeticExpression& arith, const symbols::AnnotationStore& store) {
    const std::string opLex = arith.getOperator()->getLexeme();
    if (opLex != "+" && opLex != "-") {
        return std::nullopt;
    }
    const ast::Expression& left = *arith.getLeftOperand();
    const ast::Expression& right = *arith.getRightOperand();
    const type::PointerArithmeticInfo info = type::classifyPointerArithmetic(
            type::afterLvalueConversion(left.valueType(store)),
            type::afterLvalueConversion(right.valueType(store)), opLex.front());
    if (info.form != type::PointerArithmeticForm::PtrPlusInt
            && info.form != type::PointerArithmeticForm::IntPlusPtr
            && info.form != type::PointerArithmeticForm::PtrMinusInt) {
        return std::nullopt;
    }

    const ast::Expression* addrExpr = &left;
    const ast::Expression* iceExpr = &right;
    long sign = 1;
    if (info.form == type::PointerArithmeticForm::IntPlusPtr) {
        addrExpr = &right;
        iceExpr = &left;
    } else if (info.form == type::PointerArithmeticForm::PtrMinusInt) {
        sign = -1;
    }

    auto addr = foldAddress(*addrExpr, store);
    long ice = 0;
    if (!addr || !iceExpr->evaluateConstant(ice)) {
        return std::nullopt;
    }
    addr->addend += sign * ice * static_cast<long>(info.strideBytes);
    return addr;
}

std::optional<symbols::StaticAddress> foldDesignatorAddress(
        const ast::Expression& expr, const symbols::AnnotationStore& store) {
    if (auto* id = dynamic_cast<const ast::IdentifierExpression*>(&expr)) {
        return foldIdentifierDesignator(*id, store);
    }
    if (auto* member = dynamic_cast<const ast::MemberAccess*>(&expr)) {
        return foldMemberDesignator(*member, store);
    }
    if (auto* access = dynamic_cast<const ast::ArrayAccess*>(&expr)) {
        return foldIndexDesignator(*access, store);
    }
    if (auto* unary = dynamic_cast<const ast::UnaryExpression*>(&expr)) {
        if (unary->getOperator()->getLexeme() == "*" && unary->getOperandExpression()) {
            return foldAddress(*unary->getOperandExpression(), store);
        }
    }
    return std::nullopt;
}

std::optional<symbols::StaticAddress> foldAddress(
        const ast::Expression& expr, const symbols::AnnotationStore& store) {
    if (auto* literal = dynamic_cast<const ast::StringLiteralExpression*>(&expr)) {
        if (literal->getConstantSymbol().empty()) {
            return std::nullopt;
        }
        return symbols::StaticAddress { literal->getConstantSymbol() };
    }
    if (auto* id = dynamic_cast<const ast::IdentifierExpression*>(&expr)) {
        auto addr = foldIdentifierDesignator(*id, store);
        if (!addr) {
            return std::nullopt;
        }
        if (id->hasStringConstantLabel() || id->holdsFunctionDesignator()
                || id->isArrayObjectType()) {
            return addr;
        }
        return std::nullopt;
    }
    if (auto* unary = dynamic_cast<const ast::UnaryExpression*>(&expr)) {
        if (unary->getOperator()->getLexeme() == "&" && unary->getOperandExpression()) {
            return foldDesignatorAddress(*unary->getOperandExpression(), store);
        }
    } else if (auto* cast = dynamic_cast<const ast::TypeCast*>(&expr)) {
        if (!cast->getOperandExpression()) {
            return std::nullopt;
        }
        return foldAddress(*cast->getOperandExpression(), store);
    } else if (auto* arith = dynamic_cast<const ast::ArithmeticExpression*>(&expr)) {
        return foldPointerArithmetic(*arith, store);
    }
    if (expr.holdsAggregateAddress()) {
        return foldDesignatorAddress(expr, store);
    }
    return std::nullopt;
}

std::optional<symbols::StaticInitValue> foldFloating(
        const ast::Expression& expr, const symbols::AnnotationStore& store) {
    if (auto* constant = dynamic_cast<const ast::ConstantExpression*>(&expr)) {
        util::FloatingBits parsed;
        if (!util::floatingLiteralBits(constant->getValue(), parsed)) {
            return std::nullopt;
        }
        return symbols::StaticFloat { parsed.bits, parsed.sizeBytes };
    }
    auto* unary = dynamic_cast<const ast::UnaryExpression*>(&expr);
    if (!unary) {
        return std::nullopt;
    }
    const std::string op = unary->getOperator()->getLexeme();
    if (op != "+" && op != "-") {
        return std::nullopt;
    }
    auto operand = foldStaticInit(*unary->getOperandExpression(), store);
    if (!operand) {
        return std::nullopt;
    }
    if (auto* fp = std::get_if<symbols::StaticFloat>(&*operand)) {
        if (op == "-") {
            const double negated = -util::decodeFloating(fp->bits, fp->sizeBytes);
            fp->bits = util::encodeFloating(negated, fp->sizeBytes);
        }
        return *operand;
    }
    if (auto* integer = std::get_if<symbols::StaticInteger>(&*operand)) {
        if (op == "-") {
            integer->value = -integer->value;
        }
        return *operand;
    }
    return std::nullopt;
}

std::optional<symbols::StaticInitValue> foldStaticInit(
        const ast::Expression& expr, const symbols::AnnotationStore& store) {
    if (expr.hasExpressionType() && type::isFloating(expr.expressionType())) {
        return foldFloating(expr, store);
    }
    long ice = 0;
    if (expr.evaluateConstant(ice)) {
        return symbols::StaticInteger { ice };
    }
    if (auto addr = foldAddress(expr, store)) {
        return *addr;
    }
    return std::nullopt;
}

std::optional<symbols::StaticInitValue> convertToDest(
        const type::Type& dest, symbols::StaticInitValue value) {
    if (type::isFloating(dest)) {
        const int destSize = dest.getSize();
        if (auto* integer = std::get_if<symbols::StaticInteger>(&value)) {
            return symbols::StaticFloat {
                    util::encodeFloating(static_cast<double>(integer->value), destSize), destSize };
        }
        if (auto* fp = std::get_if<symbols::StaticFloat>(&value)) {
            if (fp->sizeBytes == destSize) {
                return value;
            }
            const double decoded = util::decodeFloating(fp->bits, fp->sizeBytes);
            return symbols::StaticFloat { util::encodeFloating(decoded, destSize), destSize };
        }
        return std::nullopt;
    }
    if (auto* fp = std::get_if<symbols::StaticFloat>(&value)) {
        const double decoded = util::decodeFloating(fp->bits, fp->sizeBytes);
        if (type::isBoolean(dest)) {
            return symbols::StaticInteger { decoded != 0.0 };
        }
        return symbols::StaticInteger { type::convertScalarConstant(dest, static_cast<long>(decoded)) };
    }
    if (auto* integer = std::get_if<symbols::StaticInteger>(&value)) {
        integer->value = type::convertScalarConstant(dest, integer->value);
        return value;
    }
    if (std::holds_alternative<symbols::StaticAddress>(value)) {
        if (dest.isPointer()) {
            return value;
        }
        if (type::isIntegral(dest) && dest.getSize() == type::object_abi::MACHINE_WORD_SIZE) {
            return value;
        }
        return std::nullopt;
    }
    return value;
}

} // namespace

std::optional<symbols::StaticInitValue> evaluateStaticInit(
        SemanticAnalysisVisitor& visitor, const ast::Expression& expr, const type::Type& dest,
        const translation_unit::Context& context) {
    const type::Type src = assignSourceType(expr, dest, visitor.annotations());
    if (!visitor.checkAssign(dest, src, context, &expr)) {
        return std::nullopt;
    }
    auto folded = foldStaticInit(expr, visitor.annotations());
    if (!folded) {
        visitor.semanticError("global initializer is not a constant expression", context);
        return std::nullopt;
    }
    auto converted = convertToDest(dest, std::move(*folded));
    if (!converted) {
        visitor.semanticError("global initializer is not a constant expression", context);
        return std::nullopt;
    }
    return converted;
}

} // namespace semantic_analyzer
