#include "CodeGeneratingVisitor.h"

#include <iostream>

#include "ast/ArithmeticExpression.h"
#include "ast/ArrayAccess.h"
#include "ast/ArrayDeclarator.h"
#include "ast/BitwiseExpression.h"
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
#include "ast/LogicalExpression.h"
#include "ast/LoopStatement.h"
#include "ast/Operator.h"
#include "ast/PostfixExpression.h"
#include "ast/PrefixExpression.h"
#include "ast/ReturnStatement.h"
#include "ast/IdentifierExpression.h"
#include "ast/ConstantExpression.h"
#include "ast/UnaryExpression.h"
#include "ast/WhileLoopHeader.h"
#include "ast/ExpressionList.h"
#include "ast/LogicalAndExpression.h"
#include "ast/LogicalOrExpression.h"
#include "ast/ShiftExpression.h"
#include "ast/TypeCast.h"
#include "ast/AssignmentExpression.h"
#include "ast/Declarator.h"
#include "ast/InitializedDeclarator.h"
#include "ast/Declaration.h"

#include "semantic_analyzer/ValueEntry.h"
#include "semantic_analyzer/LabelEntry.h"

#include "quadruples/Assign.h"
#include "quadruples/Argument.h"
#include "quadruples/Call.h"
#include "quadruples/Retrieve.h"
#include "quadruples/AssignConstant.h"
#include "quadruples/Inc.h"
#include "quadruples/Dec.h"
#include "quadruples/AddressOf.h"
#include "quadruples/Dereference.h"
#include "quadruples/UnaryMinus.h"
#include "quadruples/ValueCompare.h"
#include "quadruples/ZeroCompare.h"
#include "quadruples/Jump.h"
#include "quadruples/Label.h"
#include "quadruples/Add.h"
#include "quadruples/Sub.h"
#include "quadruples/Mul.h"
#include "quadruples/Div.h"
#include "quadruples/Mod.h"
#include "quadruples/And.h"
#include "quadruples/Or.h"
#include "quadruples/Xor.h"
#include "quadruples/Return.h"
#include "quadruples/Input.h"
#include "quadruples/Output.h"
#include "quadruples/LvalueAssign.h"
#include "quadruples/StartProcedure.h"
#include "quadruples/EndProcedure.h"

