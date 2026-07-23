#include "ArrayDecay.h"

namespace semantic_analyzer {

void decayArrayInPlace(ast::Expression& expr, SymbolTable& symbolTable) {
    if (!expr.hasResultSymbol() || !expr.getResultSymbol()->getType().isArray()) {
        return;
    }
    type::Type decayedType = expr.getResultSymbol()->getType().decayArray();
    if (auto* arrayAddress = expr.getLvalueSymbol()) {
        // Member / subscript array already exposes the object address.
        expr.setResultSymbol(*arrayAddress);
        return;
    }
    auto decayTemp = symbolTable.createTemporarySymbol(decayedType);
    expr.setArrayDecaySource(expr.getResultSymbol()->getName());
    expr.setResultSymbol(decayTemp);
}

} // namespace semantic_analyzer
