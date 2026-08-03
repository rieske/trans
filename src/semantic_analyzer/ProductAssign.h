#ifndef SEMANTIC_PRODUCTASSIGN_H_
#define SEMANTIC_PRODUCTASSIGN_H_

// Single product assign gate used by SA visitor, declaration analysis, and
// aggregate init sinks. Type-only rules live in type::productAssignFrom;
// foldable zero ((void*)0, 0, (int)(1-1), ...) needs the source expression.

#include "ast/Expression.h"
#include "translation_unit/Context.h"
#include "types/Type.h"
#include "types/TypeQuery.h"

#include <functional>
#include <string>
#include <utility>

namespace semantic_analyzer {

inline bool foldsToIntegerZero(const ast::Expression& expr) {
    long value = 0;
    return expr.foldToHostLong(value) && value == 0;
}

// dest <- source. Pass sourceExpr so foldable zero is accepted into pointers.
inline bool productAssignOk(const type::Type& dest, const type::Type& source,
        const ast::Expression* sourceExpr = nullptr) {
    if (type::productAssignFrom(dest, source)) {
        return true;
    }
    return sourceExpr && dest.isPointer() && foldsToIntegerZero(*sourceExpr);
}

// Diagnose and return false on failure. Used by sinks / declaration analysis
// (visitor uses SemanticAnalysisVisitor::checkProductAssign).
inline bool reportProductAssign(
        const std::function<void(std::string, const translation_unit::Context&)>& error,
        const type::Type& dest, const type::Type& source,
        const translation_unit::Context& context,
        const ast::Expression* sourceExpr = nullptr) {
    if (productAssignOk(dest, source, sourceExpr)) {
        return true;
    }
    error(type::productAssignFailureMessage(dest, source), context);
    return false;
}

} // namespace semantic_analyzer

#endif // SEMANTIC_PRODUCTASSIGN_H_