namespace codegen {

CodeGeneratingVisitor::CodeGeneratingVisitor() {
}

CodeGeneratingVisitor::~CodeGeneratingVisitor() {
}

void CodeGeneratingVisitor::visit(ast::DeclarationSpecifiers&) {
}

void CodeGeneratingVisitor::visit(ast::Declaration& declaration) {
    declaration.visitChildren(*this);
}

void CodeGeneratingVisitor::visit(ast::Declarator& declarator) {
    declarator.visitChildren(*this);
}

void CodeGeneratingVisitor::visit(ast::InitializedDeclarator& declarator) {
    declarator.visitChildren(*this);

    if (declarator.hasInitializer()) {
        quadruples.push_back(
                std::make_unique<Assign>(declarator.getInitializerHolder()->getName(), declarator.getHolder()->getName()));
    }
}

void CodeGeneratingVisitor::visit(ast::ArrayAccess& arrayAccess) {
    arrayAccess.visitLeftOperand(*this);
    arrayAccess.visitRightOperand(*this);

    // TODO: not implemented yet
    auto offset = arrayAccess.rightOperandSymbol();
    //quadruples.push_back( { code_generator::INDEX, arrayAccess.leftOperandSymbol(), offset, arrayAccess.getResultSymbol() });
    //quadruples.push_back( { code_generator::INDEX_ADDR, arrayAccess.leftOperandSymbol(), offset, arrayAccess.getLvalue() });
}

void CodeGeneratingVisitor::visit(ast::FunctionCall& functionCall) {
    functionCall.visitOperand(*this);
    functionCall.visitArguments(*this);

    for (auto& expression : functionCall.getArgumentList()) {
        quadruples.push_back(std::make_unique<Argument>(expression->getResultSymbol()->getName()));
    }

    quadruples.push_back(std::make_unique<Call>(functionCall.getSymbol()->getName()));
    if (!functionCall.getType().isVoid()) {
        quadruples.push_back(std::make_unique<Retrieve>(functionCall.getResultSymbol()->getName()));
    }
}

void CodeGeneratingVisitor::visit(ast::IdentifierExpression&) {
}

void CodeGeneratingVisitor::visit(ast::ConstantExpression& constant) {
    quadruples.push_back(std::make_unique<AssignConstant>(constant.getValue(), constant.getResultSymbol()->getName()));
}

void CodeGeneratingVisitor::visit(ast::PostfixExpression& expression) {
    expression.visitOperand(*this);

    auto resultSymbolName = expression.getResultSymbol()->getName();
    auto preOperationSymbol = expression.getPreOperationSymbol()->getName();
    quadruples.push_back(std::make_unique<Assign>(resultSymbolName, preOperationSymbol));

    if (expression.getOperator()->getLexeme() == "++") {
        quadruples.push_back(std::make_unique<Inc>(resultSymbolName));
    } else if (expression.getOperator()->getLexeme() == "--") {
        quadruples.push_back(std::make_unique<Dec>(resultSymbolName));
    }

    expression.setResultSymbol(*expression.getPreOperationSymbol());
}

void CodeGeneratingVisitor::visit(ast::PrefixExpression& expression) {
    expression.visitOperand(*this);

    if (expression.getOperator()->getLexeme() == "++") {
        quadruples.push_back(std::make_unique<Inc>(expression.getResultSymbol()->getName()));
    } else if (expression.getOperator()->getLexeme() == "--") {
        quadruples.push_back(std::make_unique<Dec>(expression.getResultSymbol()->getName()));
    }
}

void CodeGeneratingVisitor::visit(ast::UnaryExpression& expression) {
    expression.visitOperand(*this);

    switch (expression.getOperator()->getLexeme().front()) {
    case '&':
        quadruples.push_back(
                std::make_unique<AddressOf>(expression.operandSymbol()->getName(),
                        expression.getResultSymbol()->getName()));
        expression.visitOperand(*this);
        break;
    case '*':
        quadruples.push_back(
                std::make_unique<Dereference>(expression.operandSymbol()->getName(),
                        expression.getLvalueSymbol()->getName(),
                        expression.getResultSymbol()->getName()));
        expression.visitOperand(*this);
        break;
    case '+':
        expression.visitOperand(*this);
        break;
    case '-':
        expression.visitOperand(*this);
        quadruples.push_back(
                std::make_unique<UnaryMinus>(expression.operandSymbol()->getName(),
                        expression.getResultSymbol()->getName()));
        break;
    case '!':
        expression.visitOperand(*this);
        quadruples.push_back(std::make_unique<ZeroCompare>(expression.operandSymbol()->getName()));
        quadruples.push_back(
                std::make_unique<Jump>(expression.getTruthyLabel()->getName(), JumpCondition::IF_EQUAL));
        quadruples.push_back(std::make_unique<AssignConstant>("0", expression.getResultSymbol()->getName()));
        quadruples.push_back(std::make_unique<Jump>(expression.getFalsyLabel()->getName()));
        quadruples.push_back(std::make_unique<Label>(expression.getTruthyLabel()->getName()));
        quadruples.push_back(
                std::make_unique<AssignConstant>("1", expression.getResultSymbol()->getName()));
        quadruples.push_back(std::make_unique<Label>(expression.getFalsyLabel()->getName()));
        break;
    default:
        throw std::runtime_error { "Unidentified unary operator: " + expression.getOperator()->getLexeme() };
    }
}

void CodeGeneratingVisitor::visit(ast::TypeCast& expression) {
    expression.visitOperand(*this);
    quadruples.push_back(
            std::make_unique<Assign>(expression.operandSymbol()->getName(), expression.getResultSymbol()->getName()));
}

void CodeGeneratingVisitor::visit(ast::ArithmeticExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    switch (expression.getOperator()->getLexeme().front()) {
    case '+':
        quadruples.push_back(std::make_unique<Add>(
                expression.leftOperandSymbol()->getName(),
                expression.rightOperandSymbol()->getName(),
                expression.getResultSymbol()->getName()));
        break;
    case '-':
        quadruples.push_back(std::make_unique<Sub>(
                expression.leftOperandSymbol()->getName(),
                expression.rightOperandSymbol()->getName(),
                expression.getResultSymbol()->getName()));
        break;
    case '*':
        quadruples.push_back(std::make_unique<Mul>(
                expression.leftOperandSymbol()->getName(),
                expression.rightOperandSymbol()->getName(),
                expression.getResultSymbol()->getName()));
        break;
    case '/':
        quadruples.push_back(std::make_unique<Div>(
                expression.leftOperandSymbol()->getName(),
                expression.rightOperandSymbol()->getName(),
                expression.getResultSymbol()->getName()));
        break;
    case '%':
        quadruples.push_back(std::make_unique<Mod>(
                expression.leftOperandSymbol()->getName(),
                expression.rightOperandSymbol()->getName(),
                expression.getResultSymbol()->getName()));
        break;
    default:
        throw std::runtime_error { "unidentified arithmetic operator: " + expression.getOperator()->getLexeme() };
    }
}

void CodeGeneratingVisitor::visit(ast::ShiftExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    // TODO: not implemented yet
    throw std::runtime_error { "shift operations are not implemented yet!" };
    switch (expression.getOperator()->getLexeme().front()) {
    case '<':   // <<
        //quadruples.push_back(
        //        { code_generator::SHL, expression.leftOperandSymbol(), expression.rightOperandSymbol(), expression.getResultSymbol() });
        break;
    case '>':   // >>
        //quadruples.push_back(
        //        { code_generator::SHR, expression.leftOperandSymbol(), expression.rightOperandSymbol(), expression.getResultSymbol() });
        break;
    default:
        throw std::runtime_error { "unidentified add_op operator!" };
    }
}

void CodeGeneratingVisitor::visit(ast::ComparisonExpression& expression) {
    std::cout << "comparison expression\n";
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    quadruples.push_back(
            std::make_unique<ValueCompare>(expression.leftOperandSymbol()->getName(),
                    expression.rightOperandSymbol()->getName()));

    auto truthyLabel = expression.getTruthyLabel()->getName();
    if (expression.getOperator()->getLexeme() == ">") {
        quadruples.push_back(std::make_unique<Jump>(truthyLabel, JumpCondition::IF_ABOVE));
    } else if (expression.getOperator()->getLexeme() == "<") {
        quadruples.push_back(std::make_unique<Jump>(truthyLabel, JumpCondition::IF_BELOW));
    } else if (expression.getOperator()->getLexeme() == "<=") {
        quadruples.push_back(std::make_unique<Jump>(truthyLabel, JumpCondition::IF_BELOW_OR_EQUAL));
    } else if (expression.getOperator()->getLexeme() == ">=") {
        quadruples.push_back(std::make_unique<Jump>(truthyLabel, JumpCondition::IF_ABOVE_OR_EQUAL));
    } else if (expression.getOperator()->getLexeme() == "==") {
        quadruples.push_back(std::make_unique<Jump>(truthyLabel, JumpCondition::IF_EQUAL));
    } else if (expression.getOperator()->getLexeme() == "!=") {
        std::cout << "not equal\n";
        quadruples.push_back(std::make_unique<Jump>(truthyLabel, JumpCondition::IF_NOT_EQUAL));
    } else {
        throw std::runtime_error { "unidentified ml_op operator!\n" };
    }

    quadruples.push_back(std::make_unique<AssignConstant>("0", expression.getResultSymbol()->getName()));
    quadruples.push_back(std::make_unique<Jump>(expression.getFalsyLabel()->getName()));
    quadruples.push_back(std::make_unique<Label>(truthyLabel));
    quadruples.push_back(std::make_unique<AssignConstant>("1", expression.getResultSymbol()->getName()));
    quadruples.push_back(std::make_unique<Label>(expression.getFalsyLabel()->getName()));
    std::cout << "done with comparison\n";
}

void CodeGeneratingVisitor::visit(ast::BitwiseExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    switch (expression.getOperator()->getLexeme().front()) {
    case '&':
        quadruples.push_back(std::make_unique<And>(
                expression.leftOperandSymbol()->getName(),
                expression.rightOperandSymbol()->getName(),
                expression.getResultSymbol()->getName()));
        break;
    case '|':
        quadruples.push_back(std::make_unique<Or>(
                expression.leftOperandSymbol()->getName(),
                expression.rightOperandSymbol()->getName(),
                expression.getResultSymbol()->getName()));
        break;
    case '^':
        quadruples.push_back(std::make_unique<Xor>(
                expression.leftOperandSymbol()->getName(),
                expression.rightOperandSymbol()->getName(),
                expression.getResultSymbol()->getName()));
        break;
    default:
        throw std::runtime_error { "no semantic actions defined for bitwise operator: " + expression.getOperator()->getLexeme() };
    }
}

void CodeGeneratingVisitor::visit(ast::LogicalAndExpression& expression) {
    expression.visitLeftOperand(*this);

    quadruples.push_back(std::make_unique<AssignConstant>("0", expression.getResultSymbol()->getName()));
    quadruples.push_back(std::make_unique<ZeroCompare>(expression.leftOperandSymbol()->getName()));
    quadruples.push_back(
            std::make_unique<Jump>(expression.getExitLabel()->getName(), JumpCondition::IF_EQUAL));

    expression.visitRightOperand(*this);

    quadruples.push_back(std::make_unique<ZeroCompare>(expression.rightOperandSymbol()->getName()));
    quadruples.push_back(
            std::make_unique<Jump>(expression.getExitLabel()->getName(), JumpCondition::IF_EQUAL));
    quadruples.push_back(std::make_unique<AssignConstant>("1", expression.getResultSymbol()->getName()));

    quadruples.push_back(std::make_unique<Label>(expression.getExitLabel()->getName()));
}

void CodeGeneratingVisitor::visit(ast::LogicalOrExpression& expression) {
    expression.visitLeftOperand(*this);

    quadruples.push_back(std::make_unique<AssignConstant>("1", expression.getResultSymbol()->getName()));
    quadruples.push_back(std::make_unique<ZeroCompare>(expression.leftOperandSymbol()->getName()));
    quadruples.push_back(
            std::make_unique<Jump>(expression.getExitLabel()->getName(), JumpCondition::IF_NOT_EQUAL));

    expression.visitRightOperand(*this);

    quadruples.push_back(std::make_unique<ZeroCompare>(expression.rightOperandSymbol()->getName()));
    quadruples.push_back(
            std::make_unique<Jump>(expression.getExitLabel()->getName(), JumpCondition::IF_NOT_EQUAL));
    quadruples.push_back(std::make_unique<AssignConstant>("0", expression.getResultSymbol()->getName()));

    quadruples.push_back(std::make_unique<Label>(expression.getExitLabel()->getName()));
}

void CodeGeneratingVisitor::visit(ast::AssignmentExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    auto assignmentOperator = expression.getOperator();
    if (assignmentOperator->getLexeme() == "+=")
        quadruples.push_back(std::make_unique<Add>(
                expression.getResultSymbol()->getName(),
                expression.rightOperandSymbol()->getName(),
                expression.getResultSymbol()->getName()));
    else if (assignmentOperator->getLexeme() == "-=")
        quadruples.push_back(std::make_unique<Sub>(
                expression.getResultSymbol()->getName(),
                expression.rightOperandSymbol()->getName(),
                expression.getResultSymbol()->getName()));
    else if (assignmentOperator->getLexeme() == "*=")
        quadruples.push_back(std::make_unique<Mul>(
                expression.getResultSymbol()->getName(),
                expression.rightOperandSymbol()->getName(),
                expression.getResultSymbol()->getName()));
    else if (assignmentOperator->getLexeme() == "/=")
        quadruples.push_back(std::make_unique<Div>(
                expression.getResultSymbol()->getName(),
                expression.rightOperandSymbol()->getName(),
                expression.getResultSymbol()->getName()));
    else if (assignmentOperator->getLexeme() == "%=")
        quadruples.push_back(std::make_unique<Mod>(
                expression.getResultSymbol()->getName(),
                expression.rightOperandSymbol()->getName(),
                expression.getResultSymbol()->getName()));
    else if (assignmentOperator->getLexeme() == "&=")
        quadruples.push_back(std::make_unique<And>(
                expression.getResultSymbol()->getName(),
                expression.rightOperandSymbol()->getName(),
                expression.getResultSymbol()->getName()));
    else if (assignmentOperator->getLexeme() == "^=")
        quadruples.push_back(std::make_unique<Xor>(
                expression.getResultSymbol()->getName(),
                expression.rightOperandSymbol()->getName(),
                expression.getResultSymbol()->getName()));
    else if (assignmentOperator->getLexeme() == "|=")
        quadruples.push_back(std::make_unique<Or>(
                expression.getResultSymbol()->getName(),
                expression.rightOperandSymbol()->getName(),
                expression.getResultSymbol()->getName()));
    else if (assignmentOperator->getLexeme() == "<<=") {
        throw std::runtime_error { "shift operations not implemented yet" };
    } else if (assignmentOperator->getLexeme() == ">>=") {
        throw std::runtime_error { "shift operations not implemented yet" };
    } else if (assignmentOperator->getLexeme() == "=") {
        if (expression.leftOperandLvalueSymbol()) {
            quadruples.push_back(std::make_unique<LvalueAssign>(expression.rightOperandSymbol()->getName(),
                    expression.leftOperandLvalueSymbol()->getName()));
        } else {
            quadruples.push_back(std::make_unique<Assign>(expression.rightOperandSymbol()->getName(),
                    expression.getResultSymbol()->getName()));
        }
    } else {
        throw std::runtime_error { "unidentified assignment operator: " + assignmentOperator->getLexeme() };
    }
}

void CodeGeneratingVisitor::visit(ast::ExpressionList& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);
}

