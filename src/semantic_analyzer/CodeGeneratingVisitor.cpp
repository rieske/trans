#include "CodeGeneratingVisitor.h"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#include "../ast/ArithmeticExpression.h"
#include "../ast/ArrayAccess.h"
#include "../ast/ArrayDeclarator.h"
#include "../ast/BitwiseExpression.h"
#include "../ast/Block.h"
#include "../ast/ComparisonExpression.h"
#include "../ast/ForLoopHeader.h"
#include "../ast/FunctionCall.h"
#include "../ast/FunctionDeclarator.h"
#include "../ast/FunctionDefinition.h"
#include "../ast/Identifier.h"
#include "../ast/IfElseStatement.h"
#include "../ast/IfStatement.h"
#include "../ast/IOStatement.h"
#include "../ast/JumpStatement.h"
#include "../ast/LogicalExpression.h"
#include "../ast/LoopStatement.h"
#include "../ast/Operator.h"
#include "../ast/FormalArgument.h"
#include "../ast/Pointer.h"
#include "../ast/PostfixExpression.h"
#include "../ast/PrefixExpression.h"
#include "../ast/ReturnStatement.h"
#include "../ast/IdentifierExpression.h"
#include "../ast/ConstantExpression.h"
#include "../ast/TerminalSymbol.h"
#include "../ast/UnaryExpression.h"
#include "../ast/WhileLoopHeader.h"
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

#include "code_generator/quadruples/Assign.h"
#include "code_generator/quadruples/Argument.h"
#include "code_generator/quadruples/Call.h"
#include "code_generator/quadruples/Retrieve.h"
#include "code_generator/quadruples/AssignConstant.h"
#include "code_generator/quadruples/Inc.h"
#include "code_generator/quadruples/Dec.h"
#include "code_generator/quadruples/AddressOf.h"
#include "code_generator/quadruples/Dereference.h"
#include "code_generator/quadruples/UnaryMinus.h"
#include "code_generator/quadruples/ValueCompare.h"
#include "code_generator/quadruples/ZeroCompare.h"
#include "code_generator/quadruples/Jump.h"
#include "code_generator/quadruples/Label.h"
#include "code_generator/quadruples/Add.h"
#include "code_generator/quadruples/Sub.h"
#include "code_generator/quadruples/Mul.h"
#include "code_generator/quadruples/Div.h"
#include "code_generator/quadruples/Mod.h"
#include "code_generator/quadruples/And.h"
#include "code_generator/quadruples/Or.h"
#include "code_generator/quadruples/Xor.h"
#include "code_generator/quadruples/Return.h"
#include "code_generator/quadruples/Input.h"
#include "code_generator/quadruples/Output.h"
#include "code_generator/quadruples/LvalueAssign.h"
#include "code_generator/quadruples/StartProcedure.h"
#include "code_generator/quadruples/EndProcedure.h"
#include "code_generator/quadruples/StartScope.h"
#include "code_generator/quadruples/EndScope.h"

namespace semantic_analyzer {

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
                std::make_unique<code_generator::Assign>(declarator.getInitializerHolder()->getName(), declarator.getHolder()->getName()));
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
        quadruples.push_back(std::make_unique<code_generator::Argument>(expression->getResultSymbol()->getName()));
    }

    quadruples.push_back(std::make_unique<code_generator::Call>(functionCall.getSymbol()->getName()));
    if (!functionCall.getType().isVoid()) {
        quadruples.push_back(std::make_unique<code_generator::Retrieve>(functionCall.getResultSymbol()->getName()));
    }
}

void CodeGeneratingVisitor::visit(ast::IdentifierExpression&) {
}

void CodeGeneratingVisitor::visit(ast::ConstantExpression& constant) {
    quadruples.push_back(std::make_unique<code_generator::AssignConstant>(constant.getValue(), constant.getResultSymbol()->getName()));
}

void CodeGeneratingVisitor::visit(ast::PostfixExpression& expression) {
    expression.visitOperand(*this);

    if (expression.getOperator()->getLexeme() == "++") {
        quadruples.push_back(std::make_unique<code_generator::Inc>(expression.getResultSymbol()->getName()));
    } else if (expression.getOperator()->getLexeme() == "--") {
        quadruples.push_back(std::make_unique<code_generator::Dec>(expression.getResultSymbol()->getName()));
    }
}

