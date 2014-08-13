#include "TranslationUnit.h"

#include <algorithm>

#include "AbstractSyntaxTreeVisitor.h"

namespace semantic_analyzer {

TranslationUnit::TranslationUnit(std::unique_ptr<ListCarrier> functionDeclarations) {
    children.push_back(std::move(functionDeclarations));
}

TranslationUnit::TranslationUnit(std::unique_ptr<ListCarrier> variableDeclarations,
        std::unique_ptr<ListCarrier> functionDeclarations) {
    children.push_back(std::move(variableDeclarations));
    children.push_back(std::move(functionDeclarations));
}

TranslationUnit::~TranslationUnit() {
}

const std::vector<std::unique_ptr<ListCarrier>>& TranslationUnit::getChildren() const {
    return children;
}

void TranslationUnit::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

} /* namespace semantic_analyzer */
