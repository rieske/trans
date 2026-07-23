#include "ArrayDecay.h"

namespace semantic_analyzer {


void decayArrayInPlace(ast::Expression& expr, SymbolTable& symbolTable) {
    if (!expr.hasResultSymbol() || !expr.getResultSymbol()->getType().isArray()) {
        return;
    }
    type::Type decayedType = expr.getResultSymbol()->getType().decayArray();
    if (auto* arrayAddress = expr.getLvalueSymbol()) {
        // Reuse only when the lvalue is already the decayed T* (MemberAccess /
        // multi-dim ArrayAccess). Do not reuse T(*)[N] — same bit pattern, wrong
        // type for pointer arithmetic scale.
        if (arrayAddress->getType().equivalentTo(decayedType)) {
            if (expr.hasExpressionType()) {
                expr.setResultSymbol(*arrayAddress);
            } else {
                expr.setTypedResult(*arrayAddress);
            }
            return;
        }
    }
    auto decayTemp = symbolTable.createTemporarySymbol(decayedType);
    expr.setArrayDecaySource(expr.getResultSymbol()->getName());
    // Preserve C array type for sizeof; value becomes pointer (dual ownership).
    if (expr.hasExpressionType()) {
        expr.setResultSymbol(decayTemp);
    } else {
        // Ensure expression type stays the array type, not the decayed pointer.
        expr.setType(expr.getResultSymbol()->getType());
        expr.setResultSymbol(decayTemp);
    }
}

} // namespace semantic_analyzer
