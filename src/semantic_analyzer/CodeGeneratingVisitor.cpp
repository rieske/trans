#include "CodeGeneratingVisitor.h"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

#include "../ast/ArithmeticExpression.h"
#include "../ast/ArrayAccess.h"
#include "../ast/ArrayDeclaration.h"
#include "../ast/AssignmentExpressionList.h"
#include "../ast/BitwiseExpression.h"
#include "../ast/Block.h"
#include "../ast/ComparisonExpression.h"
#include "../ast/DeclarationList.h"
#include "../ast/ForLoopHeader.h"
#include "../ast/FunctionCall.h"
#include "../ast/FunctionDeclaration.h"
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
#include "../ast/ParameterDeclaration.h"
#include "../ast/ParameterList.h"
#include "../ast/Pointer.h"
#include "../ast/PointerCast.h"
#include "../ast/PostfixExpression.h"
#include "../ast/PrefixExpression.h"
#include "../ast/ReturnStatement.h"
#include "../ast/Term.h"
#include "../ast/TerminalSymbol.h"
#include "../ast/TranslationUnit.h"
#include "../ast/UnaryExpression.h"
#include "../ast/VariableDeclaration.h"
#include "../ast/VariableDefinition.h"
#include "../ast/WhileLoopHeader.h"
#include "ast/ArithmeticExpression.h"
#include "ast/ArrayAccess.h"
#include "ast/BitwiseExpression.h"
#include "ast/ExpressionList.h"
#include "ast/LogicalAndExpression.h"
#include "ast/LogicalOrExpression.h"
#include "ast/PostfixExpression.h"
#include "ast/PrefixExpression.h"
#include "ast/ShiftExpression.h"
#include "ast/TypeCast.h"

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

void CodeGeneratingVisitor::visit(ast::ParameterList& parameterList) {
    for (auto& parameter : parameterList.getDeclaredParameters()) {
        parameter->accept(*this);
    }
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
    quadruples.push_back( { INDEX, arrayAccess.getLeftOperand()->getResultHolder(), offset, arrayAccess.getResultHolder() });
    quadruples.push_back( { INDEX_ADDR, arrayAccess.getLeftOperand()->getResultHolder(), offset, arrayAccess.getLvalue() });
}

void CodeGeneratingVisitor::visit(ast::FunctionCall& functionCall) {
    functionCall.getOperand()->accept(*this);
    functionCall.getArgumentList()->accept(*this);

    for (auto& expression : functionCall.getArgumentList()->getExpressions()) {
        quadruples.push_back( { PARAM, expression->getResultHolder(), nullptr, nullptr });
    }

    quadruples.push_back( { CALL, functionCall.getOperand()->getResultHolder(), nullptr, nullptr });
    if (!functionCall.getTypeInfo().isPlainVoid()) {
        quadruples.push_back( { RETRIEVE, functionCall.getResultHolder(), nullptr, nullptr });
    }
}

void CodeGeneratingVisitor::visit(ast::Term& term) {
    if (term.getType() != "id") {
        quadruples.push_back( { term.getValue(), term.getResultHolder() });
    }
}

void CodeGeneratingVisitor::visit(ast::PostfixExpression& expression) {
    expression.getOperand()->accept(*this);

    if (expression.getOperator()->getLexeme() == "++") {
        quadruples.push_back( { INC, expression.getResultHolder(), nullptr, expression.getResultHolder() });
    } else if (expression.getOperator()->getLexeme() == "--") {
        quadruples.push_back( { DEC, expression.getResultHolder(), nullptr, expression.getResultHolder() });
    }
}

void CodeGeneratingVisitor::visit(ast::PrefixExpression& expression) {
    if (expression.getOperator()->getLexeme() == "++") {
        quadruples.push_back( { INC, expression.getResultHolder(), nullptr, expression.getResultHolder() });
    } else if (expression.getOperator()->getLexeme() == "--") {
        quadruples.push_back( { DEC, expression.getResultHolder(), nullptr, expression.getResultHolder() });
    }

    expression.getOperand()->accept(*this);  // increment before evaluating the expression
}

