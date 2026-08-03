#ifndef SEMANTIC_SEMANTICANALYSISVISITORINTERNAL_H_
#define SEMANTIC_SEMANTICANALYSISVISITORINTERNAL_H_

// Shared includes and TU-local helpers for SemanticAnalysisVisitor*.cpp

#include "SemanticAnalysisVisitor.h"

#include "ArrayDecay.h"
#include "builtins/BuiltinRegistry.h"
#include "ConstantAddress.h"
#include "Conversion.h"
#include "ProductAssign.h"
#include "SizeofOffsetof.h"

#include <algorithm>
#include <map>
#include <optional>
#include <stdexcept>

#include "ast/ArrayAccess.h"
#include "ast/CompoundLiteralExpression.h"
#include "ast/ConstantExpression.h"
#include "ast/Declarator.h"
#include "ast/DoubleOperandExpression.h"
#include "ast/IdentifierExpression.h"
#include "ast/InitializerListExpression.h"
#include "ast/MemberAccess.h"
#include "ast/Operator.h"
#include "ast/StringLiteralExpression.h"
#include "ast/GenericSelection.h"
#include "ast/TypeCast.h"
#include "ast/TypeName.h"
#include "ast/TypeSpecifier.h"
#include "ast/UnaryExpression.h"
#include "translation_unit/Context.h"
#include "types/Type.h"
#include "types/TypeQuery.h"
#include "util/LogManager.h"
#include "util/Logger.h"

namespace semantic_analyzer {

inline type::Type valueTypeAfterDesignatorDecay(ast::Expression& expr,
        const symbols::AnnotationStore& store) {
    type::Type t = expr.valueType(store);
    if (expr.holdsFunctionDesignator() && !t.isPointer()) {
        return type::pointer(expr.expressionType());
    }
    return t;
}

// Walk VLA / pointer-to-VLA bound expressions so sizeof and pointer stride see them.
inline void visitVariableBounds(const type::Type& t, ast::AbstractSyntaxTreeVisitor& visitor) {
    if (t.isArray()) {
        if (auto bound = t.variableBound()) {
            bound->accept(visitor);
        }
        visitVariableBounds(t.getElementType(), visitor);
        return;
    }
    if (t.isPointer()) {
        visitVariableBounds(t.dereference(), visitor);
    }
}

// Skip the object load: Result is the lvalue address (same form as array a[i]).
inline void markAddressOnly(ast::Expression& expr, symbols::AnnotationStore& store) {
    auto* lv = expr.getLvalueSymbol(store);
    if (!lv || expr.holdsAggregateAddress()) {
        return;
    }
    expr.setAggregateAddressResult(store, *lv, expr.expressionType());
}

inline translation_unit::Context arrayBoundContext(const type::Type& t) {
    type::Type walk = t;
    while (walk.isArray()) {
        if (auto bound = walk.variableBound()) {
            return bound->getContext();
        }
        walk = walk.getElementType();
    }
    if (walk.isPointer()) {
        return arrayBoundContext(walk.dereference());
    }
    return translation_unit::Context { "", 0 };
}

inline Logger& semanticErrorLogger() {
    return LogManager::getErrorLogger();
}

} // namespace semantic_analyzer

#endif // SEMANTIC_SEMANTICANALYSISVISITORINTERNAL_H_
