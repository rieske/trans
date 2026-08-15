#ifndef SEMANTICANALYSISVISITOR_INTERNAL_H_
#define SEMANTICANALYSISVISITOR_INTERNAL_H_

#include "SemanticAnalysisVisitor.h"

#include <cctype>
#include <string>
#include <utility>

#include "ast/Expression.h"
#include "types/IntegerConstant.h"
#include "types/Type.h"
#include "types/TypeQuery.h"
#include "util/Logger.h"
#include "util/LogManager.h"

namespace ast {
class TypeSpecifier;
}

namespace semantic_analyzer {

struct IncompleteArrayBound {
    enum class Kind { None, Bound, Error };
    Kind kind { Kind::None };
    int bound { 0 };
    std::string error;

    static IncompleteArrayBound none() {
        return {};
    }
    static IncompleteArrayBound sized(int n) {
        IncompleteArrayBound r;
        r.kind = Kind::Bound;
        r.bound = n;
        return r;
    }
    static IncompleteArrayBound fail(std::string message) {
        IncompleteArrayBound r;
        r.kind = Kind::Error;
        r.error = std::move(message);
        return r;
    }
};

IncompleteArrayBound incompleteArrayBoundFromInitializer(ast::Expression* init);

void visitVariableBounds(const type::Type& t, ast::AbstractSyntaxTreeVisitor& visitor);
void finalizeRecordDefinition(type::Type& record, SemanticAnalysisVisitor& visitor);
void finalizeSpecifierType(ast::TypeSpecifier& spec, SemanticAnalysisVisitor& visitor);

// Prototype / definition compatibility (return + arity + arg types + variadic).
inline bool functionTypesCompatible(const type::Function& existing, const type::Function& incoming) {
    if (!existing.getReturnType().equivalentTo(incoming.getReturnType())) {
        return false;
    }
    if (existing.isVariadic() != incoming.isVariadic()) {
        return false;
    }
    const auto existingArgs = existing.getArguments();
    const auto newArgs = incoming.getArguments();
    if (existingArgs.size() != newArgs.size()) {
        return false;
    }
    for (std::size_t i = 0; i < existingArgs.size(); ++i) {
        if (!existingArgs[i].equivalentTo(newArgs[i])) {
            return false;
        }
    }
    return true;
}

inline bool staticFollowsNonStatic(bool existingInternal, bool incomingInternal) {
    return !existingInternal && incomingInternal;
}

inline std::string staticFollowsNonStaticMessage(const std::string& name) {
    return "static declaration of `" + name + "` follows non-static declaration";
}

inline std::string nonStaticFollowsStaticMessage(const std::string& name) {
    return "non-static declaration of `" + name + "` follows static declaration";
}

// Locals are stored as `$s<scopeId><name>`; strip for diagnostics / function lookup.
inline std::string unscopedSymbolName(const std::string& name) {
    if (name.size() > 2 && name[0] == '$' && name[1] == 's') {
        std::size_t i = 2;
        while (i < name.size() && std::isdigit(static_cast<unsigned char>(name[i]))) {
            ++i;
        }
        if (i > 2 && i < name.size()) {
            return name.substr(i);
        }
    }
    return name;
}

inline Logger& semanticErrorLogger() {
    return LogManager::getErrorLogger();
}

// Array lvalue used as a pointer: result is a pointer temp.
inline void decayArrayToPointer(ast::Expression& expr, const type::Type& dest,
        SymbolTable& symbolTable, symbols::AnnotationStore& store) {
    if (!expr.hasResultSymbol(store)) {
        return;
    }
    const type::Type actual = expr.getResultSymbol(store)->getType();
    if (!actual.isArray() || !dest.isPointer()) {
        return;
    }
    expr.setLvalueSymbol(store, *expr.getResultSymbol(store));
    expr.setAggregateAddressResult(store, symbolTable.createTemporarySymbol(actual.decayArray()),
            actual);
}

inline void decayArrayValue(ast::Expression& expr, SymbolTable& symbolTable,
        symbols::AnnotationStore& store) {
    if (!expr.hasResultSymbol(store)) {
        return;
    }
    const type::Type actual = expr.getResultSymbol(store)->getType();
    if (!actual.isArray()) {
        return;
    }
    decayArrayToPointer(expr, actual.decayArray(), symbolTable, store);
}

// Source type for assignment/init/return into `dest`.
// Dual-type aggregate addresses use the pointer value when dest is a pointer
// (array-row decay); structure destinations still see the aggregate expression type.
inline type::Type assignSourceType(const ast::Expression& expr, const type::Type& dest,
        symbols::AnnotationStore& store) {
    // Dual-type: C object/function type on the node, address in Result.
    if ((expr.holdsAggregateAddress() || expr.holdsFunctionDesignator()) && dest.isPointer()) {
        return expr.getResultSymbol(store)->getType();
    }
    return expr.getType();
}

// True when evaluateConstant yields 0 (0, (void*)0, (int)(1-1), ...).
// Product uses this as a null-pointer-constant proxy for pointer destinations;
// it is not a full ISO ICE validator beyond what evaluateConstant folds.
inline bool foldsToIntegerZero(const ast::Expression& expr) {
    type::IntegerConstant value;
    return expr.evaluateConstant(value) && type::isZero(value);
}

// Full product assign gate (dest, source order matches productAssignFrom / canAssignFrom).
// Type-only productAssignFrom, plus foldable zero into any pointer destination
// (including pointer-to-function) when sourceExpr is provided.
inline bool productAssignOk(const type::Type& dest, const type::Type& source,
        const ast::Expression* sourceExpr = nullptr) {
    if (dest.canAssignFrom(source)) {
        return true;
    }
    return sourceExpr && dest.isPointer() && foldsToIntegerZero(*sourceExpr);
}

// Materialize a convert temp when dest is bool (0/1) or numeric width/kind changes.
inline void maybeSetConversion(ast::Expression* expr,
        const type::Type& targetType,
        SymbolTable& symbolTable,
        symbols::AnnotationStore& store) {
    if (!expr || !expr->hasResultSymbol(store)) {
        return;
    }
    const type::Type& from = expr->getResultSymbol(store)->getType();
    if (type::needsConversion(from, targetType)) {
        store.setConversion(expr, symbolTable.createTemporarySymbol(targetType));
    }
}

inline type::Type applyUsualArithmeticConversions(ast::Expression& left,
        ast::Expression& right,
        SymbolTable& symbolTable,
        symbols::AnnotationStore& store) {
    const type::Type resultType = type::usualArithmeticResult(
            left.getResultSymbol(store)->getType(),
            right.getResultSymbol(store)->getType());
    maybeSetConversion(&left, resultType, symbolTable, store);
    maybeSetConversion(&right, resultType, symbolTable, store);
    return resultType;
}

// C 6.3.1.1: single-operand integer promotions. Result type of << >> and of
// unary + - ~ is the promoted operand (6.5.7, 6.5.3.3).
inline type::Type applyIntegerPromotion(ast::Expression& expr,
        SymbolTable& symbolTable,
        symbols::AnnotationStore& store) {
    const type::Type promoted = type::integerPromote(expr.getResultSymbol(store)->getType());
    maybeSetConversion(&expr, promoted, symbolTable, store);
    return promoted;
}

} // namespace semantic_analyzer

#endif // SEMANTICANALYSISVISITOR_INTERNAL_H_