void CodeGeneratingVisitor::visit(ast::UnaryExpression& expression) {
    expression.getOperand()->accept(*this);

    switch (expression.getOperator()->getLexeme().at(0)) {
    case '&':
        quadruples.push_back( { ADDR, expression.getOperand()->getResultHolder(), nullptr, expression.getResultHolder() });
        expression.getOperand()->accept(*this);
        break;
    case '*':
        quadruples.push_back( { DEREF, expression.getOperand()->getResultHolder(), nullptr, expression.getResultHolder() });
        expression.getOperand()->accept(*this);
        break;
    case '+':
        expression.getOperand()->accept(*this);
        break;
    case '-':
        expression.getOperand()->accept(*this);
        quadruples.push_back( { UMINUS, expression.getOperand()->getResultHolder(), nullptr, expression.getResultHolder() });
        break;
    case '!':
        expression.getOperand()->accept(*this);
        quadruples.push_back( { CMP, expression.getOperand()->getResultHolder(), 0, expression.getResultHolder() });
        quadruples.push_back( { JE, expression.getTruthyLabel(), nullptr, nullptr });
        quadruples.push_back( { "0", expression.getResultHolder() });
        quadruples.push_back( { GOTO, expression.getFalsyLabel(), nullptr, nullptr });
        quadruples.push_back( { LABEL, expression.getTruthyLabel(), nullptr, nullptr });
        quadruples.push_back( { "1", expression.getResultHolder() });
        quadruples.push_back( { LABEL, expression.getFalsyLabel(), nullptr, nullptr });
        break;
    default:
        throw std::runtime_error { "Unidentified increment operator: " + expression.getOperator()->getLexeme() };
    }
}

void CodeGeneratingVisitor::visit(ast::TypeCast& expression) {
    expression.getOperand()->accept(*this);

    quadruples.push_back( { ASSIGN, expression.getOperand()->getResultHolder(), nullptr, expression.getResultHolder() });
}

void CodeGeneratingVisitor::visit(ast::PointerCast& expression) {
    expression.getPointer()->accept(*this);
    expression.getOperand()->accept(*this);

    quadruples.push_back( { ASSIGN, expression.getOperand()->getResultHolder(), nullptr, expression.getResultHolder() });
}

