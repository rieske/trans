#ifndef DECLARATION_H_
#define DECLARATION_H_

#include <memory>
#include <string>
#include <vector>

#include "DeclarationSpecifiers.h"
#include "InitializedDeclarator.h"

namespace ast {

class Declaration: public AbstractSyntaxTreeNode {
public:
    Declaration(DeclarationSpecifiers declarationSpecifiers, std::vector<std::unique_ptr<InitializedDeclarator>> declarators = { });
    virtual ~Declaration() = default;

    void accept(AbstractSyntaxTreeVisitor& visitor);
    void visitChildren(AbstractSyntaxTreeVisitor& visitor);

    DeclarationSpecifiers getDeclarationSpecifiers() const;
    const std::vector<std::unique_ptr<InitializedDeclarator>>& getDeclarators() const;

    static const std::string ID;

private:
    DeclarationSpecifiers declarationSpecifiers;
    std::vector<std::unique_ptr<InitializedDeclarator>> declarators;
};

} // namespace ast

#endif // DECLARATION_H_
