#include "StaticInitFold.h"

#include "SemanticAnalysisVisitorInternal.h"

#include "ast/ConstantExpression.h"
#include "ast/IdentifierExpression.h"
#include "ast/StringLiteralExpression.h"
#include "ast/UnaryExpression.h"
#include "symbols/AddressPlan.h"
#include "types/TypeQuery.h"
#include "util/FloatingLiteral.h"

#include <variant>

namespace semantic_analyzer {
namespace {

symbols::StaticFloat toStaticFloat(const util::FloatingBits& fp) {
    return { fp.bits, fp.bitsHi, fp.sizeBytes };
}

util::FloatingBits toFloatingBits(const symbols::StaticFloat& fp) {
    return { fp.bits, fp.bitsHi, fp.sizeBytes };
}

std::optional<symbols::StaticInitValue> foldStaticInit(
        const ast::Expression& expr, const symbols::AnnotationStore& store);

std::optional<symbols::StaticInitValue> functionLabel(
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

std::optional<symbols::StaticInitValue> foldIdentifier(
        const ast::IdentifierExpression& id, const symbols::AnnotationStore& store) {
    if (id.hasStringConstantLabel()) {
        return symbols::StaticAddress { id.getStringConstantLabel() };
    }
    if (auto fn = functionLabel(id, store)) {
        return fn;
    }
    if (!store.hasResult(&id)) {
        return std::nullopt;
    }
    const symbols::ValueEntry* entry = store.result(&id);
    if (entry->isGlobal() && entry->getType().isArray()) {
        return symbols::StaticAddress { entry->getName() };
    }
    return std::nullopt;
}

std::optional<symbols::StaticInitValue> foldAddressOf(
        const ast::UnaryExpression& unary, const symbols::AnnotationStore& store) {
    if (unary.getOperator()->getLexeme() != "&") {
        return std::nullopt;
    }
    ast::Expression* operand = unary.getOperandExpression();
    if (!operand) {
        return std::nullopt;
    }
    if (auto fn = functionLabel(*operand, store)) {
        return fn;
    }
    auto* id = dynamic_cast<ast::IdentifierExpression*>(operand);
    if (!id || !store.hasResult(id)) {
        return std::nullopt;
    }
    const symbols::ValueEntry* entry = store.result(id);
    if (!entry->isGlobal()) {
        return std::nullopt;
    }
    return symbols::StaticAddress { entry->getName() };
}

std::optional<symbols::StaticInitValue> foldFloating(
        const ast::Expression& expr, const symbols::AnnotationStore& store) {
    if (auto* constant = dynamic_cast<const ast::ConstantExpression*>(&expr)) {
        util::FloatingBits parsed;
        if (!util::floatingLiteralBits(constant->getValue(), parsed)) {
            return std::nullopt;
        }
        return toStaticFloat(parsed);
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
            *fp = toStaticFloat(util::negateFloating(toFloatingBits(*fp)));
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
    if (auto* literal = dynamic_cast<const ast::StringLiteralExpression*>(&expr)) {
        if (literal->getConstantSymbol().empty()) {
            return std::nullopt;
        }
        return symbols::StaticAddress { literal->getConstantSymbol() };
    }
    if (auto* id = dynamic_cast<const ast::IdentifierExpression*>(&expr)) {
        return foldIdentifier(*id, store);
    }
    if (auto* unary = dynamic_cast<const ast::UnaryExpression*>(&expr)) {
        return foldAddressOf(*unary, store);
    }
    return std::nullopt;
}

std::optional<symbols::StaticInitValue> convertToDest(
        const type::Type& dest, symbols::StaticInitValue value) {
    if (type::isFloating(dest)) {
        const int destSize = dest.getSize();
        if (auto* integer = std::get_if<symbols::StaticInteger>(&value)) {
            return toStaticFloat(
                    util::encodeFloating(static_cast<long double>(integer->value), destSize));
        }
        if (auto* fp = std::get_if<symbols::StaticFloat>(&value)) {
            return toStaticFloat(util::convertFloating(toFloatingBits(*fp), destSize));
        }
        return std::nullopt;
    }
    if (auto* fp = std::get_if<symbols::StaticFloat>(&value)) {
        const long double decoded = util::decodeFloating(toFloatingBits(*fp));
        if (type::isBoolean(dest)) {
            return symbols::StaticInteger { decoded != 0.0 };
        }
        return symbols::StaticInteger { type::convertScalarConstant(dest, static_cast<long>(decoded)) };
    }
    if (auto* integer = std::get_if<symbols::StaticInteger>(&value)) {
        integer->value = type::convertScalarConstant(dest, integer->value);
        return value;
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
