#ifndef PARENTHESIZED_DECLARATOR_H_
#define PARENTHESIZED_DECLARATOR_H_

#include <memory>
#include <vector>

#include "ast/Declarator.h"
#include "ast/DirectDeclarator.h"
#include "ast/Pointer.h"

namespace ast {

// Direct declarator produced by '(' <declarator> ')'.
// Wraps a full declarator so pointer/function nesting inside parentheses is preserved.
class ParenthesizedDeclarator: public DirectDeclarator {
public:
    explicit ParenthesizedDeclarator(std::unique_ptr<Declarator> declarator);

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    type::Type getFundamentalType(std::vector<Pointer> indirection, const type::Type& baseType) override;

private:
    std::unique_ptr<Declarator> declarator;
};

} // namespace ast

#endif // PARENTHESIZED_DECLARATOR_H_
