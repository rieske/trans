#include "CodeGeneratingVisitor.h"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#include "../ast/ArithmeticExpression.h"
#include "../ast/ArrayAccess.h"
#include "../ast/ArrayDeclarator.h"
#include "../ast/AssignmentExpressionList.h"
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
#include "../ast/ListCarrier.h"
#include "../ast/LogicalExpression.h"
#include "../ast/LoopStatement.h"
#include "../ast/Operator.h"
#include "../ast/FormalArgument.h"
#include "../ast/Pointer.h"
#include "../ast/PointerCast.h"
#include "../ast/PostfixExpression.h"
#include "../ast/PrefixExpression.h"
#include "../ast/ReturnStatement.h"
#include "../ast/Term.h"
#include "../ast/TerminalSymbol.h"
#include "../ast/TranslationUnit.h"
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

void CodeGeneratingVisitor::visit(ast::TypeSpecifier&) {
}

void CodeGeneratingVisitor::visit(ast::AssignmentExpressionList& expressions) {
    for (auto& expression : expressions.getExpressions()) {
        expression->accept(*this);
    }
}

void CodeGeneratingVisitor::visit(ast::DeclarationList& declarations) {
    for (auto& declaration : declarations.getDeclarations()) {
        declaration->accept(*this);
    }
}

void CodeGeneratingVisitor::visit(ast::ArrayAccess& arrayAccess) {
    arrayAccess.getLeftOperand()->accept(*this);
    arrayAccess.getRightOperand()->accept(*this);

    auto offset = arrayAccess.getRightOperand()->getResultHolder();
    quadruples.push_back( { code_generator::INDEX, arrayAccess.getLeftOperand()->getResultHolder(), offset, arrayAccess.getResultHolder() });
    quadruples.push_back( { code_generator::INDEX_ADDR, arrayAccess.getLeftOperand()->getResultHolder(), offset, arrayAccess.getLvalue() });
}

void CodeGeneratingVisitor::visit(ast::FunctionCall& functionCall) {
    functionCall.getOperand()->accept(*this);
    functionCall.getArgumentList()->accept(*this);

    for (auto& expression : functionCall.getArgumentList()->getExpressions()) {
        quadruples.push_back( { code_generator::PARAM, expression->getResultHolder(), nullptr, nullptr });
    }

    quadruples.push_back( { code_generator::CALL, functionCall.getSymbol() });
    if (!functionCall.getType().isPlainVoid()) {
        quadruples.push_back( { code_generator::RETRIEVE, functionCall.getResultHolder(), nullptr, nullptr });
    }
}

void CodeGeneratingVisitor::visit(ast::Term& term) {
    if (term.getTypeSymbol() != "id") {
        quadruples.push_back( { term.getValue(), term.getResultHolder() });
    }
}

void CodeGeneratingVisitor::visit(ast::PostfixExpression& expression) {
    expression.getOperand()->accept(*this);

    if (expression.getOperator()->getLexeme() == "++") {
        quadruples.push_back( { code_generator::INC, expression.getResultHolder(), nullptr, expression.getResultHolder() });
    } else if (expression.getOperator()->getLexeme() == "--") {
        quadruples.push_back( { code_generator::DEC, expression.getResultHolder(), nullptr, expression.getResultHolder() });
    }
}

void CodeGeneratingVisitor::visit(ast::PrefixExpression& expression) {
    if (expression.getOperator()->getLexeme() == "++") {
        quadruples.push_back( { code_generator::INC, expression.getResultHolder(), nullptr, expression.getResultHolder() });
    } else if (expression.getOperator()->getLexeme() == "--") {
        quadruples.push_back( { code_generator::DEC, expression.getResultHolder(), nullptr, expression.getResultHolder() });
    }

    expression.getOperand()->accept(*this);  // increment before evaluating the expression
}

