#ifndef SEMANTIC_SEMANTICANALYSISVISITORINTERNAL_H_
#define SEMANTIC_SEMANTICANALYSISVISITORINTERNAL_H_

// Shared includes and TU-local helpers for SemanticAnalysisVisitor*.cpp

#include "SemanticAnalysisVisitor.h"

#include "ArrayDecay.h"
#include "builtins/BuiltinRegistry.h"
#include "ConstantAddress.h"
#include "DeclarationAnalyzer.h"
#include "InitializerLowering.h"

#include <algorithm>
#include <functional>
#include <map>
#include <sstream>
#include <stdexcept>

#include "ast/ArrayAccess.h"
#include "ast/CompoundLiteralExpression.h"
#include "ast/ConstantExpression.h"
#include "ast/Declarator.h"
#include "ast/DoubleOperandExpression.h"
#include "ast/IdentifierExpression.h"
#include "ast/InitializerListExpression.h"
#include "ast/MemberAccess.h"
#include "ast/StringLiteralExpression.h"
#include "ast/TypeCast.h"
#include "ast/TypeSpecifier.h"
#include "ast/UnaryExpression.h"
#include "types/TypeQuery.h"
#include "types/TypeQuery.h"
#include "util/ProductApprox.h"
#include "util/StringLiteralDecode.h"
#include "ast/Operator.h"
#include "ast/PendingArrayMemberStore.h"
#include "translation_unit/Context.h"
#include "types/ObjectAbiType.h"
#include "types/Type.h"
#include "types/TypeQuery.h"
#include "types/TypeQuery.h"
#include "util/LogManager.h"
#include "util/Logger.h"
#include "util/StringLiteralDecode.h"

namespace semantic_analyzer {


// File-scope "external" context for synthetic / predeclared symbols (printf, etc.).
inline const translation_unit::Context& externalContext() {
    static const translation_unit::Context ctx { "external", 0 };
    return ctx;
}

// Resolve TypeSpecifier to a concrete type. For __typeof__(e), analyze e and
// setResolvedType so isTypeof() becomes false and getType() is trustworthy.
// On failure, reports via error (if provided) and leaves type as void.
inline type::Type resolveTypeSpecifier(ast::TypeSpecifier& typeSpec,
        ast::AbstractSyntaxTreeVisitor& visitor,
        symbols::AnnotationStore& store,
        std::function<void(std::string, const translation_unit::Context&)> error = {},
        const translation_unit::Context* errorContext = nullptr) {
    if (!typeSpec.isTypeof()) {
        return typeSpec.getType();
    }
    if (!typeSpec.getTypeofOperand()) {
        if (error && errorContext) {
            error("cannot determine type of typeof operand", *errorContext);
        }
        typeSpec.setResolvedType(type::voidType());
        return type::voidType();
    }
    typeSpec.getTypeofOperand()->accept(visitor);
    type::Type resolved = type::voidType();
    bool ok = false;
    if (typeSpec.getTypeofOperand()->hasExpressionType()) {
        resolved = typeSpec.getTypeofOperand()->expressionType();
        ok = true;
    } else if (typeSpec.getTypeofOperand()->hasResult(store)) {
        resolved = typeSpec.getTypeofOperand()->result(store)->getType();
        ok = true;
    }
    if (!ok) {
        if (error && errorContext) {
            error("cannot determine type of typeof operand", *errorContext);
        }
        resolved = type::voidType();
    }
    typeSpec.setResolvedType(resolved);
    return resolved;
}

inline Logger& semanticErrorLogger() {
    return LogManager::getErrorLogger();
}

// Walk expression trees that appear in array bounds (sizeof + arithmetic + casts).
// onSizeof is invoked for sizeof unary nodes and should not recurse further unless needed.
template <typename OnSizeof>
void walkBoundExpressionTree(ast::Expression* expr, OnSizeof&& onSizeof) {
    if (!expr) {
        return;
    }
    if (auto* unary = dynamic_cast<ast::UnaryExpression*>(expr)) {
        if (unary->isSizeof()) {
            onSizeof(unary);
            return;
        }
        walkBoundExpressionTree(unary->getOperandExpression(), onSizeof);
        return;
    }
    if (auto* bin = dynamic_cast<ast::DoubleOperandExpression*>(expr)) {
        walkBoundExpressionTree(bin->getLeftOperand(), onSizeof);
        walkBoundExpressionTree(bin->getRightOperand(), onSizeof);
        return;
    }
    if (auto* cast = dynamic_cast<ast::TypeCast*>(expr)) {
        walkBoundExpressionTree(cast->getOperandExpression(), onSizeof);
    }
}

// Whether to also materialize integral width converts (ntohl → off_t on return).
// SseOnly: needsNumericConvert only (float↔int, float↔double). Call formals.
enum class ConversionPolicy {
    SseOnly,
    WithIntegralWidth,
};

inline void setNumericConversionIfNeeded(ast::Expression* expr,
        const type::Type& targetType,
        SymbolTable& symbolTable,
        symbols::AnnotationStore& store,
        ConversionPolicy policy) {
    if (!expr || !expr->hasResult(store)) {
        return;
    }
    const type::Type& from = expr->result(store)->getType();
    const bool numeric = type::needsNumericConvert(from, targetType);
    const bool intWidth = policy == ConversionPolicy::WithIntegralWidth
            && type::isIntegral(from) && type::isIntegral(targetType)
            && from.getSize() != targetType.getSize();
    if (numeric || intWidth) {
        auto convertTemp = symbolTable.createTemporarySymbol(targetType);
        store.setString(expr, symbols::StringSlot::ConversionTarget, convertTemp.getName());
    }
}

// Returns: SSE numeric convert plus integral width (ntohl → off_t).
inline void maybeSetReturnConversion(ast::Expression* expr,
        const type::Type& returnType,
        SymbolTable& symbolTable,
        symbols::AnnotationStore& store) {
    setNumericConversionIfNeeded(expr, returnType, symbolTable, store, ConversionPolicy::WithIntegralWidth);
}

// Call formals and variadic default promotions: SSE numeric convert only.
inline void maybeSetCallArgConversion(ast::Expression* expr,
        const type::Type& formalType,
        SymbolTable& symbolTable,
        symbols::AnnotationStore& store) {
    setNumericConversionIfNeeded(expr, formalType, symbolTable, store, ConversionPolicy::SseOnly);
}

} // namespace semantic_analyzer

#endif // SEMANTIC_SEMANTICANALYSISVISITORINTERNAL_H_