void CodeGeneratingVisitor::visit(ast::Operator&) {
}

void CodeGeneratingVisitor::visit(ast::JumpStatement& statement) {
    throw std::runtime_error { "not implemented" };
}

void CodeGeneratingVisitor::visit(ast::ReturnStatement& statement) {
    statement.returnExpression->accept(*this);
    quadruples.push_back(std::make_unique<Return>(statement.returnExpression->getResultSymbol()->getName()));
}

void CodeGeneratingVisitor::visit(ast::IOStatement& statement) {
    statement.expression->accept(*this);
    if (statement.ioKeyword.value == "output") {
        quadruples.push_back(std::make_unique<Output>(statement.expression->getResultSymbol()->getName()));
    } else if (statement.ioKeyword.value == "input") {
        quadruples.push_back(std::make_unique<Input>(statement.expression->getResultSymbol()->getName()));
    } else {
        throw std::runtime_error { "bad IO statement: " + statement.ioKeyword.type };
    }
}

void CodeGeneratingVisitor::visit(ast::IfStatement& statement) {
    statement.testExpression->accept(*this);

    quadruples.push_back(std::make_unique<ZeroCompare>(statement.testExpression->getResultSymbol()->getName()));
    quadruples.push_back(
            std::make_unique<Jump>(statement.getFalsyLabel()->getName(), JumpCondition::IF_EQUAL));

    statement.body->accept(*this);

    quadruples.push_back(std::make_unique<Label>(statement.getFalsyLabel()->getName()));
}

