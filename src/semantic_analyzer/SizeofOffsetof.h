#ifndef SEMANTIC_SIZEOFOFFSETOF_H_
#define SEMANTIC_SIZEOFOFFSETOF_H_

#include <optional>

#include "ast/AbstractSyntaxTreeVisitor.h"
#include "ast/DoubleOperandExpression.h"
#include "ast/GenericSelection.h"
#include "ast/OffsetofExpression.h"
#include "ast/TypeCast.h"
#include "ast/UnaryExpression.h"
#include "symbols/AnnotationStore.h"

namespace ast {
class Expression;
class MemberAccess;
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

// Walk sizeof / offsetof / TypeCast / _Generic + arithmetic in array bounds.
// onFoldable is invoked for those nodes (typeof target, GenericSelection select).
template <typename OnFoldable>
void walkBoundExpressionTree(ast::Expression* expr, OnFoldable&& onFoldable) {
    if (!expr) {
        return;
    }
    if (auto* unary = dynamic_cast<ast::UnaryExpression*>(expr)) {
        if (unary->isSizeof()) {
            onFoldable(unary);
            return;
        }
        walkBoundExpressionTree(unary->getOperandExpression(), onFoldable);
        return;
    }
    if (auto* off = dynamic_cast<ast::OffsetofExpression*>(expr)) {
        onFoldable(off);
        return;
    }
    if (auto* generic = dynamic_cast<ast::GenericSelection*>(expr)) {
        onFoldable(generic);
        return;
    }
    if (auto* bin = dynamic_cast<ast::DoubleOperandExpression*>(expr)) {
        walkBoundExpressionTree(bin->getLeftOperand(), onFoldable);
        walkBoundExpressionTree(bin->getRightOperand(), onFoldable);
        return;
    }
    if (auto* cast = dynamic_cast<ast::TypeCast*>(expr)) {
        onFoldable(cast);
        walkBoundExpressionTree(cast->getOperandExpression(), onFoldable);
        return;
    }
}

// Late ARRAY_SIZE fold: sizeof(type_name) via visitor; sizeof(id)/sizeof(arr[0])
// from the symbol table (no semanticError on missing names).
void foldSizeofInBound(ast::Expression* expr, SymbolTable& symbolTable,
        ast::AbstractSyntaxTreeVisitor& visitor);

} // namespace semantic_analyzer

#endif // SEMANTIC_SIZEOFOFFSETOF_H_
