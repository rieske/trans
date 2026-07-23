#include "Declarator.h"

#include <algorithm>

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

type::Type ast::Declarator::getFundamentalType(const type::Type& baseType) {
    return declarator->getFundamentalType(indirection, baseType);
}

void Declarator::foldArrayBoundSizeofs(const std::function<void(Expression*)>& foldSizeof) {
    std::function<void(DirectDeclarator*)> walk = [&](DirectDeclarator* dd) {
        if (!dd) {
            return;
        }
        if (auto* arr = dynamic_cast<ArrayDeclarator*>(dd)) {
            if (arr->subscriptExpression) {
                foldSizeof(arr->subscriptExpression.get());
                long size = 0;
                if (arr->subscriptExpression->evaluateConstant(size) && size > 0) {
                    arr->setArraySize(size);
                }
            }
            walk(&arr->getBaseDeclarator());
            return;
        }
        if (auto* nested = dynamic_cast<NestedDeclarator*>(dd)) {
            nested->getDeclarator().foldArrayBoundSizeofs(foldSizeof);
            return;
        }
        if (auto* fun = dynamic_cast<FunctionDeclarator*>(dd)) {
            walk(&fun->getBaseDeclarator());
        }
    };
    walk(declarator.get());
}

bool Declarator::hasArrayDeclarator() const {
    std::function<bool(DirectDeclarator*)> walk = [&](DirectDeclarator* dd) -> bool {
        if (!dd) {
            return false;
        }
        if (dynamic_cast<ArrayDeclarator*>(dd)) {
            return true;
        }
        if (auto* nested = dynamic_cast<NestedDeclarator*>(dd)) {
            return nested->getDeclarator().hasArrayDeclarator();
        }
        if (auto* fun = dynamic_cast<FunctionDeclarator*>(dd)) {
            return walk(&fun->getBaseDeclarator());
        }
        return false;
    };
    return walk(declarator.get());
}

} // namespace ast
