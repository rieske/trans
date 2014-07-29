#include "ListCarrier.h"

#include <algorithm>

#include "AbstractSyntaxTreeVisitor.h"

namespace semantic_analyzer {

ListCarrier::ListCarrier(std::unique_ptr<AbstractSyntaxTreeNode> child) {
	auto childCode = child->getCode();
	children.push_back(std::move(child));
	code.insert(code.end(), childCode.begin(), childCode.end());
}

ListCarrier::~ListCarrier() {
}

void ListCarrier::addChild(std::unique_ptr<AbstractSyntaxTreeNode> child) {
	auto childCode = child->getCode();
	children.push_back(std::move(child));
	code.insert(code.end(), childCode.begin(), childCode.end());
}

const std::vector<std::unique_ptr<AbstractSyntaxTreeNode>>& ListCarrier::getChildren() const {
	return children;
}

void ListCarrier::accept(const AbstractSyntaxTreeVisitor& visitor) const {
	visitor.visit(*this);
}

} /* namespace semantic_analyzer */
