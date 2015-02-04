#ifndef _PARAM_DECL_NODE_H_
#define _PARAM_DECL_NODE_H_

#include <memory>
#include <string>

#include "DeclarationSpecifiers.h"

namespace ast {
class AbstractSyntaxTreeVisitor;
} /* namespace ast */

namespace translation_unit {
class Context;
} /* namespace translation_unit */

namespace ast {

class Type;
class Declarator;

class FormalArgument: public AbstractSyntaxTreeNode {
public:
    FormalArgument(DeclarationSpecifiers specifiers);
    FormalArgument(DeclarationSpecifiers specifiers, std::unique_ptr<Declarator> declarator);
    FormalArgument(FormalArgument&& rhs);
    virtual ~FormalArgument() = default;

    static const std::string ID;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    void visitSpecifiers(AbstractSyntaxTreeVisitor& visitor);
    void visitDeclarator(AbstractSyntaxTreeVisitor& visitor);

    Type getType() const;
    std::string getName() const;
    translation_unit::Context getDeclarationContext() const;

private:
    DeclarationSpecifiers specifiers;
    std::unique_ptr<Declarator> declarator;
};

}

#endif // _PARAM_DECL_NODE_H_
