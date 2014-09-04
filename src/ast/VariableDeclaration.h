#ifndef _VAR_DECL_NODE_H_
#define _VAR_DECL_NODE_H_

#include <memory>
#include <string>

#include "TypeSpecifier.h"

namespace ast {

class DeclarationList;
class Expression;
class TerminalSymbol;

class VariableDeclaration: public AbstractSyntaxTreeNode {
public:
    VariableDeclaration(TypeSpecifier typeSpecifier, std::unique_ptr<DeclarationList> declarationList);

    static const std::string ID;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    TypeSpecifier declaredType;
    const std::unique_ptr<DeclarationList> declaredVariables;
private:

};

}

#endif // _VAR_DECL_NODE_H_
