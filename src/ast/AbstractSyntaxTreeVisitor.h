#ifndef ABSTRACTSYNTAXTREEVISITOR_H_
#define ABSTRACTSYNTAXTREEVISITOR_H_

namespace ast {

class ArithmeticExpression;
class ArrayAccess;
class ArrayDeclarator;
class AssignmentExpression;
class BitwiseExpression;
class Block;
class CaseLabel;
class ComparisonExpression;
class CompoundLiteral;
class ConditionalExpression;
class ConstantExpression;
class Declaration;
class DeclarationSpecifiers;
class Declarator;
class DefaultLabel;
class DoWhileLoopHeader;
class ExpressionList;
class ExpressionStatement;
class ForLoopHeader;
class FormalArgument;
class FunctionCall;
class FunctionDeclarator;
class FunctionDefinition;
class GenericSelection;
class GotoStatement;
class Identifier;
class IdentifierExpression;
class IfStatement;
class InitializedDeclarator;
class InitializerListExpression;
class JumpStatement;
class LabeledStatement;
class LogicalAndExpression;
class LogicalOrExpression;
class LoopStatement;
class MemberAccess;
class NullStatement;
class Pointer;
class PostfixExpression;
class PrefixExpression;
class ReturnStatement;
class ShiftExpression;
class StatementExpression;
class StringLiteralExpression;
class SwitchStatement;
class TypeCast;
class TypeNameExpression;
class UnaryExpression;
class WhileLoopHeader;

class AbstractSyntaxTreeVisitor {
public:
    virtual ~AbstractSyntaxTreeVisitor() = default;

    virtual void visit(DeclarationSpecifiers& declarationSpecifiers) = 0;
    virtual void visit(Declaration& declaration) = 0;

    virtual void visit(Declarator& declarator) = 0;
    virtual void visit(InitializedDeclarator& declarator) = 0;

    virtual void visit(ArrayAccess& arrayAccess) = 0;
    virtual void visit(MemberAccess& memberAccess) = 0;
    virtual void visit(InitializerListExpression& expression) = 0;
    virtual void visit(FunctionCall& functionCall) = 0;
    virtual void visit(IdentifierExpression& identifier) = 0;
    virtual void visit(ConstantExpression& constant) = 0;
    virtual void visit(StringLiteralExpression& stringLiteral) = 0;
    virtual void visit(PostfixExpression& expression) = 0;
    virtual void visit(PrefixExpression& expression) = 0;
    virtual void visit(UnaryExpression& expression) = 0;
    virtual void visit(TypeCast& expression) = 0;
    virtual void visit(TypeNameExpression& expression) = 0;
    virtual void visit(CompoundLiteral& expression) = 0;
    virtual void visit(GenericSelection& expression) = 0;
    virtual void visit(StatementExpression& expression) = 0;
    virtual void visit(ArithmeticExpression& expression) = 0;
    virtual void visit(ShiftExpression& expression) = 0;
    virtual void visit(ComparisonExpression& expression) = 0;
    virtual void visit(BitwiseExpression& expression) = 0;
    virtual void visit(LogicalAndExpression& expression) = 0;
    virtual void visit(LogicalOrExpression& expression) = 0;
    virtual void visit(ConditionalExpression& expression) = 0;
    virtual void visit(AssignmentExpression& expression) = 0;
    virtual void visit(ExpressionList& expression) = 0;

    virtual void visit(JumpStatement& statement) = 0;
    virtual void visit(GotoStatement& statement) = 0;
    virtual void visit(LabeledStatement& statement) = 0;
    virtual void visit(SwitchStatement& statement) = 0;
    virtual void visit(CaseLabel& statement) = 0;
    virtual void visit(DefaultLabel& statement) = 0;
    virtual void visit(ReturnStatement& statement) = 0;
    virtual void visit(ExpressionStatement& statement) = 0;
    virtual void visit(NullStatement& statement) = 0;
    virtual void visit(IfStatement& statement) = 0;
    virtual void visit(LoopStatement& statement) = 0;

    virtual void visit(ForLoopHeader& loopHeader) = 0;
    virtual void visit(WhileLoopHeader& loopHeader) = 0;
    virtual void visit(DoWhileLoopHeader& loopHeader) = 0;

    virtual void visit(Pointer& pointer) = 0;

    virtual void visit(Identifier& identifier) = 0;
    virtual void visit(FunctionDeclarator& declaration) = 0;
    virtual void visit(ArrayDeclarator& declaration) = 0;

    virtual void visit(FormalArgument& parameter) = 0;

    virtual void visit(FunctionDefinition& function) = 0;

    virtual void visit(Block& block) = 0;
};

} // namespace ast

#endif // ABSTRACTSYNTAXTREEVISITOR_H_
