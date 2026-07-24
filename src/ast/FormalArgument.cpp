#include "FormalArgument.h"

#include "AbstractSyntaxTreeVisitor.h"
#include "translation_unit/Context.h"

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

type::Type FormalArgument::getType() const {
    auto baseType = specifiers.getResolvedType();
    type::Type type = baseType;
    if (declarator) {
        type = declarator->getFundamentalType(baseType);
    }
    // C adjusts array parameters to pointers to the element type
    // (named or abstract: `int f(int[])` / `int f(int a[])`).
    if (type.isArray()) {
        return type::pointer(type.getElementType());
    }
    // C adjusts function parameters to pointers to the function type
    // (e.g. typedef int cb(int); void f(cb); void f(cb fn) -> int (*)(int)).
    if (type.isFunction()) {
        return type::pointer(type);
    }
    return type;
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
    return !declarator && specifiers.getResolvedType().isVoid();
}

} // namespace ast