void CodeGeneratingVisitor::visit(ast::IfElseStatement& statement) {
    statement.testExpression->accept(*this);

    quadruples.push_back(std::make_unique<ZeroCompare>(statement.testExpression->getResultSymbol()->getName()));
    quadruples.push_back(
            std::make_unique<Jump>(statement.getFalsyLabel()->getName(), JumpCondition::IF_EQUAL));

    statement.truthyBody->accept(*this);
    quadruples.push_back(std::make_unique<Jump>(statement.getExitLabel()->getName()));
    quadruples.push_back(std::make_unique<Label>(statement.getFalsyLabel()->getName()));

    statement.falsyBody->accept(*this);
    quadruples.push_back(std::make_unique<Label>(statement.getExitLabel()->getName()));
}

void CodeGeneratingVisitor::visit(ast::LoopStatement& loop) {
    std::cout << "visiting loop statement\n";
    loop.header->accept(*this);
    std::cout << "after loop header\n";
    loop.body->accept(*this);
    std::cout << "after loop body\n";
    // FIXME:
    if (loop.header->increment) {
        loop.header->increment->accept(*this);
    }

    quadruples.push_back(std::make_unique<Jump>(loop.header->getLoopEntry()->getName()));
    quadruples.push_back(std::make_unique<Label>(loop.header->getLoopExit()->getName()));
    std::cout << "after loop\n";
}

