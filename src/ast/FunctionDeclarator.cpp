#include "FunctionDeclarator.h"

#include <algorithm>

#include "../translation_unit/Context.h"
#include "AbstractSyntaxTreeVisitor.h"
#include "Declarator.h"
#include "FormalArgument.h"
#include "Pointer.h"
#include "types/FunctionType.h"
#include "types/FundamentalType.h"
#include "types/PointerType.h"

namespace ast {

FunctionDeclarator::FunctionDeclarator(std::unique_ptr<DirectDeclarator> declarator) :
        DirectDeclarator(declarator->getName(), declarator->getContext()),
        formalArguments { std::move(formalArguments) }
{
}

FunctionDeclarator::FunctionDeclarator(std::unique_ptr<DirectDeclarator> declarator, FormalArguments formalArguments) :
        DirectDeclarator(declarator->getName(), declarator->getContext()),
        formalArguments { std::move(formalArguments) }
{
}

void FunctionDeclarator::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

void FunctionDeclarator::visitFormalArguments(AbstractSyntaxTreeVisitor& visitor) {
    for (auto& argument : formalArguments) {
        argument.accept(visitor);
    }
}

const FormalArguments& ast::FunctionDeclarator::getFormalArguments() const {
    return formalArguments;
}

std::unique_ptr<FundamentalType> FunctionDeclarator::getFundamentalType(std::vector<Pointer> indirection, const FundamentalType& returnType) {
    std::vector<std::unique_ptr<FundamentalType>> argumentTypes;
    for (const auto& argument : formalArguments) {
        argumentTypes.push_back(std::unique_ptr<FundamentalType> { argument.getType()->clone() });
    }
    std::unique_ptr<FundamentalType> type = std::make_unique<FunctionType>(std::unique_ptr<FundamentalType> { returnType.clone() }, std::move(argumentTypes));
    for (Pointer pointer : indirection) {
        std::unique_ptr<FundamentalType> pointerType = std::make_unique<PointerType>(std::move(type), pointer.getQualifiers());
        type = std::move(pointerType);
    }
    return type;
}

} /* namespace ast */
