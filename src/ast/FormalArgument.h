#ifndef _PARAM_DECL_NODE_H_
#define _PARAM_DECL_NODE_H_

#include <memory>
#include <string>

#include "ast/DeclarationSpecifiers.h"
#include "ast/Declarator.h"

namespace ast {

class FormalArgument: public AbstractSyntaxTreeNode {
public:
    FormalArgument(DeclarationSpecifiers specifiers);
    FormalArgument(DeclarationSpecifiers specifiers, std::unique_ptr<Declarator> declarator);
    FormalArgument(FormalArgument&& rhs);
    virtual ~FormalArgument() = default;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    void visitSpecifiers(AbstractSyntaxTreeVisitor& visitor);
    void visitDeclarator(AbstractSyntaxTreeVisitor& visitor);

    type::Type getType() const;
    std::string getName() const;
    translation_unit::Context getDeclarationContext() const;

private:
    DeclarationSpecifiers specifiers;
    std::unique_ptr<Declarator> declarator;
};

} // namespace ast

#endif // _PARAM_DECL_NODE_H_
