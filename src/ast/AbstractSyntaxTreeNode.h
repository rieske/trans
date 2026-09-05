#ifndef ABSTRACTSYNTAXTREENODE_H_
#define ABSTRACTSYNTAXTREENODE_H_

namespace ast {

class AbstractSyntaxTreeVisitor;
class Block;
class Declaration;
class Expression;

// For local queries; SA/CG walks stay on the visitor.
enum class NodeKind {
    Expression,
    Declaration,
    Block,
    FunctionDefinition,
    IfStatement,
    IfElseStatement,
    LoopStatement,
    SwitchStatement,
    LabeledStatement,
    CaseLabel,
    DefaultLabel,
    JumpStatement,
    GotoStatement,
    ReturnStatement,
    VoidReturnStatement,
    DeclarationSpecifiers,
    Declarator,
    DirectDeclarator,
    Pointer,
    FormalArgument,
    InitializedDeclarator,
    LoopHeader,
};

class AbstractSyntaxTreeNode {
public:
    AbstractSyntaxTreeNode() = default;
    virtual ~AbstractSyntaxTreeNode() = default;

    virtual void accept(AbstractSyntaxTreeVisitor& visitor) = 0;
    virtual NodeKind nodeKind() const = 0;

    const Expression* asExpression() const;
    Expression* asExpression();
    const Declaration* asDeclaration() const;
    Declaration* asDeclaration();
    const Block* asBlock() const;
    Block* asBlock();

    // Visit this node's children directly; for a non-container node that is just the node itself.
    virtual void visitChildren(AbstractSyntaxTreeVisitor& visitor) { accept(visitor); }

protected:
    AbstractSyntaxTreeNode(const AbstractSyntaxTreeNode&) = default;
    AbstractSyntaxTreeNode(AbstractSyntaxTreeNode&&) = default;
    AbstractSyntaxTreeNode& operator=(const AbstractSyntaxTreeNode&) = default;
    AbstractSyntaxTreeNode& operator=(AbstractSyntaxTreeNode&&) = default;
};

} // namespace ast

#endif // ABSTRACTSYNTAXTREENODE_H_
