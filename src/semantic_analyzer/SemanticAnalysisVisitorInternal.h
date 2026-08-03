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
#include <functional>
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
#include "ast/PendingArrayMemberStore.h"
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

// Resolve TypeSpecifier (incl. typeof). On failure reports via error (if provided)
// and returns nullopt - callers bail, they do not invent void.
inline std::optional<type::Type> resolveTypeSpecifier(ast::TypeSpecifier& typeSpec,
        ast::AbstractSyntaxTreeVisitor& visitor,
        std::function<void(std::string, const translation_unit::Context&)> error = {},
        const translation_unit::Context* errorContext = nullptr) {
    if (typeSpec.needsSemanticResolve()) {
        typeSpec.resolveTypeof(visitor);
    }
    if (!typeSpec.hasType()) {
        if (error && errorContext) {
            error("cannot determine type of typeof operand", *errorContext);
        }
        return std::nullopt;
    }
    return typeSpec.getType();
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

// type_name: resolve specifier (incl. typeof), visit dad so array bounds fold
// via visit(ArrayDeclarator), then apply dad. Named-declaration bound errors
// are not raised here; getFundamentalType clamps BUILD_ASSERT char[-1].
inline std::optional<type::Type> resolveTypeName(ast::TypeName& typeName,
        ast::AbstractSyntaxTreeVisitor& visitor,
        std::function<void(std::string, const translation_unit::Context&)> error = {},
        const translation_unit::Context* errorContext = nullptr) {
    auto resolved = resolveTypeSpecifier(typeName.spec, visitor, error, errorContext);
    if (!resolved) {
        return std::nullopt;
    }
    type::Type result = typeName.applyDad(*resolved, visitor);
    visitVariableBounds(result, visitor);
    return result;
}

inline Logger& semanticErrorLogger() {
    return LogManager::getErrorLogger();
}

} // namespace semantic_analyzer

#endif // SEMANTIC_SEMANTICANALYSISVISITORINTERNAL_H_
