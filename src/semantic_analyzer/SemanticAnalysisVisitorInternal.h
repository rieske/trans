#ifndef SEMANTICANALYSISVISITOR_INTERNAL_H_
#define SEMANTICANALYSISVISITOR_INTERNAL_H_

#include "SemanticAnalysisVisitor.h"

#include <algorithm>
#include <cctype>
#include <limits>
#include <stdexcept>
#include <string>
#include <utility>

#include "ast/Expression.h"
#include "types/Type.h"
#include "types/TypeQuery.h"
#include "util/Logger.h"
#include "util/LogManager.h"

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

// Source type for assignment/init/return into `dest`.
// Dual-type aggregate addresses use the pointer value when dest is a pointer
// (array-row decay); structure destinations still see the aggregate expression type.
inline type::Type assignSourceType(const ast::Expression& expr, const type::Type& dest,
        symbols::AnnotationStore& store) {
    // AggregateAddress form always has a Result after successful SA (set with the form).
    if (expr.holdsAggregateAddress() && dest.isPointer()) {
        return expr.getResultSymbol(store)->getType();
    }
    return expr.getType();
}

// Materialize a convert temp when dest is bool (0/1) or float/int width changes.
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

struct MemberBaseResolution {
    type::Type structureType { type::voidType() };
    bool addressIsPointer { false };
    bool ok { false };
    const char* error { nullptr };
};

// Resolve `.` / `->` base using expression type + ValueForm (set by SA).
// Does not re-decode dual-type from raw value-type pairs.
inline MemberBaseResolution resolveMemberBase(const ast::Expression& base, bool isArrow) {
    MemberBaseResolution r;
    type::Type baseType = base.getType();

    if (isArrow) {
        if (!baseType.isPointer()) {
            r.error = "base of '->' is not a pointer to structure or union";
            return r;
        }
        r.structureType = baseType.dereference();
        if (!r.structureType.isRecord()) {
            r.error = "base of '->' is not a pointer to structure or union";
            return r;
        }
        r.addressIsPointer = true;
        r.ok = true;
        return r;
    }

    // Dot: aggregate-address form already holds the object address in the result.
    if (!baseType.isRecord()) {
        r.error = "request for member in non-structure or non-union type";
        return r;
    }
    r.structureType = baseType;
    r.addressIsPointer = base.holdsAggregateAddress();
    r.ok = true;
    return r;
}

} // namespace semantic_analyzer

#endif // SEMANTICANALYSISVISITOR_INTERNAL_H_
