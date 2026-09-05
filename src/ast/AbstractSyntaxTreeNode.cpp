#include "AbstractSyntaxTreeNode.h"

#include "Block.h"
#include "Declaration.h"
#include "Expression.h"

namespace ast {

const Expression* AbstractSyntaxTreeNode::asExpression() const {
    return nodeKind() == NodeKind::Expression
            ? static_cast<const Expression*>(this) : nullptr;
}

Expression* AbstractSyntaxTreeNode::asExpression() {
    return const_cast<Expression*>(
            static_cast<const AbstractSyntaxTreeNode*>(this)->asExpression());
}

const Declaration* AbstractSyntaxTreeNode::asDeclaration() const {
    return nodeKind() == NodeKind::Declaration
            ? static_cast<const Declaration*>(this) : nullptr;
}

Declaration* AbstractSyntaxTreeNode::asDeclaration() {
    return const_cast<Declaration*>(
            static_cast<const AbstractSyntaxTreeNode*>(this)->asDeclaration());
}

const Block* AbstractSyntaxTreeNode::asBlock() const {
    return nodeKind() == NodeKind::Block
            ? static_cast<const Block*>(this) : nullptr;
}

Block* AbstractSyntaxTreeNode::asBlock() {
    return const_cast<Block*>(
            static_cast<const AbstractSyntaxTreeNode*>(this)->asBlock());
}

} // namespace ast
