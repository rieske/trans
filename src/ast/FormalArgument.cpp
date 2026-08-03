#include "FormalArgument.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "types/TypeQuery.h"

#include <stdexcept>

namespace ast {

FormalArgument::FormalArgument(DeclarationSpecifiers specifiers) :
        specifiers { specifiers }
{
}

FormalArgument::FormalArgument(DeclarationSpecifiers specifiers, std::unique_ptr<Declarator> declarator) :
        specifiers { specifiers },
        declarator { std::move(declarator) }
{
}

ast::FormalArgument::FormalArgument(FormalArgument&& rhs) :
        specifiers { std::move(rhs.specifiers) },
        declarator { std::move(rhs.declarator) }
{
}

void FormalArgument::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

void FormalArgument::visitSpecifiers(AbstractSyntaxTreeVisitor& visitor) {
    specifiers.accept(visitor);
}

void FormalArgument::visitDeclarator(AbstractSyntaxTreeVisitor& visitor) {
    if (declarator) {
        declarator->accept(visitor);
    }
}

bool FormalArgument::hasDeclarator() const {
    return declarator != nullptr;
}

Declarator* FormalArgument::getDeclarator() const {
    return declarator.get();
}

type::Type FormalArgument::type() const {
    auto baseType = specifiers.getResolvedType();
    type::Type type = baseType;
    if (declarator) {
        type = declarator->getFundamentalType(baseType);
    }
    return type::adjustedParameterType(std::move(type));
}

std::optional<type::Type> FormalArgument::tryGetType() const {
    try {
        return type();
    } catch (const std::invalid_argument&) {
        return std::nullopt;
    }
}

std::string FormalArgument::getName() const {
    if (!declarator) {
        return "";
    }
    return declarator->getName();
}

translation_unit::Context FormalArgument::getDeclarationContext() const {
    if (!declarator) {
        return translation_unit::Context { "", 0 };
    }
    return declarator->getContext();
}

bool FormalArgument::isVoid() const {
    if (declarator || specifiers.needsSemanticResolve()) {
        return false;
    }
    return specifiers.getResolvedType().isVoid();
}

bool FormalArgument::needsSemanticResolve() const {
    return specifiers.needsSemanticResolve();
}

} // namespace ast
