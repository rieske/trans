#ifndef _PARAM_DECL_NODE_H_
#define _PARAM_DECL_NODE_H_

#include <functional>
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

    type::Type declaredType() const;
    type::Type getType() const;
    const char* arrayConstraintError() const;
    void forEachFormalArgument(const std::function<void(const FormalArgument&)>& fn) const;
    DeclarationSpecifiers& getSpecifiers() { return specifiers; }
    const DeclarationSpecifiers& getSpecifiers() const { return specifiers; }
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