void CodeGeneratingVisitor::visit(ast::ForLoopHeader& loopHeader) {
    loopHeader.initialization->accept(*this);

    quadruples.push_back(std::make_unique<Label>(loopHeader.getLoopEntry()->getName()));
    loopHeader.clause->accept(*this);
    quadruples.push_back(std::make_unique<ZeroCompare>(loopHeader.clause->getResultSymbol()->getName()));
    quadruples.push_back(
            std::make_unique<Jump>(loopHeader.getLoopExit()->getName(), JumpCondition::IF_EQUAL));
}

void CodeGeneratingVisitor::visit(ast::WhileLoopHeader& loopHeader) {
    std::cout << "visiting loop header\n";
    quadruples.push_back(std::make_unique<Label>(loopHeader.getLoopEntry()->getName()));
    std::cout << loopHeader.clause.get();
    loopHeader.clause->accept(*this);
    std::cout << "done with clause\n";
    std::cout << loopHeader.getLoopExit() << std::endl; // fails here
    std::cout << loopHeader.clause->getResultSymbol()->getName() << std::endl;
    quadruples.push_back(std::make_unique<ZeroCompare>(loopHeader.clause->getResultSymbol()->getName()));
    quadruples.push_back(
            std::make_unique<Jump>(loopHeader.getLoopExit()->getName(), JumpCondition::IF_EQUAL));
}

