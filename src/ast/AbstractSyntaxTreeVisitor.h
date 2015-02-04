#ifndef ABSTRACTSYNTAXTREEVISITOR_H_
#define ABSTRACTSYNTAXTREEVISITOR_H_

namespace ast {

class AbstractSyntaxTreeNode;
class TypeSpecifier;
class Declaration;
class DeclarationList;
class ArrayAccess;
class FunctionCall;
class IdentifierExpression;
class PostfixExpression;
class PrefixExpression;
class UnaryExpression;
class TypeCast;
class PointerCast;
class ArithmeticExpression;
class ShiftExpression;
class ComparisonExpression;
class BitwiseExpression;
class LogicalAndExpression;
class LogicalOrExpression;
class AssignmentExpression;
class ExpressionList;
class JumpStatement;
class ReturnStatement;
class IOStatement;
class WhileLoopHeader;
class ForLoopHeader;
class IfStatement;
class IfElseStatement;
class LoopStatement;
class Pointer;
class Identifier;
class FunctionDeclarator;
class FormalArgument;
class ArrayDeclarator;
class FunctionDefinition;
class VariableDeclaration;
class VariableDefinition;
class Block;
class Operator;
class ConstantExpression;
class DeclarationSpecifiers;

class AbstractSyntaxTreeVisitor {
public:
    virtual ~AbstractSyntaxTreeVisitor() = default;

    virtual void visit(DeclarationSpecifiers& declarationSpecifiers) = 0;
    virtual void visit(Declaration& declaration) = 0;

    virtual void visit(DeclarationList& declarations) = 0;
    virtual void visit(ArrayAccess& arrayAccess) = 0;
    virtual void visit(FunctionCall& functionCall) = 0;
    virtual void visit(IdentifierExpression& identifier) = 0;
    virtual void visit(ConstantExpression& constant) = 0;
    virtual void visit(PostfixExpression& expression) = 0;
    virtual void visit(PrefixExpression& expression) = 0;
    virtual void visit(UnaryExpression& expression) = 0;
    virtual void visit(TypeCast& expression) = 0;
    virtual void visit(PointerCast& expression) = 0;
    virtual void visit(ArithmeticExpression& expression) = 0;
    virtual void visit(ShiftExpression& expression) = 0;
    virtual void visit(ComparisonExpression& expression) = 0;
    virtual void visit(BitwiseExpression& expression) = 0;
    virtual void visit(LogicalAndExpression& expression) = 0;
    virtual void visit(LogicalOrExpression& expression) = 0;
    virtual void visit(AssignmentExpression& expression) = 0;
    virtual void visit(ExpressionList& expression) = 0;

    virtual void visit(Operator& op) = 0;

    virtual void visit(JumpStatement& statement) = 0;
    virtual void visit(ReturnStatement& statement) = 0;
    virtual void visit(IOStatement& statement) = 0;
    virtual void visit(IfStatement& statement) = 0;
    virtual void visit(IfElseStatement& statement) = 0;
    virtual void visit(LoopStatement& statement) = 0;

    virtual void visit(ForLoopHeader& loopHeader) = 0;
    virtual void visit(WhileLoopHeader& loopHeader) = 0;

    virtual void visit(Pointer& pointer) = 0;

    virtual void visit(Identifier& identifier) = 0;
    virtual void visit(FunctionDeclarator& declaration) = 0;
    virtual void visit(ArrayDeclarator& declaration) = 0;

    virtual void visit(FormalArgument& parameter) = 0;

    virtual void visit(FunctionDefinition& function) = 0;

    virtual void visit(VariableDeclaration& declaration) = 0;
    virtual void visit(VariableDefinition& definition) = 0;

    virtual void visit(Block& block) = 0;
};

} /* namespace ast */

#endif /* ABSTRACTSYNTAXTREEVISITOR_H_ */
