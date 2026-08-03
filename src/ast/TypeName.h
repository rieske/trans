#ifndef AST_TYPE_NAME_H_
#define AST_TYPE_NAME_H_

#include <memory>
#include <optional>

#include "Declarator.h"
#include "TypeSpecifier.h"

namespace ast {

class ParseEnvironment;

// Parsed <type_name>: specifier (incl. typeof) plus optional abstract declarator.
// dad stays on TypeName until SA resolveTypeName (casts, sizeof, offsetof,
// compound literals, _Generic associations). Exception: typeSpecTypeofTypeName
// consumes the TypeName into a TypeSpecifier (master deferAbstractDeclarator).
// Parse-time clients (typeof, types_compatible_p) use tryResolve (spec + dad).
struct TypeName {
    TypeSpecifier spec;
    std::unique_ptr<Declarator> dad;

    const std::string& getName() const { return spec.getName(); }

    std::optional<type::Type> tryResolve(const ParseEnvironment& environment) const {
        TypeSpecifier resolved = spec;
        if (!resolved.resolveTypeofAtParseTime(environment) || !resolved.hasType()) {
            return std::nullopt;
        }
        type::Type t = resolved.getType();
        if (dad) {
            t = dad->getFundamentalType(t);
        }
        return t;
    }
};

} // namespace ast

#endif
