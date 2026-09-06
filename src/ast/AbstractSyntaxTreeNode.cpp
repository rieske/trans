#include "AbstractSyntaxTreeNode.h"

#include "Block.h"
#include "Declaration.h"
#include "Expression.h"
#include "FunctionDefinition.h"
#include "Statement.h"

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

const FunctionDefinition* AbstractSyntaxTreeNode::asFunctionDefinition() const {
    return nodeKind() == NodeKind::FunctionDefinition
            ? static_cast<const FunctionDefinition*>(this) : nullptr;
}

FunctionDefinition* AbstractSyntaxTreeNode::asFunctionDefinition() {
    return const_cast<FunctionDefinition*>(
            static_cast<const AbstractSyntaxTreeNode*>(this)->asFunctionDefinition());
}

const Block* AbstractSyntaxTreeNode::asBlock() const {
    return nodeKind() == NodeKind::Block
            ? static_cast<const Block*>(this) : nullptr;
}

Block* AbstractSyntaxTreeNode::asBlock() {
    return const_cast<Block*>(
            static_cast<const AbstractSyntaxTreeNode*>(this)->asBlock());
}

const Statement* AbstractSyntaxTreeNode::asStatement() const {
    switch (nodeKind()) {
    case NodeKind::Block:
    case NodeKind::IfStatement:
    case NodeKind::IfElseStatement:
    case NodeKind::LoopStatement:
    case NodeKind::SwitchStatement:
    case NodeKind::LabeledStatement:
    case NodeKind::CaseLabel:
    case NodeKind::DefaultLabel:
    case NodeKind::JumpStatement:
    case NodeKind::GotoStatement:
    case NodeKind::ReturnStatement:
    case NodeKind::VoidReturnStatement:
    case NodeKind::ExpressionStatement:
    case NodeKind::NullStatement:
        return static_cast<const Statement*>(this);
    case NodeKind::Expression:
    case NodeKind::Declaration:
    case NodeKind::FunctionDefinition:
    case NodeKind::DeclarationSpecifiers:
    case NodeKind::Declarator:
    case NodeKind::DirectDeclarator:
    case NodeKind::Pointer:
    case NodeKind::FormalArgument:
    case NodeKind::InitializedDeclarator:
    case NodeKind::LoopHeader:
        return nullptr;
    }
    return nullptr;
}

Statement* AbstractSyntaxTreeNode::asStatement() {
    return const_cast<Statement*>(
            static_cast<const AbstractSyntaxTreeNode*>(this)->asStatement());
}

} // namespace ast
