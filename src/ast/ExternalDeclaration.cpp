#include "ExternalDeclaration.h"

#include "AbstractSyntaxTreeVisitor.h"

namespace ast {

ExternalDeclaration::ExternalDeclaration(std::unique_ptr<Declaration> declaration) :
        item_ { std::move(declaration) } {
}

ExternalDeclaration::ExternalDeclaration(std::unique_ptr<FunctionDefinition> function) :
        item_ { std::move(function) } {
}

ExternalDeclaration ExternalDeclaration::fromNode(std::unique_ptr<AbstractSyntaxTreeNode> node) {
    if (!node) {
        return ExternalDeclaration { std::unique_ptr<Declaration> {} };
    }
    if (auto* declaration = node->asDeclaration()) {
        node.release();
        return ExternalDeclaration { std::unique_ptr<Declaration> { declaration } };
    }
    if (auto* function = node->asFunctionDefinition()) {
        node.release();
        return ExternalDeclaration { std::unique_ptr<FunctionDefinition> { function } };
    }
    return ExternalDeclaration { std::unique_ptr<Declaration> {} };
}

const Declaration* ExternalDeclaration::asDeclaration() const {
    if (const auto* held = std::get_if<std::unique_ptr<Declaration>>(&item_)) {
        return held->get();
    }
    return nullptr;
}

Declaration* ExternalDeclaration::asDeclaration() {
    return const_cast<Declaration*>(
            static_cast<const ExternalDeclaration*>(this)->asDeclaration());
}

const FunctionDefinition* ExternalDeclaration::asFunctionDefinition() const {
    if (const auto* held = std::get_if<std::unique_ptr<FunctionDefinition>>(&item_)) {
        return held->get();
    }
    return nullptr;
}

FunctionDefinition* ExternalDeclaration::asFunctionDefinition() {
    return const_cast<FunctionDefinition*>(
            static_cast<const ExternalDeclaration*>(this)->asFunctionDefinition());
}

void ExternalDeclaration::accept(AbstractSyntaxTreeVisitor& visitor) const {
    std::visit([&](const auto& held) {
        if (held) {
            held->accept(visitor);
        }
    }, item_);
}

} // namespace ast
