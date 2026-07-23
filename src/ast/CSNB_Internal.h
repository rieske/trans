#ifndef AST_CSNB_INTERNAL_H
#define AST_CSNB_INTERNAL_H

#include <functional>
#include <map>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "AbstractSyntaxTreeBuilderContext.h"
#include "parser/Grammar.h"

namespace ast {
namespace csnb {

using NodeCreator = std::function<void(AbstractSyntaxTreeBuilderContext&)>;
using NodeCreatorRegistry = std::unordered_map<int, std::map<std::vector<int>, NodeCreator>>;

// Shared no-op production builder used by many nonterminals.
inline void doNothing(AbstractSyntaxTreeBuilderContext&) {
}

// Clear error for grammar productions we intentionally do not lower yet.
inline NodeCreator notImplementedYet(const char* feature) {
    return [feature](AbstractSyntaxTreeBuilderContext&) {
        throw std::runtime_error { std::string(feature) + " is not implemented yet" };
    };
}

// Discard a pending __typeof__ operand when a type_name was __typeof__(expr).
void discardTypeofIfPresent(AbstractSyntaxTreeBuilderContext& context, const TypeSpecifier& typeSpec);

// Free builder used for continue/break; class method forwards here.
void loopJumpStatement(AbstractSyntaxTreeBuilderContext& context);

void registerTypeProductions(const parser::Grammar& grammar, NodeCreatorRegistry& nodeCreatorRegistry);
void registerDeclaratorProductions(const parser::Grammar& grammar, NodeCreatorRegistry& nodeCreatorRegistry);
void registerExpressionProductions(const parser::Grammar& grammar, NodeCreatorRegistry& nodeCreatorRegistry);
void registerInitializerProductions(const parser::Grammar& grammar, NodeCreatorRegistry& nodeCreatorRegistry);
void registerStatementProductions(const parser::Grammar& grammar, NodeCreatorRegistry& nodeCreatorRegistry);
void registerBuiltinProductions(const parser::Grammar& grammar, NodeCreatorRegistry& nodeCreatorRegistry);

} // namespace csnb
} // namespace ast

#endif // AST_CSNB_INTERNAL_H