void CodeGeneratingVisitor::visit(ast::PrefixExpression& expression) {
    if (expression.getOperator()->getLexeme() == "++") {
        quadruples.push_back(std::make_unique<code_generator::Inc>(expression.getResultSymbol()->getName()));
    } else if (expression.getOperator()->getLexeme() == "--") {
        quadruples.push_back(std::make_unique<code_generator::Dec>(expression.getResultSymbol()->getName()));
    }

    expression.visitOperand(*this);  // increment before evaluating the expression
}

void CodeGeneratingVisitor::visit(ast::UnaryExpression& expression) {
    expression.visitOperand(*this);

    switch (expression.getOperator()->getLexeme().front()) {
    case '&':
        quadruples.push_back(
                std::make_unique<code_generator::AddressOf>(expression.operandSymbol()->getName(),
                        expression.getResultSymbol()->getName()));
        expression.visitOperand(*this);
        break;
    case '*':
        quadruples.push_back(
                std::make_unique<code_generator::Dereference>(expression.operandSymbol()->getName(),
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
                std::make_unique<code_generator::UnaryMinus>(expression.operandSymbol()->getName(),
                        expression.getResultSymbol()->getName()));
        break;
    case '!':
        expression.visitOperand(*this);
        quadruples.push_back(std::make_unique<code_generator::ZeroCompare>(expression.operandSymbol()->getName()));
        quadruples.push_back(
                std::make_unique<code_generator::Jump>(expression.getTruthyLabel()->getName(), code_generator::JumpCondition::IF_EQUAL));
        quadruples.push_back(std::make_unique<code_generator::AssignConstant>("0", expression.getResultSymbol()->getName()));
        quadruples.push_back(std::make_unique<code_generator::Jump>(expression.getFalsyLabel()->getName()));
        quadruples.push_back(std::make_unique<code_generator::Label>(expression.getTruthyLabel()->getName()));
        quadruples.push_back(
                std::make_unique<code_generator::AssignConstant>("1", expression.getResultSymbol()->getName()));
        quadruples.push_back(std::make_unique<code_generator::Label>(expression.getFalsyLabel()->getName()));
        break;
    default:
        throw std::runtime_error { "Unidentified unary operator: " + expression.getOperator()->getLexeme() };
    }
}

void CodeGeneratingVisitor::visit(ast::TypeCast& expression) {
    expression.visitOperand(*this);
    quadruples.push_back(
            std::make_unique<code_generator::Assign>(expression.operandSymbol()->getName(), expression.getResultSymbol()->getName()));
}

void CodeGeneratingVisitor::visit(ast::ArithmeticExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    switch (expression.getOperator()->getLexeme().front()) {
    case '+':
        quadruples.push_back(std::make_unique<code_generator::Add>(
                expression.leftOperandSymbol()->getName(),
                expression.rightOperandSymbol()->getName(),
                expression.getResultSymbol()->getName()));
        break;
    case '-':
        quadruples.push_back(std::make_unique<code_generator::Sub>(
                expression.leftOperandSymbol()->getName(),
                expression.rightOperandSymbol()->getName(),
                expression.getResultSymbol()->getName()));
        break;
    case '*':
        quadruples.push_back(std::make_unique<code_generator::Mul>(
                expression.leftOperandSymbol()->getName(),
                expression.rightOperandSymbol()->getName(),
                expression.getResultSymbol()->getName()));
        break;
    case '/':
        quadruples.push_back(std::make_unique<code_generator::Div>(
                expression.leftOperandSymbol()->getName(),
                expression.rightOperandSymbol()->getName(),
                expression.getResultSymbol()->getName()));
        break;
    case '%':
        quadruples.push_back(std::make_unique<code_generator::Mod>(
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
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    quadruples.push_back(
            std::make_unique<code_generator::ValueCompare>(expression.leftOperandSymbol()->getName(),
                    expression.rightOperandSymbol()->getName()));

    auto truthyLabel = expression.getTruthyLabel()->getName();
    if (expression.getOperator()->getLexeme() == ">") {
        quadruples.push_back(std::make_unique<code_generator::Jump>(truthyLabel, code_generator::JumpCondition::IF_ABOVE));
    } else if (expression.getOperator()->getLexeme() == "<") {
        quadruples.push_back(std::make_unique<code_generator::Jump>(truthyLabel, code_generator::JumpCondition::IF_BELOW));
    } else if (expression.getOperator()->getLexeme() == "<=") {
        quadruples.push_back(std::make_unique<code_generator::Jump>(truthyLabel, code_generator::JumpCondition::IF_BELOW_OR_EQUAL));
    } else if (expression.getOperator()->getLexeme() == ">=") {
        quadruples.push_back(std::make_unique<code_generator::Jump>(truthyLabel, code_generator::JumpCondition::IF_ABOVE_OR_EQUAL));
    } else if (expression.getOperator()->getLexeme() == "==") {
        quadruples.push_back(std::make_unique<code_generator::Jump>(truthyLabel, code_generator::JumpCondition::IF_EQUAL));
    } else if (expression.getOperator()->getLexeme() == "!=") {
        quadruples.push_back(std::make_unique<code_generator::Jump>(truthyLabel, code_generator::JumpCondition::IF_NOT_EQUAL));
    } else {
        throw std::runtime_error { "unidentified ml_op operator!\n" };
    }

    quadruples.push_back(std::make_unique<code_generator::AssignConstant>("0", expression.getResultSymbol()->getName()));
    quadruples.push_back(std::make_unique<code_generator::Jump>(expression.getFalsyLabel()->getName()));
    quadruples.push_back(std::make_unique<code_generator::Label>(truthyLabel));
    quadruples.push_back(std::make_unique<code_generator::AssignConstant>("1", expression.getResultSymbol()->getName()));
    quadruples.push_back(std::make_unique<code_generator::Label>(expression.getFalsyLabel()->getName()));
}

void CodeGeneratingVisitor::visit(ast::BitwiseExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    switch (expression.getOperator()->getLexeme().front()) {
    case '&':
        quadruples.push_back(std::make_unique<code_generator::And>(
                expression.leftOperandSymbol()->getName(),
                expression.rightOperandSymbol()->getName(),
                expression.getResultSymbol()->getName()));
        break;
    case '|':
        quadruples.push_back(std::make_unique<code_generator::Or>(
                expression.leftOperandSymbol()->getName(),
                expression.rightOperandSymbol()->getName(),
                expression.getResultSymbol()->getName()));
        break;
    case '^':
        quadruples.push_back(std::make_unique<code_generator::Xor>(
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

    quadruples.push_back(std::make_unique<code_generator::AssignConstant>("0", expression.getResultSymbol()->getName()));
    quadruples.push_back(std::make_unique<code_generator::ZeroCompare>(expression.leftOperandSymbol()->getName()));
    quadruples.push_back(
            std::make_unique<code_generator::Jump>(expression.getExitLabel()->getName(), code_generator::JumpCondition::IF_EQUAL));

    expression.visitRightOperand(*this);

    quadruples.push_back(std::make_unique<code_generator::ZeroCompare>(expression.rightOperandSymbol()->getName()));
    quadruples.push_back(
            std::make_unique<code_generator::Jump>(expression.getExitLabel()->getName(), code_generator::JumpCondition::IF_EQUAL));
    quadruples.push_back(std::make_unique<code_generator::AssignConstant>("1", expression.getResultSymbol()->getName()));

    quadruples.push_back(std::make_unique<code_generator::Label>(expression.getExitLabel()->getName()));
}

void CodeGeneratingVisitor::visit(ast::LogicalOrExpression& expression) {
    expression.visitLeftOperand(*this);

    quadruples.push_back(std::make_unique<code_generator::AssignConstant>("1", expression.getResultSymbol()->getName()));
    quadruples.push_back(std::make_unique<code_generator::ZeroCompare>(expression.leftOperandSymbol()->getName()));
    quadruples.push_back(
            std::make_unique<code_generator::Jump>(expression.getExitLabel()->getName(), code_generator::JumpCondition::IF_NOT_EQUAL));

    expression.visitRightOperand(*this);

    quadruples.push_back(std::make_unique<code_generator::ZeroCompare>(expression.rightOperandSymbol()->getName()));
    quadruples.push_back(
            std::make_unique<code_generator::Jump>(expression.getExitLabel()->getName(), code_generator::JumpCondition::IF_NOT_EQUAL));
    quadruples.push_back(std::make_unique<code_generator::AssignConstant>("0", expression.getResultSymbol()->getName()));

    quadruples.push_back(std::make_unique<code_generator::Label>(expression.getExitLabel()->getName()));
}

void CodeGeneratingVisitor::visit(ast::AssignmentExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    auto assignmentOperator = expression.getOperator();
    if (assignmentOperator->getLexeme() == "+=")
        quadruples.push_back(std::make_unique<code_generator::Add>(
                expression.getResultSymbol()->getName(),
                expression.rightOperandSymbol()->getName(),
                expression.getResultSymbol()->getName()));
    else if (assignmentOperator->getLexeme() == "-=")
        quadruples.push_back(std::make_unique<code_generator::Sub>(
                expression.getResultSymbol()->getName(),
                expression.rightOperandSymbol()->getName(),
                expression.getResultSymbol()->getName()));
    else if (assignmentOperator->getLexeme() == "*=")
        quadruples.push_back(std::make_unique<code_generator::Mul>(
                expression.getResultSymbol()->getName(),
                expression.rightOperandSymbol()->getName(),
                expression.getResultSymbol()->getName()));
    else if (assignmentOperator->getLexeme() == "/=")
        quadruples.push_back(std::make_unique<code_generator::Div>(
                expression.getResultSymbol()->getName(),
                expression.rightOperandSymbol()->getName(),
                expression.getResultSymbol()->getName()));
    else if (assignmentOperator->getLexeme() == "%=")
        quadruples.push_back(std::make_unique<code_generator::Mod>(
                expression.getResultSymbol()->getName(),
                expression.rightOperandSymbol()->getName(),
                expression.getResultSymbol()->getName()));
    else if (assignmentOperator->getLexeme() == "&=")
        quadruples.push_back(std::make_unique<code_generator::And>(
                expression.getResultSymbol()->getName(),
                expression.rightOperandSymbol()->getName(),
                expression.getResultSymbol()->getName()));
    else if (assignmentOperator->getLexeme() == "^=")
        quadruples.push_back(std::make_unique<code_generator::Xor>(
                expression.getResultSymbol()->getName(),
                expression.rightOperandSymbol()->getName(),
                expression.getResultSymbol()->getName()));
    else if (assignmentOperator->getLexeme() == "|=")
        quadruples.push_back(std::make_unique<code_generator::Or>(
                expression.getResultSymbol()->getName(),
                expression.rightOperandSymbol()->getName(),
                expression.getResultSymbol()->getName()));
    else if (assignmentOperator->getLexeme() == "<<=") {
        throw std::runtime_error { "shift operations not implemented yet" };
    } else if (assignmentOperator->getLexeme() == ">>=") {
        throw std::runtime_error { "shift operations not implemented yet" };
    } else if (assignmentOperator->getLexeme() == "=") {
        if (expression.leftOperandLvalueSymbol()) {
            quadruples.push_back(std::make_unique<code_generator::LvalueAssign>(expression.rightOperandSymbol()->getName(),
                    expression.leftOperandLvalueSymbol()->getName()));
        } else {
            quadruples.push_back(std::make_unique<code_generator::Assign>(expression.rightOperandSymbol()->getName(),
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
    quadruples.push_back(std::make_unique<code_generator::Return>(statement.returnExpression->getResultSymbol()->getName()));
}

void CodeGeneratingVisitor::visit(ast::IOStatement& statement) {
    statement.expression->accept(*this);
    if (statement.ioKeyword.value == "output") {
        quadruples.push_back(std::make_unique<code_generator::Output>(statement.expression->getResultSymbol()->getName()));
    } else if (statement.ioKeyword.value == "input") {
        quadruples.push_back(std::make_unique<code_generator::Input>(statement.expression->getResultSymbol()->getName()));
    } else {
        throw std::runtime_error { "bad IO statement: " + statement.ioKeyword.type };
    }
}

void CodeGeneratingVisitor::visit(ast::IfStatement& statement) {
    statement.testExpression->accept(*this);

    quadruples.push_back(std::make_unique<code_generator::ZeroCompare>(statement.testExpression->getResultSymbol()->getName()));
    quadruples.push_back(
            std::make_unique<code_generator::Jump>(statement.getFalsyLabel()->getName(), code_generator::JumpCondition::IF_EQUAL));

    statement.body->accept(*this);

    quadruples.push_back(std::make_unique<code_generator::Label>(statement.getFalsyLabel()->getName()));
}

void CodeGeneratingVisitor::visit(ast::IfElseStatement& statement) {
    statement.testExpression->accept(*this);

    quadruples.push_back(std::make_unique<code_generator::ZeroCompare>(statement.testExpression->getResultSymbol()->getName()));
    quadruples.push_back(
            std::make_unique<code_generator::Jump>(statement.getFalsyLabel()->getName(), code_generator::JumpCondition::IF_EQUAL));

    statement.truthyBody->accept(*this);
    quadruples.push_back(std::make_unique<code_generator::Jump>(statement.getExitLabel()->getName()));
    quadruples.push_back(std::make_unique<code_generator::Label>(statement.getFalsyLabel()->getName()));

    statement.falsyBody->accept(*this);
    quadruples.push_back(std::make_unique<code_generator::Label>(statement.getExitLabel()->getName()));
}

void CodeGeneratingVisitor::visit(ast::LoopStatement& loop) {
    loop.header->accept(*this);
    loop.body->accept(*this);
    // FIXME:
    if (loop.header->increment) {
        loop.header->increment->accept(*this);
    }

    quadruples.push_back(std::make_unique<code_generator::Jump>(loop.header->getLoopEntry()->getName()));
    quadruples.push_back(std::make_unique<code_generator::Label>(loop.header->getLoopExit()->getName()));
}

void CodeGeneratingVisitor::visit(ast::ForLoopHeader& loopHeader) {
    loopHeader.initialization->accept(*this);

    quadruples.push_back(std::make_unique<code_generator::Label>(loopHeader.getLoopEntry()->getName()));
    loopHeader.clause->accept(*this);
    quadruples.push_back(std::make_unique<code_generator::ZeroCompare>(loopHeader.clause->getResultSymbol()->getName()));
    quadruples.push_back(
            std::make_unique<code_generator::Jump>(loopHeader.getLoopExit()->getName(), code_generator::JumpCondition::IF_EQUAL));
}

void CodeGeneratingVisitor::visit(ast::WhileLoopHeader& loopHeader) {
    quadruples.push_back(std::make_unique<code_generator::Label>(loopHeader.getLoopEntry()->getName()));
    loopHeader.clause->accept(*this);
    quadruples.push_back(std::make_unique<code_generator::ZeroCompare>(loopHeader.clause->getResultSymbol()->getName()));
    quadruples.push_back(
            std::make_unique<code_generator::Jump>(loopHeader.getLoopExit()->getName(), code_generator::JumpCondition::IF_EQUAL));
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

    quadruples.push_back(std::make_unique<code_generator::StartProcedure>(function.getSymbol()->getName()));
    function.visitBody(*this);
    quadruples.push_back(std::make_unique<code_generator::EndProcedure>(function.getSymbol()->getName()));
}

void CodeGeneratingVisitor::visit(ast::Block& block) {
    std::vector<code_generator::Value> values;
    for (auto& valueSymbol : block.getSymbols()) {
        values.push_back( {
                valueSymbol.second.getName(),
                valueSymbol.second.getIndex(),
                // FIXME:
                code_generator::Type::INTEGRAL,
                false
        });
    }
    std::vector<code_generator::Value> arguments;
    for (auto& argumentSymbol : block.getArguments()) {
        arguments.push_back( {
                argumentSymbol.second.getName(),
                argumentSymbol.second.getIndex(),
                // FIXME:
                code_generator::Type::INTEGRAL,
                true
        });
    }
    quadruples.push_back(std::make_unique<code_generator::StartScope>(values, arguments));
    block.visitChildren(*this);
    quadruples.push_back(std::make_unique<code_generator::EndScope>(block.getSymbols().size()));
}

std::vector<std::unique_ptr<code_generator::Quadruple>> CodeGeneratingVisitor::getQuadruples() {
    // FIXME: temporary:
    std::cout << "\nvisitor quadruples\n";
    for (auto& quadruple : quadruples) {
        std::cout << *quadruple;
    }
    std::cout << "visitor quadruples end\n\n";
    return std::move(quadruples);
}

} /* namespace semantic_analyzer */

