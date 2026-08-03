#include "ArrayDecay.h"

namespace semantic_analyzer {


void decayArrayInPlace(ast::Expression& expr, SymbolTable& symbolTable,
        symbols::AnnotationStore& store) {
    if (!expr.hasResultSymbol(store) || !expr.getResultSymbol(store)->getType().isArray()) {
        return;
    }
    type::Type decayedType = expr.getResultSymbol(store)->getType().decayArray();
    if (auto* arrayAddress = expr.getLvalueSymbol(store)) {
        if (arrayAddress->getType().equivalentTo(decayedType)) {
            if (expr.hasExpressionType()) {
                expr.setAggregateAddressResult(store, *arrayAddress, expr.expressionType());
            } else {
                expr.setTypeAndResult(store, *arrayAddress);
            }
            return;
        }
    }
    auto decayTemp = symbolTable.createTemporarySymbol(decayedType);
    store.setAddressPlan(&expr, symbols::AddressPlan {
            symbols::ArrayDecayPlan { expr.getResultSymbol(store)->getName() } });
    if (expr.hasExpressionType()) {
        expr.setAggregateAddressResult(store, decayTemp, expr.expressionType());
    } else {
        type::Type arrTy = expr.getResultSymbol(store)->getType();
        expr.setAggregateAddressResult(store, decayTemp, arrTy);
    }
}

} // namespace semantic_analyzer
