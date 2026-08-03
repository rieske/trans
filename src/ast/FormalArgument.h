#ifndef _PARAM_DECL_NODE_H_
#define _PARAM_DECL_NODE_H_

#include <memory>
#include <optional>
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

    bool hasDeclarator() const;
    Declarator* getDeclarator() const;
    type::Type type() const;
    std::optional<type::Type> tryGetType() const;
    std::string getName() const;
    translation_unit::Context getDeclarationContext() const;

    bool isVoid() const;
    bool needsSemanticResolve() const;

private:
    DeclarationSpecifiers specifiers;
    std::unique_ptr<Declarator> declarator;
};

} // namespace ast

#endif // _PARAM_DECL_NODE_H_
