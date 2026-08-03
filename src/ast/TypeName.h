#ifndef AST_TYPE_NAME_H_
#define AST_TYPE_NAME_H_

#include <memory>
#include <optional>

#include "Declarator.h"
#include "TypeSpecifier.h"

namespace ast {

class AbstractSyntaxTreeVisitor;
class ParseEnvironment;

// Parsed <type_name>: specifier (incl. typeof) plus optional abstract declarator.
// dad is the sole abstract-declarator home (casts, sizeof, offsetof, compound
// literals, _Generic, and typeof(type_name) via TypeSpecifier holding TypeName).
// Parse-time clients use tryResolve (spec + dad).
struct TypeName {
    TypeSpecifier spec;
    std::unique_ptr<Declarator> dad;

    const std::string& getName() const { return spec.getName(); }

    // Apply dad to base and consume it. Without dad, returns base unchanged.
    type::Type applyDad(type::Type base);
    // SA path: visit dad first so array bounds fold, then apply.
    type::Type applyDad(type::Type base, AbstractSyntaxTreeVisitor& visitor);

    std::optional<type::Type> tryResolve(const ParseEnvironment& environment) const;
};

} // namespace ast

#endif
