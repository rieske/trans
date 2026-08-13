#include "ScalarBrace.h"

#include "ast/InitializerListExpression.h"

namespace semantic_analyzer {

ScalarBraceOutcome scalarBraceFromValue(ast::Expression* value, ast::Expression*& leaf) {
    if (!value) {
        return ScalarBraceOutcome::Empty;
    }
    leaf = value;
    auto* nested = dynamic_cast<ast::InitializerListExpression*>(value);
    while (nested) {
        if (nested->getElements().size() > 1) {
            return ScalarBraceOutcome::Excess;
        }
        if (nested->getElements().empty() || !nested->getElements().front().value) {
            return ScalarBraceOutcome::Empty;
        }
        leaf = nested->getElements().front().value.get();
        nested = dynamic_cast<ast::InitializerListExpression*>(leaf);
    }
    return ScalarBraceOutcome::Leaf;
}

ScalarBraceOutcome scalarBraceFromList(const ast::InitializerListExpression* list,
        ast::Expression*& leaf) {
    if (!list || list->getElements().empty()) {
        return ScalarBraceOutcome::Empty;
    }
    if (list->getElements().size() > 1) {
        return ScalarBraceOutcome::Excess;
    }
    return scalarBraceFromValue(list->getElements().front().value.get(), leaf);
}

} // namespace semantic_analyzer
