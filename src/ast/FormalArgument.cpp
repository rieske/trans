#include "FormalArgument.h"

#include "AbstractSyntaxTreeVisitor.h"

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

type::Type FormalArgument::getType() const {
    auto baseType = specifiers.getResolvedType();
    type::Type type = baseType;
    if (declarator) {
        type = declarator->getFundamentalType(baseType);
    }
    // C adjusts array parameters to pointers to the element type
    // (`int f(int[])` / `__builtin_va_list ap`).
    if (type.isArray()) {
        return type::pointer(type.getElementType());
    }
    return type;
}

std::string FormalArgument::getName() const {
    return declarator ? declarator->getName() : "";
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

} // namespace ast

