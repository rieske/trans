#include "FormalArgument.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "types/TypeQuery.h"

namespace ast {

FormalArgument::FormalArgument(DeclarationSpecifiers specifiers) :
        specifiers { std::move(specifiers) }
{
}

FormalArgument::FormalArgument(DeclarationSpecifiers specifiers, std::unique_ptr<Declarator> declarator) :
        specifiers { std::move(specifiers) },
        declarator { std::move(declarator) }
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

type::Type FormalArgument::declaredType() const {
    auto baseType = specifiers.getResolvedType();
    if (declarator) {
        return declarator->getFundamentalType(baseType);
    }
    return baseType;
}

type::Type FormalArgument::adjustedType() const {
    return type::adjustedParameterType(declaredType());
}

const char* FormalArgument::arrayConstraintError() const {
    if (declarator && !declarator->hasArrayDeclarator()) {
        return nullptr;
    }
    return type::arrayTypeError(declaredType());
}

void FormalArgument::forEachFormalArgument(const std::function<void(const FormalArgument&)>& fn) const {
    if (declarator) {
        declarator->forEachFormalArgument(fn);
    }
}

const std::string& FormalArgument::getName() const {
    static const std::string empty;
    return declarator ? declarator->getName() : empty;
}

translation_unit::Context FormalArgument::getDeclarationContext() const {
    if (declarator) {
        return declarator->getContext();
    }
    return translation_unit::Context { "", 0 };
}

bool FormalArgument::isVoid() const {
    return !declarator && specifiers.getResolvedType().isVoid();
}

bool FormalArgument::needsSemanticResolve() const {
    return specifiers.needsSemanticResolve();
}

} // namespace ast

