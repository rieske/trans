#ifndef FUNCTIONDECLARATION_H_
#define FUNCTIONDECLARATION_H_

#include <memory>
#include <vector>

#include "DirectDeclarator.h"
#include "FormalArgument.h"

namespace ast {

using FormalArguments = std::vector<FormalArgument>;

class FunctionDeclarator: public DirectDeclarator {
public:
    FunctionDeclarator(std::unique_ptr<DirectDeclarator> declarator);
    FunctionDeclarator(std::unique_ptr<DirectDeclarator> declarator, FormalArguments formalArguments);
    virtual ~FunctionDeclarator() = default;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    void visitFormalArguments(AbstractSyntaxTreeVisitor& visitor);

    const FormalArguments& getFormalArguments() const;

    type::Type getFundamentalType(std::vector<Pointer> indirection, const type::Type& baseType) override;

private:
    FormalArguments formalArguments;
};

} // namespace ast

#endif // FUNCTIONDECLARATION_H_
