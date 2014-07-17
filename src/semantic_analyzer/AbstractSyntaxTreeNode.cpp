#include "AbstractSyntaxTreeNode.h"
#include "AbstractSyntaxTreeVisitor.h"

namespace semantic_analyzer {

AbstractSyntaxTreeNode::AbstractSyntaxTreeNode() {

}

AbstractSyntaxTreeNode::~AbstractSyntaxTreeNode() {
}

void AbstractSyntaxTreeNode::accept(AbstractSyntaxTreeVisitor& visitor) const {
	visitor.visit(*this);
}

} /* namespace semantic_analyzer */
