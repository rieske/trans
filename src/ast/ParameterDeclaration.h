#ifndef _PARAM_DECL_NODE_H_
#define _PARAM_DECL_NODE_H_

#include <memory>
#include <string>

#include "TypeSpecifier.h"

namespace ast {

class Type;
class Declaration;
class TerminalSymbol;

class ParameterDeclaration: public AbstractSyntaxTreeNode {
public:
    ParameterDeclaration(TypeSpecifier typeSpecifier, std::unique_ptr<Declaration> declaration);
    virtual ~ParameterDeclaration();

    static const std::string ID;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;

    Type getTypeInfo() const;

    TypeSpecifier type;
    const std::unique_ptr<Declaration> declaration;
};

}

#endif // _PARAM_DECL_NODE_H_
