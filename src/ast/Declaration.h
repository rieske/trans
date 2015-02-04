#ifndef DECLARATION_H_
#define DECLARATION_H_

#include <memory>
#include <string>
#include <vector>

#include "DeclarationSpecifiers.h"

namespace ast {

class InitializedDeclarator;

class Declaration: public AbstractSyntaxTreeNode {
public:
    Declaration(DeclarationSpecifiers declarationSpecifiers, std::vector<std::unique_ptr<InitializedDeclarator>> declarators = { });
    virtual ~Declaration() = default;

    void accept(AbstractSyntaxTreeVisitor& visitor);
    void visitChildren(AbstractSyntaxTreeVisitor& visitor);

    static const std::string ID;

private:
    DeclarationSpecifiers declarationSpecifiers;
    std::vector<std::unique_ptr<InitializedDeclarator>> declarators;
};

} /* namespace ast */

#endif /* DECLARATION_H_ */
