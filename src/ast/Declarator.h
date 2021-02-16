#ifndef _DECLARATOR_H_
#define _DECLARATOR_H_

#include <memory>
#include <string>
#include <vector>

#include "ast/Pointer.h"
#include "ast/DirectDeclarator.h"

namespace ast {

class Declarator: public AbstractSyntaxTreeNode {
public:
    Declarator(std::unique_ptr<DirectDeclarator> declarator, std::vector<Pointer> indirection = {});
    virtual ~Declarator() = default;

    const static std::string ID;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    void visitChildren(AbstractSyntaxTreeVisitor& visitor);

    std::string getName() const;
    translation_unit::Context getContext() const;

    std::unique_ptr<FundamentalType> getFundamentalType(const FundamentalType& baseType);

private:
    std::unique_ptr<DirectDeclarator> declarator;
    std::vector<Pointer> indirection;
};

} // namespace ast

#endif // _DECLARATOR_H_
