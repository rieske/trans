#ifndef _FUNC_DECL_NODE_H_
#define _FUNC_DECL_NODE_H_

#include <memory>
#include <string>
#include <vector>

#include "ast/DeclarationSpecifiers.h"
#include "ast/Declarator.h"

namespace ast {

class FunctionDefinition: public AbstractSyntaxTreeNode {
public:
    FunctionDefinition(DeclarationSpecifiers returnType, std::unique_ptr<Declarator> declarator, std::unique_ptr<AbstractSyntaxTreeNode> body);
    virtual ~FunctionDefinition() = default;

    void accept(AbstractSyntaxTreeVisitor& visitor) override;
    void visitReturnType(AbstractSyntaxTreeVisitor& visitor);
    void visitDeclarator(AbstractSyntaxTreeVisitor& visitor);
    void visitBody(AbstractSyntaxTreeVisitor& visitor);
    void visitBodyChildren(AbstractSyntaxTreeVisitor& visitor);

    std::string getName() const;
    const DeclarationSpecifiers& getReturnTypeSpecifiers() const;
    type::Type getDeclaratorType(const type::Type& baseType) const;
    translation_unit::Context getDeclaratorContext() const;
    // Innermost FunctionDeclarator names. Throws if the declarator is not a function.
    std::vector<std::string> definedFunctionParameterNames() const;

private:
    DeclarationSpecifiers returnType;
    std::unique_ptr<Declarator> declarator;
    std::unique_ptr<AbstractSyntaxTreeNode> body;
};

} // namespace ast

#endif // _FUNC_DECL_NODE_H_
