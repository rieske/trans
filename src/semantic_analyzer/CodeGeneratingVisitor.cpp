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
#include "../ast/DeclarationList.h"
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
#include "../ast/PointerCast.h"
#include "../ast/PostfixExpression.h"
#include "../ast/PrefixExpression.h"
#include "../ast/ReturnStatement.h"
#include "../ast/IdentifierExpression.h"
#include "../ast/ConstantExpression.h"
#include "../ast/TerminalSymbol.h"
#include "../ast/types/Type.h"
#include "../ast/UnaryExpression.h"
#include "../ast/VariableDeclaration.h"
#include "../ast/VariableDefinition.h"
#include "../ast/WhileLoopHeader.h"
#include "ast/ExpressionList.h"
#include "ast/LogicalAndExpression.h"
#include "ast/LogicalOrExpression.h"
#include "ast/ShiftExpression.h"
#include "ast/TypeCast.h"
#include "ast/AssignmentExpression.h"
#include "ast/Declarator.h"

#include "code_generator/ValueEntry.h"
#include "code_generator/LabelEntry.h"

namespace semantic_analyzer {

CodeGeneratingVisitor::CodeGeneratingVisitor() {
}

CodeGeneratingVisitor::~CodeGeneratingVisitor() {
    // FIXME: temporary:
    std::cout << "\nvisitor quadruples\n";
    for (auto quadruple : quadruples) {
        quadruple.output(std::cout);
    }
    std::cout << "visitor quadruples end\n\n";
}

void CodeGeneratingVisitor::visit(ast::DeclarationSpecifiers& declarationSpecifiers) {
    throw std::runtime_error { "CodeGeneratingVisitor::visit(ast::DeclarationSpecifiers& declarationSpecifiers) not implemented" };
}

void CodeGeneratingVisitor::visit(ast::Declaration& declaration) {
    throw std::runtime_error { "CodeGeneratingVisitor::visit(ast::Declaration& declaration) not implemented" };
}

void CodeGeneratingVisitor::visit(ast::DeclarationList& declarations) {
    for (auto& declaration : declarations.getDeclarations()) {
        declaration->accept(*this);
    }
}

void CodeGeneratingVisitor::visit(ast::ArrayAccess& arrayAccess) {
    arrayAccess.visitLeftOperand(*this);
    arrayAccess.visitRightOperand(*this);

    auto offset = arrayAccess.rightOperandSymbol();
    quadruples.push_back( { code_generator::INDEX, arrayAccess.leftOperandSymbol(), offset, arrayAccess.getResultSymbol() });
    quadruples.push_back( { code_generator::INDEX_ADDR, arrayAccess.leftOperandSymbol(), offset, arrayAccess.getLvalue() });
}

void CodeGeneratingVisitor::visit(ast::FunctionCall& functionCall) {
    functionCall.visitOperand(*this);
    functionCall.visitArguments(*this);

    for (auto& expression : functionCall.getArgumentList()) {
        quadruples.push_back( { code_generator::PARAM, expression->getResultSymbol(), nullptr, nullptr });
    }

    quadruples.push_back( { code_generator::CALL, functionCall.getSymbol() });
    if (!functionCall.getType().isPlainVoid()) {
        quadruples.push_back( { code_generator::RETRIEVE, functionCall.getResultSymbol(), nullptr, nullptr });
    }
}

void CodeGeneratingVisitor::visit(ast::IdentifierExpression&) {
}

void CodeGeneratingVisitor::visit(ast::ConstantExpression& constant) {
    quadruples.push_back( { constant.getValue(), constant.getResultSymbol() });
}

void CodeGeneratingVisitor::visit(ast::PostfixExpression& expression) {
    expression.visitOperand(*this);

    if (expression.getOperator()->getLexeme() == "++") {
        quadruples.push_back( { code_generator::INC, expression.getResultSymbol(), nullptr, expression.getResultSymbol() });
    } else if (expression.getOperator()->getLexeme() == "--") {
        quadruples.push_back( { code_generator::DEC, expression.getResultSymbol(), nullptr, expression.getResultSymbol() });
    }
}

void CodeGeneratingVisitor::visit(ast::PrefixExpression& expression) {
    if (expression.getOperator()->getLexeme() == "++") {
        quadruples.push_back( { code_generator::INC, expression.getResultSymbol(), nullptr, expression.getResultSymbol() });
    } else if (expression.getOperator()->getLexeme() == "--") {
        quadruples.push_back( { code_generator::DEC, expression.getResultSymbol(), nullptr, expression.getResultSymbol() });
    }

    expression.visitOperand(*this);  // increment before evaluating the expression
}

void CodeGeneratingVisitor::visit(ast::UnaryExpression& expression) {
    expression.visitOperand(*this);

    switch (expression.getOperator()->getLexeme().front()) {
    case '&':
        quadruples.push_back( { code_generator::ADDR, expression.operandSymbol(), nullptr, expression.getResultSymbol() });
        expression.visitOperand(*this);
        break;
    case '*':
        quadruples.push_back( { code_generator::DEREF, expression.operandSymbol(), nullptr, expression.getResultSymbol() });
        expression.visitOperand(*this);
        break;
    case '+':
        expression.visitOperand(*this);
        break;
    case '-':
        expression.visitOperand(*this);
        quadruples.push_back( { code_generator::UMINUS, expression.operandSymbol(), nullptr, expression.getResultSymbol() });
        break;
    case '!':
        expression.visitOperand(*this);
        quadruples.push_back( { code_generator::CMP, expression.operandSymbol(), nullptr, expression.getResultSymbol() });
        quadruples.push_back( { code_generator::JE, expression.getTruthyLabel() });
        quadruples.push_back( { "0", expression.getResultSymbol() });
        quadruples.push_back( { code_generator::GOTO, expression.getFalsyLabel() });
        quadruples.push_back( { code_generator::LABEL, expression.getTruthyLabel() });
        quadruples.push_back( { "1", expression.getResultSymbol() });
        quadruples.push_back( { code_generator::LABEL, expression.getFalsyLabel() });
        break;
    default:
        throw std::runtime_error { "Unidentified increment operator: " + expression.getOperator()->getLexeme() };
    }
}

void CodeGeneratingVisitor::visit(ast::TypeCast& expression) {
    expression.visitOperand(*this);

    quadruples.push_back( { code_generator::ASSIGN, expression.operandSymbol(), nullptr, expression.getResultSymbol() });
}

void CodeGeneratingVisitor::visit(ast::PointerCast& expression) {
    expression.getPointer()->accept(*this);
    expression.visitOperand(*this);

    quadruples.push_back( { code_generator::ASSIGN, expression.operandSymbol(), nullptr, expression.getResultSymbol() });
}

void CodeGeneratingVisitor::visit(ast::ArithmeticExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    switch (expression.getOperator()->getLexeme().front()) {
    case '+':
        quadruples.push_back( { code_generator::ADD, expression.leftOperandSymbol(), expression.rightOperandSymbol(), expression.getResultSymbol() });
        break;
    case '-':
        quadruples.push_back( { code_generator::SUB, expression.leftOperandSymbol(), expression.rightOperandSymbol(), expression.getResultSymbol() });
        break;
    case '*':
        quadruples.push_back( { code_generator::MUL, expression.leftOperandSymbol(), expression.rightOperandSymbol(), expression.getResultSymbol() });
        break;
    case '/':
        quadruples.push_back( { code_generator::DIV, expression.leftOperandSymbol(), expression.rightOperandSymbol(), expression.getResultSymbol() });
        break;
    case '%':
        quadruples.push_back( { code_generator::MOD, expression.leftOperandSymbol(), expression.rightOperandSymbol(), expression.getResultSymbol() });
        break;
    default:
        throw std::runtime_error { "unidentified arithmetic operator: " + expression.getOperator()->getLexeme() };
    }
}

void CodeGeneratingVisitor::visit(ast::ShiftExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    switch (expression.getOperator()->getLexeme().front()) {
    case '<':   // <<
        quadruples.push_back( { code_generator::SHL, expression.leftOperandSymbol(), expression.rightOperandSymbol(), expression.getResultSymbol() });
        break;
    case '>':   // >>
        quadruples.push_back( { code_generator::SHR, expression.leftOperandSymbol(), expression.rightOperandSymbol(), expression.getResultSymbol() });
        break;
    default:
        throw std::runtime_error { "unidentified add_op operator!" };
    }
}

void CodeGeneratingVisitor::visit(ast::ComparisonExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    quadruples.push_back( { code_generator::CMP, expression.leftOperandSymbol(), expression.rightOperandSymbol(), nullptr });

    auto truthyLabel = expression.getTruthyLabel();
    if (expression.getOperator()->getLexeme() == ">") {
        quadruples.push_back( { code_generator::JA, truthyLabel });
    } else if (expression.getOperator()->getLexeme() == "<") {
        quadruples.push_back( { code_generator::JB, truthyLabel });
    } else if (expression.getOperator()->getLexeme() == "<=") {
        quadruples.push_back( { code_generator::JBE, truthyLabel });
    } else if (expression.getOperator()->getLexeme() == ">=") {
        quadruples.push_back( { code_generator::JAE, truthyLabel });
    } else if (expression.getOperator()->getLexeme() == "==") {
        quadruples.push_back( { code_generator::JE, truthyLabel });
    } else if (expression.getOperator()->getLexeme() == "!=") {
        quadruples.push_back( { code_generator::JNE, truthyLabel });
    } else {
        throw std::runtime_error { "unidentified ml_op operator!\n" };
    }

    quadruples.push_back( { "0", expression.getResultSymbol() });
    quadruples.push_back( { code_generator::GOTO, expression.getFalsyLabel() });
    quadruples.push_back( { code_generator::LABEL, truthyLabel });
    quadruples.push_back( { "1", expression.getResultSymbol() });
    quadruples.push_back( { code_generator::LABEL, expression.getFalsyLabel() });
}

void CodeGeneratingVisitor::visit(ast::BitwiseExpression& expression) {
    expression.visitLeftOperand(*this);
    expression.visitRightOperand(*this);

    code_generator::oper quadrupleOperator;
    switch (expression.getOperator()->getLexeme().front()) {
    case '&':
        quadrupleOperator = code_generator::oper::AND;
        break;
    case '|':
        quadrupleOperator = code_generator::oper::OR;
        break;
    case '^':
        quadrupleOperator = code_generator::oper::XOR;
        break;
    default:
        throw std::runtime_error { "no semantic actions defined for bitwise operator: " + expression.getOperator()->getLexeme() };
    }

    quadruples.push_back( { quadrupleOperator, expression.leftOperandSymbol(), expression.rightOperandSymbol(), expression.getResultSymbol() });
}

void CodeGeneratingVisitor::visit(ast::LogicalAndExpression& expression) {
    expression.visitLeftOperand(*this);

    quadruples.push_back( { "0", expression.getResultSymbol() });
    quadruples.push_back( { code_generator::CMP, expression.leftOperandSymbol(), 0, nullptr });
    quadruples.push_back( { code_generator::JE, expression.getExitLabel() });

    expression.visitRightOperand(*this);

    quadruples.push_back( { code_generator::CMP, expression.rightOperandSymbol(), 0, nullptr });
    quadruples.push_back( { code_generator::JE, expression.getExitLabel() });
    quadruples.push_back( { "1", expression.getResultSymbol() });

    quadruples.push_back( { code_generator::LABEL, expression.getExitLabel() });
}

void CodeGeneratingVisitor::visit(ast::LogicalOrExpression& expression) {
    expression.visitLeftOperand(*this);

    quadruples.push_back( { "1", expression.getResultSymbol() });
    quadruples.push_back( { code_generator::CMP, expression.leftOperandSymbol(), 0, nullptr });
    quadruples.push_back( { code_generator::JNE, expression.getExitLabel() });

    expression.visitRightOperand(*this);

    quadruples.push_back( { code_generator::CMP, expression.rightOperandSymbol(), 0, nullptr });
    quadruples.push_back( { code_generator::JNE, expression.getExitLabel() });
    quadruples.push_back( { "0", expression.getResultSymbol() });

    quadruples.push_back( { code_generator::LABEL, expression.getExitLabel() });
}

void CodeGeneratingVisitor::visit(ast::AssignmentExpression& expression) {
    expression.visitLeftOperand(*this);
    code_generator::ValueEntry* dereferencedLocation { nullptr };
    if (quadruples.back().getOp() == code_generator::DEREF) {
        dereferencedLocation = quadruples.back().getArg1();
        quadruples.pop_back();
    }
    expression.visitRightOperand(*this);

    auto assignmentOperator = expression.getOperator();
    if (assignmentOperator->getLexeme() == "+=")
        quadruples.push_back( { code_generator::ADD, expression.getResultSymbol(), expression.rightOperandSymbol(), expression.getResultSymbol() });
    else if (assignmentOperator->getLexeme() == "-=")
        quadruples.push_back( { code_generator::SUB, expression.getResultSymbol(), expression.rightOperandSymbol(), expression.getResultSymbol() });
    else if (assignmentOperator->getLexeme() == "*=")
        quadruples.push_back( { code_generator::MUL, expression.getResultSymbol(), expression.rightOperandSymbol(), expression.getResultSymbol() });
    else if (assignmentOperator->getLexeme() == "/=")
        quadruples.push_back( { code_generator::DIV, expression.getResultSymbol(), expression.rightOperandSymbol(), expression.getResultSymbol() });
    else if (assignmentOperator->getLexeme() == "%=")
        quadruples.push_back( { code_generator::MOD, expression.getResultSymbol(), expression.rightOperandSymbol(), expression.getResultSymbol() });
    else if (assignmentOperator->getLexeme() == "&=")
        quadruples.push_back( { code_generator::AND, expression.getResultSymbol(), expression.rightOperandSymbol(), expression.getResultSymbol() });
    else if (assignmentOperator->getLexeme() == "^=")
        quadruples.push_back( { code_generator::XOR, expression.getResultSymbol(), expression.rightOperandSymbol(), expression.getResultSymbol() });
    else if (assignmentOperator->getLexeme() == "|=")
        quadruples.push_back( { code_generator::OR, expression.getResultSymbol(), expression.rightOperandSymbol(), expression.getResultSymbol() });
    else if (assignmentOperator->getLexeme() == "<<=")
        quadruples.push_back( { code_generator::SHL, expression.getResultSymbol(), expression.rightOperandSymbol(), expression.getResultSymbol() });
    else if (assignmentOperator->getLexeme() == ">>=")
        quadruples.push_back( { code_generator::SHR, expression.getResultSymbol(), expression.rightOperandSymbol(), expression.getResultSymbol() });
    else if (assignmentOperator->getLexeme() == "=") {
        if (dereferencedLocation) {
            // FIXME:
            quadruples.push_back( { code_generator::DEREF_LVAL, expression.rightOperandSymbol(), nullptr, dereferencedLocation });
        } else {
            quadruples.push_back( { code_generator::ASSIGN, expression.rightOperandSymbol(), nullptr, expression.getResultSymbol() });
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
    quadruples.push_back( { code_generator::RETURN, statement.returnExpression->getResultSymbol(), nullptr, nullptr });
}

void CodeGeneratingVisitor::visit(ast::IOStatement& statement) {
    statement.expression->accept(*this);
    if (statement.ioKeyword.value == "output") {
        quadruples.push_back( { code_generator::OUT, statement.expression->getResultSymbol(), nullptr, nullptr });
    } else if (statement.ioKeyword.value == "input") {
        quadruples.push_back( { code_generator::IN, statement.expression->getResultSymbol(), nullptr, nullptr });
    } else {
        throw std::runtime_error { "bad IO statement: " + statement.ioKeyword.type };
    }
}

void CodeGeneratingVisitor::visit(ast::IfStatement& statement) {
    statement.testExpression->accept(*this);
    quadruples.push_back( { code_generator::CMP, statement.testExpression->getResultSymbol(), 0, nullptr });
    quadruples.push_back( { code_generator::JE, statement.getFalsyLabel() });

    statement.body->accept(*this);
    quadruples.push_back( { code_generator::LABEL, statement.getFalsyLabel() });

}

void CodeGeneratingVisitor::visit(ast::IfElseStatement& statement) {
    statement.testExpression->accept(*this);

    quadruples.push_back( { code_generator::CMP, statement.testExpression->getResultSymbol(), 0, nullptr });
    quadruples.push_back( { code_generator::JE, statement.getFalsyLabel() });

    statement.truthyBody->accept(*this);
    quadruples.push_back( { code_generator::GOTO, statement.getExitLabel() });
    quadruples.push_back( { code_generator::LABEL, statement.getFalsyLabel() });

    statement.falsyBody->accept(*this);
    quadruples.push_back( { code_generator::LABEL, statement.getExitLabel() });
}

void CodeGeneratingVisitor::visit(ast::LoopStatement& loop) {
    loop.header->accept(*this);
    loop.body->accept(*this);
    // FIXME:
    if (loop.header->increment) {
        loop.header->increment->accept(*this);
    }

    quadruples.push_back( { code_generator::GOTO, loop.header->getLoopEntry() });
    quadruples.push_back( { code_generator::LABEL, loop.header->getLoopExit() });
}

void CodeGeneratingVisitor::visit(ast::ForLoopHeader& loopHeader) {
    loopHeader.initialization->accept(*this);

    quadruples.push_back( { code_generator::LABEL, loopHeader.getLoopEntry() });
    loopHeader.clause->accept(*this);
    quadruples.push_back( { code_generator::CMP, loopHeader.clause->getResultSymbol(), 0, nullptr });
    quadruples.push_back( { code_generator::JE, loopHeader.getLoopExit() });
}

void CodeGeneratingVisitor::visit(ast::WhileLoopHeader& loopHeader) {
    quadruples.push_back( { code_generator::LABEL, loopHeader.getLoopEntry() });
    loopHeader.clause->accept(*this);
    quadruples.push_back( { code_generator::CMP, loopHeader.clause->getResultSymbol(), 0, nullptr });
    quadruples.push_back( { code_generator::JE, loopHeader.getLoopExit() });
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

    quadruples.push_back( { code_generator::PROC, function.getSymbol() });
    function.visitBody(*this);
    quadruples.push_back( { code_generator::ENDPROC, function.getSymbol() });
}

void CodeGeneratingVisitor::visit(ast::Block& block) {
    quadruples.push_back( { code_generator::SCOPE, block.getSymbols(), block.getArguments() });
    block.visitChildren(*this);
    quadruples.push_back( { code_generator::ENDSCOPE, block.getSymbols(), block.getArguments() });
}

void CodeGeneratingVisitor::visit(ast::VariableDeclaration& declaration) {
    declaration.declaredVariables->accept(*this);
}

void CodeGeneratingVisitor::visit(ast::VariableDefinition& definition) {
    definition.declaration->accept(*this);
    definition.initializerExpression->accept(*this);

    auto& declaredVariables = definition.declaration->declaredVariables->getDeclarations();
    auto& lastVariableInDeclaration = declaredVariables.back();
    // FIXME:
    //quadruples.push_back( { code_generator::ASSIGN, definition.initializerExpression->getResultSymbol(), nullptr, lastVariableInDeclaration->getHolder() });
}

std::vector<code_generator::Quadruple> CodeGeneratingVisitor::getQuadruples() const {
    return quadruples;
}

} /* namespace semantic_analyzer */