void CodeGeneratingVisitor::visit(ast::Pointer&) {
}

void CodeGeneratingVisitor::visit(ast::Identifier&) {
}

void CodeGeneratingVisitor::visit(ast::FunctionDeclarator& declarator) {
    declarator.visitFormalArguments(*this);
}

void CodeGeneratingVisitor::visit(ast::ArrayDeclarator& declaration) {
    declaration.subscriptExpression->accept(*this);
    throw std::runtime_error { "not implemented" };
}

void CodeGeneratingVisitor::visit(ast::FormalArgument& parameter) {
    parameter.visitDeclarator(*this);
}

void CodeGeneratingVisitor::visit(ast::FunctionDefinition& function) {
    function.visitDeclarator(*this);

    std::vector<Value> values;
    for (auto& valueSymbol : function.getLocalVariables()) {
        values.push_back( {
                valueSymbol.second.getName(),
                valueSymbol.second.getIndex(),
                // FIXME:
                Type::INTEGRAL,
                valueSymbol.second.getType().getSizeInBytes(),
                false
        });
    }
    std::vector<Value> arguments;
    for (auto& argumentSymbol : function.getArguments()) {
        arguments.push_back( {
                argumentSymbol.getName(),
                argumentSymbol.getIndex(),
                // FIXME:
                Type::INTEGRAL,
                argumentSymbol.getType().getSizeInBytes(),
                true
        });
    }
    quadruples.push_back(std::make_unique<StartProcedure>(function.getSymbol()->getName(), std::move(values), std::move(arguments)));
    function.visitBody(*this);
    quadruples.push_back(std::make_unique<EndProcedure>(function.getSymbol()->getName()));
}

void CodeGeneratingVisitor::visit(ast::Block& block) {
    block.visitChildren(*this);
}

std::vector<std::unique_ptr<Quadruple>> CodeGeneratingVisitor::getQuadruples() {
    // FIXME: temporary:
    std::cout << "\nvisitor quadruples\n";
    for (auto& quadruple : quadruples) {
        std::cout << *quadruple;
    }
    std::cout << "visitor quadruples end\n\n";
    return std::move(quadruples);
}

} /* namespace codegen */

