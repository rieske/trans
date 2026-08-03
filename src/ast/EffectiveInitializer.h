#ifndef AST_EFFECTIVE_INITIALIZER_H_
#define AST_EFFECTIVE_INITIALIZER_H_

#include <string>

#include "ast/Expression.h"
#include "ast/InitializerListExpression.h"
#include "types/Type.h"

namespace ast {

// Non-aggregate brace { e } peels to e (shared SA/CG policy). Aggregates and
// non-lists are returned unchanged. When excessError is non-null and the list
// has more than one element, sets *excessError to a diagnostic message.
inline Expression* effectiveInitializer(const type::Type& objectType,
        Expression* init,
        std::string* excessError = nullptr) {
    if (!init) {
        return init;
    }
    auto* list = dynamic_cast<InitializerListExpression*>(init);
    if (!list || objectType.isAggregate()) {
        return init;
    }
    const auto& elements = list->getElements();
    if (elements.size() > 1 && excessError) {
        *excessError = "excess elements in scalar initializer";
    }
    if (!elements.empty() && elements.front().value) {
        return elements.front().value.get();
    }
    return init;
}

} // namespace ast

#endif // AST_EFFECTIVE_INITIALIZER_H_
