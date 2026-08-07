#ifndef _DECLARATOR_H_
#define _DECLARATOR_H_

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "ast/Pointer.h"
#include "ast/DirectDeclarator.h"
#include "ast/Expression.h"

namespace ast {

class Declarator: public AbstractSyntaxTreeNode {
public:
    Declarator(std::unique_ptr<DirectDeclarator> declarator, std::vector<Pointer> indirection = {});
    virtual ~Declarator() = default;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    void visitChildren(AbstractSyntaxTreeVisitor& visitor);

    std::string getName() const;
    translation_unit::Context getContext() const;

    type::Type getFundamentalType(const type::Type& baseType);

    // Fold sizeof in array bounds via callback, then setArraySize when constant.
    // Used by the late struct-member array-bound pass (ARRAY_SIZE of file-scope globals).
    void foldArrayBoundSizeofs(const std::function<void(Expression*)>& foldSizeof);

    // True if this declarator has an array dimension (T a[N] or T *a[N], nested).
    bool hasArrayDeclarator() const;

private:
    std::unique_ptr<DirectDeclarator> declarator;
    std::vector<Pointer> indirection;
};

} // namespace ast

#endif // _DECLARATOR_H_