void CodeGeneratingVisitor::visit(ast::UnaryExpression& expression) {
    expression.getOperand()->accept(*this);

    switch (expression.getOperator()->getLexeme().at(0)) {
    case '&':
        quadruples.push_back( { code_generator::ADDR, expression.getOperand()->getResultHolder(), nullptr, expression.getResultHolder() });
        expression.getOperand()->accept(*this);
        break;
    case '*':
        quadruples.push_back( { code_generator::DEREF, expression.getOperand()->getResultHolder(), nullptr, expression.getResultHolder() });
        expression.getOperand()->accept(*this);
        break;
    case '+':
        expression.getOperand()->accept(*this);
        break;
    case '-':
        expression.getOperand()->accept(*this);
        quadruples.push_back( { code_generator::UMINUS, expression.getOperand()->getResultHolder(), nullptr, expression.getResultHolder() });
        break;
    case '!':
        expression.getOperand()->accept(*this);
        quadruples.push_back( { code_generator::CMP, expression.getOperand()->getResultHolder(), nullptr, expression.getResultHolder() });
        quadruples.push_back( { code_generator::JE, expression.getTruthyLabel() });
        quadruples.push_back( { "0", expression.getResultHolder() });
        quadruples.push_back( { code_generator::GOTO, expression.getFalsyLabel() });
        quadruples.push_back( { code_generator::LABEL, expression.getTruthyLabel() });
        quadruples.push_back( { "1", expression.getResultHolder() });
        quadruples.push_back( { code_generator::LABEL, expression.getFalsyLabel() });
        break;
    default:
        throw std::runtime_error { "Unidentified increment operator: " + expression.getOperator()->getLexeme() };
    }
}

void CodeGeneratingVisitor::visit(ast::TypeCast& expression) {
    expression.getOperand()->accept(*this);

    quadruples.push_back( { code_generator::ASSIGN, expression.getOperand()->getResultHolder(), nullptr, expression.getResultHolder() });
}

void CodeGeneratingVisitor::visit(ast::PointerCast& expression) {
    expression.getPointer()->accept(*this);
    expression.getOperand()->accept(*this);

    quadruples.push_back( { code_generator::ASSIGN, expression.getOperand()->getResultHolder(), nullptr, expression.getResultHolder() });
}

void CodeGeneratingVisitor::visit(ast::ArithmeticExpression& expression) {
    expression.getLeftOperand()->accept(*this);
    expression.getRightOperand()->accept(*this);

    auto leftOperand = expression.getLeftOperand()->getResultHolder();
    auto rightOperand = expression.getRightOperand()->getResultHolder();
    auto resultHolder = expression.getResultHolder();
    switch (expression.getOperator()->getLexeme().at(0)) {
    case '+':
        quadruples.push_back( { code_generator::ADD, leftOperand, rightOperand, resultHolder });
        break;
    case '-':
        quadruples.push_back( { code_generator::SUB, leftOperand, rightOperand, resultHolder });
        break;
    case '*':
        quadruples.push_back( { code_generator::MUL, leftOperand, rightOperand, resultHolder });
        break;
    case '/':
        quadruples.push_back( { code_generator::DIV, leftOperand, rightOperand, resultHolder });
        break;
    case '%':
        quadruples.push_back( { code_generator::MOD, leftOperand, rightOperand, resultHolder });
        break;
    default:
        throw std::runtime_error { "unidentified arithmetic operator: " + expression.getOperator()->getLexeme() };
    }
}

void CodeGeneratingVisitor::visit(ast::ShiftExpression& expression) {
    expression.getLeftOperand()->accept(*this);
    expression.getRightOperand()->accept(*this);

    auto leftOperand = expression.getLeftOperand()->getResultHolder();
    auto rightOperand = expression.getRightOperand()->getResultHolder();
    auto resultHolder = expression.getResultHolder();
    switch (expression.getOperator()->getLexeme().at(0)) {
    case '<':   // <<
        quadruples.push_back( { code_generator::SHL, leftOperand, rightOperand, resultHolder });
        break;
    case '>':   // >>
        quadruples.push_back( { code_generator::SHR, leftOperand, rightOperand, resultHolder });
        break;
    default:
        throw std::runtime_error { "unidentified add_op operator!" };
    }
}

void CodeGeneratingVisitor::visit(ast::ComparisonExpression& expression) {
    expression.getLeftOperand()->accept(*this);
    expression.getRightOperand()->accept(*this);

    auto leftOperand = expression.getLeftOperand()->getResultHolder();
    auto rightOperand = expression.getRightOperand()->getResultHolder();
    quadruples.push_back( { code_generator::CMP, leftOperand, rightOperand, nullptr });

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

    quadruples.push_back( { "0", expression.getResultHolder() });
    quadruples.push_back( { code_generator::GOTO, expression.getFalsyLabel() });
    quadruples.push_back( { code_generator::LABEL, truthyLabel });
    quadruples.push_back( { "1", expression.getResultHolder() });
    quadruples.push_back( { code_generator::LABEL, expression.getFalsyLabel() });
}

