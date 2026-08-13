#include "ArrayDecay.h"

namespace semantic_analyzer {


void decayArrayInPlace(ast::Expression& expr, SymbolTable& symbolTable,
        symbols::AnnotationStore& store) {
    if (!expr.hasResult(store) || !expr.result(store)->getType().isArray()) {
        return;
    }
    type::Type decayedType = expr.result(store)->getType().decayArray();
    if (auto* arrayAddress = expr.lvalueAnnotation(store)) {
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
            symbols::ArrayDecayPlan { expr.result(store)->getName() } });
    if (expr.hasExpressionType()) {
        expr.setAggregateAddressResult(store, decayTemp, expr.expressionType());
    } else {
        type::Type arrTy = expr.result(store)->getType();
        expr.setAggregateAddressResult(store, decayTemp, arrTy);
    }
}

} // namespace semantic_analyzer
