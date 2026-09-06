#ifndef EXTERNALDECLARATION_H_
#define EXTERNALDECLARATION_H_

#include <memory>
#include <variant>

#include "Declaration.h"
#include "FunctionDefinition.h"

namespace ast {

class AbstractSyntaxTreeVisitor;

class ExternalDeclaration {
public:
    explicit ExternalDeclaration(std::unique_ptr<Declaration> declaration);
    explicit ExternalDeclaration(std::unique_ptr<FunctionDefinition> function);

    static ExternalDeclaration fromNode(std::unique_ptr<AbstractSyntaxTreeNode> node);

    const Declaration* asDeclaration() const;
    Declaration* asDeclaration();
    const FunctionDefinition* asFunctionDefinition() const;
    FunctionDefinition* asFunctionDefinition();

    void accept(AbstractSyntaxTreeVisitor& visitor) const;

private:
    std::variant<std::unique_ptr<Declaration>, std::unique_ptr<FunctionDefinition>> item_;
};

} // namespace ast

#endif
