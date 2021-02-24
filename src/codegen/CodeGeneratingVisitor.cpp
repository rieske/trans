#include "CodeGeneratingVisitor.h"

#include <iostream>
#include <stdexcept>

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
#include "quadruples/VoidReturn.h"
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
        instructions.push_back(std::make_unique<Assign>(declarator.getInitializerHolder()->getName(), declarator.getHolder()->getName()));
    }
}

void CodeGeneratingVisitor::visit(ast::ArrayAccess& arrayAccess) {
    arrayAccess.visitLeftOperand(*this);
    arrayAccess.visitRightOperand(*this);

    // TODO: not implemented yet
    //auto offset = arrayAccess.rightOperandSymbol();
    //quadruples.push_back( { code_generator::INDEX, arrayAccess.leftOperandSymbol(), offset, arrayAccess.getResultSymbol() });
    //quadruples.push_back( { code_generator::INDEX_ADDR, arrayAccess.leftOperandSymbol(), offset, arrayAccess.getLvalue() });
}

void CodeGeneratingVisitor::visit(ast::FunctionCall& functionCall) {
    functionCall.visitOperand(*this);
    functionCall.visitArguments(*this);

    for (auto& expression : functionCall.getArgumentList()) {
        instructions.push_back(std::make_unique<Argument>(expression->getResultSymbol()->getName()));
    }

    instructions.push_back(std::make_unique<Call>(functionCall.getSymbol()->getName()));
    if (!functionCall.getType().isVoid()) {
        instructions.push_back(std::make_unique<Retrieve>(functionCall.getResultSymbol()->getName()));
    }
}

void CodeGeneratingVisitor::visit(ast::IdentifierExpression&) {
}

void CodeGeneratingVisitor::visit(ast::ConstantExpression& constant) {
    instructions.push_back(std::make_unique<AssignConstant>(constant.getValue(), constant.getResultSymbol()->getName()));
}

void CodeGeneratingVisitor::visit(ast::StringLiteralExpression& stringLiteral) {
    instructions.push_back(
        std::make_unique<AssignConstant>(stringLiteral.getConstantSymbol(), stringLiteral.getResultSymbol()->getName())
    );
}

void CodeGeneratingVisitor::visit(ast::PostfixExpression& expression) {
    expression.visitOperand(*this);

    auto resultSymbolName = expression.getResultSymbol()->getName();
    auto preOperationSymbol = expression.getPreOperationSymbol()->getName();
    instructions.push_back(std::make_unique<Assign>(resultSymbolName, preOperationSymbol));

    if (expression.getOperator()->getLexeme() == "++") {
        instructions.push_back(std::make_unique<Inc>(resultSymbolName));
    } else if (expression.getOperator()->getLexeme() == "--") {
        instructions.push_back(std::make_unique<Dec>(resultSymbolName));
    }

    expression.setResultSymbol(*expression.getPreOperationSymbol());
}

void CodeGeneratingVisitor::visit(ast::PrefixExpression& expression) {
    expression.visitOperand(*this);

    if (expression.getOperator()->getLexeme() == "++") {
        instructions.push_back(std::make_unique<Inc>(expression.getResultSymbol()->getName()));
    } else if (expression.getOperator()->getLexeme() == "--") {
        instructions.push_back(std::make_unique<Dec>(expression.getResultSymbol()->getName()));
    }
}

void CodeGeneratingVisitor::visit(ast::UnaryExpression& expression) {
    expression.visitOperand(*this);

    switch (expression.getOperator()->getLexeme().front()) {
    case '&':
        instructions.push_back(std::make_unique<AddressOf>(expression.operandSymbol()->getName(), expression.getResultSymbol()->getName()));
        expression.visitOperand(*this);
        break;
    case '*':
        instructions.push_back(std::make_unique<Dereference>(expression.operandSymbol()->getName(), expression.getLvalueSymbol()->getName(),
                                                             expression.getResultSymbol()->getName()));
        expression.visitOperand(*this);
        break;
    case '+':
        expression.visitOperand(*this);
        break;
    case '-':
        expression.visitOperand(*this);
        instructions.push_back(std::make_unique<UnaryMinus>(expression.operandSymbol()->getName(), expression.getResultSymbol()->getName()));
        break;
    case '!':
        expression.visitOperand(*this);
        instructions.push_back(std::make_unique<ZeroCompare>(expression.operandSymbol()->getName()));
        instructions.push_back(std::make_unique<Jump>(expression.getTruthyLabel()->getName(), JumpCondition::IF_EQUAL));
        instructions.push_back(std::make_unique<AssignConstant>("0", expression.getResultSymbol()->getName()));
        instructions.push_back(std::make_unique<Jump>(expression.getFalsyLabel()->getName()));
        instructions.push_back(std::make_unique<Label>(expression.getTruthyLabel()->getName()));
        instructions.push_back(std::make_unique<AssignConstant>("1", expression.getResultSymbol()->getName()));
        instructions.push_back(std::make_unique<Label>(expression.getFalsyLabel()->getName()));
        break;
    default:
        throw std::runtime_error { "Unidentified unary operator: " + expression.getOperator()->getLexeme() };
    }
}

