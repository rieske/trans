#ifndef SEMANTIC_CONVERSION_H_
#define SEMANTIC_CONVERSION_H_

// SA conversion temps (float/int width, bool 0/1). Free helpers so declaration
// analysis and aggregate sinks do not include the visitor surface.

#include "SymbolTable.h"
#include "ast/Expression.h"
#include "symbols/AnnotationStore.h"
#include "types/Type.h"
#include "types/TypeQuery.h"

namespace semantic_analyzer {

inline void maybeSetConversion(ast::Expression* expr,
        const type::Type& targetType,
        SymbolTable& symbolTable,
        symbols::AnnotationStore& store) {
    if (!expr || !expr->hasResultSymbol(store)) {
        return;
    }
    const type::Type& from = expr->getResultSymbol(store)->getType();
    if (type::needsConversion(from, targetType)) {
        store.setValue(expr, symbols::ValueSlot::Conversion,
                symbolTable.createTemporarySymbol(targetType));
    }
}

inline type::Type applyUsualArithmeticConversions(ast::Expression& left,
        ast::Expression& right,
        SymbolTable& symbolTable,
        symbols::AnnotationStore& store) {
    const type::Type resultType = type::usualArithmeticResult(
            left.getResultSymbol(store)->getType(),
            right.getResultSymbol(store)->getType());
    maybeSetConversion(&left, resultType, symbolTable, store);
    maybeSetConversion(&right, resultType, symbolTable, store);
    return resultType;
}

// C 6.3.1.1: single-operand integer promotions. Result type of << >> and of
// unary + - ~ is the promoted operand (6.5.7, 6.5.3.3).
inline type::Type applyIntegerPromotion(ast::Expression& expr,
        SymbolTable& symbolTable,
        symbols::AnnotationStore& store) {
    const type::Type promoted = type::integerPromote(expr.getResultSymbol(store)->getType());
    maybeSetConversion(&expr, promoted, symbolTable, store);
    return promoted;
}

} // namespace semantic_analyzer

#endif // SEMANTIC_CONVERSION_H_
