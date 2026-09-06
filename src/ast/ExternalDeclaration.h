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

    const Declaration* asDeclaration() const;
    const FunctionDefinition* asFunctionDefinition() const;

    void accept(AbstractSyntaxTreeVisitor& visitor) const;

private:
    std::variant<std::unique_ptr<Declaration>, std::unique_ptr<FunctionDefinition>> item_;
};

} // namespace ast

#endif
