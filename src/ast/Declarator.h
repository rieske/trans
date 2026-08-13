#ifndef _DECLARATOR_H_
#define _DECLARATOR_H_

#include <memory>
#include <string>
#include <vector>

#include "ast/Pointer.h"
#include "ast/DirectDeclarator.h"

namespace ast {

class AbstractSyntaxTreeVisitor;

class Declarator: public AbstractSyntaxTreeNode {
public:
    Declarator(std::unique_ptr<DirectDeclarator> declarator, std::vector<Pointer> indirection = {});
    virtual ~Declarator() = default;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    void visitChildren(AbstractSyntaxTreeVisitor& visitor);

    std::string getName() const;
    translation_unit::Context getContext() const;

    type::Type getFundamentalType(const type::Type& baseType);
    // Outer pointers (e.g. from `T *(a[N])`) apply before this declarator's own *.
    type::Type getFundamentalType(std::vector<Pointer> outerIndirection, const type::Type& baseType);

    void forEachArrayDeclarator(const std::function<void(ArrayDeclarator&)>& fn);

private:
    std::unique_ptr<DirectDeclarator> declarator;
    std::vector<Pointer> indirection;
};

} // namespace ast

#endif // _DECLARATOR_H_
