#include "ExternalDeclaration.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

ExternalDeclaration::ExternalDeclaration(std::unique_ptr<Declaration> declaration) :
        item_ { std::move(declaration) } {
}

ExternalDeclaration::ExternalDeclaration(std::unique_ptr<FunctionDefinition> function) :
        item_ { std::move(function) } {
}

const Declaration* ExternalDeclaration::asDeclaration() const {
    if (const auto* held = std::get_if<std::unique_ptr<Declaration>>(&item_)) {
        return held->get();
    }
    return nullptr;
}

const FunctionDefinition* ExternalDeclaration::asFunctionDefinition() const {
    if (const auto* held = std::get_if<std::unique_ptr<FunctionDefinition>>(&item_)) {
        return held->get();
    }
    return nullptr;
}

void ExternalDeclaration::accept(AbstractSyntaxTreeVisitor& visitor) const {
    std::visit([&](const auto& held) {
        if (held) {
            held->accept(visitor);
        }
    }, item_);
}

} // namespace ast
