#ifndef DIRECT_DECLARATOR_H_
#define DIRECT_DECLARATOR_H_

#include <functional>
#include <string>
#include <vector>

#include "ast/AbstractSyntaxTreeNode.h"
#include "ast/Pointer.h"
#include "types/Type.h"
#include "translation_unit/Context.h"

namespace ast {

class ArrayDeclarator;
class FunctionDeclarator;

class DirectDeclarator: public AbstractSyntaxTreeNode {
public:
    virtual ~DirectDeclarator() = default;

    std::string getName() const;

    translation_unit::Context getContext() const;

    virtual type::Type getFundamentalType(std::vector<Pointer> indirection, const type::Type& baseType) const = 0;

    virtual void forEachArrayDeclarator(const std::function<void(ArrayDeclarator&)>&) {}

    // FunctionDeclarator closest to the identifier, or nullptr if this is not a function.
    virtual const FunctionDeclarator* innermostFunctionDeclarator() const { return nullptr; }

protected:
    DirectDeclarator(std::string name, const translation_unit::Context& context);

private:
    std::string name;

    translation_unit::Context context;

};

} // namespace ast

#endif // DIRECT_DECLARATOR_H_
