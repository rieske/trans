#ifndef SEMANTIC_ARRAYDECAY_H_
#define SEMANTIC_ARRAYDECAY_H_

#include "ast/Expression.h"
#include "symbols/AnnotationStore.h"
#include "SymbolTable.h"

namespace semantic_analyzer {


// Array-to-pointer decay in place on `expr` (C 6.3.2.1).
void decayArrayInPlace(ast::Expression& expr, SymbolTable& symbolTable,
        symbols::AnnotationStore& store);

inline void decayArrayInPlace(ast::Expression* expr, SymbolTable& symbolTable,
        symbols::AnnotationStore& store) {
    if (expr) {
        decayArrayInPlace(*expr, symbolTable, store);
    }
}

} // namespace semantic_analyzer

#endif // SEMANTIC_ARRAYDECAY_H_
