#ifndef NESTEDDECLARATOR_H_
#define NESTEDDECLARATOR_H_

#include <memory>

#include "DirectDeclarator.h"
#include "Declarator.h"

namespace ast {

// Parenthesized declarator: ( declarator ) as a direct_declarator.
class NestedDeclarator: public DirectDeclarator {
public:
    NestedDeclarator(std::unique_ptr<Declarator> declarator);
    virtual ~NestedDeclarator() = default;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    void visitChildren(AbstractSyntaxTreeVisitor& visitor);

    type::Type getFundamentalType(std::vector<Pointer> indirection, const type::Type& baseType) override;

    Declarator& getDeclarator() const;

private:
    std::unique_ptr<Declarator> declarator;
};

} // namespace ast

#endif // NESTEDDECLARATOR_H_
