#ifndef DIRECT_DECLARATOR_H_
#define DIRECT_DECLARATOR_H_

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "ast/AbstractSyntaxTreeNode.h"
#include "ast/Pointer.h"
#include "types/Type.h"
#include "translation_unit/Context.h"

namespace ast {

class Expression;

class DirectDeclarator: public AbstractSyntaxTreeNode {
public:
    virtual ~DirectDeclarator() = default;

    std::string getName() const;

    translation_unit::Context getContext() const;

    virtual type::Type getFundamentalType(std::vector<Pointer> indirection, const type::Type& baseType) = 0;

    // Fold sizeof in array bounds via callback, then setArraySize when constant.
    // Default: no array bounds on this node (Identifier and similar leaves).
    virtual void foldArrayBoundSizeofs(const std::function<void(Expression*)>& foldSizeof);

    // True if this declarator (or a nested one) has an array dimension.
    virtual bool hasArrayDeclarator() const;

protected:
    DirectDeclarator(std::string name, const translation_unit::Context& context);

private:
    std::string name;

    translation_unit::Context context;

};

} // namespace ast

#endif // DIRECT_DECLARATOR_H_
