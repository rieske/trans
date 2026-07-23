#ifndef SEMANTIC_ARRAYDECAY_H_
#define SEMANTIC_ARRAYDECAY_H_

#include "ast/Expression.h"
#include "SymbolTable.h"

namespace semantic_analyzer {

// Array-to-pointer decay in place on `expr` (C 6.3.2.1).
//
// If expr's result is an array:
//   - when an object address is already available as lvalue, reuse it as the result
//   - otherwise create a pointer temporary, record the array storage name on the
//     expression (getArrayDecaySource), and set the result to that temporary
//
// Codegen materializes AddressOf(source, result) via materializeArrayDecay.
// Parents must not store decay sources; the decayed expression owns them.
void decayArrayInPlace(ast::Expression& expr, SymbolTable& symbolTable);

// Convenience for optional / nullable expression pointers.
inline void decayArrayInPlace(ast::Expression* expr, SymbolTable& symbolTable) {
    if (expr) {
        decayArrayInPlace(*expr, symbolTable);
    }
}

} // namespace semantic_analyzer

#endif // SEMANTIC_ARRAYDECAY_H_