void CodeGeneratingVisitor::visit(ast::TypeCast& expression) {
    expression.visitOperand(*this);
    instructions.push_back(std::make_unique<Assign>(expression.operandSymbol()->getName(), expression.getResultSymbol()->getName()));
}

void CodeGeneratingVisitor::visit(ast::ArithmeticExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    switch (expression.getOperator()->getLexeme().front()) {
    case '+':
        instructions.push_back(std::make_unique<Add>(expression.leftOperandSymbol()->getName(), expression.rightOperandSymbol()->getName(),
                                                     expression.getResultSymbol()->getName()));
        break;
    case '-':
        instructions.push_back(std::make_unique<Sub>(expression.leftOperandSymbol()->getName(), expression.rightOperandSymbol()->getName(),
                                                     expression.getResultSymbol()->getName()));
        break;
    case '*':
        instructions.push_back(std::make_unique<Mul>(expression.leftOperandSymbol()->getName(), expression.rightOperandSymbol()->getName(),
                                                     expression.getResultSymbol()->getName()));
        break;
    case '/':
        instructions.push_back(std::make_unique<Div>(expression.leftOperandSymbol()->getName(), expression.rightOperandSymbol()->getName(),
                                                     expression.getResultSymbol()->getName()));
        break;
    case '%':
        instructions.push_back(std::make_unique<Mod>(expression.leftOperandSymbol()->getName(), expression.rightOperandSymbol()->getName(),
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

    instructions.push_back(std::make_unique<ValueCompare>(expression.leftOperandSymbol()->getName(), expression.rightOperandSymbol()->getName()));

    auto truthyLabel = expression.getTruthyLabel()->getName();
    if (expression.getOperator()->getLexeme() == ">") {
        instructions.push_back(std::make_unique<Jump>(truthyLabel, JumpCondition::IF_ABOVE));
    } else if (expression.getOperator()->getLexeme() == "<") {
        instructions.push_back(std::make_unique<Jump>(truthyLabel, JumpCondition::IF_BELOW));
    } else if (expression.getOperator()->getLexeme() == "<=") {
        instructions.push_back(std::make_unique<Jump>(truthyLabel, JumpCondition::IF_BELOW_OR_EQUAL));
    } else if (expression.getOperator()->getLexeme() == ">=") {
        instructions.push_back(std::make_unique<Jump>(truthyLabel, JumpCondition::IF_ABOVE_OR_EQUAL));
    } else if (expression.getOperator()->getLexeme() == "==") {
        instructions.push_back(std::make_unique<Jump>(truthyLabel, JumpCondition::IF_EQUAL));
    } else if (expression.getOperator()->getLexeme() == "!=") {
        instructions.push_back(std::make_unique<Jump>(truthyLabel, JumpCondition::IF_NOT_EQUAL));
    } else {
        throw std::runtime_error { "unidentified ml_op operator!\n" };
    }

    instructions.push_back(std::make_unique<AssignConstant>("0", expression.getResultSymbol()->getName()));
    instructions.push_back(std::make_unique<Jump>(expression.getFalsyLabel()->getName()));
    instructions.push_back(std::make_unique<Label>(truthyLabel));
    instructions.push_back(std::make_unique<AssignConstant>("1", expression.getResultSymbol()->getName()));
    instructions.push_back(std::make_unique<Label>(expression.getFalsyLabel()->getName()));
}

void CodeGeneratingVisitor::visit(ast::BitwiseExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    switch (expression.getOperator()->getLexeme().front()) {
    case '&':
        instructions.push_back(std::make_unique<And>(expression.leftOperandSymbol()->getName(), expression.rightOperandSymbol()->getName(),
                                                     expression.getResultSymbol()->getName()));
        break;
    case '|':
        instructions.push_back(std::make_unique<Or>(expression.leftOperandSymbol()->getName(), expression.rightOperandSymbol()->getName(),
                                                    expression.getResultSymbol()->getName()));
        break;
    case '^':
        instructions.push_back(std::make_unique<Xor>(expression.leftOperandSymbol()->getName(), expression.rightOperandSymbol()->getName(),
                                                     expression.getResultSymbol()->getName()));
        break;
    default:
        throw std::runtime_error { "no semantic actions defined for bitwise operator: " + expression.getOperator()->getLexeme() };
    }
}

void CodeGeneratingVisitor::visit(ast::LogicalAndExpression& expression) {
    expression.visitLeftOperand(*this);

    instructions.push_back(std::make_unique<AssignConstant>("0", expression.getResultSymbol()->getName()));
    instructions.push_back(std::make_unique<ZeroCompare>(expression.leftOperandSymbol()->getName()));
    instructions.push_back(std::make_unique<Jump>(expression.getExitLabel()->getName(), JumpCondition::IF_EQUAL));

    expression.visitRightOperand(*this);

    instructions.push_back(std::make_unique<ZeroCompare>(expression.rightOperandSymbol()->getName()));
    instructions.push_back(std::make_unique<Jump>(expression.getExitLabel()->getName(), JumpCondition::IF_EQUAL));
    instructions.push_back(std::make_unique<AssignConstant>("1", expression.getResultSymbol()->getName()));

    instructions.push_back(std::make_unique<Label>(expression.getExitLabel()->getName()));
}

void CodeGeneratingVisitor::visit(ast::LogicalOrExpression& expression) {
    expression.visitLeftOperand(*this);

    instructions.push_back(std::make_unique<AssignConstant>("1", expression.getResultSymbol()->getName()));
    instructions.push_back(std::make_unique<ZeroCompare>(expression.leftOperandSymbol()->getName()));
    instructions.push_back(std::make_unique<Jump>(expression.getExitLabel()->getName(), JumpCondition::IF_NOT_EQUAL));

    expression.visitRightOperand(*this);

    instructions.push_back(std::make_unique<ZeroCompare>(expression.rightOperandSymbol()->getName()));
    instructions.push_back(std::make_unique<Jump>(expression.getExitLabel()->getName(), JumpCondition::IF_NOT_EQUAL));
    instructions.push_back(std::make_unique<AssignConstant>("0", expression.getResultSymbol()->getName()));

    instructions.push_back(std::make_unique<Label>(expression.getExitLabel()->getName()));
}

void CodeGeneratingVisitor::visit(ast::AssignmentExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    auto assignmentOperator = expression.getOperator();
    if (assignmentOperator->getLexeme() == "+=")
        instructions.push_back(std::make_unique<Add>(expression.getResultSymbol()->getName(), expression.rightOperandSymbol()->getName(),
                                                     expression.getResultSymbol()->getName()));
    else if (assignmentOperator->getLexeme() == "-=")
        instructions.push_back(std::make_unique<Sub>(expression.getResultSymbol()->getName(), expression.rightOperandSymbol()->getName(),
                                                     expression.getResultSymbol()->getName()));
    else if (assignmentOperator->getLexeme() == "*=")
        instructions.push_back(std::make_unique<Mul>(expression.getResultSymbol()->getName(), expression.rightOperandSymbol()->getName(),
                                                     expression.getResultSymbol()->getName()));
    else if (assignmentOperator->getLexeme() == "/=")
        instructions.push_back(std::make_unique<Div>(expression.getResultSymbol()->getName(), expression.rightOperandSymbol()->getName(),
                                                     expression.getResultSymbol()->getName()));
    else if (assignmentOperator->getLexeme() == "%=")
        instructions.push_back(std::make_unique<Mod>(expression.getResultSymbol()->getName(), expression.rightOperandSymbol()->getName(),
                                                     expression.getResultSymbol()->getName()));
    else if (assignmentOperator->getLexeme() == "&=")
        instructions.push_back(std::make_unique<And>(expression.getResultSymbol()->getName(), expression.rightOperandSymbol()->getName(),
                                                     expression.getResultSymbol()->getName()));
    else if (assignmentOperator->getLexeme() == "^=")
        instructions.push_back(std::make_unique<Xor>(expression.getResultSymbol()->getName(), expression.rightOperandSymbol()->getName(),
                                                     expression.getResultSymbol()->getName()));
    else if (assignmentOperator->getLexeme() == "|=")
        instructions.push_back(
            std::make_unique<Or>(expression.getResultSymbol()->getName(), expression.rightOperandSymbol()->getName(), expression.getResultSymbol()->getName()));
    else if (assignmentOperator->getLexeme() == "<<=") {
        throw std::runtime_error { "shift operations not implemented yet" };
    } else if (assignmentOperator->getLexeme() == ">>=") {
        throw std::runtime_error { "shift operations not implemented yet" };
    } else if (assignmentOperator->getLexeme() == "=") {
        if (expression.leftOperandLvalueSymbol()) {
            instructions.push_back(std::make_unique<LvalueAssign>(expression.rightOperandSymbol()->getName(), expression.leftOperandLvalueSymbol()->getName()));
        } else {
            instructions.push_back(std::make_unique<Assign>(expression.rightOperandSymbol()->getName(), expression.getResultSymbol()->getName()));
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
    instructions.push_back(std::make_unique<Return>(statement.returnExpression->getResultSymbol()->getName()));
}

void CodeGeneratingVisitor::visit(ast::VoidReturnStatement &statement) { instructions.push_back(std::make_unique<VoidReturn>()); }

void CodeGeneratingVisitor::visit(ast::IfStatement& statement) {
    statement.testExpression->accept(*this);

    instructions.push_back(std::make_unique<ZeroCompare>(statement.testExpression->getResultSymbol()->getName()));
    instructions.push_back(std::make_unique<Jump>(statement.getFalsyLabel()->getName(), JumpCondition::IF_EQUAL));

    statement.body->accept(*this);

    instructions.push_back(std::make_unique<Label>(statement.getFalsyLabel()->getName()));
}

void CodeGeneratingVisitor::visit(ast::IfElseStatement& statement) {
    statement.testExpression->accept(*this);

    instructions.push_back(std::make_unique<ZeroCompare>(statement.testExpression->getResultSymbol()->getName()));
    instructions.push_back(std::make_unique<Jump>(statement.getFalsyLabel()->getName(), JumpCondition::IF_EQUAL));

    statement.truthyBody->accept(*this);
    instructions.push_back(std::make_unique<Jump>(statement.getExitLabel()->getName()));
    instructions.push_back(std::make_unique<Label>(statement.getFalsyLabel()->getName()));

    statement.falsyBody->accept(*this);
    instructions.push_back(std::make_unique<Label>(statement.getExitLabel()->getName()));
}

void CodeGeneratingVisitor::visit(ast::LoopStatement& loop) {
    loop.header->accept(*this);
    loop.body->accept(*this);
    // FIXME:
    if (loop.header->increment) {
        loop.header->increment->accept(*this);
    }

    instructions.push_back(std::make_unique<Jump>(loop.header->getLoopEntry()->getName()));
    instructions.push_back(std::make_unique<Label>(loop.header->getLoopExit()->getName()));
}

void CodeGeneratingVisitor::visit(ast::ForLoopHeader& loopHeader) {
    loopHeader.initialization->accept(*this);

    instructions.push_back(std::make_unique<Label>(loopHeader.getLoopEntry()->getName()));
    loopHeader.clause->accept(*this);
    instructions.push_back(std::make_unique<ZeroCompare>(loopHeader.clause->getResultSymbol()->getName()));
    instructions.push_back(std::make_unique<Jump>(loopHeader.getLoopExit()->getName(), JumpCondition::IF_EQUAL));
}

void CodeGeneratingVisitor::visit(ast::WhileLoopHeader& loopHeader) {
    instructions.push_back(std::make_unique<Label>(loopHeader.getLoopEntry()->getName()));
    loopHeader.clause->accept(*this);
    instructions.push_back(std::make_unique<ZeroCompare>(loopHeader.clause->getResultSymbol()->getName()));
    instructions.push_back(std::make_unique<Jump>(loopHeader.getLoopExit()->getName(), JumpCondition::IF_EQUAL));
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
    instructions.push_back(std::make_unique<StartProcedure>(function.getSymbol()->getName(), std::move(values), std::move(arguments)));

    auto instructionsBak = std::move(instructions);
    function.visitBody(*this);
    auto functionBody = toBasicBlocks(std::move(instructions));
    for (auto& bb : functionBody) {
        if (!bb->terminates()) {
            bb->appendInstruction(std::make_unique<VoidReturn>());
        }
        instructionsBak.push_back(std::move(bb));
    }
    instructions = std::move(instructionsBak);

    instructions.push_back(std::make_unique<EndProcedure>(function.getSymbol()->getName()));
}

void CodeGeneratingVisitor::visit(ast::Block& block) {
    block.visitChildren(*this);
}

std::vector<std::unique_ptr<Quadruple>> CodeGeneratingVisitor::getQuadruples() {
    return std::move(instructions);
}

std::vector<std::unique_ptr<BasicBlock>> toBasicBlocks(std::vector<std::unique_ptr<Quadruple>> instructions) {
    std::vector<std::unique_ptr<BasicBlock>> basicBlocks {};

    std::unique_ptr<BasicBlock> bb = std::make_unique<BasicBlock>();

    std::vector<std::unique_ptr<Quadruple>> bbInstructions;
    for (auto& instruction : instructions) {
        if (bb->terminates() || instruction->isLabel()) {
            basicBlocks.push_back(std::move(bb));
            bb = std::make_unique<BasicBlock>();
            basicBlocks.back()->setSuccessor(bb.get());
        }
        bb->appendInstruction(std::move(instruction));
    }
    basicBlocks.push_back(std::move(bb));

    return basicBlocks;
}

} // namespace codegen

