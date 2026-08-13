#ifndef SCALAR_BRACE_H_
#define SCALAR_BRACE_H_

namespace ast {
class Expression;
class InitializerListExpression;
}

namespace semantic_analyzer {

// Pure walk of nested single-element braces around a scalar initializer.
// Callers map outcomes to diagnostics or sink actions (no side effects here).
enum class ScalarBraceOutcome {
    Leaf,
    Empty,
    Excess,
};

// Top-level brace list as a scalar initializer: { x }, {{ x }}, {1,2}, {}.
ScalarBraceOutcome scalarBraceFromList(const ast::InitializerListExpression* list,
        ast::Expression*& leaf);

// Value that may itself be nested braces (aggregate scalar slot path).
ScalarBraceOutcome scalarBraceFromValue(ast::Expression* value, ast::Expression*& leaf);

// SA mapping for Empty only (aggregate walk uses onUnwritten instead).
enum class ScalarBraceEmpty {
    Silent,
    RequireValue,
};

} // namespace semantic_analyzer

#endif
