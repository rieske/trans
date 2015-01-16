#ifndef _PARAM_DECL_NODE_H_
#define _PARAM_DECL_NODE_H_

#include <memory>
#include <string>

#include "TypeSpecifier.h"

namespace translation_unit {
class Context;
} /* namespace translation_unit */

namespace ast {

class Type;
class Declaration;
class TerminalSymbol;

class FormalArgument: public AbstractSyntaxTreeNode {
public:
    FormalArgument(TypeSpecifier typeSpecifier, std::unique_ptr<Declaration> declaration);
    virtual ~FormalArgument();

    static const std::string ID;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    void visitTypeSpecifier(AbstractSyntaxTreeVisitor& visitor);
    void visitDeclaration(AbstractSyntaxTreeVisitor& visitor);

    Type getType() const;
    std::string getName() const;
    translation_unit::Context getDeclarationContext() const;

private:
    TypeSpecifier type;
    const std::unique_ptr<Declaration> declaration;
};

}

#endif // _PARAM_DECL_NODE_H_
