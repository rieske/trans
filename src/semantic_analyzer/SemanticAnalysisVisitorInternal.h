#ifndef SEMANTIC_SEMANTICANALYSISVISITORINTERNAL_H_
#define SEMANTIC_SEMANTICANALYSISVISITORINTERNAL_H_

// Shared includes and TU-local helpers for SemanticAnalysisVisitor*.cpp

#include "SemanticAnalysisVisitor.h"

#include "ArrayDecay.h"
#include "ast/BuiltinRegistry.h"
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
#include "ast/UnaryExpression.h"
#include "types/TypeQuery.h"
#include "util/ProductApprox.h"
#include "util/StringLiteralDecode.h"
#include "ast/Operator.h"
#include "ast/PendingArrayMemberStore.h"
#include "scanner/EnumConstantRegistry.h"
#include "translation_unit/Context.h"
#include "types/ObjectAbi.h"
#include "types/Type.h"
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

// What numeric conversions to materialize as resultConversionTarget temps.
enum class ConversionPolicy {
    FloatIntOnly,       // call formals (C 6.5.2.2)
    FloatIntAndWidth    // returns: float↔int + integral width (ntohl → off_t)
};

inline void setNumericConversionIfNeeded(ast::Expression* expr,
        const type::Type& targetType,
        SymbolTable& symbolTable,
        ConversionPolicy policy) {
    if (!expr || !expr->hasResultSymbol()) {
        return;
    }
    const type::Type& from = expr->getResultSymbol()->getType();
    const bool floatInt = (type::isFloating(from) && type::isIntegral(targetType))
            || (type::isIntegral(from) && type::isFloating(targetType));
    const bool intWidth = policy == ConversionPolicy::FloatIntAndWidth
            && type::isIntegral(from) && type::isIntegral(targetType)
            && from.getSize() != targetType.getSize();
    if (floatInt || intWidth) {
        auto convertTemp = symbolTable.createTemporarySymbol(targetType);
        expr->setResultConversionTarget(convertTemp.getName());
    }
}

// Returns: float↔int and integral width (ntohl → off_t).
inline void maybeSetReturnConversion(ast::Expression* expr,
        const type::Type& returnType,
        SymbolTable& symbolTable) {
    setNumericConversionIfNeeded(expr, returnType, symbolTable, ConversionPolicy::FloatIntAndWidth);
}

// Call formals: float↔int only (not bare integral width — different ABI path).
inline void maybeSetCallArgConversion(ast::Expression* expr,
        const type::Type& formalType,
        SymbolTable& symbolTable) {
    setNumericConversionIfNeeded(expr, formalType, symbolTable, ConversionPolicy::FloatIntOnly);
}

} // namespace semantic_analyzer

#endif // SEMANTIC_SEMANTICANALYSISVISITORINTERNAL_H_
