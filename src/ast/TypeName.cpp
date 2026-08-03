#include "TypeName.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "ParseEnvironment.h"

namespace ast {

type::Type TypeName::applyDad(type::Type base) {
    if (!dad) {
        return base;
    }
    type::Type applied = dad->getFundamentalType(base);
    dad.reset();
    spec = TypeSpecifier { applied, "" };
    return applied;
}

type::Type TypeName::applyDad(type::Type base, AbstractSyntaxTreeVisitor& visitor) {
    if (dad) {
        dad->accept(visitor);
    }
    return applyDad(std::move(base));
}

std::optional<type::Type> TypeName::tryResolve(const ParseEnvironment& environment) const {
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

} // namespace ast
