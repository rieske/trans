#include "Declarator.h"

#include "translation_unit/Context.h"
#include "AbstractSyntaxTreeVisitor.h"
#include "ArrayDeclarator.h"
#include "DirectDeclarator.h"
#include "FunctionDeclarator.h"
#include "NestedDeclarator.h"

namespace ast {

Declarator::Declarator(std::unique_ptr<DirectDeclarator> declarator, std::vector<Pointer> indirection) :
        declarator { std::move(declarator) },
        indirection { std::move(indirection) }
{
}

void Declarator::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

void Declarator::visitChildren(AbstractSyntaxTreeVisitor& visitor) {
    for (auto& pointer : indirection) {
        pointer.accept(visitor);
    }
    declarator->accept(visitor);
}

std::string Declarator::getName() const {
    return declarator->getName();
}

translation_unit::Context Declarator::getContext() const {
    return declarator->getContext();
}

type::Type ast::Declarator::getFundamentalType(const type::Type& baseType) const {
    return declarator->getFundamentalType(indirection, baseType);
}

void Declarator::forEachArrayDeclarator(const std::function<void(ArrayDeclarator&)>& fn) {
    declarator->forEachArrayDeclarator(fn);
}

void Declarator::foldArrayBoundSizeofs(const std::function<void(Expression*)>& foldSizeof) {
    declarator->foldArrayBoundSizeofs(foldSizeof);
}

bool Declarator::hasArrayDeclarator() const {
    return declarator->hasArrayDeclarator();
}

std::vector<std::string> Declarator::formalParameterNames() const {
    const DirectDeclarator* node = declarator.get();
    while (node != nullptr) {
        if (const auto* function = dynamic_cast<const FunctionDeclarator*>(node)) {
            std::vector<std::string> names;
            names.reserve(function->getFormalArguments().size());
            for (const auto& argument : function->getFormalArguments()) {
                names.push_back(argument.getName());
            }
            return names;
        }
        if (const auto* array = dynamic_cast<const ArrayDeclarator*>(node)) {
            node = &array->getBaseDeclarator();
            continue;
        }
        if (const auto* nested = dynamic_cast<const NestedDeclarator*>(node)) {
            return nested->getDeclarator().formalParameterNames();
        }
        break;
    }
    return {};
}

} // namespace ast
