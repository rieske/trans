#ifndef DECLARATION_H_
#define DECLARATION_H_

#include <memory>
#include <string>
#include <vector>

#include "DeclarationSpecifiers.h"

namespace ast {

class Declarator;

class Declaration: public AbstractSyntaxTreeNode {
public:
    Declaration(DeclarationSpecifiers declarationSpecifiers, std::vector<std::unique_ptr<Declarator>> declarators = { });
    virtual ~Declaration();

    void accept(AbstractSyntaxTreeVisitor& visitor);

    static const std::string ID;

private:
    DeclarationSpecifiers declarationSpecifiers;
    std::vector<std::unique_ptr<Declarator>> declarators;
};

} /* namespace ast */

#endif /* DECLARATION_H_ */
