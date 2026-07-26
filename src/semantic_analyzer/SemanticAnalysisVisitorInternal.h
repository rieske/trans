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
#include "translation_unit/Context.h"
#include "types/Type.h"
#include "types/TypeQuery.h"
#include "util/Logger.h"
#include "util/LogManager.h"

namespace semantic_analyzer {

inline const translation_unit::Context& externalContext() {
    static const translation_unit::Context ctx { "external", 0 };
    return ctx;
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

// Source type for assignment/init/return into `dest`.
// Dual-type aggregate addresses use the pointer value when dest is a pointer
// (array-row decay); structure destinations still see the aggregate expression type.
inline type::Type assignSourceType(const ast::Expression& expr, const type::Type& dest,
        symbols::AnnotationStore& store) {
    if (expr.holdsAggregateAddress() && dest.isPointer()) {
        return expr.getResultSymbol(store)->getType();
    }
    return expr.getType();
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
            r.error = "base of ‘->’ is not a pointer to structure";
            return r;
        }
        r.structureType = baseType.dereference();
        if (!r.structureType.isStructure()) {
            r.error = "base of ‘->’ is not a pointer to structure";
            return r;
        }
        r.addressIsPointer = true;
        r.ok = true;
        return r;
    }

    // Dot: aggregate-address form already holds the object address in the result.
    if (!baseType.isStructure()) {
        r.error = "request for member in non-structure type";
        return r;
    }
    r.structureType = baseType;
    r.addressIsPointer = base.holdsAggregateAddress();
    r.ok = true;
    return r;
}

} // namespace semantic_analyzer

#endif // SEMANTICANALYSISVISITOR_INTERNAL_H_