void CodeGeneratingVisitor::visit(ast::BitwiseExpression& expression) {
    expression.getLeftOperand()->accept(*this);
    expression.getRightOperand()->accept(*this);

    code_generator::oper quadrupleOperator;
    switch (expression.getOperator()->getLexeme().at(0)) {
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
    auto leftOperand = expression.getLeftOperand()->getResultHolder();
    auto rightOperand = expression.getRightOperand()->getResultHolder();
    quadruples.push_back( { quadrupleOperator, leftOperand, rightOperand, expression.getResultHolder() });
}

void CodeGeneratingVisitor::visit(ast::LogicalAndExpression& expression) {
    expression.getLeftOperand()->accept(*this);

    auto leftOperand = expression.getLeftOperand()->getResultHolder();

    quadruples.push_back( { "0", expression.getResultHolder() });
    quadruples.push_back( { code_generator::CMP, leftOperand, 0, nullptr });
    quadruples.push_back( { code_generator::JE, expression.getExitLabel() });

    expression.getRightOperand()->accept(*this);

    auto rightOperand = expression.getRightOperand()->getResultHolder();
    quadruples.push_back( { code_generator::CMP, rightOperand, 0, nullptr });
    quadruples.push_back( { code_generator::JE, expression.getExitLabel() });
    quadruples.push_back( { "1", expression.getResultHolder() });

    quadruples.push_back( { code_generator::LABEL, expression.getExitLabel() });
}

void CodeGeneratingVisitor::visit(ast::LogicalOrExpression& expression) {
    expression.getLeftOperand()->accept(*this);

    auto leftOperand = expression.getLeftOperand()->getResultHolder();
    quadruples.push_back( { "1", expression.getResultHolder() });
    quadruples.push_back( { code_generator::CMP, leftOperand, 0, nullptr });
    quadruples.push_back( { code_generator::JNE, expression.getExitLabel() });

    expression.getRightOperand()->accept(*this);

    auto rightOperand = expression.getRightOperand()->getResultHolder();
    quadruples.push_back( { code_generator::CMP, rightOperand, 0, nullptr });
    quadruples.push_back( { code_generator::JNE, expression.getExitLabel() });
    quadruples.push_back( { "0", expression.getResultHolder() });

    quadruples.push_back( { code_generator::LABEL, expression.getExitLabel() });
}

void CodeGeneratingVisitor::visit(ast::AssignmentExpression& expression) {
    expression.getLeftOperand()->accept(*this);
    code_generator::ValueEntry* dereferencedLocation { nullptr };
    if (quadruples.back().getOp() == code_generator::DEREF) {
        dereferencedLocation = quadruples.back().getArg1();
        quadruples.pop_back();
    }
    expression.getRightOperand()->accept(*this);

    auto valueToAssign = expression.getRightOperand()->getResultHolder();
    auto resultPlace = expression.getResultHolder();
    auto assignmentOperator = expression.getOperator();
    if (assignmentOperator->getLexeme() == "+=")
        quadruples.push_back( { code_generator::ADD, resultPlace, valueToAssign, resultPlace });
    else if (assignmentOperator->getLexeme() == "-=")
        quadruples.push_back( { code_generator::SUB, resultPlace, valueToAssign, resultPlace });
    else if (assignmentOperator->getLexeme() == "*=")
        quadruples.push_back( { code_generator::MUL, resultPlace, valueToAssign, resultPlace });
    else if (assignmentOperator->getLexeme() == "/=")
        quadruples.push_back( { code_generator::DIV, resultPlace, valueToAssign, resultPlace });
    else if (assignmentOperator->getLexeme() == "%=")
        quadruples.push_back( { code_generator::MOD, resultPlace, valueToAssign, resultPlace });
    else if (assignmentOperator->getLexeme() == "&=")
        quadruples.push_back( { code_generator::AND, resultPlace, valueToAssign, resultPlace });
    else if (assignmentOperator->getLexeme() == "^=")
        quadruples.push_back( { code_generator::XOR, resultPlace, valueToAssign, resultPlace });
    else if (assignmentOperator->getLexeme() == "|=")
        quadruples.push_back( { code_generator::OR, resultPlace, valueToAssign, resultPlace });
    else if (assignmentOperator->getLexeme() == "<<=")
        quadruples.push_back( { code_generator::SHL, resultPlace, valueToAssign, resultPlace });
    else if (assignmentOperator->getLexeme() == ">>=")
        quadruples.push_back( { code_generator::SHR, resultPlace, valueToAssign, resultPlace });
    else if (assignmentOperator->getLexeme() == "=") {
        if (dereferencedLocation) {
            // FIXME:
            quadruples.push_back( { code_generator::DEREF_LVAL, valueToAssign, nullptr, dereferencedLocation });
        } else {
            quadruples.push_back( { code_generator::ASSIGN, valueToAssign, nullptr, resultPlace });
        }
    } else {
        throw std::runtime_error { "unidentified assignment operator: " + assignmentOperator->getLexeme() };
    }
}

void CodeGeneratingVisitor::visit(ast::ExpressionList& expression) {
    expression.getLeftOperand()->accept(*this);
    expression.getRightOperand()->accept(*this);
}

void CodeGeneratingVisitor::visit(ast::Operator&) {
}

void CodeGeneratingVisitor::visit(ast::JumpStatement& statement) {
    throw std::runtime_error { "not implemented" };
}

void CodeGeneratingVisitor::visit(ast::ReturnStatement& statement) {
    statement.returnExpression->accept(*this);
    quadruples.push_back( { code_generator::RETURN, statement.returnExpression->getResultHolder(), nullptr, nullptr });
}

void CodeGeneratingVisitor::visit(ast::IOStatement& statement) {
    statement.expression->accept(*this);
    if (statement.ioKeyword.value == "output") {
        quadruples.push_back( { code_generator::OUT, statement.expression->getResultHolder(), nullptr, nullptr });
    } else if (statement.ioKeyword.value == "input") {
        quadruples.push_back( { code_generator::IN, statement.expression->getResultHolder(), nullptr, nullptr });
    } else {
        throw std::runtime_error { "bad IO statement: " + statement.ioKeyword.type };
    }
}

void CodeGeneratingVisitor::visit(ast::IfStatement& statement) {
    statement.testExpression->accept(*this);
    quadruples.push_back( { code_generator::CMP, statement.testExpression->getResultHolder(), 0, nullptr });
    quadruples.push_back( { code_generator::JE, statement.getFalsyLabel() });

    statement.body->accept(*this);
    quadruples.push_back( { code_generator::LABEL, statement.getFalsyLabel() });

}

void CodeGeneratingVisitor::visit(ast::IfElseStatement& statement) {
    statement.testExpression->accept(*this);

    quadruples.push_back( { code_generator::CMP, statement.testExpression->getResultHolder(), 0, nullptr });
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
    quadruples.push_back( { code_generator::CMP, loopHeader.clause->getResultHolder(), 0, nullptr });
    quadruples.push_back( { code_generator::JE, loopHeader.getLoopExit() });
}

void CodeGeneratingVisitor::visit(ast::WhileLoopHeader& loopHeader) {
    quadruples.push_back( { code_generator::LABEL, loopHeader.getLoopEntry() });
    loopHeader.clause->accept(*this);
    quadruples.push_back( { code_generator::CMP, loopHeader.clause->getResultHolder(), 0, nullptr });
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

    auto functionHolder = function.getSymbol();
    quadruples.push_back( { code_generator::PROC, functionHolder });
    function.body->accept(*this);
    quadruples.push_back( { code_generator::ENDPROC, functionHolder });
}

void CodeGeneratingVisitor::visit(ast::Block& block) {
    quadruples.push_back( { code_generator::SCOPE, block.getSymbols(), block.getArguments() });
    for (auto& statement : block.getChildren()) {
        statement->accept(*this);
    }
    quadruples.push_back( { code_generator::ENDSCOPE, block.getSymbols(), block.getArguments() });
}

void CodeGeneratingVisitor::visit(ast::VariableDeclaration& declaration) {
    declaration.declaredVariables->accept(*this);
}

void CodeGeneratingVisitor::visit(ast::VariableDefinition& definition) {
    definition.declaration->accept(*this);
    definition.initializerExpression->accept(*this);

    auto& declaredVariables = definition.declaration->declaredVariables->getDeclarations();
    auto& lastVariableInDeclaration = declaredVariables.at(declaredVariables.size() - 1);
    quadruples.push_back( { code_generator::ASSIGN, definition.initializerExpression->getResultHolder(), nullptr, lastVariableInDeclaration->getHolder() });
}

void CodeGeneratingVisitor::visit(ast::ListCarrier& listCarrier) {
    for (const auto& child : listCarrier.getChildren()) {
        child->accept(*this);
    }
}

void CodeGeneratingVisitor::visit(ast::TranslationUnit& translationUnit) {
    for (const auto& child : translationUnit.getChildren()) {
        child->accept(*this);
    }
}

std::vector<code_generator::Quadruple> CodeGeneratingVisitor::getQuadruples() const {
    return quadruples;
}

} /* namespace semantic_analyzer */

