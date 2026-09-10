#include "ExternalDeclaration.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

ExternalDeclaration::ExternalDeclaration(std::unique_ptr<Declaration> declaration) :
        item_ { std::move(declaration) } {
}

ExternalDeclaration::ExternalDeclaration(std::unique_ptr<FunctionDefinition> function) :
        item_ { std::move(function) } {
}

void ExternalDeclaration::accept(AbstractSyntaxTreeVisitor& visitor) const {
    std::visit([&](const auto& held) {
        if (held) {
            held->accept(visitor);
        }
    }, item_);
}

} // namespace ast