void CodeGeneratingVisitor::visit(ast::ArithmeticExpression& expression) {
    expression.getLeftOperand()->accept(*this);
    expression.getRightOperand()->accept(*this);

    auto leftOperand = expression.getLeftOperand()->getResultHolder();
    auto rightOperand = expression.getRightOperand()->getResultHolder();
    auto resultHolder = expression.getResultHolder();
    switch (expression.getOperator()->getLexeme().at(0)) {
    case '+':
        quadruples.push_back( { ADD, leftOperand, rightOperand, resultHolder });
        break;
    case '-':
        quadruples.push_back( { SUB, leftOperand, rightOperand, resultHolder });
        break;
    case '*':
        quadruples.push_back( { MUL, leftOperand, rightOperand, resultHolder });
        break;
    case '/':
        quadruples.push_back( { DIV, leftOperand, rightOperand, resultHolder });
        break;
    case '%':
        quadruples.push_back( { MOD, leftOperand, rightOperand, resultHolder });
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
        quadruples.push_back( { SHL, leftOperand, rightOperand, resultHolder });
        break;
    case '>':   // >>
        quadruples.push_back( { SHR, leftOperand, rightOperand, resultHolder });
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
    quadruples.push_back( { CMP, leftOperand, rightOperand, nullptr });

    auto truthyLabel = expression.getTruthyLabel();
    if (expression.getOperator()->getLexeme() == ">") {
        quadruples.push_back( { JA, truthyLabel, nullptr, nullptr });
    } else if (expression.getOperator()->getLexeme() == "<") {
        quadruples.push_back( { JB, truthyLabel, nullptr, nullptr });
    } else if (expression.getOperator()->getLexeme() == "<=") {
        quadruples.push_back( { JBE, truthyLabel, nullptr, nullptr });
    } else if (expression.getOperator()->getLexeme() == ">=") {
        quadruples.push_back( { JAE, truthyLabel, nullptr, nullptr });
    } else if (expression.getOperator()->getLexeme() == "==") {
        quadruples.push_back( { JE, truthyLabel, nullptr, nullptr });
    } else if (expression.getOperator()->getLexeme() == "!=") {
        quadruples.push_back( { JNE, truthyLabel, nullptr, nullptr });
    } else {
        throw std::runtime_error { "unidentified ml_op operator!\n" };
    }

    quadruples.push_back( { "0", expression.getResultHolder() });
    quadruples.push_back( { GOTO, expression.getFalsyLabel(), nullptr, nullptr });
    quadruples.push_back( { LABEL, truthyLabel, nullptr, nullptr });
    quadruples.push_back( { "1", expression.getResultHolder() });
    quadruples.push_back( { LABEL, expression.getFalsyLabel(), nullptr, nullptr });
}

void CodeGeneratingVisitor::visit(ast::BitwiseExpression& expression) {
    expression.getLeftOperand()->accept(*this);
    expression.getRightOperand()->accept(*this);

    oper quadrupleOperator;
    switch (expression.getOperator()->getLexeme().at(0)) {
    case '&':
        quadrupleOperator = oper::AND;
        break;
    case '|':
        quadrupleOperator = oper::OR;
        break;
    case '^':
        quadrupleOperator = oper::XOR;
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
    quadruples.push_back( { CMP, leftOperand, 0, nullptr });
    quadruples.push_back( { JE, expression.getExitLabel(), nullptr, nullptr });

    expression.getRightOperand()->accept(*this);

    auto rightOperand = expression.getRightOperand()->getResultHolder();
    quadruples.push_back( { CMP, rightOperand, 0, nullptr });
    quadruples.push_back( { JE, expression.getExitLabel(), nullptr, nullptr });
    quadruples.push_back( { "1", expression.getResultHolder() });

    quadruples.push_back( { LABEL, expression.getExitLabel(), nullptr, nullptr });
}

void CodeGeneratingVisitor::visit(ast::LogicalOrExpression& expression) {
    expression.getLeftOperand()->accept(*this);

    auto leftOperand = expression.getLeftOperand()->getResultHolder();
    quadruples.push_back( { "1", expression.getResultHolder() });
    quadruples.push_back( { CMP, leftOperand, 0, nullptr });
    quadruples.push_back( { JNE, expression.getExitLabel(), nullptr, nullptr });

    expression.getRightOperand()->accept(*this);

    auto rightOperand = expression.getRightOperand()->getResultHolder();
    quadruples.push_back( { CMP, rightOperand, 0, nullptr });
    quadruples.push_back( { JNE, expression.getExitLabel(), nullptr, nullptr });
    quadruples.push_back( { "0", expression.getResultHolder() });

    quadruples.push_back( { LABEL, expression.getExitLabel(), nullptr, nullptr });
}

void CodeGeneratingVisitor::visit(ast::AssignmentExpression& expression) {
    expression.getLeftOperand()->accept(*this);
    SymbolEntry* dereferencedLocation { nullptr };
    if (quadruples.back().getOp() == DEREF) {
        dereferencedLocation = quadruples.back().getArg1();
        quadruples.pop_back();
    }
    expression.getRightOperand()->accept(*this);

    auto valueToAssign = expression.getRightOperand()->getResultHolder();
    auto resultPlace = expression.getResultHolder();
    auto assignmentOperator = expression.getOperator();
    if (assignmentOperator->getLexeme() == "+=")
        quadruples.push_back( { ADD, resultPlace, valueToAssign, resultPlace });
    else if (assignmentOperator->getLexeme() == "-=")
        quadruples.push_back( { SUB, resultPlace, valueToAssign, resultPlace });
    else if (assignmentOperator->getLexeme() == "*=")
        quadruples.push_back( { MUL, resultPlace, valueToAssign, resultPlace });
    else if (assignmentOperator->getLexeme() == "/=")
        quadruples.push_back( { DIV, resultPlace, valueToAssign, resultPlace });
    else if (assignmentOperator->getLexeme() == "%=")
        quadruples.push_back( { MOD, resultPlace, valueToAssign, resultPlace });
    else if (assignmentOperator->getLexeme() == "&=")
        quadruples.push_back( { AND, resultPlace, valueToAssign, resultPlace });
    else if (assignmentOperator->getLexeme() == "^=")
        quadruples.push_back( { XOR, resultPlace, valueToAssign, resultPlace });
    else if (assignmentOperator->getLexeme() == "|=")
        quadruples.push_back( { OR, resultPlace, valueToAssign, resultPlace });
    else if (assignmentOperator->getLexeme() == "<<=")
        quadruples.push_back( { SHL, resultPlace, valueToAssign, resultPlace });
    else if (assignmentOperator->getLexeme() == ">>=")
        quadruples.push_back( { SHR, resultPlace, valueToAssign, resultPlace });
    else if (assignmentOperator->getLexeme() == "=") {
        if (dereferencedLocation) {
            quadruples.push_back( { DEREF_LVAL, valueToAssign, nullptr, dereferencedLocation });
        } else {
            quadruples.push_back( { ASSIGN, valueToAssign, nullptr, resultPlace });
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
    quadruples.push_back( { RETURN, statement.returnExpression->getResultHolder(), nullptr, nullptr });
}

void CodeGeneratingVisitor::visit(ast::IOStatement& statement) {
    statement.expression->accept(*this);
    if (statement.ioKeyword.value == "output") {
        quadruples.push_back( { OUT, statement.expression->getResultHolder(), nullptr, nullptr });
    } else if (statement.ioKeyword.value == "input") {
        quadruples.push_back( { IN, statement.expression->getResultHolder(), nullptr, nullptr });
    } else {
        throw std::runtime_error { "bad IO statement: " + statement.ioKeyword.type };
    }
}

void CodeGeneratingVisitor::visit(ast::IfStatement& statement) {
    statement.testExpression->accept(*this);
    quadruples.push_back( { CMP, statement.testExpression->getResultHolder(), 0, nullptr });
    quadruples.push_back( { JE, statement.getFalsyLabel(), nullptr, nullptr });

    statement.body->accept(*this);
    quadruples.push_back( { LABEL, statement.getFalsyLabel(), nullptr, nullptr });

}

void CodeGeneratingVisitor::visit(ast::IfElseStatement& statement) {
    statement.testExpression->accept(*this);

    quadruples.push_back( { CMP, statement.testExpression->getResultHolder(), 0, nullptr });
    quadruples.push_back( { JE, statement.getFalsyLabel(), nullptr, nullptr });

    statement.truthyBody->accept(*this);
    quadruples.push_back( { GOTO, statement.getExitLabel(), nullptr, nullptr });
    quadruples.push_back( { LABEL, statement.getFalsyLabel(), nullptr, nullptr });

    statement.falsyBody->accept(*this);
    quadruples.push_back( { LABEL, statement.getExitLabel(), nullptr, nullptr });
}

void CodeGeneratingVisitor::visit(ast::LoopStatement& loop) {
    loop.header->accept(*this);
    loop.body->accept(*this);
    // FIXME:
    if (loop.header->increment) {
        loop.header->increment->accept(*this);
    }

    quadruples.push_back( { GOTO, loop.header->getLoopEntry(), nullptr, nullptr });
    quadruples.push_back( { LABEL, loop.header->getLoopExit(), nullptr, nullptr });
}

void CodeGeneratingVisitor::visit(ast::ForLoopHeader& loopHeader) {
    loopHeader.initialization->accept(*this);

    quadruples.push_back( { LABEL, loopHeader.getLoopEntry(), nullptr, nullptr });
    loopHeader.clause->accept(*this);
    quadruples.push_back( { CMP, loopHeader.clause->getResultHolder(), 0, nullptr });
    quadruples.push_back( { JE, loopHeader.getLoopExit(), nullptr, nullptr });
}

void CodeGeneratingVisitor::visit(ast::WhileLoopHeader& loopHeader) {
    quadruples.push_back( { LABEL, loopHeader.getLoopEntry(), nullptr, nullptr });
    loopHeader.clause->accept(*this);
    quadruples.push_back( { CMP, loopHeader.clause->getResultHolder(), 0, nullptr });
    quadruples.push_back( { JE, loopHeader.getLoopExit(), nullptr, nullptr });
}

void CodeGeneratingVisitor::visit(ast::Pointer&) {
}

void CodeGeneratingVisitor::visit(ast::Identifier&) {
}

void CodeGeneratingVisitor::visit(ast::FunctionDeclaration& declaration) {
    declaration.parameterList->accept(*this);
}

void CodeGeneratingVisitor::visit(ast::ArrayDeclaration& declaration) {
    declaration.subscriptExpression->accept(*this);
    throw std::runtime_error { "not implemented" };
}

void CodeGeneratingVisitor::visit(ast::ParameterDeclaration& parameter) {
    parameter.declaration->accept(*this);
}

void CodeGeneratingVisitor::visit(ast::FunctionDefinition& function) {
    function.declaration->accept(*this);

    auto functionHolder = function.declaration->getHolder();
    quadruples.push_back( { PROC, functionHolder, nullptr, nullptr });
    function.body->accept(*this);
    quadruples.push_back( { ENDPROC, functionHolder, nullptr, nullptr });
}

void CodeGeneratingVisitor::visit(ast::VariableDeclaration& declaration) {
    declaration.declaredVariables->accept(*this);
}

void CodeGeneratingVisitor::visit(ast::VariableDefinition& definition) {
    definition.declaration->accept(*this);
    definition.initializerExpression->accept(*this);

    auto& declaredVariables = definition.declaration->declaredVariables->getDeclarations();
    auto& lastVariableInDeclaration = declaredVariables.at(declaredVariables.size() - 1);
    quadruples.push_back( { ASSIGN, definition.initializerExpression->getResultHolder(), nullptr, lastVariableInDeclaration->getHolder() });
}

void CodeGeneratingVisitor::visit(ast::Block& block) {
    quadruples.push_back( { SCOPE, nullptr, nullptr, nullptr });
    for (auto& statement : block.getChildren()) {
        statement->accept(*this);
    }
    quadruples.push_back( { ENDSCOPE, nullptr, nullptr, nullptr });
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

std::vector<Quadruple> CodeGeneratingVisitor::getQuadruples() const {
    return quadruples;
}

} /* namespace semantic_analyzer */

