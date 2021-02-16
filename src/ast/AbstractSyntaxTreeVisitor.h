#ifndef ABSTRACTSYNTAXTREEVISITOR_H_
#define ABSTRACTSYNTAXTREEVISITOR_H_

#include "ast/ArrayAccess.h"
#include "ast/ArrayDeclarator.h"
#include "ast/Block.h"
#include "ast/ComparisonExpression.h"
#include "ast/ForLoopHeader.h"
#include "ast/FunctionCall.h"
#include "ast/FunctionDeclarator.h"
#include "ast/FunctionDefinition.h"
#include "ast/Identifier.h"
#include "ast/IfElseStatement.h"
#include "ast/IfStatement.h"
#include "ast/IOStatement.h"
#include "ast/JumpStatement.h"
#include "ast/LoopStatement.h"
#include "ast/Operator.h"
#include "ast/ReturnStatement.h"
#include "ast/VoidReturnStatement.h"
#include "ast/IdentifierExpression.h"
#include "ast/ConstantExpression.h"
#include "ast/TypeCast.h"
#include "ast/UnaryExpression.h"
#include "ast/WhileLoopHeader.h"
#include "ast/ArithmeticExpression.h"
#include "ast/BitwiseExpression.h"
#include "ast/ExpressionList.h"
#include "ast/LogicalAndExpression.h"
#include "ast/LogicalOrExpression.h"
#include "ast/PostfixExpression.h"
#include "ast/PrefixExpression.h"
#include "ast/ShiftExpression.h"
#include "ast/AssignmentExpression.h"
#include "ast/Declarator.h"
#include "ast/InitializedDeclarator.h"
#include "ast/Declaration.h"

namespace ast {

class AbstractSyntaxTreeVisitor {
public:
    virtual ~AbstractSyntaxTreeVisitor() = default;

    virtual void visit(DeclarationSpecifiers& declarationSpecifiers) = 0;
    virtual void visit(Declaration& declaration) = 0;

    virtual void visit(Declarator& declarator) = 0;
    virtual void visit(InitializedDeclarator& declarator) = 0;

    virtual void visit(ArrayAccess& arrayAccess) = 0;
    virtual void visit(FunctionCall& functionCall) = 0;
    virtual void visit(IdentifierExpression& identifier) = 0;
    virtual void visit(ConstantExpression& constant) = 0;
    virtual void visit(PostfixExpression& expression) = 0;
    virtual void visit(PrefixExpression& expression) = 0;
    virtual void visit(UnaryExpression& expression) = 0;
    virtual void visit(TypeCast& expression) = 0;
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
    virtual void visit(VoidReturnStatement& statement) = 0;
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

    virtual void visit(Block& block) = 0;
};

} // namespace ast

#endif // ABSTRACTSYNTAXTREEVISITOR_H_
