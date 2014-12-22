#ifndef _PARAM_DECL_NODE_H_
#define _PARAM_DECL_NODE_H_

#include <memory>
#include <string>

#include "TypeSpecifier.h"

namespace code_generator {
class ValueEntry;
} /* namespace code_generator */

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

    Type getType() const;

    void setResultHolder(code_generator::ValueEntry* resultHolder);
    code_generator::ValueEntry* getResultHolder() const;

    TypeSpecifier type;
    const std::unique_ptr<Declaration> declaration;

private:
    code_generator::ValueEntry* resultHolder { nullptr };
};

}

#endif // _PARAM_DECL_NODE_H_
