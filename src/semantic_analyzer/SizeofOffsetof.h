#ifndef SEMANTIC_SIZEOFOFFSETOF_H_
#define SEMANTIC_SIZEOFOFFSETOF_H_

#include <optional>

#include "symbols/AnnotationStore.h"

namespace ast {
class Expression;
class MemberAccess;
class UnaryExpression;
}

namespace semantic_analyzer {

class SymbolTable;

// Dual-type sizeof fold: string literals use lexical length (TypeQuery);
// otherwise sizeof is expressionType().getSize(). Operand must already be visited.
std::optional<long> foldedSizeofBytes(ast::Expression* operand);

// Product offsetof: &((T*)0)->member with SA FieldPlan already set on member.
// Returns field offset bytes when the arrow base is a constant 0.
std::optional<long> tryFoldOffsetofArrowFromNull(ast::MemberAccess* member,
        symbols::AnnotationStore& store);

// Apply sizeof result typing: size_t (unsigned long) + folded integer when known.
// Operand must already be visited.
void applySizeofResult(ast::UnaryExpression& expression, symbols::AnnotationStore& store,
        SymbolTable& symbolTable);

// If offsetof applies, set folded integer + signed int result and return true.
// Otherwise leave expression untouched and return false (caller continues AddressOf).
bool tryApplyOffsetofFold(ast::UnaryExpression& expression, ast::Expression* operand,
        symbols::AnnotationStore& store, SymbolTable& symbolTable);

} // namespace semantic_analyzer

#endif // SEMANTIC_SIZEOFOFFSETOF_H_
