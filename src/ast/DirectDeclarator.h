#ifndef DIRECT_DECLARATOR_H_
#define DIRECT_DECLARATOR_H_

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "ast/Pointer.h"
#include "translation_unit/Context.h"
#include "types/Type.h"

namespace ast {

class ArrayDeclarator;

class DirectDeclarator: public AbstractSyntaxTreeNode {
public:
    virtual ~DirectDeclarator() = default;

    std::string getName() const;

    translation_unit::Context getContext() const;

    virtual type::Type getFundamentalType(std::vector<Pointer> indirection, const type::Type& baseType) = 0;

    virtual void forEachArrayDeclarator(const std::function<void(ArrayDeclarator&)>&) {}

protected:
    DirectDeclarator(std::string name, const translation_unit::Context& context);

private:
    std::string name;

    translation_unit::Context context;

};

} // namespace ast

#endif // DIRECT_DECLARATOR_H_
