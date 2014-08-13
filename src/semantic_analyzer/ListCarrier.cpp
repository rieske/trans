#include "ListCarrier.h"

#include <algorithm>

#include "AbstractSyntaxTreeVisitor.h"

namespace semantic_analyzer {

ListCarrier::ListCarrier(std::unique_ptr<AbstractSyntaxTreeNode> child) {
    children.push_back(std::move(child));
}

ListCarrier::~ListCarrier() {
}

void ListCarrier::addChild(std::unique_ptr<AbstractSyntaxTreeNode> child) {
    children.push_back(std::move(child));
}

const std::vector<std::unique_ptr<AbstractSyntaxTreeNode>>& ListCarrier::getChildren() const {
    return children;
}

void ListCarrier::accept(AbstractSyntaxTreeVisitor& visitor) {
    visitor.visit(*this);
}

} /* namespace semantic_analyzer */
